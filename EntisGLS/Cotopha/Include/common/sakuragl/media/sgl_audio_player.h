
#if	!defined(__SAKURAGL_MEDIA_AUDIO_PLAYER_H__)
#define	__SAKURAGL_MEDIA_AUDIO_PLAYER_H__	1

#include <sakuragl/media/sgl_sound_player.h>
#include <sakuragl/media/sgl_media_composer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// オーディオファイル再生インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioPlayerInterface	: public SSystem::SObject
	{
	public:
		// オープンフラグ
		enum	OpenFlags
		{
			modeOpenAuto,
			modeOpenStatic,
			modeOpenAutoStatic,
			modeOpenDynamicOnMemory,
			modeOpenDynamicRead,
			modeOpenMask		= 0x00FF,
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioPlayerInterface, SObject )
		// 指定ファイルを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) = 0 ;
		virtual SGLError Create
			( SSystem::SFileInterface * file,
				bool flagOwner = true, uint64_t nFlags = 0 ) = 0 ;
		// データを参照する複製プレイヤー生成
		virtual SGLAudioPlayerInterface * ClonePlayer( void ) = 0 ;
		// ファイルを閉じる
		virtual SGLError Close( void ) = 0 ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) = 0 ;
		// 再生を停止する
		virtual SGLError Stop( void ) = 0 ;
		// ループポイント[/sample] を設定する
		virtual SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) = 0 ;
		// 再生を一時停止する
		virtual SGLError Pause( void ) = 0 ;
		// 再生を再開する
		virtual SGLError Restart( void ) = 0 ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 再生中か？
		virtual bool IsPlaying( void ) const = 0 ;
		// 一時停止中か？
		virtual bool IsPaused( void ) const = 0 ;
		// メディアのサンプル周波数を取得する
		virtual uint32_t GetSampleFrequency( void ) const = 0 ;
		// メディアの全長 [/sample] を取得する
		virtual uint64_t GetTotalLength( void ) const = 0 ;
		// 再生位置 [/sample] を取得する
		virtual uint64_t GetPosition( void ) = 0 ;
		// 再生位置 [/sample] を変更する
		virtual void SeekPosition( uint64_t nPos ) = 0 ;
		// オーディオストリーム取得
		virtual SGLAudioInputStream * GetAudioStream( void ) = 0 ;
		virtual void ReleaseAudioStream( SGLAudioInputStream * pStream ) = 0 ;
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) = 0 ;

	} ;

	#if	defined(__COTOPHA__)
	class	native AudioPlayer	: public SSystem::VolatileObject
	{
	public:
		// オープンフラグ
		enum	OpenFlags
		{
			modeOpenAuto,
			modeOpenStatic,
			modeOpenAutoStatic,
			modeOpenDynamicOnMemory,
			modeOpenDynamicRead,
			modeOpenMask		= 0x00FF,
		} ;
		// 指定ファイルを開く
		native SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0 ) ;
		native SGLError Create
			( SSystem::File * pFile, uint64_t nFlags = 0 ) ;
		// データを参照する複製プレイヤー生成
		native AudioPlayer * ClonePlayer( void ) ;
		// ファイルを閉じる
		native SGLError Close( void ) ;
		// 再生を開始する
		native SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		native SGLError Stop( void ) ;
		// ループポイント[/sample] を設定する
		native SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
		// 再生を一時停止する
		native SGLError Pause( void ) ;
		// 再生を再開する
		native SGLError Restart( void ) ;
		// 音量取得 [L/R]
		native SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		native SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 再生中か？
		native bool IsPlaying( void ) const ;
		// 一時停止中か？
		native bool IsPaused( void ) const ;
		// メディアのサンプル周波数を取得する
		native uint32_t GetSampleFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		native uint64_t GetTotalLength( void ) const ;
		// 再生位置 [/sample] を取得する
		native uint64_t GetPosition( void ) ;
		// 再生位置 [/sample] を変更する
		native void SeekPosition( uint64_t nPos ) ;
	} ;
	#else
	typedef	SGLAudioPlayerInterface	AudioPlayer ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// オーディオファイル再生ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaPlayer ;
	class	SGLAudioPlayer	: public SGLAudioPlayerInterface
	{
	protected:
		// ラッパーターゲット
		AudioPlayer *	m_pPlayer ;
		bool			m_flagOwner ;

		// 音量保持用
		uint32_t					m_maskLines ;
		SSystem::SArray<float32_t>	m_volumes ;

		#if	defined(__COTOPHA__)
		SSystem::SFileInterface *	m_pFile ;
		bool						m_flagFileOwner ;
		#endif

		// チェーン
		SGLAudioPlayer *			m_pPrevAudio ;
		SGLAudioPlayer *			m_pNextAudio ;

		// フェード中の音量ベジェ曲線
		SSystem::SArray<float32_t>	m_volFadeStart ;
		SSystem::SArray<float32_t>	m_volFadeEnd ;
		uint32_t					m_msecFadingDuration ;
		uint32_t					m_msecFadingTime ;

		// 同期用ミューテックス
		SSystem::SCriticalSection	m_mutex ;

		// チェーン先頭
		static SGLAudioPlayer *		m_pFirstAudioPlayer ;
		static ESL_DLL_EXPORT SSystem::SCriticalSection *	m_pMutexAll ;

		// フェード処理用ストックスレッド
		static SSystem::SThread *	m_pFadingThread ;
		static bool					m_flagReqFadingThread ;

	public:
		// 規定ライン番号
		enum	LineNumber
		{
			lineTotal	= 0,		// 全体
			lineTotal2nd,
			lineComposition,		// 動画など複合音声
			lineSystem,				// UI 操作 SE など
			lineMusic,				// BGM など
			lineSound,				// SE など
			lineVoice,				// 台詞など
			lineUserFirst,
			lineCount	= 32,
		} ;
	protected:
		// 全体音量
		static float32_t	m_volumeOfLine[lineCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioPlayer, SGLAudioPlayerInterface )
		// 構築関数
		SGLAudioPlayer( void )
			: m_pPlayer(NULL), m_flagOwner(false),
				m_maskLines((1 << lineTotal) | (1 << lineTotal2nd)),
				m_pPrevAudio(NULL), m_pNextAudio(NULL),
				m_msecFadingDuration(0), m_msecFadingTime(0) {}
		SGLAudioPlayer( AudioPlayer* pPlayer, bool flagOwner = false )
			: m_pPlayer(pPlayer), m_flagOwner(flagOwner),
				m_maskLines((1 << lineTotal) | (1 << lineTotal2nd)),
				m_pPrevAudio(NULL), m_pNextAudio(NULL),
				m_msecFadingDuration(0), m_msecFadingTime(0) {}
		SGLAudioPlayer( const SGLAudioPlayer& audio ) ;
		// 消滅関数
		virtual ~SGLAudioPlayer( void ) ;
		// 代入
		AudioPlayer * operator = ( AudioPlayer * pPlayer )
		{
			return	SetAudioPlayer( pPlayer ) ;
		}
		AudioPlayer * SetAudioPlayer
				( AudioPlayer * pPlayer, bool flagOwner = false ) ;
		// ポインタ変換
		AudioPlayer * operator -> ( void ) const
		{
			return	m_pPlayer ;
		}
		AudioPlayer * GetPlayer( void ) const
		{
			return	m_pPlayer ;
		}
		operator AudioPlayer * ( void ) const
		{
			return	m_pPlayer ;
		}

	public:
		// 指定ファイルを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SGLError Create
			( SSystem::SFileInterface * file,
				bool flagOwner = true, uint64_t nFlags = 0 ) ;
		// データを参照する複製プレイヤー生成
		virtual SGLAudioPlayerInterface * ClonePlayer( void ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
		// ループポイント[/sample] を設定する
		virtual SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
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
		// メディアのサンプル周波数を取得する
		virtual uint32_t GetSampleFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		virtual uint64_t GetTotalLength( void ) const ;
		// 再生位置 [/sample] を取得する
		virtual uint64_t GetPosition( void ) ;
		// 再生位置 [/sample] を変更する
		virtual void SeekPosition( uint64_t nPos ) ;
		// オーディオストリーム取得
		virtual SGLAudioInputStream * GetAudioStream( void ) ;
		virtual void ReleaseAudioStream( SGLAudioInputStream * pStream ) ;
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;

	public:
		// 静的な処理の初期化
		static void InitializeStatic( void ) ;
		// 静的な処理の解放
		static void ReleaseStatic( void ) ;

	public:
		// 音量を反映させるラインを取得する
		uint32_t GetVolumeLineMask( void ) const ;
		// 音量を反映させるラインをクリアする（lineTotal,lineTotal2nd 以外）
		void ClearAllVolumeLines( void ) ;
		// 音量を反映させるラインを設定する
		void SetVolumeLineMask( uint32_t maskLines ) ;
		void SetVolumeLine( size_t iLine ) ;
		void ResetVolumeLine( size_t iLine ) ;
		static void SetAudioLineMask
			( SGLAudioPlayerInterface * pPlayer, uint32_t maskLines ) ;
		// ライン音量を含め音量反映
		void ReflectVolume( void ) ;
		// ライン音量設定
		static void SetLineVolume( size_t iLine, double volume ) ;
		// ライン音量取得
		static double GetLineVolume( size_t iLine ) ;

	public:
		// 音量設定
		SGLError SetStereoVolume( float32_t volLeft, float32_t volRight ) ;
		// 音量取得
		SGLError GetStereoVolume( float32_t& volLeft, float32_t& volRight ) ;
		// 音量フェーディング開始
		void BeginFadeVolume
			( const float32_t* pVolumes, size_t nChannels, uint32_t msecDuration ) ;
		// 音量フェード中か？
		bool IsVolumeFading( void ) const ;
		// 音量フェーディングキャンセル
		void CancelFadeVolume( void ) ;
		// 音量フェーディング終了
		void FlushFadeVolume( void ) ;

	protected:
		// フェーディングスレッド生成
		static void BeginFadingThread( void ) ;
		// フェーディングスレッド関数
		static void FadingThreadProc( void * pInstance ) ;
		// 音量フェード処理
		bool OnFadingVolume( uint32_t msecPast ) ;

	protected:
		// 再生中オーディオ・チェーンに追加
		void AddToAudioChain( void ) ;
		// 再生中オーディオ・チェーンから分離
		void DetachFromAudioChain( void ) ;
		// 再生中オーディオチェーンに存在するか？
		bool IsValidOnAudioChain( void ) const ;
		// 全再生中のオーディオ音量を反映
		static void ReflectVolumesAllChain( void ) ;

		friend class SGLMediaPlayer ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// オーディオデータバッファ・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioBufferReader	: public ESLObject
	{
	public:
		class	StreamListener
		{
		public:
			virtual void OnBeginStreaming( SGLAudioBufferReader * pReader ) ;
			virtual void OnEndStreaming( SGLAudioBufferReader * pReader ) ;
			virtual void OnStreaming
				( SGLAudioBufferReader * pReader,
						const void * ptrData, size_t nBytes ) = 0 ;
		} ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioBufferReader, ESLObject )
		// サウンドフォーマットを取得する
		virtual SGLError GetAudioFormat( SGLSoundFormat & fmt ) = 0 ;
		// 静的バッファサイズ [byte] を取得する
		virtual size_t GetStaticBufferSize( void ) const = 0 ;
		// 静的バッファから読み出す
		virtual size_t ReadStaticBuffer
			( void * ptrBuf, size_t nPos, size_t nBytes ) const = 0 ;
		// ストリーミングリスナ設定
		virtual SGLError AttachStreamingListener( StreamListener * pListener ) = 0 ;
	} ;


}

#endif
