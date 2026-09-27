
//////////////////////////////////////////////////////////////////////////////
// スクリプトインターフェース＋ウィンドウインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSWindow	: public ECSSprite
{
public:
	// 構築関数
	ECSWindow( void ) ;
	// 消滅関数
	virtual ~ECSWindow( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSWindow, ECSSprite )

public:
	typedef	LRESULT (__stdcall *PFUNC_PROCEDURE)( void * pInstance ) ;
	// ウィンドウインターフェース
	class	EInterface : public EWindowSpriteInterface
	{
	public:
		// 構築関数
		EInterface( void ) ;
		// 消滅関数
		virtual ~EInterface( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( EInterface, EWindowSpriteInterface )
	public:
		ECSWindow *		m_pWnd ;
		HCURSOR			m_hArrow ;
		bool			m_fProcMsg ;
	public:
		// 更新領域を再描画
		virtual void Refresh( void ) ;
	public:
		// ウィンドウプロシージャ
		virtual LRESULT WindowProc
			( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		// マウスカーソルを設定する
		virtual bool OnSetCursor( int xPos, int yPos ) ;
		// マウスカーソルを設定する
		virtual ESLError SetMouseCursor( const wchar_t * pwszID ) ;
	} ;
	// ウィンドウスレッド
	class	EThread	: public EGLSThread
	{
	public:
		// 構築関数
		EThread( void ) ;
		// 消滅関数
		virtual ~EThread( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( EThread, EGLSThread )
	protected:
		ECSWindow *	m_pWnd ;
		HANDLE		m_hEventCreated ;
	public:
		// ウィンドウ作成
		ESLError CreateDisplay( ECSWindow * pWnd ) ;
	protected:
		// スレッド関数
		virtual DWORD ThreadProc( void ) ;
	} ;

protected:
	ECSReference		m_refParentWindow ;		// 親ウィンドウ参照（セーブ用）

	enum	CreateStatus
	{
		statusUncreated,
		statusCreateDisplay,
		statusCreateWindow,
	} ;
	CreateStatus		m_statusCreated ;
	ECSContext *		m_pContext ;			// 実行コンテキスト
	ECSEnvironment *	m_pEnv ;				// 実行環境
	EGameWindow *		m_pwndDisplay ;			// ウィンドウ
	EInterface *		m_pInterface ;			// インターフェース
	EThread				m_wndThread ;			// スレッド
	ECSInputFilter *	m_pInputFilter ;		// 関連付けられた入力フィルタ

	ECSContext *		m_pCallbackContext ;	// スクリプトコールバック実行用コンテキスト

	EString				m_strWindowName ;		// ウィンドウ名
	EGameWindow::CooperationLevel
						m_fCooperationLevel ;	// 協調レベル
	unsigned int		m_fOptionFunctions ;	// 機能フラグ
	POINT				m_posInitWindow ;		// ウィンドウ初期座標
	SIZE				m_sizeInitWindow ;
	bool				m_fInitChangeModeFlag ;	// 初期画面モード
	unsigned int		m_nInitChangeModeFlag ;
	EGL_SIZE			m_sizeDisplay ;			// ウィンドウサイズ
	unsigned int		m_nBitsPerPixel ;		// ビット深度
	unsigned int		m_nFrequency ;			// 周波数
	HICON				m_hMainIcon ;			// アイコン
	bool				m_fShowCursor ;			// カーソル表示
	E3DSDisplayPlugin::I3DImageView *
						m_pivView3D ;			// 立体視表示用インターフェース

	struct	CHANGE_DISPLAY_SIZE
	{
		ECSWindow *		pWnd ;
		ESLError		errResult ;
		ESLEventObject	eventDone ;
		unsigned int	nWidth ;
		unsigned int	nHeight ;
		unsigned int	nBitsPerPixel ;
		unsigned int	nFrequency ;
	} ;
	struct	CHANGE_COOPERATION_LEVEL
	{
		ECSWindow *		pWnd ;
		ESLError		errResult ;
		ESLEventObject	eventDone ;
		EGameWindow::CooperationLevel
						fCooperationLevel ;
	} ;
	struct	SET_OPTIONAL_FUNC_FLAG
	{
		ECSWindow *		pWnd ;
		ESLEventObject	eventDone ;
		unsigned int	flags ;
	} ;
	static LRESULT __stdcall
		CallOnWinThread_ChangeDisplaySize( void * pInstance ) ;
	static LRESULT __stdcall
		CallOnWinThread_ChangeCooperationLevel( void * pInstance ) ;
	static LRESULT __stdcall
		CallOnWinThread_SetOptionalFuncFlag( void * pInstance ) ;

	struct	CREATE_WINDOW
	{
		ECSWindow *		pWnd ;
		ESLError		errResult ;
		HWND			hwndParent ;
		ESLEventObject	eventDone ;
	} ;
	struct	CLOSE_WINDOW
	{
		ECSWindow *		pWnd ;
		ESLEventObject	eventDone ;
	} ;
	struct	SET_LAYERED_WINDOW
	{
		ECSWindow *		pWnd ;
		bool			fLayered ;
		ESLEventObject	eventDone ;
	} ;
	struct	SET_WINDOW_LAYOUT
	{
		ECSWindow *		pWnd ;
		int				nFlags ;
		int				xPos ;
		int				yPos ;
		ESLEventObject	eventDone ;
	} ;
	static LRESULT __stdcall
		CallOnWinThread_CreateWindow( void * pInstance ) ;
	static LRESULT __stdcall
		CallOnWinThread_CloseWindow( void * pInstance ) ;
	static LRESULT __stdcall
		CallOnWinThread_SetLayeredWindow( void * pInstance ) ;
	static LRESULT __stdcall
		CallOnWinThread_SetWindowLayout( void * pInstance ) ;

	struct	THREAD_LOCAL_DATA
	{
		DWORD				dwLockedCount ;
		DWORD				dwQuickLocked ;
		THREAD_LOCAL_DATA *	ptldNext ;

		THREAD_LOCAL_DATA( void )
		{
			dwLockedCount = 0 ;
			dwQuickLocked = 0 ;
			ptldNext = NULL ;
		}
	} ;
	DWORD				m_dwTlsIndex ;			// スクリプト同期用 TLS インデックス
	THREAD_LOCAL_DATA *	m_ptldListFirst ;
	ECSWindow *			m_pwndPrevList ;
	ECSWindow *			m_pwndNextList ;

	DWORD				m_dwBGFrameFlags ;
	DWORD				m_rgbBGFrameColor ;
	ECSReference		m_refFrameTile ;
	ECSReference		m_refFrameLeft ;
	ECSReference		m_refFrameRight ;
	ECSReference		m_refFrameUpper ;
	ECSReference		m_refFrameUnder ;

	static LONG			m_nTotalWindowCount ;	// 全 ECSWindow 数

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSWindow *	pBackLink ;
	} ;
	struct	PLUGIN_WINDOW
		: public PLUGIN_OBJECT_HEADER, public ECS_WINDOW_INTERFACE { } ;
	PLUGIN_WINDOW *	m_ppiw ;		// プラグイン用インターフェース

public:
	// ウィンドウ作成
	virtual ESLError CreateDisplay
		( const char * pszWindowName,
			EGameWindow::CooperationLevel fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0,
			unsigned int nFrequency = 0, HICON hIcon = NULL ) ;
	virtual ESLError CreateDisplayWindow
		( const char * pszWindowName,
			unsigned int nWidth, unsigned int nHeight,
			EWindowSpriteInterface * pwndParent = NULL, HICON hIcon = NULL ) ;
	virtual ESLError CreateDisplayWindow
		( const char * pszWindowName,
			unsigned int nWidth, unsigned int nHeight,
			ECSWindow * pParentWnd = NULL, HICON hIcon = NULL ) ;
	// インターフェースのみを作成する
	virtual ESLError CreateInterface
		( DWORD fdwFormat, unsigned int nWidth,
			unsigned int nHeight, unsigned int nBitsPerPixel ) ;
	// ウィンドウを閉じる
	virtual void CloseDisplay( void ) ;
protected:
	// m_pwndDisplay, m_pInterface を作成して初期設定する
	ESLError PrepareDisplayWindow( bool fOnlyInterface = false ) ;
	// EGameWindow を作成
	ESLError CreateDisplayWindow( HWND hwndParent ) ;
public:
	// ウィンドウサイズ変更
	virtual ESLError ChangeDisplaySize
		( unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0, unsigned int nFrequency = 0 ) ;
	virtual ESLError ChangeWindowSize
		( unsigned int nWidth, unsigned int nHeight ) ;
	// 協調レベルを変更
	virtual ESLError ChangeCooperationLevel
		( EGameWindow::CooperationLevel fCooperationLevel ) ;
	// オプショナル機能フラグを設定
	virtual void SetOptionalFuncFlag( unsigned int flagsOptionalFunc ) ;
	// 画面モード変更フラグを設定
	virtual ESLError SetChangeDisplayModeFlag( unsigned int nWithChangeMode ) ;
	// 画面外フレーム画像を設定
	virtual ESLError SetExteriorBackgroundFrame
		( DWORD dwFlags, DWORD rgbColor, ECSResource * pTile,
			ECSResource * pLeft = NULL, ECSResource * pRight = NULL,
			ECSResource * pUpper = NULL, ECSResource * pUnder = NULL ) ;
	// ステレオ立体視表示インターフェースを設定する
	void Set3DViewDisplay( E3DSDisplayPlugin::I3DImageView * pivView3D ) ;
	// ステレオ立体視インターフェースを取得する
	E3DSDisplayPlugin::I3DImageView * Get3DViewDisplay( void ) const
		{
			return	m_pivView3D ;
		}
	// カーソル表示設定
	void ShowCursor( bool fShow ) ;
	// カーソル表示状態取得
	bool IsShowCursor( void ) const
		{
			return	m_fShowCursor ;
		}
	// 初期座標設定
	void InitWindowPosition
		( int xPos = 0x80000000, int yPos = 0x80000000,
			int nWidth = 0x80000000, int nHeight = 0x80000000 ) ;
	// ウィンドウの通常時座標取得
	bool GetNormalWindowPosition( EGL_POINT & ptWindow, EGL_SIZE & sizeWindow ) const ;
	// 実行コンテキスト関連付け
	void AttachContext( ECSContext * pContext ) ;
	// 実行コンテキスト取得
	ECSContext * GetContext( void ) const
		{
			return	m_pContext ;
		}
	// 環境関連付け
	void AttachEnvironment( ECSEnvironment * pEnv ) ;
	// 環境取得
	ECSEnvironment * GetEnvironment( void ) const
		{
			return	m_pEnv ;
		}
	// ウィンドウを取得する
	EGameWindow * GetWindow( void ) const ;
	// ウィンドウインターフェースを取得する
	EInterface * GetInterface( void ) const ;
	// ウィンドウメッセージを処理する
	void HandleWindowMessage
		( int nCount = 0x20, DWORD dwTimeout = INFINITE ) const ;
	// ウィンドウスレッドから関数を呼び出す
	ESLError ProcedureOnWindowThread
		( PFUNC_PROCEDURE pfnProc, void * pInstance,
			LRESULT * pResult, bool fAsync = false ) const ;

public:
	// コールバック関数実行用コンテキストを取得する
	ECSContext * GetCallbackContext( void ) ;

public:
	// スレッド同期
	ESLError Lock( DWORD dwTimeout = INFINITE ) ;
	ESLError Unlock( void ) ;
	// 描画更新制御
	void FreezePaint( void ) ;
	void UnfreezePaint( void ) ;
	// スレッド同期（スクリプト用）
	ESLError QuickLockOnScript( void ) ;
	ESLError QuickUnlockOnScript( void ) ;
	bool IsQuickLockedOnScript( void ) const ;
	// スレッドローカルデータの解放
	void FreeThreadLocalData( void ) ;
	static void FreeAllThreadLocalData( void ) ;
	// ウィンドウリストにエントリ追加
	void AddToWidowList( void ) ;
	// ウィンドウリストからエントリ削除
	void RemoveFromWindowList( void ) ;
	// リストの最初のウィンドウ取得
	ECSWindow * GetFirstWindowList( void ) const
		{
			return	ECSSprite::m_pMainWnd ;
		}
	// 次のウィンドウ
	ECSWindow * GetNextWindowList( void ) const
		{
			return	m_pwndNextList ;
		}
	// 前のウィンドウ
	ECSWindow * GetPrevWindowList( void ) const
		{
			return	m_pwndPrevList ;
		}

public:
	enum	MessageBoxStyle
	{
		msgboxStyleOk,
		msgboxStyleOkCancel,
		msgboxStyleYesNo,
		msgboxStyleYesNoCancel,
		msgboxStyleRetryCancel,
		msgboxStyleCount,
	} ;
	enum	MessageBoxResult
	{
		msgboxResultOk,
		msgboxResultCancel,
		msgboxResultYes,
		msgboxResultNo,
		msgboxResultRetry,
	} ;
	// メッセージボックス表示
	int MessageBox
		( const char * pszMessage,
			const char * pszCaption, int nStyle ) ;

protected:
	struct	MESSAGE_BOX
	{
		ECSWindow *		pWnd ;
		int				nMBResult ;
		EString			strCaption ;
		EString			strMessage ;
		int				nMBStyle ;
		ESLEventObject	eventDone ;
	} ;
	static LRESULT __stdcall
		CallOnWinThread_MessageBox( void * pInstance ) ;

public:
	// レイヤードウィンドウ設定
	ESLError SetLayeredWindow( bool fLayeredWindow ) ;
	// ウィンドウ表示・非表示
	ESLError ShowWindow( int nShowCmd ) ;
	// レイアウト設定
	ESLError SetWindowLayout( int nFlags, int xPos, int yPos ) ;

protected:
	// ウィンドウオブジェクトを作成する
	virtual EGameWindow * OnCreateWindowObject( void ) ;
	// ウィンドウインターフェースオブジェクトを作成する
	virtual EInterface * OnCreateInterface( void ) ;

	// ウィンドウ内部の画像表示位置情報を更新する
	void UpdateImagePosition( void ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSWindow::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[37] ;
	static const PFUNC_CALL	m_pfnCallFunc[36] ;
	// メンバ関数（GameWindow 系）
	ESLError Call_CreateDisplay
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseDisplay
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetOptionalFuncFlag
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetOptionalFuncFlag
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ChangeCooperationLevel
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ChangeDisplaySize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetChangeDisplayModeFlag
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetStereoDisplayMode
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsSupportedStereoDisplayMode
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDisplaySize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_UpdateWindow
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ProcessUserInput
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsWindowActive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InitWindowPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetNormalWindowPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPhysicalMonitorSize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetExteriorBackgroundFrame
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数（汎用ウィンドウ）
	ESLError Call_MessageBox
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateWindow
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ChangeWindowSize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetLayeredWindow
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetWindowLayout
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数（WindowSpriteInterface系）
	ESLError Call_EnableCommandQueue
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushCommandQueue
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_QueueCommand
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCommand
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CallMouseMove
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Lock
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Unlock
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FreezePaint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_UnfreezePaint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SyncTimePaint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AsyncTimePaint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数（ECSWindow 拡張）
	ESLError Call_ShowCursor
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsShowCursor
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static HWND __stdcall PIC_GetWindow( ECS_WINDOW_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_CreateDisplay
		( ECS_WINDOW_INTERFACE * instance, 
			const char * pszWindowName, int fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel, unsigned int nFrequency ) ;
	static void __stdcall PIC_CloseDisplay( ECS_WINDOW_INTERFACE * instance ) ;
	static unsigned int __stdcall PIC_GetOptionalFuncFlag
						( ECS_WINDOW_INTERFACE * instance ) ;
	static void __stdcall PIC_SetOptionalFuncFlag
		( ECS_WINDOW_INTERFACE * instance, unsigned int nFlags ) ;
	static ESLError __stdcall PIC_ChangeCooperationLevel
		( ECS_WINDOW_INTERFACE * instance, int fCooperationLevel ) ;
	static ESLError __stdcall PIC_ChangeDisplaySize
		( ECS_WINDOW_INTERFACE * instance,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel, unsigned int nFrequency ) ;
	static void __stdcall PIC_GetDisplaySize
		( ECS_WINDOW_INTERFACE * instance, SIZE * pDisplaySize ) ;
	static void __stdcall PIC_UpdateWindow( ECS_WINDOW_INTERFACE * instance ) ;
	static int __stdcall PIC_IsWindowActive( ECS_WINDOW_INTERFACE * instance ) ;
	static void __stdcall PIC_EnableCommandQueue
		( ECS_WINDOW_INTERFACE * instance, int fQueueCommand ) ;
	static void __stdcall PIC_FlushCommandQueue
		( ECS_WINDOW_INTERFACE * instance, int fQueueCommand ) ;
	static ESLError __stdcall PIC_GetCommand
		( ECS_WINDOW_INTERFACE * instance,
			ECS_WINDOW_INTERFACE::WndCommand * pCmd,
						DWORD dwTimeout, int fRemove ) ;
	static void __stdcall PIC_QueueCommand
		( ECS_WINDOW_INTERFACE * instance,
			const wchar_t * pwszID, long int nNotification, long int nParameter ) ;
	static void __stdcall PIC_CallMouseMove( ECS_WINDOW_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_Lock
		( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout ) ;
	static void __stdcall PIC_Unlock( ECS_WINDOW_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_SyncTimePaint
		( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout ) ;
	static void __stdcall PIC_AsyncTimePaint( ECS_WINDOW_INTERFACE * instance ) ;
	static void __stdcall PIC_ShowCursor
		( ECS_WINDOW_INTERFACE * instance, int fShow ) ;
	static int __stdcall PIC_IsShowCursor( ECS_WINDOW_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_ProcedureOnWindowThread
		( ECS_WINDOW_INTERFACE * instance,
			PFUNC_PROCEDURE pfnProc,
			void * pInstance, LRESULT * pResult, int fAsync ) ;

	friend	EThread ;
	friend	EInterface ;
	friend	ECSInputFilter ;
} ;

inline ESLError ECSSprite::QuickLock( void )
	{
		if ( m_pMainWnd != NULL )
		{
			return	m_pMainWnd->QuickLockOnScript( ) ;
		}
		return	eslErrGeneral ;
	}
inline void ECSSprite::QuickUnlock( void )
	{
		if ( m_pMainWnd != NULL )
		{
			m_pMainWnd->QuickUnlockOnScript( ) ;
		}
	}
inline bool ECSSprite::IsLocked( void )
	{
		return	m_pMainWnd && m_pMainWnd->IsQuickLockedOnScript() ;
	}
