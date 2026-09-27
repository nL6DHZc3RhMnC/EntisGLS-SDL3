
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
     Copyright (c) 2008-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <gls.h>
#include <math.h>
#include <cloth_morph.h>

#include <stddef.h>
//#include <mmintrin.h>	// MMX
//#include <xmmintrin.h>	// SSE
//#include <emmintrin.h>	// SSE2

static const double	PI = 3.1415926535897932384626433832795 ;


//////////////////////////////////////////////////////////////////////////////
// HEGL_CLOTH_MODEL_MORPH オブジェクト構築
//////////////////////////////////////////////////////////////////////////////

HEGL_CLOTH_MODEL_MORPH eglCreateClothModelMorph( void )
{
	return	new EGLClothModelMorphObject ;
}


//////////////////////////////////////////////////////////////////////////////
// 布シミュレーション実装抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::EGLClothModelMorphObject( void )
{
	pfnRelease = Call_Release ;
	pfnDeleteCloth = Call_DeleteCloth ;
	pfnInitializeCloth = Call_InitializeCloth ;
	pfnSetHinderModel = Call_SetHinderModel ;
	pfnWeaveCloth = Call_WeaveCloth ;
	pfnPatchCloth = Call_PatchCloth ;
	pfnMorphCloth = Call_MorphCloth ;
	pfnMorphMesh = Call_MorphMesh ;
	//
	m_phmiHinder = NULL ;
	m_nHinderCount = 0 ;
	//
	m_dwRandomSeed = GetTickCount() ;
	//
	m_rStreamAmplitude[0] = 1.0 ;
	m_rStreamAmplitude[1] = Random( 0x100 ) / 256.0 ;
	m_rStreamAmplitude[2] = Random( 0x80 ) / 256.0 ;
	m_rStreamPhase[0] = 0.0 ;
	m_rStreamPhase[1] = PI * Random( 0x100 ) / 256.0 ;
	m_rStreamPhase[2] = PI * Random( 0x100 ) / 256.0 ;
	m_rStreamFrequency[0] = 1.0 ;
	m_rStreamFrequency[1] = 0.5 + (Random( 0x100 ) / 256.0) ;
	m_rStreamFrequency[2] = 0.5 + (Random( 0x100 ) / 256.0) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::~EGLClothModelMorphObject( void )
{
}

// 乱数発生
//////////////////////////////////////////////////////////////////////////////
int EGLClothModelMorphObject::Random( int nLimit )
{
	m_dwRandomSeed = m_dwRandomSeed * 9 + 0x4589AC57 ;
	return	(int) (m_dwRandomSeed >> 16) % nLimit ;
}

// ストリームパラメータ更新
//////////////////////////////////////////////////////////////////////////////
void EGLClothModelMorphObject::UpdateStream
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp )
{
	const double	PI2 = PI * 2.0 ;
	double			rAmp = 0.0 ;
	for ( int i = 0; i < 2; i ++ )
	{
		double	rFreq = pcmp->rStreamFrequency * m_rStreamFrequency[i] ;
		m_rStreamPhase[i] += PI2 * pcmp->rDeltaTime / rFreq ;
		m_rStreamPhase[i] = fmod( m_rStreamPhase[i], PI2 ) ;
		rAmp += (1.0 + sin(m_rStreamPhase[i]))
							* (m_rStreamAmplitude[i] * 0.5) ;
	}
	m_vStream = pcmp->vStream * rAmp ;
}

// 障害物当たり判定
//////////////////////////////////////////////////////////////////////////////
bool EGLClothModelMorphObject::TestCollisionSphere
	( const E3D_VECTOR & vPos,
		E3D_VECTOR & vHitPos, E3D_VECTOR & vHitNormal ) const
{
	bool		fHit = false ;
	double		rHitDistance = 0.0 ;
	E3D_VECTOR	vNearestHits, vNearestNormal ;
	//
	for ( unsigned int i = 0; i < m_nHinderCount; i ++ )
	{
		const HINDER_MODEL_INFO *	phmi = m_phmiHinder + i ;
		int	nResult ;
		if ( !phmi->hMatrix->IsHitAgainstSphere
				( &vPos, phmi->rGapRadius,
					&nResult, &vNearestHits, &vNearestNormal, NULL ) )
		{
			if ( nResult )
			{
				REAL32	r = (vPos - vNearestHits).Absolute() ;
				if ( !fHit || (r < rHitDistance) )
				{
					vHitPos = vNearestHits
							+ vNearestNormal * phmi->rGapRadius ;
					vHitNormal = vNearestHits ;
					rHitDistance = r ;
					fHit = true ;
				}
			}
		}
	}
	return	fHit ;
}

bool EGLClothModelMorphObject::TestCollisionSegment
	( const E3D_VECTOR & vPos0, const E3D_VECTOR & vPos1,
		E3D_VECTOR & vHitPos, E3D_VECTOR & vHitNormal ) const
{
	bool		fHit = false ;
	double		rHitDistance = 0.0 ;
	E3D_VECTOR	vNearestHits, vNearestNormal ;
	//
	for ( unsigned int i = 0; i < m_nHinderCount; i ++ )
	{
		const HINDER_MODEL_INFO *	phmi = m_phmiHinder + i ;
		int	nResult ;
		if ( !phmi->hMatrix->IsCrossingSegment
				( &vPos0, &vPos1, phmi->rGapRadius * 0.75,
					&nResult, &vNearestHits, &vNearestNormal, NULL ) )
		{
			if ( nResult )
			{
				REAL32	r = (vPos0 - vNearestHits).Absolute() ;
				if ( !fHit || (r < rHitDistance) )
				{
					vHitPos = vNearestHits
							+ vNearestNormal * phmi->rGapRadius ;
					vHitNormal = vNearestHits ;
					rHitDistance = r ;
					fHit = true ;
				}
			}
		}
	}
	return	fHit ;
}

// モデル情報削除
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::DeleteCloth( void )
{
	m_lstMesh.RemoveAll( ) ;
	return	eslErrSuccess ;
}

// モデル状態初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::InitializeCloth( void )
{
	unsigned int	i, nCount ;
	nCount = m_lstMesh.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		Mesh *	pMesh = m_lstMesh.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		pMesh->InitializeMesh( ) ;
	}
	return	eslErrSuccess ;
}

// 当たり判定オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::SetHinderModel
	( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
{
	m_phmiHinder = phmiHinder ;
	m_nHinderCount = nCount ;
	return	eslErrSuccess ;
}

// 布メッシュ追加
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::WeaveCloth
	( const EGL_CLOTH_ATTRIBUTE * pAttribute,
		PCE3D_PRIMITIVE_POLYGON pMeshPrimitive,
		const REAL32 * pVertexApply,
		unsigned int iApplyFirst, unsigned int nApplyCount )
{
	Mesh *	pMesh = new Mesh ;
	ESLError	err ;
	err = pMesh->AttachMesh( pAttribute, pMeshPrimitive ) ;
	if ( !err )
	{
		err = pMesh->CreateMesh( pVertexApply, iApplyFirst, nApplyCount ) ;
	}
	if ( err )
	{
		delete	pMesh ;
		return	err ;
	}
	m_lstMesh.Add( pMesh ) ;
	return	err ;
}

// 布メッシュ追加
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::PatchCloth
	( const EGL_CLOTH_ATTRIBUTE * pAttribute,
		PCE3D_PRIMITIVE_POLYGON pMeshPrimitive, REAL32 rPathRadius,
		const REAL32 * pVertexApply,
		unsigned int iApplyFirst, unsigned int nApplyCount )
{
	Mesh *		pPatchMesh = new Mesh ;
	ESLError	err ;
	err = pPatchMesh->AttachMesh( pAttribute, pMeshPrimitive ) ;
	if ( err )
	{
		delete	pPatchMesh ;
		return	err ;
	}
	EStreamBuffer	bufApply ;
	PE3D_VECTOR4	pVertices = pMeshPrimitive->mesh.vertices ;
	DWORD			dwVertexCount = pMeshPrimitive->dwVertexCount ;
	REAL32 *		pApply =
		(REAL32*) bufApply.PutBuffer( dwVertexCount * sizeof(REAL32) ) ;
	//
	for ( DWORD i = 0; i < dwVertexCount; i ++ )
	{
		if ( (iApplyFirst <= i) && (i < iApplyFirst + nApplyCount) )
		{
			if ( *((SDWORD*)(pVertexApply + (i - iApplyFirst))) >= 0x3F7FF000 )
			{
				pApply[i] = 1 ;
				continue ;
			}
		}
		E3D_VECTOR		vPos = pVertices[i] ;
		Vertex *		pAboveVertex = NULL ;
		REAL32			rMinDistance = rPathRadius ;
		//
		for ( unsigned int j = 0; j < m_lstMesh.GetSize(); j ++ )
		{
			Mesh *	pMeshEntry = m_lstMesh.GetAt( j ) ;
			ESLAssert( pMeshEntry != NULL ) ;
			ESLAssert( pMeshEntry->m_pMesh != NULL ) ;
			pAboveVertex =
				pMeshEntry->FindRedundantVertex( vPos, rMinDistance ) ;
		}
		if ( pAboveVertex != NULL )
		{
			pPatchMesh->AddVertex( pAboveVertex, i, 1 ) ;
			pApply[i] = 1 ;
		}
		else
		{
			if ( (iApplyFirst <= i) && (i < iApplyFirst + nApplyCount) )
			{
				pApply[i] = pVertexApply[i - iApplyFirst] ;
			}
			else
			{
				pApply[i] = 0 ;
			}
		}
	}
	err = pPatchMesh->CreateMesh( pApply, 0, dwVertexCount ) ;
	if ( err )
	{
		delete	pPatchMesh ;
		return	err ;
	}
	m_lstMesh.Add( pPatchMesh ) ;
	return	err ;
}

// 演算実行
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::MorphCloth
	( const EGL_CLOTH_MORPH_PARAMETER * pcmp )
{
	if ( pcmp->rDeltaTime < 0 )
	{
		return	eslErrFailed ;
	}
	//
	// 風速揺らぎ処理
	//
	UpdateStream( pcmp ) ;
	//
	// メッシュ演算
	//
	int	i, nCount = m_lstMesh.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		Mesh *	pMesh = m_lstMesh.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( pMesh != NULL )
		{
			pMesh->MorphCloth( pcmp, *this ) ;
		}
	}
	return	eslErrSuccess ;
}

ESLError EGLClothModelMorphObject::MorphMesh
	( const EGL_CLOTH_MORPH_PARAMETER * pcmp )
{
	if ( pcmp->rDeltaTime < 0 )
	{
		return	eslErrFailed ;
	}
	unsigned int	i, nCount ;
	nCount = m_lstMesh.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		Mesh *	pMesh = m_lstMesh.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( pMesh != NULL )
		{
			pMesh->MorphMesh( pcmp, *this ) ;
		}
	}
	return	eslErrSuccess ;
}

// 関数呼び出しインターフェース
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Call_Release( HEGL_CLOTH_MODEL_MORPH hCloth )
{
	if ( hCloth != NULL )
	{
		delete	PtrFromHandle(hCloth) ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

ESLError EGLClothModelMorphObject::Call_DeleteCloth( HEGL_CLOTH_MODEL_MORPH hCloth )
{
	return	PtrFromHandle(hCloth)->DeleteCloth( ) ;
}

ESLError EGLClothModelMorphObject::Call_InitializeCloth
	( HEGL_CLOTH_MODEL_MORPH hCloth )
{
	return	PtrFromHandle(hCloth)->InitializeCloth( ) ;
}

ESLError EGLClothModelMorphObject::Call_SetHinderModel
	( HEGL_CLOTH_MODEL_MORPH hCloth,
		const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
{
	return	PtrFromHandle(hCloth)->SetHinderModel( phmiHinder, nCount ) ;
}

ESLError EGLClothModelMorphObject::Call_WeaveCloth
	( HEGL_CLOTH_MODEL_MORPH hCloth,
		const EGL_CLOTH_ATTRIBUTE * pAttribute,
		PCE3D_PRIMITIVE_POLYGON pMesh,
		const REAL32 * pVertexApply,
		unsigned int iApplyFirst, unsigned int nApplyCount )
{
	return	PtrFromHandle(hCloth)->WeaveCloth
		( pAttribute, pMesh, pVertexApply, iApplyFirst, nApplyCount ) ;
}

ESLError EGLClothModelMorphObject::Call_PatchCloth
	( HEGL_CLOTH_MODEL_MORPH hCloth,
		const EGL_CLOTH_ATTRIBUTE * pAttribute,
		PCE3D_PRIMITIVE_POLYGON pMesh, REAL32 rPathRadius,
		const REAL32 * pVertexApply,
		unsigned int iApplyFirst, unsigned int nApplyCount )
{
	return	PtrFromHandle(hCloth)->PatchCloth
		( pAttribute, pMesh, rPathRadius,
			pVertexApply, iApplyFirst, nApplyCount ) ;
}

ESLError EGLClothModelMorphObject::Call_MorphCloth
	( HEGL_CLOTH_MODEL_MORPH hCloth,
		const EGL_CLOTH_MORPH_PARAMETER * pcmp )
{
	return	PtrFromHandle(hCloth)->MorphCloth( pcmp ) ;
}

ESLError EGLClothModelMorphObject::Call_MorphMesh
	( HEGL_CLOTH_MODEL_MORPH hCloth,
		const EGL_CLOTH_MORPH_PARAMETER * pcmp )
{
	return	PtrFromHandle(hCloth)->MorphCloth( pcmp ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EGLClothModelMorphObject::Mesh オブジェクト
//////////////////////////////////////////////////////////////////////////////

// EGLClothModelMorph::Mesh 構築
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::Mesh::Mesh( void )
{
	m_pClothAttr = NULL ;
	m_pMesh = NULL ;
	m_pVertices = NULL ;
	m_nVertices = 0 ;
}

// EGLClothModelMorph::Mesh 消滅
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::Mesh::~Mesh( void )
{
	if ( m_pVertices != NULL )
	{
		::eslHeapFree( NULL, m_pVertices, 0 ) ;
	}
}

// メッシュ設定
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::AttachMesh
	( const EGL_CLOTH_ATTRIBUTE * pAttribute,
				PCE3D_PRIMITIVE_POLYGON pMesh )
{
	ESLAssert( pAttribute != NULL ) ;
	ESLAssert( pMesh != NULL ) ;
	if ( (pAttribute == NULL) || (pMesh == NULL) )
	{
		return	eslErrGeneral ;
	}
	if ( !(pMesh->dwTypeFlag & E3D_MESH_POLYGON) )
	{
		return	eslErrGeneral ;
	}
	//
	m_pClothAttr = pAttribute ;
	m_pMesh = pMesh ;
	//
	ESLAssert( m_pVertices == NULL ) ;
	m_pVertices =
		(Vertex*) ::eslHeapAllocate
			( NULL, m_pMesh->dwVertexCount * sizeof(Vertex), 0 ) ;
	m_nVertices = 0 ;
	//
	return	eslErrSuccess ;
}

// メッシュ状態初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::InitializeMesh( void )
{
	for ( unsigned int i = 0; i < m_nVertices; i ++ )
	{
		Vertex *	pVertex = m_pVertices + i ;
		pVertex->vMomentum.x = 0 ;
		pVertex->vMomentum.y = 0 ;
		pVertex->vMomentum.z = 0 ;
		pVertex->vMomentum.d = 0 ;
	}
	return	eslErrSuccess ;
}

// メッシュ構築
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::CreateMesh
	( const REAL32 * pVertexApply,
		unsigned int iApplyFirst, unsigned int nApplyCount )
{
	ESLAssert( m_pMesh != NULL ) ;
	if ( m_pMesh == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 物理演算対象とならない頂点数をセットアップ
	//
	unsigned int	i ;
	const DWORD		dwVertexCount = m_pMesh->dwVertexCount ;
	if ( iApplyFirst < dwVertexCount )
	{
		if ( iApplyFirst + nApplyCount > dwVertexCount )
		{
			nApplyCount = dwVertexCount - iApplyFirst ;
		}
		for ( i = 0; i < nApplyCount; i ++ )
		{
			if ( *((SDWORD*)(pVertexApply + i)) >= 0x3F7FF000 )
			{
				if ( FindVertex( i + iApplyFirst ) < 0 )
				{
					AddVertex( NULL, i + iApplyFirst, 0 ) ;
				}
			}
		}
	}
	if ( m_nVertices == 0 )
	{
		return	eslErrGeneral ;
	}
	//
	// 頂点情報を順次構築
	//
	unsigned int	nProcessed = 0 ;
	while ( nProcessed < dwVertexCount )
	{
		EObjArray<NewVertexInfo>	lstNewVertex ;
		unsigned int	nLastCount = m_nVertices ;
		for ( i = nProcessed; i < nLastCount; i ++ )
		{
			int	iVertex = m_pVertices[i].iVertex ;
			ESLAssert( m_pMesh != NULL ) ;
			E3D_PRIMITIVE_MESH_LIST *
				pMeshList = (E3D_PRIMITIVE_MESH_LIST*)
					&(m_pMesh->mesh.uv_map[m_pMesh->dwVertexCount]) ;
			E3D_PRIMITIVE_MESH_POLY *	ppmpNext = pMeshList->mpEntries ;
			for ( unsigned int j = 0; j < pMeshList->dwPolyCount; j ++ )
			{
				unsigned int	k, nCount = ppmpNext->dwVertexCount ;
				for ( k = 0; k < nCount; k ++ )
				{
					if ( (int) ppmpNext->dwIndex[k] == iVertex )
					{
						break;
					}
				}
				if ( k < nCount )
				{
					for ( k = 0; k < nCount; k ++ )
					{
						int	nIndex = ppmpNext->dwIndex[k] ;
						if ( (nIndex != iVertex)
							&& (FindVertex( nIndex ) < 0) )
						{
							AddListNewVertexInfo
								( lstNewVertex, m_pVertices + i, nIndex ) ;
						}
					}
				}
				ppmpNext =
					(E3D_PRIMITIVE_MESH_POLY*)
						&(ppmpNext->dwIndex[nCount]) ;
			}
		}
		unsigned int	nCount = lstNewVertex.GetSize( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			NewVertexInfo *	pnvi = lstNewVertex.GetAt( i ) ;
			ESLAssert( pnvi != NULL ) ;
			REAL32	rMorphWeight = 1 ;
			int		nIndex = pnvi->iVertex ;
			nIndex -= iApplyFirst ;
			if ( (nIndex >= 0)
				&& (nIndex < (int) (nApplyCount - iApplyFirst)) )
			{
				if ( *((SDWORD*)(pVertexApply + nIndex)) >= 0x3F7FF000 )
				{
					continue ;
				}
				rMorphWeight =
					(REAL32) (1 - pVertexApply[nIndex]) ;
			}
			REAL32		rRedundantRadius = 1.0e-3f ;
			Vertex *	pRedundant =
				FindRedundantVertex
					( m_pMesh->mesh.vertices[nIndex], rRedundantRadius ) ;
			if ( pRedundant != NULL )
			{
				AddVertex( pRedundant, nIndex, rMorphWeight ) ;
			}
			else
			{
				AddVertex
					( FindNearestVertex
						( pnvi->pAboveVertex,
								nIndex, nLastCount ), nIndex, rMorphWeight ) ;
			}
		}
		nProcessed = nLastCount ;
		if ( m_nVertices == nLastCount )
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// 演算実行
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::MorphCloth
	( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
				const EGLClothModelMorphObject & cmm )
{
	if ( pcmp->rDeltaTime < 1.0e-3 )
	{
		//
		// 1ms 以下の場合はモーフィング処理はしない
		//////////////////////////////////////////////////////////////////////
		unsigned int	i ;
		for ( i = 0; i < m_nVertices; i ++ )
		{
			Vertex *				pVertex = m_pVertices + i ;
			PCE3D_PRIMITIVE_POLYGON	pMesh = pVertex->pMesh ;
			pMesh->mesh.vertices[pVertex->iVertex] = pVertex->vPosition ;
			pMesh->mesh.normals[pVertex->iVertex] = pVertex->vNormal ;
		}
		return	eslErrSuccess ;
	}
	//
	// パラメータ正規化
	//////////////////////////////////////////////////////////////////////////
	double	rSoftness = m_pClothAttr->rSoftness ;
	double	rElastically = m_pClothAttr->rElastically ;
	if ( rSoftness < 0 )
	{
		rSoftness = 0 ;
	}
	else if ( rSoftness > 1 )
	{
		rSoftness = 1 ;
	}
	if ( rElastically < 0 )
	{
		rElastically = 0 ;
	}
	else if ( rElastically > 1 )
	{
		rElastically = 1 ;
	}
	double	rDeltaSoftness = rSoftness ;				// 柔らかさ（曲がりに対する）
	double	rDeltaElastically = rElastically ;			// 伸縮性（伸びに対する）
	double	rDeltaDamping =
				pow( m_pClothAttr->rDamping, pcmp->rDeltaTime ) ;	// 速度の減衰係数（1.0で無減衰）
	if ( rDeltaDamping >= 0.999f )
	{
		rDeltaDamping = 0.999f ;
	}
	double	rDeltaWindEffect = 0 ;
	if ( m_pClothAttr->rWeight > 0 )
	{
		rDeltaWindEffect =
			pcmp->rDensity / m_pClothAttr->rWeight * pcmp->rDeltaTime ;
	}
	//
	// 変形・基本物理演算
	//
	unsigned int	i, j, nCount ;
	for ( i = 0; i < m_nVertices; i ++ )
	{
		Vertex *				pVertex = m_pVertices + i ;
		PCE3D_PRIMITIVE_POLYGON	pMesh = pVertex->pMesh ;
		PE3D_VECTOR4	pPosition = pMesh->mesh.vertices + pVertex->iVertex ;
		PE3D_VECTOR4	pNormal = pMesh->mesh.normals + pVertex->iVertex ;
		Vertex *		pAboveVertex = pVertex->pAboveVertex ;
		if ( pAboveVertex == NULL )
		{
			pVertex->vPosition = *pPosition ;
			pVertex->vNormal = *pNormal ;
			continue ;
		}
		//
		// 現在の運動量を座標に反映
		//
		E3DVector4	vPos = pVertex->vPosition ;
		vPos += pVertex->vMomentum
				* (rDeltaDamping * pcmp->rDeltaTime) + pcmp->vMoveDelta ;
		//
		// 曲がりに対する効果
		//
		E3DVector4	vAbovePos = pAboveVertex->vPosition ;
		E3DVector4	vRevDistance = pVertex->vDistance ;
		E3DVector	vDelta = vPos - vAbovePos ;
		pcmp->pBaseMatrix->RevolveVector( vRevDistance ) ;
		vDelta = vDelta * rDeltaSoftness
					+ vRevDistance * (1.0 - rDeltaSoftness) ;
		//
		// 伸縮に対する効果
		//
		const REAL32	rOrgDistance = pVertex->vDistance.Absolute( ) ;
		const REAL32	rDistance = vDelta.Absolute( ) ;
		if ( rOrgDistance < 1.0e-3 )
		{
			pVertex->vPosition = vAbovePos ;
			*pPosition = vAbovePos ;
			pVertex->vNormal = pAboveVertex->vNormal ;
			*pNormal = pAboveVertex->vNormal ;
			continue ;
		}
		if ( pVertex->pMesh == pAboveVertex->pMesh )
		{
			double	rNewDistance =
				rOrgDistance
					- (rOrgDistance - rDistance) * rDeltaElastically ;
			if ( rDistance > 0 )
			{
				vPos = vAbovePos + vDelta * (rNewDistance / rDistance) ;
			}
			else
			{
				vPos = vAbovePos ;
			}
		}
		else
		{
			//
			// 別メッシュからの結合点
			//
			pVertex->vPosition = vAbovePos ;
			*pPosition = vAbovePos ;
			pVertex->vNormal = pAboveVertex->vNormal ;
			*pNormal = pAboveVertex->vNormal ;
			continue ;
		}
		//
		// 風による効果
		//
		if ( !(pcmp->dwFlags & CMP_FLAG_NO_STREAM_EFFECT) )
		{
			REAL32	rStream = cmm.m_vStream.Absolute( ) ;
			if ( rStream > 0 )
			{
				REAL32	rStreamEffect =
					cmm.m_vStream.InnerProduct( pVertex->vNormal ) / rStream ;
				vPos += cmm.m_vStream
							* (fabs(rStreamEffect) * rDeltaWindEffect)
					+ pVertex->vNormal * (rStreamEffect * rDeltaWindEffect) ;
			}
		}
		//
		// 当たり判定
		//
		E3D_VECTOR	vHitPos, vHitNormal ;
		if ( cmm.TestCollisionSegment
			( pVertex->vPosition, vPos, vHitPos, vHitNormal ) )
		{
			vPos = vHitPos ;
			//
			REAL32	r = pVertex->vMomentum.InnerProduct( vHitNormal ) ;
			if ( r <= 0.0 )
			{
				pVertex->vMomentum -= vHitNormal * r ;
			}
		}
		else if ( cmm.TestCollisionSphere( vPos, vHitPos, vHitNormal ) )
		{
			vPos = vHitPos ;
			//
			REAL32	r = pVertex->vMomentum.InnerProduct( vHitNormal ) ;
			if ( r <= 0.0 )
			{
				pVertex->vMomentum -= vHitNormal * r ;
			}
		}
		//
		// 運動量反映
		//
		pVertex->vMomentum =
			(vPos - pVertex->vPosition) / pcmp->rDeltaTime
						+ pcmp->vGravity * pcmp->rDeltaTime ;
		//
		// 座標確定
		//
		pVertex->vPosition = vPos ;
		if ( *((DWORD*)&(pVertex->rMorphWeight)) >= 0x3F7FF000 )
		{
			*pPosition = vPos ;
		}
		else
		{
			E3DVector4	vTempPos = pVertex->vDistance ;
			pcmp->pBaseMatrix->RevolveVector( vTempPos ) ;
			vTempPos += vAbovePos ;
			*pPosition = vTempPos ;
			*pPosition +=
				(pVertex->vPosition - vTempPos) * pVertex->rMorphWeight ;
		}
		//
		// 法線確定
		//
		PCE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
		if ( pVertex->nPolygons > 0 )
		{
			unsigned int *	pPolygons = pVertex->pPolygons ;
			unsigned int	nPolygons = pVertex->nPolygons ;
			pVertex->vNormal.x = 0 ;
			pVertex->vNormal.y = 0 ;
			pVertex->vNormal.z = 0 ;
			pVertex->vNormal.d = 0 ;
			for ( j = 0; j < nPolygons; j ++ )
			{
				unsigned int	p1 = pPolygons[0] ;
				unsigned int	p2 = pPolygons[1] ;
				pPolygons += 2 ;
				E3DVector	vNormal =
					(pVertices[p1] - vPos) * (pVertices[p2] - vPos) ;
				vNormal.Normalize( ) ;
				pVertex->vNormal += vNormal ;
			}
			pVertex->vNormal.Normalize( ) ;
			*pNormal = pVertex->vNormal ;
		}
		else
		{
			pVertex->vNormal = *pNormal ;
		}
	}
	//
	// メッシュの伸縮効果
	//
	PE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
	ESLAssert( pcmp->rDeltaTime > 0 ) ;
	rDeltaSoftness = (1 - rSoftness) ;
	for ( i = 0; i < m_nVertices; i ++ )
	{
		Vertex *			pVertex = m_pVertices + i ;
		VertexRelation *	pvr = pVertex->pRelations ;
		nCount = pVertex->nRelations ;
		for ( j = 0; j < nCount; j ++, pvr ++ )
		{
			E3DVector	vDelta =
				pVertices[pvr->iRelation] - pVertex->vPosition ;
			REAL32	rDistance = vDelta.Absolute( ) ;
			if ( rDistance > 0 )
			{
				pVertex->vMomentum +=
					vDelta * (((rDistance - pvr->rLength)
							* rDeltaSoftness / rDistance) / pcmp->rDeltaTime) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 座標変換（クロスシミュレーション無し）
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::MorphMesh
	( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
				const EGLClothModelMorphObject & cmm )
{
	unsigned int	i ;
	E3DVector4		vZero( 0, 0, 0, 0 ) ;
	PE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
	for ( i = 0; i < m_nVertices; i ++ )
	{
		Vertex *		pVertex = m_pVertices + i ;
		PCE3D_PRIMITIVE_POLYGON	pMesh = pVertex->pMesh ;
		PE3D_VECTOR4	pPosition = pVertices + pVertex->iVertex ;
		PE3D_VECTOR4	pNormal = pMesh->mesh.normals + pVertex->iVertex ;
		Vertex *		pAboveVertex = pVertex->pAboveVertex ;
		if ( pAboveVertex == NULL )
		{
			pVertex->vPosition = *pPosition ;
			pVertex->vNormal = *pNormal ;
			continue ;
		}
		//
		E3DVector4	vPos = pVertex->vDistance ;
		pcmp->pBaseMatrix->RevolveVector( vPos ) ;
		vPos += pAboveVertex->vPosition ;
		//
		if ( *((DWORD*)&(pVertex->rMorphWeight)) < 0x3F7FF000 )
		{
			vPos +=
				(pVertex->vPosition - vPos) * pVertex->rMorphWeight ;
		}
		pVertex->vPosition = vPos ;
		pVertex->vMomentum = vZero ;
		*pPosition = vPos ;
	}
	return	MakeNormal( pcmp, cmm ) ;
}

// 法線計算
//////////////////////////////////////////////////////////////////////////////
ESLError EGLClothModelMorphObject::Mesh::MakeNormal
	( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
			const EGLClothModelMorphObject & cmm )
{
	unsigned int	i, j ;
	PE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
	for ( i = 0; i < m_nVertices; i ++ )
	{
		Vertex *		pVertex = m_pVertices + i ;
		PCE3D_PRIMITIVE_POLYGON	pMesh = pVertex->pMesh ;
		PE3D_VECTOR4	pNormal = pMesh->mesh.normals + pVertex->iVertex ;
		Vertex *		pAboveVertex = pVertex->pAboveVertex ;
		if ( pAboveVertex == NULL )
		{
			continue ;
		}
		E3DVector4	vPos = pVertex->vPosition ;
		if ( pVertex->nPolygons > 0 )
		{
			unsigned int *	pPolygons = pVertex->pPolygons ;
			unsigned int	nPolygons = pVertex->nPolygons ;
			pVertex->vNormal.x = 0 ;
			pVertex->vNormal.y = 0 ;
			pVertex->vNormal.z = 0 ;
			pVertex->vNormal.d = 0 ;
			for ( j = 0; j < nPolygons; j ++ )
			{
				unsigned int	p1 = pPolygons[0] ;
				unsigned int	p2 = pPolygons[1] ;
				pPolygons += 2 ;
				E3DVector	vNormal =
					(pVertices[p1] - vPos) * (pVertices[p2] - vPos) ;
				vNormal.Normalize( ) ;
				pVertex->vNormal += vNormal ;
			}
			pVertex->vNormal.Normalize( ) ;
			*pNormal = pVertex->vNormal ;
		}
	}
	return	eslErrSuccess ;
}

// 頂点追加
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::Vertex *
	EGLClothModelMorphObject::Mesh::AddVertex
		( EGLClothModelMorphObject::Vertex * pAboveVertex,
						unsigned int iVertex, REAL32 rWeight )
{
	ESLAssert( m_pVertices != NULL ) ;
	ESLAssert( m_nVertices < m_pMesh->dwVertexCount ) ;
	ESLAssert( iVertex < m_pMesh->dwVertexCount ) ;
	if ( (m_nVertices >= m_pMesh->dwVertexCount)
			|| (iVertex >= m_pMesh->dwVertexCount) )
	{
		return	NULL ;
	}
	//
	// 初期パラメータ設定
	//
	Vertex *	pVertex = m_pVertices + (m_nVertices ++) ;
	pVertex->vMomentum.x = 0 ;
	pVertex->vMomentum.y = 0 ;
	pVertex->vMomentum.z = 0 ;
	pVertex->vMomentum.d = 0 ;
	pVertex->vPosition = m_pMesh->mesh.vertices[iVertex] ;
	pVertex->vNormal = m_pMesh->mesh.normals[iVertex] ;
//	pVertex->vOrgNormal = m_pMesh->mesh.normals[iVertex] ;
	pVertex->iVertex = iVertex ;
	pVertex->pAboveVertex = pAboveVertex ;
	pVertex->pMesh = m_pMesh ;
	pVertex->rMorphWeight = rWeight ;
	pVertex->pPolygons = NULL ;
	pVertex->nPolygons = 0 ;
	pVertex->pRelations = NULL ;
	pVertex->nRelations = 0 ;
	//
	if ( pAboveVertex != NULL )
	{
		pVertex->vDistance =
			pVertex->vPosition - pAboveVertex->vPosition ;
		if ( (m_pMesh == pAboveVertex->pMesh)
			&& (pAboveVertex->pAboveVertex != NULL)
			&& (pVertex->vDistance.Absolute() < 1.0e-5) )
		{
//			pVertex->pAboveVertex = pAboveVertex->pAboveVertex ;
//			pVertex->vDistance =
//				pVertex->vPosition - pVertex->pAboveVertex->vPosition ;
		}
	}
	//
	// ポリゴンリストを作成
	//
	if ( rWeight != 0 )
	{
		CreatePolygonList( pVertex ) ;
	}
	//
	return	pVertex ;
}

// 既にエントリされている頂点の中で指定距離以下の最近頂点を検索する
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::Vertex *
	EGLClothModelMorphObject::Mesh::FindRedundantVertex
		( const E3D_VECTOR & vPos, REAL32 & rRedundantRadius )
{
	unsigned int	i, nCount = m_nVertices ;
	Vertex *		pTargetVertices = m_pVertices ;
	Vertex *		pRedundantVertex = NULL ;
	for ( i = 0; i < nCount; i ++ )
	{
		REAL32	rDistance =
			(pTargetVertices[i].vPosition - vPos).Absolute( ) ;
		if ( rDistance < rRedundantRadius )
		{
			pRedundantVertex = pTargetVertices + i ;
			rRedundantRadius = rDistance ;
		}
	}
	return	pRedundantVertex ;
}

// すでにエントリされている頂点の中で
// iVertex 指定頂点を含むポリゴンに属する最近頂点を検索する
//////////////////////////////////////////////////////////////////////////////
EGLClothModelMorphObject::Vertex *
	EGLClothModelMorphObject::Mesh::FindNearestVertex
		( EGLClothModelMorphObject::Vertex * pReference,
			unsigned int iVertex, unsigned int nScanLimit )
{
	ESLAssert( m_pMesh != NULL ) ;
	E3D_PRIMITIVE_MESH_LIST *
		pMeshList = (E3D_PRIMITIVE_MESH_LIST*)
			&(m_pMesh->mesh.uv_map[m_pMesh->dwVertexCount]) ;
	E3D_PRIMITIVE_MESH_POLY *	ppmpNext = pMeshList->mpEntries ;
	//
	PE3D_VECTOR	pVertices = m_pMesh->mesh.vertices ;
	E3DVector	vPos = pVertices[iVertex] ;
	Vertex *	pNearVertex = pReference ;
	REAL32		rMinDistance = (vPos - pReference->vPosition).Absolute( ) ;
	//
	for ( unsigned int j = 0; j < pMeshList->dwPolyCount; j ++ )
	{
		unsigned int	k, nCount = ppmpNext->dwVertexCount ;
		for ( k = 0; k < nCount; k ++ )
		{
			if ( (int) ppmpNext->dwIndex[k] == iVertex )
			{
				break;
			}
		}
		if ( k < nCount )
		{
			for ( k = 0; k < nCount; k ++ )
			{
				int	nIndex = ppmpNext->dwIndex[k] ;
				int	iFindVertex = FindVertex( nIndex ) ;
				if ( (nIndex != (int) iVertex)
					&& (iFindVertex >= 0) && (iFindVertex < (int) nScanLimit) )
				{
					REAL32	rDistance =
						(vPos - pVertices[nIndex]).Absolute( ) ;
					if ( rDistance < rMinDistance )
					{
						pNearVertex = m_pVertices + iFindVertex ;
						rMinDistance = rDistance ;
					}
				}
			}
		}
		ppmpNext =
			(E3D_PRIMITIVE_MESH_POLY*) &(ppmpNext->dwIndex[nCount]) ;
	}
	//
	return	pNearVertex ;
}

// すでにエントリされている指定の頂点を検索する
//////////////////////////////////////////////////////////////////////////////
int EGLClothModelMorphObject::Mesh::FindVertex( unsigned int iVertex )
{
	for ( unsigned int i = 0; i < m_nVertices; i ++ )
	{
		if ( m_pVertices[i].iVertex == iVertex )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// 追加する頂点情報をリストに追加する
//////////////////////////////////////////////////////////////////////////////
void EGLClothModelMorphObject::Mesh::AddListNewVertexInfo
	( EObjArray<EGLClothModelMorphObject::NewVertexInfo> & list,
						Vertex * pAboveVertex, unsigned int iVertex )
{
	for ( unsigned int i = 0; i < list.GetSize(); i ++ )
	{
		NewVertexInfo *	pnvi = list.GetAt( i ) ;
		ESLAssert( pnvi != NULL ) ;
		if ( pnvi->iVertex == iVertex )
		{
			pnvi->nRefCount ++ ;
			for ( int j = i - 1; j >= 0; j -- )
			{
				NewVertexInfo *	pnviAbove = list.GetAt( j ) ;
				ESLAssert( list.GetAt( j + 1 ) == pnvi ) ;
				if ( pnviAbove->nRefCount >= pnvi->nRefCount )
				{
					break ;
				}
				list.Swap( j, j + 1 ) ;
				ESLAssert( list.GetAt( j ) == pnvi ) ;
				ESLAssert( list.GetAt( j + 1 ) == pnviAbove ) ;
			}
			return ;
		}
	}
	NewVertexInfo *	pnvi = new NewVertexInfo ;
	pnvi->pAboveVertex = pAboveVertex ;
	pnvi->iVertex = iVertex ;
	pnvi->nRefCount = 1 ;
	list.Add( pnvi ) ;
}

// 指定頂点を含むポリゴンリストを作成
//////////////////////////////////////////////////////////////////////////////
void EGLClothModelMorphObject::Mesh::CreatePolygonList( Vertex * pNewVertex )
{
	ESLAssert( m_pMesh != NULL ) ;
	E3D_PRIMITIVE_MESH_LIST *
		pMeshList = (E3D_PRIMITIVE_MESH_LIST*)
			&(m_pMesh->mesh.uv_map[m_pMesh->dwVertexCount]) ;
	unsigned int	nPolygons = 0 ;
	unsigned int	i ;
	EPtrObjArray<E3D_PRIMITIVE_MESH_POLY>	lstPloygon ;
	EPtrObjArray<E3D_PRIMITIVE_MESH_POLY>	lstRelation ;
	E3D_PRIMITIVE_MESH_POLY *	ppmpNext = pMeshList->mpEntries ;
	//
	// 指定頂点を含むポリゴンをリストアップ
	// （既存頂点のみで構成されるポリゴンに限定）
	//
	for ( i = 0; i < pMeshList->dwPolyCount; i ++ )
	{
		DWORD	dwCount = ppmpNext->dwVertexCount ;
		for ( unsigned int j = 0; j < dwCount; j ++ )
		{
			if ( pNewVertex->iVertex == ppmpNext->dwIndex[j] )
			{
				unsigned int k ;
				for ( k = 0; k < dwCount; k ++ )
				{
					if ( (k != j)
						&& (FindVertex( ppmpNext->dwIndex[k] ) < 0) )
					{
						break ;
					}
				}
				if ( k == dwCount )
				{
					lstPloygon.Add( ppmpNext ) ;
				}
				lstRelation.Add( ppmpNext ) ;
				break ;
			}
		}
		ppmpNext =
			(E3D_PRIMITIVE_MESH_POLY*)
				&(ppmpNext->dwIndex[ppmpNext->dwVertexCount]) ;
	}
	//
	pNewVertex->nPolygons = lstPloygon.GetSize( ) ;
	if ( pNewVertex->nPolygons > 0 )
	{
		//
		// 法線パラメータを生成
		//
		PE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
		PE3D_VECTOR4	pNormals = m_pMesh->mesh.normals ;
		unsigned int	nBytes =
			pNewVertex->nPolygons * (2 * sizeof(unsigned int)) ;
		unsigned int *	pPolygons =
			(unsigned int *) m_bufPolygonList.PutBuffer( nBytes ) ;
		pNewVertex->pPolygons = pPolygons ;
		//
//		E3DVector4	vOrgNormal( 0, 0, 0, 0 ) ;
		//
		for ( i = 0; i < pNewVertex->nPolygons; i ++ )
		{
			unsigned int	j ;
			E3D_PRIMITIVE_MESH_POLY *	ppmp = lstPloygon.GetAt( i ) ;
			ESLAssert( ppmp != NULL ) ;
			DWORD	dwCount = ppmp->dwVertexCount ;
			unsigned int	iVertex = pNewVertex->iVertex ;
			for ( j = 0; j < dwCount; j ++ )
			{
				if ( ppmp->dwIndex[j] == iVertex )
				{
					break ;
				}
			}
			ESLAssert( j < ppmp->dwVertexCount ) ;
			//
			unsigned int	ip1 = (j > 0) ? (j - 1) : (dwCount - 1) ;
			unsigned int	ip2 = (j + 1 < dwCount) ? (j + 1) : 0 ;
			unsigned int	p1 = ppmp->dwIndex[ip1] ;
			unsigned int	p2 = ppmp->dwIndex[ip2] ;
			E3D_VECTOR	vNormal =
				(pVertices[p1] - pVertices[iVertex])
					* (pVertices[p2] - pVertices[iVertex]) ;
			if ( vNormal.InnerProduct( pNormals[iVertex] ) >= 0 )
			{
				pPolygons[0] = p1 ;
				pPolygons[1] = p2 ;
				//
//				vNormal.Normalize( ) ;
//				vOrgNormal += vNormal ;
			}
			else
			{
				pPolygons[0] = p2 ;
				pPolygons[1] = p1 ;
				//
//				vNormal.Normalize( ) ;
//				vOrgNormal -= vNormal ;
			}
			pPolygons += 2 ;
		}
		m_bufPolygonList.Flush( nBytes ) ;
		//
//		vOrgNormal.Normalize( ) ;
//		pNewVertex->vOrgNormal = vOrgNormal ;
	}
	else
	{
		pNewVertex->pPolygons = NULL ;
	}
	//
	// 頂点連結リスト
	//
	ENumArray<UINT>	lstRelationIndex ;
	unsigned int	iAboveVertex = -1 ;
	if ( pNewVertex->pAboveVertex != NULL )
	{
		if ( pNewVertex->pMesh == pNewVertex->pAboveVertex->pMesh )
		{
			iAboveVertex = pNewVertex->pAboveVertex->iVertex ;
		}
	}
	for ( i = 0; i < lstRelation.GetSize(); i ++ )
	{
		unsigned int	j ;
		E3D_PRIMITIVE_MESH_POLY *	ppmp = lstRelation.GetAt( i ) ;
		ESLAssert( ppmp != NULL ) ;
		DWORD	dwCount = ppmp->dwVertexCount ;
		unsigned int	iVertex = pNewVertex->iVertex ;
		for ( j = 0; j < dwCount; j ++ )
		{
			if ( ppmp->dwIndex[j] == iVertex )
			{
				break ;
			}
		}
		ESLAssert( j < ppmp->dwVertexCount ) ;
		//
		unsigned int	nIndex =
			(j > 0) ? ppmp->dwIndex[j - 1] : ppmp->dwIndex[dwCount - 1] ;
		if ( (nIndex != iVertex)
			&& (nIndex != iAboveVertex)
			&& (lstRelationIndex.Find( nIndex ) < 0) )
		{
			lstRelationIndex.Add( nIndex ) ;
		}
		nIndex = (j < dwCount - 1) ? ppmp->dwIndex[j + 1] : ppmp->dwIndex[0] ;
		if ( (nIndex != iVertex)
			&& (nIndex != iAboveVertex)
			&& (lstRelationIndex.Find( nIndex ) < 0) )
		{
			lstRelationIndex.Add( nIndex ) ;
		}
	}
	pNewVertex->nRelations = lstRelationIndex.GetSize( ) ;
	if ( pNewVertex->nRelations > 0 )
	{
		unsigned int	nBytes =
			pNewVertex->nRelations * sizeof(VertexRelation) ;
		VertexRelation *	pRelations =
			(VertexRelation *) m_bufPolygonList.PutBuffer( nBytes ) ;
		PE3D_VECTOR4	pVertices = m_pMesh->mesh.vertices ;
		E3D_VECTOR4		vVertex = pVertices[pNewVertex->iVertex] ;
		pNewVertex->pRelations = pRelations ;
		for ( i = 0; i < pNewVertex->nRelations; i ++ )
		{
			UINT	nIndex = lstRelationIndex.GetAt( i ) ;
			pRelations[i].iRelation = nIndex ;
			pRelations[i].rLength = (pVertices[nIndex] - vVertex).Absolute( ) ;
		}
		m_bufPolygonList.Flush( nBytes ) ;
	}
	else
	{
		pNewVertex->pRelations = NULL ;
	}
}


