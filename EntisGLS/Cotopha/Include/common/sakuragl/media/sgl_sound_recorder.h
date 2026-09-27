
#if	!defined(__SAKURAGL_MEDIA_SOUND_RECORDER_H__)
#define	__SAKURAGL_MEDIA_SOUND_RECORDER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundRecorderInterface	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundRecorderInterface, SObject )
		// 構築関数
		SGLSoundRecorderInterface( void ) ;
		// デバイス列挙
		virtual size_t EnumerateDevices
				( uint16_t * pwNameBuf, size_t nNameBufLength ) = 0 ;
		// フォーマットを指定して入力を準備する
		virtual SGLError Open( size_t iDevice, const SGLSoundFormat& fmt ) = 0 ;
		// 入力用に準備したサウンド入力を解放する
		virtual SGLError Close( void ) = 0 ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes = 0 ) = 0 ;
		// ストリームバッファから読み出す
		virtual size_t Read( void * ptrSound, size_t nBytes ) = 0 ;
		// 録音を開始する
		virtual SGLError Start( uint64_t nFlags = 0 ) = 0 ;
		// 録音を停止する
		virtual SGLError Stop( void ) = 0 ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 録音中か？
		virtual bool IsRecording( void ) const = 0 ;
	} ;

	#if	defined(__COTOPHA__)
	class	native SoundRecorder	: public SSystem::VolatileObject
	{
	public:
		// デバイス列挙
		native size_t EnumerateDevices
				( uint16_t * pwNameBuf, size_t nNameBufLength ) ;
		// フォーマットを指定して入力を準備する
		native SGLError Open( size_t iDevice, const SGLSoundFormat& fmt ) ;
		// 入力用に準備したサウンド入力を解放する
		native SGLError Close( void ) ;
		// ストリームバッファを準備する
		native SGLError PrepareStream( size_t nBytes = 0 ) ;
		// ストリームバッファから読み出す
		native size_t Read( void * ptrSound, size_t nBytes ) ;
		// 録音を開始する
		native SGLError Start( uint64_t nFlags = 0 ) ;
		// 録音を停止する
		native SGLError Stop( void ) ;
		// 音量取得 [L/R]
		native SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		native SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 録音中か？
		native bool IsRecording( void ) const ;
	} ;
	#else
	typedef	SGLSoundRecorderInterface	SoundRecorder ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド入力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundRecorder	: public SGLSoundRecorderInterface
	{
	protected:
		SoundRecorder *	m_pRecorder ;
		bool			m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundRecorder, SGLSoundRecorderInterface )
		// 構築関数
		SGLSoundRecorder( void ) : m_pRecorder(NULL), m_flagOwner(false) {}
		SGLSoundRecorder( SoundRecorder* pRecorder, bool flagOwner = false )
						: m_pRecorder(pRecorder), m_flagOwner(flagOwner) {}
		// 消滅関数
		virtual ~SGLSoundRecorder( void ) ;
		// 代入
		SoundRecorder * operator = ( SoundRecorder * pRecorder )
		{
			return	SetSoundRecorder( pRecorder ) ;
		}
		SoundRecorder * SetSoundRecorder
				( SoundRecorder * pRecorder, bool flagOwner = false ) ;
		// ポインタ変換
		SoundRecorder * operator -> ( void ) const
		{
			return	m_pRecorder ;
		}
		SoundRecorder * GetPlayer( void ) const
		{
			return	m_pRecorder ;
		}
		operator SoundRecorder * ( void ) const
		{
			return	m_pRecorder ;
		}

	public:
		// デバイス列挙
		virtual size_t EnumerateDevices
				( uint16_t * pwNameBuf, size_t nNameBufLength ) ;
		size_t EnumerateDevices
			( SSystem::SObjectArray<SSystem::SString>& aDevNames ) ;
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

