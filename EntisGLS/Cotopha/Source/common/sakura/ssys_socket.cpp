
#include <sakura/sakura.h>
#include <sakura/ssys_socket.h>

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ソケット
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSocket, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSocket::SSocket( void )
{
#if	defined(__COTOPHA__)
	m_socket = NULL ;
#else
	m_socket = INVALID_SOCKET ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSocket::~SSocket( void )
{
	SSocket::Close() ;
}

// ソケット作成
//////////////////////////////////////////////////////////////////////////////
SError SSocket::Create
	( uint32_t nPort, int64_t nFlags, const wchar_t * pwszAddress )
{
#if	defined(__COTOPHA__)
	if ( m_socket != NULL )
	{
		Close() ;
	}
	m_socket = new Socket ;
	return	m_socket->Create( nPort, nFlags, pwszAddress ) ;

#else
	if ( m_socket != INVALID_SOCKET )
	{
		Close() ;
	}
	//
	// 生成
	//
	int	typeSocket = SOCK_STREAM ;
	if ( (nFlags & typeMask) == typeDatagram )
	{
		typeSocket = SOCK_DGRAM ;
	}
	m_socket = socket( PF_INET, typeSocket, 0 ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	//
	// バインド
	//
	sockaddr_in	sockadin ;
	memset( &sockadin, 0, sizeof(sockaddr_in) ) ;
	sockadin.sin_family = AF_INET ;
	if ( pwszAddress == NULL )
	{
		sockadin.sin_addr.s_addr = htonl(INADDR_ANY) ;
	}
	else
	{
		SString			strAddr = pwszAddress ;
		SArray<char>	bufAddr ;
		in_addr_t	addr = inet_addr( strAddr.EncodeDefaultTo(bufAddr) ) ;
		if ( addr == INADDR_NONE )
		{
			return	errFailed ;
		}
		sockadin.sin_addr.s_addr = addr ;
	}
	sockadin.sin_port = htons( (uint16_t) nPort ) ;
	if ( bind( m_socket,
			(sockaddr*) &sockadin, sizeof(sockaddr_in) ) == SOCKET_ERROR )
	{
		return	errFailed ;
	}
	return	errSuccess ;

#endif
}

// ソケットを閉じる
//////////////////////////////////////////////////////////////////////////////
void SSocket::Close( void )
{
#if	defined(__COTOPHA__)
	if ( m_socket != NULL )
	{
		m_socket->Close() ;
		delete	m_socket ;
		m_socket = NULL ;
	}

#else
	if ( m_socket != INVALID_SOCKET )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			closesocket( m_socket ) ;
		#else
			close( m_socket ) ;
		#endif
		m_socket = INVALID_SOCKET ;
	}
#endif
}

// ソケット接続
//////////////////////////////////////////////////////////////////////////////
SError SSocket::Connect
	( const wchar_t * pwszHostAddress, uint32_t nHostPort )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	errFailed ;
	}
	return	m_socket->Connect( pwszHostAddress, nHostPort ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	SString			strAddr = pwszHostAddress ;
	SArray<char>	bufAddr ;
	const char *	pszHostAddress = strAddr.EncodeDefaultTo(bufAddr) ;
	sockaddr_in	sockadin ;
	memset( &sockadin, 0, sizeof(sockaddr_in) ) ;
	sockadin.sin_family = AF_INET ;
	sockadin.sin_addr.s_addr = inet_addr( pszHostAddress ) ;
	if ( sockadin.sin_addr.s_addr == INADDR_NONE )
	{
		hostent*	lphost ;
		lphost = gethostbyname( pszHostAddress ) ;
		if ( lphost != NULL )
		{
			sockadin.sin_addr.s_addr = ((in_addr*)lphost->h_addr)->s_addr ;
		}
		else
		{
			return	errFailed ;
		}
	}
	sockadin.sin_port = htons( (uint16_t) nHostPort ) ;
	//
	if ( connect( m_socket,
			(sockaddr*) &sockadin, sizeof(sockaddr_in) ) == SOCKET_ERROR )
	{
		return	errTimeout ;
	}
	return	errSuccess ;

#endif
}

// 接続要求を待つ
//////////////////////////////////////////////////////////////////////////////
SError SSocket::Listen( int nConnectionBacklog )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	errFailed ;
	}
	return	m_socket->Listen( nConnectionBacklog ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	if ( listen( m_socket, nConnectionBacklog ) == SOCKET_ERROR )
	{
		return	errTimeout ;
	}
	return	errSuccess ;

#endif
}

// 接続受け入れ
//////////////////////////////////////////////////////////////////////////////
SError SSocket::Accept( SSocket & socket )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	errFailed ;
	}
	if ( socket.m_socket == NULL )
	{
		socket.m_socket = new Socket ;
	}
	SError	err = m_socket->Accept( socket.m_socket ) ;
	if ( err == errSuccess )
	{
		m_socket->GetAcceptedCleintIP( m_strAcceptClient ) ;
		socket.OnAccepted() ;
	}
	return	err ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	socket.Close() ;
	//
	sockaddr_in	client ;
	socklen_t	addrlen = sizeof(sockaddr_in) ;
	socket.m_socket = accept( m_socket, (sockaddr*) &client, &addrlen ) ;
	if ( socket.m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	m_strAcceptClient = inet_ntoa( client.sin_addr ) ;
	socket.OnAccepted() ;
	return	errSuccess ;

#endif
}

// 状態ポーリング
//////////////////////////////////////////////////////////////////////////////
int64_t SSocket::Poll( int64_t nFlags, int64_t msecTimeout )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	errFailed ;
	}
	return	m_socket->Poll( nFlags, msecTimeout ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	pollError ;
	}
	int64_t	nState = 0 ;
	for ( ; ; )
	{
		timeval		tv ;
		fd_set		fdsRead, fdsWrite ;
		fd_set *	pfdsRead = NULL ;
		fd_set *	pfdsWrite = NULL ;
		tv.tv_sec = 0 ;
		if ( msecTimeout == Synchronism::Infinite )
		{
			tv.tv_usec = 10000 ;
		}
		else
		{
			tv.tv_usec = (long) msecTimeout * 1000 ;
		}
		if ( (nFlags == 0) || (nFlags & pollIn) )
		{
			FD_ZERO( &fdsRead ) ;
			FD_SET( m_socket, &fdsRead ) ;
			pfdsRead = &fdsRead ;
		}
		if ( (nFlags == 0) || (nFlags & pollOut) )
		{
			FD_ZERO( &fdsWrite ) ;
			FD_SET( m_socket, &fdsWrite ) ;
			pfdsWrite = &fdsWrite ;
		}
		SOCKET	r = select( (int) m_socket + 1, pfdsRead, pfdsWrite, NULL, &tv ) ;
		if ( r == -1 )
		{
			return	pollHangup ;
		}
		if ( (pfdsRead != NULL) && FD_ISSET( m_socket, &fdsRead ) )
		{
			nState |= pollIn ;
		}
		if ( (pfdsWrite != NULL) && FD_ISSET( m_socket, &fdsWrite ) )
		{
			nState |= pollOut ;
		}
		if ( (nState != 0)
			|| (msecTimeout != Synchronism::Infinite) )
		{
			break ;
		}
	}
	return	nState ;

#endif
}

// 受信
//////////////////////////////////////////////////////////////////////////////
size_t SSocket::Receive( void * ptrBuf, size_t nBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->Read( ptrBuf, nBytes ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	0 ;
	}
	ssize_t	nRecvLen = recv( m_socket, (char*) ptrBuf, (int) nBytes, 0 ) ;
	if ( nRecvLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nRecvLen ;

#endif
}

size_t SSocket::ReceiveFrom
	( void * ptrBuf, size_t nBytes,
		void * ptrAddrFrom, size_t& nAddrBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->ReceiveFrom
				( ptrBuf, nBytes, ptrAddrFrom, nAddrBytes ) ;
#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	socklen_t	nFromLen = (socklen_t) nAddrBytes ;
	ssize_t		nRecvLen =
		recvfrom( m_socket, (char*) ptrBuf, (int) nBytes, 0,
					(sockaddr*) ptrAddrFrom, &nFromLen ) ;
	nAddrBytes = (size_t) nFromLen ;
	if ( nRecvLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nRecvLen ;

#endif
}

// 送信
//////////////////////////////////////////////////////////////////////////////
size_t SSocket::Send( const void * ptrBuf, size_t nBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->Write( ptrBuf, nBytes ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	0 ;
	}
	ssize_t	nSentLen = send( m_socket, (const char*) ptrBuf, (int) nBytes, 0 ) ;
	if ( nSentLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nSentLen ;

#endif
}

size_t SSocket::SendTo
	( const void * ptrBuf, size_t nBytes,
			void * ptrAddrTo, size_t nAddrBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->SendTo( ptrBuf, nBytes, ptrAddrTo, nAddrBytes ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	ssize_t	nSentLen =
		sendto( m_socket, (const char*) ptrBuf, (int) nBytes, 0,
					(sockaddr*) ptrAddrTo, (socklen_t) nAddrBytes ) ;
	if ( nSentLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nSentLen ;

#endif
}

// Accept で接続したクライアントアドレスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SSocket::GetAcceptedCleintIP( SString& strAddress ) const
{
	strAddress = m_strAcceptClient ;
	if ( m_strAcceptClient.IsEmpty() )
	{
		return	errFailed ;
	}
	else
	{
		return	errSuccess ;
	}
}

// このマシンのIPアドレス(localhostでない)を取得する
//////////////////////////////////////////////////////////////////////////////
SError SSocket::GetLocalMachineIP( SString& strAddress )
{
#if	defined(__COTOPHA__)
	return	Socket::GetLocalMachineIP( strAddress ) ;

#else
	const char *	pszDummyAddr = "192.0.2.1" ;
	sockaddr_in		sockadin ;
	memset( &sockadin, 0, sizeof(sockaddr_in) ) ;
    sockadin.sin_family = AF_INET ;
	sockadin.sin_addr.s_addr = inet_addr( pszDummyAddr ) ;
	if ( sockadin.sin_addr.s_addr == INADDR_NONE )
	{
		hostent*	lphost ;
		lphost = gethostbyname( pszDummyAddr ) ;
		if ( lphost != NULL )
		{
			sockadin.sin_addr.s_addr = ((in_addr*)lphost->h_addr)->s_addr ;
		}
		else
		{
			return	errFailed ;
		}
	}
	SOCKET	s ;
	s = socket( PF_INET, SOCK_DGRAM, 0 ) ;
	if ( s == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	SError	err = errFailed ;
	do
	{
		if ( connect( s,
				(sockaddr*) &sockadin, sizeof(sockaddr_in) ) == SOCKET_ERROR )
		{
			break ;
		}
		sockaddr_in	addr ;
		socklen_t	addrlen = sizeof(sockaddr_in) ;
		memset( &addr, 0, sizeof(sockaddr_in) ) ;
		if ( getsockname( s, (struct sockaddr*)&addr, &addrlen ) < 0 )
		{
			break ;
		}
		strAddress = inet_ntoa( addr.sin_addr ) ;
		if ( !strAddress.IsEmpty() )
		{
			err = errSuccess ;
		}
	}
	while ( false ) ;
	#if	defined(__PLATFORM_WINDOWS__)
		closesocket( s ) ;
	#else
		close( s ) ;
	#endif
	return	err ;
#endif
}

// Accept で接続された socket に対して呼び出される
//////////////////////////////////////////////////////////////////////////////
void SSocket::OnAccepted( void )
{
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SSocket::GetFileObject( void )
{
	return	m_socket ;
}
#endif

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SSocket::Duplicate( void ) const
{
	return	new SSocket ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSocket::Read( void * ptrBuf, size_t nBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->Read( ptrBuf, nBytes ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	0 ;
	}
	ssize_t	nRecvLen = recv( m_socket, (char*) ptrBuf, (int) nBytes, 0 ) ;
	if ( nRecvLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nRecvLen ;

#endif
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSocket::Write( const void * ptrBuf, size_t nBytes )
{
#if	defined(__COTOPHA__)
	ESLAssert( m_socket != NULL ) ;
	if ( m_socket == NULL )
	{
		return	0 ;
	}
	return	m_socket->Write( ptrBuf, nBytes ) ;

#else
	ESLAssert( m_socket != INVALID_SOCKET ) ;
	if ( m_socket == INVALID_SOCKET )
	{
		return	errFailed ;
	}
	ssize_t	nSentLen = send( m_socket, (const char*) ptrBuf, (int) nBytes, 0 ) ;
	if ( nSentLen == SOCKET_ERROR )
	{
		return	0 ;
	}
	return	nSentLen ;

#endif
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SSocket::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSocket::GetLength( void ) const
{
	return	-1 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SSocket::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	-1 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSocket::GetPosition( void ) const
{
	return	-1 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SSocket::SetEndOfFile( void )
{
	return	errFailed ;
}



//////////////////////////////////////////////////////////////////////////////
// 同期ソケット
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSyncSocket, SSocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSyncSocket::SSyncSocket( void )
{
	m_flagClosed = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSyncSocket::~SSyncSocket( void )
{
}

// 状態ポーリング
//////////////////////////////////////////////////////////////////////////////
int64_t SSyncSocket::Poll
	( int64_t nFlags, int64_t msecTimeout )
{
	int64_t	nState = 0 ;
	if ( (nFlags & pollIn) && (m_qbufRecv.GetLength() != 0) )
	{
		nFlags &= ~pollIn ;
		nState |= pollIn ;
	}
	nState |= SSocket::Poll( nFlags, msecTimeout ) ;
	return	nState ;
}

// 受信
//////////////////////////////////////////////////////////////////////////////
size_t SSyncSocket::Receive( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	m_csSync.Lock() ;
	if ( m_qbufRecv.GetLength() >= 0 )
	{
		nReadBytes =
			m_qbufRecv.Read( ptrBuf, (size_t) m_qbufRecv.GetLength() ) ;
		ptrBuf = ((uint8_t*)ptrBuf) + nReadBytes ;
		ESLAssert( nBytes >= nReadBytes ) ;
		if ( nBytes >= nReadBytes )
		{
			nBytes -= nReadBytes ;
		}
		else
		{
			nBytes = 0 ;
		}
	}
	m_csSync.Unlock() ;
	//
	if ( nBytes != 0 )
	{
		nReadBytes += SSocket::Receive( ptrBuf, nBytes ) ;
	}
	return	nReadBytes ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SSyncSocket::Duplicate( void ) const
{
	return	new SSyncSocket ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSyncSocket::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	m_csSync.Lock() ;
	if ( m_qbufRecv.GetLength() >= (int64_t) nBytes )
	{
		nReadBytes = m_qbufRecv.Read( ptrBuf, nBytes ) ;
		m_csSync.Unlock() ;
	}
	else
	{
		if ( m_qbufRecv.GetLength() >= 0 )
		{
			nReadBytes =
				m_qbufRecv.Read( ptrBuf, (size_t) m_qbufRecv.GetLength() ) ;
			ptrBuf = ((uint8_t*)ptrBuf) + nReadBytes ;
			ESLAssert( nBytes >= nReadBytes ) ;
			if ( nBytes >= nReadBytes )
			{
				nBytes -= nReadBytes ;
			}
			else
			{
				nBytes = 0 ;
			}
		}
		m_csSync.Unlock() ;
		//
		while ( nBytes != 0 )
		{
			size_t	nRecvBytes = SSocket::Read( ptrBuf, nBytes ) ;
			nReadBytes += nRecvBytes ;
			ptrBuf = ((uint8_t*)ptrBuf) + nRecvBytes ;
			if ( nRecvBytes == 0 )
			{
				m_flagClosed = true ;
				break ;
			}
			ESLAssert( nBytes >= nRecvBytes ) ;
			if ( nBytes >= nRecvBytes )
			{
				nBytes -= nRecvBytes ;
			}
			else
			{
				break ;
			}
			if ( SSocket::Poll
				( pollIn | pollHangup | pollError, 100 )
									& (pollHangup | pollError) )
			{
				break ;
			}
		}
	}
	return	nReadBytes ;
}

size_t SSyncSocket::ReadLine( SArray<uint8_t>& buf )
{
	buf.SetLength( 0 ) ;
	for ( ; ; )
	{
		size_t	iOffset = buf.GetLength() ;
		buf.SetLength( iOffset + 0x100 ) ;
		//
		size_t	nRecvBytes =
			ReadLine( buf.GetArray() + iOffset, 0x100 ) ;
		buf.FinishArray() ;
		buf.SetLength( iOffset + nRecvBytes ) ;
		if ( nRecvBytes == 0 )
		{
			break ;
		}
		if ( buf.At( buf.GetLength() - 1 ) == L'\n' )
		{
			break ;
		}
	}
	return	buf.GetLength() ;
}

// 改行コード (\n) まで読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSyncSocket::ReadLine( uint8_t * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	while ( nBytes != 0 )
	{
		m_csSync.Lock() ;
		size_t	nGetBytes = 0x100 ;
		const uint8_t *	pbytBuf = m_qbufRecv.GetBuffer( nGetBytes ) ;
		for ( size_t i = 0; (i < nGetBytes) & (i < nBytes); i ++ )
		{
			if ( (ptrBuf[i] = pbytBuf[i]) == '\n' )
			{
				nGetBytes = i + 1 ;
				break ;
			}
		}
		m_qbufRecv.ReleaseBuffer( (ssize_t) nGetBytes ) ;
		nReadBytes += nGetBytes ;
		ptrBuf += nGetBytes ;
		if ( (nGetBytes != 0) && (ptrBuf[-1] == '\n') )
		{
			m_csSync.Unlock() ;
			break ;
		}
		m_csSync.Unlock() ;
		//
		if ( SSocket::Poll
			( pollIn | pollHangup | pollError, 100 )
								& (pollHangup | pollError) )
		{
			break ;
		}
		uint8_t	buf[0x100] ;
		size_t	nRecvBytes = SSocket::Read( buf, 0x100 ) ;
		if ( nRecvBytes == 0 )
		{
			m_flagClosed = true ;
			break ;
		}
		m_csSync.Lock() ;
		m_qbufRecv.Write( buf, nRecvBytes ) ;
		m_csSync.Unlock() ;
	}
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSyncSocket::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_flagClosed )
	{
		return	0 ;
	}
	size_t	nWrittenBytes = 0 ;
	while ( nBytes != 0 )
	{
		size_t	nSentBytes = SSocket::Write( ptrBuf, nBytes ) ;
		nWrittenBytes += nSentBytes ;
		ptrBuf = ((uint8_t*)ptrBuf) + nSentBytes ;
		if ( nBytes >= nSentBytes )
		{
			nBytes -= nSentBytes ;
		}
		else
		{
			break ;
		}
		if ( SSocket::Poll
			( pollOut | pollHangup | pollError, 100 )
								& (pollHangup | pollError) )
		{
			break ;
		}
	}
	return	nWrittenBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// 非同期ソケットリスナインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SAsyncSocketListener )

// データを受信可能
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocketListener::OnReceive( SAsyncSocket& socket )
{
}

// 全てのデータを送信完了
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocketListener::OnSent( SAsyncSocket& socket )
{
}

// 切断された
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocketListener::OnClose( SAsyncSocket& socket )
{
}



//////////////////////////////////////////////////////////////////////////////
// 非同期ソケット
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SAsyncSocket, SSocket )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SAsyncSocket::SAsyncSocket( void )
{
	m_pListener = NULL ;
	m_flagThread = false ;
	m_flagExitThread = false ;
	m_limitRecv = 0x100000 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SAsyncSocket::~SAsyncSocket( void )
{
	SAsyncSocket::Close() ;
}

// ソケットを閉じる
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::Close( void )
{
	if ( m_flagThread )
	{
		m_flagExitThread = true ;
		m_doneThread.Wait() ;
		m_doneThread.Delete() ;
		m_signalWake.Delete() ;
		m_signalAnyRecv.Delete() ;
		m_signalAllSent.Delete() ;
		m_flagThread = false ;
	}
	m_qbufRecv.ClearAll() ;
	m_qbufSend.ClearAll() ;
	//
	SSocket::Close() ;
}

// ソケット接続
//////////////////////////////////////////////////////////////////////////////
SError SAsyncSocket::Connect
	( const wchar_t * pwszHostAddress, uint32_t nHostPort )
{
	SError	err = SSocket::Connect( pwszHostAddress, nHostPort ) ;
	if ( !err )
	{
		BeginRecvThread() ;
	}
	return	err ;
}

// 受信
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::Receive( void * ptrBuf, size_t nBytes )
{
	return	SAsyncSocket::Read( ptrBuf, nBytes ) ;
}

// 送信
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::Send( const void * ptrBuf, size_t nBytes )
{
	return	SAsyncSocket::Write( ptrBuf, nBytes ) ;
}

// Accept で接続された socket に対して呼び出される
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::OnAccepted( void )
{
	BeginRecvThread() ;
}

// 受信スレッドを起動する
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::BeginRecvThread( void )
{
	if ( !m_flagThread )
	{
		m_doneThread.Initialize( false ) ;
		m_signalWake.Initialize( false ) ;
		m_signalAnyRecv.Initialize( false ) ;
		m_signalAllSent.Initialize( true ) ;
		m_qbufRecv.ClearAll() ;
		m_qbufSend.ClearAll() ;
		m_flagExitThread = false ;
		//
		if ( SThread::BeginStockThread
				( &AsyncSocketThreadProc, this ) != NULL )
		{
			m_flagThread = true ;
		}
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::AsyncSocketThreadProc( void * pInstance )
{
	((SAsyncSocket*)pInstance)->AsyncSocketProc() ;
}

void SAsyncSocket::AsyncSocketProc( void )
{
	SArray<uint8_t>	bufRecv ;
	const size_t	nRecvBufBytes = 0x1000 ;
	uint8_t *		pBufRecv = bufRecv.GetArray( nRecvBufBytes ) ;
	//
	while ( !m_flagExitThread )
	{
		int64_t nFlags = pollIn ;
		m_csSync.Lock() ;
		if ( m_qbufSend.GetLength() != 0 )
		{
			nFlags |= pollOut ;
		}
		m_csSync.Unlock() ;
		//
		int64_t	nState = SSocket::Poll( nFlags, 5 ) ;
		if ( nState & (pollHangup | pollError) )
		{
			ESLTrace( "error at Poll socket\n" ) ;
			break ;
		}
		int	msecWait = 1 ;
		if ( nState & pollIn )
		{
			m_csSync.Lock() ;
			if ( m_qbufRecv.GetLength() >= (int64_t) m_limitRecv )
			{
				// 大量の受信データが溜まっている場合にはスルー
				m_csSync.Unlock() ;
				msecWait = 10 ;
			}
			else
			{
				m_csSync.Unlock() ;
				//
				// 4KB 受信
				size_t	nRecv = SSocket::Read( pBufRecv, nRecvBufBytes ) ;
				if ( nRecv != 0 )
				{
					m_csSync.Lock() ;
					m_qbufRecv.Write( pBufRecv, nRecv ) ;
					m_signalAnyRecv.SetSignal() ;
					m_csSync.Unlock() ;
					//
					if ( m_pListener != NULL )
					{
						m_pListener->OnReceive( *this ) ;
					}
					msecWait = 0 ;
				}
				else
				{
					break ;
				}
			}
		}
		if ( nState & pollOut )
		{
			size_t			nBytes = 0x400 ;
			const uint8_t *	ptrBuf ;
			bool			flagSentAll = false ;
			m_csSync.Lock() ;
			ptrBuf = m_qbufSend.GetBuffer( nBytes ) ;
			if ( nBytes != 0 )
			{
				// データ送信
				size_t	nSent = SSocket::Write( ptrBuf, nBytes ) ;
				m_qbufSend.ReleaseBuffer( (ssize_t) nSent ) ;
				if ( m_qbufSend.GetLength() == 0 )
				{
					m_signalAllSent.SetSignal() ;
					flagSentAll = true ;
				}
				msecWait = 0 ;
			}
			else
			{
				m_qbufSend.ReleaseBuffer( 0 ) ;
			}
			m_csSync.Unlock() ;
			//
			if ( flagSentAll && (m_pListener != NULL) )
			{
				m_pListener->OnSent( *this ) ;
			}
		}
		if ( msecWait != 0 )
		{
			m_signalWake.Wait( msecWait ) ;
			m_signalWake.ResetSignal() ;
		}
	}
	bufRecv.FinishArray() ;
	//
	if ( m_pListener != NULL )
	{
		m_pListener->OnClose( *this ) ;
	}
	m_signalAllSent.SetSignal() ;
	m_signalAnyRecv.SetSignal() ;
	m_doneThread.SetSignal() ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SAsyncSocket::Duplicate( void ) const
{
	return	new SAsyncSocket ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes ;
	m_csSync.Lock() ;
	nReadBytes = m_qbufRecv.Read( ptrBuf, nBytes ) ;
	if ( m_qbufRecv.GetLength() == 0 )
	{
		m_signalAnyRecv.ResetSignal() ;
	}
	m_signalWake.SetSignal() ;
	m_csSync.Unlock() ;
	return	nReadBytes ;
}

// 改行コード (\n) まで読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::ReadLine( uint8_t * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	m_csSync.Lock() ;
	size_t	nLocked = nBytes ;
	const uint8_t *	pbytBuf = m_qbufRecv.GetBuffer( nLocked ) ;
	if ( pbytBuf != NULL )
	{
		nReadBytes = nLocked ;
		for ( size_t i = 0; i < nLocked; i ++ )
		{
			if ( (ptrBuf[i] = pbytBuf[i]) == '\n' )
			{
				nReadBytes = i + 1 ;
				break ;
			}
		}
		m_qbufRecv.ReleaseBuffer( (ssize_t) nReadBytes ) ;
	}
	if ( m_qbufRecv.GetLength() == 0 )
	{
		m_signalAnyRecv.ResetSignal() ;
	}
	m_signalWake.SetSignal() ;
	m_csSync.Unlock() ;
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::Write( const void * ptrBuf, size_t nBytes )
{
	m_csSync.Lock() ;
	nBytes = m_qbufSend.Write( ptrBuf, nBytes ) ;
	if ( m_qbufSend.GetLength() != 0 )
	{
		m_signalAllSent.ResetSignal() ;
	}
	m_signalWake.SetSignal() ;
	m_csSync.Unlock() ;
	return	nBytes ;
}

// 非同期リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::AttachListener( SAsyncSocketListener* pListener )
{
	m_pListener = pListener ;
}

// 受信バッファの最大サイズを設定する
//////////////////////////////////////////////////////////////////////////////
void SAsyncSocket::SetReceiveLimit( size_t nBytes )
{
	if ( nBytes < 0x100 )
	{
		nBytes = 0x100 ;
	}
	m_limitRecv = nBytes ;
}

// 未送信データバイト数を取得する
//////////////////////////////////////////////////////////////////////////////
size_t SAsyncSocket::GetNotSentDataBytes( void ) const
{
	size_t	nBytes ;
	m_csSync.Lock() ;
	nBytes = (size_t) m_qbufSend.GetLength() ;
	m_csSync.Unlock() ;
	return	nBytes ;
}

// 受信データを待機する
//////////////////////////////////////////////////////////////////////////////
SError SAsyncSocket::WaitToReceive( int64_t msecTimeout )
{
	return	m_signalAnyRecv.Wait( msecTimeout ) ;
}

// 全ての送信データを送信し終えるまで待機する
//////////////////////////////////////////////////////////////////////////////
SError SAsyncSocket::WaitToSendAll( int64_t msecTimeout )
{
	return	m_signalAllSent.Wait( msecTimeout ) ;
}

// ソケットが切断されるまで待機する
//////////////////////////////////////////////////////////////////////////////
SError SAsyncSocket::WaitToClose( int64_t msecTimeout )
{
	if ( m_flagThread )
	{
		return	m_doneThread.Wait( msecTimeout ) ;
	}
	return	errFailed ;
}


