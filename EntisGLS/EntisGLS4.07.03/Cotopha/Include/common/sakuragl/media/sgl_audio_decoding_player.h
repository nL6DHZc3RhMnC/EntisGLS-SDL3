
#if	!defined(__SAKURAGL_MEDIA_AUDIO_DECODING_PLAYER_H__)
#define	__SAKURAGL_MEDIA_AUDIO_DECODING_PLAYER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// AudioPlayer 標準実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioDecodingPlayer
					: public SGLAudioPlayerInterface,
							public SGLAudioBufferReader,
							public SGLAudioInputStream,
							public SGLSoundPlayerListener
	{
	protected:
		uint64_t								m_flagOpened ;
		SSystem::SString						m_strFilePath ;
		SSystem::SSmartReference<SSystem::SFileInterface>
												m_refFile ;
		SSystem::SSmartPointer<SGLAudioDecoderInterface>
												m_decoder ;
		uint64_t								m_posNextDecode ;
		SGLSoundFormat							m_fmtPCM ;
		SGLAudioDecoderInterface::OptionalInfo	m_optAudio ;
		uint64_t								m_nPCMSamples ;
		SSystem::SCriticalSection				m_csSync ;
		SSystem::SArray<uint8_t>				m_bufPCM ;
		size_t									m_posStreamPCM ;
		SSystem::SArray<uint8_t>				m_bufFile ;
		SSystem::SMemoryReferenceFile			m_memfile ;
		size_t									m_sizeThreshold ;

		SGLAudioBufferReader::StreamListener *	m_pStreamingListener ;

		SSystem::SMutex *						m_pMutexUI ;

		SGLSoundPlayer	m_player ;
		float32_t		m_volumes[0x10] ;
		bool			m_flagPlayerReady ;
		bool			m_flagPlayed ;
		bool			m_flagPaused ;
		bool			m_flagLoop ;
		int64_t			m_nPlayingBase ;
		int64_t			m_nLastPlayingBase ;
		uint64_t		m_nAccWrittenSamples ;
		uint64_t		m_nLoopStart ;
		uint64_t		m_nLoopEnd ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO4
			( SGLAudioDecodingPlayer,
				SGLAudioPlayerInterface,
				SGLAudioBufferReader,
				SGLAudioInputStream, SGLSoundPlayerListener )
		// 構築関数
		SGLAudioDecodingPlayer( void ) ;
		// 消滅関数
		virtual ~SGLAudioDecodingPlayer( void ) ;
		// modeOpenAuto/modeOpenAutoStatic の場合の閾値を設定
		void SetMemorySizeThreashold( size_t threshold ) ;

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
		SGLError CreateReferenceTo( const SGLAudioDecodingPlayer& adp ) ;
	protected:
		// デコーダー生成後処理
		void ProcessAfterCreateDecoder( void ) ;
		// PCM 静的展開判定・処理
		SGLError NormalizeAudioStatic( uint64_t nFlags ) ;
	public:
		// ファイルを閉じる
		virtual SGLError Close( void ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// オーディオデバイスを準備する
		virtual SGLError PreapreSoundPlayer( void ) ;
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

	public:	// SGLAudioBufferReader 実装
		// サウンドフォーマットを取得する
		virtual SGLError GetAudioFormat( SGLSoundFormat & fmt ) ;
		// 静的バッファサイズ [byte] を取得する
		virtual size_t GetStaticBufferSize( void ) const ;
		// 静的バッファから読み出す
		virtual size_t ReadStaticBuffer
			( void * ptrBuf, size_t nPos, size_t nBytes ) const ;
		// ストリーミングリスナ設定
		virtual SGLError AttachStreamingListener( StreamListener * pListener ) ;

	public:	// SGLAudioInputStream 実装
		// メディア補助情報取得
		virtual SGLError GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf ) ;
		// オーディオストリーム全長取得（未定は-1）[samples]
		virtual int64_t GetAudioLength( void ) const ;
		// オーディオストリーム読み込み [samples]
		virtual size_t ReadAudio( void * ptrBuf, size_t nSamples ) ;
		// オーディオストリーム位置変更
		virtual SGLError SeekAudio( uint64_t nSamples ) ;

	public:	// SGLSoundPlayerListener 実装
		// バッファへの出力タイミング
		virtual void OnStreaming( SoundPlayer * player ) ;
	} ;


}

#endif

