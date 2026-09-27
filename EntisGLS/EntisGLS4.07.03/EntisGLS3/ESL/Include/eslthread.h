
/*****************************************************************************
                    Entis Standard Library declarations
 ----------------------------------------------------------------------------
        Copyright (c) 2010-2011 Leshade Entis. All rights reserved.
 *****************************************************************************/

#if	!defined(__ESL_THREAD_H__)
#define	__ESL_THREAD_H__	1


//////////////////////////////////////////////////////////////////////////////
// クリティカルセクション
//////////////////////////////////////////////////////////////////////////////

class	ESLCriticalSection	: public ESLObject, protected CRITICAL_SECTION
{
public:
	// 構築関数
	ESLCriticalSection( void ) ;
	// 消滅関数
	virtual ~ESLCriticalSection( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESLCriticalSection, ESLObject )

public:
	// 同期
	void Lock( void ) const ;
	void Unlock( void ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 同期オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ESLSyncObject	: public ESLObject
{
public:
	// 構築関数
	ESLSyncObject( void ) ;
	// 消滅関数
	virtual ~ESLSyncObject( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESLSyncObject, ESLObject )

protected:
	HANDLE	m_hObject ;

public:
	// ハンドル取得
	operator HANDLE ( void ) const
		{
			return	m_hObject ;
		}
	HANDLE Handle( void ) const
		{
			return	m_hObject ;
		}
	// ハンドル関連付け
	void AttachHandle( HANDLE hObject ) ;
	// オブジェクト削除
	virtual void CloseObject( void ) ;
	// 同期
	ESLError Wait( DWORD dwTimeout = INFINITE ) const ;

} ;

class	ESLEventObject	: public ESLSyncObject
{
public:
	// 構築関数
	ESLEventObject( void ) {}
	// クラス情報
	DECLARE_CLASS_INFO( ESLEventObject, ESLSyncObject )

public:
	// イベント生成
	ESLError CreateEvent
		( bool fInitState = false, LPCTSTR lpName = NULL,
					LPSECURITY_ATTRIBUTES lpSecAttr = NULL ) ;
	ESLError CreateSignal
		( bool fInitState = false, LPCTSTR lpName = NULL,
					LPSECURITY_ATTRIBUTES lpSecAttr = NULL ) ;
	// イベント設定
	ESLError SetEvent( void ) ;
	// イベントリセット
	ESLError ResetEvent( void ) ;

} ;

class	ESLMutexObject	: public ESLSyncObject
{
public:
	// 構築関数
	ESLMutexObject( void ) {}
	// クラス情報
	DECLARE_CLASS_INFO( ESLMutexObject, ESLSyncObject )

public:
	// ミューテックス生成
	ESLError CreateMutex
		( bool fInitOwner = false, LPCTSTR lpName = NULL,
					LPSECURITY_ATTRIBUTES lpSecAttr = NULL ) ;
	// 所有解放
	ESLError ReleaseMutex( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッド
//////////////////////////////////////////////////////////////////////////////

class	ESLThread	: public	ESLSyncObject
{
public:
	// 構築関数
	ESLThread( void ) ;
	// 消滅関数
	virtual ~ESLThread( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESLThread, ESLSyncObject )

public:
	// スレッド関数
	typedef DWORD (*THREAD_PROC)
		( ESLThread * pThread, void * pInstance ) ;

protected:
	ESLEventObject	m_eventReadyMsgQue ;	// スレッド開始イベント
	DWORD			m_dwThreadID ;			// スレッドID
	THREAD_PROC		m_pfnThread ;			// 呼び出しスレッド関数
	void *			m_pInstance ;

public:
	// スレッド開始
	virtual ESLError BeginThread
		( DWORD dwStackSize = 0, DWORD dwCreationFlags = 0,
			THREAD_PROC pfnThread = NULL, void * pThreadInstance = NULL ) ;
	// スレッドハンドルを閉じる
	virtual void CloseObject( void ) ;
	void CloseThread( void )
		{
			CloseObject() ;
		}
	// メッセージを処理する
	ESLError HandleMessage
		( DWORD dwTimeout, HWND hWnd = NULL,
			UINT uMsgFilterMin = 0, UINT uMsgFilterMax = 0 ) ;

private:
	// スレッド関数
	static DWORD WINAPI ESLThreadProc( LPVOID param ) ;
protected:
	// スレッド開始時
	virtual void OnBeginThread( void ) ;
	// スレッド終了時
	virtual void OnEndThread( void ) ;
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;
	// スレッドメッセージ処理
	virtual void DispatchMessage( const MSG & msg ) ;

public:
	// 論理プロセッサ数取得
	static int GetLogicalProcessorCount( void ) ;

public:
	// スレッド終了コード取得
	ESLError GetThreadExitCode
		( DWORD dwTimeout = INFINITE, DWORD * pdwExitCode = NULL ) const ;
	// スレッド ID 取得
	DWORD ID( void ) const
		{
			return	m_dwThreadID ;
		}
	// スレッドメッセージ送信
	BOOL PostThreadMessage
			( UINT nMsg, WPARAM wParam = 0, LPARAM lParam = 0 ) const
		{
			return	::PostThreadMessage( m_dwThreadID, nMsg, wParam, lParam ) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// プロセス
//////////////////////////////////////////////////////////////////////////////

class	ESLProcess	: public	ESLThread
{
public:
	// 構築関数
	ESLProcess( void ) ;
	// 消滅関数
	virtual ~ESLProcess( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESLProcess, ESLThread )

protected:
	ESLSyncObject	m_syncProcess ;
	DWORD			m_dwProcessID ;

public:
	// オブジェクト削除
	virtual void CloseObject( void ) ;
	// プロセス起動
	virtual ESLError CreateProcess
		( const char * pszAppName,
			const char * pszCmdLine, DWORD dwFlags,
			const char * pszEnvironment = NULL,
			const char * pszCurrentDirectory = NULL ) ;

public:
	// プロセス終了コード取得
	ESLError GetProcessExitCode
		( DWORD dwTimeout = INFINITE, DWORD * pdwExitCode = NULL ) ;
	// プロセスハンドル取得
	const ESLSyncObject & GetProcess() const
		{
			return	m_syncProcess ;
		}
	// プロセス ID 取得
	DWORD ProcessID( void ) const
		{
			return	m_dwProcessID ;
		}
} ;


//////////////////////////////////////////////////////////////////////////////
// パフォーマンスカウンタ
//////////////////////////////////////////////////////////////////////////////

class	ESLPerformanceCounter
{
protected:
	LARGE_INTEGER	m_li64BeginTime ;
	LARGE_INTEGER	m_li64Frequency ;

public:
	ESLPerformanceCounter( void )
		{
			::QueryPerformanceFrequency( &m_li64Frequency ) ;
			::QueryPerformanceCounter( &m_li64BeginTime ) ;
		}
	void BeginCounter( void )
		{
			::QueryPerformanceCounter( &m_li64BeginTime ) ;
		}
	double Time( void ) const
		{
			LARGE_INTEGER	li64EndTime ;
			::QueryPerformanceCounter( &li64EndTime ) ;
			return	(double) (li64EndTime.QuadPart
									- m_li64BeginTime.QuadPart)
								/ m_li64Frequency.QuadPart ;
		}
	long int MilliTime( void ) const
		{
			LARGE_INTEGER	li64EndTime ;
			::QueryPerformanceCounter( &li64EndTime ) ;
			return	(long int)
				((li64EndTime.QuadPart
					- m_li64BeginTime.QuadPart)
							* 1000 / m_li64Frequency.QuadPart) ;
		}
	
} ;


#endif
