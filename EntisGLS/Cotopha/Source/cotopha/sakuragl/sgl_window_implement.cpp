
#include <gls.h>
#include <sakuragl/sgl_window.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ・ラッパ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindow, SGLAbstractWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::SGLWindow( void )
	: m_render( NULL, false )
{
	m_pWindow = new Window ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::~SGLWindow( void )
{
	delete	m_pWindow ;
	m_pWindow = NULL ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CreateDisplay
	( const wchar_t * pszWindowName,
		Window::CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	return	m_pWindow->CreateDisplay
		( pszWindowName, mode, nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CloseDisplay( void )
{
	return	m_pWindow->CloseDisplay() ;
}

// オプション機能フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLWindow::GetOptionalFlags( void )
{
	return	m_pWindow->GetOptionalFlags() ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SetOptionalFlags( uint64_t nFlags )
{
	m_pWindow->SetOptionalFlags( nFlags ) ;
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ChangeCooperationLevel( Window::CooperationMode mode )
{
	return	m_pWindow->ChangeCooperationLevel( mode ) ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ChangeDisplaySize
	( uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	return	m_pWindow->ChangeDisplaySize
				( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
}

// 仮想ディスプレイサイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetDisplaySize( SGLSize& sizeDisplay )
{
	return	m_pWindow->GetDisplaySize( sizeDisplay ) ;
}

// 物理モニタの解像度を変更するか？
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::EnableChangePhysicalMode( bool flagEnable )
{
	return	m_pWindow->EnableChangePhysicalMode( flagEnable ) ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::EnableZBuffer( bool flagZBuffer )
{
	return	m_pWindow->EnableZBuffer( flagZBuffer ) ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetStereoDisplayMode
	( const wchar_t * pszMethodID, uint64_t nParam )
{
	return	m_pWindow->SetStereoDisplayMode( pszMethodID, nParam ) ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsSupportedStereoDisplayMode( const wchar_t * pszMethodID )
{
	return	m_pWindow->IsSupportedStereoDisplayMode( pszMethodID ) ;
}

// 仮想ディスプレイ・ウィンドウ初期座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::InitWindowPosition
	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize )
{
	return	m_pWindow->InitWindowPosition( xPos, yPos, pInitExSize ) ;
}

// 仮想ディスプレイ・ウィンドウの通常座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetNormalWindowPosition
	( SGLPoint& ptWindow, SGLSize * pWindowSize )
{
	return	m_pWindow->GetNormalWindowPosition( ptWindow, pWindowSize ) ;
}

// 仮想ディスプレイ・ウィンドウ内表示座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetInternalDisplayPosition
				( SGLImageRect& rctRender, SGLImageRect& rctDisplay )
{
	return	m_pWindow->GetInternalDisplayPosition( rctRender, rctDisplay ) ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	Image *	imgTile = NULL ;
	Image *	imgLeft = NULL ;
	Image *	imgRight = NULL ;
	Image *	imgUpper = NULL ;
	Image *	imgUnder = NULL ;
	if ( pTile != NULL )
	{
		imgTile = pTile->GetImageObject() ;
	}
	if ( pLeft != NULL )
	{
		imgLeft = pLeft->GetImageObject() ;
	}
	if ( pRight != NULL )
	{
		imgRight = pRight->GetImageObject() ;
	}
	if ( pUpper != NULL )
	{
		imgUpper = pUpper->GetImageObject() ;
	}
	if ( pUnder != NULL )
	{
		imgUnder = pUnder->GetImageObject() ;
	}
	return	m_pWindow->SetExteriorBackgroundFrame
				( nFlags, rgbColor, imgTile,
					imgLeft, imgRight, imgUpper, imgUnder ) ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	SGLWindow *	pSGLWndParent = ESLTypeCast<SGLWindow>( pParentWnd ) ;
	Window *	pWndParent = NULL ;
	if ( pSGLWndParent != NULL )
	{
		pWndParent = pSGLWndParent->m_pWindow ;
	}
	return	m_pWindow->CreateWindow
				( pszWindowName, nWidth, nHeight, nFlags, pWndParent ) ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CloseWindow( void )
{
	return	m_pWindow->CloseWindow() ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetWindowLayout( uint32_t nFlags, int xPos, int yPos )
{
	return	m_pWindow->SetWindowLayout( nFlags, xPos, yPos ) ;
}

// クライアント座標→スクリーン座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLWindow::ScreenPositionFromClient( S2DDVector& vClient )
{
	return	m_pWindow->ScreenPositionFromClient( vClient ) ;
}

// スクリーン座標→クライアント座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLWindow::ClientPositionFromScreen( S2DDVector& vScreen )
{
	return	m_pWindow->ClientPositionFromScreen( vScreen ) ;
}

// 画面の更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostUpdate( const SGLImageRect* pUpdate )
{
	return	m_pWindow->PostUpdate( pUpdate ) ;
}

// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::UpdateWindow( Window::UpdateParameter * pUpdate )
{
	return	m_pWindow->UpdateWindow( pUpdate ) ;
}

// ユーザー入力処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ProcessUserInput( int64_t msecTimeout )
{
	return	m_pWindow->ProcessUserInput( msecTimeout ) ;
}

// 描画スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostRenderingThread( SSystem::SProcedure * pProc )
{
	return	m_pWindow->PostRenderingThread( pProc ) ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostUIThread( SSystem::SProcedure * pProc )
{
	return	m_pWindow->PostUIThread( pProc ) ;
}

// ウィンドウがアクティブ（最前面）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsWindowActive( void )
{
	return	m_pWindow->IsWindowActive() ;
}

// ウィンドウキャプション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetWindowCaption( const wchar_t * pszWindowName )
{
	return	m_pWindow->SetWindowCaption( pszWindowName ) ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ShowCursor( bool fShow )
{
	return	m_pWindow->ShowCursor( fShow ) ;
}

// マウスカーソル表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsShowCursor( void )
{
	return	m_pWindow->IsShowCursor() ;
}

// マウスカーソル変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetCursor( const wchar_t * pszCursorID )
{
	return	m_pWindow->SetCursor( pszCursorID ) ;
}

// マウスカーソル座標移動
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::MoveCursorPosition
	( int32_t xPos, int32_t yPos, int idMouse )
{
	return	m_pWindow->MoveCursorPosition( xPos, yPos, idMouse ) ;
}

// マウスカーソル座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetCursorPosition( SGLPoint& ptCursor, int idMouse )
{
	return	m_pWindow->GetCursorPosition( ptCursor, idMouse ) ;
}

// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
//////////////////////////////////////////////////////////////////////////////
int SGLWindow::GetMonitorFrequency( void )
{
	return	m_pWindow->GetMonitorFrequency() ;
}

// ウィンドウメニューの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::AttachMenu( SGLWindowMenu * pMenu )
{
	if ( pMenu != NULL )
	{
		return	m_pWindow->AttachMenu( pMenu->GetMenuObject() ) ;
	}
	else
	{
		return	m_pWindow->AttachMenu( NULL ) ;
	}
}

// 描画ハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLPaintInterface *
	SGLWindow::SetPaintInterface( SGLPaintInterface * pPaint )
{
	SGLAbstractWindow::SetPaintInterface( pPaint ) ;
	return	m_pWindow->SetPaintInterface( pPaint ) ;
}

SGLPaintInterface *
	SGLWindow::SetDirectPaintInterface( SGLPaintInterface * pPaint )
{
	SGLAbstractWindow::SetDirectPaintInterface( pPaint ) ;
	return	m_pWindow->SetDirectPaintInterface( pPaint ) ;
}

// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLTimerInterface *
	SGLWindow::SetTimerInterface( SGLTimerInterface * pTimer )
{
	SGLAbstractWindow::SetTimerInterface( pTimer ) ;
	return	m_pWindow->SetTimerInterface( pTimer ) ;
}

// マウス入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLMouseInterface *
	SGLWindow::SetMouseInterface( SGLMouseInterface * pMouse )
{
	SGLAbstractWindow::SetMouseInterface( pMouse ) ;
	return	m_pWindow->SetMouseInterface( pMouse ) ;
}

SGLMouseInterface *
	SGLWindow::SetDirectMouseInterface( SGLMouseInterface * pMouse )
{
	SGLAbstractWindow::SetDirectMouseInterface( pMouse ) ;
	return	m_pWindow->SetDirectMouseInterface( pMouse ) ;
}

// マウスイベントキャプチャー
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CaptureMouse( int idMouse )
{
	return	m_pWindow->CaptureMouse( idMouse ) ;
}

SGLError SGLWindow::ReleaseMouse( int idMouse )
{
	return	m_pWindow->ReleaseMouse( idMouse ) ;
}

// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLKeyInterface * SGLWindow::SetKeyInterface( SGLKeyInterface * pKey )
{
	SGLAbstractWindow::SetKeyInterface( pKey ) ;
	return	m_pWindow->SetKeyInterface( pKey ) ;
}

// 文字入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCharInputInterface *
	SGLWindow::SetCharInputInterface( SGLCharInputInterface * pChar )
{
	SGLAbstractWindow::SetCharInputInterface( pChar ) ;
	return	m_pWindow->SetCharInputInterface( pChar ) ;
}

// コマンド・インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCommandInterface *
	SGLWindow::SetCommandInterface( SGLCommandInterface * pCmd )
{
	SGLAbstractWindow::SetCommandInterface( pCmd ) ;
	return	m_pWindow->SetCommandInterface( pCmd ) ;
}

// 描画インターフェース取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLWindow::GetRenderContext
		( S3DRenderContextInterface::StereoViewIndex sviView )
{
	m_render.AttachRenderContext
			( m_pWindow->GetRenderContext(sviView), false ) ;
	return	&m_render ;
}

void SGLWindow::ReleaseRenderContext( S3DRenderContextInterface * context )
{
	if ( &m_render == context )
	{
		m_pWindow->ReleaseRenderContext( m_render.GetRenderContext() ) ;
		m_render.AttachRenderContext( NULL, false ) ;
	}
}

// Window オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
Window * SGLWindow::GetWindowObject( void ) const
{
	return	m_pWindow ;
}



