
#include <sakura/sakura.h>

#if	!defined(__COTOPHA__)
#include <stdio.h>

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <wchar.h>
#endif
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 文字列
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SString )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SString::SString( int64_t nValue, int nPrec, int nRadix )
{
	FromInteger( nValue, nPrec, nRadix ) ;
}

SString::SString( double nValue, int nPrec )
{
	FromReal( nValue, nPrec ) ;
}

// 内部バッファ
//////////////////////////////////////////////////////////////////////////////
uint16_t * SString::LockBuffer( size_t nBufSize )
{
	SArray<uint16_t>::SetLimit( nBufSize + 1 ) ;
	return	m_ptrArray ;
}

void SString::UnlockBuffer( ssize_t nLength )
{
	ESLAssert( m_nBufSize >= 1 ) ;
	if ( nLength < 0 )
	{
		nLength = (ssize_t) m_nBufSize - 1 ;
		for ( ssize_t i = 0; i < nLength; i ++ )
		{
			if ( m_ptrArray[i] == 0 )
			{
				nLength = i ;
				break ;
			}
		}
	}
	else if ( (size_t) nLength >= m_nBufSize )
	{
		nLength = (ssize_t) m_nBufSize - 1 ;
	}
	m_nLength = (uint32_t) nLength ;
	m_ptrArray[nLength] = 0 ;
	//
	OnMofiiedString() ;
}

void SString::SetLength( size_t nSize )
{
	SArray<uint16_t>::SetLimit( nSize + 1 ) ;
	m_nLength = (uint32_t) nSize ;
	m_ptrArray[nSize] = 0 ;
	//
	OnMofiiedString() ;
}

void SString::FreeArray( void )
{
	SArray<uint16_t>::FreeArray() ;
	OnMofiiedString() ;
}

#if	!defined(__WCHAR_EQU_UINT16__)
const wchar_t * SString::GetWideCharArray( void )
{
	if ( m_ptrArray == NULL )
	{
		return	NULL ;
	}
	const int	lenStr = m_nLength ;
	uint16_t *	pwSrc = m_ptrArray ;
	wchar_t *	pwTemp = m_wstrTemp.GetArray( lenStr + 1 ) ;
	for ( int i = 0; i < lenStr; i ++ )
	{
		pwTemp[i] = (wchar_t) pwSrc[i] ;
	}
	pwTemp[lenStr] = 0 ;
	m_wstrTemp.FinishArray() ;
	return	pwTemp ;
}

void SString::OnMofiiedString( void )
{
	m_wstrTemp.RemoveAll() ;
}

void SString::CheckCastStringBuffer( void ) const
{
#if	defined(__DEBUG__)
	ESLAssert( m_wstrTemp.GetLength() > GetLength() ) ;
	const int		lenStr = m_nLength ;
	uint16_t *		pwSrc = m_ptrArray ;
	const wchar_t *	pwTemp = m_wstrTemp.GetConstArray() ;
	for ( int i = 0; i < lenStr; i ++ )
	{
		ESLAssert( pwTemp[i] == (wchar_t) pwSrc[i] ) ;
	}
#endif
}
#endif

// 文字列長計算
//////////////////////////////////////////////////////////////////////////////
size_t SString::GetLength( const wchar_t * pwszStr )
{
	size_t	 i = 0 ;
	if ( pwszStr != NULL )
	{
		while ( pwszStr[i] != 0 )
		{
			i ++ ;
		}
	}
	return	i ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
void SString::SetString( const SString & strSrc )
{
	if ( strSrc.m_ptrArray != NULL )
	{
		const int	len = strSrc.m_nLength ;
		SArray<uint16_t>::SetLimit( len + 1 ) ;
		m_nLength = len ;
		memmove
			( m_ptrArray, strSrc.m_ptrArray,
						len * sizeof(uint16_t) ) ;
		m_ptrArray[len] = 0 ;
		OnMofiiedString() ;
	}
	else
	{
		FreeArray() ;
	}
}

#if	!defined(__COTOPHA__)
void SString::SetString( const wchar_t * pszSrc, ssize_t nLength )
{
	if ( pszSrc != NULL )
	{
		if ( nLength < 0 )
		{
			nLength = 0 ;
			if ( pszSrc != NULL )
			{
				while ( pszSrc[nLength] != 0 )
				{
					nLength ++ ;
				}
			}
		}
		if ( sizeof(wchar_t) == sizeof(uint16_t) )
		{
			SArray<uint16_t>::SetLimit( nLength + 1 ) ;
			m_nLength = (uint32_t) nLength ;
			//
			for ( ssize_t i = 0; i < nLength; i ++ )
			{
				m_ptrArray[i] = (uint16_t) pszSrc[i] ;
			}
			m_ptrArray[nLength] = 0 ;
		}
		else
		{
			SArray<uint16_t>::SetLimit( nLength * 2 + 1 ) ;
			//
			size_t	j = 0 ;
			for ( ssize_t i = 0; i < nLength; i ++ )
			{
				if ( pszSrc[i] >= 0x10000 )
				{
					uint32_t	code =
								SurrogatePairsFromUnicode( pszSrc[i] ) ;
					m_ptrArray[j ++] = (uint16_t) (code >> 16) ;
					m_ptrArray[j ++] = (uint16_t) (code & 0xFFFF) ;
				}
				else
				{
					m_ptrArray[j ++] = (uint16_t) pszSrc[i] ;
				}
			}
			ESLAssert( j < m_nBufSize ) ;
			m_nLength = (uint32_t) j ;
			m_ptrArray[j] = 0 ;
		}
		OnMofiiedString() ;
	}
	else
	{
		FreeArray() ;
	}
}
#endif

#if	!defined(__WCHAR_EQU_UINT16__)
void SString::SetString( const uint16_t * pszSrc, ssize_t nLength )
{
	if ( pszSrc != NULL )
	{
		if ( nLength < 0 )
		{
			nLength = 0 ;
			if ( pszSrc != NULL )
			{
				while ( pszSrc[nLength] != 0 )
				{
					nLength ++ ;
				}
			}
		}
		ulong_ptr_t	nPtrOffset =
			(ulong_ptr_t) pszSrc - (ulong_ptr_t) m_ptrArray ;
		if ( nPtrOffset < m_nLength * sizeof(uint16_t) )
		{
			ulong_ptr_t	nLeftLen =
				m_nBufSize - nPtrOffset / sizeof(uint16_t) ;
			if ( nLeftLen < (ulong_ptr_t) nLength + 1 )
			{
				nLength = (int) nLeftLen - 1 ;
				ESLAssert( nLength >= 0 ) ;
				if ( nLength < 0 )
				{
					SetLimit( 1 ) ;
					nLength = 0 ;
				}
			}
		}
		else
		{
			SArray<uint16_t>::SetLimit( nLength + 1 ) ;
		}
		m_nLength = (uint32_t) nLength ;
		//
		for ( ssize_t i = 0; i < nLength; i ++ )
		{
			m_ptrArray[i] = pszSrc[i] ;
		}
		m_ptrArray[nLength] = 0 ;
		OnMofiiedString() ;
	}
	else
	{
		FreeArray() ;
	}
}
#endif


// 文字コード変換 （コンパイラデフォルトの char 型文字セット）
//////////////////////////////////////////////////////////////////////////////
void SString::DecodeDefaultFrom( const char * pszSrc, ssize_t nLength )
{
	if ( pszSrc == NULL )
	{
		FreeArray() ;
		return ;
	}
	if ( nLength < 0 )
	{
		nLength = 0 ;
		while ( pszSrc[nLength] != 0 )
		{
			nLength ++ ;
		}
	}
	#if	defined(__COTOPHA__)
		SArray<uint16_t>::SetLimit( nLength + 1 ) ;
		m_nLength = nLength ;
		//
		for ( ssize_t i = 0; i < nLength; i ++ )
		{
			m_ptrArray[i] = pszSrc[i] ;
		}
		m_ptrArray[nLength] = 0 ;

	#elif	defined(__PLATFORM_WINDOWS__)
		int	nDstLen =
			::MultiByteToWideChar
				( CP_ACP, MB_PRECOMPOSED, pszSrc, (int) nLength, NULL, 0 ) ;
		SArray<uint16_t>::SetLimit( nDstLen + 1 ) ;
		m_nLength = nDstLen ;
		//
		ESLAssert( sizeof(WCHAR) == sizeof(uint16_t) ) ;
		::MultiByteToWideChar
			( CP_ACP, MB_PRECOMPOSED,
				pszSrc, (int) nLength, (LPWSTR) m_ptrArray, nDstLen ) ;
		m_ptrArray[m_nLength] = 0 ;

	#elif	defined(__PLATFORM_UNIX_LIKE__)
		ESLAssert( sizeof(char) == sizeof(BYTE) ) ;
		ssize_t	nDstLen =
			ESLCharset::DecodeFromUTF8
				( NULL, 0, (BYTE*) pszSrc, nLength ) ;
		SArray<uint16_t>::SetLimit( nDstLen + 1 ) ;
		m_nLength = nDstLen ;
		//
		wchar_t *	pwszTemp = new wchar_t[nDstLen] ;
		uint16_t *	ptrArray = SArray<uint16_t>::m_ptrArray ;
		ESLCharset::DecodeFromUTF8
			( pwszTemp, nDstLen, (BYTE*) pszSrc, nLength ) ;
		for ( ssize_t i = 0; i < nDstLen; i ++ )
		{
			ptrArray[i] = (uint16_t) pwszTemp[i] ;
		}
		delete []	pwszTemp ;
		SArray<uint16_t>::m_ptrArray[SArray<uint16_t>::m_nLength] = 0 ;

	#else
		if ( sizeof(char) == sizeof(BYTE) )
		{
			ssize_t	nDstLen =
				ESLCharset::DecodeFromUTF8
					( NULL, 0, (BYTE*) pszSrc, nLength ) ;
			SArray<uint16_t>::SetLimit( nDstLen + 1 ) ;
			m_nLength = nDstLen ;
			//
			wchar_t *	pwszTemp = new wchar_t[nDstLen] ;
			uint16_t *	ptrArray = SArray<uint16_t>::m_ptrArray ;
			ESLCharset::DecodeFromUTF8
				( pwszTemp, nDstLen, (BYTE*) pszSrc, nLength ) ;
			for ( ssize_t i = 0; i < nDstLen; i ++ )
			{
				ptrArray[i] = (uint16_t) pwszTemp[i] ;
			}
			delete []	pwszTemp ;
			SArray<uint16_t>::m_ptrArray[SArray<uint16_t>::m_nLength] = 0 ;
		}
		else
		{
			SArray<uint16_t>::SetLimit( nLength + 1 ) ;
			SArray<uint16_t>::m_nLength = nLength ;
			//
			for ( ssize_t i = 0; i < nLength; i ++ )
			{
				SArray<uint16_t>::m_ptrArray[i] = (uint16_t) pszSrc[i] ;
			}
			SArray<uint16_t>::m_ptrArray[nLength] = 0 ;
		}

	#endif

	OnMofiiedString() ;
}

const char * SString::EncodeDefaultTo( SArray<char>& strDst ) const
{
	if ( m_ptrArray == NULL )
	{
		strDst.FreeArray() ;
		return	NULL ;
	}

	#if	defined(__COTOPHA__)
		const size_t	nLength = SArray<uint16_t>::m_nLength ;
		strDst.SetLength( nLength ) ;
		strDst.SetLimit( nLength + 1 ) ;
		//
		char *		pszDst = strDst.GetArray() ;
		uint16_t *	pszSrc = m_ptrArray ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			pszDst[i] = pszSrc[i] ;
		}
		pszDst[m_nLength] = 0 ;
		strDst.FinishArray() ;

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( sizeof(WCHAR) == sizeof(uint16_t) ) ;
		int	nDstLen =
			::WideCharToMultiByte
				( CP_ACP, 0, (LPCWSTR) m_ptrArray, m_nLength, NULL, 0, NULL, NULL ) ;
		strDst.SetLength( nDstLen ) ;
		strDst.SetLimit( nDstLen + 1 ) ;
		//
		char *	pszDst = strDst.GetArray() ;
		::WideCharToMultiByte
			( CP_ACP, 0, (LPCWSTR) m_ptrArray, m_nLength,
							pszDst, nDstLen, NULL, NULL ) ;
		pszDst[nDstLen] = 0 ;
		strDst.FinishArray() ;

	#elif	defined(__PLATFORM_UNIX_LIKE__)
		ESLAssert( sizeof(char) == sizeof(BYTE) ) ;
		const size_t	nLength = SArray<uint16_t>::m_nLength ;
		wchar_t *		pwszTemp = new wchar_t[nLength] ;
		uint16_t *		ptrArray = SArray<uint16_t>::m_ptrArray ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			pwszTemp[i] = (wchar_t) ptrArray[i] ;
		}
		int	nDstLen =
			ESLCharset::EncodeToUTF8
				( pwszTemp, m_nLength, NULL, 0 ) ;
		strDst.SetLength( nDstLen ) ;
		strDst.SetLimit( nDstLen + 1 ) ;
		//
		char *	pszDst = strDst.GetArray() ;
		ESLCharset::EncodeToUTF8
			( pwszTemp, m_nLength, (BYTE*) pszDst, nDstLen ) ;
		pszDst[nDstLen] = 0 ;
		strDst.FinishArray() ;
		delete []	pwszTemp ;

	#else
		if ( sizeof(char) == sizeof(BYTE) )
		{
			const size_t	nLength = SArray<uint16_t>::m_nLength ;
			wchar_t *		pwszTemp = new wchar_t[nLength] ;
			uint16_t *		ptrArray = SArray<uint16_t>::m_ptrArray ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				pwszTemp[i] = (wchar_t) ptrArray[i] ;
			}
			int	nDstLen =
				ESLCharset::EncodeToUTF8
					( pwszTemp, m_nLength, NULL, 0 ) ;
			strDst.SetLength( nDstLen ) ;
			strDst.SetLimit( nDstLen + 1 ) ;
			//
			char *	pszDst = strDst.GetArray() ;
			ESLCharset::EncodeToUTF8
				( pwszTemp, m_nLength, (BYTE*) pszDst, nDstLen ) ;
			pszDst[nDstLen] = 0 ;
			strDst.FinishArray() ;
			delete []	pwszTemp ;
		}
		else
		{
			const size_t	nLength = SArray<uint16_t>::m_nLength ;
			strDst.SetLength( nLength ) ;
			strDst.SetLimit( nLength + 1 ) ;
			//
			char *			pszDst = strDst.GetArray() ;
			uint16_t *		pszSrc = SArray<uint16_t>::m_ptrArray ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				pszDst[i] = (char) pszSrc[i] ;
			}
			pszDst[nLength] = 0 ;
			strDst.FinishArray() ;
		}

	#endif
	return	strDst ;
}


// エンコーディング変換
//////////////////////////////////////////////////////////////////////////////
SArray<uint8_t> SString::ToUTF8( void ) const
{
	const wchar_t *	pwszThis = *this ;
	unsigned int	nDstLen =
		ESLCharset::EncodeToUTF8
			( pwszThis, m_nLength, nullptr, 0 ) ;
	//
	SArray<uint8_t>	strDst ;
	uint8_t *	pszDst = strDst.GetArray( nDstLen ) ;
	ESLCharset::EncodeToUTF8
		( pwszThis, m_nLength, (BYTE*) pszDst, nDstLen ) ;
	strDst.FinishArray() ;
	//
	return	strDst ;
}

SArray<uint32_t> SString::ToUTF32( void ) const
{
	const uint16_t *	pwszThis = SArray<uint16_t>::m_ptrArray ;
	const size_t		nLength = SArray<uint16_t>::m_nLength ;
	//
	SArray<uint32_t>	strDst ;
	uint32_t *	pucs4Dst = strDst.GetArray( nLength ) ;
	size_t		iDst = 0 ;
	for ( size_t iSrc = 0; iSrc < nLength; iSrc ++ )
	{
		if ( (iSrc + 1 < nLength)
			&& IsHighSurrogateCode(pwszThis[iSrc])
			&& IsLowSurrogateCode(pwszThis[iSrc + 1]) )
		{
			pucs4Dst[iDst ++] =
				UnicodeFromSurrogatePairs
					( pwszThis[iSrc], pwszThis[iSrc + 1] ) ;
			iSrc ++ ;
		}
		else
		{
			pucs4Dst[iDst ++] = pwszThis[iSrc] ;
		}
	}
	ESLAssert( iDst <= nLength ) ;
	strDst.FinishArray() ;
	strDst.SetLength( iDst ) ;
	//
	return	strDst ;
}

void SString::FromUTF8( const uint8_t * pszUTF8, ssize_t nLength )
{
	if ( nLength < 0 )
	{
		nLength = 0 ;
		if ( pszUTF8 != nullptr )
		{
			while ( pszUTF8[nLength] != 0 )
			{
				nLength ++ ;
			}
		}
	}
	ssize_t	nDstLen =
		ESLCharset::DecodeFromUTF8
			( nullptr, 0, (BYTE*) pszUTF8, (unsigned int) nLength ) ;
	SArray<uint16_t>::SetLimit( nDstLen + 1 ) ;
	SArray<uint16_t>::m_nLength = (uint32_t) nDstLen ;
	//
	wchar_t *	pwszTemp = new wchar_t[nDstLen] ;
	uint16_t *	ptrArray = SArray<uint16_t>::m_ptrArray ;
	ESLCharset::DecodeFromUTF8
		( pwszTemp, (unsigned int) nDstLen,
			(BYTE*) pszUTF8, (unsigned int) nLength ) ;
	for ( ssize_t i = 0; i < nDstLen; i ++ )
	{
		ptrArray[i] = (uint16_t) pwszTemp[i] ;
	}
	delete []	pwszTemp ;
	SArray<uint16_t>::m_ptrArray[SArray<uint16_t>::m_nLength] = 0 ;
}

void SString::FromUTF32( const uint32_t * pszUTF32, ssize_t nLength )
{
	if ( nLength < 0 )
	{
		nLength = 0 ;
		if ( pszUTF32 != nullptr )
		{
			while ( pszUTF32[nLength] != 0 )
			{
				nLength ++ ;
			}
		}
	}
	size_t	nDstLength = nLength ;
	for ( size_t i = 0; i < (size_t) nLength; i ++ )
	{
		if ( pszUTF32[i] >= 0x10000 )
		{
			nDstLength ++ ;
		}
	}
	SArray<uint16_t>::SetLimit( nDstLength + 1 ) ;
	SArray<uint16_t>::m_nLength = (uint32_t) nDstLength ;
	//
	uint16_t *	ptrArray = SArray<uint16_t>::m_ptrArray ;
	size_t		iDst = 0 ;
	for ( size_t iSrc = 0; iSrc < (size_t) nLength; iSrc ++ )
	{
		if ( pszUTF32[iSrc] >= 0x10000 )
		{
			uint32_t	pairs = SurrogatePairsFromUnicode( pszUTF32[iSrc] ) ;
			ptrArray[iDst ++] = (uint16_t) (pairs >> 16) ;
			ptrArray[iDst ++] = (uint16_t) (pairs & 0xFFFF) ;
		}
		else
		{
			ptrArray[iDst ++] = (uint16_t) pszUTF32[iSrc] ;
		}
	}
	ESLAssert( iDst == SArray<uint16_t>::m_nLength ) ;
	ptrArray[iDst] = 0 ;
}

// サロゲートペア判定
//////////////////////////////////////////////////////////////////////////////
bool SString::IsHighSurrogateCode( uint16_t code )
{
	return	(code >= 0xD800) & (code < 0xDC00) ;
}

bool SString::IsLowSurrogateCode( uint16_t code )
{
	return	(code >= 0xDC00) & (code < 0xE000) ;
}

uint32_t SString::UnicodeFromSurrogatePairs
					( uint16_t codeHigh, uint16_t codeLow )
{
	codeHigh -= 0xD800 ;
	codeLow -= 0xDC00 ;
	return	0x10000 + (codeHigh << 10) + codeLow ;
}

uint32_t SString::SurrogatePairsFromUnicode( uint32_t code )
{
	code -= 0x10000 ;
	//
	uint32_t	codeHigh = 0xD800 + (code >> 10) ;
	uint32_t	codeLow = 0xDC00 + (code & 0x3FF) ;
	//
	return	(codeHigh << 16) | codeLow ;
}

// 加算
//////////////////////////////////////////////////////////////////////////////
const SString & SString::operator += ( const SString & strAdd )
{
	if ( strAdd.m_nLength > 0 )
	{
		SArray<uint16_t>::SetLimit( m_nLength + strAdd.m_nLength + 1 ) ;
		//
		memmove
			( m_ptrArray + m_nLength,
				strAdd.m_ptrArray,
				strAdd.m_nLength * sizeof(uint16_t) ) ;
		//
		m_nLength += strAdd.m_nLength ;
		m_ptrArray[m_nLength] = 0 ;
		//
		OnMofiiedString() ;
	}
	return	*this ;
}

#if	!defined(__COTOPHA__)
const SString & SString::operator += ( const wchar_t * pszAdd )
{
	if ( pszAdd != NULL )
	{
		int	lenAdd = 0 ;
		while ( pszAdd[lenAdd] != 0 )
		{
			lenAdd ++ ;
		}
		SArray<uint16_t>::SetLimit( m_nLength + lenAdd + 1 ) ;
		//
		uint16_t *	pszDst = m_ptrArray + m_nLength ;
		for ( int i = 0; i < lenAdd; i ++ )
		{
			pszDst[i] = (uint16_t) pszAdd[i] ;
		}
		pszDst[lenAdd] = 0 ;
		//
		m_nLength += lenAdd ;
		//
		OnMofiiedString() ;
	}
	return	*this ;
}

const SString & SString::operator += ( wchar_t chAdd )
{
	SArray<uint16_t>::SetLimit( m_nLength + 2 ) ;
	m_ptrArray[m_nLength ++] = (uint16_t) chAdd ;
	m_ptrArray[m_nLength] = 0 ;
	OnMofiiedString() ;
	return	*this ;
}
#endif

const SString & SString::operator += ( const char * pszAdd )
{
	if ( pszAdd != NULL )
	{
		#if	defined(__COTOPHA__)
			size_t	nLength = 0 ;
			while ( pszAdd[nLength] != 0 )
			{
				nLength ++ ;
			}
			SArray<uint16_t>::SetLimit( m_nLength + nLength + 1 ) ;
			//
			memmove
				( m_ptrArray + m_nLength,
					pszAdd, nLength * sizeof(uint16_t) ) ;
			//
			m_nLength += nLength ;
			m_ptrArray[m_nLength] = 0 ;
		#else
			SString	strAdd = pszAdd ;
			SArray<uint16_t>::SetLimit( m_nLength + strAdd.m_nLength + 1 ) ;
			//
			memmove
				( m_ptrArray + m_nLength,
					strAdd.m_ptrArray,
					strAdd.m_nLength * sizeof(uint16_t) ) ;
			//
			m_nLength += strAdd.m_nLength ;
			m_ptrArray[m_nLength] = 0 ;
		#endif
		OnMofiiedString() ;
	}
	return	*this ;
}

const SString & SString::operator += ( char chAdd )
{
	SArray<uint16_t>::SetLimit( m_nLength + 2 ) ;
	m_ptrArray[m_nLength ++] = chAdd ;
	m_ptrArray[m_nLength] = 0 ;
	OnMofiiedString() ;
	return	*this ;
}

SString SString::operator + ( const SString & strAdd ) const
{
	SString	strTemp = *this ;
	strTemp += strAdd ;
	return	strTemp ;
}

#if	!defined(__COTOPHA__)
SString SString::operator + ( const wchar_t * pszAdd ) const
{
	SString	strTemp = *this ;
	strTemp += pszAdd ;
	return	strTemp ;
}
#endif

SString SString::operator + ( const char * pszAdd ) const
{
	SString	strTemp = *this ;
	strTemp += pszAdd ;
	return	strTemp ;
}


// 積
//////////////////////////////////////////////////////////////////////////////
SString SString::operator * ( int nCount ) const
{
	SString	strTemp = *this ;
	strTemp.Multiple( nCount ) ;
	return	strTemp ;
}


// 文字列比較
//////////////////////////////////////////////////////////////////////////////
int SString::Compare( const SString & strSrc ) const
{
	const int	len = esl_min( (int) m_nLength, (int) strSrc.m_nLength ) ;
	uint16_t *	pszSrc1 = m_ptrArray ;
	uint16_t *	pszSrc2 = strSrc.m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		int	c = (int) pszSrc1[i] - (int) pszSrc2[i] ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	(int) m_nLength - (int) strSrc.m_nLength ;
}

int SString::CompareNoCase( const SString & strSrc ) const
{
	const int	len = esl_min( (int) m_nLength, (int) strSrc.m_nLength ) ;
	uint16_t *	pszSrc1 = m_ptrArray ;
	uint16_t *	pszSrc2 = strSrc.m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		uint32_t	ch1 = pszSrc1[i] ;
		uint32_t	ch2 = pszSrc2[i] ;
		#if	!defined(__COTOPHA__)
			if ( ('a' <= ch1) & (ch1 <= 'z') )
			{
				ch1 += 'A' - 'a' ;
			}
			if ( ('a' <= ch2) & (ch2 <= 'z') )
			{
				ch2 += 'A' - 'a' ;
			}
		#else
			ch1 += ('A' - 'a') & (('a' <= ch1) & (ch1 <= 'z')) ;
			ch2 += ('A' - 'a') & (('a' <= ch2) & (ch2 <= 'z')) ;
		#endif
		int	c = (int) ch1 - (int) ch2 ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	(int) m_nLength - (int) strSrc.m_nLength ;
}


#if	!defined(__COTOPHA__)
int SString::Compare
	( const wchar_t * pszStr1, const wchar_t * pszStr2 )
{
	if ( pszStr2 == NULL )
	{
		if ( pszStr1 == NULL )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( pszStr1 == NULL )
	{
		return	pszStr2[0] ? -1 : 0 ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr1 ++) ;
		int	ch2 = *(pszStr2 ++) ;
		int	c = ch1 - ch2 ;
		if ( (c != 0) | (ch1 == 0) | (ch2 == 0) )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::CompareNoCase
	( const wchar_t * pszStr1, const wchar_t * pszStr2 )
{
	if ( pszStr2 == NULL )
	{
		if ( pszStr1 == NULL )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( pszStr1 == NULL )
	{
		return	pszStr2[0] ? -1 : 0 ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr1 ++) ;
		int	ch2 = *(pszStr2 ++) ;
		if ( ('a' <= ch1) & (ch1 <= 'z') )
		{
			ch1 += 'A' - 'a' ;
		}
		if ( ('a' <= ch2) & (ch2 <= 'z') )
		{
			ch2 += 'A' - 'a' ;
		}
		int	c = ch1 - ch2 ;
		if ( (c != 0) | (ch1 == 0) | (ch2 == 0) )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::Compare( const wchar_t * pszStr ) const
{
	if ( pszStr == NULL )
	{
		if ( m_nLength == 0 )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( m_ptrArray == NULL )
	{
		return	pszStr[0] ? -1 : 0 ;
	}
	const uint16_t *	pszThis = m_ptrArray ;
	const int			lenThis = m_nLength ;
	if ( lenThis > 0 )
	{
		int	countLoop = lenThis ;
		do
		{
			int	c = (int) *(pszThis ++) - (int) *(pszStr ++) ;
			if ( c != 0 )
			{
				return	c ;
			}
		}
		while ( (-- countLoop) != 0 ) ;
	}
	return	(*pszStr != 0) ? -1 : 0 ;
}

int SString::CompareNoCase( const wchar_t * pszStr ) const
{
	if ( pszStr == NULL )
	{
		if ( m_nLength == 0 )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( m_ptrArray == NULL )
	{
		return	pszStr[0] ? -1 : 0 ;
	}
	const uint16_t *	pszThis = m_ptrArray ;
	const int			lenThis = m_nLength ;
	if ( lenThis > 0 )
	{
		int	countLoop = lenThis ;
		do
		{
			uint32_t	ch1 = *(pszThis ++) ;
			uint32_t	ch2 = *(pszStr ++) ;
			if ( ('a' <= ch1) & (ch1 <= 'z') )
			{
				ch1 += 'A' - 'a' ;
			}
			if ( ('a' <= ch2) & (ch2 <= 'z') )
			{
				ch2 += 'A' - 'a' ;
			}
			int	c = (int) ch1 - (int) ch2 ;
			if ( c != 0 )
			{
				return	c ;
			}
		}
		while ( (-- countLoop) != 0 ) ;
	}
	return	(*pszStr != 0) ? -1 : 0 ;
}
#endif

int SString::Compare
	( const char * pszStr1, const char * pszStr2 )
{
	if ( pszStr2 == NULL )
	{
		if ( pszStr1 == NULL )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( pszStr1 == NULL )
	{
		return	pszStr2[0] ? -1 : 0 ;
	}
	for ( ; ; )
	{
		char	ch1 = *(pszStr1 ++) ;
		char	ch2 = *(pszStr2 ++) ;
		int	c = (int) ch1 - (int) ch2 ;
		if ( (c != 0) | (ch1 == 0) | (ch2 == 0) )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::CompareNoCase
	( const char * pszStr1, const char * pszStr2 )
{
	if ( pszStr2 == NULL )
	{
		if ( pszStr1 == NULL )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( pszStr1 == NULL )
	{
		return	pszStr2[0] ? -1 : 0 ;
	}
	for ( ; ; )
	{
		char	ch1 = *(pszStr1 ++) ;
		char	ch2 = *(pszStr2 ++) ;
		if ( ('a' <= ch1) & (ch1 <= 'z') )
		{
			ch1 += 'A' - 'a' ;
		}
		if ( ('a' <= ch2) & (ch2 <= 'z') )
		{
			ch2 += 'A' - 'a' ;
		}
		int	c = (int) ch1 - (int) ch2 ;
		if ( (c != 0) | (ch1 == 0) | (ch2 == 0) )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::Compare( const char * pszStr ) const
{
	if ( pszStr == NULL )
	{
		if ( m_nLength == 0 )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( m_ptrArray == NULL )
	{
		return	pszStr[0] ? -1 : 0 ;
	}
	const uint16_t *	pszThis = m_ptrArray ;
	const int			lenThis = m_nLength ;
	if ( lenThis > 0 )
	{
		int	countLoop = lenThis ;
		do
		{
			int	c = (int) *(pszThis ++) - (int) *(pszStr ++) ;
			if ( c != 0 )
			{
				return	c ;
			}
		}
		while ( (-- countLoop) != 0 ) ;
	}
	return	(*pszStr != 0) ? -1 : 0 ;
}

int SString::CompareNoCase( const char * pszStr ) const
{
	if ( pszStr == NULL )
	{
		if ( m_nLength == 0 )
		{
			return	0 ;
		}
		return	1 ;
	}
	else if ( m_ptrArray == NULL )
	{
		return	pszStr[0] ? -1 : 0 ;
	}
	const uint16_t *	pszThis = m_ptrArray ;
	const int			lenThis = m_nLength ;
	if ( lenThis > 0 )
	{
		int	countLoop = lenThis ;
		do
		{
			uint32_t	ch1 = *(pszThis ++) ;
			uint32_t	ch2 = *(pszStr ++) ;
			#if	!defined(__COTOPHA__)
				if ( ('a' <= ch1) & (ch1 <= 'z') )
				{
					ch1 += 'A' - 'a' ;
				}
				if ( ('a' <= ch2) & (ch2 <= 'z') )
				{
					ch2 += 'A' - 'a' ;
				}
			#else
				ch1 += ('A' - 'a') & (('a' <= ch1) & (ch1 <= 'z')) ;
				ch2 += ('A' - 'a') & (('a' <= ch2) & (ch2 <= 'z')) ;
			#endif
			int	c = (int) ch1 - (int) ch2 ;
			if ( c != 0 )
			{
				return	c ;
			}
		}
		while ( (-- countLoop) != 0 ) ;
	}
	return	(*pszStr != 0) ? -1 : 0 ;
}

int SString::CompareLeft
	( const wchar_t * pszStr, const wchar_t * pszLeft )
{
	if ( pszLeft == NULL )
	{
		return	0 ;
	}
	else if ( pszStr == NULL )
	{
		return	- (int) pszLeft[0] ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr ++) ;
		int	ch2 = *(pszLeft ++) ;
		if ( ch2 == 0 )
		{
			break ;
		}
		int	c = ch1 - ch2 ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::CompareLeftNoCase
	( const wchar_t * pszStr, const wchar_t * pszLeft )
{
	if ( pszLeft == NULL )
	{
		return	0 ;
	}
	else if ( pszStr == NULL )
	{
		return	- (int) pszLeft[0] ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr ++) ;
		int	ch2 = *(pszLeft ++) ;
		if ( ch2 == 0 )
		{
			break ;
		}
		if ( ('a' <= ch1) & (ch1 <= 'z') )
		{
			ch1 += 'A' - 'a' ;
		}
		if ( ('a' <= ch2) & (ch2 <= 'z') )
		{
			ch2 += 'A' - 'a' ;
		}
		int	c = ch1 - ch2 ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::CompareLeft( const wchar_t * pszLeft ) const
{
	if ( pszLeft == NULL )
	{
		return	0 ;
	}
	const uint16_t *	pszStr = m_ptrArray ;
	if ( pszStr == NULL )
	{
		return	- (int) pszLeft[0] ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr ++) ;
		int	ch2 = *(pszLeft ++) ;
		if ( ch2 == 0 )
		{
			break ;
		}
		int	c = ch1 - ch2 ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	0 ;
}

int SString::CompareLeftNoCase( const wchar_t * pszLeft ) const
{
	if ( pszLeft == NULL )
	{
		return	0 ;
	}
	const uint16_t *	pszStr = m_ptrArray ;
	if ( pszStr == NULL )
	{
		return	- (int) pszLeft[0] ;
	}
	for ( ; ; )
	{
		int	ch1 = *(pszStr ++) ;
		int	ch2 = *(pszLeft ++) ;
		if ( ch2 == 0 )
		{
			break ;
		}
		if ( ('a' <= ch1) & (ch1 <= 'z') )
		{
			ch1 += 'A' - 'a' ;
		}
		if ( ('a' <= ch2) & (ch2 <= 'z') )
		{
			ch2 += 'A' - 'a' ;
		}
		int	c = ch1 - ch2 ;
		if ( c != 0 )
		{
			return	c ;
		}
	}
	return	0 ;
}


// 文字検索
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
ssize_t SString::Find( wchar_t chFind, size_t iFirst ) const
{
	const size_t	len = m_nLength ;
	for ( size_t i = iFirst; i < len; i ++ )
	{
		if ( m_ptrArray[i] == chFind )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

ssize_t SString::Find( const wchar_t * pszFind, size_t iFirst ) const
{
	const size_t	len = m_nLength ;
	for ( size_t i = iFirst; i < len; i ++ )
	{
		const uint16_t *	pszNext = m_ptrArray + i ;
		int	j = 0 ;
		for ( ; ; )
		{
			wchar_t	chFind = pszFind[j] ;
			if ( chFind == 0 )
			{
				return	(ssize_t) i ;
			}
			if ( pszNext[j] != chFind )
			{
				break ;
			}
			j ++ ;
		}
	}
	return	-1 ;
}

#endif


ssize_t SString::Find( char chFind, size_t iFirst ) const
{
	const size_t	len = m_nLength ;
	for ( size_t i = iFirst; i < len; i ++ )
	{
		if ( m_ptrArray[i] == chFind )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

ssize_t SString::Find( const char * pszFind, size_t iFirst ) const
{
	const size_t	len = m_nLength ;
	for ( size_t i = iFirst; i < len; i ++ )
	{
		const uint16_t *	pszNext = m_ptrArray + i ;
		int	j = 0 ;
		for ( ; ; )
		{
			char	chFind = pszFind[j] ;
			if ( chFind == 0 )
			{
				return	(ssize_t) i ;
			}
			if ( pszNext[j] != chFind )
			{
				break ;
			}
			j ++ ;
		}
	}
	return	-1 ;
}


// 文字設定
//////////////////////////////////////////////////////////////////////////////
void SString::SetAt( size_t index, wchar_t wch )
{
	ESLAssert( index < SArray<uint16_t>::m_nLength ) ;

	if ( index < SArray<uint16_t>::m_nLength )
	{
		SArray<uint16_t>::m_ptrArray[index] = (uint16_t) wch ;
		OnMofiiedString() ;
	}
}


// 文字列挿入・削除
//////////////////////////////////////////////////////////////////////////////
void SString::InsertAt( size_t nIndex, wchar_t wch )
{
	SArray<uint16_t>::InsertAt( nIndex, (uint16_t) wch ) ;
	//
	size_t	nLength = SArray<uint16_t>::m_nLength ;
	if ( SArray<uint16_t>::m_nBufSize <= nLength + 1 )
	{
		SetLimit( nLength + 1 + (nLength >> 1) ) ;
	}
	SArray<uint16_t>::m_ptrArray[nLength] = 0 ;
	//
	OnMofiiedString() ;
}

void SString::RemoveAt( size_t nIndex )
{
	SArray<uint16_t>::RemoveAt( nIndex ) ;
	//
	size_t	nLength = SArray<uint16_t>::m_nLength ;
	if ( SArray<uint16_t>::m_nBufSize >= nLength + 1 )
	{
		SArray<uint16_t>::m_ptrArray[nLength] = 0 ;
	}
	OnMofiiedString() ;
}


// 部分文字列取得
//////////////////////////////////////////////////////////////////////////////
SString SString::Left( size_t nCount ) const
{
	ESLAssert( (ssize_t) nCount >= 0 ) ;
	if ( nCount > m_nLength )
	{
		nCount = m_nLength ;
	}
	return	SString( m_ptrArray, (ssize_t) nCount ) ;
}

SString SString::Right( size_t nCount ) const
{
	ESLAssert( (ssize_t) nCount >= 0 ) ;
	if ( nCount > m_nLength )
	{
		nCount = m_nLength ;
	}
	return	SString( m_ptrArray + (m_nLength - nCount), (ssize_t) nCount ) ;
}

SString SString::Middle( size_t iFirst, ssize_t nCount ) const
{
	ESLAssert( (ssize_t) iFirst >= 0 ) ;
	if ( iFirst < m_nLength )
	{
		if ( nCount < 0 )
		{
			nCount = (ssize_t) (m_nLength - iFirst) ;
		}
		return	SString( m_ptrArray + iFirst, nCount ) ;
	}
	else
	{
		return	SString() ;
	}
}


// 文字置き換え
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
void SString::Replace( wchar_t chOld, wchar_t chNew )
{
	const int	len = m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		if ( pszStr[i] == chOld )
		{
			pszStr[i] = (uint16_t) chNew ;
		}
	}
	OnMofiiedString() ;
}
#endif

void SString::Replace( char chOld, char chNew )
{
	const int	len = m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		if ( pszStr[i] == chOld )
		{
			pszStr[i] = chNew ;
		}
	}
	OnMofiiedString() ;
}


// 文字列置き換え
//////////////////////////////////////////////////////////////////////////////
void SString::PrepareFilter
	( SString::FILTER_ENTRY * pFilter, size_t nCount, uint32_t nFlags )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszMinSrc = pFilter[i].pwszSource ;
		size_t			iMinSrc = i ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			const wchar_t *	pwszSrcJ = pFilter[j].pwszSource ;
			if ( Compare( pwszMinSrc, pwszSrcJ ) > 0 )
			{
				pwszMinSrc = pwszSrcJ ;
				iMinSrc = j ;
			}
		}
		if ( iMinSrc != i )
		{
			FILTER_ENTRY	feTemp = pFilter[i] ;
			pFilter[i] = pFilter[iMinSrc] ;
			pFilter[iMinSrc] = feTemp ;
		}
	}
}

SString SString::MappingFilter
	( const SString::FILTER_ENTRY * pFilter,
					size_t nCount, uint32_t nFlags ) const
{
	SString		strResult ;
	size_t		iLast = 0 ;
	size_t		nLength = (size_t) m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	//
	for ( size_t i = 0; i < nLength; i ++, pszStr ++ )
	{
		ssize_t	iFirst = 0 ;
		ssize_t	iEnd = (ssize_t) nCount - 1 ;
		while ( iFirst <= iEnd )
		{
			const ssize_t	iMiddle = (iFirst + iEnd) >> 1 ;
			const wchar_t *	pwszSrc = pFilter[iMiddle].pwszSource ;
			int				nCompare = 0 ;
			size_t			j ;
			for ( j = 0; pwszSrc[j]; j ++ )
			{
				nCompare = (int) pszStr[j] - (int) pwszSrc[j] ;
				if ( nCompare != 0 )
				{
					break ;
				}
			}
			if ( nCompare < 0 )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( nCompare > 0 )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				if ( i > iLast )
				{
					strResult += Middle( iLast, (ssize_t) (i - iLast) ) ;
				}
				strResult += pFilter[iMiddle].pwszTarget ;
				iLast  = i + j ;
				i      += j - 1 ;
				pszStr += j - 1 ;
				break ;
			}
		}
	}
	strResult += Middle( iLast ) ;
	return	strResult ;
}


// 文字列順序反転
//////////////////////////////////////////////////////////////////////////////
void SString::Reverse( void )
{
	int	iLeft = 0 ;
	int	iRight = (int) m_nLength - 1 ;
	while ( iLeft < iRight )
	{
		uint16_t	chTemp = m_ptrArray[iLeft] ;
		m_ptrArray[iLeft] = m_ptrArray[iRight] ;
		m_ptrArray[iRight] = chTemp ;
		//
		++ iLeft ;
		++ iRight ;
	}
	OnMofiiedString() ;
}

// 文字列反復
//////////////////////////////////////////////////////////////////////////////
void SString::Multiple( int nCount )
{
	if ( nCount < 0 )
	{
		Reverse() ;
		nCount = - nCount ;
	}
	else if ( nCount == 0 )
	{
		FreeArray() ;
		return ;
	}
	if ( nCount > 0 )
	{
		SArray<uint16_t>::SetLimit( (size_t) (m_nLength * nCount + 1) ) ;
		//
		for ( int i = 1; i < nCount; i ++ )
		{
			memmove
				( m_ptrArray + i * m_nLength,
					m_ptrArray, m_nLength * sizeof(uint16_t) ) ;
		}
		m_nLength *= nCount ;
		m_ptrArray[m_nLength] = 0 ;
		OnMofiiedString() ;
	}
}

// アルファベットを大文字に変換
//////////////////////////////////////////////////////////////////////////////
void SString::MakeUpper( void )
{
	const int	len = m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		uint16_t	ch = pszStr[i] ;
		if ( ('a' <= ch) & (ch <= 'z') )
		{
			pszStr[i] = ch - ('a' - 'A') ;
		}
	}
	OnMofiiedString() ;
}

// アルファベットを小文字に変換
//////////////////////////////////////////////////////////////////////////////
void SString::MakeLower( void )
{
	const int	len = m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	for ( int i = 0; i < len; i ++ )
	{
		uint16_t	ch = pszStr[i] ;
		if ( ('A' <= ch) & (ch <= 'Z') )
		{
			pszStr[i] = ch + ('a' - 'A') ;
		}
	}
	OnMofiiedString() ;
}

// 文字列の右側の空白・タブ・改行を除去
//////////////////////////////////////////////////////////////////////////////
void SString::TrimRight( void )
{
	uint16_t *	pszStr = m_ptrArray ;
	int	i = (int) m_nLength - 1 ;
	while ( i >= 0 )
	{
		if ( pszStr[i] > L' ' )
		{
			break ;
		}
		pszStr[i --] = 0 ;
	}
	m_nLength = i + 1 ;
	OnMofiiedString() ;
}

// 文字列の左側の空白・タブ・改行を除去
//////////////////////////////////////////////////////////////////////////////
void SString::TrimLeft( void )
{
	const int	lenOrg = m_nLength ;
	uint16_t *	pszStr = m_ptrArray ;
	int	iFirst = 0 ;
	while ( iFirst < lenOrg )
	{
		if ( pszStr[iFirst] > L' ' )
		{
			break ;
		}
		iFirst ++ ;
	}
	if ( iFirst > 0 )
	{
		SArray<uint16_t>::Remove( 0, iFirst ) ;
		m_ptrArray[m_nLength] = 0 ;
		OnMofiiedString() ;
	}
}

// 文字列の右側を指定文字数カット
//////////////////////////////////////////////////////////////////////////////
void SString::ChopRight( size_t nCount )
{
	if ( nCount >= m_nLength )
	{
		m_nLength = 0 ;
	}
	else
	{
		m_nLength -= (uint32_t) nCount ;
	}
	m_ptrArray[m_nLength] = 0 ;
	OnMofiiedString() ;
}

// 文字列の左側を指定文字数カット
//////////////////////////////////////////////////////////////////////////////
void SString::ChopLeft( size_t nCount )
{
	if ( nCount >= m_nLength )
	{
		nCount = m_nLength ;
		m_nLength = 0 ;
	}
	else
	{
		m_nLength -= (uint32_t) nCount ;
	}
	for ( size_t i = 0; i <= m_nLength; i ++ )
	{
		m_ptrArray[i] = m_ptrArray[nCount + i] ;
	}
	OnMofiiedString() ;
}

// 半角文字を全角文字に変換
//////////////////////////////////////////////////////////////////////////////
void SString::MakeFullWidthChars( void )
{
	size_t		nOrgLength = SArray<uint16_t>::m_nLength ;
	uint16_t *	pwStr = LockBuffer( nOrgLength ) ;
	size_t		iDst = 0 ;
	for ( size_t iSrc = 0; iSrc < nOrgLength; iSrc ++ )
	{
		wchar_t	wchFull =
			SStringParser::WideCharFromHalfWidth( (wchar_t) pwStr[iSrc] ) ;
		if ( wchFull != 0 )
		{
			if ( iSrc + 1 < nOrgLength )
			{
				if ( pwStr[iSrc + 1] == 0xFF9E /* L'ﾞ' */ )
				{
					wchar_t	wchVoiced =
						SStringParser::MakeVoicedKana( wchFull ) ;
					if ( wchVoiced != 0 )
					{
						iSrc ++ ;
						wchFull = wchVoiced ;
					}
				}
				else if ( pwStr[iSrc + 1] == 0xFF9F /* L'ﾟ' */ )
				{
					wchar_t	wchSemivoiced =
						SStringParser::MakeSemivoicedKana( wchFull ) ;
					if ( wchSemivoiced != 0 )
					{
						iSrc ++ ;
						wchFull = wchSemivoiced ;
					}
				}
			}
			pwStr[iDst ++] = (uint16_t) wchFull ;
		}
		else
		{
			pwStr[iDst ++] = pwStr[iSrc] ;
		}
	}
	UnlockBuffer( (ssize_t) iDst ) ;
}

// 全角文字を半角文字に変換
//////////////////////////////////////////////////////////////////////////////
void SString::MakeHalfWidthChars( void )
{
	size_t		nOrgLength = SArray<uint16_t>::m_nLength ;
	size_t		iExCount = 0 ;
	uint16_t *	pwStr = LockBuffer( nOrgLength ) ;
	for ( size_t i = 0; i < nOrgLength; i ++ )
	{
		wchar_t	wchStr = (wchar_t) pwStr[i] ;
		if ( (SStringParser::ClearVoicedKana(wchStr) != 0)
			|| (SStringParser::ClearSemivoicedKana(wchStr) != 0) )
		{
			iExCount ++ ;
		}
		else
		{
			wchar_t	wchHalf = SStringParser::ToHalfWidthChar( wchStr ) ;
			if ( wchHalf != 0 )
			{
				wchStr = wchHalf ;
			}
		}
		pwStr[i] = (uint16_t) wchStr ;
	}
	UnlockBuffer( (ssize_t) nOrgLength ) ;
	//
	if ( iExCount > 0 )
	{
		pwStr = LockBuffer( nOrgLength + iExCount ) ;
		for ( size_t i = 0; i < nOrgLength; i ++ )
		{
			pwStr[nOrgLength - iExCount - i - 1] = pwStr[nOrgLength - i - 1] ;
		}
		size_t	iDst = 0 ;
		for ( size_t iSrc = 0; iSrc < nOrgLength; iSrc ++ )
		{
			wchar_t	wchStr = (wchar_t) pwStr[iExCount + iSrc] ;
			wchar_t	wchKana = SStringParser::ClearVoicedKana(wchStr) ;
			if ( wchKana != 0 )
			{
				wchar_t	wchHalf = SStringParser::ToHalfWidthChar( wchKana ) ;
				ESLAssert( wchHalf != 0 ) ;
				if ( wchHalf != 0 )
				{
					pwStr[iDst ++] = (uint16_t) wchHalf ;
					pwStr[iDst ++] = 0xFF9E /* L'ﾞ' */ ;
				}
				else
				{
					pwStr[iDst ++] = (uint16_t) wchStr ;
				}
			}
			else
			{
				wchKana = SStringParser::ClearSemivoicedKana(wchStr) ;
				if ( wchKana != 0 )
				{
					wchar_t	wchHalf = SStringParser::ToHalfWidthChar( wchKana ) ;
					ESLAssert( wchHalf != 0 ) ;
					if ( wchHalf != 0 )
					{
						pwStr[iDst ++] = (uint16_t) wchHalf ;
						pwStr[iDst ++] = 0xFF9F /* L'ﾟ' */ ;
					}
					else
					{
						pwStr[iDst ++] = (uint16_t) wchStr ;
					}
				}
				else
				{
					pwStr[iDst ++] = (uint16_t) wchStr ;
				}
			}
		}
		ESLAssert( nOrgLength + iExCount == iDst ) ;
		UnlockBuffer( (ssize_t) iDst ) ;
	}
}

// ファイル名（URL）操作
//////////////////////////////////////////////////////////////////////////////
size_t SString::FindFileNamePart( wchar_t chSeparator ) const
{
	const uint16_t *	pszStr = m_ptrArray ;
	const size_t		len = m_nLength ;
	size_t				iLastSep = 0 ;
	for ( size_t i = 0; i < len; i ++ )
	{
		uint16_t	ch = pszStr[i] ;
		if ( (ch == '\\') | (ch == '/') | (ch == chSeparator) )
		{
			iLastSep = i + 1 ;
		}
	}
	return	iLastSep ;
}

size_t SString::FindFileExtensionPart( wchar_t chSeparator ) const
{
	const uint16_t *	pszStr = m_ptrArray ;
	const size_t		len = m_nLength ;
	size_t	iFileName = FindFileNamePart( chSeparator ) ;
	ssize_t	iLastExt = -1 ;
	for ( size_t i = iFileName; i < len; i ++ )
	{
		uint32_t	ch = pszStr[i] ;
		if ( ch == '.' )
		{
			iLastExt = (ssize_t) (i + 1) ;
		}
		if ( ch == 0 )
		{
			break ;
		}
	}
	if ( iLastExt < 0 )
	{
		return	len ;
	}
	return	(size_t) iLastExt ;
}

const uint16_t * SString::GetFileNamePart( wchar_t chSeparator ) const
{
	return	m_ptrArray + FindFileNamePart(chSeparator) ;
}

const uint16_t * SString::GetFileExtensionPart( wchar_t chSeparator ) const
{
	return	m_ptrArray + FindFileExtensionPart(chSeparator) ;
}

SString SString::GetFileDirectoryPart( wchar_t chSeparator ) const
{
	return	SString( m_ptrArray, (ssize_t) FindFileNamePart(chSeparator) ) ;
}

SString SString::GetFileTitlePart( wchar_t chSeparator ) const
{
	int	iFileName = (int) FindFileNamePart(chSeparator) ;
	int	iFileExt = (int) FindFileExtensionPart(chSeparator) ;
	if ( iFileName < iFileExt )
	{
		if ( m_ptrArray[iFileExt - 1] == '.' )
		{
			-- iFileExt ;
		}
		return	SString( m_ptrArray + iFileName, iFileExt - iFileName ) ;
	}
	else
	{
		return	SString( m_ptrArray + iFileName, m_nLength - iFileName ) ;
	}
}

SString SString::GetFileDrivePart( wchar_t chSeparator ) const
{
	const uint16_t *	pszStr = m_ptrArray ;
	const int			lenStr = m_nLength ;
	int	lenDrv = 0 ;
	for ( int i = 0; i < lenStr; i ++ )
	{
		uint32_t	ch = pszStr[i] ;
		if ( ch == ':' )
		{
			lenDrv = i + 1 ;
			break ;
		}
		if ( (ch == '\\') | (ch == '/') | (ch == chSeparator) )
		{
			break ;
		}
	}
	return	SString( m_ptrArray, lenDrv ) ;
}

#if	!defined(__COTOPHA__)
SString SString::OffsetFilePath
	( const wchar_t * pszOffsetPath, wchar_t chSeparator ) const
{
	if ( (pszOffsetPath == NULL) || (pszOffsetPath[0] == 0) )
	{
		return	*this ;
	}
	SString	strTemp ;
	bool	fOffsetPath = true ;
	for ( int i = 0; pszOffsetPath[i] != 0; i ++ )
	{
		wchar_t	ch = pszOffsetPath[i] ;
		if ( ch == ':' )
		{
			strTemp = pszOffsetPath ;
			fOffsetPath = false ;
			break ;
		}
		if ( (ch == '\\') | (ch == '/') | (ch == chSeparator) )
		{
			if ( i == 0 )
			{
				wchar_t	ch1 = pszOffsetPath[1] ;
				if ( (ch1 == '\\') | (ch1 == '/') | (ch1 == chSeparator) )
				{
					strTemp = pszOffsetPath ;
				}
				else
				{
					strTemp = GetFileDrivePart() ;
					if ( (ch == L'/')
						&& (Middle( strTemp.GetLength(), 2 ) == L"//") )
					{
						strTemp += L'/' ;
					}
					strTemp += pszOffsetPath ;
				}
				fOffsetPath = false ;
			}
			break ;
		}
	}
	if ( fOffsetPath )
	{
		strTemp.SetString( *this ) ;
		if ( m_nLength > 0 )
		{
			uint32_t	chLast = m_ptrArray[m_nLength - 1] ;
			if ( (chLast != '\\')
				& (chLast != '/') & (chLast != chSeparator) )
			{
				strTemp += chSeparator ;
			}
		}
		strTemp += pszOffsetPath ;
	}
	return	strTemp ;
}
#endif

SString SString::OffsetFilePath
	( const char * pszOffsetPath, char chSeparator ) const
{
	SString	strTemp ;
	if ( (pszOffsetPath != NULL) && (pszOffsetPath[0] != 0) )
	{
		const uint16_t *	pszStr = m_ptrArray ;
		const int			lenStr = m_nLength ;
		bool	fOffsetPath = true ;
		for ( int i = 0; pszOffsetPath[i] != 0; i ++ )
		{
			uint32_t	ch = pszOffsetPath[i] ;
			if ( ch == ':' )
			{
				strTemp = pszOffsetPath ;
				fOffsetPath = false ;
				break ;
			}
			if ( (ch == '\\') | (ch == '/')
				| (ch == (uint32_t) chSeparator) )
			{
				if ( i == 0 )
				{
					uint32_t	ch1 = pszOffsetPath[1] ;
					if ( (ch1 == '\\') | (ch1 == '/')
						| (ch1 == (uint32_t) chSeparator) )
					{
						strTemp = pszOffsetPath ;
					}
					else
					{
						strTemp = GetFileDrivePart() ;
						if ( (ch == '/')
							&& (Middle( strTemp.GetLength(), 2 ) == L"//") )
						{
							strTemp += L'/' ;
						}
						strTemp += pszOffsetPath ;
					}
					fOffsetPath = false ;
				}
				break ;
			}
		}
		if ( fOffsetPath )
		{
			strTemp.SetString( *this ) ;
			if ( m_nLength > 0 )
			{
				uint32_t	chLast = m_ptrArray[m_nLength - 1] ;
				if ( (chLast != '\\') & (chLast != '/')
						& (chLast != (uint32_t) chSeparator) )
				{
					strTemp += chSeparator ;
				}
			}
			strTemp += pszOffsetPath ;
		}
	}
	else
	{
		return	*this ;
	}
	return	strTemp ;
}

#if	!defined(__COTOPHA__)
SString SString::RelativeFilePath
	( const wchar_t * pszFullPath,
		size_t nAscendLimit, wchar_t chSeparator ) const
{
	if ( (pszFullPath == NULL) || (pszFullPath[0] == 0) )
	{
		return	SString() ;
	}
	size_t	iDstLast = 0 ;		// 一致するディレクトリ位置
	size_t	iSrcLast = 0 ;
	while ( pszFullPath[iDstLast] )
	{
		size_t	iEndOfDir = iDstLast ;
		for ( size_t i = iDstLast; true; i ++ )
		{
			wchar_t	wch = pszFullPath[i] ;
			if ( (wch == L'\\') || (wch == L'/')
				|| (wch == chSeparator) || (wch == 0) )
			{
				iEndOfDir = i ;
				break ;
			}
		}
		if ( iEndOfDir > GetLength() )
		{
			break ;
		}
		wchar_t	wch = GetAt( iEndOfDir ) ;
		if ( (pszFullPath[iEndOfDir] == 0)
			&& (wch == 0) && (GetLength() == iEndOfDir) )
		{
			iDstLast = iEndOfDir ;
			iSrcLast = iEndOfDir ;
			break ;
		}
		if ( (pszFullPath[iEndOfDir] == 0)
			&& ((wch == L'\\') || (wch == L'/') || (wch == chSeparator)) )
		{
			iDstLast = iEndOfDir ;
			iSrcLast = iEndOfDir + 1 ;
			break ;
		}
		if ( CompareLeft( SString( pszFullPath, (ssize_t) iEndOfDir ) )
			|| ((wch != L'\\') && (wch != L'/')
				&& (wch != chSeparator) && (wch != 0)) )
		{
			break ;
		}
		iDstLast = iEndOfDir + 1 ;
		iSrcLast = iDstLast ;
	}
	SString	strTemp ;
	size_t	iNext = iSrcLast ;
	for ( size_t i = 0; (i < nAscendLimit) && (iNext < GetLength()); i ++ )
	{
		ssize_t	iSep1 = Find( L'\\', iNext ) ;
		ssize_t	iSep2 = Find( L'/', iNext ) ;
		ssize_t	iSep3 = Find( chSeparator, iNext ) ;
		iSep1 = (iSep1 < 0) ? (ssize_t) GetLength() : iSep1 ;
		iSep2 = (iSep2 < 0) ? (ssize_t) GetLength() : iSep2 ;
		iSep3 = (iSep3 < 0) ? (ssize_t) GetLength() : iSep3 ;
		iSep1 = esl_min( (int) iSep1, esl_min( (int) iSep2, (int) iSep3 ) ) ;
		//
		strTemp += L".." ;
		strTemp += chSeparator ;
		//
		iNext = (size_t) iSep1 + 1 ;
	}
	if ( iNext < GetLength() )
	{
		return	pszFullPath ;
	}
	strTemp += pszFullPath + iDstLast ;
	return	strTemp ;
}
#endif

const SString& SString::NormalizeFilePath( wchar_t chSeparator, uint32_t nFlags )
{
	size_t		iLastDir = 0 ;
	size_t		i = 0 ;
	while ( i < m_nLength )
	{
		uint32_t	ch = m_ptrArray[i] ;
		if ( (ch == L'\\') | (ch == L'/') | (ch == chSeparator) )
		{
			if ( (iLastDir + 1 == i)
				&& (m_ptrArray[iLastDir] == L'.') )
			{
				// .\ 削除
				SArray<uint16_t>::Remove( iLastDir, 2 ) ;
				m_ptrArray[m_nLength] = 0 ;
				i = iLastDir ;
				OnMofiiedString() ;
				continue ;
			}
			else if ( (iLastDir + 2 == i)
				&& (m_ptrArray[iLastDir] == L'.')
				&& (m_ptrArray[iLastDir + 1] == L'.') )
			{
				// <parent-dir>\..\ 削除
				if ( iLastDir > 0 )
				{
					iLastDir -- ;
					while ( iLastDir > 0 )
					{
						ch = m_ptrArray[-- iLastDir] ;
						if ( (ch == L'\\') | (ch == L'/') | (ch == chSeparator) )
						{
							iLastDir ++ ;
							break ;
						}
					}
				}
				SArray<uint16_t>::Remove( iLastDir, (i + 1 - iLastDir) ) ;
				m_ptrArray[m_nLength] = 0 ;
				i = iLastDir ;
				OnMofiiedString() ;
				continue ;
			}
			else
			{
				// 次のディレクトリ
				iLastDir = i + 1 ;
			}
		}
		i ++ ;
	}
	return	*this ;
}

// 数値変換
//////////////////////////////////////////////////////////////////////////////
int SString::NumberFromChar( uint32_t ch )
{
	if ( (ch >= '0') & (ch <= '9') )
	{
		return	(ch - '0') ;
	}
	else if ( (ch >= 'A') & (ch <= 'F') )
	{
		return	(ch - ('A' - 10)) ;
	}
	else if ( (ch >= 'a') & (ch <= 'f') )
	{
		return	(ch - ('a' - 10)) ;
	}
	return	-1 ;
}

int64_t SString::AsInteger( int nRadix, bool fSign, bool * pError ) const
{
	const uint16_t *	pszStr = m_ptrArray ;
	const int			lenStr = m_nLength ;
	int64_t	nValue = 0x8000000000000000 ;
	int64_t	nSign = 0 ;
	bool	fErr = true ;
	for ( int i = 0; i < lenStr; i ++ )
	{
		uint32_t	ch = pszStr[i] ;
		int	n = NumberFromChar( ch ) ;
		if ( n >= 0 )
		{
			if ( n >= nRadix )
			{
				fErr = true ;
				break ;
			}
			nValue = nValue * nRadix + n ;
			fErr = false ;
			fSign = false ;
		}
		else if ( fSign & (ch == '-') )
		{
			fSign = false ;
			nSign = -1 ;
		}
		else if ( fSign & (ch == '+') )
		{
			fSign = false ;
			nSign = 0 ;
		}
		else if ( ch > ' ' )
		{
			fErr = true ;
			break ;
		}
	}
	if ( pError != NULL )
	{
		*pError = (fErr | (m_nLength == 0)) ;
	}
	return	(nValue ^ nSign) - nSign ;
}

double SString::AsReal( int nRadix, bool * pError ) const
{
	const uint16_t *	pszStr = m_ptrArray ;
	const int			lenStr = m_nLength ;
	double	rValue = 0.0;
	double	rExp = 0.0 ;
	double	rDecimal = 1.0 ;
	double	rSign = 1.0 ;
	double	rExpSign = 1.0 ;
	bool	fDecimal = false ;
	bool	fSign = true ;
	bool	fExp = false ;
	bool	fErr = true ;
	for ( int i = 0; i < lenStr; i ++ )
	{
		uint32_t	ch = pszStr[i] ;
		int	n = NumberFromChar( ch ) ;
		if ( (n >= 0) && (n < nRadix) )
		{
			if ( fDecimal )
			{
				if ( fExp )
				{
					rExp = rExp * nRadix + n ;
				}
				else
				{
					rDecimal /= nRadix ;
					rValue += rDecimal * n ;
				}
			}
			else
			{
				rValue = rValue * nRadix + n ;
			}
			fErr = false ;
			fSign = false ;
		}
		else if ( !fDecimal & (ch == '.') )
		{
			fDecimal = true ;
		}
		else if ( fSign & (ch == '-') )
		{
			fSign = false ;
			if ( fExp )
			{
				rExpSign = -1.0 ;
			}
			else
			{
				rSign = -1.0 ;
			}
		}
		else if ( fSign & (ch == '+') )
		{
			fSign = false ;
			if ( fExp )
			{
				rExpSign = 1.0 ;
			}
			else
			{
				rSign = 1.0 ;
			}
		}
		else if ( !fExp && ((ch == 'E') | (ch == 'e')) )
		{
			fExp = true ;
			fDecimal = true ;
			fSign = true ;
		}
		else if ( ch > ' ' )
		{
			fErr = true ;
			break ;
		}
	}
	if ( pError != NULL )
	{
		*pError = (fErr || (m_nLength == 0)) ;
	}
	if ( fExp )
	{
		return	rValue * rSign * pow( nRadix, rExp * rExpSign ) ;
	}
	else
	{
		return	rValue * rSign ;
	}
}

void SString::FromInteger( int64_t nValue, int nPrec, int nRadix )
{
	SArray<uint16_t>::SetLimit( (nPrec < 0x47) ? 0x48 : (nPrec + 1) ) ;
	uint16_t *	pszStr = m_ptrArray ;
	//
	int		i = 0, c = 1 ;
	int64_t	v = nValue, r = nRadix ;
	if ( v < 0 )
	{
		pszStr[i ++] = '-' ;
		v = - v ;
	}
	if ( nPrec <= 0 )
	{
		int64_t	t = r ;		// 桁あふれ判定用
		while ( (v >= r) & (v >= t) )
		{
			t = r ;
			r *= nRadix ;
			c ++ ;
		}
	}
	else
	{
		c = nPrec - i ;
	}
	m_nLength = i + c ;
	pszStr[m_nLength] = 0 ;
	//
	while ( c -- )
	{
		int	n = (int) (v % nRadix) ;
		if ( n < 10 )
		{
			pszStr[i + c] = (uint16_t) ('0' + n) ;
		}
		else
		{
			pszStr[i + c] = (uint16_t) ('A' + n - 10) ;
		}
		v = (v - n) / nRadix ;
	}
	OnMofiiedString() ;
}

void SString::HexFromInteger( uint64_t nValue, int nPrec )
{
	SArray<uint16_t>::SetLimit( (nPrec < 0x20) ? 0x28 : (nPrec + 1) ) ;
	uint16_t *	pszStr = m_ptrArray ;
	//
	int			i = 0, c = 1 ;
	uint64_t	r = 16 ;
	if ( nPrec <= 0 )
	{
		while ( (nValue >= r) && (c < 16) )
		{
			r <<= 4 ;
			c ++ ;
		}
	}
	else
	{
		c = nPrec - i ;
	}
	m_nLength = i + c ;
	pszStr[m_nLength] = 0 ;
	//
	while ( c -- )
	{
		int	n = (int) (nValue & 0x0F) ;
		if ( n < 10 )
		{
			pszStr[i + c] = (uint16_t) ('0' + n) ;
		}
		else
		{
			pszStr[i + c] = (uint16_t) ('A' + n - 10) ;
		}
		nValue >>= 4 ;
	}
	OnMofiiedString() ;
}

void SString::FromReal( double rValue, int nPrec )
{
	SArray<uint16_t>::SetLimit( 0x28 ) ;
	uint16_t *	pszStr = m_ptrArray ;
	//
	int		i = 0, c = 1 ;
	int64_t	r = 10 ;
	int64_t	nValue, nIntPart ;
	//
	// 整数部フォーマット
	//
	if ( rValue < 0 )
	{
		pszStr[i ++] = '-' ;
		rValue = - rValue ;
	}
	int	nDecPrec = nPrec ;
	int	nExpNum = 0 ;
	if ( nPrec <= 0 )
	{
		nPrec = 9 ;
		//
		if ( rValue != 0.0 )
		{
			double	e = floor( log10( rValue ) ) ;
			if ( (e > 9) || (e < -4) )
			{
				nExpNum = (int) e ;
				rValue *= pow( 10.0, - nExpNum ) ;
				nPrec = 12 ;
			}
		}
	}
	int64_t	d = 1 ;	
	for ( int j = 0; j < nPrec; j ++ )
	{
		d *= 10 ;
	}
	#if	defined(__COTOPHA__)
		nIntPart = nValue = (int64_t)(rValue * d) / d ;
	#else
		nIntPart = nValue = ((int64_t)(rValue * d + 0.5)) / d ;
	#endif
	if ( nValue < 0 )
	{
		pszStr[i ++] = '0' ;
		pszStr[i ++] = '.' ;
		pszStr[i ++] = '0' ;
		pszStr[i] = '\0' ;
		m_nLength = i ;
		return ;
	}
	while ( nValue >= r )
	{
		r *= 10 ;
		if ( ++ c >= 19 )
		{
			break ;
		}
	}
	m_nLength = i + c ;
	while ( c -- )
	{
		int	n = (int) (nValue % 10) ;
		pszStr[i + c] = (uint16_t) ('0' + n) ;
		nValue = (nValue - n) / 10 ;
	}
	//
	// 小数部フォーマット
	//
	pszStr[m_nLength ++] = '.' ;
	if ( nPrec > 0 )
	{
		SArray<uint16_t>::SetLimit( m_nLength + nPrec + 1 ) ;
		pszStr = m_ptrArray ;
		//
		#if	defined(__COTOPHA__)
			nValue = (int64_t)((rValue - nIntPart) * d) ;
		#else
			nValue = (int64_t)((rValue - nIntPart) * d + 0.5) ;
		#endif
		for ( i = 0; i < nPrec; i ++ )
		{
			if ( (nValue <= 0) & (nDecPrec <= 0) )
			{
				break ;
			}
			d /= 10 ;
			int	n = (int) (nValue / d) ;
			pszStr[m_nLength ++] = (uint16_t) ('0' + n) ;
			nValue -= n * d ;
		}
	}
	//
	// 指数部フォーマット
	//
	if ( nExpNum != 0 )
	{
		pszStr[m_nLength ++] = 'E' ;
		if ( nExpNum >= 0 )
		{
			pszStr[m_nLength ++] = '+' ;
		}
		else
		{
			pszStr[m_nLength ++] = '-' ;
			nExpNum = - nExpNum ;
		}
		c = 1 ;
		r = 10 ;
		while ( nExpNum >= r )
		{
			r *= 10 ;
			if ( ++ c >= 8 )
			{
				break ;
			}
		}
		i = (int) m_nLength + c - 1 ;
		m_nLength += c ;
		while ( c -- )
		{
			int	n = (int) (nExpNum % 10) ;
			pszStr[i --] = (uint16_t) ('0' + n) ;
			nExpNum = (nExpNum - n) / 10 ;
		}
	}
	pszStr[m_nLength] = 0 ;
	//
	OnMofiiedString() ;
}

// 文字列フォーマット
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SString::Format( const wchar_t * pwszFormat, ... )
{
	va_list	vl ;
	va_start( vl, pwszFormat ) ;
	return	FormatV( pwszFormat, vl ) ;
}

const wchar_t * SString::FormatV( const wchar_t * pwszFormat, va_list argptr )
{
#if	defined(__COTOPHA__)
	const int	nBufLen =
		(vsprintf_s( NULL, 0, pwszFormat, argptr ) + 0x1F) & ~0x0F ;
	int	nStrLen =
		vsprintf_s
			( LockBuffer( (size_t) nBufLen ), nBufLen, pwszFormat, argptr ) ;
	UnlockBuffer( nStrLen ) ;

#else
	if ( pwszFormat == NULL )
	{
		return	NULL ;
	}
	SStringFormatArgList	arglist ;
	arglist.AddArgumentList( pwszFormat, argptr ) ;
	//
	SStringParser	sparsFormat = pwszFormat ;
	sparsFormat.Format( *this, arglist ) ;
#endif

	OnMofiiedString() ;
	return	*this ;
}


