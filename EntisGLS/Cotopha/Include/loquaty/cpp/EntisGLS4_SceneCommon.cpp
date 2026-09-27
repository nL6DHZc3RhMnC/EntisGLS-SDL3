
#include <loquaty.h>
#include "EntisGLS4_SceneCommon.h"

using namespace Loquaty ;


// EntisGLS4.Scene.ItemClass getItemClass( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_getItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getItemClass(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setItemClass( EntisGLS4.Scene.ItemClass cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_setItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	LQT_FUNC_ARG_UINT( cls ) ;

	// pThis->setItemClass(...) ;

	LQT_RETURN_VOID() ;
}

// uint getRenderPriority( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_getRenderPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getRenderPriority(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setRenderPriority( uint priority )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_setRenderPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	LQT_FUNC_ARG_UINT( priority ) ;

	// pThis->setRenderPriority(...) ;

	LQT_RETURN_VOID() ;
}



