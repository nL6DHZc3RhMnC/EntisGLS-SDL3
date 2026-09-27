
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2012 Leshade Entis. All rights reserved.
 ****************************************************************************/


#if	!defined(__WINDOW_BASE_H__)
#define	__WINDOW_BASE_H__	1

#include <mmsystem.h>
#include <XInput.h>
//#pragma comment( lib, "Xinput.lib" )
#pragma comment( lib, "XInput9_1_0.lib" )

class	EInputFilter ;
class	EWindowInterface ;
class	EWindowSpriteInterface ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ基底クラス
//////////////////////////////////////////////////////////////////////////////

class	EWindow	: public	ESLObject
{
public:
	// 構築関数
	EWindow( EWindowInterface * pWUI = NULL ) ;
	EWindow( HWND hWnd, EWindowInterface * pWUI ) ;
	// 消滅関数
	virtual ~EWindow( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWindow, ESLObject )

protected:
	HWND				m_hWnd ;
	EInputFilter *		m_pFilter ;
	EWindowInterface *	m_pWUI ;
	WNDPROC				m_wpDefProc ;
	CRITICAL_SECTION	m_csWUI ;

public:
	// ウィンドウハンドル取得
	operator HWND ( void ) const
		{
			return	m_hWnd ;
		}
	// ウィンドウハンドル関連付け
	void AttachWindowHandle( HWND hWnd )
		{
			m_hWnd = hWnd ;
		}
	// インターフェース取得
	EWindowInterface * GetInterface( void ) const
		{
			return	m_pWUI ;
		}
	// インターフェース設定
	void SetInterface( EWindowInterface * pWUI ) ;
	// フィルター設定
	EInputFilter * GetInputFilter( void ) const
		{
			return	m_pFilter ;
		}
	// フィルター設定
	void SetInputFilter( EInputFilter * pFilter ) ;

protected:
	// ウィンドウコールバック関数
	static LRESULT CALLBACK WindowCallbackProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// ウィンドウプロシージャ
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// デフォルトウィンドウプロシージャ
	virtual LRESULT DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) ;

public:
	// ウィンドウ作成
	virtual ESLError Create
		( const char * pszClassName, const char * pszWindowName,
			DWORD dwStyle, DWORD dwExStyle,
			int x, int y, int nWidth, int nHeight,
			HWND hWndParent, HMENU hMenu, HINSTANCE hInstance ) ;
	// ウィンドウ削除
	virtual ESLError DestroyWindow( void ) ;

	// ウィンドウクラス登録
	static ATOM RegisterClass( const WNDCLASS & wc ) ;
	static ATOM RegisterWindowClass
		( const char * pszClassName,
			UINT uStyle, HINSTANCE hInstance, HICON hIcon = NULL,
			HCURSOR hCursor = NULL, HBRUSH hbrBackground = NULL ) ;
	// ウィンドウクラス解除
	static BOOL UnregisterClass
		( const char * pszClassName, HINSTANCE hInstance ) ;

public:
	// ウィンドウスタイル
	DWORD GetStyle( void ) const ;
	DWORD GetExStyle( void ) const ;
	BOOL SetStyle( DWORD dwStyle ) ;
	BOOL SetExStyle( DWORD dwExStyle ) ;

	// ウィンドウアイコン
	HICON GetIcon( void ) const ;
	void SetIcon( HICON hIcon ) ;

	// マウスキャプチャー
	static HWND GetCapture( void ) ;
	HWND SetCapture( void ) ;
	static BOOL ReleaseCapture( void ) ;

	// ウィンドウリージョン
	int GetWindowRgn( HRGN hRgn ) const ;
	int SetWindowRgn( HRGN hRgn, BOOL bRedraw ) ;

	// ウィンドウ位置
	BOOL MoveWindow
		( int x, int y, int nWidth, int nHeight, BOOL bRepaint ) ;
	BOOL SetWindowPos
		( HWND hInsertAfter, int x, int y, int cx, int cy, UINT uFlags ) ;
	BOOL GetWindowPlacement( WINDOWPLACEMENT * lpwndpl ) const ;
	BOOL SetWindowPlacement( const WINDOWPLACEMENT * lpcwndpl ) ;
	BOOL GetWindowRect( LPRECT lpRect ) const ;
	BOOL GetClientRect( LPRECT lpRect ) const ;

	// 描画
	HDC BeginPaint( PAINTSTRUCT * lpPaint ) ;
	BOOL EndPaint( const PAINTSTRUCT * lpPaint ) ;
	HDC GetDC( void ) ;
	int ReleaseDC( HDC hDC ) ;
	BOOL InvalidateRect( const RECT * lpRect, BOOL bErase ) ;
	BOOL GetUpdateRect( RECT * lpRect, BOOL bErase = FALSE ) ;
	BOOL UpdateWindow( void ) ;
	BOOL ShowWindow( int nCmdShow ) ;

	// 座標変換
	BOOL ClientToScreen( LPPOINT lpPoint ) const ;
	BOOL ScreenToClient( LPPOINT lpPoint ) const ;

	// スクロール
	BOOL ScrollWindow
		( int xAmount, int yAmount,
			const RECT * lpRect = NULL, const RECT * lpcClip = NULL ) ;
	BOOL ScrollWindowEx
		( int xAmount, int yAmount,
			const RECT * lpRect = NULL, const RECT * lpcClip = NULL,
			HRGN hrgnUpdate = NULL, LPRECT prcUpdate = NULL,
			UINT flags = SW_INVALIDATE ) ;

	// ウィンドウタイマー
	UINT SetTimer
		( UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc = NULL ) ;
	BOOL KillTimer( UINT nIDEvent ) ;

	// メッセージボックス
	int MessageBox
		( LPCSTR lpText, LPCSTR lpCaption = NULL, UINT uType = MB_OK ) ;

	// メッセージ関数
	BOOL PostMessage( UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0 ) ;
	LRESULT SendMessage( UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0 ) ;
	LRESULT SendMessageTimeout
		( UINT uMsg, WPARAM wParam, LPARAM lParam,
			UINT uFlags, UINT uTimeout, LPDWORD lpdwResult ) ;

	friend	EInputFilter ;
	friend	EWindowInterface ;
} ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウユーザーインターフェース
//////////////////////////////////////////////////////////////////////////////

class	EInputFilter	: public	ESLObject
{
public:
	// 構築関数
	EInputFilter( void ) ;
	// 消滅関数
	virtual ~EInputFilter( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EInputFilter, ESLObject )

public:
	// フィルターフラグ
	enum	FilterFlag
	{
		fmNormal		= 0x0000,	// 何もしない
		fmJoyMouse		= 0x0001,	// マウス入力をジョイスティック入力に変換
		fmStickMouse	= 0x0002,	// ジョイスティック入力をマウス入力に変換
		fmStickKey		= 0x0004,	// ジョイスティック入力をキー入力に変換
		fmTransparent	= 0x0100,	// 変換元のメッセージを透過させる
		fmContextKey	= 0x0200,	// 特殊ーキー押下状態をキー判定に仕様
	} ;
	// 入力デバイス
	enum	InputDevice
	{
		idKeyboard,	idMouse, idJoyStick, idCommand, idSignalCommand,
	} ;
	// ジョイスティックデバイス
	enum	JoyStickDevice
	{
		joyStickId1,
		joyStickId2,
		joyStickXInput1,
		joyStickXInput2,
		joyStickXInput3,
		joyStickXInput4,
		joyStickMaxCount,
		deviceXInputCount	= 4,
	} ;
	// ジョイスティックボタン
	enum	JoyButton
	{
		jbUp,	jbDown,	jbLeft,	jbRight,
		jbButton1,	jbButton2,	jbButton3,	jbButton4,
		jbButtonMax	= jbButton1 + 32
	} ;
	// キーボード特殊キー押下マスク
	enum	ContextKeyMask
	{
		ckmShift		= 0x1000,
		ckmControl		= 0x2000,
		ckmMenu			= 0x4000,
		ckmCaptal		= 0x8000,
		ckmKeyCodeMask	= 0x00FF,
	} ;
	#if(_WIN32_WINNT < 0x0500)
	enum	ExtendedKeyCode
	{
		VK_XBUTTON1			= 0x05,
		VK_XBUTTON2			= 0x06,
	} ;
	enum	ExtendedMouseMessage
	{
		WM_XBUTTONDOWN		= 0x020B,
		WM_XBUTTONUP		= 0x020C,
		WM_XBUTTONDBLCLK	= 0x020D,
	} ;
	#endif
	// ボタンマスク
	enum	ButtonMask
	{
		bmPushed		= 0x00000001,
		bmPushedMask	= 0x7FFFFFFF,
		bmPushing		= 0x80000000
	} ;
	// ボタン状態
	struct	BUTTON_STATUS
	{
		long int	nStatus[jbButtonMax] ;
	} ;
	// 入力情報
	struct	INPUT_EVENT
	{
		InputDevice	idType ;		// デバイスの種類
		int			iDevNum ;		// デバイスの番号（ジョイスティック）
		int			iKeyNum ;		// 仮想キーコード／ジョイボタン番号
		EWideString	wstrCommand ;	// コマンド
		int			nPriority ;		// コマンド優先度

		INPUT_EVENT( void ) : idType(idKeyboard), iDevNum(0), iKeyNum(0), nPriority(0) { }
		INPUT_EVENT( InputDevice id, int iDev, int iKey )
			: idType(id), iDevNum(iDev), iKeyNum(iKey), nPriority(0) { }
		INPUT_EVENT( InputDevice id, int iDev, int iKey, const wchar_t * pwszCmdID )
			: idType(id), iDevNum(iDev), iKeyNum(iKey), wstrCommand( pwszCmdID ), nPriority(0) { }
		INPUT_EVENT( const INPUT_EVENT & ie )
			: idType(ie.idType), iDevNum(ie.iDevNum), iKeyNum(ie.iKeyNum),
				wstrCommand(ie.wstrCommand), nPriority(ie.nPriority) { }
		INPUT_EVENT & operator = ( const INPUT_EVENT & ie )
			{
				idType = ie.idType ;
				iDevNum = ie.iDevNum ;
				iKeyNum = ie.iKeyNum ;
				wstrCommand = ie.wstrCommand ;
				nPriority = ie.nPriority ;
				return	*this ;
			}
		int Compare( const INPUT_EVENT & ie ) const
			{
				if ( idType < ie.idType )
					return	-1 ;
				else if ( idType > ie.idType )
					return	1 ;
				if ( iDevNum < ie.iDevNum )
					return	-1 ;
				else if ( iDevNum > ie.iDevNum )
					return	1 ;
				if ( iKeyNum < ie.iKeyNum )
					return	-1 ;
				else if ( iKeyNum > ie.iKeyNum )
					return	1 ;
				return	wstrCommand.Compare( ie.wstrCommand ) ;
			}
		bool operator == ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) == 0) ;	}
		bool operator != ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) != 0) ;	}
		bool operator < ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) < 0) ;	}
		bool operator <= ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) <= 0) ;	}
		bool operator > ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) > 0) ;	}
		bool operator >= ( const INPUT_EVENT & ie ) const
			{	return	(Compare(ie) >= 0) ;	}
	} ;

protected:
	EWindow *		m_pAttachedWnd ;	// ウィンドウ
	EWindowSpriteInterface *
					m_pAttachedItf ;
	DWORD			m_dwFlags ;			// フィルターフラグ
	UINT			m_nBeginFilter ;	// キャプチャー開始メッセージ

	ETagSortArray<INPUT_EVENT,INPUT_EVENT>
					m_tsaFilter ;		// フィルター
	EObjArray<BUTTON_STATUS>
					m_lstVirtPad ;		// 仮想ジョイパッド
	EObjArray<INPUT_EVENT>
					m_queInputEvent ;	// 入力待ち行列

	unsigned int	m_nInputLimit ;		// 入力待ち行列の最大サイズ
	HANDLE			m_hInputEvent ;		// 入力イベントがある

	DWORD			m_fJoyCaptured ;	// ジョイスティックを使用中
	REAL32			m_rJoyThreshold ;	// ジョイスティックの閾値
	E3D_VECTOR4		m_vJoyPos[joyStickMaxCount] ;		// ジョイスティックの現在位置
	BUTTON_STATUS	m_jsJoyStatus[joyStickMaxCount] ;	// ジョイスティックの状態
	JOYCAPS			m_jcJoyCaps[joyStickMaxCount] ;		// ジョイスティックデバイス情報

	DWORD			m_maskXInputDev ;
	XINPUT_STATE	m_xinState[deviceXInputCount] ;

	EGL_POINT		m_ptMouseBase ;		// マウスの基準位置
	int				m_nMouseThreshold ;	// マウス移動の閾値

	DWORD			m_dwContextKeyMask ;// 特殊キー押下状態

	CRITICAL_SECTION	m_cs ;

public:
	// ウィンドウメッセージを処理
	virtual bool ProcessMessage( MSG & msg ) ;
protected:
	// ジョイスティックイベントを処理
	void ProcessJoystick( int iJoyNum ) ;
	// イベントを処理
	bool ProcessEvent( const INPUT_EVENT & eiInput, bool fPushed ) ;
	// 現在の特殊キー押下状態マスクを取得
	static DWORD QueryCurrentContextKeyMask( void ) ;

protected:
	// EWindow オブジェクトにアタッチされた
	virtual void OnAttachedWindow( EWindow * pAttachedWnd ) ;
	// EWindow オブジェクトにデタッチされた
	virtual void OnDetachedWindow( EWindow * pDetachedWnd ) ;
public:
	// ウィンドウ取得
	virtual EWindow * GetWindow( void ) const ;
	virtual EWindowSpriteInterface * GetWindowInterface( void ) const ;

public:
	// フィルタを読み込む
	ESLError LoadInputFilter( EDescription & dscFilter ) ;
	// フィルタを書き出す
	ESLError SaveInputFilter( EDescription & dscFilter ) ;
	// フィルタの内容を初期化する
	void DeleteInputFilter( void ) ;
protected:
	// 入力イベントをタグから読み込む
	ESLError LoadInputEvent
		( INPUT_EVENT & ieInput, EDescription & dscEvent ) ;
	// 入力イベントをタグへ設定する
	ESLError SaveInputEvent
		( EDescription & dscEvent, const INPUT_EVENT & ieInput ) ;

public:
	// フィルターフラグを取得
	DWORD GetFilterFlags( void ) const
		{
			return	m_dwFlags ;
		}
	// フィルターフラグを設定
	void SetFilterFlags( DWORD dwFlags ) ;
	// ジョイスティックの閾値を設定する
	void SetStickThreshold( REAL32 rThreshold ) ;
	// マウス入力からスティック入力への変換を設定する
	void SetMouseThreshold( EGL_POINT ptBase, int nThreshold ) ;
	// 仮想ジョイスティックの総数を設定
	ESLError SetJoyStickCount( int nJoyCount ) ;
	// インストールされているジョイスティックの数を取得
	static unsigned int GetInstalledJoyCount( void ) ;

public:
	// フィルター処理開始
	ESLError OpenFilter( EWindow * pWnd ) ;
	ESLError OpenFilter( EWindowSpriteInterface * pWnd ) ;
	// フィルター処理終了
	ESLError CloseFilter( void ) ;

protected:
	// フィルター開始処理
	ESLError BeginFilter( void ) ;
	// フィルター終了処理
	ESLError EndFilter( void ) ;

public:
	// XInput ポーリング
	void PollXInputState
		( E3D_VECTOR4& vPos, BUTTON_STATUS& bsState,
			size_t iXInput, bool fProcessEvent = true ) ;

public:
	// フィルタ追加
	ESLError AddFilter
		( const INPUT_EVENT & ieInput, const INPUT_EVENT & ieOutput ) ;
	// フィルタ削除
	ESLError RemoveFilter( const INPUT_EVENT & ieInput ) ;
	// フィルタ取得
	INPUT_EVENT * GetFilter( const INPUT_EVENT & ieInput ) ;
	// フィルタ列挙
	ESLError EnumFilter
		( EObjArray<INPUT_EVENT> & lstInput, const INPUT_EVENT & ieOutput ) ;
	// フィルタ取得
	ETagSortArray<INPUT_EVENT,INPUT_EVENT> & Filter( void )
		{
			return	m_tsaFilter ;
		}

public:
	// 入力イベント追加
	ESLError PushInputEvent( const INPUT_EVENT & ieInput ) ;
	// 入力イベントを取得
	ESLError GetInputEvent
		( INPUT_EVENT & ieEvent,
			DWORD dwTimeout = INFINITE, bool fEscMsg = true ) ;
	// 入力イベント待ち行列を初期化
	ESLError FlushInputQueue( int nLimit ) ;

public:
	// 現在使用中のジョイスティックを取得
	DWORD GetCapturedJoyStick( void ) const
		{
			return	m_fJoyCaptured ;
		}
	// 現在のスティック座標を取得
	ESLError GetStickPosition( E3D_VECTOR4 & vPos, int iDevNum = 0 ) ;
	ESLError GetStickPosition( E3D_VECTOR & vPos, int iDevNum = 0 ) ;
	// 仮想ジョイスティックのボタンの現在の状態を取得
	bool IsJoyButtonPushing( int iKeyNum, int iDevNum = 0 ) ;
	// 仮想ジョイスティックのボタンが押された回数を取得
	int GetJoyButtonPushed( int iKeyNum, int iDevNum = 0 ) ;
	// 仮想ジョイスティックのボタンの押下回数をリセット
	ESLError FlushJoyButtonPushed( int iDevNum = 0, int iKeyNum = -1 ) ;
	// 仮想ジョイスティックのボタンの押下状態をリセット
	ESLError ResetJoyButtonPushing( int iDevNum = 0, int iKeyNum = -1 ) ;

public:
	// スレッド排他処理
	void Lock( void ) ;
	void Unlock( void ) ;

	friend	EWindow ;
} ;

class	EWindowInterface	: public	ESLObject
{
public:
	// 構築関数
	EWindowInterface( void ) ;
	// 消滅関数
	virtual ~EWindowInterface( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWindowInterface, ESLObject )

protected:
	EWindow *	m_pAttachedWnd ;

protected:
	// EWindow オブジェクトにアタッチされた
	virtual void OnAttachedWindow( EWindow * pAttachedWnd ) ;
	// EWindow オブジェクトにデタッチされた
	virtual void OnDetachedWindow( EWindow * pDetachedWnd ) ;
public:
	// ウィンドウ取得
	virtual EWindow * GetWindow( void ) const ;

protected:
	// ウィンドウプロシージャ
	virtual LRESULT WindowProc
		( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// メッセージ事前変換関数
	virtual int PreTranslateMessage( MSG & msg ) ;

	friend	EWindow ;
} ;


//////////////////////////////////////////////////////////////////////////////
// スレッドオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EGLSThread	: public	ESLThread
{
public:
	// 構築関数
	EGLSThread( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGLSThread, ESLThread )

protected:
	DWORD	m_dwThreadTime ;	// スレッド開始時間
	DWORD	m_dwBeginTime ;		// 計測用基準時間
	int		m_nTimeRange ;
	int		m_nTotalTime ;

protected:
	// スレッド開始時
	virtual void OnBeginThread( void ) ;
	// スレッド終了時
	virtual void OnEndThread( void ) ;

public:
	// 時間計測
	DWORD GetThreadTime( void ) const ;
	// 局所時間計測開始
	void BeginTime( int nTotalTime, int nRange ) ;
	// 局所時間計測
	int GetOffsetTime( void ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// ディスプレイモード
//////////////////////////////////////////////////////////////////////////////

class	EDisplayMode	: public ESLObject, public DEVMODE
{
public:
	// 構築関数
	EDisplayMode( void ) ;
	// 消滅関数
	virtual ~EDisplayMode( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EDisplayMode, ESLObject )

protected:
	int		m_fChanged ;

	typedef HMONITOR
		(WINAPI *API_MonitorFromRect)
			( IN LPCRECT lprc, IN DWORD dwFlags ) ;
	typedef BOOL
		(WINAPI *API_GetMonitorInfo)
			( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi ) ;
	typedef LONG (WINAPI *API_ChangeDisplaySettingsEx)
			( IN LPCSTR lpszDeviceName,
				IN LPDEVMODEA lpDevMode,
				IN HWND hwnd, IN DWORD dwflags, IN LPVOID lParam ) ;

	HMODULE						m_hUser32 ;
	API_MonitorFromRect			m_apiMonitorFromRect ;
	API_GetMonitorInfo			m_apiGetMonitorInfo ;
	API_ChangeDisplaySettingsEx	m_apiChangeDisplaySettingsEx ;

public:
	// user32.dll をロードして各種 API を初期化する
	void PrepareMonitorAPIs( void ) ;
	// ディスプレイを取得
	const char * GetDisplayNameFromRect
		( EString & strDisplayName,
			const RECT * pRect, HMONITOR * phMonitor = NULL ) ;
	// ディスプレイの矩形を取得
	ESLError GetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpmi ) ;
	// ウィンドウ位置を正規化（画面内へ補正）
	void NormalizeWindowPos
		( POINT & posWindow,
			SIZE & sizeWindow, bool fChangeSize = false ) ;
	// DirectDraw 用ディスプレイの GUID を取得する
	GUID * GetDDMonitorGUID
		( GUID * pGUID, HMONITOR hMonitor, int * pGetIndex = NULL ) ;

public:
	// ディスプレイのビット深度を取得
	static unsigned int GetDisplayColorMode
		( const char * pszDisplayName = NULL ) ;
	// ディスプレイの周波数を取得
	static unsigned int GetDisplayFrequency
		( const char * pszDisplayName = NULL ) ;
	// ディスプレイモードを列挙する
	static EObjArray<DEVMODE> EnumDisplayMode
		( DWORD dwWidth, DWORD dwHeight,
			DWORD dwBitsPerPixel, DWORD dwFrequency = 0,
			const char * pszDisplayName = NULL ) ;

public:
	// 指定モードに切り替え可能かテストする
	ESLError TestDisplayMode
		( DWORD dwWidth, DWORD dwHeight,
			DWORD dwBitsPerPixel = 0, DWORD dwFrequency = 0,
			const char * pszDisplayName = NULL ) ;
	// ディスプレイモードを切り替える
	ESLError ChangeDisplayMode( const char * pszDisplayName = NULL ) ;
	// ディスプレイモードを元に戻す
	ESLError RestoreDisplayMode( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// レジストリキーオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ERegistryKey	: public	ESLObject
{
public:
	// 構築関数
	ERegistryKey( void ) ;
	// 消滅関数
	virtual ~ERegistryKey( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ERegistryKey, ESLObject )

protected:
	HKEY	m_hKey ;
	int		m_fOpened ;

public:
	// キー取得
	operator HKEY ( void ) const
		{
			return	m_hKey ;
		}

public:
	// レジストリキーを作成
	ESLError CreateKey
		( HKEY hKey, const char * pszSubKey,
					REGSAM samDersired = KEY_ALL_ACCESS ) ;
	// レジストリキーを削除
	static ESLError DeleteKey( HKEY hKey, const char * pszSubKey ) ;
	// レジストリキーを開く
	ESLError OpenKey
		( HKEY hKey, const char * pszSubKey,
					REGSAM samDersired = KEY_ALL_ACCESS ) ;
	// レジストリキーを閉じる
	void CloseKey( void ) ;
	// 値を削除する
	ESLError DeleteValue( const char * pszValueName = NULL ) ;
	// 値をセットする
	ESLError SetBinary
		( const char * pszValueName,
			const void * lpData, DWORD dwBytes ) ;
	ESLError SetInteger( const char * pszValueName, unsigned int nInteger ) ;
	ESLError SetInteger64( const char * pszValueName, INT64 nInteger ) ;
	ESLError SetString( const char * pszValueName, const char * pszString ) ;
	ESLError SetDoubleReal( const char * pszValueName, double nReal ) ;
	// 値を取得する
	unsigned int GetBinary
		( const char * pszValueName, void * lpData, DWORD dwBytes ) const ;
	int GetInteger( const char * pszValueName, int nDefValue = 0 ) const ;
	INT64 GetInteger64( const char * pszValueName, INT64 nDefValue = 0 ) const ;
	EString GetString
		( const char * pszValueName, const char * pszDefString = NULL ) const ;
	double GetDoubleReal
		( const char * pszValueName, double nDefValue = 0 ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 画面描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EGLDrawImage	: public	ESLObject
{
public:
	// 構築関数
	EGLDrawImage( void ) ;
	// 消滅関数
	virtual ~EGLDrawImage( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGLDrawImage, ESLObject )

public:
	// 描画フラグ
	enum	DrawFlag
	{
		dfDirectDraw		= 0x0001,
		dfWaitVerticalBlank	= 0x0002,
	} ;
	// DirectDraw オブジェクト通知インターフェース
	class	INotify
	{
	public:
		// DirectDraw オブジェクトが削除される前に呼び出される
		virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) = 0 ;
		// DirectDraw オブジェクトが作成された後に呼び出される
		virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) = 0 ;
	} ;
	class	ISmartPtrNotify	: public INotify
	{
	protected:
		long int	m_nRef ;
	public:
		ISmartPtrNotify( void ) : m_nRef(1) {}
		virtual ~ISmartPtrNotify( void ) {}
		long int GetRefCount( void ) const
			{
				return	m_nRef ;
			}
		void AddRef( void )
			{
				if ( this != NULL )
				{
					::InterlockedIncrement( &m_nRef ) ;
				}
			}
		void Release( void )
			{
				if ( (this != NULL)
					&& ::InterlockedDecrement( &m_nRef ) == 0 )
				{
					delete	this ;
				}
			}
	} ;
	// EGLDrawImage で使用する DirectDraw サーフェスオブジェクト
	class	IDD7Surface : public ISmartPtrNotify
	{
	protected:
		EGLDrawImage *					m_pdiServer ;
		struct IDirectDrawSurface7 *	m_iddsuf ;
		struct _DDSURFACEDESC2 *		m_pddsd ;
	public:
		// 構築関数
		IDD7Surface( void )
			: m_pdiServer(NULL), m_iddsuf(NULL), m_pddsd(NULL) {}
		// 消滅関数
		virtual ~IDD7Surface( void ) ;
		// IDirectDrawSurface7 インターフェース取得
		operator struct IDirectDrawSurface7 * ( void ) const
			{
				return	m_iddsuf ;
			}
		// DirectDraw オブジェクトが削除される前に呼び出される
		virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) ;
		// DirectDraw オブジェクトが作成された後に呼び出される
		virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) ;
		// IDirectDrawSurface7 分離し、IDD7Surface を削除
		struct IDirectDrawSurface7 * SmartDetach( void ) ;
		// IDirectDrawSurface7 オブジェクトを関連付ける
		void AttachSurface
			( EGLDrawImage * pdiServer,
				struct IDirectDrawSurface7 * iddsuf ) ;
	} ;

protected:
	CRITICAL_SECTION				m_csSync ;
	EPtrObjArray<INotify>			m_lstNotify ;
	GUID *							m_pguidDDDevice ;	// DirectDraw::Initialize
	GUID							m_guidDDDevice ;
	struct IDirectDraw7 *			m_iddraw7 ;			// DirectDraw
	struct IDirectDraw *			m_iddraw ;			// DirectDraw
	struct IDirectDrawClipper *		m_iddclip ;			// DirectDrawClipper
	struct IDirectDrawSurface *		m_iddsufPrimary ;	// DirectDrawSurface
	struct IDirectDrawSurface7 *	m_idd7sufPrimary ;
	struct IDirectDrawSurface *		m_iddsufSecondary ;
	struct IDirectDrawSurface7 *	m_idd7sufSecondary ;
	struct IDirectDrawSurface7 *	m_idd7sufStereoLeft ;
	//
	static HMODULE					m_hModuleD3D9 ;
	struct IDirect3D9 *				m_id3d9 ;
	struct IDirect3DDevice9 *		m_id3d9Dev ;
	struct _D3DPRESENT_PARAMETERS_ *m_pd3dpp ;
	struct IDirect3DSurface9 *		m_idds9DispBuf ;
	HWND							m_hWndDevD3D9 ;
	EGLSize							m_sizeD3D9DispBuf ;
	HEGL_DRAW_IMAGE					m_hDraw ;
	//
	DWORD							m_dwVSyncLimitScanLine ;
	DWORD							m_dwMaxScanLine ;
	//
	bool	m_fCoInitialized ;
	bool	m_fFullscreen ;
	bool	m_fStereo3D ;

public:
	// 画像描画オブジェクトを初期化する
	ESLError Initialize( GUID * pGUID = NULL ) ;
	// DirectX オブジェクトを生成する
	ESLError CreateDirectDraw( GUID * pGUID = NULL ) ;
	// ウィンドウモード初期化
	ESLError CreateSurfaceWindowMode( void ) ;
	// フルスクリーンモード初期化
	ESLError CreateSurfaceFullscreenMode
		( HWND hWnd, int nWidth, int nHeight,
			int nBitsPerPixel, int nFrequency, bool fStereo3D = false ) ;
	// Direct3D9 初期化
	ESLError CreateDirect3D9Device
		( HWND hWnd, int nWidth, int nHeight,
				int nAdapter = 0, BOOL fWindowed = FALSE ) ;
	// Direct3D9 リセット
	ESLError ResetDirect3D9Device( void ) ;
	// ウィンドウサイズ変更
	ESLError OnResizeWindow( void ) ;
	// 画像描画オブジェクトを終了する
	ESLError Release( void ) ;
	// サーフェースを開放する（ディスプレイモード復帰）
	ESLError ReleaseSurface( void ) ;

public:
	// ステレオ3D表示モードをテストする
	bool TestStereo3DGraphic
		( struct _DDSURFACEDESC2 & ddsdSupported,
			int nWidth, int nHeight,
			int nBitsPerPixel, int nFrequency ) const ;
	// ステレオ3D表示モードがサポートされているか？
	bool IsSupportedStereo3DGraphic( void ) const ;
	// DirectX 9 がインストールされているか？
	static bool IsInstalledDirectX9( void ) ;
	// フルスクリーンモードか？
	bool IsFullscreenMode( void ) const
		{
			return	m_fFullscreen ;
		}
	// ステレオ3Dモードか？
	bool IsStereo3DMode( void ) const
		{
			return	m_fStereo3D ;
		}

public:
	enum	DDSurfaceCaps
	{
	#if	!defined(DDSCAPS_OFFSCREENPLAIN)
		DDSCAPS_OFFSCREENPLAIN	= 0x00000040,
	#endif
	#if	!defined(DDSCAPS_3DDEVICE)
		DDSCAPS_3DDEVICE		= 0x00002000,
	#endif
	#if	!defined(DDSCAPS_VIDEOMEMORY)
		DDSCAPS_VIDEOMEMORY		= 0x00004000,
	#endif
//	#if	!defined(DDCAPS_STEREOVIEW)
//		DDCAPS_STEREOVIEW		= 0x00040000,
//	#endif
	} ;
	// 画像描画のためにサーフェスを作成する
	IDirectDrawSurface * CreateSurfaceOnVRAM( int nWidth, int nHeight ) ;
	// DirectDraw7 サーフェスを作成する
	IDirectDrawSurface7 * CreateDD7Surface
		( int nWidth, int nHeight,
			DWORD dwCaps = DDSCAPS_OFFSCREENPLAIN
							| DDSCAPS_3DDEVICE
							| DDSCAPS_VIDEOMEMORY, DWORD dwCaps2 = 0 ) ;
	IDD7Surface * CreateSmartDD7Surface
		( int nWidth, int nHeight,
			DWORD dwCaps = DDSCAPS_OFFSCREENPLAIN
							| DDSCAPS_3DDEVICE
							| DDSCAPS_VIDEOMEMORY, DWORD dwCaps2 = 0 ) ;
	IDD7Surface * CreateSmartDD7Surface( const struct _DDSURFACEDESC2 & ddsd ) ;
	// 画像描画のための初期化
	ESLError InitializeDrawImage( int nWidth, int nHeight ) ;
	// 画像をクライアント領域に描画する
	ESLError DrawImageToDisplay
		( HWND hwndTarget, PCEGL_IMAGE_INFO pImageInf,
			int xPos, int yPos, const EGL_SIZE * pDstSize = NULL,
			const EGL_RECT * pSrcRect = NULL, DWORD fdwFlags = 0,
			struct IDirectDrawSurface * pddsVRAM = NULL ) ;
	// ステレオ画像を描画する
	ESLError DrawStereoImageToDisplay
		( HWND hwndTarget,
			PCEGL_IMAGE_INFO pImageLeft,
			PCEGL_IMAGE_INFO pImageRight,
			int xPos, int yPos, const EGL_SIZE * pDstSize = NULL,
			const EGL_RECT * pSrcRect = NULL, DWORD fdwFlags = 0 ) ;
	ESLError DrawStereoImageToDisplay
		( HWND hwndTarget,
			struct IDirectDrawSurface7 * pddsLeft,
			struct IDirectDrawSurface7 * pddsRight,
			int xPos, int yPos, const EGL_SIZE * pDstSize,
			const EGL_RECT * pSrcRect = NULL, DWORD fdwFlags = 0 ) ;
	// Surface 間描画
	ESLError BltSurface
		( IDirectDrawSurface7 * pddsDst,
			IDirectDrawSurface7 * pddsSrc,
			int xDst, int yDst, const EGL_SIZE * pDstSize,
			const EGL_RECT * pSrcRect = NULL, bool fAsync = true ) ;
	ESLError BltSurface
		( IDirectDrawSurface * pddsDst,
			IDirectDrawSurface * pddsSrc,
			int xDst, int yDst, const EGL_SIZE * pDstSize,
			const EGL_RECT * pSrcRect = NULL, bool fAsync = true ) ;
	//　サーフェスをフリップする
	ESLError Flip( void ) ;
	// VSYNC を待つ
	ESLError WaitForVerticalBlank( void ) ;
	// VSYNC 用パラメータをリセットする
	void ResetVSync( void ) ;

public:
	// Direct3D9 画面描画
	ESLError DrawImageToDisplayD3D9
		( PCEGL_IMAGE_INFO pImageInf,
			int xPos, int yPos,
			const EGL_SIZE * pDstSize = NULL,
			const EGL_RECT * pSrcRect = NULL ) ;

public:
	// 同期処理
	void Lock( void ) ;
	void Unlock( void ) ;
	// 通知オブジェクトを追加する
	void AddNotify( INotify * pNotify ) ;
	// 通知を解除する
	void DetachNotify( INotify * pNotify ) ;
protected:
	// OnReleaseDirectDraw を通知する
	void NotifyOnReleaseDirectDraw( void ) ;
	// OnCreateDirectDraw を通知する
	void NotifyOnCreateDirectDraw( void ) ;

public:
	// DirectDraw オブジェクトを取得する
	struct IDirectDraw * GetDirectDraw( void ) const
		{
			return	m_iddraw ;
		}
	struct IDirectDraw7 * GetDirectDraw7( void ) const
		{
			return	m_iddraw7 ;
		}
	struct IDirect3D9 * GetDirect3D9( void ) const
		{
			return	m_id3d9 ;
		}
	struct IDirect3DDevice9 * GetDirect3DDevice9( void ) const
		{
			return	m_id3d9Dev ;
		}
	// プライマリサーフェスを取得する
	struct IDirectDrawSurface * GetPrimarySurface( void ) const
		{
			return	m_iddsufPrimary ;
		}
	struct IDirectDrawSurface7 * GetDD7PrimarySurface( void ) const
		{
			return	m_idd7sufPrimary ;
		}
	// バックサーフェスを取得する
	struct IDirectDrawSurface * GetSecondarySurface( void ) const
		{
			return	m_iddsufSecondary ;
		}
	struct IDirectDrawSurface7 * GetDD7SecondarySurface( void ) const
		{
			return	m_idd7sufSecondary ;
		}
	struct IDirectDrawSurface7 * GetDD7SecondaryLeftSurface( void ) const
		{
			return	m_idd7sufStereoLeft ;
		}
	// クリッパを取得する
	struct IDirectDrawClipper * GetDirectDrawClipper( void ) const
		{
			return	m_iddclip ;
		}
	// DirectX の存在を調べる
	bool IsInstalledDirectX( void ) const
		{
			return	(m_iddraw != NULL) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// ゲームアクセラレーションウィンドウ
//////////////////////////////////////////////////////////////////////////////

class	EGameWindow	: public	EWindow
{
public:
	// 構築関数
	EGameWindow( EWindowInterface * pWUI = NULL ) ;
	// 消滅関数
	virtual ~EGameWindow( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGameWindow, EWindow )

public:
	// 協調レベル
	enum	CooperationLevel
	{
		flagMouseCapturing	= 0x0001,
		flagFullScreen		= 0x0002,
		flagExclusive		= 0x0004,
		levelWindow			= 0x0000,
		levelNormal			= 0x0001,
		levelFullScreen		= 0x0003,
		levelExclusive		= 0x0007
	} ;
	// オプショナル機能フラグ
	enum	OptionalFunctions
	{
		UseDblClick			= 0x00000001,
		AllowClose			= 0x00000002,
		BlackBack			= 0x00000004,
		EnableIME			= 0x00000008,
		AllowMinimize		= 0x00000010,
		GrantScreenSave		= 0x00000020,
		GrantMonitorSave	= 0x00000040,
		GrantPowerSuspend	= 0x00000080,
		VariableWindowSize	= 0x00000100,
		AllowMaximize		= 0x00000200,
		AutoWindowAscept	= 0x00000400,
		NoAutoFitSize		= 0x00000800,
		ChildWindow			= 0x00001000,
		PopupWindow			= 0x00002000,
		InvisibleWindow		= 0x00004000,
		NoNormalizePos		= 0x00008000,
		OpenIME				= 0x00010000,
		DoMinimize			= 0x00020000,
		DoMaximize			= 0x00040000,
		StatusFlagMask		= 0xFF00FFFF,
	} ;

protected:
	EString				m_strWndClass ;			// Name of window class
	POINT				m_pointOffset ;			// Display offset position
	SIZE				m_sizeScreen ;			// Actually printed image size
	SIZE				m_sizeFullScreen ;		// Full screen size
	SIZE				m_sizeDisplay ;			// Display size
	unsigned int		m_nBitsPerPixel ;		// Bits per pixel
	unsigned int		m_nFrequency ;			// Frequency
	unsigned int		m_flagNoChangeMode ;	// Not change mode when fullscreen
	unsigned int		m_flagCreated ;			// Created flag
	unsigned int		m_flagUnmatchedSize ;	// Unmatched window size flag
	unsigned int		m_flagActivated ;		// Active or inactive flag
	unsigned int		m_flagDeactivate ;		// Deactive flag
	signed int			m_fCooperationLevel ;	// Cooperation level
	unsigned int		m_fOptionFunctions ;	// Optional functions
	UINT				m_uMsgSetOptFlag ;		// Message of optional function
	HIMC				m_hIMC ;				// Original IMC handle
	EDisplayMode		m_DisplayMode ;			// Display mode object
	POINT				m_ptMonitorUpperLeft ;	// Position of current monitor
	RECT				m_rctNormalWndPos ;		// Position of normal window
	E3DSDisplayPlugin::I3DImageView *
								m_pivView3D ;	// 立体視表示用インターフェース
	bool				m_fFullscreenByView3D ;
	bool				m_fControlWindowByView3D ;
	bool				m_fEnableStereo3D ;

protected:
	// ウィンドウプロシージャ
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	// ウィンドウのクライアント表示座標を更新する
	void UpdateClientDisplayPosition( bool flagAdjustAspect = true ) ;
	// ウィンドウのアスペクト比を調整すべきか？
	bool ShouldAdjustWindowAscept( void ) const ;

public:
	virtual LRESULT DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) ;

protected:
	// ウィンドウがアクティブになった
	virtual void OnActivate( void ) ;
	// ウィンドウが非アクティブになった
	virtual void OnDeactivate( void ) ;

public:
	// メッセージループ
	static BOOL HandleMessage
		( DWORD dwTimeout, HWND hWnd = NULL,
			UINT uMsgFilterMin = 0, UINT uMsgFilterMax = 0 ) ;

protected:
	// ディスプレイモードを検索
	virtual ESLError ChangeDisplayMode
		( unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0, unsigned int nFrequency = 0 ) ;

public:
	// ウィンドウ作成
	virtual ESLError CreateDisplayEx
		( const char * pszWindowName,
			CooperationLevel fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel,
			unsigned int nFrequency,
			const POINT * pWindowPos,
			const SIZE * pWindowSize, HWND hwndParent ) ;
	virtual ESLError CreateDisplay
		( const char * pszWindowName,
			CooperationLevel fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0,
			unsigned int nFrequency = 0,
			const POINT * pWindowPos = NULL, HWND hwndParent = NULL ) ;
	// ウィンドウサイズ変更
	virtual ESLError ChangeDisplaySize
		( unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0, unsigned int nFrequency = 0 ) ;
	// 協調レベルを変更
	virtual ESLError ChangeCooperationLevel
			( CooperationLevel fCooperationLevel ) ;
	// ウィンドウを閉じる
	virtual ESLError CloseDisplay( void ) ;

	// 強調レベルを取得
	CooperationLevel GetCooperationLevel( void ) const ;
	// オプショナル機能フラグを取得
	unsigned int GetOptionalFuncFlag( void ) const ;
	// オプショナル機能フラグを設定
	void SetOptionalFuncFlag( unsigned int flagsOptionalFunc ) ;

protected:
	// ウィンドウスタイルを取得
	DWORD GetModifiedWindowStyle( DWORD dwStyle ) const ;
	// 拡張ウィンドウスタイルを取得
	DWORD GetModifiedWindowExStyle( DWORD dwExStyle ) const ;
	// クラススタイルを取得
	LONG GetModifiedWindowClassStyle( LONG lClassStyle ) const ;

public:
	// 画面モード変更フラグを取得
	unsigned int GetChangeDisplayModeFlag( void ) const ;
	// 画面モード変更フラグを設定
	void SetChangeDisplayModeFlag( unsigned int nWithChangeMode ) ;
	// 表示用立体視インターフェースの関連付け
	void Attach3DViewDisplay( E3DSDisplayPlugin::I3DImageView * pivView3D ) ;
	// 立体視表示インターフェースが画面モードを変更している
	bool IsFullscreenBy3DView( void ) const
		{
			return	m_fFullscreenByView3D ;
		}

public:
	// 座標変換
	BOOL ClientToScreen( LPPOINT lpPoint ) const ;
	BOOL ScreenToClient( LPPOINT lpPoint ) const ;
	// ディスプレイオフセットを取得
	const POINT & GetOffsetPos( void ) const ;
	// ディスプレイオフセットを設定
	void SetOffsetPos( const POINT & ptOffset ) ;
	// ディスプレイサイズ（渡された値）を取得
	const SIZE & GetDisplaySize( void ) const ;
	// ディスプレイモード（変更された画面サイズ）を取得
	const SIZE & GetFullscreenSize( void ) const ;
	// 表示サイズ（フルスクリーン時の伸縮サイズ）を取得
	const SIZE & GetScreenSize( void ) const ;
	// 表示サイズ（フルスクリーン時の伸縮サイズ）を設定
	void SetScreenSize( const SIZE & sizeScreen ) ;
	// ディスプレイのビット深度を取得
	unsigned int GetDisplayBitCount( void ) const ;
	// ディスプレイのリフレッシュ周波数を取得
	unsigned int GetDisplayFrequency( void ) const ;
	// ウィンドウがアクティブか？
	virtual bool IsWindowActive( void ) const ;
	// 通常時ウィンドウ位置を取得
	bool GetNormalWindowPos( RECT & rctNormalPos ) const ;
	// ウィンドウ位置を正規化（画面内へ補正）
	void NormalizeWindowPos
		( POINT & posWindow, SIZE & sizeWindow ) ;
	// ウィンドウのクライアントサイズをフィットさせる
	void FitWindowClientSize( void ) ;
	// ウィンドウのクライアントサイズを変更する
	void ChangeWindowClientSize( int nWidth, int nHeight ) ;
	// EDisplayMode 取得
	EDisplayMode& GetDisplayMode( void )
	{
		return	m_DisplayMode ;
	}

	// ウィンドウクラス名を取得
	static const char * GetWindowClassName( void ) ;
	// ウィンドウクラスを登録
	static void RegisterClass( const char * pszClassName = NULL ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// ERI アニメーションファイル再生クラス
//////////////////////////////////////////////////////////////////////////////

class	ERIAnimationPlayer	: public ERIAnimation, public EWaveStreamBuffer
{
public:
	// 構築関数
	ERIAnimationPlayer( void ) ;
	// 消滅関数
	virtual ~ERIAnimationPlayer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ERIAnimationPlayer, ERIAnimation, EWaveStreamBuffer )

public:
	// プレイ方法
	enum	PlayTypeFlag
	{
		ptfDispatchMessage		= 0x0001,
		ptfNoSkipFrame			= 0x0004,
	} ;

protected:
	EWaveOutDevice *			m_pdevWave ;
	ENumArray<HWAVEBUF>			m_queWaveData ;
	DWORD						m_dwBeginPlayingTime ;
	DWORD						m_dwPlayEndFrame ;
	DWORD						m_dwBreakWaveSamples ;
	DWORD						m_dwOutputWaveSamples ;
	bool						m_fQueueWaveOut ;
	volatile bool				m_fCancelPlaying ;
	struct IDirectDrawSurface *	m_pddsVRAM ;

protected:
	// 画像展開出力バッファ要求
	virtual EGL_IMAGE_INFO * CreateImageBuffer
		( DWORD format, SDWORD width, SDWORD height, DWORD bpp ) ;
	// 画像展開出力バッファ消去
	virtual void DeleteImageBuffer( EGL_IMAGE_INFO * peii ) ;
	// 音声出力要求
	virtual bool RequestWaveOut
		( DWORD channels, DWORD frequency, DWORD bps ) ;
	// 音声出力終了
	virtual void CloseWaveOut( void ) ;
	// 音声データ出力
	virtual void PushWaveBuffer( void * ptrWaveBuf, DWORD dwBytes ) ;

protected:
	// 音声データの再生が完了した
	virtual void OnEndPlaying
		( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声出力デバイスが次の音声バッファを要求している
	virtual HWAVEBUF OnQueueNextBuffer( EWaveOutDevice * pWaveDev ) ;

public:
	// アニメーションファイルを開く
	virtual ESLError Open
		( ESLFileObject * pFile, EWaveOutDevice * pdevWave,
			unsigned int nPreloadSize = 0, DWORD fdwFlags = 0 ) ;
	// アニメーションを再生する
	virtual ESLError PlayTo
		( unsigned int nEndFrame, HWND hwndTarget,
			int xPos = 0, int yPos = 0,
			const EGL_SIZE * pViewSize = NULL,
			DWORD fdwFlags = ptfDispatchMessage,
			EGLDrawImage * pDrawImage = NULL ) ;
	virtual ESLError Play
		( HWND hwndTarget,
			int xPos = 0, int yPos = 0,
			const EGL_SIZE * pViewSize = NULL,
			DWORD fdwFlags = ptfDispatchMessage,
			EGLDrawImage * pDrawImage = NULL ) ;
	// アニメーション再生を中断する
	virtual void CancelPlaying( void ) ;
	// アニメーション終了フレームを変更する
	virtual void SetPlayEndFrame( DWORD dwEndFrame ) ;

protected:
	// 描画処理
	virtual ESLError OnDrawMovieFrame
		( HWND hwndTarget, int xPos, int yPos,
			const EGL_SIZE * pViewSize,
			EGLDrawImage * pDrawImage,
			PCEGL_IMAGE_INFO pImage, DWORD dwDuringTime ) ;
	// 再生中のメッセージ処理
	virtual ESLError OnDispatchMessage( void ) ;
	// 時間待ち
	virtual ESLError OnWaitingTime( DWORD dwTime ) ;
	// 再生中のアニメーション時間取得
	DWORD GetCurrentPlayingTime( void ) const ;

public:
	// 音声ストリーミング開始
	virtual void BeginWaveStreaming( void ) ;
	// 音声ストリーミング終了
	virtual void EndWaveStreaming( void ) ;

protected:
	// 音声待ち行列バッファ削除
	void DeleteWaveQueueBuffer( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// Internet Session クラス
//////////////////////////////////////////////////////////////////////////////

#include <wininet.h>

class	EInternetFile ;
class	EInternetSession	: public ESLObject
{
protected:
	HINTERNET	m_hInternet ;
	EString		m_strTempFileBase ;
	DWORD		m_dwTempCacheThreshold ;

public:
	// クラス情報
	DECLARE_CLASS_INFO( EInternetSession, ESLObject )
	// 構築関数
	EInternetSession( void ) ;
	// 消滅関数
	virtual ~EInternetSession( void ) ;
	// セッションを開く
	ESLError Open
		( const char * pszAgent = NULL,
			DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS,
			const char * pszProxyName = NULL,
			const char * pszProxyBypass = NULL, DWORD dwFlags = 0 ) ;
	// セッションを閉じる
	void Close( void ) ;
	// コールバック設定
	ESLError EnableCallback( bool fCallback = true ) ;
	// オプション取得
	ESLError QueryOption
		( DWORD dwOption, void * pBuffer, DWORD * pdwBufLen ) ;
	// オプション設定
	ESLError SetOption
		( DWORD dwOption, void * pBuffer,
			DWORD dwBufferLength, DWORD dwFlags = 0 ) ;
	// クッキー設定
	static ESLError SetCookie
		( const char * pszURL,
			const char * pszCookieName, const char * pszCookieData ) ;
	// クッキー取得
	static ESLError GetCookie
		( const char * pszURL, const char * pszCookieName,
			char * pszCookieData, DWORD dwBufLen ) ;
	static DWORD GetCookieLength
		( const char * pszURL, const char * pszCookieName ) ;
	static ESLError GetCookie
		( const char * pszURL,
			const char * pszCookieName, EString & strCookieData ) ;
	// セッションハンドル取得
	operator HINTERNET ( void ) const
		{
			return	m_hInternet ;
		}

public:
	// テンポラリファイルの設定
	virtual void SetTemporaryFileInfo
		( const char * pszTempFileBase, DWORD dwSizeThreshold = 0 ) ;
	// テンポラリファイルを作成
	virtual ERawFile * CreateTemporaryFile( DWORD dwSize ) ;

protected:
	EInternetSession *			m_pisPrev ;
	EInternetSession *			m_pisNext ;
	static EInternetSession *	m_pisFirst ;
	static void CALLBACK InternetStatusCallback
		( HINTERNET hInternet, DWORD_PTR dwContext,
			DWORD dwInternetStatus,
			LPVOID lpvStatusInformation, DWORD dwStatusInformationLength ) ;
	void AddChain( void ) ;
	void DetachChain( void ) ;
	// コールバック関数
	virtual void OnStatusCallback
		( DWORD_PTR dwContext, DWORD dwInternetStatus,
			LPVOID lpvStatusInformation, DWORD dwStatusInformationLength ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// Internet File クラス
//////////////////////////////////////////////////////////////////////////////

class	EInternetFile	: public ESyncStreamFile, public EGLSThread
{
protected:
	HINTERNET		m_hConnect ;
	HINTERNET		m_hFile ;
	HANDLE			m_hThreadReady ;
	ESLFileObject *	m_pTempFile ;
	bool			m_fNoDeleteTempFile ;

public:
	// クラス情報
	DECLARE_CLASS_INFO2( EInternetFile, ESyncStreamFile, EGLSThread )
	// 構築関数
	EInternetFile( void ) ;
	// 消滅関数
	virtual ~EInternetFile( void ) ;
	// 閉じる
	void Close( void ) ;
	// URL を開く
	ESLError OpenURL
		( EInternetSession & session, const char * pszURL,
			ESLFileObject * pTempFile = NULL,
			const char * pszHeaders = NULL, DWORD dwHeaderLength = 0,
			DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT
								| INTERNET_FLAG_TRANSFER_BINARY ) ;
	// 接続する
	ESLError Connect
		( EInternetSession & session,
			const char * pszServerName,
			DWORD dwService = INTERNET_SERVICE_HTTP,
			INTERNET_PORT nServerPort = INTERNET_INVALID_PORT_NUMBER, 
			const char * pszUserName = NULL,
			const char * pszPassword = NULL, DWORD dwFlags = 0 ) ;

public:
	// ダウンロードの開始
	ESLError BeginDownload
		( unsigned long int nLength = -1,
			ESLFileObject * pTempFile = NULL ) ;
protected:
	enum	ThreadMessage
	{
		tmQuit	= WM_USER + 1,
	} ;
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;

public:
	// ダウンロードが終了するまで待つ
	ESLError WaitUntilDownload
		( DWORD dwTimeout, bool fDispMsg = false ) ;
	// ダウンロードをキャンセルする
	ESLError CancelDownload
		( DWORD dwTimeout = INFINITE, bool fDispMsg = false ) ;

} ;


class	EInternetHttpFile	: public EInternetFile
{
public:
	// クラス情報
	DECLARE_CLASS_INFO( EInternetHttpFile, EInternetFile )
	// 構築関数
	EInternetHttpFile( void ) ;
	// HTTP リクエスト送信
	ESLError OpenRequest
		( const char * pszVerb,
			const char * pszObjectName, const char * pszReferer,
			LPCTSTR* ppstrAcceptTypes, const char * pszVersion = NULL,
			DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT ) ;
	ESLError SendRequest
		( const char * pszHeaders = NULL, DWORD dwHeadersLen = 0,
			void * lpOptional = NULL, DWORD dwOptionalLen = 0 ) ;
	// 情報取得
	ESLError QueryInfo
		( DWORD dwInfoLevel, void * ptrBuffer,
			DWORD * pdwBufferLength, DWORD * pdwIndex = NULL ) ;
	// HTTP ステータスコード取得
	ESLError QueryStatusCode( DWORD & dwStatusCode ) ;
	// HTTP データ長取得
	ESLError QueryContentLength( DWORD & dwContentLength ) ;
	// HTTP データタイプ取得
	ESLError QueryContentType( EString & strType ) ;
	// HTTP データエンコーディング取得
	ESLError QueryContentTransferEncoding( EString & strEncoding ) ;
	// HTTP Date 取得
	ESLError QueryContentDate( SYSTEMTIME & systime ) ;
	// HTTP Last-Modified 取得
	ESLError QueryContentLastModified( SYSTEMTIME & systime ) ;

} ;


#endif
