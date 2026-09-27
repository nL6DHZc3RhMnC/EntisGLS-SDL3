
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_WINDOW_H__)
#define	__GLSCS_SAKURA2_OBJECT_WINDOW_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// ウィンドウオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	WindowObject	: public ECSSakura2::Object,
								public SakuraGL::SGLWindow,
								public SakuraGL::SGLTimerInterface,
								public SakuraGL::SGLKeyInterface,
								public SakuraGL::SGLCharInputInterface,
								public SakuraGL::SGLCommandInterface
	{
	protected:
		// 描画関数呼び出し
		class	SGLPaintCaller	: public SakuraGL::SGLPaintInterface
		{
		public:
			WindowObject *	m_pWnd ;
			INT64			m_addrPaint ;
			INT64			m_addrRender ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLPaintCaller, SGLPaintInterface )
			// 構築関数
			SGLPaintCaller( void )
				: m_pWnd(NULL), m_addrPaint(0), m_addrRender(0) {}
			// 描画
			virtual void OnPaint
				( SakuraGL::Window * pWnd, SakuraGL::RenderContext * context ) ;
			// 描画前フレーム準備処理（全視点共通処理）
			virtual void OnPrepareFrame( SakuraGL::Window * pWnd ) ;
		} ;
		// マウス関数呼び出し
		class	SGLMouseCaller	: public SakuraGL::SGLMouseInterface
		{
		public:
			WindowObject *	m_pWnd ;
			INT64			m_addrMouse ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLMouseCaller, SGLMouseInterface )
			// 構築関数
			SGLMouseCaller( void ) : m_pWnd(NULL), m_addrMouse(0) {}
			// マウス移動
			virtual bool OnMouseMove
				( SakuraGL::Window * pWnd,
					int32_t xPos, int32_t yPos, int64_t nFlags ) ;
			virtual void OnMouseLeave
				( SakuraGL::Window * pWnd, int64_t nFlags ) ;
			// ホイール回転
			virtual bool OnMouseWheel
				( SakuraGL::Window * pWnd, int32_t zDelta,
					int32_t xPos, int32_t yPos, int64_t nFlags ) ;
			// マウスボタン
			virtual bool OnButtonDown
				( SakuraGL::Window * pWnd,
					int32_t xPos, int32_t yPos, int64_t nFlags ) ;
			virtual bool OnButtonUp
				( SakuraGL::Window * pWnd,
					int32_t xPos, int32_t yPos, int64_t nFlags ) ;
			virtual bool OnButtonDblClk
				( SakuraGL::Window * pWnd,
					int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		} ;
		// 関数呼び出し
		class	SGLProcedureCaller	: public SSystem::SProcedure
		{
		public:
			WindowObject *	m_pWnd ;
			INT64			m_addrProc ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SGLProcedureCaller, SProcedure )
			// 構築関数
			SGLProcedureCaller( void ) : m_pWnd(NULL), m_addrProc(0) {}
			SGLProcedureCaller( WindowObject * pWnd, INT64 addrProc )
						: m_pWnd(pWnd), m_addrProc(addrProc) {}
			// スレッド関数
			virtual void Run( void ) ;
			// 開始前の処理
			virtual void Prepare( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

		friend class SGLPaintCaller ;
		friend class SGLMouseCaller ;
		friend class SGLProcedureCaller ;

	protected:
		enum	CreationStatus
		{
			createdNothing,
			createdDisplay,
			createdWindow,
		} ;
		struct	CREATION_PARAM
		{
			CreationStatus		statusCreation ;
			CooperationMode		modeCopperation ;
			uint32_t			widthWindow ;
			uint32_t			heightWindow ;
			uint32_t			nBitsPerPixel ;
			uint32_t			nFrequency ;
			uint32_t			nCreationFlags ;
			uint64_t			nOptinalFlags ;
			DWORD				addrParentWnd ;
			uint32_t			nLayoutFlags ;
			SakuraGL::SGLPoint	ptLayout ;
			bool				flagChangePhysMode ;
			bool				flagZBuffer ;
		} ;
		SSystem::SString	m_strWindowName ;
		CREATION_PARAM		m_paramWindow ;

		SSystem::SString	m_strStereoMethodID ;
		uint64_t			m_nStereoParam ;

		struct	EXTERIOR_FRAME_PARAM
		{
			uint32_t	nFlags ;
			uint32_t	rgbColor ;
			DWORD		addrTile ;
			DWORD		addrLeft ;
			DWORD		addrRight ;
			DWORD		addrUpper ;
			DWORD		addrUnder ;
		} ;
		EXTERIOR_FRAME_PARAM	m_paramExFrame ;

		enum	PaintVectorIndex
		{
			vectorOnPaint,
			vectorOnPrepareFrame,
		} ;
		enum	TimerVectorIndex
		{
			vectorOnTimer,
		} ;
		enum	MouseVectorIndex
		{
			vectorOnMouseMove,
			vectorOnMouseLeave,
			vectorOnMouseWheel,
			vectorOnButtonDown,
			vectorOnButtonUp,
			vectorOnButtonDblClk,
		} ;
		enum	KeyVectorIndex
		{
			vectorOnKeyDown,
			vectorOnKeyUp,
			vectorOnSetFocus,
			vectorOnKillFocus,
		} ;
		enum	CharInputVectorIndex
		{
			vectorOnChar,
			vectorOnStartComposition,
			vectorOnEndComposition,
			vectorOnCompositionString,
		} ;
		enum	CommandVectorIndex
		{
			vectorOnCommand,
		} ;

		VirtualMachine *	m_pVM ;
		DWORD				m_dwRenderObj ;
		SGLPaintCaller		m_callPaint ;
		SGLPaintCaller		m_callDirectPaint ;
		SGLMouseCaller		m_callMouse ;
		SGLMouseCaller		m_callDirectMouse ;

		struct	HANDLER_PARAM
		{
			INT64	addrPaint ;
			INT64	addrDirectPaint ;
			INT64	addrTimer ;
			INT64	addrMouse ;
			INT64	addrDirectMouse ;
			INT64	addrKey ;
			INT64	addrCharInput ;
			INT64	addrCommand ;
		} ;
		HANDLER_PARAM	m_handler ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( WindowObject, ECSSakura2::Object, SGLWindow )
		// 構築関数
		WindowObject( VirtualMachine * vm ) ;
		// 消滅関数
		virtual ~WindowObject( void ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 破棄処理
		virtual void OnDestruction
			( VirtualMachine * vm, Context * context ) ;
		// 保存処理
		virtual SSystem::SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SSystem::SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後処理
		virtual SSystem::SError CommitAfterLoad
			( VirtualMachine * vm, Context * context ) ;

	protected:
		// レンダリングオブジェクトを仮想マシンに登録する
		void RegisterRenderObject( VirtualMachine * vm ) ;
		// ウィンドウハンドラ適用
		void EnableWindowHandler( void ) ;

	public:	// SGLAbstractWindow
		// 仮想ディスプレイ開始
		virtual SakuraGL::SGLError CreateDisplay
			( const wchar_t * pszWindowName,
				CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイ終了
		virtual SakuraGL::SGLError CloseDisplay( void ) ;
		// オプション機能フラグ設定
		virtual void SetOptionalFlags( uint64_t nFlags ) ;
		// ウィンドウモード変更
		virtual SakuraGL::SGLError ChangeCooperationLevel( CooperationMode mode ) ;
		// 仮想ディスプレイサイズ変更
		virtual SakuraGL::SGLError ChangeDisplaySize
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 物理モニタの解像度を変更するか？
		virtual SakuraGL::SGLError EnableChangePhysicalMode( bool fEnable ) ;
		// ｚバッファ設定
		virtual SakuraGL::SGLError EnableZBuffer( bool flagZBuffer ) ;
		// ステレオ立体視モード設定
		virtual SakuraGL::SGLError SetStereoDisplayMode
			( const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// 仮想ディスプレイ・有効画面外枠表示設定
		virtual SakuraGL::SGLError SetExteriorBackgroundFrame
			( uint32_t nFlags, uint32_t rgbColor,
				SakuraGL::SGLImageObject* pTile,
				SakuraGL::SGLImageObject* pLeft = NULL,
				SakuraGL::SGLImageObject* pRight = NULL,
				SakuraGL::SGLImageObject* pUpper = NULL,
				SakuraGL::SGLImageObject* pUnder = NULL ) ;

	public:
		// ウィンドウ生成
		virtual SakuraGL::SGLError CreateWindow
			( const wchar_t * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags = 0,
				SakuraGL::SGLAbstractWindow * pParentWnd = NULL ) ;
		// ウィンドウを閉じる
		virtual SakuraGL::SGLError CloseWindow( void ) ;
		// ウィンドウ位置を設定する
		virtual SakuraGL::SGLError SetWindowLayout
			( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;

	public:
		// レンダリングスレッドで実行
		SakuraGL::SGLError PostRenderingThread
			( uint64_t addrHandler, PostThreadType postType ) ;
		// UI スレッドで実行
		SakuraGL::SGLError PostUIThread( uint64_t addrHandler ) ;

	public:	// スクリプトハンドラ設定
		// 描画ハンドラ
		uint64_t SetScriptPaintHandler( uint64_t addrHandler ) ;
		uint64_t SetScriptDirectPaintHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptPaintHandler( void ) const
		{
			return	m_handler.addrPaint ;
		}
		uint64_t GetScriptDirectPaintHandler( void ) const
		{
			return	m_handler.addrDirectPaint ;
		}
		// タイマーハンドラ
		uint64_t SetScriptTimerHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptTimerHandler( void ) const
		{
			return	m_handler.addrTimer ;
		}
		// マウス入力インターフェース
		uint64_t SetScriptMouseHandler( uint64_t addrHandler ) ;
		uint64_t SetScriptDirectMouseHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptMouseHandler( void ) const
		{
			return	m_handler.addrMouse ;
		}
		uint64_t GetScriptDirectMouseHandler( void ) const
		{
			return	m_handler.addrDirectMouse ;
		}
		// キー入力インターフェース
		uint64_t SetScriptKeyHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptKeyHandler( void ) const
		{
			return	m_handler.addrKey ;
		}
		// 文字入力インターフェース
		uint64_t SetScriptCharInputHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptCharInputHandler( void ) const
		{
			return	m_handler.addrCharInput ;
		}
		// コマンド・インターフェース
		uint64_t SetScriptCommandHandler( uint64_t addrHandler ) ;
		uint64_t GetScriptCommandHandler( void ) const
		{
			return	m_handler.addrCommand ;
		}

	public:	// SGLTimerInterface
		// タイマーハンドラ
		virtual void OnTimer( SakuraGL::Window * pWnd, uint64_t idTimer ) ;

	public:	// SGLKeyInterface
		// キー入力
		virtual bool OnKeyDown
			( SakuraGL::Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( SakuraGL::Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		// フォーカス
		virtual void OnSetFocus( SakuraGL::Window * pWnd ) ;
		virtual void OnKillFocus( SakuraGL::Window * pWnd ) ;

	public:	// SGLCharInputInterface
		struct	FONT_STYLE
		{
			uint32_t	nStyles ;
			uint32_t	nSize ;
			int64_t		pszFace ;
		} ;
		struct	INPUT_START_COMPOSITION
		{
			int64_t					nFlags ;
			SakuraGL::SGLPoint		ptStart ;
			SakuraGL::SGLImageRect	rctArea ;
			FONT_STYLE				fsFontStyle ;
			int64_t					nReserved[0x10] ;
		} ;
		struct	INPUT_COMPOSITION_STRING
		{
			int64_t		nFlags ;
			int64_t		pszComposition ;
			uint32_t	nStart ;
			uint32_t	nCount ;
		} ;
		// 文字入力
		virtual bool OnChar( SakuraGL::Window * pWnd, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( SakuraGL::Window * pWnd,
					SakuraGL::SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( SakuraGL::Window * pWnd ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( SakuraGL::Window * pWnd,
					const SakuraGL::SGLInputCompositionString& icsComp ) ;

	public:	// SGLCommandInterface
		// コマンド
		virtual bool OnCommand
			( SakuraGL::Window * pWnd, const uint16_t * pszCmd,
								int64_t nParam, int64_t nCode ) ;

	} ;

}

// new SakuraGL::Window
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_Window) ;

// SGLError SakuraGL::Window::CreateDisplay
//	( const char * pszWindowName,
//		CopperationMode mode, uint32_t nWidth, uint32_t nHeight,
//		uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_CreateDisplay) ;

// SGLError CloseDisplay( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_CloseDisplay) ;

// uint64_t GetOptionalFlags( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetOptionalFlags) ;

// void SetOptionalFlags( uint64_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetOptionalFlags) ;

// SGLError ChangeCooperationLevel( CopperationMode mode ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ChangeCooperationLevel) ;

// CopperationMode GetCooperationLevel( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetCooperationLevel) ;

// SGLError ChangeDisplaySize
//		( uint32_t nWidth, uint32_t nHeight,
//			uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ChangeDisplaySize) ;

// SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetDisplaySize) ;

// SGLError EnableChangePhysicalMode( bool fEnable ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_EnableChangePhysicalMode) ;

// SGLError EnableZBuffer( bool flagZBuffer ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_EnableZBuffer) ;

// SGLError SetStereoDisplayMode
//		( const char * pszMethodID, uint64_t nParam = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetStereoDisplayMode) ;

// bool IsSupportedStereoDisplayMode( const char * pszMethodID ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_IsSupportedStereoDisplayMode) ;

// SGLError InitWindowPosition
//	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_InitWindowPosition) ;

// SGLError GetNormalWindowPosition( SGLPoint& ptWindow, SGLSize * pWindowSize ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetNormalWindowPosition) ;

// SGLError GetInternalDisplayPosition
//				( SGLImageRect& rctRender, SGLImageRect& rctDisplay ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetInternalDisplayPosition) ;

// SGLError SetExteriorBackgroundFrame
//		( uint32_t nFlags, uint32_t rgbColor, Image* pTile,
//			Image* pLeft = NULL, Image* pRight = NULL,
//			Image* pUpper = NULL, Image* pUnder = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetExteriorBackgroundFrame) ;

// SGLError CreateWindow
//	( const char * pszWindowName,
//		uint32_t nWidth, uint32_t nHeight,
//		uint32_t nFlags = 0, Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_CreateWindow) ;

// SGLError CloseWindow( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_CloseWindow) ;

// SGLError SetWindowLayout
//		( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetWindowLayout) ;

// S2DDVector& ScreenPositionFromClient( S2DDVector& vClient ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ScreenPositionFromClient) ;

// S2DDVector& ClientPositionFromScreen( S2DDVector& vScreen ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ClientPositionFromScreen) ;

// SGLError PostUpdate( const SGLImageRect* pUpdate = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_PostUpdate) ;

// SGLError UpdateWindow( Window::UpdateParameter * pUpdate ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_UpdateWindow) ;

// SGLError ProcessUserInput( int64_t msecTimeout = 1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ProcessUserInput) ;

// SGLError PostRenderingThread( SSystem::SProcedure * pProc, PostThreadType postType ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_PostRenderingThread) ;

// SGLError PostUIThread( SSystem::SProcedure * pProc ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_PostUIThread) ;

// bool IsWindowActive( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_IsWindowActive) ;

// SGLError SetWindowCaption( const wchar_t * pszWindowName ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetWindowCaption) ;

// SGLError ShowCursor( bool fShow ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ShowCursor) ;

// bool IsShowCursor( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_IsShowCursor) ;

// SGLError SetCursor( const wchar_t * pszCursorID ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetCursor) ;

// SGLError MoveCursorPosition
//		( int32_t xPos, int32_t yPos, int idMouse = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_MoveCursorPosition) ;

// SGLError GetCursorPosition
//		( SGLPoint& ptCursor, int idMouse = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetCursorPosition) ;

// int GetMonitorFrequency( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetMonitorFrequency) ;

// SGLError AttachMenu( WindowMenu * pMenu ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_AttachMenu) ;

// SGLPaintInterface * SetPaintInterface( SGLPaintInterface * pPaint ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetPaintInterface) ;

// SGLPaintInterface * SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetDirectPaintInterface) ;

// SGLTimerInterface * SetTimerInterface( SGLTimerInterface * pTimer ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetTimerInterface) ;

// SGLMouseInterface * SetMouseInterface( SGLMouseInterface * pMouse ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetMouseInterface) ;

// SGLMouseInterface * SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetDirectMouseInterface) ;

// SGLError CaptureMouse( int idMouse = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_CaptureMouse) ;

// SGLError ReleaseMouse( int idMouse = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ReleaseMouse) ;

// SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetKeyInterface) ;

// SGLCharInputInterface * SetCharInputInterface( SGLCharInputInterface * pChar ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetCharInputInterface) ;

// SGLCommandInterface * SetCommandInterface( SGLCommandInterface * pCmd ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_SetCommandInterface) ;

// RenderContext* GetRenderContext( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_GetRenderContext) ;

// void ReleaseRenderContext( RenderContext* context ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Window_ReleaseRenderContext) ;


#endif
