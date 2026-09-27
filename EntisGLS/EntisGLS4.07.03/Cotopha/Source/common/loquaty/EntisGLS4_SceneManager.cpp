
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneManager.h>


// EntisGLS4.SceneManager getCurrent( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getCurrent)
{
	LQT_FUNC_ARG_LIST ;

	S3DCompositionManager *	pManager = LSceneManagerCurrent::GetCurrent() ;
	if ( pManager == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneManager) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneManager>( pManager ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// SceneManager( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneManager)
{
	LQT_FUNC_ARG_LIST ;
	S3DCompositionManager *	pManager = new S3DCompositionManager ;
	pManager->InitializeVM( nullptr, nullptr, false, &(_context.VM()) ) ;

	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneManager, pThis, ( new SSmartObject( pManager ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void startScene( EntisGLS4.SceneManager.Context context )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_startScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_SceneManager_Context, context ) ;
	LQT_VERIFY_NULL_PTR( context ) ;

	S3DSceneComposerPluginSceneInfo	sceneInfo ;
	GetLSceneManagerContext( sceneInfo, context ) ;

	pManager->OnStartScene( sceneInfo ) ;

	LQT_RETURN_VOID() ;
}

// void endScene( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_endScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;

	pManager->OnEndScene() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneManager.Context getSceneInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getSceneInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;

	S3DSceneComposerPluginSceneInfo	scpsi ;
	if ( !pManager->GetSceneInfo( scpsi ) )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LObjPtr	valRet( _context.new_Object( L"EntisGLS4.SceneManager.Context" ) ) ;
	SetLSceneManagerContext( valRet, scpsi ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// Loquaty getLoquaty( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_getLoquaty)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;

	LObjPtr	valRet( LObject::AddRef( pManager->LoquatyVM() ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setItemClass( String typeId, Class cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_setItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LClass, cls ) ;
	LQT_VERIFY_NULL_PTR( cls ) ;

	pManager->AddItemCreator( typeId.c_str(), new LSceneItemCreator( cls.Ptr() ) ) ;

	LQT_RETURN_VOID() ;
}

// void setControllerClass( String typeId, Class cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_setControllerClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LClass, cls ) ;
	LQT_VERIFY_NULL_PTR( cls ) ;

	pManager->AddControllerCreator( typeId.c_str(), new LSceneItemCreator( cls.Ptr() ) ) ;

	LQT_RETURN_VOID() ;
}

// void addPluginMenu( EntisGLS4.SceneManager.PluginDescriptorType type, String menuPath, String typeId, Function<EntisGLS4.SceneItem(const EntisGLS4.SceneManager.EditorEnvironment*)> fncCreateItem, Function<EntisGLS4.SceneManager.PluginMenuState(EntisGLS4.SceneItem)> fncOnUpdateMenu )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneManager_addPluginMenu)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneManager, pThis ) ;
	S3DCompositionManager *	pManager = pThis->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_STRING( menuPath ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, fncCreateItem ) ;
	LQT_VERIFY_NULL_PTR( fncCreateItem ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, fncOnUpdateMenu ) ;

	pManager->AddDynamicPluginDescriptor
		( new LScenePluginDescriptor
			( (S3DSceneComposerPluginDescriptorType) type,
				menuPath.c_str(), typeId.c_str(),
				fncCreateItem, fncOnUpdateMenu ) ) ;

	LQT_RETURN_VOID() ;
}

