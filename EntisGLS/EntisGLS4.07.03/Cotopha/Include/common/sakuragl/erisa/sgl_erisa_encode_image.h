
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2015 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_ENCODE_IMAGE_H__)
#define	__SAKURA_ERISA_ENCODE_IMAGE_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// 画像圧縮オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageEncoder	: public ESLObject
	{
	protected:
		// 画像情報ヘッダ
		ERISA::ERI_INFO_HEADER		m_eihInfo ;

		// 圧縮パラメータ
		size_t						m_nBlockSize ;			// ブロッキングサイズ
		size_t						m_nBlockArea ;			// ブロック面積
		size_t						m_nBlockSamples ;		// ブロックのサンプル数
		size_t						m_nChannelCount ;		// チャネル数
		size_t						m_nWidthInBlocks ;		// 画像の幅（ブロック数）
		size_t						m_nHeightInBlocks ;		// 画像の高さ（ブロック数）

		// 入力ブロックパラメータ
		uint8_t *					m_ptrSrcBlock ;			// 入力元アドレス
		int32_t						m_nSrcLineBytes ;		// 入力元ライン長
		uint32_t					m_nSrcPixelBytes ;		// 1 ピクセルのバイト数
		size_t						m_nSrcWidth ;			// 入力元ブロック幅
		size_t						m_nSrcHeight ;			// 入力元ブロック高
		uint32_t					m_flagsEncode ;			// 符号化フラグ

		// 可逆圧縮用パラメータ
		int8_t *					m_ptrColumnBuf ;		// 列バッファ
		SSystem::SArray<int8_t>		m_bufColumn ;
		int8_t *					m_ptrLineBuf ;			// 行バッファ
		SSystem::SArray<int8_t>		m_bufLineBuf ;
		int8_t *					m_ptrEncodeBuf ;		// 圧縮バッファ
		SSystem::SArray<int8_t>		m_bufEncodeBuf ;
		int8_t *					m_ptrArrangeBuf ;		// 再配列用バッファ
		SSystem::SArray<int8_t>		m_bufArrangeBuf ;
		uint32_t *					m_pArrangeTable[4] ;	// 再配列用テーブル
		SSystem::SArray<uint32_t>	m_bufArrangeTable ;

		// 非可逆圧縮用バッファ
		size_t						m_nBlocksetCount ;		// サブブロック数
		float32_t *					m_ptrVertBufLOT ;		// 重複変換用バッファ
		SSystem::SArray<float32_t>	m_bufVertBufLOT ;
		float32_t *					m_ptrHorzBufLOT ;
		SSystem::SArray<float32_t>	m_bufHorzBufLOT ;
		float32_t *					m_ptrBlocksetBuf[36] ;	// ブロックセットバッファ
		SSystem::SArray<float32_t>	m_bufBlocksetBuf ;
		float32_t *					m_ptrMatrixBuf[16] ;	// 行列変換用バッファ
		SSystem::SArray<float32_t>	m_bufMatrixBuf ;
		float32_t *					m_pQuantumizeScale[2] ;	// 量子化テーブル
		SSystem::SArray<float32_t>	m_bufQuantumizeScale ;
		uint8_t *					m_pQuantumizeTable ;
		SSystem::SArray<uint8_t>	m_bufQuantumizeTable ;

		size_t						m_nMovingVector ;		// 動き補償ベクトルフラグ
		uint8_t *					m_pMoveVecFlags ;		// 動き補償ベクトルフラグ
		SSystem::SArray<uint8_t>	m_bufMoveVecFlags ;
		int8_t *					m_pMovingVector ;		// 動き補償ベクトル
		SSystem::SArray<int8_t>		m_bufMovingVector ;
		int							m_fPredFrameType ;		// フレームタイプ
		size_t						m_nIntraBlockCount ;	// イントラブロック数
		float32_t					m_fpDiffDeflectBlock ;	// 平均差分偏差値
		float32_t					m_fpMaxDeflectBlock ;	// 最大差分偏差値

		int8_t *					m_ptrCoefficient ;		// 量子化パラメータ出力バッファ
		SSystem::SArray<int8_t>		m_bufCoefficient ;
		uint8_t *					m_ptrImageDst ;			// 画像信号出力バッファ
		SSystem::SArray<uint8_t>	m_bufImageDst ;
		float32_t *					m_ptrSignalBuf ;
		SSystem::SArray<float32_t>	m_bufSignalBuf ;

		ERISA::ERINA_HUFFMAN_TREE *	m_pHuffmanTree ;		// ハフマン木
		ERISA::ERISA_PROB_MODEL *	m_pProbERISA ;			// 統計情報

	public:
		enum	EncodeFlag
		{
		//	flagTopDown			= 0x0001,
			flagDifferential	= 0x0002,
			flagNoMoveVector	= 0x0004,
			flagBestCmpr		= 0x0000,
			flagHighCmpr		= 0x0010,
			flagNormalCmpr		= 0x0020,
			flagLowCmpr			= 0x0030,
			flagCmprModeMask	= 0x0030,
		} ;
		enum	PresetParameter
		{
			ppClearQuality,
			ppHighQuality,
			ppAboveQuality,
			ppStandardQuality,
			ppBelowQuality,
			ppLowQuality,
			ppLowestQuality,
			ppCount,
		} ;
		enum	ParameterFlag
		{
			pfUseLoopFilter		= 0x0001,
		} ;
		struct	Parameter
		{
			uint32_t		m_nFlags ;				// enum ParameterFlag 組み合わせ
			float32_t		m_fpYScaleDC ;			// 輝度直流成分の量子化係数
			float32_t		m_fpCScaleDC ;			// 色差直流成分の量子化係数
			float32_t		m_fpYScaleLow ;			// 輝度低周波成分の量子化係数
			float32_t		m_fpCScaleLow ;			// 色差低周波成分の量子化係数
			float32_t		m_fpYScaleHigh ;		// 輝度高周波成分の量子化係数
			float32_t		m_fpCScaleHigh ;		// 色差高周波成分の量子化係数
			int				m_nYThreshold ;			// 輝度成分の閾値
			int				m_nCThreshold ;			// 色差成分の閾値
			int				m_nYLPFThreshold ;		// 輝度成分 LPF 指標
			int				m_nCLPFThreshold ;		// 色差成分 LPF 指標
			int				m_nAMDFThreshold ;		// フレーム間最大差分閾値
			float32_t		m_fpPFrameScale ;		// P フレームの量子化係数比
			float32_t		m_fpBFrameScale ;		// B フレームの量子化係数比
			size_t			m_nMaxFrameSize ;		// 最大バイト数
			size_t			m_nMinFrameSize	;		// 最小バイト数

			// 構築関数
			Parameter( void ) ;
			// プリセット値取得
			void LoadPresetParam
				( PresetParameter ppIndex, ERISA::ERI_INFO_HEADER & infhdr ) ;
		} ;
		typedef	void (SGLImageEncoder::*PTR_PROCEDURE)( void ) ;

	protected:
		Parameter	m_prmCmprOpt ;

		static const PTR_PROCEDURE	m_pfnColorOperation[0x10] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageEncoder, ESLObject )
		// 構築関数
		SGLImageEncoder( void ) ;
		// 消滅関数
		virtual ~SGLImageEncoder( void ) ;

	public:
		// 初期化（パラメータの設定）
		virtual SSystem::SError Initialize( const ERI_INFO_HEADER & infhdr ) ;
		// 終了（メモリの解放など）
		virtual void Delete( void ) ;
		// 画像を圧縮
		virtual SSystem::SError EncodeImage
			( const SakuraGL::SGLImageInfo & infSrcImage,
				const uint8_t * pSrcBuffer,
				ERISA::SGLEncodeBitStream & bstream,
				uint32_t flagsDecoding = flagNormalCmpr ) ;
		// 圧縮オプションを設定
		void SetCompressionParameter( const Parameter & prmCmprOpt ) ;

	protected:
		// 展開進行状況通知関数
		virtual SSystem::SError OnEncodedBlock( size_t line, size_t column ) ;

	public:
		// 動き補償パラメータを計算する
		SSystem::SError ProcessMovingVector
			( const SakuraGL::SGLImageBuffer & dstimg,
				const SakuraGL::SGLImageBuffer & previmg,
				int & nAbsMaxDiff,
				const SakuraGL::SGLImageBuffer * ppredimg = NULL ) ;
		// 動き補償パラメータをクリアする
		void ClearMovingVector( void ) ;
	protected:
		// 動き補償ベクトルを計算し、予測画像との差分を計算する
		int PredictMovingVector
			( const SakuraGL::SGLImageBuffer & dstimg,
				const SakuraGL::SGLImageBuffer & previmg,
				int xBlock, int yBlock, SakuraGL::SGLPoint * ptMoveVec,
				int & nAbsMaxDiff, double & fpDeflectBlock,
				const SakuraGL::SGLImageBuffer * ppredimg = NULL ) ;
		// 動き補償ベクトルを計算する
		void SearchMovingVector
			( SakuraGL::SGLImageBuffer & nextblock,
				SakuraGL::SGLImageBuffer & predblock,
				const SakuraGL::SGLImageBuffer & nextimg,
				const SakuraGL::SGLImageBuffer & predimg,
				int xBlock, int yBlock, SakuraGL::SGLPoint & ptMoveVec ) ;
	public:
		// 画像ブロックの二乗偏差（合計）を求める
		static long int CalcSumDeflectBlock( const SakuraGL::SGLImageBuffer & imgblock ) ;
		// 画像ブロックの差の二乗偏差（合計）を求める
		static long int CalcSumSqrDifferenceBlock
			( const SakuraGL::SGLImageBuffer & dstimg, const SakuraGL::SGLImageBuffer & srcimg ) ;
		// 画像ブロックの絶対差の合計を求める
		static long int CalcSumAbsDifferenceBlock
			( const SakuraGL::SGLImageBuffer & dstimg, const SakuraGL::SGLImageBuffer & srcimg ) ;
		// 2つの画像ブロックの 50% 合成画像を生成する
		static void BlendBlockHalfImage
			( const SakuraGL::SGLImageBuffer & dstimg,
					const SakuraGL::SGLImageBuffer & srcimg1,
					const SakuraGL::SGLImageBuffer & srcimg2 ) ;
		// 2つの画像の差分（飽和）と最大絶対差の取得
		static int MakeSubtractionBlock
			( const SakuraGL::SGLImageBuffer & dstimg,
					const SakuraGL::SGLImageBuffer & srcimg ) ;
		// 画像ブロックの輝度を半分にする
		static void MakeBlockValueHalf( const SakuraGL::SGLImageBuffer & imgblock ) ;

	protected:
		// 可逆圧縮
		SSystem::SError EncodeLosslessImage
			( const SakuraGL::SGLImageBuffer & imginf,
				ERISA::SGLEncodeBitStream & bstream, uint32_t flagsDecoding ) ;
		// サンプリングテーブルの初期化
		void InitializeSamplingTable( void ) ;
		// 差分処理
		void DifferentialOperation( size_t nAllBlockLines, int8_t * pNextLineBuf ) ;
		// オペレーションコードを取得
		uint32_t DecideOperationCode
			( uint32_t flagsDecode, size_t nAllBlockLines, int8_t * pNextLineBuf ) ;
		// カラーオペレーション関数群
		void ColorOperation0000( void ) ;
		void ColorOperation0001( void ) ;
		void ColorOperation0010( void ) ;
		void ColorOperation0011( void ) ;
		void ColorOperation0100( void ) ;
		void ColorOperation0101( void ) ;
		void ColorOperation0110( void ) ;
		void ColorOperation0111( void ) ;
		void ColorOperation1000( void ) ;
		void ColorOperation1001( void ) ;
		void ColorOperation1010( void ) ;
		void ColorOperation1011( void ) ;
		void ColorOperation1100( void ) ;
		void ColorOperation1101( void ) ;
		void ColorOperation1110( void ) ;
		void ColorOperation1111( void ) ;
		// グレイ画像のサンプリング
		void SamplingGray8( void ) ;
		// RGB 画像(15ビット)のサンプリング
		void SamplingRGB16( void ) ;
		// RGB 画像のサンプリング
		void SamplingRGB24( void ) ;
		// RGBA 画像のサンプリング
		void SamplingRGBA32( void ) ;
		// 画像をサンプリングする関数へのポインタを取得する
		virtual PTR_PROCEDURE GetLLSamplingFunc
			( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecode ) ;

	protected:
		// 非可逆圧縮
		SSystem::SError EncodeLossyImage
			( const SakuraGL::SGLImageBuffer & imginf,
				ERISA::SGLEncodeBitStream & bstream, uint32_t flagsDecoding ) ;
		// ブロック単位での画面サイズを計算する
		void CalcImageSizeInBlocks( DWORD fdwTransformation ) ;
		// サンプリングテーブルの初期化
		void InitializeZigZagTable( void ) ;
		// 量子化テーブルの生成
		void InitializeQuantumizeTable( double r = 1.0 ) ;
		// マクロブロックのサンプリング（4:4:4 形式）＆色空間変換
		void SamplingMacroBlock
			( int xBlock, int yBlock,
				int nLeftWidth, int nLeftHeight,
				int32_t nBlockStepAddr, uint8_t*& ptrSrcLineAddr,
				PTR_PROCEDURE pfnSamplingFunc ) ;
		// 半端領域に平均値を設定
		void FillBlockOddArea( uint32_t flagsDecoding ) ;
		// 4:4:4 スケーリング
		void BlockScaling444( void ) ;
		// 4:1:1 スケーリング
		void BlockScaling411( void ) ;
		// DCT 変換を施す
		void MatrixDCT8x8( void ) ;
		// LOT 変換を施す
		void MatrixLOT8x8( float32_t * ptrVertBufLOT ) ;
		// 量子化を施す
		void ArrangeAndQuantumize( int8_t * ptrCoefficient, uint32_t flagsDecoding ) ;
		// 画像をサンプリングする関数へのポインタを取得する
		virtual PTR_PROCEDURE GetLSSamplingFunc
			( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecoding ) ;
	} ;

}

#endif


