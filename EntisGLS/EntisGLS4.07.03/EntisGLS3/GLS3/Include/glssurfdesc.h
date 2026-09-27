
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2017 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#if	!defined(__SURFDESC_H__)
#define	__SURFDESC_H__

class	EFormResourceManager ;


//////////////////////////////////////////////////////////////////////////////
// スプライトコマンド
//////////////////////////////////////////////////////////////////////////////

class	EWndSpriteCmd	: public	ESLObject
{
public:
	enum	Priority
	{
		priorityLowest		= -10,
		priorityLow			= -5,
		priorityNormal		= 0,
		priorityHigh		= 5,
		priorityHighest		= 10,
		priorityCritical	= 0x7FFFFFFF,
	} ;
	int			m_nPriority ;
	EWideString	m_wstrID ;
	EWideString	m_wstrFullID ;
	long int	m_nNotification ;
	long int	m_nParameter ;
public:
	// クラス情報
	DECLARE_CLASS_INFO( EWndSpriteCmd, ESLObject )
	// 構築関数
	EWndSpriteCmd( void ) : m_nPriority(priorityNormal) { }
	EWndSpriteCmd( const EWndSpriteCmd & cmd )
		: m_nPriority(priorityNormal),
			m_wstrID( cmd.m_wstrID ),
			m_wstrFullID( cmd.m_wstrFullID ),
			m_nNotification( cmd.m_nNotification ),
			m_nParameter( cmd.m_nParameter ) { }
	// 消滅関数
	virtual ~EWndSpriteCmd( void ) { }
	// 代入演算子
	const EWndSpriteCmd & operator = ( const EWndSpriteCmd & cmd )
		{
			m_nPriority = cmd.m_nPriority ;
			m_wstrID = cmd.m_wstrID ;
			m_wstrFullID = cmd.m_wstrFullID ;
			m_nNotification = cmd.m_nNotification ;
			m_nParameter = cmd.m_nParameter ;
			return	*this ;
		}
} ;


//////////////////////////////////////////////////////////////////////////////
// 画面入出力インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ESpriteInterface	: public	ESpriteServer
{
public:
	// 構築関数
	ESpriteInterface( void ) ;
	// 消滅関数
	virtual ~ESpriteInterface( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESpriteInterface, ESpriteServer )

public:
	// 機能フラグ
	enum	FunctionFlag
	{
		ffTabStop			= 0x0001,
		ffGroup				= 0x0002,
		ffTimer				= 0x0004,
		ftHitTransparency	= 0x0008,
		ftModalFirst		= 0x0010,
		ftModalEnd			= 0x0020,
	} ;

protected:
	EWideString			m_wstrID ;			// 識別名
	DWORD				m_dwFlags ;			// 機能フラグ
	bool				m_fEnabled ;		// 有効化フラグ
	bool				m_fEnabledKeyInput ;
	bool				m_fEnabledMouseWheel ;

	ESpriteInterface *	m_pFocus ;			// フォーカスを持っている
	ESpriteInterface *	m_pMouseFocus ;		// フォーカスを持っている
	ESpriteInterface *	m_pCaptured ;		// キャプチャーしている

	EWStrTagArray<ESpriteInterface>
						m_wstaItems ;		// 画面アイテム
	int					m_nNextPriority ;

	EFormResourceManager *
						m_pfrmRsrc ;		// リソースマネージャ

public:	// スキン管理用
	// 画像バッファ消去
	virtual void DeleteImage( void ) ;
	// リソースマネージャへの参照を取得
	EFormResourceManager * GetResourceManager( void ) const
		{
			return	m_pfrmRsrc ;
		}

public:
	// 識別名取得
	const EWideString & ID( void ) const
		{
			return	m_wstrID ;
		}
	// 識別名設定
	void SetID( const wchar_t * pwszID ) ;
	// 有効化・無効化
	virtual void Enable( bool fEnable ) ;
	virtual bool IsEnabled( void ) const ;
	// 文字列取得・設定
	virtual const wchar_t * GetSpriteText( void ) ;
	virtual void SetSpriteText( const wchar_t * pwszText ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
public:
	// メッセージ処理（マウスが上に乗っているかキャプチャーしているもののみ）
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnSetCursor( int xPos, int yPos ) ;
	virtual bool OnMouseWheel
		( UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	// タイマー処理（タイマーフラグを持っているもののみ）
	virtual bool OnTimer( UINT nEventID ) ;
	// メッセージ処理（フォーカスを持っているアイテムのみ）
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// コマンド処理
	virtual void OnCommand
		( ESpriteInterface * pItem,
			long int nNotification = 0, long int nParameter = 0,
			int nPriority = EWndSpriteCmd::priorityNormal, bool fOverwrite = false ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;

public:
	// マウスメッセージのキャプチャー
	virtual void SetCapture( ESpriteInterface * pSprite = NULL ) ;
	virtual void ReleaseCapture( ESpriteInterface * pSprite = NULL ) ;
	ESpriteInterface * GetCapture( void ) const
		{
			return	m_pCaptured ;
		}
protected:
	// マウスのキャプチャーがリリースされた
	virtual void OnCaptureReleased( void ) ;

protected:
	// 指定されたアイテムの指標を取得する
	int GetSpriteItemIndex( ESpriteInterface * pItem ) ;
public:
	// フォーカスを取得する
	ESpriteInterface * GetFocus( void ) const
		{
			return	m_pFocus ;
		}
	// フォーカスを設定する
	void SetFocus( ESpriteInterface * pSprite ) ;
	// フォーカスを解除する
	void KillFocus( ESpriteInterface * pSprite = NULL ) ;
	// フォーカスを移動する
	bool MoveFocus( bool fNext = true ) ;
	// 同じグループに属する次のアイテムを取得する
	ESpriteInterface * GetNextItemOnGroup
		( ESpriteInterface * pItem, bool fNext = true ) ;
public:
	// フォーカスを取得した
	virtual void OnSetFocus( void ) ;
	// フォーカスを奪われた
	virtual void OnKillFocus( void ) ;

public:
	// マウスカーソルを設定する
	virtual ESLError SetMouseCursor( const wchar_t * pwszID ) ;
	// 効果音を再生する
	virtual ESLError PlaySoundEffect
		( const wchar_t * pwszID, bool fRepeat = false ) ;

public:
	// 現在の垂直スクロール位置を取得
	virtual int GetVertScrollPos( void ) const ;
	// 現在の垂直スクロール位置を設定
	virtual void SetVertScrollPos( int nPos ) ;
	// 垂直スクロールの範囲を取得
	virtual int GetVertScrollRange( void ) const ;
	// 垂直スクロールの範囲を設定
	virtual void SetVertScrollRange( int nRange ) ;
	// 現在の水平スクロール位置を取得
	virtual int GetHorzScrollPos( void ) const ;
	// 現在の水平スクロール位置を設定
	virtual void SetHorzScrollPos( int nPos ) ;
	// 水平スクロールの範囲を設定
	virtual int GetHorzScrollRange( void ) const ;
	// 水平スクロールの範囲を設定
	virtual void SetHorzScrollRange( int nRange ) ;

public:
	// 指定座標にヒットしているスプライトを取得
	virtual ESpriteInterface * GetSpriteAtPoint
		( int xPos, int yPos,
			ESpriteInterface * pFirst = NULL, bool fEnabledOnly = false ) ;
	// ローカル座標をウィンドウのクライアント座標に変換する
	virtual ESLError LocalToWindowClient( EGL_POINT & ptLocal ) ;
	// ウィンドウインターフェースを取得する
	EWindowSpriteInterface * GetWindowInterface( void ) ;

public:
	// スプライトを追加
	virtual void AddSprite( int nPriority, ESprite * pSprite ) ;
	// スプライトを分離
	virtual ESLError DetachSprite( ESprite * pSprite ) ;
	// 全てのスプライトを分離
	virtual void DetachAllSprite( void ) ;
	// スプライトを削除
	virtual ESLError RemoveSprite( ESprite * pSprite ) ;
	// 全てのスプライトを削除
	virtual void RemoveAllSprite( void ) ;

public:
	// 機能フラグを取得する
	DWORD GetFunctionFlags( void ) const
		{
			return	m_dwFlags ;
		}
	// 機能フラグを設定する
	virtual void SetFunctionFlags( DWORD dwFlags ) ;
	// 画面アイテムの基準プライオリティを設定する
	void SetBaseItemPriority( int nPriority ) ;
	// 画面アイテムを追加する
	void AddSpriteItem
		( const wchar_t * pwszID, ESpriteInterface * pSprite ) ;
	// 画面アイテムを取得する
	ESpriteInterface * GetSpriteItemAs
		( const wchar_t * pwszID, bool fChild = true ) ;

public:
	// アイテムの文字列取得
	ESLError GetSpriteItemText
		( const wchar_t * pwszID, EWideString & wstrText ) ;
	// アイテムの文字列設定
	ESLError SetSpriteItemText
		( const wchar_t * pwszID, const wchar_t * pwszText ) ;
	// アイテムのフォント設定
	ESLError SetSpriteItemFontFace
		( const wchar_t * pwszID, const wchar_t * pwszFont ) ;
	// ボタンのチェック状態を取得
	ESLError IsSpriteButtonChecked( const wchar_t * pwszID, int & nStatus ) ;
	// ボタンをチェックする
	ESLError CheckSpriteButton( const wchar_t * pwszID, bool fCheck ) ;
	// キー入力を有効・無効化する
	void EnableKeyInput( bool fKeyInput ) ;
	// キー入力が有効か判定する
	bool IsEnabledKeyInput( void ) const
		{
			return	m_fEnabledKeyInput ;
		}
	// マウスホイール入力を有効・無効化する
	void EnableMouseWheel( bool fWheel ) ;
	// マウスホイール入力が有効か判定する
	bool IsEnabledMouseWheel( void ) const
		{
			return	m_fEnabledMouseWheel ;
		}

	friend	EFormResourceManager ;

} ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ画面入出力インターフェース
//////////////////////////////////////////////////////////////////////////////

class	EWindowSpriteInterface
			: public ESpriteInterface,
				public EWindowInterface, public EGLDrawImage::INotify
{
public:
	// 構築関数
	EWindowSpriteInterface( void ) ;
	// 消滅関数
	virtual ~EWindowSpriteInterface( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2
		( EWindowSpriteInterface, ESpriteInterface, EWindowInterface )

protected:
	// ウィンドウメッセージ
	enum	WindowMessage
	{
		wmProcessMessage	= WM_USER,
		wmCallProcedure,
		wmNotifyLayout,
	} ;
	typedef	LRESULT (__stdcall *PFUNC_PROCEDURE)( void * pInstance ) ;
	// 非標準ウィンドウインターフェース
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
	EDisplayMode
				m_dmMonitorAPIs ;
	HMODULE		m_hUser32 ;				// USER32.DLL モジュール
	API_TrackMouseEvent
				m_apiTrackMouseEvent ;	// TrackMouseEvent 関数
	API_UpdateLayeredWindow
				m_apiUpdateLayeredWindow ;	// UpdateLayeredWindow 関数
	bool		m_fMouseLeaved ;		// マウスがウィンドウの外にあるか？
	bool		m_fLayeredWindow ;		// レイヤードウィンドウモードか？
	bool		m_fPendingUpdateWindow ;	// レイヤードウィンドウの更新待ち状態
	bool		m_fNoInvalidateWindow ;	// スプライトの更新通知でウィンドウを再描画しない
	bool		m_fProcMsg ;			// メッセージ処理中
	BLENDFUNCTION	m_bfLayeredWindow ;
	HANDLE		m_hMutex ;				// 画面排他アクセス用
//	CRITICAL_SECTION	m_cs ;
	DWORD		m_idMsgHandlerThread ;	// メッセージハンドラスレッドID
	long int	m_nMsgHandlerLocked ;	// メッセージハンドラ Lock カウント
	long int	m_fMsgHandlerUnlocked ;	// メッセージハンドラの Lock は解除されているか？

	HANDLE		m_hSyncTimePaint ;		// 画面描画同期処理用イベント
	HANDLE		m_hWaitTimePaint ;
	DWORD		m_dwLastTimerTick ;

	HANDLE		m_hPaintSignal ;

	EWindowSpriteInterface *
				m_pSyncTarget ;			// 同期ターゲット（同期関数はこちらを使う）

	bool		m_fQueueCommand ;		// コマンドをキューに溜めるか？
	HANDLE		m_hQueCmdEvent ;		// コマンドが1つでもキューにある
	EObjArray<EWndSpriteCmd>
				m_queCommand ;			// コマンドキュー

	EInputFilter *	m_pFilter ;			// 入力フィルタ

	bool		m_fImageStretching ;	// 画面表示を伸縮させるか？
	EGLPoint	m_ptScreenBase ;
	EGLSize		m_szScreenStretched ;

	bool		m_fStretchingByCPU ;	// CPU で補間拡大する
	EGLImage	m_imgStretchBuffer ;
	enum
	{
		countMaxDrawStretchCPUs	= 32,
	} ;
	HEGL_DRAW_IMAGE	m_hDrawStretch[countMaxDrawStretchCPUs] ;

	class	ParallelStretchDraw	: public SSystem::SParallelProcedure
	{
	protected:
		EGLImage&		m_imgStretchBuffer ;
		EGL_DRAW_PARAM	m_dp ;
		EGL_IMAGE_AXES	m_iax ;
		EGLImage *		m_pSrcImage ;
		int				m_yNextLine ;
		int				m_yBlockHeight ;
	public:
		struct	Param
		{
			HEGL_DRAW_IMAGE	hDraw ;
		} ;
	public:
		// 構築関数
		ParallelStretchDraw
			( EGLImage& imgStretchBuffer,
				EGLImage * pSrcImage, int yBlockHeight ) ;
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;

	long		m_countNullDraw ;

	EGLDrawImage *				m_pDrawImage ;	// 画面伸縮表示用
	struct IDirectDrawSurface *	m_pddsVRAM ;
	EGLSize						m_sizeVRAM ;

	ESpriteInterface *			m_pAttachedView ;	// 表示リダイレクトアイテム
	E3DSDisplayPlugin::I3DImageView *
								m_pivView3D ;		// 立体視表示用インターフェース
	atomic_int_t				m_nFreezePaint ;

	ENumArray<HWND>	m_lstRelativeLayout ;

	enum	LayoutFlag
	{
		layoutNothing	= 0,
		offsetClient	= 1,
		dockingLeft		= 2,
		dockingRight	= 3,
		dockingUpper	= 4,
		dockingUnder	= 5,
		dockingMask		= 0x0F,
		alignLeft		= 0x00,
		alignTop		= 0x00,
		alignCenter		= 0x10,
		alignRight		= 0x20,
		alignBottom		= 0x20,
		alignAccording	= 0x30,
		alignMask		= 0x30,
		alignTypeClient	= 0x00,
		alignTypeWindow	= 0x40,
	} ;
	long int	m_flagLayout ;
	EGL_POINT	m_ptLayoutOffset ;

public:
	// 領域外壁紙
	struct	BACKGROUND_IMAGE
	{
		DWORD			dwFlags ;
		EGL_PALETTE		rgbBG ;
		PEGL_IMAGE_INFO	pTile ;
		PEGL_IMAGE_INFO	pLeft ;
		PEGL_IMAGE_INFO	pRight ;
		PEGL_IMAGE_INFO	pUpper ;
		PEGL_IMAGE_INFO	pUnder ;
	} ;
	enum	BackgroundFrameFlag
	{
		bgfFillColor	= 0x00000001,
		bgfStretch		= 0x00000002,
	} ;

protected:
	BACKGROUND_IMAGE	m_bgiFrame ;			// 領域外壁紙

public:	// フィルタの関連付け
	// フィルター設定
	EInputFilter * GetInputFilter( void ) const
		{
			return	m_pFilter ;
		}
	// フィルター設定
	void SetInputFilter( EInputFilter * pFilter ) ;
	// 表示用リダイレクトスプライトの関連付け
	void AttachView( ESpriteInterface * pView ) ;
	// 表示用リダイレクトスプライトの解除
	void DetachView( ESpriteInterface * pView ) ;
	// 表示用立体視インターフェースの関連付け
	void Attach3DViewDisplay( E3DSDisplayPlugin::I3DImageView * pivView3D ) ;
	// 表示用立体視インターフェースを取得する
	E3DSDisplayPlugin::I3DImageView * GetAttached3DViewDisplay( void ) const
		{
			return	m_pivView3D ;
		}
	// EWindowSpriteInterface での描画抑制
	void AddNullficationDraw( void ) ;
	void ReleaseNullficationDraw( void ) ;
	// ウィンドウの更新領域通知抑制
	void ConntrolAutoUpdate( bool fNoUpdate ) ;
	bool GetConntrolAutoUpdate( void ) const
	{
		return	m_fNoInvalidateWindow ;
	}

public:
	// EWindow オブジェクトにアタッチされた
	virtual void OnAttachedWindow( EWindow * pAttachedWnd ) ;
	// ウィンドウプロシージャ
	virtual LRESULT WindowProc
		( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// メッセージ事前変換関数
	virtual int PreTranslateMessage( MSG & msg ) ;
	// メッセージ処理
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
public:
	// プラグイン出力用バッファ変換
	void ConvertToE3DSDisplayImageBuffer
		( E3DSDisplayPlugin::ImageBuffer & bufImage, PCEGL_IMAGE_INFO pImage ) ;
	// 画像描画
	void OnPaint( EWindow * pWnd, HDC hdc, bool fVSync = false ) ;
	void OnPaintImage
		( EWindow * pWnd, HDC hdc, EGLImage * pImage, bool fVSync = false ) ;
	ESLError DirectDrawImage
		( EWindow * pWnd, PEGL_IMAGE_INFO pImage, bool fVSync = true ) ;
public:
	// コマンド処理
	virtual void OnCommand
		( ESpriteInterface * pItem,
			long int nNotification = 0, long int nParameter = 0,
			int nPriority = EWndSpriteCmd::priorityNormal, bool fOverwrite = false ) ;
	// ウィンドウメッセージを処理する
	void HandleWindowMessage
		( int nCount = 0x20, DWORD dwTimeout = INFINITE ) const ;
	// ウィンドウスレッドから関数を呼び出す
	ESLError ProcedureOnWindowThread
		( PFUNC_PROCEDURE pfnProc, void * pInstance,
			LRESULT * pResult, bool fAsync = false ) const ;
public:
	// スプライト上の指定領域の更新通知
	virtual bool UpdateRect( EGL_RECT * pUpdateRect = NULL ) ;

public:
	// 有効化・無効化
	virtual void Enable( bool fEnable ) ;

public:
	// マウスメッセージのキャプチャー
	virtual void SetCapture( ESpriteInterface * pSprite ) ;
	virtual void ReleaseCapture( ESpriteInterface * pSprite = NULL ) ;

public:
	// コマンド待ち行列を有効化する
	void EnableCommandQueue( bool fQueueCommand = true ) ;
	// コマンド待ち行列を初期化する
	void FlushCommandQueue
		( bool fQueueCommand = true, int nPriority = 0x7FFFFFFF ) ;
	// コマンドを待ち行列に追加
	void QueueCommand
		( const wchar_t * pwszID,
			long int nNotification = 0, long int nParameter = 0,
			int nPriority = EWndSpriteCmd::priorityNormal, bool fOverwrite = false ) ;
	void QueueCommandObject
		( EWndSpriteCmd * pCmd, bool fOverwrite = false ) ;
	// 待ち行列からコマンドを取得
	ESLError GetCommand
		( EWndSpriteCmd & wscCmd, DWORD dwTimeout, bool fRemove = true ) ;
	ESLError GetCommandObject
		( EWndSpriteCmd *& pCmd, DWORD dwTimeout, bool fRemove = true ) ;
	// 待ち行列にコマンドがあるか？
	bool IsQueueCommand( void ) const ;

public:
	// マウス座標通知
	void CallMouseMove( void ) ;
	// スレッド排他処理
	virtual ESLError Lock( DWORD dwTimeout = INFINITE ) ;
	virtual ESLError Unlock( void ) ;
	// スレッド排他処理（メッセージハンドラ中の排他処理の解除）
	virtual void UnlockOnMsgHandler( void ) ;
	virtual void RelockOnMsgHandler( void ) ;
	// スレッド同期描画処理
	ESLError SyncTimePaint( DWORD dwTimeout ) ;
	void AsyncTimePaint( void ) ;
	// 描画完了同期
	ESLError WaitForDonePaint( DWORD dwTimeout ) ;
	// 同期ターゲットを設定する
	void AttachSyncObject( EWindowSpriteInterface * pSync ) ;
	// 描画更新制御
	void FreezePaint( void ) ;
	void UnfreezePaint( void ) ;

public:
	// 座標変換があるか？
	bool IsImageStretching( void ) const
		{
			return	m_fImageStretching ;
		}
	// 表示座標を取得する
	const EGLPoint & GetImageStretchingBase( void ) const
		{
			return	m_ptScreenBase ;
		}
	// 表示座標を取得する
	const EGLSize & GetImageStretchingSize( void ) const
		{
			return	m_szScreenStretched ;
		}
	// 座標変換設定
	void SetImageStretching
		( const EGL_POINT * pptBase = NULL,
				const EGL_SIZE * pszScreen = NULL ) ;
	// 画面表示のための描画オブジェクトを関連付ける
	void AttachDrawImageObject( EGLDrawImage * pDrawImage ) ;
	// 画面表示のための描画オブジェクトを取得する
	EGLDrawImage * GetAttachedDrawImageObject( void ) const
		{
			return	m_pDrawImage ;
		}
	// 伸縮描画を CPU で行うか
	bool IsImageStretchingByCPU( void ) const
		{
			return	m_fStretchingByCPU ;
		}
	// 伸縮描画を CPU で行うか設定する
	void SetImageStretchingByCPU( bool fStretchByCPU )
		{
			m_fStretchingByCPU = fStretchByCPU ;
		}
	// 座標変換
	void WindowToClient( EGL_POINT & pos ) ;
	void ClientToWindow( EGL_POINT & pos ) ;
	void ScreenToClient( EGL_POINT & pos ) ;
	void ClientToScreen( EGL_POINT & pos ) ;
	void WindowToClientSize( EGL_SIZE & sz ) ;
	void ClientToWindowSize( EGL_SIZE & sz ) ;

public:
	// 領域外フレーム画像取得
	const BACKGROUND_IMAGE & GetBackgroundFrameImage( void ) const
		{
			return	m_bgiFrame ;
		}
	// 領域外フレーム画像設定
	void SetBackgroundFrameImage( const BACKGROUND_IMAGE & bgiFrame )
		{
			m_bgiFrame = bgiFrame ;
		}

public: // DirectDraw 通知オーバーライド
	// DirectDraw オブジェクトが削除される前に呼び出される
	virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) ;
	// DirectDraw オブジェクトが作成された後に呼び出される
	virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) ;

public:
	// レイヤードウィンドウに設定
	ESLError SetLayeredWindow( bool fLayeredWindow = true ) ;
	// ウィンドウ透明度を設定
	ESLError SetLayeredWindowTransparency( unsigned int nTransparency ) ;
	// レイヤードウィンドウの表示更新
	ESLError UpdateLayeredWindow( void ) ;

public:
	// 画像からリージョン生成
	static HRGN CreateRegionFromImage
		( PEGL_IMAGE_INFO pImage, int nThreashold = 0x80 ) ;

public:
	// 関連レイアウトウィンドウ登録
	void AttachRelativeLayoutWindow( HWND hWnd ) ;
	// 関連レイアウトウィンドウ解除
	void DetachRelativeLayoutWindow( HWND hWnd ) ;
protected:
	// 関連レイアウトウィンドウに通知メッセージ送信
	void PostNotifyRelativeLayout( void ) ;

public:
	// レイアウト設定
	void SetWindowLayout( int nFlags, int xPos, int yPos ) ;
protected:
	// レイアウト反映
	void UpdateWindowLayout( HWND hRelativeWnd ) ;
	// レイアウト通知処理
	virtual void OnNotifyLayout( HWND hRelativeWnd ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// アニメーション画像スプライト
//////////////////////////////////////////////////////////////////////////////

class	EAnimationSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EAnimationSprite( void ) ;
	// 消滅関数
	virtual ~EAnimationSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EAnimationSprite, ESpriteInterface ) ;

protected:
	unsigned long int	m_nViewFrame ;
	unsigned long int	m_nCurrentSequence ;
	unsigned long int	m_nLoopCount ;
	unsigned long int	m_nRewindSequence ;
	unsigned long int	m_nTurnSequence ;
	unsigned long int	m_nAnimationDuration ;
	unsigned long int	m_nAnimationOffsetTime ;
	EGLAnimation *		m_pAnimation ;

	enum	AnimationFunctionFlag
	{
		animeNormal	= 0x01,			// 画像切り替えアニメ（EAnimationSprite）
		animeAction	= 0x02,			// 移動アニメ（ECSSprite）
		animeEffect	= 0x04,			// 特殊アニメ（ECSSuperSprite）
		animeFull	= 0x07,
	} ;
	static DWORD	m_dwAnimationFlags ;	// 有効なアニメーションフラグ

public:
	// アニメーション進行
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;
	// パラメータ複製
	virtual void CopyParameters( const EImageSprite * pSrc ) ;

public:
	// アニメーション画像スプライトを生成
	ESLError CreateAnimation( EGLAnimation * pAnimation ) ;
	// 画像バッファ消去
	virtual void DeleteImage( void ) ;
	// アニメーション画像取得
	EGLAnimation * GetAnimation( void ) const
		{
			return	m_pAnimation ;
		}
	// アニメーション任意回数再生
	ESLError BeginAnimation
		( unsigned long int nLoopCount = 1,
			unsigned long int nBeginFrame = 0,
			unsigned long int nAnimationTime = -1,
			unsigned long int nRewindSequence = 0,
			unsigned long int nTurnSequence = -1 ) ;
	// アニメーション停止
	ESLError EndAnimation( void ) ;
	// アニメーション中か？
	bool IsDuringAnimation( void ) const
		{
			return	(m_nLoopCount != 0) && (m_pAnimation != 0) ;
		}
	// 現在の表示シーケンス番号取得
	unsigned long int GetCurrentSequence( void ) const
		{
			return	m_nCurrentSequence ;
		}
	// 表示フレーム番号取得
	unsigned long int GetViewFrameIndex( void ) const
		{
			return	m_nViewFrame ;
		}

public:
	// 機能フラグを設定する
	virtual void SetFunctionFlags( DWORD dwFlags ) ;
	// アニメーションフラグを取得する
	static DWORD GetAnimationFlags( void )
		{
			return	m_dwAnimationFlags ;
		}
	// アニメーションフラグを設定する
	static void SetAnimationFlags( DWORD dwFlags )
		{
			m_dwAnimationFlags = dwFlags ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// フレーム表示スプライト
//////////////////////////////////////////////////////////////////////////////

class	EStaticFrameSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EStaticFrameSprite( void ) ;
	// 消滅関数
	virtual ~EStaticFrameSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EStaticFrameSprite, ESpriteInterface )

public:
	// フレームのタイプ
	enum	FrameType
	{
		ftUpperLeft, ftUpper, ftUpperRight,
		ftLeft, ftPane, ftRight,
		ftUnderLeft, ftUnder, ftUnderRight,
		ftMax
	} ;
	// フレームスタイル構造体
	struct	FRAME_STYLE
	{
		PEGL_IMAGE_INFO	pFrame[ftMax] ;
	} ;

protected:
	EGLImage	m_imgFrame ;
	EGL_SIZE	m_sizeLeft ;
	EGL_SIZE	m_sizeMiddle ;
	EGL_SIZE	m_sizeRight ;
	EGL_SIZE	m_sizePane ;
	FRAME_STYLE	m_fsStyle ;
	EGLImage	m_imgFrameParts[ftMax] ;

public:
	// フレーム生成
	ESLError CreateStaticFrame
		( const FRAME_STYLE & style, int nWidth, int nHeight ) ;
	// サイズ変更
	ESLError ResizeFrame( int nWidth, int nHeight ) ;
	// 当たり判定
	int HitTest( int xLocal, int yLocal ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 静テキストスプライト
//////////////////////////////////////////////////////////////////////////////

class	EStaticTextSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EStaticTextSprite( void ) ;
	// 消滅関数
	virtual ~EStaticTextSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EStaticTextSprite, ESpriteInterface )

public:
	// 文字整列
	enum	TextAlign
	{
		taLeft,				// 左揃え（横書き複数行）
		taTop,				// 上揃え（縦書き複数行）
		taRight,			// 右揃え
		taCenter,			// 中央揃え
		taAccordance		// 均等揃え
	} ;
	enum	TextExtensionFlag
	{
		txfBordering	= 0x01,	// 縁取り
	} ;
	// 文字スタイル構造体
	struct	TEXT_STYLE
	{
		TextAlign		taAlign ;			// 整列方法
		const wchar_t *	pwszText ;			// 文字列
		LOGFONT			lfFont ;			// フォント
		EGL_RECT		rectExt ;			// 文字描画領域
		EGL_PALETTE		rgbColor ;			// 文字色
		unsigned int	nTransparency ;		// 文字透明度
		EGL_PALETTE		rgbShadow ;			// 文字影色
		unsigned int	nShadowTrans ;		// 文字影透明度
		EGL_POINT		ptShadowOffset ;	// 文字影オフセット
		unsigned int	nLineHeight ;		// 行幅
		unsigned int	nIndent ;			// 字下げ幅
		unsigned int	nFlags ;			// 拡張フラグ
		EGL_PALETTE		rgbBorder ;			// 文字縁取り色
		unsigned int	nBorderTrans ;		// 文字縁取り透明度
		unsigned int	nFontPitch ;		// フォント固定ピッチ
	} ;

protected:
	EGLImage	m_imgText ;			// 文字イメージ
	EGL_SIZE	m_sizeView ;		// ビューサイズ
	int			m_nScrollRange ;	// スクロール範囲
	int			m_nScrollPos ;		// スクロール位置
	TEXT_STYLE	m_tsStyle ;			// スタイルデータ
	EWideString	m_wstrText ;

public:
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;

public:
	// 静テキストオブジェクト作成
	ESLError CreateText
		( const TEXT_STYLE & style, int nViewWidth, int nViewHeight ) ;
	// 文字列を設定する
	ESLError SetText( const wchar_t * pwszText ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
	// 文字列取得・設定
	virtual const wchar_t * GetSpriteText( void ) ;
	virtual void SetSpriteText( const wchar_t * pwszText ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;
	// スクロールレンジ取得
	virtual int GetVertScrollRange( void ) const ;
	// 表示位置取得
	virtual int GetVertScrollPos( void ) const ;
	// 表示位置設定
	virtual void SetVertScrollPos( int nPos ) ;

public:
	// テキスト描画関数
	static ESLError CreateFontImage
		( ERealFontImage & rfiText,
			const TEXT_STYLE & style, EGL_SIZE * pMaxSize = NULL ) ;
	static ESLError DrawFontImage
		( HEGL_DRAW_IMAGE hDrawImage,
			ERealFontImage & rfiText, const TEXT_STYLE & style ) ;
	// フォント情報取得
	static ESLError GetFontInformation( LOGFONT & lfFont, HFONT hFont ) ;
	// 有効なフォントフェースを取得
	static EString GetValidFontFace( const char * pszFontFaceList ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 進捗状況バースプライト
//////////////////////////////////////////////////////////////////////////////

class	EProgressBarSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EProgressBarSprite( void ) ;
	// 消滅関数
	virtual ~EProgressBarSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EProgressBarSprite, ESpriteInterface )

public:
	// 進捗状況バータイプ
	enum	ProgressBarType
	{
		pbtVert,
		pbtHorz
	} ;
	// 進捗状況バースタイル構造体
	struct	BAR_STYLE
	{
		ProgressBarType	pbtType ;
		EGL_SIZE		sizeExt ;
		EGL_POINT		ptBarOffset ;
		PEGL_IMAGE_INFO	pFrameLeft ;
		PEGL_IMAGE_INFO	pFrameRight ;
		PEGL_IMAGE_INFO	pFrameWay ;
		PEGL_IMAGE_INFO	pBarLeft ;
		PEGL_IMAGE_INFO	pBarRight ;
		PEGL_IMAGE_INFO	pBarWay ;
	} ;

protected:
	ProgressBarType	m_pbtType ;
	EGLImage		m_imgBack ;
	EGLImage		m_imgBar ;
	EGL_POINT		m_ptBar ;
	int				m_nBarLeft ;
	int				m_nBarRight ;

	int				m_nBarRange ;
	int				m_nBarPos ;

public:
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;

public:
	// 進捗状況バー生成
	ESLError CreateProgressBar( const BAR_STYLE & style ) ;
	// 画像描画
	void DrawProgressBar( void ) ;

public:
	// 全体量を取得
	int GetRange( void ) const
		{
			return	m_nBarRange ;
		}
	// 全体量を設定
	void SetRange( int nRange ) ;
	// 現在の進捗状況を取得
	int GetPos( void ) const
		{
			return	m_nBarPos ;
		}
	// 現在の進捗状況を設定
	void SetPos( int nPos ) ;

public:
	// 現在の垂直スクロール位置を取得
	virtual int GetVertScrollPos( void ) const ;
	// 現在の垂直スクロール位置を設定
	virtual void SetVertScrollPos( int nPos ) ;
	// 垂直スクロールの範囲を取得
	virtual int GetVertScrollRange( void ) const ;
	// 垂直スクロールの範囲を設定
	virtual void SetVertScrollRange( int nRange ) ;
	// 現在の水平スクロール位置を取得
	virtual int GetHorzScrollPos( void ) const ;
	// 現在の水平スクロール位置を設定
	virtual void SetHorzScrollPos( int nPos ) ;
	// 水平スクロールの範囲を設定
	virtual int GetHorzScrollRange( void ) const ;
	// 水平スクロールの範囲を設定
	virtual void SetHorzScrollRange( int nRange ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// ボタンスプライト
//////////////////////////////////////////////////////////////////////////////

class	EButtonSprite	: public	EStaticTextSprite
{
public:
	// 構築関数
	EButtonSprite( void ) ;
	// 消滅関数
	virtual ~EButtonSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EButtonSprite, EStaticTextSprite )

public:
	// ボタンの状態
	enum	ButtonStatus
	{
		bsNormal,		// 通常状態				000
		bsFocus,		// フォーカス状態		001
		bsPushed,		// 押下状態				010
		bsPushedFocus,	// 押下フォーカス状態	011
		bsDisabled,		// 禁止状態				100
		bsPushDisabled,	// 押下禁止状態			101
		bsActivePushed,	// アクティブ押下状態	110
		bsMax
	} ;
	// ボタンのタイプ
	enum	ButtonType
	{
		btStatic,		// 静的画像
		btDynamic,		// 動的画像
		btTextButton,	// 文字付加ボタン
		btCheckBox,		// チェックボックス
		btRadioButton,	// ラジオボタン
		btMax
	} ;
	// ボタンのフラグ
	enum	ButtonFlag
	{
		bfHitExtRect	= 0x0001,	// 外接矩形で当たり判定を行う
	} ;
	// ボタンスタイル構造体
	struct	BUTTON_STYLE
	{
		ButtonType		btType ;
		DWORD			dwFlags ;
		PEGL_IMAGE_INFO	pImage[bsMax] ;
		PEGL_IMAGE_INFO	pHitTestMask ;
		EGL_SIZE		sizeExt ;
		TEXT_STYLE		tsTextStyle[bsMax] ;
	} ;

protected:
	PEGL_IMAGE_INFO	m_pImage[bsMax] ;		// ボタン画像
	PEGL_IMAGE_INFO	m_pHitTestMask ;		// 当たり判定用画像
	ButtonType		m_style ;				// ボタンタイプ
	DWORD			m_dwBtnFlags ;			// ボタンフラグ
	ButtonStatus	m_status ;				// ボタン状態
	ButtonStatus	m_statusView ;			// ボタン表示状態
	bool			m_fFocus ;				// フォーカスを持っているか？
	bool			m_fPushed ;				// 押下状態か？

	EGLImage		m_imgButton[bsMax] ;	// 生成画像
	EGLImage		m_imgBase[bsMax] ;
//	EGLImage		m_imgText[bsMax] ;
	EGLImage		m_imgHitTestMask ;
	TEXT_STYLE		m_tsStyle[bsMax] ;
	EWideString		m_wstrText ;

	EWideString		m_wstrCursor ;			// フォーカス時のカーソル
	EWideString		m_wstrFocusSE ;			// フォーカス時の効果音
	EWideString		m_wstrPushedSE ;		// 押下時の効果音

	// ステータス変化の通知メッセージ
	bool			m_fStatusNotification ;
	long int		m_nStatusNoticeParameter ;
	// ステータス変化を反映する画像アイテムID
	EWideString		m_wstrStatusRefTarget ;
	// ステータス変化を反映する画像ID
	EWideString		m_wstrStatusRefImage[bsMax] ;

	// 右クリック
	bool			m_fEnableRightClick ;
	long int		m_nRightClickParameter ;

	// ボタン押下リピート用パラメータ
	DWORD			m_dwPushedTimeStamp ;	// 押下時のミリ秒カウンタ
	long int		m_nBeforeRepInterval ;	// リピート開始までの時間
	long int		m_nRepeatInterval ;		// リピート間隔

public:
	static const wchar_t *	m_pwszDefCursor ;
	static const wchar_t *	m_pwszDefFocusSE ;
	static const wchar_t *	m_pwszDefPushedSE ;

public:
	// 有効化・無効化
	virtual void Enable( bool fEnable ) ;
	// 文字列取得・設定
	virtual const wchar_t * GetSpriteText( void ) ;
	virtual void SetSpriteText( const wchar_t * pwszText ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
public:
	// メッセージ処理
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnSetCursor( int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonUp( UINT nFlags, int xPos, int yPos ) ;
	// メッセージ処理（タイマーフラグを持っているもののみ）
	virtual bool OnTimer( UINT nEventID ) ;
	// メッセージ処理
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;
public:
	// フォーカスを取得した
	virtual void OnSetFocus( void ) ;
	// フォーカスを奪われた
	virtual void OnKillFocus( void ) ;

public:
	// ボタンオブジェクト作成
	ESLError CreateButton( const BUTTON_STYLE & style ) ;
	// ボタン画像設定
	ESLError SetButtonImage( const BUTTON_STYLE & style ) ;
	// ボタン文字列設定
	ESLError SetButtonText( const wchar_t * pwszText ) ;
	// リソース破棄
	void Delete( void ) ;

public:
	// ボタンの状態を取得
	ButtonStatus GetButtonStatus( void ) const
		{
			return	m_status ;
		}
	ButtonStatus GetButtonViewStatus( void ) const
		{
			return	m_statusView ;
		}
	// ボタンの状態を設定
	void SetButtonStatus( ButtonStatus status ) ;
	// ボタンがチェックされているか？（チェックボタンのみ）
	bool IsButtonChecked( void ) ;
	// ボタンをチェックする（チェックボタンのみ）
	void CheckButton( bool fCheck ) ;

public:
	// フォーカス時のカーソル識別子を設定
	void SetCursorOnFocus( const wchar_t * pwszCursorID ) ;
	// （マウスカーソル）フォーカス時の効果音を設定
	void SetSoundOnFocus( const wchar_t * pwszSoundID ) ;
	// 押下時の効果音を設定
	void SetSoundOnPushed( const wchar_t * pwszSoundID ) ;

	// ステータス変化の通知設定
	void SetStatusNotification
		( bool fStatusNotification = true, long int nParameter = 0 ) ;
	// 右クリック通知設定
	void EnableRightClick
		( bool fRightClick = true, long int nParameter = 0 ) ;
	// ステータスリフレクション設定
	void SetStatusReflection( EDescription & dscParam ) ;
	// ボタンリピート押下機能設定
	void SetRepeatButtonNotification
		( long int nBeforeRepeat, long int nRepeatInterval ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スクロールバースプライト
//////////////////////////////////////////////////////////////////////////////

class	EScrollBarSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EScrollBarSprite( void ) ;
	// 消滅関数
	virtual ~EScrollBarSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EScrollBarSprite, ESpriteInterface )

public:
	// スクロールバータイプ列挙型
	enum	ScrollBarType
	{
		sbtVert,		// 垂直スクロールバー
		sbtHorz			// 水平スクロールバー
	} ;
	// スクロールバーの状態
	enum	BarStatus
	{
		bsNormal,		// 通常状態
		bsFocus,		// フォーカス状態
		bsTracking,		// トラッキング状態
		bsDisabled,		// 禁止状態
		bsMax
	} ;
	// スクロールバースタイル構造体
	struct	BAR_STYLE
	{
		ScrollBarType				sbtBarType ;
		EGL_SIZE					sizeBarExt ;
		EGL_POINT					ptPrevButton ;
		EButtonSprite::BUTTON_STYLE	bsPrevButton ;
		EGL_POINT					ptNextButton ;
		EButtonSprite::BUTTON_STYLE	bsNextButton ;
		EGL_POINT					ptColumnPos ;
		EGL_RECT					rctTrackSpace ;
		int							nColumnWidth ;
		PEGL_IMAGE_INFO				pColumnImage[bsMax] ;
		PEGL_IMAGE_INFO				pBarImage[bsMax] ;
		PEGL_IMAGE_INFO				pProgressImage[bsMax] ;
	} ;
	// スクロールバー通知メッセージ
	enum	NotificationCode
	{
		ncGeneric,
		ncLineUp,
		ncLineDown,
		ncClickColumn,
		ncTracking,
		ncEndTracking,
		ncOnMouse,
		ncOnLeave,
	} ;

protected:
	ScrollBarType		m_sbtType ;			// タイプ
	BarStatus			m_bsColumn ;		// 背景カラムの状態
	BarStatus			m_bsBar ;			// バーの状態
	EButtonSprite		m_btnPrev ;			// 上（左）ボタン
	EButtonSprite		m_btnNext ;			// 下（右）ボタン
	EImageSprite		m_isColumn ;		// 背景カラム
	EImageSprite		m_isProgress ;		// 進捗バー
	ESpriteInterface	m_siBar ;			// つまみ
	EGLImage			m_imgColumn[bsMax] ;
	EGLImage			m_imgBar[bsMax] ;
	EGLImage			m_imgProgress[bsMax] ;

	EGL_POINT			m_ptBarBase ;		// つまみの基準座標
	EGL_POINT			m_ptTrackBase ;
	int					m_nWidth ;			// つまみの可動幅
	int					m_nPos ;			// 現在の位置
	int					m_nRange ;			// スクロールバーの領域
	int					m_nLine ;			// 行サイズ（ボタンでの移動幅）
	int					m_nTrackOffset ;	// トラッキングオフセット

	EWideString			m_wstrCursor ;		// カーソル識別子

	ESpriteInterface *	m_pItem ;			// 関連付けられたアイテム

public:
	static const wchar_t *	m_pwszDefCursor ;

protected:
	static const wchar_t *	m_pwszPrevButtonID ;
	static const wchar_t *	m_pwszNextButtonID ;

public:
	// スクロールバーを作成
	ESLError CreateScrollBar( const BAR_STYLE & style ) ;

public:
	// 有効化・無効化
	virtual void Enable( bool fEnable ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
public:
	// メッセージ処理（マウスが上に乗っているかキャプチャーしているもののみ）
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnSetCursor( int xPos, int yPos ) ;
	virtual bool OnMouseWheel
		( UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonUp( UINT nFlags, int xPos, int yPos ) ;
	// コマンド処理
	virtual void OnCommand
		( ESpriteInterface * pItem,
			long int nNotification = 0, long int nParameter = 0,
			int nPriority = EWndSpriteCmd::priorityNormal, bool fOverwrite = false ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;
	// マウスのキャプチャーがリリースされた
	virtual void OnCaptureReleased( void ) ;

	// （ローカル）座標からスクロール位置を取得
	int GetScrollPosFromPoint( int xPos, int yPos ) const ;
	// スクロール位置から（ローカル）座標を取得
	EGL_POINT GetPointFromScrollPos( int nPos ) const ;

public:
	// 現在の垂直スクロール位置を取得
	virtual int GetVertScrollPos( void ) const ;
	// 現在の垂直スクロール位置を設定
	virtual void SetVertScrollPos( int nPos ) ;
	// 垂直スクロールの範囲を取得
	virtual int GetVertScrollRange( void ) const ;
	// 垂直スクロールの範囲を設定
	virtual void SetVertScrollRange( int nRange ) ;
	// 現在の水平スクロール位置を取得
	virtual int GetHorzScrollPos( void ) const ;
	// 現在の水平スクロール位置を設定
	virtual void SetHorzScrollPos( int nPos ) ;
	// 水平スクロールの範囲を設定
	virtual int GetHorzScrollRange( void ) const ;
	// 水平スクロールの範囲を設定
	virtual void SetHorzScrollRange( int nRange ) ;

public:
	// 現在のスクロール位置取得
	int GetScrollPos( void ) const
		{
			return	m_nPos ;
		}
	// スクロール位置設定
	void SetScrollPos( int nPos ) ;
	// スクロール領域取得
	int GetScrollRange( void ) const
		{
			return	m_nRange ;
		}
	// スクロール領域設定
	void SetScrollRange( int nRange ) ;
	// 行サイズ設定
	void SetLineSize( int nLine ) ;
	// アイテムを関連付ける
	void AttachScrollItem( ESpriteInterface * pItem ) ;
	// 関連付けられているアイテムのスクロール情報を反映する
	void ReflectAttachedItem( void ) ;

public:
	// フォーカス時のカーソル識別子を設定
	void SetCursorOnFocus( const wchar_t * pwszCursorID ) ;
	// （上下ボタン）フォーカス時の効果音を設定
	void SetSoundOnFocus( const wchar_t * pwszSoundID ) ;
	// （上下ボタン）押下時の効果音を設定
	void SetSoundOnPushed( const wchar_t * pwszSoundID ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 文字列入力スプライト
//////////////////////////////////////////////////////////////////////////////

class	ETextEditSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	ETextEditSprite( void ) ;
	// 消滅関数
	virtual ~ETextEditSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ETextEditSprite, ESpriteInterface )

public:
	// 文字列入力タイプ列挙型
	enum	EditType
	{
		etSingleLine,
		etMultiLine
	} ;
	// 機能フラグ
	enum	EditFlag
	{
		efInputReturn	= 0x0001,
		efInputTab		= 0x0002,
		efAutoIndent	= 0x0004,
		efLineTabIndent	= 0x0008,
		efReadOnly		= 0x0010,
		efDenyAlphabet	= 0x1000,
		efDenyNumber	= 0x2000,
		efDeny8bitChar	= 0x4000,
		efDenyMBChar	= 0x8000,
	} ;
	// 文字列入力スプライトのスタイル構造体
	struct	EDIT_STYLE
	{
		EditType		etType ;			// 入力タイプ
		EGL_SIZE		sizeExt ;			// サイズ
		PEGL_IMAGE_INFO	pLeftSide ;			// 左端画像
		PEGL_IMAGE_INFO	pRightSide ;		// 右端画像
		PEGL_IMAGE_INFO	pTextWay ;			// 中央画像
		int				nEditTop ;			// 編集領域
		int				nEditBottom ;
		EGL_SIZE		sizeCaret ;			// カレットサイズ
		unsigned int	nCaretInterval ;	// カレット点滅間隔 [ms]
		LOGFONT			lfEditFont ;		// 文字フォント
		EGL_PALETTE		rgbTextColor ;		// 文字色
		EGL_PALETTE		rgbSelTextColor ;	// 文字色
		EGL_PALETTE		rgbCaretColor ;		// カレット色(RGBA)
		bool			fIMEFont ;
		LOGFONT			lfIMEFont ;			// IME 入力中フォント
	} ;
	// 通知コード
	enum	NotificationCode
	{
		ncChange,
		ncKillFocus
	} ;
	// 編集コマンド
	enum	CommandID
	{
		CMDID_EDIT_CLEAR		= 0xE120,
		CMDID_EDIT_COPY			= 0xE122,
		CMDID_EDIT_CUT			= 0xE123,
		CMDID_EDIT_PASTE		= 0xE125,
		CMDID_EDIT_SELECT_ALL	= 0xE12A,
		CMDID_EDIT_UNDO			= 0xE12B,
		CMDID_EDIT_REDO			= 0xE12C,
	} ;

protected:
	// 文字画像バッファ
	class	ECharacterBuffer	: public	EImageSprite
	{
	public:
		int		m_nCharWidth ;			// 文字幅
		int		m_nCharCode ;			// 文字コード
	} ;
	// 文字情報バッファ
	class	ECharacter	: public ESLObject
	{
	public:
		ECharacterBuffer *	m_pBuf ;	// 画像バッファ
		int					m_nWidth ;	// 文字幅
	} ;
	// 行情報
	class	ELineInf
	{
	public:
		EWideString	m_wstrLine ;	// 行のテキストデータ
		EObjArray<ECharacter>
					m_aryText ;		// 表示用データ
		int			m_nFlags ;		// 行のフラグ
		int			m_nIndex ;		// 行の文字指標
		int			m_nWidth ;		// 行の幅（ピクセル）
	public:
		ELineInf( void ) : m_nFlags(0), m_nIndex(0), m_nWidth(0) { }
	} ;
	// UNDO 情報
	class	EUndoInf
	{
	public:
		int			m_iFirst ;		// Undo 開始位置
		int			m_iEnd ;		// Undo 終了位置
		EWideString	m_wstrUndo ;	// Undo 置き換え文字列
	public:
		EUndoInf( void ) : m_iFirst(0), m_iEnd(0) { }
		EUndoInf( const EUndoInf & undo )
			: m_iFirst(undo.m_iFirst),
				m_iEnd(undo.m_iEnd), m_wstrUndo(undo.m_wstrUndo) { }
		const EUndoInf & operator = ( const EUndoInf & undo )
			{
				m_iFirst = undo.m_iFirst ;
				m_iEnd = undo.m_iEnd ;
				m_wstrUndo = undo.m_wstrUndo ;
				return	*this ;
			}
	} ;

protected:
	EditType			m_etType ;			// タイプ
	DWORD				m_dwEditFlags ;		// 機能フラグ
	int					m_nLimitLength ;	// 最大入力許可文字数

	ETextEditSprite *	m_pEditServer ;		// 元データ
	EPtrObjArray<ETextEditSprite>
						m_lstEditClient ;	// クライアント

	EImageSprite		m_isBackPanel ;		// 背景スプライト
	EImageSprite		m_isTextPanel ;		// 文字表示スプライト
	EObjArray<ELineInf>	m_lstLine ;			// 行の配列
	EWideString			m_wstrTextBuf ;		// 文字列受け渡し用一時バッファ

	HEGL_DRAW_IMAGE		m_hDrawLine ;		// 描画用オブジェクト

	ERealFontImage		m_rfiText ;			// 表示用オブジェクト
	ETagSortArray<wchar_t,ECharacterBuffer>
						m_tsaImgBuf ;		// 文字画像保持用バッファ

	HFONT				m_hFont ;			// 表示用フォント
	LOGFONT				m_lfFont ;
	LOGFONT				m_lfIMC ;			// IMC 用フォント
	EGL_PALETTE			m_rgbTextColor ;	// 文字表示色
	EGL_PALETTE			m_rgbSelTextColor ;	// 文字表示色
	EGL_PALETTE			m_rgbCaretColor ;	// カレット色
	unsigned int		m_nTabWidth ;		// タブ幅
	unsigned int		m_nMaxLineWidth ;	// 最大行幅
	int					m_nWordWrapWidth ;	// 行の折り返し幅
	int					m_nPageSize ;		// ページサイズ
	int					m_nLeftSpace ;		// 左側スクロール用スペース
	int					m_xScroll ;			// 水平方向スクロール座標
	int					m_yScroll ;			// 垂直方向スクロール座標

	typedef LONG
		(WINAPI *API_ImmGetCompositionString)( HIMC, DWORD, LPVOID, DWORD ) ;
	HMODULE				m_hIMM32 ;			// IMM32.DLL モジュール
	API_ImmGetCompositionString
						m_apiGetCompositionStringW ;

	ERectangleSprite	m_rsCaret ;			// カーソル
	EGL_SIZE			m_sizeCaret ;		// カーソルサイズ
	unsigned int		m_nCaretInterval ;	// カレット点滅間隔 [ms]
	int					m_iSelFirst ;		// 選択開始指標
	int					m_iSelEnd ;			// 選択終了指標

	unsigned int		m_nUndoLimit ;		// UNDO 回数
	EObjArray<EUndoInf>	m_lstUndo ;			// UNDO バッファ
	EObjArray<EUndoInf>	m_lstRedo ;

	HMENU				m_hMenuPopup ;		// ポップアップメニュー
	int					m_fMouseSel ;		// マウスで選択操作中か？
	bool				m_fFocus ;			// フォーカスを持っている
	bool				m_fViewCaret ;		// カレットを表示するか？
	bool				m_fIMEComposition ;	// IME 文字入力中
	char				m_cLastChar ;		// 最後に入力された文字
	DWORD				m_dwLastTime ;		// カレット点滅用カウンタ
	DWORD				m_dwScrollTimer ;	// マウスでのスクロール用タイマ

	EWideString			m_wstrProhibit ;	// 禁則文字

	EWideString			m_wstrCursor ;		// カーソル識別子

public:
	static const wchar_t *	m_pwszDefCursor ;

public:
	// 文字列取得・設定
	virtual const wchar_t * GetSpriteText( void ) ;
	virtual void SetSpriteText( const wchar_t * pwszText ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
public:
	// メッセージ処理
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnSetCursor( int xPos, int yPos ) ;
	virtual bool OnMouseWheel
		( UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonUp( UINT nFlags, int xPos, int yPos ) ;
	// メッセージ処理
	virtual bool OnTimer( UINT nEventID ) ;
	// メッセージ処理
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;
protected:
	// ウィンドウの描画を更新する
	void UpdateWindow( void ) ;
	// Undo を記録
	void RecordUndo( EUndoInf * pUndo ) ;
	// 単語の境界を検出する
	virtual int GetWordBoundary
		( const EWideString & wstrLine,
			int nIndex, int & nFirst, int & nEnd ) const ;
	// 文字の種別を取得する
	virtual int GetCharacterTypeClass( wchar_t wchCode ) const ;
public:
	// フォーカスを取得した
	virtual void OnSetFocus( void ) ;
	// フォーカスを奪われた
	virtual void OnKillFocus( void ) ;
public:
	// 機能フラグを設定する
	virtual void SetFunctionFlags( DWORD dwFlags ) ;
	// カレットの表示状態を設定
	void ShowCaret( bool fShow ) ;
	// 行間を取得する
	unsigned int GetLineHeight( void ) const ;
	// 入力文字制限を取得する
	int GetLimitLength( void ) const
		{	return	m_nLimitLength ;	}
	// 入力文字制限を設定する
	void SetLimitLength( int nLimit )
		{	m_nLimitLength = nLimit ;	}
	// 編集テキストフラグを取得する
	DWORD GetEditFlags( void ) const
		{	return	m_dwEditFlags ;	}
	// 編集テキストフラグを設定する
	void SetEditFlags( DWORD dwEditFlags ) ;
	// タブ幅を取得する
	unsigned int GetTabWidth( void ) const
		{	return	m_nTabWidth ;	}
	// タブ幅を設定する
	void SetTabWidth( unsigned int nTabWidth, bool fRedraw = true ) ;
	// 左側余白幅を取得する
	int GetLeftSpaceWidth( void ) const
		{	return	m_nLeftSpace ;	}
	// 左側余白幅を設定する
	void SetLeftSpaceWidth( int nLeftSpace ) ;
	// ページサイズを取得する
	int GetPageSize( void ) const
		{	return	m_nPageSize ;	}
	// ページサイズを設定する
	void SetPageSize( int nPageSize )
		{	m_nPageSize = nPageSize ;	}
	// UNDO 制限回数を取得
	unsigned int GetUndoLimit( void ) const
		{	return	m_nUndoLimit ;	}
	// UNDO 制限回数を設定
	void SetUndoLimit( unsigned int nLimit )
		{	m_nUndoLimit = nLimit ;	}
	// 行の折り返し幅を取得する
	int GetWordWrapWidth( void ) const
		{	return	m_nWordWrapWidth ;	}
	// 行の折り返し幅を設定する
	void SetWordWrapWidth( int nWordWrap, bool fRedraw = true ) ;
	// 禁則文字を取得する
	const wchar_t * GetProhibitChar( void ) const ;
	// 禁則文字を設定する
	void SetProhibitChar( const wchar_t * pwszProhibit ) ;
	// 禁則文字か？
	bool IsProhibitChar( wchar_t wchar ) const ;

public:
	// 文字列入力オブジェクト作成
	ESLError CreateEdit( const EDIT_STYLE & style ) ;
	// サイズ変更
	ESLError ResizeEdit( const EDIT_STYLE & style ) ;
	// 属性変更
	ESLError ModifyEditStyle( const EDIT_STYLE & style ) ;
	// 全ての行の書式を再整形
	void UpdateAllLines( void ) ;

public:
	// 文字指標からｘ座標を計算
	int GetCharPosFromIndex( int iChar ) const ;
	// 文字指標から行番号を取得
	int GetLineFromIndex( int iChar ) const ;
	// 行の先頭の文字指標を取得
	int GetLineIndex( int nLine ) const ;
	// 行の文字数を取得
	int GetLineLength( int nLine ) const ;
	// 行数を取得
	int GetLineCount( void ) const ;
	// 全文字数取得
	int GetLength( void ) const ;
	// 指定行の文字列を取得
	EWideString GetLineText( int nLine ) const ;
	// スクロール位置取得（ピクセル, 行）
	EGLPoint GetScrollPos( void ) const ;
	// スクロール位置設定
	void SetScrollPos( int xPos, int nLine ) ;
	// 行の最大幅（ピクセル）を取得
	int GetMaxLineWidth( void ) const ;

protected:
	// 指定行取得
	ELineInf * GetLineAt( int nLine ) const ;
	// 文字列描画
	virtual void DrawViewText( bool fChanged ) ;
	virtual void DrawViewLine( int nLine, bool fChanged ) ;
	// 文字描画
	virtual int DrawViewCharacter
		( HEGL_DRAW_IMAGE hDraw, const ELineInf * pLInf,
			int nIndex, int xPos, int yPos, bool fSelText ) ;

public:
	// 文字列取得
	virtual EWideString GetEditText( void ) const ;
	// 文字列設定
	virtual void SetEditText( const wchar_t * pwszText ) ;
protected:
	// 文字列追加（単純処理）
	void AddTextSimply( const EWideString & wstrText ) ;

public:
	// 選択範囲取得
	virtual void GetSel( int & iSelFirst, int & iSelEnd ) const ;
	// 選択範囲設定
	virtual void SetSel( int iSelFirst, int iSelEnd ) ;
	// 選択範囲の文字列を取得
	virtual EWideString GetSelText( void ) const ;
	// 指定範囲の文字列を取得
	virtual EWideString GetRangeText( int iFirst, int iEnd ) const ;
	// 文字コピー可能か？
	virtual bool CanCopyText( void ) ;
	// 文字切り取り可能か？
	virtual bool CanCutText( void ) ;
	// 文字列貼り付け可能か？
	virtual bool CanPasteText( void ) ;
	// 選択文字列削除
	virtual void DoClear( void ) ;
	virtual void ClearSelText( EUndoInf * pUndo = NULL ) ;
	// 選択文字列切り取り
	virtual void DoCut( void ) ;
	virtual void CutSelText( EUndoInf * pUndo = NULL ) ;
	// 選択文字列コピー
	virtual void DoCopy( void ) ;
	virtual void CopySelText( void ) ;
	// 選択文字列貼りつけ
	virtual void DoPaste( void ) ;
	virtual void PasteSelText( EUndoInf * pUndo = NULL ) ;
	// 選択文字列置き換え
	virtual void DoReplace( const wchar_t * pwszText ) ;
	virtual void ReplaceSelText
		( const wchar_t * pwszText, EUndoInf * pUndo = NULL ) ;
	// UNDO 可能か？
	virtual bool CanUndo( void ) ;
	// UNDO 実行
	virtual void Undo( void ) ;
	// REDO 可能か？
	virtual bool CanRedo( void ) ;
	// REDO 実行
	virtual void Redo( void ) ;

protected:
	// カレットの表示位置を更新
	void UpdateCaretPos( int iSelFirst, int iSelEnd ) ;
	// 指定の文字コードの文字データオブジェクトを生成する
	virtual ECharacterBuffer * CreateCharacterImage( wchar_t wch ) ;
	// 指定文字の文字画像をレンダリングする
	virtual void RenderingCharacterImage( ECharacterBuffer * pCharaBuf ) ;
	// 指定行の幅の調整および以降の行の文字指標の正規化と表示の更新
	virtual void UpdateLineInfo( int nLeadLine, bool fRedraw = true ) ;

public:
	// 指定の文字列を検索する
	int FindTextUsage
		( const wchar_t * pwszMatchUsage,
			int iFirst, int nFindDir = 0, int * pFindEnd = NULL,
			EStreamWideString * pswsRule = NULL ) ;

public:
	// （ローカル）座標から文字指標へ変換
	int GetCharIndexFromPos( int xPos, int yPos ) ;
	// ｘ座標から指定行の文字指標を取得
	int GetIndexFromLinePos( int nLine, int xPos ) ;

public:
	// 編集域のカーソル識別子を設定
	void SetCursorOnEdit( const wchar_t * pwszCursorID ) ;

public:
	// クライアント関連付け
	void AttachClient( ETextEditSprite * pClient ) ;
	// クライアント分離
	void DetachClient( ETextEditSprite * pClient ) ;

protected:
	// 行情報の更新通知
	virtual void OnUpdateLineInfo( ELineInf * pLInf ) ;
	// スクロール情報通知
	virtual void OnScrollPos( void ) ;
	virtual void OnScrollSize( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// リストビュー・スプライト
//////////////////////////////////////////////////////////////////////////////

class	EListViewSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EListViewSprite( void ) ;
	// 消滅関数
	virtual ~EListViewSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EListViewSprite, ESpriteInterface )

public:
	// リストタイプ
	enum	ListType
	{
		ltSingleSelect		= 0x0000,
		ltMultiSelect		= 0x0001
	} ;
	// リスト状態
	enum	ListStatus
	{
		lsNomral,						// 通常状態
		lsFocus,						// フォーカス状態
		lsPushed,						// 選択された状態
		lsPushedFocus,					// 選択＆フォーカス状態
		lsMax
	} ;
	// 通知コード
	enum	NotificationCode
	{
		ncSelChanged,
		ncKillFocus,
		ncClickLine,
		ncDblClickLine
	} ;
	// リストビュースタイル
	struct	LINE_STYLE
	{
		EStaticTextSprite::TEXT_STYLE	tsText ;
		EGL_PALETTE	rgbaBackColor ;		// 背景色
	} ;
	struct	LIST_STYLE
	{
		int			nTypeFlags ;		// リストタイプフラグ
		EGL_SIZE	sizeExt ;			// サイズ
		int			nLineHeight ;		// デフォルトの行間
		LINE_STYLE	lsLine[lsMax] ;		// 文字の表示スタイル
	} ;
	// ラインエントリ
	class	ELine	: public	ESLObject
	{
	public:
		int						m_iImage ;
		int						m_iStatus ;
		int						m_nLineHeight ;
		EObjArray<EWideString>	m_lstText ;
	public:
		// 構築関数
		ELine( void ) : m_iImage(-1), m_iStatus(0), m_nLineHeight(0) { }
		// 消滅関数
		virtual ~ELine( void ) { }
		// クラス情報
		DECLARE_CLASS_INFO( ELine, ESLObject )
	public:
		// 比較
		virtual int Compare
			( EListViewSprite & list, const ELine & line, int col = 0 ) const ;
		// 描画
		virtual void Draw
			( EListViewSprite & list,
				HEGL_RENDER_POLYGON hRender, int nLineNum, int yPos ) ;
	} ;

protected:
	EObjArray<ELine>	m_lstLine ;
	ENumArray<int>		m_lstColWidth ;
	EPtrObjArray<EGL_IMAGE_INFO>
						m_lstImage ;
	LIST_STYLE			m_tsStyle ;
	EFontObject			m_fontText[lsMax] ;
	EGL_POINT			m_ptScroll ;
	int					m_iFocusLine ;
	int					m_iLastSelLine ;

protected:
	// 領域再描画
	virtual void RefreshRect( const EGL_RECT & rectRefresh ) ;
	// 行描画関数
	virtual void DrawLine
		( HEGL_RENDER_POLYGON hRender,
			ELine & line, int nLineNum, int yPos ) ;
	// 比較
	virtual int CompareLine
		( const ELine & line1, const ELine & line2, int col = 0 ) ;
public:
	// 文字列描画
	void DrawLineText
		( HEGL_RENDER_POLYGON hRender,
			int nStatus, int nCol, int xOffset,
				int yPos, const wchar_t * pwszText ) ;

public:
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
public:
	// メッセージ処理
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnMouseWheel
		( UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	// メッセージ処理
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;

public:
	// フォーカスを取得した
	virtual void OnSetFocus( void ) ;
	// フォーカスを奪われた
	virtual void OnKillFocus( void ) ;

public:
	// ローカル座標から該当するラインを取得する
	int GetLineFromPos( int yPos ) ;
	// 指定された行を囲む矩形を取得する
	EGL_RECT GetLineRect( int nLine ) ;
	// 指定された行の高さを取得する
	int GetLineHeight( int nLine ) ;
	// 指定の行を選択する（単一行選択）
	void SelectLine( int nLine ) ;
	// 指定の行が表示されるようにスクロール
	void ScrollToViewLine( int nLine ) ;

public:
	// 現在の垂直スクロール位置を取得
	virtual int GetVertScrollPos( void ) const ;
	// 現在の垂直スクロール位置を設定
	virtual void SetVertScrollPos( int nPos ) ;
	// 垂直スクロールの範囲を取得
	virtual int GetVertScrollRange( void ) const ;
	// 垂直スクロールの範囲を設定
	virtual void SetVertScrollRange( int nRange ) ;
	// 現在の水平スクロール位置を取得
	virtual int GetHorzScrollPos( void ) const ;
	// 現在の水平スクロール位置を設定
	virtual void SetHorzScrollPos( int nPos ) ;
	// 水平スクロールの範囲を設定
	virtual int GetHorzScrollRange( void ) const ;
	// 水平スクロールの範囲を設定
	virtual void SetHorzScrollRange( int nRange ) ;
	// 現在のスクロール位置を取得する（水平：ピクセル単位、垂直：行単位）
	const EGL_POINT & GetScrollPos( void ) const
		{
			return	m_ptScroll ;
		}

public:
	// リストオブジェクト作成
	ESLError CreateListView( const LIST_STYLE & style ) ;

public:
	// カラム幅取得
	int GetColumnWidth( int iIndex ) const
		{
			return	m_lstColWidth.GetAt( iIndex ) ;
		}
	// カラム幅設定
	void SetColumnWidth( int iIndex, int nWidth ) ;
	// カラム削除
	void RemoveColumn( int iIndex )
		{
			m_lstColWidth.RemoveAt( iIndex ) ;
		}
	// カラム数取得
	int GetColumnCount( void ) const
		{
			return	(int) m_lstColWidth.GetSize( ) ;
		}
	// カラムｘ座標取得
	int GetColumnLeftPos( int iIndex ) const ;

public:
	// 文字列追加
	int InsertItem
		( const wchar_t * pwszText,
			int iIndex, int iSubIndex = 0, int iImage = -1 ) ;
	// 行追加
	int InsertLine( int iIndex, ELine * pLine ) ;
	// 行数を取得
	int GetLineCount( void ) const
		{
			return	(int) m_lstLine.GetSize( ) ;
		}
	// 行を取得
	ELine * GetLineAt( int iLine ) const
		{
			return	m_lstLine.GetAt( iLine ) ;
		}
	// 文字列取得
	const wchar_t * GetItemText( int iIndex, int iColIndex = 0 ) const ;
	// 文字列設定
	void SetItemText( int iIndex, int iColIndex, const wchar_t * pwszText ) ;
	// 選択されているアイテムの総数を取得
	int GetSelectedItemCount( void ) const ;
	// 選択アイテム取得
	int FindSelectedItem( int iFirst = 0 ) const ;
	// ソート
	void SortList( int iSortType = 0, int iSubIndex = 0 ) ;

public:
	// 画像リスト追加
	void InsertImage( int iIndex, PEGL_IMAGE_INFO pImage ) ;
	// 画像リスト削除
	void ClearImageList( void ) ;
	// 画像リスト取得
	EPtrObjArray<EGL_IMAGE_INFO> & ImageList( void )
		{
			return	m_lstImage ;
		}
	const EPtrObjArray<EGL_IMAGE_INFO> & ImageList( void ) const
		{
			return	m_lstImage ;
		}

	friend	EListViewSprite::ELine ;
} ;


//////////////////////////////////////////////////////////////////////////////
// コンボリスト・スプライト
//////////////////////////////////////////////////////////////////////////////

class	EComboListSprite	: public	ESpriteInterface
{
public:
	// 構築関数
	EComboListSprite( void ) ;
	// 消滅関数
	virtual ~EComboListSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EComboListSprite, ESpriteInterface )

public:
	// コンボタイプ
	enum	ComboType
	{
		ctComboList,		// リストを常に表示
		ctDropDown,			// ドロップダウン
		ctDropDownList,		// ドロップダウンリスト（テキスト編集不可）
		ctMax
	} ;
	enum	StyleFlag
	{
		sfEditCtrl		= 0x0001,
		sfDropDownBtn	= 0x0002,
		sfListFrame		= 0x0004,
		sfListView		= 0x0008,
		sfListScroll	= 0x0010
	} ;
	// コンボリストスタイル
	struct	COMBO_STYLE
	{
		int			nType ;				// コンボタイプ
		int			nFlags ;			// 有効スタイルフラグ
		EGL_SIZE	sizeExt ;			// サイズ
		ETextEditSprite::EDIT_STYLE
					esEdit ;			// エディットスタイル
		EButtonSprite::BUTTON_STYLE
					bsButton ;			// ドロップダウンボタン
		EStaticFrameSprite::FRAME_STYLE
					fsFrame ;			// フレーム（リスト背景）スタイル
		EListViewSprite::LIST_STYLE
					lsList ;			// リストビュースタイル
		EScrollBarSprite::BAR_STYLE
					bsScroll ;			// スクロールバースタイル
	} ;
	// コンボボックス通知メッセージ
	enum	NotificationCode
	{
		ncChange,
		ncKillFocus
	} ;

protected:
	COMBO_STYLE				m_csStyle ;		// スタイル
	ETextEditSprite *		m_pEdit ;		// エディットコントロール
	EButtonSprite *			m_pButton ;		// ドロップダウンボタン
	EStaticFrameSprite *	m_pFrame ;		// リストビューのフレーム
	EListViewSprite *		m_pList ;		// リストビュー
	EScrollBarSprite *		m_pScroll ;		// リストビューのスクロールバー
	bool					m_fDropList ;	// リストが表示されているか？

public:
	// コンボリストを作成
	ESLError CreateCombo( const COMBO_STYLE & style ) ;

public:
	// 文字列取得・設定
	virtual const wchar_t * GetSpriteText( void ) ;
	virtual void SetSpriteText( const wchar_t * pwszText ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;

public:
	// クリックされた
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	// コマンド処理
	virtual void OnCommand
		( ESpriteInterface * pItem,
			long int nNotification = 0, long int nParameter = 0 ) ;
	// 固有の処理
	virtual long int SendCommand
		( const EDescription & dscParam,
			EWideString * pwstrResult = NULL ) ;

public:
	// フォーカスを奪われた
	virtual void OnKillFocus( void ) ;

public:
	// リストを表示・非表示状態にする
	void SetDropListVisible( bool fVisible = true ) ;
	// リストの表示状態を取得する
	bool IsDropListVisible( void ) const
		{
			return	m_fDropList ;
		}
	// エディットボックスを取得する
	ETextEditSprite * GetEdit( void ) const
		{
			return	m_pEdit ;
		}
	// リストビューを取得する
	EListViewSprite * GetListView( void ) const
		{
			return	m_pList ;
		}
	// スクロールバーを取得する
	EScrollBarSprite * GetScrollBar( void ) const
		{
			return	m_pScroll ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// フォーム読み込みクラス
//////////////////////////////////////////////////////////////////////////////

class	EFormResourceManager	: public	ESLObject
{
public:
	// 構築関数
	EFormResourceManager( void ) ;
	// 消滅関数
	virtual ~EFormResourceManager( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EFormResourceManager, ESLObject )

protected:
	EWStrTagArray<ESLObject>	m_wstaResource ;
	EWStrTagArray<EDescription>	m_wstaStyle ;
	EWStrTagArray<EDescription>	m_wstaPage ;
	int							m_nPageNest ;

public:
	// 内容削除
	virtual void DeleteContents( void ) ;
	// エラー出力
	virtual void OutputError( const char * pszErrMsg ) ;

public:
	// スキンファイルを読み込む
	virtual ESLError ReadSkinFile
		( ESLFileObject & file, EDescription & dscSkin ) ;
	// リソースセクション読み込み
	virtual ESLError ReadResourceSection
		( EDescription & descRes, ERISAArchive & file ) ;
	// リソース読み込み
	virtual ESLError ReadResourceTag
		( EDescription & descRes, ERISAArchive & file ) ;
	// リソース追加
	virtual ESLError AddResource( const wchar_t * pwszID, ESLObject * pRes ) ;
	// リソース取得
	virtual ESLObject * GetResourceAs( const wchar_t * pwszID ) ;

public:
	// スタイルセクション読み込み
	virtual ESLError ReadStyleSection( EDescription & descStyle ) ;
	// スタイル追加
	virtual ESLError AddStyle( EDescription & descStyle ) ;
	// スタイル取得
	virtual EDescription * GetStyleAs( const wchar_t * pwszID ) ;

public:
	// ページ作成
	virtual ESLError CreateFormedPage
		( ESpriteInterface & siPage, EDescription & descPage ) ;
	// フォームセクション読み込み
	virtual ESLError ReadFormSection
		( ESpriteInterface & siPage, EDescription & descPage ) ;
	// フォーム読み込み
	virtual ESLError ReadFormTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// オブジェクト作成
	virtual ESpriteInterface * ReadTagObject
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// image フォーム読み込み
	virtual ESpriteInterface * ReadImageTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// static_frame フォーム読み込み
	virtual ESpriteInterface * ReadStaticFrameTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// static_text フォーム読み込み
	virtual ESpriteInterface * ReadStaticTextTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// progress_bar フォーム読み込み
	virtual ESpriteInterface * ReadProgressBarTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// button フォーム読み込み
	virtual ESpriteInterface * ReadButtonTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// scroll_bar フォーム読み込み
	virtual ESpriteInterface * ReadScrollBarTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// edit_text フォーム読み込み
	virtual ESpriteInterface * ReadEditTextTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// list_view フォーム読み込み
	virtual ESpriteInterface * ReadListViewTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// combo_list フォーム読み込み
	virtual ESpriteInterface * ReadComboListTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// object フォーム読み込み
	virtual ESpriteInterface * ReadObjectTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;
	// 未知のフォーム読み込み
	virtual ESpriteInterface * ReadExtendedFromTag
		( ESpriteInterface & siPage, EDescription & descTag ) ;

public:
	// ページリストを設定
	ESLError AddPageList( EDescription & descTag ) ;
	// ページリストを削除
	void RemovePageList( void ) ;
	// ページのフォームを取得
	EDescription * GetPageFormAs( const wchar_t * pwszPageID ) ;

public:
	// スタイル用画像バッファサイズ
	enum	{	IMGBUF_SIZE	= 32	} ;
	// static_frame 用スタイル読み込み
	ESLError GetStaticFrameStyle
		( EStaticFrameSprite::FRAME_STYLE & style,
			EDescription & descStyle, EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] ) ;
	// static_text 用スタイル読み込み
	static ESLError GetStaticTextStyle
		( EStaticTextSprite::TEXT_STYLE & style, EDescription & descStyle ) ;
	// progress_bar 用スタイル読み込み
	ESLError GetProgressBarStyle
		( EProgressBarSprite::BAR_STYLE & style,
			EDescription & descStyle, EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] ) ;
	// button 用スタイル読み込み
	ESLError GetButtonStyle
		( EButtonSprite::BUTTON_STYLE & style,
			EDescription & descStyle,
			EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] ) ;
	// scroll_bar 用スタイル読み込み
	ESLError GetScrollBarStyle
		( EScrollBarSprite::BAR_STYLE & style,
			EDescription & descStyle,
			EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] ) ;
	// edit_text 用スタイル読み込み
	ESLError GetEditTextStyle
		( ETextEditSprite::EDIT_STYLE & style,
			EDescription & descStyle,
			EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] ) ;
	static void GetEditTextFontStyle
		( LOGFONT& lfFont, EDescription & descFont ) ;
	// list_view 用スタイル読み込み
	ESLError GetListViewStyle
		( EListViewSprite::LIST_STYLE & style, EDescription & descStyle ) ;
	// combo_list 用スタイル読み込み
	ESLError GetComboListStyle
		( EComboListSprite::COMBO_STYLE & style,
			EDescription & descStyle, EStreamBuffer bufImage ) ;

public:
	// 画像リソースを参照する
	EGL_IMAGE_RECT * GetImageResource
		( const wchar_t * pwszRes,
			EGLAnimation *& pAnime, EGL_IMAGE_RECT & irectClip ) ;
	// 静画像リソースを参照する
	PEGL_IMAGE_INFO GetStillImageResource
		( const wchar_t * pwszRes, PEGL_IMAGE_INFO pImage ) ;

} ;


#endif
