
#include <loquaty.h>
#include "EntisGLS4_Scene.h"

using namespace Loquaty ;


// const EntisGLS4.Scene.ProjectionParam* getProjection( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getProjection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LEntisGLS4_Scene_ProjectionParam	valRet ;
	// valRet = pThis->getProjection(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setProjection( const EntisGLS4.Scene.ProjectionParam* projParam )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setProjection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ProjectionParam, projParam ) ;
	LQT_VERIFY_NULL_PTR( projParam ) ;

	// pThis->setProjection(...) ;

	LQT_RETURN_VOID() ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	// pThis->setParallax(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getParallax( double* xParallax, double* zFocusRate, double* xScreenDelta ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_POINTER( LDouble, xParallax ) ;
	LQT_VERIFY_NULL_PTR( xParallax ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zFocusRate ) ;
	LQT_VERIFY_NULL_PTR( zFocusRate ) ;
	LQT_FUNC_ARG_POINTER( LDouble, xScreenDelta ) ;
	LQT_VERIFY_NULL_PTR( xScreenDelta ) ;

	LBoolean	valRet ;
	// valRet = pThis->getParallax(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setParallaxParam( const EntisGLS4.Scene.ParallaxParam* ppRight, const EntisGLS4.Scene.ParallaxParam* ppLeft )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setParallaxParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppRight ) ;
	LQT_VERIFY_NULL_PTR( ppRight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppLeft ) ;
	LQT_VERIFY_NULL_PTR( ppLeft ) ;

	// pThis->setParallaxParam(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getParallaxParam( EntisGLS4.Scene.ParallaxParam* ppRight, EntisGLS4.Scene.ParallaxParam* ppLeft ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getParallaxParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppRight ) ;
	LQT_VERIFY_NULL_PTR( ppRight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_ParallaxParam, ppLeft ) ;
	LQT_VERIFY_NULL_PTR( ppLeft ) ;

	LBoolean	valRet ;
	// valRet = pThis->getParallaxParam(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getShadingMethod( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getShadingMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getShadingMethod(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void setShadingMethod( ulong typeShading )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setShadingMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_ULONG( typeShading ) ;

	// pThis->setShadingMethod(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getBackColor( ARGB8* rgbaBack ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbaBack ) ;
	LQT_VERIFY_NULL_PTR( rgbaBack ) ;

	LBoolean	valRet ;
	// valRet = pThis->getBackColor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setBackColor( const ARGB8* rgbaBack, boolean fillBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbaBack ) ;
	LQT_VERIFY_NULL_PTR( rgbaBack ) ;
	LQT_FUNC_ARG_BOOL( fillBack ) ;

	// pThis->setBackColor(...) ;

	LQT_RETURN_VOID() ;
}

// double getVisibleNearDistance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getVisibleNearDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getVisibleNearDistance(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// double getVisibleFarDistance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getVisibleFarDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getVisibleFarDistance(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setVisibleDistance( double zNear, double zFar )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setVisibleDistance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( zNear ) ;
	LQT_FUNC_ARG_DOUBLE( zFar ) ;

	// pThis->setVisibleDistance(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getGlobalFog( EntisGLS4.Scene.FogParam* fogParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_FogParam, fogParam ) ;
	LQT_VERIFY_NULL_PTR( fogParam ) ;

	LBoolean	valRet ;
	// valRet = pThis->getGlobalFog(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setGlobalFog( const EntisGLS4.Scene.FogParam* fogParam, boolean flagFog )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Scene_FogParam, fogParam ) ;
	LQT_VERIFY_NULL_PTR( fogParam ) ;
	LQT_FUNC_ARG_BOOL( flagFog ) ;

	// pThis->setGlobalFog(...) ;

	LQT_RETURN_VOID() ;
}

// void enableGlobalFog( boolean flagFog )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_enableGlobalFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagFog ) ;

	// pThis->enableGlobalFog(...) ;

	LQT_RETURN_VOID() ;
}

// void addSceneItem( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_addSceneItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	// pThis->addSceneItem(...) ;

	LQT_RETURN_VOID() ;
}

// void detachSceneItem( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_detachSceneItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	// pThis->detachSceneItem(...) ;

	LQT_RETURN_VOID() ;
}

// void setMainCamera( EntisGLS4.SceneCamera camera )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_setMainCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneCamera, camera ) ;
	LQT_VERIFY_NULL_PTR( camera ) ;

	// pThis->setMainCamera(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneCamera getMainCamera( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getMainCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneCamera) ) ) ;
	// valRet = pThis->getMainCamera(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneCamera> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneCamera>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Scene.ItemClass getCurrentRenderingStage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getCurrentRenderingStage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getCurrentRenderingStage(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// EntisGLS4.Scene.ItemClass getCurrentRenderingPhase( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_getCurrentRenderingPhase)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getCurrentRenderingPhase(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void postSceneUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Scene_postSceneUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Scene, pThis ) ;

	// pThis->postSceneUpdate(...) ;

	LQT_RETURN_VOID() ;
}



