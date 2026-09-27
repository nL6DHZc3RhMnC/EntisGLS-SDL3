
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_android_media_player.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Android MediaPlayer インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAndroidMediaPlayer, SGLMediaPlayerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidMediaPlayer::SGLAndroidMediaPlayer( bool flagMovie )
{
	m_flagMovie = flagMovie ;
	//
	if ( JavaObject::CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/EntisMediaPlayer", "()V" ) != NULL )
	{
		MakeGlobalRef() ;
		//
		m_jmidOpenMovie =
			GetMethodID( "openMovie", "(L" JAVA_LANG_STRING ";)Z" ) ;
		m_jmidOpenMovieOnAssets =
			GetMethodID( "openMovieOnAssets", "(L" JAVA_LANG_STRING ";)Z" ) ;
		m_jmidOpenAudio =
			GetMethodID( "openAudio", "(L" JAVA_LANG_STRING ";)Z" ) ;
		m_jmidOpenAudioOnAssets =
			GetMethodID( "openAudioOnAssets", "(L" JAVA_LANG_STRING ";)Z" ) ;
		m_jmidClose = GetMethodID( "close", "()V" ) ;
		m_jmidPlay = GetMethodID( "play", "()Z" ) ;
		m_jmidStop = GetMethodID( "stop", "()Z" ) ;
		m_jmidPause = GetMethodID( "pause", "()Z" ) ;
		m_jmidRestart = GetMethodID( "restart", "()Z" ) ;
		m_jmidGetVolume = GetMethodID( "getVolume", "([D)V" ) ;
		m_jmidSetVolume = GetMethodID( "setVolume", "(DD)Z" ) ;
		m_jmidGetPlayingPosition = GetMethodID( "getPlayingPosition", "()J" ) ;
		m_jmidSeekPosition = GetMethodID( "seekPosition", "(J)Z" ) ;
		m_jmidGetSampleFrequency = GetMethodID( "getSampleFrequency", "()I" ) ;
		m_jmidGetTotalLength = GetMethodID( "getTotalLength", "()J" ) ;
		m_jmidIsPlaying = GetMethodID( "isPlaying", "()Z" ) ;
		m_jmidIsPaused = GetMethodID( "isPaused", "()Z" ) ;
		m_jmidSetLoop = GetMethodID( "setLoop", "(Z)Z" ) ;
		m_jmidGetVideoWidth = GetMethodID( "getVideoWidth", "()I" ) ;
		m_jmidGetVideoHeight = GetMethodID( "getVideoHeight", "()I" ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidMediaPlayer::~SGLAndroidMediaPlayer( void )
{
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Open
	( const wchar_t * pwszFilePath, uint64_t nFlags,
			SSystem::SEnvironmentInterface * pEnv )
{
	JNI::JavaObject	jobjFilePath ;
	if ( SString::CompareLeft( pwszFilePath, L"assets://" ) == 0 )
	{
		size_t	lenScheme = SString::GetLength( L"assets://" ) ;
		if ( m_flagMovie )
		{
			if ( !CallBooleanMethod
				( m_jmidOpenMovieOnAssets,
					jobjFilePath.CreateWideString( pwszFilePath + lenScheme ) ) )
			{
				return	sglErrFailed ;
			}
		}
		else
		{
			if ( !CallBooleanMethod
				( m_jmidOpenAudioOnAssets,
					jobjFilePath.CreateWideString( pwszFilePath + lenScheme ) ) )
			{
				return	sglErrFailed ;
			}
		}
	}
	else
	{
		SString	strFilePath ;
		if ( SFileOpener::DefaultDirectPathOf( strFilePath, pwszFilePath ) )
		{
			strFilePath = pwszFilePath ;
		}
		if ( m_flagMovie )
		{
			if ( !CallBooleanMethod
				( m_jmidOpenMovie,
					jobjFilePath.CreateWideString( strFilePath ) ) )
			{
				return	sglErrFailed ;
			}
		}
		else
		{
			if ( !CallBooleanMethod
				( m_jmidOpenAudio,
					jobjFilePath.CreateWideString( strFilePath ) ) )
			{
				return	sglErrFailed ;
			}
		}
	}
	m_strMediaPath = pwszFilePath ;
	return	sglErrSuccess ;
}

SGLError SGLAndroidMediaPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	SFile *	prf = ESLTypeCast<SFile>( file ) ;
	if ( prf == NULL )
	{
		return	sglErrFailed ;
	}
	SString	strFilePath = prf->GetFilePath() ;
	if ( flagOwner )
	{
		delete	file ;
	}
	return	Open( strFilePath, nFlags ) ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLAndroidMediaPlayer::ClonePlayer( void )
{
	if ( m_strMediaPath.IsEmpty() )
	{
		return	NULL ;
	}
	SGLAndroidMediaPlayer *	pPlayer = new SGLAndroidMediaPlayer( m_flagMovie ) ;
	if ( pPlayer->Open( m_strMediaPath ) )
	{
		delete	pPlayer ;
		return	NULL ;
	}
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Close( void )
{
	CallVoidMethod( m_jmidClose ) ;
	m_strMediaPath.FreeArray() ;
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Play( uint64_t nFlags )
{
	if ( !CallBooleanMethod( m_jmidPlay ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Stop( void )
{
	if ( !CallBooleanMethod( m_jmidStop ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	if ( !CallBooleanMethod( m_jmidSetLoop, fLoop ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Pause( void )
{
	if ( !CallBooleanMethod( m_jmidPause ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::Restart( void )
{
	if ( !CallBooleanMethod( m_jmidRestart ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	JNI::JavaObject	jobjVols ;
	jdoubleArray	jdarVols = jobjVols.CreateDoubleArray( 2 ) ;
	if ( !CallBooleanMethod( m_jmidGetVolume, jdarVols ) )
	{
		return	sglErrFailed ;
	}
	JNI::JDoubleArray	jdarBuf( jdarVols ) ;
	jdouble *	pVols = jdarBuf.GetBuffer() ;
	for ( size_t i = 0; (i < nChannels) && (i < 2); i ++ )
	{
		pVolumes[i] = (float32_t) pVols[i] ;
	}
	jdarBuf.ReleaseBuffer() ;
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	jdouble	volLeft = 1.0, volRight = 1.0 ;
	if ( nChannels >= 2 )
	{
		volLeft = pVolumes[0] ;
		volRight = pVolumes[1] ;
	}
	else if ( nChannels == 1 )
	{
		volLeft = pVolumes[0] ;
		volRight = pVolumes[0] ;
	}
	if ( !CallBooleanMethod( m_jmidSetVolume, volLeft, volRight ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidMediaPlayer::IsPlaying( void ) const
{
	return	(bool) CallBooleanMethod( m_jmidIsPlaying ) ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidMediaPlayer::IsPaused( void ) const
{
	return	(bool) CallBooleanMethod( m_jmidIsPaused ) ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLAndroidMediaPlayer::GetSampleFrequency( void ) const
{
	return	(uint32_t) CallIntMethod( m_jmidGetSampleFrequency ) ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAndroidMediaPlayer::GetTotalLength( void ) const
{
	return	(uint64_t) CallLongMethod( m_jmidGetTotalLength ) ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAndroidMediaPlayer::GetPosition( void )
{
	return	(uint64_t) CallLongMethod( m_jmidGetPlayingPosition ) ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLAndroidMediaPlayer::SeekPosition( uint64_t nPos )
{
	CallBooleanMethod( m_jmidSeekPosition, (jlong) nPos ) ;
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLAndroidMediaPlayer::GetAudioStream( void )
{
	return	NULL ;
}

void SGLAndroidMediaPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::GetVideoSize( SGLSize& sizeVideo )
{
	sizeVideo.w = CallIntMethod( m_jmidGetVideoWidth ) ;
	sizeVideo.h = CallIntMethod( m_jmidGetVideoHeight ) ;
	return	sglErrSuccess ;
}

// 表示先を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::SetVideoView
	( SGLAbstractWindow* pWindow,
		const SGLImageRect& rectVideo, uint64_t nFlags )
{
	return	sglErrSuccess ;
}

// 現在のフレームを描画する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidMediaPlayer::DrawVideo
	( SGLPaintContextInterface* pPaint,
		const SGLImageRect& rectDst,
		uint32_t nFlags, uint32_t nTransparency )
{
	return	sglErrFailed ;
}

// メディア再生通知リスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerFrameNotification *
	SGLAndroidMediaPlayer::SetNotificationListener
		( SGLMediaPlayerFrameNotification * pListener )
{
	return	NULL ;
}

// ビデオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLVideoInputStream * SGLAndroidMediaPlayer::GetVideoStream( void )
{
	return	NULL ;
}

void SGLAndroidMediaPlayer::ReleaseVideoStream( SGLVideoInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLAndroidMediaPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
}


