
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneCustomController.h>


// SceneCustomController( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneCustomController,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneCustomController>
			( new SSmartObject
				( (S3DSceneComposer::Controller*) 
					new SceneCustomController
						( _context.VM(), pThis, typeId.c_str() ) ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneCustomController.BehaviorFlag getBehaviorFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_getBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	S3DSceneComposer::Controller *	pCtrl = pThis->GetRef<S3DSceneComposer::Controller>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;

	LQT_RETURN_UINT( pCtrl->GetControllerBehaviorFlags() ) ;
}

// EntisGLS4.SceneCustomController.BehaviorFlag modifyBehaviorFlags( EntisGLS4.SceneCustomController.BehaviorFlag addFlags, EntisGLS4.SceneCustomController.BehaviorFlag removeFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_modifyBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	SceneCustomController *	pCtrl = pThis->GetRef<SceneCustomController>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;
	LQT_FUNC_ARG_UINT( addFlags ) ;
	LQT_FUNC_ARG_UINT( removeFlags ) ;

	LUint32	valRet = (pCtrl->GetControllerBehaviorFlags() & ~removeFlags) | addFlags ;
	pCtrl->SetControllerBehaviorFlags( valRet ) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getRenderEventClasses( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_getRenderEventClasses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	S3DSceneComposer::Controller *	pCtrl = pThis->GetRef<S3DSceneComposer::Controller>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;

	LQT_RETURN_UINT( pCtrl->GetBehaviorRenderEventClasses() ) ;
}

// uint modifyRenderEventClasses( uint maskAdd, uint maskRemove )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCustomController_modifyRenderEventClasses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCustomController, pThis ) ;
	SceneCustomController *	pCtrl = pThis->GetRef<SceneCustomController>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;
	LQT_FUNC_ARG_UINT( maskAdd ) ;
	LQT_FUNC_ARG_UINT( maskRemove ) ;

	LUint32	valRet = (pCtrl->GetBehaviorRenderEventClasses() & ~maskRemove) | maskAdd ;
	pCtrl->SetBehaviorRenderEventClasses( valRet ) ;

	LQT_RETURN_UINT( valRet ) ;
}



