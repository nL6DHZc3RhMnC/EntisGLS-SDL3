
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <dsound.h>
#include <sakuragl/sgl_direct_sound_player.h>
#include <dshow.h>
#include <sakuragl/sgl_dshow_audio_player.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// DirectShow オーディオファイル再生インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
( SakuraGL::SGLDirectShowAudioPlayer, SGLAudioPlayerInterface, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectShowAudioPlayer::SGLDirectShowAudioPlayer( void )
{
	m_flagPlayed = false ;
	m_flagPaused = false ;
	m_flagLoop = false ;
	m_msLoopStart = 0 ;
	m_msLoopEnd = 0 ;
	//
	m_flagThreading = false ;
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	m_pGraphBuilder = NULL ;
	m_pMediaControl = NULL ;
	m_pVideoWindow = NULL ;
	m_pBasicAudio = NULL ;
	m_pMediaPosition = NULL ;
	m_pMediaEvent = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectShowAudioPlayer::~SGLDirectShowAudioPlayer( void )
{
	Close() ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Open
	( const wchar_t * pwszFilePath, uint64_t nFlags,
					SSystem::SEnvironmentInterface * pEnv )
{
	Close() ;
	//
	// DirectShow オブジェクト生成
	//
	CoCreateInstance
		( CLSID_FilterGraph, NULL, CLSCTX_INPROC,
				IID_IGraphBuilder, (void **) &m_pGraphBuilder ) ;
	if ( m_pGraphBuilder == NULL )
	{
		return	sglErrFailed ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IMediaControl, (void **) &m_pMediaControl ) ;
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IVideoWindow, (void **) &m_pVideoWindow ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicAudio, (void **) &m_pBasicAudio ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaPosition, (void **) &m_pMediaPosition ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaEvent, (void **) &m_pMediaEvent ) ;
	//
	// ファイルを開く
	//
	if ( SFileOpener::DefaultDirectPathOf( m_strFilePath, pwszFilePath ) )
	{
		SSmartPointer<SFileInterface>	pfile ;
		if ( pEnv != NULL )
		{
			pfile = pEnv->NewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
		}
		else
		{
			pfile = SFileOpener::DefaultNewOpenFile
							( pwszFilePath, SFileOpener::shareRead ) ;
		}
		SFile *	pFile = ESLTypeCast<SFile>( pfile.Ptr() ) ;
		if ( pFile != NULL )
		{
			m_strFilePath = pFile->GetFilePath() ;
			pfile = NULL ;
		}
		else
		{
			Close() ;
			return	sglErrFailed ;
		}
	}
	HRESULT	hr = m_pGraphBuilder->RenderFile( m_strFilePath, NULL ) ;
	if ( hr )
	{
		Close() ;
		return	sglErrFailed ;
	}
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->put_AutoShow( OAFALSE ) ;
		m_pVideoWindow->put_Visible( OAFALSE ) ;
	}
	return	sglErrSuccess ;
}

SGLError SGLDirectShowAudioPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	SFile *	pFile = ESLTypeCast<SFile>( file ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	SString		strFilePath = pFile->GetFilePath() ;
	SGLError	err = Open( strFilePath, nFlags ) ;
	if ( !err && flagOwner )
	{
		delete	file ;
	}
	return	err ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLDirectShowAudioPlayer::ClonePlayer( void )
{
	SGLDirectShowAudioPlayer *	pPlayer = new SGLDirectShowAudioPlayer ;
	if ( !m_strFilePath.IsEmpty() )
	{
		pPlayer->Open( m_strFilePath ) ;
	}
	pPlayer->SetUIThreadMutex( m_pMutexUI ) ;
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Close( void )
{
	CloseLoopingThread() ;
	//
	if ( m_pMediaEvent != NULL )
	{
		m_pMediaEvent->Release() ;
		m_pMediaEvent = NULL ;
	}
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->Release() ;
		m_pVideoWindow = NULL ;
	}
	if ( m_pBasicAudio != NULL )
	{
		m_pBasicAudio->Release() ;
		m_pBasicAudio = NULL ;
	}
	if ( m_pMediaPosition != NULL )
	{
		m_pMediaPosition->Release() ;
		m_pMediaPosition = NULL ;
	}
	if ( m_pMediaControl != NULL )
	{
		m_pMediaControl->Release() ;
		m_pMediaControl = NULL ;
	}
	if ( m_pGraphBuilder != NULL )
	{
		m_pGraphBuilder->Release() ;
		m_pGraphBuilder = NULL ;
	}
	m_strFilePath.FreeArray() ;
	//
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Play( uint64_t nFlags )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_flagLoop )
	{
		BeginLoopingThread() ;
	}
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pMediaControl->Run() ;
	m_flagPlayed = true ;
	m_flagPaused = false ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Stop( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pMediaControl->Stop() ;
	m_flagPlayed = false ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagLoop = fLoop ;
	m_msLoopStart = nStart ;
	m_msLoopEnd = nEnd ;
	//
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtDuration ;
		if ( m_pMediaPosition->get_Duration( &rtDuration ) == S_OK )
		{
			REFTIME	rtLoopEnd = rtDuration ;
			if ( m_flagLoop || (m_msLoopEnd > 0) )
			{
				rtLoopEnd = (double) m_msLoopEnd / 1000.0 ;
				if ( rtLoopEnd >= rtDuration )
				{
					rtLoopEnd = rtDuration ;
				}
			}
			m_pMediaPosition->put_StopTime( rtLoopEnd ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	//
	if ( m_flagPlayed && m_flagLoop )
	{
		BeginLoopingThread() ;
	}
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Pause( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pMediaControl->Pause() ;
	m_flagPaused = true ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::Restart( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pMediaControl->Run() ;
	m_flagPaused = false ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::GetVolume
	( float32_t* pVolumes, size_t nChannels )
{
	if ( m_pBasicAudio == NULL )
	{
		return	sglErrFailed ;
	}
	long		lVolume, lBalance ;
	float32_t	volume[2] ;
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pBasicAudio->get_Volume( &lVolume ) != S_OK )
	{
		lVolume = 0 ;
	}
	if ( m_pBasicAudio->get_Balance( &lBalance ) != S_OK )
	{
		lBalance = 0 ;
	}
	m_pMutexUI->Unlock() ;
	//
	SGLDirectSoundPlayer::VolumePanToStereo( &volume[0], lVolume, lBalance ) ;
	if ( nChannels >= 1 )
	{
		pVolumes[0] = volume[0] ;
		if ( nChannels >= 2 )
		{
			pVolumes[1] = volume[1] ;
		}
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowAudioPlayer::SetVolume
	( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_pBasicAudio == NULL )
	{
		return	sglErrFailed ;
	}
	long		lVolume, lBalance ;
	float32_t	volume[2] = { 1.0, 1.0 } ;
	if ( nChannels >= 2 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[1] ;
	}
	else if ( nChannels >= 1 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[0] ;
	}
	SGLDirectSoundPlayer::VolumeStereoToPan( lVolume, lBalance, &volume[0] ) ;
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pBasicAudio->put_Volume( lVolume ) ;
	m_pBasicAudio->put_Balance( lBalance ) ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowAudioPlayer::IsPlaying( void ) const
{
	if ( m_flagPlayed && !m_flagLoop )
	{
		long		lEventCode ;
		LONG_PTR	lParam1, lParam2 ;
		bool		flagStopped = false ;
		m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
		if ( m_pMediaEvent != NULL )
		{
			if ( m_pMediaEvent->GetEvent
					( &lEventCode, &lParam1, &lParam2, 0 ) == S_OK )
			{
				if ( lEventCode == EC_COMPLETE )
				{
					flagStopped = true ;
				}
				m_pMediaEvent->FreeEventParams
						( lEventCode, lParam1, lParam2 ) ;
			}
		}
		m_pMutexUI->Unlock() ;
		//
		if ( flagStopped )
		{
			((SGLDirectShowAudioPlayer*)this)->Stop() ;
			return	false ;
		}
	}
	return	m_flagPlayed ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowAudioPlayer::IsPaused( void ) const
{
	return	m_flagPaused ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDirectShowAudioPlayer::GetSampleFrequency( void ) const
{
	return	1000 ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDirectShowAudioPlayer::GetTotalLength( void ) const
{
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_Duration( &rtTime ) == S_OK )
		{
			return	(uint64_t) (rtTime * 1000.0) ;
		}
	}
	return	0 ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDirectShowAudioPlayer::GetPosition( void )
{
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_CurrentPosition( &rtTime ) == S_OK )
		{
			return	(uint64_t) (rtTime * 1000.0) ;
		}
	}
	return	0 ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_pMediaPosition != NULL )
	{
		m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
		REFTIME	rtTime = (double) nPos / 1000.0 ;
		m_pMediaPosition->put_CurrentPosition( rtTime ) ;
		m_pMutexUI->Unlock() ;
	}
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLDirectShowAudioPlayer::GetAudioStream( void )
{
	return	NULL ;
}

void SGLDirectShowAudioPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	m_pMutexUI = pMutex ;
}

// ループ処理用スレッドを開始する
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::BeginLoopingThread( void )
{
	if ( m_flagThreading )
	{
		if ( m_thread.Wait(0) == errSuccess )
		{
			m_thread.Delete() ;
			m_eventQuit.Delete() ;
			m_flagThreading = false ;
		}
	}
	if ( !m_flagThreading )
	{
		m_eventQuit.Initialize( false ) ;
		m_thread.BeginThread( this ) ;
		m_flagThreading = true ;
	}
}

// ループ処理用スレッドを終了する
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::CloseLoopingThread( void )
{
	if ( m_flagThreading )
	{
		m_eventQuit.SetSignal() ;
		m_thread.Wait() ;
		m_thread.Delete() ;
		m_eventQuit.Delete() ;
		m_flagThreading = false ;
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::Run( void )
{
	while ( m_eventQuit.Wait( 33 ) == errTimeout )
	{
		if ( OnLoopingThread() )
		{
			break ;
		}
	}
}

// ループ処理実装
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowAudioPlayer::OnLoopingThread( void )
{
	long		lEventCode ;
	LONG_PTR	lParam1, lParam2 ;
	bool		flagStopped = false ;
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pMediaEvent != NULL )
	{
		if ( m_pMediaEvent->GetEvent
				( &lEventCode, &lParam1, &lParam2, 0 ) == S_OK )
		{
			if ( lEventCode == EC_COMPLETE )
			{
				flagStopped = true ;
			}
			m_pMediaEvent->FreeEventParams
					( lEventCode, lParam1, lParam2 ) ;
		}
	}
	if ( m_flagLoop && (m_msLoopEnd > 0) )
	{
		if ( (int64_t) GetPosition() >= m_msLoopEnd - 1 )
		{
			flagStopped = true ;
		}
	}
	if ( flagStopped )
	{
		NotifyEndOfPlayingDuration() ;
		//
		if ( m_flagLoop )
		{
			if ( m_msLoopStart < 0 )
			{
				SeekPosition( 0 ) ;
			}
			else
			{
				SeekPosition( m_msLoopStart ) ;
			}
			if ( m_pMediaControl != NULL )
			{
				m_pMediaControl->Run() ;
			}
		}
		else
		{
			m_pMutexUI->Unlock() ;
			Stop() ;
			return	true ;
		}
	}
	m_pMutexUI->Unlock() ;
	return	false ;
}

// 終端到達
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowAudioPlayer::NotifyEndOfPlayingDuration( void )
{
}



