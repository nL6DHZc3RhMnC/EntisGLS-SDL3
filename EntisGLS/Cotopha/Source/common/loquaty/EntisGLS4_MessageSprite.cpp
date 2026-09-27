
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_MessageSprite.h>


// MessageSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MessageSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_MessageSprite, pThis,
			( new SSmartObject( new SGLSpriteMessage ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setMessageStyle( const EntisGLS4.MessageSprite.MessageStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setMessageStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_MessageSprite_MessageStyle, styleObj ) ;
	LQT_VERIFY_NULL_PTR( styleObj ) ;

	SGLSpriteMessage::MessageStyle	style ;
	LString		strFontFace, strProhibition, strRubyFontFace ;
	GetLMessageStyle
		( style, strFontFace, strProhibition, strRubyFontFace, styleObj ) ;

	pSprite->SetMessageStyle( style ) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.MessageSprite.MessageStyle getMessageStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getMessageStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LClass *	pStyleClass =
		_context.VM().GetClassPathAs( L"EntisGLS4.MessageSprite.MessageStyle" ) ;
	if ( pStyleClass == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LObjPtr	valRet( pStyleClass->CreateInstance() ) ;
	SetLMessageStyle( valRet, pSprite->GetMessageStyle() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setCharacterAtlas( uint width, uint height )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setCharacterAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;

	pSprite->SetCharacterAtlas( width, height ) ;

	LQT_RETURN_VOID() ;
}

// void releaseCharacterAtlas( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_releaseCharacterAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->ReleaseCharacterAtlas() ;

	LQT_RETURN_VOID() ;
}

// void clearMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_clearMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->ClearMessage() ;

	LQT_RETURN_VOID() ;
}

// void flushMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_flushMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->FlushMessage() ;

	LQT_RETURN_VOID() ;
}

// boolean isMessagePending( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_isMessagePending)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsMessagePending() ) ;
}

// void setMessageSpeedRatio( uint fxSpeedRatio )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setMessageSpeedRatio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( fxSpeedRatio ) ;

	pSprite->SetMessageSpeedRatio( fxSpeedRatio ) ;

	LQT_RETURN_VOID() ;
}

// void addMessageText( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_addMessageText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	pSprite->AddMessageText( text.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void addMessageXML( String textXML )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_addMessageXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( textXML ) ;

	pSprite->AddMessageXML( textXML.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void verticalAlignmentMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_verticalAlignmentMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->VerticalAlignmentMessage() ;

	LQT_RETURN_VOID() ;
}

// const Point* getNextMessagePoint( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getNextMessagePoint)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LPoint	valRet = pSprite->GetNextMessagePoint() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const ImageRect* getCircumscribedRect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getCircumscribedRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLRect	rectMsg ;
	pSprite->GetCircumscribedRect( rectMsg ) ;

	LImageRect	valRet = rectMsg ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getMessageCharacterCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getMessageCharacterCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetMessageCharacterCount() ) ;
}

// String getPlainText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getPlainText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	SGLSpriteMessage *	pSprite = pThis->GetRef<SGLSpriteMessage>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_STRING( pSprite->GetPlainText() ) ;
}



