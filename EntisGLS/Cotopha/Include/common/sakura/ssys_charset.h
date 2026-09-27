
#if	!defined(__SAKURA2_CHARSET_H__)
#define	__SAKURA2_CHARSET_H__

namespace	SSystem
{
	namespace	Charset
	{
		//////////////////////////////////////////////////////////////////////
		// 文字集合・エンコーディング
		//////////////////////////////////////////////////////////////////////

		enum	EncodingType
		{
			encodingUnknown		= -1,
			encodingShiftJIS,
			encodingUTF8,
			encodingISO2022JP,
			encodingEUCJP,
			encodingUTF16,
		} ;

		extern const uint8_t	BOM_UTF8[3] ;
		extern const uint8_t	BOM_UTF16LE[2] ;

		// 文字コード種別
		const wchar_t * GetEncodingName( EncodingType type ) ;
		EncodingType GetEncodingType( const wchar_t * pszType ) ;

		// 文字コード判別
		EncodingType AnalyzeEncoding
			( const uint8_t * pbytSrc, ssize_t nSrcLength = -1 ) ;

		// エンコード処理
		size_t Encode
			( SArray<uint8_t>& strDst, EncodingType type,
				const wchar_t * pwszSrc, ssize_t nSrcLength = -1 ) ;

		// デコード処理
		size_t Decode
			( SString& strDst, EncodingType type,
				const uint8_t * pbytSrc, ssize_t nSrcLength = -1 ) ;

		// base64 エンコード
		void EncodeBase64
			( SString& strBase64,
					const uint8_t * pbytBuf, size_t nBytes ) ;

		// base64 デコード
		SError DecodeBase64
			( SArray<uint8_t>& aBinary,
				const wchar_t * pwszBase64, ssize_t nSrcLength = -1 ) ;

		// 修正 base64 エンコード
		void EncodeModifiedBase64
			( SString& strBase64,
					const uint8_t * pbytBuf, size_t nBytes ) ;

		// 修正 base64 デコード
		SError DecodeModifedBase64
			( SArray<uint8_t>& aBinary,
				const wchar_t * pwszBase64, ssize_t nSrcLength = -1 ) ;

	}
}

#endif

