
#include <loquaty.h>
#include "EntisGLS4_RenderContext.h"

using namespace Loquaty ;


// boolean copyBufferFrom( EntisGLS4.RenderContext renderSrc, EntisGLS4.RenderContext.CopyBufferFlag flags, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_copyBufferFrom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderContext, renderSrc ) ;
	LQT_VERIFY_NULL_PTR( renderSrc ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->copyBufferFrom(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean attachMultiTargetImages( EntisGLS4.Image[] targets )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_attachMultiTargetImages)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, targets ) ;
	LQT_VERIFY_NULL_PTR( targets ) ;

	LBoolean	valRet ;
	// valRet = pThis->attachMultiTargetImages(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Image[] getMultiTargetImages( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getMultiTargetImages)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getMultiTargetImages(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean setProjectionScreen( const Vector3* vScreen, double zScale, double fpPixelAspect )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_DOUBLE( zScale ) ;
	LQT_FUNC_ARG_DOUBLE( fpPixelAspect ) ;

	LBoolean	valRet ;
	// valRet = pThis->setProjectionScreen(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getProjectionScreen( Vector3* vScreen, double* zScale, double* fpPixelAspect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zScale ) ;
	LQT_VERIFY_NULL_PTR( zScale ) ;
	LQT_FUNC_ARG_POINTER( LDouble, fpPixelAspect ) ;
	LQT_VERIFY_NULL_PTR( fpPixelAspect ) ;

	LBoolean	valRet ;
	// valRet = pThis->getProjectionScreen(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setCamera( const Matrix3d* matCamera, const Vector3d* posCamera )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matCamera ) ;
	LQT_VERIFY_NULL_PTR( matCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posCamera ) ;
	LQT_VERIFY_NULL_PTR( posCamera ) ;

	// pThis->setCamera(...) ;

	LQT_RETURN_VOID() ;
}

// void setCameraAngleVector( const Vector3d* posTarget, const Vector3d* posView, const Vector3d* vAngleTop )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setCameraAngleVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posTarget ) ;
	LQT_VERIFY_NULL_PTR( posTarget ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posView ) ;
	LQT_VERIFY_NULL_PTR( posView ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vAngleTop ) ;
	LQT_VERIFY_NULL_PTR( vAngleTop ) ;

	// pThis->setCameraAngleVector(...) ;

	LQT_RETURN_VOID() ;
}

// void getCamera( Matrix3d* matCamera, Vector3d* posCamera ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matCamera ) ;
	LQT_VERIFY_NULL_PTR( matCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posCamera ) ;
	LQT_VERIFY_NULL_PTR( posCamera ) ;

	// pThis->getCamera(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isSphereIntoView( const Vector3d* vPos, double radius ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_isSphereIntoView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_DOUBLE( radius ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSphereIntoView(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	// pThis->setParallax(...) ;

	LQT_RETURN_VOID() ;
}

// double getParallax( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getParallax(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setZClipRange( double zMin, double zMax )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setZClipRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( zMin ) ;
	LQT_FUNC_ARG_DOUBLE( zMax ) ;

	// pThis->setZClipRange(...) ;

	LQT_RETURN_VOID() ;
}

// void setLightEntries( const EntisGLS4.LightEntry* pLights, ulong countLight )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setLightEntries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_LightEntry, pLights ) ;
	LQT_VERIFY_NULL_PTR( pLights ) ;
	LQT_FUNC_ARG_ULONG( countLight ) ;

	// pThis->setLightEntries(...) ;

	LQT_RETURN_VOID() ;
}

// void setShadowMap( uint idLight, EntisGLS4.Image shadowMapDepth, const EntisGLS4.ShadowMapInfo* infShadowMap, EntisGLS4.Image shadowMapColor )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setShadowMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_UINT( idLight ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, shadowMapDepth ) ;
	LQT_VERIFY_NULL_PTR( shadowMapDepth ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ShadowMapInfo, infShadowMap ) ;
	LQT_VERIFY_NULL_PTR( infShadowMap ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, shadowMapColor ) ;
	LQT_VERIFY_NULL_PTR( shadowMapColor ) ;

	// pThis->setShadowMap(...) ;

	LQT_RETURN_VOID() ;
}

// void setFog( uint rgbFog, double zFogNear, double zFogFar )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_UINT( rgbFog ) ;
	LQT_FUNC_ARG_DOUBLE( zFogNear ) ;
	LQT_FUNC_ARG_DOUBLE( zFogFar ) ;

	// pThis->setFog(...) ;

	LQT_RETURN_VOID() ;
}

// void enableFog( boolean fFog )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_enableFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_BOOL( fFog ) ;

	// pThis->enableFog(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.RenderContext.StereoViewIndex currentParallaxView( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_currentParallaxView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->currentParallaxView(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean selectParallaxView( EntisGLS4.RenderContext.StereoViewIndex sviView )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_selectParallaxView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_INT( sviView ) ;

	LBoolean	valRet ;
	// valRet = pThis->selectParallaxView(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean begin3DRenderer( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_begin3DRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->begin3DRenderer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean end3DRenderer( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_end3DRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->end3DRenderer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.RenderDevice getRenderDeviceObject( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getRenderDeviceObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	// valRet = pThis->getRenderDeviceObject(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_RenderDevice> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



