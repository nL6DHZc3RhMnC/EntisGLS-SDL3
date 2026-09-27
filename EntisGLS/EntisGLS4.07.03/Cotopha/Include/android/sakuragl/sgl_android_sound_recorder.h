
#if	!defined(__SAKURAGL_MEDIA_ANDROID_SOUND_RECORDER_H__)
#define	__SAKURAGL_MEDIA_ANDROID_SOUND_RECORDER_H__	1

#include <esl/esl_java_object.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakuragl/media/sgl_sound_recorder.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Android AudioRecord 入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAndroidSoundRecorder
				: public SGLSoundRecorderInterface, public JNI::JavaObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLAndroidSoundRecorder, SGLSoundRecorderInterface, JavaObject )
		// 構築関数
		SGLAndroidSoundRecorder( void ) ;
		// 消滅関数
		virtual ~SGLAndroidSoundRecorder( void ) ;

	protected:
		SGLSoundFormat			m_fmt ;
		SSystem::SQueueBuffer	m_qbufStream ;

		jmethodID				m_jmidSetFormat ;
		jmethodID				m_jmidClose ;
		jmethodID				m_jmidPrepareStreaming ;
		jmethodID				m_jmidGetByteData ;
		jmethodID				m_jmidGetWordData ;
		jmethodID				m_jmidStart ;
		jmethodID				m_jmidStop ;
		jmethodID				m_jmidIsRecording ;

	public:
		// デバイス列挙
		virtual size_t EnumerateDevices
				( uint16_t * pwNameBuf, size_t nNameBufLength ) ;
		// フォーマットを指定して入力を準備する
		virtual SGLError Open( size_t iDevice, const SGLSoundFormat& fmt ) ;
		// 入力用に準備したサウンド入力を解放する
		virtual SGLError Close( void ) ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes = 0 ) ;
		// ストリームバッファから読み出す
		virtual size_t Read( void * ptrSound, size_t nBytes ) ;
		// 録音を開始する
		virtual SGLError Start( uint64_t nFlags = 0 ) ;
		// 録音を停止する
		virtual SGLError Stop( void ) ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 録音中か？
		virtual bool IsRecording( void ) const ;

	} ;

}

#endif

