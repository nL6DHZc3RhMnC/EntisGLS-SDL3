
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 描画ハンドラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLPaintInterface )

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintInterface::OnPaint( Window * pWnd, RenderContext* context )
{
}

// 描画前フレーム準備処理（全視点共通処理）
//////////////////////////////////////////////////////////////////////////////
void SGLPaintInterface::OnPrepareFrame( Window * pWnd )
{
}

// 全描画完了
//////////////////////////////////////////////////////////////////////////////
void SGLPaintInterface::OnFinishedFrame( Window * pWnd )
{
}



//////////////////////////////////////////////////////////////////////////////
// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLTimerInterface )

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLTimerInterface::OnTimer( Window * pWnd, uint64_t idTimer )
{
}



//////////////////////////////////////////////////////////////////////////////
// ユーザー入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLMouseInterface )

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLMouseInterface::OnMouseMove
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

void SGLMouseInterface::OnMouseLeave( Window * pWnd, int64_t nFlags )
{
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLMouseInterface::OnMouseWheel
	( Window * pWnd, int32_t zDelta,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLMouseInterface::OnButtonDown
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonDown( pWnd, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonDown( pWnd, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLMouseInterface::OnButtonUp
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonUp( pWnd, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonUp( pWnd, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLMouseInterface::OnButtonDblClk
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonDblClk( pWnd, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonDblClk( pWnd, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLMouseInterface::OnLButtonDown
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLMouseInterface::OnLButtonUp
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLMouseInterface::OnLButtonDblClk
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLMouseInterface::OnRButtonDown
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLMouseInterface::OnRButtonUp
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLMouseInterface::OnRButtonDblClk
	( Window * pWnd, int32_t xPos, int32_t yPos, int64_t nFlags )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLKeyInterface )

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLKeyInterface::OnKeyDown
	( Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	return	false ;
}

bool SGLKeyInterface::OnKeyUp
	( Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	return	false ;
}

// フォーカス
//////////////////////////////////////////////////////////////////////////////
void SGLKeyInterface::OnSetFocus( Window * pWnd )
{
}

void SGLKeyInterface::OnKillFocus( Window * pWnd )
{
}

// シリアライズ用キーコードと名前対応
const SSystem::SXMLDocument::AttrInteger
	SakuraGL::g_aiVirtualKeyCode[100] =
{
	{ L"A", L'A' },
	{ L"B", L'B' },
	{ L"C", L'C' },
	{ L"D", L'D' },
	{ L"E", L'E' },
	{ L"F", L'F' },
	{ L"G", L'G' },
	{ L"H", L'H' },
	{ L"I", L'I' },
	{ L"J", L'J' },
	{ L"K", L'K' },
	{ L"L", L'L' },
	{ L"M", L'M' },
	{ L"N", L'N' },
	{ L"O", L'O' },
	{ L"P", L'P' },
	{ L"Q", L'Q' },
	{ L"R", L'R' },
	{ L"S", L'S' },
	{ L"T", L'T' },
	{ L"U", L'U' },
	{ L"V", L'V' },
	{ L"W", L'W' },
	{ L"X", L'X' },
	{ L"Y", L'Y' },
	{ L"Z", L'Z' },
	{ L"0", L'0' },
	{ L"1", L'1' },
	{ L"2", L'2' },
	{ L"3", L'3' },
	{ L"4", L'4' },
	{ L"5", L'5' },
	{ L"6", L'6' },
	{ L"7", L'7' },
	{ L"8", L'8' },
	{ L"9", L'9' },
	{ L"LClick", vkeyMouseLeft },
	{ L"LButton", vkeyMouseLeft },
	{ L"RClick", vkeyMouseRight },
	{ L"RButton", vkeyMouseRight },
	{ L"MClick", vkeyMouseMiddle },
	{ L"MButton", vkeyMouseMiddle },
	{ L"BackSpace", vkeyBack },
	{ L"Tab", vkeyTab },
	{ L"Return", vkeyReturn },
	{ L"Shift", vkeyShift },
	{ L"Control", vkeyControl },
	{ L"Menu", vkeyMenu },
	{ L"Pause", vkeyPause },
	{ L"Capital", vkeyCapital },
	{ L"Escape", vkeyEscape },
	{ L"Space", vkeySpace },
	{ L"PageUp", vkeyPageUp },
	{ L"PageDown", vkeyPageDown },
	{ L"End", vkeyEnd },
	{ L"Home", vkeyHome },
	{ L"Left", vkeyLeft },
	{ L"Up", vkeyUp },
	{ L"Right", vkeyRight },
	{ L"Down", vkeyDown },
	{ L"Insert", vkeyInsert },
	{ L"Delete", vkeyDelete },
	{ L"Help", vkeyHelp },
	{ L"NumPad0", vkeyNumPad0 },
	{ L"NumPad1", vkeyNumPad1 },
	{ L"NumPad2", vkeyNumPad2 },
	{ L"NumPad3", vkeyNumPad3 },
	{ L"NumPad4", vkeyNumPad4 },
	{ L"NumPad5", vkeyNumPad5 },
	{ L"NumPad6", vkeyNumPad6 },
	{ L"NumPad7", vkeyNumPad7 },
	{ L"NumPad8", vkeyNumPad8 },
	{ L"NumPad9", vkeyNumPad9 },
	{ L"NumPadMultiply", vkeyNumPadMultiply },
	{ L"NumPadAdd", vkeyNumPadAdd },
	{ L"NumPadSeparator", vkeyNumPadSeparator },
	{ L"NumPadSubtract", vkeyNumPadSubtract },
	{ L"NumPadDecimal", vkeyNumPadDecimal },
	{ L"NumPadDivide", vkeyNumPadDivide },
	{ L"Function1", vkeyFunction1 },
	{ L"Function2", vkeyFunction2 },
	{ L"Function3", vkeyFunction3 },
	{ L"Function4", vkeyFunction4 },
	{ L"Function5", vkeyFunction5 },
	{ L"Function6", vkeyFunction6 },
	{ L"Function7", vkeyFunction7 },
	{ L"Function8", vkeyFunction8 },
	{ L"Function9", vkeyFunction9 },
	{ L"Function10", vkeyFunction10 },
	{ L"Function11", vkeyFunction11 },
	{ L"Function12", vkeyFunction12 },
	{ L"NumLock", vkeyNumLock },
	{ L"Scroll", vkeyScroll },
	{ L"Play", vkeyPlay },
	{ NULL, 0 },
} ;



//////////////////////////////////////////////////////////////////////////////
// 文字入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLCharInputInterface )

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLCharInputInterface::OnChar( Window * pWnd, uint16_t codeChar )
{
	return	false ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLCharInputInterface::OnStartComposition
	( Window * pWnd, SGLInputStartComposition& iscForm )
{
	return	false ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLCharInputInterface::OnEndComposition( Window * pWnd )
{
	return	false ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLCharInputInterface::OnCompositionString
	( Window * pWnd, const SGLInputCompositionString& icsString )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// コマンド・インターフェース
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)
const wchar_t *	SysCommandId::AppExit = L"ID_APP_EXIT" ;
const wchar_t *	SysCommandId::AppBack = L"ID_APP_BACK" ;
const wchar_t *	SysCommandId::AppSuspend = L"ID_APP_SUSPEND" ;
const wchar_t *	SysCommandId::AppResume = L"ID_APP_RESUME" ;
const wchar_t *	SysCommandId::AppDestroy = L"ID_APP_DESTROY" ;
const wchar_t *	SysCommandId::WindowActive = L"ID_WINDOW_ACTIVE" ;
const wchar_t *	SysCommandId::WindowInactive = L"ID_WINDOW_INACTIVE" ;
const wchar_t *	SysCommandId::WindowSizeChanged = L"ID_WINDOW_SIZE_CHANGED" ;
const wchar_t *	SysCommandId::WindowPollJoyStick = L"ID_WINDOW_POLL_JOY_STICK" ;
#endif

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLCommandInterface )

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLCommandInterface::OnCommand
	( Window * pWnd, const uint16_t * pszCmd,
					int64_t nParam, int64_t nCode )
{
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// フレーム更新
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)
void SGLAbstractWindow::UpdateParameter::WaitFrame
			( uint32_t msecCurrentRendering, uint32_t freqMonitor )
{
	int64_t	msecCurrent = SSystem::CurrentMilliSec() ;
	int32_t	msecPast = (int32_t) (msecCurrent - this->msecLastUpdate) ;
	this->msecLastUpdate = msecCurrent ;
	//
	if ( (this->framesPerSec != 0)
		&& (this->flagsUpdate & Window::updateWaitFrames) )
	{
		//
		// フレーム同期のための調整
		//
		int32_t	msecIdealFrame =
					(this->framesPast * 1000
						+ (this->framesPerSec >> 1)) / this->framesPerSec ;
		int32_t	msecVirtualPast = msecPast + this->msecFrameError ;
		int32_t	msecWait = msecIdealFrame - msecVirtualPast ;
		if ( msecWait >= 0 )
		{
			//
			// 速く進みすぎているので待機
			//
			if ( this->flagsUpdate & Window::updateVSync )
			{
				if ( freqMonitor != 0 )
				{
					msecWait -= 500 / freqMonitor ;
				}
				else
				{
					msecWait >>= 1 ;
				}
			}
			else
			{
				msecWait -= 1 ;
			}
			if ( msecWait > 0 )
			{
				SSystem::SleepMilliSec( msecWait ) ;
				//
				msecCurrent = SSystem::CurrentMilliSec() ;
				msecPast += (int32_t) (msecCurrent - this->msecLastUpdate) ;
				this->msecLastUpdate = msecCurrent ;
			}
			//
			// 進んだ／遅れた時間累積
			//
			msecVirtualPast = msecPast + this->msecFrameError ;
			this->msecFrameError = msecVirtualPast - msecIdealFrame ;
			//
			if ( this->framesPast > 1 )
			{
				//
				// 次に進めるフレーム数を減らすか判定
				//
				uint32_t	msecPlayInFrame = 3 ;
				if ( this->framesPerSec >= 120 )
				{
					msecPlayInFrame = 1 ;
				}
				uint32_t	framesAsLastRendering =
					((this->msecRendering + msecPlayInFrame)
									* this->framesPerSec + 999) / 1000 ;
				uint32_t	framesAsCurrentRendering =
					((msecCurrentRendering + msecPlayInFrame)
									* this->framesPerSec + 999) / 1000 ;
				if ( framesAsCurrentRendering < framesAsLastRendering )
				{
					framesAsCurrentRendering = framesAsLastRendering ;
				}
				if ( framesAsCurrentRendering < this->framesPast )
				{
					this->framesPast = framesAsCurrentRendering ;
				}
			}
			else
			{
				this->framesPast = 1 ;
			}
		}
		else
		{
			//
			// 処理が間に合っていないので調整
			//
			this->msecFrameError = - msecWait ;		// 遅れた時間累積
			//
			uint32_t	framesNextPast =
					(msecPast * this->framesPerSec + 900) / 1000 ;
//					this->msecFrameError * this->framesPerSec / 1000 + 1 ;
			if ( framesNextPast > this->framesPast )
			{
				this->framesPast = framesNextPast ;
			}
			if ( this->framesPast > 4 )
			{
				this->framesPast = 4 ;
			}
		}
		//
		// 累積誤差をクリッピング
		//
		int32_t	msecQuadFrame = 4000 / this->framesPerSec ;
		this->msecFrameError -= (this->msecFrameError >> 3) ;
		if ( this->msecFrameError < -msecQuadFrame )
		{
			this->msecFrameError = -msecQuadFrame ;
		}
		else if ( this->msecFrameError > msecQuadFrame )
		{
			this->msecFrameError = msecQuadFrame ;
		}
	}
	this->msecRendering = msecCurrentRendering ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// 抽象レンダリングデバイス通知オブジェクト
//////////////////////////////////////////////////////////////////////////////

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice::Notify::~Notify( void )
{
	if ( m_pntfPrev || m_pntfNext )
	{
		QuickLock() ;
		DetachNotify() ;
		QuickUnlock() ;
	}
}

// チェインを後ろに挿入
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::Notify::InsertAfter( Notify * pNotify )
{
	ESLAssert( pNotify != NULL ) ;
	ESLAssert( pNotify->m_pntfPrev == NULL ) ;
	ESLAssert( pNotify->m_pntfNext == NULL ) ;
	//
	Notify *	pNext = m_pntfNext ;
	pNotify->m_pntfNext = pNext ;
	pNotify->m_pntfPrev = this ;
	m_pntfNext = pNotify ;
	//
	if ( pNext != NULL )
	{
		pNext->m_pntfPrev = pNotify ;
	}
}

// チェインを分離
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::Notify::DetachNotify( void )
{
	Notify *	pPrev = m_pntfPrev ;
	Notify *	pNext = m_pntfNext ;
	//
	if ( pNext != NULL )
	{
		pNext->m_pntfPrev = pPrev ;
	}
	if ( pPrev != NULL )
	{
		pPrev->m_pntfNext = pNext ;
	}
	m_pntfPrev = NULL ;
	m_pntfNext = NULL ;
}

// デバイスの再生成後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void S3DRenderDevice::Notify::OnResetDevice( S3DRenderDevice * pDev )
{
}



//////////////////////////////////////////////////////////////////////////////
// 抽象ウィンドウ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)
const wchar_t *	SGLAbstractWindow::Stereo3D::AnaglyphView = L"AnaglyphView" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::DDStereoscopic = L"DDStereoscopic" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::OpenGLQuadBuffer = L"OpenGLQuadBuffer" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::NVStereoBLT = L"NVStereoBLT" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::SideBySide = L"SideBySide" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::InterleavedView = L"InterleavedView" ;
const wchar_t *	SGLAbstractWindow::Stereo3D::MonoView = L"" ;
#endif

SGLAbstractWindow *	SGLAbstractWindow::m_pChainFirstWindow = NULL ;


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAbstractWindow, SObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAbstractWindow::~SGLAbstractWindow( void )
{
	DetachWindowFromChain() ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::CreateDisplay
	( const wchar_t * pszWindowName,
		Window::CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	return	sglErrFailed ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::CloseDisplay( void )
{
	return	sglErrFailed ;
}

// オプション機能フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAbstractWindow::GetOptionalFlags( void )
{
	return	0 ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLAbstractWindow::SetOptionalFlags( uint64_t nFlags )
{
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::ChangeCooperationLevel( Window::CooperationMode mode )
{
	return	sglErrFailed ;
}

// ウィンドウモード取得
//////////////////////////////////////////////////////////////////////////////
Window::CooperationMode SGLAbstractWindow::GetCooperationLevel( void )
{
	return	Window::modeWindow ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::ChangeDisplaySize
	( uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	return	sglErrFailed ;
}

// 仮想ディスプレイサイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::GetDisplaySize( SGLSize& sizeDisplay )
{
	return	sglErrFailed ;
}

// 物理モニタの解像度を変更するか？
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::EnableChangePhysicalMode( bool flagEnable )
{
	return	sglErrFailed ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::EnableZBuffer( bool flagZBuffer )
{
	return	sglErrFailed ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::SetStereoDisplayMode
	( const wchar_t * pszMethodID, uint64_t nParam )
{
	return	sglErrFailed ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLAbstractWindow::IsSupportedStereoDisplayMode( const wchar_t * pszMethodID )
{
	return	false ;
}

// 仮想ディスプレイ・ウィンドウ初期座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::InitWindowPosition
	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize )
{
	return	sglErrFailed ;
}

// 仮想ディスプレイ・ウィンドウの通常座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::GetNormalWindowPosition
	( SGLPoint& ptWindow, SGLSize * pInitExSize )
{
	return	sglErrFailed ;
}

// 仮想ディスプレイ・ウィンドウ内表示座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::GetInternalDisplayPosition
		( SGLImageRect& rctRender, SGLImageRect& rctDisplay )
{
	return	sglErrFailed ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	return	sglErrFailed ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	return	sglErrFailed ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::CloseWindow( void )
{
	return	sglErrFailed ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::SetWindowLayout
	( uint32_t nFlags, int xPos, int yPos )
{
	return	sglErrFailed ;
}

// ウィンドウサイズ（クライアントサイズ）変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::ChangeWindowSize( uint32_t nWidth, uint32_t nHeight )
{
	return	ChangeDisplaySize( nWidth, nHeight ) ;
}

// ウィンドウクライアントサイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::GetWindowClientRect( SGLImageRect& rctClient )
{
	SGLImageRect	rctDisplay ;
	return	GetInternalDisplayPosition( rctClient, rctDisplay ) ;
}

// クライアント座標→スクリーン座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLAbstractWindow::ScreenPositionFromClient( S2DDVector& vClient )
{
	return	vClient ;
}

// スクリーン座標→クライアント座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLAbstractWindow::ClientPositionFromScreen( S2DDVector& vScreen )
{
	return	vScreen ;
}

// 画面の更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::PostUpdate( const SGLImageRect* pUpdate )
{
	return	sglErrFailed ;
}

// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::UpdateWindow( Window::UpdateParameter * pUpdate )
{
	return	sglErrFailed ;
}

// ユーザー入力処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::ProcessUserInput( int64_t msecTimeout )
{
	return	sglErrFailed ;
}

// 描画スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::PostRenderingThread
			( SSystem::SProcedure * pProc, PostThreadType postType )
{
	return	sglErrFailed ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::PostUIThread( SSystem::SProcedure * pProc )
{
	return	sglErrFailed ;
}

// ウィンドウがアクティブ（最前面）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAbstractWindow::IsWindowActive( void )
{
	return	false ;
}

// ウィンドウキャプション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::SetWindowCaption( const wchar_t * pszWindowName )
{
	return	sglErrFailed ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::ShowCursor( bool fShow )
{
	return	sglErrFailed ;
}

// マウスカーソル表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLAbstractWindow::IsShowCursor( void )
{
	return	false ;
}

// マウスカーソル変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::SetCursor( const wchar_t * pszCursorID )
{
	return	sglErrFailed ;
}

// マウスカーソル座標移動
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::MoveCursorPosition
	( int32_t xPos, int32_t yPos, int idMouse )
{
	return	sglErrFailed ;
}

// マウスカーソル座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::GetCursorPosition
	( SGLPoint& ptCursor, int idMouse )
{
	return	sglErrFailed ;
}

// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
//////////////////////////////////////////////////////////////////////////////
int SGLAbstractWindow::GetMonitorFrequency( void )
{
	return	0 ;
}

// ウィンドウメニューの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAbstractWindow::AttachMenu( SGLWindowMenu * pMenu )
{
	return	sglErrFailed ;
}

// 描画ハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLPaintInterface *
	SGLAbstractWindow::SetPaintInterface( SGLPaintInterface * pPaint )
{
	SGLPaintInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pPaintHandler ;
	m_pPaintHandler = pPaint ;
	Unlock() ;
	return	pOldHandler ;
}

SGLPaintInterface *
	SGLAbstractWindow::SetDirectPaintInterface( SGLPaintInterface * pPaint )
{
	SGLPaintInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pDirectPaintHandler ;
	m_pDirectPaintHandler = pPaint ;
	Unlock() ;
	return	pOldHandler ;
}

// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLTimerInterface *
	SGLAbstractWindow::SetTimerInterface( SGLTimerInterface * pTimer )
{
	SGLTimerInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pTimerHandler ;
	m_pTimerHandler = pTimer ;
	Unlock() ;
	return	pOldHandler ;
}

// マウス入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLMouseInterface *
	SGLAbstractWindow::SetMouseInterface( SGLMouseInterface * pMouse )
{
	SGLMouseInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pMouseHandler ;
	m_pMouseHandler = pMouse ;
	Unlock() ;
	return	pOldHandler ;
}

SGLMouseInterface *
	SGLAbstractWindow::SetDirectMouseInterface( SGLMouseInterface * pMouse )
{
	SGLMouseInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pDirectMouseHandler ;
	m_pDirectMouseHandler = pMouse ;
	Unlock() ;
	return	pOldHandler ;
}

// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLKeyInterface * SGLAbstractWindow::SetKeyInterface( SGLKeyInterface * pKey )
{
	SGLKeyInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pKeyHandler ;
	m_pKeyHandler = pKey ;
	Unlock() ;
	return	pOldHandler ;
}

// 文字入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCharInputInterface *
	SGLAbstractWindow::SetCharInputInterface( SGLCharInputInterface * pChar )
{
	SGLCharInputInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pCharInputHandler ;
	m_pCharInputHandler = pChar ;
	Unlock() ;
	return	pOldHandler ;
}

// コマンド・インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCommandInterface *
	SGLAbstractWindow::SetCommandInterface( SGLCommandInterface * pCmd )
{
	SGLCommandInterface *	pOldHandler ;
	Lock() ;
	pOldHandler = m_pCommandHandler ;
	m_pCommandHandler = pCmd ;
	Unlock() ;
	return	pOldHandler ;
}

// ウィンドウスレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SError SGLAbstractWindow::Lock( int64_t msecTimeout ) const
{
	return	SSystem::Lock( msecTimeout ) ;
}

SError SGLAbstractWindow::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout ) const
{
	return	SSystem::LockTrace( pszSource, nLineNum, msecTimeout ) ;
}

SError SGLAbstractWindow::Unlock( void ) const
{
	return	SSystem::Unlock() ;
}

atomic_int_t SGLAbstractWindow::UnlockAll( void ) const
{
	return	SSystem::UnlockAll() ;
}

SError SGLAbstractWindow::Relock( atomic_int_t nLock ) const
{
	return	SSystem::Relock( nLock ) ;
}

atomic_int_t SGLAbstractWindow::TestLocked( void ) const
{
	return	SSystem::TestLocked() ;
}

// プラットフォーム固有オブジェクト
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
SGLAbstractWindow::operator Window* ( void ) const
{
	return	GetWindowObject() ;
}
#endif

// 互換のためのキャスト
//////////////////////////////////////////////////////////////////////////////
SGLAbstractWindow::operator SGLAbstractWindow* ( void ) const
{
	return	(SGLAbstractWindow*) this ;
}

// ウィンドウ・オブジェクト・チェーン
//////////////////////////////////////////////////////////////////////////////
void SGLAbstractWindow::AddWindowToChain( void )
{
	QuickLock() ;
	if ( m_pChainNextWindow == NULL )
	{
		SGLAbstractWindow *	pLast = NULL ;
		SGLAbstractWindow *	pNext = m_pChainFirstWindow ;
		while ( pNext != NULL )
		{
			if ( pNext == this )
			{
				QuickUnlock() ;
				return ;
			}
			pLast = pNext ;
			pNext = pNext->m_pChainNextWindow ;
		}
		if ( pLast != NULL )
		{
			pLast->m_pChainNextWindow = this ;
		}
		else
		{
			m_pChainFirstWindow = this ;
		}
	}
	QuickUnlock() ;
}

void SGLAbstractWindow::DetachWindowFromChain( void )
{
	QuickLock() ;
	SGLAbstractWindow *	pLast = NULL ;
	SGLAbstractWindow *	pNext = m_pChainFirstWindow ;
	while ( pNext != NULL )
	{
		if ( pNext == this )
		{
			pNext = m_pChainNextWindow ;
			m_pChainNextWindow = NULL ;
			//
			if ( pLast != NULL )
			{
				pLast->m_pChainNextWindow = pNext ;
			}
			else
			{
				m_pChainFirstWindow = pNext ;
			}
		}
		else
		{
			pLast = pNext ;
			pNext = pNext->m_pChainNextWindow ;
		}
	}
	QuickUnlock() ;
	ESLAssert( m_pChainNextWindow == NULL ) ;
}

#if	defined(__PLATFORM_WINDOWS__)
SGLAbstractWindow * SGLAbstractWindow::FromHandle( HWND hWnd )
{
	QuickLock() ;
	SGLAbstractWindow *	pWnd = m_pChainFirstWindow ;
	while ( pWnd != NULL )
	{
		if ( pWnd->GetWindowHandle() == hWnd )
		{
			break ;
		}
		pWnd = pWnd->m_pChainNextWindow ;
	}
	QuickUnlock() ;
	return	pWnd ;
}
#endif

// 次のウィンドウ列挙
//////////////////////////////////////////////////////////////////////////////
SGLAbstractWindow * SGLAbstractWindow::EnumerateNextWindow( void ) const
{
	SGLAbstractWindow *	pWnd ;
	QuickLock() ;
	pWnd = m_pChainNextWindow ;
	QuickUnlock() ;
	return	pWnd ;
}



//////////////////////////////////////////////////////////////////////////////
// ウィンドウ状態モニタ・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowMonitorInterface, ESLObject ) ;
