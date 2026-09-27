
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RenderDevice_ShaderDesc.h>


// ShaderDesc( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_RenderDevice_ShaderDesc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_RenderDevice_ShaderDesc, pThis,
			( new SSmartObject( new S3DRenderDevice::ShaderDescriptor ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadDescription( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_ShaderDesc_loadDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, pThis ) ;
	S3DRenderDevice::ShaderDescriptor *
		pShdDsc = pThis->GetRef<S3DRenderDevice::ShaderDescriptor>() ;
	LQT_VERIFY_NULL_PTR( pShdDsc ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( path.c_str(), xmlDoc ) )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	SXMLDocument *	pxmlShader = xmlDoc.GetElementTagAs( L"shader" ) ;
	if ( pxmlShader == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_RETURN_BOOL( pShdDsc->ParseDescriptor( *pxmlShader ) == sglErrSuccess ) ;
}

// boolean readDescription( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_ShaderDesc_readDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, pThis ) ;
	S3DRenderDevice::ShaderDescriptor *
		pShdDsc = pThis->GetRef<S3DRenderDevice::ShaderDescriptor>() ;
	LQT_VERIFY_NULL_PTR( pShdDsc ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	SLoquatyFile	lfile( file ) ;
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( lfile, xmlDoc ) )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	SXMLDocument *	pxmlShader = xmlDoc.GetElementTagAs( L"shader" ) ;
	if ( pxmlShader == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_RETURN_BOOL( pShdDsc->ParseDescriptor( *pxmlShader ) == sglErrSuccess ) ;
}



