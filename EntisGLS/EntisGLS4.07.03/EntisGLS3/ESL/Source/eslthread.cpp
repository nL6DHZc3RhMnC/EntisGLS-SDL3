
/*****************************************************************************
                    Entis Standard Library declarations
 ----------------------------------------------------------------------------
        Copyright (c) 2010 Leshade Entis. All rights reserved.
 *****************************************************************************/

#include	<windows.h>
#include	<eritypes.h>
#include	<esl.h>



//////////////////////////////////////////////////////////////////////////////
// クリティカルセクション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESLCriticalSection, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESLCriticalSection::ESLCriticalSection( void )
{
	::InitializeCriticalSection( this ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESLCriticalSection::~ESLCriticalSection( void )
{
	::DeleteCriticalSection( this ) ;
}

// 同期
//////////////////////////////////////////////////////////////////////////////
void ESLCriticalSection::Lock( void ) const
{
	::EnterCriticalSection( (CRITICAL_SECTION*) this ) ;
}

void ESLCriticalSection::Unlock( void ) const
{
	::LeaveCriticalSection( (CRITICAL_SECTION*) this ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 同期オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESLSyncObject, ESLObject )
IMPLEMENT_CLASS_INFO( ESLEventObject, ESLSyncObject )
IMPLEMENT_CLASS_INFO( ESLMutexObject, ESLSyncObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESLSyncObject::ESLSyncObject( void )
{
	m_hObject = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESLSyncObject::~ESLSyncObject( void )
{
	if ( m_hObject != NULL )
	{
		CloseObject() ;
	}
}

// ハンドル関連付け
//////////////////////////////////////////////////////////////////////////////
void ESLSyncObject::AttachHandle( HANDLE hObject )
{
	CloseObject() ;
	m_hObject = hObject ;
}

// オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void ESLSyncObject::CloseObject( void )
{
	if ( m_hObject != NULL )
	{
		::CloseHandle( m_hObject ) ;
		m_hObject = NULL ;
	}
}

// 同期
//////////////////////////////////////////////////////////////////////////////
ESLError ESLSyncObject::Wait( DWORD dwTimeout ) const
{
	if ( m_hObject == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwWaitResult = ::WaitForSingleObject( m_hObject, dwTimeout ) ;
	if ( dwWaitResult == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	else if ( dwWaitResult != WAIT_OBJECT_0 )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// イベント生成
//////////////////////////////////////////////////////////////////////////////
ESLError ESLEventObject::CreateEvent
	( bool fInitState, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpSecAttr )
{
	CloseObject() ;
	//
	m_hObject = ::CreateEvent( lpSecAttr, TRUE, fInitState, lpName ) ;
	if ( m_hObject == NULL )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

ESLError ESLEventObject::CreateSignal
	( bool fInitState, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpSecAttr )
{
	CloseObject() ;
	//
	m_hObject = ::CreateEvent( lpSecAttr, FALSE, fInitState, lpName ) ;
	if ( m_hObject == NULL )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// イベント設定
//////////////////////////////////////////////////////////////////////////////
ESLError ESLEventObject::SetEvent( void )
{
	if ( m_hObject != NULL )
	{
		if ( ::SetEvent( m_hObject ) )
		{
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}

// イベントリセット
//////////////////////////////////////////////////////////////////////////////
ESLError ESLEventObject::ResetEvent( void )
{
	if ( m_hObject != NULL )
	{
		if ( ::ResetEvent( m_hObject ) )
		{
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}

// ミューテックス生成
//////////////////////////////////////////////////////////////////////////////
ESLError ESLMutexObject::CreateMutex
	( bool fInitOwner, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpSecAttr )
{
	CloseObject() ;
	//
	m_hObject = ::CreateMutex( lpSecAttr, fInitOwner, lpName ) ;
	if ( m_hObject == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		return	eslErrPending ;
	}
	return	eslErrSuccess ;
}

// 所有解放
//////////////////////////////////////////////////////////////////////////////
ESLError ESLMutexObject::ReleaseMutex( void )
{
	if ( m_hObject != NULL )
	{
		if ( ::ReleaseMutex( m_hObject ) )
		{
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}


//////////////////////////////////////////////////////////////////////////////
// スレッド
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESLThread, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESLThread::ESLThread( void )
{
	m_dwThreadID = 0 ;
	m_pfnThread = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESLThread::~ESLThread( void )
{
	CloseThread( ) ;
}

// スレッド開始
//////////////////////////////////////////////////////////////////////////////
ESLError ESLThread::BeginThread
	( DWORD dwStackSize, DWORD dwCreationFlags,
			THREAD_PROC pfnThread, void * pThreadInstance )
{
	if ( m_hObject != NULL )
	{
		return	eslErrGeneral ;
	}
	m_pfnThread = pfnThread ;
	m_pInstance = pThreadInstance ;
	m_eventReadyMsgQue.CreateEvent( false ) ;
	m_hObject = ::CreateThread
		( NULL, dwStackSize, &ESLThread::ESLThreadProc,
					this, dwCreationFlags, &m_dwThreadID ) ;
	if ( m_hObject == NULL )
	{
		m_eventReadyMsgQue.CloseObject() ;
		return	eslErrGeneral ;
	}
	HANDLE	hEvents[2] = { m_eventReadyMsgQue, m_hObject } ;
	::WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE ) ;
	return	eslErrSuccess ;
}

// スレッドハンドルを閉じる
//////////////////////////////////////////////////////////////////////////////
void ESLThread::CloseObject( void )
{
	m_eventReadyMsgQue.CloseObject() ;
	m_pfnThread = NULL ;
	//
	ESLSyncObject::CloseObject( ) ;
}

// メッセージを処理する
//////////////////////////////////////////////////////////////////////////////
ESLError ESLThread::HandleMessage
	( DWORD dwTimeout, HWND hWnd, UINT uMsgFilterMin, UINT uMsgFilterMax )
{
	DWORD	dwBeginTime = ::timeGetTime( ) ;
	for ( ; ; )
	{
		DWORD	dwTimeoutDelta = 30 ;
		if ( dwTimeout != INFINITE )
		{
			DWORD	dwCurrentTime = ::timeGetTime() - dwBeginTime ;
			if ( dwCurrentTime >= dwTimeout )
			{
				return	eslErrTimeout ;
			}
			if ( dwTimeoutDelta + dwCurrentTime >= dwTimeout )
			{
				dwTimeoutDelta = dwTimeout - dwCurrentTime ;
			}
		}
		HANDLE	hEvents[1] = { Handle() } ;
		DWORD	dwWaitResult =
			::MsgWaitForMultipleObjects
				( 1, hEvents, FALSE, dwTimeoutDelta, QS_ALLINPUT ) ;
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{
			break ;
		}
		for ( int i = 0; i < 0x20; i ++ )
		{
			MSG		msg ;
			if ( !::PeekMessage
				( &msg, hWnd, uMsgFilterMin, uMsgFilterMax, PM_NOREMOVE ) )
			{
				break ;
			}
			if ( ::GetMessage( &msg, hWnd, uMsgFilterMin, uMsgFilterMax ) )
			{
				::TranslateMessage( &msg ) ;
				::DispatchMessage( &msg ) ;
			}
			else
			{
				return	eslErrAbort ;
			}
		}
	}
	return	eslErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ESLThread::ESLThreadProc( LPVOID param )
{
	ESLThread *	pThread = (ESLThread*) param ;
	pThread->OnBeginThread() ;
	//
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	pThread->m_eventReadyMsgQue.SetEvent() ;
	//
	DWORD	dwExit ;
	if ( pThread->m_pfnThread == NULL )
	{
		dwExit = pThread->ThreadProc( ) ;
	}
	else
	{
		dwExit = (pThread->m_pfnThread)( pThread, pThread->m_pInstance ) ;
	}
	//
	pThread->OnEndThread() ;
	//
	return	dwExit ;
}

// スレッド開始時
//////////////////////////////////////////////////////////////////////////////
void ESLThread::OnBeginThread( void )
{
}

// スレッド終了時
//////////////////////////////////////////////////////////////////////////////
void ESLThread::OnEndThread( void )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ESLThread::ThreadProc( void )
{
	MSG	msg ;
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		DispatchMessage( msg ) ;
	}
	return	0 ;
}

// スレッドメッセージ処理
//////////////////////////////////////////////////////////////////////////////
void ESLThread::DispatchMessage( const MSG & msg )
{
	::TranslateMessage( &msg ) ;
	::DispatchMessage( &msg ) ;
}

// 論理プロセッサ数取得
//////////////////////////////////////////////////////////////////////////////
int ESLThread::GetLogicalProcessorCount( void )
{
	int			nProcessorCount = 0 ;
	DWORD_PTR	dwProcessMask, dwSystemMask ;
	if ( ::GetProcessAffinityMask
			( ::GetCurrentProcess(), &dwProcessMask, &dwSystemMask ) )
	{
		DWORD_PTR	dwTest = 0x00000001 ;
		while ( dwTest != 0 )
		{
			if ( dwSystemMask & dwTest )
			{
				nProcessorCount ++ ;
			}
			dwTest <<= 1 ;
		}
	}
	return	nProcessorCount ;
}

// スレッド終了コード取得
//////////////////////////////////////////////////////////////////////////////
ESLError ESLThread::GetThreadExitCode
	( DWORD dwTimeout, DWORD * pdwExitCode ) const
{
	ESLError	err = Wait( dwTimeout ) ;
	if ( !err )
	{
		if ( !::GetExitCodeThread( Handle(), pdwExitCode ) )
		{
			err = eslErrFailed ;
		}
	}
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// プロセス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESLProcess, ESLThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESLProcess::ESLProcess( void )
{
	m_dwProcessID = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESLProcess::~ESLProcess( void )
{
}

// オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void ESLProcess::CloseObject( void )
{
	m_syncProcess.CloseObject() ;
	ESLThread::CloseObject() ;
}

// プロセス起動
//////////////////////////////////////////////////////////////////////////////
ESLError ESLProcess::CreateProcess
	( const char * pszAppName, const char * pszCmdLine, DWORD dwFlags,
		const char * pszEnvironment, const char * pszCurrentDirectory )
{
	CloseObject() ;
	//
	PROCESS_INFORMATION	pi ;
	STARTUPINFO			si ;
	memset( &pi, 0, sizeof(pi) ) ;
	memset( &si, 0, sizeof(si) ) ;
	si.cb = sizeof(si) ;
	//
	if ( pszAppName != NULL )
	{
		if ( pszAppName[0] == '\0' )
		{
			pszAppName = NULL ;
		}
	}
	if ( pszCmdLine != NULL )
	{
		if ( pszCmdLine[0] == '\0' )
		{
			pszCmdLine = NULL ;
		}
	}
	if ( pszEnvironment != NULL )
	{
		if ( pszEnvironment[0] == '\0' )
		{
			pszEnvironment = NULL ;
		}
	}
	if ( pszCurrentDirectory != NULL )
	{
		if ( pszCurrentDirectory[0] == '\0' )
		{
			pszCurrentDirectory = NULL ;
		}
	}
	EString	strCmdLine = pszCmdLine ;
	EString	strEnvironment = pszEnvironment ;
	//
	if ( ::CreateProcess
		( pszAppName, strCmdLine.GetBuffer(0x100),
			NULL, NULL, FALSE, dwFlags,
			((pszEnvironment == NULL)
				? NULL : strEnvironment.GetBuffer(0x100)),
			pszCurrentDirectory, &si, &pi ) )
	{
		AttachHandle( pi.hThread ) ;
		m_dwThreadID = pi.dwThreadId ;
		m_syncProcess.AttachHandle( pi.hProcess ) ;
		m_dwProcessID = pi.dwProcessId ;
	}
	else
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// プロセス終了コード取得
//////////////////////////////////////////////////////////////////////////////
ESLError ESLProcess::GetProcessExitCode
		( DWORD dwTimeout, DWORD * pdwExitCode )
{
	ESLError	err = m_syncProcess.Wait( dwTimeout ) ;
	if ( !err )
	{
		if ( !::GetExitCodeProcess( m_syncProcess, pdwExitCode ) )
		{
			err = eslErrFailed ;
		}
	}
	return	err ;
}

