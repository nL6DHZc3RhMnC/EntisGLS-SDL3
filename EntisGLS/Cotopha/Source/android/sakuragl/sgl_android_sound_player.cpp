
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_android_sound_player.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Android AudioTrack 出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLAndroidSoundPlayer, SGLSoundPlayerInterface, JavaObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidSoundPlayer::SGLAndroidSoundPlayer( void )
{
	SGLSoundPlayerInterface *	pPlayer = this ;
	JNI::JavaObject	jobjBuf ;
	if ( jobjBuf.CreateByteBuffer
			( pPlayer, sizeof(SGLSoundPlayerInterface) ) != NULL )
	{
		if ( JavaObject::CreateJavaObject
			( ENTIS_GLS4_JAVA_PACKAGE "/SoundPlayer",
				"(L" JAVA_NIO_BYTEBUFFER ";)V", jobjBuf.GetObject() ) != NULL )
		{
			MakeGlobalRef() ;
			//
			m_jmidSetFormat = GetMethodID( "setFormat", "(IIII)Z" ) ;
			m_jmidClose = GetMethodID( "close", "()V" ) ;
			m_jmidWriteStaticByte = GetMethodID( "writeStatic", "([B)Z" ) ;
			m_jmidWriteStaticWord = GetMethodID( "writeStatic", "([S)Z" ) ;
			m_jmidPrepareStreaming = GetMethodID( "prepareStreaming", "(I)V" ) ;
			m_jmidWriteStreamingByte = GetMethodID( "writeStreaming", "([B)I" ) ;
			m_jmidWriteStreamingWord = GetMethodID( "writeStreaming", "([S)I" ) ;
			m_jmidPlay = GetMethodID( "play", "(Z)Z" ) ;
			m_jmidStop = GetMethodID( "stop", "()Z" ) ;
			m_jmidPause = GetMethodID( "pause", "()Z" ) ;
			m_jmidRestart = GetMethodID( "restart", "()Z" ) ;
			m_jmidGetVolume = GetMethodID( "getVolume", "([D)V" ) ;
			m_jmidSetVolume = GetMethodID( "setVolume", "(DD)Z" ) ;
			m_jmidGetPlayingPosition = GetMethodID( "getPlayingPosition", "()J" ) ;
			m_jmidSeekPosition = GetMethodID( "seekPosition", "(J)Z" ) ;
			m_jmidIsPlaying = GetMethodID( "isPlaying", "()Z" ) ;
			m_jmidIsPaused = GetMethodID( "isPaused", "()Z" ) ;
			m_jmidSetStreamingListener =
				GetMethodID( "setStreamingListener",
					"(L" ENTIS_GLS4_JAVA_PACKAGE
						"/SoundPlayer$StreamingListener;)L"
							ENTIS_GLS4_JAVA_PACKAGE
							"/SoundPlayer$StreamingListener;" ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidSoundPlayer::~SGLAndroidSoundPlayer( void )
{
}

// フォーマットを指定して出力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Open( const SGLSoundFormat& fmt )
{
	if ( CallBooleanMethod
		( m_jmidSetFormat, (jint) fmt.format, (jint) fmt.frequency,
						(jint) fmt.channels, (jint) fmt.bitsPerSample ) )
	{
		m_fmt = fmt ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 出力用に準備したサウンド出力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Close( void )
{
	CallVoidMethod( m_jmidClose ) ;
	return	sglErrSuccess ;
}

// スタティックバッファを準備して書き込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::WriteStatic( const void * ptrSound, size_t nBytes )
{
	if ( m_fmt.bitsPerSample == 16 )
	{
		const size_t	nLength = (nBytes >> 1) ;
		{
			JNI::JavaObject	jobjBuf ;
			{
				JNI::JShortArray
						jsarrBuf( jobjBuf.CreateShortArray( nLength ) ) ;
				jshort *	psBuf = jsarrBuf.GetBuffer() ;
				for ( size_t i = 0; i < nLength; i ++ )
				{
					psBuf[i] = (jshort) ((int16_t*)ptrSound)[i] ;
				}
			}
			if ( CallBooleanMethod
					( m_jmidWriteStaticWord, jobjBuf.GetObject() ) )
			{
				return	sglErrSuccess ;
			}
		}
	}
	else
	{
		JNI::JavaObject	jobjBuf ;
		{
			JNI::JByteArray
					jbarrBuf( jobjBuf.CreateByteArray( nBytes ) ) ;
			jbyte *	pbBuf = jbarrBuf.GetBuffer() ;
			for ( size_t i = 0; i < nBytes; i ++ )
			{
				pbBuf[i] = (jbyte) ((uint8_t*)ptrSound)[i] ;
			}
		}
		if ( CallBooleanMethod
				( m_jmidWriteStaticByte, jobjBuf.GetObject() ) )
		{
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::PrepareStream( size_t nBytes )
{
	CallVoidMethod( m_jmidPrepareStreaming, (jint) nBytes ) ;
	return	sglErrSuccess ;
}

// ストリームバッファへ書き出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLAndroidSoundPlayer::Write( const void * ptrSound, size_t nBytes )
{
	if ( nBytes == 0 )
	{
		return	0 ;
	}
	if ( m_fmt.bitsPerSample == 16 )
	{
		const size_t	nLength = (nBytes >> 1) ;
		if ( nLength == 0 )
		{
			return	0 ;
		}
		JNI::JavaObject	jobjBuf ;
		{
			JNI::JShortArray
					jsarrBuf( jobjBuf.CreateShortArray( nLength ) ) ;
			jshort *	psBuf = jsarrBuf.GetBuffer() ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				psBuf[i] = (jshort) ((int16_t*)ptrSound)[i] ;
			}
		}
		jint	nWritten =
			CallIntMethod
				( m_jmidWriteStreamingWord, jobjBuf.GetObject() ) ;
		return	(size_t) (nWritten << 1) ;
	}
	else
	{
		JNI::JavaObject	jobjBuf ;
		{
			JNI::JByteArray
					jbarrBuf( jobjBuf.CreateByteArray( nBytes ) ) ;
			jbyte *	pbBuf = jbarrBuf.GetBuffer() ;
			for ( size_t i = 0; i < nBytes; i ++ )
			{
				pbBuf[i] = (jbyte) ((uint8_t*)ptrSound)[i] ;
			}
		}
		jint	nWritten =
			CallIntMethod
				( m_jmidWriteStreamingByte, jobjBuf.GetObject() ) ;
		return	(size_t) nWritten ;
	}
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Play( uint64_t nFlags )
{
	if ( CallBooleanMethod
		( m_jmidPlay, (jboolean) ((nFlags & flagPlayLoop) != 0) ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Stop( void )
{
	if ( CallBooleanMethod( m_jmidStop ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Pause( void )
{
	if ( CallBooleanMethod( m_jmidPause ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::Restart( void )
{
	if ( CallBooleanMethod( m_jmidRestart ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	JNI::JavaObject	jobjVolumes ;
	jdoubleArray	jdarrVolumes = jobjVolumes.CreateDoubleArray( 2 ) ;
	//
	CallVoidMethod( m_jmidGetVolume, jdarrVolumes ) ;
	//
	JNI::JDoubleArray	jdarrGotVol( jdarrVolumes ) ;
	jdouble *			pGotVol = jdarrGotVol.GetBuffer() ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		if ( i < 2 )
		{
			pVolumes[i] = (float32_t) pGotVol[i] ;
		}
		else
		{
			pVolumes[i] = 1.0f ;
		}
	}
	jdarrGotVol.ReleaseBuffer() ;
	//
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	jdouble	volLeft, volRight ;
	if ( nChannels == 0 )
	{
		return	sglErrFailed ;
	}
	if ( nChannels == 1 )
	{
		volLeft = volRight = pVolumes[0] ;
	}
	else
	{
		volLeft = pVolumes[0] ;
		volRight = pVolumes[1] ;
	}
	if ( CallBooleanMethod( m_jmidSetVolume, volLeft, volRight ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidSoundPlayer::IsPlaying( void ) const
{
	return	CallBooleanMethod( m_jmidIsPlaying ) ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidSoundPlayer::IsPaused( void ) const
{
	return	CallBooleanMethod( m_jmidIsPaused ) ;
}

// 再生済みサンプル数を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAndroidSoundPlayer::GetPlayingPosition( void )
{
	return	CallLongMethod( m_jmidGetPlayingPosition ) ;
}

// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundPlayer::SeekPosition( uint64_t nPos )
{
	if ( CallBooleanMethod( m_jmidSeekPosition, (jlong) nPos ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerListener *
	SGLAndroidSoundPlayer::SetListener( SGLSoundPlayerListener * listener )
{
	JNI::JavaObject	jobjBuf ;
	JNI::JavaObject	jobjListener ;
	if ( listener != NULL )
	{
		jobjBuf.CreateByteBuffer
			( listener, sizeof(SGLSoundPlayerListener) ) ;
		jobjListener.CreateJavaObject
			( ENTIS_GLS4_JAVA_PACKAGE "/NativeSoundPlayerListener",
				"(L" JAVA_NIO_BYTEBUFFER ";)V", jobjBuf.GetObject() ) ;
	}
	jobject	objLast =
		CallObjectMethod
			( m_jmidSetStreamingListener, jobjListener.GetObject() ) ;
	if ( objLast != NULL )
	{
		JNI::JavaObject	jobjLast( objLast, true ) ;
		jfieldID	jfidListener =
			jobjLast.GetFieldID( "m_bufListener", "L" JAVA_NIO_BYTEBUFFER ";" ) ;
		if ( jfidListener )
		{
			JNI::JDirectBuffer
				jdbLast( jobjLast.GetObjectField( jfidListener ) ) ;
			return	(SGLSoundPlayerListener*) jdbLast.GetBuffer() ;
		}
		else
		{
			JNI::GetJNIEnv()->ExceptionClear() ;
		}
	}
	return	NULL ;
}


