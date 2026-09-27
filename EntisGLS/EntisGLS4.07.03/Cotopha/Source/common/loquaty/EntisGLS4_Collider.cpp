
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Collider.h>


struct	HitAgainstSphereParam
{
	LClass *	pResultClass ;
	LArrayObj *	pHitRes ;
	LUint		nResMax ;
	LBoolean	boolHit ;
} ;

static S3DCollision::HitColliderCallback
	LCallback_OnHitCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	HitAgainstSphereParam *	param =
		reinterpret_cast<HitAgainstSphereParam*>( rsHit.ptrOnHitInstance ) ;

	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;

	S3DCollisionResult	rsTemp = rsHit ;
	rsTemp.vHitGlobal = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	rsTemp.vNormal = hcgi.matToGlobal * vHitNormal ;
	rsTemp.matLocal = hcgi.matToGlobal ;
	rsTemp.vLocalBase = hcgi.vToGlobal ;
	rsTemp.pMesh = hcgi.pGlobalMesh ;
	rsTemp.iInstance = hcgi.pGlobalMesh->iInstance ;
	rsTemp.iMesh = hcgi.pGlobalMesh->iMesh ;
	rsTemp.iPolygon = iPolygon ;

	LObjPtr	pResult( param->pResultClass->CreateInstance() ) ;
	ESLAssert( pResult != nullptr ) ;
	SetLColliderResult( pResult, rsTemp ) ;
	param->pHitRes->m_array.push_back( pResult ) ;
	param->boolHit = true ;

	if ( param->pHitRes->GetElementCount() >= param->nResMax )
	{
		return	S3DCollision::hitColliderReturn ;
	}
	return	S3DCollision::hitColliderNext ;
}


// boolean isHitAgainstSphere( const Vector3d* vPos, float radius, EntisGLS4.Collider.Result[] hitRes, uint resMax, ulong maskColliders, ulong maskClasses ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collider_isHitAgainstSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collider, pThis ) ;
	S3DCollider *	pCollider = pThis->GetRef<S3DCollider>() ;
	LQT_VERIFY_NULL_PTR( pCollider ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, hitRes ) ;
	LQT_VERIFY_NULL_PTR( hitRes ) ;
	LQT_FUNC_ARG_UINT( resMax ) ;
	LQT_FUNC_ARG_ULONG( maskColliders ) ;
	LQT_FUNC_ARG_ULONG( maskClasses ) ;

	HitAgainstSphereParam	param ;
	param.pResultClass = _context.VM().GetClassPathAs( L"EntisGLS4.Collider.Result" ) ;
	param.pHitRes = hitRes.Ptr() ;
	param.nResMax = resMax ;
	param.boolHit = false ;
	if ( param.pResultClass == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}

	S3DCollisionResult	rsHit ;
	rsHit.pfnOnHitCollider = LCallback_OnHitCollider ;
	rsHit.ptrOnHitInstance = &param ;
	rsHit.SetInclusionSceneFlags( (uint32_t) maskClasses ) ;
	rsHit.SetInclusionUserFlags( (uint32_t) maskColliders ) ;

	pCollider->IsHitAgainstSphere( *vPos, radius, rsHit ) ;

	LQT_RETURN_BOOL( param.boolHit ) ;
}

// boolean isSegmentCrossing( const Vector3d* vPos0, const Vector3d* vPos1, float errorGap, EntisGLS4.Collider.Result hitRes, ulong maskColliders, ulong maskClasses ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collider_isSegmentCrossing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collider, pThis ) ;
	S3DCollider *	pCollider = pThis->GetRef<S3DCollider>() ;
	LQT_VERIFY_NULL_PTR( pCollider ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos0 ) ;
	LQT_VERIFY_NULL_PTR( vPos0 ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos1 ) ;
	LQT_VERIFY_NULL_PTR( vPos1 ) ;
	LQT_FUNC_ARG_FLOAT( errorGap ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_Collider_Result, hitRes ) ;
	LQT_VERIFY_NULL_PTR( hitRes ) ;
	LQT_FUNC_ARG_ULONG( maskColliders ) ;
	LQT_FUNC_ARG_ULONG( maskClasses ) ;

	S3DCollisionResult	rsCross ;
	rsCross.SetInclusionSceneFlags( (uint32_t) maskClasses ) ;
	rsCross.SetInclusionUserFlags( (uint32_t) maskColliders ) ;
	rsCross.fpDistance = (float32_t) (*vPos0 - *vPos1).Absolute() ;

	LBoolean	valRet =
		pCollider->IsSegmentCrossing( *vPos0, *vPos1, errorGap, rsCross ) ;
	if ( valRet )
	{
		SetLColliderResult( hitRes, rsCross ) ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}



