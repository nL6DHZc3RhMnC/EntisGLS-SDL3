
#if	!defined(__SAKURAGL_GENERIC_WINDOW_H__)
#define	__SAKURAGL_GENERIC_WINDOW_H__	1

#include <imm.h>
#include <sakura/ssys_reference_array.h>
#include <sakuragl/window/sgl_window_producer.h>
#include <sakuragl/sgl_win_display_mode.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// SGLImageBufferInterface の DIB 実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageWin32DIBitmap	: public SGLImageBufferInterface
	{
	public:
		bool			m_flagUpdateFull ;
		bool			m_flagUpdateRect ;
		SGLImageRect	m_rectUpdate ;
		HDC				m_hDC ;
		HBITMAP			m_hBitmap ;
		HBITMAP			m_hDefBitmap ;
		BITMAPINFO		m_bmi ;
		uint8_t *		m_pPixels ;
		SGLImageBuffer	m_imgbuf ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageWin32DIBitmap, SGLImageBufferInterface )
		// 構築関数
		SGLImageWin32DIBitmap( void ) ;
		// 消滅関数
		virtual ~SGLImageWin32DIBitmap( void ) ;
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;

	public:
		// SGLImageObject から SGLImageWin32DIBiSGLWindowViewProducertmap 取得
		static SGLImageWin32DIBitmap * CommitDIB( SGLImageObject * pImage ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	class	SGLGenericWindow	: public SGLAbstractWindow
	{
	protected:
		// 環境
		SSystem::SEnvironmentInterface *	m_pEnv ;

		// 表示インターフェース
		SGLWindowViewFramework		m_wvfFramework ;
		bool						m_flagProducerFullscreen ;
		SSystem::SThread::IdType	m_tidAttachedViewThread ;

		SSystem::SCriticalSection	m_csViewSync ;
		SGLWindowViewSynchronizer *	m_pViewSync ;
		int							m_nRequestFPS ;

		// ウィンドウハンドル
		HWND					m_hWnd ;
		HIMC					m_hIMC ;
		WNDPROC					m_wpSuperClass ;
		DWORD_PTR				m_dwResultTemp ;

		// 動作モード
		bool					m_flagCreated ;
		bool					m_flagAttached ;
		bool					m_flagModeDisplay ;
		bool					m_flagFullscreen ;
		bool					m_flagLayeredWindow ;
		bool					m_flagChangePhysicalMode ;
		bool					m_flagRestoreFullscreen ;
		SSystem::SString		m_strCaption ;
		Window::CooperationMode	m_modeCooperation ;
		uint64_t				m_flagsOption ;
		SGLSize					m_sizeVirtual ;
		SGLSize					m_sizePhysical ;
		uint32_t				m_nBitsPerPixel ;
		uint32_t				m_nFrequency ;
		uint32_t				m_flagsLayout ;
		SGLPoint				m_ptLayoutOffset ;
		SGLSize					m_sizeLayoutOriginal ;

		// ウィンドウ座標
		bool					m_flagInitialPos ;
		bool					m_flagInitialSize ;
		SGLPoint				m_ptInitialPos ;
		SGLSize					m_sizeInitialSize ;
		SGLRect					m_rctNormalWndPos ;		// フルスクリーン時に通常時の座標を保存

		// レイヤードウィンドウ用フレームバッファ
		SSystem::SSmartPointer<S3DRenderContext>
								m_pLayeredRenderer ;
		SGLSmartImage			m_imgFrameColor ;
		SGLSmartImage			m_imgFrameDepth ;

		bool					m_flagStereoView ;
		SGLSmartImage			m_imgFrameRight ;

		// マウスカーソル
		bool					m_flagShowCursor ;
		HCURSOR					m_hCursor ;
		SSystem::SString		m_strCursorID ;

		// メニュー
		SSystem::SSmartReference<SGLWindowMenu>	m_refMenu ;
		HMENU					m_hMenu ;

		// 関連ウィンドウ
		SSystem::SSmartReference<SGLAbstractWindow>	m_refParentWnd ;
		SSystem::SReferenceArray<SGLGenericWindow>	m_arrChildren ;

		// ウィンドウ UI スレッド
		bool					m_flagWMPaintEntered ;
		bool					m_flagWMDestroying ;
		volatile bool			m_flagQuitMessageLoop ;
		SSystem::SThread		m_threadUI ;
		SSystem::SSignalEvent	m_signalCreated ;
		SSystem::SSignalEvent	m_signalQuit ;
		SGLError				m_errCreationResult ;

		// 非同期スレッド
		SSystem::SProcedureQueue	m_queAsyncThread ;
		SSystem::SProcedureQueue	m_queOnRenderThread ;

		// 描画抑制
		atomic_int_t			m_nFreezePaint ;

		// ウィンドウスレッド排他処理用
		SSystem::SMutex *		m_pMutexWindowUI ;

		// WM_TIMER タイミング計測
		SSystem::STimeCounter	m_timerWMTimer ;
		double					m_msecLastTimerInterval ;

		// サブクラス化の際の WM_PAINT タイミング計測用
		SSystem::STimeCounter	m_timerLastPaint ;

		// FPS 計測用
		bool					m_flagFPSonCaption ;
		bool					m_flagTracePerformance ;
		SSystem::STimeCounter	m_timerLastPeriod ;
		size_t					m_nCountRenderedFrames ;
		size_t					m_nLastFPS ;

		size_t					m_nCountOnTimer ;
		double					m_msecSumOnTimer ;
		double					m_msecMaxOnTimer ;

	public:
		// パフォーマンス・ログ
		struct	PerformanceLogInfo : public S3DRenderDevice::PerformanceLogInfo
		{
			double				msecAvgOnTimer ;
			double				msecMaxOnTimer ;
		} ;
		// パフォーマンス・ログ・リスナ
		class	PerformanceLogListener
		{
		public:
			virtual void OnPerformanceLog
				( SGLGenericWindow * pWindow,
					PerformanceLogInfo& logInfo ) = 0 ;
		} ;
	protected:
		PerformanceLogListener *	m_pLogListener ;

		// UI スレッド
		class	UIThreadProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLGenericWindow *		m_pWnd ;
			bool					m_fModeDisplay ;
			SSystem::SString		m_strCaption ;
			Window::CooperationMode	m_modeCooperation ;
			uint32_t				m_nWidth ;
			uint32_t				m_nHeight ;
			uint32_t				m_nBitsPerPixel ;
			uint32_t				m_nFrequency ;
			uint32_t				m_nFlags ;
			SGLAbstractWindow *		m_pParentWnd ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIThreadProcedure, SProcedure )
			// 構築関数
			UIThreadProcedure
				( SGLGenericWindow * pWnd,
					bool fModeDisplay, const wchar_t * pwszCaption,
					Window::CooperationMode modeCooperation,
					uint32_t nWidth, uint32_t nHeight,
					uint32_t nBitsPerPixel, uint32_t nFrequency,
					uint32_t nFlags, SGLAbstractWindow * pParentWnd ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;
		SSystem::SSmartPointer<UIThreadProcedure>	m_procUIThread ;

		// 汎用関数呼び出し
		typedef	SGLError (SGLGenericWindow::*PTR_METHOD)( void * ptrParam ) ;
		class	CallMethodOnUIThreadProcedure	: public SSystem::SProcedure
		{
		protected:
			bool						m_flagAutoDelete ;
			SGLGenericWindow *			m_pWnd ;
			PTR_METHOD					m_pfnMethod ;
			void *						m_ptrParam ;
			SSystem::SCriticalSection	m_csSync ;
			SSystem::SSignalEvent		m_signalDone ;
			SGLError					m_errResult ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CallMethodOnUIThreadProcedure, SProcedure )
			// 構築関数
			CallMethodOnUIThreadProcedure
				( SGLGenericWindow * pWnd, PTR_METHOD pfnMethod,
					void * ptrParam = NULL, bool flagAutoDelete = false ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了処理
			virtual void Finalize( void ) ;
			// 関数の終了を待つ
			SGLError WaitDone
				( int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
			// 関数の終了コード取得
			SGLError GetMethodResult( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGenericWindow, SGLAbstractWindow )
		// 構築関数
		SGLGenericWindow
			( SGLWindowViewProducer * pwvp,
				SSystem::SEnvironmentInterface * env ) ;
		// 消滅関数
		virtual ~SGLGenericWindow( void ) ;
		// 表示インターフェース取得
		SGLWindowViewProducer * GetWindowViewProducer( void ) const ;
		// 表示インターフェース変更
		SGLWindowViewProducer *
			ChangeWindowViewProducer( SGLWindowViewProducer * pwvp ) ;
		// セカンダリビュー追加
		SGLError AttachSecondaryView
				( SGLSecondaryViewProducer * psvp, bool fVSync ) ;
		// セカンダリビュー削除
		SGLError DetachSecondaryView( SGLSecondaryViewProducer * psvp ) ;
		// VSync ビュー設定
		SGLError SetVSyncSecondaryView
				( SGLSecondaryViewProducer * psvp, bool fVSync ) ;
		// 描画タイミングインターフェース設定
		SGLWindowViewSynchronizer *
				SetViewSynchronizer( SGLWindowViewSynchronizer * pViewSync ) ;
		// 設定されている描画タイミングインターフェース取得
		SGLWindowViewSynchronizer * GetViewSynchronizer( void ) const
		{
			return	m_pViewSync ;
		}
		// 描画タイミング（FPS）設定（SGLWindowViewSynchronizer未設定時動作）
		void SetViewFramePerSecond( int nReqFPS ) ;
		// 描画タイミング（FPS）取得
		int GetViewFramePerSecond( void ) const
		{
			return	m_nRequestFPS ;
		}

	public:	// 仮想ディスプレイメソッド
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

	public:	// 汎用ウィンドウ専用
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

	public:	// 仮想ディスプレイ・汎用ウィンドウ共通
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
		// ウィンドウメニューの設定
		virtual SGLError AttachMenu( SGLWindowMenu * pMenu ) ;
		// マウスイベントキャプチャー
		virtual SGLError CaptureMouse( int idMouse = 0 ) ;
		virtual SGLError ReleaseMouse( int idMouse = 0 ) ;

	public:
		// 描画インターフェース取得
		virtual S3DRenderContextInterface * GetRenderContext
			( S3DRenderContextInterface::StereoViewIndex sviView
						= S3DRenderContextInterface::stereoViewAuto ) ;
		virtual void ReleaseRenderContext
						( S3DRenderContextInterface* context ) ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;

	public:
		// ウィンドウスレッド排他処理用
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError LockTrace
			( const char * pszSource,
				size_t nLineNum,
				int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t UnlockAll( void ) const ;
		virtual SSystem::SError Relock( atomic_int_t nLock ) const ;
		virtual atomic_int_t TestLocked( void ) const ;
		// ウィンドウスレッド排他処理用ミューテックス変更
		void SetWindowUIThreadMutex( SSystem::SMutex * pMutex ) ;

	public:
		// プラットフォーム固有オブジェクト
		virtual HWND GetWindowHandle( void ) const ;
		// 既存の Window をサブクラス化する
		SGLError CreateSubclassWindow
			( HWND hWnd, const SGLSize * pVirtualDisplay = NULL ) ;
		// マウスカーソルをロードする
		static HCURSOR LoadWindowsCursor( const wchar_t * pszCursorID ) ;
		// メッセージループ（ウィンドウスレッド上から呼び出し）
		void DoMessageLoop( void ) ;
		// メッセージループ
		void QuitMessageLoop( void ) ;

		// 既存のウィンドウのハンドルを関連付ける
		void AttachWindowHandle( HWND hWnd ) ;
		// 既存のウィンドウのハンドルを分離する
		void DetachWindowHandle( void ) ;

		// ウィンドウ描画抑制
		void AddFreezePaint( void ) ;
		void ReleaseFreezePaint( void ) ;

	protected:	// ウィンドウ・レイアウト
		// ウィンドウクラス登録
		const char * RegisterWindowClass( void ) ;
		// ウィンドウ作成（低水準）
		SGLError CreateWindowSimply
			( const wchar_t * pwszWindowName,
				uint32_t nWidth, uint32_t nHeight, HWND hwndParent ) ;
		// フルスクリーン化
		SGLError ChangeWindowToFullscreen( void ) ;
		// フルスクリーン解除
		SGLError RestoreWindowFromFullscreen( void ) ;
		// オプション機能フラグの反映
		void UpdateOptionalFlags( bool fFullscreen ) ;
		// ウィンドウのフレームサイズを計算
		void GetWindowFrameMargin( SGLRect& rectMargin ) ;
		// ウィンドウのクライアントサイズを論理サイズにフィットさせる
		void FitWindowClientSize( void ) ;
		// ウィンドウのクライアント表示座標を更新する
		void UpdateClientDisplayPosition( void ) ;
		// ウィンドウのレイアウトに基づいて位置を調整する
		void UpdateWindowLayout( void ) ;
		// レイヤードウィンドウ用フレームバッファサイズをウィンドウサイズに調整する
		void ResizeFramebufferForLayeredWindow( void ) ;
		// ウィンドウ更新
		void UpdateWindowTimeout( DWORD dwTimeout ) ;
		// レイヤードウィンドウを更新する
		void UpdateLayeredWindow( void ) ;
		// アイコン読み込み
		HICON LoadMainIcon( void ) ;
		// ウィンドウスタイル
		LONG ModifyWindowStyleOf( LONG lStyle, bool fFullscreen ) const;
		// 拡張ウィンドウスタイル
		LONG ModifyWindowExStyleOf( LONG lExStyle, bool fFullscreen ) const ;
		// クラススタイル
		LONG ModifyWindowClassStyleOf( LONG lClassStyle, bool fFullscreen ) const ;
		// 現在のウィンドウのスタイルからオプションフラグへ反映
		void ReflectOptionFlagsFromCurrentStyle( void ) ;
		// ウィンドウキャプション書式
		SSystem::SString FormatWindowCaption( void ) const ;

	public:
		// 描画処理
		void DrawWindow( bool fOnWinThread ) ;
		// 表示反映処理
		void FlipView( bool fVSync, bool fOnWinThread ) ;
		// 最近の FPS 取得
		size_t GetRecentlyFramePerSecond( void ) const ;
		// FPS をキャプションに表示
		void EnableCaptionWithFPS( bool flagEnable, bool flagTracePerfom = true ) ;
		// パフォーマンス・リスナ設定
		void AttachPerformanceLogListener( PerformanceLogListener * pListener ) ;

	protected:	// UI スレッド
		// UI スレッド判定
		bool IsOnUIThread( void ) ;
		// UI スレッド上で関数呼び出し（同期実行）
		SGLError CallMethodOnUIThread
			( PTR_METHOD pfnMethod, void * ptrParam = NULL ) ;

	protected:	// UI スレッドから呼び出される
		// 仮想ディスプレイ開始
		virtual SGLError OnCreateDisplay
			( const wchar_t * pszWindowName,
				Window::CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel, uint32_t nFrequency ) ;
		// ウィンドウ生成
		virtual SGLError OnCreateWindow
			( const wchar_t * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags, SGLAbstractWindow * pParentWnd ) ;
		// メッセージループ
		virtual void OnLoop( void ) ;
		bool OnLoopDefault( void ) ;
		bool OnLoopViewSync( void ) ;
		// ウィンドウ破棄関数
		SGLError OnDestroyWindow( void * ptrParam ) ;
		// オプション機能フラグ設定
		SGLError OnSetOptionalFlags( void * ptrParam ) ;
		// ウィンドウモード変更
		SGLError OnChangeCooperationLevel( void * ptrParam ) ;
		// 仮想ディスプレイサイズ変更
		struct	METHOD_PARAM_DISPLAY_SIZE
		{
			uint32_t	nWidth, nHeight ;
			uint32_t	nBitsPerPixel, nFrequency ;
		} ;
		SGLError OnChangeDisplaySize( void * ptrParam ) ;
		// ウィンドウ位置を更新する
		SGLError OnUpdateWindowLayout( void * ptrParam ) ;
		// ユーザー入力処理
		SGLError OnProcessUserInput( void * ptrParam ) ;
		// メニューを更新する
		SGLError OnUpdateMenu( void * ptrParam ) ;
		// ステレオ立体視モード設定
		class	METHOD_PARAM_STEREO_DISPLAY_MODE
		{
		public:
			SSystem::SString	m_strMethodID ;
			uint64_t			m_nParam ;
		} ;
		SGLError OnSetStereoDisplayMode( void * ptrParam ) ;

	protected:	// Windows 固有定義
		// ウィンドウメッセージ
		/*
		enum	WindowMessage
		{
			wmCallUIProcedure	= WM_USER + 1,
			wmCallRenderProcedure,
		} ;
		*/
		UINT	wmCallUIProcedure ;
		UINT	wmCallRenderProcedure ;

		// 非標準(Windows95非互換)ウィンドウインターフェース
		typedef	BOOL (WINAPI *API_TrackMouseEvent)
					( LPTRACKMOUSEEVENT lpEventTrack ) ;
		typedef	BOOL (WINAPI *API_UpdateLayeredWindow)
			( HWND hwnd, HDC hdcDst, POINT * pptDst, SIZE * psize,
				HDC hdcSrc, POINT * pptSrc, COLORREF crKey,
						BLENDFUNCTION * pblend, DWORD dwFlags ) ;

		#if	!defined(WS_EX_LAYERED)
		enum
		{
			WS_EX_LAYERED	= 0x00080000,
			LWA_ALPHA		= 2,
			ULW_ALPHA		= 2,
		} ;
		#endif

		#if !defined(AC_SRC_OVER)
		struct BLENDFUNCTION
		{
			BYTE	BlendOp ;
			BYTE	BlendFlags ;
			BYTE	SourceConstantAlpha ;
			BYTE	AlphaFormat ;
		} ;
		enum
		{
			AC_SRC_OVER		= 0,
			AC_SRC_ALPHA	= 1,
		} ;
		#endif

		#if	!defined(WM_TOUCH)
		enum
		{
			WM_TOUCH						= 0x0240,
			TWF_FINETOUCH					= 0x00000001,
			TWF_WANTPALM					= 0x00000002,
			TOUCHEVENTF_MOVE				= 0x0001,	// TOUCHINPUT.dwFlags
			TOUCHEVENTF_DOWN				= 0x0002,
			TOUCHEVENTF_UP					= 0x0004,
			TOUCHEVENTF_INRANGE				= 0x0008,
			TOUCHEVENTF_PRIMARY				= 0x0010,
			TOUCHEVENTF_NOCOALESCE			= 0x0020,
			TOUCHEVENTF_PEN					= 0x0040,
			TOUCHEVENTF_PALM				= 0x0080,
			TOUCHINPUTMASKF_TIMEFROMSYSTEM	= 0x0001,	// TOUCHINPUT.dwMask
			TOUCHINPUTMASKF_EXTRAINFO		= 0x0002,
			TOUCHINPUTMASKF_CONTACTAREA		= 0x0004,
		} ;
		DECLARE_HANDLE(HTOUCHINPUT);
		#endif
		enum
		{
			MOUSEEVENTF_FROMTOUCH			= 0xFF515700,
		} ;
		struct	TOUCHINPUT
		{
			LONG		x ;
			LONG		y ;
			HANDLE		hSource ;
			DWORD		dwID ;
			DWORD		dwFlags ;
			DWORD		dwMask ;
			DWORD		dwTime ;
			ULONG_PTR	dwExtraInfo ;
			DWORD		cxContact ;
			DWORD		cyContact ;
		} ;

		typedef BOOL (WINAPI *API_GetTouchInputInfo)
						( HTOUCHINPUT hTouchInput, UINT cInputs,
								TOUCHINPUT * pInputs, int cbSize ) ;
		typedef BOOL (WINAPI *API_CloseTouchInputHandle)( HTOUCHINPUT hTouchInput ) ;
		typedef BOOL (WINAPI *API_RegisterTouchWindow)( HWND hwnd, ULONG ulFlags ) ;
		typedef BOOL (WINAPI *API_UnregisterTouchWindow)( HWND hwnd ) ;

		SGLDisplayMode			m_displayMode ;

		SSystem::SString		m_strClassName ;
		SSystem::SArray<char>	m_cstrClassName ;
		bool					m_flagWndClassOwner ;

		HMODULE					m_hUser32 ;					// USER32.DLL モジュール
		API_TrackMouseEvent		m_apiTrackMouseEvent ;		// TrackMouseEvent 関数
		API_UpdateLayeredWindow	m_apiUpdateLayeredWindow ;	// UpdateLayeredWindow 関数
		API_GetTouchInputInfo	m_apiGetTouchInputInfo ;	// GetTouchInputInfo 関数
		API_CloseTouchInputHandle
								m_apiCloseTouchInputHandle ;// CloseTouchInputHandle 関数
		API_RegisterTouchWindow	m_apiRegisterTouchWindow ;	// RegisterTouchWindow 関数
		API_UnregisterTouchWindow
								m_apiUnregisterTouchWindow ;// UnregisterTouchWindow 関数
		BLENDFUNCTION			m_bfLayeredWindow ;

		bool					m_flagMouseLeaved ;			// マウスがウィンドウの外にあるか？
		bool					m_flagActivated ;			// ウィンドウが最前面
		SSystem::SArray<DWORD>	m_aTouchIDs ;				// タッチID配列
		BYTE					m_bytLeadChar ;
		DWORD					m_dwLastTimer ;

		bool					m_flagFirstWindowSize ;

	public:
		static const char *	SGL_GENERIC_WINDOW_CLASS ;

	protected:	// メッセージ処理
		// メッセージ事前変換関数
		virtual bool PreTranslateMessage( MSG & msg ) ;
		// ウィンドウ・プロシージャ
		virtual LRESULT WindowProc
			( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		static LRESULT __stdcall WindowCallbackProc
			( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		// キーコンテキストフラグを取得する
		static int64_t GetKeyContextFlag( void ) ;
		// 論理座標と物理（クライアント）座標変換
		void ClientPointToVirtual( SGLPoint& pos ) ;
		void VirtualPointToClient( SGLPoint& pos ) ;
		void VirtualSizeToClient( SGLSize& size ) ;
		// Aero 判定
		bool AeroIsCompositionEnabled( void ) ;
		// WM_MOUSEFIRST～WM_MOUSELAST
		virtual bool OnWMMouseEvent
			( SGLMouseInterface * pMouse,
				UINT uMsg, WPARAM wParam, int32_t xPos, int32_t yPos ) ;
		// WM_MOUSELEAVE
		virtual void OnWMMouseLeave( void ) ;
		// WM_TOUCH
		virtual void OnWMTouchInput( const TOUCHINPUT& ti ) ;
		// WM_PAINT
		virtual void OnWMPaint( void ) ;
		// WM_TIMER
		virtual void OnWMTimer( void ) ;
		// WM_KEYDOWN
		virtual bool OnWMKeyDown( int64_t nVirtKey, int64_t nFlags ) ;
		// WM_KEYUP
		virtual bool OnWMKeyUp( int64_t nVirtKey, int64_t nFlags ) ;
		// WM_SETFOCUS
		virtual void OnWMSetFocus( void ) ;
		// WM_KILLFOCUS
		virtual void OnWMKillFocus( void ) ;
		// WM_CHAR
		virtual bool OnWMChar( uint16_t codeChar ) ;
		// WM_IME_STARTCOMPOSITION
		virtual bool OnWMImeStartComposition( WPARAM wParam, LPARAM lParam ) ;
		// WM_IME_ENDCOMPOSITION
		virtual bool OnWMImeEndComposition( WPARAM wParam, LPARAM lParam ) ;
		// WM_IME_COMPOSITION
		virtual bool OnWMImeComposition( WPARAM wParam, LPARAM lParam ) ;
		// WM_SETCURSOR
		virtual void OnWMSetCursorClient( void ) ;
		// WM_CLOSE
		virtual void OnWMClose( void ) ;

		friend class UIThreadProcedure ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// GDI 表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowGDIDisplayMethod	: public SGLWindowDisplayMethod
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowGDIDisplayMethod, SGLWindowDisplayMethod )
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd,
				SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft = NULL ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// アナグリフGDI表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAnaglyphGDIDisplayMethod	: public SGLWindowGDIDisplayMethod
	{
	protected:
		SGLImage	m_imgView ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAnaglyphGDIDisplayMethod, SGLWindowGDIDisplayMethod )
		// 構築関数
		SGLAnaglyphGDIDisplayMethod( void ) ;
		// 消滅関数
		virtual ~SGLAnaglyphGDIDisplayMethod( void ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd,
				SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft = NULL ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// インターリーブGDI表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLInterleavedGDIDisplayMethod	: public SGLWindowGDIDisplayMethod
	{
	public:
		enum	Flag
		{
			flagVertical	= 0x01,
			flagSwapEyes	= 0x02,
		} ;

	protected:
		SGLImage	m_imgView ;
		uint32_t	m_nParam ;
		uint32_t	m_nPosSwap ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLInterleavedGDIDisplayMethod, SGLWindowGDIDisplayMethod )
		// 構築関数
		SGLInterleavedGDIDisplayMethod( void ) ;
		// 消滅関数
		virtual ~SGLInterleavedGDIDisplayMethod( void ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd,
				SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft = NULL ) ;
	protected:
		void InterleaveHorizontal
			( const SGLImageBuffer& imgView,
				const SGLImageBuffer& imgRight,
				const SGLImageBuffer& imgLeft ) ;
		void InterleaveVertical
			( const SGLImageBuffer& imgView,
				const SGLImageBuffer& imgRight,
				const SGLImageBuffer& imgLeft ) ;
	public:
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
	} ;

}

struct IDirect3D9 ;
struct IDirect3DDevice9 ;
struct IDirect3DSurface9 ;
struct _D3DPRESENT_PARAMETERS_ ;

namespace SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// NVIDIA 3D Vision (NVStereoBLT) 表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLNvidia3DVisionDisplayMethod	: public SGLWindowDisplayMethod
	{
	protected:
		static HMODULE				m_hModuleD3D9 ;
		SGLAbstractWindow *			m_pWnd ;
		HWND						m_hWnd ;
		SGLDisplayMode				m_displayMode ;
		IDirect3D9 *				m_id3d9 ;
		IDirect3DDevice9 *			m_id3d9Dev ;
		 _D3DPRESENT_PARAMETERS_ *	m_pd3dpp ;
		IDirect3DSurface9 *			m_idds9View ;
		SGLSize						m_sizeView ;
		uint32_t					m_nViewParam ;

		struct Nv_Stereo_Image_Header
		{
			DWORD	dwSignature ;
			DWORD	dwWidth ;
			DWORD	dwHeight ;
			DWORD	dwBPP ;
			DWORD	dwFlags ;
		} ;
		enum	Nv_Stereo_Signature
		{
			NVSTEREO_IMAGE_SIGNATURE = 0x4433564e,	// "NV3D"
		} ;
		enum	Nv_Stereo_Flags
		{
			SIH_SWAP_EYES		= 0x00000001,
			SIH_SCALE_TO_FIT	= 0x00000002,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLNvidia3DVisionDisplayMethod, SGLWindowDisplayMethod )
		// 構築関数
		SGLNvidia3DVisionDisplayMethod( void ) ;
		// 消滅関数
		virtual ~SGLNvidia3DVisionDisplayMethod( void ) ;

	public:
		// Direct3D9 生成
		SGLError CreateDirect3D9Device
			( HWND hWnd, int nWidth, int nHeight, int nAdapter, bool fWindowed ) ;
		// Direct3D9 リセット
		SGLError ResetDirect3D9Device( void ) ;
		// Direct3D9 オブジェクト解放
		SGLError Release( void ) ;
		// 表示用サーフェス生成
		SGLError CreateViewSurface( int nWidth, int nHeight ) ;
		// サーフェス削除
		SGLError ReleaseSurface( void ) ;
		// 表示サーフェースにデータ転送
		SGLError PrepareViewSurface
			( SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft ) ;
		// 描画処理
		SGLError NVStereoBLT( void ) ;

	public:
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd,
				SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft = NULL ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
	} ;

}

#endif

