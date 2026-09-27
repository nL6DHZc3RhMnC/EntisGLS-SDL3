
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_FILE_H__)
#define	__GLSCS_SAKURA2_OBJECT_FILE_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// ファイル・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	FileObject	: public Object
	{
	protected:
		SSystem::SString			m_strFilePath ;
		SSystem::SFileInterface *	m_pFile ;
		long int					m_nOpenFlags ;
		bool						m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( FileObject, Object )
		// 構築関数
		FileObject( void ) ;
		FileObject( SSystem::SFileInterface * pFile, bool flagOwner ) ;
		FileObject( const SSystem::SString & strFilePath,
						SSystem::SFileInterface * pFile,
						long int nOpenFlags, bool flagOwner ) ;
		// 消滅関数
		virtual ~FileObject( void ) ;

	public:
		// ファイルを開く
		SSystem::SError Open
			( const wchar_t * pwszFilePath, long int nOpenFlags,
							VirtualMachine * vm, Context * context ) ;
		// ファイルを閉じる
		void Close( void ) ;
		// ファイル・オブジェクト取得
		SSystem::SFileInterface * GetFile( void ) const
			{
				return	m_pFile ;
			}

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// メモリ参照ファイル・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	MemoryReferenceFileObject	: public FileObject
	{
	public:
		// メモリ参照
		class	FileTrap	: public SSystem::SMemoryReferenceFile
		{
		public:
			VirtualMachine *	m_vm ;
			INT64				m_addr ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( FileTrap, SMemoryReferenceFile )
			// 構築関数
			FileTrap( VirtualMachine * vm ) : m_vm(vm), m_addr(0) {}
			// メモリ参照設定
			void AttachVMMemory( INT64 addrMemory, size_t nLength ) ;
			// ファイルインターフェースの複製
			virtual SFileInterface * Duplicate( void ) const ;
			// ファイルから読み込み
			virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
			// ファイルへ書き込み
			virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( MemoryReferenceFileObject, FileObject )
		// 構築関数
		MemoryReferenceFileObject( VirtualMachine * vm ) ;
		// 消滅関数
		virtual ~MemoryReferenceFileObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// HTTP ファイル・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	HttpFileObject	: public FileObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( HttpFileObject, FileObject )
		// 構築関数
		HttpFileObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// SSystem::File スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::File
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_File) ;

// static SSystem::File * SSystem::File::NewOpen
//			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_NewOpen) ;

// static bool SSystem::File::IsExistingFile( const wchar_t * pszFilePath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_IsExistingFile) ;

// static SError SSystem::File::QueryFileState
//		( const wchar_t * pszFilePath, SFileOpener::State& state ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_QueryFileState) ;

// static SError SSystem::File::RemoveFile( const wchar_t * pszFilePath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_RemoveFile) ;

// static void SSystem::File::ListFiles
//	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_ListFiles) ;

// static void SSystem::File::ListDirectories
//	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_ListDirectories) ;

// static SError SSystem::File::CreateDirectory
//	( const wchar_t * pszPath, long int nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_CreateDirectory) ;

// static SError SSystem::File::RemoveDirectory( const wchar_t * pszPath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_RemoveDirectory) ;

// static SError SSystem::File::RenameFile
//	( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_RenameFile) ;

// static SError SSystem::File::GetDefaultDirectory
//	( SString& strDirPath, const wchar_t * pwszPlacementId, const wchar_t * pwszOption ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_GetDefaultDirectory) ;

// SSystem::File * SSystem::File::Duplicate( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_Duplicate) ;

// size_t SSystem::File::Read( void * ptrBuf, size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_Read) ;

// size_t SSystem::File::Write( const void * ptrBuf, size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_Write) ;

// bool SSystem::File::IsSeekable( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_IsSeekable) ;

// int64_t SSystem::File::GetLength( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_GetLength) ;

// int64_t SSystem::File::Seek
//		( int64_t posFile, SFileInterface::SeekOrigin seekFrom ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_Seek) ;

// int64_t SSystem::File::GetPosition( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_GetPosition) ;

// SSystem::SError SSystem::File::SetEndOfFile( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_SetEndOfFile) ;

// SSystem::SError SSystem::File::SetEndOfFile( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_SetEndOfFile) ;

// SSystem::SError SSystem::File::GetFileTime( SSystem::DATE_TIME& time ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_GetFileTime) ;

// SSystem::SError SSystem::File::SetFileTime( const SSystem::DATE_TIME& time ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_File_SetFileTime) ;


//////////////////////////////////////////////////////////////////////////////
// SSystem::MemoryReferenceFile スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::MemoryReferenceFile
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_MemoryReferenceFile) ;

// void SSystem::MemoryReferenceFile::AttachMemory( void * ptrMemory, size_t nLength ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_MemoryReferenceFile_AttachMemory) ;


//////////////////////////////////////////////////////////////////////////////
// SSystem::HttpFile スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::HttpFile
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_HttpFile) ;

// SError SSystem::HttpFile::SetRequest
//	( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_SetRequest) ;

// SError SSystem::HttpFile::SetSendData
//	( const uint8_t * pbytData, ssize_t nBytes = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_SetSendData) ;

// SError SSystem::HttpFile::AddHeader( const wchar_t * pwszHeader ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_AddHeader) ;

// SError SSystem::HttpFile::Connect( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_Connect) ;

// SError SSystem::HttpFile::SendRequest( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_SendRequest) ;

// SError SSystem::HttpFile::QueryStatusCode( uint32_t& codeStatus ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryStatusCode) ;

// SError SSystem::HttpFile::QueryContentLength( uint64_t& numLength ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentLength) ;

// SError SSystem::HttpFile::QueryContentType( SString& strType ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentType) ;

// SError SSystem::HttpFile::QueryContentTransferEncoding( SString& strEncoding ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentTransferEncoding) ;

// SError SSystem::HttpFile::QueryContentDate( DATE_TIME& dt ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentDate) ;

// SError SSystem::HttpFile::QueryContentLastModified( DATE_TIME& dt ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentLastModified) ;


#endif
