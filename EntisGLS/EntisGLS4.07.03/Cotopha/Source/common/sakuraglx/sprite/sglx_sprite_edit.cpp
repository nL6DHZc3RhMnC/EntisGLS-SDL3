
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_edit.h>
#include <sakuraglx/sglx_platform_ui.h>
#include <sakuragl/window/sgl_window_menu.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// エディットテキストリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteEditListener, SObject )

// ユーザー入力によってテキストが変更された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditListener::OnChangedText( SGLSpriteEdit& edit )
{
}

// 選択範囲・カーソルが移動した
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditListener::OnMovedCursor( SGLSpriteEdit& edit )
{
}

// スクロールした
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditListener::OnScrolled( SGLSpriteEdit& edit )
{
}

// テキスト入力フィルタ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditListener::FilterInputText
	( SGLSpriteEdit& edit, SSystem::SString& strText )
{
}


//////////////////////////////////////////////////////////////////////////////
// エディット・テキスト・スプライト・スタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::EditStyle::EditStyle( void )
{
	nEditFlags = editMultiLine | editLineWrap | editAcceptReturn | editAcceptTab ;
	nCaretWidth = 2 ;
	nCaretBlinkInterval = 1000 ;
	rgbaCaretColor.ui32 = 0xFFFFFFFF ;
	rgbaSelBackColor.ui32 = 0xFFFFFFFF ;
	decoSel.rgbaBody.ui32 = 0xFF000000 ;
	fontIME.nStyles = 0 ;
	fontIME.nSize = 16 ;
	fontIME.pszFace = SGLFontStyle::StandardFont ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::EditStyle::EditStyle( const SGLSpriteEdit::EditStyle& style )
{
	memmove( this, &style, sizeof(EditStyle) ) ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteEdit::EditStyle&
	SGLSpriteEdit::EditStyle::operator =
		( const SGLSpriteEdit::EditStyle& style )
{
	memmove( this, &style, sizeof(EditStyle) ) ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// エディット・テキスト・ブックマーク
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteEdit::Bookmark, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// エディット・テキスト・表示用グラフィック
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::LineView::LineView( void )
{
	m_fUpdate = false ;
	m_iFirstChar = 0 ;
}

SGLSpriteEdit::LineView::LineView( const SGLSpriteEdit::LineView& lv )
	: m_imgView( lv.m_imgView ),
		m_ptView( lv.m_ptView ), m_ptOffset( lv.m_ptOffset ),
		m_sizeView( lv.m_sizeView ), m_iFirstChar( lv.m_iFirstChar ),
		m_arrCharRects( lv.m_arrCharRects )
{
}

// 行幅（ピクセル）取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::LineView::GetLineWidth( void ) const
{
	SGLImageRect *	pRect = m_arrCharRects.GetLastAt() ;
	if ( pRect == nullptr )
	{
		return	0 ;
	}
	return	pRect->x + pRect->w ;
}

// 指定文字の m_imgView 内表示位置取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::LineView::CharRectInView( SGLImageRect& rect, size_t i ) const
{
	SGLImageRect *	pRect = m_arrCharRects.GetAt( i ) ;
	if ( pRect == nullptr )
	{
		return	false ;
	}
	rect.x = pRect->x - m_ptOffset.x ;
	rect.y = pRect->y - m_ptOffset.y ;
	rect.w = pRect->w ;
	rect.h = pRect->h ;
	return	true ;
}


//////////////////////////////////////////////////////////////////////////////
// アンドゥ・リドゥ用記録
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SGLSpriteEdit::UndoObject, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( SGLSpriteEdit::UndoRecord, UndoObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::UndoRecord::UndoRecord( void )
{
	m_iFirst = 0 ;
	m_iEnd = 0 ;
}

SGLSpriteEdit::UndoRecord::UndoRecord( const SGLSpriteEdit::UndoRecord& undo )
{
	m_iFirst = undo.m_iFirst ;
	m_iEnd = undo.m_iEnd ;
	m_strText = undo.m_strText ;
}

SGLSpriteEdit::UndoRecord::UndoRecord( int iFirst, int iEnd, const wchar_t * pwszText )
{
	m_iFirst = iFirst ;
	m_iEnd = iEnd ;
	m_strText = pwszText ;
}

// アンドゥ実行
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::UndoObject * SGLSpriteEdit::UndoRecord::Undo( SGLSpriteEdit& edit )
{
	UndoRecord *	pRedo = new UndoRecord ;
	edit.SetSel( (ssize_t) m_iFirst, (ssize_t) m_iEnd ) ;
	edit.ReplaceSelText( m_strText, pRedo ) ;
	//
	SGLSpriteEditListener *	pListener = edit.m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnChangedText( edit ) ;
	}
	return	pRedo ;
}

// リドゥ実行
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::UndoObject * SGLSpriteEdit::UndoObject::Redo( SGLSpriteEdit& edit )
{
	return	Undo( edit ) ;
}



//////////////////////////////////////////////////////////////////////////////
// エディット・テキスト・スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteEdit, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::SGLSpriteEdit( void )
	: m_yViewLine( 0 ), m_iViewLineOffset( 0 ),
		m_xScroll( 0 ), m_yScrollOffset( 0 ),
		m_iSelFirst( 0 ), m_iSelEnd( 0 ),
		m_iCursor( 0 ), m_limitUndo( 0x100 )
{
	m_flagsUI |= uiFocusable ;
	//
	m_msecCaret = 0 ;
	m_fMouseLDown = false ;
	m_fMouseLDblClk = false ;
}

SGLSpriteEdit::SGLSpriteEdit( const SGLSpriteEdit& src )
	: SGLSprite( src ), m_strFontFace( src.m_strFontFace ),
		m_styleEdit( src.m_styleEdit ), m_strEdit( src.m_strEdit ),
		m_indexLine( src.m_indexLine ),
		m_arrLineViewBuf( src.m_arrLineViewBuf ),
		m_yViewLine( src.m_yViewLine ),
		m_iViewLineOffset( src.m_iViewLineOffset ),
		m_xScroll( src.m_xScroll ), m_yScrollOffset( src.m_yScrollOffset ),
		m_iSelFirst( src.m_iSelFirst ), m_iSelEnd( src.m_iSelEnd ),
		m_iCursor( src.m_iCursor ), m_msecCaret( src.m_msecCaret ),
		m_limitUndo( src.m_limitUndo )
{
	m_styleEdit.font.pszFace = m_strFontFace ;
	//
	m_fMouseLDown = false ;
	m_fMouseLDblClk = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::~SGLSpriteEdit( void )
{
	DetachSyncTimeout( 100 ) ;
}

// テキストスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetEditStyle( const SGLSpriteEdit::EditStyle& style )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_styleEdit = style ;
	m_strFontFace = style.font.pszFace ;
	m_styleEdit.font.pszFace = m_strFontFace ;
	//
	if ( style.nEditFlags & editFontForIME )
	{
		m_strIMEFontFace = style.fontIME.pszFace ;
		m_styleEdit.fontIME.pszFace = m_strIMEFontFace ;
	}
	//
	PostUpdateAllLines() ;
	UpdateTextImage() ;
	PostUpdate() ;
	Unlock() ;
}

// 項目名設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetEditItemName( const wchar_t * pwszName )
{
	m_strItemName = pwszName ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::AttachEditListener
			( SGLSpriteEditListener * pListener )
{
	m_refEditListener = pListener ;
}

// リスナ取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEditListener * SGLSpriteEdit::GetEditListener( void ) const
{
	return	m_refEditListener ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteEdit::GetText( void ) const
{
	return	m_strEdit ;
}

void SGLSpriteEdit::SetText( const wchar_t * pwszText )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_arrUndo.RemoveAll() ;
	m_arrRedo.RemoveAll() ;
	m_strEdit = pwszText ;
	PostUpdateAllLines() ;
	UpdateTextIndex() ;
	UpdateTextImage() ;
	SetSel( 0, 0 ) ;
	PostUpdate() ;
	//
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnChangedText( *this ) ;
	}
	Unlock() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetTextFont
	( const wchar_t * pwszFont, int nSize )
{
	m_strFontFace = pwszFont ;
	SGLSpriteText::SelectValidFont( m_strFontFace ) ;
	m_styleEdit.font.pszFace = m_strFontFace ;
	if ( nSize != 0 )
	{
		m_styleEdit.font.nSize = nSize ;
	}
	UpdateTextImage() ;
	PostUpdate() ;
}

// エディットスタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::ParseTextStyle
	( SGLSpriteEdit::EditStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strIMEFontFace,
		const SSystem::SXMLDocument& xmlStyle )
{
	SGLSpriteText::ParseTextStyle( style, strFontFace, xmlStyle ) ;
	//
	// <sel_text>
	SXMLDocument *	pxmlSelText = xmlStyle.GetElementTagAs( L"sel_text" ) ;
	style.decoSel.nFlags = 0 ;
	style.decoSel.widthBorder = 0 ;
	if ( pxmlSelText != nullptr )
	{
		SGLSpriteText::ParseTextColor( style.decoSel.rgbaBody, *pxmlSelText ) ;
	}
	//
	// <edit>
	SXMLDocument *	pxmlEdit = xmlStyle.GetElementTagAs( L"edit" ) ;
	if ( pxmlEdit != nullptr )
	{
		static const wchar_t *	pwszFlagNames[] =
		{
			L"single_line", L"multi_line",
			L"line_wrap", L"accept_return", L"accept_tab",
			L"auto_indent", L"multi_line_tab", L"read_only",
			L"deny_alphabet", L"deny_number", L"deny_8bit_char",
			L"deny_mb_char", L"style_number", L"style_password",
			L"column_caret", L"underbar_caret", nullptr
		} ;
		static const uint32_t	nFlagValues[] =
		{
			editSingleLine, editMultiLine,
			editLineWrap, editAcceptReturn, editAcceptTab,
			editAutoIndent, editMultiLineTab, editReadOnly,
			editDenyAlphabet, editDenyNumber, editDeny8bitChar,
			editDenyMBChar, editNumber, editPassword,
			editColumnCaret, editUnderbarCaret,
		} ;
		SStringParser	sparsFlags ;
		SString	strFlags = pxmlEdit->GetAttrStringAs( L"flags", nullptr ) ;
		sparsFlags.AttachString( strFlags ) ;
		style.nEditFlags = 0 ;
		while ( sparsFlags.PassSpace() )
		{
			SString	strTerm ;
			if ( sparsFlags.NextString( strTerm ) )
			{
				for ( size_t i = 0; pwszFlagNames[i]; i ++ )
				{
					if ( strTerm == pwszFlagNames[i] )
					{
						style.nEditFlags |= nFlagValues[i] ;
						break ;
					}
				}
			}
		}
		style.nCaretWidth =
			(uint32_t) pxmlEdit->GetAttrIntegerAs( L"caret_width", 1 ) ;
		style.nCaretBlinkInterval =
			(uint32_t) pxmlEdit->GetAttrIntegerAs( L"caret_interval", 1000 ) ;
		style.rgbaCaretColor.ui32 =
			(uint32_t) pxmlEdit->GetAttrRichIntegerAs
							( L"caret_color", style.rgbaCaretColor.ui32 ) ;
		style.rgbaSelBackColor.ui32 =
			(uint32_t) pxmlEdit->GetAttrRichIntegerAs
						( L"sel_back_color", style.rgbaSelBackColor.ui32 ) ;
	}
	//
	// <ime_font>
	SXMLDocument *	pxmlImeFont = xmlStyle.GetElementTagAs( L"ime_font" ) ;
	if ( pxmlImeFont != nullptr )
	{
		style.nEditFlags |= editFontForIME ;
		SGLSpriteText::ParseFontStyle
			( style.fontIME, strIMEFontFace, *pxmlImeFont ) ;
	}
}

void SGLSpriteEdit::ParseTextStyle_CompatibleGLS3
	( SGLSpriteEdit::EditStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strIMEFontFace,
		const SSystem::SXMLDocument& xmlStyle )
{
	// <arrange>
	SXMLDocument *	pxmlArrange = xmlStyle.GetElementTagAs( L"arrange" ) ;
	style.nEditFlags = SGLSpriteEdit::editMultiLine
						| SGLSpriteEdit::editAcceptReturn
						| SGLSpriteEdit::editAcceptTab ;
	if ( pxmlArrange != nullptr )
	{
		SString	strType = pxmlArrange->GetAttrStringAs( L"type", nullptr ) ;
		if ( strType == L"single" )
		{
			style.nEditFlags &=
				~(SGLSpriteEdit::editMultiLine
					| SGLSpriteEdit::editAcceptReturn) ;
			style.nEditFlags |= SGLSpriteEdit::editSingleLine ;
		}
		else if ( strType == L"multiline" )
		{
			style.nEditFlags |= SGLSpriteEdit::editMultiLine ;
		}
	}
	// <font>
	SXMLDocument *	pxmlFont = xmlStyle.GetElementTagAs( L"font" ) ;
	if ( pxmlFont != nullptr )
	{
		if ( style.font.nSize == 0 )
		{
			style.font.nSize = 16 ;
		}
		SGLSpriteText::ParseFontStyle( style.font, strFontFace, *pxmlFont ) ;
	}
	// <caret>
	SXMLDocument *	pxmlCaret = xmlStyle.GetElementTagAs( L"caret" ) ;
	if ( pxmlCaret != nullptr )
	{
		int	wCaret = (int) pxmlCaret->GetAttrRichIntegerAs( L"width", 0 ) ;
		int	hCaret = (int) pxmlCaret->GetAttrRichIntegerAs( L"height", 0 ) ;
		style.nEditFlags |= SGLSpriteEdit::editColumnCaret ;
		if ( wCaret > 0 )
		{
			style.nCaretWidth = (uint32_t) wCaret ;
		}
		style.nCaretBlinkInterval =
			(uint32_t) pxmlCaret->GetAttrRichIntegerAs( L"interval", 0 ) ;
		style.rgbaCaretColor.ui32 =
			(uint32_t) pxmlCaret->GetAttrRichIntegerAs
					( L"color", style.rgbaCaretColor.ui32 ) | 0xFF000000 ;
	}
	// <edit>
	SXMLDocument *	pxmlEdit = xmlStyle.GetElementTagAs( L"edit" ) ;
	style.context.rectWritable.left = 0 ;
	style.context.rectWritable.top = 0 ;
	style.context.rectWritable.right = 0x7FFF ;
	style.context.rectWritable.bottom = 0x7FFF ;
	style.rgbaSelBackColor = style.rgbaCaretColor ;
	style.decoSel.nFlags = 0 ;
	style.decoSel.widthBorder = 0 ;
	if ( pxmlEdit != nullptr )
	{
		style.context.pitchLine =
			(uint32_t) pxmlEdit->GetAttrRichIntegerAs( L"bottom", 15 ) + 1 ;
		style.decoration.rgbaBody.ui32 =
			(uint32_t) pxmlEdit->GetAttrRichIntegerAs
				( L"color", style.decoration.rgbaBody.ui32 ) | 0xFF000000 ;
		style.decoSel.rgbaBody.ui32 =
			(uint32_t) pxmlEdit->GetAttrRichIntegerAs
				( L"sel_color", style.decoSel.rgbaBody.ui32 ) | 0xFF000000 ;
	}
	// <ime_font>
	SXMLDocument *	pxmlImeFont = xmlStyle.GetElementTagAs( L"ime_font" ) ;
	if ( pxmlImeFont != nullptr )
	{
		if ( style.fontIME.nSize == 0 )
		{
			style.fontIME.nSize = 16 ;
		}
		SGLSpriteText::ParseFontStyle
			( style.fontIME, strIMEFontFace, *pxmlImeFont ) ;
	}
}

// カーソル位置取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetCursorIndex( void ) const
{
	return	m_iCursor ;
}

// 選択範囲取得
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::GetSel( size_t & iSelFirst, size_t & iSelEnd ) const
{
	iSelFirst = m_iSelFirst ;
	iSelEnd = m_iSelEnd ;
}

// 選択範囲設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetSel( ssize_t iFirst, ssize_t iEnd )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( iFirst < 0 )
	{
		iFirst = (ssize_t) m_strEdit.GetLength() ;
	}
	if ( iEnd < 0 )
	{
		iEnd = (ssize_t) m_strEdit.GetLength() ;
	}
	SetUpdateRange( m_iSelFirst, m_iSelEnd, 0, false ) ;
	m_iCursor = (size_t) iEnd ;
	if ( m_iCursor > m_strEdit.GetLength() )
	{
		m_iCursor = m_strEdit.GetLength() ;
	}
	if ( iFirst > iEnd )
	{
		ssize_t	t = iFirst ;
		iFirst = iEnd ;
		iEnd = t ;
	}
	if ( (size_t) iFirst < m_strEdit.GetLength() )
	{
		m_iSelFirst = (size_t) iFirst ;
		m_iSelEnd = m_iSelFirst ;
		if ( iEnd > iFirst )
		{
			if ( (size_t) iEnd < m_strEdit.GetLength() )
			{
				m_iSelEnd = (size_t) iEnd ;
			}
			else
			{
				m_iSelEnd = m_strEdit.GetLength() ;
			}
		}
	}
	else
	{
		m_iSelFirst = m_strEdit.GetLength() ;
		m_iSelEnd = m_iSelFirst ;
	}
	m_msecCaret = 0 ;
	SetUpdateRange( m_iSelFirst, m_iSelEnd, 0, false ) ;
	UpdateTextImage() ;
	TrackCharacterFor( m_iCursor ) ;
	//
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnMovedCursor( *this ) ;
	}
	Unlock() ;
}

// 選択範囲の文字列を取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteEdit::GetSelText( void ) const
{
	return	GetRangeText( (ssize_t) m_iSelFirst, (ssize_t) m_iSelEnd ) ;
}

// 指定範囲の文字列を取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteEdit::GetRangeText( ssize_t iFirst, ssize_t iEnd ) const
{
	if ( iFirst < iEnd )
	{
		return	m_strEdit.Middle( iFirst, iEnd - iFirst ) ;
	}
	return	SString() ;
}

// 文字コピー可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::CanCopyText( void ) const
{
	return	(m_iSelFirst < m_iSelEnd)
			|| (m_iSelFirst < m_strEdit.GetLength()) ;
}

// 文字切り取り可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::CanCutText( void ) const
{
	return	(m_iSelFirst < m_iSelEnd)
			&& !(m_styleEdit.nEditFlags & editReadOnly) ;
}

// 文字列貼り付け可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::CanPasteText( void ) const
{
	if ( !UI::Clipboard::HasPlaneText() )
	{
		return	false ;
	}
	return	!(m_styleEdit.nEditFlags & editReadOnly) ;
}

// 選択文字列削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DoClear( void )
{
	if ( !(m_styleEdit.nEditFlags & editReadOnly) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		UndoRecord *	pUndo = new UndoRecord ;
		ClearSelText( pUndo ) ;
		RecordUndo( pUndo ) ;
		//
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnChangedText( *this ) ;
		}
		Unlock() ;
	}
}

void SGLSpriteEdit::ClearSelText( SGLSpriteEdit::UndoRecord * pUndo )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( pUndo != nullptr )
	{
		pUndo->m_iFirst = m_iSelFirst ;
		pUndo->m_iEnd = m_iSelFirst ;
		pUndo->m_strText = GetSelText() ;
	}
	SetUpdateRange
		( m_iSelFirst, m_iSelEnd,
			(ssize_t) m_iSelFirst - (ssize_t) m_iSelEnd, true ) ;
	m_strEdit = m_strEdit.Left( m_iSelFirst )
						+ m_strEdit.Middle( m_iSelEnd ) ;
	UpdateTextIndex() ;
	SetSel( (ssize_t) m_iSelFirst, (ssize_t) m_iSelFirst ) ;
	Unlock() ;
}

// 選択文字列切り取り
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DoCut( void )
{
	if ( !(m_styleEdit.nEditFlags & editReadOnly) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		UndoRecord *	pUndo = new UndoRecord ;
		CutSelText( pUndo ) ;
		RecordUndo( pUndo ) ;
		//
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnChangedText( *this ) ;
		}
		Unlock() ;
	}
}

void SGLSpriteEdit::CutSelText( SGLSpriteEdit::UndoRecord * pUndo )
{
	LockTrace( __FILE__, __LINE__ ) ;
	CopySelText( ) ;
	ClearSelText( pUndo ) ;
	Unlock() ;
}

// 選択文字列コピー
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DoCopy( void )
{
	CopySelText() ;
}

void SGLSpriteEdit::CopySelText( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	//
	// 選択文字列取得
	//
	SString	strSelText = GetSelText() ;
	if ( strSelText.IsEmpty() )
	{
		strSelText = GetLineText( GetLineFromIndex( m_iSelFirst ) ) ;
	}
	//
	// クリップボードに保存
	//
	UI::Clipboard::SetPlaneText( strSelText ) ;
	Unlock() ;
}

// 選択文字列貼りつけ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DoPaste( void )
{
	if ( !(m_styleEdit.nEditFlags & editReadOnly) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		UndoRecord *	pUndo = new UndoRecord ;
		PasteSelText( pUndo ) ;
		RecordUndo( pUndo ) ;
		//
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnChangedText( *this ) ;
		}
		Unlock() ;
	}
}

void SGLSpriteEdit::PasteSelText( SGLSpriteEdit::UndoRecord * pUndo )
{
	//
	// クリップボードから文字列取得
	//
	SString	strClipboardText ;
	bool	fClipboardText =
				UI::Clipboard::GetPlaneText( strClipboardText );
	//
	// 貼り付け
	//
	if ( fClipboardText )
	{
		FilterInputText( strClipboardText ) ;
		ReplaceSelText( strClipboardText, pUndo ) ;
	}
}

// 選択文字列置き換え
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DoReplace( const wchar_t * pwszText )
{
	if ( !(m_styleEdit.nEditFlags & editReadOnly) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		UndoRecord *	pUndo = new UndoRecord ;
		ReplaceSelText( pwszText, pUndo ) ;
		RecordUndo( pUndo ) ;
		//
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnChangedText( *this ) ;
		}
		Unlock() ;
	}
}

void SGLSpriteEdit::ReplaceSelText
	( const wchar_t * pwszText, SGLSpriteEdit::UndoRecord * pUndo )
{
	//
	// 選択文字列を削除
	//
	ClearSelText( pUndo ) ;
	//
	// 文字列挿入
	//
	LockTrace( __FILE__, __LINE__ ) ;
	ESLAssert( m_iSelFirst == m_iSelEnd ) ;
	SString	strText = pwszText ;
	size_t	iSelEnd = m_iSelFirst + strText.GetLength() ;
	SetUpdateRange
		( m_iSelFirst, m_iSelEnd,
			(ssize_t) m_iSelFirst
				- (ssize_t) m_iSelEnd + (ssize_t) strText.GetLength(), true ) ;
	m_strEdit = m_strEdit.Left( m_iSelFirst )
						+ strText + m_strEdit.Middle( m_iSelEnd ) ;
	UpdateTextIndex() ;
	UpdateTextImage() ;
	//
	// カーソル位置
	//
	SetSel( (ssize_t) iSelEnd, (ssize_t) iSelEnd ) ;
	if ( pUndo != nullptr )
	{
		pUndo->m_iEnd = iSelEnd ;
	}
	m_xCursor = -1 ;
	Unlock() ;
}

// UNDO 可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::CanUndo( void )
{
	return	(m_arrUndo.GetLength() != 0)
			&& !(m_styleEdit.nEditFlags & editReadOnly) ;
}

// UNDO 実行
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::Undo( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	UndoObject *	pUndo = m_arrUndo.GetLastAt() ;
	if ( pUndo != nullptr )
	{
		pUndo = m_arrUndo.Pop() ;
		ESLAssert( pUndo != nullptr ) ;
		//
		UndoObject *	pRedo = pUndo->Undo( *this ) ;
		m_arrRedo.Add( pRedo ) ;
	}
	Unlock() ;
}

// REDO 可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::CanRedo( void )
{
	return	(m_arrRedo.GetLength() != 0)
			&& !(m_styleEdit.nEditFlags & editReadOnly) ;
}

// REDO 実行
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::Redo( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	UndoObject *	pRedo = m_arrRedo.GetLastAt() ;
	if ( pRedo != nullptr )
	{
		pRedo = m_arrRedo.Pop() ;
		ESLAssert( pRedo != nullptr ) ;
		//
		UndoObject *	pUndo = pRedo->Redo( *this ) ;
		m_arrUndo.Add( pUndo ) ;
	}
	Unlock() ;
}

// 文字検索
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::DoFindText
	( const wchar_t * pwszText, uint32_t nFlags )
{
	size_t	iChar = m_iSelEnd ;
	if ( nFlags & findUp )
	{
		iChar = m_iSelFirst ;
	}
	return	DoFindTextFrom( iChar, pwszText, nFlags ) ;
}

bool SGLSpriteEdit::DoFindTextFrom
	( size_t iStartChar,
		const wchar_t * pwszText, uint32_t nFlags )
{
	if ( (pwszText == nullptr) || (pwszText[0] == 0) )
	{
		return	false ;
	}
	size_t	nLength = SString::GetLength( pwszText ) ;
	size_t	iNextChar = iStartChar ;
	for ( ; ; )
	{
		if ( nFlags & findUp )
		{
			if ( iNextChar == 0 )
			{
				return	false ;
			}
			iNextChar -- ;
		}
		else
		{
			if ( iNextChar >= m_strEdit.GetLength() )
			{
				return	false ;
			}
			iNextChar ++ ;
		}
		size_t	iFirstWord, iEndWord ;
		if ( nFlags & findWholeWord )
		{
			FindWordRange( iFirstWord, iEndWord, iNextChar ) ;
			if ( nFlags & findUp )
			{
				iNextChar = iFirstWord ;
			}
			else
			{
				iNextChar = iEndWord ;
			}
		}
		bool	fMatch = true ;
		for ( size_t i = 0; pwszText[i]; i ++ )
		{
			wchar_t	wch = m_strEdit.GetAt( iNextChar + i ) ;
			if ( wch != pwszText[i] )
			{
				if ( nFlags & findNoCase )
				{
					wchar_t	wchMatch = pwszText[i] ;
					if ( (wch >= L'a') && (wch <= L'z') )
					{
						wch -= L'a' - L'A' ;
					}
					if ( (wchMatch >= L'a') && (wchMatch <= L'z') )
					{
						wchMatch -= L'a' - L'A' ;
					}
					if ( wch != wchMatch )
					{
						fMatch = false ;
						break ;
					}
				}
				else
				{
					fMatch = false ;
					break ;
				}
			}
		}
		if ( fMatch )
		{
			if ( nFlags & findWholeWord )
			{
				FindWordRange( iFirstWord, iEndWord, iNextChar ) ;
				if ( (iFirstWord != iNextChar)
					|| (iNextChar + nLength != iEndWord) )
				{
					fMatch = false ;
				}
			}
		}
		if ( fMatch )
		{
			if ( nFlags & findUp )
			{
				SetSel( (ssize_t) (iNextChar + nLength),
						(ssize_t) iNextChar ) ;
			}
			else
			{
				SetSel( (ssize_t) iNextChar,
						(ssize_t) (iNextChar + nLength) ) ;
			}
			return	true ;
		}
	}
	return	false ;
}

// 文字指標から座標を計算
//////////////////////////////////////////////////////////////////////////////
SGLPoint SGLSpriteEdit::GetCharPosFromIndex( size_t iChar ) const
{
	//
	// 表示中の文字の位置
	//
	SGLImageRect	rectChar ;
	if ( GetCharacterPosOfView( rectChar, iChar ) )
	{
		return	SGLPoint( rectChar.x, rectChar.y ) ;
	}
	//
	// ざっくりとした座標計算
	//
	size_t	nLine = GetLineFromIndex( iChar ) ;
	size_t	iLine = GetLineIndex( nLine ) ;
	size_t	nLineLen = GetLineLength( nLine ) ;
	//
	SGLFont		font ;
	font.SetStyle( m_styleEdit.font ) ;
	//
	int	xChar = 0 ;
	for ( size_t i = 0; i < nLineLen; i ++ )
	{
		wchar_t	wch = m_strEdit.GetAt( iLine + i ) ;
		if ( wch == '\t' )
		{
			int	pitchTab = m_styleEdit.context.pitchTab ;
			if ( pitchTab <= 0 )
			{
				pitchTab = m_styleEdit.font.nSize * 4 ;
			}
			xChar = ((xChar / pitchTab) + 1) * pitchTab ;
		}
		else
		{
			if ( m_styleEdit.context.pitchChar > 0 )
			{
				xChar += (m_styleEdit.context.pitchChar
							* m_styleEdit.context.scalePitch) >> 16 ;
			}
			else
			{
				SGLFontMetrics	metrics ;
				if ( !font.GetMetrics( nullptr, 0, metrics, wch ) )
				{
					xChar += (metrics.nWidth
								* m_styleEdit.context.scalePitch) >> 16 ;
				}
			}
			xChar += m_styleEdit.context.offsetChar ;
		}
	}
	int	nLinePitch = (int) GetLineHeight() ;
	return	SGLPoint( xChar - m_xScroll,
				(int) (nLine - m_yViewLine) * nLinePitch - m_yScrollOffset ) ;
}

// 文字指標から行番号(0～)を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineFromIndex( size_t iChar ) const
{
	const uint32_t *	pLineIndexed = m_indexLine.GetConstArray() ;
	const size_t		nLines = m_indexLine.GetLength() ;
	if ( nLines == 0 )
	{
		return	0 ;
	}
	size_t	iFirst = 0 ;
	size_t	iEnd = nLines - 1 ;
	while ( iFirst < iEnd )
	{
		size_t		iMiddle = (iFirst + iEnd) >> 1 ;
		uint32_t	iLine = pLineIndexed[iMiddle] ;
		if ( iChar < iLine )
		{
			ESLAssert( iMiddle > 0 ) ;
			iEnd = iMiddle - 1 ;
		}
		else if ( iChar > iLine )
		{
			if ( iMiddle + 1 >= nLines )
			{
				return	iMiddle ;
			}
			if ( iChar < pLineIndexed[iMiddle + 1] )
			{
				return	iMiddle ;
			}
			iFirst = iMiddle + 1 ;
		}
		else
		{
			return	iMiddle ;
		}
	}
	return	iFirst ;
}

// 行(0～)の先頭の文字指標を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineIndex( size_t nLine ) const
{
	if ( nLine >= m_indexLine.GetLength() )
	{
		return	m_strEdit.GetLength() ;
	}
	return	m_indexLine.At( nLine ) ;
}

// 行(0～)の文字数を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineLength( size_t nLine ) const
{
	return	GetLineIndex(nLine + 1) - GetLineIndex(nLine) ;
}

// 行数を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineCount( void ) const
{
	return	m_indexLine.GetLength() ;
}

// 全文字数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLength( void ) const
{
	return	m_strEdit.GetLength() ;
}

// 指定行(0～)の文字列を取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteEdit::GetLineText( size_t nLine, size_t iOffset ) const
{
	size_t	iLine = GetLineIndex( nLine ) + iOffset ;
	size_t	iNext = GetLineIndex( nLine + 1 ) ;
	if ( iLine < iNext )
	{
		return	m_strEdit.Middle
					( iLine, (ssize_t) iNext - (ssize_t) iLine ) ;
	}
	return	SString() ;
}

// スクロール位置取得（ピクセル, 行）
//////////////////////////////////////////////////////////////////////////////
SGLPoint SGLSpriteEdit::GetScrollPos( size_t* pLineOffset ) const
{
	if ( pLineOffset != nullptr )
	{
		*pLineOffset = (size_t) m_iViewLineOffset ;
	}
	int	yLine = m_yViewLine ;
	int	nLineHeight = (int) GetLineHeight() ;
	if ( nLineHeight != 0 )
	{
		size_t	nLines = GetLineCountOfView( m_yViewLine ) ;
		if ( (nLines != 0) && (m_yScrollOffset < 0) )
		{
			if ( (size_t) -m_yScrollOffset / nLineHeight >= nLines )
			{
				yLine ++ ;
				if ( pLineOffset != nullptr )
				{
					*pLineOffset = 0 ;
				}
			}
		}
	}
	return	SGLPoint( m_xScroll, yLine ) ;
}

// スクロール位置設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetScrollPos
	( int xPos, int yLine, int iLineOffset, int yOffset )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( yLine >= (int) GetLineCount() )
	{
		yLine = (int) GetLineCount() - 1 ;
	}
	if ( yLine < 0 )
	{
		yLine = 0 ;
	}
	m_xScroll = xPos ;
	m_yScrollOffset = yOffset ;
	if ( (m_yViewLine != yLine)
		|| (m_iViewLineOffset != iLineOffset) )
	{
		m_yViewLine = yLine ;
		m_iViewLineOffset = iLineOffset ;
		UpdateTextImage() ;
		//
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnScrolled( *this ) ;
		}
	}
	PostUpdate() ;
	Unlock() ;
}

// 行の最大幅（ピクセル）を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetMaxLineWidth( void ) const
{
	if ( m_styleEdit.nEditFlags & editLineWrap )
	{
		return	(size_t) GetImageSize().w ;
	}
	size_t	xMax = 0 ;
	for ( size_t i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv != nullptr )
		{
			size_t	wLine = plv->GetLineWidth() ;
			if ( wLine > xMax )
			{
				xMax = wLine ;
			}
		}
	}
	return	xMax ;
}

// 行間取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineHeight( void ) const
{
	if ( m_styleEdit.context.pitchLine > 0 )
	{
		return	(size_t) m_styleEdit.context.pitchLine ;
	}
	return	(size_t) m_styleEdit.font.nSize ;
}

// 文字表示幅計算（元の文字幅からスタイルで修正された表示幅を計算）
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetCharWidthOf( size_t nCharWidth ) const
{
	if ( m_styleEdit.context.pitchChar != 0 )
	{
		return	(size_t) ((m_styleEdit.context.pitchChar
					* m_styleEdit.context.scalePitch) >> 16)
								+ m_styleEdit.context.offsetChar ;
	}
	return	(size_t) ((nCharWidth * m_styleEdit.context.scalePitch) >> 16)
											+ m_styleEdit.context.offsetChar ;
}

// テキスト指標を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::UpdateTextIndex( void )
{
	m_indexLine.RemoveAll() ;
	m_indexLine.Add( 0 ) ;
	//
	SStringParser	sparsText ;
	sparsText.AttachString( m_strEdit ) ;
	//
	while ( !sparsText.IsIndexOverflow() )
	{
		if ( sparsText.SeekToNextLine() == 0 )
		{
			break ;
		}
		m_indexLine.Add( (uint32_t) sparsText.GetIndex() ) ;
	}
}

// 表示画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::UpdateTextImage( void )
{
	//
	// 表示領域初期化
	//
	SGLLetteringContext	context = m_styleEdit.context ;
	SGLSize	sizeViewBuf = GetImageSize() ;
	if ( !sizeViewBuf.IsEmpty() )
	{
		if ( m_styleEdit.nEditFlags & editLineWrap )
		{
			context.rectWritable.right = sizeViewBuf.w ;
		}
		context.rectWritable.bottom =
			sizeViewBuf.h + m_yScrollOffset + context.pitchLine - 1 ;
	}
	if ( !(m_styleEdit.nEditFlags & editLineWrap) )
	{
		context.rectWritable.right = 0x7FFFFFFF ;
	}
	context.rectWritable.top = 0 ;
	context.rectWritable.left = 0 ;
	context.ptStartWriting.x = 0 ;
	context.ptStartWriting.y = 0 ;
	context.minHyphening = 0 ;
	//
	// 各行をラスタライズ
	//
	SGLFont		font ;
	font.SetStyle( m_styleEdit.font ) ;
	//
	size_t	iViewBuf = 0 ;
	size_t	yLineView = m_yViewLine ;
	size_t	iLineOffset = m_iViewLineOffset ;
	//
	while ( (context.ptStartWriting.y < sizeViewBuf.h)
					&& (yLineView < m_indexLine.GetLength()) )
	{
		//
		// 行情報
		//
		size_t	iLineChar = GetLineIndex( yLineView ) + iLineOffset ;
		//
		// 既存バッファの判定
		//
		LineView *	plv = m_arrLineViewBuf.GetAt( iViewBuf ) ;
		while ( plv != nullptr )
		{
			if ( plv->m_iFirstChar < iLineChar )
			{
				m_arrLineViewBuf.RemoveAt( iViewBuf ) ;
				plv = m_arrLineViewBuf.GetAt( iViewBuf ) ;
			}
			else
			{
				break ;
			}
		}
		if ( (plv != nullptr)
			&& (plv->m_iFirstChar == iLineChar) )
		{
			if ( !(plv->m_fUpdate) )
			{
				plv->m_ptView = context.ptStartWriting ;
				context.ptStartWriting.x = 0 ;
				context.ptStartWriting.y += plv->m_sizeView.h ;
				iViewBuf ++ ;
				yLineView ++ ;
				iLineOffset = 0 ;
				continue ;
			}
			m_arrLineViewBuf.RemoveAt( iViewBuf ) ;
			plv = nullptr ;
		}
		//
		// 行取得
		//
		SString	strLine = GetLineText( yLineView, iLineOffset ) ;
		size_t	iLineSelFirst = m_iSelFirst - iLineChar ;
		size_t	iLineSelEnd = m_iSelEnd - iLineChar ;
		if ( m_iSelFirst < iLineChar )
		{
			iLineSelFirst = 0 ;
		}
		else if ( iLineSelFirst > strLine.GetLength() )
		{
			iLineSelFirst = strLine.GetLength() ;
		}
		if ( m_iSelEnd < iLineChar )
		{
			iLineSelEnd = 0 ;
		}
		else if ( iLineSelEnd > strLine.GetLength() )
		{
			iLineSelEnd = strLine.GetLength() ;
		}
		//
		// 新規ラスタライズ
		//
		plv = new LineView ;
		plv->m_ptView = context.ptStartWriting ;
		plv->m_sizeView.w = sizeViewBuf.w ;
		plv->m_iFirstChar = iLineChar ;
		plv->m_iSelFirst = iLineSelFirst ;
		plv->m_iSelEnd = iLineSelEnd ;
		m_arrLineViewBuf.InsertAt( iViewBuf ++, plv ) ;
		//
		SGLLetterer	letterer ;
		if ( m_styleEdit.nEditFlags & editPassword )
		{
			size_t		nLength = strLine.GetLength() ;
			uint16_t *	pwLine = strLine.LockBuffer( nLength ) ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				if ( pwLine[i] >= L' ' )
				{
					pwLine[i] = L'*' ;
				}
			}
			strLine.UnlockBuffer( (ssize_t) nLength ) ;
		}
		letterer.WriteLetter( font, context, strLine ) ;
		//
		plv->m_fUpdate =
			(letterer.GetLetterLength() < strLine.GetLength()) ;
		//
		// 文字位置情報
		//
		size_t	nLineHeight = GetLineHeight() ;
		size_t	nLength = letterer.GetLetterLength() ;
		plv->m_arrCharRects.SetLimit( nLength ) ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			SGLLetterer::Character *
				pLChar = letterer.GetCharacterAt( i ) ;
			if ( pLChar == nullptr )
			{
				continue ;
			}
			ESLAssert( pLChar->wchCode == strLine.GetAt( i ) ) ;
			SGLImageRect	rectChar ;
			rectChar.x = pLChar->ptWriting.x - plv->m_ptView.x ;
			rectChar.y = pLChar->ptWriting.y - plv->m_ptView.y ;
			rectChar.w = pLChar->sizeChar.w ;
			rectChar.h = (int32_t) nLineHeight ;
			//
			SGLLetterer::Character *
				pLCharNext = letterer.GetCharacterAt( i + 1 ) ;
			if ( (pLCharNext != nullptr)
				&& (pLChar->ptWriting.y == pLCharNext->ptWriting.y)
				&& (pLChar->ptWriting.x < pLCharNext->ptWriting.x) )
			{
				rectChar.w = pLCharNext->ptWriting.x - pLChar->ptWriting.x ;
			}
			plv->m_arrCharRects.SetAt( i, rectChar ) ;
			plv->m_sizeView.h = rectChar.y + rectChar.h ;
		}
		//
		// 文字画像装飾
		//
		PrepareToCustomTextLineDecoration( strLine, iLineChar ) ;
		//
		plv->m_rgbaBackColor = 0 ;
		plv->m_arrCharBack.RemoveAll() ;
		//
		plv->m_rgbaBackColor =
			CustomTextLineBackColor( plv, iLineChar, yLineView ) ;
		//
		size_t	iNextDeco = 0 ;
		while ( iNextDeco < strLine.GetLength() )
		{
			SGLLetterer::Decoration	deco = m_styleEdit.decoration ;
			size_t		iFromDeco = iNextDeco ;
			SGLPalette	rgbaBack( 0 ) ;
			CustomTextLineDecoration
				( deco, rgbaBack, iNextDeco, strLine, iLineChar ) ;
			//
			for ( size_t i = iFromDeco; i < iNextDeco; i ++ )
			{
				plv->m_arrCharBack.Add( rgbaBack ) ;
			}
			letterer.DecorateLetter
				( deco, iFromDeco, (ssize_t) (iNextDeco - iFromDeco) ) ;
		}
		letterer.CombineLetter() ;
		//
		// 表示用画像生成
		//
		SGLLetterer::Character *	pLChar = letterer.GetCharacterAt(0) ;
		SGLPoint	ptOffset( 0, 0 ) ;
		SGLSize		sizeImage( 0, 0 ) ;
		if ( (pLChar != nullptr) && (pLChar->pImage != nullptr) )
		{
			ptOffset = pLChar->ptWriting + pLChar->ptOffset - plv->m_ptView ;
			plv->m_ptOffset = ptOffset ;
			//
			SGLImageBuffer *	pTextImage = pLChar->pImage ;
			sizeImage.w = (int) pTextImage->width ;
			sizeImage.h = (int) pTextImage->height ;
		}
		AdjustmentTextLineSize
				( sizeImage, plv->m_ptOffset, plv, strLine ) ;
		ptOffset -= plv->m_ptOffset ;
		//
		if ( !sizeImage.IsEmpty() )
		{
			plv->m_imgView.CreateImage
				( sizeImage.w, sizeImage.h, formatImageARGB, 32 ) ;
			//
			if ( (pLChar != nullptr) && (pLChar->pImage != nullptr) )
			{
				SGLImageBuffer	imgbuf ;
				imgbuf.ptrBuffer = plv->m_imgView.LockBuffer( imgbuf ) ;
				sglConvertImageBuffer
					( imgbuf, *(pLChar->pImage), ptOffset.x, ptOffset.y ) ;
				plv->m_imgView.UnlockBuffer() ;
			}
			//
			AfterUpdateTextLineImage( plv, strLine ) ;
		}
		//
		yLineView ++ ;
		iLineOffset = 0 ;
	}
	//
	// 画面外の既存バッファを削除する
	//
	if ( iViewBuf < m_arrLineViewBuf.GetLength() )
	{
		m_arrLineViewBuf.Remove
			( iViewBuf, m_arrLineViewBuf.GetLength() - iViewBuf ) ;
	}
}

// 文字装飾のカスタマイズを準備する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::PrepareToCustomTextLineDecoration
			( const SString& strLine, size_t iLineChar )
{
}

// 行全体の背景色を決定する
//////////////////////////////////////////////////////////////////////////////
SGLPalette SGLSpriteEdit::CustomTextLineBackColor
	( const SGLSpriteEdit::LineView * plvLine,
						size_t iLineChar, size_t nLineNum )
{
	return	SGLPalette( 0 ) ;
}

// 文字装飾をカスタマイズする
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::CustomTextLineDecoration
	( SGLLetterer::Decoration& deco,
		SGLPalette& rgbaBack, size_t& iNextDeco,
		const SString& strLine, size_t iLineChar )
{
	size_t	iChar = iLineChar + iNextDeco ;
	if ( iChar < m_iSelFirst )
	{
		iNextDeco = m_iSelFirst - iLineChar ;
		if ( iNextDeco >= strLine.GetLength() )
		{
			iNextDeco = strLine.GetLength() ;
		}
		deco = m_styleEdit.decoration ;
		rgbaBack = 0 ;
	}
	else if ( iChar >= m_iSelEnd )
	{
		iNextDeco = strLine.GetLength() ;
		deco = m_styleEdit.decoration ;
		rgbaBack = 0 ;
	}
	else
	{
		iNextDeco = m_iSelEnd - iLineChar ;
		if ( iNextDeco >= strLine.GetLength() )
		{
			iNextDeco = strLine.GetLength() ;
		}
		deco = m_styleEdit.decoSel ;
		rgbaBack = m_styleEdit.rgbaSelBackColor ;
	}
}

// 表示用画像バッファサイズを調整する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::AdjustmentTextLineSize
	( SGLSize& sizeImage, SGLPoint& ptOffset,
		const SGLSpriteEdit::LineView * plvLine, const wchar_t * pwszLine )
{
}

// 表示画像に後処理する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::AfterUpdateTextLineImage
	( SGLSpriteEdit::LineView * plvLine, const wchar_t * pwszLine )
{
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::SetUpdateRange
		( size_t iFirst, size_t iEnd, ssize_t iOffset, bool fUpdateText )
{
	size_t	nCount = m_arrLineViewBuf.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv != nullptr )
		{
			if ( plv->m_iFirstChar > iFirst )
			{
				plv->m_iFirstChar += iOffset ;
			}
			size_t	iLineFirst = plv->m_iFirstChar ;
			size_t	iLineEnd = iLineFirst + plv->m_arrCharRects.GetLength() ;
			if ( fUpdateText )
			{
				if ( ((iFirst <= iLineFirst) && (iLineFirst <= iEnd))
					|| ((iFirst <= iLineEnd) && (iLineEnd <= iEnd))
					|| ((iLineFirst <= iFirst) && (iFirst <= iLineEnd))
					|| ((iLineFirst <= iEnd) && (iEnd <= iLineEnd)) )
				{
					plv->m_fUpdate = true ;
				}
			}
			else
			{
				size_t	iSelFirst = iFirst - iLineFirst ;
				size_t	iSelEnd = iEnd - iLineFirst ;
				if ( iFirst < iLineFirst )
				{
					iSelFirst = 0 ;
				}
				else if ( iFirst >= iLineEnd )
				{
					iSelFirst = plv->m_arrCharRects.GetLength() ;
				}
				if ( iEnd < iLineFirst )
				{
					iSelEnd = 0 ;
				}
				else if ( iEnd >= iLineEnd )
				{
					iSelEnd = plv->m_arrCharRects.GetLength() ;
				}
				if ( (plv->m_iSelFirst != iSelFirst)
					|| (plv->m_iSelEnd != iSelEnd) )
				{
					plv->m_fUpdate = true ;
				}
			}
		}
	}
	for ( size_t i = 0; i < m_arrBookmark.GetLength(); i ++ )
	{
		Bookmark *	pbm = m_arrBookmark.GetAt( i ) ;
		ESLAssert( pbm != nullptr ) ;
		if ( pbm->m_iPos >= iFirst )
		{
			if ( (ssize_t) pbm->m_iPos + iOffset < (ssize_t) iFirst )
			{
				m_arrBookmark.RemoveAt( i -- ) ;
				continue ;
			}
			pbm->m_iPos += iOffset ;
		}
	}
	PostUpdate() ;
}

// Undo を記録
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::RecordUndo( SGLSpriteEdit::UndoRecord * pUndo )
{
	//
	// 無意味な UNDO か判定する
	//
	if ( pUndo == nullptr )
	{
		return ;
	}
	if ( pUndo->m_strText.IsEmpty()
		&& (pUndo->m_iFirst == pUndo->m_iEnd) )
	{
		delete	pUndo ;
		return ;
	}
	//
	// 現在の REDO を削除する
	//
	m_arrRedo.RemoveAll( ) ;
	//
	// 最後の UNDO と結合できるか判定する
	//
	UndoRecord *	pLastUndo = ESLTypeCast<UndoRecord>( m_arrUndo.GetLastAt() ) ;
	if ( pLastUndo != nullptr )
	{
		if ( (pLastUndo->m_iEnd == pUndo->m_iFirst)
			&& (pUndo->m_iFirst < pUndo->m_iEnd)
			&& (pLastUndo->m_iFirst < pLastUndo->m_iEnd)
			&& pUndo->m_strText.IsEmpty()
			&& pLastUndo->m_strText.IsEmpty() )
		{
			pLastUndo->m_iEnd = pUndo->m_iEnd ;
			delete	pUndo ;
			return ;
		}
	}
	//
	// UNDO 追加
	//
	if ( m_arrUndo.GetLength() > m_limitUndo )
	{
		m_arrUndo.Remove
			( 0, m_arrUndo.GetLength() - m_limitUndo ) ;
	}
	m_arrUndo.Add( pUndo ) ;
}

// ユーザー定義 Undo を記録
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::RecordUndoObject( SGLSpriteEdit::UndoObject * pUndo )
{
	if ( pUndo == nullptr )
	{
		return ;
	}
	//
	// 現在の REDO を削除する
	//
	m_arrRedo.RemoveAll( ) ;
	//
	// UNDO 追加
	//
	if ( m_arrUndo.GetLength() > m_limitUndo )
	{
		m_arrUndo.Remove
			( 0, m_arrUndo.GetLength() - m_limitUndo ) ;
	}
	m_arrUndo.Add( pUndo ) ;
}

// 直前の Undo を取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::UndoObject * SGLSpriteEdit::GetLastUndoObject( void ) const
{
	return	m_arrUndo.GetLastAt() ;
}

// ブックマーク追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::AddBookmark( SGLSpriteEdit::Bookmark * pBookmark )
{
	size_t	i = OrderBookmarkIndex( pBookmark->m_iPos ) ;
	m_arrBookmark.InsertAt( i, pBookmark ) ;
}

// ブックマーク数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetBookmarkCount( void ) const
{
	return	m_arrBookmark.GetLength() ;
}

// ブックマーク取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::Bookmark * SGLSpriteEdit::GetBookmarkAt( size_t iBookmark ) const
{
	return	m_arrBookmark.GetAt( iBookmark ) ;
}

// 指定行(0～)に含まれるブックマーク取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEdit::Bookmark * SGLSpriteEdit::GetBookmarkLineAt( size_t nLine ) const
{
	size_t	iLineChar = GetLineIndex( nLine ) ;
	size_t	iLineEnd = GetLineIndex( nLine + 1 ) ;
	ssize_t	iBookmark = FindBookmarkOf( iLineChar, iLineEnd ) ;
	if ( iBookmark < 0 )
	{
		return	nullptr ;
	}
	return	m_arrBookmark.GetAt( (size_t) iBookmark ) ;
}

// ブックマーク指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteEdit::FindBookmark( SGLSpriteEdit::Bookmark * pBookmark ) const
{
	Bookmark *const*	ppBookmarks = m_arrBookmark.GetConstArray() ;
	size_t				iOrder = OrderBookmarkIndex( pBookmark->m_iPos ) ;
	size_t	nCount = m_arrBookmark.GetLength() ;
	for ( size_t i = iOrder; i < nCount; i ++ )
	{
		if ( ppBookmarks[i] == pBookmark )
		{
			return	(ssize_t) i ;
		}
		if ( ppBookmarks[i]->m_iPos != pBookmark->m_iPos )
		{
			break ;
		}
	}
	for ( size_t i = 1; i <= iOrder; i ++ )
	{
		if ( ppBookmarks[iOrder - i] == pBookmark )
		{
			return	(ssize_t) (iOrder - i) ;
		}
		if ( ppBookmarks[iOrder - i]->m_iPos != pBookmark->m_iPos )
		{
			break ;
		}
	}
	return	-1 ;
}

ssize_t SGLSpriteEdit::FindBookmarkOf( size_t iFirstChar, size_t iEndChar ) const
{
	Bookmark *const*	ppBookmarks = m_arrBookmark.GetConstArray() ;
	const size_t		nCount = m_arrBookmark.GetLength() ;
	size_t				i = OrderBookmarkIndex( iFirstChar ) ;
	while ( i < nCount )
	{
		Bookmark *	pbm = ppBookmarks[i] ;
		if ( pbm->m_iPos >= iEndChar )
		{
			break ;
		}
		if ( pbm->m_iPos >= iFirstChar )
		{
			return	(ssize_t) i ;
		}
		i ++ ;
	}
	return	-1 ;
}

// ブックマーク削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::RemoveBookmarkAt( size_t iBookmark )
{
	m_arrBookmark.RemoveAt( iBookmark ) ;
}

// 全ブックマーク削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::RemoveAllBookmarks( void )
{
	m_arrBookmark.RemoveAll() ;
}

// ブックマーク・バイナリ検索
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::OrderBookmarkIndex( size_t iCharPos ) const
{
	Bookmark *const*	ppBookmarks = m_arrBookmark.GetConstArray() ;
	const size_t		nCount = m_arrBookmark.GetLength() ;
	if ( nCount == 0 )
	{
		return	0 ;
	}
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) nCount - 1 ;
	while ( iFirst <= iEnd )
	{
		ssize_t		iMiddle = (iFirst + iEnd) >> 1 ;
		Bookmark *	pbm = ppBookmarks[iMiddle] ;
		ESLAssert( pbm != nullptr ) ;
		//
		if ( pbm->m_iPos > iCharPos )
		{
			iEnd = iMiddle - 1 ;
		}
		else if ( pbm->m_iPos < iCharPos )
		{
			iFirst = iMiddle + 1 ;
		}
		else
		{
			return	(size_t) iMiddle ;
		}
	}
	return	iFirst ;
}

// 指定行の更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::PostUpdateLine( int nLine )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SetUpdateRange
		( GetLineIndex( nLine ), GetLineIndex( nLine + 1 ) ) ;
	Unlock() ;
}

// すべての表示行の更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::PostUpdateAllLines( void )
{
	Lock() ;
	m_arrLineViewBuf.RemoveAll() ;
	Unlock() ;
}

// 指定文字指標を表示領域に収まるようにスクロールする
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::TrackCharacterFor( size_t iChar )
{
	LockTrace( __FILE__, __LINE__ ) ;
	//
	SGLSize	sizeViewBuf = GetImageSize() ;
	size_t	nViewLines = (size_t) (sizeViewBuf.h / GetLineHeight()) ;
	bool	fScrolled = false ;
	//
	SGLImageRect	rectChar ;
	if ( !GetCharacterPosOfView( rectChar, iChar ) )
	{
		//
		// 垂直方向スクロール位置調整
		//
		size_t	iLine = GetLineFromIndex( iChar ) ;
		if ( ((int) iLine <= m_yViewLine) || (nViewLines < 3) )
		{
			m_yViewLine = (int) iLine ;
			m_iViewLineOffset = 0 ;
			m_xScroll = 0 ;
			m_yScrollOffset = 0 ;
			fScrolled = true ;
		}
		else
		{
			m_yViewLine = (int) iLine - (int) (nViewLines - 3) ;
			m_iViewLineOffset = 0 ;
			m_xScroll = 0 ;
			m_yScrollOffset = 0 ;
			if ( m_yViewLine < 0 )
			{
				m_yViewLine = 0 ;
			}
			fScrolled = true ;
		}
		UpdateTextImage() ;
		//
		for ( ; ; )
		{
			//
			// 表示領域内の位置
			//
			if ( GetCharacterPosOfView( rectChar, iChar ) )
			{
				break ;
			}
			if ( (int) iLine > m_yViewLine )
			{
				m_yViewLine ++ ;
				fScrolled = true ;
			}
			else
			{
				m_yScrollOffset += (int) GetLineHeight() ;
				fScrolled = true ;
			}
			UpdateTextImage() ;
		}
	}
	//
	// 行のオフセットスクロール位置調整
	//
	if ( rectChar.y < 0 )
	{
		m_yScrollOffset += rectChar.y ;
		ESLAssert( m_yScrollOffset >= 0 ) ;
		fScrolled = true ;
	}
	else if ( rectChar.y >= (int) (nViewLines * GetLineHeight()) )
	{
		m_yScrollOffset +=
			rectChar.y - (int) (nViewLines - 1) * (int) GetLineHeight() ;
		fScrolled = true ;
	}
	//
	// 水平方向スクロール位置調整
	//
	if ( m_styleEdit.nEditFlags & editLineWrap )
	{
		m_xScroll = 0 ;
		fScrolled = true ;
	}
	else if ( rectChar.x < 0 )
	{
		m_xScroll += rectChar.x ;
		fScrolled = true ;
	}
	else if ( rectChar.x + (int) m_styleEdit.font.nSize > sizeViewBuf.w )
	{
		m_xScroll += rectChar.x + m_styleEdit.font.nSize * 2 - sizeViewBuf.w ;
		fScrolled = true ;
	}
	PostUpdate() ;
	//
	if ( fScrolled )
	{
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnScrolled( *this ) ;
		}
	}
	Unlock() ;
}

// 指定文字指標の座標を取得する
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::GetCharacterPosOfView( SGLImageRect& rectChar, size_t iChar ) const
{
	Lock() ;
	for ( size_t i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv != nullptr )
		{
			const SGLImageRect *	pRects = plv->m_arrCharRects.GetConstArray() ;
			const size_t			nChars = plv->m_arrCharRects.GetLength() ;
			size_t					iLine = plv->m_iFirstChar ;
			if ( (iLine <= iChar) && (iChar < iLine + nChars) )
			{
				rectChar.x = pRects[iChar - iLine].x ;
				rectChar.y = pRects[iChar - iLine].y ;
				rectChar.x += plv->m_ptView.x - m_xScroll ;
				rectChar.y += plv->m_ptView.y - m_yScrollOffset ;
				rectChar.w = pRects[iChar - iLine].w ;
				rectChar.h = pRects[iChar - iLine].h ;
				Unlock() ;
				return	true ;
			}
			else if ( (iChar >= m_strEdit.GetLength())
					&& (m_strEdit.GetLength() <= iLine + nChars) )
			{
				if ( nChars > 0 )
				{
					if ( m_strEdit.GetLastAt(0) == L'\n' )
					{
						rectChar.x = 0 ;
						rectChar.y = pRects[nChars - 1].y ;
						rectChar.y += (int32_t) GetLineHeight() ;
					}
					else
					{
						rectChar.x = pRects[nChars - 1].x ;
						rectChar.y = pRects[nChars - 1].y ;
						rectChar.x += (int32_t) GetCharWidthOf( pRects[nChars - 1].w ) ;
					}
				}
				else
				{
					rectChar.x = plv->m_ptView.x ;
					rectChar.y = plv->m_ptView.y ;
				}
				rectChar.x += plv->m_ptView.x - m_xScroll ;
				rectChar.y += plv->m_ptView.y - m_yScrollOffset ;
				rectChar.w = 0 ;
				rectChar.h = (int32_t) GetLineHeight() ;
				Unlock() ;
				return	true ;
			}
		}
	}
	if ( m_strEdit.GetLength() == 0 )
	{
		rectChar.x = 0 ;
		rectChar.y = 0 ;
		rectChar.w = 0 ;
		rectChar.h = (int32_t) GetLineHeight() ;
		Unlock() ;
		return	true ;
	}
	Unlock() ;
	return	false ;
}

// 指定行の表示行数を取得する（表示されている範囲のみ）
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteEdit::GetLineCountOfView( size_t nLine ) const
{
	Lock() ;
	size_t	nLineHeight = GetLineHeight() ;
	if ( nLineHeight == 0 )
	{
		Unlock() ;
		return	0 ;
	}
	size_t	iLine = GetLineIndex( nLine ) ;
	size_t	nLineLen = GetLineLength( nLine ) ;
	for ( size_t i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv != nullptr )
		{
			if ( (iLine <= plv->m_iFirstChar)
				&& (plv->m_iFirstChar < iLine + nLineLen) )
			{
				SGLImageRect *	pRect = plv->m_arrCharRects.GetLastAt() ;
				if ( pRect != nullptr )
				{
					size_t	nLines = (size_t) pRect->y / nLineHeight + 1 ;
					Unlock();
					return	nLines ;
				}
			}
		}
	}
	Unlock() ;
	return	0 ;
}

// 座標から文字指標へ変換
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteEdit::GetCharIndexFromPosOfView( int xPos, int yPos ) const
{
	Lock() ;
	int	nLineHeight = (int) GetLineHeight() ;
	xPos += m_xScroll ;
	yPos += m_yScrollOffset ;
	int	yLine = yPos / nLineHeight ;
	//
	for ( size_t i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv == nullptr )
		{
			continue ;
		}
		const SGLImageRect *	pRects = plv->m_arrCharRects.GetConstArray() ;
		const size_t			nChars = plv->m_arrCharRects.GetLength() ;
		SGLPoint				ptLine = plv->m_ptView ;
		size_t					iLine = plv->m_iFirstChar ;
		ssize_t					iLineEnd = -1 ;
		for ( size_t j = 0; j < nChars; j ++, pRects ++ )
		{
			int	yCharLine = (ptLine.y + pRects->y) / nLineHeight ;
			if ( yCharLine != yLine )
			{
				continue ;
			}
			if ( xPos < pRects->x )
			{
				continue ;
			}
			iLineEnd = (ssize_t) (iLine + j) ;
			if ( xPos < pRects->x + pRects->w )
			{
				Unlock() ;
				return	iLineEnd ;
			}
			wchar_t	wch = m_strEdit.GetAt( iLine + j ) ;
			if ( (wch == '\n') || (wch == L'\r') || (j + 1 >= nChars) )
			{
				Unlock() ;
				return	iLineEnd ;
			}
		}
		if ( iLine + nChars >= m_strEdit.GetLength() )
		{
			if ( plv->m_ptView.y + plv->m_sizeView.h <= yLine )
			{
				ssize_t	iChar = (ssize_t) m_strEdit.GetLength() ;
				Unlock() ;
				return	iChar ;
			}
		}
	}
	Unlock() ;
	return	-1 ;
}

// カレット表示矩形を取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::GetCaretRect( SGLImageRect& rectCaret ) const
{
	Lock() ;
	if ( GetCharacterPosOfView( rectCaret, m_iCursor ) )
	{
		if ( m_styleEdit.nEditFlags & editUnderbarCaret )
		{
			rectCaret.y += rectCaret.h - m_styleEdit.nCaretWidth ;
			rectCaret.h = m_styleEdit.nCaretWidth ;
			if ( rectCaret.w < (int) m_styleEdit.font.nSize / 2 )
			{
				rectCaret.w = m_styleEdit.font.nSize / 2 ;
			}
			if ( rectCaret.h == 0 )
			{
				rectCaret.h = (int32_t) GetLineHeight() ;
			}
		}
		else
		{
			if ( m_styleEdit.nCaretWidth > 0 )
			{
				rectCaret.w = m_styleEdit.nCaretWidth ;
			}
			else if ( rectCaret.w == 0 )
			{
				rectCaret.w = m_styleEdit.font.nSize / 2 ;
			}
		}
		Unlock() ;
		return	true ;
	}
	Unlock() ;
	return	false ;
}

// 垂直相対スクロール
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::ScrollDeltaVertical( int yDelta )
{
	LockTrace( __FILE__, __LINE__ ) ;
	int	nLineHeight = (int) GetLineHeight() ;
	if ( nLineHeight == 0 )
	{
		Unlock() ;
		return ;
	}
	bool	fScrolled = false ;
	if ( yDelta < 0 )
	{
		do
		{
			//
			// ｙオフセット範囲内か？
			//
			int	yOffset = m_yScrollOffset / nLineHeight ;
			if ( yOffset >= - yDelta )
			{
				m_yScrollOffset += yDelta * nLineHeight ;
				PostUpdate() ;
				fScrolled = true ;
				break ;
			}
			yDelta += yOffset ;
			m_yScrollOffset = 0 ;
			//
			if ( !(m_styleEdit.nEditFlags & editLineWrap) )
			{
				//
				// 行の折り返しがない場合には移動行数だけスクロール
				//
				m_yViewLine += yDelta ;
				if ( m_yViewLine < 0 )
				{
					m_yViewLine = 0 ;
				}
				UpdateTextImage() ;
				PostUpdate() ;
				fScrolled = true ;
				break ;
			}
			//
			// 折り返し行を含む処理
			//
			for ( ; ; )
			{
				size_t	iLastTop =
					GetLineIndex( m_yViewLine ) + m_iViewLineOffset ;
				if ( m_iViewLineOffset > 0 )
				{
					m_iViewLineOffset = 0 ;
				}
				else
				{
					if ( m_yViewLine <= 0 )
					{
						break ;
					}
					m_yViewLine -- ;
				}
				UpdateTextImage() ;
				fScrolled = true ;
				//
				LineView *	plv0 ;
				bool	fBreak = false ;
				for ( int i = 0; i < 8; i ++ )
				{
					plv0 = m_arrLineViewBuf.GetAt( 0 ) ;
					ESLAssert( plv0 != nullptr ) ;
					if ( plv0 == nullptr )
					{
						fBreak = true ;
						break ;
					}
					if ( plv0->m_iFirstChar
						+ plv0->m_arrCharRects.GetLength() >= iLastTop )
					{
						break ;
					}
					m_iViewLineOffset += (int) plv0->m_arrCharRects.GetLength() ;
					UpdateTextImage() ;
					PostUpdate() ;
				}
				if ( fBreak )
				{
					break ;
				}
				if ( plv0->m_iFirstChar
					+ plv0->m_arrCharRects.GetLength() < iLastTop )
				{
					break ;	// 非常に長い行の折り返し処理で
							// 処理が無制限に重くなりすぎるのを防ぐため
				}
				int	nSubLines = plv0->m_sizeView.h / nLineHeight ;
				if ( nSubLines >= - yDelta )
				{
					m_yScrollOffset = (nSubLines - 2 - yDelta) * nLineHeight ;
					PostUpdate() ;
					break ;
				}
				yDelta += nSubLines ;
			}
		}
		while ( false ) ;
	}
	else if ( yDelta > 0 )
	{
		//
		// 表示範囲からスクロール位置を算出
		//
		while ( yDelta > 0 )
		{
			size_t	i ;
			size_t	iNextViewLine =
						GetLineIndex( m_yViewLine ) + m_iViewLineOffset ;
			for ( i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
			{
				LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
				if ( plv == nullptr )
				{
					continue ;
				}
				size_t	iLineFirst = plv->m_iFirstChar ;
				if ( m_yScrollOffset
						+ yDelta * nLineHeight < plv->m_sizeView.h )
				{
					m_yScrollOffset += yDelta * nLineHeight ;
					yDelta = 0 ;
					break ;
				}
				if ( (iLineFirst
						+ plv->m_arrCharRects.GetLength()
									>= m_strEdit.GetLength())
					&& ((plv->m_ptView.y <= 0)
						|| (plv->m_ptView.y
							+ (int) GetLineHeight() <= GetImageSize().h)) )
				{
					yDelta = 0 ;
					break ;
				}
				yDelta -= (plv->m_sizeView.h - m_yScrollOffset) / nLineHeight ;
				iNextViewLine = iLineFirst + plv->m_arrCharRects.GetLength() ;
				m_yScrollOffset = 0 ;
			}
			m_yViewLine = (int) GetLineFromIndex( iNextViewLine ) ;
			m_iViewLineOffset =
					(int) iNextViewLine - (int) GetLineIndex( m_yViewLine ) ;
			UpdateTextImage() ;
			fScrolled = true ;
			//
			if ( m_arrLineViewBuf.GetLength() == 0 )
			{
				break ;
			}
		}
		PostUpdate() ;
	}
	if ( fScrolled )
	{
		SGLSpriteEditListener *	pListener = m_refEditListener ;
		if ( pListener != nullptr )
		{
			pListener->OnScrolled( *this ) ;
		}
	}
	Unlock() ;
}

// 文字種類
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteEdit::WordKindOf( wchar_t wch ) const
{
	static const DWORD	dwPunctuationMask[4] =
	{
		0xFFFFFFFF,		// All control code is punctuation.
		0x7C00FFFF,		// " !"#$%&'()*+,-./" and ":;<=>" are punctuation.
		0x78000000,		// "[\]^" are punctuation.
		0xF8000001		// '`' and "{|}~ " are punctuation.
	} ;
	static const DWORD	dwSpecialPuncMask[4] =
	{
		0x00000000,		//
		0x58001384,		// ""'(),;<>" are special punctuation.
		0x28000000,		// "[]" are special punctuation.
		0x28000000		// "{}" are special punctuation.
	} ;
	static const wchar_t	wchMarkChar[] = L"１ｚぁんァヶ" ;
	//
	if ( /*(wch >= 0) &&*/ (wch < 0x80) )
	{
		if ( wch <= L' ' )
		{
			return	wordControl ;			// 空白・制御文字
		}
		int	i = (wch >> 5) ;
		int	j = 1 << (wch & 0x1F) ;
		if ( dwSpecialPuncMask[i] & j )
		{
			return	wordSeparator ;			// 特殊区切り記号
		}
		if ( dwPunctuationMask[i] & j )
		{
			return	wordMark ;				// 区切り記号
		}
		return	wordAlphabet ;				// 半角アルファベット文字
	}
	if ( wch == L'　' )
	{
		return	wordWideSpace ;
	}
	if ( (wch >= wchMarkChar[0]) && (wch <= wchMarkChar[1]) )
	{
		return	wordWideAlphabet ;			// 全角英数字
	}
	if ( (wch >= wchMarkChar[2]) && (wch <= wchMarkChar[3]) )
	{
		return	wordHiraKana ;				// 全角ひらがな
	}
	if ( (wch >= wchMarkChar[4]) && (wch <= wchMarkChar[5]) )
	{
		return	wordWideKana ;				// 全角カタカナ
	}
	if ( IsProhibitChar( wch ) )
	{
		return	wordWidePunctuation ;		// 禁則文字（句読点などの区切り文字）
	}
	return	wordOtherCharacter ;			// 漢字や記号、或いは、それ以外の言語の文字
}

// 禁則文字判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::IsProhibitChar( wchar_t wch ) const
{
	const wchar_t *	pwszProhibition = m_styleEdit.context.pwszProhibition ;
	if ( pwszProhibition != nullptr )
	{
		for ( size_t i = 0; pwszProhibition[i]; i ++ )
		{
			if ( pwszProhibition[i] == wch )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// 単語範囲判定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::FindWordRange
	( size_t& iFirst, size_t& iEnd, size_t iChar ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	nLength = m_strEdit.GetLength() ;
	if ( iChar >= nLength )
	{
		iFirst = nLength ;
		iEnd = nLength ;
		Unlock() ;
		return ;
	}
	int		nType = WordKindOf( m_strEdit.GetAt( iChar ) ) ;
	ssize_t	iPrev = (ssize_t) iChar - 1 ;
	while ( iPrev >= 0 )
	{
		if ( WordKindOf( m_strEdit.GetAt( (size_t) iPrev ) ) != nType )
		{
			break ;
		}
		iPrev -- ;
	}
	iFirst = iPrev + 1 ;
	//
	size_t	iNext = iChar + 1 ;
	while ( iNext < nLength )
	{
		if ( WordKindOf( m_strEdit.GetAt( iNext ) ) != nType )
		{
			break ;
		}
		iNext ++ ;
	}
	iEnd = iNext ;
	Unlock() ;
}

// 入力テキストフィルタ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::FilterInputText( SSystem::SString& strText )
{
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->FilterInputText( *this, strText ) ;
	}
	size_t		iSrc = 0, iDst = 0 ;
	size_t		nLength = strText.GetLength() ;
	uint16_t *	pwText = strText.LockBuffer( nLength ) ;
	uint32_t	nEditFlags = m_styleEdit.nEditFlags ;
	while ( iSrc < nLength )
	{
		uint16_t	w = pwText[iSrc ++] ;
		if ( nEditFlags & editNumber )
		{
			if ( (L'0' <= w) && (w <= L'9') )
			{
				pwText[iDst ++] = w ;
			}
			continue ;
		}
		if ( w < 0x80 )
		{
			if ( (w == L'\n') || (w == L'\r') )
			{
				if ( nEditFlags & editAcceptReturn )
				{
					pwText[iDst ++] = w ;
				}
			}
			else if ( w == L'\t' )
			{
				if ( nEditFlags & editAcceptTab )
				{
					pwText[iDst ++] = w ;
				}
			}
			else if ( !(nEditFlags & editDeny8bitChar) )
			{
				if ( (L'0' <= w) && (w <= L'9') )
				{
					if ( !(nEditFlags & editDenyNumber) )
					{
						pwText[iDst ++] = w ;
					}
				}
				else if ( ((L'A' <= w) && (w <= L'Z'))
						|| ((L'a' <= w) && (w <= L'z')) )
				{
					if ( !(nEditFlags & editDenyAlphabet) )
					{
						pwText[iDst ++] = w ;
					}
				}
				else
				{
					pwText[iDst ++] = w ;
				}
			}
		}
		else
		{
			if ( !(nEditFlags & editDenyMBChar) )
			{
				pwText[iDst ++] = w ;
			}
		}
	}
	strText.UnlockBuffer( (ssize_t) iDst ) ;
}

// テキスト編集メッセージボックスを使用
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::CallEditMessageBoxProc( void * pInstance )
{
	SGLSpriteEdit::CallEditMessageBox *
		pcemb = (SGLSpriteEdit::CallEditMessageBox*) pInstance ;
	if ( MessageEditBox
		( pcemb->m_strEdit, nullptr,
			pcemb->m_strCaption, pcemb->m_nStyles ) == msgboxResultOk )
	{
		SGLSpriteEdit *	pEdit ;
		pEdit = pcemb->m_refEdit.GetReference() ;
		if ( pEdit != nullptr )
		{
			pEdit->LockTrace( __FILE__, __LINE__ ) ;
			pEdit->FilterInputText( pcemb->m_strEdit ) ;
			pEdit->SetText( pcemb->m_strEdit ) ;
			//
			SGLSpriteEditListener *	pListener = pEdit->m_refEditListener ;
			if ( pListener != nullptr )
			{
				pListener->OnChangedText( *pEdit ) ;
			}
			pEdit->Unlock() ;
		}
	}
	delete	pcemb ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	if ( m_styleEdit.nCaretBlinkInterval > 0 )
	{
		SGLImageRect	irCaret ;
		if ( GetCaretRect( irCaret ) )
		{
			SGLRect	rectCaret = irCaret ;
			m_msecCaret = (m_msecCaret + msecPast)
								% m_styleEdit.nCaretBlinkInterval ;
			PostUpdate( &rectCaret ) ;
		}
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEdit::DrawChildren
	( S3DRenderContextInterface& render,
			SGLSprite::Stereo3DView s3dView ) const
{
	SGLImageRect	rectView ;
	render.GetViewPort( rectView ) ;
	//
	// 文字背景描画
	//
	size_t	i ;
	for ( i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv == nullptr )
		{
			continue ;
		}
		if ( plv->m_rgbaBackColor.ui32 != 0 )
		{
			render.FillRectangle
				( plv->m_ptView.x - m_xScroll,
					plv->m_ptView.y - m_yScrollOffset,
					rectView.w, plv->m_sizeView.h,
					plv->m_rgbaBackColor.ui32 ) ;
		}
		const SGLImageRect *	pCharRects = plv->m_arrCharRects.GetConstArray() ;
		const SGLPalette *		pCharBack = plv->m_arrCharBack.GetConstArray() ;
		const size_t			nChars = plv->m_arrCharRects.GetLength() ;
		ESLAssert( nChars <= plv->m_arrCharBack.GetLength() ) ;
		for ( size_t i = 0; i < nChars; i ++ )
		{
			if ( pCharBack[i].ui32 == 0 )
			{
				continue ;
			}
			render.FillRectangle
				( plv->m_ptView.x + pCharRects[i].x - m_xScroll,
					plv->m_ptView.y + pCharRects[i].y - m_yScrollOffset,
					pCharRects[i].w, pCharRects[i].h, pCharBack[i].ui32 ) ;
		}
	}
	//
	// 文字描画
	//
	for ( i = 0; i < m_arrLineViewBuf.GetLength(); i ++ )
	{
		LineView *	plv = m_arrLineViewBuf.GetAt( i ) ;
		if ( plv == nullptr )
		{
			continue ;
		}
		if ( !plv->m_imgView.GetImageSize().IsEmpty() )
		{
			SGLPaintParam	pp ;
			pp.ptPaint = plv->m_ptView + plv->m_ptOffset ;
			pp.ptPaint.x -= m_xScroll ;
			pp.ptPaint.y -= m_yScrollOffset ;
			render.DrawImage( pp, &(plv->m_imgView) ) ;
		}
	}
	//
	// カレット描画
	//
	if ( HasKeyFocus() )
	{
		SGLImageRect	irCaret ;
		if ( GetCaretRect( irCaret ) )
		{
			unsigned int	t = 0 ;
			if ( m_styleEdit.nCaretBlinkInterval > 0 )
			{
				t = m_msecCaret * 0x200 / m_styleEdit.nCaretBlinkInterval ;
				if ( t > 0x100 )
				{
					t = 0x200 - t ;
				}
			}
			uint32_t	a = 0x100 - t ;
			render.FillRectangle
				( irCaret.x, irCaret.y,
					irCaret.w, irCaret.h,
					m_styleEdit.rgbaCaretColor.imul(a) ) ;
		}
	}
	//
	// 子スプライトを手前に描画
	//
	SGLSprite::DrawChildren( render, s3dView ) ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::IsHitSprite( double x, double y ) const
{
	if ( !m_visible || (m_paramView.nTransparency >= 0x100) )
	{
		return	false ;
	}
	if ( (m_rectClickable.x <= x)
		& (x - m_rectClickable.x < m_rectClickable.w)
		& (m_rectClickable.y <= y)
		& (y - m_rectClickable.y < m_rectClickable.h) )
	{
		return	true ;
	}
	SGLSize	sizeViewBuf = GetImageSize() ;
	if ( (x >= 0) && (x < sizeViewBuf.w)
		&& (y >= 0) && (y < sizeViewBuf.h) )
	{
		return	true ;
	}
	return	false ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEdit::InvokeCommand
	( const SSystem::SXMLDocument& xmlCmd,
		SSystem::SXMLDocument * pxmlResult )
{
	if ( xmlCmd.GetTag() == L"option" )
	{
		SString *	pDeny = xmlCmd.GetAttributeAs( L"deny" ) ;
		if ( pDeny != nullptr )
		{
			static const wchar_t *	pwszDenyIDs[] =
			{
				L"return", L"tab", L"alphabet", L"number",
				L"8bit_char", L"mbchar", nullptr
			} ;
			static const uint32_t	nDenyFlags[] =
			{
				SGLSpriteEdit::editAcceptReturn,
				SGLSpriteEdit::editAcceptTab,
				SGLSpriteEdit::editDenyAlphabet,
				SGLSpriteEdit::editDenyNumber,
				SGLSpriteEdit::editDeny8bitChar,
				SGLSpriteEdit::editDenyMBChar,
			} ;
			static const bool	fAcceptFlags[] =
			{
				true, true, false, false,
				false, false,
			} ;
			SStringParser	sparsDeny ;
			sparsDeny.AttachString( *pDeny ) ;
			while ( sparsDeny.PassSpace() )
			{
				SString	strDeny ;
				sparsDeny.NextString( strDeny ) ;
				for ( size_t i = 0; pwszDenyIDs[i]; i ++ )
				{
					if ( strDeny == pwszDenyIDs[i] )
					{
						if ( fAcceptFlags[i] )
						{
							m_styleEdit.nEditFlags &= ~nDenyFlags[i] ;
						}
						else
						{
							m_styleEdit.nEditFlags |= nDenyFlags[i] ;
						}
					}
				}
			}
		}
		SString *	pAccept = xmlCmd.GetAttributeAs( L"accept" ) ;
		if ( pAccept != nullptr )
		{
			static const wchar_t *	pwszAcceptIDs[] =
			{
				L"return", L"tab",
			} ;
			static const uint32_t	nAcceptFlags[] =
			{
				SGLSpriteEdit::editAcceptReturn,
				SGLSpriteEdit::editAcceptTab,
			} ;
			SStringParser	sparsAccept ;
			sparsAccept.AttachString( *pAccept ) ;
			while ( sparsAccept.PassSpace() )
			{
				SString	strAccept ;
				sparsAccept.NextString( strAccept ) ;
				for ( size_t i = 0; pwszAcceptIDs[i]; i ++ )
				{
					if ( strAccept == pwszAcceptIDs[i] )
					{
						m_styleEdit.nEditFlags |= nAcceptFlags[i] ;
					}
				}
			}
		}
		int	fWordwrap = ((m_styleEdit.nEditFlags & SGLSpriteEdit::editLineWrap) ? -1 : 0 ) ;
		if ( xmlCmd.GetAttrIntegerAs( L"wordwrap", fWordwrap ) )
		{
			m_styleEdit.nEditFlags |= SGLSpriteEdit::editLineWrap ;
		}
		else
		{
			m_styleEdit.nEditFlags &= ~SGLSpriteEdit::editLineWrap ;
		}
		//
		m_styleEdit.context.pitchTab =
			(int32_t) xmlCmd.GetAttrRichIntegerAs
						( L"tab", m_styleEdit.context.pitchTab ) ;
		//
		int	fAutoIndent =
			((m_styleEdit.nEditFlags & SGLSpriteEdit::editAutoIndent) ? 1 : 0 ) ;
		if ( xmlCmd.GetAttrIntegerAs( L"auto_indent", fAutoIndent ) )
		{
			m_styleEdit.nEditFlags |= SGLSpriteEdit::editAutoIndent ;
		}
		else
		{
			m_styleEdit.nEditFlags &= ~SGLSpriteEdit::editAutoIndent ;
		}
		//
		int	fLineIndent =
			((m_styleEdit.nEditFlags & SGLSpriteEdit::editMultiLineTab) ? 1 : 0 ) ;
		if ( xmlCmd.GetAttrIntegerAs( L"line_tab_indent", fLineIndent ) )
		{
			m_styleEdit.nEditFlags |= SGLSpriteEdit::editMultiLineTab ;
		}
		else
		{
			m_styleEdit.nEditFlags &= ~SGLSpriteEdit::editMultiLineTab ;
		}
	}
	return	SGLSprite::InvokeCommand( xmlCmd, pxmlResult ) ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_fMouseLDown )
	{
		ssize_t	iChar =
			GetCharIndexFromPosOfView( (int) xPos, (int) yPos ) ;
		if ( iChar >= 0 )
		{
			if ( m_fMouseLDblClk )
			{
				size_t	iFirst, iEnd ;
				FindWordRange( iFirst, iEnd, iChar ) ;
				//
				if ( iFirst > m_iDownFirstChar )
				{
					iFirst = m_iDownFirstChar ;
				}
				if ( iEnd < m_iDblClkEndChar )
				{
					iEnd = m_iDblClkEndChar ;
				}
				if ( (m_iSelFirst == iFirst)
						&& (m_iSelEnd == iEnd) )
				{
					return	true ;
				}
				m_fMouseLDownMoved = true ;
				SetSel( (ssize_t) iFirst, (ssize_t) iEnd ) ;
			}
			else
			{
				m_fMouseLDownMoved = ((ssize_t) m_iDownFirstChar != iChar) ;
				//
				if ( (size_t) iChar >= m_iDownFirstChar )
				{
					if ( (m_iSelFirst == m_iDownFirstChar)
							&& (m_iSelEnd == (size_t) iChar) )
					{
						return	true ;
					}
				}
				else
				{
					if ( (m_iSelFirst == (size_t) iChar)
							&& (m_iSelEnd == m_iDownFirstChar) )
					{
						return	true ;
					}
				}
				SetSel( (ssize_t) m_iDownFirstChar, iChar ) ;
			}
			return	true ;
		}
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteEdit::OnMouseLeave( int64_t nFlags )
{
	m_fMouseLDown = false ;
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// マウスカーソル取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteEdit::HitTestMouseCursor
	( double xPos, double yPos, int64_t nFlags )
{
	return	L"IDC_IBEAM" ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	ScrollDeltaVertical( - zDelta * 4 / WheelDeltaUnit ) ;
	return	true ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	SetKeyFocus() ;
	//
	ssize_t	iChar =
		GetCharIndexFromPosOfView( (int) xPos, (int) yPos ) ;
	if ( (iChar < 0) && (GetLineCount() > 0) )
	{
		size_t			nLastLine = GetLineCount() - 1 ;
		size_t			iLastLine = GetLineIndex( nLastLine ) ;
		SGLImageRect	rectChar ;
		if ( GetCharacterPosOfView( rectChar, iLastLine ) )
		{
			size_t	nLastLines = GetLineCountOfView( nLastLine ) ;
			iChar = GetCharIndexFromPosOfView
				( (int) xPos,
					(int) (rectChar.y + rectChar.h / 2
							+ (nLastLines - 1) * GetLineHeight()) ) ;
		}
	}
	else if ( (iChar > 0) && (iChar == m_strEdit.GetLength() - 1) )
	{
		SGLImageRect	rectChar ;
		if ( GetCharacterPosOfView( rectChar, iChar ) )
		{
			if ( rectChar.x + rectChar.w <= xPos )
			{
				iChar = (ssize_t) m_strEdit.GetLength() ;
			}
		}
	}
	if ( iChar >= 0 )
	{
		SetSel( iChar, iChar ) ;
		SetMouseCapture() ;
		m_fMouseLDown = true ;
		m_fMouseLDblClk = false ;
		m_fMouseLDownMoved = false ;
		m_iDownFirstChar = (size_t) iChar ;
		return	true ;
	}
	return	SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLSpriteEdit::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_fMouseLDown )
	{
		ReleaseMouseCapture() ;
		m_fMouseLDown = false ;
		//
		if ( !m_fMouseLDownMoved && UI::IsPlatformTablet() )
		{
			CallEditMessageBox *	pcemb = new CallEditMessageBox ;
			pcemb->m_refEdit.SetReference( this ) ;
			pcemb->m_strEdit = m_strEdit ;
			pcemb->m_strCaption = m_strItemName ;
			pcemb->m_nStyles = 0 ;
			if ( (m_styleEdit.nEditFlags
						& editMultiLineMask) == editMultiLine )
			{
				pcemb->m_nStyles |= editboxStyleMultiLine ;
			}
			if ( m_styleEdit.nEditFlags & editNumber )
			{
				pcemb->m_nStyles |= editboxStyleNumber ;
			}
			if ( m_styleEdit.nEditFlags & editPassword )
			{
				pcemb->m_nStyles |= editboxStylePassword ;
			}
			SThread::BeginStockThread
				( &SGLSpriteEdit::CallEditMessageBoxProc, pcemb ) ;
		}
		else
		{
			SetKeyFocus() ;
		}
	}
	return	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
}

bool SGLSpriteEdit::OnLButtonDblClk
	( double xPos, double yPos, int64_t nFlags )
{
	ssize_t	iChar =
		GetCharIndexFromPosOfView( (int) xPos, (int) yPos ) ;
	if ( iChar >= 0 )
	{
		size_t	iFirst, iEnd ;
		FindWordRange( iFirst, iEnd, iChar ) ;
		SetSel( (ssize_t) iFirst, (ssize_t) iEnd ) ;
		//
		SetMouseCapture() ;
		m_fMouseLDown = true ;
		m_fMouseLDblClk = true ;
		m_fMouseLDownMoved = false ;
		m_iDownFirstChar = iFirst ;
		m_iDblClkEndChar = iEnd ;
		return	true ;
	}
	return	SGLSprite::OnLButtonDblClk( xPos, yPos, nFlags ) ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnRButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_pMenu != nullptr )
	{
		m_pMenu->DeleteMenu() ;
		m_pMenu = nullptr ;
	}
	ssize_t	iChar =
		GetCharIndexFromPosOfView( (int) xPos, (int) yPos ) ;
	if ( iChar >= 0 )
	{
		if ( ((size_t) iChar < m_iSelFirst)
				|| (m_iSelEnd <= (size_t) iChar) )
		{
			SetSel( iChar, iChar ) ;
		}
		return	true ;
	}
	return	SGLSprite::OnRButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLSpriteEdit::OnRButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	S2DDVector			vPos( xPos, yPos ) ;
	SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this, &vPos ) ;
	if ( pWindow != nullptr )
	{
		WindowMenu::Entry	wmeItems[8] =
		{
			{	0, L"切り取り\tCtrl+X", L"ID_EDIT_CUT", nullptr, 0	},
			{	0, L"コピー\tCtrl+C", L"ID_EDIT_COPY", nullptr, 0	},
			{	0, L"貼り付け\tCtrl+V", L"ID_EDIT_PASTE", nullptr, 0	},
			{	0, L"削除\tDel", L"ID_EDIT_CLEAR", nullptr, 0	},
			{	0, L"全て選択\tCtrl+A", L"ID_EDIT_SELECT_ALL", nullptr, 0	},
			{	WindowMenu::flagSeparator, nullptr, nullptr, nullptr, 0	},
			{	0, L"元に戻す\tCtrl+Z", L"ID_EDIT_UNDO", nullptr, 0	},
			{	0, L"やり直し\tCtrl+Y", L"ID_EDIT_REDO", nullptr, 0	},
		} ;
		SGLWindowMenu *	pMenu = new SGLWindowMenu ;
		m_pMenu = pMenu ;
		if ( !CanCutText() )
		{
			wmeItems[0].nFlags |= WindowMenu::flagDisabled ;
			wmeItems[3].nFlags |= WindowMenu::flagDisabled ;
		}
		if ( !CanCopyText() )
		{
			wmeItems[1].nFlags |= WindowMenu::flagDisabled ;
		}
		if ( !CanPasteText() )
		{
			wmeItems[2].nFlags |= WindowMenu::flagDisabled ;
		}
		if ( !CanUndo() )
		{
			wmeItems[6].nFlags |= WindowMenu::flagDisabled ;
		}
		if ( !CanRedo() )
		{
			wmeItems[7].nFlags |= WindowMenu::flagDisabled ;
		}
		pMenu->CreatePopupMenu( wmeItems, 8 ) ;
		pMenu->ShowPopupMenu( pWindow, vPos.x, vPos.y ) ;
		SetKeyFocus() ;
		return	true ;
	}
	return	SGLSprite::OnRButtonDown( xPos, yPos, nFlags ) ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnKeyDown( int64_t nVirtKey, int64_t nFlags )
{
	if ( nFlags & vkeyContextControl )
	{
		if ( nVirtKey == 'X' )
		{
			DoCut() ;
			return	true ;
		}
		else if ( nVirtKey == 'C' )
		{
			DoCopy() ;
			return	true ;
		}
		else if ( nVirtKey == 'V' )
		{
			DoPaste() ;
			return	true ;
		}
		else if ( nVirtKey == 'Z' )
		{
			Undo() ;
			return	true ;
		}
		else if ( nVirtKey == 'Y' )
		{
			Redo() ;
			return	true ;
		}
		else if ( nVirtKey == 'A' )
		{
			SetSel( 0, -1 ) ;
			return	true ;
		}
	}
	int		nEditFlags = m_styleEdit.nEditFlags ;
	bool	fCurMove = false ;
	size_t	iCur = m_iCursor ;
	if ( nVirtKey == vkeyTab )
	{
		// TAB
		if ( nEditFlags & editAcceptTab )
		{
			size_t	iSelFirst = GetLineFromIndex( m_iSelFirst ) ;
			size_t	iSelEnd = GetLineFromIndex( m_iSelEnd ) ;
			if ( (nEditFlags & editMultiLineTab) && (iSelFirst != iSelEnd) )
			{
				if ( GetLineIndex(iSelEnd) == m_iSelEnd )
				{
					iSelEnd -- ;
				}
				SString	strLines ;
				for ( size_t i = iSelFirst; i <= iSelEnd; i ++ )
				{
					SString	strLine = GetLineText( i ) ;
					if ( nFlags & vkeyContextShift )
					{
						if ( strLine.GetAt(0) == L'\t' )
						{
							strLines += strLine.Middle( 1 ) ;
						}
						else
						{
							strLines += strLine ;
						}
					}
					else
					{
						strLines += L"\t"  ;
						strLines += strLine ;
					}
				}
				SetSel( (ssize_t) GetLineIndex(iSelFirst),
						(ssize_t) GetLineIndex(iSelEnd+1) ) ;
				DoReplace( strLines ) ;
				SetSel( (ssize_t) GetLineIndex(iSelFirst),
							(ssize_t) GetLineIndex(iSelEnd+1) ) ;
			}
			else
			{
				DoReplace( L"\t" ) ;
			}
			return	true ;
		}
	}
	else if ( nVirtKey == vkeyReturn )
	{
		// 改行
		if ( nEditFlags & editAcceptReturn )
		{
			SString	strReturn = L"\r\n" ;
			if ( nEditFlags & editAutoIndent )
			{
				size_t	nLine = GetLineFromIndex( m_iCursor ) ;
				size_t	iLineFirst = GetLineIndex( nLine ) ;
				size_t	iLineEnd = GetLineIndex( nLine + 1 ) ;
				for ( size_t i = iLineFirst; i < iLineEnd; i ++ )
				{
					wchar_t	wch = m_strEdit.GetAt( i ) ;
					if ( (wch == L' ') || (wch == L'\t') )
					{
						strReturn += wch ;
					}
					else
					{
						break ;
					}
				}
			}
			DoReplace( strReturn ) ;
			return	true ;
		}
	}
	else if ( nVirtKey == vkeyLeft )
	{
		// ←
		if ( m_iCursor > 0 )
		{
			if ( (iCur >= 2)
				&& (m_strEdit.GetAt( iCur - 1 ) == L'\n')
				&& (m_strEdit.GetAt( iCur - 2 ) == L'\r') )
			{
				iCur -- ;
			}
			iCur -- ;
			//
			if ( nFlags & vkeyContextControl )
			{
				size_t	iFirst, iEnd ;
				FindWordRange( iFirst, iEnd, iCur ) ;
				iCur = iFirst ;
			}
		}
		m_xCursor = -1 ;
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyUp )
	{
		// ↑
		bool	fMove = true ;
		if ( nFlags & vkeyContextControl )
		{
			ScrollDeltaVertical( -1 ) ;
			//
			SGLImageRect	rectCur ;
			if ( GetCharacterPosOfView( rectCur, m_iCursor )
				&& (rectCur.y + rectCur.h < GetImageSize().h) )
			{
				fMove = false ;
			}
		}
		if ( fMove )
		{
			SGLImageRect	rctCur ;
			SGLPoint	ptCur ;
			if ( GetCharacterPosOfView( rctCur, m_iCursor ) )
			{
				ptCur.x = rctCur.x ;
				ptCur.y = rctCur.y + rctCur.h / 2 ;
			}
			else
			{
				ptCur = GetCharPosFromIndex( m_iCursor ) ;
			}
			if ( m_xCursor < 0 )
			{
				m_xCursor = ptCur.x ;
			}
			else
			{
				ptCur.x = m_xCursor ;
			}
			const int	nLineHeight = (int) GetLineHeight() ;
			if ( ptCur.y < nLineHeight )
			{
				if ( (m_yViewLine > 0)
					|| (m_iViewLineOffset > 0) || (m_yScrollOffset > 0) )
				{
					ScrollDeltaVertical( -1 ) ;
					ssize_t	iChar =
						GetCharIndexFromPosOfView( ptCur.x, ptCur.y ) ;
					if ( iChar >= 0 )
					{
						iCur = (size_t) iChar ;
					}
				}
			}
			else
			{
				ssize_t	iChar =
					GetCharIndexFromPosOfView( ptCur.x, ptCur.y - nLineHeight ) ;
				if ( iChar >= 0 )
				{
					iCur = (size_t) iChar ;
				}
			}
			fCurMove = true ;
		}
	}
	else if ( nVirtKey == vkeyRight )
	{
		// →
		if ( m_iCursor < m_strEdit.GetLength() )
		{
			if ( nFlags & vkeyContextControl )
			{
				size_t	iFirst, iEnd ;
				FindWordRange( iFirst, iEnd, iCur ) ;
				iCur = iEnd ;
			}
			else
			{
				if ( (iCur + 1 < m_strEdit.GetLength())
					&& (m_strEdit.GetAt( iCur ) == L'\r')
					&& (m_strEdit.GetAt( iCur + 1 ) == L'\n') )
				{
					iCur ++ ;
				}
				iCur ++ ;
			}
		}
		m_xCursor = -1 ;
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyDown )
	{
		// ↓
		bool	fMove = true ;
		if ( nFlags & vkeyContextControl )
		{
			ScrollDeltaVertical( 1 ) ;
			//
			SGLImageRect	rectCur ;
			if ( GetCharacterPosOfView( rectCur, m_iCursor )
				&& (rectCur.y >= 0) )
			{
				fMove = false ;
			}
		}
		if ( fMove )
		{
			SGLImageRect	rctCur ;
			SGLPoint	ptCur ;
			if ( GetCharacterPosOfView( rctCur, m_iCursor ) )
			{
				ptCur.x = rctCur.x ;
				ptCur.y = rctCur.y + rctCur.h / 2 ;
			}
			else
			{
				ptCur = GetCharPosFromIndex( m_iCursor ) ;
			}
			if ( m_xCursor < 0 )
			{
				m_xCursor = ptCur.x ;
			}
			else
			{
				ptCur.x = m_xCursor ;
			}
			const int	nLineHeight = (int) GetLineHeight() ;
			ssize_t	iChar =
				GetCharIndexFromPosOfView( ptCur.x, ptCur.y + nLineHeight ) ;
			if ( (iChar < 0)
				&& (GetLineFromIndex( m_iCursor ) + 1 < GetLineCount()) )
			{
				ScrollDeltaVertical( 1 ) ;
				iChar = GetCharIndexFromPosOfView( ptCur.x, ptCur.y ) ;
			}
			if ( iChar >= 0 )
			{
				iCur = (size_t) iChar ;
			}
			fCurMove = true ;
		}
	}
	else if ( nVirtKey == vkeyPageUp )
	{
		// PageUp
		int	nPageLines = GetImageSize().h / (int) GetLineHeight() / 2 ;
		if ( nPageLines <= 4 )
		{
			nPageLines = 4 ;
		}
		SGLPoint	ptCur = GetCharPosFromIndex( m_iCursor ) ;
		ScrollDeltaVertical( - nPageLines ) ;
		ssize_t	iChar = GetCharIndexFromPosOfView( ptCur.x, ptCur.y ) ;
		if ( iChar >= 0 )
		{
			iCur = (size_t) iChar ;
		}
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyPageDown )
	{
		// PageDown
		int	nPageLines = GetImageSize().h / (int) GetLineHeight() / 2 ;
		if ( nPageLines <= 4 )
		{
			nPageLines = 4 ;
		}
		SGLPoint	ptCur = GetCharPosFromIndex( m_iCursor ) ;
		ScrollDeltaVertical( nPageLines ) ;
		ssize_t	iChar = GetCharIndexFromPosOfView( ptCur.x, ptCur.y ) ;
		if ( iChar >= 0 )
		{
			iCur = (size_t) iChar ;
		}
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyEnd )
	{
		// End
		size_t	iNextLine =
			GetLineIndex( GetLineFromIndex( m_iCursor ) + 1 ) ;
		if ( iNextLine >= 1 )
		{
			if ( (iNextLine >= 2)
				&& (m_strEdit.GetAt( iNextLine - 1 ) == L'\n')
				&& (m_strEdit.GetAt( iNextLine - 2 ) == L'\r') )
			{
				iNextLine -- ;
			}
			iCur = iNextLine - 1 ;
		}
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyHome )
	{
		// Home
		iCur = GetLineIndex( GetLineFromIndex( m_iCursor ) ) ;
		fCurMove = true ;
	}
	else if ( nVirtKey == vkeyDelete )
	{
		// Del
		if ( m_iCursor < m_strEdit.GetLength() )
		{
			if ( m_iSelFirst == m_iSelEnd )
			{
				if ( (iCur + 1 < m_strEdit.GetLength())
					&& (m_strEdit.GetAt( iCur ) == L'\r')
					&& (m_strEdit.GetAt( iCur + 1 ) == L'\n') )
				{
					iCur ++ ;
				}
				SetSel( (ssize_t) m_iCursor, (ssize_t) iCur + 1 ) ;
			}
			DoReplace( L"" ) ;
		}
		return	true ;
	}
	else if ( nVirtKey == vkeyBack )
	{
		// BackSpace
		if ( m_iCursor > 0 )
		{
			if ( m_iSelFirst == m_iSelEnd )
			{
				if ( (iCur >= 2)
					&& (m_strEdit.GetAt( iCur - 1 ) == L'\n')
					&& (m_strEdit.GetAt( iCur - 2 ) == L'\r') )
				{
					iCur -- ;
				}
				SetSel( (ssize_t) m_iCursor, (ssize_t) iCur - 1 ) ;
			}
			DoReplace( L"" ) ;
		}
		return	true ;
	}
	if ( fCurMove )
	{
		if ( nFlags & vkeyContextShift )
		{
			if ( m_iCursor == m_iSelEnd )
			{
				SetSel( (ssize_t) m_iSelFirst, (ssize_t) iCur ) ;
			}
			else
			{
				SetSel( (ssize_t) m_iSelEnd, (ssize_t) iCur ) ;
			}
		}
		else
		{
			SetSel( (ssize_t) iCur, (ssize_t) iCur ) ;
		}
		return	true ;
	}
	return	SGLSprite::OnKeyDown( nVirtKey, nFlags ) ;
}

bool SGLSpriteEdit::OnKeyUp( int64_t nVirtKey, int64_t nFlags )
{
	return	SGLSprite::OnKeyUp( nVirtKey, nFlags ) ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnChar( uint16_t codeChar )
{
	if ( codeChar >= 0x20 )
	{
		SString	strChar ;
		uint16_t *	pwBuf = strChar.LockBuffer( 1 ) ;
		pwBuf[0] = codeChar ;
		strChar.UnlockBuffer( 1 ) ;
		FilterInputText( strChar ) ;
		if ( !strChar.IsEmpty() )
		{
			DoReplace( strChar ) ;
		}
	}
	return	true ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnStartComposition( SGLInputStartComposition& iscForm )
{
	iscForm.nFlags = SGLInputStartComposition::flagPosition
					| SGLInputStartComposition::flagRectangle
					| SGLInputStartComposition::flagFont ;
	iscForm.ptStart = GetCharPosFromIndex( m_iCursor ) ;
	iscForm.rctArea.x = 0 ;
	iscForm.rctArea.y = 0 ;
	iscForm.rctArea.w = GetImageSize().w ;
	iscForm.rctArea.h = GetImageSize().h ;
	//
	if ( m_styleEdit.nEditFlags & editFontForIME )
	{
		iscForm.fsFontStyle = m_styleEdit.fontIME ;
	}
	else
	{
		iscForm.fsFontStyle = m_styleEdit.font ;
	}
	//
	S2DDVector	vPos( iscForm.ptStart.x, iscForm.ptStart.y ) ;
	LocalToGlobal( vPos ) ;
	iscForm.ptStart.x = (int32_t) vPos.x ;
	iscForm.ptStart.y = (int32_t) vPos.y ;
	//
	SGLRect	rectArea = iscForm.rctArea ;
	LocalToGlobalRect( rectArea ) ;
	iscForm.rctArea = rectArea ;
	//
	return	true ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnEndComposition( void )
{
	return	true ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::OnCompositionString
	( const SGLInputCompositionString& icsComp )
{
	if ( icsComp.nFlags & SGLInputCompositionString::flagResult )
	{
		SString	strComp = icsComp.pszComposition ;
		FilterInputText( strComp ) ;
		if ( !strComp.IsEmpty() )
		{
			DoReplace( strComp ) ;
		}
	}
	return	true ;
}

// コマンド発行処理（フォーカスアイテムへ）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteEdit::DispatchCommand
	( const wchar_t * pszCmd, int64_t nParam, int64_t nCode )
{
	SString	strCmd = pszCmd ;
	if ( strCmd == L"ID_EDIT_CUT" )
	{
		DoCut() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_COPY" )
	{
		DoCopy() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_PASTE" )
	{
		DoPaste() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_CLEAR" )
	{
		DoClear() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_UNDO" )
	{
		Undo() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_REDO" )
	{
		Redo() ;
		return	true ;
	}
	else if ( strCmd == L"ID_EDIT_SELECT_ALL" )
	{
		SetSel( 0, -1 ) ;
		return	true ;
	}
	return	false ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteEdit::GetLQClassName( void ) const
{
	return	L"EntisGLS4.EditSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteEdit::DuplicateObject( void )
{
	return	new SGLSpriteEdit( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEdit::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_styleEdit, sizeof(EditStyle) ) ;
	file.WriteString( m_strFontFace ) ;
	file.WriteString( m_strIMEFontFace ) ;
	file.WriteString( m_strItemName ) ;
	file.WriteString( m_strEdit ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEdit::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_styleEdit, sizeof(EditStyle) ) ;
	file.ReadString( m_strFontFace ) ;
	file.ReadString( m_strIMEFontFace ) ;
	file.ReadString( m_strItemName ) ;
	file.ReadString( m_strEdit ) ;
	//
	m_styleEdit.context.pwszProhibition = SGLLetteringContext::pwszDefProhibition ;
	m_styleEdit.font.pszFace = m_strFontFace ;
	m_styleEdit.fontIME.pszFace = m_strIMEFontFace ;
	//
	UpdateTextIndex() ;
	UpdateTextImage() ;
	PostUpdate() ;
	//
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// エディット・カラム（行番号表示・エディット領域サイズ調整）
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEditColumn::ColumnStyle::ColumnStyle( void )
{
	minLineNumWidth = 0 ;
	leftMargin = 0 ;
	rightMargin = 0 ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEditColumn::ColumnStyle::ColumnStyle
	( const SGLSpriteEditColumn::ColumnStyle& style )
{
	memmove( this, &style, sizeof(ColumnStyle) ) ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteEditColumn::ColumnStyle&
	SGLSpriteEditColumn::ColumnStyle::operator =
			( const SGLSpriteEditColumn::ColumnStyle& style )
{
	memmove( this, &style, sizeof(ColumnStyle) ) ;
	return	*this ;
}

// Listener クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteEditColumn::Listener, SObject )

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLSpriteEditColumn, SGLSprite, SGLSpriteEditListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEditColumn::SGLSpriteEditColumn( void )
{
	m_flagsUI |= uiFocusable ;
	m_pEdit = nullptr ;
	m_flagOwnEdit = false ;
	m_nMaxNumWidth = 8 ;
	m_nLineNumWidth = 0 ;
	m_flagUpdateColumn = false ;
}

SGLSpriteEditColumn::SGLSpriteEditColumn( const SGLSpriteEditColumn& src )
{
	m_pEdit = nullptr ;
	m_flagOwnEdit = false ;
	m_nMaxNumWidth = 8 ;
	m_nLineNumWidth = 0 ;
	//
	SetColumnStyle( src.m_styleCol ) ;
	//
	if ( src.m_pEdit != nullptr )
	{
		SGLSpriteEdit *	pEdit =
			ESLSmartCast<SGLSpriteEdit>
				( src.m_pEdit->DuplicateObject() ) ;
		if ( pEdit != nullptr )
		{
			AttachEdit( pEdit, true ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteEditColumn::~SGLSpriteEditColumn( void )
{
	DetachEdit() ;
}

// エディットアイテム設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEditColumn::AttachEdit
	( SGLSpriteEdit * pEdit, bool fAutoDelete )
{
	DetachEdit() ;
	//
	if ( pEdit == nullptr )
	{
		return	sglErrFailed ;
	}
	Lock() ;
	AddChild( pEdit ) ;
	m_pEdit = pEdit ;
	m_flagOwnEdit = fAutoDelete ;
	//
	SGLSpriteEditListener * pListener = pEdit->GetEditListener() ;
	if ( (pListener != nullptr)
		&& (pListener != (SGLSpriteEditListener*) this) )
	{
		m_refEditListener = pListener ;
	}
	pEdit->AttachEditListener( this ) ;
	Unlock() ;
	//
	return	sglErrSuccess ;
}

// エディットアイテム解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEditColumn::DetachEdit( void )
{
	Lock() ;
	SGLSpriteEditListener * pListener = m_refEditListener ;
	if ( m_pEdit != nullptr )
	{
		m_pEdit->AttachEditListener( pListener ) ;
		DetachChild( m_pEdit ) ;
		if ( m_flagOwnEdit )
		{
			delete	m_pEdit ;
		}
		m_pEdit = nullptr ;
		m_flagOwnEdit = false ;
	}
	m_refEditListener.SetReference( nullptr ) ;
	Unlock() ;
	return	sglErrSuccess ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::AttachEditListener
			( SGLSpriteEditListener * pListener )
{
	Lock() ;
	m_refEditListener = pListener ;
	Unlock() ;
}

void SGLSpriteEditColumn::AttachColumnListener
			( SGLSpriteEditColumn::Listener * pListener )
{
	Lock() ;
	m_refColListener = pListener ;
	Unlock() ;
}

// サイズ変更
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::AdjustSize( int nWidth, int nHeight )
{
	m_sizeFrame.w = nWidth ;
	m_sizeFrame.h = nHeight ;
	//
	AdjustColumnLayout() ;
}

// 行番号表示設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::SetColumnStyle
	( const SGLSpriteEditColumn::ColumnStyle& style )
{
	m_styleCol = style ;
	m_strFont = style.font.pszFace ;
	m_styleCol.font.pszFace = m_strFont ;
	//
	SGLFont	font ;
	font.SetStyle( m_styleCol.font ) ;
	//
	SGLFontMetrics	metrics ;
	SGLSize			sizeMaxNum( 0, 0 ) ;
	m_nMaxNumWidth = 0 ;
	//
	for ( wchar_t i = 0; i <= 9; i ++ )
	{
		if ( !font.GetFontImage
			( &(m_ncImage[i].m_imgChar),
				m_ncImage[i].m_metrics, L'0' + i, 0xFFFFFFFF ) )
		{
			if ( m_ncImage[i].m_metrics.nWidth > m_nMaxNumWidth )
			{
				m_nMaxNumWidth = m_ncImage[i].m_metrics.nWidth ;
			}
			SGLSize	sizeChar = m_ncImage[i].m_imgChar.GetImageSize() ;
			if ( sizeMaxNum.w < sizeChar.w )
			{
				sizeMaxNum.w = sizeChar.w ;
			}
			if ( sizeMaxNum.h < sizeChar.h )
			{
				sizeMaxNum.h = sizeChar.h ;
			}
		}
	}
	m_imgNumBuffer.CreateImage
		( sizeMaxNum.w, sizeMaxNum.h, formatImageARGB, 32 ) ;
	//
	AdjustColumnLayout() ;
}

// 更新
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::SetUpdateColumn( void )
{
	m_flagUpdateColumn = true ;
	PostUpdate( nullptr ) ;
}

// レイアウト調整
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::AdjustColumnLayout( void )
{
	if ( (m_pEdit == nullptr) || m_sizeFrame.IsEmpty() )
	{
		return ;
	}
	size_t	nLineCount = m_pEdit->GetLineCount() ;
	size_t	nColChars = 1 ;
	for ( size_t n = 10; nLineCount >= n; n *= 10 )
	{
		nColChars ++ ;
	}
	m_nLastLineCount = nLineCount ;
	m_nLineNumWidth = nColChars * m_nMaxNumWidth ;
	if ( m_nLineNumWidth < m_styleCol.minLineNumWidth )
	{
		m_nLineNumWidth = m_styleCol.minLineNumWidth ;
	}
	int	nColWidth = (int) m_nLineNumWidth
					+ m_styleCol.leftMargin + m_styleCol.rightMargin ;
	//
	m_pEdit->SetPosition( nColWidth, 0 ) ;
	//
	if ( (m_sizeFrame.w > nColWidth) && (m_sizeFrame.h > 0) )
	{
		SGLSize	sizeEdit
			( (int) (m_sizeFrame.w - nColWidth), m_sizeFrame.h ) ;
		if ( sizeEdit != m_pEdit->GetImageSize() )
		{
			SGLSpriteEdit::EditStyle	styleEdit = m_pEdit->GetEditStyle() ;
			SString	strFontFace = styleEdit.font.pszFace ;
			styleEdit.font.pszFace = strFontFace ;
			//
			styleEdit.context.rectWritable.left = 0 ;
			styleEdit.context.rectWritable.top = 0 ;
			styleEdit.context.rectWritable.SetWidth( sizeEdit.w ) ;
			styleEdit.context.rectWritable.SetHeight( sizeEdit.h ) ;
			//
			Lock() ;
			m_pEdit->CreateBuffer
				( (uint32_t) sizeEdit.w, (uint32_t) sizeEdit.h ) ;
			m_pEdit->SetEditStyle( styleEdit ) ;
			Unlock() ;
		}
	}
	//
	SGLSize	sizeCol( nColWidth, m_sizeFrame.h ) ;
	if ( sizeCol != m_imgColumn.GetImageSize() )
	{
		m_imgColumn.CreateImage
			( (uint32_t) sizeCol.w, (uint32_t) sizeCol.h, formatImageARGB, 32 ) ;
		RedrawColumn() ;
	}
}

// 行番号描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::RedrawColumn( void )
{
	SGLPalette	argbZero( 0 ) ;
	m_imgColumn.FillImage( argbZero ) ;
	//
	Listener *	pListener = m_refColListener ;
	SGLPoint	ptScroll = m_pEdit->GetScrollPos() ;
	size_t		nLine = (size_t) ptScroll.y ;
	//
	for ( ; ; )
	{
		SGLImageRect	rectChar ;
		size_t	iLine = m_pEdit->GetLineIndex( nLine ) ;
		if ( (iLine >= m_pEdit->GetLength())
			|| !m_pEdit->GetCharacterPosOfView( rectChar, iLine ) )
		{
			break ;
		}
		uint32_t	argbNumColor = 0xFFFFFFFF ;
		if ( pListener != nullptr )
		{
			argbNumColor =
				pListener->OnDrawLineNumber
					( *this, *m_pEdit, iLine, &m_imgColumn, rectChar.y ).ui32 ;
		}
		DrawLineNum
			( &m_imgColumn,
				m_styleCol.leftMargin, rectChar.y,
				(int) nLine + 1, (int) m_nLineNumWidth, argbNumColor ) ;
		//
		if ( iLine >= m_pEdit->GetLength() )
		{
			break ;
		}
		nLine ++ ;
	}
	m_flagUpdateColumn = false ;
}

void SGLSpriteEditColumn::DrawLineNum
	( SGLImageObject * pDst,
		int xPos, int yPos, int nLineNum, int nWidth, uint32_t argbColor )
{
	//
	// 数値を各桁に分解し、幅を計算
	//
	int		nNum[16] ;
	size_t	nCol = 0 ;
	int		nCharsWidth = 0 ;
	int		nCharsHeight = 0 ;
	do
	{
		int	nColNum = nLineNum % 10 ;
		nNum[nCol ++] = nColNum ;
		nLineNum = (nLineNum - nColNum) / 10 ;
		nCharsWidth += m_ncImage[nColNum].m_metrics.nWidth ;
		if ( nCharsHeight < m_ncImage[nColNum].m_metrics.rctExterior.h )
		{
			nCharsHeight = m_ncImage[nColNum].m_metrics.rctExterior.h ;
		}
	}
	while ( (nLineNum != 0) && (nCol < 15) ) ;
	//
	// 各桁の文字を描画
	//
	SGLPalette	argbTemp = argbColor ;
	uint8_t		toneBlue[0x100], toneGreen[0x100],
				toneRed[0x100], toneAlpha[0x100] ;
	sglMakeBrightnessToneFilter
		( toneBlue, argbTemp.argb.Blue - 0x100 ) ;
	sglMakeBrightnessToneFilter
		( toneGreen, argbTemp.argb.Green - 0x100 ) ;
	sglMakeBrightnessToneFilter
		( toneRed, argbTemp.argb.Red - 0x100 ) ;
	sglMakeBrightnessToneFilter
		( toneAlpha, argbTemp.argb.Alpha - 0x100 ) ;
	//
	xPos += nWidth - nCharsWidth ;
	yPos += ((int) m_pEdit->GetLineHeight() - nCharsHeight) >> 1 ;
	for ( size_t i = 0; i < nCol; i ++ )
	{
		int	nColNum = nNum[nCol - i - 1] ;
		if ( argbColor == 0xFFFFFFFF )
		{
			pDst->BlendImage
				( &(m_ncImage[nColNum].m_imgChar),
					m_ncImage[nColNum].m_metrics.rctExterior.x + xPos,
					m_ncImage[nColNum].m_metrics.rctExterior.y + yPos ) ;
		}
		else
		{
			SGLSize	sizeChar = m_ncImage[nColNum].m_imgChar.GetImageSize() ;
			SGLImageRect
					rect( 0, 0, sizeChar.w, sizeChar.h ) ;
			//
			m_imgNumBuffer.CopyImage( &(m_ncImage[nColNum].m_imgChar) ) ;
			m_imgNumBuffer.ApplyToneFilter
				( toneRed, toneGreen, toneBlue, toneAlpha, &rect ) ;
			pDst->BlendImage
				( &m_imgNumBuffer,
					m_ncImage[nColNum].m_metrics.rctExterior.x + xPos,
					m_ncImage[nColNum].m_metrics.rctExterior.y + yPos, &rect ) ;
		}
		xPos += m_ncImage[nColNum].m_metrics.nWidth ;
	}
}

// 表示状態の子スプライトに対し BeforeDraw を呼び出し
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::BeforeDrawChildren( SGLSprite::Stereo3DView s3dView )
{
	SGLSprite::BeforeDrawChildren( s3dView ) ;
	//
	if ( m_flagUpdateColumn )
	{
		RedrawColumn() ;
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;
	//
	SGLPaintParam	pp ;
#if	!defined(__COTOPHA__)
	render.DrawImage( pp, m_imgColumn.GetImage() ) ;
#else
	render.DrawImage( pp, (SGLImageObject*) &m_imgColumn ) ;
#endif
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteEditColumn::DuplicateObject( void )
{
	return	new SGLSpriteEditColumn( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEditColumn::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_styleCol, sizeof(ColumnStyle) ) ;
	file.WriteString( m_strFont ) ;
	file.Write( &m_sizeFrame, sizeof(SGLSize) ) ;
	//
	int32_t	iEdit = -1 ;
	if ( m_pEdit != nullptr )
	{
		iEdit = (int32_t) FindChildSprite( m_pEdit ) ;
	}
	file.Write( &iEdit, sizeof(int32_t) ) ;
	//
	uint32_t	nFlags = 0 ;
	if ( m_flagOwnEdit )
	{
		nFlags |= 0x0001 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteEditColumn::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	ColumnStyle	styleCol ;
	SString		strFont ;
	SGLSize		sizeFrame ;
	file.Read( &styleCol, sizeof(ColumnStyle) ) ;
	file.ReadString( strFont ) ;
	//
	int32_t	iEdit = -1 ;
	file.Read( &iEdit, sizeof(int32_t) ) ;
	if ( iEdit >= 0 )
	{
		m_pEdit =
			ESLTypeCast<SGLSpriteEdit>( GetChildAt( (size_t) iEdit ) ) ;
		if ( m_pEdit != nullptr )
		{
			m_pEdit->AttachEditListener( this ) ;
		}
	}
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	//
	styleCol.font.pszFace = strFont ;
	SetColumnStyle( styleCol ) ;
	AdjustSize( sizeFrame.w, sizeFrame.h ) ;
	//
	return	sglErrSuccess ;
}

// ユーザー入力によってテキストが変更された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::OnChangedText( SGLSpriteEdit& edit )
{
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnChangedText( edit ) ;
	}
	m_flagUpdateColumn = true ;
	//
	if ( m_nLastLineCount != edit.GetLineCount() )
	{
		AdjustColumnLayout() ;
	}
}

// 選択範囲・カーソルが移動した
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::OnMovedCursor( SGLSpriteEdit& edit )
{
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnMovedCursor( edit ) ;
	}
}

// スクロールした
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::OnScrolled( SGLSpriteEdit& edit )
{
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->OnScrolled( edit ) ;
	}
	m_flagUpdateColumn = true ;
}

// テキスト入力フィルタ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteEditColumn::FilterInputText
	( SGLSpriteEdit& edit, SSystem::SString& strText )
{
	SGLSpriteEditListener *	pListener = m_refEditListener ;
	if ( pListener != nullptr )
	{
		pListener->FilterInputText( edit, strText ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 擬似コンソール・入出力ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteConsole::FileInterface, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteConsole::FileInterface::FileInterface( SGLSpriteConsole * pConsole )
	: m_refConsole( pConsole )
{
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLSpriteConsole::FileInterface::Duplicate( void ) const
{
	return	new FileInterface( m_refConsole ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteConsole::FileInterface::Read( void * ptrBuf, size_t nBytes )
{
	SGLSpriteConsole *	pConsole = m_refConsole ;
	if ( pConsole == nullptr )
	{
		return	0 ;
	}
	size_t	nReadBytes = 0 ;
	if ( pConsole->WaitForInput( 0 ) == sglErrSuccess )
	{
		nReadBytes = pConsole->ReadInput( ptrBuf, nBytes ) ;
	}
	else
	{
		pConsole->EnableInput( true ) ;
		pConsole->WaitForInput( Synchronism::Infinite ) ;
		nReadBytes = pConsole->ReadInput( ptrBuf, nBytes ) ;
		pConsole->EnableInput( false ) ;
	}
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteConsole::FileInterface::Write( const void * ptrBuf, size_t nBytes )
{
	SGLSpriteConsole *	pConsole = m_refConsole ;
	if ( pConsole == nullptr )
	{
		return	0 ;
	}
	return	pConsole->WriteString( ptrBuf, nBytes ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::FileInterface::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLSpriteConsole::FileInterface::GetLength( void ) const
{
	return	-1 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SGLSpriteConsole::FileInterface::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	-1 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLSpriteConsole::FileInterface::GetPosition( void ) const
{
	return	-1 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SGLSpriteConsole::FileInterface::SetEndOfFile( void )
{
	return	errFailed ;
}



//////////////////////////////////////////////////////////////////////////////
// 疑似コンソール・スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteConsole, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteConsole::SGLSpriteConsole( void )
{
	m_flagsUI |= uiFocusable ;
	//
	m_rgbaFont = 0xFFFFFFFF ;
	m_fontStyle.nSize = 16 ;
	m_fontStyle.nStyles = SGLFontStyle::styleNoSmooth ;
	m_fontStyle.pszFace = SGLFontStyle::FixedPitchFont ;
	m_font.SetStyle( m_fontStyle ) ;
	//
	m_nLineHeight = 16 ;
	m_nLimitLines = 256 ;
	//
	if ( UI::IsPlatformTablet() )
	{
		m_flagsConsole = flagScrollDrag ;
	}
	else
	{
		m_flagsConsole = flagScrollWheel ;
	}
	m_yValidInput = 0 ;
	//
	m_nInputable = 0 ;
	m_iCurLine = 0 ;
	m_iCurChar = 0 ;
	m_msecCurBlink = 0 ;
	m_rgbaCursor = 0xFFFFFFFF ;
	m_msecCurBlinkInterval = 1200 ;
	//
	m_yScroll = 0 ;
	//
	m_flagDragScroll = false ;
	//
	m_limitHistory = 31 ;
	//
	m_encoding = Charset::encodingUTF8 ;
	m_eventInput.Initialize( false ) ;
}

SGLSpriteConsole::SGLSpriteConsole( const SGLSpriteConsole& src )
	: SGLSprite( src )
{
	m_rgbaFont = src.m_rgbaFont ;
	m_fontStyle = src.m_fontStyle ;
	m_strFontFace = src.m_fontStyle.pszFace ;
	m_fontStyle.pszFace = m_strFontFace ;
	m_font.SetStyle( m_fontStyle ) ;
	//
	m_nLineHeight = src.m_nLineHeight ;
	m_nLimitLines = src.m_nLimitLines ;
	//
	m_flagsConsole = src.m_flagsConsole ;
	m_yValidInput = src.m_yValidInput ;
	//
	m_nInputable = 0 ;
	m_iCurLine = 0 ;
	m_iCurChar = 0 ;
	m_msecCurBlink = 0 ;
	m_rgbaCursor = src.m_rgbaCursor ;
	m_msecCurBlinkInterval = src.m_msecCurBlinkInterval ;
	//
	m_yScroll = 0 ;
	//
	m_flagDragScroll = false ;
	//
	m_limitHistory = src.m_limitHistory ;
	m_iRefHistory = -1 ;
	//
	m_encoding = src.m_encoding ;
	m_eventInput.Initialize( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteConsole::~SGLSpriteConsole( void )
{
}

// フォント設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::SetConsoleFont
	( const SGLFontStyle& style,
		uint32_t nLinePitch, SGLPalette rgbaColor )
{
	m_fontStyle = style ;
	m_strFontFace = style.pszFace ;
	m_fontStyle.pszFace = m_strFontFace ;
	//
	m_nLineHeight = nLinePitch ;
	m_rgbaFont = rgbaColor ;
	//
	m_font.SetStyle( m_fontStyle ) ;
}

// サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::SetConsoleSize
	( uint32_t nWidth, uint32_t nHeight, uint32_t yValidInput )
{
	SGLImageRect	rect( 0, 0, nWidth, nHeight ) ;
	CreateBuffer( nWidth, nHeight ) ;
	SetClickableRect( rect ) ;
	PostUpdate( nullptr ) ;
	//
	m_yValidInput = yValidInput ;
	if ( yValidInput == 0 )
	{
		if ( UI::IsPlatformTablet() )
		{
			m_yValidInput = nHeight / 2 ;
		}
		else
		{
			m_yValidInput = nHeight ;
		}
	}
}

// 文字エンコーディング設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::SetEncoding( SSystem::Charset::EncodingType encoding )
{
	m_encoding = encoding ;
}

// ファイルインターフェース生成
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * SGLSpriteConsole::NewFileInterface( void )
{
	return	new FileInterface( this ) ;
}

// 文字列出力
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::OutputString( const wchar_t * pwszString )
{
	if ( pwszString == nullptr )
	{
		return ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	while ( *pwszString )
	{
		size_t	iLine ;
		Line *	pLine = GetLastLine( iLine ) ;
		while ( *pwszString )
		{
			wchar_t	wch = *(pwszString ++) ;
			if ( wch < 0x20 )
			{
				switch ( wch )
				{
				case	'\t':
					if ( pLine->m_flagLineFeed )
					{
						pLine->m_strLine = L"" ;
					}
					pLine->m_strLine += wch ;
					break ;
				case	'\r':
					pLine->m_flagLineFeed = true ;
					break ;
				case	'\n':
					pLine->m_flagReturn = true ;
					break ;
				}
				if ( pLine->m_flagReturn )
				{
					break ;
				}
			}
			else
			{
				if ( pLine->m_flagLineFeed )
				{
					pLine->m_strLine = L"" ;
					pLine->m_flagLineFeed = false ;
				}
				pLine->m_strLine += wch ;
			}
		}
		pLine->m_nFixedChars = pLine->m_strLine.GetLength() ;
		if ( m_nInputable == 0 )
		{
			m_iCurLine = iLine ;
			m_iCurChar = pLine->m_nFixedChars ;
		}
		RasterizeLine( pLine ) ;
	}
	PostUpdate( nullptr ) ;
	ScrollToEndLine() ;
	Unlock() ;
}

// エンコードされた文字列出力
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteConsole::WriteString( const void * ptrBuf, size_t nBytes )
{
	SString	strText ;
	Charset::Decode
		( strText, m_encoding,
			(const uint8_t*) ptrBuf, (ssize_t) nBytes ) ;
	OutputString( strText ) ;
	return	nBytes ;
}

// 入力有効化
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SGLSpriteConsole::EnableInput( bool flagInput )
{
	atomic_int_t	nInputable ;
	if ( flagInput )
	{
		nInputable = AtomicAdd( &m_nInputable, 1 ) ;
		if ( nInputable == 1 )
		{
			if ( UI::IsPlatformTablet() )
			{
				SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
				if ( pWindow != nullptr )
				{
					pWindow->SetOptionalFlags
						( pWindow->GetOptionalFlags() | Window::flagOpenIME ) ;
				}
			}
			//
			Lock() ;
			Line *	pLine = GetLastLine( m_iCurLine ) ;
			pLine->m_nFixedChars = pLine->m_strLine.GetLength() ;
			m_iCurChar = pLine->m_nFixedChars ;
			//
			ScrollForInputCursor() ;
			m_iRefHistory = -1 ;
			Unlock() ;
		}
	}
	else if ( !flagInput )
	{
		nInputable = AtomicSub( &m_nInputable, 1 ) ;
		if ( nInputable == 0 )
		{
			Lock() ;
			SGLImageRect	rectCur ;
			if ( GetInputCursorRect( rectCur ) )
			{
				SGLRect	rect = rectCur ;
				PostUpdate( &rect ) ;
			}
			Unlock() ;
		}
	}
	return	nInputable ;
}

// 入力データ取得（非同期）
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteConsole::ReadInput( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	Lock() ;
	nReadBytes = m_qbufInput.Read( ptrBuf, nBytes ) ;
	if ( m_qbufInput.GetLength() == 0 )
	{
		m_eventInput.ResetSignal() ;
	}
	Unlock() ;
	return	nReadBytes ;
}

// 何らかのデータが入力されるまで待つ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteConsole::WaitForInput( int64_t msecTimeout )
{
	SError	err = m_eventInput.Wait( msecTimeout ) ;
	if ( err == errSuccess )
	{
		return	sglErrSuccess ;
	}
	else if ( err == errTimeout )
	{
		return	sglErrTimeout ;
	}
	return	sglErrFailed ;
}

// 末尾が画面内に収まるようスクロール
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::ScrollToEndLine( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSize	sizeConsole = GetImageSize() ;
	int		yEndLine = 0 ;
	for ( size_t i = 0; i < m_lines.GetLength(); i ++ )
	{
		Line *	pLine = m_lines.GetAt( i ) ;
		ESLAssert( pLine != nullptr ) ;
		if ( pLine != nullptr )
		{
			yEndLine += pLine->m_nHeight ;
		}
	}
	if ( yEndLine - m_yScroll > sizeConsole.h )
	{
		m_yScroll = yEndLine - sizeConsole.h ;
		PostUpdate( nullptr ) ;
	}
	Unlock() ;
}

// 入力カーソルが画面内に収まるようスクロール
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::ScrollForInputCursor( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	int	yCurLine = 0 ;
	for ( size_t i = 0; i < m_lines.GetLength(); i ++ )
	{
		Line *	pLine = m_lines.GetAt( i ) ;
		ESLAssert( pLine != nullptr ) ;
		if ( pLine != nullptr )
		{
			if ( i == m_iCurLine )
			{
				SGLImageRect *	pRect =
					pLine->m_aCharRects.GetAt( m_iCurChar ) ;
				if ( pRect == nullptr )
				{
					pRect = pLine->m_aCharRects.GetAt( m_iCurChar - 1 ) ;
				}
				if ( pRect != nullptr )
				{
					yCurLine += pRect->y ;
				}
				break ;
			}
			else
			{
				yCurLine += pLine->m_nHeight ;
			}
		}
	}
	if ( yCurLine < m_yScroll )
	{
		m_yScroll = yCurLine ;
		PostUpdate( nullptr ) ;
	}
	else if ( yCurLine + (int) m_nLineHeight - m_yScroll > (int) m_yValidInput )
	{
		m_yScroll = yCurLine + (int) m_nLineHeight - (int) m_yValidInput ;
		PostUpdate( nullptr ) ;
	}
	Unlock() ;
}

// 画面全消去
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::ClearAllLines( void )
{
	Lock() ;
	m_lines.RemoveAll() ;
	m_iCurLine = 0 ;
	m_iCurChar = 0 ;
	m_yScroll = 0 ;
	PostUpdate( nullptr ) ;
	Unlock() ;
}

// 入力履歴追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::AddInputHistory( const wchar_t * pwszHistory )
{
	Lock() ;
	for ( size_t i = 0; i < m_lstHistory.GetLength(); i ++ )
	{
		SString *	pStrHis = m_lstHistory.GetAt( i ) ;
		ESLAssert( pStrHis != nullptr ) ;
		if ( (pStrHis != nullptr) && (*pStrHis == pwszHistory) )
		{
			pStrHis = m_lstHistory.DetachAt( i ) ;
			m_lstHistory.InsertAt( 0, pStrHis ) ;
			Unlock() ;
			return ;
		}
	}
	m_lstHistory.InsertAt( 0, new SString( pwszHistory ) ) ;
	if ( m_lstHistory.GetLength() > m_limitHistory )
	{
		m_lstHistory.SetLength( m_limitHistory ) ;
	}
	Unlock() ;
}

// 履歴最大数設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::SetInputHistoryLimit( size_t nLimit )
{
	Lock() ;
	m_limitHistory = nLimit ;
	if ( m_lstHistory.GetLength() > m_limitHistory )
	{
		m_lstHistory.SetLength( m_limitHistory ) ;
	}
	Unlock() ;
}

// 入力文字列を追加する
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::InputString( const wchar_t * pwszString )
{
	ESLAssert( TestLocked() > 0 ) ;
	Line *	pLine = GetCursorLine() ;
	if ( (m_nInputable == 0) || (pLine == nullptr) )
	{
		return	false ;
	}
	SGLImageRect	rectCur ;
	if ( GetInputCursorRect( rectCur ) )
	{
		SGLRect	rect = rectCur ;
		PostUpdate( &rect ) ;
	}
	while ( pwszString && *pwszString )
	{
		size_t	nLen = 0 ;
		while ( pwszString[nLen] )
		{
			if ( (pwszString[nLen] == L'\n')
				|| (pwszString[nLen] == L'\r') )
			{
				break ;
			}
			nLen ++ ;
		}
		if ( m_iCurChar >= pLine->m_strLine.GetLength() )
		{
			pLine->m_strLine += SString( pwszString, (ssize_t) nLen ) ;
			m_iCurChar += nLen ;
		}
		else
		{
			SString	strLine = pLine->m_strLine.Left( m_iCurChar ) ;
			strLine += SString( pwszString, (ssize_t) nLen ) ;
			strLine += pLine->m_strLine.Middle( m_iCurChar ) ;
			pLine->m_strLine = strLine ;
			m_iCurChar += nLen ;
		}
		RasterizeLine( pLine ) ;
		//
		pwszString += nLen ;
		if ( (*pwszString == L'\n') || (*pwszString == L'\r') )
		{
			SString	strInput =
				pLine->m_strLine.Middle( pLine->m_nFixedChars ) ;
			strInput += L'\n' ;
			//
			SArray<uint8_t>	bufInput ;
			Charset::Encode
				( bufInput, m_encoding,
					strInput, (ssize_t) strInput.GetLength() ) ;
			//
			ESLAssert( TestLocked() > 0 ) ;
			m_qbufInput.Write
				( bufInput.GetConstArray(), bufInput.GetLength() ) ;
			m_eventInput.SetSignal() ;
			//
			pLine->m_flagReturn = true ;
			pLine->m_nFixedChars = pLine->m_strLine.GetLength() ;
			//
			size_t	iLine ;
			pLine = GetLastLine( iLine ) ;
			//
			m_iCurLine = iLine ;
			m_iCurChar = 0 ;
			pwszString ++ ;
			//
			m_iRefHistory = -1 ;
		}
	}
	ScrollForInputCursor() ;
	if ( GetInputCursorRect( rectCur ) )
	{
		SGLRect	rect = rectCur ;
		PostUpdate( &rect ) ;
	}
	return	true ;
}

// 最終行を取得（新規行が制限数を超える場合には削除）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteConsole::Line * SGLSpriteConsole::GetLastLine( size_t& iLine )
{
	Line *	pLine = m_lines.GetLastAt() ;
	if ( (pLine == nullptr) || pLine->m_flagReturn )
	{
		pLine = new Line ;
		if ( m_lines.GetLength() >= m_nLimitLines )
		{
			size_t	nCount = m_lines.GetLength() + 1 - m_nLimitLines ;
			m_lines.Remove( 0, nCount ) ;
			//
			if ( m_iCurLine > nCount )
			{
				m_iCurLine = 0 ;
				m_iCurChar = 0 ;
			}
			else
			{
				m_iCurLine -= nCount ;
			}
		}
		iLine = m_lines.GetLength() ;
		m_lines.Add( pLine ) ;
	}
	else
	{
		ESLAssert( m_lines.GetLength() > 0 ) ;
		iLine = m_lines.GetLength() - 1 ;
	}
	return	pLine ;
}

// カーソル行を取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteConsole::Line * SGLSpriteConsole::GetCursorLine( void )
{
	if ( m_nInputable == 0 )
	{
		return	nullptr ;
	}
	Line *	pLine = m_lines.GetAt( m_iCurLine ) ;
	if ( (pLine == nullptr) && (m_lines.GetLength() == m_iCurLine) )
	{
		pLine = new Line ;
		m_lines.SetAt( m_iCurLine, pLine ) ;
		m_iCurChar = 0 ;
	}
	return	pLine ;
}

// 指定行の絶対ｙ座標取得
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteConsole::GetLineAbsolutePos
			( SGLSpriteConsole::Line * pLine ) const
{
	int	yLine = 0 ;
	for ( size_t i = 0; i < m_lines.GetLength(); i ++ )
	{
		Line *	p = m_lines.GetAt( i ) ;
		ESLAssert( p != nullptr ) ;
		if ( p != nullptr )
		{
			if ( p == pLine )
			{
				break ;
			}
			yLine += p->m_nHeight ;
		}
	}
	return	yLine ;
}

// 指定行矩形の更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::PostUpdateLine( SGLSpriteConsole::Line * pLine )
{
	int		yLine = GetLineAbsolutePos( pLine ) ;
	SGLSize	sizeLine = pLine->m_imgLine.GetImageSize() ;
	SGLRect	rectLine ;
	rectLine.left = pLine->m_ptOffset.x ;
	rectLine.top = pLine->m_ptOffset.y ;
	rectLine.right = pLine->m_ptOffset.x + sizeLine.w ;
	rectLine.bottom = pLine->m_ptOffset.y + sizeLine.h ;
	//
	PostUpdate( &rectLine ) ;
}

// 指定行のラスタライズ（更新通知）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::RasterizeLine( SGLSpriteConsole::Line * pLine )
{
	SGLSize	sizeConsole = GetImageSize() ;
	//
	SObjectArray<Character>	aChars ;
	SGLPoint	ptNext( 0, 0 ) ;
	//
	pLine->m_nHeight = m_nLineHeight ;
	pLine->m_aCharRects.RemoveAll() ;
	//
	for ( size_t i = 0; i < pLine->m_strLine.GetLength(); i ++ )
	{
		SGLFontMetrics	metrics ;
		Character *	pChar = new Character ;
		if ( !m_font.GetFontImage( &(pChar->m_imgChar),
				metrics, pLine->m_strLine.GetAt(i), m_rgbaFont.ui32 ) )
		{
			pChar->m_ptOffset.x = metrics.rctExterior.x ;
			pChar->m_ptOffset.y = metrics.rctExterior.y ;
			//
			if ( ptNext.x + metrics.nWidth > sizeConsole.w )
			{
				ptNext.x = 0 ;
				ptNext.y += m_nLineHeight ;
				//
				pLine->m_nHeight = ptNext.y + m_nLineHeight ;
			}
			//
			SGLImageRect
				rect( ptNext.x, ptNext.y, metrics.nWidth, m_nLineHeight ) ;
			pLine->m_aCharRects.Add( rect ) ;
			//
			pChar->m_ptOffset.x += ptNext.x ;
			pChar->m_ptOffset.y += ptNext.y ;
			ptNext.x += metrics.nWidth ;
			//
			aChars.Add( pChar ) ;
		}
		else
		{
			SGLImageRect
				rect( ptNext.x, ptNext.y, 0, m_nLineHeight ) ;
			pLine->m_aCharRects.Add( rect ) ;
			//
			delete	pChar ;
		}
	}
	//
	SGLRect	rectExt ;
	for ( size_t i = 0; i < aChars.GetLength(); i ++ )
	{
		Character *	pChar = aChars.GetAt( i ) ;
		ESLAssert( pChar != nullptr ) ;
		//
		SGLRect	rect ;
		rect.left = pChar->m_ptOffset.x ;
		rect.top = pChar->m_ptOffset.y ;
		//
		SGLSize	sizeChar = pChar->m_imgChar.GetImageSize() ;
		rect.SetWidth( sizeChar.w ) ;
		rect.SetHeight( sizeChar.h ) ;
		//
		if ( i == 0 )
		{
			rectExt = rect ;
		}
		else
		{
			rectExt |= rect ;
		}
	}
	//
	SGLSize	sizeBuf = pLine->m_imgLine.GetImageSize() ;
	if ( (sizeBuf.w != rectExt.GetWidth())
		|| (sizeBuf.h != rectExt.GetHeight()) )
	{
		pLine->m_imgLine.CreateImage
			( rectExt.GetWidth(), rectExt.GetHeight(), formatImageARGB, 32 ) ;
	}
	pLine->m_ptOffset.x = rectExt.left ;
	pLine->m_ptOffset.y = rectExt.top ;
	//
	pLine->m_imgLine.FillImage( SGLPalette( 0 ) ) ;
	//
	for ( size_t i = 0; i < aChars.GetLength(); i ++ )
	{
		Character *	pChar = aChars.GetAt( i ) ;
		ESLAssert( pChar != nullptr ) ;
		pLine->m_imgLine.BlendImage
			( &(pChar->m_imgChar),
				pChar->m_ptOffset.x - pLine->m_ptOffset.x,
				pChar->m_ptOffset.y - pLine->m_ptOffset.y ) ;
	}
	pLine->m_flagUpdate = false ;
	//
	int		yLine = GetLineAbsolutePos( pLine ) ;
	SGLRect	rectLine = rectExt ;
	rectLine.top += yLine ;
	rectLine.bottom += yLine ;
	//
	PostUpdate( &rectLine ) ;
}

// 文字表示矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::GetCharacterViewRect
	( SGLImageRect& rectChar, size_t iLine, size_t iChar ) const
{
	LockTrace( __FILE__, __LINE__ ) ;
	bool	fViewRect = false ;
	int		yLine = 0 ;
	for ( size_t i = 0; i < m_lines.GetLength(); i ++ )
	{
		Line *	pLine = m_lines.GetAt( i ) ;
		ESLAssert( pLine != nullptr ) ;
		if ( pLine != nullptr )
		{
			if ( i == iLine )
			{
				SGLImageRect *	pRect =
					pLine->m_aCharRects.GetAt( iChar ) ;
				if ( pRect != nullptr )
				{
					rectChar = *pRect ;
					rectChar.y += yLine - m_yScroll ;
					fViewRect = true ;
					break ;
				}
				pRect = pLine->m_aCharRects.GetAt( iChar - 1 ) ;
				if ( pRect == nullptr )
				{
					pRect = pLine->m_aCharRects.GetLastAt() ;
				}
				if ( pRect != nullptr )
				{
					rectChar.x = pRect->x + pRect->w ;
					rectChar.y = pRect->y + yLine - m_yScroll ;
					rectChar.w = 2 ;
					rectChar.h = pRect->h ;
					fViewRect = true ;
					break ;
				}
				rectChar.x = 0 ;
				rectChar.y = yLine - m_yScroll ;
				rectChar.w = 2 ;
				rectChar.h = m_nLineHeight ;
				fViewRect = true ;
				break ;
			}
			else
			{
				yLine += pLine->m_nHeight ;
			}
		}
	}
	Unlock() ;
	return	fViewRect ;
}

// カーソル表示矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::GetInputCursorRect( SGLImageRect& rectCur ) const
{
	return	GetCharacterViewRect( rectCur, m_iCurLine, m_iCurChar ) ;
}

// スクロール範囲を正規化
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::NormalizeScrollPos( void )
{
	if ( m_yScroll < 0 )
	{
		m_yScroll = 0 ;
	}
	else
	{
		Line *	pLine = m_lines.GetLastAt() ;
		if ( pLine != nullptr )
		{
			int	yLine = GetLineAbsolutePos( pLine ) ;
			if ( m_yScroll > yLine )
			{
				m_yScroll = yLine ;
			}
		}
		else
		{
			m_yScroll = 0 ;
		}
	}
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	SGLImageRect	rectCur ;
	if ( GetInputCursorRect( rectCur ) )
	{
		m_msecCurBlink += msecPast ;
		if ( m_msecCurBlinkInterval > 0 )
		{
			m_msecCurBlink %= m_msecCurBlinkInterval ;
		}
		SGLRect	rect = rectCur ;
		PostUpdate( &rect ) ;
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteConsole::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;
	//
	SGLSize	sizeConsole = GetImageSize() ;
	//
	int	yLine = 0 ;
	for ( size_t i = 0; i < m_lines.GetLength(); i ++ )
	{
		Line *	pLine = m_lines.GetAt( i ) ;
		ESLAssert( pLine != nullptr ) ;
		if ( pLine == nullptr )
		{
			continue ;
		}
		if ( (yLine + pLine->m_nHeight >= m_yScroll)
			&& (yLine + pLine->m_ptOffset.y - m_yScroll <= sizeConsole.h)
			&& !pLine->m_imgLine.GetImageSize().IsEmpty() )
		{
			SGLPaintParam	pp ;
			pp.ptPaint.x = pLine->m_ptOffset.x ;
			pp.ptPaint.y = yLine + pLine->m_ptOffset.y - m_yScroll ;
			render.DrawImage( pp, &(pLine->m_imgLine) ) ;
		}
		yLine += pLine->m_nHeight ;
	}
	//
	SGLImageRect	rectCur ;
	if ( GetInputCursorRect( rectCur ) )
	{
		uint32_t	t = 0x100 ;
		if ( m_msecCurBlinkInterval > 0 )
		{
			t = (uint32_t) (m_msecCurBlink * 0x200 / m_msecCurBlinkInterval) ;
			if ( t >= 0x100 )
			{
				t = 0x200 - t ;
			}
		}
		SGLPalette	rgbaCursor = m_rgbaCursor.imul(t) ;
		render.FillRectangle
			( rectCur.x, rectCur.y, rectCur.w, rectCur.h, rgbaCursor.ui32 ) ;
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagDragScroll )
	{
		double	dy = yPos - m_vBeginDragPos.y ;
		if ( fabs( dy ) > 8 )
		{
			m_flagDragMoved = true ;
		}
		m_yScroll = m_yBeginDragScroll
						- eslRoundR32ToInt( (float32_t) dy ) ;
		NormalizeScrollPos() ;
		PostUpdate( nullptr ) ;
		return	true ;
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteConsole::OnMouseLeave( int64_t nFlags )
{
	if ( m_flagDragScroll )
	{
		ReleaseMouseCapture() ;
		m_flagDragScroll = false ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	if ( SGLSprite::OnMouseWheel( zDelta, xPos, yPos, nFlags ) )
	{
		return	true ;
	}
	if ( m_flagsConsole & flagScrollWheel )
	{
		m_yScroll -= (int32_t) m_nLineHeight * zDelta / 0x100 ;
		NormalizeScrollPos() ;
		PostUpdate( nullptr ) ;
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
	//
	if ( m_flagsConsole & flagScrollDrag )
	{
		m_flagDragScroll = true ;
		m_flagDragMoved = false ;
		m_vBeginDragPos.x = xPos ;
		m_vBeginDragPos.y = yPos ;
		m_yBeginDragScroll = m_yScroll ;
		//
		SetMouseCapture() ;
	}
	return	true ;
}

bool SGLSpriteConsole::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagDragScroll )
	{
		if ( m_nInputable
			&& !m_flagDragMoved
			&& UI::IsPlatformTablet() )
		{
			SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
			if ( pWindow != nullptr )
			{
				pWindow->SetOptionalFlags
					( pWindow->GetOptionalFlags() | Window::flagOpenIME ) ;
			}
		}
		ReleaseMouseCapture() ;
		m_flagDragScroll = false ;
	}
	return	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnKeyDown
	( int64_t nVirtKey, int64_t nFlags )
{
	if ( SGLSprite::OnKeyDown( nVirtKey, nFlags ) )
	{
		return	true ;
	}
	Line *	pLine = GetCursorLine() ;
	if ( pLine == nullptr )
	{
		return	false ;
	}
	SGLImageRect	rectCur ;
	if ( GetInputCursorRect( rectCur ) )
	{
		SGLRect	rect = rectCur ;
		PostUpdate( &rect ) ;
	}
	switch ( nVirtKey )
	{
	case	vkeyLeft:
		if ( m_iCurChar > pLine->m_nFixedChars )
		{
			m_iCurChar -- ;
			if ( GetInputCursorRect( rectCur ) )
			{
				SGLRect	rect = rectCur ;
				PostUpdate( &rect ) ;
			}
		}
		return	true ;
	case	vkeyRight:
		if ( m_iCurChar < pLine->m_strLine.GetLength() )
		{
			m_iCurChar ++ ;
			if ( GetInputCursorRect( rectCur ) )
			{
				SGLRect	rect = rectCur ;
				PostUpdate( &rect ) ;
			}
		}
		return	true ;
	case	vkeyUp:
		if ( m_iRefHistory < (ssize_t) m_lstHistory.GetLength() - 1 )
		{
			if ( ++ m_iRefHistory < 0 )
			{
				m_iRefHistory = 0 ;
			}
			SString *	pstrHis =
					m_lstHistory.GetAt( (size_t) m_iRefHistory ) ;
			if ( pstrHis != nullptr )
			{
				PostUpdateLine( pLine ) ;
				//
				pLine->m_strLine =
					pLine->m_strLine.Left( pLine->m_nFixedChars ) ;
				pLine->m_strLine += *pstrHis ;
				m_iCurChar = pLine->m_strLine.GetLength() ;
				RasterizeLine( pLine ) ;
			}
		}
		return	true ;
	case	vkeyDown:
		if ( m_iRefHistory > 0 )
		{
			m_iRefHistory -- ;
			//
			SString *	pstrHis =
					m_lstHistory.GetAt( (size_t) m_iRefHistory ) ;
			if ( pstrHis != nullptr )
			{
				PostUpdateLine( pLine ) ;
				//
				pLine->m_strLine =
					pLine->m_strLine.Left( pLine->m_nFixedChars ) ;
				pLine->m_strLine += *pstrHis ;
				m_iCurChar = pLine->m_strLine.GetLength() ;
				RasterizeLine( pLine ) ;
			}
		}
		return	true ;
	case	vkeyDelete:
		if ( m_iCurChar < pLine->m_strLine.GetLength() )
		{
			PostUpdateLine( pLine ) ;
			//
			SString	strLine =
				pLine->m_strLine.Left( m_iCurChar )
					+ pLine->m_strLine.Middle( m_iCurChar + 1 ) ;
			pLine->m_strLine = strLine ;
			RasterizeLine( pLine ) ;
		}
		return	true ;
	case	vkeyBack:
		if ( (m_iCurChar > pLine->m_nFixedChars)
			&& (m_iCurChar <= pLine->m_strLine.GetLength()) )
		{
			PostUpdateLine( pLine ) ;
			//
			SString	strLine =
				pLine->m_strLine.Left( m_iCurChar - 1 )
					+ pLine->m_strLine.Middle( m_iCurChar ) ;
			pLine->m_strLine = strLine ;
			m_iCurChar -- ;
			RasterizeLine( pLine ) ;
		}
		return	true ;
	}
	return	false ;
}

bool SGLSpriteConsole::OnKeyUp
	( int64_t nVirtKey, int64_t nFlags )
{
	if ( SGLSprite::OnKeyDown( nVirtKey, nFlags ) )
	{
		return	true ;
	}
	switch ( nVirtKey )
	{
	case	vkeyLeft:
	case	vkeyRight:
	case	vkeyUp:
	case	vkeyDown:
	case	vkeyDelete:
	case	vkeyBack:
		return	true ;
	}
	return	false ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnChar( uint16_t codeChar )
{
	if ( SGLSprite::OnChar( codeChar ) )
	{
		return	true ;
	}
	if ( codeChar < 0x20 )
	{
		if ( (codeChar != '\n')
			&& (codeChar != '\r') )
		{
			return	true ;
		}
	}
	wchar_t	wszChar[2] ;
	wszChar[0] = (wchar_t) codeChar ;
	wszChar[1] = 0 ;
	return	InputString( wszChar ) ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnStartComposition
	( SGLInputStartComposition& iscForm )
{
	if ( SGLSprite::OnStartComposition( iscForm ) )
	{
		return	true ;
	}
	SGLImageRect	rectChar ;
	if ( GetInputCursorRect( rectChar ) )
	{
		SGLSize	sizeConsole = GetImageSize() ;
		//
		iscForm.nFlags = SGLInputStartComposition::flagPosition
						| SGLInputStartComposition::flagRectangle
						| SGLInputStartComposition::flagFont ;
		iscForm.ptStart.x = rectChar.x ;
		iscForm.ptStart.y = rectChar.y ;
		iscForm.rctArea.x = 0 ;
		iscForm.rctArea.y = 0 ;
		iscForm.rctArea.w = sizeConsole.w ;
		iscForm.rctArea.h = sizeConsole.h ;
		iscForm.fsFontStyle = m_fontStyle ;
		//
		S2DDVector	vPos( iscForm.ptStart.x, iscForm.ptStart.y ) ;
		LocalToGlobal( vPos ) ;
		iscForm.ptStart.x = (int32_t) vPos.x ;
		iscForm.ptStart.y = (int32_t) vPos.y ;
		//
		SGLRect	rectArea = iscForm.rctArea ;
		LocalToGlobalRect( rectArea ) ;
		iscForm.rctArea = rectArea ;
		//
		return	true ;
	}
	return	false ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnEndComposition( void )
{
	if ( SGLSprite::OnEndComposition() )
	{
		return	true ;
	}
	return	true ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteConsole::OnCompositionString
	( const SGLInputCompositionString& icsComp )
{
	if ( icsComp.nFlags & SGLInputCompositionString::flagResult )
	{
		SString	strComp = icsComp.pszComposition ;
		InputString( strComp ) ;
	}
	return	true ;
}

