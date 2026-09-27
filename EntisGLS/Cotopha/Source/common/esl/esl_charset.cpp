
/*****************************************************************************
                          文字コード変換関数
 ****************************************************************************/


#include <sakura/sakura.h>


namespace	ESLCharset
{
	// JIS <---> UNICODE 変換テーブル
	#include "esl/jis_to_unicode.h"
	#include "esl/unicode_to_jis.h"


	// 文字列配列に文字を追加する関数
	template <class T> static inline unsigned int
		AddCharToStringBuffer
			( T * pszDst, unsigned int nDstLength,
					unsigned int iDst, T cChar )
	{
		if ( pszDst != NULL )
		{
			if ( iDst < nDstLength )
			{
				pszDst[iDst ++] = cChar ;
			}
			return	iDst ;
		}
		else
		{
			return	iDst + 1 ;
		}
	}

}

WORD ESLCharset::JISCodeFromShiftJIS( WORD wShiftJIS )
{
	if ( wShiftJIS & 0xFF00 )
	{
		BYTE	sjis1 = (BYTE) (wShiftJIS >> 8) ;
		BYTE	sjis2 = (BYTE) wShiftJIS ;
		if ( sjis1 <= 0x9F )
		{
			sjis1 -= 0x71 ;
		}
		else
		{
			sjis1 -= 0xB1 ;
		}
		sjis1 = (sjis1 << 1) + 1 ;
		if ( sjis2 > 0x7F )
		{
			sjis2 -- ;
		}
		if ( sjis2 >= 0x9E )
		{
			sjis2 -= 0x7D ;
			sjis1 ++ ;
		}
		else
		{
			sjis2 -= 0x1F ;
		}
		return	(((WORD) sjis1) << 8) | sjis2 ;
	}
	return	wShiftJIS ;
}

WORD ESLCharset::ShiftJISCodeFromJIS( WORD wJIS )
{
	if ( wJIS & 0xFF00 )
	{
		BYTE	jis1 = (BYTE) (wJIS >> 8) ;
		BYTE	jis2 = (BYTE) wJIS ;
		if ( jis1 & 0x01 )
		{
			jis2 += 0x1F ;
		}
		else
		{
			jis2 += 0x7D ;
		}
		if ( jis2 >= 0x7F )
		{
			jis2 ++ ;
		}
		jis1 = ((jis1 - 0x21) >> 1) + 0x81 ;
		if ( jis1 > 0x9F )
		{
			jis1 += 0x40 ;
		}
		return	(((WORD) jis1) << 8) | jis2 ;
	}
	return	wJIS ;
}

WORD ESLCharset::JISCodeFromEUCJP( WORD wEUC )
{
	BYTE	euc1 = (BYTE) (wEUC >> 8) ;
	BYTE	euc2 = (BYTE) wEUC ;
	if ( euc1 == 0x8E )
	{
		return	euc2 ;
	}
	else if ( (euc2 >= 0xA0) /* && (euc2 <= 0xFF) */ )
	{
		euc1 &= (BYTE) ~0x80 ;
		euc2 &= (BYTE) ~0x80 ;
		return	(((WORD) euc1) << 8) | euc2 ;
	}
	return	wEUC ;
}

WORD ESLCharset::EUCJPFromJISCode( WORD wJIS )
{
	if ( wJIS & 0xFF00 )
	{
		return	wJIS | 0x8080 ;
	}
	else if ( (wJIS >= 0xA0) && (wJIS < 0xE0) )
	{
		return	0x8E00 | wJIS ;
	}
	return	wJIS ;
}

DWORD ESLCharset::UnicodeFromJISCode( WORD wJIS )
{
	if ( wJIS & 0xFF80 )
	{
		const WORD *	pwTable = g_pwJIStoUNICODETable[wJIS >> 8] ;
		if ( pwTable != NULL )
		{
			WORD	wUnicode = pwTable[wJIS & 0xFF] ;
			if ( wUnicode != 0 )
			{
				return	wUnicode ;
			}
		}
	}
	else
	{
		return	wJIS ;
	}
	return	UnicodeError ;
}

DWORD ESLCharset::JISCodeFromUnicode( WORD wUnicode )
{
	if ( wUnicode & 0xFF80 )
	{
		const WORD *	pwTable =
			g_pwUNICODEtoJISTable[wUnicode >> 8] ;
		if ( pwTable != NULL )
		{
			WORD	wJIS = pwTable[wUnicode & 0xFF] ;
			if ( wJIS != 0 )
			{
				return	wJIS ;
			}
		}
	}
	else
	{
		return	wUnicode ;
	}
	return	JiscodeError ;
}

unsigned int ESLCharset::ShiftJIStoUNICODE
	( const BYTE * pszShiftJIS, unsigned int nSJISLength,
			 wchar_t * pwszUnicode, unsigned int nUnicodeLength )
{
	unsigned int	i, j = 0 ;
	if ( nSJISLength == (unsigned int) -1 )
	{
		nSJISLength = 0 ;
		if ( pszShiftJIS != NULL )
		{
			for ( i = 0; pszShiftJIS[i]; i ++ )
			{
			}
			nSJISLength = i + 1 ;
		}
	}
	for ( i = 0; i < nSJISLength; i ++ )
	{
		BYTE	c = pszShiftJIS[i] ;
		WORD	wJIS = c ;
		if ( IsLeadByteShiftJIS( c ) )
		{
			BYTE	jis2 = pszShiftJIS[++ i] ;
			wJIS = JISCodeFromShiftJIS( (wJIS << 8) | jis2 ) ;
		}
		DWORD	dwUnicode = UnicodeFromJISCode( wJIS ) ;
		if ( dwUnicode != UnicodeError )
		{
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, (wchar_t) dwUnicode ) ;
		}
		else
		{
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, L'?' ) ;
		}
	}
	return	j ;
}

unsigned int ESLCharset::UNICODEtoShiftJIS
	( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
			BYTE * pszShiftJIS, unsigned int nSJISLength )
{
	unsigned int	i, j = 0 ;
	if ( nUnicodeLength == (unsigned int) -1 )
	{
		nUnicodeLength = 0 ;
		if ( pwszUnicode != NULL )
		{
			for ( i = 0; pwszUnicode[i]; i ++ )
			{
			}
			nUnicodeLength = i + 1 ;
		}
	}
	for ( i = 0; i < nUnicodeLength; i ++ )
	{
		wchar_t	wUnicode = pwszUnicode[i] ;
		DWORD	dwJIS = JISCodeFromUnicode( wUnicode ) ;
		if ( dwJIS != JiscodeError )
		{
			WORD	wShiftJIS = ShiftJISCodeFromJIS( (WORD) dwJIS ) ;
			if ( wShiftJIS & 0xFF00 )
			{
				j = AddCharToStringBuffer<BYTE>
					( pszShiftJIS, nSJISLength, j, (BYTE) (wShiftJIS >> 8) ) ;
				j = AddCharToStringBuffer<BYTE>
					( pszShiftJIS, nSJISLength, j, (BYTE) wShiftJIS ) ;
			}
			else
			{
				j = AddCharToStringBuffer<BYTE>
					( pszShiftJIS, nSJISLength, j, (BYTE) wShiftJIS ) ;
			}
		}
		else
		{
			j = AddCharToStringBuffer<BYTE>
				( pszShiftJIS, nSJISLength, j, (BYTE) '?' ) ;
		}
	}
	return	j ;
}

unsigned int ESLCharset::EUCJPtoUNICODE
	( const BYTE * pszECUJP, unsigned int nEUCJPLength,
			wchar_t * pwszUnicode, unsigned int nUnicodeLength )
{
	unsigned int	i, j = 0 ;
	if ( nEUCJPLength == (unsigned int) -1 )
	{
		nEUCJPLength = 0 ;
		if ( pszECUJP != NULL )
		{
			for ( i = 0; pszECUJP[i]; i ++ )
			{
			}
			nEUCJPLength = i + 1 ;
		}
	}
	for ( i = 0; i < nEUCJPLength; i ++ )
	{
		BYTE	c = pszECUJP[i] ;
		if ( (c & 0x80) && (i + 1 < nEUCJPLength) )
		{
			WORD	wEUC = (((WORD) c) << 8) | ((BYTE) pszECUJP[++ i]) ;
			WORD	wJIS =
				ESLCharset::JISCodeFromEUCJP( wEUC ) ;
			DWORD	dwUnicode =
				ESLCharset::UnicodeFromJISCode( wJIS ) ;
			//
			if ( dwUnicode != ESLCharset::UnicodeError )
			{
				j = AddCharToStringBuffer<wchar_t>
					( pwszUnicode, nUnicodeLength, j, (wchar_t) dwUnicode ) ;
			}
			else
			{
				j = AddCharToStringBuffer<wchar_t>
					( pwszUnicode, nUnicodeLength, j, L'?' ) ;
			}
		}
		else
		{
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, (wchar_t) c ) ;
		}
	}
	return	j ;
}

unsigned int ESLCharset::UNICODEtoEUCJP
	( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
			BYTE * pszECUJP, unsigned int nECUJPLength )
{
	unsigned int	i, j = 0 ;
	if ( nUnicodeLength == (unsigned int) -1 )
	{
		nUnicodeLength = 0 ;
		if ( pwszUnicode != NULL )
		{
			for ( i = 0; pwszUnicode[i]; i ++ )
			{
			}
			nUnicodeLength = i + 1 ;
		}
	}
	for ( i = 0; i < nUnicodeLength; i ++ )
	{
		wchar_t	wUnicode = pwszUnicode[i] ;
		DWORD	dwJIS = JISCodeFromUnicode( wUnicode ) ;
		if ( dwJIS != JiscodeError )
		{
			WORD	wEUC = EUCJPFromJISCode( (WORD) dwJIS ) ;
			if ( wEUC & 0xFF00 )
			{
				j = AddCharToStringBuffer<BYTE>
					( pszECUJP, nECUJPLength, j, (BYTE) (wEUC >> 8) ) ;
				j = AddCharToStringBuffer<BYTE>
					( pszECUJP, nECUJPLength, j, (BYTE) wEUC ) ;
			}
			else
			{
				j = AddCharToStringBuffer<BYTE>
					( pszECUJP, nECUJPLength, j, (BYTE) wEUC ) ;
			}
		}
		else
		{
			j = AddCharToStringBuffer<BYTE>
				( pszECUJP, nECUJPLength, j, (BYTE) '?' ) ;
		}
	}
	return	j ;
}

unsigned int ESLCharset::DecodeFromUTF8
	( wchar_t * pwszUnicode, unsigned int nUnicodeLength,
			const BYTE * pszUTF8, unsigned int nUTF8Length )
{
	unsigned int	i = 0, j = 0 ;
	if ( nUTF8Length == (unsigned int) -1 )
	{
		nUTF8Length = 0 ;
		if ( pszUTF8 != NULL )
		{
			for ( i = 0; pszUTF8[i]; i ++ )
			{
			}
			nUTF8Length = i + 1 ;
		}
	}
	while ( i < nUTF8Length )
	{
		DWORD	unicode ;
		BYTE	utf8 = pszUTF8[i ++] ;
		//
		if ( utf8 & 0x80 )
		{
			unsigned char	mask = 0x20 ;
			int	count = 1 ;
			while ( utf8 & mask )
			{
				count ++ ;
				mask >>= 1 ;
				if ( mask == 0 )
					break ;
			}
			//
			unicode = utf8 & ((1 << (6 - count)) - 1) ;
			for ( int k = 0; (k < count) && (i < nUTF8Length); k ++ )
			{
				unicode = (unicode << 6) | (pszUTF8[i ++] & 0x3F) ;
			}
		}
		else
		{
			unicode = utf8 ;
		}
		if ( (unicode < 0x10000)
			|| (sizeof(wchar_t) == 4) )
		{
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, (wchar_t) unicode ) ;
		}
		else
		{
			wchar_t	codeHigh = 0xD800 + (wchar_t) ((unicode - 0x10000) >> 10) ;
			wchar_t	codeLow = 0xDC00 + (wchar_t) ((unicode - 0x10000) & 0x3FF) ;
			//
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, codeHigh ) ;
			j = AddCharToStringBuffer<wchar_t>
				( pwszUnicode, nUnicodeLength, j, codeLow ) ;
		}
	}
	return	j ;
}

unsigned int ESLCharset::EncodeToUTF8
	( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
			BYTE * pszUTF8, unsigned int nUTF8Length )
{
	unsigned int	i = 0, j = 0 ;
	if ( nUnicodeLength == (unsigned int) -1 )
	{
		nUnicodeLength = 0 ;
		if ( pwszUnicode != NULL )
		{
			for ( i = 0; pwszUnicode[i]; i ++ )
			{
			}
			nUnicodeLength = i + 1 ;
		}
	}
	while ( i < nUnicodeLength )
	{
		DWORD	ucs4 = pwszUnicode[i ++] ;
		//
		if ( ucs4 < (DWORD) 0x80 )
		{
			j = AddCharToStringBuffer<BYTE>
				( pszUTF8, nUTF8Length, j, (BYTE) ucs4 ) ;
		}
		else
		{
			int			count = 1 ;
			signed char	first_char = -0x40 ;
			DWORD		range = 0x800 ;
			while ( ucs4 >= range )
			{
				count ++ ;
				range <<= 5 ;
				first_char >>= 1 ;
			}
			//
			int	first_width = 6 - count ;
			ucs4 <<= (32 - (count * 6 + first_width)) ;
			//
			j = AddCharToStringBuffer<BYTE>
				( pszUTF8, nUTF8Length, j,
					(BYTE) (first_char | (ucs4 >> (32 - first_width))) ) ;
			ucs4 <<= first_width ;
			//
			for ( int i = 0; i < count; i ++ )
			{
				j = AddCharToStringBuffer<BYTE>
					( pszUTF8, nUTF8Length, j,
							(BYTE) (0x80 | (ucs4 >> 26)) ) ;
				ucs4 <<= 6 ;
			}
		}
	}
	return	j ;
}


