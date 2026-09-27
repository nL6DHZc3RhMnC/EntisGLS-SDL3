
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RectangleSprite.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>


// RectangleSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_RectangleSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_RectangleSprite, pThis,
			( new SSmartObject( new SGLSpriteRectangle ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setRectangleSize( int w, int h )
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_setRectangleSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	SGLSpriteRectangle *	pSprite = pThis->GetRef<SGLSpriteRectangle>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( w ) ;
	LQT_FUNC_ARG_INT( h ) ;

	pSprite->SetRectangleSize( w, h ) ;

	LQT_RETURN_VOID() ;
}

// const Size* getRectangleSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_getRectangleSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	SGLSpriteRectangle *	pSprite = pThis->GetRef<SGLSpriteRectangle>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LSize	valRet = pSprite->GetRectStyle().size ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setRectangleColor( uint argb )
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_setRectangleColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	SGLSpriteRectangle *	pSprite = pThis->GetRef<SGLSpriteRectangle>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( argb ) ;

	pSprite->SetRectangleColor( SGLPalette( argb ) ) ;

	LQT_RETURN_VOID() ;
}

// uint getRectangleColor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RectangleSprite_getRectangleColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RectangleSprite, pThis ) ;
	SGLSpriteRectangle *	pSprite = pThis->GetRef<SGLSpriteRectangle>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_UINT( pSprite->GetRectStyle().color.ui32 ) ;
}



