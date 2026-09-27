
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/window/sgl_window_menu.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 同期プロシージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLGenericWindow::SmartLockProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::SmartLockProcedure::SmartLockProcedure
	( SSystem::SProcedure * pProc, SSystem::SMutex * pMutex )
{
	m_pProc = pProc ;
	m_pMutexUI = pMutex ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SmartLockProcedure::Run( void )
{
	m_pProc->Run() ;
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SmartLockProcedure::Prepare( void )
{
	m_pMutexUI->Lock() ;
	m_pProc->Prepare() ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SmartLockProcedure::Finalize( void )
{
	m_pProc->Finalize() ;
	m_pMutexUI->Unlock() ;
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// 汎用ウィンドウ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLGenericWindow, SGLAbstractWindow, JavaObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindow::SGLGenericWindow
	( SGLWindowViewProducer * pwvp,
		SSystem::SEnvironmentInterface * env )
	: m_wvfFramework( this, pwvp ), m_vJoystickPos( 0, 0, 0, 0 )
{
	m_pEnv = env ;
	if ( env == NULL )
	{
		m_pEnv = SEnvironmentInterface::GetInstance() ;
	}
	//
	m_pViewSync = NULL ;
	m_nRequestFPS = 0 ;
	//
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	m_flagCreated = false ;
	m_flagModeDisplay = false ;
	m_flagFullscreen = false ;
	m_flagLayeredWindow = false ;
	m_modeCooperation = Window::modeWindow ;
	m_flagsOption = 0 ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	m_flagsLayout = 0 ;
	//
	m_countTouching = 0 ;
	m_idPrimaryTouch = 0 ;
	m_flagPrimaryTouchDown = false ;
	//
	m_flagJoyStick = false ;
	m_maskJoyButtonPushed = 0 ;
	//
	m_signalDonePaint.Initialize( false ) ;
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
	SGLWindowViewSynchronizer *	pLastSync = NULL ;
	m_csViewSync.Lock() ;
	pLastSync = m_pViewSync ;
	m_pViewSync = pViewSync ;
	m_csViewSync.Unlock() ;
	return	pLastSync ;
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
	//
	// 仮想ウィンドウ生成
	//
	m_flagModeDisplay = true ;
	m_flagFullscreen = true ;
	m_flagLayeredWindow = false ;
	m_nBitsPerPixel = nBitsPerPixel ;
	m_nFrequency = nFrequency ;
	//
	if ( CreateWindowSimply( pszWindowName, nWidth, nHeight ) )
	{
		m_flagCreated = false ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CloseDisplay( void )
{
	return	CloseWindowSimply() ;
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
	m_flagsOption = nFlags ;
	m_flagsOption &= ~(flagDoMinimize | flagDoMaximize) ;
	//
	if ( m_flagsOption & flagOpenIME )
	{
		JNI::JSmartClass	jclsEntisGLS
			( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
		jmethodID	jmidShowSoftKeyboard =
			jclsEntisGLS.GetStaticMethodID( "showSoftKeyboard", "()V" ) ;
		jclsEntisGLS.CallStaticVoidMethod( jmidShowSoftKeyboard ) ;
		//
		m_flagsOption &= ~flagOpenIME ;
	}
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ChangeCooperationLevel( Window::CooperationMode mode )
{
	m_modeCooperation = mode ;
	return	sglErrSuccess ;
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
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		m_pMutexUI->Lock() ;
		m_sizeVirtual.w = nWidth ;
		m_sizeVirtual.h = nHeight ;
		m_sizeLayoutOriginal = m_sizeVirtual ;
		m_nBitsPerPixel = nBitsPerPixel ;
		m_nFrequency = nFrequency ;
		//
		UpdateWindowLayout() ;
		//
		m_pMutexUI->Unlock() ;
		//
		PostUpdate() ;
	}
	return	sglErrSuccess ;
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
	return	sglErrSuccess ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::EnableZBuffer( bool flagZBuffer )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == NULL )
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
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == NULL )
	{
		return	sglErrFailed ;
	}
	return	pwvp->SetStereoDisplayMode( this, pszMethodID, nParam ) ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsSupportedStereoDisplayMode( const wchar_t * pszMethodID )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp == NULL )
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
	m_rectWindow.x = xPos ;
	m_rectWindow.y = yPos ;
	if ( pInitExSize != NULL )
	{
		m_rectWindow.w = pInitExSize->w ;
		m_rectWindow.h = pInitExSize->h ;
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
	ptWindow.x = m_rectWindow.x ;
	ptWindow.y = m_rectWindow.y ;
	if ( pWindowSize != NULL )
	{
		pWindowSize->w = m_rectWindow.w ;
		pWindowSize->h = m_rectWindow.h ;
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
	if ( pwvp == NULL )
	{
		return	sglErrFailed ;
	}
	pwvp->GetInternalViewPosition( rctDisplay ) ;
	//
	rctRender.x = 0 ;
	rctRender.y = 0 ;
	rctRender.w = m_sizePhysicalDisplay.w ;
	rctRender.h = m_sizePhysicalDisplay.h ;
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	m_pMutexUI->Lock() ;
	m_wvfFramework.SetExteriorBackgroundFrame
		( nFlags, rgbColor, pTile, pLeft, pRight, pUpper, pUnder ) ;
	//
	if ( m_flagCreated )
	{
		PostUpdate() ;
	}
	m_pMutexUI->Unlock() ;
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
	//
	// 仮想ウィンドウ生成
	//
	m_flagModeDisplay = false ;
	m_flagFullscreen = false ;
	m_flagLayeredWindow = ((nFlags & flagLayeredWindow) != 0) ;
	m_nBitsPerPixel = 0 ;
	m_nFrequency = 0 ;
	//
	if ( CreateWindowSimply( pszWindowName, nWidth, nHeight ) )
	{
		m_flagCreated = false ;
		return	sglErrFailed ;
	}
	//
	// 親ウィンドウ関連付け
	//
	SGLGenericWindow *
		pGenParentWnd = ESLTypeCast<SGLGenericWindow>( pParentWnd ) ;
	if ( pGenParentWnd != NULL )
	{
		SSystem::QuickLock() ;
		m_refParentWnd = pParentWnd ;
		pGenParentWnd->m_arrChildren.TrimEmpty() ;
		pGenParentWnd->m_arrChildren.Add( this ) ;
		SSystem::QuickUnlock() ;
	}
	return	sglErrSuccess ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CloseWindow( void )
{
	return	CloseWindowSimply() ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetWindowLayout
	( uint32_t nFlags, int xPos, int yPos )
{
	m_pMutexUI->Lock() ;
	m_flagsLayout = nFlags ;
	m_ptLayoutOffset.x = xPos ;
	m_ptLayoutOffset.y = yPos ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	m_pMutexUI->Unlock() ;
	//
	if ( m_flagCreated )
	{
		UpdateWindowLayout() ;
		PostUpdate() ;
	}
	return	sglErrSuccess ;
}

// クライアント座標→スクリーン座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLGenericWindow::ScreenPositionFromClient( S2DDVector& vClient )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != NULL) && m_flagCreated )
	{
		pwvp->VirtualToPhysicalPosition( vClient ) ;
		//
		vClient.x += m_rectWindow.x ;
		vClient.y += m_rectWindow.y ;
	}
	return	vClient ;
}

// スクリーン座標→クライアント座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLGenericWindow::ClientPositionFromScreen( S2DDVector& vScreen )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != NULL) && m_flagCreated )
	{
		S2DDVector	vClient( vScreen.x - m_rectWindow.x,
								vScreen.y - m_rectWindow.y ) ;
		pwvp->PhysicalToVirtualPosition( vClient ) ;
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
	java_EntisGLS_postUpdateView() ;
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
	if ( pUpdate == NULL )
	{
		m_signalDonePaint.ResetSignal() ;
		PostUpdate( NULL ) ;
		return	(SGLError) m_signalDonePaint.Wait( 100 ) ;
	}
	//
	// フレーム速度同期
	//
	int64_t	msecStart = CurrentMilliSec() ;
	m_signalDonePaint.ResetSignal() ;
	PostUpdate( NULL ) ;
	m_signalDonePaint.Wait( 100 ) ;
	int64_t	msecEnd = CurrentMilliSec() ;
	//
	pUpdate->WaitFrame( msecEnd - msecStart, 0 ) ;
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
	return	sglErrSuccess ;
}

// 描画スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::PostRenderingThread
			( SSystem::SProcedure * pProc, PostThreadType postType )
{
	if ( !m_flagCreated || (pProc == NULL) )
	{
		return	sglErrFailed ;
	}
	if ( postType == postAsyncNoRender )
	{
		JNI::JavaObject	jobjBuf ;
		if ( jobjBuf.CreateByteBuffer( pProc, 1 ) == NULL )
		{
			return	sglErrFailed ;
		}
		if ( !java_EntisGLS_callNativeOnAsyncNoRenderingThread( jobjBuf.GetObject() ) )
		{
			return	sglErrFailed ;
		}
	}
	else if ( postType == postAsyncNoRenderFinally )
	{
		return	sglErrFailed ;
	}
	else
	{
		SmartLockProcedure *	pslpProc = new SmartLockProcedure( pProc, m_pMutexUI ) ;
		JNI::JavaObject	jobjBuf ;
		if ( jobjBuf.CreateByteBuffer
				( pslpProc, sizeof(SmartLockProcedure) ) == NULL )
		{
			delete	pslpProc ;
			return	sglErrFailed ;
		}
		if ( !java_EntisGLS_callNativeOnRenderingThread
				( jobjBuf.GetObject(), (postType != postNormal) ) )
		{
			delete	pslpProc ;
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::PostUIThread( SSystem::SProcedure * pProc )
{
	if ( !m_flagCreated || (pProc == NULL) )
	{
		return	sglErrFailed ;
	}
	SmartLockProcedure *	pslpProc = new SmartLockProcedure( pProc, m_pMutexUI ) ;
	JNI::JavaObject	jobjBuf ;
	if ( jobjBuf.CreateByteBuffer
			( pslpProc, sizeof(SmartLockProcedure) ) == NULL )
	{
		delete	pslpProc ;
		return	sglErrFailed ;
	}
	if ( !java_EntisGLS_callNativeOnUIThread( jobjBuf.GetObject() ) )
	{
		delete	pslpProc ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ウィンドウがアクティブ（最前面）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsWindowActive( void )
{
	JNI::JavaObject	jobjSurfaceView
						( java_EntisGLS_getMainSurfaceView(), true ) ;
	jmethodID	jmidIsViewActive =
		jobjSurfaceView.GetMethodID
			( "isViewActive", "(L" ENTIS_GLS4_JAVA_PACKAGE "/ViewInterface;)Z" ) ;
	if ( jmidIsViewActive == NULL )
	{
		return	false ;
	}
	return	jobjSurfaceView.CallBooleanMethod
					( jmidIsViewActive, JavaObject::GetObject() ) ;
}

// ウィンドウキャプション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetWindowCaption( const wchar_t * pszWindowName )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	m_pMutexUI->Lock() ;
	m_strCaption = pszWindowName ;
	m_pMutexUI->Unlock() ;
	return	sglErrSuccess ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::ShowCursor( bool fShow )
{
	return	sglErrSuccess ;
}

// マウスカーソル表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::IsShowCursor( void )
{
	return	false ;
}

// マウスカーソル変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::SetCursor( const wchar_t * pszCursorID )
{
	return	sglErrSuccess ;
}

// マウスカーソル座標移動
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::MoveCursorPosition
	( int32_t xPos, int32_t yPos, int idMouse )
{
	QuickLock() ;
	if ( idMouse == m_idPrimaryTouch )
	{
		S2DDVector	vMouse( xPos, yPos ) ;
		ScreenPositionFromClient( vMouse ) ;
		m_vPrimaryTouch = vMouse ;
	}
	QuickUnlock() ;
	return	sglErrSuccess ;
}

// マウスカーソル座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::GetCursorPosition
	( SGLPoint& ptCursor, int idMouse )
{
	SGLError	err = sglErrFailed ;
	QuickLock() ;
	if ( idMouse == m_idPrimaryTouch )
	{
		S2DDVector	vMouse = m_vPrimaryTouch ;
		ClientPositionFromScreen( vMouse ) ;
		//
		ptCursor.x = eslRoundR32ToInt( (float32_t) vMouse.x ) ;
		ptCursor.y = eslRoundR32ToInt( (float32_t) vMouse.y ) ;
		//
		if ( m_flagPrimaryTouchDown )
		{
			err = sglErrSuccess ;
		}
	}
	QuickUnlock() ;
	return	err ;
}

// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
//////////////////////////////////////////////////////////////////////////////
int SGLGenericWindow::GetMonitorFrequency( void )
{
	return	0 ;
}

// ウィンドウメニューの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::AttachMenu( SGLWindowMenu * pMenu )
{
	jobject	jobjMenu = NULL ;
	if ( pMenu != NULL )
	{
		jobjMenu = pMenu->GetObject() ;
	}
	if ( GetObject() == NULL )
	{
		JNIEnv *	env = JNI::GetJNIEnv() ;
		m_jsobjMenu.AttachObject( env->NewLocalRef(jobjMenu), env ) ;
		return	sglErrFailed ;
	}
	jmethodID	jmidSetMenu =
		JavaObject::GetMethodID
			( "setMenu", "(L" ENTIS_GLS4_JAVA_PACKAGE "/MenuData;)V" ) ;
	JavaObject::CallVoidMethod( jmidSetMenu, jobjMenu ) ;
	return	sglErrSuccess ;
}

// マウスイベントキャプチャー
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CaptureMouse( int idMouse )
{
	return	sglErrSuccess ;
}

SGLError SGLGenericWindow::ReleaseMouse( int idMouse )
{
	return	sglErrSuccess ;
}

// 描画インターフェース取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLGenericWindow::GetRenderContext
	( S3DRenderContextInterface::StereoViewIndex sviView )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( (pwvp != NULL) && m_flagCreated )
	{
		return	pwvp->BeginDrawView( this, false ) ;
	}
	return	NULL ;
}

void SGLGenericWindow::ReleaseRenderContext
				( S3DRenderContextInterface* context )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		pwvp->EndDrawView( this, context, false ) ;
		pwvp->FlipView( this, true, false ) ;
	}
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLGenericWindow::GetRenderDevice( void )
{
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		return	pwvp->GetRenderDevice() ;
	}
	return	NULL ;
}

// ウィンドウスレッド排他処理用ミューテックス変更
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::SetWindowUIThreadMutex( SSystem::SMutex * pMutex )
{
	m_pMutexUI = pMutex ;
}

// ウィンドウ作成（低水準）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CreateWindowSimply
	( const wchar_t * pwszWindowName,
				uint32_t nWidth, uint32_t nHeight )
{
	if ( m_flagCreated )
	{
		return	sglErrFailed ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		pwvp->OnChangeVirtualViewSize( this, nWidth, nHeight ) ;
		pwvp->OnChangePhysicalViewSize( this, nWidth, nHeight ) ;
	}
	m_strCaption = pwszWindowName ;
	m_modeCooperation = Window::modeWindow ;
	m_sizeVirtual.w = nWidth ;
	m_sizeVirtual.h = nHeight ;
	m_sizePhysical = m_sizeVirtual ;
	m_sizeLayoutOriginal = m_sizeVirtual ;
	//
	// new VirtualWindow( buf )
	//
	JNI::JavaObject	jobjBuf ;
	if ( jobjBuf.CreateByteBuffer( this, sizeof(SGLGenericWindow) ) == NULL )
	{
		return	sglErrFailed ;
	}
	if ( JavaObject::CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/VirtualWindow",
			"(L" JAVA_NIO_BYTEBUFFER ";)V", jobjBuf.GetObject() ) == NULL )
	{
		return	sglErrFailed ;
	}
	JavaObject::MakeGlobalRef() ;
	//
	// addView( ViewInterface view )
	//
	JNI::JavaObject	jobjSurfaceView
						( java_EntisGLS_getMainSurfaceView(), true ) ;
	jmethodID	jmidAddView =
		jobjSurfaceView.GetMethodID
			( "addView", "(L" ENTIS_GLS4_JAVA_PACKAGE "/ViewInterface;)V" ) ;
	if ( jmidAddView == NULL )
	{
		return	sglErrFailed ;
	}
	jobjSurfaceView.CallVoidMethod( jmidAddView, JavaObject::GetObject() ) ;
	//
	AddWindowToChain() ;
	m_flagCreated = true ;
	//
	if ( pwvp != NULL )
	{
		pwvp->OnAttachedWindow( this ) ;
	}
	UpdateWindowLayout() ;
	PostUpdate() ;
	//
	if ( m_jsobjMenu.GetObject() != NULL )
	{
		jmethodID	jmidSetMenu =
			JavaObject::GetMethodID
				( "setMenu", "(L" ENTIS_GLS4_JAVA_PACKAGE "/MenuData;)V" ) ;
		JavaObject::CallVoidMethod( jmidSetMenu, m_jsobjMenu.GetObject() ) ;
		m_jsobjMenu.DetachObject() ;
	}
	return	sglErrSuccess ;
}

// ウィンドウ破棄（低水準）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGenericWindow::CloseWindowSimply( void )
{
	if ( !m_flagCreated )
	{
		return	sglErrFailed ;
	}
	m_pMutexUI->Lock() ;
	SGLGenericWindow *		pParentWnd =
		ESLTypeCast<SGLGenericWindow>( m_refParentWnd.GetReference() ) ;
	if ( pParentWnd != NULL )
	{
		ssize_t	iChild = pParentWnd->m_arrChildren.FindPtr( this ) ;
		if ( iChild >= 0 )
		{
			pParentWnd->m_arrChildren.RemoveAt( iChild ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	//
	// detachView( ViewInterface view )
	//
	JNI::JavaObject	jobjSurfaceView( java_EntisGLS_getMainSurfaceView() ) ;
	jmethodID	jmidDetachView =
		jobjSurfaceView.GetMethodID
			( "detachView", "(L" ENTIS_GLS4_JAVA_PACKAGE "/ViewInterface;)V" ) ;
	if ( jmidDetachView == NULL )
	{
		return	sglErrFailed ;
	}
	atomic_int_t	countLocked = m_pMutexUI->UnlockAll() ;
	jobjSurfaceView.CallVoidMethod( jmidDetachView, JavaObject::GetObject() ) ;
	m_pMutexUI->Relock( countLocked ) ;
	//
	DetachWindowFromChain() ;
	//
	// OpenGL 解放
	//
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		pwvp->OnDetachedWindow( this ) ;
	}
	//
	// delete VirtualWindow
	//
	JavaObject::DetachJavaObject() ;
	//
	m_flagCreated = false ;
	return	sglErrSuccess ;
}

// ウィンドウのレイアウトに基づいて位置を調整する
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::UpdateWindowLayout( void )
{
	m_pMutexUI->Lock() ;
	if ( m_flagModeDisplay )
	{
		m_rectWindow.x = 0 ;
		m_rectWindow.y = 0 ;
		m_rectWindow.w = m_sizePhysicalDisplay.w ;
		m_rectWindow.h = m_sizePhysicalDisplay.h ;
	}
	else if ( m_flagsLayout != 0 )
	{
		m_rectWindow.x = m_ptLayoutOffset.x ;
		m_rectWindow.y = m_ptLayoutOffset.y ;
		m_rectWindow.w = m_sizeLayoutOriginal.w ;
		m_rectWindow.h = m_sizeLayoutOriginal.h ;
		//
		switch ( m_flagsLayout & layoutDockingMask )
		{
		case	layoutNothing:
			break ;
		case	layoutOffsetClient:
			break ;
		case	layoutDockingLeft:
		case	layoutDockingRight:
			switch ( m_flagsLayout & layoutAlignMask )
			{
			case	layoutAlignTop:
				break ;
			case	layoutAlignCenter:
				m_rectWindow.y +=
					(m_sizePhysicalDisplay.h - m_sizeLayoutOriginal.h) / 2 ;
				break ;
			case	layoutAlignBottom:
				m_rectWindow.y +=
					(m_sizePhysicalDisplay.h - m_sizeLayoutOriginal.h) ;
				break ;
			case	layoutAlignAccording:
				m_rectWindow.w = m_sizePhysicalDisplay.h
								* m_sizeLayoutOriginal.w / m_sizeLayoutOriginal.h ;
				m_rectWindow.h = m_sizePhysicalDisplay.h ;
				break ;
			}
			if ( (m_flagsLayout & layoutDockingMask) == layoutDockingRight )
			{
				m_rectWindow.x = m_sizePhysicalDisplay.w - m_rectWindow.w ;
			}
			break ;
		case	layoutDockingUpper:
		case	layoutDockingUnder:
			switch ( m_flagsLayout & layoutAlignMask )
			{
			case	layoutAlignLeft:
				break ;
			case	layoutAlignCenter:
				m_rectWindow.x +=
					(m_sizePhysicalDisplay.w - m_sizeLayoutOriginal.w) / 2 ;
				break ;
			case	layoutAlignRight:
				m_rectWindow.x +=
					(m_sizePhysicalDisplay.w - m_sizeLayoutOriginal.w) ;
				break ;
			case	layoutAlignAccording:
				m_rectWindow.h = m_sizePhysicalDisplay.w
						* m_sizeLayoutOriginal.h / m_sizeLayoutOriginal.w ;
				m_rectWindow.w = m_sizePhysicalDisplay.h ;
				break ;
			}
			if ( (m_flagsLayout & layoutDockingMask) == layoutDockingUnder )
			{
				m_rectWindow.y = m_sizePhysicalDisplay.h - m_rectWindow.h ;
			}
			break ;
		}
		m_sizeVirtual.w = m_rectWindow.w ;
		m_sizeVirtual.h = m_rectWindow.h ;
		m_sizePhysical = m_sizeVirtual ;
	}
	else
	{
		m_rectWindow.x = m_ptLayoutOffset.x ;
		m_rectWindow.y = m_ptLayoutOffset.y ;
		m_rectWindow.w = m_sizeLayoutOriginal.w ;
		m_rectWindow.h = m_sizeLayoutOriginal.h ;
	}
	SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
	if ( pwvp != NULL )
	{
		if ( m_flagModeDisplay )
		{
			pwvp->OnChangeVirtualViewSize
					( this, m_sizeVirtual.w, m_sizeVirtual.h ) ;
		}
		else
		{
			pwvp->OnChangeVirtualViewSize
					( this, m_sizePhysicalDisplay.w, m_sizePhysicalDisplay.h ) ;
		}
		pwvp->OnChangePhysicalViewSize
			( this, m_sizePhysicalDisplay.w, m_sizePhysicalDisplay.h ) ;
	}
	m_pMutexUI->Unlock() ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::DrawWindow( bool fOnWinThread )
{
	if ( !m_flagCreated )
	{
		return ;
	}
	if ( m_flagModeDisplay )
	{
		m_wvfFramework.DrawWindow( this, fOnWinThread, NULL ) ;
	}
	else
	{
		m_wvfFramework.DrawWindow( this, fOnWinThread, &m_rectWindow ) ;
	}
}

// 表示反映処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::FlipView( bool fVSync, bool fOnWinThread )
{
	m_wvfFramework.FlipView( this, fVSync, fOnWinThread ) ;
}

// EntisGLSurfaceView EntisGLS.getMainSurfaceView()
//////////////////////////////////////////////////////////////////////////////
jobject SGLGenericWindow::java_EntisGLS_getMainSurfaceView( void )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetMainSurfaceView =
		jsclsEntisGLS.GetStaticMethodID
			( "getMainSurfaceView",
				"()L" ENTIS_GLS4_JAVA_PACKAGE "/EntisGLSurfaceView;" ) ;
	return	jsclsEntisGLS.CallStaticObjectMethod( jmidGetMainSurfaceView ) ;
}

// void EntisGLS.postUpdateView()
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::java_EntisGLS_postUpdateView( void )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidPostUpdateView =
		jsclsEntisGLS.GetStaticMethodID( "postUpdateView", "()V" ) ;
	jsclsEntisGLS.CallStaticVoidMethod( jmidPostUpdateView ) ;
}

// boolean EntisGLS.callNativeOnUIThread( ByteBuffer buf )
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::java_EntisGLS_callNativeOnUIThread( jobject buf )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidCallNativeOnUIThread =
		jsclsEntisGLS.GetStaticMethodID
			( "callNativeOnUIThread",
				"(L" JAVA_NIO_BYTEBUFFER ";)Z" ) ;
	return	jsclsEntisGLS.CallStaticBooleanMethod
				( jmidCallNativeOnUIThread, buf ) ;
}

// boolean EntisGLS.callNativeOnRenderingThread( ByteBuffer buf, boolean fDelay )
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::java_EntisGLS_callNativeOnRenderingThread( jobject buf, bool flagDelay )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidCallNativeOnRenderingThread =
		jsclsEntisGLS.GetStaticMethodID
			( "callNativeOnRenderingThread",
				"(L" JAVA_NIO_BYTEBUFFER ";Z)Z" ) ;
	return	jsclsEntisGLS.CallStaticBooleanMethod
				( jmidCallNativeOnRenderingThread, buf, flagDelay ) ;
}

// boolean EntisGLS.procedureAsyncNoRenderingThread( ByteBuffer buf )
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::java_EntisGLS_callNativeOnAsyncNoRenderingThread( jobject buf )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidCallNativeAsyncNoRenderingThread =
		jsclsEntisGLS.GetStaticMethodID
			( "callNativeAsyncNoRenderingThread",
				"(L" JAVA_NIO_BYTEBUFFER ";)Z" ) ;
	return	jsclsEntisGLS.CallStaticBooleanMethod
				( jmidCallNativeAsyncNoRenderingThread, buf ) ;
}

// 画面サイズ変更通知
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnSurfaceChanged( int width, int height )
{
	m_pMutexUI->Lock() ;
	m_sizePhysicalDisplay.w = width ;
	m_sizePhysicalDisplay.h = height ;
	UpdateWindowLayout() ;
	//
	if ( m_pCommandHandler != NULL )
	{
		SString		strCmd = SysCommandId::WindowSizeChanged ;
		uint32_t	wClient = (uint32_t) width ;
		uint32_t	hClient = (uint32_t) height ;
		m_pCommandHandler->OnCommand
			( this, strCmd.GetArray(),
				(((int64_t) height) << 32) | (uint32_t) width, 0 ) ;
	}
	m_pMutexUI->Unlock() ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnDraw( void )
{
	m_pMutexUI->Lock() ;
	if ( !(m_flagsOption & flagInvisibleWindow) )
	{
		SGLWindowViewProducer *	pwvp = m_wvfFramework.GetView() ;
		if ( pwvp != NULL )
		{
			if ( pwvp->AttachViewThread( this ) == sglErrSuccess )
			{
				DrawWindow( true ) ;
				FlipView( false, true ) ;
				pwvp->DetachViewThread( this ) ;
			}
		}
	}
	m_pMutexUI->Unlock() ;
	m_signalDonePaint.SetSignal() ;
}

// タッチ通知
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnTouchedDown( double x, double y, int id )
{
	SGLSecondaryViewProducer::SetCurrent( m_wvfFramework.GetCurrentView() ) ;
	//
	int64_t	nFlags =
		((SGLMouseInterface::LeftButtonID
				<< SGLMouseInterface::ButtonIDShifter))
			| (id & SGLMouseInterface::MouseIDMask)
			| SGLMouseInterface::TouchFlag ;
	bool	fProcessed = false ;
	m_pMutexUI->Lock() ;
	if ( ++ m_countTouching == 1 )
	{
		m_idPrimaryTouch = id ;
		m_flagPrimaryTouchDown = true ;
	}
	if ( m_pDirectMouseHandler != NULL )
	{
		fProcessed =
			m_pDirectMouseHandler->OnButtonDown
				( this, x - m_rectWindow.x, y - m_rectWindow.y, nFlags ) ;
	}
	if ( !fProcessed && (m_pMouseHandler != NULL) )
	{
		S2DDVector	vMouse( x, y ) ;
		ClientPositionFromScreen( vMouse ) ;
		fProcessed =
			m_pMouseHandler->OnButtonDown
				( this, vMouse.x, vMouse.y, nFlags ) ;
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed ;
}

bool SGLGenericWindow::OnTouchedUp( double x, double y, int id )
{
	SGLSecondaryViewProducer::SetCurrent( m_wvfFramework.GetCurrentView() ) ;
	//
	int64_t	nFlags =
		((SGLMouseInterface::LeftButtonID
				<< SGLMouseInterface::ButtonIDShifter))
			| (id & SGLMouseInterface::MouseIDMask)
			| SGLMouseInterface::TouchFlag ;
	bool	fProcessed = false ;
	m_pMutexUI->Lock() ;
	if ( m_pDirectMouseHandler != NULL )
	{
		fProcessed =
			m_pDirectMouseHandler->OnButtonUp
				( this, x - m_rectWindow.x, y - m_rectWindow.y, nFlags ) ;
	}
	if ( !fProcessed && (m_pMouseHandler != NULL) )
	{
		S2DDVector	vMouse( x, y ) ;
		ClientPositionFromScreen( vMouse ) ;
		fProcessed =
			m_pMouseHandler->OnButtonUp
				( this, vMouse.x, vMouse.y, nFlags ) ;
	}
	if ( m_pDirectMouseHandler != NULL )
	{
		m_pDirectMouseHandler->OnMouseLeave( this, nFlags ) ;
	}
	if ( m_pMouseHandler != NULL )
	{
		m_pMouseHandler->OnMouseLeave( this, nFlags ) ;
	}
	if ( id == m_idPrimaryTouch )
	{
		m_flagPrimaryTouchDown = false ;
	}
	if ( m_countTouching > 0 )
	{
		m_countTouching -- ;
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed ;
}

bool SGLGenericWindow::OnTouchedMoved( double x, double y, int id )
{
	SGLSecondaryViewProducer::SetCurrent( m_wvfFramework.GetCurrentView() ) ;
	//
	int64_t	nFlags = (id & SGLMouseInterface::MouseIDMask)
								| SGLMouseInterface::TouchFlag ;
	bool	fProcessed = false ;
	m_pMutexUI->Lock() ;
	if ( m_pDirectMouseHandler != NULL )
	{
		fProcessed =
			m_pDirectMouseHandler->OnMouseMove
				( this, x - m_rectWindow.x, y - m_rectWindow.y, nFlags ) ;
	}
	if ( !fProcessed && (m_pMouseHandler != NULL) )
	{
		S2DDVector	vMouse( x, y ) ;
		ClientPositionFromScreen( vMouse ) ;
		fProcessed =
			m_pMouseHandler->OnMouseMove
				( this, vMouse.x, vMouse.y, nFlags ) ;
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed ;
}

// キー入力通知
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnKeyDown( int key )
{
	bool	fProcessed = false ;
	bool	fJoyButton = false ;
	m_pMutexUI->Lock() ;
	if ( (key >= 0) & (key < 0x100) )
	{
		int	joyButton = g_joyButtonFromAndroidKeyCode[key] ;
		if ( joyButton != 0 )
		{
			m_maskJoyButtonPushed |= (1 << (joyButton - 1)) ;
			fJoyButton = true ;
			//
			if ( m_pCommandHandler != NULL )
			{
				SString	strCmd = SysCommandId::WindowPollJoyStick ;
				m_pCommandHandler->OnCommand
						( this, strCmd.GetArray(), 0, 0 ) ;
			}
		}
	}
	if ( key == 0x04 /*KeyEvent.KEYCODE_BACK*/ )
	{
		if ( m_pCommandHandler != NULL )
		{
			SString	strCmd = SysCommandId::AppBack ;
			if ( !m_pCommandHandler->OnCommand
					( this, strCmd.GetArray(), 0, 0 ) )
			{
				strCmd = SysCommandId::AppExit ;
				m_pCommandHandler->OnCommand
					( this, strCmd.GetArray(), 0, 0 ) ;
			}
			fProcessed = true ;
		}
	}
	if ( !fProcessed && (m_pKeyHandler != NULL) )
	{
		int	vkey = VirtualKeyFromAndroidKeyCode(key) ;
		if ( vkey != 0 )
		{
			fProcessed = m_pKeyHandler->OnKeyDown( this, vkey, 0 ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed | fJoyButton ;
}

bool SGLGenericWindow::OnKeyUp( int key )
{
	bool	fProcessed = false ;
	bool	fJoyButton = false ;
	m_pMutexUI->Lock() ;
	if ( (key >= 0) & (key < 0x100) )
	{
		int	joyButton = g_joyButtonFromAndroidKeyCode[key] ;
		if ( joyButton != 0 )
		{
			m_maskJoyButtonPushed &= ~(1 << (joyButton - 1)) ;
			fJoyButton = true ;
			//
			if ( m_pCommandHandler != NULL )
			{
				SString	strCmd = SysCommandId::WindowPollJoyStick ;
				m_pCommandHandler->OnCommand
						( this, strCmd.GetArray(), 0, 0 ) ;
			}
		}
	}
	if ( key == 0x04 /*KeyEvent.KEYCODE_BACK*/ )
	{
		if ( m_pCommandHandler != NULL )
		{
			fProcessed = true ;
		}
	}
	if ( !fProcessed && (m_pKeyHandler != NULL) )
	{
		int	vkey = VirtualKeyFromAndroidKeyCode(key) ;
		if ( vkey != 0 )
		{
			fProcessed = m_pKeyHandler->OnKeyUp( this, vkey, 0 ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed | fJoyButton ;
}

bool SGLGenericWindow::OnChar( int code )
{
	bool	fProcessed = false ;
	m_pMutexUI->Lock() ;
	if ( code != 0 )
	{
		if ( m_pCharInputHandler != NULL )
		{
			m_pCharInputHandler->OnChar( this, (uint16_t) code ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	return	fProcessed ;
}

// ジョイスティック通知
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnJoystickAxis( float x, float y, float z, float rz )
{
	m_flagJoyStick = true ;
	m_vJoystickPos.x = x ;
	m_vJoystickPos.y = y ;
	m_vJoystickPos.z = z ;
	m_vJoystickPos.w = rz ;
	//
	if ( m_pCommandHandler != NULL )
	{
		SString	strCmd = SysCommandId::WindowPollJoyStick ;
		m_pCommandHandler->OnCommand
				( this, strCmd.GetArray(), 0, 0 ) ;
	}
	return	true ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnTimer( void )
{
	m_pMutexUI->Lock() ;
	if ( m_pTimerHandler != NULL )
	{
		m_pTimerHandler->OnTimer( this, 1 ) ;
	}
	m_pMutexUI->Unlock() ;
	//
	bool	fUpdate = false ;
	m_csViewSync.Lock() ;
	if ( m_pViewSync != NULL )
	{
		fUpdate = (m_pViewSync->WaitForView(0) == sglErrSuccess) ;
	}
	m_csViewSync.Unlock() ;
	if ( fUpdate )
	{
		PostUpdate() ;
	}
}

// システムイベント通知
//////////////////////////////////////////////////////////////////////////////
void SGLGenericWindow::OnSystemEvent( const wchar_t * pwszSysCommand )
{
	m_pMutexUI->Lock() ;
	if ( m_pCommandHandler != NULL )
	{
		SString	strCmd = pwszSysCommand ;
		m_pCommandHandler->OnCommand( this, strCmd.GetArray(), 0, 0 ) ;
	}
	m_pMutexUI->Unlock() ;
}

// メニューコマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLGenericWindow::OnMenuCommand( int id )
{
	const wchar_t *	pwszID = SGLWindowMenu::GetCommandIDOf( id ) ;
	if ( pwszID != NULL )
	{
		m_pMutexUI->Lock() ;
		if ( m_pCommandHandler != NULL )
		{
			SString	strCmd = pwszID ;
			m_pCommandHandler->OnCommand( this, strCmd.GetArray(), 0, 0 ) ;
		}
		m_pMutexUI->Unlock() ;
		return	true ;
	}
	return	false ;
}

// ジョイボタン変換テーブル
//////////////////////////////////////////////////////////////////////////////
const int	SGLGenericWindow::g_joyButtonFromAndroidKeyCode[0x100] =
{
	// 0x00
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x10
	0, 0, 0, joyStickUp, joyStickDown, joyStickLeft, joyStickRight, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x20
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x30
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x40
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x50
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x60
	joyButtonA, joyButtonB, joyButtonC, joyButtonX,
	joyButtonY, joyButtonZ, joyButtonL1, joyButtonR1,
	joyButtonL2, joyButtonR2, 0, 0, joyButtonStart, joyButtonSelect, 0, 0,
	// 0x70
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x80
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0x90
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xA0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xB0
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, joyButtonA, joyButtonB, joyButtonC, joyButtonX,
	// 0xC0
	joyButtonY, joyButtonZ, joyButtonL1, joyButtonR1,
	joyButtonL2, joyButtonR2, joyButtonStart, joyButtonSelect,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0xD0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xE0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xF0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
} ;

// 仮想キーコード変換テーブル
//////////////////////////////////////////////////////////////////////////////
const int	SGLGenericWindow::g_vkeyFromAndroidKeyCode[0x100] =
{
	// 0x00
	0, 0, 0, vkeyHome, 0, 0, 0, '0',
	'1', '2', '3', '4', '5', '6', '7', '8',
	// 0x10
	'9', 0xBA /*'*'*/, '#', vkeyUp, vkeyDown, vkeyLeft, vkeyRight, 0,
	0, 0, 0, 0, 0, 'A', 'B', 'C',
	// 0x20
	'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
	'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
	// 0x30
	'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0xBC /*',<'*/,
	0xBE /*'.>'*/, vkeyMenu, 0, vkeyShift, vkeyShift, vkeyTab, ' ', 0,
	// 0x40
	0, 0, vkeyReturn, vkeyBack,
	0, 0xBD /*'-='*/, 0xDE/*'^~'*/, 0xC0 /*'@`'*/,
	0xDB /*'[{'*/, 0xDD /*']}'*/, 0xBB /*';+'*/, 0xBA /*':*'*/,
	0xBF /*'/?'*/, 0xC0 /*'@'*/, 0, 0,
	// 0x50
	0, 0xBB /*'+'*/, vkeyMenu, 0, 0, 0, 0, 0,
	0, 0, 0, 0, vkeyPageUp, vkeyPageDown, 0, 0,
	// 0x60
	vkeyControl, 0, vkeyFunction1, vkeyFunction2,
	vkeyFunction3, vkeyFunction4, vkeyFunction5, vkeyFunction6,
	vkeyFunction7, vkeyFunction8, vkeyFunction9, vkeyFunction10,
	vkeyFunction11, vkeyFunction12, vkeyInsert, vkeyEscape,
	// 0x70
	0, 0, vkeyEscape, 0xE2/*'\_'*/, 0xDC/*'\|'*/, 0x19, 0, 0x1C,
	0x15, 0, 0, 0, vkeyInsert, 0, 0, 0,
	// 0x80
	0, 0, 0, vkeyFunction1,
	vkeyFunction2, vkeyFunction3, vkeyFunction4, vkeyFunction5,
	vkeyFunction6, vkeyFunction7, vkeyFunction8, vkeyFunction9,
	vkeyFunction10, vkeyFunction11, vkeyFunction12, vkeyNumLock,
	// 0x90
	vkeyNumPad0, vkeyNumPad1, vkeyNumPad2, vkeyNumPad3,
	vkeyNumPad4, vkeyNumPad5, vkeyNumPad6, vkeyNumPad7,
	vkeyNumPad8, vkeyNumPad9, vkeyNumPadDivide, 0,
	vkeyNumPadSubtract, vkeyNumPadAdd, vkeyNumPadDecimal, vkeyNumPadSeparator,
	// 0xA0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xB0
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, vkeyNumPad1, vkeyNumPad2, vkeyNumPad3, vkeyNumPad4,
	// 0xC0
	vkeyNumPad5, vkeyNumPad6, vkeyNumPad7, vkeyNumPad8, vkeyNumPad9, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0xD0
	0, 0, 0, 0, 0, 0, 0, 0,  '\\', 0, 0, 0, 0, 0, 0, 0,
	// 0xE0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	// 0xF0
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
} ;


