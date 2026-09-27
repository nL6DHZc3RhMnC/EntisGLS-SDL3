
#if	!defined(__GLS4_LOQUATY_FILE_H__)
#define	__GLS4_LOQUATY_FILE_H__

namespace	Loquaty
{
	//////////////////////////////////////////////////////////////////////////
	// Loquaty::LPureFile -> SSystem::SFileInterface
	//////////////////////////////////////////////////////////////////////////

	class	SLoquatyFile	: public SSystem::SFileInterface
	{
	protected:
		LFilePtr	m_pFile ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SLoquatyFile, SFileInterface )
		// 構築関数
		SLoquatyFile( LFilePtr pFile ) ;

	public:	// SInputStream
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;

	public:	// SOutputStream
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:	// SFileOpener
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		static long OpenFlagsGLS4ToLoquaty( long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SSystem::SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		static void StateLoquatyToGLS4
			( const LDirectory::State& lstate, SFileOpener::State& state ) ;
		static void DateTimeLoquatyToGLS4
			( const LDateTime& ldate, SSystem::DATE_TIME& date ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SSystem::SObjectArray<SSystem::SString>& listFiles,
					const wchar_t * pszDirPath = nullptr ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SSystem::SObjectArray<SSystem::SString>& listDirs,
					const wchar_t * pszDirPath = nullptr ) ;

	public:	// SFileInterface
		// ファイルインターフェースの複製
		virtual SSystem::SFileInterface * Duplicate( void ) const ;
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
	

	//////////////////////////////////////////////////////////////////////////
	// SSystem::SFileInterface -> Loquaty::LPureFile
	//////////////////////////////////////////////////////////////////////////

	class	LGLS4File	: public LPureFile, public LDirectory
	{
	protected:
		SSystem::SSmartPointer<SSystem::SFileInterface>	m_pFile ;

	public:
		// 構築関数
		LGLS4File( SSystem::SFileInterface * pFile ) ;

	public:	// LPureFile
		// ファイルから読み込む
		virtual size_t Read( void * buf, size_t bytes ) ;

		// ファイルへ読み込む
		virtual size_t Write( const void * buf, size_t bytes ) ;

		// 書き出しを確定する
		virtual void Flush( void ) ;

		// シークする
		virtual void Seek( std::int64_t pos ) ;
		virtual void Skip( std::int64_t bytes ) ;

		// シーク可能か？
		virtual bool IsSeekable( void ) const ;

		// 現在の位置を取得する (ストリームの場合は -1)
		virtual std::int64_t GetPosition( void ) const ;

		// ファイル全長 (ストリームの場合は -1)
		virtual std::int64_t GetLength( void ) const ;

		// 現在の位置にファイルを切り詰める
		virtual void Truncate( void ) ;

		// ファイルパスを取得する（大抵は開いたときのファイルパスで絶対パスではない）
		virtual LString GetFilePath( void ) const ;

		// ディレクトリを取得する
		//（可能なら GetFilePath() で取得したパスで同じファイルが開けるようにする）
		virtual LDirectory * GetDirectory( void ) ;

	public:	// LDirectory
		// ファイルを開く
		virtual LFilePtr OpenFile
			( const wchar_t * pwszPath, long nOpenFlags = modeRead ) ;
		static long OpenFlagsLoquatyToGLS4( long nOpenFlags ) ;

		// 可能なら同等のディレクトリを複製する
		virtual std::shared_ptr<LDirectory> Duplicate( void ) ;

		// ファイル情報取得
		virtual bool QueryFileState
			( LDirectory::State& state, const wchar_t * pwszPath ) ;
		static void DateTimeGLS4ToLoquaty
			( const SSystem::DATE_TIME& date, LDateTime& ldate ) ;

		// ファイル名（サブディレクトリ含む）列挙
		// ※ files へは以前のデータを削除せずに追加
		// ※ ファイル名にはディレクトリパスを含まない
		virtual void ListFiles
			( std::vector<LString>& files,
					const wchar_t * pwszSubDirPath = nullptr ) ;

		// ファイル削除
		virtual bool DeleteFile( const wchar_t * pwszPath ) ;

		// ファイル名変更
		virtual bool RenameFile
			( const wchar_t * pwszOldPath, const wchar_t * pwszNewPath ) ;

		// サブディレクトリ作成
		virtual bool CreateDirectory( const wchar_t * pwszPath ) ;

		// サブディレクトリ削除
		virtual bool DeleteDirectory( const wchar_t * pwszPath ) ;

	} ;

}

#endif

