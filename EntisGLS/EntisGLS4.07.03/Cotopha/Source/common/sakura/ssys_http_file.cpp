
#include <sakura/sakura.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_http_file.h>

#if	defined(__PLATFORM_ANDROID__)
#include <sakura/ssys_android_file.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// HTTP ファイル抽象クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SHttpFileInterface, SFileInterface )

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryStatusCode( uint32_t& codeStatus ) const
{
	return	errFailed ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentLength( uint64_t& numLength ) const
{
	return	errFailed ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentType( SString& strType ) const
{
	return	errFailed ;
}

// 文字エンコーディング取得 (Content-Type の charset)
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentTypeCharset( SString& strCharset ) const
{
	SString	strType ;
	SError	err = QueryContentType( strType ) ;
	if ( err != errSuccess )
	{
		return	err ;
	}
	SStringParser	sparsType ;
	sparsType.AttachString( strType ) ;
	sparsType.PassEnclosedString( L';' ) ;
	if ( sparsType.HasToComeChar( L";" ) != L';' )
	{
		return	errFailed ;
	}
	if ( !sparsType.HasToComeToken( L"charset" ) )
	{
		return	errFailed ;
	}
	if ( sparsType.HasToComeChar( L"=" ) != L'=' )
	{
		return	errFailed ;
	}
	if ( !sparsType.NextString( strCharset ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentTransferEncoding( SString& strEncoding ) const
{
	return	errFailed ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentDate( DATE_TIME& dt ) const
{
	return	errFailed ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::QueryContentLastModified( DATE_TIME& dt ) const
{
	return	errFailed ;
}

// URL を開く
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::OpenURL
	( const wchar_t * pwszURL, const wchar_t * pszAgent,
		const uint8_t * pbytData, ssize_t nLength,
		const wchar_t * pwszContentType, uint32_t nFlags )
{
	SError	err ;
	if ( (pbytData != NULL) && (nLength != 0) )
	{
		err = SetRequest( pwszURL, L"POST" ) ;
		if ( err )
		{
			return	err ;
		}
		err = SetSendData( pbytData, nLength ) ;
		if ( err )
		{
			return	err ;
		}
		if ( pwszContentType != NULL )
		{
			SString	strContentType = L"Content-Type: " ;
			strContentType += pwszContentType ;
			err = AddHeader( strContentType ) ;
		}
		else
		{
			err = AddHeader
				( L"Content-Type: "
					L"application/x-www-form-urlencoded; charset=utf-8" ) ;
		}
	}
	else
	{
		err = SetRequest( pwszURL ) ;
	}
	if ( err )
	{
		return	err ;
	}
	if ( pszAgent != NULL )
	{
		SString	strUserAgent = L"User-Agent: " ;
		strUserAgent += pszAgent ;
		err = AddHeader( strUserAgent ) ;
		if ( err )
		{
			return	err ;
		}
	}
	err = Connect( nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	return	SendRequest() ;
}

// URL フォームパラメータを送信データに設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::SetSendURLFormData
	( const wchar_t * pwszURLFormed, Charset::EncodingType typeEncoding )
{
	SArray<uint8_t>	bufURLFormed ;
	Charset::Encode( bufURLFormed, typeEncoding, pwszURLFormed ) ;
	//
	return	SetSendData
		( bufURLFormed.GetConstArray(), (ssize_t) bufURLFormed.GetLength() ) ;
}

// URL 解釈
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::ParseURL
	( const wchar_t * pwszURL,
		SString& strScheme, SString& strHost, SString& strPort,
		SString& strUser, SString& strPassword, SString& strPath )
{
	SStringParser	sparsURL = pwszURL ;
	sparsURL.MarkIndex() ;
	if ( !sparsURL.SeekString( L"://" ) )
	{
		return	errFailed ;
	}
	strScheme = sparsURL.SubStringFromMark() ;
	//
	sparsURL.SeekIndex( sparsURL.GetIndex() + 3 ) ;
	sparsURL.MarkIndex() ;
	sparsURL.PassEnclosedString( L'/' ) ;
	strHost = sparsURL.SubStringFromMark() ;
	strPath = sparsURL.SubString( sparsURL.GetIndex() ) ;
	if ( strPath.IsEmpty() )
	{
		strPath = L"/" ;
	}
	//
	SString	strTemp ;
	ssize_t	iSepHost = strHost.Find( L'@' ) ;
	if ( iSepHost >= 0 )
	{
		ssize_t	iSepUser = strHost.Find( L':' ) ;
		if ( (iSepUser >= 0) & (iSepUser < iSepHost) )
		{
			UnformatURL( strUser, strHost.Left( iSepUser ) ) ;
			UnformatURL
				( strPassword,
					strHost.Middle( iSepUser + 1, iSepHost - iSepUser ) ) ;
		}
		else
		{
			UnformatURL( strUser, strHost.Left( iSepHost ) ) ;
			strPassword.FreeArray() ;
		}
		strHost = strHost.Middle( iSepHost + 1 ) ;
	}
	else
	{
		strUser.FreeArray() ;
		strPassword.FreeArray() ;
	}
	//
	ssize_t	iSepPort = strHost.Find( L':' ) ;
	if ( iSepPort >= 0 )
	{
		strPort = strHost.Middle( iSepPort + 1 ) ;
		strHost = strHost.Left( iSepPort ) ;
	}
	else
	{
		strPort.FreeArray() ;
	}
	return	errSuccess ;
}

// URL 文字列のエンコード
//////////////////////////////////////////////////////////////////////////////
void SHttpFileInterface::FormatURL( SString& strURL, const wchar_t * pwszURL )
{
	static const wchar_t	wchSafeChar[] = L";:/?=&$-_.+!*'(),\"" ;
	SArray<uint8_t>	bufURL ;
	Charset::Encode( bufURL, Charset::encodingUTF8, pwszURL ) ;
	//
	const uint8_t *	pbytURL = bufURL.GetConstArray() ;
	size_t			iStart = 0 ;
	strURL = L"" ;
	//
	for ( size_t i = 0; i < bufURL.GetLength(); i ++ )
	{
		wchar_t	wchCode = pbytURL[i] ;
		bool	flagCtrl = false ;
		if ( ((L'0' <= wchCode) & (wchCode <= L'9'))
			|| ((L'A' <= wchCode) & (wchCode <= L'Z'))
			|| ((L'a' <= wchCode) & (wchCode <= L'z')) )
		{
			// 英数字
		}
		else if ( (wchCode & 0x80)
				|| (/*(0 <= wchCode) &*/ (wchCode < 0x20))
				|| (wchCode == 0x7F) )
		{
			// 制御文字
			flagCtrl = true ;
		}
		else
		{
			// 記号
			flagCtrl = true ;
			for ( size_t j = 0; wchSafeChar[j]; j ++ )
			{
				if ( wchCode == wchSafeChar[j] )
				{
					flagCtrl = false ;
					break ;
				}
			}
		}
		if ( flagCtrl )
		{
			SString	strTemp ;
			Charset::Decode
				( strTemp, Charset::encodingUTF8,
					pbytURL + iStart, (ssize_t) (i - iStart) ) ;
			strURL += strTemp ;
			//
			wchar_t	wchBuf[4] ;
			wchBuf[0] = L'%' ;
			//
			wchar_t	k = (wchCode >> 4) & 0x0F ;
			if ( k < 10 )
			{
				wchBuf[1] = (wchar_t) (L'0' + k) ;
			}
			else
			{
				wchBuf[1] = (wchar_t) (L'A' + k - 10) ;
			}
			k = wchCode & 0x0F ;
			if ( k < 10 )
			{
				wchBuf[2] = (wchar_t) (L'0' + k) ;
			}
			else
			{
				wchBuf[2] = (wchar_t) (L'A' + k - 10) ;
			}
			wchBuf[3] = 0 ;
			//
			strURL += &wchBuf[0] ;
			iStart = i + 1 ;
		}
	}
	SString	strTemp ;
	Charset::Decode
		( strTemp, Charset::encodingUTF8,
			pbytURL + iStart, (ssize_t) (bufURL.GetLength() - iStart) ) ;
	strURL += strTemp ;
}

// URL 文字列のデコード
//////////////////////////////////////////////////////////////////////////////
void SHttpFileInterface::UnformatURL( SString& strURL, const wchar_t * pwszURL )
{
	SArray<uint8_t>	bufURL ;
	Charset::Encode( bufURL, Charset::encodingUTF8, pwszURL ) ;
	//
	uint8_t *	pbytURL = bufURL.GetArray() ;
	size_t		lenURL = bufURL.GetLength() ;
	size_t		iDst = 0, iSrc = 0 ;
	while ( iSrc < lenURL )
	{
		uint8_t	nCode = pbytURL[iSrc ++] ;
		if ( nCode == L'%' )
		{
			uint32_t	nDecoded = 0 ;
			for ( size_t i = 0; (i < 2) && (iSrc < lenURL); i ++ )
			{
				nCode = pbytURL[iSrc ++] ;
				nDecoded <<= 4 ;
				if ( (nCode >= L'0') & (nCode <= L'9') )
				{
					nDecoded |= (nCode - '0') ;
				}
				else if ( (nCode >= L'A') & (nCode <= L'F') )
				{
					nDecoded |= (nCode + (10 - L'A')) ;
				}
				else if ( (nCode >= L'a') & (nCode <= L'f') )
				{
					nDecoded = (nCode + (10 - L'A')) ;
				}
				else
				{
					nDecoded >>= 4 ;
					iSrc -- ;
					break ;
				}
			}
			if ( nDecoded == 0 )
			{
				nDecoded = L'%' ;
			}
			pbytURL[iDst ++] = (uint8_t) nDecoded ;
		}
		else
		{
			pbytURL[iDst ++] = nCode ;
		}
	}
	Charset::Decode( strURL, Charset::encodingUTF8, pbytURL, (ssize_t) iDst ) ;
	bufURL.FinishArray() ;
}

// HTTP 日付の解釈
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::ParseDate( DATE_TIME& dt, const wchar_t * pwszDate )
{
	SStringParser	sparsDate = pwszDate ;
	SString	strWeek, strMonth ;
	sparsDate.NextToken( strWeek ) ;
	//
	int	iWeek = ParseDateWeek3( strWeek ) ;
	if ( iWeek >= 0 )
	{
		dt.nWeek = (uint16_t) iWeek ;
		if ( sparsDate.HasToComeChar( L"," ) == L',' )
		{
			// RFC 822 style
			dt.nDay = (uint16_t) sparsDate.NextInteger() ;
			sparsDate.NextToken( strMonth ) ;
			dt.nMonth = (uint16_t) ParseDateMonth( strMonth ) ;
			dt.nYear = (uint16_t) sparsDate.NextInteger() ;
			//
			dt.nHour = (uint16_t) sparsDate.NextInteger() ;
			if ( sparsDate.HasToComeChar( L":" ) != L':' )
			{
				return	errFailed ;
			}
			dt.nMinute = (uint16_t) sparsDate.NextInteger() ;
			if ( sparsDate.HasToComeChar( L":" ) != L':' )
			{
				return	errFailed ;
			}
			dt.nSecond = (uint16_t) sparsDate.NextInteger() ;
		}
		else
		{
			// ANSI C style
			sparsDate.NextToken( strMonth ) ;
			dt.nMonth = (uint16_t) ParseDateMonth( strMonth ) ;
			dt.nDay = (uint16_t) sparsDate.NextInteger() ;
			//
			dt.nHour = (uint16_t) sparsDate.NextInteger() ;
			if ( sparsDate.HasToComeChar( L":" ) != L':' )
			{
				return	errFailed ;
			}
			dt.nMinute = (uint16_t) sparsDate.NextInteger() ;
			if ( sparsDate.HasToComeChar( L":" ) != L':' )
			{
				return	errFailed ;
			}
			dt.nSecond = (uint16_t) sparsDate.NextInteger() ;
			//
			dt.nYear = (uint16_t) sparsDate.NextInteger() ;
		}
	}
	else
	{
		// RFC 850
		iWeek = ParseDateWeek( strWeek ) ;
		if ( iWeek < 0 )
		{
			return	errFailed ;
		}
		if ( sparsDate.HasToComeChar( L"," ) != L',' )
		{
			return	errFailed ;
		}
		//
		dt.nDay = (uint16_t) sparsDate.NextInteger() ;
		if ( sparsDate.HasToComeChar( L"-" ) != L'-' )
		{
			return	errFailed ;
		}
		sparsDate.NextToken( strMonth ) ;
		dt.nMonth = (uint16_t) ParseDateMonth( strMonth ) ;
		if ( sparsDate.HasToComeChar( L"-" ) != L'-' )
		{
			return	errFailed ;
		}
		dt.nYear = (uint16_t) sparsDate.NextInteger() ;
		if ( dt.nYear < 80 )
		{
			dt.nYear += 1900 ;
		}
		else
		{
			dt.nYear += 2000 ;
		}
		//
		dt.nHour = (uint16_t) sparsDate.NextInteger() ;
		if ( sparsDate.HasToComeChar( L":" ) != L':' )
		{
			return	errFailed ;
		}
		dt.nMinute = (uint16_t) sparsDate.NextInteger() ;
		if ( sparsDate.HasToComeChar( L":" ) != L':' )
		{
			return	errFailed ;
		}
		dt.nSecond = (uint16_t) sparsDate.NextInteger() ;
	}
	dt.nMilliSec = 0 ;
	return	errSuccess ;
}

int SHttpFileInterface::ParseDateWeek( const wchar_t * pwszWeek )
{
	static const wchar_t *	pwszWeekRFC850[7] =
	{
		L"Sunday", L"Monday", L"Tuesday",
		L"Wednesday", L"Thursday", L"Friday", L"Saturday",
	} ;
	for ( size_t iWeek = 0; iWeek < 7; iWeek ++ )
	{
		if ( SString::CompareNoCase( pwszWeekRFC850[iWeek], pwszWeek ) == 0 )
		{
			return	(int) iWeek ;
		}
	}
	return	-1 ;
}

int SHttpFileInterface::ParseDateWeek3( const wchar_t * pwszWeek )
{
	static const wchar_t *	pwszWeekRFC822[7] =
	{
		L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat",
	} ;
	for ( size_t iWeek = 0; iWeek < 7; iWeek ++ )
	{
		if ( SString::CompareNoCase( pwszWeekRFC822[iWeek], pwszWeek ) == 0 )
		{
			return	(int) iWeek ;
		}
	}
	return	-1 ;
}

int SHttpFileInterface::ParseDateMonth( const wchar_t * pwszMonth )
{
	static const wchar_t *	pwszMonthRFC822[12] =
	{
		L"Jan", L"Feb", L"Mar", L"Apr",
		L"May", L"Jun", L"Jul", L"Aug",
		L"Sep", L"Oct", L"Nov", L"Dec",
	} ;
	for ( size_t iMonth = 0; iMonth < 12; iMonth ++ )
	{
		if ( SString::CompareNoCase( pwszMonthRFC822[iMonth], pwszMonth ) == 0 )
		{
			return	(int) (iMonth + 1) ;
		}
	}
	return	-1 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SHttpFileInterface::Write( const void * ptrBuf, size_t nBytes )
{
	return	0 ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SHttpFileInterface::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SHttpFileInterface::GetLength( void ) const
{
	uint64_t	numLength ;
	if ( QueryContentLength( numLength ) == errSuccess )
	{
		return	numLength ;
	}
	return	-1 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SHttpFileInterface::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	-1 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SHttpFileInterface::GetPosition( void ) const
{
	return	-1 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileInterface::SetEndOfFile( void )
{
	return	errFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO
	( SSystem::SHttpFile, SHttpFileInterface )
#else
ESL_IMPLEMENT_CLASS_INFO_CAST
	( SSystem::SHttpFile, SHttpFileInterface, m_pHttpFile )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SHttpFile::SHttpFile( void )
{
	#if	defined(__COTOPHA__)
		m_pHttpFile = new HttpFile ;
	#elif	defined(__PLATFORM_WINDOWS__)
		m_pHttpFile = new SInternetFile ;
	#elif	defined(__PLATFORM_ANDROID__)
		m_pHttpFile = new SAndroidHttpFile ;
	#else
		m_pHttpFile = new SHttpSimpleClient ;
	#endif
}

SHttpFile::SHttpFile( HttpFile * pHttpFile )
{
	m_pHttpFile = pHttpFile ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SHttpFile::~SHttpFile( void )
{
	delete	m_pHttpFile ;
	m_pHttpFile = NULL ;
}

// URL 設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::SetRequest
	( const wchar_t * pwszURL, const wchar_t * pwszCmd )
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->SetRequest( pwszURL, pwszCmd ) ;
}

// 送信データ設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::SetSendData
	( const uint8_t * pbytData, ssize_t nBytes )
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->SetSendData( pbytData, nBytes ) ;
}

// 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::AddHeader( const wchar_t * pwszHeader )
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->AddHeader( pwszHeader ) ;
}

// サーバへ接続
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::Connect( uint32_t nFlags )
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->Connect( nFlags ) ;
}

// リクエスト送信
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::SendRequest( void )
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->SendRequest() ;
}

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryStatusCode( uint32_t& codeStatus ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryStatusCode( codeStatus ) ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryContentLength( uint64_t& numLength ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryContentLength( numLength ) ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryContentType( SString& strType ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryContentType( strType ) ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryContentTransferEncoding( SString& strEncoding ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryContentTransferEncoding( strEncoding ) ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryContentDate( DATE_TIME& dt ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryContentDate( dt ) ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpFile::QueryContentLastModified( DATE_TIME& dt ) const
{
	if ( m_pHttpFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pHttpFile->QueryContentLastModified( dt ) ;
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SHttpFile::GetFileObject( void )
{
	return	m_pHttpFile ;
}

#else
SFileInterface * SHttpFile::GetFileObject( void )
{
	return	this ;
}
#endif

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SHttpFile::Duplicate( void ) const
{
	return	new SHttpFile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SHttpFile::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_pHttpFile == NULL )
	{
		return	0 ;
	}
	return	m_pHttpFile->Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SHttpFile::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_pHttpFile == NULL )
	{
		return	0 ;
	}
	return	m_pHttpFile->Write( ptrBuf, nBytes ) ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP クライアント簡易実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SHttpSimpleClient, SHttpFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SHttpSimpleClient::SHttpSimpleClient( void )
{
	m_codeStatus = (uint32_t) -1 ;
	m_flagChunked = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SHttpSimpleClient::~SHttpSimpleClient( void )
{
}

// URL 設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::SetRequest
	( const wchar_t * pwszURL, const wchar_t * pwszCmd )
{
	m_strCmd = pwszCmd ;
	return	ParseURL( pwszURL, m_strScheme, m_strHost,
						m_strPort, m_strUser, m_strPassword, m_strPath ) ;
}

// 送信データ設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::SetSendData
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
	strHeader += SString( nBytes ) ;
	//
	return	AddHeader( strHeader ) ;
}

// 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::AddHeader( const wchar_t * pwszHeader )
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
SError SHttpSimpleClient::Connect( uint32_t nFlags )
{
	SError	err ;
	m_codeStatus = (uint32_t) -1 ;
	//
	err = m_socket.Create() ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nHostPort = 80 ;
	if ( !m_strPort.IsEmpty() )
	{
		bool	flagError ;
		nHostPort = (uint32_t) m_strPort.AsInteger( 10, false, &flagError ) ;
		if ( flagError )
		{
			nHostPort = 80 ;
		}
	}
	return	m_socket.Connect( m_strHost, nHostPort ) ;
}

// リクエスト送信
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::SendRequest( void )
{
	//
	// Host ヘッダ追加
	//
	SError	err ;
	SString	strHostHeader = L"Host: " ;
	strHostHeader += m_strHost ;
	if ( !m_strPort.IsEmpty() )
	{
		strHostHeader += L":" ;
		strHostHeader += m_strPort ;
	}
	err = AddHeader( strHostHeader ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// コマンド送信
	//
	SString	strCmd = m_strCmd ;
	strCmd += L" " ;
	strCmd += m_strPath ;
	strCmd += L" HTTP/1.1\r\n" ;
	m_socket.WriteEncodedString( strCmd, Charset::encodingUTF8 ) ;
	//
	// ヘッダ送信
	//
	const size_t	countHeader = m_sendHeader.GetLength() ;
	SString			strTemp ;
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
			m_socket.WriteEncodedString( strTemp, Charset::encodingUTF8 ) ;
		}
	}
	m_sendHeader.RemoveAll() ;
	m_socket.WriteEncodedString( L"\r\n", 2, Charset::encodingUTF8 ) ;
	//
	// 本文送信 (POST コマンドのみ)
	//
	if ( (m_strCmd.CompareNoCase( L"POST" ) == 0)
				&& (m_bufSendData.GetLength() != 0) )
	{
		m_socket.Write
			( m_bufSendData.GetConstArray(), m_bufSendData.GetLength() ) ;
		m_bufSendData.FreeArray() ;
	}
	//
	// ステータスコード受信
	//
	SArray<uint8_t>	bufRecvLine ;
	SString			strRecvLine ;
	m_socket.ReadLine( bufRecvLine ) ;
	Charset::Decode
		( strRecvLine, Charset::encodingUTF8,
			bufRecvLine.GetConstArray(), (ssize_t) bufRecvLine.GetLength() ) ;
	if ( strRecvLine.CompareLeftNoCase( L"HTTP/1.1" ) != 0 )
	{
		return	errFailed ;
	}
	SStringParser	sparsLine ;
	sparsLine.AttachString( strRecvLine ) ;
	sparsLine.SeekIndex( 8 ) ;
	//
	m_codeStatus = (uint32_t) sparsLine.NextInteger() ;
	//
	// ヘッダ受信
	//
	m_recvHeader.RemoveAll() ;
	for ( ; ; )
	{
		if ( m_socket.ReadLine( bufRecvLine ) == 0 )
		{
			break ;
		}
		Charset::Decode
			( strRecvLine, Charset::encodingUTF8,
				bufRecvLine.GetConstArray(), (ssize_t) bufRecvLine.GetLength() ) ;
		strRecvLine.TrimRight() ;
		if ( strRecvLine.IsEmpty() )
		{
			break ;
		}
		ssize_t	iSep = strRecvLine.Find( L':' ) ;
		if ( iSep >= 0 )
		{
			SString *	pstrValue =
					new SString( strRecvLine.Middle( iSep + 1 ) ) ;
			pstrValue->TrimRight() ;
			pstrValue->TrimLeft() ;
			m_recvHeader.SetAs( strRecvLine.Left(iSep), pstrValue ) ;
		}
	}
	//
	// Transfer-Encoding 判定
	//
	SString *	pstrTransferEncoding =
					m_recvHeader.GetAs( L"Transfer-Encoding" ) ;
	m_flagChunked = false ;
	m_qbufRecv.ClearAll() ;
	if ( (pstrTransferEncoding != NULL)
		&& (*pstrTransferEncoding == L"chunked") )
	{
		m_flagChunked = true ;
		ReceiveNextChunk() ;
	}
	//
	// ステータスコード
	//
	switch ( m_codeStatus / 100 )
	{
	case	2:
	case	3:
		return	errSuccess ;
	case	1:
		return	errContinue ;
	case	4:
		return	errInvalidParam ;
	}
	return	errFailed ;
}

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryStatusCode( uint32_t& codeStatus ) const
{
	codeStatus = m_codeStatus ;
	if ( m_codeStatus == (uint32_t) -1 )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryContentLength( uint64_t& numLength ) const
{
	SString *	pstrContentLength = m_recvHeader.GetAs( L"Content-Length" ) ;
	if ( pstrContentLength == NULL )
	{
		return	errFailed ;
	}
	bool	flagError ;
	numLength = pstrContentLength->AsInteger( 10, false, &flagError ) ;
	if ( flagError )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryContentType( SString& strType ) const
{
	SString *	pstrContentType = m_recvHeader.GetAs( L"Content-Type" ) ;
	if ( pstrContentType == NULL )
	{
		return	errFailed ;
	}
	strType = *pstrContentType ;
	return	errSuccess ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryContentTransferEncoding( SString& strEncoding ) const
{
	SString *	pstrContentTransferEncoding =
			m_recvHeader.GetAs( L"Content-Transfer-Encoding" ) ;
	if ( pstrContentTransferEncoding == NULL )
	{
		return	errFailed ;
	}
	strEncoding = *pstrContentTransferEncoding ;
	return	errSuccess ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryContentDate( DATE_TIME& dt ) const
{
	SString *	pstrDate = m_recvHeader.GetAs( L"Date" ) ;
	if ( pstrDate == NULL )
	{
		return	errFailed ;
	}
	return	ParseDate( dt, *pstrDate ) ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryContentLastModified( DATE_TIME& dt ) const
{
	SString *	pstrLastModified = m_recvHeader.GetAs( L"Last-Modified" ) ;
	if ( pstrLastModified == NULL )
	{
		return	errFailed ;
	}
	return	ParseDate( dt, *pstrLastModified ) ;
}

// 受信ヘッダ取得
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleClient::QueryHeader
	( SString& strValue, const wchar_t * pwszName )
{
	SString *	pstrValue = m_recvHeader.GetAs( pwszName ) ;
	if ( pstrValue == NULL )
	{
		return	errFailed ;
	}
	strValue = *pstrValue ;
	return	errSuccess ;
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SHttpSimpleClient::GetFileObject( void )
{
	return	m_socket.GetFileObject() ;
}

#else
SFileInterface * SHttpSimpleClient::GetFileObject( void )
{
	return	m_socket.GetFileObject() ;
}

#endif

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SHttpSimpleClient::Duplicate( void ) const
{
	return	new SHttpSimpleClient ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SHttpSimpleClient::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_flagChunked )
	{
		if ( m_qbufRecv.GetLength() == 0 )
		{
			ReceiveNextChunk() ;
		}
		return	m_qbufRecv.Read( ptrBuf, nBytes ) ;
	}
	else
	{
		return	m_socket.Receive( ptrBuf, nBytes ) ;
	}
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SHttpSimpleClient::Write( const void * ptrBuf, size_t nBytes )
{
	return	m_socket.Send( ptrBuf, nBytes ) ;
}

// chunked 転送に対応するために次の１チャンクを読み込む
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleClient::ReceiveNextChunk( void )
{
	//
	// チャンクサイズ（16進数）
	//
	SArray<uint8_t>	bufRecvLine ;
	SString			strRecvLine ;
	m_socket.ReadLine( bufRecvLine ) ;
	Charset::Decode
		( strRecvLine, Charset::encodingUTF8,
			bufRecvLine.GetConstArray(), (ssize_t) bufRecvLine.GetLength() ) ;
	//
	bool	flagError ;
	size_t	nChunkBytes =
		(size_t) strRecvLine.AsInteger( 16, false, &flagError ) ;
	if ( flagError || (nChunkBytes == 0) )
	{
		return ;
	}
	//
	// チャンクデータ受信
	//
	uint8_t *	pbytBuf = m_qbufRecv.PutBuffer( nChunkBytes ) ;
	size_t		nRecvBytes = m_socket.Read( pbytBuf, nChunkBytes ) ;
	m_qbufRecv.FlushBuffer( nRecvBytes ) ;
	//
	// チャンク終端に追加される CRLF
	//
	m_socket.ReadLine( bufRecvLine ) ;
}


//////////////////////////////////////////////////////////////////////////////
// HTTP・オープン・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SHttpFileOpener, SFileOpener )

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SHttpFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SString	strURL = pszFilePath ;
	if ( strURL.Find( L"://" ) < 0 )
	{
		strURL = SString( L"http://" ) + strURL ;
	}
	SHttpFile *	pHttpFile = new SHttpFile ;
	if ( pHttpFile->OpenURL( strURL ) )
	{
		delete	pHttpFile ;
		return	NULL ;
	}
	return	pHttpFile ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SHttpFileOpener::IsExisting( const wchar_t * pszFilePath )
{
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SHttpFileOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SHttpFileOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SHttpFileOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
}



//////////////////////////////////////////////////////////////////////////////
// HTTP ヘッダ
//////////////////////////////////////////////////////////////////////////////

// ヘッダ追加
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::HeaderList::AddHeader( const wchar_t * pwszHeader )
{
	Add( new SString( pwszHeader ) ) ;
}

// ヘッダ取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SHttpSimpleServer::HeaderList::GetHeader
				( const wchar_t * pwszFieldName ) const
{
	size_t	nFieldNameLen = SString::GetLength( pwszFieldName ) ;
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SString *	pstrHeader = GetAt( i ) ;
		if ( (pstrHeader != NULL)
			&& (pstrHeader->CompareLeft( pwszFieldName ) == 0)
			&& (pstrHeader->GetAt( nFieldNameLen ) == L':') )
		{
			size_t	j = nFieldNameLen + 1 ;
			while ( pstrHeader->GetAt( j ) == L' ' )
			{
				j ++ ;
			}
			return	((const wchar_t*) *pstrHeader) + j ;
		}
	}
	return	NULL ;
}

// ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::HeaderList::SetHeader
	( const wchar_t * pwszFieldName, const wchar_t * pwszValue )
{
	size_t	nFieldNameLen = SString::GetLength( pwszFieldName ) ;
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SString *	pstrHeader = GetAt( i ) ;
		if ( (pstrHeader != NULL)
			&& (pstrHeader->CompareLeft( pwszFieldName ) == 0)
			&& (pstrHeader->GetAt( nFieldNameLen ) == L':') )
		{
			*pstrHeader =
				pstrHeader->Left( nFieldNameLen + 1 ) + L" " + pwszValue ;
			return ;
		}
	}
	SString *	pstrHeader = new SString( pwszFieldName ) ;
	*pstrHeader += L": " ;
	*pstrHeader += pwszValue ;
	Add( pstrHeader ) ;
}



//////////////////////////////////////////////////////////////////////////////
// HTTP 簡易サーバー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SSystem::SHttpSimpleServer, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SHttpSimpleServer::SHttpSimpleServer( void )
{
	m_fStartup = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SHttpSimpleServer::~SHttpSimpleServer( void )
{
	if ( m_fStartup )
	{
		Shutdown() ;
	}
}

// サービス開始
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleServer::Startup( uint32_t nPort )
{
	if ( m_fStartup )
	{
		return	errFailed ;
	}
	if ( m_socketListen.Create( nPort ) != errSuccess )
	{
		return	errFailed ;
	}
	if ( m_socketListen.Listen() != errSuccess )
	{
		return	errFailed ;
	}
	m_sigExitListen.Initialize( false ) ;
	if ( m_threadListen.BeginThread( this ) )
	{
		m_socketListen.Close() ;
		return	errFailed ;
	}
	m_fStartup = true ;
	return	errSuccess ;
}

// サービス終了
//////////////////////////////////////////////////////////////////////////////
SError SHttpSimpleServer::Shutdown( void )
{
	if ( !m_fStartup )
	{
		return	errFailed ;
	}
	m_sigExitListen.SetSignal() ;
	m_threadListen.Wait() ;
	m_sigExitListen.Delete() ;
	m_threadListen.Delete() ;
	m_socketListen.Close() ;
	m_fStartup = false ;
	return	errSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::Run( void )
{
	while ( m_sigExitListen.Wait(1) == errTimeout )
	{
		if ( m_socketListen.Poll( 0, 10 ) & SSocket::pollIn )
		{
			SSyncSocket *	socket = new SSyncSocket ;
			if ( !m_socketListen.Accept( *socket ) )
			{
				OnAccept( socket ) ;
			}
			else
			{
				delete	socket ;
			}
		}
	}
}

// 接続受付
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::OnAccept( SSyncSocket * socket )
{
	//
	// コマンド取得
	//
	SStringParser	sparsCmdLine = ReadLine( *socket ) ;
	SString	strCmd = sparsCmdLine.GetString() ;
	SString	strPath = sparsCmdLine.GetString() ;
	SString	strVer = sparsCmdLine.GetString() ;
	//
	// ヘッダ読み込み
	//
	HeaderList	lstHeaders ;
	for ( ; ; )
	{
		SString	strHeader = ReadLine( *socket ) ;
		strHeader.TrimRight() ;
		if ( strHeader.IsEmpty() )
		{
			break ;
		}
		lstHeaders.AddHeader( strHeader ) ;
	}
	//
	// 応答処理
	//
	OnResponse( socket, strCmd, strPath, lstHeaders ) ;
}

// 応答
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::OnResponse
	( SSyncSocket * socket,
		const SString& strCmd,
		const SString& strPath,
		const SHttpSimpleServer::HeaderList& lstHeader )
{
	Response *	res = CreateResponse( socket, strCmd, strPath, lstHeader ) ;
	SendHTTPResponse( *socket, *res ) ;
	socket->Close() ;
	delete	res ;
	delete	socket ;
}

SHttpSimpleServer::Response *
	SHttpSimpleServer::CreateResponse
		( SSyncSocket * socket,
			const SString& strCmd,
			const SString& strPath,
			const SHttpSimpleServer::HeaderList& lstHeaders )
{
	Response *	res = new Response( 200 ) ;
	res->AddHeader( L"Content-Type: text/plain" ) ;
	return	res ;
}

// 1行受信
//////////////////////////////////////////////////////////////////////////////
SString SHttpSimpleServer::ReadLine( SSyncSocket& socket )
{
	SArray<uint8_t>	buf ;
	socket.ReadLine( buf ) ;
	//
	SString	strLine ;
	Charset::Decode
		( strLine, Charset::encodingUTF8,
			buf.GetConstArray(), (ssize_t) buf.GetLength() ) ;
	//
	return	strLine ;
}

// HTTP応答送信
//////////////////////////////////////////////////////////////////////////////
void SHttpSimpleServer::SendHTTPResponse
	( SSyncSocket& socket, SHttpSimpleServer::Response& res )
{
	socket.WriteEncodedString
		( SString(L"HTTP/1.1 ") + SString(res.m_nStatus) + L" OK\n" ) ;
	//
	res.SetHeader
		( L"Content-Length", SString( res.m_bufContent.GetLength() ) ) ;
	//
	for ( size_t i = 0; i < res.m_lstHeaders.GetLength(); i ++ )
	{
		SString *	pstrHeader = res.m_lstHeaders.GetAt( i ) ;
		if ( pstrHeader != NULL )
		{
			pstrHeader->TrimRight() ;
			socket.WriteEncodedString( *pstrHeader + L"\n" ) ;
		}
	}
	socket.WriteEncodedString( L"\n" ) ;
	//
	if ( res.m_bufContent.GetLength() > 0 )
	{
		socket.Write
			( res.m_bufContent.GetConstArray(),
				res.m_bufContent.GetLength() ) ;
	}
}

