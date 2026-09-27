
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/sgl_win_screen_capture.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画面キャプチャー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLScreenCapture, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLScreenCapture::SGLScreenCapture( void )
	: m_rectCapture( 0, 0, 0, 0 )
{
	m_hWndCapture = NULL ;
	m_fWndClientArea = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLScreenCapture::~SGLScreenCapture( void )
{
}

// キャプチャーターゲット設定
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::SetCaptureTarget
		( HWND hWnd, bool fClientArea, bool fCaptureMode )
{
	m_hWndCapture = hWnd ;
	m_fWndClientArea = fClientArea ;
	m_fCaptureMode = fCaptureMode ;
}

// キャプチャーサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::SetCaptureSize( int nWidth, int nHeight )
{
	if ( (m_rectCapture.w != nWidth)
		|| (m_rectCapture.h != nHeight) )
	{
		m_rectCapture.w = nWidth ;
		m_rectCapture.h = nHeight ;
		//
		m_imgCapture.CreateImage( nWidth, nHeight, formatImageRGB, 32 ) ;
	}
}

// キャプチャー座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::SetCapturePosition( int xPos, int yPos )
{
	m_rectCapture.x = xPos ;
	m_rectCapture.y = yPos ;
}

// キャプチャーターゲット取得
//////////////////////////////////////////////////////////////////////////////
HWND SGLScreenCapture::GetCaptureTarget( void ) const
{
	return	m_hWndCapture ;
}

bool SGLScreenCapture::IsCaptureClient( void ) const
{
	return	m_fWndClientArea ;
}

bool SGLScreenCapture::IsCaptureMode( void )
{
	return	m_fCaptureMode ;
}

// キャプチャー実行
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLScreenCapture::Capture( void )
{
	HDC	hDC = NULL ;
	if ( m_hWndCapture && !::IsWindow( m_hWndCapture ) )
	{
		return	NULL ;
	}
	/*
	if ( m_hWndCapture && m_fWndClientArea )
	{
		hDC = ::GetDC( m_hWndCapture ) ;
	}
	else
	{
		hDC = ::GetWindowDC( m_hWndCapture ) ;
	}
	*/
	//
	SGLImageWin32DIBitmap *
			pDIB = SGLImageWin32DIBitmap::CommitDIB( &m_imgCapture ) ;
	if ( pDIB == NULL )
	{
		return	NULL ;
	}
	/*
	if ( ::BitBlt
		( pDIB->m_hDC,
			0, 0, m_rectCapture.w, m_rectCapture.h,
			hDC, m_rectCapture.x, m_rectCapture.y,
			SRCCOPY | NOMIRRORBITMAP | (m_fCaptureMode ? CAPTUREBLT : 0) ) )
	{
		m_imgCapture.ReflectImageObject( imageObjectWin32DIBitmap ) ;
	}
	else
	{
		ESLTrace( "Failed to BitBlt to capture.\n" ) ;
	}
	*/
	if ( m_hWndCapture == NULL )
	{
		hDC = ::GetWindowDC( m_hWndCapture ) ;
		::BitBlt
			( pDIB->m_hDC,
				0, 0, m_rectCapture.w, m_rectCapture.h,
				hDC, m_rectCapture.x, m_rectCapture.y,
				SRCCOPY | NOMIRRORBITMAP | (m_fCaptureMode ? CAPTUREBLT : 0) ) ;
		m_imgCapture.ReflectImageObject( imageObjectWin32DIBitmap ) ;
		::ReleaseDC( m_hWndCapture, hDC ) ;
	}
	else
	{
		#ifndef	PW_RENDERFULLCONTENT
		const DWORD	PW_RENDERFULLCONTENT = 0x00000002 ;
		#endif
		::PrintWindow
			( m_hWndCapture, pDIB->m_hDC, PW_RENDERFULLCONTENT | PW_CLIENTONLY ) ;
		m_imgCapture.ReflectImageObject( imageObjectWin32DIBitmap ) ;
	}
//	::ReleaseDC( m_hWndCapture, hDC ) ;
	//
	return	&m_imgCapture ;
}

// キー操作イベント発生
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::KeyboardEvent
	( int64_t nVirtKey, int64_t nFlags, bool fRelease )
{
	BYTE	bScan = 0 ;
	DWORD	dwFlags = 0 ;
	if ( fRelease )
	{
		dwFlags |= KEYEVENTF_KEYUP ; 
	}
	switch ( nVirtKey )
	{
	case	VK_SHIFT:
	case	VK_CONTROL:
	case	VK_CAPITAL:
	case	VK_NUMLOCK:
	case	VK_SCROLL:
	case	VK_LSHIFT:
	case	VK_RSHIFT:
	case	VK_LCONTROL:
	case	VK_RCONTROL:
		dwFlags |= KEYEVENTF_EXTENDEDKEY ; 
		break ;
	}
	keybd_event( (BYTE) nVirtKey, bScan, dwFlags, 0 ) ;

	if ( fRelease )
	{
		switch ( nVirtKey )
		{
		case	VK_SHIFT:
		case	VK_LSHIFT:
		case	VK_RSHIFT:
			keybd_event( VK_SHIFT, bScan, KEYEVENTF_KEYUP, 0 ) ;
			break ;
		case	VK_CONTROL:
		case	VK_LCONTROL:
		case	VK_RCONTROL:
			keybd_event( VK_CONTROL, bScan, KEYEVENTF_KEYUP, 0 ) ;
			break ;
		}
	}
}

// マウス操作イベント発生
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::MouseEvent
		( SGLScreenCapture::MouseEventCode code, int nDelta )
{
#ifndef	XBUTTON1
	const DWORD	XBUTTON1 = 0x0001 ;
#endif
#ifndef	XBUTTON2
	const DWORD	XBUTTON2 = 0x0002 ;
#endif
	DWORD	dwFlags = 0 ;
	DWORD	dwData = 0 ;
	switch ( code )
	{
	case	mouseLeftDown:
		dwFlags = MOUSEEVENTF_LEFTDOWN ;
		break ;
	case	mouseLeftUp:
		dwFlags = MOUSEEVENTF_LEFTUP ;
		break ;
	case	mouseRightDown:
		dwFlags = MOUSEEVENTF_RIGHTDOWN ;
		break ;
	case	mouseRightUp:
		dwFlags = MOUSEEVENTF_RIGHTUP ;
		break ;
	case	mouseMiddleDown:
		dwFlags = MOUSEEVENTF_MIDDLEDOWN ;
		break ;
	case	mouseMiddleUp:
		dwFlags = MOUSEEVENTF_MIDDLEUP ;
		break ;
	case	mouseWheel:
		dwFlags = MOUSEEVENTF_WHEEL ;
		dwData = (DWORD) (nDelta * WHEEL_DELTA / 0x100) ;
		break ;
	case	mouseXButton1Down:
		dwFlags = MOUSEEVENTF_XDOWN ;
		dwData = XBUTTON1 ;
		break ;
	case	mouseXButton1Up:
		dwFlags = MOUSEEVENTF_XUP ;
		dwData = XBUTTON1 ;
		break ;
	case	mouseXButton2Down:
		dwFlags = MOUSEEVENTF_XDOWN ;
		dwData = XBUTTON2 ;
		break ;
	case	mouseXButton2Up:
		dwFlags = MOUSEEVENTF_XUP ;
		dwData = XBUTTON2 ;
		break ;
	default:
		return ;
	}
	mouse_event( dwFlags, 0, 0, dwData, 0 ) ;
}

// マウス座標移動
//////////////////////////////////////////////////////////////////////////////
void SGLScreenCapture::MouseMove( int xPos, int yPos, bool fDelta )
{
	if ( fDelta )
	{
		mouse_event( MOUSEEVENTF_MOVE, xPos, yPos, 0, 0 ) ;
	}
	else
	{
		xPos = xPos * 0xFFFF / ::GetSystemMetrics( SM_CXSCREEN ) ;
		yPos = yPos * 0xFFFF / ::GetSystemMetrics( SM_CYSCREEN ) ;
		mouse_event
			( MOUSEEVENTF_MOVE
				| MOUSEEVENTF_ABSOLUTE, xPos, yPos, 0, 0 ) ;
	}
}
