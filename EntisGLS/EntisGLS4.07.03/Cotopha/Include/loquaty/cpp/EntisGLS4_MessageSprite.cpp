
#include <loquaty.h>
#include "EntisGLS4_MessageSprite.h"

using namespace Loquaty ;


// MessageSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MessageSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_MessageSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setMessageStyle( const EntisGLS4.MessageSprite.MessageStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setMessageStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_MessageSprite_MessageStyle, style ) ;
	LQT_VERIFY_NULL_PTR( style ) ;

	// pThis->setMessageStyle(...) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.MessageSprite.MessageStyle getMessageStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getMessageStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getMessageStyle(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setCharacterAtlas( uint width, uint height )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setCharacterAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;

	// pThis->setCharacterAtlas(...) ;

	LQT_RETURN_VOID() ;
}

// void releaseCharacterAtlas( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_releaseCharacterAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	// pThis->releaseCharacterAtlas(...) ;

	LQT_RETURN_VOID() ;
}

// void clearMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_clearMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	// pThis->clearMessage(...) ;

	LQT_RETURN_VOID() ;
}

// void flushMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_flushMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	// pThis->flushMessage(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isMessagePending( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_isMessagePending)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isMessagePending(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setMessageSpeedRatio( uint fxSpeedRatio )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_setMessageSpeedRatio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	LQT_FUNC_ARG_UINT( fxSpeedRatio ) ;

	// pThis->setMessageSpeedRatio(...) ;

	LQT_RETURN_VOID() ;
}

// void addMessageText( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_addMessageText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	// pThis->addMessageText(...) ;

	LQT_RETURN_VOID() ;
}

// void addMessageXML( String textXML )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_addMessageXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;
	LQT_FUNC_ARG_STRING( textXML ) ;

	// pThis->addMessageXML(...) ;

	LQT_RETURN_VOID() ;
}

// void verticalAlignmentMessage( )
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_verticalAlignmentMessage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	// pThis->verticalAlignmentMessage(...) ;

	LQT_RETURN_VOID() ;
}

// const Point* getNextMessagePoint( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getNextMessagePoint)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LPoint	valRet ;
	// valRet = pThis->getNextMessagePoint(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const ImageRect* getCircumscribedRect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getCircumscribedRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LImageRect	valRet ;
	// valRet = pThis->getCircumscribedRect(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getMessageCharacterCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getMessageCharacterCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMessageCharacterCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getPlainText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MessageSprite_getPlainText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MessageSprite, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getPlainText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}



