
/*****************************************************************************
                          文字コード変換関数
 ****************************************************************************/


#if	!defined(__ESL_CHARSET_H__)
#define	__ESL_CHARSET_H__

namespace	ESLCharset
{
	enum	ErrorCode
	{
		UnicodeError = 0xFFFFFFFF,
		JiscodeError = 0xFFFFFFFF,
	} ;
	inline bool IsLeadByteShiftJIS( unsigned char cCode )
		{
			return	(cCode & 0x80) && ((cCode < 0xA0) || (cCode >= 0xE0)) ;
		}
	WORD JISCodeFromShiftJIS( WORD wShiftJIS ) ;
	WORD ShiftJISCodeFromJIS( WORD wJIS ) ;
	WORD JISCodeFromEUCJP( WORD wEUC ) ;
	WORD EUCJPFromJISCode( WORD wJIS ) ;
	DWORD UnicodeFromJISCode( WORD wJIS ) ;
	DWORD JISCodeFromUnicode( WORD wUnicode ) ;
	unsigned int ShiftJIStoUNICODE
		( const BYTE * pszShiftJIS, unsigned int nSJISLength,
				wchar_t * pwszUnicode, unsigned int nUnicodeLength ) ;
	unsigned int UNICODEtoShiftJIS
		( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
				BYTE * pszShiftJIS, unsigned int nSJISLength ) ;
	unsigned int EUCJPtoUNICODE
		( const BYTE * pszECUJP, unsigned int nEUCJPLength,
				wchar_t * pwszUnicode, unsigned int nUnicodeLength ) ;
	unsigned int UNICODEtoEUCJP
		( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
				BYTE * pszECUJP, unsigned int nECUJPLength ) ;
	unsigned int DecodeFromUTF8
		( wchar_t * pwszUnicode, unsigned int nUnicodeLength,
			const BYTE * pszUTF8, unsigned int nUTF8Length ) ;
	unsigned int EncodeToUTF8
		( const wchar_t * pwszUnicode, unsigned int nUnicodeLength,
				BYTE * pszUTF8, unsigned int nUTF8Length ) ;

}

#endif

