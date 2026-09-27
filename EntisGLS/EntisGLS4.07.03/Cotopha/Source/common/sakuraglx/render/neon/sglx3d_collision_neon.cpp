
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_collision.h>

#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)

#include <arm_neon.h>

using namespace SSystem ;
using namespace SakuraGL ;


#define	__m128				float32x4_t
#define	_mm_add_ps			vaddq_f32
#define	_mm_sub_ps			vsubq_f32
#define	_mm_mul_ps			vmulq_f32
#define	_mm_cmplt_ps(x,y)	((float32x4_t) vcltq_f32(x,y))
#define	_mm_cmple_ps(x,y)	((float32x4_t) vcleq_f32(x,y))
#define	_mm_cmpge_ps(x,y)	((float32x4_t) vcgeq_f32(x,y))
#define	_mm_cmpneq_ps(x,y)	((float32x4_t) vornq_u32( veorq_u32((uint32x4_t)(x),(uint32x4_t)(x)), vceqq_f32(x,y) ))
#define	_mm_and_ps(x,y)		((float32x4_t) vandq_u32((uint32x4_t)(x),(uint32x4_t)(y)))

inline float32x4_t _mm_load_ps( const float32_t * pvSrc )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = pvSrc[0] ;
	vqTemp[1] = pvSrc[1] ;
	vqTemp[2] = pvSrc[2] ;
	vqTemp[3] = pvSrc[3] ;
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

inline void _mm_store_ss( float32_t * pvDst, float32x4_t src )
{
	*pvDst = src[0] ;
}

inline float32x4_t _mm_sqrt_ps( float32x4_t x )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = sqrt( x[0] ) ;
	vqTemp[1] = sqrt( x[1] ) ;
	vqTemp[2] = sqrt( x[2] ) ;
	vqTemp[3] = sqrt( x[3] ) ;
	return	vqTemp ;
}

inline float32x4_t _mm_div_ps( float32x4_t x, float32x4_t y )
{
	float32x4_t	vqTemp ;
	vqTemp[0] = x[0] / y[0] ;
	vqTemp[1] = x[1] / y[1] ;
	vqTemp[2] = x[2] / y[2] ;
	vqTemp[3] = x[3] / y[3] ;
	return	vqTemp ;
}

inline int _mm_movemask_ps( float32x4_t x )
{
	uint32_t *	p = (uint32_t*) &x ;
	return	(p[0] >> 31) | ((p[1] >> 31) << 1)
			| ((p[2] >> 31) << 2) | ((p[3] >> 31) << 3) ;
}

inline float32x4_t _mm_shuffle_ps( float32x4_t x, float32x4_t y, int s )
{
	if ( s == 0x39 )
	{
		return	vextq_f32( x, y, 1 ) ;
	}
	else
	{
		float32x4_t	vqTemp ;
		vqTemp[0] = x[s & 0x03] ;
		vqTemp[1] = x[(s >> 2) & 0x03];
		vqTemp[2] = y[(s >> 4) & 0x03];
		vqTemp[3] = y[(s >> 6) & 0x03];
		return	vqTemp ;
	}
}


// 球との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::DoesTriangleMeshHitAgainstSphere_NEON
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos, float fpRadius,
		S3DCollision::Result& rsHit, size_t iFirstPoly )
{
	ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
	//
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector		vLocalPos = vPos ;
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = 1 << nQPolyPackShift ;
	const size_t	nQPolyPackOddMask = nQPolyPackCount - 1 ;
	const size_t	nPolygons = pMeshCol->nPolygons ;
	const size_t	nQPolygons = (nPolygons + nQPolyPackOddMask)
												>> nQPolyPackShift ;
	float			errGap = fpRadius * 0.1f ;
	//
	__m128	xmmMaskAbs ;
	((uint32_t*)&xmmMaskAbs)[0] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[1] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[2] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[3] = 0x7FFFFFFF ;
	//
	__m128	xmmRadius = _mm_load1_ps( &fpRadius ) ;
	__m128	xmmErrGap = _mm_load1_ps( &errGap ) ;
	__m128	xLocalPos = _mm_load1_ps( &vLocalPos.x ) ;
	__m128	yLocalPos = _mm_load1_ps( &vLocalPos.y ) ;
	__m128	zLocalPos = _mm_load1_ps( &vLocalPos.z ) ;
	//
	for ( size_t i = (iFirstPoly & ~0x03); i < nQPolygons; i += 4 )
	{
		const QuadSphereCollision&	qsc = pSphereCol[i >> 2] ;
		__m128	dx = _mm_sub_ps
						( xLocalPos,
							_mm_load_ps( qsc.xCenter ) ) ;
		__m128	dy = _mm_sub_ps
						( yLocalPos,
							_mm_load_ps( qsc.yCenter ) ) ;
		__m128	dz = _mm_sub_ps
						( zLocalPos,
							_mm_load_ps( qsc.zCenter ) ) ;
		__m128	r = _mm_add_ps
						( xmmRadius,
							_mm_load_ps( qsc.fpRadius ) ) ;
		// dx * dx + dy * dy + dz * dz < r * r
		__m128	qscTest =
				_mm_cmplt_ps
				( _mm_add_ps
					( _mm_add_ps
						( _mm_mul_ps( dx, dx ),
							_mm_mul_ps( dy, dy ) ),
						_mm_mul_ps( dz, dz ) ),
					_mm_mul_ps( r, r ) ) ;
		int	bitsQscTest = _mm_movemask_ps( qscTest ) ;
		for ( size_t j = 0; (j < 4) & (bitsQscTest != 0); j ++, bitsQscTest >>= 1 )
		{
			if ( !(bitsQscTest & 0x01) | (i + j >= nQPolygons) )
			{
				// 範囲外
				continue ;
			}
			for ( size_t p = 0; p < nQSPackCount; p ++ )
			{
				const size_t	iQPolygon = ((i + j) << nQSPackScale) + p ;
				const size_t	iPolygon = iQPolygon << 2 ;
				if ( iPolygon >= nPolygons )
				{
					break ;
				}
				const QuadPolygonsCollision&	qpc = pCollision[iQPolygon] ;
				//
				// 交点計算
				//
				__m128	xNormal = _mm_load_ps( qpc.xNormal ) ;
				__m128	yNormal = _mm_load_ps( qpc.yNormal ) ;
				__m128	zNormal = _mm_load_ps( qpc.zNormal ) ;
				__m128	xA = _mm_load_ps( qpc.xA ) ;
				__m128	yA = _mm_load_ps( qpc.yA ) ;
				__m128	zA = _mm_load_ps( qpc.zA ) ;
				dx = _mm_sub_ps( xLocalPos, xA ) ;
				dy = _mm_sub_ps( yLocalPos, yA ) ;
				dz = _mm_sub_ps( zLocalPos, zA ) ;
				r = _mm_add_ps( _mm_mul_ps( dx, xNormal ),
						_mm_add_ps( _mm_mul_ps( dy, yNormal ),
									_mm_mul_ps( dz, zNormal ) ) ) ;
				// fabs(r) < fpRadius
				int	bitsCrossTest =
					_mm_movemask_ps
						( _mm_cmplt_ps
							( _mm_and_ps( r, xmmMaskAbs ), xmmRadius ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 非接触
					continue ;
				}
				__m128	xHit =
					_mm_sub_ps( xLocalPos, _mm_mul_ps( xNormal, r ) ) ;
				__m128	yHit =
					_mm_sub_ps( yLocalPos, _mm_mul_ps( yNormal, r ) ) ;
				__m128	zHit =
					_mm_sub_ps( zLocalPos, _mm_mul_ps( zNormal, r ) ) ;
				//
				// 三角形範囲判定
				//
				__m128	xB = _mm_load_ps( qpc.xB ) ;
				__m128	yB = _mm_load_ps( qpc.yB ) ;
				__m128	zB = _mm_load_ps( qpc.zB ) ;
				__m128	xPB = _mm_sub_ps( xHit, xB ) ;
				__m128	yPB = _mm_sub_ps( yHit, yB ) ;
				__m128	zPB = _mm_sub_ps( zHit, zB ) ;
				__m128	absPB =
					_mm_sqrt_ps( _mm_add_ps
						( _mm_mul_ps( xPB, xPB ),
							_mm_add_ps( _mm_mul_ps( yPB, yPB ),
										_mm_mul_ps( zPB, zPB ) ) ) ) ;
				__m128	absAB = _mm_load_ps( qpc.absAB ) ;
				__m128	cosB = _mm_load_ps( qpc.cosB ) ;
				__m128	sinB = _mm_load_ps( qpc.sinB ) ;
				__m128	cosB_gap =
					_mm_sub_ps( _mm_mul_ps( cosB, absPB ),
								_mm_mul_ps( xmmErrGap, sinB ) ) ;
				// (xA - xB) * xPB + (yA - yB) * yPB + (zA - zB) * zPB
				//									>= cosB_gap * absAB
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xA,xB),xPB),
								_mm_mul_ps(_mm_sub_ps(yA,yB),yPB) ),
								_mm_mul_ps(_mm_sub_ps(zA,zB),zPB) ),
							_mm_mul_ps( cosB_gap, absAB ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				__m128	xC = _mm_load_ps( qpc.xC ) ;
				__m128	yC = _mm_load_ps( qpc.yC ) ;
				__m128	zC = _mm_load_ps( qpc.zC ) ;
				__m128	absBC = _mm_load_ps( qpc.absBC ) ;
				// (xC - xB) * xPB + (yC - yB) * yPB + (zC - zB) * zPB
				//									>= cosB_gap * absBC
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xC,xB),xPB),
								_mm_mul_ps(_mm_sub_ps(yC,yB),yPB) ),
								_mm_mul_ps(_mm_sub_ps(zC,zB),zPB) ),
							_mm_mul_ps( cosB_gap, absBC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				__m128	xPC = _mm_sub_ps( xHit, xC ) ;
				__m128	yPC = _mm_sub_ps( yHit, yC ) ;
				__m128	zPC = _mm_sub_ps( zHit, zC ) ;
				__m128	absPC =
					_mm_sqrt_ps( _mm_add_ps
						( _mm_mul_ps( xPC, xPC ),
							_mm_add_ps( _mm_mul_ps( yPC, yPC ),
										_mm_mul_ps( zPC, zPC ) ) ) ) ;
				__m128	absAC = _mm_load_ps( qpc.absAC ) ;
				__m128	cosC = _mm_load_ps( qpc.cosC ) ;
				__m128	sinC = _mm_load_ps( qpc.sinC ) ;
				__m128	cosC_gap =
					_mm_sub_ps( _mm_mul_ps( cosC, absPC ),
								_mm_mul_ps( xmmErrGap, sinC ) ) ;
				// (xA - xC) * xPC + (yA - yC) * yPC + (zA - zC) * zPC
				//									>= cosC_gap * absAC )
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xA,xC),xPC),
								_mm_mul_ps(_mm_sub_ps(yA,yC),yPC) ),
								_mm_mul_ps(_mm_sub_ps(zA,zC),zPC) ),
							_mm_mul_ps( cosC_gap, absAC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				// (xB - xC) * xPC + (yB - yC) * yPC + (zB - zC) * zPC
				//									>= cosC_gap * absBC
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xB,xC),xPC),
								_mm_mul_ps(_mm_sub_ps(yB,yC),yPC) ),
								_mm_mul_ps(_mm_sub_ps(zB,zC),zPC) ),
							_mm_mul_ps( cosC_gap, absBC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				//
				// 交差
				//
				for ( size_t k = 0; k < 4; k ++ )
				{
					if ( (bitsCrossTest & 0x01)
						&& (iPolygon + k < nPolygons)
						&& (iPolygon + k >= iFirstPoly) )
					{
						S3DVector	vHitPos, vHitNormal ;
						_mm_store_ss( &vHitPos.x, xHit ) ;
						_mm_store_ss( &vHitPos.y, yHit ) ;
						_mm_store_ss( &vHitPos.z, zHit ) ;
						_mm_store_ss( &vHitNormal.x, xNormal ) ;
						_mm_store_ss( &vHitNormal.y, yNormal ) ;
						_mm_store_ss( &vHitNormal.z, zNormal ) ;
						//
						HitColliderCallback
							hccResult = rsHit.OnHitCollider
								( vHitPos, vHitNormal, pMeshCol, iPolygon + k ) ;
						if ( hccResult == S3DCollision::hitColliderReturn )
						{
							rsHit.vHitLocal = vHitPos ;
							rsHit.vHitGlobal = vHitPos ;
							rsHit.vNormalLocal = vHitNormal ;
							rsHit.vNormal = vHitNormal ;
							rsHit.iPolygon = (uint32_t) (iPolygon + k) ;
							rsHit.pMesh = pMeshCol ;
							rsHit.pPrimitiveMesh = pMeshCol ;
							rsHit.pqpcHit = &qpc ;
							return	true ;
						}
						else if ( hccResult == S3DCollision::hitColliderNextMesh )
						{
							return	false ;
						}
					}
					xHit = _mm_shuffle_ps( xHit, xHit, 0x39 ) ;
					yHit = _mm_shuffle_ps( yHit, yHit, 0x39 ) ;
					zHit = _mm_shuffle_ps( zHit, zHit, 0x39 ) ;
					xNormal = _mm_shuffle_ps( xNormal, xNormal, 0x39 ) ;
					yNormal = _mm_shuffle_ps( yNormal, yNormal, 0x39 ) ;
					zNormal = _mm_shuffle_ps( zNormal, zNormal, 0x39 ) ;
					bitsCrossTest >>= 1 ;
				}
			}
		}
	}
	return	false ;
}

// 線分との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSegmentCrossingTriangleMesh_NEON
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos0, const S3DVector& vPos1,
			float fpErrorGap, S3DCollision::Result& rsCross )
{
	ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
	//
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector	vLocalPos0 = vPos0 ;
	S3DVector	vLocalPos1 = vPos1 ;
	S3DVector	vSegRay = vLocalPos1 - vLocalPos0 ;
	double		fpSegLength = vSegRay.Absolute() ;
	vSegRay.Normalize() ;
	//
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = 1 << nQPolyPackShift ;
	const size_t	nQPolyPackOddMask = nQPolyPackCount - 1 ;
	const size_t	nPolygons = pMeshCol->nPolygons ;
	const size_t	nQPolygons = (nPolygons + nQPolyPackOddMask)
												>> nQPolyPackShift ;
	//
	bool	fCross = false ;
	//
	__m128	xmmZero ;
	((uint32_t*)&xmmZero)[0] = 0 ;
	((uint32_t*)&xmmZero)[1] = 0 ;
	((uint32_t*)&xmmZero)[2] = 0 ;
	((uint32_t*)&xmmZero)[3] = 0 ;
	//
	__m128	xmmMaskAbs ;
	((uint32_t*)&xmmMaskAbs)[0] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[1] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[2] = 0x7FFFFFFF ;
	((uint32_t*)&xmmMaskAbs)[3] = 0x7FFFFFFF ;
	//
	__m128	xmmLittle ;
	((float32_t*)&xmmLittle)[0] = 1.0e-6f ;
	((float32_t*)&xmmLittle)[1] = 1.0e-6f ;
	((float32_t*)&xmmLittle)[2] = 1.0e-6f ;
	((float32_t*)&xmmLittle)[3] = 1.0e-6f ;
	//
	float32_t	fpsSegLength = (float32_t) fpSegLength ;
	__m128	xmmErrorGap = _mm_load1_ps( &fpErrorGap ) ;
	__m128	xmmSegLength = _mm_load1_ps( &fpsSegLength ) ;
	__m128	xmmDistance = _mm_load1_ps( &rsCross.fpDistance ) ;
	__m128	xLocalPos0 = _mm_load1_ps( &vLocalPos0.x ) ;
	__m128	yLocalPos0 = _mm_load1_ps( &vLocalPos0.y ) ;
	__m128	zLocalPos0 = _mm_load1_ps( &vLocalPos0.z ) ;
	__m128	xSegRay = _mm_load1_ps( &vSegRay.x ) ;
	__m128	ySegRay = _mm_load1_ps( &vSegRay.y ) ;
	__m128	zSegRay = _mm_load1_ps( &vSegRay.z ) ;
	//
	for ( size_t i = 0; i < nQPolygons; i += 4 )
	{
		const QuadSphereCollision&	qsc = pSphereCol[i >> 2] ;
		__m128	dx = _mm_sub_ps( _mm_load_ps( qsc.xCenter ), xLocalPos0 ) ;
		__m128	dy = _mm_sub_ps( _mm_load_ps( qsc.yCenter ), yLocalPos0 ) ;
		__m128	dz = _mm_sub_ps( _mm_load_ps( qsc.zCenter ), zLocalPos0 ) ;
		__m128	r = _mm_add_ps( xmmErrorGap, _mm_load_ps( qsc.fpRadius ) ) ;
		__m128	t = _mm_add_ps( _mm_mul_ps( dx, xSegRay ),
						_mm_add_ps( _mm_mul_ps( dy, ySegRay ),
									_mm_mul_ps( dz, zSegRay ) ) ) ;
		__m128	t_sub_r = _mm_sub_ps( t, r ) ;
		// (t >= -r) & (t - r <= fpSegLength)
		//				& (t - r <= rsCross.fpDistance) )
		int	bitsQscTest = _mm_movemask_ps
			( _mm_and_ps( _mm_cmpge_ps( _mm_add_ps(t,r), xmmZero ),
				_mm_and_ps( _mm_cmple_ps( t_sub_r, xmmSegLength ),
							_mm_cmple_ps( t_sub_r, xmmDistance ) ) ) ) ;
		if ( bitsQscTest == 0 )
		{
			// 範囲外
			continue ;
		}
		// dx * dx + dy * dy + dz * dz < r * r + t * t )
		bitsQscTest &= _mm_movemask_ps( _mm_cmplt_ps
			( _mm_add_ps( _mm_mul_ps(dx,dx),
				_mm_add_ps( _mm_mul_ps(dy,dy),
							_mm_mul_ps(dz,dz) ) ),
				_mm_add_ps( _mm_mul_ps(r,r),
							_mm_mul_ps(t,t) ) ) ) ;
		for ( size_t j = 0; (j < 4) & (bitsQscTest != 0); j ++, bitsQscTest >>= 1 )
		{
			if ( !(bitsQscTest & 0x01) | (i + j >= nQPolygons) )
			{
				// 範囲外
				continue ;
			}
			for ( size_t p = 0; p < nQSPackCount; p ++ )
			{
				const size_t	iQPolygon = ((i + j) << nQSPackScale) + p ;
				const size_t	iPolygon = iQPolygon << 2 ;
				if ( iPolygon >= nPolygons )
				{
					break ;
				}
				const QuadPolygonsCollision&	qpc = pCollision[iQPolygon] ;
				//
				// 交点計算
				//
				__m128	xNormal = _mm_load_ps( qpc.xNormal ) ;
				__m128	yNormal = _mm_load_ps( qpc.yNormal ) ;
				__m128	zNormal = _mm_load_ps( qpc.zNormal ) ;
				__m128	xA = _mm_load_ps( qpc.xA ) ;
				__m128	yA = _mm_load_ps( qpc.yA ) ;
				__m128	zA = _mm_load_ps( qpc.zA ) ;
				dx = _mm_sub_ps( xA, xLocalPos0 ) ;
				dy = _mm_sub_ps( yA, yLocalPos0 ) ;
				dz = _mm_sub_ps( zA, zLocalPos0 ) ;
				r = _mm_add_ps( _mm_mul_ps( xSegRay, xNormal ),
						_mm_add_ps( _mm_mul_ps( ySegRay, yNormal ),
									_mm_mul_ps( zSegRay, zNormal ) ) ) ;
				t = _mm_div_ps(
						_mm_add_ps( _mm_mul_ps(dx,xNormal),
						_mm_add_ps( _mm_mul_ps(dy,yNormal),
									_mm_mul_ps(dz,zNormal) ) ), r ) ;
				t = _mm_and_ps( t, _mm_cmpneq_ps( r, xmmZero ) ) ;
				// (t >= 0.0) & (t < rsCross.fpDistance)
				int	bitsCrossTest =
					_mm_movemask_ps(
						_mm_cmpge_ps( _mm_and_ps( r, xmmMaskAbs ), xmmLittle ) ) ;
				bitsCrossTest &=
					_mm_movemask_ps( _mm_and_ps
						( _mm_cmpge_ps( t, xmmZero ),
							_mm_cmplt_ps( t, xmmDistance ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 非接触
					continue ;
				}
				__m128	xHit =
					_mm_add_ps( xLocalPos0, _mm_mul_ps( xSegRay, t ) ) ;
				__m128	yHit =
					_mm_add_ps( yLocalPos0, _mm_mul_ps( ySegRay, t ) ) ;
				__m128	zHit =
					_mm_add_ps( zLocalPos0, _mm_mul_ps( zSegRay, t ) ) ;
				//
				// 三角形範囲判定
				//
				__m128	xB = _mm_load_ps( qpc.xB ) ;
				__m128	yB = _mm_load_ps( qpc.yB ) ;
				__m128	zB = _mm_load_ps( qpc.zB ) ;
				__m128	xPB = _mm_sub_ps( xHit, xB ) ;
				__m128	yPB = _mm_sub_ps( yHit, yB ) ;
				__m128	zPB = _mm_sub_ps( zHit, zB ) ;
				__m128	absPB =
					_mm_sqrt_ps( _mm_add_ps
						( _mm_mul_ps( xPB, xPB ),
							_mm_add_ps( _mm_mul_ps( yPB, yPB ),
										_mm_mul_ps( zPB, zPB ) ) ) ) ;
				__m128	absAB = _mm_load_ps( qpc.absAB ) ;
				__m128	cosB = _mm_load_ps( qpc.cosB ) ;
				__m128	sinB = _mm_load_ps( qpc.sinB ) ;
				__m128	cosB_gap =
					_mm_sub_ps( _mm_mul_ps( cosB, absPB ),
								_mm_mul_ps( xmmErrorGap, sinB ) ) ;
				// (xA - xB) * xPB + (yA - yB) * yPB + (zA - zB) * zPB
				//									>= cosB_gap * absAB
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xA,xB),xPB),
								_mm_mul_ps(_mm_sub_ps(yA,yB),yPB) ),
								_mm_mul_ps(_mm_sub_ps(zA,zB),zPB) ),
							_mm_mul_ps( cosB_gap, absAB ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				__m128	xC = _mm_load_ps( qpc.xC ) ;
				__m128	yC = _mm_load_ps( qpc.yC ) ;
				__m128	zC = _mm_load_ps( qpc.zC ) ;
				__m128	absBC = _mm_load_ps( qpc.absBC ) ;
				// (xC - xB) * xPB + (yC - yB) * yPB + (zC - zB) * zPB
				//									>= cosB_gap * absBC
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xC,xB),xPB),
								_mm_mul_ps(_mm_sub_ps(yC,yB),yPB) ),
								_mm_mul_ps(_mm_sub_ps(zC,zB),zPB) ),
							_mm_mul_ps( cosB_gap, absBC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				__m128	xPC = _mm_sub_ps( xHit, xC ) ;
				__m128	yPC = _mm_sub_ps( yHit, yC ) ;
				__m128	zPC = _mm_sub_ps( zHit, zC ) ;
				__m128	absPC =
					_mm_sqrt_ps( _mm_add_ps
						( _mm_mul_ps( xPC, xPC ),
							_mm_add_ps( _mm_mul_ps( yPC, yPC ),
										_mm_mul_ps( zPC, zPC ) ) ) ) ;
				__m128	absAC = _mm_load_ps( qpc.absAC ) ;
				__m128	cosC = _mm_load_ps( qpc.cosC ) ;
				__m128	sinC = _mm_load_ps( qpc.sinC ) ;
				__m128	cosC_gap =
					_mm_sub_ps( _mm_mul_ps( cosC, absPC ),
								_mm_mul_ps( xmmErrorGap, sinC ) ) ;
				// (xA - xC) * xPC + (yA - yC) * yPC + (zA - zC) * zPC
				//									>= cosC_gap * absAC
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xA,xC),xPC),
								_mm_mul_ps(_mm_sub_ps(yA,yC),yPC) ),
								_mm_mul_ps(_mm_sub_ps(zA,zC),zPC) ),
							_mm_mul_ps( cosC_gap, absAC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				// (xB - xC) * xPC + (yB - yC) * yPC + (zB - zC) * zPC
				//									>= cosC_gap * absBC
				bitsCrossTest &=
					_mm_movemask_ps( _mm_cmpge_ps
						( _mm_add_ps( _mm_add_ps
							( _mm_mul_ps(_mm_sub_ps(xB,xC),xPC),
								_mm_mul_ps(_mm_sub_ps(yB,yC),yPC) ),
								_mm_mul_ps(_mm_sub_ps(zB,zC),zPC) ),
							_mm_mul_ps( cosC_gap, absBC ) ) ) ;
				if ( bitsCrossTest == 0 )
				{
					// 範囲外
					continue ;
				}
				//
				// 交差
				//
				for ( size_t k = 0; k < 4; k ++ )
				{
					if ( (bitsCrossTest & 0x01)
						&& (iPolygon + k < nPolygons) )
					{
						float32_t	fpDistance ;
						_mm_store_ss( &fpDistance, t ) ;
						if ( fpDistance < rsCross.fpDistance )
						{
							S3DVector	vHitPos, vHitNormal ;
							_mm_store_ss( &vHitPos.x, xHit ) ;
							_mm_store_ss( &vHitPos.y, yHit ) ;
							_mm_store_ss( &vHitPos.z, zHit ) ;
							_mm_store_ss( &vHitNormal.x, xNormal ) ;
							_mm_store_ss( &vHitNormal.y, yNormal ) ;
							_mm_store_ss( &vHitNormal.z, zNormal ) ;
							//
							HitColliderCallback
								hccResult = rsCross.OnHitCollider
									( vHitPos, vHitNormal, pMeshCol, iPolygon + k ) ;
							if ( hccResult == S3DCollision::hitColliderReturn )
							{
								rsCross.vHitLocal = vHitPos ;
								rsCross.vHitGlobal = vHitPos ;
								rsCross.vNormalLocal = vHitNormal ;
								rsCross.vNormal = vHitNormal ;
								rsCross.fpDistance = fpDistance ;
								rsCross.iPolygon = (uint32_t) (iPolygon + k) ;
								rsCross.pMesh = pMeshCol ;
								rsCross.pPrimitiveMesh = pMeshCol ;
								rsCross.pqpcHit = &qpc ;
								fCross = true ;
								xmmDistance = _mm_load1_ps( &fpDistance ) ;
							}
							else if ( hccResult == S3DCollision::hitColliderNextMesh )
							{
								return	false ;
							}
						}
					}
					bitsCrossTest >>= 1 ;
					if ( bitsCrossTest == 0 )
					{
						break ;
					}
					xHit = _mm_shuffle_ps( xHit, xHit, 0x39 ) ;
					yHit = _mm_shuffle_ps( yHit, yHit, 0x39 ) ;
					zHit = _mm_shuffle_ps( zHit, zHit, 0x39 ) ;
					xNormal = _mm_shuffle_ps( xNormal, xNormal, 0x39 ) ;
					yNormal = _mm_shuffle_ps( yNormal, yNormal, 0x39 ) ;
					zNormal = _mm_shuffle_ps( zNormal, zNormal, 0x39 ) ;
					t = _mm_shuffle_ps( t, t, 0x39 ) ;
				}
			}
		}
	}
	return	fCross ;
}


#endif

