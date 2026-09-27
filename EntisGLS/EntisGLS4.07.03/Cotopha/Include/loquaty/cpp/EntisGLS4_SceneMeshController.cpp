
#include <loquaty.h>
#include "EntisGLS4_SceneMeshController.h"

using namespace Loquaty ;


// SceneMeshController( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneMeshController,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneMeshController>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}



