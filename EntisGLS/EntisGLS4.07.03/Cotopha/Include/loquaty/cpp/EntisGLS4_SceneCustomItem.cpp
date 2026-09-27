
#include <loquaty.h>
#include "EntisGLS4_SceneCustomItem.h"

using namespace Loquaty ;


// SceneCustomItem( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneCustomItem,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneCustomItem>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}



