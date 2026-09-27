
#if	!defined(__SAKURAGL_MEDIA_DIRECT_SOUND_PLAYER_H__)
#define	__SAKURAGL_MEDIA_DIRECT_SOUND_PLAYER_H__	1

#include <dsound.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// DirectSound 出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLDirectSoundPlayer : public SGLSoundPlayerInterface
	{
	protected:
		struct IDirectSoundBuffer *	m_idsbuf ;		// 再生バッファ
		WAVEFORMATEX	m_wfxFormat ;

		// バッファ
		enum	ConstantValues
		{
			countBuffer = 2,
		} ;
		class	SoundPCMBuffer : public SSystem::SArray<uint8_t>
		{
		public:
			uint8_t *	m_pbytBuffer ;
			size_t		m_sizeBuffer ;
			size_t		m_sizeStuffed ;
		} ;
		SoundPCMBuffer	m_bufSound[countBuffer] ;	// バッファ
		size_t			m_nBufferSize ;
		size_t			m_iBuffering ;				// Write 対象バッファ
		ssize_t			m_iPlayingBuf ;				// 再生中のバッファ
		uint64_t		m_nBasePosition ;			// 再生の基準位置 [bytes]
		DWORD			m_dwPausedPosition ;		// 一時停止の位置 [bytes]
		bool			m_flagStreaming ;			// ストリーミング
		bool			m_flagPlayed ;
		bool			m_flagLooping ;
		bool			m_flagPaused ;

		// ストリーミング・スレッド
		bool						m_flagThread ;
		SSystem::SThread *			m_pThread ;
		SSystem::SignalEvent		m_signalExit ;
		SSystem::SignalEvent		m_doneThread ;
		SSystem::SMutex				m_mutexSync ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLDirectSoundPlayer, SGLSoundPlayerInterface )
		// 構築関数
		SGLDirectSoundPlayer( void ) ;
		// 消滅関数
		virtual ~SGLDirectSoundPlayer( void ) ;

	public:
		// フォーマットを指定して出力を準備する
		virtual SGLError Open( const SGLSoundFormat& fmt ) ;
		// 出力用に準備したサウンド出力を解放する
		virtual SGLError Close( void ) ;
		// スタティックバッファを準備して書き込む
		virtual SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes ) ;
		// ストリームバッファへ書き出す
		virtual size_t Write( const void * ptrSound, size_t nBytes ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
		// 再生を一時停止する
		virtual SGLError Pause( void ) ;
		// 再生を再開する
		virtual SGLError Restart( void ) ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 再生中か？
		virtual bool IsPlaying( void ) const ;
		// 一時停止中か？
		virtual bool IsPaused( void ) const ;
		// 再生済みサンプル数を取得する
		virtual uint64_t GetPlayingPosition( void ) ;
		// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
		virtual SGLError SeekPosition( uint64_t nPos ) ;
		// コールバック設定
		virtual SGLSoundPlayerListener *
					SetListener( SGLSoundPlayerListener * listener ) ;

	public:
		// Volume と Pan から左右チャネルの音量へ変換
		static void VolumePanToStereo
			( float32_t* pVolumes, LONG lVolume, LONG lPan ) ;
		// 左右チャネルの音量から Volume と Pan へ変換
		static void VolumeStereoToPan
			( LONG& lVolume, LONG& lPan, const float32_t* pVolumes ) ;

	protected:
		// スレッド関数
		static void StreamingThreadProc( void * pInstance ) ;
		void StreamingProc( void ) ;

	protected:
		// 再生位置ポーリングと情報更新
		SGLError PollingCurrentPosition( size_t& iPlayingBuf, DWORD& dwPos ) ;
		// バッファロスト正常化
		void NormalizeLostBuffer( void ) ;
		// バッファを初期化
		void InitializeStreamingBuffer( void ) ;
		// バッファが指定位置まで満たされるまでリスナを呼び出す
		void CallbackListenerForBuffer( size_t iBuffer, size_t nBytes ) ;
		// DirectSoundBuffer にデータを更新する
		SGLError UpdateDirectSoundBuffer( size_t nOffset, size_t nBytes ) ;
		// スレッド開始
		void BeginListenerThread( void ) ;
		// スレッド終了
		void EndListenerThread( void ) ;

	protected:
		// DirectSound
		static ESL_DLL_EXPORT struct IDirectSound *			m_idsound ;			// DirectSound object
		static ESL_DLL_EXPORT struct IDirectSoundBuffer *	m_idsbPrimary ;		// primary DirectSound buffer

	public:
		// 初期化
		static SGLError Initialize
			( const SGLSoundFormat& fmt, LPCGUID pcGuidDev = NULL ) ;
		static SGLError Initialize( void ) ;
		// 終了
		static SGLError Finalize( void ) ;
		// 再生デバイス列挙
		static void EnumerateDSDevices
			( SSystem::SObjectArray<GUID>& aDevGUIDs,
				SSystem::SObjectArray<SSystem::SString>& aDevNames ) ;
		static void EnumerateDevices
			( SSystem::SArray<GUID>& aDevGUIDs,
				SSystem::SObjectArray<SSystem::SString>& aDevNames ) ;

	protected:
		struct	EnumContext
		{
			SSystem::SObjectArray<GUID> *				pDevGUIDs ;
			SSystem::SObjectArray<SSystem::SString> *	pDevNames ;
		} ;
		static BOOL CALLBACK DSEnumCallback
			( LPGUID lpGuid, LPCSTR lpcstrDescription,
				LPCSTR lpcstrModule, LPVOID lpContext ) ;

	public:
		// SGLSoundFormat -> WAVEFORMATEX 変換
		static void ConvertWaveFormat
			( WAVEFORMATEX& wfx, const SGLSoundFormat& fmt ) ;

	} ;

}

#endif
