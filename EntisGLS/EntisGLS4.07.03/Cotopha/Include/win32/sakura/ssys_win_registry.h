
#if	!defined(__SAKURA2_WIN_REGISTRY_H__)
#define	__SAKURA2_WIN_REGISTRY_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// レジストリ
	//////////////////////////////////////////////////////////////////////////

	class	SRegistryKey	: public ESLObject
	{
	protected:
		HKEY	m_hKey ;
		bool	m_fOpened ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SRegistryKey, ESLObject )
		// 構築関数
		SRegistryKey( void ) ;
		// 消滅関数
		virtual ~SRegistryKey( void ) ;
		// キー取得
		operator HKEY ( void ) const
			{
				return	m_hKey ;
			}

	public:
		typedef	SObjectArray<SString>	MultiString ;

	public:
		// レジストリキーを作成
		SError CreateKey
			( HKEY hKey, const wchar_t * pszSubKey,
						REGSAM samDersired = KEY_ALL_ACCESS ) ;
		// レジストリキーを削除
		static SError DeleteKey( HKEY hKey, const wchar_t * pszSubKey ) ;
		// レジストリキーを開く
		SError OpenKey
			( HKEY hKey, const wchar_t * pszSubKey,
						REGSAM samDersired = KEY_ALL_ACCESS ) ;
		// レジストリキーを閉じる
		void CloseKey( void ) ;
		// 下層キーを列挙する
		SError EnumerateSubKeys( SObjectArray<SString>& lstKeyNames ) const ;
		// 値名を列挙する
		SError EnumerateValueNames( SObjectArray<SString>& lstValueNames ) const ;
		// 値を削除する
		SError DeleteValue( const wchar_t * pszValueName = NULL ) ;
		// 値をセットする
		SError SetBinary
			( const wchar_t * pszValueName,
				const void * ptrData, size_t nBytes ) ;
		SError SetInteger( const wchar_t * pszValueName, int32_t nInteger ) ;
		SError SetLargeInteger( const wchar_t * pszValueName, int64_t nInteger ) ;
		SError SetString( const wchar_t * pszValueName, const wchar_t * pszString ) ;
		SError SetDoubleReal( const wchar_t * pszValueName, double nReal ) ;
		SError SetMultiStrings
			( const wchar_t * pszValueName,
				const SObjectArray<MultiString> & lstMultiStrings ) ;
	protected:
		SError SetValue
			( const wchar_t * pszValueName,
				DWORD dwType, const void * ptrData, size_t nBytes ) ;
	public:
		// 値を取得する
		size_t GetBinary
			( const wchar_t * pszValueName, SArray<uint8_t>& bufData ) const ;
		int32_t GetInteger( const wchar_t * pszValueName, int32_t nDefValue = 0 ) const ;
		int64_t GetLargeInteger( const wchar_t * pszValueName, int64_t nDefValue = 0 ) const ;
		SString GetString
			( const wchar_t * pszValueName, const wchar_t * pszDefString = NULL ) const ;
		double GetDoubleReal
			( const wchar_t * pszValueName, double nDefValue = 0 ) const ;
		SError GetMultiStrings
			( SObjectArray<MultiString> & lstMultiStrings,
						const wchar_t * pszValueName ) const ;

	} ;

}

#endif

