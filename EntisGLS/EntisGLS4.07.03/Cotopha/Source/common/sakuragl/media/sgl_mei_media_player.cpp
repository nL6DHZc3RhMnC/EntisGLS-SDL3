
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mei_media_player.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// サウンドストリーム用バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
( SakuraGL::SGLMEIMediaPlayer::SoundStreamBuffer, SGLSoundPlayerListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaPlayer::SoundStreamBuffer::SoundStreamBuffer( void )
{
	m_pMutexUI = SSystem::g_mutexGlobal ;
}

// 追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLMEIMediaPlayer::SoundStreamBuffer::Add( SByteBuffer * pBuf )
{
	return	SObjectArray<SByteBuffer>::Add( pBuf ) ;
}

// 取得
//////////////////////////////////////////////////////////////////////////////
SByteBuffer * SGLMEIMediaPlayer::SoundStreamBuffer::GetAt( size_t nIndex ) const
{
	return	SObjectArray<SByteBuffer>::GetAt( nIndex ) ;
}

// 要素削除
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::SoundStreamBuffer::RemoveAt( size_t nIndex )
{
	SObjectArray<SByteBuffer>::RemoveAt( nIndex ) ;
}

// 全要素削除
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::SoundStreamBuffer::RemoveAll( void )
{
	SObjectArray<SByteBuffer>::RemoveAll() ;
}

// バッファへの出力タイミング
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::SoundStreamBuffer::OnStreaming( SoundPlayer * player )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	while ( GetLength() > 0 )
	{
		SByteBuffer *	pBuf = GetAt(0) ;
		if ( pBuf != NULL )
		{
			size_t	pos = (size_t) pBuf->GetPosition() ;
			if ( pos < (size_t) pBuf->GetLength() )
			{
				pos += player->Write
					( pBuf->GetConstArray() + pos,
						(size_t) pBuf->GetLength() - pos ) ;
				pBuf->Seek( pos ) ;
				if ( pos < (size_t) pBuf->GetLength() )
				{
					m_pMutexUI->Unlock() ;
					return ;
				}
			}
		}
		RemoveAt( 0 ) ;
	}
	m_pMutexUI->Unlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// MEI ファイル再生インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLMEIMediaPlayer,
		SGLMediaPlayerInterface, SGLMovieFilePlayer, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaPlayer::SGLMEIMediaPlayer( void )
{
	m_status = statusNothing ;
	//
	m_pWindow = NULL ;
	m_nViewFlags = 0 ;
	//
	m_flagThreading = false ;
	m_nPlayFlags = 0 ;
	//
	m_flagLoop = false ;
	//
	m_pListener = NULL ;
	//
	m_pMutexUI = SSystem::g_mutexGlobal ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaPlayer::~SGLMEIMediaPlayer( void )
{
	SGLMEIMediaPlayer::Close() ;
}

// 再生スレッド開始
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::BeginPlayerThread( void )
{
	if ( !m_flagThreading )
	{
		m_eventQuit.Initialize( false ) ;
		m_threadPlayer.BeginThread( this ) ;
		m_flagThreading = true ;
	}
}

// 再生スレッド終了
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::EndPlayerThread( void )
{
	if ( m_flagThreading )
	{
//		atomic_int_t	nRelock = SSystem::UnlockAll() ;
//		ESLAssert( nRelock == 0 ) ;
		//
		m_eventQuit.SetSignal() ;
		m_threadPlayer.Wait() ;
		m_eventQuit.Delete() ;
		m_threadPlayer.Delete() ;
		m_flagThreading = false ;
		//
//		SSystem::Relock( nRelock ) ;
	}
}

// SGLMEIMediaInputStream 取得
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaInputStream * SGLMEIMediaPlayer::GetMEIMediaStream( void )
{
	if ( (m_pMediaStream == NULL) && (m_refFile != NULL) )
	{
		SFileInterface *	pFile = m_refFile->Duplicate() ;
		if ( pFile != NULL )
		{
			m_pMediaStream = new SGLMEIMediaInputStream ;
			pFile->Seek( 0 ) ;
			if ( m_pMediaStream->Open( pFile, true ) )
			{
				m_pMediaStream->Close() ;
				m_pMediaStream = NULL ;
			}
		}
	}
	return	m_pMediaStream ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
{
	SFileInterface *	pFile = NULL ;
	if ( pEnv != NULL )
	{
		pFile = pEnv->NewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	}
	else
	{
		pFile = SFileOpener::DefaultNewOpenFile
						( pwszFilePath, SFileOpener::shareRead ) ;
	}
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	SGLMEIMediaPlayer::Create( pFile, true, nFlags ) ;
}

SGLError SGLMEIMediaPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	Close() ;
	//
	if ( SGLMovieFilePlayer::OpenMovieFile( file, flagOwner ) )
	{
		return	sglErrFailed ;
	}
	m_refFile = file ;
	m_status = statusOpened ;
	//
	return	sglErrSuccess ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLMEIMediaPlayer::ClonePlayer( void )
{
	SGLMEIMediaPlayer *	pPlayer = new SGLMEIMediaPlayer ;
	if ( m_refFile != NULL )
	{
		SFileInterface *	pFile = m_refFile->Duplicate() ;
		if ( pFile != NULL )
		{
			pFile->Seek( 0 ) ;
			pPlayer->Create( pFile, true ) ;
		}
	}
	pPlayer->SetUIThreadMutex( m_pMutexUI ) ;
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Close( void )
{
	if ( m_status >= statusPlayed )
	{
		Stop() ;
	}
	if ( m_status >= statusOpened )
	{
		SGLMovieFilePlayer::Close() ;
		m_status = statusNothing ;
	}
	m_refFile = NULL ;
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Play( uint64_t nFlags )
{
	if ( m_status != statusOpened )
	{
		return	sglErrFailed ;
	}
	m_nPlayFlags = nFlags ;
	m_status = statusPlayed ;
	BeginPlayerThread() ;
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Stop( void )
{
	if ( m_status < statusPlayed )
	{
		return	sglErrFailed ;
	}
	EndPlayerThread() ;
	m_status = statusOpened ;
	return	sglErrSuccess ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	m_csSync.Lock() ;
	m_flagLoop = fLoop ;
	m_nLoopStart = nStart ;
	m_nLoopEnd = nEnd ;
	m_csSync.Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Pause( void )
{
	if ( m_status != statusPlayed )
	{
		return	sglErrFailed ;
	}
	EndPlayerThread() ;
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::Restart( void )
{
	if ( m_status != statusPaused )
	{
		return	sglErrFailed ;
	}
	m_status = statusPlayed ;
	BeginPlayerThread() ;
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	return	m_sndPlayer.GetVolume( pVolumes, nChannels ) ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	return	m_sndPlayer.SetVolume( pVolumes, nChannels ) ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLMEIMediaPlayer::IsPlaying( void ) const
{
	if ( m_status >= statusPlayed )
	{
		if ( m_status == statusPaused )
		{
			return	true ;
		}
		return	(((SThread*)&m_threadPlayer)->Wait(0) == errTimeout) ;
	}
	return	false ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLMEIMediaPlayer::IsPaused( void ) const
{
	return	(m_status == statusPaused) ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLMEIMediaPlayer::GetSampleFrequency( void ) const
{
	return	1000 ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMEIMediaPlayer::GetTotalLength( void ) const
{
	return	SGLMovieFilePlayer::GetTotalTime() ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMEIMediaPlayer::GetPosition( void )
{
	return	SGLMovieFilePlayer::FrameIndexToTime
					( SGLMovieFilePlayer::CurrentIndex() ) ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_status == statusPlayed )
	{
		EndPlayerThread() ;
		m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
		m_csSync.Lock() ;
		SeekToFrame( nPos ) ;
		m_csSync.Unlock() ;
		m_pMutexUI->Unlock() ;
		BeginPlayerThread() ;
	}
	else
	{
		m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
		m_csSync.Lock() ;
		SeekToFrame( nPos ) ;
		m_csSync.Unlock() ;
		m_pMutexUI->Unlock() ;
	}
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLMEIMediaPlayer::GetAudioStream( void )
{
	return	GetMEIMediaStream() ;
}

void SGLMEIMediaPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	m_pMutexUI = pMutex ;
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::GetVideoSize( SGLSize& sizeVideo )
{
	SGLError			err = sglErrSuccess ;
	SGLImageObject *	pImage ;
	m_csSync.Lock() ;
	pImage = CurrentFrame() ;
	if ( pImage != NULL )
	{
		SGLImageInfo	imginf ;
		pImage->GetImageInfo( imginf ) ;
		//
		sizeVideo.w = imginf.width ;
		sizeVideo.h = imginf.height ;
	}
	else
	{
		err = sglErrFailed ;
	}
	m_csSync.Unlock() ;
	return	err ;
}

// 表示先を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::SetVideoView
	( SGLAbstractWindow* pWindow,
		const SGLImageRect& rectVideo, uint64_t nFlags )
{
	m_pWindow = pWindow ;
	m_rectDstView = rectVideo ;
	m_nViewFlags = nFlags ;
	return	sglErrSuccess ;
}

// 現在のフレームを描画する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaPlayer::DrawVideo
	( SGLPaintContextInterface* pPaint,
		const SGLImageRect& rectDst, uint32_t nFlags, uint32_t nTransparency )
{
	SGLError			err = sglErrFailed ;
	SGLImageObject *	pImage ;
	m_csSync.Lock() ;
	pImage = CurrentFrame() ;
	if ( pImage != NULL )
	{
		SGLImageInfo	imginf ;
		pImage->GetImageInfo( imginf ) ;
		//
		SGLPaintParam	ppPaint ;
		SGLAffine		affine ;
		ppPaint.nFlags = nFlags ;
		ppPaint.nTransparency = nTransparency ;
		ppPaint.SetAffine
			( affine, rectDst.x, rectDst.y, 0, 0,
				(double) rectDst.w / imginf.width,
				(double) rectDst.h / imginf.height ) ;
		//
		err = pPaint->DrawImage( ppPaint, pImage ) ;
	}
	m_csSync.Unlock() ;
	return	err ;
}

// メディア再生通知リスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerFrameNotification *
	SGLMEIMediaPlayer::SetNotificationListener
		( SGLMediaPlayerFrameNotification * pListener )
{
	SGLMediaPlayerFrameNotification *	pLast ;
	m_csSync.Lock() ;
	pLast = m_pListener ;
	m_pListener = pListener ;
	m_csSync.Unlock() ;
	return	pLast ;
}

// ビデオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLVideoInputStream * SGLMEIMediaPlayer::GetVideoStream( void )
{
	return	GetMEIMediaStream() ;
}

void SGLMEIMediaPlayer::ReleaseVideoStream( SGLVideoInputStream * pStream )
{
}

// 音声出力要求
//////////////////////////////////////////////////////////////////////////////
bool SGLMEIMediaPlayer::RequestWaveOut
	( uint32_t channels, uint32_t frequency, uint32_t bps )
{
	SGLSoundFormat	fmt ;
	fmt.format = formatSoundLinearPCM ;
	fmt.frequency = frequency ;
	fmt.channels = channels ;
	fmt.bitsPerSample = bps ;
	//
	if ( m_sndPlayer.Open( fmt ) )
	{
		return	false ;
	}
	if ( m_sndPlayer.PrepareStream() )
	{
		return	false ;
	}
	m_sndPlayer.SetListener( &m_sndListener ) ;
	return	true ;
}

// 音声出力終了
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::CloseWaveOut( void )
{
	m_sndPlayer.Close() ;
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_sndListener.RemoveAll() ;
	m_pMutexUI->Unlock() ;
}

// 音声データ出力
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::PushWaveBuffer( const void * ptrWaveBuf, size_t nBytes )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	//
	// バッファ追加
	//
	SByteBuffer *	pBuf = new SByteBuffer ;
	pBuf->SetLength( nBytes ) ;
	pBuf->Write( ptrWaveBuf, nBytes ) ;
	pBuf->Seek( 0 ) ;
	m_sndListener.Add( pBuf ) ;
	//
	// 溜まっているデータを書き出す
	//
	if ( m_flagWaveStreaming )
	{
		while ( m_sndListener.GetLength() > 0 )
		{
			SByteBuffer *	pBuf = m_sndListener.GetAt(0) ;
			if ( pBuf != NULL )
			{
				size_t	pos = (size_t) pBuf->GetPosition() ;
				if ( pos < (size_t) pBuf->GetLength() )
				{
					pos += m_sndPlayer.Write
						( pBuf->GetConstArray() + pos,
							(size_t) pBuf->GetLength() - pos ) ;
					pBuf->Seek( pos ) ;
					if ( pos < (size_t) pBuf->GetLength() )
					{
						m_pMutexUI->Unlock() ;
						return ;
					}
				}
			}
			m_sndListener.RemoveAt( 0 ) ;
		}
	}
	m_pMutexUI->Unlock() ;
}

// 音声ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::BeginWaveStreaming( void )
{
	if ( !SystemLock() )
	{
		return ;
	}
	m_csSync.Lock() ;
	m_sndListener.RemoveAll() ;
	m_csSync.Unlock() ;
	m_pMutexUI->Unlock() ;
	//
	m_sndPlayer.Play() ;
	//
	SGLMovieFilePlayer::BeginWaveStreaming() ;
}

// 音声ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::EndWaveStreaming( void )
{
	SGLMovieFilePlayer::EndWaveStreaming() ;
	m_sndPlayer.Stop() ;
	//
	if ( !SystemLock() )
	{
		return ;
	}
	m_csSync.Lock() ;
	m_sndListener.RemoveAll() ;
	m_csSync.Unlock() ;
	m_pMutexUI->Unlock() ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaPlayer::Run( void )
{
	BeginWaveStreaming() ;

	uint64_t	msecStart ;
	msecStart = CurrentMilliSec() - FrameIndexToTime( CurrentIndex() ) ;

	size_t nSkipFrame = 0 ;
	for ( ; ; )
	{
		//
		// ループ判定と次のフレームへ移動
		//
		uint64_t	msecCurrent, msecFrame, msecWait ;
		bool		flagEnd = false, flagLoop = false ;
		if ( !SystemLock() )
		{
			break ;
		}
		m_csSync.Lock() ;
		//
		if ( CurrentIndex() + nSkipFrame + 1 >= GetAllFrameCount() )
		{
			nSkipFrame = (size_t) (GetAllFrameCount() - CurrentIndex() - 2) ;
			flagEnd = true ;
		}
		if ( m_flagLoop )
		{
			if ( (m_nLoopEnd >= 0)
				&& ((CurrentIndex() + nSkipFrame + 1)
									>= (uint64_t) m_nLoopEnd) )
			{
				flagEnd = true ;
			}
			if ( flagEnd )
			{
				flagLoop = true ;
			}
		}
		if ( flagLoop )
		{
			if ( m_pListener != NULL )
			{
				m_pListener->OnEndOfDuration( this ) ;
			}
			EndWaveStreaming() ;
			if ( m_nLoopStart <= 0 )
			{
				SeekToBegin() ;
			}
			else
			{
				SeekToFrame( m_nLoopStart ) ;
			}
			BeginWaveStreaming() ;
			msecStart = CurrentMilliSec()
							- FrameIndexToTime( CurrentIndex() ) ;
		}
		else
		{
			if ( (ssize_t) nSkipFrame >= 0 )
			{
				SeekToNextFrame( nSkipFrame ) ;
			}
			if ( flagEnd )
			{
				if ( m_pListener != NULL )
				{
					m_pListener->OnEndOfDuration( this ) ;
				}
				m_csSync.Unlock() ;
				m_pMutexUI->Unlock() ;
				break ;
			}
		}
		//
		msecCurrent = CurrentMilliSec() - msecStart ;
		msecFrame = FrameIndexToTime( CurrentIndex() ) ;
		nSkipFrame = GetBestSkipFrames( msecCurrent ) ;
		m_csSync.Unlock() ;
		m_pMutexUI->Unlock() ;
		//
		// フレーム時間同期
		//
		msecWait = 1 ;
		if ( msecFrame > msecCurrent )
		{
			msecWait = msecFrame - msecCurrent ;
		}
		if ( msecWait > 1 )
		{
			msecWait -- ;
		}
		if ( msecWait == 0 )
		{
			msecWait = 1 ;
		}
		if ( m_eventQuit.Wait( msecWait ) == errSuccess )
		{
			break ;
		}
		//
		// 画面へ描画
		//
		if ( m_pListener != NULL )
		{
			if ( !SystemLock() )
			{
				break ;
			}
			m_pListener->OnFrameUpdate( this ) ;
			m_pMutexUI->Unlock() ;
		}
		if ( m_pWindow != NULL )
		{
			if ( m_nViewFlags & (flagPostUpdate | flagUpdateWindow) )
			{
				if ( m_nViewFlags & flagPostUpdate )
				{
					if ( !SystemLock() )
					{
						break ;
					}
					m_pWindow->PostUpdate() ;
					m_pMutexUI->Unlock() ;
				}
				if ( m_nViewFlags & flagUpdateWindow )
				{
					m_pWindow->UpdateWindow() ;
				}
			}
			else
			{
				S3DRenderContextInterface *	render = NULL ;
				if ( !SystemLock() )
				{
					break ;
				}
				render = m_pWindow->GetRenderContext() ;
				if ( render != NULL )
				{
					DrawVideo( render, m_rectDstView ) ;
					m_pWindow->ReleaseRenderContext( render ) ;
				}
				m_pMutexUI->Unlock() ;
			}
		}
	}

	EndWaveStreaming() ;
}

// m_eventQuit 脱出判定つき Lock
//////////////////////////////////////////////////////////////////////////////
bool SGLMEIMediaPlayer::SystemLock( void )
{
	while ( m_pMutexUI->LockTrace( __FILE__, __LINE__, 10 ) != errSuccess )
	{
		if ( m_eventQuit.Wait( 0 ) == errSuccess )
		{
			return	false ;
		}
	}
	return	true ;
}


