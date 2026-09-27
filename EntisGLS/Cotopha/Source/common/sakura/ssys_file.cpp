
#include <sakura/sakura.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakura/ssys_win_registry.h>
#endif

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#if	defined(__PLATFORM_ANDROID__)
#include <sakura/ssys_android_file.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 入力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SSystem::SInputStream, ESLObject )

// 文字列読み込み
//////////////////////////////////////////////////////////////////////////////
SError SInputStream::ReadString( SString & strBuf )
{
	DWORD	dwLength ;
	if ( Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( dwLength != -1 )
	{
		ESLAssert( dwLength < 0x20000000 ) ;
		DWORD	dwBytes = dwLength * sizeof(uint16_t) ;
		if ( Read( strBuf.LockBuffer( dwLength ), dwBytes ) < dwBytes )
		{
			strBuf.UnlockBuffer( 0 ) ;
			return	errFailed ;
		}
		strBuf.UnlockBuffer( dwLength ) ;
	}
	else
	{
		strBuf.FreeArray() ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 出力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( SSystem::SOutputStream, ESLObject )

// 文字列書き出し
//////////////////////////////////////////////////////////////////////////////
SError SOutputStream::WriteString( const SString & strBuf )
{
	DWORD	dwLength = (DWORD) strBuf.GetLength() ;
	if ( Write( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( dwLength > 0 )
	{
		DWORD	dwBytes = dwLength * sizeof(uint16_t) ;
		if ( Write( strBuf.GetConstArray(), dwBytes ) < dwBytes )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

size_t SOutputStream::WriteEncodedString
	( const SString & strBuf, Charset::EncodingType encoding )
{
	return	WriteEncodedString
		( strBuf, (ssize_t) strBuf.GetLength(), encoding ) ;
}

size_t SOutputStream::WriteEncodedString
	( const wchar_t * pszStr,
		ssize_t nLength, Charset::EncodingType encoding )
{
	SArray<uint8_t>	strDst ;
	Charset::Encode( strDst, encoding, pszStr, nLength ) ;
	return	Write( strDst.GetConstArray(), strDst.GetLength() ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ファイル・オープン・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFileOpener, SObject )

SFileOpener *	SFileOpener::m_pDefaultOpener = NULL ;

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::RemoveSubFile( const wchar_t * pszFilePath )
{
	return	errFailed ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	return	errFailed ;
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::RemoveSubDirectory( const wchar_t * pszPath )
{
	return	errFailed ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	return	errFailed ;
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	return	errFailed ;
}

// システム規定のファイルオープン
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFileOpener::DefaultNewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFileInterface *	pFile = NULL ;
#if	!defined(__COTOPHA__)
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		pFile = pEnv->NewOpenFile( pszFilePath, nOpenFlags ) ;
		if ( pFile != nullptr )
		{
			return	pFile ;
		}
		if ( (nOpenFlags & SFileOpener::modeWrite)
			&& !(pEnv->CanOpenAllFileForWriting()) )
		{
			return	nullptr ;
		}
	}
#endif
	if ( m_pDefaultOpener != nullptr )
	{
		pFile = m_pDefaultOpener->NewOpenFile( pszFilePath, nOpenFlags ) ;
	}
	if ( pFile == nullptr )
	{
		pFile = SFile::NewOpen( pszFilePath, nOpenFlags ) ;
	}
	return	pFile ;
}

// システム規定のファイル削除
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::DefaultRemoveFile( const wchar_t * pszFilePath )
{
#if	!defined(__COTOPHA__)
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		SFileOpener *	pOpener = pEnv->GetWritableFileOpener() ;
		if ( pOpener != NULL )
		{
			if ( pOpener->IsExisting( pszFilePath ) )
			{
				return	pOpener->RemoveSubFile( pszFilePath ) ;
			}
		}
	}
#endif
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->RemoveSubFile( pszFilePath ) ;
	}
	return	SFile::RemoveFile( pszFilePath ) ;
}

// システム規定の仮想パスから直接パスへ変換
//////////////////////////////////////////////////////////////////////////////
SError SFileOpener::DefaultDirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
#if	!defined(__COTOPHA__)
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( (pEnv != NULL) && (g_defURLOpener.FindScheme( pszFilePath ) < 0) )
	{
		SFileOpener *	pOpener = pEnv->GetWritableFileOpener() ;
		if ( pOpener != NULL )
		{
			if ( pOpener->IsExisting( pszFilePath ) )
			{
				return	pOpener->DirectPathOf( strDirectPath, pszFilePath ) ;
			}
		}
		size_t	nCount = pEnv->GetFileOpenerCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pOpener = pEnv->GetFileOpenerAt( i ) ;
			if ( pOpener != NULL )
			{
				if ( pOpener->IsExisting( pszFilePath ) )
				{
					return	pOpener->DirectPathOf
									( strDirectPath, pszFilePath ) ;
				}
			}
		}
	}
#endif
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->DirectPathOf( strDirectPath, pszFilePath ) ;
	}
	strDirectPath = pszFilePath ;
	return	errSuccess ;
}

// システム規定のファイル存在確認
//////////////////////////////////////////////////////////////////////////////
SFileOpener * SFileOpener::DefaultGetExisting
		( const wchar_t * pszFilePath, bool fWritable )
{
#if	!defined(__COTOPHA__)
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( (pEnv != NULL) && (g_defURLOpener.FindScheme( pszFilePath ) < 0) )
	{
		SFileOpener *	pOpener = pEnv->GetWritableFileOpener() ;
		if ( pOpener != NULL )
		{
			if ( pOpener->IsExisting( pszFilePath ) )
			{
				return	pOpener ;
			}
		}
		if ( !fWritable )
		{
			size_t	nCount = pEnv->GetFileOpenerCount() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				pOpener = pEnv->GetFileOpenerAt( i ) ;
				if ( pOpener != NULL )
				{
					if ( pOpener->IsExisting( pszFilePath ) )
					{
						return	pOpener ;
					}
				}
			}
		}
	}
#endif
	if ( m_pDefaultOpener != NULL )
	{
		if ( m_pDefaultOpener->IsExisting( pszFilePath ) )
		{
			return	m_pDefaultOpener ;
		}
	}
	return	NULL ;
}

bool SFileOpener::DefaultIsExisting( const wchar_t * pszFilePath )
{
	if ( DefaultGetExisting( pszFilePath ) != NULL )
	{
		return	true ;
	}
	return	SFile::IsExistingAbsPath( pszFilePath ) ;
}

// ワイルドカード判定
//////////////////////////////////////////////////////////////////////////////
bool SFileOpener::IsMatchWildCardTo
	( const wchar_t * pwszWildCard, const wchar_t * pwszFileName )
{
	SStringParser	sparsWildCard ;
	size_t	i = 0, j = 0 ;
	while ( pwszWildCard[i] )
	{
		wchar_t	wc = pwszWildCard[i ++] ;
		if ( wc == L'*' )
		{
			wchar_t	wch = pwszWildCard[i] ;
			if ( (wch == L'.')
				&& (pwszWildCard[i + 1] == L'*')
				&& (pwszWildCard[i + 2] == L'\0') )
			{
				return	true ;
			}
			while ( pwszFileName[j] )
			{
				if ( pwszFileName[j] == wch )
				{
					for ( size_t k = 0; true; k ++ )
					{
						wchar_t	wchNext = pwszWildCard[i + k] ;
						if ( (wchNext == L'*')
							|| (wchNext == L'?') )
						{
							break ;
						}
						if ( !IsMatchFileChar( pwszFileName[j + k], wchNext ) )
						{
							if ( (pwszFileName[j + k] == L'\0')
								&& (wchNext == L'.')
								&& (pwszWildCard[i + k + 1] == L'*')
								&& (pwszWildCard[i + k + 2] == L'\0') )
							{
								return	true ;
							}
							return	false ;
						}
						if ( (wchNext == L'\0')
							|| (wchNext == L'.') )
						{
							break ;
						}
					}
					break ;
				}
				j ++ ;
			}
			if ( pwszFileName[j] != wch )
			{
				return	false ;
			}
		}
		else if ( wc == L'?' )
		{
			wchar_t	wch = pwszWildCard[i] ;
			bool	fMatchNext = false ;
			if ( IsMatchFileChar( pwszFileName[j], wch ) )
			{
				for ( size_t k = 0; true; k ++ )
				{
					wc = pwszWildCard[i + k] ;
					if ( (wc == 0) || (wc == L'.')
							|| (wc == L'*') || (wc == L'?') )
					{
						fMatchNext = true ;
						break ;
					}
					if ( !IsMatchFileChar( wc, pwszFileName[j + k] ) )
					{
						break ;
					}
				}
			}
			if ( !fMatchNext )
			{
				j ++ ;
			}
		}
		else
		{
			if ( (wc == L'.')
				&& (pwszWildCard[i] == L'*')
				&& (pwszWildCard[i + 1] == L'\0')
				&& (pwszFileName[j] == L'\0') )
			{
				return	true ;
			}
			if ( !IsMatchFileChar( pwszFileName[j ++], wc ) )
			{
				return	false ;
			}
		}
	}
	return	(pwszFileName[j] == 0) ;
}

bool SFileOpener::IsMatchFileChar( wchar_t wch1, wchar_t wch2 )
{
	if ( (L'a' <= wch1) && (wch1 <= L'z') )
	{
		wch1 -= L'a' - L'A' ;
	}
	if ( (L'a' <= wch2) && (wch2 <= L'z') )
	{
		wch2 -= L'a' - L'A' ;
	}
	return	(wch1 == wch2) ;
}



//////////////////////////////////////////////////////////////////////////////
// ファイル・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SSystem::SFileInterface, SFileOpener, SInputStream, SOutputStream )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFileInterface::SFileInterface( void )
{
}

// 構築関数（ダミー）
//////////////////////////////////////////////////////////////////////////////
SFileInterface::SFileInterface( const SFileInterface& file )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFileInterface::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	return	DefaultNewOpenFile( pszFilePath, nOpenFlags ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SFileInterface::IsExisting( const wchar_t * pszFilePath )
{
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->IsExisting( pszFilePath ) ;
	}
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SFileInterface::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->QueryState( pszFilePath, state ) ;
	}
	return	errFailed ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SFileInterface::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->RenameSubFile( pszOldPath, pszNewPath ) ;
	}
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFileInterface::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	listFiles.RemoveAll() ;
	if ( m_pDefaultOpener != NULL )
	{
		m_pDefaultOpener->ListSubDirectories( listFiles, pszDirPath ) ;
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFileInterface::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	listDirs.RemoveAll() ;
	if ( m_pDefaultOpener != NULL )
	{
		m_pDefaultOpener->ListSubDirectories( listDirs, pszDirPath ) ;
	}
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFileInterface::RemoveSubFile( const wchar_t * pszFilePath )
{
	if ( m_pDefaultOpener != NULL )
	{
		return	m_pDefaultOpener->RemoveSubFile( pszFilePath ) ;
	}
	return	errFailed ;
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SFileInterface::GetFileObject( void )
{
	return	NULL ;
}
#else
SFileInterface * SFileInterface::GetFileObject( void )
{
	return	this ;
}
#endif



//////////////////////////////////////////////////////////////////////////////
// 標準ファイル
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SSystem::SFile::DefaultName::StandardOutput = L"<stdout>" ;
const wchar_t *	SSystem::SFile::DefaultName::StandardInput = L"<stdin>" ;

const wchar_t *	SSystem::SFile::DefaultDirectory::CurrentDirectory = L"Current" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsDirectory = L"Windows" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsSystemDirectory = L"WindowsSystem" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsStartMenu = L"WindowsStartMenu" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsCommonStartMenu = L"WindowsCommonStartMenu" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsDesktop = L"WindowsDesktop" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::WindowsProgramFiles = L"WindowsProgramFiles" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::ApplicationData = L"AppData" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::UserDocuments = L"UserDocuments" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::UserMusic = L"UserMusic" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::UserPictures = L"UserPictures" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::UserVideos = L"UserVideos" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::ApplicationInstalled = L"AppInstalled" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::AndroidLocalFiles = L"AndroidLocalFiles" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::AndroidExternalStorage = L"AndroidExternalStorage" ;
const wchar_t *	SSystem::SFile::DefaultDirectory::AndroidExternalStoragePrivate = L"AndroidExternalStoragePrivate" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFile::SFile( void )
{
	#if	defined(__COTOPHA__)
		m_pFile = NULL ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hFile = INVALID_HANDLE_VALUE ;
		m_nFlags = 0 ;

	#else
		m_fdFile = -1 ;
		m_nFlags = 0 ;
	#endif
}

#if	defined(__COTOPHA__)
SFile::SFile( const SString & strFilePath, File * pFile, long int nFlags )
	: m_strFilePath( strFilePath )
{
	m_pFile = pFile ;
	m_nFlags = nFlags ;
}

#elif	defined(__PLATFORM_WINDOWS__)
SFile::SFile( const SString & strFilePath, HANDLE hFile, long int nFlags )
	: m_strFilePath( strFilePath )
{
	m_hFile = hFile ;
	m_nFlags = nFlags ;
}

#else
SFile::SFile( const SString & strFilePath, int fdFile, long int nFlags )
	: m_strFilePath( strFilePath )
{
	m_fdFile = fdFile ;
	m_nFlags = nFlags ;
}
#endif

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFile::~SFile( void )
{
	SFile::Close() ;
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SFile::GetFileObject( void )
{
	return	m_pFile ;
}
#endif

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError SFile::Open
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	Close() ;

	#if	defined(__COTOPHA__)
		m_pFile = File::NewOpen( pszFilePath, nOpenFlags ) ;
		if ( m_pFile == NULL )
		{
			return	errFailed ;
		}
		m_strFilePath = pszFilePath ;
		m_nFlags = nOpenFlags ;

	#elif	defined(__PLATFORM_WINDOWS__)
		//
		// ファイルオープンフラグを変換
		//
		DWORD	dwAccess = 0 ;
		DWORD	dwShareMode = 0 ;
		DWORD	dwCreate = OPEN_EXISTING ;
		if ( nOpenFlags & modeCreateFlag )
		{
			dwCreate = CREATE_ALWAYS ;
		}
		if ( nOpenFlags & modeRead )
		{
			dwAccess |= GENERIC_READ ;
		}
		if ( nOpenFlags & modeWrite )
		{
			dwAccess |= GENERIC_WRITE ;
		}
		if ( nOpenFlags & shareReadFlag )
		{
			dwShareMode |= FILE_SHARE_READ ;
		}
		if ( nOpenFlags & shareWriteFlag )
		{
			dwShareMode |= FILE_SHARE_WRITE ;
		}
		//
		// ファイルを開く
		//
		if ( (pszFilePath == NULL)
			|| (pszFilePath[0] == 0)
			|| (SString::Compare(pszFilePath,DefaultName::StandardOutput) == 0)
			|| (SString::Compare(pszFilePath,DefaultName::StandardInput) == 0) )
		{
			HANDLE	hStd = NULL ;
			if ( (nOpenFlags & modeWrite)
				&& (SString::Compare(pszFilePath,DefaultName::StandardOutput) == 0) )
			{
				hStd = ::GetStdHandle( STD_OUTPUT_HANDLE ) ;
				//
				DWORD	dwMode = 0 ;
				::GetConsoleMode( hStd, &dwMode ) ;
				dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING ;
				::SetConsoleMode( hStd, dwMode ) ;
			}
			else if ( SString::Compare(pszFilePath,DefaultName::StandardInput) == 0 )
			{
				hStd = ::GetStdHandle( STD_INPUT_HANDLE ) ;
			}
			else
			{
				return	errFailed ;
			}
			if ( !::DuplicateHandle
				( ::GetCurrentProcess(), hStd,
					::GetCurrentProcess(), &m_hFile, dwAccess, TRUE, 0 ) )
			{
				return	errFailed ;
			}
			m_strFilePath = pszFilePath ;
			m_nFlags = nOpenFlags ;
			return	errSuccess ;
		}
		if ( (pszFilePath != NULL) && (pszFilePath[1] == L':') )
		{
			char	szDrvRoot[] = "A:\\" ;
			szDrvRoot[0] = (char) pszFilePath[0] ;
			UINT	nDrvType = ::GetDriveType( szDrvRoot ) ;
			if ( (nDrvType == DRIVE_UNKNOWN)
					|| (nDrvType == DRIVE_NO_ROOT_DIR) )
			{
				return	errFailed ;
			}
		}
		if ( (nOpenFlags & (modeCreateFlag|modeCreateDirFlag))
							== (modeCreateFlag|modeCreateDirFlag) )
		{
			SString	strFilePath = pszFilePath ;
			CreateFullDirectory( strFilePath.GetFileDirectoryPart() ) ;
		}
		//
		UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
		//
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			m_hFile = ::CreateFileW
				( pszFilePath, dwAccess, dwShareMode,
					NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		}
		else
		{
			SArray<uint8_t>	strFilePath ;
			Charset::Encode
				( strFilePath, Charset::encodingShiftJIS, pszFilePath ) ;
			strFilePath.Add( 0 ) ;
			//
			m_hFile = ::CreateFileA
				( (const char*) strFilePath.GetConstArray(),
					dwAccess, dwShareMode,
					NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		}
		//
		::SetErrorMode( nErrorMode ) ;
		//
		if ( m_hFile == INVALID_HANDLE_VALUE )
		{
			// ファイルのオープンに失敗
			return	errFailed ;
		}
		m_strFilePath = pszFilePath ;
		m_nFlags = nOpenFlags ;

	#else
		if ( (pszFilePath == NULL)
			|| (pszFilePath[0] == 0)
			|| (SString::Compare(pszFilePath,DefaultName::StandardOutput) == 0)
			|| (SString::Compare(pszFilePath,DefaultName::StandardInput) == 0) )
		{
			if ( (nOpenFlags & modeWrite)
				&& (SString::Compare(pszFilePath,DefaultName::StandardOutput) == 0) )
			{
				m_fdFile = 0 ;
			}
			else if ( SString::Compare(pszFilePath,DefaultName::StandardInput) == 0 )
			{
				m_fdFile = 1 ;
			}
			else
			{
				return	errFailed ;
			}
			m_strFilePath = pszFilePath ;
			m_nFlags = nOpenFlags ;
		}
		else
		{
			//
			// ファイルオープンフラグを変換
			//
			int			flags = 0 ;
			mode_t		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;
			long int	nPermission =
							(nOpenFlags & SFileOpener::maskPermission)
										>> SFileOpener::shifterPermission ;
			if ( nPermission != 0 )
			{
				mode = 0 ;
				if ( nPermission & SFileOpener::permissionRUSR )
				{
					mode |= S_IRUSR ;
				}
				if ( nPermission & SFileOpener::permissionWUSR )
				{
					mode |= S_IWUSR ;
				}
				if ( nPermission & SFileOpener::permissionXUSR )
				{
					mode |= S_IXUSR ;
				}
				if ( nPermission & SFileOpener::permissionRGRP )
				{
					mode |= S_IRGRP ;
				}
				if ( nPermission & SFileOpener::permissionWGRP )
				{
					mode |= S_IWGRP ;
				}
				if ( nPermission & SFileOpener::permissionXGRP )
				{
					mode |= S_IXGRP ;
				}
				if ( nPermission & SFileOpener::permissionROTH )
				{
					mode |= S_IROTH ;
				}
				if ( nPermission & SFileOpener::permissionWOTH )
				{
					mode |= S_IWOTH ;
				}
				if ( nPermission & SFileOpener::permissionXOTH )
				{
					mode |= S_IXOTH ;
				}
			}
			if ( nOpenFlags & modeCreateFlag )
			{
				flags |= O_CREAT ;
			}
			if ( (nOpenFlags & modeReadWrite) == modeReadWrite )
			{
				flags |= O_RDWR ;
			}
			else if ( nOpenFlags & modeWrite )
			{
				flags |= O_WRONLY ;
			}
			else
			{
				flags |= O_RDONLY ;
			}
			m_strFilePath = pszFilePath ;
			m_strFilePath.Replace( L'\\', L'/' ) ;
			m_nFlags = nOpenFlags ;
			//
			if ( (nOpenFlags & (modeCreateFlag|modeCreateDirFlag))
								== (modeCreateFlag|modeCreateDirFlag) )
			{
				CreateFullDirectory
					( m_strFilePath.GetFileDirectoryPart() ) ;
			}
			SArray<char>	bufFilePath ;
			const char *	pszBufFilePath =
						m_strFilePath.EncodeDefaultTo(bufFilePath) ;
			m_fdFile = open( pszBufFilePath, flags, mode ) ;
			if ( m_fdFile == -1 )
			{
				// ファイルのオープンに失敗
				Trace( "failed to open \'%s\' %03X (error=%d, %s)\n",
						bufFilePath.GetConstArray(), mode, errno, strerror(errno) ) ;
				return	errFailed ;
			}
			if ( nOpenFlags & modeCreateFlag )
			{
				// ※ Android では以前のファイルは残ったままになるので
				ftruncate( m_fdFile, 0 ) ;
			}
		}

	#endif
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SFile::Close( void )
{
	#if	defined(__COTOPHA__)
		delete	m_pFile ;
		m_pFile = NULL ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( m_hFile ) ;
			m_hFile = INVALID_HANDLE_VALUE ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			close( m_fdFile ) ;
			m_fdFile = -1 ;
		}
	#endif
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFile::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	return	NewOpenFileAbsPath( OffsetPath( pszFilePath ), nOpenFlags ) ;
}

SFile * SFile::NewOpenFileAbsPath
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFile *	pFile = new SFile ;
	if ( pFile->Open( pszFilePath, nOpenFlags ) != errSuccess )
	{
		delete	pFile ;
		return	NULL ;
	}
	return	pFile ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SFile::IsExisting( const wchar_t * pszFilePath )
{
	return	IsExistingAbsPath( OffsetPath( pszFilePath ) ) ;
}

bool SFile::IsExistingAbsPath( const wchar_t * pszFilePath )
{
	#if	defined(__COTOPHA__)
		return	File::IsExistingFile( pszFilePath ) ;

	#else
		State	st ;
		return	(SFile::QueryFileState( pszFilePath, st ) == errSuccess) ;

	#endif
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFile::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	ListSubFilesAbsPath( listFiles, OffsetPath( pszDirPath ) ) ;
}

void SFile::ListSubFilesAbsPath
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	listFiles.RemoveAll() ;

	#if	defined(__COTOPHA__)
		File::ListFiles( listFiles, pszDirPath ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			WIN32_FIND_DATAW	wfd ;
			HANDLE	hFind =
				::FindFirstFileW
					( SString(pszDirPath).OffsetFilePath("*.*"), &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				do
				{
					if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						listFiles.Add( new SString( wfd.cFileName ) ) ;
					}
				}
				while ( ::FindNextFileW( hFind, &wfd ) ) ;
				::FindClose( hFind ) ;
			}
		}
		else
		{
			SArray<uint8_t>	strDirPath ;
			Charset::Encode
				( strDirPath, Charset::encodingShiftJIS,
					SString(pszDirPath).OffsetFilePath("*.*") ) ;
			strDirPath.Add( 0 ) ;
			//
			WIN32_FIND_DATAA	wfd ;
			HANDLE	hFind =
				::FindFirstFileA
					( (const char*) strDirPath.GetConstArray(), &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				do
				{
					if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						listFiles.Add( new SString( wfd.cFileName ) ) ;
					}
				}
				while ( ::FindNextFileA( hFind, &wfd ) ) ;
				::FindClose( hFind ) ;
			}
		}

	#elif	defined(__PLATFORM_UNIX_LIKE__)
		SString	strDirPath = pszDirPath ;
		strDirPath.Replace( L'\\', L'/' ) ;
		DIR *	dir = opendir( strDirPath.ToCharArray() ) ;
		if ( dir != NULL )
		{
			for ( ; ; )
			{
				dirent*	dent = readdir( dir ) ;
				if ( dent != NULL )
				{
					if ( !(dent->d_type & DT_DIR) )
					{
						listFiles.Add( new SString( dent->d_name ) ) ;
					}
				}
				else
				{
					break ;
				}
			}
			closedir( dir ) ;
		}

	#endif
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFile::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	ListSubDirectoriesAbsPath( listDirs, OffsetPath( pszDirPath ) ) ;
}

void SFile::ListSubDirectoriesAbsPath
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	listDirs.RemoveAll() ;

	#if	defined(__COTOPHA__)
		File::ListDirectories( listDirs, pszDirPath ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			WIN32_FIND_DATAW	wfd ;
			HANDLE	hFind =
				::FindFirstFileW
					( SString(pszDirPath).OffsetFilePath("*.*"), &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				do
				{
					if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						listDirs.Add( new SString( wfd.cFileName ) ) ;
					}
				}
				while ( ::FindNextFileW( hFind, &wfd ) ) ;
				::FindClose( hFind ) ;
			}
		}
		else
		{
			SArray<uint8_t>	strDirPath ;
			Charset::Encode
				( strDirPath, Charset::encodingShiftJIS,
					SString(pszDirPath).OffsetFilePath("*.*") ) ;
			strDirPath.Add( 0 ) ;
			//
			WIN32_FIND_DATAA	wfd ;
			HANDLE	hFind =
				::FindFirstFileA
					( (const char*) strDirPath.GetConstArray(), &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				do
				{
					if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						listDirs.Add( new SString( wfd.cFileName ) ) ;
					}
				}
				while ( ::FindNextFileA( hFind, &wfd ) ) ;
				::FindClose( hFind ) ;
			}
		}

	#elif	defined(__PLATFORM_UNIX_LIKE__)
		SString	strDirPath = pszDirPath ;
		strDirPath.Replace( L'\\', L'/' ) ;
		DIR *	dir = opendir( strDirPath.ToCharArray() ) ;
		if ( dir != NULL )
		{
			for ( ; ; )
			{
				dirent*	dent = readdir( dir ) ;
				if ( dent != NULL )
				{
					if ( dent->d_type & DT_DIR )
					{
						listDirs.Add( new SString( dent->d_name ) ) ;
					}
				}
				else
				{
					break ;
				}
			}
			closedir( dir ) ;
		}
	#endif
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RemoveSubFile( const wchar_t * pszPath )
{
	return	RemoveFile( OffsetPath( pszPath ) ) ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	return	RenameFile( OffsetPath( pszOldPath ), OffsetPath( pszNewPath ) ) ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SFile::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	return	CreateDirectory( OffsetPath( pszPath ), nFlags ) ;
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RemoveSubDirectory( const wchar_t * pszPath )
{
	return	RemoveDirectory( OffsetPath( pszPath ) ) ;
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SFile::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	strDirectPath = OffsetPath( pszFilePath ) ;
#if	defined(__PLATFORM_UNIX_LIKE__)
	strDirectPath.Replace( L'\\', L'/' ) ;
#endif
	return	errSuccess ;
}

// オフセットパスを取得する
//////////////////////////////////////////////////////////////////////////////
SString SFile::OffsetPath( const wchar_t * pszFilePath )
{
	return	m_strFilePath.GetFileDirectoryPart().OffsetFilePath( pszFilePath ) ;
}

// ベースパスを設定する
//////////////////////////////////////////////////////////////////////////////
void SFile::SetFilePath( const wchar_t * pszFilePath )
{
	m_strFilePath = pszFilePath ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFile::Duplicate( void ) const
{
	#if	defined(__COTOPHA__)
		if ( m_pFile != NULL )
		{
			File *	pFile = m_pFile->Duplicate() ;
			if ( pFile != NULL )
			{
				return	new SFile( m_strFilePath, pFile, m_nFlags ) ;
			}
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			long int	nFlags = m_nFlags & ~modeCreateFlag ;
			DWORD	dwAccess = 0 ;
			DWORD	dwShareMode = 0 ;
			DWORD	dwCreate = OPEN_EXISTING ;
			if ( m_nFlags & modeRead )
			{
				dwAccess |= GENERIC_READ ;
				dwShareMode |= FILE_SHARE_READ ;
				nFlags |= shareReadFlag ;
			}
			if ( m_nFlags & modeWrite )
			{
				dwAccess |= GENERIC_WRITE ;
				dwShareMode |= FILE_SHARE_WRITE ;
				nFlags |= shareWriteFlag ;
			}
			UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
			//
			HANDLE	hFile ;
			if ( g_infoPlatform.runtimeOS != platformOS_Windows )
			{
				hFile = ::CreateFileW
					( m_strFilePath, dwAccess, dwShareMode,
						NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			}
			else
			{
				SArray<char>	strFilePath ;
				hFile = ::CreateFileA
					( m_strFilePath.EncodeDefaultTo( strFilePath ),
						dwAccess, dwShareMode,
						NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			}
			//
			::SetErrorMode( nErrorMode ) ;
			//
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				return	new SFile( m_strFilePath, hFile, nFlags ) ;
			}
			else
			{
				HANDLE 	hProcess = ::GetCurrentProcess() ;
				if ( ::DuplicateHandle
					( hProcess, m_hFile,
						hProcess, &hFile, dwAccess,
						TRUE, DUPLICATE_SAME_ACCESS ) )
				{
					return	new SFile( m_strFilePath, hFile, m_nFlags ) ;
				}
			}
		}

	#else
		if ( m_fdFile != -1 )
		{
			int	flags = 0 ;
			if ( (m_nFlags & modeReadWrite) == modeReadWrite )
			{
				flags |= O_RDWR ;
			}
			else if ( m_nFlags & modeWrite )
			{
				flags |= O_WRONLY ;
			}
			else
			{
				flags |= O_RDONLY ;
			}
			int	fdFile = open( m_strFilePath.ToCharArray().GetConstArray(), flags ) ;
			if ( fdFile != -1 )
			{
				return	new SFile( m_strFilePath, fdFile, m_nFlags ) ;
			}
			else
			{
				fdFile = dup( m_fdFile ) ;
				if ( fdFile != -1 )
				{
					return	new SFile( m_strFilePath, fdFile, m_nFlags ) ;
				}
			}
		}

	#endif
	return	NULL ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SFile::Read( void * ptrBuf, size_t nBytes )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->Read( ptrBuf, nBytes ) ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		ESLAssert( m_nFlags & modeRead ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			DWORD	dwReadBytes = 0 ;
			::ReadFile( m_hFile, ptrBuf, (DWORD) nBytes, &dwReadBytes, NULL ) ;
			return	dwReadBytes ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			return	read( m_fdFile, ptrBuf, nBytes ) ;
		}
	#endif
	return	0 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SFile::Write( const void * ptrBuf, size_t nBytes )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->Write( ptrBuf, nBytes ) ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		ESLAssert( m_nFlags & modeWrite ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			DWORD	dwWrittenBytes = 0 ;
			::WriteFile( m_hFile, ptrBuf, (DWORD) nBytes, &dwWrittenBytes, NULL ) ;
			return	dwWrittenBytes ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			return	write( m_fdFile, ptrBuf, nBytes ) ;
		}
	#endif
	return	0 ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SFile::IsSeekable( void ) const
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->IsSeekable() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			return	true ;	// 但しシーク不可の場合もあり
		}

	#else
		if ( m_fdFile != -1 )
		{
			return	true ;	// 但しシーク不可の場合もあり
		}
	#endif
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFile::GetLength( void ) const
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->GetLength() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			SetLastError( NO_ERROR ) ;
			DWORD	dwHigh = 0 ;
			DWORD	dwSize = ::GetFileSize( m_hFile, &dwHigh ) ;
			if ( (dwSize == (DWORD) -1) && (GetLastError() != NO_ERROR) )
			{
				dwSize = 0 ;
			}
			return	(((int64_t) dwHigh) << 32) | dwSize ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			off64_t	offCur = lseek64( m_fdFile, 0, SEEK_CUR ) ;
			if ( offCur != -1 )
			{
				off64_t	offEnd = lseek64( m_fdFile, 0, SEEK_END ) ;
				lseek64( m_fdFile, offCur, SEEK_SET ) ;
				//
				if ( offEnd != -1 )
				{
					return	offEnd ;
				}
			}
		}
	#endif
	return	0 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->Seek( posFile, seekFrom ) ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			DWORD	dwMoveMethod ;
			DWORD	dwNewPos ;
			LONG	nHigh = (LONG) (posFile >> 32) ;
			DWORD	dwHigh = 0 ;
			switch ( seekFrom )
			{
			case	FromBegin:
			default:
				dwMoveMethod = FILE_BEGIN ;
				break ;
			case	FromCurrent:
				dwMoveMethod = FILE_CURRENT ;
				break ;
			case	FromEnd:
				dwMoveMethod = FILE_END ;
				break ;
			}
			SetLastError( NO_ERROR ) ;
			dwNewPos =
				::SetFilePointer
					( m_hFile, (DWORD) posFile, &nHigh, dwMoveMethod ) ;
			if ( (dwNewPos == INVALID_SET_FILE_POINTER)
							&& (GetLastError() != NO_ERROR) )
			{
				nHigh = 0 ;
				dwNewPos = 0 ;
			}
			return	(((int64_t) nHigh) << 32) | dwNewPos ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			int	whence ;
			switch ( seekFrom )
			{
			case	FromBegin:
			default:
				whence = SEEK_SET ;
				break ;
			case	FromCurrent:
				whence = SEEK_CUR ;
				break ;
			case	FromEnd:
				whence = SEEK_END ;
				break ;
			}
			off64_t	offCur = lseek64( m_fdFile, posFile, whence ) ;
			if ( offCur != -1 )
			{
				return	offCur ;
			}
		}
	#endif
	return	0 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFile::GetPosition( void ) const
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->GetPosition() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		if ( m_hFile != INVALID_HANDLE_VALUE )
		{
			LONG	nHigh = 0 ;
			DWORD	dwNewPos ;
			SetLastError( NO_ERROR ) ;
			dwNewPos = ::SetFilePointer( m_hFile, 0, &nHigh, FILE_CURRENT ) ;
			if ( (dwNewPos == INVALID_SET_FILE_POINTER)
							&& GetLastError() != NO_ERROR )
			{
				nHigh = 0 ;
				dwNewPos = 0 ;
			}
			return	(((int64_t) nHigh) << 32) | dwNewPos ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			off64_t	offCur = lseek64( m_fdFile, 0, SEEK_CUR ) ;
			if ( offCur != -1 )
			{
				return	offCur ;
			}
		}
	#endif
	return	0 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SFile::SetEndOfFile( void )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->SetEndOfFile() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		if ( ::SetEndOfFile( m_hFile ) )
		{
			return	errSuccess ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			if ( ftruncate( m_fdFile, GetPosition() ) == 0 )
			{
				return	errSuccess ;
			}
		}
	#endif
	return	errFailed ;
}

// ファイル時刻取得
//////////////////////////////////////////////////////////////////////////////
SError SFile::GetFileTime( DATE_TIME& time )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->GetFileTime( time ) ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		FILETIME	ftCreation, ftLastAccess, ftLastWrite ;
		if( ::GetFileTime
			( m_hFile, &ftCreation, &ftLastAccess, &ftLastWrite ) )
		{
			FILETIME	ftLocalTime ;
			SYSTEMTIME	stLocalTime ;
			::FileTimeToLocalFileTime( &ftLastWrite, &ftLocalTime ) ;
			::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) ;
			//
			time.nYear = stLocalTime.wYear ;
			time.nMonth = stLocalTime.wMonth ;
			time.nDay = stLocalTime.wDay ;
			time.nWeek = stLocalTime.wDayOfWeek ;
			time.nHour = stLocalTime.wHour ;
			time.nMinute = stLocalTime.wMinute ;
			time.nSecond = stLocalTime.wSecond ;
			time.nMilliSec = stLocalTime.wMilliseconds ;
			return	errSuccess ;
		}

	#else
		if ( m_fdFile != -1 )
		{
			struct stat	st ;
			if ( fstat( m_fdFile, &st ) == 0 )
			{
				tm		tmLocal ;
				tm *	ptmLocal ;
				ptmLocal = localtime_r
					( (const time_t*) &st.st_mtime, &tmLocal ) ;
				if ( ptmLocal != NULL )
				{
					time.nYear = (uint16_t) (ptmLocal->tm_year + 1900) ;
					time.nMonth = (uint16_t) (ptmLocal->tm_mon + 1) ;
					time.nDay = (uint16_t) (ptmLocal->tm_mday) ;
					time.nWeek = (uint16_t) (ptmLocal->tm_wday) ;
					time.nHour = (uint16_t) (ptmLocal->tm_hour) ;
					time.nMinute = (uint16_t) (ptmLocal->tm_min) ;
					time.nSecond = (uint16_t) (ptmLocal->tm_sec) ;
					time.nMilliSec = 0 ;
					return	errSuccess ;
				}
			}
		}
	#endif
	return	errFailed ;
}

// ファイル時刻設定
//////////////////////////////////////////////////////////////////////////////
SError SFile::SetFileTime( const DATE_TIME& time )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->SetFileTime( time ) ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
		FILETIME	ftLastWrite ;
		FILETIME	ftLocalTime ;
		SYSTEMTIME	stLocalTime ;
		stLocalTime.wYear = time.nYear  ;
		stLocalTime.wMonth = time.nMonth ;
		stLocalTime.wDay = time.nDay ;
		stLocalTime.wDayOfWeek = time.nWeek ;
		stLocalTime.wHour = time.nHour ;
		stLocalTime.wMinute = time.nMinute ;
		stLocalTime.wSecond = time.nSecond ;
		stLocalTime.wMilliseconds = time.nMilliSec ;
		::SystemTimeToFileTime( &stLocalTime, &ftLocalTime ) ;
		::LocalFileTimeToFileTime( &ftLocalTime, &ftLastWrite ) ;
		//
		if( ::SetFileTime( m_hFile, NULL, NULL, &ftLastWrite ) )
		{
			return	errSuccess ;
		}

	#else
	#endif
	return	errFailed ;
}


// 生成
//////////////////////////////////////////////////////////////////////////////
SFile * SFile::NewOpen
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFile *	pFile = new SFile ;
	if ( pFile->Open( pszFilePath, nOpenFlags ) == errSuccess )
	{
		return	pFile ;
	}
	delete	pFile ;
	return	NULL ;
}

// ファイルは存在しているか？
//////////////////////////////////////////////////////////////////////////////
bool SFile::IsExistingFile( const wchar_t * pszFilePath )
{
	State	state ;
	return	(QueryFileState( pszFilePath, state ) == errSuccess) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SFile::QueryFileState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	#if	defined(__COTOPHA__)
		return	File::QueryFileState( pszFilePath, state ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			WIN32_FIND_DATAW	wfd ;
			HANDLE	hFind = ::FindFirstFileW( pszFilePath, &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				state.bitFields = SFileOpener::fieldAttributes
								| SFileOpener::fieldFileSize ;
				state.bitAttributes = SFileOpener::permissionRUSR
										| SFileOpener::permissionWUSR
										| SFileOpener::permissionXUSR ;
				if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					state.bitAttributes |= SFileOpener::attrDirectory ;
				}
				state.nFileSize =
					wfd.nFileSizeLow
						| (((uint64_t)wfd.nFileSizeHigh) << 32) ;
				//
				FILETIME	ftLocalTime ;
				SYSTEMTIME	stLocalTime ;
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftLastAccessTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldAccessedTime ;
					state.dtAccessed.nYear = stLocalTime.wYear ;
					state.dtAccessed.nMonth = stLocalTime.wMonth ;
					state.dtAccessed.nDay = stLocalTime.wDay ;
					state.dtAccessed.nWeek = stLocalTime.wDayOfWeek ;
					state.dtAccessed.nHour = stLocalTime.wHour ;
					state.dtAccessed.nMinute = stLocalTime.wMinute ;
					state.dtAccessed.nSecond = stLocalTime.wSecond ;
					state.dtAccessed.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftLastWriteTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldModifiedTime ;
					state.dtModified.nYear = stLocalTime.wYear ;
					state.dtModified.nMonth = stLocalTime.wMonth ;
					state.dtModified.nDay = stLocalTime.wDay ;
					state.dtModified.nWeek = stLocalTime.wDayOfWeek ;
					state.dtModified.nHour = stLocalTime.wHour ;
					state.dtModified.nMinute = stLocalTime.wMinute ;
					state.dtModified.nSecond = stLocalTime.wSecond ;
					state.dtModified.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftCreationTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldCreatedTime ;
					state.dtCreated.nYear = stLocalTime.wYear ;
					state.dtCreated.nMonth = stLocalTime.wMonth ;
					state.dtCreated.nDay = stLocalTime.wDay ;
					state.dtCreated.nWeek = stLocalTime.wDayOfWeek ;
					state.dtCreated.nHour = stLocalTime.wHour ;
					state.dtCreated.nMinute = stLocalTime.wMinute ;
					state.dtCreated.nSecond = stLocalTime.wSecond ;
					state.dtCreated.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( FindNextFileW( hFind, &wfd ) )
				{
					::FindClose( hFind ) ;
					return	errFailed ;
				}
				::FindClose( hFind ) ;
				return	errSuccess ;
			}
		}
		else
		{
			SArray<char>	bufPath ;
			SString			strPath = pszFilePath ;
			WIN32_FIND_DATA	wfd ;
			HANDLE	hFind =
				::FindFirstFile
					( strPath.EncodeDefaultTo(bufPath), &wfd ) ;
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				state.bitFields = SFileOpener::fieldAttributes
								| SFileOpener::fieldFileSize ;
				state.bitAttributes = 0 ;
				if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					state.bitAttributes |= SFileOpener::attrDirectory ;
				}
				state.nFileSize =
					wfd.nFileSizeLow
						| (((uint64_t)wfd.nFileSizeHigh) << 32) ;
				//
				FILETIME	ftLocalTime ;
				SYSTEMTIME	stLocalTime ;
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftLastAccessTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldAccessedTime ;
					state.dtAccessed.nYear = stLocalTime.wYear ;
					state.dtAccessed.nMonth = stLocalTime.wMonth ;
					state.dtAccessed.nDay = stLocalTime.wDay ;
					state.dtAccessed.nWeek = stLocalTime.wDayOfWeek ;
					state.dtAccessed.nHour = stLocalTime.wHour ;
					state.dtAccessed.nMinute = stLocalTime.wMinute ;
					state.dtAccessed.nSecond = stLocalTime.wSecond ;
					state.dtAccessed.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftLastWriteTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldModifiedTime ;
					state.dtModified.nYear = stLocalTime.wYear ;
					state.dtModified.nMonth = stLocalTime.wMonth ;
					state.dtModified.nDay = stLocalTime.wDay ;
					state.dtModified.nWeek = stLocalTime.wDayOfWeek ;
					state.dtModified.nHour = stLocalTime.wHour ;
					state.dtModified.nMinute = stLocalTime.wMinute ;
					state.dtModified.nSecond = stLocalTime.wSecond ;
					state.dtModified.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( ::FileTimeToLocalFileTime
							( &wfd.ftCreationTime, &ftLocalTime )
					&& ::FileTimeToSystemTime( &ftLocalTime, &stLocalTime ) )
				{
					state.bitFields |= SFileOpener::fieldCreatedTime ;
					state.dtCreated.nYear = stLocalTime.wYear ;
					state.dtCreated.nMonth = stLocalTime.wMonth ;
					state.dtCreated.nDay = stLocalTime.wDay ;
					state.dtCreated.nWeek = stLocalTime.wDayOfWeek ;
					state.dtCreated.nHour = stLocalTime.wHour ;
					state.dtCreated.nMinute = stLocalTime.wMinute ;
					state.dtCreated.nSecond = stLocalTime.wSecond ;
					state.dtCreated.nMilliSec = stLocalTime.wMilliseconds ;
				}
				if ( FindNextFile( hFind, &wfd ) )
				{
					::FindClose( hFind ) ;
					return	errFailed ;
				}
				::FindClose( hFind ) ;
				return	errSuccess ;
			}
		}

	#else
		SString			strPath = pszFilePath ;
		SArray<char>	bufPath ;
		strPath.Replace( L'\\', L'/' ) ;
		struct stat	st ;
		if ( stat( strPath.EncodeDefaultTo(bufPath), &st ) == 0 )
		{
			state.bitFields = SFileOpener::fieldAttributes
							| SFileOpener::fieldFileSize ;
			state.bitAttributes = 0 ;
			if ( S_ISDIR(st.st_mode) )
			{
				state.bitAttributes |= SFileOpener::attrDirectory ;
			}
			if ( st.st_mode & S_IRUSR )
			{
				state.bitAttributes |= SFileOpener::permissionRUSR ;
			}
			if ( st.st_mode & S_IWUSR )
			{
				state.bitAttributes |= SFileOpener::permissionWUSR ;
			}
			if ( st.st_mode & S_IXUSR )
			{
				state.bitAttributes |= SFileOpener::permissionXUSR ;
			}
			if ( st.st_mode & S_IRGRP )
			{
				state.bitAttributes |= SFileOpener::permissionRGRP ;
			}
			if ( st.st_mode & S_IWGRP )
			{
				state.bitAttributes |= SFileOpener::permissionWGRP ;
			}
			if ( st.st_mode & S_IXGRP )
			{
				state.bitAttributes |= SFileOpener::permissionXGRP ;
			}
			if ( st.st_mode & S_IROTH )
			{
				state.bitAttributes |= SFileOpener::permissionROTH ;
			}
			if ( st.st_mode & S_IWOTH )
			{
				state.bitAttributes |= SFileOpener::permissionWOTH ;
			}
			if ( st.st_mode & S_IXOTH )
			{
				state.bitAttributes |= SFileOpener::permissionXOTH ;
			}
			state.nFileSize = st.st_size ;
			//
			tm		tmLocal ;
			tm *	ptmLocal ;
			ptmLocal = localtime_r
				( (const time_t*) &st.st_atime, &tmLocal ) ;
			if ( ptmLocal != NULL )
			{
				state.bitFields |= SFileOpener::fieldAccessedTime ;
				state.dtAccessed.nYear = (uint16_t) (ptmLocal->tm_year + 1900) ;
				state.dtAccessed.nMonth = (uint16_t) (ptmLocal->tm_mon + 1) ;
				state.dtAccessed.nDay = (uint16_t) (ptmLocal->tm_mday) ;
				state.dtAccessed.nWeek = (uint16_t) (ptmLocal->tm_wday) ;
				state.dtAccessed.nHour = (uint16_t) (ptmLocal->tm_hour) ;
				state.dtAccessed.nMinute = (uint16_t) (ptmLocal->tm_min) ;
				state.dtAccessed.nSecond = (uint16_t) (ptmLocal->tm_sec) ;
				state.dtAccessed.nMilliSec = 0 ;
			}
			ptmLocal = localtime_r
				( (const time_t*) &st.st_mtime, &tmLocal ) ;
			if ( ptmLocal != NULL )
			{
				state.bitFields |= SFileOpener::fieldModifiedTime ;
				state.dtModified.nYear = (uint16_t) (ptmLocal->tm_year + 1900) ;
				state.dtModified.nMonth = (uint16_t) (ptmLocal->tm_mon + 1) ;
				state.dtModified.nDay = (uint16_t) (ptmLocal->tm_mday) ;
				state.dtModified.nWeek = (uint16_t) (ptmLocal->tm_wday) ;
				state.dtModified.nHour = (uint16_t) (ptmLocal->tm_hour) ;
				state.dtModified.nMinute = (uint16_t) (ptmLocal->tm_min) ;
				state.dtModified.nSecond = (uint16_t) (ptmLocal->tm_sec) ;
				state.dtModified.nMilliSec = 0 ;
			}
			ptmLocal = localtime_r
				( (const time_t*) &st.st_ctime, &tmLocal ) ;
			if ( ptmLocal != NULL )
			{
				state.bitFields |= SFileOpener::fieldCreatedTime ;
				state.dtCreated.nYear = (uint16_t) (ptmLocal->tm_year + 1900) ;
				state.dtCreated.nMonth = (uint16_t) (ptmLocal->tm_mon + 1) ;
				state.dtCreated.nDay = (uint16_t) (ptmLocal->tm_mday) ;
				state.dtCreated.nWeek = (uint16_t) (ptmLocal->tm_wday) ;
				state.dtCreated.nHour = (uint16_t) (ptmLocal->tm_hour) ;
				state.dtCreated.nMinute = (uint16_t) (ptmLocal->tm_min) ;
				state.dtCreated.nSecond = (uint16_t) (ptmLocal->tm_sec) ;
				state.dtCreated.nMilliSec = 0 ;
			}
			return	errSuccess ;
		}

	#endif
	return	errFailed ;
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RemoveFile( const wchar_t * pszFilePath )
{
	#if	defined(__COTOPHA__)
		return	File::RemoveFile( pszFilePath ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			if ( ::DeleteFileW( pszFilePath ) )
			{
				return	errSuccess ;
			}
		}
		else
		{
			SArray<char>	bufPath ;
			SString			strPath = pszFilePath ;
			if ( ::DeleteFileA( strPath.EncodeDefaultTo(bufPath) ) )
			{
				return	errSuccess ;
			}
		}

	#else
		SString			strPath = pszFilePath ;
		SArray<char>	bufPath ;
		strPath.Replace( L'\\', L'/' ) ;
		if ( remove( strPath.EncodeDefaultTo(bufPath) ) == 0 )
		{
			return	errSuccess ;
		}
		ESLTrace( "failed to remove \'%s\'\n", bufPath.GetConstArray() ) ;

	#endif
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFile::ListFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	SFile	file ;
	file.ListSubFiles( listFiles, pszDirPath ) ;
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SFile::ListDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	SFile	file ;
	file.ListSubDirectories( listDirs, pszDirPath ) ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SFile::CreateDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	#if	defined(__COTOPHA__)
		return	File::CreateDirectory( pszPath, nFlags ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			if ( ::CreateDirectoryW( pszPath, NULL ) )
			{
				return	errSuccess ;
			}
		}
		else
		{
			SArray<char>	bufPath ;
			SString			strPath = pszPath ;
			if ( ::CreateDirectoryA
				( strPath.EncodeDefaultTo(bufPath), NULL ) )
			{
				return	errSuccess ;
			}
		}
		return	errFailed ;

	#else
		mode_t	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;
		if ( nFlags != 0 )
		{
			mode = 0 ;
			if ( nFlags & SFileOpener::permissionRUSR )
			{
				mode |= S_IRUSR ;
			}
			if ( nFlags & SFileOpener::permissionWUSR )
			{
				mode |= S_IWUSR ;
			}
			if ( nFlags & SFileOpener::permissionXUSR )
			{
				mode |= S_IXUSR ;
			}
			if ( nFlags & SFileOpener::permissionRGRP )
			{
				mode |= S_IRGRP ;
			}
			if ( nFlags & SFileOpener::permissionWGRP )
			{
				mode |= S_IWGRP ;
			}
			if ( nFlags & SFileOpener::permissionXGRP )
			{
				mode |= S_IXGRP ;
			}
			if ( nFlags & SFileOpener::permissionROTH )
			{
				mode |= S_IROTH ;
			}
			if ( nFlags & SFileOpener::permissionWOTH )
			{
				mode |= S_IWOTH ;
			}
			if ( nFlags & SFileOpener::permissionXOTH )
			{
				mode |= S_IXOTH ;
			}
		}
		SString			strPath = pszPath ;
		SArray<char>	bufPath ;
		strPath.Replace( L'\\', L'/' ) ;
		int	err = mkdir( strPath.EncodeDefaultTo(bufPath), mode ) ;
		if ( err == 0 )
		{
			return	errSuccess ;
		}
		ESLTrace( "failed to mkdir \'%s\' %03X (error=%d, %s)\n",
						bufPath.GetConstArray(), mode, errno, strerror(errno) ) ;
		return	errFailed ;

	#endif
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RemoveDirectory( const wchar_t * pszPath )
{
	#if	defined(__COTOPHA__)
		return	File::RemoveDirectory( pszPath ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			if ( ::RemoveDirectoryW( pszPath ) )
			{
				return	errSuccess ;
			}
		}
		else
		{
			SArray<char>	bufPath ;
			SString			strPath = pszPath ;
			if ( ::RemoveDirectoryA
				( strPath.EncodeDefaultTo(bufPath) ) )
			{
				return	errSuccess ;
			}
		}
		return	errFailed ;

	#else
		SString			strPath = pszPath ;
		SArray<char>	bufPath ;
		strPath.Replace( L'\\', L'/' ) ;
		if ( rmdir( strPath.EncodeDefaultTo(bufPath) ) == 0 )
		{
			return	errSuccess ;  
		}
		ESLTrace( "failed to rmdir \'%s\'\n", bufPath.GetConstArray() ) ;
		return	errFailed ;

	#endif
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SFile::RenameFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	#if	defined(__COTOPHA__)
		return	File::RenameFile( pszOldPath, pszNewPath ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( g_infoPlatform.runtimeOS != platformOS_Windows )
		{
			if ( ::MoveFileW( pszOldPath, pszNewPath ) )
			{
				return	errSuccess ;
			}
		}
		else
		{
			SArray<char>	bufOldPath, bufNewPath ;
			if ( ::MoveFileA
				( SString(pszOldPath).EncodeDefaultTo(bufOldPath),
					SString(pszNewPath).EncodeDefaultTo(bufNewPath) ) )
			{
				return	errSuccess ;
			}
		}
		return	errFailed ;

	#else
		SArray<char>	bufOldPath, bufNewPath ;
		SString			strOldPath = pszOldPath ;
		SString			strNewPath = pszNewPath ;
		strOldPath.Replace( L'\\', L'/' ) ;
		strNewPath.Replace( L'\\', L'/' ) ;
		if ( rename
			( strOldPath.EncodeDefaultTo(bufOldPath),
				strNewPath.EncodeDefaultTo(bufNewPath) ) == 0 )
		{
			return	errSuccess ;  
		}
		ESLTrace( "failed to rename \'%s\' to \'%s\'\n",
					bufOldPath.GetArray(), bufNewPath.GetConstArray() ) ;
		return	errFailed ;

	#endif
}

// 規定ディレクトリ取得
//////////////////////////////////////////////////////////////////////////////
SError SFile::GetDefaultDirectory
	( SString& strDirPath,
		const wchar_t * pwszPlacementId, const wchar_t * pwszOption )
{
	#if	defined(__COTOPHA__)
		return	File::GetDefaultDirectory
					( strDirPath, pwszPlacementId, pwszOption ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		SArray<wchar_t>	bufPath ;
		wchar_t *		pwszPath ;
		bufPath.SetLength( MAX_PATH + 1 ) ;
		pwszPath = bufPath.GetArray() ;
		//
		if ( SString::Compare
			( pwszPlacementId,
				DefaultDirectory::CurrentDirectory ) == 0 )
		{
			::GetCurrentDirectoryW( MAX_PATH, pwszPath ) ;
			strDirPath = pwszPath ;
			return	errSuccess ;
		}
		else if ( SString::Compare
			( pwszPlacementId,
				DefaultDirectory::WindowsDirectory ) == 0 )
		{
			::GetWindowsDirectoryW( pwszPath, MAX_PATH ) ;
			strDirPath = pwszPath ;
			return	errSuccess ;
		}
		else if ( SString::Compare
			( pwszPlacementId,
				DefaultDirectory::WindowsSystemDirectory ) == 0 )
		{
			::GetSystemDirectoryW( pwszPath, MAX_PATH ) ;
			strDirPath = pwszPath ;
			return	errSuccess ;
		}
		else
		{
			HKEY			hKeyRoot = HKEY_CURRENT_USER ;
			const wchar_t *	pszRegPath =
								L"Software\\Microsoft\\Windows\\"
								L"CurrentVersion\\Explorer\\Shell Folders" ;
			const wchar_t *	pszRegName = NULL ;
			SString			strRegPath ;
			if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::WindowsStartMenu ) == 0 )
			{
				pszRegName = L"Programs" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::WindowsDesktop ) == 0 )
			{
				pszRegName = L"Desktop" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::ApplicationData ) == 0 )
			{
				pszRegName = L"AppData" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::UserDocuments ) == 0 )
			{
				pszRegName = L"Personal" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::UserMusic ) == 0 )
			{
				pszRegName = L"My Music" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::UserPictures ) == 0 )
			{
				pszRegName = L"My Pictures" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::UserVideos ) == 0 )
			{
				pszRegName = L"My Video" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::WindowsCommonStartMenu ) == 0 )
			{
				hKeyRoot = HKEY_LOCAL_MACHINE ;
				pszRegName = L"Common Programs" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::WindowsProgramFiles ) == 0 )
			{
				hKeyRoot = HKEY_LOCAL_MACHINE ;
				pszRegPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion" ;
				pszRegName = L"ProgramFilesDir" ;
			}
			else if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::ApplicationInstalled ) == 0 )
			{
				hKeyRoot = HKEY_LOCAL_MACHINE ;
				strRegPath = L"SOFTWARE\\Microsoft\\Windows\\"
								L"CurrentVersion\\Uninstall\\" ;
				strRegPath += pwszOption ;
				pszRegPath = strRegPath ;
				pszRegName = L"InstallLocation" ;
			}
			if ( pszRegName != NULL )
			{
				SRegistryKey	key ;
				if ( !key.OpenKey( hKeyRoot, pszRegPath ) )
				{
					strDirPath = key.GetString( pszRegName ) ;
					return	errSuccess ;
				}
			}
		}
		bufPath.FinishArray() ;

	#elif	defined(__PLATFORM_ANDROID__)
		if ( SString::Compare
				( pwszPlacementId,
					DefaultDirectory::ApplicationData ) == 0 )
		{
			JNI::GetAndroidLocalFilesDirectory( strDirPath ) ;
			return	errSuccess ;
		}
		else if ( SString::Compare
					( pwszPlacementId,
						DefaultDirectory::AndroidLocalFiles ) == 0 )
		{
			JNI::GetAndroidLocalFilesDirectory( strDirPath ) ;
			return	errSuccess ;
		}
		else if ( SString::Compare
					( pwszPlacementId,
						DefaultDirectory::AndroidExternalStorage ) == 0 )
		{
			JNI::GetAndroidStorageDirectory( strDirPath ) ;
			return	errSuccess ;
		}
		else if ( SString::Compare
					( pwszPlacementId,
						DefaultDirectory::AndroidExternalStoragePrivate ) == 0 )
		{
			JNI::GetAndroidStoragePrivateDirectory( strDirPath ) ;
			return	errSuccess ;
		}

	#endif

	return	errFailed ;
}

// 指定ディレクトリパスのディレクトリを作成する（親ディレクトリを含む）
//////////////////////////////////////////////////////////////////////////////
SError SFile::CreateFullDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	SString	strPath = pszPath ;
	for ( ; ; )
	{
		wchar_t	wchLast = strPath.GetLastAt(0) ;
		if ( (wchLast == L'\\') || (wchLast == L'/') )
		{
			ESLAssert( strPath.GetLength() >= 1 ) ;
			strPath.SetLength( strPath.GetLength() - 1 ) ;
		}
		else if ( (wchLast == 0) || (wchLast == L':') )
		{
			return	errSuccess ;
		}
		else
		{
			break ;
		}
	}
	if ( IsExistingAbsPath( strPath ) )
	{
		// ディレクトリ（orファイル）は存在
		return	errSuccess ;
	}
	//
	// 親ディレクトリ作成
	//
	CreateFullDirectory( strPath.GetFileDirectoryPart(), nFlags ) ;
	//
	// 現ディレクトリ作成
	//
	return	CreateDirectory( strPath, nFlags ) ;
}


//////////////////////////////////////////////////////////////////////////////
// メモリ参照・ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SMemoryReferenceFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SMemoryReferenceFile::SMemoryReferenceFile( void )
{
	m_pbytMemory = NULL ;
	m_nLength = 0 ;
	m_nPosition = 0 ;

#if	defined(__COTOPHA__)
	m_pMemFile = NULL ;
#endif
}

SMemoryReferenceFile::SMemoryReferenceFile( const SMemoryReferenceFile& memfile )
{
	m_pbytMemory = memfile.m_pbytMemory ;
	m_nLength = memfile.m_nLength ;
	m_nPosition = memfile.m_nPosition ;

#if	defined(__COTOPHA__)
	m_pMemFile = NULL ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SMemoryReferenceFile::~SMemoryReferenceFile( void )
{
#if	defined(__COTOPHA__)
	delete	m_pMemFile ;
	m_pMemFile = NULL ;
#endif
}

// メモリ関連付け
//////////////////////////////////////////////////////////////////////////////
void SMemoryReferenceFile::AttachMemory( void * ptrMemory, size_t nLength )
{
	m_pbytMemory = (uint8_t*) ptrMemory ;
	m_nLength = nLength ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SMemoryReferenceFile&
	SMemoryReferenceFile::operator = ( const SMemoryReferenceFile& mem )
{
	m_pbytMemory = mem.m_pbytMemory ;
	m_nLength = mem.m_nLength ;
	m_nPosition = mem.m_nPosition ;
	return	*this ;
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SMemoryReferenceFile::GetFileObject( void )
{
	if ( m_pMemFile == NULL )
	{
		m_pMemFile = new MemoryReferenceFile ;
	}
	m_pMemFile->AttachMemory( m_pbytMemory, m_nLength ) ;
	m_pMemFile->Seek( m_nPosition ) ;
	return	m_pMemFile ;
}
#endif

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SMemoryReferenceFile::Duplicate( void ) const
{
	SMemoryReferenceFile *	pfile = new SMemoryReferenceFile ;
	pfile->AttachMemory( m_pbytMemory, m_nLength ) ;
	pfile->Seek( m_nPosition ) ;
	return	pfile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SMemoryReferenceFile::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_nPosition >= m_nLength )
	{
		return	0 ;
	}
	if ( m_nPosition + nBytes > m_nLength )
	{
		nBytes = m_nLength - m_nPosition ;
	}
	memmove( ptrBuf, m_pbytMemory + m_nPosition, nBytes ) ;
	m_nPosition += nBytes ;
	return	nBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SMemoryReferenceFile::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_nPosition >= m_nLength )
	{
		return	0 ;
	}
	if ( m_nPosition + nBytes > m_nLength )
	{
		nBytes = m_nLength - m_nPosition ;
	}
	memmove( m_pbytMemory + m_nPosition, ptrBuf, nBytes ) ;
	m_nPosition += nBytes ;
	return	nBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SMemoryReferenceFile::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SMemoryReferenceFile::GetLength( void ) const
{
	return	m_nLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SMemoryReferenceFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	FromBegin:
		break ;
	case	FromCurrent:
		posFile += m_nPosition ;
		break ;
	case	FromEnd:
		posFile += m_nLength ;
		break ;
	}
	if ( posFile >= (int64_t) m_nLength )
	{
		posFile = m_nLength ;
	}
	else if ( posFile < 0 )
	{
		posFile = 0 ;
	}
	m_nPosition = (size_t) posFile ;
	return	m_nPosition ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SMemoryReferenceFile::GetPosition( void ) const
{
	return	m_nPosition ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SMemoryReferenceFile::SetEndOfFile( void )
{
	if ( m_nPosition <= m_nLength )
	{
		m_nLength = m_nPosition ;
		return	errSuccess ;
	}
	return	errFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// URL オープン・インターフェース
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL(SVirtualURLOpener	SSystem::g_defURLOpener) ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SVirtualURLOpener, SFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SVirtualURLOpener::SVirtualURLOpener( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SVirtualURLOpener::~SVirtualURLOpener( void )
{
	SVirtualURLOpener::UnregisterAllScheme() ;
}

// スキーム判定
//////////////////////////////////////////////////////////////////////////////
ssize_t SVirtualURLOpener::FindScheme( const wchar_t * pszFilePath ) const
{
	size_t	countSheme = m_vectorScheme.GetLength() ;
	for ( size_t i = 0; i < countSheme; i ++ )
	{
		SCHEME&	scheme = m_vectorScheme.At(i) ;
		if ( SString::CompareLeftNoCase( pszFilePath, scheme.pwszName ) == 0 )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// スキーム取得
//////////////////////////////////////////////////////////////////////////////
const SVirtualURLOpener::SCHEME *
	SVirtualURLOpener::GetSchemeAt( size_t iScheme ) const
{
	return	m_vectorScheme.GetAt( iScheme ) ;
}

// スキームを除去したパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SVirtualURLOpener::GetRidPathOfScheme
	( const wchar_t * pszFilePath, const SCHEME& scheme )
{
	size_t	lenScheme = SString::GetLength(scheme.pwszName) ;
	while ( pszFilePath
		&& ((pszFilePath[lenScheme] == L'/')
			|| (pszFilePath[lenScheme] == L'\\')) )
	{
		lenScheme ++ ;
	}
	return	pszFilePath + lenScheme ;
}

// スキーム追加登録
//////////////////////////////////////////////////////////////////////////////
void SVirtualURLOpener::RegisterScheme
	( const wchar_t * pwszName, SFileOpener * pOpener, uint32_t nFlags )
{
	SCHEME	scheme ;
	scheme.pwszName = pwszName ;
	scheme.nFlags = nFlags ;
	scheme.pOpener = pOpener ;
	//
	m_vectorScheme.Add( scheme ) ;
}

// スキーム全削除
//////////////////////////////////////////////////////////////////////////////
void SVirtualURLOpener::UnregisterAllScheme( void )
{
	size_t	countSheme = m_vectorScheme.GetLength() ;
	for ( size_t i = 0; i < countSheme; i ++ )
	{
		SCHEME&	scheme = m_vectorScheme.At(i) ;
		delete	scheme.pOpener ;
		scheme.pOpener = NULL ;
	}
	m_vectorScheme.FreeArray() ;
}

// オフセット・オープナー生成
//////////////////////////////////////////////////////////////////////////////
SFileOpener * SVirtualURLOpener::NewOffsetOpener
			( const wchar_t * pszFilePath, wchar_t wchSeparator )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	new SOffsetFileOpener
				( GetRidPathOfScheme(pszFilePath,scheme),
								wchSeparator, pOpener, false ) ;
		}
	}
	return	new SOffsetFileOpener
		( pszFilePath, wchSeparator, new SStandardFileOpener, true ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SVirtualURLOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			SFileInterface *
				pFile = pOpener->NewOpenFile
					( GetRidPathOfScheme(pszFilePath,scheme), nOpenFlags ) ;
			if ( pFile != NULL )
			{
				return	pFile ;
			}
		}
	}
	return	SFile::NewOpen( pszFilePath, nOpenFlags ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SVirtualURLOpener::IsExisting( const wchar_t * pszFilePath )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			if ( pOpener->IsExisting
					( GetRidPathOfScheme(pszFilePath,scheme) ) )
			{
				return	true ;
			}
		}
	}
	return	SFile::IsExistingFile( pszFilePath ) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			SError	err =
				pOpener->QueryState
					( GetRidPathOfScheme(pszFilePath,scheme), state ) ;
			if ( !err )
			{
				return	err ;
			}
		}
	}
	return	SFile::QueryFileState( pszFilePath, state ) ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SVirtualURLOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	ssize_t	iScheme = FindScheme( pszDirPath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			pOpener->ListSubFiles
				( listFiles, GetRidPathOfScheme(pszDirPath,scheme) ) ;
			return ;
		}
	}
	SFile::ListFiles( listFiles, pszDirPath ) ;
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SVirtualURLOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	ssize_t	iScheme = FindScheme( pszDirPath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			pOpener->ListSubDirectories
				( listDirs, GetRidPathOfScheme(pszDirPath,scheme) ) ;
			return ;
		}
	}
	SFile::ListDirectories( listDirs, pszDirPath ) ;
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::RemoveSubFile( const wchar_t * pszFilePath )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	pOpener->RemoveSubFile
						( GetRidPathOfScheme(pszFilePath,scheme) ) ;
		}
	}
	return	SFile::RemoveFile( pszFilePath ) ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	ssize_t	iScheme = FindScheme( pszPath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	pOpener->CreateSubDirectory
						( GetRidPathOfScheme(pszPath,scheme), nFlags ) ;
		}
	}
	return	SFile::CreateDirectory( pszPath, nFlags ) ;
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::RemoveSubDirectory( const wchar_t * pszPath )
{
	ssize_t	iScheme = FindScheme( pszPath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	pOpener->RemoveSubDirectory
						( GetRidPathOfScheme(pszPath,scheme) ) ;
		}
	}
	return	SFile::RemoveDirectory( pszPath ) ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pszNewPath )
{
	ssize_t	iOldScheme = FindScheme( pszOldPath ) ;
	ssize_t	iNewScheme = FindScheme( pszNewPath ) ;
	if ( (iOldScheme >= 0) && (iNewScheme == iOldScheme) )
	{
		SCHEME&	scheme = m_vectorScheme.At(iOldScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	pOpener->RenameSubFile
						( GetRidPathOfScheme(pszOldPath,scheme),
							GetRidPathOfScheme(pszNewPath,scheme) ) ;
		}
	}
	return	SFile::RenameFile( pszOldPath, pszNewPath ) ;
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SVirtualURLOpener::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	ssize_t	iScheme = FindScheme( pszFilePath ) ;
	if ( iScheme >= 0 )
	{
		SCHEME&	scheme = m_vectorScheme.At(iScheme) ;
		SFileOpener *	pOpener = scheme.pOpener ;
		if ( pOpener != NULL )
		{
			return	pOpener->DirectPathOf
						( strDirectPath,
							GetRidPathOfScheme(pszFilePath,scheme) ) ;
		}
	}
	strDirectPath = pszFilePath ;
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準ファイル・オープン・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SStandardFileOpener, SFileOpener )

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SStandardFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
#if	defined(__COTOPHA__)
	return	SFile::NewOpenFileAbsPath( pszFilePath, nOpenFlags ) ;
#else
	return	g_defURLOpener.NewOpenFile( pszFilePath, nOpenFlags ) ;
#endif
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SStandardFileOpener::IsExisting( const wchar_t * pszFilePath )
{
#if	defined(__COTOPHA__)
	return	SFile::IsExistingAbsPath( pszFilePath ) ;
#else
	return	g_defURLOpener.IsExisting( pszFilePath ) ;
#endif
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
#if	defined(__COTOPHA__)
	return	SFile::QueryFileState( pszFilePath, state ) ;
#else
	return	g_defURLOpener.QueryState( pszFilePath, state ) ;
#endif
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SStandardFileOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
#if	defined(__COTOPHA__)
	SFile::ListSubFilesAbsPath( listFiles, pszDirPath ) ;
#else
	g_defURLOpener.ListSubFiles( listFiles, pszDirPath ) ;
#endif
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SStandardFileOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
#if	defined(__COTOPHA__)
	SFile::ListSubDirectoriesAbsPath( listDirs, pszDirPath ) ;
#else
	g_defURLOpener.ListSubDirectories( listDirs, pszDirPath ) ;
#endif
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::RemoveSubFile( const wchar_t * pszFilePath )
{
#if	defined(__COTOPHA__)
	return	SFile::RemoveFile( pszFilePath ) ;
#else
	return	g_defURLOpener.RemoveSubFile( pszFilePath ) ;
#endif
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
#if	defined(__COTOPHA__)
	return	SFile::CreateDirectory( pszPath, nFlags ) ;
#else
	return	g_defURLOpener.CreateSubDirectory( pszPath, nFlags ) ;
#endif
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::RemoveSubDirectory( const wchar_t * pszPath )
{
#if	defined(__COTOPHA__)
	return	SFile::RemoveDirectory( pszPath ) ;
#else
	return	g_defURLOpener.RemoveSubDirectory( pszPath ) ;
#endif
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pwszNewPath )
{
#if	defined(__COTOPHA__)
	return	SFile::RenameFile( pszOldPath, pwszNewPath ) ;
#else
	return	g_defURLOpener.RenameSubFile( pszOldPath, pwszNewPath ) ;
#endif
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SStandardFileOpener::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	strDirectPath = pszFilePath ;
#if	defined(__PLATFORM_UNIX_LIKE__)
	strDirectPath.Replace( L'\\', L'/' ) ;
#endif
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 相対パス・オープン・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SOffsetFileOpener, SFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SOffsetFileOpener::SOffsetFileOpener
	( const wchar_t * pszBasePath,
		wchar_t wchSeparator, SFileOpener * pOpener, bool flagOwner )
	: m_strBasePath( pszBasePath ),
		m_wchSeparator( wchSeparator ),
		m_pOpener( pOpener ), m_flagOwner( flagOwner )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SOffsetFileOpener::~SOffsetFileOpener( void )
{
	if ( m_flagOwner )
	{
		delete	m_pOpener ;
		m_pOpener = NULL ;
		m_flagOwner = false ;
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SOffsetFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	if ( m_pOpener == NULL )
	{
		return	NULL ;
	}
	return	m_pOpener->NewOpenFile( OffsetPath( pszFilePath ), nOpenFlags ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SOffsetFileOpener::IsExisting( const wchar_t * pszFilePath )
{
	if ( (m_pOpener == NULL)
		|| (pszFilePath == NULL) || (pszFilePath[0] == 0) )
	{
		return	false ;
	}
	return	m_pOpener->IsExisting( OffsetPath( pszFilePath ) ) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->QueryState( OffsetPath( pszFilePath ), state ) ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SOffsetFileOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	if ( m_pOpener == NULL )
	{
		return ;
	}
	m_pOpener->ListSubFiles( listFiles, OffsetPath( pszDirPath ) ) ;
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SOffsetFileOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	if ( m_pOpener == NULL )
	{
		return ;
	}
	m_pOpener->ListSubDirectories( listDirs, OffsetPath( pszDirPath ) ) ;
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::RemoveSubFile( const wchar_t * pszFilePath )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->RemoveSubFile( OffsetPath( pszFilePath ) ) ;
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pwszNewPath )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->RenameSubFile
			( OffsetPath( pszOldPath ), OffsetPath( pwszNewPath ) ) ;
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->DirectPathOf
				( strDirectPath, OffsetPath( pszFilePath ) ) ;
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->CreateSubDirectory( OffsetPath( pszPath ), nFlags ) ;
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SOffsetFileOpener::RemoveSubDirectory( const wchar_t * pszPath )
{
	if ( m_pOpener == NULL )
	{
		return	errFailed ;
	}
	return	m_pOpener->RemoveSubDirectory( OffsetPath( pszPath ) ) ;
}

// オフセットパスを取得する
//////////////////////////////////////////////////////////////////////////////
SString SOffsetFileOpener::OffsetPath( const wchar_t * pszFilePath )
{
	if ( pszFilePath != NULL )
	{
		while ( (pszFilePath[0] == L'\\')
			|| (pszFilePath[0] == L'/')
			|| (pszFilePath[0] == m_wchSeparator) )
		{
			pszFilePath ++ ;
		}
	}
	return	m_strBasePath.OffsetFilePath( pszFilePath, m_wchSeparator ) ;
}

// ベースパスを取得する
//////////////////////////////////////////////////////////////////////////////
SString SOffsetFileOpener::GetFullBasePath( void ) const
{
	SOffsetFileOpener *	pOffsetOpener =
			ESLTypeCast<SOffsetFileOpener>( m_pOpener ) ;
	if ( pOffsetOpener == NULL )
	{
		return	m_strBasePath ;
	}
	return	pOffsetOpener->GetFullBasePath().
				OffsetFilePath( m_strBasePath, m_wchSeparator ) ;
}


//////////////////////////////////////////////////////////////////////////////
// オープナー参照ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSmartFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSmartFile::SSmartFile
	( SFileOpener * pOpener, SFileInterface * pFile, bool flagOwner )
	: m_refOpener( pOpener ), m_pFile( pFile ), m_flagOwner( flagOwner )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSmartFile::~SSmartFile( void )
{
	SSmartFile::Close() ;
}

// ファイルオープナーを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SSmartFile::AttachFileOpener( SFileOpener * pOpener )
{
	m_refOpener.SetReference( pOpener ) ;
}

// ファイルを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SSmartFile::AttachFile
	( SFileInterface * pFile, bool flagOwner )
{
	Close() ;
	//
	m_pFile = pFile ;
	m_flagOwner = flagOwner ;
}

// ファイルの参照を解除する
//////////////////////////////////////////////////////////////////////////////
void SSmartFile::Close( void )
{
	if ( m_flagOwner )
	{
		delete	m_pFile ;
		m_flagOwner = false ;
	}
	m_pFile = NULL ;
}

// SFileOpener 取得
//////////////////////////////////////////////////////////////////////////////
SFileOpener * SSmartFile::GetFileOpener( void ) const
{
	return	ESLTypeCast<SFileOpener>( m_refOpener.GetReference() ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SSmartFile::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->NewOpenFile( pszFilePath, nOpenFlags ) ;
	}
	return	SFileInterface::NewOpenFile( pszFilePath, nOpenFlags ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SSmartFile::IsExisting( const wchar_t * pszFilePath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->IsExisting( pszFilePath ) ;
	}
	return	SFileInterface::IsExisting( pszFilePath ) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->QueryState( pszFilePath, state ) ;
	}
	return	SFileInterface::QueryState( pszFilePath, state ) ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SSmartFile::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		pOpener->ListSubFiles( listFiles, pszDirPath ) ;
	}
	else
	{
		SFileInterface::ListSubFiles( listFiles, pszDirPath ) ;
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SSmartFile::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		pOpener->ListSubFiles( listDirs, pszDirPath ) ;
	}
	else
	{
		SFileInterface::ListSubFiles( listDirs, pszDirPath ) ;
	}
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::RemoveSubFile( const wchar_t * pszFilePath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->RemoveSubFile( pszFilePath ) ;
	}
	else
	{
		return	SFileInterface::RemoveSubFile( pszFilePath ) ;
	}
}

// ファイル名を変更する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::RenameSubFile
	( const wchar_t * pszOldPath, const wchar_t * pwszNewPath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->RenameSubFile( pszOldPath, pwszNewPath ) ;
	}
	else
	{
		return	SFileInterface::RenameSubFile( pszOldPath, pwszNewPath ) ;
	}
}

// システム上の直接パスを取得する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::DirectPathOf
	( SString& strDirectPath, const wchar_t * pszFilePath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->DirectPathOf( strDirectPath, pszFilePath ) ;
	}
	else
	{
		return	SFileInterface::DirectPathOf( strDirectPath, pszFilePath ) ;
	}
}

// ディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::CreateSubDirectory
	( const wchar_t * pszPath, long int nFlags )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->CreateSubDirectory( pszPath, nFlags ) ;
	}
	else
	{
		return	SFileInterface::CreateSubDirectory( pszPath, nFlags ) ;
	}
}

// ディレクトリを削除する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::RemoveSubDirectory( const wchar_t * pszPath )
{
	SFileOpener *	pOpener = GetFileOpener() ;
	if ( pOpener != NULL )
	{
		return	pOpener->RemoveSubDirectory( pszPath ) ;
	}
	else
	{
		return	SFileInterface::RemoveSubDirectory( pszPath ) ;
	}
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SSmartFile::Duplicate( void ) const
{
	if ( m_pFile == NULL )
	{
		return	NULL ;
	}
	return	new SSmartFile( GetFileOpener(), m_pFile->Duplicate(), true ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartFile::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartFile::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->Write( ptrBuf, nBytes ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SSmartFile::IsSeekable( void ) const
{
	if ( m_pFile == NULL )
	{
		return	false ;
	}
	return	m_pFile->IsSeekable() ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartFile::GetLength( void ) const
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->Seek( posFile, seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartFile::GetPosition( void ) const
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SSmartFile::SetEndOfFile( void )
{
	if ( m_pFile == NULL )
	{
		return	errFailed ;
	}
	return	m_pFile->SetEndOfFile() ;
}

