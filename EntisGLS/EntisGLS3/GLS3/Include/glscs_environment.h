
//////////////////////////////////////////////////////////////////////////////
// プラグイン用ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSFilePIInterface	: public	ECS_FILE
{
public:
	ESLFileObject *	m_pfile ;
	bool			m_fOwnFile ;

public:
	// 構築関数
	ECSFilePIInterface
			( ESLFileObject * pfile, bool fAutoDelete = false )
			: m_pfile(pfile), m_fOwnFile(fAutoDelete)
		{
			pfnRelease = PIC_Release ;
			pfnRead = PIC_Read ;
			pfnWrite = PIC_Write ;
			pfnGetLength = PIC_GetLength ;
			pfnSeek = PIC_Seek ;
			pfnGetPosition = PIC_GetPosition ;
			pfnSetEndOfFile = PIC_SetEndOfFile ;
			pfnGetFilePath = PIC_GetFilePath ;
		}
	// 消滅関数
	~ECSFilePIInterface( void )
		{
			if ( m_pfile && m_fOwnFile )
			{
				delete	m_pfile ;
			}
		}

protected:
	static void __stdcall PIC_Release( ECS_FILE * pfile ) ;
	static unsigned long int __stdcall PIC_Read
		( ECS_FILE * pfile, void * ptrBuffer, unsigned long int nBytes ) ;
	static unsigned long int __stdcall PIC_Write
		( ECS_FILE * pfile, const void * ptrBuffer, unsigned long int nBytes ) ;
	static unsigned long int __stdcall PIC_GetLength( ECS_FILE * pfile ) ;
	static unsigned long int __stdcall PIC_Seek
		( ECS_FILE * pfile, long int nOffsetPos, ECS_FILE::SeekOrigin fSeekFrom ) ;
	static unsigned long int __stdcall PIC_GetPosition( ECS_FILE * pfile ) ;
	static ESLError __stdcall PIC_SetEndOfFile( ECS_FILE * pfile ) ;
	static const char * __stdcall PIC_GetFilePath( ECS_FILE * pfile ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行環境オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSEnvironment
			: public SSystem::SEnvironmentInterface, public EDescription 
{
public:
	// 構築関数
	ECSEnvironment( void ) ;
	// 消滅関数
	virtual ~ECSEnvironment( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2
		( ECSEnvironment, SEnvironmentInterface, EDescription )

public:
	// EFile -> SFileOpener 変換インターフェース
	class	EFile ;
	class	EFileOpener	: public SSystem::SFileOpener
	{
	protected:
		EFile *	m_pFile ;
		bool	m_flagOwner ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( EFileOpener, SFileOpener )
		// 構築関数
		EFileOpener( EFile * pFile, bool flagOwner = false ) ;
		// 消滅関数
		virtual ~EFileOpener( void ) ;
	public:
		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SSystem::SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SSystem::SObjectArray<SSystem::SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SSystem::SObjectArray<SSystem::SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
	} ;
	// ファイル検索
	class	EFile	: public	ERISAArchive
	{
	public:
		bool			m_fDisabled ;
		bool			m_fArchive ;		// アーカイブファイルか？
		ERawFile		m_file ;			// アーカイブファイル
		ESLFileObject *	m_pOwnFile ;
		EString			m_strDirPath ;		// ディレクトリパス
		EString			m_strPassword ;		// アーカイブパスワード
		EWideString		m_wstrID ;
		EFileOpener *	m_pOpener ;
		SSystem::SCriticalSection	m_cs ;
	public:
		EFile( void )
			: m_fDisabled( false ), m_fArchive( false ),
				m_pOwnFile( NULL ), m_pOpener( NULL ) { }
		virtual ~EFile( void ) ;
	} ;
	// 外部モジュール（プラグイン）
	class	EPlugin
	{
	public:
		bool						m_fStartup ;
		EString						m_strModuleName ;
		HMODULE						m_hModule ;	// モジュールハンドル
		ECS_PLUGIN_ENTRY_TABLE *	m_ppiet ;	// エントリテーブル
	public:
		EPlugin( void )
			: m_fStartup( false ),
				m_hModule( NULL ), m_ppiet( NULL ) { }
		~EPlugin( void ) ;
	} ;

public:
	HINSTANCE			m_hInstance ;		// インスタンス
	ECSContext *		m_pContext ;		// プライマリコンテキスト

	EWStrTagArray<EWideString>
						m_staEnvDirPath ;	// 環境ディレクトリパス

	EString				m_strScriptFile ;	// スクリプトファイル名
	EString				m_strSaveDir ;		// セーブディレクトリ
	bool				m_fAcceptOtherSaveDir ;
	HICON				m_hMainIcon ;		// メインアイコン
	EString				m_strIconID ;
	EObjArray<EString>	m_lstIconID ;
	EObjArray<EString>	m_lstIconFile ;
	ENumArray<HCURSOR>	m_lstCursor ;		// カーソルリスト
	EObjArray<EString>	m_lstCursorFile ;
	ECSStrBufTagArray	m_staCursor ;
	EObjArray<EFile>	m_lstFiles ;		// ファイルリスト
	EObjArray<EPlugin>	m_lstModule ;		// 外部モジュールリスト

	EString				m_strCaption ;		// キャプション
	EString				m_strBootName ;		// 二重起動防止用名前
	EGL_SIZE			m_sizeDisplay ;		// 画面サイズ
	unsigned int		m_nDisplayDepth ;	// 画面ビット深度
	unsigned int		m_nFrequency ;		// 画面周波数
	EGameWindow::CooperationLevel
						m_clCooperation ;	// アプリケーションモード
	bool				m_fNoChangeMode ;

	SSystem::SString	m_strBaseFilePath ;
	size_t				m_nMaxHeapBlock ;
	size_t				m_nDefaultHeapSize ;
	size_t				m_nDefaultStackSize ;
	bool				m_fCompileToNative ;
	bool				m_fNativeBoundary ;

	EInternetSession *	m_pisSession ;		// インターネットセッション

public:
	// 設定ファイル初期化＆読み込み
	ESLError Initialize
		( ESLFileObject * pfile = NULL, ECSContext * pContext = NULL ) ;
	// 設定リソース解法
	void Release( void ) ;
	// 環境ディレクトリパスを設定する
	void SetEnvironmentPath
		( const wchar_t * pwszCurrentDir, ECSContext * pContext = NULL ) ;
	// 設定ファイル読み込み
	ESLError LoadEnvironment( ESLFileObject & file, HINSTANCE hInstance ) ;
	// ファイルパスフィルタリング
	EWideString FilterFilePath( const wchar_t * pwszPath ) ;

public:
	// 書き出し可能ディレクトリ設定
	void SetSaveDirectory
		( const char * pszSaveDir, bool fAcceptOtherSaveDir = false ) ;
	// 読み込みアーカイブファイル追加
	ESLError AddFileArchive
		( const wchar_t * pwszFilePath,
			const char * pszPassword, const wchar_t * pwszID = NULL ) ;
	ESLError AddFileArchive
		( EMemoryFile * pmemfile,
			const char * pszPassword, const wchar_t * pwszID = NULL ) ;
	// 読み込みディレクトリパス追加
	ESLError AddFileDirectory
		( const wchar_t * pwszFileDir, const wchar_t * pwszID = NULL ) ;
	// パス有効設定
	ESLError EnableFilePath( const wchar_t * pwszID, bool fEnable ) ;
	// アイコン追加
	ESLError AddIcon( const char * pszID, const char * pszFile ) ;
	// カーソル追加
	ESLError AddCursor( const char * pszID, const char * pszFile ) ;
	// モジュール追加
	ESLError AddModule
		( const wchar_t * pwszModuleName, ECSContext * pContext = NULL ) ;
	// フォント追加
	ESLError AddFont
		( const wchar_t * pwszFontName, const wchar_t * pwszFontFile ) ;

public:
	// カーソル取得
	HCURSOR GetCursorAs( const wchar_t * pwszID ) ;
	// ファイルを開く
	ESLFileObject * OpenFileObject
		( const char * pszFileName,
			long int nOpenFlag = ESLFileObject::modeRead
									| ESLFileObject::shareRead ) ;
	// ディレクトリ作成
	void CreateDirectory( const char * pszFilePath ) ;
	// インターネットセッション作成
	void OpenInternetSession( void ) ;
	// DLL 関数を検索する
	FARPROC FindPluginedFunction( const char * pszFuncName ) ;

public:	// SEnvironmentInterface オーバーライド
	// 設定情報取得
	virtual bool GetEnvironmentString
		( SSystem::SString& strValue, const wchar_t * pszValuePath ) ;
	// アプリケーション名
	virtual void GetApplicationName( SSystem::SString& strAppName ) ;
	virtual void SetApplicationName( const wchar_t * pszAppName ) ;
	// Sakura2 JIT Compiler
	virtual bool IsEnabledSakura2JITCompiler( void ) ;
	virtual void EnableSakura2JITCompiler( bool fJIT ) ;
	virtual bool IsEnabledSakura2JITBoundary( void ) ;
	virtual void EnableSakura2JITBoundary( bool fBoundary ) ;
	virtual uint64_t GetSakura2JITCpuFeatures( void ) ;
	// ヒープメモリ
	virtual size_t GetHeapBlockMaxSize( void ) ;
	virtual void SetHeapBlockMaxSize( size_t nSize ) ;
	virtual size_t GetDefaultHeapSize( void ) ;
	virtual void SetDefaultHeapSize( size_t nSize ) ;
	// 初期スタックサイズ
	virtual size_t GetDefaultStackSize( void ) ;
	virtual void SetDefaultStackSize( size_t nSize ) ;
	// ファイル・オープナー
	virtual SSystem::SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
	virtual bool IsExistingFile( const wchar_t * pszFilePath ) ;
	virtual SSystem::SError QueryFileState
		( const wchar_t * pszFilePath, SSystem::SFileOpener::State& state ) ;
	virtual size_t GetFileOpenerCount( void ) ;
	virtual ssize_t FindFileOpenerAs( const wchar_t * pwszID ) ;
	virtual SSystem::SFileOpener * GetFileOpenerAt( size_t iOpener ) ;
	virtual bool GetFileOpenerIDAt( SSystem::SString& strID, size_t iOpener ) ;
	virtual void AddFileOpener
		( SSystem::SFileOpener * pOpener,
			const wchar_t * pwszID, const wchar_t * pwszDefaultDir = nullptr ) ;
	virtual void RemoveFileOpener( const wchar_t * pwszID ) ;
	virtual void EnableFileOpener
				( const wchar_t * pwszID, bool fEnable ) ;
	// 書き込み可能ファイル・オープナー
	virtual bool CanOpenAllFileForWriting( void ) ;
	virtual void AcceptAllFileForWriting( bool fAllWriting ) ;
	virtual SSystem::SFileOpener * GetWritableFileOpener( void ) ;
	virtual void SetWritableFileOpener( SSystem::SFileOpener * pOpener ) ;
	// ファイル・パス
	virtual const SSystem::SString & GetBaseFilePath( void ) const ;
	virtual void SetBaseFilePath( const wchar_t * pszFilePath ) ;
	virtual SSystem::SString OffsetFilePath( const wchar_t * pszFilePath ) ;

} ;

