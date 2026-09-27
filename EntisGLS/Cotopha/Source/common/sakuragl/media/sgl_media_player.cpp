
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mei_media_player.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_dshow_audio_player.h>
#include <sakuragl/sgl_dshow_media_player.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_media_player.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// メディアファイル再生インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLMediaPlayerInterface, SGLAudioPlayerInterface )


//////////////////////////////////////////////////////////////////////////
// メディアファイル再生通知リスナ
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLMediaPlayerFrameNotification )

// フレーム更新通知
#if	defined(__COTOPHA__)
void SGLMediaPlayerFrameNotification::OnFrameUpdateCaller( MediaPlayer * player )
{
	SGLMediaPlayer	mp( player, false ) ;
	OnFrameUpdate( &mp ) ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// メディアファイル再生ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST
	( SakuraGL::SGLMediaPlayer, SGLMediaPlayerInterface, (&m_audio) )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayer::~SGLMediaPlayer( void )
{
	if ( m_flagOwner )
	{
		delete	m_pMedia ;
	}
	m_pMedia = NULL ;
	m_flagOwner = false ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
MediaPlayer * SGLMediaPlayer::SetMediaPlayer
		( MediaPlayer * pPlayer, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pMedia ;
	}
	m_pMedia = pPlayer ;
	m_flagOwner = flagOwner ;
	//
	m_audio.SetAudioPlayer( pPlayer, false ) ;
	//
	return	m_pMedia ;
}

// 音量ライン設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaPlayer::SetAudioVolumeLine( size_t iLine )
{
	m_iAudioLine = iLine ;
}

// 音量ライン取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMediaPlayer::GetAudioVolumeLine( void ) const
{
	return	m_iAudioLine ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
{
	if ( m_pMedia == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pMedia = new MediaPlayer ;
		#else
			m_pMedia = new SGLMEIMediaPlayer ;
		#endif
		m_flagOwner = true ;
	}
	m_audio.SetAudioPlayer( NULL, false ) ;

	SGLError	err ;
	#if	defined(__COTOPHA__)
		err = m_pMedia->Open( pwszFilePath, nFlags ) ;
	#else
		err = m_pMedia->Open( pwszFilePath, nFlags, pEnv ) ;
	#endif
	if ( !err )
	{
		m_audio.SetAudioPlayer( m_pMedia, false ) ;
		return	sglErrSuccess ;
	}
	//
	#if	defined(__PLATFORM_WINDOWS__)
		if ( m_flagOwner )
		{
			delete	m_pMedia ;
			//
			m_pMedia = (SGLMediaPlayerInterface*) new SGLDSRenderMediaPlayer ;
			m_flagOwner = true ;
			if ( !m_pMedia->Open( pwszFilePath, nFlags, pEnv ) )
			{
				m_audio.SetAudioPlayer( m_pMedia, false ) ;
				return	sglErrSuccess ;
			}
			delete	m_pMedia ;
			m_pMedia = NULL ;
		}
	#elif	defined(__PLATFORM_ANDROID__)	
		if ( m_flagOwner )
		{
			delete	m_pMedia ;
			//
			m_pMedia = (SGLMediaPlayerInterface*) new SGLAndroidMediaPlayer ;
			m_flagOwner = true ;
			if ( !m_pMedia->Open( pwszFilePath, nFlags, pEnv ) )
			{
				m_audio.SetAudioPlayer( m_pMedia, false ) ;
				return	sglErrSuccess ;
			}
			delete	m_pMedia ;
			m_pMedia = NULL ;
		}
	#endif
	return	err ;
}

SGLError SGLMediaPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	if ( m_pMedia == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pMedia = new MediaPlayer ;
		#else
			m_pMedia = new SGLMEIMediaPlayer ;
		#endif
		m_flagOwner = true ;
	}
	m_audio.SetAudioPlayer( NULL, false ) ;

	#if	defined(__COTOPHA__)
		File *	pFile = file->GetFileObject() ;
		if ( pFile == NULL )
		{
			return	sglErrFailed ;
		}
		SGLError	err = m_pMedia->Create( pFile, nFlags ) ;
		if ( m_flagFileOwner )
		{
			delete	m_pFile ;
		}
		m_pFile = file ;
		m_flagFileOwner = flagOwner ;
		m_audio.SetAudioPlayer( m_pMedia, false ) ;
		return	err ;
	#else
		SGLError	err ;
		int64_t	pos = file->GetPosition() ;
		err = m_pMedia->Create( file, flagOwner, nFlags ) ;
		if ( !err )
		{
			m_audio.SetAudioPlayer( m_pMedia, false ) ;
			return	sglErrSuccess ;
		}
		#if	defined(__PLATFORM_WINDOWS__)
		if ( m_flagOwner )
		{
			delete	m_pMedia ;
			//
			m_pMedia = (SGLMediaPlayerInterface*) new SGLDSRenderMediaPlayer ;
			m_flagOwner = true ;
			file->Seek( pos ) ;
			if ( !m_pMedia->Create( file, flagOwner, nFlags ) )
			{
				m_audio.SetAudioPlayer( m_pMedia, false ) ;
				return	sglErrSuccess ;
			}
			delete	m_pMedia ;
			m_pMedia = NULL ;
		}
		#endif
		return	err ;
	#endif
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLMediaPlayer::ClonePlayer( void )
{
	if ( m_pMedia == NULL )
	{
		return	new SGLMediaPlayer ;
	}
	SGLMediaPlayer *
		pMedia = new SGLMediaPlayer
			( (MediaPlayer*) m_pMedia->ClonePlayer(), true ) ;
	pMedia->m_iAudioLine = m_iAudioLine ;
	return	pMedia ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::Close( void )
{
	m_audio.SetAudioPlayer( NULL, false ) ;
	//
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = m_pMedia->Close() ;

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
SGLError SGLMediaPlayer::Play( uint64_t nFlags )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	m_audio.SetAudioPlayer( m_pMedia, false ) ;
	m_audio.SetVolumeLineMask( 1 << m_iAudioLine ) ;
	m_audio.ReflectVolume() ;
	m_audio.AddToAudioChain() ;
	return	m_pMedia->Play( nFlags ) ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::Stop( void )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	m_audio.DetachFromAudioChain() ;
	return	m_pMedia->Stop() ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pMedia->SetLoop( fLoop, nStart, nEnd ) ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::Pause( void )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pMedia->Pause() ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::Restart( void )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pMedia->Restart() ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_audio.GetVolume( pVolumes, nChannels ) ;
//	return	m_pMedia->GetVolume( pVolumes, nChannels ) ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_audio.SetVolume( pVolumes, nChannels ) ;
//	return	m_pMedia->SetVolume( pVolumes, nChannels ) ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaPlayer::IsPlaying( void ) const
{
	if ( m_pMedia == NULL )
	{
		return	false ;
	}
	return	m_pMedia->IsPlaying() ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaPlayer::IsPaused( void ) const
{
	if ( m_pMedia == NULL )
	{
		return	false ;
	}
	return	m_pMedia->IsPaused() ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLMediaPlayer::GetSampleFrequency( void ) const
{
	if ( m_pMedia == NULL )
	{
		return	0 ;
	}
	return	m_pMedia->GetSampleFrequency() ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMediaPlayer::GetTotalLength( void ) const
{
	if ( m_pMedia == NULL )
	{
		return	0 ;
	}
	return	m_pMedia->GetTotalLength() ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMediaPlayer::GetPosition( void )
{
	if ( m_pMedia == NULL )
	{
		return	0 ;
	}
	return	m_pMedia->GetPosition() ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLMediaPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_pMedia == NULL )
	{
		return ;
	}
	m_pMedia->SeekPosition( nPos ) ;
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLMediaPlayer::GetAudioStream( void )
{
	if ( m_pMedia == NULL )
	{
		return	NULL ;
	}
	return	m_pMedia->GetAudioStream( ) ;
}

void SGLMediaPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
	if ( m_pMedia != NULL )
	{
		m_pMedia->ReleaseAudioStream( pStream ) ;
	}
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	if ( m_pMedia != NULL )
	{
		m_pMedia->SetUIThreadMutex( pMutex ) ;
	}
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::GetVideoSize( SGLSize& sizeVideo )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pMedia->GetVideoSize( sizeVideo ) ;
}

// 表示先を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::SetVideoView
	( SGLAbstractWindow* pWindow,
		const SGLImageRect& rectVideo, uint64_t nFlags )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrSuccess ;
	}
	#if	defined(__COTOPHA__)
		Window *	pWinObj = NULL ;
		if ( pWindow != NULL )
		{
			pWinObj = pWindow->GetWindowObject() ;
		}
		return	m_pMedia->SetVideoView( pWinObj, rectVideo, nFlags ) ;
	#else
		return	m_pMedia->SetVideoView( pWindow, rectVideo, nFlags ) ;
	#endif
}

// 現在のフレームを描画する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaPlayer::DrawVideo
	( SGLPaintContextInterface* pPaint,
		const SGLImageRect& rectDst, uint32_t nFlags, uint32_t nTransparency )
{
	if ( m_pMedia == NULL )
	{
		return	sglErrFailed ;
	}
	#if	defined(__COTOPHA__)
		PaintContext *	pContext = pPaint->GetPaintContextObject() ;
		if ( pContext == NULL )
		{
			return	sglErrFailed ;
		}
		return	m_pMedia->DrawVideo( pContext, rectDst, nFlags, nTransparency ) ;
	#else
		return	m_pMedia->DrawVideo( pPaint, rectDst, nFlags, nTransparency ) ;
	#endif
}

// メディア再生通知リスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerFrameNotification *
	SGLMediaPlayer::SetNotificationListener
		( SGLMediaPlayerFrameNotification * pListener )
{
	if ( m_pMedia == NULL )
	{
		return	NULL ;
	}
	return	m_pMedia->SetNotificationListener( pListener ) ;
}

// ビデオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLVideoInputStream * SGLMediaPlayer::GetVideoStream( void )
{
	if ( m_pMedia == NULL )
	{
		return	NULL ;
	}
	return	m_pMedia->GetVideoStream() ;
}

void SGLMediaPlayer::ReleaseVideoStream( SGLVideoInputStream * pStream )
{
	if ( m_pMedia != NULL )
	{
		m_pMedia->ReleaseVideoStream( pStream ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// ビデオキャプチャー・インターフェース
//////////////////////////////////////////////////////////////////////////////

// SGLVideoCaptureInterface::DeviceInfo
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLVideoCaptureInterface::DeviceInfo, ESLObject )

SGLVideoCaptureInterface::DeviceInfo::DeviceInfo( void )
{
}

SGLVideoCaptureInterface::DeviceInfo::~DeviceInfo( void )
{
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVideoCaptureInterface, ESLObject )

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerInterface * SGLVideoCaptureInterface::CreatePlayer( void )
{
	#if	defined(__PLATFORM_WINDOWS__)
		return	new SGLMediaPlayer( new SGLDSVideoCapturePlayer, true ) ;
	#else
		return	NULL ;
	#endif
}

