
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <loquaty/EntisGLS4_SpriteFilterShadingOff.h>


// SpriteFilterShadingOff( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterShadingOff)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteFilterShadingOff, pThis,
			( new SSmartObject( new SGLSpriteFilterShadingOff ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setTransitionOption( boolean flagTransition, uint argbOverColor )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterShadingOff_setTransitionOption)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterShadingOff, pThis ) ;
	SGLSpriteFilterShadingOff *	pFilter = pThis->GetRef<SGLSpriteFilterShadingOff>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_BOOL( flagTransition ) ;
	LQT_FUNC_ARG_UINT( argbOverColor ) ;

	pFilter->SetTransitionOption( flagTransition, argbOverColor ) ;

	LQT_RETURN_VOID() ;
}



