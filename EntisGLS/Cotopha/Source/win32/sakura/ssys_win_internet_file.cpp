
#include <sakura/sakura.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_http_file.h>
#include <sakura/ssys_win_internet_file.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// インターネットセッション
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( SInternetSession *	SInternetSession::m_pisDefault = NULL ) ;
ESL_DLL_DECL( SInternetSession *	SInternetSession::m_pisFirst = NULL ) ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SInternetSession, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SInternetSession::SInternetSession( void )
{
	m_hInternet = NULL ;
	m_pisPrev = NULL ;
	m_pisNext = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SInternetSession::~SInternetSession( void )
{
	SInternetSession::Close( ) ;
}

// セッションを開く
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::Open
	( const wchar_t * pwszAgent, DWORD dwAccessType,
		const wchar_t * pwszProxyName,
		const wchar_t * pwszProxyBypass, DWORD dwFlags )
{
	Close( ) ;
	//
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		m_hInternet = ::InternetOpenW
			( pwszAgent, dwAccessType,
				pwszProxyName, pwszProxyBypass, dwFlags ) ;
	}
	else
	{
		SArray<char>	bufAgent, bufProxyName, bufProxyBypass ;
		SString			strAgent, strProxyName, strProxyBypass ;
		const char *	pszAgent = NULL ;
		const char *	pszProxyName = NULL ;
		const char *	pszProxyBypass = NULL ;
		if ( pwszAgent != NULL )
		{
			strAgent = pszAgent ;
			pszAgent = strAgent.EncodeDefaultTo( bufAgent ) ;
		}
		if ( pwszProxyName != NULL )
		{
			strProxyName = pwszProxyName ;
			pszProxyName = strProxyName.EncodeDefaultTo( bufProxyName ) ;
		}
		if ( pwszProxyBypass != NULL )
		{
			strProxyBypass = pwszProxyBypass ;
			pszProxyBypass = strProxyBypass.EncodeDefaultTo( bufProxyBypass ) ;
		}
		m_hInternet = ::InternetOpenA
			( pszAgent, dwAccessType,
				pszProxyName, pszProxyBypass, dwFlags ) ;
	}
	if ( m_hInternet == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// セッションを閉じる
//////////////////////////////////////////////////////////////////////////////
void SInternetSession::Close( void )
{
	if ( m_hInternet != NULL )
	{
		::InternetCloseHandle( m_hInternet ) ;
		m_hInternet = NULL ;
		//
		DetachChain( ) ;
	}
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::EnableCallback( bool fCallback )
{
	if ( m_hInternet == NULL )
	{
		return	errFailed ;
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
			return	errFailed ;
		}
	}
	else
	{
		::InternetSetStatusCallback( m_hInternet, NULL ) ;
		DetachChain( ) ;
	}
	return	errSuccess ;
}

// オプション取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::QueryOption
	( DWORD dwOption, void * pBuffer, DWORD * pdwBufLen )
{
	ESLAssert( (dwOption >= INTERNET_FIRST_OPTION) &&
					(dwOption <= INTERNET_LAST_OPTION) ) ;
	if ( ::InternetQueryOption
		( m_hInternet, dwOption, pBuffer, pdwBufLen ) )
	{
		return	errSuccess ;
	}
	return	errFailed ;
}

// オプション設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::SetOption
	( DWORD dwOption, void * pBuffer,
		DWORD dwBufferLength, DWORD dwFlags )
{
	ESLAssert( (dwOption >= INTERNET_FIRST_OPTION) &&
					(dwOption <= INTERNET_LAST_OPTION) ) ;
	if ( ::InternetSetOptionEx
		( m_hInternet, dwOption, pBuffer, dwBufferLength, dwFlags ) )
	{
		return	errSuccess ;
	}
	return	errFailed ;
}

// クッキー設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::SetCookie
	( const wchar_t * pwszURL,
		const wchar_t * pwszCookieName, const wchar_t * pwszCookieData )
{
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		if ( ::InternetSetCookieW( pwszURL, pwszCookieName, pwszCookieData ) )
		{
			return	errSuccess ;
		}
	}
	else
	{
		SArray<char>	bufURL, bufCookieName, bufCookieData ;
		SString			strURL = pwszURL ;
		SString			strCookieName = pwszCookieName ;
		SString			strCookieData = pwszCookieData ;
		if ( ::InternetSetCookieA
			( strURL.EncodeDefaultTo(bufURL),
				strCookieName.EncodeDefaultTo(bufCookieName),
				strCookieData.EncodeDefaultTo(bufCookieData) ) )
		{
			return	errSuccess ;
		}
	}
	return	errFailed ;
}

// クッキー取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetSession::GetCookie
	( const wchar_t * pwszURL,
		const wchar_t * pwszCookieName, SString & strCookieData )
{
	DWORD	dwBufLen = 0 ;
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		if ( !::InternetGetCookieW
			( pwszURL, pwszCookieName, NULL, &dwBufLen ) )
		{
			return	errFailed ;
		}
		ESLAssert( sizeof(uint16_t) == sizeof(WCHAR) ) ;
		if ( !::InternetGetCookieW
			( pwszURL, pwszCookieName,
				(LPWSTR) strCookieData.LockBuffer( dwBufLen ), &dwBufLen ) )
		{
			strCookieData.FreeArray() ;
			return	errFailed ;
		}
		strCookieData.UnlockBuffer( dwBufLen ) ;
	}
	else
	{
		SArray<char>	bufURL, bufCookieName, bufCookieData ;
		SString			strURL = pwszURL ;
		SString			strCookieName = pwszCookieName ;
		const char *	pszURL = strURL.EncodeDefaultTo( bufURL ) ;
		const char *	pszCookieName = strCookieName.EncodeDefaultTo( bufCookieName ) ;
		//
		if ( !::InternetGetCookieA
			( pszURL, pszCookieName, NULL, &dwBufLen ) )
		{
			return	errFailed ;
		}
		bufCookieData.SetLength( dwBufLen + 1 ) ;
		//
		if ( !::InternetGetCookieA
			( pszURL, pszCookieName,
				bufCookieData.GetArray(), &dwBufLen ) )
		{
			bufCookieData.FinishArray() ;
			return	errFailed ;
		}
		bufCookieData.FinishArray() ;
		strCookieData.SetString( bufCookieData.GetConstArray(), dwBufLen ) ;
	}
	return	errSuccess ;
}

// デフォルトセッション取得
//////////////////////////////////////////////////////////////////////////////
SInternetSession * SInternetSession::GetInstance( void )
{
	return	m_pisDefault ;
}

// デフォルトセッション設定
//////////////////////////////////////////////////////////////////////////////
void SInternetSession::AttachInstance( SInternetSession * pisDef )
{
	m_pisDefault = pisDef ;
}

// コールバック用チェーン
//////////////////////////////////////////////////////////////////////////////
void CALLBACK SInternetSession::InternetStatusCallback
	( HINTERNET hInternet, DWORD_PTR dwContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
	SInternetSession *	pisTarget = NULL ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	SInternetSession *	pisNext = m_pisFirst ;
	while ( pisNext != NULL )
	{
		if ( pisNext->m_hInternet == hInternet )
		{
			pisTarget = pisNext ;
			break ;
		}
		pisNext = pisNext->m_pisNext ;
	}
	SSystem::Unlock() ;
	//
	if ( pisTarget != NULL )
	{
		pisTarget->OnStatusCallback
			( dwContext, dwInternetStatus,
				lpvStatusInformation, dwStatusInformationLength ) ;
	}
}

void SInternetSession::AddChain( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
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
		SInternetSession *	pisNext = m_pisFirst ;
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
	SSystem::Unlock() ;
}

void SInternetSession::DetachChain( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
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
	SSystem::Unlock() ;
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
void SInternetSession::OnStatusCallback
	( DWORD_PTR dwContext, DWORD dwInternetStatus,
		LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
}


//////////////////////////////////////////////////////////////////////////////
// インターネットファイル (HTTP, HTTPS, FTP)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SInternetFile, SHttpFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SInternetFile::SInternetFile( void )
{
	m_pSession = NULL ;
	m_hConnect = NULL ;
	m_hFile = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SInternetFile::~SInternetFile( void )
{
	SInternetFile::Close() ;
}

// SInternetSession 関連付け
//////////////////////////////////////////////////////////////////////////////
void SInternetFile::AttachSession( SInternetSession * pis )
{
	m_pSession = pis ;
}

// 終了処理
//////////////////////////////////////////////////////////////////////////////
void SInternetFile::Close( void )
{
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
}

// URL 設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::SetRequest
	( const wchar_t * pwszURL, const wchar_t * pwszCmd )
{
	m_strCmd = pwszCmd ;
	return	ParseURL( pwszURL, m_strScheme, m_strHost,
						m_strPort, m_strUser, m_strPassword, m_strPath ) ;
}

// 送信データ設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::SetSendData
	( const uint8_t * pbytData, ssize_t nBytes )
{
	if ( m_strCmd.CompareNoCase( L"POST" ) != 0 )
	{
		return	errFailed ;
	}
	if ( nBytes < 0 )
	{
		for ( nBytes = 0; pbytData[nBytes]; nBytes ++ )
		{
		}
	}
	m_bufSendData.RemoveAll() ;
	m_bufSendData.AddArray( pbytData, nBytes ) ;
	//
	SString	strContentLength ;
	SString	strHeader = L"Content-Length: " ;
	strContentLength.FromInteger( nBytes ) ;
	strHeader += strContentLength ;
	//
	return	AddHeader( strHeader ) ;
}

// 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::AddHeader( const wchar_t * pwszHeader )
{
	if ( pwszHeader == NULL )
	{
		return	errFailed ;
	}
	ssize_t	iSep = -1 ;
	for ( size_t i = 0; pwszHeader[i]; i ++ )
	{
		if ( pwszHeader[i] == L':' )
		{
			iSep = (ssize_t) i ;
			break ;
		}
	}
	if ( iSep < 0 )
	{
		return	errFailed ;
	}
	m_sendHeader.SetAs
		( SString( pwszHeader, iSep ), new SString( pwszHeader ) ) ;
	return	errSuccess ;
}

// サーバへ接続
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::Connect( uint32_t nFlags )
{
	if ( m_hConnect != NULL )
	{
		return	errFailed ;
	}
	HINTERNET		hInternet = NULL ;
	DWORD			dwService = INTERNET_SERVICE_HTTP ;
	INTERNET_PORT	nHostPort = INTERNET_INVALID_PORT_NUMBER ;
	if ( m_pSession != NULL )
	{
		hInternet = *m_pSession ;
	}
	else if ( SInternetSession::GetInstance() != NULL )
	{
		hInternet = *(SInternetSession::GetInstance()) ;
	}
	if ( m_strScheme.CompareNoCase( L"http" ) == 0 )
	{
		dwService = INTERNET_SERVICE_HTTP ;
		nHostPort = INTERNET_DEFAULT_HTTP_PORT ;
	}
	else if ( m_strScheme.CompareNoCase( L"https" ) == 0 )
	{
		dwService = INTERNET_SERVICE_HTTP ;
		nHostPort = INTERNET_DEFAULT_HTTPS_PORT ;
	}
	else if ( m_strScheme.CompareNoCase( L"ftp" ) == 0 )
	{
		dwService = INTERNET_SERVICE_FTP ;
		nHostPort = INTERNET_DEFAULT_FTP_PORT ;
	}
	if ( !m_strPort.IsEmpty() )
	{
		bool			flagError ;
		INTERNET_PORT	nPort =
			(INTERNET_PORT) m_strPort.AsInteger( 10, false, &flagError ) ;
		if ( !flagError )
		{
			nHostPort = nPort ;
		}
	}
	DWORD	dwFlags = 0 ;
	m_nConnectFlags = nFlags ;
	/*
	if ( nFlags & connectNoCache )
	{
		dwFlags |= INTERNET_FLAG_RELOAD ;
	}
	*/
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		m_hConnect = ::InternetConnectW
			( hInternet, m_strHost, nHostPort,
				m_strUser, m_strPassword,
				dwService, dwFlags, (DWORD_PTR) this ) ;
	}
	else
	{
		SArray<char>	bufHost, bufUser, bufPassword ;
		m_hConnect = ::InternetConnectA
			( hInternet,
				m_strHost.EncodeDefaultTo(bufHost), nHostPort,
				m_strUser.EncodeDefaultTo(bufUser),
				m_strPassword.EncodeDefaultTo(bufPassword),
				dwService, dwFlags, (DWORD_PTR) this ) ;
	}
	if ( m_hConnect == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// リクエスト送信
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::SendRequest( void )
{
	if ( (m_hFile != NULL) || (m_hConnect == NULL) )
	{
		return	errFailed ;
	}
	DWORD	dwFlags = INTERNET_FLAG_EXISTING_CONNECT ;
	if ( m_strScheme.CompareNoCase( L"https" ) == 0 )
	{
		dwFlags |= INTERNET_FLAG_SECURE ;
	}
	if ( m_nConnectFlags & connectNoCache )
	{
		dwFlags |= INTERNET_FLAG_RELOAD ;
	}
	SString			strHeader, strTemp ;
	const size_t	countHeader = m_sendHeader.GetLength() ;
	for ( size_t i = 0; i < countHeader; i ++ )
	{
		SString *	pstrHeader = m_sendHeader.GetAt( i ) ;
		if ( pstrHeader != NULL )
		{
			strTemp = *pstrHeader ;
			if ( pstrHeader->GetLastAt( 0 ) != L'\n' )
			{
				strTemp += L"\r\n" ;
			}
			strHeader += strTemp ;
		}
	}
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		LPCWSTR		pwszAcceptTypes[] = { L"*/*\0\0", NULL } ;
		m_hFile = ::HttpOpenRequestW
			( m_hConnect, m_strCmd, m_strPath,
				L"HTTP/1.1", NULL,
				pwszAcceptTypes, dwFlags, (DWORD_PTR) this ) ;
		if ( m_hFile == NULL )
		{
			return	errFailed ;
		}
		if ( !::HttpSendRequestW
			( m_hFile, strHeader, (DWORD) strHeader.GetLength(),
				m_bufSendData.GetArray(), (DWORD) m_bufSendData.GetLength() ) )
		{
			m_bufSendData.FinishArray() ;
			return	errFailed ;
		}
		m_bufSendData.FinishArray() ;
	}
	else
	{
		LPCSTR			pszAcceptTypes[] = { "*/*\0\0", NULL } ;
		SArray<char>	bufCmd, bufPath ;
		m_hFile = ::HttpOpenRequestA
			( m_hConnect,
				m_strCmd.EncodeDefaultTo(bufCmd),
				m_strPath.EncodeDefaultTo(bufPath),
				"HTTP/1.1", NULL,
				pszAcceptTypes, dwFlags, (DWORD_PTR) this ) ;
		if ( m_hFile == NULL )
		{
			return	errFailed ;
		}
		SArray<char>	bufHeader ;
		const char *	pszHeader = strHeader.EncodeDefaultTo( bufHeader ) ;
		if ( !::HttpSendRequestA
			( m_hFile, pszHeader, (DWORD) strlen(pszHeader),
				m_bufSendData.GetArray(), (DWORD) m_bufSendData.GetLength() ) )
		{
			m_bufSendData.FinishArray() ;
			return	errFailed ;
		}
		m_bufSendData.FinishArray() ;
	}
	return	errSuccess ;
}

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryStatusCode( uint32_t& codeStatus ) const
{
	DWORD	dwBufLen = sizeof(uint32_t) ;
	if ( !::HttpQueryInfo
		( m_hFile,
			HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
			&codeStatus, &dwBufLen, NULL ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryContentLength( uint64_t& numLength ) const
{
	DWORD	dwBufLen = sizeof(DWORD) ;
	DWORD	dwLength = 0 ;
	if ( !::HttpQueryInfo
		( m_hFile,
			HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH,
			&dwLength, &dwBufLen, NULL ) )
	{
		return	errFailed ;
	}
	numLength = dwLength ;
	return	errSuccess ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryContentType( SString& strType ) const
{
	return	QueryInfoString( strType, HTTP_QUERY_CONTENT_TYPE ) ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryContentTransferEncoding( SString& strEncoding ) const
{
	return	QueryInfoString( strEncoding, HTTP_QUERY_CONTENT_TRANSFER_ENCODING ) ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryContentDate( DATE_TIME& dt ) const
{
	SYSTEMTIME	st ;
	DWORD	dwBufLen = sizeof(SYSTEMTIME) ;
	if ( !::HttpQueryInfo
		( m_hFile,
			HTTP_QUERY_FLAG_SYSTEMTIME | HTTP_QUERY_DATE,
			&st, &dwBufLen, NULL ) )
	{
		return	errFailed ;
	}
	dt.nYear = st.wYear ;
	dt.nMonth = st.wMonth ;
	dt.nWeek = st.wDayOfWeek ;
	dt.nDay = st.wDay ;
	dt.nHour = st.wHour ;
	dt.nMinute = st.wMinute ;
	dt.nSecond = st.wSecond ;
	dt.nMilliSec = st.wMilliseconds ;
	return	errSuccess ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryContentLastModified( DATE_TIME& dt ) const
{
	SYSTEMTIME	st ;
	DWORD	dwBufLen = sizeof(SYSTEMTIME) ;
	if ( !::HttpQueryInfo
		( m_hFile,
			HTTP_QUERY_FLAG_SYSTEMTIME | HTTP_QUERY_LAST_MODIFIED,
			&st, &dwBufLen, NULL ) )
	{
		return	errFailed ;
	}
	dt.nYear = st.wYear ;
	dt.nMonth = st.wMonth ;
	dt.nWeek = st.wDayOfWeek ;
	dt.nDay = st.wDay ;
	dt.nHour = st.wHour ;
	dt.nMinute = st.wMinute ;
	dt.nSecond = st.wSecond ;
	dt.nMilliSec = st.wMilliseconds ;
	return	errSuccess ;
}

// 受信ヘッダ文字列取得
//////////////////////////////////////////////////////////////////////////////
SError SInternetFile::QueryInfoString
	( SString& strValue, DWORD dwInfoLevel ) const
{
	DWORD	dwBufLen = 0 ;
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		if ( !::HttpQueryInfoW( m_hFile, dwInfoLevel, NULL, &dwBufLen, 0 ) )
		{
			if ( dwBufLen == 0 )
			{
				return	errFailed ;
			}
		}
		if ( !::HttpQueryInfoW
			( m_hFile, dwInfoLevel,
				strValue.LockBuffer( dwBufLen >> 1 ), &dwBufLen, 0 ) )
		{
			strValue.UnlockBuffer( 0 ) ;
			return	errFailed ;
		}
		strValue.UnlockBuffer( dwBufLen >> 1 ) ;
	}
	else
	{
		if ( !::HttpQueryInfoA( m_hFile, dwInfoLevel, NULL, &dwBufLen, 0 ) )
		{
			if ( dwBufLen == 0 )
			{
				return	errFailed ;
			}
		}
		SArray<char>	bufValue ;
		bufValue.SetLength( dwBufLen + 1 ) ;
		if ( !::HttpQueryInfoA
			( m_hFile, dwInfoLevel, bufValue.GetArray(), &dwBufLen, 0 ) )
		{
			bufValue.FinishArray() ;
			return	errFailed ;
		}
		bufValue.FinishArray() ;
		strValue.SetString( bufValue.GetConstArray(), dwBufLen ) ;
	}
	return	errSuccess ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SInternetFile::Duplicate( void ) const
{
	return	new SInternetFile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SInternetFile::Read( void * ptrBuf, size_t nBytes )
{
	DWORD	dwReadBytes = 0 ;
	if ( !::InternetReadFile
		( m_hFile, ptrBuf, (DWORD) nBytes, &dwReadBytes ) )
	{
		return	0 ;
	}
	return	dwReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SInternetFile::Write( const void * ptrBuf, size_t nBytes )
{
	DWORD	dwWrittenBytes = 0 ;
	if ( !::InternetWriteFile
		( m_hFile, ptrBuf, (DWORD) nBytes, &dwWrittenBytes ) )
	{
		return	0 ;
	}
	return	dwWrittenBytes ;
}

