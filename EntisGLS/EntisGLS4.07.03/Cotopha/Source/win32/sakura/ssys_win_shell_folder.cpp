
#define	_WIN32_WINNT	0x05010000
#include <sakura/sakura.h>
#include <sakura/ssys_win_shell_folder.h>
#include <shlwapi.h>

#pragma comment( lib, "shlwapi.lib" )

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ストリームファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SWin32StreamFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32StreamFile::SWin32StreamFile
	( IStream * stream, bool fSeekable, int64_t nSpecLength )
{
	m_stream = stream ;
	m_fSeekable = fSeekable ;
	m_nSpecLength = nSpecLength ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32StreamFile::~SWin32StreamFile( void )
{
	if ( m_stream != NULL )
	{
		m_stream->Release() ;
		m_stream = NULL ;
	}
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32StreamFile::Duplicate( void ) const
{
	if ( m_stream != NULL )
	{
		IStream *	stream = NULL ;
		if ( SUCCEEDED( m_stream->Clone( &stream ) ) )
		{
			return	new SWin32StreamFile( stream ) ;
		}
	}
	return	NULL ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SWin32StreamFile::Read( void * ptrBuf, size_t nBytes )
{
	ULONG	nRead = 0 ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->Read( ptrBuf, (ULONG) nBytes, &nRead ) ) )
	{
		return	(size_t) nRead ;
	}
	return	0 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SWin32StreamFile::Write( const void * ptrBuf, size_t nBytes )
{
	ULONG	nWritten = 0 ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->Write( ptrBuf, (ULONG) nBytes, &nWritten ) ) )
	{
		return	(size_t) nWritten ;
	}
	return	0 ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SWin32StreamFile::IsSeekable( void ) const
{
	return	m_fSeekable ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SWin32StreamFile::GetLength( void ) const
{
	STATSTG	stat ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->Stat( &stat, STATFLAG_NONAME ) ) )
	{
		return	(int64_t) stat.cbSize.QuadPart ;
	}
	return	m_nSpecLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SWin32StreamFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	DWORD	dwOrigin ;
	switch ( seekFrom )
	{
	case	FromBegin:
	default:
		dwOrigin = STREAM_SEEK_SET ;
		break ;
	case	FromCurrent:
		dwOrigin = STREAM_SEEK_CUR ;
		break ;
	case	FromEnd:
		dwOrigin = STREAM_SEEK_END ;
		break ;
	}
	LARGE_INTEGER	dlibMove ;
	ULARGE_INTEGER	dlibNewPos ;
	dlibMove.QuadPart = posFile ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->Seek( dlibMove, dwOrigin, &dlibNewPos ) ) )
	{
		return	(int64_t) dlibNewPos.QuadPart ;
	}
	return	-1 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SWin32StreamFile::GetPosition( void ) const
{
	LARGE_INTEGER	dlibMove ;
	ULARGE_INTEGER	dlibNewPos ;
	dlibMove.QuadPart = 0 ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->Seek( dlibMove, STREAM_SEEK_CUR, &dlibNewPos ) ) )
	{
		return	(int64_t) dlibNewPos.QuadPart ;
	}
	return	-1 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SWin32StreamFile::SetEndOfFile( void )
{
	int64_t	nPos = GetPosition() ;
	if ( nPos < 0 )
	{
		return	errFailed ;
	}
	ULARGE_INTEGER	dlibNewSize ;
	dlibNewSize.QuadPart = (ULONGLONG) nPos ;
	if ( (m_stream != NULL)
		&& SUCCEEDED( m_stream->SetSize( dlibNewSize ) ) )
	{
		return	errSuccess ;
	}
	return	errFailed ;
}



//////////////////////////////////////////////////////////////////////////////
// シェルフォルダー
//////////////////////////////////////////////////////////////////////////////

const GUID SWin32ShellFolder::CLSID_FileOperation =
	{ 0x3ad05575,0x8857,0x4850, { 0x92,0x77,0x11,0xb8,0x5b,0xdb,0x8e,0x09 } } ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32ShellFolder::SWin32ShellFolder( void )
{
	HMODULE	hModule = ::GetModuleHandle( "shell32.dll" ) ;
	m_apiSHGetFolderLocation =
		(API_SHGetFolderLocation)
			::GetProcAddress
				( hModule, "SHGetFolderLocation" ) ;
	m_apiSHCreateShellItem =
		(API_SHCreateShellItem)
			::GetProcAddress
				( hModule, "SHCreateShellItem" ) ;
	//
	m_pfOperation = NULL ;
	m_psiFolder = NULL ;
	m_pshFolder = NULL ;
	m_pidlFolder = NULL ;
	//
	m_fEnumFolders = false ;
	m_fEnumFiles = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32ShellFolder::~SWin32ShellFolder( void )
{
	Release() ;
	//
	if ( m_pfOperation != NULL )
	{
		m_pfOperation->Release() ;
		m_pfOperation = NULL ;
	}
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SWin32ShellFolder::Release( void )
{
	if ( m_pshFolder != NULL )
	{
		m_pshFolder->Release() ;
		m_pshFolder = NULL ;
	}
	if ( m_psiFolder != NULL )
	{
		m_psiFolder->Release() ;
		m_psiFolder = NULL ;
	}
	m_strFullPath.FreeArray() ;
	//
	m_aFolderNest.RemoveAll() ;
	//
	ReleaseChildrenItems() ;
}

void SWin32ShellFolder::ReleaseChildrenItems( void )
{
	const LPITEMIDLIST *	pidlFolders = m_aFolders.GetConstArray() ;
	for ( size_t i = 0; i < m_aFolders.GetLength(); i ++ )
	{
		if ( pidlFolders[i] )
		{
			CoTaskMemFree( pidlFolders[i] ) ;
		}
	}
	m_fEnumFolders = false ;
	m_aFolders.RemoveAll() ;
	//
	const LPITEMIDLIST *	pidlFiles = m_aFiles.GetConstArray() ;
	for ( size_t i = 0; i < m_aFiles.GetLength(); i ++ )
	{
		if ( pidlFiles[i] )
		{
			CoTaskMemFree( pidlFiles[i] ) ;
		}
	}
	m_fEnumFiles = false ;
	m_aFiles.RemoveAll() ;
	m_aFileNames.RemoveAll() ;
}

// シェル上のフォルダパスを指定して移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::SearchShellFolder( const wchar_t * pwszPath )
{
	SString	strPath = pwszPath ;
	if ( strPath.IsEmpty() )
	{
		return	errFailed ;
	}
	if ( m_strFullPath.CompareNoCase( pwszPath ) == 0 )
	{
		return	errSuccess ;
	}
	//
	// 現在のフォルダの親フォルダに一致するフォルダを検索
	//
	size_t	iDir = strPath.GetLength() ;
	for ( ; ; )
	{
		SString	strFolderPath = strPath.Left( iDir ) ;
		ssize_t	iFolder = -1 ;
		for ( size_t i = 0; i < m_aFolderNest.GetLength(); i ++ )
		{
			ShellItemIdList *	psidl = m_aFolderNest.GetAt( i ) ;
			if ( (psidl != NULL)
				&& (psidl->m_path == strFolderPath) )
			{
				iFolder = (ssize_t) i ;
				break ;
			}
		}
		if ( iFolder >= 0 )
		{
			while ( m_aFolderNest.GetLength() > (size_t) iFolder + 1 )
			{
				SError	err = AscendFolder() ;
				if ( err )
				{
					return	err ;
				}
			}
			break ;
		}
		while ( iDir > 0 )
		{
			wchar_t	wch = strPath.GetAt( -- iDir ) ;
			if ( (wch == L'\\') || (wch == L'/') )
			{
				break ;
			}
		}
		if ( iDir == 0 )
		{
			Release() ;
			break ;
		}
	}
	if ( m_aFolderNest.GetLength() == 0 )
	{
		//
		// ルート（コンピューター）を開く
		//
		SError	err = OpenDrives() ;
		if ( err )
		{
			return	err ;
		}
		size_t	nPathLen = m_strFullPath.GetLength() ;
		if ( (m_strFullPath.CompareNoCase
						( strPath.Left( nPathLen ) ) != 0)
			|| ((strPath.GetAt(nPathLen) != L'\\')
				&& (strPath.GetAt(nPathLen) != L'/')) )
		{
			return	errFailed ;
		}
		iDir = nPathLen ;
	}
	//
	// 順次フォルダを開いていく
	//
	while ( iDir < strPath.GetLength() )
	{
		wchar_t	wchSep = strPath.GetAt( iDir ) ;
		ESLAssert( (wchSep == L'\\') || (wchSep == L'/') ) ;
		//
		if ( ++ iDir >= strPath.GetLength() )
		{
			break ;
		}
		size_t	iLastDir = iDir ;
		do
		{
			wchSep = strPath.GetAt( iDir ) ;
			if ( (wchSep == L'\\') || (wchSep == L'/') )
			{
				break ;
			}
		}
		while ( ++ iDir < strPath.GetLength() ) ;
		//
		SError	err =
			DescendFolder
				( strPath.Middle
					( iLastDir, (ssize_t) (iDir - iLastDir) ) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	errSuccess ;
}

// 指定パスへ移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::OpenFolderPath( const wchar_t * pwszPath )
{
	Release() ;
	//
	m_pidlFolder = ILCreateFromPathW( pwszPath ) ;
	if ( m_pidlFolder == NULL )
	{
		return	errFailed ;
	}
	if ( (m_apiSHCreateShellItem == NULL)
		|| !SUCCEEDED
				( m_apiSHCreateShellItem
					( NULL, NULL, m_pidlFolder, &m_psiFolder ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED
		( m_psiFolder->BindToHandler
			( NULL, BHID_SFObject,
				IID_IShellFolder, (void**) &m_pshFolder ) ) )
	{
		return	errFailed ;
	}
	//
	m_aFolderNest.RemoveAll() ;
	//
	ShellItemIdList *	psidl = new ShellItemIdList ;
	psidl->m_path = pwszPath ;
	psidl->m_pidl = m_pidlFolder ;
	m_aFolderNest.Add( psidl ) ;
	//
	return	errSuccess ;
}

// ドライブルート（コンピューター）を取得
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::OpenDrives( void )
{
	Release() ;
	//
	if ( (m_apiSHGetFolderLocation == NULL)
		|| !SUCCEEDED( m_apiSHGetFolderLocation
			( NULL, CSIDL_DRIVES, NULL, 0, &m_pidlFolder ) ) )
	{
		return	errFailed ;
	}
	if ( (m_apiSHCreateShellItem == NULL)
		|| !SUCCEEDED( m_apiSHCreateShellItem
			( NULL, NULL, m_pidlFolder, &m_psiFolder ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED
		( m_psiFolder->BindToHandler
			( NULL, BHID_SFObject,
				IID_IShellFolder, (void**) &m_pshFolder ) ) )
	{
		return	errFailed ;
	}
	//
	LPWSTR	pszName = NULL ;
	if ( SUCCEEDED
		( m_psiFolder->GetDisplayName( SIGDN_NORMALDISPLAY, &pszName ) ) )
	{
		m_strFullPath = pszName ;
		::CoTaskMemFree( pszName ) ;
		ESLTrace( "%s\n", m_strFullPath.ToCharArray().GetConstArray() ) ;
	}
	//
	m_aFolderNest.RemoveAll() ;
	//
	ShellItemIdList *	psidl = new ShellItemIdList ;
	psidl->m_path = m_strFullPath ;
	psidl->m_pidl = m_pidlFolder ;
	m_aFolderNest.Add( psidl ) ;
	//
	return	errSuccess ;
}

// 下層フォルダへ移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::DescendFolder( const wchar_t * pwszName )
{
	EnumFolderItems() ;
	//
	for ( size_t i = 0; i < m_aFolders.GetLength(); i ++ )
	{
		LPITEMIDLIST	pidl = m_aFolders.At( i ) ;
		//
		IShellItem *	psiFolder = NULL ;
		if ( (m_apiSHCreateShellItem == NULL)
			|| !SUCCEEDED
				( m_apiSHCreateShellItem
					( NULL, m_pshFolder, pidl, &psiFolder ) ) )
		{
			continue ;
		}
		LPWSTR	pszName = NULL ;
		if ( SUCCEEDED
			( psiFolder->GetDisplayName
				( SIGDN_NORMALDISPLAY, &pszName ) ) )
		{
			FileName	strName = pszName ;
			::CoTaskMemFree( pszName ) ;
			//
			if ( strName == pwszName )
			{
				LPITEMIDLIST	pidlFolder = ILCombine( m_pidlFolder, pidl ) ;
				ReleaseChildrenItems() ;
				//
				m_psiFolder->Release() ;
				m_pshFolder->Release() ;
				m_psiFolder = NULL ;
				m_pshFolder = NULL ;
				//
				IShellFolder *	pshFolder = NULL ;
				if ( !SUCCEEDED
					( psiFolder->BindToHandler
						( NULL, BHID_SFObject,
							IID_IShellFolder, (void**) &pshFolder ) ) )
				{
					CoTaskMemFree( pidlFolder ) ;
					psiFolder->Release() ;
					return	errFailed ;
				}
				//
				m_psiFolder = psiFolder ;
				m_pshFolder = pshFolder ;
				m_pidlFolder = pidlFolder ;
				//
				m_strFullPath = m_strFullPath.OffsetFilePath( strName ) ;
				//
				ShellItemIdList *	psidl = new ShellItemIdList ;
				psidl->m_path = m_strFullPath ;
				psidl->m_pidl = m_pidlFolder ;
				m_aFolderNest.Add( psidl ) ;
				//
				return	errSuccess ;
			}
		}
		psiFolder->Release() ;
	}
	return	errFailed ;
}

// 上層フォルダへ移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::AscendFolder( void )
{
	size_t	nNest = m_aFolderNest.GetLength() ;
	if ( nNest <= 1 )
	{
		return	errFailed ;
	}
	m_aFolderNest.RemoveAt( nNest - 1 ) ;
	//
	ShellItemIdList *	psidl = m_aFolderNest.GetLastAt() ;
	if ( psidl == NULL )
	{
		return	errFailed ;
	}
	//
	IShellItem *	psiFolder = NULL ;
	if ( (m_apiSHCreateShellItem == NULL)
		|| !SUCCEEDED
			( m_apiSHCreateShellItem
				( NULL, NULL, psidl->m_pidl, &psiFolder ) ) )
	{
		return	errFailed ;
	}
	IShellFolder *	pshFolder = NULL ;
	if ( !SUCCEEDED
		( psiFolder->BindToHandler
			( NULL, BHID_SFObject,
				IID_IShellFolder, (void**) &pshFolder ) ) )
	{
		psiFolder->Release() ;
		return	errFailed ;
	}
	//
	ReleaseChildrenItems() ;
	//
	m_psiFolder->Release() ;
	m_pshFolder->Release() ;
	//
	m_psiFolder = psiFolder ;
	m_pshFolder = pshFolder ;
	m_pidlFolder = psidl->m_pidl ;
	m_strFullPath = psidl->m_path ;
	//
	return	errSuccess ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32ShellFolder::OpenFile
	( const wchar_t * pwszName, long int nOpenFlags )
{
	IShellItem *	psiFile = GetFileShellItem( pwszName ) ;
	if ( (psiFile == NULL)
		&& (nOpenFlags & SFileOpener::modeCreateFile) )
	{
		IStorage *	pStorage = NULL ;
		if ( SUCCEEDED
			( m_psiFolder->BindToHandler
				( NULL, BHID_Storage,
					IID_IStorage, (void**) &pStorage ) ) )
		{
			IStream *	pStream = NULL ;
			if ( SUCCEEDED
				( pStorage->CreateStream
					( pwszName,
						AccessModeFromOpenFlags( nOpenFlags ),
												0, 0, &pStream ) ) )
			{
				pStorage->Release() ;
				m_fEnumFiles = true ;
				return	new SWin32StreamFile( pStream ) ;
			}
			pStorage->Release() ;
		}
		return	NULL ;
	}
	if ( psiFile != NULL )
	{
		IStream *	pStream = NULL ;
		IBindCtx *	pbc = CreateBindCtxWithOpenFlags( nOpenFlags ) ;
		if ( SUCCEEDED
			( psiFile->BindToHandler
				( pbc, BHID_Stream,
					IID_IStream, (void**) &pStream ) ) )
		{
			psiFile->Release() ;
			pbc->Release() ;
			return	new SWin32StreamFile( pStream ) ;
		}
		psiFile->Release() ;
		pbc->Release() ;
	}
	return	NULL ;
}

// ファイル作成
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32ShellFolder::NewFile( const wchar_t * pwszName )
{
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED
		( pfop->NewItem
			( m_psiFolder, FILE_ATTRIBUTE_NORMAL, pwszName, NULL, NULL ) ) )
	{
		return	NULL ;
	}
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	NULL ;
	}
	m_fEnumFiles = false ;
	return	OpenFile
				( pwszName, SFileOpener::modeCreateFile
							| SFileOpener::modeReadWrite ) ;
}

// フォルダ作成
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::NewFolder( const wchar_t * pwszName )
{
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED
		( pfop->NewItem
			( m_psiFolder, FILE_ATTRIBUTE_DIRECTORY, pwszName, NULL, NULL ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFolders = false ;
	return	errSuccess ;
}

// ファイル削除
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::DeleteFile( const wchar_t * pwszName )
{
	IShellItem *	psiFile = GetFileShellItem( pwszName ) ;
	if ( psiFile == NULL )
	{
		return	errFailed ;
	}
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED( pfop->DeleteItem( psiFile, NULL ) ) )
	{
		psiFile->Release() ;
		return	errFailed ;
	}
	psiFile->Release() ;
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFiles = false ;
	return	errSuccess ;
}

// フォルダ削除
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::DeleteFolder( const wchar_t * pwszName )
{
	IShellItem *	psiFolder = GetFolderShellItem( pwszName ) ;
	if ( psiFolder == NULL )
	{
		return	errFailed ;
	}
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED( pfop->DeleteItem( psiFolder, NULL ) ) )
	{
		psiFolder->Release() ;
		return	errFailed ;
	}
	psiFolder->Release() ;
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFolders = false ;
	return	errSuccess ;
}

// ファイル名変更
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::RenameFile
	( const wchar_t * pwszOldName, const wchar_t * pwszNewName )
{
	IShellItem *	psiFile = GetFileShellItem( pwszOldName ) ;
	if ( psiFile == NULL )
	{
		return	errFailed ;
	}
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED( pfop->RenameItem( psiFile, pwszNewName, NULL ) ) )
	{
		psiFile->Release() ;
		return	errFailed ;
	}
	psiFile->Release() ;
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFiles = false ;
	return	errSuccess ;
}

// フォルダ名変更
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::RenameFolder
	( const wchar_t * pwszOldName, const wchar_t * pwszNewName )
{
	IShellItem *	psiFolder = GetFolderShellItem( pwszOldName ) ;
	if ( psiFolder == NULL )
	{
		return	errFailed ;
	}
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED( pfop->RenameItem( psiFolder, pwszNewName, NULL ) ) )
	{
		psiFolder->Release() ;
		return	errFailed ;
	}
	psiFolder->Release() ;
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFolders = false ;
	return	errSuccess ;
}

// ファイル複製
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::CopyFile
	( IShellItem * psiSrcFile, const wchar_t * pwszDstName )
{
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED
		( pfop->CopyItem
			( psiSrcFile, m_psiFolder, pwszDstName, NULL ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFiles = false ;
	return	errSuccess ;
}

// ファイル移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32ShellFolder::MoveFile
	( IShellItem * psiSrcFile, const wchar_t * pwszDstName )
{
	IFileOperation *	pfop = GetFileOperation() ;
	if ( !SUCCEEDED
		( pfop->MoveItem 
			( psiSrcFile, m_psiFolder, pwszDstName, NULL ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED( pfop->PerformOperations() ) )
	{
		return	errFailed ;
	}
	m_fEnumFiles = false ;
	return	errSuccess ;
}

// 現在のフォルダ IShellItem を取得
//////////////////////////////////////////////////////////////////////////////
IShellItem * SWin32ShellFolder::GetFolderShellItem( void ) const
{
	IShellItem *	pshiFolder = m_psiFolder ;
	if ( pshiFolder != NULL )
	{
		pshiFolder->AddRef() ;
	}
	return	pshiFolder ;
}

// 現在のフォルダ IShellFolder を取得
//////////////////////////////////////////////////////////////////////////////
IShellFolder * SWin32ShellFolder::GetFolderShellFolder( void ) const
{
	IShellFolder *	psfFolder = m_pshFolder ;
	if ( psfFolder != NULL )
	{
		psfFolder->AddRef() ;
	}
	return	psfFolder ;
}

// フォルダの IShellItem を取得
//////////////////////////////////////////////////////////////////////////////
IShellItem * SWin32ShellFolder::GetFolderShellItem( const wchar_t * pwszName )
{
	EnumFolderItems() ;
	//
	for ( size_t i = 0; i < m_aFolders.GetLength(); i ++ )
	{
		LPITEMIDLIST	pidl = m_aFolders.At( i ) ;
		//
		IShellItem *	psiFolder = NULL ;
		if ( (m_apiSHCreateShellItem == NULL)
			|| !SUCCEEDED
				( m_apiSHCreateShellItem
					( NULL, m_pshFolder, pidl, &psiFolder ) ) )
		{
			continue ;
		}
		LPWSTR	pszName = NULL ;
		if ( SUCCEEDED
			( psiFolder->GetDisplayName
				( SIGDN_NORMALDISPLAY, &pszName ) ) )
		{
			FileName	strName = pszName ;
			::CoTaskMemFree( pszName ) ;
			//
			if ( strName == pwszName )
			{
				return	psiFolder ;
			}
		}
		psiFolder->Release() ;
	}
	return	NULL ;
}

// ファイルの IShellItem を取得
//////////////////////////////////////////////////////////////////////////////
IShellItem * SWin32ShellFolder::GetFileShellItem( const wchar_t * pwszName )
{
	EnumFileItems() ;
	//
	ssize_t	iFind = m_aFileNames.FindIndex( pwszName ) ;
	if ( iFind >= 0 )
	{
		LPITEMIDLIST *	ppidl = m_aFiles.GetAt( (size_t) iFind ) ;
		ESLAssert( ppidl != NULL ) ;
		if ( ppidl != NULL )
		{
			IShellItem *	psiFile = NULL ;
			if ( (m_apiSHCreateShellItem != NULL)
				&& SUCCEEDED
					( m_apiSHCreateShellItem
						( NULL, m_pshFolder, *ppidl, &psiFile ) ) )
			{
				return	psiFile ;
			}
		}
	}
	for ( size_t i = m_aFileNames.GetLength(); i < m_aFiles.GetLength(); i ++ )
	{
		LPITEMIDLIST	pidl = m_aFiles.At( i ) ;
		//
		IShellItem *	psiFile = NULL ;
		if ( (m_apiSHCreateShellItem == NULL)
			|| !SUCCEEDED
				( m_apiSHCreateShellItem
					( NULL, m_pshFolder, pidl, &psiFile ) ) )
		{
			continue ;
		}
		LPWSTR	pszName = NULL ;
		if ( SUCCEEDED
			( psiFile->GetDisplayName
				( SIGDN_NORMALDISPLAY, &pszName ) ) )
		{
			FileName *	pFileName = new FileName( pszName ) ;
			::CoTaskMemFree( pszName ) ;
			//
			ESLAssert( m_aFileNames.GetLength() == i ) ;
			m_aFileNames.Add( pFileName ) ;
			//
			if ( *pFileName == pwszName )
			{
				return	psiFile ;
			}
		}
		else
		{
			ESLAssert( m_aFileNames.GetLength() == i ) ;
			m_aFileNames.Add( new FileName ) ;
		}
		psiFile->Release() ;
	}
	return	NULL ;
}

// SFileOpener::OpenFlag から IBindCtx を生成
//////////////////////////////////////////////////////////////////////////////
IBindCtx * SWin32ShellFolder::CreateBindCtxWithOpenFlags( long int nOpenFlags )
{
	IBindCtx *	pbc = NULL ;
	if ( !SUCCEEDED( CreateBindCtx( 0, &pbc ) ) )
	{
		return	NULL ;
	}
	BIND_OPTS	bopt ;
	bopt.cbStruct = sizeof(BIND_OPTS) ;
	bopt.grfFlags = 0 ;
	bopt.grfMode = AccessModeFromOpenFlags( nOpenFlags ) ;
	bopt.dwTickCountDeadline = 0 ;
	//
	pbc->SetBindOptions( &bopt ) ;
	//
	return	pbc ;
}

DWORD SWin32ShellFolder::AccessModeFromOpenFlags( long int nOpenFlags )
{
	DWORD	grfMode = 0 ;
	if ( nOpenFlags & SFileOpener::modeWriteFlag )
	{
		if ( nOpenFlags & SFileOpener::modeReadFlag )
		{
			grfMode |= STGM_READWRITE ;
		}
		else
		{
			grfMode |= STGM_WRITE ;
		}
	}
	if ( nOpenFlags & SFileOpener::shareReadFlag )
	{
		if ( nOpenFlags & SFileOpener::shareWriteFlag )
		{
			grfMode |= STGM_SHARE_DENY_NONE ;
		}
		else
		{
			grfMode |= STGM_SHARE_DENY_WRITE ;
		}
	}
	else if ( nOpenFlags & SFileOpener::shareWriteFlag )
	{
		grfMode |= STGM_SHARE_DENY_READ ;
	}
	else
	{
//		grfMode |= STGM_SHARE_EXCLUSIVE ;
	}
	if ( nOpenFlags & SFileOpener::modeCreateFlag )
	{
		grfMode |= STGM_CREATE ;
	}
	return	grfMode ;
}

// ファイルオペレーション取得
//////////////////////////////////////////////////////////////////////////////
IFileOperation * SWin32ShellFolder::GetFileOperation( void )
{
	if ( m_pfOperation == NULL )
	{
		::CoCreateInstance
			( CLSID_FileOperation, NULL, CLSCTX_ALL,
				__uuidof(IFileOperation), (void**) &m_pfOperation ) ;
//		m_pfOperation->SetOperationFlags( FOF_NO_UI ) ;
	}
	return	m_pfOperation ;
}

// フォルダアイテム列挙
//////////////////////////////////////////////////////////////////////////////
void SWin32ShellFolder::EnumFolderItems( void )
{
	if ( !m_fEnumFolders )
	{
		m_aFolders.RemoveAll() ;
		//
		IEnumIDList *	peidl = NULL ;
		if ( SUCCEEDED
			( m_pshFolder->EnumObjects
				( NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN, &peidl ) ) )
		{
			LPITEMIDLIST	pidl = NULL ;
			while ( peidl->Next( 1, &pidl, NULL ) == S_OK )
			{
				m_aFolders.Add( pidl ) ;
			}
			peidl->Release() ;
		}
		m_fEnumFolders = true ;
	}
}

// ファイルアイテム列挙
//////////////////////////////////////////////////////////////////////////////
void SWin32ShellFolder::EnumFileItems( void )
{
	if ( !m_fEnumFiles )
	{
		m_aFiles.RemoveAll() ;
		//
		IEnumIDList *	peidl = NULL ;
		if ( SUCCEEDED
			( m_pshFolder->EnumObjects
				( NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN
						| SHCONTF_STORAGE | SHCONTF_SHAREABLE, &peidl ) ) )
		{
			LPITEMIDLIST	pidl = NULL ;
			while ( peidl->Next( 1, &pidl, NULL ) == S_OK )
			{
				m_aFiles.Add( pidl ) ;
			}
			peidl->Release() ;
		}
		m_fEnumFiles = true ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// デバイスへ転送するストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SSystem::SWin32PortableDevice::TransferToDevice, SWin32StreamFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::TransferToDevice::TransferToDevice
		( SWin32PortableDevice * pDev, IStream * pStream )
	: SWin32StreamFile( pStream, false ),
		m_refDevice( pDev ),
		m_idParent( pDev->GetCurrentContentsParent() ),
		m_ppddsStream( NULL )
{
	if ( SUCCEEDED
		( pStream->QueryInterface
			( __uuidof(IPortableDeviceDataStream),
							(void**) &m_ppddsStream ) ) )
	{
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::TransferToDevice::~TransferToDevice( void )
{
	if ( m_ppddsStream != NULL )
	{
		Commit() ;
	}
}

// コミット
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::TransferToDevice::Commit( void )
{
	if ( m_ppddsStream != NULL )
	{
		m_ppddsStream->Commit( STGC_DEFAULT ) ;
		//
		PWSTR	strNewObjID = NULL ;
		if ( SUCCEEDED( m_ppddsStream->GetObjectID( &strNewObjID ) ) )
		{
			SWin32PortableDevice *	pDev = m_refDevice ;
			if ( pDev != NULL )
			{
				ContentInfo *	pContent = pDev->NewContentInfo( strNewObjID ) ;
				if ( pContent != NULL )
				{
					FolderContents *	pContents ;
					pDev->LockContentsList() ;
					pContents = pDev->m_ssoaFolders.GetAs( m_idParent ) ;
					if ( pContents != NULL )
					{
						pContents->Add( pContent ) ;
					}
					else
					{
						delete	pContent ;
					}
					pDev->UnlockContentsList() ;
				}
			}
			CoTaskMemFree( strNewObjID ) ;
		}
		m_refDevice = NULL ;
		//
		m_ppddsStream->Release() ;
		m_ppddsStream = NULL ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// バッファ遅延転送ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SSystem::SWin32PortableDevice::BufferedTransferToDevice, SSmartBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::BufferedTransferToDevice::BufferedTransferToDevice
		( SWin32PortableDevice * pDev, const wchar_t * pwszName )
	: m_refDevice( pDev ), m_strFileName( pwszName )
{
	m_strFolderPath = pDev->GetCurrentFolderPath() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::BufferedTransferToDevice::~BufferedTransferToDevice( void )
{
	Commit() ;
}

// コミット
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::BufferedTransferToDevice::Commit( void )
{
	SWin32PortableDevice *	pDev = m_refDevice ;
	if ( pDev == NULL )
	{
		return	errFailed ;
	}
	pDev->SetCurrentFolder( m_strFolderPath ) ;
	//
	if ( pDev->FindContentIndex( m_strFileName ) >= 0 )
	{
		pDev->DeleteContent( m_strFileName ) ;
	}
	SFileInterface *
		pDstFile = pDev->NewFileContent( m_strFileName, GetLength() ) ;
	if ( pDstFile == NULL )
	{
		return	errFailed ;
	}
	WriteToStream( *pDstFile ) ;
	delete	pDstFile ;
	m_refDevice = NULL ;
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ポータブルデバイス (USB/MTP)
//////////////////////////////////////////////////////////////////////////////

const GUID	SWin32PortableDevice::CLSID_PortableDeviceManager =
	{ 0x0af10cec, 0x2ecd, 0x4b92, { 0x95,0x81, 0x34,0xf6,0xae,0x06,0x37,0xf3 } } ;
const GUID	SWin32PortableDevice::CLSID_PortableDeviceFTM =
	{ 0xf7c0039a, 0x4762, 0x488a, { 0xb4,0xb3, 0x76,0x0e,0xf9,0xa1,0xba,0x9b } } ;
const GUID	SWin32PortableDevice::CLSID_PortableDeviceValues =
	{ 0x0c15d503, 0xd017, 0x47ce, { 0x90,0x16, 0x7b,0x3f,0x97,0x87,0x21,0xcc } } ;
const GUID	SWin32PortableDevice::CLSID_PortableDeviceKeyCollection =
	{ 0xde2d022d, 0x2480, 0x43be, { 0x97,0xf0, 0xd1,0xfa,0x2c,0xf9,0x8f,0x4f } } ;
const GUID	SWin32PortableDevice::CLSID_PortableDevicePropVariantCollection =
	{ 0x08a99e2f, 0x6d6d, 0x4b80, { 0xaf,0x5a, 0xba,0xf2,0xbc,0xbe,0x4c,0xb9 } } ;

const PROPERTYKEY	SWin32PortableDevice::WPD_CLIENT_DESIRED_ACCESS =
	{ { 0x204D9F0C, 0x2292, 0x4080, { 0x9F,0x42,0x40,0x66,0x4E,0x70,0xF8,0x59 } }, 9 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_CONTENT_TYPE =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 7 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_PARENT_ID =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 3 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_NAME =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 4 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_ORIGINAL_FILE_NAME =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 12 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_SIZE =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 11 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_DATE_CREATED =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 18 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_DATE_MODIFIED =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 19 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_OBJECT_DATE_AUTHORED =
	{ { 0xEF6B490D, 0x5CD8, 0x437A, { 0xAF,0xFC,0xDA,0x8B,0x60,0xEE,0x4A,0x3C } }, 20 } ;
const PROPERTYKEY	SWin32PortableDevice::WPD_RESOURCE_DEFAULT =
	{ { 0xE81E79BE, 0x34F0, 0x41BF, { 0xB5,0x3F,0xF1,0xA0,0x6A,0xE8,0x78,0x42 } }, 0 } ;
const GUID	SWin32PortableDevice::WPD_FUNCTIONAL_CATEGORY_STORAGE =
	{ 0x23F05BBC, 0x15DE, 0x4C2A, { 0xA5,0x5B,0xA9,0xAF,0x5C,0xE4,0x12,0xEF } } ;
const GUID	SWin32PortableDevice::WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT =
	{ 0x99ED0160, 0x17FF, 0x4C44, { 0x9D,0x98,0x1D,0x7A,0x6F,0x94,0x19,0x21 } } ;
const GUID	SWin32PortableDevice::WPD_CONTENT_TYPE_FOLDER =
	{ 0x27E2E392, 0xA111, 0x48E0, { 0xAB,0x0C,0xE1,0x77,0x05,0xA0,0x5F,0x85 } } ;
const GUID	SWin32PortableDevice::WPD_CONTENT_TYPE_UNSPECIFIED =
	{ 0x28D8D31E, 0x249C, 0x454E, { 0xAA,0xBC,0x34,0x88,0x31,0x68,0xE6,0x34 } } ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SWin32PortableDevice, SFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::SWin32PortableDevice( void )
{
	m_pDevManager = NULL ;
	m_pDevice = NULL ;
	m_pContent = NULL ;
	m_pProp = NULL ;
	m_fReadOnly = false ;
	m_pContents = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::~SWin32PortableDevice( void )
{
	Release() ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::Release( void )
{
	m_csSync.Lock() ;
	m_aDevInfos.RemoveAll() ;
	m_ssoaFolderIDs.RemoveAll() ;
	m_ssoaFolders.RemoveAll() ;
	m_fReadOnly = false ;
	m_idDevice.FreeArray() ;
	m_idParent.FreeArray() ;
	m_strCurrentPath.FreeArray() ;
	m_pContents = NULL ;
	m_csSync.Unlock() ;
	//
	if ( m_pProp != NULL )
	{
		m_pProp->Release() ;
		m_pProp = NULL ;
	}
	if ( m_pContent != NULL )
	{
		m_pContent->Release() ;
		m_pContent = NULL ;
	}
	if ( m_pDevice != NULL )
	{
		m_pDevice->Release() ;
		m_pDevice = NULL ;
	}
	if ( m_pDevManager != NULL )
	{
		m_pDevManager->Release() ;
		m_pDevManager = NULL ;
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32PortableDevice::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SString	strFilePath = pszFilePath ;
	SString	strDirPath = strFilePath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( (nOpenFlags & modeCreateFlag)
		&& (nOpenFlags & modeCreateDirFlag) )
	{
		if ( SetCurrentFolder( strDirPath, true ) )
		{
			return	NULL ;
		}
	}
	else if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	NULL ;
	}
	return	OpenFileContent
				( SString(strFilePath.GetFileNamePart()), nOpenFlags ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SWin32PortableDevice::IsExisting( const wchar_t * pszFilePath )
{
	SString	strFilePath = pszFilePath ;
	SString	strDirPath = strFilePath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	errFailed ;
	}
	return	(FindContentIndex( SString(strFilePath.GetFileNamePart()) ) >= 0) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	SString	strFilePath = pszFilePath ;
	SString	strDirPath = strFilePath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	errFailed ;
	}
	const ContentInfo *	pci =
		GetContentAs( SString(strFilePath.GetFileNamePart()) ) ;
	if ( pci == NULL )
	{
		return	errFailed ;
	}
	state.bitFields = fieldAttributes | fieldFileSize ;
	state.bitAttributes = 0 ;
	if ( pci->m_flags & typeFolder )
	{
		state.bitAttributes |= attrDirectory ;
	}
	state.nFileSize = pci->m_size ;
	//
	if ( pci->m_flags & flagCreatedDate )
	{
		state.bitFields |= fieldCreatedTime ;
		state.dtCreated = pci->m_dtCreated ;
	}
	if ( pci->m_flags & flagModifiedDate )
	{
		state.bitFields |= fieldModifiedTime ;
		state.dtModified = pci->m_dtModified ;
	}
	return	errSuccess ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	listFiles.RemoveAll() ;
	//
	if ( pszDirPath != NULL )
	{
		if ( SetCurrentFolder( pszDirPath, false ) )
		{
			return ;
		}
		m_csSync.Lock() ;
		ESLAssert( m_pContents != NULL ) ;
		if ( m_pContents != NULL )
		for ( size_t i = 0; i < m_pContents->GetLength(); i ++ )
		{
			ContentInfo *	pci = m_pContents->GetAt( i ) ;
			if ( (pci != NULL)
				&& !(pci->m_flags & typeFolder) )
			{
				if ( pci->m_filename.IsEmpty() )
				{
					listFiles.Add( new SString(pci->m_name) ) ;
				}
				else
				{
					listFiles.Add( new SString(pci->m_filename) ) ;
				}
			}
		}
		m_csSync.Unlock() ;
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	listDirs.RemoveAll() ;
	//
	if ( pszDirPath != NULL )
	{
		if ( SetCurrentFolder( pszDirPath, false ) )
		{
			return ;
		}
		m_csSync.Lock() ;
		ESLAssert( m_pContents != NULL ) ;
		if ( m_pContents != NULL )
		for ( size_t i = 0; i < m_pContents->GetLength(); i ++ )
		{
			ContentInfo *	pci = m_pContents->GetAt( i ) ;
			if ( (pci != NULL)
				&& (pci->m_flags & typeFolder) )
			{
				if ( pci->m_filename.IsEmpty() )
				{
					listDirs.Add( new SString(pci->m_name) ) ;
				}
				else
				{
					listDirs.Add( new SString(pci->m_filename) ) ;
				}
			}
		}
		m_csSync.Unlock() ;
	}
	else
	{
		for ( size_t i = 0; i < m_aDevInfos.GetLength(); i ++ )
		{
			DeviceInfo *	pdi = m_aDevInfos.GetAt( i ) ;
			if ( pdi != NULL )
			{
				if ( pdi->m_FriendlyName.IsEmpty() )
				{
					listDirs.Add( new SString(pdi->m_Description) ) ;
				}
				else
				{
					listDirs.Add( new SString(pdi->m_FriendlyName) ) ;
				}
			}
		}
	}
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::RemoveSubFile( const wchar_t * pszFilePath )
{
	SString	strFilePath = pszFilePath ;
	SString	strDirPath = strFilePath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	errFailed ;
	}
	SString	strName = strFilePath.GetFileNamePart() ;
	const ContentInfo *	pci = GetContentAs( strName ) ;
	if ( pci == NULL )
	{
		return	errFailed ;
	}
	if ( pci->m_flags & typeFolder )
	{
		return	errFailed ;
	}
	return	DeleteContent( strName ) ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	SString	strPath = pszPath ;
	SString	strDirPath = strPath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	errFailed ;
	}
	return	CreateFolder( SString( strPath.GetFileNamePart() ) ) ;
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::RemoveSubDirectory( const wchar_t * pszPath )
{
	SString	strPath = pszPath ;
	SString	strDirPath = strPath.GetFileDirectoryPart() ;
	strDirPath.ChopRight( 1 ) ;
	//
	if ( SetCurrentFolder( strDirPath, false ) )
	{
		return	errFailed ;
	}
	SString	strName = strPath.GetFileNamePart() ;
	const ContentInfo *	pci = GetContentAs( strName ) ;
	if ( pci == NULL )
	{
		return	errFailed ;
	}
	if ( !(pci->m_flags & typeFolder) )
	{
		return	errFailed ;
	}
	return	DeleteContent( strName ) ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	SString	strOldPath = pszOldPath ;
	SString	strNewPath = pszNewPath ;
	SString	strSrcDir = strOldPath.GetFileDirectoryPart() ;
	SString	strDstDir = strNewPath.GetFileDirectoryPart() ;
	strSrcDir.ChopRight( 1 ) ;
	strDstDir.ChopRight( 1 ) ;
	//
	if ( strSrcDir != strDstDir )
	{
		return	errFailed ;
	}
	if ( SetCurrentFolder( strSrcDir, false ) )
	{
		return	errFailed ;
	}
	SString	strOldName = strOldPath.GetFileNamePart() ;
	SString	strNewName = strNewPath.GetFileNamePart() ;
	return	RenameFile( strOldName, strNewName ) ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::EnumerateDevices( void )
{
	HRESULT	hr ;
	if ( m_pDevManager == NULL )
	{
		hr = CoCreateInstance
				( CLSID_PortableDeviceManager,
					NULL, CLSCTX_INPROC_SERVER,
					__uuidof(IPortableDeviceManager), (void**) &m_pDevManager ) ;
		if ( !SUCCEEDED( hr ) )
		{
			return	errFailed ;
		}
	}
	//
	// デバイス列挙
	//
	DWORD	dwDevCount = 0 ;
	hr = m_pDevManager->GetDevices( NULL, &dwDevCount ) ;
	if ( !SUCCEEDED( hr ) )
	{
		return	errFailed ;
	}
	SArray<PWSTR>	aDevIds ;
	PWSTR *			pDevIds = aDevIds.GetArray( (size_t) dwDevCount ) ;
	hr = m_pDevManager->GetDevices( pDevIds, &dwDevCount ) ;
	if ( !SUCCEEDED( hr ) )
	{
		aDevIds.FinishArray() ;
		return	errFailed ;
	}
	aDevIds.FinishArray() ;
	//
	// デバイス情報取得
	//
	m_aDevInfos.RemoveAll() ;
	for ( size_t i = 0; i < dwDevCount; i ++ )
	{
		DeviceInfo *	pDevInf = new DeviceInfo ;
		m_aDevInfos.Add( pDevInf ) ;
		//
		pDevInf->m_DeviceId = pDevIds[i] ;
		//
		// 名前
		//
		DWORD	nName = 0 ;
		if ( SUCCEEDED
			( m_pDevManager->GetDeviceFriendlyName
							( pDevIds[i], NULL, &nName ) ) )
		{
			SArray<WCHAR>	bufName ;
			WCHAR *			pwszName = bufName.GetArray( (size_t) nName ) ;
			if ( SUCCEEDED
				( m_pDevManager->GetDeviceFriendlyName
							( pDevIds[i], pwszName, &nName ) ) )
			{
				pDevInf->m_FriendlyName = pwszName ;
			}
			bufName.FinishArray() ;
		}
		//
		// 製造元
		//
		DWORD	nManufacture = 0 ;
		if ( SUCCEEDED
			( m_pDevManager->GetDeviceManufacturer
							( pDevIds[i], NULL, &nManufacture ) ) )
		{
			SArray<WCHAR>	bufManufacture ;
			WCHAR *			pwszManufacture =
								bufManufacture.GetArray( (size_t) nManufacture ) ;
			if ( SUCCEEDED
				( m_pDevManager->GetDeviceManufacturer
						( pDevIds[i], pwszManufacture, &nManufacture ) ) )
			{
				pDevInf->m_Manufacturer = pwszManufacture ;
			}
			bufManufacture.FinishArray() ;
		}
		//
		// 詳細説明
		//
		DWORD	nDescription = 0 ;
		if ( SUCCEEDED
			( m_pDevManager->GetDeviceDescription
							( pDevIds[i], NULL, &nDescription ) ) )
		{
			SArray<WCHAR>	bufDescription ;
			WCHAR *			pwszDescription =
								bufDescription.GetArray( (size_t) nDescription ) ;
			if ( SUCCEEDED
				( m_pDevManager->GetDeviceDescription
						( pDevIds[i], pwszDescription, &nDescription ) ) )
			{
				pDevInf->m_Description = pwszDescription ;
			}
			bufDescription.FinishArray() ;
		}
		//
		CoTaskMemFree( pDevIds[i] ) ;
	}
	return	errSuccess ;
}

// デバイス数取得
//////////////////////////////////////////////////////////////////////////////
size_t SWin32PortableDevice::GetDeviceCount( void ) const
{
	return	m_aDevInfos.GetLength() ;
}

// デバイス情報取得
//////////////////////////////////////////////////////////////////////////////
const SWin32PortableDevice::DeviceInfo *
	SWin32PortableDevice::GetDeviceInfoAt( size_t i ) const
{
	return	m_aDevInfos.GetAt( i ) ;
}

// デバイス検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SWin32PortableDevice::FindDeviceIndex( const wchar_t * pwszName ) const
{
	for ( size_t i = 0; i < m_aDevInfos.GetLength(); i ++ )
	{
		DeviceInfo *	pdi = m_aDevInfos.GetAt( i ) ;
		if ( pdi != NULL )
		{
			if ( pdi->m_FriendlyName.IsEmpty() )
			{
				if ( pdi->m_Description == pwszName )
				{
					return	(ssize_t) i ;
				}
			}
			else
			{
				if ( pdi->m_FriendlyName == pwszName )
				{
					return	(ssize_t) i ;
				}
			}
		}
	}
	return	-1 ;
}

const SWin32PortableDevice::DeviceInfo *
	SWin32PortableDevice::GetDeviceInfoAs( const wchar_t * pwszName ) const
{
	return	m_aDevInfos.GetAt( (size_t) FindDeviceIndex( pwszName ) ) ;
}

// デバイス選択
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::SelectDevice( const DeviceInfo * pDevInfo )
{
	if ( m_pProp != NULL )
	{
		m_pProp->Release() ;
		m_pProp = NULL ;
	}
	if ( m_pContent != NULL )
	{
		m_pContent->Release() ;
		m_pContent = NULL ;
	}
	if ( m_pDevice != NULL )
	{
		m_pDevice->Release() ;
		m_pDevice = NULL ;
	}
	HRESULT	hr ;
	hr = CoCreateInstance
			( CLSID_PortableDeviceFTM,
				NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IPortableDevice), (void**) &m_pDevice ) ;
	if ( !SUCCEEDED( hr ) )
	{
		return	errFailed ;
	}
	IPortableDeviceValues *	ppdvClient = NULL ;
	hr = CoCreateInstance
		( CLSID_PortableDeviceValues,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDeviceValues), (void**) &ppdvClient ) ;
	if ( !SUCCEEDED( hr ) )
	{
		return	errFailed ;
	}
	m_fReadOnly = false ;
	//
	hr = m_pDevice->Open( pDevInfo->m_DeviceId, ppdvClient ) ;
    if ( hr == E_ACCESSDENIED )
    {
        ppdvClient->SetUnsignedIntegerValue
			( WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ ) ;
		//
        hr = m_pDevice->Open( pDevInfo->m_DeviceId, ppdvClient ) ;
		//
		m_fReadOnly = true ;
    }
	ppdvClient->Release() ;
	if ( !SUCCEEDED( hr ) )
	{
		return	errFailed ;
	}
	//
	m_csSync.Lock() ;
	m_ssoaFolderIDs.RemoveAll() ;
	m_ssoaFolders.RemoveAll() ;
	m_idDevice = pDevInfo->m_DeviceId ;
	m_idParent = L"" ;
	m_pContents = NULL ;
	//
	m_strCurrentPath = pDevInfo->m_FriendlyName ;
	if ( pDevInfo->m_FriendlyName.IsEmpty() )
	{
		m_strCurrentPath = pDevInfo->m_Description ;
	}
	m_csSync.Unlock() ;
	//
	if ( !SUCCEEDED( m_pDevice->Content( &m_pContent ) ) )
	{
		return	errFailed ;
	}
	if ( !SUCCEEDED( m_pContent->Properties( &m_pProp ) ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// ルートストレージ列挙
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::EnumerateRootStorages( void )
{
	if ( m_pDevice == NULL )
	{
		return	errFailed ;
	}
	IPortableDeviceCapabilities *			ppdc = NULL ;
	IPortableDevicePropVariantCollection *	ppdpvcFunc = NULL ;
	SError				err = errFailed ;
	FolderContents *	pContents = new FolderContents ;
	do
	{
		if ( !SUCCEEDED( m_pDevice->Capabilities( &ppdc ) ) )
		{
			break ;
		}
		if ( !SUCCEEDED( ppdc->GetFunctionalCategories( &ppdpvcFunc ) ) )
		{
			break ;
		}
		DWORD	nCount = 0 ;
		if ( !SUCCEEDED( ppdpvcFunc->GetCount( &nCount ) ) )
		{
			break ;
		}
		for ( DWORD i = 0; i < nCount; i ++ )
		{
            PROPVARIANT pv = {0};
            if ( SUCCEEDED( ppdpvcFunc->GetAt( i, &pv ) )
				&& (pv.vt == VT_CLSID)
				&& (pv.puuid != NULL)
				&& IsEqualGUID( *pv.puuid, WPD_FUNCTIONAL_CATEGORY_STORAGE ) )
			{
				IPortableDevicePropVariantCollection *	ppdpvc = NULL ;
				if ( SUCCEEDED
					( ppdc->GetFunctionalObjects( *pv.puuid, &ppdpvc ) ) )
				{
					DWORD	nObjCount = 0 ;
					if ( SUCCEEDED( ppdpvc->GetCount( &nObjCount ) ) )
					for ( DWORD j = 0; j < nObjCount; j ++ )
					{
						PROPVARIANT	pvObjID = {0};
						if ( SUCCEEDED( ppdpvc->GetAt( j, &pvObjID ) )
							&& (pvObjID.vt == VT_LPWSTR)
							&& (pvObjID.pwszVal != NULL) )
						{
							ContentInfo *
								pContent = NewContentInfo( pvObjID.pwszVal ) ;
							if ( pContent != NULL )
							{
								pContents->Add( pContent ) ;
							}
						}
						PropVariantClear( &pvObjID ) ;
					}
					ppdpvc->Release() ;
				}
			}
			PropVariantClear( &pv ) ;
		}
		err = errSuccess ;
	}
	while ( false ) ;
	//
	m_csSync.Lock() ;
	m_idParent = L"" ;
	m_ssoaFolders.SetAs( L"", pContents ) ;
	m_pContents = pContents ;
	m_csSync.Unlock() ;
	//
	if ( ppdpvcFunc != NULL )
	{
		ppdpvcFunc->Release() ;
		ppdpvcFunc = NULL ;
	}
	if ( ppdc != NULL )
	{
		ppdc->Release() ;
		ppdc = NULL ;
	}
	return	err ;
}

// コンテンツ列挙
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::EnumerateContents( const wchar_t * pwszParentID )
{
	if ( m_pContent == NULL )
	{
		return	errFailed ;
	}
	IEnumPortableDeviceObjectIDs *	pepdoIDs = NULL ;
	if ( !SUCCEEDED
		( m_pContent->EnumObjects
			( 0, pwszParentID, NULL, &pepdoIDs ) ) )
	{
		return	errFailed ;
	}
	FolderContents *	pContents = new FolderContents ;
	DWORD	nFetched = 0 ;
	PWSTR	pstrID = NULL ;
	while ( pepdoIDs->Next( 1, &pstrID, &nFetched ) == S_OK )
	{
		ContentInfo *	pContent = NewContentInfo( pstrID ) ;
		if ( pContent != NULL )
		{
			m_csSync.Lock() ;
			pContents->Add( pContent ) ;
			m_csSync.Unlock() ;
		}
		CoTaskMemFree( pstrID ) ;
	}
	pepdoIDs->Release() ;
	//
	m_csSync.Lock() ;
	m_idParent = pwszParentID ;
	m_ssoaFolders.SetAs( pwszParentID, pContents ) ;
	m_pContents = pContents ;
	m_csSync.Unlock() ;
	//
	return	errSuccess ;
}

// 現在のコンテンツリストの親ID
//////////////////////////////////////////////////////////////////////////////
SString SWin32PortableDevice::GetCurrentContentsParent( void ) const
{
	SString	strID ;
	m_csSync.Lock() ;
	strID = m_idParent ;
	m_csSync.Unlock() ;
	return	strID ;
}

// コンテンツリストの排他処理用
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::LockContentsList( void )
{
	m_csSync.Lock() ;
}

void SWin32PortableDevice::UnlockContentsList( void )
{
	m_csSync.Unlock() ;
}

// 現在のフォルダを移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::SetCurrentFolder
		( const wchar_t * pwszPath, bool fCreateDirectories )
{
	SString	strPath = pwszPath ;
	strPath.Replace( L'/', L'\\' ) ;
	if ( m_strCurrentPath == strPath )
	{
		return	errSuccess ;
	}
	FolderContents *	pContents = NULL ;
	SString *	pstrID = m_ssoaFolderIDs.GetAs( pwszPath ) ;
	if ( pstrID != NULL )
	{
		//
		// 既知のフォルダー
		//
		m_csSync.Lock() ;
		pContents = m_ssoaFolders.GetAs( *pstrID ) ;
		if ( pContents != NULL )
		{
			SetCurrentFolderContents( *pstrID, pwszPath, pContents ) ;
		}
		else
		{
			m_csSync.Unlock() ;
			if ( EnumerateContents( *pstrID ) )
			{
				return	errFailed ;
			}
			m_csSync.Lock() ;
			m_strCurrentPath = pwszPath ;
		}
		m_csSync.Unlock() ;
		return	errSuccess ;
	}
	//
	// デバイス解釈
	//
	size_t	iPath = 0 ;
	SString	strDevName = ParseNextFolderName( pwszPath, iPath ) ;
	const DeviceInfo *	pDevInfo = GetDeviceInfoAs( strDevName ) ;
	if ( pDevInfo == NULL )
	{
		if ( m_pDevManager == NULL )
		{
			SError	err = EnumerateDevices() ;
			if ( err )
			{
				return	err ;
			}
		}
		pDevInfo = GetDeviceInfoAs( strDevName ) ;
		if ( pDevInfo == NULL )
		{
			return	errFailed ;
		}
	}
	if ( m_idDevice != pDevInfo->m_DeviceId )
	{
		SError	err = SelectDevice( pDevInfo ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 既知のパス解釈
	//
	SString	strKnownPath = pDevInfo->m_FriendlyName ;
	if ( strKnownPath.IsEmpty() )
	{
		strKnownPath = pDevInfo->m_Description ;
	}
	SString	strKnownID ;
	SString	strName = ParseNextFolderName( pwszPath, iPath ) ;
	if ( strName.IsEmpty() )
	{
		return	EnumerateRootStorages() ;
	}
	for ( ; ; )
	{
		SString	strNextPath = strKnownPath + L"\\" + strName ;
		m_csSync.Lock() ;
		pstrID = m_ssoaFolderIDs.GetAs( strNextPath ) ;
		if ( pstrID == NULL )
		{
			m_csSync.Unlock() ;
			break ;
		}
		pContents = m_ssoaFolders.GetAs( *pstrID ) ;
		m_csSync.Unlock() ;
		//
		strKnownPath = strNextPath ;
		strKnownID = *pstrID ;
		strName = ParseNextFolderName( pwszPath, iPath ) ;
		if ( strName.IsEmpty() )
		{
			if ( pContents != NULL )
			{
				SetCurrentFolderContents( strKnownID, strKnownPath, pContents ) ;
			}
			else
			{
				if ( EnumerateContents( strKnownID ) )
				{
					return	errFailed ;
				}
				m_csSync.Lock() ;
				m_strCurrentPath = strKnownPath ;
				m_csSync.Unlock() ;
			}
			return	errSuccess ;
		}
	}
	m_csSync.Lock() ;
	if ( pContents != NULL )
	{
		SetCurrentFolderContents( strKnownID, strKnownPath, pContents ) ;
	}
	else
	{
		m_csSync.Unlock() ;
		if ( strKnownID.IsEmpty() )
		{
			if ( EnumerateRootStorages() )
			{
				return	errFailed ;
			}
		}
		else
		{
			if ( EnumerateContents( strKnownID ) )
			{
				return	errFailed ;
			}
		}
		m_csSync.Lock() ;
		m_strCurrentPath = strKnownPath ;
	}
	m_csSync.Unlock() ;
	//
	// 順次フォルダ移動
	//
	for ( ; ; )
	{
		SError	err = DescendFolder( strName ) ;
		if ( err )
		{
			if ( !fCreateDirectories
				|| (FindContentIndex( strName ) >= 0) )
			{
				return	err ;
			}
			err = CreateFolder( strName ) ;
			if ( err )
			{
				return	err ;
			}
			err = DescendFolder( strName ) ;
			if ( err )
			{
				return	err ;
			}
		}
		strName = ParseNextFolderName( pwszPath, iPath ) ;
		if ( strName.IsEmpty() )
		{
			break ;
		}
	}
	return	errSuccess ;
}

SString SWin32PortableDevice::ParseNextFolderName
			( const wchar_t * pwszPath, size_t& iNext )
{
	size_t	iLast = iNext ;
	size_t	i = iLast ;
	if ( (pwszPath[i] == L'\\') || (pwszPath[i] == L'/') )
	{
		iLast = ++ i ;
	}
	while ( pwszPath[i] )
	{
		if ( pwszPath[i] == L'\\' )
		{
			if ( (pwszPath[i + 1] == L'\\')
					|| (pwszPath[i + 1] == 0) )
			{
				i ++ ;
			}
			break ;
		}
		if ( pwszPath[i] == L'/' )
		{
			break ;
		}
		i ++ ;
	}
	iNext = i ;
	return	SString( pwszPath + iLast, (ssize_t) (i - iLast) ) ;
}

// 現在のフォルダパス取得
//////////////////////////////////////////////////////////////////////////////
SString SWin32PortableDevice::GetCurrentFolderPath( void ) const
{
	SString	strPath ;
	m_csSync.Lock() ;
	strPath = m_strCurrentPath ;
	m_csSync.Unlock() ;
	return	strPath ;
}

// 現在のフォルダコンテンツリストを設定
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::SetCurrentFolderContents
	( const wchar_t * pwszParentID,
		const wchar_t * pwszFolderPath,
		SWin32PortableDevice::FolderContents * pContents )
{
	m_csSync.Lock() ;
	m_idParent = pwszParentID ;
	m_strCurrentPath = pwszFolderPath ;
	m_pContents = pContents ;
	m_csSync.Unlock() ;
}

// 下層フォルダへ移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::DescendFolder( const wchar_t * pwszName )
{
	const ContentInfo *	pContent = GetContentAs( pwszName ) ;
	if ( pContent == NULL )
	{
		return	errFailed ;
	}
	if ( !(pContent->m_flags & typeFolder) )
	{
		return	errFailed ;
	}
	if ( EnumerateContents( pContent->m_id ) )
	{
		return	errFailed ;
	}
	m_csSync.Lock() ;
	m_strCurrentPath += L"\\" ;
	m_strCurrentPath += pwszName ;
	if ( m_ssoaFolderIDs.GetAs( m_strCurrentPath ) == NULL )
	{
		m_ssoaFolderIDs.Add
			( m_strCurrentPath, new SString( m_idParent ) ) ;
	}
	m_csSync.Unlock() ;
	return	errSuccess ;
}

// 上層フォルダへ移動
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::AscendFolder( void )
{
	SString	strParentPath ;
	size_t	iLastDir = 0 ;
	if ( m_strCurrentPath.GetLastAt( 0 ) == L'\\' )
	{
		iLastDir ++ ;
	}
	while ( iLastDir < m_strCurrentPath.GetLength() )
	{
		wchar_t	wch = m_strCurrentPath.GetLastAt( iLastDir ++ ) ;
		if ( (wch == L'\\') || (wch == L'/') )
		{
			break ;
		}
	}
	if ( iLastDir >= m_strCurrentPath.GetLength() )
	{
		return	errFailed ;
	}
	strParentPath =
		m_strCurrentPath.Left( m_strCurrentPath.GetLength() - iLastDir ) ;
	if ( strParentPath.Find( L'\\' ) >= 0 )
	{
		SString *	pstrID = m_ssoaFolderIDs.GetAs( strParentPath ) ;
		if ( pstrID == NULL )
		{
			m_idParent = L"" ;
			m_strCurrentPath = strParentPath ;
			return	errFailed ;
		}
		m_csSync.Lock() ;
		FolderContents *	pContents = m_ssoaFolders.GetAs( *pstrID ) ;
		if ( pContents != NULL )
		{
			SetCurrentFolderContents
				( *pstrID, strParentPath, pContents ) ;
		}
		else
		{
			m_csSync.Unlock() ;
			if ( EnumerateContents( *pstrID ) )
			{
				return	errFailed ;
			}
			m_csSync.Lock() ;
			m_strCurrentPath = strParentPath ;
		}
		m_csSync.Unlock() ;
		return	errSuccess ;
	}
	else
	{
		m_csSync.Lock() ;
		FolderContents *	pContents = m_ssoaFolders.GetAs( L"" ) ;
		if ( pContents != NULL )
		{
			SetCurrentFolderContents
				( L"", strParentPath, pContents ) ;
			return	errSuccess ;
		}
		else
		{
			m_idParent = L"" ;
			m_strCurrentPath = strParentPath ;
			return	EnumerateRootStorages() ;
		}
	}
}

// フォルダ作成
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::CreateFolder( const wchar_t * pwszName )
{
	if ( (m_pDevice == NULL) || m_idParent.IsEmpty() || (m_pContents == NULL) )
	{
		return	errFailed ;
	}
	IPortableDeviceValues *	ppdvProp = NULL ;
    if ( !SUCCEEDED( CoCreateInstance
		( CLSID_PortableDeviceValues,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDeviceValues), (void**) &ppdvProp ) ) )
	{
		return	errFailed ;
	}
	ppdvProp->SetStringValue( WPD_OBJECT_PARENT_ID, m_idParent ) ;
	ppdvProp->SetStringValue( WPD_OBJECT_NAME, pwszName ) ;
	ppdvProp->SetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, pwszName ) ;
	ppdvProp->SetGuidValue
			( WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER ) ;
	//
	SError	err = errFailed ;
	PWSTR	strNewObjID = NULL ;
	if ( SUCCEEDED
		( m_pContent->CreateObjectWithPropertiesOnly( ppdvProp, &strNewObjID ) ) )
	{
		ContentInfo *	pContent = NewContentInfo( strNewObjID ) ;
		if ( pContent != NULL )
		{
			m_csSync.Lock() ;
			ESLAssert( m_pContents != NULL ) ;
			if ( m_pContents != NULL )
			{
				m_pContents->Add( pContent ) ;
			}
			else
			{
				delete	pContent ;
			}
			m_csSync.Unlock() ;
		}
		CoTaskMemFree( strNewObjID ) ;
		err = errSuccess ;
	}
	ppdvProp->Release() ;
	//
	return	err ;
}

// フォルダ作成
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::DeleteContent( const wchar_t * pwszName )
{
	ssize_t	iContent = FindContentIndex( pwszName ) ;
	if ( iContent < 0 )
	{
		return	errFailed ;
	}
	ESLAssert( m_pContents != NULL ) ;
	ContentInfo *	pContent = m_pContents->GetAt( (size_t) iContent ) ;
	if ( pContent == NULL )
	{
		return	errFailed ;
	}
	SString	idContent = pContent->m_id ;
	if ( m_pContent == NULL )
	{
		return	errFailed ;
	}
	IPortableDevicePropVariantCollection *	ppdpvc = NULL ;
	if ( !SUCCEEDED( CoCreateInstance
		( CLSID_PortableDevicePropVariantCollection,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDevicePropVariantCollection),
			(void**) &ppdpvc ) ) )
	{
		return	errFailed ;
	}
	SError		err = errFailed ;
	PROPVARIANT	pv = { 0 } ;
	pv.vt = VT_LPWSTR ;
	if ( SUCCEEDED( SHStrDupW( idContent, &pv.pwszVal ) ) )
	{
		ppdpvc->Add( &pv ) ;
		if ( SUCCEEDED( m_pContent->Delete
			( PORTABLE_DEVICE_DELETE_NO_RECURSION, ppdpvc, NULL ) ) )
		{
			m_csSync.Lock() ;
			ESLAssert( m_pContents != NULL ) ;
			FolderContents *	pFolder = m_ssoaFolders.GetAs( idContent ) ;
			ESLAssert( pFolder != m_pContents ) ;
			m_ssoaFolders.RemoveAs( idContent ) ;
			if ( pFolder == m_pContents )
			{
				m_pContents = NULL ;
				m_idParent = L"" ;
			}
			else if ( m_pContents != NULL )
			{
				m_pContents->RemoveAt( (size_t) iContent ) ;
			}
			m_csSync.Unlock() ;
			err = errSuccess ;
		}
		PropVariantClear( &pv ) ;
	}
	ppdpvc->Release() ;
	//
	return	err ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface *
	SWin32PortableDevice::OpenFileContent
		( const wchar_t * pwszName, long int nOpenFlags )
{
	if ( m_pContent == NULL )
	{
		return	NULL ;
	}
	if ( nOpenFlags & SFileOpener::modeCreateFlag )
	{
		return	new BufferedTransferToDevice( this, pwszName ) ;
	}
	const ContentInfo *	pContent = GetContentAs( pwszName ) ;
	if ( pContent == NULL )
	{
		return	NULL ;
	}
	IPortableDeviceResources *	ppdrRsrc = NULL ;
	if ( !SUCCEEDED( m_pContent->Transfer( &ppdrRsrc ) ) )
	{
		return	NULL ;
	}
	DWORD		dwMode = STGM_READ ;
	if ( nOpenFlags & SFileOpener::modeWriteFlag )
	{
		if ( nOpenFlags & SFileOpener::modeReadFlag )
		{
			dwMode = STGM_READWRITE ;
		}
		else
		{
			dwMode = STGM_WRITE ;
		}
	}
	IStream *	pStream = NULL ;
	DWORD		nTransferSizeBytes ;
	if ( !SUCCEEDED( ppdrRsrc->GetStream
		( pContent->m_id, WPD_RESOURCE_DEFAULT,
			dwMode, &nTransferSizeBytes, &pStream ) ) )
	{
		ppdrRsrc->Release() ;
		return	NULL ;
	}
	ppdrRsrc->Release() ;
	return	new SWin32StreamFile
				( pStream, false, (int64_t) pContent->m_size ) ;
}

// ファイル新規作成
//////////////////////////////////////////////////////////////////////////////
SFileInterface *
	SWin32PortableDevice::NewFileContent
			( const wchar_t * pwszName, uint64_t nFileBytes )
{
	if ( (m_pDevice == NULL) || m_idParent.IsEmpty() )
	{
		return	NULL ;
	}
	IPortableDeviceValues *	ppdvProp = NULL ;
    if ( !SUCCEEDED( CoCreateInstance
		( CLSID_PortableDeviceValues,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDeviceValues), (void**) &ppdvProp ) ) )
	{
		return	NULL ;
	}
	ppdvProp->SetStringValue( WPD_OBJECT_PARENT_ID, m_idParent ) ;
	ppdvProp->SetStringValue( WPD_OBJECT_NAME, pwszName ) ;
	ppdvProp->SetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, pwszName ) ;
	ppdvProp->SetUnsignedLargeIntegerValue( WPD_OBJECT_SIZE, nFileBytes ) ;
//	ppdvProp->SetGuidValue
//			( WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_UNSPECIFIED ) ;
	//
	TransferToDevice *	pFile = NULL ;
	IStream *	pStream = NULL ;
	DWORD		nOptWriteBufSize = 0 ;
	if ( SUCCEEDED
		( m_pContent->CreateObjectWithPropertiesAndData
			( ppdvProp, &pStream, &nOptWriteBufSize, NULL ) ) )
	{
		pFile = new TransferToDevice( this, pStream ) ;
	}
	ppdvProp->Release() ;
	//
	return	pFile ;
}

// ファイル名変更
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::RenameFile
	( const wchar_t * pwszOldName, const wchar_t * pwszNewName )
{
	if ( (m_pProp == NULL) || m_idParent.IsEmpty() )
	{
		return	errFailed ;
	}
	ContentInfo *	pContent = (ContentInfo*) GetContentAs( pwszOldName ) ;
	if ( pContent == NULL )
	{
		return	errFailed ;
	}
	IPortableDeviceValues *	ppdvProp = NULL ;
    if ( !SUCCEEDED( CoCreateInstance
		( CLSID_PortableDeviceValues,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDeviceValues), (void**) &ppdvProp ) ) )
	{
		return	errFailed ;
	}
	ppdvProp->SetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, pwszNewName ) ;
	//
	SError					err = errFailed ;
	IPortableDeviceValues *	ppdvResult = NULL ;
	if ( SUCCEEDED
		( m_pProp->SetValues
			( pContent->m_id, ppdvProp, &ppdvResult ) ) )
	{
		pContent->m_filename = pwszNewName ;
		//
		ppdvResult->Release() ;
		err = errSuccess ;
	}
	ppdvProp->Release() ;
	//
	return	err ;
}

// ファイル時刻変更
//////////////////////////////////////////////////////////////////////////////
SError SWin32PortableDevice::SetFileTime
	( const wchar_t * pwszName,
		const DATE_TIME * pdtCreated,
		const DATE_TIME * pdtModified,
		const DATE_TIME * pdtAuthored )
{
	if ( (m_pProp == NULL) || m_idParent.IsEmpty() )
	{
		return	errFailed ;
	}
	ContentInfo *	pContent = (ContentInfo*) GetContentAs( pwszName ) ;
	if ( pContent == NULL )
	{
		return	errFailed ;
	}
	IPortableDeviceValues *	ppdvProp = NULL ;
    if ( !SUCCEEDED( CoCreateInstance
		( CLSID_PortableDeviceValues,
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IPortableDeviceValues), (void**) &ppdvProp ) ) )
	{
		return	errFailed ;
	}
	if ( pdtCreated != NULL )
	{
		PROPVARIANT	pv = { 0 } ;
		PropVariantFromDate( pv, *pdtCreated ) ;
		ppdvProp->SetValue( WPD_OBJECT_DATE_CREATED, &pv ) ;
	}
	if ( pdtModified != NULL )
	{
		PROPVARIANT	pv = { 0 } ;
		PropVariantFromDate( pv, *pdtModified ) ;
		ppdvProp->SetValue( WPD_OBJECT_DATE_MODIFIED, &pv ) ;
	}
	if ( pdtAuthored != NULL )
	{
		PROPVARIANT	pv = { 0 } ;
		PropVariantFromDate( pv, *pdtAuthored ) ;
		ppdvProp->SetValue( WPD_OBJECT_DATE_AUTHORED, &pv ) ;
	}
	SError					err = errFailed ;
	IPortableDeviceValues *	ppdvResult = NULL ;
	if ( SUCCEEDED
		( m_pProp->SetValues
			( pContent->m_id, ppdvProp, &ppdvResult ) ) )
	{
		if ( pdtCreated != NULL )
		{
			pContent->m_dtCreated = *pdtCreated ;
		}
		if ( pdtModified != NULL )
		{
			pContent->m_dtModified = *pdtModified ;
		}
		if ( pdtAuthored != NULL )
		{
			pContent->m_dtAuthored = *pdtAuthored ;
		}
		ppdvResult->Release() ;
		err = errSuccess ;
	}
	ppdvProp->Release() ;
	//
	return	err ;
}

// アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SWin32PortableDevice::GetContentCount( void ) const
{
	size_t	nCount = 0 ;
	m_csSync.Lock() ;
	if ( m_pContents != NULL )
	{
		nCount = m_pContents->GetLength() ;
	}
	m_csSync.Unlock() ;
	return	nCount ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
const SWin32PortableDevice::ContentInfo *
	SWin32PortableDevice::GetContentInfo( size_t i ) const
{
	const ContentInfo *	pContent = NULL ;
	m_csSync.Lock() ;
	if ( m_pContents != NULL )
	{
		pContent = m_pContents->GetAt( i ) ;
	}
	m_csSync.Unlock() ;
	return	pContent ;
}

// アイテム検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SWin32PortableDevice::FindContentIndex( const wchar_t * pwszName ) const
{
	m_csSync.Lock() ;
	if ( m_pContents != NULL )
	for ( size_t i = 0; i < m_pContents->GetLength(); i ++ )
	{
		ContentInfo *	pci = m_pContents->GetAt( i ) ;
		if ( pci != NULL )
		{
			if ( !pci->m_filename.IsEmpty() )
			{
				if ( pci->m_filename.CompareNoCase( pwszName ) == 0 )
				{
					m_csSync.Unlock() ;
					return	(ssize_t) i ;
				}
			}
			else if ( pci->m_name.CompareNoCase( pwszName ) == 0 )
			{
				m_csSync.Unlock() ;
				return	(ssize_t) i ;
			}
		}
	}
	m_csSync.Unlock() ;
	return	-1 ;
}

const SWin32PortableDevice::ContentInfo *
	SWin32PortableDevice::GetContentAs( const wchar_t * pwszName ) const
{
	const ContentInfo *	pContent = NULL ;
	m_csSync.Lock() ;
	if ( m_pContents != NULL )
	{
		pContent =
			m_pContents->GetAt( (size_t) FindContentIndex( pwszName ) ) ;
	}
	m_csSync.Unlock() ;
	return	pContent ;
}

// アイテム情報取得
//////////////////////////////////////////////////////////////////////////////
SWin32PortableDevice::ContentInfo *
	SWin32PortableDevice::NewContentInfo( const wchar_t * pwszID ) const
{
	if ( m_pProp == NULL )
	{
		return	NULL ;
	}
	ContentInfo *	pContent = NULL ;
	IPortableDeviceKeyCollection *	pdkcKeys = NULL ;
	IPortableDeviceValues *			ppdvValues = NULL ;
	do
	{
		if ( !SUCCEEDED( CoCreateInstance
			( CLSID_PortableDeviceKeyCollection,
				NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IPortableDeviceKeyCollection),
				(void**) &pdkcKeys ) ) )
		{
			break ;
		}
        pdkcKeys->Add( WPD_OBJECT_NAME ) ;
        pdkcKeys->Add( WPD_OBJECT_CONTENT_TYPE ) ;
		//
		if ( !SUCCEEDED
			( m_pProp->GetValues( pwszID, pdkcKeys, &ppdvValues ) ) )
		{
			break ;
		}
		//
		pContent = new ContentInfo ;
		pContent->m_flags = 0 ;
		pContent->m_id = pwszID ;
		pContent->m_size = 0 ;
		//
		PWSTR	strName = NULL ;
		if ( SUCCEEDED( ppdvValues->GetStringValue( WPD_OBJECT_NAME, &strName ) ) )
		{
			pContent->m_name = strName ;
			CoTaskMemFree( strName ) ;
		}
		GUID	guidType = GUID_NULL ;
		if ( SUCCEEDED
			( ppdvValues->GetGuidValue
				( WPD_OBJECT_CONTENT_TYPE, &guidType ) ) )
		{
			if ( IsEqualGUID( guidType, WPD_CONTENT_TYPE_FOLDER ) )
			{
				pContent->m_flags |= typeFolder ;
			}
			else if ( IsEqualGUID( guidType, WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT ) )
			{
				pContent->m_flags |= typeFolder | typeStorage ;
				break ;
			}
		}
		//
		GetContentStringProperty
			( pContent->m_filename, pwszID, WPD_OBJECT_ORIGINAL_FILE_NAME ) ;
		GetContentULargeIntegerProperty
			( pContent->m_size, pwszID, WPD_OBJECT_SIZE ) ;
		if ( GetContentDateProperty
			( pContent->m_dtCreated, pwszID, WPD_OBJECT_DATE_CREATED ) )
		{
			pContent->m_flags |= flagCreatedDate ;
		}
		if ( GetContentDateProperty
			( pContent->m_dtModified, pwszID, WPD_OBJECT_DATE_MODIFIED ) )
		{
			pContent->m_flags |= flagModifiedDate ;
		}
		if ( GetContentDateProperty
			( pContent->m_dtModified, pwszID, WPD_OBJECT_DATE_AUTHORED ) )
		{
			pContent->m_flags |= flagAuthoredDate ;
		}
	}
	while ( false ) ;
	//
	if ( pdkcKeys != NULL )
	{
		pdkcKeys->Release() ;
		pdkcKeys = NULL ;
	}
	if ( ppdvValues != NULL )
	{
		ppdvValues->Release() ;
		ppdvValues = NULL ;
	}
	return	pContent ;
}

// 文字列プロパティ取得
//////////////////////////////////////////////////////////////////////////////
bool SWin32PortableDevice::GetContentStringProperty
	( SString& strValue,
		const wchar_t * pwszID, const PROPERTYKEY& propKey ) const
{
	ESLAssert( m_pProp != NULL ) ;
	IPortableDeviceKeyCollection *	pdkcKeys = NULL ;
	IPortableDeviceValues *			ppdvValues = NULL ;
	bool							fSuccessed = false ;
	do
	{
		if ( !SUCCEEDED( CoCreateInstance
			( CLSID_PortableDeviceKeyCollection,
				NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IPortableDeviceKeyCollection),
				(void**) &pdkcKeys ) ) )
		{
			break ;
		}
        pdkcKeys->Add( propKey ) ;
		//
		if ( !SUCCEEDED
			( m_pProp->GetValues( pwszID, pdkcKeys, &ppdvValues ) ) )
		{
			break ;
		}
		PWSTR	strPropValue = NULL ;
		if ( SUCCEEDED
			( ppdvValues->GetStringValue( propKey, &strPropValue ) ) )
		{
			strValue = strPropValue ;
			CoTaskMemFree( strPropValue ) ;
			fSuccessed = true ;
		}
	}
	while ( false ) ;
	//
	if ( pdkcKeys != NULL )
	{
		pdkcKeys->Release() ;
		pdkcKeys = NULL ;
	}
	if ( ppdvValues != NULL )
	{
		ppdvValues->Release() ;
		ppdvValues = NULL ;
	}
	return	fSuccessed ;
}

// 数値プロパティ取得
//////////////////////////////////////////////////////////////////////////////
bool SWin32PortableDevice::GetContentULargeIntegerProperty
	( uint64_t& nValue,
		const wchar_t * pwszID, const PROPERTYKEY& propKey ) const
{
	ESLAssert( m_pProp != NULL ) ;
	IPortableDeviceKeyCollection *	pdkcKeys = NULL ;
	IPortableDeviceValues *			ppdvValues = NULL ;
	bool							fSuccessed = false ;
	do
	{
		if ( !SUCCEEDED( CoCreateInstance
			( CLSID_PortableDeviceKeyCollection,
				NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IPortableDeviceKeyCollection),
				(void**) &pdkcKeys ) ) )
		{
			break ;
		}
        pdkcKeys->Add( propKey ) ;
		//
		if ( !SUCCEEDED
			( m_pProp->GetValues( pwszID, pdkcKeys, &ppdvValues ) ) )
		{
			break ;
		}
		ULONGLONG	nPropValue = NULL ;
		if ( SUCCEEDED
			( ppdvValues->GetUnsignedLargeIntegerValue( propKey, &nPropValue ) ) )
		{
			nValue = nPropValue ;
			fSuccessed = true ;
		}
	}
	while ( false ) ;
	//
	if ( pdkcKeys != NULL )
	{
		pdkcKeys->Release() ;
		pdkcKeys = NULL ;
	}
	if ( ppdvValues != NULL )
	{
		ppdvValues->Release() ;
		ppdvValues = NULL ;
	}
	return	fSuccessed ;
}

// 日付プロパティ取得
//////////////////////////////////////////////////////////////////////////////
bool SWin32PortableDevice::GetContentDateProperty
	( DATE_TIME& dtValue,
		const wchar_t * pwszID, const PROPERTYKEY& propKey ) const
{
	ESLAssert( m_pProp != NULL ) ;
	IPortableDeviceKeyCollection *	pdkcKeys = NULL ;
	IPortableDeviceValues *			ppdvValues = NULL ;
	bool							fSuccessed = false ;
	do
	{
		if ( !SUCCEEDED( CoCreateInstance
			( CLSID_PortableDeviceKeyCollection,
				NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IPortableDeviceKeyCollection),
				(void**) &pdkcKeys ) ) )
		{
			break ;
		}
        pdkcKeys->Add( propKey ) ;
		//
		if ( !SUCCEEDED
			( m_pProp->GetValues( pwszID, pdkcKeys, &ppdvValues ) ) )
		{
			break ;
		}
		PROPVARIANT	pv = { 0 } ;
		if ( SUCCEEDED( ppdvValues->GetValue( propKey, &pv ) ) )
		{
			if ( DateFromPropVariant( dtValue, pv ) )
			{
				fSuccessed = true ;
			}
			PropVariantClear( &pv ) ;
		}
	}
	while ( false ) ;
	//
	if ( pdkcKeys != NULL )
	{
		pdkcKeys->Release() ;
		pdkcKeys = NULL ;
	}
	if ( ppdvValues != NULL )
	{
		ppdvValues->Release() ;
		ppdvValues = NULL ;
	}
	return	fSuccessed ;
}

// PROPVARIANT から時刻へ変換
//////////////////////////////////////////////////////////////////////////////
bool SWin32PortableDevice::DateFromPropVariant( DATE_TIME& dt, const PROPVARIANT& pv )
{
	if ( (pv.vt == VT_DATE) || (pv.vt == VT_R8) )
	{
		// 1899/12/31 を基準とする日数
		int64_t	nDays = eslRoundR64ToLInt( floor( pv.date ) ) ;
		int64_t	nMilliSec =
			eslRoundR64ToLInt
				( (pv.date - (double) nDays) * (24 * 60 * 60 * 1000) ) ;
		nDays += DATE_TIME::GetAccumulatedDayCount( 1899, 12, 31 ) - 1 ;
		//
		dt.SetAccumulatedDayCount( nDays ) ;
		dt.nMilliSec = (uint16_t) (nMilliSec % 1000) ;
		int	nSec = (int) ((nMilliSec - dt.nMilliSec) / 1000) ;
		dt.nSecond = (uint16_t) (nSec % 60) ;
		int	nMin = (int) ((nSec - dt.nSecond) / 60) ;
		dt.nMinute = (uint16_t) (nMin % 60) ;
		dt.nHour = (uint16_t) ((nMin - dt.nMinute) / 60) ;
		dt.nWeek = (uint16_t) dt.ComputeDayOfWeek() ;
		return	true ;
	}
	else if ( pv.vt == VT_FILETIME )
	{
		// Windows 64bit UTC
		SYSTEMTIME	st ;
		::FileTimeToSystemTime( &(pv.filetime), &st ) ;
		//
		dt.nYear = st.wYear ;
		dt.nMonth = st.wMonth ;
		dt.nDay = st.wYear ;
		dt.nWeek = st.wDayOfWeek ;
		dt.nHour = st.wHour ;
		dt.nMinute = st.wMinute ;
		dt.nSecond = st.wSecond ;
		dt.nMilliSec = st.wMilliseconds ;
		return	true ;
	}
	return	false ;
}

// 時刻から PROPVARIANT へ変換
//////////////////////////////////////////////////////////////////////////////
void SWin32PortableDevice::PropVariantFromDate( PROPVARIANT& pv, const DATE_TIME& dt )
{
	int64_t	nDays = dt.GetAccumulatedDayCount() ;
	int64_t	nMilliSec = dt.nHour * (60 * 60 * 1000)
						+ dt.nMinute * (60 * 1000)
						+ dt.nSecond * 1000 + dt.nMilliSec ;
	nDays -= DATE_TIME::GetAccumulatedDayCount( 1899, 12, 31 ) - 1 ;
	//
	pv.vt = VT_DATE ;
	pv.date = (double) nDays + (double) nMilliSec / (24 * 60 * 60 * 1000) ;
}

