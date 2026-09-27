
#include <loquaty.h>
#include "EntisGLS4_RenderDevice_ShaderDesc.h"

using namespace Loquaty ;


// ShaderDesc( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_RenderDevice_ShaderDesc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadDescription( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_ShaderDesc_loadDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadDescription(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean readDescription( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderDevice_ShaderDesc_readDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderDevice_ShaderDesc, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->readDescription(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



