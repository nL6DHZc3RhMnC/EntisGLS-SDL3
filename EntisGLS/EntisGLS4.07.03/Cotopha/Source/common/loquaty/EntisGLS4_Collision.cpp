
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Collision.h>

using namespace Loquaty ;


// Collision( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Collision)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_Collision, pThis,
			( new SSmartObject( (S3DRenderBufferInterface*) new S3DCollision ) ) ) ;

	LQT_RETURN_VOID() ;
}

// float getCurrentThickness( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getCurrentThickness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	LQT_RETURN_FLOAT( pCol->GetCurrentThickness() ) ;
}

// void setCurrentThickness( float thickness )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setCurrentThickness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_FLOAT( thickness ) ;

	pCol->SetCurrentThickness( thickness ) ;

	LQT_RETURN_VOID() ;
}

// void attachMeshUserData( EntisGLS4.SceneItem userData )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_attachMeshUserData)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, userData ) ;
	LQT_VERIFY_NULL_PTR( userData ) ;

	pCol->AttachMeshUserData( userData->GetReference() ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getMeshUserData( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getMeshUserData)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	S3DSceneComposer::Parameter *	pParam =
		ESLTypeCast<S3DSceneComposer::Parameter>( pCol->GetMeshUserData() ) ;
	if ( pParam == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pParam ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pParam) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setSceneClassesMask( uint maskClasses )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setSceneClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_UINT( maskClasses ) ;

	pCol->SetSceneClassesMask( maskClasses ) ;

	LQT_RETURN_VOID() ;
}

// uint getSceneClassesMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getSceneClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	LQT_RETURN_UINT( pCol->GetSceneClassesMask() ) ;
}

// void setUserClassesMask( uint maskClasses )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setUserClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_UINT( maskClasses ) ;

	pCol->SetUserClassesMask( maskClasses ) ;

	LQT_RETURN_VOID() ;
}

// uint getUserClassesMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getUserClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	LQT_RETURN_UINT( pCol->GetUserClassesMask() ) ;
}

// boolean addSolidSphere( const Vector3d* vPos, float radius, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LQT_RETURN_BOOL
		( pCol->AddSolidSphere
			( *vPos, radius, (size_t) iInstanceNum ) == sglErrSuccess ) ;
}

// boolean addSolidCube( const Vector3d* vPos, const Vector3d* vCubeSize, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidCube)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vCubeSize ) ;
	LQT_VERIFY_NULL_PTR( vCubeSize ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LQT_RETURN_BOOL
		( pCol->AddSolidCube
			( *vPos, *vCubeSize, (size_t) iInstanceNum ) == sglErrSuccess ) ;
}

// boolean addSolidTubeList( const Vector3* pPoints, uint nPointCount, float radius )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidTubeList)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector3, pPoints, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_UINT( nPointCount ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;

	LQT_RETURN_BOOL
		( pCol->AddSolidTubeList
			( pPoints, (size_t) nPointCount, radius ) == sglErrSuccess ) ;
}

// boolean addColliderObject( EntisGLS4.Collider collider, const Matrix4* pmatInstancing, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addColliderObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pThisCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pThisCol ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Collider, collider ) ;
	LQT_VERIFY_NULL_PTR( collider ) ;
	S3DCollider *	pCollider = collider->GetRef<S3DCollider>() ;
	LQT_VERIFY_NULL_PTR( pCollider ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, pmatInstancing ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LQT_RETURN_BOOL
		( pThisCol->AddColliderObject
			( pCollider, pmatInstancing, (size_t) iInstanceNum ) == sglErrSuccess ) ;
}

// void beginBatchBuild( )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_beginBatchBuild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	pCol->BeginBatchBuild() ;

	LQT_RETURN_VOID() ;
}

// void endBatchBuild( )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_endBatchBuild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	S3DCollision *	pCol = pThis->GetRef<S3DCollision>() ;
	LQT_VERIFY_NULL_PTR( pCol ) ;

	pCol->EndBatchBuild() ;

	LQT_RETURN_VOID() ;
}



