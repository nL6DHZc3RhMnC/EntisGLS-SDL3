
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_heap_memory.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/window/sgl_window_menu.h>
#include <sakuragl/sgl_opengl_context.h>
#include <d3d9.h>
#include <stdio.h>

using namespace SSystem ;
using namespace SakuraGL ;

#if	!defined(GWLP_WNDPROC)
#define	GWLP_WNDPROC	GWL_WNDPROC
#endif

#if	!defined(GWLP_USERDATA)
#define	GWLP_USERDATA	GWL_USERDATA
#endif

#if	!defined(GCLP_HICON)
#define	GCLP_HICON	GCL_HICON
#endif

#if	!defined(GCLP_HBRBACKGROUND)
#define	GCLP_HBRBACKGROUND	GCL_HBRBACKGROUND
#endif


//////////////////////////////////////////////////////////////////////////////
// SGLImageBufferInterface の DIB 実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLImageWin32DIBitmap, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageWin32DIBitmap::SGLImageWin32DIBitmap( void )
{
	m_typeObject = imageObjectWin32DIBitmap ;
	m_flagUpdateFull = true ;
	m_flagUpdateRect = false ;
	m_hDC = nullptr ;
	m_hBitmap = nullptr ;
	m_hDefBitmap = nullptr ;
	m_pPixels = nullptr ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageWin32DIBitmap::~SGLImageWin32DIBitmap( void )
{
	if ( m_hBitmap != nullptr )
	{
		if ( m_hDC != nullptr )
		{
			::SelectObject( m_hDC, m_hDefBitmap ) ;
		}
		::DeleteObject( m_hBitmap ) ;
		m_hBitmap = nullptr ;
		m_hDefBitmap = nullptr ;
	}
	if ( m_hDC != nullptr )
	{
		::DeleteDC( m_hDC ) ;
		m_hDC = nullptr ;
	}
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageWin32DIBitmap::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	if ( !m_flagUpdateFull && (pRect != nullptr) )
	{
		if ( m_flagUpdateRect )
		{
			m_rectUpdate = SGLRect( m_rectUpdate ) | SGLRect( *pRect ) ;
		}
		else
		{
			m_rectUpdate = *pRect ;
			m_flagUpdateRect = true ;
		}
	}
	else
	{
		m_flagUpdateFull = true ;
	}
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageWin32DIBitmap::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	if ( m_hDC == nullptr )
	{
		m_hDC = ::CreateCompatibleDC( nullptr ) ;
	}
	if ( m_hBitmap == nullptr )
	{
		memset( &m_bmi, 0, sizeof(BITMAPINFO) ) ;
		m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
		m_bmi.bmiHeader.biBitCount = pImageBuf->depth ;
		m_bmi.bmiHeader.biWidth = pImageBuf->width ;
		m_bmi.bmiHeader.biHeight = pImageBuf->height ;
	    m_bmi.bmiHeader.biCompression = BI_RGB ;
	    m_bmi.bmiHeader.biPlanes = 1 ;
		//
		m_hBitmap =
			::CreateDIBSection
				( m_hDC, &m_bmi,
					DIB_RGB_COLORS, (void**) &m_pPixels, nullptr, 0 ) ;
		if ( m_hBitmap != nullptr )
		{
			m_hDefBitmap = (HBITMAP) ::SelectObject( m_hDC, m_hBitmap ) ;
			//
			m_imgbuf.format = pImageBuf->format ;
			if ( (m_imgbuf.format & formatImageTypeMask) == formatImageBGR )
			{
				m_imgbuf.format =
					formatImageRGB | (m_imgbuf.format & ~formatImageTypeMask) ;
			}
			m_imgbuf.depth = pImageBuf->depth ;
			m_imgbuf.width = pImageBuf->width ;
			m_imgbuf.height = pImageBuf->height ;
			m_imgbuf.pitchPixel = (m_imgbuf.depth >> 3) ;
			m_imgbuf.pitchLine =
				(((m_imgbuf.width * m_imgbuf.depth + 0x1F) >> 5) << 2) ;
			m_imgbuf.ptrBuffer =
				m_pPixels + (m_imgbuf.height - 1) * m_imgbuf.pitchLine ;
			m_imgbuf.pitchLine = - m_imgbuf.pitchLine ;
		}
		else
		{
			m_pPixels = nullptr ;
		}
		::GdiFlush() ;
	}
	if ( m_flagUpdateFull )
	{
		m_rectUpdate.x = 0 ;
		m_rectUpdate.y = 0 ;
		m_rectUpdate.w = m_imgbuf.width ;
		m_rectUpdate.h = m_imgbuf.height ;
		m_flagUpdateRect = true ;
		m_flagUpdateFull = false ;
	}
	if ( m_flagUpdateRect )
	{
		if ( (m_imgbuf.format == pImageBuf->format)
			&& (m_imgbuf.depth == pImageBuf->depth) )
		{
			sglCopyImageBuffer
				( m_imgbuf, *pImageBuf,
					m_rectUpdate.x, m_rectUpdate.y, &m_rectUpdate ) ;
		}
		else
		{
			sglConvertImageBuffer
				( m_imgbuf, *pImageBuf,
					m_rectUpdate.x, m_rectUpdate.y, &m_rectUpdate ) ;
		}
		m_flagUpdateRect = false ;
	}
	return	sglErrSuccess ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageWin32DIBitmap::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	if( m_imgbuf.ptrBuffer != nullptr )
	{
		int	xPos = 0, yPos = 0 ;
		if ( pRect != nullptr )
		{
			xPos = pRect->x ;
			yPos = pRect->y ;
		}
		if ( (m_imgbuf.format == pImageBuf->format)
			&& (m_imgbuf.depth == pImageBuf->depth) )
		{
			sglCopyImageBuffer
				( *pImageBuf, m_imgbuf,
					m_rectUpdate.x, m_rectUpdate.y, &m_rectUpdate ) ;
		}
		else
		{
			sglConvertImageBuffer
				( *pImageBuf, m_imgbuf,
					m_rectUpdate.x, m_rectUpdate.y, &m_rectUpdate ) ;
		}
		m_flagUpdateFull = false ;
		m_flagUpdateRect = false ;
	}
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageWin32DIBitmap::MakeMipmap( void )
{
	return	sglErrSuccess ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool SGLImageWin32DIBitmap::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}

// SGLImageObject から SGLImageWin32DIBitmap 取得
//////////////////////////////////////////////////////////////////////////////
SGLImageWin32DIBitmap * SGLImageWin32DIBitmap::CommitDIB( SGLImageObject * pImage )
{
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	SGLImageRect	rectRef ;
	SGLImageBufferInterface *
		pObject = pImage->CommitImageObject
					( imageObjectWin32DIBitmap, rectRef, true ) ;
	if ( pObject == nullptr )
	{
		pImage->AddImageObject( new SGLImageWin32DIBitmap, false ) ;
		pObject = pImage->CommitImageObject
						( imageObjectWin32DIBitmap, rectRef, true ) ;
	}
	return	ESLTypeCast<SGLImageWin32DIBitmap>( pObject ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 汎用ウィンドウ・UI スレッド
//////////////////////////////////////////////////////////////////////////////

const char *	SGLGenericWindow::SGL_GENERIC_WINDOW_CLASS = "EntisGLS4_SGLGenericWindow" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLGenericWindow::UIThreadProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::UIThreadProcedure::UIThreadProcedure
	( SGLGenericWindow * pWnd,
		bool fModeDisplay, const wchar_t * pwszCaption,
		Window::CooperationMode modeCooperation,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	m_pWnd = pWnd ;
	m_fModeDisplay = fModeDisplay ;
	m_strCaption = pwszCaption ;
	m_modeCooperation = modeCooperation ;
	m_nWidth = nWidth ;
	m_nHeight = nHeight ;
	m_nBitsPerPixel = nBitsPerPixel ;
	m_nFrequency = nFrequency ;
	m_nFlags = nFlags ;
	m_pParentWnd = pParentWnd ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UIThreadProcedure::Run( void )
{
	SGLError	err ;
	if ( m_fModeDisplay )
	{
		err = m_pWnd->OnCreateDisplay
			( m_strCaption, m_modeCooperation,
				m_nWidth, m_nHeight, m_nBitsPerPixel, m_nFrequency ) ;
	}
	else
	{
		err = m_pWnd->OnCreateWindow
			( m_strCaption, m_nWidth, m_nHeight, m_nFlags, m_pParentWnd ) ;
	}
	m_pWnd->m_errCreationResult = err ;
	m_pWnd->m_signalCreated.SetSignal() ;

	if ( !err && (m_pParentWnd == nullptr) )
	{
		m_pWnd->OnLoop() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 汎用関数呼び出し
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLGenericWindow::CallMethodOnUIThreadProcedure, SProcedure )

	// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::CallMethodOnUIThreadProcedure::CallMethodOnUIThreadProcedure
	( SGLGenericWindow * pWnd,
		SGLGenericWindow::PTR_METHOD pfnMethod,
		void * ptrParam, bool flagAutoDelete )
{
	m_flagAutoDelete = flagAutoDelete ;
	m_pWnd = pWnd ;
	m_pfnMethod = pfnMethod ;
	m_ptrParam = ptrParam ;
	m_signalDone.Initialize( false ) ;
	m_errResult = sglErrFailed ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::CallMethodOnUIThreadProcedure::Run( void )
{
	ESLAssert( m_pWnd != nullptr ) ;
	ESLAssert( m_pfnMethod != nullptr ) ;
	m_errResult = (m_pWnd->*m_pfnMethod)( m_ptrParam ) ;
}

// 完了処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::CallMethodOnUIThreadProcedure::Finalize( void )
{
	bool	flagAutoDelete = m_flagAutoDelete ;
	m_csSync.Lock() ;
	m_signalDone.SetSignal() ;
	m_csSync.Unlock() ;
	if ( flagAutoDelete )
	{
		delete	this ;
	}
}

// 関数の終了を待つ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CallMethodOnUIThreadProcedure::WaitDone( int64_t msecTimeout )
{
	return	(SGLError) m_signalDone.Wait( msecTimeout ) ;
}

// 関数の終了コード取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CallMethodOnUIThreadProcedure::GetMethodResult( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	return	m_errResult ;
}


//////////////////////////////////////////////////////////////////////////////
// 汎用ウィンドウ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST
	( SakuraGL::SGLGenericWindow, SGLAbstractWindow, m_wvfFramework.GetView() )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::SGLGenericWindow
	( SGLWindowViewProducer * pwvp, SEnvironmentInterface * env )
	: m_wvfFramework( this, pwvp )
{
	m_pEnv = env ;
	if ( env == nullptr )
	{
		m_pEnv = SEnvironmentInterface::GetInstance() ;
	}
	m_flagProducerFullscreen = false ;
	m_tidAttachedViewThread = SThread::InvalidId ;
	//
	m_pViewSync = nullptr ;
	m_nRequestFPS = 60 ;
	//
	m_hWnd = nullptr ;
	m_hIMC = nullptr ;
	m_wpSuperClass = nullptr ;
	//
	m_flagCreated = false ;
	m_flagAttached = false ;
	m_flagModeDisplay = false ;
	m_flagFullscreen = false ;
	m_flagChangePhysicalMode = false ;
	m_flagRestoreFullscreen = false ;
	m_flagLayeredWindow = false ;
	m_modeCooperation = Window::modeWindow ;
	m_flagsOption = 0 ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_flagsLayout = 0 ;
	//
	m_flagInitialPos = false ;
	m_flagInitialSize = false ;
	//
	m_flagStereoView = false ;
	//
	m_flagShowCursor = false ;
	m_hCursor = ::LoadCursor( nullptr, IDC_ARROW ) ;
	m_strCursorID = L"IDC_ARROW" ;
	//
	wmCallUIProcedure =
		::RegisterWindowMessage( "GLS4_WINDOW_CALL_UI_PROCEDURE" ) ;
	wmCallRenderProcedure =
		::RegisterWindowMessage( "GLS4_WINDOW_CALL_RENDER_PROCEDURE" ) ;
	//
	m_hMenu = nullptr ;
	//
	m_flagWMPaintEntered = false ;
	m_flagWMDestroying = false ;
	m_flagQuitMessageLoop = false ;
	//
	m_nFreezePaint = 0 ;
	//
	m_pMutexWindowUI = SSystem::g_mutexGlobal ;
	ESLAssert( m_pMutexWindowUI != nullptr ) ;
	//
#if	defined(__DEBUG__)
	m_flagFPSonCaption = true ;
	m_flagTracePerformance = false ;
#else
	m_flagFPSonCaption = false ;
	m_flagTracePerformance = false ;
#endif
	m_nCountRenderedFrames = 0 ;
	m_nLastFPS = 0 ;
	//
	m_nCountOnTimer = 0 ;
	m_msecSumOnTimer = 0.0 ;
	m_msecMaxOnTimer = 0.0 ;
	m_pLogListener = nullptr ;
	//
	m_flagWndClassOwner = false ;
	//
	m_hUser32 = ::LoadLibrary( "user32.dll" ) ;
	m_apiTrackMouseEvent = nullptr ;
	m_apiUpdateLayeredWindow = nullptr ;
	m_apiGetTouchInputInfo = nullptr ;
	m_apiCloseTouchInputHandle = nullptr ;
	m_apiRegisterTouchWindow = nullptr ;
	m_apiUnregisterTouchWindow = nullptr ;
	m_flagMouseLeaved = true ;
	m_flagActivated = false ;
	m_bytLeadChar = 0 ;
	//
	if ( m_hUser32 != nullptr )
	{
		m_apiTrackMouseEvent = (API_TrackMouseEvent)
			::GetProcAddress( m_hUser32, "TrackMouseEvent" ) ;
		m_apiUpdateLayeredWindow = (API_UpdateLayeredWindow)
			::GetProcAddress( m_hUser32, "UpdateLayeredWindow" ) ;
		m_apiGetTouchInputInfo = (API_GetTouchInputInfo)
			::GetProcAddress( m_hUser32, "GetTouchInputInfo" ) ;
		m_apiCloseTouchInputHandle = (API_CloseTouchInputHandle)
			::GetProcAddress( m_hUser32, "CloseTouchInputHandle" ) ;
		m_apiRegisterTouchWindow = (API_RegisterTouchWindow)
			::GetProcAddress( m_hUser32, "RegisterTouchWindow" ) ;
		m_apiUnregisterTouchWindow = (API_UnregisterTouchWindow)
			::GetProcAddress( m_hUser32, "UnregisterTouchWindow" ) ;
	}
	m_flagFirstWindowSize = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::~SGLGenericWindow( void )
{
	if ( m_flagCreated )
	{
		if ( m_flagModeDisplay )
		{
			CloseDisplay() ;
		}
		else
		{
			CloseWindow() ;
		}
	}
	if ( m_hUser32 != nullptr )
	{
		::FreeLibrary( m_hUser32 ) ;
		m_hUser32 = nullptr ;
	}
}

// 表示インターフェース取得
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewProducer * SGLGenericWindow::GetWindowViewProducer( void ) const
{
	return	m_wvfFramework.GetView() ;
}

// 表示インターフェース変更
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewProducer *
	SGLGenericWindow::ChangeWindowViewProducer( SGLWindowViewProducer * pwvp )
{
	return	m_wvfFramework.ChangeWindowViewProducer( pwvp ) ;
}

// セカンダリビュー追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::AttachSecondaryView
		( SGLSecondaryViewProducer * psvp, bool fVSync )
{
	return	m_wvfFramework.AttachSecondaryView( psvp, fVSync ) ;
}

// セカンダリビュー削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::DetachSecondaryView( SGLSecondaryViewProducer * psvp )
{
	return	m_wvfFramework.DetachSecondaryView( psvp ) ;
}

// VSync ビュー設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetVSyncSecondaryView
		( SGLSecondaryViewProducer * psvp, bool fVSync )
{
	return	m_wvfFramework.SetVSyncSecondaryView( psvp, fVSync ) ;
}

// 描画タイミングインターフェース設定
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewSynchronizer *
	SGLGenericWindow::SetViewSynchronizer
			( SGLWindowViewSynchronizer * pViewSync )
{
	SGLWindowViewSynchronizer *	pLastSync = nullptr ;
	m_csViewSync.Lock() ;
	pLastSync = m_pViewSync ;
	m_pViewSync = pViewSync ;
	m_csViewSync.Unlock() ;
	return	pLastSync ;
}

// 描画タイミング（FPS）設定（SGLWindowViewSynchronizer未設定時動作）
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SetViewFramePerSecond( int nReqFPS )
{
	m_nRequestFPS = nReqFPS ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CreateDisplay
	( const wchar_t * pszWindowName,
		Window::CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	if ( m_flagCreated )
	{
		return	sglErrFailed ;
	}
	UIThreadProcedure *	pProc =
		new UIThreadProcedure
			( this, true, pszWindowName, mode,
				nWidth, nHeight, nBitsPerPixel, nFrequency, 0, nullptr ) ;
	m_procUIThread = pProc ;
	//
	m_signalCreated.Initialize( false ) ;
	m_signalQuit.Initialize( false ) ;
	m_errCreationResult = sglErrFailed ;
	if ( m_threadUI.BeginThread( pProc ) )
	{
		return	sglErrFailed ;
	}
	m_signalCreated.Wait() ;
	return	m_errCreationResult ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CloseDisplay( void )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	//
	// 非同期スレッド終了
	//
	m_queAsyncThread.RequestQuit() ;
	m_queAsyncThread.WaitAllRunLoops() ;
	//
	if ( m_wpSuperClass != nullptr )
	{
		// サブクラス化したウィンドウの解除
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != nullptr )
		{
			pwvp->OnDetachedWindow( this ) ;
			m_flagProducerFullscreen = false ;
		}
		::SetWindowLongPtr
			( m_hWnd, GWLP_WNDPROC, (LONG_PTR) m_wpSuperClass ) ;
		DetachWindowFromChain() ;
		m_hWnd = nullptr ;
		m_flagCreated = false ;
		return	sglErrSuccess ;
	}
	//
	// ウィンドウの破棄
	//
	SGLError	err =
		CallMethodOnUIThread( &SGLGenericWindow::OnDestroyWindow ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !IsOnUIThread() )
	{
		if ( m_refParentWnd.GetReference() == nullptr )
		{
			m_threadUI.Wait() ;
			m_threadUI.Delete() ;
		}
		else
		{
			m_signalQuit.Wait() ;
		}
		m_signalCreated.Delete() ;
		m_signalQuit.Delete() ;
		m_flagCreated = false ;
		m_refParentWnd = nullptr ;
	}
	return	sglErrSuccess ;
}

// オプション機能フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLGenericWindow::GetOptionalFlags( void )
{
	return	m_flagsOption ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SetOptionalFlags( uint64_t nFlags )
{
	if ( !m_flagCreated )
	{
		m_flagsOption = nFlags ;
		return ;
	}
	CallMethodOnUIThread
		( &SGLGenericWindow::OnSetOptionalFlags, &nFlags ) ;
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ChangeCooperationLevel( Window::CooperationMode mode )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	if ( mode == m_modeCooperation )
	{
		return	sglErrSuccess ;
	}
	return	CallMethodOnUIThread
				( &SGLGenericWindow::OnChangeCooperationLevel, &mode ) ;
}

// ウィンドウモード取得
//////////////////////////////////////////////////////////////////////////////
Window::CooperationMode SGLGenericWindow::GetCooperationLevel( void )
{
	return	m_modeCooperation ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ChangeDisplaySize
	( uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	METHOD_PARAM_DISPLAY_SIZE	param ;
	param.nWidth = nWidth ;
	param.nHeight = nHeight ;
	param.nBitsPerPixel = nBitsPerPixel ;
	param.nFrequency = nFrequency ;
	return	CallMethodOnUIThread
				( &SGLGenericWindow::OnChangeDisplaySize, &param ) ;
}

// 仮想ディスプレイサイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::GetDisplaySize( SGLSize& sizeDisplay )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	sizeDisplay = m_sizeVirtual ;
	return	sglErrSuccess ;
}

// 物理モニタの解像度を変更するか？
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::EnableChangePhysicalMode( bool flagEnable )
{
	bool	fChanged = (m_flagChangePhysicalMode != flagEnable) ;
	m_flagChangePhysicalMode = flagEnable ;
	if ( m_flagCreated && fChanged )
	{
		CooperationMode	mode = m_modeCooperation ;
		if ( mode >= modeFullScreen )
		{
			ChangeCooperationLevel( modeWindow ) ;
			ChangeCooperationLevel( mode ) ;
		}
	}
	return	sglErrSuccess ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::EnableZBuffer( bool flagZBuffer )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == nullptr )
	{
		return	sglErrFailed ;
	}
	return	pwvp->EnableZBuffer( this, flagZBuffer ) ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetStereoDisplayMode
	( const wchar_t * pszMethodID, uint64_t nParam )
{
	METHOD_PARAM_STEREO_DISPLAY_MODE	mpsdm ;
	mpsdm.m_strMethodID = pszMethodID ;
	mpsdm.m_nParam = nParam ;
	//
	return	CallMethodOnUIThread
		( &SGLGenericWindow::OnSetStereoDisplayMode, &mpsdm ) ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsSupportedStereoDisplayMode( const wchar_t * pszMethodID )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == nullptr )
	{
		return	false ;
	}
	return	pwvp->IsSupportedStereoDisplayMode( this, pszMethodID ) ;
}

// 仮想ディスプレイ・ウィンドウ初期座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::InitWindowPosition
	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize )
{
	m_flagInitialPos = true ;
	m_ptInitialPos.x = xPos ;
	m_ptInitialPos.y = yPos ;
	//
	if ( pInitExSize != nullptr )
	{
		m_flagInitialSize = true ;
		m_sizeInitialSize = *pInitExSize ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・ウィンドウの通常座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::GetNormalWindowPosition
		( SGLPoint& ptWindow, SGLSize * pWindowSize )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	if ( m_modeCooperation >= modeFullScreen )
	{
		ptWindow.x = m_rctNormalWndPos.left ;
		ptWindow.y = m_rctNormalWndPos.top ;
		//
		if ( pWindowSize != nullptr )
		{
			pWindowSize->w = m_rctNormalWndPos.right - m_rctNormalWndPos.left ;
			pWindowSize->h = m_rctNormalWndPos.bottom - m_rctNormalWndPos.top ;
		}
	}
	else
	{
		WINDOWPLACEMENT	wp ;
		if ( !::GetWindowPlacement( m_hWnd, &wp ) )
		{
			return	sglErrFailed ;
		}
		ptWindow.x = wp.rcNormalPosition.left ;
		ptWindow.y = wp.rcNormalPosition.top ;
		//
		if ( pWindowSize != nullptr )
		{
			pWindowSize->w = wp.rcNormalPosition.right - wp.rcNormalPosition.left ;
			pWindowSize->h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top ;
		}
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・ウィンドウ内表示座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::GetInternalDisplayPosition
		( SGLImageRect& rctRender, SGLImageRect& rctDisplay )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == nullptr )
	{
		return	sglErrFailed ;
	}
	pwvp->GetInternalViewPosition( rctDisplay ) ;
	//
	if ( !pwvp->GetExternalViewPosition( rctRender ) )
	{
		RECT	rectClient ;
		if ( ::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_MINIMIZE )
		{
			WINDOWPLACEMENT	wp ;
			if ( !::GetWindowPlacement( m_hWnd, &wp ) )
			{
				return	sglErrFailed ;
			}
			rectClient = wp.rcNormalPosition ;
		}
		else if ( !::GetClientRect( m_hWnd, &rectClient ) )
		{
			return	sglErrFailed ;
		}
		rctRender.x = rectClient.left ;
		rctRender.y = rectClient.top ;
		rctRender.w = rectClient.right - rectClient.left ;
		rctRender.h = rectClient.bottom - rectClient.top ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_wvfFramework.SetExteriorBackgroundFrame
		( nFlags, rgbColor, pTile, pLeft, pRight, pUpper, pUnder ) ;
	//
	if ( m_flagCreated )
	{
		PostUpdate() ;
	}
	Unlock() ;
	return	sglErrFailed ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	if ( m_flagCreated )
	{
		return	sglErrFailed ;
	}
	UIThreadProcedure *	pProc =
		new UIThreadProcedure
			( this, false, pszWindowName, Window::modeWindow,
				nWidth, nHeight, 0, 0, nFlags, pParentWnd ) ;
	m_procUIThread = pProc ;
	m_signalCreated.Initialize( false ) ;
	m_signalQuit.Initialize( false ) ;
	//
	if ( pParentWnd != nullptr )
	{
		if ( pParentWnd->PostUIThread( pProc ) )
		{
			return	sglErrFailed ;
		}
	}
	else
	{
		m_errCreationResult = sglErrFailed ;
		if ( m_threadUI.BeginThread( pProc ) )
		{
			return	sglErrFailed ;
		}
	}
	m_signalCreated.Wait() ;
	return	m_errCreationResult ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CloseWindow( void )
{
	return	SGLGenericWindow::CloseDisplay() ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetWindowLayout
			( uint32_t nFlags, int xPos, int yPos )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_flagsLayout = nFlags ;
	m_ptLayoutOffset.x = xPos ;
	m_ptLayoutOffset.y = yPos ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	Unlock() ;
	//
	if ( m_flagCreated )
	{
		return	CallMethodOnUIThread
					( &SGLGenericWindow::OnUpdateWindowLayout ) ;
	}
	return	sglErrSuccess ;
}

// クライアント座標→スクリーン座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLGenericWindow::ScreenPositionFromClient( S2DDVector& vClient )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != nullptr) && m_flagCreated )
	{
		pwvp->VirtualToPhysicalPosition( vClient ) ;
		//
		POINT	ptClient ;
		ptClient.x = eslRoundR32ToInt( (float32_t) vClient.x ) ;
		ptClient.y = eslRoundR32ToInt( (float32_t) vClient.y ) ;
		//
		::ClientToScreen( m_hWnd, &ptClient ) ;
		//
		vClient.x = ptClient.x ;
		vClient.y = ptClient.y ;
	}
	return	vClient ;
}

// スクリーン座標→クライアント座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLGenericWindow::ClientPositionFromScreen( S2DDVector& vScreen )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != nullptr) && m_flagCreated )
	{
		POINT	ptScreen ;
		ptScreen.x = eslRoundR32ToInt( (float32_t) vScreen.x ) ;
		ptScreen.y = eslRoundR32ToInt( (float32_t) vScreen.y ) ;
		//
		::ScreenToClient( m_hWnd, &ptScreen ) ;
		//
		S2DDVector	vClient( ptScreen.x, ptScreen.y ) ;
		pwvp->PhysicalToVirtualPosition( vClient ) ;
		//
		vScreen = vClient ;
	}
	return	vScreen ;
}

// 画面の更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::PostUpdate( const SGLImageRect* pUpdate )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	if ( m_pViewSync == nullptr )
	{
		if ( m_flagLayeredWindow )
		{
			::PostMessage( m_hWnd, WM_PAINT, 0, 0 ) ;
		}
		else
		{
			::InvalidateRect( m_hWnd, nullptr, FALSE ) ;
		}
	}
	return	sglErrSuccess ;
}

// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::UpdateWindow( Window::UpdateParameter * pUpdate )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	if ( pUpdate == nullptr )
	{
		if ( m_flagLayeredWindow )
		{
			UpdateLayeredWindow() ;
		}
		else
		{
			UpdateWindowTimeout( 100 ) ;
		}
		return	sglErrSuccess ;
	}
	//
	// 外部同期を利用する場合、タイマーを強制呼び出し
	//
	if ( m_pViewSync != nullptr )
	{
		::SendMessageTimeout
			( m_hWnd, WM_TIMER, 1, 0, SMTO_NORMAL, 100, &m_dwResultTemp ) ;
	}
	//
	// 直接描画
	//
	bool			fOnWinThread = IsOnUIThread() ;
	STimeCounter	timer ;
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != nullptr)
		&& (pwvp->GetCapacityFlags()
				& SGLWindowViewProducer::renderableAnyThread) )
	{
		S3DRenderDevice *	pDevice = pwvp->GetRenderDevice() ;
		ESLAssert( pDevice != nullptr ) ;
		if ( LockTrace( __FILE__, __LINE__, 100 ) == errSuccess )
		{
			if ( pwvp->AttachViewThread( this ) == sglErrSuccess )
			{
				const bool	fVSync =
					((pUpdate->flagsUpdate & Window::updateVSync) != 0) ;
				m_tidAttachedViewThread = SThread::GetCurrentId() ;

				if ( pDevice != nullptr )
				{
					pDevice->BeginFramePerformanceLog() ;
				}
				DrawWindow( true ) ;
				if ( pDevice != nullptr )
				{
					pDevice->EndFramePerformanceLog() ;
				}
				FlipView( fVSync, true ) ;
				//
				m_tidAttachedViewThread = SThread::InvalidId ;
				pwvp->DetachViewThread( this ) ;
			}
			Unlock() ;
		}
	}
	else
	{
		atomic_int_t	countLocked = UnlockAll() ;
		if ( m_flagLayeredWindow )
		{
			::SendMessageTimeout
				( m_hWnd, WM_PAINT, 0, 0, SMTO_NORMAL, 100, &m_dwResultTemp ) ;
		}
		else
		{
			::InvalidateRect( m_hWnd, nullptr, FALSE ) ;
			UpdateWindowTimeout( 100 ) ;
		}
		Relock( countLocked ) ;
	}
	//
	// 時間更新
	//
	uint32_t	msecRendering = (uint32_t) timer.GetTime() ;
	uint32_t	freqMonitor = GetMonitorFrequency() ;
//	Trace( "rendering at UpdateWindow %d[ms]\n", msecRendering ) ;
	pUpdate->WaitFrame( msecRendering, freqMonitor ) ;
	//
	return	sglErrSuccess ;
}

// ユーザー入力処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ProcessUserInput( int64_t msecTimeout )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	return	CallMethodOnUIThread
				( &SGLGenericWindow::OnProcessUserInput, &msecTimeout ) ;
}

// 描画スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::PostRenderingThread
		( SSystem::SProcedure * pProc, PostThreadType postType )
{
	if ( !m_flagCreated || (pProc == nullptr) )
	{
		return	sglErrFailed ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( postType == postAsyncNoRender )
	{
		m_queAsyncThread.AddProcedure( pProc, nullptr, false, false ) ;
		return	sglErrSuccess ;
	}
	if ( postType == postAsyncNoRenderFinally )
	{
		m_queAsyncThread.AttachFinallyProcedur( pProc ) ;
		return	sglErrSuccess ;
	}
	DWORD	dwProcessId ;
	if ( (postType == postNormal)
		&& (::GetCurrentThreadId()
				== ::GetWindowThreadProcessId( m_hWnd, &dwProcessId )) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		if ( pwvp != nullptr )
		{
			if ( pwvp->AttachViewThread( this ) != sglErrSuccess )
			{
				pwvp = nullptr ;
			}
		}
		if ( !m_queOnRenderThread.IsEmpty() )
		{
			m_queOnRenderThread.Flush() ;
		}
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
		if ( pwvp != nullptr )
		{
			pwvp->DetachViewThread( this ) ;
		}
		Unlock() ;
	}
	else
	{
		m_queOnRenderThread.Lock() ;
		m_queOnRenderThread.AddProcedure( pProc ) ;
//		::PostMessage( m_hWnd, wmCallRenderProcedure, 0, (LPARAM) pProc ) ;
		m_queOnRenderThread.Unlock() ;
	}
	return	sglErrSuccess ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::PostUIThread( SSystem::SProcedure * pProc )
{
	if ( !m_flagCreated || (pProc == nullptr) )
	{
		if ( m_flagActivated && (pProc != nullptr) )
		{
			pProc->Prepare() ;
			pProc->Run() ;
			pProc->Finalize() ;
			return	sglErrSuccess ;
		}
		return	sglErrFailed ;
	}
	DWORD	dwProcessId ;
	if ( ::GetCurrentThreadId()
			== ::GetWindowThreadProcessId( m_hWnd, &dwProcessId ) )
	{
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
	}
	else
	{
		::PostMessage( m_hWnd, wmCallUIProcedure, 0, (LPARAM) pProc ) ;
	}
	return	sglErrSuccess ;
}

// ウィンドウがアクティブ（最前面）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsWindowActive( void )
{
	return	m_flagCreated & m_flagActivated ;
}

// ウィンドウキャプション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetWindowCaption( const wchar_t * pszWindowName )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	SSystem::QuickLock() ;
	m_strCaption = pszWindowName ;
	//
	SString			strDisplay = FormatWindowCaption() ;
	SArray<char>	bufName ;
	const char *	pszName ;
	pszName = strDisplay.EncodeDefaultTo(bufName) ;
	SSystem::QuickUnlock() ;
	//
	::SetWindowText( m_hWnd, pszName ) ;
	return	sglErrSuccess ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ShowCursor( bool fShow )
{
	m_flagShowCursor = fShow ;
	return	sglErrSuccess ;
}

// マウスカーソル表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsShowCursor( void )
{
	return	m_flagShowCursor ;
}

// マウスカーソル変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetCursor( const wchar_t * pszCursorID )
{
	if ( pszCursorID == nullptr )
	{
		pszCursorID = L"IDC_ARROW" ;
	}
	if ( m_strCursorID == pszCursorID )
	{
		return	sglErrSuccess ;
	}
	m_hCursor = LoadWindowsCursor( pszCursorID ) ;
	if ( m_hCursor == nullptr )
	{
		m_hCursor = ::LoadCursor( nullptr, IDC_ARROW ) ;
	}
	m_strCursorID = pszCursorID ;
	return	sglErrSuccess ;
}

// マウスカーソル座標移動
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::MoveCursorPosition
	( int32_t xPos, int32_t yPos, int idMouse )
{
	if ( m_flagCreated )
	{
		S2DDVector	vCursor( xPos, yPos ) ;
		ScreenPositionFromClient( vCursor ) ;
		::SetCursorPos
			( eslRoundR32ToInt( (float32_t) vCursor.x ),
				eslRoundR32ToInt( (float32_t) vCursor.y ) ) ;
	}
	else
	{
		::SetCursorPos( xPos, yPos ) ;
	}
	return	sglErrSuccess ;
}

// マウスカーソル座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::GetCursorPosition( SGLPoint& ptCursor, int idMouse )
{
	POINT	posCursor ;
	::GetCursorPos( &posCursor ) ;
	//
	if ( m_flagCreated )
	{
		S2DDVector	vCursor( posCursor.x, posCursor.y ) ;
		ClientPositionFromScreen( vCursor ) ;
		ptCursor.x = eslRoundR32ToInt( (float32_t) vCursor.x ) ;
		ptCursor.y = eslRoundR32ToInt( (float32_t) vCursor.y ) ;
	}
	else
	{
		ptCursor.x = posCursor.x ;
		ptCursor.y = posCursor.y ;
	}
	return	sglErrSuccess ;
}

// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
//////////////////////////////////////////////////////////////////////////////
int SGLGenericWindow::GetMonitorFrequency( void )
{
	if ( !m_flagCreated )
	{
		return	0 ;
	}
	RECT	rectWindow ;
	if ( !::GetWindowRect( m_hWnd, &rectWindow ) )
	{
		return	0 ;
	}
	SString			strDisplayName ;
	SGLImageRect	irctWindow ;
	irctWindow.x = rectWindow.left ;
	irctWindow.y = rectWindow.top ;
	irctWindow.w = rectWindow.right - rectWindow.left ;
	irctWindow.h = rectWindow.bottom - rectWindow.top ;
	//
	return	(int) SGLDisplayMode::GetDisplayFrequency
					( m_displayMode.GetDisplayNameFromRect
								( strDisplayName, &irctWindow ) ) ;
}

// ウィンドウメニューの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::AttachMenu( SGLWindowMenu * pMenu )
{
	SGLWindowMenu *	pLastMenu = m_refMenu ;
	if ( pLastMenu != nullptr )
	{
		pLastMenu->OnAttachReferenceWindow( nullptr ) ;
	}
	m_refMenu = pMenu ;
	m_hMenu = nullptr ;
	if ( pMenu != nullptr )
	{
		m_refMenu->OnAttachReferenceWindow( this ) ;
		m_hMenu = pMenu->GetMenuHandle() ;
	}
	if ( (m_hWnd != nullptr) && ::IsWindow( m_hWnd ) )
	{
		CallMethodOnUIThread( &SGLGenericWindow::OnUpdateMenu ) ;
	}
	return	sglErrSuccess ;
}

// マウスイベントキャプチャー
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CaptureMouse( int idMouse )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	::SetCapture( m_hWnd ) ;
	return	sglErrSuccess ;
}

class	SGLGenericWindow_ReleaseMouseProc	: public SProcedure
{
public:
	// スレッド関数
	virtual void Run( void )
	{
		::ReleaseCapture() ;
	}
	// 完了後の処理
	virtual void Finalize( void )
	{
		delete	this ;
	}
} ;

SGLError SGLGenericWindow::ReleaseMouse( int idMouse )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	DWORD	dwProcessId ;
	if ( ::GetCurrentThreadId()
			== ::GetWindowThreadProcessId( m_hWnd, &dwProcessId ) )
	{
		::ReleaseCapture() ;
		return	sglErrSuccess ;
	}
	else
	{
		return	PostUIThread( new SGLGenericWindow_ReleaseMouseProc ) ;
	}
}

// 描画インターフェース取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLGenericWindow::GetRenderContext
	( S3DRenderContextInterface::StereoViewIndex sviView )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != nullptr) && m_flagCreated )
	{
		return	pwvp->BeginDrawView
			( this, (m_tidAttachedViewThread = SThread::GetCurrentId()) ) ;
	}
	return	nullptr ;
}

void SGLGenericWindow::ReleaseRenderContext
					( S3DRenderContextInterface* context )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		bool	flagInAttachedThread =
					(m_tidAttachedViewThread = SThread::GetCurrentId()) ;
		pwvp->EndDrawView( this, context, flagInAttachedThread ) ;
		pwvp->FlipView( this, true, flagInAttachedThread ) ;
	}
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLGenericWindow::GetRenderDevice( void )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		return	pwvp->GetRenderDevice() ;
	}
	return	nullptr ;
}

// ウィンドウスレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLGenericWindow::Lock( int64_t msecTimeout ) const
{
	return	m_pMutexWindowUI->Lock( msecTimeout ) ;
}

SSystem::SError SGLGenericWindow::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout ) const
{
#if	defined(__DEBUG__)
	return	m_pMutexWindowUI->LockTrace( pszSource, nLineNum, msecTimeout ) ;
#else
	return	m_pMutexWindowUI->Lock( msecTimeout ) ;
#endif
}

SSystem::SError SGLGenericWindow::Unlock( void ) const
{
	m_pMutexWindowUI->Unlock() ;
	return	errSuccess ;
}

atomic_int_t SGLGenericWindow::UnlockAll( void ) const
{
	return	m_pMutexWindowUI->UnlockAll() ;
}

SSystem::SError SGLGenericWindow::Relock( atomic_int_t nLock ) const
{
	return	m_pMutexWindowUI->Relock( nLock ) ;
}

atomic_int_t SGLGenericWindow::TestLocked( void ) const
{
	return	m_pMutexWindowUI->TestLocked() ;
}

// ウィンドウスレッド排他処理用ミューテックス変更
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SetWindowUIThreadMutex( SSystem::SMutex * pMutex )
{
	ESLAssert( pMutex != nullptr ) ;
	m_pMutexWindowUI = pMutex ;
}

// プラットフォーム固有オブジェクト
//////////////////////////////////////////////////////////////////////////////
HWND SGLGenericWindow::GetWindowHandle( void ) const
{
	return	m_hWnd ;
}

// 既存の Window をサブクラス化する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CreateSubclassWindow
		( HWND hWnd, const SGLSize * pVirtualDisplay )
{
	if ( (hWnd == nullptr) || (m_hWnd != nullptr) )
	{
		return	sglErrFailed ;
	}
	RECT	rectClient ;
	if ( !::GetClientRect( hWnd, &rectClient ) )
	{
		return	sglErrFailed ;
	}
	//
	// 変数設定
	//
	m_flagModeDisplay = (pVirtualDisplay != nullptr) ;
	m_flagFullscreen = false ;
	m_flagLayeredWindow = false ;
	m_flagRestoreFullscreen = false ;
	m_modeCooperation = Window::modeWindow ;
	if ( pVirtualDisplay != nullptr )
	{
		m_sizeVirtual = *pVirtualDisplay ;
		m_sizePhysical.w = rectClient.right - rectClient.left ;
		m_sizePhysical.h = rectClient.bottom - rectClient.top ;
	}
	else
	{
		m_sizeVirtual.w = rectClient.right - rectClient.left ;
		m_sizeVirtual.h = rectClient.bottom - rectClient.top ;
		m_sizePhysical = m_sizeVirtual ;
	}
	m_sizeLayoutOriginal = m_sizeVirtual ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_flagsLayout = 0 ;
	//
	// 動作フラグ変換
	//
	ReflectOptionFlagsFromCurrentStyle() ;
	m_flagsOption |= flagVariableWindowSize ;
	//
	// サブクラス化
	//
	m_hWnd = hWnd ;
	m_wpSuperClass =
		(WNDPROC) ::GetWindowLongPtr( m_hWnd, GWLP_WNDPROC ) ;
	::SetWindowLongPtr
		( m_hWnd, GWLP_WNDPROC,
				(LONG_PTR) &SGLGenericWindow::WindowCallbackProc ) ;
	::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR) this ) ;
	//
	::SetTimer( m_hWnd, 1, 16, nullptr ) ;
	//
	// 初期設定
	//
	m_flagCreated = true ;
	AddWindowToChain() ;
	//
	// 非同期スレッド開始
	//
	m_queAsyncThread.AsyncRun() ;
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		pwvp->OnAttachedWindow( this ) ;
	}
	UpdateClientDisplayPosition() ;
	//
	return	sglErrSuccess ;
}

// マウスカーソルをロードする
//////////////////////////////////////////////////////////////////////////////
HCURSOR SGLGenericWindow::LoadWindowsCursor( const wchar_t * pszCursorID )
{
	SString			strCursorID = pszCursorID ;
	SArray<char>	bufCursorID ;
	HMODULE			hModule = ::GetModuleHandle( nullptr ) ;
	LPCTSTR			lpCursorID = strCursorID.EncodeDefaultTo(bufCursorID) ;
	HCURSOR			hCursor = ::LoadCursor( hModule, lpCursorID ) ;
	if ( hCursor == nullptr )
	{
		static const wchar_t *	pwszSystemIDs[] =
		{
			L"IDC_ARROW", L"IDC_IBEAM", L"IDC_WAIT", L"IDC_CROSS",
			L"IDC_UPARROW", L"IDC_SIZE",
			L"IDC_SIZENWSE", L"IDC_SIZENESW",
			L"IDC_SIZEWE", L"IDC_SIZENS", L"IDC_SIZEALL",
			L"IDC_NO",
			#if	defined(IDC_HELP)
			L"IDC_HELP",
			#endif
			#if	defined(IDC_HAND)
			L"IDC_HAND",
			#endif
			nullptr
		} ;
		static const LPSTR	nSystemIDs[] =
		{
			IDC_ARROW, IDC_IBEAM, IDC_WAIT, IDC_CROSS,
			IDC_UPARROW, IDC_SIZE,
			IDC_SIZENWSE, IDC_SIZENESW,
			IDC_SIZEWE, IDC_SIZENS, IDC_SIZEALL,
			IDC_NO,
			#if	defined(IDC_HELP)
			IDC_HELP,
			#endif
			#if	defined(IDC_HAND)
			IDC_HAND,
			#endif
			IDC_ARROW
		} ;
		for ( size_t i = 0; pwszSystemIDs[i]; i ++ )
		{
			if ( strCursorID == pwszSystemIDs[i] )
			{
				hCursor = ::LoadCursor( nullptr, nSystemIDs[i] ) ;
			}
		}
	}
	return	hCursor ;
}

// メッセージループ（ウィンドウスレッド上から呼び出し）
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::DoMessageLoop( void )
{
	OnLoop() ;
}

// メッセージループ
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::QuitMessageLoop( void )
{
	m_flagQuitMessageLoop = true ;
}

// 既存のウィンドウのハンドルを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::AttachWindowHandle( HWND hWnd )
{
	ESLAssert( !m_flagCreated ) ;
	m_hWnd = hWnd ;
	m_flagActivated = true ;
}

// 既存のウィンドウのハンドルを分離する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::DetachWindowHandle( void )
{
	ESLAssert( !m_flagCreated ) ;
	m_flagActivated = false ;
	m_hWnd = nullptr ;
}

// ウィンドウ描画抑制
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::AddFreezePaint( void )
{
	AtomicAdd( &m_nFreezePaint, 1 ) ;
}

void SGLGenericWindow::ReleaseFreezePaint( void )
{
	ESLAssert( m_nFreezePaint >= 1 ) ;
	AtomicSub( &m_nFreezePaint, 1 ) ;
}

// ウィンドウクラス登録
//////////////////////////////////////////////////////////////////////////////
const char * SGLGenericWindow::RegisterWindowClass( void )
{
	//
	// 登録済みクラスのテスト
	//
	WNDCLASS		wndclass ;
	HMODULE			hModule = ::GetModuleHandle( nullptr ) ;
	const char *	pszClassName ;
	m_strClassName = SGL_GENERIC_WINDOW_CLASS ;
	pszClassName = m_strClassName.EncodeDefaultTo(m_cstrClassName) ;
	m_flagWndClassOwner = false ;
	//
	if ( ::GetClassInfo( hModule, pszClassName, &wndclass ) )
	{
		if ( wndclass.lpfnWndProc == &SGLGenericWindow::WindowCallbackProc )
		{
			return	pszClassName ;
		}
		for ( int i = 0; i < 0x10000; i ++ )
		{
			m_strClassName = SGL_GENERIC_WINDOW_CLASS ;
			m_strClassName += SString(i) ;
			pszClassName = m_strClassName.EncodeDefaultTo(m_cstrClassName) ;
			if ( !::GetClassInfo( hModule, pszClassName, &wndclass ) )
			{
				m_flagWndClassOwner = true ;
				break ;
			}
		}
	}
	//
	// ウィンドウクラス登録
	//
	wndclass.style = 0 ;
	wndclass.lpfnWndProc = &SGLGenericWindow::WindowCallbackProc ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = sizeof(SGLGenericWindow*) ;
	wndclass.hInstance = ::GetModuleHandle( nullptr ) ;
	wndclass.hIcon = nullptr ;
	wndclass.hCursor = nullptr ;
	wndclass.hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH ) ;
	wndclass.lpszMenuName = nullptr ;
	wndclass.lpszClassName = pszClassName ;
	//
	if ( ::RegisterClass( &wndclass ) == 0 )
	{
		ESLTrace( "failed to register class for SGLGenericWindow\n" ) ;
	}
	return	pszClassName ;
}

// ウィンドウ作成（低水準）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CreateWindowSimply
	( const wchar_t * pwszWindowName,
		uint32_t nWidth, uint32_t nHeight, HWND hwndParent )
{
	//
	// 初期座標
	//
	SGLSize	sizeWindow ;
	SGLRect	rectMargin ;
	GetWindowFrameMargin( rectMargin ) ;
	sizeWindow.w = nWidth + rectMargin.left + rectMargin.right ;
	sizeWindow.h = nHeight + rectMargin.top + rectMargin.bottom ;
	//
	if ( !m_flagInitialPos )
	{
		m_ptInitialPos.x =
			(::GetSystemMetrics(SM_CXSCREEN) - sizeWindow.w) / 2 ;
		m_ptInitialPos.y =
			(::GetSystemMetrics(SM_CYSCREEN) - sizeWindow.h) / 2 ;
		m_flagInitialPos = false ;
	}
	if ( m_flagInitialSize )
	{
		if ( m_flagsOption & flagVariableWindowSize )
		{
			sizeWindow = m_sizeInitialSize ;
		}
		m_flagInitialSize = false ;
	}
	m_displayMode.NormalizeWindowPos( m_ptInitialPos, sizeWindow, false ) ;
	//
	// 初期ウィンドウスタイル
	//
	DWORD	dwStyle = ModifyWindowStyleOf( 0, false ) & ~WS_VISIBLE ;
	DWORD	dwExStyle = ModifyWindowExStyleOf( 0, false ) ;
	//
	// ウィンドウ生成
	//
	const char *	pszClassName = RegisterWindowClass() ;
	m_strCaption = pwszWindowName ;
	//
	SString			strDisplay = FormatWindowCaption() ;
	SArray<char>	cstrWindowName ;
	const char *	pszWindowName ;
	pszWindowName = strDisplay.EncodeDefaultTo( cstrWindowName ) ;
	//
	m_flagFirstWindowSize = true ;
	//
	m_hWnd = ::CreateWindowEx
		( dwExStyle, pszClassName, pszWindowName, dwStyle,
			m_ptInitialPos.x, m_ptInitialPos.y,
			sizeWindow.w, sizeWindow.h,
			hwndParent, m_hMenu, ::GetModuleHandle(nullptr), this ) ;
	if ( (m_hWnd == nullptr) || !::IsWindow( m_hWnd ) )
	{
		ESLTrace( "failed to create window.\n" ) ;
		return	sglErrFailed ;
	}
	//
	// ウィンドウ初期設定
	//
	::SetWindowLongPtr
		( m_hWnd, GWLP_WNDPROC,
				(LONG_PTR) &SGLGenericWindow::WindowCallbackProc ) ;
	::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR) this ) ;
	//
	LONG	lClassStyle = ::GetClassLong( m_hWnd, GCL_STYLE ) ;
	::SetClassLong
		( m_hWnd, GCL_STYLE,
			ModifyWindowClassStyleOf( lClassStyle, false ) ) ;
	m_hIMC = ::ImmAssociateContext( m_hWnd, nullptr ) ;
	//
	HICON	hIcon = LoadMainIcon() ;
	if ( hIcon != nullptr )
	{
		::SetClassLongPtr( m_hWnd, GCLP_HICON, (LONG_PTR) hIcon ) ;
		::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM) hIcon ) ;
		::SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon ) ;
	}
	//
	if ( m_apiRegisterTouchWindow != nullptr )
	{
		m_apiRegisterTouchWindow( m_hWnd, 0 ) ;
	}
	//
	// 非同期スレッド開始
	//
	m_queAsyncThread.AsyncRun() ;
	m_flagCreated = true ;
	//
	// 表示設定
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		pwvp->OnAttachedWindow( this ) ;
	}
	UpdateClientDisplayPosition() ;
	//
	return	sglErrSuccess ;
}

// フルスクリーン化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ChangeWindowToFullscreen( void )
{
	//
	// 通常時の座標を保存
	//
	if ( m_flagFullscreen )
	{
		RestoreWindowFromFullscreen() ;
	}
	WINDOWPLACEMENT	wp ;
	if ( !::GetWindowPlacement( m_hWnd, &wp ) )
	{
		::GetWindowRect( m_hWnd, &wp.rcNormalPosition ) ;
	}
	m_rctNormalWndPos.left = wp.rcNormalPosition.left ;
	m_rctNormalWndPos.top = wp.rcNormalPosition.top ;
	m_rctNormalWndPos.right = wp.rcNormalPosition.right ;
	m_rctNormalWndPos.bottom = wp.rcNormalPosition.bottom ;
	//
	// モニタの判別と画面モード変更
	//
	SGLDisplayMode::MonitorHandle	hMonitor = nullptr ;
	SString			strDisplayName ;
	SGLImageRect	rectWndPos = m_rctNormalWndPos ;
	const wchar_t *	pwszDisplayName =
			m_displayMode.GetDisplayNameFromRect
					( strDisplayName, &rectWndPos, &hMonitor ) ;
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	m_flagProducerFullscreen = false ;
	if ( pwvp != nullptr )
	{
		m_flagProducerFullscreen =
			pwvp->OnChangeFullscreen
				( this, m_nBitsPerPixel, m_nFrequency,
					m_flagChangePhysicalMode, pwszDisplayName ) ;
		if ( m_flagProducerFullscreen )
		{
			m_flagFullscreen = true ;
			return	sglErrSuccess ;
		}
	}
	if ( m_flagChangePhysicalMode )
	{
		SGLDisplayMode::MatchingLevel	level =
			m_displayMode.TestDisplayMode
				( m_sizeVirtual.w, m_sizeVirtual.h,
					m_nBitsPerPixel, m_nFrequency, pwszDisplayName ) ;
		if ( level > SGLDisplayMode::matchNo )
		{
			m_displayMode.ChangeDisplayMode( pwszDisplayName ) ;
		}
	}
	//
	// 全画面表示
	//
	SGLImageRect	rectMonitor, rectVirtualWork ;
	if ( m_displayMode.GetMonitorRect
			( hMonitor, rectMonitor, rectVirtualWork ) )
	{
		rectMonitor.x = 0 ;
		rectMonitor.y = 0 ;
		rectMonitor.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
		rectMonitor.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
	}
	UpdateOptionalFlags( true ) ;
	::MoveWindow
		( m_hWnd, rectMonitor.x, rectMonitor.y,
					rectMonitor.w, rectMonitor.h, TRUE ) ;
	m_flagFullscreen = true ;
	return	sglErrSuccess ;
}

// フルスクリーン解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::RestoreWindowFromFullscreen( void )
{
	if ( m_displayMode.IsChangedDisplayMode() )
	{
		m_displayMode.RestoreDisplayMode() ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		pwvp->OnRestoreFullscreen( this ) ;
		m_flagProducerFullscreen = false ;
	}
	if ( m_flagFullscreen && !m_flagWMDestroying )
	{
		::MoveWindow
			( m_hWnd, m_rctNormalWndPos.left, m_rctNormalWndPos.top,
				m_rctNormalWndPos.right - m_rctNormalWndPos.left,
				m_rctNormalWndPos.bottom - m_rctNormalWndPos.top, TRUE ) ;
		UpdateOptionalFlags( false ) ;
	}
	m_flagFullscreen = false ;
	return	sglErrSuccess ;
}

// オプション機能フラグの反映
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateOptionalFlags( bool fFullscreen )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	LONG	lClassStyle = ::GetClassLong( m_hWnd, GCL_STYLE ) ;
	LONG	lStyle = ::GetWindowLong( m_hWnd, GWL_STYLE ) ;
	LONG	lExStyle = ::GetWindowLong( m_hWnd, GWL_EXSTYLE ) ;
	//
	if ( !(m_flagsOption & flagEnableIME) )
	{
		::ImmAssociateContext( m_hWnd, nullptr ) ;
	}
	else
	{
		::ImmAssociateContext( m_hWnd, m_hIMC ) ;
		if ( m_flagsOption & flagOpenIME )
		{
			::ImmSetOpenStatus( m_hIMC, TRUE ) ;
			m_flagsOption &= ~flagOpenIME ;
		}
	}
	::SetClassLong
		( m_hWnd, GCL_STYLE,
			ModifyWindowClassStyleOf( lClassStyle, fFullscreen ) ) ;
	::SetWindowLong
		( m_hWnd, GWL_STYLE,
			ModifyWindowStyleOf( lStyle, fFullscreen ) ) ;
	::SetWindowLong
		( m_hWnd, GWL_EXSTYLE,
			ModifyWindowExStyleOf( lExStyle, fFullscreen ) ) ;
	//
	::RedrawWindow
		( m_hWnd, nullptr, nullptr,
			RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW ) ;
	::SetWindowPos
		( m_hWnd, nullptr, 0, 0, 0, 0,
			(SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME) ) ;
	//
	if ( m_flagsOption & flagBlackBack )
	{
		::SetClassLongPtr
			( m_hWnd, GCLP_HBRBACKGROUND,
				(LONG_PTR) ::GetStockObject( BLACK_BRUSH ) ) ;
	}
	else
	{
		::SetClassLongPtr( m_hWnd, GCLP_HBRBACKGROUND, NULL ) ;
	}
	if ( !fFullscreen && !(m_flagsOption & flagVariableWindowSize) )
	{
		FitWindowClientSize() ;
	}
	else
	{
		UpdateClientDisplayPosition() ;
	}
	if ( m_flagsOption & flagDoMinimize )
	{
		::ShowWindow( m_hWnd, SW_MINIMIZE ) ;
	}
	else if ( m_flagsOption & flagDoMaximize )
	{
		if ( !fFullscreen
			&& (m_flagsOption & flagVariableWindowSize) )
		{
			::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED ) ;
		}
	}
	else if ( m_flagsOption & flagDoNormalize )
	{
		if ( !fFullscreen )
		{
			FitWindowClientSize() ;
		}
	}
	m_flagsOption &= ~(flagDoMinimize | flagDoMaximize | flagDoNormalize) ;
}

// ウィンドウのフレームサイズを計算
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::GetWindowFrameMargin( SGLRect& rectMargin )
{
	if ( m_flagsOption & flagVariableWindowSize )
	{
		rectMargin.left = ::GetSystemMetrics(SM_CXSIZEFRAME) ;
		rectMargin.right = rectMargin.left ;
		rectMargin.top = ::GetSystemMetrics(SM_CYSIZEFRAME) ;
		rectMargin.bottom = rectMargin.top ;
	}
	else if ( m_flagsOption & flagPopupWindow )
	{
		rectMargin.left = 0 ;
		rectMargin.right = 0 ;
		rectMargin.top = 0 ;
		rectMargin.bottom = 0 ;
	}
	else
	{
		rectMargin.left = ::GetSystemMetrics(SM_CXFIXEDFRAME) ;
		rectMargin.right = rectMargin.left ;
		rectMargin.top = ::GetSystemMetrics(SM_CYFIXEDFRAME) ;
		rectMargin.bottom = rectMargin.top ;
	}
	if ( m_refMenu != nullptr )
	{
		rectMargin.top += ::GetSystemMetrics(SM_CYMENU) ;
	}
	if ( !(m_flagsOption & flagPopupWindow) )
	{
		rectMargin.top += ::GetSystemMetrics(SM_CYCAPTION) ;
	}
}

// ウィンドウのクライアントサイズを論理サイズにフィットさせる
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::FitWindowClientSize( void )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	RECT	rectWindow, rectClient ;
	::GetWindowRect( m_hWnd, &rectWindow ) ;
	::GetClientRect( m_hWnd, &rectClient ) ;
	//
	SGLSize	sizeVirtual = m_sizeVirtual ;
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		sizeVirtual = pwvp->GetStandardDisplaySize() ;
	}
	SGLSize	sizeAdd ;
	sizeAdd.w = sizeVirtual.w - (rectClient.right - rectClient.left) ;
	sizeAdd.h = sizeVirtual.h - (rectClient.bottom - rectClient.top) ;
	if ( (sizeAdd.w == 0) && (sizeAdd.h == 0) )
	{
		return ;
	}
	SGLPoint	ptWindow( rectWindow.left, rectWindow.top ) ;
	SGLSize		sizeWindow
					( rectWindow.right - rectWindow.left + sizeAdd.w,
						rectWindow.bottom - rectWindow.top + sizeAdd.h ) ;
	m_displayMode.NormalizeWindowPos( ptWindow, sizeWindow, false ) ;
	::MoveWindow
		( m_hWnd, ptWindow.x, ptWindow.y,
			sizeWindow.w, sizeWindow.h, TRUE ) ;
}

// ウィンドウのクライアント表示座標を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateClientDisplayPosition( void )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	RECT	rectClient ;
	::GetClientRect( m_hWnd, &rectClient ) ;
	m_sizePhysical.w = rectClient.right - rectClient.left ;
	m_sizePhysical.h = rectClient.bottom - rectClient.top ;
	if ( !m_flagModeDisplay )
	{
		m_sizeVirtual = m_sizePhysical ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		pwvp->OnChangeVirtualViewSize
				( this, m_sizeVirtual.w, m_sizeVirtual.h ) ;
		pwvp->OnChangePhysicalViewSize
				( this, m_sizePhysical.w, m_sizePhysical.h ) ;
	}
}

// ウィンドウのレイアウトに基づいて位置を調整する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateWindowLayout( void )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	//
	// 親ウィンドウに対して自分自身の位置を調整する
	//
	SGLAbstractWindow *	pParentWnd = m_refParentWnd.GetReference() ;
	if ( (pParentWnd != nullptr) && (m_flagsLayout != 0) )
	{
		//
		// 基準座標
		//
		RECT	rectRelative ;
		RECT	rectWindow ;
		HWND	hwndRelative = pParentWnd->GetWindowHandle() ;
		::GetWindowRect( hwndRelative, &rectWindow ) ;
		if ( m_flagsLayout & layoutTypeWindow )
		{
			rectRelative = rectWindow ;
		}
		else
		{
			POINT	ptClient = { 0, 0 } ;
			::GetClientRect( hwndRelative, &rectRelative ) ;
			::ClientToScreen( hwndRelative, &ptClient ) ;
			rectRelative.left += ptClient.x ;
			rectRelative.top += ptClient.y ;
			rectRelative.right += ptClient.x ;
			rectRelative.bottom += ptClient.y ;
		}
		SGLSize	sizeRelative ;
		sizeRelative.w = rectRelative.right - rectRelative.left ;
		sizeRelative.h = rectRelative.bottom - rectRelative.top ;
		//
		// 自分自身のウィンドウ座標とフレーム幅を取得
		//
		RECT	rectThisWindow ;
		RECT	rectThisClient ;
		RECT	rectThisFrame ;
		POINT	ptThisClient = { 0, 0 } ;
		::GetWindowRect( m_hWnd, &rectThisWindow ) ;
		::GetClientRect( m_hWnd, &rectThisClient ) ;
		::ClientToScreen( m_hWnd, &ptThisClient ) ;
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
		SGLPoint	ptLayout = m_ptLayoutOffset ;
		SGLSize		sizeWnd = m_sizeLayoutOriginal ;
		//
		switch ( m_flagsLayout & layoutDockingMask )
		{
		case	layoutNothing:
			break ;
		case	layoutOffsetClient:
			ptLayout.x += rectRelative.left ;
			ptLayout.y += rectRelative.top ;
			sizeWnd.w = rectThisClient.right - rectThisClient.left ;
			sizeWnd.h = rectThisClient.bottom - rectThisClient.top ;
			break ;
		case	layoutDockingLeft:
		case	layoutDockingRight:
			switch ( m_flagsLayout & layoutAlignMask )
			{
			case	layoutAlignTop:
				ptLayout.y += rectRelative.top ;
				break ;
			case	layoutAlignCenter:
				ptLayout.y +=
					rectRelative.top
						+ (sizeRelative.h - sizeWnd.h) / 2 ;
				break ;
			case	layoutAlignBottom:
				ptLayout.y +=
					rectRelative.top
						+ (sizeRelative.h - sizeWnd.h) ;
				break ;
			case	layoutAlignAccording:
				sizeWnd.w = sizeRelative.h * sizeWnd.w / sizeWnd.h ;
				sizeWnd.h = sizeRelative.h ;
				ptLayout.y += rectRelative.top ;
				break ;
			}
			if ( !(m_flagsLayout & layoutTypeWindow) )
			{
				ptLayout.y -= rectThisFrame.top ;
			}
			if ( (m_flagsLayout & layoutDockingMask) == layoutDockingLeft )
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
		case	layoutDockingUpper:
		case	layoutDockingUnder:
			switch ( m_flagsLayout & layoutAlignMask )
			{
			case	layoutAlignLeft:
				ptLayout.x += rectRelative.left ;
				break ;
			case	layoutAlignCenter:
				ptLayout.x +=
					rectRelative.left
						+ (sizeRelative.w - sizeWnd.w) / 2 ;
				break ;
			case	layoutAlignRight:
				ptLayout.x +=
					rectRelative.left
						+ (sizeRelative.w - sizeWnd.w) ;
				break ;
			case	layoutAlignAccording:
				sizeWnd.h = sizeRelative.w * sizeWnd.h / sizeWnd.w ;
				sizeWnd.w = sizeRelative.h ;
				ptLayout.x += rectRelative.left ;
				break ;
			}
			if ( !(m_flagsLayout & layoutTypeWindow) )
			{
				ptLayout.x -= rectThisFrame.left ;
			}
			if ( (m_flagsLayout & layoutDockingMask) == layoutDockingUnder )
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
		sizeWnd.w += rectThisFrame.left + rectThisFrame.right ;
		sizeWnd.h += rectThisFrame.top + rectThisFrame.bottom ;
		//
		m_displayMode.NormalizeWindowPos( ptLayout, sizeWnd, true ) ;
		//
		::SetWindowPos
			( m_hWnd, HWND_TOP, ptLayout.x, ptLayout.y,
						sizeWnd.w, sizeWnd.h, dwFlags ) ;
	}
	//
	// 子ウィンドウのレイアウトを調整する
	//
	const size_t	countChildren = m_arrChildren.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		SGLGenericWindow *	pChildWnd = m_arrChildren.GetAt( i ) ;
		if ( pChildWnd != nullptr )
		{
			pChildWnd->UpdateWindowLayout() ;
		}
	}
	m_arrChildren.TrimEmpty() ;
	Unlock() ;
}

// レイヤードウィンドウ用フレームバッファサイズをウィンドウサイズに調整する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::ResizeFramebufferForLayeredWindow( void )
{
	if ( m_flagCreated && m_flagLayeredWindow )
	{
		RECT	rectClient ;
		::GetClientRect( m_hWnd, &rectClient ) ;
		//
		SGLSize	sizeWindow
			( rectClient.right - rectClient.left,
				rectClient.bottom - rectClient.top ) ;
		SGLSize	sizeBuffer = m_imgFrameColor.GetImageSize() ;
		if ( sizeBuffer != sizeWindow )
		{
			m_imgFrameColor.CreateImage
				( sizeWindow.w, sizeWindow.h,
					g_defaultImageFormat, 32, SGLImageObject::bufferForRenderTarget ) ;
			m_imgFrameDepth.CreateImage
				( sizeWindow.w, sizeWindow.h,
					formatImageZ, 32, SGLImageObject::bufferOnDeviceOnly ) ;
		}
		if ( m_pLayeredRenderer == nullptr )
		{
			m_pLayeredRenderer = new S3DRenderContext ;
		}
	}
}

// ウィンドウ更新
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateWindowTimeout( DWORD dwTimeout )
{
	if ( m_flagCreated
		&& ::GetUpdateRect( m_hWnd, nullptr, FALSE ) )
	{
		DWORD_PTR	dwResult;
		::SendMessageTimeout
			( m_hWnd, WM_PAINT, 0, 0, SMTO_NORMAL, dwTimeout, &dwResult ) ;
	}
}

// レイヤードウィンドウを更新する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateLayeredWindow( void )
{
	if ( m_flagCreated && m_flagLayeredWindow
				&& (m_apiUpdateLayeredWindow != nullptr) )
	{
		SGLImageWin32DIBitmap *	pDIB =
			SGLImageWin32DIBitmap::CommitDIB( &m_imgFrameColor ) ;
		if ( pDIB != nullptr )
		{
			SIZE	szWndDst =
				{ (LONG) m_imgFrameColor.GetImageWidth(),
						(LONG) m_imgFrameColor.GetImageHeight() } ;
			POINT	ptWndSrc = { 0, 0 } ;
			m_apiUpdateLayeredWindow
				( m_hWnd, nullptr, nullptr, &szWndDst,
					pDIB->m_hDC, &ptWndSrc, 0, &m_bfLayeredWindow, ULW_ALPHA ) ;
		}
	}
}

// アイコン読み込み
//////////////////////////////////////////////////////////////////////////////
HICON SGLGenericWindow::LoadMainIcon( void )
{
	SEnvironmentInterface *	pEnv = m_pEnv ;
	if ( pEnv == nullptr )
	{
		pEnv = SEnvironmentInterface::GetInstance() ;
		if ( pEnv == nullptr )
		{
			return	nullptr ;
		}
	}
	HICON	hIcon = nullptr ;
	SString	strIconID ;
	if ( pEnv->GetEnvironmentString( strIconID, L"script\\icon\\id" )
		|| pEnv->GetEnvironmentString( strIconID, L"cotopha\\icon\\id" ) )
	{
		SArray<char>	bufIconID ;
		const char *	pszIconID = strIconID.EncodeDefaultTo(bufIconID) ;
		HMODULE			hModule = ::GetModuleHandle(nullptr) ;
		hIcon = ::LoadIcon( hModule, pszIconID ) ;
		if ( hIcon == nullptr )
		{
			hIcon = (HICON) ::LoadImage
				( hModule, pszIconID, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) ;
		}
	}
	if ( (hIcon == nullptr)
		&& (pEnv->GetEnvironmentString( strIconID, L"script\\icon\\src" )
			|| pEnv->GetEnvironmentString( strIconID, L"cotopha\\icon\\src" )) )
	{
		SArray<char>	bufIconSrc ;
		hIcon = (HICON) ::LoadImage
			( nullptr, strIconID.EncodeDefaultTo(bufIconSrc),
				IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE ) ;
	}
	return	hIcon ;
}

// ウィンドウスタイル
//////////////////////////////////////////////////////////////////////////////
LONG SGLGenericWindow::ModifyWindowStyleOf
					( LONG lStyle, bool fFullscreen ) const
{
	lStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
	//
	if ( m_flagsOption & flagChildWindow )
	{
		lStyle |= WS_CHILD ;
	}
	else
	{
		lStyle &= ~WS_CHILD ;
	}
	if ( m_flagsOption & flagPopupWindow )
	{
		lStyle |= WS_POPUP ;
		lStyle &= ~WS_CAPTION ;
	}
	else
	{
		lStyle |= WS_CAPTION ;
		lStyle &= ~WS_POPUP ;
	}
	if ( m_flagsOption & flagInvisibleWindow )
	{
		lStyle &= ~WS_VISIBLE ;
	}
	else
	{
		lStyle |= WS_VISIBLE ;
	}
	if ( m_flagsOption
			& (flagAllowClose | flagAllowMinimize | flagAllowMaximize) )
	{
		lStyle |= WS_SYSMENU ;
	}
	else
	{
		lStyle &= ~WS_SYSMENU ;
	}
	if ( m_flagsOption & flagAllowMinimize )
	{
		lStyle |= WS_SYSMENU | WS_MINIMIZEBOX ;
	}
	else
	{
		lStyle &= ~(WS_MINIMIZEBOX) ;
	}
	lStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX) ;
	if ( !fFullscreen )
	{
		if ( m_flagsOption & flagVariableWindowSize )
		{
			lStyle |= WS_THICKFRAME ;
			if ( m_flagsOption & flagAllowMaximize )
			{
				lStyle |= WS_SYSMENU | WS_MAXIMIZEBOX ;
			}
		}
	}
	if ( fFullscreen )
	{
		lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX) | WS_POPUP ;
	}
	return	lStyle ;
}

// 拡張ウィンドウスタイル
//////////////////////////////////////////////////////////////////////////////
LONG SGLGenericWindow::ModifyWindowExStyleOf
				( LONG lExStyle, bool fFullscreen ) const
{
	lExStyle &= ~WS_EX_LAYERED ;
	if ( fFullscreen && (m_modeCooperation >= modeExclusive) )
	{
		lExStyle |= WS_EX_TOPMOST ;
	}
	else
	{
		lExStyle &= ~WS_EX_TOPMOST ;
		if ( m_flagLayeredWindow )
		{
			lExStyle |= WS_EX_LAYERED ;
		}
	}
	return	lExStyle ;
}

// クラススタイル
//////////////////////////////////////////////////////////////////////////////
LONG SGLGenericWindow::ModifyWindowClassStyleOf
					( LONG lClassStyle, bool fFullscreen ) const
{
	lClassStyle &= ~(CS_IME | CS_DBLCLKS | CS_NOCLOSE) ;
	if ( m_flagsOption & flagUseDblClick )
	{
		lClassStyle |= CS_DBLCLKS ;
	}
	if ( !(m_flagsOption & flagAllowClose) )
	{
		lClassStyle |= CS_NOCLOSE ;
	}
	return	lClassStyle ;
}

// 現在のウィンドウのスタイルからオプションフラグへ反映
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::ReflectOptionFlagsFromCurrentStyle( void )
{
	LONG	lClassStyle = ::GetClassLong( m_hWnd, GCL_STYLE ) ;
	LONG	lStyle = ::GetWindowLong( m_hWnd, GWL_STYLE ) ;
	LONG	lExStyle = ::GetWindowLong( m_hWnd, GWL_EXSTYLE ) ;
	//
	m_flagsOption &= ~(m_flagsOption | flagAllowClose) ;
	if ( lClassStyle & CS_DBLCLKS )
	{
		m_flagsOption |= flagUseDblClick ;
	}
	if ( lClassStyle & CS_NOCLOSE )
	{
		m_flagsOption |= flagAllowClose ;
	}
	m_flagsOption &=
		~(flagChildWindow | flagPopupWindow | flagInvisibleWindow
			| flagAllowMinimize | flagAllowMaximize | flagVariableWindowSize) ;
	if ( lStyle & WS_CHILD )
	{
		m_flagsOption |= flagChildWindow ;
	}
	if ( lStyle & WS_POPUP )
	{
		m_flagsOption |= flagPopupWindow ;
	}
	if ( !(lStyle & WS_VISIBLE) )
	{
		m_flagsOption |= flagInvisibleWindow ;
	}
	if ( lStyle & WS_MINIMIZEBOX )
	{
		m_flagsOption |= flagAllowMinimize ;
	}
	if ( lStyle & WS_MAXIMIZEBOX )
	{
		m_flagsOption |= flagAllowMaximize ;
	}
	if ( lStyle & WS_THICKFRAME )
	{
		m_flagsOption |= flagVariableWindowSize ;
	}
}

// ウィンドウキャプション書式
//////////////////////////////////////////////////////////////////////////////
SString SGLGenericWindow::FormatWindowCaption( void ) const
{
	if ( m_flagFPSonCaption )
	{
		SString	strDisplay ;
		strDisplay.Format
			( L"%s / FPS %d", (const wchar_t*) m_strCaption, m_nLastFPS ) ;
		return	strDisplay ;
	}
	else
	{
		return	m_strCaption ;
	}
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::DrawWindow( bool fOnWinThread )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	if ( m_flagLayeredWindow )
	{
		m_wvfFramework.DrawWindow
			( this, fOnWinThread, nullptr,
				&m_imgFrameColor, &m_imgFrameDepth ) ;
	}
	else
	{
		m_wvfFramework.DrawWindow( this, fOnWinThread ) ;
	}
}

// 表示反映処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::FlipView( bool fVSync, bool fOnWinThread )
{
	m_wvfFramework.FlipView( this, fVSync, fOnWinThread ) ;
	m_nCountRenderedFrames ++ ;
}

// 最近の FPS 取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLGenericWindow::GetRecentlyFramePerSecond( void ) const
{
	return	m_nLastFPS ;
}

// FPS をキャプションに表示
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::EnableCaptionWithFPS
			( bool flagEnable, bool flagTracePerfom )
{
	if ( flagEnable )
	{
		m_flagTracePerformance = flagTracePerfom ;
		m_flagFPSonCaption = true ;
	}
	else
	{
		m_flagFPSonCaption = false ;
	}
}

// パフォーマンス・リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::AttachPerformanceLogListener
	( SGLGenericWindow::PerformanceLogListener * pListener )
{
	Lock() ;
	m_pLogListener = pListener ;
	Unlock() ;
}

// UI スレッド判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsOnUIThread( void )
{
	SGLGenericWindow *	pParentWnd =
			ESLTypeCast<SGLGenericWindow>( m_refParentWnd.GetReference() ) ;
	if ( pParentWnd != nullptr )
	{
		return	pParentWnd->IsOnUIThread() ;
	}
	if ( !m_flagCreated )
	{
		return	false ;
	}
	return	m_threadUI.IsCurrentThread() ;
}

// UI スレッド上で関数呼び出し（同期実行）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CallMethodOnUIThread
	( SGLGenericWindow::PTR_METHOD pfnMethod, void * ptrParam )
{
	if ( IsOnUIThread() )
	{
		return	(this->*pfnMethod)( ptrParam ) ;
	}
	CallMethodOnUIThreadProcedure	method( this, pfnMethod, ptrParam, false ) ;
	SGLError	errResult = sglErrFailed ;
	if ( !PostUIThread( &method ) )
	{
		if( method.WaitDone( 100 ) == errTimeout )
		{
			Trace( "Timeout for SGLGenericWindow::CallMethodOnUIThread.\n" ) ;
			atomic_int_t	countLocked = UnlockAll() ;
			method.WaitDone() ;
			errResult = method.GetMethodResult() ;
			Relock( countLocked ) ;
		}
		else
		{
			errResult = method.GetMethodResult() ;
		}
	}
	return	errResult ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnCreateDisplay
	( const wchar_t * pwszWindowName,
		Window::CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	//
	// ウィンドウ作成
	//
	bool	flagInitSize = m_flagInitialSize
						&& (m_flagsOption & flagVariableWindowSize) ;
	m_flagModeDisplay = true ;
	m_flagFullscreen = false ;
	m_flagLayeredWindow = false ;
	m_flagRestoreFullscreen = false ;
	m_modeCooperation = mode ;
	m_sizeVirtual.w = nWidth ;
	m_sizeVirtual.h = nHeight ;
	m_sizePhysical = m_sizeVirtual ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	m_nBitsPerPixel = nBitsPerPixel ;
	m_nFrequency = nFrequency ;
	m_flagsLayout = 0 ;
	//
	SGLError	err =
		CreateWindowSimply( pwszWindowName, nWidth, nHeight, nullptr ) ;
	if ( err )
	{
		return	err ;
	}
	m_flagCreated = true ;
	AddWindowToChain() ;
	//
	UpdateClientDisplayPosition() ;
	//
	// ウィンドウスタイル更新
	//
	UpdateOptionalFlags( false ) ;
	//
	if ( !flagInitSize )
	{
		FitWindowClientSize() ;
	}
	if ( m_modeCooperation >= modeFullScreen )
	{
		ChangeWindowToFullscreen() ;
	}
	if ( !(m_flagsOption & flagInvisibleWindow) )
	{
		::ShowWindow( m_hWnd, SW_SHOW ) ;
		::SetForegroundWindow( m_hWnd ) ;
	}
	return	sglErrSuccess ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnCreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	//
	// ウィンドウ作成
	//
	m_flagModeDisplay = false ;
	m_flagFullscreen = false ;
	m_flagLayeredWindow = false ;
	m_flagRestoreFullscreen = false ;
	m_modeCooperation = Window::modeWindow ;
	m_sizeVirtual.w = nWidth ;
	m_sizeVirtual.h = nHeight ;
	m_sizePhysical = m_sizeVirtual ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_flagsLayout = 0 ;
	//
	HWND	hwndParent = nullptr ;
	if ( pParentWnd != nullptr )
	{
		hwndParent = pParentWnd->GetWindowHandle() ;
	}
	SGLError	err =
		CreateWindowSimply
			( pszWindowName, nWidth, nHeight, hwndParent ) ;
	if ( err )
	{
		return	err ;
	}
	m_flagCreated = true ;
	m_refParentWnd = pParentWnd ;
	AddWindowToChain() ;
	//
	UpdateClientDisplayPosition() ;
	//
	// 親ウィンドウ関連付け
	//
	SGLGenericWindow *
		pGenParentWnd = ESLTypeCast<SGLGenericWindow>( pParentWnd ) ;
	if ( pGenParentWnd != nullptr )
	{
		SSystem::QuickLock() ;
		pGenParentWnd->m_arrChildren.TrimEmpty() ;
		pGenParentWnd->m_arrChildren.Add( this ) ;
		SSystem::QuickUnlock() ;
	}
	//
	// ウィンドウスタイル更新
	//
	UpdateOptionalFlags( false ) ;
	//
	if ( nFlags & flagLayeredWindow )
	{
		m_bfLayeredWindow.BlendOp = AC_SRC_OVER ;
		m_bfLayeredWindow.BlendFlags = 0 ;
		m_bfLayeredWindow.AlphaFormat = 0 ;
		m_bfLayeredWindow.SourceConstantAlpha = 255 ;
		m_flagLayeredWindow = true ;
		//
		UpdateOptionalFlags( false ) ;
		ResizeFramebufferForLayeredWindow() ;
		//
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != nullptr )
		{
			if ( pwvp->EnableLayeredWindow( this, true ) )
			{
				m_flagLayeredWindow = false ;
				UpdateOptionalFlags( false ) ;
			}
		}
	}
	if ( !(m_flagsOption & flagInvisibleWindow) )
	{
		::ShowWindow( m_hWnd, SW_SHOW ) ;
		::SetForegroundWindow( m_hWnd ) ;
	}
	return	sglErrSuccess ;
}

// メッセージループ
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnLoop( void )
{
	m_flagQuitMessageLoop = false ;
	for ( ; ; )
	{
		if ( OnLoopDefault() )
		{
			break ;
		}
		m_csViewSync.Lock() ;
		if ( m_pViewSync != nullptr )
		{
			m_csViewSync.Unlock() ;
			//
			if ( OnLoopViewSync() )
			{
				break ;
			}
		}
		else
		{
			m_csViewSync.Unlock() ;
		}
	}
}

bool SGLGenericWindow::OnLoopDefault( void )
{
	STimeCounter	timerPaint ;
	STimeCounter	timerMsg ;
	double			msLPRenderTime = 0.0 ;
	const double	msReqFrame = 1000.0 / m_nRequestFPS ;
	const double	msFReqFrame = esl_fmax( 1.0, msReqFrame - 1.0 ) ;
	while ( !m_flagQuitMessageLoop )
	{
		MSG	msg ;
		while ( !m_flagQuitMessageLoop
			&& ::PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			if ( (msg.message == WM_PAINT)
				&& (m_timerWMTimer.GetRealTime() >= msFReqFrame) )
			{
				OnWMTimer() ;
			}
			timerMsg.Reset() ;
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
			//
			if ( msg.message == WM_QUIT )
			{
				ESLTrace( "Quit Window message loop by WM_QUIT.\n" ) ;
				return	true ;
			}
			if ( (msg.message == WM_PAINT) && (m_hWnd == msg.hwnd) )
			{
				double	msRenderTime = timerMsg.GetRealTime() ;
				timerPaint.Reset( timerMsg.GetTime() ) ;
				msLPRenderTime = msLPRenderTime * 0.5
										+ msRenderTime * 0.5 ;
				if ( (msLPRenderTime >= msReqFrame)
					|| (msRenderTime >= msReqFrame) )
				{
					if ( m_flagTracePerformance )
					{
						ESLTrace( "delay paint %f[ms]\n", msRenderTime ) ;
					}
				}
			}
			m_csViewSync.Lock() ;
			if ( m_pViewSync != nullptr )
			{
				m_csViewSync.Unlock() ;
				return	false ;
			}
			m_csViewSync.Unlock() ;
			//
			if ( (msLPRenderTime < msReqFrame)
				&& (timerPaint.GetRealTime() >= msFReqFrame) )
			{
				break ;
			}
		}
		if ( (msLPRenderTime < msReqFrame)
			&& (timerPaint.GetRealTime() >= msFReqFrame) )
		{
			// WM_TIMER は 30fps の精度しかないため
			// 強制的にタイマーを発生させる
			if ( m_timerWMTimer.GetRealTime() >= msFReqFrame )
			{
				OnWMTimer() ;
			}
			timerPaint.Reset() ;
			OnWMPaint() ;
			//
			double	msRenderTime = timerPaint.GetRealTime() ;
			msLPRenderTime = msLPRenderTime * 0.5
								+ msRenderTime * 0.5 ;
			continue ;
		}
		HANDLE	hQuit = m_signalQuit.GetHandle() ;
		DWORD	dwCount = (hQuit != nullptr) ? 1 : 0 ;
		DWORD	dwTimeout = 1 ;
		DWORD	dwWaitResult =
			::MsgWaitForMultipleObjects
				( dwCount, &hQuit, FALSE, dwTimeout, QS_ALLEVENTS ) ;
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{
			ESLTrace( "Quit Window message loop by Quit signal.\n" ) ;
			return	true ;
		}
	}
	return	true ;
}

bool SGLGenericWindow::OnLoopViewSync( void )
{
	SGLSecondaryViewProducer *	pViewProc = nullptr ;
	bool			fTimerMsg = false ;
	STimeCounter	timerPaint ;
	while ( !m_flagQuitMessageLoop )
	{
		MSG	msg ;
		while ( ::PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			timerPaint.Reset() ;
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
			//
			if ( msg.message == WM_QUIT )
			{
				ESLTrace( "Quit Window message loop by WM_QUIT.\n" ) ;
				return	true ;
			}
			if ( (msg.message == WM_TIMER) && (m_hWnd == msg.hwnd) )
			{
				fTimerMsg = (m_msecLastTimerInterval >= 1) ;
			}
			else if ( (msg.message == WM_PAINT) && (m_hWnd == msg.hwnd) )
			{
				fTimerMsg = false ;
				//
				if ( m_flagTracePerformance
					&& (timerPaint.GetRealTime() > 16.0) )
				{
					ESLTrace( "delay paint %f[ms]\n", timerPaint.GetRealTime() ) ;
				}
			}
			else
			{
				bool	fUpdateView = false ;
				m_csViewSync.Lock() ;
				if ( m_pViewSync == nullptr )
				{
					m_csViewSync.Unlock() ;
					return	false ;
				}
				pViewProc = ESLTypeCast<SGLSecondaryViewProducer>( m_pViewSync ) ;
				fUpdateView = (m_pViewSync->WaitForView(0) == sglErrSuccess) ;
				m_csViewSync.Unlock() ;
				//
				if ( fUpdateView )
				{
					// 更新タイミング
					if ( !fTimerMsg )
					{
						OnWMTimer() ;
					}
					timerPaint.Reset() ;
					OnWMPaint() ;
					fTimerMsg = false ;
					//
					if ( m_flagTracePerformance
						&& (timerPaint.GetRealTime() > 16.0) )
					{
						ESLTrace( "delay paint %f[ms]\n", timerPaint.GetRealTime() ) ;
					}
				}
			}
		}
		if ( m_signalQuit.Wait(0) == sglErrSuccess )
		{
			ESLTrace( "Quit Window message loop by Quit signal.\n" ) ;
			return	true ;
		}
		//
		// 更新タイミング待ち
		//
		bool	fUpdateView = false ;
		m_csViewSync.Lock() ;
		if ( m_pViewSync == nullptr )
		{
			m_csViewSync.Unlock() ;
			return	false ;
		}
		pViewProc = ESLTypeCast<SGLSecondaryViewProducer>( m_pViewSync ) ;
		fUpdateView = (m_pViewSync->WaitForView(1) == sglErrSuccess) ;
		m_csViewSync.Unlock() ;
		//
		if ( fUpdateView )
		{
			// 更新タイミング
			if ( !fTimerMsg )
			{
				OnWMTimer() ;
			}
			timerPaint.Reset() ;
			OnWMPaint() ;
			fTimerMsg = false ;
			//
			if ( m_flagTracePerformance
				&& (timerPaint.GetRealTime() > 16.0) )
			{
				ESLTrace( "delay paint %f[ms]\n", timerPaint.GetRealTime() ) ;
			}
		}
	}
	return	true ;
}

// ウィンドウ破棄関数
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnDestroyWindow( void * ptrParam )
{
	if ( m_flagCreated )
	{
		if ( m_apiUnregisterTouchWindow != nullptr )
		{
			m_apiUnregisterTouchWindow( m_hWnd ) ;
		}
		m_flagWMDestroying = true ;
		::DestroyWindow( m_hWnd ) ;
		m_flagWMDestroying = false ;
		m_hWnd = nullptr ;
		m_flagCreated = false ;
	}
	return	sglErrSuccess ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnSetOptionalFlags( void * ptrParam )
{
	m_flagsOption = *((uint64_t*)ptrParam) ;
	UpdateOptionalFlags( m_flagFullscreen ) ;
	return	sglErrSuccess ;
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnChangeCooperationLevel( void * ptrParam )
{
	m_modeCooperation = *((CooperationMode*)ptrParam) ;
	if ( m_modeCooperation >= modeFullScreen )
	{
		if ( !m_flagFullscreen )
		{
			ChangeWindowToFullscreen() ;
		}
		UpdateOptionalFlags( true ) ;
	}
	else
	{
		if ( m_flagFullscreen )
		{
			RestoreWindowFromFullscreen() ;
		}
		UpdateOptionalFlags( false ) ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnChangeDisplaySize( void * ptrParam )
{
	METHOD_PARAM_DISPLAY_SIZE *	pmpds = (METHOD_PARAM_DISPLAY_SIZE*) ptrParam ;
	bool	fChangeSize = ((uint32_t) m_sizeVirtual.w != pmpds->nWidth)
							| ((uint32_t) m_sizeVirtual.h != pmpds->nHeight) ;
	bool	fChangeMode = (m_nBitsPerPixel != pmpds->nBitsPerPixel)
							| (m_nFrequency != pmpds->nFrequency) ;
	if ( !fChangeSize & !fChangeMode )
	{
		return	sglErrSuccess ;
	}
	m_sizeVirtual.w = pmpds->nWidth ;
	m_sizeVirtual.h = pmpds->nHeight ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	m_nBitsPerPixel = pmpds->nBitsPerPixel ;
	m_nFrequency = pmpds->nFrequency ;
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		pwvp->OnChangeVirtualViewSize
				( this, m_sizeVirtual.w, m_sizeVirtual.h ) ;
	}
	if ( m_flagFullscreen )
	{
		RestoreWindowFromFullscreen() ;
	}
	if ( !m_flagModeDisplay )
	{
		UpdateWindowLayout() ;
	}
	FitWindowClientSize() ;
	//
	if ( m_modeCooperation >= modeFullScreen )
	{
		ChangeWindowToFullscreen() ;
	}
	return	sglErrSuccess ;
}

// ウィンドウ位置を更新する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnUpdateWindowLayout( void * ptrParam )
{
	UpdateWindowLayout() ;
	return	sglErrSuccess ;
}

// ユーザー入力処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnProcessUserInput( void * ptrParam )
{
	uint64_t	msecTimeout = *((int64_t*)ptrParam) ;
	uint64_t	msecStart = CurrentMilliSec() ;
	MSG		msg ;
	for ( int i = 0; i < 0x100; i ++ )
	{
		if ( ::PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
		else
		{
			break ;
		}
		if ( CurrentMilliSec() - msecStart >= msecTimeout )
		{
			break ;
		}
	}
	return	sglErrSuccess ;
}

// メニューを更新する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnUpdateMenu( void * ptrParam )
{
	::SetMenu( m_hWnd, m_hMenu ) ;
	::DrawMenuBar( m_hWnd ) ;
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::OnSetStereoDisplayMode( void * ptrParam )
{
	METHOD_PARAM_STEREO_DISPLAY_MODE *
		pmpsdm = (METHOD_PARAM_STEREO_DISPLAY_MODE*) ptrParam ;
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLWindowDisplayMethod *	pwdm = nullptr ;
	if ( !pwvp->IsSupportedStereoDisplayMode( this, pmpsdm->m_strMethodID ) )
	{
		pwdm = SGLWindowDisplayMethodProducer::NewDisplayMethod( pmpsdm->m_strMethodID ) ;
		if ( pwdm != nullptr )
		{
			SGLWindowDisplayMethodProducer *
				pwdmp = ESLTypeCast<SGLWindowDisplayMethodProducer>( pwvp ) ;
			if ( pwdmp == nullptr )
			{
				m_wvfFramework.ChangeWindowViewProducer
					( new SGLWindowDisplayMethodProducer( this, pwvp ) ) ;
				pwvp = m_wvfFramework.GetView() ;
				pwdmp = ESLTypeCast<SGLWindowDisplayMethodProducer>( pwvp ) ;
				ESLAssert( pwdmp != nullptr ) ;
				UpdateClientDisplayPosition() ;
			}
			pwdmp->SetDisplayMethod( pwdm ) ;
		}
		else
		{
			SGLWindowDisplayMethodProducer *
				pwdmp = ESLTypeCast<SGLWindowDisplayMethodProducer>( pwvp ) ;
			if ( pwdmp != nullptr )
			{
//				pwdmp->OnDetachedWindow( this ) ;
				//
				SGLWindowViewProducer *
					pwvpOrg = pwdmp->DetachWindowViewProducer() ;
				ESLAssert( pwvpOrg != nullptr ) ;
				m_wvfFramework.ChangeWindowViewProducer( pwvpOrg ) ;
				delete	pwdmp ;
				pwvp = pwvpOrg ;
				//
//				pwvp->OnAttachedWindow( this ) ;
				UpdateClientDisplayPosition() ;
			}
		}
	}
	SGLError	err =
		pwvp->SetStereoDisplayMode
			( this, pmpsdm->m_strMethodID, pmpsdm->m_nParam ) ;
	if ( !err )
	{
		UpdateClientDisplayPosition() ;
	}
	return	err ;
}

// メッセージ事前変換関数
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::PreTranslateMessage( MSG & msg )
{
	return	false ;
}

// ウィンドウ・プロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT SGLGenericWindow::WindowProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( (m_wpSuperClass != NULL)
		&& (uMsg != WM_PAINT)
		&& (m_nFreezePaint == 0)
		&& (m_timerLastPaint.GetTime() > 50)
		&& ::GetUpdateRect( m_hWnd, nullptr, FALSE ) )
	{
		::UpdateWindow( hWnd ) ;
	}
	if ( (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) )
	{
		if ( (GetMessageExtraInfo()
				& MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH )
		{
			return	0 ;
		}
		POINT	ptMouse ;
		POINTSTOPOINT(ptMouse,lParam) ;
		if ( uMsg == WM_MOUSEMOVE )
		{
			if ( m_flagMouseLeaved )
			{
				if ( m_apiTrackMouseEvent != nullptr )
				{
					TRACKMOUSEEVENT	tmeEvent ;
					tmeEvent.cbSize = sizeof(tmeEvent) ;
					tmeEvent.dwFlags = TME_LEAVE ;
					tmeEvent.hwndTrack = m_hWnd ;
					m_apiTrackMouseEvent( &tmeEvent ) ;
				}
				m_flagMouseLeaved = false ;
			}
		}
		else if ( uMsg == WM_MOUSEWHEEL )
		{
			::ScreenToClient( m_hWnd, &ptMouse ) ;
		}
		bool	fProcessed ;
		LockTrace( __FILE__, __LINE__ ) ;
		fProcessed =
			OnWMMouseEvent
				( m_pDirectMouseHandler, uMsg, wParam, ptMouse.x, ptMouse.y ) ;
		if ( !fProcessed )
		{
			SGLPoint	ptVirtualPos( ptMouse.x, ptMouse.y ) ;
			ClientPointToVirtual( ptVirtualPos ) ;
			//
			fProcessed =
				OnWMMouseEvent
					( m_pMouseHandler, uMsg, wParam,
							ptVirtualPos.x, ptVirtualPos.y ) ;
		}
		Unlock() ;
		if ( fProcessed )
		{
			return	0 ;
		}
	}
	else if ( uMsg == WM_MOUSELEAVE )
	{
		m_flagMouseLeaved = true ;
		OnWMMouseLeave() ;
		return	0 ;
	}
	else if ( uMsg == WM_PAINT )
	{
		OnWMPaint() ;
		return	0 ;
	}
	else if ( uMsg == WM_TIMER )
	{
		OnWMTimer() ;
		return	0 ;
	}
	else if ( (uMsg == WM_TOUCH)
			&& m_apiGetTouchInputInfo && m_apiCloseTouchInputHandle )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		//
		SArray<TOUCHINPUT>	aInputs ;
		UINT				cInputs = LOWORD(wParam) ;
		HTOUCHINPUT			hTouchInput = (HTOUCHINPUT) lParam ;
		TOUCHINPUT *		pInputs = aInputs.GetArray( cInputs ) ;
		if ( m_apiGetTouchInputInfo
			( hTouchInput, cInputs, pInputs, sizeof(TOUCHINPUT) ) )
		{
			for ( UINT i = 0; i < cInputs; i ++ )
			{
				const TOUCHINPUT&	ti = pInputs[i] ;
				if ( ti.dwFlags & TOUCHEVENTF_PRIMARY )
				{
					m_aTouchIDs.SetAt( 0, ti.dwID ) ;
					break ;
				}
			}
			for ( UINT i = 0; i < cInputs; i ++ )
			{
				const TOUCHINPUT&	ti = pInputs[i] ;
				OnWMTouchInput( ti ) ;
			}
			if ( m_aTouchIDs.GetLength() > 0 )
			{
				const DWORD *	pdwIDs = m_aTouchIDs.GetConstArray() ;
				size_t			nIDCount = m_aTouchIDs.GetLength() ;
				bool			flagMulti = false ;
				for ( size_t i = 1; i < nIDCount; i ++ )
				{
					if ( pdwIDs[i] != pdwIDs[0] )
					{
						flagMulti = true ;
						break ;
					}
				}
				if ( !flagMulti )
				{
					m_aTouchIDs.RemoveAll() ;
				}
			}
		}
		else
		{
			m_aTouchIDs.RemoveAll() ;
		}
		aInputs.FinishArray() ;
		m_apiCloseTouchInputHandle( hTouchInput ) ;
		//
		Unlock() ;
		return	0 ;
	}
	else if ( (uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST) )
	{
		bool	fProcessed = false ;
		switch ( uMsg )
		{
		case	WM_KEYDOWN:
			fProcessed = OnWMKeyDown( wParam, GetKeyContextFlag() ) ;
			break ;
		case	WM_KEYUP:
			fProcessed = OnWMKeyUp( wParam, GetKeyContextFlag() ) ;
			break ;
		case	WM_CHAR:
			if ( m_bytLeadChar != 0 )
			{
				char	ch[3] ;
				ch[0] = m_bytLeadChar ;
				ch[1] = (char) wParam ;
				ch[2] = 0 ;
				m_bytLeadChar = 0 ;
				//
				SString	strChar = ch ;
				fProcessed = OnWMChar( (uint16_t) strChar.GetAt(0) ) ;
			}
			else if ( ::IsDBCSLeadByte( (BYTE) wParam ) )
			{
				m_bytLeadChar = (BYTE) wParam ;
			}
			else if ( wParam & 0x80 )
			{
				char	ch[2] ;
				ch[0] = (char) wParam ;
				ch[1] = 0 ;
				//
				SString	strChar = ch ;
				fProcessed = OnWMChar( (uint16_t) strChar.GetAt(0) ) ;
			}
			else
			{
				fProcessed = OnWMChar( (uint16_t) wParam ) ;
			}
			break ;
		}
		if ( fProcessed )
		{
			return	0 ;
		}
	}
	else if ( uMsg == WM_IME_CHAR )
	{
		if ( wParam & 0xFF80 )
		{
			char	ch[3] ;
			if ( wParam & 0xFF00 )
			{
				ch[0] = (char) ((wParam >> 8) & 0xFF) ;
				ch[1] = (char) (wParam & 0xFF) ;
				ch[2] = 0 ;
			}
			else
			{
				ch[0] = (char) (wParam & 0xFF) ;
				ch[1] = 0 ;
			}
			SString	strChar = ch ;
			if ( OnWMChar( (uint16_t) strChar.GetAt(0) ) )
			{
				return	0 ;
			}
		}
		else
		{
			if ( OnWMChar( (uint16_t) wParam ) )
			{
				return	0 ;
			}
		}
	}
	else if ( uMsg == WM_IME_STARTCOMPOSITION )
	{
		if ( OnWMImeStartComposition( wParam, lParam ) )
		{
			return	0 ;
		}
	}
	else if ( uMsg == WM_IME_ENDCOMPOSITION )
	{
		if ( OnWMImeEndComposition( wParam, lParam ) )
		{
			return	0 ;
		}
	}
	else if ( uMsg == WM_IME_COMPOSITION )
	{
		if ( OnWMImeComposition( wParam, lParam ) )
		{
			return	0 ;
		}
	}
	else if ( uMsg == WM_SETFOCUS )
	{
		OnWMSetFocus() ;
	}
	else if ( uMsg == WM_KILLFOCUS )
	{
		OnWMKillFocus() ;
	}
	else if ( uMsg == WM_SETCURSOR )
	{
		if ( LOWORD(lParam) == HTCLIENT )
		{
			OnWMSetCursorClient() ;
		}
		else
		{
			if ( m_flagsOption & flagVariableWindowSize )
			{
				switch ( LOWORD(lParam) )
				{
				case	HTLEFT:
				case	HTRIGHT:
					::SetCursor( ::LoadCursor( nullptr, IDC_SIZEWE ) ) ;
					break ;
				case	HTTOP:
				case	HTBOTTOM:
					::SetCursor( ::LoadCursor( nullptr, IDC_SIZENS ) ) ;
					break ;
				case	HTTOPLEFT:
				case	HTBOTTOMRIGHT:
					::SetCursor( ::LoadCursor( nullptr, IDC_SIZENWSE ) ) ;
					break ;
				case	HTTOPRIGHT:
				case	HTBOTTOMLEFT:
					::SetCursor( ::LoadCursor( nullptr, IDC_SIZENESW ) ) ;
					break ;
				default:
					::SetCursor( ::LoadCursor( nullptr, IDC_ARROW ) ) ;
					break ;
				}
			}
			else
			{
				::SetCursor( ::LoadCursor( nullptr, IDC_ARROW ) ) ;
			}
		}
		return	0 ;
	}
	else if ( uMsg == WM_COMMAND )
	{
		UINT	nID = LOWORD( wParam ) ;
		UINT	nNCode = HIWORD( wParam ) ;
		const wchar_t *	pwszID = SGLWindowMenu::GetCommandIDOf( nID ) ;
		if ( pwszID != nullptr )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			if ( m_pCommandHandler != nullptr )
			{
				SString	strCmd = pwszID ;
				m_pCommandHandler->OnCommand
						( this, strCmd.GetConstArray(), 0, 0 ) ;
			}
			Unlock() ;
		}
		return	0 ;
	}
	else if ( uMsg == wmCallUIProcedure )
	{
		SProcedure *	pProc = (SProcedure*) lParam ;
		if ( pProc != nullptr )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			pProc->Prepare() ;
			pProc->Run() ;
			pProc->Finalize() ;
			Unlock() ;
		}
		return	0 ;
	}
	else if ( uMsg == wmCallRenderProcedure )
	{
		if ( !m_queOnRenderThread.IsEmpty() )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
			if ( pwvp != nullptr )
			{
				if ( pwvp->AttachViewThread( this ) != sglErrSuccess )
				{
					pwvp = nullptr ;
				}
			}
			m_queOnRenderThread.Flush() ;
			if ( pwvp != nullptr )
			{
				pwvp->DetachViewThread( this ) ;
			}
			Unlock() ;
		}
		else
		{
			ESLTrace( "empty on wmCallRenderProcedure.\n" ) ;
		}
		return	0 ;
	}
	else if ( uMsg == WM_SIZING )
	{
		if ( m_flagModeDisplay
			&& !(::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_MINIMIZE) )
		{
			RECT *	pRect = (RECT*) lParam ;
			RECT	rectWindow, rectClient ;
			::GetWindowRect( m_hWnd, &rectWindow ) ;
			::GetClientRect( m_hWnd, &rectClient ) ;
			//
			SGLSize	sizeClient
				( rectClient.right - rectClient.left,
					rectClient.bottom - rectClient.top ) ;
			SGLSize	sizeWindow
				( rectWindow.right - rectWindow.left,
					rectWindow.bottom - rectWindow.top ) ;
			//
			sizeClient.w = pRect->right - pRect->left
									- (sizeWindow.w - sizeClient.w) ;
			sizeClient.h = pRect->bottom - pRect->top
									- (sizeWindow.h - sizeClient.h) ;
			//
			SGLSize	sizeVirtual = m_sizeVirtual ;
			SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
			if ( pwvp != nullptr )
			{
				sizeVirtual = pwvp->GetStandardDisplaySize() ;
			}
			SGLSize	sizeNormalized = sizeClient ;
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
			SGLSize	sizeDelta
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
			SGLPoint	ptWindow( pRect->left, pRect->top ) ;
			sizeWindow.w = pRect->right - pRect->left ;
			sizeWindow.h = pRect->bottom - pRect->top ;
			//
			m_displayMode.NormalizeWindowPos( ptWindow, sizeWindow, false ) ;
			//
			pRect->left = ptWindow.x ;
			pRect->top = ptWindow.y ;
			pRect->right = ptWindow.x + sizeWindow.w ;
			pRect->bottom = ptWindow.y + sizeWindow.h ;
			return	TRUE ;
		}
	}
	else if ( uMsg == WM_SIZE )
	{
		if ( m_flagFirstWindowSize )
		{
			m_flagFirstWindowSize = false ;
			//
			if ( AeroIsCompositionEnabled() )
			{
				// Aero モードの際、初期の OpenGL 表示範囲が
				// 正しくないことがあるため
				// 強制的に Aero のバックバッファを更新する
				RECT	rectWindow ;
				if ( ::GetWindowRect( m_hWnd, &rectWindow ) )
				{
					::MoveWindow
						( m_hWnd, rectWindow.left, rectWindow.top,
							rectWindow.right - rectWindow.left + 1,
							rectWindow.bottom - rectWindow.top + 1, TRUE ) ;
					::UpdateWindow( m_hWnd ) ;
					::MoveWindow
						( m_hWnd, rectWindow.left, rectWindow.top,
							rectWindow.right - rectWindow.left,
							rectWindow.bottom - rectWindow.top, TRUE ) ;
					::UpdateWindow( m_hWnd ) ;
				}
			}
		}
		//
		UpdateClientDisplayPosition() ;
		//
		if ( !(m_flagsOption & flagVariableWindowSize)
						&& (m_modeCooperation < modeFullScreen) )
		{
			FitWindowClientSize() ;
		}
		else
		{
			PostUpdate( nullptr ) ;
		}
		if ( m_flagLayeredWindow )
		{
			ResizeFramebufferForLayeredWindow() ;
		}
		UpdateWindowLayout() ;
		//
		RECT	rectClient ;
		bool	flagGotClient = false ;
		if ( ::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_MINIMIZE )
		{
			WINDOWPLACEMENT	wp ;
			if ( ::GetWindowPlacement( m_hWnd, &wp ) )
			{
				rectClient = wp.rcNormalPosition ;
				flagGotClient = true ;
			}
		}
		else if ( ::GetClientRect( m_hWnd, &rectClient ) )
		{
			flagGotClient = true ;
		}
		if ( flagGotClient )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			if ( m_pCommandHandler != nullptr )
			{
				SString		strCmd = SysCommandId::WindowSizeChanged ;
				uint32_t	wClient = rectClient.right - rectClient.left ;
				uint32_t	hClient = rectClient.bottom - rectClient.top ;
				m_pCommandHandler->OnCommand
					( this, strCmd.GetConstArray(),
						(((int64_t) hClient) << 32) | wClient, 0 ) ;
			}
			Unlock() ;
		}
	}
	else if ( uMsg == WM_MOVE )
	{
		UpdateWindowLayout() ;
		//
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != nullptr )
		{
			pwvp->OnMovedWindow( this ) ;
		}
	}
	else if ( uMsg == WM_GETMINMAXINFO )
	{
		if ( m_flagModeDisplay )
		{
			LPMINMAXINFO	lpmmi = (LPMINMAXINFO)lParam ;
			lpmmi->ptMinTrackSize.x = m_sizeVirtual.w / 2 ;
			lpmmi->ptMinTrackSize.y = m_sizeVirtual.h / 2 ;
			return	0 ;
		}
	}
	else if ( (uMsg == MM_JOY1BUTTONDOWN)
			|| (uMsg == MM_JOY1BUTTONUP)
			|| (uMsg == MM_JOY1MOVE)
			|| (uMsg == MM_JOY2BUTTONDOWN)
			|| (uMsg == MM_JOY2BUTTONUP)
			|| (uMsg == MM_JOY2MOVE) )
	{
		Lock() ;
		if ( m_pCommandHandler != nullptr )
		{
			SString	strCmd = SysCommandId::WindowPollJoyStick ;
			m_pCommandHandler->OnCommand( this, strCmd.GetConstArray(), 0, 0 ) ;
		}
		Unlock() ;
		return	0 ;
	}
	else if ( uMsg == WM_ACTIVATE )
	{
		if ( !m_flagWMDestroying )
		{
			if ( (wParam & 0xFFFF) == WA_INACTIVE )
			{
				DWORD	dwActiveProcID, dwProcessID ;
				HWND	hWndFG = (HWND) lParam ;
				if ( hWndFG == nullptr )
				{
					hWndFG = ::GetForegroundWindow() ;
				}
				::GetWindowThreadProcessId( hWndFG, &dwActiveProcID ) ;
				::GetWindowThreadProcessId( m_hWnd, &dwProcessID ) ;
				if ( (dwActiveProcID != dwProcessID) && m_flagActivated )
				{
					m_flagActivated = false ;
					m_flagRestoreFullscreen = false ;
					if ( m_flagFullscreen )
					{
						m_flagRestoreFullscreen = true ;
						RestoreWindowFromFullscreen() ;
						::ShowWindow( m_hWnd, SW_SHOWMINIMIZED ) ;
					}
					LockTrace( __FILE__, __LINE__ ) ;
					if ( m_pCommandHandler != nullptr )
					{
						SString	strCmd = SysCommandId::WindowInactive ;
						m_pCommandHandler->OnCommand
							( this, strCmd.GetConstArray(), 0, 0 ) ;
					}
					Unlock() ;
				}
			}
			else
			{
				if ( !m_flagActivated )
				{
					m_flagActivated = true ;
					if ( m_flagRestoreFullscreen )
					{
						::ShowWindow( m_hWnd, SW_RESTORE ) ;
						ChangeWindowToFullscreen() ;
						m_flagRestoreFullscreen = false ;
					}
					LockTrace( __FILE__, __LINE__ ) ;
					if ( m_pCommandHandler != nullptr )
					{
						SString	strCmd = SysCommandId::WindowActive ;
						m_pCommandHandler->OnCommand
							( this, strCmd.GetConstArray(), 0, 0 ) ;
					}
					Unlock() ;
				}
			}
		}
	}
	else if ( uMsg == WM_CLOSE )
	{
		OnWMClose() ;
		return	0 ;
	}
	else if ( uMsg == WM_SYSCOMMAND )
	{
		switch( wParam )
		{
		case	SC_MOVE:
		case	SC_SIZE:
		case	SC_TASKLIST:
			if ( m_modeCooperation >= modeFullScreen )
			{
				return	0 ;
			}
			break ;

		case	SC_RESTORE:
			if ( m_modeCooperation >= modeFullScreen )
			{
				return	0 ;
			}
			break ;

		case	SC_MINIMIZE:
			if ( m_flagsOption & flagAllowMinimize )
			{
				break ;
			}
			return	0 ;

		case	SC_MAXIMIZE:
			if ( (m_modeCooperation < modeFullScreen)
					&& (m_flagsOption & flagVariableWindowSize)
					&& (m_flagsOption & flagAllowMaximize) )
			{
				break ;
			}
			return	0 ;

		case	SC_MOUSEMENU:
			return	0 ;

		case	SC_SCREENSAVE:
			if ( m_flagsOption & flagGrantScreenSave )
			{
				break ;
			}
			return	0 ;

		case	SC_MONITORPOWER:
			if ( m_flagsOption & flagGrantMonitorSave )
			{
				break ;
			}
			return	0 ;

		case	SC_CLOSE:
			if ( m_flagsOption & flagAllowClose )
			{
				break ;
			}
			return	0 ;
		}
	}
	else if ( uMsg == WM_POWERBROADCAST )
	{
		if ( !(m_flagsOption & flagGrantPowerSuspend) )
		{
			return	BROADCAST_QUERY_DENY ;
		}
	}
	else if ( uMsg == WM_SYSKEYDOWN )
	{
		if ( (m_modeCooperation >= Window::modeFullScreen)
					&& (m_flagsOption & flagAllowClose) )
		{
			int	nVirtKey = (int) wParam ;
			if ( (nVirtKey == VK_F4) && (GetKeyState(VK_MENU) != 0) )
			{
				::PostMessage( hWnd, WM_CLOSE, 0, 0 ) ;
				return	0 ;
			}
		}
	}
	else if ( uMsg == WM_CREATE )
	{
		m_flagMouseLeaved = true ;
		m_flagActivated = false ;
		m_bytLeadChar = 0 ;
		//
		::SetTimer( m_hWnd, 1, 16, nullptr ) ;
	}
	else if ( uMsg == WM_DESTROY )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		SGLGenericWindow *		pParentWnd =
			ESLTypeCast<SGLGenericWindow>( m_refParentWnd.GetReference() ) ;
		if ( pParentWnd != nullptr )
		{
			ssize_t	iChild = pParentWnd->m_arrChildren.FindPtr( this ) ;
			if ( iChild >= 0 )
			{
				pParentWnd->m_arrChildren.RemoveAt( iChild ) ;
			}
		}
		Unlock() ;
		//
		if ( m_flagFullscreen )
		{
			RestoreWindowFromFullscreen() ;
		}
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != nullptr )
		{
			pwvp->OnDetachedWindow( this ) ;
			m_flagProducerFullscreen = false ;
		}
	}
	else if ( uMsg == WM_NCDESTROY )
	{
		ESLTrace( "WM_NCDESTROY\n" ) ;
		if ( m_flagWndClassOwner )
		{
			::UnregisterClass
				( m_cstrClassName.GetConstArray(), ::GetModuleHandle(nullptr) ) ;
			m_flagWndClassOwner = false ;
		}
		DetachWindowFromChain() ;
		m_flagCreated = false ;
		m_signalQuit.SetSignal() ;
		ESLTrace( "Window quit signal for %08X\n", this ) ;
	}
	if ( m_wpSuperClass != nullptr )
	{
		return	CallWindowProc( m_wpSuperClass, hWnd, uMsg, wParam, lParam ) ;
	}
	else
	{
		return	::DefWindowProc( hWnd, uMsg, wParam, lParam ) ;
	}
}

LRESULT __stdcall SGLGenericWindow::WindowCallbackProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SGLGenericWindow *	pWnd = nullptr ;
	if ( uMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT	pcs = (LPCREATESTRUCT) lParam ;
		pWnd = (SGLGenericWindow*) pcs->lpCreateParams ;
		::SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR) pWnd ) ;
		pWnd->m_hWnd = hWnd ;
	}
	else
	{
		pWnd = (SGLGenericWindow*) ::GetWindowLongPtr( hWnd, GWLP_USERDATA ) ;
		if ( pWnd != nullptr )
		{
			if ( pWnd->m_hWnd != hWnd )
			{
				pWnd = nullptr ;
			}
		}
	}
	if ( pWnd != nullptr )
	{
		MSG		msg ;
		LONG	lmpt = ::GetMessagePos() ;
		msg.hwnd = hWnd ;
		msg.message = uMsg ;
		msg.wParam = wParam ;
		msg.lParam = lParam ;
		msg.time = ::GetMessageTime() ;
		POINTSTOPOINT(msg.pt,lmpt) ;
		if ( !pWnd->PreTranslateMessage( msg ) )
		{
			return	pWnd->WindowProc
						( hWnd, msg.message, msg.wParam, msg.lParam ) ;
		}
	}
	return	::DefWindowProc( hWnd, uMsg, wParam, lParam ) ;
}

// キーコンテキストフラグを取得する
//////////////////////////////////////////////////////////////////////////////
int64_t SGLGenericWindow::GetKeyContextFlag( void )
{
	int64_t	nFlags = 0 ;
	if ( ::GetKeyState( VK_CAPITAL ) & 0x01 )
	{
		nFlags |= vkeyContextCapital ;
	}
	if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
	{
		nFlags |= vkeyContextShift ;
	}
	if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
	{
		nFlags |= vkeyContextControl ;
	}
	if ( ::GetKeyState( VK_MENU ) & 0x80 )
	{
		nFlags |= vkeyContextMenu ;
	}
	return	nFlags ;
}

// 論理座標と物理（クライアント）座標変換
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::ClientPointToVirtual( SGLPoint& pos )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		S2DDVector	vPoint( pos.x, pos.y ) ;
		pwvp->PhysicalToVirtualPosition( vPoint ) ;
		//
		pos.x = eslRoundR32ToInt( (float32_t) vPoint.x ) ;
		pos.y = eslRoundR32ToInt( (float32_t) vPoint.y ) ;
	}
}

void SGLGenericWindow::VirtualPointToClient( SGLPoint& pos )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		S2DDVector	vPoint( pos.x, pos.y ) ;
		pwvp->VirtualToPhysicalPosition( vPoint ) ;
		//
		pos.x = eslRoundR32ToInt( (float32_t) vPoint.x ) ;
		pos.y = eslRoundR32ToInt( (float32_t) vPoint.y ) ;
	}
}

void SGLGenericWindow::VirtualSizeToClient( SGLSize& size )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != nullptr )
	{
		SGLAffine	affine ;
		pwvp->GetAffineVirtualToPhysical( affine ) ;
		//
		S2DDVector	vSize( size.w, size.h ) ;
		affine.a13 = 0.0f ;
		affine.a23 = 0.0f ;
		affine.TransformVectors( &vSize, &vSize, 1 ) ;
		//
		size.w = eslRoundR32ToInt( (float32_t) fabs( vSize.x ) ) ;
		size.h = eslRoundR32ToInt( (float32_t) fabs( vSize.y ) ) ;
	}
}

// Aero 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::AeroIsCompositionEnabled( void )
{
	HMODULE	hModule = ::GetModuleHandle( "dwmapi.dll" ) ;
	if ( hModule != nullptr )
	{
		typedef HRESULT (WINAPI *API_DwmIsCompositionEnabled)( BOOL *pfEnabled ) ;
		API_DwmIsCompositionEnabled
			apiDwmIsCompositionEnabled =
				(API_DwmIsCompositionEnabled)
					::GetProcAddress( hModule, "DwmIsCompositionEnabled" ) ;
		if ( apiDwmIsCompositionEnabled != nullptr )
		{
			BOOL	bEnabled = 0 ;
			if ( apiDwmIsCompositionEnabled( &bEnabled ) == S_OK )
			{
				return	(bEnabled != FALSE) ;
			}
		}
	}
	return	false ;
}

// WM_MOUSEFIRST～WM_MOUSELAST
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMMouseEvent
	( SGLMouseInterface * pMouse,
		UINT uMsg, WPARAM wParam, int32_t xPos, int32_t yPos )
{
	if ( pMouse == nullptr )
	{
		return	false ;
	}
	int64_t	nFlags = 0 ;
	int32_t	zDelta ;
	#if	defined(__DEBUG__)
	if ( GetKeyContextFlag() == (vkeyContextShift | vkeyContextCapital) )
	{
		nFlags = 1 ;
	}
	#endif

	switch ( uMsg )
	{
	case	WM_MOUSEMOVE:
		return	pMouse->OnMouseMove( this, xPos, yPos, nFlags ) ;

	case	WM_MOUSEWHEEL:
		zDelta = (SWORD) (wParam >> 16) ;
		zDelta = zDelta * SGLMouseInterface::WheelDeltaUnit / WHEEL_DELTA ;
		return	pMouse->OnMouseWheel( this, zDelta, xPos, yPos, nFlags ) ;

	case	WM_LBUTTONDOWN:
		nFlags |= (SGLMouseInterface::LeftButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDown( this, xPos, yPos, nFlags ) ;

	case	WM_LBUTTONUP:
		nFlags |= (SGLMouseInterface::LeftButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonUp( this, xPos, yPos, nFlags ) ;

	case	WM_LBUTTONDBLCLK:
		nFlags |= (SGLMouseInterface::LeftButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDblClk( this, xPos, yPos, nFlags ) ;

	case	WM_RBUTTONDOWN:
		nFlags |= (SGLMouseInterface::RightButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDown( this, xPos, yPos, nFlags ) ;

	case	WM_RBUTTONUP:
		nFlags |= (SGLMouseInterface::RightButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonUp( this, xPos, yPos, nFlags ) ;

	case	WM_RBUTTONDBLCLK:
		nFlags |= (SGLMouseInterface::RightButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDblClk( this, xPos, yPos, nFlags ) ;

	case	WM_MBUTTONDOWN:
		nFlags |= (SGLMouseInterface::MiddleButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDown( this, xPos, yPos, nFlags ) ;

	case	WM_MBUTTONUP:
		nFlags |= (SGLMouseInterface::MiddleButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonUp( this, xPos, yPos, nFlags ) ;

	case	WM_MBUTTONDBLCLK:
		nFlags |= (SGLMouseInterface::MiddleButtonID
						<< SGLMouseInterface::ButtonIDShifter) ;
		return	pMouse->OnButtonDblClk( this, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

// WM_MOUSELEAVE
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMMouseLeave( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pDirectMouseHandler != nullptr )
	{
		m_pDirectMouseHandler->OnMouseLeave( this, 0 ) ;
		#if	defined(__DEBUG__)
		m_pDirectMouseHandler->OnMouseLeave( this, 1 ) ;
		#endif
	}
	if ( m_pMouseHandler != nullptr )
	{
		m_pMouseHandler->OnMouseLeave( this, 0 ) ;
		#if	defined(__DEBUG__)
		m_pMouseHandler->OnMouseLeave( this, 1 ) ;
		#endif
	}
	Unlock() ;
}

// WM_TOUCH
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMTouchInput( const TOUCHINPUT& ti )
{
	POINT	ptMouse = { (ti.x + 50) / 100, (ti.y + 50) / 100 } ;
	::ScreenToClient( m_hWnd, &ptMouse ) ;
	SGLPoint	ptVirtualPos( ptMouse.x, ptMouse.y ) ;
	ClientPointToVirtual( ptVirtualPos ) ;
	int64_t		nFlags = (SGLMouseInterface::LeftButtonID
							<< SGLMouseInterface::ButtonIDShifter)
						| SGLMouseInterface::TouchFlag ;
	ssize_t		iMouse = -1 ;
	for ( size_t i = 0; i < m_aTouchIDs.GetLength(); i ++ )
	{
		if ( m_aTouchIDs.At(i) == ti.dwID )
		{
			iMouse = (ssize_t) i ;
			break ;
		}
	}
	if ( iMouse < 0 )
	{
		if ( ti.dwFlags & TOUCHEVENTF_PRIMARY )
		{
			iMouse = 0 ;
		}
		else
		{
			iMouse = (ssize_t) m_aTouchIDs.GetLength() ;
			if ( iMouse <= 0 )
			{
				iMouse = 1 ;
			}
		}
		m_aTouchIDs.SetAt( (size_t) iMouse, ti.dwID ) ;
	}
	nFlags |= (iMouse & SGLMouseInterface::MouseIDMask) ;
	//
	if ( ti.dwFlags & TOUCHEVENTF_DOWN )
	{
		bool	fProcessed = false ;
		if ( m_pDirectMouseHandler != nullptr )
		{
			fProcessed =
				m_pDirectMouseHandler->OnButtonDown
					( this, ptMouse.x, ptMouse.y, nFlags ) ;
		}
		if ( !fProcessed && (m_pMouseHandler != nullptr) )
		{
			m_pMouseHandler->OnButtonDown
				( this, ptVirtualPos.x, ptVirtualPos.y, nFlags ) ;
		}
	}
	if ( ti.dwFlags & TOUCHEVENTF_MOVE )
	{
		bool	fProcessed = false ;
		if ( m_pDirectMouseHandler != nullptr )
		{
			fProcessed =
				m_pDirectMouseHandler->OnMouseMove
					( this, ptMouse.x, ptMouse.y, nFlags ) ;
		}
		if ( !fProcessed && (m_pMouseHandler != nullptr) )
		{
			m_pMouseHandler->OnMouseMove
				( this, ptVirtualPos.x, ptVirtualPos.y, nFlags ) ;
		}
	}
	if ( ti.dwFlags & TOUCHEVENTF_UP )
	{
		bool	fProcessed = false ;
		if ( m_pDirectMouseHandler != nullptr )
		{
			fProcessed =
				m_pDirectMouseHandler->OnButtonUp
					( this, ptMouse.x, ptMouse.y, nFlags ) ;
		}
		if ( !fProcessed && (m_pMouseHandler != nullptr) )
		{
			m_pMouseHandler->OnButtonUp
				( this, ptVirtualPos.x, ptVirtualPos.y, nFlags ) ;
		}
		if ( m_pDirectMouseHandler != nullptr )
		{
			m_pDirectMouseHandler->OnMouseLeave( this, nFlags ) ;
		}
		if ( m_pMouseHandler != nullptr )
		{
			m_pMouseHandler->OnMouseLeave( this, nFlags ) ;
		}
		m_aTouchIDs.SetAt( (size_t) iMouse, m_aTouchIDs.At(0) ) ;
	}
}

// WM_PAINT
//////////////////////////////////////////////////////////////////////////////
static LONG WINAPI MyUnhandledExceptionFilter( EXCEPTION_POINTERS * excpinf ) ;
void SGLGenericWindow::OnWMPaint( void )
{
	if ( m_nFreezePaint > 0 )
	{
		return ;
	}
	if ( !m_flagWMPaintEntered )
	{
		m_flagWMPaintEntered = true ;
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != nullptr )
		{
			S3DRenderDevice *	pDevice = pwvp->GetRenderDevice() ;
			LockTrace( __FILE__, __LINE__ ) ;
			if ( pwvp->AttachViewThread( this ) == sglErrSuccess )
			{
				// WM_PAINT ハンドラがシステム側のローカル例外ハンドラで処理され
				// デバッガで例外発生時に発生場所を特定できないので
				#if	!defined(__DEBUG__)
				__try
				#endif
				{
					m_tidAttachedViewThread = SThread::GetCurrentId() ;
					if ( pDevice != nullptr )
					{
						pDevice->BeginFramePerformanceLog() ;
					}
					DrawWindow( true ) ;

					if ( pDevice != nullptr )
					{
						pDevice->EndFramePerformanceLog() ;
					}
					FlipView( false, true ) ;
				}
				#if	!defined(__DEBUG__)
				__except( MyUnhandledExceptionFilter( GetExceptionInformation () ) )
				{
					ESLTrace( "exception at Window drawing.\n" ) ;
					FlipView( false, true ) ;
				}
				#endif
				if ( !m_queOnRenderThread.IsEmpty() )
				{
					m_queOnRenderThread.Flush() ;
				}
				m_tidAttachedViewThread = SThread::InvalidId ;
				pwvp->DetachViewThread( this ) ;
			}
			Unlock() ;
		}
		if ( m_flagLayeredWindow )
		{
			UpdateLayeredWindow() ;
		}
		if ( ::GetUpdateRect( m_hWnd, nullptr, FALSE ) )
		{
			::ValidateRect( m_hWnd, nullptr ) ;
			/*
			PAINTSTRUCT	ps ;
			HDC	hdc = ::BeginPaint( m_hWnd, &ps ) ;
			::EndPaint( m_hWnd, &ps ) ;
			*/
		}
		m_timerLastPaint.Reset() ;
		m_flagWMPaintEntered = false ;
	}
	else
	{
		ESLTrace( "warning: re-entered WM_PAINT.\n" ) ;
	}
}

LONG WINAPI MyUnhandledExceptionFilter( EXCEPTION_POINTERS * excpinf )
{
	PEXCEPTION_RECORD	pexcp = excpinf->ExceptionRecord ;
	PCONTEXT			pcontx = excpinf->ContextRecord ;
	//
#if	defined(__PROCESSOR_INTEL_X86_64__)
	static const char	szBasicInfo[] =
		"例外エラーコード：%08X\r\n"
		"レジスタダンプ；\r\n"
		"CS:RIP  = %04X:%16I64X\r\n"
		"RAX = %16I64X, RBX = %16I64X, RCX = %16I64X, RDX = %16I64X\r\n"
		"RSI = %16I64X, RDI = %16I64X, RBP = %16I64X, RSP = %16I64X\r\n"
		"DS = %04X, ES = %04X, FS = %04X, GS = %04X, SS = %04X\r\n"
		"\r\n\r\n" ;
	//
	char	bufDump[0x400] ;
	::sprintf_s
		( bufDump, 0x400, szBasicInfo,
			pexcp->ExceptionCode,
			pcontx->SegCs, pcontx->Rip,
			pcontx->Rax, pcontx->Rbx, pcontx->Rcx, pcontx->Rdx,
			pcontx->Rsi, pcontx->Rdi, pcontx->Rbp, pcontx->Rsp,
			pcontx->SegDs, pcontx->SegEs,
			pcontx->SegFs, pcontx->SegGs, pcontx->SegSs ) ;
#else
	static const char	szBasicInfo[] =
		"例外エラーコード：%08X\r\n"
		"レジスタダンプ；\r\n"
		"CS:EIP  = %04X:%08X\r\n"
		"EAX = %08X, EBX = %08X, ECX = %08X, EDX = %08X\r\n"
		"ESI = %08X, EDI = %08X, EBP = %08X, ESP = %08X\r\n"
		"DS = %04X, ES = %04X, FS = %04X, GS = %04X, SS = %04X\r\n"
		"\r\n\r\n" ;
	//
	char	bufDump[0x400] ;
	::sprintf_s
		( bufDump, 0x400, szBasicInfo,
			pexcp->ExceptionCode,
			pcontx->SegCs, pcontx->Eip,
			pcontx->Eax, pcontx->Ebx, pcontx->Ecx, pcontx->Edx,
			pcontx->Esi, pcontx->Edi, pcontx->Ebp, pcontx->Esp,
			pcontx->SegDs, pcontx->SegEs,
			pcontx->SegFs, pcontx->SegGs, pcontx->SegSs ) ;
#endif
	::OutputDebugString( bufDump ) ;
	//
#if	defined(__DEBUG__)
	return	EXCEPTION_EXECUTE_HANDLER ;
#else
	return	EXCEPTION_EXECUTE_HANDLER ;
#endif
}

// WM_TIMER
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMTimer( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	AddFreezePaint() ;
	//
	if ( m_pTimerHandler != nullptr )
	{
		STimeCounter	timer ;
		//
		m_pTimerHandler->OnTimer( this, 1 ) ;
		//
		double	msecOnTimer = timer.GetRealTime() ;
		m_msecSumOnTimer += msecOnTimer ;
		m_msecMaxOnTimer = esl_fmax( msecOnTimer, m_msecMaxOnTimer ) ;
		m_nCountOnTimer ++ ;
	}
	if ( m_timerLastPeriod.GetTime() >= 1000 )
	{
		m_nLastFPS = m_nCountRenderedFrames ;
		m_nCountRenderedFrames = 0 ;
		m_timerLastPeriod.Reset() ;
		//
		if ( m_flagFPSonCaption && !m_strCaption.IsEmpty() )
		{
			SString			strCaption = FormatWindowCaption() ;
			SArray<char>	bufName ;
			const char *	pszName ;
			pszName = strCaption.EncodeDefaultTo(bufName) ;
			::SetWindowText( m_hWnd, pszName ) ;
		}
		//
		if ( (m_flagTracePerformance || m_pLogListener != nullptr) && (m_nLastFPS != 0) )
		{
			if ( m_flagTracePerformance )
			{
				Trace( "--------------------\nFrame Per Second %d\n", m_nLastFPS ) ;
			}
			SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
			if ( pwvp != nullptr )
			{
				S3DRenderDevice *	pDevice = pwvp->GetRenderDevice() ;
				if ( pDevice != nullptr )
				{
					if ( m_pLogListener != nullptr )
					{
						PerformanceLogInfo	logInfo ;
						pDevice->DebugTracePerformanceLog( &logInfo ) ;
						//
						logInfo.msecAvgOnTimer =
							(m_nCountOnTimer > 0)
								? m_msecSumOnTimer / m_nCountOnTimer : 0.0 ;
						logInfo.msecMaxOnTimer = m_msecMaxOnTimer ;
						//
						m_pLogListener->OnPerformanceLog( this, logInfo ) ;
					}
					else
					{
						pDevice->DebugTracePerformanceLog() ;
					}
				}
			}
			if ( m_flagTracePerformance )
			{
				Trace( "--------------------\n\n" ) ;
			}
		}
		m_msecSumOnTimer = 0.0 ;
		m_msecMaxOnTimer = 0.0 ;
		m_nCountOnTimer = 0 ;
	}
	ReleaseFreezePaint() ;
	//
	if ( !m_queOnRenderThread.IsEmpty() )
	{
		::PostMessage( m_hWnd, wmCallRenderProcedure, 0, 0 ) ;
	}
	Unlock() ;
	//
	m_msecLastTimerInterval = m_timerWMTimer.GetRealTime() ;
	m_timerWMTimer.Reset() ;
	m_dwLastTimer = ::timeGetTime() ;
}

// WM_KEYDOWN
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMKeyDown( int64_t nVirtKey, int64_t nFlags )
{
	bool	fProcessed = false ;
	#if	defined(__DEBUG__)
	if ( (nVirtKey == vkeyFunction12) && (nFlags == vkeyContextShift) )
	{
		SSystem::Trace
			( "using memory: %d [bytes]\n",
					SSystem::eslHeapCommitCharge() ) ;
		SSystem::Trace
			( "max memory used: %d [bytes]\n",
					SSystem::eslHeapMaxCommitCharge() ) ;
		SGLImageBuffer::DumpAllBufferChain() ;
		fProcessed = true ;
	}
	#endif

	if ( (nVirtKey == vkeyFunction12) && (nFlags == 0) )
	{
		m_flagTracePerformance = !m_flagTracePerformance ;
		fProcessed = true ;
	}

	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pKeyHandler != nullptr )
	{
		fProcessed |= m_pKeyHandler->OnKeyDown( this, nVirtKey, nFlags ) ;
	}
	Unlock() ;
	return	fProcessed ;
}

// WM_KEYUP
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMKeyUp( int64_t nVirtKey, int64_t nFlags )
{
	bool	fProcessed = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pKeyHandler != nullptr )
	{
		fProcessed = m_pKeyHandler->OnKeyUp( this, nVirtKey, nFlags ) ;
	}
	Unlock() ;
	return	fProcessed ;
}

// WM_SETFOCUS
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMSetFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pKeyHandler != nullptr )
	{
		m_pKeyHandler->OnSetFocus( this ) ;
	}
	Unlock() ;
}

// WM_KILLFOCUS
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMKillFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pKeyHandler != nullptr )
	{
		m_pKeyHandler->OnKillFocus( this ) ;
	}
	Unlock() ;
}

// WM_CHAR
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMChar( uint16_t codeChar )
{
	bool	fProcessed = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pCharInputHandler != nullptr )
	{
		fProcessed = m_pCharInputHandler->OnChar( this, codeChar ) ;
	}
	Unlock() ;
	return	fProcessed ;
}

// WM_IME_STARTCOMPOSITION
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMImeStartComposition( WPARAM wParam, LPARAM lParam )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLInputStartComposition	iscForm ;
	::eslFillMemory
		( &iscForm, 0, sizeof(SGLInputStartComposition) ) ;
	if ( (m_pCharInputHandler != nullptr)
		&& m_pCharInputHandler->OnStartComposition( this, iscForm ) )
	{
		HIMC	hIMC = ::ImmGetContext( m_hWnd ) ;
		if ( iscForm.nFlags
			& (SGLInputStartComposition::flagPosition
					| SGLInputStartComposition::flagRectangle) )
		{
			COMPOSITIONFORM	cf ;
			::eslFillMemory( &cf, 0, sizeof(cf) ) ;
			if ( iscForm.nFlags
				& SGLInputStartComposition::flagPosition )
			{
				SGLPoint	ptPos
					( iscForm.ptStart.x, iscForm.ptStart.y ) ;
				VirtualPointToClient( ptPos ) ;
				//
				cf.dwStyle |= CFS_POINT ;
				cf.ptCurrentPos.x = ptPos.x ;
				cf.ptCurrentPos.y = ptPos.y ;
			}
			if ( iscForm.nFlags
				& SGLInputStartComposition::flagRectangle )
			{
				SGLPoint	ptPos0
					( iscForm.rctArea.x, iscForm.rctArea.y ) ;
				SGLPoint	ptPos1
					( iscForm.rctArea.x + iscForm.rctArea.w,
						iscForm.rctArea.y + iscForm.rctArea.h ) ;
				VirtualPointToClient( ptPos0 ) ;
				VirtualPointToClient( ptPos1 ) ;
				//
				cf.dwStyle |= CFS_RECT ;
				cf.rcArea.left = ptPos0.x ;
				cf.rcArea.top = ptPos0.y ;
				cf.rcArea.right = ptPos1.x ;
				cf.rcArea.bottom = ptPos1.y ;
			}
			::ImmSetCompositionWindow( hIMC, &cf ) ;
		}
		if ( iscForm.nFlags
				& SGLInputStartComposition::flagFont )
		{
			LOGFONT			lf ;
			SGLFontStyle	fs = iscForm.fsFontStyle ;
			SGLSize			szFont( fs.nSize, fs.nSize ) ;
			VirtualSizeToClient( szFont ) ;
			fs.nSize = szFont.h ;
			fs.ToLogFont( lf ) ;
			::ImmSetCompositionFont( hIMC, &lf ) ;
		}
		::ImmReleaseContext( m_hWnd, hIMC ) ;
	}
	Unlock() ;
	return	false ;
}

// WM_IME_ENDCOMPOSITION
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMImeEndComposition( WPARAM wParam, LPARAM lParam )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pCharInputHandler != nullptr )
	{
		m_pCharInputHandler->OnEndComposition( this ) ;
	}
	Unlock() ;
	return	false ;
}

// WM_IME_COMPOSITION
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnWMImeComposition( WPARAM wParam, LPARAM lParam )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pCharInputHandler != nullptr )
	{
		DWORD	dwIndex = GCS_COMPSTR ;
		if ( lParam & GCS_RESULTSTR )
		{
			dwIndex = GCS_RESULTSTR ;
		}
		HIMC	hIMC = ::ImmGetContext( m_hWnd ) ;
		LONG	nLen = ::ImmGetCompositionStringW( hIMC, dwIndex, nullptr, 0 ) ;
		nLen /= sizeof(wchar_t) ;
		//
		SString	strIME ;
		::ImmGetCompositionStringW
			( hIMC, dwIndex,
				strIME.LockBuffer( nLen ),
				nLen * sizeof(wchar_t) + 1 ) ;
		strIME.UnlockBuffer( nLen ) ;
		::ImmReleaseContext( m_hWnd, hIMC ) ;
		//
		SGLInputCompositionString	icsComp ;
		::eslFillMemory
			( &icsComp, 0, sizeof(SGLInputCompositionString) ) ;
		if ( lParam & GCS_RESULTSTR )
		{
			icsComp.nFlags =
				SGLInputCompositionString::flagResult ;
		}
		icsComp.pszComposition = strIME ;
		icsComp.nStart = 0 ;
		icsComp.nCount = (uint32_t) strIME.GetLength() ;
		//
		if ( m_pCharInputHandler->OnCompositionString( this, icsComp ) )
		{
			if ( lParam & GCS_RESULTSTR )
			{
				Unlock() ;
				return	true ;
			}
		}
	}
	Unlock() ;
	return	false ;
}

// WM_SETCURSOR
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMSetCursorClient( void )
{
	if ( m_flagShowCursor )
	{
		::SetCursor( m_hCursor ) ;
	}
	else
	{
		::SetCursor( nullptr ) ;
	}
}

// WM_CLOSE
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnWMClose( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pCommandHandler != nullptr )
	{
		SString	strCmd = SysCommandId::AppExit ;
		m_pCommandHandler->OnCommand( this, strCmd.GetConstArray(), 0, 0 ) ;
	}
	Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// GDI 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowGDIDisplayMethod, SGLWindowDisplayMethod )

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::OnMovedWindow( SGLAbstractWindow * pWnd )
{
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowGDIDisplayMethod::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowGDIDisplayMethod::FlipView
	( SGLAbstractWindow * pWnd,
		SGLImageObject * pImageRight, SGLImageObject * pImageLeft )
{
	SGLImageWin32DIBitmap *	pDIB =
		SGLImageWin32DIBitmap::CommitDIB( pImageRight ) ;
	if ( pDIB != nullptr )
	{
		SGLSize	sizeImage = pImageRight->GetImageSize() ;
		HWND	hWnd = pWnd->GetWindowHandle() ;
		HDC		hDC = ::GetDC( hWnd ) ;
		::BitBlt
			( hDC, 0, 0,
				sizeImage.w, sizeImage.h,
				pDIB->m_hDC, 0, 0, SRCCOPY ) ;
		::ReleaseDC( hWnd, hDC ) ;
	}
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowGDIDisplayMethod::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::MonoView ) == 0 )
	{
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowGDIDisplayMethod::IsStereoDisplayMode( void )
{
	return	false ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowGDIDisplayMethod::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::MonoView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// アナグリフGDI表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLAnaglyphGDIDisplayMethod, SGLWindowGDIDisplayMethod )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAnaglyphGDIDisplayMethod::SGLAnaglyphGDIDisplayMethod( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAnaglyphGDIDisplayMethod::~SGLAnaglyphGDIDisplayMethod( void )
{
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLAnaglyphGDIDisplayMethod::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView = m_imgView.GetImageSize() ;
	if ( ((uint32_t) sizeView.w != nWidth)
		|| ((uint32_t) sizeView.h != nHeight) )
	{
		m_imgView.CreateImage( nWidth, nHeight, formatImageARGB, 32 ) ;
	}
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLAnaglyphGDIDisplayMethod::FlipView
	( SGLAbstractWindow * pWnd,
		SGLImageObject * pImageRight, SGLImageObject * pImageLeft )
{
	if ( pImageLeft == nullptr )
	{
		SGLWindowGDIDisplayMethod::FlipView( pWnd, pImageRight, nullptr ) ;
		return ;
	}
	SGLImageBuffer	infView ;
	SGLImageBuffer	infRight ;
	SGLImageBuffer	infLeft ;
	infView.ptrBuffer =
		m_imgView.LockBuffer( infView, SGLImageObject::lockReadWrite ) ;
	infRight.ptrBuffer =
		pImageRight->LockBuffer( infRight, SGLImageObject::lockRead ) ;
	infLeft.ptrBuffer =
		pImageLeft->LockBuffer( infLeft, SGLImageObject::lockRead ) ;
	//
	if ( (infRight.width == infLeft.width)
		&& (infRight.height == infLeft.height)
		&& (infView.height == infRight.height)
		&& (infRight.depth == 32) && (infLeft.depth == 32) )
	{
		uint8_t *	pbytView = infView.ptrBuffer ;
		uint8_t *	pbytRight = infRight.ptrBuffer ;
		uint8_t *	pbytLeft = infLeft.ptrBuffer ;
		for ( uint32_t y = 0; y < infView.height; y ++ )
		{
			SGLPalette *	prgbView = (SGLPalette*) pbytView ;
			SGLPalette *	prgbRight = (SGLPalette*) pbytRight ;
			SGLPalette *	prgbLeft = (SGLPalette*) pbytLeft ;
			//
			if ( (infRight.format & formatImageTypeMask) == formatImageBGR )
			{
				for ( uint32_t x = 0; x < infView.width; x ++ )
				{
					prgbView->argb.Blue =
						(uint8_t) (((unsigned int) prgbRight->abgr.Blue
												+ prgbLeft->abgr.Blue) >> 1) ;
					prgbView->argb.Green = prgbRight->abgr.Green ;
					prgbView->argb.Red =
						(prgbLeft->abgr.Blue >> 2)
							+ (prgbLeft->abgr.Green >> 2)
							+ (prgbLeft->abgr.Red >> 1) ;
					//
					prgbView ++ ;
					prgbRight ++ ;
					prgbLeft ++ ;
				}
			}
			else
			{
				for ( uint32_t x = 0; x < infView.width; x ++ )
				{
					prgbView->argb.Blue =
						(uint8_t) (((unsigned int) prgbRight->argb.Blue
												+ prgbLeft->argb.Blue) >> 1) ;
					prgbView->argb.Green = prgbRight->argb.Green ;
					prgbView->argb.Red =
						(prgbLeft->argb.Blue >> 2)
							+ (prgbLeft->argb.Green >> 2)
							+ (prgbLeft->argb.Red >> 1) ;
					//
					prgbView ++ ;
					prgbRight ++ ;
					prgbLeft ++ ;
				}
			}
			pbytView += infView.pitchLine ;
			pbytRight += infRight.pitchLine ;
			pbytLeft += infLeft.pitchLine ;
		}
	}
	//
	m_imgView.UnlockBuffer( SGLImageObject::lockReadWrite ) ;
	pImageRight->UnlockBuffer( SGLImageObject::lockRead ) ;
	pImageLeft->UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	SGLWindowGDIDisplayMethod::FlipView( pWnd, &m_imgView, nullptr ) ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAnaglyphGDIDisplayMethod::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::AnaglyphView ) == 0 )
	{
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLAnaglyphGDIDisplayMethod::IsStereoDisplayMode( void )
{
	return	true ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLAnaglyphGDIDisplayMethod::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::AnaglyphView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// インターリーブGDI表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SGLInterleavedGDIDisplayMethod, SGLWindowGDIDisplayMethod )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLInterleavedGDIDisplayMethod::SGLInterleavedGDIDisplayMethod( void )
{
	m_nParam = 0 ;
	m_nPosSwap = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLInterleavedGDIDisplayMethod::~SGLInterleavedGDIDisplayMethod( void )
{
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLInterleavedGDIDisplayMethod::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView = m_imgView.GetImageSize() ;
	if ( ((uint32_t) sizeView.w != nWidth)
		|| ((uint32_t) sizeView.h != nHeight) )
	{
		m_imgView.CreateImage( nWidth, nHeight, formatImageARGB, 32 ) ;
	}
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLInterleavedGDIDisplayMethod::OnMovedWindow( SGLAbstractWindow * pWnd )
{
	HWND	hWnd = pWnd->GetWindowHandle() ;
	if ( hWnd != nullptr )
	{
		POINT	ptClient = { 0, 0 } ;
		::ClientToScreen( hWnd, &ptClient ) ;
		//
		uint32_t	nSwap = 0 ;
		if ( m_nParam & flagVertical )
		{
			if ( ptClient.x & 0x01 )
			{
				nSwap = flagSwapEyes ;
			}
		}
		else
		{
			if ( ptClient.y & 0x01 )
			{
				nSwap = flagSwapEyes ;
			}
		}
		if ( nSwap != m_nPosSwap )
		{
			m_nPosSwap = nSwap ;
			pWnd->PostUpdate() ;
		}
	}
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLInterleavedGDIDisplayMethod::FlipView
	( SGLAbstractWindow * pWnd,
		SGLImageObject * pImageRight, SGLImageObject * pImageLeft )
{
	if ( pImageLeft == nullptr )
	{
		SGLWindowGDIDisplayMethod::FlipView( pWnd, pImageRight, nullptr ) ;
		return ;
	}
	SGLImageBuffer	infView ;
	SGLImageBuffer	infRight ;
	SGLImageBuffer	infLeft ;
	infView.ptrBuffer =
		m_imgView.LockBuffer( infView, SGLImageObject::lockReadWrite ) ;
	infRight.ptrBuffer =
		pImageRight->LockBuffer( infRight, SGLImageObject::lockRead ) ;
	infLeft.ptrBuffer =
		pImageLeft->LockBuffer( infLeft, SGLImageObject::lockRead ) ;
	//
	if ( (infRight.width == infLeft.width)
		&& (infRight.height == infLeft.height)
		&& (infView.height == infRight.height)
		&& (infRight.depth == 32) && (infLeft.depth == 32) )
	{
		if ( !(m_nParam & flagVertical) )
		{
			if ( !((m_nParam ^ m_nPosSwap) & flagSwapEyes) )
			{
				InterleaveHorizontal( infView, infRight, infLeft ) ;
			}
			else
			{
				InterleaveHorizontal( infView, infLeft, infRight ) ;
			}
		}
		else
		{
			if ( !((m_nParam ^ m_nPosSwap) & flagSwapEyes) )
			{
				InterleaveVertical( infView, infRight, infLeft ) ;
			}
			else
			{
				InterleaveVertical( infView, infLeft, infRight ) ;
			}
		}
	}
	//
	m_imgView.UnlockBuffer( SGLImageObject::lockReadWrite ) ;
	pImageRight->UnlockBuffer( SGLImageObject::lockRead ) ;
	pImageLeft->UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	SGLWindowGDIDisplayMethod::FlipView( pWnd, &m_imgView, nullptr ) ;
}

void SGLInterleavedGDIDisplayMethod::InterleaveHorizontal
	( const SGLImageBuffer& imgView,
		const SGLImageBuffer& imgRight, const SGLImageBuffer& imgLeft )
{
	SGLImageInfo	infView = imgView ;
	SGLImageInfo	infRight = imgRight ;
	SGLImageInfo	infLeft = imgLeft ;
	uint8_t *		pbytView = imgView.ptrBuffer ;
	uint8_t *		pbytRight = imgRight.ptrBuffer ;
	uint8_t *		pbytLeft = imgLeft.ptrBuffer ;
	//
	for ( uint32_t y = 0; y < infView.height; y ++ )
	{
		SGLPalette *	prgbView = (SGLPalette*) pbytView ;
		SGLPalette *	prgbRight = (SGLPalette*) pbytRight ;
		if ( (infRight.format & formatImageTypeMask) == formatImageBGR )
		{
			for ( uint32_t x = 0; x < infView.width; x ++ )
			{
				uint32_t	argb = prgbRight[x].ui32 ;
				prgbView[x].ui32 = (argb & 0xFF00FF00)
									| ((argb >> 16) & 0xFF)
									| ((argb << 16) & 0xFF0000) ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < infView.width; x ++ )
			{
				prgbView[x].ui32 = prgbRight[x].ui32 ;
			}
		}
		uint8_t *	pbytTemp = pbytRight ;
		pbytView += infView.pitchLine ;
		pbytRight = pbytLeft + infLeft.pitchLine ;
		pbytLeft = pbytTemp + infRight.pitchLine ;
	}
}

void SGLInterleavedGDIDisplayMethod::InterleaveVertical
	( const SGLImageBuffer& imgView,
		const SGLImageBuffer& imgRight, const SGLImageBuffer& imgLeft )
{
	SGLImageInfo	infView = imgView ;
	SGLImageInfo	infRight = imgRight ;
	SGLImageInfo	infLeft = imgLeft ;
	uint8_t *		pbytView = imgView.ptrBuffer ;
	uint8_t *		pbytRight = imgRight.ptrBuffer ;
	uint8_t *		pbytLeft = imgLeft.ptrBuffer ;
	//
	for ( uint32_t y = 0; y < infView.height; y ++ )
	{
		SGLPalette *	prgbView = (SGLPalette*) pbytView ;
		SGLPalette *	prgbRight = (SGLPalette*) pbytRight ;
		SGLPalette *	prgbLeft = (SGLPalette*) pbytLeft ;
		if ( (infRight.format & formatImageTypeMask) == formatImageBGR )
		{
			for ( uint32_t x = 0; x + 1 < infView.width; x += 2 )
			{
				uint32_t	argb0 = prgbRight[x].ui32 ;
				uint32_t	argb1 = prgbRight[x + 1].ui32 ;
				prgbView[x].ui32 = (argb0 & 0xFF00FF00)
									| ((argb0 >> 16) & 0xFF)
									| ((argb0 << 16) & 0xFF0000) ;
				prgbView[x + 1].ui32 = (argb1 & 0xFF00FF00)
									| ((argb1 >> 16) & 0xFF)
									| ((argb1 << 16) & 0xFF0000) ;
			}
			if ( infView.width & 0x01 )
			{
				uint32_t	x = infView.width - 1 ;
				uint32_t	argb = prgbRight[x].ui32 ;
				prgbView[x].ui32 = (argb & 0xFF00FF00)
									| ((argb >> 16) & 0xFF)
									| ((argb << 16) & 0xFF0000) ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x + 1 < infView.width; x += 2 )
			{
				prgbView[x].ui32 = prgbRight[x].ui32 ;
				prgbView[x + 1].ui32 = prgbLeft[x + 1].ui32 ;
			}
			if ( infView.width & 0x01 )
			{
				uint32_t	x = infView.width - 1 ;
				prgbView[x].ui32 = prgbRight[x].ui32 ;
			}
		}
		uint8_t *	pbytTemp = pbytRight ;
		pbytView += infView.pitchLine ;
		pbytRight += infRight.pitchLine ;
		pbytLeft += infLeft.pitchLine ;
	}
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLInterleavedGDIDisplayMethod::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::InterleavedView ) == 0 )
	{
		if ( m_nParam != nParam )
		{
			m_nParam = (uint32_t) nParam ;
			OnMovedWindow( pWnd ) ;
			//
			HWND	hWnd = pWnd->GetWindowHandle() ;
			if ( hWnd != nullptr )
			{
				InvalidateRect( hWnd, nullptr, FALSE ) ;
			}
		}
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLInterleavedGDIDisplayMethod::IsStereoDisplayMode( void )
{
	return	true ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLInterleavedGDIDisplayMethod::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::InterleavedView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// NVIDIA 3D Vision 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

HMODULE	SGLNvidia3DVisionDisplayMethod::m_hModuleD3D9 = nullptr ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SGLNvidia3DVisionDisplayMethod, SGLWindowDisplayMethod )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLNvidia3DVisionDisplayMethod::SGLNvidia3DVisionDisplayMethod( void )
{
	if ( m_hModuleD3D9 == nullptr )
	{
		m_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
	}
	m_pWnd = nullptr ;
	m_hWnd = nullptr ;
	m_id3d9 = nullptr ;
	m_id3d9Dev = nullptr ;
	m_pd3dpp = nullptr ;
	m_idds9View = nullptr ;
	m_nViewParam = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLNvidia3DVisionDisplayMethod::~SGLNvidia3DVisionDisplayMethod( void )
{
	Release() ;
}

// Direct3D9 生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::CreateDirect3D9Device
	( HWND hWnd, int nWidth, int nHeight, int nAdapter, bool fWindowed )
{
	if ( m_hModuleD3D9 == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( m_id3d9 != nullptr )
	{
		Release() ;
	}
	//
	// Dreict3D9 生成
	//
	typedef	IDirect3D9 * (WINAPI *API_Direct3DCreate9)( UINT SDKVersion ) ;
	API_Direct3DCreate9	apiDirect3DCreate9 =
		(API_Direct3DCreate9)
			::GetProcAddress( m_hModuleD3D9, "Direct3DCreate9" ) ;
	if ( apiDirect3DCreate9 == nullptr )
	{
		ESLTrace( "Not found Direct3DCreate.\n" ) ;
		return	sglErrFailed ;
	}
	m_id3d9 = apiDirect3DCreate9( D3D_SDK_VERSION ) ;
	if ( m_id3d9 == nullptr )
	{
		ESLTrace( "Failed to create Direct3D9.\n" ) ;
		return	sglErrFailed ;
	}
	//
	// Direct3DDevice9 生成
	//
	if ( m_pd3dpp == nullptr )
	{
		m_pd3dpp = new D3DPRESENT_PARAMETERS ;
	}
	memset( m_pd3dpp, 0, sizeof(D3DPRESENT_PARAMETERS) ) ;
	m_hWnd = hWnd ;
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
			( nAdapter, D3DDEVTYPE_HAL, hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
								m_pd3dpp, &m_id3d9Dev ) != D3D_OK )
	{
		ESLTrace( "Failed to create Direct3DDevice9.\n" ) ;
		//
		Release() ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// Direct3D9 リセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::ResetDirect3D9Device( void )
{
	if ( m_id3d9 && m_id3d9Dev )
	{
		ReleaseSurface() ;
		//
		if ( m_pd3dpp->Windowed )
		{
			RECT	rect ;
			::GetClientRect( m_hWnd, &rect ) ;
			m_pd3dpp->BackBufferWidth = rect.right - rect.left ;
			m_pd3dpp->BackBufferHeight = rect.bottom - rect.top ;
		}
		HRESULT	hr = m_id3d9Dev->Reset( m_pd3dpp ) ;
		if ( hr != D3D_OK )
		{
			ESLTrace( "failed ot IDirect3DDevice9::Reset %d x %d (%08X).",
					m_pd3dpp->BackBufferWidth,
					m_pd3dpp->BackBufferHeight, hr ) ;
			return	sglErrFailed ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// Direct3D9 オブジェクト解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::Release( void )
{
	ReleaseSurface() ;
	//
	if ( m_id3d9Dev != nullptr )
	{
		m_id3d9Dev->Release() ;
		m_id3d9Dev = nullptr ;
	}
	if ( m_id3d9 != nullptr )
	{
		m_id3d9->Release() ;
		m_id3d9 = nullptr ;
	}
	if ( m_pd3dpp != nullptr )
	{
		delete	m_pd3dpp ;
		m_pd3dpp = nullptr ;
	}
	return	sglErrSuccess ;
}

// 表示用サーフェス生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::CreateViewSurface( int nWidth, int nHeight )
{
	if ( m_id3d9Dev == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( m_idds9View != nullptr )
	{
		m_idds9View->Release() ;
		m_idds9View = nullptr ;
	}
	m_id3d9Dev->CreateOffscreenPlainSurface
		( nWidth * 2, nHeight + 1, D3DFMT_A8R8G8B8,
					D3DPOOL_DEFAULT, &m_idds9View, nullptr ) ;
	if ( m_idds9View == nullptr )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// サーフェス削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::ReleaseSurface( void )
{
	if ( m_idds9View != nullptr )
	{
		m_idds9View->Release() ;
		m_idds9View = nullptr ;
	}
	return	sglErrSuccess ;
}

// 表示サーフェースにデータ転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::PrepareViewSurface
	( SGLImageObject * pImageRight, SGLImageObject * pImageLeft )
{
	if ( m_idds9View == nullptr )
	{
		if ( m_sizeView.IsEmpty()
			|| CreateViewSurface( m_sizeView.w, m_sizeView.h )
			|| (m_idds9View == nullptr) )
		{
			return	sglErrFailed ;
		}
	}
	//
	// サーフェスメモリロック
	//
	D3DSURFACE_DESC	d3dsd ;
	if ( m_idds9View->GetDesc( &d3dsd ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DSurface9::GetDesc.\n" ) ;
		return	sglErrFailed ;
	}
	if ( d3dsd.Format != D3DFMT_A8R8G8B8 )
	{
		ESLTrace( "Missmatch IDirect3DSurface9 format.\n" ) ;
		return	sglErrFailed ;
	}
	D3DLOCKED_RECT	lr ;
	if ( m_idds9View->LockRect( &lr, nullptr, 0 ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DSurface9::LockRect.\n" ) ;
		return	sglErrFailed ;
	}
	SGLImageBuffer	imgView ;
	imgView.format = formatImageARGB ;
	imgView.depth = 32 ;
	imgView.width = d3dsd.Width ;
	imgView.height = d3dsd.Height ;
	imgView.pitchPixel = 4 ;
	imgView.pitchLine = lr.Pitch ;
	imgView.ptrBuffer = (uint8_t*) lr.pBits ;
	//
	// 右視点画像描画
	//
	if ( pImageRight != nullptr )
	{
		SGLImageBuffer	imgRight ;
		imgRight.ptrBuffer =
			pImageRight->LockBuffer( imgRight, SGLImageObject::lockRead ) ;
		//
		if ( (imgRight.format == imgView.format)
			&& (imgRight.depth == imgView.depth) )
		{
			sglCopyImageBuffer( imgView, imgRight, m_sizeView.w, 0 ) ;
		}
		else
		{
			sglConvertImageBuffer( imgView, imgRight, m_sizeView.w, 0 ) ;
		}
		//
		pImageRight->UnlockBuffer( SGLImageObject::lockRead ) ;
	}
	//
	// 左視点画像描画
	//
	if ( pImageLeft != nullptr )
	{
		SGLImageBuffer	imgLeft ;
		imgLeft.ptrBuffer =
			pImageLeft->LockBuffer( imgLeft, SGLImageObject::lockRead ) ;
		//
		if ( (imgLeft.format == imgView.format)
			&& (imgLeft.depth == imgView.depth) )
		{
			sglCopyImageBuffer( imgView, imgLeft, 0, 0 ) ;
		}
		else
		{
			sglConvertImageBuffer( imgView, imgLeft, 0, 0 ) ;
		}
		//
		pImageLeft->UnlockBuffer( SGLImageObject::lockRead ) ;
	}
	//
	// NVIDIA シグネチャ設定
	//
	Nv_Stereo_Image_Header *	pnvsih =
		(Nv_Stereo_Image_Header*)
			(imgView.ptrBuffer
				+ (imgView.pitchLine * (imgView.height - 1))) ;
	//
	pnvsih->dwSignature = NVSTEREO_IMAGE_SIGNATURE ;
	pnvsih->dwWidth = imgView.width ;
	pnvsih->dwHeight = imgView.height - 1 ;
	pnvsih->dwBPP = imgView.depth ;
	pnvsih->dwFlags =
		(m_nViewParam & Window::stereoFlagSwapEyes) ? SIH_SWAP_EYES : 0 ;
	//
	// アンロック
	//
	m_idds9View->UnlockRect() ;
	//
	return	sglErrSuccess ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::NVStereoBLT( void )
{
	if ( m_id3d9Dev == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// バックサーフェスへ描画
	//
	IDirect3DSurface9 *	id3dsBack = nullptr ;
	if ( m_id3d9Dev->GetBackBuffer
		( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::GetBackBuffer.\n" ) ;
		return	sglErrFailed ;
	}
	m_id3d9Dev->Clear
		( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 1.0, 0 ) ;
	//
	if ( m_id3d9Dev->BeginScene() != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::BeginScene.\n" ) ;
	}
	//
	RECT	rcSrc, rcDst ;
	rcSrc.left = 0 ;
	rcSrc.top = 0 ;
	rcSrc.right = m_sizeView.w * 2 ;
	rcSrc.bottom = m_sizeView.h + 1 ;
	rcDst.left = 0 ;
	rcDst.top = 0 ;
	rcDst.right = m_sizeView.w ;
	rcDst.bottom = m_sizeView.h ;
	//
	SGLError	err = sglErrSuccess ;
	if ( m_id3d9Dev->StretchRect
		( m_idds9View, &rcSrc,
			id3dsBack, &rcDst, D3DTEXF_NONE ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::StretchRect.\n" ) ;
		err = sglErrFailed ;
	}
	if ( m_id3d9Dev->EndScene() != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::EndScene.\n" ) ;
	}
	HRESULT	hr = m_id3d9Dev->Present( nullptr, nullptr, nullptr, nullptr ) ;
	//
	id3dsBack->Release() ;
	//
	if ( hr != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::Present.(%08X)\n", hr ) ;
		err = sglErrFailed ;
		//
		if ( (hr == D3DERR_DEVICEREMOVED) || (hr == D3DERR_DEVICELOST) )
		{
			//
			// デバイスロストの復帰
			//
			ResetDirect3D9Device() ;
		}
	}
	return	err ;
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	m_sizeView.w = (int32_t) nWidth ;
	m_sizeView.h = (int32_t) nHeight ;
	//
	ResetDirect3D9Device() ;
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	m_pWnd = pWnd ;
	//
	WINDOWPLACEMENT	wp ;
	HWND	hWnd = pWnd->GetWindowHandle() ;
	if ( !::GetWindowPlacement( hWnd, &wp ) )
	{
		::GetWindowRect( hWnd, &wp.rcNormalPosition ) ;
	}
	SGLDisplayMode::MonitorHandle	hMonitor = nullptr ;
	SString			strDisplayName ;
	SGLImageRect	rectWndPos
		( wp.rcNormalPosition.left, wp.rcNormalPosition.top,
			wp.rcNormalPosition.right - wp.rcNormalPosition.left,
			wp.rcNormalPosition.bottom - wp.rcNormalPosition.top );
	const wchar_t *	pwszDisplayName =
			m_displayMode.GetDisplayNameFromRect
					( strDisplayName, &rectWndPos, &hMonitor ) ;
	//
	SGLImageRect	rectMonitor, rectVirtWork ;
	size_t	nAdapter = 0 ;
	GUID	guidMonitor ;
	if ( m_displayMode.GetMonitorRect
		( pwszDisplayName, rectMonitor, rectVirtWork, &hMonitor ) )
	{
		rectMonitor.x = 0 ;
		rectMonitor.y = 0 ;
		rectMonitor.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
		rectMonitor.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
	}
	else
	{
		m_displayMode.GetDDMonitorGUID( &guidMonitor, hMonitor, &nAdapter ) ;
	}
	if ( CreateDirect3D9Device
		( hWnd, rectMonitor.w, rectMonitor.h, (int) nAdapter, false ) )
	{
		ESLTrace( "failed to CreateDirect3D9Device.\n" ) ;
		return ;
	}
	PrepareViewSurface( nullptr, nullptr ) ;
	NVStereoBLT() ;
	NVStereoBLT() ;
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
	Release() ;
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::OnMovedWindow( SGLAbstractWindow * pWnd )
{
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLNvidia3DVisionDisplayMethod::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLNvidia3DVisionDisplayMethod::FlipView
	( SGLAbstractWindow * pWnd,
		SGLImageObject * pImageRight,
		SGLImageObject * pImageLeft )
{
	PrepareViewSurface( pImageRight, pImageLeft ) ;
	if ( NVStereoBLT() )
	{
		PrepareViewSurface( pImageRight, pImageLeft ) ;
		NVStereoBLT() ;
	}
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLNvidia3DVisionDisplayMethod::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::NVStereoBLT ) == 0 )
	{
		m_nViewParam = (uint32_t) nParam ;
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLNvidia3DVisionDisplayMethod::IsStereoDisplayMode( void )
{
	return	true ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLNvidia3DVisionDisplayMethod::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
			( pszMethodID, Window::Stereo3D::NVStereoBLT ) == 0 )
	{
		return	true ;
	}
	return	false ;
}



