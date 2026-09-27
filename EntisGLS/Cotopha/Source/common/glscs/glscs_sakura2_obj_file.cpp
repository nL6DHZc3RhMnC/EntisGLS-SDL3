
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_http_file.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_file.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ファイル・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( ECSSakura2::FileObject, Object, m_pFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
FileObject::FileObject( void )
{
	m_pFile = NULL ;
	m_flagOwner = false ;
}

FileObject::FileObject
	( SFileInterface * pFile, bool flagOwner )
	: m_pFile( pFile ), m_nOpenFlags( 0 ), m_flagOwner( flagOwner )
{
}

FileObject::FileObject
	( const SString & strFilePath,
		SFileInterface * pFile,
		long int nOpenFlags, bool flagOwner )
	: m_strFilePath( strFilePath ), m_pFile( pFile ),
			m_nOpenFlags( nOpenFlags ), m_flagOwner( flagOwner )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
FileObject::~FileObject( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileObject::Open
	( const wchar_t * pwszFilePath, long int nOpenFlags,
					VirtualMachine * vm, Context * context )
{
	SFileInterface *	pFile = vm->NewOpenFile( pwszFilePath, nOpenFlags ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	Close() ;
	//
	m_strFilePath = pwszFilePath ;
	m_pFile = pFile ;
	m_nOpenFlags = nOpenFlags ;
	m_flagOwner = true ;
	//
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void FileObject::Close( void )
{
	if ( (m_pFile != NULL) && m_flagOwner )
	{
		delete	m_pFile ;
	}
	m_strFilePath.FreeArray() ;
	m_pFile = NULL ;
	m_flagOwner = false ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * FileObject::GetTypeName( void ) const
{
	return	L"SSystem::File" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError FileObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	if ( file->WriteString( m_strFilePath ) )
	{
		return	errFailed ;
	}
	if ( !m_strFilePath.IsEmpty() )
	{
		int64_t	nFilePos = 0 ;
		if ( m_pFile != NULL )
		{
			m_pFile->GetPosition() ;
		}
		file->Write( &m_nOpenFlags, sizeof(long int) ) ;
		file->Write( &nFilePos, sizeof(int64_t) ) ;
	}
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError FileObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SString	strFilePath ;
	if ( file->ReadString( strFilePath ) )
	{
		return	errFailed ;
	}
	Close() ;
	//
	if ( !strFilePath.IsEmpty() )
	{
		long int	nOpenFlags ;
		int64_t		nFilePos ;
		file->Read( &nOpenFlags, sizeof(long int) ) ;
		file->Read( &nFilePos, sizeof(int64_t) ) ;
		//
		if ( Open( strFilePath, nOpenFlags, vm, context ) )
		{
			ESLTrace( "failed to open file at LoadStatic.\n" ) ;
		}
		else
		{
			ESLAssert( m_pFile != NULL ) ;
			m_pFile->Seek( nFilePos ) ;
		}
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// メモリ参照ファイル・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// ECSSakura2::MemoryReferenceFileObject::FileTrap クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::MemoryReferenceFileObject::FileTrap, SMemoryReferenceFile )

// メモリ参照設定
//////////////////////////////////////////////////////////////////////////////
void MemoryReferenceFileObject::FileTrap::AttachVMMemory( INT64 addrMemory, size_t nLength )
{
	m_addr = addrMemory ;
	//
	SMemoryReferenceFile::AttachMemory
		( m_vm->TranslateAddress( addrMemory, nLength ), nLength ) ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * MemoryReferenceFileObject::FileTrap::Duplicate( void ) const
{
	FileTrap *	pfile = new FileTrap( m_vm ) ;
	pfile->AttachVMMemory( m_addr, m_nLength ) ;
	return	pfile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t MemoryReferenceFileObject::FileTrap::Read( void * ptrBuf, size_t nBytes )
{
	SMemoryReferenceFile::AttachMemory
		( m_vm->TranslateAddress( m_addr, m_nLength ), m_nLength ) ;
	if ( m_pbytMemory == NULL )
	{
		return	0 ;
	}
	return	SMemoryReferenceFile::Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t MemoryReferenceFileObject::FileTrap::Write( const void * ptrBuf, size_t nBytes )
{
	SMemoryReferenceFile::AttachMemory
		( m_vm->TranslateAddress( m_addr, m_nLength ), m_nLength ) ;
	if ( m_pbytMemory == NULL )
	{
		return	0 ;
	}
	return	SMemoryReferenceFile::Write( ptrBuf, nBytes ) ;
}


// MemoryReferenceFileObject クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::MemoryReferenceFileObject, FileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
MemoryReferenceFileObject::MemoryReferenceFileObject( VirtualMachine * vm )
	: FileObject( new FileTrap( vm ), true )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
MemoryReferenceFileObject::~MemoryReferenceFileObject( void )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * MemoryReferenceFileObject::GetTypeName( void ) const
{
	return	L"SSystem::MemoryReferenceFile" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError MemoryReferenceFileObject::SaveStatic
	( SSystem::SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	FileObject::SaveStatic( file, vm, context ) ;
	//
	FileTrap *	pTrap = ESLTypeCast<FileTrap>( GetFile() ) ;
	INT64		addrMemory = 0 ;
	INT64		nLength = 0 ;
	if ( pTrap != NULL )
	{
		addrMemory = pTrap->m_addr ;
		nLength = pTrap->GetLength() ;
	}
	file->Write( &addrMemory, sizeof(INT64) ) ;
	file->Write( &nLength, sizeof(INT64) ) ;
	//
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError MemoryReferenceFileObject::LoadStatic
	( SSystem::SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	FileObject::LoadStatic( file, vm, context ) ;
	//
	FileTrap *	pTrap = ESLTypeCast<FileTrap>( GetFile() ) ;
	INT64		addrMemory = 0 ;
	INT64		nLength = 0 ;
	file->Read( &addrMemory, sizeof(INT64) ) ;
	file->Read( &nLength, sizeof(INT64) ) ;
	if ( pTrap != NULL )
	{
		pTrap->AttachVMMemory( addrMemory, (size_t) nLength ) ;
	}
	//
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// HTTP ファイル・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::HttpFileObject, FileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
HttpFileObject::HttpFileObject( void )
	: FileObject( new SHttpFile, true )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * HttpFileObject::GetTypeName( void ) const
{
	return	L"SSystem::HttpFile" ;
}



//////////////////////////////////////////////////////////////////////////////
// SSystem::File スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::File
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_File, context, cls_id )
{
	return	new FileObject ;
}

// static SSystem::File * SSystem::File::NewOpen
//			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_NewOpen, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszFilePath,
				pArg[0].i, pszFilePath at File::NewOpen ) ;
	//
	SString				strFilePath = pszFilePath ;
	SFileInterface *	pFile =
			pVM->NewOpenFile( strFilePath, (long int) pArg[1].i ) ;
	if ( pFile == NULL )
	{
		pContext->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	FileObject *	pFileObj =
		new FileObject( strFilePath, pFile, (long int) pArg[1].i, true ) ;
	AssertLock() ;
	pContext->m_regset[regAcc].i =
				pVM->AllocateHeapObjectAddress( pFileObj ) ;
	AssertUnlock() ;
	return	NULL ;
}

// static bool SSystem::File::IsExistingFile( const wchar_t * pszFilePath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_IsExistingFile, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszFilePath,
				pArg[0].i, pszFilePath at File::IsExistingFile ) ;
	//
	SString	strFilePath = pszFilePath ;
	pContext->m_regset[regAcc].i = 0 ;
	if ( pVM->IsExistingFile( strFilePath ) )
	{
		pContext->m_regset[regAcc].i = -1 ;
	}
	return	NULL ;
}

// static SError SSystem::File::QueryFileState
//		( const wchar_t * pszFilePath, SFileOpener::State& state ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_QueryFileState, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszFilePath,
				pArg[0].i, pszFilePath at File::QueryFileState ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, SFileOpener::State, pState,
				pArg[1].i, state at File::QueryFileState ) ;
	//
	SString	strFilePath = pszFilePath ;
	pContext->m_regset[regAcc].i =
			pVM->QueryFileState( strFilePath, *pState ) ;
	//
	return	NULL ;
}

// static SError SSystem::File::RemoveFile( const wchar_t * pszFilePath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_RemoveFile, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszFilePath,
				pArg[0].i, pszFilePath at File::IsExistingFile ) ;
	//
	SString	strFilePath = pszFilePath ;
	//
	SOffsetFileOpener *		pOpener = NULL ;
	SEnvironmentInterface *	pEnv = pVM->GetEnvironment() ;
	if ( pEnv != NULL )
	{
		pOpener = ESLTypeCast<SOffsetFileOpener>
							( pEnv->GetWritableFileOpener() ) ;
	}
	if ( pOpener != NULL )
	{
		pContext->m_regset[regAcc].i =
			SFile::RemoveFile( pOpener->OffsetPath( strFilePath ) ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i = SFile::RemoveFile( strFilePath ) ;
	}
	return	NULL ;
}

// static void SSystem::File::ListFiles
//	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_ListFiles, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, SSystem_Array, pArray,
				pArg[0].i, listFiles at File::ListFiles ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszDirPath,
				pArg[1].i, pszDirPath at File::ListFiles ) ;
	//
	SObjectArray<SString>	listFiles ;
	SString	strDirPath = pszDirPath ;
	strDirPath = pVM->OffsetFilePath( strDirPath ) ;
	SFile::ListFiles( listFiles, strDirPath ) ;
	//
	const size_t	nFileCount = listFiles.GetLength() ;
	INT64 *	pStrArray =
		(INT64*) pArray->AllocateArray
						( nFileCount, sizeof(INT64), pVM ) ;
	if ( pStrArray != NULL )
	{
		for ( size_t i = 0; i < nFileCount; i ++ )
		{
			SString *	pFileName = listFiles.GetAt( i ) ;
			if ( pFileName == NULL )
			{
				pStrArray[i] = 0 ;
				continue ;
			}
			pStrArray[i] = pVM->AllocateHeapMemory( sizeof(SSystem_Array) ) ;
			SSystem_Array *	pString =
				(SSystem_Array*) pContext->AtomicTranslateAddress( pStrArray[i] ) ;
			if ( pString != NULL )
			{
				eslFillMemory( pString, 0, sizeof(SSystem_Array) ) ;
				//
				WORD *	pszStrFile =
					(WORD*) pString->AllocateArray
						( pFileName->GetLength() + 1, sizeof(WORD), pVM ) ;
				if ( pszStrFile != NULL )
				{
					::eslMoveMemory
						( pszStrFile,
							pFileName->GetArray(),
							(DWORD) (pFileName->GetLength() + 1) * sizeof(WORD) ) ;
					pFileName->FinishArray() ;
					pString->m_nLength = (DWORD) pFileName->GetLength() ;
				}
			}
		}
	}
	return	NULL ;
}

// static void SSystem::File::ListDirectories
//	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_ListDirectories, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, SSystem_Array, pArray,
				pArg[0].i, listDirs at File::ListDirectories ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszDirPath,
				pArg[1].i, pszDirPath at File::ListDirectories ) ;
	//
	SObjectArray<SString>	listDirs ;
	SString	strDirPath = pszDirPath ;
	strDirPath = pVM->OffsetFilePath( strDirPath ) ;
	SFile::ListDirectories( listDirs, strDirPath ) ;
	//
	const size_t	nFileCount = listDirs.GetLength() ;
	INT64 *	pStrArray =
		(INT64*) pArray->AllocateArray
						( nFileCount, sizeof(INT64), pVM ) ;
	if ( pStrArray != NULL )
	{
		for ( size_t i = 0; i < nFileCount; i ++ )
		{
			SString *	pDirName = listDirs.GetAt( i ) ;
			if ( pDirName == NULL )
			{
				pStrArray[i] = 0 ;
				continue ;
			}
			pStrArray[i] = pVM->AllocateHeapMemory( sizeof(SSystem_Array) ) ;
			SSystem_Array *	pString =
				(SSystem_Array*) pContext->AtomicTranslateAddress( pStrArray[i] ) ;
			if ( pString != NULL )
			{
				eslFillMemory( pString, 0, sizeof(SSystem_Array) ) ;
				//
				WORD *	pszStrDir =
					(WORD*) pString->AllocateArray
						( pDirName->GetLength() + 1, sizeof(WORD), pVM ) ;
				if ( pszStrDir != NULL )
				{
					::eslMoveMemory
						( pszStrDir,
							pDirName->GetArray(),
							(pDirName->GetLength() + 1) * sizeof(WORD) ) ;
					pDirName->FinishArray() ;
					pString->m_nLength = (DWORD) pDirName->GetLength() ;
				}
			}
		}
	}
	return	NULL ;
}

// static SError SSystem::File::CreateDirectory
//	( const wchar_t * pszPath, long int nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_CreateDirectory, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszPath,
				pArg[0].i, pszPath at File::CreateDirectory ) ;
	//
	SFileOpener *	pOpener = NULL ;
	SEnvironmentInterface *	pEnv = pVM->GetEnvironment() ;
	if ( pEnv != NULL )
	{
		pOpener = pEnv->GetWritableFileOpener() ;
	}
	if ( pOpener != NULL )
	{
		pContext->m_regset[regAcc].i =
			pOpener->CreateSubDirectory
				( SString(pszPath), (long int) pArg[1].i ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i =
			SFile::CreateDirectory( SString(pszPath), (long int) pArg[1].i ) ;
	}
	return	NULL ;
}

// static SError SSystem::File::RemoveDirectory( const wchar_t * pszPath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_RemoveDirectory, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszPath,
				pArg[0].i, pszPath at File::RemoveDirectory ) ;
	//
	SFileOpener *	pOpener = NULL ;
	SEnvironmentInterface *	pEnv = pVM->GetEnvironment() ;
	if ( pEnv != NULL )
	{
		pOpener = pEnv->GetWritableFileOpener() ;
	}
	if ( pOpener != NULL )
	{
		pContext->m_regset[regAcc].i =
			pOpener->RemoveSubDirectory( SString(pszPath) ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i =
				SFile::RemoveDirectory( SString(pszPath) ) ;
	}
	return	NULL ;
}

// static SError SSystem::File::RenameFile
//	( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_RenameFile, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszOldPath,
				pArg[0].i, pszPath at File::RenameFile ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszNewPath,
				pArg[1].i, pszPath at File::RenameFile ) ;
	//
	SFileOpener *	pOpener =
		SFileOpener::DefaultGetExisting( SString(pszOldPath), true ) ;
	if ( pOpener != NULL )
	{
		pContext->m_regset[regAcc].i =
			pOpener->RenameSubFile
				( SString(pszOldPath), SString(pszNewPath) ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i = errFailed ;
	}
	return	NULL ;
}

// static SError SSystem::File::GetDefaultDirectory
//	( SString& strDirPath, const wchar_t * pwszPlacementId, const wchar_t * pwszOption ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_File_GetDefaultDirectory, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, SSystem_Array, pstrDirPath,
				pArg[0].i, strDirPath at File::GetDefaultDirectory ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, const uint16_t, pszPlacementId,
				pArg[1].i, pwszPlacementId at File::GetDefaultDirectory ) ;
	const uint16_t *	pszOption =
		(const uint16_t*) pContext->AtomicTranslateAddress
								( pArg[2].i, sizeof(uint16_t) ) ;
	//
	SString	strDirPath ;
	SString	strPlacementId = pszPlacementId ;
	SString	strOption = pszOption ;
	SError	err =
		SFile::GetDefaultDirectory
				( strDirPath, strPlacementId, strOption ) ;
	//
	pContext->m_regset[regAcc].i = err ;
	if ( !err )
	{
		size_t		lenResult = strDirPath.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pstrDirPath->AllocateArray
						( lenResult + 1, sizeof(uint16_t), pVM ) ;
		//
		const uint16_t *	pszResult = strDirPath ;
		for ( size_t i = 0; i < lenResult; i ++ )
		{
			pStrArray[i] = pszResult[i] ;
		}
		pStrArray[lenResult] = 0 ;
		//
		pstrDirPath->m_nLength = (DWORD) lenResult ;
	}
	return	NULL ;
}

// SSystem::File * SSystem::File::Duplicate( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_Duplicate, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM( pContext, pVM ) ;
	//
	SFileInterface *	pFile =
		ESLTypeCast<SFileInterface>( pVM->ObjectFromAddress( pArg[0].h32 ) ) ;
	pContext->m_regset[regAcc].i = 0 ;
	if ( pFile != NULL )
	{
		pFile = pFile->Duplicate() ;
		if ( pFile != NULL )
		{
			FileObject *	pFileObj = new FileObject( pFile, true ) ;
			AssertLock() ;
			pContext->m_regset[regAcc].i =
					pVM->AllocateHeapObjectAddress( pFileObj ) ;
			AssertUnlock() ;
		}
	}
	return	NULL ;
}

// size_t SSystem::File::Read( void * ptrBuf, size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_Read, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::Read ) ;
	//
	pContext->m_regset[regAcc].i = 0 ;
	if ( pArg[2].i != 0 )
	{
		size_t	nBytes = (size_t) pArg[2].i ;
		void *	ptrBuf =
			pContext->AtomicTranslateAddress( pArg[1].i, nBytes ) ;
		if ( ptrBuf == NULL )
		{
			return	L"invalid buffer pointer at File::Read" ;
		}
		pContext->m_regset[regAcc].i = pFile->Read( ptrBuf, nBytes ) ;
	}
	return	NULL ;
}

// size_t SSystem::File::Write( const void * ptrBuf, size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_Write, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::Write ) ;
	//
	pContext->m_regset[regAcc].i = 0 ;
	if ( pArg[2].i != 0 )
	{
		size_t	nBytes = (size_t) pArg[2].i ;
		void *	ptrBuf =
			pContext->AtomicTranslateAddress( pArg[1].i, nBytes ) ;
		if ( ptrBuf == NULL )
		{
			return	L"invalid buffer pointer at File::Write" ;
		}
		pContext->m_regset[regAcc].i = pFile->Write( ptrBuf, nBytes ) ;
	}
	return	NULL ;
}

// bool SSystem::File::IsSeekable( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_IsSeekable, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::IsSeekable ) ;
	//
	pContext->m_regset[regAcc].i = pFile->IsSeekable() ? -1 : 0 ;
	//
	return	NULL ;
}

// int64_t SSystem::File::GetLength( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_GetLength, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::GetLength ) ;
	//
	pContext->m_regset[regAcc].i = pFile->GetLength() ;
	//
	return	NULL ;
}

// int64_t SSystem::File::Seek
//		( int64_t posFile, SFileInterface::SeekOrigin seekFrom ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_Seek, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::Seek ) ;
	//
	pContext->m_regset[regAcc].i =
		pFile->Seek( pArg[1].i, (SFileInterface::SeekOrigin) pArg[2].i ) ;
	//
	return	NULL ;
}

// int64_t SSystem::File::GetPosition( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_GetPosition, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::GetPosition ) ;
	//
	pContext->m_regset[regAcc].i = pFile->GetPosition() ;
	//
	return	NULL ;
}

// SSystem::SError SSystem::File::SetEndOfFile( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_SetEndOfFile, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::SetEndOfFile ) ;
	//
	pContext->m_regset[regAcc].i = pFile->SetEndOfFile() ;
	//
	return	NULL ;
}

// SSystem::SError SSystem::File::GetFileTime( SSystem::DATE_TIME& time ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_GetFileTime, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::GetFileTime ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, DATE_TIME, pDate, pArg[1].i, time at File::GetFileTime ) ;
	//
	SFile *	pStdFile = ESLTypeCast<SFile>( pFile ) ;
	if ( pStdFile != NULL )
	{
		pContext->m_regset[regAcc].i = pStdFile->GetFileTime( *pDate ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i = errFailed ;
	}
	return	NULL ;
}

// SSystem::SError SSystem::File::SetFileTime( const SSystem::DATE_TIME& time ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_File_SetFileTime, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, SFileInterface, pFile, pArg, File::SetFileTime ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( pContext, DATE_TIME, pDate, pArg[1].i, time at File::SetFileTime ) ;
	//
	SFile *	pStdFile = ESLTypeCast<SFile>( pFile ) ;
	if ( pStdFile != NULL )
	{
		pContext->m_regset[regAcc].i = pStdFile->SetFileTime( *pDate ) ;
	}
	else
	{
		pContext->m_regset[regAcc].i = errFailed ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// SSystem::MemoryReferenceFile スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::MemoryReferenceFile
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SSystem_MemoryReferenceFile, context, cls_id)
{
	return	new MemoryReferenceFileObject( context->m_pSakura2VM ) ;
}

// void SSystem::MemoryReferenceFile::AttachMemory( void * ptrMemory, size_t nLength ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_MemoryReferenceFile_AttachMemory, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, MemoryReferenceFileObject::FileTrap, pFile,
							arg, MemoryReferenceFile::AttachMemory ) ;
	//
	pFile->AttachVMMemory( arg[1].i, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// SSystem::HttpFile スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::HttpFile
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SSystem_HttpFile, context, cls_id)
{
	return	new HttpFileObject ;
}

// SError SSystem::HttpFile::SetRequest
//	( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_SetRequest, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::SetRequest ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pwszURL,
				arg[1].i, pwszURL at HttpFile::SetRequest ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pwszCmd,
				arg[2].i, pwszCmd at HttpFile::SetRequest ) ;
	//
	context->m_regset[regAcc].i =
		pFile->SetRequest( SString(pwszURL), SString(pwszCmd) ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::SetSendData
//	( const uint8_t * pbytData, ssize_t nBytes = -1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_SetSendData, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::SetSendData ) ;
	//
	uint8_t *	pbytData =
		(uint8_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	context->m_regset[regAcc].i =
		pFile->SetSendData( pbytData, (ssize_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::AddHeader( const wchar_t * pwszHeader ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_AddHeader, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::AddHeader ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pwszHeader,
				arg[1].i, pwszHeader at HttpFile::AddHeader ) ;
	//
	context->m_regset[regAcc].i =
		pFile->AddHeader( SString(pwszHeader) ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::Connect( uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_Connect, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::Connect ) ;
	//
	context->m_regset[regAcc].i = pFile->Connect( (uint32_t) arg[1].i ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::SendRequest( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_SendRequest, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::SendRequest ) ;
	//
	context->m_regset[regAcc].i = pFile->SendRequest() ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryStatusCode( uint32_t& codeStatus ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryStatusCode, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile, arg, HttpFile::QueryStatusCode ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, uint32_t, codeStatus,
				arg[1].i, codeStatus at HttpFile::QueryStatusCode ) ;
	//
	context->m_regset[regAcc].i = pFile->QueryStatusCode( *codeStatus ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryContentLength( uint64_t& numLength ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentLength, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile,
					arg, HttpFile::QueryContentLength ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, uint64_t, numLength,
				arg[1].i, numLength at HttpFile::QueryContentLength ) ;
	//
	context->m_regset[regAcc].i = pFile->QueryContentLength( *numLength ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryContentType( SString& strType ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentType, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile,
					arg, HttpFile::QueryContentType ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrType,
				arg[1].i, strType at HttpFile::QueryContentType ) ;
	//
	SString	strType ;
	SError	err = pFile->QueryContentType( strType ) ;
	context->m_regset[regAcc].i = err ;
	//
	if ( !err )
	{
		uint16_t *	pStrArray =
			(uint16_t*) pstrType->AllocateArray
					( strType.GetLength() + 1, sizeof(uint16_t), vm ) ;
		eslMoveMemory
			( pStrArray, strType.GetConstArray(),
				(strType.GetLength() + 1) * sizeof(uint16_t) ) ;
		pstrType->m_nLength = (DWORD) strType.GetLength() ;
	}
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryContentTransferEncoding( SString& strEncoding ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentTransferEncoding, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile,
					arg, HttpFile::QueryContentTransferEncoding ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrEncoding,
				arg[1].i, strEncoding at HttpFile::QueryContentTransferEncoding ) ;
	//
	SString	strEncoding ;
	SError	err = pFile->QueryContentTransferEncoding( strEncoding ) ;
	context->m_regset[regAcc].i = err ;
	//
	if ( !err )
	{
		uint16_t *	pStrArray =
			(uint16_t*) pstrEncoding->AllocateArray
					( strEncoding.GetLength() + 1, sizeof(uint16_t), vm ) ;
		eslMoveMemory
			( pStrArray, strEncoding.GetConstArray(),
				(strEncoding.GetLength() + 1) * sizeof(uint16_t) ) ;
		pstrEncoding->m_nLength = (DWORD) strEncoding.GetLength() ;
	}
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryContentDate( DATE_TIME& dt ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentDate, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile,
					arg, HttpFile::QueryContentDate ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, DATE_TIME, pDate,
				arg[1].i, dt at HttpFile::QueryContentDate ) ;
	//
	context->m_regset[regAcc].i = pFile->QueryContentDate( *pDate ) ;
	//
	return	NULL ;
}

// SError SSystem::HttpFile::QueryContentLastModified( DATE_TIME& dt ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_HttpFile_QueryContentLastModified, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SHttpFileInterface, pFile,
					arg, HttpFile::QueryContentLastModified ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, DATE_TIME, pDate,
				arg[1].i, dt at HttpFile::QueryContentLastModified ) ;
	//
	context->m_regset[regAcc].i = pFile->QueryContentLastModified( *pDate ) ;
	//
	return	NULL ;
}

#endif
