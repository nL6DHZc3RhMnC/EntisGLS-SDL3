
#include <loquaty.h>
#include "EntisGLS4_SceneCustomController.h"

using namespace Loquaty ;


// SceneCustomController( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneCustomController,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneCustomController>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneCustomController.BehaviorFlag getBehaviorFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_getBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getBehaviorFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// EntisGLS4.SceneCustomController.BehaviorFlag modifyBehaviorFlags( EntisGLS4.SceneCustomController.BehaviorFlag addFlags, EntisGLS4.SceneCustomController.BehaviorFlag removeFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_modifyBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	LQT_FUNC_ARG_UINT( addFlags ) ;
	LQT_FUNC_ARG_UINT( removeFlags ) ;

	LUint32	valRet ;
	// valRet = pThis->modifyBehaviorFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getRenderEventClasses( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_getRenderEventClasses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getRenderEventClasses(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint modifyRenderEventClasses( uint maskAdd, uint maskRemove )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_modifyRenderEventClasses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	LQT_FUNC_ARG_UINT( maskAdd ) ;
	LQT_FUNC_ARG_UINT( maskRemove ) ;

	LUint32	valRet ;
	// valRet = pThis->modifyRenderEventClasses(...) ;

	LQT_RETURN_UINT( valRet ) ;
}



