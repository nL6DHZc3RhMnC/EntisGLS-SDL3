
#include <sakura/sakura.h>
#include <sakura/ssys_win_registry.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// レジストリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SRegistryKey, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SRegistryKey::SRegistryKey( void )
	: m_hKey( NULL ), m_fOpened( false )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SRegistryKey::~SRegistryKey( void )
{
	CloseKey() ;
}

// レジストリキーを作成
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::CreateKey
	( HKEY hKey, const wchar_t * pszSubKey, REGSAM samDersired )
{
	CloseKey() ;

	DWORD	dwDisposition = 0 ;
	LSTATUS	nResult ;
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegCreateKeyExW
			( hKey, pszSubKey, NULL, L"", REG_OPTION_NON_VOLATILE,
						samDersired, NULL, &m_hKey, &dwDisposition ) ;
	}
	else
	{
		SArray<char>	bufSubKey ;
		const char *	pcsSubKey =
				SString(pszSubKey).EncodeDefaultTo( bufSubKey ) ;
		//
		nResult = ::RegCreateKeyExA
			( hKey, pcsSubKey,
				NULL, "", REG_OPTION_NON_VOLATILE,
				samDersired, NULL, &m_hKey, &dwDisposition ) ;
	}
	if ( nResult == ERROR_SUCCESS )
	{
		m_fOpened = true ;
		return	errSuccess ;
	}
	return	errFailed ;
}

// レジストリキーを削除
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::DeleteKey( HKEY hKey, const wchar_t * pszSubKey )
{
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		if ( ::RegDeleteKeyW( hKey, pszSubKey ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	else
	{
		SArray<char>	bufSubKey ;
		const char *	pcsSubKey =
				SString(pszSubKey).EncodeDefaultTo( bufSubKey ) ;
		//
		if ( ::RegDeleteKeyA( hKey, pcsSubKey ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	return	errFailed ;
}

// レジストリキーを開く
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::OpenKey
	( HKEY hKey, const wchar_t * pszSubKey, REGSAM samDersired )
{
	CloseKey( ) ;

	LSTATUS	nResult ;
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegOpenKeyExW
			( hKey, pszSubKey, 0, samDersired, &m_hKey ) ;
	}
	else
	{
		SArray<char>	bufSubKey ;
		const char *	pcsSubKey =
				SString(pszSubKey).EncodeDefaultTo( bufSubKey ) ;
		//
		nResult = ::RegOpenKeyExA
			( hKey, pcsSubKey, 0, samDersired, &m_hKey ) ;
	}
	if ( nResult == ERROR_SUCCESS )
	{
		m_fOpened = true ;
		return	errSuccess ;
	}
	return	errFailed ;
}

// レジストリキーを閉じる
//////////////////////////////////////////////////////////////////////////////
void SRegistryKey::CloseKey( void )
{
	if ( m_fOpened )
	{
		::RegCloseKey( m_hKey ) ;
		m_fOpened = false ;
	}
}

// 下層キーを列挙する
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::EnumerateSubKeys( SObjectArray<SString>& lstKeyNames ) const
{
	DWORD	dwIndex = 0 ;
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		SArray<wchar_t>	bufName ;
		const size_t	nBufLength = 0x400 ;
		wchar_t *		pwcNameBuf = bufName.GetArray( nBufLength ) ;
		//
		for ( ; ; )
		{
			DWORD	dwNameLen = (DWORD) nBufLength ;
			if ( ::RegEnumKeyExW
				( m_hKey, dwIndex,
					pwcNameBuf, &dwNameLen,
					NULL, NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				break ;
			}
			lstKeyNames.Add( new SString( pwcNameBuf ) ) ;
			dwIndex ++ ;
		}
		bufName.FinishArray() ;
	}
	else
	{
		SArray<char>	bufName ;
		const size_t	nBufLength = 0x400 ;
		char *			pcNameBuf = bufName.GetArray( nBufLength ) ;
		//
		for ( ; ; )
		{
			DWORD	dwNameLen = (DWORD) nBufLength ;
			if ( ::RegEnumKeyExA
				( m_hKey, dwIndex,
					pcNameBuf, &dwNameLen,
					NULL, NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				break ;
			}
			lstKeyNames.Add( new SString( pcNameBuf ) ) ;
			dwIndex ++ ;
		}
		bufName.FinishArray() ;
	}
	return	errSuccess ;
}

// 値名を列挙する
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::EnumerateValueNames( SObjectArray<SString>& lstValueNames ) const
{
	DWORD	dwIndex = 0 ;
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		SArray<wchar_t>	bufName ;
		const size_t	nBufLength = 0x400 ;
		wchar_t *		pwcNameBuf = bufName.GetArray( nBufLength ) ;
		//
		for ( ; ; )
		{
			DWORD	dwNameLen = (DWORD) nBufLength ;
			DWORD	dwType ;
			if ( ::RegEnumValueW
				( m_hKey, dwIndex,
					pwcNameBuf, &dwNameLen,
					NULL, &dwType, NULL, NULL ) != ERROR_SUCCESS )
			{
				break ;
			}
			lstValueNames.Add( new SString( pwcNameBuf ) ) ;
			dwIndex ++ ;
		}
		bufName.FinishArray() ;
	}
	else
	{
		SArray<char>	bufName ;
		const size_t	nBufLength = 0x400 ;
		char *			pcNameBuf = bufName.GetArray( nBufLength ) ;
		//
		for ( ; ; )
		{
			DWORD	dwNameLen = (DWORD) nBufLength ;
			DWORD	dwType ;
			if ( ::RegEnumValueA
				( m_hKey, dwIndex,
					pcNameBuf, &dwNameLen,
					NULL, &dwType, NULL, NULL ) != ERROR_SUCCESS )
			{
				break ;
			}
			lstValueNames.Add( new SString( pcNameBuf ) ) ;
			dwIndex ++ ;
		}
		bufName.FinishArray() ;
	}
	return	errSuccess ;
}

// 値を削除する
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::DeleteValue( const wchar_t * pszValueName )
{
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		if ( ::RegDeleteValueW( m_hKey, pszValueName ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	else
	{
		const char *	pcsName = NULL ;
		SArray<char>	bufName ;
		if ( pszValueName != NULL )
		{
			pcsName = SString(pszValueName).EncodeDefaultTo( bufName ) ;
		}
		if ( ::RegDeleteValueA( m_hKey, pcsName ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	return	errFailed ;
}

// 値をセットする
//////////////////////////////////////////////////////////////////////////////
SError SRegistryKey::SetBinary
	( const wchar_t * pszValueName, const void * ptrData, size_t nBytes )
{
	return	SetValue( pszValueName, REG_BINARY, ptrData, nBytes ) ;
}

SError SRegistryKey::SetInteger( const wchar_t * pszValueName, int32_t nInteger )
{
	return	SetValue( pszValueName, REG_DWORD, &nInteger, sizeof(DWORD) ) ;
}

SError SRegistryKey::SetLargeInteger( const wchar_t * pszValueName, int64_t nInteger )
{
	return	SetValue( pszValueName, REG_QWORD, &nInteger, sizeof(int64_t) ) ;
}

SError SRegistryKey::SetString( const wchar_t * pszValueName, const wchar_t * pszString )
{
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		return	SetValue
					( pszValueName, REG_SZ,
						pszString, (SString::GetLength(pszString) + 1) * sizeof(wchar_t) ) ;
	}
	else
	{
		SArray<uint8_t>	strValue ;
		Charset::Encode
			( strValue, Charset::encodingShiftJIS, pszString ) ;
		strValue.Add( 0 ) ;
		//
		return	SetValue
			( pszValueName, REG_SZ,
				strValue.GetConstArray(), strValue.GetLength() ) ;
	}
}

SError SRegistryKey::SetDoubleReal( const wchar_t * pszValueName, double nReal )
{
	return	SetValue( pszValueName, REG_BINARY, &nReal, sizeof(double) ) ;
}

SError SRegistryKey::SetMultiStrings
	( const wchar_t * pszValueName,
		const SObjectArray<SRegistryKey::MultiString> & lstMultiStrings )
{
	SString	strMultiSZ ;
	for ( size_t i = 0; i < lstMultiStrings.GetLength(); i ++ )
	{
		MultiString *	pms = lstMultiStrings.GetAt( i ) ;
		if ( pms == NULL )
		{
			continue ;
		}
		for ( size_t j = 0; j < pms->GetLength(); j ++ )
		{
			SString *	pstr = pms->GetAt( i ) ;
			if ( pstr != NULL )
			{
				strMultiSZ += *pstr ;
				strMultiSZ += L'\0' ;
			}
		}
		strMultiSZ += L'\0' ;
	}
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		return	SetValue
					( pszValueName, REG_MULTI_SZ,
						(const wchar_t *) strMultiSZ,
						strMultiSZ.GetLength() * sizeof(wchar_t) ) ;
	}
	else
	{
		SArray<uint8_t>	strValue ;
		Charset::Encode
			( strValue, Charset::encodingShiftJIS,
				strMultiSZ, (ssize_t) strMultiSZ.GetLength() ) ;
		//
		return	SetValue
					( pszValueName, REG_MULTI_SZ,
						strValue.GetConstArray(), strValue.GetLength() ) ;
	}
}

SError SRegistryKey::SetValue
	( const wchar_t * pszValueName,
		DWORD dwType, const void * ptrData, size_t nBytes )
{
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		if ( ::RegSetValueExW
			( m_hKey, pszValueName, 0, dwType,
				(const BYTE *) ptrData, (DWORD) nBytes ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	else
	{
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		if ( ::RegSetValueExA
			( m_hKey, pcsName, 0, dwType,
				(const BYTE *) ptrData, (DWORD) nBytes ) == ERROR_SUCCESS )
		{
			return	errSuccess ;
		}
	}
	return	errFailed ;
}

// 値を取得する
//////////////////////////////////////////////////////////////////////////////
size_t SRegistryKey::GetBinary
	( const wchar_t * pszValueName, SArray<uint8_t>& bufData ) const
{
	DWORD	dwType = REG_BINARY ;
	DWORD	dwBytes = 0x100 ;
	LSTATUS	nResult ;
	//
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegQueryValueExW
				( m_hKey, pszValueName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExW
			( m_hKey, pszValueName, NULL,
				&dwType, (LPBYTE) bufData.GetArray( dwBytes ), &dwBytes ) ;
		bufData.FinishArray() ;
	}
	else
	{
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		nResult = ::RegQueryValueExA
				( m_hKey, pcsName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExA
			( m_hKey, pcsName, NULL,
				&dwType, (LPBYTE) bufData.GetArray( dwBytes ), &dwBytes ) ;
		bufData.FinishArray() ;
	}
	if ( nResult == ERROR_SUCCESS )
	{
		return	bufData.GetLength() ;
	}
	return	0 ;
}

int32_t SRegistryKey::GetInteger( const wchar_t * pszValueName, int32_t nDefValue ) const
{
	DWORD	dwType = REG_DWORD ;
	DWORD	dwBytes = sizeof(DWORD) ;
	DWORD	dwData = 0 ;
	LSTATUS	nResult ;
	//
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegQueryValueExW
				( m_hKey, pszValueName, NULL,
						&dwType, (LPBYTE) &dwData, &dwBytes ) ;
	}
	else
	{
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		nResult = ::RegQueryValueExA
				( m_hKey, pcsName, NULL,
						&dwType, (LPBYTE) &dwData, &dwBytes ) ;
	}
	if ( nResult == ERROR_SUCCESS )
	{
		return	(int32_t) dwData ;
	}
	return	nDefValue ;
}

int64_t SRegistryKey::GetLargeInteger( const wchar_t * pszValueName, int64_t nDefValue ) const
{
	DWORD	dwType = REG_QWORD ;
	DWORD	dwBytes = sizeof(int64_t) ;
	int64_t	nData = 0 ;
	LSTATUS	nResult ;
	//
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegQueryValueExW
				( m_hKey, pszValueName, NULL,
						&dwType, (LPBYTE) &nData, &dwBytes ) ;
	}
	else
	{
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		nResult = ::RegQueryValueExA
				( m_hKey, pcsName, NULL,
						&dwType, (LPBYTE) &nData, &dwBytes ) ;
	}
	if ( nResult == ERROR_SUCCESS )
	{
		return	nData ;
	}
	return	nDefValue ;
}

SString SRegistryKey::GetString
	( const wchar_t * pszValueName, const wchar_t * pszDefString ) const
{
	DWORD	dwType = REG_SZ ;
	DWORD	dwBytes = 0x100 ;
	SString	strValue ;
	LSTATUS	nResult ;
	//
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegQueryValueExW
				( m_hKey, pszValueName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExW
			( m_hKey, pszValueName, NULL,
				&dwType, (LPBYTE) strValue.LockBuffer( dwBytes / 2 + 1 ), &dwBytes ) ;
		strValue.UnlockBuffer() ;
	}
	else
	{
		SArray<uint8_t>	bufValue ;
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		nResult = ::RegQueryValueExA
				( m_hKey, pcsName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExA
			( m_hKey, pcsName, NULL,
				&dwType, (LPBYTE) bufValue.GetArray( dwBytes ), &dwBytes ) ;
		bufValue.FinishArray() ;
		//
		if ( nResult == ERROR_SUCCESS )
		{
			Charset::Decode
				( strValue, Charset::encodingShiftJIS, bufValue.GetConstArray() ) ;
		}
	}
	if ( nResult == ERROR_SUCCESS )
	{
		return	strValue ;
	}
	return	SString( pszDefString ) ;
}

double SRegistryKey::GetDoubleReal
	( const wchar_t * pszValueName, double nDefValue ) const
{
	SArray<uint8_t>	bufData ;
	if ( GetBinary( pszValueName, bufData ) == sizeof(double) )
	{
		return	*((const double*)bufData.GetConstArray()) ;
	}
	return	nDefValue ;
}

SError SRegistryKey::GetMultiStrings
	( SObjectArray<SRegistryKey::MultiString> & lstMultiStrings,
								const wchar_t * pszValueName ) const
{
	DWORD	dwType = REG_MULTI_SZ ;
	DWORD	dwBytes = 0x100 ;
	SString	strValue ;
	LSTATUS	nResult ;
	//
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		nResult = ::RegQueryValueExW
				( m_hKey, pszValueName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExW
			( m_hKey, pszValueName, NULL,
				&dwType, (LPBYTE) strValue.LockBuffer( dwBytes / 2 + 1 ), &dwBytes ) ;
		strValue.UnlockBuffer( dwBytes / 2 ) ;
	}
	else
	{
		SArray<uint8_t>	bufValue ;
		SArray<char>	bufName ;
		const char *	pcsName =
				SString(pszValueName).EncodeDefaultTo( bufName ) ;
		//
		nResult = ::RegQueryValueExA
				( m_hKey, pcsName, NULL, &dwType, NULL, &dwBytes ) ;
		nResult = ::RegQueryValueExA
			( m_hKey, pcsName, NULL,
				&dwType, (LPBYTE) bufValue.GetArray( dwBytes ), &dwBytes ) ;
		bufValue.FinishArray() ;
		//
		if ( nResult == ERROR_SUCCESS )
		{
			Charset::Decode
				( strValue, Charset::encodingShiftJIS,
						bufValue.GetConstArray(), (ssize_t) dwBytes ) ;
		}
	}
	if ( nResult != ERROR_SUCCESS )
	{
		return	errFailed ;
	}
	const wchar_t *	pwszValue = strValue ;
	const size_t	nLength = strValue.GetLength() ;
	lstMultiStrings.RemoveAll() ;
	size_t	i = 0 ;
	while ( i < nLength )
	{
		MultiString *	pms = new MultiString ;
		while ( pwszValue[i] )
		{
			SString *	pstr = new SString( pwszValue + i ) ;
			pms->Add( pstr ) ;
			i += pstr->GetLength() + 1 ;
		}
		lstMultiStrings.Add( pms ) ;
		i ++ ;
	}
	return	errSuccess ;
}

