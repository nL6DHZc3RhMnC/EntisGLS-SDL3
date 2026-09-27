

#include <sakura/sakura.h>

using	namespace SSystem ;
using	namespace SSystem::Charset ;


//////////////////////////////////////////////////////////////////////////////
// 文字コード・エンコーディング変換
//////////////////////////////////////////////////////////////////////////////

static const wchar_t *	g_pszCharaEncodingType[] =
{
	L"shift_jis",
	L"utf-8",
	L"iso-2022-jp",
	L"euc-jp",
	L"utf-16",
	NULL
} ;

const uint8_t	SSystem::Charset::BOM_UTF8[3] = { 0xEF, 0xBB, 0xBF } ;
const uint8_t	SSystem::Charset::BOM_UTF16LE[2] = { 0xFF, 0xFE } ;

// 文字コード種別
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SSystem::Charset::GetEncodingName( EncodingType type )
{
	return	g_pszCharaEncodingType[type] ;
}

EncodingType SSystem::Charset::GetEncodingType( const wchar_t * pszType )
{
	for ( int i = 0; g_pszCharaEncodingType[i] != NULL; i ++ )
	{
		const wchar_t *	pszTypeName = g_pszCharaEncodingType[i] ;
		int	j ;
		for ( j = 0; pszTypeName[j] != 0; j ++ )
		{
			wchar_t	c1 = pszTypeName[j] ;
			wchar_t	c2 = pszType[j] ;
			if ( (L'A' <= c1) & (c1 <= L'Z') )
			{
				c1 += L'a' - L'A' ;
			}
			if ( (L'A' <= c2) & (c2 <= L'Z') )
			{
				c2 += L'a' - L'A' ;
			}
			if ( c1 != c2 )
			{
				break ;
			}
		}
		if ( pszTypeName[j] == pszType[j] )
		{
			return	(EncodingType) i ;
		}
	}
	return	encodingUnknown ;
}

// 文字コード判別
//////////////////////////////////////////////////////////////////////////////
EncodingType SSystem::Charset::AnalyzeEncoding
	( const uint8_t * pbytSrc, ssize_t nSrcLength )
{
	const int	maskShiftJIS	= 0x0001 ;
	const int	maskUTF8		= 0x0002 ;
	const int	maskISO2022JP	= 0x0004 ;
	const int	maskEUCJP		= 0x0008 ;
	const int	maskAll			= 0x000F ;
	uint32_t	nFlags = maskAll ;
	bool		fEscJIS = false ;
	bool		fSecondSJIS = false ;
	bool		fEUCFlag = false ;
	int			nUTFCount = 0 ;
	//
	if ( pbytSrc == NULL )
	{
		return	encodingUnknown ;
	}
	if ( nSrcLength < 0 )
	{
		nSrcLength = 0 ;
		while ( pbytSrc[nSrcLength] != 0 )
		{
			nSrcLength ++ ;
		}
	}
	if ( nSrcLength >= 3 )
	{
		if ( (pbytSrc[0] == 0xEF)
			& (pbytSrc[1] == 0xBB) & (pbytSrc[2] == 0xBF) )
		{
			return	encodingUTF8 ;
		}
	}
	if ( nSrcLength >= 2 )
	{
		if ( (pbytSrc[0] == 0xFF) & (pbytSrc[1] == 0xFE) )
		{
			return	encodingUTF16 ;
		}
	}
	for ( ssize_t i = 0; (nFlags != 0) & (i < nSrcLength); i ++ )
	{
		if ( (nFlags & 1) + ((nFlags >> 1) & 1)
			+ ((nFlags >> 2) & 1) + ((nFlags >> 3) & 1) <= 1 )
		{
			break ;
		}
		uint8_t	c = pbytSrc[i] ;
		if ( !(c & 0x80) )
		{
			if ( fSecondSJIS )
			{
				if ( c < 0x40 )
				{
					nFlags &= ~maskShiftJIS ;
				}
				fSecondSJIS = false ;
			}
			if ( fEUCFlag )
			{
				nFlags &= ~maskEUCJP ;
				fEUCFlag = false ;
			}
			if ( nUTFCount != 0 )
			{
				nFlags &= ~maskUTF8 ;
			}
			if ( ((nFlags & maskISO2022JP) != 0)
				&& (c == 0x1B) && (i + 2 < nSrcLength) )
			{
				if ( (pbytSrc[i + 1] == '$') && (pbytSrc[i + 2] == 'B') )
				{
					fEscJIS = true ;
				}
			}
			continue ;			// ASCII
		}
		//
		nFlags &= ~maskISO2022JP ;
		//
		if ( (c < 0xA0) && (c != 0x8E) && (c != 0x8F) )
		{
			nFlags &= ~maskEUCJP ;
		}
		else
		{
			fEUCFlag = !fEUCFlag ;
		}
		if ( fSecondSJIS )
		{
			if ( c > 0xFC )
			{
				nFlags &= ~maskShiftJIS ;
			}
			fSecondSJIS = false ;
		}
		else
		{
			if ( c >= 0xF0 )
			{
				nFlags &= ~maskShiftJIS ;
			}
			fSecondSJIS = ((c >= 0x80) && (c < 0xA0)) || (c >= 0xE0) ;
		}
		if ( nFlags & maskUTF8 )
		{
			if ( nUTFCount == 0 )
			{
				uint8_t	mask = 0x20 ;
				int		count = 1 ;
				while ( c & mask )
				{
					count ++ ;
					mask >>= 1 ;
					if ( mask == 0 )
						break ;
				}
				nUTFCount = count ;
			}
			else
			{
				nUTFCount -- ;
			}
		}
	}
	if ( (nFlags & maskISO2022JP) && fEscJIS )
	{
		return	encodingISO2022JP ;
	}
	if ( (nFlags & ~maskISO2022JP) == (maskAll & ~maskISO2022JP) )
	{
		return	encodingUTF8 ;
	}
	if ( nFlags & maskEUCJP )
	{
		return	encodingEUCJP ;
	}
	if ( nFlags & maskShiftJIS )
	{
		return	encodingShiftJIS ;
	}
	if ( nFlags & maskUTF8 )
	{
		return	encodingUTF8 ;
	}
	return	encodingUnknown ;
}

// エンコード処理
//////////////////////////////////////////////////////////////////////////////
size_t SSystem::Charset::Encode
	( SArray<uint8_t>& strDst, EncodingType type,
		const wchar_t * pwszSrc, ssize_t nSrcLength )
{
	//
	// 変換元パラメータ正規化
	//
	strDst.RemoveAll() ;
	if ( pwszSrc == NULL )
	{
		return	0 ;
	}
	if ( nSrcLength == (ssize_t) -1 )
	{
		for ( nSrcLength = 0; pwszSrc[nSrcLength] != 0; nSrcLength ++ )
		{
		}
	}
	unsigned int	nDstCount ;
	ssize_t			i ;
	WORD *			pwDst ;
	switch ( type )
	{
	case	encodingShiftJIS:
		//
		// UNICODE -> ShiftJIS
		//
		nDstCount =
			ESLCharset::UNICODEtoShiftJIS
				( pwszSrc, (unsigned int) nSrcLength, NULL, 0 ) ;
		strDst.SetLength(nDstCount) ;
		//
		nDstCount =
			ESLCharset::UNICODEtoShiftJIS
				( pwszSrc, (unsigned int) nSrcLength,
					(BYTE*) strDst.GetArray(), nDstCount ) ;
		strDst.FinishArray() ;
		strDst.SetLength(nDstCount) ;
		break ;

	case	encodingUTF8:
	default:
		//
		// UNICODE -> UTF8
		//
		nDstCount =
			ESLCharset::EncodeToUTF8
				( pwszSrc, (unsigned int) nSrcLength, NULL, 0 ) ;
		strDst.SetLength(nDstCount) ;
		//
		nDstCount =
			ESLCharset::EncodeToUTF8
				( pwszSrc, (unsigned int) nSrcLength,
					(BYTE*) strDst.GetArray(), nDstCount ) ;
		strDst.FinishArray() ;
		strDst.SetLength(nDstCount) ;
		break ;

	case	encodingISO2022JP:
		//
		// UNICODE -> ISO-2022-JP
		//
		i = 0 ;
		while ( i < nSrcLength )
		{
			wchar_t	wUnicode = pwszSrc[i] ;
			DWORD	dwJIS = ESLCharset::JISCodeFromUnicode( wUnicode ) ;
			if ( dwJIS == ESLCharset::JiscodeError )
			{
				i ++ ;
				strDst.Add( (BYTE) '?' ) ;
				continue ;
			}
			if ( dwJIS & 0xFF00 )
			{
				strDst.Add( (BYTE) 0x1B ) ;	// \x1B$B
				strDst.Add( (BYTE) '$' ) ;
				strDst.Add( (BYTE) 'B' ) ;
				do
				{
					strDst.Add( (BYTE) (dwJIS >> 8) ) ;
					strDst.Add( (BYTE) dwJIS ) ;
					if ( ++ i >= nSrcLength )
					{
						break ;
					}
					wUnicode = pwszSrc[i] ;
					dwJIS = ESLCharset::JISCodeFromUnicode( wUnicode ) ;
					if ( dwJIS == ESLCharset::JiscodeError )
					{
						break ;
					}
				}
				while ( dwJIS & 0xFF00 ) ;
				strDst.Add( (BYTE) 0x1B ) ;	// \x1B(B
				strDst.Add( (BYTE) '(' ) ;
				strDst.Add( (BYTE) 'B' ) ;
			}
			else
			{
				i ++ ;
				strDst.Add( (BYTE) dwJIS ) ;
			}
		}
		break ;

	case	encodingEUCJP:
		//
		// UNICODE -> EUC-JP
		//
		nDstCount =
			ESLCharset::UNICODEtoEUCJP
				( pwszSrc, (unsigned int) nSrcLength, NULL, 0 ) ;
		strDst.SetLength(nDstCount) ;
		//
		nDstCount =
			ESLCharset::UNICODEtoEUCJP
				( pwszSrc, (unsigned int) nSrcLength,
					(BYTE*) strDst.GetArray(), nDstCount ) ;
		strDst.FinishArray() ;
		strDst.SetLength(nDstCount) ;
		break ;

	case	encodingUTF16:
		//
		// UNICODE -> UTF16
		//
		strDst.SetLength
			( (nSrcLength + 1) * (sizeof(WORD) / sizeof(BYTE)) ) ;
		pwDst = (WORD*) strDst.GetArray() ;
		*(pwDst ++) = 0xFEFF ;
		for ( i = 0; i < nSrcLength; i ++ )
		{
			pwDst[i] = (WORD) pwszSrc[i] ;
		}
		strDst.FinishArray() ;
		break ;
	}
	return	strDst.GetLength() ;
}

// デコード処理
//////////////////////////////////////////////////////////////////////////////
size_t SSystem::Charset::Decode
	( SString& strDst, EncodingType type,
		const uint8_t * pbytSrc, ssize_t nSrcLength )
{
	//
	// 変換元パラメータ正規化
	//
	strDst.RemoveAll() ;
	if ( pbytSrc == NULL )
	{
		return	0 ;
	}
	if ( nSrcLength == (ssize_t) -1 )
	{
		for ( nSrcLength = 0; pbytSrc[nSrcLength] != 0; nSrcLength ++ )
		{
		}
	}
	ssize_t			nDstCount ;
	ssize_t			i ;
	bool			jismode ;
	const WORD *	pwSrc ;
	WORD *			pwDst ;
	wchar_t *		pwDstTemp ;
	switch ( type )
	{
	case	encodingShiftJIS:
		//
		// ShiftJIS -> UNICODE
		//
		nDstCount =
			ESLCharset::ShiftJIStoUNICODE
				( pbytSrc, (unsigned int) nSrcLength, NULL, 0 ) ;
		if ( sizeof(uint16_t) == sizeof(wchar_t) )
		{
			nDstCount =
				ESLCharset::ShiftJIStoUNICODE
					( pbytSrc, (unsigned int) nSrcLength,
						(wchar_t*) strDst.LockBuffer( nDstCount ),
						(unsigned int) nDstCount ) ;
			strDst.UnlockBuffer( nDstCount ) ;
		}
		else
		{
			pwDstTemp = new wchar_t[nDstCount] ;
			nDstCount =
				ESLCharset::ShiftJIStoUNICODE
					( pbytSrc, (unsigned int) nSrcLength,
						pwDstTemp, (unsigned int) nDstCount ) ;
			strDst.SetString( pwDstTemp, nDstCount ) ;
			delete []	pwDstTemp ;
		}
		break ;

	case	encodingUTF8:
	default:
		//
		// UTF8 -> UNICODE
		//
		if ( (nSrcLength >= 3) && (pbytSrc[0] == 0xEF)
			&& (pbytSrc[1] == 0xBB) && (pbytSrc[2] == 0xBF) )
		{
			pbytSrc +=3 ;
			nSrcLength -= 3 ;
		}
		nDstCount =
			ESLCharset::DecodeFromUTF8
				( NULL, 0, pbytSrc, (unsigned int) nSrcLength ) ;
		if ( sizeof(uint16_t) == sizeof(wchar_t) )
		{
			nDstCount =
				ESLCharset::DecodeFromUTF8
					( (wchar_t*) strDst.LockBuffer( nDstCount ),
						(unsigned int) nDstCount,
						pbytSrc, (unsigned int) nSrcLength ) ;
			strDst.UnlockBuffer( nDstCount ) ;
		}
		else
		{
			pwDstTemp = new wchar_t[nDstCount] ;
			nDstCount =
				ESLCharset::DecodeFromUTF8
					( pwDstTemp, (unsigned int) nDstCount,
						pbytSrc, (unsigned int) nSrcLength ) ;
			strDst.SetString( pwDstTemp, nDstCount ) ;
			delete []	pwDstTemp ;
		}
		break ;


	case	encodingISO2022JP:
		//
		// ISO-2022-JP -> UNICODE
		//
		jismode = false ;
		i = 0 ;
		while ( i < nSrcLength )
		{
			//
			// エスケープシーケンス
			while ( (pbytSrc[i] == 0x1B) && (i + 2 < nSrcLength) )
			{
				if ( ((pbytSrc[i + 1] == '$')
						&& (pbytSrc[i + 2] == 'B')) ||
					((pbytSrc[i + 1] == '$')
						&& (pbytSrc[i + 2] == '@')) ||
					((pbytSrc[i + 1] == '(')
						&& (pbytSrc[i + 2] == 'J')) )
				{
					jismode = true ;
					i += 3 ;
				}
				else if ( (pbytSrc[i + 1] == '(')
							&& (pbytSrc[i + 2] == 'B') )
				{
					jismode = false ;
					i += 3 ;
				}
				else
				{
					break ;
				}
			}
			if ( i >= nSrcLength )
			{
				break ;
			}
			//
			// ポインタを進める
			if ( jismode )
			{
				WORD	wJIS =
					(((WORD) pbytSrc[i]) << 8) | ((BYTE) pbytSrc[i + 1]) ;
				i += 2 ;
				//
				DWORD	dwUnicode =
					ESLCharset::UnicodeFromJISCode( wJIS ) ;
				//
				if ( dwUnicode != ESLCharset::UnicodeError )
				{
					strDst.Add( (uint16_t) dwUnicode ) ;
				}
				else
				{
					strDst.Add( (uint16_t) L'?' ) ;
				}
			}
			else
			{
				strDst.Add( (uint16_t) pbytSrc[i ++] ) ;
			}
		}
		break ;


	case	encodingEUCJP:
		//
		// EUC-JP -> UNICODE
		//
		nDstCount =
			ESLCharset::EUCJPtoUNICODE
				( pbytSrc, (unsigned int) nSrcLength, NULL, 0 ) ;
		if ( sizeof(uint16_t) == sizeof(wchar_t) )
		{
			nDstCount =
				ESLCharset::EUCJPtoUNICODE
					( pbytSrc, (unsigned int) nSrcLength,
						(wchar_t*) strDst.LockBuffer( nDstCount ),
						(unsigned int) nDstCount ) ;
			strDst.UnlockBuffer( nDstCount ) ;
		}
		else
		{
			pwDstTemp = new wchar_t[nDstCount] ;
			nDstCount =
				ESLCharset::EUCJPtoUNICODE
					( pbytSrc, (unsigned int) nSrcLength,
						pwDstTemp, (unsigned int) nDstCount ) ;
			strDst.SetString( pwDstTemp, nDstCount ) ;
			delete []	pwDstTemp ;
		}
		break ;

	case	encodingUTF16:
		//
		// UTF16 -> UNICODE
		//
		nDstCount = nSrcLength / (sizeof(WORD) / sizeof(BYTE)) ;
		pwSrc = (const WORD*) pbytSrc ;
		if ( (nDstCount > 0) && (*pwSrc == 0xFEFF) )
		{
			pwSrc ++ ;
			nDstCount -- ;
		}
		strDst.SetLength( nDstCount ) ;
		pwDst = (WORD*) strDst.GetArray() ;
		for ( i = 0; i < nDstCount; i ++ )
		{
			pwDst[i] = pwSrc[i] ;
		}
		strDst.FinishArray() ;
		break ;
	}
	return	strDst.GetLength() ;
}

// base64 エンコード
//////////////////////////////////////////////////////////////////////////////
void SSystem::Charset::EncodeBase64
	( SString& strBase64,
			const uint8_t * pbytBuf, size_t nBytes )
{
	//
	// base64 変換テーブル
	//
	static const wchar_t
		wchBase64Table[] =
			L"ABCDEFGHIJKLMNOP"
			L"QRSTUVWXYZabcdef"
			L"ghijklmnopqrstuv"
			L"wxyz0123456789+/" ;
	//
	// 変換ループ
	//
	uint16_t *	pwDst = strBase64.LockBuffer( (nBytes * 4 + 2) / 3 + 4 ) ;
	size_t		iDst = 0, iSrc = 0 ;
	while ( nBytes != 0 )
	{
		if ( nBytes >= 3 )
		{
			//
			// 末尾以外処理
			//
			pwDst[iDst ++] =
				(uint16_t) wchBase64Table[(pbytBuf[iSrc] >> 2)] ;
			pwDst[iDst ++] =
				(uint16_t) wchBase64Table
								[((pbytBuf[iSrc] & 0x03) << 4)
										| (pbytBuf[iSrc+1] >> 4)] ;
			pwDst[iDst ++] =
				(uint16_t) wchBase64Table
								[((pbytBuf[iSrc+1] & 0x0F) << 2)
										| (pbytBuf[iSrc+2] >> 6)] ;
			pwDst[iDst ++] =
				(uint16_t) wchBase64Table[(pbytBuf[iSrc+2] & 0x3F)] ;
			iSrc += 3 ;
			nBytes -= 3 ;
		}
		else
		{
			//
			// 末尾用特殊処理
			//
			pwDst[iDst ++] =
				(uint16_t) wchBase64Table[(pbytBuf[iSrc] >> 2)] ;
			if ( nBytes <= 1 )
			{
				pwDst[iDst ++] =
					(uint16_t) wchBase64Table
									[((pbytBuf[iSrc] & 0x03) << 4)] ;
				pwDst[iDst ++] = (uint16_t) '=' ;
				pwDst[iDst ++] = (uint16_t) '=' ;
			}
			else
			{
				pwDst[iDst ++] =
					(uint16_t) wchBase64Table
									[((pbytBuf[iSrc] & 0x03) << 4)
										| (pbytBuf[iSrc+1] >> 4)] ;
				pwDst[iDst ++] =
					(uint16_t) wchBase64Table
									[((pbytBuf[iSrc+1] & 0x0F) << 2)] ;
				pwDst[iDst ++] = (uint16_t) '=' ;
			}
			break ;
		}
	}
	strBase64.UnlockBuffer( (ssize_t) iDst ) ;
}

// base64 デコード
//////////////////////////////////////////////////////////////////////////////
SError SSystem::Charset::DecodeBase64
	( SArray<uint8_t>& aBinary,
		const wchar_t * pwszBase64, ssize_t nSrcLength )
{
	//
	// 出力バッファを確保
	//
	if ( nSrcLength < 0 )
	{
		nSrcLength = 0 ;
		while ( pwszBase64[nSrcLength] )
		{
			nSrcLength ++ ;
		}
	}
	uint8_t *	pbytDst ;
	aBinary.SetLength( nSrcLength * 3 / 4 ) ;
	pbytDst = aBinary.GetArray() ;
	//
	// 復号
	//
	size_t	iSrc = 0 ;
	size_t	iDst = 0 ;
	SError	err = errSuccess ;
	//
	while ( iSrc < (size_t) nSrcLength )
	{
		//
		// 空白を読み飛ばす
		//
		while ( iSrc < (size_t) nSrcLength )
		{
			if ( pwszBase64[iSrc] > L' ' )
			{
				break ;
			}
			iSrc ++ ;
		}
		if ( iSrc >= (size_t) nSrcLength )
		{
			break ;
		}
		//
		// 4文字取得
		//
		uint8_t	bin[4] ;	// 取得したバイナリ
		size_t	j = 0 ;		// 無効なバイト数
		for ( size_t i = 0; i < 4; i ++ )
		{
			if ( iSrc >= (size_t) nSrcLength )
			{
				err = errFailed ;
				break ;
			}
			wchar_t	c = pwszBase64[iSrc ++] ;
			if ( (c >= L'A') && (c <= L'Z') )
			{
				c -= L'A' ;
			}
			else if ( (c >= L'a') && (c <= L'z') )
			{
				c -= (L'a' - 0x1A) ;
			}
			else if ( (c >= L'0') && (c <= L'9') )
			{
				c -= (L'0' - 0x34) ;
			}
			else if ( c == L'+' )
			{
				c = 0x3E ;
			}
			else if ( c == L'/' )
			{
				c = 0x3f ;
			}
			else if ( c == L'=' )
			{
				j ++ ;
				c = 0 ;
			}
			else
			{
				err = errFailed ;
				break ;
			}
			bin[i] = (uint8_t) c ;
   		}
		if ( err )
		{
			break ;
		}
		//
		// 3バイト出力
		//
		if ( j == 0 )
		{
			// 末尾以外の処理
			pbytDst[iDst ++] = (bin[0] << 2) | (bin[1] >> 4) ;
			pbytDst[iDst ++] = ((bin[1] & 0x0F) << 4) | (bin[2] >> 2) ;
			pbytDst[iDst ++] = ((bin[2] & 0x03) << 6) | bin[3] ;
		}
		else
		{
			// 末尾処理
			pbytDst[iDst ++] = (bin[0] << 2) | (bin[1] >> 4) ;
			if ( j < 2 )
			{
				pbytDst[iDst ++] = ((bin[1] & 0x0F) << 4) | (bin[2] >> 2) ;
			}
			break ;
		}
	}
	//
	// バッファを確定
	//
	aBinary.FinishArray() ;
	aBinary.SetLength( iDst ) ;

	return	err ;
}

// 修正 base64 エンコード
//////////////////////////////////////////////////////////////////////////////
void SSystem::Charset::EncodeModifiedBase64
	( SString& strBase64,
			const uint8_t * pbytBuf, size_t nBytes )
{
	EncodeBase64( strBase64, pbytBuf, nBytes ) ;
	//
	size_t		nLength = strBase64.GetLength() ;
	uint16_t *	pwBuf = strBase64.LockBuffer( nLength ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		if ( pwBuf[i] == L'+' )
		{
			pwBuf[i] = (uint16_t) L'.' ;
		}
		else if ( pwBuf[i] == L'/' )
		{
			pwBuf[i] = (uint16_t) L',' ;
		}
		else if ( pwBuf[i] == L'=' )
		{
			pwBuf[i] = (uint16_t) L'_' ;
		}
	}
	strBase64.UnlockBuffer( (ssize_t) nLength ) ;
}

// 修正 base64 デコード
//////////////////////////////////////////////////////////////////////////////
SError SSystem::Charset::DecodeModifedBase64
	( SArray<uint8_t>& aBinary,
		const wchar_t * pwszBase64, ssize_t nSrcLength )
{
	SString		strBase64( pwszBase64, nSrcLength ) ;
	size_t		nLength = strBase64.GetLength() ;
	uint16_t *	pwBuf = strBase64.LockBuffer( nLength ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		if ( pwBuf[i] == L'.' )
		{
			pwBuf[i] = (uint16_t) L'+' ;
		}
		else if ( pwBuf[i] == L',' )
		{
			pwBuf[i] = (uint16_t) L'/' ;
		}
		else if ( pwBuf[i] == L'_' )
		{
			pwBuf[i] = (uint16_t) L'=' ;
		}
	}
	strBase64.UnlockBuffer( (ssize_t) nLength ) ;
	//
	return	DecodeBase64( aBinary, strBase64, (ssize_t) strBase64.GetLength() ) ;
}


