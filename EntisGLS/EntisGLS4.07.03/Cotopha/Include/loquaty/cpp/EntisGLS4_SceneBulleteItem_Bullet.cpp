
#include <loquaty.h>
#include "EntisGLS4_SceneBulleteItem_Bullet.h"

using namespace Loquaty ;


// Object getUserObject( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneBulleteItem_Bullet_getUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneBulleteItem_Bullet, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getUserObject(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



