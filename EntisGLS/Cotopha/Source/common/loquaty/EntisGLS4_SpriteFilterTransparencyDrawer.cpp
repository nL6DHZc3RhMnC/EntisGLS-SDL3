
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <loquaty/EntisGLS4_SpriteFilterTransparencyDrawer.h>


// SpriteFilterTransparencyDrawer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterTransparencyDrawer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteFilterTransparencyDrawer, pThis,
			( new SSmartObject( new SGLSpriteFilterTransparencyDrawer ) ) ) ;

	LQT_RETURN_VOID() ;
}



