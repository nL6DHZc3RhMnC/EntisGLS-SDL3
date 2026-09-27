
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RenderContext.h>


// boolean copyBufferFrom( EntisGLS4.RenderContext renderSrc, uint flags, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_copyBufferFrom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderContext, renderSrc ) ;
	LQT_VERIFY_NULL_PTR( renderSrc ) ;
	S3DRenderContextInterface *	pRenderSrc = renderSrc->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRenderSrc ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LQT_RETURN_BOOL
		( pRender->CopyBufferFrom
			( *pRenderSrc, flags, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean attachMultiTargetImages( EntisGLS4.Image[] targets )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_attachMultiTargetImages)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, targets ) ;
	LQT_VERIFY_NULL_PTR( targets ) ;

	SPointerArray<SGLImageObject>	bufTargets ;
	const size_t		nCount = targets->GetElementCount() ;
	SGLImageObject**	ppTargets = bufTargets.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		std::shared_ptr<LEntisGLS4_Image>
			pImage = targets->GetElementNativeAt<LEntisGLS4_Image>( i ) ;
		if ( pImage != nullptr )
		{
			ppTargets[i] = pImage->GetRef<SGLImageObject>() ;
		}
	}

	LQT_RETURN_BOOL
		( pRender->AttachMultiTargetImages( ppTargets, nCount ) == sglErrSuccess ) ;
}

// EntisGLS4.Image[] getMultiTargetImages( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getMultiTargetImages)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	size_t	nCount = 0 ;
	SGLImageObject*const*
			ppTargets = pRender->GetMultiTargetImages( nCount ) ;

	LClass *		pImageClass = LQT_GET_CLASS(EntisGLS4.Image) ;
	LPtr<LArrayObj>	pImageArray( _context.new_Array( pImageClass ) ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		LPtr<LNativeObj>	pImageObj ;
		if ( ppTargets[i] != nullptr )
		{
			pImageObj.SetPtr( new LNativeObj( pImageClass ) ) ;
			pImageObj->SetNative( std::make_shared<LEntisGLS4_Image>( ppTargets[i] ) ) ;
		}
		pImageArray->m_array.push_back( pImageObj ) ;
	}

	LQT_RETURN_OBJECT( pImageArray ) ;
}

// boolean setProjectionScreen( const Vector3* vScreen, double zScale, double fpPixelAspect )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_DOUBLE( zScale ) ;
	LQT_FUNC_ARG_DOUBLE( fpPixelAspect ) ;

	LQT_RETURN_BOOL
		( pRender->SetProjectionScreen
			( *vScreen, zScale, fpPixelAspect ) == sglErrSuccess ) ;
}

// boolean getProjectionScreen( Vector3* vScreen, double* zScale, double* fpPixelAspect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zScale ) ;
	LQT_VERIFY_NULL_PTR( zScale ) ;
	LQT_FUNC_ARG_POINTER( LDouble, fpPixelAspect ) ;
	LQT_VERIFY_NULL_PTR( fpPixelAspect ) ;

	LQT_RETURN_BOOL
		( pRender->GetProjectionScreen
			( *vScreen, *zScale, *fpPixelAspect ) == sglErrSuccess ) ;
}

// void setCamera( const Matrix3d* matCamera, const Vector3d* posCamera )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matCamera ) ;
	LQT_VERIFY_NULL_PTR( matCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posCamera ) ;
	LQT_VERIFY_NULL_PTR( posCamera ) ;

	pRender->SetCamera( *matCamera, *posCamera ) ;

	LQT_RETURN_VOID() ;
}

// void setCameraAngleVector( const Vector3d* posTarget, const Vector3d* posView, const Vector3d* vAngleTop )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setCameraAngleVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posTarget ) ;
	LQT_VERIFY_NULL_PTR( posTarget ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posView ) ;
	LQT_VERIFY_NULL_PTR( posView ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vAngleTop ) ;
	LQT_VERIFY_NULL_PTR( vAngleTop ) ;

	pRender->SetCameraAngleVector( *posTarget, *posView, *vAngleTop ) ;

	LQT_RETURN_VOID() ;
}

// void getCamera( Matrix3d* matCamera, Vector3d* posCamera ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matCamera ) ;
	LQT_VERIFY_NULL_PTR( matCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, posCamera ) ;
	LQT_VERIFY_NULL_PTR( posCamera ) ;

	pRender->GetCamera( *matCamera, *posCamera ) ;

	LQT_RETURN_VOID() ;
}

// boolean isSphereIntoView( const Vector3d* vPos, double radius ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_isSphereIntoView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_DOUBLE( radius ) ;

	LQT_RETURN_BOOL( pRender->IsSphereIntoView( *vPos, radius ) ) ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	pRender->SetParallax( xParallax, zFocusRate, xScreenDelta ) ;

	LQT_RETURN_VOID() ;
}

// double getParallax( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_DOUBLE( pRender->GetParallax() ) ;
}

// void setZClipRange( double zMin, double zMax )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setZClipRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_DOUBLE( zMin ) ;
	LQT_FUNC_ARG_DOUBLE( zMax ) ;

	pRender->SetZClipRange( zMin, zMax ) ;

	LQT_RETURN_VOID() ;
}

// void setLightEntries( const EntisGLS4.LightEntry* pLights, ulong countLight )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setLightEntries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_LightEntry, pLights, LQT_ARG_LONG(2) ) ;
	LQT_FUNC_ARG_ULONG( countLight ) ;

	if ( pLights != nullptr )
	{
		pRender->SetLightEntries( pLights, (size_t) countLight ) ;
	}
	else
	{
		pRender->SetLightEntries( nullptr, 0 ) ;
	}

	LQT_RETURN_VOID() ;
}

// void setShadowMap( uint idLight, EntisGLS4.Image shadowMapDepth, const EntisGLS4.ShadowMapInfo* infShadowMap, EntisGLS4.Image shadowMapColor )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setShadowMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_UINT( idLight ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, shadowMapDepth ) ;
	SGLImageObject *	pShadowMapDepth = nullptr ;
	if ( shadowMapDepth != nullptr )
	{
		pShadowMapDepth = shadowMapDepth->GetRef<SGLImageObject>() ;
	}
	LQT_VERIFY_NULL_PTR( pShadowMapDepth ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ShadowMapInfo, infShadowMap ) ;
	LQT_VERIFY_NULL_PTR( infShadowMap ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, shadowMapColor ) ;
	SGLImageObject *	pShadowMapColor = nullptr ;
	if ( shadowMapColor != nullptr )
	{
		pShadowMapColor = shadowMapColor->GetRef<SGLImageObject>() ;
	}

	pRender->SetShadowMap( idLight, pShadowMapColor, *infShadowMap, pShadowMapColor ) ;

	LQT_RETURN_VOID() ;
}

// void setFog( uint rgbFog, double zFogNear, double zFogFar )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_setFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_UINT( rgbFog ) ;
	LQT_FUNC_ARG_DOUBLE( zFogNear ) ;
	LQT_FUNC_ARG_DOUBLE( zFogFar ) ;

	pRender->SetFog( rgbFog, zFogNear, zFogFar ) ;

	LQT_RETURN_VOID() ;
}

// void enableFog( boolean fFog )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_enableFog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_BOOL( fFog ) ;

	pRender->EnableFog( fFog ) ;

	LQT_RETURN_VOID() ;
}

// int currentParallaxView( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_currentParallaxView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_INT( pRender->CurrentParallaxView() ) ;
}

// boolean selectParallaxView( int sviView )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_selectParallaxView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_INT( sviView ) ;

	LQT_RETURN_BOOL
		( pRender->SelectParallaxView
			( (S3DRenderContextInterface::StereoViewIndex) sviView ) == sglErrSuccess ) ;
}

// boolean begin3DRenderer( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_begin3DRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LQT_RETURN_BOOL( pRender->Begin3DRenderer( flags ) == sglErrSuccess ) ;
}

// boolean end3DRenderer( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_end3DRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LQT_RETURN_BOOL( pRender->End3DRenderer( flags ) == sglErrSuccess ) ;
}

// EntisGLS4.RenderDevice getRenderDeviceObject( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderContext_getRenderDeviceObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderContext, pThis ) ;
	S3DRenderContextInterface *	pRender = pThis->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	S3DRenderDevice *	pDevice = pRender->GetRenderDeviceObject( flags ) ;
	if ( pDevice == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>(pDevice) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



