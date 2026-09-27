
#if	!defined(__SAKURAGLX_VERSION_DOWNLOADER_H__)
#define	__SAKURAGLX_VERSION_DOWNLOADER_H__	1

#include <sakuraglx/sglx_std_app.h>
#include <rosetta/rosetta.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakura/ssys_win_registry.h>
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// インストーラー・ファイルマネージャ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSetupFileManager	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSetupFileManager, ESLObject )
		// 構築関数
		SGLSetupFileManager( void ) ;
		// 消滅関数
		virtual ~SGLSetupFileManager( void ) ;

	protected:
		SSystem::SString		m_strLogFile ;
		SSystem::SXMLDocument	m_xmlSetup ;

	public:
		// インストールログファイル読み込み
		SSystem::SError LoadInstallLog( const wchar_t * pwszLogFile ) ;
		// インストールログファイル書き出し
		SSystem::SError SaveInstallLog( const wchar_t * pwszLogFile ) ;
		SSystem::SError WriteInstallLog( SSystem::SFileInterface& file ) ;
		// 読み込んでいるインストールログファイル取得
		const SSystem::SString& GetLogFilePath( void ) const
		{
			return	m_strLogFile ;
		}
		// 内容消去
		void ClearAll( void ) ;
		// ファイルとディレクトリを消去
		bool UninstallFiles( void ) ;
	protected:
		bool UninstallDirFiles
			( SSystem::SXMLDocument * pxmlDir,
						const wchar_t * pwszBaseDir ) ;

	public:
		// ディレクトリを検索
		SSystem::SXMLDocument *
			GetDirectoryAs( const wchar_t * pwszDirPath, bool flagCreate ) ;
		// ファイルを検索
		SSystem::SXMLDocument *
			GetFileAs( const wchar_t * pwszFilePath, bool flagCreate ) ;
		SSystem::SXMLDocument *
			GetFileAs( SSystem::SXMLDocument * pxmlDir,
									const wchar_t * pwszFileName ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// バージョンチェック・ダウンローダ
	//////////////////////////////////////////////////////////////////////////

	class	SGLVersionDownloader	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLVersionDownloader, ESLObject )
		// 構築関数
		SGLVersionDownloader( void ) ;
		// 消滅関数
		virtual ~SGLVersionDownloader( void ) ;

	protected:
		SSystem::SString		m_strBaseURL ;
		SSystem::SXMLDocument	m_xmlDoc ;
		SGLSetupFileManager		m_fmLog ;

	public:
		// リスナ
		class	DownloadListener	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( DownloadListener, ESLObject )
			// ダウンロード開始
			virtual void OnBeginDownload
				( const wchar_t * pwszPackageName,
					size_t iFile, size_t nFileCount,
					uint64_t nCurrentTotal, uint64_t nTotalBytes,
					const wchar_t * pwszName,
					const wchar_t * pwszDstPath, const wchar_t * pwszSrcURL ) ;
			// ダウンロード完了
			virtual void OnFinishDownload( void ) ;
			// ダウンロード進行度
			virtual SSystem::SError OnDownloading
						( uint64_t nBytes, uint64_t nTotal ) ;
			// エラー
			virtual void OnError( const wchar_t * pwszErrMsg ) ;
		} ;

	protected:
		DownloadListener *	m_pListener ;

	public:
		// インストールログファイル読み込み
		SSystem::SError LoadInstallLog( const wchar_t * pwszLogFile ) ;
		// インストールログファイル書き出し
		SSystem::SError SaveInstallLog( const wchar_t * pwszLogFile ) ;
		// インストールログ・ファイルマネージャ取得
		SGLSetupFileManager& GetInstallLog( void )
		{
			return	m_fmLog ;
		}
		// ダウンロードリスナ設定
		void AttachListener( DownloadListener * pListener ) ;

	public:
		// 定義ファイルをダウンロード
		SSystem::SError DownloadVersionList( const wchar_t * pwszURL ) ;
		// 条件に一致するバージョンを検索する
		size_t EnumMatchVersion
			( SSystem::SPointerArray
				<SSystem::SXMLDocument>& aPackages, int64_t nCurVersion ) ;
		// バージョン情報取得
		class	PackageInfo
		{
		public:
			int					m_nVersion ;
			SSystem::SString	m_strDisplayName ;
			SSystem::SString	m_strDescription ;
			size_t				m_nFileCount ;
			uint64_t			m_nTotalBytes ;
		} ;
		SSystem::SError GetPackageInfo
			( PackageInfo& pckinf, SSystem::SXMLDocument * pxmlPackage ) const ;

	protected:
		// ダウンロードファイル情報
		class	DownloadFile
		{
		public:
			SSystem::SString	m_strDstFile ;
			SSystem::SString	m_strTempFile ;
		} ;
		SSystem::SObjectArray<DownloadFile>	m_aDelayRename ;
	public:
		// ダウンロード実行
		SSystem::SError DoDownloadPackage
			( SSystem::SXMLDocument * pxmlPackage, const wchar_t * pwszInstallDir ) ;
		// 置き換えが必要なファイルの有無
		bool ShouldRenameFile( void ) const ;
		// ファイルの置き換えを実行
		SSystem::SError DoRenameFileList( void ) ;
		// 置き換え元一時ファイルを削除（書き換えに失敗したファイルを削除するため）
		void DoDeleteTemporaryFileList( void ) ;
		// 置き換えが必要なファイルリストをファイルに保存
		SSystem::SError SaveFileListToRename( const wchar_t * pwszListFile ) ;
		// 置き換えが必要なファイルリストを XML に出力
		void FormatFileListToRename( SSystem::SXMLDocument& xmlRename ) ;
		// 置き換えが必要なファイルリストを XML から入力
		void ParseFileListToRename( const SSystem::SXMLDocument& xmlRename ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ランチャー・アプリケーション
	//////////////////////////////////////////////////////////////////////////

	class	SGLLauncherApplication	: public SGLStdApplication
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLLauncherApplication, SGLStdApplication )
		// 構築関数
		SGLLauncherApplication( void ) ;
		// 消滅関数
		virtual ~SGLLauncherApplication( void ) ;

	public:
		// プロファイルパスを取得
		virtual const wchar_t *
			GetProfileFilePath( SSystem::SString& strFilePath ) const ;
		// 実行
		virtual int Run( void ) ;

	} ;


#if	!defined(__COTOPHA__)

	//////////////////////////////////////////////////////////////////////////
	// アップデーター・アプリケーション
	//////////////////////////////////////////////////////////////////////////

	class	SGLUpdaterApplication
				: public SGLStdApplication,
					public SGLVersionDownloader::DownloadListener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLUpdaterApplication, SGLStdApplication, DownloadListener )
		// 構築関数
		SGLUpdaterApplication( void ) ;
		// 消滅関数
		virtual ~SGLUpdaterApplication( void ) ;

	protected:
		SSystem::SString	m_strUpdateURL ;
		SSystem::SString	m_strAppCmdLine ;
		SSystem::SString	m_strInstallLog ;
		int64_t				m_nAppVersion ;
		bool				m_fPrivilegeWriteServer ;

		SSystem::SProgressiveDialog
							m_dlgProgress ;
		SSystem::SString	m_strPackagenName ;
		SSystem::SString	m_strFileName ;
		size_t				m_iCurFile ;
		size_t				m_nFileCount ;
		SSystem::SString	m_strMsgBase ;

	public:
		// 引数解釈
		virtual SGLError ParseCmdLine( const wchar_t * pwszArg ) ;
		// 環境設定ファイルパスを取得
		virtual const wchar_t *
			GetEnvironmentFilePath( SSystem::SString& strFilePath ) const ;
		// プロファイルパスを取得
		virtual const wchar_t *
			GetProfileFilePath( SSystem::SString& strFilePath ) const ;
		// 実行
		virtual int Run( void ) ;
		// アップデート実行
		virtual int DoUpdate( void ) ;

	public:	// DownloadListener
		// ダウンロード開始
		virtual void OnBeginDownload
			( const wchar_t * pwszPackageName,
				size_t iFile, size_t nFileCount,
				uint64_t nCurrentTotal, uint64_t nTotalBytes,
				const wchar_t * pwszName,
				const wchar_t * pwszDstPath, const wchar_t * pwszSrcURL ) ;
		// ダウンロード完了
		virtual void OnFinishDownload( void ) ;
		// ダウンロード進行度
		virtual SSystem::SError OnDownloading
					( uint64_t nBytes, uint64_t nTotal ) ;
		// エラー
		virtual void OnError( const wchar_t * pwszErrMsg ) ;

	#if	defined(__PLATFORM_WINDOWS__)
	public:
		// 書き込みサーバー用メッセージ
		enum	WindowMessage
		{
			wmShutdown	= WM_APP,
			wmConnect,
			wmDisonnect,
			wmOpenFile,
			wmCloseFile,
			wmWriteAsync,
			wmReadAsync,
			wmSeek,
			wmGetLength,
			wmTruncate,
			wmDeleteFile,
		} ;
		enum	ConnectResult
		{
			connectSuccessed	= 0x53544E45,
			connectFailed		= 0,
		} ;
		// ファイルを開くパラメータ
		struct	OpenFileParam
		{
			const wchar_t *	pwszFilePath ;
			size_t			nFileLen ;
			int64_t			nOpenFlags ;
		} ;
		// 非同期書き込みエントリ
		struct	WriteAsyncEntry
		{
			void *		ptrData ;
			uint32_t	nBytes ;
			DWORD		idThread ;
			UINT		uMsgDone ;
			LPARAM		lParam ;
		} ;
		// ファイルを削除パラメータ
		struct	DeleteFileParam
		{
			const wchar_t *	pwszFilePath ;
			size_t			nFileLen ;
		} ;

	protected:
		HWND						m_hWndServer ;		// サーバーウィンドウ
		SSystem::SArray<char>		m_bszWndClassName ;
		HANDLE						m_hConnectProcess ;
		SSystem::SFile				m_fileServ ;
		SSystem::SThread			m_threadServ ;
		SSystem::SSignalEvent		m_sigWriteReq ;
		SSystem::SSignalEvent		m_sigExitThread ;
		SSystem::SCriticalSection	m_csWriteQue ;
		SSystem::SObjectArray<WriteAsyncEntry>
									m_queWriteAsync ;

		class	ServerProc	: public SSystem::SProcedure
		{
		protected:
			SGLUpdaterApplication *	m_app ;
		public:
			// 構築関数
			ServerProc( SGLUpdaterApplication * app ) : m_app( app ) { }
			// 実行
			virtual void Run( void ) ;
		} ;
		friend class ServerProc ;

		SSystem::SSmartPointer<ServerProc>	m_procServ ;

	public:
		// 特権ファイル書き込みサーバー実行
		virtual int DoWriteServer( void ) ;
	protected:
		// ウィンドウクラス登録
		const char * RegisterWindowClass( void ) ;
		// ウィンドウ・プロシージャ
		virtual LRESULT WindowProc
			( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		static LRESULT __stdcall WindowCallbackProc
			( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		// プロセスメモリ読み込み
		bool ReadConnectedMemory
			( void * ptrDst, ulong_ptr_t addrSrc, size_t nBytes ) ;
		// プロセスメモリ書き出し
		bool WriteConnectedMemory
			( ulong_ptr_t addrDst, const void * ptrSrc, size_t nBytes ) ;

	#endif
	} ;


#if	defined(__PLATFORM_WINDOWS__)

	//////////////////////////////////////////////////////////////////////////
	// 特権ファイル書き込みクライアント
	//////////////////////////////////////////////////////////////////////////

	class	SGLPrivilegeWriteClient	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLPrivilegeWriteClient, ESLObject )
		// 構築関数
		SGLPrivilegeWriteClient( void ) ;
		// 消滅関数
		virtual ~SGLPrivilegeWriteClient( void ) ;

	protected:
		HANDLE		m_hProcess ;
		DWORD		m_dwProcessID ;
		HWND		m_hWndServ ;
		UINT		m_uMsgWritten ;

	public:
		// サーバー起動
		SSystem::SError StartupServer( const wchar_t * pwszServerCmdLine ) ;
		// サーバー終了
		SSystem::SError ShutdownServer( void ) ;
		// サーバー起動済み判定
		bool IsStandServer( void ) const ;

	protected:
		// ウィンドウ列挙
		static BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam ) ;

	public:
		// ファイル書き出しインターフェース
		class	File : public SSystem::SFileInterface
		{
		protected:
			SGLPrivilegeWriteClient *	m_ppwc ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( File, SFileInterface )
			// 構築関数
			File( SGLPrivilegeWriteClient * ppwc ) ;
			// 消滅関数
			virtual ~File( void ) ;
		public:
			// ファイルインターフェースの複製
			virtual SFileInterface * Duplicate( void ) const ;
			// ファイルから読み込み
			virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
			// ファイルへ書き込み
			virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
			// シーク可能か否か？
			virtual bool IsSeekable( void ) const ;
			// ファイル長の取得
			virtual int64_t GetLength( void ) const ;
			// ファイルポインタを移動
			virtual int64_t Seek
				( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
			// ファイルポインタを取得
			virtual int64_t GetPosition( void ) const ;
			// ファイルの終端を現在の位置に設定する
			virtual SSystem::SError SetEndOfFile( void ) ;
		} ;

	public:
		// ファイルを開く
		SSystem::SError OpenFile
			( const wchar_t * pwszFilePath, long int nFlags ) ;
		File * NewOpenFile
			( const wchar_t * pwszFilePath, long int nFlags ) ;
		// ファイルを閉じる
		void CloseFile( void ) ;
		// ファイルへ書き出す
		size_t Write( const void * ptrData, size_t nBytes ) ;
		// ポインタシーク
		int64_t Seek( int64_t pos, SSystem::SFileInterface::SeekOrigin seekFrom ) ;
		// ファイルポインタを取得
		int64_t GetPosition( void ) const ;
		// ファイル長を取得
		int64_t GetLength( void ) const ;
		// ファイルの終端を現在の位置に設定する
		SSystem::SError SetEndOfFile( void ) ;
		// ファイルを削除する
		SSystem::SError RemoveFile( const wchar_t * pwszFilePath ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// インストーラー・アプリケーション
	//////////////////////////////////////////////////////////////////////////

	class	SGLInstallerApplication
				: public SGLStdApplication,
					public SSystem::SCustomDialog::Listener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLInstallerApplication, SGLStdApplication, Listener )
		// 構築関数
		SGLInstallerApplication( void ) ;
		// 消滅関数
		virtual ~SGLInstallerApplication( void ) ;

	public:
		// 再起動移動ファイルマネージャー
		class	RebootMoveFile
		{
		protected:
			bool	m_flagLoaded ;
			SSystem::SObjectArray
				<SSystem::SRegistryKey::MultiString>	m_lstFileSet ;
		public:
			// 構築関数
			RebootMoveFile( void ) ;
			// 消滅関数
			~RebootMoveFile( void ) ;
			// リストをレジストリから取得
			SSystem::SError LoadList( void ) ;
			// リストをレジストリへ保存
			SSystem::SError SaveList( void ) ;
			// 指定移動元ファイルをリストから除外
			bool RemoveMoveSource( const wchar_t * pwszFilePath ) ;
		} ;
		// インストールモード
		enum	InstallMode
		{
			modeInstall,
			modeUpdate,
		} ;

	protected:
		InstallMode			m_mode ;
		SSystem::SString	m_strBlandName ;	// ブランド名（ディレクトリに使用）
		SSystem::SString	m_strAppName ;		// アプリ名（ディレクトリに使用）
		SSystem::SString	m_strRegName ;		// インストールレジストリ名
		SSystem::SString	m_strInstLogFile ;	// インストールログファイル名
		SSystem::SString	m_strUninst ;		// アンインストーラー
		SGLSetupFileManager	m_fmLog ;			// インストールログ

		SSystem::SString	m_strInstDir ;		// インストール先パス
		bool	m_flagShortcutDesktop ;			// デスクトップにショートカット作成
		bool	m_flagShortcutStartMenu ;		// スタートメニューにショートカット作成

		SSystem::SString	m_strOperation ;		// 実行種別
		SSystem::SString	m_strInlineRSSource ;	// インライン Rosetta スクリプト
		Rosetta::RSVirtualMachine	m_vmRosetta ;	// Rosetta 仮想マシン

		class	FileInfo
		{
		public:
			SSystem::SString	m_strFilePath ;
			int64_t				m_nFileLength ;
		} ;
		class	InstallFileInfo
		{
		public:
			bool				m_flagOnline ;
			SSystem::SString	m_strDstDir ;	// インストール先（相対）
			SSystem::SString	m_strSrcDir ;	// インストール元
			SSystem::SObjectArray<FileInfo>
								m_aSrcFiles ;	// インストール元ファイル
			SSystem::SSmartPointer
					<SSystem::SFileOpener>
								m_pOpener ;		// 書庫ファイル
			int64_t				m_nTotalBytes ;	// 合計バイト数
			bool				m_flagEncrypt ;	// 簡易暗号化
			bool				m_flagSrcCrypt ;	// 元ファイルが簡易暗号化されている
			SSystem::SString	m_strSrcPassword ;	// 元ファイルの簡易暗号化パスワード
		public:
			InstallFileInfo( void )
				: m_nTotalBytes( 0 ), m_flagEncrypt( false ),
					m_flagSrcCrypt( false ) { }
		} ;
		SSystem::SObjectArray<InstallFileInfo>
							m_lstInstFiles ;
		int64_t				m_nTotalFileBytes ;	// 全ファイル容量合計
		SSystem::SCustomDialog *
							m_pDlg ;
		bool				m_flagInstalling ;	// インストール処理中
		bool				m_flagCancelDlg ;	// インストールキャンセル

	public:
		// 引数解釈
		virtual SGLError ParseCmdLine( const wchar_t * pwszArg ) ;
		// 環境設定ファイルパスを取得
		virtual const wchar_t *
			GetEnvironmentFilePath( SSystem::SString& strFilePath ) const ;
		// プロファイルパスを取得
		virtual const wchar_t *
			GetProfileFilePath( SSystem::SString& strFilePath ) const ;
		// アプリケーション準備
		virtual SGLError StartUpApp( void ) ;
		// 実行
		virtual int Run( void ) ;

	protected:
		SSystem::SString	m_strUUIDBaseDir ;
		SSystem::SString	m_strUUIDFileName ;
		SSystem::SString	m_strUUIDPassword ;

		// UUID ストレージ保存用ベースディレクトリ（オフセットパス）
		virtual SSystem::SString GetUUIDStorageBaseDirectory( void ) ;
		// UUID ストレージ保存用ファイル名
		virtual SSystem::SString GetUUIDStorageFileName( void ) ;
		// UUID ストレージ保存用パスワード
		virtual SSystem::SString GetUUIDCryptyPassword( void ) const ;
		// UUID をストレージに保存するか？（false では読み込みのみ）
		virtual bool IsSaveUUIDintoStorage( void ) const ;

	public:
		// UUID ストレージ保存用パス設定
		void SetUUIDStoragePath
			( const wchar_t * pwszBaseDir,
				const wchar_t * pwszFileName, const wchar_t * pwszPassword ) ;

	public:
		// インストールファイル追加
		void AddInstallFiles
			( const wchar_t * pwszDstDir,
				const wchar_t * pwszSrcFiles,
				bool flagSubDirectory = true, bool flagEncrypt = false,
				const wchar_t * pwszSrcCryptPass = NULL,
				const wchar_t * pwszSrcArchive = NULL ) ;
	protected:
		void AddInstallSubFiles
			( InstallFileInfo& ifi,
				const wchar_t * pwszSrcFiles, bool flagSubDirectory ) ;
	public:
		// 定義されたショートカット作成
		void InstallShortcutFiles
			( const wchar_t * pwszDstDir, const wchar_t * pwszEnvPath ) ;
		// ショートカット作成
		static bool CreateShortcutFile
			( const wchar_t * pwszFilePath,
				const wchar_t * pwszName,
				const wchar_t * pwszLinkPath,
				const wchar_t * pwszParameters = NULL,
				const wchar_t * pwszWorkDir = NULL ) ;
		// インストール情報をレジストリに登録
		static void RegisterUninstall
			( const wchar_t * pwszRegName,
				const wchar_t * pwszDispName,
				const wchar_t * pwszCmdLine,
				const wchar_t * pwszInstDir,
				const wchar_t * pwszPublisher ) ;
		// インストール先ディレクトリ取得
		bool GetInstallLocation( SSystem::SString& strInstDir ) const ;
		static bool GetInstallLocation
			( const wchar_t * pwszRegName, SSystem::SString& strInstDir ) ;
		// ファイルを開く
		void ShellOpenFiles( const wchar_t * pwszEnvPath ) ;

	public:
		// アンインストール実行
		void DoUninstall( Window * pParentWnd = NULL ) ;
		// インストーラー・ダイアログ入力
		int DoModal( Window * pParentWnd = NULL ) ;

	protected:
		// 参照ボタン
		static bool BrowseButtonCallback
			( SSystem::SCustomDialog& dlg,
				const SSystem::SCustomDialog::ElementInfo& item,
				int code, void * instance ) ;
		// インストールボタン
		static bool InstallButtonCallback
			( SSystem::SCustomDialog& dlg,
				const SSystem::SCustomDialog::ElementInfo& item,
				int code, void * instance ) ;
		static void InstallButtonThreadProc( void * pInstance ) ;
		void InstallButtonProc( void ) ;
		// キャンセルボタン
		static bool CancelButtonCallback
			( SSystem::SCustomDialog& dlg,
				const SSystem::SCustomDialog::ElementInfo& item,
				int code, void * instance ) ;
		// ダイアログキャンセル処理
		virtual bool OnCancel( SSystem::SCustomDialog& dlg ) ;

		friend class	RSInstallerClass ;
	} ;

	class	RSInstallerClass	: public Rosetta::RSClass
	{
	protected:
		SGLInstallerApplication *	m_app ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInstallerClass, RSClass )
		// 構築関数
		RSInstallerClass
			( SGLInstallerApplication * app, RSClass * pClass,
				const wchar_t * pwszClassName = L"Installer" ) ;

	public:
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( Rosetta::RSContext& context ) ;

	public:
		// void doInstall( WindowSprite window )
		static RSObject * method_doInstall
			( Rosetta::RSContext& context, void * pInstace,
				Rosetta::RSObject* pThis,
				Rosetta::RSObject** ppArg, size_t count ) ;
		// void doUninstall( WindowSprite window )
		static RSObject * method_doUninstall
			( Rosetta::RSContext& context, void * pInstace,
				Rosetta::RSObject* pThis,
				Rosetta::RSObject** ppArg, size_t count ) ;
		// String getInstalledPath()
		static RSObject * method_getInstalledPath
			( Rosetta::RSContext& context, void * pInstace,
				Rosetta::RSObject* pThis,
				Rosetta::RSObject** ppArg, size_t count ) ;
		// void prepareUUID
		//	( String strBaseDir,
		//		String strFileName, String strPassword = null ) ;
		static RSObject * method_prepareUUID
			( Rosetta::RSContext& context, void * pInstace,
				Rosetta::RSObject* pThis,
				Rosetta::RSObject** ppArg, size_t count ) ;
	} ;

#endif
#endif

}

#endif
