
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


// 高速 DCT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastDCT
	(
		float32_t *		ptrDst,
		ssize_t			nDstInterval,
		float32_t *		ptrSrc,
		float32_t *		ptrWorkBuf,
		size_t			nDegreeDCT
	)
{
	//
	// DCT 次数検証
	//
	ESLAssert( (nDegreeDCT >= ERISA::MIN_DCT_DEGREE)
				&& (nDegreeDCT <= ERISA::MAX_DCT_DEGREE) ) ;
	//
	if ( nDegreeDCT == ERISA::MIN_DCT_DEGREE )
	{
		//
		// 4次 DCT の時は特殊条件
		//////////////////////////////////////////////////////////////////////
		float32_t	r32Buf[4] ;
		//
		// 交差加減算
		//
		r32Buf[0] = ptrSrc[0] + ptrSrc[3] ;
		r32Buf[2] = ptrSrc[0] - ptrSrc[3] ;
		r32Buf[1] = ptrSrc[1] + ptrSrc[2] ;
		r32Buf[3] = ptrSrc[1] - ptrSrc[2] ;
		//
		// 前半 : A2 * DCT2
		//
		ptrDst[0]                = sclf_Half * (r32Buf[0] + r32Buf[1]) ;
		ptrDst[nDstInterval * 2] = sclf_CosPI4 * (r32Buf[0] - r32Buf[1]) ;
		//
		// 後半 : R2 * 2 * A2 * DCT2 * K2
		//
		r32Buf[2] = sclf_DCTofK2[0] * r32Buf[2] ;
		r32Buf[3] = sclf_DCTofK2[1] * r32Buf[3] ;
		//
		r32Buf[0] =                 r32Buf[2] + r32Buf[3] ;
		r32Buf[1] = sclf_2CosPI4 * (r32Buf[2] - r32Buf[3]) ;
		//
		r32Buf[1] -= r32Buf[0] ;
		//
		ptrDst[nDstInterval]     = r32Buf[0] ;
		ptrDst[nDstInterval * 3] = r32Buf[1] ;
	}
	else
	{
		//
		// 汎用 DCT 変換
		//////////////////////////////////////////////////////////////////////
		//              | I   J |
		// 交差加減算 = |       |
		//              | I  -J |
		size_t	i ;
		size_t	nDegreeNum = ((size_t) 1 << nDegreeDCT) ;
		size_t	nHalfDegree = (nDegreeNum >> 1) ;
		for ( i = 0; i < nHalfDegree; i ++ )
		{
			ptrWorkBuf[i] = ptrSrc[i] + ptrSrc[nDegreeNum - i - 1] ;
			ptrWorkBuf[i + nHalfDegree] =
							ptrSrc[i] - ptrSrc[nDegreeNum - i - 1] ;
		}
		//
		// 前半 DCT : A * DCT
		//
		ssize_t	nDstStep = (nDstInterval << 1) ;
		//
		sclfFastDCT
			( ptrDst, nDstStep,
					ptrWorkBuf, ptrSrc, (nDegreeDCT - 1) ) ;
		//
		// 後半 DCT-IV : R * 2 * A * DCT * K
		//
		float32_t *	pDCTofK = sclf_pMatrixDCTofK[nDegreeDCT - 1] ;
		ptrSrc = ptrWorkBuf + nHalfDegree ;
		ptrDst += nDstInterval ;
		//
		for ( i = 0; i < nHalfDegree; i ++ )
		{
			ptrSrc[i] *= pDCTofK[i] ;
		}
		//
		sclfFastDCT
			( ptrDst, nDstStep,
					ptrSrc, ptrWorkBuf, (nDegreeDCT - 1) ) ;
		//
		float32_t *	ptrNext = ptrDst ;
		for ( i = 0; i < nHalfDegree; i ++ )
		{
			*ptrNext += *ptrNext ;
			ptrNext += nDstStep ;
		}
		//
		ptrNext = ptrDst ;
		for ( i = 1; i < nHalfDegree; i ++ )
		{
			ptrNext[nDstStep] -= *ptrNext ;
			ptrNext += nDstStep ;
		}
	}
}

// 高速 IDCT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastIDCT
	(
		float32_t *		ptrDst,
		float32_t *		ptrSrc,
		ssize_t			nSrcInterval,
		float32_t *		ptrWorkBuf,
		size_t			nDegreeDCT
	)
{
	//
	// DCT 次数検証
	//
	ESLAssert( (nDegreeDCT >= ERISA::MIN_DCT_DEGREE)
				&& (nDegreeDCT <= ERISA::MAX_DCT_DEGREE) ) ;
	//
	if ( nDegreeDCT == ERISA::MIN_DCT_DEGREE )
	{
		//
		// 4次 DCT の時は特殊条件
		//////////////////////////////////////////////////////////////////////
		float32_t	r32Buf1[2] ;
		float32_t	r32Buf2[4] ;
		//
		// 偶数行 : IDCT2
		//
		r32Buf1[0] = ptrSrc[0] ;
		r32Buf1[1] = sclf_CosPI4 * ptrSrc[nSrcInterval * 2] ;
		//
		r32Buf2[0] = r32Buf1[0] + r32Buf1[1] ;
		r32Buf2[1] = r32Buf1[0] - r32Buf1[1] ;
		//
		// 奇数行 : R * 2 * A * DCT * K
		//
		r32Buf1[0] = sclf_DCTofK2[0] * ptrSrc[nSrcInterval] ;
		r32Buf1[1] = sclf_DCTofK2[1] * ptrSrc[nSrcInterval * 3] ;
		//
		r32Buf2[2] =                 r32Buf1[0] + r32Buf1[1] ;
		r32Buf2[3] = sclf_2CosPI4 * (r32Buf1[0] - r32Buf1[1]) ;
		//
		r32Buf2[3] -= r32Buf2[2] ;
		//
		// 交差加減算
		//
		ptrDst[0] = r32Buf2[0] + r32Buf2[2] ;
		ptrDst[3] = r32Buf2[0] - r32Buf2[2] ;
		ptrDst[1] = r32Buf2[1] + r32Buf2[3] ;
		ptrDst[2] = r32Buf2[1] - r32Buf2[3] ;
	}
	else
	{
		//
		// 汎用 IDCT 変換
		//////////////////////////////////////////////////////////////////////
		//
		// 偶数行 : IDCT
		//
		size_t	i ;
		size_t	nDegreeNum = ((size_t) 1 << nDegreeDCT) ;
		size_t	nHalfDegree = (nDegreeNum >> 1) ;
		ssize_t	nSrcStep = (nSrcInterval << 1) ;
		//
		sclfFastIDCT
			( ptrDst, ptrSrc,
					nSrcStep, ptrWorkBuf, (nDegreeDCT - 1) ) ;
		//
		// 奇数行 : R * 2 * A * DCT * K
		//
		float32_t *	pDCTofK = sclf_pMatrixDCTofK[nDegreeDCT - 1] ;
		float32_t *	pOddSrc = ptrSrc + nSrcInterval ;
		float32_t *	pOddDst = ptrDst + nHalfDegree ;
		//
		float32_t *	ptrNext = pOddSrc ;
		for ( i = 0; i < nHalfDegree; i ++ )
		{
			ptrWorkBuf[i] = *ptrNext * pDCTofK[i] ;
			ptrNext += nSrcStep ;
		}
		//
		sclfFastDCT
			( pOddDst, 1, ptrWorkBuf,
					(ptrWorkBuf + nHalfDegree), (nDegreeDCT - 1) ) ;
		//
		for ( i = 0; i < nHalfDegree; i ++ )
		{
			pOddDst[i] += pOddDst[i] ;
		}
		//
		for ( i = 1; i < nHalfDegree; i ++ )
		{
			pOddDst[i] -= pOddDst[i - 1] ;
		}
		//              | I   I |
		// 交差加減算 = |       |
		//              | J  -J |
		float32_t	r32Buf[4] ;
		size_t		nQuadDegree = (nHalfDegree >> 1) ;
		for ( i = 0; i < nQuadDegree; i ++ )
		{
			r32Buf[0] = ptrDst[i] + ptrDst[nHalfDegree + i] ;
			r32Buf[3] = ptrDst[i] - ptrDst[nHalfDegree + i] ;
			r32Buf[1] =
				ptrDst[nHalfDegree - 1 - i] + ptrDst[nDegreeNum - 1 - i] ;
			r32Buf[2] =
				ptrDst[nHalfDegree - 1 - i] - ptrDst[nDegreeNum - 1 - i] ;
			//
			ptrDst[i]                   = r32Buf[0] ;
			ptrDst[nHalfDegree - 1 - i] = r32Buf[1] ;
			ptrDst[nHalfDegree + i]     = r32Buf[2] ;
			ptrDst[nDegreeNum - 1 - i]  = r32Buf[3] ;
		}
	}
}


// 高速2次元DCT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastDCT8x8( float32_t * ptrDst )
{
	float32_t	rWork[8] ;
	float32_t	rTemp[64] ;
	size_t		i ;
	//
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastDCT( &rTemp[i], 8, ptrDst + i * 8, &rWork[0], 3 ) ;
	}
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastDCT( ptrDst + i, 8, &rTemp[i * 8], &rWork[0], 3 ) ;
	}
}


// 高速2次元逆DCT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastIDCT8x8( float32_t * ptrDst )
{
	float32_t	rWork[8] ;
	float32_t	rTemp[64] ;
	size_t		i ;
	//
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastIDCT( &rTemp[i * 8], ptrDst + i, 8, &rWork[0], 3 ) ;
	}
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastIDCT( ptrDst + i * 8, &rTemp[i], 8, &rWork[0], 3 ) ;
	}
}


// 高速2次元 DCT 変換
//////////////////////////////////////////////////////////////////////////////
const int16_t	ERISA::sclw_Param_DCT8x8[8][8] =
{
	{	 8192,   8192,   8192,   8192,   8192,   8192,   8192,   8192	},
	{	16069,  13623,   9102,   3196,  -3196,  -9102, -13623, -16069	},
	{	15137,   6270,  -6270, -15137, -15137,  -6270,   6270,  15137	},
	{	13623,  -3196, -16069,  -9102,   9102,  16069,   3196, -13623	},
	{	11585, -11585, -11585,  11585,  11585, -11585, -11585,  11585	},
	{	 9102, -16069,   3196,  13623, -13623,  -3196,  16069,  -9102	},
	{	 6270, -15137,  15137,  -6270,  -6270,  15137, -15137,   6270	},
	{	 3196,  -9102,  13623, -16069,  16069, -13623,   9102,  -3196	},
} ;

void ERISA::sclwFastDCT8x8( int16_t * ptrDst )
{
	int16_t	wTemp[64+8] ;
	int16_t *	pDst ;
	int16_t *	pSrc ;
	size_t		i, j ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, 2
			lea		edi, wTemp[0]
			mov		esi, ptrDst
			add		edi, 0FH
			and		edi, NOT 0FH
LOOP_BEGIN1:
				push	edi
				lea		ebx, ERISA::sclw_Param_DCT8x8
				mov		edx, 8
LOOP_BEGIN2:
					movdqu	xmm6, [ebx]
					movdqu	xmm0, [esi]
					movdqu	xmm2, [esi + 10H]
						pmaddwd	xmm0, xmm6
						pmaddwd	xmm2, xmm6
					movdqu	xmm4, [esi + 20H]
					;
					add		ebx, 16
						pmaddwd	xmm4, xmm6
					pshufd	xmm1, xmm0, 0EEH
					pshufd	xmm3, xmm2, 0EEH
					paddd	xmm0, xmm1
						movdqu	xmm1, [esi + 30H]
					paddd	xmm2, xmm3
						pshufd	xmm5, xmm4, 0EEH
						pmaddwd	xmm1, xmm6
						paddd	xmm4, xmm5
					;
					punpckldq	xmm0, xmm2
						pshufd	xmm3, xmm1, 0EEH
						paddd	xmm1, xmm3
					pshufd	xmm6, xmm0, 0EEH
						punpckldq	xmm4, xmm1
						pshufd		xmm7, xmm4, 0EEH
					punpcklqdq	xmm0, xmm4
					punpcklqdq	xmm6, xmm7
					paddd	xmm0, xmm6
					psrad	xmm0, 14
					packssdw	xmm0, xmm0
					movq	MMWORD PTR [edi], xmm0
					add		edi, 8 * 2
					;
					dec		edx
				jnz		LOOP_BEGIN2
				;
				pop		edi
				add		esi, 40H
				add		edi, 8
				dec		ecx
			jnz		LOOP_BEGIN1
			;
			mov		ecx, 2
			lea		esi, wTemp[0]
			mov		edi, ptrDst
			add		esi, 0FH
			and		esi, NOT 0FH
LOOP_BEGIN3:
				push	edi
				lea		ebx, ERISA::sclw_Param_DCT8x8
				mov		edx, 8
LOOP_BEGIN4:
					movdqu	xmm6, [ebx]
					movdqu	xmm0, [esi]
					movdqu	xmm2, [esi + 10H]
						pmaddwd	xmm0, xmm6
						pmaddwd	xmm2, xmm6
					movdqu	xmm4, [esi + 20H]
					;
					add		ebx, 16
						pmaddwd	xmm4, xmm6
					pshufd	xmm1, xmm0, 0EEH
					pshufd	xmm3, xmm2, 0EEH
					paddd	xmm0, xmm1
						movdqu	xmm1, [esi + 30H]
					paddd	xmm2, xmm3
						pshufd	xmm5, xmm4, 0EEH
						pmaddwd	xmm1, xmm6
						paddd	xmm4, xmm5
					;
					punpckldq	xmm0, xmm2
						pshufd	xmm3, xmm1, 0EEH
						paddd	xmm1, xmm3
					pshufd	xmm6, xmm0, 0EEH
						punpckldq	xmm4, xmm1
						pshufd		xmm7, xmm4, 0EEH
					punpcklqdq	xmm0, xmm4
					punpcklqdq	xmm6, xmm7
					paddd	xmm0, xmm6
					psrad	xmm0, 16
					packssdw	xmm0, xmm0
					movq	MMWORD PTR [edi], xmm0
					add		edi, 8 * 2
					;
					dec		edx
				jnz		LOOP_BEGIN4
				;
				pop		edi
				add		esi, 40H
				add		edi, 8
				dec		ecx
			jnz		LOOP_BEGIN3
		}
	}
	else
	{
	#endif
		pDst = &wTemp[0] ;
		pSrc = ptrDst ;
		for ( i = 0; i < 8; i ++ )
		{
			const int16_t *	pwIDCT = &sclw_Param_DCT8x8[0][0] ;
			for ( j = 0; j < 8; j ++ )
			{
				pDst[j << 3] =
					(int16_t) (((int32_t)pwIDCT[0] * pSrc[0]
								+ (int32_t)pwIDCT[1] * pSrc[1]
								+ (int32_t)pwIDCT[2] * pSrc[2]
								+ (int32_t)pwIDCT[3] * pSrc[3]
								+ (int32_t)pwIDCT[4] * pSrc[4]
								+ (int32_t)pwIDCT[5] * pSrc[5]
								+ (int32_t)pwIDCT[6] * pSrc[6]
								+ (int32_t)pwIDCT[7] * pSrc[7]) >> 14) ;
				pwIDCT += 8 ;
			}
			pSrc += 8 ;
			pDst ++ ;
		}
		pDst = ptrDst ;
		pSrc = &wTemp[0] ;
		for ( i = 0; i < 8; i ++ )
		{
			const int16_t *	pwIDCT = &sclw_Param_DCT8x8[0][0] ;
			for ( j = 0; j < 8; j ++ )
			{
				pDst[j << 3] =
					(int16_t) (((int32_t)pwIDCT[0] * pSrc[0]
								+ (int32_t)pwIDCT[1] * pSrc[1]
								+ (int32_t)pwIDCT[2] * pSrc[2]
								+ (int32_t)pwIDCT[3] * pSrc[3]
								+ (int32_t)pwIDCT[4] * pSrc[4]
								+ (int32_t)pwIDCT[5] * pSrc[5]
								+ (int32_t)pwIDCT[6] * pSrc[6]
								+ (int32_t)pwIDCT[7] * pSrc[7]) >> 16) ;
				pwIDCT += 8 ;
			}
			pSrc += 8 ;
			pDst ++ ;
		}
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	}
	#endif
}

// 高速2次元逆 DCT 変換
//////////////////////////////////////////////////////////////////////////////
const int16_t	ERISA::sclw_Param_IDCT8x8[8][8] =
{
	{	16385,  16069,  15137,  13623,  11585,   9103,   6271,   3197	},
	{	16385,  13623,   6271,  -3196, -11585, -16069, -15137,  -9102	},
	{	16385,   9103,  -6270, -16069, -11585,   3197,  15137,  13623	},
	{	16385,   3197, -15137,  -9102,  11585,  13623,  -6270, -16069	},
	{	16385,  -3196, -15137,   9103,  11585, -13623,  -6270,  16069	},
	{	16385,  -9102,  -6270,  16069, -11585,  -3196,  15137, -13623	},
	{	16385, -13623,   6271,   3197, -11585,  16069, -15137,   9103	},
	{	16385, -16069,  15137, -13623,  11585,  -9102,   6271,  -3196	},
} ;

void ERISA::sclwFastIDCT8x8( int16_t * ptrDst )
{
	int16_t	wTemp[64+8] ;
#if	defined(__COTOPHA__)
	asm
	{
		REG ALLOC	mm(8) : int64
		REG ALLOC	pSrc : int16*
		REG ALLOC	pParamIDCT : int16*
		REG ALLOC	pDst1 : int16*
		REG ALLOC	pDst2 : int16*
		REG ALLOC	nLoop1 : uint32
		REG ALLOC	nLoop2 : uint32
		move	nLoop1, 2
		move	pSrc, ptrDst
		lea		pDst1, wTemp[0]
		.REPEAT
			move	pDst2, pDst1
			move	pParamIDCT, ERISA::sclw_Param_IDCT8x8
			move	nLoop2, 8
			//
			.REPEAT
				load.64		mm(6), [pParamIDCT]
				load.64		mm(7), [pParamIDCT + 0x08]
				load.64		mm(0), [pSrc]
				load.64		mm(1), [pSrc + 0x08]
				load.64		mm(2), [pSrc + 0x10]
				load.64		mm(3), [pSrc + 0x18]
					pmadd.wd	mm(0), mm(6)
					pmadd.wd	mm(1), mm(7)
				load.64		mm(4), [pSrc + 0x20]
				load.64		mm(5), [pSrc + 0x28]
					pmadd.wd	mm(2), mm(6)
					pmadd.wd	mm(3), mm(7)
				//
				add			pParamIDCT, 0x10
					pmadd.wd	mm(4), mm(6)
					pmadd.wd	mm(5), mm(7)
				padd.d		mm(0), mm(1)
				padd.d		mm(2), mm(3)
					load.64		mm(1), [pSrc + 0x30]
					load.64		mm(3), [pSrc + 0x38]
					padd.d		mm(4), mm(5)
					pmadd.wd	mm(1), mm(6)
					pmadd.wd	mm(3), mm(7)
				//
				move		mm(6), mm(0)
				punpack.ldq	mm(0), mm(2)
				srl			mm(6), 32
				srl			mm(2), 32
					move		mm(7), mm(4)
					padd.d		mm(1), mm(3)
				punpack.ldq	mm(6), mm(2)
					punpack.ldq	mm(4), mm(1)
					srl			mm(1), 32
					srl			mm(7), 32
					punpack.ldq	mm(7), mm(1)
				padd.d		mm(0), mm(6)
				padd.d		mm(4), mm(7)
				//
				psra.d		mm(0), 14
				psra.d		mm(4), 14
				pcvt.sdw	mm(0), mm(4)
				store.64	[pDst2], mm(0)
				add			pDst2, 8 * sizeof(int16)
				//
				dec			nLoop2
			.UNTIL	(uint32) nLoop2 == (uint32) #zero
			//
			add		pSrc, 0x40
			add		pDst1, 0x08
			dec		nLoop1
		.UNTIL	(uint32) nLoop1 == (uint32) #zero
		//
		lea		pSrc, wTemp[0]
		move	pDst1, ptrDst
		.REPEAT
			move	pDst2, pDst1
			move	pParamIDCT, ERISA::sclw_Param_IDCT8x8
			move	nLoop2, 8
			//
			.REPEAT
				load.64		mm(6), [pParamIDCT]
				load.64		mm(7), [pParamIDCT + 0x08]
				load.64		mm(0), [pSrc]
				load.64		mm(1), [pSrc + 0x08]
				load.64		mm(2), [pSrc + 0x10]
				load.64		mm(3), [pSrc + 0x18]
					pmadd.wd	mm(0), mm(6)
					pmadd.wd	mm(1), mm(7)
				load.64		mm(4), [pSrc + 0x20]
				load.64		mm(5), [pSrc + 0x28]
					pmadd.wd	mm(2), mm(6)
					pmadd.wd	mm(3), mm(7)
				//
				add			pParamIDCT, pParamIDCT, 0x10
					pmadd.wd	mm(4), mm(6)
					pmadd.wd	mm(5), mm(7)
				padd.d		mm(0), mm(1)
				padd.d		mm(2), mm(3)
					load.64		mm(1), [pSrc + 0x30]
					load.64		mm(3), [pSrc + 0x38]
					padd.d		mm(4), mm(5)
					pmadd.wd	mm(1), mm(6)
					pmadd.wd	mm(3), mm(7)
				//
				move		mm(6), mm(0)
				punpack.ldq	mm(0), mm(2)
				srl			mm(6), 32
				srl			mm(2), 32
					move		mm(7), mm(4)
					padd.d		mm(1), mm(3)
				punpack.ldq	mm(6), mm(2)
					punpack.ldq	mm(4), mm(1)
					srl			mm(1), 32
					srl			mm(7), 32
					punpack.ldq	mm(7), mm(1)
				padd.d		mm(0), mm(6)
				padd.d		mm(4), mm(7)
				//
				psra.d		mm(0), 16
				psra.d		mm(4), 16
				pcvt.sdw	mm(0), mm(4)
				store.64	[pDst2], mm(0)
				add			pDst2, pDst2, 8 * sizeof(int16)
				//
				dec			nLoop2
			.UNTIL	(uint32) nLoop2 == (uint32) #zero
			//
			add		pSrc, 0x40
			add		pDst1, 0x08
			dec		nLoop1
		.UNTIL	(uint32) nLoop1 == (uint32) #zero
	}
#else
	int16_t *	pDst ;
	int16_t *	pSrc ;
	size_t		i, j ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, 2
			lea		edi, wTemp[0]
			mov		esi, ptrDst
			add		edi, 0FH
			and		edi, NOT 0FH
LOOP_BEGIN1:
				push	edi
				lea		ebx, ERISA::sclw_Param_IDCT8x8
				mov		edx, 8
LOOP_BEGIN2:
					movdqu	xmm6, [ebx]
					movdqu	xmm0, [esi]
					movdqu	xmm2, [esi + 10H]
						pmaddwd	xmm0, xmm6
						pmaddwd	xmm2, xmm6
					movdqu	xmm4, [esi + 20H]
					;
					add		ebx, 16
						pmaddwd	xmm4, xmm6
					pshufd	xmm1, xmm0, 0EEH
					pshufd	xmm3, xmm2, 0EEH
					paddd	xmm0, xmm1
						movdqu	xmm1, [esi + 30H]
					paddd	xmm2, xmm3
						pshufd	xmm5, xmm4, 0EEH
						pmaddwd	xmm1, xmm6
						paddd	xmm4, xmm5
					;
					punpckldq	xmm0, xmm2
						pshufd	xmm3, xmm1, 0EEH
						paddd	xmm1, xmm3
					pshufd	xmm6, xmm0, 0EEH
						punpckldq	xmm4, xmm1
						pshufd		xmm7, xmm4, 0EEH
					punpcklqdq	xmm0, xmm4
					punpcklqdq	xmm6, xmm7
					paddd	xmm0, xmm6
					psrad	xmm0, 14
					packssdw	xmm0, xmm0
					movq	MMWORD PTR [edi], xmm0
					add		edi, 8 * 2
					;
					dec		edx
				jnz		LOOP_BEGIN2
				;
				pop		edi
				add		esi, 40H
				add		edi, 8
				dec		ecx
			jnz		LOOP_BEGIN1
			;
			mov		ecx, 2
			lea		esi, wTemp[0]
			mov		edi, ptrDst
			add		esi, 0FH
			and		esi, NOT 0FH
LOOP_BEGIN3:
				push	edi
				lea		ebx, ERISA::sclw_Param_IDCT8x8
				mov		edx, 8
LOOP_BEGIN4:
					movdqu	xmm6, [ebx]
					movdqu	xmm0, [esi]
					movdqu	xmm2, [esi + 10H]
						pmaddwd	xmm0, xmm6
						pmaddwd	xmm2, xmm6
					movdqu	xmm4, [esi + 20H]
					;
					add		ebx, 16
						pmaddwd	xmm4, xmm6
					pshufd	xmm1, xmm0, 0EEH
					pshufd	xmm3, xmm2, 0EEH
					paddd	xmm0, xmm1
						movdqu	xmm1, [esi + 30H]
					paddd	xmm2, xmm3
						pshufd	xmm5, xmm4, 0EEH
						pmaddwd	xmm1, xmm6
						paddd	xmm4, xmm5
					;
					punpckldq	xmm0, xmm2
						pshufd	xmm3, xmm1, 0EEH
						paddd	xmm1, xmm3
					pshufd	xmm6, xmm0, 0EEH
						punpckldq	xmm4, xmm1
						pshufd		xmm7, xmm4, 0EEH
					punpcklqdq	xmm0, xmm4
					punpcklqdq	xmm6, xmm7
					paddd	xmm0, xmm6
					psrad	xmm0, 16
					packssdw	xmm0, xmm0
					movq	MMWORD PTR [edi], xmm0
					add		edi, 8 * 2
					;
					dec		edx
				jnz		LOOP_BEGIN4
				;
				pop		edi
				add		esi, 40H
				add		edi, 8
				dec		ecx
			jnz		LOOP_BEGIN3
		}
	}
	else
	{
	#endif
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		ERISA_sclwFastIDCT8x8_ARMv7A
			( ptrDst, &ERISA::sclw_Param_IDCT8x8[0][0] ) ;
	#else
		pDst = &wTemp[0] ;
		pSrc = ptrDst ;
		for ( i = 0; i < 8; i ++ )
		{
			const int16_t *	pwIDCT = &sclw_Param_IDCT8x8[0][0] ;
			for ( j = 0; j < 8; j ++ )
			{
				pDst[j << 3] =
					(int16_t) (((int32_t)pwIDCT[0] * pSrc[0]
								+ (int32_t)pwIDCT[1] * pSrc[1]
								+ (int32_t)pwIDCT[2] * pSrc[2]
								+ (int32_t)pwIDCT[3] * pSrc[3]
								+ (int32_t)pwIDCT[4] * pSrc[4]
								+ (int32_t)pwIDCT[5] * pSrc[5]
								+ (int32_t)pwIDCT[6] * pSrc[6]
								+ (int32_t)pwIDCT[7] * pSrc[7]) >> 14) ;
				pwIDCT += 8 ;
			}
			pSrc += 8 ;
			pDst ++ ;
		}
		pDst = ptrDst ;
		pSrc = &wTemp[0] ;
		for ( i = 0; i < 8; i ++ )
		{
			const int16_t *	pwIDCT = &sclw_Param_IDCT8x8[0][0] ;
			for ( j = 0; j < 8; j ++ )
			{
				pDst[j << 3] =
					(int16_t) (((int32_t)pwIDCT[0] * pSrc[0]
								+ (int32_t)pwIDCT[1] * pSrc[1]
								+ (int32_t)pwIDCT[2] * pSrc[2]
								+ (int32_t)pwIDCT[3] * pSrc[3]
								+ (int32_t)pwIDCT[4] * pSrc[4]
								+ (int32_t)pwIDCT[5] * pSrc[5]
								+ (int32_t)pwIDCT[6] * pSrc[6]
								+ (int32_t)pwIDCT[7] * pSrc[7]) >> 16) ;
				pwIDCT += 8 ;
			}
			pSrc += 8 ;
			pDst ++ ;
		}
	#endif
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	}
	#endif
#endif
}


// 高速2次元 LOT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastLOT8x8
	(
		float32_t *		ptrDst,
		float32_t *		ptrHorzCur,
		float32_t *		ptrVertCur
	)
{
	static const ERISA::SFP_SIN_COS	escRev[3] =
	{
		{	0.734510F, 0.678598F	},
		{	0.887443F, 0.460917F	},
		{	0.970269F, 0.242030F	}
	} ;
	float32_t	rWork[8] ;
	float32_t	rTemp[64] ;
	float32_t	s1, s2, r1, r2, r3 ;
	size_t		i, j, k ;
	//
	// 2次元 8x8 DCT 変換
	//
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastDCT( &rTemp[i], 8, ptrDst + i * 8, &rWork[0], 3 ) ;
	}
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastDCT( ptrDst + i, 8, &rTemp[i * 8], &rWork[0], 3 ) ;
	}
	//
	// 水平方向重複変換
	//
	for ( i = 0; i < 64; i += 8 )
	{
		for ( j = 0; j < 8; j += 2 )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 1] ;
			r1 = s1 + s2 ;
			r2 = s1 - s2 ;
			//
			r3 = ptrHorzCur[k + 1] ;
			ptrHorzCur[k]     = r1 ;
			ptrHorzCur[k + 1] = r2 ;
			ptrDst[k]     = sclf_Half * (r1 + r3) ;
			ptrDst[k + 1] = sclf_Half * (r1 - r3) ;
		}
		for ( j = 0, k = i + 1; j < 3; j ++, k += 2 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 2] ;
			ptrDst[k]     = r1 * escRev[j].fpCos - r2 * escRev[j].fpSin ;
			ptrDst[k + 2] = r1 * escRev[j].fpSin + r2 * escRev[j].fpCos ;
		}
	}
	//
	// 垂直方向重複変換
	//
	for ( i = 0; i < 64; i += 16 )
	{
		for ( j = 0; j < 8; j ++ )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 8] ;
			r1 = s1 + s2 ;
			r2 = s1 - s2 ;
			//
			r3 = ptrVertCur[k + 8] ;
			ptrVertCur[k]     = r1 ;
			ptrVertCur[k + 8] = r2 ;
			ptrDst[k]     = sclf_Half * (r1 + r3) ;
			ptrDst[k + 8] = sclf_Half * (r1 - r3) ;
		}
	}
	for ( i = 0; i < 8; i ++ )
	{
		for ( j = 0, k = i + 8; j < 3; j ++, k += 16 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 16] ;
			ptrDst[k]      = r1 * escRev[j].fpCos - r2 * escRev[j].fpSin ;
			ptrDst[k + 16] = r1 * escRev[j].fpSin + r2 * escRev[j].fpCos ;
		}
	}
}


// 高速2次元逆 LOT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclfFastILOT8x8
	(
		float32_t *		ptrDst,
		float32_t *		ptrHorzCur,
		float32_t *		ptrVertCur
	)
{
	static const ERISA::SFP_SIN_COS	sfpscRev[3] =
	{
		{	0.734510F, 0.678598F	},
		{	0.887443F, 0.460917F	},
		{	0.970269F, 0.242030F	}
	} ;
	float32_t	rWork[8] ;
	float32_t	rTemp[64] ;
	float32_t	s1, s2, r1, r2, r3 ;
	size_t		i, k ;
	ssize_t		j ;
	//
	// 垂直方向重複変換
	//
	for ( i = 0; i < 8; i ++ )
	{
		for ( j = 2, k = i + 40; j >= 0; j --, k -= 16 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 16] ;
			ptrDst[k]      = r1 * sfpscRev[j].fpCos + r2 * sfpscRev[j].fpSin ;
			ptrDst[k + 16] = r2 * sfpscRev[j].fpCos - r1 * sfpscRev[j].fpSin ;
		}
	}
	for ( i = 0; i < 64; i += 16 )
	{
		for ( j = 0; j < 8; j ++ )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 8] ;
			r1 = sclf_Half * (s1 + s2) ;
			r2 = sclf_Half * (s1 - s2) ;
			//
			r3 = ptrVertCur[k] ;
			ptrVertCur[k]     = r1 ;
			ptrVertCur[k + 8] = r2 ;
			ptrDst[k]     = r3 + r2 ;
			ptrDst[k + 8] = r3 - r2 ;
		}
	}
	//
	// 水平方向重複変換
	//
	for ( i = 0; i < 64; i += 8 )
	{
		for ( j = 2, k = i + 5; j >= 0; j --, k -= 2 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 2] ;
			ptrDst[k]     = r1 * sfpscRev[j].fpCos + r2 * sfpscRev[j].fpSin ;
			ptrDst[k + 2] = r2 * sfpscRev[j].fpCos - r1 * sfpscRev[j].fpSin ;
		}
		for ( j = 0; j < 8; j += 2 )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 1] ;
			r1 = sclf_Half * (s1 + s2) ;
			r2 = sclf_Half * (s1 - s2) ;
			//
			r3 = ptrHorzCur[k] ;
			ptrHorzCur[k]     = r1 ;
			ptrHorzCur[k + 1] = r2 ;
			ptrDst[k]     = r3 + r2 ;
			ptrDst[k + 1] = r3 - r2 ;
		}
	}
	//
	// 2次元 8x8 IDCT 変換
	//
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastIDCT( &rTemp[i * 8], ptrDst + i, 8, &rWork[0], 3 ) ;
	}
	for ( i = 0; i < 8; i ++ )
	{
		sclfFastIDCT( ptrDst + i * 8, &rTemp[i], 8, &rWork[0], 3 ) ;
	}
}


// 高速2次元逆 LOT 変換
//////////////////////////////////////////////////////////////////////////////
void ERISA::sclwFastILOT8x8
	(
		int16_t *		ptrDst,
		int16_t *		ptrHorzCur,
		int16_t *		ptrVertCur
	)
{
	static const ERISA::WFX_SIN_COS	wfxscRev[3] =
	{
		{	12034, 11118	},		// fixed x4000H
		{	14540, 7552		},		// fixed x4000H
		{	15897, 3965		},		// fixed x4000H
	} ;
	int32_t	s1, s2, r1, r2, r3 ;
	size_t	i, k ;
	ssize_t	j ;
	//
	// 垂直方向重複変換
	//
	for ( i = 0; i < 8; i ++ )
	{
		for ( j = 2, k = i + 40; j >= 0; j --, k -= 16 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 16] ;
			ptrDst[k]      = (int16_t) ((r1 * wfxscRev[j].fxCos
										+ r2 * wfxscRev[j].fxSin) >> 14) ;
			ptrDst[k + 16] = (int16_t) ((r2 * wfxscRev[j].fxCos
										- r1 * wfxscRev[j].fxSin) >> 14) ;
		}
	}
	for ( i = 0; i < 64; i += 16 )
	{
		for ( j = 0; j < 8; j ++ )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 8] ;
			r1 = s1 + s2 ;
			r2 = s1 - s2 ;
			//
			r3 = ptrVertCur[k] ;
			ptrVertCur[k]     = (int16_t) r1 ;
			ptrVertCur[k + 8] = (int16_t) r2 ;
			ptrDst[k]     = (int16_t) ((r3 + r2) >> 1) ;
			ptrDst[k + 8] = (int16_t) ((r3 - r2) >> 1) ;
		}
	}
	//
	// 水平方向重複変換
	//
	for ( i = 0; i < 64; i += 8 )
	{
		for ( j = 2, k = i + 5; j >= 0; j --, k -= 2 )
		{
			r1 = ptrDst[k] ;
			r2 = ptrDst[k + 2] ;
			ptrDst[k]     = (int16_t) ((r1 * wfxscRev[j].fxCos
										+ r2 * wfxscRev[j].fxSin) >> 14) ;
			ptrDst[k + 2] = (int16_t) ((r2 * wfxscRev[j].fxCos
										- r1 * wfxscRev[j].fxSin) >> 14) ;
		}
		for ( j = 0; j < 8; j += 2 )
		{
			k = i + j ;
			s1 = ptrDst[k] ;
			s2 = ptrDst[k + 1] ;
			r1 = s1 + s2 ;
			r2 = s1 - s2 ;
			//
			r3 = ptrHorzCur[k] ;
			ptrHorzCur[k]     = (int16_t) r1 ;
			ptrHorzCur[k + 1] = (int16_t) r2 ;
			ptrDst[k]     = (int16_t) ((r3 + r2) >> 1) ;
			ptrDst[k + 1] = (int16_t) ((r3 - r2) >> 1) ;
		}
	}
	//
	// 2次元 8x8 IDCT 変換
	//
	sclwFastIDCT8x8( ptrDst ) ;
}


