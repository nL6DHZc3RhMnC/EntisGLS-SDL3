
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Scene.h>


// const EntisGLS4.Scene.ProjectionParam* getProjection( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getProjection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LEntisGLS4_Scene_ProjectionParam	valRet ;
	pScene->GetProjection( valRet ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setProjection( const EntisGLS4.Scene.ProjectionParam* projParam )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setProjection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ProjectionParam, projParam ) ;
	LQT_VERIFY_NULL_PTR( projParam ) ;

	pScene->SetProjection( *projParam ) ;

	LQT_RETURN_VOID() ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	pScene->SetParallax( xParallax, zFocusRate, xScreenDelta ) ;

	LQT_RETURN_VOID() ;
}

// boolean getParallax( double* xParallax, double* zFocusRate, double* xScreenDelta ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_POINTER( LDouble, xParallax ) ;
	LQT_VERIFY_NULL_PTR( xParallax ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zFocusRate ) ;
	LQT_VERIFY_NULL_PTR( zFocusRate ) ;
	LQT_FUNC_ARG_POINTER( LDouble, xScreenDelta ) ;
	LQT_VERIFY_NULL_PTR( xScreenDelta ) ;


	LQT_RETURN_BOOL( pScene->GetParallax( *xParallax, *zFocusRate, *xScreenDelta ) ) ;
}

// void setParallaxParam( const EntisGLS4.Scene.ParallaxParam* ppRight, const EntisGLS4.Scene.ParallaxParam* ppLeft )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setParallaxParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppRight ) ;
	LQT_VERIFY_NULL_PTR( ppRight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppLeft ) ;
	LQT_VERIFY_NULL_PTR( ppLeft ) ;

	pScene->SetParallaxParam( *ppRight, *ppLeft ) ;

	LQT_RETURN_VOID() ;
}

// boolean getParallaxParam( EntisGLS4.Scene.ParallaxParam* ppRight, EntisGLS4.Scene.ParallaxParam* ppLeft ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getParallaxParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppRight ) ;
	LQT_VERIFY_NULL_PTR( ppRight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppLeft ) ;
	LQT_VERIFY_NULL_PTR( ppLeft ) ;

	LQT_RETURN_BOOL( pScene->GetParallaxParam( *ppRight, *ppLeft ) ) ;
}

// ulong getShadingMethod( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getShadingMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LQT_RETURN_ULONG( pScene->GetShadingMethod() ) ;
}

// void setShadingMethod( ulong typeShading )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setShadingMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_ULONG( typeShading ) ;

	pScene->SetShadingMethod( (uint32_t) typeShading ) ;

	LQT_RETURN_VOID() ;
}

// boolean getBackColor( ARGB8* rgbaBack ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbaBack ) ;
	LQT_VERIFY_NULL_PTR( rgbaBack ) ;

	LQT_RETURN_BOOL( pScene->GetBackColor( *rgbaBack ) ) ;
}

// void setBackColor( const ARGB8* rgbaBack, boolean fillBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbaBack ) ;
	LQT_VERIFY_NULL_PTR( rgbaBack ) ;
	LQT_FUNC_ARG_BOOL( fillBack ) ;

	pScene->SetBackColor( *rgbaBack, fillBack ) ;

	LQT_RETURN_VOID() ;
}

// double getVisibleNearDistance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getVisibleNearDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LQT_RETURN_DOUBLE( pScene->GetVisibleNearDistance() ) ;
}

// double getVisibleFarDistance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getVisibleFarDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LQT_RETURN_DOUBLE( pScene->GetVisibleFarDistance() ) ;
}

// void setVisibleDistance( double zNear, double zFar )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setVisibleDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_DOUBLE( zNear ) ;
	LQT_FUNC_ARG_DOUBLE( zFar ) ;

	pScene->SetVisibleDistance( zNear, zFar ) ;

	LQT_RETURN_VOID() ;
}

// boolean getGlobalFog( EntisGLS4.Scene.FogParam* fogParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_FogParam, fogParam ) ;
	LQT_VERIFY_NULL_PTR( fogParam ) ;

	LQT_RETURN_BOOL( pScene->GetGlobalFog( *fogParam ) ) ;
}

// void setGlobalFog( const EntisGLS4.Scene.FogParam* fogParam, boolean flagFog )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_FogParam, fogParam ) ;
	LQT_VERIFY_NULL_PTR( fogParam ) ;
	LQT_FUNC_ARG_BOOL( flagFog ) ;

	pScene->SetGlobalFog( *fogParam, flagFog ) ;

	LQT_RETURN_VOID() ;
}

// void enableGlobalFog( boolean flagFog )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_enableGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_BOOL( flagFog ) ;

	pScene->EnableGlobalFog( flagFog ) ;

	LQT_RETURN_VOID() ;
}

// void addSceneItem( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_addSceneItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	S3DScene::Space *	pSpace = item->GetRef<S3DScene::Space>() ;
	if ( pSpace != nullptr )
	{
		pScene->GetRootSpace().AddChild( pSpace ) ;
	}
	else
	{
		S3DSceneComposer::ItemCommonSerializer *
				pCmnItem = item->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
		if ( pCmnItem != nullptr )
		{
			S3DScene::Item *	pItem = pCmnItem->GetSceneItem() ;
			if ( pItem != nullptr )
			{
				pScene->GetRootSpace().AddItem( pItem ) ;
			}
		}
	}

	LQT_RETURN_VOID() ;
}

// void detachSceneItem( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_detachSceneItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	S3DScene::Space *	pSpace = item->GetRef<S3DScene::Space>() ;
	if ( pSpace != nullptr )
	{
		pScene->GetRootSpace().RemoveChild( pSpace ) ;
	}
	else
	{
		S3DSceneComposer::ItemCommonSerializer *
				pCmnItem = item->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
		if ( pCmnItem != nullptr )
		{
			S3DScene::Item *	pItem = pCmnItem->GetSceneItem() ;
			if ( pItem != nullptr )
			{
				pScene->GetRootSpace().RemoveItem( pItem ) ;
			}
		}
	}

	LQT_RETURN_VOID() ;
}

// void setMainCamera( EntisGLS4.SceneCamera camera )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setMainCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneCamera, camera ) ;
	S3DScene::Camera *	pCamera = nullptr ;
	if ( camera != nullptr )
	{
		pCamera = camera->GetRef<S3DScene::Camera>() ;
	}

	pScene->SetMainCamera( pCamera ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneCamera getMainCamera( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getMainCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	S3DSceneComposer::CameraSerializer *	pCamera =
		ESLTypeCast<S3DSceneComposer::CameraSerializer>( pScene->GetMainCamera() ) ;
	if ( pCamera == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pCamera ) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_SceneCamera>
				( (S3DSceneComposer::ItemSerializer*) pCamera) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Scene.ItemClass getCurrentRenderingStage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getCurrentRenderingStage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LQT_RETURN_UINT( pScene->GetCurrentRenderingStage() ) ;
}

// EntisGLS4.Scene.ItemClass getCurrentRenderingPhase( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getCurrentRenderingPhase)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	LQT_RETURN_UINT( pScene->GetCurrentRenderingPhase() ) ;
}

// void postSceneUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_postSceneUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	S3DScene *	pScene = pThis->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;

	pScene->PostSceneUpdate() ;

	LQT_RETURN_VOID() ;
}

