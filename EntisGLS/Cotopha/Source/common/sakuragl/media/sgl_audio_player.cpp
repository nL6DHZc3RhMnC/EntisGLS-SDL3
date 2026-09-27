
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_audio_decoding_player.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_dshow_audio_player.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_media_player.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// オーディオファイル再生インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioPlayerInterface, SObject )



//////////////////////////////////////////////////////////////////////////////
// オーディオファイル再生ラッパー
//////////////////////////////////////////////////////////////////////////////

// チェーン先頭
SGLAudioPlayer *			SGLAudioPlayer::m_pFirstAudioPlayer = NULL ;
ESL_DLL_DECL( SSystem::SCriticalSection *	SGLAudioPlayer::m_pMutexAll = NULL ) ;

// フェード処理用ストックスレッド
SThread *			SGLAudioPlayer::m_pFadingThread = NULL ;
bool				SGLAudioPlayer::m_flagReqFadingThread = false ;

// 全体音量
float32_t	SGLAudioPlayer::m_volumeOfLine[SGLAudioPlayer::lineCount] =
{
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioPlayer, SGLAudioPlayerInterface )
#else
ESL_IMPLEMENT_CLASS_INFO_CAST
	( SakuraGL::SGLAudioPlayer, SGLAudioPlayerInterface, m_pPlayer )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayer::SGLAudioPlayer( const SGLAudioPlayer& audio )
	: m_pPlayer(NULL), m_flagOwner(false),
		m_maskLines(audio.m_maskLines),
		m_volumes(audio.m_volumes),
		m_pPrevAudio(NULL), m_pNextAudio(NULL),
		m_msecFadingDuration(0), m_msecFadingTime(0)
{
	if ( audio.m_pPlayer != NULL )
	{
		m_pPlayer = audio.m_pPlayer->ClonePlayer() ;
		m_flagOwner = true ;
		ReflectVolume() ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayer::~SGLAudioPlayer( void )
{
	m_pMutexAll->Lock() ;
	m_mutex.Lock() ;
	if ( (m_pFirstAudioPlayer == this)
		| (m_pPrevAudio != NULL) | (m_pNextAudio != NULL) )
	{
		DetachFromAudioChain() ;
	}
	m_mutex.Unlock() ;
	m_pMutexAll->Unlock() ;

	if ( m_flagOwner )
	{
		delete	m_pPlayer ;
		m_pPlayer = NULL ;
		m_flagOwner = false ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
AudioPlayer * SGLAudioPlayer::SetAudioPlayer
				( AudioPlayer * pPlayer, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pPlayer ;
	}
	m_pPlayer = pPlayer ;
	m_flagOwner = flagOwner ;

	#if	defined(__COTOPHA__)
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagFileOwner = false ;
	#endif
	return	m_pPlayer ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
{
	if ( m_pPlayer == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pPlayer = new AudioPlayer ;
		#else
			m_pPlayer = new SGLAudioDecodingPlayer ;
		#endif
		m_flagOwner = true ;
	}
	SGLError	err ;
	#if	defined(__COTOPHA__)
		err = m_pPlayer->Open( pwszFilePath, nFlags ) ;
	#else
		err = m_pPlayer->Open( pwszFilePath, nFlags, pEnv ) ;
		#if	defined(__PLATFORM_WINDOWS__)
			if ( err )
			{
				if ( m_flagOwner )
				{
					delete	m_pPlayer ;
				}
				m_pPlayer = new SGLDirectShowAudioPlayer ;
				m_flagOwner = true ;
				err = m_pPlayer->Open( pwszFilePath, nFlags, pEnv ) ;
				if ( err )
				{
					delete	m_pPlayer ;
					m_flagOwner = false ;
					m_pPlayer = NULL ;
				}
			}
		#elif	defined(__PLATFORM_ANDROID__)
			if ( err )
			{
				if ( m_flagOwner )
				{
					delete	m_pPlayer ;
				}
				m_pPlayer = new SGLAndroidMediaPlayer( false ) ;
				m_flagOwner = true ;
				err = m_pPlayer->Open( pwszFilePath, nFlags, pEnv ) ;
				if ( err )
				{
					delete	m_pPlayer ;
					m_flagOwner = false ;
					m_pPlayer = NULL ;
				}
			}
		#endif
	#endif
	ReflectVolume() ;
	return	err ;
}

SGLError SGLAudioPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	if ( m_pPlayer == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pPlayer = new AudioPlayer ;
		#else
			m_pPlayer = new SGLAudioDecodingPlayer ;
		#endif
		m_flagOwner = true ;
	}
	#if	defined(__COTOPHA__)
		File *	pFile = file->GetFileObject() ;
		if ( pFile == NULL )
		{
			return	sglErrFailed ;
		}
		SGLError	err = m_pPlayer->Create( pFile, nFlags ) ;
		if ( m_flagFileOwner )
		{
			delete	m_pFile ;
		}
		m_pFile = file ;
		m_flagFileOwner = flagOwner ;
		ReflectVolume() ;
		return	err ;
	#else
		SGLError	err = m_pPlayer->Create( file, flagOwner, nFlags ) ;
		ReflectVolume() ;
		return	err ;
	#endif
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLAudioPlayer::ClonePlayer( void )
{
	if ( m_pPlayer == NULL )
	{
		return	new SGLAudioPlayer ;
	}
	SGLAudioPlayer *	pPlayer =
		new SGLAudioPlayer( m_pPlayer->ClonePlayer(), true ) ;
	pPlayer->m_maskLines = m_maskLines ;
	pPlayer->m_volumes = m_volumes ;
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Close( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	DetachFromAudioChain() ;

	SGLError	err = m_pPlayer->Close() ;

	#if	defined(__COTOPHA__)
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagFileOwner = false ;
	#endif

	return	err ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Play( uint64_t nFlags )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	ReflectVolume() ;
	AddToAudioChain() ;
	return	m_pPlayer->Play( nFlags ) ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Stop( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	DetachFromAudioChain() ;
	m_msecFadingDuration = 0 ;
	return	m_pPlayer->Stop() ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::SetLoop
	( bool fLoop , int64_t nStart, int64_t nEnd )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->SetLoop( fLoop, nStart, nEnd ) ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Pause( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Pause() ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::Restart( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Restart() ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	const float32_t *	pAudioVol = m_volumes.GetConstArray() ;
	const size_t		nVolCount = m_volumes.GetLength() ;
	size_t	 i ;
	for ( i = 0; i < nVolCount; i ++ )
	{
		if ( i >= nChannels )
		{
			break ;
		}
		pVolumes[i] = pAudioVol[i] ;
	}
	for ( i = nVolCount; i < nChannels; i ++ )
	{
		pVolumes[i] = 1.0f ;
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		m_volumes.SetAt( i, pVolumes[i] ) ;
	}
	ReflectVolume() ;
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioPlayer::IsPlaying( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	false ;
	}
	return	m_pPlayer->IsPlaying() ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioPlayer::IsPaused( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	false ;
	}
	return	m_pPlayer->IsPaused() ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLAudioPlayer::GetSampleFrequency( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	0 ;
	}
	return	m_pPlayer->GetSampleFrequency() ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAudioPlayer::GetTotalLength( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	0 ;
	}
	return	m_pPlayer->GetTotalLength() ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAudioPlayer::GetPosition( void )
{
	if ( m_pPlayer == NULL )
	{
		return	0 ;
	}
	return	m_pPlayer->GetPosition() ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_pPlayer == NULL )
	{
		return ;
	}
	m_pPlayer->SeekPosition( nPos ) ;
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLAudioPlayer::GetAudioStream( void )
{
	if ( m_pPlayer == NULL )
	{
		return	NULL ;
	}
	return	m_pPlayer->GetAudioStream() ;
}

void SGLAudioPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
	if ( m_pPlayer != NULL )
	{
		m_pPlayer->ReleaseAudioStream( pStream ) ;
	}
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	if ( m_pPlayer != nullptr )
	{
		m_pPlayer->SetUIThreadMutex( pMutex ) ;
	}
}

// 静的な処理の初期化
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::InitializeStatic( void )
{
	m_pMutexAll = new SCriticalSection ;
}

// 静的な処理の解放
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::ReleaseStatic( void )
{
	delete	m_pMutexAll ;
	m_pMutexAll = nullptr ;
}

// 音量を反映させるラインを取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLAudioPlayer::GetVolumeLineMask( void ) const
{
	return	m_maskLines ;
}

// 音量を反映させるラインをクリアする（lineTotal,lineTotal2nd 以外）
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::ClearAllVolumeLines( void )
{
	SetVolumeLineMask( 0 ) ;
}

// 音量を反映させるラインを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::SetVolumeLineMask( uint32_t maskLines )
{
	m_maskLines = maskLines | (1 << lineTotal) | (1 << lineTotal2nd) ;
	ReflectVolume() ;
}

void SGLAudioPlayer::SetVolumeLine( size_t iLine )
{
	m_maskLines |= (1 << iLine) ;
	ReflectVolume() ;
}

void SGLAudioPlayer::ResetVolumeLine( size_t iLine )
{
	m_maskLines &= ~(1 << iLine) ;
	m_maskLines |= (1 << lineTotal) | (1 << lineTotal2nd) ;
	ReflectVolume() ;
}

void SGLAudioPlayer::SetAudioLineMask
	( SGLAudioPlayerInterface * pPlayer, uint32_t maskLines )
{
	SGLAudioPlayer *	pAudio = ESLTypeCast<SGLAudioPlayer>( pPlayer ) ;
	if ( pAudio != NULL )
	{
		pAudio->SetVolumeLineMask( maskLines ) ;
	}
}

// ライン音量を含め音量反映
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::ReflectVolume( void )
{
	if ( m_pPlayer == NULL )
	{
		return ;
	}
	float32_t	volumes[0x10] =
	{
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	} ;
	size_t	nChannels = 2 ;
	size_t	nCount = m_volumes.GetLength() ;
	if ( nCount > 0x10 )
	{
		nCount = 0x10 ;
	}
	const float32_t *	pVolumes = m_volumes.GetConstArray() ;
	size_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		volumes[i] = pVolumes[i] ;
	}
	if ( nChannels < nCount )
	{
		nChannels = nCount ;
	}
	uint32_t	maskLines = m_maskLines ;
	size_t		iLine = 0 ;
	while ( maskLines != 0 )
	{
		if ( maskLines & 0x01 )
		{
			float32_t	volumeLine = m_volumeOfLine[iLine] ;
			for ( i = 0; i < nChannels; i ++ )
			{
				volumes[i] *= volumeLine ;
			}
		}
		maskLines >>= 1 ;
		iLine ++ ;
	}
	m_pPlayer->SetVolume( &volumes[0], nChannels ) ;
}

// ライン音量設定
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::SetLineVolume( size_t iLine, double volume )
{
	if ( iLine < lineCount )
	{
		m_volumeOfLine[iLine] = (float32_t) volume ;
		ReflectVolumesAllChain() ;
	}
}

// ライン音量取得
//////////////////////////////////////////////////////////////////////////////
double SGLAudioPlayer::GetLineVolume( size_t iLine )
{
	if ( iLine < lineCount )
	{
		return	m_volumeOfLine[iLine] ;
	}
	return	1.0 ;
}

// 音量設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::SetStereoVolume( float32_t volLeft, float32_t volRight )
{
	float32_t	vols[2] = { volLeft, volRight } ;
	return	SetVolume( vols, 2 ) ;
}

// 音量取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioPlayer::GetStereoVolume( float32_t& volLeft, float32_t& volRight )
{
	float32_t	vols[2] = { 1.0, 1.0 } ;
	SGLError	err = GetVolume( vols, 2 ) ;
	if ( !err )
	{
		volLeft = vols[0] ;
		volRight = vols[1] ;
	}
	return	err ;
}

// 音量フェーディング開始
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::BeginFadeVolume
	( const float32_t* pVolumes, size_t nChannels, uint32_t msecDuration )
{
	CancelFadeVolume() ;
	//
	if ( msecDuration > 0 )
	{
		m_pMutexAll->Lock() ;
		m_mutex.Lock() ;
		m_volFadeStart = m_volumes ;
		m_volFadeEnd.RemoveAll() ;
		m_volFadeEnd.AddArray( pVolumes, nChannels ) ;
		for ( size_t i = m_volFadeStart.GetLength();
							i < m_volFadeEnd.GetLength(); i ++ )
		{
			m_volFadeStart.SetAt( i, 1.0f ) ;
		}
		m_msecFadingDuration = msecDuration ;
		m_msecFadingTime = 0 ;
		m_mutex.Unlock() ;
		m_pMutexAll->Unlock() ;
		//
		BeginFadingThread() ;
	}
	else
	{
		SetVolume( pVolumes, nChannels ) ;
	}
}

// 音量フェード中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioPlayer::IsVolumeFading( void ) const
{
	return	(m_msecFadingDuration > 0) ;
}

// 音量フェーディングキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::CancelFadeVolume( void )
{
	if ( m_msecFadingDuration > 0 )
	{
		m_pMutexAll->Lock() ;
		m_mutex.Lock() ;
		m_volFadeStart.FreeArray() ;
		m_volFadeEnd.FreeArray() ;
		m_msecFadingDuration = 0 ;
		m_msecFadingTime = 0 ;
		m_mutex.Unlock() ;
		m_pMutexAll->Unlock() ;
	}
}

// 音量フェーディング終了
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::FlushFadeVolume( void )
{
	m_pMutexAll->Lock() ;
	m_mutex.Lock() ;
	if ( m_msecFadingDuration > 0 )
	{
		if ( m_volFadeEnd.GetLength() > 0 )
		{
			SetVolume( m_volFadeEnd.GetConstArray(), m_volFadeEnd.GetLength() ) ;
		}
		m_volFadeStart.FreeArray() ;
		m_volFadeEnd.FreeArray() ;
		m_msecFadingDuration = 0 ;
		m_msecFadingTime = 0 ;
	}
	m_mutex.Unlock() ;
	m_pMutexAll->Unlock() ;
}

// フェーディングスレッド生成
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::BeginFadingThread( void )
{
	m_pMutexAll->Lock() ;
	if ( m_pFadingThread == NULL )
	{
		m_pFadingThread =
			SThread::BeginStockThread
				( &SGLAudioPlayer::FadingThreadProc, NULL ) ;
	}
	else
	{
		m_flagReqFadingThread = true ;
	}
	m_pMutexAll->Unlock() ;
}

// フェーディングスレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::FadingThreadProc( void * pInstance )
{
	SReferenceArray<SGLAudioPlayer>	aAudioPlayers ;
	STimeCounter	timer ;
	uint64_t		msecLast = timer.GetTime() ;
	for ( ; ; )
	{
		SleepMilliSec( 30 ) ;
		//
		uint64_t	msecCur = timer.GetTime() ;
		uint32_t	msecPast = (uint32_t) (msecCur - msecLast) ;
		msecLast = msecCur ;
		//
		aAudioPlayers.RemoveAll() ;
		//
		m_pMutexAll->Lock() ;
		SGLAudioPlayer *	pNextAudio = m_pFirstAudioPlayer ;
		while ( pNextAudio != NULL )
		{
			aAudioPlayers.Add( pNextAudio ) ;
			pNextAudio = pNextAudio->m_pNextAudio ;
		}
		//
		SReferenceArray<SGLAudioPlayer>::Iterator	iter( &aAudioPlayers ) ;
		bool				fFadingVol = false ;
		while ( iter.HasNext() )
		{
			SGLAudioPlayer *	pNextAudio = iter.Next() ;
			if ( (pNextAudio != nullptr)
				&& pNextAudio->OnFadingVolume( msecPast ) )
			{
				fFadingVol = true ;
			}
		}
		//
		if ( !fFadingVol && !m_flagReqFadingThread )
		{
			m_pFadingThread = NULL ;
			m_pMutexAll->Unlock() ;
			break ;
		}
		m_flagReqFadingThread = false ;
		m_pMutexAll->Unlock() ;
	}
}

// 音量フェード処理
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioPlayer::OnFadingVolume( uint32_t msecPast )
{
	SSmartLock<SCriticalSection>	lock( &m_mutex ) ;
	if ( !IsValidOnAudioChain() )
	{
		return	false ;
	}
	if ( (m_msecFadingDuration > 0)
		&& (m_volFadeStart.GetLength() > 0)
		&& (m_volFadeStart.GetLength() == m_volFadeEnd.GetLength()) )
	{
		m_msecFadingTime += msecPast ;
		if ( m_msecFadingTime < m_msecFadingDuration )
		{
			SArray<float32_t>	vol ;
			double	t = (double) m_msecFadingTime / m_msecFadingDuration ;
			size_t	ch = m_volFadeStart.GetLength() ;
			vol.SetLength( ch ) ;
			for ( size_t i = 0; i < ch; i ++ )
			{
				float32_t	v0 = m_volFadeStart.At(i) ;
				float32_t	v1 = m_volFadeEnd.At(i) ;
				vol.SetAt( i, (float32_t) ((v1 - v0) * t + v0) ) ;
			}
			SetVolume( vol.GetConstArray(), ch ) ;
			return	true ;
		}
		else
		{
			FlushFadeVolume() ;
		}
	}
	return	false ;
}

// 再生中オーディオ・チェーンに追加
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::AddToAudioChain( void )
{
	m_pMutexAll->Lock() ;
	if ( (m_pFirstAudioPlayer != this)
		& (m_pPrevAudio == NULL) & (m_pNextAudio == NULL) )
	{
		m_pNextAudio = m_pFirstAudioPlayer ;
		m_pFirstAudioPlayer = this ;
		if ( m_pNextAudio != NULL )
		{
			m_pNextAudio->m_pPrevAudio = this ;
		}
	}
	if ( IsVolumeFading() )
	{
		BeginFadingThread() ;
	}
	m_pMutexAll->Unlock() ;
}

// 再生中オーディオ・チェーンから分離
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::DetachFromAudioChain( void )
{
	m_pMutexAll->Lock() ;
	if ( (m_pFirstAudioPlayer == this)
		| (m_pPrevAudio != NULL) | (m_pNextAudio != NULL) )
	{
		SGLAudioPlayer *	pPrevAudio = m_pPrevAudio ;
		SGLAudioPlayer *	pNextAudio = m_pNextAudio ;
		if ( pPrevAudio != NULL )
		{
			pPrevAudio->m_pNextAudio = pNextAudio ;
		}
		else
		{
			m_pFirstAudioPlayer = pNextAudio ;
		}
		if ( pNextAudio != NULL )
		{
			pNextAudio->m_pPrevAudio = pPrevAudio ;
		}
		m_pPrevAudio = NULL ;
		m_pNextAudio = NULL ;
	}
	m_pMutexAll->Unlock() ;
}

// 再生中オーディオチェーンに存在するか？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioPlayer::IsValidOnAudioChain( void ) const
{
	m_pMutexAll->Lock() ;
	if ( (m_pFirstAudioPlayer == this)
		| (m_pPrevAudio != NULL) | (m_pNextAudio != NULL) )
	{
		m_pMutexAll->Unlock() ;
		return	true ;
	}
	m_pMutexAll->Unlock() ;
	return	false ;
}

// 全再生中のオーディオ音量を反映
//////////////////////////////////////////////////////////////////////////////
void SGLAudioPlayer::ReflectVolumesAllChain( void )
{
	SReferenceArray<SGLAudioPlayer>	aAudioPlayers ;
	m_pMutexAll->Lock() ;
	SGLAudioPlayer *	pNextAudio = m_pFirstAudioPlayer ;
	while ( pNextAudio != NULL )
	{
		aAudioPlayers.Add( pNextAudio ) ;
		pNextAudio = pNextAudio->m_pNextAudio ;
	}
	//
	SReferenceArray<SGLAudioPlayer>::Iterator	iter( &aAudioPlayers ) ;
	bool	fFadingVol = false ;
	while ( iter.HasNext() )
	{
		SGLAudioPlayer *	pAudio = iter.Next() ;
		if ( pAudio != nullptr )
		{
			if ( pAudio->IsPlaying() )
			{
				pAudio->ReflectVolume() ;
			}
			else
			{
				pAudio->DetachFromAudioChain() ;
			}
		}
	}
	m_pMutexAll->Unlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// オーディオデータバッファ・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioBufferReader, ESLObject )

