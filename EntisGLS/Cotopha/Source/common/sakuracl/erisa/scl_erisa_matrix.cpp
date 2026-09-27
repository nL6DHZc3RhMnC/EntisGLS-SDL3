
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


// 定数テーブル
//////////////////////////////////////////////////////////////////////////////

const float32_t	ERISA::sclf_Half = 0.5F ;			// = 0.5
const float32_t	ERISA::sclf_2 = 2.0F ;				// = 2.0
float32_t		ERISA::sclf_CosPI4 ;				// = cos(pi/4)
float32_t		ERISA::sclf_2CosPI4 ;				// = 2*cos(pi/4)

//
// 行列係数配列 : k(n,i) = cos( (2*i+1) / (4*n) )
//
float32_t		ERISA::sclf_DCTofK2[2] ;			// = cos( (2*i+1) / 8 )
float32_t		ERISA::sclf_DCTofK4[4] ;			// = cos( (2*i+1) / 16 )
float32_t		ERISA::sclf_DCTofK8[8] ;			// = cos( (2*i+1) / 32 )
float32_t		ERISA::sclf_DCTofK16[16] ;			// = cos( (2*i+1) / 64 )
float32_t		ERISA::sclf_DCTofK32[32] ;			// = cos( (2*i+1) / 128 )
float32_t		ERISA::sclf_DCTofK64[64] ;			// = cos( (2*i+1) / 256 )
float32_t		ERISA::sclf_DCTofK128[128] ;		// = cos( (2*i+1) / 512 )
float32_t		ERISA::sclf_DCTofK256[256] ;		// = cos( (2*i+1) / 1024 )
float32_t		ERISA::sclf_DCTofK512[512] ;		// = cos( (2*i+1) / 2048 )
float32_t		ERISA::sclf_DCTofK1024[1024] ;		// = cos( (2*i+1) / 4096 )
float32_t		ERISA::sclf_DCTofK2048[2048] ;		// = cos( (2*i+1) / 8192 )

//
// 行列係数配列へのテーブル
//
float32_t *	ERISA::sclf_pMatrixDCTofK[ERISA::MAX_DCT_DEGREE] =
{
	NULL,
	&sclf_DCTofK2[0],
	&sclf_DCTofK4[0],
	&sclf_DCTofK8[0],
	&sclf_DCTofK16[0],
	&sclf_DCTofK32[0],
	&sclf_DCTofK64[0],
	&sclf_DCTofK128[0],
	&sclf_DCTofK256[0],
	&sclf_DCTofK512[0],
	&sclf_DCTofK1024[0],
	&sclf_DCTofK2048[0],
} ;

//
// LOT 変換用回転行列
//
ERISA::SFP_SIN_COS
	ERISA::sclf_RevolveParameter[ERISA::MAX_DCT_DEGREE+1][5*8] ;


// 行列テーブルの初期化
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfInitializeMatrix( void )
{
	//
	// 特殊条件の定数を準備
	//
	sclf_CosPI4 = (float32_t) cos( SSystem::PI * 0.25 ) ;
	sclf_2CosPI4 = 2.0F * sclf_CosPI4 ;
	//
	// 行列係数配列初期化
	//
	int	i ;
	for ( i = 1; i < MAX_DCT_DEGREE; i ++ )
	{
		int			n = (1 << i) ;
		float32_t *	pDCTofK = sclf_pMatrixDCTofK[i] ;
		double		nr = SSystem::PI / (4.0 * n) ;
		double		dr = nr + nr ;
		double		ir = nr ;
		//
		for ( int j = 0; j < n; j ++ )
		{
			pDCTofK[j] = (float32_t) cos( ir ) ;
			ir += dr ;
		}
	}
	//
	// 回転行列初期化
	//
	for ( i = 1; i <= MAX_DCT_DEGREE; i ++ )
	{
		ESLAssert( sclfGetRevolveParameterCount(i) <= 5*8 ) ;
		sclfMakeRevolveParameter( &sclf_RevolveParameter[i][0], i ) ;
	}
}

// 32 ビット浮動小数点配列を 16 ビット整数配列に変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfRoundR32ToWordArray
	( int16_t * ptrDst, ssize_t nStep,
			const float32_t * ptrSrc, size_t nCount )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	nCount
		REG LOAD	nStep
		REG LOAD	ptrDst
		REG LOAD	ptrSrc
		REG ALLOC	nCount8 : size_t
		REG ALLOC	nStepX3 : ssize_t
		REG ALLOC	nStepX4 : ssize_t
		REG ALLOC	mm(4) : int64
		ASSUME		r1 : int16_t*
		//
		sll			nStep, nStep, 1
		sll			nStepX3, nStep, 1
		add			nStepX3, nStep
		sll			nStepX4, nStep, 2
		move		acc, 0x07
		srl			nCount8, nCount, 3
		and			nCount, acc
		move		r1, ptrDst
		//
		.WHILE	(uint32) nCount8 != (uint32) #zero
			vmove		mm(0), [ptrSrc]
			vmove		mm(2), [ptrSrc + 0x10]
			add			ptrSrc, 0x20
			vcvt.f2w	mm(0), mm(0)
			vcvt.f2w	mm(2), mm(2)
			//
			store.int16	[r1],           mm(0)
			srl			mm(0), 16
			store.int16	[r1 + nStep],   mm(0)
			srl			mm(0), 16
			store.int16	[r1 + nStep*2], mm(0)
			srl			mm(0), 16
			store.int16	[r1 + nStepX3], mm(0)
			add			r1, nStepX4
			//
			store.int16	[r1],           mm(2)
			srl			mm(2), 16
			store.int16	[r1 + nStep],   mm(2)
			srl			mm(2), 16
			store.int16	[r1 + nStep*2], mm(2)
			srl			mm(2), 16
			store.int16	[r1 + nStepX3], mm(2)
			add			r1, nStepX4
			dec			nCount8
		.ENDW
		//
		.WHILE	(uint32) nCount != (uint32) #zero
			fload.32	mm(0), [ptrSrc]
			add			ptrSrc, 4
			vcvt.f2w	mm(0), mm(0)
			store.int16	[r1], mm(0)
			add			r1, nStep
			dec			nCount
		.ENDW
	}
#else
	if ( nCount != 0 )
	do
	{
		int	nValue = eslRoundR32ToInt( *ptrSrc ) + 0x8000 ;
		if ( (unsigned int) nValue > 0xFFFF )
		{
			nValue = ~(nValue >> 15) & 0xFFFF ;
		}
		*ptrDst = (int16_t) (nValue - 0x8000) ;
		//
		ptrDst += nStep ;
		++ ptrSrc ;
	}
	while ( -- nCount ) ;
#endif
}

// 16ビット符号あり整数を8ビット符号あり整数に変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclwConvertArraySWordToSByte
	( int8_t * ptrDst, const int16_t * ptrSrc, size_t nCount )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	nCount
		REG LOAD	ptrDst
		REG LOAD	ptrSrc
		REG ALLOC	countOdd : int64
		REG ALLOC	mm(8) : int64
		//
		move	countOdd, nCount
		move	acc, 0x07
		and		countOdd, acc
		srl		nCount, 3
		//
		.WHILE	(uint32) nCount != (uint32) #zero
			load.64		mm(0), [ptrSrc]
			load.64		mm(1), [ptrSrc + 0x08]
			pcvt.swb	mm(0), mm(1)
			store.64	[ptrDst], mm(0)
			dec			nCount
			add			ptrSrc, 8 * sizeof(int16_t)
			add			ptrDst, 8 * sizeof(int8_t)
		.ENDW
		//
		.WHILE	(uint32) countOdd != (uint32) #zero
			load.uint16	mm(0), [ptrSrc]
			pcvt.swb	mm(0), #zero
			store.int8	[ptrDst], mm(0)
			dec			countOdd
			add			ptrSrc, sizeof(int16_t)
			add			ptrDst, sizeof(int8_t)
		.ENDW
	}
#else
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	nSrc = ptrSrc[i] + 0x80 ;
		if ( (unsigned int) nSrc > 0xFF )
		{
			nSrc = ~(nSrc >> 31) & 0xFF ;
		}
		ptrDst[i] = (int8_t) (nSrc - 0x80) ;
	}
#endif
}

// 16ビット符号あり整数を8ビット符号なし整数に変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclwConvertArraySWordToByte
	( uint8_t * ptrDst, const int16_t * ptrSrc, size_t nCount )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	nCount
		REG LOAD	ptrDst
		REG LOAD	ptrSrc
		REG ALLOC	countOdd : int64
		REG ALLOC	mm(8) : int64
		//
		move	countOdd, nCount
		move	acc, 0x07
		and		countOdd, acc
		srl		nCount, 3
		//
		.WHILE	(uint32) nCount != (uint32) #zero
			load.64		mm(0), [ptrSrc]
			load.64		mm(1), [ptrSrc + 0x08]
			pcvt.uswb	mm(0), mm(1)
			store.64	[ptrDst], mm(0)
			dec			nCount
			add			ptrSrc, 8 * sizeof(int16_t)
			add			ptrDst, 8 * sizeof(int8_t)
		.ENDW
		//
		.WHILE	(uint32) countOdd != (uint32) #zero
			load.uint16	mm(0), [ptrSrc]
			pcvt.uswb	mm(0), #zero
			store.int8	[ptrDst], mm(0)
			dec			countOdd
			add			ptrSrc, sizeof(int16_t)
			add			ptrDst, sizeof(int8_t)
		.ENDW
	}
#else
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	nSrc = ptrSrc[i] ;
		if ( (unsigned int) nSrc > 0xFF )
		{
			nSrc = ~(nSrc >> 31) & 0xFF ;
		}
		ptrDst[i] = (uint8_t) nSrc ;
	}
#endif
}

// スカラ乗算
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfScalarMultiply
		( float32_t * ptrDst, float32_t fpScalar, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ptrDst[i] *= fpScalar ;
	}
}

// ベクトル乗算
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfVectorMultiply
	( float32_t * ptrDst, const float32_t * ptrSrc, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ptrDst[i] *= ptrSrc[i] ;
	}
}

// 2x2 回転行列
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfRevolve2x2
	( float32_t * ptrBuf1, float32_t * ptrBuf2,
		float32_t fpSin, float32_t fpCos, size_t nStep, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	r1 = *ptrBuf1 ;
		float32_t	r2 = *ptrBuf2 ;
		//
		float32_t	R1 = r1 * fpCos - r2 * fpSin ;
		float32_t	R2 = r1 * fpSin + r2 * fpCos ;
		//
		*ptrBuf1 = R1 ;
		*ptrBuf2 = R2 ;
		//
		ptrBuf1 += nStep ;
		ptrBuf2 += nStep ;
	}
}

// LOT 変換用回転パラメータを生成する
//////////////////////////////////////////////////////////////////////////////
const ERISA::SFP_SIN_COS * ERISA::sclfGetRevolveParameter( size_t nDegreeDCT )
{
	if ( nDegreeDCT <= ERISA::MAX_DCT_DEGREE )
	{
		return	&sclf_RevolveParameter[nDegreeDCT][0] ;
	}
	return	NULL ;
}

size_t ERISA::sclfGetRevolveParameterCount( size_t nDegreeDCT )
{
	size_t	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	size_t	lc = 1, n = nDegreeNum / 2 ;
	while ( n >= 8 )
	{
		n /= 8 ;
		++ lc ;
	}
	return	lc * 8 ;
}

void ERISA::sclfMakeRevolveParameter
		( ERISA::SFP_SIN_COS * pSinCos, size_t nDegreeDCT )
{
	int						nDegreeNum = 1 << nDegreeDCT ;
	int						nStep = 2 ;
	double					k = SSystem::PI / (nDegreeNum * 2) ;
	ERISA::SFP_SIN_COS *	ptrNextRev = pSinCos ;
	do
	{
		for ( size_t i = 0; i < 7; i ++ )
		{
			double	ws = 1.0 ;
			double	a = 0.0 ;
			for ( size_t j = 0; j < i; j ++ )
			{
				a += nStep ;
				ws = ws * ptrNextRev[j].fpSin
					+ ptrNextRev[j].fpCos * cos( a * k ) ;
			}
			double	r = atan2( ws, cos( (a + nStep) * k ) ) ;
			ptrNextRev[i].fpSin = (REAL32) sin( r ) ;
			ptrNextRev[i].fpCos = (REAL32) cos( r ) ;
		}
		ptrNextRev += 7 ;
		nStep *= 8 ;
	}
	while ( nStep < nDegreeNum ) ;
}

// 高速 LOT 変換事前処理
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastPLOT
	(
		float32_t *		ptrSrc,
		size_t			nDegreeDCT
	)
{
	size_t	i, nDegreeNum ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 偶数周波と奇数周波を合算する
	//
	for ( i = 0; i < nDegreeNum; i += 2 )
	{
		float32_t	r1, r2 ;
		r1 = ptrSrc[i] ;
		r2 = ptrSrc[i + 1] ;
		ptrSrc[i]     = r1 + r2 ;
		ptrSrc[i + 1] = r1 - r2 ;
	}
}

//
// 高速 LOT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastLOT
	(
		float32_t *			ptrDst,
		const float32_t *	ptrSrc1,
		const float32_t *	ptrSrc2,
		size_t				nDegreeDCT
	)
{
	size_t	i, nDegreeNum ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 重複化
	//
	for ( i = 0; i < nDegreeNum; i += 2 )
	{
		float32_t	r1, r2 ;
		r1 = ptrSrc2[i] ;
		r2 = ptrSrc1[i + 1] ;
		ptrDst[i]     = sclf_Half * (r1 + r2) ;
		ptrDst[i + 1] = sclf_Half * (r1 - r2) ;
	}
}

// 平面回転行列
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfOddGivensMatrix
	(
		float32_t *					ptrDst,
		const ERISA::SFP_SIN_COS *	ptrRevolve,
		size_t						nDegreeDCT
	)
{
	float32_t	r1, r2 ;
	size_t		i, j, k, nDegreeNum ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 奇数周波を回転操作
	//
	size_t	nStep, lc, index ;
	index = 1 ;
	nStep = 2 ;
	lc = (nDegreeNum / 2) / 8 ;
	for ( ; ; )
	{
		for ( i = 0; i < lc; i ++ )
		{
			k = i * (nStep * 8) + index ;
			for ( j = 0; j < 7; j ++ )
			{
				r1 = ptrDst[k] ;
				r2 = ptrDst[k + nStep] ;
				ptrDst[k] =
					r1 * ptrRevolve[j].fpCos - r2 * ptrRevolve[j].fpSin ;
				ptrDst[k + nStep] =
					r1 * ptrRevolve[j].fpSin + r2 * ptrRevolve[j].fpCos ;
				k += nStep ;
			}
		}
		ptrRevolve += 7 ;
		index += nStep * 7 ;
		nStep *= 8 ;
		if ( lc <= 8 )
		{
			break ;
		}
		lc /= 8 ;
	}
	k = index ;
	for ( j = 0; j < lc - 1; j ++ )
	{
		r1 = ptrDst[k] ;
		r2 = ptrDst[k + nStep] ;
		ptrDst[k] =
			r1 * ptrRevolve[j].fpCos - r2 * ptrRevolve[j].fpSin ;
		ptrDst[k + nStep] =
			r1 * ptrRevolve[j].fpSin + r2 * ptrRevolve[j].fpCos ;
		k += nStep ;
	}
}

// 逆変換平面回転行列
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfOddGivensInverseMatrix
	(
		float32_t *					ptrSrc,
		const ERISA::SFP_SIN_COS *	ptrRevolve,
		size_t						nDegreeDCT
	)
{
	float32_t	r1, r2 ;
	size_t		i, k, nDegreeNum ;
	ssize_t		j ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 奇数周波を回転操作
	//
	size_t	nStep, lc, index ;
	index = 1 ;
	nStep = 2 ;
	lc = (nDegreeNum / 2) / 8 ;
	for ( ; ; )
	{
		ptrRevolve += 7 ;
		index += nStep * 7 ;
		nStep *= 8 ;
		if ( lc <= 8 )
		{
			break ;
		}
		lc /= 8 ;
	}
	k = index + nStep * (lc - 2) ;
	for ( j = (ssize_t) lc - 2; j >= 0; j -- )
	{
		r1 = ptrSrc[k] ;
		r2 = ptrSrc[k + nStep] ;
		ptrSrc[k] =
			r1 * ptrRevolve[j].fpCos + r2 * ptrRevolve[j].fpSin ;
		ptrSrc[k + nStep] =
			r2 * ptrRevolve[j].fpCos - r1 * ptrRevolve[j].fpSin ;
		k -= nStep ;
	}
	for ( ; ; )
	{
		if ( lc > (nDegreeNum / 2) / 8 )
		{
			break ;
		}
		ptrRevolve -= 7 ;
		nStep /= 8 ;
		index -= nStep * 7 ;
		//
		for ( i = 0; i < lc; i ++ )
		{
			k = i * (nStep * 8) + index + nStep * 6 ;
			for ( j = 6; j >= 0; j -- )
			{
				r1 = ptrSrc[k] ;
				r2 = ptrSrc[k + nStep] ;
				ptrSrc[k] =
					r1 * ptrRevolve[j].fpCos + r2 * ptrRevolve[j].fpSin ;
				ptrSrc[k + nStep] =
					r2 * ptrRevolve[j].fpCos - r1 * ptrRevolve[j].fpSin ;
				k -= nStep ;
			}
		}
		lc *= 8 ;
	}
}

// 高速 LOT 逆変換事前処理
////////////////////////////////////////////////////////////////////////////// 
void ERISA::sclfFastIPLOT
	(
		float32_t *		ptrSrc,
		size_t			nDegreeDCT
	)
{
	float32_t	r1, r2 ;
	size_t		i, nDegreeNum ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 奇数周波と偶数周波の成分を分解
	//
	for ( i = 0; i < nDegreeNum; i += 2 )
	{
		r1 = ptrSrc[i] ;
		r2 = ptrSrc[i + 1] ;
		ptrSrc[i]     = sclf_Half * (r1 + r2) ;
		ptrSrc[i + 1] = sclf_Half * (r1 - r2) ;
	}
}

//
// 高速 LOT 逆変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastILOT
	(
		float32_t *			ptrDst,
		const float32_t *	ptrSrc1,
		const float32_t *	ptrSrc2,
		size_t				nDegreeDCT
	)
{
	float32_t	r1, r2 ;
	size_t		i, nDegreeNum ;
	nDegreeNum = (size_t) 1 << nDegreeDCT ;
	//
	// 逆重複化
	//
	for ( i = 0; i < nDegreeNum; i += 2 )
	{
		r1 = ptrSrc1[i] ;
		r2 = ptrSrc2[i + 1] ;
		ptrDst[i]     = r1 + r2 ;
		ptrDst[i + 1] = r1 - r2 ;
	}
}


// 階乗関数
//////////////////////////////////////////////////////////////////////////////
double ERISA::sclfFactorial( int i )
{
	if ( i <= 1 )
	{
		return	1.0 ;
	}
	double	f = i ;
	while ( -- i > 1 )
	{
		f *= i ;
	}
	return	f ;
}


// ０次第１種ベッセル関数
//////////////////////////////////////////////////////////////////////////////
double ERISA::sclfBesselI0( double z )
{
	// Ia(x) = Σm=0 ∞ { (x/2)^(2m+a) / (m!Γ(m+a+1)) }
	// I0(x) = Σm=0 ∞ { (x/2)^(2m) / (m!m!) }
	double	b = 1.0 ;
	for ( int m = 1; m < 20; m ++ )
	{
		double	f = sclfFactorial(m) ;
		b += pow( z*0.5, 2.0*m ) / (f * f) ;
	}
	return	b ;
}


// カイザー窓生成 [0,n]
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfGenerateKaiserWindow
	( float32_t * ptrWindow, float32_t fpAlpha,  size_t nSize )
{
	double	pia = PI * fpAlpha ;
	double	d = 1.0 / sclfBesselI0( pia ) ;
	double	r = 1.0 / nSize ;
	for ( size_t i = 0; i <= nSize; i ++ )
	{
		double	x = (2*i*r - 1.0) ;
		ptrWindow[i] =
			(float32_t) (sclfBesselI0( pia * sqrt(1.0 - x*x)) * d) ;
	}
}


// カイザー・ベッセル派生 (KBD) 窓生成 [0,(n-1)]
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfGenerateKaiserBesselDerivedWindow
	( float32_t * ptrKDB /*[0,n-1]*/,
		float32_t * ptrKaiser /*[0,n]*/,
		float32_t fpAlpha, size_t nSize /*=n*/ )
{
	sclfGenerateKaiserWindow( ptrKaiser, fpAlpha, nSize ) ;
	//
	double	s = 0.0 ;
	for ( size_t i = 0; i < nSize; i ++ )
	{
		s += ptrKaiser[i] ;
		ptrKDB[i] = (float32_t) s ;
	}
	s += ptrKaiser[nSize] ;
	//
	for ( size_t i = 0; i < nSize; i ++ )
	{
		ptrKDB[i] = (float32_t) (ptrKDB[i] / s) ;
	}
}


// RGB-YUV 色空間変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfConvertRGBtoYUV
	( float32_t * ptrBuf1, float32_t * ptrBuf2,
		float32_t * ptrBuf3, size_t nCount )
{
	static const double	rMatrixRGB2YUV[3][3] =
	{
		{
			7.0 / 24.0,		7.0 / 12.0,		1.0 / 8.0
		},
		{
			-1.0 / 6.0,		-1.0 / 3.0,		1.0 / 2.0
		},
		{
			17.0 / 36.0,	-7.0 / 18.0,	-1.0 / 12.0
		}
	} ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	fpBlue = ptrBuf1[i] ;
		float32_t	fpGreen = ptrBuf2[i] ;
		float32_t	fpRed = ptrBuf3[i] ;
		ptrBuf1[i] =
			(REAL32)(rMatrixRGB2YUV[0][0] * fpRed
					+ rMatrixRGB2YUV[0][1] * fpGreen
					+ rMatrixRGB2YUV[0][2] * fpBlue) ;
		ptrBuf2[i] =
			(REAL32)(rMatrixRGB2YUV[1][0] * fpRed
					+ rMatrixRGB2YUV[1][1] * fpGreen
					+ rMatrixRGB2YUV[1][2] * fpBlue) ;
		ptrBuf3[i] =
			(REAL32)(rMatrixRGB2YUV[2][0] * fpRed
					+ rMatrixRGB2YUV[2][1] * fpGreen
					+ rMatrixRGB2YUV[2][2] * fpBlue) ;
	}
}
