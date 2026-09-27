
#include <sakuragl/sakuragl.h>

#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
#include <xmmintrin.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 色配列積加算
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::AddProductedColorArray
	( SGLPalette * pDst,
		const SGLPalette * pvSrc, float32_t fpWeight, size_t nCount )
{
	unsigned int	w =
		(unsigned int) esl_clampi
			( eslRoundR32ToInt( fpWeight * 0x100 ), 0, 0x100 ) ;
	for ( size_t j = 0; j < nCount; j ++ )
	{
		pDst[j].ui32 =
			sglPackedColorAdd
				( pDst[j].ui32, sglPackedColorMul( pvSrc[j].ui32, w ) ) ;
	}
}

void SakuraGL::AddProductedColorPairArray
	( S3DColor * pDst,
		const S3DColor * pvSrc, float32_t fpWeight, size_t nCount )
{
	AddProductedColorArray
		( (SGLPalette*) pDst,
			(const SGLPalette*) pvSrc, fpWeight, nCount * 2 ) ;
}

void SakuraGL::AddProductedColorPairArrayWithWeight
	( S3DColor * pDst,
		const S3DColor * pvSrc,
		const float32_t * pfpWeightMap,
		float32_t fpWeight, size_t nCount )
{
	for ( size_t j = 0; j < nCount; j ++ )
	{
		if ( pfpWeightMap[j] != 0.0f )
		{
			unsigned int	w =
				(unsigned int) esl_clampi
					( eslRoundR32ToInt
						( pfpWeightMap[j] * fpWeight * 0x100 ), 0, 0x100 ) ;
			pDst[j].rgbMul.ui32 =
				sglPackedColorAdd
					( pDst[j].rgbMul.ui32,
						sglPackedColorMul( pvSrc[j].rgbMul.ui32, w ) ) ;
			pDst[j].rgbAdd.ui32 =
				sglPackedColorAdd
					( pDst[j].rgbAdd.ui32,
						sglPackedColorMul( pvSrc[j].rgbAdd.ui32, w ) ) ;
		}
	}
}

void SakuraGL::AddProductedColorPairArrayWithNegWeight
	( S3DColor * pDst,
		const S3DColor * pvSrc,
		const float32_t * pfpWeightMap, size_t nCount )
{
	for ( size_t j = 0; j < nCount; j ++ )
	{
		if ( pfpWeightMap[j] != 1.0f )
		{
			unsigned int	w =
				(unsigned int) esl_clampi
					( eslRoundR32ToInt
						( (1.0f - pfpWeightMap[j]) * 0x100 ), 0, 0x100 ) ;
			pDst[j].rgbMul.ui32 =
				sglPackedColorAdd
					( pDst[j].rgbMul.ui32,
						sglPackedColorMul( pvSrc[j].rgbMul.ui32, w ) ) ;
			pDst[j].rgbAdd.ui32 =
				sglPackedColorAdd
					( pDst[j].rgbAdd.ui32,
						sglPackedColorMul( pvSrc[j].rgbAdd.ui32, w ) ) ;
		}
	}
}


// ベクトル配列最小最大値
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::MinMaxVector4DArray
	( S3DVector4& vMin, S3DVector4& vMax,
		const S3DVector4 * pvSrc, size_t nCount )
{
	if ( nCount == 0 )
	{
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		vMax.d = 0 ;
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		vMin.d = 0 ;
		return ;
	}
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		float32_t	fpMax[4], fpMin[4] ;
		//
		__m128	xmmMax = _mm_loadu_ps( (float*) pvSrc ) ;
		__m128	xmmMin = xmmMax ;
		//
		for ( size_t i = 1; i < nCount; i ++ )
		{
			__m128	xmmNext = _mm_loadu_ps( (float*) (pvSrc + i) ) ;
			xmmMax =_mm_max_ps( xmmMax, xmmNext ) ;
			xmmMin =_mm_min_ps( xmmMin, xmmNext ) ;
		}
		_mm_storeu_ps( fpMax, xmmMax ) ;
		_mm_storeu_ps( fpMin, xmmMin ) ;
		//
		vMax.x = fpMax[0] ;
		vMax.y = fpMax[1] ;
		vMax.z = fpMax[2] ;
		vMax.d = fpMax[3] ;
		vMin.x = fpMin[0] ;
		vMin.y = fpMin[1] ;
		vMin.z = fpMin[2] ;
		vMin.d = fpMin[3] ;
	}
	else
	{
	#endif
		int32_t	xMax = *((int32_t*)&(pvSrc->x)) ;
		int32_t	yMax = *((int32_t*)&(pvSrc->y)) ;
		int32_t	zMax = *((int32_t*)&(pvSrc->z)) ;
		int32_t	wMax = *((int32_t*)&(pvSrc->d)) ;
		xMax ^= (xMax >> 31) & 0x7FFFFFFF ;
		yMax ^= (yMax >> 31) & 0x7FFFFFFF ;
		zMax ^= (zMax >> 31) & 0x7FFFFFFF ;
		wMax ^= (wMax >> 31) & 0x7FFFFFFF ;
		//
		int32_t	xMin = xMax, yMin = yMax, zMin = zMax, wMin = wMax ;
		//
		for ( size_t i = 1; i < nCount; i ++ )
		{
			int32_t	x = *((int32_t*)&(pvSrc->x)) ;
			int32_t	y = *((int32_t*)&(pvSrc->y)) ;
			int32_t	z = *((int32_t*)&(pvSrc->z)) ;
			int32_t	w = *((int32_t*)&(pvSrc->d)) ;
			//
			x ^= (x >> 31) & 0x7FFFFFFF ;
			y ^= (y >> 31) & 0x7FFFFFFF ;
			z ^= (z >> 31) & 0x7FFFFFFF ;
			w ^= (w >> 31) & 0x7FFFFFFF ;
			//
			#if	defined(__COTOPHA__)
				int32_t	fx = (int) (x > xMax) ;
				int32_t	fy = (int) (y > yMax) ;
				int32_t	fz = (int) (z > zMax) ;
				int32_t	fw = (int) (w > zMax) ;
			#else
				int32_t	fx = - (int) (x > xMax) ;
				int32_t	fy = - (int) (y > yMax) ;
				int32_t	fz = - (int) (z > zMax) ;
				int32_t	fw = - (int) (w > zMax) ;
			#endif
			xMax = (xMax & ~fx) | (x & fx) ;
			yMax = (yMax & ~fy) | (y & fy) ;
			zMax = (zMax & ~fz) | (z & fz) ;
			wMax = (wMax & ~fw) | (w & fw) ;
			//
			#if	defined(__COTOPHA__)
				fx = (int) (x < xMin) ;
				fy = (int) (y < yMin) ;
				fz = (int) (z < zMin) ;
				fw = (int) (w < wMin) ;
			#else
				fx = - (int) (x < xMin) ;
				fy = - (int) (y < yMin) ;
				fz = - (int) (z < zMin) ;
				fw = - (int) (w < wMin) ;
			#endif
			xMin = (xMin & ~fx) | (x & fx) ;
			yMin = (yMin & ~fy) | (y & fy) ;
			zMin = (zMin & ~fz) | (z & fz) ;
			wMin = (wMin & ~fw) | (w & fw) ;
		}
		xMax ^= (xMax >> 31) & 0x7FFFFFFF ;
		yMax ^= (yMax >> 31) & 0x7FFFFFFF ;
		zMax ^= (zMax >> 31) & 0x7FFFFFFF ;
		wMax ^= (wMax >> 31) & 0x7FFFFFFF ;
		xMin ^= (xMin >> 31) & 0x7FFFFFFF ;
		yMin ^= (yMin >> 31) & 0x7FFFFFFF ;
		zMin ^= (zMin >> 31) & 0x7FFFFFFF ;
		wMin ^= (wMin >> 31) & 0x7FFFFFFF ;
		//
		vMax.x = *((float32_t*)&xMax) ;
		vMax.y = *((float32_t*)&yMax) ;
		vMax.z = *((float32_t*)&zMax) ;
		vMax.d = *((float32_t*)&wMax) ;
		vMin.x = *((float32_t*)&xMin) ;
		vMin.y = *((float32_t*)&yMin) ;
		vMin.z = *((float32_t*)&zMin) ;
		vMin.d = *((float32_t*)&wMin) ;
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	}
	#endif
}


//////////////////////////////////////////////////////////////////////////////
// ベクトル配列積加算
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::AddProductedVector1DArray
	( float32_t * pfpDst,
		const float32_t * pfpSrc, float32_t fpWeight, size_t nCount )
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		__m128	w = _mm_load1_ps( &fpWeight ) ;
		size_t	nAlignedCount = nCount >> 2 ;
		size_t	nOddCount = nCount & 0x03 ;
		size_t	i ;
		for ( i = 0; i < nAlignedCount; ++ i )
		{
			__m128	s = _mm_loadu_ps( pfpSrc ) ;
			__m128	d = _mm_loadu_ps( pfpDst ) ;
			_mm_storeu_ps
				( pfpDst, _mm_add_ps( d, _mm_mul_ps( s, w ) ) ) ;
			pfpSrc += 4 ;
			pfpDst += 4 ;
		}
		for ( i = 0; i < nOddCount; i ++ )
		{
			pfpDst[i] += pfpSrc[i] * fpWeight ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddProductedVector1DArray_NEON( pfpDst, pfpSrc, fpWeight, nCount ) ;
	}
	else
	#endif
	{
		for ( size_t i = 0; i < nCount; ++ i )
		{
			pfpDst[i] += pfpSrc[i] * fpWeight ;
		}
	}
}

void SakuraGL::AddProductedVector2DArray
	( S2DVector * pvDst,
		const S2DVector * pvSrc, float32_t fpWeight, size_t nCount )
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		__m128	w = _mm_load1_ps( &fpWeight ) ;
		size_t	nAlignedCount = nCount >> 1 ;
		for ( size_t i = 0; i < nAlignedCount; ++ i )
		{
			__m128	s = _mm_loadu_ps( &pvSrc->x ) ;
			__m128	d = _mm_loadu_ps( &pvDst->x ) ;
			_mm_storeu_ps
				( &(pvDst->x),
					_mm_add_ps( d, _mm_mul_ps( s, w ) ) ) ;
			pvSrc += 2 ;
			pvDst += 2 ;
		}
		if ( nCount & 0x01 )
		{
			pvDst->x += pvSrc->x * fpWeight ;
			pvDst->y += pvSrc->y * fpWeight ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddProductedVector1DArray_NEON
			( &pvDst->x, &pvSrc->x, fpWeight, nCount * 2 ) ;
	}
	else
	#endif
	{
		for ( size_t i = 0; i < nCount; ++ i )
		{
			pvDst->x += pvSrc->x * fpWeight ;
			pvDst->y += pvSrc->y * fpWeight ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
}

void SakuraGL::AddProductedVector4DArray
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc, float32_t fpWeight, size_t nCount )
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		__m128	w = _mm_load1_ps( &fpWeight ) ;
		for ( size_t i = 0; i < nCount; ++ i )
		{
			__m128	s = _mm_loadu_ps( &pvSrc->x ) ;
			__m128	d = _mm_loadu_ps( &pvDst->x ) ;
			_mm_storeu_ps
				( &(pvDst->x),
					_mm_add_ps( d, _mm_mul_ps( s, w ) ) ) ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddProductedVector4DArray_NEON( pvDst, pvSrc, fpWeight, nCount ) ;
	}
	else
	#endif
	{
		for ( size_t i = 0; i < nCount; ++ i )
		{
			pvDst->x += pvSrc->x * fpWeight ;
			pvDst->y += pvSrc->y * fpWeight ;
			pvDst->z += pvSrc->z * fpWeight ;
			pvDst->d += pvSrc->d * fpWeight ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ベクトル配列積加算（ウェイトマップ付き）
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::AddProductedVector2DArrayWithWeight
	( S2DVector * pvDst,
		const S2DVector * pvSrc,
		const float32_t * pfpWeightMap,
		float32_t fpWeight, size_t nCount )
{
	for ( size_t i = 0; i < nCount; ++ i )
	{
		if ( *pfpWeightMap != 0.0f )
		{
			float32_t	w = *pfpWeightMap * fpWeight ;
			pvDst->x += pvSrc->x * w ;
			pvDst->y += pvSrc->y * w ;
		}
		++ pfpWeightMap ;
		++ pvSrc ;
		++ pvDst ;
	}
}

void SakuraGL::AddProductedVector4DArrayWithWeight
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc,
		const float32_t * pfpWeightMap,
		float32_t fpWeight, size_t nCount )
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		for ( size_t i = 0; i < nCount; ++ i )
		{
			if ( *pfpWeightMap != 0.0f )
			{
				float32_t	w = *pfpWeightMap * fpWeight ;
				__m128	pw = _mm_load1_ps( &w ) ;
				__m128	s = _mm_loadu_ps( &pvSrc->x ) ;
				__m128	d = _mm_loadu_ps( &pvDst->x ) ;
				_mm_storeu_ps
					( &(pvDst->x),
						_mm_add_ps( d, _mm_mul_ps( s, pw ) ) ) ;
			}
			++ pfpWeightMap ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddProductedVector4DArrayWithWeight_NEON
				( pvDst, pvSrc, pfpWeightMap, fpWeight, nCount ) ;
	}
	else
	#endif
	{
		for ( size_t i = 0; i < nCount; ++ i )
		{
			if ( *pfpWeightMap != 0.0f )
			{
				float32_t	w = *pfpWeightMap * fpWeight ;
				pvDst->x += pvSrc->x * w ;
				pvDst->y += pvSrc->y * w ;
				pvDst->z += pvSrc->z * w ;
				pvDst->d += pvSrc->d * w ;
			}
			++ pfpWeightMap ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
}

void SakuraGL::AddProductedVector2DArrayWithNegWeight
	( S2DVector * pvDst,
		const S2DVector * pvSrc,
		const float32_t * pfpWeightMap, size_t nCount )
{
	for ( size_t i = 0; i < nCount; ++ i )
	{
		if ( *pfpWeightMap != 1.0f )
		{
			float32_t	w = 1.0f - *pfpWeightMap ;
			pvDst->x += pvSrc->x * w ;
			pvDst->y += pvSrc->y * w ;
		}
		++ pfpWeightMap ;
		++ pvSrc ;
		++ pvDst ;
	}
}

void SakuraGL::AddProductedVector4DArrayWithNegWeight
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc,
		const float32_t * pfpWeightMap, size_t nCount )
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		for ( size_t i = 0; i < nCount; ++ i )
		{
			if ( *pfpWeightMap != 1.0f )
			{
				float32_t	w = 1.0f - *pfpWeightMap ;
				__m128	pw = _mm_load1_ps( &w ) ;
				__m128	s = _mm_loadu_ps( &pvSrc->x ) ;
				__m128	d = _mm_loadu_ps( &pvDst->x ) ;
				_mm_storeu_ps
					( &(pvDst->x),
						_mm_add_ps( d, _mm_mul_ps( s, pw ) ) ) ;
			}
			++ pfpWeightMap ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddProductedVector4DArrayWithNegWeight_NEON
				( pvDst, pvSrc, pfpWeightMap, nCount ) ;
	}
	else
	#endif
	{
		for ( size_t i = 0; i < nCount; ++ i )
		{
			if ( *pfpWeightMap != 1.0f )
			{
				float32_t	w = 1.0f - *pfpWeightMap ;
				pvDst->x += pvSrc->x * w ;
				pvDst->y += pvSrc->y * w ;
				pvDst->z += pvSrc->z * w ;
				pvDst->d += pvSrc->d * w ;
			}
			++ pfpWeightMap ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
}

void SakuraGL::AddRevolvedVectorsWithIndexedWeight
	( S3DVector4 * pvDst,
		const S3DMatrix * pMatrics,
		const S3DVector * pvTranslate,
		const S3DVector4 * pvSrc,
		const uint32_t * pIndexedMap,
		const float32_t * pfpWeightMap, size_t nCount )
{
	S3DVector	vTemp ;
	for ( size_t i = 0; i < nCount; ++ i )
	{
		float32_t	w = pfpWeightMap[i] ;
		if ( w != 0.0f )
		{
			vTemp = pvSrc[i] ;
			//
			uint32_t	iMat = pIndexedMap[i] ;
			pMatrics[iMat].RevolveVector( vTemp ) ;
			//
			if ( pvTranslate != NULL )
			{
				vTemp += pvTranslate[iMat] ;
			}
			//
			pvDst->x += vTemp.x * w ;
			pvDst->y += vTemp.y * w ;
			pvDst->z += vTemp.z * w ;
		}
		++ pvDst ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 三次元変換行列変換
//////////////////////////////////////////////////////////////////////////////

void S3DMatrix::RevolveVectors
	( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
			size_t nCount, const S3DVector & vOffset ) const
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		//
		// 行列スウィズリング
		//
		S3DVector4	v4Offset( vOffset, 0.0f ) ;
		//
		__m128	m0 = _mm_loadu_ps( &m[0][0] ) ;			// x0y0z0w0
		__m128	m1 = _mm_loadu_ps( &m[1][0] ) ;			// x1y1z1w1
		__m128	m2 = _mm_loadu_ps( &m[2][0] ) ;			// x2y2z2w2
		//
		__m128	p0 = _mm_unpacklo_ps( m0, m1 ) ;		// x0x1y0y1
		__m128	p1 = _mm_unpackhi_ps( m0, m1 ) ;		// z0z1w0w1
		//
		__m128	s0 = _mm_shuffle_ps( p0, m2, 0xC4 ) ;	// x0x1x2w2
		__m128	s1 = _mm_shuffle_ps( p0, m2, 0xDE ) ;	// y0y1y2w2
		__m128	s2 = _mm_shuffle_ps( p1, m2, 0xE4 ) ;	// z0z1z2w2
		__m128	s3 = _mm_loadu_ps( &v4Offset.x ) ;		// x3y3z300
		//
		for ( size_t i = 0; i < nCount; ++ i )
		{
			__m128	x = _mm_load1_ps( &pvSrc->x ) ;
			__m128	y = _mm_load1_ps( &pvSrc->y ) ;
			__m128	z = _mm_load1_ps( &pvSrc->z ) ;
			//
			_mm_storeu_ps
				( &(pvDst->x),
					_mm_add_ps
						( _mm_add_ps( _mm_mul_ps( z, s2 ), s3 ),
							_mm_add_ps( _mm_mul_ps( x, s0 ),
											_mm_mul_ps( y, s1 ) ) ) ) ;
			//
			++ pvSrc ;
			++ pvDst ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		RevolveVectors_NEON( pvDst, pvSrc, nCount, vOffset ) ;
	}
	else
	#endif
	{
		const float32_t	xOffset = vOffset.x ;
		const float32_t	yOffset = vOffset.y ;
		const float32_t	zOffset = vOffset.z ;
		for ( size_t i = 0; i < nCount; ++ i )
		{
			float32_t	x = pvSrc->x ;
			float32_t	y = pvSrc->y ;
			float32_t	z = pvSrc->z ;
			pvDst->x = m[0][0] * x + m[0][1] * y + m[0][2] * z + xOffset ;
			pvDst->y = m[1][0] * x + m[1][1] * y + m[1][2] * z + yOffset ;
			pvDst->z = m[2][0] * x + m[2][1] * y + m[2][2] * z + zOffset ;
			++ pvSrc ;
			++ pvDst ;
		}
	}
}

void S3DMatrix::AddRevolvedVectorsWithWeight
	( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
			const float32_t * pWeight,
			size_t nCount, const S3DVector & vOffset ) const
{
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		//
		// 行列スウィズリング
		//
		S3DVector4	v4Offset( vOffset, 0.0f ) ;
		//
		__m128	m0 = _mm_loadu_ps( &m[0][0] ) ;			// x0y0z0w0
		__m128	m1 = _mm_loadu_ps( &m[1][0] ) ;			// x1y1z1w1
		__m128	m2 = _mm_loadu_ps( &m[2][0] ) ;			// x2y2z2w2
		//
		__m128	p0 = _mm_unpacklo_ps( m0, m1 ) ;		// x0x1y0y1
		__m128	p1 = _mm_unpackhi_ps( m0, m1 ) ;		// z0z1w0w1
		//
		__m128	s0 = _mm_shuffle_ps( p0, m2, 0xC4 ) ;	// x0x1x2w2
		__m128	s1 = _mm_shuffle_ps( p0, m2, 0xDE ) ;	// y0y1y2w2
		__m128	s2 = _mm_shuffle_ps( p1, m2, 0xE4 ) ;	// z0z1z2w2
		__m128	s3 = _mm_loadu_ps( &v4Offset.x ) ;		// x3y3z300
		//
		for ( size_t i = 0; i < nCount; ++ i )
		{
			float32_t	ws = *pWeight ;
			if ( ws != 0.0f )
			{
				__m128	x = _mm_load1_ps( &pvSrc->x ) ;
				__m128	y = _mm_load1_ps( &pvSrc->y ) ;
				__m128	z = _mm_load1_ps( &pvSrc->z ) ;
				__m128	w = _mm_load1_ps( &ws ) ;
				__m128	dv = _mm_loadu_ps( &pvDst->x ) ;
				//
				__m128	d = 
						_mm_add_ps
							( _mm_add_ps( _mm_mul_ps( z, s2 ), s3 ),
								_mm_add_ps( _mm_mul_ps( x, s0 ),
												_mm_mul_ps( y, s1 ) ) ) ;
				_mm_storeu_ps
					( &(pvDst->x), _mm_add_ps( dv, _mm_mul_ps( d, w ) ) ) ;
			}
			++ pvSrc ;
			++ pWeight ;
			++ pvDst ;
		}
	}
	else
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		AddRevolvedVectorsWithWeight_NEON
			( pvDst, pvSrc, pWeight, nCount, vOffset ) ;
	}
	else
	#endif
	{
		const float32_t	xOffset = vOffset.x ;
		const float32_t	yOffset = vOffset.y ;
		const float32_t	zOffset = vOffset.z ;
		for ( size_t i = 0; i < nCount; ++ i )
		{
			float32_t	x = pvSrc->x ;
			float32_t	y = pvSrc->y ;
			float32_t	z = pvSrc->z ;
			float32_t	w = *pWeight ;
			if ( w != 0.0f )
			{
				pvDst->x += (m[0][0] * x + m[0][1] * y + m[0][2] * z + xOffset) * w ;
				pvDst->y += (m[1][0] * x + m[1][1] * y + m[1][2] * z + yOffset) * w ;
				pvDst->z += (m[2][0] * x + m[2][1] * y + m[2][2] * z + zOffset) * w ;
			}
			++ pvSrc ;
			++ pWeight ;
			++ pvDst ;
		}
	}
}

