
#include <loquaty.h>
#include "EntisGLS4_Collision.h"

using namespace Loquaty ;


// Collision( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Collision)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_Collision, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// float getCurrentThickness( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getCurrentThickness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	LFloat	valRet ;
	// valRet = pThis->getCurrentThickness(...) ;

	LQT_RETURN_FLOAT( valRet ) ;
}

// void setCurrentThickness( float thickness )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setCurrentThickness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_FLOAT( thickness ) ;

	// pThis->setCurrentThickness(...) ;

	LQT_RETURN_VOID() ;
}

// void attachMeshUserData( EntisGLS4.SceneItem userData )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_attachMeshUserData)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, userData ) ;
	LQT_VERIFY_NULL_PTR( userData ) ;

	// pThis->attachMeshUserData(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getMeshUserData( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getMeshUserData)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getMeshUserData(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setSceneClassesMask( uint maskClasses )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setSceneClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_UINT( maskClasses ) ;

	// pThis->setSceneClassesMask(...) ;

	LQT_RETURN_VOID() ;
}

// uint getSceneClassesMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getSceneClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getSceneClassesMask(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setUserClassesMask( uint maskClasses )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_setUserClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_UINT( maskClasses ) ;

	// pThis->setUserClassesMask(...) ;

	LQT_RETURN_VOID() ;
}

// uint getUserClassesMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_getUserClassesMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getUserClassesMask(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// boolean addSolidSphere( const Vector3d* vPos, float radius, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LBoolean	valRet ;
	// valRet = pThis->addSolidSphere(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addSolidCube( const Vector3d* vPos, const Vector3d* vCubeSize, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidCube)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vCubeSize ) ;
	LQT_VERIFY_NULL_PTR( vCubeSize ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LBoolean	valRet ;
	// valRet = pThis->addSolidCube(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addSolidTubeList( const Vector3* pPoints, uint nPointCount, float radius )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addSolidTubeList)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, pPoints ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_UINT( nPointCount ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;

	LBoolean	valRet ;
	// valRet = pThis->addSolidTubeList(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addColliderObject( EntisGLS4.Collider collider, const Matrix4* pmatInstancing, uint iInstanceNum )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_addColliderObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Collider, collider ) ;
	LQT_VERIFY_NULL_PTR( collider ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, pmatInstancing ) ;
	LQT_VERIFY_NULL_PTR( pmatInstancing ) ;
	LQT_FUNC_ARG_UINT( iInstanceNum ) ;

	LBoolean	valRet ;
	// valRet = pThis->addColliderObject(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void beginBatchBuild( )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_beginBatchBuild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	// pThis->beginBatchBuild(...) ;

	LQT_RETURN_VOID() ;
}

// void endBatchBuild( )
IMPL_LOQUATY_FUNC(EntisGLS4_Collision_endBatchBuild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collision, pThis ) ;

	// pThis->endBatchBuild(...) ;

	LQT_RETURN_VOID() ;
}



