
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2013 Leshade Entis. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <ddraw.h>
#include <d3d9.h>
//#include <dxerr9.h>
#include <dxerr.h>


//////////////////////////////////////////////////////////////////////////////
// ゲームアクセラレーションウィンドウ
//////////////////////////////////////////////////////////////////////////////

static const char *	ENTIS_GAME_WINDOW_CLASS = "EntisGLS_GameWindow" ;

#define	ENABLE_GLS_FULLSCREEN	1

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGameWindow, EWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGameWindow::EGameWindow( EWindowInterface * pWUI )
	: EWindow( pWUI ), m_flagCreated( FALSE ),
		m_fOptionFunctions( EGameWindow::BlackBack ), m_hIMC( NULL )
{
	m_flagNoChangeMode = false ;
	m_ptMonitorUpperLeft.x = 0 ;
	m_ptMonitorUpperLeft.y = 0 ;
	m_pivView3D = NULL ;
	m_fFullscreenByView3D = false ;
	m_fControlWindowByView3D = false ;
	m_fEnableStereo3D = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGameWindow::~EGameWindow( void )
{
	if ( m_flagCreated )
	{
		CloseDisplay( ) ;
	}
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EGameWindow::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( m_pivView3D != NULL )
	{
		using	namespace E3DSDisplayPlugin ;
		LRESULT	lrResult ;
		WindowProcMethod	wpmMethod =
			m_pivView3D->WindowProc( m_hWnd, uMsg, wParam, lParam, &lrResult ) ;
		if ( wpmMethod == methodReturnProc )
		{
			return	lrResult ;
		}
	}
	switch( uMsg )
	{
#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	case	WM_TIMER:
		//
		// To set foreground window
		//
		if ( m_flagDeactivate && (wParam == 0)
				&& (m_fCooperationLevel & flagExclusive) )
		{
			::SetForegroundWindow( m_hWnd ) ;
			m_flagDeactivate = FALSE ;
			//
			if( (m_sizeFullScreen.cx != ::GetSystemMetrics( SM_CXSCREEN ))
				|| (m_sizeFullScreen.cy != ::GetSystemMetrics( SM_CYSCREEN )) )
			{
				ChangeDisplayMode
					( m_sizeDisplay.cx, m_sizeDisplay.cy,
								m_nBitsPerPixel, m_nFrequency ) ;
			}
		}
		break ;

	case	WM_PAINT:
		UpdateClientDisplayPosition( false ) ;
		break ;

	case	WM_SYSCOMMAND:
		//
		// System command message processing
		//
		switch( wParam )
		{
		case	SC_MOVE:
		case	SC_SIZE:
		case	SC_TASKLIST:
			if ( m_fCooperationLevel & flagFullScreen )
			{
				return	0 ;
			}
			break ;

		case	SC_RESTORE:
			if ( m_fCooperationLevel & flagFullScreen )
			{
				return	0 ;
			}
			break ;
		case	SC_MINIMIZE:
			if ( m_fOptionFunctions & AllowMinimize )
			{
				break ;
			}
			return	0 ;
		case	SC_MAXIMIZE:
			if ( !(m_fCooperationLevel & flagFullScreen)
					&& (m_fOptionFunctions & (VariableWindowSize | NoAutoFitSize))
					&& (m_fOptionFunctions & AllowMaximize) )
			{
				break ;
			}
		case	SC_MOUSEMENU:
			return	0 ;

		case	SC_SCREENSAVE:
			if ( m_fOptionFunctions & GrantScreenSave )
			{
				break ;
			}
			return	0 ;

		case	SC_MONITORPOWER:
			if ( m_fOptionFunctions & GrantMonitorSave )
			{
				break ;
			}
			return	0 ;

		case	SC_CLOSE:
			if ( m_fOptionFunctions & AllowClose )
			{
				break ;
			}
			return	0 ;
		}
		return	EWindow::WindowProc( uMsg, wParam, lParam ) ;

	case	WM_POWERBROADCAST:
		if ( !(m_fOptionFunctions & GrantPowerSuspend) )
		{
			return	BROADCAST_QUERY_DENY ;
		}
		break ;
#endif

	case	WM_SETCURSOR:
		if ( LOWORD(lParam) != HTCLIENT )
		{
			if ( m_fOptionFunctions & VariableWindowSize )
			{
				switch ( LOWORD(lParam) )
				{
				case	HTLEFT:
				case	HTRIGHT:
					::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) ) ;
					break ;
				case	HTTOP:
				case	HTBOTTOM:
					::SetCursor( ::LoadCursor( NULL, IDC_SIZENS ) ) ;
					break ;
				case	HTTOPLEFT:
				case	HTBOTTOMRIGHT:
					::SetCursor( ::LoadCursor( NULL, IDC_SIZENWSE ) ) ;
					break ;
				case	HTTOPRIGHT:
				case	HTBOTTOMLEFT:
					::SetCursor( ::LoadCursor( NULL, IDC_SIZENESW ) ) ;
					break ;
				default:
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) ) ;
					break ;
				}
			}
			else
			{
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) ) ;
			}
			return	0 ;
		}
		break ;

	case	WM_GETMINMAXINFO:
		//
		// To get minimum and maximum window information
		//
		{
			LPMINMAXINFO	lpmmi = (LPMINMAXINFO)lParam ;
			if ( (m_fCooperationLevel & flagFullScreen)
				|| !(m_fOptionFunctions & (VariableWindowSize | NoAutoFitSize))
				|| !(m_fOptionFunctions & AllowMaximize) )
			{
				lpmmi->ptMaxSize.x = m_sizeFullScreen.cx ;
				lpmmi->ptMaxSize.y = m_sizeFullScreen.cy ;
				lpmmi->ptMaxPosition = m_ptMonitorUpperLeft ;
			}
			lpmmi->ptMinTrackSize.x = m_sizeDisplay.cx / 2 ;
			lpmmi->ptMinTrackSize.y = m_sizeDisplay.cy / 2 ;
		}
		return	0 ;

	case	WM_ACTIVATE:
		if ( (wParam & 0xFFFF) == WA_INACTIVE )
		{
			DWORD	dwActiveProcID, dwProcessID ;
			::GetWindowThreadProcessId( (HWND)lParam, &dwActiveProcID ) ;
			::GetWindowThreadProcessId( m_hWnd, &dwProcessID ) ;
			if ( dwActiveProcID == dwProcessID )
			{
				break ;
			}
			wParam = FALSE ; /*(dwActiveProcID == dwProcessID)*/ ;
		}
		else
		{
			wParam = TRUE ;
		}

//	case	WM_ACTIVATEAPP:
		//
		// Processing when activated or deactivated
		//
		if ( wParam == FALSE )
		{
			//
			// When deactivated
			//
			if ( m_flagActivated /*&& (m_hWnd != (HWND)lParam)
				&& !::IsChild( m_hWnd, (HWND)lParam )*/ )
			{
				#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
				if ( m_fCooperationLevel & flagExclusive )
				{
					m_flagDeactivate = TRUE ;
					PostMessage( WM_TIMER, 0, 0 ) ;
				}
				else
				#endif
				{
					m_flagActivated = FALSE ;
					if ( m_fCooperationLevel & flagMouseCapturing )
					{
						::ClipCursor( NULL ) ;
					}
					if ( m_fCooperationLevel & flagFullScreen )
					{
						if ( m_fFullscreenByView3D && m_pivView3D )
						{
							if ( m_pivView3D->OnRestoreDisplayMode()
									== E3DSDisplayPlugin::methodNoModeChanged )
							{
								m_DisplayMode.RestoreDisplayMode( ) ;
							}
							m_fFullscreenByView3D = false ;
							m_fControlWindowByView3D = false ;
						}
						else
						{
							m_DisplayMode.RestoreDisplayMode( ) ;
						}
						ShowWindow( SW_SHOWMINIMIZED ) ;
					}
				}
				OnDeactivate( ) ;
			}
		}
		else
		{
			//
			// When activated
			//
			if ( !m_flagActivated )
			{
				m_flagActivated = TRUE ;
				#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
				if ( m_fCooperationLevel & flagExclusive )
				{
					if( (m_sizeFullScreen.cx
							!= ::GetSystemMetrics( SM_CXSCREEN )) ||
						(m_sizeFullScreen.cy
							!= ::GetSystemMetrics( SM_CYSCREEN )) )
					{
						ChangeDisplayMode
							( m_sizeDisplay.cx, m_sizeDisplay.cy,
										m_nBitsPerPixel, m_nFrequency ) ;
					}
				}
				else
				#endif
				{
					if ( m_fCooperationLevel & flagFullScreen )
					{
						ChangeDisplayMode
							( m_sizeDisplay.cx, m_sizeDisplay.cy,
										m_nBitsPerPixel, m_nFrequency ) ;
						if ( !m_fControlWindowByView3D )
						{
//						#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
//							ShowWindow( SW_SHOWMAXIMIZED ) ;
//						#else
							ShowWindow( SW_SHOWNORMAL ) ;
//						#endif
						}
					}
					if ( (m_fCooperationLevel & flagMouseCapturing)
											&& !m_fControlWindowByView3D )
					{
						RECT	rectClient ;
						GetClientRect( &rectClient ) ;
						POINT	pointOffset = { 0, 0 } ;
						EWindow::ClientToScreen( &pointOffset ) ;
						rectClient.left += pointOffset.x ;
						rectClient.top += pointOffset.y ;
						rectClient.right += pointOffset.x - 1 ;
						rectClient.bottom += pointOffset.y - 1 ;
						::ClipCursor( &rectClient ) ;
						PostMessage( WM_SETCURSOR, (WPARAM)m_hWnd, 0 ) ;
					}
				}
				if ( !(m_fOptionFunctions & EnableIME) )
				{
					::ImmAssociateContext( m_hWnd, NULL ) ;
				}
				OnActivate( ) ;
			}
		}
		break ;

	case	WM_SIZE:
		#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
		if ( !(m_fCooperationLevel & flagFullScreen) )
		#endif
		{
			if ( m_fOptionFunctions & (VariableWindowSize | NoAutoFitSize) )
			{
				UpdateClientDisplayPosition() ;
			}
			else
			{
				RECT	rctClient, rctWindow ;
				GetClientRect( &rctClient ) ;
				rctClient.right -= rctClient.left ;
				rctClient.bottom -= rctClient.top ;
				if ( (m_sizeDisplay.cx != rctClient.right)
					|| (m_sizeDisplay.cy != rctClient.bottom) )
				{
					GetWindowRect( &rctWindow ) ;
					rctWindow.right += m_sizeDisplay.cx - rctClient.right ;
					rctWindow.bottom += m_sizeDisplay.cy - rctClient.bottom ;
					SetWindowPos
						( NULL, 0, 0,
							rctWindow.right - rctWindow.left,
							rctWindow.bottom - rctWindow.top,
									SWP_NOMOVE | SWP_NOZORDER ) ;
				}
				UpdateClientDisplayPosition( false ) ;
			}
		}
	case	WM_MOVE:
		//
		// Processing when the window moved
		//
		if ( m_flagActivated && (m_fCooperationLevel & flagMouseCapturing) )
		{
			if ( !m_fControlWindowByView3D )
			{
				RECT	rectClient ;
				GetClientRect( &rectClient ) ;
				POINT	pointOffset = { 0, 0 } ;
				EWindow::ClientToScreen( &pointOffset ) ;
				rectClient.left += pointOffset.x ;
				rectClient.top += pointOffset.y ;
				rectClient.right += pointOffset.x ;
				rectClient.bottom += pointOffset.y ;
				::ClipCursor( &rectClient ) ;
			}
			else
			{
				::ClipCursor( NULL ) ;
			}
		}
		break ;

	case	WM_SIZING:
		if ( ShouldAdjustWindowAscept() )
		{
			RECT *	pRect = (RECT*) lParam ;
			RECT	rectWindow, rectClient ;
			::GetWindowRect( m_hWnd, &rectWindow ) ;
			::GetClientRect( m_hWnd, &rectClient ) ;
			//
			EGLSize	sizeClient
				( rectClient.right - rectClient.left,
					rectClient.bottom - rectClient.top ) ;
			SIZE	sizeWindow =
				{ rectWindow.right - rectWindow.left,
					rectWindow.bottom - rectWindow.top } ;
			//
			sizeClient.w = pRect->right - pRect->left
									- (sizeWindow.cx - sizeClient.w) ;
			sizeClient.h = pRect->bottom - pRect->top
									- (sizeWindow.cy - sizeClient.h) ;
			//
			EGLSize	sizeVirtual( m_sizeDisplay.cx, m_sizeDisplay.cy ) ;
			EGLSize	sizeNormalized = sizeClient ;
			bool	fSizeVirtical = false ;
			switch ( wParam )
			{
			case	WMSZ_RIGHT:
			case	WMSZ_LEFT:
				fSizeVirtical = true ;
				break ;
			case	WMSZ_BOTTOM:
			case	WMSZ_TOP:
				fSizeVirtical = false ;
				break ;
			default:
				fSizeVirtical =
					!(sizeClient.w * sizeVirtual.h
							< sizeClient.h * sizeVirtual.w) ;
				break ;
			}
			if ( fSizeVirtical )
			{
				sizeNormalized.h =
					sizeVirtual.h * sizeClient.w / sizeVirtual.w ;
			}
			else
			{
				sizeNormalized.w =
					sizeVirtual.w * sizeClient.h / sizeVirtual.h ;
			}
			EGLSize	sizeDelta
				( sizeNormalized.w - sizeClient.w,
					sizeNormalized.h - sizeClient.h ) ;
			//
			switch ( wParam )
			{
			case	WMSZ_BOTTOM:
			case	WMSZ_RIGHT:
			case	WMSZ_BOTTOMRIGHT:
				pRect->right += sizeDelta.w ;
				pRect->bottom += sizeDelta.h ;
				break ;
			case	WMSZ_TOPRIGHT:
				pRect->right += sizeDelta.w ;
				pRect->top -= sizeDelta.h ;
				break ;
			case	WMSZ_LEFT:
			case	WMSZ_TOP:
			case	WMSZ_TOPLEFT:
				pRect->left -= sizeDelta.w ;
				pRect->top -= sizeDelta.h ;
				break ;
			case	WMSZ_BOTTOMLEFT:
				pRect->left -= sizeDelta.w ;
				pRect->bottom += sizeDelta.h ;
				break ;
			}
			POINT	ptWindow = { pRect->left, pRect->top } ;
			sizeWindow.cx = pRect->right - pRect->left ;
			sizeWindow.cy = pRect->bottom - pRect->top ;
			//
			if ( !(m_fOptionFunctions & NoNormalizePos) )
			{
				m_DisplayMode.NormalizeWindowPos( ptWindow, sizeWindow, false ) ;
			}
			//
			pRect->left = ptWindow.x ;
			pRect->top = ptWindow.y ;
			pRect->right = ptWindow.x + sizeWindow.cx ;
			pRect->bottom = ptWindow.y + sizeWindow.cy ;
			return	TRUE ;
		}
		break ;

	case	WM_DEVMODECHANGE:
		//
		// Device mode is changed
		//
		#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
			if( m_flagActivated || (m_fCooperationLevel & flagExclusive) )
			{
				ChangeDisplayMode
					( m_sizeDisplay.cx, m_sizeDisplay.cy,
								m_nBitsPerPixel, m_nFrequency ) ;
			}
		#endif
		break ;

	case	WM_SYSKEYDOWN:
		if ( (m_fCooperationLevel & flagFullScreen)
				&& (m_fOptionFunctions & AllowClose) )
		{
			int	nVirtKey = (int) wParam ;
			if ( (nVirtKey == VK_F4) && (GetKeyState(VK_MENU) != 0) )
			{
				PostMessage( WM_CLOSE, 0, 0 ) ;
				return	0 ;
			}
		}
		break ;

	case	WM_DESTROY:
		if ( !m_strWndClass.IsEmpty() )
		{
			::UnregisterClass( m_strWndClass, ::GetModuleHandle( NULL ) ) ;
		}
		break ;
	}
	if ( uMsg == m_uMsgSetOptFlag )
	{
		SetOptionalFuncFlag( wParam ) ;
	}

	return	EWindow::WindowProc( uMsg, wParam, lParam ) ;
}

// ウィンドウのクライアント表示座標を更新する
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::UpdateClientDisplayPosition( bool flagAdjustAspect )
{
	RECT	rctClient ;
	GetClientRect( &rctClient ) ;
	rctClient.right -= rctClient.left ;
	rctClient.bottom -= rctClient.top ;
	//
	if ( rctClient.right * m_sizeDisplay.cy
				>= rctClient.bottom * m_sizeDisplay.cx )
	{
		m_sizeScreen.cy = rctClient.bottom ;
		m_sizeScreen.cx =
			m_sizeScreen.cy * m_sizeDisplay.cx / m_sizeDisplay.cy ;
		m_pointOffset.x = (rctClient.right - m_sizeScreen.cx) / 2 ;
		m_pointOffset.y = 0 ;
		//
		if ( (m_pointOffset.x != 0)
			&& flagAdjustAspect
			&& ShouldAdjustWindowAscept() )
		{
			RECT	rctWindow ;
			GetWindowRect( &rctWindow ) ;
			//
			::MoveWindow
				( m_hWnd, rctWindow.left + m_pointOffset.x, rctWindow.top,
					(rctWindow.right - rctWindow.left)
							- (rctClient.right - m_sizeScreen.cx),
					rctWindow.bottom - rctWindow.top, TRUE ) ;
			m_pointOffset.x = 0 ;
		}
	}
	else
	{
		m_sizeScreen.cx = rctClient.right ;
		m_sizeScreen.cy =
			m_sizeScreen.cx * m_sizeDisplay.cy / m_sizeDisplay.cx ;
		m_pointOffset.x = 0 ;
		m_pointOffset.y = (rctClient.bottom - m_sizeScreen.cy) / 2 ;
		//
		if ( (m_pointOffset.y != 0)
			&& flagAdjustAspect
			&& ShouldAdjustWindowAscept() )
		{
			RECT	rctWindow ;
			GetWindowRect( &rctWindow ) ;
			//
			::MoveWindow
				( m_hWnd, rctWindow.left, rctWindow.top + m_pointOffset.y,
					rctWindow.right - rctWindow.left,
					(rctWindow.bottom - rctWindow.top)
						- (rctClient.bottom - m_sizeScreen.cy), TRUE ) ;
			m_pointOffset.x = 0 ;
		}
	}
}

// ウィンドウのアスペクト比を調整すべきか？
//////////////////////////////////////////////////////////////////////////////
bool EGameWindow::ShouldAdjustWindowAscept( void ) const
{
	return	(m_fOptionFunctions & AutoWindowAscept)
			&& !(m_fCooperationLevel & flagFullScreen)
			&& !(::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_MAXIMIZE)
			&& !(::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_MINIMIZE) ;
}

// デフォルトウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EGameWindow::DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_SETCURSOR )
	{
		::SetCursor( NULL ) ;
		return	0 ;
	}
	else if ( uMsg == WM_CLOSE )
	{
		return	0 ;
	}
	return	EWindow::DefWindowProc( uMsg, wParam, lParam ) ;
}

// ウィンドウがアクティブになった
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::OnActivate( void )
{
}

// ウィンドウが非アクティブになった
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::OnDeactivate( void )
{
}

// メッセージを処理する
//////////////////////////////////////////////////////////////////////////////
BOOL EGameWindow::HandleMessage
	( DWORD dwTimeout, HWND hWnd, UINT uMsgFilterMin, UINT uMsgFilterMax )
{
	MSG		msg ;
	DWORD	dwTimeoutTime = ::GetCurrentTime() + dwTimeout ;
	if ( dwTimeout == INFINITE )
	{
		while ( ::GetMessage( &msg, hWnd, uMsgFilterMin, uMsgFilterMax ) )
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
		return	TRUE ;
	}
	else
	{
		for ( ; ; )
		{
			while ( ::PeekMessage
				( &msg, hWnd, uMsgFilterMin, uMsgFilterMax, PM_NOREMOVE ) )
			{
				if ( ::GetMessage( &msg, hWnd, uMsgFilterMin, uMsgFilterMax ) )
				{
					::TranslateMessage( &msg ) ;
					::DispatchMessage( &msg ) ;
				}
				else
				{
					return	TRUE ;
				}
				if ( (signed int)(msg.time - dwTimeoutTime) >= 0 )
				{
					return	FALSE ;
				}
			}
			if ( (signed int)(::GetCurrentTime() - dwTimeoutTime) >= 0 )
			{
				return	FALSE ;
			}
			::Sleep( 1 ) ;
		}
	}
}

// ディスプレイモードを検索し変更する
//////////////////////////////////////////////////////////////////////////////
ESLError EGameWindow::ChangeDisplayMode
	( unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency )
{
	//
	// Test that is just fit
	//
//	RECT			rctWindow ;
	WINDOWPLACEMENT	wp ;
	EString			strDisplayName ;
	const char *	pszDisplayName = NULL ;
	HMONITOR		hMonitor = NULL ;
	MONITORINFO		moninf ;
	moninf.cbSize = sizeof(moninf) ;
//	GetWindowRect( &rctWindow ) ;
	wp.length = sizeof(wp) ;
	GetWindowPlacement( &wp ) ;
	pszDisplayName =
		m_DisplayMode.GetDisplayNameFromRect
				( strDisplayName, &wp.rcNormalPosition, &hMonitor ) ;
	//
	m_sizeDisplay.cx = nWidth ;
	m_sizeDisplay.cy = nHeight ;
	m_sizeScreen = m_sizeDisplay ;
	//
	if ( m_flagNoChangeMode )
	{
		//
		// case without changing display mode
		//
		if ( !m_DisplayMode.GetMonitorInfo( hMonitor, &moninf ) )
		{
			m_ptMonitorUpperLeft.x = moninf.rcMonitor.left ;
			m_ptMonitorUpperLeft.y = moninf.rcMonitor.top ;
			m_sizeFullScreen.cx =
				moninf.rcMonitor.right - moninf.rcMonitor.left ;
			m_sizeFullScreen.cy =
				moninf.rcMonitor.bottom - moninf.rcMonitor.top ;
		}
		else
		{
			m_ptMonitorUpperLeft.x = 0 ;
			m_ptMonitorUpperLeft.y = 0 ;
			m_sizeFullScreen.cx = ::GetSystemMetrics( SM_CXSCREEN ) ;
			m_sizeFullScreen.cy = ::GetSystemMetrics( SM_CYSCREEN ) ;
		}
		m_nBitsPerPixel = EDisplayMode::GetDisplayColorMode( pszDisplayName ) ;
		m_nFrequency = EDisplayMode::GetDisplayFrequency( pszDisplayName ) ;
		//
		#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
			if ( m_fEnableStereo3D && (m_pivView3D != NULL) )
			{
				using namespace E3DSDisplayPlugin ;
				DisplayModeMethod	dmmResult =
					m_pivView3D->OnChangeDisplayMode
						( hMonitor,
							m_sizeFullScreen.cx,
							m_sizeFullScreen.cy,
							m_nBitsPerPixel, m_nFrequency ) ;
				m_fFullscreenByView3D =
						(dmmResult != methodNoModeChanged) ;
				m_fControlWindowByView3D =
						(dmmResult == methodModeChangedAndWindow) ;
			}
			//
			m_sizeScreen = m_sizeFullScreen ;
			//
			if ( m_sizeFullScreen.cx * m_sizeDisplay.cy
					> m_sizeFullScreen.cy * m_sizeDisplay.cx )
			{
				if ( m_sizeDisplay.cy > 0 )
				{
					m_sizeScreen.cx =
						m_sizeDisplay.cx * m_sizeFullScreen.cy / m_sizeDisplay.cy ;
				}
			}
			else
			{
				if ( m_sizeDisplay.cx > 0 )
				{
					m_sizeScreen.cy =
						m_sizeDisplay.cy * m_sizeFullScreen.cx / m_sizeDisplay.cx ;
				}
			}
		#else
			m_sizeScreen = m_sizeDisplay ;
		#endif
	}
	else
	{
		//
		// case with changing display mode
		//
		if ( m_fEnableStereo3D && (m_pivView3D != NULL) )
		{
			using namespace E3DSDisplayPlugin ;
			DisplayModeMethod	dmmResult =
				m_pivView3D->OnChangeDisplayMode
					( hMonitor, nWidth, nHeight,
						nBitsPerPixel, nFrequency ) ;
			if ( dmmResult != methodNoModeChanged )
			{
				m_sizeFullScreen = m_sizeDisplay ;
				m_nBitsPerPixel = nBitsPerPixel ;
				m_nFrequency = nFrequency ;
				//
				if ( !m_DisplayMode.GetMonitorInfo( hMonitor, &moninf ) )
				{
					m_ptMonitorUpperLeft.x = moninf.rcMonitor.left ;
					m_ptMonitorUpperLeft.y = moninf.rcMonitor.top ;
				}
				else
				{
					m_ptMonitorUpperLeft.x = 0 ;
					m_ptMonitorUpperLeft.y = 0 ;
				}
				m_fFullscreenByView3D = true ;
				m_fControlWindowByView3D =
						(dmmResult == methodModeChangedAndWindow) ;
				return	eslErrSuccess ;
			}
		}
		if ( !m_DisplayMode.TestDisplayMode
				( nWidth, nHeight, nBitsPerPixel, nFrequency, pszDisplayName ) )
		{
			#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
			if ( m_DisplayMode.ChangeDisplayMode( pszDisplayName ) == eslErrSuccess )
			#endif
			{
				m_sizeFullScreen = m_sizeDisplay ;
				m_nBitsPerPixel = m_DisplayMode.dmBitsPerPel ;
				m_nFrequency = m_DisplayMode.dmDisplayFrequency ;
				//
				if ( !m_DisplayMode.GetMonitorInfo( hMonitor, &moninf ) )
				{
					m_ptMonitorUpperLeft.x = moninf.rcMonitor.left ;
					m_ptMonitorUpperLeft.y = moninf.rcMonitor.top ;
				}
				else
				{
					m_ptMonitorUpperLeft.x = 0 ;
					m_ptMonitorUpperLeft.y = 0 ;
				}
				return	eslErrSuccess ;
			}
		}
		//
		// Enumerate display mode
		//
		DEVMODE			DevMode ;
		bool			fFoundMode = false ;
		unsigned int	nModeIndex = 0 ;
		unsigned int	nColorMode =
			EDisplayMode::GetDisplayColorMode( pszDisplayName ) ;
		unsigned int	nCurrentFreq =
			EDisplayMode::GetDisplayFrequency( pszDisplayName ) ;
		m_sizeFullScreen.cx = 0x7FFFFFFF ;
		m_sizeFullScreen.cy = 0x7FFFFFFF ;
		for ( ; ; )
		{
			if ( !::EnumDisplaySettings( pszDisplayName, nModeIndex, &DevMode ) )
			{
				break ;
			}
			nModeIndex ++ ;
			//
			// Compare display mode
			//
			if ( (DevMode.dmPelsWidth >= nWidth) &&
				(DevMode.dmPelsHeight >= nHeight) &&
				(DevMode.dmPelsWidth <= (unsigned int) m_sizeFullScreen.cx) &&
				(DevMode.dmPelsHeight <= (unsigned int) m_sizeFullScreen.cy) )
			{
				if ( nBitsPerPixel != 0 )
				{
					if ( nBitsPerPixel <= 8 )
					{
						if ( DevMode.dmBitsPerPel > 8 )
							continue ;
					}
					else //	( nBitsPerPixel > 8 )
					{
						if ( DevMode.dmBitsPerPel <= 8 )
							continue ;
					}
				}
				if ( (DevMode.dmPelsWidth == (unsigned int) m_sizeFullScreen.cx) &&
					(DevMode.dmPelsHeight == (unsigned int) m_sizeFullScreen.cy) )
				{
					if ( fFoundMode
						&& (m_sizeFullScreen.cx == (int) nWidth)
						&& (m_sizeFullScreen.cy == (int) nHeight)
						&& ((m_nBitsPerPixel == nBitsPerPixel) ||
							((nBitsPerPixel == 0)
								&& (m_nBitsPerPixel == nColorMode))) )
					{
						continue ;
					}
					if ( (m_nFrequency == nFrequency) ||
						(/*(nFrequency == 0) && */(m_nFrequency == nCurrentFreq)) )
					{
						continue ;
					}
				}
				//
				fFoundMode = true ;
				m_sizeFullScreen.cx = DevMode.dmPelsWidth ;
				m_sizeFullScreen.cy = DevMode.dmPelsHeight ;
				m_nBitsPerPixel = DevMode.dmBitsPerPel ;
				m_nFrequency = DevMode.dmDisplayFrequency ;
			}
		}
		//
		if ( !fFoundMode )
		{
			return	ESLErrorMsg
				( "適合するディスプレイモードが見つかりませんでした。" ) ;
		}
		//
		// Change display mode
		//
		if ( m_DisplayMode.TestDisplayMode
			( m_sizeFullScreen.cx, m_sizeFullScreen.cy,
						m_nBitsPerPixel, m_nFrequency, pszDisplayName ) )
		{
			return	ESLErrorMsg
				( "適合するディスプレイモードが見つかりませんでした。" ) ;
		}
		if ( (m_sizeFullScreen.cx != (int) nWidth)
			|| (m_sizeFullScreen.cy != (int) nHeight) )
		{
			ESLTrace( "警告：完全に適合していないディスプレイモードです。\n" ) ;
			m_flagUnmatchedSize = TRUE ;
		}
		else
		{
			m_flagUnmatchedSize = FALSE ;
		}
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
		if ( m_DisplayMode.ChangeDisplayMode( pszDisplayName ) == eslErrSuccess )
		{
			if ( !m_DisplayMode.GetMonitorInfo( hMonitor, &moninf ) )
			{
				m_ptMonitorUpperLeft.x = moninf.rcMonitor.left ;
				m_ptMonitorUpperLeft.y = moninf.rcMonitor.top ;
			}
			else
			{
				m_ptMonitorUpperLeft.x = 0 ;
				m_ptMonitorUpperLeft.y = 0 ;
			}
			return	eslErrSuccess ;
		}
	#endif
	}

	return	eslErrSuccess ;
}

// ウィンドウを作成し、ディスプレイモードを切り替える
//////////////////////////////////////////////////////////////////////////////
ESLError EGameWindow::CreateDisplayEx
	( const char * pszWindowName,
		CooperationLevel fCooperationLevel,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel,
		unsigned int nFrequency,
		const POINT * pWindowPos,
		const SIZE * pWindowSize, HWND hwndParent )
{
	//
	// Preparing the function
	//
	if ( m_flagCreated )
	{
		ESLTrace( "既にウィンドウは作成されています。\n" ) ;
		return	ESLErrorMsg( "既にウィンドウは作成されています。" ) ;
	}
	//
	m_fCooperationLevel = fCooperationLevel ;
	m_pointOffset.x = 0 ;
	m_pointOffset.y = 0 ;
	m_sizeDisplay.cx = nWidth ;
	m_sizeDisplay.cy = nHeight ;
	m_nBitsPerPixel = nBitsPerPixel ;
	m_nFrequency = 0 ;
	m_flagActivated = TRUE ;
	m_flagDeactivate = TRUE ;
	//
	m_uMsgSetOptFlag =
		::RegisterWindowMessage( "GLS_GAMEWINDOW_SET_OPT_FLAG" ) ;
	//
	// Get window rectangle
	//
	SIZE	sizeWindow ;
	SIZE	sizeFrame ;
	if ( !(m_fOptionFunctions & VariableWindowSize) )
	{
		sizeFrame.cx = ::GetSystemMetrics(SM_CXFIXEDFRAME) ;
		sizeFrame.cy = ::GetSystemMetrics(SM_CYFIXEDFRAME) ;
	}
	else
	{
		sizeFrame.cx = ::GetSystemMetrics(SM_CXSIZEFRAME) ;
		sizeFrame.cy = ::GetSystemMetrics(SM_CYSIZEFRAME) ;
	}
	sizeWindow.cx = nWidth + sizeFrame.cx * 2 ;
	sizeWindow.cy = nHeight + sizeFrame.cy * 2
							+ ::GetSystemMetrics(SM_CYCAPTION) ;
	POINT	posWindow ;
	if ( pWindowPos == NULL )
	{
		posWindow.x = (::GetSystemMetrics(SM_CXSCREEN) - sizeWindow.cx) / 2 ;
		posWindow.y = (::GetSystemMetrics(SM_CYSCREEN) - sizeWindow.cy) / 2 ;
	}
	else
	{
		posWindow = *pWindowPos ;
	}
	bool	fFitClientSize = true ;
	if ( (pWindowSize != NULL)
		&& (m_fOptionFunctions & (VariableWindowSize | NoAutoFitSize)) )
	{
		sizeWindow = *pWindowSize ;
		fFitClientSize = false ;
	}
	//
	// Create window
	//
//	m_fOptionFunctions = BlackBack ;
	//
	DWORD	dwStyle = GetModifiedWindowStyle( 0 ) & ~WS_VISIBLE ;
	DWORD	dwExStyle = GetModifiedWindowExStyle( 0 ) ;
	//
	EString		strClassName = ENTIS_GAME_WINDOW_CLASS ;
	WNDCLASS	wndcls ;
	HMODULE		hModule = ::GetModuleHandle( NULL ) ;
	bool		fClassOwner = true ;
	if ( ::GetClassInfo( hModule, strClassName, &wndcls ) )
	{
		fClassOwner = false ;
		for ( int i = 0; i < 0x100; i ++ )
		{
			strClassName = ENTIS_GAME_WINDOW_CLASS ;
			strClassName += EString(i) ;
			if ( !::GetClassInfo( hModule, strClassName, &wndcls ) )
			{
				fClassOwner = true ;
				break ;
			}
		}
	}
	RegisterClass( strClassName ) ;
	if ( fClassOwner )
	{
		m_strWndClass = strClassName ;
	}
	else
	{
		m_strWndClass.FreeString() ;
	}
	NormalizeWindowPos( posWindow, sizeWindow ) ;
	//
	if ( EWindow::Create
		( strClassName,
			pszWindowName, dwStyle, dwExStyle, posWindow.x, posWindow.y,
			sizeWindow.cx, sizeWindow.cy,
			hwndParent, NULL, ::GetModuleHandle(NULL) ) )
	{
		ESLTrace( "ゲームウィンドウの作成に失敗しました。\n" ) ;
		return	ESLErrorMsg( "ゲームウィンドウの作成に失敗しました。" ) ;
	}
	m_hIMC = ::ImmAssociateContext( m_hWnd, NULL ) ;
	//
	GetWindowRect( &m_rctNormalWndPos ) ;
	//
	if ( m_pivView3D != NULL )
	{
		m_pivView3D->AttachWindow( m_hWnd ) ;
	}
	//
	// Change display mode
	//
	if ( m_fCooperationLevel & flagFullScreen )
	{
		ESLError	err = ChangeDisplayMode
			( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
		if ( err != eslErrSuccess )
		{
//			DestroyWindow( ) ;
//			return	err ;
			m_fCooperationLevel &= ~(flagFullScreen | flagExclusive) ;
		}
	}
	if ( !(m_fCooperationLevel & flagFullScreen) )
	{
		m_sizeDisplay.cx = nWidth ;
		m_sizeDisplay.cy = nHeight ;
		m_sizeScreen = m_sizeDisplay ;
		m_sizeFullScreen = m_sizeDisplay ;
		m_nBitsPerPixel = EDisplayMode::GetDisplayColorMode( ) ;
		m_flagUnmatchedSize = FALSE ;
		//
		LONG	lClassStyle = ::GetClassLong( m_hWnd, GCL_STYLE ) ;
		DWORD	dwWndStyle = GetStyle( ) ;
		::SetClassLong
			( m_hWnd, GCL_STYLE,
				GetModifiedWindowClassStyle( lClassStyle ) ) ;
		SetStyle( GetModifiedWindowStyle( dwWndStyle ) & ~WS_VISIBLE ) ;
		//
		if ( !fFitClientSize )
		{
			UpdateClientDisplayPosition() ;
		}
	}
	//
	m_flagCreated = TRUE ;
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	if ( m_fCooperationLevel & flagFullScreen )
	{
		SetStyle( GetModifiedWindowStyle( GetStyle() ) & ~WS_VISIBLE ) ;
		MoveWindow
			( m_ptMonitorUpperLeft.x, m_ptMonitorUpperLeft.y,
				m_sizeFullScreen.cx, m_sizeFullScreen.cy, TRUE ) ;
		m_pointOffset.x = (m_sizeFullScreen.cx - m_sizeScreen.cx) / 2 ;
		m_pointOffset.y = (m_sizeFullScreen.cy - m_sizeScreen.cy) / 2 ;
	}
	#endif
	//
	if ( !(m_fCooperationLevel & flagFullScreen) && fFitClientSize )
	{
		FitWindowClientSize() ;
	}
	if ( !(m_fOptionFunctions & InvisibleWindow) )
	{
		ShowWindow( SW_SHOW ) ;
		::SetForegroundWindow( m_hWnd ) ;
	}
	//
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	if ( m_fCooperationLevel & flagExclusive )
	{
		::SetPriorityClass( ::GetCurrentProcess(), HIGH_PRIORITY_CLASS ) ;
	}
	#endif

	return	eslErrSuccess ;
}

ESLError EGameWindow::CreateDisplay
	( const char * pszWindowName,
		CooperationLevel fCooperationLevel,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency,
		const POINT * pWindowPos, HWND hwndParent )
{
	return	CreateDisplayEx
		( pszWindowName, fCooperationLevel, nWidth, nHeight,
			nBitsPerPixel, nFrequency, pWindowPos, NULL, hwndParent ) ;
}

// ウィンドウサイズを変更
//////////////////////////////////////////////////////////////////////////////
ESLError EGameWindow::ChangeDisplaySize
	( unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency )
{
	//
	// Preparing the function
	//
	if ( !m_flagCreated )
	{
		ESLTrace( "ゲームウィンドウは作成されていません。\n" ) ;
		return	ESLErrorMsg( "ゲームウィンドウは作成されていません。" ) ;
	}
	//
	// Change display mode
	//
	if ( m_flagActivated )
	{
		if ( m_fCooperationLevel & flagFullScreen )
		{
			ESLError	err =
				ChangeDisplayMode
					( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
			if ( err != eslErrSuccess )
				return	err ;
		}
		else
		{
			m_sizeDisplay.cx = nWidth ;
			m_sizeDisplay.cy = nHeight ;
			m_sizeScreen = m_sizeDisplay ;
			m_sizeFullScreen = m_sizeDisplay ;
			m_nBitsPerPixel = EDisplayMode::GetDisplayColorMode( ) ;
			m_flagUnmatchedSize = FALSE ;
		}
	}
	//
	// Set window placement
	//
	WINDOWPLACEMENT	wp ;
	GetWindowPlacement( &wp ) ;
	SIZE	sizeWindow ;
	sizeWindow.cx = nWidth + ::GetSystemMetrics(SM_CXBORDER) * 2 ;
	sizeWindow.cy = nHeight + ::GetSystemMetrics(SM_CYCAPTION)
							+ ::GetSystemMetrics(SM_CXBORDER) * 2 ;
	wp.rcNormalPosition.right = wp.rcNormalPosition.left + sizeWindow.cx ;
	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + sizeWindow.cy ;
	SetWindowPlacement( &wp ) ;
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	if ( m_flagActivated && (m_fCooperationLevel & flagFullScreen) )
	{
		MoveWindow( 0, 0, m_sizeFullScreen.cx, m_sizeFullScreen.cy, TRUE ) ;
	}
	#endif

	return	eslErrSuccess ;
}

// 協調レベルを変更する
//////////////////////////////////////////////////////////////////////////////
ESLError EGameWindow::ChangeCooperationLevel
			( CooperationLevel fCooperationLevel )
{
	if ( m_fCooperationLevel == fCooperationLevel )
	{
		return	eslErrSuccess ;
	}

	unsigned int	nChangedFlags = m_fCooperationLevel ^ fCooperationLevel ;
	m_fCooperationLevel = fCooperationLevel ;
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
		if ( nChangedFlags & flagExclusive )
		{
			if ( m_fCooperationLevel & flagExclusive )
			{
				SetWindowPos( HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE) ) ;
				::SetPriorityClass( ::GetCurrentProcess(), HIGH_PRIORITY_CLASS ) ;
			}
			else
			{
				SetWindowPos( HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE) ) ;
				::SetPriorityClass( ::GetCurrentProcess(), NORMAL_PRIORITY_CLASS ) ;
			}
		}
	#endif
	if ( m_flagActivated )
	{
		if ( nChangedFlags & flagFullScreen )
		{
			if ( m_fCooperationLevel & flagFullScreen )
			{
				WINDOWPLACEMENT	wp ;
				if ( GetWindowPlacement( &wp ) )
				{
					m_rctNormalWndPos = wp.rcNormalPosition ;
				}
				else
				{
					GetWindowRect( &m_rctNormalWndPos ) ;
				}
				ESLError	err =
					ChangeDisplayMode
						( m_sizeDisplay.cx, m_sizeDisplay.cy,
									m_nBitsPerPixel, m_nFrequency ) ;
				if ( err != eslErrSuccess )
				{
					return	err ;
				}
				#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
				if ( !m_fControlWindowByView3D )
				{
					WINDOWPLACEMENT	wp ;
					eslFillMemory( &wp, 0, sizeof(wp) ) ;
					wp.length = sizeof(wp) ;
					GetWindowPlacement( &wp ) ;
					//
					DWORD	dwStyle = GetModifiedWindowStyle( GetStyle() ) ;
					SetStyle( dwStyle ) ;
					//
					MoveWindow
						( m_ptMonitorUpperLeft.x, m_ptMonitorUpperLeft.y,
							m_sizeFullScreen.cx, m_sizeFullScreen.cy, TRUE ) ;
					//
					m_pointOffset.x = (m_sizeFullScreen.cx - m_sizeScreen.cx) / 2 ;
					m_pointOffset.y = (m_sizeFullScreen.cy - m_sizeScreen.cy) / 2 ;
				}
				#endif
			}
			else
			{
				m_flagUnmatchedSize = FALSE ;
				#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
				if ( m_fFullscreenByView3D && m_pivView3D )
				{
					using namespace E3DSDisplayPlugin ;
					if ( m_pivView3D->OnRestoreDisplayMode()
											== methodNoModeChanged )
					{
						m_DisplayMode.RestoreDisplayMode( ) ;
					}
					m_fFullscreenByView3D = false ;
					m_fControlWindowByView3D = false ;
				}
				else
				{
					m_DisplayMode.RestoreDisplayMode( ) ;
				}
				ShowWindow( SW_RESTORE ) ;
				//
				DWORD	dwStyle = GetModifiedWindowStyle( GetStyle() ) ;
				SetStyle( dwStyle ) ;
				//
				MoveWindow
					( m_rctNormalWndPos.left, m_rctNormalWndPos.top,
						m_rctNormalWndPos.right - m_rctNormalWndPos.left,
						m_rctNormalWndPos.bottom - m_rctNormalWndPos.top, TRUE ) ;
				SetWindowPos
					( NULL, 0, 0, 0, 0,
						(SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME) ) ;
				#endif
			}
		}
		if ( nChangedFlags & flagMouseCapturing )
		{
			if ( (m_fCooperationLevel & flagMouseCapturing)
									&& !m_fControlWindowByView3D )
			{
				SetCapture( ) ;
				RECT	rectClient ;
				GetClientRect( &rectClient ) ;
				POINT	pointOffset = { 0, 0 } ;
				EWindow::ClientToScreen( &pointOffset ) ;
				rectClient.left += pointOffset.x ;
				rectClient.top += pointOffset.y ;
				rectClient.right += pointOffset.x - 1 ;
				rectClient.bottom += pointOffset.y - 1 ;
				::ClipCursor( &rectClient ) ;
			}
			else
			{
				ReleaseCapture( ) ;
				::ClipCursor( NULL ) ;
			}
		}
	}
	if ( !(m_fCooperationLevel & flagFullScreen) )
	{
		DWORD	dwLastStyle = GetStyle() ;
		DWORD	dwStyle = GetModifiedWindowStyle( dwLastStyle ) ;
		if ( dwLastStyle != dwStyle )
		{
			SetStyle( dwStyle ) ;
			SetWindowPos
				( NULL, 0, 0, 0, 0,
					(SWP_NOZORDER | SWP_NOMOVE
						| SWP_NOSIZE | SWP_DRAWFRAME) ) ;
		}
		if ( !(m_fCooperationLevel & (VariableWindowSize | NoAutoFitSize)) )
		{
			FitWindowClientSize() ;
		}
		else
		{
			UpdateClientDisplayPosition() ;
		}
	}

	return	eslErrSuccess ;
}

// ウィンドウを閉じて、ディスプレイモードを元に戻す
//////////////////////////////////////////////////////////////////////////////
ESLError EGameWindow::CloseDisplay( void )
{
	//
	// Preparing the function
	//
	if ( !m_flagCreated )
	{
		ESLTrace( "ゲームウィンドウは作成されていません。\n" ) ;
		return	ESLErrorMsg( "ゲームウィンドウは作成されていません。" ) ;
	}
	//
	// Close window
	//
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
		if ( m_fCooperationLevel & flagExclusive )
		{
			::SetPriorityClass( ::GetCurrentProcess(), NORMAL_PRIORITY_CLASS ) ;
		}
		if ( m_fFullscreenByView3D && m_pivView3D )
		{
			using namespace E3DSDisplayPlugin ;
			if ( m_pivView3D->OnRestoreDisplayMode() == methodNoModeChanged )
			{
				m_DisplayMode.RestoreDisplayMode( ) ;
			}
			m_pivView3D->DetachWindow() ;
			m_fFullscreenByView3D = false ;
			m_fControlWindowByView3D = false ;
		}
		else if ( m_flagActivated && (m_fCooperationLevel & flagFullScreen) )
		{
			m_DisplayMode.RestoreDisplayMode( ) ;
		}
	#endif
	if ( m_flagActivated && (m_fCooperationLevel & flagMouseCapturing) )
	{
		ReleaseCapture( ) ;
		::ClipCursor( NULL ) ;
	}
	//
	DestroyWindow( ) ;
	m_flagCreated = false ;
	m_hIMC = NULL ;

	return	eslErrSuccess ;
}

// 強調レベルを取得
//////////////////////////////////////////////////////////////////////////////
EGameWindow::CooperationLevel EGameWindow::GetCooperationLevel( void ) const
{
	return	(CooperationLevel) m_fCooperationLevel ;
}

// オプショナル機能フラグを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned int EGameWindow::GetOptionalFuncFlag( void ) const
{
	return	m_fOptionFunctions ;
}

// オプショナル機能フラグを設定する
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::SetOptionalFuncFlag( unsigned int flagsOptionalFunc )
{
	if ( m_fOptionFunctions != flagsOptionalFunc )
	{
		if ( !m_flagCreated || (m_hWnd == NULL) || !::IsWindow(m_hWnd) )
		{
			m_fOptionFunctions = flagsOptionalFunc ;
			return ;
		}
		DWORD	dwThreadID = ::GetWindowThreadProcessId( m_hWnd, NULL ) ;
		if ( ::GetCurrentThreadId() == dwThreadID )
		{
			DWORD	dwChanged = m_fOptionFunctions ^ flagsOptionalFunc ;
			m_fOptionFunctions = flagsOptionalFunc & StatusFlagMask ;
			LONG	lClassStyle = ::GetClassLong( m_hWnd, GCL_STYLE ) ;
			DWORD	dwWndStyle = GetStyle( ) ;
			//
			if ( !(m_fOptionFunctions & EnableIME) )
			{
				::ImmAssociateContext( m_hWnd, NULL ) ;
			}
			else
			{
				::ImmAssociateContext( m_hWnd, m_hIMC ) ;
				if ( flagsOptionalFunc & OpenIME )
				{
					::ImmSetOpenStatus( m_hIMC, TRUE ) ;
				}
			}
			::SetClassLong
				( m_hWnd, GCL_STYLE,
					GetModifiedWindowClassStyle( lClassStyle ) ) ;
			SetStyle( GetModifiedWindowStyle( dwWndStyle ) ) ;
			//
			::RedrawWindow
				( m_hWnd, NULL, NULL,
					RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW ) ;
			SetWindowPos
				( NULL, 0, 0, 0, 0,
					(SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME) ) ;
			//
			if ( m_fOptionFunctions & BlackBack )
			{
				::SetClassLong
					( m_hWnd, GCL_HBRBACKGROUND,
						(LONG)::GetStockObject( BLACK_BRUSH ) ) ;
			}
			else
			{
				::SetClassLong( m_hWnd, GCL_HBRBACKGROUND, NULL ) ;
			}
			if ( ((dwChanged & VariableWindowSize)
						|| !(m_fOptionFunctions & VariableWindowSize))
					&& !(m_fCooperationLevel & flagFullScreen) )
			{
				FitWindowClientSize() ;
				UpdateClientDisplayPosition( false ) ;
			}
			else
			{
				UpdateClientDisplayPosition() ;
			}
			if ( flagsOptionalFunc & DoMinimize )
			{
				ShowWindow( SW_MINIMIZE ) ;
			}
			else if ( flagsOptionalFunc & DoMaximize )
			{
				if ( (m_fOptionFunctions & VariableWindowSize)
					&& !(m_fCooperationLevel & flagFullScreen) )
				{
					ShowWindow( SW_SHOWMAXIMIZED ) ;
				}
			}
		}
		else
		{
			SendMessage( m_uMsgSetOptFlag, flagsOptionalFunc, 0 ) ;
		}
	}
}

// ウィンドウスタイルを取得
//////////////////////////////////////////////////////////////////////////////
DWORD EGameWindow::GetModifiedWindowStyle( DWORD dwStyle ) const
{
	dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
	//
	if ( m_fOptionFunctions & ChildWindow )
	{
		dwStyle |= WS_CHILD ;
	}
	else
	{
		dwStyle &= ~WS_CHILD ;
	}
	if ( m_fOptionFunctions & PopupWindow )
	{
		dwStyle |= WS_POPUP ;
		dwStyle &= ~WS_CAPTION ;
	}
	else
	{
		dwStyle |= WS_CAPTION ;
		dwStyle &= ~WS_POPUP ;
	}
	if ( m_fOptionFunctions & InvisibleWindow )
	{
		dwStyle &= ~WS_VISIBLE ;
	}
	else
	{
		dwStyle |= WS_VISIBLE ;
	}
	if ( m_fOptionFunctions
			& (AllowClose | AllowMinimize | AllowMaximize) )
	{
		dwStyle |= WS_SYSMENU ;
	}
	else
	{
		dwStyle &= ~WS_SYSMENU ;
	}
	if ( m_fOptionFunctions & AllowMinimize )
	{
		dwStyle |= WS_SYSMENU | WS_MINIMIZEBOX ;
	}
	else
	{
		dwStyle &= ~(WS_MINIMIZEBOX) ;
	}
	dwStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX) ;
	if ( !(m_fCooperationLevel & flagFullScreen) )
	{
		if ( m_fOptionFunctions & VariableWindowSize )
		{
			dwStyle |= WS_THICKFRAME ;
			if ( m_fOptionFunctions & AllowMaximize )
			{
				dwStyle |= WS_SYSMENU | WS_MAXIMIZEBOX ;
			}
		}
	}
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	if ( m_fCooperationLevel & flagFullScreen )
	{
		dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX) | WS_POPUP ;
	}
	#endif
	return	dwStyle ;
}

// 拡張ウィンドウスタイルを取得
//////////////////////////////////////////////////////////////////////////////
DWORD EGameWindow::GetModifiedWindowExStyle( DWORD dwExStyle ) const
{
	#if	!defined(_DEBUG) || ENABLE_GLS_FULLSCREEN
	if ( m_fCooperationLevel & flagExclusive )
	{
		dwExStyle |= WS_EX_TOPMOST ;
	}
	else
	{
		dwExStyle &= ~WS_EX_TOPMOST ;
	}
	#endif
	return	dwExStyle ;
}

// クラススタイルを取得
//////////////////////////////////////////////////////////////////////////////
LONG EGameWindow::GetModifiedWindowClassStyle( LONG lClassStyle ) const
{
	lClassStyle &= ~(CS_IME | CS_DBLCLKS | CS_NOCLOSE) ;
	if ( m_fOptionFunctions & UseDblClick )
	{
		lClassStyle |= CS_DBLCLKS ;
	}
	if ( !(m_fOptionFunctions & AllowClose) )
	{
		lClassStyle |= CS_NOCLOSE ;
	}
	return	lClassStyle ;
}

// 画面モード変更フラグを取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EGameWindow::GetChangeDisplayModeFlag( void ) const
{
	return	!m_flagNoChangeMode ;
}

// 画面モード変更フラグを設定
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::SetChangeDisplayModeFlag( unsigned int nWithChangeMode )
{
	m_flagNoChangeMode = !nWithChangeMode ;
}

// 表示用立体視インターフェースの関連付け
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::Attach3DViewDisplay
		( E3DSDisplayPlugin::I3DImageView * pivView3D )
{
	if ( m_pivView3D != pivView3D )
	{
		if ( m_pivView3D != NULL )
		{
			m_pivView3D->DetachWindow() ;
		}
		//
		m_fEnableStereo3D = (pivView3D != NULL) ;
		m_pivView3D = pivView3D ;
		//
		if ( ::IsWindow( m_hWnd ) )
		{
			if ( m_pivView3D != NULL )
			{
				m_pivView3D->AttachWindow( m_hWnd ) ;
			}
			InvalidateRect( NULL, TRUE ) ;
		}
	}
}

// 座標変換
//////////////////////////////////////////////////////////////////////////////
BOOL EGameWindow::ClientToScreen( LPPOINT lpPoint ) const
{
	if ( m_flagNoChangeMode )
	{
		ESLAssert( m_flagCreated ) ;
		lpPoint->x =
			(long int) ((INT64) lpPoint->x
							* m_sizeScreen.cx
							/ m_sizeDisplay.cx) + m_pointOffset.x ;
		lpPoint->y =
			(long int) ((INT64) lpPoint->y
							* m_sizeScreen.cy
							/ m_sizeDisplay.cy) + m_pointOffset.y ;
	}
	else
	{
		lpPoint->x += m_pointOffset.x ;
		lpPoint->y += m_pointOffset.y ;
	}
	return	EWindow::ClientToScreen( lpPoint ) ;
}

BOOL EGameWindow::ScreenToClient( LPPOINT lpPoint ) const
{
	if ( !EWindow::ScreenToClient( lpPoint ) )
	{
		return	FALSE ;
	}
	if ( m_flagNoChangeMode )
	{
		ESLAssert( m_flagCreated ) ;
		lpPoint->x =
			(long int) ((INT64) lpPoint->x
							* m_sizeDisplay.cx
							/ m_sizeScreen.cx) - m_pointOffset.x ;
		lpPoint->y =
			(long int) ((INT64) lpPoint->y
							* m_sizeDisplay.cy
							/ m_sizeScreen.cy) - m_pointOffset.y ;
	}
	else
	{
		lpPoint->x -= m_pointOffset.x ;
		lpPoint->y -= m_pointOffset.y ;
	}
	return	TRUE ;
}

// ディスプレイオフセット取得
//////////////////////////////////////////////////////////////////////////////
const POINT & EGameWindow::GetOffsetPos( void ) const
{
	return	m_pointOffset ;
}

// ディスプレイオフセットを設定
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::SetOffsetPos( const POINT & ptOffset )
{
	m_pointOffset = ptOffset ;
}

// ディスプレイサイズを取得
//////////////////////////////////////////////////////////////////////////////
const SIZE & EGameWindow::GetDisplaySize( void ) const
{
	return	m_sizeDisplay ;
}

// ディスプレイモード（変更された画面サイズ）を取得
//////////////////////////////////////////////////////////////////////////////
const SIZE & EGameWindow::GetFullscreenSize( void ) const
{
	return	m_sizeFullScreen ;
}

// 表示サイズ（フルスクリーン時の伸縮サイズ）を取得
//////////////////////////////////////////////////////////////////////////////
const SIZE & EGameWindow::GetScreenSize( void ) const
{
	return	m_sizeScreen ;
}

// 表示サイズ（フルスクリーン時の伸縮サイズ）を設定
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::SetScreenSize( const SIZE & sizeScreen )
{
	m_sizeScreen = sizeScreen ;
}

// ディスプレイのビット深度を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EGameWindow::GetDisplayBitCount( void ) const
{
	return	m_nBitsPerPixel ;
}

// ディスプレイのリフレッシュ周波数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EGameWindow::GetDisplayFrequency( void ) const
{
	return	m_nFrequency ;
}

// ウィンドウがアクティブか？
//////////////////////////////////////////////////////////////////////////////
bool EGameWindow::IsWindowActive( void ) const
{
	return	(m_flagActivated != 0) ;
}

// 通常時ウィンドウ位置を取得
//////////////////////////////////////////////////////////////////////////////
bool EGameWindow::GetNormalWindowPos( RECT & rctNormalPos ) const
{
	if ( m_fCooperationLevel & flagFullScreen )
	{
		rctNormalPos = m_rctNormalWndPos ;
		return	true ;
	}
	else if ( ::IsWindow( m_hWnd ) )
	{
		WINDOWPLACEMENT	wp ;
		if ( GetWindowPlacement( &wp ) )
		{
			rctNormalPos = wp.rcNormalPosition ;
			return	true ;
		}
		else if ( GetWindowRect( &rctNormalPos ) )
		{
			return	true ;
		}
	}
	return	false ;
}

// ウィンドウ位置を正規化（画面内へ補正）
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::NormalizeWindowPos
	( POINT & posWindow, SIZE & sizeWindow )
{
	if ( !(m_fOptionFunctions & NoNormalizePos) )
	{
		m_DisplayMode.NormalizeWindowPos
			( posWindow, sizeWindow,
				((m_fOptionFunctions & VariableWindowSize) != 0) ) ;
	}
}

// ウィンドウのクライアントサイズをフィットさせる
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::FitWindowClientSize( void )
{
	if ( m_fOptionFunctions & NoAutoFitSize )
	{
		return ;
	}
	RECT	rcClient, rcWindow ;
	GetClientRect( &rcClient ) ;
	GetWindowRect( &rcWindow ) ;
	rcWindow.right +=
		m_sizeDisplay.cx - (rcClient.right - rcClient.left) ;
	rcWindow.bottom +=
		m_sizeDisplay.cy - (rcClient.bottom - rcClient.top) ;
	//
	POINT	posWindow = { rcWindow.left, rcWindow.top } ;
	SIZE	sizeWindow = 
				{ rcWindow.right - rcWindow.left,
					rcWindow.bottom - rcWindow.top } ;
	NormalizeWindowPos( posWindow, sizeWindow ) ;
	//
	SetWindowPos
		( NULL, posWindow.x, posWindow.y,
			sizeWindow.cx, sizeWindow.cy,
			(SWP_NOZORDER | SWP_DRAWFRAME) ) ;
	//
	m_pointOffset.x = 0 ;
	m_pointOffset.y = 0 ;
	m_sizeScreen.cx = m_sizeDisplay.cx ;
	m_sizeScreen.cy = m_sizeDisplay.cy ;
}

// ウィンドウのクライアントサイズを変更する
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::ChangeWindowClientSize( int nWidth, int nHeight )
{
	if ( m_fCooperationLevel & flagFullScreen )
	{
		if ( !(m_fOptionFunctions & PopupWindow) )
		{
			nHeight += GetSystemMetrics( SM_CYCAPTION ) ;
		}
		if ( m_fOptionFunctions & VariableWindowSize )
		{
			nWidth += GetSystemMetrics( SM_CXSIZEFRAME ) * 2
						+ GetSystemMetrics( SM_CXFIXEDFRAME ) * 2
						+ GetSystemMetrics( SM_CXBORDER ) * 2
						+ GetSystemMetrics( SM_CXPADDEDBORDER ) * 2 ;
			nHeight += GetSystemMetrics( SM_CYSIZEFRAME ) * 2
						+ GetSystemMetrics( SM_CYFIXEDFRAME ) * 2
						+ GetSystemMetrics( SM_CYBORDER ) * 2 ;
		}
		m_rctNormalWndPos.right = m_rctNormalWndPos.left + nWidth ;
		m_rctNormalWndPos.bottom = m_rctNormalWndPos.top + nHeight ;
		return ;
	}
	RECT	rectWindow, rectClient ;
	GetWindowRect( &rectWindow ) ;
	GetClientRect( &rectClient ) ;
	nWidth += rectWindow.right - rectWindow.left - rectClient.right ;
	nHeight += rectWindow.bottom - rectWindow.top - rectClient.bottom ;

	SetWindowPos
		( NULL, 0, 0, nWidth, nHeight,
			(SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME) ) ;
}

// ウィンドウクラス名を取得
//////////////////////////////////////////////////////////////////////////////
const char * EGameWindow::GetWindowClassName( void )
{
	return	ENTIS_GAME_WINDOW_CLASS ;
}

// ウィンドウクラスを登録
//////////////////////////////////////////////////////////////////////////////
void EGameWindow::RegisterClass( const char * pszClassName )
{
	WNDCLASS	wndclass ;
	if ( pszClassName == NULL )
	{
		pszClassName = ENTIS_GAME_WINDOW_CLASS ;
	}
	wndclass.style = 0 /*CS_IME | CS_NOCLOSE*/ ;
	wndclass.lpfnWndProc = NULL ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = 0 ;
	wndclass.hInstance = ::GetModuleHandle( NULL ) ;
	wndclass.hIcon = NULL ;
	wndclass.hCursor = NULL ;
	wndclass.hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH ) ;
	wndclass.lpszMenuName = NULL ;
	wndclass.lpszClassName = pszClassName ;
	if ( EWindow::RegisterClass( wndclass ) == 0 )
	{
		ESLTrace( "EGameWindow 用ウィンドウクラスの登録に失敗しました。\n" ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 画像描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

static const GUID GLS_CLSID_DirectDraw =
	{ 0xD7B70EE0,0x4340,0x11CF, { 0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35 } } ;
static const GUID GLS_CLSID_DirectDraw7 =
	{ 0x3c305196,0x50db,0x11d3, { 0x9c,0xfe,0x00,0xc0,0x4f,0xd9,0x30,0xc5 } } ;
static const GUID GLS_IID_IDirectDraw  =
	{ 0x6C14DB80,0xA733,0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 } } ;
static const GUID GLS_IID_IDirectDraw7 =
	{ 0x15e65ec0,0x3b9c,0x11d2, { 0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b } } ;
static const GUID GLS_IID_IDirectDrawSurface =
	{ 0x6C14DB81,0xA733,0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 } } ;
static const GUID GLS_IID_IDirectDrawSurface7 =
	{ 0x06675a80,0x3b9b,0x11d2, { 0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b } } ;
static const GUID GLS_CLSID_DirectDrawClipper =
	{ 0x593817A0,0x7DB3,0x11CF, { 0xA2,0xDE,0x00,0xAA,0x00,0xb9,0x33,0x56 } } ;
static const GUID GLS_IID_IDirectDrawClipper =
	{ 0x6C14DB85,0xA733,0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 } } ;
static const GUID GLS_IID_IDirect3D7 =
	{ 0xf5049e77,0x4861,0x11d2, { 0xa4,0x7,0x0,0xa0,0xc9,0x6,0x29,0xa8 } } ;

HMODULE	EGLDrawImage::m_hModuleD3D9 = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLDrawImage, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLDrawImage::EGLDrawImage( void )
{
	m_pguidDDDevice = NULL ;
	m_iddraw7 = NULL ;
	m_iddraw = NULL ;
	m_iddclip = NULL ;
	m_iddsufPrimary = NULL ;
	m_idd7sufPrimary = NULL ;
	m_iddsufSecondary = NULL ;
	m_idd7sufSecondary = NULL ;
	m_idd7sufStereoLeft = NULL ;
	//
	m_id3d9 = NULL ;
	m_id3d9Dev = NULL ;
	m_pd3dpp = NULL ;
	m_idds9DispBuf = NULL ;
	m_hWndDevD3D9 = NULL ;
	m_hDraw = NULL ;
	//
	m_dwVSyncLimitScanLine = -1 ;
	m_dwMaxScanLine = 0 ;
	//
	m_fCoInitialized = false ;
	m_fFullscreen = false ;
	m_fStereo3D = false ;
	//
	::InitializeCriticalSection( &m_csSync ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLDrawImage::~EGLDrawImage( void )
{
	Release( ) ;
	//
	if ( m_fCoInitialized )
	{
//		::CoUninitialize( ) ;
		m_fCoInitialized = false ;
	}
	::DeleteCriticalSection( &m_csSync ) ;
}

// 画像描画オブジェクトを初期化する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::Initialize( GUID * pGUID )
{
	//
	// DirectDraw 生成
	//
	ESLError	err ;
	err = CreateDirectDraw( pGUID ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ウィンドウモード初期化
	//
	return	CreateSurfaceWindowMode( ) ;
}

// DirectX オブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::CreateDirectDraw( GUID * pGUID )
{
	//
	// COM 初期化
	//
	if ( !m_fCoInitialized )
	{
		::CoInitialize( NULL ) ;
		m_fCoInitialized = true ;
	}
	//
	// DirectDraw オブジェクト生成
	//
	if ( m_iddraw != NULL )
	{
		bool	fEqualGUID = false ;
		if ( pGUID == NULL )
		{
			if ( m_pguidDDDevice == NULL )
			{
				fEqualGUID = true ;
			}
		}
		else if ( m_pguidDDDevice != NULL )
		{
			fEqualGUID = (IsEqualGUID( *pGUID, *m_pguidDDDevice ) != 0) ;
		}
		if ( !fEqualGUID )
		{
			Release() ;
		}
	}
	if ( m_iddraw == NULL )
	{
		ESLAssert( m_iddraw7 == NULL ) ;
		m_iddraw7 = NULL ;
		m_iddraw = NULL ;
		//
		if ( SUCCEEDED( ::CoCreateInstance( GLS_CLSID_DirectDraw7, NULL,
				CLSCTX_ALL, GLS_IID_IDirectDraw7, (void**) &m_iddraw7 ) ) )
		{
			if ( SUCCEEDED( m_iddraw7->QueryInterface
					( GLS_IID_IDirectDraw, (void**) &m_iddraw ) ) )
			{
				m_iddraw7->Initialize( pGUID ) ;
			}
			else
			{
				ESLTrace( "Failed to QueryInterface DirectDraw.\n" ) ;
				m_iddraw7->Release() ;
				m_iddraw7 = NULL ;
				m_iddraw = NULL ;
				return	eslErrGeneral ;
			}
		}
		else
		{
			if ( SUCCEEDED( ::CoCreateInstance( GLS_CLSID_DirectDraw, NULL,
					CLSCTX_ALL, GLS_IID_IDirectDraw, (void**) &m_iddraw ) ) )
			{
				m_iddraw->Initialize( pGUID ) ;
			}
			else
			{
				ESLTrace( "Failed to create DirectDraw.\n" ) ;
				m_iddraw = NULL ;
				return	eslErrGeneral ;
			}
		}
		//
		if ( pGUID != NULL )
		{
			m_guidDDDevice = *pGUID ;
			m_pguidDDDevice = &m_guidDDDevice ;
		}
		else
		{
			m_pguidDDDevice = NULL ;
		}
	}
	return	eslErrSuccess ;
}

// ウィンドウモード初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::CreateSurfaceWindowMode( void )
{
	if ( m_iddraw == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 既存サーフェス削除
	//
	ReleaseSurface( ) ;
	//
	// 協調レベル設定
	//
	if ( m_iddraw7 != NULL )
	{
		if ( m_iddraw7->SetCooperativeLevel( NULL, DDSCL_NORMAL ) != DD_OK )
		{
			ESLTrace( "Failed to IDirectDraw7::SetCooperativeLevel.\n" ) ;
		}
	}
	else
	{
		if ( m_iddraw->SetCooperativeLevel( NULL, DDSCL_NORMAL ) != DD_OK )
		{
			ESLTrace( "Failed to IDirectDraw::SetCooperativeLevel.\n" ) ;
		}
	}
	//
	// クリッパ生成
	//
	ESLAssert( m_iddclip == NULL ) ;
	if ( SUCCEEDED( ::CoCreateInstance
		( GLS_CLSID_DirectDrawClipper, NULL,
			CLSCTX_ALL, GLS_IID_IDirectDrawClipper, (void**) &m_iddclip ) ) )
	{
		if ( m_iddclip->Initialize( m_iddraw, 0 ) != DD_OK )
		{
			ESLTrace( "Failed to initialize DirectDrawClipper.\n" ) ;
			m_iddclip = NULL ;
			return	eslErrGeneral ;
		}
		//
		// プライマリサーフェース生成
		//
		ESLAssert( m_iddsufPrimary == NULL ) ;
		DDSURFACEDESC	ddsd ;
		memset( &ddsd, 0, sizeof(ddsd) ) ;
		ddsd.dwSize = sizeof(ddsd) ;
		ddsd.dwFlags = DDSD_CAPS ;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE ;
		if ( m_iddraw->CreateSurface
				( &ddsd, &m_iddsufPrimary, NULL ) != DD_OK )
		{
			ESLTrace( "Failed to create primary surface.\n" ) ;
			m_iddsufPrimary = NULL ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		m_iddclip = NULL ;
		return	eslErrGeneral ;
	}
	NotifyOnCreateDirectDraw( ) ;
	return	eslErrSuccess ;
}

// フルスクリーンモード初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::CreateSurfaceFullscreenMode
	( HWND hWnd, int nWidth, int nHeight,
		int nBitsPerPixel, int nFrequency, bool fStereo3D )
{
	if ( m_iddraw == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 既存サーフェス削除
	//
	ReleaseSurface( ) ;
	//
	// 協調レベル設定
	//
	if ( m_iddraw7 != NULL )
	{
		if ( FAILED( m_iddraw7->SetCooperativeLevel
				( hWnd, DDSCL_ALLOWREBOOT
							| DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN ) ) )
		{
			ESLTrace( "Failed to SetCooperativeLevel to fullscreen mode.\n" ) ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		if ( FAILED( m_iddraw->SetCooperativeLevel
				( hWnd, DDSCL_ALLOWREBOOT
							| DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN ) ) )
		{
			ESLTrace( "Failed to SetCooperativeLevel to fullscreen mode.\n" ) ;
			return	eslErrGeneral ;
		}
	}
	//
	// ディスプレイモード設定
	//
	if ( nBitsPerPixel == 0 )
	{
		nBitsPerPixel = 32 ;
	}
	if ( m_iddraw7 != NULL )
	{
		if ( fStereo3D )
		{
			DDSURFACEDESC2	ddsd ;
			if ( TestStereo3DGraphic
				( ddsd, nWidth, nHeight, nBitsPerPixel, nFrequency ) )
			{
				if ( nFrequency == 0 )
				{
					nFrequency = ddsd.dwRefreshRate ;
				}
			}
			else if ( TestStereo3DGraphic
				( ddsd, nWidth, nHeight, nBitsPerPixel, 0 ) )
			{
				nFrequency = ddsd.dwRefreshRate ;
			}
			else if ( TestStereo3DGraphic
				( ddsd, nWidth, nHeight, 0, 0 ) )
			{
				nBitsPerPixel = ddsd.ddpfPixelFormat.dwRGBBitCount ;
				nFrequency = ddsd.dwRefreshRate ;
			}
		}
		if ( FAILED( m_iddraw7->SetDisplayMode
			( nWidth, nHeight, nBitsPerPixel, nFrequency, 0 ) ) )
		{
			ESLTrace( "Failed to SetDisplayMode %d, %d, %d, %d, 0.\n",
							nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		if ( FAILED( m_iddraw->SetDisplayMode
			( nWidth, nHeight, nBitsPerPixel ) ) )
		{
			ESLTrace( "Failed to SetDisplayMode %d, %d, %d.\n",
								nWidth, nHeight, nBitsPerPixel ) ;
			return	eslErrGeneral ;
		}
	}
	//
	// DirectDraw7 でステレオ表示用 Quad Buffer 生成
	//////////////////////////////////////////////////////////////////////////
	ESLAssert( m_iddsufPrimary == NULL ) ;
	if ( fStereo3D && (m_iddraw7 != NULL) )
	do
	{
		//
		// ステレオ表示用プライマリサーフェースを生成
		//
		DDSURFACEDESC2	ddsd2 ;
		memset( &ddsd2, 0, sizeof(ddsd2) ) ;
		ddsd2.dwSize = sizeof(ddsd2) ;
		ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
								DDSCAPS_VIDEOMEMORY |
								DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE ;
		ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_STEREOSURFACELEFT ;
		ddsd2.dwBackBufferCount = 1 ;
		//
		HRESULT	hr ;
		if ( (hr = m_iddraw7->CreateSurface
				( &ddsd2, &m_idd7sufPrimary, NULL )) != DD_OK )
		{
			#if	defined(_DEBUG)
			ESLTrace( "Failed to create stereo 3D primary surface.(%08x)\n", hr ) ;
			if ( hr == DDERR_OUTOFVIDEOMEMORY )
			{
				ESLTrace( "out of video memory.\n" ) ;
			}
			else if ( hr == DDERR_NOSTEREOHARDWARE )
			{
				ESLTrace( "no stereo hardware.\n" ) ;
			}
			else if ( hr == DDERR_UNSUPPORTED )
			{
				ESLTrace( "unsupported.\n" ) ;
			}
			#endif
			m_idd7sufPrimary = NULL ;
			break ;
		}
		//
		// バックバッファ取得
		//
		DDSCAPS2	ddcaps2 ;
		memset( &ddcaps2, 0, sizeof(ddcaps2) ) ;
		ddcaps2.dwCaps = DDSCAPS_BACKBUFFER ;
		//
		if ( m_idd7sufPrimary->GetAttachedSurface
				( &ddcaps2, &m_idd7sufSecondary ) != DD_OK )
		{
			ESLTrace( "Failed to create stereo 3D backbuffer surface.\n" ) ;
			m_idd7sufPrimary->Release() ;
			m_idd7sufPrimary = NULL ;
			m_idd7sufSecondary = NULL ;
			break ;
		}
		m_idd7sufSecondary->AddRef() ;
		//
		// ステレオ左目用バッファ取得
		//
		memset( &ddcaps2, 0, sizeof(ddcaps2) ) ;
		ddcaps2.dwCaps2 = DDSCAPS2_STEREOSURFACELEFT ;
		//
		if ( m_idd7sufSecondary->GetAttachedSurface
				( &ddcaps2, &m_idd7sufStereoLeft ) != DD_OK )
		{
			ESLTrace( "Failed to create left-side buffer surface.\n" ) ;
			m_idd7sufSecondary->Release() ;
			m_idd7sufSecondary = NULL ;
			m_idd7sufPrimary->Release() ;
			m_idd7sufPrimary = NULL ;
			m_idd7sufStereoLeft = NULL ;
			break ;
		}
		m_idd7sufStereoLeft->AddRef() ;
		//
		m_fFullscreen = true ;
		m_fStereo3D = true ;
		//
		NotifyOnCreateDirectDraw( ) ;
		return	eslErrSuccess ;
	}
	while ( false ) ;
	//
	// DirectDraw7 でのプライマリサーフェスの生成
	//////////////////////////////////////////////////////////////////////////
	if ( m_iddraw7 != NULL )
	do
	{
		//
		// プライマリサーフェースを生成
		//
		DDSURFACEDESC2	ddsd2 ;
		memset( &ddsd2, 0, sizeof(ddsd2) ) ;
		ddsd2.dwSize = sizeof(ddsd2) ;
		ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
								DDSCAPS_VIDEOMEMORY |
								DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE ;
		ddsd2.dwBackBufferCount = 1 ;
		//
		HRESULT	hr ;
		if ( (hr = m_iddraw7->CreateSurface
				( &ddsd2, &m_idd7sufPrimary, NULL )) != DD_OK )
		{
			ESLTrace( "Failed to create DirectDraw7 primary surface.(%08x)\n", hr ) ;
			m_idd7sufPrimary = NULL ;
			break ;
		}
		//
		// バックバッファ取得
		//
		DDSCAPS2	ddcaps2 ;
		memset( &ddcaps2, 0, sizeof(ddcaps2) ) ;
		ddcaps2.dwCaps = DDSCAPS_BACKBUFFER ;
		//
		if ( m_idd7sufPrimary->GetAttachedSurface
				( &ddcaps2, &m_idd7sufSecondary ) != DD_OK )
		{
			ESLTrace( "Failed to create DirectDraw7 backbuffer surface.\n" ) ;
			m_idd7sufPrimary->Release() ;
			m_idd7sufPrimary = NULL ;
			m_idd7sufSecondary = NULL ;
			break ;
		}
		m_idd7sufSecondary->AddRef() ;
		//
		m_idd7sufPrimary->QueryInterface
			( GLS_IID_IDirectDrawSurface, (void**) &m_iddsufPrimary ) ;
		m_idd7sufSecondary->QueryInterface
			( GLS_IID_IDirectDrawSurface, (void**) &m_iddsufSecondary ) ;
		//
		m_fFullscreen = true ;
		m_fStereo3D = false ;
		//
		NotifyOnCreateDirectDraw( ) ;
		return	eslErrSuccess ;
	}
	while ( false ) ;
	//
	// DirectDraw1 でのプライマリサーフェスの生成
	//////////////////////////////////////////////////////////////////////////
	DDSURFACEDESC	ddsd ;
	memset( &ddsd, 0, sizeof(ddsd) ) ;
	ddsd.dwSize = sizeof(ddsd) ;
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
							DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE ;
	ddsd.dwBackBufferCount = 1 ;
	//
	if ( m_iddraw->CreateSurface
			( &ddsd, &m_iddsufPrimary, NULL ) != DD_OK )
	{
		ESLTrace( "Failed to create primary surface.\n" ) ;
		m_iddsufPrimary = NULL ;
		return	eslErrGeneral ;
	}
	//
	// バックバッファ取得
	//
	DDSCAPS	ddcaps ;
	memset( &ddcaps, 0, sizeof(ddcaps) ) ;
	ddcaps.dwCaps = DDSCAPS_BACKBUFFER ;
	//
	if ( m_iddsufPrimary->GetAttachedSurface
			( &ddcaps, &m_iddsufSecondary ) != DD_OK )
	{
		ESLTrace( "Failed to create backbuffer surface.\n" ) ;
		m_iddsufSecondary = NULL ;
		return	eslErrGeneral ;
	}
	m_iddsufSecondary->AddRef() ;
	//
	m_fFullscreen = true ;
	m_fStereo3D = false ;
	//
	NotifyOnCreateDirectDraw( ) ;
	return	eslErrSuccess ;
}

// Direct3D9 初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::CreateDirect3D9Device
	( HWND hWnd, int nWidth, int nHeight, int nAdapter, BOOL fWindowed )
{
	Release() ;
	//
	// Direct3D9 生成
	//
	if ( m_hModuleD3D9 == NULL )
	{
		m_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
		if ( m_hModuleD3D9 == NULL )
		{
			ESLTrace( "d3d9.dll をロードできませんでした。\n" ) ;
			return	eslErrGeneral ;
		}
	}
	typedef	IDirect3D9 * (WINAPI *API_Direct3DCreate9)( UINT SDKVersion ) ;
	API_Direct3DCreate9	apiDirect3DCreate9 =
		(API_Direct3DCreate9)
			::GetProcAddress( m_hModuleD3D9, "Direct3DCreate9" ) ;
	if ( apiDirect3DCreate9 == NULL )
	{
		ESLTrace( "Direct3DCreate が見つかりませんでした。\n" ) ;
		return	eslErrGeneral ;
	}
	m_id3d9 = apiDirect3DCreate9( D3D_SDK_VERSION ) ;
	if ( m_id3d9 == NULL )
	{
		ESLTrace( "Direct3D の生成に失敗しました。" ) ;
		return	eslErrGeneral ;
	}
	//
	// Direct3DDevice9 生成
	//
	if ( m_pd3dpp == NULL )
	{
		m_pd3dpp = new D3DPRESENT_PARAMETERS ;
	}
	memset( m_pd3dpp, 0, sizeof(D3DPRESENT_PARAMETERS) ) ;
	m_hWndDevD3D9 = hWnd ;
	m_pd3dpp->Windowed = fWindowed ;
	m_pd3dpp->EnableAutoDepthStencil = TRUE ;
	m_pd3dpp->AutoDepthStencilFormat = D3DFMT_D16 ;
	m_pd3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD ;
	m_pd3dpp->BackBufferWidth = nWidth ;
	m_pd3dpp->BackBufferHeight = nHeight ;
	m_pd3dpp->BackBufferFormat = D3DFMT_A8R8G8B8 ;
	m_pd3dpp->PresentationInterval = D3DPRESENT_INTERVAL_ONE ;
	m_pd3dpp->BackBufferCount = 1 ;
	//
	if ( m_id3d9->CreateDevice
			( nAdapter /*D3DADAPTER_DEFAULT*/,
				D3DDEVTYPE_HAL, hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING
					| D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED,
								m_pd3dpp, &m_id3d9Dev ) != D3D_OK )
	{
		ESLTrace( "Direct3DDevice9 の生成に失敗しました。\n" ) ;
		//
		Release() ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// Direct3D9 リセット
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::ResetDirect3D9Device( void )
{
	if ( (m_id3d9Dev != NULL) && (m_pd3dpp != NULL)
		&& (m_id3d9Dev->TestCooperativeLevel() != D3DERR_DEVICELOST) )
	{
		NotifyOnReleaseDirectDraw() ;
		ReleaseSurface() ;
		//
		RECT	rect ;
		if ( m_pd3dpp->Windowed
			&& ::GetClientRect( m_hWndDevD3D9, &rect )
			&& (rect.right - rect.left != 0)
			&& (rect.bottom - rect.top != 0) )
		{
			m_pd3dpp->BackBufferWidth = rect.right - rect.left ;
			m_pd3dpp->BackBufferHeight = rect.bottom - rect.top ;
		}
		HRESULT	hr = m_id3d9Dev->Reset( m_pd3dpp ) ;
		if ( hr != D3D_OK )
		{
			ESLTrace( "IDirect3DDevice9::Reset %d x %d は"
									"失敗しました。(%08X)\n",
					m_pd3dpp->BackBufferWidth,
					m_pd3dpp->BackBufferHeight, hr ) ;
			return	eslErrGeneral ;
		}
		//
		NotifyOnCreateDirectDraw( ) ;
		//
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ウィンドウサイズ変更
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::OnResizeWindow( void )
{
	return	ResetDirect3D9Device() ;
}

// 画像描画オブジェクトを終了する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::Release( void )
{
	NotifyOnReleaseDirectDraw() ;
	//
	ReleaseSurface( ) ;
	//
	if ( m_iddraw != NULL )
	{
		m_iddraw->Release() ;
		m_iddraw = NULL ;
	}
	if ( m_iddraw7 != NULL )
	{
		m_iddraw7->Release() ;
		m_iddraw7 = NULL ;
	}
	if ( m_idds9DispBuf != NULL )
	{
		m_idds9DispBuf->Release() ;
		m_idds9DispBuf = NULL ;
	}
	if ( m_id3d9Dev != NULL )
	{
		m_id3d9Dev->Release() ;
		m_id3d9Dev = NULL ;
	}
	if ( m_id3d9 != NULL )
	{
		m_id3d9->Release() ;
		m_id3d9 = NULL ;
	}
	if ( m_pd3dpp != NULL )
	{
		delete	m_pd3dpp ;
		m_pd3dpp = NULL ;
	}
	if ( m_hDraw != NULL )
	{
		m_hDraw->Release() ;
		m_hDraw = NULL ;
	}
	//
	m_dwVSyncLimitScanLine = -1 ;
	m_dwMaxScanLine = 0 ;
	//
	return	eslErrSuccess ;
}

// サーフェース開放（ディスプレイモード復帰）
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::ReleaseSurface( void )
{
	if ( m_iddsufSecondary != NULL )
	{
		m_iddsufSecondary->Release( ) ;
		m_iddsufSecondary = NULL ;
	}
	if ( m_iddsufPrimary != NULL )
	{
		m_iddsufPrimary->Release( ) ;
		m_iddsufPrimary = NULL ;
	}
	if ( m_idd7sufStereoLeft != NULL )
	{
		m_idd7sufStereoLeft->Release( ) ;
		m_idd7sufStereoLeft = NULL ;
	}
	if ( m_idd7sufSecondary != NULL )
	{
		m_idd7sufSecondary->Release( ) ;
		m_idd7sufSecondary = NULL ;
	}
	if ( m_idd7sufPrimary != NULL )
	{
		m_idd7sufPrimary->Release( ) ;
		m_idd7sufPrimary = NULL ;
	}
	if ( m_idds9DispBuf != NULL )
	{
		m_idds9DispBuf->Release() ;
		m_idds9DispBuf = NULL ;
	}
	if ( m_iddclip != NULL )
	{
		m_iddclip->Release( ) ;
		m_iddclip = NULL ;
	}
	if ( m_fFullscreen )
	{
		if ( m_iddraw7 != NULL )
		{
			m_iddraw7->RestoreDisplayMode( ) ;
		}
		else if ( m_iddraw != NULL )
		{
			m_iddraw->RestoreDisplayMode( ) ;
		}
	}
	m_fFullscreen = false ;
	m_fStereo3D = false ;
	return	eslErrSuccess ;
}

// ステレオ3Dモードテスト用構造体
//////////////////////////////////////////////////////////////////////////////
struct	TestStereo3DGraphic_EnumModesContext
{
	DDSURFACEDESC2 *	pddsdGetSupported ;
	bool				fStereo3D ;
	DWORD				dwWidth ;
	DWORD				dwHeight ;
	DWORD				dwBitsPerPixel ;
	DWORD				dwFrequency ;
} ;

static HRESULT WINAPI
	TestStereo3DGraphic_EnumModesCallback2
		( LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext ) ;

// ステレオ3D表示モードをテストする
//////////////////////////////////////////////////////////////////////////////
bool EGLDrawImage::TestStereo3DGraphic
	( DDSURFACEDESC2 & ddsdSupported,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) const
{
	if ( m_iddraw7 == NULL )
	{
		return	false ;
	}
	//
	TestStereo3DGraphic_EnumModesContext	ts3dg_context ;
	ts3dg_context.pddsdGetSupported = &ddsdSupported ;
	ts3dg_context.fStereo3D = false ;
	ts3dg_context.dwWidth = (DWORD) nWidth ;
	ts3dg_context.dwHeight = (DWORD) nHeight ;
	ts3dg_context.dwBitsPerPixel = (DWORD) nBitsPerPixel ;
	ts3dg_context.dwFrequency = (DWORD) nFrequency ;
	//
	ESLTrace( "EnumDisplayModes\n" ) ;
	//
	m_iddraw7->EnumDisplayModes
		( 0, NULL, &ts3dg_context,
			TestStereo3DGraphic_EnumModesCallback2 ) ;
	//
	if ( !ts3dg_context.fStereo3D )
	{
		m_iddraw7->EnumDisplayModes
			( DDEDM_REFRESHRATES , NULL, &ts3dg_context,
					TestStereo3DGraphic_EnumModesCallback2 ) ;
	}
	ESLTrace( "\n" ) ;
	//
	return	ts3dg_context.fStereo3D ;
}

static HRESULT WINAPI
	TestStereo3DGraphic_EnumModesCallback2
		( LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext )
{
	TestStereo3DGraphic_EnumModesContext *
		pts3dg = (TestStereo3DGraphic_EnumModesContext*) lpContext ;
	//
	ESLTrace( "enum mode %d x %d, %d bit, %d Hz, "
				"dwCaps = %08X, dwCaps2 = %08X\n",
					lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight,
					lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
					lpDDSurfaceDesc->dwRefreshRate,
					lpDDSurfaceDesc->ddsCaps.dwCaps,
					lpDDSurfaceDesc->ddsCaps.dwCaps2 ) ;
	//
	if ( lpDDSurfaceDesc->ddsCaps.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT )
	{
		if ( ((pts3dg->dwWidth == 0)
				|| (pts3dg->dwWidth == lpDDSurfaceDesc->dwWidth))
			&& ((pts3dg->dwHeight == 0)
				|| (pts3dg->dwHeight == lpDDSurfaceDesc->dwHeight))
			&& ((pts3dg->dwBitsPerPixel == 0)
				|| (pts3dg->dwBitsPerPixel
						== lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount))
			&& ((pts3dg->dwFrequency == 0)
				|| (pts3dg->dwFrequency == lpDDSurfaceDesc->dwRefreshRate)) )
		{
			*(pts3dg->pddsdGetSupported) = *lpDDSurfaceDesc ;
			pts3dg->fStereo3D = true ;
			return	DDENUMRET_CANCEL ;
		}
	}
	return	DDENUMRET_OK ;
}

// ステレオ3D表示モードがサポートされているか？
//////////////////////////////////////////////////////////////////////////////
bool EGLDrawImage::IsSupportedStereo3DGraphic( void ) const
{
	DDSURFACEDESC2	ddsd ;
	return	(m_iddraw7 != NULL)
				&& TestStereo3DGraphic( ddsd, 0, 0, 0, 0 ) ;
}

// DirectX 9 がインストールされているか？
//////////////////////////////////////////////////////////////////////////////
bool EGLDrawImage::IsInstalledDirectX9( void )
{
	if ( m_hModuleD3D9 == NULL )
	{
		m_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
		if ( m_hModuleD3D9 == NULL )
		{
			return	false ;
		}
	}
	return	true ;
}

// 画像描画のためにサーフェスを作成する
//////////////////////////////////////////////////////////////////////////////
IDirectDrawSurface *
	EGLDrawImage::CreateSurfaceOnVRAM( int nWidth, int nHeight )
{
	if ( m_iddraw != NULL )
	{
		DDSURFACEDESC			ddsd ;
		IDirectDrawSurface *	iddssuf ;
		memset( &ddsd, 0, sizeof(ddsd) ) ;
		ddsd.dwSize = sizeof(ddsd) ;
		ddsd.dwFlags = DDSD_CAPS ;
		//
		if ( !m_fFullscreen )
		{
			if ( (m_iddsufPrimary != NULL)
					&& m_iddsufPrimary->IsLost() )
			{
				m_iddsufPrimary->Release( ) ;
				m_iddsufPrimary = NULL ;
			}
			if ( m_iddsufPrimary == NULL )
			{
				ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE ;
				if ( m_iddraw->CreateSurface
						( &ddsd, &m_iddsufPrimary, NULL ) != DD_OK )
				{
					ESLTrace( "Failed to create primary surface.\n" ) ;
					m_iddsufPrimary = NULL ;
				}
			}
		}
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
		ddsd.dwHeight = nHeight ;
		ddsd.dwWidth = nWidth ;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY ;
		//
		if ( m_iddraw->CreateSurface( &ddsd, &iddssuf, NULL ) == DD_OK )
		{
			return	iddssuf ;
		}
	}
	return	NULL ;
}

// DirectDraw7 サーフェスを作成する
//////////////////////////////////////////////////////////////////////////////
IDirectDrawSurface7 * EGLDrawImage::CreateDD7Surface
	( int nWidth, int nHeight, DWORD dwCaps, DWORD dwCaps2 )
{
	if ( m_iddraw7 == NULL )
	{
		return	NULL ;
	}
	//
	DDSURFACEDESC2	ddsd ;
	memset( &ddsd, 0, sizeof(ddsd) ) ;
	ddsd.dwSize = sizeof(ddsd) ;
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH ;
	ddsd.dwHeight = nHeight ;
	ddsd.dwWidth = nWidth ;
	ddsd.ddsCaps.dwCaps = dwCaps ;
	ddsd.ddsCaps.dwCaps2 = dwCaps2 ;
	//
	if ( (dwCaps != 0) || (dwCaps2 != 0) )
	{
		ddsd.dwFlags |= DDSD_CAPS ;
	}
	//
	IDirectDrawSurface7 *	iddsuf = NULL ;
	if ( m_iddraw7->CreateSurface( &ddsd, &iddsuf, NULL ) == DD_OK )
	{
		return	iddsuf ;
	}
	return	NULL ;
}

// 画像描画のための初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::InitializeDrawImage( int nWidth, int nHeight )
{
	if ( (m_iddraw != NULL) && !m_fFullscreen )
	{
		//
		// 既存サーフェス削除
		//
		if ( m_iddsufSecondary != NULL )
		{
			m_iddsufSecondary->Release( ) ;
			m_iddsufSecondary = NULL ;
		}
		//
		// サーフェス作成
		//
		DDSURFACEDESC	ddsd ;
		::eslFillMemory( &ddsd, 0, sizeof(ddsd) ) ;
		ddsd.dwSize = sizeof(ddsd) ;
		ddsd.dwFlags = DDSD_CAPS ;
		if ( (m_iddsufPrimary != NULL) && m_iddsufPrimary->IsLost() )
		{
			m_iddsufPrimary->Release( ) ;
			m_iddsufPrimary = NULL ;
		}
		if ( m_iddsufPrimary == NULL )
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE ;
			if ( m_iddraw->CreateSurface
					( &ddsd, &m_iddsufPrimary, NULL ) != DD_OK )
			{
				ESLTrace( "Failed to create primary surface.\n" ) ;
				m_iddsufPrimary = NULL ;
			}
		}
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
		ddsd.dwHeight = nHeight ;
		ddsd.dwWidth = nWidth ;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY ;
		//
		if ( m_iddraw->CreateSurface
				( &ddsd, &m_iddsufSecondary, NULL ) != DD_OK )
		{
			ESLTrace( "Failed to CreateSurface on video memory.\n" ) ;
			m_iddsufSecondary = NULL ;
		}
	}
	return	eslErrSuccess ;
}

// 画像をクライアント領域に描画する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::DrawImageToDisplay
	( HWND hwndTarget, PCEGL_IMAGE_INFO pImageInf,
		int xPos, int yPos, const EGL_SIZE * pDstSize,
		const EGL_RECT * pSrcRect, DWORD fdwFlags,
		IDirectDrawSurface * pddsVRAM )
{
	ESLError	err = eslErrSuccess ;
	if ( pImageInf != NULL )
	do
	{
		if ( m_id3d9Dev != NULL )
		{
			return	DrawImageToDisplayD3D9
						( pImageInf, xPos, yPos, pDstSize, pSrcRect ) ;
		}
		if ( (pddsVRAM == NULL) && !m_fFullscreen )
		{
			pddsVRAM = m_iddsufSecondary ;
		}
		//
		// 描画領域とサイズを取得する
		//
		bool		fReverseVertical = false ;
		EGL_SIZE	sizeDst ;
		EGL_RECT	rectSrc ;
		int			xPosVRAM = 0 ;
		int			yPosVRAM = 0 ;
		EGL_SIZE	sizeVRAM = { (SDWORD) pImageInf->dwImageWidth, (SDWORD) pImageInf->dwImageHeight } ;
		if ( pSrcRect != NULL )
		{
			rectSrc = *pSrcRect ;
			if ( rectSrc.left < 0 )
				rectSrc.left = 0 ;
			if ( rectSrc.top < 0 )
				rectSrc.top = 0 ;
			if ( rectSrc.right >= (int) pImageInf->dwImageWidth )
				rectSrc.right = pImageInf->dwImageWidth - 1 ;
			if ( rectSrc.bottom >= (int) pImageInf->dwImageHeight )
				rectSrc.bottom = pImageInf->dwImageHeight - 1 ;
			if ( (rectSrc.left > rectSrc.right)
				|| (rectSrc.top > rectSrc.bottom) )
			{
				err = eslErrSuccess ;
				break ;
			}
		}
		else
		{
			rectSrc.left = 0 ;
			rectSrc.top = 0 ;
			rectSrc.right = pImageInf->dwImageWidth - 1 ;
			rectSrc.bottom = pImageInf->dwImageHeight - 1 ;
		}
		if ( pDstSize != NULL )
		{
			sizeDst = *pDstSize ;
			if ( sizeDst.h < 0 )
			{
				fReverseVertical = true ;
				sizeDst.h = - sizeDst.h ;
				yPos -= sizeDst.h - 1 ;
				yPosVRAM = sizeVRAM.h - 1 ;
				sizeVRAM.h = - sizeVRAM.h ;
			}
		}
		else
		{
			sizeDst.w = rectSrc.right - rectSrc.left + 1 ;
			sizeDst.h = rectSrc.bottom - rectSrc.top + 1 ;
		}
		//
		// 描画
		//
		bool	fStretchBlt =
				(sizeDst.w != (rectSrc.right - rectSrc.left + 1))
					| (sizeDst.h != (rectSrc.bottom - rectSrc.top + 1)) ;
		bool	fNormalDraw = true ;
		//
		if ( m_fFullscreen
			&& (m_iddsufSecondary != NULL)
			&& (fdwFlags & dfWaitVerticalBlank) )
		{
			//
			// フルスクリーンモードでフリップ処理を行う
			//
			if ( fStretchBlt && (pddsVRAM != NULL) )
			{
				ESLAssert( pddsVRAM != m_iddsufSecondary ) ;
				HDC	hdcSuf = NULL ;
				if ( pddsVRAM->GetDC( &hdcSuf ) == DD_OK )
				{
					::eglDrawToDC
						( hdcSuf, pImageInf,
							xPosVRAM, yPosVRAM, &sizeVRAM, NULL ) ;
					pddsVRAM->ReleaseDC( hdcSuf ) ;
					//
					// VRAM 間の伸縮描画
					//
					if ( !BltSurface
						( m_iddsufSecondary, pddsVRAM,
							xPos, yPos, &sizeDst, &rectSrc, false ) )
					{
						fNormalDraw = false ;
					}
				}
			}
			else
			{
				HDC	hdcSuf = NULL ;
				if ( m_iddsufSecondary->GetDC( &hdcSuf ) == DD_OK )
				{
					if ( fReverseVertical )
					{
						fStretchBlt = true ;
						fReverseVertical = false ;
						yPos += sizeDst.h - 1 ;
						sizeDst.h = - sizeDst.h ;
					}
					if ( fStretchBlt )
					{
						err = ::eglDrawToDC
							( hdcSuf, pImageInf,
								xPos, yPos, &sizeDst, &rectSrc ) ;
					}
					else
					{
						err = ::eglDrawToDC
							( hdcSuf, pImageInf,
								xPos, yPos, NULL, &rectSrc ) ;
					}
					m_iddsufSecondary->ReleaseDC( hdcSuf ) ;
					if ( !err )
					{
						fNormalDraw = false ;
					}
				}
			}
			if ( !fNormalDraw )
			{
				m_iddsufPrimary->Flip( NULL, DDFLIP_WAIT ) ;
			}
		}
		else if ( (pddsVRAM != NULL)
			&& (m_iddsufPrimary != NULL)
			&& (fStretchBlt || (fdwFlags & dfWaitVerticalBlank)) )
		do
		{
			//
			// ロストしたサーフェスを復元する
			//
			ESLAssert( m_iddsufPrimary != NULL ) ;
			if ( m_iddsufPrimary->IsLost() )
			{
				m_iddsufPrimary->Restore( ) ;
			}
			if ( pddsVRAM->IsLost() )
			{
				pddsVRAM->Restore( ) ;
			}
			//
			// クリッパを設定する
			//
			if ( m_iddclip != NULL )
			{
				m_iddclip->SetHWnd( 0, hwndTarget ) ;
				m_iddsufPrimary->SetClipper( m_iddclip ) ;
			}
			//
			// VRAM に転送する
			//
			HDC	hdcSuf = NULL ;
			if ( pddsVRAM->GetDC( &hdcSuf ) == DD_OK )
			{
				::eglDrawToDC
					( hdcSuf, pImageInf,
						xPosVRAM, yPosVRAM, &sizeVRAM, NULL ) ;
				pddsVRAM->ReleaseDC( hdcSuf ) ;
				//
				// VSYNC
				//
				if ( fdwFlags & dfWaitVerticalBlank )
				{
					WaitForVerticalBlank( ) ;
				}
				//
				// VRAM 間の伸縮描画
				//
				POINT	ptDraw ;
				ptDraw.x = xPos ;
				ptDraw.y = yPos ;
				if ( !m_fFullscreen )
				{
					::ClientToScreen( hwndTarget, &ptDraw ) ;
				}
				err = BltSurface
					( m_iddsufPrimary, pddsVRAM,
						ptDraw.x, ptDraw.y, &sizeDst, &rectSrc ) ;
				if ( !err )
				{
					fNormalDraw = false ;
				}
			}
		}
		while ( false ) ;
		//
		if ( fNormalDraw )
		{
			HDC	hdc = ::GetDC( hwndTarget ) ;
			if ( fReverseVertical )
			{
				fStretchBlt = true ;
				fReverseVertical = false ;
				yPos += sizeDst.h - 1 ;
				sizeDst.h = - sizeDst.h ;
			}
			if ( fStretchBlt )
			{
				err = ::eglDrawToDC
					( hdc, pImageInf, xPos, yPos, &sizeDst, &rectSrc ) ;
			}
			else
			{
				err = ::eglDrawToDC
					( hdc, pImageInf, xPos, yPos, NULL, &rectSrc ) ;
			}
			::ReleaseDC( hwndTarget, hdc ) ;
		}
	}
	while ( false ) ;
	return	err ;
}

// ステレオ画像を描画する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::DrawStereoImageToDisplay
	( HWND hwndTarget,
		PCEGL_IMAGE_INFO pImageLeft,
		PCEGL_IMAGE_INFO pImageRight,
		int xPos, int yPos,
		const EGL_SIZE * pDstSize,
		const EGL_RECT * pSrcRect, DWORD fdwFlags )
{
	//
	// 描画領域を決定する
	//
	EGL_SIZE	sizeDst ;
	EGL_RECT	rectSrc ;
	if ( pSrcRect != NULL )
	{
		rectSrc = *pSrcRect ;
		if ( rectSrc.left < 0 )
			rectSrc.left = 0 ;
		if ( rectSrc.top < 0 )
			rectSrc.top = 0 ;
		if ( rectSrc.right >= (int) pImageRight->dwImageWidth )
			rectSrc.right = pImageRight->dwImageWidth - 1 ;
		if ( rectSrc.bottom >= (int) pImageRight->dwImageHeight )
			rectSrc.bottom = pImageRight->dwImageHeight - 1 ;
		if ( (rectSrc.left > rectSrc.right)
			|| (rectSrc.top > rectSrc.bottom) )
		{
			return	eslErrSuccess ;
		}
	}
	else
	{
		rectSrc.left = 0 ;
		rectSrc.top = 0 ;
		rectSrc.right = pImageLeft->dwImageWidth - 1 ;
		rectSrc.bottom = pImageLeft->dwImageHeight - 1 ;
	}
	if ( pDstSize != NULL )
	{
		sizeDst = *pDstSize ;
	}
	else
	{
		sizeDst.w = rectSrc.right - rectSrc.left + 1 ;
		sizeDst.h = rectSrc.bottom - rectSrc.top + 1 ;
	}
	//
	// 描画方法判定
	//
	ESLError	err = eslErrGeneral ;
	if ( m_fStereo3D && m_fFullscreen
			&& m_iddraw7 && m_idd7sufPrimary
			&& m_idd7sufSecondary && m_idd7sufStereoLeft )
	{
		//
		// ステレオ3Dモード
		//
		HDC	hdcSuf = NULL ;
		if ( m_idd7sufSecondary->GetDC( &hdcSuf ) == DD_OK )
		{
			err = ::eglDrawToDC
				( hdcSuf, pImageRight, xPos, yPos, &sizeDst, &rectSrc ) ;
			m_idd7sufSecondary->ReleaseDC( hdcSuf ) ;
			//
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			return	eslErrGeneral ;
		}
		if ( m_idd7sufStereoLeft->GetDC( &hdcSuf ) == DD_OK )
		{
			err = ::eglDrawToDC
				( hdcSuf, pImageLeft, xPos, yPos, &sizeDst, &rectSrc ) ;
			m_idd7sufStereoLeft->ReleaseDC( hdcSuf ) ;
			//
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			return	eslErrGeneral ;
		}
		//
		if ( m_idd7sufPrimary->Flip
			( NULL, DDFLIP_WAIT | DDFLIP_STEREO ) != DD_OK )
		{
			ESLTrace( "Failed to flip at stereo graphic.\n" ) ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		//
		// 通常モード
		//
		HDC	hdc = ::GetDC( hwndTarget ) ;
		err = ::eglDrawToDC
			( hdc, pImageRight, xPos, yPos, &sizeDst, &rectSrc ) ;
		::ReleaseDC( hwndTarget, hdc ) ;
	}
	return	err ;
}

ESLError EGLDrawImage::DrawStereoImageToDisplay
	( HWND hwndTarget,
		struct IDirectDrawSurface7 * pddsLeft,
		struct IDirectDrawSurface7 * pddsRight,
		int xPos, int yPos, const EGL_SIZE * pDstSize,
		const EGL_RECT * pSrcRect, DWORD fdwFlags )
{
	//
	// 描画領域を決定する
	//
	EGL_SIZE	sizeDst ;
	EGL_RECT	rectSrc ;
	if ( pSrcRect != NULL )
	{
		rectSrc = *pSrcRect ;
		if ( rectSrc.left < 0 )
		{
			rectSrc.left = 0 ;
		}
		if ( rectSrc.top < 0 )
		{
			rectSrc.top = 0 ;
		}
		if ( (rectSrc.left > rectSrc.right)
			|| (rectSrc.top > rectSrc.bottom) )
		{
			return	eslErrSuccess ;
		}
	}
	else if ( pDstSize != NULL )
	{
		rectSrc.left = 0 ;
		rectSrc.top = 0 ;
		rectSrc.right = pDstSize->w - 1 ;
		rectSrc.bottom = pDstSize->h - 1 ;
	}
	else
	{
		return	eslErrGeneral ;
	}
	if ( pDstSize != NULL )
	{
		sizeDst = *pDstSize ;
	}
	else
	{
		sizeDst.w = rectSrc.right - rectSrc.left + 1 ;
		sizeDst.h = rectSrc.bottom - rectSrc.top + 1 ;
	}
	//
	// 描画方法判定
	//
	ESLError	err = eslErrGeneral ;
	if ( m_fStereo3D )
	{
		ESLAssert( m_fFullscreen ) ;
		ESLAssert( m_idd7sufPrimary != NULL ) ;
		ESLAssert( m_idd7sufSecondary != NULL ) ;
		ESLAssert( m_idd7sufStereoLeft != NULL ) ;
		//
		// ステレオ3Dモード
		//
		err = BltSurface
			( m_idd7sufSecondary, pddsRight,
					xPos, yPos, &sizeDst, &rectSrc, false ) ;
		if ( err )
		{
			return	err ;
		}
		err = BltSurface
			( m_idd7sufStereoLeft, pddsLeft,
					xPos, yPos, &sizeDst, &rectSrc, false ) ;
		if ( err )
		{
			return	err ;
		}
		if ( m_idd7sufPrimary->Flip
			( NULL, DDFLIP_WAIT | DDFLIP_STEREO ) != DD_OK )
		{
			ESLTrace( "Failed to flip at stereo graphic.\n" ) ;
			return	eslErrGeneral ;
		}
	}
	else if ( m_iddsufPrimary != NULL )
	{
		IDirectDrawSurface *	pddsTemp = NULL ;
		pddsRight->QueryInterface
			( GLS_IID_IDirectDrawSurface, (void**) &pddsTemp ) ;
		if ( pddsTemp != NULL )
		{
			if ( m_fFullscreen )
			{
				ESLAssert( m_iddsufSecondary != NULL ) ;
				//
				// フルスクリーンモード
				//
				err = BltSurface
					( m_iddsufSecondary, pddsTemp,
						xPos, yPos, &sizeDst, &rectSrc, false ) ;
				//
				if ( m_iddsufPrimary->Flip( NULL, DDFLIP_WAIT ) != DD_OK )
				{
					ESLTrace( "Failed to flip.\n" ) ;
					err = eslErrGeneral ;
				}
			}
			else
			{
				//
				// 通常モード
				//
				if ( m_iddraw != NULL )
				{
					WaitForVerticalBlank( ) ;
				}
				//
				POINT	ptDraw ;
				ptDraw.x = xPos ;
				ptDraw.y = yPos ;
				//
				::ClientToScreen( hwndTarget, &ptDraw ) ;
				//
				err = BltSurface
					( m_iddsufPrimary, pddsTemp,
						ptDraw.x, ptDraw.y, &sizeDst, &rectSrc ) ;
			}
			pddsTemp->Release() ;
		}
	}
	return	err ;
}

// Surface 間描画
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::BltSurface
	( IDirectDrawSurface7 * pddsDst,
		IDirectDrawSurface7 * pddsSrc,
		int xDst, int yDst,
		const EGL_SIZE * pDstSize,
		const EGL_RECT * pSrcRect, bool fAsync )
{
	DDBLTFX		bfx ;
	RECT		rctDstView ;
	RECT		rctSrcView ;
	EGL_SIZE	sizeDst ;
	//
	if ( pDstSize != NULL )
	{
		sizeDst = *pDstSize ;
	}
	else if ( pSrcRect != NULL )
	{
		sizeDst.w = pSrcRect->right - pSrcRect->left + 1 ;
		sizeDst.h = pSrcRect->bottom - pSrcRect->top + 1 ;
	}
	else
	{
		return	eslErrGeneral ;
	}
	if ( pSrcRect != NULL )
	{
		rctSrcView.left = pSrcRect->left ;
		rctSrcView.top = pSrcRect->top ;
		rctSrcView.right = pSrcRect->right + 1 ;
		rctSrcView.bottom = pSrcRect->bottom + 1 ;
	}
	else
	{
		rctSrcView.left = 0 ;
		rctSrcView.top = 0 ;
		rctSrcView.right = sizeDst.w ;
		rctSrcView.bottom = sizeDst.h ;
	}
	rctDstView.left = xDst ;
	rctDstView.top = yDst ;
	rctDstView.right = xDst + sizeDst.w ;
	rctDstView.bottom = yDst + sizeDst.h ;
	//
	::eslFillMemory( &bfx, 0, sizeof(bfx) ) ;
	bfx.dwSize = sizeof(bfx) ;
	bfx.dwROP = SRCCOPY ;
	bfx.dwDDFX = 0 /*DDBLTFX_ARITHSTRETCHY*/ ;
	//
	if ( pddsDst->IsLost() )
	{
		pddsDst->Restore( ) ;
	}
	if ( pddsSrc->IsLost() )
	{
		pddsSrc->Restore( ) ;
	}
	if ( fAsync )
	{
		if ( pddsDst->Blt
			( &rctDstView, pddsSrc, &rctSrcView,
				DDBLT_ASYNC | DDBLT_ROP /*| DDBLT_DDFX*/, &bfx ) != DD_OK )
		{
			ESLTrace( "Failed to draw image with DirectDraw7 asynchronously.\n" ) ;
		}
		else
		{
			return	eslErrSuccess ;
		}
	}
	if ( pddsDst->Blt
		( &rctDstView, pddsSrc, &rctSrcView,
			DDBLT_WAIT | DDBLT_ROP /*| DDBLT_DDFX*/, &bfx ) != DD_OK )
	{
		ESLTrace( "Failed to draw image with DirectDraw7.\n" ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

ESLError EGLDrawImage::BltSurface
	( IDirectDrawSurface * pddsDst,
		IDirectDrawSurface * pddsSrc,
		int xDst, int yDst,
		const EGL_SIZE * pDstSize,
		const EGL_RECT * pSrcRect, bool fAsync )
{
	DDBLTFX		bfx ;
	RECT		rctDstView ;
	RECT		rctSrcView ;
	EGL_SIZE	sizeDst ;
	//
	if ( pDstSize != NULL )
	{
		sizeDst = *pDstSize ;
	}
	else if ( pSrcRect != NULL )
	{
		sizeDst.w = pSrcRect->right - pSrcRect->left + 1 ;
		sizeDst.h = pSrcRect->bottom - pSrcRect->top + 1 ;
	}
	else
	{
		return	eslErrGeneral ;
	}
	if ( pSrcRect != NULL )
	{
		rctSrcView.left = pSrcRect->left ;
		rctSrcView.top = pSrcRect->top ;
		rctSrcView.right = pSrcRect->right + 1 ;
		rctSrcView.bottom = pSrcRect->bottom + 1 ;
	}
	else
	{
		rctSrcView.left = 0 ;
		rctSrcView.top = 0 ;
		rctSrcView.right = sizeDst.w ;
		rctSrcView.bottom = sizeDst.h ;
	}
	rctDstView.left = xDst ;
	rctDstView.top = yDst ;
	rctDstView.right = xDst + sizeDst.w ;
	rctDstView.bottom = yDst + sizeDst.h ;
	//
	::eslFillMemory( &bfx, 0, sizeof(bfx) ) ;
	bfx.dwSize = sizeof(bfx) ;
	bfx.dwROP = SRCCOPY ;
	bfx.dwDDFX = 0 /*DDBLTFX_ARITHSTRETCHY*/ ;
	//
	if ( pddsDst->IsLost() )
	{
		pddsDst->Restore( ) ;
	}
	if ( pddsSrc->IsLost() )
	{
		pddsSrc->Restore( ) ;
	}
	if ( fAsync )
	{
		if ( pddsDst->Blt
			( &rctDstView, pddsSrc, &rctSrcView,
				DDBLT_ASYNC | DDBLT_ROP /*| DDBLT_DDFX*/, &bfx ) != DD_OK )
		{
			ESLTrace( "Failed to draw image with DirectDraw asynchronously.\n" ) ;
		}
		else
		{
			return	eslErrSuccess ;
		}
	}
	if ( pddsDst->Blt
		( &rctDstView, pddsSrc, &rctSrcView,
			DDBLT_WAIT | DDBLT_ROP /*| DDBLT_DDFX*/, &bfx ) != DD_OK )
	{
		ESLTrace( "Failed to draw image with DirectDraw.\n" ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

//　サーフェスをフリップする
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::Flip( void )
{
	if ( m_fFullscreen )
	{
		if ( m_fStereo3D && m_idd7sufPrimary )
		{
			if ( m_idd7sufPrimary->Flip
				( NULL, DDFLIP_WAIT | DDFLIP_STEREO ) != DD_OK )
			{
				ESLTrace( "Failed to flip at stereo graphic.\n" ) ;
				return	eslErrGeneral ;
			}
		}
		else if ( m_iddsufPrimary )
		{
			if ( m_iddsufPrimary->Flip( NULL, DDFLIP_WAIT ) != DD_OK )
			{
				ESLTrace( "Failed to flip.\n" ) ;
				return	eslErrGeneral ;
			}
		}
		else
		{
			return	eslErrGeneral ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// VSYNC を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::WaitForVerticalBlank( void )
{
	if ( m_iddraw == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwScanLine, dwLastScan = 0 ;
	for ( ; ; )
	{
		HRESULT	hrslt = m_iddraw->GetScanLine( &dwScanLine ) ;
		if ( hrslt == DD_OK )
		{
			if ( dwScanLine == 0 )
			{
				ESLTrace( "ScanLine 0\n" ) ;
				break ;
			}
			if ( m_dwMaxScanLine < dwScanLine )
			{
				m_dwMaxScanLine = dwScanLine ;
			}
			if ( dwScanLine < dwLastScan )
			{
				if ( (dwLastScan > m_dwMaxScanLine * 14 / 16)
							&& (dwLastScan < m_dwVSyncLimitScanLine) )
				{
					m_dwVSyncLimitScanLine = dwLastScan ;
				}
				ESLTrace( "Failed to synchronize for V-SYNC.(%d,%d,%d,%d)\n",
									dwScanLine, dwLastScan,
									m_dwVSyncLimitScanLine, m_dwMaxScanLine ) ;
				return	eslErrGeneral ;
			}
			if ( dwScanLine < m_dwVSyncLimitScanLine )
			{
				::Sleep( 1 ) ;
			}
			dwLastScan = dwScanLine ;
		}
		else if ( hrslt != DDERR_VERTICALBLANKINPROGRESS )
		{
			ESLTrace( "Failed to IDirectDraw::GetScanLine.\n" ) ;
			return	eslErrGeneral ;
		}
		else
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// VSYNC 用パラメータをリセットする
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::ResetVSync( void )
{
	m_dwVSyncLimitScanLine = -1 ;
	m_dwMaxScanLine = 0 ;
}

// Direct3D9 画面描画
//////////////////////////////////////////////////////////////////////////////
ESLError EGLDrawImage::DrawImageToDisplayD3D9
	( PCEGL_IMAGE_INFO pImageInf, int xPos, int yPos,
		const EGL_SIZE * pDstSize, const EGL_RECT * pSrcRect )
{
	ESLError	err = eslErrFailed ;
	if ( (pImageInf != NULL) && (m_id3d9Dev != NULL) )
	do
	{
		//
		// 描画用サーフェス準備
		//
		EGLSize	sizeVRAM( pImageInf->dwImageWidth, pImageInf->dwImageHeight ) ;
		if ( (m_idds9DispBuf == NULL)
			|| (sizeVRAM != m_sizeD3D9DispBuf) )
		{
			if ( m_idds9DispBuf != NULL )
			{
				m_idds9DispBuf->Release() ;
				m_idds9DispBuf = NULL ;
			}
			m_id3d9Dev->CreateOffscreenPlainSurface
				( sizeVRAM.w, sizeVRAM.h, D3DFMT_A8R8G8B8,
							D3DPOOL_DEFAULT, &m_idds9DispBuf, NULL ) ;
			if ( m_idds9DispBuf == NULL )
			{
				ESLTrace( "Failed to CreateOffscreenPlainSurface.\n" ) ;
				break ;
			}
			m_sizeD3D9DispBuf = sizeVRAM ;
		}
		//
		// 描画用サーフェスに画像データ転送
		//
		EGL_IMAGE_INFO	infSurface ;
		memset( &infSurface, 0, sizeof(infSurface) ) ;
		infSurface.dwInfoSize = sizeof(infSurface) ;
		//
		D3DSURFACE_DESC	d3dsd ;
		if ( m_idds9DispBuf->GetDesc( &d3dsd ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DSurface9::GetDesc.\n" ) ;
			break ;
		}
		D3DLOCKED_RECT	lr ;
		HRESULT	hr = m_idds9DispBuf->LockRect( &lr, NULL, 0 ) ;
		if ( hr != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DSurface9::LockRect.(%08X)\n", hr ) ;
			break ;
		}
		infSurface.fdwFormatType = EIF_RGBA_BITMAP ;
		infSurface.ptrImageArray = lr.pBits ;
		infSurface.dwImageWidth = d3dsd.Width ;
		infSurface.dwImageHeight = d3dsd.Height ;
		infSurface.dwBitsPerPixel = 32 ;
		infSurface.dwBytesPerLine = lr.Pitch ;
		infSurface.dwSizeOfImage =
					infSurface.dwImageHeight * infSurface.dwBytesPerLine ;
		//
		if ( m_hDraw == NULL )
		{
			m_hDraw = ::eglCreateDrawImage() ;
		}
		m_hDraw->Initialize( &infSurface, NULL, NULL ) ;
		//
		EGL_DRAW_PARAM	dp ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.pSrcImage = (PEGL_IMAGE_INFO) pImageInf ;
		//
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			m_hDraw->DrawImage() ;
		}
		//
		m_idds9DispBuf->UnlockRect() ;
		//
		// 画面に描画
		//
		if ( m_id3d9Dev->BeginScene() != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::BeginScene.\n" ) ;
		}
		//
		IDirect3DSurface9 *	id3dsBack = NULL ;
		if ( m_id3d9Dev->GetBackBuffer
			( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::GetBackBuffer.\n" ) ;
			break ;
		}
		//
		if ( m_id3d9Dev->Clear
			( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
					D3DCOLOR_ARGB(0xFF,0,0,0), 1.0, 0 ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::Clear.\n" ) ;
		}
		//
		RECT	rcSrc, rcDst ;
		if ( pSrcRect != NULL )
		{
			rcSrc.left = pSrcRect->left ;
			rcSrc.top = pSrcRect->top ;
			rcSrc.right = pSrcRect->right + 1 ;
			rcSrc.bottom = pSrcRect->bottom + 1 ;
		}
		else
		{
			rcSrc.left = 0 ;
			rcSrc.top = 0 ;
			rcSrc.right = sizeVRAM.w ;
			rcSrc.bottom = sizeVRAM.h ;
		}
		rcDst.left = xPos ;
		rcDst.top = yPos ;
		if ( pDstSize != NULL )
		{
			rcDst.right = xPos + pDstSize->w ;
			rcDst.bottom = yPos + pDstSize->h ;
			//
			if ( pDstSize->h < 0 )
			{
				rcDst.top = yPos + pDstSize->h + 1 ;
				rcDst.bottom = yPos + 1 ;
			}
		}
		else
		{
			rcDst.right = xPos + sizeVRAM.w ;
			rcDst.bottom = yPos + sizeVRAM.h ;
		}
//		ESLAssert( rcDst.left >= 0 ) ;
//		ESLAssert( rcDst.top >= 0 ) ;
//		ESLAssert( rcDst.right <= (int) m_pd3dpp->BackBufferWidth ) ;
//		ESLAssert( rcDst.bottom <= (int) m_pd3dpp->BackBufferHeight ) ;
		if ( rcDst.left < 0 )
		{
			rcDst.left = 0 ;
		}
		if ( rcDst.top < 0 )
		{
			rcDst.top = 0 ;
		}
		if ( rcDst.right > (int) m_pd3dpp->BackBufferWidth )
		{
			rcDst.right = (int) m_pd3dpp->BackBufferWidth ;
		}
		if ( rcDst.bottom > (int) m_pd3dpp->BackBufferHeight )
		{
			rcDst.bottom = (int) m_pd3dpp->BackBufferHeight ;
		}
		//
		err = eslErrSuccess ;
		//
		hr = m_id3d9Dev->StretchRect
			( m_idds9DispBuf, &rcSrc,
				id3dsBack, &rcDst, D3DTEXF_LINEAR ) ;
		if ( hr != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::StretchRect.(%08X)\n", hr ) ;
			ESLTrace( "(%08X:%s)\n", hr, DXGetErrorString(hr) ) ;
			err = eslErrFailed ;
		}
		if ( m_id3d9Dev->EndScene() != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::EndScene.\n" ) ;
		}
		hr = m_id3d9Dev->Present( NULL, NULL, NULL, NULL ) ;
		//
		id3dsBack->Release() ;
		//
		if ( hr != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::Present.(%08X)\n", hr ) ;
			ESLTrace( "(%08X:%s)\n", hr, DXGetErrorString(hr) ) ;
			err = eslErrFailed ;
			//
			if ( (hr == D3DERR_DEVICEREMOVED) || (hr == D3DERR_DEVICELOST)
				&& (m_id3d9Dev->TestCooperativeLevel() != D3DERR_DEVICELOST) )
			{
				//
				// ロストデバイスの復帰
				//
				ESLError	errReset ;
				errReset = ResetDirect3D9Device() ;
				//
				if ( !errReset &&
					(m_id3d9Dev->GetBackBuffer
						( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) == D3D_OK) )
				{
					//
					// 描画しなおす
					//
					m_id3d9Dev->Clear
						( 0, NULL, D3DCLEAR_TARGET,
							D3DCOLOR_ARGB(0xFF,0,0,0), 1.0, 0 ) ;
					//
					m_id3d9Dev->BeginScene() ;
					m_id3d9Dev->StretchRect
						( m_idds9DispBuf, &rcSrc,
							id3dsBack, &rcDst, D3DTEXF_NONE ) ;
					m_id3d9Dev->EndScene() ;
					m_id3d9Dev->Present( NULL, NULL, NULL, NULL ) ;
					//
					id3dsBack->Release() ;
				}
			}
		}
	}
	while ( false ) ;
	return	err ;
}

// 同期処理
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::Lock( void )
{
	::EnterCriticalSection( &m_csSync ) ;
}

void EGLDrawImage::Unlock( void )
{
	::LeaveCriticalSection( &m_csSync ) ;
}

// 通知オブジェクトを追加する
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::AddNotify( INotify * pNotify )
{
	Lock() ;
	if ( m_lstNotify.FindPtr( pNotify ) < 0 )
	{
		m_lstNotify.Add( pNotify ) ;
	}
	Unlock() ;
}

// 通知を解除する
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::DetachNotify( INotify * pNotify )
{
	Lock() ;
	int	nIndex = m_lstNotify.FindPtr( pNotify ) ;
	if ( nIndex >= 0 )
	{
		m_lstNotify.RemoveAt( nIndex ) ;
	}
	Unlock() ;
}

// OnReleaseDirectDraw を通知する
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::NotifyOnReleaseDirectDraw( void )
{
	Lock() ;
	int	i, nCount = m_lstNotify.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		INotify *	pNotify = m_lstNotify.GetAt( i ) ;
		if ( pNotify != NULL )
		{
			pNotify->OnReleaseDirectDraw( this ) ;
		}
	}
	Unlock() ;
}

// OnCreateDirectDraw を通知する
//////////////////////////////////////////////////////////////////////////////
void EGLDrawImage::NotifyOnCreateDirectDraw( void )
{
	Lock() ;
	int	i, nCount = m_lstNotify.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		INotify *	pNotify = m_lstNotify.GetAt( i ) ;
		if ( pNotify != NULL )
		{
			pNotify->OnCreateDirectDraw( this ) ;
		}
	}
	Unlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// ERI アニメーションファイル再生クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ERIAnimationPlayer, ERIAnimation, EWaveStreamBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ERIAnimationPlayer::ERIAnimationPlayer( void )
{
	m_pdevWave = NULL ;
	m_dwBeginPlayingTime = 0 ;
	m_dwBreakWaveSamples = 0 ;
	m_dwOutputWaveSamples = 0 ;
	m_fQueueWaveOut = false ;
	m_fCancelPlaying = false ;
	m_pddsVRAM = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ERIAnimationPlayer::~ERIAnimationPlayer( void )
{
	DeleteWaveQueueBuffer( ) ;
	//
	if ( m_pddsVRAM != NULL )
	{
		m_pddsVRAM->Release( ) ;
	}
}

// 画像展開出力バッファ要求
//////////////////////////////////////////////////////////////////////////////
EGL_IMAGE_INFO * ERIAnimationPlayer::CreateImageBuffer
	( DWORD format, SDWORD width, SDWORD height, DWORD bpp )
{
	if ( bpp == 24 )
	{
		bpp = 32 ;
	}
	if ( format & ERI_SIDE_BY_SIDE )
	{
		width /= 2 ;
	}
	EGL_IMAGE_INFO *	pImage = ::eglCreateImageBuffer
			( format, width, height, bpp, EGL_IMAGE_HAS_DC ) ;
	::eglReverseVertically( pImage ) ;
	return	pImage ;
}

// 画像展開出力バッファ消去
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::DeleteImageBuffer( EGL_IMAGE_INFO * peii )
{
	::eglDeleteImageBuffer( peii ) ;
}

// 音声出力要求
//////////////////////////////////////////////////////////////////////////////
bool ERIAnimationPlayer::RequestWaveOut
	( DWORD channels, DWORD frequency, DWORD bps )
{
	if ( m_pdevWave != NULL )
	{
		WAVEFORMATEX	wfx ;
		::memset( &wfx, 0, sizeof(wfx) ) ;
		wfx.wFormatTag = WAVE_FORMAT_PCM ;
		wfx.nChannels = (WORD) channels ;
		wfx.nSamplesPerSec = frequency ;
		wfx.wBitsPerSample = (WORD) bps ;
		wfx.nBlockAlign = (WORD) (bps * channels / 8) ;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign ;
		//
		SetWaveFormat( &wfx ) ;
		m_fQueueWaveOut = true ;
		//
		return	true ;
	}
	return	false ;
}

// 音声出力終了
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::CloseWaveOut( void )
{
	m_pdevWave = NULL ;
}

// 音声データ出力
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::PushWaveBuffer( void * ptrWaveBuf, DWORD dwBytes )
{
	if ( m_pdevWave != NULL )
	{
		HWAVEBUF	hWaveBuf =
			m_pdevWave->PrepareBuffer( this, ptrWaveBuf, dwBytes ) ;
		if ( hWaveBuf != NULL )
		{
			m_pdevWave->Lock( ) ;
			m_queWaveData.Add( hWaveBuf ) ;
			//
			if ( !m_fQueueWaveOut && (m_queWaveData.GetSize() >= 2) )
			{
				hWaveBuf = m_queWaveData.GetAt( 0 ) ;
				m_queWaveData.RemoveAt( 0 ) ;
				m_fQueueWaveOut = true ;
//				m_pdevWave->Lock( ) ;
				if ( m_pdevWave->GetCurrentSample( this ) == 0 )
				{
					m_dwBreakWaveSamples = m_dwOutputWaveSamples ;
				}
				m_dwOutputWaveSamples +=
					BytesToSample( hWaveBuf->dwBufferLength ) ;
				m_pdevWave->Play( this, hWaveBuf ) ;
//				m_pdevWave->Unlock( ) ;
			}
			m_pdevWave->Unlock( ) ;
		}
	}
}

// 音声データの再生が完了した
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::OnEndPlaying
	( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength )
{
	if ( m_pdevWave != NULL )
	{
		m_pdevWave->UnprepareBuffer( hWaveBuf ) ;
		//
		DeleteWaveBuffer( ptrBuffer ) ;
	}
}

// 音声出力デバイスが次の音声バッファを要求している
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF ERIAnimationPlayer::OnQueueNextBuffer( EWaveOutDevice * pWaveDev )
{
	HWAVEBUF	hWaveBuf = m_queWaveData.GetAt( 0 ) ;
	m_queWaveData.RemoveAt( 0 ) ;
	if ( hWaveBuf != NULL )
	{
		m_dwOutputWaveSamples +=
			BytesToSample( hWaveBuf->dwBufferLength ) ;
	}
	else
	{
		m_fQueueWaveOut = false ;
	}
	return	hWaveBuf ;
}

// アニメーションファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ERIAnimationPlayer::Open
	( ESLFileObject * pFile, EWaveOutDevice * pdevWave,
				unsigned int nPreloadSize, DWORD fdwFlags )
{
	m_pdevWave = pdevWave ;
	m_fCancelPlaying = false ;
	//
	return	ERIAnimation::Open
		( pFile, nPreloadSize, fdwFlags | ERISADecoder::dfTopDown ) ;
}

// アニメーションを再生する
//////////////////////////////////////////////////////////////////////////////
ESLError ERIAnimationPlayer::Play
	( HWND hwndTarget, int xPos, int yPos,
		const EGL_SIZE * pViewSize,
		DWORD fdwFlags, EGLDrawImage * pDrawImage )
{
	return	PlayTo
		( GetAllFrameCount(),
			hwndTarget, xPos, yPos, pViewSize, fdwFlags, pDrawImage ) ;
}

ESLError ERIAnimationPlayer::PlayTo
	( unsigned int nEndFrame,
		HWND hwndTarget, int xPos, int yPos,
		const EGL_SIZE * pViewSize,
		DWORD fdwFlags, EGLDrawImage * pDrawImage )
{
	ESLError		errResult = eslErrSuccess ;
	unsigned int	iCurrentFrame = CurrentIndex( ) ;
	unsigned int	nTotalFrame = GetAllFrameCount( ) ;
	unsigned int	nTimePerFrame = GetTotalTime() ;
	DWORD			dwStartFrameTime = FrameIndexToTime( iCurrentFrame ) ;
	if ( nTotalFrame != 0 )
	{
		nTimePerFrame /= nTotalFrame ;
	}
	if ( nEndFrame > nTotalFrame )
	{
		nEndFrame = nTotalFrame ;
	}
	m_dwPlayEndFrame = nEndFrame ;
	//
	const EGL_IMAGE_INFO *	pImage = GetImageInfo( ) ;
	ESLAssert( pImage != NULL ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pDrawImage != NULL )
	{
		if ( m_pddsVRAM != NULL )
		{
			m_pddsVRAM->Release( ) ;
		}
		m_pddsVRAM = pDrawImage->CreateSurfaceOnVRAM
			( (int) pImage->dwImageWidth, (int) pImage->dwImageHeight ) ;
	}
	//
	BeginWaveStreaming( ) ;
	//
	DWORD	dwDrawingTime = 0, dwSeekingTime = 0 ;
	DWORD	dwWaitTime = 0 ;
	SDWORD	dwShortFrameTime = 0 ;
	m_dwBeginPlayingTime = ::timeGetTime( ) ;
	bool	fWrapAroundPlay = false ;
	//
	while ( !m_fCancelPlaying )
	{
		//
		// 現在のフレームを描画する
		//
//		DWORD	dwBeginDraw = ::timeGetTime( ) ;
		pImage = GetImageInfo( ) ;
		OnDrawMovieFrame
			( hwndTarget, xPos, yPos,
				pViewSize, pDrawImage, pImage, dwWaitTime ) ;
//		DWORD	dwEndDraw = ::timeGetTime( ) ;
//		dwDrawingTime = (dwDrawingTime + (dwEndDraw - dwBeginDraw)) / 2 ;
		if ( fWrapAroundPlay )
		{
			break ;
		}
		//
		// シークするべきフレームを計算する
		//
		DWORD	dwCurrent = GetCurrentPlayingTime( ) + dwStartFrameTime ;
		unsigned int	nSkip =
			GetBestSkipFrames( dwCurrent + (nTimePerFrame / 2) ) ;
		if ( (iCurrentFrame + nSkip + 1 >= m_dwPlayEndFrame)
			|| (iCurrentFrame + nSkip + 1 >= GetAllFrameCount()) )
		{
			if ( nSkip == 0 )
			{
				break ;
			}
			fWrapAroundPlay = true ;
			nSkip = m_dwPlayEndFrame - iCurrentFrame - 2 ;
		}
		DWORD	dwBeginSeek = ::timeGetTime( ) ;
		if ( fdwFlags & ptfNoSkipFrame )
		{
			nSkip = 0 ;
		}
		if ( SeekToNextFrame( nSkip ) )
		{
			break ;
		}
		DWORD	dwEndSeek = ::timeGetTime( ) ;
		dwSeekingTime = (dwSeekingTime + (dwEndSeek - dwBeginSeek)) / 2 ;
		//
		// ウィンドウメッセージを処理する
		//
		if ( fdwFlags & ptfDispatchMessage )
		{
			errResult = OnDispatchMessage( ) ;
			if ( errResult )
			{
				break ;
			}
		}
		//
		// 次のフレームを描画すべきタイミングまで待つ
		//
		unsigned int	nNextTiming ;
//		DWORD			dwTimeBias = (dwDrawingTime + dwSeekingTime) / 2 ;
		iCurrentFrame = CurrentIndex( ) ;
		nNextTiming = FrameIndexToTime( iCurrentFrame - nSkip ) ;
		dwCurrent = GetCurrentPlayingTime( ) + dwStartFrameTime ;
//		if ( dwCurrent + dwTimeBias < nNextTiming )
		if ( dwCurrent < nNextTiming )
		{
//			dwWaitTime = nNextTiming - (dwCurrent + dwTimeBias) ;
			dwWaitTime = nNextTiming - dwCurrent ;
			if ( dwWaitTime > nTimePerFrame )
			{
				dwWaitTime = nTimePerFrame ;
			}
//			errResult = OnWaitingTime( dwWaitTime ) ;
//			if ( errResult )
//			{
//				break ;
//			}
		}
		else
		{
			dwWaitTime = 0 ;
		}
	}
	//
	EndWaveStreaming( ) ;
	//
	if ( m_pddsVRAM != NULL )
	{
		m_pddsVRAM->Release( ) ;
		m_pddsVRAM = NULL ;
	}
	if ( !errResult && m_fCancelPlaying )
	{
		errResult = eslErrTimeout ;
	}
	m_fCancelPlaying = false ;
	//
	return	errResult ;
}

// アニメーション再生を中断する
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::CancelPlaying( void )
{
	m_fCancelPlaying = true ;
}

// アニメーション終了フレームを変更する
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::SetPlayEndFrame( DWORD dwEndFrame )
{
	m_dwPlayEndFrame = dwEndFrame ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
ESLError ERIAnimationPlayer::OnDrawMovieFrame
	( HWND hwndTarget, int xPos, int yPos,
		const EGL_SIZE * pViewSize,
		EGLDrawImage * pDrawImage,
		PCEGL_IMAGE_INFO pImage, DWORD dwDuringTime )
{
	DWORD	dwBeginTime = ::timeGetTime( ) ;
	if ( pImage != NULL )
	{
		if ( pDrawImage != NULL )
		{
			const DWORD	fdwFlags =
				EGLDrawImage::dfDirectDraw | EGLDrawImage::dfWaitVerticalBlank ;
			pDrawImage->DrawImageToDisplay
				( hwndTarget, pImage,
					xPos, yPos, pViewSize, NULL, fdwFlags, m_pddsVRAM ) ;
		}
		else
		{
			HDC	hdc = ::GetDC( hwndTarget ) ;
			::eglDrawToDC( hdc, pImage, xPos, yPos, pViewSize, NULL ) ;
			::ReleaseDC( hwndTarget, hdc ) ;
		}
	}
	DWORD	dwEndTime = ::timeGetTime( ) ;
	DWORD	dwDrawingTime = dwEndTime - dwBeginTime ;
	if ( dwDrawingTime < dwDuringTime )
	{
		return	OnWaitingTime( dwDuringTime - dwDrawingTime ) ;
	}
	return	eslErrSuccess ;
}

// 再生中のメッセージ処理
//////////////////////////////////////////////////////////////////////////////
ESLError ERIAnimationPlayer::OnDispatchMessage( void )
{
	for ( int i = 0; i < 0x20; i ++ )
	{
		MSG	msg ;
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
	}
	return	eslErrSuccess ;
}

// 時間待ち
//////////////////////////////////////////////////////////////////////////////
ESLError ERIAnimationPlayer::OnWaitingTime( DWORD dwTime )
{
	::Sleep( dwTime ) ;
	return	eslErrSuccess ;
}

// 再生中のアニメーション時間取得
//////////////////////////////////////////////////////////////////////////////
DWORD ERIAnimationPlayer::GetCurrentPlayingTime( void ) const
{
	if ( m_pdevWave )
	{
		unsigned long int	nCurrentSamples =
				(unsigned long int) m_pdevWave->GetCurrentSample( this ) ;
		if ( nCurrentSamples != 0 )
		{
			unsigned int	nMilliSec =
				SampleToTime( m_dwBreakWaveSamples + nCurrentSamples ) ;
			return	nMilliSec ;
		}
	}
	return	::timeGetTime( ) - m_dwBeginPlayingTime ;
}

// 音声ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::BeginWaveStreaming( void )
{
	m_fQueueWaveOut = true ;
	m_dwBreakWaveSamples = 0 ;
	m_dwOutputWaveSamples = 0 ;
	DeleteWaveQueueBuffer( ) ;
	//
	ERIAnimation::BeginWaveStreaming( ) ;
	//
	if ( m_pdevWave != NULL )
	{
		HWAVEBUF	hWaveBuf = m_queWaveData.GetAt( 0 ) ;
		m_queWaveData.RemoveAt( 0 ) ;
		if ( hWaveBuf != NULL )
		{
			m_fQueueWaveOut = true ;
			m_dwOutputWaveSamples +=
				BytesToSample( hWaveBuf->dwBufferLength ) ;
			m_pdevWave->Play( this, hWaveBuf ) ;
		}
	}
	else
	{
		m_fQueueWaveOut = false ;
	}
}

// 音声ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::EndWaveStreaming( void )
{
	if ( m_pdevWave != NULL )
	{
		m_pdevWave->Stop( this ) ;
	}
	DeleteWaveQueueBuffer( ) ;
	//
	if ( m_pddsVRAM != NULL )
	{
		m_pddsVRAM->Release( ) ;
		m_pddsVRAM = NULL ;
	}
	//
	ERIAnimation::EndWaveStreaming( ) ;
}

// 音声バッファ削除
//////////////////////////////////////////////////////////////////////////////
void ERIAnimationPlayer::DeleteWaveQueueBuffer( void )
{
	if ( m_pdevWave != NULL )
	{
		m_pdevWave->Lock( ) ;
	}
	for ( int i = 0; i < (int) m_queWaveData.GetSize(); i ++ )
	{
		HWAVEBUF	hWaveBuf = m_queWaveData.GetAt( i ) ;
		void *		ptrWaveBuf = hWaveBuf->lpData ;
		if ( m_pdevWave != NULL )
		{
			m_pdevWave->UnprepareBuffer( hWaveBuf ) ;
		}
		DeleteWaveBuffer( ptrWaveBuf ) ;
	}
	m_queWaveData.RemoveAll( ) ;
	if ( m_pdevWave != NULL )
	{
		m_pdevWave->Unlock( ) ;
	}
}

