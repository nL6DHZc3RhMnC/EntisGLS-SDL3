
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_queue_buffer.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_number.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_array.h>
#include <rosetta/rosetta_file.h>
#include <rosetta/rosetta_date.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// File
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFile, RSObject )

// パス設定
//////////////////////////////////////////////////////////////////////////////
void RSFile::SetFilePath( const wchar_t * pwszPath )
{
	m_strPath = pwszPath ;
	UpdateFileState() ;
}

void RSFile::UpdateFileState( void )
{
	m_strDirectPath = m_strPath ;
	m_existing = false ;
	m_state.bitFields = 0 ;
	//
	if ( !m_strPath.IsEmpty() )
	{
		SFileOpener *	pOpener = SFileOpener::DefaultGetExisting( m_strPath ) ;
		m_existing = (pOpener != NULL) ;
		if ( (pOpener == NULL)
			|| pOpener->QueryState( m_strPath, m_state ) )
		{
			m_state.bitFields = 0 ;
		}
		if ( pOpener != NULL )
		{
			SString	strDirectPath ;
			if ( !pOpener->DirectPathOf( strDirectPath, m_strPath ) )
			{
				m_strDirectPath = strDirectPath ;
			}
		}
	}
}

// 権限判定
//////////////////////////////////////////////////////////////////////////////
bool RSFile::CanExecute( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& (m_state.bitAttributes
				& (SFileOpener::permissionXUSR
					| SFileOpener::permissionXGRP
					| SFileOpener::permissionXOTH)) ;
}

bool RSFile::CanRead( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& (m_state.bitAttributes
				& (SFileOpener::permissionRUSR
					| SFileOpener::permissionRGRP
					| SFileOpener::permissionROTH)) ;
}

bool RSFile::CanWrite( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& (m_state.bitAttributes
				& (SFileOpener::permissionWUSR
					| SFileOpener::permissionWGRP
					| SFileOpener::permissionWOTH)) ;
}

// ファイル／ディレクトリ削除
//////////////////////////////////////////////////////////////////////////////
bool RSFile::Delete( void )
{
	if ( m_existing && !m_strPath.IsEmpty() )
	{
		SFileOpener *	pOpener = SFileOpener::DefaultGetExisting( m_strPath ) ;
		if ( pOpener != NULL )
		{
			bool	fSuccessful = false ;
			if ( IsDirectory() )
			{
				fSuccessful =
					(pOpener->RemoveSubDirectory( m_strPath ) == errSuccess) ;
			}
			else
			{
				fSuccessful =
					(pOpener->RemoveSubFile( m_strPath ) == errSuccess) ;
			}
			UpdateFileState() ;
			return	fSuccessful ;
		}
	}
	return	false ;
}

// ファイル存在判定
//////////////////////////////////////////////////////////////////////////////
bool RSFile::Exists( void ) const
{
	return	m_existing ;
}

// 絶対パス取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString RSFile::GetAbsolutePath( void ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( (m_strDirectPath.CompareLeft( L"\\\\" ) != 0)
					&& (m_strDirectPath.Find( L':' ) < 0) )
	{
		char	szDirPath[MAX_PATH + 1] ;
		::GetCurrentDirectory( MAX_PATH, szDirPath ) ;
		//
		SString	strCurDir = szDirPath ;
		return	strCurDir.OffsetFilePath( m_strDirectPath ) ;
	}
#endif
	return	m_strDirectPath ;
}

// ファイル名取得
//////////////////////////////////////////////////////////////////////////////
SString RSFile::GetName( void ) const
{
	return	SString( m_strPath.GetFileNamePart() ) ;
}

// 親パス（ディレクトリ）取得
//////////////////////////////////////////////////////////////////////////////
SString RSFile::GetParent( void ) const
{
	SString	strDir = m_strPath.GetFileDirectoryPart() ;
	wchar_t	wchLast = strDir.GetLastAt(0) ;
	if ( (wchLast == L'\\') || (wchLast == L'/') )
	{
		return	strDir.Left( strDir.GetLength() - 1 ) ;
	}
	return	strDir ;
}

// ディレクトリ判定
//////////////////////////////////////////////////////////////////////////////
bool RSFile::IsDirectory( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& (m_state.bitAttributes & SFileOpener::attrDirectory) ;
}

// ファイル判定
//////////////////////////////////////////////////////////////////////////////
bool RSFile::IsFile( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& !(m_state.bitAttributes & SFileOpener::attrDirectory) ;
}

// 隠し属性判定
//////////////////////////////////////////////////////////////////////////////
bool RSFile::IsHidden( void ) const
{
	return	(m_state.bitFields & SFileOpener::fieldAttributes)
			&& (m_state.bitAttributes & SFileOpener::attrHidden) ;
}

// 最終更新時間取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSFile::GetLastModified( void ) const
{
	if ( !(m_state.bitFields & SFileOpener::fieldModifiedTime) )
	{
		return	0 ;
	}
	uint64_t	dayModified = m_state.dtModified.GetAccumulatedDayCount() ;
	uint64_t	dayUTC = DATE_TIME::GetAccumulatedDayCount( 1970, 1, 1 ) ;
	int64_t		dayOffset = (int64_t) dayModified - (int64_t) dayUTC ;
	int64_t		msecModified =
					(dayOffset * (24 * 60 * 60 * 1000))
						+ m_state.dtModified.nHour * (60 * 60 * 1000)
						+ m_state.dtModified.nMinute * (60 * 1000)
						+ m_state.dtModified.nSecond * 1000
						+ m_state.dtModified.nMilliSec
						+ DifferenceInLocalTime() * 1000 ;
	return	msecModified ;
}

// ファイルサイズ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t RSFile::GetLength( void ) const
{
	if ( m_state.bitFields & SFileOpener::fieldFileSize )
	{
		return	m_state.nFileSize ;
	}
	return	0 ;
}

// ディレクトリ作成
//////////////////////////////////////////////////////////////////////////////
bool RSFile::MakeDirectory( void )
{
	if ( !m_strPath.IsEmpty() )
	{
		SError	err = SFile::RemoveDirectory( m_strPath ) ;
		UpdateFileState() ;
		return	(err == errSuccess) ;
	}
	return	false ;
}

bool RSFile::MakeDirectories( void )
{
	if ( !m_strPath.IsEmpty() )
	{
		SError	err = SFile::CreateFullDirectory( m_strPath ) ;
		UpdateFileState() ;
		return	(err == errSuccess) ;
	}
	return	false ;
}

// ファイル名変更／移動
//////////////////////////////////////////////////////////////////////////////
bool RSFile::RenameTo( const wchar_t * pwszPath )
{
	if ( !m_strPath.IsEmpty() )
	{
		SError	err = SFile::RenameFile( m_strPath, pwszPath ) ;
		UpdateFileState() ;
		return	(err == errSuccess) ;
	}
	return	false ;
}

// ファイル列挙
//////////////////////////////////////////////////////////////////////////////
void RSFile::ListFiles
	( SSystem::SObjectArray<SSystem::SString>& listFiles ) const
{
	SFileOpener *	pOpener = SFileOpener::DefaultGetExisting( m_strPath ) ;
	if ( pOpener != NULL )
	{
		pOpener->ListSubFiles( listFiles, m_strPath ) ;
	}
	else
	{
		SFile::ListFiles( listFiles, m_strPath ) ;
	}
}

void RSFile::ListDirectories
	( SSystem::SObjectArray<SSystem::SString>& listDirs ) const
{
	SFileOpener *	pOpener = SFileOpener::DefaultGetExisting( m_strPath ) ;
	if ( pOpener != NULL )
	{
		pOpener->ListSubDirectories( listDirs, m_strPath ) ;
	}
	else
	{
		SFile::ListDirectories( listDirs, m_strPath ) ;
	}
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSFile::AsString( SSystem::SString& strValue ) const
{
	strValue = m_strPath ;
	return	true ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFile::CloneObject( RSContext& context ) const
{
	return	new RSFile( GetRSClass(), m_strPath ) ;
}


//////////////////////////////////////////////////////////////////////////////
// File クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFileClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSFileClass::RSFileClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSFileClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSFile( this, NULL ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String path",
				NULL, &RSFileClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"canExecute", L"boolean", L"",
				NULL, &RSFileClass::method_canExecute, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"canRead", L"boolean", L"",
				NULL, &RSFileClass::method_canRead, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"canWrite", L"boolean", L"",
				NULL, &RSFileClass::method_canWrite, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"delete", L"boolean", L"",
				NULL, &RSFileClass::method_delete, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"exists", L"boolean", L"",
				NULL, &RSFileClass::method_exists, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAbsolutePath", L"String", L"",
				NULL, &RSFileClass::method_getAbsolutePath, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getName", L"String", L"",
				NULL, &RSFileClass::method_getName, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParent", L"String", L"",
				NULL, &RSFileClass::method_getParent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isDirectory", L"boolean", L"",
				NULL, &RSFileClass::method_isDirectory, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isFile", L"boolean", L"",
				NULL, &RSFileClass::method_isFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isHidden", L"boolean", L"",
				NULL, &RSFileClass::method_isHidden, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"lastModified", L"long", L"",
				NULL, &RSFileClass::method_lastModified, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"long", L"",
				NULL, &RSFileClass::method_length, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mkdir", L"boolean", L"",
				NULL, &RSFileClass::method_mkdir, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mkdirs", L"boolean", L"",
				NULL, &RSFileClass::method_mkdirs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"renameTo", L"boolean", L"File dst",
				NULL, &RSFileClass::method_renameTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"list", L"String[]", L"",
				NULL, &RSFileClass::method_list, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"list", L"String[]", L"String wildcard",
				NULL, &RSFileClass::method_list2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"listFiles", L"File[]", L"",
				NULL, &RSFileClass::method_listFiles, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"listFiles", L"File[]", L"String wildcard",
				NULL, &RSFileClass::method_listFiles2, NULL ) ;
}

// void <init>( String path )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.<init> 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pFile->SetFilePath( arg.StringAt(0) ) ;
	return	NULL ;
}

// boolean canExecute()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_canExecute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.canExecute 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->CanExecute() ) ;
}

// boolean canRead()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_canRead
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.canRead 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->CanRead() ) ;
}

// boolean canWrite()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_canWrite
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.canWrite 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->CanWrite() ) ;
}

// boolean delete()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_delete
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.delete 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->Delete() ) ;
}

// boolean exists()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_exists
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.exists 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->Exists() ) ;
}

// String getAbsolutePath()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_getAbsolutePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.getName 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_String( pFile->GetAbsolutePath() ) ;
}

// String getName()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_getName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.getName 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_String( pFile->GetName() ) ;
}

// String getParent()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_getParent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.getParent 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_String( pFile->GetParent() ) ;
}

// boolean isDirectory()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_isDirectory
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.isDirectory 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->IsDirectory() ) ;
}

// boolean isFile()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_isFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.isFile 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->IsFile() ) ;
}

// boolean isHidden()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_isHidden
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.isHidden 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->IsHidden() ) ;
}

// long lastModified()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_lastModified
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.isHidden 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pFile->GetLastModified() ) ;
}

// long length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.length 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pFile->GetLength() ) ;
}

// boolean mkdir()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_mkdir
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.mkdir 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->MakeDirectory() ) ;
}

// boolean mkdirs()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_mkdirs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.mkdirs 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->MakeDirectories() ) ;
}

// boolean renameTo( File dest )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_renameTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.renameTo 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSFile *	pFileDst = ESLTypeCast<RSFile>( arg.ObjectAt(0) ) ;
	if ( pFileDst == NULL )
	{
		context.ThrowExceptionError
			( L"renameTo の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pFile->RenameTo( pFileDst->m_strPath ) ) ;
}

// String[] list()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_list
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.list 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	SObjectArray<SString>	listDir ;
	SObjectArray<SString>	listFiles ;
	pFile->ListDirectories( listDir ) ;
	pFile->ListFiles( listFiles ) ;
	//
	RSObject *	pArray =
		context.new_Array( 0x7FFFFFFF, context.GetStringClass() ) ;
	size_t	i = 0, j ;
	for ( j = 0; j < listDir.GetLength(); j ++ )
	{
		pArray->SetElementStringAt( context, (int) (i ++), listDir.At(j) ) ;
	}
	for ( j = 0; j < listFiles.GetLength(); j ++ )
	{
		pArray->SetElementStringAt( context, (int) (i ++), listFiles.At(j) ) ;
	}
	return	pArray ;
}

// String[] list( String wildcard )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_list2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.list 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strWildCard = arg.StringAt( 0 ) ;
	//
	SObjectArray<SString>	listDir ;
	SObjectArray<SString>	listFiles ;
	pFile->ListDirectories( listDir ) ;
	pFile->ListFiles( listFiles ) ;
	//
	RSObject *	pArray =
		context.new_Array( 0x7FFFFFFF, context.GetStringClass() ) ;
	size_t	i = 0, j ;
	for ( j = 0; j < listDir.GetLength(); j ++ )
	{
		if ( IsMatchWildCardTo( strWildCard, listDir.At(j) ) )
		{
			pArray->SetElementStringAt( context, (int) (i ++), listDir.At(j) ) ;
		}
	}
	for ( j = 0; j < listFiles.GetLength(); j ++ )
	{
		if ( IsMatchWildCardTo( strWildCard, listFiles.At(j) ) )
		{
			pArray->SetElementStringAt( context, (int) (i ++), listFiles.At(j) ) ;
		}
	}
	return	pArray ;
}

// File[] listFiles()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_listFiles
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.listFiles 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	SObjectArray<SString>	listFiles ;
	pFile->ListFiles( listFiles ) ;
	//
	RSClass *	pFileClass = pFile->GetRSClass() ;
	RSObject *	pArray =
			context.new_Array( 0x7FFFFFFF, pFileClass ) ;
	size_t	i = 0, j ;
	for ( j = 0; j < listFiles.GetLength(); j ++ )
	{
		RSFile *	pf =
			new RSFile
				( pFileClass,
					pFile->m_strPath.OffsetFilePath( listFiles.At(j) ) ) ;
		RSObject::ReleaseRef
			( pArray->SetElementAt( context, (int) (i ++), pf ) ) ;
	}
	return	pArray ;
}

// File[] listFiles( String wildcard )
//////////////////////////////////////////////////////////////////////////
RSObject * RSFileClass::method_listFiles2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFile *	pFile = ESLTypeCast<RSFile>( pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"File.listFiles 関数の this が File ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strWildCard = arg.StringAt( 0 ) ;
	//
	SObjectArray<SString>	listFiles ;
	pFile->ListFiles( listFiles ) ;
	//
	RSClass *	pFileClass = pFile->GetRSClass() ;
	RSObject *	pArray =
			context.new_Array( 0x7FFFFFFF, pFileClass ) ;
	size_t	i = 0, j ;
	for ( j = 0; j < listFiles.GetLength(); j ++ )
	{
		if ( IsMatchWildCardTo( strWildCard, listFiles.At(j) ) )
		{
			RSFile *	pf =
				new RSFile
					( pFileClass,
						pFile->m_strPath.OffsetFilePath( listFiles.At(j) ) ) ;
			RSObject::ReleaseRef
				( pArray->SetElementAt( context, (int) (i ++), pf ) ) ;
		}
	}
	return	pArray ;
}

// ワイルドカード判定
//////////////////////////////////////////////////////////////////////////
bool RSFileClass::IsMatchWildCardTo
	( const wchar_t * pwszWildCard, const wchar_t * pwszFileName )
{
	return	SFileOpener::IsMatchWildCardTo( pwszWildCard, pwszFileName ) ;
}



//////////////////////////////////////////////////////////////////////////
// InputStream クラスオブジェクト
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSInputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////
RSInputStreamClass::RSInputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////
void RSInputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"String path, String encoding = null",
				NULL, &RSInputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"File file, String encoding = null",
				NULL, &RSInputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int",
				L"Uint8Pointer b, int off, int len",
				NULL, &RSInputStreamClass::method_read4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int",
				L"ArrayBuffer b, int off, int len",
				NULL, &RSInputStreamClass::method_read4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"",
				NULL, &RSInputStreamClass::method_read1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"byte[] b",
				NULL, &RSInputStreamClass::method_read2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"byte[] b, int off, int len",
				NULL, &RSInputStreamClass::method_read3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"skip", L"long", L"long n",
				NULL, &RSInputStreamClass::method_skip, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"available", L"int", L"",
				NULL, &RSInputStreamClass::method_available, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close", NULL, L"",
				NULL, &RSInputStreamClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mark", NULL, L"int readlimit",
				NULL, &RSInputStreamClass::method_mark, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"reset", NULL, L"",
				NULL, &RSInputStreamClass::method_reset, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"markSupported", L"boolean", L"",
				NULL, &RSInputStreamClass::method_markSupported, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readBoolean", L"boolean", L"",
				NULL, &RSInputStreamClass::method_readBoolean, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readByte", L"byte", L"",
				NULL, &RSInputStreamClass::method_readByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readChar", L"char", L"",
				NULL, &RSInputStreamClass::method_readChar, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readShort", L"short", L"",
				NULL, &RSInputStreamClass::method_readShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readInt", L"int", L"",
				NULL, &RSInputStreamClass::method_readInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLong", L"long", L"",
				NULL, &RSInputStreamClass::method_readLong, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUnsignedByte", L"int", L"",
				NULL, &RSInputStreamClass::method_readUnsignedByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUnsignedShort", L"int", L"",
				NULL, &RSInputStreamClass::method_readUnsignedShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readFloat", L"float", L"",
				NULL, &RSInputStreamClass::method_readFloat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readDouble", L"double", L"",
				NULL, &RSInputStreamClass::method_readDouble, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLine", L"String", L"",
				NULL, &RSInputStreamClass::method_readLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUTF", L"String", L"",
				NULL, &RSInputStreamClass::method_readUTF, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////
SFileInterface * RSInputStreamClass::GetThisFile( RSContext& context, RSObject* pThis )
{
	SFileInterface *	pFile = GetFileOf( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が InputStream ではありません" ) ;
	}
	return	pFile ;
}

SSystem::SFileInterface *
	RSInputStreamClass::GetFileOf( RSContext& context, RSObject* pObj )
{
	SFileInterface *	pFile = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<SFileInterface>( pNativeObj->GetObject() ) ;
	}
	return	pFile ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////
bool RSInputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SFileInterface>(pObj) != nullptr) ;
}

// void <init>( String path, String encoding = null )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"InputStream.<init> の this が InputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt( 0 ) ;
	SFileInterface *
			pFile = SFileOpener::DefaultNewOpenFile
							( strPath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( strPath + L" ファイルを開けませんでした",
								L"FileNotFoundException" ) ;
		return	NULL ;
	}
	Charset::EncodingType	encoding = Charset::encodingUTF8 ;
	SString	strEncodingType = arg.StringAt( 1 ) ;
	if ( !strEncodingType.IsEmpty() )
	{
		encoding = Charset::GetEncodingType( strEncodingType ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	SBufferedFile *	pBufFile = new SBufferedFile( pFile, pFile, true ) ;
	pBufFile->SetCharsetEncoding( encoding ) ;
	pNativeObj->SetObject( *pBufFile ) ;
	return	NULL ;
}

// int read()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_read1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint8_t	buf[1] ;
	if ( pFile->Read( &buf[0], 1 ) < 1 )
	{
		return	context.new_Integer( -1 ) ;
	}
	return	context.new_Integer( buf[0] ) ;
}

// int read( byte[] b )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_read2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pArray = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"read 引数 byte[] が null です",
									L"NullPointerException" ) ;
		return	NULL ;
	}
	if ( pArray->m_limitLength == 0x7FFFFFFF )
	{
		context.ThrowExceptionError
			( L"read 引数 byte[] が可変長配列です" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	buf ;
	uint8_t *	pBuf = buf.GetArray( pArray->m_limitLength ) ;
	size_t	nRead = pFile->Read( pBuf, buf.GetLength() ) ;
	for ( size_t i = 0; i < nRead; i ++ )
	{
		pArray->SetElementIntegerAt( context, (int) i, pBuf[i] ) ;
	}
	buf.FinishArray() ;
	return	context.new_Integer( nRead ) ;
}

// int read( byte[] b, int off, int len )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_read3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pArray = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"read 引数 byte[] が null です",
									L"NullPointerException" ) ;
		return	NULL ;
	}
	const int	nOffset = arg.IntAt( 1 ) ;
	const int	nLength = arg.IntAt( 2 ) ;
	if ( (nOffset < 0) || (nLength < 0)
		|| (pArray->m_limitLength < (size_t) (nOffset + nLength)) )
	{
		context.ThrowExceptionError
			( L"read 引数の指標が範囲外です",
									L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	buf ;
	uint8_t *	pBuf = buf.GetArray( (size_t) nLength ) ;
	size_t	nRead = pFile->Read( pBuf, buf.GetLength() ) ;
	for ( size_t i = 0; i < nRead; i ++ )
	{
		pArray->SetElementIntegerAt
			( context, nOffset + (int) i, pBuf[i] ) ;
	}
	buf.FinishArray() ;
	return	context.new_Integer( nRead ) ;
}

// int read( ArrayBuffer b, int off, int len )
// int read( Uint8Pointer b, int off, int len )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_read4
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int				nOffset = arg.IntAt( 1 ) ;
	int				nLength = arg.IntAt( 2 ) ;
	RSArrayBuffer *	pBuf = ESLTypeCast<RSArrayBuffer>( arg.ObjectAt( 0 ) ) ;
	if ( pBuf == NULL )
	{
		RSTypedArrayPointer *
			pPtr = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt( 0 ) ) ;
		if ( pPtr == NULL )
		{
			context.ThrowExceptionError
				( L"read 引数 ArrayBuffer が null です",
										L"NullPointerException" ) ;
			return	NULL ;
		}
		if ( (size_t) (nOffset + nLength) > pPtr->m_nLimit )
		{
			context.ThrowExceptionError
				( L"read 引数の指標が範囲外です",
										L"IndexOutOfBoundsException" ) ;
			return	NULL ;
		}
		pBuf = pPtr->m_pRefBuffer ;
		nOffset += (int) pPtr->m_iOffset ;
	}
	if ( (nOffset < 0) || (nLength < 0)
		|| (pBuf->m_lenBuf < (size_t) (nOffset + nLength)) )
	{
		context.ThrowExceptionError
			( L"read 引数の指標が範囲外です",
									L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	size_t	nRead =
				pFile->Read
					( pBuf->m_ptrBuf + nOffset, (size_t) nLength ) ;
	return	context.new_Integer( nRead ) ;
}

// long skip( long n )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_skip
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	if ( !pFile->IsSeekable() )
	{
		context.ThrowExceptionError
			( L"シークできません", L"IOException" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	skipPos = arg.LongAt( 0 ) ;
	if ( skipPos <= 0 )
	{
		return	context.new_Integer( 0 ) ;
	}
	int64_t	pos = pFile->GetPosition() ;
	return	context.new_Integer
		( pFile->Seek( skipPos, SFileInterface::FromCurrent ) - pos ) ;
}

// int available()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_available
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Integer( 0 ) ;
}

// void close()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNative = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNative == NULL )
	{
		context.ThrowExceptionError( L"this が InputStream ではありません" ) ;
		return	NULL ;
	}
	pNative->AttachObject( NULL ) ;
	return	NULL ;
}

// void mark( int readlimit )
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_mark
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	int64_t	pos = pFile->GetPosition() ;
	//
	RSObject *	pObj = pThis->GetMemberAs( context, L"@mark" ) ;
	if ( pObj != NULL )
	{
		RSObject *	pPos = context.new_Integer( pos ) ;
		context.ReleaseObjectRef( pObj->OperatorMove( context, pPos ) ) ;
		context.ReleaseObjectRef( pPos ) ;
		context.ReleaseObjectRef( pObj ) ;
	}
	else
	{
		context.ReleaseObjectRef
			( pThis->CreateMemberAs
				( context, L"@mark", context.new_Integer( pos ) ) ) ;
	}
	return	NULL ;
}

// void reset()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_reset
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = pThis->GetMemberAs( context, L"@mark" ) ;
	if ( pObj != NULL )
	{
		int64_t	pos ;
		if ( pObj->AsInteger( pos ) )
		{
			pFile->Seek( pos ) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	return	NULL ;
}

// boolean markSupported()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_markSupported
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pFile->IsSeekable() ) ;
}

// boolean readBoolean()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readBoolean
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint8_t	b ;
	if ( pFile->Read( &b, 1 ) < 1 )
	{
		context.ThrowExceptionError
			( L"readBoolean が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( b != 0 ) ;
}

// byte readByte()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readByte
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	int8_t	b ;
	if ( pFile->Read( &b, 1 ) < 1 )
	{
		context.ThrowExceptionError
			( L"readByte が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( b, RSInteger::typeUint16 ) ;
}

// char readChar()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readChar
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint16_t	c ;
	if ( pFile->Read( &c, sizeof(uint16_t) ) < sizeof(uint16_t) )
	{
		context.ThrowExceptionError
			( L"readChar が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( c, RSInteger::typeUint16 ) ;
}

// short readShort()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readShort
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	int16_t	n ;
	if ( pFile->Read( &n, sizeof(int16_t) ) < sizeof(int16_t) )
	{
		context.ThrowExceptionError
			( L"readShort が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( n, RSInteger::typeInt16 ) ;
}

// int readInt()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	int32_t	n ;
	if ( pFile->Read( &n, sizeof(int32_t) ) < sizeof(int32_t) )
	{
		context.ThrowExceptionError
			( L"readInt が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( n, RSInteger::typeInt32 ) ;
}

// long readLong()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readLong
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	int64_t	n ;
	if ( pFile->Read( &n, sizeof(int64_t) ) < sizeof(int64_t) )
	{
		context.ThrowExceptionError
			( L"readLong が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( n, RSInteger::typeInt64 ) ;
}

// int readUnsignedByte()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readUnsignedByte
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint8_t	n ;
	if ( pFile->Read( &n, sizeof(uint8_t) ) < sizeof(uint8_t) )
	{
		context.ThrowExceptionError
			( L"readUnsignedByte が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( n, RSInteger::typeInt32 ) ;
}

// int readUnsignedShort()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readUnsignedShort
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint16_t	n ;
	if ( pFile->Read( &n, sizeof(uint16_t) ) < sizeof(uint16_t) )
	{
		context.ThrowExceptionError
			( L"readUnsignedShort が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Integer( n, RSInteger::typeInt32 ) ;
}

// float readFloat()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readFloat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	float32_t	n ;
	if ( pFile->Read( &n, sizeof(float32_t) ) < sizeof(float32_t) )
	{
		context.ThrowExceptionError
			( L"readFloat が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Number( n ) ;
}

// double readDouble()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readDouble
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	float64_t	n ;
	if ( pFile->Read( &n, sizeof(float64_t) ) < sizeof(float64_t) )
	{
		context.ThrowExceptionError
			( L"readDouble が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	return	context.new_Number( n ) ;
}

// String readLine()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SBufferedFile *	pFile =
			ESLTypeCast<SBufferedFile>( GetThisFile( context, pThis ) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"readLine の this が InputStream ではありません" ) ;
		return	NULL ;
	}
	SString	strLine ;
	if ( pFile->ReadStringLine( strLine ) == 0 )
	{
		return	NULL ;
	}
	if ( strLine.GetLastAt(0) == L'\n' )
	{
		if ( strLine.GetLastAt(1) == L'\r' )
		{
			return	context.new_String
						( strLine.Left( strLine.GetLength() - 2 ) ) ;
		}
		else
		{
			return	context.new_String
						( strLine.Left( strLine.GetLength() - 1 ) ) ;
		}
	}
	else if ( strLine.GetLastAt(0) == L'\r' )
	{
		return	context.new_String
					( strLine.Left( strLine.GetLength() - 1 ) ) ;
	}
	return	context.new_String( strLine ) ;
}

// String readUTF()
//////////////////////////////////////////////////////////////////////////
RSObject * RSInputStreamClass::method_readUTF
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"readUTF の this が InputStream ではありません" ) ;
		return	NULL ;
	}
	uint32_t	nBytes ;
	if ( pFile->Read( &nBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		context.ThrowExceptionError
			( L"readUTF が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	bufUTF ;
	if ( pFile->Read( bufUTF.GetArray( (size_t) nBytes ), nBytes ) < nBytes )
	{
		bufUTF.FinishArray() ;
		context.ThrowExceptionError
			( L"readUTF が失敗しました", L"IOException" ) ;
		return	NULL ;
	}
	bufUTF.FinishArray() ;
	//
	SString	strUTF ;
	Charset::Decode
		( strUTF, Charset::encodingUTF8, bufUTF.GetArray(), nBytes ) ;
	bufUTF.FinishArray() ;
	return	context.new_String( strUTF ) ;
}


//////////////////////////////////////////////////////////////////////////////
// OutputStream クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSOutputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSOutputStreamClass::RSOutputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSOutputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String path",
				NULL, &RSOutputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String path, boolean append",
				NULL, &RSOutputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String path, boolean append, String encoding",
				NULL, &RSOutputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write",
				NULL, L"Uint8Pointer b, int off, int len",
				NULL, &RSOutputStreamClass::method_write4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write",
				NULL, L"ArrayBuffer b, int off, int len",
				NULL, &RSOutputStreamClass::method_write4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"int b",
				NULL, &RSOutputStreamClass::method_write1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"byte[] b",
				NULL, &RSOutputStreamClass::method_write2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"byte[] b, int off, int len",
				NULL, &RSOutputStreamClass::method_write3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flush", NULL, L"",
				NULL, &RSOutputStreamClass::method_flush, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close", NULL, L"",
				NULL, &RSOutputStreamClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeBoolean", NULL, L"boolean v",
				NULL, &RSOutputStreamClass::method_writeBoolean, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeByte", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeShort", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeChar", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeChar, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeInt", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeLong", NULL, L"long v",
				NULL, &RSOutputStreamClass::method_writeLong, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeFloat", NULL, L"float v",
				NULL, &RSOutputStreamClass::method_writeFloat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeDouble", NULL, L"double v",
				NULL, &RSOutputStreamClass::method_writeDouble, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeChars", NULL, L"String s",
				NULL, &RSOutputStreamClass::method_writeChars, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeUTF", NULL, L"String s",
				NULL, &RSOutputStreamClass::method_writeUTF, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"printf", L"OutputStream", L"String fmt, ...",
				NULL, &RSOutputStreamClass::method_printf, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSOutputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SFileInterface>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	RSOutputStreamClass::GetThisFile( RSContext& context, RSObject* pThis )
{
	SFileInterface *	pFile = GetFileOf( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が OutputStream ではありません" ) ;
	}
	return	pFile ;
}

SSystem::SFileInterface *
	RSOutputStreamClass::GetFileOf( RSContext& context, RSObject* pObj )
{
	SFileInterface *	pFile = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<SFileInterface>( pNativeObj->GetObject() ) ;
	}
	return	pFile ;
}

// void <init>( String path, boolean append = false, String endoing = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"OutputStream.<init> の this が OutputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt( 0 ) ;
	bool	fAppend = arg.BooleanAt( 1, false ) ;
	SFileInterface *
			pFile = SFileOpener::DefaultNewOpenFile
							( strPath, SFileOpener::shareWrite ) ;
	if ( pFile == NULL )
	{
		pFile = SFileOpener::DefaultNewOpenFile
					( strPath, SFileOpener::modeCreate ) ;
		if ( pFile == NULL )
		{
			context.ThrowExceptionError
				( strPath + L" ファイルを開けませんでした",
									L"FileNotFoundException" ) ;
			return	NULL ;
		}
	}
	else
	{
		if ( fAppend )
		{
			pFile->Seek( 0, SFileInterface::FromEnd ) ;
		}
		else
		{
			pFile->SetEndOfFile() ;
		}
	}
	Charset::EncodingType	encoding = Charset::encodingUTF8 ;
	SString	strEncodingType = arg.StringAt( 2 ) ;
	if ( !strEncodingType.IsEmpty() )
	{
		encoding = Charset::GetEncodingType( strEncodingType ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	SBufferedFile *	pBufFile = new SBufferedFile( pFile, pFile, true ) ;
	pBufFile->SetCharsetEncoding( encoding ) ;
	pNativeObj->SetObject( *pBufFile ) ;
	return	NULL ;
}

// void write( int b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_write1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t	buf[1] ;
	buf[0] = (uint8_t) arg.IntAt( 0 ) ;
	if ( pFile->Write( &buf[0], 1 ) < 1 )
	{
		context.ThrowExceptionError
			( L"write に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void write( byte[] b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_write2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pArray = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"write 引数 byte[] が null です",
									L"NullPointerException" ) ;
		return	NULL ;
	}
	if ( pArray->m_limitLength == 0x7FFFFFFF )
	{
		context.ThrowExceptionError
			( L"write 引数 byte[] が可変長配列です" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	buf ;
	uint8_t *	pBuf = buf.GetArray( pArray->m_limitLength ) ;
	for ( size_t i = 0; i < pArray->m_limitLength; i ++ )
	{
		pBuf[i] = (uint8_t) pArray->GetElementIntegerAt( context, (int) i ) ;
	}
	if ( pFile->Write( pBuf, buf.GetLength() ) < buf.GetLength() )
	{
		context.ThrowExceptionError
			( L"write に失敗しました", L"IOException" ) ;
	}
	buf.FinishArray() ;
	return	NULL ;
}

// void write( byte[] b, int off, int len )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_write3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pArray = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"write 引数 byte[] が null です",
									L"NullPointerException" ) ;
		return	NULL ;
	}
	const int	nOffset = arg.IntAt( 1 ) ;
	const int	nLength = arg.IntAt( 2 ) ;
	if ( (nOffset < 0) || (nLength < 0)
		|| (pArray->m_limitLength < (size_t) (nOffset + nLength)) )
	{
		context.ThrowExceptionError
			( L"write 引数の指標が範囲外です",
									L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	buf ;
	uint8_t *	pBuf = buf.GetArray( (size_t) nLength ) ;
	for ( size_t i = 0; i < (size_t) nLength; i ++ )
	{
		pBuf[i] = (uint8_t) pArray->GetElementIntegerAt
								( context, nOffset + (int) i ) ;
	}
	if ( pFile->Write( pBuf, buf.GetLength() ) < buf.GetLength() )
	{
		context.ThrowExceptionError
			( L"write に失敗しました", L"IOException" ) ;
	}
	buf.FinishArray() ;
	return	NULL ;
}

// void write( ArrayBuffer b, int off, int len )
// void write( Uint8Pointer b, int off, int len )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_write4
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int					nOffset	= arg.IntAt( 1 ) ;
	int					nLength = arg.IntAt( 2 ) ;
	RSArrayBuffer *		pBuf = ESLTypeCast<RSArrayBuffer>( arg.ObjectAt( 0 ) ) ;
	if ( pBuf == NULL )
	{
		RSTypedArrayPointer *
			pPtr = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt( 0 ) ) ;
		if ( pPtr == NULL )
		{
			context.ThrowExceptionError
				( L"write 引数 ArrayBuffer が null です",
										L"NullPointerException" ) ;
			return	NULL ;
		}
		if ( (size_t) (nOffset + nLength) > pPtr->m_nLimit )
		{
			context.ThrowExceptionError
				( L"write 引数の指標が範囲外です",
										L"IndexOutOfBoundsException" ) ;
			return	NULL ;
		}
		pBuf = pPtr->m_pRefBuffer ;
		nOffset += (int) pPtr->m_iOffset ;
	}
	if ( (nOffset < 0) || (nLength < 0)
		|| (pBuf->m_lenBuf < (size_t) (nOffset + nLength)) )
	{
		context.ThrowExceptionError
			( L"write 引数の指標が範囲外です",
									L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	size_t	nWritten =
				pFile->Write
					( pBuf->m_ptrBuf + nOffset, (size_t) nLength ) ;
	if ( nWritten < (size_t) nLength )
	{
		context.ThrowExceptionError
			( L"write に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void flush()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_flush
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SBufferedFile *	pFile =
			ESLTypeCast<SBufferedFile>( GetThisFile( context, pThis ) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"OutputStream.flush の this が OutputStream ではありません" ) ;
		return	NULL ;
	}
	pFile->FlushBuffer() ;
	return	NULL ;
}

// void close()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNative = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNative == NULL )
	{
		context.ThrowExceptionError( L"this が OutputStream ではありません" ) ;
		return	NULL ;
	}
	pNative->AttachObject( NULL ) ;
	return	NULL ;
}

// void writeBoolean( boolean v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeBoolean
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t	b = 0 ;
	if ( arg.BooleanAt( 0 ) )
	{
		b = 1 ;
	}
	if ( pFile->Write( &b, sizeof(uint8_t) ) < sizeof(uint8_t) )
	{
		context.ThrowExceptionError
			( L"writeBoolean に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeByte( int v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeByte
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t	b = (uint8_t) arg.IntAt( 0 ) ;
	if ( pFile->Write( &b, sizeof(uint8_t) ) < sizeof(uint8_t) )
	{
		context.ThrowExceptionError
			( L"writeByte に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeShort( int v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeShort
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint16_t	w = (uint16_t) arg.IntAt( 0 ) ;
	if ( pFile->Write( &w, sizeof(uint16_t) ) < sizeof(uint16_t) )
	{
		context.ThrowExceptionError
			( L"writeShort に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeChar( int v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeChar
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint16_t	w = (uint16_t) arg.IntAt( 0 ) ;
	if ( pFile->Write( &w, sizeof(uint16_t) ) < sizeof(uint16_t) )
	{
		context.ThrowExceptionError
			( L"writeChar に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeInt( int v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint32_t	dw = (uint32_t) arg.IntAt( 0 ) ;
	if ( pFile->Write( &dw, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		context.ThrowExceptionError
			( L"writeInt に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeLong( long v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeLong
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	n = arg.LongAt( 0 ) ;
	if ( pFile->Write( &n, sizeof(int64_t) ) < sizeof(int64_t) )
	{
		context.ThrowExceptionError
			( L"writeLong に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeFloat( float v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeFloat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	float32_t	n = (float32_t) arg.DoubleAt( 0 ) ;
	if ( pFile->Write( &n, sizeof(float32_t) ) < sizeof(float32_t) )
	{
		context.ThrowExceptionError
			( L"writeFloat に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeDouble( double v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeDouble
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	float64_t	n = arg.DoubleAt( 0 ) ;
	if ( pFile->Write( &n, sizeof(float64_t) ) < sizeof(float64_t) )
	{
		context.ThrowExceptionError
			( L"writeDouble に失敗しました", L"IOException" ) ;
	}
	return	NULL ;
}

// void writeChars( String s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeChars
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	str = arg.StringAt( 0 ) ;
	if ( pFile->Write
		( str.GetArray(),
			str.GetLength() * sizeof(uint16_t) )
					< str.GetLength() * sizeof(uint16_t) )
	{
		context.ThrowExceptionError
			( L"writeChars に失敗しました", L"IOException" ) ;
	}
	str.FinishArray() ;
	return	NULL ;
}

// void writeUTF( String s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_writeUTF
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	str = arg.StringAt( 0 ) ;
	//
	SArray<uint8_t>	bufUTF ;
	Charset::Encode
		( bufUTF, Charset::encodingUTF8, str, (ssize_t) str.GetLength() ) ;
	//
	uint32_t	nBytes = (uint32_t) bufUTF.GetLength() ;
	pFile->Write( &nBytes, sizeof(uint32_t) ) ;
	//
	if ( pFile->Write
		( bufUTF.GetArray(), bufUTF.GetLength() ) < bufUTF.GetLength() )
	{
		context.ThrowExceptionError
			( L"writeUTF に失敗しました", L"IOException" ) ;
	}
	bufUTF.FinishArray() ;
	return	NULL ;
}

// OutputStream printf( String fmt, ... )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSOutputStreamClass::method_printf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	if ( count < 1 )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString				strFormat = arg.StringAt( 0 ) ;
	SString				strDst ;
	context.FormatStringVlist
				( strDst, strFormat, ppArg + 1, count - 1 ) ;
	//
	Charset::EncodingType	encoding = Charset::encodingUTF8 ;
	SBufferedFile *			pBufFile = ESLTypeCast<SBufferedFile>( pFile ) ;
	if ( pBufFile != nullptr )
	{
		encoding = pBufFile->GetCharsetEncoding() ;
	}
	SArray<uint8_t>	bufUTF ;
	Charset::Encode
		( bufUTF, encoding, strDst, (ssize_t) strDst.GetLength() ) ;
	if ( pFile->Write
		( bufUTF.GetArray(), bufUTF.GetLength() ) < bufUTF.GetLength() )
	{
		context.ThrowExceptionError
			( L"printf に失敗しました", L"IOException" ) ;
	}
	bufUTF.FinishArray() ;
	//
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}


//////////////////////////////////////////////////////////////////////////////
// RandomAccessFile クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRandomAccessFileClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRandomAccessFileClass::RSRandomAccessFileClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRandomAccessFileClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String path, String mode",
				NULL, &RSRandomAccessFileClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close", NULL, L"",
				NULL, &RSRandomAccessFileClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLine", L"String", L"",
				NULL, &RSRandomAccessFileClass::method_readLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFilePointer", L"long", L"",
				NULL, &RSRandomAccessFileClass::method_getFilePointer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"long", L"",
				NULL, &RSRandomAccessFileClass::method_length, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seek", NULL, L"long pos",
				NULL, &RSRandomAccessFileClass::method_length, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setLength", NULL, L"long newLength",
				NULL, &RSRandomAccessFileClass::method_setLength, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputStream", L"InputStream", L"",
				NULL, &RSRandomAccessFileClass::method_getInputStream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputStream", L"InputStream", L"String encoding",
				NULL, &RSRandomAccessFileClass::method_getInputStream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getOutputStream", L"OutputStream", L"",
				NULL, &RSRandomAccessFileClass::method_getOutputStream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getOutputStream", L"OutputStream", L"String encoding",
				NULL, &RSRandomAccessFileClass::method_getOutputStream, NULL ) ;
	//
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int",
				L"Uint8Pointer b, int off, int len",
				NULL, &RSInputStreamClass::method_read4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int",
				L"ArrayBuffer b, int off, int len",
				NULL, &RSInputStreamClass::method_read4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"",
				NULL, &RSInputStreamClass::method_read1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"byte[] b",
				NULL, &RSInputStreamClass::method_read2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"read", L"int", L"byte[] b, int off, int len",
				NULL, &RSInputStreamClass::method_read3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readBoolean", L"boolean", L"",
				NULL, &RSInputStreamClass::method_readBoolean, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readByte", L"byte", L"",
				NULL, &RSInputStreamClass::method_readByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readChar", L"char", L"",
				NULL, &RSInputStreamClass::method_readChar, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readShort", L"short", L"",
				NULL, &RSInputStreamClass::method_readShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readInt", L"int", L"",
				NULL, &RSInputStreamClass::method_readInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLong", L"long", L"",
				NULL, &RSInputStreamClass::method_readLong, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUnsignedByte", L"int", L"",
				NULL, &RSInputStreamClass::method_readUnsignedByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUnsignedShort", L"int", L"",
				NULL, &RSInputStreamClass::method_readUnsignedShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readFloat", L"float", L"",
				NULL, &RSInputStreamClass::method_readFloat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readDouble", L"double", L"",
				NULL, &RSInputStreamClass::method_readDouble, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readUTF", L"String", L"",
				NULL, &RSInputStreamClass::method_readUTF, NULL ) ;
	//
	AddVirtualDescriptiveAs
		( context, perr, L"write",
				NULL, L"Uint8Pointer b, int off, int len",
				NULL, &RSOutputStreamClass::method_write4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write",
				NULL, L"ArrayBuffer b, int off, int len",
				NULL, &RSOutputStreamClass::method_write4, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"int b",
				NULL, &RSOutputStreamClass::method_write1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"byte[] b",
				NULL, &RSOutputStreamClass::method_write2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write", NULL, L"byte[] b, int off, int len",
				NULL, &RSOutputStreamClass::method_write3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeBoolean", NULL, L"boolean v",
				NULL, &RSOutputStreamClass::method_writeBoolean, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeByte", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeByte, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeShort", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeShort, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeChar", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeChar, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeInt", NULL, L"int v",
				NULL, &RSOutputStreamClass::method_writeInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeLong", NULL, L"long v",
				NULL, &RSOutputStreamClass::method_writeLong, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeFloat", NULL, L"float v",
				NULL, &RSOutputStreamClass::method_writeFloat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeDouble", NULL, L"double v",
				NULL, &RSOutputStreamClass::method_writeDouble, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeChars", NULL, L"String s",
				NULL, &RSOutputStreamClass::method_writeChars, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeUTF", NULL, L"String s",
				NULL, &RSOutputStreamClass::method_writeUTF, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"printf", L"RandomAccessFile", L"String fmt, ...",
				NULL, &RSOutputStreamClass::method_printf, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRandomAccessFileClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SFileInterface>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	RSRandomAccessFileClass::GetThisFile( RSContext& context, RSObject* pThis )
{
	SFileInterface *	pFile = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<SFileInterface>( pNativeObj->GetObject() ) ;
	}
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が RandomAccessFile ではありません" ) ;
	}
	return	pFile ;
}

SSystem::SFileInterface *
	RSRandomAccessFileClass::GetFileOf( RSContext& context, RSObject* pObj )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		return	ESLTypeCast<SFileInterface>( pNativeObj->GetObject() ) ;
	}
	return	NULL ;
}

// void <init>( String path, String mode )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RandomAccessFile.<init> の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt( 0 ) ;
	SString	strMode = arg.StringAt( 1 ) ;
	long	modeOpen = 0 ;
	if ( strMode == L"r" )
	{
		modeOpen = SFileOpener::shareRead ;
	}
	else if ( (strMode == "rw")
			|| (strMode == "rws") || (strMode == "rwd") )
	{
		modeOpen = SFileOpener::shareReadWrite ;
	}
	else
	{
		context.ThrowExceptionError
			( L"RandomAccessFile.<init> の mode 引数が不正です",
										L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	SFileInterface *
			pFile = SFileOpener::DefaultNewOpenFile( strPath, modeOpen ) ;
	if ( pFile == NULL )
	{
		if ( modeOpen == SFileOpener::shareReadWrite )
		{
			pFile = SFileOpener::DefaultNewOpenFile
				( strPath, SFileOpener::modeCreate | SFileOpener::modeRead ) ;
		}
		if ( pFile == NULL )
		{
			context.ThrowExceptionError
				( strPath + L" ファイルを開けませんでした",
									L"FileNotFoundException" ) ;
			return	NULL ;
		}
	}
	pNativeObj->SetObject( *pFile ) ;
	return	NULL ;
}

// void close()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNative = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNative == NULL )
	{
		context.ThrowExceptionError( L"this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	pNative->AttachObject( NULL ) ;
	return	NULL ;
}

// String readLine()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_readLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"readLine の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	size_t	nBlockSize = 1 ;
	if ( pFile->IsSeekable() )
	{
		nBlockSize = 0x100 ;
	}
	SQueueBuffer	qbufRead ;
	SArray<uint8_t>	bufLine ;
	uint8_t *	pBuf = bufLine.GetArray( nBlockSize ) ;
	for ( ; ; )
	{
		size_t	nReadBytes = pFile->Read( pBuf, nBlockSize ) ;
		if ( nReadBytes == 0 )
		{
			break ;
		}
		ssize_t	iEndOfLine = -1 ;
		for ( size_t i = 0; i < nBlockSize; i ++ )
		{
			if ( pBuf[i] == '\n' )
			{
				qbufRead.Write( pBuf, i ) ;
				iEndOfLine = (ssize_t) i + 1 ;
				break ;
			}
			else if ( pBuf[i] == '\r' )
			{
				qbufRead.Write( pBuf, i ) ;
				if ( i + 1 < nBlockSize )
				{
					if ( pBuf[i + 1] == '\n' )
					{
						iEndOfLine = (ssize_t) i + 2 ;
					}
					else
					{
						iEndOfLine = (ssize_t) i + 1 ;
					}
				}
				else
				{
					nReadBytes = pFile->Read( pBuf, 1 ) ;
					if ( pBuf[0] == '\n' )
					{
						iEndOfLine = 1 ;
					}
					else
					{
						iEndOfLine = 0 ;
					}
				}
				break ;
			}
		}
		if ( iEndOfLine >= 0 )
		{
			if ( ((size_t) iEndOfLine < nReadBytes) && pFile->IsSeekable() )
			{
				pFile->Seek
					( iEndOfLine - (ssize_t) nReadBytes,
								SFileInterface::FromCurrent ) ;
			}
			break ;
		}
		qbufRead.Write( pBuf, nReadBytes ) ;
	}
	bufLine.FinishArray() ;
	//
	SString			strLine ;
	size_t			nBytes ;
	const uint8_t *	pBufLine = qbufRead.GetBuffer( nBytes ) ;
	Charset::Decode
		( strLine, Charset::encodingUTF8, pBufLine, (ssize_t) nBytes ) ;
	qbufRead.ReleaseBuffer( (ssize_t) nBytes ) ;
	//
	return	context.new_String( strLine ) ;
}

// long getFilePointer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_getFilePointer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"length の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pFile->GetPosition() ) ;
}

// long length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"length の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pFile->GetLength() ) ;
}

// void seek( long pos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_seek
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"seek の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pFile->Seek( arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// void setLength( long newLength )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_setLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"seek の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pFile->IsSeekable() )
	{
		int64_t	fpos = pFile->GetPosition() ;
		pFile->Seek( arg.LongAt( 0 ) ) ;
		pFile->SetEndOfFile() ;
		pFile->Seek( fpos ) ;
	}
	return	NULL ;
}

// InputStream getInputStream()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_getInputStream
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"getInputStream の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList		arg( ppArg, count ) ;
	Charset::EncodingType	encoding = Charset::encodingUTF8 ;
	SString	strEncodingType = arg.StringAt( 0 ) ;
	if ( !strEncodingType.IsEmpty() )
	{
		encoding = Charset::GetEncodingType( strEncodingType ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	SBufferedFile *	pBufFile = new SBufferedFile( pFile, pFile, false ) ;
	pBufFile->SetCharsetEncoding( encoding ) ;
	//
	RSNativeObject *	pNativeObj =
			new RSNativeObject
					( NULL, context.GetClassAs( L"InputStream" ) ) ;
	pNativeObj->SetObject( *pBufFile ) ;
	pNativeObj->AddOwnObject( pThis ) ;
	return	pNativeObj ;
}

// OutputStream getOutputStream()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomAccessFileClass::method_getOutputStream
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SFileInterface *	pFile = GetThisFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"getOutputStream の this が RandomAccessFile ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList		arg( ppArg, count ) ;
	Charset::EncodingType	encoding = Charset::encodingUTF8 ;
	SString	strEncodingType = arg.StringAt( 0 ) ;
	if ( !strEncodingType.IsEmpty() )
	{
		encoding = Charset::GetEncodingType( strEncodingType ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	SBufferedFile *	pBufFile = new SBufferedFile( pFile, pFile, false ) ;
	pBufFile->SetCharsetEncoding( encoding ) ;
	//
	RSNativeObject *	pNativeObj =
			new RSNativeObject
					( NULL, context.GetClassAs( L"OutputStream" ) ) ;
	pNativeObj->SetObject( *pBufFile ) ;
	pNativeObj->AddOwnObject( pThis ) ;
	return	pNativeObj ;
}


//////////////////////////////////////////////////////////////////////////////
// SmartBufferFile クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSmartBufferFileClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSmartBufferFileClass::RSSmartBufferFileClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSSmartBufferFileClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"RandomAccessFile" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSmartBufferFileClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSSmartBufferFileClass::method_init, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSmartBufferFileClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SSmartBuffer>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SSmartBuffer *
	RSSmartBufferFileClass::GetThisFile( RSContext& context, RSObject* pThis )
{
	SSmartBuffer *	pFile = GetFileOf( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が SmartBufferFile ではありません" ) ;
	}
	return	pFile ;
}

SSystem::SSmartBuffer *
	RSSmartBufferFileClass::GetFileOf( RSContext& context, RSObject* pObj )
{
	SSmartBuffer *	pFile = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<SSmartBuffer>( pNativeObj->GetObject() ) ;
	}
	return	pFile ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSmartBufferFileClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"SmartBufferFile.<init> の this が SmartBufferFile ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SFileOpener*) new SSmartBuffer ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// NoaFileArchiver.FileInfo クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSNoaFileArchiverClass::RSFileInfoClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSNoaFileArchiverClass::RSFileInfoClass::RSFileInfoClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSNoaFileArchiverClass::RSFileInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nBytes", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nAttribute", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nEncodeType", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"dtFileTime", context.new_ObjectPointer( L"Date" ) ) ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strFilename", L"" ) ;
}

// 変換
//////////////////////////////////////////////////////////////////////////////
void RSNoaFileArchiverClass::RSFileInfoClass::ObjectToFileEntry
	( RSContext& context, RSObject * pObj,
		ERISA::SGLArchiveFile::FILE_ENTRY_EX& fe, SSystem::SString& strFileName )
{
	fe.nBytes =
		(uint64_t) pObj->GetMemberIntegerAs( context, L"nBytes" ) ;
	fe.nAttribute =
		(uint32_t) pObj->GetMemberIntegerAs( context, L"nAttribute" ) ;
	fe.nEncodeType =
		(uint32_t) pObj->GetMemberIntegerAs( context, L"nEncodeType" ) ;
	//
	RSObject *	pObjDate = pObj->GetMemberAs( context, L"dtFileTime" ) ;
	ESLAssert( pObjDate != NULL ) ;
	RSDate *	pDate = ESLTypeCast<RSDate>( pObjDate->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		fe.ftFileTime.nSecond = (uint8_t) pDate->m_date.nSecond ;
		fe.ftFileTime.nMinute = (uint8_t) pDate->m_date.nMinute ;
		fe.ftFileTime.nHour = (uint8_t) pDate->m_date.nHour ;
		fe.ftFileTime.nWeek = (uint8_t) pDate->m_date.nWeek ;
		fe.ftFileTime.nDay = (uint8_t) pDate->m_date.nDay ;
		fe.ftFileTime.nMonth = (uint8_t) pDate->m_date.nMonth ;
		fe.ftFileTime.nYear = (uint16_t) pDate->m_date.nYear ;
	}
	fe.nExtraInfoBytes = 0 ;
	//
	RSObject::ReleaseRef( pObjDate ) ;
	//
	strFileName = pObj->GetMemberStringAs( context, L"strFilename" ) ;
}

void RSNoaFileArchiverClass::RSFileInfoClass::ObjectFromFileEntry
	( RSContext& context, RSObject * pObj,
		const ERISA::SGLArchiveFile::FILE_ENTRY_EX& fe,
						const wchar_t * pwszFileName )
{
	pObj->SetMemberIntegerAs( context, L"nBytes", fe.nBytes ) ;
	pObj->SetMemberIntegerAs( context, L"nAttribute", fe.nAttribute ) ;
	pObj->SetMemberIntegerAs( context, L"nEncodeType", fe.nEncodeType ) ;
	//
	RSObject *	pObjDate = pObj->GetMemberAs( context, L"dtFileTime" ) ;
	ESLAssert( pObjDate != NULL ) ;
	RSDate *	pDate = ESLTypeCast<RSDate>( pObjDate->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		pDate->m_date.nMilliSec = 0 ;
		pDate->m_date.nSecond = fe.ftFileTime.nSecond ;
		pDate->m_date.nMinute = fe.ftFileTime.nMinute ;
		pDate->m_date.nHour = fe.ftFileTime.nHour ;
		pDate->m_date.nWeek = fe.ftFileTime.nWeek ;
		pDate->m_date.nDay = fe.ftFileTime.nDay ;
		pDate->m_date.nMonth = fe.ftFileTime.nMonth ;
		pDate->m_date.nYear = fe.ftFileTime.nYear ;
	}
	RSObject::ReleaseRef( pObjDate ) ;
	//
	pObj->SetMemberStringAs( context, L"strFilename", pwszFileName ) ;
}


//////////////////////////////////////////////////////////////////////////////
// NoaFileArchiver クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNoaFileArchiverClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSNoaFileArchiverClass::RSNoaFileArchiverClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSNoaFileArchiverClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"RandomAccessFile" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSNoaFileArchiverClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	RSFileInfoClass *
		pFileInfoClass = new RSFileInfoClass( context.GetClassClass() ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"FileInfo", pFileInfoClass ) ) ;
	pFileInfoClass->Initialize( context ) ;
	//
	CreateMemberIntegerAs
		( context, L"attrNormal",
			ERISA::SGLArchiveFile::attrNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrReadOnly",
			ERISA::SGLArchiveFile::attrReadOnly, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrHidden",
			ERISA::SGLArchiveFile::attrHidden, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrSystem",
			ERISA::SGLArchiveFile::attrSystem, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrDirectory",
			ERISA::SGLArchiveFile::attrDirectory, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrEndOfDirectory",
			ERISA::SGLArchiveFile::attrEndOfDirectory, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrNextDirectory",
			ERISA::SGLArchiveFile::attrNextDirectory, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"attrFileNameUTF8",
			ERISA::SGLArchiveFile::attrFileNameUTF8, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"encodeRaw",
			ERISA::SGLArchiveFile::encodeRaw, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodeERISA",
			ERISA::SGLArchiveFile::encodeERISA, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodeCrypt32",
			ERISA::SGLArchiveFile::encodeCrypt32, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodeERISACrypt32",
			ERISA::SGLArchiveFile::encodeERISACrypt32, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSNoaFileArchiverClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"openArchive",
			L"boolean", L"RandomAccessFile file",
			NULL, &RSNoaFileArchiverClass::method_openArchive, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createArchive",
			L"boolean",
			L"RandomAccessFile file, NoaFileArchiver.FileInfo[] dirRoot",
			NULL, &RSNoaFileArchiverClass::method_createArchive, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close", NULL, L"",
			NULL, &RSNoaFileArchiverClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"descendDirectory",
			L"boolean", L"String sDirName",
			NULL, &RSNoaFileArchiverClass::method_descendDirectory, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createDirectory",
			L"boolean", L"String sDirName, NoaFileArchiver.FileInfo[] dirFiles",
			NULL, &RSNoaFileArchiverClass::method_createDirectory, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"ascendDirectory",
			L"boolean", L"",
			NULL, &RSNoaFileArchiverClass::method_ascendDirectory, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"descendFile",
			L"boolean", L"String sFileName, "
				L"String sPassword = null, boolean flagStream = false",
			NULL, &RSNoaFileArchiverClass::method_descendFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"ascendFile",
			L"boolean", L"",
			NULL, &RSNoaFileArchiverClass::method_ascendFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCurrentFileInfo",
			L"NoaFileArchiver.FileInfo", L"",
			NULL, &RSNoaFileArchiverClass::method_getCurrentFileInfo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"openFile",
			L"RandomAccessFile", L"String sFilePath",
			NULL, &RSNoaFileArchiverClass::method_openFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isExisting",
			L"boolean", L"String sFilePath",
			NULL, &RSNoaFileArchiverClass::method_isExisting, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"listFiles",
			L"String[]", L"String sDirPath",
			NULL, &RSNoaFileArchiverClass::method_listFiles, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"listDirectories",
			L"String[]", L"String sDirPath",
			NULL, &RSNoaFileArchiverClass::method_listDirectories, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSNoaFileArchiverClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<ERISA::SGLArchiveFile>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
ERISA::SGLArchiveFile *
	RSNoaFileArchiverClass::GetThisFile( RSContext& context, RSObject* pThis )
{
	ERISA::SGLArchiveFile *	pFile = GetFileOf( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が NoaFileArchiver ではありません" ) ;
	}
	return	pFile ;
}

ERISA::SGLArchiveFile *
	RSNoaFileArchiverClass::GetFileOf( RSContext& context, RSObject* pObj )
{
	ERISA::SGLArchiveFile *	pFile = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<ERISA::SGLArchiveFile>( pNativeObj->GetObject() ) ;
	}
	return	pFile ;
}

// NoaFileArchiver.FileInfo[] から ERISA::SGLArchiveFile::SDirectory
//////////////////////////////////////////////////////////////////////////////
void RSNoaFileArchiverClass::DirectoryFromObject
	( RSContext& context,
		ERISA::SGLArchiveFile::SDirectory& dirFiles, RSObject * pObj )
{
	dirFiles.RemoveAll() ;
	if ( pObj == NULL )
	{
		return ;
	}
	size_t	nCount = pObj->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObjFile = pObj->GetElementAt( context, (int) i ) ;
		if ( pObjFile != NULL )
		{
			ERISA::SGLArchiveFile::FILE_ENTRY_EX	fe ;
			ERISA::SGLArchiveFile::FILE_EXTRA_INFO	fxi ;
			SString	strFileName ;
			//
			RSFileInfoClass::ObjectToFileEntry
				( context, pObjFile, fe, strFileName ) ;
			fe.nAttribute |= ERISA::SGLArchiveFile::attrFileNameUTF8 ;
			//
			pObjFile->ReleaseRef() ;
			//
			eslFillMemory( &fxi, 0, sizeof(fxi) ) ;
			fe.nExtraInfoBytes = sizeof(fxi) ;
			//
			dirFiles.AddFileEntry( strFileName, fe, &fxi ) ;
		}
	}
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"NoaFileArchiver.<init> の this が NoaFileArchiver ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( *(new ERISA::SGLArchiveFile) ) ;
	return	NULL ;
}

// boolean openArchive( RandomAccessFile file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_openArchive
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *			pObjFile = arg.ObjectAt( 0 ) ;
	SFileInterface *	pArcFile =
				RSRandomAccessFileClass::GetFileOf( context, pObjFile ) ;
	if ( pArcFile == NULL )
	{
		context.ThrowExceptionError
			( L"引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	if ( pThisFile->OpenArchive( pArcFile, false ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	ESLAssert( pNativeObj != NULL ) ;
	pNativeObj->AddOwnObject( pObjFile ) ;
	return	context.new_Boolean( true ) ;
}

// boolean createArchive
//	( RandomAccessFile file, NoaFileArchiver.FileInfo[] dirRoot )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_createArchive
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *			pObjFile = arg.ObjectAt( 0 ) ;
	SFileInterface *	pArcFile =
				RSRandomAccessFileClass::GetFileOf( context, pObjFile ) ;
	if ( pArcFile == NULL )
	{
		context.ThrowExceptionError
			( L"引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	ERISA::SGLArchiveFile::SDirectory	dirRoot ;
	DirectoryFromObject( context, dirRoot, arg.ObjectAt( 1 ) ) ;
	if ( pThisFile->OpenArchive
		( pArcFile, false, SFileOpener::modeCreate, &dirRoot ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	ESLAssert( pNativeObj != NULL ) ;
	pNativeObj->AddOwnObject( pObjFile ) ;
	return	context.new_Boolean( true ) ;
}

// void close()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	ESLAssert( pNativeObj != NULL ) ;
	RSNativeObject *	pRefFile =
			pNativeObj->FindOwnNativeObject( pThisFile->GetAttachedFile() ) ;
	//
	pThisFile->CloseArchive() ;
	//
	pNativeObj->ReleaseOwnObject( pRefFile ) ;
	return	NULL ;
}

// boolean descendDirectory( String sDirName )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_descendDirectory
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pThisFile->DescendDirectory( arg.StringAt(0) ) == errSuccess ) ;
}

// boolean createDirectory
//	( String sDirName, NoaFileArchiver.FileInfo[] dirFiles )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_createDirectory
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	ERISA::SGLArchiveFile::SDirectory	dirFiles ;
	DirectoryFromObject( context, dirFiles, arg.ObjectAt( 1 ) ) ;
	return	context.new_Boolean
		( pThisFile->DescendDirectory
			( arg.StringAt(0), &dirFiles ) == errSuccess ) ;
}

// boolean ascendDirectory()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_ascendDirectory
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pThisFile->AscendDirectory() == errSuccess ) ;
}

// boolean descendFile
//	( String sFileName,
//		String sPassword, boolean flagStream = false )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_descendFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pThisFile->DescendFile
			( arg.StringAt(0), arg.StringAt(1),
				(arg.BooleanAt(2)
					? ERISA::SGLArchiveFile::openAsStream
					: ERISA::SGLArchiveFile::openAsNormal) ) == errSuccess ) ;
}

// boolean ascendFile()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_ascendFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pThisFile->AscendFile() == errSuccess ) ;
}

// NoaFileArchiver.FileInfo getCurrentFileInfo()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_getCurrentFileInfo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	const ERISA::SGLArchiveFile::FileReferenceInfo *
				pfri = pThisFile->GetCurrentFileInfo() ;
	if ( pfri == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObjInfo = context.new_Object( L"NoaFileArchiver.FileInfo" ) ;
	if ( pObjInfo != NULL )
	{
		SString	strFileName ;
		Charset::Decode
			( strFileName, Charset::encodingUTF8,
				pfri->pszFilename, pfri->lenFilename ) ;
		//
		RSFileInfoClass::ObjectFromFileEntry
			( context, pObjInfo, *(pfri->pfeEntry), strFileName ) ;
	}
	return	pObjInfo ;
}

// RandomAccessFile openFile( String sFilePath )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_openFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	strPath.Replace( L'/', L'\\' ) ;
	//
	SFileInterface *	pFile =
		pThisFile->NewOpenFile( strPath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSNativeObject *	pObjFile =
		new RSNativeObject
			( NULL, context.GetClassAs( L"RandomAccessFile" ) ) ;
	pObjFile->SetObject( *pFile ) ;
	return	pObjFile ;
}

// boolean isExisting( String sFilePath )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_isExisting
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	strPath.Replace( L'/', L'\\' ) ;
	//
	return	context.new_Boolean( pThisFile->IsExisting( strPath ) ) ;
}

// String[] listFiles( String sDirPath )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_listFiles
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	strPath.Replace( L'/', L'\\' ) ;
	//
	SObjectArray<SString>	lstFiles ;
	pThisFile->ListSubFiles( lstFiles, strPath ) ;
	//
	RSObject *	pObjList =
		context.new_Array( 0x7FFFFFFF, context.GetStringClass() ) ;
	for ( size_t i = 0; i < lstFiles.GetLength(); i ++ )
	{
		SString *	pstrFile = lstFiles.GetAt( i ) ;
		if ( pstrFile != NULL )
		{
			pObjList->SetElementStringAt( context, (int) i, *pstrFile ) ;
		}
	}
	return	pObjList ;
}

// String[] listDirectories( String sDirPath )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNoaFileArchiverClass::method_listDirectories
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLArchiveFile *	pThisFile = GetThisFile( context, pThis ) ;
	if ( pThisFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	strPath.Replace( L'/', L'\\' ) ;
	//
	SObjectArray<SString>	lstDirs ;
	pThisFile->ListSubDirectories( lstDirs, strPath ) ;
	//
	RSObject *	pObjList =
		context.new_Array( 0x7FFFFFFF, context.GetStringClass() ) ;
	for ( size_t i = 0; i < lstDirs.GetLength(); i ++ )
	{
		SString *	pstrDir = lstDirs.GetAt( i ) ;
		if ( pstrDir != NULL )
		{
			pObjList->SetElementStringAt( context, (int) i, *pstrDir ) ;
		}
	}
	return	pObjList ;
}


//////////////////////////////////////////////////////////////////////////////
// HttpInputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSHttpInputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSHttpInputStreamClass::RSHttpInputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSHttpInputStreamClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"InputStream" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSHttpInputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSHttpInputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"openURL",
			L"boolean", L"String url, String agent, "
			L"Uint8Pointer data = null, int len = -1, "
			L"String strContentType = null",
			NULL, &RSHttpInputStreamClass::method_openURL, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRequest",
			L"boolean", L"String url, String cmd = \"GET\"",
			NULL, &RSHttpInputStreamClass::method_setRequest, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSendURLFormData",
			L"boolean", L"String param, int encoding = String.encodingUTF8",
			NULL, &RSHttpInputStreamClass::method_setSendURLFormData, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSendData",
			L"boolean", L"Uint8Pointer b, int len",
			NULL, &RSHttpInputStreamClass::method_setSendData, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addHeader",
			L"boolean", L"String strHeader",
			NULL, &RSHttpInputStreamClass::method_addHeader, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"connect", L"boolean", L"",
			NULL, &RSHttpInputStreamClass::method_connect, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sendRequest", L"boolean", L"",
			NULL, &RSHttpInputStreamClass::method_sendRequest, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryStatusCode", L"int", L"",
			NULL, &RSHttpInputStreamClass::method_queryStatusCode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentLength", L"long", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentLength, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentType", L"String", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentType, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentTypeCharset", L"String", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentTypeCharset, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentTransferEncoding", L"String", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentTransferEncoding, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentDate", L"Date", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentDate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryContentLastModified", L"Date", L"",
			NULL, &RSHttpInputStreamClass::method_queryContentLastModified, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSHttpInputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SHttpFileInterface>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SHttpFileInterface *
	RSHttpInputStreamClass::GetThisHttpFile( RSContext& context, RSObject* pThis )
{
	SHttpFileInterface *	pFile = GetHttpFileOf( context, pThis ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"this が HttpInputStream ではありません" ) ;
	}
	return	pFile ;
}

SHttpFileInterface *
	RSHttpInputStreamClass::GetHttpFileOf( RSContext& context, RSObject* pObj )
{
	SHttpFileInterface *	pFile = NULL ;
	RSNativeObject *		pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pFile = ESLTypeCast<SHttpFileInterface>( pNativeObj->GetObject() ) ;
	}
	return	pFile ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"InputStream.<init> の this が InputStream ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SFileOpener*) new SHttpFile ) ;
	return	NULL ;
}

// boolean openURL
//	( String url, String agent,
//		Uint8Pointer data = null, int len = -1,
//		String strContentType = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_openURL
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString		strURL = arg.StringAt( 0 ) ;
	SString		strAgent = arg.StringAt( 1 ) ;
	size_t		nDataBound ;
	uint8_t *	pbytData = arg.PointerAt( 2, &nDataBound ) ;
	ssize_t		nBytes = (ssize_t) arg.IntAt( 3, -1 ) ;
	SString		strContentType = arg.StringAt( 4 ) ;
	//
	return	context.new_Boolean
		( pFile->OpenURL
			( strURL, strAgent,
				pbytData, nBytes, strContentType ) == errSuccess ) ;
}

// boolean setRequest( String url, String cmd = "GET" )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_setRequest
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strURL = arg.StringAt( 0 ) ;
	SString	strCmd = arg.StringAt( 1, L"GET" ) ;
	//
	return	context.new_Boolean
		( pFile->SetRequest( strURL, strCmd ) == errSuccess ) ;
}

// boolean setSendURLFormData
//	( String param, int encoding = String.encodingUTF8 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_setSendURLFormData
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strParam = arg.StringAt( 0 ) ;
	int		nEncoding = arg.IntAt( 1, Charset::encodingUTF8 ) ;
	//
	return	context.new_Boolean
		( pFile->SetSendURLFormData
			( strParam, (Charset::EncodingType) nEncoding ) == errSuccess ) ;
}

// boolean setSendData( Uint8Pointer b, int len )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_setSendData
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nDataBound ;
	uint8_t *	pbytData = arg.PointerAt( 0, &nDataBound ) ;
	ssize_t		nBytes = (ssize_t) arg.IntAt( 1, -1 ) ;
	//
	return	context.new_Boolean
		( pFile->SetSendData( pbytData, (ssize_t) nBytes ) == errSuccess ) ;
}

// boolean addHeader( String strHeader )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_addHeader
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strHeader = arg.StringAt( 0 ) ;
	//
	return	context.new_Boolean
				( pFile->AddHeader( strHeader ) == errSuccess ) ;
}

// boolean connect()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_connect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pFile->Connect() == errSuccess ) ;
}

// boolean sendRequest()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_sendRequest
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pFile->SendRequest() == errSuccess ) ;
}

// int queryStatusCode()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryStatusCode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint32_t	codeStatus ;
	if ( pFile->QueryStatusCode( codeStatus ) )
	{
		context.ThrowExceptionError( L"HTTP ステータスコードを取得できません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( codeStatus ) ;
}

// long queryContentLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	uint64_t	nLength ;
	if ( pFile->QueryContentLength( nLength ) )
	{
		context.ThrowExceptionError( L"HTTP コンテンツ長を取得できません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( nLength ) ;
}

// String queryContentType()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	SString	strType ;
	if ( pFile->QueryContentType( strType ) )
	{
		context.ThrowExceptionError
			( L"HTTP コンテンツタイプを取得できません" ) ;
		return	NULL ;
	}
	return	context.new_String( strType ) ;
}

// String queryContentTypeCharset()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentTypeCharset
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	SString	strCharset ;
	if ( pFile->QueryContentTypeCharset( strCharset ) )
	{
		context.ThrowExceptionError
			( L"HTTP コンテンツ文字セットを取得できません" ) ;
		return	NULL ;
	}
	return	context.new_String( strCharset ) ;
}

// String queryContentTransferEncoding()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentTransferEncoding
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	SString	strEncoding ;
	if ( pFile->QueryContentTransferEncoding( strEncoding ) )
	{
		context.ThrowExceptionError
			( L"HTTP コンテンツ転送エンコーディングを取得できません" ) ;
		return	NULL ;
	}
	return	context.new_String( strEncoding ) ;
}

// Date queryContentDate()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentDate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	DATE_TIME	dt ;
	if ( pFile->QueryContentDate( dt ) )
	{
		context.ThrowExceptionError
			( L"HTTP コンテンツ転送エンコーディングを取得できません" ) ;
		return	NULL ;
	}
	return	new RSDate( context.GetClassAs( L"Date" ), dt ) ;
}

// Date queryContentLastModified()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSHttpInputStreamClass::method_queryContentLastModified
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SHttpFileInterface *	pFile = GetThisHttpFile( context, pThis ) ;
	if ( pFile == NULL )
	{
		return	NULL ;
	}
	DATE_TIME	dt ;
	if ( pFile->QueryContentLastModified( dt ) )
	{
		context.ThrowExceptionError
			( L"HTTP コンテンツ転送エンコーディングを取得できません" ) ;
		return	NULL ;
	}
	return	new RSDate( context.GetClassAs( L"Date" ), dt ) ;
}


