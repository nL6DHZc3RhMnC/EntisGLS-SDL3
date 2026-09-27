
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_TextSprite.h>


// TextSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_TextSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_TextSprite, pThis,
			( new SSmartObject( new SGLSpriteText ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setTextStyle( const EntisGLS4.TextSprite.TextStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_TextSprite_setTextStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextSprite, pThis ) ;
	SGLSpriteText *	pSprite = pThis->GetRef<SGLSpriteText>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_TextSprite_TextStyle, styleObj ) ;
	LQT_VERIFY_NULL_PTR( styleObj ) ;

	SGLSpriteText::TextStyle	style ;
	LString						strFontFace, strProhibition ;
	GetLTextStyle( style, strFontFace, strProhibition, styleObj ) ;
	pSprite->SetTextStyle( style ) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.TextSprite.TextStyle getTextStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextSprite_getTextStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextSprite, pThis ) ;
	SGLSpriteText *	pSprite = pThis->GetRef<SGLSpriteText>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LClass *	pStyleClass =
		_context.VM().GetClassPathAs( L"EntisGLS4.TextSprite.TextStyle" ) ;
	if ( pStyleClass == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LObjPtr	valRet( pStyleClass->CreateInstance() ) ;
	SetLTextStyle( valRet, pSprite->GetTextStyle() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



