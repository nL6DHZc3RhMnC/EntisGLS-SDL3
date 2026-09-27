
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2004 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// リストビュー・スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EListViewSprite, ESpriteInterface )
IMPLEMENT_CLASS_INFO( EListViewSprite::ELine, ESLObject )

// 比較
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::ELine::Compare
	( EListViewSprite & list, const ELine & line, int col ) const
{
	EWideString *	pwstrText1 = m_lstText.GetAt( col ) ;
	EWideString *	pwstrText2 = line.m_lstText.GetAt( col ) ;
	if ( pwstrText1 == NULL )
	{
		if ( pwstrText2 == NULL )
		{
			return	0 ;
		}
		return	-1 ;
	}
	if ( pwstrText2 == NULL )
	{
		return	1 ;
	}
	return	pwstrText1->Compare( *pwstrText2 ) ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::ELine::Draw
	( EListViewSprite & list,
		HEGL_RENDER_POLYGON hRender, int nLineNum, int yPos )
{
	int	nLineHeight = m_nLineHeight ;
	if ( nLineHeight == 0 )
	{
		nLineHeight = list.m_tsStyle.nLineHeight ;
	}
	HEGL_DRAW_IMAGE	hDraw = hRender->GetDrawImage( ) ;
	EGL_RECT	rectLine = list.GetLineRect( nLineNum ) ;
	if ( !hDraw->PrepareFillRect
		( &rectLine, list.m_tsStyle.lsLine[m_iStatus].rgbaBackColor, 0, 0 ) )
	{
		hDraw->FillRegion( ) ;
	}
	//
	for ( unsigned int i = 0; i < m_lstText.GetSize(); i ++ )
	{
		int	xOffset = 0 ;
		if ( i == 0 )
		{
			xOffset = 8 ;
			if ( m_iImage >= 0 )
			{
				EGL_DRAW_PARAM	dp ;
				::eslFillMemory( &dp, 0, sizeof(dp) ) ;
				dp.pSrcImage = list.ImageList().GetAt( i ) ;
				if ( dp.pSrcImage != NULL )
				{
					xOffset += dp.pSrcImage->dwImageWidth + 8 ;
					dp.ptBasePos.x = 8 - list.GetScrollPos().x ;
					if ( !hDraw->PrepareDraw( &dp ) )
					{
						hDraw->DrawImage( ) ;
					}
				}
			}
		}
		EWideString *	pwstrText = m_lstText.GetAt( i ) ;
		if ( pwstrText == NULL )
		{
			continue ;
		}
		list.DrawLineText
			( hRender, m_iStatus, i, xOffset, yPos, *pwstrText ) ;
	}
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EListViewSprite::EListViewSprite( void )
{
	m_dwFlags |= ffTabStop ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EListViewSprite::~EListViewSprite( void )
{
}

// 領域再描画
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::RefreshRect( const EGL_RECT & rectRefresh )
{
	ESpriteInterface::RefreshRect( rectRefresh ) ;
	//
	if ( GetInfo() == NULL )
	{
		return ;
	}
	if ( m_hRenderPoly == NULL )
	{
		m_hRenderPoly = ::eglCreateRenderPolygon( ) ;
	}
	int	yPos = 0 ;
	for ( unsigned int i = m_ptScroll.y; i < m_lstLine.GetSize(); i ++ )
	{
		ELine *	pLine = m_lstLine.GetAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		DrawLine( m_hRenderPoly, *pLine, i, yPos ) ;
		if ( pLine->m_nLineHeight == 0 )
		{
			yPos += m_tsStyle.nLineHeight ;
		}
		else
		{
			yPos += pLine->m_nLineHeight ;
		}
		if ( yPos >= (int) GetInfo()->dwImageHeight )
		{
			break ;
		}
	}
}

// 行描画関数
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::DrawLine
	( HEGL_RENDER_POLYGON hRender, ELine & line, int nLineNum, int yPos )
{
	line.Draw( *this, hRender, nLineNum, yPos ) ;
}

// 比較
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::CompareLine
	( const ELine & line1, const ELine & line2, int col )
{
	return	line1.Compare( *this, line2, col ) ;
}

// 文字列描画
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::DrawLineText
( HEGL_RENDER_POLYGON hRender,
	int nStatus, int nCol, int xOffset,
		int yPos, const wchar_t * pwszText )
{
	//
	// パラメータの検証
	//
	if ( (nStatus < 0) || (nStatus >= lsMax) )
	{
		return ;
	}
	if ( (nCol < 0) || (nCol >= (int) m_lstColWidth.GetSize()) )
	{
		return ;
	}
	if ( (pwszText == NULL) || (pwszText[0] == L'\0') )
	{
		return ;
	}
	//
	// 描画幅計算
	//
	int	nWidth = m_lstColWidth[nCol] - xOffset ;
	if ( nWidth <= 0 )
	{
		return ;
	}
	//
	// 描画オブジェクト初期化
	//
	ERealFontImage	rfi ;
	rfi.SetFont( m_fontText[nStatus] ) ;
	rfi.SetViewRect( EGLRect( 0, 0, 0xFFFF, 0xFFFF ) ) ;
	rfi.SetColor( m_tsStyle.lsLine[nStatus].tsText.rgbColor ) ;
	rfi.SetLineHeight( m_tsStyle.lsLine[nStatus].tsText.nLineHeight ) ;
	//
	// 描画文字数決定
	//
	EWideString	wstrText = pwszText ;
	int	iLine = wstrText.Find( L'\n' ) ;
	if ( iLine >= 0 )
	{
		wstrText = wstrText.Left( iLine ) ;
	}
	int	nTextWidth = rfi.GetTextWidth( wstrText ) ;
	if ( nTextWidth > nWidth )
	{
		int	iFirst = 0, iEnd = wstrText.GetLength() - 1 ;
		for ( ; ; )
		{
			int	iMiddle = (iFirst + iEnd) / 2 ;
			nTextWidth = rfi.GetTextWidth( wstrText.Left( iMiddle ) ) ;
			if ( nTextWidth <= nWidth )
			{
				iFirst = iMiddle ;
				if ( iFirst + 1 >= iEnd )
				{
					break ;
				}
			}
			else
			{
				iEnd = iMiddle ;
				if ( iEnd <= iFirst )
				{
					break ;
				}
			}
		}
		wstrText = wstrText.Left( iFirst ) ;
	}
	//
	// 描画
	//
	rfi.MoveCursorPos( EGLPoint
		( GetColumnLeftPos(nCol) + xOffset - GetScrollPos().x, yPos ) ) ;
	rfi.DrawText( wstrText ) ;
	//
	EStaticTextSprite::DrawFontImage
		( hRender->GetDrawImage(),
			rfi, m_tsStyle.lsLine[nStatus].tsText ) ;
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	EString	strFont = pwszFont ;
	for ( int i = 0; i < lsMax; i ++ )
	{
		m_fontText[i].SetFaceName( strFont ) ;
		m_fontText[i].Create( ) ;
	}
	UpdateRect( NULL ) ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool EListViewSprite::IsHitSprite( int xPos, int yPos )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo() ;
	if ( pInfo == NULL )
	{
		return	false ;
	}
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	if ( (ptLocal.x < 0) || (ptLocal.y < 0)
		|| ((DWORD) ptLocal.x >= pInfo->dwImageWidth)
		|| ((DWORD) ptLocal.y >= pInfo->dwImageHeight) )
	{
		return	false ;
	}
	return	true ;
}

// マウスが移動した
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	int	nLine = GetLineFromPos( ptLocal.y ) ;
	if ( nLine != m_iFocusLine )
	{
		ELine *	pLine = GetLineAt( m_iFocusLine ) ;
		if ( pLine != NULL )
		{
			pLine->m_iStatus &= ~lsFocus ;
			UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
		}
		pLine = GetLineAt( nLine ) ;
		if ( pLine != NULL )
		{
			pLine->m_iStatus |= lsFocus ;
			UpdateRect( &GetLineRect( nLine ) ) ;
		}
		m_iFocusLine = nLine ;
	}
}

// マウスが領域外に移動した
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
	ELine *	pLine = GetLineAt( m_iFocusLine ) ;
	if ( pLine != NULL )
	{
		pLine->m_iStatus &= ~lsFocus ;
		UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
	}
}

// マウスホイールが回転した
//////////////////////////////////////////////////////////////////////////////
bool EListViewSprite::OnMouseWheel
	( UINT nFlags, short int zDelta, int xPos, int yPos )
{
	SetVertScrollPos( GetVertScrollPos() + zDelta / WHEEL_DELTA ) ;
	return	true ;
}

// 左ボタンがクリックされた
//////////////////////////////////////////////////////////////////////////////
bool EListViewSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	int	nLine = GetLineFromPos( ptLocal.y ) ;
	//
	// 以前のフォーカスキャンセル
	//
	ELine *	pLine = GetLineAt( m_iFocusLine ) ;
	if ( pLine != NULL )
	{
		pLine->m_iStatus &= ~lsFocus ;
		UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
	}
	//
	// 選択の変更
	//
	pLine = GetLineAt( nLine ) ;
	if ( pLine != NULL )
	{
		pLine->m_iStatus |= lsFocus ;
		if ( m_tsStyle.nTypeFlags & ltMultiSelect )
		{
			if ( nFlags & MK_SHIFT )
			{
				//
				// 範囲選択
				//
				if ( m_iLastSelLine >= 0 )
				{
					int	iFirst = m_iLastSelLine ;
					int	iEnd = nLine ;
					if ( iFirst > iEnd )
					{
						iFirst = nLine ;
						iEnd = m_iLastSelLine ;
					}
					for ( int i = iFirst; i <= iEnd; i ++ )
					{
						ELine *	pOther = GetLineAt( i ) ;
						if ( pOther == NULL )
						{
							continue ;
						}
						pOther->m_iStatus |= lsPushed ;
						UpdateRect( &GetLineRect( i ) ) ;
					}
					OnCommand( this, ncSelChanged, nLine ) ;
				}
				else
				{
					SelectLine( nLine ) ;
				}
			}
			else if ( nFlags & MK_CONTROL )
			{
				//
				// 選択反転
				//
				pLine->m_iStatus ^= lsPushed ;
				OnCommand( this, ncSelChanged, nLine ) ;
			}
			else
			{
				SelectLine( nLine ) ;
			}
		}
		else
		{
			SelectLine( nLine ) ;
		}
		UpdateRect( &GetLineRect( nLine ) ) ;
		//
		// 通知コマンド発行
		//
		OnCommand( this, ncClickLine, nLine ) ;
	}
	else
	{
		SelectLine( nLine ) ;
	}
	m_iLastSelLine = nLine ;
	m_iFocusLine = nLine ;
	//
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		pParent->SetFocus( this ) ;
	}
	return	true ;
}

// ダブルクリックされた
//////////////////////////////////////////////////////////////////////////////
bool EListViewSprite::OnLButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	int	nLine = GetLineFromPos( ptLocal.y ) ;
	if ( nLine >= 0 )
	{
		OnCommand( this, ncDblClickLine, nLine ) ;
	}
	return	true ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
bool EListViewSprite::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
	case	WM_KEYDOWN:
		if ( (wParam == VK_UP) || (wParam == VK_DOWN) )
		{
			//
			// フォーカス移動
			//
			ELine *	pLine ;
			ELine *	pNext ;
			int		iFocus ;
			pLine = GetLineAt( m_iFocusLine ) ;
			if ( pLine == NULL )
			{
				if ( wParam == VK_DOWN )
				{
					iFocus = 0 ;
				}
				else
				{
					iFocus = GetLineCount() - 1 ;
				}
			}
			else
			{
				if ( wParam == VK_DOWN )
				{
					iFocus = m_iFocusLine + 1 ;
				}
				else
				{
					iFocus = m_iFocusLine - 1 ;
				}
			}
			pNext = GetLineAt( iFocus ) ;
			if ( pNext != NULL )
			{
				if ( pLine != NULL )
				{
					pLine->m_iStatus &= ~lsFocus ;
					UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
				}
				pNext->m_iStatus |= lsFocus ;
				m_iFocusLine = iFocus ;
				UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
				ScrollToViewLine( m_iFocusLine ) ;
			}
		}
		else if ( wParam == VK_SPACE )
		{
			//
			// フォーカスのあるラインを選択
			//
			ELine *	pLine = GetLineAt( m_iFocusLine ) ;
			if ( pLine != NULL )
			{
				SelectLine( m_iFocusLine ) ;
			}
		}
		return	true ;
	}
	return	ESpriteInterface::MessageProc( hWnd, uMsg, wParam, lParam ) ;
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EListViewSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// フォーカスを取得した
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::OnSetFocus( void )
{
	ELine *	pLine = GetLineAt( m_iFocusLine ) ;
	if ( pLine != NULL )
	{
		pLine->m_iStatus |= lsFocus ;
		UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
	}
}

// フォーカスを奪われた
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::OnKillFocus( void )
{
	ELine *	pLine = GetLineAt( m_iFocusLine ) ;
	if ( pLine != NULL )
	{
		pLine->m_iStatus &= ~lsFocus ;
		UpdateRect( &GetLineRect( m_iFocusLine ) ) ;
	}
}

// ローカル座標から該当するラインを取得する
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetLineFromPos( int yPos )
{
	for ( int i = GetScrollPos().y; i < GetLineCount(); i ++ )
	{
		int	nHeight = GetLineHeight( i ) ;
		if ( yPos < nHeight )
		{
			return	i ;
		}
		yPos -= nHeight ;
	}
	return	-1 ;
}

// 指定された行を囲む矩形を取得する
//////////////////////////////////////////////////////////////////////////////
EGL_RECT EListViewSprite::GetLineRect( int nLine )
{
	EGLRect	rect( 0, 0, -1, -1 ) ;
	PEGL_IMAGE_INFO	pImage = GetInfo() ;
	if ( (pImage == NULL)
		|| (nLine < GetScrollPos().y)
		|| (nLine >= GetLineCount()) )
	{
		return	rect ;
	}
	int		yPos = 0 ;
	for ( int i = GetScrollPos().y; i < nLine; i ++ )
	{
		int	nHeight = GetLineHeight( i ) ;
		yPos += nHeight ;
		if ( yPos >= (int) pImage->dwImageHeight )
		{
			return	rect ;
		}
	}
	rect.left = 0 ;
	rect.right = pImage->dwImageWidth - 1 ;
	rect.top = yPos ;
	rect.bottom = yPos + GetLineHeight( nLine ) ;
	return	rect ;
}

// 指定された行の高さを取得する
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetLineHeight( int nLine )
{
	ELine *	pLine = GetLineAt( nLine ) ;
	if ( pLine == NULL )
	{
		return	0 ;
	}
	int	nHeight = m_tsStyle.nLineHeight ;
	if ( pLine->m_nLineHeight > 0 )
	{
		nHeight = pLine->m_nLineHeight ;
	}
	return	nHeight ;
}

// 指定の行を選択する
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SelectLine( int nLine )
{
	bool	fCommand = false ;
	for ( int i = 0; i < GetLineCount(); i ++ )
	{
		ELine *	pLine = GetLineAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		if ( i == nLine )
		{
			if ( !(pLine->m_iStatus & lsPushed) )
			{
				pLine->m_iStatus |= lsPushed ;
				fCommand = true ;
			}
		}
		else
		{
			if ( pLine->m_iStatus & lsPushed )
			{
				pLine->m_iStatus &= ~lsPushed ;
				fCommand = true ;
			}
		}
	}
	if ( fCommand )
	{
		UpdateRect( NULL ) ;
		OnCommand( this, ncSelChanged, nLine ) ;
	}
}

// 指定の行が表示されるようにスクロール
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::ScrollToViewLine( int nLine )
{
	//
	// 表示領域を取得する
	//
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( pInfo == NULL )
	{
		return ;
	}
	int	nVisibleLines =
		(int) pInfo->dwImageHeight / m_tsStyle.nLineHeight ;
	//
	// スクロール判定
	//
	if ( m_ptScroll.y > nLine )
	{
		SetVertScrollPos( nLine ) ;
	}
	else if ( m_ptScroll.y + nVisibleLines <= nLine )
	{
		SetVertScrollPos( nLine - nVisibleLines + 1 ) ;
	}
}

// 現在の垂直スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetVertScrollPos( void ) const
{
	return	m_ptScroll.y ;
}

// 現在の垂直スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetVertScrollPos( int nPos )
{
	if ( nPos >= GetLineCount() )
	{
		nPos = GetLineCount() - 1 ;
	}
	if ( nPos < 0 )
	{
		nPos = 0 ;
	}
	m_ptScroll.y = nPos ;
	UpdateRect( NULL ) ;
}

// 垂直スクロールの範囲を取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetVertScrollRange( void ) const
{
	return	GetLineCount() ;
}

// 垂直スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetVertScrollRange( int nRange )
{
}

// 現在の水平スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetHorzScrollPos( void ) const
{
	return	m_ptScroll.x ;
}

// 現在の水平スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetHorzScrollPos( int nPos )
{
	if ( nPos < 0 )
	{
		nPos = 0 ;
	}
	else if ( nPos >= GetHorzScrollRange() )
	{
		nPos = GetHorzScrollRange() ;
	}
	m_ptScroll.x = nPos ;
	UpdateRect( NULL ) ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetHorzScrollRange( void ) const
{
	PCEGL_IMAGE_INFO	pImage = *this ;
	if ( pImage == NULL )
	{
		return	0 ;
	}
	int	nRange = - (int) pImage->dwImageWidth ;
	for ( int i = 0; i < GetColumnCount(); i ++ )
	{
		nRange += GetColumnWidth( i ) ;
	}
	if ( nRange <= 0 )
	{
		return	0 ;
	}
	return	nRange ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetHorzScrollRange( int nRange )
{
}

// リストオブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError EListViewSprite::CreateListView( const LIST_STYLE & style )
{
	//
	// スタイル設定
	//
	m_tsStyle = style ;
	//
	// 画像バッファ生成
	//
	if ( CreateImage
		( EIF_RGBA_BITMAP,
			m_tsStyle.sizeExt.w, m_tsStyle.sizeExt.h, 32 ) == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// フォント設定
	//
	for ( int i = 0; i < lsMax; i ++ )
	{
		m_fontText[i].LogFont() = m_tsStyle.lsLine[i].tsText.lfFont ;
		m_fontText[i].Create( ) ;
	}
	//
	// パラメータ初期化
	//
	m_lstColWidth.RemoveAll( ) ;
	m_lstColWidth.SetAt( 0, m_tsStyle.sizeExt.w ) ;
	//
	m_ptScroll.x = 0 ;
	m_ptScroll.y = 0 ;
	m_iFocusLine = -1 ;
	m_iLastSelLine = -1 ;
	//
	return	eslErrSuccess ;
}

// カラム幅設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetColumnWidth( int iIndex, int nWidth )
{
	m_lstColWidth.SetAt( iIndex, nWidth ) ;
}

// カラムｘ座標取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetColumnLeftPos( int iIndex ) const
{
	int	xPos = 0 ;
	for ( int i = 0; i < iIndex; i ++ )
	{
		xPos += m_lstColWidth.GetAt( i ) ;
	}
	return	xPos ;
}

// 文字列追加
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::InsertItem
	( const wchar_t * pwszText,
		int iIndex, int iSubIndex, int iImage )
{
	ELine *	pLine = GetLineAt( iIndex ) ;
	if ( pLine == NULL )
	{
		pLine = new ELine ;
		m_lstLine.InsertAt( iIndex, pLine ) ;
	}
	if ( iImage >= 0 )
	{
		pLine->m_iImage = iImage ;
	}
	pLine->m_lstText.InsertAt( iSubIndex, new EWideString( pwszText ) ) ;
	return	iIndex ;
}

// 行追加
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::InsertLine( int iIndex, ELine * pLine )
{
	m_lstLine.InsertAt( iIndex, pLine ) ;
	return	iIndex ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * EListViewSprite::GetItemText( int iIndex, int iColIndex ) const
{
	ELine *	pLine = GetLineAt( iIndex ) ;
	if ( pLine == NULL )
	{
		return	NULL ;
	}
	EWideString *	pwstrText = pLine->m_lstText.GetAt( iColIndex ) ;
	if ( pwstrText == NULL )
	{
		return	NULL ;
	}
	return	*pwstrText ;
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SetItemText
	( int iIndex, int iColIndex, const wchar_t * pwszText )
{
	ELine *	pLine = GetLineAt( iIndex ) ;
	if ( pLine == NULL )
	{
		return ;
	}
	pLine->m_lstText.SetAt( iColIndex, new EWideString( pwszText ) ) ;
}

// 選択されているアイテムの総数を取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::GetSelectedItemCount( void ) const
{
	int	nCount = 0 ;
	int	iFind = FindSelectedItem( 0 ) ;
	while ( iFind >= 0 )
	{
		nCount ++ ;
		iFind = FindSelectedItem( iFind + 1 ) ;
	}
	return	nCount ;
}

// 選択アイテム取得
//////////////////////////////////////////////////////////////////////////////
int EListViewSprite::FindSelectedItem( int iFirst ) const
{
	for ( int i = iFirst; i < GetLineCount(); i ++ )
	{
		ELine *	pLine = GetLineAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		if ( pLine->m_iStatus & lsPushed )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// ソート
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::SortList( int iSortType, int iSubIndex )
{
	int	nCount = GetLineCount( ) ;
	iSortType = (iSortType >= 0) ? 0 : -1 ;
	for ( int i = 0; i < nCount - 1; i ++ )
	{
		ELine *	pLine1 = GetLineAt( i ) ;
		if ( pLine1 == NULL )
		{
			continue ;
		}
		for ( int j = i + 1; j < nCount; j ++ )
		{
			ELine *	pLine2 = GetLineAt( j ) ;
			if ( pLine2 == NULL )
			{
				continue ;
			}
			int	nCompare = CompareLine( *pLine1, *pLine2, iSubIndex ) ;
			nCompare = (nCompare ^ iSortType) - iSortType ;
			if ( nCompare < 0 )
			{
				m_lstLine.Swap( i, j ) ;
				pLine1 = pLine2 ;
			}
		}
	}
}

// 画像リスト追加
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::InsertImage( int iIndex, PEGL_IMAGE_INFO pImage )
{
	m_lstImage.InsertAt( iIndex, pImage ) ;
}

// 画像リスト削除
//////////////////////////////////////////////////////////////////////////////
void EListViewSprite::ClearImageList( void )
{
	m_lstImage.RemoveAll( ) ;
}


//////////////////////////////////////////////////////////////////////////////
// コンボリスト・スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EComboListSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EComboListSprite::EComboListSprite( void )
{
	m_dwFlags |= ffTabStop ;
	m_pEdit = NULL ;
	m_pButton = NULL ;
	m_pFrame = NULL ;
	m_pList = NULL ;
	m_pScroll = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EComboListSprite::~EComboListSprite( void )
{
}

// コンボリストを作成
//////////////////////////////////////////////////////////////////////////////
ESLError EComboListSprite::CreateCombo( const COMBO_STYLE & style )
{
	RemoveAllSprite( ) ;
	//
	// スタイル複製
	//
	if ( !(style.nFlags & sfEditCtrl) )
	{
		return	eslErrGeneral ;
	}
	m_csStyle = style ;
	//
	// 画像バッファ生成
	//
	CreateImage
		( EIF_RGBA_BITMAP,
			m_csStyle.sizeExt.w, m_csStyle.sizeExt.h, 32 ) ;
	//
	// エディットボックス配置計算
	//
	PEGL_IMAGE_INFO	pInfo ;
	ESLAssert( m_csStyle.nFlags & sfEditCtrl ) ;
	m_csStyle.esEdit.sizeExt.w = m_csStyle.sizeExt.w ;
	m_csStyle.esEdit.sizeExt.h = 0 ;
	if ( m_csStyle.esEdit.pTextWay != NULL )
	{
		m_csStyle.esEdit.sizeExt.h =
			m_csStyle.esEdit.pTextWay->dwImageHeight ;
	}
	m_csStyle.bsButton.sizeExt.w = 0 ;
	m_csStyle.bsButton.sizeExt.h = 0 ;
	if ( m_csStyle.nFlags & sfDropDownBtn )
	{
		pInfo = m_csStyle.bsButton.pImage[EButtonSprite::bsNormal] ;
		if ( pInfo != NULL )
		{
			m_csStyle.bsButton.sizeExt.w = pInfo->dwImageWidth ;
			m_csStyle.bsButton.sizeExt.h = pInfo->dwImageHeight ;
		}
	}
	m_csStyle.esEdit.sizeExt.w -= m_csStyle.bsButton.sizeExt.w ;
	int	nEditLineHeight = m_csStyle.esEdit.sizeExt.h ;
	if ( nEditLineHeight > m_csStyle.bsButton.sizeExt.h )
	{
		nEditLineHeight = m_csStyle.bsButton.sizeExt.h ;
	}
	//
	// エディットボックス生成
	//
	m_pEdit = new ETextEditSprite ;
	AddSprite( 0, m_pEdit ) ;
	if ( m_pEdit->CreateEdit( m_csStyle.esEdit ) )
	{
		return	eslErrGeneral ;
	}
	m_pEdit->SetVisible( true ) ;
	//
	m_pButton = NULL ;
	if ( m_csStyle.nFlags & sfDropDownBtn )
	{
		m_pButton = new EButtonSprite ;
		AddSprite( 0, m_pButton ) ;
		if ( m_pButton->CreateButton( m_csStyle.bsButton ) )
		{
			return	eslErrGeneral ;
		}
		m_pButton->MovePosition( EGLPoint( m_csStyle.esEdit.sizeExt.w, 0 ) ) ;
		m_pButton->SetVisible( true ) ;
	}
	//
	// リストビュー配置計算
	//
	m_pFrame = NULL ;
	m_pList = NULL ;
	m_pScroll = NULL ;
	if ( (nEditLineHeight < m_csStyle.sizeExt.h)
					&& (m_csStyle.nFlags & sfListView) )
	{
		m_csStyle.lsList.sizeExt.w = m_csStyle.sizeExt.w ;
		m_csStyle.lsList.sizeExt.h = m_csStyle.sizeExt.h - nEditLineHeight ;
		m_csStyle.bsScroll.sizeBarExt.w = 0 ;
		m_csStyle.bsScroll.sizeBarExt.h = m_csStyle.lsList.sizeExt.h ;
		if ( m_csStyle.nFlags & sfListScroll )
		{
			pInfo = m_csStyle.bsScroll.bsPrevButton.
							pImage[EButtonSprite::bsNormal] ;
			if ( pInfo != NULL )
			{
				m_csStyle.bsScroll.sizeBarExt.w = pInfo->dwImageWidth ;
			}
		}
		m_csStyle.lsList.sizeExt.w -= m_csStyle.bsScroll.sizeBarExt.w ;
		//
		// リストビュー生成
		//
		m_pList = new EListViewSprite ;
		AddSprite( 0, m_pList ) ;
		if ( m_pList->CreateListView( m_csStyle.lsList ) )
		{
			return	eslErrSuccess ;
		}
		m_pList->MovePosition( EGLPoint( 0, nEditLineHeight ) ) ;
		//
		if ( m_csStyle.nFlags & sfListFrame )
		{
			m_pFrame = new EStaticFrameSprite ;
			AddSprite( 1, m_pFrame ) ;
			if ( m_pFrame->CreateStaticFrame
				( m_csStyle.fsFrame,
					m_csStyle.lsList.sizeExt.w, m_csStyle.lsList.sizeExt.h ) )
			{
				return	eslErrSuccess ;
			}
			m_pFrame->MovePosition( EGLPoint( 0, nEditLineHeight ) ) ;
		}
		if ( m_csStyle.nFlags & sfListScroll )
		{
			m_pScroll = new EScrollBarSprite ;
			AddSprite( 0, m_pScroll ) ;
			if ( m_pScroll->CreateScrollBar( m_csStyle.bsScroll ) )
			{
				return	eslErrSuccess ;
			}
			m_pScroll->MovePosition
				( EGLPoint( m_csStyle.lsList.sizeExt.w, nEditLineHeight ) ) ;
		}
	}
	//
	// コンボスタイル適用
	//
	m_fDropList = false ;
	if ( m_csStyle.nType == ctComboList )
	{
		SetDropListVisible( true ) ;
	}
	else if ( m_csStyle.nType == ctDropDownList )
	{
		ESLAssert( m_pEdit != NULL ) ;
		m_pEdit->Enable( false ) ;
	}
	//
	return	eslErrSuccess ;
}

// 文字列取得・設定
//////////////////////////////////////////////////////////////////////////////
const wchar_t * EComboListSprite::GetSpriteText( void )
{
	if ( m_pEdit == NULL )
	{
		return	NULL ;
	}
	return	m_pEdit->GetSpriteText( ) ;
}

void EComboListSprite::SetSpriteText( const wchar_t * pwszText )
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->SetSpriteText( pwszText ) ;
	}
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void EComboListSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->SetSpriteFontFace( pwszFont ) ;
	}
	if ( m_pList != NULL )
	{
		m_pList->SetSpriteFontFace( pwszFont ) ;
	}
}

// クリックされた
//////////////////////////////////////////////////////////////////////////////
bool EComboListSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	EGLPoint	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( GetSpriteAtPoint( ptLocal.x, ptLocal.y ) == m_pEdit )
	{
		if ( !m_pEdit->IsEnabled() )
		{
			SetDropListVisible( true ) ;
			return	true ;
		}
		else if ( m_csStyle.nType != ctComboList )
		{
			SetDropListVisible( false ) ;
		}
	}
	return	ESpriteInterface::OnLButtonDown( nFlags, xPos, yPos ) ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
void EComboListSprite::OnCommand
	( ESpriteInterface * pItem,
		long int nNotification, long int nParameter )
{
	if ( pItem == this )
	{
		ESpriteInterface::OnCommand( pItem, nNotification, nParameter ) ;
		return ;
	}
	if ( pItem == m_pEdit )
	{
		if ( nNotification == ETextEditSprite::ncChange )
		{
			ESpriteInterface::OnCommand( this, ncChange, nParameter ) ;
		}
	}
	else if ( pItem == m_pButton )
	{
		SetDropListVisible( !IsDropListVisible() ) ;
	}
	else if ( pItem == m_pList )
	{
		if ( nNotification == EListViewSprite::ncClickLine )
		{
			ESLAssert( m_pEdit != NULL ) ;
			ESLAssert( m_pList != NULL ) ;
			if ( m_pEdit && m_pList )
			{
				m_pEdit->SetSpriteText( m_pList->GetItemText( nParameter ) ) ;
			}
			if ( m_csStyle.nType != ctComboList )
			{
				SetDropListVisible( false ) ;
			}
		}
	}
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EComboListSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// フォーカスを奪われた
//////////////////////////////////////////////////////////////////////////////
void EComboListSprite::OnKillFocus( void )
{
	if ( m_csStyle.nType != ctComboList )
	{
		SetDropListVisible( false ) ;
	}
	ESpriteInterface::OnCommand( this, ncKillFocus, 0 ) ;
}

// リストを表示・非表示状態にする
//////////////////////////////////////////////////////////////////////////////
void EComboListSprite::SetDropListVisible( bool fVisible )
{
	if ( m_pFrame != NULL )
	{
		m_pFrame->SetVisible( fVisible ) ;
	}
	if ( m_pList != NULL )
	{
		m_pList->SetVisible( fVisible ) ;
	}
	if ( m_pScroll != NULL )
	{
		m_pScroll->SetVisible( fVisible ) ;
	}
	m_fDropList = fVisible ;
}

