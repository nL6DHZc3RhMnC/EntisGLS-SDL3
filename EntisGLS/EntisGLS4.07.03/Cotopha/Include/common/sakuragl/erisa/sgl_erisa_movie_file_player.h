
#if	!defined(__SAKURAGL_ERISA_MOVIE_FILE_PLAYER_H__)
#define	__SAKURAGL_ERISA_MOVIE_FILE_PLAYER_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// MEI 動画ファイルストリーム再生オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLMovieFilePlayer	: public	ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMovieFilePlayer, ESLObject )
		// 構築関数
		SGLMovieFilePlayer( void ) ;
		// 消滅関数
		virtual ~SGLMovieFilePlayer( void ) ;

	protected:
		// レコード先読みオブジェクト
		class	PreloadBuffer	: public	SSystem::SByteBuffer
		{
		public:
			uint64_t	m_iFrameIndex ;
			uint64_t	m_ui64ChunkType ;
		public:
			// 構築関数
			PreloadBuffer( size_t nLength ) ;
			PreloadBuffer( const PreloadBuffer& src ) ;
		} ;

		// キーフレームポイント構造体
		struct	KeyPoint
		{
			uint64_t	m_iKeyFrame ;
			uint64_t	m_nSubSample ;
			uint64_t	m_nRecOffset ;

			// 構築関数
			KeyPoint( void ) { }
			KeyPoint( const KeyPoint & key )
				: m_iKeyFrame(key.m_iKeyFrame),
					m_nSubSample(key.m_nSubSample),
					m_nRecOffset(key.m_nRecOffset ) { }
			// 代入
			const KeyPoint & operator = ( const KeyPoint & key )
			{
				m_iKeyFrame = key.m_iKeyFrame ;
				m_nSubSample = key.m_nSubSample ;
				m_nRecOffset = key.m_nRecOffset ;
				return	*this ;
			}
		} ;

		// ERI メディアファイル
		bool						m_flagTopDown ;
		bool						m_flagWaveOutput ;
		bool						m_flagWaveStreaming ;
		SGLMediaFile				m_erif ;
		// 展開オブジェクト
		uint32_t					m_flagsDecode ;
		SGLImageDecoder				m_decoderImage ;
		SGLSoundDecoder				m_decoderSound ;
		SSystem::SSmartPointer<SGLDecodeBitStream>
									m_bstream ;
		// 画像バッファ
		enum	FrameType
		{
			typeOther		= -1,
			typeIntraFrame,				// 独立フレーム（I ピクチャ）
			typePredictionalFrame,		// 差分フレーム（P ピクチャ）
			typeBidirectionalFrame,		// 双差分フレーム（B ピクチャ）
		} ;
		enum	ImageBufferIndex
		{
			bufferIFrame,
			bufferPFrame,
			bufferBFrame,
			bufferFilter0,
			bufferFilter1,
			bufferCount,
		} ;
		uint64_t					m_iCurrentFrame ;	// 現在読み込んでいるサンプル数
		size_t						m_iDstBufIndex ;	// 直後フレームの指標
		ssize_t						m_nCacheBFrames ;	// 現在の先読みキューで
														// キャッシュされた B フレーム数
														// -1 の時には B フレームに
														// 対応していないフォーマット
		SakuraGL::SGLImageObject *	m_pDstImage[bufferCount] ;
		SakuraGL::SGLImageBuffer	m_bufDstImage[bufferCount] ;
		int64_t						m_iDstFrame[bufferCount] ;	// m_pDstImage に対応する
																// フレーム番号
		// 先読みキュー
		uint64_t					m_iPreloadFrame ;
		uint64_t					m_nPreloadWaveSamples ;
		size_t						m_nPreloadLimit ;
		SSystem::SObjectArray<PreloadBuffer>
									m_queueImage ;
		// 音声シーク用キーポイント配列
		SSystem::SArray<KeyPoint>	m_arrayKeyFrame ;
		SSystem::SArray<KeyPoint>	m_arrayKeyWave ;

	public:
		// アニメーションファイルを開く
		SSystem::SError OpenMovieFile
			( SSystem::SFileInterface * pFile,
				bool flagOwner = false,
				uint32_t flagsDecode = SGLImageDecoder::flagTopDown ) ;
		// アニメーションファイルを閉じる
		void Close( void ) ;

		// 先頭フレームへ移動
		SSystem::SError SeekToBegin( void ) ;
		// 次のフレームへ移動
		SSystem::SError SeekToNextFrame( size_t nSkipFrame = 0 ) ;
		// 指定のフレームに移動
		SSystem::SError SeekToFrame( uint64_t iFrameIndex ) ;
		// 指定のフレームはキーフレームか？
		bool IsKeyFrame( uint64_t iFrameIndex ) ;
		// 最適なフレームスキップ数を取得する
		size_t GetBestSkipFrames( uint64_t nCurrentTime ) ;

	protected:
		// 画像展開出力バッファ要求
		virtual SakuraGL::SGLImageObject * CreateImageBuffer
			( uint32_t format, uint32_t width, uint32_t height, uint32_t bpp ) ;
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

	protected:
		// フレームを展開する
		SSystem::SError DecodeFrame
			( PreloadBuffer * pFrame, uint32_t flagsDecode = 0 ) ;
		// パレットテーブルを適用する
		void ApplyPaletteTable( PreloadBuffer * pBuffer ) ;
		// 先読みバッファを取得する
		PreloadBuffer * GetPreloadBuffer( void ) ;
		// 先読みバッファに追加する
		void AddPreloadBuffer( PreloadBuffer * pBuffer ) ;
		// 指定のフレームが I, P, B ピクチャか判定する
		FrameType GetFrameBufferType( PreloadBuffer * pBuffer ) ;

	public:
		// SGLMediaFile オブジェクトを取得する
		const SGLMediaFile & GetMediaFile( void ) const ;
		// カレントフレームのインデックスを取得する
		uint64_t CurrentIndex( void ) const ;
		// カレントフレームの画像を取得
		SakuraGL::SGLImageObject * CurrentFrame( void ) const ;
		// パレットテーブル取得
		const SakuraGL::SGLPalette * GetPaletteEntries( void ) const ;
		// キーフレームを取得
		uint32_t GetKeyFrameCount( void ) const ;
		// 全フレーム数を取得
		uint64_t GetAllFrameCount( void ) const ;
		// 全アニメーション時間を取得
		uint64_t GetTotalTime( void ) const ;
		// フレーム番号から時間へ変換
		uint64_t FrameIndexToTime( uint64_t iFrameIndex ) const ;
		// 時間からフレーム番号へ変換
		uint64_t TimeToFrameIndex( uint64_t nMilliSec ) const ;

	protected:
		// 動画像ストリームを読み込む
		PreloadBuffer * LoadMovieStream( uint64_t & iCurrentFrame ) ;
		// キーフレームポイントを追加する
		void AddKeyPoint
			( SSystem::SArray<KeyPoint> & arrayKeyPoint, const KeyPoint & key ) ;
		// 指定のキーフレームを検索する
		KeyPoint * SearchKeyPoint
			( SSystem::SArray<KeyPoint> & arrayKeyPoint, uint64_t iKeyFrame ) ;
		// 指定のフレームにシークする
		void SeekKeyPoint
			( SSystem::SArray<KeyPoint> & arrayKeyPoint,
					uint64_t iFrame, uint64_t & iCurtrentFrame ) ;
		// 指定の音声データまでシークしてストリーミング出力する
		void SeekKeyWave
			( SSystem::SArray<KeyPoint> & arrayKeyPoint, uint64_t iFrame ) ;

	} ;

}

#endif
