
#if	!defined(__SAKURA2_WIN_RESOURCE_FILE_H__)
#define	__SAKURA2_WIN_RESOURCE_FILE_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// Win32 PE バイナリリソース・ファイルインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SWin32PEBinResourceFile	: public SMemoryReferenceFile
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SWin32PEBinResourceFile, SMemoryReferenceFile )
		// 構築関数
		SWin32PEBinResourceFile( void ) ;
		// 消滅関数
		virtual ~SWin32PEBinResourceFile( void ) ;

	protected:
		HRSRC		m_hRsrc ;
		HGLOBAL		m_hGlobal ;
		LPVOID		m_lpData ;

	public:
		// バイナリリソースを開く
		SError OpenResource( HMODULE hModule, const wchar_t * pwszRsrcID ) ;
		// リソースを解放する
		SError CloseResource( void ) ;

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Win32 PE バイナリリソース・オープナー
	//////////////////////////////////////////////////////////////////////////

	class	SWin32PEBinResourceOpener	: public SFileOpener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SWin32PEBinResourceOpener, SFileOpener )
		// 構築関数
		SWin32PEBinResourceOpener( HMODULE hModule ) ;
		// 消滅関数
		virtual ~SWin32PEBinResourceOpener( void ) ;

	protected:
		HMODULE	m_hModule ;

	public:
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
	} ;

}

#endif
