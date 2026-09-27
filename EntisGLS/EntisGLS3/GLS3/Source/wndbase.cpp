
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2025 Leshade Entis. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <ddraw.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/wglext.h>


//////////////////////////////////////////////////////////////////////////////
// 初期化関数
//////////////////////////////////////////////////////////////////////////////

static const char	szGLSVersion[] = "EntisGLS version 3.10b" ;

DWORD glsGetLibraryVersion( void )
{
	return	0x000310B0 ;
}

extern	"C"	HESLHEAP	EGL_hImageHeap ;
static long int	gls_nInitializedCount = 0 ;
static int		gls_nTLSIndex = -1 ;

struct	GLS_TASK_INFO
{
	long int			nInitializedCount ;
	HEGL_RENDER_POLYGON	hRenderPoly ;

	GLS_TASK_INFO( void )
		{
			nInitializedCount = 0 ;
			hRenderPoly = NULL ;
		}
	~GLS_TASK_INFO( void )
		{
			if ( hRenderPoly != NULL )
			{
				hRenderPoly->Release( ) ;
			}
		}
	void * operator new ( size_t stObj )
		{
			return	::eslHeapAllocate( NULL, stObj, 0 ) ;
		}
	void operator delete ( void * ptrObj )
		{
			::eslHeapFree( NULL, ptrObj ) ;
		}
} ;

void glsInitializeLibrary( void )
{
	if ( gls_nTLSIndex == -1 )
	{
		gls_nTLSIndex = ::TlsAlloc( ) ;
	}
	long int	nInitCount ;
#if	defined(ERI_INTEL_X86)
	__asm
	{
		mov		eax, 1
		lock	xadd	gls_nInitializedCount, eax
		inc		eax
		mov		nInitCount, eax
	}
#else
	nInitCount = ++ gls_nInitializedCount ;
#endif
	if ( nInitCount == 1 )
	{
		::eriInitializeLibrary( ) ;
		::eglInitializeMathFunctions( ) ;
		::eslGetGlobalHeap( ) ;
		EGL_hImageHeap = ::eslHeapCreate( 0, 0, ESL_HEAP_ZERO_INIT ) ;
		::glsInitializeTask( ) ;
		::timeBeginPeriod( 1 ) ;
	}
}

void glsCloseLibrary( void )
{
	if ( ::InterlockedDecrement( &gls_nInitializedCount ) == 0 )
	{
		::glsCloseTask( ) ;
		::eriCloseLibrary( ) ;
		//
		if ( gls_nTLSIndex != -1 )
		{
			::TlsFree( gls_nTLSIndex ) ;
			gls_nTLSIndex = -1 ;
		}
		__try
		{
			::eslFreeGlobalHeap( ) ;
			::eslHeapDump( EGL_hImageHeap, 100 ) ;
			::eslHeapDestroy( EGL_hImageHeap ) ;
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			::OutputDebugString
				( "glsCloseLibrary 内で例外エラーが発生しました。\n" ) ;
		}
		EGL_hImageHeap = NULL ;
	}
}

LONG WINAPI eslUnhandledExceptionFilter( EXCEPTION_POINTERS * excpinf ) ;

void glsInitializeTask( void )
{
	if ( gls_nTLSIndex != -1 )
	{
		GLS_TASK_INFO *	pglsti =
			(GLS_TASK_INFO*) ::TlsGetValue( gls_nTLSIndex ) ;
		if ( pglsti == NULL )
		{
			pglsti = new GLS_TASK_INFO ;
			::TlsSetValue( gls_nTLSIndex, pglsti ) ;
		}
		if ( ::InterlockedIncrement( &(pglsti->nInitializedCount) ) == 1 )
		{
			::eriInitializeTask( ) ;
			::SetUnhandledExceptionFilter( &eslUnhandledExceptionFilter ) ;
		}
	}
}

void glsCloseTask( void )
{
	if ( gls_nTLSIndex != -1 )
	{
		GLS_TASK_INFO *	pglsti =
			(GLS_TASK_INFO*) ::TlsGetValue( gls_nTLSIndex ) ;
		if ( pglsti != NULL )
		{
			if ( ::InterlockedDecrement( &(pglsti->nInitializedCount) ) == 0 )
			{
				delete	pglsti ;
				::TlsSetValue( gls_nTLSIndex, NULL ) ;
				::eriCloseTask( ) ;
			}
		}
	}
}

HESLHEAP glsGetImageGlobalHeap( void )
{
	if ( EGL_hImageHeap == NULL )
	{
		EGL_hImageHeap = ::eslHeapCreate( 0, 0, ESL_HEAP_ZERO_INIT, NULL ) ;
	}
	return	EGL_hImageHeap ;
}

HEGL_RENDER_POLYGON eglCurrentRenderPolygon( void )
{
	if ( gls_nTLSIndex == -1 )
	{
		::glsInitializeLibrary( ) ;
	}
	GLS_TASK_INFO *	pglsti =
		(GLS_TASK_INFO*) ::TlsGetValue( gls_nTLSIndex ) ;
	if ( pglsti == NULL )
	{
		pglsti = new GLS_TASK_INFO ;
		::TlsSetValue( gls_nTLSIndex, pglsti ) ;
	}
	if ( pglsti->hRenderPoly == NULL )
	{
		pglsti->hRenderPoly = ::eglCreateRenderPolygon( ) ;
	}
	return	pglsti->hRenderPoly ;
}

void glsEnableProcessorType( DWORD dwForceEnable )
{
	::eriEnableMMX( dwForceEnable ) ;
	::eglInitializeMathFunctions( ) ;
}

void glsDisableProcessorType( DWORD dwForceDisable )
{
	::eriDisableMMX( dwForceDisable ) ;
	::eglInitializeMathFunctions( ) ;
}

DWORD glsGetEnabledProcessorType( void )
{
	return	ERI_EnabledProcessorType ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWindow, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWindow::EWindow( EWindowInterface * pWUI )
	: m_hWnd( NULL ), m_pWUI( pWUI ), m_pFilter( NULL )
{
	m_wpDefProc = & ::DefWindowProc ;
	//
	::InitializeCriticalSection( &m_csWUI ) ;
	//
	if ( m_pWUI )
	{
		m_pWUI->OnAttachedWindow( this ) ;
	}
}

EWindow::EWindow( HWND hWnd, EWindowInterface * pWUI )
	: m_hWnd( hWnd ), m_pWUI( pWUI ), m_pFilter( NULL )
{
	m_wpDefProc = (WNDPROC) ::GetWindowLong( hWnd, GWL_WNDPROC ) ;
	::SetWindowLong( hWnd, GWL_USERDATA, (LONG) this ) ;
	::SetWindowLong( hWnd, GWL_WNDPROC, (LONG) &EWindow::WindowCallbackProc ) ;
	//
	::InitializeCriticalSection( &m_csWUI ) ;
	//
	if ( m_pWUI )
	{
		m_pWUI->OnAttachedWindow( this ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWindow::~EWindow( void )
{
	::EnterCriticalSection( &m_csWUI ) ;
	if ( m_pWUI )
	{
		m_pWUI->OnDetachedWindow( this ) ;
	}
	if ( m_pFilter )
	{
		m_pFilter->OnDetachedWindow( this ) ;
	}
	::LeaveCriticalSection( &m_csWUI ) ;
	//
	if ( (m_hWnd != NULL) &&
		::IsWindow( m_hWnd ) && (m_wpDefProc != & ::DefWindowProc) )
	{
		::SetWindowLong( m_hWnd, GWL_WNDPROC, (LONG) m_wpDefProc ) ;
	}
	::DeleteCriticalSection( &m_csWUI ) ;
}

// インターフェース設定
//////////////////////////////////////////////////////////////////////////////
void EWindow::SetInterface( EWindowInterface * pWUI )
{
	::EnterCriticalSection( &m_csWUI ) ;
	if ( m_pWUI )
	{
		m_pWUI->OnDetachedWindow( this ) ;
	}
	m_pWUI = pWUI ;
	if ( m_pWUI )
	{
		m_pWUI->OnAttachedWindow( this ) ;
	}
	::LeaveCriticalSection( &m_csWUI ) ;
}

// フィルター設定
//////////////////////////////////////////////////////////////////////////////
void EWindow::SetInputFilter( EInputFilter * pFilter )
{
	::EnterCriticalSection( &m_csWUI ) ;
	if ( m_pFilter )
	{
		m_pFilter->OnDetachedWindow( this ) ;
	}
	m_pFilter = pFilter ;
	if ( m_pFilter )
	{
		m_pFilter->OnAttachedWindow( this ) ;
	}
	::LeaveCriticalSection( &m_csWUI ) ;
}

// ウィンドウコールバック関数
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK EWindow::WindowCallbackProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	EWindow *	pWindow ;
	if ( uMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT	pcs = (LPCREATESTRUCT) lParam ;
		pWindow = (EWindow*) pcs->lpCreateParams ;
		::SetWindowLong( hWnd, GWL_USERDATA, (LONG) pWindow ) ;
		pWindow->m_hWnd = hWnd ;
	}
	else
	{
		pWindow = (EWindow*) ::GetWindowLong( hWnd, GWL_USERDATA ) ;
		if ( pWindow != NULL )
		{
			try
			{
				if ( pWindow->m_hWnd != hWnd )
				{
					pWindow = NULL ;
				}
			}
			catch ( ... )
			{
			}
		}
	}
	if ( pWindow != NULL )
	{
		return	pWindow->WindowProc( uMsg, wParam, lParam ) ;
	}
	else
	{
		return	::DefWindowProc( hWnd, uMsg, wParam, lParam ) ;
	}
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EWindow::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	::EnterCriticalSection( &m_csWUI ) ;
	//
	MSG	msg ;
	msg.hwnd = m_hWnd ;
	msg.message = uMsg ;
	msg.wParam = wParam ;
	msg.lParam = lParam ;
	msg.time = 0 ;
	msg.pt.x = 0 ;
	msg.pt.y = 0 ;
	//
	if ( m_pFilter != NULL )
	{
		if ( m_pFilter->GetWindowInterface() == NULL )
		{
			if ( m_pFilter->ProcessMessage( msg ) )
			{
				::LeaveCriticalSection( &m_csWUI ) ;
				return	0 ;
			}
		}
	}
	if ( m_pWUI != NULL )
	{
		LRESULT	lResult = 0 ;
		if ( !m_pWUI->PreTranslateMessage( msg ) )
		{
			::LeaveCriticalSection( &m_csWUI ) ;
			lResult = m_pWUI->WindowProc
				( this, msg.message, msg.wParam, msg.lParam ) ;
		}
		else
		{
			::LeaveCriticalSection( &m_csWUI ) ;
		}
		return	lResult ;
	}
	else
	{
		::LeaveCriticalSection( &m_csWUI ) ;
		return	DefWindowProc( msg.message, msg.wParam, msg.lParam ) ;
	}
}

// デフォルトウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EWindow::DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( (m_wpDefProc == NULL) || (m_wpDefProc == & ::DefWindowProc) )
	{
		return	::DefWindowProc( m_hWnd, uMsg, wParam, lParam ) ;
	}
	return	::CallWindowProc( m_wpDefProc, m_hWnd, uMsg, wParam, lParam ) ;
}

// ウィンドウ作成
//////////////////////////////////////////////////////////////////////////////
ESLError EWindow::Create
	( const char * pszClassName, const char * pszWindowName,
		DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU hMenu, HINSTANCE hInstance )
{
	m_hWnd = ::CreateWindowEx
		( dwExStyle, pszClassName, pszWindowName, dwStyle,
			x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this ) ;
	if ( m_hWnd != NULL )
	{
//		m_wpDefProc = (WNDPROC) ::GetWindowLong( m_hWnd, GWL_WNDPROC ) ;
//		if ( m_wpDefProc == &EWindow::WindowCallbackProc )
//			m_wpDefProc = & ::DefWindowProc ;
		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG) this ) ;
		::SetWindowLong
			( m_hWnd, GWL_WNDPROC, (LONG) &EWindow::WindowCallbackProc ) ;
		//
		return	eslErrSuccess ;
	}
	else
	{
		return	ESLErrorMsg( "ウィンドウの作成に失敗しました。" ) ; ;
	}
}

// ウィンドウ削除
//////////////////////////////////////////////////////////////////////////////
ESLError EWindow::DestroyWindow( void )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	BOOL	fResult = ::DestroyWindow( m_hWnd ) ;
	if ( fResult )
	{
		m_hWnd = NULL ;
		return	eslErrSuccess ;
	}
	else
	{
		return	ESLErrorMsg( "ウィンドウの破棄に失敗しました。" ) ;
	}
}

// ウィンドウクラス登録
//////////////////////////////////////////////////////////////////////////////
ATOM EWindow::RegisterClass( const WNDCLASS & wc )
{
	WNDCLASS	wndclass = wc ;
	wndclass.lpfnWndProc = &EWindow::WindowCallbackProc ;
	wndclass.cbWndExtra = sizeof(EWindow*) ;
	return	::RegisterClass( &wndclass ) ;
}

ATOM EWindow::RegisterWindowClass
	( const char * pszClassName,
		UINT uStyle, HINSTANCE hInstance,
		HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground )
{
	WNDCLASS	wndclass ;
	wndclass.style = uStyle ;
	wndclass.lpfnWndProc = &EWindow::WindowCallbackProc ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = sizeof(EWindow*) ;
	wndclass.hInstance = hInstance ;
	wndclass.hIcon = hIcon ;
	wndclass.hCursor = hCursor ;
	wndclass.hbrBackground = hbrBackground ;
	wndclass.lpszMenuName = NULL ;
	wndclass.lpszClassName = pszClassName ;
	return	::RegisterClass( &wndclass ) ;
}

// ウィンドウクラス解除
//////////////////////////////////////////////////////////////////////////////
BOOL EWindow::UnregisterClass( const char * pszClassName, HINSTANCE hInstance )
{
	return	::UnregisterClass( pszClassName, hInstance ) ;
}

// ウィンドウスタイル
//////////////////////////////////////////////////////////////////////////////
DWORD EWindow::GetStyle( void ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetWindowLong( m_hWnd, GWL_STYLE ) ;
}

DWORD EWindow::GetExStyle( void ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetWindowLong( m_hWnd, GWL_EXSTYLE ) ;
}

BOOL EWindow::SetStyle( DWORD dwStyle )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetWindowLong( m_hWnd, GWL_STYLE, dwStyle ) ;
}

BOOL EWindow::SetExStyle( DWORD dwExStyle )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetWindowLong( m_hWnd, GWL_EXSTYLE, dwExStyle ) ;
}

// ウィンドウアイコン
//////////////////////////////////////////////////////////////////////////////
HICON EWindow::GetIcon( void ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	(HICON)::GetClassLong( m_hWnd, GCL_HICON ) ;
}

void EWindow::SetIcon( HICON hIcon )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	::SetClassLong( m_hWnd, GCL_HICON, (LONG) hIcon ) ;
	SendMessage( WM_SETICON, ICON_BIG, (LPARAM) hIcon ) ;
	SendMessage( WM_SETICON, ICON_SMALL, (LPARAM) hIcon ) ;
}

// マウスキャプチャー
//////////////////////////////////////////////////////////////////////////////
HWND EWindow::GetCapture( void )
{
	return	::GetCapture( ) ;
}

HWND EWindow::SetCapture( void )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetCapture( m_hWnd ) ;
}

BOOL EWindow::ReleaseCapture( void )
{
	return	::ReleaseCapture( ) ;
}

// ウィンドウリージョン
//////////////////////////////////////////////////////////////////////////////
int EWindow::GetWindowRgn( HRGN hRgn ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetWindowRgn( m_hWnd, hRgn ) ;
}

int EWindow::SetWindowRgn( HRGN hRgn, BOOL bRedraw )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetWindowRgn( m_hWnd, hRgn, bRedraw ) ;
}

// ウィンドウ位置
//////////////////////////////////////////////////////////////////////////////
BOOL EWindow::MoveWindow
( int x, int y, int nWidth, int nHeight, BOOL bRepaint )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::MoveWindow( m_hWnd, x, y, nWidth, nHeight, bRepaint ) ;
}

BOOL EWindow::SetWindowPos
( HWND hInsertAfter, int x, int y, int cx, int cy, UINT uFlags )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetWindowPos( m_hWnd, hInsertAfter, x, y, cx, cy, uFlags ) ;
}

BOOL EWindow::GetWindowPlacement( WINDOWPLACEMENT * lpwndpl ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetWindowPlacement( m_hWnd, lpwndpl ) ;
}

BOOL EWindow::SetWindowPlacement( const WINDOWPLACEMENT * lpcwndpl )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetWindowPlacement( m_hWnd, lpcwndpl ) ;
}

BOOL EWindow::GetWindowRect( LPRECT lpRect ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetWindowRect( m_hWnd, lpRect ) ;
}

BOOL EWindow::GetClientRect( LPRECT lpRect ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetClientRect( m_hWnd, lpRect ) ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
HDC EWindow::BeginPaint( PAINTSTRUCT * lpPaint )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::BeginPaint( m_hWnd, lpPaint ) ;
}

BOOL EWindow::EndPaint( const PAINTSTRUCT * lpPaint )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::EndPaint( m_hWnd, lpPaint ) ;
}

HDC EWindow::GetDC( void )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetDC( m_hWnd ) ;
}

int EWindow::ReleaseDC( HDC hDC )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ReleaseDC( m_hWnd, hDC ) ;
}

BOOL EWindow::InvalidateRect( const RECT * lpRect, BOOL bErase )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::InvalidateRect( m_hWnd, lpRect, bErase ) ;
}

BOOL EWindow::GetUpdateRect( RECT * lpRect, BOOL bErase )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::GetUpdateRect( m_hWnd, lpRect, bErase ) ;
}

BOOL EWindow::UpdateWindow( void )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::UpdateWindow( m_hWnd ) ;
}

BOOL EWindow::ShowWindow( int nCmdShow )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ShowWindow( m_hWnd, nCmdShow ) ;
}

// 座標変換
//////////////////////////////////////////////////////////////////////////////
BOOL EWindow::ClientToScreen( LPPOINT lpPoint ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ClientToScreen( m_hWnd, lpPoint ) ;
}

BOOL EWindow::ScreenToClient( LPPOINT lpPoint ) const
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ScreenToClient( m_hWnd, lpPoint ) ;
}

// スクロール
//////////////////////////////////////////////////////////////////////////////
BOOL EWindow::ScrollWindow
( int xAmount, int yAmount, const RECT * lpRect, const RECT * lpcClip )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ScrollWindow( m_hWnd, xAmount, yAmount, lpRect, lpcClip ) ;
}

BOOL EWindow::ScrollWindowEx
( int xAmount, int yAmount,
	const RECT * lpRect, const RECT * lpcClip,
	HRGN hrgnUpdate, LPRECT prcUpdate, UINT flags )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::ScrollWindowEx
		( m_hWnd, xAmount, yAmount,
			lpRect, lpcClip, hrgnUpdate, prcUpdate, flags ) ;
}

// ウィンドウタイマー
//////////////////////////////////////////////////////////////////////////////
UINT EWindow::SetTimer
( UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SetTimer( m_hWnd, nIDEvent, uElapse, lpTimerFunc ) ;
}

BOOL EWindow::KillTimer( UINT nIDEvent )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::KillTimer( m_hWnd, nIDEvent ) ;
}

// メッセージボックス
//////////////////////////////////////////////////////////////////////////////
int EWindow::MessageBox( LPCSTR lpText, LPCSTR lpCaption, UINT uType )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::MessageBox( m_hWnd, lpText, lpCaption, uType ) ;
}

// メッセージ関数
//////////////////////////////////////////////////////////////////////////////
BOOL EWindow::PostMessage
( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::PostMessage( m_hWnd, uMsg, wParam, lParam ) ;
}

LRESULT EWindow::SendMessage
( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SendMessage( m_hWnd, uMsg, wParam, lParam ) ;
}

LRESULT EWindow::SendMessageTimeout
( UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT uFlags, UINT uTimeout, LPDWORD lpdwResult )
{
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	return	::SendMessageTimeout
		( m_hWnd, uMsg, wParam, lParam, uFlags, uTimeout, lpdwResult ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウユーザーインターフェースフィルター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EInputFilter, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EInputFilter::EInputFilter( void )
{
	m_pAttachedWnd = NULL ;
	m_pAttachedItf = NULL ;
	m_dwFlags = fmNormal ;
	m_nInputLimit = 0 ;
	m_hInputEvent = NULL ;
	//
	m_fJoyCaptured = 0 ;
	m_rJoyThreshold = 0.0 ;
	//
	m_maskXInputDev = 0 ;
	//
	m_dwContextKeyMask = 0 ;
	//
	m_nBeginFilter = ::RegisterWindowMessage( "EInputFilter_BeginFilter" ) ;
	//
	m_hInputEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	::InitializeCriticalSection( &m_cs ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EInputFilter::~EInputFilter( void )
{
	CloseFilter( ) ;
	//
	if ( m_hInputEvent != NULL )
	{
		::CloseHandle( m_hInputEvent ) ;
		m_hInputEvent = NULL ;
	}
	::DeleteCriticalSection( &m_cs ) ;
}

// ウィンドウメッセージを処理
//////////////////////////////////////////////////////////////////////////////
bool EInputFilter::ProcessMessage( MSG & msg )
{
	if ( m_pAttachedWnd == NULL )
	{
		return	false ;
	}
	INPUT_EVENT	iev ;
	bool		fMsgProcess = false ;
	switch ( msg.message )
	{
	case	WM_TIMER:
		if ( msg.wParam == 1 )
		{
			if ( (m_dwFlags & fmStickMouse) && !(m_dwFlags & fmJoyMouse) )
			{
				//
				// 仮想ジョイスティックの方向キー入力をマウス座標に反映させる
				//////////////////////////////////////////////////////////////
				POINT	ptCursor ;
				::GetCursorPos( &ptCursor ) ;
				bool	fMove = false ;
				for ( int i = 0; i < (int) m_lstVirtPad.GetSize(); i ++ )
				{
					BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( i ) ;
					if ( pbs == NULL )
						continue ;
					if ( pbs->nStatus[jbUp] & bmPushing )
					{
						fMove = true ;
						ptCursor.y -= 2 ;
					}
					if ( pbs->nStatus[jbDown] & bmPushing )
					{
						fMove = true ;
						ptCursor.y += 2 ;
					}
					if ( pbs->nStatus[jbLeft] & bmPushing )
					{
						fMove = true ;
						ptCursor.x -= 2 ;
					}
					if ( pbs->nStatus[jbRight] & bmPushing )
					{
						fMove = true ;
						ptCursor.x += 2 ;
					}
				}
				if ( fMove )
				{
					::SetCursorPos( ptCursor.x, ptCursor.y ) ;
				}
			}
			for ( int iXInput = 0; iXInput < deviceXInputCount; iXInput ++ )
			{
				const int	iJoy = joyStickXInput1 + iXInput ;
				PollXInputState
					( m_vJoyPos[iJoy], m_jsJoyStatus[iJoy], iXInput, true ) ;
			}
			ProcessJoystick( 0 ) ;
			ProcessJoystick( 1 ) ;
		}
		break ;

	case	WM_MOUSEMOVE:
		if ( m_dwFlags & fmJoyMouse )
		{
			//
			// マウスの移動を仮想ジョイスティックの方向キー入力に変換する
			//////////////////////////////////////////////////////////////////
			POINT	ptCursor ;
			ptCursor.x = (__int16) (msg.lParam & 0xFFFF) ;
			ptCursor.y = (__int16) (msg.lParam >> 16) ;
			ptCursor.x -= m_ptMouseBase.x ;
			ptCursor.y -= m_ptMouseBase.y ;
			//
			bool	fMove = false ;
			iev.idType = idJoyStick ;
			for ( ; ; )
			{
				if ( ptCursor.x >= m_nMouseThreshold )
				{
					iev.iKeyNum = jbRight ;
					ptCursor.x -= m_nMouseThreshold ;
				}
				else if ( ptCursor.x <= - m_nMouseThreshold )
				{
					iev.iKeyNum = jbLeft ;
					ptCursor.x += m_nMouseThreshold ;
				}
				else
				{
					break ;
				}
				fMove = true ;
				ProcessEvent( iev, true ) ;
				ProcessEvent( iev, false ) ;
			}
			for ( ; ; )
			{
				if ( ptCursor.y >= m_nMouseThreshold )
				{
					iev.iKeyNum = jbDown ;
					ptCursor.y -= m_nMouseThreshold ;
				}
				else if ( ptCursor.y <= - m_nMouseThreshold )
				{
					iev.iKeyNum = jbUp ;
					ptCursor.y += m_nMouseThreshold ;
				}
				else
				{
					break ;
				}
				fMove = true ;
				ProcessEvent( iev, true ) ;
				ProcessEvent( iev, false ) ;
			}
			if ( fMove )
			{
				ptCursor.x += m_ptMouseBase.x ;
				ptCursor.y += m_ptMouseBase.y ;
				::ClientToScreen( msg.hwnd, &ptCursor ) ;
				::SetCursorPos( ptCursor.x, ptCursor.y ) ;
			}
			fMsgProcess = true ;
		}
		break ;

	case	MM_JOY1BUTTONDOWN:
	case	MM_JOY1BUTTONUP:
	case	MM_JOY1MOVE:
		//
		// ジョイスティック１処理
		//
		ProcessJoystick( 0 ) ;
		PollXInputState
			( m_vJoyPos[joyStickXInput1], m_jsJoyStatus[joyStickXInput1], 0, true ) ;
		fMsgProcess = true ;
		break ;

	case	MM_JOY2BUTTONDOWN:
	case	MM_JOY2BUTTONUP:
	case	MM_JOY2MOVE:
		//
		// ジョイスティック２処理
		//
		ProcessJoystick( 1 ) ;
		PollXInputState
			( m_vJoyPos[joyStickXInput2], m_jsJoyStatus[joyStickXInput2], 1, true ) ;
		fMsgProcess = true ;
		break ;

	case	WM_LBUTTONDOWN:
	case	WM_LBUTTONUP:
		//
		// マウス左ボタン処理
		//
		iev.idType = idMouse ;
		iev.iKeyNum = VK_LBUTTON ;
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_LBUTTONDOWN) ) ;
		break ;

	case	WM_RBUTTONDOWN:
	case	WM_RBUTTONUP:
		//
		// マウス左ボタン処理
		//
		iev.idType = idMouse ;
		iev.iKeyNum = VK_RBUTTON ;
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_RBUTTONDOWN) ) ;
		break ;

	case	WM_MOUSELEAVE:
		if ( ::GetCapture() != (HWND) *m_pAttachedWnd )
		{
			iev.idType = idMouse ;
			iev.iKeyNum = VK_LBUTTON ;
			fMsgProcess = ProcessEvent( iev, false ) ;
			//
			iev.iKeyNum = VK_RBUTTON ;
			fMsgProcess |= ProcessEvent( iev, false ) ;
		}
		break ;

	case	WM_MBUTTONDOWN:
	case	WM_MBUTTONUP:
		//
		// マウス中央ボタン処理
		//
		iev.idType = idMouse ;
		iev.iKeyNum = VK_MBUTTON ;
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_MBUTTONDOWN) ) ;
		break ;

	case	WM_XBUTTONDOWN:
	case	WM_XBUTTONUP:
		//
		// マウス第4ボタン処理
		//
		iev.idType = idMouse ;
		iev.iKeyNum = VK_XBUTTON1 ;
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_XBUTTONDOWN) ) ;
		break ;

	case	WM_MOUSEWHEEL:
		//
		// マウスホイール処理
		//
		iev.idType = idMouse ;
		iev.iKeyNum = (((signed __int32) msg.wParam) < 0) ? VK_UP : VK_DOWN ;
		ProcessEvent( iev, true ) ;
		ProcessEvent( iev, false ) ;
		fMsgProcess = true ;
		break ;

	case	WM_KEYDOWN:
	case	WM_KEYUP:
		//
		// キー入力処理
		//
		iev.idType = idKeyboard ;
		iev.iKeyNum = msg.wParam ;
		if ( m_dwFlags & fmContextKey )
		{
			switch ( iev.iKeyNum )
			{
			case	VK_SHIFT:
			case	VK_CONTROL:
			case	VK_MENU:
			case	VK_CAPITAL:
				m_dwContextKeyMask = QueryCurrentContextKeyMask( ) ;
				break ;
			default:
				iev.iKeyNum |= m_dwContextKeyMask ;
				break ;
			}
		}
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_KEYDOWN) ) ;
		break ;

	case	WM_SYSKEYDOWN:
	case	WM_SYSKEYUP:
		iev.idType = idKeyboard ;
		iev.iKeyNum = msg.wParam ;
		if ( m_dwFlags & fmContextKey )
		{
			switch ( iev.iKeyNum )
			{
			case	VK_SHIFT:
			case	VK_CONTROL:
			case	VK_MENU:
			case	VK_CAPITAL:
				m_dwContextKeyMask = QueryCurrentContextKeyMask( ) ;
				break ;
			default:
				iev.iKeyNum |= m_dwContextKeyMask ;
				break ;
			}
		}
		fMsgProcess = ProcessEvent( iev, (msg.message == WM_SYSKEYDOWN) ) ;
		break ;

	case	WM_ACTIVATE:
		if ( LOWORD(msg.wParam) != WA_INACTIVE )
		{
			if ( m_dwFlags & fmJoyMouse )
			{
				POINT	ptCursor ;
				ptCursor.x = m_ptMouseBase.x ;
				ptCursor.y = m_ptMouseBase.y ;
				::ClientToScreen( msg.hwnd, &ptCursor ) ;
				::SetCursorPos( ptCursor.x, ptCursor.y ) ;
			}
/*			if ( ::joySetCapture
					( msg.hwnd, JOYSTICKID1, 0, TRUE ) == JOYERR_NOERROR )
			{
				m_fJoyCaptured |= 0x01 ;
			}
			if ( ::joySetCapture
					( msg.hwnd, JOYSTICKID2, 0, TRUE ) == JOYERR_NOERROR )
			{
				m_fJoyCaptured |= 0x02 ;
			}*/
			m_dwContextKeyMask = QueryCurrentContextKeyMask( ) ;
			break ;
		}
	case	WM_KILLFOCUS:
		{
			for ( int i = 0; i < (int) m_lstVirtPad.GetSize(); i ++ )
			{
				BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( i ) ;
				ESLAssert( pbs != NULL ) ;
				if ( pbs != NULL )
				{
					for ( int j = 0; j < jbButtonMax; j ++ )
					{
						pbs->nStatus[j] &= ~bmPushing ;
					}
				}
			}
			m_dwContextKeyMask = 0 ;
		}
		break ;

	case	WM_CAPTURECHANGED:
		if ( (HWND) msg.lParam != m_pAttachedWnd->m_hWnd )
		{
			for ( int i = 0; i < (int) m_lstVirtPad.GetSize(); i ++ )
			{
				BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( i ) ;
				ESLAssert( pbs != NULL ) ;
				if ( pbs != NULL )
				{
					for ( int j = 0; j < jbButtonMax; j ++ )
					{
						pbs->nStatus[j] &= ~bmPushing ;
					}
				}
			}
			m_dwContextKeyMask = 0 ;
		}
		break ;

	case	WM_CREATE:
		BeginFilter( ) ;
		break ;

	default:
		if ( msg.message == m_nBeginFilter )
		{
			BeginFilter( ) ;
		}
		break ;
	}
	//
	return	fMsgProcess && !(m_dwFlags & fmTransparent) ;
}

// ジョイスティックイベントを処理
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::ProcessJoystick( int iJoyNum )
{
	static const UINT	nJoyIDs[2] =
	{
		JOYSTICKID1, JOYSTICKID2
	} ;
	if ( ::GetFocus() == NULL )
	{
		// 非アクティブ時はすべてのボタン押下を解除する
		INPUT_EVENT	iev ;
		iev.idType = idJoyStick ;
		iev.iDevNum = iJoyNum ;
		for ( int i = 0; i < jbButtonMax; i ++ )
		{
			if ( m_jsJoyStatus[iJoyNum].nStatus[i] != 0 )
			{
				m_jsJoyStatus[iJoyNum].nStatus[i] = 0 ;
				iev.iKeyNum = i ;
				ProcessEvent( iev, false ) ;
			}
		}
		return ;
	}
	JOYINFOEX	jix ;
	jix.dwSize = sizeof(jix) ;
	jix.dwFlags = JOY_RETURNALL ;
	if ( ::joyGetPosEx( nJoyIDs[iJoyNum], &jix ) != JOYERR_NOERROR )
	{
		return ;
	}
	//
	// 座標取得
	//
	m_vJoyPos[iJoyNum].x =
		(REAL32) (jix.dwXpos - m_jcJoyCaps[iJoyNum].wXmin) * 2
			/ (REAL32) (m_jcJoyCaps[iJoyNum].wXmax
						- m_jcJoyCaps[iJoyNum].wXmin) - 1.0F ;
	m_vJoyPos[iJoyNum].y =
		(REAL32) (jix.dwYpos - m_jcJoyCaps[iJoyNum].wYmin) * 2
			/ (REAL32) (m_jcJoyCaps[iJoyNum].wYmax
						- m_jcJoyCaps[iJoyNum].wYmin) - 1.0F ;
	if ( m_jcJoyCaps[iJoyNum].wCaps & JOYCAPS_HASZ )
	{
		m_vJoyPos[iJoyNum].z =
			(REAL32) (jix.dwZpos - m_jcJoyCaps[iJoyNum].wZmin) * 2
				/ (REAL32) (m_jcJoyCaps[iJoyNum].wZmax
							- m_jcJoyCaps[iJoyNum].wZmin) - 1.0F ;
	}
	else
	{
		m_vJoyPos[iJoyNum].z = 0 ;
	}
	if ( m_jcJoyCaps[iJoyNum].wCaps & JOYCAPS_HASR )
	{
		m_vJoyPos[iJoyNum].d =
			(REAL32) (jix.dwRpos - m_jcJoyCaps[iJoyNum].wRmin) * 2
				/ (REAL32) (m_jcJoyCaps[iJoyNum].wRmax
							- m_jcJoyCaps[iJoyNum].wRmin) - 1.0F ;
	}
	else
	{
		m_vJoyPos[iJoyNum].d = 0 ;
	}
	//
	// 方向キー状態設定
	//
	DWORD	dwButtons = jix.dwButtons << 4 ;
	if ( m_vJoyPos[iJoyNum].x < -0.125 )
	{
		dwButtons |= 1 << jbLeft ;
	}
	if ( m_vJoyPos[iJoyNum].x > 0.125 )
	{
		dwButtons |= 1 << jbRight ;
	}
	if ( m_vJoyPos[iJoyNum].y < -0.125 )
	{
		dwButtons |= 1 << jbUp ;
	}
	if ( m_vJoyPos[iJoyNum].y > 0.125 )
	{
		dwButtons |= 1 << jbDown ;
	}
	//
	// POV から方向キーの状態設定
	//
	if ( (jix.dwFlags & JOY_RETURNPOV) && (jix.dwPOV != JOY_POVCENTERED) )
	{
		if ( (jix.dwPOV < JOY_POVRIGHT) || (jix.dwPOV > JOY_POVLEFT) )
		{
			dwButtons |= 1 << jbUp ;
		}
		if ( (jix.dwPOV > JOY_POVRIGHT) && (jix.dwPOV < JOY_POVLEFT) )
		{
			dwButtons |= 1 << jbDown ;
		}
		if ( (jix.dwPOV > JOY_POVFORWARD) && (jix.dwPOV < JOY_POVBACKWARD) )
		{
			dwButtons |= 1 << jbRight ;
		}
		if ( jix.dwPOV > JOY_POVBACKWARD )
		{
			dwButtons |= 1 << jbLeft ;
		}
	}
	//
	// ボタン状態設定
	//
	DWORD	dwMask = 0x00000001 ;
	for ( int i = 0; i < 32; i ++ )
	{
		INPUT_EVENT	iev ;
		iev.idType = idJoyStick ;
		iev.iDevNum = iJoyNum ;
		iev.iKeyNum = i ;
		if ( dwButtons & dwMask )
		{
			if ( m_jsJoyStatus[iJoyNum].nStatus[iev.iKeyNum] == 0 )
			{
				m_jsJoyStatus[iJoyNum].nStatus[iev.iKeyNum] = bmPushing ;
				ProcessEvent( iev, true ) ;
			}
		}
		else
		{
			if ( m_jsJoyStatus[iJoyNum].nStatus[iev.iKeyNum] != 0 )
			{
				m_jsJoyStatus[iJoyNum].nStatus[iev.iKeyNum] = 0 ;
				ProcessEvent( iev, false ) ;
			}
		}
		dwMask <<= 1 ;
	}
}

// イベントを処理
//////////////////////////////////////////////////////////////////////////////
bool EInputFilter::ProcessEvent
	( const EInputFilter::INPUT_EVENT & ieInput, bool fPushed )
{
	INPUT_EVENT		ieOutput ;
	INPUT_EVENT *	piev = GetFilter( ieInput ) ;
	if ( piev != NULL )
	{
		for ( int i = 0; i < 16; i ++ )
		{
			INPUT_EVENT *	pievFilter = GetFilter( *piev ) ;
			if ( pievFilter == NULL )
				break ;
			piev = pievFilter ;
		}
		ieOutput = *piev ;
	}
	else
	{
		ieOutput = ieInput ;
	}
	if ( fPushed )
	{
		PushInputEvent( ieOutput ) ;
	}
	if ( (ieOutput.idType == idJoyStick)
		&& (ieOutput.iKeyNum >= 0) && (ieOutput.iKeyNum < jbButtonMax) )
	{
		BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( ieOutput.iDevNum ) ;
		if ( pbs != NULL )
		{
			if ( fPushed )
			{
				pbs->nStatus[ieOutput.iKeyNum] = bmPushing |
					((pbs->nStatus[ieOutput.iKeyNum] + 1) & bmPushedMask) ;
			}
			else
			{
				pbs->nStatus[ieOutput.iKeyNum] &= bmPushedMask ;
			}
		}
	}
	if ( (piev != NULL) && (piev->idType == idKeyboard)
		&& (m_dwFlags & fmStickKey) && (GetFilter(*piev) == NULL) )
	{
		if ( m_pAttachedWnd != NULL )
		{
			if ( fPushed )
			{
				m_pAttachedWnd->PostMessage( WM_KEYDOWN, piev->iKeyNum, 0 ) ;
			}
			else
			{
				m_pAttachedWnd->PostMessage( WM_KEYUP, piev->iKeyNum, 0 ) ;
			}
		}
	}
	if ( (ieOutput.idType == idCommand)
			|| (ieOutput.idType == idSignalCommand) )
	{
		if ( m_pAttachedItf != NULL )
		{
			if ( fPushed )
			{
				m_pAttachedItf->QueueCommand
					( ieOutput.wstrCommand, 0, 0,
						ieOutput.nPriority,
						(ieOutput.idType == idSignalCommand) ) ;
			}
		}
	}
	return	(piev != NULL) ;
}

// 現在の特殊キー押下状態マスクを取得
//////////////////////////////////////////////////////////////////////////////
DWORD EInputFilter::QueryCurrentContextKeyMask( void )
{
	DWORD	dwMask = 0 ;
	if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
	{
		dwMask |= ckmShift ;
	}
	if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
	{
		dwMask |= ckmControl ;
	}
	if ( ::GetKeyState( VK_MENU ) & 0x80 )
	{
		dwMask |= ckmMenu ;
	}
	if ( ::GetKeyState( VK_CAPITAL ) & 0x01 )
	{
		dwMask |= ckmCaptal ;
	}
	return	dwMask ;
}

// EWindow オブジェクトにアタッチされた
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::OnAttachedWindow( EWindow * pAttachedWnd )
{
	m_pAttachedWnd = pAttachedWnd ;
	//
	if ( m_pAttachedWnd != NULL )
	{
		if ( (m_pAttachedWnd->m_hWnd != NULL)
				&& ::IsWindow( *m_pAttachedWnd ) )
		{
			BeginFilter( ) ;
		}
	}
}

// EWindow オブジェクトにデタッチされた
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::OnDetachedWindow( EWindow * pDetachedWnd )
{
	EndFilter( ) ;
	m_pAttachedWnd = NULL ;
	m_pAttachedItf = NULL ;
}

// ウィンドウ取得
//////////////////////////////////////////////////////////////////////////////
EWindow * EInputFilter::GetWindow( void ) const
{
	return	m_pAttachedWnd ;
}

EWindowSpriteInterface * EInputFilter::GetWindowInterface( void ) const
{
	return	m_pAttachedItf ;
}

// フィルターフラグを設定
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::SetFilterFlags( DWORD dwFlags )
{
	m_dwFlags = dwFlags ;
}

// ジョイスティックの閾値を設定する
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::SetStickThreshold( REAL32 rThreshold )
{
	m_rJoyThreshold = rThreshold ;
	//
	UINT	nThreshold ;
	if ( m_fJoyCaptured & 0x01 )
	{
		nThreshold =
			(UINT) (rThreshold * (REAL32)
				(m_jcJoyCaps[0].wXmax - m_jcJoyCaps[0].wXmin) * 0.5F) ;
		::joySetThreshold( JOYSTICKID1, nThreshold ) ;
	}
	if ( m_fJoyCaptured & 0x02 )
	{
		nThreshold =
			(UINT) (rThreshold * (REAL32)
				(m_jcJoyCaps[1].wXmax - m_jcJoyCaps[1].wXmin) * 0.5F) ;
		::joySetThreshold( JOYSTICKID2, nThreshold ) ;
	}
}

// マウス入力からスティック入力への変換を設定する
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::SetMouseThreshold( EGL_POINT ptBase, int nThreshold )
{
	m_dwFlags |= fmJoyMouse ;
	m_ptMouseBase = ptBase ;
	m_nMouseThreshold = nThreshold ;
}

// 仮想ジョイスティックの総数を設定
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::SetJoyStickCount( int nJoyCount )
{
	m_lstVirtPad.RemoveAll( ) ;
	//
	for ( int i = 0; i < nJoyCount; i ++ )
	{
		BUTTON_STATUS *	pbs = new BUTTON_STATUS ;
		::eslFillMemory( pbs->nStatus, 0, sizeof(pbs->nStatus) ) ;
		m_lstVirtPad.Add( pbs ) ;
	}
	return	eslErrSuccess ;
}

// インストールされているジョイスティックの数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EInputFilter::GetInstalledJoyCount( void )
{
	return	::joyGetNumDevs( ) ;
}

// フィルター処理開始
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::OpenFilter( EWindow * pWnd )
{
	if ( (pWnd != NULL) && (m_pAttachedWnd != pWnd) )
	{
		pWnd->SetInputFilter( this ) ;
	}
	return	eslErrSuccess ;
}

ESLError EInputFilter::OpenFilter( EWindowSpriteInterface * pItf )
{
	if ( pItf != NULL )
	{
		EWindow *	pWnd = pItf->GetWindow( ) ;
		if ( (pWnd != NULL) && (m_pAttachedWnd != pWnd) )
		{
			m_pAttachedItf = pItf ;
			pItf->SetInputFilter( this ) ;
			pWnd->SetInputFilter( this ) ;
		}
	}
	return	eslErrSuccess ;
}

// フィルター処理終了
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::CloseFilter( void )
{
	if ( m_pAttachedItf != NULL )
	{
		m_pAttachedItf->SetInputFilter( NULL ) ;
		//
		EWindow *	pWnd = m_pAttachedItf->GetWindow( ) ;
		if ( pWnd != NULL )
		{
			pWnd->SetInputFilter( NULL ) ;
		}
	}
	else if ( m_pAttachedWnd != NULL )
	{
		m_pAttachedWnd->SetInputFilter( NULL ) ;
	}
	m_pAttachedItf = NULL ;
	m_pAttachedWnd = NULL ;
	return	eslErrSuccess ;
}

// フィルター開始処理
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::BeginFilter( void )
{
	if ( (m_pAttachedWnd == NULL) || !::IsWindow( *m_pAttachedWnd ) )
	{
		return	eslErrGeneral ;
	}
	HWND	hwnd = *m_pAttachedWnd ;
	DWORD	dwProcessId ;
	if ( GetCurrentThreadId()
		!= ::GetWindowThreadProcessId( hwnd, &dwProcessId ) )
	{
		m_pAttachedWnd->PostMessage( m_nBeginFilter, 0, 0 ) ;
		return	eslErrSuccess ;
	}
	//
	// 入力待ち行列初期化
	//
	m_queInputEvent.RemoveAll( ) ;
	::ResetEvent( m_hInputEvent ) ;
	//
	// ジョイスティック初期化
	//
	if ( !(m_fJoyCaptured & 0x03) && (GetInstalledJoyCount() > 0) )
	{
		::joyGetDevCaps( JOYSTICKID1, &m_jcJoyCaps[0], sizeof(JOYCAPS) ) ;
		::joyGetDevCaps( JOYSTICKID2, &m_jcJoyCaps[1], sizeof(JOYCAPS) ) ;
		//
		HWND	hWnd = *m_pAttachedWnd ;
		m_fJoyCaptured &= ~0x03 ;
		if ( ::joySetCapture
				( hWnd, JOYSTICKID1, 15, FALSE ) == JOYERR_NOERROR )
		{
			m_fJoyCaptured |= 0x01 ;
		}
		if ( ::joySetCapture
				( hWnd, JOYSTICKID2, 15, FALSE ) == JOYERR_NOERROR )
		{
			m_fJoyCaptured |= 0x02 ;
		}
		if ( m_fJoyCaptured )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				m_vJoyPos[i].x = 0 ;
				m_vJoyPos[i].y = 0 ;
				m_vJoyPos[i].z = 0 ;
				//
				::eslFillMemory( &m_jsJoyStatus[i], 0, sizeof(BUTTON_STATUS) ) ;
			}
			SetStickThreshold( m_rJoyThreshold ) ;
		}
	}
	for ( int iXInput = 0; iXInput < deviceXInputCount; iXInput ++ )
	{
		const int	iJoy = joyStickXInput1 + iXInput ;
		eslFillMemory( &m_jsJoyStatus[iJoy], 0, sizeof(BUTTON_STATUS) ) ;
		PollXInputState
			( m_vJoyPos[iJoy], m_jsJoyStatus[iJoy], iXInput, false ) ;
	}
	//
	// マウスの基準座標設定
	//
	if ( m_dwFlags & fmJoyMouse )
	{
		POINT	ptCursor ;
		ptCursor.x = m_ptMouseBase.x ;
		ptCursor.y = m_ptMouseBase.y ;
		::ClientToScreen( *m_pAttachedWnd, &ptCursor ) ;
		::SetCursorPos( ptCursor.x, ptCursor.y ) ;
	}
	//
	m_dwContextKeyMask = QueryCurrentContextKeyMask( ) ;
	//
	return	eslErrSuccess ;
}

// フィルター終了処理
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::EndFilter( void )
{
	if ( m_fJoyCaptured )
	{
//		if ( m_fJoyCaptured & 0x01 )
//			::joyReleaseCapture( JOYSTICKID1 ) ;
//		if ( m_fJoyCaptured & 0x02 )
//			::joyReleaseCapture( JOYSTICKID2 ) ;
		m_fJoyCaptured = 0 ;
	}
	return	eslErrSuccess ;
}

// XInput ポーリング
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::PollXInputState
	( E3D_VECTOR4& vPos,
		EInputFilter::BUTTON_STATUS& bsState,
		size_t iXInput, bool fProcessEvent )
{
	DWORD	dwResult =
				XInputGetState( (DWORD) iXInput, &m_xinState[iXInput] ) ;
	if ( dwResult != ERROR_SUCCESS )
	{
		m_maskXInputDev &= ~(1 << iXInput) ;
		return ;
	}
	m_maskXInputDev |= (1 << iXInput) ;
	//
	const XINPUT_STATE	xis = m_xinState[iXInput] ;
	//
	// 方向キー
	//
	if ( (xis.Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
	{
		vPos.x = (float32_t) xis.Gamepad.sThumbLX / 0x8000 ;
	}
	else
	{
		vPos.x = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
	{
		vPos.y = - (float32_t) xis.Gamepad.sThumbLY / 0x8000 ;
	}
	else
	{
		vPos.y = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
	{
		vPos.z = (float32_t) xis.Gamepad.sThumbRX / 0x8000 ;
	}
	else
	{
		vPos.z = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
	{
		vPos.d = - (float32_t) xis.Gamepad.sThumbRY / 0x8000 ;
	}
	else
	{
		vPos.d = 0.0f ;
	}
	//
	// ボタン状態
	//
	INPUT_EVENT	iev ;
	iev.idType = idJoyStick ;
	iev.iDevNum = joyStickXInput1 + iXInput ;
	//
	DWORD	dwMask = 0x00000001 ;
	for ( int i = 0; i < 16; i ++ )
	{
		iev.iKeyNum = i ;
		if ( xis.Gamepad.wButtons & dwMask )
		{
			if ( bsState.nStatus[iev.iKeyNum] == 0 )
			{
				bsState.nStatus[iev.iKeyNum] = bmPushing ;
				if ( fProcessEvent )
				{
					ProcessEvent( iev, true ) ;
				}
			}
		}
		else
		{
			if ( bsState.nStatus[iev.iKeyNum] != 0 )
			{
				bsState.nStatus[iev.iKeyNum] = 0 ;
				if ( fProcessEvent )
				{
					ProcessEvent( iev, false ) ;
				}
			}
		}
		dwMask <<= 1 ;
	}
}

// フィルタを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::LoadInputFilter( EDescription & dscFilter )
{
	static const wchar_t *	pwszFlagName[] =
	{
		L"normal", L"joy_mouse", L"stick_mouse",
		L"stick_key", L"transparent", L"context_key", NULL
	} ;
	static const DWORD	dwFlagValue[] =
	{
		fmNormal, fmJoyMouse, fmStickMouse,
		fmStickKey, fmTransparent, fmContextKey
	} ;
	//
	// 現在のデータを削除する
	//
	DeleteInputFilter( ) ;
	//
	// フィルタタグを取得する
	//
	EDescription *	pTag = dscFilter.GetContentTagAs( 0, L"filter" ) ;
	if ( pTag == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwFlags = 0 ;
	EStreamWideString	swsFlags = pTag->GetAttrString( L"flags", NULL ) ;
	while ( !swsFlags.DisregardSpace() )
	{
		EWideString	wstrToken = swsFlags.GetAToken( ) ;
		for ( int i = 0; pwszFlagName[i] != NULL; i ++ )
		{
			if ( wstrToken == pwszFlagName[i] )
			{
				dwFlags |= dwFlagValue[i] ;
				break ;
			}
		}
	}
	SetFilterFlags( dwFlags ) ;
	//
	if ( m_dwFlags & fmJoyMouse )
	{
		EGL_POINT	ptBase ;
		ptBase.x = pTag->GetAttrInteger( L"mouse_base_x", 320 ) ;
		ptBase.y = pTag->GetAttrInteger( L"mouse_base_y", 160 ) ;
		SetMouseThreshold
			( ptBase, pTag->GetAttrInteger( L"mouse_threshold", 16 ) ) ;
	}
	SetStickThreshold
		( (REAL32) pTag->GetAttrReal( L"joystick_threshold", 0.01 ) ) ;
	SetJoyStickCount( pTag->GetAttrInteger( L"joystick_count", 6 ) ) ;
	//
	// フィルタを順次追加
	//
	for ( int i = 0; i < pTag->GetContentTagCount(); i ++ )
	{
		EDescription *	pFilter = pTag->GetContentTagAt( i ) ;
		if ( pFilter == NULL )
			continue ;
		if ( pFilter->Tag() != L"key_assign" )
			continue ;
		//
		EDescription *	pInput = pFilter->GetContentTagAs( 0, L"input" ) ;
		EDescription *	pOutput = pFilter->GetContentTagAs( 0, L"output" ) ;
		if ( (pInput != NULL) && (pOutput != NULL) )
		{
			INPUT_EVENT	ieInput, ieOutput ;
			LoadInputEvent( ieInput, *pInput ) ;
			LoadInputEvent( ieOutput, *pOutput ) ;
			AddFilter( ieInput, ieOutput ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// フィルタを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::SaveInputFilter( EDescription & dscFilter )
{
	static const wchar_t *	pwszFlagName[] =
	{
		L"normal", L"joy_mouse", L"stick_mouse",
		L"stick_key", L"transparent", L"context_key", NULL
	} ;
	static const DWORD	dwFlagValue[] =
	{
		fmNormal, fmJoyMouse, fmStickMouse,
		fmStickKey, fmTransparent, fmContextKey
	} ;
	//
	// フィルタタグを設定する
	//
	EDescription *	pTag = new EDescription ;
	pTag->SetTag( L"filter" ) ;
	dscFilter.AddContentTag( pTag ) ;
	//
	int			i ;
	EWideString	wstrFlags ;
	for ( i = 0; pwszFlagName[i] != NULL; i ++ )
	{
		if ( m_dwFlags & dwFlagValue[i] )
		{
			if ( !wstrFlags.IsEmpty() )
				wstrFlags += L" " ;
			wstrFlags += pwszFlagName[i] ;
		}
	}
	if ( wstrFlags.IsEmpty() )
	{
		wstrFlags = L"normal" ;
	}
	pTag->SetAttrString( L"flags", wstrFlags ) ;
	//
	if ( m_dwFlags & fmJoyMouse )
	{
		pTag->SetAttrInteger( L"mouse_base_x", m_ptMouseBase.x ) ;
		pTag->SetAttrInteger( L"mouse_base_y", m_ptMouseBase.y ) ;
		pTag->SetAttrInteger( L"mouse_threshold", m_nMouseThreshold ) ;
	}
	pTag->SetAttrReal( L"joystick_threshold", m_rJoyThreshold ) ;
	pTag->SetAttrInteger( L"joystick_count", m_lstVirtPad.GetSize() ) ;
	//
	// フィルタを順次追加
	//
	for ( i = 0; i < (int) m_tsaFilter.GetSize(); i ++ )
	{
		ETaggedElement<INPUT_EVENT,INPUT_EVENT> *	pElement ;
		pElement = m_tsaFilter.GetAt( i ) ;
		if ( (pElement == NULL) || (pElement->GetObject() == NULL) )
			continue ;
		//
		EDescription *	pFilter = new EDescription ;
		pFilter->SetTag( L"key_assign" ) ;
		pTag->AddContentTag( pFilter ) ;
		//
		EDescription *	pInput = new EDescription ;
		EDescription *	pOutput = new EDescription ;
		pInput->SetTag( L"input" ) ;
		pOutput->SetTag( L"output" ) ;
		pFilter->AddContentTag( pInput ) ;
		pFilter->AddContentTag( pOutput ) ;
		SaveInputEvent( *pInput, pElement->Tag() ) ;
		SaveInputEvent( *pOutput, *(pElement->GetObject()) ) ;
	}
	//
	return	eslErrSuccess ;
}

// フィルタの内容を初期化する
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::DeleteInputFilter( void )
{
	CloseFilter( ) ;
	m_tsaFilter.RemoveAll( ) ;
}

// 入力イベントをタグから読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::LoadInputEvent
	( EInputFilter::INPUT_EVENT & ieInput, EDescription & dscEvent )
{
	static const wchar_t *	pwszDeviceName[] =
	{
		L"keyboard", L"mouse", L"joystick", L"command", L"signal", NULL
	} ;
	static const wchar_t *	pwszKeyName[] =
	{
		L"up", L"down", L"left", L"right", NULL
	} ;
	static const int	nVirtKeyValue[] =
	{
		VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT
	} ;
	int		i ;
	EWideString	wstrDevice = dscEvent.GetAttrString( L"device", NULL ) ;
	ieInput.idType = idKeyboard ;
	for ( i = 0; pwszDeviceName[i] != NULL; i ++ )
	{
		if ( wstrDevice == pwszDeviceName[i] )
		{
			ieInput.idType = (InputDevice) i ;
			break ;
		}
	}
	ieInput.iDevNum = dscEvent.GetAttrInteger( L"device_number", 0 ) ;
	//
	EStreamWideString	swsKey = dscEvent.GetAttrString( L"key", 0 ) ;
	EWideString	wstrToken = swsKey.GetAToken( ) ;
	for ( i = 0; pwszKeyName[i] != NULL; i ++ )
	{
		if ( wstrToken == pwszKeyName[i] )
		{
			if ( ieInput.idType == idJoyStick )
			{
				ieInput.iKeyNum = i ;
			}
			else
			{
				ieInput.iKeyNum = nVirtKeyValue[i] ;
			}
			return	eslErrSuccess ;
		}
	}
	if ( wstrToken == L"button" )
	{
		swsKey.HasToComeChar( L":" ) ;
		ieInput.iKeyNum = swsKey.GetInteger() + jbButton1 ;
	}
	else if ( wstrToken == L"code" )
	{
		swsKey.HasToComeChar( L":" ) ;
		ieInput.iKeyNum = swsKey.GetInteger() ;
	}
	else if ( wstrToken == L"ascii" )
	{
		swsKey.HasToComeChar( L":" ) ;
		ieInput.iKeyNum = swsKey.GetCharacter() ;
	}
	else if ( (ieInput.idType != idCommand)
				&& (ieInput.idType != idSignalCommand) )
	{
		ESLTrace( "キー指定が不正です。\n" ) ;
		return	eslErrGeneral ;
	}
	//
	ieInput.wstrCommand = dscEvent.GetAttrString( L"command", NULL ) ;
	ieInput.nPriority = (ieInput.idType != idSignalCommand) ? 0 : 1 ;
	ieInput.nPriority =
		dscEvent.GetAttrInteger( L"cmd_priority", ieInput.nPriority ) ;
	//
	return	eslErrSuccess ;
}

// 入力イベントをタグへ設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::SaveInputEvent
	( EDescription & dscEvent, const EInputFilter::INPUT_EVENT & ieInput )
{
	static const wchar_t *	pwszDeviceName[] =
	{
		L"keyboard", L"mouse", L"joystick", L"command", L"signal", NULL
	} ;
	static const wchar_t *	pwszKeyName[] =
	{
		L"up", L"down", L"left", L"right", NULL
	} ;
	static const int	nVirtKeyValue[] =
	{
		VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT
	} ;
	//
	dscEvent.SetAttrString( L"device", pwszDeviceName[ieInput.idType] ) ;
	dscEvent.SetAttrInteger( L"device_number", ieInput.iDevNum ) ;
	//
	if ( ieInput.idType == idJoyStick )
	{
		if ( ieInput.iKeyNum < jbButton1 )
		{
			dscEvent.SetAttrString( L"key", pwszKeyName[ieInput.iKeyNum] ) ;
			return	eslErrSuccess ;
		}
		dscEvent.SetAttrString
			( L"key", L"button:" + EWideString(ieInput.iKeyNum - jbButton1) ) ;
	}
	else
	{
		for ( int i = 0; i < 4; i ++ )
		{
			if ( ieInput.iKeyNum == nVirtKeyValue[i] )
			{
				dscEvent.SetAttrString( L"key", pwszKeyName[i] ) ;
				return	eslErrSuccess ;
			}
		}
		if ( (ieInput.iKeyNum >= 'A') && (ieInput.iKeyNum <= 'Z') )
		{
			dscEvent.SetAttrString
				( L"key", L"ascii:" + EWideString((wchar_t)ieInput.iKeyNum) ) ;
		}
		else
		{
			dscEvent.SetAttrString
				( L"key", L"code:" + EWideString((int)ieInput.iKeyNum) ) ;
		}
	}
	//
	if ( !ieInput.wstrCommand.IsEmpty() )
	{
		dscEvent.SetAttrString( L"command", ieInput.wstrCommand ) ;
	}
	if ( (ieInput.idType == idCommand)
		|| (ieInput.idType == idSignalCommand) )
	{
		dscEvent.SetAttrInteger( L"cmd_priority", ieInput.nPriority ) ;
	}
	//
	return	eslErrSuccess ;
}

// フィルタ追加
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::AddFilter
	( const EInputFilter::INPUT_EVENT & ieInput,
		const EInputFilter::INPUT_EVENT & ieOutput )
{
	Lock( ) ;
	m_tsaFilter.SetAs( ieInput, new INPUT_EVENT(ieOutput) ) ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// フィルタ削除
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::RemoveFilter
	( const EInputFilter::INPUT_EVENT & ieInput )
{
	Lock( ) ;
	m_tsaFilter.RemoveAs( ieInput ) ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// フィルタ取得
//////////////////////////////////////////////////////////////////////////////
EInputFilter::INPUT_EVENT *
	EInputFilter::GetFilter( const EInputFilter::INPUT_EVENT & ieInput )
{
	INPUT_EVENT *	pEvent ;
	Lock( ) ;
	pEvent = m_tsaFilter.GetAs( ieInput ) ;
	Unlock( ) ;
	return	pEvent ;
}

// フィルタ列挙
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::EnumFilter
	( EObjArray<EInputFilter::INPUT_EVENT> & lstInput,
			const EInputFilter::INPUT_EVENT & ieOutput )
{
	unsigned int	i, nCount ;
	Lock( ) ;
	nCount = m_tsaFilter.GetSize( ) ;
	lstInput.RemoveAll( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<INPUT_EVENT,INPUT_EVENT> *	pElement ;
		pElement = m_tsaFilter.GetAt( i ) ;
		if ( pElement == NULL )
			continue ;
		if ( pElement->GetObject() == NULL )
			continue ;
		if ( pElement->GetObject()->Compare( ieOutput ) )
			continue ;
		lstInput.Add( new INPUT_EVENT( pElement->Tag() ) ) ;
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 入力イベント追加
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::PushInputEvent
	( const EInputFilter::INPUT_EVENT & ieInput )
{
	if ( m_nInputLimit > 0 )
	{
		Lock( ) ;
		if ( m_queInputEvent.GetSize() >= m_nInputLimit )
		{
			m_queInputEvent.RemoveAt( 0 ) ;
		}
		m_queInputEvent.Add( new INPUT_EVENT( ieInput ) ) ;
		::SetEvent( m_hInputEvent ) ;
		Unlock( ) ;
	}
	return	eslErrSuccess ;
}

// 入力イベントを取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::GetInputEvent
	( EInputFilter::INPUT_EVENT & ieEvent, DWORD dwTimeout, bool fEscMsg )
{
	DWORD	dwWaitResult ;
	INPUT_EVENT *	piev = NULL ;
	if ( fEscMsg )
	{
		dwWaitResult = ::MsgWaitForMultipleObjects
			( 1, &m_hInputEvent, FALSE, dwTimeout, QS_ALLINPUT ) ;
	}
	else
	{
		dwWaitResult = ::WaitForSingleObject( m_hInputEvent, dwTimeout ) ;
	}
	if ( dwWaitResult == WAIT_OBJECT_0 )
	{
		Lock( ) ;
		piev = m_queInputEvent.GetAt( 0 ) ;
		m_queInputEvent.DetachAt( 0 ) ;
		if ( m_queInputEvent.GetSize() == 0 )
		{
			::ResetEvent( m_hInputEvent ) ;
		}
		Unlock( ) ;
	}
	if ( piev != NULL )
	{
		ieEvent = *piev ;
		delete	piev ;
		return	eslErrSuccess ;
	}
	return	eslErrTimeout ;
}

// 入力イベント待ち行列を初期化
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::FlushInputQueue( int nLimit )
{
	Lock( ) ;
	m_nInputLimit = nLimit ;
	m_queInputEvent.RemoveAll( ) ;
	::ResetEvent( m_hInputEvent ) ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 現在のスティック座標を取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::GetStickPosition( E3D_VECTOR4 & vPos, int iDevNum )
{
	vPos.x = 0 ;
	vPos.y = 0 ;
	vPos.z = 0 ;
	vPos.d = 0 ;
	if ( (m_fJoyCaptured & (1 << iDevNum))
			&& (iDevNum >= joyStickId1) && (iDevNum <= joyStickId2) )
	{
		vPos = m_vJoyPos[iDevNum] ;
	}
	else if ( (iDevNum >= joyStickXInput1)
			&& (iDevNum <= joyStickXInput4)
			&& (m_maskXInputDev & (1 << (iDevNum - joyStickXInput1))) )
	{
		vPos = m_vJoyPos[iDevNum] ;
	}
	else
	{
		BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( iDevNum ) ;
		if ( pbs != NULL )
		{
			if ( pbs->nStatus[jbUp] & bmPushing )
				vPos.y -= 1.0F ;
			if ( pbs->nStatus[jbDown] & bmPushing )
				vPos.y += 1.0F ;
			if ( pbs->nStatus[jbLeft] & bmPushing )
				vPos.x -= 1.0F ;
			if ( pbs->nStatus[jbRight] & bmPushing )
				vPos.x += 1.0F ;
		}
	}
	return	eslErrSuccess ;
}

ESLError EInputFilter::GetStickPosition( E3D_VECTOR & vPos, int iDevNum )
{
	E3D_VECTOR4	vPos4 ;
	ESLError	err = GetStickPosition( vPos4, iDevNum ) ;
	if ( !err )
	{
		vPos = vPos4 ;
	}
	return	err ;
}

// 仮想ジョイスティックのボタンの現在の状態を取得
//////////////////////////////////////////////////////////////////////////////
bool EInputFilter::IsJoyButtonPushing( int iKeyNum, int iDevNum )
{
	BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( iDevNum ) ;
	if ( pbs != NULL )
	{
		if ( (iKeyNum >= 0) && (iKeyNum < jbButtonMax) )
		{
			return	((pbs->nStatus[iKeyNum] & bmPushing) != 0) ;
		}
	}
	return	false ;
}

// 仮想ジョイスティックのボタンが押された回数を取得
//////////////////////////////////////////////////////////////////////////////
int EInputFilter::GetJoyButtonPushed( int iKeyNum, int iDevNum )
{
	BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( iDevNum ) ;
	if ( pbs != NULL )
	{
		if ( (iKeyNum >= 0) && (iKeyNum < jbButtonMax) )
		{
			return	(pbs->nStatus[iKeyNum] & bmPushedMask) ;
		}
	}
	return	0 ;
}

// 仮想ジョイスティックのボタンの押下回数をリセット
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::FlushJoyButtonPushed( int iDevNum, int iKeyNum )
{
	BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( iDevNum ) ;
	if ( pbs != NULL )
	{
		if ( (unsigned int) iKeyNum >= jbButtonMax )
		{
			for ( int i = 0; i < jbButtonMax; i ++ )
			{
				pbs->nStatus[i] &= bmPushing ;
			}
		}
		else
		{
			pbs->nStatus[iKeyNum] &= bmPushing ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 仮想ジョイスティックのボタンの押下状態をリセット
//////////////////////////////////////////////////////////////////////////////
ESLError EInputFilter::ResetJoyButtonPushing( int iDevNum, int iKeyNum )
{
	BUTTON_STATUS *	pbs = m_lstVirtPad.GetAt( iDevNum ) ;
	if ( pbs != NULL )
	{
		if ( (unsigned int) iKeyNum >= jbButtonMax )
		{
			for ( int i = 0; i < jbButtonMax; i ++ )
			{
				pbs->nStatus[i] &= bmPushedMask ;
			}
		}
		else
		{
			pbs->nStatus[iKeyNum] &= bmPushedMask ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void EInputFilter::Lock( void )
{
	::EnterCriticalSection( &m_cs ) ;
}

void EInputFilter::Unlock( void )
{
	::LeaveCriticalSection( &m_cs ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウユーザーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWindowInterface, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWindowInterface::EWindowInterface( void )
	: m_pAttachedWnd( NULL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWindowInterface::~EWindowInterface( void )
{
	if ( m_pAttachedWnd )
	{
		m_pAttachedWnd->SetInterface( NULL ) ;
	}
}

// EWindow オブジェクトにアタッチされた
//////////////////////////////////////////////////////////////////////////////
void EWindowInterface::OnAttachedWindow( EWindow * pAttachedWnd )
{
	m_pAttachedWnd = pAttachedWnd ;
}

// EWindow オブジェクトにデタッチされた
//////////////////////////////////////////////////////////////////////////////
void EWindowInterface::OnDetachedWindow( EWindow * pDetachedWnd )
{
	m_pAttachedWnd = NULL ;
}

// ウィンドウ取得
//////////////////////////////////////////////////////////////////////////////
EWindow * EWindowInterface::GetWindow( void ) const
{
	return	m_pAttachedWnd ;
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT EWindowInterface::WindowProc
	( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return	pWnd->DefWindowProc( uMsg, wParam, lParam ) ;
}

// メッセージ事前変換関数
//////////////////////////////////////////////////////////////////////////////
int EWindowInterface::PreTranslateMessage( MSG & msg )
{
	return	0 ;
}


//////////////////////////////////////////////////////////////////////////////
// スレッドオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLSThread, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLSThread::EGLSThread( void )
{
	m_nTimeRange = 0 ;
	m_nTotalTime = 0 ;
}

// スレッド開始時
//////////////////////////////////////////////////////////////////////////////
void EGLSThread::OnBeginThread( void )
{
	::glsInitializeTask() ;
	m_dwThreadTime = ::timeGetTime() ;
}

// スレッド終了時
//////////////////////////////////////////////////////////////////////////////
void EGLSThread::OnEndThread( void )
{
	::glsCloseTask() ;
}

// 時間計測
//////////////////////////////////////////////////////////////////////////////
DWORD EGLSThread::GetThreadTime( void ) const
{
	return	::timeGetTime() - m_dwThreadTime ;
}

// 局所時間計測開始
//////////////////////////////////////////////////////////////////////////////
void EGLSThread::BeginTime( int nTotalTime, int nRange )
{
	m_dwBeginTime = GetThreadTime( ) ;
	m_nTimeRange = nRange ;
	m_nTotalTime = nTotalTime ;
}

// 局所時間計測
//////////////////////////////////////////////////////////////////////////////
int EGLSThread::GetOffsetTime( void ) const
{
	if ( m_nTotalTime <= 0 )
	{
		return	0 ;
	}
	int		nTime = (int) (GetThreadTime() - m_dwBeginTime) ;
	if ( nTime < 0 )
	{
		return	0 ;
	}
	if ( nTime > m_nTotalTime )
	{
		nTime = m_nTotalTime ;
	}
	return	(int)((INT64) nTime * m_nTimeRange / m_nTotalTime) ;
}


//////////////////////////////////////////////////////////////////////////////
// ディスプレイモード
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EDisplayMode, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EDisplayMode::EDisplayMode( void )
	: m_fChanged( FALSE )
{
	m_hUser32 = NULL ;
	m_apiMonitorFromRect = NULL ;
	m_apiGetMonitorInfo = NULL ;
	m_apiChangeDisplaySettingsEx = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EDisplayMode::~EDisplayMode( void )
{
	if ( m_fChanged )
	{
		RestoreDisplayMode( ) ;
	}
	if ( m_hUser32 != NULL )
	{
		::FreeLibrary( m_hUser32 ) ;
	}
}

// user32.dll をロードして各種 API を初期化する
//////////////////////////////////////////////////////////////////////////////
void EDisplayMode::PrepareMonitorAPIs( void )
{
	if ( m_hUser32 == NULL )
	{
		m_hUser32 = ::LoadLibrary( "user32.dll" ) ;
		m_apiMonitorFromRect =
			(API_MonitorFromRect)
				::GetProcAddress( m_hUser32, "MonitorFromRect" ) ;
		m_apiGetMonitorInfo =
			(API_GetMonitorInfo)
				::GetProcAddress( m_hUser32, "GetMonitorInfoA" ) ;
		m_apiChangeDisplaySettingsEx =
			(API_ChangeDisplaySettingsEx)
				::GetProcAddress( m_hUser32, "ChangeDisplaySettingsExA" ) ;
	}
}

// ディスプレイを取得
//////////////////////////////////////////////////////////////////////////////
const char * EDisplayMode::GetDisplayNameFromRect
	( EString & strDisplayName, const RECT * pRect, HMONITOR * phMonitor )
{
	strDisplayName.FreeString( ) ;
	//
	PrepareMonitorAPIs() ;
	if ( (m_apiMonitorFromRect == NULL) || (m_apiGetMonitorInfo == NULL) )
	{
		return	NULL ;
	}
	HMONITOR	hMonitor = m_apiMonitorFromRect( pRect, MONITOR_DEFAULTTONEAREST ) ;
	if ( hMonitor == NULL )
	{
		return	NULL ;
	}
	MONITORINFOEX	mix ;
	mix.cbSize = sizeof(mix) ;
	if ( !m_apiGetMonitorInfo( hMonitor, &mix ) )
	{
		return	NULL ;
	}
	strDisplayName = mix.szDevice ;
	if ( phMonitor != NULL )
	{
		*phMonitor = hMonitor ;
	}
	return	strDisplayName ;
}

// ディスプレイの矩形を取得
//////////////////////////////////////////////////////////////////////////////
ESLError EDisplayMode::GetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpmi )
{
	PrepareMonitorAPIs() ;
	if ( m_apiGetMonitorInfo == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( m_apiGetMonitorInfo( hMonitor, lpmi ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ウィンドウ位置を正規化（画面内へ補正）
//////////////////////////////////////////////////////////////////////////////
void EDisplayMode::NormalizeWindowPos
	( POINT & posWindow, SIZE & sizeWindow, bool fChangeSize )
{
	RECT		rctMonitor ;
	EString		strDisplayName ;
	HMONITOR	hMonitor ;
	RECT		rctWindow ;
	//
	rctMonitor.left = 0 ;
	rctMonitor.top = 0 ;
	rctMonitor.right = ::GetSystemMetrics( SM_CXSCREEN ) ;
	rctMonitor.bottom = ::GetSystemMetrics( SM_CYSCREEN ) ;
	//
	rctWindow.left = posWindow.x ;
	rctWindow.top = posWindow.y ;
	rctWindow.right = posWindow.x + sizeWindow.cx ;
	rctWindow.bottom = posWindow.y + sizeWindow.cy ;
	//
	if ( GetDisplayNameFromRect
		( strDisplayName, &rctWindow, &hMonitor ) != NULL )
	{
		MONITORINFO	mi ;
		memset( &mi, 0, sizeof(mi) ) ;
		mi.cbSize = sizeof(mi) ;
		if ( !GetMonitorInfo( hMonitor, &mi ) )
		{
			rctMonitor = mi.rcMonitor ;
		}
	}
	//
	if ( posWindow.x + sizeWindow.cx > rctMonitor.right )
	{
		posWindow.x = rctMonitor.right - sizeWindow.cx ;
	}
	if ( posWindow.x < rctMonitor.left )
	{
		posWindow.x = rctMonitor.left ;
	}
	if ( fChangeSize
		&& (posWindow.x + sizeWindow.cx > rctMonitor.right) )
	{
		sizeWindow.cx = rctMonitor.right - posWindow.x ;
	}
	if ( posWindow.y + sizeWindow.cy > rctMonitor.bottom )
	{
		posWindow.y = rctMonitor.bottom - sizeWindow.cy ;
	}
	if ( posWindow.y < rctMonitor.top )
	{
		posWindow.y = rctMonitor.top ;
	}
	if ( fChangeSize
		&& (posWindow.y + sizeWindow.cy > rctMonitor.bottom) )
	{
		sizeWindow.cy = rctMonitor.bottom - posWindow.y ;
	}
}

// DirectDraw 用ディスプレイの GUID を取得する
//////////////////////////////////////////////////////////////////////////////
static BOOL WINAPI DDEnumCallback_GetDDMonitorGUID
	( GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm ) ;
struct	DDEnumCallback_GetDDMonitorGUID_Param
{
	bool		fFound ;
	int			nIndex ;
	HMONITOR	hMonitor ;
	GUID		guidFound ;
} ;

GUID * EDisplayMode::GetDDMonitorGUID
	( GUID * pGUID, HMONITOR hMonitor, int * pGetIndex )
{
	if ( pGetIndex != NULL )
	{
		*pGetIndex = 0 ;
	}
	typedef	HRESULT (WINAPI *API_DirectDrawEnumerateEx)
		( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags ) ;
	HMODULE	hModule = ::LoadLibrary( "ddraw.dll" ) ;
	if ( hModule == NULL )
	{
		return	NULL ;
	}
	GUID *	pRetGUID = NULL ;
	API_DirectDrawEnumerateEx
		apiDirectDrawEnumerateEx =
			(API_DirectDrawEnumerateEx) ::GetProcAddress
							( hModule, "DirectDrawEnumerateExA" ) ;
	if ( apiDirectDrawEnumerateEx != NULL )
	{
		DDEnumCallback_GetDDMonitorGUID_Param	param ;
		param.fFound = false ;
		param.nIndex = 0 ;
		param.hMonitor = hMonitor ;
		//
		apiDirectDrawEnumerateEx
			( DDEnumCallback_GetDDMonitorGUID,
				&param, DDENUM_ATTACHEDSECONDARYDEVICES ) ;
		//
		if ( param.fFound )
		{
			*pGUID = param.guidFound ;
			pRetGUID = pGUID ;
			//
			if ( (pGetIndex != NULL) && (param.nIndex > 0) )
			{
				*pGetIndex = param.nIndex - 1 ;
			}
		}
	}
	//
	::FreeLibrary( hModule ) ;
	//
	return	pRetGUID ;
}

static BOOL WINAPI DDEnumCallback_GetDDMonitorGUID
	( GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm )
{
	DDEnumCallback_GetDDMonitorGUID_Param *	pParam =
		(DDEnumCallback_GetDDMonitorGUID_Param*) lpContext ;
	if ( (hm == pParam->hMonitor) && (lpGUID != NULL) )
	{
		pParam->guidFound = *lpGUID ;
		pParam->fFound = true ;
		return	false ;
	}
	pParam->nIndex ++ ;
	return	true ;
}

// 現在のディスプレイとのビット深度を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EDisplayMode::GetDisplayColorMode( const char * pszDisplayName )
{
	HDC	hDC = ::CreateCompatibleDC( NULL ) ;
	if ( hDC == NULL )
	{
		return	0 ;
	}
	unsigned int	nBitsPerPixel = ::GetDeviceCaps( hDC, BITSPIXEL ) ;
	::DeleteDC( hDC ) ;
	return	nBitsPerPixel ;
}

// 現在のディスプレイの周波数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EDisplayMode::GetDisplayFrequency( const char * pszDisplayName )
{
	DEVMODE	dv ;
	if ( !::EnumDisplaySettings( pszDisplayName, ENUM_CURRENT_SETTINGS, &dv ) )
	{
		return	0 ;
	}
	return	dv.dmDisplayFrequency ;
}

// ディスプレイモードを列挙する
//////////////////////////////////////////////////////////////////////////////
EObjArray<DEVMODE> EDisplayMode::EnumDisplayMode
	( DWORD dwWidth, DWORD dwHeight, DWORD dwBitsPerPixel,
			DWORD dwFrequency, const char * pszDisplayName )
{
	EObjArray<DEVMODE>	DevList ;
	DEVMODE				DevMode ;
	unsigned int		nModeIndex = 0 ;
	for ( ; ; )
	{
		if ( !::EnumDisplaySettings( pszDisplayName, nModeIndex, &DevMode ) )
		{
			return	DevList ;
		}
		nModeIndex ++ ;
		if ( ((DevMode.dmPelsWidth == dwWidth)
				&& (DevMode.dmPelsHeight == dwHeight)) &&
			((dwBitsPerPixel != 0)
				&& (DevMode.dmBitsPerPel != dwBitsPerPixel)) &&
			((dwFrequency != 0) &&
				(DevMode.dmDisplayFrequency == dwFrequency)) )
		{
			DEVMODE *	pDevMode = new DEVMODE ;
			*pDevMode = DevMode ;
			DevList.Add( pDevMode ) ;
		}
	}
}

// 指定モードに切り替え可能かテストする
//////////////////////////////////////////////////////////////////////////////
ESLError EDisplayMode::TestDisplayMode
	( DWORD dwWidth, DWORD dwHeight, DWORD dwBitsPerPixel,
			DWORD dwFrequency, const char * pszDisplayName )
{
	if ( dwBitsPerPixel == 0 )
	{
		dwBitsPerPixel = GetDisplayColorMode( ) ;
	}
	if ( dwFrequency == 0 )
	{
		dwFrequency = GetDisplayFrequency( ) ;
	}
	unsigned int	nModeIndex = 0 ;
	for ( ; ; )
	{
		if ( !::EnumDisplaySettings( pszDisplayName, nModeIndex, this ) )
		{
			return	eslErrGeneral ;
		}
		nModeIndex ++ ;
		if ( (dmBitsPerPel == dwBitsPerPixel) &&
			(dmPelsWidth == dwWidth) && (dmPelsHeight == dwHeight) )
		{
			if ( (dwFrequency == 0) ||
					(dmDisplayFrequency == dwFrequency) )
			{
				break ;
			}
		}
	}
	dmSize = sizeof(DEVMODE) ;
	dmFields = DM_BITSPERPEL | DM_PELSWIDTH |
		DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY ;
	return	eslErrSuccess ;
}

// ディスプレイモードを切り替える
//////////////////////////////////////////////////////////////////////////////
ESLError EDisplayMode::ChangeDisplayMode( const char * pszDisplayName )
{
	int	nResult ;
	PrepareMonitorAPIs() ;
	if ( m_apiChangeDisplaySettingsEx && pszDisplayName )
	{
		nResult = m_apiChangeDisplaySettingsEx
			( pszDisplayName, this, NULL, CDS_FULLSCREEN, 0 ) ;
	}
	else
	{
		nResult = ::ChangeDisplaySettings( this, CDS_FULLSCREEN ) ;
	}
	if ( nResult == DISP_CHANGE_SUCCESSFUL )
	{
		m_fChanged = TRUE ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ディスプレイモードを元に戻す
//////////////////////////////////////////////////////////////////////////////
ESLError EDisplayMode::RestoreDisplayMode( void )
{
	if ( !m_fChanged )
	{
		return	eslErrSuccess ;
	}
	int	nResult = ::ChangeDisplaySettings( NULL, CDS_FULLSCREEN ) ;
	if ( nResult == DISP_CHANGE_SUCCESSFUL )
	{
		m_fChanged = FALSE ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}


//////////////////////////////////////////////////////////////////////////////
// レジストリキーオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ERegistryKey, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ERegistryKey::ERegistryKey( void )
	: m_hKey( NULL ), m_fOpened( FALSE )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ERegistryKey::~ERegistryKey( void )
{
	CloseKey( ) ;
}

// レジストリキーを作成
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::CreateKey
	( HKEY hKey, const char * pszSubKey, REGSAM samDersired )
{
	CloseKey( ) ;

	DWORD	dwDisposition = 0 ;
	int	nResult = ::RegCreateKeyEx
		( hKey, pszSubKey, NULL, "", REG_OPTION_NON_VOLATILE,
			samDersired, NULL, &m_hKey, &dwDisposition ) ;
	if ( nResult == ERROR_SUCCESS )
	{
		m_fOpened = TRUE ;
		return	eslErrSuccess ;
	}

	return	eslErrGeneral ;
}

// レジストリキーを削除
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::DeleteKey( HKEY hKey, const char * pszSubKey )
{
	if ( ::RegDeleteKey( hKey, pszSubKey ) == ERROR_SUCCESS )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// レジストリキーを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::OpenKey
	( HKEY hKey, const char * pszSubKey, REGSAM samDersired )
{
	CloseKey( ) ;

	int	nResult = ::RegOpenKeyEx
		( hKey, pszSubKey, 0, samDersired, &m_hKey ) ;
	if ( nResult == ERROR_SUCCESS )
	{
		m_fOpened = TRUE ;
		return	eslErrSuccess ;
	}

	return	eslErrGeneral ;
}

// レジストリキーを閉じる
//////////////////////////////////////////////////////////////////////////////
void ERegistryKey::CloseKey( void )
{
	if ( m_fOpened )
	{
		int	nResult = ::RegCloseKey( m_hKey ) ;
		if ( nResult == ERROR_SUCCESS )
		{
			m_fOpened = FALSE ;
		}
	}
}

// 値を削除する
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::DeleteValue( const char * pszValueName )
{
	ESLAssert( m_fOpened ) ;
	if ( ::RegDeleteValue( m_hKey, pszValueName ) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// バイナリデータをセットする
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::SetBinary
	( const char * pszValueName, const void * lpData, DWORD dwBytes )
{
	ESLAssert( m_fOpened ) ;
	if ( ::RegSetValueEx
		( m_hKey, pszValueName, 0,
			REG_BINARY, (const BYTE *)lpData, dwBytes ) == ERROR_SUCCESS )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 整数値をセットする
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::SetInteger
	( const char * pszValueName, unsigned int nInteger )
{
	ESLAssert( m_fOpened ) ;
	if ( ::RegSetValueEx
		( m_hKey, pszValueName, 0, REG_DWORD,
			(const BYTE *)&nInteger, sizeof(DWORD) ) == ERROR_SUCCESS )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

ESLError ERegistryKey::SetInteger64
	( const char * pszValueName, INT64 nInteger )
{
	ESLAssert( m_fOpened ) ;
	if ( ::RegSetValueEx
		( m_hKey, pszValueName, 0, REG_QWORD,
			(const BYTE *)&nInteger, sizeof(INT64) ) == ERROR_SUCCESS )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 文字列をセットする
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::SetString
	( const char * pszValueName, const char * pszString )
{
	ESLAssert( m_fOpened ) ;
	unsigned int	nLen = ::lstrlen( pszString ) + 1 ;
	if ( ::RegSetValueEx
		( m_hKey, pszValueName, 0, REG_SZ,
			(const BYTE *) pszString, nLen ) == ERROR_SUCCESS )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 倍精度浮動小数点値をセットする
//////////////////////////////////////////////////////////////////////////////
ESLError ERegistryKey::SetDoubleReal
	( const char * pszValueName, double nReal )
{
	return	SetBinary( pszValueName, &nReal, sizeof(double) ) ;
}

// バイナリデータを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned int ERegistryKey::GetBinary
	( const char * pszValueName, void * lpData, DWORD dwBytes ) const
{
	ESLAssert( m_fOpened ) ;
	DWORD	dwType = REG_BINARY ;
	if ( ::RegQueryValueEx
		( m_hKey, pszValueName,
			NULL, &dwType, (LPBYTE)lpData, &dwBytes ) == ERROR_SUCCESS )
	{
		return	dwBytes ;
	}
	return	0 ;
}

// 整数値を取得する
//////////////////////////////////////////////////////////////////////////////
int ERegistryKey::GetInteger
	( const char * pszValueName, int nDefValue ) const
{
	ESLAssert( m_fOpened ) ;
	DWORD	dwType = REG_DWORD ;
	DWORD	dwBytes = sizeof(DWORD) ;
	DWORD	dwData = 0 ;
	if ( ::RegQueryValueEx
		( m_hKey, pszValueName,
			NULL, &dwType, (LPBYTE)&dwData, &dwBytes ) == ERROR_SUCCESS )
	{
		return	dwData ;
	}
	return	nDefValue ;
}

INT64 ERegistryKey::GetInteger64
	( const char * pszValueName, INT64 nDefValue ) const
{
	ESLAssert( m_fOpened ) ;
	DWORD	dwType = REG_QWORD ;
	DWORD	dwBytes = sizeof(INT64) ;
	INT64	nData = 0 ;
	if ( ::RegQueryValueEx
		( m_hKey, pszValueName,
			NULL, &dwType, (LPBYTE)&nData, &dwBytes ) == ERROR_SUCCESS )
	{
		return	nData ;
	}
	return	nDefValue ;
}

// 文字列を取得する
//////////////////////////////////////////////////////////////////////////////
EString ERegistryKey::GetString
	( const char * pszValueName, const char * pszDefString ) const
{
	ESLAssert( m_fOpened ) ;
	//
	// 文字列を取得する
	EString	strData ;
	DWORD	dwType = REG_SZ ;
	DWORD	dwBytes = 0x100 ;
	LONG	nResult ;
	nResult = ::RegQueryValueEx
		( m_hKey, pszValueName, NULL, &dwType, NULL, &dwBytes ) ;
	nResult = ::RegQueryValueEx
		( m_hKey, pszValueName, NULL, &dwType,
			(LPBYTE)strData.GetBuffer(dwBytes + 1), &dwBytes ) ;
	strData.ReleaseBuffer( ) ;
	if ( nResult == ERROR_SUCCESS )
	{
		return	strData ;
	}
	return	pszDefString ;
}

// 倍精度浮動小数点値を取得する
//////////////////////////////////////////////////////////////////////////////
double ERegistryKey::GetDoubleReal
	( const char * pszValueName, double nDefValue ) const
{
	double	nReal ;
	if ( GetBinary
		( pszValueName, &nReal, sizeof(double) ) == sizeof(double) )
	{
		return	nReal ;
	}
	return	nDefValue ;
}
