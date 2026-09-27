
#include <loquaty.h>
#include "EntisGLS4_EditSprite.h"

using namespace Loquaty ;


// EditSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_EditSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_EditSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setEditStyle( const EntisGLS4.EditSprite.ExitStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setEditStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_EditSprite_ExitStyle, style ) ;
	LQT_VERIFY_NULL_PTR( style ) ;

	// pThis->setEditStyle(...) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.EditSprite.ExitStyle getEditStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getEditStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getEditStyle(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getCursorIndex( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getCursorIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getCursorIndex(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void getSel( ulong* iSelFirst, ulong* iSelEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getSel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint64, iSelFirst ) ;
	LQT_VERIFY_NULL_PTR( iSelFirst ) ;
	LQT_FUNC_ARG_POINTER( LUint64, iSelEnd ) ;
	LQT_VERIFY_NULL_PTR( iSelEnd ) ;

	// pThis->getSel(...) ;

	LQT_RETURN_VOID() ;
}

// void setSel( long iFirst, long iEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setSel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_LONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	// pThis->setSel(...) ;

	LQT_RETURN_VOID() ;
}

// String getSelText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getSelText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getSelText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getRangeText( long iFirst, long iEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getRangeText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_LONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	LString	valRet ;
	// valRet = pThis->getRangeText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// boolean canCopyText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canCopyText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->canCopyText(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean canCutText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canCutText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->canCutText(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean canPasteText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canPasteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->canPasteText(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void doClear( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doClear)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->doClear(...) ;

	LQT_RETURN_VOID() ;
}

// void doCut( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doCut)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->doCut(...) ;

	LQT_RETURN_VOID() ;
}

// void doCopy( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doCopy)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->doCopy(...) ;

	LQT_RETURN_VOID() ;
}

// void doPaste( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doPaste)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->doPaste(...) ;

	LQT_RETURN_VOID() ;
}

// void doReplace( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doReplace)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	// pThis->doReplace(...) ;

	LQT_RETURN_VOID() ;
}

// boolean canUndo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canUndo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->canUndo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void undo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_undo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->undo(...) ;

	LQT_RETURN_VOID() ;
}

// boolean canRedo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canRedo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->canRedo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void redo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_redo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	// pThis->redo(...) ;

	LQT_RETURN_VOID() ;
}

// boolean doFindText( String text, EntisGLS4.EditSprite.FindTextFlag nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doFindText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_STRING( text ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->doFindText(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean doFindTextFrom( ulong iStartChar, String text, EntisGLS4.EditSprite.FindTextFlag nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doFindTextFrom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( iStartChar ) ;
	LQT_FUNC_ARG_STRING( text ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->doFindTextFrom(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// const Point* getCharPosFromIndex( ulong iChar ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getCharPosFromIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( iChar ) ;

	LPoint	valRet ;
	// valRet = pThis->getCharPosFromIndex(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getLineFromIndex( ulong iChar ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineFromIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( iChar ) ;

	LUint64	valRet ;
	// valRet = pThis->getLineFromIndex(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getLineIndex( ulong nLine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;

	LUint64	valRet ;
	// valRet = pThis->getLineIndex(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getLineLength( ulong nLine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;

	LUint64	valRet ;
	// valRet = pThis->getLineLength(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getLineCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getLineCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getLength(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getLineText( ulong nLine, ulong iOffset ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;
	LQT_FUNC_ARG_ULONG( iOffset ) ;

	LString	valRet ;
	// valRet = pThis->getLineText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// const Point* getScrollPos( ulong* pLineOffset ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getScrollPos_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pLineOffset ) ;
	LQT_VERIFY_NULL_PTR( pLineOffset ) ;

	LPoint	valRet ;
	// valRet = pThis->getScrollPos(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setScrollPos( int xPos, int yLine, int iLineOffset, int yOffset )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setScrollPos_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yLine ) ;
	LQT_FUNC_ARG_INT( iLineOffset ) ;
	LQT_FUNC_ARG_INT( yOffset ) ;

	// pThis->setScrollPos(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getMaxLineWidth( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getMaxLineWidth)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMaxLineWidth(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getLineHeight( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineHeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getLineHeight(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void scrollDeltaVertical( int yDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_scrollDeltaVertical)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	LQT_FUNC_ARG_INT( yDelta ) ;

	// pThis->scrollDeltaVertical(...) ;

	LQT_RETURN_VOID() ;
}



