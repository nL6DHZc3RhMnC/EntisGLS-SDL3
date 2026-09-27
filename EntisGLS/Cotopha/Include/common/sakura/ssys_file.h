
#if	!defined(__SAKURA2_FILE_H__)
#define	__SAKURA2_FILE_H__

namespace	SSystem
{
	#if	defined(__COTOPHA__)
	class	native File ;
	#endif
	class	SFileInterface ;

	//////////////////////////////////////////////////////////////////////////
	// 入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SInputStream	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SInputStream, ESLObject )

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) = 0 ;

	public:
		// 文字列読み込み
		SError ReadString( SString & strBuf ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SOutputStream	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SOutputStream, ESLObject )

	public:
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) = 0 ;

	public:
		// 文字列書き出し
		SError WriteString( const SString & strBuf ) ;
		size_t WriteEncodedString
			( const SString & strBuf,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) ;
		size_t WriteEncodedString
			( const wchar_t * pszStr,
				ssize_t nLength = -1,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ファイル・オープン・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SFileOpener	: public SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFileOpener, SObject )

	public:
		// 属性
		enum	OpenFlag
		{
			modeCreateFlag		= 0x0001,
			modeReadFlag		= 0x0002,
			modeWriteFlag		= 0x0004,
			modeReadWriteFlag	= 0x0006,
			shareReadFlag		= 0x0010,
			shareWriteFlag		= 0x0020,
			modeCreateDirFlag	= 0x0080,
			modeStreaming		= 0x0100,
			modeCreateFile		= (modeCreateFlag | modeWriteFlag),
			modeCreate			= (modeCreateFile | modeCreateDirFlag),
			modeRead			= modeReadFlag,
			modeWrite			= modeWriteFlag,
			modeReadWrite		= (modeReadFlag | modeWriteFlag),
			shareRead			= (shareReadFlag | modeReadFlag),
			shareWrite			= (shareWriteFlag | modeWriteFlag),
			shareReadWrite		= (shareRead | shareWrite),
			shifterPermission	= 16,
			maskPermission		= 0x01FF0000,
		} ;
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) = 0 ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) = 0 ;
		// ファイル状態
		struct	State
		{
			uint32_t	bitFields ;
			uint32_t	bitAttributes ;
			uint64_t	nFileSize ;
			DATE_TIME	dtAccessed ;
			DATE_TIME	dtModified ;
			DATE_TIME	dtCreated ;
		} ;
		enum	StateField
		{
			fieldAttributes		= 0x0001,
			fieldFileSize		= 0x0002,
			fieldAccessedTime	= 0x0010,
			fieldModifiedTime	= 0x0020,
			fieldCreatedTime	= 0x0040,
		} ;
		enum	FileAttributes
		{
			attrDirectory	= 0x80000000,
			attrHidden		= 0x40000000,
			permissionRUSR	=  000000400,
			permissionWUSR	=  000000200,
			permissionXUSR	=  000000100,
			permissionRWXU	=  000000700,
			permissionRGRP	=  000000040,
			permissionWGRP	=  000000020,
			permissionXGRP	=  000000010,
			permissionRWXG	=  000000070,
			permissionROTH	=  000000004,
			permissionWOTH	=  000000002,
			permissionXOTH	=  000000001,
			permissionRWXO	=  000000007,
		} ;
		virtual SError QueryState
			( const wchar_t * pszFilePath, State& state ) = 0 ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) = 0 ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) = 0 ;
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;

	protected:
		static SFileOpener *	m_pDefaultOpener ;
	public:
		// システム規定のファイルオープン
		static SFileInterface * DefaultNewOpenFile
				( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// システム規定のファイル削除
		static SError DefaultRemoveFile( const wchar_t * pszFilePath ) ;
		// システム規定の仮想パスから直接パスへ変換
		static SError DefaultDirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
		// システム規定のファイル存在確認
		static SFileOpener * DefaultGetExisting
			( const wchar_t * pszFilePath, bool fWritable = false ) ;
		static bool DefaultIsExisting( const wchar_t * pszFilePath ) ;
		// デフォルトのオープナー設定
		static void SetDefaultOpener( SFileOpener * pOpener )
			{
				m_pDefaultOpener = pOpener ;
			}
		// デフォルトのオープナー取得
		static SFileOpener * GetDefaultOpener( void )
			{
				return	m_pDefaultOpener ;
			}
		// ワイルドカード判定
		static bool IsMatchWildCardTo
			( const wchar_t * pwszWildCard, const wchar_t * pwszFileName ) ;
	protected:
		static bool IsMatchFileChar( wchar_t wch1, wchar_t wch2 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ファイル・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SFileInterface	: public SFileOpener,
								public SInputStream, public SOutputStream
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SFileInterface, SFileOpener, SInputStream, SOutputStream )
		// 構築関数
		SFileInterface( void ) ;
		// 構築関数（ダミー）
		SFileInterface( const SFileInterface& file ) ;

	public:
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;

	public:
		// File 変換
		#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
		#else
		virtual SFileInterface * GetFileObject( void ) ;
		#endif
		// ESLObject キャスト
		operator ESLObject * ( void )
		{
			return	(SFileOpener*) this ;
		}

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const = 0 ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) = 0 ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) = 0 ;

	public:
		// シーク基準
		enum	SeekOrigin
		{
			FromBegin,
			FromCurrent,
			FromEnd,
		} ;
		// シーク可能か否か？
		virtual bool IsSeekable( void ) const = 0 ;
		// ファイル長の取得
		virtual int64_t GetLength( void ) const = 0 ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) = 0 ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const = 0 ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) = 0 ;
	} ;

	#if	defined(__COTOPHA__)
	class	native File
	{
	public:
		// デフォルトのファイル名
		enum	DefaultName<String>
		{
			StandardOutput	= "<stdout>",
			StandardInput	= "<stdin>",
		} ;
		// 規定ディレクトリ識別子
		enum	DefaultDirectory<String>
		{
			CurrentDirectory		= "Current",
			WindowsDirectory		= "Windows",
			WindowsSystemDirectory	= "WindowsSystem",
			WindowsStartMenu		= "WindowsStartMenu",
			WindowsCommonStartMenu	= "WindowsCommonStartMenu",
			WindowsDesktop			= "WindowsDesktop",
			WindowsProgramFiles		= "WindowsProgramFiles",
			ApplicationData			= "AppData",
			UserDocuments			= "UserDocuments",
			UserMusic				= "UserMusic",
			UserPictures			= "UserPictures",
			UserVideos				= "UserVideos",
			ApplicationInstalled	= "AppInstalled",
			AndroidLocalFiles		= "AndroidLocalFiles",
			AndroidExternalStorage	= "AndroidExternalStorage",
			AndroidExternalStoragePrivate	= "AndroidExternalStoragePrivate",
		} ;
		// ファイルを開く
		static native File * NewOpen
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルは存在しているか？
		static native bool IsExistingFile( const wchar_t * pszFilePath ) ;
		// ファイル状態
		static native SError QueryFileState
			( const wchar_t * pszFilePath, SFileOpener::State& state ) ;
		// ファイルを削除する
		static native SError RemoveFile( const wchar_t * pszFilePath ) ;
		// ファイルの一覧取得
		static native void ListFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		static native void ListDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリ作成
		static native SError CreateDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリ削除
		static native SError RemoveDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		static native SError RenameFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// 規定ディレクトリ取得
		static native SError GetDefaultDirectory
			( SString& strDirPath,
				const wchar_t * pwszPlacementId,
				const wchar_t * pwszOption = NULL ) ;

	public:
		// ファイルオブジェクトの複製
		native File * Duplicate( void ) const ;

	public:
		// ファイルから読み込み
		native size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		native size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:
		// シーク可能か否か？
		native bool IsSeekable( void ) const ;
		// ファイル長の取得
		native int64_t GetLength( void ) const ;
		// ファイルポインタを移動
		native int64_t Seek
			( int64_t posFile,
				SFileInterface::SeekOrigin
						seekFrom = SFileInterface::FromBegin ) ;
		// ファイルポインタを取得
		native int64_t GetPosition( void ) const ;
		// ファイルの終端を現在の位置に設定する
		native SError SetEndOfFile( void ) ;

	public:
		// ファイル時刻取得
		native SError GetFileTime( SSystem::DATE_TIME& time ) ;
		// ファイル時刻設定
		native SError SetFileTime( const SSystem::DATE_TIME& time ) ;
	} ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 標準ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SFile	: public SFileInterface
	{
	protected:
	#if	defined(__COTOPHA__)
		File *	m_pFile ;
	#elif	defined(__PLATFORM_WINDOWS__)
		HANDLE		m_hFile ;
	#else
		int			m_fdFile ;
	#endif
		long int	m_nFlags ;
		SString		m_strFilePath ;

	public:
		// デフォルトファイル名（File 互換性のため）
		struct	DefaultName
		{
			static const wchar_t *	StandardOutput ;
			static const wchar_t *	StandardInput ;
		} ;
		// 規定ディレクトリ識別子（File 互換性のため）
		struct	DefaultDirectory
		{
			static const wchar_t *	CurrentDirectory ;
			static const wchar_t *	WindowsDirectory ;
			static const wchar_t *	WindowsSystemDirectory ;
			static const wchar_t *	WindowsStartMenu ;
			static const wchar_t *	WindowsCommonStartMenu ;
			static const wchar_t *	WindowsDesktop ;
			static const wchar_t *	WindowsProgramFiles ;
			static const wchar_t *	ApplicationData ;
			static const wchar_t *	UserDocuments ;
			static const wchar_t *	UserMusic ;
			static const wchar_t *	UserPictures ;
			static const wchar_t *	UserVideos ;
			static const wchar_t *	ApplicationInstalled ;
			static const wchar_t *	AndroidLocalFiles ;
			static const wchar_t *	AndroidExternalStorage ;
			static const wchar_t *	AndroidExternalStoragePrivate ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFile, SFileInterface )
		// 構築関数
		SFile( void ) ;
		#if	defined(__COTOPHA__)
		SFile( const SString & strFilePath,
					File * pFile, long int nFlags ) ;
		#elif	defined(__PLATFORM_WINDOWS__)
		SFile( const SString & strFilePath,
					HANDLE hFile, long int nFlags ) ;
		#else
		SFile( const SString & strFilePath,
					int fdFile, long int nFlags ) ;
		#endif
		// 消滅関数
		virtual ~SFile( void ) ;
		// File 変換
		#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
		#endif

	public:
		// ファイルを開く
		virtual SError Open
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルを閉じる
		virtual void Close( void ) ;

	public:
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		static SFile * NewOpenFileAbsPath
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		static bool IsExistingAbsPath( const wchar_t * pszFilePath ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		static void ListSubFilesAbsPath
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
		static void ListSubDirectoriesAbsPath
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
		// オフセットパスを取得する
		SString OffsetPath( const wchar_t * pszFilePath ) ;
		// ベースパスを設定する
		void SetFilePath( const wchar_t * pszFilePath ) ;

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:
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
		virtual SError SetEndOfFile( void ) ;

	public:
		// ファイル（更新）時刻取得
		virtual SError GetFileTime( DATE_TIME& time ) ;
		// ファイル（更新）時刻設定
		virtual SError SetFileTime( const DATE_TIME& time ) ;
		// ファイルパス取得
		const SString & GetFilePath( void ) const
			{
				return	m_strFilePath ;
			}

		// 生成
		static SFile * NewOpen
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルは存在しているか？
		static bool IsExistingFile( const wchar_t * pszFilePath ) ;
		// ファイル状態
		static SError QueryFileState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルを削除する
		static SError RemoveFile( const wchar_t * pszFilePath ) ;
		// ファイルの一覧取得
		static void ListFiles
			( SObjectArray<SString>& listFiles,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		static void ListDirectories
			( SObjectArray<SString>& listDirs,
					const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリを作成する
		static SError CreateDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		static SError RemoveDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		static SError RenameFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// 規定ディレクトリ取得
		static SError GetDefaultDirectory
			( SString& strDirPath,
				const wchar_t * pwszPlacementId,
				const wchar_t * pwszOption = NULL ) ;

		// 指定ディレクトリパスのディレクトリを作成する（親ディレクトリを含む）
		static SError CreateFullDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SFile	File ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// メモリ参照・ファイルインターフェース
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native MemoryReferenceFile	: public File
	{
	public:
		// メモリ関連付け
		native void AttachMemory( void * ptrMemory, size_t nLength ) ;
	} ;
	#endif

	class	SMemoryReferenceFile	: public SFileInterface
	{
	protected:
		uint8_t *	m_pbytMemory ;
		size_t		m_nLength ;
		size_t		m_nPosition ;

		#if	defined(__COTOPHA__)
		MemoryReferenceFile *	m_pMemFile ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SMemoryReferenceFile, SFileInterface )
		// 構築関数
		SMemoryReferenceFile( void ) ;
		SMemoryReferenceFile( const SMemoryReferenceFile& memfile ) ;
		// 消滅関数
		virtual ~SMemoryReferenceFile( void ) ;
		// メモリ関連付け
		virtual void AttachMemory( void * ptrMemory, size_t nLength ) ;
		// メモリ取得
		uint8_t * GetMemory( void ) const
		{
			return	m_pbytMemory ;
		}
		// 代入
		const SMemoryReferenceFile& operator = ( const SMemoryReferenceFile& mem ) ;
		// File 変換
		#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
		#endif

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:
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
		virtual SError SetEndOfFile( void ) ;
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SMemoryReferenceFile	MemoryReferenceFile ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// URL オープン・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SVirtualURLOpener	: public SFileOpener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SVirtualURLOpener, SFileOpener )
		// 構築関数
		SVirtualURLOpener( void ) ;
		// 消滅関数
		virtual ~SVirtualURLOpener( void ) ;

	public:
		enum	SchemeFlag
		{
			schemeOverNetwork	= 0x0001,
		} ;
		struct	SCHEME
		{
			const wchar_t *	pwszName ;
			uint32_t		nFlags ;
			SFileOpener *	pOpener ;
		} ;
		SArray<SCHEME>	m_vectorScheme ;

	public:
		// スキーム判定
		ssize_t FindScheme( const wchar_t * pszFilePath ) const ;
		// スキーム取得
		const SCHEME * GetSchemeAt( size_t iScheme ) const ;
		// スキームを除去したパスを取得
		static const wchar_t * GetRidPathOfScheme
			( const wchar_t * pszFilePath, const SCHEME& scheme ) ;

	public:
		// スキーム追加登録
		void RegisterScheme
			( const wchar_t * pwszName,
				SFileOpener * pOpener, uint32_t nFlags = 0 ) ;
		// スキーム全削除
		void UnregisterAllScheme( void ) ;
		// オフセット・オープナー生成
		SFileOpener * NewOffsetOpener
			( const wchar_t * pszFilePath, wchar_t wchSeparator ) ;

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
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
	} ;

	// デフォルトの基底オープナー
	extern ESL_DLL_EXPORT	SVirtualURLOpener	g_defURLOpener ;


	//////////////////////////////////////////////////////////////////////////
	// 標準ファイル・オープン・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SStandardFileOpener	: public SFileOpener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SStandardFileOpener, SFileOpener )

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
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pwszNewPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 相対パス・オープン・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SOffsetFileOpener	: public SFileOpener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SOffsetFileOpener, SFileOpener )
		// 構築関数
		SOffsetFileOpener
			( const wchar_t * pszBasePath,
				wchar_t wchSeparator,
				SFileOpener * pOpener, bool flagOwner ) ;
		// 消滅関数
		virtual ~SOffsetFileOpener( void ) ;

	protected:
		SString			m_strBasePath ;
		wchar_t			m_wchSeparator ;
		SFileOpener *	m_pOpener ;
		bool			m_flagOwner ;

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
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pwszNewPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
		// オフセットパスを取得する
		SString OffsetPath( const wchar_t * pszFilePath ) ;
		// ベースパスを取得する
		const SString& GetBasePath( void ) const
		{
			return	m_strBasePath ;
		}
		SString GetFullBasePath( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// オープナー参照ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SSmartFile	: public SFileInterface
	{
	protected:
		SSyncReference		m_refOpener ;
		SFileInterface *	m_pFile ;
		bool				m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSmartFile, SFileInterface )
		// 構築関数
		SSmartFile( void ) : m_pFile(NULL), m_flagOwner(false) {}
		SSmartFile
			( SFileOpener * pOpener,
				SFileInterface * pFile, bool flagOwner = true ) ;
		// 消滅関数
		virtual ~SSmartFile( void ) ;
		// ファイルオープナーを関連付ける
		virtual void AttachFileOpener( SFileOpener * pOpener ) ;
		// ファイルを関連付ける
		virtual void AttachFile
			( SFileInterface * pFile, bool flagOwner = true ) ;
		// ファイルの参照を解除する
		virtual void Close( void ) ;
		// SFileOpener 取得
		SFileOpener * GetFileOpener( void ) const ;

	public:	// SFileOpener オーバーライド
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
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pwszNewPath ) ;
		// システム上の直接パスを取得する
		virtual SError DirectPathOf
			( SString& strDirectPath, const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;

	public:	// SFileInterface オーバーライド
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
		virtual SError SetEndOfFile( void ) ;

	} ;


} ;


#endif
