
#if	!defined(__SAKURA2_STRING_H__)
#define	__SAKURA2_STRING_H__


//////////////////////////////////////////////////////////////////////////////
// 文字列
// ※ native とのやり取りを SArray と同等にするため
//    ESLObject から派生はせず、仮想関数もメンバに持たない
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	class	SString	: public SArray<uint16_t>
	{
	private:
		#if	!defined(__WCHAR_EQU_UINT16__)
		SArray<wchar_t>	m_wstrTemp ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SSystem::SString )
		// 構築関数
		SString( void )
			{
			}
		SString( const SString & strSrc )
			{
				SetString( strSrc ) ;
			}
		#if	!defined(__COTOPHA__)
		SString( const wchar_t * pszSrc, ssize_t nLength = -1 )
			{
				SetString( pszSrc, nLength ) ;
			}
		#endif
		#if	!defined(__WCHAR_EQU_UINT16__)
		SString( const uint16_t * pszSrc, ssize_t nLength = -1 )
			{
				SetString( pszSrc, nLength ) ;
			}
		#endif
		SString( const char * pszSrc, ssize_t nLength = -1 )
			{
				SetString( pszSrc, nLength ) ;
			}
		SString( int64_t nValue, int nPrec = 0, int nRadix = 10 ) ;
		SString( double nValue, int nPrec ) ;

		// 内部バッファ
		uint16_t * LockBuffer( size_t nBufSize ) ;
		void UnlockBuffer( ssize_t nLength = -1 ) ;
		void SetLength( size_t nSize ) ;
		void FreeArray( void ) ;

		#if	!defined(__WCHAR_EQU_UINT16__)
		operator const wchar_t * ( void ) const
		{
			// wchar_t が 16bit のときだけそのままキャスト可能
			if ( sizeof(wchar_t) == sizeof(uint16_t) )
			{
				return	(const wchar_t *) SArray<uint16_t>::GetConstArray() ;
			}
			else
			{
				if ( m_wstrTemp.GetLength() == 0 )
				{
					return	((SString*)this)->GetWideCharArray() ;
				}
				#if	defined(__DEBUG__)
				CheckCastStringBuffer() ;
				#endif
				return	m_wstrTemp.GetConstArray() ;
			}
		}
		const wchar_t * GetWideCharArray( void ) ;
		void OnMofiiedString( void ) ;
		void CheckCastStringBuffer( void ) const ;
		#else
		void OnMofiiedString( void ) { }
		#endif

		// 文字列長計算
		static size_t GetLength( const wchar_t * pwszStr ) ;
		size_t GetLength( void ) const
		{
			return	m_nLength ;
		}

		// 代入
		void SetString( const SString & strSrc ) ;
		#if	!defined(__COTOPHA__)
		void SetString( const wchar_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		#if	!defined(__WCHAR_EQU_UINT16__)
		void SetString( const uint16_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		void SetString( const char * pszSrc, ssize_t nLength = -1 )
			{
				DecodeDefaultFrom( pszSrc, nLength ) ;
			}

		const SString & operator = ( const SString & strSrc )
			{
				SetString( strSrc ) ;
				return	*this ;
			}
		#if	!defined(__COTOPHA__)
		const SString & operator = ( const wchar_t * pszSrc )
			{
				SetString( pszSrc ) ;
				return	*this ;
			}
		#endif
		#if	!defined(__WCHAR_EQU_UINT16__)
		const SString & operator = ( const uint16_t * pszSrc )
			{
				SetString( pszSrc ) ;
				return	*this ;
			}
		#endif
		const SString & operator = ( const char * pszSrc )
			{
				SetString( pszSrc ) ;
				return	*this ;
			}

		// 文字コード変換 （コンパイラデフォルトの char 型文字セット）
		void DecodeDefaultFrom( const char * pszSrc, ssize_t nLength = -1 ) ;
		const char * EncodeDefaultTo( SArray<char>& strDst ) const ;
		SArray<char> ToCharArray( void ) const
			{
				SArray<char>	arrayChar ;
				if ( SArray<uint16_t>::m_ptrArray != NULL )
				{
					EncodeDefaultTo( arrayChar ) ;
					arrayChar.Add( (char) 0 ) ;
				}
				return	arrayChar ;
			}

		// エンコーディング変換
		SArray<uint8_t> ToUTF8( void ) const ;
		SArray<uint32_t> ToUTF32( void ) const ;
		void FromUTF8( const uint8_t * pszUTF8, ssize_t nLength = -1 ) ;
		void FromUTF32( const uint32_t * pszUTF32, ssize_t nLength = -1 ) ;

		// サロゲートペア判定
		static bool IsHighSurrogateCode( uint16_t code ) ;
		static bool IsLowSurrogateCode( uint16_t code ) ;
		// サロゲートペア変換
		static uint32_t UnicodeFromSurrogatePairs
							( uint16_t codeHigh, uint16_t codeLow ) ;
		static uint32_t SurrogatePairsFromUnicode( uint32_t code ) ;

		// 加算
		const SString & operator += ( const SString & strAdd ) ;
		#if	!defined(__COTOPHA__)
		const SString & operator += ( const wchar_t * pszAdd ) ;
		const SString & operator += ( wchar_t chAdd ) ;
		#endif
		const SString & operator += ( const char * pszAdd ) ;
		const SString & operator += ( char chAdd ) ;
		SString operator + ( const SString & strAdd ) const ;
		#if	!defined(__COTOPHA__)
		SString operator + ( const wchar_t * pszAdd ) const ;
		#endif
		SString operator + ( const char * pszAdd ) const ;

		// 積
		const SString & operator *= ( int nCount )
			{
				Multiple( nCount ) ;
				return	*this ;
			}
		SString operator * ( int nCount ) const ;

	public:
		// 文字列比較
		int Compare( const SString & strSrc ) const ;
		int CompareNoCase( const SString & strSrc ) const ;
		bool operator == ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) == 0 ;
			}
		bool operator != ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) != 0 ;
			}
		bool operator < ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) < 0 ;
			}
		bool operator <= ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) <= 0 ;
			}
		bool operator > ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) > 0 ;
			}
		bool operator >= ( const SString & strSrc ) const
			{
				return	Compare( strSrc ) >= 0 ;
			}

		#if	!defined(__COTOPHA__)
		static int Compare
			( const wchar_t * pszStr1, const wchar_t * pszStr2 ) ;
		static int CompareNoCase
			( const wchar_t * pszStr1, const wchar_t * pszStr2 ) ;
		int Compare( const wchar_t * pszStr ) const ;
		int CompareNoCase( const wchar_t * pszStr ) const ;
		bool operator == ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) == 0 ;
			}
		bool operator != ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) != 0 ;
			}
		bool operator < ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) < 0 ;
			}
		bool operator <= ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) <= 0 ;
			}
		bool operator > ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) > 0 ;
			}
		bool operator >= ( const wchar_t * pszStr ) const
			{
				return	Compare( pszStr ) >= 0 ;
			}
		#endif

		static int Compare
			( const char * pszStr1, const char * pszStr2 ) ;
		static int CompareNoCase
			( const char * pszStr1, const char * pszStr2 ) ;
		int Compare( const char * pszStr ) const ;
		int CompareNoCase( const char * pszStr ) const ;
		bool operator == ( const char * pszStr ) const
			{
				return	Compare( pszStr ) == 0 ;
			}
		bool operator != ( const char * pszStr ) const
			{
				return	Compare( pszStr ) != 0 ;
			}
		bool operator < ( const char * pszStr ) const
			{
				return	Compare( pszStr ) < 0 ;
			}
		bool operator <= ( const char * pszStr ) const
			{
				return	Compare( pszStr ) <= 0 ;
			}
		bool operator > ( const char * pszStr ) const
			{
				return	Compare( pszStr ) > 0 ;
			}
		bool operator >= ( const char * pszStr ) const
			{
				return	Compare( pszStr ) >= 0 ;
			}

		static int CompareLeft
			( const wchar_t * pszStr, const wchar_t * pszLeft ) ;
		static int CompareLeftNoCase
			( const wchar_t * pszStr, const wchar_t * pszLeft ) ;
		int CompareLeft( const wchar_t * pszLeft ) const ;
		int CompareLeftNoCase( const wchar_t * pszLeft ) const ;

	public:
		// 文字検索
		#if	!defined(__COTOPHA__)
		ssize_t Find( wchar_t chFind, size_t iFirst = 0 ) const ;
		ssize_t Find( const wchar_t * pszFind, size_t iFirst = 0 ) const ;
		#endif
		ssize_t Find( char chFind, size_t iFirst = 0 ) const ;
		ssize_t Find( const char * pszFind, size_t iFirst = 0 ) const ;

		// 文字取得
		wchar_t GetAt( size_t index ) const
		{
			if ( index < SArray<uint16_t>::m_nLength )
			{
				return	SArray<uint16_t>::m_ptrArray[index] ;
			}
			return	0 ;
		}
		wchar_t GetLastAt( size_t index ) const
		{
			if ( index < SArray<uint16_t>::m_nLength )
			{
				return	SArray<uint16_t>::m_ptrArray
							[SArray<uint16_t>::m_nLength - index - 1] ;
			}
			return	0 ;
		}

		// 文字設定
		void SetAt( size_t index, wchar_t wch ) ;

		// 文字列挿入・削除
		void InsertAt( size_t nIndex, wchar_t wch ) ;
		void RemoveAt( size_t nIndex ) ;

		// 部分文字列取得
		SString Left( size_t nCount ) const ;
		SString Right( size_t nCount ) const ;
		SString Middle( size_t iFirst, ssize_t nCount = -1 ) const ;

		// 文字置き換え
		#if	!defined(__COTOPHA__)
		void Replace( wchar_t chOld, wchar_t chNew ) ;
		#endif
		void Replace( char chOld, char chNew ) ;

		// 文字列置き換え
		struct	FILTER_ENTRY
		{
			const wchar_t *	pwszSource ;
			const wchar_t *	pwszTarget ;
		} ;
		static void PrepareFilter
			( FILTER_ENTRY * pFilter,
					size_t nCount, uint32_t nFlags = 0 ) ;
		SString MappingFilter
			( const FILTER_ENTRY * pFilter,
						size_t nCount, uint32_t nFlags = 0 ) const ;

		// 文字列順序反転
		void Reverse( void ) ;
		// 文字列反復
		void Multiple( int nCount ) ;
		// 半角アルファベットを大文字に変換
		void MakeUpper( void ) ;
		// 半角アルファベットを小文字に変換
		void MakeLower( void ) ;
		// 文字列の右側の空白・制御文字を除去
		void TrimRight( void ) ;
		// 文字列の左側の空白・制御文字を除去
		void TrimLeft( void ) ;
		// 文字列の右側を指定文字数カット
		void ChopRight( size_t nCount ) ;
		// 文字列の左側を指定文字数カット
		void ChopLeft( size_t nCount ) ;
		// 空の文字列か
		bool IsEmpty( void ) const
			{
				return	(SArray<uint16_t>::m_nLength == 0) ;
			}
		// 半角文字を全角文字に変換
		void MakeFullWidthChars( void ) ;
		// 全角文字を半角文字に変換
		void MakeHalfWidthChars( void ) ;

	public:
		// ファイル名（URL）操作
		const uint16_t * GetFileNamePart( wchar_t chSeparator = L'\\' ) const ;
		const uint16_t * GetFileExtensionPart( wchar_t chSeparator = L'\\' ) const ;
		SString GetFileDirectoryPart( wchar_t chSeparator = L'\\' ) const ;
		SString GetFileTitlePart( wchar_t chSeparator = L'\\' ) const ;
		SString GetFileDrivePart( wchar_t chSeparator = L'\\' ) const ;
		#if	!defined(__COTOPHA__)
		SString OffsetFilePath
			( const wchar_t * pszOffsetPath,
						wchar_t chSeparator = L'\\' ) const ;
		SString RelativeFilePath
			( const wchar_t * pszFullPath,
					size_t nAscendLimit = 5, wchar_t chSeparator = L'\\' ) const ;
		#endif
		SString OffsetFilePath
			( const char * pszOffsetPath, char chSeparator = '\\' ) const ;
		const SString& NormalizeFilePath( wchar_t chSeparator = '\\', uint32_t nFlags = 0 ) ;

	public:
		size_t FindFileNamePart( wchar_t chSeparator ) const ;
		size_t FindFileExtensionPart( wchar_t chSeparator ) const ;

	public:
		// 数値変換
		static int NumberFromChar( uint32_t ch ) ;
		int64_t AsInteger
			( int nRadix = 10, bool fSign = true, bool * pError = NULL ) const ;
		double AsReal( int nRadix = 10, bool * pError = NULL ) const ;
		void FromInteger( int64_t nValue, int nPrec = 0, int nRadix = 10 ) ;
		void HexFromInteger( uint64_t nValue, int nPrec = 0 ) ;
		void FromReal( double rValue, int nPrec = 0 ) ;
		// 文字列フォーマット
		const wchar_t * Format( const wchar_t * pwszFormat, ... ) ;
		const wchar_t * FormatV( const wchar_t * pwszFormat, va_list argptr ) ;

	} ;
} ;

#endif

