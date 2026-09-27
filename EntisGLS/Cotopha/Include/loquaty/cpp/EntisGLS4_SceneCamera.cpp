
#include <loquaty.h>
#include "EntisGLS4_SceneCamera.h"

using namespace Loquaty ;


// SceneCamera( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneCamera, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getCameraPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraPosition( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	// pThis->setCameraPosition(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraTarget( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getCameraTarget(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraTarget( const Vector3d* vTarget )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTarget ) ;
	LQT_VERIFY_NULL_PTR( vTarget ) ;

	// pThis->setCameraTarget(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraTop( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraTop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getCameraTop(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraTop( const Vector3d* vTop )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraTop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTop ) ;
	LQT_VERIFY_NULL_PTR( vTop ) ;

	// pThis->setCameraTop(...) ;

	LQT_RETURN_VOID() ;
}



