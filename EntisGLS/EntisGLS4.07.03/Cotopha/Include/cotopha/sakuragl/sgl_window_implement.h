
#if	!defined(__SAKURAGL_SGL_WINDOW_IMPLEMENT_H__)
#define	__SAKURAGL_SGL_WINDOW_IMPLEMENT_H__	1

#include <sakuragl/window/sgl_window_menu.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 標準レンダリング実装
	//////////////////////////////////////////////////////////////////////////

	typedef	S3DRenderContext	SGLStandardRenderContext ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ・ラッパ
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindow	: public SGLAbstractWindow
	{
	protected:
		Window *			m_pWindow ;
		S3DRenderContext	m_render ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindow, SGLAbstractWindow )
		// 構築関数
		SGLWindow( void ) ;
		// 消滅関数
		virtual ~SGLWindow( void ) ;

	public:	// 仮想ディスプレイメソッド・オーバーライド
		// 仮想ディスプレイ開始
		virtual SGLError CreateDisplay
			( const wchar_t * pszWindowName,
				Window::CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイ終了
		virtual SGLError CloseDisplay( void ) ;
		// オプション機能フラグ取得
		virtual uint64_t GetOptionalFlags( void ) ;
		// オプション機能フラグ設定
		virtual void SetOptionalFlags( uint64_t nFlags ) ;
		// ウィンドウモード変更
		virtual SGLError ChangeCooperationLevel( Window::CooperationMode mode ) ;
		// 仮想ディスプレイサイズ変更
		virtual SGLError ChangeDisplaySize
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイサイズ取得
		virtual SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
		// 物理モニタの解像度を変更するか？
		virtual SGLError EnableChangePhysicalMode( bool flagEnable ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer( bool flagZBuffer ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode( const wchar_t * pszMethodID ) ;
		// 仮想ディスプレイ・ウィンドウ初期座標設定
		virtual SGLError InitWindowPosition
			( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウの通常座標取得
		virtual SGLError GetNormalWindowPosition
			( SGLPoint& ptWindow, SGLSize * pWindowSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウ内表示座標取得
		virtual SGLError GetInternalDisplayPosition
				( SGLImageRect& rctRender, SGLImageRect& rctDisplay ) ;
		// 仮想ディスプレイ・有効画面外枠表示設定
		virtual SGLError SetExteriorBackgroundFrame
			( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
				SGLImageObject* pLeft = NULL, SGLImageObject* pRight = NULL,
				SGLImageObject* pUpper = NULL, SGLImageObject* pUnder = NULL ) ;

	public:	// 汎用ウィンドウ専用・オーバーライド
		// ウィンドウ生成
		virtual SGLError CreateWindow
			( const wchar_t * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags = 0, SGLAbstractWindow * pParentWnd = NULL ) ;
		// ウィンドウを閉じる
		virtual SGLError CloseWindow( void ) ;
		// ウィンドウ位置を設定する
		virtual SGLError SetWindowLayout
			( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;
		// クライアント座標→スクリーン座標変換
		virtual S2DDVector& ScreenPositionFromClient( S2DDVector& vClient ) ;
		// スクリーン座標→クライアント座標変換
		virtual S2DDVector& ClientPositionFromScreen( S2DDVector& vScreen ) ;

	public:	// 仮想ディスプレイ・汎用ウィンドウ共通・オーバーライド
		// 画面の更新通知
		virtual SGLError PostUpdate( const SGLImageRect* pUpdate = NULL ) ;
		// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
		virtual SGLError UpdateWindow( Window::UpdateParameter * pUpdate = NULL ) ;
		// ユーザー入力処理
		virtual SGLError ProcessUserInput( int64_t msecTimeout = 1 ) ;
		// 描画スレッドで実行
		virtual SGLError PostRenderingThread( SSystem::SProcedure * pProc ) ;
		// UI スレッドで実行
		virtual SGLError PostUIThread( SSystem::SProcedure * pProc ) ;
		// ウィンドウがアクティブ（最前面）か？
		virtual bool IsWindowActive( void ) ;
		// ウィンドウキャプション設定
		virtual SGLError SetWindowCaption( const wchar_t * pszWindowName ) ;
		// マウスカーソル表示
		virtual SGLError ShowCursor( bool fShow ) ;
		// マウスカーソル表示状態取得
		virtual bool IsShowCursor( void ) ;
		// マウスカーソル変更
		virtual SGLError SetCursor( const wchar_t * pszCursorID ) ;
		// マウスカーソル座標移動
		virtual SGLError MoveCursorPosition
			( int32_t xPos, int32_t yPos, int idMouse = 0 ) ;
		// マウスカーソル座標取得
		virtual SGLError GetCursorPosition
			( SGLPoint& ptCursor, int idMouse = 0 ) ;
		// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
		virtual int GetMonitorFrequency( void ) ;
		// ウィンドウメニューの設定
		virtual SGLError AttachMenu( SGLWindowMenu * pMenu ) ;

	public:
		// 描画ハンドラ
		virtual SGLPaintInterface *
					SetPaintInterface( SGLPaintInterface * pPaint ) ;
		virtual SGLPaintInterface *
					SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
		// タイマーハンドラ
		virtual SGLTimerInterface *
					SetTimerInterface( SGLTimerInterface * pTimer ) ;
		// マウス入力インターフェース
		virtual SGLMouseInterface *
					SetMouseInterface( SGLMouseInterface * pMouse ) ;
		virtual SGLMouseInterface *
					SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
		// マウスイベントキャプチャー
		virtual SGLError CaptureMouse( int idMouse = 0 ) ;
		virtual SGLError ReleaseMouse( int idMouse = 0 ) ;
		// キー入力インターフェース
		virtual SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
		// 文字入力インターフェース
		virtual SGLCharInputInterface *
					SetCharInputInterface( SGLCharInputInterface * pChar ) ;
		// コマンド・インターフェース
		virtual SGLCommandInterface *
					SetCommandInterface( SGLCommandInterface * pCmd ) ;

	public:
		// 描画インターフェース取得
		virtual S3DRenderContextInterface * GetRenderContext
			( S3DRenderContextInterface::StereoViewIndex sviView
						= S3DRenderContextInterface::stereoViewAuto ) ;
		virtual void ReleaseRenderContext
						( S3DRenderContextInterface * context ) ;

	public:
		// Window オブジェクト取得
		virtual Window * GetWindowObject( void ) const ;

	} ;

}

#endif

