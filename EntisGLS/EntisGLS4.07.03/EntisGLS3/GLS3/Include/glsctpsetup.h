

//////////////////////////////////////////////////////////////////////////////
// セットアップコンポーネント
//////////////////////////////////////////////////////////////////////////////

class	ECSSetup	: public ECSObject, public EGLSThread
{
public:
	// 構築関数
	ECSSetup( void ) ;
	// 消滅関数
	virtual ~ECSSetup( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSSetup, ECSObject, EGLSThread )

public:
	// ファイルリスト管理クラス
	class	EFileList	: public	EDescription
	{
	public:
		// 構築関数
		EFileList( void ) ;
		// 消滅関数
		virtual ~EFileList( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( EFileList, EDescription ) ;
	public:
		// ディレクトリを追加する
		EDescription * AddDirectory
			( const wchar_t * pwszBasePath, int nCreated ) ;
		// アーカイブディレクトリを追加する
		ESLError AddArchiveDirectory
			( const wchar_t * pwszBasePath,
				const char * pszPassword, int nType = 0 ) ;
		// ファイルを追加する
		EDescription * AddFile
			( const wchar_t * pwszBasePath,
				const wchar_t * pwszFileName ) ;
		// ファイル名部分を取得する
		static const wchar_t * GetFileNamePart( const wchar_t * pwszFilePath ) ;
		// ディレクトリ部分を取得する
		static EWideString GetFileDirectoryPart( const wchar_t * pwszFilePath ) ;
	public:
		// アーカイブファイルを追加する
		ESLError AddArchiveTree
			( const wchar_t * pwszBasePath,
				const wchar_t * pwszSrcArchive,
					const char * pszPassword ) ;
		ESLError AddArchiveSubTree
			( EDescription * pdscDir, const wchar_t * pwszBasePath,
				ERISAArchive & arcfile,
				const wchar_t * pwszSrcFileBase, const char * pszPassword ) ;
		// ディレクトリを追加する
		ESLError AddDirectoryTree
			( const wchar_t * pwszBasePath,
				const wchar_t * pwszSrcDirectory ) ;
	public:
		// 容量を計算する
		ESLError MeasureSize( UINT64 & nBytes ) ;
		static ESLError MeasureSizeDirectory
			( EDescription * pdscDir, UINT64 & nBytes ) ;
	} ;
	// インストール情報
	struct	UNINSTALL_INFO
	{
		EWideString	wstrDisplayName ;
		EWideString	wstrDisplayIcon ;
		EWideString	wstrUninstallCmdLine ;
		EWideString	wstrUninstallPath ;
		EWideString	wstrInstallLocation ;
		EWideString	wstrPublisher ;
		EWideString	wstrVersionMajor ;
		EWideString	wstrVersionMinor ;
	} ;

protected:
	EFileList		m_flstLog ;		// インストールされたファイルのログ
	EFileList		m_flstSetup ;	// セットアップするファイルのリスト

	EWideString		m_wstrInstDir ;
	bool			m_fRebootToDelete ;

	EDescription *	m_pdscCurrentDir ;
	int				m_iCurrent ;
	UINT64			m_nTotalInstallationBytes ;
	UINT64			m_nTotalInstalledBytes ;
	UINT64			m_nCurrentFileBytes ;
	UINT64			m_nCurrentCopiedBytes ;
	ESLCriticalSection	m_csStatus ;

	ESLFileObject *	m_pdstfile ;
	ESLFileObject *	m_dstfile ;
	ERISAArchive	m_dstarcf ;
	bool			m_fDstArchive ;
	EWideString		m_wstrDstCurrentDir ;
	EWideString		m_wstrDstFile ;
	EString			m_strDstPassword ;
	DWORD			m_dwDstEncodeType ;

	ESLFileObject *	m_psrcfile ;
	ESLFileObject *	m_srcfile ;
	ERISAArchive	m_srcarcf ;
	bool			m_fSrcArchive ;
	EWideString		m_wstrSrcFile ;
	EWideString		m_wstrSrcArchivePath ;

	HANDLE			m_hThreadReady ;
	HANDLE			m_hFinishedCopy ;
	ESLError		m_errCopyResult ;
	EWideString		m_wstrErrMsg ;

	enum	ThreadMessage
	{
		tmBeginCopy	= WM_USER,
		tmQuitThread,
	} ;

	HANDLE			m_hBootMutex ;

	ESLProcess		m_processSub ;

public:
	struct	INST_DLG_PARAM
	{
		ECSSetup *	pSetup ;
		ECSWindow *	pWindow ;
		HANDLE		hEventOkCancel ;
		HANDLE		hEventReady ;
		int			nEventResult ;
		HWND		hDlg ;
		EWideString	wstrInstDir ;
		EWideString	wstrDefSubDir ;
		EWideString	wstrCaption ;
		DWORD		dwOptionFlags ;
	} ;
protected:
	INST_DLG_PARAM *	m_pInstDlgParam ;

public:
	// セットアップするファイルリストを読み込む
	ESLError ReadSetupList( ESLFileObject & file ) ;
	// セットアップするファイルリストを書き出す
	ESLError WriteSetupList( ESLFileObject & file ) ;
	// インストールログファイルを読み込む
	ESLError ReadInstalledLog( ESLFileObject & file ) ;
	// インストールログファイルを書き出す
	ESLError WriteInstalledLog( ESLFileObject & file ) ;
	// インストール先ディレクトリを追加する
	EDescription * AddInstallDirectory
		( const wchar_t * pwszBasePath ) ;
	// インストール先アーカイブディレクトリを追加する
	ESLError AddInstallArchiveDirectory
		( const wchar_t * pwszBasePath,
			const char * pszPassword, int nType = 0 ) ;
	// インストール先ファイルを追加する
	EDescription * AddInstallFile
		( const wchar_t * pwszBasePath,
			const wchar_t * pwszFileName ) ;
	// インストール先アーカイブファイルを追加する
	ESLError AddInstallArchiveTree
		( const wchar_t * pwszBasePath,
			const wchar_t * pwszSrcArchive,
				const char * pszPassword ) ;
	// インストール先ディレクトリを追加する
	ESLError AddInstallDirectoryTree
		( const wchar_t * pwszBasePath,
			const wchar_t * pwszSrcDirectory ) ;
	// 容量を計算する
	ESLError MeasureInstallSize( UINT64 & nBytes ) ;
	// 起動チェック
	bool BootCheck( const char * pszCheckName, bool fDisableBoot = false ) ;
	// 起動チェック解放
	void ReleaseBootCheck( void ) ;

public:
	// ProductID を取得する
	static EString GetWindowsProductID( void ) ;
	// MD5 ハッシュを生成する
	static EString MakeMD5Digest( const char * pszBuf, int nLength = -1 ) ;
	// CRC32 を生成する
	static DWORD CalcCRC32( const char * pszBuf, int nLength = -1 ) ;
	// 32bitチェックサムを生成する
	static DWORD CheckSum32( const char * pszBuf, int nLength = -1 ) ;
	// デスクトップパスを取得する
	static EString GetDesktopDirectory( void ) ;
	// スタートメニューパスを取得する
	static EString GetStartMenuDirectory( void ) ;
	static EString GetCommonStartMenuDirectory( void ) ;
	// アプリケーションデータパスを取得する
	static EString GetAppDataDirectory( void ) ;
	// Windows ディレクトリを取得する
	static EString GetWindowsDirectory( void ) ;
	// モジュール（インストーラー基底）パスを取得する
	static EString GetCurrentModulePath( void ) ;
	// ディスクボリュームを取得する
	static ESLError GetDiskVolumeName
		( const char * pszDrv, EString & strVolumeName ) ;
	// ディスクのシリアル番号を取得する
	static ESLError GetDiskSerialNumber
		( const char * pszDrv, DWORD & dwSerialNumber ) ;
	// ディスクの空き容量を取得する
	static ESLError GetDiskFreeSpace
		( const char * pszDrv, UINT64 & nFreeAvailable,
				UINT64 & nTotalBytes, UINT64 & nFreeSpace ) ;
	// インストール情報を取得する
	static ESLError GetUninstallInfo
		( const wchar_t * pwszRegName, UNINSTALL_INFO & uninst_info ) ;
	// ショートカットファイルを作成する
	static ESLError CreateShortcutFile
		( const wchar_t * pwszShortcutFile,
			const char * pszLinkFile, const char * pszArg ) ;
	// シェルでファイルを開く
	static ESLError ShellExecute
		( const wchar_t * pwszVerb,
			const wchar_t * pwszFile, const wchar_t * pwszParameter = NULL ) ;
	// Win32 実行可能ファイルを起動する
	ESLError ExecuteProcess
		( const wchar_t * pwszFile, const wchar_t * pwszParameter,
			DWORD dwFlags, const wchar_t * pwszEnvironment = NULL,
			const wchar_t * pwszCurrentDirectory = NULL ) ;
	// Execute で起動したプロセスの完了を待つ
	ESLError GetExitCodeExecute( DWORD dwTimeout, DWORD * pExitCode ) ;
	// ディレクトリを選択する
	static ESLError BrowseForFolder
		( EWideString & wstrDir,
			const wchar_t * pwszCaption,
			ECSWindow * pWindow, HWND hwndParent = NULL ) ;
	// ファイルを選択する
	static ESLError BrowseFileDialog
		( EWideString & wstrFile,
			bool fSaveFileDialog,
			const wchar_t * pwszCaption,
			const wchar_t * pwszFilters,
			ECSWindow * pWindow, HWND hwndParent = NULL ) ;
	// システムを再起動させる
	static void RebootWindows( void ) ;

public:
	// CRC32 計算用クラス
	class	CRC32
	{
	private:
		DWORD	crc ;
	public:
		static const DWORD	CRC32Table[256] ;
	public:
		CRC32( void ) : crc(0xFFFFFFFF) {}
		DWORD GetCRC32( void ) const
			{	return	crc ^ 0xFFFFFFFF ;	}
		void Stream( const BYTE * pbytBuf, int nBytes ) ;
	} ;
	// MD5 計算用クラス
	class	MD5
	{
	public:
		DWORD *	pX ;
		static const DWORD T[64] ;
	public:
		void Round_Calculate
			( const unsigned char * block,
				DWORD &A, DWORD &B, DWORD &C, DWORD &D ) ;
		void String
			( char * output, const char * string, int nLength ) ;
	public:
		inline DWORD ROTATE_LEFT( DWORD x, DWORD n)
			{
				return	(((x) << (n)) | ((x) >> (32-(n)))) ;
			}
		inline DWORD F( DWORD X, DWORD Y, DWORD Z)
			{
				return	(X & Y) | (~X & Z) ;
			}
		inline DWORD G( DWORD X, DWORD Y, DWORD Z)
			{
				return	(X & Z) | (Y & ~Z) ;
			}
		inline DWORD H( DWORD X, DWORD Y, DWORD Z)
			{
				return	X ^ Y ^ Z ;
			}
		inline DWORD I( DWORD X, DWORD Y, DWORD Z)
			{
				return	Y ^ (X | ~Z) ;
			}
		inline DWORD Round
			( DWORD a, DWORD b, DWORD FGHI, DWORD k, DWORD s, DWORD i )
			{
				return	b + ROTATE_LEFT( a + FGHI + pX[k] + T[i], s ) ;
			}
		inline void Round1
			( DWORD &a, DWORD b, DWORD c,
				DWORD d, DWORD k,  DWORD s, DWORD i )
			{
				a = Round( a, b, F(b,c,d), k, s, i ) ;
			}
		inline void Round2
			( DWORD &a, DWORD b, DWORD c,
				DWORD d, DWORD k, DWORD s, DWORD i )
			{
				a = Round( a, b, G(b,c,d), k, s, i ) ;
			}
		void Round3
			( DWORD &a, DWORD b, DWORD c,
				DWORD d, DWORD k,  DWORD s, DWORD i )
			{
				a = Round( a, b, H(b,c,d), k, s, i ) ;
			}
		void Round4
			( DWORD &a, DWORD b, DWORD c,
				DWORD d, DWORD k,  DWORD s, DWORD i )
			{
				 a = Round( a, b, I(b,c,d), k, s, i ) ;
			}
	} ;

protected:
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;
	// ファイルコピー
	void OnFileCopy( void ) ;

public:
	// 最後のエラーメッセージを取得する
	ESLError GetLastErrorMsg( EWideString & wstrErrMsg ) ;
	// ファイルのインストールを開始する
	ESLError BeginInstall( const wchar_t * pwszInstallDir ) ;
	// ファイルのインストール完了か？
	bool IsFinishedInstall( void ) const ;
	// ファイルのコピーを開始する
	ESLError InstallNextFile
		( EWideString & wstrDstPath,
			EWideString & wstrSrcPath,
			ECSContext * context = NULL ) ;
protected:
	// 現在インストールしているディレクトリの終了処理
	bool FinishInstallDirectory( void ) ;
	// 次のインストールファイルのコピーを開始する
	ESLError BeginNextCopyFile
		( EDescription * pdscNext, ECSContext * context ) ;
	// 次のインストールディレクトリの準備をする
	ESLError BeginNextInstallDirectory
		( EDescription * pdscNext, ECSContext * context ) ;
	// アーカイブ用ディレクトリテーブルを生成する
	ESLError CreateArchiveDirectory
		( ERISAArchive::EDirectory & dirList, EDescription * pdscDir ) ;
	// 指定パスのディレクトリを作成する
	int CreateDirectoryAsFilePath( const wchar_t * pwszFilePath ) ;
public:
	// ファイルコピーの進行状況を取得する
	UINT64 GetCurrentCopiedBytes( UINT64 & nFileSize ) ;
	// 全体の進行状況を取得する
	UINT64 GetTotalCopiedBytes( UINT64 & nTotalSize ) ;
	// ファイルコピーの完了を待つ
	ESLError WaitForCurrentCopy
		( DWORD dwTimeout, ECSContext * pContext = NULL ) ;
	// ファイルのインストールを終了する
	ESLError EndInstall( void ) ;
	// ディレクトリを生成する
	ESLError InstallCreateDirectory
				( const wchar_t * pwszDirPath, int nCreated ) ;
	// ショートカットファイルを作成する
	ESLError InstallCreateShortcutFile
		( const wchar_t * pwszBaseDir, const wchar_t * pwszName,
			const char * pszLinkFile, const char * pszArg ) ;
	// アンインストール情報を登録する
	ESLError RegisterUninstall
		( const wchar_t * pwszRegName,
			const UNINSTALL_INFO & uninst_info ) ;
	// アンインストール情報を取得する
	DWORD GetRegUninstallInteger32
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, DWORD nDefValue ) ;
	INT64 GetRegUninstallInteger64
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, INT64 nDefValue ) ;
	EWideString GetRegUninstallString
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, const wchar_t * pwszDefValue ) ;
	// アンインストール情報に任意に記録する
	ESLError SetRegUninstallInteger32
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, DWORD nValue ) ;
	ESLError SetRegUninstallInteger64
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, INT64 nValue ) ;
	ESLError SetRegUninstallString
		( const wchar_t * pwszRegName,
			const wchar_t * pwszValueName, const wchar_t * pwszValue ) ;

public:
	enum	InstallationDialogOptionFlags
	{
		instoptShortCutDesktop	= 0x0001,
		instoptShortCutPrograms	= 0x0002,
		instoptDisableDesktop	= 0x0100,
		instoptDisablePrograms	= 0x0200,
		instoptDisableInstDir	= 0x0400,
	} ;
	// インストールダイアログインターフェース
	ESLError CreateInstallationDialog
		( EWideString & wstrInitDir, DWORD & dwOptionFlags,
			const wchar_t * pwszCaption, ECSWindow * pWindow,
			const wchar_t * pwszDefSubDir = NULL ) ;
	// ダイアログを閉じる
	ESLError CloseInstallationDialog( void ) ;
	// インストールダイアログのキャンセル・閉じるボタンが押されたか調べる
	bool IsInstallationDialogCanceled( void ) const ;
	// インストール中のファイル名を設定する
	ESLError SetInstallationDialogFileText
				( const wchar_t * pwszFileText ) ;
	// 進行状況を更新する
	ESLError SetInstallationDialogProgress
				( int nFile, int nTotal ) ;
	// メッセージボックスを表示する
	int InstallationMessageBox
		( const wchar_t * pwszText,
			const wchar_t * pwszCaption,
			int nMBType, ECSWindow * pParentWnd ) ;

public:
	// インストールしたファイルをアンインストールする
	ESLError Uninstall( const wchar_t * pwszInstallDir ) ;
	// ログのファイルを削除する
	ESLError DeleteLogFiles
		( const wchar_t * pwszBaseDir, EDescription * pdscDir ) ;
	// ファイルを削除する
	void DeleteInstalledFile( const char * pszFilePath ) ;
	// ファイル削除に再起動が必要か？
	bool IsNecessaryRebootToDelete( void ) const
		{
			return	m_fRebootToDelete ;
		}
	// アンインストール情報を削除する
	ESLError UnRegisterUninstall
		( const wchar_t * pwszRegName ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSSetup::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[61] ;
	static const PFUNC_CALL	m_pfnCallFunc[60] ;
	// メンバ関数
	// ユーザーインターフェース関数 6
	ESLError Call_CreateInstallationDialog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseInstallationDialog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsInstallationDialogCanceled
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetInstallationDialogFileText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetInstallationDialogProgress
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InstallationMessageBox
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// インストール支援関数 16
	ESLError Call_GetFontList
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetWindowsProductID
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MakeMD5Digest
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CalcCRC32
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CheckSum32
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDesktopDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetStartMenuDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetAppDataDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetWindowsDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCurrentModulePath
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetEnvironmentVariable
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FilterEnvironmentPath
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDiskVolumeName
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDiskSerialNumber
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDiskFreeSpace
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ShellExecute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ExecuteProcess
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetExecuteExitCode
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BrowseForFolder
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BrowseFileDialog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// インストールファイルリスト関数 8
	ESLError Call_ReadInstalledLog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_WriteInstalledLog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MeasureInstallSize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallArchiveDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallArchiveTree
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallDirectoryTree
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// インストール処理関数 12
	ESLError Call_BootCheck
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ReleaseBootCheck
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetLastErrorMsg
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BeginInstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsFinishedInstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InstallNextFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCurrentCopiedBytes
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTotalCopiedBytes
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_WaitForCurrentCopy
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_EndInstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddInstallFileLog
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InstallCreateDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InstallCreateShortcutFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// レジストリ関数 8
	ESLError Call_GetUninstallInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RegisterUninstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetRegUninstallInteger32
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetRegUninstallInteger64
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetRegUninstallString
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRegUninstallInteger32
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRegUninstallInteger64
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRegUninstallString
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// アンインストール関数 4
	ESLError Call_Uninstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteInstalledFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsNecessaryRebootToDelete
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_UnRegisterUninstall
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RebootWindows
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;

