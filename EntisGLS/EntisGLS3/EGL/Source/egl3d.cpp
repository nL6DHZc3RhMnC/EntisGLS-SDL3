
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
       Copyright (c) 2003-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>

static const double	pi_rad = 3.141592653589 / 180.0 ;


//////////////////////////////////////////////////////////////////////////////
// 骨組み込みモデルオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DBonePolygonModel, E3DPolygonModel )
IMPLEMENT_CLASS_INFO( E3DBonePolygonModel::E3DBoneJoint, E3DModelJoint )
IMPLEMENT_CLASS_INFO( E3DBonePolygonModel::E3DClothMorph, ESLObject )
IMPLEMENT_CLASS_INFO( E3DBonePolygonModel::E3DClothMorphModelSet, EPtrArray )
IMPLEMENT_CLASS_INFO( E3DBonePolygonModel::E3DPoseAnimation, ESLObject )

// E3DBonePolygonModel::E3DBoneJoint 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DBoneJoint::E3DBoneJoint( void )
	: m_vCenter(0,0,0)
{
	m_fVertexApplyAll = false ;
	m_fNormalApplyAll = false ;
	m_iRefVertex = 0 ;
	m_nRefVertexCount = 0 ;
	m_pVertexApply = NULL ;
	m_pVertexBuf = NULL ;
	m_iRefNormal = 0 ;
	m_nRefNormalCount = 0 ;
	m_pNormalApply = NULL ;
	m_pNormalBuf = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DBoneJoint::~E3DBoneJoint( void )
{
	DeleteContents( ) ;
}

// ジョイント生成
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint * E3DBonePolygonModel::E3DBoneJoint::CreateJoint( void ) const
{
	return	new E3DBonePolygonModel::E3DBoneJoint ;
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::RefreshJoint( void )
{
	unsigned int	i ;
	for ( i = 0; i < JointList().GetSize(); i ++ )
	{
		ESLAssert( JointList().GetAt(i) != NULL ) ;
		ESLAssert( JointList().GetAt(i)->
					IsKindOf( ESL_RUNTIME_CLASS(E3DBonePolygonModel::E3DBoneJoint) ) ) ;
		JointList().GetAt(i)->RefreshJoint( ) ;
	}
	E3DModelJoint::RefreshJoint( ) ;
}

// ジョイント回転処理
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::
		TransformJoint( const E3DModelJoint & mjParent )
{
}

// モデル回転処理
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::TransformModel
						( E3DPolygonModel & model ) const
{
	ESLAssert( model.IsKindOf( ESL_RUNTIME_CLASS(E3DBonePolygonModel) ) ) ;
	unsigned int	i ;
	for ( i = 0; i < JointList().GetSize(); i ++ )
	{
		ESLAssert( JointList().GetAt(i) != NULL ) ;
		ESLAssert( JointList().GetAt(i)->
					IsKindOf( ESL_RUNTIME_CLASS(E3DBonePolygonModel::E3DBoneJoint) ) ) ;
		JointList().GetAt(i)->TransformModel( model ) ;
	}
	if ( m_nRefVertexCount > 0 )
	{
		if ( m_fVertexApplyAll )
		{
			ESLAssert( m_iRefVertex + m_nRefVertexCount <= model.GetVertexCount() ) ;
			PE3D_VECTOR4	pVertexes = model.GetVertexList( m_iRefVertex ) ;
			for ( i = 0; i < m_nRefVertexCount; i ++ )
			{
				*((E3DVector4*)(m_pVertexBuf + i)) = pVertexes[i] - m_vCenter ;
			}
			E3DVector	vMove = m_vmove ;
			vMove += m_vCenter ;
			//
			m_rvmat.RevolveVectors
				( pVertexes, m_pVertexBuf, &vMove, m_nRefVertexCount ) ;
		}
		else
		{
			unsigned int	i, iLast = 0 ;
			for ( i = 0; i < m_nRefVertexCount; i ++ )
			{
				if ( ((DWORD*)m_pVertexApply)[i] == 0 )
				{
					if ( iLast < i )
					{
						TransformVertex
							( model, iLast + m_iRefVertex, i - iLast ) ;
					}
					iLast = i + 1 ;
				}
			}
			if ( iLast < i )
			{
				TransformVertex
					( model, iLast + m_iRefVertex, i - iLast ) ;
			}
		}
	}
	if ( m_nRefNormalCount > 0 )
	{
		if ( m_fNormalApplyAll )
		{
			ESLAssert( m_iRefNormal + m_nRefNormalCount <= model.GetNormalCount() ) ;
			PE3D_VECTOR4	pNormals = model.GetNormalList( m_iRefNormal ) ;
			m_rvmat.RevolveVectors
				( pNormals, pNormals, NULL, m_nRefNormalCount ) ;
		}
		else
		{
			unsigned int	i, iLast = 0 ;
			for ( i = 0; i < m_nRefNormalCount; i ++ )
			{
				if ( ((DWORD*)m_pNormalApply)[i] == 0 )
				{
					if ( iLast < i )
					{
						TransformNormal
							( model, iLast + m_iRefNormal, i - iLast ) ;
					}
					iLast = i + 1 ;
				}
			}
			if ( iLast < i )
			{
				TransformNormal
					( model, iLast + m_iRefNormal, i - iLast ) ;
			}
		}
	}
}

void E3DBonePolygonModel::E3DBoneJoint::TransformVertex
	( E3DPolygonModel & model,
		unsigned int iFirst, unsigned int nCount ) const
{
	ESLAssert( iFirst + nCount <= model.GetVertexCount() ) ;
	ESLAssert( m_iRefVertex <= iFirst ) ;
	ESLAssert( iFirst + nCount <= m_iRefVertex + m_nRefVertexCount ) ;
	PE3D_VECTOR4	pVertexes = model.GetVertexList( iFirst ) ;
	unsigned int	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		*((E3DVector4*)(m_pVertexBuf + i)) = pVertexes[i] - m_vCenter ;
	}
	E3DVector	vMove = m_vmove ;
	vMove += m_vCenter ;
	//
	m_rvmat.RevolveVectors
		( m_pVertexBuf, m_pVertexBuf, &vMove, nCount ) ;
	//
	unsigned int	iOffset = iFirst - m_iRefVertex ;
	for ( i = 0; i < nCount; i ++ )
	{
		REAL32	rApply = m_pVertexApply[i + iOffset] ;
		if ( *((DWORD*)&rApply) != 0 )
		{
			if ( *((SDWORD*)&rApply) < 0x3F7FF000 )
			{
				E3D_VECTOR4	v = m_pVertexBuf[i] ;
				v -= pVertexes[i] ;
				v *= rApply ;
				pVertexes[i] += v ;
			}
			else
			{
				pVertexes[i] = m_pVertexBuf[i] ;
			}
		}
	}
}

void E3DBonePolygonModel::E3DBoneJoint::TransformNormal
	( E3DPolygonModel & model,
		unsigned int iFirst, unsigned int nCount ) const
{
	ESLAssert( m_iRefNormal + m_nRefNormalCount <= model.GetNormalCount() ) ;
	ESLAssert( m_iRefNormal <= iFirst ) ;
	ESLAssert( iFirst + nCount <= m_iRefNormal + m_nRefNormalCount ) ;
	PE3D_VECTOR4	pNormals = model.GetNormalList( iFirst ) ;
	::eslMoveMemory
		( m_pNormalBuf, pNormals, nCount * sizeof(E3D_VECTOR4) ) ;
	//
	m_rvmat.RevolveVectors
		( m_pNormalBuf, m_pNormalBuf, NULL, nCount ) ;
	//
	unsigned int	i, iOffset = iFirst - m_iRefNormal ;
	for ( i = 0; i < nCount; i ++ )
	{
		REAL32	rApply = m_pNormalApply[i + iOffset] ;
		if ( *((DWORD*)&rApply) != 0 )
		{
			if ( *((SDWORD*)&rApply) < 0x3F7FF000 )
			{
				E3D_VECTOR4	v = m_pNormalBuf[i] ;
				v -= pNormals[i] ;
				v *= rApply ;
				pNormals[i] += v ;
			}
			else
			{
				pNormals[i] = m_pNormalBuf[i] ;
			}
		}
	}
}

// 全てのジョイントを削除
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::DeleteContents( void )
{
	m_wstrName.FreeString( ) ;
	m_fVertexApplyAll = false ;
	m_fNormalApplyAll = false ;
	//
	if ( m_pVertexApply != NULL )
	{
		::eslHeapFree( NULL, m_pVertexApply ) ;
		m_pVertexApply = NULL ;
	}
	if ( m_pVertexBuf != NULL )
	{
		::eslHeapFree( NULL, m_pVertexBuf ) ;
		m_pVertexBuf = NULL ;
	}
	m_iRefVertex = 0 ;
	m_nRefVertexCount = 0 ;
	//
	if ( m_pNormalApply != NULL )
	{
		::eslHeapFree( NULL, m_pNormalApply ) ;
		m_pNormalApply = NULL ;
	}
	if ( m_pNormalBuf != NULL )
	{
		::eslHeapFree( NULL, m_pNormalBuf ) ;
		m_pNormalBuf = NULL ;
	}
	m_iRefNormal = 0 ;
	m_nRefNormalCount = 0 ;
	//
	E3DModelJoint::DeleteContents( ) ;
}

// ボーンを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DBoneJoint::ReadBoneData( ESLFileObject & file )
{
	DeleteContents( ) ;
	//
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	file.Read( m_wstrName.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
	m_wstrName.ReleaseBuffer( dwLength ) ;
	//
	file.Read( &m_vCenter, sizeof(E3D_VECTOR) ) ;
	//
	unsigned int	i, iFirst, nCount ;
	file.Read( &iFirst, sizeof(unsigned int) ) ;
	file.Read( &nCount, sizeof(unsigned int) ) ;
	AllocateVertexBuffer( iFirst, nCount ) ;
	file.Read( m_pVertexApply, nCount * sizeof(REAL32) ) ;
	//
	m_fVertexApplyAll = true ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( *((SDWORD*)(m_pVertexApply + i)) < 0x3F7FF000 )
		{
			m_fVertexApplyAll = false ;
			break ;
		}
	}
	//
	file.Read( &iFirst, sizeof(unsigned int) ) ;
	file.Read( &nCount, sizeof(unsigned int) ) ;
	AllocateNormalBuffer( iFirst, nCount ) ;
	file.Read( m_pNormalApply, nCount * sizeof(REAL32) ) ;
	//
	m_fNormalApplyAll = true ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( *((SDWORD*)(m_pNormalApply + i)) < 0x3F7FF000 )
		{
			m_fNormalApplyAll = false ;
			break ;
		}
	}
	//
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwLength; i ++ )
	{
		E3DBoneJoint *	pBone = new E3DBoneJoint ;
		ESLError	err = pBone->ReadBoneData( file ) ;
		if ( err )
		{
			delete	pBone ;
			return	err ;
		}
		AddSubJoint( pBone ) ;
	}
	return	eslErrSuccess ;
}

// ボーンを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DBoneJoint::WriteBoneData( ESLFileObject & file )
{
	DWORD	dwLength ;
	dwLength = m_wstrName.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	file.Write( m_wstrName.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	//
	file.Write( &m_vCenter, sizeof(E3D_VECTOR) ) ;
	//
	file.Write( &m_iRefVertex, sizeof(unsigned int) ) ;
	file.Write( &m_nRefVertexCount, sizeof(unsigned int) ) ;
	file.Write( m_pVertexApply, m_nRefVertexCount * sizeof(REAL32) ) ;
	//
	file.Write( &m_iRefNormal, sizeof(unsigned int) ) ;
	file.Write( &m_nRefNormalCount, sizeof(unsigned int) ) ;
	file.Write( m_pNormalApply, m_nRefNormalCount * sizeof(REAL32) ) ;
	//
	dwLength = JointList().GetSize( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		E3DBoneJoint *	pBone = (E3DBoneJoint*) JointList().GetAt( i ) ;
		ESLAssert( pBone->IsKindOf( ESL_RUNTIME_CLASS(E3DBonePolygonModel::E3DBoneJoint) ) ) ;
		ESLError	err = pBone->WriteBoneData( file ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	return	eslErrSuccess ;
}

// 頂点用バッファ確保
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::
		AllocateVertexBuffer( unsigned int iFirst, unsigned int nCount )
{
	if ( (m_iRefVertex != iFirst) || (m_nRefVertexCount != nCount) )
	{
		m_pVertexApply =
			(REAL32*) ::eslHeapReallocate
				( NULL, m_pVertexApply,
					nCount * sizeof(REAL32), ESL_HEAP_ZERO_INIT ) ;
		m_pVertexBuf =
			(PE3D_VECTOR4) ::eslHeapReallocate
				( NULL, m_pVertexBuf,
					nCount * sizeof(E3D_VECTOR4), ESL_HEAP_ZERO_INIT ) ;
		//
/*		int		nSrcOffset = 0,
				nDstOffset = (int) (m_iRefVertex - iFirst),
				nMoveCount = (int) m_nRefVertexCount ;
		if ( nDstOffset < 0 )
		{
			nMoveCount += nDstOffset ;
			nSrcOffset = - nDstOffset ;
			nDstOffset = 0 ;
		}
		if ( nDstOffset + nMoveCount > (int) nCount )
		{
			nMoveCount = (int) nCount - nDstOffset ;
		}
		if ( nMoveCount > 0 )
		{
			::eslMoveMemory
				( m_pVertexApply + nDstOffset,
					m_pVertexApply + nSrcOffset,
					nMoveCount * sizeof(REAL32) ) ;
		}
		if ( (nSrcOffset >= (int) m_nRefVertexCount)
			|| (nDstOffset >= (int) m_nRefVertexCount) )
		{
			nDstOffset = 0 ;
			nMoveCount = m_nRefVertexCount ;
		}
		else if ( nSrcOffset < (int) m_nRefVertexCount )
		{
			nDstOffset = m_nRefVertexCount - nSrcOffset ;
			nMoveCount = nSrcOffset ;
		}
		else
		{
			ESLAssert( nDstOffset < (int) m_nRefVertexCount ) ;
			nMoveCount = nDstOffset ;
			nDstOffset = 0 ;
		}
		if ( nMoveCount > 0 )
		{
			::eslFillMemory
				( m_pVertexApply + nDstOffset,
					0, nMoveCount * sizeof(REAL32) ) ;
		}
*/		//
		m_iRefVertex = iFirst ;
		m_nRefVertexCount = nCount ;
	}
}

// 法線用バッファ確保
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DBoneJoint::
		AllocateNormalBuffer( unsigned int iFirst, unsigned int nCount )
{
	if ( (m_iRefNormal != iFirst) || (m_nRefNormalCount != nCount) )
	{
		m_pNormalApply =
			(REAL32*) ::eslHeapReallocate
				( NULL, m_pNormalApply,
					nCount * sizeof(REAL32), ESL_HEAP_ZERO_INIT ) ;
		m_pNormalBuf =
			(PE3D_VECTOR4) ::eslHeapReallocate
				( NULL, m_pNormalBuf,
					nCount * sizeof(E3D_VECTOR4), ESL_HEAP_ZERO_INIT ) ;
		//
/*		int		nSrcOffset = 0,
				nDstOffset = (int) (m_iRefNormal - iFirst),
				nMoveCount = (int) m_nRefNormalCount ;
		if ( nDstOffset < 0 )
		{
			nMoveCount += nDstOffset ;
			nSrcOffset = - nDstOffset ;
			nDstOffset = 0 ;
		}
		if ( nDstOffset + nMoveCount > (int) nCount )
		{
			nMoveCount = (int) nCount - nDstOffset ;
		}
		if ( nMoveCount > 0 )
		{
			::eslMoveMemory
				( m_pNormalApply + nDstOffset,
					m_pNormalApply + nSrcOffset,
					nMoveCount * sizeof(REAL32) ) ;
		}
		if ( (nSrcOffset >= (int) m_nRefNormalCount)
			|| (nDstOffset >= (int) m_nRefNormalCount) )
		{
			nDstOffset = 0 ;
			nMoveCount = m_nRefNormalCount ;
		}
		else if ( nSrcOffset < (int) m_nRefNormalCount )
		{
			nDstOffset = m_nRefNormalCount - nSrcOffset ;
			nMoveCount = nSrcOffset ;
		}
		else
		{
			ESLAssert( nDstOffset < (int) m_nRefNormalCount ) ;
			nMoveCount = nDstOffset ;
			nDstOffset = 0 ;
		}
		if ( nMoveCount > 0 )
		{
			::eslFillMemory
				( m_pNormalApply + nDstOffset,
					0, nMoveCount * sizeof(REAL32) ) ;
		}
*/		//
		m_iRefNormal = iFirst ;
		m_nRefNormalCount = nCount ;
	}
}

// ボーンを名前で検索
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DBoneJoint *
	E3DBonePolygonModel::E3DBoneJoint::FindBoneAs( const wchar_t * pwszName )
{
	if ( m_wstrName == pwszName )
	{
		return	this ;
	}
	for ( unsigned int i = 0; i < JointList().GetSize(); i ++ )
	{
		E3DBoneJoint *	pBone = (E3DBoneJoint*) JointList().GetAt( i ) ;
		ESLAssert( pBone->IsKindOf( ESL_RUNTIME_CLASS(E3DBonePolygonModel::E3DBoneJoint) ) ) ;
		pBone = pBone->FindBoneAs( pwszName ) ;
		if ( pBone != NULL )
		{
			return	pBone ;
		}
	}
	return	NULL ;
}

// E3DBonePolygonModel::E3DClothMorph 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorph::E3DClothMorph( void )
	: m_ptrHinderList( NULL, 0 )
{
	m_pJoint = NULL ;
	m_hClothModel = NULL ;
}

// E3DBonePolygonModel::E3DClothMorph 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorph::~E3DClothMorph( void )
{
	ReleaseData( ) ;
}

// 所有データを開放する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::ReleaseData( void )
{
	DeleteClothMorph( ) ;
	//
	m_lstMeshEntry.RemoveAll( ) ;
	m_wstrName.FreeString( ) ;
	//
	return	eslErrSuccess ;
}

// モーフィングオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::CreateClothMorph
	( E3DBonePolygonModel & model )
{
	DeleteClothMorph( ) ;
	//
	m_pJoint = model.FindBoneAs( m_wstrName ) ;
	if ( m_pJoint == NULL )
	{
		return	eslErrGeneral ;
	}
	m_hClothModel = eglCreateClothModelMorph( ) ;
	if ( m_hClothModel == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	unsigned int	i, nCount = m_lstMeshEntry.GetSize( ) ;
	ULONG_PTR		ptrVertexBuf = (LONG_PTR) model.GetVertexBuffer( ) ;
	ULONG_PTR		ptrVertexList = (LONG_PTR) model.GetVertexList( ) ;
	ULONG_PTR		ptrNormalBuf = (LONG_PTR) model.GetNormalBuffer( ) ;
	ULONG_PTR		ptrNormalList = (ULONG_PTR) model.GetNormalList( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		//
		// メッシュデータの複製
		//
		CLOTH_MESH_ENTRY *	pcme = m_lstMeshEntry.GetAt( i ) ;
		ESLAssert( pcme != NULL ) ;
		ESLAssert( pcme->pMesh == NULL ) ;
		//
		PE3D_PRIMITIVE_POLYGON	pOrgMesh =
			model.GetPrimitiveAt( pcme->dwMeshIndex ) ;
		if ( (pOrgMesh == NULL)
			&& !(pOrgMesh->dwTypeFlag & E3D_MESH_POLYGON) )
		{
			continue ;
		}
		DWORD	dwBytes = model.CalcPrimitiveDataSize( pOrgMesh ) ;
		PE3D_PRIMITIVE_POLYGON	pMesh =
			(PE3D_PRIMITIVE_POLYGON) eslHeapAllocate( NULL, dwBytes, 0 ) ;
		pcme->pMesh = pMesh ;
		eslMoveMemory( pMesh, pOrgMesh, dwBytes ) ;
		//
		unsigned int	iVertex =
			(((ULONG_PTR)pMesh->mesh.vertices)
									- ptrVertexBuf) / sizeof(E3D_VECTOR4) ;
		pMesh->mesh.vertices = (PE3D_VECTOR4)
			(((ULONG_PTR)pMesh->mesh.vertices)
							- ptrVertexBuf + ptrVertexList) ;
		if ( pMesh->mesh.normals != NULL )
		{
			pMesh->mesh.normals = (PE3D_VECTOR4)
				(((ULONG_PTR)pMesh->mesh.normals)
								- ptrNormalBuf + ptrNormalList) ;
		}
		if ( pcme->dwType == cmtHinder )
		{
			//
			// 当たり判定オブジェクト生成
			//
			ESLAssert( pcme->hmiHinderInfo.hMatrix == NULL ) ;
			pcme->hmiHinderInfo.hMatrix = eglCreateModelMatrix( ) ;
			pcme->hmiHinderInfo.hMatrix->Initialize
				( (const E3D_PRIMITIVE_POLYGON **) &pMesh, 1 ) ;
		}
		else
		{
			//
			// 布設定
			//
			unsigned int	iApplyFirst = m_pJoint->m_iRefVertex ;
			unsigned int	nApplyCount = m_pJoint->m_nRefVertexCount ;
			const REAL32 *	pVertexApply = m_pJoint->m_pVertexApply ;
			//
			// 適用範囲正規化
			//
			if ( iVertex < iApplyFirst )
			{
				iApplyFirst -= iVertex ;
			}
			else if ( iVertex < iApplyFirst + nApplyCount )
			{
				unsigned int	nBias = iVertex - iApplyFirst ;
				iApplyFirst = 0 ;
				nApplyCount -= nBias ;
				pVertexApply += nBias ;
			}
			else
			{
				iApplyFirst = 0 ;
				nApplyCount = 0 ;
			}
			if ( iApplyFirst + nApplyCount > pMesh->dwVertexCount )
			{
				if ( iApplyFirst < pMesh->dwVertexCount )
				{
					nApplyCount = pMesh->dwVertexCount - iApplyFirst ;
				}
				else
				{
					iApplyFirst = 0 ;
					nApplyCount = 0 ;
				}
			}
			//
			// 階層化パラメータ反映
			//
			EStreamBuffer	bufApply ;
			if ( pcme->caClothAttr.nEffectLayers > 0 )
			{
				//
				// バッファ初期化
				//
				REAL32 *	pApplyBuf =
					(REAL32*) bufApply.PutBuffer
						( pMesh->dwVertexCount * sizeof(REAL32) ) ;
				if ( iApplyFirst > 0 )
				{
					eslFillMemory
						( pApplyBuf, 0, iApplyFirst * sizeof(REAL32) ) ;
				}
				eslMoveMemory
					( pApplyBuf + iApplyFirst,
						pVertexApply, nApplyCount * sizeof(REAL32) ) ;
				if ( iApplyFirst + nApplyCount < pMesh->dwVertexCount )
				{
					unsigned int	iEnd = iApplyFirst + nApplyCount ;
					eslFillMemory
						( pApplyBuf + iEnd, 0,
							(pMesh->dwVertexCount - iEnd) * sizeof(REAL32) ) ;
				}
				pVertexApply = pApplyBuf ;
				iApplyFirst = 0 ;
				nApplyCount = pMesh->dwVertexCount ;
				//
				// 上位階層処理
				//
				unsigned int	j, k ;
				E3DBoneJoint *	pJoint = m_pJoint ;
				for ( j = 0; j < pcme->caClothAttr.nEffectLayers; j ++ )
				{
					pJoint = ESLTypeCast<E3DBoneJoint>( pJoint->m_parent ) ;
					if ( pJoint == NULL )
					{
						break ;
					}
					unsigned int	iRefFirst = pJoint->m_iRefVertex ;
					unsigned int	nRefCount = pJoint->m_nRefVertexCount ;
					const REAL32 *	pRefApply = pJoint->m_pVertexApply ;
					//
					if ( iVertex < iRefFirst )
					{
						iRefFirst -= iVertex ;
					}
					else if ( iVertex < iRefFirst + nRefCount )
					{
						unsigned int	nBias = iVertex - iRefFirst ;
						iRefFirst = 0 ;
						nRefCount -= nBias ;
						pRefApply += nBias ;
					}
					else
					{
						iRefFirst = 0 ;
						nRefCount = 0 ;
					}
					if ( iRefFirst + nRefCount > pMesh->dwVertexCount )
					{
						if ( iRefFirst < pMesh->dwVertexCount )
						{
							nRefCount = pMesh->dwVertexCount - iRefFirst ;
						}
						else
						{
							iRefFirst = 0 ;
							nRefCount = 0 ;
						}
					}
					for ( k = 0; k < nRefCount; k ++ )
					{
						if ( pRefApply[k] > 0 )
						{
							pApplyBuf[k + iRefFirst] =
								(REAL32) (1.0 - (1.0 - pRefApply[k])
									* (1.0 - pApplyBuf[k + iRefFirst])) ;
						}
					}
				}
			}
			//
			// パラメータセットアップ
			//
			if ( pcme->dwType == cmtWeaveCloth )
			{
				m_hClothModel->WeaveCloth
					( &(pcme->caClothAttr), pMesh,
						pVertexApply, iApplyFirst, nApplyCount ) ;
			}
			else
			{
				m_hClothModel->PatchCloth
					( &(pcme->caClothAttr), pMesh,
						pcme->hmiHinderInfo.rGapRadius,
						pVertexApply, iApplyFirst, nApplyCount ) ;
			}
		}
	}
	//
	// 当たり判定設定
	//
	return	SetHinderModel( NULL, 0 ) ;
}

// 当たり判定オブジェクトを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::SetHinderModel
	( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
{
	if ( m_hClothModel == NULL )
	{
		return	eslErrGeneral ;
	}
	m_bufHinderList.Delete( ) ;
	for ( unsigned int i = 0; i < m_lstMeshEntry.GetSize(); i ++ )
	{
		CLOTH_MESH_ENTRY *	pcme = m_lstMeshEntry.GetAt( i ) ;
		ESLAssert( pcme != NULL ) ;
		if ( pcme->dwType == cmtHinder )
		{
			m_bufHinderList.Write
				( &(pcme->hmiHinderInfo), sizeof(pcme->hmiHinderInfo) ) ;
		}
	}
	m_bufHinderList.Write
		( phmiHinder, nCount * sizeof(HINDER_MODEL_INFO) ) ;
	m_ptrHinderList = m_bufHinderList.GetBuffer( ) ;
	return	m_hClothModel->SetHinderModel
		( (const HINDER_MODEL_INFO *) m_ptrHinderList.GetBuffer(),
			m_ptrHinderList.GetLength() / sizeof(HINDER_MODEL_INFO) ) ;
}

// 当たり判定座標を更新する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::UpdateHinderModel( void )
{
	unsigned int	i, nCount = m_lstMeshEntry.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		CLOTH_MESH_ENTRY *	pcme = m_lstMeshEntry.GetAt( i ) ;
		ESLAssert( pcme != NULL ) ;
		if ( pcme->dwType == cmtHinder )
		{
			if ( pcme->hmiHinderInfo.hMatrix != NULL )
			{
				pcme->hmiHinderInfo.hMatrix->Initialize
					( (const E3D_PRIMITIVE_POLYGON **) &(pcme->pMesh), 1 ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// モーフィングオブジェクトを削除する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::DeleteClothMorph( void )
{
	if ( m_hClothModel != NULL )
	{
		m_hClothModel->Release( ) ;
		m_hClothModel = NULL ;
	}
	for ( unsigned int i = 0; i < m_lstMeshEntry.GetSize( ); i ++ )
	{
		CLOTH_MESH_ENTRY *	pcme = m_lstMeshEntry.GetAt( i ) ;
		ESLAssert( pcme != NULL ) ;
		if ( pcme->dwType == cmtHinder )
		{
			if ( pcme->hmiHinderInfo.hMatrix != NULL )
			{
				pcme->hmiHinderInfo.hMatrix->Release( ) ;
				pcme->hmiHinderInfo.hMatrix = NULL ;
			}
		}
		if ( pcme->pMesh != NULL )
		{
			eslHeapFree( NULL, pcme->pMesh, 0 ) ;
			pcme->pMesh = NULL ;
		}
	}
	m_pJoint = NULL ;
	return	eslErrSuccess ;
}

// データを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::ReadClothData
	( ESLFileObject & file )
{
	ReleaseData( ) ;
	//
	// 関連付けるジョイント名を読み込む
	//
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	eslErrGeneral ;
	}
	file.Read( m_wstrName.GetBuffer( dwLength ), dwLength * sizeof(wchar_t) ) ;
	m_wstrName.ReleaseBuffer( dwLength ) ;
	//
	// メッシュ配列を読み込む
	//
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		CLOTH_MESH_ENTRY_DATA	cmed ;
		if ( file.Read( &cmed, sizeof(cmed) ) < sizeof(cmed) )
		{
			return	eslErrGeneral ;
		}
		CLOTH_MESH_ENTRY *	pcme = new CLOTH_MESH_ENTRY ;
		m_lstMeshEntry.Add( pcme ) ;
		pcme->dwType = cmed.dwType ;
		pcme->dwMeshIndex = cmed.dwMeshIndex ;
		pcme->pMesh = NULL ;
		pcme->hmiHinderInfo = cmed.hmiHinderInfo ;
		pcme->hmiHinderInfo.hMatrix = NULL ;
		pcme->caClothAttr = cmed.caClothAttr ;
	}
	return	eslErrSuccess ;
}

// データを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::WriteClothData
	( ESLFileObject & file )
{
	//
	// 関連付けるジョイント名を書き出す
	//
	DWORD	dwLength = m_wstrName.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	file.Write( m_wstrName.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	//
	// メッシュ配列を書き出す
	//
	DWORD	dwCount = m_lstMeshEntry.GetSize( ) ;
	file.Write( &dwCount, sizeof(dwCount) ) ;
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		CLOTH_MESH_ENTRY_DATA	cmed ;
		CLOTH_MESH_ENTRY *	pcme = m_lstMeshEntry.GetAt( i ) ;
		ESLAssert( pcme != NULL ) ;
		//
		cmed.dwType = pcme->dwType ;
		cmed.dwMeshIndex = pcme->dwMeshIndex ;
		cmed.dwReserved[0] = 0 ;
		cmed.dwReserved[1] = 0 ;
		cmed.hmiHinderInfo = pcme->hmiHinderInfo ;
		cmed.hmiHinderInfo.hMatrix = NULL ;
		cmed.caClothAttr = pcme->caClothAttr ;
		//
		file.Write( &cmed, sizeof(cmed) ) ;
	}
	return	eslErrSuccess ;
}

// モーフィング実行
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorph::MorphCloth
	( const EGL_CLOTH_MORPH_PARAMETER & cmp )
{
	if ( m_hClothModel == NULL )
	{
		return	eslErrGeneral ;
	}
	return	m_hClothModel->MorphCloth( &cmp ) ;
}

ESLError E3DBonePolygonModel::E3DClothMorph::MorphMesh
	( const EGL_CLOTH_MORPH_PARAMETER & cmp )
{
	if ( m_hClothModel == NULL )
	{
		return	eslErrGeneral ;
	}
	return	m_hClothModel->MorphMesh( &cmp ) ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorphModelSet::E3DClothMorphModelSet
	( const E3DClothMorphModelSet & cmms )
{
	operator = ( cmms ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorphModelSet::~E3DClothMorphModelSet( void )
{
	ReleaseData( ) ;
}

// 代入（データ複製）
//////////////////////////////////////////////////////////////////////////////
const E3DBonePolygonModel::E3DClothMorphModelSet &
	E3DBonePolygonModel::E3DClothMorphModelSet::operator =
			( const E3DBonePolygonModel::E3DClothMorphModelSet & cmms )
{
	ReleaseData( ) ;
	//
	EMemoryFile	memfile ;
	memfile.Create( 0x1000 ) ;
	//
	unsigned int	i, nCount ;
	nCount = cmms.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcmSrc = cmms.GetAt( i ) ;
		if ( pcmSrc != NULL )
		{
			memfile.Seek( 0, memfile.FromBegin ) ;
			memfile.SetEndOfFile( ) ;
			if ( !pcmSrc->WriteClothData( memfile ) )
			{
				E3DClothMorph *	pcmDst = new E3DClothMorph ;
				memfile.Seek( 0, memfile.FromBegin ) ;
				if ( !pcmDst->ReadClothData( memfile ) )
				{
					Add( pcmDst ) ;
				}
				else
				{
					delete	pcmDst ;
				}
			}
		}
	}
	//
	return	*this ;
}

// データを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::ReadClothData
	( ESLFileObject & file )
{
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	//
	ReleaseData( ) ;
	//
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		E3DClothMorph *	pcm = new E3DClothMorph ;
		ESLError	err = pcm->ReadClothData( file ) ;
		if ( err )
		{
			delete	pcm ;
			return	err ;
		}
		Add( pcm ) ;
	}
	return	eslErrSuccess ;
}

// データを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::WriteClothData
	( ESLFileObject & file )
{
	DWORD	dwCount = GetSize( ) ;
	file.Write( &dwCount, sizeof(DWORD) ) ;
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		ESLError	err = pcm->WriteClothData( file ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// 所有データを開放する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::ReleaseData( void )
{
	RemoveAll( ) ;
	return	eslErrSuccess ;
}

// モーフィングオブジェクトを削除する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::DeleteClothMorph( void )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			pcm->DeleteClothMorph( ) ;
		}
	}
	return	eslErrSuccess ;
}

// 布シミュレータオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::CreateClothMorph
	( E3DBonePolygonModel & model )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			pcm->CreateClothMorph( model ) ;
		}
	}
	return	eslErrSuccess ;
}

// 外部当たり判定オブジェクトを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::SetHinderModel
	( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
{
	unsigned int	i, n ;
	n = GetSize( ) ;
	for ( i = 0; i < n; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			pcm->SetHinderModel( phmiHinder, nCount ) ;
		}
	}
	return	eslErrSuccess ;
}

// 当たり判定座標を更新する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::UpdateHinderModel( void )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			pcm->UpdateHinderModel( ) ;
		}
	}
	return	eslErrSuccess ;
}

// 布メッシュ処理
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DClothMorphModelSet::MorphCloth
	( const EGL_CLOTH_MORPH_PARAMETER & cmp )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			if ( pcm->m_pJoint != NULL )
			{
				EGL_CLOTH_MORPH_PARAMETER	cmpParam = cmp ;
				cmpParam.pBaseMatrix = &(pcm->m_pJoint->m_rvmat) ;
				pcm->MorphCloth( cmpParam ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

ESLError E3DBonePolygonModel::E3DClothMorphModelSet::MorphMesh
	( const EGL_CLOTH_MORPH_PARAMETER & cmp )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			if ( pcm->m_pJoint != NULL )
			{
				EGL_CLOTH_MORPH_PARAMETER	cmpParam = cmp ;
				cmpParam.pBaseMatrix = &(pcm->m_pJoint->m_rvmat) ;
				pcm->MorphMesh( cmpParam ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 布メッシュを検索する
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorph *
	E3DBonePolygonModel::E3DClothMorphModelSet::FindClothAs
		( const wchar_t * pwszName, unsigned int * pIndex )
{
	unsigned int	i, nCount ;
	nCount = GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DClothMorph *	pcm = GetAt( i ) ;
		ESLAssert( pcm != NULL ) ;
		if ( pcm != NULL )
		{
			if ( pcm->m_wstrName == pwszName )
			{
				if ( pIndex != NULL )
				{
					*pIndex = i ;
				}
				return	pcm ;
			}
		}
	}
	return	NULL ;
}


// E3DBonePolygonModel::E3DPose 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DPose::E3DPose( const E3DPose & pose )
{
	unsigned int	i, nCount ;
	nCount = pose.GetSize( ) ;
	for ( i = 0 ; i < nCount; i ++ )
	{
		ETaggedElement<EWideString,POSE_ENTRY> *	pElement ;
		pElement = pose.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		if ( pElement->GetObject() == NULL )
		{
			continue ;
		}
		POSE_ENTRY *	ppeNew = new POSE_ENTRY ;
		*ppeNew = *(pElement->GetObject()) ;
		Add( pElement->Tag(), ppeNew ) ;
	}
}

// データ複製
//////////////////////////////////////////////////////////////////////////////
const E3DBonePolygonModel::E3DPose &
	E3DBonePolygonModel::E3DPose::operator = ( const E3DPose & pose )
{
	unsigned int	i, nCount ;
	RemoveAll( ) ;
	nCount = pose.GetSize( ) ;
	for ( i = 0 ; i < nCount; i ++ )
	{
		ETaggedElement<EWideString,POSE_ENTRY> *	pElement ;
		pElement = pose.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		if ( pElement->GetObject() == NULL )
		{
			continue ;
		}
		Add( pElement->Tag(), new POSE_ENTRY( *(pElement->GetObject()) ) ) ;
	}
	return	*this ;
}

// データを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPose::ReadPose( ESLFileObject & file )
{
	RemoveAll( ) ;
	//
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0 ; i < dwCount; i ++ )
	{
		EWideString	wstrName ;
		DWORD		dwLen ;
		if ( file.Read( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
		{
			return	eslErrGeneral ;
		}
		file.Read( wstrName.GetBuffer(dwLen), dwLen * sizeof(wchar_t) ) ;
		wstrName.ReleaseBuffer( dwLen ) ;
		//
		POSE_ENTRY	peEntry ;
		if ( file.Read( &peEntry, sizeof(peEntry) ) < sizeof(peEntry) )
		{
			return	eslErrGeneral ;
		}
		Add( wstrName, new POSE_ENTRY( peEntry ) ) ;
	}
	return	eslErrSuccess ;
}

// データを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPose::WritePose( ESLFileObject & file )
{
	DWORD	dwCount = GetSize() ;
	if ( file.Write( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0 ; i < dwCount; i ++ )
	{
		ETaggedElement<EWideString,POSE_ENTRY> *
									pElement = GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			return	eslErrGeneral ;
		}
		ESLAssert( pElement->GetObject() != NULL ) ;
		if ( pElement->GetObject() == NULL )
		{
			return	eslErrGeneral ;
		}
		DWORD		dwLen = pElement->Tag().GetLength() ;
		if ( file.Write( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
		{
			return	eslErrGeneral ;
		}
		file.Write( pElement->Tag().CharPtr(), dwLen * sizeof(wchar_t) ) ;
		//
		POSE_ENTRY	peEntry = *(pElement->GetObject()) ;
		if ( file.Write( &peEntry, sizeof(peEntry) ) < sizeof(peEntry) )
		{
			return	eslErrGeneral ;
		}
	}
	return	eslErrSuccess ;
}

// E3DBonePolygonModel::E3DPoseAnimation 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DPoseAnimation::E3DPoseAnimation( void )
{
	m_nDuration = 0 ;
}

// E3DBonePolygonModel::E3DPoseAnimation 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DPoseAnimation::~E3DPoseAnimation( void )
{
}

// データを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPoseAnimation::ReadPoseAnimation
	( ESLFileObject & file )
{
	ReleasePoseAnimation( ) ;
	//
	// ポーズアニメーション名を読み込む
	//
	DWORD	dwLen ;
	if ( file.Read( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
	{
		return	eslErrGeneral ;
	}
	file.Read( m_wstrName.GetBuffer(dwLen), dwLen * sizeof(wchar_t) ) ;
	m_wstrName.ReleaseBuffer( dwLen ) ;
	//
	// ポーズエントリリストを読み込む
	//
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		E3DPoseAnimationEntry *	ppae = new E3DPoseAnimationEntry ;
		m_lstPose.Add( ppae ) ;
		//
		if ( file.Read( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
		{
			return	eslErrGeneral ;
		}
		file.Read( ppae->m_wstrPose.GetBuffer(dwLen), dwLen * sizeof(wchar_t) ) ;
		ppae->m_wstrPose.ReleaseBuffer( dwLen ) ;
		//
		POSE_ANIMATION_ENTRY	pae ;
		file.Read( &pae, sizeof(pae) ) ;
		//
		ppae->m_rVelocity = pae.rVelocity ;
		ppae->m_nDuration = pae.nDuration ;
	}
	//
	return	eslErrSuccess ;
}

// データを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPoseAnimation::WritePoseAnimation
	( ESLFileObject & file )
{
	//
	// ポーズアニメーション名を書き出す
	//
	DWORD	dwLen = m_wstrName.GetLength() ;
	if ( file.Write( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
	{
		return	eslErrGeneral ;
	}
	file.Write( m_wstrName.CharPtr(), dwLen * sizeof(wchar_t) ) ;
	//
	// ポーズエントリリストを読み込む
	//
	DWORD	dwCount = m_lstPose.GetSize() ;
	if ( file.Write( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		E3DPoseAnimationEntry *	ppae = m_lstPose.GetAt( i ) ;
		ESLAssert( ppae != NULL ) ;
		if ( ppae == NULL )
		{
			return	eslErrGeneral ;
		}
		dwLen = ppae->m_wstrPose.GetLength( ) ;
		if ( file.Write( &dwLen, sizeof(dwLen) ) < sizeof(dwLen) )
		{
			return	eslErrGeneral ;
		}
		file.Write( ppae->m_wstrPose.CharPtr(), dwLen * sizeof(wchar_t) ) ;
		//
		POSE_ANIMATION_ENTRY	pae ;
		pae.rVelocity = ppae->m_rVelocity ;
		pae.nDuration = ppae->m_nDuration ;
		pae.nReserved = 0 ;
		file.Write( &pae, sizeof(pae) ) ;
	}
	//
	return	eslErrSuccess ;
}

// データを解放する
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::E3DPoseAnimation::ReleasePoseAnimation( void )
{
	m_wstrName.FreeString( ) ;
	m_lstPose.RemoveAll( ) ;
	m_lstBone.RemoveAll( ) ;
	m_nDuration = 0 ;
}

// データ複製
//////////////////////////////////////////////////////////////////////////////
const E3DBonePolygonModel::E3DPoseAnimation &
	E3DBonePolygonModel::E3DPoseAnimation::operator =
		( const E3DBonePolygonModel::E3DPoseAnimation & poseanime )
{
	m_wstrName = poseanime.m_wstrName ;
	m_lstPose.RemoveAll( ) ;
	m_lstBone.RemoveAll( ) ;
	//
	unsigned int	i, nCount ;
	nCount = poseanime.m_lstPose.GetSize( ) ;
	m_lstPose.SetSize( 0, nCount ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DPoseAnimationEntry *	ppae = poseanime.m_lstPose.GetAt( i ) ;
		if ( ppae != NULL )
		{
			m_lstPose.Add( new E3DPoseAnimationEntry( *ppae ) ) ;
		}
	}
	nCount = poseanime.m_lstBone.GetSize( ) ;
	m_lstBone.SetSize( 0, nCount ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<EWideString,E3DBoneAnimation> *
					pElement = poseanime.m_lstBone.GetAt( i ) ;
		if ( (pElement != NULL) && (pElement->GetObject() != NULL) )
		{
			m_lstBone.Add
				( pElement->Tag(),
					new E3DBoneAnimation( *(pElement->GetObject()) ) ) ;
		}
	}
	return	*this ;
}

// モーフィングオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPoseAnimation::InitializePoseAnimation
	( E3DBonePolygonModel & model )
{
	ClosePoseAnimation( ) ;
	//
	// 順次ポーズを追加する
	//
	unsigned int	i, nCount ;
	unsigned int	nCurrentDuration = 0 ;
	nCount = m_lstPose.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DPoseAnimationEntry *	ppae = m_lstPose.GetAt( i ) ;
		if ( ppae == NULL )
		{
			continue ;
		}
		E3DPose *	pPose = model.FindPoseAs( ppae->m_wstrPose ) ;
		if ( pPose == NULL )
		{
			continue ;
		}
		nCurrentDuration += ppae->m_nDuration ;
		//
		// ポーズ要素（ボーン）をアニメーションベジェ曲線に追加する
		//
		unsigned int	j, m ;
		m = pPose->GetSize( ) ;
		for ( j = 0 ; j < m; j ++ )
		{
			ETaggedElement<EWideString,POSE_ENTRY> *
				pElement = pPose->GetAt( j ) ;
			ESLAssert( pElement != NULL ) ;
			if ( pElement == NULL )
			{
				continue ;
			}
			E3DBoneJoint *	pBone = model.FindBoneAs( pElement->Tag() ) ;
			if ( pBone == NULL )
			{
				continue ;
			}
			POSE_ENTRY *	pPoseEntry = pElement->GetObject( ) ;
			ESLAssert( pPoseEntry != NULL ) ;
			if ( pPoseEntry == NULL )
			{
				continue ;
			}
			if ( pPoseEntry->dwFlags & psfNotUsedEntry )
			{
				continue ;
			}
			E3DBoneAnimation *
				pBoneAnime = m_lstBone.GetAs( pElement->Tag() ) ;
			if ( pBoneAnime == NULL )
			{
				//
				// 未エントリボーン
				//
				pBoneAnime = new E3DBoneAnimation ;
				m_lstBone.SetAs( pElement->Tag(), pBoneAnime ) ;
				pBoneAnime->m_pJoint = pBone ;
				//
				ESLAssert( nCurrentDuration >= pBoneAnime->GetDuration() ) ;
				pBoneAnime->m_nDurations.Add
					( nCurrentDuration - pBoneAnime->GetDuration() ) ;
				//
				pBoneAnime->m_bzRev.SetCount( 4 ) ;
				pBoneAnime->m_bzMove.SetCount( 4 ) ;
				//
				pBoneAnime->m_bzRev.SetLine
					( E3DQuaternion( E3DRevMatrix( pBone->Matrix() ) ),
						pPoseEntry->qtRev, 0, ppae->m_rVelocity ) ;
				pBoneAnime->m_bzMove.SetLine
					( E3DVector( pBone->Position() ),
						pPoseEntry->vOffset, 0, ppae->m_rVelocity ) ;
				pBoneAnime->m_rLastVelocity = ppae->m_rVelocity ;
				//
				continue ;
			}
			ESLAssert( pBoneAnime->m_nDurations.GetSize() >= 1 ) ;
			ESLAssert( nCurrentDuration >= pBoneAnime->GetDuration() ) ;
			unsigned int	nLastDuration =
					pBoneAnime->m_nDurations.GetLastAt( ) ;
			unsigned int	nNextDuration =
					nCurrentDuration >= pBoneAnime->GetDuration() ;
			pBoneAnime->m_nDurations.Add( nNextDuration ) ;
			double	rLastVelocity = pBoneAnime->m_rLastVelocity ;
			if ( nLastDuration > 0 )
			{
				rLastVelocity *= (double) nNextDuration / nLastDuration ;
			}
			int	nDivision = (pBoneAnime->m_bzRev.GetCount() - 1) / 3 ;
			pBoneAnime->m_bzRev.SetCount( nDivision * 3 + 4 ) ;
			pBoneAnime->m_bzMove.SetCount( nDivision * 3 + 4) ;
			if ( nDivision <= 1 )
			{
				//
				// 3点曲線生成
				//
				E3D_QUATERNION	qt0 = pBoneAnime->m_bzRev[0] ;
				E3D_QUATERNION	qt1 = pBoneAnime->m_bzRev[3] ;
				E3D_VECTOR		vp0 = pBoneAnime->m_bzMove[0] ;
				E3D_VECTOR		vp1 = pBoneAnime->m_bzMove[3] ;
				pBoneAnime->m_bzRev.SetCurveUnsmoothSpeed
					( qt0, qt1, pPoseEntry->qtRev,
						0, pBoneAnime->m_rLastVelocity,
						rLastVelocity, ppae->m_rVelocity ) ;
				pBoneAnime->m_bzMove.SetCurveUnsmoothSpeed
					( vp0, vp1, pPoseEntry->vOffset,
						0, pBoneAnime->m_rLastVelocity,
						rLastVelocity, ppae->m_rVelocity ) ;
				pBoneAnime->m_rLastVelocity = ppae->m_rVelocity ;
				continue ;
			}
			//
			// 4点目以降
			//
			pBoneAnime->m_bzRev.AddCurveUnsmoothSpeed
				( pPoseEntry->qtRev,
					rLastVelocity, ppae->m_rVelocity, nDivision ) ;
			pBoneAnime->m_bzMove.AddCurveUnsmoothSpeed
				( pPoseEntry->vOffset,
					rLastVelocity, ppae->m_rVelocity, nDivision ) ;
		}
	}
	m_nDuration = nCurrentDuration ;
	return	eslErrSuccess ;
}

// アニメーションデータを開放する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPoseAnimation::ClosePoseAnimation( void )
{
	m_lstBone.RemoveAll( ) ;
	m_nDuration = 0 ;
	return	eslErrSuccess ;
}

// アニメーション設定
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::E3DPoseAnimation::UpdateAnimation
	( E3DBonePolygonModel & model, unsigned int nTime )
{
	unsigned int	i, nCount ;
	nCount = m_lstBone.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		E3DBoneAnimation *	pBoneAnime = m_lstBone.GetObjectAt( i ) ;
		ESLAssert( pBoneAnime != NULL ) ;
		if ( pBoneAnime == NULL )
		{
			continue ;
		}
		ESLAssert( pBoneAnime->m_pJoint != NULL ) ;
		if ( pBoneAnime->m_pJoint == NULL )
		{
			continue ;
		}
		double	t = 0 ;
		unsigned int	j, m ;
		unsigned int	nLastTime = nTime ;
		m = pBoneAnime->m_nDurations.GetSize( ) ;
		if ( m > 0 )
		{
			for ( j = 0; j < m; j ++ )
			{
				if ( nLastTime <= pBoneAnime->m_nDurations[j] )
				{
					t += (double) nLastTime / pBoneAnime->m_nDurations[j] ;
					break ;
				}
				nLastTime -= pBoneAnime->m_nDurations[j] ;
				t += 1 ;
			}
			t /= m ;
		}
		E3DBoneJoint *	pBone = pBoneAnime->m_pJoint ;
		E3D_REV_MATRIX	matTemp ;
		E3D_QUATERNION	qtTemp = pBoneAnime->m_bzRev.pt( t ) ;
		qtTemp.Normalize() ;
		qtTemp.ToMatrix( matTemp ) ;
		pBone->Matrix() = matTemp ;
		pBone->Position() = pBoneAnime->m_bzMove.pt( t ) ;
	}
	return	eslErrSuccess ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DBonePolygonModel( void )
{
	m_pOrgVertexes = NULL ;
	m_pOrgNormals = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::~E3DBonePolygonModel( void )
{
	DeleteContents( ) ;
}

// モデルデータを削除する
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::DeleteContents( void )
{
	if ( m_pOrgVertexes != NULL )
	{
		::eslHeapFree( NULL, m_pOrgVertexes ) ;
		m_pOrgVertexes = NULL ;
	}
	if ( m_pOrgNormals != NULL )
	{
		::eslHeapFree( NULL, m_pOrgNormals ) ;
		m_pOrgNormals = NULL ;
	}
	m_lstBones.RemoveAll( ) ;
	m_cmmsClothes.ReleaseData( ) ;
	m_wstaPoses.RemoveAll( ) ;
	m_wstaPoseAnimations.RemoveAll( ) ;
	//
	E3DPolygonModel::DeleteContents( ) ;
}

// 頂点バッファを確保する
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::AllocateVertexBuffer( unsigned int nCount )
{
	m_pOrgVertexes =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pOrgVertexes,
				nCount * sizeof(E3D_VECTOR4), ESL_HEAP_ZERO_INIT ) ;

	E3DPolygonModel::AllocateVertexBuffer( nCount ) ;
}

// 法線バッファを確保する
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::AllocateNormalBuffer( unsigned int nCount )
{
	m_pOrgNormals =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pOrgNormals,
				nCount * sizeof(E3D_VECTOR4), ESL_HEAP_ZERO_INIT ) ;

	E3DPolygonModel::AllocateNormalBuffer( nCount ) ;
}

// 頂点リストの座標をコミットする
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::CommitVertexBuffer( int iVertex, int nCount )
{
	if ( ((unsigned int) iVertex < m_nVertexCount)
		&& ((unsigned int) (iVertex + nCount) <= m_nVertexCount) )
	{
		::eslMoveMemory
			( m_pOrgVertexes + iVertex,
				m_pVertexesBuf + iVertex, nCount * sizeof(E3D_VECTOR4) ) ;
	}
	E3DPolygonModel::CommitVertexBuffer( iVertex, nCount ) ;
}

// 法線リストの座標をコミットする
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::CommitNormalBuffer( int iNormal, int nCount )
{
	if ( ((unsigned int) iNormal < m_nNormalCount)
		&& ((unsigned int) (iNormal + nCount) <= m_nNormalCount) )
	{
		::eslMoveMemory
			( m_pOrgNormals + iNormal,
				m_pNormalsBuf + iNormal, nCount * sizeof(E3D_VECTOR4) ) ;
	}
	E3DPolygonModel::CommitNormalBuffer( iNormal, nCount ) ;
}

// モデルデータを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::ReadModel( ESLFileObject & file )
{
	ESLError	err ;
	err = E3DPolygonModel::ReadModel( file ) ;
	if ( err )
	{
		return	err ;
	}
	if ( GetVertexCount() > 0 )
	{
		::eslMoveMemory
			( m_pOrgVertexes, GetVertexList(),
				GetVertexCount() * sizeof(E3D_VECTOR4) ) ;
	}
	if ( GetNormalCount() > 0 )
	{
		::eslMoveMemory
			( m_pOrgNormals, GetNormalList(),
				GetNormalCount() * sizeof(E3D_VECTOR4) ) ;
	}
	return	err ;
}

// ユーザー定義のレコードを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::ReadUserRecord( EMCFile & file, UINT64 idRec )
{
	if ( idRec == *((UINT64*)"bonjoint") )
	{
		DWORD	dwCount ;
		if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrGeneral ;
		}
		for ( DWORD i = 0; i < dwCount; i ++ )
		{
			E3DBoneJoint *	pBone = new E3DBoneJoint ;
			ESLError	err = pBone->ReadBoneData( file ) ;
			if ( err )
			{
				delete	pBone ;
				return	err ;
			}
			m_lstBones.Add( pBone ) ;
		}
		return	eslErrSuccess ;
	}
	else if ( idRec == *((UINT64*)"clothmsh") )
	{
		return	m_cmmsClothes.ReadClothData( file ) ;
	}
	return	E3DPolygonModel::ReadUserRecord( file, idRec ) ;
}

// モデルデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::WriteModel( ESLFileObject & file )
{
	::eslMoveMemory
		( GetVertexList(), m_pOrgVertexes,
			GetVertexCount() * sizeof(E3D_VECTOR4) ) ;
	::eslMoveMemory
		( GetNormalList(), m_pOrgNormals,
			GetNormalCount() * sizeof(E3D_VECTOR4) ) ;
	//
	return	E3DPolygonModel::WriteModel( file ) ;
}

// ユーザー定義のレコードを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::WriteUserRecord( EMCFile & file )
{
	ESLError	err ;
	err = E3DPolygonModel::WriteUserRecord( file ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwCount = m_lstBones.GetSize( ) ;
	if ( dwCount > 0 )
	{
		if ( file.DescendRecord( (UINT64*)"bonjoint" ) )
		{
			return	eslErrGeneral ;
		}
		file.Write( &dwCount, sizeof(DWORD) ) ;
		for ( DWORD i = 0; i < dwCount; i ++ )
		{
			E3DBoneJoint *	pBone = m_lstBones.GetAt( i ) ;
			ESLAssert( pBone != NULL ) ;
			err = pBone->WriteBoneData( file ) ;
			if ( err )
			{
				return	err ;
			}
		}
		file.AscendRecord( ) ;
	}
	dwCount = m_cmmsClothes.GetSize( ) ;
	if ( dwCount > 0 )
	{
		if ( file.DescendRecord( (UINT64*)"clothmsh" ) )
		{
			return	eslErrGeneral ;
		}
		err = m_cmmsClothes.WriteClothData( file ) ;
		if ( err )
		{
			return	err ;
		}
		file.AscendRecord( ) ;
	}
	return	eslErrSuccess ;
}

// ボーンの現在のパラメータをモデルに反映する
//////////////////////////////////////////////////////////////////////////////
void E3DBonePolygonModel::TransformAccordingAsBone( void )
{
	if ( GetVertexCount() > 0 )
	{
		::eslMoveMemory
			( GetVertexList(), m_pOrgVertexes,
				GetVertexCount() * sizeof(E3D_VECTOR4) ) ;
	}
	if ( GetNormalCount() > 0 )
	{
		::eslMoveMemory
			( GetNormalList(), m_pOrgNormals,
				GetNormalCount() * sizeof(E3D_VECTOR4) ) ;
	}
	for ( unsigned int i = 0; i < m_lstBones.GetSize(); i ++ )
	{
		E3DBoneJoint *	pBone = m_lstBones.GetAt( i ) ;
		ESLAssert( pBone != NULL ) ;
		pBone->RefreshJoint( ) ;
		pBone->TransformModel( *this ) ;
	}
	m_cmmsClothes.UpdateHinderModel() ;
}

// ボーンを検索する
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DBoneJoint *
	E3DBonePolygonModel::FindBoneAs( const wchar_t * pwszName )
{
	for ( unsigned int i = 0; i < m_lstBones.GetSize(); i ++ )
	{
		E3DBoneJoint *	pBone = m_lstBones.GetAt( i ) ;
		ESLAssert( pBone != NULL ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		pBone = pBone->FindBoneAs( pwszName ) ;
		if ( pBone != NULL )
		{
			return	pBone ;
		}
	}
	return	NULL ;
}

// 布シミュレータオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::CreateClothMorph( void )
{
	return	m_cmmsClothes.CreateClothMorph( *this ) ;
}

// 外部当たり判定オブジェクトを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::SetHinderModel
	( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
{
	return	m_cmmsClothes.SetHinderModel( phmiHinder, nCount ) ;
}

// 布メッシュ処理
//////////////////////////////////////////////////////////////////////////////
ESLError E3DBonePolygonModel::MorphCloth
	( const EGL_CLOTH_MORPH_PARAMETER & cmp )
{
	return	m_cmmsClothes.MorphCloth( cmp ) ;
}

// 布メッシュを検索する
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DClothMorph *
	E3DBonePolygonModel::FindClothAs
		( const wchar_t * pwszName, unsigned int * pIndex )
{
	return	m_cmmsClothes.FindClothAs( pwszName, pIndex ) ;
}

// ポーズを取得する
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DPose *
	E3DBonePolygonModel::FindPoseAs
		( const wchar_t * pwszName, unsigned int * pIndex )
{
	return	m_wstaPoses.GetAs( pwszName, pIndex ) ;
}

// ポーズを取得する
//////////////////////////////////////////////////////////////////////////////
E3DBonePolygonModel::E3DPoseAnimation *
	E3DBonePolygonModel::FindPoseAnimationAs
		( const wchar_t * pwszName, unsigned int * pIndex )
{
	return	m_wstaPoseAnimations.GetAs( pwszName, pIndex ) ;
}



//////////////////////////////////////////////////////////////////////////////
// レンダリングオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DRenderPolygon, ESLObject )

// E3DRenderPolygon::RENDER_THREAD 構築
//////////////////////////////////////////////////////////////////////////////
E3DRenderPolygon::RENDER_THREAD::RENDER_THREAD( DWORD dwStackSize )
{
	pRenderPoly = NULL ;
	hRenderPoly = ::eglCreateRenderPolygon( ) ;
	hStackHeap = NULL ;
	if ( dwStackSize )
	{
		hStackHeap =
			::eslStackHeapCreate( dwStackSize, dwStackSize, 0 ) ;
	}
	hThread = NULL ;
	dwThreadID = 0 ;
	hRenderEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	hRendered = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
}

// E3DRenderPolygon::RENDER_THREAD 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DRenderPolygon::RENDER_THREAD::~RENDER_THREAD( void )
{
	if ( hThread != NULL )
	{
		::PostThreadMessage( dwThreadID, rtmQuit, 0, 0 ) ;
		::WaitForSingleObject( hThread, INFINITE ) ;
		::CloseHandle( hThread ) ;
	}
	if ( hStackHeap != NULL )
	{
		::eslStackHeapDestroy( hStackHeap ) ;
	}
	hRenderPoly->Release( ) ;
	::CloseHandle( hRenderEvent ) ;
	::CloseHandle( hRendered ) ;
}

// E3DRenderPolygon::POLYGON_LIST 構築
//////////////////////////////////////////////////////////////////////////////
E3DRenderPolygon::POLYGON_LIST::POLYGON_LIST( void )
{
	dwCount = 0 ;
	dwLimit = 0 ;
	pEntries = NULL ;
}

// E3DRenderPolygon::POLYGON_LIST 初期化
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::POLYGON_LIST::Release( void )
{
	dwCount = 0 ;
	dwLimit = 0 ;
	pEntries = NULL ;
}

// バッファ確保
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::POLYGON_LIST::Allocate
		( HESLHEAP hHeap, DWORD dwBufSize )
{
	ESLAssert( pEntries == NULL ) ;
	dwCount = 0 ;
	dwLimit = dwBufSize ;
	pEntries =
		(PE3D_POLYGON_ENTRY*) ::eslHeapAllocate
			( hHeap, dwBufSize * sizeof(PE3D_POLYGON_ENTRY), 0 ) ;
}

// エントリ追加
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::POLYGON_LIST::AddEntry
		( HESLHEAP hHeap, PE3D_POLYGON_ENTRY pEntry )
{
	if ( dwLimit <= dwCount )
	{
		dwLimit = dwCount + 0x10 + (dwLimit / 2) ;
		pEntries =
			(PE3D_POLYGON_ENTRY*) ::eslHeapReallocate
				( hHeap, pEntries,
					dwLimit * sizeof(PE3D_POLYGON_ENTRY), 0 ) ;
		ESLTrace( "E3DRenderPolygon : polygon entry "
					"list size was expanded in %d\n", dwLimit ) ;
	}
	ESLAssert( dwCount < dwLimit ) ;
	pEntries[dwCount ++] = pEntry ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DRenderPolygon::E3DRenderPolygon( void )
{
	m_hRenderPoly = NULL ;
	m_dwSortingFlags = E3D_SORT_TRANSPARENT | E3D_SORT_OPAQUE ;
	m_dwRayTracingFlags = 0 ;
	//
	m_hLocalHeap = NULL ;
	m_hStackHeap = NULL ;
	m_dwStackSize = 0 ;
	//
	m_iCurrentView = 0 ;
	//
	m_pDstImage = NULL ;
	m_pZBuffer = NULL ;
	//
	m_pLightEntries = NULL ;
	m_pShadowMapEntries = NULL ;
	m_nLightCount = 0 ;
	//
	m_fGlobalEnvironment = false ;
	//
	m_pGPUInterface = NULL ;
	m_hGPUBuffer = NULL ;
	//
	m_fBeginBuildModel = false ;
	m_fMultiThreadBuildModel = false ;
	m_dwExceptionShadeFlags = 0 ;
	//
	m_mrfRenderingFlag = mrfSingle ;
	m_dwUsingThreads = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DRenderPolygon::~E3DRenderPolygon( void )
{
	Release( ) ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::Initialize
	( PEGL_IMAGE_INFO pDstImage, PCEGL_RECT pClipRect,
		PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos,
		DWORD dwHeapSize, DWORD dwPolyLimit,
		E3DRenderPolygon::MultiRenderingFlag mrfFlag )
{
	if ( m_hRenderPoly == NULL )
	{
		//
		// レンダリングオブジェクト作成
		//
		CreateRenderingObject() ;
		//
		// メモリ確保
		//
		SetRenderingHeapSize( dwHeapSize, dwPolyLimit ) ;
		//
		// マルチプロセッサレンダリング初期化
		//
		SetRenderingProcessorCount( mrfFlag ) ;
	}
	//
	// レンダリングターゲットを設定する
	//
	return	SetRenderTarget( pDstImage, pClipRect, pZBuffer, pScreenPos ) ;
}

// リソース開放
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::Release( void )
{
	//
	// スレッド終了
	//
	m_lstThreads.RemoveAll( ) ;
	m_mrfRenderingFlag = mrfSingle ;
	//
	// オブジェクト解放
	//
	ReleaseRenderingHeap() ;
	//
	if ( m_hRenderPoly != NULL )
	{
		m_hRenderPoly->Release( ) ;
		m_hRenderPoly = NULL ;
	}
	//
	if ( m_pLightEntries != NULL )
	{
		delete []	m_pLightEntries ;
		m_pLightEntries = NULL ;
	}
	if ( m_pShadowMapEntries != NULL )
	{
		delete []	m_pShadowMapEntries ;
		m_pShadowMapEntries = NULL ;
	}
	m_nLightCount = 0 ;
	//
	AttachGPUInterface( NULL ) ;
	//
	return	eslErrSuccess ;
}

// レンダリングオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
HEGL_RENDER_POLYGON E3DRenderPolygon::CreateRenderingObject( void )
{
	if ( m_hRenderPoly == NULL )
	{
		m_hRenderPoly = ::eglCreateRenderPolygon( ) ;
	}
	return	m_hRenderPoly ;
}

// レンダリングターゲットを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::SetRenderTarget
	( PEGL_IMAGE_INFO pDstImage, PCEGL_RECT pClipRect,
		PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos )
{
	//
	// 出力先バッファ情報設定
	//
	m_pDstImage = pDstImage ;
	m_pZBuffer = pZBuffer ;
	if ( pClipRect != NULL )
	{
		m_rectDst = *pClipRect ;
	}
	else
	{
		ESLAssert( pDstImage != NULL ) ;
		if ( pDstImage == NULL )
		{
			return	eslErrInvalidParam ;
		}
		m_rectDst.left = 0 ;
		m_rectDst.top = 0 ;
		m_rectDst.right = pDstImage->dwImageWidth - 1 ;
		m_rectDst.bottom = pDstImage->dwImageHeight - 1 ;
	}
	ESLAssert( pScreenPos != NULL ) ;
	if ( pScreenPos == NULL )
	{
		return	eslErrInvalidParam ;
	}
	if ( m_hRenderPoly != NULL )
	{
		CreateRenderingObject() ;
	}
	return	m_hRenderPoly->Initialize
		( pDstImage, pClipRect, pZBuffer, pScreenPos ) ;
}

// ヒープを設定する
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::SetRenderingHeapSize( DWORD dwHeapSize, DWORD dwPolyLimit )
{
	ReleaseRenderingHeap() ;
	//
	ESLAssert( m_hLocalHeap == NULL ) ;
	ESLAssert( m_hStackHeap == NULL ) ;
	m_hLocalHeap = ::eslHeapCreate( 0, 0, ESL_HEAP_NO_SERIALIZE ) ;
	m_hStackHeap = ::eslStackHeapCreate( dwHeapSize, dwHeapSize, 0 ) ;
	m_dwStackSize = dwHeapSize ;
	//
	for ( int i = 0; i < viewCount; i ++ )
	{
		m_plObject[i].Allocate( m_hLocalHeap, dwPolyLimit ) ;
		m_plShadow[i].Allocate( m_hLocalHeap, dwPolyLimit ) ;
		m_plReflect[i].Allocate( m_hLocalHeap, dwPolyLimit ) ;
		m_plGlobalRef[i].Allocate( m_hLocalHeap, dwPolyLimit ) ;
	}
}

// ヒープを解放する
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::ReleaseRenderingHeap( void )
{
	if ( m_hLocalHeap != NULL )
	{
		::eslHeapDestroy( m_hLocalHeap ) ;
		m_hLocalHeap = NULL ;
	}
	if ( m_hStackHeap != NULL )
	{
		::eslStackHeapDestroy( m_hStackHeap ) ;
		m_hStackHeap = NULL ;
	}
	for ( int i = 0; i < viewCount; i ++ )
	{
		m_plObject[i].Release( ) ;
		m_plShadow[i].Release( ) ;
		m_plReflect[i].Release( ) ;
		m_plGlobalRef[i].Release( ) ;
	}
}

// レンダリングプロセッサ設定
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::SetRenderingProcessorCount( MultiRenderingFlag mrfFlag )
{
	//
	// 実装プロセッサ数取得
	//
	DWORD	dwProcessorCount = ESLThread::GetLogicalProcessorCount() ;
	//
	// マルチプロセッサレンダリング初期化
	//
	m_mrfRenderingFlag = mrfFlag ;
	m_lstThreads.RemoveAll( ) ;
	//
	DWORD	dwCount = dwProcessorCount ;
	DWORD	dwPriorityThreadhold = dwProcessorCount ;
	if ( mrfFlag == mrfSingle )
	{
		dwCount = 1 ;
	}
	else if ( mrfFlag == mrfDual )
	{
		dwCount = 2 ;
	}
	if ( m_pGPUInterface != NULL )
	{
		dwCount ++ ;
		dwPriorityThreadhold -- ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		RENDER_THREAD *	prt = new RENDER_THREAD( i ? m_dwStackSize : 0 ) ;
		m_lstThreads.Add( prt ) ;
		prt->pRenderPoly = this ;
		if ( i )
		{
			prt->hThread = ::CreateThread
				( NULL, 0, &E3DRenderPolygon::RenderThreadProc,
										prt, 0, &(prt->dwThreadID) ) ;
			::WaitForSingleObject( prt->hRenderEvent, INFINITE ) ;
			if ( i < dwPriorityThreadhold )
			{
				::SetThreadPriority
					( prt->hThread, THREAD_PRIORITY_HIGHEST ) ;
			}
			else
			{
				::SetThreadPriority
					( prt->hThread, THREAD_PRIORITY_LOWEST ) ;
			}
		}
		else
		{
			prt->dwThreadID = ::GetCurrentThreadId( ) ;
		}
	}
}

// ビュー選択
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::SetCurrentView( int iView )
{
	if ( m_fBeginBuildModel )
	{
		EndBuildModel() ;
	}
	m_iCurrentView = iView ;
}

// モデルエントリ追加開始
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::BeginBuildModel
		( bool fMultiThread, DWORD dwExceptionShadeFlags )
{
	if ( !m_fBeginBuildModel )
	{
		m_dwExceptionShadeFlags = dwExceptionShadeFlags ;
		//
		if ( m_eventAddModel.Handle() == NULL )
		{
			m_eventAddModel.CreateEvent( false ) ;
		}
		else
		{
			m_eventAddModel.ResetEvent( ) ;
		}
		if ( m_eventEndBuildModel.Handle() == NULL )
		{
			m_eventEndBuildModel.CreateEvent( false ) ;
		}
		else
		{
			m_eventEndBuildModel.ResetEvent( ) ;
		}
		ESLAssert( m_queAddModel.GetSize() == 0 ) ;
		//
		DWORD	dwProcessorCount = ESLThread::GetLogicalProcessorCount() ;
		m_dwUsingThreads = 0 ;
		m_fMultiThreadBuildModel = false ;
		//
		if ( fMultiThread )
		{
			for ( DWORD i = 1; i < m_lstThreads.GetSize(); i ++ )
			{
				if ( i >= dwProcessorCount )
				{
					break ;
				}
				RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
				ESLAssert( prt != NULL ) ;
				//
				::ResetEvent( prt->hRendered ) ;
				::PostThreadMessage
					( prt->dwThreadID, rtmBeginBuildModel, 0, 0 ) ;
				//
				m_dwUsingThreads = i + 1 ;
				m_fMultiThreadBuildModel = true ;
			}
		}
		m_fBeginBuildModel = true ;
	}
	return	eslErrSuccess ;
}

// モデルエントリ追加終了
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::EndBuildModel( void )
{
	if ( m_fBeginBuildModel )
	{
		BuildModelQueue( m_hStackHeap ) ;
		m_eventEndBuildModel.SetEvent() ;
		//
		DWORD	i ;
		for ( i = 1; i < m_dwUsingThreads; i ++ )
		{
			RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
			ESLAssert( prt != NULL ) ;
			::WaitForSingleObject( prt->hRendered, INFINITE ) ;
		}
		m_dwUsingThreads = 0 ;
		m_fBeginBuildModel = false ;
		m_fMultiThreadBuildModel = false ;
		m_dwExceptionShadeFlags = 0 ;
	}
	return	eslErrSuccess ;
}

// モデル追加
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::AddModel
	( E3DModelJoint & joint,
		const E3D_COLOR * pColor, unsigned int nTransparency,
		bool fAddSubJoint, DWORD dwExceptionShadeFlags )
{
	E3DModelJoint *	pParent = joint.m_parent ;
	if ( pParent == NULL )
	{
		pParent = &m_vpjView ;
	}
	joint.RefreshJoint( ) ;
	joint.TransformJoint( *pParent ) ;
	//
	dwExceptionShadeFlags |= m_dwExceptionShadeFlags ;
	//
	// プリミティブを順次追加
	//
	E3DPolygonModel *	pModel ;
	ESLAssert( m_hRenderPoly != NULL ) ;
	//
	int		i, nCount ;
	nCount = joint.m_models.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		pModel = joint.m_models.GetAt( i ) ;
		if ( pModel == NULL )
		{
			continue ;
		}
		if ( !m_fMultiThreadBuildModel )
		{
			//
			// モデルの座標変換
			//
			joint.TransformModel( *pModel ) ;
			//
			// モデルのプリミティブを順次追加
			//
			AddModelEntry
				( m_hStackHeap, pModel,
					pColor, nTransparency, dwExceptionShadeFlags ) ;
		}
		else
		{
			//
			// マルチスレッド非同期処理
			//
			QueueAddModelEntry
				( joint, pModel,
					pColor, nTransparency, dwExceptionShadeFlags ) ;
		}
	}
	//
	// サブジョイントを処理
	//
	if ( fAddSubJoint )
	{
		nCount = joint.m_joints.GetSize( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			E3DModelJoint *	pJoint = joint.m_joints.GetAt( i ) ;
			if ( pJoint == NULL )
				continue ;
			//
			AddModel
				( *pJoint, pColor, nTransparency,
					fAddSubJoint, dwExceptionShadeFlags ) ;
		}
	}
	return	eslErrSuccess ;
}

// モデル構築スレッド
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::BuildModelThreadProc( RENDER_THREAD * prt )
{
	ESLAssert( prt->hStackHeap != NULL ) ;
	if ( prt->hStackHeap == NULL )
	{
		return ;
	}
	HANDLE	hEvent[2] ;
	hEvent[0] = m_eventEndBuildModel.Handle() ;
	hEvent[1] = m_eventAddModel.Handle() ;
	//
	for ( ; ; )
	{
		DWORD	dwResult =
			::WaitForMultipleObjects( 2, hEvent, FALSE, INFINITE ) ;
		if ( dwResult == WAIT_OBJECT_0 )
		{
			break ;
		}
		ESLAssert( prt->hStackHeap != NULL ) ;
		BuildModelQueue( prt->hStackHeap ) ;
	}
}

// モデル追加処理をキューに追加
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::QueueAddModelEntry
	( E3DModelJoint & model,
		E3DPolygonModel * pModel, const E3D_COLOR * pColor,
		unsigned int nTransparency, DWORD dwExceptionShadeFlags )
{
	ESLAssert( m_fBeginBuildModel ) ;
	//
	ADD_MODEL_ENTRY *	pamEntry = new ADD_MODEL_ENTRY ;
	pamEntry->rvmat = model.m_rvmat ;
	pamEntry->vmove = model.m_vmove ;
	pamEntry->pModel = pModel ;
	pamEntry->pColor = NULL ;
	pamEntry->nTransparency = nTransparency ;
	pamEntry->dwExceptionShadeFlags = dwExceptionShadeFlags ;
	//
	if ( pColor != NULL )
	{
		pamEntry->bufColor = *pColor ;
		pamEntry->pColor = &(pamEntry->bufColor) ;
	}
	//
	Lock() ;
	m_queAddModel.Add( pamEntry ) ;
	m_eventAddModel.SetEvent() ;
	Unlock() ;
}

// モデル追加処理
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::AddModelEntry
	( HSTACKHEAP hStackHeap, E3DPolygonModel * pModel,
		const E3D_COLOR * pColor, unsigned int nTransparency,
									DWORD dwExceptionShadeFlags )
{
	bool	fRayTracing =
		(m_dwRayTracingFlags != 0)
			&& (GetFunctionFlags() &
				(E3D_FLAG_RAY_SHADOWING
					| E3D_FLAG_RAY_REFLECTING
					| E3D_FLAG_RAY_REFRACTING)) ;
	//
	PFUNC_LOCK_FUNC	pfnLock ;
	PFUNC_LOCK_FUNC	pfnUnlock ;
	//
	if ( m_fMultiThreadBuildModel )
	{
		pfnLock = &E3DRenderPolygon::Lock ;
		pfnUnlock = &E3DRenderPolygon::Unlock ;
	}
	else
	{
		pfnLock = &E3DRenderPolygon::LockAsync ;
		pfnUnlock = &E3DRenderPolygon::UnlockAsync ;
	}
	int		j, m ;
	bool	fApplyAttr = pColor || nTransparency ;
	m = pModel->GetPrimitiveCount( ) ;
	for ( j = 0; j < m; j ++ )
	{
		//
		// プリミティブからレンダリング用エントリを作成する
		//
		const E3D_PRIMITIVE_POLYGON *	pPrimitive ;
		E3D_POLYGON_ENTRY *				pPolyEntry ;
		//
		pPrimitive = pModel->GetPrimitiveAt( j ) ;
		//
		PE3D_SURFACE_ATTRIBUTE	pAttr = pPrimitive->pSurfaceAttr ;
		if ( pAttr && (pAttr->dwShadingFlags & dwExceptionShadeFlags) )
		{
			continue ;
		}
		if ( !fRayTracing )
		{
			pPolyEntry = m_hRenderPoly->
				CreatePolygonEntry( hStackHeap, pPrimitive ) ;
			if ( pPolyEntry == NULL )
			{
				continue ;
			}
		}
		else
		{
			DWORD	dwResult ;
			pPolyEntry =
				m_hRenderPoly->CreatePolygonEntryRT
					( hStackHeap, pPrimitive, &dwResult ) ;
			if ( pPolyEntry == NULL )
			{
				continue ;
			}
			if ( !(dwResult & E3D_RTCPE_RESULT_NO_SHADOWING) )
			{
				//
				// 陰オブジェクトリストに追加する
				//
				(this->*pfnLock)() ;
				m_plShadow[m_iCurrentView].AddEntry
								( m_hLocalHeap, pPolyEntry ) ;
				(this->*pfnUnlock)() ;
			}
			if ( !(dwResult & E3D_RTCPE_RESULT_NO_REFLECTING) )
			{
				//
				// 反射オブジェクトリストに追加する
				//
				if ( !(pPolyEntry->dwShadingFlags
						& E3DSAF_GLOBAL_REFLECT_OBJECT) )
				{
					(this->*pfnLock)() ;
					m_plReflect[m_iCurrentView].AddEntry
									( m_hLocalHeap, pPolyEntry ) ;
					(this->*pfnUnlock)() ;
				}
				else
				{
					(this->*pfnLock)() ;
					m_plGlobalRef[m_iCurrentView].AddEntry
									( m_hLocalHeap, pPolyEntry ) ;
					(this->*pfnUnlock)() ;
				}
			}
		}
		//
		// レンダリングパラメータを修飾する
		//
		pPolyEntry = m_hRenderPoly->
			MakeUpPolygon( hStackHeap, pPolyEntry ) ;
		if ( pPolyEntry == NULL )
		{
			continue ;
		}
		if ( fApplyAttr )
		{
			if ( m_hRenderPoly->ApplyAttribute
					( pPolyEntry, pColor, nTransparency ) )
			{
				continue ;
			}
		}
		//
		// レンダリングリストに追加する
		//
		(this->*pfnLock)() ;
		m_plObject[m_iCurrentView].AddEntry( m_hLocalHeap, pPolyEntry ) ;
		(this->*pfnUnlock)() ;
	}
}

// モデルエントリ待ち行列を処理する
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::BuildModelQueue( HSTACKHEAP hStackHeap )
{
	for ( ; ; )
	{
		//
		// 待ち行列から先頭エントリ取得
		//
		ADD_MODEL_ENTRY *	pamEntry ;
		Lock() ;
		if ( m_queAddModel.GetSize() == 0 )
		{
			Unlock() ;
			return ;
		}
		pamEntry = m_queAddModel.GetAt( 0 ) ;
		m_queAddModel.DetachAt( 0 ) ;
		//
		if ( m_queAddModel.GetSize() == 0 )
		{
			m_eventAddModel.ResetEvent() ;
		}
		Unlock() ;
		//
		// エントリ処理
		//
		ESLAssert( pamEntry != NULL ) ;
		E3DPolygonModel *	pModel = pamEntry->pModel ;
		ESLAssert( pModel != NULL ) ;
		//
		pModel->Lock() ;
		//
		if ( pModel->GetVertexCount() > 0 )
		{
			pamEntry->rvmat.RevolveVectors
				( pModel->GetVertexBuffer(),
					pModel->GetVertexList(),
					&(pamEntry->vmove),
					pModel->GetVertexCount() ) ;
		}
		if ( pModel->GetNormalCount() > 0 )
		{
			pamEntry->rvmat.RevolveVectors
				( pModel->GetNormalBuffer(),
					pModel->GetNormalList(),
					NULL, pModel->GetNormalCount() ) ;
		}
		//
		AddModelEntry
			( hStackHeap, pModel,
				pamEntry->pColor, pamEntry->nTransparency,
							pamEntry->dwExceptionShadeFlags ) ;
		//
		pModel->Unlock() ;
		//
		delete	pamEntry ;
	}
}

// 同期処理用関数
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::Lock( void ) const
{
	m_csSyncContext.Lock() ;
}

void E3DRenderPolygon::Unlock( void ) const
{
	m_csSyncContext.Unlock() ;
}

// 非同期処理用関数
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::LockAsync( void ) const
{
}

void E3DRenderPolygon::UnlockAsync( void ) const
{
}

// プリミティブ追加
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::AddPrimitive
	( const E3D_PRIMITIVE_POLYGON * pPrimitive,
		const E3D_COLOR * pColor,
		unsigned int nTransparency, DWORD dwExceptionShadeFlags,
		const E3D_COLOR * pvVertexColors )
{
	bool	fRayTracing =
		(m_dwRayTracingFlags != 0)
			&& (GetFunctionFlags() &
				(E3D_FLAG_RAY_SHADOWING
					| E3D_FLAG_RAY_REFLECTING
					| E3D_FLAG_RAY_REFRACTING)) ;
	HSTACKHEAP	hStackHeap = m_hStackHeap ;
	//
	PFUNC_LOCK_FUNC	pfnLock ;
	PFUNC_LOCK_FUNC	pfnUnlock ;
	//
	if ( m_fMultiThreadBuildModel )
	{
		pfnLock = &E3DRenderPolygon::Lock ;
		pfnUnlock = &E3DRenderPolygon::Unlock ;
	}
	else
	{
		pfnLock = &E3DRenderPolygon::LockAsync ;
		pfnUnlock = &E3DRenderPolygon::UnlockAsync ;
	}
	//
	// プリミティブ処理
	//
	ESLAssert( m_hRenderPoly != NULL ) ;
	E3D_POLYGON_ENTRY *	pPolyEntry ;
	if ( !fRayTracing )
	{
		pPolyEntry =
			m_hRenderPoly->CreatePolygonEntry( hStackHeap, pPrimitive ) ;
		if ( pPolyEntry == NULL )
		{
			return	eslErrSuccess ;
		}
		if ( pvVertexColors != NULL )
		{
			if ( pPolyEntry->pVertexColors == NULL )
			{
				pPolyEntry->pVertexColors =
					(E3D_COLOR*) eslStackHeapAllocate
						( hStackHeap, pPolyEntry->dwVertexCount
											* sizeof(E3D_COLOR) ) ;
			}
			eslMoveMemory
				( pPolyEntry->pVertexColors,
					pvVertexColors,
					pPolyEntry->dwVertexCount * sizeof(E3D_COLOR) ) ;
		}
	}
	else
	{
		DWORD	dwResult ;
		pPolyEntry =
			m_hRenderPoly->CreatePolygonEntryRT
				( hStackHeap, pPrimitive, &dwResult ) ;
		if ( pPolyEntry == NULL )
		{
			return	eslErrSuccess ;
		}
		if ( pvVertexColors != NULL )
		{
			if ( pPolyEntry->pVertexColors == NULL )
			{
				pPolyEntry->pVertexColors =
					(E3D_COLOR*) eslStackHeapAllocate
						( hStackHeap, pPolyEntry->dwVertexCount
											* sizeof(E3D_COLOR) ) ;
			}
			eslMoveMemory
				( pPolyEntry->pVertexColors,
					pvVertexColors,
					pPolyEntry->dwVertexCount * sizeof(E3D_COLOR) ) ;
		}
		if ( !(dwResult & E3D_RTCPE_RESULT_NO_SHADOWING) )
		{
			//
			// 陰オブジェクトリストに追加する
			//
			(this->*pfnLock)() ;
			m_plShadow[m_iCurrentView].AddEntry
							( m_hLocalHeap, pPolyEntry ) ;
			(this->*pfnUnlock)() ;
		}
		if ( !(dwResult & E3D_RTCPE_RESULT_NO_REFLECTING) )
		{
			//
			// 反射オブジェクトリストに追加する
			//
			if ( !(pPolyEntry->dwShadingFlags
					& E3DSAF_GLOBAL_REFLECT_OBJECT) )
			{
				(this->*pfnLock)() ;
				m_plReflect[m_iCurrentView].AddEntry
								( m_hLocalHeap, pPolyEntry ) ;
				(this->*pfnUnlock)() ;
			}
			else
			{
				(this->*pfnLock)() ;
				m_plGlobalRef[m_iCurrentView].AddEntry
								( m_hLocalHeap, pPolyEntry ) ;
				(this->*pfnUnlock)() ;
			}
		}
	}
	pPolyEntry = m_hRenderPoly->MakeUpPolygon( hStackHeap, pPolyEntry ) ;
	if ( pPolyEntry == NULL )
	{
		return	eslErrSuccess ;
	}
	if ( m_hRenderPoly->ApplyAttribute( pPolyEntry, pColor, nTransparency ) )
	{
		return	eslErrSuccess ;
	}
	//
	// ポリゴン追加
	//
	(this->*pfnLock)() ;
	m_plObject[m_iCurrentView].AddEntry( m_hLocalHeap, pPolyEntry ) ;
	(this->*pfnUnlock)() ;
	//
	return	eslErrSuccess ;
}

// レンダリング準備
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::PrepareRendering( void )
{
	if ( m_hRenderPoly == NULL )
	{
		return	eslErrSuccess ;
	}
	if ( m_fBeginBuildModel )
	{
		EndBuildModel() ;
	}
	//
	E3DRevMatrix	matView = m_vpjView.Matrix() ;
	m_hRenderPoly->SetEnvironmentMapping
		( &matView, (m_fGlobalEnvironment ? &m_envmapGlobal : NULL) ) ;
	//
	ESLError	err =
		m_hRenderPoly->SortPolygonEntry
			( m_plObject[m_iCurrentView].pEntries,
				m_plObject[m_iCurrentView].dwCount, m_dwSortingFlags ) ;
	if ( m_dwRayTracingFlags != 0 )
	{
		m_hRenderPoly->AttachRayTracingTarget
			( m_hStackHeap,
				m_plShadow[m_iCurrentView].pEntries,
					m_plShadow[m_iCurrentView].dwCount,
				m_plReflect[m_iCurrentView].pEntries,
					m_plReflect[m_iCurrentView].dwCount,
				m_plGlobalRef[m_iCurrentView].pEntries,
					m_plGlobalRef[m_iCurrentView].dwCount ) ;
	}
	return	err ;
}

// レンダリング実行
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::RenderAllPolygon
	( HEGL_RENDER_POLYGON hRenderPoly, bool fRayTracing, int nThreadLines )
{
	if ( hRenderPoly == NULL )
	{
		ESLAssert( m_hRenderPoly != NULL ) ;
		hRenderPoly = m_hRenderPoly ;
	}
	if ( hRenderPoly == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( m_mrfRenderingFlag != mrfSingle )
	{
		//
		// マルチプロセッサレンダリング
		//////////////////////////////////////////////////////////////////////
		if ( !fRayTracing )
		{
			EGL_DRAW_DEST	ddst ;
			HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
			hDraw->GetDestination( &ddst ) ;
			EGL_SIZE	sizeDraw ;
			sizeDraw.w = ddst.rectDstClip.right - ddst.rectDstClip.left + 1 ;
			sizeDraw.h = ddst.rectDstClip.bottom - ddst.rectDstClip.top + 1 ;
			if ( (sizeDraw.w <= 0) || (sizeDraw.h <= 0) )
			{
				return	eslErrGeneral ;
			}
			DWORD	dwThreadLines = 4096 / sizeDraw.w ;
			if ( dwThreadLines < 1 )
			{
				dwThreadLines = 1 ;
			}
			DWORD	dwUseThreadCount = sizeDraw.h / dwThreadLines ;
			if ( dwUseThreadCount < 1 )
			{
				dwUseThreadCount = 1 ;
			}
			else if ( dwUseThreadCount > m_lstThreads.GetSize() )
			{
				dwUseThreadCount = m_lstThreads.GetSize( ) ;
			}
			if ( (nThreadLines <= 0)
				|| ((int) (sizeDraw.h / dwUseThreadCount) <= nThreadLines) )
			{
				//
				// レンダリング実行開始（画面をスレッド数で分割）
				//////////////////////////////////////////////////////////////
				int		yLastLine = ddst.rectDstClip.top ;
				DWORD	i ;
				m_dwUsingThreads = dwUseThreadCount ;
				//
				RENDER_THREAD *	prtPrim = m_lstThreads.GetAt( 0 ) ;
				ESLAssert( prtPrim != NULL ) ;
				prtPrim->dwThreadID = ::GetCurrentThreadId( ) ;
				//
				for ( i = 0; i < dwUseThreadCount; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::ResetEvent( prt->hRendered ) ;
				}
				for ( i = 0; i < dwUseThreadCount; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					//
					int		yNextLine =
								ddst.rectDstClip.top
									+ sizeDraw.h * (i + 1) / dwUseThreadCount ;
					ESLAssert( yNextLine > yLastLine ) ;
					EGL_RECT	rectClip ;
					rectClip.left = ddst.rectDstClip.left ;
					rectClip.top = yLastLine ;
					rectClip.right = ddst.rectDstClip.right ;
					rectClip.bottom = yNextLine - 1 ;
					yLastLine = yNextLine ;
					//
					prt->hRenderPoly->InitializeToReference
							( hRenderPoly, &rectClip, 0 ) ;
					//
					::ResetEvent( prt->hRenderEvent ) ;
					//
					if ( i )
					{
						::PostThreadMessage
							( prt->dwThreadID, rtmRender, i, 0 ) ;
					}
				}
				RenderAllPolygonProc( prtPrim->hRenderPoly, 0 ) ;
				::SetEvent( prtPrim->hRendered ) ;
				OnAfterAllRenderPolygon( prtPrim->hRenderPoly, 0 ) ;
				//
				// 全ての処理が終了するまで待機
				//
				for ( i = 1; i < dwUseThreadCount; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::WaitForSingleObject( prt->hRenderEvent, INFINITE ) ;
				}
				m_dwUsingThreads = 0 ;
			}
			else
			{
				//
				// レンダリング実行開始（画面区画を動的にスケジュール）
				//////////////////////////////////////////////////////////////
				m_rcContext.hRender = hRenderPoly ;
				m_rcContext.ddst = ddst ;
				m_rcContext.yNextLine = ddst.rectDstClip.top ;
				m_rcContext.nThreadLines = nThreadLines ;
				//
				m_dwUsingThreads = m_lstThreads.GetSize() ;
				//
				DWORD	i ;
				for ( i = 0; i < m_dwUsingThreads; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::ResetEvent( prt->hRendered ) ;
				}
				for ( i = 1; i < m_dwUsingThreads; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::ResetEvent( prt->hRenderEvent ) ;
					::PostThreadMessage
						( prt->dwThreadID, rtmRenderDynamically, i, 0 ) ;
				}
				//
				RENDER_THREAD *	prtPrim = m_lstThreads.GetAt( 0 ) ;
				ESLAssert( prtPrim != NULL ) ;
				prtPrim->dwThreadID = ::GetCurrentThreadId( ) ;
				//
				if ( (m_dwRayTracingFlags != 0) && (m_pGPUInterface != NULL) )
				{
					m_pGPUInterface->AttachThread() ;
					//
					PrepareGPURayTracing
						( (ddst.rectDstClip.right
							- ddst.rectDstClip.left + 1) * nThreadLines ) ;
					//
					if ( PrepareNextRenderingRect( prtPrim ) )
					{
						EGL_DRAW_DEST	ddstTemp ;
						prtPrim->hRenderPoly->
							GetDrawImage()->GetDestination( &ddstTemp ) ;
						//
						RenderByGPURayTracing
							( prtPrim->hRenderPoly, ddstTemp.rectDstClip ) ;
						//
						while ( PrepareNextRenderingRect( prtPrim ) )
						{
							prtPrim->hRenderPoly->
								GetDrawImage()->GetDestination( &ddstTemp ) ;
							if ( !RenderByGPURayTracing
								( prtPrim->hRenderPoly, ddstTemp.rectDstClip ) )
							{
								GetResultOfGPURayTracing( prtPrim->hRenderPoly ) ;
							}
						}
						//
						GetResultOfGPURayTracing( prtPrim->hRenderPoly ) ;
					}
					m_pGPUInterface->DetachThread() ;
				}
				else
				{
					while ( PrepareNextRenderingRect( prtPrim ) )
					{
						RenderAllPolygonProc( prtPrim->hRenderPoly, 0 ) ;
					}
				}
				::SetEvent( prtPrim->hRendered ) ;
				//
				WaitUntilAllThread3DRendering( ) ;
				//
				// レンダリング後の処理
				//
				m_rcContext.yNextLine = ddst.rectDstClip.top ;
				//
				for ( i = 1; i < m_dwUsingThreads; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::ResetEvent( prt->hRenderEvent ) ;
					::PostThreadMessage
						( prt->dwThreadID,
							rtmAfterRenderingDynamically, i, 0 ) ;
				}
				while ( PrepareNextRenderingRect( prtPrim ) )
				{
					OnAfterAllRenderPolygon( prtPrim->hRenderPoly, 0 ) ;
				}
				//
				// 全ての処理が終了するまで待機
				//
				for ( i = 1; i < m_dwUsingThreads; i ++ )
				{
					RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
					ESLAssert( prt != NULL ) ;
					::WaitForSingleObject( prt->hRenderEvent, INFINITE ) ;
				}
				m_dwUsingThreads = 0 ;
			}
		}
		else
		{
			//
			// レイトレーシング
			//////////////////////////////////////////////////////////////////
			RENDER_THREAD *	prtPrim = m_lstThreads.GetAt( 0 ) ;
			ESLAssert( prtPrim != NULL ) ;
			prtPrim->dwThreadID = ::GetCurrentThreadId( ) ;
			prtPrim->hRenderPoly->InitializeToReference( hRenderPoly, NULL ) ;
			prtPrim->hRenderPoly->PrepareRenderRayTracing( NULL ) ;
			//
			DWORD	i ;
			DWORD	dwThreadCount = m_lstThreads.GetSize() ;
			m_dwUsingThreads = dwThreadCount ;
			for ( i = 0; i < m_dwUsingThreads; i ++ )
			{
				RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
				ESLAssert( prt != NULL ) ;
				::ResetEvent( prt->hRendered ) ;
			}
			for ( i = 1; i < dwThreadCount; i ++ )
			{
				RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
				ESLAssert( prt != NULL ) ;
				//
				prt->hRenderPoly->
					InitializeToReference( hRenderPoly, NULL ) ;
				//
				::ResetEvent( prt->hRenderEvent ) ;
				::PostThreadMessage
					( prt->dwThreadID, rtmRayTracing,
							i, (LPARAM) prtPrim->hRenderPoly ) ;
			}
			if ( m_pGPUInterface != NULL )
			{
				RenderRayTracingByGPUThread
					( hRenderPoly, prtPrim, true ) ;
			}
			else
			{
				prtPrim->hRenderPoly->RenderRayTracing( NULL ) ;
			}
			::SetEvent( prtPrim->hRendered ) ;
			//
			// 全ての処理が終了するまで待機
			//
			for ( i = 1; i < dwThreadCount; i ++ )
			{
				RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
				ESLAssert( prt != NULL ) ;
				::WaitForSingleObject( prt->hRenderEvent, INFINITE ) ;
			}
			OnAfterAllRenderPolygon( prtPrim->hRenderPoly, 0 ) ;
			m_dwUsingThreads = 0 ;
		}
	}
	else
	{
		//
		// シングルプロセッサレンダリング処理
		//////////////////////////////////////////////////////////////////////
		if ( !fRayTracing )
		{
			RenderAllPolygonProc( hRenderPoly, 0 ) ;
			OnAfterAllRenderPolygon( hRenderPoly, 0 ) ;
		}
		else
		{
			if ( m_pGPUInterface != NULL )
			{
				RENDER_THREAD *	prtPrim = m_lstThreads.GetAt( 0 ) ;
				ESLAssert( prtPrim != NULL ) ;
				prtPrim->dwThreadID = ::GetCurrentThreadId( ) ;
				prtPrim->hRenderPoly->InitializeToReference( hRenderPoly, NULL ) ;
				prtPrim->hRenderPoly->PrepareRenderRayTracing( NULL ) ;
				//
				RenderRayTracingByGPUThread( hRenderPoly, prtPrim, false ) ;
			}
			else
			{
				hRenderPoly->PrepareRenderRayTracing( NULL ) ;
				hRenderPoly->RenderRayTracing( NULL ) ;
			}
			OnAfterAllRenderPolygon( hRenderPoly, 0 ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

void E3DRenderPolygon::RenderAllPolygonProc
	( HEGL_RENDER_POLYGON hRenderPoly, int iThread )
{
	DWORD	i, dwCount ;
	dwCount = m_plObject[m_iCurrentView].dwCount ;
	for ( i = 0; i < dwCount; i ++ )
	{
		PE3D_POLYGON_ENTRY	pPolyEntry =
						m_plObject[m_iCurrentView].pEntries[i] ;
		if ( pPolyEntry == NULL )
		{
			continue ;
		}
		if ( !hRenderPoly->PrepareRender( pPolyEntry ) )
		{
			hRenderPoly->RenderPolygon( ) ;
		}
	}
}

// レンダリング後処理関数
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::OnAfterAllRenderPolygon
	( HEGL_RENDER_POLYGON hRenderPoly, int iThread )
{
}

// 次のレンダリング領域をセットアップ
//////////////////////////////////////////////////////////////////////////////
bool E3DRenderPolygon::PrepareNextRenderingRect( RENDER_THREAD * prt )
{
	bool	fNext = false ;
	m_csSyncContext.Lock() ;
	if ( m_rcContext.yNextLine <= m_rcContext.ddst.rectDstClip.bottom )
	{
		EGL_RECT	rctClip ;
		rctClip.left = m_rcContext.ddst.rectDstClip.left ;
		rctClip.top = m_rcContext.yNextLine ;
		rctClip.right = m_rcContext.ddst.rectDstClip.right ;
		rctClip.bottom = m_rcContext.yNextLine + m_rcContext.nThreadLines - 1 ;
		//
		if ( rctClip.bottom > m_rcContext.ddst.rectDstClip.bottom )
		{
			rctClip.bottom = m_rcContext.ddst.rectDstClip.bottom ;
		}
		m_rcContext.yNextLine = rctClip.bottom + 1 ;
		//
		prt->hRenderPoly->InitializeToReference
					( m_rcContext.hRender, &rctClip, 0 ) ;
		//
		fNext = true ;
	}
	m_csSyncContext.Unlock() ;
	return	fNext ;
}

// GPU でのレイトレーシングスレッド処理
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::RenderRayTracingByGPUThread
	( HEGL_RENDER_POLYGON hRenderPoly,
		RENDER_THREAD * prtPrim, bool fMultiThread )
{
	ESLAssert( m_pGPUInterface != NULL ) ;
	m_pGPUInterface->AttachThread() ;
	//
	EGL_DRAW_DEST	ddst ;
	hRenderPoly->GetDrawImage()->GetDestination( &ddst ) ;
	//
	int	nWidth = (ddst.rectDstClip.right
					- ddst.rectDstClip.left + 1) ;
	int	nLines = 640 * 8 / nWidth ;
	if ( nLines < 2 )
	{
		nLines = 2 ;
	}
	else
	{
		nLines &= ~0x01 ;
	}
	//
	PrepareGPURayTracing( nWidth * nLines ) ;
	//
	EGL_RECT	rctRender ;
	if ( prtPrim->hRenderPoly->
			AllocateRayTraceRect
				( &rctRender, nLines, 0 ) > 0 )
	{
		RenderByGPURayTracing
			( prtPrim->hRenderPoly, rctRender ) ;
		//
		int	nOffsetLines = fMultiThread ? nLines : 0 ;
		while ( prtPrim->hRenderPoly->
				AllocateRayTraceRect
					( &rctRender, nLines, nOffsetLines ) > 0 )
		{
			if ( !RenderByGPURayTracing
					( prtPrim->hRenderPoly, rctRender ) )
			{
				GetResultOfGPURayTracing( prtPrim->hRenderPoly ) ;
			}
		}
		//
		GetResultOfGPURayTracing( prtPrim->hRenderPoly ) ;
	}
	//
	m_pGPUInterface->DetachThread() ;
}

// GPU レイトレーシング準備
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::PrepareGPURayTracing( int nMaxPixelCount )
{
	ESLAssert( m_pGPUInterface != NULL ) ;
	if ( m_hGPUBuffer == NULL )
	{
		m_hGPUBuffer = m_pGPUInterface->CreateRayTracingBuffer() ;
	}
	m_pGPUInterface->SetRayTracingTarget
		( m_hGPUBuffer,
			m_plShadow[m_iCurrentView].pEntries,
				m_plShadow[m_iCurrentView].dwCount,
			m_plReflect[m_iCurrentView].pEntries,
				m_plReflect[m_iCurrentView].dwCount,
			m_plGlobalRef[m_iCurrentView].pEntries,
				m_plGlobalRef[m_iCurrentView].dwCount ) ;
	m_pGPUInterface->SetLightEntries
		( m_hGPUBuffer, m_nLightCount, m_pLightEntries ) ;
	//
	ESLAssert( m_dwRayTracingFlags != 0 ) ;
	m_pGPUInterface->PrepareToTraceRays
		( m_hGPUBuffer,
			m_rrtpParam.rShadowingDistance,
			m_rrtpParam.rRayTracingDistance,
			nMaxPixelCount, m_rrtpParam.dwRayReflectCount ) ;
}

// GPU レイトレーシング
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::RenderByGPURayTracing
		( HEGL_RENDER_POLYGON hRenderPoly, const EGL_RECT & rctRender )
{
	ESLAssert( m_pGPUInterface != NULL ) ;
	//
	ESLAssert( m_dwRayTracingFlags != 0 ) ;
	EGLImageRect	irRender = rctRender ;
	ESLError	err =
		m_pGPUInterface->BeginTraceRaysRect
			( m_hGPUBuffer, irRender,
				hRenderPoly->GetScreenPos(),
				m_rrtpParam.dwRayReflectCount ) ;
	//
	return	err ;
}

// GPU レイトレーシング結果取得
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::GetResultOfGPURayTracing
	( HEGL_RENDER_POLYGON hRenderPoly )
{
	ESLAssert( m_pGPUInterface != NULL ) ;
	//
	E3D_GPU_PLUGIN_RENDER_RESULT	result ;
	ESLError	err =
		m_pGPUInterface->GetResultToTraceRays( m_hGPUBuffer, result ) ;
	if ( !err )
	{
		EGL_DRAW_DEST	ddst ;
		hRenderPoly->GetDrawImage()->GetDestination( &ddst ) ;
		//
		EGLRect	rctRender = result.rctRender ;
		WriteFromGPUResultBuffer
			( ddst.pDstImage, rctRender, result.prgbaImage ) ;
		//
		if ( ddst.pZBuffer != NULL )
		{
			WriteFromGPUResultBuffer
				( ddst.pZBuffer,
					rctRender, (const DWORD *) result.pzBuffer ) ;
		}
	}
	return	err ;
}

// 結果を画像バッファに書き込む
//////////////////////////////////////////////////////////////////////////////
void E3DRenderPolygon::WriteFromGPUResultBuffer
	( PEGL_IMAGE_INFO pImageInf,
		const EGL_RECT & rctRender, const DWORD * pdwBuffer )
{
	ESLAssert( pImageInf != NULL ) ;
	ESLAssert( pImageInf->dwBitsPerPixel == 32 ) ;
	int		nWidth = (rctRender.right - rctRender.left + 1) ;
	BYTE *	pbytLine = (BYTE*) pImageInf->ptrImageArray ;
	pbytLine +=
		pImageInf->dwBytesPerLine * rctRender.top
						+ rctRender.left * sizeof(DWORD) ;
	//
	if ( pImageInf->dwBytesPerLine == (SDWORD) (nWidth * sizeof(DWORD)) )
	{
		int	nHeight = (rctRender.bottom - rctRender.top + 1) ;
		::eslMoveMemory
			( pbytLine, pdwBuffer,
				nWidth * nHeight * sizeof(DWORD) ) ;
	}
	else
	{
		const DWORD *	pdwSrcBuf = pdwBuffer ;
		for ( int y = rctRender.top; y <= rctRender.bottom; y ++ )
		{
			DWORD *	pdwDst = (DWORD*) pbytLine ;
			for ( int i = 0; i < nWidth; i ++ )
			{
				pdwDst[i] = pdwSrcBuf[i] ;
			}
			pdwSrcBuf += nWidth ;
			pbytLine += pImageInf->dwBytesPerLine ;
		}
	}
}

// 現在のレンダリングスレッド取得
//////////////////////////////////////////////////////////////////////////////
int E3DRenderPolygon::GetCurrentRenderingThread( void ) const
{
	if ( m_mrfRenderingFlag != mrfSingle )
	{
		DWORD	dwThreadID = ::GetCurrentThreadId( ) ;
		for ( DWORD i = 0; i < m_dwUsingThreads; i ++ )
		{
			RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
			if ( prt != NULL )
			{
				if ( prt->dwThreadID == dwThreadID )
				{
					return	(int) i ;
				}
			}
		}
		return	-1 ;
	}
	return	0 ;
}

// 現在のレンダリングスレッドの描画用オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
HEGL_RENDER_POLYGON E3DRenderPolygon::GetCurrentThreadRenderer( void ) const
{
	if ( m_mrfRenderingFlag != mrfSingle )
	{
		DWORD	dwThreadID = ::GetCurrentThreadId( ) ;
		for ( DWORD i = 0; i < m_dwUsingThreads; i ++ )
		{
			RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
			if ( prt != NULL )
			{
				if ( prt->dwThreadID == dwThreadID )
				{
					return	prt->hRenderPoly ;
				}
			}
		}
	}
	return	m_hRenderPoly ;
}

// 全スレッドの3Dレンダリング完了を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::WaitUntilAllThread3DRendering( DWORD dwTimeout ) const
{
	if ( m_mrfRenderingFlag != mrfSingle )
	{
		DWORD	dwThreadID = ::GetCurrentThreadId( ) ;
		DWORD	dwBeginTime = ::GetCurrentTime() ;
		for ( DWORD i = 0; i < m_dwUsingThreads; i ++ )
		{
			RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
			if ( prt != NULL )
			{
				if ( prt->dwThreadID != dwThreadID )
				{
					DWORD	dwWaitTime = dwTimeout ;
					if ( dwWaitTime != INFINITE )
					{
						DWORD	dwPastTime =
							::GetCurrentTime() - dwBeginTime ;
						if ( dwPastTime < dwTimeout )
						{
							dwWaitTime = dwTimeout - dwPastTime ;
						}
						else
						{
							dwWaitTime = 0 ;
						}
					}
					if ( ::WaitForSingleObject
						( prt->hRendered, dwWaitTime ) == WAIT_TIMEOUT )
					{
						return	eslErrTimeout ;
					}
				}
			}
		}
	}
	return	eslErrSuccess ;
}

// ポリゴンを消去
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::FlushAllPolygon( void )
{
	if ( (m_hRenderPoly != NULL)
		&& (m_plShadow[m_iCurrentView].dwCount
			|| m_plReflect[m_iCurrentView].dwCount
			|| m_plGlobalRef[m_iCurrentView].dwCount ) )
	{
		m_hRenderPoly->AttachRayTracingTarget
			( m_hStackHeap, NULL, 0, NULL, 0, NULL, 0 ) ;
	}
	if ( m_hStackHeap != NULL )
	{
		::eslStackHeapFree( m_hStackHeap ) ;
	}
	m_plObject[m_iCurrentView].dwCount = 0 ;
	m_plShadow[m_iCurrentView].dwCount = 0 ;
	m_plReflect[m_iCurrentView].dwCount = 0 ;
	m_plGlobalRef[m_iCurrentView].dwCount = 0 ;
	//
	ESLAssert( !m_fBeginBuildModel ) ;
	unsigned int	i, nCount ;
	nCount = m_lstThreads.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
		ESLAssert( prt != NULL ) ;
		if ( prt->hStackHeap != NULL )
		{
			::eslStackHeapFree( prt->hStackHeap ) ;
		}
	}
	return	eslErrSuccess ;
}

// 大域環境マッピングを設定
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::SetGlobalEnvironmentMapping
		( const E3D_ENVIRONMENT_MAPPING * envmap )
{
	if ( envmap != NULL )
	{
		m_envmapGlobal = *envmap ;
		m_fGlobalEnvironment = true ;
	}
	else
	{
		m_fGlobalEnvironment = false ;
	}
	return	eslErrSuccess ;
}

// ライトを設定
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::SetLightEntries
		( unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries )
{
	ESLAssert( m_hRenderPoly != NULL ) ;
	ESLAssert( m_hStackHeap != NULL ) ;
	ESLError	errResult ;
	//
	if ( nLightCount > m_nLightCount )
	{
		if ( m_pLightEntries != NULL )
		{
			delete [] m_pLightEntries ;
		}
		if ( m_pShadowMapEntries != NULL )
		{
			delete [] m_pShadowMapEntries ;
		}
		m_pLightEntries = new E3D_LIGHT_ENTRY[nLightCount] ;
		m_pShadowMapEntries = new E3D_SHADOW_MAP_INFO[nLightCount] ;
	}
	m_nLightCount = nLightCount ;
	//
	E3D_LIGHT_ENTRY *	pLights = m_pLightEntries ;
	for ( unsigned int i = 0; i < nLightCount; i ++ )
	{
		DWORD	dwLightType =
			(pLightEntries[i].dwLightType & E3D_LIGHT_TYPE_MASK) ;
		pLights[i] = pLightEntries[i] ;
		//
		if ( dwLightType == E3D_VECTOR_LIGHT )
		{
			m_vpjView.RevolveVector( pLights[i].vecLight ) ;
			//
			PE3D_SHADOW_MAP_INFO
				pShadowMapInf = pLightEntries[i].ShadowMap.pMapInfo ;
			if ( (pLights[i].dwLightType & E3D_LIGHT_SHADOW_MAP)
										&& (pShadowMapInf != NULL) )
			{
				m_pShadowMapEntries[i] = *pShadowMapInf ;
				pLights[i].ShadowMap.pMapInfo = &(m_pShadowMapEntries[i]) ;
				//
				m_vpjView.TransformPosition
						( m_pShadowMapEntries[i].vLightPos ) ;
				m_vpjView.RevolveVector
						( m_pShadowMapEntries[i].vLightRay ) ;
				m_vpjView.TransformPosition
						( m_pShadowMapEntries[i].vOriginPos ) ;
				m_vpjView.RevolveVector
						( m_pShadowMapEntries[i].vAxisX ) ;
				m_vpjView.RevolveVector
						( m_pShadowMapEntries[i].vAxisY ) ;
			}
			else
			{
				pLights[i].dwLightType &= ~E3D_LIGHT_SHADOW_MAP ;
				pLights[i].vecLight.d = 0 ;
			}
		}
		else if ( dwLightType == E3D_POINT_LIGHT )
		{
			m_vpjView.TransformPosition( pLights[i].vecLight ) ;
		}
	}
	//
	errResult = m_hRenderPoly->PrepareLight
				( m_hStackHeap, nLightCount, pLights ) ;
	//
	return	errResult ;
}

// レイトレーシング中断
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::AbortRenderRayTracing( void )
{
	for ( unsigned int i = 0; i < m_lstThreads.GetSize(); i ++ )
	{
		RENDER_THREAD *	prt = m_lstThreads.GetAt( i ) ;
		ESLAssert( prt != NULL ) ;
		prt->hRenderPoly->AbortRenderRayTracing( ) ;
	}
	if ( m_hRenderPoly != NULL )
	{
		return	m_hRenderPoly->AbortRenderRayTracing( ) ;
	}
	return	eslErrGeneral ;
}

// GPU プラグインを関連付け
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderPolygon::AttachGPUInterface
	( PE3D_GPU_PLUGIN_INTERFACE pGPUInterface )
{
	if ( m_hGPUBuffer != NULL )
	{
		ESLAssert( m_pGPUInterface != NULL ) ;
		m_pGPUInterface->ReleaseBuffer( m_hGPUBuffer ) ;
		m_pGPUInterface->DetachThread() ;
		m_hGPUBuffer = NULL ;
	}
	//
	m_pGPUInterface = pGPUInterface ;
	//
	if ( m_pGPUInterface != NULL )
	{
		m_pGPUInterface->AttachThread() ;
	}
	return	eslErrSuccess ;
}

// レンダリングスレッド
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI E3DRenderPolygon::RenderThreadProc( LPVOID param )
{
	RENDER_THREAD *	prt = (RENDER_THREAD*) param ;
	return	prt->pRenderPoly->RenderingThread( prt ) ;
}

DWORD E3DRenderPolygon::RenderingThread
	( E3DRenderPolygon::RENDER_THREAD * prt )
{
	MSG		msg ;
	::eriInitializeTask( ) ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	::SetEvent( prt->hRenderEvent ) ;
	//
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( msg.message == rtmRender )
		{
			//
			// レンダリング処理
			//
			prt->pRenderPoly->RenderAllPolygonProc
						( prt->hRenderPoly, msg.wParam ) ;
			::SetEvent( prt->hRendered ) ;
			//
			prt->pRenderPoly->OnAfterAllRenderPolygon
						( prt->hRenderPoly, msg.wParam ) ;
			::SetEvent( prt->hRenderEvent ) ;
		}
		else if ( msg.message == rtmRenderDynamically )
		{
			//
			// レンダリング処理（動的割り当て）
			//
			while ( prt->pRenderPoly->PrepareNextRenderingRect( prt ) )
			{
				prt->pRenderPoly->RenderAllPolygonProc
							( prt->hRenderPoly, msg.wParam ) ;
			}
			::SetEvent( prt->hRendered ) ;
			::SetEvent( prt->hRenderEvent ) ;
		}
		else if ( msg.message == rtmAfterRenderingDynamically )
		{
			//
			// レンダリング後処理（動的割り当て）
			//
			while ( prt->pRenderPoly->PrepareNextRenderingRect( prt ) )
			{
				prt->pRenderPoly->OnAfterAllRenderPolygon
							( prt->hRenderPoly, msg.wParam ) ;
			}
			::SetEvent( prt->hRenderEvent ) ;
		}
		else if ( msg.message == rtmRayTracing )
		{
			//
			// レンダリング処理（レイトレーシング）
			//
			HEGL_RENDER_POLYGON	hRenderPoly =
					(HEGL_RENDER_POLYGON) msg.lParam ;
			prt->hRenderPoly->RenderRayTracing( hRenderPoly ) ;
			::SetEvent( prt->hRendered ) ;
//			prt->pRenderPoly->OnAfterAllRenderPolygon
//								( hRenderPoly, msg.wParam ) ;
			::SetEvent( prt->hRenderEvent ) ;
		}
		else if ( msg.message == rtmBeginBuildModel )
		{
			//
			// モデル構築
			//
			prt->pRenderPoly->BuildModelThreadProc( prt ) ;
			::SetEvent( prt->hRendered ) ;
		}
		else if ( msg.message == rtmQuit )
		{
			//
			// スレッド終了
			//
			break ;
		}
	}
	::eriCloseTask( ) ;
	//
	return	0 ;
}
