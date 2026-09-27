
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2010 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// ボタンスプライト
//////////////////////////////////////////////////////////////////////////////

// デフォルトのリソース識別子
//////////////////////////////////////////////////////////////////////////////
const wchar_t *	EButtonSprite::m_pwszDefCursor = L"IDC_FOCUS" ;
const wchar_t *	EButtonSprite::m_pwszDefFocusSE = L"IDSE_BUTTON_FOCUS" ;
const wchar_t *	EButtonSprite::m_pwszDefPushedSE = L"IDSE_BUTTON_PUSHED" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EButtonSprite, EStaticTextSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EButtonSprite::EButtonSprite( void )
{
	for ( int i = 0; i < bsMax; i ++ )
		m_pImage[i] = NULL ;
	m_pHitTestMask = NULL ;
	//
	m_dwFlags |= ffTabStop ;
	m_style = btStatic ;
	m_status = bsNormal ;
	m_statusView = bsNormal ;
	m_fFocus = false ;
	m_fPushed = false ;
	m_fStatusNotification = false ;
	m_fEnableRightClick = false ;
	m_nBeforeRepInterval = -1 ;
	m_nRepeatInterval = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EButtonSprite::~EButtonSprite( void )
{
}

// 有効化・無効化
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::Enable( bool fEnable )
{
	EStaticTextSprite::Enable( fEnable ) ;
	if ( !fEnable || (m_statusView != bsActivePushed) )
	{
		m_statusView = m_status ;
	}
	SetButtonStatus( m_status ) ;
}

// 文字列取得・設定
//////////////////////////////////////////////////////////////////////////////
const wchar_t * EButtonSprite::GetSpriteText( void )
{
	return	m_wstrText ;
}

void EButtonSprite::SetSpriteText( const wchar_t * pwszText )
{
	SetButtonText( pwszText ) ;
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	EString	strFont = pwszFont ;
	int		nFontLen = strFont.GetLength() + 1 ;
	if ( nFontLen > LF_FACESIZE )
	{
		nFontLen = LF_FACESIZE ;
	}
	for ( int i = 0; i < bsMax; i ++ )
	{
		::eslMoveMemory
			( m_tsStyle[i].lfFont.lfFaceName, strFont.CharPtr(), nFontLen ) ;
	}
	EWideString	wstrText = m_wstrText ;
	SetButtonText( wstrText ) ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool EButtonSprite::IsHitSprite( int xPos, int yPos )
{
	//
	// 画像の当たり判定
	//
	do
	{
		PEGL_IMAGE_INFO	pInfo = m_pHitTestMask ;
		if ( pInfo == NULL )
		{
			pInfo = m_pImage[bsNormal] ;
			if ( pInfo == NULL )
			{
				break ;
			}
		}
		EGL_POINT	ptGlobal, ptLocal ;
		ptGlobal.x = xPos ;
		ptGlobal.y = yPos ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		if ( (ptLocal.x < 0) || (ptLocal.y < 0)
			|| ((DWORD) ptLocal.x >= pInfo->dwImageWidth)
			|| ((DWORD) ptLocal.y >= pInfo->dwImageHeight) )
		{
			break ;
		}
		if ( pInfo->fdwFormatType & EIF_WITH_ALPHA )
		{
			EGL_PALETTE	px = ::eglGetPixel( pInfo, ptLocal.x, ptLocal.y ) ;
			if ( px.rgba.Alpha >= 0x80 )
			{
				return	true ;
			}
		}
		else if ( pInfo->fdwFormatType & EIF_WITH_CLIPPING )
		{
			EGL_PALETTE	px = ::eglGetPixel( pInfo, ptLocal.x, ptLocal.y ) ;
			if ( px.dwPixelCode != pInfo->dwClippedPixel )
			{
				return	true ;
			}
		}
		else if ( (m_pHitTestMask == pInfo)
					&& (pInfo->fdwFormatType == EIF_GRAY_BITMAP) )
		{
			EGL_PALETTE	px = ::eglGetPixel( pInfo, ptLocal.x, ptLocal.y ) ;
			if ( px.dwPixelCode > 0 )
			{
				return	true ;
			}
		}
		else
		{
			return	true ;
		}
	}
	while ( false ) ;
	//
	// アイテム領域判定
	//
	if ( (m_style == btCheckBox) ||
			(m_style == btRadioButton) || (m_dwBtnFlags & bfHitExtRect) )
	{
		PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
		if ( pInfo == NULL )
		{
			return	false ;
		}
		EGL_POINT	ptGlobal, ptLocal ;
		ptGlobal.x = xPos ;
		ptGlobal.y = yPos ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		if ( (ptLocal.x >= 0) && (ptLocal.y >= 0)
			&& ((DWORD) ptLocal.x < pInfo->dwImageWidth)
			&& ((DWORD) ptLocal.y < pInfo->dwImageHeight) )
		{
			return	true ;
		}
	}
	return	false ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	if ( !(m_status & bsFocus) && IsEnabled() )
	{
		//
		// 効果音再生
		//
		if ( m_wstrFocusSE.IsEmpty() )
			PlaySoundEffect( m_pwszDefFocusSE ) ;
		else
			PlaySoundEffect( m_wstrFocusSE ) ;
	}
	//
	// 表示変化
	//
	if ( !(m_status & bsFocus) && IsEnabled() )
	{
		SetButtonStatus( IsButtonChecked() ? bsPushedFocus : bsFocus ) ;
	}
}

void EButtonSprite::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
	m_fFocus = false ;
	SetButtonStatus( IsButtonChecked() ? bsPushed : bsNormal ) ;
}

bool EButtonSprite::OnSetCursor( int xPos, int yPos )
{
	ESLError	errResult = eslErrGeneral ;
	//
	if ( IsEnabled() )
	{
		if ( m_wstrCursor.IsEmpty() )
			errResult = SetMouseCursor( m_pwszDefCursor ) ;
		else
			errResult = SetMouseCursor( m_wstrCursor ) ;
	}
	//
	return	(errResult == eslErrSuccess) ;
}

bool EButtonSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( IsEnabled() )
	{
		//
		// 効果音再生
		//
		if ( m_wstrPushedSE.IsEmpty() )
		{
			PlaySoundEffect( m_pwszDefPushedSE ) ;
		}
		else
		{
			PlaySoundEffect( m_wstrPushedSE ) ;
		}
		//
		// 表示変化＆コマンド処理
		//
		if ( m_style == btCheckBox )
		{
			ESpriteInterface *	pParent =
				ESLTypeCast<ESpriteInterface>( GetParent() ) ;
			if ( pParent != NULL )
			{
				pParent->SetFocus( this ) ;
			}
			SetButtonStatus( bsActivePushed ) ;
		}
		else if ( m_style == btRadioButton )
		{
			ESpriteInterface *	pParent =
				ESLTypeCast<ESpriteInterface>( GetParent() ) ;
			if ( pParent != NULL )
			{
				pParent->SetFocus( this ) ;
			}
			SetButtonStatus( bsActivePushed ) ;
//			OnCommand( this ) ;
		}
		else
		{
			m_dwPushedTimeStamp =
				::timeGetTime() + m_nBeforeRepInterval ;
			SetButtonStatus( bsActivePushed ) ;
		}
	}
	return	true ;
}

bool EButtonSprite::OnLButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( IsEnabled() )
	{
		if ( m_style == btCheckBox )
		{
			if ( m_statusView == bsActivePushed )
			{
				SetButtonStatus( m_fPushed ? bsFocus : bsPushedFocus ) ;
				OnCommand( this ) ;
			}
		}
		else if ( m_style == btRadioButton )
		{
			if ( m_statusView == bsActivePushed )
			{
				SetButtonStatus( bsPushedFocus ) ;
				OnCommand( this ) ;
			}
		}
		else
		{
			if ( m_statusView == bsActivePushed )
			{
				ESpriteInterface *	pParent =
					ESLTypeCast<ESpriteInterface>( GetParent() ) ;
				if ( pParent != NULL )
				{
					pParent->SetFocus( this ) ;
				}
//				SetButtonStatus( bsFocus ) ;
				m_statusView = m_status ;
				SetButtonStatus( m_status ) ;
				OnCommand( this ) ;
			}
		}
	}
	return	true ;
}

bool EButtonSprite::OnRButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( IsEnabled() && m_fEnableRightClick )
	{
		//
		// 効果音再生
		//
		if ( m_wstrPushedSE.IsEmpty() )
		{
			PlaySoundEffect( m_pwszDefPushedSE ) ;
		}
		else
		{
			PlaySoundEffect( m_wstrPushedSE ) ;
		}
		//
		// 表示変化＆コマンド処理
		//
		if ( m_style == btCheckBox )
		{
			ESpriteInterface *	pParent =
				ESLTypeCast<ESpriteInterface>( GetParent() ) ;
			if ( pParent != NULL )
			{
				pParent->SetFocus( this ) ;
			}
			SetButtonStatus( bsActivePushed ) ;
		}
		else if ( m_style == btRadioButton )
		{
			ESpriteInterface *	pParent =
				ESLTypeCast<ESpriteInterface>( GetParent() ) ;
			if ( pParent != NULL )
			{
				pParent->SetFocus( this ) ;
			}
			SetButtonStatus( bsActivePushed ) ;
		}
		else
		{
			m_dwPushedTimeStamp =
				::timeGetTime() + m_nBeforeRepInterval ;
			SetButtonStatus( bsActivePushed ) ;
		}
		return	true ;
	}
	return	false ;
}

bool EButtonSprite::OnRButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( IsEnabled() && m_fEnableRightClick )
	{
		if ( m_style == btCheckBox )
		{
			if ( m_statusView == bsActivePushed )
			{
				SetButtonStatus( m_fPushed ? bsFocus : bsPushedFocus ) ;
				OnCommand( this, 0, m_nRightClickParameter ) ;
			}
		}
		else if ( m_style == btRadioButton )
		{
			if ( m_statusView == bsActivePushed )
			{
				SetButtonStatus( bsPushedFocus ) ;
				OnCommand( this, 0, m_nRightClickParameter ) ;
			}
		}
		else
		{
			if ( m_statusView == bsActivePushed )
			{
				ESpriteInterface *	pParent =
					ESLTypeCast<ESpriteInterface>( GetParent() ) ;
				if ( pParent != NULL )
				{
					pParent->SetFocus( this ) ;
				}
				m_statusView = m_status ;
				SetButtonStatus( m_status ) ;
				OnCommand( this, 0, m_nRightClickParameter ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// メッセージ処理（タイマーフラグを持っているもののみ）
//////////////////////////////////////////////////////////////////////////////
bool EButtonSprite::OnTimer( UINT nEventID )
{
	if ( (m_nBeforeRepInterval >= 0) && (m_nRepeatInterval > 0)
		&& (m_statusView == bsActivePushed)
		&& (m_style != btCheckBox) && (m_style != btRadioButton) )
	{
		long int	nCurrentTime =
			(long int) (::timeGetTime() - m_dwPushedTimeStamp) ;
		if ( nCurrentTime >= m_nRepeatInterval )
		{
			OnCommand( this ) ;
			m_dwPushedTimeStamp += m_nRepeatInterval ;
		}
	}
	return	EStaticTextSprite::OnTimer( nEventID ) ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
bool EButtonSprite::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( m_fEnabledKeyInput && IsEnabled() )
	{
		if ( uMsg == WM_KEYDOWN )
		{
			if ( wParam == VK_SPACE )
			{
				if ( m_style == btCheckBox )
				{
					SetButtonStatus( bsActivePushed ) ;
				}
				else if ( m_style == btRadioButton )
				{
					SetButtonStatus( bsActivePushed ) ;
					OnCommand( this ) ;
				}
				else
				{
					m_dwPushedTimeStamp =
						::timeGetTime() + m_nBeforeRepInterval ;
					SetButtonStatus( bsActivePushed ) ;
				}
				return	true ;
			}
			else if ( (wParam == VK_LEFT) || (wParam == VK_UP) )
			{
				if ( m_style == btRadioButton )
				{
					ESpriteInterface *	pParent =
						ESLTypeCast<ESpriteInterface>( GetParent() ) ;
					if ( pParent != NULL )
					{
						ESpriteInterface *	pPrev = this ;
						for ( ; ; )
						{
							pPrev = pParent->GetNextItemOnGroup( pPrev, false ) ;
							if ( pPrev == NULL )
								break ;
							if ( pPrev->IsVisible() && pPrev->IsEnabled() )
							{
								pParent->SetFocus( pPrev ) ;
								break ;
							}
						}
					}
					return	true ;
				}
			}
			else if ( (wParam == VK_RIGHT) || (wParam == VK_DOWN) )
			{
				if ( m_style == btRadioButton )
				{
					ESpriteInterface *	pParent =
						ESLTypeCast<ESpriteInterface>( GetParent() ) ;
					if ( pParent != NULL )
					{
						ESpriteInterface *	pNext = this ;
						for ( ; ; )
						{
							pNext = pParent->GetNextItemOnGroup( pNext, true ) ;
							if ( pNext == NULL )
								break ;
							if ( pNext->IsVisible() && pNext->IsEnabled() )
							{
								pParent->SetFocus( pNext ) ;
								break ;
							}
						}
					}
					return	true ;
				}
			}
		}
		else if ( uMsg == WM_KEYUP )
		{
			if ( wParam == VK_SPACE )
			{
				if ( m_style == btCheckBox )
				{
//					SetButtonStatus( m_fPushed ? bsFocus : bsPushedFocus ) ;
					SetButtonStatus( m_fPushed ? bsNormal : bsPushed ) ;
					OnCommand( this ) ;
				}
				else if ( m_style == btRadioButton )
				{
//					SetButtonStatus( bsPushedFocus ) ;
					SetButtonStatus( bsPushed ) ;
				}
				else
				{
					if ( m_statusView )
					{
//						SetButtonStatus( bsFocus ) ;
						m_statusView = m_status ;
						SetButtonStatus( m_status ) ;
						OnCommand( this ) ;
					}
				}
				ESpriteInterface *	pParent =
					ESLTypeCast<ESpriteInterface>( GetParent() ) ;
				if ( pParent != NULL )
				{
					pParent->SetFocus( this ) ;
				}
				return	true ;
			}
		}
	}
	return	EStaticTextSprite::MessageProc( hWnd, uMsg, wParam, lParam ) ;
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EButtonSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	for ( int i = 0; i < dscParam.GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = dscParam.GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"status_reflection" )
		{
			SetStatusReflection( *pdscTag ) ;
		}
		else if ( pdscTag->Tag() == L"status_notification" )
		{
			SetStatusNotification
				( true, pdscTag->GetAttrInteger( L"parameter", 1 ) ) ;
		}
		else if ( pdscTag->Tag() == L"right_click" )
		{
			EnableRightClick
				( true, pdscTag->GetAttrInteger( L"parameter", 2 ) ) ;
		}
		else if ( pdscTag->Tag() == L"push_repeat" )
		{
			SetRepeatButtonNotification
				( pdscTag->GetAttrInteger( L"before_repeat", 0 ),
						pdscTag->GetAttrInteger( L"interval", 0 ) ) ;
		}
	}
	return	EStaticTextSprite::SendCommand( dscParam, pwstrResult ) ;
}

// フォーカスを取得した
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::OnSetFocus( void )
{
	m_fFocus = true ;
	SetButtonStatus( m_status ) ;
}

// フォーカスを奪われた
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::OnKillFocus( void )
{
	m_fFocus = false ;
/*	if ( m_status == bsFocus )
	{
		m_status = bsNormal ;
	}
	else if ( m_status == bsPushedFocus )
	{
		m_status = bsPushed ;
	}
	m_statusView = m_status ;
	SetButtonStatus( m_status ) ;*/
	SetButtonStatus( m_fPushed ? bsPushed : bsNormal ) ;
}

// ボタンオブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError EButtonSprite::CreateButton( const BUTTON_STYLE & style )
{
	//
	// 画像バッファ参照型
	//
	if ( (style.btType == btStatic) || (style.btType == btDynamic) )
	{
		return	SetButtonImage( style ) ;
	}
	//
	// ボタン画像生成
	//
	HEGL_DRAW_IMAGE	hDrawImage ;
	EGL_DRAW_PARAM	edp ;
	ESLError	errResult = eslErrSuccess ;
	hDrawImage = ::eglCreateDrawImage( ) ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	//
	m_pHitTestMask = style.pHitTestMask ;
	if ( m_pHitTestMask != NULL )
	{
		m_imgHitTestMask.DuplicateImage( m_pHitTestMask ) ;
		m_pHitTestMask = m_imgHitTestMask ;
	}
	m_style = style.btType ;
	m_dwBtnFlags = style.dwFlags ;
	m_wstrText = style.tsTextStyle[bsNormal].pwszText ;
	//
	for ( int i = 0; i < bsMax; i ++ )
	{
		//
		// 画像バッファ生成
		//
		PEGL_IMAGE_INFO	pInfo = style.pImage[i] ;
		const TEXT_STYLE *	pTextStyle = &style.tsTextStyle[i] ;
		if ( pInfo == NULL )
		{
			if ( i == bsPushedFocus )
			{
				pInfo = style.pImage[bsPushed] ;
				pTextStyle = &style.tsTextStyle[bsPushed] ;
			}
			else if ( i == bsPushDisabled )
			{
				pInfo = style.pImage[bsDisabled] ;
				pTextStyle = &style.tsTextStyle[bsDisabled] ;
				if ( pInfo == NULL )
				{
					pInfo = style.pImage[bsPushed] ;
					pTextStyle = &style.tsTextStyle[bsPushed] ;
				}
			}
			else if ( i == bsActivePushed )
			{
				pInfo = style.pImage[bsPushedFocus] ;
				pTextStyle = &style.tsTextStyle[bsPushedFocus] ;
				if ( pInfo == NULL )
				{
					pInfo = style.pImage[bsPushed] ;
					pTextStyle = &style.tsTextStyle[bsPushed] ;
				}
			}
			if ( pInfo == NULL )
			{
				pInfo = style.pImage[bsNormal] ;
				pTextStyle = &style.tsTextStyle[bsNormal] ;
			}
/*			if ( pInfo == NULL )
			{
				errResult = ESLErrorMsg( "画像が指定されていません。" ) ;
				break ;
			}*/
		}
		m_pImage[i] = m_imgButton[i].CreateImage
			( EIF_RGBA_BITMAP, style.sizeExt.w, style.sizeExt.h, 32, 0 ) ;
		if ( m_pImage[i] == NULL )
		{
			errResult = ESLErrorMsg( "画像バッファの生成に失敗しました。" ) ;
			break ;
		}
		//
		// ボタン画像複製
		//
		hDrawImage->Initialize( m_imgButton[i], NULL, NULL ) ;
		//
		if ( pInfo != NULL )
		{
			m_imgBase[i].CreateImage
				( EIF_RGBA_BITMAP,
					pInfo->dwImageWidth, pInfo->dwImageHeight, 32, 0 ) ;
			//
			m_imgBase[i].ConvertFrom( pInfo ) ;
			//
			edp.dwFlags = 0 ;
			edp.pSrcImage = m_imgBase[i] ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
		}
		//
		// 文字列描画
		//
		m_tsStyle[i] = *pTextStyle ;
		if ( pTextStyle->pwszText != NULL )
		{
			ERealFontImage	rfiText ;
			if ( !CreateFontImage( rfiText, *pTextStyle, NULL ) )
			{
				hDrawImage->Initialize
					( m_imgButton[i], &(pTextStyle->rectExt), NULL ) ;
				DrawFontImage( hDrawImage, rfiText, *pTextStyle ) ;
			}
		}
	}
	hDrawImage->Release( ) ;
	m_fEnabledKeyInput = true ;
	SetButtonStatus( m_status ) ;
	return	errResult ;
}

// ボタン画像設定
//////////////////////////////////////////////////////////////////////////////
ESLError EButtonSprite::SetButtonImage( const BUTTON_STYLE & style )
{
	if ( style.btType == btStatic )
	{
		m_pHitTestMask = style.pHitTestMask ;
		m_style = style.btType ;
		//
		for ( int i = 0; i < bsMax; i ++ )
		{
			m_pImage[i] = style.pImage[bsNormal] ;
		}
		SetButtonStatus( m_status ) ;
		return	eslErrSuccess ;
	}
	else if ( style.btType == btDynamic )
	{
		m_pHitTestMask = style.pHitTestMask ;
		m_style = style.btType ;
		//
		for ( int i = 0; i < bsMax; i ++ )
		{
			m_pImage[i] = style.pImage[i] ;
			if ( m_pImage[i] == NULL )
			{
				m_pImage[i] = style.pImage[bsNormal] ;
			}
		}
		SetButtonStatus( m_status ) ;
		return	eslErrSuccess ;
	}
	//
	return	eslErrGeneral ;
}

// ボタン文字列設定
//////////////////////////////////////////////////////////////////////////////
ESLError EButtonSprite::SetButtonText( const wchar_t * pwszText )
{
	if ( (m_style == btStatic) || (m_style == btDynamic) )
	{
		return	eslErrGeneral ;
	}
	//
	HEGL_DRAW_IMAGE	hDrawImage ;
	EGL_DRAW_PARAM	edp ;
	hDrawImage = ::eglCreateDrawImage( ) ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	m_wstrText = pwszText ;
	//
	for ( int i = 0; i < bsMax; i ++ )
	{
		if ( m_imgButton[i].GetInfo() == NULL )
		{
			continue ;
		}
		//
		// ベース画像描画
		//
		m_imgButton[i].FillImage( EGLPalette( 0 ) ) ;
		hDrawImage->Initialize( m_imgButton[i], NULL, NULL ) ;
		edp.dwFlags = 0 ;
		edp.pSrcImage = m_imgBase[i] ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		//
		// 文字列描画
		//
		m_tsStyle[i].pwszText = pwszText ;
		if ( pwszText != NULL )
		{
			ERealFontImage	rfiText ;
			if ( !CreateFontImage( rfiText, m_tsStyle[i], NULL ) )
			{
				hDrawImage->Initialize
					( m_imgButton[i], &(m_tsStyle[i].rectExt), NULL ) ;
				DrawFontImage( hDrawImage, rfiText, m_tsStyle[i] ) ;
			}
		}
	}
	hDrawImage->Release( ) ;
	SetButtonStatus( m_status ) ;
	return	eslErrSuccess ;
}

// リソース破棄
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::Delete( void )
{
	for ( int i = 0; i < bsMax; i ++ )
	{
		m_pImage[i] = NULL ;
		m_imgButton[i].DeleteImage( ) ;
	}
	m_fStatusNotification = false ;
	m_wstrStatusRefTarget.FreeString( ) ;
	//
	SetButtonStatus( m_status ) ;
}

// ボタンの状態を設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetButtonStatus( EButtonSprite::ButtonStatus status )
{
	//
	// ステータス変化通知
	//
	ButtonStatus statusView = status ;
	bool	fEnabled = IsEnabled( ) ;
	if ( status == m_status )
	{
		statusView = m_statusView ;
	}
	if ( (status >= bsNormal) && (status <= bsPushedFocus) )
	{
		m_fPushed = (status & bsPushed) != 0 ;
	}
	if ( status == bsActivePushed )
	{
		status = m_status ;
		statusView = bsActivePushed ;
	}
	else
	{
		if ( statusView == bsActivePushed )
		{
			statusView = status ;
		}
		if ( m_fFocus )
		{
			if ( status == bsNormal )
			{
				statusView = bsFocus ;
			}
			if ( status == bsPushed )
			{
				statusView = bsPushedFocus ;
			}
		}
		else
		{
			if ( (statusView == bsFocus) || (statusView == bsPushedFocus) )
			{
				statusView = status ;
			}
		}
	}
	if ( !fEnabled )
	{
		if ( ((status & bsPushed) != 0) || (status == bsPushDisabled) )
		{
			statusView = bsPushDisabled ;
		}
		else
		{
			statusView = bsDisabled ;
		}
	}
	if ( m_fStatusNotification && (m_statusView != statusView) )
	{
		OnCommand( this, statusView, m_nStatusNoticeParameter ) ;
	}
	//
	// ステータス変化
	//
	m_status = status ;
	m_statusView = statusView ;
	//
	// 画像取得
	//
	PEGL_IMAGE_INFO	pNewImage = m_pImage[statusView] ;
	if ( pNewImage != GetInfo() )
	{
		AttachImage( pNewImage ) ;
	}
	//
	// ラジオボタン処理
	//
	if ( (m_style == btRadioButton) && IsButtonChecked() )
	{
		ESpriteInterface *	pParent =
			ESLTypeCast<ESpriteInterface>( GetParent() ) ;
		if ( pParent != NULL )
		{
			EButtonSprite *	pNext = this ;
			for ( ; ; )
			{
				pNext = ESLTypeCast<EButtonSprite>
						( pParent->GetNextItemOnGroup( pNext, false ) ) ;
				if ( (pNext == NULL) || (pNext == this) )
					break ;
				pNext->SetButtonStatus
					( (EButtonSprite::ButtonStatus)
						(pNext->GetButtonStatus() & ~bsPushed) ) ;
			}
			pNext = this ;
			for ( ; ; )
			{
				pNext = ESLTypeCast<EButtonSprite>
						( pParent->GetNextItemOnGroup( pNext, true ) ) ;
				if ( (pNext == NULL) || (pNext == this) )
					break ;
				pNext->SetButtonStatus
					( (EButtonSprite::ButtonStatus)
						(pNext->GetButtonStatus() & ~bsPushed) ) ;
			}
		}
	}
	//
	// ステータスリフレクション処理
	//
	if ( !m_wstrStatusRefTarget.IsEmpty() )
	do
	{
		ESpriteInterface *	pParent =
			ESLTypeCast<ESpriteInterface>( GetParent() ) ;
		if ( pParent == NULL )
		{
			break ;
		}
		EFormResourceManager *	pfrmRsrc = pParent->GetResourceManager( ) ;
		if ( pfrmRsrc == NULL )
		{
			break ;
		}
		EAnimationSprite *	pTargetItem =
			ESLTypeCast<EAnimationSprite>
				( pParent->GetSpriteItemAs( m_wstrStatusRefTarget ) ) ;
		if ( pTargetItem == NULL )
		{
			break ;
		}
		for ( ; ; )
		{
			EGL_IMAGE_INFO	eiiImage ;
			EGLAnimation *	pImage =
				ESLTypeCast<EGLAnimation>
					( pfrmRsrc->GetResourceAs
						( m_wstrStatusRefImage[statusView] ) ) ;
			if ( pImage != NULL )
			{
				pTargetItem->UpdateRect( NULL ) ;
				pTargetItem->CreateAnimation( pImage ) ;
			}
			else if ( pfrmRsrc->GetStillImageResource
					( m_wstrStatusRefImage[statusView], &eiiImage ) != NULL )
			{
				pTargetItem->SetImageView( &eiiImage ) ;
			}
			else
			{
				static const ButtonStatus	bsStatusRemap[bsMax] =
				{
					bsNormal, bsNormal, bsFocus, bsPushed,
					bsNormal, bsPushed, bsPushedFocus
				} ;
				if ( statusView == bsNormal )
				{
					pTargetItem->UpdateRect( NULL ) ;
					pTargetItem->AttachImage( NULL ) ;
				}
				else
				{
					statusView = bsStatusRemap[statusView] ;
					continue ;
				}
			}
			break ;
		}
	}
	while ( false ) ;
}

// ボタンがチェックされているか？（チェックボタンのみ）
//////////////////////////////////////////////////////////////////////////////
bool EButtonSprite::IsButtonChecked( void )
{
	if ( (m_style == btCheckBox) || (m_style == btRadioButton) )
	{
		return	(m_status == bsPushed)
			|| (m_status == bsPushedFocus) || (m_status == bsPushDisabled) ;
	}
	return	false ;
}

// ボタンをチェックする（チェックボタンのみ）
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::CheckButton( bool fCheck )
{
	if ( (m_style == btCheckBox) || (m_style == btRadioButton) )
	{
		int		status = m_status ;
		m_fPushed = fCheck ;
		if ( fCheck )
		{
			if ( status < bsDisabled )
			{
				status |= bsPushed ;
			}
			else
			{
				status = bsPushDisabled ;
			}
		}
		else
		{
			if ( status < bsDisabled )
			{
				status &= ~bsPushed ;
			}
			else
			{
				status = bsDisabled ;
			}
		}
		SetButtonStatus( (ButtonStatus) status ) ;
	}
}

// フォーカス時のカーソル識別子を設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetCursorOnFocus( const wchar_t * pwszCursorID )
{
	m_wstrCursor = pwszCursorID ;
}

// （マウスカーソル）フォーカス時の効果音を設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetSoundOnFocus( const wchar_t * pwszSoundID )
{
	m_wstrFocusSE = pwszSoundID ;
}

// 押下時の効果音を設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetSoundOnPushed( const wchar_t * pwszSoundID )
{
	m_wstrPushedSE = pwszSoundID ;
}

// ステータス変化の通知設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetStatusNotification
	( bool fStatusNotification, long int nParameter )
{
	m_fStatusNotification = fStatusNotification ;
	m_nStatusNoticeParameter = nParameter ;
}

// 右クリック通知設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::EnableRightClick
	( bool fRightClick, long int nParameter )
{
	m_fEnableRightClick = fRightClick ;
	m_nRightClickParameter = nParameter ;
}

// ステータスリフレクション設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetStatusReflection( EDescription & dscParam )
{
	ESLAssert( dscParam.Tag() == L"status_reflection" ) ;
	m_wstrStatusRefTarget = dscParam.GetAttrString( L"target", NULL ) ;
	//
	for ( int i = 0; i < dscParam.GetContentTagCount(); i ++ )
	{
		static const wchar_t *	pwszStatusType[bsMax] =
		{
			L"normal", L"focus",
			L"pushed", L"pushed_focus",
			L"disabled", L"push_disabled",
			L"active_pushed"
		} ;
		EDescription *	pdscStatus = dscParam.GetContentTagAt( i ) ;
		if ( pdscStatus == NULL )
		{
			continue ;
		}
		for ( int j = 0; j < bsMax; j ++ )
		{
			if ( pdscStatus->Tag() == pwszStatusType[j] )
			{
				m_wstrStatusRefImage[j] =
					pdscStatus->GetAttrString( L"image", NULL ) ;
				break ;
			}
		}
	}
	//
	SetButtonStatus( m_status ) ;
}

// ボタンリピート押下機能設定
//////////////////////////////////////////////////////////////////////////////
void EButtonSprite::SetRepeatButtonNotification
	( long int nBeforeRepeat, long int nRepeatInterval )
{
	m_nBeforeRepInterval = nBeforeRepeat ;
	m_nRepeatInterval = nRepeatInterval ;
	//
	if ( (m_nBeforeRepInterval >= 0) && (m_nRepeatInterval > 0) )
	{
		SetFunctionFlags( GetFunctionFlags() | ffTimer ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// スクロールバースプライト
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	EScrollBarSprite::m_pwszDefCursor = L"IDC_FOCUS" ;
const wchar_t *	EScrollBarSprite::m_pwszPrevButtonID = L"ID_PREV" ;
const wchar_t *	EScrollBarSprite::m_pwszNextButtonID = L"ID_NEXT" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EScrollBarSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EScrollBarSprite::EScrollBarSprite( void )
{
	m_sbtType = sbtVert ;
	m_bsColumn = bsNormal ;
	m_bsBar = bsNormal ;
	m_pItem = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EScrollBarSprite::~EScrollBarSprite( void )
{
	DetachAllSprite( ) ;
}

// スクロールバーを作成
//////////////////////////////////////////////////////////////////////////////
ESLError EScrollBarSprite::CreateScrollBar( const BAR_STYLE & style )
{
	DetachAllSprite( ) ;
	//
	// 画像バッファ生成
	//
	if ( CreateImage( EIF_RGBA_BITMAP,
			style.sizeBarExt.w, style.sizeBarExt.h, 32 ) == NULL )
	{
		return	ESLErrorMsg( "画像バッファの生成に失敗しました。" ) ;
	}
	//
	// 上下ボタン生成
	//
	ESLError	err = eslErrSuccess ;
	if ( style.bsPrevButton.btType != EButtonSprite::btMax )
	{
		err = m_btnPrev.CreateButton( style.bsPrevButton ) ;
		if ( err )
		{
			return	err ;
		}
		m_btnPrev.SetRepeatButtonNotification( 1000, 200 ) ;
		SetFunctionFlags( GetFunctionFlags() | ffTimer ) ;
		m_btnPrev.MovePosition( style.ptPrevButton ) ;
		AddSpriteItem( m_pwszPrevButtonID, &m_btnPrev ) ;
		m_btnPrev.SetVisible( true ) ;
	}
	if ( style.bsNextButton.btType != EButtonSprite::btMax )
	{
		err = m_btnNext.CreateButton( style.bsNextButton ) ;
		if ( err )
		{
			return	err ;
		}
		m_btnNext.SetRepeatButtonNotification( 1000, 200 ) ;
		SetFunctionFlags( GetFunctionFlags() | ffTimer ) ;
		m_btnNext.MovePosition( style.ptNextButton ) ;
		AddSpriteItem( m_pwszNextButtonID, &m_btnNext ) ;
		m_btnNext.SetVisible( true ) ;
	}
	//
	// つまみ画像設定
	//
	EGL_DRAW_PARAM	edp ;
	HEGL_DRAW_IMAGE	hDrawImage = ::eglCreateDrawImage( ) ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	//
	for ( int i = 0; i < bsMax; i ++ )
	{
		//
		// 背景カラム画像
		//
		PEGL_IMAGE_INFO	pColumn = style.pColumnImage[i] ;
		if ( pColumn == NULL )
		{
			pColumn = style.pColumnImage[bsNormal] ;
/*			if ( pColumn == NULL )
			{
				return	ESLErrorMsg( "カラム画像が指定されていません。" ) ;
			}*/
		}
		edp.pSrcImage = pColumn ;
		//
		if ( style.sbtBarType == sbtVert )
		{
			if ( pColumn != NULL )
			{
				m_imgColumn[i].CreateImage
					( EIF_RGBA_BITMAP,
						pColumn->dwImageWidth, style.nColumnWidth, 32 ) ;
				hDrawImage->Initialize( m_imgColumn[i], NULL, NULL ) ;
				//
				for ( int y = 0; y < style.nColumnWidth;
									y += pColumn->dwImageHeight )
				{
					edp.ptBasePos.y = y ;
					if ( !hDrawImage->PrepareDraw( &edp ) )
					{
						hDrawImage->DrawImage( ) ;
					}
				}
			}
			else
			{
				m_imgColumn[i].CreateImage
					( EIF_RGBA_BITMAP,
						style.sizeBarExt.w, style.nColumnWidth, 32 ) ;
			}
		}
		else
		{
			if ( pColumn != NULL )
			{
				m_imgColumn[i].CreateImage
					( EIF_RGBA_BITMAP,
						style.nColumnWidth, pColumn->dwImageHeight, 32 ) ;
				hDrawImage->Initialize( m_imgColumn[i], NULL, NULL ) ;
				//
				for ( int x = 0; x < style.nColumnWidth;
									x += pColumn->dwImageWidth )
				{
					edp.ptBasePos.x = x ;
					if ( !hDrawImage->PrepareDraw( &edp ) )
					{
						hDrawImage->DrawImage( ) ;
					}
				}
			}
			else
			{
				m_imgColumn[i].CreateImage
					( EIF_RGBA_BITMAP,
						style.nColumnWidth, style.sizeBarExt.h, 32 ) ;
			}
		}
		//
		// つまみ画像
		//
		PEGL_IMAGE_INFO	pBar = style.pBarImage[i] ;
		if ( pBar == NULL )
		{
			pBar = style.pBarImage[bsNormal] ;
			if ( pBar == NULL )
			{
				return	ESLErrorMsg( "つまみ画像が指定されていません。" ) ;
			}
		}
		m_imgBar[i].DuplicateImage( pBar ) ;
		//
		// 進捗バー画像
		//
		PEGL_IMAGE_INFO	pProgress = style.pProgressImage[i] ;
		if ( pProgress == NULL )
		{
			pProgress = style.pProgressImage[bsNormal] ;
		}
		if ( pProgress != NULL )
		{
			m_imgProgress[i].DuplicateImage( pProgress ) ;
		}
	}
	hDrawImage->Release( ) ;
	//
	// ステータス設定
	//
	m_sbtType = style.sbtBarType ;
	m_bsColumn = bsNormal ;
	m_bsBar = bsNormal ;
	m_ptBarBase = style.ptColumnPos ;
	m_ptTrackBase.x = m_ptBarBase.x + style.rctTrackSpace.left ;
	m_ptTrackBase.y = m_ptBarBase.y + style.rctTrackSpace.top ;
	m_nWidth = style.nColumnWidth ;
	if ( m_sbtType == sbtVert )
	{
		m_nWidth -= m_imgBar[bsNormal].GetInfo()->dwImageHeight
			+ (style.rctTrackSpace.top + style.rctTrackSpace.bottom) ;
	}
	else
	{
		m_nWidth -= m_imgBar[bsNormal].GetInfo()->dwImageWidth
			+ (style.rctTrackSpace.left + style.rctTrackSpace.right) ;
	}
	if ( m_nWidth <= 0 )
	{
		m_nWidth = 1 ;
	}
	m_nPos = 0 ;
	m_nRange = 0 ;
	m_nLine = 32 ;
	//
	// つまみ画像設定
	//
	m_isColumn.AttachImage( m_imgColumn[bsDisabled] ) ;
	m_siBar.AttachImage( m_imgBar[bsDisabled] ) ;
	m_isColumn.MovePosition( m_ptBarBase ) ;
	m_isProgress.MovePosition( m_ptBarBase ) ;
	m_siBar.MovePosition( m_ptTrackBase ) ;
	AddSprite( 0x18, &m_isColumn ) ;
	AddSprite( 0x14, &m_isProgress ) ;
	AddSprite( 0x10, &m_siBar ) ;
	m_isColumn.SetVisible( true ) ;
	m_isProgress.SetVisible( true ) ;
	m_siBar.SetVisible( true ) ;
	//
	// 初期状態設定
	//
	SetScrollRange( m_nRange ) ;
	//
	return	eslErrSuccess ;
}

// 有効化・無効化
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::Enable( bool fEnable )
{
	ESpriteInterface::Enable( fEnable ) ;
	//
	if ( fEnable )
	{
		SetScrollRange( m_nRange ) ;
	}
	else
	{
		m_btnPrev.Enable( false ) ;
		m_btnNext.Enable( false ) ;
		m_isColumn.AttachImage( m_imgColumn[bsDisabled] ) ;
		m_siBar.AttachImage( m_imgBar[bsDisabled] ) ;
	}
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool EScrollBarSprite::IsHitSprite( int xPos, int yPos )
{
	//
	// スクロールバー全体の矩形判定
	//
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
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
	//
	// 上下ボタン判定
	//
	if ( m_btnPrev.IsHitSprite( ptLocal.x, ptLocal.y ) )
	{
		return	true ;
	}
	if ( m_btnNext.IsHitSprite( ptLocal.x, ptLocal.y ) )
	{
		return	true ;
	}
	//
	// つまみ判定
	//
	EGL_RECT	rect = m_isColumn.GetRectangle( ) ;
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		return	true ;
	}
//	if ( m_siBar.IsHitSprite( ptLocal.x, ptLocal.y ) )
//	{
//		return	true ;
//	}
	rect = m_siBar.GetRectangle( ) ;
	if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
		&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
	{
		return	true ;
	}
	//
	return	false ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	//
	if ( m_bsBar == bsTracking )
	{
		//
		// トラッキング中
		//
		ESLAssert( m_nRange > 0 ) ;
		EGL_POINT	ptTracking = ptLocal ;
		if ( m_sbtType == sbtVert )
		{
			ptTracking.y -= m_nTrackOffset ;
		}
		else
		{
			ptTracking.x -= m_nTrackOffset ;
		}
		int	nPos = GetScrollPosFromPoint( ptTracking.x, ptTracking.y ) ;
		if ( nPos != m_nPos )
		{
			SetScrollPos( nPos ) ;
			OnCommand
				( this, ncTracking, m_nPos,
						EWndSpriteCmd::priorityNormal, true ) ;
		}
	}
	else
	{
		//
		// 上下ボタン等
		//
		ESpriteInterface::OnMouseMove( nFlags, xPos, yPos ) ;
		//
		if ( (m_nRange > 0) && IsEnabled() )
		{
			//
			// つまみ
			//
			if ( m_bsColumn == bsNormal )
			{
				OnCommand
					( this, ncOnMouse, m_nPos,
							EWndSpriteCmd::priorityNormal, false ) ;
			}
			EGL_RECT	rect = m_siBar.GetRectangle( ) ;
			if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
				&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
//			if ( m_siBar.IsHitSprite( ptLocal.x, ptLocal.y ) )
			{
				m_bsBar = bsFocus ;
				m_siBar.AttachImage( m_imgBar[bsFocus] ) ;
				m_bsColumn = bsFocus ;
				m_isColumn.AttachImage( m_imgColumn[bsFocus] ) ;
			}
			else
			{
				//
				// 背景カラム
				//
				EGL_RECT	rect = m_isColumn.GetRectangle( ) ;
				if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
					&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
				{
					m_bsColumn = bsFocus ;
					m_isColumn.AttachImage( m_imgColumn[bsFocus] ) ;
				}
				m_bsBar = bsNormal ;
				m_siBar.AttachImage( m_imgBar[bsNormal] ) ;
			}
		}
	}
}

void EScrollBarSprite::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
	//
	// 上下ボタン等
	//
	ESpriteInterface::OnMouseLeave( nFlags, xPos, yPos ) ;
	//
	// つまみ
	//
	if ( m_bsColumn != bsNormal )
	{
		OnCommand
			( this, ncOnLeave, m_nPos,
					EWndSpriteCmd::priorityNormal, false ) ;
	}
	m_bsColumn = bsNormal ;
	m_bsBar = bsNormal ;
	SetScrollRange( m_nRange ) ;
}

bool EScrollBarSprite::OnSetCursor( int xPos, int yPos )
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

bool EScrollBarSprite::OnMouseWheel
	( UINT nFlags, short int zDelta, int xPos, int yPos )
{
	int	nPos = m_nPos - zDelta * m_nLine / WHEEL_DELTA ;
	if ( nPos != m_nPos )
	{
		SetScrollPos( nPos ) ;
		OnCommand
			( this, ncGeneric, m_nPos,
				EWndSpriteCmd::priorityNormal, true ) ;
	}
	return	true ;
}

bool EScrollBarSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = xPos ;
	ptGlobal.y = yPos ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	//
	// 上下ボタン
	//
	if ( ESpriteInterface::OnLButtonDown( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	//
	if ( (m_nRange > 0) && IsEnabled() )
	{
		//
		// つまみ
		//
		EGL_RECT	rect = m_siBar.GetRectangle( ) ;
		if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
			&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
//		if ( m_siBar.IsHitSprite( ptLocal.x, ptLocal.y ) )
		{
			//
			// トラッキング開始
			//
			EGL_POINT	ptBar = m_siBar.GetPosition( ) ;
			if ( m_sbtType == sbtVert )
			{
				m_nTrackOffset = ptLocal.y - ptBar.y ;
			}
			else
			{
				m_nTrackOffset = ptLocal.x - ptBar.x ;
			}
			//
			m_bsBar = bsTracking ;
			m_siBar.AttachImage( m_imgBar[bsTracking] ) ;
			//
			SetCapture( ) ;
			//
			return	true ;
		}
		//
		// カラム判定
		//
		rect = m_isColumn.GetRectangle( ) ;
		if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
			&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
		{
			int	nPos ;
			PEGL_IMAGE_INFO	pBar = m_imgBar[bsNormal] ;
			if ( pBar != NULL )
			{
				if ( m_sbtType == sbtVert )
				{
					nPos = GetScrollPosFromPoint
						( ptLocal.x, ptLocal.y - pBar->dwImageHeight / 2 ) ;
				}
				else
				{
					nPos = GetScrollPosFromPoint
						( ptLocal.x - pBar->dwImageWidth / 2, ptLocal.y ) ;
				}
				SetScrollPos( nPos ) ;
				OnCommand
					( this, ncClickColumn, m_nPos,
						EWndSpriteCmd::priorityNormal, true ) ;
				return	true ;
			}
		}
	}
	return	true ;
}

bool EScrollBarSprite::OnLButtonUp( UINT nFlags, int xPos, int yPos )
{
	ESpriteInterface::OnLButtonUp( nFlags, xPos, yPos ) ;
	//
	if ( m_bsBar == bsTracking )
	{
		EGL_POINT	ptGlobal, ptLocal ;
		ptGlobal.x = xPos ;
		ptGlobal.y = yPos ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		EGL_RECT	rect = m_siBar.GetRectangle( ) ;
		if ( (rect.left <= ptLocal.x) && (ptLocal.x <= rect.right)
			&& (rect.top <= ptLocal.y) && (ptLocal.y <= rect.bottom) )
//		if ( m_siBar.IsHitSprite( ptLocal.x, ptLocal.y ) )
		{
			m_bsBar = bsFocus ;
		}
		else
		{
			m_bsBar = bsNormal ;
		}
		//
		OnCommand( this, ncEndTracking, m_nPos ) ;
		m_siBar.AttachImage( m_imgBar[m_bsBar] ) ;
		ReleaseCapture( ) ;
	}
	return	true ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::OnCommand
	( ESpriteInterface * pItem,
		long int nNotification, long int nParameter,
		int nPriority, bool fOverwrite )
{
	if ( (pItem == this) || (pItem == NULL) )
	{
		ESpriteInterface::OnCommand
			( pItem, nNotification, nParameter, nPriority, fOverwrite ) ;
		return ;
	}
	if ( pItem->ID() == m_pwszPrevButtonID )
	{
		SetScrollPos( m_nPos - m_nLine ) ;
		ESpriteInterface::OnCommand
			( this, ncLineUp, m_nPos, nPriority, fOverwrite ) ;
	}
	else if ( pItem->ID() == m_pwszNextButtonID )
	{
		SetScrollPos( m_nPos + m_nLine ) ;
		ESpriteInterface::OnCommand
			( this, ncLineUp, m_nPos, nPriority, fOverwrite ) ;
	}
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EScrollBarSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	EDescription *	pdscBar = dscParam.GetContentTagAs( 0, L"bar" ) ;
	if ( pdscBar != NULL )
	{
		SetScrollRange( pdscBar->GetAttrInteger( L"range", 0 ) ) ;
		SetScrollPos( pdscBar->GetAttrInteger( L"pos", 0 ) ) ;
	}
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// マウスのキャプチャーがリリースされた
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::OnCaptureReleased( void )
{
	if ( m_bsBar == bsTracking )
	{
		m_bsBar = bsNormal ;
		if ( m_nRange > 0 )
		{
			m_siBar.AttachImage( m_imgBar[m_bsBar] ) ;
		}
	}
}

// （ローカル）座標からスクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EScrollBarSprite::GetScrollPosFromPoint( int xPos, int yPos ) const
{
	if ( m_nWidth != 0 )
	{
		if ( m_sbtType == sbtVert )
		{
			return	(int)(((INT64) ((yPos - m_ptTrackBase.y)
							* m_nRange * 2 / m_nWidth) + 1) >> 1) ;
		}
		else
		{
			return	(int)(((INT64) ((xPos - m_ptTrackBase.x)
							* m_nRange * 2 / m_nWidth) + 1) >> 1) ;
		}
	}
	return	0 ;
}

// スクロール位置から（ローカル）座標を取得
//////////////////////////////////////////////////////////////////////////////
EGL_POINT EScrollBarSprite::GetPointFromScrollPos( int nPos ) const
{
	EGL_POINT	ptBar = m_ptTrackBase ;
	if ( m_nRange > 0 )
	{
		if ( m_sbtType == sbtVert )
		{
			ptBar.y += (int)((INT64) nPos * m_nWidth / m_nRange) ;
		}
		else
		{
			ptBar.x += (int)((INT64) nPos * m_nWidth / m_nRange) ;
		}
	}
	return	ptBar ;
}

// 現在の垂直スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EScrollBarSprite::GetVertScrollPos( void ) const
{
	return	m_nPos ;
}

// 現在の垂直スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetVertScrollPos( int nPos )
{
	SetScrollPos( nPos ) ;
}

// 垂直スクロールの範囲を取得
//////////////////////////////////////////////////////////////////////////////
int EScrollBarSprite::GetVertScrollRange( void ) const
{
	return	m_nRange ;
}

// 垂直スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetVertScrollRange( int nRange )
{
	SetScrollRange( nRange ) ;
}

// 現在の水平スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EScrollBarSprite::GetHorzScrollPos( void ) const
{
	return	m_nPos ;
}

// 現在の水平スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetHorzScrollPos( int nPos )
{
	SetScrollPos( nPos ) ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
int EScrollBarSprite::GetHorzScrollRange( void ) const
{
	return	m_nRange ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetHorzScrollRange( int nRange )
{
	SetScrollRange( nRange ) ;
}

// スクロール位置設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetScrollPos( int nPos )
{
	if ( m_nRange > 0 )
	{
		if ( nPos < 0 )
			m_nPos = 0 ;
		else if ( nPos > m_nRange )
			m_nPos = m_nRange ;
		else
			m_nPos = nPos ;
		//
		m_siBar.MovePosition( GetPointFromScrollPos( m_nPos ) ) ;
		//
		PEGL_IMAGE_INFO	pProgress = m_imgProgress[m_bsColumn] ;
		if ( pProgress != NULL )
		{
			EGL_RECT	rect ;
			rect.left = 0 ;
			rect.top = 0 ;
			rect.right = pProgress->dwImageWidth - 1 ;
			rect.bottom = pProgress->dwImageHeight - 1 ;
			//
			if ( m_sbtType == sbtVert )
			{
				rect.bottom = rect.bottom * m_nPos / m_nRange ;
			}
			else
			{
				rect.right = rect.right * m_nPos / m_nRange ;
			}
			m_isProgress.SetImageView( pProgress, &rect ) ;
		}
		else
		{
			m_isProgress.AttachImage( NULL ) ;
		}
		//
		if ( m_pItem != NULL )
		{
			if ( m_sbtType == sbtVert )
			{
				m_pItem->SetVertScrollPos( m_nPos ) ;
			}
			else
			{
				m_pItem->SetHorzScrollPos( m_nPos ) ;
			}
		}
	}
	else
	{
		m_isProgress.AttachImage( NULL ) ;
	}
}

// スクロール領域設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetScrollRange( int nRange )
{
	if ( nRange > 0 )
	{
		//
		// スクロール位置更新
		//
		m_nRange = nRange ;
		m_btnPrev.Enable( true ) ;
		m_btnNext.Enable( true ) ;
		m_isColumn.AttachImage( m_imgColumn[bsNormal] ) ;
		m_siBar.AttachImage( m_imgBar[bsNormal] ) ;
		SetScrollPos( m_nPos ) ;
	}
	else
	{
		//
		// スクロール不可能状態
		//
		m_nRange = 0 ;
		m_nPos = 0 ;
		m_btnPrev.Enable( false ) ;
		m_btnNext.Enable( false ) ;
		m_isColumn.AttachImage( m_imgColumn[bsDisabled] ) ;
		m_siBar.AttachImage( m_imgBar[bsDisabled] ) ;
		m_siBar.MovePosition( m_ptTrackBase ) ;
	}
	//
	if ( m_pItem != NULL )
	{
		if ( m_sbtType == sbtVert )
		{
			m_pItem->SetVertScrollRange( nRange ) ;
		}
		else
		{
			m_pItem->SetHorzScrollRange( nRange ) ;
		}
	}
}

// 行サイズ設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetLineSize( int nLine )
{
	m_nLine = nLine ;
	if ( m_nLine <= 0 )
	{
		m_nLine = 1 ;
	}
}

// アイテムを関連付ける
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::AttachScrollItem( ESpriteInterface * pItem )
{
	if ( pItem != NULL )
	{
		if ( m_sbtType == sbtVert )
		{
			SetScrollRange( pItem->GetVertScrollRange() ) ;
			SetScrollPos( pItem->GetVertScrollPos() ) ;
		}
		else
		{
			SetScrollRange( pItem->GetHorzScrollRange() ) ;
			SetScrollPos( pItem->GetHorzScrollPos() ) ;
		}
	}
	m_pItem = pItem ;
}

// 関連付けられているアイテムのスクロール情報を反映する
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::ReflectAttachedItem( void )
{
	AttachScrollItem( m_pItem ) ;
}

// フォーカス時のカーソル識別子を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetCursorOnFocus( const wchar_t * pwszCursorID )
{
	m_wstrCursor = pwszCursorID ;
}

// （上下ボタン）フォーカス時の効果音を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetSoundOnFocus( const wchar_t * pwszSoundID )
{
	m_btnPrev.SetSoundOnFocus( pwszSoundID ) ;
	m_btnNext.SetSoundOnFocus( pwszSoundID ) ;
}

// （上下ボタン）押下時の効果音を設定
//////////////////////////////////////////////////////////////////////////////
void EScrollBarSprite::SetSoundOnPushed( const wchar_t * pwszSoundID )
{
	m_btnPrev.SetSoundOnPushed( pwszSoundID ) ;
	m_btnNext.SetSoundOnPushed( pwszSoundID ) ;
}


