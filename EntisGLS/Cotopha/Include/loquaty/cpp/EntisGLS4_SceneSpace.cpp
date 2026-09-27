
#include <loquaty.h>
#include "EntisGLS4_SceneSpace.h"

using namespace Loquaty ;


// SceneSpace( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSpace)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneSpace, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean addChild( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSpace_addChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSpace, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LBoolean	valRet ;
	// valRet = pThis->addChild(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean removeChild( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSpace_removeChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSpace, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeChild(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



