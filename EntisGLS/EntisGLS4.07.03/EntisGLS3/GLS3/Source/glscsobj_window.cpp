
/*****************************************************************************
               Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2003-2021 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#endif


//////////////////////////////////////////////////////////////////////////////
// スクリプトインターフェース＋ウィンドウインターフェース
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// ECSWindow::EInterface クラス
//////////////////////////////////////////////////////////////////////////////
// このクラスは、ECSSprite クラスと一体となって機能する
// 1つだけ ECSSprite オブジェクトを子に持ち、
// ECSSprite の画像バッファを参照する。
// 子供の ECSSprite オブジェクトの操作を
// EWindowSpriteInterface に伝えるためだけのクラスである。
// しかし、画像バッファを二重に所有しないのでロスはない。
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSWindow::EInterface, EWindowSpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EInterface::EInterface( void )
{
	m_pWnd = NULL ;
	m_hArrow = NULL ;
	m_fProcMsg = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EInterface::~EInterface( void )
{
	DetachSprite( m_pWnd ) ;
}

// 更新領域を再描画
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::EInterface::Refresh( void )
{
	if ( m_pWnd != NULL )
	{
		m_pWnd->Refresh( ) ;
	}
	FlushUpdatedRect( ) ;
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT ECSWindow::EInterface::WindowProc
	( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_CLOSE )
	{
		Lock( ) ;
		QueueCommand
			( L"ID_APP_EXIT", 0, 0, EWndSpriteCmd::priorityCritical, true ) ;
		Unlock( ) ;
		return	0 ;
	}
	else if ( uMsg == WM_SIZE )
	{
		if ( m_pWnd != NULL )
		{
			m_pWnd->Lock() ;
			m_pWnd->UpdateImagePosition() ;
			m_pWnd->Unlock() ;
		}
	}
	else if ( uMsg == WM_PAINT )
	{
		if ( m_pWnd != NULL )
		{
			m_pWnd->Lock() ;
			m_pWnd->UpdateImagePosition() ;
			m_pWnd->Unlock() ;
		}
	}
#if	defined(_DEBUG)
	else if ( uMsg == WM_KEYUP )
	{
		if ( wParam == VK_PAUSE )
		{
			if ( m_pWnd != NULL )
			{
				ECSContext *	pContext = m_pWnd->GetContext( ) ;
				if ( pContext != NULL )
				{
					pContext->SetStatus( ECSContext::xsHalt ) ;
				}
			}
		}
	}
#endif
	LRESULT	lr =
		EWindowSpriteInterface::WindowProc( pWnd, uMsg, wParam, lParam ) ;
	return	lr ;
}

// マウスカーソルを設定する
//////////////////////////////////////////////////////////////////////////////
bool ECSWindow::EInterface::OnSetCursor( int xPos, int yPos )
{
	if ( m_pWnd && m_pWnd->IsShowCursor() )
	{
		if ( !EWindowSpriteInterface::OnSetCursor( xPos, yPos ) )
		{
			SetMouseCursor( L"IDC_ARROW" ) ;
		}
		return	true ;
	}
	else
	{
		::SetCursor( NULL ) ;
	}
	return	true ;
}

// マウスカーソルを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::EInterface::SetMouseCursor( const wchar_t * pwszID )
{
	if ( m_pWnd && m_pWnd->IsShowCursor() )
	{
		HCURSOR				hCursor = NULL ;
		ECSEnvironment *	pEnv = m_pWnd->GetEnvironment( ) ;
		if ( pEnv != NULL )
		{
			hCursor = pEnv->GetCursorAs( pwszID ) ;
		}
		if ( hCursor )
		{
			::SetCursor( hCursor ) ;
			return	eslErrSuccess ;
		}
		if ( m_hArrow == NULL )
		{
			m_hArrow = ::LoadCursor( NULL, IDC_ARROW ) ;
		}
		::SetCursor( m_hArrow ) ;
	}
	return	EWindowSpriteInterface::SetMouseCursor( pwszID ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ECSWindow::EThread クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSWindow::EThread, EGLSThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EThread::EThread( void )
{
	m_pWnd = NULL ;
	m_hEventCreated = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EThread::~EThread( void )
{
	::CloseHandle( m_hEventCreated ) ;
}

// ウィンドウ作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::EThread::CreateDisplay( ECSWindow * pWnd )
{
	m_pWnd = pWnd ;
	if ( pWnd == NULL )
	{
		return	eslErrGeneral ;
	}
	::ResetEvent( m_hEventCreated ) ;
	ESLError	err = BeginThread( ) ;
	if ( err )
	{
		return	err ;
	}
//	::SetThreadPriority( Handle(), THREAD_PRIORITY_BELOW_NORMAL ) ;
	//
	HANDLE	hHandles[2] = { m_hEventCreated, Handle() } ;
	if ( ::WaitForMultipleObjects
		( 2, hHandles, FALSE, INFINITE ) != WAIT_OBJECT_0 )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ECSWindow::EThread::ThreadProc( void )
{
	::CoInitialize( NULL ) ;
	//
	// ウィンドウ作成
	//
	ESLAssert( m_pWnd != NULL ) ;
	if ( m_pWnd->CreateDisplayWindow( NULL ) )
	{
		return	1 ;
	}
	EInterface *	pInterface = m_pWnd->GetInterface() ;
	EGLDrawImage *	pDrawImage =
		pInterface ? pInterface->GetAttachedDrawImageObject() : NULL ;
	if ( (pDrawImage != NULL)
		&& (pDrawImage->GetDirect3DDevice9() == NULL) )
	{
		EDisplayMode	dmMode ;
		dmMode.PrepareMonitorAPIs() ;
		//
		EString		strDispName ;
		HMONITOR	hMonitor ;
		RECT		rectWindow ;
		m_pWnd->GetWindow()->GetWindowRect( &rectWindow ) ;
		if ( dmMode.GetDisplayNameFromRect
			( strDispName, &rectWindow, &hMonitor ) != NULL )
		{
			GUID	guid ;
			int		nIndex ;
			GUID *	pGUID =
				dmMode.GetDDMonitorGUID( &guid, hMonitor, &nIndex ) ;
			if ( pGUID != NULL )
			{
				m_pWnd->GetWindow()->GetClientRect( &rectWindow ) ;
				pDrawImage->CreateDirect3D9Device
					( *(m_pWnd->GetWindow()),
						rectWindow.right - rectWindow.left,
						rectWindow.bottom - rectWindow.top, nIndex, TRUE ) ;
			}
		}
	}
	::SetEvent( m_hEventCreated ) ;
	//
	// メッセージループ
	//
#if	_MSC_VER >= 1800
	if ( IsWindowsVistaOrGreater() )
#else
	OSVERSIONINFO	osvi ;
	osvi.dwOSVersionInfoSize = sizeof(osvi) ;
	::GetVersionEx( &osvi ) ;
	if ( osvi.dwMajorVersion >= 6 )
#endif
	{
		EWindow *	pWnd = m_pWnd->GetWindow() ;
		SSystem::STimeCounter	timerLast ;
		SSystem::STimeCounter	timerMsg ;
		ESLEventObject			eventDummy ;
		eventDummy.CreateEvent( false ) ;
		bool	fMode60fps = true ;
		bool	fModeMinimized = ((pWnd->GetStyle() & WS_MINIMIZE) != 0) ;
		const UINT	wMsgFilters[][2] =
		{
			{ WM_MOUSEFIRST, WM_MOUSELAST },
			{ 0, 0 },
		} ;
		for ( ; ; )
		{
			MSG		msg ;
			bool	fPaintDelay = false ;
			bool	fQuit = false ;
			int		j = fModeMinimized ? 1 : 0 ;
			for ( int i = 0; i < 0x20; i ++ )
			{
				bool	fMsg = false ;
				if ( fModeMinimized )
				{
					if ( ::GetMessage( &msg, NULL, 0, 0 ) )
					{
						fMsg = true ;
					}
					else
					{
						fQuit = true ;
						break ;
					}
				}
				else
				{
					fMsg = (::PeekMessage
						( &msg, NULL,
							wMsgFilters[j][0],
							wMsgFilters[j][1], PM_REMOVE ) != 0) ;
				}
				if ( fMsg )
				{
					timerMsg.Reset() ;
					DispatchMessage( msg ) ;
					fModeMinimized = ((pWnd->GetStyle() & WS_MINIMIZE) != 0) ;
					if ( msg.message == WM_QUIT )
					{
						fQuit = true ;
						break ;
					}
					if ( msg.message == WM_TIMER )
					{
						timerLast.Reset() ;
						if ( (pWnd != NULL)
//							&& !m_pWnd->m_pInterface->IsQueueCommand()
							&& !fModeMinimized )
						{
							timerMsg.Reset() ;
							pWnd->UpdateWindow() ;
							fMode60fps = (timerMsg.GetRealTime() <= 15) ;
							fPaintDelay = !fMode60fps ;
						}
						break ;
					}
					else if ( msg.message == WM_PAINT )
					{
						fMode60fps = (timerMsg.GetRealTime() <= 15) ;
						fPaintDelay = !fMode60fps ;
						break ;
					}
					if ( fModeMinimized )
					{
						break ;
					}
				}
				else if ( wMsgFilters[j][0] != 0 )
				{
					j ++ ;
				}
				else
				{
					break ;
				}
			}
			if ( fQuit )
			{
				break ;
			}
			if ( fPaintDelay
				|| m_pWnd->m_pInterface->IsQueueCommand() )
			{
				::Sleep( 1 ) ;
			}
			uint64_t	nCurrentTime = timerLast.GetTime() ;
			DWORD		dwTimeout = fMode60fps ? 16 : 33 ;
			if ( dwTimeout > nCurrentTime )
			{
				dwTimeout = dwTimeout - (DWORD) nCurrentTime ;
			}
			else
			{
				dwTimeout = 1 ;
			}
			if ( fModeMinimized )
			{
				::Sleep( dwTimeout ) ;
			}
			else
			{
				HANDLE	hEventDummy = eventDummy ;
				DWORD	dwWaitResult =
					::MsgWaitForMultipleObjects
						( 1, &hEventDummy, FALSE, dwTimeout, QS_ALLEVENTS ) ;
				if ( (dwWaitResult == WAIT_TIMEOUT)
					|| (timerMsg.GetTime() >= dwTimeout) )
				{
					if ( fMode60fps && (pWnd != NULL)
//						&& !m_pWnd->m_pInterface->IsQueueCommand()
						&& !fModeMinimized )
					{
						pWnd->SendMessage( WM_TIMER, 1 ) ;
						timerLast.Reset() ;
						//
						timerMsg.Reset() ;
						pWnd->UpdateWindow() ;
						fMode60fps = (timerMsg.GetRealTime() <= 15) ;
						if ( !fMode60fps )
						{
							::Sleep( 1 ) ;
						}
					}
				}
			}
		}
	}
	else
	{
		EGLSThread::ThreadProc( ) ;
	}
	//
	// ウィンドウを閉じる
	//
	m_pWnd->m_pwndDisplay->CloseDisplay( ) ;
	//
//	::CoUninitialize( ) ;
	//
	return	0 ;
}


//////////////////////////////////////////////////////////////////////////////
// ECSWindow クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSWindow, ECSSprite )

LONG			ECSWindow::m_nTotalWindowCount = 0 ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::ECSWindow( void )
{
	m_dwFlags |= ffTabStop ;
	//
	m_statusCreated = statusUncreated ;
	m_pContext = NULL ;
	m_pEnv = NULL ;
	m_pwndDisplay = NULL ;
	m_pInterface = NULL ;
	m_pInputFilter = NULL ;
	//
	m_pCallbackContext = NULL ;
	//
	m_fCooperationLevel = EGameWindow::levelWindow ;
	m_fOptionFunctions = EGameWindow::BlackBack ;
	m_fInitChangeModeFlag = false ;
	m_posInitWindow.x = 0x80000000 ;
	m_posInitWindow.y = 0x80000000 ;
	m_sizeInitWindow.cx = 0x80000000 ;
	m_sizeInitWindow.cy = 0x80000000 ;
	m_sizeDisplay.w = 0 ;
	m_sizeDisplay.h = 0 ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_hMainIcon = 0 ;
	m_fShowCursor = false ;
	m_pivView3D = NULL ;
	//
	m_dwTlsIndex = 0xFFFFFFFF ;
	m_ptldListFirst = NULL ;
	m_pwndPrevList = NULL ;
	m_pwndNextList = NULL ;
	//
	m_ppiw = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSWindow::~ECSWindow( void )
{
	CloseDisplay( ) ;
	//
	if ( m_pivView3D != NULL )
	{
		m_pivView3D->Release() ;
	}
	//
	delete	m_ppiw ;
}

// ウィンドウ作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CreateDisplay
	( const char * pszWindowName,
		EGameWindow::CooperationLevel fCooperationLevel,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency, HICON hIcon )
{
	//
	// 作成の準備
	//
	CloseDisplay( ) ;
	//
	if ( m_dwTlsIndex == 0xFFFFFFFF )
	{
		m_dwTlsIndex = ::TlsAlloc( ) ;
	}
	m_strWindowName = pszWindowName ;
	m_fCooperationLevel = fCooperationLevel ;
	m_sizeDisplay.w = nWidth ;
	m_sizeDisplay.h = nHeight ;
	m_nBitsPerPixel = nBitsPerPixel ;
	m_nFrequency = nFrequency ;
	m_hMainIcon = hIcon ;
	//
	// インターフェース・画像バッファ準備
	//
	if ( PrepareDisplayWindow() )
	{
		return	eslErrFailed ;
	}
	AddToWidowList( ) ;
	//
	// スレッド起動
	//
	ESLError	err = m_wndThread.CreateDisplay( this ) ;
	//
	m_statusCreated = statusCreateDisplay ;
	//
	return	err ;
}

// ウィンドウ作成（フルスクリーン以外・子ウィンドウ可能）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CreateDisplayWindow
	( const char * pszWindowName,
		unsigned int nWidth, unsigned int nHeight,
		EWindowSpriteInterface * pwndParent, HICON hIcon )
{
	if ( pwndParent == NULL )
	{
		return	CreateDisplay
					( pszWindowName,
						EGameWindow::levelWindow,
						nWidth, nHeight, 0, 0, hIcon ) ;
	}
	//
	// 以前のウィンドウを削除
	//
	CloseDisplay( ) ;
	//
	// 作成の準備
	//
	EWindow *	pWnd = pwndParent->GetWindow() ;
	if ( (pWnd == NULL) || !::IsWindow( *pWnd ) )
	{
		return	eslErrFailed ;
	}
	//
	m_strWindowName = pszWindowName ;
	m_fCooperationLevel = EGameWindow::levelWindow ;
	m_sizeDisplay.w = nWidth ;
	m_sizeDisplay.h = nHeight ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_hMainIcon = hIcon ;
	//
	// インターフェース・画像バッファ準備
	//
	if ( PrepareDisplayWindow() )
	{
		return	eslErrFailed ;
	}
	ESLAssert( m_pInterface != NULL ) ;
	m_pInterface->AttachSyncObject( pwndParent ) ;
	//
	// ウィンドウスレッドからウィンドウ作成
	//
	CREATE_WINDOW	cw ;
	LRESULT			lResult ;
	cw.pWnd = this ;
	cw.errResult = eslErrFailed ;
	cw.hwndParent = *pWnd ;
	cw.eventDone.CreateEvent() ;
	//
	if ( pwndParent->ProcedureOnWindowThread
		( CallOnWinThread_CreateWindow, &cw, &lResult, true ) )
	{
		return	eslErrFailed ;
	}
	cw.eventDone.Wait( INFINITE ) ;
	//
	m_statusCreated = statusCreateWindow ;
	//
	return	cw.errResult ;
}

ESLError ECSWindow::CreateDisplayWindow
	( const char * pszWindowName,
		unsigned int nWidth, unsigned int nHeight,
		ECSWindow * pParentWnd, HICON hIcon )
{
	EWindowSpriteInterface *	pwndParent = NULL ;
	if ( pParentWnd != NULL )
	{
		pwndParent = pParentWnd->GetInterface() ;
	}
	if ( CreateDisplayWindow
		( pszWindowName, nWidth, nHeight, pwndParent, hIcon ) )
	{
		return	eslErrFailed ;
	}
	m_refParentWindow.SetReference( pParentWnd, m_pContext ) ;
	return	eslErrSuccess ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_CreateWindow( void * pInstance )
{
	CREATE_WINDOW *	pcw = (CREATE_WINDOW*) pInstance ;
	if ( pcw->pWnd->CreateDisplayWindow( pcw->hwndParent ) )
	{
		pcw->errResult = eslErrFailed ;
		pcw->eventDone.SetEvent() ;
		return	1 ;
	}
	pcw->errResult = eslErrSuccess ;
	pcw->eventDone.SetEvent() ;
	return	0 ;
}

// インターフェースのみを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CreateInterface
	( DWORD fdwFormat, unsigned int nWidth,
		unsigned int nHeight, unsigned int nBitsPerPixel )
{
	//
	// 作成の準備
	//
	if ( (m_pwndDisplay != NULL) || (m_pInterface != NULL) )
	{
		CloseDisplay( ) ;
	}
	if ( m_dwTlsIndex == 0xFFFFFFFF )
	{
		m_dwTlsIndex = ::TlsAlloc( ) ;
	}
	m_sizeDisplay.w = nWidth ;
	m_sizeDisplay.h = nHeight ;
	m_nBitsPerPixel = nBitsPerPixel ;
	//
	// インターフェース・画像バッファ準備
	//
	if ( PrepareDisplayWindow( true ) )
	{
		return	eslErrFailed ;
	}
	AddToWidowList( ) ;
	//
	return	eslErrSuccess ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::CloseDisplay( void )
{
	RemoveFromWindowList( ) ;
	//
	if ( m_pwndDisplay != NULL )
	{
		if ( m_statusCreated == statusCreateWindow )
		{
			if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
											!= ::GetCurrentThreadId() )
			{
				if ( m_pInterface != NULL )
				{
					CLOSE_WINDOW	cw ;
					LRESULT			lResult ;
					cw.pWnd = this ;
					cw.eventDone.CreateEvent() ;
					//
					if ( !m_pInterface->ProcedureOnWindowThread
						( CallOnWinThread_CloseWindow, &cw, &lResult, true ) )
					{
						cw.eventDone.Wait( INFINITE ) ;
					}
				}
			}
			else
			{
				m_pwndDisplay->CloseDisplay() ;
			}
		}
		else
		{
			m_pwndDisplay->PostMessage( WM_QUIT ) ;
			::WaitForSingleObject( m_wndThread.Handle(), 10000 ) ;
		}
	}
	if ( m_pInterface != NULL )
	{
		m_pInterface->DetachView( this ) ;
	}
	delete	m_pwndDisplay ;
	delete	m_pInterface ;
	m_pwndDisplay = NULL ;
	m_pInterface = NULL ;
	m_pInputFilter = NULL ;
	//
	m_statusCreated = statusUncreated ;
	m_sizeDisplay.w = 0 ;
	m_sizeDisplay.h = 0 ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_hMainIcon = NULL ;
	m_fShowCursor = false ;
	//
	m_refParentWindow.SetReference( NULL, m_pContext ) ;
	//
	ECSSprite::DeleteImage( ) ;
	//
	if ( m_pCallbackContext != NULL )
	{
		m_pCallbackContext->ReleaseContext( false ) ;
		delete	m_pCallbackContext ;
		m_pCallbackContext = NULL ;
	}
	if ( m_dwTlsIndex != 0xFFFFFFFF )
	{
		::TlsFree( m_dwTlsIndex ) ;
		m_dwTlsIndex = 0xFFFFFFFF ;
	}
	THREAD_LOCAL_DATA *	ptldNext = m_ptldListFirst ;
	while ( ptldNext != NULL )
	{
		THREAD_LOCAL_DATA *	ptldTemp = ptldNext->ptldNext ;
		delete	ptldNext ;
		ptldNext = ptldTemp ;
	}
	m_ptldListFirst = NULL ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_CloseWindow( void * pInstance )
{
	CLOSE_WINDOW *	pcw = (CLOSE_WINDOW*) pInstance ;
	pcw->pWnd->m_pwndDisplay->CloseDisplay() ;
	pcw->eventDone.SetEvent() ;
	return	0 ;
}

// m_pwndDisplay, m_pInterface を作成して初期設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::PrepareDisplayWindow( bool fOnlyInterface )
{
	if ( !fOnlyInterface )
	{
		ESLAssert( m_pwndDisplay == NULL ) ;
		m_pwndDisplay = OnCreateWindowObject( ) ;
		if ( m_pwndDisplay == NULL )
		{
			return	eslErrFailed ;
		}
	}
	ESLAssert( m_pInterface == NULL ) ;
	m_pInterface = OnCreateInterface( ) ;
	if ( m_pInterface == NULL )
	{
		return	eslErrFailed ;
	}
	EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
	m_pInterface->AttachDrawImageObject( pDrawImage ) ;
	//
	if ( !fOnlyInterface )
	{
		m_pwndDisplay->Attach3DViewDisplay( m_pivView3D ) ;
		m_pwndDisplay->SetInterface( m_pInterface ) ;
		if ( m_fInitChangeModeFlag )
		{
			m_pwndDisplay->SetChangeDisplayModeFlag( m_nInitChangeModeFlag ) ;
			m_fInitChangeModeFlag = false ;
		}
		else
		{
			if ( m_pEnv != NULL )
			{
				m_pwndDisplay->SetChangeDisplayModeFlag( !m_pEnv->m_fNoChangeMode ) ;
			}
		}
		m_pwndDisplay->SetOptionalFuncFlag( m_fOptionFunctions ) ;
	}
	if ( CreateImage
		( EIF_RGBA_BITMAP,
			m_sizeDisplay.w, m_sizeDisplay.h, 32, EGL_IMAGE_HAS_DC ) == NULL )
	{
		return	eslErrFailed ;
	}
	if ( m_pivView3D != NULL )
	{
		using namespace E3DSDisplayPlugin ;
		m_pivView3D->SetBufferSize
			( 0, formatRGB,
				m_sizeDisplay.w, m_sizeDisplay.h, stereoBufferCount ) ;
	}
	ReverseVertically( ) ;
	m_pInterface->m_pWnd = this ;
	m_pInterface->AttachView( this ) ;
	//
	if ( m_pInputFilter != NULL )
	{
		if ( (m_pInputFilter->m_dwFilterMode & 0x01) && !fOnlyInterface )
		{
			m_pInputFilter->OpenFilter( m_pwndDisplay ) ;
		}
		else
		{
			m_pInputFilter->OpenFilter( m_pInterface ) ;
		}
		m_pInputFilter = NULL ;
	}
	//
	return	eslErrSuccess ;
}

// EGameWindow を作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CreateDisplayWindow( HWND hwndParent )
{
	if ( m_pwndDisplay == NULL )
	{
		return	eslErrFailed ;
	}
	m_pwndDisplay->SetInterface( m_pInterface ) ;
	//
	POINT *	pInitPos = NULL ;
	SIZE *	pInitSize = NULL ;
	if ( (m_posInitWindow.x != 0x80000000)
		&& (m_posInitWindow.y != 0x80000000) )
	{
		pInitPos = &m_posInitWindow ;
	}
	if ( (m_sizeInitWindow.cx != 0x80000000)
		&& (m_sizeInitWindow.cy != 0x80000000) )
	{
		pInitSize = &m_sizeInitWindow ;
	}
	if ( m_pwndDisplay->CreateDisplayEx
		( m_strWindowName, m_fCooperationLevel,
			m_sizeDisplay.w, m_sizeDisplay.h,
			m_nBitsPerPixel, m_nFrequency,
			pInitPos, pInitSize, hwndParent ) )
	{
		return	eslErrFailed ;
	}
	if ( m_hMainIcon != NULL )
	{
		m_pwndDisplay->SetIcon( m_hMainIcon ) ;
	}
	m_statusCreated = statusCreateDisplay ;
	UpdateImagePosition( ) ;
	//
	return	eslErrSuccess ;
}

// ウィンドウサイズ変更
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::ChangeDisplaySize
	( unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency )
{
	ESLError	errResult = eslErrGeneral ;
	if ( m_pwndDisplay != NULL )
	{
		if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
										!= ::GetCurrentThreadId() )
		{
			CHANGE_DISPLAY_SIZE	cds ;
			cds.pWnd = this ;
			cds.eventDone.CreateEvent() ;
			cds.nWidth = nWidth ;
			cds.nHeight = nHeight ;
			cds.nBitsPerPixel = nBitsPerPixel ;
			cds.nFrequency = nFrequency ;
			//
			LRESULT	lrResult ;
			if ( !ProcedureOnWindowThread
				( CallOnWinThread_ChangeDisplaySize, &cds, &lrResult, true ) )
			{
				cds.eventDone.Wait( INFINITE ) ;
			}
			return	cds.errResult ;
		}
		if ( m_pivView3D != NULL )
		{
			using namespace E3DSDisplayPlugin ;
			m_pivView3D->SetBufferSize
				( 0, formatRGB,
					nWidth, nHeight, stereoBufferCount ) ;
		}
		errResult =
			m_pwndDisplay->ChangeDisplaySize
				( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
		//
		EGLSize	sizeImageView = GetSize() ;
		if ( (sizeImageView.w != (int) nWidth)
					|| (sizeImageView.h != (int) nHeight) )
		{
			m_pInterface->Lock() ;
			CreateImage
				( EIF_RGBA_BITMAP,
					nWidth, nHeight, 32, EGL_IMAGE_HAS_DC ) ;
			ReverseVertically( ) ;
			m_pInterface->AttachView( this ) ;
			m_pInterface->Unlock() ;
		}
		m_sizeDisplay.w = nWidth ;
		m_sizeDisplay.h = nHeight ;
		m_nBitsPerPixel = nBitsPerPixel ;
		m_nFrequency = nFrequency ;
		//
		UpdateImagePosition( ) ;
	}
	return	errResult ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_ChangeDisplaySize( void * pInstance )
{
	CHANGE_DISPLAY_SIZE *	pcds = (CHANGE_DISPLAY_SIZE*) pInstance ;
	pcds->errResult =
		pcds->pWnd->ChangeDisplaySize
			( pcds->nWidth, pcds->nHeight,
				pcds->nBitsPerPixel, pcds->nFrequency ) ;
	pcds->eventDone.SetEvent() ;
	return	0 ;
}

ESLError ECSWindow::ChangeWindowSize
	( unsigned int nWidth, unsigned int nHeight )
{
	ESLError	errResult = eslErrGeneral ;
	if ( m_pwndDisplay != NULL )
	{
		m_pwndDisplay->ChangeWindowClientSize( nWidth, nHeight ) ;
		errResult = eslErrSuccess ;
	}
	return	errResult ;
}

// 協調レベルを変更
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::ChangeCooperationLevel
		( EGameWindow::CooperationLevel fCooperationLevel )
{
	ESLError	errResult = eslErrGeneral ;
	if ( (m_pwndDisplay != NULL)
		&& (m_pwndDisplay->GetCooperationLevel() != fCooperationLevel) )
	{
		if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
										!= ::GetCurrentThreadId() )
		{
			CHANGE_COOPERATION_LEVEL	ccl ;
			ccl.pWnd = this ;
			ccl.eventDone.CreateEvent() ;
			ccl.fCooperationLevel = fCooperationLevel ;
			//
			LRESULT	lrResult ;
			if ( !ProcedureOnWindowThread
				( CallOnWinThread_ChangeCooperationLevel,
										&ccl, &lrResult, true ) )
			{
				ccl.eventDone.Wait( INFINITE ) ;
			}
			return	ccl.errResult ;
		}
		errResult =
			m_pwndDisplay->ChangeCooperationLevel( fCooperationLevel ) ;
		if ( !errResult )
		{
			m_fCooperationLevel = fCooperationLevel ;
		}
		UpdateImagePosition( ) ;
	}
	return	errResult ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_ChangeCooperationLevel( void * pInstance )
{
	CHANGE_COOPERATION_LEVEL *	pccl = (CHANGE_COOPERATION_LEVEL*) pInstance ;
	pccl->errResult =
		pccl->pWnd->ChangeCooperationLevel( pccl->fCooperationLevel ) ;
	pccl->eventDone.SetEvent() ;
	return	0 ;
}

// オプショナル機能フラグを設定
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::SetOptionalFuncFlag( unsigned int flagsOptionalFunc )
{
	ESLError	errResult = eslErrGeneral ;
	m_fOptionFunctions = flagsOptionalFunc ;
	if ( m_pwndDisplay != NULL )
	{
		if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
										!= ::GetCurrentThreadId() )
		{
			SET_OPTIONAL_FUNC_FLAG	soff ;
			soff.pWnd = this ;
			soff.eventDone.CreateEvent() ;
			soff.flags = flagsOptionalFunc ;
			//
			LRESULT	lrResult ;
			if ( !ProcedureOnWindowThread
				( CallOnWinThread_SetOptionalFuncFlag,
										&soff, &lrResult, true ) )
			{
				soff.eventDone.Wait( INFINITE ) ;
			}
			return ;
		}
		m_pwndDisplay->SetOptionalFuncFlag( flagsOptionalFunc ) ;
		UpdateImagePosition( ) ;
	}
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_SetOptionalFuncFlag( void * pInstance )
{
	SET_OPTIONAL_FUNC_FLAG *
		psoff = (SET_OPTIONAL_FUNC_FLAG*) pInstance ;
	psoff->pWnd->SetOptionalFuncFlag( psoff->flags ) ;
	psoff->eventDone.SetEvent() ;
	return	0 ;
}

// 画面モード変更フラグを設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::SetChangeDisplayModeFlag( unsigned int nWithChangeMode )
{
	if ( m_pwndDisplay == NULL )
	{
		m_fInitChangeModeFlag = true ;
		m_nInitChangeModeFlag = nWithChangeMode ;
		return	eslErrSuccess ;
	}
	if ( m_pwndDisplay->GetChangeDisplayModeFlag()
				!= (unsigned int) (nWithChangeMode != 0) )
	{
		EGameWindow::CooperationLevel
			fCooperationLevel = m_pwndDisplay->GetCooperationLevel( ) ;
		if ( fCooperationLevel & EGameWindow::flagFullScreen )
		{
			ChangeCooperationLevel( EGameWindow::levelWindow ) ;
			m_pwndDisplay->SetChangeDisplayModeFlag( nWithChangeMode ) ;
			ChangeCooperationLevel( fCooperationLevel ) ;
			UpdateImagePosition( ) ;
		}
		else
		{
			m_pwndDisplay->SetChangeDisplayModeFlag( nWithChangeMode ) ;
		}
	}
	return	eslErrSuccess ;
}

// 画面外フレーム画像を設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::SetExteriorBackgroundFrame
	( DWORD dwFlags, DWORD rgbColor, ECSResource * pTile,
		ECSResource * pLeft, ECSResource * pRight,
		ECSResource * pUpper, ECSResource * pUnder )
{
	m_dwBGFrameFlags = dwFlags ;
	m_rgbBGFrameColor = rgbColor ;
	m_refFrameTile.SetReference( pTile ) ;
	m_refFrameLeft.SetReference( pLeft ) ;
	m_refFrameRight.SetReference( pRight ) ;
	m_refFrameUpper.SetReference( pUpper ) ;
	m_refFrameUnder.SetReference( pUnder ) ;
	//
	EWindowSpriteInterface::BACKGROUND_IMAGE	bgiFrame ;
	::eslFillMemory( &bgiFrame, 0, sizeof(bgiFrame) ) ;
	bgiFrame.dwFlags = dwFlags ;
	bgiFrame.rgbBG.dwPixelCode = rgbColor ;
	//
	if ( pTile != NULL )
	{
		bgiFrame.pTile = pTile->GetImageInfo() ;
	}
	if ( pLeft != NULL )
	{
		bgiFrame.pLeft = pLeft->GetImageInfo() ;
	}
	if ( pRight != NULL )
	{
		bgiFrame.pRight = pRight->GetImageInfo() ;
	}
	if ( pUpper != NULL )
	{
		bgiFrame.pUpper = pUpper->GetImageInfo() ;
	}
	if ( pUnder != NULL )
	{
		bgiFrame.pUnder = pUnder->GetImageInfo() ;
	}
	if ( m_pInterface != NULL )
	{
		m_pInterface->Lock() ;
		m_pInterface->SetBackgroundFrameImage( bgiFrame ) ;
		m_pInterface->UpdateRect( NULL ) ;
		m_pInterface->Unlock() ;
	}
	return	eslErrSuccess ;
}

// ステレオ立体視表示インターフェースを設定する
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::Set3DViewDisplay
		( E3DSDisplayPlugin::I3DImageView * pivView3D )
{
	if ( (m_pivView3D == NULL) && (pivView3D == NULL) )
	{
		return ;
	}
	//
	// 現在の画面モードが立体視インターフェースによるものだった場合、
	// 通常モードに復帰してから、既存の立体視インターフェースを削除する
	//
	EGameWindow::CooperationLevel fCooperationLevel = m_fCooperationLevel ;
	bool	fReChangeCooperationLevel = false ;
	if ( m_pwndDisplay != NULL )
	{
		fCooperationLevel = m_pwndDisplay->GetCooperationLevel() ;
		if ( fCooperationLevel & EGameWindow::flagFullScreen )
		{
			fReChangeCooperationLevel = true ;
			//
			ChangeCooperationLevel( EGameWindow::levelWindow ) ;
		}
		m_pwndDisplay->Attach3DViewDisplay( NULL ) ;
	}
	if ( m_pivView3D != NULL )
	{
		Lock() ;
		if ( m_pInterface != NULL )
		{
			m_pInterface->Attach3DViewDisplay( NULL ) ;
		}
		m_pivView3D->Release() ;
		m_pivView3D = NULL ;
		Unlock() ;
	}
	//
	// 立体視表示インターフェースを設定する
	//
	ESLAssert( m_pivView3D == NULL ) ;
	Lock() ;
	m_pivView3D = pivView3D ;
	//
	if ( m_pwndDisplay != NULL )
	{
		m_pwndDisplay->Attach3DViewDisplay( m_pivView3D ) ;
	}
	if ( m_pInterface != NULL )
	{
		m_pInterface->Attach3DViewDisplay( m_pivView3D ) ;
		//
		if ( m_pivView3D != NULL )
		{
			using namespace E3DSDisplayPlugin ;
			EGLSize	sizeImage = m_pInterface->GetSize() ;
			m_pivView3D->SetBufferSize
				( 0, formatRGB,
					sizeImage.w, sizeImage.h, stereoBufferCount ) ;
		}
	}
	Unlock() ;
	//
	// 画面モードを一旦復帰していた場合、再度画面モードを変更する
	//
	if ( fReChangeCooperationLevel )
	{
		ChangeCooperationLevel( fCooperationLevel ) ;
	}
	Lock() ;
	if ( m_pivView3D != NULL )
	{
		UpdateImagePosition( ) ;
	}
	Unlock() ;
}

// カーソル表示設定
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::ShowCursor( bool fShow )
{
	m_fShowCursor = fShow ;
}

// 初期座標設定
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::InitWindowPosition( int xPos, int yPos, int nWidth, int nHeight )
{
	m_posInitWindow.x = xPos ;
	m_posInitWindow.y = yPos ;
	m_sizeInitWindow.cx = nWidth ;
	m_sizeInitWindow.cy = nHeight ;
}

// ウィンドウの通常時座標取得
//////////////////////////////////////////////////////////////////////////////
bool ECSWindow::GetNormalWindowPosition( EGL_POINT & ptWindow, EGL_SIZE & sizeWindow ) const
{
	RECT	rctNormalPos ;
	if ( m_pwndDisplay != NULL )
	{
		if ( m_pwndDisplay->GetNormalWindowPos( rctNormalPos ) )
		{
			ptWindow.x = rctNormalPos.left ;
			ptWindow.y = rctNormalPos.top ;
			sizeWindow.w = rctNormalPos.right - rctNormalPos.left ;
			sizeWindow.h = rctNormalPos.bottom - rctNormalPos.top ;
			return	true ;
		}
	}
	return	false ;
}

// 実行コンテキスト関連付け
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::AttachContext( ECSContext * pContext )
{
	m_pContext = pContext ;
	m_pEnv = NULL ;
	if ( pContext != NULL )
	{
		m_pEnv = pContext->GetEnvironment( ) ;
	}
}

// 環境設定
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::AttachEnvironment( ECSEnvironment * pEnv )
{
	m_pEnv = pEnv ;
}

// ウィンドウを取得する
//////////////////////////////////////////////////////////////////////////////
EGameWindow * ECSWindow::GetWindow( void ) const
{
	return	m_pwndDisplay ;
}

// ウィンドウインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EInterface * ECSWindow::GetInterface( void ) const
{
	return	m_pInterface ;
}

// ウィンドウメッセージを処理する
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::HandleWindowMessage( int nCount, DWORD dwTimeout ) const
{
	if ( m_pInterface != NULL )
	{
		m_pInterface->HandleWindowMessage( nCount, dwTimeout ) ;
	}
}

// ウィンドウスレッドから関数を呼び出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::ProcedureOnWindowThread
	( PFUNC_PROCEDURE pfnProc, void * pInstance,
					LRESULT * pResult, bool fAsync ) const
{
	if ( (m_pwndDisplay != NULL) && (pfnProc != NULL) )
	{
		LRESULT	lrResult = 0 ;
		if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
										== ::GetCurrentThreadId() )
		{
			lrResult = pfnProc( pInstance ) ;
			if ( pResult != NULL )
			{
				*pResult = lrResult ;
			}
		}
		else
		{
			ESLAssert( m_pInterface != NULL ) ;
			ESLError	err =
				m_pInterface->ProcedureOnWindowThread
							( pfnProc, pInstance, pResult, fAsync ) ;
			if ( err )
			{
				return	err ;
			}
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// コールバック関数実行用コンテキストを取得する
//////////////////////////////////////////////////////////////////////////////
ECSContext * ECSWindow::GetCallbackContext( void )
{
	ESLAssert( m_pContext != NULL ) ;
	if ( m_pCallbackContext == NULL )
	{
		m_pCallbackContext = new ECSContext ;
		m_pCallbackContext->InitializeContext( m_pContext->m_pcsxi, false ) ;
	}
	return	m_pCallbackContext ;
}

// スレッド同期
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Lock( DWORD dwTimeout )
{
	if ( m_pInterface != NULL )
	{
		ESLError	err = m_pInterface->Lock( dwTimeout ) ;
		if ( err == eslErrSuccess )
		{
			if ( m_dwTlsIndex != 0xFFFFFFFF )
			{
				THREAD_LOCAL_DATA *	ptld =
					(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
				if ( ptld == NULL )
				{
					ptld = new THREAD_LOCAL_DATA ;
					ULONG_PTR	pOldList =
						::InterlockedExchange
							( (LPLONG) &m_ptldListFirst, (ULONG_PTR) ptld ) ;
					ptld->ptldNext = (THREAD_LOCAL_DATA*) pOldList ;
					::TlsSetValue( m_dwTlsIndex, ptld ) ;
				}
				ptld->dwLockedCount += 1 ;
			}
		}
		return	err ;
	}
	return	eslErrSuccess ;
}

ESLError ECSWindow::Unlock( void )
{
	if ( m_pInterface != NULL )
	{
		if ( m_dwTlsIndex != 0xFFFFFFFF )
		{
			THREAD_LOCAL_DATA *	ptld =
				(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
			if ( ptld != NULL )
			{
				ptld->dwLockedCount -= (int) (ptld->dwLockedCount > 0) ;
			}
		}
		return	m_pInterface->Unlock() ;
	}
	return	eslErrGeneral ;
}

// 描画更新制御
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::FreezePaint( void )
{
	if ( m_pInterface != NULL )
	{
		m_pInterface->FreezePaint() ;
	}
}

void ECSWindow::UnfreezePaint( void )
{
		m_pInterface->UnfreezePaint() ;
}

// スレッド同期（スクリプト用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::QuickLockOnScript( void )
{
	if ( m_dwTlsIndex != 0xFFFFFFFF )
	{
		THREAD_LOCAL_DATA *	ptld =
			(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
		if ( ptld && (ptld->dwLockedCount > 0) )
		{
			ptld->dwQuickLocked ++ ;
			return	eslErrSuccess ;
		}
	}
	return	Lock( ) ;
}

ESLError ECSWindow::QuickUnlockOnScript( void )
{
	if ( m_dwTlsIndex != 0xFFFFFFFF )
	{
		THREAD_LOCAL_DATA *	ptld =
			(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
		if ( ptld && (ptld->dwQuickLocked > 0) )
		{
			ptld->dwQuickLocked -- ;
			return	eslErrSuccess ;
		}
	}
	return	Unlock( ) ;
}

bool ECSWindow::IsQuickLockedOnScript( void ) const
{
	if ( m_dwTlsIndex != 0xFFFFFFFF )
	{
		THREAD_LOCAL_DATA *	ptld =
			(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
		if ( ptld != NULL )
		{
			return	(ptld->dwLockedCount > 0) || (ptld->dwQuickLocked > 0) ;
		}
	}
	return	false ;
}

// スレッドローカルデータの解放
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::FreeThreadLocalData( void )
{
	THREAD_LOCAL_DATA *	ptldTarget =
		(THREAD_LOCAL_DATA*) ::TlsGetValue( m_dwTlsIndex ) ;
	if ( ptldTarget != NULL )
	{
		ECotophaScript::Lock( ) ;
		//
		THREAD_LOCAL_DATA *	ptldPrev = NULL ;
		THREAD_LOCAL_DATA *	ptldNext = m_ptldListFirst ;
		while ( ptldNext != NULL )
		{
			if ( ptldNext == ptldTarget )
			{
				if ( ptldPrev != NULL )
				{
					ptldPrev->ptldNext = ptldNext->ptldNext ;
				}
				else
				{
					m_ptldListFirst = ptldNext->ptldNext ;
				}
				::TlsSetValue( m_dwTlsIndex, NULL ) ;
				delete	ptldTarget ;
				break ;
			}
			ptldPrev = ptldNext ;
			ptldNext = ptldNext->ptldNext ;
		}
		//
		ECotophaScript::Unlock( ) ;
	}
}

void ECSWindow::FreeAllThreadLocalData( void )
{
	ECotophaScript::Lock( ) ;
	//
	ECSWindow *	pwndNext = ECSSprite::m_pMainWnd ;
	while ( pwndNext != NULL )
	{
		pwndNext->FreeThreadLocalData( ) ;
		pwndNext = pwndNext->m_pwndNextList ;
	}
	//
	ECotophaScript::Unlock( ) ;
}

// ウィンドウリストにエントリ追加
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::AddToWidowList( void )
{
	ECotophaScript::Lock( ) ;
	//
	RemoveFromWindowList( ) ;
	//
	ESLAssert( m_pwndPrevList == NULL ) ;
	ESLAssert( m_pwndNextList == NULL ) ;
	//
	if ( ECSSprite::m_pMainWnd == NULL )
	{
		ESLAssert( m_nTotalWindowCount == 0 ) ;
		m_nTotalWindowCount ++ ;
		ECSSprite::m_pMainWnd = this ;
	}
	else
	{
		m_nTotalWindowCount ++ ;
		//
		ECSWindow *	pwndNext = ECSSprite::m_pMainWnd ;
		ESLAssert( pwndNext != NULL ) ;
		while ( pwndNext->m_pwndNextList != NULL )
		{
			pwndNext = pwndNext->m_pwndNextList ;
		}
		ESLAssert( pwndNext->m_pwndNextList == NULL ) ;
		pwndNext->m_pwndNextList = this ;
		m_pwndPrevList = pwndNext ;
	}
	//
	ECotophaScript::Unlock( ) ;
}

// ウィンドウリストからエントリ削除
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::RemoveFromWindowList( void )
{
	ECotophaScript::Lock( ) ;
	//
	ECSWindow *	pwndLast = NULL ;
	ECSWindow *	pwndNext = ECSSprite::m_pMainWnd ;
	while ( pwndNext != NULL )
	{
		if ( pwndNext == this )
		{
			break ;
		}
		pwndLast = pwndNext ;
		pwndNext = pwndNext->m_pwndNextList ;
	}
	if ( pwndNext != NULL )
	{
		ESLAssert( pwndNext == this ) ;
		if ( m_pwndPrevList != NULL )
		{
			m_pwndPrevList->m_pwndNextList = m_pwndNextList ;
		}
		else
		{
			ECSSprite::m_pMainWnd = m_pwndNextList ;
		}
		if ( m_pwndNextList != NULL )
		{
			m_pwndNextList->m_pwndPrevList = m_pwndPrevList ;
		}
		m_pwndPrevList = NULL ;
		m_pwndNextList = NULL ;
		//
		ESLAssert( m_nTotalWindowCount > 0 ) ;
		m_nTotalWindowCount -- ;
	}
	//
	ESLAssert( m_pwndPrevList == NULL ) ;
	ESLAssert( m_pwndNextList == NULL ) ;
	//
	ECotophaScript::Unlock( ) ;
}

// メッセージボックス表示
//////////////////////////////////////////////////////////////////////////////
int ECSWindow::MessageBox
	( const char * pszMessage,
		const char * pszCaption, int nStyle )
{
	static const int	s_nMBStyle[] =
	{
		MB_OK | MB_ICONINFORMATION,
		MB_OKCANCEL | MB_ICONSTOP,
		MB_YESNO | MB_ICONQUESTION,
		MB_YESNOCANCEL | MB_ICONQUESTION,
		MB_RETRYCANCEL | MB_ICONHAND,
	} ;
	if ( (nStyle < 0) || (nStyle >= msgboxStyleCount) )
	{
		nStyle = msgboxStyleOk ;
	}
	int	nMBResult = IDOK ;
	if ( m_pwndDisplay == NULL )
	{
		nMBResult =
			::MessageBox
				( NULL, pszMessage, pszCaption, s_nMBStyle[nStyle] ) ;
	}
	else if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
										== ::GetCurrentThreadId() )
	{
		nMBResult =
			m_pwndDisplay->MessageBox
				( pszMessage, pszCaption, s_nMBStyle[nStyle] ) ;
	}
	else
	{
		if ( m_pInterface != NULL )
		{
			MESSAGE_BOX	mbx ;
			LRESULT		lResult ;
			mbx.pWnd = this ;
			mbx.strCaption = pszCaption ;
			mbx.strMessage = pszMessage ;
			mbx.nMBStyle = s_nMBStyle[nStyle] ;
			mbx.eventDone.CreateEvent() ;
			//
			if ( !m_pInterface->ProcedureOnWindowThread
				( CallOnWinThread_MessageBox, &mbx, &lResult, true ) )
			{
				mbx.eventDone.Wait( INFINITE ) ;
				nMBResult = mbx.nMBResult ;
			}
		}
	}
	static const int	s_nMBResult[] =
	{
		IDOK, IDCANCEL, IDYES, IDNO, IDRETRY,
	} ;
	static const int	s_nResultCode[] =
	{
		msgboxResultOk, msgboxResultCancel,
		msgboxResultYes, msgboxResultNo, msgboxResultRetry,
	} ;
	int	nResult = msgboxResultOk ;
	for ( int i = 0; i < sizeof(s_nMBResult)/sizeof(s_nMBResult[0]); i ++ )
	{
		if ( s_nMBResult[i] == nMBResult )
		{
			nResult = s_nResultCode[i] ;
			break ;
		}
	}
	return	nResult ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_MessageBox( void * pInstance )
{
	MESSAGE_BOX *	pmbx = (MESSAGE_BOX*) pInstance ;
	pmbx->nMBResult =
		pmbx->pWnd->m_pwndDisplay->MessageBox
			( pmbx->strMessage, pmbx->strCaption, pmbx->nMBStyle ) ;
	pmbx->eventDone.SetEvent() ;
	return	0 ;
}

// レイヤードウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::SetLayeredWindow( bool fLayeredWindow )
{
	if ( (m_pwndDisplay == NULL) || (m_pInterface == NULL) )
	{
		return	eslErrFailed ;
	}
	if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
									!= ::GetCurrentThreadId() )
	{
		SET_LAYERED_WINDOW	slw ;
		LRESULT				lResult ;
		slw.pWnd = this ;
		slw.eventDone.CreateEvent() ;
		slw.fLayered = fLayeredWindow ;
		//
		if ( !m_pInterface->ProcedureOnWindowThread
			( CallOnWinThread_SetLayeredWindow, &slw, &lResult, true ) )
		{
			slw.eventDone.Wait( INFINITE ) ;
		}
	}
	else
	{
		m_pInterface->SetLayeredWindow( fLayeredWindow ) ;
	}
	return	eslErrSuccess ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_SetLayeredWindow( void * pInstance )
{
	SET_LAYERED_WINDOW *	pslw = (SET_LAYERED_WINDOW*) pInstance ;
	pslw->pWnd->m_pInterface->SetLayeredWindow( pslw->fLayered ) ;
	pslw->eventDone.SetEvent() ;
	return	0 ;
}

// ウィンドウ表示・非表示
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::ShowWindow( int nShowCmd )
{
	if ( m_pwndDisplay == NULL )
	{
		return	eslErrFailed ;
	}
	m_pwndDisplay->ShowWindow( nShowCmd ) ;
	return	eslErrSuccess ;
}

// レイアウト設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::SetWindowLayout( int nFlags, int xPos, int yPos )
{
	if ( (m_pwndDisplay == NULL) || (m_pInterface == NULL) )
	{
		return	eslErrFailed ;
	}
	if ( m_pwndDisplay->GetCooperationLevel() != EGameWindow::levelWindow )
	{
		return	eslErrFailed ;
	}
	if ( ::GetWindowThreadProcessId( *m_pwndDisplay, NULL )
									!= ::GetCurrentThreadId() )
	{
		SET_WINDOW_LAYOUT	swl ;
		LRESULT				lResult ;
		swl.pWnd = this ;
		swl.eventDone.CreateEvent() ;
		swl.nFlags = nFlags ;
		swl.xPos = xPos ;
		swl.yPos = yPos ;
		//
		if ( !m_pInterface->ProcedureOnWindowThread
			( CallOnWinThread_SetWindowLayout, &swl, &lResult, true ) )
		{
			swl.eventDone.Wait( INFINITE ) ;
		}
	}
	else
	{
		m_pInterface->SetWindowLayout( nFlags, xPos, yPos ) ;
	}
	return	eslErrSuccess ;
}

LRESULT __stdcall
	ECSWindow::CallOnWinThread_SetWindowLayout( void * pInstance )
{
	SET_WINDOW_LAYOUT *	pswl = (SET_WINDOW_LAYOUT*) pInstance ;
	pswl->pWnd->m_pInterface->SetWindowLayout
			( pswl->nFlags, pswl->xPos, pswl->yPos ) ;
	pswl->eventDone.SetEvent() ;
	return	0 ;
}

// ウィンドウオブジェクトを作成する
//////////////////////////////////////////////////////////////////////////////
EGameWindow * ECSWindow::OnCreateWindowObject( void )
{
	return	new EGameWindow ;
}

// ウィンドウインターフェースオブジェクトを作成する
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EInterface * ECSWindow::OnCreateInterface( void )
{
	return	new EInterface ;
}

// ウィンドウ内部の画像表示位置情報を更新する
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::UpdateImagePosition( void )
{
	if ( m_pwndDisplay && m_pInterface )
	{
		if ( m_statusCreated == statusCreateDisplay )
		{
			const POINT &	ptBasePos = m_pwndDisplay->GetOffsetPos( ) ;
			const SIZE &	sizeScreen = m_pwndDisplay->GetScreenSize( ) ;
			EGLPoint	ptBase( ptBasePos.x, ptBasePos.y ) ;
			EGLSize		szScreen( sizeScreen.cx, sizeScreen.cy ) ;
			if ( (ptBase != EGLPoint( 0, 0 )) || (szScreen != m_sizeDisplay) )
			{
				m_pInterface->SetImageStretching( &ptBase, &szScreen ) ;
			}
			else
			{
				m_pInterface->SetImageStretching( NULL, NULL ) ;
			}
		}
		else if ( m_statusCreated == statusCreateWindow )
		{
			RECT	rectClient ;
			m_pwndDisplay->GetClientRect( &rectClient ) ;
			//
			POINT	ptOffset = { 0, 0 } ;
			SIZE	sizeScreen = { rectClient.right, rectClient.bottom } ;
			m_pwndDisplay->SetOffsetPos( ptOffset ) ;
			m_pwndDisplay->SetScreenSize( sizeScreen ) ;
			//
			m_pInterface->SetImageStretching( NULL, NULL ) ;
			//
			EGLSize	sizeImageView = GetSize() ;
			if ( (sizeImageView.w != sizeScreen.cx)
					|| (sizeImageView.h != sizeScreen.cy) )
			{
				CreateImage
					( EIF_RGBA_BITMAP,
						sizeScreen.cx, sizeScreen.cy, 32, EGL_IMAGE_HAS_DC ) ;
				ReverseVertically( ) ;
				m_pInterface->AttachView( this ) ;
			}
		}
	}
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSWindow::GetTypeName( void ) const
{
	return	L"Window" ;
}

ECSObject * ECSWindow::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"Window" ) )
	{
		return	this ;
	}
	return	ECSSprite::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSWindow::Duplicate( void )
{
	return	new ECSWindow ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		ESLError	err =
			ECSSprite::GetFunction( context, nIndex, pwszName ) ;
		if ( err )
		{
			return	err ;
		}
		nIndex += m_staFuncName->GetSize( ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( nIndex >= (int) m_staFuncName->GetSize() )
	{
		nIndex -= m_staFuncName->GetSize() ;
		return	ECSSprite::CallFunction( context, nIndex, lstArg ) ;
	}
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていない Window 型のメンバ関数を呼び出しています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSWindow::IndexAllMember( void )
{
	ECSSprite::IndexAllMember( ) ;
	m_refParentWindow.IndexAllMember() ;
	//
	m_refFrameTile.IndexAllMember() ;
	m_refFrameLeft.IndexAllMember() ;
	m_refFrameRight.IndexAllMember() ;
	m_refFrameUpper.IndexAllMember() ;
	m_refFrameUnder.IndexAllMember() ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::CommitAllReference( ECSContext & context )
{
	ESLError	err = ECSSprite::CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	m_refParentWindow.CommitAllReference( context ) ;
	//
	m_refFrameTile.CommitAllReference( context ) ;
	m_refFrameLeft.CommitAllReference( context ) ;
	m_refFrameRight.CommitAllReference( context ) ;
	m_refFrameUpper.CommitAllReference( context ) ;
	m_refFrameUnder.CommitAllReference( context ) ;
	//
	if ( (m_statusCreated != statusUncreated)
		&& (m_sizeDisplay.w != 0) && (m_sizeDisplay.h != 0) )
	{
		ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
		HICON	hIcon = NULL ;
		if ( pEnv != NULL )
		{
			hIcon = pEnv->m_hMainIcon ;
		}
		AttachContext( &context ) ;
		//
		if ( m_statusCreated == statusCreateDisplay )
		{
			CreateDisplay
				( EString(m_strWindowName),
					m_fCooperationLevel,
					m_sizeDisplay.w, m_sizeDisplay.h,
					m_nBitsPerPixel, m_nFrequency, hIcon ) ;
		}
		else if ( m_statusCreated == statusCreateWindow )
		{
			ECSWindow *	pParentWnd =
				ESLTypeCast<ECSWindow>( m_refParentWindow.m_pRef ) ;
			CreateDisplayWindow
				( EString(m_strWindowName),
					m_sizeDisplay.w, m_sizeDisplay.h, pParentWnd, hIcon ) ;
		}
	}
	if ( m_pInterface != NULL )
	{
		EWindowSpriteInterface::BACKGROUND_IMAGE
				bgiFrame = m_pInterface->GetBackgroundFrameImage() ;
		//
		bgiFrame.dwFlags = m_dwBGFrameFlags ;
		bgiFrame.rgbBG.dwPixelCode = m_rgbBGFrameColor ;
		//
		ECSResource *	prsTile = ESLTypeCast<ECSResource>( m_refFrameTile.m_pRef ) ;
		if ( prsTile != NULL )
		{
			bgiFrame.pTile = prsTile->GetImageInfo() ;
		}
		ECSResource *	prsLeft = ESLTypeCast<ECSResource>( m_refFrameLeft.m_pRef ) ;
		if ( prsLeft != NULL )
		{
			bgiFrame.pLeft = prsLeft->GetImageInfo() ;
		}
		ECSResource *	prsRight = ESLTypeCast<ECSResource>( m_refFrameRight.m_pRef ) ;
		if ( prsRight != NULL )
		{
			bgiFrame.pRight = prsRight->GetImageInfo() ;
		}
		ECSResource *	prsUpper = ESLTypeCast<ECSResource>( m_refFrameUpper.m_pRef ) ;
		if ( prsUpper != NULL )
		{
			bgiFrame.pUpper = prsUpper->GetImageInfo() ;
		}
		ECSResource *	prsUnder = ESLTypeCast<ECSResource>( m_refFrameUnder.m_pRef ) ;
		if ( prsUnder != NULL )
		{
			bgiFrame.pUnder = prsUnder->GetImageInfo() ;
		}
		m_pInterface->SetBackgroundFrameImage( bgiFrame ) ;
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refParentWindow.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_statusCreated, sizeof(m_statusCreated) ) ;
	//
	int		nStrLen = m_strWindowName.GetLength( ) ;
	file.Write( &nStrLen, sizeof(int) ) ;
	if ( nStrLen != 0 )
	{
		file.Write( m_strWindowName.CharPtr(), nStrLen ) ;
	}
	file.Write( &m_fCooperationLevel, sizeof(m_fCooperationLevel) ) ;
	file.Write( &m_sizeDisplay, sizeof(m_sizeDisplay) ) ;
	file.Write( &m_nBitsPerPixel, sizeof(m_nBitsPerPixel) ) ;
	file.Write( &m_nFrequency, sizeof(m_nFrequency) ) ;
	//
	int	fShowCursor = m_fShowCursor ;
	file.Write( &fShowCursor, sizeof(fShowCursor) ) ;
	//
	if ( m_pInterface != NULL )
	{
		const EWindowSpriteInterface::BACKGROUND_IMAGE &
					bgiFrame = m_pInterface->GetBackgroundFrameImage() ;
		m_dwBGFrameFlags = bgiFrame.dwFlags ;
		m_rgbBGFrameColor = bgiFrame.rgbBG.dwPixelCode ;
	}
	file.Write( &m_dwBGFrameFlags, sizeof(m_dwBGFrameFlags) ) ;
	file.Write( &m_rgbBGFrameColor, sizeof(m_rgbBGFrameColor) ) ;
	//
	m_refFrameTile.Save( file, context ) ;
	m_refFrameLeft.Save( file, context ) ;
	m_refFrameRight.Save( file, context ) ;
	m_refFrameUpper.Save( file, context ) ;
	m_refFrameUnder.Save( file, context ) ;
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refParentWindow.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_statusCreated, sizeof(m_statusCreated) ) ;
	//
	int		nStrLen ;
	file.Read( &nStrLen, sizeof(int) ) ;
	if ( nStrLen != 0 )
	{
		file.Read( m_strWindowName.GetBuffer( nStrLen ), nStrLen ) ;
		m_strWindowName.ReleaseBuffer( nStrLen ) ;
	}
	file.Read( &m_fCooperationLevel, sizeof(m_fCooperationLevel) ) ;
	file.Read( &m_sizeDisplay, sizeof(m_sizeDisplay) ) ;
	file.Read( &m_nBitsPerPixel, sizeof(m_nBitsPerPixel) ) ;
	file.Read( &m_nFrequency, sizeof(m_nFrequency) ) ;
	//
	int	fShowCursor ;
	file.Read( &fShowCursor, sizeof(fShowCursor) ) ;
	m_fShowCursor = (fShowCursor != 0) ;
	//
	file.Read( &m_dwBGFrameFlags, sizeof(m_dwBGFrameFlags) ) ;
	file.Read( &m_rgbBGFrameColor, sizeof(m_rgbBGFrameColor) ) ;
	//
	m_refFrameTile.Load( file, context ) ;
	m_refFrameLeft.Load( file, context ) ;
	m_refFrameRight.Load( file, context ) ;
	m_refFrameUpper.Load( file, context ) ;
	m_refFrameUnder.Load( file, context ) ;
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strInfo ;
	if ( m_pInterface != NULL )
	{
		strInfo = EString(m_sizeDisplay.w) + "x" + EString(m_sizeDisplay.h) ;
	}
	else
	{
		strInfo = "" ;
	}
	buf.Write( strInfo.CharPtr(), strInfo.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *			ECSWindow::m_staFuncName = NULL ;
const wchar_t *				ECSWindow::m_pwszFuncName[37] =
{
	L"CreateDisplay", L"CloseDisplay",
	L"GetOptionalFuncFlag", L"SetOptionalFuncFlag",
	L"ChangeCooperationLevel", L"ChangeDisplaySize",
	L"SetChangeDisplayModeFlag",
	L"SetStereoDisplayMode",
	L"IsSupportedStereoDisplayMode",
	L"GetDisplaySize", L"UpdateWindow",
	L"ProcessUserInput", L"IsWindowActive",
	L"InitWindowPosition", L"GetNormalWindowPosition",
	L"GetPhysicalMonitorSize",
	L"SetExteriorBackgroundFrame", L"MessageBox",
	L"CreateWindow", L"CloseWindow", L"ChangeWindowSize",
	L"SetLayeredWindow", L"SetWindowLayout",
	L"EnableCommandQueue", L"FlushCommandQueue",
	L"QueueCommand", L"GetCommand",
	L"CallMouseMove", L"Lock", L"Unlock", L"FreezePaint", L"UnfreezePaint",
	L"SyncTimePaint", L"AsyncTimePaint",
	L"ShowCursor", L"IsShowCursor",
	NULL
} ;
const ECSWindow::PFUNC_CALL	ECSWindow::m_pfnCallFunc[36] =
{
	// メンバ関数（GameWindow 系）
	&ECSWindow::Call_CreateDisplay,
	&ECSWindow::Call_CloseDisplay,
	&ECSWindow::Call_GetOptionalFuncFlag,
	&ECSWindow::Call_SetOptionalFuncFlag,
	&ECSWindow::Call_ChangeCooperationLevel,
	&ECSWindow::Call_ChangeDisplaySize,
	&ECSWindow::Call_SetChangeDisplayModeFlag,
	&ECSWindow::Call_SetStereoDisplayMode,
	&ECSWindow::Call_IsSupportedStereoDisplayMode,
	&ECSWindow::Call_GetDisplaySize,
	&ECSWindow::Call_UpdateWindow,
	&ECSWindow::Call_ProcessUserInput,
	&ECSWindow::Call_IsWindowActive,
	&ECSWindow::Call_InitWindowPosition,
	&ECSWindow::Call_GetNormalWindowPosition,
	&ECSWindow::Call_GetPhysicalMonitorSize,
	&ECSWindow::Call_SetExteriorBackgroundFrame,
	// メンバ関数（汎用ウィンドウ）
	&ECSWindow::Call_MessageBox,
	&ECSWindow::Call_CreateWindow,
	&ECSWindow::Call_CloseDisplay,
	&ECSWindow::Call_ChangeWindowSize,
	&ECSWindow::Call_SetLayeredWindow,
	&ECSWindow::Call_SetWindowLayout,
	// メンバ関数（WindowSpriteInterface系）
	&ECSWindow::Call_EnableCommandQueue,
	&ECSWindow::Call_FlushCommandQueue,
	&ECSWindow::Call_QueueCommand,
	&ECSWindow::Call_GetCommand,
	&ECSWindow::Call_CallMouseMove,
	&ECSWindow::Call_Lock,
	&ECSWindow::Call_Unlock,
	&ECSWindow::Call_FreezePaint,
	&ECSWindow::Call_UnfreezePaint,
	&ECSWindow::Call_SyncTimePaint,
	&ECSWindow::Call_AsyncTimePaint,
	&ECSWindow::Call_ShowCursor,
	&ECSWindow::Call_IsShowCursor,
} ;

// メンバ関数 : CreateDisplay
//  ( [String strWindowName, Integer fCopperationLevel,
//		Integer nWidth, Integer nHeight,
//		Integer nBitsPerPixel := 0, Integer nFrequency := 0] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_CreateDisplay
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 7 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrWindowName ;
	int		fCooperationLevel = EGameWindow::levelWindow,
			nWidth = 640, nHeight = 480, nBitsPerPixel = 0, nFrequency = 0 ;
	HICON	hIcon = NULL ;
	AttachContext( &context ) ;
	//
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		wstrWindowName = pEnv->m_strCaption ;
		fCooperationLevel = pEnv->m_clCooperation ;
		nWidth = pEnv->m_sizeDisplay.w ;
		nHeight = pEnv->m_sizeDisplay.h ;
		nBitsPerPixel = pEnv->m_nDisplayDepth ;
		nFrequency = pEnv->m_nFrequency ;
		hIcon = pEnv->m_hMainIcon ;
	}
	err = context.GetArgumentAsStr
		( wstrWindowName, lstArg, 1, ECSWideString(wstrWindowName) ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
		( fCooperationLevel, lstArg, 2, fCooperationLevel ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 3, nWidth ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 4, nHeight ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nBitsPerPixel, lstArg, 5, nBitsPerPixel ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFrequency, lstArg, 6, nFrequency ) ;
	if ( err )
		return	err ;
	//
	err = CreateDisplay
		( EString(wstrWindowName),
			(EGameWindow::CooperationLevel) fCooperationLevel,
				nWidth, nHeight, nBitsPerPixel, nFrequency, hIcon ) ;
	AttachContext( &context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : CloseDisplay()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_CloseDisplay
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	CloseDisplay( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetOptionalFuncFlag()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_GetOptionalFuncFlag
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	int	nOptFlags = 0 ;
	if ( m_pwndDisplay != NULL )
	{
		nOptFlags = m_pwndDisplay->GetOptionalFuncFlag( ) ;
	}
	//
	return	context.PushObject( new ECSInteger( nOptFlags ) ) ;
}

// メンバ関数 : SetOptionalFuncFlag( Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetOptionalFuncFlag
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nFlags ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetOptionalFuncFlag( nFlags ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ChangeCooperationLevel( [Integer fCooperationLevel] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_ChangeCooperationLevel
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		fCooperationLevel = EGameWindow::levelWindow ;
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		fCooperationLevel = pEnv->m_clCooperation ;
	}
	err = context.GetArgumentAsInt
		( fCooperationLevel, lstArg, 1, fCooperationLevel ) ;
	if ( err )
		return	err ;
	//
	ChangeCooperationLevel
		( (EGameWindow::CooperationLevel) fCooperationLevel ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ChangeDisplaySize
//	( [Integer nWidth, Integer nHeight,
//		Integer nBitsPerPixel := 0, Integer nFrequency := 0] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_ChangeDisplaySize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 5 ) ;
	if ( err )
		return	err ;
	//
	int		nWidth = 640, nHeight = 480, nBitsPerPixel = 0, nFrequency = 0 ;
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		nWidth = pEnv->m_sizeDisplay.w ;
		nHeight = pEnv->m_sizeDisplay.h ;
		nBitsPerPixel = pEnv->m_nDisplayDepth ;
		nFrequency = pEnv->m_nFrequency ;
	}
	err = context.GetArgumentAsInt( nWidth, lstArg, 1, nWidth ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 2, nHeight ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nBitsPerPixel, lstArg, 3, nBitsPerPixel ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFrequency, lstArg, 4, nFrequency ) ;
	if ( err )
		return	err ;
	//
	ChangeDisplaySize
		( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	Integer SetChangeDisplayModeFlag( Integer fWithChangeMode := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetChangeDisplayModeFlag
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fWithChangeMode ;
	err = context.GetArgumentAsInt( fWithChangeMode, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	//
	err = SetChangeDisplayModeFlag( fWithChangeMode ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetStereoDisplayMode( String sViewID [, nSubParam] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetStereoDisplayMode
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrViewID ;
	err = context.GetArgumentAsStr( wstrViewID, lstArg, 1, L"" ) ;
	if ( err )
	{
		return	err ;
	}
//	QuickLock() ;
	if ( wstrViewID == L"AnaglyphView" )
	{
		int	nMode ;
		err = context.GetArgumentAsInt( nMode, lstArg, 2, 0 ) ;
		if ( err )
		{
			return	err ;
		}
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		Set3DViewDisplay
			( new E3DStereoDisplayAnaglyphView
				( pDrawImage,
					(E3DStereoDisplayAnaglyphView::Mode) nMode ) ) ;
		err = eslErrSuccess ;
	}
	else if ( wstrViewID == L"DDStereoscopic" )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		Set3DViewDisplay( new E3DStereoDisplayDDStereoscopic( pDrawImage ) ) ;
		err = eslErrSuccess ;
	}
	else if ( wstrViewID == L"OpenGLQuadBuffer" )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		Set3DViewDisplay( new E3DStereoDisplayOpenGL ) ;
		err = eslErrSuccess ;
	}
	else if ( wstrViewID == L"NVStereoBLT" )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		Set3DViewDisplay( new E3DStereoDisplayNVStereoBLT( pDrawImage ) ) ;
		err = eslErrSuccess ;
	}
	else if ( wstrViewID == L"InterleavedView" )
	{
		int	nMode ;
		err = context.GetArgumentAsInt( nMode, lstArg, 2, 0 ) ;
		if ( err )
		{
			return	err ;
		}
		EGLDrawImage *
			pDrawImage = ECotophaScript::GetDrawImage() ;
		Set3DViewDisplay( new E3DStereoDisplayInterleaved
				( pDrawImage, ((nMode & 0x01) != 0),
									((nMode & 0x02) != 0) ) ) ;
		err = eslErrSuccess ;
	}
	else
	{
		Set3DViewDisplay( NULL ) ;
		err = eslErrSuccess ;
	}
//	QuickUnlock() ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsSupportedStereoDisplayMode( String sViewID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_IsSupportedStereoDisplayMode
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nSupported = 0 ;
	ECSWideString	wstrViewID ;
	err = context.GetArgumentAsStr( wstrViewID, lstArg, 1, L"" ) ;
	if ( err )
	{
		return	err ;
	}
	if ( wstrViewID == L"AnaglyphView" )
	{
		nSupported = -1 ;
	}
	else if ( wstrViewID == L"DDStereoscopic" )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		if ( pDrawImage != NULL )
		{
			if ( pDrawImage->IsSupportedStereo3DGraphic() )
			{
				nSupported = -1 ;
			}
		}
	}
	else if ( wstrViewID == L"OpenGLQuadBuffer" )
	{
		nSupported = E3DStereoDisplayOpenGL::IsSupportedStereo() ? -1 : 0 ;
	}
	else if ( wstrViewID == L"NVStereoBLT" )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		if ( pDrawImage != NULL )
		{
			if ( pDrawImage->IsInstalledDirectX9() )
			{
				nSupported = -1 ;
			}
		}
	}
	else if ( wstrViewID == L"InterleavedView" )
	{
		nSupported = -1 ;
	}
	return	context.PushObject( new ECSInteger( nSupported ) ) ;
}

// メンバ関数 : Size GetDisplaySize()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_GetDisplaySize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pStruct = context.CreateUserStructure( L"Size" ) ;
	pStruct->SetMemberAsInt( L"w", m_sizeDisplay.w ) ;
	pStruct->SetMemberAsInt( L"h", m_sizeDisplay.h ) ;
	//
	return	context.PushObject( *pStruct ) ;
}

// メンバ関数 : UpdateWindow()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_UpdateWindow
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	if ( (m_pwndDisplay != NULL) && !IsQuickLockedOnScript() )
	{
		m_pwndDisplay->UpdateWindow( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ProcessUserInput( Integer nTimeout := 30 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_ProcessUserInput
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, 30 ) ;
	if ( err )
		return	err ;
	//
	HandleWindowMessage( 0x20, nTimeout ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsWindowActive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_IsWindowActive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSInteger *	pRetValue = new ECSInteger( 0 ) ;
	if ( m_pwndDisplay != NULL )
	{
		pRetValue->SetValue
			( m_pwndDisplay->IsWindowActive() ? -1 : 0 ) ;
	}
	return	context.PushObject( pRetValue ) ;
}

// メンバ関数 : InitWindowPosition
//		( Integer xPos := 80000000H, Integer yPos := 80000000H,
//			Integer nWidth := 80000000H, Integer nHeight := 80000000H )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_InitWindowPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 5 ) ;
	if ( err )
		return	err ;
	//
	int	xPos, yPos, nWidth, nHeight ;
	err = context.GetArgumentAsInt( xPos, lstArg, 1, 0x80000000 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( yPos, lstArg, 2, 0x80000000 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 3, 0x80000000 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 4, 0x80000000 ) ;
	if ( err )
		return	err ;
	//
	InitWindowPosition( xPos, yPos, nWidth, nHeight ) ;
	//
	return	context.PushObject( context.new_CSInteger( 0 ) ) ;
}

// メンバ関数 : Integer GetNormalWindowPosition( Point ptWindow [, Size sizeWindow] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_GetNormalWindowPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pWindowPos =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Point" ) ) ;
	if ( pWindowPos == NULL )
	{
		return	ESLErrorMsg
			( "GetNormalWindowPosition に Point 構造体が指定されていません" ) ;
	}
	ECSStructureInterface *	pWindowSize =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Size" ) ) ;
	int			fResult = 0 ;
	EGL_POINT	ptWindow ;
	EGL_SIZE	sizeWindow ;
	if ( GetNormalWindowPosition( ptWindow, sizeWindow ) )
	{
		pWindowPos->SetMemberAsInt( L"x", ptWindow.x ) ;
		pWindowPos->SetMemberAsInt( L"y", ptWindow.y ) ;
		if ( pWindowSize != NULL )
		{
			pWindowSize->SetMemberAsInt( L"w", sizeWindow.w ) ;
			pWindowSize->SetMemberAsInt( L"h", sizeWindow.h ) ;
		}
		fResult = -1 ;
	}
	return	context.PushObject( context.new_CSInteger( fResult ) ) ;
}

// メンバ関数 : Integer GetPhysicalMonitorSize( Size sizeMonitor )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_GetPhysicalMonitorSize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pMonitorSize =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Size" ) ) ;
	int	nResult = 0 ;
	if ( pMonitorSize != NULL )
	{
		pMonitorSize->SetMemberAsInt
			( L"w", ::GetSystemMetrics(SM_CXSCREEN) ) ;
		pMonitorSize->SetMemberAsInt
			( L"h", ::GetSystemMetrics(SM_CYSCREEN) ) ;
		//
		if ( m_pwndDisplay != NULL )
		{
			EString			strDisplayName ;
			EDisplayMode	devmode ;
			HMONITOR		hMonitor ;
			RECT			rectWindow ;
			m_pwndDisplay->GetWindowRect( &rectWindow ) ;
			if ( devmode.GetDisplayNameFromRect
				( strDisplayName, &rectWindow, &hMonitor ) != NULL )
			{
				MONITORINFO	mi ;
				mi.cbSize = sizeof(MONITORINFO) ;
				if ( !devmode.GetMonitorInfo( hMonitor, &mi ) )
				{
					pMonitorSize->SetMemberAsInt
						( L"w", mi.rcMonitor.right - mi.rcMonitor.left ) ;
					pMonitorSize->SetMemberAsInt
						( L"h", mi.rcMonitor.bottom - mi.rcMonitor.top ) ;
				}
			}
		}
	}
	return	context.PushObject( context.new_CSInteger( nResult ) ) ;
}

// メンバ関数 : Integer SetExteriorBackgroundFrame
//	( Integer nFlags, Integer rgbColor, Resource& rsTile,
//						Resource& rsLeft, Resource& rsRight,
//						Resource& rsUpper, Resource& rsUnder )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetExteriorBackgroundFrame
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 8 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nFlags, rgbColor ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( rgbColor, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSResource *	prsTile =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 3, L"Resource" ) ) ;
	ECSResource *	prsLeft =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 4, L"Resource" ) ) ;
	ECSResource *	prsRight =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 5, L"Resource" ) ) ;
	ECSResource *	prsUpper =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 6, L"Resource" ) ) ;
	ECSResource *	prsUnder =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 7, L"Resource" ) ) ;
	//
	err = SetExteriorBackgroundFrame
		( nFlags, rgbColor, prsTile, prsLeft, prsRight, prsUpper, prsUnder ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer MessageBox
//		( String sMessage,
//			[String sCaption, Integer nStyle := MsgBoxStyle::OK] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_MessageBox
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	EWideString			wstrDefCaption ;
	ECSEnvironment *	pEnv = context.GetEnvironment() ;
	if ( pEnv != NULL )
	{
		wstrDefCaption = pEnv->m_strCaption ;
	}
	ECSWideString	wstrMessage, wstrCaption ;
	int				nStyle ;
	//
	err = context.GetArgumentAsStr( wstrMessage, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrCaption, lstArg, 2, wstrDefCaption ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nStyle, lstArg, 3, msgboxStyleOk ) ;
	if ( err )
		return	err ;
	//
	int	nResult =
		MessageBox( EString(wstrMessage), EString(wstrCaption), nStyle ) ;
	//
	return	context.PushObject( context.new_CSInteger( nResult ) ) ;
}

// メンバ関数 : CreateWindow
//  ( String strWindowName,
//		Integer nWidth, Integer nHeight,
//		Window& refParentWnd := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_CreateWindow
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4, 5 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrWindowName ;
	int				nWidth = 640, nHeight = 480 ;
	HICON			hIcon = NULL ;
	ECSWindow *		pParentWnd = NULL ;
	//
	AttachContext( &context ) ;
	//
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		hIcon = pEnv->m_hMainIcon ;
	}
	err = context.GetArgumentAsStr
		( wstrWindowName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 2, nWidth ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 3, nHeight ) ;
	if ( err )
		return	err ;
	//
	pParentWnd =
		ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 4, L"Window" ) ) ;
	//
	err = CreateDisplayWindow
		( EString(wstrWindowName), nWidth, nHeight, pParentWnd, hIcon ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Error ChangeWindowSize( Integer nWidth, Integer nHeight )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_ChangeWindowSize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int	nWidth = 640, nHeight = 480 ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 1, nWidth ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 2, nHeight ) ;
	if ( err )
		return	err ;
	//
	err = ChangeWindowSize( nWidth, nHeight ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Error SetLayeredWindow( Boolean fLayered := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetLayeredWindow
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fLayered ;
	err = context.GetArgumentAsInt( fLayered, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	//
	err = SetLayeredWindow( fLayered != 0 ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Error SetWindowLayout
//					( Integer nFlags, Integer xPos := 0, Integer yPos := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SetWindowLayout
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags, xPos, yPos ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( xPos, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( yPos, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	err = SetWindowLayout( nFlags, xPos, yPos ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : EnableCommandQueue( Integer fQueueCommand := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_EnableCommandQueue
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		fQueueCommand ;
	err = context.GetArgumentAsInt( fQueueCommand, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pInterface != NULL )
	{
		m_pInterface->EnableCommandQueue( fQueueCommand != 0 ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : FlushCommandQueue
//		( Integer fQueueCommand := true,
//			Integer nPriority := 10 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_FlushCommandQueue
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		fQueueCommand, nPriority ;
	err = context.GetArgumentAsInt( fQueueCommand, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
			( nPriority, lstArg, 2, EWndSpriteCmd::priorityHighest ) ;
	if ( err )
		return	err ;
	//
	if ( m_pInterface != NULL )
	{
		m_pInterface->FlushCommandQueue
				( (fQueueCommand != 0), nPriority ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	QueueCommand( String strID,
//			Integer nNotification := 0, Integer nParameter := 0,
//			Integer nPriority := 0, Boolean fOverwrite := false )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_QueueCommand
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 6 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	int				nNotification, nParameter, nPriority, fOverwrite ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nNotification, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nParameter, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nPriority, lstArg, 4, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fOverwrite, lstArg, 5, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pInterface != NULL )
	{
		m_pInterface->QueueCommand
			( wstrID, nNotification, nParameter,
					nPriority, (fOverwrite != 0) ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	Integer GetCommand( Reference wscCmd,
//				Integer nTimeout, Integer fRemove := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_GetCommand
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHash =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"WndSpriteCmd" ) ) ;
	if ( pHash == NULL )
	{
		return	ESLErrorMsg( "引数に構造体が指定されていません。" ) ;
	}
	int		nTimeout, fRemove ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 2, INFINITE ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fRemove, lstArg, 3, 1 ) ;
	if ( err )
		return	err ;
	//
	ESLError	errResult = eslErrTimeout ;
	if ( m_pInterface != NULL )
	{
		EWndSpriteCmd	wsc ;
		if ( (unsigned int) nTimeout >= 100 )
		{
			DWORD	dwBeginTime = ::timeGetTime( ) ;
			while ( context.GetStatus() == context.xsExecution )
			{
				errResult = m_pInterface->GetCommand( wsc, 10, fRemove != 0 ) ;
				if ( errResult == eslErrSuccess )
				{
					break ;
				}
				if ( ::timeGetTime() - dwBeginTime >= (DWORD) nTimeout )
				{
					break ;
				}
			}
		}
		else
		{
			errResult = m_pInterface->GetCommand( wsc, nTimeout, fRemove != 0 ) ;
		}
		if ( !errResult )
		{
			pHash->SetMemberAsStr( L"strID", wsc.m_wstrID ) ;
			pHash->SetMemberAsStr( L"strFullID", wsc.m_wstrFullID ) ;
			pHash->SetMemberAsInt( L"nNotification", wsc.m_nNotification ) ;
			pHash->SetMemberAsInt( L"nParameter", wsc.m_nParameter ) ;
		}
	}
	return	context.PushObject( new ECSInteger( errResult ) ) ;
}

// メンバ関数 : CallMouseMove()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_CallMouseMove
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	if ( (m_pwndDisplay != NULL) && (m_pInterface != NULL) )
	{
//		m_pInterface->Lock( ) ;
		m_pInterface->CallMouseMove( ) ;
//		m_pwndDisplay->PostMessage( WM_SETCURSOR, 0, HTCLIENT ) ;
//		m_pInterface->Unlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer Lock( Integer nTimeout := INFINITE )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_Lock
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, INFINITE ) ;
	if ( err )
		return	err ;
	//
	ESLError	errResult ;
	if ( (unsigned int) nTimeout >= 100 )
	{
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		errResult = Lock( 33 ) ;
		if ( errResult )
		{
			while ( context.GetStatus() == context.xsExecution )
			{
				DWORD	dwWaitTime = 33 ;
				if ( nTimeout != INFINITE )
				{
					DWORD	dwPastTime = ::timeGetTime() - dwBeginTime ;
					if ( dwPastTime > (DWORD) nTimeout )
					{
						break ;
					}
					dwWaitTime = nTimeout - dwPastTime ;
					if ( dwWaitTime > 33 )
					{
						dwWaitTime = 33 ;
					}
				}
				errResult = Lock( dwWaitTime ) ;
				if ( errResult == eslErrSuccess )
				{
					break ;
				}
			}
		}
	}
	else
	{
		errResult = Lock( nTimeout ) ;
	}
	return	context.PushObject( new ECSInteger( errResult ) ) ;
}

// メンバ関数 : Integer Unlock()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_Unlock
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
#if	defined(_DEBUG)
	if ( (m_dwTlsIndex != 0xFFFFFFFF) && !IsQuickLockedOnScript() )
	{
		ESLAssert( IsQuickLockedOnScript() ) ;
		ESLFileObject *	pfile =
			context.OpenFileOnScript
				( L"assert_error.log", ESLFileObject::modeCreate ) ;
		if ( pfile != NULL )
		{
			context.m_pcsxi->m_csgGlobal.IndexAllMember( ) ;
			context.m_pcsxi->m_csgData.IndexAllMember( ) ;
			context.m_stack.IndexAllMember( ) ;
			context.m_arg.IndexAllMember( ) ;
			//
			EStreamBuffer	bufDump ;
			EString			strDump ;
			strDump = "\r\n[stack]\r\n" ;
			bufDump.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			context.m_stack.DumpObject( bufDump, 0, context ) ;
			//
			strDump = "\r\n\r\n[global]\r\n" ;
			bufDump.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			context.m_pcsxi->m_csgGlobal.DumpObject( bufDump, 0, context ) ;
			//
			strDump = "\r\n\r\n[data]\n" ;
			bufDump.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			context.m_pcsxi->m_csgData.DumpObject( bufDump, 0, context ) ;
			//
			EPtrBuffer	ptrbuf = bufDump.GetBuffer( ) ;
			pfile->Write( ptrbuf, ptrbuf.GetLength() ) ;
			delete	pfile ;
		}
	}
#endif
	err = Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer FreezePaint()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_FreezePaint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;

	FreezePaint() ;

	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer UnfreezePaint()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_UnfreezePaint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;

	UnfreezePaint() ;

	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : SyncTimePaint( Integer nTimeout )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_SyncTimePaint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, 16 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pInterface != NULL )
	{
		m_pInterface->SyncTimePaint( nTimeout ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : AsyncTimePaint()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_AsyncTimePaint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pInterface != NULL )
	{
		m_pInterface->AsyncTimePaint() ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ShowCursor( Integer fShow := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_ShowCursor
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fShow ;
	err = context.GetArgumentAsInt( fShow, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	//
	ShowCursor( fShow != 0 ) ;
	if ( m_pInterface != NULL )
	{
		m_pInterface->CallMouseMove() ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsShowCursor()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSWindow::Call_IsShowCursor
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (long int) IsShowCursor() ) ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSWindow::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::Compare( pwszType, L"ECS_WINDOW_INTERFACE" ) )
	{
		if ( m_ppiw == NULL )
		{
			m_ppiw = new PLUGIN_WINDOW ;
			m_ppiw->pBackLink = this ;
			m_ppiw->pfnGetWindow = PIC_GetWindow ;
			m_ppiw->pfnCreateDisplay = PIC_CreateDisplay ;
			m_ppiw->pfnCloseDisplay = PIC_CloseDisplay ;
			m_ppiw->pfnGetOptionalFuncFlag = PIC_GetOptionalFuncFlag ;
			m_ppiw->pfnSetOptionalFuncFlag = PIC_SetOptionalFuncFlag ;
			m_ppiw->pfnChangeCooperationLevel = PIC_ChangeCooperationLevel ;
			m_ppiw->pfnChangeDisplaySize = PIC_ChangeDisplaySize ;
			m_ppiw->pfnGetDisplaySize = PIC_GetDisplaySize ;
			m_ppiw->pfnUpdateWindow = PIC_UpdateWindow ;
			m_ppiw->pfnIsWindowActive = PIC_IsWindowActive ;
			m_ppiw->pfnEnableCommandQueue = PIC_EnableCommandQueue ;
			m_ppiw->pfnFlushCommandQueue = PIC_FlushCommandQueue ;
			m_ppiw->pfnGetCommand = PIC_GetCommand ;
			m_ppiw->pfnQueueCommand = PIC_QueueCommand ;
			m_ppiw->pfnCallMouseMove = PIC_CallMouseMove ;
			m_ppiw->pfnLock = PIC_Lock ;
			m_ppiw->pfnUnlock = PIC_Unlock ;
			m_ppiw->pfnSyncTimePaint = PIC_SyncTimePaint ;
			m_ppiw->pfnAsyncTimePaint = PIC_AsyncTimePaint ;
			m_ppiw->pfnShowCursor = PIC_ShowCursor ;
			m_ppiw->pfnIsShowCursor = PIC_IsShowCursor ;
			m_ppiw->pfnProcedureOnWindowThread = PIC_ProcedureOnWindowThread ;
		}
		return	(ECS_WINDOW_INTERFACE*) m_ppiw ;
	}
	return	ECSSprite::GetObjectInterface( pwszType ) ;
}

HWND __stdcall ECSWindow::PIC_GetWindow( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay != NULL )
	{
		return	*(ppiw->pBackLink->m_pwndDisplay) ;
	}
	return	NULL ;
}

ESLError __stdcall ECSWindow::PIC_CreateDisplay
	( ECS_WINDOW_INTERFACE * instance, 
		const char * pszWindowName, int fCooperationLevel,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	return	ppiw->pBackLink->CreateDisplay
		( pszWindowName,
			(EGameWindow::CooperationLevel) fCooperationLevel,
			nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
}

void __stdcall ECSWindow::PIC_CloseDisplay( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	ppiw->pBackLink->CloseDisplay( ) ;
}

unsigned int __stdcall ECSWindow::PIC_GetOptionalFuncFlag
					( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay != NULL )
	{
		return	ppiw->pBackLink->m_pwndDisplay->GetOptionalFuncFlag( ) ;
	}
	return	0 ;
}

void __stdcall ECSWindow::PIC_SetOptionalFuncFlag
	( ECS_WINDOW_INTERFACE * instance, unsigned int nFlags )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay != NULL )
	{
		ppiw->pBackLink->m_pwndDisplay->SetOptionalFuncFlag( nFlags ) ;
	}
}

ESLError __stdcall ECSWindow::PIC_ChangeCooperationLevel
	( ECS_WINDOW_INTERFACE * instance, int fCooperationLevel )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err =
		ppiw->pBackLink->m_pwndDisplay->ChangeCooperationLevel
				( (EGameWindow::CooperationLevel) fCooperationLevel ) ;
	ppiw->pBackLink->UpdateImagePosition( ) ;
	return	err ;
}

ESLError __stdcall ECSWindow::PIC_ChangeDisplaySize
	( ECS_WINDOW_INTERFACE * instance,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nBitsPerPixel, unsigned int nFrequency )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err =
		ppiw->pBackLink->m_pwndDisplay->ChangeDisplaySize
				( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
	ppiw->pBackLink->UpdateImagePosition( ) ;
	return	err ;
}

void __stdcall ECSWindow::PIC_GetDisplaySize
	( ECS_WINDOW_INTERFACE * instance, SIZE * pDisplaySize )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( pDisplaySize != NULL )
	{
		pDisplaySize->cx = 0 ;
		pDisplaySize->cy = 0 ;
		//
		if ( ppiw->pBackLink->m_pwndDisplay != NULL )
		{
			*pDisplaySize = ppiw->pBackLink->m_pwndDisplay->GetDisplaySize() ;
		}
	}
}

void __stdcall ECSWindow::PIC_UpdateWindow( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay != NULL )
	{
		ppiw->pBackLink->m_pwndDisplay->UpdateWindow( ) ;
	}
}

int __stdcall ECSWindow::PIC_IsWindowActive( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pwndDisplay != NULL )
	{
		return	ppiw->pBackLink->m_pwndDisplay->IsWindowActive( ) ;
	}
	return	false ;
}

void __stdcall ECSWindow::PIC_EnableCommandQueue
	( ECS_WINDOW_INTERFACE * instance, int fQueueCommand )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		ppiw->pBackLink->m_pInterface->
			EnableCommandQueue( (fQueueCommand != 0) ) ;
	}
}

void __stdcall ECSWindow::PIC_FlushCommandQueue
	( ECS_WINDOW_INTERFACE * instance, int fQueueCommand )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		ppiw->pBackLink->m_pInterface->
			FlushCommandQueue( (fQueueCommand != 0) ) ;
	}
}

ESLError __stdcall ECSWindow::PIC_GetCommand
	( ECS_WINDOW_INTERFACE * instance,
		ECS_WINDOW_INTERFACE::WndCommand * pCmd,
					DWORD dwTimeout, int fRemove )
{
	ESLError	err = eslErrGeneral ;
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		EWndSpriteCmd	wscCmd ;
		err = ppiw->pBackLink->m_pInterface->
				GetCommand( wscCmd, dwTimeout, (fRemove != 0) ) ;
		if ( !err )
		{
			if ( pCmd->m_pID != NULL )
			{
				pCmd->m_pID->Release( ) ;
			}
			if ( pCmd->m_pFullID != NULL )
			{
				pCmd->m_pFullID->Release( ) ;
			}
			pCmd->m_pID =
				(new ECSString( wscCmd.m_wstrID ))->CreateInterface( ) ;
			pCmd->m_pFullID =
				(new ECSString( wscCmd.m_wstrFullID ))->CreateInterface( ) ;
			pCmd->m_nNotification = wscCmd.m_nNotification ;
			pCmd->m_nParameter = wscCmd.m_nParameter ;
		}
	}
	return	err ;
}

void __stdcall ECSWindow::PIC_QueueCommand
	( ECS_WINDOW_INTERFACE * instance,
		const wchar_t * pwszID, long int nNotification, long int nParameter )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		ppiw->pBackLink->m_pInterface->
			QueueCommand( pwszID, nNotification, nParameter ) ;
	}
}

void __stdcall ECSWindow::PIC_CallMouseMove( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		ppiw->pBackLink->m_pInterface->CallMouseMove( ) ;
	}
}

ESLError __stdcall ECSWindow::PIC_Lock
	( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	return	ppiw->pBackLink->Lock( dwTimeout ) ;
}

void __stdcall ECSWindow::PIC_Unlock( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	ppiw->pBackLink->Unlock( ) ;
}

ESLError __stdcall ECSWindow::PIC_SyncTimePaint
	( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface == NULL )
	{
		return	eslErrGeneral ;
	}
	return	ppiw->pBackLink->m_pInterface->SyncTimePaint( dwTimeout ) ;
}

void __stdcall ECSWindow::PIC_AsyncTimePaint( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	if ( ppiw->pBackLink->m_pInterface != NULL )
	{
		ppiw->pBackLink->m_pInterface->AsyncTimePaint( ) ;
	}
}

void __stdcall ECSWindow::PIC_ShowCursor
	( ECS_WINDOW_INTERFACE * instance, int fShow )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	ppiw->pBackLink->ShowCursor( (fShow != 0) ) ;
}

int __stdcall ECSWindow::PIC_IsShowCursor( ECS_WINDOW_INTERFACE * instance )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	return	ppiw->pBackLink->IsShowCursor( ) ;
}

ESLError __stdcall ECSWindow::PIC_ProcedureOnWindowThread
	( ECS_WINDOW_INTERFACE * instance,
		PFUNC_PROCEDURE pfnProc,
			void * pInstance, LRESULT * pResult, int fAsync )
{
	PLUGIN_WINDOW *	ppiw = (PLUGIN_WINDOW*) instance ;
	ESLAssert( ppiw->pBackLink->m_ppiw == ppiw ) ;
	return	ppiw->pBackLink->ProcedureOnWindowThread
					( pfnProc, pInstance, pResult, (fAsync != 0) ) ;
}
