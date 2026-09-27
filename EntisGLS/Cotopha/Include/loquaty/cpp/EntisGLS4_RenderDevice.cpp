
#include <loquaty.h>
#include "EntisGLS4_RenderDevice.h"

using namespace Loquaty ;


// boolean isOnRenderThread( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_isOnRenderThread)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isOnRenderThread(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean procedure( Function<void()> func, EntisGLS4.RenderDevice.ProcedurePriority priority )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_procedure)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, func ) ;
	LQT_VERIFY_NULL_PTR( func ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	LBoolean	valRet ;
	// valRet = pThis->procedure(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean waitUntilAsyncAllProcedures( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_waitUntilAsyncAllProcedures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->waitUntilAsyncAllProcedures(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.RenderContext newRenderer( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_newRenderer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderContext) ) ) ;
	// valRet = pThis->newRenderer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_RenderContext> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderContext>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean commitImage( EntisGLS4.Image img, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_commitImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, img ) ;
	LQT_VERIFY_NULL_PTR( img ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->commitImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean commitVertexBuffer( EntisGLS4.VertexBuffer vb, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_commitVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->commitVertexBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean releaseImage( EntisGLS4.Image img, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_releaseImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, img ) ;
	LQT_VERIFY_NULL_PTR( img ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->releaseImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean releaseVertexBuffer( EntisGLS4.VertexBuffer vb, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_releaseVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->releaseVertexBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getDeviceFeatures( EntisGLS4.RenderDevice.Features* features )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_getDeviceFeatures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_RenderDevice_Features, features ) ;
	LQT_VERIFY_NULL_PTR( features ) ;

	LBoolean	valRet ;
	// valRet = pThis->getDeviceFeatures(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.CustomShader getCustomShaderAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_getCustomShaderAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	// valRet = pThis->getCustomShaderAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_CustomShader> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.CustomShader buildCustomShader( String id, const EntisGLS4.RenderDevice.ShaderDesc shddsc )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_buildCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, shddsc ) ;
	LQT_VERIFY_NULL_PTR( shddsc ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	// valRet = pThis->buildCustomShader(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_CustomShader> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeCustomShader( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_removeCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	// pThis->removeCustomShader(...) ;

	LQT_RETURN_VOID() ;
}



