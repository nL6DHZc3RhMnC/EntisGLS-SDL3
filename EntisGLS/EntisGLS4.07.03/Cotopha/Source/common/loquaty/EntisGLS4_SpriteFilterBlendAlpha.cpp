
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <loquaty/EntisGLS4_SpriteFilterBlendAlpha.h>


// SpriteFilterBlendAlpha( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterBlendAlpha)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteFilterBlendAlpha, pThis,
			( new SSmartObject( new SGLSpriteFilterBlendAlpha ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadAlphaImage( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_loadAlphaImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	SGLSpriteFilterBlendAlpha *	pFilter = pThis->GetRef<SGLSpriteFilterBlendAlpha>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LQT_RETURN_BOOL( pFilter->LoadAlphaImage( path.c_str() ) == sglErrSuccess ) ;
}

// void attachAlphaImage( EntisGLS4.Image image )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_attachAlphaImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	SGLSpriteFilterBlendAlpha *	pFilter = pThis->GetRef<SGLSpriteFilterBlendAlpha>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	pFilter->AttachAlphaImage( pImage ) ;

	LQT_RETURN_VOID() ;
}

// void setAlphaParameter( int fxAlphaCoefficient )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterBlendAlpha_setAlphaParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterBlendAlpha, pThis ) ;
	SGLSpriteFilterBlendAlpha *	pFilter = pThis->GetRef<SGLSpriteFilterBlendAlpha>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_INT( fxAlphaCoefficient ) ;

	pFilter->SetAlphaParameter( fxAlphaCoefficient ) ;

	LQT_RETURN_VOID() ;
}



