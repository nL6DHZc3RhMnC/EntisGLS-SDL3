
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_DECODE_IMAGE_H__)
#define	__SAKURA_ERISA_DECODE_IMAGE_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// ERI 画像ファイル構造体
	//////////////////////////////////////////////////////////////////////////

	struct	ERI_FILE_HEADER
	{
		DWORD	dwVersion ;				// enum ERIFileVersion
		DWORD	dwContainedFlag ;		// complex enum ERIFileContainFlag
		DWORD	dwKeyFrameCount ;
		DWORD	dwFrameCount ;
		DWORD	dwAllFrameTime ;
	} ;

	enum	ERIFileVersion
	{
		eriFileStandardVersinon	= 0x00020100,
		eriFileEnhancedVersinon	= 0x00020200,
	} ;

	enum	ERIFileContainFlag
	{
		eriFileContainImage		= 0x00000001,
		eriFileContainAlpha		= 0x00000002,
		eriFileContainPalette	= 0x00000010,
		eriFileContainWave		= 0x00000100,
		eriFileContainSequence	= 0x00000200,
	} ;

	struct	ERI_INFO_HEADER
	{
		DWORD	dwVersion ;
		DWORD	fdwTransformation ;		// enum ERITransformation
		DWORD	dwArchitecture ;		// enum ERIArchitecture
		DWORD	fdwFormatType ;			// enum SakuraGL::SGLImageFormatFlag
		SDWORD	nImageWidth ;
		SDWORD	nImageHeight ;
		DWORD	dwBitsPerPixel ;
		DWORD	dwClippedPixel ;
		DWORD	dwSamplingFlag ;		// enum ERISamplingFlag
		SDWORD	dwQuantumizedBits[2] ;
		DWORD	dwAllottedBits[2] ;
		DWORD	dwBlockingDegree ;
		DWORD	dwLappedBlock ;
		DWORD	dwFrameTransform ;
		DWORD	dwFrameDegree ;
	} ;

	enum	ERITransformation
	{
		eriTransformationLossless	= 0x03020000,
		eriTransformationDCT		= 0x00000001,
		eriTransformationLOT		= 0x00000005,
		eriTransformationLOT_MSS	= 0x00000105,
	} ;

	enum	ERIArchitecture
	{
		eriArithmeticCode	= 32,
		eriRunlengthGamma	= 0xFFFFFFFF,
		eriRunlengthHuffman	= 0xFFFFFFFC,
		erisaNemesisCode	= 0xFFFFFFF0,
		erisaRunlengthGamma	= 0xFFFFFFFE,	// ※非可逆圧縮での RunlengthGamma
	} ;

	enum	ERISamplingFlag
	{
		eriSamplingYUV444	= 0x00040404,
		eriSamplingYUV422	= 0x00040202,
		eriSamplingYUV411	= 0x00040101,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像処理関数
	//////////////////////////////////////////////////////////////////////////

	// 加算（ARGB 加算・可逆圧縮差分復元用）
	SakuraGL::SGLError
		eriWrapAroundAddImageBuffer
			( const SakuraGL::SGLImageBuffer& imgDst,
				const SakuraGL::SGLImageBuffer& imgSrc,
				int xPos = 0, int yPos = 0,
				const SakuraGL::SGLImageRect * pSrcRect = NULL ) ;
	// 減算（ARGB 減算・可逆圧縮差分用）
	SakuraGL::SGLError
		eriWrapAroundSubImageBuffer
			( const SakuraGL::SGLImageBuffer& imgDst,
				const SakuraGL::SGLImageBuffer& imgSrc,
				int xPos = 0, int yPos = 0,
				const SakuraGL::SGLImageRect * pSrcRect = NULL ) ;
	// 半画素フィルタ
	SakuraGL::SGLError
		eriImageFilterHalf1111
			( const SakuraGL::SGLImageBuffer & bufDstImage,
				const SakuraGL::SGLImageBuffer & bufSrcImage ) ;


	//////////////////////////////////////////////////////////////////////////
	// 画像展開オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageDecoder	: public	ESLObject
	{
	public:
		typedef	void (SGLImageDecoder::*PTR_PROCEDURE)( void ) ;
		typedef	void (SGLImageDecoder::*PTR_RESTORE_FUNC)
						( uint8_t * ptrDstImage,
							const uint8_t * ptrSrcImage,
							size_t nDstWidth, size_t nDstHeight ) const ;
		typedef	void (SGLImageDecoder::*PTR_BLOCK_MATRIX)( int16_t * ptrVertBufLOT ) const ;
		typedef	void (SGLImageDecoder::*PTR_BLOCK_SCALING)
				( unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		typedef	void (SGLImageDecoder::*PTR_BLOCK_SCALING_LINE)
			( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
				unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		typedef	void (SGLImageDecoder::*PTR_MOVE_BLOCK)
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;

	protected:
		// 画像情報ヘッダ
		ERISA::ERI_INFO_HEADER		m_eihInfo ;

		// 展開用パラメータ
		size_t						m_nBlockSize ;			// ブロッキングサイズ
		size_t						m_nBlockArea ;			// ブロック面積
		size_t						m_nBlockSamples ;		// ブロックのサンプル数
		size_t						m_nChannelCount ;		// チャネル数
		size_t						m_nWidthInBlocks ;		// 画像の幅（ブロック数）
		size_t						m_nHeightInBlocks ;		// 画像の高さ（ブロック数）

		// 展開先ブロックパラメータ
		uint8_t *					m_ptrDstBlock ;			// 出力先アドレス
		int32_t						m_nDstLineBytes ;		// 出力先ライン長
		uint32_t					m_nDstPixelBytes ;		// 1 ピクセルのバイト数
		size_t						m_nDstWidth ;			// 出力先ブロック幅
		size_t						m_nDstHeight ;			// 出力先ブロック高
		uint32_t					m_flagsDecode ;			// 復号フラグ

		// 可逆展開用バッファ
		uint32_t					m_flagEnhancedMode ;	// 拡張モードフラグ
		SSystem::SArray<uint8_t>	m_bufOperations ;		// オペレーションテーブル
		uint8_t *					m_ptrOperations ;
		SSystem::SArray<int8_t>		m_bufColumn ;			// 列バッファ
		int8_t *					m_ptrColumnBuf ;
		SSystem::SArray<int8_t>		m_bufLine ;				// 行バッファ
		int8_t *					m_ptrLineBuf ;
		SSystem::SArray<int8_t>		m_bufDecode ;			// 展開バッファ
		int8_t *					m_ptrDecodeBuf ;
		SSystem::SArray<int8_t>		m_bufArrange ;			// 再配列用バッファ
		int8_t *					m_ptrArrangeBuf ;
		SSystem::SArray<uint32_t>	m_bufArrangeTable ;		// 再配列用テーブル
		uint32_t *					m_pArrangeTable[4] ;

		// 非可逆展開用バッファ
		size_t						m_nBlocksetCount ;		// サブブロック数
		SSystem::SArray<int16_t>	m_bufVertLOT ;			// 重複変換用バッファ
		SSystem::SArray<int16_t>	m_bufHorzLOT ;
		int16_t *					m_ptrVertBufLOT ;
		int16_t *					m_ptrHorzBufLOT ;
		SSystem::SArray<int16_t>	m_bufBlockset ;			// ブロックセットバッファ
		int16_t *					m_ptrBlocksetBuf[16] ;
		SSystem::SArray<int16_t>	m_bufMatrix ;
		int16_t *					m_ptrMatrixBuf ;
		SSystem::SArray<int16_t>	m_bufIQParam ;			// 逆量子化パラメータ
		SSystem::SArray<uint8_t>	m_bufIQParamTable ;
		int16_t *					m_ptrIQParamBuf ;
		uint8_t *					m_ptrIQParamTable ;

		SSystem::SArray<int8_t>		m_bufBlockLine ;		// ブロック行中間バッファ
		int8_t *					m_ptrBlockLineBuf ;
		int8_t *					m_ptrNextBlockBuf ;
		SSystem::SArray<int8_t>		m_bufSrcImage ;			// 入力画像信号バッファ
		int8_t *					m_ptrSrcImageBuf ;
		SSystem::SArray<int8_t>		m_bufYUVImage ;			// YUV 画像出力バッファ
		int8_t *					m_ptrYUVImage ;
		int32_t						m_nYUVLineBytes ;		// YUV 画像ライン長
		uint32_t					m_nYUVPixelBytes ;		// YUV ピクセルのバイト数
		uint8_t *					m_ptrRGBImage ;			// RGB 画像出力用バッファ
		int32_t						m_nRGBLineBytes ;		// RGB 画像ライン長
		uint32_t					m_nRGBPixelBytes ;		// RGB ピクセルのバイト数

		// 動き補償情報
		struct	MOVE_PREV_BLOCK
		{
			uint8_t *	pPrevFrame ;	// 直前フレームピクセル参照
			uint8_t *	pNextFrame ;	// 直後フレームピクセル参照
			ulong_ptr_t	flagPrevHalf ;	// 直前フレーム 0.5 pixel bit0:x, bit1:y
			ulong_ptr_t	flagNextHalf ;	// 直後フレーム 0.5 pixel bit0:x, bit1:y
		} ;
		SSystem::SArray<int8_t>		m_bufMovingVector ;		// 動き補償ベクトル
		int8_t *					m_ptrMovingVector ;
		SSystem::SArray<int8_t>		m_bufMoveVecFlags ;
		int8_t *					m_ptrMoveVecFlags ;
		SSystem::SArray<MOVE_PREV_BLOCK>
									m_bufMovePrevBlocks ;
		MOVE_PREV_BLOCK*			m_ptrMovePrevBlocks ;	// 参照ブロックへのポインタ
		MOVE_PREV_BLOCK*			m_ptrNextPrevBlocks ;
		SakuraGL::SGLImageInfo *	m_pPrevImageInf ;		// 直前フレームへの参照
		uint8_t *					m_pPrevImageBuf ;
		int32_t						m_nPrevLineBytes ;		// 直前フレームラインステップ
		size_t						m_iPrevFormat ;
		SakuraGL::SGLImageInfo *	m_pNextImageInf ;		// 直後フレームへの参照
		uint8_t *					m_pNextImageBuf ;
		int32_t						m_nNextLineBytes ;		// 直後フレームラインステップ
		size_t						m_iNextFormat ;

		SakuraGL::SGLImageInfo *	m_pFilterImageInf ;		// フィルタ処理用バッファ参照
		uint8_t *					m_pFilterImageBuf ;

		ERISA::ERINA_HUFFMAN_TREE *	m_pHuffmanTree ;		// ハフマン木
		ERISA::ERISA_PROB_MODEL *	m_pProbERISA ;			// 統計情報

		// 非同期用
		bool						m_flagAsyncDecoding ;
		SSystem::SArray<int8_t>		m_bufSrcImageNext ;		// 入力画像信号バッファ
		SSystem::SSignalEvent		m_sigReadySrcNext ;
		SSystem::SSignalEvent		m_sigDecodeSrcNext ;

		struct	ASYNC_DECODING_THREAD_PARAM
		{
			bool						flagExit ;
			size_t						countLoop ;
			SGLAbstractDecodeContext *	pContext ;
			size_t						bytesBuffer ;
			int8_t *					pSrcBuffer ;
			SSystem::SSignalEvent *		pReadyNext ;
			SSystem::SSignalEvent *		pDecodeNext ;
		} ;

		// 並列処理用
		struct	LOSSY_DCT_DECODING_LINE
		{
			size_t				nPosY ;
			int8_t *			ptrSrcData ;			// 入力信号配列
			int8_t *			ptrQParam ;				// 量子化パラメータ配列
			int16_t *			ptrIQParamBuf ;			// 逆量子化中間バッファ
			int16_t *			ptrBlocksetBuf[16] ;	// 逆 DCT 変換バッファ
			int8_t *			ptrYUVImage ;			// YUV 画像出力バッファ
			uint8_t	*			ptrRGBImage ;			// RGB 画像出力バッファ
			uint8_t *			ptrDstBlock ;			// 出力先アドレス
			MOVE_PREV_BLOCK *	pMovePrevBlock ;		// 動き補償情報
		} ;
		class	LossyDCTDecodingLine	: public LOSSY_DCT_DECODING_LINE
		{
		public:
			SSystem::SArray<int8_t>		m_bufSrcImage ;
			SSystem::SArray<int16_t>	m_bufIQParam ;
			SSystem::SArray<int16_t>	m_bufBlockset ;
			SSystem::SArray<int8_t>		m_bufYUVImage ;
		} ;
		SSystem::SObjectArray<LossyDCTDecodingLine>	m_aLossyDecInstance ;

		class	DecodeLossyLineProc	: public SSystem::SParallelProcedure
		{
		protected:
			SGLImageDecoder *			m_decoder ;

			uint32_t					m_flagsDecoding ;
			size_t						m_nPosY ;
			int8_t *					m_ptrQParam ;
			uint8_t *					m_ptrDstBlock ;
			MOVE_PREV_BLOCK *			m_ptrNextPrevBlocks ;

			SGLAbstractDecodeContext *	m_pContext ;
			PTR_BLOCK_SCALING_LINE		m_pfnScaling ;
			PTR_RESTORE_FUNC			m_pfnRestore ;

		public:
			// 構築関数
			DecodeLossyLineProc
				( SGLImageDecoder * decoder,
					uint32_t flagsDecoding,
					SGLAbstractDecodeContext * pContext,
					PTR_BLOCK_SCALING_LINE pfnScaling,
					PTR_RESTORE_FUNC pfnRestore ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
		} ;
		friend class DecodeLossyLineProc ;

	protected:
		static const PTR_PROCEDURE	m_pfnColorOperation[0x10] ;
		static const PTR_MOVE_BLOCK	m_pfnMoveBlockPFrame[2][4] ;
		static const PTR_MOVE_BLOCK	m_pfnMoveBlockBFrame[2][4] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageDecoder, ESLObject )
		// 構築関数
		SGLImageDecoder( void ) ;
		// 消滅関数
		virtual ~SGLImageDecoder( void ) ;

	public:
		enum	DecodingFlag
		{
			flagTopDown			= 0x0001,
			flagDifferential	= 0x0002,
			flagQuickDecode		= 0x0100,
			flagQualityDecode	= 0x0200,
			flagNoHalfFilter	= 0x0400,
			flagUseHalfFilter	= 0x0800,
			flagPreviewDecode	= 0x1000,
		} ;
		// 初期化（パラメータの設定）
		virtual SSystem::SError Initialize
					( const ERISA::ERI_INFO_HEADER & infhdr ) ;
		// 終了（メモリの解放など）
		virtual void Delete( void ) ;
		// 画像を展開
		virtual SSystem::SError DecodeImage
			( const SakuraGL::SGLImageInfo & infDstImage,
				uint8_t * pDstBuffer,
				ERISA::SGLDecodeBitStream & bstream,
				uint32_t flagsDecoding = flagTopDown ) ;
		// 直前フレームへの参照を設定
		void SetRefPreviousFrame
			( SakuraGL::SGLImageInfo * pPrevFrame,
					uint8_t * pPrevFrameBuf,
				SakuraGL::SGLImageInfo * pNextFrame = NULL,
							uint8_t * pNextFrameBuf = NULL ) ;
		// フィルタ処理された画像を受け取る
		SakuraGL::SGLImageInfo * GetFilteredImageInfo( void ) const
			{
				return	m_pFilterImageInf ;
			}
		uint8_t * GetFilteredImageBuffer( void ) const
			{
				return	m_pFilterImageBuf ;
			}
		// フィルタ処理画像を受け取るバッファを設定する
		void SetFilteredImageBuffer
			( SakuraGL::SGLImageInfo * pImageInf, uint8_t * pImageBuf ) ;

	protected:
		// 展開進行状況通知関数
		virtual SSystem::SError OnDecodedBlock
					( const SakuraGL::SGLImageRect & rect ) ;

	protected:
		// 可逆画像展開
		SSystem::SError DecodeLosslessImage
			( const SakuraGL::SGLImageInfo & infDstImage,
				uint8_t * pDstBuffer,
				ERISA::SGLDecodeBitStream & bstream,
				uint32_t flagsDecoding ) ;
		// アレンジテーブルの初期化
		void InitializeArrangeTable( void ) ;
		// オペレーション実行
		void PerformOperation
			( uint32_t nOpCode,
				size_t nAllBlockLines, int8_t * pNextLineBuf ) ;
		// カラーオペレーション関数群
		void ColorOperation0000( void ) ;
		void ColorOperation0101( void ) ;
		void ColorOperation0110( void ) ;
		void ColorOperation0111( void ) ;
		void ColorOperation1001( void ) ;
		void ColorOperation1010( void ) ;
		void ColorOperation1011( void ) ;
		void ColorOperation1101( void ) ;
		void ColorOperation1110( void ) ;
		void ColorOperation1111( void ) ;
		// グレイ画像／256色画像の出力
		void RestoreGray8( void ) ;
		// RGB 画像（15ビット）の出力
		void RestoreRGB16( void ) ;
		// RGB 画像の出力
		void RestoreRGB24( void ) ;
		void RestoreBGR24( void ) ;
		void RestoreRGB32( void ) ;
		void RestoreBGR32( void ) ;
		// RGBA 画像の出力
		void RestoreRGBA32( void ) ;
		void RestoreBGRA32( void ) ;
		// RGB 画像の差分出力
		void LL_RestoreDeltaRGB24( void ) ;
		void LL_RestoreDeltaBGR24( void ) ;
		// RGBA 画像の差分出力
		void LL_RestoreDeltaRGBA32( void ) ;
		void LL_RestoreDeltaBGRA32( void ) ;
		// 画像出力関数取得
		virtual PTR_PROCEDURE GetLLRestoreFunc
			( uint32_t formatImage,
				uint32_t nBitsPerPixel, uint32_t flagsDecode ) ;

	protected:
		// 非可逆画像展開
		SSystem::SError DecodeLossyImage
			( const SakuraGL::SGLImageInfo & infDstImage,
				uint8_t * pDstBuffer,
				ERISA::SGLDecodeBitStream & bstream,
				uint32_t flagsDecoding ) ;
		// 非同期符号デコードスレッド関数
		static void AsyncDecodingThreadProc( void * pInstance ) ;
		// ブロック単位での画面サイズを計算する
		void CalcImageSizeInBlocks( DWORD fdwTransformation ) ;
		// サンプリングテーブルの初期化
		void InitializeZigZagTable( void ) ;
		// 動きベクトルをセットアップする
		void SetupMovingVector( void ) ;
		// 逆量子化
		void ArrangeAndIQuantumize_atLine
			( int16_t *const* ppBlocksetBufs, int16_t * ptrIQParamBuf,
				const int8_t * ptrSrcData, const int8_t * ptrCoefficient ) const ;
		void ArrangeAndIQuantumize
			( const int8_t * ptrSrcData, const int8_t * ptrCoefficient ) const ;
		// 逆 DCT 変換
		void MatrixIDCT8x8_atLine( int16_t *const* ppBlocksetBufs ) const ;
		void MatrixIDCT8x8( int16_t * ptrVertBufLOT ) const ;
		// 逆 LOT 変換
		void MatrixILOT8x8( int16_t * ptrVertBufLOT ) const ;
		// 4:4:4 スケーリング (汎用)
		void BlockScaling444_atLine
			( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
				unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		void BlockScaling444
			( unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		// 4:1:1 スケーリング (DCT 独立フレーム)
		void BlockDCTScaling411_IFrame_atLine
			( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
				unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		void BlockDCTScaling411_IFrame
			( unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		// 4:1:1 スケーリング (DCT 差分フレーム)
		void BlockDCTScaling411_PFrame_atLine
			( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
				unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		void BlockDCTScaling411_PFrame
			( unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		// 4:1:1 スケーリング (汎用)
		void BlockLOTScaling411
			( unsigned int x, unsigned int y, uint32_t flagsDecode ) const ;
		// 中間画像バッファに 1 チャネル書き出す
		void StoreYUVImageChannelByte_atLine
			( int8_t * ptrYUVImage,
				size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		void StoreYUVImageChannelByte
			( size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		void StoreYUVImageChannelSByte_atLine
			( int8_t * ptrYUVImage,
				size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		void StoreYUVImageChannelSByte
			( size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		// 中間画像バッファに 1 チャネル書き出す（スケーリング）
		void StoreYUVImageChannelX2_atLine
			( int8_t * ptrYUVImage,
				size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		void StoreYUVImageChannelX2
			( size_t xBlock, size_t yBlock,
				size_t iChannel, const int16_t * pwSrcChannel ) const ;
		// 中間バッファを YUV から RGB 形式へ変換
		void ConvertImageYUVtoRGB_atLine
			( uint8_t * ptrRGBLine, int8_t * ptrYUVLine,
				size_t heightInBlockset, uint32_t flagsDecode ) const ;
		void ConvertImageYUVtoRGB
			( size_t heightInBlockset, uint32_t flagsDecode ) const ;
		// 動き補償を適用した上で画像を複製する
		void MoveImageAllBlockWithVector( void ) ;
		void MoveImageBlockLineWithVector( void ) ;
		void MoveImageWithVector_atLine
			( uint8_t * ptrRGBImage, MOVE_PREV_BLOCK * pNextPrevBlocks ) const ;
		// 前後フレームサンプリング（ゼロフィル）
		void FillZeroMoveIBlock0( uint8_t * pDstImage ) const ;
		// 前フレームサンプリング (RGB形式)
		void SamplingRGBMovePBlock0
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMovePBlock1
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMovePBlock2
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMovePBlock3
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		// 前フレームサンプリング (BGR形式)
		void SamplingBGRMovePBlock0
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMovePBlock1
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMovePBlock2
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMovePBlock3
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		static void FlipBlockPixelRGBtoBGR
					( uint8_t * pImage, int32_t pitchLine ) ;
		// 後フレームサンプリング (RGB形式)（出力先に合成）
		void SamplingRGBMoveBBlock0
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMoveBBlock1
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMoveBBlock2
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingRGBMoveBBlock3
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		// 後フレームサンプリング (BGR形式)（出力先に合成）
		void SamplingBGRMoveBBlock0
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMoveBBlock1
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMoveBBlock2
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		void SamplingBGRMoveBBlock3
			( uint8_t * pDstImage,
				const uint8_t * pSrcImage, int32_t pitchSrcLine ) const ;
		// グレイ画像の出力
		void LS_RestoreGray8
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		// RGB 画像の出力
		void LS_RestoreRGB24
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		void LS_RestoreBGR24
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		void LS_RestoreRGB32
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		void LS_RestoreBGR32
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		// RGBA 画像の出力
		void LS_RestoreRGBA32
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		void LS_RestoreBGRA32
			( uint8_t * ptrDstImage,
				const uint8_t * ptrSrcImage,
				size_t nDstWidth, size_t nDstHeight ) const ;
		// 画像出力関数取得
		virtual PTR_RESTORE_FUNC GetLSRestoreFunc
			( uint32_t formatImage,
				uint32_t nBitsPerPixel, uint32_t flagsDecode ) const ;

	} ;

}

#endif

