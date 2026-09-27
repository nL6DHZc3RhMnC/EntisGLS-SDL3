
#if	!defined(__SAKURAGL_MEI_MEDIA_MEDIA_PLAYER_H__)
#define	__SAKURAGL_MEI_MEDIA_MEDIA_PLAYER_H__	1

#include <sakuragl/media/sgl_mei_media_composer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// MEI ファイル再生インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLMEIMediaPlayer
				: public SGLMediaPlayerInterface,
					public ERISA::SGLMovieFilePlayer,
					public SSystem::SProcedure
	{
	protected:
		// サウンドストリーム用バッファ
		class	SoundStreamBuffer
					: public SGLSoundPlayerListener,
						public SSystem::SObjectArray< SSystem::SByteBuffer >
		{
		public:
			SSystem::SMutex *	m_pMutexUI ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SoundStreamBuffer, SGLSoundPlayerListener )
			// 構築関数
			SoundStreamBuffer( void ) ;
		public:
			// 追加
			size_t Add( SSystem::SByteBuffer * pBuf ) ;
			// 取得
			SSystem::SByteBuffer * GetAt( size_t nIndex ) const ;
			// 要素削除
			void RemoveAt( size_t nIndex ) ;
			// 全要素削除
			void RemoveAll( void ) ;
		public:
			// バッファへの出力タイミング
			void OnStreaming( SoundPlayer * player ) ;
		} ;

		// ステータス
		enum	Status
		{
			statusNothing,
			statusOpened,
			statusPlayed,
			statusPaused,
		} ;
		Status						m_status ;
		SSystem::SSmartReference<SSystem::SFileInterface>
									m_refFile ;

		// 表示先
		SGLAbstractWindow *			m_pWindow ;
		SGLImageRect				m_rectDstView ;
		uint64_t					m_nViewFlags ;

		// サウンド再生用
		SGLSoundPlayer				m_sndPlayer ;
//		SSystem::SObjectArray< SSystem::SByteBuffer >
//									m_queSoundBuffer ;
		SoundStreamBuffer			m_sndListener ;

		// スレッド
		SSystem::SCriticalSection	m_csSync ;
		bool						m_flagThreading ;
		uint64_t					m_nPlayFlags ;
		SSystem::SThread			m_threadPlayer ;
		SSystem::SSignalEvent		m_eventQuit ;

		// ループ
		bool						m_flagLoop ;
		int64_t						m_nLoopStart ;
		int64_t						m_nLoopEnd ;

		// 通知リスナ
		SGLMediaPlayerFrameNotification *	m_pListener ;

		// SGLMEIMediaInputStream
		SSystem::SSmartPointer<SGLMEIMediaInputStream>
									m_pMediaStream ;

		SSystem::SMutex *			m_pMutexUI ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLMEIMediaPlayer,
				SGLMediaPlayerInterface, SGLMovieFilePlayer, SProcedure )
		// 構築関数
		SGLMEIMediaPlayer( void ) ;
		// 消滅関数
		virtual ~SGLMEIMediaPlayer( void ) ;

	protected:
		// 再生スレッド開始
		void BeginPlayerThread( void ) ;
		// 再生スレッド終了
		void EndPlayerThread( void ) ;
		// SGLMEIMediaInputStream 取得
		SGLMEIMediaInputStream * GetMEIMediaStream( void ) ;

	public:	// SGLAudioPlayerInterface
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

	public:	// SGLMediaPlayerInterface
		// ビデオサイズを取得する
		virtual SGLError GetVideoSize( SGLSize& sizeVideo ) ;
		// 表示先を設定する
		virtual SGLError SetVideoView
			( SGLAbstractWindow* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
		// 現在のフレームを描画する
		virtual SGLError DrawVideo
			( SGLPaintContextInterface* pPaint,
				const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
		// メディア再生通知リスナ設定
		virtual SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) ;
		// ビデオストリーム取得
		virtual SGLVideoInputStream * GetVideoStream( void ) ;
		virtual void ReleaseVideoStream( SGLVideoInputStream * pStream ) ;

	protected:	// ERISA::SGLMovieFilePlayer
		// 音声出力要求
		virtual bool RequestWaveOut
			( uint32_t channels, uint32_t frequency, uint32_t bps ) ;
		// 音声出力終了
		virtual void CloseWaveOut( void ) ;
		// 音声データ出力
		virtual void PushWaveBuffer( const void * ptrWaveBuf, size_t nBytes ) ;

	public:
		// 音声ストリーミング開始
		virtual void BeginWaveStreaming( void ) ;
		// 音声ストリーミング終了
		virtual void EndWaveStreaming( void ) ;

	public:	// SProcedure
		// スレッド関数
		virtual void Run( void ) ;
	protected:
		// m_eventQuit 脱出判定つき Lock
		bool SystemLock( void ) ;

	} ;


}

#endif

