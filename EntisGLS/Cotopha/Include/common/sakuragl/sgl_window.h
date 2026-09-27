
#if	!defined(__SAKURAGL_SGL_WINDOW_H__)
#define	__SAKURAGL_SGL_WINDOW_H__	1

#include <sakuragl/sgl3d_image.h>
#include <sakuragl/sgl2d/sgl_font.h>

namespace	SakuraGL
{
	#if	defined(__COTOPHA__)
	class	native Window ;
	class	native WindowMenu ;
	class	SGLWindowMenu ;
	#else
	class	SGLAbstractWindow ;
	class	SGLWindowMenu ;
	typedef	SGLAbstractWindow	Window ;
	typedef	SGLWindowMenu		WindowMenu ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 描画ハンドラ
	//////////////////////////////////////////////////////////////////////////

	class	SGLPaintInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLPaintInterface )
		// 描画
		virtual void OnPaint( Window * pWnd, RenderContext * context ) ;
		// 描画前フレーム準備処理（全視点共通処理）
		virtual void OnPrepareFrame( Window * pWnd ) ;
		// 全描画完了
		virtual void OnFinishedFrame( Window * pWnd ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// タイマーハンドラ
	//////////////////////////////////////////////////////////////////////////

	class	SGLTimerInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLTimerInterface )
		// タイマー
		virtual void OnTimer( Window * pWnd, uint64_t idTimer ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ユーザー入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLMouseInterface
	{
	public:
		// フラグ
		enum	MouseFlag
		{
			MouseIDMask		= 0x0000FFFF,
			ButtonIDMask	= 0x00FF0000,
			TouchFlag		= 0x01000000,
			ButtonIDShifter	= 16,
			LeftButtonID	= 0,
			RightButtonID	= 1,
			MiddleButtonID	= 2,
			WheelDeltaUnit	= 0x100,
		} ;
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLMouseInterface )
		// マウス移動
		virtual bool OnMouseMove
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( Window * pWnd, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( Window * pWnd, int32_t zDelta,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnLButtonDblClk
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool OnRButtonDown
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnRButtonUp
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnRButtonDblClk
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
	public:
		// マウス識別子を取得（マルチタッチ用）
		static uint32_t GetMouseID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & MouseIDMask) ;
		}
		// マウスボタン識別子を取得
		static uint32_t GetButtonID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & ButtonIDMask) >> ButtonIDShifter ;
		}
		// タッチパネル入力判定
		static bool IsFromTouch( int64_t nFlags )
		{
			return	((nFlags & TouchFlag) != 0) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// キー入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLKeyInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLKeyInterface )
		// キー入力
		virtual bool OnKeyDown
			( Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		// フォーカス
		virtual void OnSetFocus( Window * pWnd ) ;
		virtual void OnKillFocus( Window * pWnd ) ;
	} ;

	enum	VirtualKeyCode
	{
		vkeyMouseLeft	= 0x01,
		vkeyMouseRight	= 0x02,
		vkeyMouseMiddle	= 0x04,
		vkeyBack		= 0x08,
		vkeyTab			= 0x09,
		vkeyReturn		= 0x0D,
		vkeyShift		= 0x10,
		vkeyControl		= 0x11,
		vkeyMenu		= 0x12,
		vkeyPause		= 0x13,
		vkeyCapital		= 0x14,
		vkeyEscape		= 0x1B,
		vkeySpace		= 0x20,
		vkeyPageUp		= 0x21,
		vkeyPageDown	= 0x22,
		vkeyEnd			= 0x23,
		vkeyHome		= 0x24,
		vkeyLeft		= 0x25,
		vkeyUp			= 0x26,
		vkeyRight		= 0x27,
		vkeyDown		= 0x28,
		vkeyInsert		= 0x2D,
		vkeyDelete		= 0x2E,
		vkeyHelp		= 0x2F,
		vkeyNumPad0		= 0x60,
		vkeyNumPad1, vkeyNumPad2, vkeyNumPad3,
		vkeyNumPad4, vkeyNumPad5, vkeyNumPad6,
		vkeyNumPad7, vkeyNumPad8, vkeyNumPad9,
		vkeyNumPadMultiply, vkeyNumPadAdd,
		vkeyNumPadSeparator, vkeyNumPadSubtract,
		vkeyNumPadDecimal, vkeyNumPadDivide,
		vkeyFunction1, vkeyFunction2,
		vkeyFunction3, vkeyFunction4,
		vkeyFunction5, vkeyFunction6,
		vkeyFunction7, vkeyFunction8,
		vkeyFunction9, vkeyFunction10,
		vkeyFunction11, vkeyFunction12,
		vkeyNumLock		= 0x90,
		vkeyScroll		= 0x91,
		vkeyPlay		= 0xFA,
		vkeyCodeMask	= 0xFFFF,
	} ;
	enum	VirtualKeyFlag
	{
		vkeyContextCapital	= 0x010000,
		vkeyContextShift	= 0x100000,
		vkeyContextControl	= 0x200000,
		vkeyContextMenu		= 0x400000,
		vkeyContextMask		= 0xFF0000,
	} ;

	// シリアライズ用キーコードと名前対応
	extern const SSystem::SXMLDocument::AttrInteger	g_aiVirtualKeyCode[100] ;


	//////////////////////////////////////////////////////////////////////////
	// 文字入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	struct	SGLInputStartComposition
	{
		enum	Flags
		{
			flagPosition	= 0x0001,
			flagRectangle	= 0x0002,
			flagFont		= 0x0004,
		} ;
		int64_t			nFlags ;
		SGLPoint		ptStart ;
		SGLImageRect	rctArea ;
		SGLFontStyle	fsFontStyle ;
		int64_t			nReserved[0x10] ;
	} ;

	struct	SGLInputCompositionString
	{
		enum	Flags
		{
			flagResult		= 0x01,
		} ;
		int64_t			nFlags ;
		const wchar_t *	pszComposition ;
		uint32_t		nStart ;
		uint32_t		nCount ;
	} ;

	class	SGLCharInputInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLCharInputInterface )
		// 文字入力
		virtual bool OnChar( Window * pWnd, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( Window * pWnd, SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( Window * pWnd ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( Window * pWnd, const SGLInputCompositionString& icsComp ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コマンド・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLCommandInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLCommandInterface )
		// コマンド
		virtual bool OnCommand
			( Window * pWnd, const uint16_t * pszCmd,
							int64_t nParam, int64_t nCode ) ;
	} ;

	#if	defined(__COTOPHA__)
	enum	SysCommandId<String>
	{
		AppExit				= "ID_APP_EXIT",
		AppBack				= "ID_APP_BACK",
		AppSuspend			= "ID_APP_SUSPEND",
		AppResume			= "ID_APP_RESUME",
		AppDestroy			= "ID_APP_DESTROY",
		WindowActive		= "ID_WINDOW_ACTIVE",
		WindowInactive		= "ID_WINDOW_INACTIVE",
		WindowSizeChanged	= "ID_WINDOW_SIZE_CHANGED",
		WindowPollJoyStick	= "ID_WINDOW_POLL_JOY_STICK",
	} ;

	#else
	class	SysCommandId
	{
	public:
		static const wchar_t *	AppExit ;
		static const wchar_t *	AppBack ;
		static const wchar_t *	AppSuspend ;
		static const wchar_t *	AppResume ;
		static const wchar_t *	AppDestroy ;
		static const wchar_t *	WindowActive ;
		static const wchar_t *	WindowInactive ;
		static const wchar_t *	WindowSizeChanged ;
		static const wchar_t *	WindowPollJoyStick ;
	} ;

	#endif


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Window
	{
	public:
		// ウィンドウモード
		enum	CooperationMode
		{
			modeWindow		= 0x0000,
			modeNormal		= 0x0001,
			modeFullScreen	= 0x0003,
			modeExclusive	= 0x0007,
		} ;
		// オプション機能フラグ
		enum	OptionalFlag
		{
			flagUseDblClick			= 0x00000001,
			flagAllowClose			= 0x00000002,
			flagBlackBack			= 0x00000004,
			flagEnableIME			= 0x00000008,
			flagAllowMinimize		= 0x00000010,
			flagGrantScreenSave		= 0x00000020,
			flagGrantMonitorSave	= 0x00000040,
			flagGrantPowerSuspend	= 0x00000080,
			flagVariableWindowSize	= 0x00000100,
			flagAllowMaximize		= 0x00000200,
			flagChildWindow			= 0x00001000,
			flagPopupWindow			= 0x00002000,
			flagInvisibleWindow		= 0x00004000,
			flagOpenIME				= 0x00010000,
			flagDoMinimize			= 0x00020000,
			flagDoMaximize			= 0x00040000,
			flagDoNormalize			= 0x00080000,
		} ;
		// ウィンドウフラグ
		enum	WindowFlag
		{
			flagLayeredWindow		= 0x00000001,
		} ;
		// ウィンドウレイアウト
		enum	Layout
		{
			layoutNothing			= 0,
			layoutOffsetClient		= 1,
			layoutDockingLeft		= 2,
			layoutDockingRight		= 3,
			layoutDockingUpper		= 4,
			layoutDockingUnder		= 5,
			layoutAlignLeft			= 0x00,
			layoutAlignTop			= 0x00,
			layoutAlignCenter		= 0x10,
			layoutAlignRight		= 0x20,
			layoutAlignBottom		= 0x20,
			layoutAlignAccording	= 0x30,
			layoutTypeClient		= 0x00,
			layoutTypeWindow		= 0x40,
		} ;
		// 有効画面外枠フラグ
		enum	ExteriorFrameType
		{
			exteriorFillColor	= 0x01,
			exteriorStretch		= 0x02,
		} ;
		// ステレオ立体視メソッド
		enum	Stereo3D<String>
		{
			AnaglyphView		= "AnaglyphView",
			DDStereoscopic		= "DDStereoscopic",
			OpenGLQuadBuffer	= "OpenGLQuadBuffer",
			NVStereoBLT			= "NVStereoBLT",
			SideBySide			= "SideBySide",
			InterleavedView		= "InterleavedView",
			MonoView			= "",
		} ;
		// ウィンドウ更新動作
		enum	UpdateFlags
		{
			updateVSync			= 0x01,		// V-SYNC 同期
			updateWaitFrames	= 0x02,		// 可変フレームレートに対応した待機処理
		} ;
		struct	UpdateParameter
		{
			int64_t		flagsUpdate ;		// enum UpdateFlags の複合
			uint32_t	framesPerSec ;		// 期待するフレームレート（零の時は無視）
			uint32_t	framesPast ;		// 待機処理／次に進めるべき推奨フレーム数
			int64_t		msecLastUpdate ;	// 更新完了時間 [ms]
			uint32_t	msecRendering ;		// 描画ハンドラに要した時間 [ms]
			int32_t		msecFrameError ;	// 描画タイミング累積誤差 [ms]
		} ;

	public:	// 仮想ディスプレイメソッド
		// 仮想ディスプレイ開始
		native SGLError CreateDisplay
			( const char * pszWindowName,
				CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイ終了
		native SGLError CloseDisplay( void ) ;
		// オプション機能フラグ取得
		native uint64_t GetOptionalFlags( void ) ;
		// オプション機能フラグ設定
		native void SetOptionalFlags( uint64_t nFlags ) ;
		// ウィンドウモード変更
		native SGLError ChangeCooperationLevel( CooperationMode mode ) ;
		// ウィンドウモード取得
		native CooperationMode GetCooperationLevel( void ) ;
		// 仮想ディスプレイサイズ変更
		native SGLError ChangeDisplaySize
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイサイズ取得
		native SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
		// 物理モニタの解像度を変更するか？
		native SGLError EnableChangePhysicalMode( bool flagEnable ) ;
		// ｚバッファ設定
		native SGLError EnableZBuffer( bool flagZBuffer ) ;
		// ステレオ立体視モード設定
		native SGLError SetStereoDisplayMode
			( const char * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードテスト
		native bool IsSupportedStereoDisplayMode( const char * pszMethodID ) ;
		// 仮想ディスプレイ・ウィンドウ初期座標設定
		native SGLError InitWindowPosition
			( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウの通常座標取得
		native SGLError GetNormalWindowPosition
			( SGLPoint& ptWindow, SGLSize * pWindowSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウ内表示座標取得
		native SGLError GetInternalDisplayPosition
				( SGLImageRect& rctRender, SGLImageRect& rctDisplay ) ;
		// 仮想ディスプレイ・有効画面外枠表示設定
		native SGLError SetExteriorBackgroundFrame
			( uint32_t nFlags, uint32_t rgbColor, Image* pTile,
				Image* pLeft = NULL, Image* pRight = NULL,
				Image* pUpper = NULL, Image* pUnder = NULL ) ;

	public:	// 汎用ウィンドウ専用
		// ウィンドウ生成
		native SGLError CreateWindow
			( const char * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags = 0, Window * pParentWnd = NULL ) ;
		// ウィンドウを閉じる
		native SGLError CloseWindow( void ) ;
		// ウィンドウ位置を設定する
		native SGLError SetWindowLayout
			( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;
		// クライアント座標→スクリーン座標変換
		native S2DDVector& ScreenPositionFromClient( S2DDVector& vClient ) ;
		// スクリーン座標→クライアント座標変換
		native S2DDVector& ClientPositionFromScreen( S2DDVector& vScreen ) ;

	public:	// 仮想ディスプレイ・汎用ウィンドウ共通
		// 画面の更新通知
		native SGLError PostUpdate( const SGLImageRect* pUpdate = NULL ) ;
		// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
		native SGLError UpdateWindow( UpdateParameter * pUpdate = NULL ) ;
		// ユーザー入力処理
		native SGLError ProcessUserInput( int64_t msecTimeout = 1 ) ;
		// 描画スレッドで実行
		enum	PostThreadType
		{
			postNormal	= 0,
			postDelay,
			postAsyncNoRender,
		} ;
		native SGLError PostRenderingThread
			( SSystem::SProcedure * pProc, PostThreadType postType ) ;
		// UI スレッドで実行
		native SGLError PostUIThread( SSystem::SProcedure * pProc ) ;
		// ウィンドウがアクティブ（最前面）か？
		native bool IsWindowActive( void ) ;
		// ウィンドウキャプション設定
		native SGLError SetWindowCaption( const wchar_t * pszWindowName ) ;
		// マウスカーソル表示
		native SGLError ShowCursor( bool fShow ) ;
		// マウスカーソル表示状態取得
		native bool IsShowCursor( void ) ;
		// マウスカーソル変更
		native SGLError SetCursor( const wchar_t * pszCursorID ) ;
		// マウスカーソル座標移動
		native SGLError MoveCursorPosition
			( int32_t xPos, int32_t yPos, int idMouse = 0 ) ;
		// マウスカーソル座標取得
		native SGLError GetCursorPosition
			( SGLPoint& ptCursor, int idMouse = 0 ) ;
		// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
		native int GetMonitorFrequency( void ) ;
		// ウィンドウメニューの設定
		native SGLError AttachMenu( WindowMenu * pMenu ) ;

	public:
		// 描画ハンドラ
		native SGLPaintInterface *
					SetPaintInterface( SGLPaintInterface * pPaint ) ;
		native SGLPaintInterface *
					SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
		// タイマーハンドラ
		native SGLTimerInterface *
					SetTimerInterface( SGLTimerInterface * pTimer ) ;
		// マウス入力インターフェース
		native SGLMouseInterface *
					SetMouseInterface( SGLMouseInterface * pMouse ) ;
		native SGLMouseInterface *
					SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
		// マウスイベントキャプチャー
		native SGLError CaptureMouse( int idMouse = 0 ) ;
		native SGLError ReleaseMouse( int idMouse = 0 ) ;
		// キー入力インターフェース
		native SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
		// 文字入力インターフェース
		native SGLCharInputInterface *
					SetCharInputInterface( SGLCharInputInterface * pChar ) ;
		// コマンド・インターフェース
		native SGLCommandInterface *
					SetCommandInterface( SGLCommandInterface * pCmd ) ;

	public:
		// 描画インターフェース取得
		native RenderContext* GetRenderContext
				( S3DRenderContextInterface::StereoViewIndex sviView
								= S3DRenderContextInterface::stereoViewAuto ) ;
		native void ReleaseRenderContext( RenderContext* context ) ;

	} ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 抽象ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	#if	defined(CreateWindow)
	// winuser.h 内での定義の副作用を消去
	#undef	CreateWindow
	#endif

	class	SGLAbstractWindow	: public SSystem::SObject
	{
	public:
		// ウィンドウモード
		#if	!defined(__COTOPHA__)
		enum	CooperationMode
		{
			modeWindow		= 0x0000,
			modeNormal		= 0x0001,
			modeFullScreen	= 0x0003,
			modeExclusive	= 0x0007,
		} ;
		// オプション機能フラグ
		enum	OptionalFlag
		{
			flagUseDblClick			= 0x00000001,
			flagAllowClose			= 0x00000002,
			flagBlackBack			= 0x00000004,
			flagEnableIME			= 0x00000008,
			flagAllowMinimize		= 0x00000010,
			flagGrantScreenSave		= 0x00000020,
			flagGrantMonitorSave	= 0x00000040,
			flagGrantPowerSuspend	= 0x00000080,
			flagVariableWindowSize	= 0x00000100,
			flagAllowMaximize		= 0x00000200,
			flagChildWindow			= 0x00001000,
			flagPopupWindow			= 0x00002000,
			flagInvisibleWindow		= 0x00004000,
			flagOpenIME				= 0x00010000,
			flagDoMinimize			= 0x00020000,
			flagDoMaximize			= 0x00040000,
			flagDoNormalize			= 0x00080000,
		} ;
		// ウィンドウフラグ
		enum	WindowFlag
		{
			flagLayeredWindow		= 0x00000001,
		} ;
		// ウィンドウレイアウト
		enum	Layout
		{
			layoutNothing			= 0,
			layoutOffsetClient		= 1,
			layoutDockingLeft		= 2,
			layoutDockingRight		= 3,
			layoutDockingUpper		= 4,
			layoutDockingUnder		= 5,
			layoutDockingMask		= 0x0F,
			layoutAlignLeft			= 0x00,
			layoutAlignTop			= 0x00,
			layoutAlignCenter		= 0x10,
			layoutAlignRight		= 0x20,
			layoutAlignBottom		= 0x20,
			layoutAlignAccording	= 0x30,
			layoutAlignMask			= 0x30,
			layoutTypeClient		= 0x00,
			layoutTypeWindow		= 0x40,
		} ;
		// 有効画面外枠フラグ
		enum	ExteriorFrameType
		{
			exteriorFillColor	= 0x01,
			exteriorStretch		= 0x02,
		} ;
		// ステレオ立体視メソッド
		class	Stereo3D
		{
		public:
			static const wchar_t *	AnaglyphView ;
			static const wchar_t *	DDStereoscopic ;
			static const wchar_t *	OpenGLQuadBuffer ;
			static const wchar_t *	NVStereoBLT ;
			static const wchar_t *	SideBySide ;
			static const wchar_t *	InterleavedView ;
			static const wchar_t *	MonoView ;
		} ;
		enum	StereoViewFlag
		{
			stereoFlagVertical			= 0x01,	// 垂直インターリーブ
			stereoFlagSwapEyes			= 0x02,	// 左右入れ替え
			stereoFlagPixelAspect1_1	= 0x04,	// ピクセルアスペクト比 1:1
			stereoFlagPixelAspect1_2	= 0x08,	// ピクセルアスペクト比 1:2
			stereoFlagLensScale			= 0x10,	// VR 用拡大率適用
			stereoFlagLensDistortion	= 0x20,	// VR レンズ歪み適用
		} ;
		// ウィンドウ更新動作
		enum	UpdateFlags
		{
			updateVSync			= 0x01,		// V-SYNC 同期
			updateWaitFrames	= 0x02,		// 可変フレームレートに対応した待機処理
		} ;
		struct	UpdateParameter
		{
			int64_t		flagsUpdate ;		// enum UpdateFlags の複合
			uint32_t	framesPerSec ;		// 期待するフレームレート（零の時は無視）
			uint32_t	framesPast ;		// 待機処理／次に進めるべき推奨フレーム数
			int64_t		msecLastUpdate ;	// 更新完了時間 [ms]
			uint32_t	msecRendering ;		// 描画ハンドラに要した時間 [ms]
			int32_t		msecFrameError ;	// 描画タイミング累積誤差 [ms]

			UpdateParameter( void )
				: flagsUpdate(0), framesPerSec(0), framesPast(0),
					msecLastUpdate(0), msecRendering(0), msecFrameError(0) { }
			void WaitFrame( uint32_t msecCurrentRendering, uint32_t freqMonitor ) ;
		} ;
		#endif

	protected:
		SGLPaintInterface *		m_pPaintHandler ;
		SGLPaintInterface *		m_pDirectPaintHandler ;
		SGLTimerInterface *		m_pTimerHandler ;
		SGLMouseInterface *		m_pMouseHandler ;
		SGLMouseInterface *		m_pDirectMouseHandler ;
		SGLKeyInterface *		m_pKeyHandler ;
		SGLCharInputInterface *	m_pCharInputHandler ;
		SGLCommandInterface *	m_pCommandHandler ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAbstractWindow, SObject )
		// 構築関数
		SGLAbstractWindow( void )
			: m_pChainNextWindow(NULL),
				m_pPaintHandler(NULL), m_pDirectPaintHandler(NULL),
				m_pTimerHandler(NULL),
				m_pMouseHandler(NULL), m_pDirectMouseHandler(NULL),
				m_pKeyHandler(NULL), m_pCharInputHandler(NULL),
				m_pCommandHandler(NULL) { }
		// 消滅関数
		virtual ~SGLAbstractWindow( void ) ;

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
		// ウィンドウサイズ（クライアントサイズ）変更
		virtual SGLError ChangeWindowSize( uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウクライアントサイズ取得
		virtual SGLError GetWindowClientRect( SGLImageRect& rctClient ) ;
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
		enum	PostThreadType
		{
			postNormal	= 0,
			postDelay,
			postAsyncNoRender,
			postAsyncNoRenderFinally,
		} ;
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

	public:
		// 描画ハンドラ
		virtual SGLPaintInterface *
					SetPaintInterface( SGLPaintInterface * pPaint ) ;
		virtual SGLPaintInterface *
					SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
		SGLPaintInterface * GetPaintInterface( void ) const
		{
			return	m_pPaintHandler ;
		}
		SGLPaintInterface * GetDirectPaintInterface( void ) const
		{
			return	m_pDirectPaintHandler ;
		}
		// タイマーハンドラ
		virtual SGLTimerInterface *
					SetTimerInterface( SGLTimerInterface * pTimer ) ;
		SGLTimerInterface * GetTimerInterface( void ) const
		{
			return	m_pTimerHandler ;
		}
		// マウス入力インターフェース
		virtual SGLMouseInterface *
					SetMouseInterface( SGLMouseInterface * pMouse ) ;
		virtual SGLMouseInterface *
					SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
		SGLMouseInterface * GetMouseInterface( void ) const
		{
			return	m_pMouseHandler ;
		}
		SGLMouseInterface * GetDirectMouseInterface( void ) const
		{
			return	m_pDirectMouseHandler ;
		}
		// マウスイベントキャプチャー
		virtual SGLError CaptureMouse( int idMouse = 0 ) = 0 ;
		virtual SGLError ReleaseMouse( int idMouse = 0 ) = 0 ;
		// キー入力インターフェース
		virtual SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
		SGLKeyInterface * GetKeyInterface( void ) const
		{
			return	m_pKeyHandler ;
		}
		// 文字入力インターフェース
		virtual SGLCharInputInterface *
					SetCharInputInterface( SGLCharInputInterface * pChar ) ;
		SGLCharInputInterface * GetCharInputInterface( void ) const
		{
			return	m_pCharInputHandler ;
		}
		// コマンド・インターフェース
		virtual SGLCommandInterface *
					SetCommandInterface( SGLCommandInterface * pCmd ) ;
		SGLCommandInterface * GetCommandInterface( void ) const
		{
			return	m_pCommandHandler ;
		}

	public:
		// 描画インターフェース取得
		virtual S3DRenderContextInterface * GetRenderContext
			( S3DRenderContextInterface::StereoViewIndex sviView
						= S3DRenderContextInterface::stereoViewAuto ) = 0 ;
		virtual void ReleaseRenderContext
						( S3DRenderContextInterface* context ) = 0 ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) = 0 ;

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

	public:
		// プラットフォーム固有オブジェクト
		#if	defined(__COTOPHA__)
		virtual Window* GetWindowObject( void ) const = 0 ;
		operator Window* ( void ) const ;
		#elif	defined(__PLATFORM_WINDOWS__)
		virtual HWND GetWindowHandle( void ) const = 0 ;
		operator HWND ( void ) const
		{
			return	GetWindowHandle() ;
		}
		#endif
		// 互換のためのキャスト
		operator SGLAbstractWindow* ( void ) const ;

	protected:
		// ウィンドウ・オブジェクト・チェーン
		SGLAbstractWindow *			m_pChainNextWindow ;
		static SGLAbstractWindow *	m_pChainFirstWindow ;

		void AddWindowToChain( void ) ;
		void DetachWindowFromChain( void ) ;

		#if	defined(__PLATFORM_WINDOWS__)
		static SGLAbstractWindow * FromHandle( HWND hWnd ) ;
		#endif

	public:
		// デフォルトウィンドウ取得
		static SGLAbstractWindow * GetDefaultWindow( void )
		{
			return	m_pChainFirstWindow ;
		}
		// 次のウィンドウ列挙
		SGLAbstractWindow * EnumerateNextWindow( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ状態モニタ・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowMonitorInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowMonitorInterface, ESLObject ) ;
		// 描画完了通知受け取り登録（シグナルを受け取ると自動的に登録解除）
		virtual void AttachOnceSignalForFramePaint( SSystem::SSignalEvent * pSignal ) = 0 ;
		// 描画処理中か？
		virtual bool IsWindowPainting( double * pmsecPastPainting = nullptr ) const = 0 ;

	} ;

}


#include <sakuragl/sgl_window_implement.h>


#endif
