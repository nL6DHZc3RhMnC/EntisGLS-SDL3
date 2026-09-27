
#include <loquaty.h>
#include "EntisGLS4_SpriteFilterShadingOff.h"

using namespace Loquaty ;


// SpriteFilterShadingOff( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterShadingOff)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SpriteFilterShadingOff, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setTransitionOption( boolean flagTransition, uint argbOverColor )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterShadingOff_setTransitionOption)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterShadingOff, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagTransition ) ;
	LQT_FUNC_ARG_UINT( argbOverColor ) ;

	// pThis->setTransitionOption(...) ;

	LQT_RETURN_VOID() ;
}



