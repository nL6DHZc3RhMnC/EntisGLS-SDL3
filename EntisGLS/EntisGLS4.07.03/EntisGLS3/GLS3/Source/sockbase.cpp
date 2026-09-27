
#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 低水準ソケットクラス
//////////////////////////////////////////////////////////////////////////////

HANDLE					ESocket::m_hThread = NULL ;		// 送受信スレッド
DWORD					ESocket::m_dwThreadID = 0 ;
HWND					ESocket::m_hWnd = NULL ;		// 通知ウィンドウ
EIntTagArray<ESocket> *	ESocket::m_pitaSocket = NULL ;	// ソケットリスト


// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESocket, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESocket::ESocket( void )
{
	m_hSocket = INVALID_SOCKET ;
	m_hCancelBlocking = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
	m_hConnected = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hSendEmpty = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
	m_hRecvData = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	::InitializeCriticalSection( &m_cs ) ;
	m_dwRecvBufLimit = 0x100000 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESocket::~ESocket( void )
{
	Close( ) ;
	//
	::CloseHandle( m_hCancelBlocking ) ;
	::CloseHandle( m_hConnected ) ;
	::CloseHandle( m_hSendEmpty ) ;
	::CloseHandle( m_hRecvData ) ;
	::DeleteCriticalSection( &m_cs ) ;
}

// ソケット作成
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Create
	( UINT nPort, int nType, long int nEvent, const char * pszAddress )
{
	ESLAssert( m_hSocket == INVALID_SOCKET ) ;
	//
	// ソケット作成
	//
	m_hSocket = socket( PF_INET, nType, 0 ) ;
	if ( m_hSocket == INVALID_SOCKET )
		return	eslErrGeneral ;
	//
	ESLError	err ;
	err = Attach( m_hSocket, nEvent ) ;
	if ( err )
		return	err ;
	//
	// バインド
	//
	return	Bind( nPort, pszAddress ) ;
}

// ソケットを関連付ける
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Attach( SOCKET hSocket, long int nEvent )
{
	ESLAssert( m_hWnd != NULL ) ;
	if ( m_hWnd == NULL )
	{
		ESLTrace( "ESocket::InitSocket() が呼び出されていません。\n" ) ;
		InitSocket( ) ;
	}
	//
	int		nResult ;
	::ResetEvent( m_hCancelBlocking ) ;
	::ResetEvent( m_hConnected ) ;
	::SendMessage( m_hWnd, wmAdd, (WPARAM) hSocket, (LPARAM) this ) ;
	//
	nResult = ::WSAAsyncSelect
		( hSocket, m_hWnd, wmSockNotify, nEvent ) ;
	if ( nResult == SOCKET_ERROR )
	{
		return	eslErrGeneral ;
	}
	m_hSocket = hSocket ;
	//
	return	eslErrSuccess ;
}

// ソケットをローカルアドレスに結びつける
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Bind( UINT nPort, const char * pszAddress )
{
	SOCKADDR_IN	sockadin ;
	memset( &sockadin, 0, sizeof(sockadin) ) ;
	sockadin.sin_family = AF_INET ;
	if ( pszAddress == NULL )
	{
		sockadin.sin_addr.s_addr = htonl(INADDR_ANY) ;
	}
	else
	{
		DWORD	dwResult = inet_addr( pszAddress ) ;
		if ( dwResult == INADDR_NONE )
		{
			return	eslErrGeneral ;
		}
		sockadin.sin_addr.s_addr = dwResult ;
	}
	sockadin.sin_port = htons( (u_short) nPort ) ;
	if ( ::bind( m_hSocket,
			(SOCKADDR*) &sockadin, sizeof(sockadin) ) != SOCKET_ERROR )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ソケットを閉じる
//////////////////////////////////////////////////////////////////////////////
void ESocket::Close( void )
{
	if ( m_hSocket != INVALID_SOCKET )
	{
		ESLAssert( m_hWnd != NULL ) ;
		::SendMessage( m_hWnd, wmClose, NULL, (LPARAM) this ) ;
		::SetEvent( m_hCancelBlocking ) ;
		m_hSocket = INVALID_SOCKET ;
	}
}

// ソケット接続
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Connect( const char * pszHostAddress, UINT nHostPort )
{
	if ( m_hSocket != INVALID_SOCKET )
	{
		SOCKADDR_IN	sockadin ;
		memset( &sockadin, 0, sizeof(sockadin) ) ;
		sockadin.sin_family = AF_INET ;
		sockadin.sin_addr.s_addr = inet_addr( pszHostAddress ) ;
		if ( sockadin.sin_addr.s_addr == INADDR_NONE )
		{
			LPHOSTENT	lphost ;
			lphost = gethostbyname( pszHostAddress ) ;
			if ( lphost != NULL )
			{
				sockadin.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr ;
			}
			else
			{
				return	eslErrGeneral ;
			}
		}
		sockadin.sin_port = htons( (u_short) nHostPort ) ;
		//
		if ( ::connect( m_hSocket,
				(SOCKADDR*) &sockadin, sizeof(sockadin) ) == SOCKET_ERROR )
		{
			return	eslErrTimeout ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 接続要求を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Listen( int nConnectionBacklog )
{
	ESLAssert( m_hSocket != INVALID_SOCKET ) ;
	if ( listen( m_hSocket, nConnectionBacklog ) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 接続受け入れ
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::Accept
	( ESocket & socket, SOCKADDR * pSockAddr, int * pSockAddrLen )
{
	SOCKET	hSocket = accept( m_hSocket, pSockAddr, pSockAddrLen ) ;
	if ( hSocket != INVALID_SOCKET )
	{
		return	socket.Attach( hSocket ) ;
	}
	return	eslErrGeneral ;
}

// 受信
//////////////////////////////////////////////////////////////////////////////
int ESocket::Receive( void * ptrBuf, int nBufLen )
{
	int	nRecvLen ;
	Lock( ) ;
	nRecvLen = m_bufRecv.Read( ptrBuf, nBufLen ) ;
	if ( m_bufRecv.GetLength() == 0 )
	{
		::ResetEvent( m_hRecvData ) ;
	}
	Unlock( ) ;
	return	nRecvLen ;
}

// 送信
//////////////////////////////////////////////////////////////////////////////
int ESocket::Send( const void * ptrBuf, int nBufLen )
{
	int	nSendLen ;
	Lock( ) ;
	nSendLen = m_bufSend.Write( ptrBuf, nBufLen ) ;
	if ( m_bufSend.GetLength() > 0 )
	{
		::ResetEvent( m_hSendEmpty ) ;
	}
	Unlock( ) ;
	::SendMessage( m_hWnd, wmSend, 0, (LPARAM) this ) ;
	return	nSendLen ;
}

// 1行受信
//////////////////////////////////////////////////////////////////////////////
int ESocket::ReceiveLine( EString & strLine )
{
	int		nResult ;
	Lock( ) ;
	EPtrBuffer	ptrbuf = m_bufRecv.GetBuffer( ) ;
	const char *	pstrBuf = (const char *) ptrbuf.GetBuffer( ) ;
	unsigned int	nBufLen = ptrbuf.GetLength( ) ;
	unsigned int	i ;
	for ( i = 0; i < nBufLen; i ++ )
	{
		if ( pstrBuf[i] == '\n' )
			break ;
	}
	if ( i < nBufLen )
	{
		strLine = EString( pstrBuf, i + 1 ) ;
		m_bufRecv.Release( i + 1 ) ;
		nResult = i + 1 ;
		if ( m_bufRecv.GetLength() == 0 )
		{
			::ResetEvent( m_hRecvData ) ;
		}
	}
	else
	{
//		strLine = "" ;
//		m_bufRecv.Release( 0 ) ;
		nResult = -1 ;
	}
	Unlock( ) ;
	return	nResult ;
}

// ブロッキングをキャンセル
//////////////////////////////////////////////////////////////////////////////
void ESocket::CancelBlocking( void )
{
	SetEvent( m_hCancelBlocking ) ;
}

// ブロッキングがキャンセルされているか？
//////////////////////////////////////////////////////////////////////////////
bool ESocket::IsBlockingCanceled( void ) const
{
	return	(::WaitForSingleObject( m_hCancelBlocking, 0 ) == WAIT_OBJECT_0) ;
}

// 接続完了を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::WaitUntilConnected( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hConnected, dwTimeout, fDispMsg ) ;
}

// 送信完了を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::WaitUntilSent( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hSendEmpty, dwTimeout, fDispMsg ) ;
}

// 何らかのデータを受信するまで待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::WaitUntilReceived( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hRecvData, dwTimeout, fDispMsg ) ;
}

// イベントを待機
//////////////////////////////////////////////////////////////////////////////
ESLError ESocket::WaitEvent( HANDLE hEvent, DWORD dwTimeout, bool fDispMsg )
{
	HANDLE	hWaitEvent[2] ;
	hWaitEvent[0] = hEvent ;
	hWaitEvent[1] = m_hCancelBlocking ;
	//
	if ( fDispMsg )
	{
		DWORD	dwStartTime = ::GetCurrentTime( ) ;
		for ( ; ; )
		{
			DWORD	dwResult =
				::WaitForMultipleObjects( 2, hWaitEvent, FALSE, 10 ) ;
			if ( dwResult == WAIT_OBJECT_0 )	
			{
				return	eslErrSuccess ;
			}
			else if ( dwResult == WAIT_OBJECT_0 + 1 )
			{
				return	eslErrAbort ;
			}
			else if ( dwResult != WAIT_TIMEOUT )
			{
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < 0x10; i ++ )
			{
				if ( ::GetCurrentTime() - dwStartTime >= dwTimeout )
				{
					return	eslErrTimeout ;
				}
				MSG		msg ;
				if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					::TranslateMessage( &msg ) ;
					::DispatchMessage( &msg ) ;
				}
				else
				{
					break ;
				}
			}
		}
	}
	else
	{
		DWORD	dwResult =
			::WaitForMultipleObjects( 2, hWaitEvent, FALSE, dwTimeout ) ;
		if ( dwResult == WAIT_OBJECT_0 )	
		{
			return	eslErrSuccess ;
		}
		else if ( dwResult == WAIT_OBJECT_0 + 1 )
		{
			return	eslErrAbort ;
		}
		else if ( dwResult != WAIT_TIMEOUT )
		{
			return	eslErrGeneral ;
		}
		return	eslErrTimeout ;
	}
}

// 指定バイト数受信
//////////////////////////////////////////////////////////////////////////////
int ESocket::ReceiveTimeout
	( void * ptrBuf, int nBufLen, DWORD dwTimeout, bool fDispMsg )
{
	int		nRecvBytes = 0 ;
	DWORD	dwStartTime = ::GetCurrentTime( ) ;
	for ( ; ; )
	{
		DWORD	dwLeftTime ;
		DWORD	dwCurrentTime = ::GetCurrentTime() - dwStartTime ;
		if ( dwCurrentTime > dwTimeout )
			dwLeftTime = 0 ;
		else
			dwLeftTime = dwTimeout - dwCurrentTime ;
		//
		ESLError	err = WaitUntilReceived( dwLeftTime, fDispMsg ) ;
		nRecvBytes += Receive
			( ((PBYTE)ptrBuf) + nRecvBytes, nBufLen - nRecvBytes ) ;
		if ( (nRecvBytes >= nBufLen) || err )
		{
			break ;
		}
	}
	return	nRecvBytes ;
}

// １行受信
//////////////////////////////////////////////////////////////////////////////
int ESocket::ReceiveLineTimeout
	( EString & strLine, DWORD dwTimeout, bool fDispMsg )
{
	DWORD	dwStartTime = ::GetCurrentTime( ) ;
	for ( ; ; )
	{
		int	nResult = ReceiveLine( strLine ) ;
		if ( nResult >= 0 )
		{
			return	nResult ;
		}
		if ( fDispMsg )
		{
			for ( int i = 0; i < 0x10; i ++ )
			{
				if ( ::GetCurrentTime() - dwStartTime >= dwTimeout )
				{
					return	-1 ;
				}
				MSG		msg ;
				if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					::TranslateMessage( &msg ) ;
					::DispatchMessage( &msg ) ;
				}
				else
				{
					break ;
				}
			}
		}
		if ( ::WaitForSingleObject
				( m_hCancelBlocking, 10 ) == WAIT_OBJECT_0 )
		{
			return	ReceiveLine( strLine ) ;
		}
	}
	return	-1 ;
}

// 受信バッファ最大サイズを設定する
//////////////////////////////////////////////////////////////////////////////
void ESocket::SetReceiveBufferLimit( DWORD dwBufLimit )
{
	m_dwRecvBufLimit = dwBufLimit ;
}

// 排他アクセス
//////////////////////////////////////////////////////////////////////////////
void ESocket::Lock( void )
{
	::EnterCriticalSection( &m_cs ) ;
}

void ESocket::Unlock( void )
{
	::LeaveCriticalSection( &m_cs ) ;
}

// 受信データがある
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnReceive( int nErrorCode )
{
	Lock( ) ;
	DWORD	dwRecvBytes = 0x400 ;
	if ( m_bufRecv.GetLength() + dwRecvBytes > m_dwRecvBufLimit )
	{
		if ( m_bufRecv.GetLength() < m_dwRecvBufLimit )
		{
			dwRecvBytes = m_dwRecvBufLimit - m_bufRecv.GetLength() ;
		}
		else
		{
			dwRecvBytes = 0 ;
		}
	}
	char *	ptrBuf = (char*) m_bufRecv.PutBuffer( dwRecvBytes ) ;
	int		nLength = ::recv( m_hSocket, ptrBuf, dwRecvBytes, 0 ) ;
	if ( nLength == SOCKET_ERROR )
	{
		nLength = 0 ;
	}
	m_bufRecv.Flush( nLength ) ;
	//
	if ( m_bufRecv.GetLength() > 0 )
	{
		::SetEvent( m_hRecvData ) ;
	}
	Unlock( ) ;
	//
	if ( nLength == 0 )
	{
		::Sleep( 1 ) ;
	}
}

// 送信可能状態になった
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnSend( int nErrorCode )
{
	Lock( ) ;
	while ( m_bufSend.GetLength() > 0 )
	{
		EPtrBuffer	ptrbuf = m_bufSend.GetBuffer( ) ;
		const char *	ptrBuf = (const char *) ptrbuf.GetBuffer( ) ;
		unsigned int	nLength = ptrbuf.GetLength( ) ;
		nLength = ::send( m_hSocket, ptrBuf, nLength, 0 ) ;
		if ( nLength == SOCKET_ERROR )
		{
			if ( WSAGetLastError() == WSAEWOULDBLOCK )
			{
				m_bufSend.Release( 0 ) ;
				break ;
			}
			::Sleep( 1 ) ;
			nLength = 0 ;
		}
		m_bufSend.Release( nLength ) ;
	}
	if ( m_bufSend.GetLength() == 0 )
	{
		::SetEvent( m_hSendEmpty ) ;
	}
	Unlock( ) ;
}

// 帯域外データがある
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnOutOfBandData( int nErrorCode )
{
}

// 接続要求受け入れ可能
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnAccept( int nErrorCode )
{
}

// ソケットが接続された
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnConnect( int nErrorCode )
{
	::SetEvent( m_hConnected ) ;
}

// ソケットが閉じられた
//////////////////////////////////////////////////////////////////////////////
void ESocket::OnClose( int nErrorCode )
{
	Lock( ) ;
	DWORD	dwRecvBytes = 0x400 ;
	for ( ; ; )
	{
		char *	ptrBuf = (char*) m_bufRecv.PutBuffer( dwRecvBytes ) ;
		int		nLength = ::recv( m_hSocket, ptrBuf, dwRecvBytes, 0 ) ;
		if ( nLength == SOCKET_ERROR )
		{
			nLength = 0 ;
		}
		m_bufRecv.Flush( nLength ) ;
		if ( nLength == 0 )
		{
			break ;
		}
	}
	if ( m_bufRecv.GetLength() > 0 )
	{
		::SetEvent( m_hRecvData ) ;
	}
	Unlock( ) ;
	//
	::SetEvent( m_hCancelBlocking ) ;
}

// ESocket クラス初期化
//////////////////////////////////////////////////////////////////////////////
void ESocket::InitSocket( void )
{
	if ( m_hWnd == NULL )
	{
		m_pitaSocket = new EIntTagArray<ESocket> ;
		//
		WORD	wReqVer = MAKEWORD( 1, 1 );
		WSADATA	wsaData ;
		::WSAStartup( wReqVer, &wsaData ) ;
		//
		HANDLE	hEventBegin = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		m_hThread = ::CreateThread
			( NULL, 0, &ESocket::ThreadProc,
				(LPVOID) hEventBegin, 0, &m_dwThreadID ) ;
		::WaitForSingleObject( hEventBegin, INFINITE ) ;
		::CloseHandle( hEventBegin ) ;
	}
}

// ESocket クラス終了
//////////////////////////////////////////////////////////////////////////////
void ESocket::ExitSocket( void )
{
	if ( m_hWnd != NULL )
	{
		::PostMessage( m_hWnd, wmExit, 0, 0 ) ;
		::WaitForSingleObject( m_hThread, INFINITE ) ;
		::CloseHandle( m_hThread ) ;
		m_hWnd = NULL ;
		m_hThread = NULL ;
		delete	m_pitaSocket ;
	}
}

// スレッドプロシージャ
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ESocket::ThreadProc( LPVOID param )
{
	//
	// ウィンドウクラス登録
	//
	WNDCLASS	wc ;
	wc.style = 0 ;
	wc.lpfnWndProc = &ESocket::WindowProc ;
	wc.cbClsExtra = 0 ;
	wc.cbWndExtra = 0 ;
	wc.hInstance = (HINSTANCE) ::GetModuleHandle( NULL ) ;
	wc.hIcon = NULL ;
	wc.hCursor = NULL ;
	wc.hbrBackground = NULL ;
	wc.lpszMenuName = NULL ;
	wc.lpszClassName = "ESocket_WindowClass" ;
	//
	if ( !::RegisterClass( &wc ) )
	{
		ESLTrace( "ウィンドウクラスの登録に失敗しました。\n" ) ;
		::SetEvent( (HANDLE) param ) ;
		return	0 ;
	}
	//
	// ウィンドウ作成
	//
	m_hWnd = ::CreateWindowA
		( wc.lpszClassName, "ESocket_Window", WS_POPUP,
			0, 0, 100, 100, NULL/*HWND_MESSAGE*/, NULL, wc.hInstance, NULL ) ;
	::SetEvent( (HANDLE) param ) ;
	if ( m_hWnd == NULL )
	{
		ESLTrace( "ウィンドウの作成に失敗しました。\n" ) ;
		return	0 ;
	}
	//
	// メッセージループ
	//
	MSG		msg ;
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		::TranslateMessage( &msg ) ;
		::DispatchMessage( &msg ) ;
	}
	//
	// 終了処理
	//
	ESLAssert( m_pitaSocket != NULL ) ;
	while ( m_pitaSocket->GetSize() != 0 )
	{
		ETaggedElement<int,ESocket> *	pElement = m_pitaSocket->GetAt( 0 ) ;
		if ( pElement == NULL )
			break ;
		ESocket *	pSocket = pElement->GetObject( ) ;
		if ( pSocket == NULL )
			break ;
		//
		pElement->DetachObject()->Close( ) ;
		m_pitaSocket->RemoveAt( 0 ) ;
	}
	//
	return	0 ;
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
	ESocket::WindowProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
	if ( nMsg == wmSockNotify )
	{
		//
		// 非ブロッキング通知
		//
		unsigned int	nIndex ;
		ESLAssert( m_pitaSocket != NULL ) ;
		ESocket *	pSocket = m_pitaSocket->GetAs( (int) wParam, &nIndex ) ;
		if ( pSocket != NULL )
		{
			int		nEventCode = WSAGETSELECTEVENT(lParam) ;
			int		nErrorCode = WSAGETSELECTERROR(lParam) ;
			switch ( nEventCode )
			{
			case	FD_READ:
				pSocket->OnReceive( nErrorCode ) ;
				break ;
			case	FD_WRITE:
				pSocket->OnSend( nErrorCode ) ;
				break ;
			case	FD_ACCEPT:
				pSocket->OnAccept( nErrorCode ) ;
				break ;
			case	FD_CONNECT:
				pSocket->OnConnect( nErrorCode ) ;
				break ;
			case	FD_OOB:
				pSocket->OnOutOfBandData( nErrorCode ) ;
				break ;
			case	FD_CLOSE:
				pSocket->OnClose( nErrorCode ) ;
				break ;
			}
			return	0 ;
		}
	}
	else if ( nMsg == wmSend )
	{
		//
		// データ送信
		//
		ESocket *	pSocket = (ESocket*) lParam ;
		pSocket->Lock( ) ;
		while ( pSocket->m_bufSend.GetLength() > 0 )
		{
			EPtrBuffer	ptrbuf = pSocket->m_bufSend.GetBuffer( ) ;
			const char *	ptrBuf = (const char *) ptrbuf.GetBuffer( ) ;
			unsigned int	nLength = ptrbuf.GetLength( ) ;
			nLength = ::send( pSocket->m_hSocket, ptrBuf, nLength, 0 ) ;
			if ( nLength == SOCKET_ERROR )
			{
//				if ( GetLastError() == WSAEWOULDBLOCK )
				{
					pSocket->m_bufSend.Release( 0 ) ;
					break ;
				}
				nLength = 0 ;
			}
			pSocket->m_bufSend.Release( nLength ) ;
		}
		if ( pSocket->m_bufSend.GetLength() == 0 )
		{
			::SetEvent( pSocket->m_hSendEmpty ) ;
		}
		pSocket->Unlock( ) ;
		return	0 ;
	}
	else if ( nMsg == wmClose )
	{
		//
		// ソケットを閉じる
		//
		ESocket *	pSocket = (ESocket*) lParam ;
		ESLVerify( ::closesocket( pSocket->m_hSocket ) != SOCKET_ERROR ) ;
		ESLAssert( m_pitaSocket != NULL ) ;
		m_pitaSocket->DetachAs( (int) pSocket->m_hSocket ) ;
		return	0 ;
	}
	else if ( nMsg == wmAdd )
	{
		//
		// ソケットを追加登録
		//
		ESocket *	pSocket = (ESocket*) lParam ;
		ESLAssert( m_pitaSocket != NULL ) ;
		m_pitaSocket->Add( (int) wParam, pSocket ) ;
		return	0 ;
	}
	else if ( nMsg == wmExit )
	{
		//
		// 終了
		//
		DestroyWindow( m_hWnd ) ;
		PostQuitMessage( 0 ) ;
		ESLAssert( m_pitaSocket != NULL ) ;
		m_pitaSocket->DetachAll( ) ;
		return	0 ;
	}
	//
	return	::DefWindowProc( hWnd, nMsg, wParam, lParam ) ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP 転送プロコトル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EHttpConnection, ESocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EHttpConnection::EHttpConnection( void )
{
	m_hHeaderRecv = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hHtmlRecv = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hFinishRecv = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_dwStatusCode = (DWORD) -1 ;
	m_dwCurrentBytes = 0 ;
	m_dwTotalBytes = 0 ;
	m_hContentRecv = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_pszEndOfHTML = "</ html >" ;
	m_iSeekEndOfHTML = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EHttpConnection::~EHttpConnection( void )
{
	Close( ) ;
	::CloseHandle( m_hHeaderRecv ) ;
	::CloseHandle( m_hHtmlRecv ) ;
	::CloseHandle( m_hFinishRecv ) ;
	::CloseHandle( m_hContentRecv ) ;
}

// URL を開いてデータをダウンロードする
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::OpenURL
	( const char * pszURL, DWORD dwTimeout, bool fDispMsg,
		const char * pszAgent, const char * pszData,
		unsigned int nLength, const char * pszContentType )
{
	//
	// パラメータ設定
	//
	if ( pszData != NULL )
	{
		SetRequest( pszURL, "POST" ) ;
		SetSendData( pszData, nLength ) ;
		if ( pszContentType != NULL )
		{
			AddHeader( EString("Content-Type: ") + pszContentType ) ;
		}
		else
		{
			AddHeader( "Content-Type: application/x-www-form-urlencoded" ) ;
		}
	}
	else
	{
		SetRequest( pszURL, "GET" ) ;
	}
	if ( pszAgent != NULL )
	{
		SetHeaderUserAgent( pszAgent ) ;
	}
	//
	// ホストに接続
	//
	if ( ConnectHost( ) )
	{
		if ( WaitUntilConnected( dwTimeout, fDispMsg ) != eslErrSuccess )
		{
			return	ESLErrorMsg( "ホストに接続できませんでした。" ) ;
		}
	}
	//
	// リクエスト送信
	//
	ESLError	err = SendRequest( ) ;
	if ( err )
	{
		return	err ;
	}
	if ( WaitRecvHeader( dwTimeout, fDispMsg ) != eslErrSuccess )
	{
		return	ESLErrorMsg( "サーバから応答がありません。" ) ;
	}
	//
	// エラー判定
	//
	DWORD	dwStatusCode = GetStatusCode( ) ;
	if ( (dwStatusCode < 200) || (dwStatusCode > 299) )
	{
		return	ESLErrorMsg( "サーバからエラーが返されました。" ) ;
	}
	return	eslErrSuccess ;
}

// URL 設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetRequest
	( const char * pszURL, const char * pszCmd )
{
	EString	strURL = pszURL ;
	if ( !EString(strURL.Left(5)).CompareNoCase( "http:" ) )
	{
		strURL = strURL.Middle( 5 ) ;
	}
	if ( !strURL.Left(2).Compare( "//" ) )
	{
		strURL = strURL.Middle( 2 ) ;
	}
	int	iHost = strURL.Find( '/' ) ;
	if ( iHost >= 0 )
	{
		m_strHost = strURL.Left( iHost ) ;
		m_strPath = strURL.Middle( iHost ) ;
	}
	else
	{
		m_strHost = strURL ;
	}
	if ( m_strPath.IsEmpty() )
	{
		m_strPath += "/" ;
	}
	m_strPath = FormatURL( m_strPath ) ;
	//
	m_strCmd = pszCmd ;
	m_strCmd.TrimRight( ) ;
	m_strCmd.TrimLeft( ) ;
	//
	AddHeader( "Host: " + m_strHost ) ;
}

// 送信データ設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetSendData( const char * pszData, unsigned int nLength )
{
	if ( !m_strCmd.CompareNoCase( "POST" ) )
	{
		m_strSendData = EString( pszData, nLength ) ;
		//
		EString	strContentLength ;
		wsprintf( strContentLength.GetBuffer( 0x100 ),
				"Content-Length: %d\r\n", m_strSendData.GetLength() ) ;
		strContentLength.ReleaseBuffer( ) ;
		AddHeader( strContentLength ) ;
	}
}

// URL フォームパラメータを送信データに設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetSendURLFormData
	( const char * pszData, const char * pszCharset )
{
	if ( !m_strCmd.CompareNoCase( "POST" ) )
	{
		m_strSendData = pszData ;
		//
		EString	strContentLength ;
		::wsprintf( strContentLength.GetBuffer( 0x100 ),
				"Content-Length: %d\r\n", m_strSendData.GetLength() ) ;
		strContentLength.ReleaseBuffer( ) ;
		AddHeader( strContentLength ) ;
		//
		EString	strContentType ;
		strContentType = "Content-Type: application/x-www-form-urlencoded" ;
		if ( pszCharset != NULL )
		{
			strContentType += "; charset=" ;
			strContentType += pszCharset ;
		}
		AddHeader( strContentType ) ;
	}
}

// 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::AddHeader( const char * pszHeader )
{
	EString	strHeader( pszHeader ) ;
	int	iField = strHeader.Find( ':' ) ;
	ESLAssert( iField >= 0 ) ;
	if ( iField >= 0 )
	{
		unsigned int	i, nCount ;
		EString	strField = strHeader.Left( ++ iField ) ;
		nCount = m_listSendHeader.GetSize( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			EString *	pstrSendHdr = m_listSendHeader.GetAt( i ) ;
			ESLAssert( pstrSendHdr != NULL ) ;
			if ( !EString(pstrSendHdr->Left(iField)).CompareNoCase(strField) )
			{
				m_listSendHeader.RemoveAt( i ) ;
				break ;
			}
		}
	}
	m_listSendHeader.Add( new EString(strHeader) ) ;
}

// Accept 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetHeaderAccept( const char * pszAccept )
{
	if ( pszAccept == NULL )
		pszAccept = "*/*" ;
	//
	AddHeader( EString("Accept: ") + pszAccept ) ;
}

// Accept-Encoding 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetHeaderAcceptEncoding( const char * pszAcceptEncoding )
{
	AddHeader( EString("Accept-Encoding: ") + pszAcceptEncoding ) ;
}

// User-Agent 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::SetHeaderUserAgent( const char * pszUserAgent )
{
	AddHeader( EString("User-Agent: ") + pszUserAgent ) ;
}

// ホストに接続
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::ConnectHost( void )
{
	ESLError	err ;
	err = Create( ) ;
	if ( err )
		return	err ;
	//
	return	Connect( m_strHost, 80 ) ;
}

// リクエスト送信
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::SendRequest( void )
{
	//
	// コマンド送信
	//
	EString	strCmd = m_strCmd + " " + m_strPath + " HTTP/1.1\r\n" ;
	Send( strCmd.CharPtr(), strCmd.GetLength() ) ;
	//
	// ヘッダ送信
	//
	unsigned int	i, nCount ;
	nCount = m_listSendHeader.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EString *	pstrHeader = m_listSendHeader.GetAt( i ) ;
		if ( (pstrHeader != NULL) && !pstrHeader->IsEmpty() )
		{
			if ( pstrHeader->Right(1) != "\n" )
				*pstrHeader += "\r\n" ;
			//
			Send( pstrHeader->CharPtr(), pstrHeader->GetLength() ) ;
		}
	}
	Send( "\r\n", 2 ) ;
	//
	// 本文送信（POST コマンドのみ）
	//
	if ( !m_strCmd.CompareNoCase("POST") && !m_strSendData.IsEmpty() )
	{
		Send( m_strSendData.CharPtr(), m_strSendData.GetLength() ) ;
	}
	//
	return	eslErrSuccess ;
}

// ヘッダ場受信し終えているか？
//////////////////////////////////////////////////////////////////////////////
bool EHttpConnection::HasHeaderReceived( void ) const
{
	return	(::WaitForSingleObject( m_hHeaderRecv, 0 ) == WAIT_OBJECT_0) ;
}

// ヘッダを受信するまで待機
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::WaitRecvHeader( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hHeaderRecv, dwTimeout, fDispMsg ) ;
}

// HTML データを受信し終えるまで待機
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::WaitRecvHTMLData( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hHtmlRecv, dwTimeout, fDispMsg ) ;
}

// 内容を受信し終えているか？
//////////////////////////////////////////////////////////////////////////////
bool EHttpConnection::HasContentsReceived( void ) const
{
	return	(::WaitForSingleObject( m_hFinishRecv, 0 ) == WAIT_OBJECT_0) ;
}

// 全て受信するまで待機
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::WaitRecvContents( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hFinishRecv, dwTimeout, fDispMsg ) ;
}

// ステータスコードを取得
//////////////////////////////////////////////////////////////////////////////
DWORD EHttpConnection::GetStatusCode( void ) const
{
	return	m_dwStatusCode ;
}

// 現在の受信状況取得
//////////////////////////////////////////////////////////////////////////////
DWORD EHttpConnection::GetCurrentStatus( DWORD * pdwTotal ) const
{
	if ( pdwTotal != NULL )
		*pdwTotal = m_dwTotalBytes ;
	//
	return	m_dwCurrentBytes ;
}

// 受信データがある
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::OnReceive( int nErrorCode )
{
	ESocket::OnReceive( nErrorCode ) ;
	//
	for ( ; ; )
	{
		if ( m_dwStatusCode == (DWORD) -1 )
		{
			//
			// ステータスコード受信
			//
			EString	strLine ;
			Lock( ) ;
			if ( ESocket::ReceiveLine( strLine ) >= 0 )
			{
				if ( !EString(strLine.Left(8)).CompareNoCase( "HTTP/1.1" ) )
				{
					EStreamWideString	swsLine = strLine.Middle( 8 ) ;
					m_dwStatusCode = swsLine.GetInteger( 10 ) ;
				}
				else
				{
					m_dwStatusCode = 0 ;
				}
				Unlock( ) ;
			}
			else
			{
				Unlock( ) ;
				break ;
			}
		}
		else if ( !HasHeaderReceived() )
		{
			//
			// ヘッダ受信
			//
			EString	strLine ;
			Lock( ) ;
			if ( ESocket::ReceiveLine( strLine ) >= 0 )
			{
				strLine.TrimRight( ) ;
				if ( strLine.IsEmpty() )
				{
					//
					// 全てのヘッダを受信完了
					//
					EString *	pstrContentLength =
						m_staRecvHeader.GetAs( "Content-Length" ) ;
					if ( pstrContentLength != NULL )
					{
						EStreamWideString	swsLength = *pstrContentLength ;
						m_dwTotalBytes = swsLength.GetInteger( 10 ) ;
					}
					else
					{
						m_dwTotalBytes = 0 ;
					}
					//
					if ( (m_dwStatusCode == 100) &&
							!m_strCmd.CompareNoCase( "POST" ) )
					{
						m_dwStatusCode = (DWORD) -1 ;
					}
					else
					{
						m_dwCurrentBytes = 0 ;
						::SetEvent( m_hHeaderRecv ) ;
					}
					OnBeginHTMLData( ) ;
				}
				else
				{
					//
					// ヘッダ追加
					//
					if ( strLine[0] <= 0x20 )
					{
						EString *	pstrHeader
							= m_staRecvHeader.GetAs( m_strLastHeader ) ;
						if ( pstrHeader != NULL )
						{
							*pstrHeader += strLine ;
						}
					}
					else
					{
						int		iField = strLine.Find( ':' ) ;
						if ( iField >= 0 )
						{
							m_strLastHeader = strLine.Left( iField ) ;
							EString	strBody = strLine.Middle( iField + 1 ) ;
							m_staRecvHeader.
								Add( m_strLastHeader, new EString(strBody) ) ;
						}
					}
				}
				Unlock( ) ;
			}
			else
			{
				Unlock( ) ;
				break ;
			}
		}
		else
		{
			//
			// 本文受信
			//
			EPtrBuffer	ptrbuf = m_bufRecv.GetBuffer( ) ;
			const BYTE *	ptrBuf = (const BYTE *) ptrbuf.GetBuffer( ) ;
			DWORD			dwBufLen = ptrbuf.GetLength( ) ;
			bool	fEndOfData = IsEndOfHTMLData( ptrBuf, dwBufLen ) ;
			Lock( ) ;
			m_bufRecvHttp.Write( ptrBuf, dwBufLen ) ;
			m_bufRecv.Release( ptrbuf.GetLength() ) ;
			m_dwCurrentBytes += ptrbuf.GetLength() ;
			if ( (m_dwTotalBytes != 0)
					&& (m_dwCurrentBytes >= m_dwTotalBytes) )
			{
				::SetEvent( m_hFinishRecv ) ;
			}
			if ( fEndOfData )
			{
				::SetEvent( m_hHtmlRecv ) ;
			}
			if ( m_bufRecvHttp.GetLength() > 0 )
			{
				::SetEvent( m_hContentRecv ) ;
			}
			Unlock( ) ;
			break ;
		}
	}
}

// ソケットが閉じられた
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::OnClose( int nErrorCode )
{
	ESocket::OnClose( nErrorCode ) ;
	EHttpConnection::OnReceive( nErrorCode ) ;
	//
	::SetEvent( m_hFinishRecv ) ;
}

// HTML データの終端記号チェックの準備処理
//////////////////////////////////////////////////////////////////////////////
void EHttpConnection::OnBeginHTMLData( void )
{
	m_iSeekEndOfHTML = 0 ;
}

// HTML データの終端記号のチェック
//////////////////////////////////////////////////////////////////////////////
bool EHttpConnection::IsEndOfHTMLData
	( const BYTE * ptrBuf, unsigned long int nBufLen )
{
	const BYTE *	ptrEndOfHTML = (const BYTE *) m_pszEndOfHTML ;
	bool			fFindCloser = false ;
	for ( unsigned long int i = 0; i < nBufLen; i ++ )
	{
		BYTE	c1 = ptrEndOfHTML[m_iSeekEndOfHTML] ;
		if ( c1 == ' ' )
		{
			if ( ptrBuf[i] > ' ' )
			{
				if ( ptrEndOfHTML[++ m_iSeekEndOfHTML] == '\0' )
				{
					m_iSeekEndOfHTML = 0 ;
					fFindCloser = true ;
					break ;
				}
				i -- ;
				continue ;
			}
		}
		else if ( (c1 >= 'a') & (c1 <= 'z') )
		{
			BYTE	c2 = ptrBuf[i] ;
			if ( (c2 >= 'A') & (c2 <= 'Z') )
			{
				c2 += (BYTE) ('a' - 'A') ;
			}
			if ( c1 != c2 )
			{
				if ( m_iSeekEndOfHTML > 0 )
				{
					i -- ;
				}
				m_iSeekEndOfHTML = 0 ;
			}
			else
			{
				m_iSeekEndOfHTML ++ ;
			}
		}
		else if ( (c1 >= 'A') & (c1 <= 'Z') )
		{
			BYTE	c2 = ptrBuf[i] ;
			if ( (c2 >= 'a') & (c2 <= 'z') )
			{
				c2 -= (BYTE) ('a' - 'A') ;
			}
			if ( c1 != c2 )
			{
				if ( m_iSeekEndOfHTML > 0 )
				{
					i -- ;
				}
				m_iSeekEndOfHTML = 0 ;
			}
			else
			{
				m_iSeekEndOfHTML ++ ;
			}
		}
		else if ( !c1 )
		{
			m_iSeekEndOfHTML = 0 ;
			fFindCloser = true ;
			break ;
		}
		else if ( c1 == ptrBuf[i] )
		{
			m_iSeekEndOfHTML ++ ;
		}
		else
		{
			if ( m_iSeekEndOfHTML > 0 )
			{
				i -- ;
			}
			m_iSeekEndOfHTML = 0 ;
		}
	}
	return	fFindCloser || (ptrEndOfHTML[m_iSeekEndOfHTML] == '\0') ;
}

// 受信
//////////////////////////////////////////////////////////////////////////////
int EHttpConnection::Receive( void * ptrBuf, int nBufLen )
{
	int	nRecvLen ;
	Lock( ) ;
	nRecvLen = m_bufRecvHttp.Read( ptrBuf, nBufLen ) ;
	if ( m_bufRecvHttp.GetLength() == 0 )
	{
		::ResetEvent( m_hRecvData ) ;
	}
	Unlock( ) ;
	return	nRecvLen ;
}

// １行受信
//////////////////////////////////////////////////////////////////////////////
int EHttpConnection::ReceiveLine( EString & strLine )
{
	int		nResult ;
	Lock( ) ;
	EPtrBuffer	ptrbuf = m_bufRecvHttp.GetBuffer( ) ;
	const char *	pstrBuf = (const char *) ptrbuf.GetBuffer( ) ;
	unsigned int	nBufLen = ptrbuf.GetLength( ) ;
	unsigned int	i ;
	for ( i = 0; i < nBufLen; i ++ )
	{
		if ( pstrBuf[i] == '\n' )
			break ;
	}
	if ( i < nBufLen )
	{
		strLine = EString( pstrBuf, i + 1 ) ;
		m_bufRecvHttp.Release( i + 1 ) ;
		nResult = i + 1 ;
		if ( m_bufRecvHttp.GetLength() == 0 )
		{
			::ResetEvent( m_hContentRecv ) ;
		}
	}
	else
	{
		strLine = "" ;
		m_bufRecvHttp.Release( 0 ) ;
		nResult = -1 ;
	}
	Unlock( ) ;
	return	nResult ;
}

// 何らかのデータを受信するまで待つ
//////////////////////////////////////////////////////////////////////////////
ESLError EHttpConnection::WaitUntilReceived( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hContentRecv, dwTimeout, fDispMsg ) ;
}

// URL フォーマット
//////////////////////////////////////////////////////////////////////////////
EString EHttpConnection::FormatURL( const char * pszURL )
{
	static const char	cSafeChar[] = ";/?:@=&$-_.+!*'(),\"" ;
	int		i, iStart = 0 ;
	EString	strURL ;
	for ( i = 0; pszURL[i]; i ++ )
	{
		char	c = pszURL[i] ;
		bool	f = false ;
		if ( (('0' <= c) && (c <= '9'))
			|| (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z')) )
		{
			// 英数字
		}
		else if ( (c & 0x80) || ((0 <= c) && (c < 0x20)) || (c == 0x7F) )
		{
			// 制御文字
			f = true ;
		}
		else
		{
			// 記号
			f = true ;
			for ( int j = 0; cSafeChar[j]; j ++ )
			{
				if ( c == cSafeChar[j] )
				{
					f = false ;
					break ;
				}
			}
		}
		if ( f )
		{
			strURL += EString( pszURL + iStart, i - iStart ) ;
			//
			char	cBuf[4] ;
			int		k ;
			cBuf[0] = '%' ;
			//
			k = (c >> 4) & 0x0F ;
			if ( k < 10 )
				cBuf[1] = (char)('0' + k) ;
			else
				cBuf[1] = (char)('A' + k - 10) ;
			//
			k = c & 0x0F ;
			if ( k < 10 )
				cBuf[2] = (char)('0' + k) ;
			else
				cBuf[2] = (char)('A' + k - 10) ;
			//
			cBuf[3] = 0 ;
			//
			strURL += cBuf ;
			iStart = i + 1 ;
		}
	}
	strURL += EString( pszURL + iStart, i - iStart ) ;
	return	strURL ;
}

// URL フォーマットを復元
//////////////////////////////////////////////////////////////////////////////
EString EHttpConnection::UnformatURL( const char * pszURL )
{
	if ( pszURL == NULL )
	{
		return	EString() ;
	}
	EString	strUnformatURL ;
	int	i = 0, iLast = 0 ;
	for ( ; ; )
	{
		char	c = pszURL[i ++] ;
		if ( c == '%' )
		{
			char	cCode = 0 ;
			strUnformatURL += EString( pszURL + iLast, i - iLast - 1 ) ;
			c = pszURL[i] ;
			if ( c )
			{
				if ( (c >= '0') && (c <= '9') )
				{
					cCode = (c - '0') << 4 ;
				}
				else if ( (c >= 'A') && (c <= 'F') )
				{
					cCode = (c + (10 - 'A')) << 4 ;
				}
				else if ( (c >= 'a') && (c <= 'f') )
				{
					cCode = (c + (10 - 'A')) << 4 ;
				}
				c = pszURL[++ i] ;
				if ( c )
				{
					if ( (c >= '0') && (c <= '9') )
					{
						cCode += (c - '0') ;
					}
					else if ( (c >= 'A') && (c <= 'F') )
					{
						cCode += (c + (10 - 'A')) ;
					}
					else if ( (c >= 'a') && (c <= 'f') )
					{
						cCode += (c + (10 - 'A')) ;
					}
					i ++ ;
					strUnformatURL += cCode ;
				}
			}
			iLast = i ;
		}
		else if ( c == '\0' )
		{
			break ;
		}
	}
	strUnformatURL += pszURL + iLast ;
	return	strUnformatURL ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP (Hypertext Transfer Protocol) ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ESyncHttpFile, ESyncStreamFile, EHttpConnection )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESyncHttpFile::ESyncHttpFile( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESyncHttpFile::~ESyncHttpFile( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncHttpFile::OpenURL
	( const char * pszURL, DWORD dwTimeout, bool fDispMsg,
		const char * pszAgent, const char * pszData,
		unsigned int nLength, const char * pszContentType )
{
	//
	// パラメータ設定
	//
	if ( pszData != NULL )
	{
		SetRequest( pszURL, "POST" ) ;
		SetSendData( pszData, nLength ) ;
		if ( pszContentType != NULL )
		{
			AddHeader( EString("Content-Type: ") + pszContentType ) ;
		}
		else
		{
			AddHeader( "Content-Type: application/x-www-form-urlencoded" ) ;
		}
	}
	else
	{
		SetRequest( pszURL, "GET" ) ;
	}
	if ( pszAgent != NULL )
	{
		SetHeaderUserAgent( pszAgent ) ;
	}
	//
	// ホストに接続
	//
	if ( ConnectHost( ) )
	{
		if ( WaitUntilConnected( dwTimeout, fDispMsg ) != eslErrSuccess )
		{
			return	ESLErrorMsg( "ホストに接続できませんでした。" ) ;
		}
	}
	//
	// リクエスト送信
	//
	ESLError	err = SendRequest( ) ;
	if ( err )
	{
		return	err ;
	}
	ESyncStreamFile::Lock( ) ;
	if ( WaitRecvHeader( dwTimeout, fDispMsg ) != eslErrSuccess )
	{
		ESyncStreamFile::Unlock( ) ;
		return	ESLErrorMsg( "サーバから応答がありません。" ) ;
	}
	//
	// エラー判定
	//
	DWORD	dwStatusCode = GetStatusCode( ) ;
	if ( (dwStatusCode < 200) || (dwStatusCode > 299) )
	{
		ESyncStreamFile::Unlock( ) ;
		return	ESLErrorMsg( "サーバからエラーが返されました。" ) ;
	}
	//
	// 初期化
	//
	DWORD	dwTotal ;
	GetCurrentStatus( &dwTotal ) ;
	ESyncStreamFile::Initialize( !dwTotal ? -1 : dwTotal ) ;
	ESyncStreamFile::Unlock( ) ;
	//
	return	eslErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void ESyncHttpFile::Close( void )
{
	EHttpConnection::Close( ) ;
	//
	FinishStream( ) ;
}

// 排他アクセス
//////////////////////////////////////////////////////////////////////////////
void ESyncHttpFile::Lock( void )
{
	ESyncStreamFile::Lock( ) ;
}

void ESyncHttpFile::Unlock( void )
{
	ESyncStreamFile::Unlock( ) ;
}

// 受信データがある
//////////////////////////////////////////////////////////////////////////////
void ESyncHttpFile::OnReceive( int nErrorCode )
{
	EHttpConnection::OnReceive( nErrorCode ) ;
	//
	EHttpConnection::Lock( ) ;
	EPtrBuffer	ptrbuf = m_bufRecvHttp.GetBuffer( ) ;
	if ( ptrbuf.GetLength() != 0 )
	{
		ESyncStreamFile::Lock( ) ;
		ESyncStreamFile::Write( ptrbuf, ptrbuf.GetLength() ) ;
		ESyncStreamFile::Unlock( ) ;
	}
	m_bufRecvHttp.Release( ptrbuf.GetLength() ) ;
	EHttpConnection::Unlock( ) ;
}

// ソケットが閉じられた
//////////////////////////////////////////////////////////////////////////////
void ESyncHttpFile::OnClose( int nErrorCode )
{
	EHttpConnection::OnClose( nErrorCode ) ;
	ESyncHttpFile::OnReceive( nErrorCode ) ;
	//
	FinishStream( ) ;
}


//////////////////////////////////////////////////////////////////////////////
// FTP (FILE TRANSFER PROTOCOL)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EFtpConnection, ESocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EFtpConnection::EFtpConnection( void )
{
	m_dwDataSize = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EFtpConnection::~EFtpConnection( void )
{
}

// FTP サーバに接続する
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::ConnectHost
	( const char * pszHostAddr, int nPort, DWORD dwTimeout )
{
	//
	// サーバに接続
	//
	ESLError	err = Create( ) ;
	if ( err )
	{
		return	err ;
	}
	Connect( pszHostAddr, nPort ) ;
	//
	err = WaitUntilConnected( dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// サーバからの応答を取得
	//
	EString	strResLine ;
	WaitUntilReceived( dwTimeout ) ;
	if ( ReceiveLine( strResLine ) >= 3 )
	{
		if ( strResLine.GetAt(0) != '2' )
		{
			return	eslErrGeneral ;
		}
	}
	else
	{
		Close( ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// FTP サーバにログインする
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::Login
	( const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	//
	// USER コマンド
	//
	ESLError	err ;
	EString	strResponse ;
	EString	strUserCmd = "USER " ;
	strUserCmd += pszUser ;
	err = SendCommand( strUserCmd, strResponse, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (strResponse.GetAt(0) != '2')
		&& (strResponse.GetAt(0) != '3') )
	{
		return	eslErrGeneral ;
	}
	//
	// PASS コマンド
	//
	EString	strPassCmd = "PASS " ;
	if ( pszPassword != NULL )
	{
		strPassCmd += pszPassword ;
	}
	err = SendCommand( strPassCmd, strResponse, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	if ( strResponse.GetAt(0) != '2' )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// コマンドを実行する
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::SendCommand
	( const char * pszCmdLine, EString & strResponse, DWORD dwTimeout )
{
	EString	strCmdLine = pszCmdLine ;
	strCmdLine.TrimRight( ) ;
	strCmdLine += "\r\n" ;
	Send( strCmdLine.CharPtr(), strCmdLine.GetLength() ) ;
	//
	if ( ReceiveLineTimeout( strResponse, dwTimeout ) < 0 )
	{
		return	eslErrTimeout ;
	}
	return	eslErrSuccess ;
}

// PASV コマンドを実行する
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::PassiveMode( ESocket & sockData, DWORD dwTimeout )
{
	//
	// PASV コマンド送信
	//
	ESLError	err ;
	EString		strResponse ;
	err = SendCommand( "PASV", strResponse, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (strResponse.GetAt(0) != '2')
		|| (strResponse.GetAt(1) != '2')
		|| (strResponse.GetAt(2) != '7') )
	{
		return	eslErrGeneral ;
	}
	//
	// 接続先取得
	//
	int	iFind = strResponse.Find( '(' ) ;
	if ( iFind < 0 )
	{
		return	eslErrGeneral ;
	}
	int	nNums[6] ;
	int	n = 0, i = 0, j = iFind + 1 ;
	for ( ; ; )
	{
		char	c = strResponse.GetAt( j ++ ) ;
		if ( c == ',' )
		{
			if ( i >= 5 )
			{
				return	eslErrGeneral ;
			}
			nNums[i ++] = n ;
			n = 0 ;
		}
		else if ( c == ')' )
		{
			if ( i < 5 )
			{
				return	eslErrGeneral ;
			}
			nNums[i] = n ;
			break ;
		}
		else if ( (c >= '0') && (c <= '9') )
		{
			n = n * 10 + (c - '0') ;
		}
		else if ( c > ' ' )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// ホストに接続
	//
	EString	strHostAddr =
		EString(nNums[0]) + "." + EString(nNums[1])
			+ "." + EString(nNums[2]) + "." + EString(nNums[3]) ;
	int		nHostPort = nNums[4] * 256 + nNums[5] ;
	//
	err = sockData.Create( ) ;
	if ( err )
	{
		return	err ;
	}
	sockData.Connect( strHostAddr, nHostPort ) ;
	err = sockData.WaitUntilConnected( dwTimeout ) ;
	return	err ;

}

// FTP 上のファイルを開く（ダウンロード）
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::OpenURL
	( ESocket & sockData,
		const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	//
	// URL 解釈
	//
	EString	strHost, strPath, strUser, strPass ;
	ParseURL( strHost, strPath, strUser, strPass, pszURL ) ;
	if ( strHost.IsEmpty() )
	{
		return	eslErrGeneral ;
	}
	if ( pszUser == NULL )
	{
		if ( !strUser.IsEmpty() )
		{
			pszUser = strUser ;
			pszPassword = strPass ;
		}
		else
		{
			pszUser = "anonymous" ;
		}
	}
	//
	// FTP 接続
	//
	ESLError	err ;
	strPath = EHttpConnection::UnformatURL( strPath ) ;
	err = OpenFTP( sockData, strHost, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ファイル取得コマンド
	//
	EString	strResponse ;
	EString	strCmdLine = "RETR " + strPath ;
	err = SendCommand( strCmdLine, strResponse, dwTimeout ) ;
	if ( !err )
	{
		if ( (strResponse.GetAt(0) != '1')
			&& (strResponse.GetAt(0) != '2') )
		{
			return	eslErrGeneral ;
		}
		int	iFileSize = strResponse.Find( '(' ) ;
		if ( iFileSize >= 0 )
		{
			m_dwDataSize =
				strResponse.Middle( iFileSize + 1 ).AsInteger() ;
		}
		else
		{
			m_dwDataSize = 0 ;
		}
	}
	return	err ;
}

// FTP 上のファイルリストを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::OpenList
	( ESocket & sockData,
		const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	//
	// URL 解釈
	//
	EString	strHost, strPath, strUser, strPass ;
	ParseURL( strHost, strPath, strUser, strPass, pszURL ) ;
	if ( strHost.IsEmpty() )
	{
		return	eslErrGeneral ;
	}
	if ( pszUser == NULL )
	{
		if ( !strUser.IsEmpty() )
		{
			pszUser = strUser ;
			pszPassword = strPass ;
		}
		else
		{
			pszUser = "anonymous" ;
		}
	}
	//
	// FTP 接続
	//
	ESLError	err ;
	strPath = EHttpConnection::UnformatURL( strPath ) ;
	err = OpenFTP( sockData, strHost, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// LIST コマンド
	//
	EString	strResponse ;
	if ( strPath.Right(1) == "/" )
	{
		strPath = strPath.Left( strPath.GetLength() - 1 ) ;
	}
	EString	strCmdLine = "LIST " + strPath ;
	err = SendCommand( strCmdLine, strResponse, dwTimeout ) ;
	if ( !err )
	{
		if ( (strResponse.GetAt(0) != '1')
			&& (strResponse.GetAt(0) != '2') )
		{
			return	eslErrGeneral ;
		}
	}
	return	err ;
}

// FTP にファイルをアップロードする
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::OpenUpload
	( ESocket & sockData,
		const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	//
	// URL 解釈
	//
	EString	strHost, strPath, strUser, strPass ;
	ParseURL( strHost, strPath, strUser, strPass, pszURL ) ;
	if ( strHost.IsEmpty() )
	{
		return	eslErrGeneral ;
	}
	if ( pszUser == NULL )
	{
		if ( !strUser.IsEmpty() )
		{
			pszUser = strUser ;
			pszPassword = strPass ;
		}
		else
		{
			pszUser = "anonymous" ;
		}
	}
	//
	// FTP 接続
	//
	ESLError	err ;
	strPath = EHttpConnection::UnformatURL( strPath ) ;
	err = OpenFTP( sockData, strHost, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ファイル送信コマンド
	//
	EString	strResponse ;
	EString	strCmdLine = "STOR " + strPath ;
	err = SendCommand( strCmdLine, strResponse, dwTimeout ) ;
	if ( !err )
	{
		if ( (strResponse.GetAt(0) != '1')
			&& (strResponse.GetAt(0) != '2') )
		{
			return	eslErrGeneral ;
		}
	}
	return	err ;
}

// FTP を開く（共通動作）
//////////////////////////////////////////////////////////////////////////////
ESLError EFtpConnection::OpenFTP
	( ESocket & sockData,
		const char * pszHost, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	//
	// ホストに接続
	//
	ESLError	err ;
	err = ConnectHost( pszHost, 21, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ログイン
	//
	if ( pszUser != NULL )
	{
		err = Login( pszUser, pszPassword, dwTimeout ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 転送モード設定
	//
	EString	strResponse ;
	err = SendCommand( "MODE S", strResponse, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	err = SendCommand( "TYPE I", strResponse, dwTimeout ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// パッシブモード設定
	//
	return	PassiveMode( sockData, dwTimeout ) ;
}

// URL からホスト名とパスを分離
//////////////////////////////////////////////////////////////////////////////
void EFtpConnection::ParseURL
	( EString & strHost, EString & strPath,
		EString & strUser, EString & strPassword, const char * pszURL )
{
	EString	strURL = EHttpConnection::FormatURL( pszURL ) ;
	if ( !strURL.CompareLeftNoCase( "ftp:" ) )
	{
		strURL = strURL.Middle( 4 ) ;
	}
	if ( !strURL.CompareLeft( "//" ) )
	{
		strURL = strURL.Middle( 2 ) ;
	}
	int	iHost = strURL.Find( '/' ) ;
	if ( iHost >= 0 )
	{
		strHost = strURL.Left( iHost ) ;
		strPath = strURL.Middle( iHost ) ;
	}
	else
	{
		strHost = strURL ;
		strPath = "/" ;
	}
	int	iUser = strHost.Find( '@' ) ;
	if ( iUser > 0 )
	{
		int	iPass = strHost.Find( ':' ) ;
		if ( (iPass >= 0) && (iPass < iUser) )
		{
			strUser = strHost.Left( iPass ) ;
			strPassword = strHost.Middle( iPass + 1, iUser - iPass - 1 ) ;
			strHost = strHost.Middle( iUser + 1 ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ソケットファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ESocketFile, ESLFileObject, ESocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESocketFile::ESocketFile( void )
{
	SetAttribute( modeReadWrite ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESocketFile::~ESocketFile( void )
{
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ESocketFile::Duplicate( void ) const
{
	ESLAssert( false ) ;
	return	NULL ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESocketFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	unsigned long int	nPos = 0 ;
	while ( nPos < nBytes )
	{
		int	nRecvBytes =
			Receive( ((BYTE*)ptrBuffer) + nPos, nBytes - nPos ) ;
		nPos += nRecvBytes ;
		if ( nPos >= nBytes )
		{
			break ;
		}
		if ( WaitUntilReceived( INFINITE ) )
		{
			nRecvBytes = Receive( ((BYTE*)ptrBuffer) + nPos, nBytes - nPos ) ;
			nPos += nRecvBytes ;
			break ;
		}
	}
	return	nPos ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESocketFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	int	nSentBytes = Send( ptrBuffer, nBytes ) ;
	//
	WaitUntilSent( INFINITE ) ;
	//
	return	nSentBytes ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESocketFile::GetLength( void ) const
{
	ESLAssert( false ) ;
	return	0 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESocketFile::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	ESLAssert( false ) ;
	return	0 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESocketFile::GetPosition( void ) const
{
	ESLAssert( false ) ;
	return	0 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESocketFile::SetEndOfFile( void )
{
	Close( ) ;
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// FTP (FILE TRANSFER PROTOCOL) ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ESyncFtpFile, ESyncStreamFile, ESocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESyncFtpFile::ESyncFtpFile( void )
{
	m_pFtpConnection = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESyncFtpFile::~ESyncFtpFile( void )
{
	Close( ) ;
}

// FTP 上のファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncFtpFile::OpenURL
	( const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	ESLError	err ;
	//
	Close( ) ;
	//
	ESLAssert( m_pFtpConnection == NULL ) ;
	m_pFtpConnection = new EFtpConnection ;
	//
	Initialize( ) ;
	err = m_pFtpConnection->OpenURL
			( *this, pszURL, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		Close( ) ;
		return	err ;
	}
	return	err ;
}

// FTP 上のファイルリストを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncFtpFile::OpenList
	( const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	ESLError	err ;
	//
	Close( ) ;
	//
	ESLAssert( m_pFtpConnection == NULL ) ;
	m_pFtpConnection = new EFtpConnection ;
	//
	Initialize( ) ;
	err = m_pFtpConnection->OpenList
			( *this, pszURL, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		Close( ) ;
		return	err ;
	}
	return	err ;
}

// FTP にファイルをアップロードする
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncFtpFile::OpenUpload
	( const char * pszURL, const char * pszUser,
		const char * pszPassword, DWORD dwTimeout )
{
	ESLError	err ;
	//
	Close( ) ;
	//
	ESLAssert( m_pFtpConnection == NULL ) ;
	m_pFtpConnection = new EFtpConnection ;
	//
	SetAttribute( modeWrite ) ;
	err = m_pFtpConnection->OpenUpload
			( *this, pszURL, pszUser, pszPassword, dwTimeout ) ;
	if ( err )
	{
		Close( ) ;
		return	err ;
	}
	return	err ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void ESyncFtpFile::Close( void )
{
	if ( m_pFtpConnection != NULL )
	{
		FinishStream( ) ;
		m_pFtpConnection->Close( ) ;
		delete	m_pFtpConnection ;
		m_pFtpConnection = NULL ;
	}
	ESocket::Close( ) ;
}

// 排他アクセス
//////////////////////////////////////////////////////////////////////////////
void ESyncFtpFile::Lock( void )
{
	ESyncStreamFile::Lock( ) ;
}

void ESyncFtpFile::Unlock( void )
{
	ESyncStreamFile::Unlock( ) ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncFtpFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( GetAttribute() & modeRead ) ;
	return	ESyncStreamFile::Read( ptrBuffer, nBytes ) ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncFtpFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( !(GetAttribute() & modeRead) ) ;
	unsigned long int	nSend = Send( ptrBuffer, nBytes ) ;
	WaitUntilSent( INFINITE ) ;
	return	nSend ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncFtpFile::GetLength( void ) const
{
	if ( m_pFtpConnection != NULL )
	{
		return	m_pFtpConnection->GetDownloadDataSize( ) ;
	}
	return	0 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncFtpFile::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	ESLAssert( GetAttribute() & modeRead ) ;
	return	ESyncStreamFile::Seek( nOffsetPos, fSeekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncFtpFile::GetPosition( void ) const
{
	ESLAssert( GetAttribute() & modeRead ) ;
	return	ESyncStreamFile::GetPosition( ) ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncFtpFile::SetEndOfFile( void )
{
	ESocket::Close( ) ;
	FinishStream( ) ;
	//
	if ( m_pFtpConnection != NULL )
	{
		m_pFtpConnection->Close( ) ;
		delete	m_pFtpConnection ;
		m_pFtpConnection = NULL ;
	}
	return	eslErrSuccess ;
}

// 受信データがある
//////////////////////////////////////////////////////////////////////////////
void ESyncFtpFile::OnReceive( int nErrorCode )
{
	ESocket::OnReceive( nErrorCode ) ;
	//
	ESocket::Lock( ) ;
	EPtrBuffer	ptrbuf = m_bufRecv.GetBuffer( ) ;
	if ( ptrbuf.GetLength() != 0 )
	{
		ESyncStreamFile::Lock( ) ;
		ESyncStreamFile::Write( ptrbuf, ptrbuf.GetLength() ) ;
		ESyncStreamFile::Unlock( ) ;
	}
	m_bufRecv.Release( ptrbuf.GetLength() ) ;
	ESocket::Unlock( ) ;
}

// ソケットが閉じられた
//////////////////////////////////////////////////////////////////////////////
void ESyncFtpFile::OnClose( int nErrorCode )
{
	ESocket::OnClose( nErrorCode ) ;
	ESyncFtpFile::OnReceive( nErrorCode ) ;
	//
	FinishStream( ) ;
}


//////////////////////////////////////////////////////////////////////////////
// GCTP (Game Command Transfer Protocol)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGctpConnection, ESocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGctpConnection::EGctpConnection( void )
{
	m_hRecvAnyCmd = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	//
	m_hWndNotify = NULL ;
	//
	m_pDecodeERISA = NULL ;
	m_pDecodeBSHF = NULL ;
	m_pEncodeERISA = NULL ;
	m_pEnocdeBSHF = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGctpConnection::~EGctpConnection( void )
{
	Close( ) ;
	//
	::CloseHandle( m_hRecvAnyCmd ) ;
}

// ソケットを閉じる
//////////////////////////////////////////////////////////////////////////////
void EGctpConnection::Close( void )
{
	ESocket::Close( ) ;
	//
	m_queRecvCmd.RemoveAll( ) ;
	::ResetEvent( m_hRecvAnyCmd ) ;
	m_hWndNotify = NULL ;
	//
	delete	m_pDecodeERISA ;
	m_pDecodeERISA = NULL ;
	delete	m_pDecodeBSHF ;
	m_pDecodeBSHF = NULL ;
	delete	m_pEncodeERISA ;
	m_pEncodeERISA = NULL ;
	delete	m_pEnocdeBSHF ;
	m_pEnocdeBSHF = NULL ;
}

// 何らかのコマンドを受信したことを通知するメッセージを設定
//////////////////////////////////////////////////////////////////////////////
void EGctpConnection::SetNotifyWindow( HWND hWndNotify, UINT nMsg )
{
	m_hWndNotify = hWndNotify ;
	m_nMsgNotify = nMsg ;
}

// 何らかのコマンドを受信するまで待機
//////////////////////////////////////////////////////////////////////////////
ESLError EGctpConnection::WaitForCommand( DWORD dwTimeout, bool fDispMsg )
{
	return	WaitEvent( m_hRecvAnyCmd, dwTimeout, fDispMsg ) ;
}

// コマンドを取得
//////////////////////////////////////////////////////////////////////////////
ESLError EGctpConnection::GetCommand( EDescription & cmd, const char * pszCmd )
{
	unsigned int	i, nCount ;
	Lock( ) ;
	i = 0 ;
	nCount = m_queRecvCmd.GetSize( ) ;
	if ( pszCmd != NULL )
	{
		EWideString	wstrCmd = pszCmd ;
		for ( i = 0; i < nCount; i ++ )
		{
			EDescription *	pCmd = m_queRecvCmd.GetAt( i ) ;
			if ( pCmd != NULL )
			{
				if ( pCmd->Tag() == wstrCmd )
					break ;
			}
		}
	}
	if ( i < nCount )
	{
		EDescription *	pCmd = m_queRecvCmd.GetAt( i ) ;
		if ( pCmd != NULL )
		{
			cmd = *pCmd ;
			m_queRecvCmd.RemoveAt( i ) ;
			if ( m_queRecvCmd.GetSize() == 0 )
			{
				::ResetEvent( m_hRecvAnyCmd ) ;
			}
			Unlock( ) ;
			return	eslErrSuccess ;
		}
	}
	Unlock( ) ;
	return	eslErrGeneral ;
}

// コマンドを送信
//////////////////////////////////////////////////////////////////////////////
ESLError EGctpConnection::SendCommand( const EDescription & cmd, bool fEncoding )
{
	EString	strCmd ;
	EncodeCommand( strCmd, cmd, fEncoding ) ;
	if ( ESocket::Send
		( strCmd.CharPtr(), strCmd.GetLength() ) < (int) strCmd.GetLength() )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 符号化方式設定
//////////////////////////////////////////////////////////////////////////////
ESLError EGctpConnection::SetEncodingType
			( int fEncodingType, const char * pszPassword )
{
	//
	// 既存の符号化オブジェクトを削除する
	//
	delete	m_pDecodeERISA ;
	m_pDecodeERISA = NULL ;
	delete	m_pDecodeBSHF ;
	m_pDecodeBSHF = NULL ;
	delete	m_pEncodeERISA ;
	m_pEncodeERISA = NULL ;
	delete	m_pEnocdeBSHF ;
	m_pEnocdeBSHF = NULL ;
	//
	// 符号化オブジェクト初期化
	//
	if ( fEncodingType == ERISAArchive::etERISACode )
	{
		m_pDecodeERISA = new ERISADecodeContext( 0x1000 ) ;
		m_pEncodeERISA = new ERISAEncodeContext( 0x1000 ) ;
		m_pDecodeERISA->PrepareToDecodeERISANCode( ) ;
		m_pEncodeERISA->PrepareToEncodeERISANCode( ) ;
	}
	else if ( fEncodingType == ERISAArchive::etBSHFCrypt )
	{
		m_pDecodeBSHF = new ERISADecodeContext( 0x1000 ) ;
		m_pEnocdeBSHF = new ERISAEncodeContext( 0x1000 ) ;
		m_pDecodeBSHF->PrepareToDecodeBSHFCode( pszPassword ) ;
		m_pEnocdeBSHF->PrepareToEncodeBSHFCode( pszPassword ) ;
	}
	else if ( fEncodingType == ERISAArchive::etERISACrypt )
	{
		m_pDecodeERISA = new ERISADecodeContext( 0x1000 ) ;
		m_pEncodeERISA = new ERISAEncodeContext( 0x1000 ) ;
		m_pDecodeBSHF = new ERISADecodeContext( 0x1000 ) ;
		m_pEnocdeBSHF = new ERISAEncodeContext( 0x1000 ) ;
		m_pDecodeERISA->PrepareToDecodeERISANCode( ) ;
		m_pEncodeERISA->PrepareToEncodeERISANCode( ) ;
		m_pDecodeBSHF->PrepareToDecodeBSHFCode( pszPassword ) ;
		m_pEnocdeBSHF->PrepareToEncodeBSHFCode( pszPassword ) ;
	}
	//
	return	eslErrSuccess ;
}

// コマンドをエンコードする
//////////////////////////////////////////////////////////////////////////////
void EGctpConnection::EncodeCommand
	( EString & strCmd, const EDescription & cmd, bool fEncoding )
{
	//
	// コマンドラインをフォーマットする
	//
	EWideString	wstrCmd = L"<" ;
	wstrCmd += cmd.Tag( ) ;
	//
	int		i, nCount ;
	nCount = cmd.GetAttributeCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EWideString	wstrName = cmd.GetAttributeNameAt( i ) ;
		EWideString	wstrValue = cmd.GetAttributeValueAt( i ) ;
		EDescription::EncodeTextCEscSequence( wstrValue ) ;
		wstrCmd += L" " + wstrName + L"=\"" + wstrValue + L"\"" ;
	}
	wstrCmd += L"/>" ;
	//
	EDescription::EncodeText( strCmd, wstrCmd, EDescription::ceShiftJIS ) ;
	//
	// 圧縮・暗号化を施す
	//
	if ( m_pEncodeERISA || m_pEnocdeBSHF )
	{
		EStreamFileBuffer	sfb ;
		EStreamBuffer	buf ;
		EString	strHdr = "#" ;
		strCmd += "\r\n" ;
		buf.Write( strCmd.CharPtr(), strCmd.GetLength() ) ;
		//
		if ( m_pEncodeERISA != NULL )
		{
			strHdr += "p" ;
			//
			EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
			m_pEncodeERISA->AttachOutputFile( &sfb ) ;
			m_pEncodeERISA->EncodeERISANCodeBytes
				( (const SBYTE *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
			m_pEncodeERISA->EncodeERISANCodeEOF( ) ;
			m_pEncodeERISA->FinishERISACode( ) ;
			buf.Release( ptrbuf.GetLength() ) ;
			//
			unsigned int	nLength = sfb.GetLength( ) ;
			sfb.Read( buf.PutBuffer(nLength), nLength ) ;
			buf.Flush( nLength ) ;
		}
		//
		if ( m_pEnocdeBSHF != NULL )
		{
			strHdr += "c" ;
			//
			EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
			m_pEnocdeBSHF->AttachOutputFile( &sfb ) ;
			m_pEnocdeBSHF->EncodeBSHFCodeBytes
				( (const SBYTE *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
			m_pEnocdeBSHF->FinishBSHFCode( ) ;
			buf.Release( ptrbuf.GetLength() ) ;
			//
			unsigned int	nLength = sfb.GetLength( ) ;
			sfb.Read( buf.PutBuffer(nLength), nLength ) ;
			buf.Flush( nLength ) ;
		}
		//
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		EDescription::EncodeBase64
			( strCmd, ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
		strCmd = strHdr + ":" + strCmd ;
	}
	//
	// 改行コードを付加する
	//
	strCmd += "\r\n" ;
}

// コマンドをデコードする
//////////////////////////////////////////////////////////////////////////////
ESLError EGctpConnection::DecodeCommand( EDescription & cmd, EString & strCmd )
{
	if ( strCmd[0] == '#' )
	{
		//
		// 暗号化・圧縮の書式を解析
		//
		int		iSep = strCmd.Find( ':', 1 ) ;
		if ( iSep <= 1 )
		{
			ESLTrace( "不正なコマンドフォーマットです。\n" ) ;
			return	eslErrGeneral ;
		}
		EString	strData = strCmd.Middle( iSep + 1 ) ;
		EStreamBuffer	buf ;
		EDescription::DecodeBase64( buf, strData ) ;
		//
		// 暗号化を復号
		//
		iSep -- ;
		if ( strCmd[iSep] == 'c' )
		{
			if ( m_pDecodeBSHF == NULL )
			{
				ESLTrace( "暗号化されたコマンドを受信しましたが、"
						"暗号化が設定されていないので、復号できません。\n" ) ;
				return	eslErrGeneral ;
			}
			iSep -- ;
			//
			EStreamFileBuffer	sfb ;
			EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
			unsigned int	nLength = ptrbuf.GetLength( ) ;
			sfb.Write( ptrbuf.GetBuffer(), nLength ) ;
			buf.Release( nLength ) ;
			//
			m_pDecodeBSHF->FlushBuffer( ) ;
			m_pDecodeBSHF->AttachInputFile( &sfb ) ;
			m_pDecodeBSHF->DecodeBSHFCodeBytes
				( (SBYTE*) buf.PutBuffer(nLength), nLength ) ;
			buf.Flush( nLength ) ;
		}
		//
		// 圧縮を展開
		//
		if ( strCmd[iSep] == 'p' )
		{
			if ( m_pDecodeERISA == NULL )
			{
				ESLTrace( "圧縮されたコマンドを受信しましたが、"
						"圧縮が設定されていないので、展開できません。\n" ) ;
				return	eslErrGeneral ;
			}
			//
			EStreamFileBuffer	sfb ;
			EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
			unsigned int	nLength = ptrbuf.GetLength( ) ;
			sfb.Write( ptrbuf.GetBuffer(), nLength ) ;
			buf.Release( nLength ) ;
			//
			m_pDecodeERISA->FlushBuffer( ) ;
			m_pDecodeERISA->AttachInputFile( &sfb ) ;
			//
			char	cBuf[0x20] ;
			for ( ; ; )
			{
				ULONG	nDecoded = m_pDecodeERISA->
					DecodeERISANCodeBytes( (SBYTE*) cBuf, 0x20 ) ;
				buf.Write( &cBuf[0], nDecoded ) ;
				if ( nDecoded < 0x20 )
				{
					break ;
				}
			}
		}
		//
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		strCmd = EString
			( (const char *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
	}
	//
	// 書式を解析
	//
	EStreamBuffer	buf ;
	EDescription	desc ;
	buf.Write( strCmd.CharPtr(), strCmd.GetLength() ) ;
	desc.ReadDescription( buf ) ;
	EDescription *	pTag = desc.GetContentTagAt( 0 ) ;
	if ( pTag != NULL )
	{
		cmd = *pTag ;
	}
	//
	return	eslErrSuccess ;
}

// 受信データがある
//////////////////////////////////////////////////////////////////////////////
void EGctpConnection::OnReceive( int nErrorCode )
{
	ESocket::OnReceive( nErrorCode ) ;
	//
	EString	strLine ;
	Lock( ) ;
	while ( ReceiveLine( strLine ) >= 0 )
	{
		EDescription *	pCmd = new EDescription ;
		if ( DecodeCommand( *pCmd, strLine ) == eslErrSuccess )
		{
			m_queRecvCmd.Add( pCmd ) ;
			::SetEvent( m_hRecvAnyCmd ) ;
			//
			if ( m_hWndNotify != NULL )
			{
				::PostMessage( m_hWndNotify, m_nMsgNotify, 0, (LPARAM) this ) ;
			}
		}
		else
		{
			delete	pCmd ;
		}
	}
	Unlock( ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Internet Session クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EInternetSession, ESLObject )

EInternetSession *	EInternetSession::m_pisFirst = NULL ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EInternetSession::EInternetSession( void )
{
	m_hInternet = NULL ;
	m_pisPrev = NULL ;
	m_pisNext = NULL ;
	m_dwTempCacheThreshold = 0x100000 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EInternetSession::~EInternetSession( void )
{
	Close( ) ;
}

// セッションを開く
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::Open
	( const char * pszAgent, DWORD dwAccessType,
		const char * pszProxyName,
		const char * pszProxyBypass, DWORD dwFlags )
{
	Close( ) ;
	//
	m_hInternet = ::InternetOpen
		( pszAgent, dwAccessType, pszProxyName, pszProxyBypass, dwFlags ) ;
	//
	if ( m_hInternet == NULL )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// セッションを閉じる
//////////////////////////////////////////////////////////////////////////////
void EInternetSession::Close( void )
{
	if ( m_hInternet != NULL )
	{
		InternetCloseHandle( m_hInternet ) ;
		m_hInternet = NULL ;
		//
		DetachChain( ) ;
	}
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::EnableCallback( bool fCallback )
{
	if ( m_hInternet == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( fCallback )
	{
		AddChain( ) ;
		//
		INTERNET_STATUS_CALLBACK
			pOld = ::InternetSetStatusCallback
				( m_hInternet, InternetStatusCallback ) ;
		if ( pOld == INTERNET_INVALID_STATUS_CALLBACK )
		{
			DetachChain( ) ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		::InternetSetStatusCallback( m_hInternet, NULL ) ;
		DetachChain( ) ;
	}
	return	eslErrSuccess ;
}

// オプション取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::QueryOption
	( DWORD dwOption, void * pBuffer, DWORD * pdwBufLen )
{
	ESLAssert( (dwOption >= INTERNET_FIRST_OPTION) &&
					(dwOption <= INTERNET_LAST_OPTION) ) ;

	if ( ::InternetQueryOption
		( m_hInternet, dwOption, pBuffer, pdwBufLen ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// オプション設定
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::SetOption
	( DWORD dwOption, void * pBuffer,
		DWORD dwBufferLength, DWORD dwFlags )
{
	ESLAssert( (dwOption >= INTERNET_FIRST_OPTION) &&
					(dwOption <= INTERNET_LAST_OPTION) ) ;

	if ( ::InternetSetOptionEx
		( m_hInternet, dwOption, pBuffer, dwBufferLength, dwFlags ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// クッキー設定
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::SetCookie
	( const char * pszURL,
		const char * pszCookieName, const char * pszCookieData )
{
	if ( ::InternetSetCookie( pszURL, pszCookieName, pszCookieData ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// クッキー取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetSession::GetCookie
	( const char * pszURL, const char * pszCookieName,
		char * pszCookieData, DWORD dwBufLen )
{
	if ( ::InternetGetCookie
		( pszURL, pszCookieName, pszCookieData, &dwBufLen ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

DWORD EInternetSession::GetCookieLength
	( const char * pszURL, const char * pszCookieName )
{
	DWORD dwRet;
	if ( !::InternetGetCookie( pszURL, pszCookieName, NULL, &dwRet ) )
	{
		dwRet = 0 ;
	}
	return dwRet ;
}

ESLError EInternetSession::GetCookie
	( const char * pszURL,
		const char * pszCookieName, EString & strCookieData )
{
	DWORD dwLen = GetCookieLength( pszURL, pszCookieName ) ;
	if ( ::InternetGetCookie
		( pszURL, pszCookieName, strCookieData.GetBuffer( dwLen ), &dwLen ) )
	{
		strCookieData.ReleaseBuffer( dwLen ) ;
	}
	else
	{
		strCookieData.FreeString( ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// テンポラリファイルの設定
//////////////////////////////////////////////////////////////////////////////
void EInternetSession::SetTemporaryFileInfo
	( const char * pszTempFileBase, DWORD dwSizeThreshold )
{
	m_strTempFileBase = pszTempFileBase ;
	m_dwTempCacheThreshold = dwSizeThreshold ;
}

// テンポラリファイルを作成
//////////////////////////////////////////////////////////////////////////////
ERawFile * EInternetSession::CreateTemporaryFile( DWORD dwSize )
{
	if ( dwSize < m_dwTempCacheThreshold )
	{
		return	NULL ;
	}
	for ( int j = 0; j < 10; j ++ )
	{
		EString	strTempBase =
			m_strTempFileBase + EString( ::timeGetTime(), 8 ) ;
		WIN32_FIND_DATA	wfd ;
		HANDLE	hFind = ::FindFirstFile( strTempBase, &wfd ) ;
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			::FindClose( hFind ) ;
			for ( int i = 1; ; i ++ )
			{
				hFind = ::FindFirstFile
					( strTempBase + EString(i), &wfd ) ;
				if ( hFind == INVALID_HANDLE_VALUE )
				{
					strTempBase += EString(i) ;
					break ;
				}
				::FindClose( hFind ) ;
			}
		}
		ERawFile *	pfile = new ERawFile ;
		if ( pfile->Open
			( strTempBase, ESLFileObject::modeCreate | ESLFileObject::modeRead ) )
		{
			delete	pfile ;
			continue ;
		}
		return	pfile ;
	}
	return	NULL ;
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
void CALLBACK EInternetSession::InternetStatusCallback
	( HINTERNET hInternet, DWORD_PTR dwContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
	EInternetSession *	pisTarget = NULL ;
	eslHeapLock( NULL ) ;
	EInternetSession *	pisNext = m_pisFirst ;
	while ( pisNext != NULL )
	{
		if ( pisNext->m_hInternet == hInternet )
		{
			pisTarget = pisNext ;
			break ;
		}
		pisNext = pisNext->m_pisNext ;
	}
	eslHeapUnlock( NULL ) ;
	//
	if ( pisTarget != NULL )
	{
		pisTarget->OnStatusCallback
			( dwContext, dwInternetStatus,
				lpvStatusInformation, dwStatusInformationLength ) ;
	}
}

void EInternetSession::AddChain( void )
{
	eslHeapLock( NULL ) ;
	if ( !m_pisPrev && !m_pisNext )
	{
		if ( m_pisFirst != this )
		{
			m_pisNext = m_pisFirst ;
			if ( m_pisNext != NULL )
			{
				ESLAssert( m_pisNext->m_pisPrev == NULL ) ;
				m_pisNext->m_pisPrev = this ;
			}
			m_pisFirst = this ;
		}
	}
	else
	{
		ESLAssert( m_pisFirst != NULL ) ;
		#if	defined(_DEBUG)
		EInternetSession *	pisNext = m_pisFirst ;
		while ( pisNext != NULL )
		{
			if ( pisNext == this )
			{
				break ;
			}
			pisNext = pisNext->m_pisNext ;
		}
		ESLAssert( pisNext == this ) ;
		#endif
	}
	eslHeapUnlock( NULL ) ;
}

void EInternetSession::DetachChain( void )
{
	eslHeapLock( NULL ) ;
	if ( m_pisNext != NULL )
	{
		m_pisNext->m_pisPrev = m_pisPrev ;
	}
	if ( m_pisPrev != NULL )
	{
		m_pisPrev->m_pisNext = m_pisNext ;
	}
	else if ( m_pisFirst == this )
	{
		m_pisFirst = m_pisNext ;
	}
	m_pisPrev = NULL ;
	m_pisNext = NULL ;
	eslHeapUnlock( NULL ) ;
}

void EInternetSession::OnStatusCallback
	( DWORD_PTR dwContext, DWORD dwInternetStatus,
		LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
}


//////////////////////////////////////////////////////////////////////////////
// Internet File クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( EInternetFile, ESyncStreamFile, EGLSThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EInternetFile::EInternetFile( void )
{
	m_hConnect = NULL ;
	m_hFile = NULL ;
	m_hThreadReady = NULL ;
	m_pTempFile = NULL ;
	m_fNoDeleteTempFile = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EInternetFile::~EInternetFile( void )
{
	Close( ) ;
}

// 閉じる
//////////////////////////////////////////////////////////////////////////////
void EInternetFile::Close( void )
{
	CancelDownload( INFINITE ) ;
	ESyncStreamFile::Initialize( ) ;
	EGLSThread::CloseThread( ) ;
	//
	if ( m_hFile != NULL )
	{
		::InternetCloseHandle( m_hFile ) ;
		m_hFile = NULL ;
	}
	if ( m_hConnect != NULL )
	{
		::InternetCloseHandle( m_hConnect ) ;
		m_hConnect = NULL ;
	}
	if ( m_hThreadReady != NULL )
	{
		::CloseHandle( m_hThreadReady ) ;
		m_hThreadReady = NULL ;
	}
	if ( m_pTempFile != NULL )
	{
		if ( !m_fNoDeleteTempFile )
		{
			ERawFile *	pTempFile = ESLTypeCast<ERawFile>( m_pTempFile ) ;
			if ( pTempFile != NULL )
			{
				EString	strFilePath = pTempFile->GetFilePath( ) ;
				pTempFile->Seek( 0, ESLFileObject::FromBegin ) ;
				pTempFile->SetEndOfFile( ) ;
				delete	m_pTempFile ;
				::DeleteFile( strFilePath ) ;
			}
			else
			{
				delete	m_pTempFile ;
			}
		}
		else
		{
			delete	m_pTempFile ;
		}
		m_pTempFile = NULL ;
	}
}

// URL を開く
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetFile::OpenURL
	( EInternetSession & session, const char * pszURL,
		ESLFileObject * pTempFile,
		const char * pszHeaders, DWORD dwHeaderLength, DWORD dwFlags )
{
	Close( ) ;
	//
	m_hFile = ::InternetOpenUrl
		( session, pszURL,
			pszHeaders, dwHeaderLength, dwFlags, (DWORD_PTR) this ) ;
	if ( m_hFile == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwTempFileSize = -1 ;
	if ( pTempFile == NULL )
	{
		EString	strURL = pszURL ;
		if ( !strURL.CompareLeft( "http:" )
			|| !strURL.CompareLeft( "https://" ) )
		do
		{
			DWORD	dwBufLen = 0 ;
			if ( !::HttpQueryInfo
				( m_hFile, HTTP_QUERY_CONTENT_LENGTH, NULL, &dwBufLen, 0 ) )
			{
				break ;
			}
			EString	strLength ;
			if ( !::HttpQueryInfo
				( m_hFile, HTTP_QUERY_CONTENT_LENGTH,
					strLength.GetBuffer( dwBufLen ), &dwBufLen, 0 ) )
			{
				break ;
			}
			strLength.ReleaseBuffer( dwBufLen ) ;
			//
			EStreamWideString	swsLength = strLength ;
			INT64	nLen = swsLength.GetLargeInteger( ) ;
			if ( (nLen <= 0x7FFFFFFF) && (nLen >= 0) )
			{
				dwTempFileSize = (DWORD) nLen ;
			}
		}
		while ( false ) ;
		//
		m_pTempFile =
			session.CreateTemporaryFile( dwTempFileSize ) ;
		pTempFile = m_pTempFile ;
		m_fNoDeleteTempFile = false ;
	}
	else
	{
		m_pTempFile = pTempFile ;
		m_fNoDeleteTempFile = true ;
	}
	return	BeginDownload( dwTempFileSize, pTempFile ) ;
}

// 接続する
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetFile::Connect
	( EInternetSession & session,
		const char * pszServerName,
		DWORD dwService, INTERNET_PORT nServerPort, 
		const char * pszUserName, const char * pszPassword, DWORD dwFlags )
{
	Close( ) ;
	//
	m_hConnect = ::InternetConnect
		( session, pszServerName, nServerPort,
			pszUserName, pszPassword, dwService, 0, (DWORD_PTR) this ) ;
	if ( m_hConnect == NULL )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// ダウンロードの開始
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetFile::BeginDownload
	( unsigned long int nLength, ESLFileObject * pTempFile )
{
	if ( m_hFile == NULL )
	{
		return	eslErrGeneral ;
	}
	ESyncStreamFile::Initialize( nLength, pTempFile ) ;
	//
	if ( m_hThreadReady == NULL )
	{
		m_hThreadReady = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	}
	else
	{
		::ResetEvent( m_hThreadReady ) ;
	}
	ESLError	err = EGLSThread::BeginThread() ;
	if ( err )
	{
		return	err ;
	}
	::WaitForSingleObject( m_hThreadReady, INFINITE ) ;
	return	eslErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD EInternetFile::ThreadProc( void )
{
	MSG	msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	::SetEvent( m_hThreadReady ) ;
	//
	EStreamBuffer	buf ;
	const DWORD		dwBufSize = 0x100 ;
	void *			ptrBuf = buf.PutBuffer( dwBufSize ) ;
	for ( ; ; )
	{
		DWORD	dwReadBytes = 0 ;
		if ( !::InternetReadFile
			( m_hFile, ptrBuf, dwBufSize, &dwReadBytes ) )
		{
			break ;
		}
		ESyncStreamFile::Write( ptrBuf, dwReadBytes ) ;
		if ( dwReadBytes == 0 )
		{
			break ;
		}
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == tmQuit )
			{
				break ;
			}
		}
		if ( dwReadBytes < dwBufSize )
		{
			Sleep( 1 ) ;
		}
	}
	ESyncStreamFile::FinishStream( ) ;
	return	0 ;
}

// ダウンロードが終了するまで待つ
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetFile::WaitUntilDownload( DWORD dwTimeout, bool fDispMsg )
{
	HANDLE	hWaitEvent[1] ;
	hWaitEvent[0] = EGLSThread::Handle() ;
	//
	if ( fDispMsg )
	{
		DWORD	dwStartTime = ::GetCurrentTime( ) ;
		for ( ; ; )
		{
			DWORD	dwResult =
				::WaitForMultipleObjects( 1, hWaitEvent, FALSE, 10 ) ;
			if ( dwResult == WAIT_OBJECT_0 )	
			{
				return	eslErrSuccess ;
			}
			else if ( dwResult != WAIT_TIMEOUT )
			{
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < 0x10; i ++ )
			{
				if ( ::GetCurrentTime() - dwStartTime >= dwTimeout )
				{
					return	eslErrTimeout ;
				}
				MSG		msg ;
				if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					::TranslateMessage( &msg ) ;
					::DispatchMessage( &msg ) ;
				}
				else
				{
					break ;
				}
			}
		}
	}
	else
	{
		DWORD	dwResult =
			::WaitForSingleObject( EGLSThread::Handle(), dwTimeout ) ;
		if ( dwResult == WAIT_OBJECT_0 )	
		{
			return	eslErrSuccess ;
		}
		else if ( dwResult != WAIT_TIMEOUT )
		{
			return	eslErrGeneral ;
		}
		return	eslErrTimeout ;
	}
}

// ダウンロードをキャンセルする
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetFile::CancelDownload( DWORD dwTimeout, bool fDispMsg )
{
	if ( EGLSThread::Handle() == NULL )
	{
		return	eslErrGeneral ;
	}
	EGLSThread::PostThreadMessage( tmQuit, 0, 0 ) ;
	return	WaitUntilDownload( dwTimeout, fDispMsg ) ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP Internet File クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EInternetHttpFile, EInternetFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EInternetHttpFile::EInternetHttpFile( void )
{
}

// HTTP リクエスト送信
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::OpenRequest
	( const char * pszVerb,
		const char * pszObjectName, const char * pszReferer,
		LPCTSTR* ppstrAcceptTypes, const char * pszVersion, DWORD dwFlags )
{
	if ( (m_hFile != NULL) || (m_hConnect == NULL) )
	{
		return	eslErrGeneral ;
	}
	if ( pszVersion == NULL )
	{
		pszVersion = "HTTP/1.0" ;
	}
	m_hFile = ::HttpOpenRequest
		( m_hConnect, pszVerb, pszObjectName,
			pszVersion, pszReferer,
			ppstrAcceptTypes, dwFlags, (DWORD_PTR) this ) ;
	if ( m_hFile == NULL )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

ESLError EInternetHttpFile::SendRequest
	( const char * pszHeaders, DWORD dwHeadersLen,
		void * lpOptional, DWORD dwOptionalLen )
{
	if ( m_hFile == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( !::HttpSendRequest
		( m_hFile, pszHeaders, dwHeadersLen, lpOptional, dwOptionalLen ) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 情報取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryInfo
	( DWORD dwInfoLevel, void * ptrBuffer,
		DWORD * pdwBufferLength, DWORD * pdwIndex )
{
	ESLAssert( ((HTTP_QUERY_HEADER_MASK & dwInfoLevel)
						<= HTTP_QUERY_MAX) && (dwInfoLevel != 0) ) ;
	if ( m_hFile == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( !::HttpQueryInfo
		( m_hFile, dwInfoLevel, ptrBuffer, pdwBufferLength, pdwIndex ) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryStatusCode( DWORD & dwStatusCode )
{
	DWORD	dwBufLen = sizeof(DWORD) ;
	return	QueryInfo
		( HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
								&dwStatusCode, &dwBufLen ) ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryContentLength( DWORD & dwContentLength )
{
	DWORD	dwBufLen = sizeof(DWORD) ;
	return	QueryInfo
		( HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH,
								&dwContentLength, &dwBufLen ) ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryContentType( EString & strType )
{
	DWORD	dwBufLen = 0 ;
	ESLError	err =
		QueryInfo( HTTP_QUERY_CONTENT_TYPE, NULL, &dwBufLen, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = QueryInfo
		( HTTP_QUERY_CONTENT_TYPE,
			strType.GetBuffer( dwBufLen ), &dwBufLen, 0 ) ;
	if ( err )
	{
		strType.ReleaseBuffer( ) ;
		return	err ;
	}
	strType.ReleaseBuffer( dwBufLen ) ;
	strType.TrimRight( ) ;
	return	eslErrSuccess ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryContentTransferEncoding
	( EString & strEncoding )
{
	DWORD	dwBufLen = 0 ;
	ESLError	err =
		QueryInfo
			( HTTP_QUERY_CONTENT_TRANSFER_ENCODING, NULL, &dwBufLen, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = QueryInfo
		( HTTP_QUERY_CONTENT_TRANSFER_ENCODING,
			strEncoding.GetBuffer( dwBufLen ), &dwBufLen, 0 ) ;
	if ( err )
	{
		strEncoding.ReleaseBuffer( ) ;
		return	err ;
	}
	strEncoding.ReleaseBuffer( dwBufLen ) ;
	strEncoding.TrimRight( ) ;
	return	eslErrSuccess ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryContentDate( SYSTEMTIME & systime )
{
	DWORD	dwBufLen = sizeof(SYSTEMTIME) ;
	return	QueryInfo
		( HTTP_QUERY_FLAG_SYSTEMTIME | HTTP_QUERY_DATE,
									&systime, &dwBufLen ) ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
ESLError EInternetHttpFile::QueryContentLastModified( SYSTEMTIME & systime )
{
	DWORD	dwBufLen = sizeof(SYSTEMTIME) ;
	return	QueryInfo
		( HTTP_QUERY_FLAG_SYSTEMTIME | HTTP_QUERY_LAST_MODIFIED,
											&systime, &dwBufLen ) ;
}

