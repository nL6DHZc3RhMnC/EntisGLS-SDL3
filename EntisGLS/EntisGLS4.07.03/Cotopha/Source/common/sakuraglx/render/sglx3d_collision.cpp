
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_collision.h>
#include <sakuraglx/render/sglx3d_scene.h>

#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
#include <xmmintrin.h>
#endif

#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
#include <sakuragl/sgl_erisa_lib.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;



//////////////////////////////////////////////////////////////////////////////
// 当たり判定結果構造体
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCollisionResult::S3DCollisionResult( void )
	: matLocal( 1, 1, 1 ), vLocalBase( 0, 0, 0 )
{
	pMaterial = nullptr ;
	attrTexture.nTextureFlags = 0 ;
	fpDistance = 1.0e+30f ;
	iInstance = (size_t) -1 ;
	iMesh = (size_t) -1 ;
	iPolygon = (size_t) -1 ;
	pMesh = nullptr ;
	pPrimitiveMesh = nullptr ;
	pqpcHit = nullptr ;
	maskExcClasses = 0 ;
	maskIncClasses = (uint64_t) -1 ;
	pExcUserData = (ESLObject*) this ;	// 有効ではないポインタ
	ppExceptions = nullptr ;
	nException = 0 ;
	pfnOnHitCollider = nullptr ;
	ptrOnHitInstance = nullptr ;
	pccChain = nullptr ;
}

S3DCollisionResult::S3DCollisionResult( const S3DCollisionResult& rs )
	: vHitLocal( rs.vHitLocal ),
		vHitGlobal( rs.vHitGlobal ),
		vNormalLocal( rs.vNormalLocal ),
		vNormal( rs.vNormal ),
		vReflection( rs.vReflection ),
		vLocalCoord( rs.vLocalCoord ),
		vTexCoord( rs.vTexCoord ),
		vtxColor( rs.vtxColor ),
		pMaterial( rs.pMaterial ),
		attrTexture( rs.attrTexture ),
		matLocal( rs.matLocal ),
		vLocalBase( rs.vLocalBase )
{
	fpDistance = rs.fpDistance ;
	iInstance = rs.iInstance ;
	iMesh = rs.iMesh ;
	iPolygon = rs.iPolygon ;
	pMesh = rs.pMesh ;
	pPrimitiveMesh = rs.pPrimitiveMesh ;
	pqpcHit = rs.pqpcHit ;
	maskExcClasses = rs.maskExcClasses ;
	maskIncClasses = rs.maskIncClasses ;
	pExcUserData = rs.pExcUserData ;
	ppExceptions = rs.ppExceptions ;
	nException = rs.nException ;
	pfnOnHitCollider = rs.pfnOnHitCollider ;
	ptrOnHitInstance = rs.ptrOnHitInstance ;
	pccChain = nullptr ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DCollisionResult&
	S3DCollisionResult::operator = ( const S3DCollisionResult& rs )
{
	vHitLocal = rs.vHitLocal ;
	vHitGlobal = rs.vHitGlobal ;
	vNormalLocal = rs.vNormalLocal ;
	vNormal = rs.vNormal ;
	vReflection = rs.vReflection ;
	vLocalCoord = rs.vLocalCoord ;
	vTexCoord = rs.vTexCoord ;
	vtxColor = rs.vtxColor ;
	pMaterial = rs.pMaterial ;
	attrTexture = rs.attrTexture ;
	matLocal = rs.matLocal ;
	vLocalBase = rs.vLocalBase ;
	fpDistance = rs.fpDistance ;
	iInstance = rs.iInstance ;
	iMesh = rs.iMesh ;
	iPolygon = rs.iPolygon ;
	pMesh = rs.pMesh ;
	pPrimitiveMesh = rs.pPrimitiveMesh ;
	pqpcHit = rs.pqpcHit ;
	maskExcClasses = rs.maskExcClasses ;
	maskIncClasses = rs.maskIncClasses ;
	pExcUserData = rs.pExcUserData ;
	ppExceptions = rs.ppExceptions ;
	nException = rs.nException ;
	pfnOnHitCollider = rs.pfnOnHitCollider ;
	ptrOnHitInstance = rs.ptrOnHitInstance ;
	pccChain = nullptr ;
	return	*this ;
}

// 除外クラス設定
//////////////////////////////////////////////////////////////////////////////
void S3DCollisionResult::ModifyExceptionSceneFlags( uint32_t nAddClasses, uint32_t nRemoveFlags )
{
	uint32_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	nAddClasses &= maskSys ;
	nRemoveFlags &= maskSys ;
	maskExcClasses = (maskExcClasses & ~(uint64_t)nRemoveFlags) | nAddClasses ;
}

void S3DCollisionResult::ModifyExceptionUserFlags( uint32_t nAddClasses, uint32_t nRemoveFlags )
{
	uint64_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	nAddClasses &= maskSys ;
	nRemoveFlags &= maskSys ;
	maskExcClasses = (maskExcClasses
				& ~((uint64_t) nRemoveFlags << S3DScene::classCount))
				| ((uint64_t) nAddClasses << S3DScene::classCount) ;
}

uint32_t S3DCollisionResult::GetExceptionSceneFlags( void ) const
{
	uint32_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	return	(uint32_t) (maskExcClasses & maskSys) ;
}

uint32_t S3DCollisionResult::GetExceptionUserFlags( void ) const
{
	return	(uint32_t) (maskExcClasses >> S3DScene::classCount) ;
}

// 対象クラス設定
//////////////////////////////////////////////////////////////////////////////
void S3DCollisionResult::ModifyInclusionSceneFlags( uint32_t nAddClasses, uint32_t nRemoveFlags )
{
	uint32_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	nAddClasses &= maskSys ;
	nRemoveFlags &= maskSys ;
	maskIncClasses = (maskIncClasses & ~(uint64_t)nRemoveFlags) | nAddClasses ;
}

void S3DCollisionResult::ModifyInclusionUserFlags( uint32_t nAddClasses, uint32_t nRemoveFlags )
{
	maskIncClasses = (maskIncClasses
				& ~((uint64_t) nRemoveFlags << S3DScene::classCount))
				| ((uint64_t) nAddClasses << S3DScene::classCount) ;
}

uint32_t S3DCollisionResult::GetInclusionSceneFlags( void ) const
{
	uint32_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	return	(uint32_t) (maskIncClasses & maskSys) ;
}

uint32_t S3DCollisionResult::GetInclusionUserFlags( void ) const
{
	return	(uint32_t) (maskIncClasses >> S3DScene::classCount) ;
}

// 除外判定
bool S3DCollisionResult::IsException( S3DCollision::MeshCollision * pMeshCol ) const
{
	if ( !(pMeshCol->maskClasses & maskIncClasses)
		|| (pMeshCol->maskClasses & maskExcClasses)
		|| (pMeshCol->pUserData == pExcUserData) )
	{
		return	true ;
	}
	const S3DCollision::MeshCollision *const*	ppExcpts = ppExceptions ;
	size_t	nCount = nException ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( ppExcpts[i] == pMeshCol )
		{
			return	true ;
		}
	}
	return	false ;
}

// チェイン追加・削除（STACK順序）
//////////////////////////////////////////////////////////////////////////////
void S3DCollisionResult::PushColliderChain( ColliderChain * pChain )
{
	ESLAssert( pChain != nullptr ) ;
	ESLAssert( pChain->pccParent == nullptr ) ;
	pChain->pccParent = pccChain ;
	pccChain = pChain ;
}

void S3DCollisionResult::PopColliderChain( ColliderChain * pChain )
{
	ESLAssert( pChain == pccChain ) ;
	pccChain = const_cast<ColliderChain*>( pChain->pccParent ) ;
}

// 当たり判定コールバック関数内でコライダーマスクの判定を行う
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DCollisionResult::TestColliderMask
	( const S3DCollision::MeshCollision * pMesh, uint64_t mask ) const
{
	if ( (pMesh != nullptr) && (pMesh->maskClasses & mask) )
	{
		return	(pMesh->maskClasses & mask) ;
	}
	const S3DCollisionResult::ColliderChain *	pccNext = pccChain ;
	while ( pccNext != nullptr )
	{
		pMesh = pccNext->pMeshCol ;
		if ( (pMesh != nullptr) && (pMesh->maskClasses & mask) )
		{
			return	(pMesh->maskClasses & mask) ;
		}
		pccNext = pccNext->pccParent ;
	}
	return	0 ;
}

// 当たり判定コールバック関数内でユーザーデータを取得する
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DCollisionResult::GetUserDataOnHitCollider
	( const S3DCollision::MeshCollision * pMesh, const ESLRuntimeClass& rtClass ) const
{
	if ( (pMesh != nullptr)
		&& (pMesh->pUserData != nullptr)
		&& pMesh->pUserData->IsKindOf( rtClass ) )
	{
		return	pMesh->pUserData ;
	}
	const S3DCollisionResult::ColliderChain *	pccNext = pccChain ;
	while ( pccNext != nullptr )
	{
		pMesh = pccNext->pMeshCol ;
		if ( (pMesh != nullptr)
			&& (pMesh->pUserData != nullptr)
			&& pMesh->pUserData->IsKindOf( rtClass ) )
		{
			return	pMesh->pUserData ;
		}
		pccNext = pccNext->pccParent ;
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// 3D 当たり判定
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCollider, SObject )
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DCollision, S3DRenderBuffer, S3DCollider )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCollision::S3DCollision( void )
{
	m_pRootNode = nullptr ;
	m_countBatchBuild = 0 ;
	m_pUserData = nullptr ;
	m_maskClasses = (uint64_t) colliderShape << S3DScene::classCount ;
	m_fpAddThickness = 0.0f ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCollision::~S3DCollision( void )
{
}

// 範囲取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const
{
	if ( m_pRootNode != nullptr )
	{
		vCenter = m_pRootNode->vCenter ;
		fpRadius = m_pRootNode->fpRadius ;
		return	true ;
	}
	return	false ;
}

// 凸形状の内側判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSphereInclusive
	( const S3DDVector& vPos,
		float fpRadius, S3DCollisionResult& rsIncluded ) const
{
	if ( m_pRootNode != nullptr )
	{
		if ( IsSphereIncludedNode
				( m_pRootNode, vPos, fpRadius, rsIncluded ) )
		{
			return	true ;
		}
	}
	rsIncluded.pMesh = nullptr ;
	rsIncluded.pqpcHit = nullptr ;
	return	false ;
}

// 球との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsHitAgainstSphere
	( const S3DDVector& vPos,
		float fpRadius, S3DCollisionResult& rsHit ) const
{
	if ( m_pRootNode != nullptr )
	{
		if ( DoesNodeHitAgainstSphere
				( m_pRootNode, vPos, fpRadius, rsHit ) )
		{
			if ( rsHit.pMesh != nullptr )
			{
				rsHit.iInstance = rsHit.pMesh->iInstance ;
				rsHit.iMesh = rsHit.pMesh->iMesh ;
			}
			if ( rsHit.pPrimitiveMesh != nullptr )
			{
				rsHit.iMesh = rsHit.pPrimitiveMesh->iMesh ;
			}
			return	true ;
		}
	}
	rsHit.pMesh = nullptr ;
	rsHit.pqpcHit = nullptr ;
	return	false ;
}

// 線分との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSegmentCrossing
	( const S3DDVector& vPos0, const S3DDVector& vPos1,
		float fpErrorGap, S3DCollisionResult& rsCross ) const
{
	bool	fCross = false ;
	if ( m_pRootNode != nullptr )
	{
		S3DDVector		vGlobalSegRay = vPos1 - vPos0 ;
		const double	fpGlobalRayScale = vGlobalSegRay.Absolute() ;
		if ( fpGlobalRayScale > 1.0e-8 )
		{
			const double	fpGlobalRcpRayScale = 1.0 / fpGlobalRayScale ;
			vGlobalSegRay *= (1.0 / fpGlobalRayScale) ;
			ESLAssert( !vGlobalSegRay.IsNaN() ) ;
			//
			fCross = IsSegmentCrossingNode
				( m_pRootNode, vPos0, vPos1,
					vGlobalSegRay, fpGlobalRayScale,
							fpGlobalRcpRayScale, fpErrorGap, rsCross ) ;
		}
	}
	if ( fCross )
	{
		S3DDVector	vDelta = vPos1 - vPos0 ;
		vDelta.Normalize() ;
		//
		S3DDVector	vHitGlobal = vPos0 ;
		vHitGlobal += vDelta * rsCross.fpDistance ;
		ESLAssert( !vHitGlobal.IsNaN() ) ;
		rsCross.vHitGlobal = vHitGlobal ;
		//
		S3DVector	vRay = vDelta ;
		rsCross.vNormal.Normalize() ;
		rsCross.vReflection = vRay ;
		rsCross.vReflection -=
			rsCross.vNormal
				* (vRay.InnerProduct( rsCross.vNormal ) * 2.0) ;
		//
		if ( rsCross.pMesh != nullptr )
		{
			rsCross.iInstance = rsCross.pMesh->iInstance ;
			rsCross.iMesh = rsCross.pMesh->iMesh ;
		}
		if ( rsCross.pPrimitiveMesh != nullptr )
		{
			rsCross.iMesh = rsCross.pPrimitiveMesh->iMesh ;
		}
	}
	return	fCross ;
}

// 複数の当たり判定取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::AddHitEntriesAgainstSphere
	( SSystem::SArray<S3DCollision::HitEntryInfo>& aHitEntries,
		const S3DDVector& vPos, float fpRadius,
		S3DCollision::MultiHitEntriesFlag flagsMultiHit,
		uint64_t maskCollider, uint64_t maskException ) const
{
	AddHitEntriesAgainstSphereParam	param ;
	param.pHitEntries = &aHitEntries ;
	param.pCollision = this ;
	param.flagMultiHit = flagsMultiHit ;
	//
	S3DCollisionResult	rsHit ;
	rsHit.maskIncClasses = maskCollider ;
	rsHit.maskExcClasses = maskException ;
	rsHit.pfnOnHitCollider = &S3DCollision::Callback_OnAddHitEntriesAgainstSphere ;
	rsHit.ptrOnHitInstance = &param ;
	//
	size_t	nOldCount = aHitEntries.GetLength() ;
	IsHitAgainstSphere( vPos, fpRadius, rsHit ) ;
	return	aHitEntries.GetLength() > nOldCount ;
}

S3DCollision::HitColliderCallback
	S3DCollision::Callback_OnAddHitEntriesAgainstSphere
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	AddHitEntriesAgainstSphereParam *
			param = (AddHitEntriesAgainstSphereParam*) rsHit.ptrOnHitInstance ;
	//
	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;
	//
	if ( param->flagMultiHit != multiHitAny )
	{
		const HitEntryInfo *	pheiEntries = param->pHitEntries->GetConstArray() ;
		const size_t			nEntries = param->pHitEntries->GetLength() ;
		for ( size_t i = 0; i < nEntries; i ++ )
		{
			if ( pheiEntries[i].pmcHitGlobal == hcgi.pGlobalMesh )
			{
				return	S3DCollision::hitColliderNextMesh ;
			}
			if ( (param->flagMultiHit == multiHitUniqueUserData)
				&& (hcgi.pGlobalMesh->pUserData != nullptr)
				&& (pheiEntries[i].pmcHitGlobal->pUserData == hcgi.pGlobalMesh->pUserData) )
			{
				return	S3DCollision::hitColliderNextMesh ;
			}
		}
	}
	HitEntryInfo	hei ;
	hei.pmcHitPrimitive = pMesh ;
	hei.pmcHitGlobal = hcgi.pGlobalMesh ;
	hei.vHitGlobalPos = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	hei.vHitGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	param->pHitEntries->Add( hei ) ;
	return	S3DCollision::hitColliderNextMesh ;
}

// 特定メッシュに対して凸形状の内側判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSphereInclusiveForMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DDVector& vPos, float fpRadius, S3DCollision::Result& rsIncluded )
{
	//
	// ローカル座標へ変換
	//
	S3DVector	vLocalPos ;
	vLocalPos.x = (float32_t) (vPos.x - pMeshCol->itrans.vPos.x) ;
	vLocalPos.y = (float32_t) (vPos.y - pMeshCol->itrans.vPos.y) ;
	vLocalPos.z = (float32_t) (vPos.z - pMeshCol->itrans.vPos.z) ;
	pMeshCol->itrans.matIRev.RevolveVector( vLocalPos ) ;
	fpRadius *= pMeshCol->itrans.fpScale ;
	//
	// 範囲の交差判定
	//
	float32_t	dx = vLocalPos.x - pMeshCol->vCenter.x ;
	float32_t	dy = vLocalPos.y - pMeshCol->vCenter.y ;
	float32_t	dz = vLocalPos.z - pMeshCol->vCenter.z ;
	float32_t	r = pMeshCol->fpRadius + (float32_t) fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		if ( IsSphereIncludedMesh
			( pMeshCol, vLocalPos, fpRadius, rsIncluded ) )
		{
			// メッシュに含まれている
			rsIncluded.iInstance = pMeshCol->iInstance ;
			rsIncluded.iMesh = pMeshCol->iMesh ;
			rsIncluded.pMesh = pMeshCol ;
			rsIncluded.pqpcHit = nullptr ;
			return	true ;
		}
	}
	return	false ;
}

// 特定メッシュに対して球との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsHitAgainstSphereForMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DDVector& vPos, float fpRadius, S3DCollision::Result& rsHit )
{
	//
	// ローカル座標へ変換
	//
	S3DVector	vLocalPos ;
	vLocalPos.x = (float32_t) (vPos.x - pMeshCol->itrans.vPos.x) ;
	vLocalPos.y = (float32_t) (vPos.y - pMeshCol->itrans.vPos.y) ;
	vLocalPos.z = (float32_t) (vPos.z - pMeshCol->itrans.vPos.z) ;
	pMeshCol->itrans.matIRev.RevolveVector( vLocalPos ) ;
	fpRadius *= pMeshCol->itrans.fpScale ;
	//
	// 範囲の交差判定
	//
	float32_t	dx = vLocalPos.x - pMeshCol->vCenter.x ;
	float32_t	dy = vLocalPos.y - pMeshCol->vCenter.y ;
	float32_t	dz = vLocalPos.z - pMeshCol->vCenter.z ;
	float32_t	r = pMeshCol->fpRadius + fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		if ( DoesMeshHitAgainstSphere
			( pMeshCol, vLocalPos, fpRadius, rsHit ) )
		{
			// メッシュに交差
			ComputeGlobalOfHitAgainstSphere( rsHit, *pMeshCol ) ;
			rsHit.iMesh = pMeshCol->iMesh ;
			return	true ;
		}
	}
	return	false ;
}

// 特定メッシュに対して線分との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSegmentCrossingForMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DDVector& vPos0, const S3DDVector& vPos1,
					float fpErrorGap, S3DCollision::Result& rsCross )
{
	S3DDVector		vGlobalSegRay = vPos1 - vPos0 ;
	const double	fpGlobalRayScale = vGlobalSegRay.Absolute() ;
	const double	fpGlobalRcpRayScale = 1.0 / fpGlobalRayScale ;
	vGlobalSegRay *= (1.0 / fpGlobalRayScale) ;
	//
	return	IsSegmentCrossingForMesh
		( pMeshCol, vPos0, vPos1, vGlobalSegRay,
			fpGlobalRayScale, fpGlobalRcpRayScale, fpErrorGap, rsCross ) ;
}

bool S3DCollision::IsSegmentCrossingForMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DDVector& vPos0, const S3DDVector& vPos1,
		const S3DDVector& vGlobalSegRay,
		double fpGlobalRayScale, double fpGlobalRcpRayScale,
					float fpErrorGap, S3DCollision::Result& rsCross )
{
	S3DVector	vLocalPos0, vLocalPos1 ;
	vLocalPos0.x = (float32_t) (vPos0.x - pMeshCol->itrans.vPos.x) ;
	vLocalPos0.y = (float32_t) (vPos0.y - pMeshCol->itrans.vPos.y) ;
	vLocalPos0.z = (float32_t) (vPos0.z - pMeshCol->itrans.vPos.z) ;
	vLocalPos1.x = (float32_t) (vPos1.x - pMeshCol->itrans.vPos.x) ;
	vLocalPos1.y = (float32_t) (vPos1.y - pMeshCol->itrans.vPos.y) ;
	vLocalPos1.z = (float32_t) (vPos1.z - pMeshCol->itrans.vPos.z) ;
	pMeshCol->itrans.matIRev.RevolveVector( vLocalPos0 ) ;
	pMeshCol->itrans.matIRev.RevolveVector( vLocalPos1 ) ;
	fpErrorGap *= pMeshCol->itrans.fpScale ;
	//
	// 交差判定
	//
	S3DVector		vSegRay = vLocalPos1 - vLocalPos0 ;
	const float32_t	fpRayScale = (float32_t) vSegRay.Absolute() ;
	const float32_t	fpRcpRayScale = 1.0f / fpRayScale ;
	float32_t		fpLastDistance = rsCross.fpDistance ;
	vSegRay *= (1.0f / fpRayScale) ;
	rsCross.fpDistance *= fpRayScale * (float32_t) fpGlobalRcpRayScale ;
	//
	S3DVector	vDelta = pMeshCol->vCenter - vLocalPos0 ;
	float32_t	t = vSegRay.InnerProduct( vDelta ) ;
	float32_t	r = pMeshCol->fpRadius + fpErrorGap ;
	//
	if ( (t < - r) | (t > rsCross.fpDistance + r) )
	{
		// 範囲外
		rsCross.fpDistance = fpLastDistance ;
		return	false ;
	}
	if ( vDelta.InnerProduct( vDelta ) - t * t > r * r )
	{
		// 範囲外
		rsCross.fpDistance = fpLastDistance ;
		return	false ;
	}
	if ( IsSegmentCrossingMesh
		( pMeshCol, vLocalPos0, vLocalPos1, fpErrorGap, rsCross ) )
	{
		S3DVector	vNormal = rsCross.vNormalLocal ;
		if ( pMeshCol->itrans.flagOrthMat )
		{
			pMeshCol->itrans.matRev.RevolveVector( vNormal ) ;
			vNormal.Normalize() ;
		}
		else
		{
			ComputeLocalNormalToGlobal( vNormal, pMeshCol->itrans.matRev ) ;
		}
		rsCross.vNormal = vNormal ;
		rsCross.fpDistance *= (float32_t) fpGlobalRayScale * fpRcpRayScale ;
		//
		if ( pMeshCol->typeCol == typeCollider )
		{
			S3DMatrix	matTrans = pMeshCol->itrans.matRev ;
			matTrans.RevolveVector( rsCross.vLocalBase ) ;
			rsCross.matLocal = matTrans * rsCross.matLocal ;
			rsCross.vLocalBase += S3DVector( pMeshCol->itrans.vPos ) ;
		}
		else
		{
			rsCross.matLocal = pMeshCol->itrans.matRev ;
			rsCross.vLocalBase = pMeshCol->itrans.vPos ;
		}
		return	true ;
	}
	else
	{
		rsCross.fpDistance = fpLastDistance ;
	}
	return	false ;
}

// 凸形状の内側判定（ツリー探索）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSphereIncludedNode
	( const S3DCollision::MeshTreeNode * pNode,
		const S3DDVector& vPos, float fpRadius,
			S3DCollision::Result& rsIncluded )
{
	ESLAssert( pNode != nullptr ) ;
	MeshCollision *	pMeshCol = pNode->pMeshCol ;
	if ( (pMeshCol != nullptr)
		&& !rsIncluded.IsException( pMeshCol ) )
	{
		if ( IsSphereInclusiveForMesh( pMeshCol, vPos, fpRadius, rsIncluded ) )
		{
			return	true ;
		}
	}
	MeshTreeNode *	pChild0 = pNode->pChild[0] ;
	if ( pChild0 == nullptr )
	{
		ESLAssert( pNode->pChild[1] == nullptr ) ;
		return	false ;
	}
	double	dx = pChild0->vCenter.x - vPos.x ;
	double	dy = pChild0->vCenter.y - vPos.y ;
	double	dz = pChild0->vCenter.z - vPos.z ;
	double	r = pChild0->fpRadius + fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		if ( IsSphereIncludedNode( pChild0, vPos, fpRadius, rsIncluded ) )
		{
			return	true ;
		}
	}
	MeshTreeNode *	pChild1 = pNode->pChild[1] ;
	if ( pChild1 == nullptr )
	{
		return	false ;
	}
	dx = pChild1->vCenter.x - vPos.x ;
	dy = pChild1->vCenter.y - vPos.y ;
	dz = pChild1->vCenter.z - vPos.z ;
	r = pChild1->fpRadius + fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		return	IsSphereIncludedNode( pChild1, vPos, fpRadius, rsIncluded ) ;
	}
	return	false ;
}

// 球との交差判定（ツリー探索）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::DoesNodeHitAgainstSphere
	( const S3DCollision::MeshTreeNode * pNode,
		const S3DDVector& vPos, float fpRadius, S3DCollision::Result& rsHit )
{
	ESLAssert( pNode != nullptr ) ;
	MeshCollision *	pMeshCol = pNode->pMeshCol ;
	if ( (pMeshCol != nullptr)
		&& !rsHit.IsException( pMeshCol ) )
	{
		if ( IsHitAgainstSphereForMesh( pMeshCol, vPos, fpRadius, rsHit ) )
		{
			return	true ;
		}
	}
	MeshTreeNode *	pChild0 = pNode->pChild[0] ;
	if ( pChild0 == nullptr )
	{
		ESLAssert( pNode->pChild[1] == nullptr ) ;
		return	false ;
	}
	double	dx = pChild0->vCenter.x - vPos.x ;
	double	dy = pChild0->vCenter.y - vPos.y ;
	double	dz = pChild0->vCenter.z - vPos.z ;
	double	r = pChild0->fpRadius + fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		if ( DoesNodeHitAgainstSphere( pChild0, vPos, fpRadius, rsHit ) )
		{
			return	true ;
		}
	}
	MeshTreeNode *	pChild1 = pNode->pChild[1] ;
	if ( pChild1 == nullptr )
	{
		return	false ;
	}
	dx = pChild1->vCenter.x - vPos.x ;
	dy = pChild1->vCenter.y - vPos.y ;
	dz = pChild1->vCenter.z - vPos.z ;
	r = pChild1->fpRadius + fpRadius ;
	if ( (dx * dx + dy * dy + dz * dz) <= r * r )
	{
		return	DoesNodeHitAgainstSphere( pChild1, vPos, fpRadius, rsHit ) ;
	}
	return	false ;
}

// 線分との交差判定（ツリー探索）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSegmentCrossingNode
	( const S3DCollision::MeshTreeNode * pNode,
		const S3DDVector& vPos0, const S3DDVector& vPos1,
		const S3DDVector& vGlobalSegRay,
		double fpGlobalRayScale, double fpGlobalRcpRayScale,
		float fpErrorGap, S3DCollision::Result& rsCross )
{
	ESLAssert( pNode != nullptr ) ;
	MeshCollision *	pMeshCol = pNode->pMeshCol ;
	bool			fCross = false ;
	if ( (pMeshCol != nullptr)
		&& !rsCross.IsException( pMeshCol ) )
	{
		fCross = IsSegmentCrossingForMesh
					( pMeshCol, vPos0, vPos1,
						vGlobalSegRay, fpGlobalRayScale,
						fpGlobalRcpRayScale, fpErrorGap, rsCross ) ;
	}
	//
	MeshTreeNode *	pChild0 = pNode->pChild[0] ;
	if ( pChild0 != nullptr )
	do
	{
		//
		// 交差判定
		//
		S3DDVector	vDelta = pChild0->vCenter - vPos0 ;
		double		t = vGlobalSegRay.InnerProduct( vDelta ) ;
		double		r = pChild0->fpRadius + fpErrorGap ;
		//
		if ( (t < - r) | (t > rsCross.fpDistance + r) )
		{
			// 範囲外
			break ;
		}
		if ( vDelta.InnerProduct( vDelta ) - t * t > r * r )
		{
			// 範囲外
			break ;
		}
		fCross |= IsSegmentCrossingNode
					( pChild0, vPos0, vPos1,
						vGlobalSegRay, fpGlobalRayScale,
						fpGlobalRcpRayScale, fpErrorGap, rsCross ) ;
	}
	while ( false ) ;
	//
	MeshTreeNode *	pChild1 = pNode->pChild[1] ;
	if ( pChild1 != nullptr )
	do
	{
		//
		// 交差判定
		//
		S3DDVector	vDelta = pChild1->vCenter - vPos0 ;
		double		t = vGlobalSegRay.InnerProduct( vDelta ) ;
		double		r = pChild1->fpRadius + fpErrorGap ;
		//
		if ( (t < - r) | (t > rsCross.fpDistance + r) )
		{
			// 範囲外
			break ;
		}
		if ( vDelta.InnerProduct( vDelta ) - t * t > r * r )
		{
			// 範囲外
			break ;
		}
		fCross |= IsSegmentCrossingNode
					( pChild1, vPos0, vPos1,
						vGlobalSegRay, fpGlobalRayScale,
						fpGlobalRcpRayScale, fpErrorGap, rsCross ) ;
	}
	while ( false ) ;
	//
	return	fCross ;
}

// △ABC の辺 AB, AC を基底とした交差座標を計算
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::ComputeHitLocalCoord( S3DCollision::Result& rsCross )
{
	const QuadPolygonsCollision *	pqpc = rsCross.pqpcHit ;
	if ( pqpc == nullptr )
	{
		return	sglErrFailed ;
	}
	static const int	iIShift[3][3] =
	{
		{ 0, 1, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
	} ;
	const size_t	i = rsCross.iPolygon & 0x03 ;
	const size_t	j = pqpc->iShiftA[i & 0x03] ;
	ESLAssert( j < 3 ) ;
	const int	iA = iIShift[j][0] ;
	const int	iB = iIShift[j][1] ;
	const int	iC = iIShift[j][2] ;
	//
	S3DVector	v[3] ;
	v[iA].x = pqpc->xA[i] ;
	v[iA].y = pqpc->yA[i] ;
	v[iA].z = pqpc->zA[i] ;
	v[iB].x = pqpc->xB[i] ;
	v[iB].y = pqpc->yB[i] ;
	v[iB].z = pqpc->zB[i] ;
	v[iC].x = pqpc->xC[i] ;
	v[iC].y = pqpc->yC[i] ;
	v[iC].z = pqpc->zC[i] ;
	//
	S3DVector	vBA, vCA, vPA, vXBC ;
	vBA = v[1] - v[0] ;
	vCA = v[2] - v[0] ;
	vPA = rsCross.vHitLocal - v[0] ;
	vXBC = vBA * vCA ;
	//
	float32_t	d = vXBC.InnerProduct( vXBC ) ;
	if ( d >= 1.0e-8 )
	{
		vXBC *= 1.0f / d ;
	}
	//
	rsCross.vLocalCoord.x = (float32_t) vXBC.InnerProduct( vPA * vCA ) ;
	rsCross.vLocalCoord.y = (float32_t) vXBC.InnerProduct( vBA * vPA ) ;
	return	sglErrSuccess ;
}

// 法線補完計算
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::ComplementeLocalNormal( S3DCollision::Result& rsCross )
{
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DVector4 *	pvNormal = preMesh->pvNormal ;
	if ( pvNormal == nullptr )
	{
		return	sglErrFailed ;
	}
	ESLAssert( preMesh->pIndexedList != nullptr ) ;
	const size_t	i = rsCross.iPolygon ;
	uint32_t *		pIndexedList = preMesh->pIndexedList + (i * 3) ;
	//
	S3DVector	vNormal0 = pvNormal[pIndexedList[0]] ;
	S3DVector	vNormal1 = pvNormal[pIndexedList[1]] ;
	S3DVector	vNormal2 = pvNormal[pIndexedList[2]] ;
	vNormal1 -= vNormal0 ;
	vNormal2 -= vNormal0 ;
	vNormal1 *= rsCross.vLocalCoord.x ;
	vNormal2 *= rsCross.vLocalCoord.y ;
	vNormal0 += vNormal1 ;
	vNormal0 += vNormal2 ;
	vNormal0.Normalize() ;
	rsCross.vNormalLocal = vNormal0 ;
	return	sglErrSuccess ;
}

// UV座標計算
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::ComplementeTextureCoord( S3DCollision::Result& rsCross )
{
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	S2DVector *	pvUVMap = preMesh->pvUVMap ;
	if ( pvUVMap == nullptr )
	{
		return	sglErrFailed ;
	}
	ESLAssert( preMesh->pIndexedList != nullptr ) ;
	const size_t	i = rsCross.iPolygon ;
	uint32_t *		pIndexedList = preMesh->pIndexedList + (i * 3) ;
	//
	S2DVector	vUV0 = pvUVMap[pIndexedList[0]] ;
	S2DVector	vUV1 = pvUVMap[pIndexedList[1]] ;
	S2DVector	vUV2 = pvUVMap[pIndexedList[2]] ;
	vUV1 -= vUV0 ;
	vUV2 -= vUV0 ;
	vUV1 *= rsCross.vLocalCoord.x ;
	vUV2 *= rsCross.vLocalCoord.y ;
	vUV0 += vUV1 ;
	vUV0 += vUV2 ;
	rsCross.vTexCoord = vUV0 ;
	return	sglErrSuccess ;
}

// 頂点色計算
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::ComplementeVertexColor( Result& rsCross )
{
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DColor *	pColor = preMesh->pColor ;
	if ( pColor == nullptr )
	{
		return	sglErrFailed ;
	}
	ESLAssert( preMesh->pIndexedList != nullptr ) ;
	const size_t	i = rsCross.iPolygon ;
	uint32_t *		pIndexedList = preMesh->pIndexedList + (i * 3) ;
	//
	S3DColor	clr0 = pColor[pIndexedList[0]] ;
	S3DColor	clr1 = pColor[pIndexedList[1]] ;
	S3DColor	clr2 = pColor[pIndexedList[2]] ;
	double		t = esl_fmax( 1.0 - rsCross.vLocalCoord.x
									- rsCross.vLocalCoord.y, 0.0 ) ;
	rsCross.vtxColor.rgbMul =
		SGLWPaletteARGB(clr0.rgbMul) * t
			+ SGLWPaletteARGB(clr1.rgbMul) * (double) rsCross.vLocalCoord.x
			+ SGLWPaletteARGB(clr2.rgbMul) * (double) rsCross.vLocalCoord.y ;
	rsCross.vtxColor.rgbAdd =
		SGLWPaletteARGB(clr0.rgbAdd) * t
			+ SGLWPaletteARGB(clr1.rgbAdd) * (double) rsCross.vLocalCoord.x
			+ SGLWPaletteARGB(clr2.rgbAdd) * (double) rsCross.vLocalCoord.y ;
	return	sglErrSuccess ;
}

// テクスチャサンプリング
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::SampleDiffusionTexture
	( const S3DCollision::Result& rsCross, SGLPalette& rgbaTexture )
{
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DMaterial *	pMaterial = preMesh->pMaterial ;
	if ( pMaterial == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( !pMaterial->SampleDiffusionTexture
			( rgbaTexture, rsCross.vTexCoord.x, rsCross.vTexCoord.y ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError S3DCollision::SampleLuminousTexture
		( const Result& rsCross, SGLPalette& rgbaTexture )
{
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DMaterial *	pMaterial = preMesh->pMaterial ;
	if ( pMaterial == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( !pMaterial->SampleLuminousTexture
			( rgbaTexture, rsCross.vTexCoord.x, rsCross.vTexCoord.y ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 法線・UV・頂点色計算／各種テクスチャサンプリング
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::ComplementeAndSampleAttributes( S3DCollision::Result& rsCross )
{
	//
	// メッシュ情報参照
	//
	const MeshCollision *	pMesh = rsCross.GetPrimitiveMesh() ;
	if ( pMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	RENDER_ENTRY *	preMesh = pMesh->preMesh ;
	if ( preMesh == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// ポリゴン参照
	//
	ESLAssert( preMesh->pIndexedList != nullptr ) ;
	const size_t	i = rsCross.iPolygon ;
	uint32_t *		pIndexedList = preMesh->pIndexedList + (i * 3) ;
	uint32_t		iIndex0 = pIndexedList[0] ;
	uint32_t		iIndex1 = pIndexedList[1] ;
	uint32_t		iIndex2 = pIndexedList[2] ;
	//
	// 法線計算
	//
	S3DVector4 *	pvNormal = preMesh->pvNormal ;
	if ( pvNormal != nullptr )
	{
		S3DVector	vNormal0 = pvNormal[iIndex0] ;
		S3DVector	vNormal1 = pvNormal[iIndex1] ;
		S3DVector	vNormal2 = pvNormal[iIndex2] ;
		vNormal1 -= vNormal0 ;
		vNormal2 -= vNormal0 ;
		vNormal1 *= rsCross.vLocalCoord.x ;
		vNormal2 *= rsCross.vLocalCoord.y ;
		vNormal0 += vNormal1 ;
		vNormal0 += vNormal2 ;
		vNormal0.Normalize() ;
		rsCross.vNormalLocal = vNormal0 ;
	}
	//
	// UV 計算
	//
	S2DVector *	pvUVMap = preMesh->pvUVMap ;
	rsCross.vTexCoord.x = 0 ;
	rsCross.vTexCoord.y = 0 ;
	if ( pvUVMap != nullptr )
	{
		S2DVector	vUV0 = pvUVMap[iIndex0] ;
		S2DVector	vUV1 = pvUVMap[iIndex1] ;
		S2DVector	vUV2 = pvUVMap[iIndex2] ;
		vUV1 -= vUV0 ;
		vUV2 -= vUV0 ;
		vUV1 *= rsCross.vLocalCoord.x ;
		vUV2 *= rsCross.vLocalCoord.y ;
		vUV0 += vUV1 ;
		vUV0 += vUV2 ;
		rsCross.vTexCoord = vUV0 ;
	}
	//
	// 頂点色計算
	//
	S3DColor *	pColor = preMesh->pColor ;
	rsCross.vtxColor.rgbMul = 0xFFFFFFFF ;
	rsCross.vtxColor.rgbAdd = 0 ;
	if ( pColor != nullptr )
	{
		S3DColor	clr0 = pColor[iIndex0] ;
		S3DColor	clr1 = pColor[iIndex1] ;
		S3DColor	clr2 = pColor[iIndex2] ;
		S4DVector	vclrMul0 = clr0.rgbMul ;
		S4DVector	vclrAdd0 = clr0.rgbAdd ;
		S4DVector	vclrMul1 = clr1.rgbMul ;
		S4DVector	vclrAdd1 = clr1.rgbAdd ;
		S4DVector	vclrMul2 = clr2.rgbMul ;
		S4DVector	vclrAdd2 = clr2.rgbAdd ;
		//
		float32_t	t = 1.0f - rsCross.vLocalCoord.x
								- rsCross.vLocalCoord.y ;
		//
		S4DVector	vclrMul =
			vclrMul0 * t + vclrMul1 * rsCross.vLocalCoord.x
							+ vclrMul2 * rsCross.vLocalCoord.y ;
		S4DVector	vclrAdd =
			vclrAdd0 * t + vclrAdd1 * rsCross.vLocalCoord.x
							+ vclrAdd2 * rsCross.vLocalCoord.y ;
		rsCross.vtxColor.rgbMul = vclrMul.ToColor() ;
		rsCross.vtxColor.rgbAdd = vclrAdd.ToColor() ;
	}
	//
	// テクスチャサンプリング
	//
	S3DMaterial *	pMaterial = preMesh->pMaterial ;
	rsCross.pMaterial = pMaterial ;
	if ( pMaterial != nullptr )
	{
		pMaterial->SampleTextures
			( rsCross.attrTexture, rsCross.vTexCoord.x, rsCross.vTexCoord.y ) ;
	}
	else
	{
		rsCross.attrTexture.Clear() ;
	}
	return	sglErrSuccess ;
}

// 当たり判定コールバック関数に渡される座標空間からグローバルへの変換情報
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::ComputeHitColliderGlobalInfo
	( S3DCollision::HitColliderGlobalInfo& hcgi,
		const S3DCollision::Result& rs,
		const S3DCollision::MeshCollision * pMeshCol )
{
	hcgi.matToGlobal = S3DMatrix( 1, 1, 1 ) ;
	hcgi.vToGlobal = S3DVector( 0, 0, 0 ) ;
	hcgi.pUserData = nullptr ;
	hcgi.iInstance = 0 ;
	hcgi.pGlobalMesh = nullptr ;
	//
	const S3DCollisionResult::ColliderChain *	pccNext = rs.pccChain ;
	while ( pMeshCol != nullptr )
	{
		S3DMatrix	matLocal( 1, 1, 1 ) ;
		S3DVector	vLocal( 0, 0, 0 ) ;
		//
		RENDER_ENTRY *	preMesh = pMeshCol->preMesh ;
		if ( preMesh != nullptr )
		{
			Transformation *	pTrans = preMesh->pTransform ;
			if ( pTrans != nullptr )
			{
				matLocal = pTrans->matTransform ;
				vLocal = pTrans->vTransform ;
			}
		}
		else
		{
			matLocal = pMeshCol->itrans.matRev ;
			vLocal = pMeshCol->itrans.vPos ;
		}
		//
		hcgi.matToGlobal = matLocal * hcgi.matToGlobal ;
		hcgi.vToGlobal = matLocal * hcgi.vToGlobal + vLocal ;
		hcgi.pUserData = pMeshCol->pUserData ;
		hcgi.iInstance = pMeshCol->iInstance ;
		hcgi.pGlobalMesh = pMeshCol ;
		//
		if ( pccNext != nullptr )
		{
			pMeshCol = pccNext->pMeshCol ;
			pccNext = pccNext->pccParent ;
		}
		else
		{
			break ;
		}
	}
}

// 当たり判定コールバックに渡されるパラメータを反映
void S3DCollision::SetResultParamOnHitCollider
	( Result& rs, const S3DCollision::HitColliderGlobalInfo& hcgi,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	rs.vHitLocal = vHitPos ;
	rs.vHitGlobal = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	rs.vNormalLocal = vHitNormal ;
	rs.vNormal = hcgi.matToGlobal * vHitNormal ;
	rs.iPolygon = (uint32_t) iPolygon ;
	rs.pMesh = pMesh ;
	rs.pPrimitiveMesh = pMesh ;
	rs.pqpcHit = &(pMesh->pCollision[iPolygon >> 2]) ;
}

// 法線のローカル空間からグローバル空間への変換（一般）
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::ComputeLocalNormalToGlobal
	( S3DVector& vNormal, const S3DMatrix& matToGlobal )
{
	S3DMatrix	matR( 1, 1, 1 ) ;
	matR.RevolveForAngle( vNormal ) ;

	S3DVector	vGlobal = (matToGlobal * matR.GetColumn( 0 ))
					* (matToGlobal * matR.GetColumn( 1 )) ;
	vGlobal.Normalize() ;

	ESLAssert( vGlobal.InnerProduct( matToGlobal * vNormal ) > 0.0f ) ;
	vNormal = vGlobal ;
}

// 球との交差判定、次候補準備
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::PrepareNextHitSphere
	( S3DCollision::NextHitContext& nhc,
		const S3DCollision::Result& rsHit,
		const S3DDVector& vPos, float fpRadius )
{
	const S3DCollision::MeshCollision *	pmcMesh = rsHit.pMesh ;
	ESLAssert( pmcMesh != nullptr ) ;
	if ( pmcMesh == nullptr )
	{
		return ;
	}
	nhc.vMesh += nhc.matMesh * S3DVector(pmcMesh->itrans.vPos) ;
	nhc.matMesh *= pmcMesh->itrans.matRev ;
	nhc.pmcMeshCol = pmcMesh ;
	nhc.iNextPoly = 0 ;
	//
	TransformToMeshLocal
		( pmcMesh, nhc.vTestPos, nhc.fpRadius, vPos, fpRadius ) ;
}

// 次の当たり判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::NextHitAgainstSphere
	( S3DCollision::NextHitContext& nhc, S3DCollision::Result& rsHit )
{
	ESLAssert( nhc.pmcMeshCol != nullptr ) ;
	const MeshCollision *	pmcMesh = nhc.pmcMeshCol ;
	if ( (pmcMesh->typeCol == typeCollider)
		&& (pmcMesh->pCollider != nullptr) )
	{
		S3DDVector	vPos = nhc.vTestPos ;
		if ( pmcMesh->pCollider->
				IsHitAgainstSphere( vPos, nhc.fpRadius, rsHit ) )
		{
			PrepareNextHitSphere( nhc, rsHit, vPos, nhc.fpRadius ) ;
			//
			rsHit.vHitGlobal = nhc.matMesh * rsHit.vHitLocal + nhc.vMesh ;
			rsHit.vNormal = nhc.matMesh * rsHit.vNormal ;
			return	true ;
		}
	}
	else
	{
		if ( DoesMeshHitAgainstSphere
			( pmcMesh, nhc.vTestPos, nhc.fpRadius, rsHit, nhc.iNextPoly ) )
		{
			nhc.iNextPoly = rsHit.iPolygon + 1 ;
			//
			rsHit.vHitGlobal = nhc.matMesh * rsHit.vHitLocal + nhc.vMesh ;
			rsHit.vNormal = nhc.matMesh * rsHit.vNormal ;
			return	true ;
		}
	}
	return	false ;
}

// ローカル座標へ変換
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::TransformToMeshLocal
	( const MeshCollision * pMeshCol,
		S3DVector& vLocalPos, float32_t& fpLocalRadius,
		const S3DDVector& vGlobalPos, float fpGlobalRadius )
{
	vLocalPos.x = (float32_t) (vGlobalPos.x - pMeshCol->itrans.vPos.x) ;
	vLocalPos.y = (float32_t) (vGlobalPos.y - pMeshCol->itrans.vPos.y) ;
	vLocalPos.z = (float32_t) (vGlobalPos.z - pMeshCol->itrans.vPos.z) ;
	pMeshCol->itrans.matIRev.RevolveVector( vLocalPos ) ;
	fpLocalRadius = fpGlobalRadius * pMeshCol->itrans.fpScale ;
}

// 凸形状の内側判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSphereIncludedMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos, float fpRadius,
				S3DCollision::Result& rsIncluded )
{
	MeshCollisionType	typeCol = pMeshCol->typeCol ;
	switch ( typeCol )
	{
	case	typeTriangleList:
		{
			QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
			S3DVector		vLocalPos = vPos ;
			const size_t	nPolygons = pMeshCol->nPolygons ;
			for ( size_t i = 0; i < nPolygons; i ++ )
			{
				QuadPolygonsCollision&	qpc = pCollision[i >> 2] ;
				const size_t	j = (i & 0x03) ;
				double	dx = vLocalPos.x - qpc.xA[j] ;
				double	dy = vLocalPos.y - qpc.yA[j] ;
				double	dz = vLocalPos.z - qpc.zA[j] ;
				double	r = dx * qpc.xNormal[j]
							+ dy * qpc.yNormal[j] + dz * qpc.zNormal[j] ;
				if ( r > fpRadius )
				{
					// 外側
					return	false ;
				}
			}
			rsIncluded.pMesh = pMeshCol ;
		}
		return	true ;

	case	typeCollider:
		if ( pMeshCol->pCollider != nullptr )
		{
			S3DCollisionResult::ColliderChain	colChain( pMeshCol ) ;
			rsIncluded.PushColliderChain( &colChain ) ;
			//
			S3DDVector	vdPos = vPos ;
			if ( pMeshCol->pCollider->IsSphereInclusive
					( vdPos, fpRadius + pMeshCol->fpThickness, rsIncluded ) )
			{
				rsIncluded.PopColliderChain( &colChain ) ;
				rsIncluded.pPrimitiveMesh = rsIncluded.pMesh ;
				rsIncluded.pMesh = pMeshCol ;
				rsIncluded.iInstance = pMeshCol->iInstance ;
				return	true ;
			}
			rsIncluded.PopColliderChain( &colChain ) ;
		}
		break ;

	case	typeSolidSphere:
		{
			double		r = fpRadius + pMeshCol->fpRadius ;
			S3DVector	vDelta = vPos ;
			vDelta -= pMeshCol->vCenter ;
			if ( vDelta.InnerProduct( vDelta ) <= r * r )
			{
				rsIncluded.pMesh = pMeshCol ;
				return	true ;
			}
		}
		break ;

	case	typeLineList:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLine ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			uint32_t *		pIndex = pre->pIndexedList ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			for ( size_t i = 0, j = 0; i < nLines; i ++, j += 2 )
			{
				if ( IsSphereIncludedLine
					( pvVertex[pIndex[j]],
						pvVertex[pIndex[j + 1]], vPos, fpThickness ) )
				{
					rsIncluded.pMesh = pMeshCol ;
					return	true ;
				}
			}
		}
		break ;

	case	typeLineStrip:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLineStrip ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			for ( size_t i = 0; i < nLines; i ++ )
			{
				if ( IsSphereIncludedLine
					( pvVertex[i], pvVertex[i + 1], vPos, fpThickness ) )
				{
					rsIncluded.pMesh = pMeshCol ;
					return	true ;
				}
			}
		}
		break ;

	case	typePoints:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitivePoint ) ;
			const size_t	nPoints = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			float32_t		fpThickness2 = fpThickness * fpThickness ;
			for ( size_t i = 0; i < nPoints; i ++ )
			{
				S3DVector	vDelta = pvVertex[i] ;
				vDelta -= vDelta ;
				if ( vDelta.InnerProduct( vDelta ) <= fpThickness2 )
				{
					rsIncluded.pMesh = pMeshCol ;
					return	true ;
				}
			}
		}
		break ;

	case	typeSolidCube:
		{
			float	minR = -1.0f - fpRadius ;
			float	maxR = 1.0f + fpRadius ;
			if ( (minR <= vPos.x) && (vPos.x <= maxR)
				&& (minR <= vPos.y) && (vPos.y <= maxR)
				&& (minR <= vPos.z) && (vPos.z <= maxR) )
			{
				rsIncluded.pMesh = pMeshCol ;
				return	true ;
			}
		}
		break ;
	}
	return	false ;
}

bool S3DCollision::IsSphereIncludedLine
	( const S3DVector& vLine0, const S3DVector& vLine1,
		const S3DVector& vPos, float fpRadius )
{
	S3DVector	vLine = vLine1 ;
	S3DVector	vDelta0 = vPos ;
	vLine -= vLine0 ;
	vDelta0 -= vLine0 ;
	//
	double	rLine = vLine.Absolute() ;
	if ( rLine <= 1.0e-8 )
	{
		return	vDelta0.InnerProduct( vDelta0 ) <= fpRadius * fpRadius ;
	}
	double	t = vLine.InnerProduct( vDelta0 ) / rLine ;
	if ( t >= 0.0 )
	{
		if ( t <= rLine )
		{
			double	r2 = vDelta0.InnerProduct( vDelta0 ) ;
			double	s2 = r2 - t * t ;
			return	s2 <= fpRadius * fpRadius ;
		}
		else if ( t < fpRadius + rLine )
		{
			S3DVector	vDelta1 = vPos ;
			vDelta1 -= vLine1 ;
			return	vDelta1.InnerProduct( vDelta1 ) <= fpRadius * fpRadius ;
		}
	}
	else if ( - fpRadius < t )
	{
		return	vDelta0.Absolute() <= fpRadius ;
	}
	return	false ;
}

// 球との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::DoesMeshHitAgainstSphere
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos, float fpRadius,
		S3DCollision::Result& rsHit, size_t iFirstPoly )
{
	MeshCollisionType	typeCol = pMeshCol->typeCol ;
	switch ( typeCol )
	{
	case	typeTriangleList:
		#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
		if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
		{
			ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
			return	DoesTriangleMeshHitAgainstSphere_SSE
					( pMeshCol, vPos, fpRadius, rsHit, iFirstPoly ) ;
		}
		else
		#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
		{
			ESLAssert( (g_cpuFamily == cpuFamily_ARM) || (g_cpuFamily == cpuFamily_ARM64) ) ;
			return	DoesTriangleMeshHitAgainstSphere_NEON
					( pMeshCol, vPos, fpRadius, rsHit, iFirstPoly ) ;
		}
		else
		#endif
		{
			return	DoesTriangleMeshHitAgainstSphere
					( pMeshCol, vPos, fpRadius, rsHit, iFirstPoly ) ;
		}
		break ;

	case	typeCollider:
		if ( pMeshCol->pCollider != nullptr )
		{
			S3DCollisionResult::ColliderChain	colChain( pMeshCol ) ;
			rsHit.PushColliderChain( &colChain ) ;
			//
			S3DDVector	vdPos = vPos ;
			if ( pMeshCol->pCollider->
						IsHitAgainstSphere( vdPos, fpRadius, rsHit ) )
			{
				rsHit.PopColliderChain( &colChain ) ;
				rsHit.pMesh = pMeshCol ;
				rsHit.iInstance = pMeshCol->iInstance ;
				return	true ;
			}
			rsHit.PopColliderChain( &colChain ) ;
		}
		break ;

	case	typeSolidSphere:
		{
			double		r = fpRadius + pMeshCol->fpRadius ;
			S3DVector	vDelta = vPos ;
			vDelta -= pMeshCol->vCenter ;
			//
			float32_t	d2 = vDelta.InnerProduct( vDelta ) ;
			if ( d2 <= r * r )
			{
				float32_t	d = (float32_t) sqrt( d2 ) ;
				if ( d > 1.0e-7 )
				{
					vDelta *= 1.0f / d ;
				}
				else
				{
					vDelta.x = 0 ;
					vDelta.y = 0 ;
					vDelta.z = 1 ;
				}
				S3DVector	vHit = pMeshCol->vCenter ;
				vHit += vDelta * pMeshCol->fpRadius ;
				//
				if ( rsHit.OnHitCollider( vHit, vDelta, pMeshCol, 0 )
										== S3DCollision::hitColliderReturn )
				{
					rsHit.vHitLocal = vHit ;
					rsHit.vHitGlobal = vHit ;
					rsHit.vNormal = vDelta ;
					rsHit.vNormalLocal = vDelta ;
					rsHit.iPolygon = 0 ;
					rsHit.pMesh = pMeshCol ;
					rsHit.pPrimitiveMesh = pMeshCol ;
					rsHit.pqpcHit = nullptr ;
					return	true ;
				}
			}
		}
		break ;

	case	typeLineList:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLine ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			uint32_t *		pIndex = pre->pIndexedList ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			for ( size_t i = iFirstPoly, j = iFirstPoly * 2; i < nLines; i ++, j += 2 )
			{
				if ( DoesLineHitAgainstSphere
					( pvVertex[pIndex[j]],
						pvVertex[pIndex[j + 1]],
						vPos, fpThickness, rsHit )
					&& (rsHit.OnHitCollider
						( rsHit.vHitLocal, rsHit.vNormal, pMeshCol, 0 )
										== S3DCollision::hitColliderReturn) )
				{
					rsHit.iPolygon = i ;
					rsHit.pMesh = pMeshCol ;
					rsHit.pPrimitiveMesh = pMeshCol ;
					return	true ;
				}
			}
		}
		break ;

	case	typeLineStrip:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLineStrip ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			for ( size_t i = iFirstPoly; i < nLines; i ++ )
			{
				if ( DoesLineHitAgainstSphere
						( pvVertex[i], pvVertex[i + 1], vPos, fpThickness, rsHit )
					&& (rsHit.OnHitCollider
						( rsHit.vHitLocal, rsHit.vNormal, pMeshCol, 0 )
										== S3DCollision::hitColliderReturn) )
				{
					rsHit.iPolygon = i ;
					rsHit.pMesh = pMeshCol ;
					rsHit.pPrimitiveMesh = pMeshCol ;
					return	true ;
				}
			}
		}
		break ;

	case	typePoints:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitivePoint ) ;
			const size_t	nPoints = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpRadius + pMeshCol->fpThickness ;
			float32_t		fpThickness2 = fpThickness * fpThickness ;
			for ( size_t i = iFirstPoly; i < nPoints; i ++ )
			{
				S3DVector	vDelta = pvVertex[i] ;
				vDelta -= vPos ;
				float32_t	d2 = vDelta.InnerProduct( vDelta ) ;
				if ( d2 <= fpThickness2 )
				{
					float32_t	d = (float32_t) sqrt( d2 ) ;
					vDelta *= 1.0f / d ;
					//
					S3DVector	vHit = pvVertex[i] ;
					vHit += vDelta * fpThickness ;
					//
					if ( rsHit.OnHitCollider( vHit, vDelta, pMeshCol, 0 )
											== S3DCollision::hitColliderReturn )
					{
						rsHit.vHitLocal = vHit ;
						rsHit.vHitGlobal = vHit ;
						rsHit.vNormal = vDelta ;
						rsHit.vNormalLocal = vDelta ;
						rsHit.iPolygon = i ;
						rsHit.pMesh = pMeshCol ;
						rsHit.pPrimitiveMesh = pMeshCol ;
						rsHit.pqpcHit = nullptr ;
						return	true ;
					}
				}
			}
		}
		break ;

	case	typeSolidCube:
		{
			float	minR = -1.0f - fpRadius ;
			float	maxR = 1.0f + fpRadius ;
			if ( (minR <= vPos.x) && (vPos.x <= maxR)
				&& (minR <= vPos.y) && (vPos.y <= maxR)
				&& (minR <= vPos.z) && (vPos.z <= maxR) )
			{
				S3DVector	vHit = vPos ;
				rsHit.vNormal.x = 0 ;
				rsHit.vNormal.y = 0 ;
				rsHit.vNormal.z = 0 ;
				rsHit.iPolygon = 0 ;
				rsHit.pMesh = pMeshCol ;
				rsHit.pPrimitiveMesh = pMeshCol ;
				rsHit.pqpcHit = nullptr ;
				//
				if ( fabs( vPos.x ) > fabs( vPos.y ) )
				{
					if ( fabs( vPos.x ) > fabs( vPos.z ) )
					{
						rsHit.vNormal.x = (vPos.x >= 0.0f) ? 1.0f : -1.0f ;
						vHit.x = rsHit.vNormal.x ;
					}
					else
					{
						rsHit.vNormal.z = (vPos.z >= 0.0f) ? 1.0f : -1.0f ;
						vHit.z = rsHit.vNormal.z ;
					}
				}
				else if ( fabs( vPos.y ) > fabs( vPos.z ) )
				{
					rsHit.vNormal.y = (vPos.y >= 0.0f) ? 1.0f : -1.0f ;
					vHit.y = rsHit.vNormal.y ;
				}
				else
				{
					rsHit.vNormal.z = (vPos.z >= 0.0f) ? 1.0f : -1.0f ;
					vHit.z = rsHit.vNormal.z ;
				}
				rsHit.vNormalLocal = rsHit.vNormal ;
				//
				if ( rsHit.OnHitCollider( vHit, rsHit.vNormal, pMeshCol, 0 )
											== S3DCollision::hitColliderReturn )
				{
					rsHit.vHitLocal = vHit ;
					rsHit.vHitGlobal = vHit ;
					return	true ;
				}
			}
		}
		break ;
	}
	return	false ;
}

bool S3DCollision::DoesLineHitAgainstSphere
	( const S3DVector& vLine0, const S3DVector& vLine1,
		const S3DVector& vPos, float fpRadius, S3DCollision::Result& rsHit )
{
	S3DVector	vLine = vLine1 ;
	S3DVector	vDelta0 = vPos ;
	vLine -= vLine0 ;
	vDelta0 -= vLine0 ;
	//
	double	rLine = vLine.Absolute() ;
	if ( rLine <= 1.0e-8 )
	{
		float32_t	d2 = vDelta0.InnerProduct( vDelta0 ) ;
		if ( d2 <= fpRadius * fpRadius )
		{
			vDelta0 *= 1.0f / (float32_t) sqrt( d2 ) ;
			//
			S3DVector	vHit = vLine0 ;
			vHit += vDelta0 * fpRadius ;
			rsHit.vHitLocal = vHit ;
			rsHit.vHitGlobal = vHit ;
			rsHit.vNormal = vDelta0 ;
			rsHit.vNormalLocal = vDelta0 ;
			rsHit.iPolygon = 0 ;
			rsHit.pqpcHit = nullptr ;
			return	true ;
		}
		return	false ;
	}
	double	t = vLine.InnerProduct( vDelta0 ) / rLine ;
	if ( t >= 0.0 )
	{
		if ( t <= rLine )
		{
			double	r2 = vDelta0.InnerProduct( vDelta0 ) ;
			double	s2 = r2 - t * t ;
			if ( s2 <= fpRadius * fpRadius )
			{
				S3DVector	vPost = vLine0 + vLine * (t / rLine) ;
				S3DVector	vDelta = vPos - vPost ;
				vDelta.Normalize() ;
				//
				S3DVector	vHit = vPost ;
				vHit += vDelta * fpRadius ;
				rsHit.vHitLocal = vHit ;
				rsHit.vHitGlobal = vHit ;
				rsHit.vNormal = vDelta ;
				rsHit.vNormalLocal = vDelta ;
				rsHit.iPolygon = 0 ;
				rsHit.pqpcHit = nullptr ;
				return	true ;
			}
			return	false ;
		}
		else if ( t < fpRadius + rLine )
		{
			S3DVector	vDelta1 = vPos ;
			vDelta1 -= vLine1 ;
			if ( vDelta1.InnerProduct( vDelta1 ) <= fpRadius * fpRadius )
			{
				S3DVector	vHit = vLine1 ;
				vDelta1.Normalize() ;
				vHit += vDelta1 * fpRadius ;
				rsHit.vHitLocal = vHit ;
				rsHit.vHitGlobal = vHit ;
				rsHit.vNormal = vDelta1 ;
				rsHit.vNormalLocal = vDelta1 ;
				rsHit.iPolygon = 0 ;
				rsHit.pqpcHit = nullptr ;
				return	true ;
			}
			return	false ;
		}
	}
	else if ( - fpRadius < t )
	{
		if ( vDelta0.Absolute() <= fpRadius )
		{
			S3DVector	vHit = vLine0 ;
			vDelta0.Normalize() ;
			vHit += vDelta0 * fpRadius ;
			rsHit.vHitLocal = vHit ;
			rsHit.vHitGlobal = vHit ;
			rsHit.vNormal = vDelta0 ;
			rsHit.vNormalLocal = vDelta0 ;
			rsHit.iPolygon = 0 ;
			rsHit.pqpcHit = nullptr ;
			return	true ;
		}
		return	false ;
	}
	return	false ;
}

bool S3DCollision::DoesTriangleMeshHitAgainstSphere
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos, float fpRadius,
		S3DCollision::Result& rsHit, size_t iFirstPoly )
{
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector		vLocalPos = vPos ;
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = (size_t) 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = (size_t) 1 << nQPolyPackShift ;
	const size_t	nQPolyPackOddMask = nQPolyPackCount - 1 ;
	const size_t	nPolygons = pMeshCol->nPolygons ;
	const size_t	nQPolygons = (nPolygons + nQPolyPackOddMask)
												>> nQPolyPackShift ;
	float			errGap = fpRadius * 0.1f ;
	//
	for ( size_t i = iFirstPoly; i < nQPolygons; i ++ )
	{
		const QuadSphereCollision&	qsc = pSphereCol[i >> 2] ;
		size_t	k = i & 0x03 ;
		float	dx = vLocalPos.x - qsc.xCenter[k] ;
		float	dy = vLocalPos.y - qsc.yCenter[k] ;
		float	dz = vLocalPos.z - qsc.zCenter[k] ;
		float	r = fpRadius + qsc.fpRadius[k] ;
		if ( dx * dx + dy * dy + dz * dz > r * r )
		{
			// 範囲外
			continue ;
		}
		for ( size_t p = 0; p < nQPolyPackCount; p ++ )
		{
			const size_t	iPolygon = (i << nQPolyPackShift) + p ;
			if ( iPolygon >= nPolygons )
			{
				break ;
			}
			const QuadPolygonsCollision&	qpc = pCollision[iPolygon >> 2] ;
			const size_t					j = iPolygon & 0x03 ;
			//
			// 交点計算
			//
			float	xNormal = qpc.xNormal[j] ;
			float	yNormal = qpc.yNormal[j] ;
			float	zNormal = qpc.zNormal[j] ;
			float	xA = qpc.xA[j] ;
			float	yA = qpc.yA[j] ;
			float	zA = qpc.zA[j] ;
			dx = vLocalPos.x - xA ;
			dy = vLocalPos.y - yA ;
			dz = vLocalPos.z - zA ;
			r = dx * xNormal + dy * yNormal + dz * zNormal ;
			if ( fabs(r) > fpRadius )
			{
				// 非接触
				continue ;
			}
			float	xHit = vLocalPos.x - xNormal * r ;
			float	yHit = vLocalPos.y - yNormal * r ;
			float	zHit = vLocalPos.z - zNormal * r ;
			//
			// 三角形範囲判定
			//
			float	xB = qpc.xB[j] ;
			float	yB = qpc.yB[j] ;
			float	zB = qpc.zB[j] ;
			float	xPB = xHit - xB ;
			float	yPB = yHit - yB ;
			float	zPB = zHit - zB ;
			double	absPB = sqrt( xPB * xPB + yPB * yPB + zPB * zPB ) ;
			float	absAB = qpc.absAB[j] ;
			float	cosB = qpc.cosB[j] ;
			double	cosB_gap = cosB * absPB - errGap * qpc.sinB[j] ;
			if ( (xA - xB) * xPB + (yA - yB) * yPB + (zA - zB) * zPB
													< cosB_gap * absAB )
			{
				// 範囲外
				continue ;
			}
			float	xC = qpc.xC[j] ;
			float	yC = qpc.yC[j] ;
			float	zC = qpc.zC[j] ;
			float	absBC = qpc.absBC[j] ;
			if ( (xC - xB) * xPB + (yC - yB) * yPB + (zC - zB) * zPB
													< cosB_gap * absBC )
			{
				// 範囲外
				continue ;
			}
			float	xPC = xHit - xC ;
			float	yPC = yHit - yC ;
			float	zPC = zHit - zC ;
			double	absPC = sqrt( xPC * xPC + yPC * yPC + zPC * zPC ) ;
			float	absAC = qpc.absAC[j] ;
			float	cosC = qpc.cosC[j] ;
			double	cosC_gap = cosC * absPC - errGap * qpc.sinC[j] ;
			if ( (xA - xC) * xPC + (yA - yC) * yPC + (zA - zC) * zPC
													< cosC_gap * absAC )
			{
				// 範囲外
				continue ;
			}
			if ( (xB - xC) * xPC + (yB - yC) * yPC + (zB - zC) * zPC
													< cosC_gap * absBC )
			{
				// 範囲外
				continue ;
			}
			//
			// 交差
			//
			S3DVector	vHitPos( xHit, yHit, zHit ) ;
			S3DVector	vHitNormal( xNormal, yNormal, zNormal ) ;
			HitColliderCallback
				hccResult = rsHit.OnHitCollider
					( vHitPos, vHitNormal, pMeshCol, iPolygon ) ;
			if ( hccResult == S3DCollision::hitColliderReturn )
			{
				rsHit.vHitLocal.x = xHit ;
				rsHit.vHitLocal.y = yHit ;
				rsHit.vHitLocal.z = zHit ;
				rsHit.vHitGlobal.x = xHit ;
				rsHit.vHitGlobal.y = yHit ;
				rsHit.vHitGlobal.z = zHit ;
				rsHit.vNormalLocal.x = xNormal ;
				rsHit.vNormalLocal.y = yNormal ;
				rsHit.vNormalLocal.z = zNormal ;
				rsHit.vNormal.x = xNormal ;
				rsHit.vNormal.y = yNormal ;
				rsHit.vNormal.z = zNormal ;
				rsHit.iPolygon = (uint32_t) iPolygon ;
				rsHit.pMesh = pMeshCol ;
				rsHit.pPrimitiveMesh = pMeshCol ;
				rsHit.pqpcHit = &qpc ;
				return	true ;
			}
		}
	}
	return	false ;
}

#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
bool S3DCollision::DoesTriangleMeshHitAgainstSphere_SSE
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos, float fpRadius,
		S3DCollision::Result& rsHit, size_t iFirstPoly )
{
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector		vLocalPos = vPos ;
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = (size_t) 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = (size_t) 1 << nQPolyPackShift ;
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
#endif

// 球との交差判定の結果を大域空間に変換
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::ComputeGlobalOfHitAgainstSphere
	( S3DCollision::Result& rsHit,
		const S3DCollision::MeshCollision& mcMeshCol )
{
	RENDER_ENTRY *	preMesh = mcMeshCol.preMesh ;
	if ( preMesh != nullptr )
	{
		Transformation *	pTrans = preMesh->pTransform ;
		if ( pTrans != nullptr )
		{
			S3DMatrix	matTrans = pTrans->matTransform ;
			S3DVector	vTrans = pTrans->vTransform ;
			matTrans.RevolveVector( rsHit.vHitGlobal ) ;
			matTrans.RevolveVector( rsHit.vNormal ) ;
			matTrans.RevolveVector( rsHit.vLocalBase ) ;
			rsHit.vHitGlobal += vTrans ;
			rsHit.vNormal.Normalize() ;
			rsHit.matLocal = matTrans * rsHit.matLocal ;
			rsHit.vLocalBase += vTrans ;
		}
	}
	else
	{
		S3DMatrix	matTrans = mcMeshCol.itrans.matRev ;
		matTrans.RevolveVector( rsHit.vHitGlobal ) ;
		matTrans.RevolveVector( rsHit.vLocalBase ) ;
		rsHit.vHitGlobal += S3DVector( mcMeshCol.itrans.vPos ) ;
		if ( mcMeshCol.itrans.flagOrthMat )
		{
			matTrans.RevolveVector( rsHit.vNormal ) ;
			rsHit.vNormal.Normalize() ;
		}
		else
		{
			ComputeLocalNormalToGlobal( rsHit.vNormal, matTrans ) ;
		}
		rsHit.matLocal = matTrans * rsHit.matLocal ;
		rsHit.vLocalBase += S3DVector( mcMeshCol.itrans.vPos ) ;
	}
}

// 線分との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::IsSegmentCrossingMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos0, const S3DVector& vPos1,
			float fpErrorGap, S3DCollision::Result& rsCross )
{
	MeshCollisionType	typeCol = pMeshCol->typeCol ;
	switch ( typeCol )
	{
	case	typeTriangleList:
		#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
		if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
		{
			ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
			return	IsSegmentCrossingTriangleMesh_SSE
						( pMeshCol, vPos0, vPos1, fpErrorGap, rsCross ) ;
		}
		else
		#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
		{
			ESLAssert( (g_cpuFamily == cpuFamily_ARM) || (g_cpuFamily == cpuFamily_ARM64) ) ;
			return	IsSegmentCrossingTriangleMesh_NEON
						( pMeshCol, vPos0, vPos1, fpErrorGap, rsCross ) ;
		}
		else
		#endif
		{
			return	IsSegmentCrossingTriangleMesh
						( pMeshCol, vPos0, vPos1, fpErrorGap, rsCross ) ;
		}
		break ;

	case	typeCollider:
		if ( pMeshCol->pCollider != nullptr )
		{
			S3DCollisionResult::ColliderChain	colChain( pMeshCol ) ;
			rsCross.PushColliderChain( &colChain ) ;
			//
			S3DDVector	vClippedPos1 = vPos1 ;
			S3DDVector	vdPos0 = vPos0 ;
			vClippedPos1 -= vdPos0 ;
			vClippedPos1.Normalize() ;
			vClippedPos1 *= rsCross.fpDistance ;
			vClippedPos1 += vdPos0 ;
			//
			if ( pMeshCol->pCollider->
						IsSegmentCrossing
							( vdPos0, vClippedPos1, fpErrorGap, rsCross ) )
			{
				rsCross.PopColliderChain( &colChain ) ;
				rsCross.pMesh = pMeshCol ;
				rsCross.iInstance = pMeshCol->iInstance ;
				return	true ;
			}
			rsCross.PopColliderChain( &colChain ) ;
		}
		break ;

	case	typeSolidSphere:
		{
			float32_t	fpHitRadius = pMeshCol->fpRadius + fpErrorGap ;
			S3DVector	vSegRay = vPos1 - vPos0 ;
			double		fpSegLength = vSegRay.Absolute() ;
			if ( fpSegLength <= 1.0e-8 )
			{
				break ;
			}
			vSegRay *= 1.0f / (float32_t) fpSegLength ;
			//
			if ( IsSegmentCrossingSolidSphere
				( pMeshCol, vPos0, vSegRay, fpSegLength,
					pMeshCol->vCenter, fpHitRadius, rsCross ) )
			{
				rsCross.pMesh = pMeshCol ;
				rsCross.pPrimitiveMesh = pMeshCol ;
				return	true ;
			}
		}
		break ;

	case	typeLineList:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLine ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			uint32_t *		pIndex = pre->pIndexedList ;
			float32_t		fpThickness = fpErrorGap + pMeshCol->fpThickness ;
			S3DVector		vSegRay = vPos1 - vPos0 ;
			double			fpSegLength = vSegRay.Absolute() ;
			if ( fpSegLength <= 1.0e-8 )
			{
				break ;
			}
			vSegRay *= 1.0f / (float32_t) fpSegLength ;
			//
			bool	flagHit = false ;
			for ( size_t i = 0, j = 0; i < nLines; i ++, j += 2 )
			{
				if ( IsSegmentCrossingLine
					( pMeshCol, vPos0, vSegRay, fpSegLength,
						pvVertex[pIndex[j]],
						pvVertex[pIndex[j + 1]],
						fpThickness, rsCross ) )
				{
					rsCross.iPolygon = i ;
					rsCross.pMesh = pMeshCol ;
					rsCross.pPrimitiveMesh = pMeshCol ;
					flagHit = true ;
				}
			}
			return	flagHit ;
		}
		break ;

	case	typeLineStrip:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitiveLineStrip ) ;
			const size_t	nLines = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpErrorGap + pMeshCol->fpThickness ;
			S3DVector		vSegRay = vPos1 - vPos0 ;
			double			fpSegLength = vSegRay.Absolute() ;
			if ( fpSegLength <= 1.0e-8 )
			{
				break ;
			}
			vSegRay *= 1.0f / (float32_t) fpSegLength ;
			//
			bool	flagHit = false ;
			for ( size_t i = 0; i < nLines; i ++ )
			{
				if ( IsSegmentCrossingLine
					( pMeshCol, vPos0, vSegRay, fpSegLength,
						pvVertex[i], pvVertex[i + 1], fpThickness, rsCross ) )
				{
					rsCross.iPolygon = i ;
					rsCross.pMesh = pMeshCol ;
					rsCross.pPrimitiveMesh = pMeshCol ;
					flagHit = true ;
				}
			}
			return	flagHit ;
		}
		break ;

	case	typePoints:
		{
			RENDER_ENTRY *	pre = pMeshCol->preMesh ;
			ESLAssert( pre != nullptr ) ;
			ESLAssert( pre->nType == primitivePoint ) ;
			const size_t	nPoints = pre->countPrimitive ;
			S3DVector4 *	pvVertex = pre->pvVertex ;
			float32_t		fpThickness = fpErrorGap + pMeshCol->fpThickness ;
			S3DVector		vSegRay = vPos1 - vPos0 ;
			double			fpSegLength = vSegRay.Absolute() ;
			if ( fpSegLength <= 1.0e-8 )
			{
				break ;
			}
			vSegRay *= 1.0f / (float32_t) fpSegLength ;
			//
			bool	flagHit = false ;
			for ( size_t i = 0; i < nPoints; i ++ )
			{
				if ( IsSegmentCrossingSolidSphere
					( pMeshCol, vPos0, vSegRay, fpSegLength,
						pvVertex[i], fpThickness, rsCross ) )
				{
					rsCross.pMesh = pMeshCol ;
					rsCross.pPrimitiveMesh = pMeshCol ;
					flagHit = true ;
				}
			}
			return	flagHit ;
		}
		break ;

	case	typeSolidCube:
		{
			S3DVector	vLocalPos0 = vPos0 ;
			S3DVector	vLocalPos1 = vPos1 ;
			float	minR = -1.0f - fpErrorGap ;
			float	maxR = 1.0f + fpErrorGap ;
			if ( (minR <= vLocalPos0.x) && (vLocalPos0.x <= maxR)
				&& (minR <= vLocalPos0.y) && (vLocalPos0.y <= maxR)
				&& (minR <= vLocalPos0.z) && (vLocalPos0.z <= maxR) )
			{
				S3DVector	vNormal( 0, 0, 0 ) ;
				if ( fabs( vLocalPos0.x ) > fabs( vLocalPos0.y ) )
				{
					if ( fabs( vLocalPos0.x ) > fabs( vLocalPos0.z ) )
					{
						vNormal.x = (vLocalPos0.x >= 0.0f) ? 1.0f : -1.0f ;
					}
					else
					{
						vNormal.z = (vLocalPos0.z >= 0.0f) ? 1.0f : -1.0f ;
					}
				}
				else if ( fabs( vLocalPos0.y ) > fabs( vLocalPos0.z ) )
				{
					vNormal.y = (vLocalPos0.y >= 0.0f) ? 1.0f : -1.0f ;
				}
				else
				{
					vNormal.z = (vLocalPos0.z >= 0.0f) ? 1.0f : -1.0f ;
				}
				if ( rsCross.OnHitCollider
					( vLocalPos0, vNormal, pMeshCol, 0 )
									== S3DCollision::hitColliderReturn )
				{
					rsCross.vHitLocal = vLocalPos0 ;
					rsCross.vHitGlobal = vLocalPos0 ;
					rsCross.vNormal = vNormal ;
					rsCross.vNormalLocal = vNormal ;
					rsCross.fpDistance = 0 ;
					rsCross.iPolygon = 0 ;
					rsCross.pMesh = pMeshCol ;
					rsCross.pPrimitiveMesh = pMeshCol ;
					rsCross.pqpcHit = nullptr ;
					//
					return	true ;
				}
			}
			S3DVector	vSegRay = vLocalPos1 - vLocalPos0 ;
			double		fpSegLength = vSegRay.Absolute() ;
			if ( fpSegLength < 1.0e-8 )
			{
				break ;
			}
			vSegRay *= 1.0f / (float32_t) fpSegLength ;
			//
			float32_t	xc = (vLocalPos0.x > 0) ? 1.0f : -1.0f ;
			float32_t	yc = (vLocalPos0.y > 0) ? 1.0f : -1.0f ;
			float32_t	zc = (vLocalPos0.z > 0) ? 1.0f : -1.0f ;
			//
			float32_t	xt = rsCross.fpDistance + 1.0f ;
			float32_t	yt = xt ;
			float32_t	zt = xt ;
			//
			S3DVector	vx, vy, vz ;
			bool		hx = false, hy = false, hz = false ;
			//
			if ( fabs( vSegRay.x ) > 1.0e-8 )
			{
				double	t = (xc - vLocalPos0.x) / vSegRay.x ;
				if ( (t > 0.0)
					&& (t < fpSegLength) && (t < rsCross.fpDistance) )
				{
					xt = (float32_t) t ;
					vx = vLocalPos0 ;
					vx += vSegRay * t ;
					hx = (minR <= vx.y) && (vx.y <= maxR)
						&& (minR <= vx.z) && (vx.z <= maxR) ;
				}
			}
			if ( fabs( vSegRay.y ) > 1.0e-8 )
			{
				double	t = (yc - vLocalPos0.y) / vSegRay.y ;
				if ( (t > 0.0)
					&& (t < fpSegLength) && (t < rsCross.fpDistance) )
				{
					yt = (float32_t) t ;
					vy = vLocalPos0 ;
					vy += vSegRay * t ;
					hy = (minR <= vy.x) && (vy.x <= maxR)
						&& (minR <= vy.z) && (vy.z <= maxR) ;
				}
			}
			if ( fabs( vSegRay.z ) > 1.0e-8 )
			{
				double	t = (zc - vLocalPos0.z) / vSegRay.z ;
				if ( (t > 0.0)
					&& (t < fpSegLength) && (t < rsCross.fpDistance) )
				{
					zt = (float32_t) t ;
					vz = vLocalPos0 ;
					vz += vSegRay * t ;
					hz = (minR <= vz.x) && (vz.x <= maxR)
						&& (minR <= vz.y) && (vz.y <= maxR) ;
				}
			}
			S3DVector	vHitPos ;
			S3DVector	vNormal( 0, 0, 0 ) ;
			float32_t	fpDistance ;
			if ( xt < yt )
			{
				if ( xt < zt )
				{
					if ( !hx )
					{
						break ;
					}
					vHitPos = vx ;
					vNormal.x = (vLocalPos0.x > 0) ? 1.0f : -1.0f ;
					fpDistance = xt ;
				}
				else
				{
					if ( !hz )
					{
						break ;
					}
					vHitPos = vz ;
					vNormal.z = (vLocalPos0.z > 0) ? 1.0f : -1.0f ;
					fpDistance = zt ;
				}
			}
			else
			{
				if ( yt < zt )
				{
					if ( !hy )
					{
						break ;
					}
					vHitPos = vy ;
					vNormal.y = (vLocalPos0.y > 0) ? 1.0f : -1.0f ;
					fpDistance = yt ;
				}
				else
				{
					if ( !hz )
					{
						break ;
					}
					vHitPos = vz ;
					vNormal.z = (vLocalPos0.z > 0) ? 1.0f : -1.0f ;
					fpDistance = zt ;
				}
			}
			if ( rsCross.OnHitCollider( vHitPos, vNormal, pMeshCol, 0 )
									== S3DCollision::hitColliderReturn )
			{
				rsCross.vHitLocal = vHitPos ;
				rsCross.vHitGlobal = vHitPos ;
				rsCross.vNormal = vNormal ;
				rsCross.vNormalLocal = vNormal ;
				rsCross.fpDistance = fpDistance ;
				rsCross.iPolygon = 0 ;
				rsCross.pMesh = pMeshCol ;
				rsCross.pPrimitiveMesh = pMeshCol ;
				rsCross.pqpcHit = nullptr ;
				return	true ;
			}
		}
		break ;
	}
	return	false ;
}

bool S3DCollision::IsSegmentCrossingSolidSphere
	( const MeshCollision * pMeshCol,
		const S3DVector& vPos0,
		const S3DVector& vSegRay, double fpSegLength,
		const S3DVector& vCenter, float fpRadius,
		S3DCollision::Result& rsCross )
{
	S3DVector	vDelta = vCenter - vPos0 ;
	double		fpDistance = vDelta.Absolute() ;
	if ( fpDistance <= 1.0e-8 )
	{
		S3DVector	vNormal( 1, 0, 0 ) ;
		if ( rsCross.OnHitCollider( vPos0, vNormal, pMeshCol, 0 )
								== S3DCollision::hitColliderReturn )
		{
			rsCross.vHitLocal = vPos0 ;
			rsCross.vHitGlobal = vPos0 ;
			rsCross.vNormal = vNormal ;
			rsCross.vNormalLocal = vNormal ;
			rsCross.iPolygon = 0 ;
			rsCross.pqpcHit = nullptr ;
			return	true ;
		}
	}
	//
	S3DVector	vHitPos = vPos0 ;
	double		rx = vSegRay.InnerProduct( vDelta ) ;
	if ( rx < 0.0 )
	{
		if ( fpDistance > fpRadius )
		{
			return	false ;
		}
	}
	else
	{
		vHitPos += vSegRay * rx ;
		//
		double	r = (vHitPos - vCenter).Absolute() ;
		if ( r > fpRadius )
		{
			return	false ;
		}
		double	t = sqrt( fpRadius * fpRadius - r * r ) ;
		rx -= t ;
		if ( (rx > fpSegLength)
			|| (rx > rsCross.fpDistance) )
		{
			return	false ;
		}
		if ( rx > 0.0 )
		{
			vHitPos -= vSegRay * t ;
		}
		else
		{
			vHitPos = vPos0 ;
		}
	}
	S3DVector	vNormal = (vHitPos - vCenter).Normalized() ;
	if ( rsCross.OnHitCollider( vHitPos, vNormal, pMeshCol, 0 )
								== S3DCollision::hitColliderReturn )
	{
		rsCross.vHitLocal = vHitPos ;
		rsCross.vHitGlobal = vHitPos ;
		rsCross.vNormal = vNormal ;
		rsCross.vNormalLocal = vNormal ;
		rsCross.fpDistance = (float32_t) rx ;
		rsCross.iPolygon = 0 ;
		rsCross.pqpcHit = nullptr ;
		return	true ;
	}
	return	false ;
}

bool S3DCollision::IsSegmentCrossingLine
	( const MeshCollision * pMeshCol,
		const S3DVector& vPos0,
		const S3DVector& vSegRay, double fpSegLength,
		const S3DVector& vLine0,
		const S3DVector& vLine1, float fpThickness,
		S3DCollision::Result& rsCross )
{
	S3DVector	vCross = vSegRay * (vLine1 - vLine0) ;
	S3DVector	vDelta0 = vLine0 - vPos0 ;
	S3DVector	vDelta1 = vLine1 - vPos0 ;
	double		d0 = vDelta0.Absolute() ;
	double		d1 = vDelta1.Absolute() ;
	float32_t	t0 = vSegRay.InnerProduct( vDelta0 ) ;
	float32_t	t1 = vSegRay.InnerProduct( vDelta1 ) ;
	double		cr = vCross.Absolute() ;
	float32_t	t = t0 ;
	if ( cr < 1.0e-7 )
	{
		if ( t0 * t1 > 0.0 )
		{
			if ( fabs(t0) > fabs(t1) )
			{
				t = t1 ;
			}
		}
	}
	else
	{
		S3DVector	vCrossY = (vCross * vSegRay).Normalized() ;
		vCross *= (float32_t) (1.0f / cr) ;
		//
		float32_t	y0 = vCrossY.InnerProduct( vDelta0 ) ;
		float32_t	y1 = vCrossY.InnerProduct( vDelta1 ) ;
		if ( fabs( t1 - t0 ) > 1.0e-7 )
		{
			float32_t	a = (y1 - y0) / (t1 - t0) ;
			if ( fabs( a ) > 1.0e-7 )
			{
				float32_t	b = y0 - a * t0 ;
				t = -b / a ;
			}
		}
	}
	bool	fCrossHit = false ;
	if ( (t >= 0.0f) && (t <= fpSegLength) )
	{
		S3DVector	vTest = vPos0 + vSegRay * t
							+ vCross * vDelta0.InnerProduct(vCross) ;
		fCrossHit = IsSegmentCrossingSolidSphere
			( pMeshCol, vPos0, vSegRay, fpSegLength, vTest, fpThickness, rsCross ) ;
	}
	return	fCrossHit
		|| IsSegmentCrossingSolidSphere
			( pMeshCol, vPos0, vSegRay, fpSegLength, vLine0, fpThickness, rsCross )
		|| IsSegmentCrossingSolidSphere
			( pMeshCol, vPos0, vSegRay, fpSegLength, vLine1, fpThickness, rsCross ) ;
}

bool S3DCollision::IsSegmentCrossingTriangleMesh
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos0, const S3DVector& vPos1,
						float fpErrorGap, S3DCollision::Result& rsCross )
{
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector	vLocalPos0 = vPos0 ;
	S3DVector	vLocalPos1 = vPos1 ;
	S3DVector	vSegRay = vLocalPos1 - vLocalPos0 ;
	double		fpSegLength = vSegRay.Absolute() ;
	vSegRay.Normalize() ;
	//
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = (size_t) 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = (size_t) 1 << nQPolyPackShift ;
	const size_t	nQPolyPackOddMask = nQPolyPackCount - 1 ;
	const size_t	nPolygons = pMeshCol->nPolygons ;
	const size_t	nQPolygons = (nPolygons + nQPolyPackOddMask)
												>> nQPolyPackShift ;
	//
	bool	fCross = false ;
	//
	for ( size_t i = 0; i < nQPolygons; i ++ )
	{
		const QuadSphereCollision&	qsc = pSphereCol[i >> 2] ;
		size_t	k = i & 0x03 ;
		float	dx = qsc.xCenter[k] - vLocalPos0.x ;
		float	dy = qsc.yCenter[k] - vLocalPos0.y ;
		float	dz = qsc.zCenter[k] - vLocalPos0.z ;
		float	r = (float) fpErrorGap + qsc.fpRadius[k] ;
		float	t = dx * vSegRay.x + dy * vSegRay.y + dz * vSegRay.z ;
		if ( (t < -r) | (t > fpSegLength + r)
					| (t - r > rsCross.fpDistance) )
		{
			// 範囲外
			continue ;
		}
		if ( dx * dx + dy * dy + dz * dz - t * t > r * r )
		{
			// 範囲外
			continue ;
		}
		for ( size_t p = 0; p < nQPolyPackCount; p ++ )
		{
			const size_t	iPolygon = (i << nQPolyPackShift) + p ;
			if ( iPolygon >= nPolygons )
			{
				break ;
			}
			const QuadPolygonsCollision&	qpc = pCollision[iPolygon >> 2] ;
			const size_t					j = iPolygon & 0x03 ;
			//
			// 交点計算
			//
			float	xNormal = qpc.xNormal[j] ;
			float	yNormal = qpc.yNormal[j] ;
			float	zNormal = qpc.zNormal[j] ;
			float	xA = qpc.xA[j] ;
			float	yA = qpc.yA[j] ;
			float	zA = qpc.zA[j] ;
			dx = xA - vLocalPos0.x ;
			dy = yA - vLocalPos0.y ;
			dz = zA - vLocalPos0.z ;
			r = vSegRay.x * xNormal
				+ vSegRay.y * yNormal + vSegRay.z * zNormal ;
			t = 0.0 ;
			if ( fabs(r) < 1.0e-6 )
			{
				continue ;
			}
			t = (dx * xNormal + dy * yNormal + dz * zNormal) / r ;
			if ( (t < 0.0)
				| (t >= rsCross.fpDistance) )
			{
				// 非接触
				continue ;
			}
			float	xHit = vLocalPos0.x + vSegRay.x * t ;
			float	yHit = vLocalPos0.y + vSegRay.y * t ;
			float	zHit = vLocalPos0.z + vSegRay.z * t ;
			//
			// 三角形範囲判定
			//
			float	xB = qpc.xB[j] ;
			float	yB = qpc.yB[j] ;
			float	zB = qpc.zB[j] ;
			float	xPB = xHit - xB ;
			float	yPB = yHit - yB ;
			float	zPB = zHit - zB ;
			double	absPB = sqrt( xPB * xPB + yPB * yPB + zPB * zPB ) ;
			float	absAB = qpc.absAB[j] ;
			float	cosB = qpc.cosB[j] ;
			double	cosB_gap = cosB * absPB - fpErrorGap * qpc.sinB[j] ;
			if ( (xA - xB) * xPB + (yA - yB) * yPB + (zA - zB) * zPB
													< cosB_gap * absAB )
			{
				// 範囲外
				continue ;
			}
			float	xC = qpc.xC[j] ;
			float	yC = qpc.yC[j] ;
			float	zC = qpc.zC[j] ;
			float	absBC = qpc.absBC[j] ;
			if ( (xC - xB) * xPB + (yC - yB) * yPB + (zC - zB) * zPB
													< cosB_gap * absBC )
			{
				// 範囲外
				continue ;
			}
			float	xPC = xHit - xC ;
			float	yPC = yHit - yC ;
			float	zPC = zHit - zC ;
			double	absPC = sqrt( xPC * xPC + yPC * yPC + zPC * zPC ) ;
			float	absAC = qpc.absAC[j] ;
			float	cosC = qpc.cosC[j] ;
			double	cosC_gap = cosC * absPC - fpErrorGap * qpc.sinC[j] ;
			if ( (xA - xC) * xPC + (yA - yC) * yPC + (zA - zC) * zPC
													< cosC_gap * absAC )
			{
				// 範囲外
				continue ;
			}
			if ( (xB - xC) * xPC + (yB - yC) * yPC + (zB - zC) * zPC
													< cosC_gap * absBC )
			{
				// 範囲外
				continue ;
			}
			//
			// 交差
			//
			S3DVector	vHitPos( xHit, yHit, zHit ) ;
			S3DVector	vHitNormal( xNormal, yNormal, zNormal ) ;
			HitColliderCallback
				hccResult = rsCross.OnHitCollider
					( vHitPos, vHitNormal, pMeshCol, iPolygon ) ;
			if ( hccResult == S3DCollision::hitColliderReturn )
			{
				rsCross.vHitLocal.x = xHit ;
				rsCross.vHitLocal.y = yHit ;
				rsCross.vHitLocal.z = zHit ;
				rsCross.vHitGlobal.x = xHit ;
				rsCross.vHitGlobal.y = yHit ;
				rsCross.vHitGlobal.z = zHit ;
				rsCross.vNormalLocal.x = xNormal ;
				rsCross.vNormalLocal.y = yNormal ;
				rsCross.vNormalLocal.z = zNormal ;
				rsCross.vNormal.x = xNormal ;
				rsCross.vNormal.y = yNormal ;
				rsCross.vNormal.z = zNormal ;
				rsCross.fpDistance = t ;
				rsCross.iPolygon = (uint32_t) iPolygon ;
				rsCross.pMesh = pMeshCol ;
				rsCross.pPrimitiveMesh = pMeshCol ;
				rsCross.pqpcHit = &qpc ;
				fCross = true ;
			}
		}
	}
	return	fCross ;
}

#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
bool S3DCollision::IsSegmentCrossingTriangleMesh_SSE
	( const S3DCollision::MeshCollision * pMeshCol,
		const S3DVector& vPos0, const S3DVector& vPos1,
						float fpErrorGap, S3DCollision::Result& rsCross )
{
	QuadSphereCollision *	pSphereCol = pMeshCol->pSphereCol ;
	QuadPolygonsCollision *	pCollision = pMeshCol->pCollision ;
	S3DVector	vLocalPos0 = vPos0 ;
	S3DVector	vLocalPos1 = vPos1 ;
	S3DVector	vSegRay = vLocalPos1 - vLocalPos0 ;
	double		fpSegLength = vSegRay.Absolute() ;
	vSegRay.Normalize() ;
	//
	const size_t	nQSPackScale = pMeshCol->nQSPackScale ;
	const size_t	nQSPackCount = (size_t) 1 << nQSPackScale ;
	const size_t	nQPolyPackShift = nQSPackScale + 2 ;
	const size_t	nQPolyPackCount = (size_t) 1 << nQPolyPackShift ;
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

// 追加判定幅
//////////////////////////////////////////////////////////////////////////////
float32_t S3DCollision::GetCurrentThickness( void ) const
{
	return	m_fpAddThickness ;
}

void S3DCollision::SetCurrentThickness( float32_t fpThickness )
{
	m_fpAddThickness = fpThickness ;
}

// 以降に追加するメッシュに関連付けるユーザーデータを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::AttachMeshUserData( ESLObject * pUserData )
{
	m_pUserData = pUserData ;
}

ESLObject * S3DCollision::GetMeshUserData( void ) const
{
	return	m_pUserData ;
}

// シーンクラスマスク（システム規定）
// （※S3DScene::ItemClass に対応するビットマスク）
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::SetSceneClassesMask( uint32_t maskClasses )
{
	uint64_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	m_maskClasses = (m_maskClasses & ~maskSys) | (maskClasses & maskSys) ;
}

uint32_t S3DCollision::GetSceneClassesMask( void ) const
{
	uint32_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	return	(uint32_t) (m_maskClasses & maskSys) ;
}

// ユーザー拡張クラスマスク
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::SetUserClassesMask( uint32_t maskClasses )
{
	uint64_t	maskSys = (1 << S3DScene::classCount) - 1 ;
	m_maskClasses = (m_maskClasses & maskSys)
					| ((uint64_t) maskClasses << S3DScene::classCount) ;
}

uint32_t S3DCollision::GetUserClassesMask( void ) const
{
	return	(uint32_t) (m_maskClasses >> S3DScene::classCount) ;
}

// maskClasses から UserColliderClass 成分の分離
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DCollision::GetUserColliderMask( uint64_t maskClasses )
{
	return	(uint32_t) (maskClasses >> S3DScene::classCount) ;
}

// maskClasses へ UserColliderClass 成分の合成用
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DCollision::MakeUserColliderMask( uint32_t maskColliders )
{
	return	((uint64_t) maskColliders) << S3DScene::classCount ;
}

// コンテキストの階層化
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::PushTransformation( void )
{
	CollisionContext	ctx ;
	ctx.pUserData = m_pUserData ;
	ctx.maskClasses = m_maskClasses ;
	ctx.fpAddThickness = m_fpAddThickness ;
	//
	m_arrCtxStack.Push( ctx ) ;
	//
	return	S3DRenderBuffer::PushTransformation() ;
}

SGLError S3DCollision::PopTransformation( void )
{
	if ( m_arrCtxStack.GetLength() >= 1 )
	{
		CollisionContext	ctx = m_arrCtxStack.Pop() ;
		m_pUserData = ctx.pUserData ;
		m_maskClasses = ctx.maskClasses ;
		m_fpAddThickness = ctx.fpAddThickness ;
	}
	return	S3DRenderBuffer::PopTransformation() ;
}

SGLError S3DCollision::ResetTransformation( void )
{
	m_arrCtxStack.RemoveAll() ;
	m_pUserData = nullptr ;
	m_fpAddThickness = 0.0f ;
	m_maskClasses = (uint64_t) colliderShape << S3DScene::classCount ;
	//
	return	S3DRenderBuffer::ResetTransformation() ;
}

// ソリッド球を追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddSolidSphere
	( const S3DVector& vPos, float32_t fpRadius, size_t iInstanceNum )
{
	if ( fpRadius <= 0.0 )
	{
		return	sglErrFailed ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	size_t		iCol = m_arrCollision.GetLength() ;
	//
	MeshCollision *	pMeshCol =
		(MeshCollision*) m_bufRender.Allocate( sizeof(MeshCollision) ) ;
	m_arrCollision.Add( pMeshCol ) ;
	//
	GetITransformation( pMeshCol->itrans, m_pTransformation ) ;
	//
	pMeshCol->typeCol = typeSolidSphere ;
	pMeshCol->vCenter = vPos ;
	pMeshCol->fpRadius = fpRadius + m_fpAddThickness ;
	pMeshCol->fpThickness = m_fpAddThickness ;
	pMeshCol->pSphereCol = nullptr ;
	pMeshCol->pCollision = nullptr ;
	pMeshCol->pCollider = nullptr ;
	pMeshCol->iInstance = iInstanceNum ;
	pMeshCol->iMesh = m_arrCollisionIndex.GetLength() ;
	pMeshCol->preMesh = nullptr ;
	pMeshCol->nPolygons = 0 ;
	pMeshCol->pUserData = m_pUserData ;
	pMeshCol->maskClasses = m_maskClasses ;
	//
	m_arrCollisionIndex.Add( iCol ) ;
	//
	AddMeshNodeToTree( pMeshCol ) ;
	//
	return	sglErrSuccess ;
}

// ソリッド直方体を追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddSolidCube
	( const S3DVector& vPos, const S3DVector& vCubeSize, size_t iInstanceNum )
{
	if ( (fabs( vCubeSize.x ) < 1.0e-8)
		|| (fabs( vCubeSize.y ) < 1.0e-8)
		|| (fabs( vCubeSize.z ) < 1.0e-8) )
	{
		return	sglErrFailed ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	size_t		iCol = m_arrCollision.GetLength() ;
	//
	MeshCollision *	pMeshCol =
		(MeshCollision*) m_bufRender.Allocate( sizeof(MeshCollision) ) ;
	m_arrCollision.Add( pMeshCol ) ;
	//
	S3DDMatrix	matdSize
		( vCubeSize.x, 0, 0,  0, vCubeSize.y, 0,  0, 0, vCubeSize.z ) ;
	S3DDVector	vdPos = vPos ;
	//
	PushTransformation() ;
	AppendMatrixTransformation( matdSize, vdPos ) ;
	//
	GetITransformation( pMeshCol->itrans, m_pTransformation ) ;
	//
	pMeshCol->typeCol = typeSolidCube ;
	pMeshCol->vCenter.x = 0 ;
	pMeshCol->vCenter.y = 0 ;
	pMeshCol->vCenter.z = 0 ;
	pMeshCol->fpRadius =
		(float32_t) vCubeSize.Absolute() + m_fpAddThickness * 2.0f ;
	pMeshCol->fpThickness = m_fpAddThickness ;
	pMeshCol->pSphereCol = nullptr ;
	pMeshCol->pCollision = nullptr ;
	pMeshCol->pCollider = nullptr ;
	pMeshCol->iInstance = iInstanceNum ;
	pMeshCol->iMesh = m_arrCollisionIndex.GetLength() ;
	pMeshCol->preMesh = nullptr ;
	pMeshCol->nPolygons = 0 ;
	pMeshCol->pUserData = m_pUserData ;
	pMeshCol->maskClasses = m_maskClasses ;
	//
	m_arrCollisionIndex.Add( iCol ) ;
	//
	AddMeshNodeToTree( pMeshCol ) ;
	//
	PopTransformation() ;
	//
	return	sglErrSuccess ;
}

// ソリッド円柱を追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddSolidTubeList
	( const S3DVector* pPoints, size_t nPointCount, float32_t fpRadius )
{
	if ( fpRadius <= 0.0 )
	{
		return	sglErrFailed ;
	}
	S3DVector4			vTempPoints[16] ;
	S3DVector4 *		pvPoints = vTempPoints ;
	SArray<S3DVector4>	bufTempPoints ;
	if ( nPointCount > 16 )
	{
		pvPoints = bufTempPoints.GetArray( nPointCount ) ;
	}
	for ( size_t i = 0; i < nPointCount; i ++ )
	{
		pvPoints[i] = pPoints[i] ;
	}
	//
	SGLError	err ;
	PushTransformation() ;
	SetCurrentThickness( fpRadius ) ;
	err = AddIndexedPrimitiveList
		( &S3DMaterial::m_materialDefault[S3DMaterial::defaultWhite], 0,
			primitiveLineStrip, 0, nPointCount,
			pvPoints, nullptr, nullptr, nullptr, nullptr ) ;
	PopTransformation() ;
	//
	return	err ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	size_t		iCol = m_arrCollision.GetLength() ;
	SGLError	err =
		S3DRenderBuffer::AddIndexedPrimitiveList
			( pMaterial, nFlags,
				typePrimitive, countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	if ( !err )
	{
		m_arrCollisionIndex.Add( iCol ) ;
	}
	return	err ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	size_t			iCol = m_arrCollision.GetLength() ;
	SGLError		err = sglErrSuccess ;
	S3DCollision *	pCol = ESLTypeCast<S3DCollision>( pBuffer ) ;
	if ( (pCol != nullptr) && (iFirst == 0)
			&& ((iEnd < 0) || ((size_t) iEnd >= pBuffer->GetMeshCount())) )
	{
		if ( nInstancing && pmatInstancing )
		{
			for ( size_t i = 0; i < nInstancing; i ++ )
			{
				err = AddColliderObject( pCol, pmatInstancing + i, i ) ;
			}
		}
		else
		{
			err = AddColliderObject( pCol, nullptr, 0 ) ;
		}
	}
	else
	{
		S3DRenderBuffer *	prb = ESLTypeCast<S3DRenderBuffer>( pBuffer ) ;
		if ( prb != nullptr )
		{
			err = prb->RenderTemporaryBufferTo
				( this, 0, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
		}
		else
		{
			err = S3DRenderBuffer::AddVertexBuffer
					( pMaterial, nFlags, pBuffer, iFirst, iEnd,
						nInstancing, pmatInstancing, pColorInstancing ) ;
		}
	}
	return	err ;
}

SGLError S3DCollision::AddColliderObject
	( S3DCollider * pCollider,
		const S4DMatrix * pmatInstancing, size_t iInstanceNum )
{
	SSmartLock<SCriticalSection>	lock( &m_csBufSync ) ;
	//
	size_t		iCol = m_arrCollision.GetLength() ;
	//
	MeshCollision *	pMeshCol =
		(MeshCollision*) m_bufRender.Allocate( sizeof(MeshCollision) ) ;
	m_arrCollision.Add( pMeshCol ) ;
	//
	GetITransformationInstancing
		( pMeshCol->itrans, m_pTransformation, pmatInstancing ) ;
	//
	S3DDVector	vColCenter ;
	double		fpColRadius ;
	pCollider->GetCollisionRange( vColCenter, fpColRadius ) ;
	//
	pMeshCol->typeCol = typeCollider ;
	pMeshCol->vCenter = vColCenter ;
	pMeshCol->fpRadius =
		(float32_t) (fpColRadius + m_fpAddThickness * 2.0f) * 1.001f ;
	pMeshCol->fpThickness = m_fpAddThickness ;
	pMeshCol->pSphereCol = nullptr ;
	pMeshCol->pCollision = nullptr ;
	pMeshCol->pCollider = pCollider ;
	pMeshCol->iMesh = m_arrCollisionIndex.GetLength() ;
	pMeshCol->iInstance = iInstanceNum ;
	pMeshCol->preMesh = nullptr ;
	pMeshCol->nPolygons = 0 ;
	pMeshCol->pUserData = m_pUserData ;
	pMeshCol->maskClasses = m_maskClasses ;
	//
	m_arrCollisionIndex.Add( iCol ) ;
	//
	AddMeshNodeToTree( pMeshCol ) ;
	//
	return	sglErrSuccess ;
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddColliderDescription
	( const S3DCollision::ColliderDescription& desc, size_t iInstanceNum )
{
	switch ( desc.type )
	{
	case	coliderTypeBuffer:
		if ( desc.pColider != nullptr )
		{
			return	AddColliderObject( desc.pColider, nullptr, iInstanceNum ) ;
		}
		break ;

	case	coliderTypeSolidSphere:
		return	AddSolidSphere( desc.vPos, desc.fpRadius, iInstanceNum ) ;

	case	coliderTypeSolidCube:
		return	AddSolidCube( desc.vPos, desc.vCubeSize, iInstanceNum ) ;

	default:
		break ;
	}
	return	sglErrFailed ;
}

// プリミティブを追加するためのバッファを確保する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AllocatePrimitiveBuffer
	( PrimitiveBuffer& prmbuf,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	S3DVector4 *	pvBuf =
		(S3DVector4*) esl_malloc
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
			(uint32_t*) esl_malloc( countIndex * sizeof(uint32_t) ) ;
	}
	else
	{
		prmbuf.pIndexedList = nullptr ;
	}
	return	sglErrSuccess ;
}

// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::AddPrimitiveBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		const PrimitiveBuffer& prmbuf,
		size_t countIndex, size_t countVertex )
{
	SGLError	err =
		AddIndexedPrimitiveList
			( pMaterial, nFlags, typePrimitive,
				countIndex, countVertex,
				prmbuf.pvVertex, prmbuf.pvNormal,
				prmbuf.pvUVMap, prmbuf.pColor, prmbuf.pIndexedList ) ;
	FreePrimitiveBuffer( prmbuf ) ;
	return	err ;
}

// プリミティブを追加せずにバッファを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::FreePrimitiveBuffer
	( const PrimitiveBuffer& prmbuf )
{
	if ( prmbuf.pvVertex != nullptr )
	{
		esl_free( prmbuf.pvVertex ) ;
	}
	if ( prmbuf.pIndexedList != nullptr )
	{
		esl_free( prmbuf.pIndexedList ) ;
	}
	return	sglErrSuccess ;
}

// プリミティブリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::UpdateIndexedPrimitiveList
	( size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	SGLError	err =
		S3DRenderBuffer::UpdateIndexedPrimitiveList
			( iMesh, nFlags, countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	if ( !err )
	{
		size_t *	pColIndex = m_arrCollisionIndex.GetAt( iMesh ) ;
		if ( pColIndex != nullptr )
		{
			MeshCollision *	pCol = m_arrCollision.GetAt( *pColIndex ) ;
			if ( pCol != nullptr )
			{
				BuildTriangleListCollision
					( pCol->pSphereCol, pCol->pCollision,
						pCol->nQSPackScale,
						countIndex / 3, pvVertex, pIndexedList ) ;
				UpdateMeshNodeForTree( pCol ) ;
			}
		}
	}
	return	err ;
}

// バッファを消去
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::ClearBuffer( void )
{
	S3DRenderBuffer::ClearBuffer() ;
	//
	m_pRootNode = nullptr ;
	m_arrCollision.RemoveAll() ;
	m_arrCollisionIndex.RemoveAll() ;
	m_arrMeshNode.RemoveAll() ;
	m_pUserData = nullptr ;
}

// デバイスリソースを解放
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::ReleaseAllDeviceResources( void )
{
}

// 追加時処理 (デフォルトは頂点バッファ複製)
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::OnAddRenderBuffer
	( S3DRenderBuffer::RENDER_ENTRY& entry,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	if ( !S3DRenderBuffer::OnAddRenderBuffer
		( entry, pvVertex, pvNormal, pvUVMap, pColor ) )
	{
		return	false ;
	}
	MeshCollision *	pMeshCol =
		(MeshCollision*) m_bufRender.Allocate( sizeof(MeshCollision) ) ;
	pMeshCol->typeCol = typeTriangleList ;
	pMeshCol->fpThickness = m_fpAddThickness ;
	pMeshCol->pCollider = nullptr ;
	pMeshCol->iInstance = 0 ;
	pMeshCol->iMesh = m_arrCollisionIndex.GetLength() ;
	pMeshCol->preMesh = &entry ;
	pMeshCol->nPolygons = entry.countPrimitive ;
	pMeshCol->pUserData = m_pUserData ;
	pMeshCol->maskClasses = m_maskClasses ;
	//
	m_arrCollision.Add( pMeshCol ) ;
	//
	// 逆変換行列
	//
	GetITransformation( pMeshCol->itrans, entry.pTransform ) ;
	//
	// 中心座標とサイズ計算
	//
	pMeshCol->vCenter = entry.vCenter ;
	pMeshCol->fpRadius = (entry.fpRadius + m_fpAddThickness * 2.0f) * 1.001f ;
	//
	if ( (entry.nType == typeIndexedTriangleList)
		|| (entry.nType == typeTriangleStrip) )
	{
		//
		// ポリゴン単位の当たり判定パラメータ計算
		//
		size_t	countPolygon = entry.countPrimitive ;
		size_t	nQPackShift, nQPackCount, nQPackOdd ;
		pMeshCol->nQSPackScale =
					CalculateScaleForQuadSpherePolygons( countPolygon ) ;
		nQPackShift = pMeshCol->nQSPackScale + 4 ;
		nQPackCount = (size_t) 1 << nQPackShift ;
		nQPackOdd = nQPackCount - 1 ;
		//
		QuadSphereCollision *	pSphereCol =
			(QuadSphereCollision*)
				m_bufRender.Allocate
					( ((countPolygon + nQPackOdd) >> nQPackShift)
							* sizeof(QuadSphereCollision) ) ;
		QuadPolygonsCollision *	pCollision =
			(QuadPolygonsCollision*)
				m_bufRender.Allocate
					( ((countPolygon + 0x03) >> 2)
							* sizeof(QuadPolygonsCollision) ) ;
		ESLAssert( ((ulong_ptr_t) pSphereCol & 0x0F) == 0 ) ;
		ESLAssert( ((ulong_ptr_t) pCollision & 0x0F) == 0 ) ;
		pMeshCol->pSphereCol = pSphereCol ;
		pMeshCol->pCollision = pCollision ;
		//
		if ( entry.nType == typeIndexedTriangleList )
		{
			if ( entry.pIndexedList == nullptr )
			{
				const size_t	nIndexCount = countPolygon * 3 ;
				entry.pIndexedList =
					(uint32_t*) m_bufRender.Allocate
									( nIndexCount * sizeof(uint32_t) ) ;
				for ( size_t i = 0; i < nIndexCount; i ++ )
				{
					entry.pIndexedList[i] = (uint32_t) i ;
				}
			}
			BuildTriangleListCollision
				( pSphereCol, pCollision,
					pMeshCol->nQSPackScale,
					countPolygon, entry.pvVertex, entry.pIndexedList ) ;
		}
		else if ( entry.nType == typeTriangleStrip )
		{
			const uint32_t *	pIndexedList =
				m_indexTriangleStrip.MakeIndexList( countPolygon ) ;
			entry.pIndexedList =
				(uint32_t*) m_bufRender.Allocate
								( countPolygon * (sizeof(uint32_t) * 3) ) ;
			eslMoveMemory
				( entry.pIndexedList, pIndexedList,
						countPolygon * (sizeof(uint32_t) * 3) ) ;
			BuildTriangleListCollision
				( pSphereCol, pCollision,
					pMeshCol->nQSPackScale,
					countPolygon, entry.pvVertex, pIndexedList ) ;
		}
	}
	else if ( entry.nType == primitiveLine )
	{
		pMeshCol->typeCol = typeLineList ;
		pMeshCol->pSphereCol = nullptr ;
		pMeshCol->pCollision = nullptr ;
	}
	else if ( entry.nType == primitiveLineStrip )
	{
		pMeshCol->typeCol = typeLineStrip ;
		pMeshCol->pSphereCol = nullptr ;
		pMeshCol->pCollision = nullptr ;
	}
	else if ( entry.nType == primitivePoint )
	{
		pMeshCol->typeCol = typePoints ;
		pMeshCol->pSphereCol = nullptr ;
		pMeshCol->pCollision = nullptr ;
	}
	else
	{
		ESLAssert( entry.nType == typeVertexBuffer ) ;
	}
	//
	AddMeshNodeToTree( pMeshCol ) ;
	return	true ;
}

// 逆変換行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::GetITransformation
	( S3DCollision::ITransformation& itrans,
			S3DRenderBuffer::Transformation * pTrans )
{
	if ( pTrans != nullptr )
	{
		S3DMatrix	matTrans = pTrans->matTransform ;
		itrans.vPos = pTrans->vTransform ;
		itrans.matRev = matTrans ;
		itrans.matIRev.InverseOf( matTrans ) ;
		itrans.fpScale =
			(float32_t) pow( fabs( itrans.matIRev.Determinant() ), 1.0f / 3.0f ) ;
		//
		double	s[6] ;
		s[0] = itrans.matIRev.GetLine(0).Absolute() ;
		s[1] = itrans.matIRev.GetLine(1).Absolute() ;
		s[2] = itrans.matIRev.GetLine(2).Absolute() ;
		s[3] = itrans.matIRev.GetColumn(0).Absolute() ;
		s[4] = itrans.matIRev.GetColumn(1).Absolute() ;
		s[5] = itrans.matIRev.GetColumn(2).Absolute() ;
		//
		itrans.flagOrthMat =
			(fabs(s[0] - itrans.fpScale) < 0.01)
			&& (fabs(s[1] - itrans.fpScale) < 0.01)
			&& (fabs(s[2] - itrans.fpScale) < 0.01)
			&& (fabs(s[3] - itrans.fpScale) < 0.01)
			&& (fabs(s[4] - itrans.fpScale) < 0.01)
			&& (fabs(s[5] - itrans.fpScale) < 0.01) ;
	}
	else
	{
		itrans.vPos.x = 0 ;
		itrans.vPos.y = 0 ;
		itrans.vPos.z = 0 ;
		itrans.matRev.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
		itrans.matIRev = itrans.matRev ;
		itrans.fpScale = 1.0f ;
		itrans.flagOrthMat = true ;
	}
}

void S3DCollision::GetITransformationInstancing
	( ITransformation& itrans,
		Transformation * pTrans, const S4DMatrix * pmatInstancing )
{
	if ( pmatInstancing == nullptr )
	{
		GetITransformation( itrans, pTrans ) ;
		return ;
	}
	S3DMatrix	matInstance ;
	S3DVector	vInstance ;
	Matrix3x3From4x4<S3DMatrix,S3DVector,float32_t>
				( matInstance, vInstance, *pmatInstancing ) ;
	//
	if ( pTrans != nullptr )
	{
		S3DMatrix	matTrans = pTrans->matTransform ;
		itrans.vPos = matTrans * vInstance + S3DVector(pTrans->vTransform) ;
		itrans.matRev = matTrans * matInstance ;
		itrans.matIRev.InverseOf( itrans.matRev ) ;
		itrans.fpScale =
			(float32_t) pow( fabs( itrans.matIRev.Determinant() ), 1.0f / 3.0f ) ;
	}
	else
	{
		itrans.vPos = vInstance ;
		itrans.matRev = matInstance ;
		itrans.matIRev.InverseOf( matInstance ) ;
		itrans.fpScale =
			(float32_t) pow( fabs( itrans.matIRev.Determinant() ), 1.0f / 3.0f ) ;
	}
	double	s[6] ;
	s[0] = itrans.matIRev.GetLine(0).Absolute() ;
	s[1] = itrans.matIRev.GetLine(1).Absolute() ;
	s[2] = itrans.matIRev.GetLine(2).Absolute() ;
	s[3] = itrans.matIRev.GetColumn(0).Absolute() ;
	s[4] = itrans.matIRev.GetColumn(1).Absolute() ;
	s[5] = itrans.matIRev.GetColumn(2).Absolute() ;
	//
	itrans.flagOrthMat =
		(fabs(s[0] - itrans.fpScale) < 0.01)
		&& (fabs(s[1] - itrans.fpScale) < 0.01)
		&& (fabs(s[2] - itrans.fpScale) < 0.01)
		&& (fabs(s[3] - itrans.fpScale) < 0.01)
		&& (fabs(s[4] - itrans.fpScale) < 0.01)
		&& (fabs(s[5] - itrans.fpScale) < 0.01) ;
}

// 当たり判定情報生成
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::BuildTriangleListCollision
	( S3DCollision::QuadSphereCollision * pqsc,
		S3DCollision::QuadPolygonsCollision * pqpc,
		size_t nQSPackScale, size_t countPolygon,
		const S3DVector4 * pvVertex, const uint32_t * pIndexedList )
{
	size_t	nQPackShift = nQSPackScale + 4 ;
	size_t	nQPackCount = (size_t) 1 << nQPackShift ;
	size_t	nQPackOdd = nQPackCount - 1 ;
	size_t	nQElementCount = (size_t) 1 << (nQSPackScale + 2) ;
	//
	eslFillMemory
		( pqsc, 0,
			((countPolygon + nQPackOdd) >> nQPackShift)
								* sizeof(QuadSphereCollision) ) ;
	eslFillMemory
		( pqpc, 0,
			((countPolygon + 0x03) >> 2) * sizeof(QuadPolygonsCollision) ) ;
	//
	size_t	i ;
	for ( i = 0; i < countPolygon; i ++ )
	{
		QuadPolygonsCollision&	qpc = pqpc[i >> 2] ;
		const size_t	j = i & 0x03 ;
		S3DVector		v0 = pvVertex[pIndexedList[0]] ;	// D
		S3DVector		v1 = pvVertex[pIndexedList[1]] ;	// E
		S3DVector		v2 = pvVertex[pIndexedList[2]] ;	// F
		pIndexedList += 3 ;
		//
		S3DVector	v01 = v0 - v1 ;
		S3DVector	v02 = v0 - v2 ;
		S3DVector	v21 = v2 - v1 ;
		float32_t	absAB = (float32_t) v01.Absolute() ;
		float32_t	absAC = (float32_t) v02.Absolute() ;
		float32_t	absBC = (float32_t) v21.Absolute() ;
		uint32_t	iShift = 0 ;
		//
		if ( absAC > absBC )
		{
			if ( absAB > absAC )
			{
				// △ABC <= △FDE
				iShift = 2 ;
			}
			else
			{
				// △ABC <= △EFD
				iShift = 1 ;
			}
		}
		else if ( absAB > absBC )
		{
			// △ABC <= △FDE
			iShift = 2 ;
		}
		if ( iShift != 0 )
		{
			S3DVector	vTemp1, vTemp2 ;
			if ( iShift == 2 )
			{
				// △ABC <= △FDE
				vTemp1 = v0 ;
				vTemp2 = v1 ;
				v0 = v2 ;
				v1 = vTemp1 ;
				v2 = vTemp2 ;
			}
			else
			{
				// △ABC <= △EFD
				vTemp1 = v0 ;
				v0 = v1 ;
				v1 = v2 ;
				v2 = vTemp1 ;
			}
			v01 = v0 - v1 ;
			v02 = v0 - v2 ;
			v21 = v2 - v1 ;
			absAB = (float32_t) v01.Absolute() ;
			absAC = (float32_t) v02.Absolute() ;
			absBC = (float32_t) v21.Absolute() ;
		}
		//
		qpc.xA[j] = v0.x ;
		qpc.yA[j] = v0.y ;
		qpc.zA[j] = v0.z ;
		qpc.xB[j] = v1.x ;
		qpc.yB[j] = v1.y ;
		qpc.zB[j] = v1.z ;
		qpc.xC[j] = v2.x ;
		qpc.yC[j] = v2.y ;
		qpc.zC[j] = v2.z ;
		//
		qpc.absAB[j] = absAB ;
		qpc.absAC[j] = absAC ;
		qpc.absBC[j] = absBC ;
		//
		if ( (absAB == 0.0f) || (absAC == 0.0f) || (absBC == 0.0f) )
		{
			qpc.xNormal[j] = 0 ;
			qpc.yNormal[j] = 0 ;
			qpc.zNormal[j] = 0 ;
			qpc.cosB[j] = 1.0f ;
			qpc.cosC[j] = 1.0f ;
			qpc.sinB[j] = 0.0f ;
			qpc.sinC[j] = 0.0f ;
		}
		else
		{
			S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
			vNormal.Normalize() ;
			qpc.xNormal[j] = vNormal.x ;
			qpc.yNormal[j] = vNormal.y ;
			qpc.zNormal[j] = vNormal.z ;
			//
			qpc.cosB[j] = (v01.InnerProduct( v21 ) / (absAB * absBC)) ;
			qpc.cosC[j] = (- v02.InnerProduct( v21 ) / (absAC * absBC)) ;
			qpc.sinB[j] = (float32_t) sqrt( esl_fmaxf( 1.0f - qpc.cosB[j] * qpc.cosB[j], 0.0f ) ) ;
			qpc.sinC[j] = (float32_t) sqrt( esl_fmaxf( 1.0f - qpc.cosC[j] * qpc.cosC[j], 0.0f ) ) ;
		}
		//
		qpc.iShiftA[j] = iShift ;
	}
	for ( i = 0; i < countPolygon; i += nQElementCount )
	{
		size_t	nElements = countPolygon - i ;
		if ( nElements > nQElementCount )
		{
			nElements = nQElementCount ;
		}
		QuadPolygonsCollision&	qpcFirst = pqpc[i >> 2] ;
		S3DVector	vMin, vMax ;
		vMin.x = qpcFirst.xA[0] ;
		vMin.y = qpcFirst.yA[0] ;
		vMin.z = qpcFirst.zA[0] ;
		vMax = vMin ;
		//
		size_t	j ;
		for ( size_t p = 0; p < nElements; p ++ )
		{
			QuadPolygonsCollision&	qpc = pqpc[(i + p) >> 2] ;
			j = (i + p) & 0x03 ;
			vMin.x = esl_fminf( vMin.x, qpc.xA[j] ) ;
			vMin.y = esl_fminf( vMin.y, qpc.yA[j] ) ;
			vMin.z = esl_fminf( vMin.z, qpc.zA[j] ) ;
			vMin.x = esl_fminf( vMin.x, qpc.xB[j] ) ;
			vMin.y = esl_fminf( vMin.y, qpc.yB[j] ) ;
			vMin.z = esl_fminf( vMin.z, qpc.zB[j] ) ;
			vMin.x = esl_fminf( vMin.x, qpc.xC[j] ) ;
			vMin.y = esl_fminf( vMin.y, qpc.yC[j] ) ;
			vMin.z = esl_fminf( vMin.z, qpc.zC[j] ) ;
			//
			vMax.x = esl_fmaxf( vMax.x, qpc.xA[j] ) ;
			vMax.y = esl_fmaxf( vMax.y, qpc.yA[j] ) ;
			vMax.z = esl_fmaxf( vMax.z, qpc.zA[j] ) ;
			vMax.x = esl_fmaxf( vMax.x, qpc.xB[j] ) ;
			vMax.y = esl_fmaxf( vMax.y, qpc.yB[j] ) ;
			vMax.z = esl_fmaxf( vMax.z, qpc.zB[j] ) ;
			vMax.x = esl_fmaxf( vMax.x, qpc.xC[j] ) ;
			vMax.y = esl_fmaxf( vMax.y, qpc.yC[j] ) ;
			vMax.z = esl_fmaxf( vMax.z, qpc.zC[j] ) ;
		}
		double	fpRadius = (vMax - vMin).Absolute() * (1.001 / 2) ;
		//
		QuadSphereCollision&	qsc = pqsc[i >> nQPackShift] ;
		j = (i >> (nQSPackScale + 2)) & 0x03 ;
		qsc.xCenter[j] = (float32_t) (vMax.x + vMin.x) * 0.5f ;
		qsc.yCenter[j] = (float32_t) (vMax.y + vMin.y) * 0.5f ;
		qsc.zCenter[j] = (float32_t) (vMax.z + vMin.z) * 0.5f ;
		qsc.fpRadius[j] = (float32_t) fpRadius ;
	}
}

// メッシュ階層スケール計算
//////////////////////////////////////////////////////////////////////////////
size_t S3DCollision::CalculateScaleForQuadSpherePolygons( size_t nPolygons )
{
	double	fpQQPolys = (double) nPolygons / 16.0 ;
	double	fpLog2Poly = log(fpQQPolys) / log(2.0) ;
	return	(size_t) floor( fpLog2Poly * 0.5 ) ;
}

// メッシュツリー・一括構築開始宣言（複数回可）
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::BeginBatchBuild( void )
{
	LockSyncBuffer() ;
	if ( ++ m_countBatchBuild == 1 )
	{
		m_arrMeshNode.RemoveAll() ;
	}
	UnlockSyncBuffer() ;
}

// メッシュツリー・一括構築終了（BeginBatchBuild と同じ回数呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::EndBatchBuild( void )
{
	LockSyncBuffer() ;
	ESLAssert( m_countBatchBuild >= 1 ) ;
	if ( -- m_countBatchBuild == 0 )
	{
		AddNodeToTree( BuildNodeTreeFromArray( m_arrMeshNode ) ) ;
		m_arrMeshNode.RemoveAll() ;
	}
	UnlockSyncBuffer() ;
}

// メッシュツリー最適化（現在のツリーを再構築）
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::OptimizeMeshNodeTree( void )
{
	//
	// ノード列挙
	//
	m_arrMeshNode.SetLimit( m_arrCollision.GetLength() ) ;
	m_arrMeshNode.SetLength( 0 ) ;
	EnumerateMeshNode( m_arrMeshNode, m_pRootNode ) ;
	//
	// ノードを分離
	//
	MeshTreeNode**	ppNodes = m_arrMeshNode.GetArray() ;
	size_t			nNodeCount = m_arrMeshNode.GetLength() ;
	for ( size_t i = 0; i < nNodeCount; i ++ )
	{
		MeshTreeNode*	pNode = ppNodes[i] ;
		ESLAssert( pNode != nullptr ) ;
		pNode->pParent = nullptr ;
		for ( int j = 0; j < MeshTreeNode::countChild; j ++ )
		{
			pNode->pChild[j] = nullptr ;
		}
		CalcNodeSizeAndPosition( pNode ) ;
	}
	m_arrMeshNode.FinishArray() ;
	//
	// ノードを構築
	//
	m_pRootNode = BuildNodeTreeFromArray( m_arrMeshNode ) ;
}

// MeshTreeNode* 配列からノードを構築
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshTreeNode *
	S3DCollision::BuildNodeTreeFromArray
		( SSystem::SPointerArray<MeshTreeNode>& arrNodes )
{
	for ( ; ; )
	{
		//
		// ノードサイズで昇順ソート
		//
		MeshTreeNode**	ppNodes = arrNodes.GetArray() ;
		size_t			nNodeCount = arrNodes.GetLength() ;
		if ( nNodeCount <= 1 )
		{
			if ( nNodeCount >= 1 )
			{
				return	ppNodes[0] ;
			}
			break ;
		}
		SortNodeArray( ppNodes, nNodeCount ) ;
		//
		// ノードの結合
		//
		for ( size_t i = 0; i < nNodeCount; i ++ )
		{
			MeshTreeNode *	pNode0 = ppNodes[i] ;
			if ( pNode0 == nullptr )
			{
				continue ;
			}
			//
			// 近いノードを探して結合する
			//
			MeshTreeNode *	pNearNode = nullptr ;
			size_t			iNear = 0 ;
			double			fpNearLength = 1.0e+10 ;
			for ( size_t j = i + 1; j < nNodeCount; j ++ )
			{
				MeshTreeNode *	pNode = ppNodes[j] ;
				if ( pNode == nullptr )
				{
					continue ;
				}
				double	fpLength = pNode0->fpRadius + pNode->fpRadius ;
				if ( (fpLength > fpNearLength) && (pNearNode != nullptr) )
				{
					break ;
				}
				fpLength += (pNode0->vCenter - pNode->vCenter).Absolute() ;
				if ( (fpLength < fpNearLength) || (pNearNode == nullptr) )
				{
					pNearNode = pNode ;
					iNear = j ;
					fpNearLength = fpLength ;
				}
			}
			if ( pNearNode == nullptr )
			{
				break ;
			}
			ESLAssert( iNear > i ) ;
			//
			// ノード結合
			//
			ppNodes[i] = BindNodeTree( pNode0, pNearNode ) ;
			ppNodes[iNear] = nullptr ;
		}
		arrNodes.FinishArray() ;
		arrNodes.TrimEmpty() ;
	}
	return	nullptr ;
}

// メッシュノード列挙
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::EnumerateMeshNode
	( SSystem::SPointerArray<MeshTreeNode>& arrNodes, MeshTreeNode * pParentNode )
{
	if ( pParentNode != nullptr )
	{
		if ( pParentNode->pMeshCol != nullptr )
		{
			arrNodes.Add( pParentNode ) ;
		}
		for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
		{
			if ( pParentNode->pChild[i] != nullptr )
			{
				EnumerateMeshNode( arrNodes, pParentNode->pChild[i] ) ;
			}
		}
	}
}

// メッシュノードサイズで昇順ソート
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::SortNodeArray( MeshTreeNode** ppNodes, size_t nNodeCount )
{
	if ( nNodeCount < 5 )
	{
		//
		// 選択ソート
		//
		for ( size_t i = 0; i < nNodeCount; i ++ )
		{
			MeshTreeNode *	pNode0 = ppNodes[i] ;
			ESLAssert( pNode0 != nullptr ) ;
			size_t			iMin = i ;
			MeshTreeNode *	pMinNode = pNode0 ;
			for ( size_t j = i + 1; j < nNodeCount; j ++ )
			{
				MeshTreeNode *	pNode = ppNodes[j] ;
				if ( pNode->fpRadius < pMinNode->fpRadius )
				{
					iMin = j ;
					pMinNode = pNode ;
				}
			}
			ppNodes[i] = pMinNode ;
			ppNodes[iMin] = pNode0 ;
		}
		return ;
	}
	//
	// クイックソート
	//
	size_t	iEnd = nNodeCount - 1 ;
	size_t	iPivot = iEnd >> 1 ;
	ESLAssert( ppNodes[iPivot] != nullptr ) ;
	double	fpPivotRadius = ppNodes[iPivot]->fpRadius ;
	for ( size_t i = 0; i < iEnd; i ++ )
	{
		if ( ppNodes[i]->fpRadius > fpPivotRadius )
		{
			while ( iEnd > i )
			{
				if ( ppNodes[iEnd]->fpRadius < fpPivotRadius )
				{
					break ;
				}
				-- iEnd ;
			}
			if ( iEnd <= i )
			{
				break ;
			}
			MeshTreeNode *	pTemp = ppNodes[i] ;
			ppNodes[i] = ppNodes[iEnd] ;
			ppNodes[iEnd] = pTemp ;
		}
	}
	if ( iEnd >= 2 )
	{
		SortNodeArray( ppNodes, iEnd ) ;
	}
	if ( nNodeCount - (iEnd + 1) >= 2 )
	{
		SortNodeArray( ppNodes + iEnd + 1, nNodeCount - (iEnd + 1) ) ;
	}
}

// ノードを結合する
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshTreeNode *
	S3DCollision::BindNodeTree
		( S3DCollision::MeshTreeNode * pNode0,
				S3DCollision::MeshTreeNode * pNode1 )
{
	for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
	{
		if ( pNode1->pChild[i] == nullptr )
		{
			pNode0->pParent = pNode1 ;
			pNode1->pChild[i] = pNode0 ;
			CalcNodeSizeAndPosition( pNode1 ) ;
			return	pNode1 ;
		}
		if ( pNode0->pChild[i] == nullptr )
		{
			pNode1->pParent = pNode0 ;
			pNode0->pChild[i] = pNode1 ;
			CalcNodeSizeAndPosition( pNode0 ) ;
			return	pNode0 ;
		}
	}
	MeshTreeNode *	pmtnBridge =
		(MeshTreeNode*) m_bufRender.Allocate( sizeof(MeshTreeNode) ) ;
	pmtnBridge->pParent = nullptr ;
	pmtnBridge->pMeshCol = nullptr ;
	pmtnBridge->pChild[0] = pNode0 ;
	pmtnBridge->pChild[1] = pNode1 ;
	pNode0->pParent = pmtnBridge ;
	pNode1->pParent = pmtnBridge ;
	CalcNodeSizeAndPosition( pmtnBridge ) ;
	return	pmtnBridge ;
}

// メッシュツリーにノードを追加
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::AddMeshNodeToTree( S3DCollision::MeshCollision * pMeshCol )
{
	S3DDVector	vMeshCenter = pMeshCol->vCenter ;
	double		fpMeshRadius = pMeshCol->fpRadius ;
	ESLAssert( fpMeshRadius == fpMeshRadius /* not NaN */ ) ;
	if ( m_pTransformation != nullptr )
	{
		vMeshCenter = m_pTransformation->matTransform * vMeshCenter
										+ m_pTransformation->vTransform ;
		//
		S3DDVector	vRadius( fpMeshRadius, fpMeshRadius, fpMeshRadius ) ;
		m_pTransformation->matTransform.RevolveVector( vRadius ) ;
		fpMeshRadius = vRadius.Absolute() ;
	}
	//
	MeshTreeNode *	pmtnMesh =
		(MeshTreeNode*) m_bufRender.Allocate( sizeof(MeshTreeNode) ) ;
	pmtnMesh->vCenter = vMeshCenter ;
	pmtnMesh->fpRadius = fpMeshRadius ;
	pmtnMesh->pMeshCol = pMeshCol ;
	pmtnMesh->pParent = nullptr ;
	for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
	{
		pmtnMesh->pChild[i] = nullptr ;
	}
	pMeshCol->pParentNode = pmtnMesh ;
	//
	ESLVerify( m_csBufSync.TestLocked() ) ;
	if ( m_countBatchBuild <= 0 )
	{
		AddNodeToTree( pmtnMesh ) ;
	}
	else
	{
		CalcNodeSizeAndPosition( pmtnMesh ) ;
		m_arrMeshNode.Add( pmtnMesh ) ;
	}
}

void S3DCollision::AddNodeToTree( S3DCollision::MeshTreeNode * pAddNode )
{
	if ( pAddNode == nullptr )
	{
		return ;
	}
	ESLAssert( pAddNode->pParent == nullptr ) ;

	MeshTreeNode *	pNode = m_pRootNode ;
	if ( pNode == nullptr )
	{
		//
		// ルートノード
		//
		m_pRootNode = pAddNode ;
		return ;
	}
	//
	// 追加するノードを探索する
	//
	for ( ; ; )
	{
		MeshTreeNode *	pNextNode = nullptr ;
		double	fpNearest = 1.0e8 ;
		int		iNearest = -1 ;
		for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
		{
			MeshTreeNode *	pChild = pNode->pChild[i] ;
			if ( pChild == nullptr )
			{
				pNode->pChild[i] = pAddNode ;
				pAddNode->pParent = pNode ;
				UpdateNodeForTree( pAddNode ) ;
				return ;
			}
			ESLAssert( pChild->pParent == pNode ) ;
			//
			// 交差判定
			//
			S3DDVector	vDelta = pChild->vCenter ;
			double		r ;
			vDelta -= pAddNode->vCenter ;
			r = vDelta.Absolute() ;
			//
			if ( pChild->fpRadius + r < pAddNode->fpRadius )
			{
				//
				// メッシュがノードを含んでいる
				//
				InsertNodeForTree( pNode, i, pAddNode ) ;
				UpdateNodeForTree( pAddNode ) ;
				return ;
			}
			else if ( (iNearest < 0) || (fpNearest > r) )
			{
				// 最近ノード更新
				fpNearest = r ;
				iNearest = i ;
				//
				if ( pAddNode->fpRadius + r < pChild->fpRadius )
				{
					// ノードがメッシュを含んでいる
					pNextNode = pChild ;
				}
				else
				{
					pNextNode = nullptr ;
				}
			}
		}
		if ( pNextNode == nullptr )
		{
			ESLAssert( pNode->pChild[1] != nullptr ) ;
			ESLAssert( iNearest >= 0 ) ;
			size_t	iAnother = iNearest ^ 1 ;
			if ( pNode->fpRadius < fpNearest )
			{
				//
				// 距離が遠い場合には新規ノードを挿入する
				//
				BridgeNodeForTree( pNode, pAddNode ) ;
				UpdateNodeForTree( pAddNode ) ;
				return ;
			}
			else if ( pNode->pChild[iNearest]->fpRadius < pAddNode->fpRadius )
			{
				//
				// ノードが小さい場合には子ノードにする
				//
				InsertNodeForTree( pNode, iNearest, pAddNode ) ;
				UpdateNodeForTree( pAddNode ) ;
				return ;
			}
			pNextNode = pNode->pChild[iNearest] ;
		}
		pNode = pNextNode ;
	}
}

// 子ノードとしてノードを挿入する
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::InsertNodeForTree
	( MeshTreeNode * pNode, size_t iChild, MeshTreeNode * pAddNode )
{
	ESLAssert( iChild < MeshTreeNode::countChild ) ;
	MeshTreeNode *	pChild = pNode->pChild[iChild] ;
	if ( pChild == nullptr )
	{
		// 空いてるのでそのまま挿入
		pNode->pChild[iChild] = pAddNode ;
		pAddNode->pParent = pNode ;
		return ;
	}
	for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
	{
		if ( pAddNode->pChild[i] == nullptr )
		{
			// pChild を pAddNode の子ノードにして挿入する
			pAddNode->pChild[i] = pChild ;
			pChild->pParent = pAddNode ;
			//
			pNode->pChild[iChild] = pAddNode ;
			pAddNode->pParent = pNode ;
			return ;
		}
	}
	// 空きがないので新規ノード作成
	BridgeNodeForTree( pChild, pAddNode ) ;
}

// pNode0 ノード位置に新規ノードを作成して pNode0, pNode1 の子を持つノードにする
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshTreeNode *
	S3DCollision::BridgeNodeForTree
		( S3DCollision::MeshTreeNode * pNode0,
			S3DCollision::MeshTreeNode * pNode1 )
{
	MeshTreeNode *	pmtnBridge =
		(MeshTreeNode*) m_bufRender.Allocate( sizeof(MeshTreeNode) ) ;
	MeshTreeNode *	pParent = pNode0->pParent ;
	pmtnBridge->pParent = pParent ;
	pmtnBridge->pMeshCol = nullptr ;
	pNode0->pParent = pmtnBridge ;
	if ( pParent != nullptr )
	{
		ESLAssert( MeshTreeNode::countChild == 2 ) ;
		if ( pParent->pChild[0] == pNode0 )
		{
			pParent->pChild[0] = pmtnBridge ;
		}
		else
		{
			ESLAssert( pParent->pChild[1] == pNode0 ) ;
			pParent->pChild[1] = pmtnBridge ;
		}
	}
	else
	{
		m_pRootNode = pmtnBridge ;
	}
	pmtnBridge->pChild[0] = pNode0 ;
	pmtnBridge->pChild[1] = pNode1 ;
	pNode1->pParent = pmtnBridge ;
	return	pmtnBridge ;
}

// メッシュのサイズ更新に伴うノード情報の更新
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::UpdateNodeForTree( MeshTreeNode * pNode ) const
{
	ESLAssert( pNode != nullptr ) ;
	do
	{
		//
		// ノードのサイズ更新
		//
		CalcNodeSizeAndPosition( pNode ) ;
		//
		ESLAssert( (pNode->pParent != nullptr) || (pNode == m_pRootNode) ) ;
		pNode = pNode->pParent ;
	}
	while ( pNode != nullptr ) ;
}

void S3DCollision::UpdateMeshNodeForTree( S3DCollision::MeshCollision * pMeshCol ) const
{
	MeshTreeNode *	pNode = pMeshCol->pParentNode ;
	ESLAssert( pNode != nullptr ) ;
	ESLAssert( pNode->pMeshCol == pMeshCol ) ;
	UpdateNodeForTree( pNode ) ;
}

// ノード中心座標・半径の計算
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::CalcNodeSizeAndPosition( MeshTreeNode * pNode )
{
	ESLAssert( pNode != nullptr ) ;
	S3DDVector	vCenter( 0, 0, 0 ) ;
	double		fpRadius = 0 ;
	bool		fNodeSize = false ;
	//
	MeshCollision *	pMeshCol = pNode->pMeshCol ;
	if ( pMeshCol != nullptr )
	{
		S3DVector	vColSize
			( pMeshCol->fpRadius,
				pMeshCol->fpRadius, pMeshCol->fpRadius ) ;
		pMeshCol->itrans.matRev.RevolveVector( vColSize ) ;
		//
		vCenter = pMeshCol->itrans.matRev * pMeshCol->vCenter ;
		vCenter += pMeshCol->itrans.vPos ;
		fpRadius = vColSize.Absolute() ;
		fNodeSize = true ;
	}
	for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
	{
		MeshTreeNode *	pChild = pNode->pChild[i] ;
		if ( pChild != nullptr )
		{
			if ( fNodeSize )
			{
				S3DDVector	vDelta = pChild->vCenter ;
				vDelta -= vCenter ;
				//
				double	r = vDelta.Absolute() ;
				double	rMin = esl_fmin( - fpRadius, r - pChild->fpRadius ) ;
				double	rMax = esl_fmax( fpRadius, r + pChild->fpRadius ) ;
				double	c = (rMin + rMax) * 0.5 ;
				//
				vDelta.Normalize() ;
				vCenter += vDelta * c ;
				fpRadius = (rMax - rMin) * (0.5 * 1.0001) ;
			}
			else
			{
				vCenter = pChild->vCenter ;
				fpRadius = pChild->fpRadius ;
				fNodeSize = true ;
			}
		}
	}
	pNode->vCenter = vCenter ;
	pNode->fpRadius = fpRadius ;
}

// ノード取得（MeshCollision.iMesh の番号）
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshCollision *
	S3DCollision::GetMeshCollisionAt( size_t iMesh ) const
{
	size_t *	pIndex = m_arrCollisionIndex.GetAt( iMesh ) ;
	if ( pIndex != nullptr )
	{
		return	m_arrCollision.GetAt( *pIndex ) ;
	}
	return	nullptr ;
}

// 全ノードを列挙
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::EnumerateAllMeshCollision
	( SSystem::SPointerArray<MeshCollision>& aMeshColls,
		uint64_t maskExcClasses, uint64_t maskIncClasses ) const
{
	if ( m_pRootNode != nullptr )
	{
		EnumerateMeshCollisionOfNode
			( aMeshColls, m_pRootNode, maskExcClasses, maskIncClasses ) ;
	}
}

void S3DCollision::EnumerateMeshCollisionOfNode
	( SSystem::SPointerArray<MeshCollision>& aMeshColls,
		const S3DCollision::MeshTreeNode * pNode,
		uint64_t maskExcClasses, uint64_t maskIncClasses ) const
{
	MeshCollision *	pMeshCol = pNode->pMeshCol ;
	if ( (pMeshCol != nullptr)
		&& (pMeshCol->maskClasses & maskIncClasses)
		&& !(pMeshCol->maskClasses & maskExcClasses) )
	{
		aMeshColls.Add( pMeshCol ) ;
	}
	MeshTreeNode *	pChild0 = pNode->pChild[0] ;
	if ( pChild0 == nullptr )
	{
		return ;
	}
	EnumerateMeshCollisionOfNode
		( aMeshColls, pChild0, maskExcClasses, maskIncClasses ) ;
	//
	MeshTreeNode *	pChild1 = pNode->pChild[1] ;
	if ( pChild1 == nullptr )
	{
		return ;
	}
	EnumerateMeshCollisionOfNode
		( aMeshColls, pChild1, maskExcClasses, maskIncClasses ) ;
}

// ノードの正常性テスト
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::VerifyMeshTree( void ) const
{
	if ( m_pRootNode == nullptr )
	{
		return	true ;
	}
	return	VerifyMeshNode( m_pRootNode ) ;
}

bool S3DCollision::VerifyMeshNode( S3DCollision::MeshTreeNode * pNode ) const
{
	bool	flagValid = true ;
	if ( pNode->pMeshCol != nullptr )
	{
		MeshCollision *	pMeshCol = pNode->pMeshCol ;
		S3DDVector		vMeshCenter =
			S3DDVector(pMeshCol->itrans.matRev
							* pMeshCol->vCenter) + pMeshCol->itrans.vPos ;
		if ( (vMeshCenter - pNode->vCenter).Absolute()
				+ pMeshCol->fpRadius
					/ pMeshCol->itrans.fpScale > pNode->fpRadius * 1.001 )
		{
			return	false ;
		}
	}
	for ( int i = 0; i < MeshTreeNode::countChild; i ++ )
	{
		if ( pNode->pChild[i] == nullptr )
		{
			continue ;
		}
		S3DCollision::MeshTreeNode *	pChild = pNode->pChild[i] ;
		if ( (pChild->vCenter - pNode->vCenter).Absolute()
							+ pChild->fpRadius > pNode->fpRadius * 1.001 )
		{
			return	false ;
		}
		if ( !VerifyMeshNode( pChild ) )
		{
			return	false ;
		}
	}
	return	flagValid ;
}



//////////////////////////////////////////////////////////////////////////////
// 単独メッシュ当たり判定オブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshObject::MeshObject( void )
{
	pSphereCol = nullptr ;
	pCollision = nullptr ;
	nPolygons = 0 ;
	m_pBuffer = nullptr ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCollision::MeshObject::~MeshObject( void )
{
	if ( m_pBuffer != nullptr )
	{
		esl_free( m_pBuffer ) ;
	}
}

// 当たり判定メッシュ構築
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::MeshObject::CreateMesh
	( size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const uint32_t * pIndexedList )
{
	//
	// 初期化
	//
	typeCol = typeTriangleList ;
	itrans.vPos = S3DDVector( 0, 0, 0 ) ;
	itrans.matIRev.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	pParentNode = nullptr ;
	nQSPackScale = 0 ;
	pSphereCol = nullptr ;
	pCollision = nullptr ;
	pCollider = nullptr ;
	iMesh = 0 ;
	preMesh = nullptr ;
	nPolygons = countPolygon ;
	pUserData = nullptr ;
	//
	// バッファ確保
	//
	nQSPackScale = CalculateScaleForQuadSpherePolygons( countPolygon ) ;
	size_t	nQPackShift = nQSPackScale + 4 ;
	size_t	nQPackCount = (size_t) 1 << nQPackShift ;
	size_t	nQPackOdd = nQPackCount - 1 ;
	// 
	size_t	nSpheres = (countPolygon + nQPackOdd) >> nQPackShift ;
	size_t	nPolygons = (countPolygon + 0x03) >> 2 ;
	ESLAssert( m_pBuffer == nullptr ) ;
	m_pBuffer = esl_malloc
		( nSpheres * sizeof(QuadSphereCollision)
			+ nPolygons * sizeof(QuadPolygonsCollision) + 0x10 ) ;
	//
	ulong_ptr_t	ulpBuf = (((ulong_ptr_t) m_pBuffer) + 0x0F) & ~0x0F ;
	pSphereCol = (QuadSphereCollision*) ulpBuf ;
	pCollision =
		(QuadPolygonsCollision*)
			(ulpBuf + nSpheres * sizeof(QuadSphereCollision)) ;
	//
	// 頂点構築
	//
	BuildTriangleListCollision
		( pSphereCol, pCollision,
			nQSPackScale, countPolygon, pvVertex, pIndexedList ) ;
	//
	// すべての頂点を含む球を計算
	//
	S3DVector	vMin, vMax ;
	MinMaxRangeOfMesh( vMin, vMax, pvVertex, countVertex ) ;
	//
	vCenter = (vMin + vMax) * 0.5f ;
	fpRadius = (float32_t) (vMax - vMin).Absolute() * (1.001f * 0.5f) ;
	//
	return	sglErrSuccess ;
}

// 当たり判定メッシュ更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCollision::MeshObject::UpdateMesh
	( size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const uint32_t * pIndexedList )
{
	if ( nPolygons != countPolygon )
	{
		return	sglErrFailed ;
	}
	ESLAssert( pSphereCol != nullptr ) ;
	ESLAssert( pCollision != nullptr ) ;
	//
	// 頂点構築
	//
	BuildTriangleListCollision
		( pSphereCol, pCollision,
			nQSPackScale, countPolygon, pvVertex, pIndexedList ) ;
	//
	// すべての頂点を含む球を計算
	//
	S3DVector	vMin, vMax ;
	MinMaxRangeOfMesh( vMin, vMax, pvVertex, countVertex ) ;
	//
	vCenter = (vMin + vMax) * 0.5f ;
	fpRadius = (float32_t) (vMax - vMin).Absolute() * (1.001f * 0.5f) ;
	//
	return	sglErrSuccess ;
}

// 当たり判定メッシュ用バッファ解放
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::MeshObject::Release( void )
{
	if ( m_pBuffer != nullptr )
	{
		esl_free( m_pBuffer ) ;
		m_pBuffer = nullptr ;
	}
	pSphereCol = nullptr ;
	pCollision = nullptr ;
	nPolygons = 0 ;
}

// 範囲取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::MeshObject::GetCollisionRange
	( S3DDVector& vOutCenter, double& fpOutRadius ) const
{
	vOutCenter = vCenter ;
	fpOutRadius = fpRadius ;
	return	true ;
}

// 凸形状の内側判定
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::MeshObject::IsSphereInclusive
	( const S3DDVector& vPos,
		float fpInRadius, S3DCollisionResult& rsIncluded ) const
{
	return	IsSphereIncludedMesh( this, vPos, fpInRadius, rsIncluded ) ;
}

// 球との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::MeshObject::IsHitAgainstSphere
	( const S3DDVector& vPos,
		float fpInRadius, S3DCollisionResult& rsHit ) const
{
	S3DVector	vsPos = vPos ;
	return	DoesMeshHitAgainstSphere( this, vsPos, fpInRadius, rsHit ) ;
}

// 線分との交差判定（メッシュ内）
//////////////////////////////////////////////////////////////////////////////
bool S3DCollision::MeshObject::IsSegmentCrossing
	( const S3DDVector& vPos0, const S3DDVector& vPos1,
		float fpErrorGap, S3DCollisionResult& rsCross ) const
{
	S3DVector	vsPos0 = vPos0 ;
	S3DVector	vsPos1 = vPos1 ;
	return	IsSegmentCrossingMesh( this, vsPos0, vsPos1, fpErrorGap, rsCross ) ;
}

// メッシュ頂点の最大値・最小値取得
//////////////////////////////////////////////////////////////////////////////
void S3DCollision::MinMaxRangeOfMesh
	( S3DVector& vMin, S3DVector& vMax,
		const S3DVector4 * pvVertex, size_t countVertex )
{
	vMin.x = 0 ;
	vMin.y = 0 ;
	vMin.z = 0 ;
	vMax.x = 0 ;
	vMax.y = 0 ;
	vMax.z = 0 ;
	//
	if ( countVertex == 0 )
	{
		return ;
	}
	//
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE )
	{
		ESLAssert( (g_cpuFamily == cpuFamily_X86) || (g_cpuFamily == cpuFamily_X86_64) ) ;
		float32_t	fpMax[4], fpMin[4] ;
		//
		__m128	xmmMax = _mm_loadu_ps( (float*) pvVertex ) ;
		__m128	xmmMin = xmmMax ;
		//
		for ( size_t i = 1; i < countVertex; i ++ )
		{
			__m128	xmmNext = _mm_loadu_ps( (float*) (pvVertex + i) ) ;
			xmmMax =_mm_max_ps( xmmMax, xmmNext ) ;
			xmmMin =_mm_min_ps( xmmMin, xmmNext ) ;
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
		ESLAssert( (g_cpuFamily == cpuFamily_ARM) || (g_cpuFamily == cpuFamily_ARM64) ) ;
		//
		S3DVector4	vMaxMin[2] ;
		//
		ERISA_sclfMoveVertexAndCubeRange_ARM_NEON
			( &vMaxMin[0], (S3DVector4*) pvVertex, pvVertex, countVertex ) ;
		//
		vMax = vMaxMin[0] ;
		vMin = vMaxMin[1] ;
	}
	else
	{
	#endif
		float32_t	xMax = pvVertex->x ;
		float32_t	yMax = pvVertex->y ;
		float32_t	zMax = pvVertex->z ;
		float32_t	xMin = xMax ;
		float32_t	yMin = yMax ;
		float32_t	zMin = zMax ;
		//
		pvVertex ++ ;
		//
		for ( size_t i = 1; i < countVertex; i ++ )
		{
			xMax = esl_fmaxf( xMax, pvVertex->x ) ;
			yMax = esl_fmaxf( yMax, pvVertex->y ) ;
			zMax = esl_fmaxf( zMax, pvVertex->z ) ;
			xMin = esl_fminf( xMin, pvVertex->x ) ;
			yMin = esl_fminf( yMin, pvVertex->y ) ;
			zMin = esl_fminf( zMin, pvVertex->z ) ;
			pvVertex ++ ;
		}
		//
		vMax.x = xMax ;
		vMax.y = yMax ;
		vMax.z = zMax ;
		vMin.x = xMin ;
		vMin.y = yMin ;
		vMin.z = zMin ;
	#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
	}
	#elif	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	}
	#endif
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算基底オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPhysicsScene::Object, SGLObject )



//////////////////////////////////////////////////////////////////////////////
// 物理演算アクター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPhysicsScene::Actor, Object )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor::Actor( ESLObject * pOwner )
	: m_pOwnerObj( pOwner ),
		m_pMaterial( &S3DPhysicsScene::m_materialDefault ),
		m_vPos( 0, 0, 0 ), m_vLastPos( 0, 0, 0 ),
		m_vSpeed( 0, 0, 0 ), m_vLastSpeed( 0, 0, 0 ),
		m_qPosture( 1, 0, 0, 0 ), m_qLastPosture( 1, 0, 0, 0 ),
		m_vRotate( 1, 0, 0 ), m_fpRotSpeed( 0.0f ),
		m_vZoom( 1, 1, 1 ),
		m_pHitPoints( nullptr ), m_nHitPointCount( 0 )
{
}

S3DPhysicsScene::Actor::Actor( const S3DPhysicsScene::Actor& act )
	: m_pOwnerObj( act.m_pOwnerObj ),
		m_pMaterial( act.m_pMaterial ),
		m_vPos( act.m_vPos ), m_vLastPos( act.m_vLastPos ),
		m_vSpeed( act.m_vSpeed ), m_vLastSpeed( act.m_vLastSpeed ),
		m_qPosture( act.m_qPosture ), m_qLastPosture( act.m_qLastPosture ),
		m_vRotate( act.m_vRotate ), m_fpRotSpeed( act.m_fpRotSpeed ),
		m_vZoom( act.m_vZoom ),
		m_pHitPoints( act.m_pHitPoints ), m_nHitPointCount( act.m_nHitPointCount )
{
}

// （物理演算以外の）イベント当たり判定ハンドラ (false の場合衝突処理はしない）
//////////////////////////////////////////////////////////////////////////////
bool S3DPhysicsScene::Actor::OnHitEventCollider
	( const S3DCollisionResult& rsHit, Object * pHit,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	return	rsHit.TestUserColliderMask( pMesh, S3DCollision::colliderPhysItem ) ;
}

// 衝突時ハンドラ
//////////////////////////////////////////////////////////////////////////////
bool S3DPhysicsScene::Actor::OnCollidedWith
	( S3DPhysicsScene::Object * pObj,
		const S3DDVector& vImpactPos,
		const S3DVector& vActNormal, float32_t secAdv )
{
	return	false ;
}

// 動的環境
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::DynamicEnvironment( S3DPhysicsScene::Environment& env )
{
}

// 慣性運動／接地先への追従
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::KineticMove
	( const S3DPhysicsScene::Environment& env, float32_t secAdv )
{
	const Material *	pMaterial = m_pMaterial ;
	ESLAssert( pMaterial != nullptr ) ;
	if ( pMaterial == nullptr )
	{
		return ;
	}
	//
	// 環境変数更新
	//
	S3DPhysicsScene::Environment	envTemp = env ;
	DynamicEnvironment( envTemp ) ;
	//
	m_vLastPos = m_vPos ;
	m_vLastSpeed = m_vSpeed ;
	m_qLastPosture = m_qPosture ;
	//
	m_vSpeed += envTemp.m_vGravity * secAdv ;
	//
	S3DVector	vSpeed = m_vSpeed ;
	vSpeed -= envTemp.m_vStream ;
	//
	// 媒質比重（減速率）計算
	//
	float32_t	fpMediumWeight = pMaterial->fpVolume * envTemp.m_fpDensity ;
	float32_t	fpSlowSpeed = 1.0f ;
	if ( (fpMediumWeight + pMaterial->fpWeight) > 0.0f )
	{
		fpSlowSpeed = pMaterial->fpWeight / (fpMediumWeight + pMaterial->fpWeight) ;
	}
	float32_t	f = (float32_t) pow( fpSlowSpeed, secAdv ) ;
	vSpeed *= f ;
	vSpeed += envTemp.m_vStream ;
	//
	m_fpRotSpeed *= f ;
	//
	// 移動処理
	//
	m_vSpeed = vSpeed ;
	if ( !(pMaterial->nFlags & actorNoMove) )
	{
		m_vPos += S3DDVector(vSpeed) * secAdv ;
	}
	//
	if ( !(pMaterial->nFlags & actorNoRotation) )
	{
		// 回転
		S3DQuaternion	qRot ;
		qRot.SetRodriguesRotaion( m_vRotate, m_fpRotSpeed * secAdv ) ;
		m_qPosture = qRot * m_qPosture ;
		m_qPosture.Normalize() ;
	}
}

// 衝突計算
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::Collide
	( const S3DDVector& vImpactPos, const S3DVector& vActNormal, float32_t secAdv )
{
	S3DVector	vSpeed( 0, 0, 0 ) ;
	CollideWith( vImpactPos, vActNormal, secAdv, vSpeed, 1.0f ) ;
}

void S3DPhysicsScene::Actor::CollideWith
	( const S3DDVector& vImpactPos,
		const S3DVector& vActNormal, float32_t secAdv,
		const S3DVector& vActSpeed, float32_t fpActElasticity )
{
	ESLAssert( fabs(vActNormal.Absolute() - 1.0) < 1.0e-5 ) ;
	//
	const Material *	pMaterial = m_pMaterial ;
	ESLAssert( pMaterial != nullptr ) ;
	if ( pMaterial == nullptr )
	{
		return ;
	}
	if ( pMaterial->nFlags & actorNoRotation )
	{
		//
		// 回転無し衝突
		//
		S3DVector	vBaseSpeed = m_vSpeed - vActSpeed ;
		float32_t	fpElasticity = pMaterial->fpElasticity * fpActElasticity ;
		float32_t	fpVSpeed = vBaseSpeed.InnerProduct( vActNormal ) ;
		float32_t	fpImpactPower = - fpVSpeed * pMaterial->fpWeight ;
#if	1
		float32_t	fpResistancePower =
						esl_fminf( fpImpactPower / pMaterial->fpStillFriction, 1.0f )
														* pMaterial->fpResistance ;
		float32_t	fpSlipResistance = 1.0f - esl_fclampf( fpResistancePower, 0.0f, 1.0f ) ;
#else
		float32_t	fpResistancePower = fpImpactPower * pMaterial->fpResistance ;
		float32_t	fpSlipResistance =
						(float32_t) pow( fpElasticity,
											pMaterial->fpResistance
												* fpResistancePower ) ;
		fpSlipResistance = (0.1f + fpSlipResistance * 0.9f) ;
#endif
		//
		if ( fpVSpeed <= 0.0f )
		{
			S3DVector	vRefSpeed = vBaseSpeed - vActNormal * fpVSpeed ;
			vRefSpeed *= fpSlipResistance ;
			vRefSpeed -= vActNormal * (fpVSpeed * fpElasticity) ;
			m_vSpeed = vRefSpeed ;
		}
	}
	else
	{
		//
		// 衝突点の回転ベクトル
		//		vImpactPos0			: 重心点からの衝突点
		//		vImpactDir			: 重心点から衝突点方向
		//		fpImpactPosLen		: 重心点から衝突点までの距離
		//		fpRotateWeight		: 回転による実質質量
		//		vRotateSpeed0		: 衝突点の回転速度
		//		vRotatePower0		: 衝突点の回転速度×質量
		//
		S3DVector	vImpactPos0 = vImpactPos - m_vPos ;
		float32_t	fpImpactPosLen = (float32_t) vImpactPos0.Absolute() ;
		float32_t	fpRcpImpactPosLen =
						(fpImpactPosLen < 1.0e-5) ? 1.0f : (1.0f / fpImpactPosLen) ;
		float32_t	fpRotateWeight = esl_fminf( pMaterial->fpRotWeight, 1.0f ) ;
		float32_t	fpRcpRotateWeight = esl_fminf( 1.0f / pMaterial->fpRotWeight, 1.0f ) ;
		S3DVector	vImpactDir = vImpactPos0 * fpRcpImpactPosLen ;
		S3DMatrix	matRotate ;
		double		rad = PI * (1.0 / 180.0) ;
		matRotate.RotationOnVectorOf( m_vRotate, sin(rad), cos(rad) ) ;
		S3DVector	vRotateSpeed0 =
						(matRotate * vImpactPos0 - vImpactPos0)
											* (m_fpRotSpeed * 180.0 / PI) ;
		S3DVector	vRotatePower0 =
						vRotateSpeed0 * (pMaterial->fpWeight * fpRotateWeight) ;
#if	1
		//
		// 衝突力
		//		vDeltaSpeed			: 衝突点の速度（回転含む）
		//		vPower0				: 速度×質量（回転含む）
		//		fpElasticity		: 反発係数（加速率）
		//		fpImpactSpeed		: 衝突法線方向への速度（回転含む）
		//		vImpactSpeed		: 法線方向への衝突速度（ベクトル）
		//		vCrossSpeed			: 衝突面に平行な方向への速度
		//		fpImpactPower		: 衝突法線方向への速度×質量（絶対値）
		//		fpResistancePower	: 摩擦抵抗力
		//		fpSlipResistance	: 衝突面に平行な方向へ影響する摩擦係数（減速率）
		//
		S3DVector	vBaseSpeed = m_vSpeed - vActSpeed ;
		S3DVector	vDeltaSpeed = vBaseSpeed + vRotateSpeed0 ;
		S3DVector	vPower0 = vBaseSpeed * pMaterial->fpWeight + vRotatePower0 ;
		float32_t	fpElasticity = pMaterial->fpElasticity * fpActElasticity ;
		float32_t	fpImpactSpeed = vDeltaSpeed.InnerProduct( vActNormal ) ;
		if ( fpImpactSpeed >= 0.0f )
		{
			// 離脱コースなので衝突無し
			return ;
		}
		S3DVector	vImpactSpeed = vActNormal * fpImpactSpeed ;
		S3DVector	vCrossSpeed = vDeltaSpeed - vImpactSpeed ;
		float32_t	fpImpactPower = - vPower0.InnerProduct( vActNormal ) ;
		float32_t	fpResistancePower =
						esl_fminf( fpImpactPower / pMaterial->fpStillFriction, 1.0f )
														* pMaterial->fpResistance ;
		float32_t	fpSlipResistance = esl_fclampf( fpResistancePower, 0.0f, 1.0f ) ;
		//
		// 運動反射
		//		fpBaseVSpeed		: 法線方向の運動速度
		//		vBaseImpactSpeed	: 法線方向の運動速度
		//		vBaseCrossSpeed		: 衝突面に平行な方向への運動速度
		//
		S3DVector	vLeftPower = vPower0 * fpElasticity ;
		float32_t	fpBaseVSpeed = vBaseSpeed.InnerProduct( vActNormal ) ;
		float32_t	fpRotVSpeed = vRotateSpeed0.InnerProduct( vActNormal ) ;
		S3DVector	vRotHSpeed = vRotateSpeed0 - vActNormal * fpRotVSpeed ;
		S3DVector	vBaseImpactSpeed = vActNormal * fpBaseVSpeed ;
		S3DVector	vBaseCrossSpeed = vBaseSpeed - vBaseImpactSpeed ;
		S3DVector	vRotateSpeed = vRotateSpeed0 ;
		if ( fpBaseVSpeed < 0.0f )
		{
			// 運動速度反映
			float32_t	fpCrossRotElasticity = fpSlipResistance * fpRcpRotateWeight ;
			float32_t	fpBaseVRefSpeed = fpBaseVSpeed * fpElasticity ;
			m_vSpeed = vBaseCrossSpeed * (1.0f - fpSlipResistance)
						+ vActNormal * (-fpBaseVRefSpeed * fpRotateWeight) + vActSpeed ;
			vRotateSpeed -= vBaseCrossSpeed * fpCrossRotElasticity
							+ vRotHSpeed
							+ vActNormal * (fpBaseVRefSpeed * fpRcpRotateWeight) ;
			vLeftPower -= vActNormal * fpBaseVRefSpeed ;
		}
		float32_t	fpImpactPower0 = vLeftPower.InnerProduct( vActNormal ) ;
		if ( fpImpactPower0 < 0.0f )
		{
			// 運動量と回転運動量の比率
			//		cosImpact	: 衝突点－重心点・速度の成す角の余弦＝運動速度影響度（>0）
			S3DVector	vBaseSpeedDir = vBaseSpeed ;
			vBaseSpeedDir.Normalize() ;
			//
			float32_t	cosImpact =
					(float32_t) fabs( vImpactDir.InnerProduct( vBaseSpeedDir ) ) ;
			float32_t	sinImpact =
					(float32_t) sqrt( esl_fmaxf( 1.0f - cosImpact * cosImpact, 0.0f ) ) ;
			//
			vRotateSpeed += vActNormal * (-fpImpactPower0 * sinImpact * fpRcpRotateWeight) ;
		}
		//
		// 回転の水平方向影響
		//
		float32_t	fpImpactPower1 = vLeftPower.InnerProduct( vActNormal ) ;
		S3DVector	vHorzPower = vLeftPower - vActNormal * fpImpactPower1 ;
		m_vSpeed -= vCrossSpeed * fpSlipResistance ;
		//
		// 回転合成
		//
		float32_t	cosRotGimbal =
					(float32_t) fabs( m_vRotate.InnerProduct( vImpactDir ) ) ;
		m_fpRotSpeed *= cosRotGimbal * fpElasticity ;
		//
		S3DVector	vRotate ;
		float32_t	fpRotSpeed ;
		CalcRotation
			( vRotate, fpRotSpeed,
				vImpactPos0, vRotateSpeed, fpImpactPosLen ) ;
		//
		AddRotate( vRotate, fpRotSpeed ) ;
#else
		//
		// 衝突力
		//		vDeltaSpeed			: 衝突点の速度（回転含む）
		//		vPower0				: 速度×質量（回転含む）
		//		fpElasticity		: 反発係数（加速率）
		//		fpImpactSpeed		: 衝突法線方向への速度（回転含む）
		//		fpAbsImpactSpeed	: 衝突法線方向への速度（絶対値）
		//		vImpactSpeed		: 法線方向への衝突速度（ベクトル）
		//		vCrossSpeed			: 衝突面に平行な方向への速度
		//		fpImpactPower		: 衝突法線方向への速度×質量（絶対値）
		//		fpResistancePower	: 摩擦抵抗力
		//		fpSlipResistance	: 衝突面に平行な方向へ影響する摩擦係数（加速率）
		//
		S3DVector	vBaseSpeed = m_vSpeed - vActSpeed ;
		S3DVector	vDeltaSpeed = vBaseSpeed + vRotateSpeed0 ;
		S3DVector	vPower0 = vBaseSpeed * pMaterial->fpWeight + vRotatePower0 ;
		float32_t	fpElasticity = pMaterial->fpElasticity * fpActElasticity ;
		float32_t	fpImpactSpeed = vDeltaSpeed.InnerProduct( vActNormal ) ;
		if ( fpImpactSpeed >= 0.0f )
		{
			// 離脱コースなので衝突無し
			return ;
		}
		float32_t	fpAbsImpactSpeed = - fpImpactSpeed ;
		S3DVector	vImpactSpeed = vActNormal * fpImpactSpeed ;
		S3DVector	vCrossSpeed = vDeltaSpeed - vImpactSpeed ;
		float32_t	fpImpactPower = - vPower0.InnerProduct( vActNormal ) ; ;
		float32_t	fpResistancePower = fpImpactPower * pMaterial->fpResistance ;
		float32_t	fpSlipResistanceSpeed = fpResistancePower / pMaterial->fpWeight ;
		float32_t	fpSlipResistance = 0.9f ;
		//
		// 運動量と回転運動量の比率
		//		cosImpact0	: 衝突点－重心点・速度の成す角の余弦＝運動速度影響度（>0）
		//
		S3DVector	vBaseSpeedDir = vBaseSpeed ;
		vBaseSpeedDir.Normalize() ;
		//
		float32_t	cosImpact0 =
				(float32_t) fabs( vImpactDir.InnerProduct( vBaseSpeedDir ) ) ;
		//
		// 速度反射
		//		fpBaseVSpeed		: 法線方向の重心点速度
		//		vBaseImpactSpeed	: 法線方向の重心点速度
		//		vBaseCrossSpeed		: 衝突面に平行な方向への重心点速度
		//		fpRefVBaseSpeed		: 法線方向の反射速度（回転含む）
		//		fpBaseRefRatio		: 衝突が重心に影響を及ぼす比率
		//		vBaseRefSpeed		: 重心点のの反射速度
		//
		float32_t	fpBaseVSpeed = vBaseSpeed.InnerProduct( vActNormal ) ;
		float32_t	fpRotVSpeed = (vRotateSpeed0 - vActSpeed).InnerProduct( vActNormal ) ;
		S3DVector	vBaseImpactSpeed = vActNormal * fpBaseVSpeed ;
		S3DVector	vBaseCrossSpeed = vBaseSpeed - vBaseImpactSpeed ;
		S3DVector	vRotCrossSpeed = (vRotateSpeed0 - vActSpeed) - vActNormal * fpRotVSpeed ;
		float32_t	fpBaseRefRatio = cosImpact0 + (1.0f - cosImpact0) * fpRotateWeight ;
		float32_t	fpRefVBaseSpeed =
						(fpBaseVSpeed - fpImpactSpeed * 2.0f * fpBaseRefRatio) * fpElasticity ;
		S3DVector	vBaseRefSpeed = vBaseCrossSpeed * fpElasticity
									- vRotCrossSpeed
										* (pMaterial->fpResistance * fpRotateWeight
											* (1.0f - fpElasticity) * fpElasticity)
									+ vActNormal * fpRefVBaseSpeed ;
		m_vSpeed = vBaseRefSpeed + vActSpeed ;
		//
		// 回転成分の衝突判定
		//		fpVRotSpeed			: 法線方向の回転速度
		//		fpVRotSpeed1		: 衝突後の法線方向の回転速度
		//		vRotImpactSpeed0	: 法線方向の回転速度（ベクトル）
		//		vRotImpactSpeed1	: 法線方向の回転速度（重心点速度ベース）
		//		vCrossRotSpeed0		: 衝突面に平行な方向への回転速度
		//		vCrossRotSpeed1		: 衝突面に平行な方向への回転速度（重心点速度ベース）
		//		fpRotCrossVSpeed	: 回転水平成分の垂直影響速度
		//		vImpactCrossDir		: vImpactDir の衝突面方向成分（正規化済み）
		//
		float32_t	cosImpactNormal =
				(float32_t) fabs( vImpactDir.InnerProduct( vActNormal ) ) ;
		float32_t	sinImpactNormal =
				(float32_t) sqrt( esl_fmaxf( 1.0f - cosImpactNormal * cosImpactNormal, 0.0f ) ) ;
		S3DVector	vRotateCrossSpeed = vCrossSpeed - vBaseCrossSpeed ;
		float32_t	fpCrossSpeed = (float32_t) vCrossSpeed.Absolute() ;
		float32_t	fpGripSpeed =  esl_fminf( fpSlipResistanceSpeed, fpCrossSpeed * 0.9f ) ;
		float32_t	fpGripResistance = (fpCrossSpeed > 0.0f) ? fpGripSpeed / fpCrossSpeed : 0.5f ;
		float32_t	fpGripRotSign = 1.0f ;
		float32_t	fpGripSpeedSign = 1.0f ;
		float32_t	fpRefVRotSpeed =
						(fpImpactSpeed - fpBaseVSpeed)
						- fpImpactSpeed * fpElasticity ;
		float32_t	fpRotCrossVSpeed =
						(float32_t) vRotateCrossSpeed.Absolute()
								* (sinImpactNormal * fpElasticity) ;
		S3DVector	vImpactCrossDir =
						vImpactDir - vActNormal * vImpactDir.InnerProduct( vActNormal ) ;
		vImpactCrossDir.Normalize() ;
		//
		if ( fpRefVRotSpeed * sinImpactNormal
				- fpRotCrossVSpeed + fpRefVBaseSpeed < 0.0f )
		{
			// 回転反射
			fpRefVRotSpeed = - fpRefVBaseSpeed + fpRotCrossVSpeed ;
		}
		S3DVector	vRotateSpeed = vActNormal * (fpRefVRotSpeed * sinImpactNormal)
								+ vImpactCrossDir * (fpRefVRotSpeed * cosImpactNormal)
								+ vRotateCrossSpeed * fpSlipResistance
								- vCrossSpeed * (fpGripResistance * fpGripRotSign) ;
		m_vSpeed -= vCrossSpeed * (fpGripResistance * fpGripSpeedSign) ;
		//
		// 回転合成
		//
		float32_t	cosRotGimbal =
					(float32_t) fabs( m_vRotate.InnerProduct( vImpactDir ) ) ;
		m_fpRotSpeed *= cosRotGimbal * fpElasticity ;
		//
		S3DVector	vRotate ;
		float32_t	fpRotSpeed ;
		CalcRotation
			( vRotate, fpRotSpeed,
				vImpactPos0, vRotateSpeed, fpImpactPosLen ) ;
		//
		AddRotate( vRotate, fpRotSpeed ) ;
#endif
	}
}

void S3DPhysicsScene::Actor::CollideWithObject
	( S3DPhysicsScene& scene,
		S3DPhysicsScene::Object& obj,
		const S3DDVector& vImpactPos,
		const S3DVector& vActNormal, float32_t secAdv )
{
	S3DPhysicsScene::Actor *	pAct = obj.GetPhysicalActor() ;
	if ( pAct != nullptr )
	{
		CollideWithActor
			( scene, *pAct, vImpactPos, vActNormal, secAdv ) ;
	}
	else
	{
		Collide( vImpactPos, vActNormal, secAdv ) ;
	}
}

void S3DPhysicsScene::Actor::CollideWithActor
	( S3DPhysicsScene& scene,
		S3DPhysicsScene::Actor& act,
		const S3DDVector& vImpactPos,
		const S3DVector& vActNormal, float32_t secAdv )
{
	ESLAssert( fabs(vActNormal.Absolute() - 1.0) < 1.0e-5 ) ;
	//
	// 二重処理禁止
	//
	scene.LockActor() ;
	if ( m_arrCurrentHit.FindPtr( &act ) >= 0 )
	{
		ESLAssert( act.m_arrCurrentHit.FindPtr( this ) >= 0 ) ;
		scene.UnlockActor() ;
		return ;
	}
	m_arrCurrentHit.Add( &act ) ;
	act.m_arrCurrentHit.Add( this ) ;
	scene.UnlockActor() ;
	//
	// 質量比（回転は考慮しない）
	//
	const Material *	pMaterial = m_pMaterial ;
	ESLAssert( pMaterial != nullptr ) ;
	const Material *	pActMaterial = act.m_pMaterial ;
	ESLAssert( pActMaterial != nullptr ) ;
	if ( (pMaterial == nullptr) || (pActMaterial == nullptr) )
	{
		return ;
	}
	float32_t	fpWeight0 = pMaterial->fpWeight
							/ (pMaterial->fpWeight + pActMaterial->fpWeight) ;
	float32_t	fpWeight1 = 1.0f - fpWeight0 ;
	float32_t	fpElasticity0 = pMaterial->fpElasticity ;
	float32_t	fpElasticity1 = pActMaterial->fpElasticity ;
	//
	// 衝突前の速度（回転は考慮しない）
	//
	S3DVector	vSpeed0 = m_vSpeed ;
	S3DVector	vSpeed1 = act.m_vSpeed ;
	//
	// この Actor の衝突処理
	//
	CollideWith
		( vImpactPos, vActNormal, secAdv,
			vSpeed1, fpWeight1 * fpElasticity1 * 2.0f ) ;
	//
	// 対象 Actor の衝突処理
	//
	S3DVector	vActNormal1 = - vActNormal ;
	act.CollideWith
		( vImpactPos, vActNormal1, secAdv,
			vSpeed0, fpWeight0 * fpElasticity0 * 2.0f ) ;
}

// 回転速度を合成
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::AddRotate
	( const S3DVector& vRotate, float32_t fpRotSpeed )
{
	m_vRotate = m_vRotate * m_fpRotSpeed + vRotate.Normalized() * fpRotSpeed ;
	m_fpRotSpeed = (float32_t) m_vRotate.Absolute() ;
	if ( m_fpRotSpeed > 1.0e-5 )
	{
		m_vRotate /= m_fpRotSpeed ;
	}
	else
	{
		m_vRotate.x = 1.0f ;
		m_vRotate.y = 0.0f ;
		m_vRotate.z = 0.0f ;
		m_fpRotSpeed = 0.0f ;
	}
}

// 回転速度を計算
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::CalcRotation
	( S3DVector& vRotate, float32_t& fpRotSpeed,
		const S3DVector& vOffsetPos,
		const S3DVector& vSpeed, float32_t fpRadius )
{
	vRotate = vOffsetPos * vSpeed ;
	double	r = vRotate.Absolute() ;
	if ( r > 1.0e-5 )
	{
		vRotate *= (float32_t) (1.0 / r) ;
		fpRotSpeed = (float32_t) (vSpeed.Absolute() / fpRadius) ;
	}
	else
	{
		vRotate.x = 1.0f ;
		vRotate.y = 0.0 ;
		vRotate.z = 0.0 ;
		fpRotSpeed = 0.0f ;
	}
}

// 物理演算オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor * S3DPhysicsScene::Actor::GetPhysicalActor( void ) const
{
	return	(S3DPhysicsScene::Actor*) this ;
}

// オーナーオブジェクト取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DPhysicsScene::Actor::GetOwnerObject( void ) const
{
	return	m_pOwnerObj ;
}

// 座標取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector&
	S3DPhysicsScene::Actor::GetPhysicsPosition( S3DDVector& vPos ) const
{
	vPos = m_vPos ;
	return	vPos ;
}

// 座標設定
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::SetPhysicsPosition( const S3DDVector& vPos )
{
	m_vPos = vPos ;
}

// 姿勢取得
//////////////////////////////////////////////////////////////////////////////
const S3DQuaternion&
	S3DPhysicsScene::Actor::GetPhysicsPosture( S3DQuaternion& qPosture ) const
{
	qPosture = m_qPosture ;
	return	qPosture ;
}

// 姿勢設定
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Actor::SetPhysicsPosture( const S3DQuaternion& qPosture )
{
	m_qPosture = qPosture ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPhysicsScene, ESLObject )

const S3DPhysicsScene::Material	S3DPhysicsScene::m_materialDefault =
{
	0, 1.0f, 1.5f, 1.0f, 2.0f, 0.3f, 0.5f
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::S3DPhysicsScene( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::~S3DPhysicsScene( void )
{
}

// 環境設定
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::SetEnvironment( const S3DPhysicsScene::Environment& env )
{
	m_env = env ;
}

// ソリッド球を追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DPhysicsScene::AddSolidSphere
	( S3DPhysicsScene::Actor * pAct,
		const S3DVector& vPos, float32_t fpRadius, size_t iInstanceNum )
{
	m_collision.AttachMeshUserData( pAct ) ;
	m_collision.SetUserClassesMask( S3DCollision::colliderPhysItem ) ;
	return	m_collision.AddSolidSphere( vPos, fpRadius, iInstanceNum ) ;
}

// ソリッド直方体を追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DPhysicsScene::AddSolidCube
	( S3DPhysicsScene::Actor * pAct,
		const S3DVector& vPos, const S3DVector& vCubeSize, size_t iInstanceNum )
{
	m_collision.AttachMeshUserData( pAct ) ;
	m_collision.SetUserClassesMask( S3DCollision::colliderPhysItem ) ;
	return	m_collision.AddSolidCube( vPos, vCubeSize, iInstanceNum ) ;
}

// コライダーバッファを追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DPhysicsScene::AddColliderObject
	( S3DPhysicsScene::Actor * pAct, S3DCollider * pCollider,
		const S4DMatrix * pmatInstancing, size_t iInstanceNum )
{
	m_collision.AttachMeshUserData( pAct ) ;
	m_collision.SetUserClassesMask( S3DCollision::colliderPhysItem ) ;
	return	m_collision.AddColliderObject( pCollider, pmatInstancing, iInstanceNum ) ;
}

// スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::Lock( void )
{
	m_collision.LockSyncBuffer() ;
}

void S3DPhysicsScene::Unlock( void )
{
	m_collision.UnlockSyncBuffer() ;
}

// アクター追加
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::AddActor( S3DPhysicsScene::Actor * pAct )
{
	m_actors.Add( pAct ) ;
}

void S3DPhysicsScene::AddActors( S3DPhysicsScene::Actor *const* ppAct, size_t nCount )
{
	m_actors.AddArray( ppAct, nCount ) ;
}

// アクター総数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DPhysicsScene::GetActorsCount( void ) const
{
	return	m_actors.GetLength() ;
}

// アクター取得
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor * S3DPhysicsScene::GetActorAt( size_t nIndex ) const
{
	return	m_actors.GetAt( nIndex ) ;
}

// アクター指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DPhysicsScene::FindActor( S3DPhysicsScene::Actor * pAct ) const
{
	return	m_actors.FindPtr( pAct ) ;
}

// アクター削除
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::DetachAllActor( void )
{
	m_actors.RemoveAll() ;
}

// 時間進行処理
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::AdvanceTime
	( S3DCollider& collider,
		const S3DPhysicsScene::ErrorGap& eg, float32_t secAdv )
{
	ESLAssert( secAdv > 0.00001 ) ;
	if ( secAdv <= 0.00001 )
	{
		return ;
	}
	//
	// 初期化
	//
	size_t			nActCount = m_actors.GetLength() ;
	Actor*const*	ppActors = m_actors.GetConstArray() ;
	//
	// 慣性運動
	//
	for ( size_t i = 0; i < nActCount; i ++ )
	{
		Actor *	pAct = ppActors[i] ;
		if ( pAct != nullptr )
		{
			pAct->m_arrCurrentHit.RemoveAll() ;
			pAct->KineticMove( m_env, secAdv ) ;
		}
	}
	//
	// 当たり判定
	//
	CollisionProcInstance	cpiInstance[MAX_THREADS] ;
	void *					ppInstance[MAX_THREADS] ;
	const size_t	nThreads =
			(size_t) esl_min( (int) SSystem::g_cpuLogicalCount, MAX_THREADS ) ;
	for ( size_t i = 0; i < nThreads; i ++ )
	{
		ppInstance[i] = &(cpiInstance[i]) ;
	}
	CollisionProc	proc( this, collider, eg, secAdv, nActCount, ppActors ) ;
	proc.Start( &(ppInstance[0]), nThreads ) ;
}

// CollisionProc 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::CollisionProc::CollisionProc
	( S3DPhysicsScene * pScene,
		S3DCollider& collider, const ErrorGap& eg, float32_t secAdv,
		size_t nActCount, Actor*const* ppActors )
	: m_pScene( pScene ), m_colider( collider ),
		m_errGap( eg ), m_secAdv( secAdv ),
		m_iNextAct( 0 ), m_nActCount( nActCount ), m_ppActors( ppActors )
{
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool S3DPhysicsScene::CollisionProc::Continue( void * pInstance )
{
	S3DPhysicsScene::CollisionProcInstance *
		pcpi = (S3DPhysicsScene::CollisionProcInstance*) pInstance ;

	if ( m_iNextAct < m_nActCount )
	{
		pcpi->pAct = m_ppActors[m_iNextAct ++] ;
		return	true ;
	}
	return	false ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::CollisionProc::RunParallel( void * pInstance )
{
	S3DPhysicsScene::CollisionProcInstance *
		pcpi = (S3DPhysicsScene::CollisionProcInstance*) pInstance ;

	Actor *	pAct = pcpi->pAct ;
	if ( pAct == nullptr )
	{
		return ;
	}
	const Material *	pMaterial = pAct->m_pMaterial ;
	ESLAssert( pMaterial != nullptr ) ;
	if ( (pMaterial == nullptr)
		|| (pMaterial->nFlags & actorNoMove) )
	{
		return ;
	}
	ActorHitDescTable	ahdtTable ;
	ahdtTable.pScene = m_pScene ;
	ahdtTable.pThisAct = pAct ;
	ahdtTable.nEntryCount = 0 ;
	//
	S3DCollision::Result	rsHit ;
	rsHit.SetInclusionSceneFlags( 0 ) ;
	rsHit.SetInclusionUserFlags( S3DCollision::colliderPhysTarget ) ;
	rsHit.ptrOnHitInstance = &ahdtTable ;
	//
	const size_t		nHitPoints = pAct->m_nHitPointCount ;
	const S3DVector4 *	pvHitPoints = pAct->m_pHitPoints ;
	//
	// 移動線分当たり判定
	//
	S3DQuaternion	qLastAct = pAct->m_qLastPosture ;
	S3DQuaternion	qCurAct = pAct->m_qPosture ;
	S3DMatrix		matLastAct, matCurAct ;
	float32_t		fpZoom = (pAct->m_vZoom.x + pAct->m_vZoom.y + pAct->m_vZoom.z) / 3.0f ;
	pAct->m_qPosture.ToMatrix( matCurAct ) ;
	pAct->m_qLastPosture.ToMatrix( matLastAct ) ;
	matCurAct.MagnifyByVector( pAct->m_vZoom ) ;
	matLastAct.MagnifyByVector( pAct->m_vZoom ) ;
	//
	for ( size_t iHit = 0; iHit < nHitPoints; iHit ++ )
	{
		S3DDVector	vLastPos = matLastAct * pvHitPoints[iHit] ;
		S3DDVector	vCurPos = matCurAct * pvHitPoints[iHit] ;
		vLastPos += pAct->m_vLastPos ;
		vCurPos += pAct->m_vPos ;
		//
		S3DVector	vMoveDelta = vCurPos - vLastPos ;
		double		fpMoveNorm = vMoveDelta.Absolute() ;
		if ( fpMoveNorm < 1.0e-8 )
		{
			continue ;
		}
		if ( rsHit.fpDistance > fpMoveNorm )
		{
			rsHit.fpDistance = (float32_t) fpMoveNorm ;
		}
		vMoveDelta *= 1.0f / (float32_t) fpMoveNorm ;
		ahdtTable.vMoveDelta = vMoveDelta ;
		//
		rsHit.fpDistance = (float32_t) fpMoveNorm ;
		rsHit.pMesh = nullptr ;
		rsHit.pqpcHit = nullptr ;
		rsHit.pfnOnHitCollider = &S3DPhysicsScene::Callback_MoveTestOnHitCollider ;
		//
		if ( m_colider.IsSegmentCrossing
			( vLastPos, vCurPos, m_errGap.fpHitGap, rsHit ) )
		{
			AddActorHitDescriptor
				( ahdtTable, rsHit,
					true, rsHit.fpDistance, iHit,
					rsHit.vHitGlobal, vMoveDelta, rsHit.vNormal ) ;
		}
		else
		{
			rsHit.pfnOnHitCollider = &S3DPhysicsScene::Callback_SphereTestOnHitCollider ;
			if ( m_colider.IsHitAgainstSphere
				( vCurPos, pvHitPoints[iHit].d * fpZoom + m_errGap.fpHitGap, rsHit ) )
			{
				S3DVector	vDelta = S3DVector(vCurPos) - rsHit.vHitGlobal ;
				AddActorHitDescriptor
					( ahdtTable, rsHit, false,
						(float32_t) vDelta.Absolute(), iHit,
						rsHit.vHitGlobal, vDelta, rsHit.vNormal ) ;
			}
		}
	}
	//
	// 衝突処理
	//
	if ( ahdtTable.nEntryCount > 0 )
	{
		//
		// 当たり判定座標計算
		//
		const size_t		iHit0 = ahdtTable.ahdeTable[0].iHitPoint ;
		Object * const		pHitObject0 = ahdtTable.ahdeTable[0].pHitObject ;
		const S3DDVector	vCurActPos = pAct->m_vPos ;
		const S3DDVector	vHitPos0 = ahdtTable.ahdeTable[0].vHitPos ;
		const S3DVector		vHitNormal0 = ahdtTable.ahdeTable[0].vHitNormal ;
		ESLAssert( pHitObject0 != nullptr ) ;
		//
		S3DDVector	vDirHitPoint = matCurAct * pvHitPoints[iHit0] ;
		S3DDVector	vCurPos = vDirHitPoint + vCurActPos ;
		//
		S3DDVector	vImpactPos = vHitPos0 ;
		S3DVector	vHitNormal = vHitNormal0 ;
		if ( ahdtTable.ahdeTable[0].flagMove )
		{
			S3DVector	vDelta = vCurPos - vHitPos0 ;
			float32_t	h = vDelta.InnerProduct(vHitNormal0) ;
			h = esl_fminf( h, 0.0f ) ;
			pAct->m_vPos += S3DDVector(vHitNormal0 * (pvHitPoints[iHit0].d * fpZoom - h)) ;
			vImpactPos = vCurPos - S3DDVector(vHitNormal0 * h) ;
		}
		else
		{
			S3DVector	vDelta = ahdtTable.ahdeTable[0].vHitDelta ;
			float32_t	fpDistance = vHitNormal0.InnerProduct( vDelta ) ;
			float32_t	h = pvHitPoints[iHit0].d * fpZoom - fpDistance ;
			if ( h > 0.0f )
			{
				pAct->m_vPos += S3DDVector( vHitNormal0 * h ) ;
			}
		}
		//
		// 座標調整
		//
		for ( size_t j = 1; j < ahdtTable.nEntryCount; j ++ )
		{
			const size_t		iHit = ahdtTable.ahdeTable[j].iHitPoint ;
			const S3DDVector	vHitPos = ahdtTable.ahdeTable[j].vHitPos ;
			const S3DVector		vNormal = ahdtTable.ahdeTable[j].vHitNormal ;
			//
			vDirHitPoint = matCurAct * pvHitPoints[iHit] ;
			vCurPos = vDirHitPoint + pAct->m_vPos ;
			//
			vImpactPos += vCurPos - S3DDVector(vNormal * (pvHitPoints[iHit].d * fpZoom)) ;
			vHitNormal += vNormal ;
			//
			S3DVector	vDelta = vCurPos - vHitPos ;
			float32_t	h = vDelta.InnerProduct(vNormal) ;
			if ( pvHitPoints[iHit].d * fpZoom - h > 0.0f )
			{
				S3DDVector	vOffset = vNormal * (pvHitPoints[iHit].d * fpZoom - h) ;
				pAct->m_vPos += vOffset ;
				vImpactPos += vOffset ;
			}
		}
		vImpactPos *= 1.0 / ahdtTable.nEntryCount ;
		if ( vHitNormal.Absolute() < 1.0e-5 )
		{
			vHitNormal = vHitNormal0 ;
		}
		vHitNormal.Normalize() ;
		//
		// 当たり判定処理
		//
		if ( !pAct->OnCollidedWith
			( pHitObject0, vImpactPos, vHitNormal, m_secAdv ) )
		{
			pAct->CollideWithObject
				( *m_pScene, *pHitObject0, vImpactPos, vHitNormal, m_secAdv ) ;
		}
		if ( ahdtTable.nEntryCount == 2 )
		{
			//
			// 2点で衝突している場合には
			// 2点を結ぶ線分を回転軸にする回転に制限する
			//
			const size_t	iHit1 = ahdtTable.ahdeTable[1].iHitPoint ;
			S3DDVector	vDirHitPoint0 = matCurAct * pvHitPoints[iHit0] ;
			S3DDVector	vDirHitPoint1 = matCurAct * pvHitPoints[iHit1] ;
			S3DDVector	vHitNormal0 = ahdtTable.ahdeTable[0].vHitNormal ;
			S3DDVector	vHitNormal1 = ahdtTable.ahdeTable[1].vHitNormal ;
			vDirHitPoint0 -= vHitNormal0 * (pvHitPoints[iHit0].d * fpZoom) ;
			vDirHitPoint1 -= vHitNormal1 * (pvHitPoints[iHit1].d * fpZoom) ;
				//
			S3DVector	vRotate = vDirHitPoint1 - vDirHitPoint0 ;
			vRotate.Normalize() ;
			//
			float32_t	cosRotate = vRotate.InnerProduct( pAct->m_vRotate ) ;
			pAct->m_fpRotSpeed *= cosRotate ;
			pAct->m_vRotate = vRotate ;
		}
		else if ( ahdtTable.nEntryCount >= 3 )
		{
			//
			// 3点以上で衝突している場合には
			// 3点を含む平面上での回転に制限する
			// ※衝突重心が三角形の内側を貫いていると暗黙に扱う
			// 　より複雑な形状を扱う場合にはあらかじめその判定が必要
			//
			const size_t	iHit1 = ahdtTable.ahdeTable[1].iHitPoint ;
			const size_t	iHit2 = ahdtTable.ahdeTable[2].iHitPoint ;
			S3DDVector	vDirHitPoint0 = matCurAct * pvHitPoints[iHit0] ;
			S3DDVector	vDirHitPoint1 = matCurAct * pvHitPoints[iHit1] ;
			S3DDVector	vDirHitPoint2 = matCurAct * pvHitPoints[iHit2] ;
			S3DDVector	vHitNormal0 = ahdtTable.ahdeTable[0].vHitNormal ;
			S3DDVector	vHitNormal1 = ahdtTable.ahdeTable[1].vHitNormal ;
			S3DDVector	vHitNormal2 = ahdtTable.ahdeTable[2].vHitNormal ;
			vDirHitPoint0 -= vHitNormal0 * (pvHitPoints[iHit0].d * fpZoom) ;
			vDirHitPoint1 -= vHitNormal1 * (pvHitPoints[iHit1].d * fpZoom) ;
			vDirHitPoint2 -= vHitNormal2 * (pvHitPoints[iHit2].d * fpZoom) ;
			//
			S3DVector	vRotate = (vDirHitPoint0 - vDirHitPoint)
									* (vDirHitPoint1 - vDirHitPoint) ;
			vRotate.Normalize() ;
			//
			float32_t	cosRotate = vRotate.InnerProduct( pAct->m_vRotate ) ;
			pAct->m_fpRotSpeed *= cosRotate ;
			pAct->m_vRotate = vRotate ;
		}
	}
}


// アクター当たり判定スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void S3DPhysicsScene::LockActor( void )
{
	m_csActor.Lock() ;
}

void S3DPhysicsScene::UnlockActor( void )
{
	m_csActor.Unlock() ;
}

// アクターの衝突処理済みか？
//////////////////////////////////////////////////////////////////////////////
bool S3DPhysicsScene::HasCollidedWith
	( S3DPhysicsScene::Actor * pAct1, S3DPhysicsScene::Actor * pAct2 ) const
{
	bool	flagCollided = false ;
	ESLAssert( pAct1 != nullptr ) ;
	ESLAssert( pAct2 != nullptr ) ;
	m_csActor.Lock() ;
	flagCollided = (pAct1->m_arrCurrentHit.FindPtr( pAct2 ) >= 0) ;
	m_csActor.Unlock() ;
	return	flagCollided ;
}

// 当たり判定テーブルに追加
//////////////////////////////////////////////////////////////////////////////
bool S3DPhysicsScene::AddActorHitDescriptor
	( ActorHitDescTable& ahdt,
		const S3DCollisionResult& rsHit,
		bool flagMove, float32_t fpDistance, size_t iHitPoint,
		const S3DVector& vHitPos,
		const S3DVector& vHitDelta, const S3DVector& vHitNormal )
{
	Object *	pHit = nullptr ;
	if ( rsHit.pPrimitiveMesh != nullptr )
	{
		pHit = ESLTypeCast<Object>( rsHit.pPrimitiveMesh->pUserData ) ;
	}
	if ( pHit == nullptr )
	{
		if ( rsHit.pMesh == nullptr )
		{
			return	false ;
		}
		pHit = ESLTypeCast<Object>( rsHit.pMesh->pUserData ) ;
		if ( pHit == nullptr )
		{
			return	false ;
		}
	}
	size_t	iDesc = 0 ;
	while ( iDesc < ahdt.nEntryCount )
	{
		ESLAssert( iDesc < ActorHitDescTableLength ) ;
		if ( ahdt.ahdeTable[iDesc].fpDistance > fpDistance )
		{
			break ;
		}
		iDesc ++ ;
	}
	if ( iDesc >= ActorHitDescTableLength )
	{
		return	false ;
	}
	for ( ssize_t j = (ssize_t) ahdt.nEntryCount; j - 1 >= (ssize_t) iDesc; j -- )
	{
		if ( j < ActorHitDescTableLength )
		{
			ESLAssert( j - 1 >= 0 ) ;
			ahdt.ahdeTable[j] = ahdt.ahdeTable[j - 1] ;
		}
	}
	ESLAssert( iDesc < ActorHitDescTableLength ) ;
	ActorHitDescEntry&	desc = ahdt.ahdeTable[iDesc] ;
	desc.flagMove = flagMove ;
	desc.fpDistance = fpDistance ;
	desc.iHitPoint = iHitPoint ;
	desc.pHitObject = pHit ;
	desc.vHitPos = vHitPos ;
	desc.vHitDelta = vHitDelta ;
	desc.vHitNormal = vHitNormal ;
	desc.vHitNormal.Normalize();
	//
	if ( ahdt.nEntryCount < ActorHitDescTableLength )
	{
		ahdt.nEntryCount ++ ;
	}
	return	true ;
}

// 当たり判定関数
//////////////////////////////////////////////////////////////////////////////
S3DCollision::HitColliderCallback
	S3DPhysicsScene::Callback_MoveTestOnHitCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	ActorHitDescTable *	pahdtTable = (ActorHitDescTable*) rsHit.ptrOnHitInstance ;
	ESLAssert( pahdtTable != nullptr ) ;
	Actor *	pThisAct = pahdtTable->pThisAct ;
	ESLAssert( pThisAct != nullptr ) ;
	//
	Object *	pHit = nullptr ;
	if ( pMesh != nullptr )
	{
		pHit = ESLTypeCast<Object>( pMesh->pUserData ) ;
	}
	if ( pHit == nullptr )
	{
		S3DCollisionResult::ChainIterator	iter( rsHit ) ;
		while ( iter.HasNext() )
		{
			const S3DCollision::MeshCollision *	pmcNext = iter.Next() ;
			if ( pmcNext != nullptr )
			{
				pHit = ESLTypeCast<Object>( pmcNext->pUserData ) ;
				if ( pHit != nullptr )
				{
					break ;
				}
			}
		}
		if ( pHit == nullptr )
		{
			// Object にしかヒットしない
			pThisAct->OnHitEventCollider
				( rsHit, pHit, vHitPos, vHitNormal, pMesh, iPolygon ) ;
			return	S3DCollision::hitColliderNextMesh ;
		}
	}
	if ( pHit == pThisAct )
	{
		// 自分自身の Actor にはヒットしない
		return	S3DCollision::hitColliderNextMesh ;
	}
	if ( rsHit.TestUserColliderMask( pMesh, S3DCollision::colliderEvent ) )
	{
		// 物理演算以外のイベント処理
		if ( !pThisAct->OnHitEventCollider
				( rsHit, pHit, vHitPos, vHitNormal, pMesh, iPolygon ) )
		{
			return	S3DCollision::hitColliderNextMesh ;
		}
	}
	/*
	Actor *	pHitAct = ESLTypeCast<Actor>(pHit) ;
	if ( (pHitAct != nullptr)
		&& pahdtTable->pScene->HasCollidedWith( pThisAct, pHitAct ) )
	{
		// 既に衝突済み Actor にはヒットしない
		return	S3DCollision::hitColliderNextMesh ;
	}
	*/
	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;
	//
	S3DVector	vNormal = hcgi.matToGlobal * vHitNormal ;
	float32_t	cosHit = vNormal.InnerProduct( pahdtTable->vMoveDelta ) ;
	if ( cosHit >= -1.0e-8f )
	{
		// 裏から表に抜ける方向にはヒットしない
		return	S3DCollision::hitColliderNext ;
	}
	// 当たり判定対象である
	return	S3DCollision::hitColliderReturn ;
}

S3DCollision::HitColliderCallback
	S3DPhysicsScene::Callback_SphereTestOnHitCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	ActorHitDescTable *	pahdtTable = (ActorHitDescTable*) rsHit.ptrOnHitInstance ;
	ESLAssert( pahdtTable != nullptr ) ;
	Actor *	pThisAct = pahdtTable->pThisAct ;
	ESLAssert( pThisAct != nullptr ) ;
	//
	Object *	pHit = nullptr ;
	if ( pMesh != nullptr )
	{
		pHit = ESLTypeCast<Object>( pMesh->pUserData ) ;
	}
	if ( pHit == nullptr )
	{
		S3DCollisionResult::ChainIterator	iter( rsHit ) ;
		while ( iter.HasNext() )
		{
			const S3DCollision::MeshCollision *	pmcNext = iter.Next() ;
			if ( pmcNext != nullptr )
			{
				pHit = ESLTypeCast<Object>( pmcNext->pUserData ) ;
				if ( pHit != nullptr )
				{
					break ;
				}
			}
		}
		if ( pHit == nullptr )
		{
			// Object にしかヒットしない
			pThisAct->OnHitEventCollider
				( rsHit, pHit, vHitPos, vHitNormal, pMesh, iPolygon ) ;
			return	S3DCollision::hitColliderNextMesh ;
		}
	}
	if ( pHit == pThisAct )
	{
		// 自分自身の Actor にはヒットしない
		return	S3DCollision::hitColliderNextMesh ;
	}
	if ( rsHit.TestUserColliderMask( pMesh, S3DCollision::colliderEvent ) )
	{
		// 物理演算以外のイベント処理
		if ( !pThisAct->OnHitEventCollider
				( rsHit, pHit, vHitPos, vHitNormal, pMesh, iPolygon ) )
		{
			return	S3DCollision::hitColliderNextMesh ;
		}
	}
	/*
	Actor *	pHitAct = ESLTypeCast<Actor>(pHit) ;
	if ( (pHitAct != nullptr)
		&& pahdtTable->pScene->HasCollidedWith( pThisAct, pHitAct ) )
	{
		// 既に衝突済み Actor にはヒットしない
		return	S3DCollision::hitColliderNextMesh ;
	}
	*/
	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;
	//
	S3DVector	vNormal = hcgi.matToGlobal * vHitNormal ;
	float32_t	cosHit = vNormal.InnerProduct( pThisAct->m_vSpeed ) ;
	if ( cosHit >= 0.0f )
	{
		// 裏から表に抜ける方向にはヒットしない
		return	S3DCollision::hitColliderNext ;
	}
	// 当たり判定対象である
	return	S3DCollision::hitColliderReturn ;
}




