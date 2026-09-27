
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
    Copyright (c) 2003-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <ddraw.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#endif


//////////////////////////////////////////////////////////////////////////////
// 画面入出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESpriteInterface, ESpriteServer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface::ESpriteInterface( void )
{
	m_dwFlags = ffGroup | ffTimer ;
	m_fEnabled = true ;
	m_fEnabledKeyInput = true ;
	m_fEnabledMouseWheel = true ;
	m_pFocus = NULL ;
	m_pMouseFocus = NULL ;
	m_pCaptured = NULL ;
	m_nNextPriority = 0 ;
	m_pfrmRsrc = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface::~ESpriteInterface( void )
{
	m_wstaItems.DetachAll( ) ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::DeleteImage( void )
{
	ESpriteServer::DeleteImage( ) ;
	m_pfrmRsrc = NULL ;
}

// 識別名設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetID( const wchar_t * pwszID )
{
	m_wstrID = pwszID ;
}

// 有効化・無効化
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::Enable( bool fEnable )
{
	if ( m_fEnabled && !fEnable )
	{
		KillFocus( ) ;
	}
	m_fEnabled = fEnable ;
}

bool ESpriteInterface::IsEnabled( void ) const
{
	return	m_fEnabled ;
}

// 文字列取得・設定
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ESpriteInterface::GetSpriteText( void )
{
	return	L"" ;
}

void ESpriteInterface::SetSpriteText( const wchar_t * pwszText )
{
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetSpriteFontFace( const wchar_t * pwszFont )
{
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::IsHitSprite( int xPos, int yPos )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	EGL_POINT	ptGlobal, ptLocal ;
	do
	{
		if ( m_dwFlags & ftHitTransparency )
		{
			return	false ;
		}
		ptGlobal.x = xPos ;
		ptGlobal.y = yPos ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		if ( pInfo == NULL )
		{
			break ;
		}
		if ( (ptLocal.x < 0) || (ptLocal.y < 0)
			|| ((DWORD) ptLocal.x >= pInfo->dwImageWidth)
			|| ((DWORD) ptLocal.y >= pInfo->dwImageHeight) )
		{
			return	false ;
		}
		if ( pInfo->fdwFormatType & EIF_WITH_ALPHA )
		{
			EGL_PALETTE	px = GetPixel( ptLocal.x, ptLocal.y ) ;
			if ( px.rgba.Alpha >= 0x80 )
			{
				return	true ;
			}
		}
		else if ( pInfo->fdwFormatType & EIF_WITH_CLIPPING )
		{
			EGL_PALETTE	px = GetPixel( ptLocal.x, ptLocal.y ) ;
			if ( px.dwPixelCode != pInfo->dwClippedPixel )
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
	return	(GetSpriteAtPoint( ptLocal.x, ptLocal.y ) != NULL) ;
}

// マウスが移動した（マウスがアイテムの上に移動した）
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y ) ;
	}
	//
	if ( (pSprite != m_pMouseFocus) && (m_pMouseFocus != NULL) )
	{
		m_pMouseFocus->OnMouseLeave( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	if ( pSprite != NULL )
	{
		pSprite->OnMouseMove( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	m_pMouseFocus = pSprite ;
}

// マウスが移動した（マウスがアイテムの領域外に移動した）
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
	if ( m_pMouseFocus != NULL )
	{
		EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
		m_pMouseFocus->OnMouseLeave( nFlags, ptLocal.x, ptLocal.y ) ;
		m_pMouseFocus = NULL ;
	}
}

// マウスカーソルを設定する（マウスがアイテムの領域の上で移動したとき）
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnSetCursor( int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		return	pSprite->OnSetCursor( ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// ホイールが移動した（マウスが領域内にあるかキャプチャーしているとき）
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnMouseWheel
	( UINT nFlags, short int zDelta, int xPos, int yPos )
{
	if ( !m_fEnabled || !IsEnabledMouseWheel() )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y ) ;
	}
	if ( (pSprite != NULL)
		&& pSprite->IsEnabled() && pSprite->IsEnabledMouseWheel() )
	{
		return	pSprite->OnMouseWheel( nFlags, zDelta, ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// マウスの左ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		bool	fResult =
			pSprite->OnLButtonDown( nFlags, ptLocal.x, ptLocal.y ) ;
		if ( (m_pFocus != pSprite) && (m_pFocus != NULL) )
		{
			SetFocus( NULL ) ;
		}
		return	fResult ;
	}
	return	false ;
}

// マウスの左ボタンが離された
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnLButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		return	pSprite->OnLButtonUp( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// マウスの左ボタンがダブルクリックされた
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnLButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		return	pSprite->OnLButtonDblClk( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// マウスの右ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnRButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		bool	fResult =
			pSprite->OnRButtonDown( nFlags, ptLocal.x, ptLocal.y ) ;
		if ( (m_pFocus != pSprite) && (m_pFocus != NULL) )
		{
			SetFocus( NULL ) ;
		}
		return	fResult ;
	}
	return	false ;
}

// マウスの右ボタンが離された
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnRButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		return	pSprite->OnRButtonUp( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// マウスの右ボタンがダブルクリックされた
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnRButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	if ( !m_fEnabled )
	{
		return	false ;
	}
	ESpriteInterface *	pSprite = m_pCaptured ;
	EGL_POINT	ptLocal = GlobalToLocal( EGLPoint( xPos, yPos ) ) ;
	if ( pSprite == NULL )
	{
		pSprite = GetSpriteAtPoint( ptLocal.x, ptLocal.y, NULL, true ) ;
	}
	if ( pSprite != NULL )
	{
		return	pSprite->OnRButtonDblClk( nFlags, ptLocal.x, ptLocal.y ) ;
	}
	return	false ;
}

// タイマー処理（タイマーフラグを持っているもののみ）
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::OnTimer( UINT nEventID )
{
	if ( (nEventID == 1) && (GetFunctionFlags() & ffTimer) )
	{
		unsigned int	i, nCount ;
		nCount = GetSpriteCount( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			ESpriteInterface *	pSprite =
				ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
			if ( pSprite != NULL )
			{
				if ( pSprite->GetFunctionFlags() & ffTimer )
				{
					pSprite->OnTimer( nEventID ) ;
				}
			}
		}
	}
	return	false ;
}

// メッセージ処理（フォーカスを持っているアイテムのみ）
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	bool	fKeyInput = (uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST) ;
	if ( (m_pFocus != NULL)
		&& (IsEnabledKeyInput() || !fKeyInput) )
	{
		if ( m_pFocus->IsEnabled()
			&& (m_pFocus->IsEnabledKeyInput() || !fKeyInput) )
		{
			return	m_pFocus->MessageProc( hWnd, uMsg, wParam, lParam ) ;
		}
	}
	return	false ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnCommand
	( ESpriteInterface * pItem,
		long int nNotification, long int nParameter,
		int nPriority, bool fOverwrite )
{
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		pParent->OnCommand
			( pItem, nNotification, nParameter, nPriority, fOverwrite ) ;
	}
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int ESpriteInterface::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	for ( int i = 0; i < dscParam.GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = dscParam.GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"basic_flag" )
		{
			if ( pdscTag->GetAttrString
				( L"hit_transparency", L"false" ) == L"true" )
			{
				m_dwFlags |= ftHitTransparency ;
			}
			else
			{
				m_dwFlags &= ~ftHitTransparency ;
			}
			if ( pdscTag->GetAttrString
				( L"modal_first", L"false" ) == L"true" )
			{
				m_dwFlags |= ftModalFirst ;
			}
			else
			{
				m_dwFlags &= ~ftModalFirst ;
			}
			if ( pdscTag->GetAttrString
				( L"modal_end", L"false" ) == L"true" )
			{
				m_dwFlags |= ftModalEnd ;
			}
			else
			{
				m_dwFlags &= ~ftModalEnd ;
			}
		}
		else if ( pdscTag->Tag() == L"input" )
		{
			EnableKeyInput
				( (pdscTag->GetAttrString( L"key",
					(IsEnabledKeyInput() ? L"true" : L"false") ) == L"true") ) ;
			EnableMouseWheel
				( (pdscTag->GetAttrString( L"wheel",
					(IsEnabledMouseWheel() ? L"true" : L"false") ) == L"true") ) ;
		}
	}
	return	0 ;
}

// マウスメッセージのキャプチャー設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetCapture( ESpriteInterface * pSprite )
{
	if ( m_pCaptured != NULL )
	{
		m_pCaptured->OnCaptureReleased( ) ;
		m_pCaptured = NULL ;
	}
	if ( pSprite != this )
	{
		m_pCaptured = pSprite ;
	}
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		pParent->SetCapture( this ) ;
	}
}

// キャプチャー解除
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::ReleaseCapture( ESpriteInterface * pSprite )
{
	if ( (m_pCaptured == pSprite) || (pSprite == NULL) )
	{
		//
		// 親アイテムにキャプチャー解除を通知
		//
		ESpriteInterface *	pParent =
			ESLTypeCast<ESpriteInterface>( GetParent() ) ;
		if ( pParent != NULL )
		{
			pParent->ReleaseCapture( NULL ) ;
		}
		else
		{
			//
			// 子アイテムにキャプチャー解除を通知
			//
			if ( m_pCaptured != NULL )
			{
				m_pCaptured->OnCaptureReleased( ) ;
			}
			m_pCaptured = NULL ;
		}
	}
}

// マウスのキャプチャーがリリースされた
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnCaptureReleased( void )
{
	if ( m_pCaptured != NULL )
	{
		ESpriteInterface *	pCaptured = m_pCaptured ;
		m_pCaptured = NULL ;
		pCaptured->OnCaptureReleased( ) ;
	}
}

// 指定されたアイテムの指標を取得する
//////////////////////////////////////////////////////////////////////////////
int ESpriteInterface::GetSpriteItemIndex( ESpriteInterface * pItem )
{
	unsigned int	i, nCount ;
	if ( pItem != NULL )
	{
		nCount = GetSpriteCount( ) ;
		ESprite *	pChild =
			m_itaSprite.GetAs( pItem->GetPriority(), &i ) ;
		while ( i > 0 )
		{
			pChild = GetSpriteAt( i - 1 ) ;
			if ( pChild->GetPriority() != pItem->GetPriority() )
				break ;
			i -- ;
		}
		while ( i < nCount )
		{
			ESpriteInterface *	pSprite =
				ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
			if ( pSprite == pItem )
				break ;
			i ++ ;
		}
		if ( i >= nCount )
		{
			i = 0 ;
		}
	}
	else
	{
		i = 0 ;
	}
	return	i ;
}

// フォーカスを設定する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetFocus( ESpriteInterface * pSprite )
{
	bool	fFocusThis = false ;
	if ( pSprite == this )
	{
		fFocusThis = true ;
		pSprite = NULL ;
	}
	if ( m_pFocus != pSprite )
	{
		if ( (m_pFocus != NULL) && (m_pFocus != pSprite) )
		{
			m_pFocus->OnKillFocus( ) ;
		}
		m_pFocus = pSprite ;
		if ( m_pFocus != NULL )
		{
			m_pFocus->OnSetFocus( ) ;
		}
	}
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		if ( (pSprite != NULL) || fFocusThis )
		{
			pParent->SetFocus( this ) ;
		}
		else
		{
			pParent->SetFocus( NULL ) ;
		}
	}
}

// フォーカスを解除する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::KillFocus( ESpriteInterface * pSprite )
{
	if ( pSprite != NULL )
	{
		pSprite->SetFocus( NULL ) ;
	}
	else
	{
		SetFocus( NULL ) ;
	}
}

// フォーカスを移動する
//////////////////////////////////////////////////////////////////////////////
bool ESpriteInterface::MoveFocus( bool fNext )
{
	//
	// フォーカスの設定されているアイテムのインデックスを取得する
	//
	signed int	i, nCount ;
	signed int	iFirst, iSearchCount ;
	nCount = GetSpriteCount( ) ;
	if ( m_pFocus != NULL )
	{
		if ( m_pFocus->IsEnabled() && m_pFocus->IsVisible() )
		{
			if ( m_pFocus->MoveFocus( fNext ) )
			{
				return	true ;
			}
			if ( (m_pFocus->GetFunctionFlags()
					& (ftModalFirst|ftModalEnd)) == (ftModalFirst|ftModalEnd) )
			{
				return	true ;
			}
		}
		iFirst = GetSpriteItemIndex( m_pFocus ) ;
	}
	else
	{
		iFirst = fNext ? -1 : nCount ;
	}
	//
	// 次のタブストップの設定されているアイテムを検索する
	//
	bool	fLoopFlag = false ;
	bool	fFocusFlag = false ;
	i = iFirst ;
	iSearchCount = 0 ;
	for ( ; ; )
	{
		if ( fNext )
		{
			if ( ++ i >= nCount )
			{
				i = 0 ;
				fLoopFlag = true ;
			}
		}
		else
		{
			if ( i > 0 )
			{
				i -- ;
			}
			else
			{
				i = nCount - 1 ;
				if ( i < 0 )
					i = 0 ;
				fLoopFlag = true ;
			}
		}
		if ( iSearchCount ++ >= nCount )
		{
			break ;
		}
		ESpriteInterface *	pSprite =
			ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
		if ( pSprite != NULL )
		{
			if ( pSprite->IsEnabled() && pSprite->IsVisible()
					&& (pSprite->GetFunctionFlags() & ffTabStop) )
			{
				//
				// フォーカス設定
				//
				SetFocus( pSprite ) ;
				if ( pSprite->m_pFocus == NULL )
				{
					pSprite->MoveFocus( fNext ) ;
				}
				fFocusFlag = !fLoopFlag ;
				break ;
			}
		}
	}
	return	fFocusFlag ;
}

// 同じグループに属する次のアイテムを取得する
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * ESpriteInterface::GetNextItemOnGroup
					( ESpriteInterface * pItem, bool fNext )
{
	//
	// 指定アイテムのインデックスを取得する
	//
	signed int	i, nCount ;
	nCount = GetSpriteCount( ) ;
	i = GetSpriteItemIndex( pItem ) ;
	if ( !fNext )
	{
		ESpriteInterface *	pSprite =
			ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
		if ( pSprite != NULL )
		{
			if ( pSprite->GetFunctionFlags() & ffGroup )
			{
				return	NULL ;
			}
		}
	}
	//
	// 次のアイテムを取得する
	//
	for ( ; ; )
	{
		if ( fNext )
		{
			if ( ++ i >= nCount )
			{
				return	NULL ;
			}
		}
		else
		{
			if ( i > 0 )
			{
				i -- ;
			}
			else
			{
				return	NULL ;
			}
		}
		ESpriteInterface *	pSprite =
			ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
		if ( pSprite != NULL )
		{
			if ( pSprite->GetFunctionFlags() & ffGroup )
			{
				if ( !fNext )
				{
					return	pSprite ;
				}
				return	NULL ;
			}
			return	pSprite ;
		}
	}
	//
	return	NULL ;
}

// フォーカスを取得した
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnSetFocus( void )
{
	if ( m_pFocus != NULL )
	{
		m_pFocus->OnSetFocus( ) ;
	}
}

// フォーカスを奪われた
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::OnKillFocus( void )
{
	if ( m_pFocus != NULL )
	{
		m_pFocus->OnKillFocus( ) ;
	}
}

// マウスカーソルを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::SetMouseCursor( const wchar_t * pwszID )
{
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		return	pParent->SetMouseCursor( pwszID ) ;
	}
	return	eslErrGeneral ;
}

// 効果音を再生する
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::PlaySoundEffect
	( const wchar_t * pwszID, bool fRepeat )
{
	ESpriteInterface *	pParent =
		ESLTypeCast<ESpriteInterface>( GetParent() ) ;
	if ( pParent != NULL )
	{
		return	pParent->PlaySoundEffect( pwszID, fRepeat ) ;
	}
	return	eslErrGeneral ;
}

// 現在の垂直スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int ESpriteInterface::GetVertScrollPos( void ) const
{
	return	0 ;
}

// 現在の垂直スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetVertScrollPos( int nPos )
{
}

// 垂直スクロールの範囲を取得
//////////////////////////////////////////////////////////////////////////////
int ESpriteInterface::GetVertScrollRange( void ) const
{
	return	0 ;
}

// 垂直スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetVertScrollRange( int nRange )
{
}

// 現在の水平スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int ESpriteInterface::GetHorzScrollPos( void ) const
{
	return	0 ;
}

// 現在の水平スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetHorzScrollPos( int nPos )
{
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
int ESpriteInterface::GetHorzScrollRange( void ) const
{
	return	0 ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetHorzScrollRange( int nRange )
{
}

// 指定座標にヒットしているスプライトを取得
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface *
	ESpriteInterface::GetSpriteAtPoint
		( int xPos, int yPos, ESpriteInterface * pFirst, bool fEnabledOnly )
{
	unsigned int	i, iFirst, nCount ;
	iFirst = 0 ;
	nCount = GetSpriteCount( ) ;
	//
	if ( pFirst != NULL )
	{
		//
		// 1つ奥のプライオリティのスプライトのインデックスを取得
		//
		nCount = GetSpriteCount( ) ;
		ESprite *	pChild =
			m_itaSprite.GetAs( pFirst->GetPriority(), &i ) ;
		while ( iFirst > 0 )
		{
			pChild = GetSpriteAt( iFirst - 1 ) ;
			if ( pChild->GetPriority() != m_pFocus->GetPriority() )
				break ;
			iFirst -- ;
		}
		while ( iFirst < nCount )
		{
			ESpriteInterface *	pSprite =
				ESLTypeCast<ESpriteInterface>( GetSpriteAt( iFirst ++ ) ) ;
			if ( pSprite == pFirst )
				break ;
		}
	}
	//
	// 順次スプライトの当たり判定を実施
	//
	for ( i = iFirst; i < nCount; i ++ )
	{
		ESpriteInterface *	pSprite =
			ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
		if ( pSprite != NULL )
		{
			if ( pSprite->IsVisible()
					&& (!fEnabledOnly || pSprite->IsEnabled()) )
			{
				if ( pSprite->IsHitSprite( xPos, yPos ) )
				{
					return	pSprite ;
				}
			}
		}
	}
	//
	return	NULL ;
}

// ローカル座標をウィンドウのクライアント座標に変換する
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::LocalToWindowClient( EGL_POINT & ptLocal )
{
	ESpriteInterface *	pParent = this ;
	for ( ; ; )
	{
		ptLocal = pParent->LocalToGlobal( ptLocal ) ;
		pParent = ESLTypeCast<ESpriteInterface>( pParent->GetParent() ) ;
		if ( pParent == NULL )
		{
			return	eslErrGeneral ;
		}
		if ( pParent->IsKindOf( ESL_RUNTIME_CLASS(EWindowInterface) ) )
		{
			EWindowSpriteInterface *
				pWndInterface = ESLTypeCast<EWindowSpriteInterface>( pParent ) ;
			if ( pWndInterface != NULL )
			{
				pWndInterface->ClientToWindow( ptLocal ) ;
			}
			return	eslErrSuccess ;
		}
	}
}

// ウィンドウインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
EWindowSpriteInterface * ESpriteInterface::GetWindowInterface( void )
{
	ESpriteInterface *	pParent = this ;
	for ( ; ; )
	{
		EWindowSpriteInterface *	pWindow =
			ESLTypeCast<EWindowSpriteInterface>( pParent ) ;
		if ( pWindow != NULL )
		{
			return	pWindow ;
		}
		pParent = ESLTypeCast<ESpriteInterface>( pParent->GetParent() ) ;
		if ( pParent == NULL )
		{
			return	NULL ;
		}
	}
}

// スプライトを追加
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::AddSprite( int nPriority, ESprite * pSprite )
{
	ESpriteInterface *	pItem = ESLTypeCast<ESpriteInterface>( pSprite ) ;
	if ( pItem != NULL )
	{
		if ( !pItem->ID().IsEmpty() )
		{
			if ( m_wstaItems.GetAs( pItem->ID() ) == NULL )
			{
				m_wstaItems.Add( pItem->ID(), pItem ) ;
			}
		}
	}
	ESpriteServer::AddSprite( nPriority, pSprite ) ;
}

// スプライトを分離
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::DetachSprite( ESprite * pSprite )
{
	ESpriteInterface *	pItem = ESLTypeCast<ESpriteInterface>( pSprite ) ;
	if ( pItem != NULL )
	{
		if ( m_pFocus == pItem )
		{
			m_pFocus->OnKillFocus( ) ;
			m_pFocus = NULL ;
		}
		if ( m_pMouseFocus == pItem )
		{
			m_pMouseFocus->OnMouseLeave( 0, 0, 0 ) ;
			m_pMouseFocus = NULL ;
		}
		if ( m_pCaptured == pItem )
		{
			ReleaseCapture( pItem ) ;
		}
		if ( !pItem->ID().IsEmpty() )
		{
			m_wstaItems.DetachAs( pItem->ID() ) ;
		}
	}
	return	ESpriteServer::DetachSprite( pSprite ) ;
}

// 全てのスプライトを分離
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::DetachAllSprite( void )
{
	if ( m_pFocus != NULL )
	{
		m_pFocus->OnKillFocus( ) ;
		m_pFocus = NULL ;
	}
	if ( m_pMouseFocus != NULL )
	{
		m_pMouseFocus->OnMouseLeave( 0, 0, 0 ) ;
		m_pMouseFocus = NULL ;
	}
	if ( m_pCaptured != NULL )
	{
		ReleaseCapture( NULL ) ;
	}
	m_wstaItems.DetachAll( ) ;
	ESpriteServer::DetachAllSprite( ) ;
}

// スプライトを削除
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::RemoveSprite( ESprite * pSprite )
{
	ESpriteInterface *	pItem = ESLTypeCast<ESpriteInterface>( pSprite ) ;
	if ( pItem != NULL )
	{
		if ( m_pFocus == pItem )
		{
			m_pFocus->OnKillFocus( ) ;
			m_pFocus = NULL ;
		}
		if ( m_pMouseFocus == pItem )
		{
			m_pMouseFocus->OnMouseLeave( 0, 0, 0 ) ;
			m_pMouseFocus = NULL ;
		}
		if ( m_pCaptured == pItem )
		{
			ReleaseCapture( pItem ) ;
		}
		if ( !pItem->ID().IsEmpty() )
		{
			m_wstaItems.DetachAs( pItem->ID() ) ;
		}
	}
	return	ESpriteServer::RemoveSprite( pSprite ) ;
}

// 全てのスプライトを削除
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::RemoveAllSprite( void )
{
	if ( m_pFocus != NULL )
	{
		m_pFocus->OnKillFocus( ) ;
		m_pFocus = NULL ;
	}
	if ( m_pMouseFocus != NULL )
	{
		m_pMouseFocus->OnMouseLeave( 0, 0, 0 ) ;
		m_pMouseFocus = NULL ;
	}
	if ( m_pCaptured != NULL )
	{
		ReleaseCapture( NULL ) ;
	}
	m_wstaItems.DetachAll( ) ;
	ESpriteServer::RemoveAllSprite( ) ;
}

// 機能フラグを設定する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetFunctionFlags( DWORD dwFlags )
{
	m_dwFlags = dwFlags ;
}

// 画面アイテムの基準プライオリティを設定する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::SetBaseItemPriority( int nPriority )
{
	m_nNextPriority = nPriority ;
}

// 画面アイテムを追加する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::AddSpriteItem
	( const wchar_t * pwszID, ESpriteInterface * pSprite )
{
	pSprite->SetID( pwszID ) ;
	AddSprite( m_nNextPriority, pSprite ) ;
	m_nNextPriority += 0x10 ;
	m_dwFlags |= (pSprite->m_dwFlags & ffTabStop) ;
}

// 画面アイテムを取得する
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * ESpriteInterface::GetSpriteItemAs
	( const wchar_t * pwszID, bool fChild )
{
	ESpriteInterface *	pSprite = m_wstaItems.GetAs( pwszID ) ;
	if ( (pSprite == NULL) && fChild )
	{
		for ( int i = 0; i < (int) GetSpriteCount(); i ++ )
		{
			pSprite = ESLTypeCast<ESpriteInterface>( GetSpriteAt( i ) ) ;
			if ( pSprite != NULL )
			{
				pSprite = pSprite->GetSpriteItemAs( pwszID, fChild ) ;
				if ( pSprite != NULL )
					break ;
			}
		}
	}
	return	pSprite ;
}

// アイテムの文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::GetSpriteItemText
	( const wchar_t * pwszID, EWideString & wstrText )
{
	ESpriteInterface *	pSprite = GetSpriteItemAs( pwszID ) ;
	if ( pSprite == NULL )
		return	eslErrGeneral ;
	//
	wstrText = pSprite->GetSpriteText( ) ;
	return	eslErrSuccess ;
}

// アイテムの文字列設定
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::SetSpriteItemText
	( const wchar_t * pwszID, const wchar_t * pwszText )
{
	ESpriteInterface *	pSprite = GetSpriteItemAs( pwszID ) ;
	if ( pSprite == NULL )
		return	eslErrGeneral ;
	//
	pSprite->SetSpriteText( pwszText ) ;
	return	eslErrSuccess ;
}

// アイテムのフォント設定
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::SetSpriteItemFontFace
	( const wchar_t * pwszID, const wchar_t * pwszFont )
{
	ESpriteInterface *	pSprite = GetSpriteItemAs( pwszID ) ;
	if ( pSprite == NULL )
		return	eslErrGeneral ;
	//
	pSprite->SetSpriteFontFace( pwszFont ) ;
	return	eslErrSuccess ;
}

// ボタンのチェック状態を取得
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::IsSpriteButtonChecked
	( const wchar_t * pwszID, int & nStatus )
{
	EButtonSprite *	pSprite =
		ESLTypeCast<EButtonSprite>( GetSpriteItemAs( pwszID ) ) ;
	if ( pSprite == NULL )
		return	eslErrGeneral ;
	//
	nStatus = pSprite->IsButtonChecked( ) ;
	return	eslErrSuccess ;
}

// ボタンをチェックする
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteInterface::CheckSpriteButton
	( const wchar_t * pwszID, bool fCheck )
{
	EButtonSprite *	pSprite =
		ESLTypeCast<EButtonSprite>( GetSpriteItemAs( pwszID ) ) ;
	if ( pSprite == NULL )
		return	eslErrGeneral ;
	//
	pSprite->CheckButton( fCheck ) ;
	return	eslErrSuccess ;
}

// キー入力を有効・無効化する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::EnableKeyInput( bool fKeyInput )
{
	m_fEnabledKeyInput = fKeyInput ;
}

// マウスホイール入力を有効・無効化する
//////////////////////////////////////////////////////////////////////////////
void ESpriteInterface::EnableMouseWheel( bool fWheel )
{
	m_fEnabledMouseWheel = fWheel ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ画面入出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWndSpriteCmd, ESLObject )
IMPLEMENT_CLASS_INFO2
	( EWindowSpriteInterface, ESpriteInterface, EWindowInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWindowSpriteInterface::EWindowSpriteInterface( void )
{
	m_hMutex = ::CreateMutex( NULL, FALSE, NULL ) ;
//	::InitializeCriticalSection( &m_cs ) ;
	m_idMsgHandlerThread = 0 ;
	m_nMsgHandlerLocked = 0 ;
	m_fMsgHandlerUnlocked = 0 ;
	m_hSyncTimePaint = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hWaitTimePaint = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
	m_hPaintSignal = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_pSyncTarget = NULL ;
	//
	m_hUser32 = ::LoadLibrary( "user32.dll" ) ;
	m_apiTrackMouseEvent = NULL ;
	m_apiUpdateLayeredWindow = NULL ;
	m_fMouseLeaved = true ;
	m_fLayeredWindow = false ;
	m_fPendingUpdateWindow = false ;
	m_fNoInvalidateWindow = false ;
	m_fProcMsg = false ;
	if ( m_hUser32 != NULL )
	{
		m_apiTrackMouseEvent = (API_TrackMouseEvent)
			::GetProcAddress( m_hUser32, "TrackMouseEvent" ) ;
		m_apiUpdateLayeredWindow = (API_UpdateLayeredWindow)
			::GetProcAddress( m_hUser32, "UpdateLayeredWindow" ) ;
	}
	m_fQueueCommand = false ;
	m_hQueCmdEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_pFilter = NULL ;
	m_fImageStretching = false ;
	m_pDrawImage = NULL ;
	m_pddsVRAM = NULL ;
	m_fStretchingByCPU = false ;
	for ( int i = 0; i < countMaxDrawStretchCPUs; i ++ )
	{
		m_hDrawStretch[i] = NULL ;
	}
	//
#if	_MSC_VER >= 1800
	if ( IsWindowsVistaOrGreater() )
#else
	OSVERSIONINFO	osvi ;
	osvi.dwOSVersionInfoSize = sizeof(osvi) ;
	::GetVersionEx( &osvi ) ;
	if ( (osvi.dwMajorVersion == 6)
		&& (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) )
#endif
	{
		m_fStretchingByCPU = true ;
	}
	//
	m_pAttachedView = NULL ;
	m_pivView3D = NULL ;
	m_nFreezePaint = 0 ;
	m_countNullDraw = 0 ;
	//
	m_flagLayout = 0 ;
	m_ptLayoutOffset.x = 0 ;
	m_ptLayoutOffset.y = 0 ;
	//
	::eslFillMemory( &m_bgiFrame, 0, sizeof(m_bgiFrame) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWindowSpriteInterface::~EWindowSpriteInterface( void )
{
	::CloseHandle( m_hMutex ) ;
//	::DeleteCriticalSection( &m_cs ) ;
	::CloseHandle( m_hSyncTimePaint ) ;
	::CloseHandle( m_hWaitTimePaint ) ;
	::CloseHandle( m_hPaintSignal ) ;
	::CloseHandle( m_hQueCmdEvent ) ;
	//
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->DetachNotify( this ) ;
	}
	if ( m_pddsVRAM != NULL )
	{
		m_pddsVRAM->Release( ) ;
		m_pddsVRAM = NULL ;
	}
	for ( int i = 0; i < countMaxDrawStretchCPUs; i ++ )
	{
		if ( m_hDrawStretch[i] != NULL )
		{
			m_hDrawStretch[i]->Release() ;
			m_hDrawStretch[i] = NULL ;
		}
	}
	if ( m_hUser32 != NULL )
	{
		::FreeLibrary( m_hUser32 ) ;
	}
}

// フィルター設定
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::SetInputFilter( EInputFilter * pFilter )
{
	Lock( ) ;
	m_pFilter = pFilter ;
	Unlock( ) ;
}

// 表示用リダイレクトスプライトの関連付け
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::AttachView( ESpriteInterface * pView )
{
	m_pAttachedView = pView ;
	//
	if ( pView != NULL )
	{
		AddSprite( 0, pView ) ;
		pView->SetVisible( true ) ;
		AttachImage( pView->GetInfo() ) ;
	}
}

// 表示用リダイレクトスプライトの解除
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::DetachView( ESpriteInterface * pView )
{
	if ( m_pAttachedView == pView )
	{
		DetachSprite( pView ) ;
		AttachImage( NULL ) ;
		m_pAttachedView = NULL ;
	}
}

// 表示用立体視インターフェースの関連付け
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::Attach3DViewDisplay
		( E3DSDisplayPlugin::I3DImageView * pivView3D )
{
	m_pivView3D = pivView3D ;
}

// EWindowSpriteInterface での描画抑制
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::AddNullficationDraw( void )
{
	::InterlockedIncrement( &m_countNullDraw ) ;
}

void EWindowSpriteInterface::ReleaseNullficationDraw( void )
{
	::InterlockedDecrement( &m_countNullDraw ) ;
}

// ウィンドウの更新領域通知抑制
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::ConntrolAutoUpdate( bool fNoUpdate )
{
	m_fNoInvalidateWindow = fNoUpdate ;
}

// EWindow オブジェクトにアタッチされた
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnAttachedWindow( EWindow * pAttachedWnd )
{
	EWindowInterface::OnAttachedWindow( pAttachedWnd ) ;
	m_dwLastTimerTick = ::timeGetTime( ) ;
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EWindowSpriteInterface::WindowProc
	( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_idMsgHandlerThread = ::GetCurrentThreadId( ) ;
	//
	if ( (uMsg == WM_PAINT) || (uMsg == WM_TIMER) )
	{
		::SetEvent( m_hSyncTimePaint ) ;
		::WaitForSingleObject( m_hWaitTimePaint, 100 ) ;
	}
	MSG		msg ;
	msg.hwnd = *pWnd ;
	msg.message = uMsg ;
	msg.wParam = wParam ;
	msg.lParam = lParam ;
	msg.time = 0 ;
	msg.pt.x = 0 ;
	msg.pt.y = 0 ;
	//
	Lock( ) ;
	//
	EInputFilter *	pFilter = m_pFilter ;
	if ( (pFilter == NULL) && (m_pSyncTarget != NULL) )
	{
		pFilter = m_pSyncTarget->m_pFilter ;
	}
	ESpriteInterface *	pAttachedView = m_pAttachedView ;
	if ( pAttachedView == NULL )
	{
		pAttachedView = this ;
	}
	if ( uMsg == WM_PAINT )
	{
		//
		// 画面描画
		//
		PAINTSTRUCT	ps ;
		HDC	hdc = pWnd->BeginPaint( &ps ) ;
		if ( m_countNullDraw <= 0 )
		{
			if ( m_fLayeredWindow )
			{
				UpdateLayeredWindow( ) ;
			}
			else
			{
				OnPaint( pWnd, hdc ) ;
			}
		}
		pWnd->EndPaint( &ps ) ;
		::SetEvent( m_hPaintSignal ) ;
		Unlock( ) ;
		//
		return	0 ;
	}
	else if ( (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) )
	{
		//
		// マウスメッセージ
		//
		bool	fProcessed = false ;
		int		xPos, yPos ;
		UINT	nFlags = wParam ;
		xPos = (SWORD) lParam ;
		yPos = (SWORD) (lParam >> 16) ;
		EGL_POINT	ptCursor ;
		//
		switch ( uMsg )
		{
		case	WM_MOUSEMOVE:
			// マウスが移動した
			if ( m_fMouseLeaved )
			{
				if ( m_apiTrackMouseEvent != NULL )
				{
					TRACKMOUSEEVENT	tmeEvent ;
					tmeEvent.cbSize = sizeof(tmeEvent) ;
					tmeEvent.dwFlags = TME_LEAVE ;
					tmeEvent.hwndTrack = *pWnd ;
					(*m_apiTrackMouseEvent)( &tmeEvent ) ;
				}
				m_fMouseLeaved = false ;
			}
			pAttachedView->OnMouseMove( nFlags, xPos, yPos ) ;
			break ;

		case	WM_MOUSEWHEEL:
			// マウスホイールが回転した
			ptCursor.x = xPos ;
			ptCursor.y = yPos ;
			ScreenToClient( ptCursor ) ;
			fProcessed = pAttachedView->OnMouseWheel
				( (WORD) wParam, (SWORD) (wParam >> 16),
									ptCursor.x, ptCursor.y ) ;
			break ;

		case	WM_LBUTTONDOWN:
			// 左ボタンが押された
			fProcessed = pAttachedView->OnLButtonDown( nFlags, xPos, yPos ) ;
			break ;

		case	WM_LBUTTONUP:
			// 左ボタンが離された
			fProcessed = pAttachedView->OnLButtonUp( nFlags, xPos, yPos ) ;
			//
			if ( pFilter && fProcessed )
			{
				pFilter->ProcessMessage( msg ) ;
			}
			break ;

		case	WM_LBUTTONDBLCLK:
			// 左ボタンがダブルクリックされた
			fProcessed = pAttachedView->OnLButtonDblClk( nFlags, xPos, yPos ) ;
			break ;

		case	WM_RBUTTONDOWN:
			// 右ボタンが押された
			fProcessed = pAttachedView->OnRButtonDown( nFlags, xPos, yPos ) ;
			break ;

		case	WM_RBUTTONUP:
			// 右ボタンが離された
			fProcessed = pAttachedView->OnRButtonUp( nFlags, xPos, yPos ) ;
			//
			if ( pFilter && fProcessed )
			{
				pFilter->ProcessMessage( msg ) ;
			}
			break ;

		case	WM_RBUTTONDBLCLK:
			// 右ボタンがダブルクリックされた
			fProcessed = pAttachedView->OnRButtonDblClk( nFlags, xPos, yPos ) ;
			break ;
		}
		//
		if ( fProcessed )
		{
			Unlock( ) ;
			return	0 ;
		}
	}
	else if ( uMsg == WM_SETCURSOR )
	{
		//
		// マウスカーソル設定
		//
		if ( LOWORD(lParam) == HTCLIENT )
		{
			DWORD	dwMsgPos = ::GetMessagePos( ) ;
			EGL_POINT	ptCursor ;
			ptCursor.x = (SWORD) dwMsgPos ;
			ptCursor.y = (SWORD) (dwMsgPos >> 16) ;
			ScreenToClient( ptCursor ) ;
			//
			if ( OnSetCursor( ptCursor.x, ptCursor.y ) )
			{
				Unlock( ) ;
				return	0 ;
			}
		}
//		Unlock( ) ;
//		return	0 ;
	}
	else if ( uMsg == WM_TIMER )
	{
		//
		// タイマー
		//
		DWORD	dwCurrentTime = ::timeGetTime( ) ;
		DWORD	dwOffsetTime = dwCurrentTime - m_dwLastTimerTick ;
		if ( dwOffsetTime > 5 )
		{
			if ( dwOffsetTime > 10000 )
			{
				dwOffsetTime = 10000 ;
			}
			m_dwLastTimerTick = dwCurrentTime ;
			pAttachedView->OnAdvanceAnimation( dwOffsetTime ) ;
		}
		if ( pAttachedView->OnTimer( wParam ) )
		{
			if ( pFilter )
			{
				pFilter->ProcessMessage( msg ) ;
			}
			Unlock( ) ;
			return	0 ;
		}
	}
	else if ( uMsg == WM_MOUSELEAVE )
	{
		//
		// マウスがウィンドウの外に移動した
		//
		m_fMouseLeaved = true ;
		CallMouseMove( ) ;
		//
		if ( pFilter )
		{
			pFilter->ProcessMessage( msg ) ;
		}
		Unlock( ) ;
		return	0 ;
	}
	else if ( uMsg == WM_KILLFOCUS )
	{
		//
		// フォーカスが奪われた
		//
		pAttachedView->OnKillFocus( ) ;
	}
	else if ( uMsg == WM_SETFOCUS )
	{
		//
		// フォーカスが設定された
		//
		pAttachedView->OnSetFocus( ) ;
	}
	else if ( uMsg == WM_ACTIVATE )
	{
		//
		// ウィンドウがアクティブ・非アクティブになった
		//
		if ( LOWORD(wParam) == WA_INACTIVE )
		{
			pAttachedView->ReleaseCapture( ) ;
		}
	}
	else if ( uMsg == wmProcessMessage )
	{
		Unlock( ) ;
		if ( !m_fProcMsg )
		{
			MSG	msg ;
			int	i, nCount = wParam ;
			m_fProcMsg = true ;
			for ( i = 0; i < nCount; i ++ )
			{
				if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					::TranslateMessage( &msg ) ;
					::DispatchMessage( &msg ) ;
				}
				else
				{
					break ;
				}
			}
			m_fProcMsg = false ;
		}
		return	0 ;
	}
	else if ( uMsg == wmCallProcedure )
	{
		Unlock( ) ;
		PFUNC_PROCEDURE	pfnProc = (PFUNC_PROCEDURE) wParam ;
		if ( pfnProc != NULL )
		{
			return	pfnProc( (void*) lParam ) ;
		}
		return	0 ;
	}
	else if ( uMsg == wmNotifyLayout )
	{
		Unlock() ;
		OnNotifyLayout( (HWND) lParam ) ;
		return	0 ;
	}
	else if ( uMsg == WM_CREATE )
	{
		//
		// ウィンドウ初期化処理
		//
		pWnd->SetTimer( 1, 16, NULL ) ;
	}
	else if ( uMsg == WM_MOVE )
	{
		PostNotifyRelativeLayout() ;
	}
	else if ( uMsg == WM_SIZE )
	{
		if ( (m_pDrawImage != NULL)
			&& (m_pDrawImage->GetDirect3DDevice9() != NULL) )
		{
			m_pDrawImage->OnResizeWindow() ;
		}
		PostNotifyRelativeLayout() ;
	}
	//
	// 汎用ウィンドウメッセージ
	//
	if ( pAttachedView->MessageProc( (HWND) *pWnd, uMsg, wParam, lParam ) )
	{
		if ( (uMsg == WM_KEYUP) && pFilter )
		{
			pFilter->ProcessMessage( msg ) ;
		}
		Unlock( ) ;
		return	0 ;
	}
	//
	// 入力フィルタ
	//
	if ( pFilter )
	{
		pFilter->ProcessMessage( msg ) ;
	}
	Unlock( ) ;
	//
	// デフォルトの処理
	//
	return	EWindowInterface::WindowProc( pWnd, uMsg, wParam, lParam ) ;
}

// メッセージ事前変換関数
//////////////////////////////////////////////////////////////////////////////
int EWindowSpriteInterface::PreTranslateMessage( MSG & msg )
{
	if ( m_fImageStretching )
	{
		UINT	uMsg = msg.message ;
		if ( (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) )
		{
			if ( uMsg != WM_MOUSEWHEEL )
			{
				EGL_POINT	ptCursor ;
				ptCursor.x = (SWORD) (msg.lParam) ;
				ptCursor.y = (SWORD) (msg.lParam >> 16) ;
				WindowToClient( ptCursor ) ;
				msg.lParam = (ptCursor.x & 0xFFFF)
							| ((ptCursor.y & 0xFFFF) << 16) ;
			}
		}
	}
	return	0 ;
}

// メッセージ処理
//////////////////////////////////////////////////////////////////////////////
bool EWindowSpriteInterface::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	//
	// フォーカスを持っているアイテムの処理
	//
	if ( ESpriteInterface::MessageProc( hWnd, uMsg, wParam, lParam ) )
	{
		return	true ;
	}
	//
	// 規定のキーボード操作
	//
	if ( uMsg == WM_KEYDOWN )
	{
		if ( wParam == VK_TAB )
		{
			//
			// フォーカスアイテムを移動
			//
			if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
			{
				MoveFocus( false ) ;
			}
			else
			{
				MoveFocus( true ) ;
			}
			return	true ;
		}
	}
	//
	return	false ;
}

// プラグイン出力用バッファ変換
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::ConvertToE3DSDisplayImageBuffer
	( E3DSDisplayPlugin::ImageBuffer & bufImage, PCEGL_IMAGE_INFO pImage )
{
	bufImage.ConvertFrom( *pImage ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnPaint( EWindow * pWnd, HDC hdc, bool fVSync )
{
	ESpriteInterface *	pView = this ;
	if ( m_pAttachedView != NULL )
	{
		pView = m_pAttachedView ;
	}
	if ( pView->GetInfo() != NULL )
	{
		if ( m_nFreezePaint == 0 )
		{
			Refresh( ) ;
		}
		if ( m_pivView3D != NULL )
		{
			using namespace E3DSDisplayPlugin ;
			ImageBuffer			bufImages[2] ;
			const ImageBuffer *	pbufImages[2] =
			{
				&bufImages[0], &bufImages[1]
			} ;
			ConvertToE3DSDisplayImageBuffer
				( bufImages[stereoRightBuffer], pView->GetInfo() ) ;
			int	nViewCount = 1 ;
			//
			if ( pView->IsEnabledStereoView() )
			{
				ConvertToE3DSDisplayImageBuffer
					( bufImages[stereoLeftBuffer],
										pView->GetStereoLeftBuffer() ) ;
				nViewCount = 2 ;
			}
			bool	fSuccessful = false ;
			//
			m_pivView3D->AttachThread() ;
			//
			if ( !m_pivView3D->DrawBuffer
				( drawDynamic | drawTemporary,
						0, 0, pbufImages, nViewCount ) )
			{
				if ( !m_pivView3D->PrepareView() )
				{
					if ( !m_pivView3D->ViewImage
							( fVSync ? viewWaitVSync : viewNoWait ) )
					{
						fSuccessful = true ;
					}
				}
				else
				{
					ESLTrace( "Failed to I3DImageView::PrepareView\n" ) ;
				}
			}
			else
			{
				ESLTrace( "Failed to I3DImageView::DrawBuffer\n" ) ;
			}
			m_pivView3D->DetachThread() ;
			//
			if ( !fSuccessful )
			{
				ESLTrace( "Failed to view stereo image.\n" ) ;
				OnPaintImage( pWnd, hdc, pView, fVSync ) ;
			}
		}
		else
		{
			OnPaintImage( pWnd, hdc, pView, fVSync ) ;
		}
	}
}

void EWindowSpriteInterface::OnPaintImage
		( EWindow * pWnd, HDC hdc, EGLImage * pImage, bool fVSync )
{
	if ( pWnd->GetStyle() & WS_MINIMIZE )
	{
		return ;
	}
	//
	// 画像描画
	//
	if ( (m_pDrawImage != NULL)
		&& (m_pDrawImage->GetDirect3DDevice9() != NULL) )
	{
		if ( !DirectDrawImage( pWnd, pImage->GetInfo(), fVSync ) )
		{
			return ;
		}
	}
	EGL_IMAGE_RECT	irDraw ;
	if ( m_fImageStretching )
	{
		EGLSize	sizeImage = pImage->GetSize( ) ;
		if ( sizeImage != m_szScreenStretched )
		{
			if ( !m_fStretchingByCPU )
			{
				if ( DirectDrawImage( pWnd, pImage->GetInfo(), fVSync ) )
				{
					::SetStretchBltMode( hdc, COLORONCOLOR ) ;
					pImage->DrawToDC
						( hdc, m_ptScreenBase.x, m_ptScreenBase.y,
										&m_szScreenStretched, NULL ) ;
				}
			}
			else
			{
				unsigned int	nThreads = SSystem::g_cpuLogicalCount ;
				if ( nThreads > countMaxDrawStretchCPUs )
				{
					nThreads = countMaxDrawStretchCPUs ;
				}
				ESLAssert( nThreads != 0 ) ;
				if ( nThreads == 0 )
				{
					nThreads = 1 ;
				}
				ParallelStretchDraw::Param	psdParam[countMaxDrawStretchCPUs] ;
				void *						ppsdParam[countMaxDrawStretchCPUs] ;
				unsigned int	i ;
				for ( i = 0; i < nThreads; i ++ )
				{
					if ( m_hDrawStretch[i] == NULL )
					{
						m_hDrawStretch[i] = ::eglCreateDrawImage() ;
					}
					ppsdParam[i] = &psdParam[i] ;
					psdParam[i].hDraw = m_hDrawStretch[i] ;
				}
				if ( (m_imgStretchBuffer.GetInfo() == NULL)
					|| (m_imgStretchBuffer.GetSize() != m_szScreenStretched) )
				{
					m_imgStretchBuffer.CreateImage
						( EIF_RGB_BITMAP,
							m_szScreenStretched.w,
							m_szScreenStretched.h, 32, EGL_IMAGE_HAS_DC ) ;
					m_imgStretchBuffer.ReverseVertically() ;
				}
				int	yBlockHeight =
						(m_szScreenStretched.h + nThreads - 1) / nThreads ;
				//
				ParallelStretchDraw
						psd( m_imgStretchBuffer, pImage, yBlockHeight ) ;
				psd.Start( &ppsdParam[0], nThreads ) ;
				/*
				EGL_DRAW_PARAM	dp ;
				EGL_IMAGE_AXES	iax ;
				memset( &dp, 0, sizeof(dp) ) ;
				dp.dwFlags = EGL_SMOOTH_STRETCH | EGL_DRAW_A_MOVE ;
				dp.pSrcImage = pImage->GetInfo() ;
				dp.pImageAxes = &iax ;
				int	nSrcWidth = sizeImage.w - 1 ;
				int	nSrcHeight = sizeImage.h - 1 ;
				int	nDstWidth = m_szScreenStretched.w - 1 ;
				int	nDstHeight = m_szScreenStretched.h - 1 ;
				if ( (nSrcWidth == 0) || (nSrcHeight == 0) )
				{
					nSrcWidth = sizeImage.w ;
					nSrcHeight = sizeImage.h ;
					nDstWidth = m_szScreenStretched.w ;
					nDstHeight = m_szScreenStretched.h ;
				}
				iax.xAxis.x = (REAL32) ((double) nDstWidth / nSrcWidth) ;
				iax.xAxis.y = 0.0 ;
				iax.yAxis.x = 0.0 ;
				iax.yAxis.y = (REAL32) ((double) nDstHeight / nSrcHeight) ;
				//
				m_hDrawStretch->Initialize( m_imgStretchBuffer, NULL, NULL ) ;
				if ( !m_hDrawStretch->PrepareDraw( &dp ) )
				{
					m_hDrawStretch->DrawImage() ;
				}
				*/
				m_imgStretchBuffer.DrawToDC
					( hdc, m_ptScreenBase.x, m_ptScreenBase.y, NULL, NULL ) ;
			}
			irDraw.x = m_ptScreenBase.x ;
			irDraw.y = m_ptScreenBase.y ;
			irDraw.w = m_szScreenStretched.w ;
			irDraw.h = m_szScreenStretched.h ;
		}
		else
		{
			::SetStretchBltMode( hdc, COLORONCOLOR ) ;
			pImage->DrawToDC
				( hdc, m_ptScreenBase.x, m_ptScreenBase.y,
												NULL, NULL ) ;
			//
			irDraw.x = m_ptScreenBase.x ;
			irDraw.y = m_ptScreenBase.y ;
			irDraw.w = sizeImage.w ;
			irDraw.h = sizeImage.h ;
		}
	}
	else
	{
		if ( fVSync && m_pDrawImage )
		{
			if ( DirectDrawImage( pWnd, pImage->GetInfo(), fVSync ) )
			{
				::SetStretchBltMode( hdc, COLORONCOLOR ) ;
				pImage->DrawToDC( hdc, 0, 0, NULL, NULL ) ;
			}
		}
		else
		{
			::SetStretchBltMode( hdc, COLORONCOLOR ) ;
			pImage->DrawToDC( hdc, 0, 0, NULL, NULL ) ;
		}
		irDraw.x = 0 ;
		irDraw.y = 0 ;
		irDraw.w = pImage->GetWidth() ;
		irDraw.h = pImage->GetHeight() ;
	}
	//
	// 画面外領域描画
	//
	HBRUSH	hBrush = NULL ;
	if ( m_bgiFrame.dwFlags & bgfFillColor )
	{
		hBrush = ::CreateSolidBrush
			( RGB( m_bgiFrame.rgbBG.rgb.Red,
					m_bgiFrame.rgbBG.rgb.Green,
					m_bgiFrame.rgbBG.rgb.Blue ) ) ;
	}
	RECT	rectClient ;
	if ( !pWnd->GetClientRect( &rectClient ) )
	{
		if ( hBrush != NULL )
		{
			::DeleteObject( hBrush ) ;
		}
		return ;
	}
	if ( irDraw.x > rectClient.left )
	{
		//
		// 画面左側領域描画
		//
		PEGL_IMAGE_INFO	pLeft = m_bgiFrame.pLeft ;
		if ( pLeft == NULL )
		{
			pLeft = m_bgiFrame.pTile ;
		}
		int	xLeft = irDraw.x ;
		if ( pLeft != NULL )
		{
			EGL_SIZE	sizeLeft ;
			sizeLeft.w = pLeft->dwImageWidth ;
			sizeLeft.h = pLeft->dwImageHeight ;
			//
			if ( m_bgiFrame.dwFlags & bgfStretch )
			{
				::SetStretchBltMode( hdc, COLORONCOLOR ) ;
				sizeLeft.w = sizeLeft.w * irDraw.h / sizeLeft.h ;
				sizeLeft.h = irDraw.h ;
			}
			xLeft -= sizeLeft.w ;
			for ( int y = 0; y < irDraw.h; y += sizeLeft.h )
			{
				pImage->DrawToDC
					( hdc, xLeft, y + irDraw.y, &sizeLeft, NULL ) ;
			}
		}
		if ( (xLeft > 0) && (m_bgiFrame.dwFlags & bgfFillColor) )
		{
			RECT	rectFill = { 0, 0, xLeft, rectClient.bottom } ;
			::FillRect( hdc, &rectFill, hBrush ) ;
		}
	}
	if ( irDraw.x + irDraw.w < rectClient.right )
	{
		//
		// 画面右側領域描画
		//
		PEGL_IMAGE_INFO	pRight = m_bgiFrame.pRight ;
		if ( pRight == NULL )
		{
			pRight = m_bgiFrame.pTile ;
		}
		int			xRight = irDraw.x + irDraw.w ;
		EGL_SIZE	sizeRight = { 0, 0 } ;
		if ( pRight != NULL )
		{
			sizeRight.w = pRight->dwImageWidth ;
			sizeRight.h = pRight->dwImageHeight ;
			//
			if ( m_bgiFrame.dwFlags & bgfStretch )
			{
				::SetStretchBltMode( hdc, COLORONCOLOR ) ;
				sizeRight.w = sizeRight.w * irDraw.h / sizeRight.h ;
				sizeRight.h = irDraw.h ;
			}
			for ( int y = 0; y < irDraw.h; y += sizeRight.h )
			{
				pImage->DrawToDC
					( hdc, xRight, y + irDraw.y, &sizeRight, NULL ) ;
			}
		}
		if ( (xRight < rectClient.right)
					&& (m_bgiFrame.dwFlags & bgfFillColor) )
		{
			RECT	rectFill =
						{ xRight + sizeRight.w, 0,
							rectClient.right, rectClient.bottom } ;
			::FillRect( hdc, &rectFill, hBrush ) ;
		}
	}
	if ( irDraw.y > rectClient.top )
	{
		//
		// 画面上側領域描画
		//
		PEGL_IMAGE_INFO	pUpper = m_bgiFrame.pUpper ;
		if ( pUpper == NULL )
		{
			pUpper = m_bgiFrame.pTile ;
		}
		int	yUpper = irDraw.y ;
		if ( pUpper != NULL )
		{
			EGL_SIZE	sizeUpper ;
			sizeUpper.w = pUpper->dwImageWidth ;
			sizeUpper.h = pUpper->dwImageHeight ;
			//
			if ( m_bgiFrame.dwFlags & bgfStretch )
			{
				::SetStretchBltMode( hdc, COLORONCOLOR ) ;
				sizeUpper.h = sizeUpper.h * irDraw.w / sizeUpper.w ;
				sizeUpper.w = irDraw.w ;
			}
			yUpper -= sizeUpper.h ;
			for ( int x = 0; x < irDraw.w; x += sizeUpper.w )
			{
				pImage->DrawToDC
					( hdc, x + irDraw.x, yUpper, &sizeUpper, NULL ) ;
			}
		}
		if ( (yUpper > 0) && (m_bgiFrame.dwFlags & bgfFillColor) )
		{
			RECT	rectFill = { 0, 0, rectClient.right, yUpper } ;
			::FillRect( hdc, &rectFill, hBrush ) ;
		}
	}
	if ( irDraw.y + irDraw.h < rectClient.bottom )
	{
		//
		// 画面下側領域描画
		//
		PEGL_IMAGE_INFO	pUnder = m_bgiFrame.pUnder ;
		if ( pUnder == NULL )
		{
			pUnder = m_bgiFrame.pTile ;
		}
		int			yUnder = irDraw.y + irDraw.h ;
		EGL_SIZE	sizeUnder = { 0, 0 } ;
		if ( pUnder != NULL )
		{
			sizeUnder.w = pUnder->dwImageWidth ;
			sizeUnder.h = pUnder->dwImageHeight ;
			//
			if ( m_bgiFrame.dwFlags & bgfStretch )
			{
				::SetStretchBltMode( hdc, COLORONCOLOR ) ;
				sizeUnder.h = sizeUnder.h * irDraw.w / sizeUnder.w ;
				sizeUnder.w = irDraw.w ;
			}
			for ( int x = 0; x < irDraw.w; x += sizeUnder.w )
			{
				pImage->DrawToDC
					( hdc, x + irDraw.x, yUnder, &sizeUnder, NULL ) ;
			}
		}
		if ( (yUnder < rectClient.bottom)
				&& (m_bgiFrame.dwFlags & bgfFillColor) )
		{
			RECT	rectFill =
						{ 0, yUnder + sizeUnder.h,
							rectClient.right, rectClient.bottom } ;
			::FillRect( hdc, &rectFill, hBrush ) ;
		}
	}
	if ( hBrush != NULL )
	{
		::DeleteObject( hBrush ) ;
	}
}

ESLError EWindowSpriteInterface::DirectDrawImage
	( EWindow * pWnd, PEGL_IMAGE_INFO pImage, bool fVSync )
{
	if ( m_pDrawImage != NULL )
	{
		if ( m_pDrawImage->GetDirect3DDevice9() != NULL )
		{
			if ( !m_fImageStretching )
			{
				m_ptScreenBase.x = 0 ;
				m_ptScreenBase.y = 0 ;
				m_szScreenStretched.w = pImage->dwImageWidth ;
				m_szScreenStretched.h = pImage->dwImageHeight ;
			}
			return	m_pDrawImage->DrawImageToDisplayD3D9
				( pImage,
					m_ptScreenBase.x, m_ptScreenBase.y,
					&m_szScreenStretched, NULL ) ;
		}
		else
		{
			EGLSize	sizeImage( pImage->dwImageWidth, pImage->dwImageHeight ) ;
			if ( m_sizeVRAM != sizeImage )
			{
				if ( m_pddsVRAM != NULL )
				{
					m_pddsVRAM->Release( ) ;
				}
				m_pddsVRAM =
					m_pDrawImage->
						CreateSurfaceOnVRAM( sizeImage.w, sizeImage.h ) ;
				m_sizeVRAM = sizeImage ;
			}
			if ( m_pddsVRAM != NULL )
			{
				DWORD	fdwFlags = EGLDrawImage::dfDirectDraw ;
				if ( fVSync )
				{
					fdwFlags |= EGLDrawImage::dfWaitVerticalBlank ;
				}
				if ( !m_fImageStretching )
				{
					m_ptScreenBase.x = 0 ;
					m_ptScreenBase.y = 0 ;
					m_szScreenStretched.w = pImage->dwImageWidth ;
					m_szScreenStretched.h = pImage->dwImageHeight ;
				}
				return	m_pDrawImage->DrawImageToDisplay
					( *pWnd, pImage,
						m_ptScreenBase.x, m_ptScreenBase.y,
						&m_szScreenStretched, NULL, fdwFlags, m_pddsVRAM ) ;
			}
		}
	}
	return	eslErrFailed ;
}

// ParallelStretchDraw 構築関数
//////////////////////////////////////////////////////////////////////////////
EWindowSpriteInterface::ParallelStretchDraw::ParallelStretchDraw
( EGLImage& imgStretchBuffer, EGLImage * pSrcImage, int yBlockHeight )
	: m_imgStretchBuffer( imgStretchBuffer ),
		m_yNextLine( 0 ), m_yBlockHeight( yBlockHeight )
{
	memset( &m_dp, 0, sizeof(m_dp) ) ;
	m_dp.dwFlags = EGL_SMOOTH_STRETCH | EGL_DRAW_A_MOVE ;
	m_dp.pSrcImage = pSrcImage->GetInfo() ;
	m_dp.pImageAxes = &m_iax ;
	//
	EGLSize	szStretched = m_imgStretchBuffer.GetSize() ;
	EGLSize	sizeImage = pSrcImage->GetSize() ;
	int	nSrcWidth = sizeImage.w - 1 ;
	int	nSrcHeight = sizeImage.h - 1 ;
	int	nDstWidth = szStretched.w - 1 ;
	int	nDstHeight = szStretched.h - 1 ;
	if ( (nSrcWidth == 0) || (nSrcHeight == 0) )
	{
		nSrcWidth = sizeImage.w ;
		nSrcHeight = sizeImage.h ;
		nDstWidth = szStretched.w ;
		nDstHeight = szStretched.h ;
	}
	m_iax.xAxis.x = (REAL32) ((double) nDstWidth / nSrcWidth) ;
	m_iax.xAxis.y = 0.0 ;
	m_iax.yAxis.x = 0.0 ;
	m_iax.yAxis.y = (REAL32) ((double) nDstHeight / nSrcHeight) ;
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool EWindowSpriteInterface::ParallelStretchDraw::Continue( void * pInstance )
{
	Param *		p = (Param*) pInstance ;
	EGLSize		size = m_imgStretchBuffer.GetSize() ;
	EGL_RECT	rectDst ;
	//
	if ( m_yNextLine >= size.h )
	{
		return	false ;
	}
	rectDst.left = 0 ;
	rectDst.top = m_yNextLine ;
	rectDst.right = size.w - 1 ;
	rectDst.bottom = rectDst.top + m_yBlockHeight - 1 ;
	if ( rectDst.bottom >= size.h )
	{
		rectDst.bottom = size.h - 1 ;
	}
	p->hDraw->Initialize( m_imgStretchBuffer, &rectDst, NULL ) ;
	//
	m_yNextLine += m_yBlockHeight ;
	return	true ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::ParallelStretchDraw::RunParallel( void * pInstance )
{
	Param *	p = (Param*) pInstance ;
	if ( !p->hDraw->PrepareDraw( &m_dp ) )
	{
		p->hDraw->DrawImage() ;
	}
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnCommand
	( ESpriteInterface * pItem,
		long int nNotification, long int nParameter,
		int nPriority, bool fOverwrite )
{
	if ( m_fQueueCommand )
	{
		if ( pItem != NULL )
		{
			EWideString	wstrID = pItem->ID() ;
			ESpriteInterface *	pParent =
					ESLTypeCast<ESpriteInterface>( pItem->GetParent( ) ) ;
			while ( (pParent != NULL) && (this != pParent) )
			{
				if ( !pParent->ID().IsEmpty() )
				{
					wstrID = pParent->ID() + L"\\" + wstrID ;
				}
				pParent =
					ESLTypeCast<ESpriteInterface>( pParent->GetParent( ) ) ;
			}
			QueueCommand
				( wstrID, nNotification, nParameter, nPriority, fOverwrite ) ;
		}
	}
	else
	{
		ESpriteInterface::OnCommand
			( pItem, nNotification, nParameter, nPriority, fOverwrite ) ;
	}
}

// ウィンドウメッセージを処理する
//////////////////////////////////////////////////////////////////////////////
struct	HANDLE_MSG_INFO
{
	int		nCount ;
	HANDLE	hDone ;
} ;
static LRESULT __stdcall HandleWindowMessageProc( void * pInstance ) ;
void EWindowSpriteInterface::HandleWindowMessage
	( int nCount, DWORD dwTimeout ) const
{
	if ( GetWindow() != NULL )
	{
		HANDLE_MSG_INFO	hmi ;
		hmi.nCount = nCount ;
		hmi.hDone = ::CreateEvent( NULL, FALSE, FALSE, NULL ) ;
		if ( !ProcedureOnWindowThread
			( &HandleWindowMessageProc, &hmi, NULL, true ) )
		{
			if ( ::WaitForSingleObject
					( hmi.hDone, dwTimeout ) == WAIT_TIMEOUT )
			{
				hmi.nCount = 0 ;
				::WaitForSingleObject( hmi.hDone, INFINITE ) ;
			}
		}
		::CloseHandle( hmi.hDone ) ;
	}
}

LRESULT __stdcall HandleWindowMessageProc( void * pInstance )
{
	volatile HANDLE_MSG_INFO *	phmi = (HANDLE_MSG_INFO*) pInstance ;
	MSG		msg ;
	for ( int i = 0; i < phmi->nCount; i ++ )
	{
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
		else
		{
			break ;
		}
	}
	::SetEvent( phmi->hDone ) ;
	return	0 ;
}

// ウィンドウスレッドから関数を呼び出す
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::ProcedureOnWindowThread
	( PFUNC_PROCEDURE pfnProc, void * pInstance,
					LRESULT * pResult, bool fAsync ) const
{
	if ( (GetWindow() != NULL) && (pfnProc != NULL) )
	{
		LRESULT	lrResult = 0 ;
		if ( ::GetWindowThreadProcessId( *(GetWindow()), NULL )
										== ::GetCurrentThreadId() )
		{
			lrResult = pfnProc( pInstance ) ;
		}
		else
		{
			if ( !fAsync )
			{
				lrResult = GetWindow()->SendMessage
					( wmCallProcedure, (WPARAM) pfnProc, (LPARAM) pInstance ) ;
			}
			else
			{
				if ( !GetWindow()->PostMessage
					( wmCallProcedure, (WPARAM) pfnProc, (LPARAM) pInstance ) )
				{
					return	eslErrGeneral ;
				}
			}
		}
		if ( pResult != NULL )
		{
			*pResult = lrResult ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// スプライト上の指定領域の更新通知
//////////////////////////////////////////////////////////////////////////////
bool EWindowSpriteInterface::UpdateRect( EGL_RECT * pUpdateRect )
{
	bool	fResult = false ;
	Lock( ) ;
	if ( ESpriteInterface::UpdateRect( pUpdateRect ) )
	{
		EWindow *	pWnd = GetWindow( ) ;
		if ( (pWnd != NULL) && ::IsWindow( *pWnd ) )
		{
			if ( m_fLayeredWindow )
			{
				if ( !m_fPendingUpdateWindow && !m_fNoInvalidateWindow )
				{
					pWnd->PostMessage( WM_PAINT, 0, 0 ) ;
					m_fPendingUpdateWindow = true ;
				}
			}
			else if ( !m_fNoInvalidateWindow )
			{
				if ( pUpdateRect == NULL )
				{
					pWnd->InvalidateRect( NULL, FALSE ) ;
				}
				else
				{
					RECT	rect ;
					if ( m_fImageStretching )
					{
						EGL_POINT	ptUpperLeft, ptUnderRight ;
						ptUpperLeft.x = pUpdateRect->left ;
						ptUpperLeft.y = pUpdateRect->top ;
						ptUnderRight.x = pUpdateRect->right ;
						ptUnderRight.y = pUpdateRect->bottom ;
						//
						ClientToWindow( ptUpperLeft ) ;
						ClientToWindow( ptUnderRight ) ;
						//
						rect.left = ptUpperLeft.x - 1 ;
						rect.top = ptUpperLeft.y - 1 ;
						rect.right = ptUnderRight.x + 3 ;
						rect.bottom = ptUnderRight.y + 3 ;
					}
					else
					{
						rect.left = pUpdateRect->left ;
						rect.top = pUpdateRect->top ;
						rect.right = pUpdateRect->right + 2 ;
						rect.bottom = pUpdateRect->bottom + 2 ;
					}
					pWnd->InvalidateRect( &rect, FALSE ) ;
				}
				::ResetEvent( m_hPaintSignal ) ;
			}
		}
		fResult = true ;
	}
	Unlock( ) ;
	return	fResult ;
}

// 有効化・無効化
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::Enable( bool fEnable )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( pWnd != NULL )
	{
		::EnableWindow( *pWnd, fEnable ) ;
	}
	ESpriteInterface::Enable( fEnable ) ;
	//
	if ( !m_fEnabled && (m_pMouseFocus != NULL) )
	{
		POINT	ptCursor ;
		if ( pWnd != NULL )
		{
			::GetCursorPos( &ptCursor ) ;
			HWND	hWnd = ::WindowFromPoint( ptCursor ) ;
			pWnd->ScreenToClient( &ptCursor ) ;
		}
		else
		{
			ptCursor.x = 0 ;
			ptCursor.y = 0 ;
		}
		m_pMouseFocus->OnMouseLeave( 0, ptCursor.x, ptCursor.y ) ;
	}
}

// マウスメッセージのキャプチャー設定
//////////////////////////////////////////////////////////////////////////////
static LRESULT __stdcall
	EWindowSpriteInterface_SetCaptureProc( void * pInstance ) ;
void EWindowSpriteInterface::SetCapture( ESpriteInterface * pSprite )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( pWnd != NULL )
	{
		ProcedureOnWindowThread
			( EWindowSpriteInterface_SetCaptureProc, (HWND) *pWnd, NULL, true ) ;
//		::SetCapture( *pWnd ) ;
	}
	ESpriteInterface::SetCapture( pSprite ) ;
}

static LRESULT __stdcall
	EWindowSpriteInterface_SetCaptureProc( void * pInstance )
{
	::SetCapture( (HWND) pInstance ) ;
	return	0 ;
}

// マウスメッセージのキャプチャー解除
//////////////////////////////////////////////////////////////////////////////
static LRESULT __stdcall
	EWindowSpriteInterface_ReleaseCaptureProc( void * pInstance ) ;
void EWindowSpriteInterface::ReleaseCapture( ESpriteInterface * pSprite )
{
	ProcedureOnWindowThread
		( EWindowSpriteInterface_ReleaseCaptureProc, NULL, NULL, true ) ;
//	::ReleaseCapture( ) ;
	ESpriteInterface::ReleaseCapture( pSprite ) ;
}


static LRESULT __stdcall
	EWindowSpriteInterface_ReleaseCaptureProc( void * pInstance )
{
	::ReleaseCapture( ) ;
	return	0 ;
}

// コマンド待ち行列を有効化する
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::EnableCommandQueue( bool fQueueCommand )
{
	m_fQueueCommand = fQueueCommand ;
}

// コマンド待ち行列を初期化する
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::FlushCommandQueue
	( bool fQueueCommand, int nPriority )
{
	Lock( ) ;
	m_fQueueCommand = fQueueCommand ;
	//
	const int	nCount = m_queCommand.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		EWndSpriteCmd *	pCmd = m_queCommand.GetAt( i ) ;
		if ( (pCmd != NULL) && (pCmd->m_nPriority <= nPriority) )
		{
			m_queCommand.SetAt( i, NULL ) ;
		}
	}
	m_queCommand.TrimEmpty() ;
	//
	if ( m_queCommand.GetSize() == 0 )
	{
		::ResetEvent( m_hQueCmdEvent ) ;
	}
	Unlock( ) ;
}

// コマンドを待ち行列に追加
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::QueueCommand
	( const wchar_t * pwszID,
		long int nNotification, long int nParameter,
		int nPriority, bool fOverwrite )
{
	if ( m_pSyncTarget != NULL )
	{
		m_pSyncTarget->QueueCommand
			( pwszID, nNotification, nParameter, nPriority, fOverwrite ) ;
		return ;
	}
	EWndSpriteCmd *	pCmd = new EWndSpriteCmd ;
	pCmd->m_nPriority = nPriority ;
	pCmd->m_wstrFullID = pwszID ;
	pCmd->m_wstrID = pCmd->m_wstrFullID.GetFileNamePart( ) ;
	pCmd->m_nNotification = nNotification ;
	pCmd->m_nParameter = nParameter ;
	//
	QueueCommandObject( pCmd, fOverwrite ) ;
}

void EWindowSpriteInterface::QueueCommandObject
	( EWndSpriteCmd * pCmd, bool fOverwrite )
{
	if ( m_pSyncTarget != NULL )
	{
		m_pSyncTarget->QueueCommandObject( pCmd, fOverwrite ) ;
		return ;
	}
	ESLAssert( pCmd != NULL ) ;
	if ( pCmd != NULL )
	{
		Lock( ) ;
		unsigned int	i, nCount ;
		if ( fOverwrite )
		{
			nCount = m_queCommand.GetSize() ;
			for ( i = 0; i < nCount; i ++ )
			{
				EWndSpriteCmd *	pwsc = m_queCommand.GetAt( i ) ;
				if ( (pwsc != NULL)
					&& (pwsc->m_wstrFullID == pCmd->m_wstrFullID) )
				{
					m_queCommand.RemoveAt( i ) ;
					break ;
				}
			}
		}
		nCount = m_queCommand.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			EWndSpriteCmd *	pwsc = m_queCommand.GetAt( i ) ;
			if ( (pwsc != NULL)
				&& (pwsc->m_nPriority < pCmd->m_nPriority) )
			{
				break ;
			}
		}
		m_queCommand.InsertAt( i, pCmd ) ;
		::SetEvent( m_hQueCmdEvent ) ;
		Unlock( ) ;
	}
}

// 待ち行列からコマンドを取得
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::GetCommand
	( EWndSpriteCmd & wscCmd, DWORD dwTimeout, bool fRemove )
{
	EWndSpriteCmd *	pCmd = NULL ;
	ESLError	err = GetCommandObject( pCmd, dwTimeout, fRemove ) ;
	if ( !err )
	{
		wscCmd = *pCmd ;
		if ( fRemove )
		{
			delete	pCmd ;
		}
	}
	return	err ;
}

ESLError EWindowSpriteInterface::GetCommandObject
	( EWndSpriteCmd *& pCmd, DWORD dwTimeout, bool fRemove )
{
	DWORD	dwWaitResult ;
	pCmd = NULL ;
	dwWaitResult = ::WaitForSingleObject( m_hQueCmdEvent, dwTimeout ) ;
	if ( dwWaitResult == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	Lock( ) ;
	pCmd = m_queCommand.GetAt( 0 ) ;
	ESLAssert( pCmd != NULL ) ;
	if ( pCmd == NULL )
	{
		ESLAssert( m_queCommand.GetSize() != 0 ) ;
		m_queCommand.RemoveAt( 0 ) ;
		if ( m_queCommand.GetSize() == 0 )
		{
			::ResetEvent( m_hQueCmdEvent ) ;
		}
		Unlock( ) ;
		return	eslErrGeneral ;
	}
	if ( fRemove )
	{
		m_queCommand.DetachAt( 0 ) ;
		if ( m_queCommand.GetSize() == 0 )
		{
			::ResetEvent( m_hQueCmdEvent ) ;
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 待ち行列にコマンドがあるか？
//////////////////////////////////////////////////////////////////////////////
bool EWindowSpriteInterface::IsQueueCommand( void ) const
{
	bool	fCmd = false ;
	((EWindowSpriteInterface*)this)->Lock( ) ;
	fCmd = (m_queCommand.GetAt(0) != NULL) ;
	((EWindowSpriteInterface*)this)->Unlock( ) ;
	return	fCmd ;
}

// マウス座標通知
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::CallMouseMove( void )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( pWnd != NULL )
	{
		POINT	ptCursorPos ;
		::GetCursorPos( &ptCursorPos ) ;
		HWND	hWnd = ::WindowFromPoint( ptCursorPos ) ;
		EGLPoint	ptCursor( ptCursorPos.x, ptCursorPos.y ) ;
		ScreenToClient( ptCursor ) ;
		//
		Lock( ) ;
		if ( ((HWND) *pWnd == hWnd) || (m_pCaptured != NULL) )
		{
			EGLSize	sizeWindow = GetSize( ) ;
			RECT	rectClient ;
			POINT	ptClientCursorPos = ptCursorPos ;
			pWnd->GetClientRect( &rectClient ) ;
			pWnd->ScreenToClient( &ptClientCursorPos ) ;
			if ( ((ptCursor.x >= 0) && (ptCursor.x < sizeWindow.w)
					&& (ptCursor.y >= 0) && (ptCursor.y < sizeWindow.h))
				|| (m_pCaptured != NULL) )
			{
				if ( (ptClientCursorPos.x < rectClient.right)
						&& (ptClientCursorPos.y < rectClient.bottom) )
				{
					OnMouseMove( 0, ptCursor.x, ptCursor.y ) ;
					//
					if ( ::GetWindowThreadProcessId( *pWnd, NULL )
											== ::GetCurrentThreadId() )
					{
						OnSetCursor( ptCursor.x, ptCursor.y ) ;
					}
					else
					{
						pWnd->PostMessage
							( WM_SETCURSOR, (WPARAM) (HWND) *pWnd, HTCLIENT ) ;
					}
				}
			}
			else
			{
				OnMouseLeave( 0, ptCursor.x, ptCursor.y ) ;
			}
		}
		else
		{
			OnMouseLeave( 0, ptCursor.x, ptCursor.y ) ;
		}
		Unlock( ) ;
	}
}

// スレッド排他処理開始
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::Lock( DWORD dwTimeout )
{
	if ( m_pSyncTarget != NULL )
	{
		return	m_pSyncTarget->Lock( dwTimeout ) ;
	}
	if ( ::GetCurrentThreadId() == m_idMsgHandlerThread )
	{
		if ( m_fMsgHandlerUnlocked )
		{
			RelockOnMsgHandler( ) ;
		}
		if ( ::WaitForSingleObject( m_hMutex, dwTimeout ) == WAIT_TIMEOUT )
		{
			return	eslErrTimeout ;
		}
		m_nMsgHandlerLocked ++ ;
	}
	else if ( ::WaitForSingleObject( m_hMutex, dwTimeout ) == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
//	::EnterCriticalSection( &m_cs ) ;
	return	eslErrSuccess ;
}

// スレッド排他処理終了
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::Unlock( void )
{
	if ( m_pSyncTarget != NULL )
	{
		return	m_pSyncTarget->Unlock() ;
	}
	if ( ::GetCurrentThreadId() == m_idMsgHandlerThread )
	{
		ESLAssert( m_nMsgHandlerLocked > 0 ) ;
		if ( m_fMsgHandlerUnlocked )
		{
			if ( (-- m_nMsgHandlerLocked) == 0 )
			{
				m_fMsgHandlerUnlocked = 0 ;
			}
			return	eslErrSuccess ; ;
		}
		if ( !::ReleaseMutex( m_hMutex ) )
		{
			return	eslErrGeneral ;
		}
		m_nMsgHandlerLocked -- ;
	}
	else
	{
	//	::LeaveCriticalSection( &m_cs ) ;
		if ( !::ReleaseMutex( m_hMutex ) )
		{
			return	eslErrGeneral ;
		}
	}
	return	eslErrSuccess ;
}

// スレッド排他処理（メッセージハンドラ中の排他処理の解除）
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::UnlockOnMsgHandler( void )
{
	if ( m_pSyncTarget != NULL )
	{
		m_pSyncTarget->UnlockOnMsgHandler() ;
		return ;
	}
	if ( ::GetCurrentThreadId() == m_idMsgHandlerThread )
	{
		if ( !m_fMsgHandlerUnlocked )
		{
			for ( int i = 0; i < m_nMsgHandlerLocked; i ++ )
			{
				::ReleaseMutex( m_hMutex ) ;
			}
			m_fMsgHandlerUnlocked = 1 ;
		}
	}
}

void EWindowSpriteInterface::RelockOnMsgHandler( void )
{
	if ( m_pSyncTarget != NULL )
	{
		m_pSyncTarget->RelockOnMsgHandler() ;
		return ;
	}
	if ( ::GetCurrentThreadId() == m_idMsgHandlerThread )
	{
		if ( m_fMsgHandlerUnlocked )
		{
			for ( int i = 0; i < m_nMsgHandlerLocked; i ++ )
			{
				if ( ::WaitForSingleObject
						( m_hMutex, INFINITE ) == WAIT_TIMEOUT )
				{
					break ;
				}
			}
			m_fMsgHandlerUnlocked = 0 ;
		}
	}
}

// スレッド同期描画処理
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::SyncTimePaint( DWORD dwTimeout )
{
	if ( m_pSyncTarget != NULL )
	{
		return	m_pSyncTarget->SyncTimePaint( dwTimeout ) ;
	}
	::ResetEvent( m_hWaitTimePaint ) ;
	::ResetEvent( m_hSyncTimePaint ) ;
	if ( ::WaitForSingleObject
			( m_hSyncTimePaint, dwTimeout ) == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	return	eslErrSuccess ;
}

void EWindowSpriteInterface::AsyncTimePaint( void )
{
	if ( m_pSyncTarget != NULL )
	{
		m_pSyncTarget->AsyncTimePaint() ;
		return ;
	}
	::SetEvent( m_hWaitTimePaint ) ;
}

// 描画完了同期
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::WaitForDonePaint( DWORD dwTimeout )
{
	DWORD	dwResult = ::WaitForSingleObject( m_hPaintSignal, dwTimeout ) ;
	if ( dwResult == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	if ( dwResult == WAIT_OBJECT_0 )
	{
		::ResetEvent( m_hPaintSignal ) ;
		return	eslErrSuccess ;
	}
	return	eslErrFailed ;
}

// 同期ターゲットを設定する
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::AttachSyncObject( EWindowSpriteInterface * pSync )
{
	m_pSyncTarget = pSync ;
}

// 描画更新制御
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::FreezePaint( void )
{
	SSystem::AtomicAdd( &m_nFreezePaint, 1 ) ;
}

void EWindowSpriteInterface::UnfreezePaint( void )
{
	ESLAssert( m_nFreezePaint >= 1 ) ;
	if ( SSystem::AtomicSub( &m_nFreezePaint, 1 ) == 0 )
	{
		UpdateRect( NULL ) ;
	}
}

// 座標変換設定
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::SetImageStretching
	( const EGL_POINT * pptBase, const EGL_SIZE * pszScreen )
{
	bool	fChangeMode = false ;
	if ( pptBase && pszScreen )
	{
		if ( !m_fImageStretching
			|| (m_ptScreenBase != *pptBase)
			|| (m_szScreenStretched != *pszScreen) )
		{
			fChangeMode = true ;
		}
		m_fImageStretching = true ;
		m_ptScreenBase = *pptBase ;
		m_szScreenStretched = *pszScreen ;
		//
		EGLSize	sizeImage = GetSize( ) ;
		if ( (m_pDrawImage != NULL) && !m_fStretchingByCPU )
		{
			if ( m_sizeVRAM != sizeImage )
			{
				if ( m_pddsVRAM != NULL )
				{
					m_pddsVRAM->Release( ) ;
					m_pddsVRAM = NULL ;
				}
			}
			if ( m_pddsVRAM == NULL )
			{
				m_pddsVRAM =
					m_pDrawImage->CreateSurfaceOnVRAM
								( sizeImage.w, sizeImage.h ) ;
				m_sizeVRAM = sizeImage ;
			}
			m_pDrawImage->ResetVSync() ;
		}
		if ( m_pivView3D != NULL )
		{
			m_pivView3D->SetViewPosition
				( m_ptScreenBase.x, m_ptScreenBase.y,
					m_szScreenStretched.w, m_szScreenStretched.h ) ;
		}
	}
	else
	{
		if ( m_fImageStretching )
		{
			fChangeMode = true ;
		}
		m_fImageStretching = false ;
		//
		if ( m_pDrawImage != NULL )
		{
			m_pDrawImage->ResetVSync() ;
		}
		if ( m_pivView3D != NULL )
		{
			EGLSize	sizeImage = GetSize( ) ;
			m_pivView3D->SetViewPosition( 0, 0, sizeImage.w, sizeImage.h ) ;
		}
	}
	EWindow *	pWnd = GetWindow( ) ;
	if ( (pWnd != NULL) && fChangeMode )
	{
		pWnd->InvalidateRect( NULL, TRUE ) ;
	}
}

// 画面伸縮描画のための描画オブジェクトを関連付ける
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::AttachDrawImageObject( EGLDrawImage * pDrawImage )
{
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->DetachNotify( this ) ;
	}
	//
	m_pDrawImage = pDrawImage ;
	//
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->AddNotify( this ) ;
	}
}

// 座標変換
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::WindowToClient( EGL_POINT & pos )
{
	if ( m_fImageStretching )
	{
		PEGL_IMAGE_INFO	pInfo = GetInfo() ;
		if ( (pInfo != NULL)
			&& (pInfo->dwImageWidth > 1) && (pInfo->dwImageHeight > 1)
			&& (m_szScreenStretched.w >= 1) && (m_szScreenStretched.h >= 1) )
		{
			INT64	x =
				(INT64) (pos.x - m_ptScreenBase.x) * pInfo->dwImageWidth
												/ m_szScreenStretched.w ;
			INT64	y =
				(INT64) (pos.y - m_ptScreenBase.y) * pInfo->dwImageHeight
												/ m_szScreenStretched.h ;
			//
			pos.x = (long int) x ;
			pos.y = (long int) y ;
		}
		else
		{
			pos.x -= m_ptScreenBase.x ;
			pos.y -= m_ptScreenBase.y ;
		}
	}
}

void EWindowSpriteInterface::ClientToWindow( EGL_POINT & pos )
{
	if ( m_fImageStretching )
	{
		PEGL_IMAGE_INFO	pInfo = GetInfo() ;
		if ( (pInfo != NULL)
			&& (pInfo->dwImageWidth > 1) && (pInfo->dwImageHeight > 1) )
		{
			INT64	x =
				(INT64) pos.x * m_szScreenStretched.w
									/ pInfo->dwImageWidth + m_ptScreenBase.x ;
			INT64	y =
				(INT64) pos.y * m_szScreenStretched.h
									/ pInfo->dwImageHeight + m_ptScreenBase.y ;
			//
			pos.x = (long int) x ;
			pos.y = (long int) y ;
		}
		else
		{
			pos.x += m_ptScreenBase.x ;
			pos.y += m_ptScreenBase.y ;
		}
	}
}

void EWindowSpriteInterface::ScreenToClient( EGL_POINT & pos )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( pWnd != NULL )
	{
		POINT	pt ;
		pt.x = pos.x ;
		pt.y = pos.y ;
		pWnd->ScreenToClient( &pt ) ;
		pos.x = pt.x ;
		pos.y = pt.y ;
	}
	WindowToClient( pos ) ;
}

void EWindowSpriteInterface::ClientToScreen( EGL_POINT & pos )
{
	ClientToWindow( pos ) ;
	//
	EWindow *	pWnd = GetWindow( ) ;
	if ( pWnd != NULL )
	{
		POINT	pt ;
		pt.x = pos.x ;
		pt.y = pos.y ;
		pWnd->ClientToScreen( &pt ) ;
		pos.x = pt.x ;
		pos.y = pt.y ;
	}
}

void EWindowSpriteInterface::WindowToClientSize( EGL_SIZE & sz )
{
	if ( m_fImageStretching )
	{
		PEGL_IMAGE_INFO	pInfo = GetInfo() ;
		if ( (pInfo != NULL)
			&& (pInfo->dwImageWidth > 1) && (pInfo->dwImageHeight > 1)
			&& (m_szScreenStretched.w >= 1) && (m_szScreenStretched.h >= 1) )
		{
			INT64	w =
				(INT64) sz.w * pInfo->dwImageWidth
										/ m_szScreenStretched.w ;
			INT64	h =
				(INT64) sz.h * pInfo->dwImageHeight
										/ m_szScreenStretched.h ;
			//
			sz.w = (long int) w ;
			sz.h = (long int) h ;
		}
	}
}

void EWindowSpriteInterface::ClientToWindowSize( EGL_SIZE & sz )
{
	if ( m_fImageStretching )
	{
		PEGL_IMAGE_INFO	pInfo = GetInfo() ;
		if ( (pInfo != NULL)
			&& (pInfo->dwImageWidth > 1) && (pInfo->dwImageHeight > 1) )
		{
			INT64	w =
				(INT64) sz.w * m_szScreenStretched.w
									/ pInfo->dwImageWidth ;
			INT64	h =
				(INT64) sz.h * m_szScreenStretched.h
									/ pInfo->dwImageHeight ;
			//
			sz.w = (long int) w ;
			sz.h = (long int) h ;
		}
	}
}

// DirectDraw オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnReleaseDirectDraw( EGLDrawImage * pdi )
{
	if ( m_pddsVRAM != NULL )
	{
		m_pddsVRAM->Release() ;
		m_pddsVRAM = NULL ;
	}
}

// DirectDraw オブジェクトが作成された後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnCreateDirectDraw( EGLDrawImage * pdi )
{
}

// レイヤードウィンドウに設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::SetLayeredWindow( bool fLayeredWindow )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( (pWnd == NULL) || !::IsWindow( *pWnd ) )
	{
		return	eslErrGeneral ;
	}
	if ( fLayeredWindow )
	{
		PEGL_IMAGE_INFO	pImageInf = GetInfo( ) ;
		if ( (m_apiUpdateLayeredWindow == NULL) || (pImageInf == NULL) )
		{
			return	eslErrGeneral ;
		}
		if ( ::eglGetDC( pImageInf ) == NULL )
		{
			return	eslErrGeneral ;
		}
		//
		m_bfLayeredWindow.BlendOp = AC_SRC_OVER ;
		m_bfLayeredWindow.BlendFlags = 0 ;
		m_bfLayeredWindow.AlphaFormat = 0 ;
		m_bfLayeredWindow.SourceConstantAlpha = 255 ;
		//
		if ( pImageInf->fdwFormatType == EIF_RGBA_BITMAP )
		{
			m_bfLayeredWindow.AlphaFormat = AC_SRC_ALPHA ;
		}
		//
		m_fLayeredWindow = true ;
		pWnd->SetExStyle( pWnd->GetExStyle() | WS_EX_LAYERED ) ;
		UpdateLayeredWindow( ) ;
	}
	else
	{
		pWnd->SetExStyle( pWnd->GetExStyle() & ~WS_EX_LAYERED ) ;
		m_fLayeredWindow = false ;
	}
	return	eslErrSuccess ;
}

// ウィンドウ透明度を設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::SetLayeredWindowTransparency
	( unsigned int nTransparency )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( (pWnd == NULL) || !::IsWindow( *pWnd ) || !m_fLayeredWindow )
	{
		return	eslErrGeneral ;
	}
	if ( nTransparency < 0x100 )
	{
		m_bfLayeredWindow.SourceConstantAlpha =
			(BYTE) (0xFF - nTransparency + (nTransparency >> 7)) ;
	}
	else
	{
		m_bfLayeredWindow.SourceConstantAlpha = 0 ;
	}
	return	UpdateLayeredWindow( ) ;
}

// レイヤードウィンドウの表示更新
//////////////////////////////////////////////////////////////////////////////
ESLError EWindowSpriteInterface::UpdateLayeredWindow( void )
{
	EWindow *	pWnd = GetWindow( ) ;
	if ( (pWnd == NULL) || !::IsWindow( *pWnd ) || !m_fLayeredWindow )
	{
		return	eslErrGeneral ;
	}
	ESLAssert( m_apiUpdateLayeredWindow != NULL ) ;
	PEGL_IMAGE_INFO	pImageInf = GetInfo( ) ;
	if ( (m_apiUpdateLayeredWindow == NULL) || (pImageInf == NULL) )
	{
		return	eslErrGeneral ;
	}
	SIZE	szWndDst = { (LONG) pImageInf->dwImageWidth, (LONG) pImageInf->dwImageHeight } ;
	POINT	ptWndSrc = { 0, 0 } ;
	HDC		hdcSrc = ::eglGetDC( pImageInf ) ;
	if ( hdcSrc == NULL )
	{
		return	eslErrGeneral ;
	}
	Lock( ) ;
	Refresh( ) ;
/*	if ( m_fImageStretching )
	{
		POINT	ptWndDst ;
		ptWndDst.x = m_ptScreenBase.x ;
		ptWndDst.y = m_ptScreenBase.y ;
		szWndDst.cx = m_szScreenStretched.w ;
		szWndDst.cy = m_szScreenStretched.h ;
		m_apiUpdateLayeredWindow
			( *pWnd, NULL, &ptWndDst, &szWndDst,
				hdcSrc, &ptWndSrc, 0, &m_bfLayeredWindow, ULW_ALPHA ) ;
	}
	else
*/	{
		m_apiUpdateLayeredWindow
			( *pWnd, NULL, NULL, &szWndDst,
				hdcSrc, &ptWndSrc, 0, &m_bfLayeredWindow, ULW_ALPHA ) ;
	}
	m_fPendingUpdateWindow = false ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 画像からリージョン生成
//////////////////////////////////////////////////////////////////////////////
HRGN EWindowSpriteInterface::CreateRegionFromImage
			( PEGL_IMAGE_INFO pImage, int nThreashold )
{
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	if ( !(pImage->fdwFormatType & (EIF_WITH_CLIPPING | EIF_WITH_ALPHA))
						&& (pImage->fdwFormatType != EIF_GRAY_BITMAP) )
	{
		//
		// 矩形リージョン
		//
		return	::CreateRectRgn
			( 0, 0, pImage->dwImageWidth, pImage->dwImageHeight ) ;
	}
	//
	// 複雑なリージョン生成のためのメモリを確保
	//
	int		nWidth, nHeight, iNext ;
	DWORD	fdwFormatType, dwBitsPerPixel, dwClippedPixel ;
	RGNDATA *	pRgnData ;
	RECT *		pRect ;
	nWidth = pImage->dwImageWidth ;
	nHeight = pImage->dwImageHeight ;
	fdwFormatType = pImage->fdwFormatType ;
	dwBitsPerPixel = pImage->dwBitsPerPixel ;
	dwClippedPixel = pImage->dwClippedPixel ;
	pRgnData = (RGNDATA*) ::eslHeapAllocate
		( NULL, sizeof(RGNDATA)
				+ sizeof(RECT) * ((nWidth + 1) / 2 * nHeight), 0 ) ;
	pRect = (RECT*) &(pRgnData->Buffer[0]) ;
	iNext = 0 ;
	//
	RECT *	pLastLine = (RECT*)
		::eslHeapAllocate( NULL, sizeof(RECT) * ((nWidth + 1) / 2), 0 ) ;
	int		nLastLineCount = 0 ;
	RECT *	pCurLine = (RECT*)
		::eslHeapAllocate( NULL, sizeof(RECT) * ((nWidth + 1) / 2), 0 ) ;
	int	iCurLine, iLastLine ;
	//
	// 順次ラインを走査
	//
	for ( int y = 0; y < nHeight; y ++ )
	{
		BYTE *	pbytLine =
			((BYTE*) pImage->ptrImageArray) + y * pImage->dwBytesPerLine ;
		iCurLine = 0 ;
		iLastLine = 0 ;
		//
		for ( int x = 0; x < nWidth; x ++ )
		{
			//
			// "抜け"ていないピクセルの範囲を走査
			//
			int		nLeft ;
			if ( dwBitsPerPixel == 32 )
			{
				if ( fdwFormatType & EIF_WITH_ALPHA )
				{
					// RGBA-32 ： αチャネル判定
					while ( x < nWidth )
					{
						if ( pbytLine[x * 4 + 3] >= (BYTE) nThreashold )
							break ;
						x ++ ;
					}
					if ( x >= nWidth )
						break ;
					//
					nLeft = x ;
					while ( ++ x < nWidth )
					{
						if ( pbytLine[x * 4 + 3] < (BYTE) nThreashold )
							break ;
					}
				}
				else
				{
					// RGB-32 ： 抜け色判定
					while ( x < nWidth )
					{
						if ( ((DWORD*)pbytLine)[x] != dwClippedPixel )
							break ;
						x ++ ;
					}
					if ( x >= nWidth )
						break ;
					//
					nLeft = x ;
					while ( ++ x < nWidth )
					{
						if ( ((DWORD*)pbytLine)[x] == dwClippedPixel )
							break ;
					}
				}
			}
			else if ( dwBitsPerPixel == 8 )
			{
				if ( fdwFormatType == EIF_GRAY_BITMAP )
				{
					// Gray-8 ： 明度判定
					while ( x < nWidth )
					{
						if ( pbytLine[x] >= (BYTE) nThreashold )
							break ;
						x ++ ;
					}
					if ( x >= nWidth )
						break ;
					//
					nLeft = x ;
					while ( ++ x < nWidth )
					{
						if ( pbytLine[x] < (BYTE) nThreashold )
							break ;
					}
				}
				else
				{
					// Index-8 ： 抜け色判定
					while ( x < nWidth )
					{
						if ( pbytLine[x] != dwClippedPixel )
							break ;
						x ++ ;
					}
					if ( x >= nWidth )
						break ;
					//
					nLeft = x ;
					while ( ++ x < nWidth )
					{
						if ( pbytLine[x] == dwClippedPixel )
							break ;
					}
				}
			}
			else if ( dwBitsPerPixel == 24 )
			{
				// RGB-24 ： 抜け色判定
				DWORD	dwPixel ;
				BYTE *	pNextPixel = pbytLine + x * 3 ;
				while ( x < nWidth )
				{
					dwPixel = *((WORD*) pNextPixel)
							| (((DWORD) pNextPixel[2]) << 16) ;
					if ( dwPixel != dwClippedPixel )
						break ;
					x ++ ;
					pNextPixel += 3 ;
				}
				if ( x >= nWidth )
					break ;
				//
				nLeft = x ;
				pNextPixel += 3 ;
				while ( ++ x < nWidth )
				{
					dwPixel = *((WORD*) pNextPixel)
							| (((DWORD) pNextPixel[2]) << 16) ;
					if ( dwPixel == dwClippedPixel )
						break ;
					pNextPixel += 3 ;
				}
			}
			else if ( dwBitsPerPixel == 16 )
			{
				// RGB-16 ： 抜け色判定
				WORD *	pwLine = (WORD*) pbytLine ;
				while ( x < nWidth )
				{
					if ( pwLine[x] != dwClippedPixel )
						break ;
					x ++ ;
				}
				if ( x >= nWidth )
					break ;
				//
				nLeft = x ;
				while ( ++ x < nWidth )
				{
					if ( pwLine[x] == dwClippedPixel )
						break ;
				}
			}
			//
			// 直前ラインと一致する領域が無いか判定する
			//
			while ( iLastLine < nLastLineCount )
			{
				if ( pLastLine[iLastLine].right >= x )
					break ;
				//
				pRect[iNext ++] = pLastLine[iLastLine ++] ;
			}
			//
			if ( (iLastLine < nLastLineCount)
				&& (pLastLine[iLastLine].left == nLeft)
				&& (pLastLine[iLastLine].right == x) )
			{
				pCurLine[iCurLine] = pLastLine[iLastLine ++] ;
				pCurLine[iCurLine].bottom = y + 1 ;
				iCurLine ++ ;
			}
			else
			{
				pCurLine[iCurLine].left = nLeft ;
				pCurLine[iCurLine].top = y ;
				pCurLine[iCurLine].right = x ;
				pCurLine[iCurLine].bottom = y + 1 ;
				iCurLine ++ ;
			}
		}
		//
		// 直前ラインの残りをバッファに追加する
		//
		while ( iLastLine < nLastLineCount )
		{
			pRect[iNext ++] = pLastLine[iLastLine ++] ;
		}
		//
		// 直前ラインをスワップする
		//
		RECT *	pTemp = pLastLine ;
		pLastLine = pCurLine ;
		nLastLineCount = iCurLine ;
		pCurLine = pTemp ;
	}
	//
	// 直前ラインの残りをバッファに追加する
	//
	iLastLine = 0 ;
	while ( iLastLine < nLastLineCount )
	{
		pRect[iNext ++] = pLastLine[iLastLine ++] ;
	}
	//
	// リージョンを生成
	//
	HRGN	hRgn ;
	pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER) ;
	pRgnData->rdh.iType = RDH_RECTANGLES ;
	pRgnData->rdh.nCount = iNext ;
	pRgnData->rdh.nRgnSize = iNext * sizeof(RECT) ;
	pRgnData->rdh.rcBound.left = 0 ;
	pRgnData->rdh.rcBound.top = 0 ;
	pRgnData->rdh.rcBound.right = nWidth ;
	pRgnData->rdh.rcBound.bottom = nHeight ;
	//
	hRgn = ::ExtCreateRegion
		( NULL, sizeof(RGNDATAHEADER) + iNext * sizeof(RECT), pRgnData ) ;
	::eslHeapFree( NULL, pRgnData ) ;
	::eslHeapFree( NULL, pLastLine ) ;
	::eslHeapFree( NULL, pCurLine ) ;
	//
	return	hRgn ;
}

// 関連レイアウトウィンドウ登録
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::AttachRelativeLayoutWindow( HWND hWnd )
{
	Lock() ;
	if ( m_lstRelativeLayout.Find( hWnd ) < 0 )
	{
		m_lstRelativeLayout.Add( hWnd ) ;
	}
	Unlock() ;
}

// 関連レイアウトウィンドウ解除
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::DetachRelativeLayoutWindow( HWND hWnd )
{
	Lock() ;
	int	iFind = m_lstRelativeLayout.Find( hWnd ) ;
	if ( iFind >= 0 )
	{
		m_lstRelativeLayout.RemoveAt( iFind ) ;
	}
	Unlock() ;
}

// 関連レイアウトウィンドウに通知メッセージ送信
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::PostNotifyRelativeLayout( void )
{
	EWindow *	pWnd = GetWindow() ;
	HWND	hWndRelative = NULL ;
	if ( pWnd != NULL )
	{
		hWndRelative = *pWnd ;
	}
	Lock() ;
	for ( int i = 0; i < (int) m_lstRelativeLayout.GetSize(); i ++ )
	{
		HWND	hWnd = m_lstRelativeLayout.GetAt( i ) ;
		if ( (hWnd != NULL) && ::IsWindow( hWnd ) )
		{
			::PostMessage
				( hWnd, wmNotifyLayout, 0, (LPARAM) hWndRelative ) ;
		}
		else
		{
			m_lstRelativeLayout.RemoveAt( i -- ) ;
		}
	}
	Unlock() ;
}

// レイアウト設定
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::SetWindowLayout( int nFlags, int xPos, int yPos )
{
	EWindow *	pWndThis = GetWindow() ;
	if ( pWndThis == NULL )
	{
		return ;
	}
	if ( m_pSyncTarget == NULL )
	{
		pWndThis->SetWindowPos
			( HWND_TOP, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE ) ;
		return ;
	}
	EWindow *	pWndRelative = m_pSyncTarget->GetWindow() ;
	if ( pWndRelative == NULL )
	{
		return ;
	}
	//
	m_flagLayout = nFlags ;
	m_ptLayoutOffset.x = xPos ;
	m_ptLayoutOffset.y = yPos ;
	//
	if ( (m_flagLayout & dockingMask) == layoutNothing )
	{
		m_pSyncTarget->DetachRelativeLayoutWindow( *pWndThis ) ;
	}
	else
	{
		m_pSyncTarget->AttachRelativeLayoutWindow( *pWndThis ) ;
		UpdateWindowLayout( *pWndRelative ) ;
	}
}

// レイアウト反映
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::UpdateWindowLayout( HWND hRelativeWnd )
{
	EWindow *	pWnd = GetWindow() ;
	if ( (pWnd == NULL) || !::IsWindow( *pWnd ) )
	{
		return ;
	}
	if ( (m_flagLayout & dockingMask) == layoutNothing )
	{
		pWnd->SetWindowPos
			( HWND_TOP, m_ptLayoutOffset.x, m_ptLayoutOffset.y,
								0, 0, SWP_NOZORDER | SWP_NOSIZE ) ;
		return ;
	}
	if ( (hRelativeWnd == NULL) || !::IsWindow( hRelativeWnd ) )
	{
		return ;
	}
	PEGL_IMAGE_INFO	pImageInf = GetInfo() ;
	if ( pImageInf == NULL )
	{
		return ;
	}
	//
	// レイアウト基準座標取得
	//
	RECT	rectRelative ;
	RECT	rectWindow ;
	::GetWindowRect( hRelativeWnd, &rectWindow ) ;
	if ( m_flagLayout & alignTypeWindow )
	{
		rectRelative = rectWindow ;
	}
	else
	{
		POINT	ptClient = { 0, 0 } ;
		::GetClientRect( hRelativeWnd, &rectRelative ) ;
		::ClientToScreen( hRelativeWnd, &ptClient ) ;
		rectRelative.left += ptClient.x ;
		rectRelative.top += ptClient.y ;
		rectRelative.right += ptClient.x ;
		rectRelative.bottom += ptClient.y ;
	}
	EGLSize	sizeRelative ;
	sizeRelative.w = rectRelative.right - rectRelative.left ;
	sizeRelative.h = rectRelative.bottom - rectRelative.top ;
	//
	// 自分自身のウィンドウ座標とフレーム幅を取得
	//
	RECT	rectThisWindow ;
	RECT	rectThisClient ;
	RECT	rectThisFrame ;
	POINT	ptThisClient = { 0, 0 } ;
	pWnd->GetWindowRect( &rectThisWindow ) ;
	pWnd->GetClientRect( &rectThisClient ) ;
	pWnd->ClientToScreen( &ptThisClient ) ;
	rectThisFrame.left = ptThisClient.x - rectThisWindow.left ;
	rectThisFrame.top = ptThisClient.y - rectThisWindow.top ;
	rectThisFrame.right =
		rectThisWindow.right - (rectThisClient.right + ptThisClient.x) ;
	rectThisFrame.bottom =
		rectThisWindow.bottom - (rectThisClient.bottom + ptThisClient.y) ;
	//
	// レイアウト処理
	//
	DWORD		dwFlags = SWP_NOZORDER ;
	POINT		ptLayout = { m_ptLayoutOffset.x, m_ptLayoutOffset.y } ;
	EGLSize		sizeWnd( pImageInf->dwImageWidth, pImageInf->dwImageHeight ) ;
	switch ( m_flagLayout & dockingMask )
	{
	case	layoutNothing:
		break ;
	case	offsetClient:
		ptLayout.x += rectRelative.left ;
		ptLayout.y += rectRelative.top ;
		sizeWnd.w = rectThisClient.right - rectThisClient.left ;
		sizeWnd.h = rectThisClient.bottom - rectThisClient.top ;
		break ;
	case	dockingLeft:
	case	dockingRight:
		switch ( m_flagLayout & alignMask )
		{
		case	alignTop:
			ptLayout.y += rectRelative.top ;
			break ;
		case	alignCenter:
			ptLayout.y +=
				rectRelative.top
					+ (sizeRelative.h - sizeWnd.h) / 2 ;
			break ;
		case	alignBottom:
			ptLayout.y +=
				rectRelative.top
					+ (sizeRelative.h - sizeWnd.h) ;
			break ;
		case	alignAccording:
			sizeWnd.w = sizeRelative.h * sizeWnd.w / sizeWnd.h ;
			sizeWnd.h = sizeRelative.h ;
			ptLayout.y += rectRelative.top ;
			break ;
		}
		if ( !(m_flagLayout & alignTypeWindow) )
		{
			ptLayout.y -= rectThisFrame.top ;
		}
		if ( (m_flagLayout & dockingMask) == dockingLeft )
		{
			ptLayout.x =
				rectWindow.left
					- (sizeWnd.w + rectThisFrame.left + rectThisFrame.right) ;
		}
		else
		{
			ptLayout.x = rectWindow.right ;
		}
		break ;
	case	dockingUpper:
	case	dockingUnder:
		switch ( m_flagLayout & alignMask )
		{
		case	alignLeft:
			ptLayout.x += rectRelative.left ;
			break ;
		case	alignCenter:
			ptLayout.x +=
				rectRelative.left
					+ (sizeRelative.w - sizeWnd.w) / 2 ;
			break ;
		case	alignRight:
			ptLayout.x +=
				rectRelative.left
					+ (sizeRelative.w - sizeWnd.w) ;
			break ;
		case	alignAccording:
			sizeWnd.h = sizeRelative.w * sizeWnd.h / sizeWnd.w ;
			sizeWnd.w = sizeRelative.h ;
			ptLayout.x += rectRelative.left ;
			break ;
		}
		if ( !(m_flagLayout & alignTypeWindow) )
		{
			ptLayout.x -= rectThisFrame.left ;
		}
		if ( (m_flagLayout & dockingMask) == dockingUpper )
		{
			ptLayout.y =
				rectWindow.top
					- (sizeWnd.h + rectThisFrame.top + rectThisFrame.bottom) ;
		}
		else
		{
			ptLayout.y = rectWindow.bottom ;
		}
		break ;
	}
	//
	// 位置の正規化
	//
	SIZE	sizeWindow ;
	sizeWindow.cx = sizeWnd.w + rectThisFrame.left + rectThisFrame.right ;
	sizeWindow.cy = sizeWnd.h + rectThisFrame.top + rectThisFrame.bottom ;
	//
	m_dmMonitorAPIs.NormalizeWindowPos( ptLayout, sizeWindow ) ;
	//
	pWnd->SetWindowPos
		( HWND_TOP, ptLayout.x, ptLayout.y,
					sizeWindow.cx, sizeWindow.cy, dwFlags ) ;
}

// レイアウト通知処理
//////////////////////////////////////////////////////////////////////////////
void EWindowSpriteInterface::OnNotifyLayout( HWND hRelativeWnd )
{
	UpdateWindowLayout( hRelativeWnd ) ;
}
