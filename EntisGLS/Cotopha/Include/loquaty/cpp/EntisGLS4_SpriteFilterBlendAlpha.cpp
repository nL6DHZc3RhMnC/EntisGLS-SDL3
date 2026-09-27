
#include <loquaty.h>
#include "EntisGLS4_SpriteFilterBlendAlpha.h"

using namespace Loquaty ;


// SpriteFilterBlendAlpha( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterBlendAlpha)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadAlphaImage( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_loadAlphaImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadAlphaImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void attachAlphaImage( EntisGLS4.Image image )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_attachAlphaImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;

	// pThis->attachAlphaImage(...) ;

	LQT_RETURN_VOID() ;
}

// void setAlphaParameter( int fxAlphaCoefficient )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_setAlphaParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	LQT_FUNC_ARG_INT( fxAlphaCoefficient ) ;

	// pThis->setAlphaParameter(...) ;

	LQT_RETURN_VOID() ;
}



