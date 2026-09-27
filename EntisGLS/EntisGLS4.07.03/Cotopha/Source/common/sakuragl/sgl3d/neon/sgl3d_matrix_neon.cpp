
#include <sakuragl/sakuragl.h>

#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)

#include <arm_neon.h>

using namespace SSystem ;
using namespace SakuraGL ;

#define	__m128				float32x4_t
#define	__m64				float32x2_t
#define	_mm_add_ps			vaddq_f32
#define	_mm_sub_ps			vsubq_f32
#define	_mm_mul_ps			vmulq_f32

inline float32x4_t _mm_loadu_ps( const float32_t * pvSrc )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = pvSrc[0] ;
	vqTemp[1] = pvSrc[1] ;
	vqTemp[2] = pvSrc[2] ;
	vqTemp[3] = pvSrc[3] ;
	return	vqTemp ;
}

inline float32x4_t _mm_load_ss( const float32_t * pvSrc )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = pvSrc[0] ;
	vqTemp[1] = 0.0f ;
	vqTemp[2] = 0.0f ;
	vqTemp[3] = 0.0f ;
	return	vqTemp ;
}

inline float32x4_t _mm_load1_ps( const float32_t * pvSrc )
{
	float32x4_t	vqTemp ;
	float32_t	s = pvSrc[0] ;
	vqTemp[0] = s ;
	vqTemp[1] = s ;
	vqTemp[2] = s ;
	vqTemp[3] = s ;
	return	vqTemp ;
}

inline void _mm_storeu_ps( float32_t * pvDst, float32x4_t x )
{
	pvDst[0] = x[0] ;
	pvDst[1] = x[1] ;
	pvDst[2] = x[2] ;
	pvDst[3] = x[3] ;
}

inline float32x4_t _mm_unpacklo_ps( float32x4_t x, float32x4_t y )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = x[0] ;
	vqTemp[1] = y[0] ;
	vqTemp[2] = x[1] ;
	vqTemp[3] = y[1] ;
	return	vqTemp ;
}

inline float32x4_t _mm_unpackhi_ps( float32x4_t x, float32x4_t y )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = x[2] ;
	vqTemp[1] = y[2] ;
	vqTemp[2] = x[3] ;
	vqTemp[3] = y[3] ;
	return	vqTemp ;
}

inline float32x4_t _mm_shuffle_ps( float32x4_t x, float32x4_t y, int s )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = x[s & 0x03] ;
	vqTemp[1] = x[(s >> 2) & 0x03];
	vqTemp[2] = y[(s >> 4) & 0x03];
	vqTemp[3] = y[(s >> 6) & 0x03];
	return	vqTemp ;
}


//////////////////////////////////////////////////////////////////////////////
// ベクトル配列積加算
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::AddProductedVector1DArray_NEON
	( float32_t * pfpDst,
		const float32_t * pfpSrc, float32_t fpWeight, size_t nCount )
{
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

void SakuraGL::AddProductedVector4DArray_NEON
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc, float32_t fpWeight, size_t nCount )
{
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


//////////////////////////////////////////////////////////////////////////////
// ベクトル配列積加算（ウェイトマップ付き）
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::AddProductedVector4DArrayWithWeight_NEON
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc,
		const float32_t * pfpWeightMap,
		float32_t fpWeight, size_t nCount )
{
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

void SakuraGL::AddProductedVector4DArrayWithNegWeight_NEON
	( S3DVector4 * pvDst,
		const S3DVector4 * pvSrc,
		const float32_t * pfpWeightMap, size_t nCount )
{
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



//////////////////////////////////////////////////////////////////////////////
// 三次元変換行列変換
//////////////////////////////////////////////////////////////////////////////

void S3DMatrix::RevolveVectors_NEON
	( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
			size_t nCount, const S3DVector & vOffset ) const
{
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

void S3DMatrix::AddRevolvedVectorsWithWeight_NEON
	( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
			const float32_t * pWeight,
			size_t nCount, const S3DVector & vOffset ) const
{
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


#endif

