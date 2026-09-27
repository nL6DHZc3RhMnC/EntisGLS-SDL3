
#if	!defined(__SAKURA2_ENVIRONMENT_H__)
#define	__SAKURA2_ENVIRONMENT_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// プロセス（仮想マシン）環境設定
	//////////////////////////////////////////////////////////////////////////

	class	__native Environment
	{
	public:
		// 設定情報取得
		static __native bool GetEnvironmentString
			( SArray<uint16_t>& strValue, const wchar_t * pszValuePath ) ;
		// アプリケーション名
		static __native void GetApplicationName( SArray<uint16_t> & strAppName ) ;
		static __native void SetApplicationName( const wchar_t * pszAppName ) ;
		// Sakura2 JIT Compiler
		static __native bool IsEnabledSakura2JITCompiler( void ) ;
		static __native void EnableSakura2JITCompiler( bool fJIT ) ;
		static __native bool IsEnabledSakura2JITBoundary( void ) ;
		static __native void EnableSakura2JITBoundary( bool fBoundary ) ;
		// 書き込み可能ファイル
		static __native bool CanOpenAllFileForWriting( void ) ;
		static __native void AcceptAllFileForWriting( bool fAllWriting ) ;
	} ;

	class	SEnvironmentInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SEnvironmentInterface, ESLObject )
		// 構築関数
		SEnvironmentInterface( void ) ;
		// 消滅関数
		virtual ~SEnvironmentInterface( void ) ;

	protected:
		static ESL_DLL_EXPORT SEnvironmentInterface *	m_pDefault ;

	public:
		// 設定情報取得
		virtual bool GetEnvironmentString
			( SString& strValue, const wchar_t * pszValuePath ) = 0 ;
		// アプリケーション名
		virtual void GetApplicationName( SString& strAppName ) = 0 ;
		virtual void SetApplicationName( const wchar_t * pszAppName ) = 0 ;
		// Sakura2 JIT Compiler
		virtual bool IsEnabledSakura2JITCompiler( void ) = 0 ;
		virtual void EnableSakura2JITCompiler( bool fJIT ) = 0 ;
		virtual bool IsEnabledSakura2JITBoundary( void ) = 0 ;
		virtual void EnableSakura2JITBoundary( bool fBoundary ) = 0 ;
		virtual uint64_t GetSakura2JITCpuFeatures( void ) = 0 ;
		// ヒープメモリ
		virtual size_t GetHeapBlockMaxSize( void ) = 0 ;
		virtual void SetHeapBlockMaxSize( size_t nSize ) = 0 ;
		virtual size_t GetDefaultHeapSize( void ) = 0 ;
		virtual void SetDefaultHeapSize( size_t nSize ) = 0 ;
		// 初期スタックサイズ
		virtual size_t GetDefaultStackSize( void ) = 0 ;
		virtual void SetDefaultStackSize( size_t nSize ) = 0 ;
		// ファイル・オープナー
		virtual SFileInterface * NewOpenFile
				( const wchar_t * pszFilePath, long int nOpenFlags ) = 0 ;
		virtual bool IsExistingFile( const wchar_t * pszFilePath ) = 0 ;
		virtual SError QueryFileState
			( const wchar_t * pszFilePath, SFileOpener::State& state ) = 0 ;
		virtual SError ListReadableFiles
			( SObjectArray<SString>& listFiles,
				const wchar_t * pszFilePathWildCard ) ;
		virtual size_t GetFileOpenerCount( void ) = 0 ;
		virtual ssize_t FindFileOpenerAs( const wchar_t * pwszID ) = 0 ;
		virtual SFileOpener * GetFileOpenerAt( size_t iOpener ) = 0 ;
		virtual bool GetFileOpenerIDAt( SString& strID, size_t iOpener ) = 0 ;
		virtual void AddFileOpener
				( SFileOpener * pOpener,
					const wchar_t * pwszID,
					const wchar_t * pwszDefaultDir = nullptr ) = 0 ;
		virtual void RemoveFileOpener( const wchar_t * pwszID ) = 0 ;
		virtual void EnableFileOpener
					( const wchar_t * pwszID, bool fEnable ) = 0 ;
		// 書き込み可能ファイル・オープナー
		virtual bool CanOpenAllFileForWriting( void ) = 0 ;
		virtual void AcceptAllFileForWriting( bool fAllWriting ) = 0 ;
		virtual SFileOpener * GetWritableFileOpener( void ) = 0 ;
		virtual void SetWritableFileOpener( SFileOpener * pOpener ) = 0 ;
		// ファイル・パス
		virtual const SString & GetBaseFilePath( void ) const = 0 ;
		virtual void SetBaseFilePath( const wchar_t * pszFilePath ) = 0 ;
		virtual SString OffsetFilePath( const wchar_t * pszFilePath ) = 0 ;

	public:
		// グローバル環境設定
		static SEnvironmentInterface * GetInstance( void ) ;
		static void AttachInstance( SEnvironmentInterface * pEnv ) ;
	} ;

	#if	!defined(__COTOPHA__)
	class	SEnvironment	: public SEnvironmentInterface,
								public SProgressiveUserInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SEnvironment, SEnvironmentInterface, SProgressiveUserInterface )
		// 構築関数
		SEnvironment( void ) ;
		// 消滅関数
		virtual ~SEnvironment( void ) ;

	protected:
		class	FileOpenerEntry
		{
		public:
			SString			m_strID ;
			SString			m_strDefaultDir ;
			bool			m_fDisabled ;
			SFileOpener *	m_pOpener ;
		public:
			FileOpenerEntry
				( SFileOpener * pOpener,
					const wchar_t * pwszID, const wchar_t * pwszDefDir )
				: m_strID(pwszID), m_strDefaultDir(pwszDefDir),
					m_fDisabled( false ), m_pOpener( pOpener ) {}
			~FileOpenerEntry( void )
			{
				delete	m_pOpener ;
				m_pOpener = NULL ;
			}
		} ;
		SXMLDocument					m_xmlEnv ;
		SStrSortObjectArray<SString>	m_ssoaEnvVar ;

		SObjectArray<FileOpenerEntry>	m_vectorOpener ;
		SSmartPointer<SFileOpener>		m_pWritableOpener ;
		SObjectArray<SOffsetFileOpener>	m_vectorTempOpener ;

		SString		m_strAppName ;
		SString		m_strBasePath ;
		bool		m_flagJITCompiler ;
		bool		m_flagJITBoundary ;
		bool		m_flagAllFileWritable ;
		uint64_t	m_maskCpuFeatures ;
		size_t		m_sizeMaxHeapBlock ;
		size_t		m_sizeDefaultHeap ;
		size_t		m_sizeDefaultStack ;
		size_t		m_sizeReqMemory ;
		uint32_t	m_maskReqJITFeatures ;

		bool		m_flagAppUpdate ;
		bool		m_flagAutoCheckUpdate ;
		bool		m_flagNoConfirmToUpdate ;
		bool		m_flagMustUpdate ;
		int64_t		m_nAppVersion ;
		SString		m_strUpdateURL ;
		SString		m_strUpdaterCmd ;

		SProgressiveDialog	m_dlgProgressive ;

	public:
		// デフォルトの環境変数設定
		virtual void RegisterDefaultEnvironmentString( void ) ;
		// XML 読み込み
		virtual SError ReadDocument
			( SFileInterface& file, SParserErrorInterface& perr ) ;
		// XML ドキュメント取得
		const SXMLDocument& GetXMLDocumnet( void ) const ;
		// 環境設定初期化
		void ClearEnvironment( void ) ;

	public:
		// パラメータ解釈
		virtual void InitEnvironment( const SXMLDocument& xmlDoc ) ;
	protected:
		// <save_dir> タグ解釈
		virtual void ParseEnvironmentSaveDirTag( const SXMLDocument& xmlTag ) ;
		// <file> タグ解釈
		virtual void ParseEnvironmentFileTag( const SXMLDocument& xmlTag ) ;
		// <file> タグ用オープナー生成
		virtual SFileOpener * CreateFileOpener
			( const wchar_t * pwszPath,
				bool fFragment = false, ssize_t nFragmentCache = -1 ) ;
		// <archive> タグ解釈
		virtual void ParseEnvironmentArchiveTag( const SXMLDocument& xmlTag ) ;
		// <archive> タグ用オープナー生成
		virtual SFileOpener * CreateArchiveOpener
			( const wchar_t * pwszPath,
				const wchar_t * pwszPassword,
				bool fFragment = false, ssize_t nFragmentCache = -1,
				bool fCrypt32 = false, const wchar_t * pwszDecryptPass = nullptr ) ;
		// テンポラルな <file> オープナー生成
		SOffsetFileOpener * CreateTempFileOpener( const wchar_t * pwszPath ) ;
		// <display> タグ解釈
		virtual void ParseEnvironmentDisplayTag( const SXMLDocument& xmlTag ) ;
		// <vm> タグ解釈
		virtual void ParseEnvironmentVMTag( const SXMLDocument& xmlTag ) ;
		// <update> タグ解釈
		virtual void ParseEnvironmentUpdateTag( const SXMLDocument& xmlTag ) ;
		// <requirement> タグ解釈
		virtual void ParseEnvironmentRequirementTag( const SXMLDocument& xmlTag ) ;
		// 非標準タグ解釈
		virtual void ParseExtendedEnvironment( const SXMLDocument& xmlTag ) ;

	public:
		// 文字列置き換えフィルタ処理
		virtual void FilterEnvironmentString( SString& strValue ) ;
		// 文字列置き換え登録
		virtual void RegisterEnvironmentString
			( const wchar_t * pszVarName, const wchar_t * pszVarValue ) ;
		// テキスト・コンテキスト取得
		virtual SString GetTextResourceAs
			( const wchar_t * pwszID, const wchar_t * pwszDef ) ;

	public:
		// 設定情報取得
		virtual bool GetEnvironmentString
			( SString& strValue, const wchar_t * pszValuePath ) ;
		// アプリケーション名
		virtual void GetApplicationName( SString& strAppName ) ;
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
		virtual SFileInterface * NewOpenFile
				( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		virtual bool IsExistingFile( const wchar_t * pszFilePath ) ;
		virtual SError QueryFileState
			( const wchar_t * pszFilePath, SFileOpener::State& state ) ;
		virtual size_t GetFileOpenerCount( void ) ;
		virtual ssize_t FindFileOpenerAs( const wchar_t * pwszID ) ;
		virtual SFileOpener * GetFileOpenerAt( size_t iOpener ) ;
		virtual bool GetFileOpenerIDAt( SString& strID, size_t iOpener ) ;
		virtual void AddFileOpener
			( SFileOpener * pOpener,
				const wchar_t * pwszID, const wchar_t * pwszDefaultDir = nullptr ) ;
		virtual void RemoveFileOpener( const wchar_t * pwszID ) ;
		virtual void EnableFileOpener
					( const wchar_t * pwszID, bool fEnable ) ;
		// 書き込み可能ファイル・オープナー
		virtual bool CanOpenAllFileForWriting( void ) ;
		virtual void AcceptAllFileForWriting( bool fAllWriting ) ;
		virtual SFileOpener * GetWritableFileOpener( void ) ;
		virtual void SetWritableFileOpener( SFileOpener * pOpener ) ;
		// ファイル・パス
		virtual const SString & GetBaseFilePath( void ) const ;
		virtual void SetBaseFilePath( const wchar_t * pszFilePath ) ;
		virtual SString OffsetFilePath( const wchar_t * pszFilePath ) ;
		// ディレクトリを生成（途中のディレクトリが存在しない場合にも自動生成）
		static SError CreateFullDirectory( const wchar_t * pszPath ) ;

	protected:
		// HTTP ダウンロードエントリ
		class	DownloadFile
		{
		public:
			bool		m_flagIndirect ;
			bool		m_flagUpdatable ;
			bool		m_flagCrypt32 ;
			bool		m_flagCheckCRC32 ;
			bool		m_flagDownloaded ;
			bool		m_flagArchive ;
			bool		m_flagDynamicLoad ;
			SString		m_strID ;
			SString		m_urlIndirect ;
			SString		m_urlDownload ;
			SString		m_strLocalPath ;
			SString		m_strPassword ;
			SString		m_strDisplayName ;
			uint32_t	m_crc32 ;
			uint64_t	m_length ;
			SString		m_strDefaultDir ;
		public:
			// 構築関数
			DownloadFile( void )
				: m_flagIndirect(false),
					m_flagUpdatable(false),
					m_flagCrypt32(false),
					m_flagCheckCRC32(false),
					m_flagDownloaded(false),
					m_flagArchive(false),
					m_flagDynamicLoad(false),
					m_crc32(0), m_length(0) {}
			// ファイルのダウンロードか？ローカルコピーか？
			bool WillDownloadOnlineURL( void ) const ;
		} ;
		SXMLDocument				m_xmlDownloads ;
		SObjectArray<DownloadFile>	m_arrayDownloads ;

		SString						m_strDynamicEnvFile ;
		SXMLDocument				m_xmlDynamicEnv ;
		SXMLDocument *				m_pxmlDynamicEnv ;
		SObjectArray<DownloadFile>	m_arrayDynamicFiles ;

	public:
		// アプリケーション更新判定
		virtual SError DoCheckAppUpdate
			( SProgressiveUserInterface * pUI = NULL ) ;
		// ダウンロードエントリの有無
		virtual bool AreDownloadFiles( void ) const ;
		// ファイルのダウンロードを実行
		virtual SError DoDownloadFiles
			( bool fNoConfirm = false,
				SProgressiveUserInterface * pUI = NULL ) ;
		// システム要求チェック
		virtual SError DoCheckRequirement( void ) ;

	public:
		// 要ダウンロード書庫を追加する
		virtual DownloadFile * AddDownloadIndirectArchive
			( const wchar_t * pwszID,
				const wchar_t * pwszDisplayName,
				const wchar_t * pwszLocalPath,
				const wchar_t * pwszIndirectURL,
				const wchar_t * pwszPassword,
				bool fUpdatable, bool fCrypt32 ) ;
		virtual DownloadFile * AddDownloadArchiveFile
			( const wchar_t * pwszID,
				const wchar_t * pwszDisplayName,
				const wchar_t * pwszLocalPath,
				const wchar_t * pwszDownloadURL,
				const wchar_t * pwszPassword,
				uint32_t nCRC32, uint64_t nFileSize, bool fCrypt32 ) ;

	protected:
		DownloadFile * NewDownloadIndirectArchive
			( const wchar_t * pwszID,
				const wchar_t * pwszDisplayName,
				const wchar_t * pwszLocalPath,
				const wchar_t * pwszIndirectURL,
				const wchar_t * pwszPassword,
				bool fUpdatable, bool fCrypt32 ) ;
		DownloadFile * NewDownloadArchiveFile
			( const wchar_t * pwszID,
				const wchar_t * pwszDisplayName,
				const wchar_t * pwszLocalPath,
				const wchar_t * pwszDownloadURL,
				const wchar_t * pwszPassword,
				uint32_t nCRC32, uint64_t nFileSize, bool fCrypt32 ) ;

	public:
		// 動的ダウンロードファイルを要ダウンロード書庫に追加する
		virtual void AddDynamicDownloadArchive( const wchar_t * pwszID ) ;

	protected:
		// ダウンロード予約リストから検索
		DownloadFile * GetDownloadScheduleFileInfo( const wchar_t * pwszID ) const ;
		// 動的ダウンロード可能情報取得
		DownloadFile * GetDynamicDownloadFileInfo( const wchar_t * pwszID ) const ;
		// 動的拡張環境ファイル保存
		SError SaveDynamicEnvironmentFile( void ) ;

	protected:
		struct	DOWNLOAD_FILES
		{
			size_t		nOnlineFiles ;
			size_t		nLocalFiles ;
			uint64_t	nOnlineBytes ;
			uint64_t	nLocalBytes ;
		} ;
		// ダウンロード済み CRC チェック実行
		// (ダウンロードが必要なファイル数を返却)
		ssize_t CheckDownloadedFiles
			( DOWNLOAD_FILES * pdf, SProgressiveUserInterface * pUI ) ;
		// ファイル更新の必要性チェック
		bool CheckDownloadedFile
			( DownloadFile * pdf, bool fNoCheckCRC,
					SProgressiveUserInterface * pUI ) ;
		// ダウンロード情報ファイルをダウンロード
		SError GetDownloadFileInfo( DownloadFile * pdf, SProgressiveUserInterface * pUI ) ;
		// ファイルのダウンロードを実行
		SError DownloadAllFiles( SProgressiveUserInterface * pUI ) ;
		// ダウンロードファイルをアーカイブファイルとして追加
		void AddDownloadedArchiveOpener( void ) ;
		// ダウンロード済みファイル情報 (downloaded.xml) を読み込む
		SError LoadDownloadedInfo( void ) ;
		// ダウンロード済みファイル情報 (downloaded.xml) を書き出す
		SError SaveDownloadedInfo( void ) ;
		// ダウンロード済み情報エントリを取得
		SXMLDocument * GetDownloadedInfo( const wchar_t * pwszLocalPath ) ;
		// ダウンロード済み情報エントリを生成
		SXMLDocument * CreateDownloadedInfo( const wchar_t * pwszLocalPath ) ;

	public:	// DoDownloadFiles 用 UI
		// 進行状況ダイアログ表示
		virtual void CreateProgressiveDialog( void ) ;
		// 進行状況ダイアログ消去
		virtual void CloseProgressiveDialog( void ) ;
		// 進行状況ダイアログキャプション設定
		virtual void SetProgressiveCaption( const wchar_t * pwszCaption ) ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveMessage( const wchar_t * pwszMessage ) ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveStatus( int nCurrent, int nTotal ) ;
		// ユーザーがキャンセル操作したか？
		virtual bool IsProgressiveCanceled( void ) ;
		// メッセージボックス表示
		virtual int DoMessageBox
			( const wchar_t * pwszMsg,
				const wchar_t * pwszCaption = NULL,
						int nStyles = msgboxStyleOk ) ;

	} ;
	#endif

}

#endif
