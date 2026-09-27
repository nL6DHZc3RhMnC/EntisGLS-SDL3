
#include <loquaty.h>
#include "EntisGLS4_Environment.h"

using namespace Loquaty ;


// Environment( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Environment)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_Environment, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadEnvironment( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_Environment_loadEnvironment)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Environment, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadEnvironment(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



