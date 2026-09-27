
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 文字列入力スプライト
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	ETextEditSprite::m_pwszDefCursor = L"IDC_IBEAM" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ETextEditSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ETextEditSprite::ETextEditSprite( void )
{
	m_dwFlags |= ffTabStop ;
	m_etType = etSingleLine ;
	m_dwEditFlags = 0 ;
	m_nLimitLength = 0 ;
	m_pEditServer = NULL ;
	m_hDrawLine = NULL ;
	m_hFont = NULL ;
	m_nTabWidth = 0 ;
	m_nMaxLineWidth = 0 ;
	m_nWordWrapWidth = -1 ;
	m_nPageSize = 8 ;
	m_nLeftSpace = 0 ;
	m_xScroll = 0 ;
	m_yScroll = 0 ;
	m_iSelFirst = 0 ;
	m_iSelEnd = 0 ;
	m_hIMM32 = NULL ;
	m_apiGetCompositionStringW = NULL ;
	m_nUndoLimit = 0x100 ;
	m_hMenuPopup = NULL ;
	m_fMouseSel = 0 ;
	m_fFocus = false ;
	m_fViewCaret = true ;
	m_fIMEComposition = false ;
	m_cLastChar = '\0' ;
	m_dwLastTime = 0 ;
	m_wstrProhibit = L",.!?;:)]，．、。！？；：」】）〕｝〉》』"
						L"ぁぃぅぇぉっゃゅょァィゥェォッャュョ" ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ETextEditSprite::~ETextEditSprite( void )
{
	if ( m_hDrawLine != NULL )
	{
		m_hDrawLine->Release( ) ;
	}
	if ( m_hFont != NULL )
	{
		::DeleteObject( m_hFont ) ;
	}
	if ( m_hMenuPopup != NULL )
	{
		::DestroyMenu( m_hMenuPopup ) ;
	}
	if ( m_hIMM32 != NULL )
	{
		::FreeLibrary( m_hIMM32 ) ;
	}
	DetachAllSprite( ) ;
}

// 文字列取得・設定
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ETextEditSprite::GetSpriteText( void )
{
	m_wstrTextBuf = GetEditText( ) ;
	return	m_wstrTextBuf ;
}

void ETextEditSprite::SetSpriteText( const wchar_t * pwszText )
{
	m_wstrTextBuf.FreeString( ) ;
	SetEditText( pwszText ) ;
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	EString	strFont = pwszFont ;
	int		nFontLen = strFont.GetLength() + 1 ;
	if ( nFontLen > LF_FACESIZE )
	{
		nFontLen = LF_FACESIZE ;
	}
	::eslMoveMemory( m_lfFont.lfFaceName, strFont.CharPtr(), nFontLen ) ;
	//
	if ( m_hFont != NULL )
		::DeleteObject( m_hFont ) ;
//	m_hFont = ::CreateFontIndirect( &m_lfIMC ) ;
//	m_rfiText.SetFont( m_hFont ) ;
	SakuraGL::SGLFont *		pFont = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	styleTemp ;
	SSystem::SString		strFontTemp ;
	styleTemp.FromLogFont( m_lfFont, strFontTemp ) ;
	pFont->SetStyle( styleTemp ) ;
	m_rfiText.SetSGLFont( pFont ) ;
	//
	EWideString	wstrText = GetEditText() ;
	SetEditText( L"" ) ;
	m_tsaImgBuf.RemoveAll( ) ;
	//
	SetEditText( wstrText ) ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::IsHitSprite( int xPos, int yPos )
{
	if ( !IsEnabled() )
	{
		return	false ;
	}
	if ( ESpriteInterface::IsHitSprite( xPos, yPos ) )
	{
		return	true ;
	}
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	EGL_RECT	rect = m_isTextPanel.GetRectangle( ) ;
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		return	true ;
	}
	return	false ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	if ( m_fMouseSel )
	do
	{
		EGL_POINT	ptGlobal, ptLocal ;
		DWORD		dwCurrentTime = ::GetCurrentTime() ;
		int			xScroll = m_xScroll ;
		int			yScroll = m_yScroll ;
		ptGlobal.x = xPos ;
		ptGlobal.y = yPos ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		//
		if ( (int) (dwCurrentTime - m_dwScrollTimer) < 100 )
		{
			break ;
		}
		int	iChar = GetCharIndexFromPos( ptLocal.x, ptLocal.y ) ;
		m_dwScrollTimer = dwCurrentTime ;
		//
		if ( m_fMouseSel == 1 )
		{
			SetSel( m_iSelFirst, iChar ) ;
			//
			UpdateWindow( ) ;
		}
		else if ( m_fMouseSel == 2 )
		{
			int	iLine = GetLineFromIndex( iChar ) ;
			const ELineInf *	pLInf = GetLineAt( iLine ) ;
			if ( pLInf != NULL )
			{
				int	iFirst, iEnd ;
				GetWordBoundary
					( pLInf->m_wstrLine,
						iChar - pLInf->m_nIndex, iFirst, iEnd ) ;
				iFirst += pLInf->m_nIndex ;
				iEnd += pLInf->m_nIndex ;
				//
				if ( iFirst < m_iSelFirst )
				{
					iEnd = iFirst ;
				}
				SetSel( m_iSelFirst, iEnd ) ;
				//
				UpdateWindow( ) ;
			}
		}
		if ( (xScroll == m_xScroll) && (yScroll == m_yScroll) )
		{
			m_dwScrollTimer = 0 ;
		}
	}
	while ( false ) ;
}

void ETextEditSprite::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
}

bool ETextEditSprite::OnSetCursor( int xPos, int yPos )
{
	ESLError	errResult ;
	//
	if ( m_wstrCursor.IsEmpty() )
		errResult = SetMouseCursor( m_pwszDefCursor ) ;
	else
		errResult = SetMouseCursor( m_wstrCursor ) ;
	//
	return	(errResult == eslErrSuccess) ;
}

bool ETextEditSprite::OnMouseWheel
	( UINT nFlags, short int zDelta, int xPos, int yPos )
{
	if ( m_etType == etMultiLine )
	{
		int	yScroll = m_yScroll - zDelta / WHEEL_DELTA ;
		SetScrollPos( m_xScroll, yScroll ) ;
		return	true ;
	}
	return	false ;
}

bool ETextEditSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	EGL_RECT	rect = m_isTextPanel.GetRectangle( ) ;
	//
	SetFocus( NULL ) ;
	//
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		int	iSel = GetCharIndexFromPos( ptLocal.x, ptLocal.y ) ;
		SetSel( iSel, iSel ) ;
		m_fMouseSel = 1 ;
		m_dwScrollTimer = 0 ;
		SetCapture( ) ;
	}
	//
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		pParent->SetFocus( this ) ;
	}
	//
	return	true ;
}

bool ETextEditSprite::OnLButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( m_fMouseSel )
	{
		m_fMouseSel = 0 ;
		ReleaseCapture( ) ;
	}
	return	true ;
}

bool ETextEditSprite::OnLButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	EGL_RECT	rect = m_isTextPanel.GetRectangle( ) ;
	//
	SetFocus( NULL ) ;
	//
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		int	iChar = GetCharIndexFromPos( ptLocal.x, ptLocal.y ) ;
		int	iLine = GetLineFromIndex( iChar ) ;
		const ELineInf *	pLInf = GetLineAt( iLine ) ;
		if ( pLInf != NULL )
		{
			int	iFirst, iEnd ;
			GetWordBoundary
				( pLInf->m_wstrLine, iChar - pLInf->m_nIndex, iFirst, iEnd ) ;
			SetSel( iFirst + pLInf->m_nIndex, iEnd + pLInf->m_nIndex ) ;
			//
			m_fMouseSel = 2 ;
			m_dwScrollTimer = 0 ;
			SetCapture( ) ;
		}
	}
	//
	return	true ;
}

bool ETextEditSprite::OnRButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( m_hMenuPopup != NULL )
	{
		::DestroyMenu( m_hMenuPopup ) ;
		m_hMenuPopup = NULL ;
	}
	//
	SetFocus( NULL ) ;
	//
	return	true ;
}

bool ETextEditSprite::OnRButtonUp( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	EGL_RECT	rect = m_isTextPanel.GetRectangle( ) ;
	//
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		//
		// ポップアップメニュー
		//
		if ( m_hMenuPopup != NULL )
		{
			::DestroyMenu( m_hMenuPopup ) ;
		}
		m_hMenuPopup = ::CreatePopupMenu( ) ;
		//
		UINT	nSelFlag = MF_BYCOMMAND | MF_STRING ;
		if ( !CanCopyText() )
			nSelFlag |= MF_GRAYED ;
		//
		UINT	nCutFlag = MF_BYCOMMAND | MF_STRING ;
		if ( !CanCutText() )
			nCutFlag |= MF_GRAYED ;
		//
		UINT	nPasteFlag = MF_BYCOMMAND | MF_STRING ;
		if ( !CanPasteText() )
			nPasteFlag |= MF_GRAYED ;
		//
		::InsertMenu
			( m_hMenuPopup, -1, nCutFlag,
				CMDID_EDIT_CUT, "切り取り\tCtrl+X" ) ;
		::InsertMenu
			( m_hMenuPopup, -1, nSelFlag,
				CMDID_EDIT_COPY, "コピー\tCtrl+C" ) ;
		::InsertMenu
			( m_hMenuPopup, -1, nPasteFlag,
				CMDID_EDIT_PASTE, "貼り付け\tCtrl+P" ) ;
		::InsertMenu
			( m_hMenuPopup, -1, nCutFlag,
				CMDID_EDIT_CLEAR, "削除\tDel" ) ;
		//
		// メニュー表示
		//
		EWindowSpriteInterface *	pWndItf = GetWindowInterface( ) ;
		if ( pWndItf != NULL )
		{
			EWindow *	pWnd = pWndItf->GetWindow( ) ;
			if ( pWnd != NULL )
			{
				POINT	ptCursor ;
				::GetCursorPos( &ptCursor ) ;
				UINT	nFlags =
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON ;
				::TrackPopupMenu
					( m_hMenuPopup, nFlags,
						ptCursor.x, ptCursor.y, 0, *pWnd, NULL ) ;
			}
		}
	}
	//
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		pParent->SetFocus( this ) ;
	}
	//
	return	true ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::OnTimer( UINT nEventID )
{
	if ( m_fFocus )
	{
		DWORD	dwCurrentTime = ::GetTickCount( ) ;
		DWORD	dwDeltaTime = dwCurrentTime - m_dwLastTime ;
		if ( dwDeltaTime < m_nCaretInterval * 2 )
		{
			unsigned int	nTrans ;
			unsigned int	nInterval = m_nCaretInterval ;
			if ( dwDeltaTime <= nInterval )
			{
				nTrans = dwDeltaTime * 0x200 / nInterval ;
				if ( nTrans > 0x100 )
					nTrans = 0x100 ;
			}
			else
			{
				nTrans = (dwDeltaTime - nInterval) * 0x200 / nInterval ;
				if ( nTrans > 0x100 )
					nTrans = 0x100 ;
				nTrans = 0x100 - nTrans ;
			}
			m_rsCaret.SetTransparency( nTrans ) ;
		}
		else
		{
			if ( m_nCaretInterval != 0 )
			{
				dwDeltaTime =
					(dwDeltaTime - m_nCaretInterval * 2)
									% (m_nCaretInterval * 2) ;
			}
			else
			{
				dwDeltaTime = 0 ;
			}
			m_dwLastTime = dwCurrentTime - dwDeltaTime ;
		}
	}
	if ( m_fMouseSel )
	{
		DWORD	dwCurrentTime = ::GetCurrentTime() ;
		if ( (int) (dwCurrentTime - m_dwScrollTimer) >= 100 )
		{
			EWindowSpriteInterface *
				pInterface = GetWindowInterface() ;
			if ( pInterface != NULL )
			{
				pInterface->CallMouseMove() ;
			}
		}
	}
	return	true ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	EUndoInf *	pUndo ;
	switch ( uMsg )
	{
	case	WM_COMMAND:
		//
		// コマンド処理
		//
		if ( m_dwEditFlags & efReadOnly )
		{
			if ( LOWORD(wParam) != CMDID_EDIT_COPY )
			{
				break ;
			}
		}
		switch ( LOWORD(wParam) )
		{
		case	CMDID_EDIT_CLEAR:
			DoClear( ) ;
			return	true ;
		case	CMDID_EDIT_COPY:
			DoCopy( ) ;
			return	true ;
		case	CMDID_EDIT_CUT:
			DoCut( ) ;
			return	true ;
		case	CMDID_EDIT_PASTE:
			DoPaste( ) ;
			return	true ;
		case	CMDID_EDIT_SELECT_ALL:
			SetSel( 0, -1 ) ;
			return	true ;
		case	CMDID_EDIT_UNDO:
			Undo( ) ;
			return	true ;
		case	CMDID_EDIT_REDO:
			Redo( ) ;
			return	true ;
		}
		break ;

	case	WM_KEYDOWN:
		//
		// ショートカットキー判定
		//
		if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
		{
			if ( m_dwEditFlags & efReadOnly )
			{
				if ( (wParam != 'C') && (wParam != 'A') )
				{
					break ;
				}
			}
			switch ( wParam )
			{
			case	'C':
				DoCopy( ) ;
				return	true ;
			case	'X':
				DoCut( ) ;
				return	true ;
			case	'V':
				DoPaste( ) ;
				return	true ;
			case	'A':
				SetSel( 0, -1 ) ;
				return	true ;
			case	'Z':
				Undo( ) ;
				return	true ;
			case	'Y':
				Redo( ) ;
				return	true ;
			}
		}
		//
		// キーボード操作
		//
		switch ( wParam )
		{
		case	VK_DELETE:
			// 1 文字削除
			if ( !(m_dwEditFlags & efReadOnly) )
			{
				if ( m_iSelFirst == m_iSelEnd )
				{
					SetSel( m_iSelFirst, m_iSelFirst + 1 ) ;
				}
				DoClear( ) ;
			}
			return	true ;

		case	VK_BACK:
			// バックスペース
			if ( !(m_dwEditFlags & efReadOnly) )
			{
				pUndo = new EUndoInf ;
				if ( m_iSelFirst == m_iSelEnd )
				{
					if ( m_iSelEnd > 0 )
					{
						SetSel( m_iSelFirst, m_iSelEnd - 1 ) ;
					}
				}
				DoClear( ) ;
			}
			return	true ;

		case	VK_LEFT:
			// 左へ
			if ( m_iSelEnd > 0 )
			{
				int	iSelEnd = m_iSelEnd ;
				if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
				{
					int		iLine = GetLineFromIndex( iSelEnd ) ;
					const ELineInf *	pLInf = GetLineAt( iLine ) ;
					if ( (pLInf != NULL) && (pLInf->m_nIndex < iSelEnd) )
					{
						int	nType, iFirst, iEnd ;
						iSelEnd -= pLInf->m_nIndex ;
						while ( iSelEnd > 0 )
						{
							nType = GetWordBoundary
								( pLInf->m_wstrLine, iSelEnd - 1, iFirst, iEnd ) ;
							iSelEnd = iFirst ;
							if ( nType != 0 )
							{
								break ;
							}
						}
						iSelEnd += pLInf->m_nIndex ;
					}
					else if ( iSelEnd > 0 )
					{
						iSelEnd -- ;
					}
				}
				else if ( iSelEnd > 0 )
				{
					iSelEnd -- ;
				}
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
				{
					SetSel( m_iSelFirst, iSelEnd ) ;
				}
				else
				{
					SetSel( iSelEnd, iSelEnd ) ;
				}
			}
			return	true ;

		case	VK_RIGHT:
			// 右へ
			if ( m_iSelEnd < GetLength() )
			{
				int	iSelEnd = m_iSelEnd ;
				if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
				{
					int		iLine = GetLineFromIndex( iSelEnd ) ;
					const ELineInf *	pLInf = GetLineAt( iLine ) ;
					if ( pLInf != NULL )
					{
						int	nType, iFirst, iEnd ;
						iSelEnd -= pLInf->m_nIndex ;
						nType = GetWordBoundary
							( pLInf->m_wstrLine, iSelEnd, iFirst, iEnd ) ;
						iSelEnd = iEnd ;
						nType = GetWordBoundary
							( pLInf->m_wstrLine, iSelEnd, iFirst, iEnd ) ;
						if ( nType == 0 )
						{
							iSelEnd = iEnd ;
						}
						iSelEnd += pLInf->m_nIndex ;
					}
					else if ( iSelEnd < GetLength() )
					{
						iSelEnd ++ ;
					}
				}
				else if ( iSelEnd < GetLength() )
				{
					iSelEnd ++ ;
				}
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
				{
					SetSel( m_iSelFirst, iSelEnd ) ;
				}
				else
				{
					SetSel( iSelEnd, iSelEnd ) ;
				}
			}
			return	true ;

		case	VK_UP:
		case	VK_PRIOR:
			// 上へ
			if ( m_etType == etMultiLine )
			{
				int	iSelPos = __min( m_iSelFirst, m_iSelEnd ) ;
				int	nLine = GetLineFromIndex( iSelPos ) ;
				int	nUpLines = 1 ;
				bool	fScroll = ((GetKeyState(VK_CONTROL) & 0x80) != 0) ;
				if ( wParam == VK_PRIOR )
				{
					nUpLines = m_nPageSize ;
					fScroll = true ;
				}
				if ( fScroll )
				{
					if ( m_yScroll >= nUpLines )
					{
						m_yScroll -= nUpLines ;
						DrawViewText( false ) ;
						OnScrollPos() ;
					}
				}
				if ( nLine >= nUpLines )
				{
					int	nPosX = GetCharPosFromIndex( iSelPos ) - m_xScroll ;
					int	iNextSel =
						GetIndexFromLinePos( nLine - nUpLines, nPosX ) ;
					if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					{
						SetSel( m_iSelFirst, iNextSel ) ;
					}
					else
					{
						SetSel( iNextSel, iNextSel ) ;
					}
					return	true ;
				}
				else
				{
					SetSel( 0, 0 ) ;
					return	true ;
				}
			}
		case	VK_HOME:
			if ( m_etType == etSingleLine )
			{
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					SetSel( m_iSelFirst, 0 ) ;
				else
					SetSel( 0, 0 ) ;
			}
			else
			{
				int	nLine = GetLineFromIndex( m_iSelEnd ) ;
				int	nLineIndex = GetLineIndex( nLine ) ;
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					SetSel( m_iSelFirst, nLineIndex ) ;
				else
					SetSel( nLineIndex, nLineIndex ) ;
			}
			return	true ;

		case	VK_DOWN:
		case	VK_NEXT:
			// 下へ
			if ( m_etType == etMultiLine )
			{
				int	iSelPos = __max( m_iSelFirst, m_iSelEnd ) ;
				int	nLine = GetLineFromIndex( iSelPos ) ;
				int	nDownLines = 1 ;
				bool	fScroll = ((GetKeyState(VK_CONTROL) & 0x80) != 0) ;
				if ( wParam == VK_NEXT )
				{
					nDownLines = m_nPageSize ;
					fScroll = true ;
				}
				if ( fScroll )
				{
					if ( m_yScroll < GetLineCount() - nDownLines )
					{
						m_yScroll += nDownLines ;
						DrawViewText( false ) ;
						OnScrollPos() ;
					}
				}
				if ( nLine < GetLineCount() - nDownLines )
				{
					int	nPosX = GetCharPosFromIndex( iSelPos ) - m_xScroll ;
					int	iNextSel =
						GetIndexFromLinePos( nLine + nDownLines, nPosX ) ;
					if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					{
						SetSel( m_iSelFirst, iNextSel ) ;
					}
					else
					{
						SetSel( iNextSel, iNextSel ) ;
					}
					return	true ;
				}
			}
		case	VK_END:
			if ( m_etType == etSingleLine )
			{
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					SetSel( m_iSelFirst, GetLength() ) ;
				else
					SetSel( GetLength(), GetLength() ) ;
			}
			else
			{
				int	nLine = GetLineFromIndex( m_iSelEnd ) ;
				int	nLineIndex = GetLineIndex( nLine + 1 ) ;
				if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
					SetSel( m_iSelFirst, nLineIndex ) ;
				else
					SetSel( nLineIndex, nLineIndex ) ;
			}
			return	true ;

		case	VK_TAB:
			if ( m_dwEditFlags & efInputTab )
			{
				//
				// タブの入力
				//
				int	iFirstLine = GetLineFromIndex( m_iSelFirst ) ;
				int	iEndLine = GetLineFromIndex( m_iSelEnd ) ;
				if ( iFirstLine == iEndLine )
				{
					char	szText[2] ;
					szText[0] = (char) wParam ;
					szText[1] = '\0' ;
					pUndo = new EUndoInf ;
					ReplaceSelText( EWideString( szText ), pUndo ) ;
					RecordUndo( pUndo ) ;
				}
				else
				{
					//
					// 行の字下げ
					//
					bool	fIndent = !(::GetKeyState( VK_SHIFT ) & 0x80) ;
					int		iSelFirst = m_iSelFirst ;
					int		iSelEnd = m_iSelEnd ;
					if ( iFirstLine > iEndLine )
					{
						int	iTemp = iFirstLine ;
						iFirstLine = iEndLine ;
						iEndLine = iTemp ;
						iTemp = iSelFirst ;
						iSelFirst = iSelEnd ;
						iSelEnd = iTemp ;
					}
					if ( iSelEnd == GetLineIndex( iEndLine ) )
					{
						iEndLine -- ;
						ESLAssert( iEndLine >= iFirstLine ) ;
					}
					//
					// 置き換え文字列生成
					//
					EWideString	wstrReplace ;
					bool		fStartLine = true ;
					iSelFirst = GetLineIndex( iFirstLine ) ;
					iSelEnd = GetLineIndex( iEndLine + 1 ) ;
					SetSel( iSelFirst, iSelEnd ) ;
					//
					for ( int iLine = iFirstLine; iLine <= iEndLine; iLine ++ )
					{
						const ELineInf *	pLInf = GetLineAt( iLine ) ;
						if ( pLInf == NULL )
						{
							continue ;
						}
						if ( fStartLine )
						{
							if ( fIndent )
							{
								wstrReplace += L'\t' ;
								wstrReplace += pLInf->m_wstrLine ;
								iSelEnd ++ ;
							}
							else
							{
								wchar_t	wch = pLInf->m_wstrLine.GetAt(0) ;
								if ( (wch == L' ') || (wch == L'\t') )
								{
									wstrReplace += pLInf->m_wstrLine.Middle(1) ;
									iSelEnd -- ;
								}
								else
								{
									wstrReplace += pLInf->m_wstrLine ;
								}
							}
						}
						else
						{
							wstrReplace += pLInf->m_wstrLine ;
						}
						fStartLine = (pLInf->m_wstrLine.Find( L'\n' ) >= 0) ;
					}
					//
					// 文字列置き換え
					//
					pUndo = new EUndoInf ;
					ReplaceSelText( wstrReplace, pUndo ) ;
					RecordUndo( pUndo ) ;
					SetSel( iSelFirst, iSelEnd ) ;
				}
				return	true ;
			}
		}
		break ;

	case	WM_CHAR:
		//
		// 文字入力
		//
		if ( m_dwEditFlags & efReadOnly )
		{
			break ;
		}
		if ( m_cLastChar )
		{
			if ( !(m_dwEditFlags & efDenyMBChar) )
			{
				char	szText[3] ;
				pUndo = new EUndoInf ;
				szText[0] = m_cLastChar ;
				szText[1] = (char) wParam ;
				szText[2] = '\0' ;
				m_cLastChar = '\0' ;
				ReplaceSelText( EWideString( szText ), pUndo ) ;
				RecordUndo( pUndo ) ;
			}
		}
		else if ( (BYTE) wParam >= 0x20 )
		{
			if ( ::IsDBCSLeadByte( (BYTE) wParam ) )
			{
				m_cLastChar = (char) wParam ;
			}
			else
			{
				char	c = (char) wParam ;
				if ( (m_dwEditFlags & efDeny8bitChar)
					|| ((m_dwEditFlags & efDenyAlphabet)
						&& (((c >= 'A') && (c <= 'Z'))
							|| ((c >= 'a') && (c <= 'z'))))
					|| ((m_dwEditFlags & efDenyNumber)
							&& (c >= '0') && (c <= '9')) )
				{
				}
				else
				{
					char	szText[2] ;
					szText[0] = c ;
					szText[1] = '\0' ;
					pUndo = new EUndoInf ;
					ReplaceSelText( EWideString( szText ), pUndo ) ;
					RecordUndo( pUndo ) ;
				}
			}
		}
		else if ( (wParam == '\r') || (wParam == '\n') )
		{
			//
			// 改行の入力
			//
			pUndo = new EUndoInf ;
			ReplaceSelText( L"\n", pUndo ) ;
			RecordUndo( pUndo ) ;
			//
			if ( m_dwEditFlags & efAutoIndent )
			{
				//
				// 自動字下げ
				//
				int			iLine = GetLineFromIndex( m_iSelEnd ) ;
				const ELineInf *	pLInf = GetLineAt( iLine - 1 ) ;
				if ( pLInf != NULL )
				{
					int	nIndent = 0 ;
					while ( nIndent < (int) pLInf->m_wstrLine.GetLength() )
					{
						wchar_t	wch = pLInf->m_wstrLine.GetAt( nIndent ) ;
						if ( (wch > 0x20) || (wch == L'\n') || (wch == L'\r') )
						{
							break ;
						}
						nIndent ++ ;
					}
					if ( nIndent != 0 )
					{
						pUndo = new EUndoInf ;
						ReplaceSelText
							( pLInf->m_wstrLine.Left( nIndent ), pUndo ) ;
						RecordUndo( pUndo ) ;
					}
				}
			}
		}
		return	true ;

	case	WM_IME_CHAR:
		//
		// 文字入力
		//
		if ( m_dwEditFlags & efReadOnly )
		{
			break ;
		}
		if ( wParam & 0xFF00 )
		{
			if ( !(m_dwEditFlags & efDenyMBChar) )
			{
				char	szText[3] ;
				szText[0] = (char) (wParam >> 8) ;
				szText[1] = (char) (wParam & 0xFF) ;
				szText[2] = '\0' ;
				m_cLastChar = '\0' ;
				pUndo = new EUndoInf ;
				ReplaceSelText( EWideString( szText ), pUndo ) ;
				RecordUndo( pUndo ) ;
			}
		}
		else
		{
			char	c = (char) (wParam & 0xFF) ;
			if ( (m_dwEditFlags & efDeny8bitChar)
				|| ((m_dwEditFlags & efDenyAlphabet)
					&& (((c >= 'A') && (c <= 'Z'))
						|| ((c >= 'a') && (c <= 'z'))))
				|| ((m_dwEditFlags & efDenyNumber)
						&& (c >= '0') && (c <= '9')) )
			{
			}
			else
			{
				char	szText[2] ;
				szText[0] = c ;
				szText[1] = '\0' ;
				if ( (BYTE) szText[0] >= 0x20 )
				{
					pUndo = new EUndoInf ;
					ReplaceSelText( EWideString( szText ), pUndo ) ;
					RecordUndo( pUndo ) ;
				}
			}
		}
		return	true ;

	case	WM_IME_STARTCOMPOSITION:
		if ( m_dwEditFlags & efReadOnly )
		{
			break ;
		}
		else
		{
			HIMC	hIMC ;
			COMPOSITIONFORM	cf ;
			EWindowSpriteInterface *	pWndItf = GetWindowInterface( ) ;
			EGL_POINT	ptCursor = m_isTextPanel.GetPosition( ) ;
			ptCursor.x += GetCharPosFromIndex( m_iSelEnd ) - m_xScroll ;
			ptCursor.y +=
				(GetLineFromIndex(m_iSelEnd) - m_yScroll + 1)
									* m_rfiText.GetLineHeight() ;
			if ( m_lfIMC.lfHeight > 0 )
			{
				ptCursor.y -= m_lfIMC.lfHeight
					+ (m_rfiText.GetLineHeight() - m_lfIMC.lfHeight) / 2 ;
			}
			else
			{
				ptCursor.y -= m_rfiText.GetLineHeight() ;
			}
			LocalToWindowClient( ptCursor ) ;
			if ( (pWndItf != NULL) && (pWndItf->GetWindow() != NULL) )
			{
				LOGFONT	lfIMC = m_lfIMC ;
				if ( pWndItf->IsImageStretching() )
				{
					EGL_SIZE	sz = { lfIMC.lfWidth, lfIMC.lfHeight } ;
					pWndItf->ClientToWindowSize( sz ) ;
					lfIMC.lfWidth = sz.w ;
					lfIMC.lfHeight = sz.h ;
				}
				HWND	hWnd = *(pWndItf->GetWindow()) ;
				hIMC = ::ImmGetContext( hWnd ) ;
				cf.dwStyle = CFS_POINT ;
				cf.ptCurrentPos.x = ptCursor.x ;
				cf.ptCurrentPos.y = ptCursor.y ;
				::ImmSetCompositionWindow( hIMC, &cf ) ;
				::ImmSetCompositionFont( hIMC, &lfIMC ) ;
				::ImmReleaseContext( hWnd, hIMC ) ;
				//
				m_fIMEComposition = true ;
				m_rsCaret.SetVisible( false ) ;
			}
		}
		break ;

	case	WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR )
		{
			if ( m_apiGetCompositionStringW != NULL )
			{
				EWideString	wstrIME ;
				HIMC	hIMC = ::ImmGetContext( hWnd ) ;
				LONG	nLen =
					m_apiGetCompositionStringW( hIMC, GCS_RESULTSTR, NULL, 0 ) ;
				nLen /= sizeof(wchar_t) ;
				m_apiGetCompositionStringW
					( hIMC, GCS_RESULTSTR,
						wstrIME.GetBuffer( nLen ), nLen * sizeof(wchar_t) + 1 ) ;
				wstrIME.ReleaseBuffer( nLen ) ;
				::ImmReleaseContext( hWnd, hIMC ) ;
				//
				pUndo = new EUndoInf ;
				ReplaceSelText( wstrIME, pUndo ) ;
				RecordUndo( pUndo ) ;
				return	true ;
			}
		}
		break ;

	case	WM_IME_ENDCOMPOSITION:
		m_fIMEComposition = false ;
		m_rsCaret.SetVisible( m_fViewCaret && m_fFocus ) ;
		break ;
	}
	//
	return	false ;
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int ETextEditSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	EDescription *	pdscOption = dscParam.GetContentTagAs( 0, L"option" ) ;
	if ( pdscOption != NULL )
	{
		m_nLimitLength =
			pdscOption->GetAttrInteger( L"limit", m_nLimitLength ) ;
		//
		EStreamWideString	swsDeny
			= pdscOption->GetAttrString( L"deny", NULL ) ;
		while ( !swsDeny.DisregardSpace() )
		{
			EWideString	wstrToken = swsDeny.GetAToken() ;
			if ( wstrToken == L"return" )
				m_dwEditFlags &= ~efInputReturn ;
			else if ( wstrToken == L"tab" )
				m_dwEditFlags &= ~efInputTab ;
			else if ( wstrToken == L"alphabet" )
				m_dwEditFlags |= efDenyAlphabet ;
			else if ( wstrToken == L"number" )
				m_dwEditFlags |= efDenyNumber ;
			else if ( wstrToken == L"8bit_char" )
				m_dwEditFlags |= efDeny8bitChar ;
			else if ( wstrToken == L"mbchar" )
				m_dwEditFlags |= efDenyMBChar ;
		}
		EStreamWideString	swsAccept
			= pdscOption->GetAttrString( L"accept", NULL ) ;
		while ( !swsAccept.DisregardSpace() )
		{
			EWideString	wstrToken = swsAccept.GetAToken() ;
			if ( wstrToken == L"return" )
				m_dwEditFlags |= efInputReturn ;
			else if ( wstrToken == L"tab" )
				m_dwEditFlags |= efInputTab ;
		}
		if ( pdscOption->GetAttrInteger
			( L"line_tab_indent", m_dwEditFlags & efLineTabIndent ) )
		{
			m_dwEditFlags |= efLineTabIndent ;
		}
		else
		{
			m_dwEditFlags &= ~efLineTabIndent ;
		}
		//
		if ( pdscOption->GetAttrInteger
			( L"auto_indent", m_dwEditFlags & efAutoIndent ) )
		{
			m_dwEditFlags |= efAutoIndent ;
		}
		else
		{
			m_dwEditFlags &= ~efAutoIndent ;
		}
		//
		SetLeftSpaceWidth
			( pdscOption->GetAttrInteger( L"left_space", 0 ) ) ;
		SetWordWrapWidth
			( pdscOption->GetAttrInteger( L"wordwrap", -1 ) ) ;
		SetTabWidth
			( pdscOption->GetAttrInteger( L"tab", m_nTabWidth ) ) ;
	}
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// ウィンドウの描画を更新する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::UpdateWindow( void )
{
	EWindowSpriteInterface *	pItf = GetWindowInterface( ) ;
	if ( pItf != NULL )
	{
		EWindow *	pWnd = pItf->GetWindow( ) ;
		if ( pWnd != NULL )
		{
			if ( ::GetWindowThreadProcessId( *pWnd, NULL )
									== ::GetCurrentThreadId() )
			{
				pWnd->UpdateWindow( ) ;
			}
		}
	}
}

// UNDO 記録
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::RecordUndo( ETextEditSprite::EUndoInf * pUndo )
{
	//
	// エディットクライアントの処理
	//
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->RecordUndo( pUndo ) ;
		return ;
	}
	//
	// 無意味な UNDO か判定する
	//
	if ( pUndo == NULL )
	{
		return ;
	}
	if ( pUndo->m_wstrUndo.IsEmpty()
		&& (pUndo->m_iFirst == pUndo->m_iEnd) )
	{
		delete	pUndo ;
		return ;
	}
	//
	// 現在の REDO を削除する
	//
	m_lstRedo.RemoveAll( ) ;
	//
	// 最後の UNDO と結合できるか判定する
	//
	EUndoInf *	pLastUndo = m_lstUndo.GetLastAt() ;
	if ( pLastUndo != NULL )
	{
		if ( (pLastUndo->m_iEnd == pUndo->m_iFirst)
			&& (pUndo->m_iFirst < pUndo->m_iEnd)
			&& (pLastUndo->m_iFirst < pLastUndo->m_iEnd)
			&& pUndo->m_wstrUndo.IsEmpty()
			&& pLastUndo->m_wstrUndo.IsEmpty() )
		{
			pLastUndo->m_iEnd = pUndo->m_iEnd ;
			delete	pUndo ;
			return ;
		}
	}
	//
	// UNDO 追加
	//
	if ( m_lstUndo.GetSize() > m_nUndoLimit )
	{
		m_lstUndo.RemoveBetween
			( 0, m_lstUndo.GetSize() - m_nUndoLimit ) ;
	}
	m_lstUndo.Add( pUndo ) ;
}

// 単語の境界を検出する
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetWordBoundary
	( const EWideString & wstrLine,
		int nIndex, int & nFirst, int & nEnd ) const
{
	//
	// 指標の範囲チェック
	//
	int	nLength = wstrLine.GetLength() ;
	if ( (nIndex < 0) || (nIndex >= nLength) )
	{
		nFirst = nIndex ;
		nEnd = nIndex ;
		return	0 ;
	}
	//
	// 基準となる文字種別取得
	//
	int	nTypeClass = GetCharacterTypeClass( wstrLine[nIndex] ) ;
	if ( nTypeClass == 1 )
	{
		nFirst = nIndex ;
		nEnd = nIndex + 1 ;
		return	nTypeClass ;
	}
	//
	// 前方チェック
	//
	int	i, nType ;
	for ( i = nIndex - 1; i >= 0; i -- )
	{
		nType = GetCharacterTypeClass( wstrLine[i] ) ;
		if ( nType != nTypeClass )
			break ;
	}
	nFirst = i + 1 ;
	//
	// 後方チェック
	//
	for ( i = nIndex + 1; i < nLength; i ++ )
	{
		nType = GetCharacterTypeClass( wstrLine[i] ) ;
		if ( nType != nTypeClass )
			break ;
	}
	nEnd = i ;
	return	nTypeClass ;
}

// 文字の種別を取得する
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetCharacterTypeClass( wchar_t wchCode ) const
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
	if ( wchCode <= 0x20 )
	{
		return	0 ;			// 空白・制御文字
	}
	if ( wchCode < 0x80 )
	{
		int	i = (wchCode >> 5) ;
		int	j = 1 << (wchCode & 0x1F) ;
		if ( dwSpecialPuncMask[i] & j )
		{
			return	1 ;		// 特殊区切り記号
		}
		if ( dwPunctuationMask[i] & j )
		{
			return	2 ;		// 区切り記号
		}
		return	3 ;			// 半角アルファベット文字
	}
	if ( wchCode == L'　' )
	{
		return	4 ;
	}
	if ( (wchCode >= wchMarkChar[0]) && (wchCode <= wchMarkChar[1]) )
	{
		return	5 ;			// 全角英数字
	}
	if ( (wchCode >= wchMarkChar[2]) && (wchCode <= wchMarkChar[3]) )
	{
		return	6 ;			// 全角ひらがな
	}
	if ( (wchCode >= wchMarkChar[4]) && (wchCode <= wchMarkChar[5]) )
	{
		return	7 ;			// 全角カタカナ
	}
	if ( IsProhibitChar( wchCode ) )
	{
		return	4 ;			// 禁則文字（句読点などの区切り文字）
	}
	return	8 ;				// 漢字や記号、或いは、それ以外の言語の文字
}

// フォーカスを取得した
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::OnSetFocus( void )
{
	m_fFocus = true ;
	m_rsCaret.SetVisible( m_fViewCaret && !m_fIMEComposition ) ;
	UpdateCaretPos( m_iSelFirst, m_iSelEnd ) ;
}

// フォーカスを奪われた
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::OnKillFocus( void )
{
	m_fFocus = false ;
	m_rsCaret.SetVisible( false ) ;
	OnCommand( this, ncKillFocus, 0 ) ;
}

// 機能フラグを設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetFunctionFlags( DWORD dwFlags )
{
	ESpriteInterface::SetFunctionFlags( dwFlags | ffTimer ) ;
}

// カレットの表示状態を設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::ShowCaret( bool fShow )
{
	m_fViewCaret = fShow ;
	m_rsCaret.SetVisible( m_fViewCaret && m_fFocus && !m_fIMEComposition ) ;
}

// 行間を取得する
//////////////////////////////////////////////////////////////////////////////
unsigned int ETextEditSprite::GetLineHeight( void ) const
{
	return	m_rfiText.GetLineHeight( ) ;
}

// 編集テキストフラグを設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetEditFlags( DWORD dwEditFlags )
{
	m_dwEditFlags = dwEditFlags ;
	//
	DrawViewText( true ) ;
}

// タブ幅を設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetTabWidth( unsigned int nTabWidth, bool fRedraw )
{
	if ( m_nTabWidth != nTabWidth )
	{
		m_nTabWidth = nTabWidth ;
		if ( fRedraw )
		{
			UpdateAllLines( ) ;
		}
	}
}

// 左側余白幅を設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetLeftSpaceWidth( int nLeftSpace )
{
	if ( m_nLeftSpace != nLeftSpace )
	{
		m_nLeftSpace = nLeftSpace ;
		UpdateCaretPos( m_iSelFirst, m_iSelEnd ) ;
	}
}

// 行の折り返し幅を設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetWordWrapWidth( int nWordWrap, bool fRedraw )
{
	if ( m_nWordWrapWidth != nWordWrap )
	{
		m_nWordWrapWidth = nWordWrap ;
		if ( fRedraw )
		{
			UpdateAllLines( ) ;
		}
	}
}

// 禁則文字を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ETextEditSprite::GetProhibitChar( void ) const
{
	return	m_wstrProhibit ;
}

// 禁則文字を設定する
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetProhibitChar( const wchar_t * pwszProhibit )
{
	m_wstrProhibit = pwszProhibit ;
}

// 禁則文字か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::IsProhibitChar( wchar_t wchar ) const
{
	return	(m_wstrProhibit.Find( wchar ) >= 0) ;
}

// 文字列入力オブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError ETextEditSprite::CreateEdit( const EDIT_STYLE & style )
{
	DetachAllSprite( ) ;
	//
	// 操作情報初期化
	//
	m_lstLine.RemoveAll( ) ;
	m_tsaImgBuf.RemoveAll( ) ;
	m_lstUndo.RemoveAll( ) ;
	m_lstRedo.RemoveAll( ) ;
	//
	// 画像バッファを生成する
	//
	m_etType = style.etType ;
	m_dwEditFlags = 0 ;
	m_nLimitLength = 0 ;
	//
	if ( ResizeEdit( style ) )
	{
		return	eslErrGeneral ;
	}
	//
	// 各種属性設定
	//
	m_sizeCaret = style.sizeCaret ;
	m_nCaretInterval = style.nCaretInterval ;
	m_rgbTextColor = style.rgbTextColor ;
	m_rgbSelTextColor = style.rgbSelTextColor ;
	m_rgbCaretColor = style.rgbCaretColor ;
	//
	if ( style.fIMEFont )
	{
		::eslMoveMemory( &m_lfIMC, &style.lfIMEFont, sizeof(LOGFONT) ) ;
	}
	else
	{
		::eslMoveMemory( &m_lfIMC, &style.lfEditFont, sizeof(LOGFONT) ) ;
	}
	::eslMoveMemory( &m_lfFont, &style.lfEditFont, sizeof(LOGFONT) ) ;
	if ( m_hFont != NULL )
	{
		::DeleteObject( m_hFont ) ;
	}
	m_hFont = ::CreateFontIndirect( &m_lfFont ) ;
	//
	if ( m_hIMM32 == NULL )
	{
		m_hIMM32 = ::LoadLibrary( "imm32.dll" ) ;
		if ( m_hIMM32 != NULL )
		{
			m_apiGetCompositionStringW = (API_ImmGetCompositionString)
				::GetProcAddress( m_hIMM32, "ImmGetCompositionStringW" ) ;
		}
	}
	//
	m_xScroll = 0 ;
	m_yScroll = 0 ;
	m_iSelFirst = 0 ;
	m_iSelEnd = 0 ;
	//
	m_rfiText.RemoveAllCharacter( ) ;
//	m_rfiText.SetFont( m_hFont ) ;
	SakuraGL::SGLFont *		pFont = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	styleTemp ;
	SSystem::SString		strFontTemp ;
	styleTemp.FromLogFont( m_lfFont, strFontTemp ) ;
	pFont->SetStyle( styleTemp ) ;
	m_rfiText.SetSGLFont( pFont ) ;
	//
	m_rfiText.SetViewRect( EGLRect( 0, 0, 0xFFFF, 0xFFFF ) ) ;
	m_rfiText.SetColor( style.rgbTextColor ) ;
	m_rfiText.SetLineHeight( style.nEditBottom - style.nEditTop + 1 ) ;
	//
	// 表示設定
	//
	AddSprite( 0x10, &m_isBackPanel ) ;
	AddSprite( 0, &m_isTextPanel ) ;
	AddSprite( -0x10, &m_rsCaret ) ;
	m_rsCaret.SetColor( m_rgbCaretColor ) ;
	m_isBackPanel.SetVisible( true ) ;
	m_isTextPanel.SetVisible( true ) ;
	//
	SetFunctionFlags( GetFunctionFlags() ) ;
	//
	return	eslErrSuccess ;
}

// サイズ変更
//////////////////////////////////////////////////////////////////////////////
ESLError ETextEditSprite::ResizeEdit( const EDIT_STYLE & style )
{
	int		nWidth, nHeight ;
	int		nWayCount, nSideWidth ;
	//
	if ( (style.pLeftSide == NULL)
		|| (style.pRightSide == NULL)
		|| (style.pTextWay == NULL) )
	{
		nWidth = style.sizeExt.w ;
		nHeight = style.sizeExt.h ;
		//
		// 画像バッファを生成する
		//
		if ( CreateImage( EIF_RGBA_BITMAP, nWidth, nHeight, 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
/*		if ( m_isBackPanel.CreateImage
				( EIF_RGBA_BITMAP, nWidth, nHeight, 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
*/		if ( m_isTextPanel.CreateImage
				( EIF_RGBA_BITMAP, nWidth, nHeight, 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
	}
	else
	{
		nSideWidth = style.pLeftSide->dwImageWidth
					+ style.pRightSide->dwImageWidth ;
		nWayCount = (style.sizeExt.w - nSideWidth)
					/ (int) style.pTextWay->dwImageWidth ;
		if ( nWayCount <= 0 )
		{
			nWayCount = 1 ;
		}
		nWidth = nSideWidth + nWayCount * style.pTextWay->dwImageWidth ;
		//
		nHeight = style.sizeExt.h ;
		if ( nHeight < (int) style.pLeftSide->dwImageHeight )
			nHeight = style.pLeftSide->dwImageHeight ;
		if ( nHeight < (int) style.pRightSide->dwImageHeight )
			nHeight = style.pRightSide->dwImageHeight ;
		if ( nHeight < (int) style.pTextWay->dwImageHeight )
			nHeight = style.pTextWay->dwImageHeight ;
		//
		// 画像バッファを生成する
		//
		if ( CreateImage( EIF_RGBA_BITMAP, nWidth, nHeight, 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
		if ( m_isBackPanel.CreateImage
				( EIF_RGBA_BITMAP, nWidth, nHeight, 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
		if ( m_isTextPanel.CreateImage
				( EIF_RGBA_BITMAP, (nWidth - nSideWidth),
					(style.nEditBottom - style.nEditTop + 1), 32 ) == NULL )
		{
			return	eslErrGeneral ;
		}
		m_isTextPanel.MovePosition
			( EGLPoint( style.pLeftSide->dwImageWidth, style.nEditTop ) ) ;
		//
		// 背景画像をセットアップする
		//
		EGL_DRAW_PARAM	edp ;
		int		i, x = 0 ;
		::eslFillMemory( &edp, 0, sizeof(edp) ) ;
		if ( m_hDrawLine == NULL )
		{
			m_hDrawLine = ::eglCreateDrawImage( ) ;
		}
		m_hDrawLine->Initialize( m_isBackPanel, NULL, NULL ) ;
		//
		edp.pSrcImage = style.pLeftSide ;
		if ( !m_hDrawLine->PrepareDraw( &edp ) )
		{
			m_hDrawLine->DrawImage( ) ;
		}
		//
		x += style.pLeftSide->dwImageWidth ;
		for ( i = 0; i < nWayCount; i ++ )
		{
			edp.ptBasePos.x = x ;
			edp.pSrcImage = style.pTextWay ;
			if ( !m_hDrawLine->PrepareDraw( &edp ) )
			{
				m_hDrawLine->DrawImage( ) ;
			}
			x += style.pTextWay->dwImageWidth ;
		}
		//
		edp.ptBasePos.x = x ;
		edp.pSrcImage = style.pRightSide ;
		if ( !m_hDrawLine->PrepareDraw( &edp ) )
		{
			m_hDrawLine->DrawImage( ) ;
		}
	}
	//
	if ( m_nWordWrapWidth >= 0 )
	{
		DrawViewText( false ) ;
	}
	else
	{
		UpdateAllLines( ) ;
		UpdateCaretPos( m_iSelFirst, m_iSelEnd ) ;
	}
	//
	return	eslErrSuccess ;
}

// 属性変更
//////////////////////////////////////////////////////////////////////////////
ESLError ETextEditSprite::ModifyEditStyle( const EDIT_STYLE & style )
{
	//
	// 各種属性設定
	//
	m_sizeCaret = style.sizeCaret ;
	m_nCaretInterval = style.nCaretInterval ;
	m_rgbTextColor = style.rgbTextColor ;
	m_rgbSelTextColor = style.rgbSelTextColor ;
	m_rgbCaretColor = style.rgbCaretColor ;
	//
	if ( style.fIMEFont )
	{
		::eslMoveMemory( &m_lfIMC, &style.lfIMEFont, sizeof(LOGFONT) ) ;
	}
	else
	{
		::eslMoveMemory( &m_lfIMC, &style.lfEditFont, sizeof(LOGFONT) ) ;
	}
	m_rfiText.SetFont( NULL ) ;
	if ( m_hFont != NULL )
	{
		::DeleteObject( m_hFont ) ;
	}
	m_hFont = ::CreateFontIndirect( &(style.lfEditFont) ) ;
	//
	m_rfiText.RemoveAllCharacter( ) ;
//	m_rfiText.SetFont( m_hFont ) ;
	SakuraGL::SGLFont *		pFont = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	styleTemp ;
	SSystem::SString		strFontTemp ;
	styleTemp.FromLogFont( m_lfIMC, strFontTemp ) ;
	pFont->SetStyle( styleTemp ) ;
	m_rfiText.SetSGLFont( pFont ) ;
	//
	m_rfiText.SetViewRect( EGLRect( 0, 0, 0xFFFF, 0xFFFF ) ) ;
	m_rfiText.SetColor( style.rgbTextColor ) ;
	m_rfiText.SetLineHeight( style.nEditBottom - style.nEditTop + 1 ) ;
	//
	m_rsCaret.SetColor( m_rgbCaretColor ) ;
	//
	// フォント再構築
	//
	if ( m_pEditServer == NULL )
	{
		EWideString	wstrText = GetEditText( ) ;
		int			iSelFirst = m_iSelFirst, iSelEnd = m_iSelEnd ;
		int			xScroll = m_xScroll, yScroll = m_yScroll ;
		//
		m_rfiText.RemoveAllCharacter( ) ;
		m_tsaImgBuf.RemoveAll( ) ;
		m_lstLine.RemoveAll( ) ;
		//
		m_xScroll = 0 ;
		m_yScroll = 0 ;
		SetSel( 0, 0 ) ;
		SetEditText( wstrText ) ;
		//
		SetScrollPos( xScroll, yScroll ) ;
		SetSel( iSelFirst, iSelEnd ) ;
	}
	return	eslErrSuccess ;
}

// 全ての行の書式を再整形
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::UpdateAllLines( void )
{
	UpdateLineInfo( -1, true ) ;
}

// 文字指標からｘ座標を計算
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetCharPosFromIndex( int iChar ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetCharPosFromIndex( iChar ) ;
	}
	int					iLine = GetLineFromIndex( iChar ) ;
	const ELineInf *	pLInf = GetLineAt( iLine ) ;
	int					x = 0 ;
	if ( pLInf != NULL )
	{
		iChar -= GetLineIndex( iLine ) ;
		for ( int i = 0; i < iChar; i ++ )
		{
			ECharacter *	pChar = pLInf->m_aryText.GetAt( i ) ;
			if ( pChar != NULL )
			{
				x += pChar->m_nWidth ;
			}
		}
	}
	return	x ;
}

// 文字指標から行番号を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetLineFromIndex( int iChar ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLineFromIndex( iChar ) ;
	}
	if ( (m_etType == etSingleLine) || (m_lstLine.GetSize() == 0) )
	{
		return	0 ;
	}
	for ( int i = 0; i < (int) m_lstLine.GetSize(); i ++ )
	{
		int	nLineLen = m_lstLine[i].m_wstrLine.GetLength() ;
		if ( iChar < nLineLen )
		{
			return	i ;
		}
		iChar -= nLineLen ;
	}
	ELineInf *	pLInf = m_lstLine.GetLastAt( ) ;
	if ( pLInf != NULL )
	{
		if ( !pLInf->m_wstrLine.IsEmpty() )
		{
			if ( pLInf->m_wstrLine.GetAt
					( pLInf->m_wstrLine.GetLength() - 1 ) == L'\n' )
			{
				return	m_lstLine.GetSize() ;
			}
		}
	}
	return	m_lstLine.GetSize() - 1 ;
}

// 行の先頭の文字指標を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetLineIndex( int nLine ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLineIndex( nLine ) ;
	}
	if ( (unsigned int) nLine >= m_lstLine.GetSize() )
	{
		nLine = m_lstLine.GetSize() - 1 ;
		if ( nLine < 0 )
		{
			return	0 ;
		}
		return	m_lstLine[nLine].m_nIndex
				+ m_lstLine[nLine].m_wstrLine.GetLength( ) ;
	}
	return	m_lstLine[nLine].m_nIndex ;
}

// 行の先頭の文字数を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetLineLength( int nLine ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLineLength( nLine ) ;
	}
	if ( (unsigned int) nLine >= m_lstLine.GetSize() )
	{
		return	0 ;
	}
	return	m_lstLine[nLine].m_wstrLine.GetLength( ) ;
}

// 行数を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetLineCount( void ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLineCount( ) ;
	}
	return	m_lstLine.GetSize( ) ;
}

// 全文字数取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetLength( void ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLength( ) ;
	}
	return	GetLineIndex( GetLineCount() ) ;
}

// 指定行の文字列を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ETextEditSprite::GetLineText( int nLine ) const
{
	const ELineInf *	pLInf = GetLineAt( nLine ) ;
	if ( pLInf != NULL )
	{
		return	pLInf->m_wstrLine ;
	}
	return	EWideString() ;
}

// スクロール位置取得（ピクセル, 行）
//////////////////////////////////////////////////////////////////////////////
EGLPoint ETextEditSprite::GetScrollPos( void ) const
{
	return	EGLPoint( m_xScroll, m_yScroll ) ;
}

// スクロール位置設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetScrollPos( int xPos, int nLine )
{
	if ( xPos < - m_nLeftSpace )
	{
		xPos = - m_nLeftSpace ;
	}
	if ( nLine < 0 )
	{
		nLine = 0 ;
	}
	else if ( nLine > GetLineCount() )
	{
		nLine = GetLineCount() ;
	}
	if ( (m_xScroll != xPos) || (m_yScroll != nLine) )
	{
		EGLPoint	ptMove ;
		EGLRect	rect = m_rsCaret.GetRectangle() ;
		ptMove.x = m_xScroll - xPos ;
		ptMove.y = (m_yScroll - nLine) * (int) GetLineHeight() ;
		rect.left += ptMove.x ;
		rect.top += ptMove.y ;
		rect.right += ptMove.x ;
		rect.bottom += ptMove.y ;
		m_xScroll = xPos ;
		m_yScroll = nLine ;
		m_rsCaret.SetRectangle( rect ) ;
		DrawViewText( false ) ;
		OnScrollPos( ) ;
	}
}

// 行の最大幅（ピクセル）を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetMaxLineWidth( void ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetMaxLineWidth( ) ;
	}
	return	m_nMaxLineWidth ;
}

// 指定行取得
//////////////////////////////////////////////////////////////////////////////
ETextEditSprite::ELineInf * ETextEditSprite::GetLineAt( int nLine ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetLineAt( nLine ) ;
	}
	return	m_lstLine.GetAt( nLine ) ;
}

// 文字列描画
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DrawViewText( bool fChanged )
{
//	m_isTextPanel.FillImage( EGLPalette( 0 ) ) ;
	if ( m_etType == etSingleLine )
	{
		DrawViewLine( 0, fChanged ) ;
	}
	else
	{
		m_isTextPanel.UpdateRect( NULL ) ;
		for ( int y = m_yScroll; y <= GetLineCount(); y ++ )
		{
			if ( (y - m_yScroll) * m_rfiText.GetLineHeight()
									> m_isTextPanel.GetHeight() )
			{
				break ;
			}
			DrawViewLine( y, false ) ;
		}
	}
	if ( fChanged )
	{
		unsigned int		i, nCount ;
		nCount = m_lstEditClient.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ETextEditSprite *	pClient = m_lstEditClient.GetAt( i ) ;
			if ( pClient != NULL )
			{
				pClient->DrawViewText( fChanged ) ;
			}
		}
	}
}

void ETextEditSprite::DrawViewLine( int nLine, bool fChanged )
{
	//
	// エディットクライアントの表示を更新させる
	//
	unsigned int		i, nCount ;
	if ( fChanged )
	{
		nCount = m_lstEditClient.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ETextEditSprite *	pClient = m_lstEditClient.GetAt( i ) ;
			if ( pClient != NULL )
			{
				pClient->DrawViewLine( nLine, fChanged ) ;
			}
		}
	}
	//
	// 処理準備
	//
	if ( (nLine < m_yScroll)
		|| ((nLine - m_yScroll) * m_rfiText.GetLineHeight()
									>= m_isTextPanel.GetHeight()) )
	{
		return ;
	}
	EGL_DRAW_PARAM		dp ;
	EGL_RECT			rctExLine, rctFill ;
	const ELineInf *	pLInf = GetLineAt( nLine ) ;
	//
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
	//
	dp.ptBasePos.y = (nLine - m_yScroll) * m_rfiText.GetLineHeight() ;
	rctExLine.top = dp.ptBasePos.y ;
	rctExLine.bottom = rctExLine.top + m_rfiText.GetLineHeight() - 1 ;
	rctExLine.left = 0 ;
	rctExLine.right = m_isTextPanel.GetWidth() - 1 ;
	//
	// 最終行の場合には余白をクリアする
	//
	rctFill = rctExLine ;
	if ( (nLine >= GetLineCount())
		&& (rctFill.bottom < (int) m_isTextPanel.GetHeight()) )
	{
		rctFill.bottom = m_isTextPanel.GetHeight() - 1 ;
	}
	//
	if ( m_hDrawLine == NULL )
	{
		m_hDrawLine = ::eglCreateDrawImage( ) ;
	}
	m_hDrawLine->Initialize( m_isTextPanel, NULL/*&rctFill*/, NULL ) ;
	//
	// 背景初期化
	//
	if ( !m_hDrawLine->PrepareFillRect( &rctFill, EGLPalette(0), 0, 0 ) )
	{
		m_hDrawLine->FillRegion( ) ;
	}
	if ( (nLine < m_yScroll) || (pLInf == NULL)
		|| (dp.ptBasePos.y >= (int) m_isTextPanel.GetHeight()) )
	{
		m_isTextPanel.UpdateRect( &rctFill ) ;
		return ;
	}
	int		iSelFirst, iSelEnd, iSelFirstLine, iSelEndLine ;
	if ( m_iSelFirst < m_iSelEnd )
	{
		iSelFirst = m_iSelFirst ;
		iSelEnd = m_iSelEnd ;
	}
	else
	{
		iSelFirst = m_iSelEnd ;
		iSelEnd = m_iSelFirst ;
	}
	if ( iSelFirst != iSelEnd )
	{
		iSelFirstLine = GetLineFromIndex( iSelFirst ) ;
		iSelEndLine = GetLineFromIndex( iSelEnd ) ;
		if ( (iSelFirstLine <= nLine) && (nLine <= iSelEndLine) )
		{
			EGL_RECT	rectSel ;
			if ( iSelFirstLine < nLine )
			{
				rectSel.left = - m_xScroll ;
				if ( rectSel.left < 0 )
				{
					rectSel.left = 0 ;
				}
			}
			else
			{
				rectSel.left = GetCharPosFromIndex( iSelFirst ) - m_xScroll ;
			}
			if ( iSelEndLine > nLine )
			{
				rectSel.right = pLInf->m_nWidth - 1 - m_xScroll ;
			}
			else
			{
				rectSel.right = GetCharPosFromIndex( iSelEnd ) - 1 - m_xScroll ;
			}
			rectSel.top = rctExLine.top ;
			rectSel.bottom = rctExLine.bottom ;
			//
			if ( !m_hDrawLine->PrepareFillRect
				( &rectSel, m_rgbCaretColor, 0, EGL_DRAW_BLEND_ALPHA ) )
			{
				m_hDrawLine->FillRegion( ) ;
			}
		}
	}
	//
	// 文字列描画
	//
	int		xPos = 0 ;
	nCount = pLInf->m_aryText.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		bool	fSelText =
			(iSelFirst <= (int) (i + pLInf->m_nIndex))
				&& ((int) (i + pLInf->m_nIndex) < iSelEnd) ;
		xPos += DrawViewCharacter
			( m_hDrawLine, pLInf, i,
				xPos - m_xScroll, rctExLine.top, fSelText ) ;
	}
	//
	m_isTextPanel.UpdateRect( &rctFill ) ;
}

// 文字描画
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::DrawViewCharacter
	( HEGL_DRAW_IMAGE hDraw,
		const ELineInf * pLInf,
		int nIndex, int xPos, int yPos, bool fSelText )
{
	EGL_DRAW_PARAM	dp ;
	ECharacter *	pChar = pLInf->m_aryText.GetAt( nIndex ) ;
	if ( (pChar == NULL) || (pChar->m_pBuf == NULL) )
	{
		return	0 ;
	}
	if ( pChar->m_pBuf->GetInfo() == NULL )
	{
		RenderingCharacterImage( pChar->m_pBuf ) ;
	}
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
	dp.ptBasePos = pChar->m_pBuf->GetPosition() ;
	dp.ptBasePos.x += xPos ;
	dp.ptBasePos.y += yPos ;
	dp.pSrcImage = *(pChar->m_pBuf) ;
	if ( dp.pSrcImage != NULL )
	{
		if ( fSelText )
		{
			dp.rgbDimColor = m_rgbSelTextColor ;
			dp.rgbLightColor = m_rgbSelTextColor ;
		}
		else
		{
			dp.rgbDimColor = m_rgbTextColor ;
			dp.rgbLightColor = m_rgbTextColor ;
		}
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
	return	pChar->m_nWidth ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
EWideString ETextEditSprite::GetEditText( void ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetEditText( ) ;
	}
	EWideString	wstrText ;
	for ( int i = 0; i < (int) m_lstLine.GetSize(); i ++ )
	{
		unsigned int	nLen ;
		wstrText += m_lstLine[i].m_wstrLine ;
		nLen = wstrText.GetLength( ) ;
		if ( nLen >= 1 )
		{
			wchar_t	wch = wstrText.GetAt( nLen - 1 ) ;
			if ( wch == L'\r' )
			{
				wstrText += L'\n' ;
			}
			else if ( wch == L'\n' )
			{
				wstrText.SetAt( nLen - 1, L'\r' ) ;
				wstrText += L'\n' ;
			}
		}
	}
	return	wstrText ;
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetEditText( const wchar_t * pwszText )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->SetEditText( pwszText ) ;
		return ;
	}
	m_rfiText.RemoveAllCharacter( ) ;
	m_tsaImgBuf.RemoveAll( ) ;
	m_lstLine.RemoveAll( ) ;
	m_lstUndo.RemoveAll( ) ;
	m_lstRedo.RemoveAll( ) ;
	//
	m_xScroll = 0 ;
	m_yScroll = 0 ;
	SetSel( 0, 0 ) ;
	//
	if ( (m_etType == etSingleLine)
		|| !(m_dwEditFlags & efInputReturn) )
	{
		ReplaceSelText( pwszText ) ;
	}
	else if ( pwszText != NULL )
	{
		int	iLast = 0, iNext = 0 ;
		int	nLimitLeft = m_nLimitLength ;
		if ( nLimitLeft == 0 )
		{
			nLimitLeft = 0x7FFFFFFF ;
		}
		while ( pwszText[iNext] )
		{
			if ( pwszText[iNext] == L'\n' )
			{
				EWideString	wstrLine( pwszText + iLast, iNext - iLast + 1 ) ;
				if ( nLimitLeft >= 0 )
				{
					if ( (int) wstrLine.GetLength() > nLimitLeft )
					{
						wstrLine = wstrLine.Left( nLimitLeft ) ;
					}
					nLimitLeft -= wstrLine.GetLength( ) ;
				}
				else
				{
					EString	strLine = wstrLine ;
					if ( (int) strLine.GetLength() > - nLimitLeft )
					{
						strLine = strLine.Left( - nLimitLeft ) ;
						wstrLine = strLine ;
					}
					nLimitLeft += strLine.GetLength( ) ;
				}
				AddTextSimply( wstrLine ) ;
				iLast = iNext + 1 ;
				//
				if ( nLimitLeft == 0 )
				{
					break ;
				}
			}
			iNext ++ ;
		}
		EWideString	wstrLastLine( pwszText + iLast ) ;
		if ( wstrLastLine.GetLength() )
		{
			if ( nLimitLeft >= 0 )
			{
				if ( (int) wstrLastLine.GetLength() > nLimitLeft )
				{
					wstrLastLine = wstrLastLine.Left( nLimitLeft ) ;
				}
				nLimitLeft -= wstrLastLine.GetLength( ) ;
			}
			else
			{
				EString	strLine = wstrLastLine ;
				if ( (int) strLine.GetLength() > - nLimitLeft )
				{
					strLine = strLine.Left( - nLimitLeft ) ;
					wstrLastLine = strLine ;
				}
				nLimitLeft += strLine.GetLength( ) ;
			}
			AddTextSimply( wstrLastLine ) ;
		}
		UpdateAllLines( ) ;
		OnScrollSize( ) ;
	}
}

// 文字列追加（単純処理）
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::AddTextSimply( const EWideString & wstrText )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->AddTextSimply( wstrText ) ;
		return ;
	}
	//
	// 最後の行を追加
	//
	int			iLine = GetLineCount( ) ;
	ELineInf *	pLInf = GetLineAt( iLine ) ;
	if ( pLInf == NULL )
	{
		pLInf = new ELineInf ;
		pLInf->m_nIndex = GetLength( ) ;
		m_lstLine.SetAt( iLine, pLInf ) ;
	}
	//
	// 文字データ生成
	//
	EWideString	wstrReplace ;
	int			i, nTextLen = wstrText.GetLength( ) ;
	pLInf->m_wstrLine.AllocString
		( pLInf->m_wstrLine.GetLength() + nTextLen + 1 ) ;
	pLInf->m_aryText.SetLimit
		( pLInf->m_aryText.GetSize() + nTextLen + 1 ) ;
	ESLAssert( pLInf->m_aryText.GetSize()
					== pLInf->m_wstrLine.GetLength() ) ;
	//
	for ( i = 0; i < nTextLen; i ++ )
	{
		ECharacterBuffer *	pCharImg ;
		wchar_t	wch = wstrText.GetAt( i ) ;
		if ( wch == L'\n' )
		{
			if ( (m_etType == etSingleLine)
				|| !(m_dwEditFlags & efInputReturn) )
			{
				continue ;
			}
		}
		else if ( (wch < 0x20) && (wch != '\t') )
		{
			continue ;
		}
		pCharImg = m_tsaImgBuf.GetAs( wch ) ;
		if ( pCharImg == NULL )
		{
			pCharImg = CreateCharacterImage( wch ) ;
			m_tsaImgBuf.SetAs( wch, pCharImg ) ;
		}
		//
		ECharacter *	pChar = new ECharacter ;
		pChar->m_pBuf = pCharImg ;
		pChar->m_nWidth = pCharImg->m_nCharWidth ;
		pLInf->m_aryText.Add( pChar ) ;
		pLInf->m_wstrLine += wch ;
	}
	ESLAssert( pLInf->m_aryText.GetSize()
					== pLInf->m_wstrLine.GetLength() ) ;
}

// 選択範囲取得
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::GetSel( int & iSelFirst, int & iSelEnd ) const
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->GetSel( iSelFirst, iSelEnd ) ;
		return ;
	}
	iSelFirst = m_iSelFirst ;
	iSelEnd = m_iSelEnd ;
}

// 選択範囲設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetSel( int iSelFirst, int iSelEnd )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->SetSel( iSelFirst, iSelEnd ) ;
		return ;
	}
	//
	// 選択範囲を正規化
	//
	if ( (iSelFirst == 0) && (iSelEnd < 0) )
	{
		iSelFirst = 0 ;
		iSelEnd = GetLength() ;
	}
	else
	{
		int	nTotalLength = GetLineIndex( GetLineCount() ) ;
		if ( iSelFirst < 0 )
		{
			iSelFirst = 0 ;
		}
		else if ( iSelFirst > nTotalLength )
		{
			iSelFirst = nTotalLength ;
		}
		if ( iSelEnd < 0 )
		{
			iSelEnd = 0 ;
		}
		else if ( iSelEnd > nTotalLength )
		{
			iSelEnd = nTotalLength ;
		}
	}
	//
	// カレットの情報を更新
	//
	UpdateCaretPos( iSelFirst, iSelEnd ) ;
	//
	unsigned int	i, nCount ;
	nCount = m_lstEditClient.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETextEditSprite *	pClient = m_lstEditClient.GetAt( i ) ;
		if ( pClient != NULL )
		{
			pClient->UpdateCaretPos( iSelFirst, iSelEnd ) ;
		}
	}
}

// カレットの表示位置を更新
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::UpdateCaretPos( int iSelFirst, int iSelEnd )
{
	int	iLastSelFirst = m_iSelFirst ;
	int	iLastSelEnd = m_iSelEnd ;
	m_iSelFirst = iSelFirst ;
	m_iSelEnd = iSelEnd ;
	//
	if ( m_isTextPanel.GetInfo() == NULL )
	{
		return ;
	}
	bool	fScroll = false ;
	if ( m_etType == etSingleLine )
	{
		int		xCaret = GetCharPosFromIndex( iSelEnd ) ;
		int		nWidth = m_isTextPanel.GetWidth() ;
		if ( m_fFocus )
		{
			if ( xCaret < m_xScroll )
			{
				m_xScroll = xCaret ;
				fScroll = true ;
			}
			else if ( xCaret + 8 >= m_xScroll + nWidth )
			{
				m_xScroll = xCaret + 8 - nWidth ;
				fScroll = true ;
			}
		}
		//
		// カレットの位置を設定
		//
		EGL_RECT	rcCaret ;
		EGL_RECT	rcTextPanel ;
		rcTextPanel = m_isTextPanel.GetRectangle( ) ;
		rcCaret.left = rcTextPanel.left + xCaret - m_xScroll ;
		rcCaret.bottom = rcTextPanel.bottom ;
		if ( m_sizeCaret.h == 0 )
		{
			rcCaret.top = rcTextPanel.top ;
		}
		else
		{
			rcCaret.top = rcTextPanel.bottom - m_sizeCaret.h + 1 ;
		}
		if ( m_sizeCaret.w == 0 )
		{
			const ELineInf *	pLInf = GetLineAt( 0 ) ;
			rcCaret.right = rcCaret.left + 7 ;
			ECharacter *	pChar = NULL ;
			if ( pLInf != NULL )
			{
				pChar = pLInf->m_aryText.GetAt( m_iSelEnd ) ;
				if ( pChar == NULL )
				{
					pChar = pLInf->m_aryText.GetLastAt( ) ;
				}
			}
			if ( pChar != NULL )
			{
				rcCaret.right = rcCaret.left + pChar->m_nWidth - 1 ;
			}
		}
		else
		{
			rcCaret.right = rcCaret.left + m_sizeCaret.w - 1 ;
		}
		if ( rcCaret.right >= rcTextPanel.right )
		{
			rcCaret.right = rcTextPanel.right ;
		}
		//
		m_rsCaret.SetRectangle( rcCaret ) ;
		//
		// 再描画
		//
		DrawViewText( true ) ;
	}
	else
	{
		//
		// スクロール位置を調整
		//
		int		xCaret = GetCharPosFromIndex( iSelEnd ) ;
		int		nWidth = m_isTextPanel.GetWidth() ;
		if ( m_fFocus )
		{
			if ( xCaret < m_xScroll + m_nLeftSpace )
			{
				m_xScroll = xCaret - m_nLeftSpace ;
				fScroll = true ;
			}
			else if ( (m_nWordWrapWidth >= 0)
					&& ((xCaret + m_sizeCaret.w + m_lfIMC.lfHeight)
												>= (m_xScroll + nWidth)) )
			{
				m_xScroll = xCaret + m_sizeCaret.w + m_lfIMC.lfHeight - nWidth ;
				fScroll = true ;
			}
		}
		int		yCaret = GetLineFromIndex( iSelEnd ) ;
		if ( m_fFocus )
		{
			if ( (yCaret - m_yScroll + 1) * (int) m_rfiText.GetLineHeight()
										>= (int) m_isTextPanel.GetHeight() )
			{
				m_yScroll =
					yCaret - m_isTextPanel.GetHeight()
									/ m_rfiText.GetLineHeight() + 1 ;
				fScroll = true ;
			}
			if ( yCaret < m_yScroll )
			{
				m_yScroll = yCaret ;
				fScroll = true ;
			}
			if ( m_yScroll < 0 )
			{
				m_yScroll = 0 ;
				fScroll = true ;
			}
		}
		//
		// カレットの位置を設定
		//
		EGL_RECT	rcCaret ;
		const ELineInf *	pLInf = GetLineAt( yCaret ) ;
		rcCaret.left = GetCharPosFromIndex( iSelEnd ) - m_xScroll ;
		rcCaret.bottom =
			(yCaret - m_yScroll + 1) * m_rfiText.GetLineHeight() - 1 ;
		if ( m_sizeCaret.h == 0 )
		{
			rcCaret.top = rcCaret.bottom - m_rfiText.GetLineHeight() + 1 ;
		}
		else
		{
			rcCaret.top = rcCaret.bottom - m_sizeCaret.h + 1 ;
		}
		if ( m_sizeCaret.w == 0 )
		{
			rcCaret.right = rcCaret.left + 7 ;
			ECharacter *	pChar = NULL ;
			if ( pLInf != NULL )
			{
				pChar = pLInf->m_aryText.GetAt
							( iSelEnd - GetLineIndex( yCaret ) ) ;
				if ( pChar == NULL )
				{
					pChar = pLInf->m_aryText.GetLastAt( 0 ) ;
				}
			}
			if ( pChar != NULL )
			{
				rcCaret.right = rcCaret.left + pChar->m_nWidth - 1 ;
			}
		}
		else
		{
			rcCaret.right = rcCaret.left + m_sizeCaret.w - 1 ;
		}
		m_rsCaret.SetRectangle( rcCaret ) ;
		//
		// 再描画
		//
		int	iFirst = iSelFirst, iEnd = iSelEnd ;
		if ( iSelFirst > iSelEnd )
		{
			iFirst = iSelEnd ;
			iEnd = iSelFirst ;
		}
		if ( iLastSelFirst < iLastSelEnd )
		{
			iFirst = __min( iFirst, iLastSelFirst ) ;
			iEnd = __max( iEnd, iLastSelEnd ) ;
		}
		else
		{
			iFirst = __min( iFirst, iLastSelEnd ) ;
			iEnd = __max( iEnd, iLastSelFirst ) ;
		}
		int	iFirstLine = GetLineFromIndex( iFirst ) ;
		int	iEndLine = GetLineFromIndex( iEnd ) ;
		if ( fScroll )
		{
			DrawViewText( true ) ;
		}
		else
		{
			for ( int i = iFirstLine; i <= iEndLine; i ++ )
			{
				DrawViewLine( i, true ) ;
			}
		}
	}
	if ( fScroll )
	{
		OnScrollPos( ) ;
	}
	if ( m_fIMEComposition )
	{
		HIMC	hIMC ;
		COMPOSITIONFORM	cf ;
		EWindowSpriteInterface *	pWndItf = GetWindowInterface( ) ;
		EGL_POINT	ptCursor = m_isTextPanel.GetPosition( ) ;
		ptCursor.x += GetCharPosFromIndex( m_iSelEnd ) - m_xScroll ;
		ptCursor.y +=
			(GetLineFromIndex(m_iSelEnd) - m_yScroll + 1)
								* m_rfiText.GetLineHeight() ;
		if ( m_lfIMC.lfHeight > 0 )
		{
			ptCursor.y -= m_lfIMC.lfHeight ;
		}
		else
		{
			ptCursor.y -= m_rfiText.GetLineHeight() ;
		}
		LocalToWindowClient( ptCursor ) ;
		if ( (pWndItf != NULL) && (pWndItf->GetWindow() != NULL) )
		{
			HWND	hWnd = *(pWndItf->GetWindow()) ;
			hIMC = ::ImmGetContext( hWnd ) ;
			cf.dwStyle = CFS_POINT ;
			cf.ptCurrentPos.x = ptCursor.x ;
			cf.ptCurrentPos.y = ptCursor.y ;
			::ImmSetCompositionWindow( hIMC, &cf ) ;
			::ImmReleaseContext( hWnd, hIMC ) ;
		}
	}
	m_dwLastTime = ::GetTickCount( ) ;
}

// 選択範囲の文字列を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ETextEditSprite::GetSelText( void ) const
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->GetSelText() ;
	}
	return	GetRangeText( m_iSelFirst, m_iSelEnd ) ;
}

// 指定範囲の文字列を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ETextEditSprite::GetRangeText( int iFirst, int iEnd ) const
{
	//
	// 選択範囲の正規化
	//
	int		iSelFirst, iSelEnd ;
	if ( iFirst < iEnd )
	{
		iSelFirst = iFirst ;
		iSelEnd = iEnd ;
	}
	else
	{
		iSelFirst = iEnd ;
		iSelEnd = iFirst ;
	}
	//
	// 行情報取得
	//
	const ELineInf *	pLInf ;
	int					iFirstLine, iEndLine ;
	EWideString			wstrText ;
	iFirstLine = GetLineFromIndex( iSelFirst ) ;
	iEndLine = GetLineFromIndex( iSelEnd ) ;
	pLInf = GetLineAt( iFirstLine ) ;
	if ( iFirstLine != iEndLine )
	{
		//
		// 先頭行取得
		//
		if ( pLInf != NULL )
		{
			wstrText +=
				pLInf->m_wstrLine.Middle
					( iSelFirst - pLInf->m_nIndex ) ;
		}
		//
		// 中間の行を追加
		//
		for ( int i = iFirstLine + 1; i < iEndLine; i ++ )
		{
			pLInf = GetLineAt( i ) ;
			if ( pLInf != NULL )
			{
				wstrText += pLInf->m_wstrLine ;
			}
		}
		//
		// 最終行を追加
		//
		pLInf = GetLineAt( iEndLine ) ;
		if ( pLInf != NULL )
		{
			wstrText +=
				pLInf->m_wstrLine.Left( iSelEnd - pLInf->m_nIndex ) ;
		}
	}
	else if ( pLInf != NULL )
	{
		//
		// 行内処理
		//
		int		iLFirst = iSelFirst - pLInf->m_nIndex ;
		int		iLEnd = iSelEnd - pLInf->m_nIndex ;
		wstrText = pLInf->m_wstrLine.Middle( iLFirst, iLEnd - iLFirst ) ;
	}
	//
	return	wstrText ;
}

// 文字コピー可能か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::CanCopyText( void )
{
	return	(m_iSelFirst != m_iSelEnd) ;
}

// 文字切り取り可能か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::CanCutText( void )
{
	return	(m_iSelFirst != m_iSelEnd) && !(m_dwEditFlags & efReadOnly) ;
}

// 文字列貼り付け可能か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::CanPasteText( void )
{
	if ( !::OpenClipboard( NULL ) )
	{
		return	false ;
	}
	//
	HGLOBAL	hGlobal ;
	hGlobal = ::GetClipboardData( CF_UNICODETEXT ) ;
	if ( hGlobal == NULL )
	{
		hGlobal = ::GetClipboardData( CF_TEXT ) ;
		if ( hGlobal == NULL )
		{
			::CloseClipboard( ) ;
			return	false ;
		}
	}
	::CloseClipboard( ) ;
	//
	return	!(m_dwEditFlags & efReadOnly) ;
}

// 選択文字列削除
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DoClear( void )
{
	EUndoInf *	pUndo = new EUndoInf ;
	ClearSelText( pUndo ) ;
	RecordUndo( pUndo ) ;
}

void ETextEditSprite::ClearSelText( ETextEditSprite::EUndoInf * pUndo )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->ClearSelText( pUndo ) ;
		return ;
	}
	//
	// 選択範囲の正規化
	//
	if ( pUndo != NULL )
	{
		pUndo->m_iFirst = pUndo->m_iEnd = __min( m_iSelFirst, m_iSelEnd ) ;
		pUndo->m_wstrUndo = GetSelText( ) ;
	}
	if ( m_iSelFirst == m_iSelEnd )
	{
		return ;
	}
	int		iSelFirst, iSelEnd ;
	if ( m_iSelFirst < m_iSelEnd )
	{
		iSelFirst = m_iSelFirst ;
		iSelEnd = m_iSelEnd ;
	}
	else
	{
		iSelFirst = m_iSelEnd ;
		iSelEnd = m_iSelFirst ;
	}
	//
	// 文字表示位置シフト・行結合
	//
	int			iFirstLine, iEndLine ;
	bool		fFullRedraw = false ;
	ELineInf *	pLInf ;
	ELineInf *	pLInfFirst ;
	iFirstLine = GetLineFromIndex( iSelFirst ) ;
	iEndLine = GetLineFromIndex( iSelEnd ) ;
	pLInfFirst = GetLineAt( iFirstLine ) ;
	if ( pLInfFirst == NULL )
	{
		return ;
	}
	if ( iFirstLine != iEndLine )
	{
		//
		// 中間の行を削除
		//
		if ( iEndLine - iFirstLine >= 2 )
		{
			m_lstLine.RemoveBetween
				( iFirstLine + 1, iEndLine - (iFirstLine + 1) ) ;
		}
		iEndLine = iFirstLine + 1 ;
		//
		// 先頭行を整形
		//
		int	nLength = iSelFirst - pLInfFirst->m_nIndex ;
		ESLAssert( pLInfFirst->m_wstrLine.GetLength()
						== pLInfFirst->m_aryText.GetSize() ) ;
		pLInfFirst->m_wstrLine = pLInfFirst->m_wstrLine.Left( nLength ) ;
		pLInfFirst->m_aryText.RemoveBetween
			( nLength, pLInfFirst->m_aryText.GetSize() - nLength ) ;
		ESLAssert( pLInfFirst->m_wstrLine.GetLength()
						== pLInfFirst->m_aryText.GetSize() ) ;
		//
		// 最終行を整形
		//
		pLInf = GetLineAt( iEndLine ) ;
		if ( pLInf != NULL )
		{
			ESLAssert( pLInf->m_wstrLine.GetLength()
							== pLInf->m_aryText.GetSize() ) ;
			nLength = iSelEnd - pLInf->m_nIndex ;
			pLInf->m_wstrLine = pLInf->m_wstrLine.Middle( nLength ) ;
			pLInf->m_aryText.RemoveBetween( 0, nLength ) ;
			ESLAssert( pLInf->m_wstrLine.GetLength()
							== pLInf->m_aryText.GetSize() ) ;
		}
		fFullRedraw = true ;
	}
	else
	{
		//
		// 行内処理
		//
		int		iLFirst = iSelFirst - pLInfFirst->m_nIndex ;
		int		iLEnd = iSelEnd - pLInfFirst->m_nIndex ;
		ESLAssert( pLInfFirst->m_wstrLine.GetLength()
						== pLInfFirst->m_aryText.GetSize() ) ;
		pLInfFirst->m_wstrLine =
			pLInfFirst->m_wstrLine.Left( iLFirst )
				+ pLInfFirst->m_wstrLine.Middle( iLEnd ) ;
		pLInfFirst->m_aryText.RemoveBetween( iLFirst, iLEnd - iLFirst ) ;
		ESLAssert( pLInfFirst->m_wstrLine.GetLength()
						== pLInfFirst->m_aryText.GetSize() ) ;
	}
	//
	// 再描画
	//
	m_iSelFirst = m_iSelEnd = iSelFirst ;
	UpdateLineInfo( iFirstLine, !fFullRedraw ) ;
	if ( fFullRedraw )
	{
		DrawViewText( true ) ;
	}
	SetSel( iSelFirst, iSelFirst ) ;
	//
	OnCommand( this, ncChange, 0 ) ;
}

// 選択文字列切り取り
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DoCut( void )
{
	EUndoInf *	pUndo = new EUndoInf ;
	CutSelText( pUndo ) ;
	RecordUndo( pUndo ) ;
}

void ETextEditSprite::CutSelText( ETextEditSprite::EUndoInf * pUndo )
{
	CopySelText( ) ;
	ClearSelText( pUndo ) ;
}

// 選択文字列コピー
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DoCopy( void )
{
	CopySelText( ) ;
}

void ETextEditSprite::CopySelText( void )
{
	//
	// 選択文字列取得
	//
	EWideString	wstrSelText = GetSelText( ) ;
	EString		strSelText = wstrSelText ;
	//
	// クリップボードに保存
	//
	if ( ::OpenClipboard( NULL ) )
	{
		HGLOBAL	hGlobalText = ::GlobalAlloc
			( GMEM_MOVEABLE, (strSelText.GetLength() + 1) * sizeof(char) ) ;
		LPVOID	lpBuf = ::GlobalLock( hGlobalText ) ;
		::eslMoveMemory
			( lpBuf, strSelText.CharPtr(),
				(strSelText.GetLength() + 1) * sizeof(char) ) ;
		::GlobalUnlock( hGlobalText ) ;
		//
		HGLOBAL	hGlobalUnicode = ::GlobalAlloc
			( GMEM_MOVEABLE, (wstrSelText.GetLength() + 1) * sizeof(wchar_t) ) ;
		lpBuf = ::GlobalLock( hGlobalUnicode ) ;
		::eslMoveMemory
			( lpBuf, wstrSelText.CharPtr(),
				(wstrSelText.GetLength() + 1) * sizeof(wchar_t) ) ;
		::GlobalUnlock( hGlobalUnicode ) ;
		//
		::EmptyClipboard( ) ;
		::SetClipboardData( CF_TEXT, hGlobalText ) ;
		::SetClipboardData( CF_UNICODETEXT, hGlobalUnicode ) ; 
		::CloseClipboard( ) ;
	}
}

// 選択文字列貼りつけ
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DoPaste( void )
{
	EUndoInf *	pUndo = new EUndoInf ;
	PasteSelText( pUndo ) ;
	RecordUndo( pUndo ) ;
}

void ETextEditSprite::PasteSelText( ETextEditSprite::EUndoInf * pUndo )
{
	//
	// クリップボードから文字列取得
	//
	if ( !::OpenClipboard( NULL ) )
		return ;
	//
	HGLOBAL	hGlobal ;
	hGlobal = ::GetClipboardData( CF_UNICODETEXT ) ;
	if ( hGlobal != NULL )
	{
		LPVOID	lpBuf = ::GlobalLock( hGlobal ) ;
		if ( lpBuf != NULL )
		{
			EWideString	wstrText( (const wchar_t *) lpBuf ) ;
			::GlobalUnlock( hGlobal ) ;
			ReplaceSelText( wstrText, pUndo ) ;
		}
	}
	else
	{
		hGlobal = ::GetClipboardData( CF_TEXT ) ;
		if ( hGlobal != NULL )
		{
			LPVOID	lpBuf = ::GlobalLock( hGlobal ) ;
			if ( lpBuf != NULL )
			{
				EWideString	wstrText( (const char *) lpBuf ) ;
				::GlobalUnlock( hGlobal ) ;
				ReplaceSelText( wstrText, pUndo ) ;
			}
		}
	}
	//
	::CloseClipboard( ) ;
}

// 選択文字列置き換え
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DoReplace( const wchar_t * pwszText )
{
	EUndoInf *	pUndo = new EUndoInf ;
	ReplaceSelText( pwszText, pUndo ) ;
	RecordUndo( pUndo ) ;
}

void ETextEditSprite::ReplaceSelText
	( const wchar_t * pwszText, ETextEditSprite::EUndoInf * pUndo )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->ReplaceSelText( pwszText, pUndo ) ;
		return ;
	}
	//
	// 選択文字列を削除
	//
	ClearSelText( pUndo ) ;
	//
	// 現在の行を取得
	//
	ESLAssert( m_iSelFirst == m_iSelEnd ) ;
	m_iSelFirst = m_iSelEnd ;
	//
	ELineInf *	pLInf ;
	int			iLine = GetLineFromIndex( m_iSelEnd ) ;
	pLInf = GetLineAt( iLine ) ;
	if ( pLInf == NULL )
	{
		pLInf = new ELineInf ;
		pLInf->m_nIndex = GetLength( ) ;
		m_lstLine.SetAt( iLine, pLInf ) ;
	}
	//
	// 文字長取得
	//
	int			nTextLen = 0 ;
	if ( pwszText != NULL )
	{
		while ( pwszText[nTextLen] )
		{
			nTextLen ++ ;
		}
	}
	pLInf->m_aryText.SetLimit( pLInf->m_aryText.GetSize() + nTextLen + 1 ) ;
	//
	// 文字画像生成
	//
	int			i, iSelFirst, iSelEnd, nLimitLeft ;
	iSelFirst = m_iSelFirst - pLInf->m_nIndex ;
	iSelEnd = m_iSelEnd - pLInf->m_nIndex ;
	//
	nLimitLeft = 0x7FFFFFFF ;
	if ( m_nLimitLength != 0 )
	{
		if ( m_nLimitLength > 0 )
		{
			nLimitLeft = m_nLimitLength - GetLength( ) ;
		}
		else
		{
			EString	strText = GetEditText() ;
			EString	strReplace = pwszText ;
			nLimitLeft = - m_nLimitLength - strText.GetLength( ) ;
			if ( nLimitLeft > 0 )
			{
				nLimitLeft =
					EWideString( strReplace, nLimitLeft ).GetLength() ;
			}
		}
	}
	//
	EWideString	wstrReplace ;
	wstrReplace.AllocString( nTextLen + 1 ) ;
	for ( i = 0; (i < nTextLen) && (nLimitLeft > 0); i ++ )
	{
		ECharacterBuffer *	pCharImg ;
		wchar_t	wch = pwszText[i] ;
		if ( wch == L'\n' )
		{
			if ( (m_etType == etSingleLine)
				|| !(m_dwEditFlags & efInputReturn) )
			{
				continue ;
			}
		}
		else if ( (wch < 0x20) && (wch != '\t') )
		{
			continue ;
		}
		pCharImg = m_tsaImgBuf.GetAs( wch ) ;
		if ( pCharImg == NULL )
		{
			pCharImg = CreateCharacterImage( wch ) ;
			m_tsaImgBuf.SetAs( wch, pCharImg ) ;
		}
		//
		ECharacter *	pChar = new ECharacter ;
		pChar->m_pBuf = pCharImg ;
		pChar->m_nWidth = pCharImg->m_nCharWidth ;
		pLInf->m_aryText.InsertAt( iSelEnd ++, pChar ) ;
		//
		wstrReplace += wch ;
		nLimitLeft -- ;
	}
	//
	// 文字列挿入
	//
	pLInf->m_wstrLine =
		pLInf->m_wstrLine.Left( iSelFirst )
			+ wstrReplace + pLInf->m_wstrLine.Middle( iSelFirst ) ;
	//
	// 表示更新
	//
	iSelEnd += pLInf->m_nIndex ;
	UpdateLineInfo( iLine ) ;
	SetSel( iSelEnd, iSelEnd ) ;
	//
	if ( pUndo != NULL )
	{
		pUndo->m_iEnd = iSelEnd ;
	}
	//
	OnCommand( this, ncChange, 0 ) ;
}

// UNDO 可能か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::CanUndo( void )
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->CanUndo( ) ;
	}
	return	(m_lstUndo.GetSize() != 0) ;
}

// UNDO 実行
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::Undo( void )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->Undo( ) ;
		return ;
	}
	EUndoInf *	pUndo = m_lstUndo.GetLastAt() ;
	if ( pUndo != NULL )
	{
		EUndoInf *	pRedo = new EUndoInf ;
		SetSel( pUndo->m_iFirst, pUndo->m_iEnd ) ;
		ReplaceSelText( pUndo->m_wstrUndo, pRedo ) ;
		m_lstUndo.RemoveAt( m_lstUndo.GetSize() - 1 ) ;
		m_lstRedo.Add( pRedo ) ;
	}
}

// REDO 可能か？
//////////////////////////////////////////////////////////////////////////////
bool ETextEditSprite::CanRedo( void )
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->CanRedo( ) ;
	}
	return	(m_lstRedo.GetSize() != 0) ;
}

// REDO 実行
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::Redo( void )
{
	if ( m_pEditServer != NULL )
	{
		m_pEditServer->Redo( ) ;
		return ;
	}
	EUndoInf *	pRedo = m_lstRedo.GetLastAt() ;
	if ( pRedo != NULL )
	{
		EUndoInf *	pUndo = new EUndoInf ;
		SetSel( pRedo->m_iFirst, pRedo->m_iEnd ) ;
		ReplaceSelText( pRedo->m_wstrUndo, pUndo ) ;
		m_lstRedo.RemoveAt( m_lstRedo.GetSize() - 1 ) ;
		m_lstUndo.Add( pUndo ) ;
	}
}

// 指定の文字コードの文字画像を生成する
//////////////////////////////////////////////////////////////////////////////
ETextEditSprite::ECharacterBuffer *
	ETextEditSprite::CreateCharacterImage( wchar_t wch )
{
	ECharacterBuffer *	pCharaBuf = new ECharacterBuffer ;
	wchar_t	wchText[2] ;
	wchText[0] = wch ;
	wchText[1] = 0 ;
	pCharaBuf->m_nCharWidth = m_rfiText.GetTextWidth( wchText ) ;
	pCharaBuf->m_nCharCode = wch ;
	if ( (wchText[0] < 0x20) && (wchText[0] != L'\t') )
	{
		pCharaBuf->m_nCharWidth = 0 ;
	}
	return	pCharaBuf ;
}

// 指定文字コードの文字画像をレンダリングする
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::RenderingCharacterImage( ECharacterBuffer * pCharaBuf )
{
	wchar_t	wchText[2] ;
	wchText[0] = (wchar_t) pCharaBuf->m_nCharCode ;
	wchText[1] = 0 ;
	m_rfiText.RemoveAllCharacter( ) ;
	m_rfiText.MoveCursorPos( EGLPoint( 0, 0 ) ) ;
	m_rfiText.DrawText( wchText ) ;
	//
	EImageSprite *	pis = m_rfiText.GetCharacterAt( 0 ) ;
/*	EGL_POINT		ptNext = m_rfiText.GetCursorPos( ) ;
	pChara->m_nCharWidth = ptNext.x ;
	if ( (wchText[0] < 0x20) && (wchText[0] != L'\t') )
	{
		pChara->m_nCharWidth = 0 ;
	}
*/	if ( pis != NULL )
	{
		EGL_POINT	ptChar = pis->GetPosition() ;
		ptChar.y -= (m_rfiText.GetLineHeight() - m_lfIMC.lfHeight) / 2 ;
		pCharaBuf->DuplicateImage( pis->GetInfo() ) ;
		pCharaBuf->MovePosition( ptChar ) ;
	}
}

// 指定行の幅の調整および以降の行の文字指標の正規化と表示の更新
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::UpdateLineInfo( int nLeadLine, bool fRedraw )
{
	bool	fAllUpdate = false ;
	if ( nLeadLine < 0 )
	{
		nLeadLine = 0 ;
		fAllUpdate = true ;
	}
	ELineInf *	pLInf = GetLineAt( nLeadLine ) ;
	ELineInf *	pLInfNext ;
	if ( pLInf == NULL )
	{
		return ;
	}
	else if ( (nLeadLine > 0)
			&& IsProhibitChar( pLInf->m_wstrLine.GetAt(0) ) )
	{
		pLInfNext = GetLineAt( nLeadLine - 1 ) ;
		if ( (pLInfNext != NULL)
			&& (pLInfNext->m_wstrLine.Find( L'\n' ) < 0) )
		{
			pLInfNext->m_wstrLine += pLInf->m_wstrLine ;
			pLInfNext->m_aryText.Merge
				( pLInfNext->m_aryText.GetSize(), pLInf->m_aryText ) ;
			m_lstLine.RemoveAt( nLeadLine -- ) ;
			pLInf = pLInfNext ;
		}
	}
	int		nOrgLineCount = m_lstLine.GetSize( ) ;
	int		iLine = nLeadLine ;
	int		nLimitWidth = m_nWordWrapWidth ;
	int		nDelLines = 0 ;
	bool	fLineWidth = (m_etType == etMultiLine) ;
	bool	fFullRedraw = (m_etType == etSingleLine) ;
	bool	fChgMaxLineWidth = false ;
	bool	fSearchMaxLineWidth = true ;
	if ( m_nWordWrapWidth == 0 )
	{
		nLimitWidth = 0x7FFFFFFF ;
	}
	else if ( m_nWordWrapWidth < 0 )
	{
		nLimitWidth = m_isTextPanel.GetWidth()
					- (m_sizeCaret.w + m_lfIMC.lfHeight + m_nLeftSpace) ;
		m_nMaxLineWidth = nLimitWidth ;
		fSearchMaxLineWidth = false ;
	}
	if ( nLimitWidth < m_lfIMC.lfHeight * 2 )
	{
		nLimitWidth = m_lfIMC.lfHeight * 2 ;
	}
	for ( ; ; )
	{
		//
		// 行の幅を計算
		//
		bool	fRedrawLine = fLineWidth ;
		bool	fLineEdit = fAllUpdate ;
		bool	fProhibitWrap = false ;
		int		i, nWidth = 0 ;
		if ( fLineWidth )
		{
			ESLAssert( pLInf->m_aryText.GetSize()
							== pLInf->m_wstrLine.GetLength() ) ;
			for ( i = 0; i < (int) pLInf->m_aryText.GetSize(); i ++ )
			{
				int				nReturn = -1 ;
				ECharacter *	pChara = pLInf->m_aryText.GetAt( i ) ;
				wchar_t			wch = pLInf->m_wstrLine.GetAt( i ) ;
				if ( wch == L'\n' )
				{
					nReturn = 1 ;
				}
				else if ( pChara != NULL )
				{
					int	nCharaWidth = pChara->m_nWidth ;
					if ( (wch == L'\t') && (m_nTabWidth != 0) )
					{
						nCharaWidth = m_nTabWidth - (nWidth % m_nTabWidth) ;
						pChara->m_nWidth = nCharaWidth ;
					}
					nWidth += nCharaWidth ;
					if ( (nWidth > nLimitWidth) && (i >= 2) )
					{
						nWidth -= nCharaWidth ;
						nReturn = 0 ;
						//
						if ( IsProhibitChar( wch ) && (i >= 2)
							&& !IsProhibitChar
									( pLInf->m_wstrLine.GetAt(i - 1) ) )
						{
							i -- ;
							if ( pLInf->m_aryText.GetAt( i ) != NULL )
							{
								nWidth -= pLInf->m_aryText[i].m_nWidth ;
							}
							fProhibitWrap = true ;
						}
					}
				}
				if ( (nReturn >= 0) &&
					(i + nReturn < (int) pLInf->m_aryText.GetSize()) )
				{
					pLInfNext = new ELineInf ;
					nReturn += i ;
					m_lstLine.InsertAt( iLine + 1, pLInfNext ) ;
					pLInfNext->m_wstrLine =
						pLInf->m_wstrLine.Middle( nReturn ) ;
					pLInf->m_wstrLine = pLInf->m_wstrLine.Left( nReturn ) ;
					pLInfNext->m_aryText.Merge
						( 0, pLInf->m_aryText, nReturn,
							pLInf->m_aryText.GetSize() - nReturn ) ;
//					pLInf->m_aryText.SetSize( nReturn ) ;
					pLInf->m_aryText.SetLimit( nReturn ) ;
					ESLAssert( pLInf->m_aryText.GetSize()
									== pLInf->m_wstrLine.GetLength() ) ;
					fFullRedraw = true ;
					fLineEdit = true ;
					break ;
				}
			}
			pLInf->m_nWidth = nWidth ;
			//
			if ( (int) m_nMaxLineWidth < pLInf->m_nWidth )
			{
				m_nMaxLineWidth = pLInf->m_nWidth ;
				fChgMaxLineWidth = true ;
				fSearchMaxLineWidth = false ;
			}
		}
		OnUpdateLineInfo( pLInf ) ;
		//
		// 次の行取得
		//
		for ( ; ; )
		{
			pLInfNext = GetLineAt( ++ iLine ) ;
			if ( pLInfNext == NULL )
			{
				if ( (fRedrawLine || fFullRedraw) && fRedraw )
				{
					DrawViewLine( iLine - 1, true ) ;
					DrawViewLine( iLine, true ) ;
				}
				break ;
			}
			if ( !(pLInfNext->m_wstrLine.IsEmpty()) )
			{
				break ;
			}
			m_lstLine.RemoveAt( iLine -- ) ;
			nDelLines ++ ;
		}
		if ( pLInfNext == NULL )
		{
			break ;
		}
		//
		// 行の結合判定
		//
		if ( fLineWidth && !fProhibitWrap )
		{
			int	nLineEnd = pLInf->m_wstrLine.Find( L'\n' ) ;
			if ( nLineEnd < 0 )
			{
				int		nMergeLen = 0 ;
				nWidth = pLInf->m_nWidth ;
				for ( i = 0; i < (int) pLInfNext->m_aryText.GetSize(); i ++ )
				{
					if ( pLInfNext->m_aryText.GetAt(i) != NULL )
					{
						int	nCharWidth = pLInfNext->m_aryText[i].m_nWidth ;
						if ( nWidth + nCharWidth > nLimitWidth )
						{
							break ;
						}
						nWidth += nCharWidth ;
						if ( pLInfNext->m_wstrLine.GetAt(i) == L'\n' )
						{
							i += 1 ;
							break ;
						}
					}
				}
				nMergeLen = i ;
				//
				if ( nMergeLen > 0 )
				{
					fFullRedraw = true ;
					pLInf->m_nWidth = nWidth ;
					pLInf->m_wstrLine +=
						pLInfNext->m_wstrLine.Left( nMergeLen ) ;
					pLInfNext->m_wstrLine =
						pLInfNext->m_wstrLine.Middle( nMergeLen ) ;
					pLInf->m_aryText.Merge
						( pLInf->m_aryText.GetSize(),
							pLInfNext->m_aryText, 0, nMergeLen ) ;
//					pLInfNext->m_aryText.RemoveBetween( 0, nMergeLen ) ;
					//
					if ( pLInfNext->m_wstrLine.IsEmpty() )
					{
						m_lstLine.RemoveAt( iLine -- ) ;
						nDelLines ++ ;
						DrawViewLine( iLine + 1, true ) ;
						continue ;
					}
				}
				else
				{
					fLineWidth = fLineEdit ;
				}
			}
			else
			{
				fLineWidth = fLineEdit ;
			}
		}
		//
		// 文字指標更新
		//
		pLInfNext->m_nIndex =
			pLInf->m_nIndex + pLInf->m_wstrLine.GetLength() ;
		pLInf = pLInfNext ;
		//
		// 再描画
		//
		if ( (fRedrawLine || fFullRedraw) && fRedraw )
		{
			DrawViewLine( iLine - 1, true ) ;
			DrawViewLine( iLine, true ) ;
		}
	}
	/*
	if ( fRedraw )
	{
		while ( nDelLines -- > 0 )
		{
			DrawViewLine( iLine ++, true ) ;
		}
	}
	*/
	//
	// 行の最大幅の再計算
	//
	if ( !fChgMaxLineWidth && fSearchMaxLineWidth )
	{
		unsigned int	nWidth = 0 ;//m_isTextPanel.GetWidth() ;
		for ( int i = 0; i < (int) m_lstLine.GetSize(); i ++ )
		{
			pLInf = GetLineAt( i ) ;
			if ( pLInf != NULL )
			{
				if ( pLInf->m_nWidth > (int) nWidth )
				{
					nWidth = pLInf->m_nWidth ;
				}
			}
		}
		if ( nWidth != m_nMaxLineWidth )
		{
			m_nMaxLineWidth = nWidth ;
			fChgMaxLineWidth = true ;
		}
	}
	if ( fChgMaxLineWidth
		|| (nOrgLineCount != (int) m_lstLine.GetSize()) )
	{
		OnScrollSize( ) ;
	}
}

// 指定の文字列を検索する
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::FindTextUsage
	( const wchar_t * pwszMatchUsage,
		int iFirst, int nFindDir,
		int * pFindEnd, EStreamWideString * pswsRule )
{
	if ( m_pEditServer != NULL )
	{
		return	m_pEditServer->FindTextUsage
			( pwszMatchUsage, iFirst, nFindDir, pFindEnd, pswsRule ) ;
	}
	EStreamWideString::EUsage	lstUsage ;
	int					iLine = GetLineFromIndex( iFirst ) ;
	const ELineInf *	pLInf = GetLineAt( iLine ) ;
	EStreamWideString	swsUsage = pwszMatchUsage ;
	EStreamWideString	swsRule ;
	EString				strErrMsg ;
	if ( pswsRule == NULL )
	{
		pswsRule = &swsRule ;
	}
	if ( pswsRule->ParseUsage( lstUsage, swsUsage, strErrMsg ) )
	{
		return	-1 ;
	}
	if ( pLInf != NULL )
	{
		iFirst -= pLInf->m_nIndex ;
	}
	else
	{
		iFirst = 0 ;
	}
	if ( nFindDir >= 0 )
	{
		nFindDir = 1 ;
	}
	else
	{
		nFindDir = -1 ;
	}
	for ( ; ; )
	{
		//
		// 行の開始位置を取得
		//
		while ( -- iLine >= 0 )
		{
			pLInf = GetLineAt( iLine ) ;
			if ( pLInf != NULL )
			{
				if ( pLInf->m_wstrLine.Find( L'\n' ) >= 0 )
				{
					break ;
				}
				iFirst += pLInf->m_wstrLine.GetLength() ;
			}
		}
		iLine ++ ;
		//
		// 1 行取得
		//
		int					iLineBase = -1 ;
		int					iNextLine = iLine + nFindDir ;
		pswsRule->FreeString() ;
		pswsRule->MoveIndex( 0 ) ;
		while ( iLine < (int) m_lstLine.GetSize() )
		{
			pLInf = GetLineAt( iLine ++ ) ;
			if ( nFindDir > 0 )
			{
				iNextLine = iLine ;
			}
			if ( pLInf != NULL )
			{
				if ( iLineBase == -1 )
				{
					iLineBase = pLInf->m_nIndex ;
				}
				*pswsRule += pLInf->m_wstrLine ;
				if ( pLInf->m_wstrLine.Find( L'\n' ) >= 0 )
				{
					break ;
				}
			}
		}
		//
		// 書式の一致判定
		//
		int		iLastFindFirst = -1, iLastFindEnd = -1 ;
		int		iFindNext ;
		if ( nFindDir >= 0 )
		{
			pswsRule->MoveIndex( iFirst ) ;
		}
		iFindNext = pswsRule->FindMatchUsageList
			( lstUsage, 0, EStreamWideString::utCharacters ) ;
		if ( iFindNext >= 0 )
		{
			for ( ; ; )
			{
				//
				// 書式の完全一致判定
				//
				pswsRule->MoveIndex( iFindNext ) ;
				if ( !pswsRule->IsMatchUsageList( lstUsage, 0, strErrMsg ) )
				{
					if ( nFindDir >= 0 )
					{
						iLastFindFirst = iFindNext ;
						iLastFindEnd = pswsRule->GetIndex( ) ;
						break ;
					}
					else if ( iFindNext < iFirst )
					{
						iLastFindFirst = iFindNext ;
						iLastFindEnd = pswsRule->GetIndex( ) ;
					}
					else
					{
						break ;
					}
				}
				//
				// 次の書式を探す
				//
				iFindNext =
					pswsRule->FindMatchUsageList
						( lstUsage, 0, EStreamWideString::utCharacters ) ;
				if ( iFindNext < 0 )			// 先頭部分一致
				{
					break ;
				}
			}
			if ( iLastFindFirst >= 0 )
			{
				if ( pFindEnd != NULL )
				{
					*pFindEnd = iLineBase + iLastFindEnd ;
					return	iLineBase + iLastFindFirst ;
				}
			}
		}
		//
		// 次の行へ移動
		//
		if ( nFindDir >= 0 )
		{
			iLine = iNextLine ;
			iFirst = 0 ;
			if ( iLine >= GetLineCount() )
			{
				break ;
			}
		}
		else
		{
			iLine = iNextLine ;
			iFirst = 0x70000000 ;
			if ( iLine < 0 )
			{
				break ;
			}
		}
	}
	return	-1 ;
}

// （ローカル）座標から文字指標へ変換
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetCharIndexFromPos( int xPos, int yPos )
{
	//
	// 座標を正規化
	//
	EGL_RECT	rcText = m_isTextPanel.GetRectangle( ) ;
	xPos -= rcText.left ;
	yPos -= rcText.top ;
	rcText.right -= rcText.left ;
	rcText.bottom -= rcText.top ;
	//
/*	if ( xPos < 0 )
		xPos = 0 ;
	else if ( xPos > rcText.right )
		xPos = rcText.right ;
	//
	if ( yPos < 0 )
		yPos = 0 ;
	else if ( yPos > rcText.bottom )
		yPos = rcText.bottom ;*/
	//
	// 文字指標取得
	//
	return	GetIndexFromLinePos
		( yPos / (int) m_rfiText.GetLineHeight() + m_yScroll, xPos ) ;
}

// ｘ座標から指定行の文字指標を取得
//////////////////////////////////////////////////////////////////////////////
int ETextEditSprite::GetIndexFromLinePos( int nLine, int xPos )
{
	//
	// 行を取得
	//
	const ELineInf *	pLInf ;
	if ( m_etType == etSingleLine )
	{
		pLInf = GetLineAt( 0 ) ;
	}
	else
	{
		pLInf = GetLineAt( nLine ) ;
		if ( pLInf == NULL )
		{
			if ( nLine < 0 )
			{
				return	0 ;
			}
			else
			{
				pLInf = m_lstLine.GetLastAt( ) ;
			}
		}
	}
	if ( pLInf == NULL )
	{
		return	0 ;
	}
	//
	// 文字を検索
	//
	xPos += m_xScroll ;
	for ( int i = 0; i < (int) pLInf->m_aryText.GetSize(); i ++ )
	{
		ECharacter *	pChar = pLInf->m_aryText.GetAt( i ) ;
		if ( pChar != NULL )
		{
			if ( pLInf->m_wstrLine.GetAt(i) == L'\n' )
			{
				return	pLInf->m_nIndex + i ;
			}
			if ( xPos < pChar->m_nWidth )
			{
				if ( xPos <= pChar->m_nWidth / 2 + 1 )
					return	pLInf->m_nIndex + i ;
				else
					return	pLInf->m_nIndex + i + 1 ;
			}
			xPos -= pChar->m_nWidth ;
		}
	}
	//
	return	pLInf->m_nIndex + pLInf->m_wstrLine.GetLength( ) ;
}

// 編集域のカーソル識別子を設定
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::SetCursorOnEdit( const wchar_t * pwszCursorID )
{
	m_wstrCursor = pwszCursorID ;
}

// クライアント関連付け
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::AttachClient( ETextEditSprite * pClient )
{
	if ( pClient->m_pEditServer != NULL )
	{
		pClient->m_pEditServer->DetachClient( pClient ) ;
	}
	pClient->m_pEditServer = this ;
	m_lstEditClient.Add( pClient ) ;
}

// クライアント分離
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::DetachClient( ETextEditSprite * pClient )
{
	pClient->m_pEditServer = NULL ;
	for ( int i = 0; i < (int) m_lstEditClient.GetSize(); i ++ )
	{
		if ( m_lstEditClient.GetAt(i) == pClient )
		{
			m_lstEditClient.RemoveAt( i -- ) ;
		}
	}
}

// 行情報の更新通知
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::OnUpdateLineInfo( ELineInf * pLInf )
{
}

// スクロール情報通知
//////////////////////////////////////////////////////////////////////////////
void ETextEditSprite::OnScrollPos( void )
{
}

void ETextEditSprite::OnScrollSize( void )
{
	unsigned int	i, nCount ;
	nCount = m_lstEditClient.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETextEditSprite *	pClient = m_lstEditClient.GetAt( i ) ;
		if ( pClient != NULL )
		{
			pClient->OnScrollSize( ) ;
		}
	}
}
