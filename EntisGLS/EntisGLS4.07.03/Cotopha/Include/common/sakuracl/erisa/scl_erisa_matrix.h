
/*****************************************************************************
                         E R I S A - L i b r a r y
 ----------------------------------------------------------------------------
    Copyright (C) 2000-2018 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SCL_ERISA_MATRIX_H__)
#define	__SCL_ERISA_MATRIX_H__	1

namespace	ERISA
{

	/*****************************************************************************
								DCT行列演算用定数
	 *****************************************************************************/

	enum	DCTDegreeLimit
	{
		MIN_DCT_DEGREE	= 2,
		MAX_DCT_DEGREE	= 12,
	} ;

	struct	SFP_SIN_COS
	{
		float32_t	fpSin ;
		float32_t	fpCos ;
	} ;

	struct	WFX_SIN_COS
	{
		int16_t		fxSin ;
		int16_t		fxCos ;
	} ;

	extern const float32_t	sclf_Half ;			// = 0.5
	extern const float32_t	sclf_2 ;			// = 2.0
	extern float32_t		sclf_CosPI4 ;		// = cos(pi/4)
	extern float32_t		sclf_2CosPI4 ;		// = 2*cos(pi/4)

	extern float32_t	sclf_DCTofK2[2] ;			// = cos( (2*i+1) / 8 )
	extern float32_t	sclf_DCTofK4[4] ;			// = cos( (2*i+1) / 16 )
	extern float32_t	sclf_DCTofK8[8] ;			// = cos( (2*i+1) / 32 )
	extern float32_t	sclf_DCTofK16[16] ;			// = cos( (2*i+1) / 64 )
	extern float32_t	sclf_DCTofK32[32] ;			// = cos( (2*i+1) / 128 )
	extern float32_t	sclf_DCTofK64[64] ;			// = cos( (2*i+1) / 256 )
	extern float32_t	sclf_DCTofK128[128] ;		// = cos( (2*i+1) / 512 )
	extern float32_t	sclf_DCTofK256[256] ;		// = cos( (2*i+1) / 1024 )
	extern float32_t	sclf_DCTofK512[512] ;		// = cos( (2*i+1) / 2048 )
	extern float32_t	sclf_DCTofK1024[1024] ;		// = cos( (2*i+1) / 4096 )
	extern float32_t	sclf_DCTofK2048[2048] ;		// = cos( (2*i+1) / 8192 )
	extern float32_t *	sclf_pMatrixDCTofK[MAX_DCT_DEGREE] ;
	extern SFP_SIN_COS	sclf_RevolveParameter[ERISA::MAX_DCT_DEGREE+1][5*8] ;

	extern const int16_t	sclw_Param_DCT8x8[8][8] ;
	extern const int16_t	sclw_Param_IDCT8x8[8][8] ;


	/*****************************************************************************
								  丸め・飽和関数
	 *****************************************************************************/

	// 32 ビット浮動小数点配列を 16 ビット整数配列に変換
	void sclfRoundR32ToWordArray
		( int16_t * ptrDst, ssize_t nStep,
				const float32_t * ptrSrc, size_t nCount ) ;
	/*
	// 8 ビット符号無し整数配列を 32 ビット浮動小数点配列に変換
	void eriConvertArrayByteToR32
		( float32_t * ptrDst, const BYTE * ptrSrc, size_t nCount ) ;
	// 8 ビット符号有り整数配列を 32 ビット浮動小数点配列に変換
	void eriConvertArraySByteToR32
		( float32_t * ptrDst, const SBYTE * ptrSrc, size_t nCount ) ;
	// 32 ビット浮動小数点配列を 8 ビット符号なし整数配列に変換
	void eriConvertArrayR32ToByte
		( BYTE * ptrDst, const float32_t * ptrSrc, size_t nCount ) ;
	// 32 ビット浮動小数点配列を 8 ビット符号あり整数配列に変換
	void eriConvertArrayR32ToSByte
		( SBYTE * ptrDst, const float32_t * ptrSrc, size_t nCount ) ;
	*/
	// 16ビット符号あり整数を8ビット符号あり整数に変換
	void sclwConvertArraySWordToSByte
		( int8_t * ptrDst, const int16_t * ptrSrc, size_t nCount ) ;
	// 16ビット符号あり整数を8ビット符号なし整数に変換
	void sclwConvertArraySWordToByte
		( uint8_t * ptrDst, const int16_t * ptrSrc, size_t nCount ) ;


	/*****************************************************************************
								 行列演算関数
	 *****************************************************************************/

	// 行列初期化
	void sclfInitializeMatrix( void ) ;

	// スカラ乗算
	void sclfScalarMultiply
		( float32_t * ptrDst, float32_t fpScalar, size_t nCount ) ;
	// ベクトル乗算
	void sclfVectorMultiply
		( float32_t * ptrDst, const float32_t * ptrSrc, size_t nCount ) ;

	// 2 点回転変換
	void sclfRevolve2x2
		( float32_t * ptrBuf1, float32_t * ptrBuf2,
			float32_t fpSin, float32_t fpCos,
			size_t nStep, size_t nCount ) ;

	// 高速 DCT 変換
	void sclfFastDCT
		(
			float32_t *		ptrDst,
			ssize_t			nDstInterval,
			float32_t *		ptrSrc,
			float32_t *		ptrWorkBuf,
			size_t			nDegreeDCT
		) ;
	// 高速逆 DCT 変換
	void sclfFastIDCT
		(
			float32_t *		ptrDst,
			float32_t *		ptrSrc,
			ssize_t			nSrcInterval,
			float32_t *		ptrWorkBuf,
			size_t			nDegreeDCT
		) ;

	// LOT 変換用回転行列を生成する
	const SFP_SIN_COS * sclfGetRevolveParameter( size_t nDegreeDCT ) ;
	size_t sclfGetRevolveParameterCount( size_t nDegreeDCT ) ;
	void sclfMakeRevolveParameter
			( SFP_SIN_COS * pSinCos, size_t nDegreeDCT ) ;

	// LOT 変換事前行列
	void sclfFastPLOT
		(
			float32_t *		ptrSrc,
			size_t			nDegreeDCT
		) ;
	// LOT 変換
	void sclfFastLOT
		(
			float32_t *			ptrDst,
			const float32_t *	ptrSrc1,
			const float32_t *	ptrSrc2,
			size_t				nDegreeDCT
		) ;

	// Givens 回転行列
	void sclfOddGivensMatrix
		(
			float32_t *			ptrDst,
			const SFP_SIN_COS *	ptrRevolve,
			size_t				nDegreeDCT
		) ;
	// 逆 Givens 回転行列
	void sclfOddGivensInverseMatrix
		(
			float32_t *			ptrSrc,
			const SFP_SIN_COS *	ptrRevolve,
			size_t				nDegreeDCT
		) ;

	// 逆 LOT 変換事前行列
	void sclfFastIPLOT
		(
			float32_t *		ptrSrc,
			size_t			nDegreeDCT
		) ;
	// 逆 LOT 変換
	void sclfFastILOT
		(
			float32_t *			ptrDst,
			const float32_t *	ptrSrc1,
			const float32_t *	ptrSrc2,
			size_t				nDegreeDCT
		) ;

	// 高速2次元 DCT 変換
	void sclfFastDCT8x8( float32_t * ptrDst ) ;
	void sclwFastDCT8x8( int16_t * ptrDst ) ;
	// 高速2次元逆 DCT 変換
	void sclfFastIDCT8x8( float32_t * ptrDst ) ;
	void sclwFastIDCT8x8( int16_t * ptrDst ) ;
	// 高速2次元 LOT 変換
	void sclfFastLOT8x8
		(
			float32_t *		ptrDst,
			float32_t *		ptrHorzCur,
			float32_t *		ptrVertCur
		) ;
	// 高速2次元逆 LOT 変換
	void sclfFastILOT8x8
		(
			float32_t *		ptrDst,
			float32_t *		ptrHorzCur,
			float32_t *		ptrVertCur
		) ;
	void sclwFastILOT8x8
		(
			int16_t *		ptrDst,
			int16_t *		ptrHorzCur,
			int16_t *		ptrVertCur
		) ;

	// 階乗関数
	double sclfFactorial( int i ) ;

	// ０次第１種ベッセル関数
	double sclfBesselI0( double z ) ;

	// カイザー窓生成 [0,n]
	void sclfGenerateKaiserWindow
		( float32_t * ptrWindow, float32_t fpAlpha, size_t nSize ) ;

	// カイザー・ベッセル派生 (KBD) 窓生成 [0,(n-1)]
	void sclfGenerateKaiserBesselDerivedWindow
		( float32_t * ptrKDB /*[0,n-1]*/,
			float32_t * ptrKaiser /*[0,n]*/,
			float32_t fpAlpha, size_t nSize /*=n*/ ) ;

	// RGB-YUV 色空間変換
	void sclfConvertRGBtoYUV
		( float32_t * ptrBuf1, float32_t * ptrBuf2,
			float32_t * ptrBuf3, size_t nCount ) ;


}

#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)

// ARMv7-A 以上で使用可能な関数

// 16ビット符号あり整数を8ビット符号あり整数に変換
extern "C" void ERISA_sclwConvertArraySWordToSByte_ARMv7A
	( int8_t * ptrDst, const int16_t * ptrSrc, size_t nCount ) ;
// 16ビット符号あり整数を8ビット符号なし整数に変換
extern "C" void ERISA_sclwConvertArraySWordToByte_ARMv7A
	( uint8_t * ptrDst, const int16_t * ptrSrc, size_t nCount ) ;

// 高速2次元逆 DCT 変換
extern "C" void ERISA_sclwFastIDCT8x8_ARMv7A
	( int16_t * ptrDst, const int16_t *	ptrParamIDCT8x8 ) ;
extern "C" void ERISA_sclwFastIDCT8x8_ARM_NEON
	( int16_t * ptrDst, const int16_t *	ptrParamIDCT8x8 ) ;

// 逆量子化・並び替え
extern "C" void ERISA_sclwShuffleVectorMul8x8_ARM_NEON
	( int16_t * ptrDstArray, uint32_t * ptrDstIndex,
		const int16_t * ptrSrcArray,
		const int16_t * ptrMulParams, int nCoefficient ) ;

// YUV 4:4:4, 4:1:1 サブブロック内部変換
extern "C" void ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A
	( uint8_t * ptrYUVBlock, ssize_t nYUVLineBytes, const int16_t * pwSrcCahnnel ) ;
extern "C" void ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A
	( int8_t * ptrYUVBlock, ssize_t nYUVLineBytes, const int16_t * pwSrcCahnnel ) ;
extern "C" void ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A
	( int8_t * ptrYUVBlock, ssize_t nYUVBlockBytes, ssize_t nYUVLineBytes, const int16_t * pwSrcCahnnel ) ;


// YUV->RGB 8x8 変換（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
extern "C" void ERISA_sclbConvertYUVtoRGB8x8_ARMv7A
	( uint8_t * ptrRGBLine,
		const int8_t * ptrYUVLine, size_t widthInPacked ) ;
extern "C" void ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON
	( uint8_t * ptrRGBLine,
		const int8_t * ptrYUVLine, size_t widthInPacked ) ;

// YUV->RGB 8x8 変換＆飽和加算（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
extern "C" void ERISA_sclbAddYUVtoRGB8x8_ARMv7A
	( uint8_t * ptrRGBLine,
		const int8_t * ptrYUVLine, size_t widthInPacked ) ;
extern "C" void ERISA_sclbAddYUVtoRGB8x8_ARM_NEON
	( uint8_t * ptrRGBLine,
		const int8_t * ptrYUVLine, size_t widthInPacked ) ;

// 動画 Pブロック・サンプリング関数
extern "C" void ERISA_Sampling16x16RGBMovePBlock1_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;
extern "C" void ERISA_Sampling16x16RGBMovePBlock2_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;
extern "C" void ERISA_Sampling16x16RGBMovePBlock3_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;

// 動画 Bブロック・サンプリング関数
extern "C" void ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;
extern "C" void ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;
extern "C" void ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;
extern "C" void ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) ;

// 画像半画素フィルタ
extern "C" void ERISA_ImageFilterHalf1111_ARMv7A
	( uint8_t * pDstImage, int32_t pitchDstLine,
		const uint8_t * pSrcImage, int32_t pitchSrcLine,
		size_t widthImage, size_t heightImage ) ;

// ベクトル配列の複製と最大・最小値の取得
extern "C" void ERISA_sclfMoveVertexAndCubeRange_ARM_NEON
	( SakuraGL::S3DVector4 * pvMaxMin,
		SakuraGL::S3DVector4 * pvDst,
		const SakuraGL::S3DVector4 * pvSrc, size_t nCount ) ;

#endif

#endif

