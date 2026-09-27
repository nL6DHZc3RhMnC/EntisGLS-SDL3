
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl3d/sgl_render_parameter_context.h>
#include <sakuragl/sgl3d/sgl_render_buffer.h>

#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
#include <xmmintrin.h>
#endif

#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
#include <sakuragl/sgl_erisa_lib.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;



//////////////////////////////////////////////////////////////////////////////
// トライアングル・ストリップ→インデックスリスト用一時バッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryIndexTriangleStrip::S3DTemporaryIndexTriangleStrip( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryIndexTriangleStrip::~S3DTemporaryIndexTriangleStrip( void )
{
}

// トライアングルストリップのインデックスリストを取得
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DTemporaryIndexTriangleStrip::MakeIndexList( size_t countTriangleStrip )
{
	uint32_t *	pIndexedList = GetArray( countTriangleStrip * 3 ) ;
	for ( uint32_t i = 0, j = 0; i < countTriangleStrip; i ++, j += 3 )
	{
		pIndexedList[j] = i ;
		pIndexedList[j + 1] = i + 1 + (i & 0x01) ;
		pIndexedList[j + 2] = i + 2 - (i & 0x01) ;
	}
	FinishArray() ;
	return	GetConstArray() ;
}


//////////////////////////////////////////////////////////////////////////////
// 法線計算用一時バッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryNormalBuffer::S3DTemporaryNormalBuffer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryNormalBuffer::~S3DTemporaryNormalBuffer( void )
{
}

// 法線取得
//////////////////////////////////////////////////////////////////////////////
const S3DVector4 * S3DTemporaryNormalBuffer::GetNormalBuffer( void ) const
{
	return	m_bufNormals.GetConstArray() ;
}

// プリミティブリストの法線を計算
//////////////////////////////////////////////////////////////////////////////
bool S3DTemporaryNormalBuffer::SetForIndexedPrimitiveList
	( S3DPrimitiveType typePrimitive,
		size_t countPrimitive, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S2DVector * pvUVMap, const uint32_t * pIndexedList )
{
	if ( pIndexedList != NULL )
	{
		if ( typePrimitive == primitiveTriangle )
		{
			SetForIndexedTriangleList
				( countPrimitive, countVertex,
					pvVertex, pvUVMap, pIndexedList ) ;
			return	true ;
		}
	}
	else
	{
		if ( typePrimitive == primitiveTriangleStrip )
		{
			SetForTriangleStrip
				( countPrimitive, pvVertex, pvUVMap ) ;
			return	true ;
		}
		else if ( typePrimitive == primitiveTriangle )
		{
			SetForTriangleList
				( countPrimitive, countVertex, pvVertex, pvUVMap ) ;
			return	true ;
		}
	}
	return	false ;
}

// 三角ポリゴンリストの法線を計算
//////////////////////////////////////////////////////////////////////////////
void S3DTemporaryNormalBuffer::SetForIndexedTriangleList
	( size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S2DVector * pvUVMap,
		const uint32_t * pIndexedList )
{
	S3DVector4 *	pNormalBuf = m_bufNormals.GetArray( countVertex ) ;
	eslFillMemory( pNormalBuf, 0, countVertex * sizeof(S3DVector4) ) ;
	//
	size_t	i, j ;
	for ( i = 0, j = 0; i < countPolygon; i ++, j += 3 )
	{
		uint32_t	vi0 = pIndexedList[j] ;
		uint32_t	vi1 = pIndexedList[j + 1] ;
		uint32_t	vi2 = pIndexedList[j + 2] ;
		S3DVector	v0 = pvVertex[vi0] ;
		S3DVector	v1 = pvVertex[vi1] ;
		S3DVector	v2 = pvVertex[vi2] ;
		//
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		vNormal.Normalize() ;
		//
		pNormalBuf[vi0] += vNormal ;
		pNormalBuf[vi1] += vNormal ;
		pNormalBuf[vi2] += vNormal ;
	}
	for ( i = 0; i < countVertex; i ++ )
	{
		pNormalBuf[i].Normalize() ;
	}
	m_bufNormals.FinishArray() ;
}

void S3DTemporaryNormalBuffer::SetForTriangleList
	( size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S2DVector * pvUVMap )
{
	S3DVector4 *	pNormalBuf = m_bufNormals.GetArray( countVertex ) ;
	//
	size_t	i, j ;
	for ( i = 0, j = 0; i < countPolygon; i ++, j += 3 )
	{
		S3DVector	v0 = pvVertex[j] ;
		S3DVector	v1 = pvVertex[j + 1] ;
		S3DVector	v2 = pvVertex[j + 2] ;
		//
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		vNormal.Normalize() ;
		//
		pNormalBuf[j] = vNormal ;
		pNormalBuf[j + 1] = vNormal ;
		pNormalBuf[j + 2] = vNormal ;
	}
	m_bufNormals.FinishArray() ;
}

// トライアングルストリップの法線を計算
//////////////////////////////////////////////////////////////////////////////
void S3DTemporaryNormalBuffer::SetForTriangleStrip
	( size_t countTriangleStrip,
		const S3DVector4 * pvVertex, const S2DVector * pvUVMap )
{
	size_t countVertex = countTriangleStrip + 2 ;
	S3DVector4 *	pNormalBuf = m_bufNormals.GetArray( countVertex ) ;
	eslFillMemory( pNormalBuf, 0, countVertex * sizeof(S3DVector4) ) ;
	//
	uint32_t	i ;
	for ( i = 0; i < countTriangleStrip; i ++ )
	{
		uint32_t	vi0 = i ;
		uint32_t	vi1 = i + 1 + (i & 0x01) ;
		uint32_t	vi2 = i + 2 - (i & 0x01) ;
		S3DVector	v0 = pvVertex[vi0] ;
		S3DVector	v1 = pvVertex[vi1] ;
		S3DVector	v2 = pvVertex[vi2] ;
		//
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		vNormal.Normalize() ;
		//
		pNormalBuf[vi0] += vNormal ;
		pNormalBuf[vi1] += vNormal ;
		pNormalBuf[vi2] += vNormal ;
	}
	for ( i = 0; i < countVertex; i ++ )
	{
		pNormalBuf[i].Normalize() ;
	}
	m_bufNormals.FinishArray() ;
}


//////////////////////////////////////////////////////////////////////////////
// テクスチャ基底ベクトル計算用一時バッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryTextureAxisBuffer::S3DTemporaryTextureAxisBuffer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DTemporaryTextureAxisBuffer::~S3DTemporaryTextureAxisBuffer( void )
{
}

// バッファ取得
//////////////////////////////////////////////////////////////////////////////
const S3DVector4 * S3DTemporaryTextureAxisBuffer::GetBufferAxisX( void ) const
{
	return	m_bufAxisX.GetConstArray() ;
}

const S3DVector4 * S3DTemporaryTextureAxisBuffer::GetBufferAxisY( void ) const
{
	return	m_bufAxisY.GetConstArray() ;
}

// プリミティブリストの基底ベクトルを計算
//////////////////////////////////////////////////////////////////////////////
bool S3DTemporaryTextureAxisBuffer::SetForIndexedPrimitiveList
	( S3DPrimitiveType typePrimitive,
		size_t countPrimitive, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S2DVector * pvUVMap, const uint32_t * pIndexedList )
{
	if ( pIndexedList != NULL )
	{
		if ( typePrimitive == primitiveTriangle )
		{
			SetForIndexedTriangleList
				( countPrimitive, countVertex,
					pvVertex, pvUVMap, pIndexedList ) ;
			return	true ;
		}
	}
	else
	{
		if ( typePrimitive == primitiveTriangleStrip )
		{
			SetForTriangleStrip
				( countPrimitive, pvVertex, pvUVMap ) ;
			return	true ;
		}
	}
	return	false ;
}

// 三角ポリゴンリストの基底ベクトルを計算
//////////////////////////////////////////////////////////////////////////////
void S3DTemporaryTextureAxisBuffer::SetForIndexedTriangleList
	( size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S2DVector * pvUVMap,
		const uint32_t * pIndexedList )
{
	S3DVector4 *	pBufAxisX = m_bufAxisX.GetArray( countVertex ) ;
	S3DVector4 *	pBufAxisY = m_bufAxisY.GetArray( countVertex ) ;
	int32_t *		pBufCount = m_bufCount.GetArray( countVertex ) ;
	eslFillMemory( pBufAxisX, 0, countVertex * sizeof(S3DVector4) ) ;
	eslFillMemory( pBufAxisY, 0, countVertex * sizeof(S3DVector4) ) ;
	eslFillMemory( pBufCount, 0, countVertex * sizeof(int32_t) ) ;
	//
	size_t	i, j ;
	for ( i = 0, j = 0; i < countPolygon; i ++, j += 3 )
	{
		uint32_t	vi0 = pIndexedList[j] ;
		uint32_t	vi1 = pIndexedList[j + 1] ;
		uint32_t	vi2 = pIndexedList[j + 2] ;
		S3DVector	v0 = pvVertex[vi0] ;
		S3DVector	v1 = pvVertex[vi1] ;
		S3DVector	v2 = pvVertex[vi2] ;
		S2DVector	uv0 = pvUVMap[vi0] ;
		S2DVector	uv1 = pvUVMap[vi1] ;
		S2DVector	uv2 = pvUVMap[vi2] ;
		//
		v1 -= v0 ;
		v2 -= v0 ;
		uv1 -= uv0 ;
		uv2 -= uv0 ;
		//
		S3DVector	vTexAxisX, vTexAxisY ;
		TextureBaseAxis( vTexAxisX, vTexAxisY, v1, v2, uv1, uv2 ) ;
		//
		pBufAxisX[vi0] += vTexAxisX ;
		pBufAxisX[vi1] += vTexAxisX ;
		pBufAxisX[vi2] += vTexAxisX ;
		//
		pBufAxisY[vi0] += vTexAxisY ;
		pBufAxisY[vi1] += vTexAxisY ;
		pBufAxisY[vi2] += vTexAxisY ;
		//
		pBufCount[vi0] ++ ;
		pBufCount[vi1] ++ ;
		pBufCount[vi2] ++ ;
	}
	for ( i = 0; i < countVertex; i ++ )
	{
		float32_t	r = (float32_t) (1.0 / pBufCount[i]) ;
		pBufAxisX[i] *= r ;
		pBufAxisY[i] *= r ;
	}
	m_bufAxisX.FinishArray() ;
	m_bufAxisY.FinishArray() ;
	m_bufCount.FinishArray() ;
}

// トライアングルストリップの基底ベクトルを計算
//////////////////////////////////////////////////////////////////////////////
void S3DTemporaryTextureAxisBuffer::SetForTriangleStrip
	( size_t countTriangleStrip,
		const S3DVector4 * pvVertex, const S2DVector * pvUVMap )
{
	size_t countVertex = countTriangleStrip + 2 ;
	S3DVector4 *	pBufAxisX = m_bufAxisX.GetArray( countVertex ) ;
	S3DVector4 *	pBufAxisY = m_bufAxisY.GetArray( countVertex ) ;
	int32_t *		pBufCount = m_bufCount.GetArray( countVertex ) ;
	eslFillMemory( pBufAxisX, 0, countVertex * sizeof(S3DVector4) ) ;
	eslFillMemory( pBufAxisY, 0, countVertex * sizeof(S3DVector4) ) ;
	eslFillMemory( pBufCount, 0, countVertex * sizeof(int32_t) ) ;
	//
	uint32_t	i ;
	for ( i = 0; i < countTriangleStrip; i ++ )
	{
		uint32_t	vi0 = i ;
		uint32_t	vi1 = i + 1 + (i & 0x01) ;
		uint32_t	vi2 = i + 2 - (i & 0x01) ;
		S3DVector	v0 = pvVertex[vi0] ;
		S3DVector	v1 = pvVertex[vi1] ;
		S3DVector	v2 = pvVertex[vi2] ;
		S2DVector	uv0 = pvUVMap[vi0] ;
		S2DVector	uv1 = pvUVMap[vi1] ;
		S2DVector	uv2 = pvUVMap[vi2] ;
		//
		v1 -= v0 ;
		v2 -= v0 ;
		uv1 -= uv0 ;
		uv2 -= uv0 ;
		//
		S3DVector	vTexAxisX, vTexAxisY ;
		TextureBaseAxis( vTexAxisX, vTexAxisY, v1, v2, uv1, uv2 ) ;
		//
		pBufAxisX[vi0] += vTexAxisX ;
		pBufAxisX[vi1] += vTexAxisX ;
		pBufAxisX[vi2] += vTexAxisX ;
		//
		pBufAxisY[vi0] += vTexAxisY ;
		pBufAxisY[vi1] += vTexAxisY ;
		pBufAxisY[vi2] += vTexAxisY ;
		//
		pBufCount[vi0] ++ ;
		pBufCount[vi1] ++ ;
		pBufCount[vi2] ++ ;
	}
	for ( i = 0; i < countVertex; i ++ )
	{
		float32_t	r = (float32_t) (1.0 / pBufCount[i]) ;
		pBufAxisX[i] *= r ;
		pBufAxisY[i] *= r ;
	}
	m_bufAxisX.FinishArray() ;
	m_bufAxisY.FinishArray() ;
	m_bufCount.FinishArray() ;
}

// テクスチャｘｙ基底ベクトル計算
//////////////////////////////////////////////////////////////////////////////
void S3DTemporaryTextureAxisBuffer::TextureBaseAxis
	( S3DVector& vTexAxisX, S3DVector& vTexAxisY,
		const S3DVector& vDelta1, const S3DVector& vDelta2,
		const S2DVector& vTexDelta1, const S2DVector& vTexDelta2 )
{
	double	d = vTexDelta1.x * vTexDelta2.y - vTexDelta2.x * vTexDelta1.y ;
	if ( fabs(d) > 1.0e-8 )
	{
		d = 1.0 / d ;
		vTexAxisX = vDelta1 * (vTexDelta2.y * d)
						- vDelta2 * (vTexDelta1.y * d) ;
		vTexAxisY = - vDelta1 * (vTexDelta2.x * d)
						+ vDelta2 * (vTexDelta1.x * d) ;
	}
	else
	{
		vTexAxisX.x = 0.0 ;
		vTexAxisX.y = 0.0 ;
		vTexAxisX.z = 0.0 ;
		vTexAxisY.x = 0.0 ;
		vTexAxisY.y = 0.0 ;
		vTexAxisY.z = 0.0 ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// バリアント・バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DRenderBuffer::VariantBuffer, S3DVertexVariantBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::VariantBuffer::VariantBuffer( void )
{
}

// メッシュ表示状態を反映する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::VariantBuffer::ReflectMeshVisibleTo( S3DRenderBuffer * pBuffer )
{
	ESLAssert( pBuffer->m_arrRender.GetLength() >= pBuffer->m_iShiftOffset ) ;
	const size_t		nCount = pBuffer->m_arrRender.GetLength()
												- pBuffer->m_iShiftOffset ;
	RENDER_ENTRY*const*	ppEntries = pBuffer->m_arrRender.GetConstArray()
												+ pBuffer->m_iShiftOffset ;
	if ( nCount != m_arrMeshVars.GetLength() )
	{
		return ;
	}
	const MESH_VARIANT *	pmvArray = m_arrMeshVars.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RENDER_ENTRY *	pre = ppEntries[i] ;
		ESLAssert( pre != NULL ) ;
		//
		const MESH_VARIANT *	pmv = pmvArray + i ;
		pre->flagRenderable = pmv->flagRenderable ;
	}
}

// メッシュにボーン行列設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::VariantBuffer::SetBoneMatrix
	( size_t iMesh, size_t nCount,
		const S3DMatrix * pMatrix, const S3DVector * pTrans )
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pmv->countBone != nCount )
	{
		return	sglErrFailed ;
	}
	pmv->flagUpdateBone = true ;
	eslMoveMemory
		( pmv->pBoneMatrix, pMatrix, nCount * sizeof(S3DMatrix) ) ;
	eslMoveMemory
		( pmv->pBoneTrans, pTrans, nCount * sizeof(S3DVector) ) ;
	return	sglErrSuccess ;
}

// メッシュのボーン行列取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderBuffer::VariantBuffer::GetBoneMatrix
	( size_t iMesh, size_t nCount, S3DMatrix * pMatrix, S3DVector * pTrans )
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	0 ;
	}
	if ( nCount == 0 )
	{
		return	pmv->countBone ;
	}
	if ( nCount > pmv->countBone )
	{
		nCount = pmv->countBone ;
	}
	eslMoveMemory
		( pMatrix, pmv->pBoneMatrix, nCount * sizeof(S3DMatrix) ) ;
	eslMoveMemory
		( pTrans, pmv->pBoneTrans, nCount * sizeof(S3DVector) ) ;
	return	nCount ;
}

// モーフィング設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::VariantBuffer::SetMorphingApplication
	( size_t iMesh, const ssize_t * pTargetMesh,
			const float32_t * pApplication, size_t nTargetMeshCount )
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pmv->countMorph + 1 < nTargetMeshCount )
	{
		return	sglErrFailed ;
	}
	bool		flagUpdate = (pmv->nTargetMeshCount != nTargetMeshCount) ;
	ssize_t *	pMorphTargetMesh = pmv->pMorphTargetMesh ;
	float32_t *	pMorphApplication = pmv->pMorphApplication ;
	for ( size_t i = 0; i < nTargetMeshCount; i ++ )
	{
		if ( (pTargetMesh[i] >= 0)
			&& ((size_t) pTargetMesh[i] >= pmv->countMorph) )
		{
			return	sglErrFailed ;
		}
		flagUpdate |= (pMorphTargetMesh[i] != pTargetMesh[i])
					| (pMorphApplication[i] != pApplication[i]) ;
		pMorphTargetMesh[i] = pTargetMesh[i] ;
		pMorphApplication[i] = pApplication[i] ;
	}
	pmv->nTargetMeshCount = nTargetMeshCount ;
	pmv->flagUpdateMorph = flagUpdate ;
	return	sglErrSuccess ;
}

// モーフィング設定取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::VariantBuffer::GetMorphingApplication
	( size_t iMesh, ssize_t& iTargetMesh,
			float32_t& fpApplication, size_t iTargetMeshIndex )
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	sglErrFailed ;
	}
	if ( iTargetMeshIndex >= pmv->nTargetMeshCount )
	{
		return	sglErrFailed ;
	}
	iTargetMesh = pmv->pMorphTargetMesh[iTargetMeshIndex] ;
	fpApplication = pmv->pMorphApplication[iTargetMeshIndex] ;
	return	sglErrSuccess ;
}

// メッシュ表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::VariantBuffer::EnableToRenderMesh
	( size_t iFirst, ssize_t iEnd, bool fEnable )
{
	if ( iEnd < 0 )
	{
		iEnd = (ssize_t) m_arrMeshVars.GetLength() ;
	}
	for ( size_t i = iFirst; i < (size_t) iEnd; i ++ )
	{
		MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( i ) ;
		if ( pmv == NULL )
		{
			return	sglErrFailed ;
		}
		pmv->flagRenderable = fEnable ;
	}
	return	sglErrSuccess ;
}

// メッシュ表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::VariantBuffer::IsEnabledToRenderMesh( size_t iMesh ) const
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	false ;
	}
	return	pmv->flagRenderable ;
}

// メッシュマテリアル設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::VariantBuffer::SetMaterialToRenderMesh
	( size_t iMesh, S3DMaterial * pMaterial )
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	sglErrFailed ;
	}
	pmv->flagUpdateMaterial = true ;
	pmv->pMaterial = pMaterial ;
	return	sglErrSuccess ;
}

// メッシュマテリアル取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DRenderBuffer::VariantBuffer::GetMaterialToRenderMesh( size_t iMesh ) const
{
	MESH_VARIANT *	pmv = m_arrMeshVars.GetAt( iMesh ) ;
	if ( pmv == NULL )
	{
		return	NULL ;
	}
	return	pmv->pMaterial ;
}



//////////////////////////////////////////////////////////////////////////////
// 座標変換  S3DRenderBuffer::Transformation
//////////////////////////////////////////////////////////////////////////////

// 初期化
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::Transformation::InitTransformation( void )
{
	matTransform.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	vTransform.x = 0.0 ;
	vTransform.y = 0.0 ;
	vTransform.z = 0.0 ;
	colorEffect.rgbMul.ui32 = 0x00FFFFFF ;
	colorEffect.rgbAdd.ui32 = 0 ;
	nTransparency = 0 ;
	nPriority = 0x04 ;
	S3DRenderParameterContext::DefaultOptionalContext( optContext ) ;
}


//////////////////////////////////////////////////////////////////////////////
// メッシュバッファ  S3DRenderBuffer::MeshBuffer
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::MeshBuffer::MeshBuffer( const S3DRenderBuffer::MeshBuffer& buf )
	: m_pMaterial( buf.m_pMaterial ),
		m_type( buf.m_type ),
		m_nVertexCount( buf.m_nVertexCount ),
		m_nExAttrCount( buf.m_nExAttrCount ),
		m_nIndexCount( buf.m_nIndexCount ),
		m_bufVertex( buf.m_bufVertex ),
		m_bufNormal( buf.m_bufNormal ),
		m_bufUVMap( buf.m_bufUVMap ),
		m_bufColor( buf.m_bufColor ),
		m_bufExAttr( buf.m_bufExAttr ),
		m_bufIndex( buf.m_bufIndex )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DRenderBuffer::MeshBuffer&
	S3DRenderBuffer::MeshBuffer::operator = ( const S3DRenderBuffer::MeshBuffer& buf )
{
	m_pMaterial = buf.m_pMaterial ;
	m_type = buf.m_type ;
	m_nVertexCount = buf.m_nVertexCount ;
	m_nExAttrCount = buf.m_nExAttrCount ;
	m_nIndexCount = buf.m_nIndexCount ;
	m_bufVertex = buf.m_bufVertex ;
	m_bufNormal = buf.m_bufNormal ;
	m_bufUVMap = buf.m_bufUVMap ;
	m_bufColor = buf.m_bufColor ;
	m_bufExAttr = buf.m_bufExAttr ;
	m_bufIndex = buf.m_bufIndex ;
	return	*this ;
}

// 結合
//////////////////////////////////////////////////////////////////////////////
const S3DRenderBuffer::MeshBuffer&
	S3DRenderBuffer::MeshBuffer::operator += ( const S3DRenderBuffer::MeshBuffer& buf )
{
	AddIndexedPrimitiveList
		( buf.m_pMaterial, buf.m_type,
			buf.m_nIndexCount, buf.m_nVertexCount, buf.m_nExAttrCount,
			buf.m_bufVertex.GetConstArray(),
			buf.m_bufNormal.GetConstArray(),
			buf.m_bufUVMap.GetConstArray(),
			buf.m_bufColor.GetConstArray(),
			buf.m_bufExAttr.GetConstArray(),
			buf.m_bufIndex.GetConstArray() ) ;
	return	*this ;
}

SGLError S3DRenderBuffer::MeshBuffer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial,
		S3DPrimitiveType type,
		size_t countIndex,
		size_t countVertex, size_t countExAttr,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const float32_t * pfpExAttrs,
		const uint32_t * pIndexedList,
		const S3DRenderBuffer::Transformation * pTransform )
{
	if ( countVertex == 0 )
	{
		return	sglErrSuccess ;
	}
	if ( m_nVertexCount == 0 )
	{
		m_pMaterial = pMaterial ;
		m_type = type ;
		m_nExAttrCount = countExAttr ;
	}
	else if ( (m_pMaterial != pMaterial)
			|| (m_type != type)
			|| ((countExAttr != 0) && (m_nExAttrCount != countExAttr))
			|| (pvVertex == NULL) )
	{
		return	sglErrFailed ;
	}
	const size_t	nLastVertexCount = m_nVertexCount ;
	if ( pvVertex != NULL )
	{
		#if	defined(__DEBUG__)
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			ESLAssert( !pvVertex[i].IsNaN() ) ;
		}
		#endif
		m_bufVertex.AddArray( pvVertex, countVertex ) ;
		//
		if ( pTransform != nullptr )
		{
			S3DMatrix		matTransform = pTransform->matTransform ;
			S3DVector		vTransform = pTransform->vTransform ;
			S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
			matTransform.RevolveVectors
				( pvVertex + nLastVertexCount,
					pvVertex + nLastVertexCount, countVertex, vTransform ) ;
			m_bufVertex.FinishArray() ;
		}
	}
	else
	{
		m_bufVertex.SetLength( m_bufVertex.GetLength() + countVertex ) ;
	}
	if ( pvNormal != NULL )
	{
		m_bufNormal.SetLength( nLastVertexCount ) ;
		m_bufNormal.AddArray( pvNormal, countVertex ) ;
		//
		if ( pTransform != nullptr )
		{
			S3DMatrix		matTransform = pTransform->matTransform ;
			S3DVector		vTransform( 0, 0, 0 ) ;
			S3DVector4 *	pvNormal = m_bufNormal.GetArray() ;
			matTransform.RevolveVectors
				( pvNormal + nLastVertexCount,
					pvNormal + nLastVertexCount, countVertex, vTransform ) ;
			m_bufNormal.FinishArray() ;
		}
	}
	else if ( m_bufNormal.GetConstArray() != NULL )
	{
		m_bufNormal.SetLength( nLastVertexCount + countVertex ) ;
	}
	if ( pvUVMap != NULL )
	{
		m_bufUVMap.SetLength( nLastVertexCount ) ;
		m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	}
	else if ( m_bufUVMap.GetConstArray() != NULL )
	{
		m_bufUVMap.SetLength( nLastVertexCount + countVertex ) ;
	}
	if ( (m_bufColor.GetConstArray() != NULL) || (pColor != NULL)
		|| ((pTransform != nullptr) && (pTransform->nTransparency > 0)) )
	{
		ESLAssert( m_bufColor.GetLength() >= nLastVertexCount ) ;
		S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
		size_t		nDummyCount = m_bufColor.GetLength() - nLastVertexCount ;
		if ( nDummyCount > 0 )
		{
			S3DColor *	pDummy = m_bufColor.AppendArray( nDummyCount ) ;
			for ( size_t i = 0; i < nDummyCount; i ++ )
			{
				pDummy[i] = clrDummy ;
			}
			m_bufColor.FinishArray() ;
		}
		if ( pColor != NULL )
		{
			m_bufColor.AddArray( pColor, countVertex ) ;
		}
		else
		{
			S3DColor *	pDummy = m_bufColor.AppendArray( countVertex ) ;
			for ( size_t i = 0; i < countVertex; i ++ )
			{
				pDummy[i] = clrDummy ;
			}
			m_bufColor.FinishArray() ;
		}
		if ( pTransform != nullptr )
		{
			S3DColor *	pColorBuf = m_bufColor.GetArray() + nLastVertexCount ;
			if ( !pTransform->colorEffect.IsTransparent() )
			{
				S3DColor	clrEffect = pTransform->colorEffect ;
				clrEffect.rgbAdd.argb.Alpha = 0 ;
				clrEffect.rgbMul.argb.Alpha = 0xFF ;
				//
				for ( size_t i = 0; i < countVertex; i ++ )
				{
					S3DColor	clr = clrEffect ;
					clr *= pColorBuf[i] ;
					pColorBuf[i] = clr ;
				}
			}
			if ( pTransform->nTransparency > 0 )
			{
				uint32_t	nAlpha = (pTransform->nTransparency < 0x100)
									? 0x100 - pTransform->nTransparency : 0 ;
				for ( size_t i = 0; i < countVertex; i ++ )
				{
					pColorBuf[i].rgbMul.argb.Alpha =
						(uint8_t) (((uint32_t) pColorBuf[i].rgbMul.argb.Alpha * nAlpha) >> 8) ;
				}
			}
			m_bufColor.FinishArray() ;
		}
	}
	if ( countExAttr != 0 )
	{
		ESLAssert( pfpExAttrs != NULL ) ;
		ESLAssert( m_nExAttrCount == countExAttr ) ;
		m_bufExAttr.SetLength( nLastVertexCount * m_nExAttrCount ) ;
		m_bufExAttr.AddArray( pfpExAttrs, countVertex * m_nExAttrCount ) ;
	}
	else if ( m_bufExAttr.GetConstArray() != NULL )
	{
		m_bufExAttr.SetLength( (nLastVertexCount + countVertex) * m_nExAttrCount ) ;
	}
	m_nVertexCount += countVertex ;
	//
	if ( pIndexedList != NULL )
	{
		uint32_t *	pAppendIndex = m_bufIndex.AppendArray( countIndex ) ;
		for ( size_t i = 0; i < countIndex; i ++ )
		{
			pAppendIndex[i] = pIndexedList[i] + (uint32_t) nLastVertexCount ;
		}
		m_bufIndex.FinishArray() ;
		m_nIndexCount += countIndex ;
	}
	else
	{
		uint32_t *	pAppendIndex = m_bufIndex.AppendArray( countVertex ) ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pAppendIndex[i] = (uint32_t) (nLastVertexCount + i) ;
		}
		m_bufIndex.FinishArray() ;
		m_nIndexCount += countVertex ;
	}
	return	sglErrSuccess ;
}

// 空か？
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::MeshBuffer::IsEmpty( void ) const
{
	return	(m_nIndexCount == 0) ;
}

// 描画頂点数（インデックス数）取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderBuffer::MeshBuffer::GetIndexCount( void ) const
{
	return	m_nIndexCount ;
}

// クリア
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MeshBuffer::ClearBuffer( void )
{
	m_pMaterial = NULL ;
	m_nVertexCount = 0 ;
	m_nExAttrCount = 0 ;
	m_nIndexCount = 0 ;
	m_bufVertex.RemoveAll() ;
	m_bufNormal.RemoveAll() ;
	m_bufUVMap.RemoveAll() ;
	m_bufColor.RemoveAll() ;
	m_bufExAttr.RemoveAll() ;
	m_bufIndex.RemoveAll() ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MeshBuffer::FreeBuffer( void )
{
	m_pMaterial = NULL ;
	m_nVertexCount = 0 ;
	m_nExAttrCount = 0 ;
	m_nIndexCount = 0 ;
	m_bufVertex.FreeArray() ;
	m_bufNormal.FreeArray() ;
	m_bufUVMap.FreeArray() ;
	m_bufColor.FreeArray() ;
	m_bufExAttr.FreeArray() ;
	m_bufIndex.FreeArray() ;
}

// 出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::MeshBuffer::RenderToVertexBuffer( S3DVertexBufferInterface& vbo ) const
{
	SGLError	err = Render( vbo ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_nExAttrCount > 0 )
	{
		err = vbo.SetExtendVertexAttribute
			( 0, m_nExAttrCount, m_nVertexCount,
						m_bufExAttr.GetConstArray() ) ;
	}
	return	err ;
}

SGLError S3DRenderBuffer::MeshBuffer::Render( S3DRenderBufferInterface& render ) const
{
	return	render.AddIndexedPrimitiveList
				( m_pMaterial, 0,
					m_type, m_nIndexCount,
					m_nVertexCount,
					m_bufVertex.GetConstArray(),
					m_bufNormal.GetConstArray(),
					m_bufUVMap.GetConstArray(),
					m_bufColor.GetConstArray(),
					m_bufIndex.GetConstArray() ) ;
}

// 頂点変換
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MeshBuffer::Transform
		( const S3DMatrix& matTransform, const S3DVector& vMove )
{
	ESLAssert( m_bufVertex.GetLength() >= m_nVertexCount ) ;
	ESLAssert( m_bufNormal.GetLength() >= m_nVertexCount ) ;
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DVector4 *	pvNormal = m_bufNormal.GetArray() ;
	S3DVector		vZero( 0, 0, 0 ) ;
	//
	matTransform.RevolveVectors( pvVertex, pvVertex, m_nVertexCount, vMove ) ;
	matTransform.RevolveVectors( pvNormal, pvNormal, m_nVertexCount, vZero ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
}

// 外接直方体
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::MeshBuffer::GetCircumscribedBox( S3DVector& vMin, S3DVector& vMax ) const
{
	if ( m_bufVertex.GetLength() == 0 )
	{
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		return	false ;
	}
	S3DVector4	vMin4, vMax4 ;
	MinMaxVector4DArray
		( vMin4, vMax4, m_bufVertex.GetConstArray(), m_bufVertex.GetLength() ) ;
	vMin = vMin4 ;
	vMax = vMax4 ;
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// レンダリング・バッファ
//////////////////////////////////////////////////////////////////////////////

const S3DRenderBuffer::PrimitiveTypeIndex
	S3DRenderBuffer::m_iPrimitiveIndex[primitiveCount] =
{
	S3DRenderBuffer::indexPrimitivePoints,
	S3DRenderBuffer::indexPrimitivePoints,
	S3DRenderBuffer::indexPrimitiveLines,
	S3DRenderBuffer::indexPrimitiveLines,
	S3DRenderBuffer::indexPrimitiveTriangles,
	S3DRenderBuffer::indexPrimitiveTriangles,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DRenderBuffer, S3DRenderBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::S3DRenderBuffer( void )
	: m_matCamera( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vCameraPos( 0, 0, 0 )
{
	m_pDefaultMaterial = NULL ;
	m_nBufCtrlFlags = 0 ;
	//
	m_signalFreeRefMesh.Initialize( true ) ;
	//
	m_iShiftOffset = 0 ;
	m_nRefShiftMesh = 0 ;
	m_iFenceOrder = 0 ;
	m_pLastShaderContext = NULL ;
	//
	m_pTransformation = NULL ;
	m_pGarbage = NULL ;
	m_flagSorting = false ;
	m_flagCircumscribed = false ;
	//
	S3DRenderParameterContext::DefaultOptionalContext( m_optContext ) ;
	m_nCurShaderHash = 0 ;
	m_nRenderPriority = 0x04 ;
	//
	m_pvvbLast = NULL ;
	m_pFirstDevBuf = NULL ;
}

S3DRenderBuffer::S3DRenderBuffer( const S3DRenderBuffer & buf )
	: m_matCamera( buf.m_matCamera ), m_vCameraPos( buf.m_vCameraPos )
{
	m_pDefaultMaterial = NULL ;
	m_nBufCtrlFlags = 0 ;
	//
	m_signalFreeRefMesh.Initialize( true ) ;
	//
	m_iShiftOffset = 0 ;
	m_nRefShiftMesh = 0 ;
	m_iFenceOrder = 0 ;
	m_pLastShaderContext = NULL ;
	//
	m_pTransformation = NULL ;
	m_pGarbage = NULL ;
	buf.RenderBufferTo( this ) ;
	m_flagSorting = buf.m_flagSorting ;
	//
	m_flagCircumscribed = buf.m_flagCircumscribed ;
	m_vCircumscribedCenter = buf.m_vCircumscribedCenter ;
	m_fpCircumscribedRadius = buf.m_fpCircumscribedRadius ;
	//
	S3DRenderParameterContext::DefaultOptionalContext( m_optContext ) ;
	m_nCurShaderHash = 0 ;
	m_nRenderPriority = 0x04 ;
	//
	m_pvvbLast = NULL ;
	m_pFirstDevBuf = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::~S3DRenderBuffer( void )
{
	ResetTransformation() ;
	//
	TransformationList *	pGarbage = m_pGarbage ;
	while ( pGarbage != NULL )
	{
		TransformationList *	pPrev = pGarbage->pPrev ;
		delete	pGarbage ;
		pGarbage = pPrev ;
	}
	m_pGarbage = NULL ;
	//
	if ( m_pFirstDevBuf != NULL )
	{
		delete	m_pFirstDevBuf ;
		m_pFirstDevBuf = NULL ;
	}
}

// バッファにデータがあるか？
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::IsEmptyBuffer( void ) const
{
	return	(m_arrRender.GetLength() <= m_iShiftOffset) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const S3DRenderBuffer &
	S3DRenderBuffer::operator = ( const S3DRenderBuffer & buf )
{
	ClearBuffer() ;
	buf.RenderTemporaryBufferTo( this ) ;
	return	*this ;
}


// 3D 変換行列を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::GetTransformMatrix( S3DMatrix& mat, S3DVector& pos ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		mat = pTransform->matTransform ;
		pos = pTransform->vTransform ;
		return	true ;
	}
	else
	{
		mat.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
		pos = S3DVector( 0, 0, 0 ) ;
		return	false ;
	}
}

// 透明度を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int S3DRenderBuffer::GetTransparency( void ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		return	pTransform->nTransparency ;
	}
	else
	{
		return	0 ;
	}
}

// 色効果を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::GetColorEffect( S3DColor& colorEffect ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		colorEffect = pTransform->colorEffect ;
		return	!(((colorEffect.rgbMul.ui32 & 0x00FFFFFF) == 0x00FFFFFF)
							& ((colorEffect.rgbAdd.ui32 & 0x00FFFFFF) == 0)) ;
	}
	else
	{
		colorEffect.rgbMul.ui32 = 0x00FFFFFF ;
		colorEffect.rgbAdd.ui32 = 0 ;
		return	false ;
	}
}

// 以前に Push された Transformation 取得
//////////////////////////////////////////////////////////////////////////////
const S3DRenderBuffer::Transformation *
		S3DRenderBuffer::GetPrevTransformation( void ) const
{
	Transformation *	pPrev = nullptr ;
	m_csBufSync.Lock() ;
	if ( m_pTransformation != nullptr )
	{
		pPrev = m_pTransformation->pPrev ;
	}
	m_csBufSync.Unlock() ;
	return	pPrev ;
}

// ソートを有効／無効化
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::EnableSorting( bool flagSorting )
{
	m_flagSorting = flagSorting ;
}

// カメラ変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	m_matCamera = matCamera ;
	m_vCameraPos = posCamera ;
}

// 直接 S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::RenderTemporaryBufferTo
	( S3DRenderBufferInterface * render,
			uint64_t flagsExclusion, size_t iFirst, ssize_t iEnd,
			size_t nInstancing,
			const S4DMatrix * pmatInstancing,
			const S3DColor * pColorInstancing ) const
{
	if ( render == NULL )
	{
		return	sglErrInvalidParam ;
	}
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	size_t	nCount = m_arrRender.GetLength() - m_iShiftOffset ;
	if ( (nCount == 0) | (iFirst >= nCount) )
	{
		return	sglErrSuccess ;
	}
	if ( (nInstancing != 0) && (pmatInstancing != NULL) )
	{
		for ( size_t i = 0; i < nInstancing; i ++ )
		{
			S3DDMatrix	matTrans ;
			S3DDVector	vTrans ;
			Matrix3x3From4x4<S3DDMatrix,S3DDVector>
				( matTrans, vTrans, pmatInstancing[i] ) ;
			//
			const S3DColor *	pColor = NULL ;
			unsigned int		nTransparency = 0 ;
			if ( pColorInstancing != NULL )
			{
				pColor = pColorInstancing + i ;
				nTransparency = 0xFF - pColor->rgbMul.argb.Alpha ;
			}
			//
			render->PushTransformation() ;
			render->AppendMatrixTransformation
				( matTrans, vTrans, pColor, nTransparency ) ;
			RenderTemporaryBufferTo
				( render, flagsExclusion, iFirst, iEnd, 0, NULL, NULL ) ;
			render->PopTransformation() ;
		}
		return	sglErrSuccess ;
	}
	RENDER_ENTRY*const*		ppRender = m_arrRender.GetConstArray() + m_iShiftOffset ;
	ShaderContextEntry *	pShaderContext = NULL ;
	//
	OptionalContextSet	optContext ;
	optContext.nOptionMask = optionContextAll ;
	render->GetOptionalFeature
		( featureContextSet,
			0, &optContext, sizeof(OptionalContextSet) ) ;
	//
	render->PushTransformation() ;
	//
	SGLError		errLast = sglErrSuccess ;
	if ( iEnd < (ssize_t) iFirst )
	{
		iEnd = (ssize_t) nCount ;
	}
	for ( size_t i = iFirst; i < (size_t) iEnd; i ++ )
	{
		RENDER_ENTRY *	pre = ppRender[i] ;
		if ( pre == NULL )
		{
			if ( i >= m_arrRender.GetLength() - m_iShiftOffset )
			{
				break ;
			}
			continue ;
		}
		if ( !pre->flagRenderable )
		{
			i ++ ;
			continue ;
		}
		S3DMaterial *	pMaterial = pre->pMaterial ;
		if ( (pMaterial != nullptr)
			&& (pMaterial->m_attrSurface.flagsShading & flagsExclusion) )
		{
			if ( !pMaterial->m_flagBack
				|| (pMaterial->m_attrBack.flagsShading & flagsExclusion) )
			{
				continue ;
			}
		}
		if ( pre->pShaderContext != pShaderContext )
		{
			pShaderContext = pre->pShaderContext ;
			//
			if ( (pShaderContext != NULL)
				&& !S3DRenderParameterContext::IsEqualOptionalContext
							( optContext, pShaderContext->optContext ) )
			{
				render->SetOptionalFeature
					( featureContextSet,
						0, &(pShaderContext->optContext),
								sizeof(OptionalContextSet) ) ;
				optContext = pShaderContext->optContext ;
			}
			else
			{
				if ( pShaderContext == NULL )
				{
					render->AttachCustomShader( NULL ) ;
					optContext.pShader = NULL ;
				}
			}
			//
			render->ResetCustomShaderUniform() ;
			//
			if ( pShaderContext != NULL )
			{
				CustomUniformEntry *	pcuEntry = pShaderContext->pUniformBuf ;
				for ( size_t j = 0; j < pShaderContext->nUnitofms; j ++ )
				{
					render->SetCustomShaderUniform
						( pcuEntry->m_pwszID, pcuEntry->m_type,
							pcuEntry->m_pData, pcuEntry->m_nLength ) ;
					pcuEntry ++ ;
				}
			}
		}
		Transformation *	pTrans = pre->pTransform ;
		if ( pTrans != NULL )
		{
			render->SetMatrixTransformation
				( pTrans->matTransform,
					pTrans->vTransform,
					&(pTrans->colorEffect), pTrans->nTransparency ) ;
		}
		SGLError	err = sglErrNotSupported ;
		if ( pre->nType == typeVertexBuffer )
		{
			size_t				nVBOInstancing = pre->nInstancingCount ;
			const S4DMatrix *	pVBOInstancingMatrix = pre->pInstancingMatrix ;
			const S3DColor *	pVBOInstancingColor = pre->pInstancingColor ;
			//
			size_t		nMergeEntries = 1 ;
			while ( i + nMergeEntries < (size_t) iEnd )
			{
				RENDER_ENTRY *	preNext = ppRender[i + nMergeEntries] ;
				if ( (preNext == NULL)
					|| !preNext->flagRenderable )
				{
					continue ;
				}
				if ( (preNext->nType != typeVertexBuffer)
					|| (preNext->pVertexBuffer != pre->pVertexBuffer)
					|| (preNext->iFirstBuf != pre->iFirstBuf)
					|| (preNext->iEndBuf != pre->iEndBuf)
					|| (preNext->pShaderContext != pre->pShaderContext) )
				{
					break ;
				}
				nMergeEntries ++ ;
			}
			if ( nMergeEntries > 1 )
			{
				S3DDVector	vInstancingBase( 0, 0, 0 ) ;
				if ( pTrans != NULL )
				{
					vInstancingBase = pTrans->vTransform ;
				}
				nVBOInstancing =
					((S3DRenderBuffer*)this)->MergeEntryInstancing
									( i, nMergeEntries, vInstancingBase ) ;
				pVBOInstancingMatrix = m_bufTempInstancingMatrix.GetConstArray() ;
				pVBOInstancingColor = m_bufTempInstancingColor.GetConstArray() ;
				ESLAssert( nVBOInstancing != 0 ) ;
				//
				S3DDMatrix	matI( 1, 1, 1 ) ;
				S3DColor	clrTrans( 0xFFFFFF, 0 ) ;
				render->SetMatrixTransformation
						( matI, vInstancingBase, &clrTrans, 0 ) ;
			}
			err = render->AddVertexBuffer
				( pre->pMaterial, pre->nFlags,
					pre->pVertexBuffer, pre->iFirstBuf, pre->iEndBuf,
					nVBOInstancing,
					pVBOInstancingMatrix, pVBOInstancingColor ) ;
			//
			i += nMergeEntries - 1 ;
		}
		else
		{
			if ( pre->IsMorphing() )
			{
				((S3DRenderBuffer*)this)->MorphMeshVertics( *pre ) ;
				//
				err = render->AddIndexedPrimitiveList
						( pMaterial, pre->nFlags,
							(S3DPrimitiveType) pre->nType,
							pre->countIndex, pre->countVertex,
							pre->pvTempVertex, pre->pvTempNormal,
							pre->pvTempUVMap, pre->pvTempColor, pre->pIndexedList ) ;
			}
			else if ( pre->countBone != 0 )
			{
				((S3DRenderBuffer*)this)->TransformMeshVerticsByBone( *pre ) ;
				//
				err = render->AddIndexedPrimitiveList
						( pMaterial, pre->nFlags,
							(S3DPrimitiveType) pre->nType,
							pre->countIndex, pre->countVertex,
							pre->pvTempVertex, pre->pvTempNormal,
							pre->pvTempUVMap, pre->pvTempColor, pre->pIndexedList ) ;
			}
			else
			{
				err = render->AddIndexedPrimitiveList
						( pMaterial, pre->nFlags,
							(S3DPrimitiveType) pre->nType,
							pre->countIndex, pre->countVertex,
							pre->pvVertex, pre->pvNormal,
							pre->pvUVMap, pre->pColor, pre->pIndexedList ) ;
			}
		}
		if ( err )
		{
			errLast = err ;
		}
	}
	render->PopTransformation() ;
	return	errLast ;
}

// Flush でフェンスされた範囲を順次 S3DRenderBufferInterface へ出力し、
// スレッドセーフに順次バッファリストから除外する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::FlushRenderTemporaryBufferTo
	( S3DRenderBufferInterface * render,
			uint64_t flagsExclusion, size_t iFirst, ssize_t iEnd )
{
	m_csBufSync.Lock() ;
	if ( m_iShiftOffset >= m_iFenceOrder )
	{
		m_csBufSync.Unlock() ;
		return	sglErrSuccess ;
	}
	m_csBufSync.Unlock() ;

	ShaderContextEntry *	pShaderContext = NULL ;
	OptionalContextSet		optContext ;
	optContext.nOptionMask = optionContextAll ;
	render->GetOptionalFeature
		( featureContextSet,
			0, &optContext, sizeof(OptionalContextSet) ) ;
	//
	render->PushTransformation() ;
	//
	SGLError	errLast = sglErrSuccess ;
	for ( ; ; )
	{
		RENDER_ENTRY *	pre = ShiftMeshEntry() ;
		if ( pre == NULL )
		{
			break ;
		}
		if ( !pre->flagRenderable )
		{
			ReleaseShiftMeshEntry( pre ) ;
			continue ;
		}
		S3DMaterial *	pMaterial = pre->pMaterial ;
		if ( pMaterial != nullptr )
		{
			if ( pMaterial->m_attrSurface.flagsShading & flagsExclusion )
			{
				if ( !pMaterial->m_flagBack
					|| (pMaterial->m_attrBack.flagsShading & flagsExclusion) )
				{
					ReleaseShiftMeshEntry( pre ) ;
					continue ;
				}
			}
		}
		SGLError	err =
			RenderEntryTemporaryBufferTo
				( render, pre, pShaderContext, optContext ) ;
		if ( err )
		{
			errLast = err ;
		}
		ReleaseShiftMeshEntry( pre ) ;
	}
	//
	render->PopTransformation() ;
	return	errLast ;
}

SGLError S3DRenderBuffer::RenderEntryTemporaryBufferTo
	( S3DRenderBufferInterface * render,
		S3DRenderBuffer::RENDER_ENTRY * pre,
		S3DRenderBuffer::ShaderContextEntry *& pShaderContext,
		S3DRenderContextInterface::OptionalContextSet& optContext )
{
	if ( pre->pShaderContext != pShaderContext )
	{
		pShaderContext = pre->pShaderContext ;
		//
		if ( (pShaderContext != NULL)
			&& !S3DRenderParameterContext::IsEqualOptionalContext
						( optContext, pShaderContext->optContext ) )
		{
			render->SetOptionalFeature
				( featureContextSet,
					0, &(pShaderContext->optContext),
							sizeof(OptionalContextSet) ) ;
			optContext = pShaderContext->optContext ;
		}
		else
		{
			if ( pShaderContext == NULL )
			{
				render->AttachCustomShader( NULL ) ;
				optContext.pShader = NULL ;
			}
		}
		//
		render->ResetCustomShaderUniform() ;
		//
		if ( pShaderContext != NULL )
		{
			CustomUniformEntry *	pcuEntry = pShaderContext->pUniformBuf ;
			for ( size_t j = 0; j < pShaderContext->nUnitofms; j ++ )
			{
				render->SetCustomShaderUniform
					( pcuEntry->m_pwszID, pcuEntry->m_type,
						pcuEntry->m_pData, pcuEntry->m_nLength ) ;
				pcuEntry ++ ;
			}
		}
	}
	Transformation *	pTrans = pre->pTransform ;
	if ( pTrans != NULL )
	{
		render->SetMatrixTransformation
			( pTrans->matTransform,
				pTrans->vTransform,
				&(pTrans->colorEffect), pTrans->nTransparency ) ;
	}
	SGLError	err = sglErrNotSupported ;
	if ( pre->nType == typeVertexBuffer )
	{
		size_t				nVBOInstancing = pre->nInstancingCount ;
		const S4DMatrix *	pVBOInstancingMatrix = pre->pInstancingMatrix ;
		const S3DColor *	pVBOInstancingColor = pre->pInstancingColor ;
		//
		err = render->AddVertexBuffer
			( pre->pMaterial, pre->nFlags,
				pre->pVertexBuffer, pre->iFirstBuf, pre->iEndBuf,
				nVBOInstancing,
				pVBOInstancingMatrix, pVBOInstancingColor ) ;
	}
	else
	{
		S3DMaterial *	pMaterial = pre->pMaterial ;
		if ( pre->IsMorphing() )
		{
			((S3DRenderBuffer*)this)->MorphMeshVertics( *pre ) ;
			//
			err = render->AddIndexedPrimitiveList
					( pMaterial, pre->nFlags,
						(S3DPrimitiveType) pre->nType,
						pre->countIndex, pre->countVertex,
						pre->pvTempVertex, pre->pvTempNormal,
						pre->pvTempUVMap, pre->pvTempColor, pre->pIndexedList ) ;
		}
		else if ( pre->countBone != 0 )
		{
			((S3DRenderBuffer*)this)->TransformMeshVerticsByBone( *pre ) ;
			//
			err = render->AddIndexedPrimitiveList
					( pMaterial, pre->nFlags,
						(S3DPrimitiveType) pre->nType,
						pre->countIndex, pre->countVertex,
						pre->pvTempVertex, pre->pvTempNormal,
						pre->pvTempUVMap, pre->pvTempColor, pre->pIndexedList ) ;
		}
		else
		{
			err = render->AddIndexedPrimitiveList
					( pMaterial, pre->nFlags,
						(S3DPrimitiveType) pre->nType,
						pre->countIndex, pre->countVertex,
						pre->pvVertex, pre->pvNormal,
						pre->pvUVMap, pre->pColor, pre->pIndexedList ) ;
		}
	}
	return	err ;
}

// 指定の VertexBuffer 又は バリアントを
// VertexBuffer として S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::RenderVertexBufferTo
	( S3DRenderBufferInterface * render,
		S3DVertexBufferInterface * pvb,
		uint64_t flagsExclusion, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing ) const
{
	if ( render == NULL )
	{
		return	sglErrInvalidParam ;
	}
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	size_t	nCount = m_arrRender.GetLength() - m_iShiftOffset ;
	if ( (nCount == 0) | (iFirst >= nCount) )
	{
		return	sglErrSuccess ;
	}
	render->PushTransformation() ;
	//
	const bool		flagVariantMultiInstanceBuf =
						(nInstancing == 0)
						&& pvb->IsMultiInstancingMode()
						&& (pvb->GetInstancingCount() > 0) ;
	SGLError		errLast = sglErrSuccess ;
	if ( iEnd < (ssize_t) iFirst )
	{
		iEnd = (ssize_t) nCount ;
	}
	RENDER_ENTRY*const*		ppRender = m_arrRender.GetConstArray() + m_iShiftOffset ;
	//
	size_t	i = iFirst ;
	while ( i < (size_t) iEnd )
	{
		RENDER_ENTRY *	pre = ppRender[i] ;
		if ( pre == NULL )
		{
			if ( i >= m_arrRender.GetLength() - m_iShiftOffset )
			{
				break ;
			}
			i ++ ;
			continue ;
		}
		if ( !pvb->IsEnabledToRenderMesh( i ) )
		{
			i ++ ;
			continue ;
		}
		S3DMaterial *	pMaterial = pre->pMaterial ;
		if ( (pMaterial != nullptr)
			&& (pMaterial->m_attrSurface.flagsShading & flagsExclusion) )
		{
			if ( !pMaterial->m_flagBack
				|| (pMaterial->m_attrBack.flagsShading & flagsExclusion) )
			{
				i ++ ;
				continue ;
			}
		}
		Transformation *	pTrans = pre->pTransform ;
		if ( pTrans != NULL )
		{
			render->SetMatrixTransformation
				( pTrans->matTransform,
					pTrans->vTransform,
					&(pTrans->colorEffect), pTrans->nTransparency ) ;
		}
		SGLError	err = sglErrSuccess ;
		if ( pre->nType == typeVertexBuffer )
		{
			if ( pre->nInstancingCount == 0 )
			{
				err = render->AddVertexBuffer
					( pre->pMaterial, pre->nFlags,
						pre->pVertexBuffer, pre->iFirstBuf, pre->iEndBuf,
						nInstancing, pmatInstancing, pColorInstancing ) ;
			}
			else
			{
				err = render->AddVertexBuffer
					( pre->pMaterial, pre->nFlags,
						pre->pVertexBuffer, pre->iFirstBuf, pre->iEndBuf,
						pre->nInstancingCount,
						pre->pInstancingMatrix, pre->pInstancingColor ) ;
			}
			i ++ ;
		}
		else
		{
			S3DVertexBufferInterface *	pvbTemp = pvb ;
			if ( !flagVariantMultiInstanceBuf
				&& (pre->countBone == 0) && (pre->countMorph == 0) )
			{
				pvbTemp = (S3DRenderBuffer*) this ;
			}
			ssize_t	iNext = (ssize_t) i + 1 ;
			while ( iNext < iEnd )
			{
				RENDER_ENTRY *	preNext = ppRender[iNext] ;
				if ( preNext == NULL )
				{
					break ;
				}
				if ( preNext->pMaterial != pMaterial )
				{
					break ;
				}
				if ( preNext->pTransform != pTrans )
				{
					break ;
				}
				if ( (preNext->countBone != 0) | (preNext->countMorph != 0) )
				{
					pvbTemp = pvb ;
				}
				iNext ++ ;
			}
			err = render->AddVertexBuffer
				( pre->pMaterial, pre->nFlags, pvbTemp, i, iNext,
					nInstancing, pmatInstancing, pColorInstancing ) ;
			i = (size_t) iNext ;
		}
		if ( err )
		{
			errLast = err ;
		}
	}
	render->PopTransformation() ;
	return	errLast ;
}

// インスタンス結合
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderBuffer::MergeEntryInstancing
	( size_t iFirst, size_t nCount, const S3DDVector& vInstancingBase )
{
	static atomic_int_t	s_nExclusion = 0 ;
	while ( AtomicXchg( &s_nExclusion, 1 ) != 0 )
	{
		// ※そもそも複数のスレッドから呼び出してはいけない
		ESLAssert( 0 ) ;
	}
	m_bufTempInstancingMatrix.SetLength( 0 ) ;
	m_bufTempInstancingColor.SetLength( 0 ) ;
	//
	RENDER_ENTRY*const*	ppRender = m_arrRender.GetConstArray() + m_iShiftOffset ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RENDER_ENTRY *	pre = ppRender[iFirst + i] ;
		if ( (pre == NULL) || !pre->flagRenderable )
		{
			continue ;
		}
		Transformation *	pTrans = pre->pTransform ;
		S3DMatrix	matBase( 1, 1, 1 ) ;
		S3DVector	vBasePos = - vInstancingBase ;
		S3DColor	clrBase( 0xFFFFFF, 0 ) ;
		uint32_t	nBaseAlpha = 0xFF ;
		if ( pTrans != NULL )
		{
			matBase = pTrans->matTransform ;
			vBasePos = pTrans->vTransform - vInstancingBase ;
			clrBase = pTrans->colorEffect ;
			nBaseAlpha =
				(uint32_t) esl_clampi
						( 0x100 - (int) pTrans->nTransparency, 0, 0xFF ) ;
		}
		S4DMatrix	mat4Base ;
		Matrix4x4From3x3
			<float32_t,S3DMatrix,S3DVector>
					( mat4Base, matBase, vBasePos ) ;
		//
		if ( pre->nInstancingCount == 0 )
		{
			clrBase.rgbMul.argb.Alpha = (uint8_t) nBaseAlpha ;
			//
			m_bufTempInstancingMatrix.Add( mat4Base ) ;
			m_bufTempInstancingColor.Add( clrBase ) ;
		}
		else
		{
			ESLAssert( pre->pInstancingMatrix != NULL ) ;
			ESLAssert( pre->pInstancingColor != NULL ) ;
			//
			ESLAssert( m_bufTempInstancingMatrix.GetLength() == m_bufTempInstancingColor.GetLength() ) ;
			const size_t	iInstance = m_bufTempInstancingMatrix.GetLength() ;
			m_bufTempInstancingMatrix.SetLength( iInstance + pre->nInstancingCount ) ;
			m_bufTempInstancingColor.SetLength( iInstance + pre->nInstancingCount ) ;
			//
			const S4DMatrix *	pSrcMatrix = pre->pInstancingMatrix ;
			const S3DColor *	pSrcColor = pre->pInstancingColor ;
			S4DMatrix *	pMatrix = m_bufTempInstancingMatrix.GetAt( iInstance ) ;
			S3DColor *	pColor = m_bufTempInstancingColor.GetAt( iInstance ) ;
			for ( size_t j = 0; j < pre->nInstancingCount; j ++ )
			{
				S3DColor	clrSrc = pSrcColor[j] ;
				S3DColor	clrInstance = clrBase * clrSrc ;
				clrInstance.rgbMul.argb.Alpha =
					(uint8_t) (((nBaseAlpha + 1)
									* clrSrc.rgbMul.argb.Alpha) >> 8) ;
				pMatrix[j] = mat4Base * pSrcMatrix[j] ;
				pColor[j] = clrInstance ;
			}
		}
	}
	ESLAssert( m_bufTempInstancingMatrix.GetLength() == m_bufTempInstancingColor.GetLength() ) ;
	ESLVerify( AtomicXchg( &s_nExclusion, 0 ) != 0 ) ;
	return	m_bufTempInstancingMatrix.GetLength() ;
}

// メッシュエントリ取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::RENDER_ENTRY *
	S3DRenderBuffer::GetMeshEntryAt( size_t iMesh ) const
{
	return	m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
}

// 先頭のメッシュエントリを取得し、バッファのリストから除外する
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::RENDER_ENTRY * S3DRenderBuffer::ShiftMeshEntry( void )
{
	RENDER_ENTRY *	pre = NULL ;
	m_csBufSync.Lock() ;
	ESLAssert( m_iFenceOrder <= m_arrRender.GetLength() ) ;
	if ( m_iShiftOffset < m_iFenceOrder )
	{
		pre = m_arrRender.GetAt( m_iShiftOffset ++ ) ;
		if ( pre != nullptr )
		{
			if ( ++ m_nRefShiftMesh == 1 )
			{
				m_signalFreeRefMesh.ResetSignal() ;
			}
		}
	}
	m_csBufSync.Unlock() ;
	return	pre ;
}

// ShiftMeshEntry で取得した RENDER_ENTRY の参照を完了した
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::ReleaseShiftMeshEntry( RENDER_ENTRY * pre )
{
	if ( pre != nullptr )
	{
		m_csBufSync.Lock() ;
		if ( -- m_nRefShiftMesh <= 0 )
		{
			ESLAssert( m_nRefShiftMesh == 0 ) ;
			m_signalFreeRefMesh.SetSignal() ;
		}
		m_csBufSync.Unlock() ;
	}
}

// 法線・頂点色・接線要素の生成
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::NormalizeVertexElements
		( S3DRenderBuffer::RENDER_ENTRY& entry, uint32_t nFlags )
{
	if ( (nFlags & renderAutoNormal) && (entry.pvNormal == NULL) )
	{
		m_csBufSync.Lock() ;
		const size_t	countVertex = entry.countVertex ;
		entry.pvNormal =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
		GenerateDefaultNormal( entry ) ;
		m_csBufSync.Unlock() ;
	}
	if ( (nFlags & renderAutoColor) && (entry.pColor == NULL) )
	{
		m_csBufSync.Lock() ;
		const size_t	countVertex = entry.countVertex ;
		entry.pColor =
			(S3DColor*) m_bufRender.Allocate
							( countVertex * sizeof(S3DColor) ) ;
		//
		S3DColor		colorDummy( 0xFFFFFFFF, 0 ) ;
		S3DColor *		pDstColor = entry.pColor ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			*pDstColor = colorDummy ;
			pDstColor ++ ;
		}
		m_csBufSync.Unlock() ;
	}
	if ( (nFlags & renderAutoTexAxis)
		&& (entry.pvUVMap != NULL)
		&& (entry.pvTexAxisX == NULL) && (entry.pvTexAxisY == NULL) )
	{
		m_csBufSync.Lock() ;
		const size_t	countVertex = entry.countVertex ;
		entry.pvTexAxisX =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
		entry.pvTexAxisY =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
		//
		if ( m_bufTempTexAxis.SetForIndexedPrimitiveList
				( (S3DPrimitiveType) entry.nType,
					entry.countPrimitive, entry.countVertex,
					entry.pvVertex, entry.pvUVMap, entry.pIndexedList ) )
		{
			eslCopyMemory
				( entry.pvTexAxisX,
					m_bufTempTexAxis.GetBufferAxisX(),
					entry.countVertex * sizeof(S3DVector4) ) ;
			eslCopyMemory
				( entry.pvTexAxisY,
					m_bufTempTexAxis.GetBufferAxisY(),
					entry.countVertex * sizeof(S3DVector4) ) ;
		}
		m_csBufSync.Unlock() ;
	}
}

// モーフィングによる頂点変換処理
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MorphMeshVertics( S3DRenderBuffer::RENDER_ENTRY& entry )
{
	if ( (entry.countMorph == 0) || !entry.flagUpdateMorph )
	{
		TransformMeshVerticsByBone( entry ) ;
		return ;
	}
	//
	// 中間バッファ準備
	//
	size_t	countVertex = entry.countVertex ;
	if ( entry.pvTempMorphVertex == NULL )
	{
		entry.pvTempMorphVertex =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	if ( entry.pvTempMorphNormal == NULL )
	{
		entry.pvTempMorphNormal =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	if ( (entry.pvUVMap != NULL) && (entry.pvTempUVMap == NULL) )
	{
		entry.pvTempUVMap =
			(S2DVector*) m_bufRender.Allocate
							( countVertex * sizeof(S2DVector) ) ;
	}
	if ( (entry.pColor != NULL) && (entry.pvTempColor == NULL) )
	{
		entry.pvTempColor =
			(S3DColor*) m_bufRender.Allocate
							( countVertex * sizeof(S3DColor) ) ;
	}
	entry.pvTempVertex = entry.pvTempMorphVertex ;
	entry.pvTempNormal = entry.pvTempMorphNormal ;
	//
	if ( entry.nTargetMeshCount == 0 )
	{
		//
		// モーフィング無し
		//
		eslCopyMemory
			( entry.pvTempVertex, entry.pvVertex,
						countVertex * sizeof(S3DVector4) ) ;
		eslCopyMemory
			( entry.pvTempNormal, entry.pvNormal,
						countVertex * sizeof(S3DVector4) ) ;
		//
		if ( entry.pvUVMap != NULL )
		{
			eslCopyMemory
				( entry.pvTempUVMap, entry.pvUVMap,
							countVertex * sizeof(S2DVector) ) ;
		}
		if ( entry.pColor != NULL )
		{
			eslCopyMemory
				( entry.pvTempColor, entry.pColor,
							countVertex * sizeof(S3DColor) ) ;
		}
	}
	else
	{
		//
		// モーフィング処理
		//
		eslFillMemory
			( entry.pvTempVertex, 0, countVertex * sizeof(S3DVector4) ) ;
		eslFillMemory
			( entry.pvTempNormal, 0, countVertex * sizeof(S3DVector4) ) ;
		if ( entry.pvUVMap != NULL )
		{
			eslFillMemory
				( entry.pvTempUVMap, 0, countVertex * sizeof(S2DVector) ) ;
		}
		if ( entry.pColor != NULL )
		{
			eslFillMemory
				( entry.pvTempColor, 0, countVertex * sizeof(S3DColor) ) ;
		}
		S3DVector4 *	pvVertexBuf = entry.pvTempVertex ;
		S3DVector4 *	pvNormalBuf = entry.pvTempNormal ;
		S2DVector *		pvUVMapBuf = entry.pvTempUVMap ;
		S3DColor *		pColorBuf = entry.pvTempColor ;
		if ( entry.flagMorphWithWeight )
		{
			//
			// ウェイトマップ計算
			//
			if ( entry.pfpTempMorphWeight == NULL )
			{
				entry.pfpTempMorphWeight =
					(float32_t*) m_bufRender.Allocate
									( countVertex * sizeof(float32_t) ) ;
			}
			float32_t *		pfpWeightBuf = entry.pfpTempMorphWeight ;
			eslFillMemory
				( pfpWeightBuf, 0, countVertex * sizeof(float32_t) ) ;
			for ( size_t i = 0; i < entry.nTargetMeshCount; i ++ )
			{
				ssize_t		iMorphTarget = entry.pMorphTargetMesh[i] ;
				float32_t	fpApply = entry.pMorphApplication[i] ;
				if ( iMorphTarget >= 0 )
				{
					AddProductedVector1DArray
						( pfpWeightBuf,
							entry.pfpMorphWeight
								+ iMorphTarget * countVertex,
							fpApply, countVertex ) ;
				}
			}
			//
			// メッシュ合成（ウェイト付き）
			//
			AddProductedVector4DArrayWithNegWeight
				( pvVertexBuf, entry.pvVertex, pfpWeightBuf, countVertex ) ;
			AddProductedVector4DArrayWithNegWeight
				( pvNormalBuf, entry.pvNormal, pfpWeightBuf, countVertex ) ;
			//
			if ( entry.pvUVMap != NULL )
			{
				AddProductedVector2DArrayWithNegWeight
					( pvUVMapBuf, entry.pvUVMap, pfpWeightBuf, countVertex ) ;
			}
			if ( entry.pColor != NULL )
			{
				AddProductedColorPairArrayWithNegWeight
					( pColorBuf, entry.pColor, pfpWeightBuf, countVertex ) ;
			}
			for ( size_t i = 0; i < entry.nTargetMeshCount; i ++ )
			{
				ssize_t	iMorphTarget = entry.pMorphTargetMesh[i] ;
				if ( iMorphTarget < 0 )
				{
					continue ;
				}
				const size_t	iMorphIndex = iMorphTarget * countVertex ;
				S3DVector4 *	pvVertex = entry.pvMorphVertex + iMorphIndex ;
				S3DVector4 *	pvNormal = entry.pvMorphNormal + iMorphIndex ;
				S2DVector *		pvUVMap = entry.pvUVMap ;
				S3DColor *		pColor = entry.pColor ;
				float32_t *		pfpWeight = entry.pfpMorphWeight + iMorphIndex ;
				//
				float32_t	fpApply = entry.pMorphApplication[i] ;
				//
				AddProductedVector4DArrayWithWeight
					( pvVertexBuf, pvVertex, pfpWeight, fpApply, countVertex ) ;
				AddProductedVector4DArrayWithWeight
					( pvNormalBuf, pvNormal, pfpWeight, fpApply, countVertex ) ;
				//
				if ( pvUVMap != NULL )
				{
					pvUVMap = entry.pvMorphUVMap + iMorphIndex ;
					AddProductedVector2DArrayWithWeight
						( pvUVMapBuf, pvUVMap, pfpWeight, fpApply, countVertex ) ;
				}
				if ( pColor != NULL )
				{
					pColor = entry.pMorphColor + iMorphIndex ;
					AddProductedColorPairArrayWithWeight
						( pColorBuf, pColor, pfpWeight, fpApply, countVertex ) ;
				}
			}
		}
		else
		{
			//
			// メッシュ合成（単純モーフィング）
			//
			float32_t	fpNonDefWeight = 0.0f ;
			size_t		i ;
			for ( i = 0; i < entry.nTargetMeshCount; i ++ )
			{
				if ( entry.pMorphTargetMesh[i] >= 0 )
				{
					fpNonDefWeight += entry.pMorphApplication[i] ;
				}
			}
			for ( i = 0; i < entry.nTargetMeshCount; i ++ )
			{
				ssize_t			iMorphTarget = entry.pMorphTargetMesh[i] ;
				S3DVector4 *	pvVertex = entry.pvVertex ;
				S3DVector4 *	pvNormal = entry.pvNormal ;
				S2DVector *		pvUVMap = entry.pvUVMap ;
				S3DColor *		pColor = entry.pColor ;
				float32_t		fpApply = entry.pMorphApplication[i] ;
				if ( iMorphTarget >= 0 )
				{
					pvVertex = entry.pvMorphVertex + iMorphTarget * countVertex ;
					pvNormal = entry.pvMorphNormal + iMorphTarget * countVertex ;
					if ( pvUVMap != NULL )
					{
						pvUVMap = entry.pvMorphUVMap + iMorphTarget * countVertex ;
					}
					if ( pColor != NULL )
					{
						pColor = entry.pMorphColor + iMorphTarget * countVertex ;
					}
				}
				else
				{
					fpApply = 1.0f - fpNonDefWeight ;
				}
				if ( fabs(fpApply) < 1.0e-8 )
				{
					continue ;
				}
				//
				AddProductedVector4DArray
					( pvVertexBuf, pvVertex, fpApply, countVertex ) ;
				AddProductedVector4DArray
					( pvNormalBuf, pvNormal, fpApply, countVertex ) ;
				//
				if ( pvUVMap != NULL )
				{
					AddProductedVector2DArray
						( pvUVMapBuf, pvUVMap, fpApply, countVertex ) ;
				}
				if ( pColor != NULL )
				{
					AddProductedColorPairArray
						( pColorBuf, pColor, fpApply, countVertex ) ;
				}
			}
		}
	}
	//
	entry.flagUpdateMorph = false ;
	entry.flagUpdateBone = (entry.countBone != 0) ;
	//
	TransformMeshVerticsByBone( entry ) ;
}

void S3DRenderBuffer::MorphMeshInfoVertics
	( S3DVertexBufferInterface::MeshInfo& info,
		S3DRenderBuffer::RENDER_ENTRY& entry, size_t iFirst, size_t nCount )
{
	if ( entry.countMorph == 0 )
	{
		return ;
	}
	if ( entry.nTargetMeshCount == 0 )
	{
		return ;
	}
	const size_t	countVertex = entry.countVertex ;
	ESLAssert( iFirst + nCount <= countVertex ) ;
	//
	// クリア
	//
	if ( info.pvVertex != NULL )
	{
		eslFillMemory
			( info.pvVertex, 0, nCount * sizeof(S3DVector4) ) ;
	}
	if ( info.pvNormal != NULL )
	{
		eslFillMemory
			( info.pvNormal, 0, nCount * sizeof(S3DVector4) ) ;
	}
	if ( (info.pvUVMap != NULL) && (entry.pvUVMap != NULL) )
	{
		eslFillMemory
			( info.pvUVMap, 0, nCount * sizeof(S2DVector) ) ;
	}
	if ( (info.pColor != NULL) && (entry.pColor != NULL) )
	{
		eslFillMemory
			( info.pColor, 0, nCount * sizeof(S3DColor) ) ;
	}
	if ( entry.flagMorphWithWeight )
	{
		//
		// ウェイトマップ計算
		//
		if ( entry.pfpTempMorphWeight == NULL )
		{
			entry.pfpTempMorphWeight =
				(float32_t*) m_bufRender.Allocate
								( countVertex * sizeof(float32_t) ) ;
		}
		float32_t *		pfpWeightBuf = entry.pfpTempMorphWeight ;
		eslFillMemory
			( pfpWeightBuf, 0, nCount * sizeof(float32_t) ) ;
		for ( size_t i = 0; i < entry.nTargetMeshCount; i ++ )
		{
			ssize_t		iMorphTarget = entry.pMorphTargetMesh[i] ;
			float32_t	fpApply = entry.pMorphApplication[i] ;
			if ( iMorphTarget >= 0 )
			{
				AddProductedVector1DArray
					( pfpWeightBuf,
						entry.pfpMorphWeight
							+ iFirst
							+ iMorphTarget * countVertex,
						fpApply, nCount ) ;
			}
		}
		//
		// メッシュ合成（ウェイト付き）
		//
		if ( info.pvVertex != NULL )
		{
			AddProductedVector4DArrayWithNegWeight
				( info.pvVertex,
					entry.pvVertex + iFirst, pfpWeightBuf, nCount ) ;
		}
		if ( info.pvNormal != NULL )
		{
			AddProductedVector4DArrayWithNegWeight
				( info.pvNormal,
					entry.pvNormal + iFirst, pfpWeightBuf, nCount ) ;
		}
		if ( (info.pvUVMap != NULL) && (entry.pvUVMap != NULL) )
		{
			AddProductedVector2DArrayWithNegWeight
				( info.pvUVMap, entry.pvUVMap, pfpWeightBuf, nCount ) ;
		}
		if ( (info.pColor != NULL) && (entry.pColor != NULL) )
		{
			AddProductedColorPairArrayWithNegWeight
				( info.pColor, entry.pColor, pfpWeightBuf, nCount ) ;
		}
		for ( size_t i = 0; i < entry.nTargetMeshCount; i ++ )
		{
			ssize_t	iMorphTarget = entry.pMorphTargetMesh[i] ;
			if ( iMorphTarget < 0 )
			{
				continue ;
			}
			const size_t	iMorphIndex = iMorphTarget * countVertex ;
			S3DVector4 *	pvVertex = entry.pvMorphVertex + iMorphIndex ;
			S3DVector4 *	pvNormal = entry.pvMorphNormal + iMorphIndex ;
			S2DVector *		pvUVMap = entry.pvUVMap ;
			S3DColor *		pColor = entry.pColor ;
			float32_t *		pfpWeight = entry.pfpMorphWeight + iMorphIndex ;
			//
			float32_t	fpApply = entry.pMorphApplication[i] ;
			//
			if ( info.pvVertex != NULL )
			{
				AddProductedVector4DArrayWithWeight
					( info.pvVertex,
						pvVertex + iFirst, pfpWeight, fpApply, nCount ) ;
			}
			if ( info.pvNormal != NULL )
			{
				AddProductedVector4DArrayWithWeight
					( info.pvNormal,
						pvNormal + iFirst, pfpWeight, fpApply, nCount ) ;
			}
			if ( (info.pvUVMap != NULL) && (pvUVMap != NULL) )
			{
				pvUVMap = entry.pvMorphUVMap + iMorphIndex ;
				AddProductedVector2DArrayWithWeight
					( info.pvUVMap,
						pvUVMap + iFirst, pfpWeight, fpApply, nCount ) ;
			}
			if ( (info.pColor != NULL) && (pColor != NULL) )
			{
				pColor = entry.pMorphColor + iMorphIndex ;
				AddProductedColorPairArrayWithWeight
					( info.pColor,
						pColor + iFirst, pfpWeight, fpApply, nCount ) ;
			}
		}
	}
	else
	{
		//
		// 単純モーフィング
		//
		float32_t	fpNonDefWeight = 0.0f ;
		size_t		i ;
		for ( i = 0; i < entry.nTargetMeshCount; i ++ )
		{
			if ( entry.pMorphTargetMesh[i] >= 0 )
			{
				fpNonDefWeight += entry.pMorphApplication[i] ;
			}
		}
		for ( i = 0; i < entry.nTargetMeshCount; i ++ )
		{
			ssize_t			iMorphTarget = entry.pMorphTargetMesh[i] ;
			S3DVector4 *	pvVertex = entry.pvVertex ;
			S3DVector4 *	pvNormal = entry.pvNormal ;
			S2DVector *		pvUVMap = entry.pvUVMap ;
			S3DColor *		pColor = entry.pColor ;
			float32_t		fpApply = entry.pMorphApplication[i] ;
			if ( iMorphTarget >= 0 )
			{
				pvVertex = entry.pvMorphVertex + iMorphTarget * countVertex ;
				pvNormal = entry.pvMorphNormal + iMorphTarget * countVertex ;
				if ( pvUVMap != NULL )
				{
					pvUVMap = entry.pvMorphUVMap + iMorphTarget * countVertex ;
				}
				if ( pColor != NULL )
				{
					pColor = entry.pMorphColor + iMorphTarget * countVertex ;
				}
			}
			else
			{
				fpApply = 1.0f - fpNonDefWeight ;
			}
			if ( fabs(fpApply) < 1.0e-8 )
			{
				continue ;
			}
			if ( info.pvVertex != NULL )
			{
				AddProductedVector4DArray
					( info.pvVertex, pvVertex + iFirst, fpApply, nCount ) ;
			}
			if ( info.pvNormal != NULL )
			{
				AddProductedVector4DArray
					( info.pvNormal, pvNormal + iFirst, fpApply, nCount ) ;
			}
			if ( (info.pvUVMap != NULL) && (pvUVMap != NULL) )
			{
				AddProductedVector2DArray
					( info.pvUVMap, pvUVMap + iFirst, fpApply, nCount ) ;
			}
			if ( (info.pColor != NULL) && (pColor != NULL) )
			{
				AddProductedColorPairArray
					( info.pColor, pColor +iFirst, fpApply, nCount ) ;
			}
		}
	}
}

// ボーンによる頂点変換処理
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::TransformMeshVerticsByBone
					( S3DRenderBuffer::RENDER_ENTRY& entry )
{
	size_t	countBone = entry.countBone ;
	if ( (countBone == 0) || !entry.flagUpdateBone )
	{
		return ;
	}
	size_t	countVertex = entry.countVertex ;
	if ( entry.pvTempBoneVertex == NULL )
	{
		entry.pvTempBoneVertex =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	if ( entry.pvTempBoneNormal == NULL )
	{
		entry.pvTempBoneNormal =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	S3DVector4 *	pvSrcVertex = entry.pvVertex ;
	S3DVector4 *	pvSrcNormal = entry.pvNormal ;
	if ( entry.IsMorphing() )
	{
		pvSrcVertex = entry.pvTempMorphVertex ? entry.pvTempMorphVertex : pvSrcVertex ;
		pvSrcNormal = entry.pvTempMorphNormal ? entry.pvTempMorphNormal : pvSrcNormal ;
	}
	entry.pvTempVertex = entry.pvTempBoneVertex ;
	entry.pvTempNormal = entry.pvTempBoneNormal ;
	//
	eslFillMemory
		( entry.pvTempVertex, 0, countVertex * sizeof(S3DVector4) ) ;
	eslFillMemory
		( entry.pvTempNormal, 0, countVertex * sizeof(S3DVector4) ) ;
	//
	if ( entry.ppJointMap != NULL )
	{
		size_t	countWeightMap = entry.countWeightMap ;
		for ( size_t i = 0; i < countWeightMap; i ++ )
		{
			AddRevolvedVectorsWithIndexedWeight
				( entry.pvTempVertex,
					entry.pBoneMatrix, entry.pBoneTrans,
					pvSrcVertex, entry.ppJointMap[i],
					entry.ppWeightMap[i], countVertex ) ;
			AddRevolvedVectorsWithIndexedWeight
				( entry.pvTempNormal,
					entry.pBoneMatrix, NULL,
					pvSrcNormal, entry.ppJointMap[i],
					entry.ppWeightMap[i], countVertex ) ;
		}
	}
	else
	{
		for ( size_t i = 0; i < countBone; i ++ )
		{
			const size_t	iBone = i ;
			float32_t *		pWeightMap = entry.ppWeightMap[iBone] ;
			S3DMatrix &		matBone = entry.pBoneMatrix[iBone] ;
			S3DVector		vBone = entry.pBoneTrans[iBone] ;
			S3DVector		vZero( 0, 0, 0 ) ;
			//
			matBone.AddRevolvedVectorsWithWeight
				( entry.pvTempVertex, pvSrcVertex, pWeightMap, countVertex, vBone ) ;
			matBone.AddRevolvedVectorsWithWeight
				( entry.pvTempNormal, pvSrcNormal, pWeightMap, countVertex, vZero ) ;
		}
	}
	entry.flagUpdateBone = false ;
}

void S3DRenderBuffer::TransformMeshVerticsByBone
	( S3DVertexBufferInterface::MeshInfo& info,
		S3DRenderBuffer::RENDER_ENTRY& entry, size_t iFirst, size_t nCount )
{
	const size_t	countBone = entry.countBone ;
	if ( countBone == 0 )
	{
		return ;
	}
	const size_t	countVertex = entry.countVertex ;
	ESLAssert( iFirst + nCount <= countVertex ) ;
	if ( entry.pvTempBoneVertex == NULL )
	{
		entry.pvTempBoneVertex =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	if ( entry.pvTempBoneNormal == NULL )
	{
		entry.pvTempBoneNormal =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	eslFillMemory
		( entry.pvTempBoneVertex + iFirst, 0, nCount * sizeof(S3DVector4) ) ;
	eslFillMemory
		( entry.pvTempBoneNormal + iFirst, 0, nCount * sizeof(S3DVector4) ) ;
	//
	if ( entry.ppJointMap != NULL )
	{
		size_t	countWeightMap = entry.countWeightMap ;
		if ( info.pvVertex != NULL )
		{
			for ( size_t i = 0; i < countWeightMap; i ++ )
			{
				AddRevolvedVectorsWithIndexedWeight
					( entry.pvTempBoneVertex + iFirst,
						entry.pBoneMatrix, entry.pBoneTrans,
						info.pvVertex, entry.ppJointMap[i] + iFirst,
						entry.ppWeightMap[i] + iFirst, nCount ) ;
			}
		}
		if ( info.pvNormal != NULL )
		{
			for ( size_t i = 0; i < countWeightMap; i ++ )
			{
				AddRevolvedVectorsWithIndexedWeight
					( entry.pvTempBoneNormal + iFirst,
						entry.pBoneMatrix, NULL,
						info.pvNormal, entry.ppJointMap[i] + iFirst,
						entry.ppWeightMap[i] + iFirst, nCount ) ;
			}
		}
	}
	else
	{
		for ( size_t i = 0; i < countBone; i ++ )
		{
			const size_t	iBone = i ;
			float32_t *		pWeightMap = entry.ppWeightMap[iBone] ;
			S3DMatrix &		matBone = entry.pBoneMatrix[iBone] ;
			S3DVector		vBone = entry.pBoneTrans[iBone] ;
			S3DVector		vZero( 0, 0, 0 ) ;
			//
			if ( info.pvVertex != NULL )
			{
				matBone.AddRevolvedVectorsWithWeight
					( entry.pvTempBoneVertex + iFirst,
						info.pvVertex, pWeightMap + iFirst, nCount, vBone ) ;
			}
			if ( info.pvNormal != NULL )
			{
				matBone.AddRevolvedVectorsWithWeight
					( entry.pvTempBoneNormal + iFirst,
						info.pvNormal, pWeightMap + iFirst, nCount, vZero ) ;
			}
		}
	}
	if ( info.pvVertex != NULL )
	{
		eslCopyMemory
			( info.pvVertex,
				entry.pvTempBoneVertex + iFirst,
				nCount * sizeof(S3DVector4) ) ;
	}
	if ( info.pvNormal != NULL )
	{
		eslCopyMemory
			( info.pvNormal,
				entry.pvTempBoneNormal + iFirst,
				nCount * sizeof(S3DVector4) ) ;
	}
}

// メッシュ外接球取得（モーフ・ボーン変形有り）
//////////////////////////////////////////////////////////////////////////////
double S3DRenderBuffer::GetCircumscribedSphereTransformed( S3DVector& vCenter )
{
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	size_t				nCount = m_arrRender.GetLength() - m_iShiftOffset ;
	RENDER_ENTRY*const*	ppEntries = m_arrRender.GetConstArray() + m_iShiftOffset ;
	//
	S3DVector	vMax( 0, 0, 0 ), vMin( 0, 0, 0 ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RENDER_ENTRY *	pre = ppEntries[i] ;
		TransformMeshVerticsByBone( *pre ) ;
		//
		S3DVector4	vMaxTemp, vMinTemp ;
		if ( pre->pvTempVertex != NULL )
		{
			MinMaxVector4DArray
				( vMinTemp, vMaxTemp, pre->pvTempVertex, pre->countVertex ) ;
		}
		else
		{
			MinMaxVector4DArray
				( vMinTemp, vMaxTemp, pre->pvVertex, pre->countVertex ) ;
		}
		if ( i >= 1 )
		{
			vMax.x = esl_fmaxf( vMax.x, vMaxTemp.x ) ;
			vMax.y = esl_fmaxf( vMax.y, vMaxTemp.y ) ;
			vMax.z = esl_fmaxf( vMax.z, vMaxTemp.z ) ;
			//
			vMin.x = esl_fminf( vMin.x, vMinTemp.x ) ;
			vMin.y = esl_fminf( vMin.y, vMinTemp.y ) ;
			vMin.z = esl_fminf( vMin.z, vMinTemp.z ) ;
		}
		else
		{
			vMax = vMaxTemp ;
			vMin = vMinTemp ;
		}
	}
	vCenter = (vMax + vMin) * 0.5f ;
	return	(vMax - vMin).Absolute() * 0.5 ;
}

// スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::LockSyncBuffer( void ) const
{
	m_csBufSync.Lock() ;
}

void S3DRenderBuffer::UnlockSyncBuffer( void ) const
{
	m_csBufSync.Unlock() ;
}

bool S3DRenderBuffer::TestLockedSyncBuffer( void ) const
{
	return	m_csBufSync.TestLocked() ;
}

// 全てのプリミティブが同一タイプ・同一マテリアル・同一拡張属性数で、
// ボーン・モーフィングがない場合、結合して単一のプリミティブとして再構築する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::RebuildAsSinglePrimitive( void )
{
	if ( m_arrRender.GetLength() <= m_iShiftOffset + 1 )
	{
		return	sglErrSuccess ;
	}
	MeshBuffer	mbufTemp ;
	SGLError	err = GetMeshBufferAsSinglePrimitive( mbufTemp ) ;
	if ( err )
	{
		return	err ;
	}
	ClearBuffer() ;
	//
	err = AddIndexedPrimitiveList
		( mbufTemp.m_pMaterial, 0,
			mbufTemp.m_type, mbufTemp.m_nIndexCount,
			mbufTemp.m_nVertexCount,
			mbufTemp.m_bufVertex.GetConstArray(),
			mbufTemp.m_bufNormal.GetConstArray(),
			mbufTemp.m_bufUVMap.GetConstArray(),
			mbufTemp.m_bufColor.GetConstArray(),
			mbufTemp.m_bufIndex.GetConstArray() ) ;
	if ( err )
	{
		return	err ;
	}
	if ( mbufTemp.m_nExAttrCount > 0 )
	{
		err = SetExtendVertexAttribute
			( 0, mbufTemp.m_nExAttrCount,
				mbufTemp.m_nVertexCount,
				mbufTemp.m_bufExAttr.GetConstArray() ) ;
	}
	return	err ;
}

SGLError S3DRenderBuffer::GetMeshBufferAsSinglePrimitive
				( S3DRenderBuffer::MeshBuffer& mbuf ) const
{
	for ( size_t i = m_iShiftOffset; i < m_arrRender.GetLength(); i ++ )
	{
		RENDER_ENTRY *	pre = m_arrRender.GetAt( i ) ;
		ESLAssert( pre != NULL ) ;
		if ( pre == NULL )
		{
			continue ;
		}
		if ( (pre->countBone > 0)
			|| (pre->countMorph > 0)
			|| (pre->nType == typeVertexBuffer) )
		{
			return	sglErrFailed ;
		}
		SGLError	err = mbuf.AddIndexedPrimitiveList
			( pre->pMaterial, (S3DPrimitiveType) pre->nType,
				pre->countIndex,
				pre->countVertex, pre->nExAttrElements,
				pre->pvVertex, pre->pvNormal,
				pre->pvUVMap, pre->pColor,
				pre->pfpExAttrElements, pre->pIndexedList ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::RebuildAsSinglePrimitiveForVB( S3DVertexBufferInterface& vb )
{
	MeshBuffer	mbufTemp ;
	SGLError	err = GetMeshBufferAsSinglePrimitiveOfVB( mbufTemp, vb ) ;
	if ( err )
	{
		return	err ;
	}
	vb.ClearBuffer() ;
	//
	if ( mbufTemp.m_nVertexCount == 0 )
	{
		return	err ;
	}
	err = vb.AddIndexedPrimitiveList
		( mbufTemp.m_pMaterial, 0,
			mbufTemp.m_type, mbufTemp.m_nIndexCount,
			mbufTemp.m_nVertexCount,
			mbufTemp.m_bufVertex.GetConstArray(),
			mbufTemp.m_bufNormal.GetConstArray(),
			mbufTemp.m_bufUVMap.GetConstArray(),
			mbufTemp.m_bufColor.GetConstArray(),
			mbufTemp.m_bufIndex.GetConstArray() ) ;
	if ( err )
	{
		return	err ;
	}
	if ( mbufTemp.m_nExAttrCount > 0 )
	{
		err = vb.SetExtendVertexAttribute
			( 0, mbufTemp.m_nExAttrCount,
				mbufTemp.m_nVertexCount,
				mbufTemp.m_bufExAttr.GetConstArray() ) ;
	}
	return	err ;
}

SGLError S3DRenderBuffer::GetMeshBufferAsSinglePrimitiveOfVB
				( MeshBuffer& mbuf, S3DVertexBufferInterface& vb )
{
	SArray<S3DVector4>	bufVertex ;
	SArray<S3DVector4>	bufNormal ;
	SArray<S2DVector>	bufUVMap ;
	SArray<S3DColor>	bufColor ;
	SArray<float32_t>	bufExAttr ;
	SArray<uint32_t>	bufIndex ;

	const size_t	nCount = vb.GetMeshCount() ;
	for ( size_t iMesh = 0; iMesh < nCount; iMesh ++ )
	{
		S3DVertexBufferInterface::MeshInfo	miMesh ;
		eslFillMemory( &miMesh, 0, sizeof(miMesh) ) ;
		//
		if ( vb.GetMeshInfoAt( miMesh, iMesh, 0 ) )
		{
			continue ;
		}
		const size_t	nIndexCount =
			miMesh.countPrimitive * GetPrimitiveVertexCount(miMesh.typeMesh) ;
		miMesh.pvVertex = bufVertex.GetArray( miMesh.countVertex ) ;
		miMesh.pvNormal = bufNormal.GetArray( miMesh.countVertex ) ;
		miMesh.pvUVMap = bufUVMap.GetArray( miMesh.countVertex ) ;
		miMesh.pColor = bufColor.GetArray( miMesh.countVertex ) ;
		miMesh.pfpExAttrElements =
				bufExAttr.GetArray( miMesh.countVertex * miMesh.nExAttrElements ) ;
		miMesh.pIndexedList = bufIndex.GetArray( nIndexCount ) ;
		//
		if ( vb.GetMeshInfoAt( miMesh, iMesh, miMesh.countVertex ) )
		{
			continue ;
		}
		SGLError	err = mbuf.AddIndexedPrimitiveList
			( miMesh.pMaterial, miMesh.typeMesh,
				nIndexCount, miMesh.countVertex, miMesh.nExAttrElements,
				miMesh.pvVertex, miMesh.pvNormal,
				miMesh.pvUVMap, miMesh.pColor,
				miMesh.pfpExAttrElements, miMesh.pIndexedList ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	sglErrSuccess ;
}

// シェーディングフラグ（デフォルトシェーダー）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetShadingFlag( uint64_t nShadingMethod )
{
	if ( m_optContext.nShadingFlags != nShadingMethod )
	{
		m_optContext.nShadingFlags = nShadingMethod ;
		if ( m_optContext.pShader == NULL )
		{
			m_nCurShaderHash = GetDefaultShaderType( m_optContext.nShadingFlags ) ;
			//
			TransformationList *	pTransform = m_pTransformation ;
			if ( pTransform != NULL )
			{
				pTransform->pContextBuf = NULL ;
			}
		}
	}
}

// シェーディング取得
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DRenderBuffer::GetShadingFlag( void )
{
	return	m_optContext.nShadingFlags ;
}

// カスタムシェーダー
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AttachCustomShader( S3DCustomShader * pShader )
{
	if ( m_optContext.pShader != pShader )
	{
		m_optContext.pShader = pShader ;
		if ( m_optContext.pShader == NULL )
		{
			m_nCurShaderHash = GetDefaultShaderType( m_optContext.nShadingFlags ) ;
		}
		else
		{
			uint32_t *	pShaderIndex = m_psaShaderMap.GetAs( pShader ) ;
			if ( pShaderIndex == NULL )
			{
				m_nCurShaderHash =
					(uint32_t) m_psaShaderMap.GetLength() + shaderUserCustom ;
				m_psaShaderMap.Add( pShader, m_nCurShaderHash ) ;
			}
			else
			{
				m_nCurShaderHash = *pShaderIndex ;
			}
		}
		TransformationList *	pTransform = m_pTransformation ;
		if ( pTransform != NULL )
		{
			pTransform->pContextBuf = NULL ;
		}
	}
	return	sglErrSuccess ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderBuffer::GetCustomShader( void ) const
{
	return	m_optContext.pShader ;
}

// デフォルトシェーダータイプ
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer::ShaderType
	S3DRenderBuffer::GetDefaultShaderType( uint64_t nShadingMethod )
{
	if ( nShadingMethod & shadingMethodPhong )
	{
		if ( nShadingMethod & shadingMethodToon )
		{
			return	shaderDefaultPhongToon ;
		}
		else
		{
			return	shaderDefaultPhong ;
		}
	}
	else if ( nShadingMethod & shadingMethodGouraud )
	{
		return	shaderDefaultGouraud ;
	}
	return	shaderDefaultNoShade ;
}

// 輪郭線描画パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetOffsetBorderColor( uint32_t rgbBorder )
{
	if ( m_optContext.opbBorder.rgbBorder.ui32 != rgbBorder )
	{
		m_optContext.opbBorder.rgbBorder = rgbBorder ;
		//
		TransformationList *	pTransform = m_pTransformation ;
		if ( pTransform != NULL )
		{
			pTransform->pContextBuf = NULL ;
		}
	}
}

// 輪郭描画オフセット係数設定 (ax+b)
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	if ( (m_optContext.opbBorder.aThickness != a)
		| (m_optContext.opbBorder.bThickness != b) )
	{
		m_optContext.opbBorder.aThickness = a ;
		m_optContext.opbBorder.bThickness = b ;
		//
		TransformationList *	pTransform = m_pTransformation ;
		if ( pTransform != NULL )
		{
			pTransform->pContextBuf = NULL ;
		}
	}
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetOptionalFeature
	( RenderContext::FeatureType feature, int32_t nParam1,
				const void * pParam2, size_t sizeOfParam2 )
{
	if ( feature == featureContextSet )
	{
		if ( sizeOfParam2 >= sizeof(OptionalContextSet) )
		{
			const OptionalContextSet *
				pocs = (const OptionalContextSet*) pParam2 ;
			bool	flagUpdate = false ;
			if ( (pocs->nOptionMask & optionShadingFlag)
				&& (m_optContext.nShadingFlags != pocs->nShadingFlags) )
			{
				m_optContext.nShadingFlags = pocs->nShadingFlags ;
				flagUpdate = true ;
			}
			if ( (pocs->nOptionMask & optionCustomShader)
				&& (m_optContext.pShader != pocs->pShader) )
			{
				m_optContext.pShader = pocs->pShader ;
				flagUpdate = true ;
			}
			if ( pocs->nOptionMask & optionBorderParam )
			{
				m_optContext.opbBorder = pocs->opbBorder ;
				flagUpdate |=
					(m_optContext.nShadingFlags & shadingDrawOffsetBorder)
						&& !S3DRenderParameterContext::IsEqualBorderParam
								( m_optContext.opbBorder, pocs->opbBorder ) ;
			}
			if ( (pocs->nOptionMask & optionFaceCulling)
				&& (m_optContext.faceCulling != pocs->faceCulling) )
			{
				m_optContext.faceCulling = pocs->faceCulling ;
				flagUpdate = true ;
			}
			if ( (pocs->nOptionMask & optionDepthMask)
				&& (m_optContext.depthMask != pocs->depthMask) )
			{
				m_optContext.depthMask = pocs->depthMask ;
				flagUpdate = true ;
			}
			if ( (pocs->nOptionMask & optionBlendOperation)
				&& (m_optContext.depthMask != pocs->depthMask) )
			{
				m_optContext.blendOp = pocs->blendOp ;
				flagUpdate = true ;
			}
			if ( (pocs->nOptionMask & optionPointSize)
				&& (m_optContext.pointSize != pocs->pointSize) )
			{
				m_optContext.pointSize = pocs->pointSize ;
				flagUpdate = true ;
			}
			if ( (pocs->nOptionMask & optionLineWidth)
				&& (m_optContext.lineWidth != pocs->lineWidth) )
			{
				m_optContext.lineWidth = pocs->lineWidth ;
				flagUpdate = true ;
			}
			if ( flagUpdate )
			{
				FlushOptionalContextBuffer() ;
			}
			return	sglErrSuccess ;
		}
		else
		{
			return	sglErrInvalidParam ;
		}
	}
	else if ( feature == featureOrderPriority )
	{
		m_nRenderPriority = nParam1 ;
		//
		if ( m_pTransformation != NULL )
		{
			m_pTransformation->nPriority = m_nRenderPriority ;
		}
		return	sglErrSuccess ;
	}
	else if ( feature == featureOffsetBorder )
	{
		if ( sizeOfParam2 >= sizeof(OffsetBorderParam) )
		{
			const OffsetBorderParam *
					pBorder = (const OffsetBorderParam*) pParam2 ;
			SetOffsetBorderColor( pBorder->rgbBorder ) ;
			SetOffsetBorderCoefficient
					( pBorder->aThickness, pBorder->bThickness ) ;
			return	sglErrSuccess ;
		}
		else
		{
			return	sglErrInvalidParam ;
		}
	}
	else if ( feature == featureFaceCulling )
	{
		if ( m_optContext.faceCulling != nParam1 )
		{
			m_optContext.faceCulling = (FaceCullingOperation) nParam1 ;
			//
			FlushOptionalContextBuffer() ;
		}
		return	sglErrSuccess ;
	}
	else if ( feature == featureDepthMask )
	{
		if ( m_optContext.depthMask != nParam1 )
		{
			m_optContext.depthMask = (DepthMaskOperation) nParam1 ;
			//
			FlushOptionalContextBuffer() ;
		}
		return	sglErrSuccess ;
	}
	else if ( feature == featureBlendOperation )
	{
		if ( m_optContext.blendOp != nParam1 )
		{
			m_optContext.blendOp = (BlendOperation) nParam1 ;
			//
			FlushOptionalContextBuffer() ;
		}
		return	sglErrSuccess ;
	}
	else if ( feature == featurePointSize )
	{
		if ( (sizeOfParam2 == sizeof(float32_t))
			&& (m_optContext.pointSize != *((float32_t*)pParam2)) )
		{
			m_optContext.pointSize = *((float32_t*)pParam2) ;
			//
			FlushOptionalContextBuffer() ;
		}
		return	sglErrSuccess ;
	}
	else if ( feature == featureLineWidth )
	{
		if ( (sizeOfParam2 == sizeof(float32_t))
			&& (m_optContext.lineWidth != *((float32_t*)pParam2)) )
		{
			m_optContext.lineWidth = *((float32_t*)pParam2) ;
			//
			FlushOptionalContextBuffer() ;
		}
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

void S3DRenderBuffer::FlushOptionalContextBuffer( void )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		pTransform->pContextBuf = NULL ;
	}
}

// オプショナル機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::GetOptionalFeature
	( RenderContext::FeatureType feature, int32_t nParam1,
				void * pParam2, size_t sizeOfParam2 ) const
{
	if ( feature == featureOffsetBorder )
	{
		if ( sizeOfParam2 >= sizeof(OffsetBorderParam) )
		{
			OffsetBorderParam *	pBorder = (OffsetBorderParam*) pParam2 ;
			*pBorder = m_optContext.opbBorder ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featureOrderPriority )
	{
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_nRenderPriority ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featureFaceCulling )
	{
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.faceCulling ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featureDepthMask )
	{
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.depthMask ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featureBlendOperation )
	{
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.blendOp ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featurePointSize )
	{
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			*((float32_t*)pParam2) = m_optContext.pointSize ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else if ( feature == featureLineWidth )
	{
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			*((float32_t*)pParam2) = m_optContext.lineWidth ;
			return	sglErrSuccess ;
		}
		return	sglErrInvalidParam ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AppendMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	if ( m_pTransformation == NULL )
	{
		return	SetMatrixTransformation( mat, pos, color, nTransparency ) ;
	}
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pTransformation ;
	pTransform->vTransform += m_pTransformation->matTransform * pos ;
	pTransform->matTransform *= mat ;
	if ( color != NULL )
	{
		pTransform->colorEffect *= *color ;
	}
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	pTransform->nTransparency =
		0x100 - (0x100 - nTransparency)
					* (0x100 - pTransform->nTransparency) / 0x100 ;
	pTransform->pTransBuf = NULL ;
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::SetMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		pTransform = m_pGarbage ;
		if ( pTransform != NULL )
		{
			m_pGarbage = pTransform->pPrev ;
			pTransform->InitTransformation() ;
			pTransform->pPrev = NULL ;
		}
		else
		{
			pTransform = new TransformationList ;
		}
		m_pTransformation = pTransform ;
	}
	pTransform->matTransform = mat ;
	pTransform->vTransform = pos ;
	if ( color != NULL )
	{
		pTransform->colorEffect = *color ;
	}
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	pTransform->nTransparency = nTransparency ;
	pTransform->pTransBuf = NULL ;
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::GetMatrixTransformation
	( S3DDMatrix& mat, S3DDVector& pos,
		S3DColor * color, unsigned int * pTransparency ) const
{
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		mat = pTransform->matTransform ;
		pos = pTransform->vTransform ;
		if ( color != NULL )
		{
			*color = pTransform->colorEffect ;
		}
		if ( pTransparency != NULL )
		{
			*pTransparency = pTransform->nTransparency ;
		}
	}
	else
	{
		mat.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
		pos = S3DVector( 0, 0, 0 ) ;
		if ( color != NULL )
		{
			color->rgbMul.ui32 = 0x00FFFFFF ;
			color->rgbAdd.ui32 = 0 ;
		}
		if ( pTransparency != NULL )
		{
			*pTransparency = 0 ;
		}
	}
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::PushTransformation( void )
{
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pGarbage ;
	if ( pTransform != NULL )
	{
		m_pGarbage = pTransform->pPrev ;
		pTransform->InitTransformation() ;
	}
	else
	{
		pTransform = new TransformationList ;
	}
	pTransform->pPrev = m_pTransformation ;
	pTransform->optContext = m_optContext ;
	//
	if ( m_pTransformation != NULL )
	{
		m_pTransformation->optContext = m_optContext ;
		m_pTransformation->nPriority = m_nRenderPriority ;
		//
		pTransform->matTransform = m_pTransformation->matTransform ;
		pTransform->vTransform = m_pTransformation->vTransform ;
		pTransform->colorEffect = m_pTransformation->colorEffect ;
		pTransform->nTransparency = m_pTransformation->nTransparency ;
		pTransform->nPriority = m_pTransformation->nPriority ;
		pTransform->pTransBuf = m_pTransformation->pTransBuf ;
		//
		if ( m_pTransformation->pcus != NULL )
		{
			pTransform->pcus =
				new CustomUniformSet( *(m_pTransformation->pcus) ) ;
		}
		pTransform->pContextBuf = m_pTransformation->pContextBuf ;
		pTransform->pUniformBuf = m_pTransformation->pUniformBuf ;
		pTransform->nUnitofms = m_pTransformation->nUnitofms ;
		ESLAssert( (pTransform->pUniformBuf != NULL)
				|| ((pTransform->pUniformBuf == NULL)
							&& (pTransform->nUnitofms == 0)) ) ;
	}
	m_pTransformation = pTransform ;
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::PopTransformation( void )
{
	if ( m_pTransformation == NULL )
	{
		return	sglErrFailed ;
	}
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pTransformation ;
	m_pTransformation = pTransform->pPrev ;
	pTransform->pPrev = m_pGarbage ;
	m_pGarbage = pTransform ;
	//
	if ( m_pTransformation != NULL )
	{
		m_optContext = m_pTransformation->optContext ;
		m_nRenderPriority = m_pTransformation->nPriority ;
	}
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::ResetTransformation( void )
{
	TransformationList *	pLast = m_pTransformation ;
	if ( pLast != NULL )
	{
		m_csBufSync.Lock() ;
		TransformationList *	pFirst = pLast ;
		while ( pFirst->pPrev != NULL )
		{
			pFirst = pFirst->pPrev ;
		}
		ESLAssert( pFirst->pPrev == NULL ) ;
		pFirst->pPrev = m_pGarbage ;
		m_pGarbage = pLast ;
		m_pTransformation = NULL ;
		//
		m_optContext = pFirst->optContext ;
		m_nRenderPriority = pFirst->nPriority ;
		m_csBufSync.Unlock() ;
	}
	return	sglErrSuccess ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
		S3DCustomShader::UniformType type,
		const void * pData, size_t nCount )
{
	m_csBufSync.Lock() ;
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		pTransform = m_pGarbage ;
		if ( pTransform != NULL )
		{
			m_pGarbage = pTransform->pPrev ;
			pTransform->InitTransformation() ;
			pTransform->pPrev = NULL ;
		}
		else
		{
			pTransform = new TransformationList ;
		}
		m_pTransformation = pTransform ;
	}
	//
	CustomUniformSet *	pcus = pTransform->pcus ;
	if ( pcus == NULL )
	{
		pcus = new CustomUniformSet ;
		pTransform->pcus = pcus ;
	}
	CustomUniform *	pcu = pcus->GetAs( pwszUniformId ) ;
	if ( pcu == NULL )
	{
		pcu = new CustomUniform ;
		pcus->SetAs( pwszUniformId, pcu ) ;
	}
	pcu->SetData( type, pData, nCount ) ;
	//
	pTransform->pContextBuf = NULL ;
	pTransform->pUniformBuf = NULL ;
	pTransform->nUnitofms = 0 ;
	//
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DRenderBuffer::ResetCustomShaderUniform( void )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		m_csBufSync.Lock() ;
		delete	pTransform->pcus ;
		pTransform->pcus = NULL ;
		pTransform->pContextBuf = NULL ;
		pTransform->pUniformBuf = NULL ;
		pTransform->nUnitofms = 0 ;
		m_csBufSync.Unlock() ;
	}
	return	sglErrSuccess ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	return	AddIndexedPrimitiveList
				( pMaterial, nFlags, primitiveTriangle,
					countPolygon * 3, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	return	AddIndexedPrimitiveList
				( pMaterial, nFlags, primitiveTriangleStrip,
					countTriangleStrip + 2, countTriangleStrip + 2,
					pvVertex, pvNormal, pvUVMap, pColor, NULL ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( countVertex != 0 ) ;
	//
	if ( nFlags & renderFenceOrder )
	{
		S3DRenderBuffer::Flush() ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	RENDER_ENTRY *	pre =
		(RENDER_ENTRY*) m_bufRender.Allocate( sizeof(RENDER_ENTRY) ) ;
	pre->nType = typePrimitive ;
	pre->nFlags = nFlags ;
	pre->pTransform = NULL ;
	pre->pShaderContext = NULL ;
	pre->flagRenderable = true ;
	//
	if ( pIndexedList != NULL )
	{
		pre->countPrimitive =
				countIndex / GetPrimitiveVertexCount(typePrimitive) ;
	}
	else
	{
		switch ( typePrimitive )
		{
		case	primitiveTriangleStrip:
			pre->countPrimitive = countVertex - 2 ;
			break ;
		case	primitiveLineStrip:
			pre->countPrimitive = countVertex - 1 ;
			break ;
		case	primitiveTriangle:
		case	primitiveLine:
		case	primitivePoint:
		default:
			pre->countPrimitive =
					countVertex / GetPrimitiveVertexCount(typePrimitive) ;
			break ;
		}
	}
	pre->countIndex = countIndex ;
	pre->countIndexLimit = countIndex ;
	pre->countVertex = countVertex ;
	pre->pMaterial = pMaterial ? pMaterial : m_pDefaultMaterial ;
	pre->pvVertex =
		(S3DVector4*) m_bufRender.Allocate
						( countVertex * sizeof(S3DVector4) ) ;
	pre->pvNormal = NULL ;
	if ( (pvNormal != NULL) || (nFlags & renderAutoNormal) )
	{
		pre->pvNormal =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	pre->pvUVMap = NULL ;
	if ( pvUVMap != NULL )
	{
		pre->pvUVMap =
			(S2DVector*) m_bufRender.Allocate
							( countVertex * sizeof(S2DVector) ) ;
	}
	pre->pColor = NULL ;
	if ( (pColor != NULL) || (nFlags & renderAutoColor) )
	{
		pre->pColor =
			(S3DColor*) m_bufRender.Allocate
							( countVertex * sizeof(S3DColor) ) ;
	}
	pre->pvTexAxisX = NULL ;
	pre->pvTexAxisY = NULL ;
	pre->pIndexedList = NULL ;
	if ( pIndexedList != NULL )
	{
		pre->pIndexedList =
			(uint32_t*) m_bufRender.Allocate
							( countIndex * sizeof(uint32_t) ) ;
		eslMoveMemory
			( pre->pIndexedList, pIndexedList,
					countIndex * sizeof(uint32_t) ) ;
	}
	for ( int i = 0; i < countSubMesh; i ++ )
	{
		pre->pSubIndexedList[i] = pre->pIndexedList ;
		pre->nSubIndexCount[i] = (uint32_t) countIndex ;
	}
	pre->iSubMeshSelector = -1 ;
	pre->fpSubMeshDensity = 4.0f ;
	//
	pre->nExAttrElements = 0 ;
	pre->pfpExAttrElements = 0 ;
	//
	pre->nInstancingCount = 0 ;
	pre->pInstancingMatrix = NULL ;
	pre->pInstancingColor = NULL ;
	//
	if ( nFlags & renderAutoTexAxis )
	{
		pre->pvTexAxisX =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
		pre->pvTexAxisY =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	//
	pre->pvTempVertex = NULL ;
	pre->pvTempNormal = NULL ;
	pre->pvTempUVMap = NULL ;
	pre->pvTempColor = NULL ;
	pre->pvTempMorphVertex = NULL ;
	pre->pvTempMorphNormal = NULL ;
	pre->pfpTempMorphWeight = NULL ;
	pre->pvTempBoneVertex = NULL ;
	pre->pvTempBoneNormal = NULL ;
	//
	pre->countBone = 0 ;
	pre->countWeightMap = 0 ;
	pre->countFullBone = 0 ;
	pre->flagUpdateBone = false ;
	pre->countMorph = 0 ;
	pre->nTargetMeshCount = 0 ;
	pre->flagUpdateMorph = false ;
	//
	if ( OnAddRenderBuffer
		( *pre, pvVertex, pvNormal, pvUVMap, pColor ) )
	{
		m_arrRender.Add( pre ) ;
		//
		if ( nFlags & renderFenceOrder )
		{
			m_iFenceOrder = m_arrRender.GetLength() ;
		}
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

// プリミティブを追加するためのバッファを確保する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AllocatePrimitiveBuffer
	( S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	m_csBufSync.Lock() ;
	//
	if ( m_nBufCtrlFlags & bufferAutoMerge )
	{
		MergedPrimitiveBuffer&
			mpbuf = m_mpbuf[m_iPrimitiveIndex[typePrimitive]] ;
		ESLAssert( !mpbuf.m_flagAllocPrmBuf ) ;
		if ( !mpbuf.m_flagAllocPrmBuf
			&& (mpbuf.m_nVertexCount + countVertex <= mpbuf.m_nMaxVertexCount)
			&& (mpbuf.m_nIndexCount + countIndex <= mpbuf.m_nMaxIndexCount) )
		{
			prmbuf.pvVertex = mpbuf.m_bufVertex.GetArray() + mpbuf.m_nVertexCount ;
			prmbuf.pvNormal = mpbuf.m_bufNormal.GetArray() + mpbuf.m_nVertexCount ;
			prmbuf.pvUVMap = mpbuf.m_bufUVMap.GetArray() + mpbuf.m_nVertexCount ;
			prmbuf.pColor = mpbuf.m_bufColor.GetArray() + mpbuf.m_nVertexCount ;
			prmbuf.pIndexedList = mpbuf.m_bufIndex.GetArray() + mpbuf.m_nIndexCount ;
			mpbuf.m_prmbuf = prmbuf ;
			mpbuf.m_flagAllocPrmBuf = true ;
			return	sglErrSuccess ;
		}
	}
	S3DVector4 *	pvBuf =
		(S3DVector4*) m_bufRender.Allocate
			( countVertex * (sizeof(S3DVector4) * 2
							+ sizeof(S2DVector) + sizeof(S3DColor)) ) ;
	prmbuf.pvVertex = pvBuf ;
	prmbuf.pvNormal = pvBuf + countVertex ;
	prmbuf.pvUVMap = (S2DVector*) (pvBuf + countVertex * 2) ;
	prmbuf.pColor = (S3DColor*) (prmbuf.pvUVMap + countVertex) ;
	//
	if ( countIndex > 0 )
	{
		prmbuf.pIndexedList =
			(uint32_t*) m_bufRender.Allocate( countIndex * sizeof(uint32_t) ) ;
	}
	else
	{
		prmbuf.pIndexedList = NULL ;
	}
	return	sglErrSuccess ;
}

// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AddPrimitiveBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		size_t countIndex, size_t countVertex )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	MergedPrimitiveBuffer&
		mpbuf = m_mpbuf[m_iPrimitiveIndex[typePrimitive]] ;
	if ( mpbuf.m_flagAllocPrmBuf
		&& (prmbuf.pvVertex == mpbuf.m_prmbuf.pvVertex)
		&& (prmbuf.pIndexedList == mpbuf.m_prmbuf.pIndexedList) )
	{
		mpbuf.m_bufVertex.FinishArray() ;
		mpbuf.m_bufNormal.FinishArray() ;
		mpbuf.m_bufUVMap.FinishArray() ;
		mpbuf.m_bufColor.FinishArray() ;
		mpbuf.m_bufIndex.FinishArray() ;
		//
		SGLError	err = MergePrimitiveBuffer
			( mpbuf, nFlags, typePrimitive, countIndex, countVertex ) ;
		m_csBufSync.Unlock() ;
		return	err ;
	}
	else if ( (m_nBufCtrlFlags & bufferAutoMerge) && (pMaterial == NULL) )
	{
		mpbuf.m_nVertexCount += countVertex ;
		mpbuf.m_nIndexCount += countIndex ;
		//
		if ( mpbuf.m_nMaxVertexCount < mpbuf.m_nVertexCount )
		{
			mpbuf.m_nMaxVertexCount = mpbuf.m_nVertexCount ;
		}
		if ( mpbuf.m_nMaxIndexCount < mpbuf.m_nIndexCount )
		{
			mpbuf.m_nMaxIndexCount = mpbuf.m_nIndexCount ;
		}
	}
	ESLAssert( !mpbuf.m_flagAllocPrmBuf ) ;
	//
	if ( nFlags & renderFenceOrder )
	{
		S3DRenderBuffer::Flush() ;
	}
	RENDER_ENTRY *	pre =
		(RENDER_ENTRY*) m_bufRender.Allocate( sizeof(RENDER_ENTRY) ) ;
	pre->nType = typePrimitive ;
	pre->nFlags = nFlags ;
	pre->pTransform = NULL ;
	pre->pShaderContext = NULL ;
	pre->flagRenderable = true ;
	//
	if ( prmbuf.pIndexedList != NULL )
	{
		pre->countPrimitive =
				countIndex / GetPrimitiveVertexCount(typePrimitive) ;
	}
	else
	{
		switch ( typePrimitive )
		{
		case	primitiveTriangleStrip:
			pre->countPrimitive = countVertex - 2 ;
			break ;
		case	primitiveLineStrip:
			pre->countPrimitive = countVertex - 1 ;
			break ;
		case	primitiveTriangle:
		case	primitiveLine:
		case	primitivePoint:
		default:
			pre->countPrimitive =
					countVertex / GetPrimitiveVertexCount(typePrimitive) ;
			break ;
		}
	}
	pre->countIndex = countIndex ;
	pre->countIndexLimit = countIndex ;
	pre->countVertex = countVertex ;
	pre->pMaterial = pMaterial ? pMaterial : m_pDefaultMaterial ;
	pre->pvVertex = prmbuf.pvVertex ;
	pre->pvNormal = prmbuf.pvNormal ;
	pre->pvUVMap = prmbuf.pvUVMap ;
	pre->pColor = prmbuf.pColor ;
	pre->pvTexAxisX = NULL ;
	pre->pvTexAxisY = NULL ;
	pre->pIndexedList = prmbuf.pIndexedList ;
	for ( int i = 0; i < countSubMesh; i ++ )
	{
		pre->pSubIndexedList[i] = pre->pIndexedList ;
		pre->nSubIndexCount[i] = (uint32_t) countIndex ;
	}
	pre->iSubMeshSelector = -1 ;
	pre->fpSubMeshDensity = 4.0f ;
	//
	pre->nExAttrElements = 0 ;
	pre->pfpExAttrElements = 0 ;
	//
	pre->nInstancingCount = 0 ;
	pre->pInstancingMatrix = NULL ;
	pre->pInstancingColor = NULL ;
	//
	if ( nFlags & renderAutoTexAxis )
	{
		pre->pvTexAxisX =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
		pre->pvTexAxisY =
			(S3DVector4*) m_bufRender.Allocate
							( countVertex * sizeof(S3DVector4) ) ;
	}
	//
	pre->pvTempVertex = NULL ;
	pre->pvTempNormal = NULL ;
	pre->pvTempUVMap = NULL ;
	pre->pvTempColor = NULL ;
	pre->pvTempMorphVertex = NULL ;
	pre->pvTempMorphNormal = NULL ;
	pre->pfpTempMorphWeight = NULL ;
	pre->pvTempBoneVertex = NULL ;
	pre->pvTempBoneNormal = NULL ;
	//
	pre->countBone = 0 ;
	pre->countWeightMap = 0 ;
	pre->countFullBone = 0 ;
	pre->flagUpdateBone = false ;
	pre->countMorph = 0 ;
	pre->flagUpdateMorph = false ;
	//
	if ( OnAddRenderBuffer
		( *pre, prmbuf.pvVertex, prmbuf.pvNormal, prmbuf.pvUVMap, prmbuf.pColor ) )
	{
		m_arrRender.Add( pre ) ;
		//
		if ( nFlags & renderFenceOrder )
		{
			m_iFenceOrder = m_arrRender.GetLength() ;
		}
		m_csBufSync.Unlock() ;
		return	sglErrSuccess ;
	}
	else
	{
		m_csBufSync.Unlock() ;
		return	sglErrFailed ;
	}
}

// プリミティブを追加せずにバッファを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::FreePrimitiveBuffer
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	for ( int i = 0; i < indexPrimitiveCount; i ++ )
	{
		MergedPrimitiveBuffer&	mpbuf = m_mpbuf[i] ;
		if ( mpbuf.m_flagAllocPrmBuf
			&& (prmbuf.pvVertex == mpbuf.m_prmbuf.pvVertex)
			&& (prmbuf.pIndexedList == mpbuf.m_prmbuf.pIndexedList) )
		{
			mpbuf.m_flagAllocPrmBuf = false ;
			m_csBufSync.Unlock() ;
			return	sglErrSuccess ;
		}
	}
	if ( prmbuf.pIndexedList != NULL )
	{
		m_bufRender.Free( (uint8_t*) prmbuf.pIndexedList ) ;
	}
	if ( prmbuf.pvVertex != NULL )
	{
		m_bufRender.Free( (uint8_t*) prmbuf.pvVertex ) ;
	}
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	if ( nFlags & renderFenceOrder )
	{
		S3DRenderBuffer::Flush() ;
	}
	if ( iEnd < 0 )
	{
		iEnd = (ssize_t) pBuffer->GetMeshCount() ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	// エントリ作成
	//
	RENDER_ENTRY *	pre =
		(RENDER_ENTRY*) m_bufRender.Allocate( sizeof(RENDER_ENTRY) ) ;
	pre->nType = typeVertexBuffer ;
	pre->nFlags = nFlags ;
	pre->pTransform = NULL ;
	pre->pShaderContext = NULL ;
	pre->pMaterial = pMaterial ;
	pre->pVertexBuffer = pBuffer ;
	pre->iFirstBuf = iFirst ;
	pre->iEndBuf = iEnd ;
	pre->flagRenderable = true ;
	pre->pvVertex = NULL ;
	pre->pvNormal = NULL ;
	pre->pvUVMap = NULL ;
	pre->pColor = NULL ;
	pre->pvTexAxisX = NULL ;
	pre->pvTexAxisY = NULL ;
	pre->pIndexedList = NULL ;
	for ( int i = 0; i < countSubMesh; i ++ )
	{
		pre->pSubIndexedList[i] = NULL ;
		pre->nSubIndexCount[i] = 0 ;
	}
	pre->iSubMeshSelector = -1 ;
	pre->fpSubMeshDensity = 4.0f ;
	//
	pre->nExAttrElements = 0 ;
	pre->pfpExAttrElements = 0 ;
	//
	pre->nInstancingCount = nInstancing ;
	pre->pInstancingMatrix = NULL ;
	pre->pInstancingColor = NULL ;
	if ( nInstancing > 0 )
	{
		if ( pmatInstancing != NULL )
		{
			pre->pInstancingMatrix =
				(S4DMatrix*) m_bufRender.Allocate
								( nInstancing * sizeof(S4DMatrix) ) ;
			eslCopyMemory
				( pre->pInstancingMatrix,
					pmatInstancing, nInstancing * sizeof(S4DMatrix) ) ;
		}
		if ( pColorInstancing != NULL )
		{
			pre->pInstancingColor =
				(S3DColor*) m_bufRender.Allocate
								( nInstancing * sizeof(S3DColor) ) ;
			eslCopyMemory
				( pre->pInstancingColor,
					pColorInstancing, nInstancing * sizeof(S3DColor) ) ;
		}
	}
	//
	pre->pvTempVertex = NULL ;
	pre->pvTempNormal = NULL ;
	pre->pvTempUVMap = NULL ;
	pre->pvTempColor = NULL ;
	pre->pvTempMorphVertex = NULL ;
	pre->pvTempMorphNormal = NULL ;
	pre->pfpTempMorphWeight = NULL ;
	pre->pvTempBoneVertex = NULL ;
	pre->pvTempBoneNormal = NULL ;
	pre->countBone = 0 ;
	pre->countWeightMap = 0 ;
	pre->countFullBone = 0 ;
	pre->flagUpdateBone = false ;
	pre->countMorph = 0 ;
	pre->flagUpdateMorph = false ;
	//
	SetEntryTransformation( *pre ) ;
	//
	// ｚソート用領域判定
	//
	MeshInfo	mshinf ;
	eslFillMemory( &mshinf, 0, sizeof(MeshInfo) ) ;
	pBuffer->GetMeshInfoAt( mshinf, iFirst, 0 ) ;
	//
	if ( iFirst + 1 < (size_t) iEnd )
	{
		S3DVector	vCenter = mshinf.vCenter ;
		float32_t	fpRadius = mshinf.fpRadius ;
		for ( size_t i = iFirst + 1; i < (size_t) iEnd; i ++ )
		{
			pBuffer->GetMeshInfoAt( mshinf, i, 0 ) ;
			//
			S3DVector	vDelta = mshinf.vCenter ;
			vDelta -= vCenter ;
			double	d = vDelta.Absolute() ;
			float32_t	minMesh = (float32_t) d - mshinf.fpRadius ;
			float32_t	maxMesh = (float32_t) d + mshinf.fpRadius ;
			if ( minMesh > - fpRadius )
			{
				minMesh = - fpRadius ;
			}
			if ( maxMesh < fpRadius )
			{
				maxMesh = fpRadius ;
			}
			vCenter += vDelta * ((minMesh + maxMesh) * 0.5 / d) ;
			fpRadius = (maxMesh - minMesh) * 0.5f ;
		}
		mshinf.vCenter = vCenter ;
		mshinf.fpRadius = fpRadius ;
	}
	float32_t	zFloatSort = 0.0 ;
	S3DDVector	vPos = mshinf.vCenter ;
	if ( pre->pTransform != NULL )
	{
		vPos += pre->pTransform->vTransform ;
	}
	zFloatSort = (float32_t) (m_matCamera * vPos).z - mshinf.fpRadius ;
	//
	// ｚソート用ハッシュ計算
	//
	MakeSoftHash( *pre, zFloatSort ) ;
	//
	// 追加
	//
	m_arrRender.Add( pre ) ;
	m_flagCircumscribed = false ;
	//
	if ( nFlags & renderFenceOrder )
	{
		m_iFenceOrder = m_arrRender.GetLength() ;
	}
	return	sglErrSuccess ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::Flush( void )
{
	m_csBufSync.Lock() ;
	const size_t	countRender = m_arrRender.GetLength() ;
	if ( m_flagSorting )
	{
		RENDER_ENTRY**	ppRender = m_arrRender.GetArray() ;
		for ( size_t i = m_iFenceOrder; i < countRender; i ++ )
		{
			ppRender[i]->nFlags |= renderFenceOrder ;
		}
		if ( (countRender - m_iFenceOrder) >= 2 )
		{
			OnSortRenderBuffer
				( ppRender + m_iFenceOrder, countRender - m_iFenceOrder ) ;
		}
		m_arrRender.FinishArray() ;
	}
	m_iFenceOrder = countRender ;
	//
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnFlush( this ) ;
	}
	m_csBufSync.Unlock() ;
	return	sglErrSuccess ;
}

// 遅延削除オブジェクト追加（Flush 時に削除）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::AddTemporaryObject( ESLObject * pObj )
{
	m_csBufSync.Lock() ;
	m_arrayTemporary.Add( pObj ) ;
	m_csBufSync.Unlock() ;
}

// バッファ制御フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DRenderBuffer::GetBufferControlFlags( void ) const
{
	return	m_nBufCtrlFlags ;
}

void S3DRenderBuffer::SetBufferControlFlags( uint32_t nFlags )
{
	m_nBufCtrlFlags = nFlags ;
}

// デフォルトマテリアル
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DRenderBuffer::GetDefaultMaterial( void ) const
{
	return	m_pDefaultMaterial ;
}

void S3DRenderBuffer::AttachDefaultMaterial( S3DMaterial * pMaterial )
{
	m_pDefaultMaterial = pMaterial ;
}

// メッシュ数を取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderBuffer::GetMeshCount( void ) const
{
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	return	m_arrRender.GetLength() - m_iShiftOffset ;
}

// メッシュ情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::GetMeshInfoAt
	( S3DVertexBufferInterface::MeshInfo& info, size_t iMesh,
		size_t nCopyVertices, size_t iFirstVertex, uint32_t nFlags ) const
{
	RENDER_ENTRY *	pre = GetMeshEntryAt( iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	info.pMaterial = pre->pMaterial ;
	if ( pre->nType != typeVertexBuffer )
	{
		info.typeMesh = (S3DPrimitiveType) pre->nType ;
	}
	else
	{
		return	sglErrFailed ;
	}
	info.countPrimitive = (uint32_t) pre->countPrimitive ;
	info.countVertex = (uint32_t) pre->countVertex ;
	info.vCenter = pre->vCenter ;
	info.fpRadius = pre->fpRadius ;
	info.iSubMeshSelector = pre->iSubMeshSelector ;
	info.fpSubMeshDensity = pre->fpSubMeshDensity ;
	info.nExAttrElements = pre->nExAttrElements ;
	//
	if ( nCopyVertices == 0 )
	{
		return	sglErrSuccess ;
	}
	if ( iFirstVertex >= pre->countVertex )
	{
		return	sglErrInvalidParam ;
	}
	if ( iFirstVertex + nCopyVertices > pre->countVertex )
	{
		nCopyVertices = pre->countVertex - iFirstVertex ;
	}
	if ( info.pvVertex != NULL )
	{
		if ( pre->pvVertex != NULL )
		{
			eslCopyMemory
				( info.pvVertex,
					pre->pvVertex + iFirstVertex,
					nCopyVertices * sizeof(S3DVector4) ) ;
		}
		else
		{
			info.pvVertex = NULL ;
		}
	}
	if ( info.pvNormal != NULL )
	{
		if ( pre->pvNormal != NULL )
		{
			eslCopyMemory
				( info.pvNormal,
					pre->pvNormal + iFirstVertex,
					nCopyVertices * sizeof(S3DVector4) ) ;
		}
		else
		{
			info.pvNormal = NULL ;
		}
	}
	if ( info.pvUVMap != NULL )
	{
		if ( pre->pvUVMap != NULL )
		{
			eslCopyMemory
				( info.pvUVMap,
					pre->pvUVMap + iFirstVertex,
					nCopyVertices * sizeof(S2DVector) ) ;
		}
		else
		{
			info.pvUVMap = NULL ;
		}
	}
	if ( info.pColor != NULL )
	{
		if ( pre->pColor != NULL )
		{
			eslCopyMemory
				( info.pColor,
					pre->pColor + iFirstVertex,
					nCopyVertices * sizeof(S3DColor) ) ;
		}
		else
		{
			info.pColor = NULL ;
		}
	}
	if ( nFlags & flagMeshMorphedVertex )
	{
		((S3DRenderBuffer*)this)->
			MorphMeshInfoVertics
				( info, *pre, iFirstVertex, nCopyVertices ) ;
	}
	if ( nFlags & flagMeshBoneTransformed )
	{
		((S3DRenderBuffer*)this)->
			TransformMeshVerticsByBone
				( info, *pre, iFirstVertex, nCopyVertices ) ;
	}
	if ( info.pIndexedList != NULL )
	{
		if ( (pre->nType == typeIndexedTriangleList)
						&& (pre->pIndexedList != NULL) )
		{
			eslCopyMemory
				( info.pIndexedList, pre->pIndexedList,
					pre->countIndex * sizeof(uint32_t) ) ;
		}
		else
		{
			info.pIndexedList = NULL ;
		}
	}
	for ( int i = 0; i < countSubMesh; i ++ )
	{
		if ( (info.pSubIndexedList[i] != NULL)
			&& (pre->pSubIndexedList[i] != NULL) )
		{
			eslCopyMemory
				( info.pSubIndexedList[i], pre->pSubIndexedList[i],
					pre->nSubIndexCount[i] * sizeof(uint32_t) ) ;
		}
		else
		{
			info.pSubIndexedList[i] = NULL ;
		}
		pre->nSubIndexCount[i] = pre->nSubIndexCount[i] ;
	}
	if ( info.pfpExAttrElements != NULL )
	{
		if ( pre->pfpExAttrElements != NULL )
		{
			const size_t	nElementBytes = pre->nExAttrElements * sizeof(float32_t) ;
			eslCopyMemory
				( info.pfpExAttrElements,
					pre->pfpExAttrElements + iFirstVertex * pre->nExAttrElements,
					nCopyVertices * pre->nExAttrElements * sizeof(float32_t) ) ;
		}
		else
		{
			info.pfpExAttrElements = NULL ;
		}
	}
	return	sglErrSuccess ;
}

// ポリゴンリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::UpdateIndexedTriangleList
	( size_t iMesh, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	return	UpdateIndexedPrimitiveList
		( iMesh, nFlags,
			countPolygon * 3, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::UpdateTriangleStrip
	( size_t iMesh, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	return	UpdateIndexedPrimitiveList
		( iMesh, nFlags,
			countTriangleStrip * 3,
			countTriangleStrip + 2,
			pvVertex, pvNormal, pvUVMap, pColor, NULL ) ;
}

// プリミティブリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::UpdateIndexedPrimitiveList
	( size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (pre->countIndexLimit < countIndex)
		| (pre->countVertex != countVertex) )
	{
		return	sglErrFailed ;
	}
	if ( ((pvVertex != NULL) & (pre->pvVertex == NULL))
		| ((pvNormal != NULL) & (pre->pvNormal == NULL))
		| ((pvUVMap != NULL) & (pre->pvUVMap == NULL))
		| ((pColor != NULL) & (pre->pColor == NULL)) )
	{
		return	sglErrFailed ;
	}
	if ( pColor == NULL )
	{
		pColor = pre->pColor ;
	}
	if ( pIndexedList != NULL )
	{
		if ( pre->pIndexedList == NULL )
		{
			return	sglErrFailed ;
		}
		eslMoveMemory
			( pre->pIndexedList, pIndexedList,
					countIndex * sizeof(uint32_t) ) ;
		pre->countIndex = countIndex ;
	}
	if ( (pvVertex != NULL) | (pvNormal != NULL)
			| (pvUVMap != NULL) | (pColor != NULL) )
	{
		UpdateRenderBuffer( *pre, pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	//
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnUpdateIndexedPrimitiveList
			( this, iMesh, nFlags, countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
	return	sglErrSuccess ;
}

// サブメッシュ（ポリゴンリスト）を更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::UpdateSubIndexedTriangleList
	( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
		size_t countPolygon, const uint32_t * pIndexedList )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( iSubMesh >= countSubMesh )
	{
		return	sglErrFailed ;
	}
	size_t	countIndex = countPolygon * 3 ;
	if ( (pre->pSubIndexedList[iSubMesh] == NULL)
		|| (pre->pSubIndexedList[iSubMesh] == pre->pIndexedList) )
	{
		pre->pSubIndexedList[iSubMesh] = 
			(uint32_t*) m_bufRender.Allocate
							( countIndex * sizeof(uint32_t) ) ;
	}
	else if ( countIndex > pre->nSubIndexCount[iSubMesh] )
	{
		return	sglErrFailed ;
	}
	pre->nSubIndexCount[iSubMesh] = (uint32_t) countIndex ;
	//
	eslMoveMemory
		( pre->pSubIndexedList[iSubMesh], pIndexedList,
						countIndex * sizeof(uint32_t) ) ;
	//
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnUpdateSubIndexedTriangleList
			( this, iMesh, iSubMesh, nFlags, countPolygon, pIndexedList ) ;
	}
	return	sglErrSuccess ;
}

// サブメッシュ切り替えｚ座標比を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetSubMeshDensity
	( size_t iMesh, float32_t fpDensity, ssize_t iSelector )
{
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	pre->iSubMeshSelector = iSelector ;
	pre->fpSubMeshDensity = fpDensity ;
	return	sglErrSuccess ;
}

// 追加的な頂点属性を設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetExtendVertexAttribute
	( size_t iMesh, size_t countElements,
		size_t countVertex, const float32_t * pfpAttrElements )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pre->countVertex != countVertex )
	{
		return	sglErrFailed ;
	}
	if ( pre->pfpExAttrElements != NULL )
	{
		if ( pre->nExAttrElements < countElements )
		{
			return	sglErrFailed ;
		}
		pre->nExAttrElements = countElements ;
	}
	else
	{
		pre->pfpExAttrElements =
			(float32_t*) m_bufRender.Allocate
							( countVertex * countElements * sizeof(float32_t) ) ;
		pre->nExAttrElements = countElements ;
	}
	eslCopyMemory
		( pre->pfpExAttrElements,
			pfpAttrElements,
			countVertex * countElements * sizeof(float32_t) ) ;
	//
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnSetExtendVertexAttribute
			( this, iMesh, countElements, countVertex, pfpAttrElements ) ;
	}
	return	sglErrSuccess ;
}

// メッシュにウェイトマップを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetBoneWeightMap
	( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pre->countWeightMap == 0 )
	{
		float32_t *	pWeightMapBuf =
			(float32_t*)
				m_bufRender.Allocate
					( pre->countVertex * nCount * sizeof(float32_t) ) ;
		//
		pre->countWeightMap = nCount ;
		pre->flagUpdateBone = true ;
		//
		pre->ppWeightMap =
			(float32_t**) m_bufRender.Allocate( nCount * sizeof(float32_t*) ) ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pre->ppWeightMap[i] = pWeightMapBuf + (i * pre->countVertex) ;
		}
		//
		if ( pre->countBone == 0 )
		{
			pre->countBone = nCount ;
			pre->ppJointMap = NULL ;
			pre->pBoneMatrix = NULL ;
			pre->pBoneTrans = NULL ;
		}
	}
	else if ( pre->countWeightMap != nCount )
	{
		return	sglErrFailed ;
	}
	bool	flagCountFull = true ;
	pre->countFullBone = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		eslMoveMemory
			( pre->ppWeightMap[i], ppWeightMaps[i],
					pre->countVertex * sizeof(float32_t) ) ;
		if ( flagCountFull )
		{
			const float32_t *	pWeightMap = ppWeightMaps[i] ;
			const size_t		nCount = pre->countVertex ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				if ( pWeightMap[i] < 0.999999 )
				{
					flagCountFull = false ;
					break ;
				}
			}
			if ( flagCountFull )
			{
				pre->countFullBone ++ ;
			}
		}
	}
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnSetBoneWeightMap( this, iMesh, nCount, ppWeightMaps ) ;
	}
	return	sglErrSuccess ;
}

// メッシュにジョイントマップを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetBoneJointMap
	( size_t iMesh, size_t nBoneCount,
				size_t nJointCount, const uint32_t ** ppJointMaps )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pre->countBone == 0 )
	{
		pre->countWeightMap = 0 ;
		pre->pBoneMatrix = NULL ;
		pre->pBoneTrans = NULL ;
	}
	else if ( pre->ppJointMap != NULL )
	{
		return	sglErrFailed ;
	}
	float32_t *	pWeightMapBuf =
		(float32_t*)
			m_bufRender.Allocate
				( pre->countVertex * nJointCount * sizeof(float32_t) ) ;
	uint32_t *	pJointMapBuf =
		(uint32_t*)
			m_bufRender.Allocate
				( pre->countVertex * nJointCount * sizeof(uint32_t) ) ;
	//
	pre->countBone = nBoneCount ;
	pre->countWeightMap = nJointCount ;
	pre->flagUpdateBone = true ;
	//
	pre->ppWeightMap =
		(float32_t**) m_bufRender.Allocate
				( nJointCount * sizeof(float32_t*) ) ;
	pre->ppJointMap =
		(uint32_t**) m_bufRender.Allocate
				( nJointCount * sizeof(uint32_t*) ) ;
	//
	for ( size_t i = 0; i < nJointCount; i ++ )
	{
		pre->ppWeightMap[i] = pWeightMapBuf + (i * pre->countVertex) ;
		pre->ppJointMap[i] = pJointMapBuf + (i * pre->countVertex) ;
	}
	//
	ESLAssert( pre->ppJointMap != NULL ) ;
	for ( size_t i = 0; i < nJointCount; i ++ )
	{
		eslCopyMemory
			( pre->ppJointMap[i],
				ppJointMaps[i],
				pre->countVertex * sizeof(uint32_t) ) ;
	}
	return	sglErrSuccess ;
}

// メッシュにボーン行列設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetBoneMatrix
	( size_t iMesh, size_t nCount,
		const S3DMatrix * pMatrix, const S3DVector * pTrans )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pre->countBone != nCount )
	{
		return	sglErrFailed ;
	}
	if ( pre->pBoneMatrix == NULL )
	{
		pre->pBoneMatrix =
			(S3DMatrix*) m_bufRender.Allocate
							( pre->countBone * sizeof(S3DMatrix) ) ;
	}
	if ( pre->pBoneTrans == NULL )
	{
		pre->pBoneTrans =
			(S3DVector*) m_bufRender.Allocate
							( pre->countBone * sizeof(S3DVector) ) ;
	}
	pre->flagUpdateBone = true ;
	eslMoveMemory
		( pre->pBoneMatrix, pMatrix, nCount * sizeof(S3DMatrix) ) ;
	eslMoveMemory
		( pre->pBoneTrans, pTrans, nCount * sizeof(S3DVector) ) ;
	return	sglErrSuccess ;
}

// メッシュのボーン行列取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderBuffer::GetBoneMatrix
	( size_t iMesh, size_t nCount, S3DMatrix * pMatrix, S3DVector * pTrans )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	0 ;
	}
	if ( nCount == 0 )
	{
		return	pre->countBone ;
	}
	if ( nCount >= pre->countBone )
	{
		nCount = pre->countBone ;
	}
	eslMoveMemory
		( pMatrix, pre->pBoneMatrix, nCount * sizeof(S3DMatrix) ) ;
	eslMoveMemory
		( pTrans, pre->pBoneTrans, nCount * sizeof(S3DVector) ) ;
	return	nCount ;
}

// メッシュにモーフターゲット枠を確保
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::AllocateMorphing( size_t iMesh, size_t nCount )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pre->countMorph == 0 )
	{
		pre->countMorph = nCount ;
		pre->flagUpdateMorph = false ;
		pre->flagMorphWithWeight = false ;
		pre->pMorphTargetMesh =
			(ssize_t*) m_bufRender.Allocate
							( (nCount + 1) * sizeof(ssize_t) ) ;
		pre->pMorphApplication =
			(float32_t*) m_bufRender.Allocate
							( (nCount + 1) * sizeof(float32_t) ) ;
		pre->nTargetMeshCount = 0 ;
		//
		size_t	nMorphBufLen = pre->countVertex * nCount ;
		pre->pvMorphVertex =
			(S3DVector4*) m_bufRender.Allocate
								( nMorphBufLen * sizeof(S3DVector4) ) ;
		pre->pvMorphNormal =
			(S3DVector4*) m_bufRender.Allocate
								( nMorphBufLen * sizeof(S3DVector4) ) ;
		pre->pvMorphUVMap =
			(S2DVector*) m_bufRender.Allocate
								( nMorphBufLen * sizeof(S2DVector) ) ;
		pre->pMorphColor =
			(S3DColor*) m_bufRender.Allocate
								( nMorphBufLen * sizeof(S3DColor) ) ;
		pre->pvMorphTexAxisX = NULL ;
		pre->pvMorphTexAxisY = NULL ;
		if ( pre->nFlags & renderAutoTexAxis )
		{
			pre->pvMorphTexAxisX =
				(S3DVector4*) m_bufRender.Allocate
									( nMorphBufLen * sizeof(S3DVector4) ) ;
			pre->pvMorphTexAxisY =
				(S3DVector4*) m_bufRender.Allocate
									( nMorphBufLen * sizeof(S3DVector4) ) ;
		}
		pre->pbMorphWeight =
				(bool*) m_bufRender.Allocate( nCount * sizeof(bool) ) ;
		pre->pfpMorphWeight =
				(float32_t*) m_bufRender.Allocate
									( nMorphBufLen * sizeof(float32_t) ) ;
		eslFillMemory( pre->pbMorphWeight, 0, nCount * sizeof(bool) ) ;
		eslFillMemory
			( pre->pfpMorphWeight, 0, nMorphBufLen * sizeof(float32_t) ) ;
		//
		if ( m_pFirstDevBuf != NULL )
		{
			m_pFirstDevBuf->OnAllocateMorphing( this, iMesh, nCount ) ;
		}
	}
	else if ( pre->countMorph < nCount )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// メッシュにモーフターゲットを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetMorphingTargetMesh
	( size_t iMesh, size_t iMorph, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( iMorph >= pre->countMorph )
	{
		return	sglErrFailed ;
	}
	size_t	iFirst = iMorph * pre->countVertex ;
	if ( countVertex > pre->countVertex )
	{
		countVertex = pre->countVertex ;
	}
	if ( pvVertex == NULL )
	{
		pvVertex = pre->pvVertex ;
	}
	if ( pvNormal == NULL )
	{
		pvNormal = pre->pvNormal ;
	}
	if ( pvUVMap == NULL )
	{
		pvUVMap = pre->pvUVMap ;
	}
	if ( pColor == NULL )
	{
		pColor = pre->pColor ;
	}
	//
	S3DVector	vMax, vMin ;
	MoveVertexAndCubeRange
		( vMax, vMin, pre->pvMorphVertex + iFirst, pvVertex, countVertex ) ;
	//
	S3DVector	vCenter = (vMax + vMin) * 0.5f ;
	float32_t	fpRadius = (float32_t) (vMax - vMin).Absolute() * 0.5f ;
	//
	S3DVector	vDelta = pre->vCenter - vCenter ;
	double		d = vDelta.Absolute() ;
	float32_t	minMesh = (float32_t) d - pre->fpRadius ;
	float32_t	maxMesh = (float32_t) d + pre->fpRadius ;
	if ( minMesh > - fpRadius )
	{
		minMesh = (float32_t) - fpRadius ;
	}
	if ( maxMesh < fpRadius )
	{
		maxMesh = (float32_t) fpRadius ;
	}
	if ( d > 0.0 )
	{
		vCenter += vDelta * ((minMesh + maxMesh) * 0.5 / d) ;
	}
	fpRadius = (maxMesh - minMesh) * 0.5f ;
	//
	pre->vVertexMax.x = esl_fmaxf( pre->vVertexMax.x, vMax.x ) ;
	pre->vVertexMax.y = esl_fmaxf( pre->vVertexMax.y, vMax.y ) ;
	pre->vVertexMax.z = esl_fmaxf( pre->vVertexMax.z, vMax.z ) ;
	pre->vVertexMin.x = esl_fminf( pre->vVertexMin.x, vMax.x ) ;
	pre->vVertexMin.y = esl_fminf( pre->vVertexMin.y, vMax.y ) ;
	pre->vVertexMin.z = esl_fminf( pre->vVertexMin.z, vMax.z ) ;
	pre->vCenter = vCenter ;
	pre->fpRadius = fpRadius ;
	//
	if ( pvNormal != NULL )
	{
		eslMoveMemory
			( pre->pvMorphNormal + iFirst,
				pvNormal, countVertex * sizeof(S3DVector4) ) ;
	}
	if ( pvUVMap != NULL )
	{
		eslMoveMemory
			( pre->pvMorphUVMap + iFirst,
				pvUVMap, countVertex * sizeof(S2DVector) ) ;
	}
	else
	{
		eslFillMemory
			( pre->pvMorphUVMap + iFirst,
				0, countVertex * sizeof(S2DVector) ) ;
	}
	if ( pColor != NULL )
	{
		eslMoveMemory
			( pre->pMorphColor + iFirst,
				pColor, countVertex * sizeof(S3DColor) ) ;
	}
	else
	{
		S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
		S3DColor *	pDstMorphColor = pre->pMorphColor ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pDstMorphColor[i] = clrDummy ;
		}
	}
	if ( (pre->nFlags & renderAutoTexAxis) && (pre->pvMorphUVMap != NULL) )
	{
		if ( m_bufTempTexAxis.SetForIndexedPrimitiveList
				( (S3DPrimitiveType) pre->nType,
					pre->countPrimitive, pre->countVertex,
					pre->pvMorphVertex + iFirst,
					pre->pvMorphUVMap + iFirst, pre->pIndexedList ) )
		{
			eslMoveMemory
				( pre->pvMorphTexAxisX + iFirst,
					m_bufTempTexAxis.GetBufferAxisX(),
					pre->countPrimitive * sizeof(S3DVector4) ) ;
			eslMoveMemory
				( pre->pvMorphTexAxisY + iFirst,
					m_bufTempTexAxis.GetBufferAxisY(),
					pre->countPrimitive * sizeof(S3DVector4) ) ;
		}
	}
	if ( pre->pfpMorphWeight != NULL )
	{
		float32_t *	pfpWeight = pre->pfpMorphWeight + iFirst ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pfpWeight[i] = 1.0f ;
		}
	}
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnSetMorphingTargetMesh
			( this, iMesh, iMorph, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	return	sglErrSuccess ;
}

// メッシュのモーフターゲットにウェイトを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetMorphingTargetWeight
	( size_t iMesh, size_t iMorph,
		size_t countVertex, const float32_t * pfpWeight )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( iMorph >= pre->countMorph )
	{
		return	sglErrFailed ;
	}
	size_t	iFirst = iMorph * pre->countVertex ;
	if ( countVertex > pre->countVertex )
	{
		countVertex = pre->countVertex ;
	}
	if ( pre->pfpMorphWeight != NULL )
	{
		pre->pbMorphWeight[iMorph] = false ;
		if ( pfpWeight != NULL )
		{
			float32_t *	pfpDstWeight = pre->pfpMorphWeight + iFirst ;
			bool		fWeight = false ;
			for ( size_t i = 0; i < countVertex; i ++ )
			{
				pfpDstWeight[i] = pfpWeight[i] ;
				if ( fabs( pfpWeight[i] - 1.0f ) < 1.0e-8 )
				{
					fWeight = true ;
				}
			}
			pre->pbMorphWeight[iMorph] = fWeight ;
		}
		else
		{
			pre->pbMorphWeight[iMorph] = false ;
		}
	}
	return	sglErrSuccess ;
}

// モーフィング設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetMorphingApplication
	( size_t iMesh, const ssize_t * pTargetMesh,
			const float32_t * pApplication, size_t nTargetMeshCount )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	ESLAssert( (ssize_t) nTargetMeshCount >= 0 ) ;
	if ( pre->countMorph + 1 < nTargetMeshCount )
	{
		return	sglErrFailed ;
	}
	bool		flagUpdate = (pre->nTargetMeshCount != nTargetMeshCount) ;
	bool		flagMorphWithWeight = false ;
	ssize_t *	pMorphTargetMesh = pre->pMorphTargetMesh ;
	float32_t *	pMorphApplication = pre->pMorphApplication ;
	float32_t	fpTotalApp = 0.0f ;
	ssize_t		iDefaultRef = -1 ;
	for ( size_t i = 0; i < nTargetMeshCount; i ++ )
	{
		if ( (pTargetMesh[i] >= 0)
			&& ((size_t) pTargetMesh[i] >= pre->countMorph) )
		{
			return	sglErrFailed ;
		}
		flagUpdate |= (pMorphTargetMesh[i] != pTargetMesh[i])
					| (pMorphApplication[i] != pApplication[i]) ;
		pMorphTargetMesh[i] = pTargetMesh[i] ;
		pMorphApplication[i] = pApplication[i] ;
		fpTotalApp += pApplication[i] ;
		if ( pTargetMesh[i] >= 0 )
		{
			flagMorphWithWeight |= pre->pbMorphWeight[ pTargetMesh[i] ] ;
		}
		else
		{
			iDefaultRef = (ssize_t) i ;
		}
	}
	if ( !flagMorphWithWeight )
	{
		if ( iDefaultRef < 0 )
		{
			if ( nTargetMeshCount <= pre->countMorph )
			{
				pMorphTargetMesh[nTargetMeshCount] = -1 ;
				pMorphApplication[nTargetMeshCount] = 1.0f - fpTotalApp ;
				nTargetMeshCount ++ ;
			}
		}
		else
		{
			pMorphApplication[iDefaultRef] += 1.0f - fpTotalApp ;
		}
	}
	pre->nTargetMeshCount = nTargetMeshCount ;
	pre->flagUpdateMorph = flagUpdate ;
	pre->flagMorphWithWeight = flagMorphWithWeight ;
	return	sglErrSuccess ;
}

// モーフィング設定取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::GetMorphingApplication
	( size_t iMesh, ssize_t& iTargetMesh,
			float32_t& fpApplication, size_t iTargetMeshIndex )
{
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	if ( iTargetMeshIndex >= pre->nTargetMeshCount )
	{
		return	sglErrFailed ;
	}
	iTargetMesh = pre->pMorphTargetMesh[iTargetMeshIndex] ;
	fpApplication = pre->pMorphApplication[iTargetMeshIndex] ;
	return	sglErrSuccess ;
}

// メッシュ表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::EnableToRenderMesh
	( size_t iFirst, ssize_t iEnd, bool fEnable )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	if ( iEnd < 0 )
	{
		ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
		iEnd = (ssize_t) (m_arrRender.GetLength() - m_iShiftOffset) ;
	}
	for ( size_t i = iFirst; i < (size_t) iEnd; i ++ )
	{
		RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + i ) ;
		if ( pre == NULL )
		{
			return	sglErrFailed ;
		}
		pre->flagRenderable = fEnable ;
	}
	return	sglErrSuccess ;
}

// メッシュ表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::IsEnabledToRenderMesh( size_t iMesh ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	false ;
	}
	return	pre->flagRenderable ;
}

// メッシュマテリアル設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::SetMaterialToRenderMesh
	( size_t iMesh, S3DMaterial * pMaterial )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	sglErrFailed ;
	}
	pre->pMaterial = pMaterial ;
	return	sglErrSuccess ;
}

// メッシュマテリアル取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DRenderBuffer::GetMaterialToRenderMesh( size_t iMesh ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csBufSync ) ;
	RENDER_ENTRY *	pre = m_arrRender.GetAt( m_iShiftOffset + iMesh ) ;
	if ( pre == NULL )
	{
		return	NULL ;
	}
	return	pre->pMaterial ;
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
		uint64_t flagsExclusion, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csBufSync ) ;
	return	RenderTemporaryBufferTo
				( render, flagsExclusion, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// バッファを消去
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::ClearBuffer( void )
{
	for ( ; ; )
	{
		m_signalFreeRefMesh.Wait() ;
		m_csBufSync.Lock() ;
		if ( m_nRefShiftMesh == 0 )
		{
			break ;
		}
		m_csBufSync.Unlock() ;
	}

	for ( int i = 0; i < indexPrimitiveCount; i ++ )
	{
		MergedPrimitiveBuffer&	mpbuf = m_mpbuf[i] ;
		mpbuf.m_pMaterial = NULL ;
		mpbuf.m_preRel = NULL ;
		mpbuf.m_flagAllocPrmBuf = false ;
		mpbuf.m_nVertexCount = 0 ;
		mpbuf.m_nIndexCount = 0 ;
		mpbuf.m_nMaxVertexCount = (mpbuf.m_nMaxVertexCount + 0xFF) & ~0xFF ;
		mpbuf.m_nMaxIndexCount = (mpbuf.m_nMaxIndexCount + 0x3FF) & ~0x3FF ;
		mpbuf.m_bufVertex.SetLength( mpbuf.m_nMaxVertexCount ) ;
		mpbuf.m_bufNormal.SetLength( mpbuf.m_nMaxVertexCount ) ;
		mpbuf.m_bufUVMap.SetLength( mpbuf.m_nMaxVertexCount ) ;
		mpbuf.m_bufColor.SetLength( mpbuf.m_nMaxVertexCount ) ;
		mpbuf.m_bufIndex.SetLength( mpbuf.m_nMaxIndexCount ) ;
	}
	//
	m_arrRender.SetLength( 0 ) ;
	m_iShiftOffset = 0 ;
	m_iFenceOrder = 0 ;
	m_bufRender.FreeAll() ;
	m_pLastShaderContext = NULL ;
	//
	m_arrayTemporary.RemoveAll() ;
	//
	TransformationList *	pTrans = m_pTransformation ;
	while ( pTrans != NULL )
	{
		pTrans->pTransBuf = NULL ;
		pTrans = pTrans->pPrev ;
	}
	//
	m_flagCircumscribed = false ;
	//
	if ( m_pFirstDevBuf != NULL )
	{
		m_pFirstDevBuf->OnClearBuffer( this ) ;
	}
	m_csBufSync.Unlock() ;
}

// デバイスリソースを解放
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::ReleaseAllDeviceResources( void )
{
	m_csBufSync.Lock() ;
	if ( m_pFirstDevBuf != NULL )
	{
		S3DVertexDeviceBufferInterface *	pDevBuf = m_pFirstDevBuf ;
		m_pFirstDevBuf = NULL ;
		m_csBufSync.Unlock() ;
		//
		delete	pDevBuf ;
	}
	else
	{
		m_csBufSync.Unlock() ;
	}
}

// バッファのメモリブロックサイズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetBufferUnitSize( size_t nBytes )
{
	m_bufRender.SetBlockSize( nBytes ) ;
}

// メッシュ外接球取得
//////////////////////////////////////////////////////////////////////////////
double S3DRenderBuffer::GetCircumscribedSphere( S3DVector& vSphereCenter )
{
	S3DVector	vMin, vMax ;
	GetCircumscribedParallelepiped( vMin, vMax ) ;
	vSphereCenter = (vMin + vMax) * 0.5f ;
	return	(vMax - vMin).Absolute() ;
}

// メッシュ外接直方体取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::GetCircumscribedParallelepiped
					( S3DVector& vMin, S3DVector& vMax )
{
	if ( !m_flagCircumscribed )
	{
		SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
		ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
		S3DVector			vMinTemp( 0, 0, 0 ), vMaxTemp( 0, 0, 0 ) ;
		size_t				nCount = m_arrRender.GetLength() - m_iShiftOffset ;
		RENDER_ENTRY*const*	ppEntries = m_arrRender.GetConstArray() + m_iShiftOffset ;
		//
		if ( nCount >= 1 )
		{
			vMinTemp = ppEntries[0]->vVertexMin ;
			vMaxTemp = ppEntries[0]->vVertexMax ;
		}
		for ( size_t i = 1; i < nCount; i ++ )
		{
			RENDER_ENTRY *	pre = ppEntries[i] ;
			vMinTemp.x = esl_fminf( vMinTemp.x, pre->vVertexMin.x ) ;
			vMinTemp.y = esl_fminf( vMinTemp.y, pre->vVertexMin.y ) ;
			vMinTemp.z = esl_fminf( vMinTemp.z, pre->vVertexMin.z ) ;
			vMaxTemp.x = esl_fmaxf( vMaxTemp.x, pre->vVertexMax.x ) ;
			vMaxTemp.y = esl_fmaxf( vMaxTemp.y, pre->vVertexMax.y ) ;
			vMaxTemp.z = esl_fmaxf( vMaxTemp.z, pre->vVertexMax.z ) ;
		}
		//
		m_vMinParallelepiped = vMinTemp ;
		m_vMaxParallelepiped = vMaxTemp ;
		m_flagCircumscribed = true ;
	}
	vMin = m_vMinParallelepiped ;
	vMax = m_vMaxParallelepiped ;
	return	(m_arrRender.GetLength() > m_iShiftOffset) ;
}

// 現在の設定に適合する S3DVertexVariantBuffer を生成
//////////////////////////////////////////////////////////////////////////////
S3DVertexVariantBuffer * S3DRenderBuffer::CreateVariantBuffer( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	const size_t		nCount = m_arrRender.GetLength() - m_iShiftOffset ;
	RENDER_ENTRY*const*	ppEntries = m_arrRender.GetConstArray() + m_iShiftOffset ;
	//
	size_t	nTotalBones = 0 ;
	size_t	nTotalMorphTargets = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RENDER_ENTRY *	pre = ppEntries[i] ;
		ESLAssert( pre != NULL ) ;
		nTotalBones += pre->countBone ;
		nTotalMorphTargets += pre->countMorph + 1 ;
	}
	//
	VariantBuffer *	pvb = new VariantBuffer ;
	MESH_VARIANT *	pmvArray = pvb->m_arrMeshVars.GetArray( nCount ) ;
	S3DMatrix *		pBoneMatrixBuf =
						pvb->m_bufBoneMatrix.GetArray( nTotalBones ) ;
	S3DVector *		pBoneTransBuf =
						pvb->m_bufBoneTrans.GetArray( nTotalBones ) ;
	ssize_t *		pMorphTargetBuf =
						pvb->m_bufMorphTarget.GetArray( nTotalMorphTargets ) ;
	float32_t *		pMorphApplyBuf =
						pvb->m_bufMorphApplication.GetArray( nTotalMorphTargets ) ;
	size_t			iNextBoneBuf = 0 ;
	size_t			iNextMorphBuf = 0 ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RENDER_ENTRY *	pre = ppEntries[i] ;
		ESLAssert( pre != NULL ) ;
		//
		MESH_VARIANT *	pmv = pmvArray + i ;
		//
		pmv->flagRenderable = pre->flagRenderable ;
		pmv->flagUpdateBone = false ;
		pmv->flagUpdateMorph = false ;
		pmv->flagUpdateMaterial = false ;
		pmv->pMaterial = pre->pMaterial ;
		pmv->countBone = pre->countBone ;
		pmv->pBoneMatrix = pBoneMatrixBuf + iNextBoneBuf ;
		pmv->pBoneTrans = pBoneTransBuf + iNextBoneBuf ;
		pmv->countMorph = pre->countMorph ;
		pmv->pMorphTargetMesh = pMorphTargetBuf + iNextMorphBuf ;
		pmv->pMorphApplication = pMorphApplyBuf + iNextMorphBuf ;
		pmv->nTargetMeshCount = pre->nTargetMeshCount ;
		//
		if ( pre->countBone > 0 )
		{
			eslCopyMemory
				( pmv->pBoneMatrix,
					pre->pBoneMatrix,
					pre->countBone * sizeof(S3DMatrix) ) ;
			eslCopyMemory
				( pmv->pBoneTrans,
					pre->pBoneTrans,
					pre->countBone * sizeof(S3DVector) ) ;
			iNextBoneBuf += pre->countBone ;
		}
		//
		if ( pre->nTargetMeshCount > 0 )
		{
			eslCopyMemory
				( pmv->pMorphTargetMesh,
					pre->pMorphTargetMesh,
					pre->nTargetMeshCount * sizeof(ssize_t) ) ;
			eslCopyMemory
				( pmv->pMorphApplication,
					pre->pMorphApplication,
					pre->nTargetMeshCount * sizeof(float32_t) ) ;
		}
		iNextMorphBuf += pre->countMorph + 1 ;
	}
	pvb->m_arrMeshVars.FinishArray() ;
	pvb->m_bufBoneMatrix.FinishArray() ;
	pvb->m_bufBoneTrans.FinishArray() ;
	pvb->m_bufMorphTarget.FinishArray() ;
	pvb->m_bufMorphApplication.FinishArray() ;
	//
	ESLAssert( iNextBoneBuf <= pvb->m_bufBoneMatrix.GetLength() ) ;
	ESLAssert( iNextMorphBuf <= pvb->m_bufMorphTarget.GetLength() ) ;
	//
	return	pvb ;
}

// S3DVertexVariantBuffer のパラメータを VertexBuffer へ反映
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::UpdateVertexVariant
	( S3DVertexVariantBuffer * pVVB, size_t iFirst, ssize_t iEnd )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	VariantBuffer *	pvb = ESLTypeCast<VariantBuffer>( pVVB ) ;
	if ( pvb == NULL )
	{
		return	sglErrFailed ;
	}
	ESLAssert( m_arrRender.GetLength() >= m_iShiftOffset ) ;
	const size_t		nCount = m_arrRender.GetLength() - m_iShiftOffset ;
	RENDER_ENTRY*const*	ppEntries = m_arrRender.GetConstArray() + m_iShiftOffset ;
	if ( nCount != pvb->m_arrMeshVars.GetLength() )
	{
		return	sglErrFailed ;
	}
	if ( iEnd < 0 )
	{
		iEnd = (ssize_t) nCount ;
	}
	const MESH_VARIANT *	pmvArray = pvb->m_arrMeshVars.GetConstArray() ;
	bool					flagAllUpdate = (m_pvvbLast != pvb) ;
	for ( size_t i = iFirst; i < (size_t) iEnd; i ++ )
	{
		RENDER_ENTRY *	pre = ppEntries[i] ;
		ESLAssert( pre != NULL ) ;
		//
		const MESH_VARIANT *	pmv = pmvArray + i ;
		pre->flagRenderable = pmv->flagRenderable ;
		pre->pMaterial = pmv->pMaterial ;
		if ( (flagAllUpdate | pmv->flagUpdateBone)
								& (pre->countBone > 0) )
		{
			if ( pre->countBone == pmv->countBone )
			{
				eslCopyMemory
					( pre->pBoneMatrix,
						pmv->pBoneMatrix,
						pre->countBone * sizeof(S3DMatrix) ) ;
				eslCopyMemory
					( pre->pBoneTrans,
						pmv->pBoneTrans,
						pre->countBone * sizeof(S3DVector) ) ;
				pre->flagUpdateBone = true ;
			}
		}
		if ( (flagAllUpdate | pmv->flagUpdateMorph)
								& (pre->countMorph > 0) )
		{
			pre->nTargetMeshCount = pmv->nTargetMeshCount ;
			if ( pre->nTargetMeshCount > 0 )
			{
				eslCopyMemory
					( pre->pMorphTargetMesh,
						pmv->pMorphTargetMesh,
						pre->nTargetMeshCount * sizeof(ssize_t) ) ;
				eslCopyMemory
					( pre->pMorphApplication,
						pmv->pMorphApplication,
						pre->nTargetMeshCount * sizeof(float32_t) ) ;
			}
			pre->flagMorphWithWeight = false ;
			if ( pre->pbMorphWeight )
			{
				for ( size_t i = 0; i < pre->nTargetMeshCount; i ++ )
				{
					ssize_t	j = pre->pMorphTargetMesh[i] ;
					if ( (j >= 0) && pre->pbMorphWeight[j] )
					{
						pre->flagMorphWithWeight = true ;
						break ;
					}
				}
			}
			pre->flagUpdateMorph = true ;
		}
	}
	m_pvvbLast = pvb ;
	return	sglErrSuccess ;
}

// 参照バリアントを生成
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DRenderBuffer::NewReferenceVariantBuffer( void )
{
	return	new S3DRenderVariantBuffer( this ) ;
}

// S3DVertexDeviceBufferInterface 取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexDeviceBufferInterface *
	S3DRenderBuffer::GetDeviceBufferTypeOf( SGLImageBufferObjectType type )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	if ( m_pFirstDevBuf != NULL )
	{
		return	m_pFirstDevBuf->GetDeviceBufferTypeOf( type ) ;
	}
	return	NULL ;
}

S3DVertexDeviceBufferInterface *
	S3DRenderBuffer::GetDeviceBufferAs( const ESLRuntimeClass& rtClass )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	if ( m_pFirstDevBuf != NULL )
	{
		return	m_pFirstDevBuf->GetDeviceBufferAs( rtClass ) ;
	}
	return	NULL ;
}

// S3DVertexDeviceBufferInterface 追加
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::AttachDeviceBuffer( S3DVertexDeviceBufferInterface * pDevBuf )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	if ( m_pFirstDevBuf != NULL )
	{
		pDevBuf->AddNextVertexDeviceBuffer( m_pFirstDevBuf ) ;
	}
	m_pFirstDevBuf = pDevBuf ;
}

// 自動結合プリミティブ追加処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBuffer::MergePrimitiveBuffer
	( S3DRenderBuffer::MergedPrimitiveBuffer& mpbuf, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	if ( mpbuf.m_preRel == NULL )
	{
		if ( nFlags & renderFenceOrder )
		{
			S3DRenderBuffer::Flush() ;
		}
		RENDER_ENTRY *	pre =
			(RENDER_ENTRY*) m_bufRender.Allocate( sizeof(RENDER_ENTRY) ) ;
		pre->nType = typePrimitive ;
		pre->nFlags = nFlags ;
		pre->pTransform = NULL ;
		pre->pShaderContext = NULL ;
		pre->flagRenderable = true ;
		//
		ESLAssert( mpbuf.m_prmbuf.pIndexedList != NULL ) ;
		pre->countPrimitive =
				countIndex / GetPrimitiveVertexCount(typePrimitive) ;
		//
		pre->countIndex = countIndex ;
		pre->countIndexLimit = countIndex ;
		pre->countVertex = countVertex ;
		pre->pMaterial = m_pDefaultMaterial ;
		pre->pvVertex = mpbuf.m_prmbuf.pvVertex ;
		pre->pvNormal = mpbuf.m_prmbuf.pvNormal ;
		pre->pvUVMap = mpbuf.m_prmbuf.pvUVMap ;
		pre->pColor = mpbuf.m_prmbuf.pColor ;
		pre->pvTexAxisX = NULL ;
		pre->pvTexAxisY = NULL ;
		pre->pIndexedList = mpbuf.m_prmbuf.pIndexedList ;
		for ( int i = 0; i < countSubMesh; i ++ )
		{
			pre->pSubIndexedList[i] = pre->pIndexedList ;
			pre->nSubIndexCount[i] = (uint32_t) countIndex ;
		}
		pre->iSubMeshSelector = -1 ;
		pre->fpSubMeshDensity = 4.0f ;
		//
		pre->nExAttrElements = 0 ;
		pre->pfpExAttrElements = 0 ;
		//
		pre->nInstancingCount = 0 ;
		pre->pInstancingMatrix = NULL ;
		pre->pInstancingColor = NULL ;
		//
		if ( nFlags & renderAutoTexAxis )
		{
			pre->pvTexAxisX =
				(S3DVector4*) m_bufRender.Allocate
								( countVertex * sizeof(S3DVector4) ) ;
			pre->pvTexAxisY =
				(S3DVector4*) m_bufRender.Allocate
								( countVertex * sizeof(S3DVector4) ) ;
		}
		//
		pre->pvTempVertex = NULL ;
		pre->pvTempNormal = NULL ;
		pre->pvTempUVMap = NULL ;
		pre->pvTempColor = NULL ;
		pre->pvTempMorphVertex = NULL ;
		pre->pvTempMorphNormal = NULL ;
		pre->pfpTempMorphWeight = NULL ;
		pre->pvTempBoneVertex = NULL ;
		pre->pvTempBoneNormal = NULL ;
		//
		pre->countBone = 0 ;
		pre->countWeightMap = 0 ;
		pre->countFullBone = 0 ;
		pre->flagUpdateBone = false ;
		pre->countMorph = 0 ;
		pre->flagUpdateMorph = false ;
		//
		if ( OnAddRenderBuffer
			( *pre, pre->pvVertex, pre->pvNormal,
						pre->pvUVMap, pre->pColor ) )
		{
			m_arrRender.Add( pre ) ;
			//
			if ( nFlags & renderFenceOrder )
			{
				m_iFenceOrder = m_arrRender.GetLength() ;
			}
			mpbuf.m_preRel = pre ;
		}
		else
		{
			mpbuf.m_flagAllocPrmBuf = false ;
			return	sglErrFailed ;
		}
	}
	else
	{
		RENDER_ENTRY *	pre = mpbuf.m_preRel ;
		if ( m_pTransformation &&
			(pre->pTransform != m_pTransformation->pTransBuf) )
		{
			S3DDMatrix	matITrans ;
			S3DDVector	vIPos = - pre->pTransform->vTransform ;
			matITrans.InverseOf( pre->pTransform->matTransform ) ;
			vIPos = matITrans * vIPos ;
			//
			S3DMatrix	matTrans = matITrans * m_pTransformation->matTransform ;
			S3DVector	vPos = matITrans * m_pTransformation->vTransform + vIPos ;
			S3DVector	vZero( 0, 0, 0 ) ;
			//
			matTrans.RevolveVectors
				( mpbuf.m_prmbuf.pvVertex,
					mpbuf.m_prmbuf.pvVertex, countVertex, vPos ) ;
			matTrans.RevolveVectors
				( mpbuf.m_prmbuf.pvNormal,
					mpbuf.m_prmbuf.pvNormal, countVertex, vZero ) ;
		}
		uint32_t	nBaseIndex = (uint32_t) pre->countVertex ;
		pre->countPrimitive +=
				countIndex / GetPrimitiveVertexCount(typePrimitive) ;
		pre->countIndex += countIndex ;
		pre->countIndexLimit += countIndex ;
		pre->countVertex += countVertex ;
		for ( int i = 0; i < countSubMesh; i ++ )
		{
			pre->nSubIndexCount[i] = (uint32_t) pre->countIndex ;
		}
		uint32_t *	pIndexBuf = mpbuf.m_prmbuf.pIndexedList ;
		for ( size_t i = 0; i < countIndex; i ++ )
		{
			pIndexBuf[i] += nBaseIndex ;
		}
		//
		S3DVector4	vMax4, vMin4 ;
		MinMaxVector4DArray
			( vMin4, vMax4, mpbuf.m_prmbuf.pvVertex, countVertex ) ;
		//
		pre->vVertexMax.x = esl_fmaxf( vMax4.x, pre->vVertexMax.x ) ;
		pre->vVertexMax.y = esl_fmaxf( vMax4.y, pre->vVertexMax.y ) ;
		pre->vVertexMax.z = esl_fmaxf( vMax4.z, pre->vVertexMax.z ) ;
		pre->vVertexMin.x = esl_fminf( vMin4.x, pre->vVertexMin.x ) ;
		pre->vVertexMin.y = esl_fminf( vMin4.y, pre->vVertexMin.y ) ;
		pre->vVertexMin.z = esl_fminf( vMin4.z, pre->vVertexMin.z ) ;
		pre->vCenter = pre->vVertexMax * 0.5 + pre->vVertexMin * 0.5 ;
		pre->fpRadius = (float32_t) (pre->vVertexMax - pre->vCenter).Absolute() ;
		pre->flagUpdateBone = true ;
		m_flagCircumscribed = false ;
	}
	//
	mpbuf.m_flagAllocPrmBuf = false ;
	mpbuf.m_nVertexCount += countVertex ;
	mpbuf.m_nIndexCount += countIndex ;
	//
	if ( mpbuf.m_nMaxVertexCount < mpbuf.m_nVertexCount )
	{
		mpbuf.m_nMaxVertexCount = mpbuf.m_nVertexCount ;
	}
	if ( mpbuf.m_nMaxIndexCount < mpbuf.m_nIndexCount )
	{
		mpbuf.m_nMaxIndexCount = mpbuf.m_nIndexCount ;
	}
	return	sglErrSuccess ;
}

// ソート処理
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::OnSortRenderBuffer
	( S3DRenderBuffer::RENDER_ENTRY** ppRender, size_t nCount )
{
	if ( nCount <= 4 )
	{
		//
		// ソート用ハッシュ値の昇順に選択ソート
		//
		size_t	i = nCount ;
		while ( -- i >= 1 )
		{
			RENDER_ENTRY *	prePivot = ppRender[i] ;
			RENDER_ENTRY *	preMax = prePivot ;
			RENDER_ENTRY *	preTemp ;
			uint64_t		nHashMax = prePivot->nSortHash ;
			uint64_t		nHashTemp ;
			size_t			iMax = i ;
			size_t			j = i - 1 ;
			do
			{
				preTemp = ppRender[j] ;
				nHashTemp = preTemp->nSortHash ;
				if ( nHashTemp > nHashMax )
				{
					preMax = preTemp ;
					nHashMax = nHashTemp ;
					iMax = j ;
				}
			}
			while ( (j --) > 0 ) ;
			//
			ppRender[i] = preMax ;
			ppRender[iMax] = prePivot ;
		}
		return ;
	}
	//
	// ソート用ハッシュ値の昇順にクイックソート
	//
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) nCount - 1 ;
	ssize_t	iPivot = iEnd ;
	RENDER_ENTRY *	prePivot = ppRender[iPivot] ;
	RENDER_ENTRY *	preLeft ;
	RENDER_ENTRY *	preRight ;
	uint64_t		nHashPivot = prePivot->nSortHash ;
	while ( iFirst < iEnd )
	{
		// 基準値より大きい要素を左側から順次検索
		preLeft = ppRender[iFirst] ;
		if ( preLeft->nSortHash > nHashPivot )
		{
			ppRender[iEnd --] = preLeft ;
			//
			while ( iFirst < iEnd )
			{
				// 基準値より小さい要素を右側から順次検索
				preRight = ppRender[iEnd] ;
				if ( preRight->nSortHash < nHashPivot )
				{
					ppRender[iFirst ++] = preRight ;
					break ;
				}
				-- iEnd ;
			}
			continue ;
		}
		++ iFirst ;
	}
	ESLAssert( iFirst == iEnd ) ;
	ESLAssert( iFirst >= 0 ) ;
	ppRender[iFirst] = prePivot ;
	//
	if ( iFirst >= 2 )
	{
		OnSortRenderBuffer( ppRender, iFirst ) ;
	}
	++ iFirst ;
	if ( (ssize_t) nCount > iFirst + 1 )
	{
		OnSortRenderBuffer( ppRender + iFirst, nCount - iFirst ) ;
	}
}

// 追加時処理 (デフォルトは頂点変換)
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBuffer::OnAddRenderBuffer
	( S3DRenderBuffer::RENDER_ENTRY& entry,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	//
	// 頂点座標設定
	//
	UpdateRenderBuffer( entry, pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	// 座標変換
	//
	SetEntryTransformation( entry ) ;
	//
	// ソート用ハッシュ値計算
	//
	S3DDVector	vSortPos =
		(entry.pvVertex[0]
			+ entry.pvVertex[entry.countVertex/4]
			+ entry.pvVertex[entry.countVertex/2]
			+ entry.pvVertex[entry.countVertex*3/4]) * 0.25 ;
	float32_t	zFloatSort = (float32_t) vSortPos.z ;
	if ( entry.pTransform != NULL )
	{
		entry.pTransform->matTransform.RevolveVector( vSortPos ) ;
		vSortPos += entry.pTransform->vTransform ;
		zFloatSort = (float32_t) (m_matCamera * vSortPos).z ;
	}
	MakeSoftHash( entry, zFloatSort ) ;
	//
	return	true ;
}

// バッファ更新処理
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::UpdateRenderBuffer
	( S3DRenderBuffer::RENDER_ENTRY& entry,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	const size_t	countVertex = entry.countVertex ;
	if ( pvVertex != NULL )
	{
		S3DVector	vMax, vMin ;
		if ( entry.pvVertex != pvVertex )
		{
			MoveVertexAndCubeRange
				( vMax, vMin, entry.pvVertex, pvVertex, entry.countVertex ) ;
		}
		else
		{
			S3DVector4	vMax4, vMin4 ;
			MinMaxVector4DArray
				( vMin4, vMax4, entry.pvVertex, entry.countVertex ) ;
			//
			vMax = vMax4 ;
			vMin = vMin4 ;
		}
		ESLAssert( !vMax.IsNaN() ) ;
		ESLAssert( !vMin.IsNaN() ) ;
		//
		entry.vVertexMax = vMax ;
		entry.vVertexMin = vMin ;
		entry.vCenter = vMax * 0.5 + vMin * 0.5 ;
		entry.fpRadius = (float32_t) (vMax - entry.vCenter).Absolute() ;
		entry.flagUpdateBone = true ;
		m_flagCircumscribed = false ;
	}
	if ( (pvNormal != NULL) && (entry.pvNormal != pvNormal) )
	{
		eslCopyMemory
			( entry.pvNormal, pvNormal,
				entry.countVertex * sizeof(S3DVector4) ) ;
		entry.flagUpdateBone = true ;
	}
	if ( (pvUVMap != NULL) && (entry.pvUVMap != pvUVMap) )
	{
		eslCopyMemory
			( entry.pvUVMap, pvUVMap,
				entry.countVertex * sizeof(S2DVector) ) ;
	}
	if ( pColor != NULL )
	{
		if ( entry.pColor != pColor )
		{
			eslCopyMemory
				( entry.pColor, pColor,
					entry.countVertex * sizeof(S3DColor) ) ;
		}
	}
	else if ( entry.pColor != NULL )
	{
		S3DColor	colorDummy( 0xFFFFFFFF, 0 ) ;
		S3DColor *	pDstColor = entry.pColor ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			*pDstColor = colorDummy ;
			pDstColor ++ ;
		}
		entry.nFlags &= ~renderAutoColor ;
	}
	if ( entry.nFlags & renderAutoNormal )
	{
		GenerateDefaultNormal( entry ) ;
	}
	if ( (entry.nFlags & renderAutoTexAxis)
		&& (pvUVMap != NULL)
		&& (entry.pvTexAxisX != NULL) && (entry.pvTexAxisY != NULL) )
	{
		if ( m_bufTempTexAxis.SetForIndexedPrimitiveList
				( (S3DPrimitiveType) entry.nType,
					entry.countPrimitive, entry.countVertex,
					entry.pvVertex, entry.pvUVMap, entry.pIndexedList ) )
		{
			eslCopyMemory
				( entry.pvTexAxisX,
					m_bufTempTexAxis.GetBufferAxisX(),
					entry.countVertex * sizeof(S3DVector4) ) ;
			eslCopyMemory
				( entry.pvTexAxisY,
					m_bufTempTexAxis.GetBufferAxisY(),
					entry.countVertex * sizeof(S3DVector4) ) ;
		}
	}
}

// 頂点を複製するとともに最大値と最小値を取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MoveVertexAndCubeRange
	( S3DVector& vMax, S3DVector& vMin,
		S3DVector4 * pvDstVertex,
		const S3DVector4 * pvSrcVertex, size_t nCount )
{
	if ( nCount == 0 )
	{
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		return ;
	}
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		float32_t	fpMax[4], fpMin[4] ;
		//
		__m128	xmmMax = _mm_loadu_ps( (float*) pvSrcVertex ) ;
		__m128	xmmMin = xmmMax ;
		_mm_storeu_ps( (float*) pvDstVertex, xmmMax ) ;
		//
		for ( size_t i = 1; i < nCount; i ++ )
		{
			__m128	xmmNext = _mm_loadu_ps( (float*) (pvSrcVertex + i) ) ;
			xmmMax =_mm_max_ps( xmmMax, xmmNext ) ;
			xmmMin =_mm_min_ps( xmmMin, xmmNext ) ;
			_mm_storeu_ps( (float*) (pvDstVertex + i), xmmNext ) ;
		}
		_mm_storeu_ps( fpMax, xmmMax ) ;
		_mm_storeu_ps( fpMin, xmmMin ) ;
		//
		vMax.x = fpMax[0] ;
		vMax.y = fpMax[1] ;
		vMax.z = fpMax[2] ;
		vMin.x = fpMin[0] ;
		vMin.y = fpMin[1] ;
		vMin.z = fpMin[2] ;
	}
	else
	{
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		ESLAssert( g_cpuFamily == cpuFamily_ARM ) ;
		//
		S3DVector4	vMaxMin[2] ;
		//
		ERISA_sclfMoveVertexAndCubeRange_ARM_NEON
			( &vMaxMin[0], pvDstVertex, pvSrcVertex, nCount ) ;
		//
		vMax = vMaxMin[0] ;
		vMin = vMaxMin[1] ;
	}
	else
	{
	#endif
		int32_t	xMax = *((int32_t*)&(pvSrcVertex->x)) ;
		int32_t	yMax = *((int32_t*)&(pvSrcVertex->y)) ;
		int32_t	zMax = *((int32_t*)&(pvSrcVertex->z)) ;
		xMax ^= (xMax >> 31) & 0x7FFFFFFF ;
		yMax ^= (yMax >> 31) & 0x7FFFFFFF ;
		zMax ^= (zMax >> 31) & 0x7FFFFFFF ;
		//
		int32_t	xMin = xMax, yMin = yMax, zMin = zMax ;
		//
		*(pvDstVertex ++) = *(pvSrcVertex ++) ;
		//
		for ( size_t i = 1; i < nCount; i ++ )
		{
			int32_t	x = *((int32_t*)&(pvSrcVertex->x)) ;
			int32_t	y = *((int32_t*)&(pvSrcVertex->y)) ;
			int32_t	z = *((int32_t*)&(pvSrcVertex->z)) ;
			//
			*(pvDstVertex ++) = *(pvSrcVertex ++) ;
			//
			x ^= (x >> 31) & 0x7FFFFFFF ;
			y ^= (y >> 31) & 0x7FFFFFFF ;
			z ^= (z >> 31) & 0x7FFFFFFF ;
			//
			#if	defined(__COTOPHA__)
				int32_t	fx = (int) (x > xMax) ;
				int32_t	fy = (int) (y > yMax) ;
				int32_t	fz = (int) (z > zMax) ;
			#else
				int32_t	fx = - (int) (x > xMax) ;
				int32_t	fy = - (int) (y > yMax) ;
				int32_t	fz = - (int) (z > zMax) ;
			#endif
			xMax = (xMax & ~fx) | (x & fx) ;
			yMax = (yMax & ~fy) | (y & fy) ;
			zMax = (zMax & ~fz) | (z & fz) ;
			//
			#if	defined(__COTOPHA__)
				fx = (int) (x < xMin) ;
				fy = (int) (y < yMin) ;
				fz = (int) (z < zMin) ;
			#else
				fx = - (int) (x < xMin) ;
				fy = - (int) (y < yMin) ;
				fz = - (int) (z < zMin) ;
			#endif
			xMin = (xMin & ~fx) | (x & fx) ;
			yMin = (yMin & ~fy) | (y & fy) ;
			zMin = (zMin & ~fz) | (z & fz) ;
		}
		xMax ^= (xMax >> 31) & 0x7FFFFFFF ;
		yMax ^= (yMax >> 31) & 0x7FFFFFFF ;
		zMax ^= (zMax >> 31) & 0x7FFFFFFF ;
		xMin ^= (xMin >> 31) & 0x7FFFFFFF ;
		yMin ^= (yMin >> 31) & 0x7FFFFFFF ;
		zMin ^= (zMin >> 31) & 0x7FFFFFFF ;
		//
		vMax.x = *((float32_t*)&xMax) ;
		vMax.y = *((float32_t*)&yMax) ;
		vMax.z = *((float32_t*)&zMax) ;
		vMin.x = *((float32_t*)&xMin) ;
		vMin.y = *((float32_t*)&yMin) ;
		vMin.z = *((float32_t*)&zMin) ;
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	}
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	}
	#endif
}

// 現在の変換をエントリに設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::SetEntryTransformation
					( S3DRenderBuffer::RENDER_ENTRY& entry )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	entry.pTransform = NULL ;
	entry.pShaderContext = NULL ;
	//
	TransformationList *	pTrans = m_pTransformation ;
	if ( pTrans != NULL )
	{
		if ( pTrans->pTransBuf == NULL )
		{
			pTrans->pTransBuf =
				(Transformation*)
					m_bufRender.Allocate( sizeof(Transformation) ) ;
			*(pTrans->pTransBuf) = *((Transformation*)m_pTransformation) ;
		}
		entry.pTransform = pTrans->pTransBuf ;
		//
		if ( pTrans->pContextBuf == NULL )
		{
			AllocateCustumShaderUniformList( pTrans ) ;
		}
		entry.pShaderContext = pTrans->pContextBuf ;
	}
	else
	{
		entry.pShaderContext = AllocateShaderContextEntry() ;
	}
}

// カスタムシェーダーパラメータリストを生成
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::AllocateCustumShaderUniformList
		( S3DRenderBuffer::TransformationList * pTrans )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	if ( pTrans->pUniformBuf == NULL )
	do
	{
		CustomUniformSet *	pcus = pTrans->pcus ;
		if ( pcus == NULL )
		{
			break ;
		}
		size_t	nCount = pcus->GetLength() ;
		if ( nCount == 0 )
		{
			break ;
		}
		CustomUniformEntry *	pcuList =
			(CustomUniformEntry*)
				m_bufRender.Allocate
					( nCount * sizeof(CustomUniformEntry) ) ;
		pTrans->pUniformBuf = pcuList ;
		//
		size_t	nAccCount = 0 ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			const SString *	pstrID = pcus->GetTagAt( i ) ;
			CustomUniform *	pcu = pcus->GetAt( i ) ;
			if ( pstrID && pcu && pcu->m_nLength )
			{
				size_t		nIdLen = pstrID->GetLength() ;
				wchar_t *	pwszID =
					(wchar_t*) m_bufRender.Allocate
									( (nIdLen + 1) * sizeof(wchar_t) ) ;
				pcuList->m_pwszID = pwszID ;
				eslMoveMemory
					( pwszID, (const wchar_t*) *pstrID,
								(nIdLen + 1) * sizeof(wchar_t) ) ;
				//
				pcuList->m_type = pcu->m_type ;
				pcuList->m_nLength = pcu->m_nLength ;
				//
				size_t	nBytes = pcu->GetDataBytes() ;
				pcuList->m_pData = m_bufRender.Allocate( nBytes ) ;
				eslMoveMemory
					( pcuList->m_pData, pcu->m_pData, nBytes ) ;
				//
				pcuList ++ ;
				nAccCount ++ ;
			}
		}
		pTrans->nUnitofms = nAccCount ;
	}
	while ( false );
	if ( m_pLastShaderContext != NULL )
	{
		if ( S3DRenderParameterContext::IsEqualOptionalContext
				( m_optContext, m_pLastShaderContext->optContext )
			&& (m_pLastShaderContext->pUniformBuf == pTrans->pUniformBuf)
			&& (m_pLastShaderContext->nUnitofms == pTrans->nUnitofms) )
		{
			pTrans->pContextBuf = m_pLastShaderContext ;
			return ;
		}
	}
	ShaderContextEntry *	psceContext = AllocateShaderContextEntry() ;
	//
	psceContext->pUniformBuf = pTrans->pUniformBuf ;
	psceContext->nUnitofms = pTrans->nUnitofms ;
	ESLAssert( (psceContext->pUniformBuf != NULL)
			|| ((psceContext->pUniformBuf == NULL)
						&& (psceContext->nUnitofms == 0)) ) ;
	//
	pTrans->pContextBuf = psceContext ;
	m_pLastShaderContext = psceContext ;
}

S3DRenderBuffer::ShaderContextEntry *
	S3DRenderBuffer::AllocateShaderContextEntry( void )
{
	ESLAssert( m_csBufSync.TestLocked() > 0 ) ;
	ShaderContextEntry *	psceContext =
		(ShaderContextEntry*)
			m_bufRender.Allocate( sizeof(ShaderContextEntry) ) ;
	psceContext->optContext = m_optContext ;
	psceContext->pUniformBuf = NULL ;
	psceContext->nUnitofms = 0 ;
	return	psceContext ;
}

// 法線を自動生成
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::GenerateDefaultNormal
					( S3DRenderBuffer::RENDER_ENTRY& entry )
{
	if ( entry.pvNormal != NULL )
	{
		if ( m_bufTempNormal.SetForIndexedPrimitiveList
			( (S3DPrimitiveType) entry.nType,
				entry.countPrimitive, entry.countVertex,
				entry.pvVertex, entry.pvUVMap, entry.pIndexedList ) )
		{
			eslCopyMemory
				( entry.pvNormal,
					m_bufTempNormal.GetNormalBuffer(),
					entry.countVertex * sizeof(S3DVector4) ) ;
		}
	}
}

// ソート用ハッシュ値計算
//////////////////////////////////////////////////////////////////////////////
// hash code:
//   63      56        48        40        32 31           0
//   +---- ----+---- ----+---- ----+---- ----+----- ... ----+
//    YYYY ZZXX [shader ] [VBO+material hash] [   z value   ]
//
//  ZZ:   Z-buffer operation, 00=enabled, 01=no write, 10=disabled
//  YYYY: rendering order priority
//  XX:   material priority
//
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBuffer::MakeSoftHash
	( S3DRenderBuffer::RENDER_ENTRY& entry, float32_t zFloatSort )
{
	uint32_t	zUintSort = *((uint32_t*)&zFloatSort) ;
	zUintSort = (~zUintSort ^ 0x80000000)
					^ (((int32_t)zUintSort >> 31) & 0x7FFFFFFF) ; 
	//
	uint32_t	hashMaterial = (uint32_t) ((ulong_ptr_t) entry.pMaterial) ;
	if ( entry.nType == typeVertexBuffer )
	{
		uint32_t	hashVBO = (uint32_t) ((ulong_ptr_t) entry.pVertexBuffer) ;
		hashMaterial =
			(((hashMaterial & 0xFF00FF00)
				^ ((hashMaterial & 0x00FF00FF) << 8))
			| ((hashVBO & 0x00FF00FF)
				^ ((hashVBO & 0xFF00FF00) >> 8)))
								^ ((uint32_t) entry.iFirstBuf) ;
	}
	uint64_t	flagsShading =
					entry.pMaterial
						? entry.pMaterial->m_attrSurface.flagsShading : 0 ;
	uint64_t	hashShading =
					(flagsShading & shadingHintMask) >> shadingHintShifter ;
	if ( flagsShading & shadingNoZBuffer )
	{
		hashShading |= 0x08 | ((m_nRenderPriority & 0x0F) << 4) ;
		entry.nSortHash = (hashShading << 56) | zUintSort ;
		//
		if ( flagsShading & shadingHintNoZSort )
		{
			hashMaterial = (hashMaterial ^ (hashMaterial >> 16)) & 0xFFFF ;
			hashMaterial |= m_nCurShaderHash << 16 ;
			entry.nSortHash |= ((uint64_t)hashMaterial) << 32 ;
		}
	}
	else if ( flagsShading & shadingZBufferNoWrite )
	{
		hashShading |= 0x04 | ((m_nRenderPriority & 0x0F) << 4) ;
		entry.nSortHash = (hashShading << 56) | zUintSort ;
		//
		if ( flagsShading & shadingHintNoZSort )
		{
			hashMaterial = (hashMaterial ^ (hashMaterial >> 16)) & 0xFFFF ;
			hashMaterial |= m_nCurShaderHash << 16 ;
			entry.nSortHash |= ((uint64_t)hashMaterial) << 32 ;
		}
	}
	else
	{
		hashMaterial = (hashMaterial ^ (hashMaterial >> 16)) & 0xFFFF ;
		hashMaterial |= m_nCurShaderHash << 16 ;
		//
		if ( (hashShading >= (shadingHintPriority2 >> shadingHintShifter))
			&& !(flagsShading & shadingHintNoZSort) )
		{
			hashMaterial = (m_nCurShaderHash << 16) | 0xFF ;
		}
		hashShading |= ((m_nRenderPriority & 0x0F) << 4) ;
		entry.nSortHash = (hashShading << 56) | zUintSort
								| (((uint64_t)hashMaterial) << 32) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// バーテックス・バッファとして使う S3DRenderBuffer
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderVertexBuffer, S3DRenderBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderVertexBuffer::S3DRenderVertexBuffer( void )
{
}

S3DRenderVertexBuffer::S3DRenderVertexBuffer( const S3DRenderBuffer & buf )
	: S3DRenderBuffer( buf )
{
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVertexBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
			uint64_t flagsExclusion,
			size_t iFirst, ssize_t iEnd,
			size_t nInstancing,
			const S4DMatrix * pmatInstancing,
			const S3DColor * pColorInstancing ) const
{
	return	RenderVertexBufferTo
				( render, (S3DVertexBufferInterface*) this,
					flagsExclusion, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}


//////////////////////////////////////////////////////////////////////////////
// レンダリング・バリアント・バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderVariantBuffer, S3DVertexBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderVariantBuffer::S3DRenderVariantBuffer( S3DRenderBuffer * prb )
	: S3DVertexBuffer( prb, false )
{
	ESLAssert( prb != NULL ) ;
	m_pvvbVar = prb->CreateVariantBuffer() ;
	m_flagInstancing = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderVariantBuffer::~S3DRenderVariantBuffer( void )
{
	delete	m_pvvbVar ;
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
			uint64_t flagsExclusion,
			size_t iFirst, ssize_t iEnd,
			size_t nInstancing,
			const S4DMatrix * pmatInstancing,
			const S3DColor * pColorInstancing ) const
{
	S3DRenderBuffer *	prb = ESLTypeCast<S3DRenderBuffer>( m_buffer ) ;
	ESLAssert( prb != NULL ) ;
	if ( prb == NULL )
	{
		return	sglErrFailed ;
	}
	// S3DRenderVariantBuffer 自身を VertexBuffer として
	// レンダリングキューに追加する
	// 最終的にデバイス固有レンダラの AddVertexBuffer によって
	// prb->UpdateVertexVariant( m_pvvbVar, iFirst, iEnd ) が呼び出される
	return	prb->RenderVertexBufferTo
				( render, (S3DVertexBufferInterface*) this,
					flagsExclusion, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// VertexBuffer 取得
// ※S3DRenderVariantBuffer自身を返す
// 　参照先を取得するには GetVertexBuffer を呼び出す
//////////////////////////////////////////////////////////////////////////////
VertexBuffer * S3DRenderVariantBuffer::GetVertexBufferObject( void ) const
{
#if	defined(__COTOPHA__)
	return	S3DVertexBuffer::GetVertexBufferObject() ;
#else
	return	S3DVertexBufferInterface::GetVertexBufferObject() ;
#endif
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::EnableMultiInstancingMode( bool flagEnable )
{
	m_flagInstancing = flagEnable ;
	return	sglErrSuccess ;
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderVariantBuffer::IsMultiInstancingMode( void ) const
{
	return	m_flagInstancing ;
}

// インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderVariantBuffer::GetInstancingCount( void ) const
{
	return	m_aInstanceVVB.GetLength() ;
}

// インスタンス取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderVariantBuffer::GetInstancingEntries
	( S3DVertexVariantBuffer** ppVVB,
		S4DMatrix * pmatInstance,
		S3DColor * pcolorInstance,
		size_t iFirst, size_t nCount ) const
{
	if ( (m_aInstanceMatrixs.GetLength() < iFirst + nCount)
		|| (m_aInstanceColors.GetLength() < iFirst + nCount)
		|| (m_aInstanceVVB.GetLength() < iFirst + nCount) )
	{
		return	0 ;
	}
	m_csInstanceSync.Lock() ;
	eslCopyMemory
		( ppVVB, m_aInstanceVVB.GetConstArray() + iFirst,
			nCount * sizeof(S3DVertexVariantBuffer*) ) ;
	eslCopyMemory
		( pmatInstance,
			m_aInstanceMatrixs.GetConstArray() + iFirst,
			nCount * sizeof(S4DMatrix) ) ;
	eslCopyMemory
		( pcolorInstance,
			m_aInstanceColors.GetConstArray() + iFirst,
			nCount * sizeof(S3DColor) ) ;
	m_csInstanceSync.Unlock() ;
	return	nCount ;
}

// インスタンス全消去
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::ClearAllInstance( void )
{
	m_csInstanceSync.Lock() ;
	m_aInstanceMatrixs.RemoveAll() ;
	m_aInstanceColors.RemoveAll() ;
	m_aInstanceVVB.RemoveAll() ;
	m_csInstanceSync.Unlock() ;
	return	sglErrSuccess ;
}

// インスタンス追加設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::AddInstanceVariant
	( S3DVertexVariantBuffer * pVVB,
		const S4DMatrix & matInstance,
		const S3DColor & colorInstance )
{
	m_csInstanceSync.Lock() ;
	m_aInstanceVVB.Add( pVVB ) ;
	m_aInstanceMatrixs.Add( matInstance ) ;
	m_aInstanceColors.Add( colorInstance ) ;
	ESLAssert( m_aInstanceVVB.GetLength() == m_aInstanceMatrixs.GetLength() ) ;
	ESLAssert( m_aInstanceVVB.GetLength() == m_aInstanceColors.GetLength() ) ;
	m_csInstanceSync.Unlock() ;
	return	sglErrSuccess ;
}

// メッシュにボーン行列設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::SetBoneMatrix
	( size_t iMesh, size_t nCount,
		const S3DMatrix * pMatrix, const S3DVector * pTrans )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->SetBoneMatrix( iMesh, nCount, pMatrix, pTrans ) ;
}

// メッシュのボーン行列取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DRenderVariantBuffer::GetBoneMatrix
	( size_t iMesh, size_t nCount,
		S3DMatrix * pMatrix, S3DVector * pTrans )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->GetBoneMatrix( iMesh, nCount, pMatrix, pTrans ) ;
}

// モーフィング設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::SetMorphingApplication
	( size_t iMesh, const ssize_t * pTargetMesh,
			const float32_t * pApplication, size_t nTargetMeshCount )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->SetMorphingApplication
				( iMesh, pTargetMesh, pApplication, nTargetMeshCount ) ;
}

// モーフィング設定取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::GetMorphingApplication
	( size_t iMesh, ssize_t& iTargetMesh,
			float32_t& fpApplication, size_t iTargetMeshIndex )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->GetMorphingApplication
				( iMesh, iTargetMesh, fpApplication, iTargetMeshIndex ) ;
}

// メッシュ表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::EnableToRenderMesh
			( size_t iFirst, ssize_t iEnd, bool fEnable )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->EnableToRenderMesh( iFirst, iEnd, fEnable ) ;
}

// メッシュ表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderVariantBuffer::IsEnabledToRenderMesh( size_t iMesh ) const
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->IsEnabledToRenderMesh( iMesh ) ;
}

// メッシュマテリアル設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderVariantBuffer::SetMaterialToRenderMesh
	( size_t iMesh, S3DMaterial * pMaterial )
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->SetMaterialToRenderMesh( iMesh, pMaterial ) ;
}

// メッシュマテリアル取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DRenderVariantBuffer::GetMaterialToRenderMesh( size_t iMesh ) const
{
	ESLAssert( m_pvvbVar != NULL ) ;
	return	m_pvvbVar->GetMaterialToRenderMesh( iMesh ) ;
}

