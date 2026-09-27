
#include <loquaty.h>
#include "EntisGLS4_TextSprite.h"

using namespace Loquaty ;


// TextSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_TextSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_TextSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setTextStyle( const EntisGLS4.TextSprite.TextStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_TextSprite_setTextStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextSprite, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_TextSprite_TextStyle, style ) ;
	LQT_VERIFY_NULL_PTR( style ) ;

	// pThis->setTextStyle(...) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.TextSprite.TextStyle getTextStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextSprite_getTextStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextSprite, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getTextStyle(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



