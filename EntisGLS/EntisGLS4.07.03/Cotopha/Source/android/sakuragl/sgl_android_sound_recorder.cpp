
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_android_sound_recorder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Android AudioRecord 入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLAndroidSoundRecorder, SGLSoundRecorderInterface, JavaObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidSoundRecorder::SGLAndroidSoundRecorder( void )
{
	SGLSoundRecorderInterface *	pRec = this ;
	JNI::JavaObject	jobjBuf ;
	if ( jobjBuf.CreateByteBuffer
			( pRec, sizeof(SGLAndroidSoundRecorder) ) != NULL )
	{
		if ( JavaObject::CreateJavaObject
			( ENTIS_GLS4_JAVA_PACKAGE "/SoundRecorder",
				"(L" JAVA_NIO_BYTEBUFFER ";)V", jobjBuf.GetObject() ) != NULL )
		{
			MakeGlobalRef() ;
			//
			m_jmidSetFormat = GetMethodID( "setFormat", "(IIIII)Z" ) ;
			m_jmidClose = GetMethodID( "close", "()V" ) ;
			m_jmidPrepareStreaming = GetMethodID( "prepareStreaming", "(I)V" ) ;
			m_jmidGetByteData = GetMethodID( "getByteData", "()[B" ) ;
			m_jmidGetWordData = GetMethodID( "getWordData", "()[S" ) ;
			m_jmidStart = GetMethodID( "start", "()Z" ) ;
			m_jmidStop = GetMethodID( "stop", "()Z" ) ;
			m_jmidIsRecording = GetMethodID( "isRecording", "()Z" ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidSoundRecorder::~SGLAndroidSoundRecorder( void )
{
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
size_t SGLAndroidSoundRecorder::EnumerateDevices
		( uint16_t * pwNameBuf, size_t nNameBufLength )
{
	static const wchar_t *	s_pwszDevNames[3] =
	{
		L"Default", L"Mic", L"Camera",
	} ;
	size_t	iBuf = 0 ;
	for ( size_t i = 0; i < 3; i ++ )
	{
		const wchar_t *	pwszDevName = s_pwszDevNames[i] ;
		for ( size_t j = 0; iBuf + j < nNameBufLength; j ++ )
		{
			pwNameBuf[iBuf + j] = (uint16_t) pwszDevName[j] ;
			if ( pwszDevName[j] == 0 )
			{
				break ;
			}
		}
	}
	return	3 ;
}

// フォーマットを指定して入力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::Open( size_t iDevice, const SGLSoundFormat& fmt )
{
	if ( CallBooleanMethod
		( m_jmidSetFormat,
			(jint) iDevice, (jint) fmt.format, (jint) fmt.frequency,
						(jint) fmt.channels, (jint) fmt.bitsPerSample ) )
	{
		m_fmt = fmt ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 入力用に準備したサウンド入力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::Close( void )
{
	CallVoidMethod( m_jmidClose ) ;
	return	sglErrSuccess ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::PrepareStream( size_t nBytes )
{
	CallVoidMethod( m_jmidPrepareStreaming, (jint) nBytes ) ;
	return	sglErrSuccess ;
}

// ストリームバッファから読み出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLAndroidSoundRecorder::Read( void * ptrSound, size_t nBytes )
{
	while ( m_qbufStream.GetLength() < nBytes )
	{
		if ( m_fmt.bitsPerSample == 16 )
		{
			JNI::JSmartObject
				jsobjBuf( CallObjectMethod( m_jmidGetWordData ) ) ;
			if ( jsobjBuf.GetObject() == NULL )
			{
				break ;
			}
			JNI::JShortArray	jsaBuf( (jshortArray) jsobjBuf.GetObject() ) ;
			jshort *	psBuf = jsaBuf.GetBuffer() ;
			jsize		nLength = jsaBuf.GetLength() ;
			if ( nLength == 0 )
			{
				break ;
			}
			m_qbufStream.Write( psBuf, nLength * 2 ) ;
		}
		else if ( m_fmt.bitsPerSample == 8 )
		{
			JNI::JSmartObject
				jsobjBuf( CallObjectMethod( m_jmidGetByteData ) ) ;
			if ( jsobjBuf.GetObject() == NULL )
			{
				break ;
			}
			JNI::JByteArray	jbaBuf( (jbyteArray) jsobjBuf.GetObject() ) ;
			jbyte *	pbBuf = jbaBuf.GetBuffer() ;
			jsize	nLength = jbaBuf.GetLength() ;
			if ( nLength == 0 )
			{
				break ;
			}
			m_qbufStream.Write( pbBuf, nLength ) ;
		}
	}
	return	m_qbufStream.Read( ptrSound, nBytes ) ;
}

// 録音を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::Start( uint64_t nFlags )
{
	if ( CallBooleanMethod( m_jmidStart ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 録音を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::Stop( void )
{
	if ( CallBooleanMethod( m_jmidStop ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] = 1.0f ;
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidSoundRecorder::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	return	sglErrFailed ;
}

// 録音中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidSoundRecorder::IsRecording( void ) const
{
	return	CallBooleanMethod( m_jmidIsRecording ) ;
}



