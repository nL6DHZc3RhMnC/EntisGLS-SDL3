
#include <loquaty.h>
#include "EntisGLS4_RectangleSprite.h"

using namespace Loquaty ;


// RectangleSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_RectangleSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_RectangleSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setRectangleSize( int w, int h )
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_setRectangleSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	LQT_FUNC_ARG_INT( w ) ;
	LQT_FUNC_ARG_INT( h ) ;

	// pThis->setRectangleSize(...) ;

	LQT_RETURN_VOID() ;
}

// const Size* getRectangleSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_getRectangleSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;

	LSize	valRet ;
	// valRet = pThis->getRectangleSize(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setRectangleColor( uint argb )
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_setRectangleColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	LQT_FUNC_ARG_UINT( argb ) ;

	// pThis->setRectangleColor(...) ;

	LQT_RETURN_VOID() ;
}

// uint getRectangleColor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_getRectangleColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getRectangleColor(...) ;

	LQT_RETURN_UINT( valRet ) ;
}



