
#include <loquaty.h>
#include "EntisGLS4_SceneManager.h"

using namespace Loquaty ;


// EntisGLS4.SceneManager getCurrent( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getCurrent)
{
	LQT_FUNC_ARG_LIST ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneManager) ) ) ;
	// valRet = getCurrent(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneManager> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneManager>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// SceneManager( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneManager)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneManager, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void startScene( EntisGLS4.SceneManager.Context context )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_startScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_SceneManager_Context, context ) ;
	LQT_VERIFY_NULL_PTR( context ) ;

	// pThis->startScene(...) ;

	LQT_RETURN_VOID() ;
}

// void endScene( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_endScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;

	// pThis->endScene(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneManager.Context getSceneInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getSceneInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getSceneInfo(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// Loquaty getLoquaty( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getLoquaty)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getLoquaty(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setItemClass( String typeId, Class cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_setItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LClass, cls ) ;
	LQT_VERIFY_NULL_PTR( cls ) ;

	// pThis->setItemClass(...) ;

	LQT_RETURN_VOID() ;
}

// void setControllerClass( String typeId, Class cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_setControllerClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LClass, cls ) ;
	LQT_VERIFY_NULL_PTR( cls ) ;

	// pThis->setControllerClass(...) ;

	LQT_RETURN_VOID() ;
}

// void addPluginMenu( EntisGLS4.SceneManager.PluginDescriptorType type, String menuPath, String typeId, Function<EntisGLS4.SceneProperty(const EntisGLS4.SceneManager.EditorEnvironment*)> fncCreateItem, Function<EntisGLS4.SceneManager.PluginMenuState(EntisGLS4.SceneItem)> fncOnUpdateMenu )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_addPluginMenu)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_STRING( menuPath ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, fncCreateItem ) ;
	LQT_VERIFY_NULL_PTR( fncCreateItem ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, fncOnUpdateMenu ) ;
	LQT_VERIFY_NULL_PTR( fncOnUpdateMenu ) ;

	// pThis->addPluginMenu(...) ;

	LQT_RETURN_VOID() ;
}



