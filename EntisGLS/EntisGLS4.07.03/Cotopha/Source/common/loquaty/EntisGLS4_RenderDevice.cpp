
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RenderDevice.h>


// boolean isOnRenderThread( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_isOnRenderThread)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;

	LQT_RETURN_BOOL( pDevice->IsOnRenderThread() ) ;
}

// boolean procedure( Function<void()> func, int priority )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_procedure)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, func ) ;
	LQT_VERIFY_NULL_PTR( func ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	LPtr<LTaskObj>	pTask( new LTaskObj( _context.VM().GetTaskClass() ) ) ;
	LProcedure *	pProc = new LProcedure( pTask, func ) ;

	LQT_RETURN_BOOL
		( pDevice->Procedure
			( pProc, (S3DRenderDevice::ProcedurePriority) priority ) == sglErrSuccess ) ;
}

// boolean waitUntilAsyncAllProcedures( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_waitUntilAsyncAllProcedures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pDevice->WaitUntilAsyncAllProcedures( msecTimeout ) == sglErrSuccess ) ;
}

// EntisGLS4.RenderContext newRenderer( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_newRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;

	S3DRenderContextInterface *	pRender = pDevice->NewRenderer() ;
	if ( pRender == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderContext) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_RenderContext>
			( new SSmartObject( (SGLPaintContextInterface*) pRender ) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean commitImage( EntisGLS4.Image img, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_commitImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, img ) ;
	LQT_VERIFY_NULL_PTR( img ) ;
	SGLImageObject *	pImage = img->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pDevice->CommitDeviceImage( pImage, msecTimeout ) == sglErrSuccess ) ;
}

// boolean commitVertexBuffer( EntisGLS4.VertexBuffer vb, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_commitVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	S3DVertexBufferInterface *	pVB = vb->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pDevice->CommitDeviceVertexBuffer( pVB, msecTimeout ) == sglErrSuccess ) ;
}

// boolean releaseImage( EntisGLS4.Image img, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_releaseImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, img ) ;
	LQT_VERIFY_NULL_PTR( img ) ;
	SGLImageObject *	pImage = img->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pDevice->ReleaseDeviceImage( pImage, msecTimeout ) == sglErrSuccess ) ;
}

// boolean releaseVertexBuffer( EntisGLS4.VertexBuffer vb, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_releaseVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	S3DVertexBufferInterface *	pVB = vb->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pDevice->ReleaseDeviceVertexBuffer( pVB, msecTimeout ) == sglErrSuccess ) ;
}

// boolean getDeviceFeatures( EntisGLS4.RenderDevice.Features* features )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_getDeviceFeatures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_RenderDevice_Features, features ) ;
	LQT_VERIFY_NULL_PTR( features ) ;

	LQT_RETURN_BOOL( pDevice->GetDeviceFeatures( *features ) == sglErrSuccess ) ;
}

// EntisGLS4.CustomShader getCustomShaderAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_getCustomShaderAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	S3DCustomShader *	pShader = pDevice->GetShaderProgramAs( id.c_str() ) ;
	if ( pShader == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>( pShader ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.CustomShader buildCustomShader( String id, const EntisGLS4.RenderDevice.ShaderDesc shddsc )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_buildCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, shddsc ) ;
	LQT_VERIFY_NULL_PTR( shddsc ) ;
	S3DRenderDevice::ShaderDescriptor *
		pShdDsc = shddsc->GetRef<S3DRenderDevice::ShaderDescriptor>() ;
	LQT_VERIFY_NULL_PTR( pShdDsc ) ;

	S3DCustomShader *	pShader = pDevice->BuildCustomShader( id.c_str(), pShdDsc ) ;
	if ( pShader == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>(pShader) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeCustomShader( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_removeCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	S3DRenderDevice *	pDevice = pThis->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	pDevice->RemoveCustomShaderAs( id.c_str() ) ;

	LQT_RETURN_VOID() ;
}



