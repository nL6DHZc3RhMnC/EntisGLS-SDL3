
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl3d/sgl_render_shaper.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 形状生成オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshShaper, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshShaper::S3DMeshShaper( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshShaper::~S3DMeshShaper( void )
{
}

// 格子状メッシュを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::GridMesh
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial, uint32_t nFlags,
		size_t widthMesh, size_t heightMesh,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	//
	// ポリゴン数計算
	//
	GridMeshGenParams	gmgp ;
	size_t	countPolygons =
		CalcGridMeshPolygonCount( gmgp, nFlags, widthMesh, heightMesh ) ;
	if ( countPolygons == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	size_t	countIndex = countPolygons * 3 ;
	if ( m_bufIndex.GetLength() < countIndex )
	{
		m_bufIndex.FreeArray() ;
		m_bufIndex.SetLength( (countIndex + 0xFF) & ~0xFF ) ;
	}
	//
	// インデックス生成
	//
	MakeGridMeshIndex
		( nFlags, widthMesh, heightMesh, gmgp, m_bufIndex.GetArray() ) ;
	m_bufIndex.FinishArray() ;
	//
	// 追加
	//
	return	render.AddIndexedTriangleList
				( pMaterial, 0, countPolygons, gmgp.countVertex,
					pvVertex, pvNormal, pvUVMap, pColor,
					m_bufIndex.GetConstArray() ) ;
}

SGLError S3DMeshShaper::AddGridMesh
	( S3DVertexBufferInterface & vbuf,
		const S3DMeshShaper::GridMeshGenParams& gmgp,
		const S3DMeshShaper::GridMeshParam& gmp,
		const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf )
{
	MakeGridMeshIndex
		( gmp.nFlags, gmp.widthMesh,
			gmp.heightMesh, gmgp, prmbuf.pIndexedList ) ;
	//
	return	vbuf.AddPrimitiveBuffer
		( NULL, 0, primitiveTriangle, prmbuf,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
}

size_t S3DMeshShaper::CalcGridMeshPolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		uint32_t nFlags, size_t widthMesh, size_t heightMesh )
{
	//
	// ポリゴン数計算
	//
	gmgp.countPolygon = 0 ;
	//
	size_t	countLinePolygons = widthMesh * 2 ;
	if ( nFlags & gridHorzLoop )
	{
		countLinePolygons += 2 ;
	}
	gmgp.iGridBodyFirst = 0 ;
	gmgp.iGridBodyEnd = (widthMesh + 1) * (heightMesh + 1) ;
	gmgp.countBodyLines = heightMesh ;
	gmgp.countVertex = gmgp.iGridBodyEnd ;
	if ( nFlags & gridVertLoop )
	{
		gmgp.countBodyLines ++ ;
		nFlags &= ~(gridTopTip | gridBottomTip) ;
	}
	else
	{
		gmgp.countVertex = 0 ;
		if ( nFlags & gridTopTip )
		{
			if ( gmgp.countBodyLines == 0 )
			{
				gmgp.countPolygon = 0 ;
				return	0 ;
			}
			gmgp.countPolygon += (countLinePolygons >> 1) ;
			gmgp.countBodyLines -- ;
			gmgp.iGridBodyFirst = 1 ;
			gmgp.countVertex ++ ;
		}
		if ( nFlags & gridBottomTip )
		{
			if ( gmgp.countBodyLines == 0 )
			{
				gmgp.countPolygon = 0 ;
				return	0 ;
			}
			gmgp.countPolygon += (countLinePolygons >> 1) ;
			gmgp.countBodyLines -- ;
			gmgp.countVertex ++ ;
		}
		gmgp.iGridBodyEnd = gmgp.iGridBodyFirst
						+ (widthMesh + 1) * (gmgp.countBodyLines + 1) ;
		gmgp.countVertex += (widthMesh + 1) * (gmgp.countBodyLines + 1) ;
	}
	gmgp.countPolygon += countLinePolygons * gmgp.countBodyLines ;
	return	gmgp.countPolygon ;
}

void S3DMeshShaper::MakeGridMeshIndex
	( uint32_t nFlags,
		size_t widthMesh, size_t heightMesh,
		const S3DMeshShaper::GridMeshGenParams& gmgp, uint32_t * pIndexedBuf )
{
	uint32_t * pIndexed = pIndexedBuf ;
	//
	// 先端扇部
	//
	size_t	i ;
	if ( nFlags & gridTopTip )
	{
		for ( i = 0; i < widthMesh; i ++ )
		{
			pIndexed[0] = 0 ;
			pIndexed[1] = 2 + (uint32_t) i ;
			pIndexed[2] = 1 + (uint32_t) i ;
			pIndexed += 3 ;
		}
		if ( nFlags & gridHorzLoop )
		{
			pIndexed[0] = 0 ;
			pIndexed[1] = 1 ;
			pIndexed[2] = 1 + (uint32_t) widthMesh ;
			pIndexed += 3 ;
		}
	}
	//
	// 中央格子部
	//
	size_t	iLine0 = gmgp.iGridBodyFirst ;
	for ( i = 0; i < gmgp.countBodyLines; i ++ )
	{
		size_t	iLine1 = iLine0 + (widthMesh + 1) ;
		if ( iLine1 >= gmgp.iGridBodyEnd )
		{
			iLine1 -= (gmgp.iGridBodyEnd - gmgp.iGridBodyFirst) ;
		}
		for ( size_t j = 0; j < widthMesh; j ++ )
		{
			pIndexed[0] = (uint32_t) (iLine0 + j) ;
			pIndexed[1] = (uint32_t) (iLine0 + j + 1) ;
			pIndexed[2] = (uint32_t) (iLine1 + j) ;
			pIndexed[3] = (uint32_t) (iLine0 + j + 1) ;
			pIndexed[4] = (uint32_t) (iLine1 + j + 1) ;
			pIndexed[5] = (uint32_t) (iLine1 + j) ;
			pIndexed += 6 ;
		}
		if ( nFlags & gridHorzLoop )
		{
			pIndexed[0] = (uint32_t) (iLine0 + widthMesh) ;
			pIndexed[1] = (uint32_t) iLine0 ;
			pIndexed[2] = (uint32_t) (iLine1 + widthMesh) ;
			pIndexed[3] = (uint32_t) iLine0 ;
			pIndexed[4] = (uint32_t) iLine1 ;
			pIndexed[5] = (uint32_t) (iLine1 + widthMesh) ;
			pIndexed += 6 ;
		}
		iLine0 = iLine1 ;
	}
	//
	// 終端逆扇部
	//
	if ( nFlags & gridBottomTip )
	{
		for ( i = 0; i < widthMesh; i ++ )
		{
			pIndexed[0] = (uint32_t) (iLine0 + i) ;
			pIndexed[1] = (uint32_t) (iLine0 + i + 1) ;
			pIndexed[2] = (uint32_t) gmgp.iGridBodyEnd ;
			pIndexed += 3 ;
		}
		if ( nFlags & gridHorzLoop )
		{
			pIndexed[0] = (uint32_t) (iLine0 + widthMesh) ;
			pIndexed[1] = (uint32_t) iLine0 ;
			pIndexed[2] = (uint32_t) gmgp.iGridBodyEnd ;
			pIndexed += 3 ;
		}
	}
	ESLAssert( pIndexedBuf + gmgp.countPolygon * 3 == pIndexed ) ;
	//
	// 面反転
	//
	if ( nFlags & gridBackface )
	{
		pIndexed = pIndexedBuf ;
		for ( i = 0; i < gmgp.countPolygon; i ++ )
		{
			uint32_t	t = pIndexed[1] ;
			pIndexed[1] = pIndexed[2] ;
			pIndexed[2] = t ;
			pIndexed += 3 ;
		}
	}
}

// 凸でない頂点を含む多角形を三角形リストに分割したインデックスリスト
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshShaper::MakeTriangleIndexedList
	( SSystem::SArray<uint32_t>& aIndexedList,
		SSystem::SArray<uint32_t>& aWorkIndex,
		const S3DVector4 * pvVertex, size_t nVertexCount )
{
	if ( nVertexCount < 3 )
	{
		return	0 ;
	}
	const size_t	nBufVertexCount = nVertexCount ;
	//
	// 頂点連結リスト生成
	//
	S3DVector	vValidFace( 0, 0, 0 ) ;
	uint32_t *	pWorkIndex = aWorkIndex.GetArray( nVertexCount * 2 ) ;
	for ( uint32_t i = 0; i < nVertexCount; i ++ )
	{
		uint32_t	iPrev = (i == 0) ? (uint32_t) nVertexCount - 1 : i - 1 ;
		uint32_t	iNext = (i == nVertexCount - 1) ? 0 : i + 1 ;
		pWorkIndex[i * 2]     = iPrev ;
		pWorkIndex[i * 2 + 1] = iNext ;
		//
		S3DVector	v0 = pvVertex[i] ;
		S3DVector	v1 = pvVertex[iPrev] ;
		S3DVector	v2 = pvVertex[iNext] ;
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		vValidFace += vNormal.Normalized() ;
	}
	//
	S3DVector4	vMin, vMax ;
	MinMaxVector4DArray( vMin, vMax, pvVertex, nVertexCount ) ;
	//
	const float32_t	fpErrorGap = (float32_t) ((vMax - vMin).Absolute()) * 0.001f ;
	//
	// 縮退点排除
	//
	nVertexCount = 0 ;
	for ( uint32_t i = 0; i < nBufVertexCount; i ++ )
	{
		uint32_t	iPrev = pWorkIndex[i * 2] ;
		ESLAssert( iPrev != (uint32_t) -1 ) ;
		ESLAssert( pWorkIndex[iPrev * 2] != (uint32_t) -1 ) ;
		if ( (pvVertex[iPrev] - pvVertex[i]).Absolute() < fpErrorGap )
		{
			uint32_t	iNext = pWorkIndex[i * 2 + 1] ;
			ESLAssert( iNext != (uint32_t) -1 ) ;
			ESLAssert( pWorkIndex[iNext * 2] != (uint32_t) -1 ) ;
			//
			ESLAssert( pWorkIndex[iPrev * 2 + 1] == i ) ;
			ESLAssert( pWorkIndex[iNext * 2] == i ) ;
			pWorkIndex[i * 2] = (uint32_t) -1 ;
			pWorkIndex[iPrev * 2 + 1] = iNext ;
			pWorkIndex[iNext * 2] = iPrev ;
		}
		else
		{
			nVertexCount ++ ;
		}
	}
	if ( nVertexCount < 3 )
	{
		return	0 ;
	}
	for ( ; ; )
	{
		//
		// 最も遠い頂点を検索（凸な頂点）
		//
		S3DVector	vCenter( 0, 0, 0 ) ;
		size_t		nCount = 0 ;
		for ( size_t i = 0; i < nBufVertexCount; i ++ )
		{
			if ( pWorkIndex[i * 2] != (uint32_t) -1 )
			{
				vCenter += pvVertex[i] ;
				nCount ++ ;
			}
		}
		ESLAssert( nCount == nVertexCount ) ;
		vCenter *= 1.0f / (float32_t) nCount ;
		//
		double		fpMax = -1.0 ;
		uint32_t	iCur = 0 ;
		for ( size_t i = 0; i < nBufVertexCount; i ++ )
		{
			if ( pWorkIndex[i * 2] != (uint32_t) -1 )
			{
				double	d = (vCenter - pvVertex[i]).Absolute() ;
				if ( d > fpMax )
				{
					fpMax = d ;
					iCur = (uint32_t) i ;
				}
			}
		}
		ESLAssert( pWorkIndex[iCur * 2] != (uint32_t) -1 ) ;
		//
		// 面の向き計算
		//
		uint32_t	iv1 = pWorkIndex[iCur * 2] ;
		uint32_t	iv2 = pWorkIndex[iCur * 2 + 1] ;
		S3DVector	v0 = pvVertex[iCur] ;
		S3DVector	v1 = pvVertex[iv1] ;
		S3DVector	v2 = pvVertex[iv2] ;
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		//
		// ほかの頂点を含んでいないか判定
		//
		if ( vValidFace.InnerProduct( vNormal ) < 0.0 )
		{
			iCur = FindValidTriangle
				( iCur, iv1, iv2, pWorkIndex, vValidFace, pvVertex, fpErrorGap ) ;
			//
			iv1 = pWorkIndex[iCur * 2] ;
			iv2 = pWorkIndex[iCur * 2 + 1] ;
			v0 = pvVertex[iCur] ;
			v1 = pvVertex[iv1] ;
			v2 = pvVertex[iv2] ;
			vNormal = (v1 - v0) * (v2 - v0) ;
		}
		else if ( IsTriangleIncludeOtherPoint
				( iCur, pWorkIndex, vNormal, pvVertex, fpErrorGap ) )
		{
			if ( IsValidTriangleFace( iv2, pWorkIndex, vValidFace, pvVertex )
				&& !IsTriangleIncludeOtherPoint
					( iv2, pWorkIndex, vNormal, pvVertex, fpErrorGap ) )
			{
				iCur = iv2 ;
			}
			else if ( IsValidTriangleFace( iv1, pWorkIndex, vValidFace, pvVertex )
				&& !IsTriangleIncludeOtherPoint
					( iv1, pWorkIndex, vNormal, pvVertex, fpErrorGap ) )
			{
				iCur = iv1 ;
			}
			else
			{
				iCur = FindValidTriangle
					( iCur, iv1, iv2, pWorkIndex, vValidFace, pvVertex, fpErrorGap ) ;
			}
			iv1 = pWorkIndex[iCur * 2] ;
			iv2 = pWorkIndex[iCur * 2 + 1] ;
			v0 = pvVertex[iCur] ;
			v1 = pvVertex[iv1] ;
			v2 = pvVertex[iv2] ;
			vNormal = (v1 - v0) * (v2 - v0) ;
		}
		//
		// １つ三角を追加
		//
		aIndexedList.Add( iCur ) ;
		aIndexedList.Add( iv2 ) ;
		aIndexedList.Add( iv1 ) ;
		//
		// 頂点を１つ削除
		//
		pWorkIndex[iCur * 2]    = (uint32_t) -1 ;
		pWorkIndex[iv1 * 2 + 1] = iv2 ;
		pWorkIndex[iv2 * 2]     = iv1 ;
		ESLAssert( pWorkIndex[iv1 * 2] != (uint32_t) -1 ) ;
		//
		if ( -- nVertexCount <= 2 )
		{
			break ;
		}
		uint32_t	iNext = iv1 ;
		iCur = iv2 ;
		do
		{
			//
			// 頂点が凸か？
			//
			iv1 = pWorkIndex[iCur * 2] ;
			iv2 = pWorkIndex[iCur * 2 + 1] ;
			if ( iv1 == (uint32_t) -1 )
			{
				break ;
			}
			ESLAssert( pWorkIndex[iv1 * 2] != (uint32_t) -1 ) ;
			ESLAssert( pWorkIndex[iv2 * 2] != (uint32_t) -1 ) ;
			v0 = pvVertex[iCur] ;
			v1 = pvVertex[iv1] ;
			v2 = pvVertex[iv2] ;
			vNormal = (v1 - v0) * (v2 - v0) ;
			//
			// この頂点は凸である
			// > 他の頂点が三角形内に含まれていないか？
			//
			if ( !IsTriangleIncludeOtherPoint
					( iCur, pWorkIndex, vNormal, pvVertex, fpErrorGap ) )
			{
				//
				// １つ三角を追加
				//
				if ( vValidFace.InnerProduct( vNormal ) >= 0.0f )
				{
					aIndexedList.Add( iCur ) ;
					aIndexedList.Add( iv2 ) ;
					aIndexedList.Add( iv1 ) ;
				}
				else
				{
					aIndexedList.Add( iCur ) ;
					aIndexedList.Add( iv1 ) ;
					aIndexedList.Add( iv2 ) ;
				}
				//
				// 頂点を１つ削除
				//
				pWorkIndex[iCur * 2]    = (uint32_t) -1 ;
				pWorkIndex[iv1 * 2 + 1] = iv2 ;
				pWorkIndex[iv2 * 2]     = iv1 ;
				//
				if ( -- nVertexCount <= 2 )
				{
					return	aIndexedList.GetLength() ;
				}
				iCur = iNext ;
				iNext = iv2 ;
			}
			else
			{
				iCur = iNext ;
				iNext = (uint32_t) -1 ;
			}
		}
		while ( iCur != (uint32_t) -1 ) ;
	}
	aWorkIndex.FinishArray() ;
	return	aIndexedList.GetLength() ;
}

bool S3DMeshShaper::IsValidTriangleFace
	( uint32_t iTriangleTop, const uint32_t * pVertexChain,
		const S3DVector& vValidNormal, const S3DVector4 * pvVertex )
{
	uint32_t	iv1 = pVertexChain[iTriangleTop * 2] ;
	uint32_t	iv2 = pVertexChain[iTriangleTop * 2 + 1] ;
	S3DVector	v0 = pvVertex[iTriangleTop] ;
	S3DVector	v1 = pvVertex[iv1] ;
	S3DVector	v2 = pvVertex[iv2] ;
	S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
	return	(vValidNormal.InnerProduct( vNormal ) >= 0.0) ;
}

uint32_t S3DMeshShaper::FindValidTriangle
	( uint32_t iv0, uint32_t iv1, uint32_t iv2,
		const uint32_t * pVertexChain,
		const S3DVector& vValidNormal,
		const S3DVector4 * pvVertex, float32_t fpErrorGap )
{
	uint32_t	i = pVertexChain[iv2 * 2 + 1] ;
	while ( i != iv1 )
	{
		uint32_t	i1 = pVertexChain[i * 2] ;
		uint32_t	i2 = pVertexChain[i * 2 + 1] ;
		S3DVector	v0 = pvVertex[i] ;
		S3DVector	v1 = pvVertex[i1] ;
		S3DVector	v2 = pvVertex[i2] ;
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		if ( (vValidNormal.InnerProduct( vNormal ) >= 0.0)
			&& !IsTriangleIncludeOtherPoint
				( i, pVertexChain, vNormal, pvVertex, fpErrorGap ) )
		{
			iv0 = i ;
			break ;
		}
		i = pVertexChain[i * 2 + 1] ;
	}
	return	iv0 ;
}

bool S3DMeshShaper::IsTriangleIncludeOtherPoint
	( uint32_t iTriangleTop,
		const uint32_t * pVertexChain,
		const S3DVector& vNormal,
		const S3DVector4 * pvVertex, float32_t fpErrorGap )
{
	const uint32_t	iv1 = pVertexChain[iTriangleTop * 2] ;
	const uint32_t	iv2 = pVertexChain[iTriangleTop * 2 + 1] ;
	const S3DVector	v0 = pvVertex[iTriangleTop] ;
	const S3DVector	v1 = pvVertex[iv1] ;
	const S3DVector	v2 = pvVertex[iv2] ;
	const S3DVector	v10 = (v1 - v0).Normalized() ;
	const S3DVector	v21 = (v2 - v1).Normalized() ;
	const S3DVector	v02 = (v0 - v2).Normalized() ;
	const float32_t	r10 = (float32_t) (v1 - v0).Absolute() - fpErrorGap ;
	const float32_t	r21 = (float32_t) (v2 - v1).Absolute() - fpErrorGap ;
	const float32_t	r02 = (float32_t) (v0 - v2).Absolute() - fpErrorGap ;
	const float32_t	fpErrorGap_p2 = fpErrorGap * fpErrorGap ;
	uint32_t		i = pVertexChain[iv2 * 2 + 1] ;
	while ( i != iv1 )
	{
		// 頂点が三角に含まれているか？
		const S3DVector	v = pvVertex[i] ;
		const S3DVector	vd0 = v0 - v ;
		const S3DVector	vd1 = v1 - v ;
		const S3DVector	vd2 = v2 - v ;
		if ( (vNormal.InnerProduct( vd1 * vd2 ) > 0.0f)
			&& (vNormal.InnerProduct( vd2 * vd0 ) > 0.0f)
			&& (vNormal.InnerProduct( vd0 * vd1 ) > 0.0f) )
		{
			return	true ;
		}
		float32_t	cos_r0 = -v10.InnerProduct(vd0) ;
		float32_t	sin_r0_p2 = vd0.InnerProduct(vd0) - cos_r0 * cos_r0 ;
		float32_t	cos_r1 = -v21.InnerProduct(vd1) ;
		float32_t	sin_r1_p2 = vd1.InnerProduct(vd1) - cos_r1 * cos_r1 ;
		float32_t	cos_r2 = -v02.InnerProduct(vd2) ;
		float32_t	sin_r2_p2 = vd2.InnerProduct(vd2) - cos_r2 * cos_r2 ;
		if ( (cos_r0 > fpErrorGap) && (cos_r0 < r10) && (sin_r0_p2 < fpErrorGap_p2)
			|| (cos_r1 > fpErrorGap) && (cos_r1 < r21) && (sin_r1_p2 < fpErrorGap_p2)
			|| (cos_r2 > fpErrorGap) && (cos_r2 < r02) && (sin_r2_p2 < fpErrorGap_p2) )
		{
			return	true ;
		}
		i = pVertexChain[i * 2 + 1] ;
	}
	return	false ;
}

// 太さを付加した線分を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::ThickLines
	( S3DRenderBufferInterface & render,
		const S3DVector & vCameraRay,
		S3DMaterial * pMaterial,
		const S3DMeshShaper::ThickLinesParam & tlpParam,
		size_t nPoints, const S3DVector4 * pvPoints,
		const float32_t * pThickness, const uint32_t * pAlphas )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcThickLinesPolygonCount( gmgp, gmp, tlpParam, nPoints ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	size_t	countVertex = gmgp.countVertex ;
	size_t	countPolygon = gmgp.countPolygon ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	//
	// メッシュ生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = NULL ;
	//
	MakeThickLinesGridMesh
		( prmbuf, vCameraRay, tlpParam,
			nPoints, pvPoints, pThickness, pAlphas ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	//
	return	GridMesh
		( render, pMaterial, gmp.nFlags, gmp.widthMesh, gmp.heightMesh,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray() ) ;
}

SGLError S3DMeshShaper::AddThickLines
	( S3DVertexBufferInterface & vbuf,
		const S3DVector & vCameraRay,
		const S3DMeshShaper::ThickLinesParam & tlpParam,
		size_t nPoints,
		const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlphas )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcThickLinesPolygonCount( gmgp, gmp, tlpParam, nPoints ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	SGLError	err = vbuf.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッシュ生成
	//
	MakeThickLinesGridMesh
		( prmbuf, vCameraRay, tlpParam,
			nPoints, pvPoints, pThickness, pAlphas ) ;
	//
	// プリミティブ追加
	//
	return	AddGridMesh( vbuf, gmgp, gmp, prmbuf ) ;
}

size_t S3DMeshShaper::CalcThickLinesPolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		S3DMeshShaper::GridMeshParam& gmp,
		const S3DMeshShaper::ThickLinesParam & tlpParam, size_t nPoints )
{
	if ( (nPoints < 2) || (tlpParam.nColorDiv < 2) )
	{
		gmgp.countVertex = 0 ;
		gmgp.countPolygon = 0 ;
		return	0 ;
	}
	size_t	stepPoints = tlpParam.nColorDiv * 2 - 1 ;
	//
	gmp.nFlags = 0 ;
	if ( tlpParam.nFlags & thickLineLoop )
	{
		gmp.nFlags |= gridVertLoop ;
	}
	gmp.widthMesh = stepPoints - 1 ;
	gmp.heightMesh = nPoints - 1 ;
	//
	return	CalcGridMeshPolygonCount
				( gmgp, gmp.nFlags, gmp.widthMesh, gmp.heightMesh );
}

void S3DMeshShaper::MakeThickLinesGridMesh
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DVector & vCameraRay,
		const S3DMeshShaper::ThickLinesParam & tlpParam,
		size_t nPoints,
		const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlphas )
{
	ThickLinesParam	tlp = tlpParam ;
	S3DVector		vCameraZ = vCameraRay ;
	S3DVector		vBaisZ ;
	vCameraZ.Normalize() ;
	vBaisZ = vCameraZ * tlp.zBais ;
	//
	size_t			stepPoints = tlp.nColorDiv * 2 - 1 ;
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	S3DVector4		vNormal = - vCameraZ ;
	S3DVector		vLastDirection = pvPoints[1] - pvPoints[0] ;
	vLastDirection.Normalize() ;
	if ( tlp.nFlags & thickLineLoop )
	{
		S3DVector	vLastDirection2 = pvPoints[0] - pvPoints[nPoints - 1] ;
		vLastDirection2.Normalize() ;
		vLastDirection = (vLastDirection + vLastDirection2) * 0.5 ;
	}
	//
	const float32_t	fpDivColor = 1.0f / (float32_t) tlp.nColorDiv ;
	const float32_t	fpStepU = tlp.uWidth / (float32_t) (stepPoints - 1) ;
	float32_t		vCoordY = tlp.vOffset ;
	//
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		//
		// ベクトル補完
		//
		S3DVector	vNextDirection ;
		S3DVector	vDirection ;
		float32_t	fpSegLength = 0.0 ;
		if ( i + 1 < nPoints )
		{
			vNextDirection = pvPoints[i + 1] - pvPoints[i] ;
			fpSegLength = (float32_t) vNextDirection.Absolute() ;
			if ( fpSegLength > 1.0e-8 )
			{
				vNextDirection *= (1.0f / fpSegLength) ;
			}
			vDirection = vLastDirection + vNextDirection ;
			vDirection.Normalize() ;
		}
		else
		{
			vNextDirection = vLastDirection ;
			vDirection = vLastDirection ;
		}
		S3DVector	vX = vCameraZ * vDirection ;
		vX.Normalize() ;
		//
		// 座標計算
		//
		S3DVector4	vPoint = pvPoints[i] ;
		vPoint += vBaisZ ;
		size_t	k = tlp.nColorDiv - 1 ;
		//
		pvVertex[k] = vPoint ;
		pvNormal[k] = vNormal ;
		pColor[k] = tlp.pColors[0] ;
		//
		float32_t	fpThickness = pThickness[i] ;
		if ( tlp.pThickDiv == NULL )
		{
			fpThickness *= fpDivColor ;
		}
		size_t		j ;
		for ( j = 1; j < tlp.nColorDiv; j ++ )
		{
			float32_t	t ;
			if ( tlp.pThickDiv != NULL )
			{
				t = tlp.pThickDiv[j] * fpThickness ;
			}
			else
			{
				t = (float32_t) j * fpThickness ;
			}
			k = tlp.nColorDiv + j - 1 ;
			pvVertex[k].x = vPoint.x + vX.x * t ;
			pvVertex[k].y = vPoint.y + vX.y * t ;
			pvVertex[k].z = vPoint.z + vX.z * t ;
			pvVertex[k].d = 0.0f ;
			pvNormal[k] = vNormal ;
			pColor[k] = tlp.pColors[j] ;
			//
			k = tlp.nColorDiv - j - 1 ;
			pvVertex[k].x = vPoint.x - vX.x * t ;
			pvVertex[k].y = vPoint.y - vX.y * t ;
			pvVertex[k].z = vPoint.z - vX.z * t ;
			pvVertex[k].d = 0.0f ;
			pvNormal[k] = vNormal ;
			pColor[k] = tlp.pColors[j] ;
		}
		//
		// UV 座標計算
		//
		float32_t	u = 0.0f ;
		for ( j = 0; j < stepPoints; j ++ )
		{
			pvUVMap[j].x = u ;
			pvUVMap[j].y = vCoordY ;
			u += fpStepU ;
		}
		//
		// 頂点α設定
		//
		if ( pAlphas != NULL )
		{
			uint32_t	a = pAlphas[i] ;
			if ( a >= 0x100 )
			{
				a = 0xFF ;
			}
			for ( j = 0; j < stepPoints; j ++ )
			{
				pColor[j].rgbMul.argb.Alpha = (uint8_t) a ;
			}
		}
		//
		vCoordY += fpSegLength * tlp.vRatio ;
		pvVertex += stepPoints ;
		pvNormal += stepPoints ;
		pvUVMap += stepPoints ;
		pColor += stepPoints ;
		//
		vLastDirection = vNextDirection ;
	}
}

// （見かけ上の）太さを付加した線分を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::RegularThickLines
	( S3DRenderBufferInterface & render,
		const S3DVector & vCameraRay,
		const S3DVector & vCameraPos,
		S3DMaterial * pMaterial,
		const ThickLinesParam & tlpParam,
		size_t nPoints, float32_t fpThicknessByZ,
		const S3DVector4 * pvPoints, const uint32_t * pAlphas )
{
	float32_t *	pThickness = m_bufThickness.GetArray( nPoints ) ;
	S3DVector	vCameraZ = vCameraRay ;
	vCameraZ.Normalize() ;
	//
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		double	z = (vCameraZ | (pvPoints[i] - vCameraPos)) ;
		pThickness[i] = (float32_t) (z * fpThicknessByZ) ;
	}
	m_bufThickness.FinishArray() ;
	//
	return	ThickLines
		( render, vCameraRay, pMaterial,
			tlpParam, nPoints, pvPoints, pThickness, pAlphas ) ;
}

// ビルボード頂点計算
//////////////////////////////////////////////////////////////////////////////
void S3DMeshShaper::BillboardParam::CalcVertex( S3DVector4 * pvVertex ) const
{
	SGLAffine	af ;
	af.SetRotation( PI * (zAngle / 180.0) ) ;
	af *= SGLAffine( vZoom.x, 0.0f, 0.0f,  0.0f, vZoom.y, 0.0f ) ;
	af *= SGLAffine( 1.0f, 0.0f, -vCenter.x,  0.0f, 1.0, -vCenter.y ) ;
	//
	ESLAssert( pImage != NULL ) ;
	SGLSize	sizeImage = pImage->GetImageSize() ;
	//
	S2DVector	vRect[4] ;
	vRect[0].x = 0 ;
	vRect[0].y = 0 ;
	vRect[1].x = (float32_t) sizeImage.w ;
	vRect[1].y = 0 ;
	vRect[2].x = 0 ;
	vRect[2].y = (float32_t) sizeImage.h ;
	vRect[3].x = (float32_t) sizeImage.w ;
	vRect[3].y = (float32_t) sizeImage.h ;
	af.TransformVectors( &vRect[0], &vRect[0], 4 ) ;
	//
	for ( size_t i = 0; i < 4; i ++ )
	{
		pvVertex[i].x = vRect[i].x ;
		pvVertex[i].y = vRect[i].y ;
		pvVertex[i].z = 0.0f ;
		pvVertex[i].d = 0.0f ;
	}
}

void S3DMeshShaper::BillboardParam::CalcVertex
	( S3DVector4 * pvVertex, const S3DMatrix & matICamera ) const
{
	SGLAffine	af ;
	af.SetRotation( PI * (zAngle / 180.0) ) ;
	af *= SGLAffine( vZoom.x, 0.0f, 0.0f,  0.0f, vZoom.y, 0.0f ) ;
	af *= SGLAffine( 1.0f, 0.0f, -vCenter.x,  0.0f, 1.0, -vCenter.y ) ;
	//
	ESLAssert( pImage != NULL ) ;
	SGLSize	sizeImage = pImage->GetImageSize() ;
	//
	S2DVector	vRect[4] ;
	vRect[0].x = 0 ;
	vRect[0].y = 0 ;
	vRect[1].x = (float32_t) sizeImage.w ;
	vRect[1].y = 0 ;
	vRect[2].x = 0 ;
	vRect[2].y = (float32_t) sizeImage.h ;
	vRect[3].x = (float32_t) sizeImage.w ;
	vRect[3].y = (float32_t) sizeImage.h ;
	af.TransformVectors( &vRect[0], &vRect[0], 4 ) ;
	//
	S3DVector	vX( matICamera.m[0][0], matICamera.m[1][0], matICamera.m[2][0] ) ;
	S3DVector	vY( matICamera.m[0][1], matICamera.m[1][1], matICamera.m[2][1] ) ;
//	S3DVector	vZ( matICamera.m[0][2], matICamera.m[1][2], matICamera.m[2][2] ) ;
//	S3DVector	vBase = vZ * zBias ;
	//
	for ( size_t i = 0; i < 4; i ++ )
	{
		float32_t	x = vRect[i].x ;
		float32_t	y = vRect[i].y ;
		pvVertex[i].x = /*vBase.x +*/ vX.x * x + vY.x * y ;
		pvVertex[i].y = /*vBase.y +*/ vX.y * x + vY.y * y ;
		pvVertex[i].z = /*vBase.z +*/ vX.z * x + vY.z * y ;
		pvVertex[i].d = 0.0f ;
	}
}

// ビルボードパーティクルを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::BillboardParticle
	( S3DRenderBufferInterface & render,
		const S3DMatrix & matICamera,
		const S3DVector & vCameraPos,
		uint64_t nDrawShadingFlag,
		const BillboardParam & bpParam, size_t nPoints,
		const S3DVector4 * pvPoints, const S3DColor * pSrcColors,
		const float32_t * pZooms, const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	if ( nPoints == 0 )
	{
		return	sglErrSuccess ;
	}
	SGLImageObject *	pImage = bpParam.pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pSrcColors != NULL )
	{
		nDrawShadingFlag |= shadingVertexAlpha ;
	}
	//
	// 一時バッファ確保
	//
	const size_t	countVertex = nPoints * 4 ;
	const size_t	countPolygon = nPoints * 2 ;
	const size_t	countIndex = countPolygon * 3 ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufIndex.GetLength() < countIndex )
	{
		m_bufIndex.FreeArray() ;
		m_bufIndex.SetLength( (countIndex + 0xFF) & ~0xFF ) ;
	}
	//
	// ビルボード頂点計算
	//
	S3DVector4	vBaseVertex[4] ;
	S2DVector	vUVMap[4] ;
	if ( pFaceDirs != NULL )
	{
		bpParam.CalcVertex( &vBaseVertex[0] ) ;
	}
	else
	{
		bpParam.CalcVertex( &vBaseVertex[0], matICamera ) ;
	}
	//
	SGLSize		sizeImage = pImage->GetImageSize() ;
	vUVMap[0].x = 0.0f ;
	vUVMap[0].y = 0.0f ;
	vUVMap[1].x = (float32_t) sizeImage.w ;
	vUVMap[1].y = 0.0f ;
	vUVMap[2].x = 0.0f ;
	vUVMap[2].y = (float32_t) sizeImage.h ;
	vUVMap[3].x = (float32_t) sizeImage.w ;
	vUVMap[3].y = (float32_t) sizeImage.h ;
	//
	// ポリゴンリスト生成
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DVector4 *	pvNormal = m_bufNormal.GetArray() ;
	S2DVector *		pvUVMap = m_bufUVMap.GetArray() ;
	S3DColor *		pColor = m_bufColor.GetArray() ;
	uint32_t *		pIndex = m_bufIndex.GetArray() ;
//	SGLImageRect *	pRect = m_bufRect.GetArray() ;
	//
	S3DVector4		vNormal = matICamera * S3DVector( 0, 0, -1 ) ;
	S3DColor		clrDummy( 0xFFFFFFFF, 0 ) ;
	S3DColor		clrTemp ;
	float32_t		zBias = bpParam.zBias ;
	//
	for ( size_t i = 0, j = 0; i < nPoints; i ++, j += 4 )
	{
		S3DVector4		vPos = pvPoints[i] ;
		if ( zBias != 0.0f )
		{
			float32_t	r ;
			vPos -= vCameraPos ;
			r = (float32_t) vPos.Absolute() ;
			if ( r != 0.0f )
			{
				vPos *= (r + zBias) / r ;
			}
			vPos += vCameraPos ;
		}
		//
		pvVertex[0] = vBaseVertex[0] ;
		pvVertex[1] = vBaseVertex[1] ;
		pvVertex[2] = vBaseVertex[2] ;
		pvVertex[3] = vBaseVertex[3] ;
		if ( pZooms != NULL )
		{
			float32_t	zoom = pZooms[i] ;
			pvVertex[0] *= zoom ;
			pvVertex[1] *= zoom ;
			pvVertex[2] *= zoom ;
			pvVertex[3] *= zoom ;
		}
		if ( pFaceDirs != NULL )
		{
			S3DMatrix	matFace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			S3DVector	vZero( 0, 0, 0 ) ;
			matFace.RevolveForAngle( pFaceDirs[i] ) ;
			matFace.RevolveOnZ( sin(pFaceDirs[i].w), cos(pFaceDirs[i].w) ) ;
			if ( pxAspect != NULL )
			{
				matFace.MagnifyByVector( S3DVector( pxAspect[i], 1.0f, 1.0f ) ) ;
			}
			matFace.RevolveVectors( pvVertex, pvVertex, 4, vZero ) ;
		}
		pvVertex[0] += vPos ;
		pvVertex[1] += vPos ;
		pvVertex[2] += vPos ;
		pvVertex[3] += vPos ;
		//
		pvNormal[0] = vNormal ;
		pvNormal[1] = vNormal ;
		pvNormal[2] = vNormal ;
		pvNormal[3] = vNormal ;
		//
		pvUVMap[0] = vUVMap[0] ;
		pvUVMap[1] = vUVMap[1] ;
		pvUVMap[2] = vUVMap[2] ;
		pvUVMap[3] = vUVMap[3] ;
		//
		clrTemp = clrDummy ;
		if ( pSrcColors != NULL )
		{
			clrTemp = pSrcColors[i] ;
		}
		pColor[0] = clrTemp ;
		pColor[1] = clrTemp ;
		pColor[2] = clrTemp ;
		pColor[3] = clrTemp ;
		//
		pIndex[0] = (uint32_t) j ;
		pIndex[1] = (uint32_t) j + 1 ;
		pIndex[2] = (uint32_t) j + 2 ;
		pIndex[3] = (uint32_t) j + 1 ;
		pIndex[4] = (uint32_t) j + 3 ;
		pIndex[5] = (uint32_t) j + 2 ;
		//
		pvVertex += 4 ;
		pvNormal += 4 ;
		pvUVMap += 4 ;
		pColor += 4 ;
		pIndex += 6 ;
	}
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndex.FinishArray() ;
	//
	// マテリアル取得
	//
	SGLImageRect	rectRef ;
	S3DMaterial *	pMaterial =
		SGLImageNoShadeMaterialInterface::GetMaterialBy
			( pImage, shadingTextureSmoothing | nDrawShadingFlag, &rectRef ) ;
	if ( rectRef.x | rectRef.y )
	{
		S2DVector	vUVOffset( (float32_t) rectRef.x, (float32_t) rectRef.y ) ;
		size_t		nPointsX4 = nPoints * 4 ;
		pvUVMap = m_bufUVMap.GetArray() ;
		for ( size_t i = 0; i < nPointsX4; i ++ )
		{
			pvUVMap[i] += vUVOffset ;
		}
		m_bufUVMap.FinishArray() ;
	}
	//
	// ポリゴンリスト出力
	//
	return	render.AddIndexedTriangleList
		( pMaterial, 0, countPolygon, countVertex,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray(),
			m_bufIndex.GetConstArray() ) ;
}

// アニメーション画像ビルボードパーティクルを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::AnimationBillboardParticle
	( S3DRenderBufferInterface & render,
		const S3DMatrix & matICamera,
		const S3DVector & vCameraPos,
		uint64_t nDrawShadingFlag,
		const S3DMeshShaper::BillboardParam & bpParam,
		size_t nPoints, const S3DVector4 * pvPoints,
		const size_t * pFrames, const S3DColor * pSrcColors,
		const float32_t * pZooms, const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	//
	// フレームマップを取得
	//
	if ( nPoints == 0 )
	{
		return	sglErrSuccess ;
	}
	SGLImageObject *	pImage = bpParam.pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pSrcColors != NULL )
	{
		nDrawShadingFlag |= shadingVertexAlpha ;
	}
	const size_t	nFrameCount = pImage->GetFrameCount() ;
	if ( m_bufRect.GetLength() < nFrameCount )
	{
		m_bufRect.FreeArray() ;
		m_bufRect.SetLength( (nFrameCount + 0xF) & ~0xF ) ;
	}
	SGLImageObject *	pAnime =
			pImage->NewAnimationReference
					( m_bufRect.GetArray(), nFrameCount ) ;
	m_bufRect.FinishArray() ;
	//
	if ( pAnime == NULL )
	{
		//
		// アニメーション画像は1枚の画像に統合されていないので
		// 複数に分けて描画する
		//
		ESLTrace( "failed to NewAnimationReference "
					"in S3DMeshShaper::AnimationBillboardParticle.\n" ) ;
		SPointerArray<SGLImageObject>	lstFrames ;
		S3DMeshShaper::BillboardParam	bpParamFrame = bpParam ;
		lstFrames.SetLimit( nFrameCount ) ;
		//
		for ( size_t i = 0; i < nPoints; i ++ )
		{
			size_t	iFrame = pFrames[i] ;
			size_t	nCount = 1 ;
			while ( (i + nCount < nPoints)
					&& (pFrames[i + nCount] == iFrame) )
			{
				nCount ++ ;
			}
			SGLImageObject *	pFrame = lstFrames.GetAt( iFrame ) ;
			if ( pFrame == NULL )
			{
				pFrame = pImage->NewReference( NULL, (ssize_t) iFrame ) ;
				lstFrames.SetAt( iFrame, pFrame ) ;
				render.AddTemporaryObject( pFrame ) ;
			}
			bpParamFrame.pImage = pFrame ;
			//
			BillboardParticle
				( render, matICamera, vCameraPos,
					nDrawShadingFlag, bpParamFrame, nCount,
					pvPoints + i,
					((pSrcColors != NULL) ? (pSrcColors + i) : NULL),
					((pZooms != NULL) ? (pZooms + i) : NULL),
					((pFaceDirs != NULL) ? (pFaceDirs + i) : NULL),
					((pxAspect != NULL) ? (pxAspect + i) : NULL) ) ;
			//
			i += nCount - 1 ;
		}
		return	sglErrSuccess ;
	}
	//
	// 一時バッファ確保
	//
	const size_t	countVertex = nPoints * 4 ;
	const size_t	countPolygon = nPoints * 2 ;
	const size_t	countIndex = countPolygon * 3 ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufIndex.GetLength() < countIndex )
	{
		m_bufIndex.FreeArray() ;
		m_bufIndex.SetLength( (countIndex + 0xFF) & ~0xFF ) ;
	}
	//
	// マテリアル取得
	//
	S3DMaterial *	pMaterial =
		SGLImageNoShadeMaterialInterface::GetMaterialBy
				( pAnime, shadingTextureSmoothing | nDrawShadingFlag, NULL ) ;
	render.AddTemporaryObject( pAnime ) ;
	//
	// ポリゴンリスト生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = m_bufIndex.GetArray() ;
	//
	MakeAnimationBillboardParticle
		( prmbuf, matICamera, vCameraPos, bpParam,
			m_bufRect.GetArray(), nFrameCount,
			nPoints, pvPoints, pFrames,
			pSrcColors, pZooms, pFaceDirs, pxAspect ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndex.FinishArray() ;
	//
	// ポリゴンリスト出力
	//
	return	render.AddIndexedTriangleList
		( pMaterial, 0,
			countPolygon, countVertex,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray(),
			m_bufIndex.GetConstArray() ) ;
}

SGLImageRect * S3DMeshShaper::GetImageRectBuffer( size_t nFrameCount )
{
	if ( m_bufRect.GetLength() < nFrameCount )
	{
		m_bufRect.FreeArray() ;
		m_bufRect.SetLength( (nFrameCount + 0xF) & ~0xF ) ;
	}
	return	m_bufRect.GetArray() ;
}

void S3DMeshShaper::MakeAnimationBillboardParticle
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DMatrix & matICamera,
		const S3DVector & vCameraPos,
		const S3DMeshShaper::BillboardParam & bpParam,
		const SGLImageRect * pFrameRect,
		size_t nFrameCount,
		size_t nPoints, const S3DVector4 * pvPoints,
		const size_t * pFrames, const S3DColor * pColors,
		const float32_t * pZooms, const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	//
	// ビルボード頂点計算
	//
	S3DVector4	vBaseVertex[4] ;
	if ( pFaceDirs != NULL )
	{
		bpParam.CalcVertex( &vBaseVertex[0] ) ;
	}
	else
	{
		bpParam.CalcVertex( &vBaseVertex[0], matICamera ) ;
	}
	//
	// ポリゴンリスト生成
	//
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	uint32_t *		pIndex = prmbuf.pIndexedList ;
	//
	S3DVector4		vNormal = matICamera * S3DVector( 0, 0, -1 ) ;
	S3DColor		clrDummy( 0xFFFFFFFF, 0 ) ;
	S3DColor		clrTemp ;
	float32_t		zBias = bpParam.zBias ;
	//
	size_t	iLast = 0 ;
	for ( size_t i = 0, j = 0; i < nPoints; i ++, j += 4 )
	{
		size_t		iFrame = pFrames[i] ;
		S3DVector4	vPos = pvPoints[i] ;
		ESLAssert( !vPos.IsNaN() ) ;
		ESLAssert( iFrame < nFrameCount ) ;
		if ( iFrame >= nFrameCount )
		{
			iFrame = 0 ;
		}
		SGLImageRect	rect = pFrameRect[iFrame] ;
		//
		if ( zBias != 0.0f )
		{
			float32_t	r ;
			vPos -= vCameraPos ;
			r = (float32_t) vPos.Absolute() ;
			if ( r != 0.0f )
			{
				vPos *= (r + zBias) / r ;
			}
			vPos += vCameraPos ;
		}
		pvVertex[0] = vBaseVertex[0] ;
		pvVertex[1] = vBaseVertex[1] ;
		pvVertex[2] = vBaseVertex[2] ;
		pvVertex[3] = vBaseVertex[3] ;
		if ( pZooms != NULL )
		{
			float32_t	zoom = pZooms[i] ;
			pvVertex[0] *= zoom ;
			pvVertex[1] *= zoom ;
			pvVertex[2] *= zoom ;
			pvVertex[3] *= zoom ;
		}
		if ( pFaceDirs != NULL )
		{
			S3DMatrix	matFace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			S3DVector	vZero( 0, 0, 0 ) ;
			matFace.RevolveForAngle( pFaceDirs[i] ) ;
			matFace.RevolveOnZ( sin(pFaceDirs[i].w), cos(pFaceDirs[i].w) ) ;
			if ( pxAspect != NULL )
			{
				matFace.MagnifyByVector( S3DVector( pxAspect[i], 1.0f, 1.0f ) ) ;
			}
			matFace.RevolveVectors( pvVertex, pvVertex, 4, vZero ) ;
		}
		pvVertex[0] += vPos ;
		pvVertex[1] += vPos ;
		pvVertex[2] += vPos ;
		pvVertex[3] += vPos ;
		//
		pvNormal[0] = vNormal ;
		pvNormal[1] = vNormal ;
		pvNormal[2] = vNormal ;
		pvNormal[3] = vNormal ;
		//
		pvUVMap[0].x = (float32_t) rect.x ;
		pvUVMap[0].y = (float32_t) rect.y ;
		pvUVMap[3].x = (float32_t) (rect.x + rect.w) ;
		pvUVMap[3].y = (float32_t) (rect.y + rect.h) ;
		pvUVMap[1].x = pvUVMap[3].x ;
		pvUVMap[1].y = pvUVMap[0].y ;
		pvUVMap[2].x = pvUVMap[0].x ;
		pvUVMap[2].y = pvUVMap[3].y ;
		//
		clrTemp = clrDummy ;
		if ( pColors != NULL )
		{
			clrTemp = pColors[i] ;
		}
		pColor[0] = clrTemp ;
		pColor[1] = clrTemp ;
		pColor[2] = clrTemp ;
		pColor[3] = clrTemp ;
		//
		uint32_t	k = (uint32_t) (j - iLast * 4) ;
		pIndex[0] = k ;
		pIndex[1] = k + 1 ;
		pIndex[2] = k + 2 ;
		pIndex[3] = k + 1 ;
		pIndex[4] = k + 3 ;
		pIndex[5] = k + 2 ;
		//
		pvVertex += 4 ;
		pvNormal += 4 ;
		pvUVMap += 4 ;
		pColor += 4 ;
		pIndex += 6 ;
	}
}

// 汎用ビルボード
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::RenderMultiBillboards
	( S3DRenderBufferInterface & render,
		const S3DMeshShaper::BillboardRenderParam& brp,
		size_t nBillboardCount,
		const S3DMeshShaper::BillboardEntry * pBillboards )
{
	SGLImageObject *	pAtlasImage = nullptr ;
	SGLImageRect *		pAtlasRects = GetImageRectBuffer( nBillboardCount ) ;
	//
	size_t	iLastEntry = 0 ;
	size_t	iNextEntry = 0 ;
	while ( iNextEntry < nBillboardCount )
	{
		const BillboardEntry&	be = pBillboards[iNextEntry] ;
		ESLAssert( be.pImage != nullptr ) ;
		SGLImageObject *	pImage =
			(be.pImage == nullptr) ? nullptr :
			be.pImage->GetImageReference( pAtlasRects[iNextEntry], be.iFrame ) ;
		if ( pImage != pAtlasImage )
		{
			if ( (pAtlasImage != nullptr) && (iNextEntry > iLastEntry) )
			{
				RenderBillboardsOfAtlasImage
					( render, brp, pAtlasImage, iNextEntry - iLastEntry,
						pAtlasRects + iLastEntry, pBillboards + iLastEntry ) ;
			}
			pAtlasImage = pImage ;
			iLastEntry = iNextEntry ;
		}
		iNextEntry ++ ;
	}
	if ( (pAtlasImage != nullptr) && (iNextEntry > iLastEntry) )
	{
		RenderBillboardsOfAtlasImage
			( render, brp, pAtlasImage, iNextEntry - iLastEntry,
				pAtlasRects + iLastEntry, pBillboards + iLastEntry ) ;
	}
	return	sglErrSuccess ;
}

SGLError S3DMeshShaper::RenderBillboardsOfAtlasImage
	( S3DRenderBufferInterface & render,
		const S3DMeshShaper::BillboardRenderParam& brp,
		SGLImageObject * pAtlasImage,
		size_t nBillboardCount,
		const SGLImageRect * pAtlasRects,
		const S3DMeshShaper::BillboardEntry * pBillboards )
{
	//
	// マテリアル取得
	//
	S3DMaterial *	pMaterial =
		SGLImageNoShadeMaterialInterface::GetMaterialBy
				( pAtlasImage, brp.flagsShadingOpt, nullptr ) ;
	//
	// バッファ確保
	//
	const size_t	countVertex = nBillboardCount * 4 ;
	const size_t	countPolygon = nBillboardCount * 2 ;
	const size_t	countIndex = countPolygon * 3 ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufIndex.GetLength() < countIndex )
	{
		m_bufIndex.FreeArray() ;
		m_bufIndex.SetLength( (countIndex + 0xFF) & ~0xFF ) ;
	}
	//
	// 計算準備
	//
	S3DMatrix	matICamera = brp.matICamera ;
	matICamera.RevolveOnZ( sin(brp.zAngle*PI/180.0), cos(brp.zAngle*PI/180.0) ) ;
	//
	S2DVector	vUVScale( 1.0f, 1.0f ) ;
	SGLSize		sizeImage = pAtlasImage->GetImageSize() ;
	if ( pMaterial->m_attrSurface.flagsShading & shadingNormalizedUVScale )
	{
		vUVScale.x = 1.0f / (float32_t) sizeImage.w ;
		vUVScale.y = 1.0f / (float32_t) sizeImage.h ;
	}
	//
	// 頂点計算
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DVector4 *	pvNormal = m_bufNormal.GetArray() ;
	S2DVector *		pvUVMap = m_bufUVMap.GetArray() ;
	S3DColor *		pColor = m_bufColor.GetArray() ;
	uint32_t *		pIndex = m_bufIndex.GetArray() ;
	//
	for ( size_t i = 0; i < nBillboardCount; i ++ )
	{
		const S3DMeshShaper::BillboardEntry&	be = pBillboards[i] ;
		const SGLImageRect&						rect = pAtlasRects[i] ;
		//
		// 座標
		S3DVector	vPosition = be.vPosition - brp.vCameraPos ;
		double		rPos = vPosition.Absolute() ;
		if ( rPos > brp.zBias )
		{
			vPosition *= (float32_t) ((rPos + brp.zBias) / rPos) ;
		}
		vPosition += brp.vCameraPos ;
		//
		// 回転行列
		S3DMatrix	matRotate = matICamera ;
		if ( brp.flagsBillboard & billboardWithFaceDir )
		{
			S3DMatrix	matFace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			matRotate.RevolveForAngle( be.vFaceDir ) ;
			matRotate.RevolveOnZ( sin(be.vFaceDir.w), cos(be.vFaceDir.w) ) ;
		}
		//
		// 頂点座標
		pvVertex[0].x = - be.vOffset.x ;
		pvVertex[0].y = - be.vOffset.y ;
		pvVertex[1].x = - be.vOffset.x + (float32_t) be.rectImage.w ;
		pvVertex[1].y = - be.vOffset.y ;
		pvVertex[2].x = - be.vOffset.x ;
		pvVertex[2].y = - be.vOffset.y + (float32_t) be.rectImage.h ;
		pvVertex[3].x = pvVertex[1].x ;
		pvVertex[3].y = pvVertex[2].y ;
		//
		for ( size_t j = 0; j < 4; j ++ )
		{
			pvVertex[j].x = pvVertex[j].x * be.vZoom.x ;
			pvVertex[j].y = pvVertex[j].y * be.vZoom.y ;
			pvVertex[j].z = 0.0f ;
		}
		matRotate.RevolveVectors( pvVertex, pvVertex, 4, vPosition ) ;
		//
		// UV
		pvUVMap[0].x = (float32_t) (rect.x + be.rectImage.x) ;
		pvUVMap[0].y = (float32_t) (rect.y + be.rectImage.y) ;
		pvUVMap[1].x = (float32_t) (rect.x + be.rectImage.x + be.rectImage.w) ;
		pvUVMap[1].y = pvUVMap[0].y ;
		pvUVMap[2].x = pvUVMap[0].x ;
		pvUVMap[2].y = (float32_t) (rect.y + be.rectImage.y + be.rectImage.h) ;
		pvUVMap[3].x = pvUVMap[1].x ;
		pvUVMap[3].y = pvUVMap[2].y ;
		//
		if ( pMaterial->m_attrSurface.flagsShading & shadingNormalizedUVScale )
		{
			for ( size_t j = 0; j < 4; j ++ )
			{
				pvUVMap[j].x *= vUVScale.x ;
				pvUVMap[j].y *= vUVScale.y ;
			}
		}
		//
		// 法線
		S3DVector	vNormal = matRotate * S3DVector( 0, 0, -1 ) ;
		pvNormal[0] = vNormal ;
		pvNormal[1] = vNormal ;
		pvNormal[2] = vNormal ;
		pvNormal[3] = vNormal ;
		//
		// 頂点色
		pColor[0] = be.clrEffect ;
		pColor[1] = be.clrEffect ;
		pColor[2] = be.clrEffect ;
		pColor[3] = be.clrEffect ;
		//
		// インデックス
		uint32_t	iBase = (uint32_t) i * 4 ;
		pIndex[0] = iBase ;
		pIndex[1] = iBase + 2 ;
		pIndex[2] = iBase + 3 ;
		pIndex[3] = iBase ;
		pIndex[4] = iBase + 3 ;
		pIndex[5] = iBase + 1 ;
		//
		pvVertex += 4 ;
		pvNormal += 4 ;
		pvUVMap += 4 ;
		pColor += 4 ;
		pIndex += 6 ;
	}
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndex.FinishArray() ;
	//
	// 三角ポリゴンリスト出力
	//
	return	render.AddIndexedTriangleList
		( pMaterial, 0, countPolygon, countVertex,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray(),
			m_bufIndex.GetConstArray() ) ;
}

// 立方体を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::Cube
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial,
		const S3DVector & vPosition, const S3DVector & vSize )
{
	S3DVector	vCube[8] =
	{
		S3DVector( 1, -1, 1 ),
		S3DVector( 1, -1, -1 ),
		S3DVector( -1, -1, -1 ),
		S3DVector( -1, -1, 1 ),
		S3DVector( 1, 1, 1 ),
		S3DVector( 1, 1, -1 ),
		S3DVector( -1, 1, -1 ),
		S3DVector( -1, 1, 1 ),
	} ;
	S3DVector	vFace[6] =
	{
		S3DVector( 0, -1, 0 ),
		S3DVector( 0, 0, 1 ),
		S3DVector( 1, 0, 0 ),
		S3DVector( 0, 0, -1 ),
		S3DVector( -1, 0, 0 ),
		S3DVector( 0, 1, 0 ),
	} ;
	size_t	nCubeIndex[12][3] =
	{
		{ 0, 3, 2 }, { 0, 2, 1 },
		{ 0, 4, 7 }, { 0, 7, 3 },
		{ 1, 5, 4 }, { 1, 4, 0 },
		{ 2, 6, 5 }, { 2, 5, 1 },
		{ 3, 7, 6 }, { 3, 6, 2 },
		{ 7, 4, 5 }, { 7, 5, 6 },
	} ;
	S3DVector4	vVertex[12][3] ;
	S3DVector4	vNormal[12][3] ;
	uint32_t	nIndex[12][3] ;
	//
	for ( int i = 0; i < 6; i ++ )
	{
		vCube[i].x *= vSize.x ;
		vCube[i].y *= vSize.y ;
		vCube[i].z *= vSize.z ;
		vCube[i] += vPosition ;
	}
	for ( int i = 0; i < 12; i ++ )
	{
		for ( int j = 0; j < 3; j ++ )
		{
			size_t	k = nCubeIndex[i][j] ;
			vVertex[i][j] = vCube[k] ;
			vNormal[i][j] = vFace[i >> 1] ;
			nIndex[i][j] = (uint32_t) (i * 3 + j) ;
		}
	}
	return	render.AddIndexedTriangleList
				( pMaterial, 0, 12, 12 * 3,
					&vVertex[0][0], &vNormal[0][0],
					NULL, NULL, &nIndex[0][0] ) ;
}

// 太さのあるリングを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::ThickRing
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial,
		const S3DMeshShaper::ThickRingParam & trpParam )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcThickRingPolygonCount( gmgp, gmp, trpParam ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	const size_t	countVertex = gmgp.countVertex ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	//
	// メッシュ生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = NULL ;
	//
	MakeThickRingGridMesh( prmbuf, trpParam ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	//
	return	GridMesh
		( render, pMaterial, gmp.nFlags,
			gmp.widthMesh, gmp.heightMesh,
			prmbuf.pvVertex, prmbuf.pvNormal,
			prmbuf.pvUVMap, prmbuf.pColor ) ;
}

SGLError S3DMeshShaper::AddThickRing
	( S3DVertexBufferInterface & vbuf,
		const S3DMeshShaper::ThickRingParam & trpParam )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcThickRingPolygonCount( gmgp, gmp, trpParam ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	SGLError	err = vbuf.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッシュ生成
	//
	MakeThickRingGridMesh( prmbuf, trpParam ) ;
	//
	// プリミティブ追加
	//
	return	AddGridMesh( vbuf, gmgp, gmp, prmbuf ) ;
}

size_t S3DMeshShaper::CalcThickRingPolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		S3DMeshShaper::GridMeshParam& gmp,
		const S3DMeshShaper::ThickRingParam & trpParam )
{
	if ( (trpParam.nDivision < 3) || (trpParam.nColorDiv < 2) )
	{
		gmgp.countPolygon = 0 ;
		gmgp.countVertex = 0 ;
		return	0 ;
	}
	const size_t	nColorDiv = trpParam.nColorDiv ;
	const size_t	nDivision = trpParam.nDivision ;
	//
	gmp.nFlags = gridHorzLoop ;
	gmp.widthMesh = nDivision - 1 ;
	gmp.heightMesh = nColorDiv - 1 ;
	//
	return	CalcGridMeshPolygonCount
				( gmgp, gmp.nFlags, gmp.widthMesh, gmp.heightMesh ) ;
}

void S3DMeshShaper::MakeThickRingGridMesh
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DMeshShaper::ThickRingParam & trpParam )
{
	const size_t	nColorDiv = trpParam.nColorDiv ;
	const size_t	nDivision = trpParam.nDivision ;
	//
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	S3DVector4		vNormal = trpParam.vNormal ;
	//
	S3DMatrix	matRing( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	matRing.RevolveForAngle( trpParam.vNormal ) ;
	matRing.MagnifyByVector
		( S3DVector( trpParam.vZoom.x, trpParam.vZoom.y, 1.0 ) ) ;
	//
	const double	fpDeltaRad = PI * 2.0f / (nDivision - 1) ;
	const float32_t	fpDeltaThick =
						trpParam.fpThickness / (float32_t) (nColorDiv - 1) ;
	const float32_t	fpDeltaCoordX =
						trpParam.uvScale.x / (float32_t) (nDivision - 1) ;
	const float32_t	fpDeltaCoordY =
						trpParam.uvScale.y / (float32_t) (nColorDiv - 1) ;
	//
	float32_t	fpRadius = trpParam.fpRadius + trpParam.fpThickness ;
	float32_t	fpCoordY = 0.0f ;
	for ( size_t i = 0; i < nColorDiv; i ++ )
	{
		S3DColor	clr( 0xFFFFFFFF, 0 ) ;
		if ( trpParam.pColors != NULL )
		{
			clr = trpParam.pColors[i] ;
		}
		double		rad = 0.0 ;
		float32_t	fpCoordX = 0.0f ;
		for ( size_t j = 0; j < nDivision; j ++ )
		{
			S3DVector4	vPos
				( fpRadius * cos(rad), fpRadius * sin(rad), 0 ) ;
			matRing.RevolveVector( vPos ) ;
			vPos += trpParam.vCenter ;
			*(pvVertex ++) = vPos ;
			*(pvNormal ++) = vNormal ;
			pvUVMap->x = fpCoordX ;
			pvUVMap->y = fpCoordY ;
			pvUVMap ++ ;
			*(pColor ++) = clr ;
			//
			rad += fpDeltaRad ;
			fpCoordX += fpDeltaCoordX ;
		}
		fpRadius -= fpDeltaThick ;
		fpCoordY += fpDeltaCoordY ;
	}
}

// 円柱を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::Cylinder
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial,
		const S3DMeshShaper::CylinderParam & cpParam, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcCylinderPolygonCount( gmgp, gmp, cpParam, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	const size_t	countVertex = gmgp.countVertex ;
	const size_t	countPolygon = gmgp.countPolygon ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	//
	// メッシュ生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = NULL ;
	//
	MakeCylinderGridMesh( prmbuf, cpParam, fBackface ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	//
	return	GridMesh
		( render, pMaterial, gmp.nFlags,
			gmp.widthMesh, gmp.heightMesh,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray() ) ;
}

SGLError S3DMeshShaper::AddCylinder
	( S3DVertexBufferInterface & vbuf,
		const S3DMeshShaper::CylinderParam & cpParam, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcCylinderPolygonCount( gmgp, gmp, cpParam, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	SGLError	err = vbuf.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッシュ生成
	//
	MakeCylinderGridMesh( prmbuf, cpParam, fBackface ) ;
	//
	// プリミティブ追加
	//
	return	AddGridMesh( vbuf, gmgp, gmp, prmbuf ) ;
}

size_t S3DMeshShaper::CalcCylinderPolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		S3DMeshShaper::GridMeshParam& gmp,
		const S3DMeshShaper::CylinderParam & cpParam, bool fBackface )
{
	if ( (cpParam.hDivision < 1) || (cpParam.vDivision < 2) )
	{
		gmgp.countPolygon = 0 ;
		gmgp.countVertex = 0 ;
		return	0 ;
	}
	gmp.nFlags = gridHorzLoop ;
	if ( fBackface )
	{
		gmp.nFlags |= gridBackface ;
	}
	gmp.widthMesh = cpParam.hDivision - 1 ;
	gmp.heightMesh = cpParam.vDivision ;
	//
	return	CalcGridMeshPolygonCount
				( gmgp, gmp.nFlags, gmp.widthMesh, gmp.heightMesh ) ;
}

void S3DMeshShaper::MakeCylinderGridMesh
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DMeshShaper::CylinderParam & cpParam, bool fBackface )
{
	CylinderParam	cp = cpParam ;
	//
	const size_t	countVertex = cp.hDivision * (cp.vDivision + 1) ;
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	S3DColor		clrVertex = cpParam.colorBase ;
	//
	const double	radDeltaX = PI * 2.0 / (double) cp.hDivision ;
	const double	deltaY = cp.fpHeight / (double) cp.vDivision ;
	const float32_t	fpDeltaU = cp.uvScale.x / (float32_t) cp.hDivision ;
	const float32_t	fpDeltaV = cp.uvScale.y / (float32_t) cp.vDivision ;
	float32_t		fpV = cp.uvOffset.y ;
	S3DVector		vCenter = cp.vCenter ;
	//
	S3DMatrix		matAngle( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	cp.vDirection.Normalize() ;
	matAngle.RevolveForAngle( cp.vDirection ) ;
	//
	for ( size_t i = 0; i <= cp.vDivision; i ++ )
	{
		float32_t	fpU = cp.uvOffset.x ;
		double		radX = 0.0 ;
		//
		for( size_t j = 0; j < cp.hDivision; j ++ )
		{
			S3DVector	vRing ;
			vRing.x = (float32_t) (cp.fpRadius * cos(radX)) ;
			vRing.y = (float32_t) (cp.fpRadius * sin(radX)) ;
			vRing.z = 0 ;
			matAngle.RevolveVector( vRing ) ;
			//
			pvVertex->x = vCenter.x + vRing.x ;
			pvVertex->y = vCenter.y + vRing.y ;
			pvVertex->z = vCenter.z + vRing.z ;
			pvVertex->d = 0.0f ;
			//
			*pvNormal = vRing ;
			pvNormal->Normalize() ;
			//
			pvUVMap->x = fpU ;
			pvUVMap->y = fpV ;
			//
			*pColor = clrVertex ;
			//
			pvVertex ++ ;
			pvNormal ++ ;
			pvUVMap ++ ;
			pColor ++ ;
			//
			fpU += fpDeltaU ;
			radX += radDeltaX ;
		}
		vCenter += cp.vDirection * deltaY ;
		fpV += fpDeltaV ;
	}
	if ( fBackface )
	{
		pvNormal = prmbuf.pvNormal ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pvNormal->x = - pvNormal->x ;
			pvNormal->y = - pvNormal->y ;
			pvNormal->z = - pvNormal->z ;
			pvNormal ++ ;
		}
	}
}

// 球を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::Sphere
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial,
		const S3DMeshShaper::SphereParam & spParam, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcSpherePolygonCount( gmgp, gmp, spParam, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	const size_t	countVertex = gmgp.countVertex ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	//
	// メッシュ生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = NULL ;
	//
	MakeSphereGridMesh( prmbuf, spParam, fBackface ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	//
	return	GridMesh
		( render, pMaterial, gmp.nFlags,
			gmp.widthMesh, gmp.heightMesh,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray() ) ;
}

SGLError S3DMeshShaper::AddSphere
	( S3DVertexBufferInterface & vbuf,
		const S3DMeshShaper::SphereParam & spParam, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcSpherePolygonCount( gmgp, gmp, spParam, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	SGLError	err = vbuf.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッシュ生成
	//
	MakeSphereGridMesh( prmbuf, spParam, fBackface ) ;
	//
	// プリミティブ追加
	//
	return	AddGridMesh( vbuf, gmgp, gmp, prmbuf ) ;
}

size_t S3DMeshShaper::CalcSpherePolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		S3DMeshShaper::GridMeshParam& gmp,
		const S3DMeshShaper::SphereParam & spParam, bool fBackface )
{
	if ( (spParam.hDivision < 1) || (spParam.vDivision < 2) )
	{
		gmgp.countPolygon = 0 ;
		gmgp.countVertex = 0 ;
		return	0 ;
	}
	gmp.nFlags = 0 ;
	if ( fBackface )
	{
		gmp.nFlags |= gridBackface ;
	}
	gmp.widthMesh = spParam.hDivision ;
	gmp.heightMesh = spParam.vDivision ;
	//
	return	CalcGridMeshPolygonCount
				( gmgp, gmp.nFlags, gmp.widthMesh, gmp.heightMesh ) ;
}

void S3DMeshShaper::MakeSphereGridMesh
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DMeshShaper::SphereParam & spParam, bool fBackface )
{
	const SphereParam	sp = spParam ;
	//
	const size_t	countVertex = (sp.hDivision + 1) * (sp.vDivision + 1) ;
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	S3DColor		clrVertex = spParam.colorBase ;
	PFUNC_SPHERE_COLOR_MAP
					pfnColorMap = spParam.pfnColorMap ;
	void *			pColorMapInstance = spParam.pColorMapInstance ;
	S3DColor		clrTemp ;
	//
	const double	radDeltaX = PI * 2.0 / (double) sp.hDivision ;
	const double	radDeltaY = PI * (sp.degEndLatitude - sp.degStartLatitude)
										/ ((double) sp.vDivision * 180.0) ;
	const float32_t	fpDeltaU = sp.uvScale.x / (float32_t) sp.hDivision ;
	const float32_t	fpDeltaV = sp.uvScale.y / (float32_t) sp.vDivision ;
	double			radY = PI * (sp.degStartLatitude + 90.0) / 180.0 ;
	double			radYOffset = 90.0 * PI / 180.0 ;
	float32_t		fpV = sp.uvOffset.y ;
	//
	for ( size_t i = 0; i <= sp.vDivision; i ++ )
	{
		float32_t	y = (float32_t) (sp.fpRadius * sp.vSizeScale.y * cos(radY)) ;
		double		w = (-sp.fpRadius * sin(radY)) ;
		float32_t	fpU = sp.uvOffset.x ;
		double		radX = 0.0 ;
		//
		for( size_t j = 0; j <= sp.hDivision; j ++ )
		{
			float32_t	x = (float32_t) (w * sp.vSizeScale.x * cos(radX)) ;
			float32_t	z = (float32_t) (w * sp.vSizeScale.z * sin(radX)) ;
			//
			pvVertex->x = x ;
			pvVertex->y = y ;
			pvVertex->z = z ;
			pvVertex->d = 0.0f ;
			//
			*pvNormal = *pvVertex ;
			pvNormal->Normalize() ;
			//
			pvUVMap->x = fpU ;
			pvUVMap->y = fpV ;
			//
			clrTemp = clrVertex ;
			if ( pfnColorMap != NULL )
			{
				pfnColorMap
					( clrTemp, pColorMapInstance, sp, radX, radY - radYOffset ) ;
			}
			*pColor = clrTemp ;
			//
			pvVertex ++ ;
			pvNormal ++ ;
			pvUVMap ++ ;
			pColor ++ ;
			//
			fpU += fpDeltaU ;
			radX += radDeltaX ;
		}
		radY += radDeltaY ;
		fpV += fpDeltaV ;
	}
	if ( fBackface )
	{
		pvNormal = prmbuf.pvNormal ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pvNormal->x = - pvNormal->x ;
			pvNormal->y = - pvNormal->y ;
			pvNormal->z = - pvNormal->z ;
			pvNormal ++ ;
		}
	}
}

// チューブを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshShaper::Tube
	( S3DRenderBufferInterface & render,
		S3DMaterial * pMaterial,
		const TubeParam & tpParam,
		const S3DVector& vHandle,
		size_t nPoints, const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlphas, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcTubePolygonCount
		( gmgp, gmp, tpParam, nPoints, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	size_t	countVertex = gmgp.countVertex ;
	//
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.FreeArray() ;
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufNormal.GetLength() < countVertex )
	{
		m_bufNormal.FreeArray() ;
		m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufUVMap.GetLength() < countVertex )
	{
		m_bufUVMap.FreeArray() ;
		m_bufUVMap.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	if ( m_bufColor.GetLength() < countVertex )
	{
		m_bufColor.FreeArray() ;
		m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	//
	// メッシュ生成
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = m_bufVertex.GetArray() ;
	prmbuf.pvNormal = m_bufNormal.GetArray() ;
	prmbuf.pvUVMap = m_bufUVMap.GetArray() ;
	prmbuf.pColor = m_bufColor.GetArray() ;
	prmbuf.pIndexedList = NULL ;
	//
	MakeTubeGridMesh
		( prmbuf, tpParam, vHandle,
			nPoints, pvPoints, pThickness, pAlphas, fBackface ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	//
	return	GridMesh
		( render, pMaterial, gmp.nFlags,
			gmp.widthMesh, gmp.heightMesh,
			m_bufVertex.GetConstArray(), m_bufNormal.GetConstArray(),
			m_bufUVMap.GetConstArray(), m_bufColor.GetConstArray() ) ;
}

SGLError S3DMeshShaper::AddTube
	( S3DVertexBufferInterface & vbuf,
		const S3DMeshShaper::TubeParam & tpParam,
		const S3DVector& vHandle,
		size_t nPoints, const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlphas, bool fBackface )
{
	GridMeshGenParams	gmgp ;
	GridMeshParam		gmp ;
	if ( CalcTubePolygonCount
		( gmgp, gmp, tpParam, nPoints, fBackface ) == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// 一時バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	SGLError	err = vbuf.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
				gmgp.countPolygon * 3, gmgp.countVertex ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッシュ生成
	//
	MakeTubeGridMesh
		( prmbuf, tpParam, vHandle,
			nPoints, pvPoints, pThickness, pAlphas, fBackface ) ;
	//
	// プリミティブ追加
	//
	return	AddGridMesh( vbuf, gmgp, gmp, prmbuf ) ;
}

size_t S3DMeshShaper::CalcTubePolygonCount
	( S3DMeshShaper::GridMeshGenParams& gmgp,
		S3DMeshShaper::GridMeshParam& gmp,
		const S3DMeshShaper::TubeParam & tpParam,
		size_t nPoints,  bool fBackface )
{
	gmgp.countPolygon = 0 ;
	gmgp.countVertex = 0 ;
	if ( (nPoints <= 1) || (tpParam.hDivision < 2) )
	{
		return	0 ;
	}
	gmp.nFlags = gridHorzLoop ;
	if ( fBackface )
	{
		gmp.nFlags |= gridBackface ;
	}
	gmp.widthMesh = tpParam.hDivision - 1 ;
	gmp.heightMesh = nPoints - 1 ;
	if ( tpParam.nFlags & tubeCapHead )
	{
		gmp.heightMesh += tpParam.vDivision ;
	}
	if ( tpParam.nFlags & tubeCapTail )
	{
		gmp.heightMesh += tpParam.vDivision ;
	}
	return	CalcGridMeshPolygonCount
				( gmgp, gmp.nFlags, gmp.widthMesh, gmp.heightMesh ) ;
}

void S3DMeshShaper::MakeTubeGridMesh
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		const S3DMeshShaper::TubeParam & tpParam,
		const S3DVector& vHandle,
		size_t nPoints, const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlphas, bool fBackface )
{
	const TubeParam	tp = tpParam ;
	//
	S3DVector4 *	pvVertex = prmbuf.pvVertex ;
	S3DVector4 *	pvNormal = prmbuf.pvNormal ;
	S2DVector *		pvUVMap = prmbuf.pvUVMap ;
	S3DColor *		pColor = prmbuf.pColor ;
	S3DColor		clrVertex = tp.colorBase ;
	uint32_t		nBaseAlpha = tp.colorBase.rgbMul.argb.Alpha ;
	//
	S3DVector	vLastDir = pvPoints[1] - pvPoints[0] ;
	S3DVector	vLastHandle = vHandle ;
	vLastDir.Normalize() ;
	vLastHandle.Normalize() ;
	//
	const double	radDeltaRoundV = PI * 2.0 / (double) tp.hDivision ;
	const float32_t	fpDeltaU = tp.uvScale.x / (float32_t) tp.hDivision ;
	float32_t		fpV = tp.uvOffset.y ;
	//
	// 始点半球
	//
	if ( tp.nFlags & tubeCapHead )
	{
		S3DVector	vCross = vLastDir * vLastHandle ;
		S3DMatrix	matR ;
		matR.RotationOnVectorOf
			( vLastDir, sin(radDeltaRoundV), cos(radDeltaRoundV) ) ;
		//
		float32_t	fpRadius = tp.fpRadius ;
		if ( pThickness != NULL )
		{
			fpRadius *= pThickness[0] ;
		}
		if ( pAlphas != NULL )
		{
			clrVertex.rgbMul.argb.Alpha =
				(uint8_t) (nBaseAlpha
							* esl_clampi( (int) pAlphas[0], 0, 256 ) / 0x100) ;
		}
		for ( size_t i = 0; i < tp.vDivision; i ++ )
		{
			S3DMatrix	matCap ;
			double		rad = PI * -0.5 * ((double) i / tp.vDivision - 1.0) ;
			matCap.RotationOnVectorOf( vCross, sin(rad), cos(rad) ) ;
			//
			S3DVector	vTempHandle = matCap * vLastHandle ;
			float32_t	fpU = tp.uvOffset.x ;
			for ( size_t j = 0; j < tp.hDivision; j ++ )
			{
				S3DVector4	vPos = pvPoints[0] ;
				vPos += vTempHandle * fpRadius ;
				//
				*pvVertex = vPos ;
				//
				*pvNormal = vTempHandle ;
				//
				pvUVMap->x = fpU ;
				pvUVMap->y = fpV ;
				//
				*pColor = clrVertex ;
				//
				pvVertex ++ ;
				pvNormal ++ ;
				pvUVMap ++ ;
				pColor ++ ;
				//
				matR.RevolveVector( vTempHandle ) ;
				fpU += fpDeltaU ;
			}
			fpV += tp.vTerminal / (float32_t) tp.vDivision ;
		}
	}
	//
	// チューブ本体
	//
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		S3DVector4	vCenter = pvPoints[i] ;
		S3DVector	vNextDir = vLastDir ;
		S3DVector	vNextHandle = vLastHandle ;
		if ( i + 1 < nPoints )
		{
			vNextDir = pvPoints[i + 1] - vCenter ;
			vNextDir.Normalize() ;
			//
			S3DMatrix	matR ;
			matR.VectorRotationOf( vLastDir, vNextDir ) ;
			matR.RevolveVector( vNextHandle ) ;
			vNextHandle.Normalize() ;
		}
		S3DVector	vCurDir = (vNextDir + vLastDir) * 0.5 ;
		S3DVector	vCurHandle = (vNextHandle + vLastHandle) * 0.5 ;
		vCurDir.Normalize() ;
		vCurHandle.Normalize() ;
		//
		S3DVector	vCross = vCurDir * vCurHandle ;
		S3DMatrix	matR ;
		matR.RotationOnVectorOf
			( vCurDir, sin(radDeltaRoundV), cos(radDeltaRoundV) ) ;
		//
		float32_t	fpRadius = tp.fpRadius ;
		if ( pThickness != NULL )
		{
			fpRadius *= pThickness[i] ;
		}
		if ( pAlphas != NULL )
		{
			clrVertex.rgbMul.argb.Alpha =
				(uint8_t) (nBaseAlpha
							* esl_clampi( (int) pAlphas[i], 0, 256 ) / 0x100) ;
		}
		//
		float32_t	fpU = tp.uvOffset.x ;
		for ( size_t j = 0; j < tp.hDivision; j ++ )
		{
			S3DVector4	vPos = vCenter ;
			vPos += vCurHandle * fpRadius ;
			//
			*pvVertex = vPos ;
			//
			*pvNormal = vCurHandle ;
			//
			pvUVMap->x = fpU ;
			pvUVMap->y = fpV ;
			//
			*pColor = clrVertex ;
			//
			pvVertex ++ ;
			pvNormal ++ ;
			pvUVMap ++ ;
			pColor ++ ;
			//
			matR.RevolveVector( vCurHandle ) ;
			fpU += fpDeltaU ;
		}
		//
		vLastDir = vNextDir ;
		vLastHandle = vNextHandle ;
		//
		if ( i > 0 )
		{
			fpV += tp.uvScale.y
					* (float32_t) (pvPoints[i] - pvPoints[i - 1]).Absolute() ;
		}
	}
	//
	// 終端半球
	//
	if ( tp.nFlags & tubeCapTail )
	{
		S3DVector	vCross = vLastDir * vLastHandle ;
		S3DMatrix	matR ;
		matR.RotationOnVectorOf
			( vLastDir, sin(radDeltaRoundV), cos(radDeltaRoundV) ) ;
		//
		float32_t	fpRadius = tp.fpRadius ;
		if ( pThickness != NULL )
		{
			fpRadius *= pThickness[nPoints - 1] ;
		}
		if ( pAlphas != NULL )
		{
			clrVertex.rgbMul.argb.Alpha =
				(uint8_t) (nBaseAlpha * esl_clampi
							( (int) pAlphas[nPoints - 1], 0, 256 ) / 0x100) ;
		}
		for ( size_t i = 0; i < tp.vDivision; i ++ )
		{
			S3DMatrix	matCap ;
			double		rad = PI * -0.5 * (i + 1) / tp.vDivision ;
			matCap.RotationOnVectorOf( vCross, sin(rad), cos(rad) ) ;
			//
			S3DVector	vTempHandle = matCap * vLastHandle ;
			float32_t	fpU = tp.uvOffset.x ;
			for ( size_t j = 0; j < tp.hDivision; j ++ )
			{
				S3DVector4	vPos = pvPoints[nPoints - 1] ;
				vPos += vTempHandle * fpRadius ;
				//
				*pvVertex = vPos ;
				//
				*pvNormal = vTempHandle ;
				//
				pvUVMap->x = fpU ;
				pvUVMap->y = fpV ;
				//
				*pColor = clrVertex ;
				//
				pvVertex ++ ;
				pvNormal ++ ;
				pvUVMap ++ ;
				pColor ++ ;
				//
				matR.RevolveVector( vTempHandle ) ;
				fpU += fpDeltaU ;
			}
			fpV += tp.vTerminal / (float32_t) tp.vDivision ;
		}
	}
	if ( fBackface )
	{
		size_t	countVertex = tp.hDivision * nPoints ;
		if ( tp.nFlags & tubeCapHead )
		{
			countVertex += tp.hDivision * tp.vDivision ;
		}
		if ( tp.nFlags & tubeCapTail )
		{
			countVertex += tp.hDivision * tp.vDivision ;
		}
		pvNormal = prmbuf.pvNormal ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pvNormal->x = - pvNormal->x ;
			pvNormal->y = - pvNormal->y ;
			pvNormal->z = - pvNormal->z ;
			pvNormal ++ ;
		}
	}
}

void S3DMeshShaper::CalcDefaultTubeHandle
	( S3DVector& vHandle, size_t nPoints, const S3DVector4 * pvPoints )
{
	if ( nPoints < 2 )
	{
		vHandle.x = 1 ;
		vHandle.y = 0 ;
		vHandle.z = 0 ;
	}
	else
	{
		S3DMatrix	matR( 1, 1, 1 ) ;
		S3DVector	vDir = pvPoints[1] - pvPoints[0] ;
		matR.RevolveForAngle( vDir ) ;
		vHandle = matR * S3DVector( 1, 0, 0 ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 稲妻生成器
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshShaper::ThunderContext::ThunderContext( void )
	: m_index( 0 ), m_vLastPos( 0, 0, 0 ),
		m_fpNextAmpX( 0.0 ), m_fpNextAmpY( 0.0 ),
		m_fpLastAmpX( 0.0 ), m_fpLastAmpY( 0.0 ), m_fpPhase( 0.0 )
{
}

// 稲妻生成
//////////////////////////////////////////////////////////////////////////////
void S3DMeshShaper::ThunderContext::Create
	( SakuraCL::SCLRandomizer& randomizer,
		const S3DMeshShaper::ThunderParam& param,
		size_t nPointCount, const S3DDVector * pvPoints )
{
	if ( nPointCount == 0 )
	{
		m_aPoint.RemoveAll() ;
		m_aIndex.RemoveAll() ;
		m_index = 0 ;
		return ;
	}
	InitContext( randomizer, pvPoints[0], param ) ;
	for ( size_t i = 1; i < nPointCount; i ++ )
	{
		NextPoint( randomizer, pvPoints[i], param ) ;
	}
}

// 初期設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshShaper::ThunderContext::InitContext
	( SakuraCL::SCLRandomizer& randomizer,
		const S3DDVector& vTrack0, const S3DMeshShaper::ThunderParam& param )
{
	m_aPoint.RemoveAll() ;
	m_aIndex.RemoveAll() ;
	//
	m_fpNextAmpX = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
	m_fpNextAmpY = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
	m_fpLastAmpX = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
	m_fpLastAmpY = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
	m_fpPhase = 0.0 ;
	//
	m_vLastPos = vTrack0 ;
	m_aPoint.Add( m_vLastPos ) ;
	m_aIndex.Add( 0.0 ) ;
	m_index = 1 ;
}

// 次の点（元線分）追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshShaper::ThunderContext::NextPoint
	( SakuraCL::SCLRandomizer& randomizer,
		const S3DDVector& vTrackX, const S3DMeshShaper::ThunderParam& param )
{
	S3DVector	vCurPos = vTrackX ;
	S3DVector	vDelta = vCurPos - m_vLastPos ;
	double		fpLen = vDelta.Absolute() ;
	if ( fpLen < 1.0e-7 )
	{
		return ;
	}
	S3DMatrix	matDir( 1, 1, 1 ) ;
	matDir.RevolveForAngle( vDelta ) ;
	//
	S3DVector	vBaseX = matDir * S3DVector( 1, 0, 0 ) ;
	S3DVector	vBaseY = matDir * S3DVector( 0, 1, 0 ) ;
	//
	size_t	nDivCount = (size_t) esl_lroundfi( fpLen / param.fpJointLen ) ;
	if ( nDivCount == 0 )
	{
		nDivCount = 1 ;
	}
	else if ( nDivCount > 16 )
	{
		nDivCount = 16 ;
	}
	double	tLast = 0.0 ;
	for ( size_t j = 1; j <= nDivCount; j ++ )
	{
		double	t = (double) j / nDivCount ;
		m_fpPhase += (t - tLast) * fpLen ;
		if ( m_fpPhase > param.fpWaveLen )
		{
			m_fpLastAmpX = m_fpNextAmpX ;
			m_fpLastAmpY = m_fpNextAmpY ;
			m_fpNextAmpX = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
			m_fpNextAmpY = randomizer.QuickRandomDouble( param.fpWaveEffect ) ;
			m_fpPhase -= param.fpWaveLen * floor( m_fpPhase / param.fpWaveLen ) ;
		}
		//
		double	ax = randomizer.QuickRandomDouble( param.fpJointEffect ) ;
		double	ay = randomizer.QuickRandomDouble( param.fpJointEffect ) ;
		//
		double	tw = m_fpPhase / param.fpWaveLen ;
		ax += (m_fpNextAmpX - m_fpLastAmpX) * tw + m_fpLastAmpX ;
		ay += (m_fpNextAmpY - m_fpLastAmpY) * tw + m_fpLastAmpY ;
		//
		S3DVector4	vPos = m_vLastPos ;
		vPos += vDelta * t ;
		vPos += vBaseX * ax ;
		vPos += vBaseY * ay ;
		//
		m_aPoint.Add( vPos ) ;
		m_aIndex.Add( m_index + t ) ;
		//
		tLast = t ;
	}
	m_vLastPos = vCurPos ;
	m_index ++ ;
}

// 次の指標（元線分）
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshShaper::ThunderContext::GetNextIndex( void ) const
{
	return	m_index ;
}

// 指標削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshShaper::ThunderContext::DecreaseIndex( size_t nDec )
{
	if ( nDec > m_index )
	{
		nDec = m_index ;
	}
	const size_t	nCount = m_aIndex.GetLength() ;
	const double *	pIndex = m_aIndex.GetConstArray() ;
	size_t			nDecCount = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pIndex[i] >= nDec )
		{
			break ;
		}
		nDecCount = i + 1 ;
	}
	ESLAssert( m_aPoint.GetLength() == nCount ) ;
	m_aPoint.Remove( 0, nDecCount ) ;
	m_aIndex.Remove( 0, nDecCount ) ;
	//
	const size_t	nEditCount = m_aIndex.GetLength() ;
	double *		pEditIndex = m_aIndex.GetArray() ;
	for ( size_t i = 0; i < nEditCount; i ++ )
	{
		pEditIndex[i] -= nDec ;
	}
	m_aIndex.FinishArray() ;
	//
	m_index -= nDec ;
}

// 生成された稲妻
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshShaper::ThunderContext::GetPointCount( void ) const
{
	ESLAssert( m_aPoint.GetLength() == m_aIndex.GetLength() ) ;
	return	m_aPoint.GetLength() ;
}

const S3DVector4 * S3DMeshShaper::ThunderContext::GetPointArray( void ) const
{
	return	m_aPoint.GetConstArray() ;
}

const double * S3DMeshShaper::ThunderContext::GetIndexArray( void ) const
{
	return	m_aIndex.GetConstArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// 三角多面体
//////////////////////////////////////////////////////////////////////////////

// 稜線を検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DPolyhedronMesh::VertexEdge::Find( size_t iVertex ) const
{
	for ( size_t i = 0; i < nEdge; i ++ )
	{
		if ( iEdge[i] == iVertex )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 稜線を追加
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::VertexEdge::AddEdge( size_t iVertex )
{
	for ( size_t i = 0; i < nEdge; i ++ )
	{
		if ( iEdge[i] == iVertex )
		{
			return ;
		}
	}
	ESLAssert( nEdge + 1 <= vertexMaxEdge ) ;
	iEdge[nEdge ++] = iVertex ;
}

// 正20面体生成
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TrianglePolyhedron::CreateIcosahedron( void )
{
	m_aVertex.RemoveAll() ;
	m_aFace.RemoveAll() ;
	m_aEdge.RemoveAll() ;
	//
	// 頂点座標計算
	//
	float32_t	y1 = (float32_t) cos( PI * 2.0 / 3.0 ) ;
	float32_t	y2 = (float32_t) cos( PI / 3.0 ) ;
	float32_t	x1 = (float32_t) sin( PI / 3.0 ) ;
	//
	S3DVector	vTemp[4] =
	{
		S3DVector( 0, -1, 0 ),
		S3DVector( 0, y1, 0 ),
		S3DVector( 0, y2, 0 ),
		S3DVector( 0, 1, 0 ),
	} ;
	S3DVector *	pvVertex = m_aVertex.GetArray( 12 ) ;
	pvVertex[0] = vTemp[0] ;
	pvVertex[11] = vTemp[3] ;
	//
	for ( int i = 0; i < 5; i ++ )
	{
		double	r0 = PI * 2.0 * i / 5.0 ;
		double	r1 = PI * 2.0 * (i + 0.5) / 5.0 ;
		//
		vTemp[1].x = x1 * (float32_t) cos( r0 ) ;
		vTemp[1].z = x1 * (float32_t) sin( r0 ) ;
		vTemp[2].x = x1 * (float32_t) cos( r1 ) ;
		vTemp[2].z = x1 * (float32_t) sin( r1 ) ;
		//
		pvVertex[1 + i] = vTemp[1] ;
		pvVertex[6 + i] = vTemp[2] ;
	}
	m_aVertex.FinishArray() ;
	//
	// 三角面
	//
	Triangle *	pFace = m_aFace.GetArray( 20 ) ;
	for ( size_t i = 0; i < 5; i ++ )
	{
		size_t	j = (i + 1) % 5 ;
		//
		Triangle&	face0 = pFace[i * 4] ;
		face0.iVertex[0] = 0 ;
		face0.iVertex[1] = 1 + i ;
		face0.iVertex[2] = 1 + j ;
		//
		Triangle&	face1 = pFace[i * 4 + 1] ;
		face1.iVertex[0] = 1 + i ;
		face1.iVertex[1] = 6 + i ;
		face1.iVertex[2] = 1 + j ;
		//
		Triangle&	face2 = pFace[i * 4 + 2] ;
		face2.iVertex[0] = 1 + j ;
		face2.iVertex[1] = 6 + i ;
		face2.iVertex[2] = 6 + j ;
		//
		Triangle&	face3 = pFace[i * 4 + 3] ;
		face3.iVertex[0] = 6 + i ;
		face3.iVertex[1] = 11 ;
		face3.iVertex[2] = 6 + j ;
	}
	m_aFace.FinishArray() ;
	//
	// 稜線情報構築
	//
	m_aEdge.SetLength( m_aVertex.GetLength() ) ;
	for ( size_t i = 0; i < m_aFace.GetLength(); i ++ )
	{
		const Triangle&	face = m_aFace.At(i) ;
		AddEdge( face.iVertex[0], face.iVertex[1] ) ;
		AddEdge( face.iVertex[1], face.iVertex[2] ) ;
		AddEdge( face.iVertex[2], face.iVertex[0] ) ;
	}
}

// 分割（三角を4分割）多面体生成
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TrianglePolyhedron::DividedTriangle( void )
{
	//
	// 頂点分割
	//
	m_aEdgeTemp = m_aEdge ;
	m_aVertex.SetLimit( m_aVertex.GetLength() * 2 ) ;
	m_aDivEdge.SetLength( m_aEdge.GetLength() ) ;
	{
		const VertexEdge *	pEdge = m_aEdge.GetConstArray() ;
		const size_t		nVertexCount = m_aEdge.GetLength() ;
		VertexEdge *		pDivEdge = m_aDivEdge.GetArray() ;
		//
		for ( size_t iVertex = 0; iVertex < nVertexCount; iVertex ++ )
		{
			const VertexEdge&	edge = pEdge[iVertex] ;
			VertexEdge&			edgeDiv = pDivEdge[iVertex] ;
			//
			edgeDiv.nEdge = edge.nEdge ;
			for ( size_t iEdge = 0; iEdge < edge.nEdge; iEdge ++ )
			{
				if ( edgeDiv.iEdge[iEdge] >= nVertexCount )
				{
					continue ;
				}
				// 頂点挿入
				const size_t	iVertex1 = edge.iEdge[iEdge] ;
				const size_t	iVertex2 = m_aVertex.GetLength() ;
				S3DVector		vVertex = (m_aVertex.At(iVertex)
										+ m_aVertex.At(iVertex1)).Normalize() ;
				m_aVertex.Add( vVertex ) ;
				edgeDiv.iEdge[iEdge] = iVertex2 ;
				//
				// 反対側からの情報設定
				const VertexEdge&	edge2 = pEdge[iVertex1] ;
				VertexEdge&			edgeDiv2 = pDivEdge[iVertex1] ;
				const ssize_t		iEdge2 = edge2.Find( iVertex ) ;
				ESLAssert( iEdge2 >= 0 ) ;
				if ( iEdge2 >= 0 )
				{
					ESLAssert( edgeDiv2.iEdge[iEdge2] == 0 ) ;
					edgeDiv2.iEdge[iEdge2] = iVertex2 ;
				}
			}
		}
		m_aDivEdge.FinishArray() ;
		m_aEdge = m_aDivEdge ;
		m_aDivEdge.RemoveAll() ;
	}
	//
	// 面分割
	//
	m_aEdge.SetLength( m_aVertex.GetLength() ) ;
	//
	size_t	nOrgFaceCount = m_aFace.GetLength() ;
	for ( size_t iFace = 0; iFace < nOrgFaceCount; iFace ++ )
	{
		Triangle&		face = m_aFace.At(iFace) ;
		VertexEdge *	pEdge[3] =
		{
			m_aEdgeTemp.GetAt( face.iVertex[0] ),
			m_aEdgeTemp.GetAt( face.iVertex[1] ),
			m_aEdgeTemp.GetAt( face.iVertex[2] ),
		} ;
		VertexEdge *	pEdgeDiv[3] =
		{
			m_aEdge.GetAt( face.iVertex[0] ),
			m_aEdge.GetAt( face.iVertex[1] ),
			m_aEdge.GetAt( face.iVertex[2] ),
		} ;
		const ssize_t		iEdge[3][2] =
		{
			{
				pEdge[0]->Find(face.iVertex[1]), 
				pEdge[0]->Find(face.iVertex[2]),
			},
			{
				pEdge[1]->Find(face.iVertex[2]), 
				pEdge[1]->Find(face.iVertex[0]),
			},
			{
				pEdge[2]->Find(face.iVertex[0]), 
				pEdge[2]->Find(face.iVertex[1]),
			},
		} ;
		Triangle	triDiv[4] ;
		for ( size_t i = 0; i < 3; i ++ )
		{
			ESLAssert( iEdge[i][0] >= 0 ) ;
			ESLAssert( iEdge[i][1] >= 0 ) ;
			triDiv[i].iVertex[0] = face.iVertex[i] ;
			triDiv[i].iVertex[1] = pEdgeDiv[i]->iEdge[iEdge[i][0]] ;
			triDiv[i].iVertex[2] = pEdgeDiv[i]->iEdge[iEdge[i][1]] ;
			triDiv[3].iVertex[i] = pEdgeDiv[i]->iEdge[iEdge[i][0]] ;
		}
		//
		// 稜線情報追加
		//
		for ( size_t i = 0; i < 4; i ++ )
		{
			AddEdge( triDiv[i].iVertex[0], triDiv[i].iVertex[1] ) ;
			AddEdge( triDiv[i].iVertex[1], triDiv[i].iVertex[2] ) ;
			AddEdge( triDiv[i].iVertex[2], triDiv[i].iVertex[0] ) ;
		}
		//
		// 面更新・追加
		//
		face = triDiv[3] ;
		m_aFace.AddArray( triDiv, 3 ) ;
	}
}

// 稜線情報追加
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TrianglePolyhedron::AddEdge( size_t iVertex0, size_t iVertex1 )
{
	ESLAssert( iVertex0 != iVertex1 ) ;
	m_aEdge.At(iVertex0).AddEdge( iVertex1 ) ;
	m_aEdge.At(iVertex1).AddEdge( iVertex0 ) ;
}

// メッシュ出力
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TrianglePolyhedron::RenderMesh
	( S3DVertexBufferInterface& vb, S3DPolyhedronMesh::Modifier * pModifier )
{
	const S3DVector *	pVertex = m_aVertex.GetConstArray() ;
	const size_t		nVertexCount = m_aVertex.GetLength() ;
	const Triangle *	pFace = m_aFace.GetConstArray() ;
	const size_t		nFaceCount = m_aFace.GetLength() ;
	//
	VertexBuffer::PrimitiveBuffer	prmbuf ;
	vb.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nFaceCount * 3, nFaceCount * 3 ) ;
	//
	for ( size_t i = 0, j = 0; i < nFaceCount; i ++, j += 3 )
	{
		size_t		v0 = pFace[i].iVertex[0] ;
		size_t		v1 = pFace[i].iVertex[1] ;
		size_t		v2 = pFace[i].iVertex[2] ;
		S3DVector	vVertex[3] =
		{
			pVertex[v0],
			pVertex[v1],
			pVertex[v2],
		} ;
		S3DVector	vNormal = (vVertex[1] - vVertex[0]) * (vVertex[2] - vVertex[0]) ;
		vNormal.Normalize() ;
		//
		for ( size_t k = 0; k < 3; k ++ )
		{
			prmbuf.pvVertex[j + k] = vVertex[k] ;
			prmbuf.pvNormal[j + k] = vNormal ;
			prmbuf.pvUVMap[j + k].x = vVertex[k].x ;
			prmbuf.pvUVMap[j + k].y = vVertex[k].z ;
			prmbuf.pColor[j + k].rgbMul.ui32 = 0xFFFFFFFF ;
			prmbuf.pColor[j + k].rgbAdd.ui32 = 0 ;
			prmbuf.pIndexedList[j + k] = (uint32_t) (j + k) ;
		}
	}
	if ( pModifier != nullptr )
	{
		pModifier->ModifyTriangles
			( prmbuf.pvVertex, prmbuf.pvNormal,
				prmbuf.pvUVMap, prmbuf.pColor, nFaceCount * 3 ) ;
	}
	//
	vb.AddPrimitiveBuffer
		( nullptr, 0, primitiveTriangle,
			prmbuf, nFaceCount * 3, nFaceCount * 3 ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 切頂多面体
//////////////////////////////////////////////////////////////////////////////

// 切頂多面体生成
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TruncatedPolyhedron::CreateTruncatedFace( const TrianglePolyhedron& tp )
{
	m_aFace.RemoveAll() ;
	m_aFace.SetLimit( tp.m_aVertex.GetLength() + tp.m_aFace.GetLength() ) ;

	TruncatedFace	tfFace ;

	const S3DVector *	pVertex = tp.m_aVertex.GetConstArray() ;
	const Triangle *	pFace = tp.m_aFace.GetConstArray() ;
	const VertexEdge *	pEdge = tp.m_aEdge.GetConstArray() ;
	const size_t		nVertexCount = tp.m_aVertex.GetLength() ;
	const size_t		nFaceCount = tp.m_aFace.GetLength() ;
	ESLAssert( tp.m_aEdge.GetLength() >= nVertexCount ) ;

	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		const VertexEdge&	edge = pEdge[i] ;
		ESLAssert( edge.nEdge >= 3 ) ;

		S3DVector	vCenter( 0, 0, 0 ) ;
		S3DVector	vVertex = pVertex[i] ;
		for ( size_t j = 0; j < edge.nEdge; j ++ )
		{
			tfFace.vVertex[j] =
				vVertex + (pVertex[edge.iEdge[j]] - vVertex) * (1.0 / 3.0) ;
			vCenter += tfFace.vVertex[j] ;
		}
		tfFace.nVertex = edge.nEdge ;
		tfFace.vCenter = vCenter / (float32_t) edge.nEdge ;

		for ( size_t j = 1; j + 1 < tfFace.nVertex; j ++ )
		{
			double	fpMin = 1.0e9 ;
			size_t	jMin = j + 1 ;
			for ( size_t k = j + 1; k < tfFace.nVertex; k ++ )
			{
				S3DVector	v = tfFace.vVertex[k] - tfFace.vVertex[j] ;
				double	d = v.InnerProduct( v ) ;
				if ( d < fpMin )
				{
					fpMin = d ;
					jMin = k ;
				}
			}
			if ( j + 1 != jMin )
			{
				S3DVector	vTemp = tfFace.vVertex[j + 1] ;
				tfFace.vVertex[j + 1] = tfFace.vVertex[jMin] ;
				tfFace.vVertex[jMin] = vTemp ;
			}
		}

		S3DVector	vx = (tfFace.vVertex[1] - tfFace.vVertex[0])
							*( tfFace.vVertex[2]- tfFace.vVertex[0]) ;
		if ( vx.InnerProduct( vCenter ) < 0 )
		{
			for ( size_t j = 0; j < tfFace.nVertex - j - 1; j ++ )
			{
				const size_t	k = tfFace.nVertex - j - 1 ;
				const S3DVector	vTemp = tfFace.vVertex[j] ;
				tfFace.vVertex[j] = tfFace.vVertex[k] ;
				tfFace.vVertex[k] = vTemp ;
			}
		}

		m_aFace.Add( tfFace ) ;
	}

	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		const Triangle&	face = pFace[i] ;
		const S3DVector	v[3] =
		{
			pVertex[face.iVertex[0]],
			pVertex[face.iVertex[1]],
			pVertex[face.iVertex[2]],
		} ;
		const S3DVector	vd[3] =
		{
			v[1] - v[0], v[2] - v[1], v[0] - v[2]
		} ;
		tfFace.nVertex = 6 ;
		tfFace.vCenter = (v[0] + v[1] + v[2]) / 3.0 ;
		tfFace.vVertex[0] = v[0] + vd[0] * (1.0 / 3.0) ;
		tfFace.vVertex[1] = v[0] + vd[0] * (2.0 / 3.0) ;
		tfFace.vVertex[2] = v[1] + vd[1] * (1.0 / 3.0) ;
		tfFace.vVertex[3] = v[1] + vd[1] * (2.0 / 3.0) ;
		tfFace.vVertex[4] = v[2] + vd[2] * (1.0 / 3.0) ;
		tfFace.vVertex[5] = v[2] + vd[2] * (2.0 / 3.0) ;

		m_aFace.Add( tfFace ) ;
	}
}

// メッシュ出力
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::TruncatedPolyhedron::RenderMesh
	( S3DVertexBufferInterface& vb, S3DPolyhedronMesh::Modifier * pModifier )
{
	const TruncatedFace *	pFace = m_aFace.GetConstArray() ;
	const size_t			nFaceCount = m_aFace.GetLength() ;
	//
	VertexBuffer::PrimitiveBuffer	prmbuf ;
	vb.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nFaceCount * 18, nFaceCount * 7 ) ;
	//
	size_t	nIndexCount = 0 ;
	size_t	nVertexCount = 0 ;
	for ( size_t iFace = 0; iFace < nFaceCount; iFace ++ )
	{
		const TruncatedFace&	face = pFace[iFace] ;

		S3DVector	vNormal = face.vCenter ;
		vNormal.Normalize() ;
		//
		for ( size_t i = 0; i < face.nVertex; i ++ )
		{
			size_t	j = nVertexCount + i ;
			prmbuf.pvVertex[j] = face.vVertex[i] ;
			prmbuf.pvNormal[j] = vNormal ;
			prmbuf.pvUVMap[j].x = face.vVertex[i].x ;
			prmbuf.pvUVMap[j].y = face.vVertex[i].z ;
			prmbuf.pColor[j].rgbMul.ui32 = 0xFFFFFFFF ;
			prmbuf.pColor[j].rgbAdd.ui32 = 0 ;
		}
		size_t	j = nVertexCount + face.nVertex ;
		prmbuf.pvVertex[j] = face.vCenter ;
		prmbuf.pvNormal[j] = vNormal ;
		prmbuf.pvUVMap[j].x = face.vCenter.x ;
		prmbuf.pvUVMap[j].y = face.vCenter.z ;
		prmbuf.pColor[j].rgbMul.ui32 = 0xFFFFFFFF ;
		prmbuf.pColor[j].rgbAdd.ui32 = 0 ;
		//
		size_t	iLast = nVertexCount + face.nVertex - 1 ;
		size_t	iCenter = nVertexCount + face.nVertex ;
		for ( size_t i = 0; i < face.nVertex; i ++ )
		{
			prmbuf.pIndexedList[nIndexCount]     = (uint32_t) iLast ;
			prmbuf.pIndexedList[nIndexCount + 1] = (uint32_t) (nVertexCount + i) ;
			prmbuf.pIndexedList[nIndexCount + 2] = (uint32_t) iCenter ;
			iLast = nVertexCount + i ;
			nIndexCount += 3 ;
		}
		if ( pModifier != nullptr )
		{
			pModifier->ModifyFace
				( prmbuf.pvVertex + nVertexCount,
					prmbuf.pvNormal + nVertexCount,
					prmbuf.pvUVMap + nVertexCount,
					prmbuf.pColor + nVertexCount,
					face.nVertex + 1, face ) ;
		}
		nVertexCount += face.nVertex + 1 ;
	}
	ESLAssert( nIndexCount < nFaceCount * 18 ) ;
	ESLAssert( nVertexCount < nFaceCount * 7 ) ;

	vb.AddPrimitiveBuffer
		( nullptr, 0, primitiveTriangle,
			prmbuf, nIndexCount, nVertexCount ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 多面体（正20面体 / 20*4^n 面体 / 切頂多面体）生成オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPolyhedronMesh, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DPolyhedronMesh::S3DPolyhedronMesh( void )
{
}

// 形状生成
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::CreatePolyhedron( size_t nDivCount, bool flagTruncate )
{
	m_triangles.CreateIcosahedron() ;
	for ( size_t i = 0; i < nDivCount; i ++ )
	{
		m_triangles.DividedTriangle() ;
	}
	if ( flagTruncate )
	{
		m_truncated.CreateTruncatedFace( m_triangles ) ;
	}
	m_flagTruncated = flagTruncate ;
}

// メッシュ出力
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMesh::RenderMesh( S3DVertexBufferInterface& vb, Modifier * pModifier )
{
	if ( m_flagTruncated )
	{
		m_truncated.RenderMesh( vb, pModifier ) ;
	}
	else
	{
		m_triangles.RenderMesh( vb, pModifier ) ;
	}
}


