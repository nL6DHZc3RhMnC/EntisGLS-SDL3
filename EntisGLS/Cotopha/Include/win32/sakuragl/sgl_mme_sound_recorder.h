
#if	!defined(__SAKURAGL_MEDIA_MME_SOUND_RECORDER_H__)
#define	__SAKURAGL_MEDIA_MME_SOUND_RECORDER_H__	1

#include <sakura/ssys_queue_buffer.h>
#include <sakuragl/media/sgl_sound_recorder.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Win32 マルチメディア API サウンド入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWin32MMSoundRecorder	: public SGLSoundRecorderInterface
	{
	protected:
		HWAVEIN						m_hWaveIn ;
		bool						m_fRecording ;
		WAVEHDR						m_wavhdr[2] ;
		WAVEFORMATEX				m_wfx ;
		SSystem::SArray<uint8_t>	m_bufRecord ;

		SSystem::SCriticalSection	m_csStream ;
		SSystem::SQueueBuffer		m_qbufStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWin32MMSoundRecorder, SGLSoundRecorderInterface )
		// 構築関数
		SGLWin32MMSoundRecorder( void ) ;
		// 消滅関数
		virtual ~SGLWin32MMSoundRecorder( void ) ;
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

	protected:
		// コールバック関数
		static void CALLBACK waveInProc
			( HWAVEIN hwi, UINT uMsg,
				DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD dwParam2 ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// WASAPI サウンド入力インターフェース (Windows Vista 以降でのみ動作)
	//////////////////////////////////////////////////////////////////////////

	class	SGLWinCoreAudioRecorder
				: public SGLSoundRecorderInterface, public SSystem::SProcedure
	{
	public:
		enum	TargetDevice
		{
			deviceCapture	= 0x0001,
			deviceRender	= 0x0002,
		} ;

	protected:
		IMMDevice *					m_pCapDevice ;
		IAudioClient *				m_pAudioClient ;
		IAudioCaptureClient *		m_pCaptureClient ;
		uint32_t					m_maskTargetDevices ;
		bool						m_flagReady ;
		bool						m_flagRecording ;
		bool						m_flagStopRecord ;
		SGLSoundFormat				m_fmtCapture ;
		SGLSoundFormat				m_fmtOutput ;
		uint64_t					m_nCapSamples ;
		uint64_t					m_nOutSamples ;
		uint64_t					m_lcmFreqConvert ;
		SSystem::SQueueBuffer		m_queRecorded ;
		SSystem::SArray<int16_t>	m_bufConvertPCM16 ;
		SSystem::SArray<int16_t>	m_bufConvertFreq ;
		SSystem::SThread			m_thread ;
		SSystem::SCriticalSection	m_csSync ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWinCoreAudioRecorder, SGLSoundRecorderInterface )
		// 構築関数
		SGLWinCoreAudioRecorder( void ) ;
		// 消滅関数
		virtual ~SGLWinCoreAudioRecorder( void ) ;
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

	public:
		// ターゲットデバイス
		uint32_t GetTargetDevices( void ) const ;
		void SetTargetDevices( uint32_t maskDev ) ;
		// 対応しているか？
		static bool IsSupported( void ) ;

	public:
		// 録音スレッド関数
		virtual void Run( void ) ;
	} ;

}

#endif

