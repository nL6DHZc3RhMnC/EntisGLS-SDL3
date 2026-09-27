
#if	!defined(__SAKURAGL_SGL_GLS3_WINDOW_IMPLEMENT_H__)
#define	__SAKURAGL_SGL_GLS3_WINDOW_IMPLEMENT_H__	1

#include <vfw.h>
#include <egl.h>
#include <glsmidimusic.h>
#include <glssound.h>
#include <glswndbase.h>
#include <glssurfdesc.h>
#include <glsscript.h>
#include <glsctpsprite.h>
#include <sakuragl/sgl3d/sgl_render_parameter_context.h>
#include <sakuragl/sgl_gls3_window_producer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 標準レンダリング実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLStandardRenderContext : public SGLRenderPolygonInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLStandardRenderContext, SGLRenderPolygonInterface )
		// 構築関数
		SGLStandardRenderContext( void ) ;
		// 消滅関数
		virtual ~SGLStandardRenderContext( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ実装
	//////////////////////////////////////////////////////////////////////////

#if	defined(USE_ENTIS_GLS3_WINDOW)
	class	SGLWindow	: public SGLAbstractWindow
	{
	protected:
		// ECSWindow 派生
		class	SGLCSWindow : public ECSWindow
		{
		public:
			SGLWindow *	m_pWnd ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLCSWindow, ECSWindow )
			// ウィンドウインターフェースオブジェクトを作成する
			virtual ECSWindow::EInterface * OnCreateInterface( void ) ;
		protected:
			// 描画処理
			virtual void AfterRefreshRectWith3DView
				( HEGL_RENDER_POLYGON hRender, const VIEW3D_INFO * pv3dInfo ) ;
		} ;
		// ウィンドウインターフェース
		class	SGLInterface	: public ECSWindow::EInterface
		{
		protected:
			SSystem::SString	m_strCursorID ;
			BYTE				m_bytLeadChar ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLInterface, EInterface )
			// 構築関数
			SGLInterface( void ) ;
			// 消滅関数
			virtual ~SGLInterface( void ) ;
			// ウィンドウプロシージャ
			virtual LRESULT WindowProc
				( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
			// マウスカーソルを設定する
			virtual bool OnSetCursor( int xPos, int yPos ) ;
			// マウスカーソルを変更
			void ChangeMouseCursor( const wchar_t * pwszCursorID ) ;
			// キーコンテキストフラグを取得する
			static int64_t GetKeyContextFlag( void ) ;
		} ;
		// ウィンドウ描画インターフェース
		class	SGLRenderPolygon	: public SGLRenderPolygonInterface
		{
		public:
			ESpriteInterface *	m_pSprite ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO
				( SGLRenderPolygon, SGLRenderPolygonInterface )
			// 構築関数
			SGLRenderPolygon( void )
				: SGLRenderPolygonInterface( NULL ), m_pSprite( NULL ) {}
		public:
			// 描画デフォルトフラグ
			virtual void SetPaintFlags( int64_t nFlags ) ;
			// シェーディング設定
			virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
			// 投影スクリーン座標設定
			virtual SGLError SetProjectionScreen
				( const S3DVector& vScreen,
					double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
			// 立体視視差設定
			virtual void SetParallax( double xParallax, double zFocusRate ) ;
		public:
			// ステレオ立体視パラメータ取得
			void GetView3DInfo( ESprite::VIEW3D_INFO& v3dInfo ) ;
		} ;

	protected:
		SGLCSWindow			m_cswnd ;			// ECSWindow
		ECSContext			m_contextDummy ;	// ダミー
		uint64_t			m_flagOptions ;
		SGLRenderPolygon	m_render ;			// 描画インターフェース
		HEGL_RENDER_POLYGON	m_hRender ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindow, SGLAbstractWindow )
		// 構築関数
		SGLWindow( void ) ;
		// 消滅関数
		virtual ~SGLWindow( void ) ;

	protected:
		// アイコン取得
		static HICON LoadMainIcon( void ) ;

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
		// ウィンドウモード取得
		virtual Window::CooperationMode GetCooperationLevel( void ) ;
		// 仮想ディスプレイサイズ変更
		virtual SGLError ChangeDisplaySize
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイサイズ取得
		virtual SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
		// 物理モニタの解像度を変更するか？
		virtual SGLError EnableChangePhysicalMode( bool fEnable ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer( bool flagZBuffer ) ;
		SGLError CreateZBuffer( void ) ;
		SGLError DeleteZBuffer( void ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		SGLError CreateStereoBuffer( void ) ;
		SGLError DeleteStereoBuffer( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode( const wchar_t * pszMethodID ) ;
		// 仮想ディスプレイ・ウィンドウ初期座標設定
		virtual SGLError InitWindowPosition
			( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウの通常座標取得
		virtual SGLError GetNormalWindowPosition
			( SGLPoint& ptWindow, SGLSize * pExSize = NULL ) ;
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
		virtual SGLError PostRenderingThread
			( SSystem::SProcedure * pProc, PostThreadType postType ) ;
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

	public:
		// マウスイベントキャプチャー
		virtual SGLError CaptureMouse( int idMouse = 0 ) ;
		virtual SGLError ReleaseMouse( int idMouse = 0 ) ;

	protected:
		// ウィンドウスレッド実行関数
		static LRESULT __stdcall WindowThreadCallerProc( void * pInstance ) ;

	public:
		// 描画インターフェース取得
		virtual S3DRenderContextInterface * GetRenderContext
				( S3DRenderContextInterface::StereoViewIndex sviView
								= S3DRenderContextInterface::stereoViewAuto ) ;
		virtual void ReleaseRenderContext
						( S3DRenderContextInterface * context ) ;

	public:
		// プラットフォーム固有オブジェクト
		virtual HWND GetWindowHandle( void ) const ;
		ECSWindow * GetGameWindow( void )
		{
			return	&m_cswnd ;
		}


		friend class SGLCSWindow ;
		friend class SGLInterface ;
	} ;

#else
	class	SGLWindow	: public SGLGenericWindowGLS3View
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindow, SGLGenericWindowGLS3View )
		// 構築関数
		SGLWindow( void ) : SGLGenericWindowGLS3View( NULL ) { }
	} ;

#endif

}

#endif

