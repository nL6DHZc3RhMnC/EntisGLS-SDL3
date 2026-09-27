
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Environment.h>

using namespace Loquaty ;


// Environment( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Environment)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_Environment, pThis,
			( new SSmartObject
				( (SEnvironmentInterface*) new ECSSakura2::EnvironmentVM ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadEnvironment( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_Environment_loadEnvironment)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Environment, pThis ) ;
	ECSSakura2::EnvironmentVM *	pEnv = pThis->GetRef<ECSSakura2::EnvironmentVM>() ;
	LQT_VERIFY_NULL_PTR( pEnv ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( file.c_str(), SFileOpener::shareRead ) ;
	if ( pFile == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	
	LQT_RETURN_BOOL( pEnv->LoadEnvironment( *pFile ) == errSuccess ) ;
}



