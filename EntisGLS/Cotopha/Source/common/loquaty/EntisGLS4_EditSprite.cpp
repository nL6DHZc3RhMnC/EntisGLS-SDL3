
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_EditSprite.h>


// EditSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_EditSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_EditSprite, pThis,
			( new SSmartObject( new SGLSpriteEdit ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setEditStyle( const EntisGLS4.EditSprite.ExitStyle style )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setEditStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_EditSprite_ExitStyle, styleObj ) ;
	LQT_VERIFY_NULL_PTR( styleObj ) ;

	SGLSpriteEdit::EditStyle	style ;
	LString						 strFontFace, strProhibition, strIMEFontFace ;
	GetLEditStyle( style, strFontFace, strProhibition, strIMEFontFace, styleObj ) ;

	pSprite->SetEditStyle( style ) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.EditSprite.ExitStyle getEditStyle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getEditStyle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LClass *	pStyleClass =
		_context.VM().GetClassPathAs( L"EntisGLS4.EditSprite.ExitStyle" ) ;
	if ( pStyleClass == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LObjPtr	valRet( pStyleClass->CreateInstance() ) ;
	SetLEditStyle( valRet, pSprite->GetEditStyle() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getCursorIndex( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getCursorIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetCursorIndex() ) ;
}

// void getSel( ulong* iSelFirst, ulong* iSelEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getSel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pSelFirst ) ;
	LQT_VERIFY_NULL_PTR( pSelFirst ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pSelEnd ) ;
	LQT_VERIFY_NULL_PTR( pSelEnd ) ;

	size_t	iSelFirst, iSelEnd ;
	pSprite->GetSel( iSelFirst, iSelEnd ) ;

	*pSelFirst = (LUint64) iSelFirst ;
	*pSelEnd = (LUint64) iSelEnd ;

	LQT_RETURN_VOID() ;
}

// void setSel( long iFirst, long iEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setSel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_LONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	pSprite->SetSel( (ssize_t) iFirst, (ssize_t) iEnd ) ;

	LQT_RETURN_VOID() ;
}

// String getSelText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getSelText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_STRING( pSprite->GetSelText() ) ;
}

// String getRangeText( long iFirst, long iEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getRangeText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_LONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	LQT_RETURN_STRING( pSprite->GetRangeText( (ssize_t) iFirst, (ssize_t) iEnd ) ) ;
}

// boolean canCopyText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canCopyText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CanCopyText() ) ;
}

// boolean canCutText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canCutText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CanCutText() ) ;
}

// boolean canPasteText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canPasteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CanPasteText() ) ;
}

// void doClear( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doClear)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->DoClear() ;

	LQT_RETURN_VOID() ;
}

// void doCut( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doCut)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->DoCut() ;

	LQT_RETURN_VOID() ;
}

// void doCopy( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doCopy)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->DoCopy() ;

	LQT_RETURN_VOID() ;
}

// void doPaste( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doPaste)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->DoPaste() ;

	LQT_RETURN_VOID() ;
}

// void doReplace( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doReplace)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	pSprite->DoReplace( text.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// boolean canUndo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canUndo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CanUndo() ) ;
}

// void undo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_undo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->Undo() ;

	LQT_RETURN_VOID() ;
}

// boolean canRedo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_canRedo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CanRedo() ) ;
}

// void redo( )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_redo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->Redo() ;

	LQT_RETURN_VOID() ;
}

// boolean doFindText( String text, uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doFindText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( text ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL( pSprite->DoFindText( text.c_str(), nFlags ) ) ;
}

// boolean doFindTextFrom( ulong iStartChar, String text, uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_doFindTextFrom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( iStartChar ) ;
	LQT_FUNC_ARG_STRING( text ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL
		( pSprite->DoFindTextFrom( (size_t) iStartChar, text.c_str(), nFlags ) ) ;
}

// const Point* getCharPosFromIndex( ulong iChar ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getCharPosFromIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( iChar ) ;

	LPoint	valRet = pSprite->GetCharPosFromIndex( (size_t) iChar ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getLineFromIndex( ulong iChar ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineFromIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( iChar ) ;

	LQT_RETURN_ULONG( pSprite->GetLineFromIndex( (size_t) iChar ) ) ;
}

// ulong getLineIndex( ulong nLine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;

	LQT_RETURN_ULONG( pSprite->GetLineIndex( (size_t) nLine ) ) ;
}

// ulong getLineLength( ulong nLine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;

	LQT_RETURN_ULONG( pSprite->GetLineLength( (size_t) nLine ) ) ;
}

// ulong getLineCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetLineCount() ) ;
}

// ulong getLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetLength() ) ;
}

// String getLineText( ulong nLine, ulong iOffset ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nLine ) ;
	LQT_FUNC_ARG_ULONG( iOffset ) ;

	LQT_RETURN_STRING
		( pSprite->GetLineText( (size_t) nLine, (size_t) iOffset ) ) ;
}

// const Point* getScrollPos( ulong* pLineOffset ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getScrollPos_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pLineOffset ) ;

	size_t	nLineOffset ;
	LPoint	valRet = pSprite->GetScrollPos( &nLineOffset ) ;
	if ( pLineOffset != nullptr )
	{
		*pLineOffset = nLineOffset ;
	}

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setScrollPos( int xPos, int yLine, int iLineOffset, int yOffset )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_setScrollPos_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yLine ) ;
	LQT_FUNC_ARG_INT( iLineOffset ) ;
	LQT_FUNC_ARG_INT( yOffset ) ;

	pSprite->SetScrollPos( xPos, yLine, iLineOffset, yOffset ) ;

	LQT_RETURN_VOID() ;
}

// ulong getMaxLineWidth( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getMaxLineWidth)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetMaxLineWidth() ) ;
}

// ulong getLineHeight( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_getLineHeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetLineHeight() ) ;
}

// void scrollDeltaVertical( int yDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_EditSprite_scrollDeltaVertical)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_EditSprite, pThis ) ;
	SGLSpriteEdit *	pSprite = pThis->GetRef<SGLSpriteEdit>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( yDelta ) ;

	pSprite->ScrollDeltaVertical( yDelta ) ;

	LQT_RETURN_VOID() ;
}



