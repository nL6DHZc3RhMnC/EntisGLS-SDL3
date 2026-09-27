
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2021 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_bitmap_font.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// EFile -> SFileOpener 変換インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSEnvironment::EFileOpener, SFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::EFileOpener::EFileOpener( EFile * pFile, bool flagOwner )
{
	m_pFile = pFile ;
	m_flagOwner = flagOwner ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::EFileOpener::~EFileOpener( void )
{
	if ( m_flagOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagOwner = false ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * ECSEnvironment::EFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	if ( m_pFile->m_fDisabled || (nOpenFlags & ESLFileObject::modeWrite) )
	{
		return	NULL ;
	}
	if ( m_pFile->m_fArchive )
	{
		m_pFile->m_cs.Lock() ;
		if ( (m_pFile->m_pOwnFile == NULL)
			&& !m_pFile->m_file.IsFileOpened() )
		{
			if ( !m_pFile->m_file.Open
				( m_pFile->m_strDirPath, ESLFileObject::modeRead
										| ESLFileObject::shareRead ) )
			{
				if ( m_pFile->Open( &(m_pFile->m_file) ) )
				{
					m_pFile->Close( ) ;
					m_pFile->m_file.Close( ) ;
				}
			}
		}
		if ( (m_pFile->m_pOwnFile != NULL)
			|| m_pFile->m_file.IsFileOpened() )
		{
			if ( !m_pFile->OpenFile
				( SString(pszFilePath).ToCharArray(),
								m_pFile->m_strPassword ) )
			{
				size_t			nBytes = (size_t) m_pFile->GetLength() ;
				SByteBuffer *	pbuf = new SByteBuffer ;
				m_pFile->Read
					( pbuf->GetArray(nBytes), (unsigned long) nBytes ) ;
				pbuf->FinishArray() ;
				m_pFile->AscendFile( ) ;
				m_pFile->m_cs.Unlock() ;
				return	new SSmartFile( this, pbuf, true ) ;
			}
		}
		m_pFile->m_cs.Unlock() ;
	}
	else
	{
		SString	strDir = m_pFile->m_strDirPath ;
		SString	strFile = strDir.OffsetFilePath( pszFilePath ) ;
		SFile *	pFile = new SFile ;
		if ( pFile->Open( strFile, SFileOpener::shareRead ) )
		{
			delete	pFile ;
			return	NULL ;
		}
		return	pFile ;
	}
	return	NULL ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool ECSEnvironment::EFileOpener::IsExisting( const wchar_t * pszFilePath )
{
	SFileInterface *
		pfile = NewOpenFile( pszFilePath, ESLFileObject::modeRead ) ;
	if ( pfile != NULL )
	{
		delete	pfile ;
		return	true ;
	}
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError ECSEnvironment::EFileOpener::QueryState
	( const wchar_t * pszFilePath, State& state )
{
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::EFileOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	if ( m_pFile->m_fArchive )
	{
		m_pFile->m_cs.Lock() ;
		if ( (m_pFile->m_pOwnFile != NULL)
			|| m_pFile->m_file.IsFileOpened() )
		{
			m_pFile->OpenDirectory( EString(pszDirPath) ) ;

			ERISAArchive::EDirectory&
					dirEntries = m_pFile->ReferFileEntries() ;
			for ( size_t i = 0; i < dirEntries.GetSize(); i ++ )
			{
				ERISAArchive::EFileName *	pFileName = dirEntries.GetTagAt( i ) ;
				ERISAArchive::FILE_INFO *	pFileInfo = dirEntries.GetObjectAt( i ) ;
				if ( pFileInfo->dwAttribute & ERISAArchive::attrDirectory )
				{
					continue ;
				}
				if ( pFileInfo->dwAttribute & ERISAArchive::attrFileNameUTF8 )
				{
					SString *	pStrFileName = new SString ;
					Charset::Decode
						( *pStrFileName,
							Charset::encodingUTF8,
							(const uint8_t*) pFileName->CharPtr(),
							(ssize_t) pFileName->GetLength() ) ;
					listFiles.Add( pStrFileName ) ;
				}
				else
				{
					listFiles.Add( new SString(*pFileName) ) ;
				}
			}
		}
		m_pFile->m_cs.Unlock() ;
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::EFileOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	if ( m_pFile->m_fArchive )
	{
		m_pFile->m_cs.Lock() ;
		if ( (m_pFile->m_pOwnFile != NULL)
			|| m_pFile->m_file.IsFileOpened() )
		{
			m_pFile->OpenDirectory( EString(pszDirPath) ) ;

			ERISAArchive::EDirectory&
					dirEntries = m_pFile->ReferFileEntries() ;
			for ( size_t i = 0; i < dirEntries.GetSize(); i ++ )
			{
				ERISAArchive::EFileName *	pFileName = dirEntries.GetTagAt( i ) ;
				ERISAArchive::FILE_INFO *	pFileInfo = dirEntries.GetObjectAt( i ) ;
				if ( !(pFileInfo->dwAttribute & ERISAArchive::attrDirectory) )
				{
					continue ;
				}
				if ( pFileInfo->dwAttribute & ERISAArchive::attrFileNameUTF8 )
				{
					SString *	pStrFileName = new SString ;
					Charset::Decode
						( *pStrFileName,
							Charset::encodingUTF8,
							(const uint8_t*) pFileName->CharPtr(),
							(ssize_t) pFileName->GetLength() ) ;
					listDirs.Add( pStrFileName ) ;
				}
				else
				{
					listDirs.Add( new SString(*pFileName) ) ;
				}
			}
		}
		m_pFile->m_cs.Unlock() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行環境オブジェクト
//////////////////////////////////////////////////////////////////////////////

// ECSEnvironment::EFile 消滅
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::EFile::~EFile( void )
{
	if ( m_file.IsFileOpened() )
	{
		ERISAArchive::Close( ) ;
		m_file.Close( ) ;
	}
	if ( m_pOwnFile != NULL )
	{
		ERISAArchive::Close( ) ;
		delete	m_pOwnFile ;
		m_pOwnFile = NULL ;
	}
	delete	m_pOpener ;
	m_pOpener = NULL ;
}

// ECSEnvironment::EPlugin 消滅
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::EPlugin::~EPlugin( void )
{
	if ( m_hModule != NULL )
	{
		::FreeLibrary( m_hModule ) ;
	}
}


// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSEnvironment, SEnvironmentInterface, EDescription )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::ECSEnvironment( void )
{
	m_pContext = NULL ;
	m_fAcceptOtherSaveDir = false ;
	m_hMainIcon = NULL ;
	m_sizeDisplay.w = 640 ;
	m_sizeDisplay.h = 480 ;
	m_nDisplayDepth = 0 ;
	m_nFrequency = 0 ;
	m_clCooperation = EGameWindow::levelNormal ;
	m_fNoChangeMode = false ;
	m_fCompileToNative = true ;
	m_fNativeBoundary = false ;
	m_nMaxHeapBlock = 0x2000 ;
	m_nDefaultHeapSize = 0xFF00 ;
	m_nDefaultStackSize = 0x1000 ;
	m_pisSession = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSEnvironment::~ECSEnvironment( void )
{
	Release( ) ;
}

// 設定ファイル初期化＆読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::Initialize
	( ESLFileObject * pfile, ECSContext * pContext )
{
	//
	// カレントディレクトリ取得
	//
	EString	strCurDir ;
	::GetCurrentDirectory( 0x400, strCurDir.GetBuffer(0x400) ) ;
	strCurDir.ReleaseBuffer( ) ;
	if ( (strCurDir.GetLength() >= 1)
		&& (strCurDir.GetAt(strCurDir.GetLength() - 1) == '\\') )
	{
		strCurDir = strCurDir.Left( strCurDir.GetLength() - 1 ) ;
	}
	m_staEnvDirPath.RemoveAll( ) ;
	SetEnvironmentPath( EWideString( strCurDir ), pContext ) ;
	//
	// 設定ファイル読み込み
	//
	HINSTANCE	hInstance = ::GetModuleHandle( NULL ) ;
	//
	Release( ) ;
	m_hInstance = hInstance ;
	//
	if ( pfile != NULL )
	{
		return	LoadEnvironment( *pfile, hInstance ) ;
	}
	return	eslErrSuccess ;
}

// 設定リソース解法
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::Release( void )
{
	m_hMainIcon = NULL ;
	m_strIconID.FreeString( ) ;
	m_lstIconID.RemoveAll( ) ;
	m_lstIconFile.RemoveAll( ) ;
	m_lstCursor.RemoveAll( ) ;
	m_lstCursorFile.RemoveAll( ) ;
	m_staCursor.RemoveAll( ) ;
	m_lstFiles.RemoveAll( ) ;
	m_lstModule.RemoveAll( ) ;
	//
	m_strCaption.FreeString( ) ;
	m_sizeDisplay.w = 640 ;
	m_sizeDisplay.h = 480 ;
	m_nDisplayDepth = 0 ;
	m_nFrequency = 0 ;
	m_clCooperation = EGameWindow::levelNormal ;
	//
	if ( m_pisSession != NULL )
	{
		delete	m_pisSession ;
		m_pisSession = NULL ;
	}
}

// 環境ディレクトリパスを設定する
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::SetEnvironmentPath
	( const wchar_t * pwszCurrent, ECSContext * pContext )
{
	m_pContext = pContext ;
	//
	// $(CURRENT) 設定
	//
	EWideString	wstrCurrent = pwszCurrent ;
	if ( wstrCurrent.Right(1) == L"\\" )
	{
		wstrCurrent = wstrCurrent.Left( wstrCurrent.GetLength() - 1 ) ;
	}
	m_staEnvDirPath.SetAs( L"CURRENT", new EWideString(wstrCurrent) ) ;
	m_strBaseFilePath = wstrCurrent ;
	//
	// $(SYSTEM), $(SYSDRV) 設定
	//
	EString	strSysDir ;
	::GetWindowsDirectory( strSysDir.GetBuffer(0x400), 0x400 ) ;
	strSysDir.ReleaseBuffer( ) ;
	if ( strSysDir.Right(1) == "\\" )
	{
		strSysDir = strSysDir.Left( strSysDir.GetLength() - 1 ) ;
	}
	//
	m_staEnvDirPath.SetAs( L"SYSTEM", new EWideString( strSysDir ) ) ;
	m_staEnvDirPath.SetAs( L"SYSDRV", new EWideString( strSysDir.Left(2) ) ) ;
	//
	// $(APPDATA) 設定
	//
	ERegistryKey	key ;
	if ( !key.OpenKey
		( HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\"
			"CurrentVersion\\Explorer\\Shell Folders" ) )
	{
		EString	strPath = key.GetString( "AppData", NULL ) ;
		if ( !strPath.IsEmpty() )
		{
			if ( strPath.Right(1) == "\\" )
			{
				strPath = strPath.Left( strPath.GetLength() - 1 ) ;
			}
			m_staEnvDirPath.SetAs( L"APPDATA", new EWideString( strPath ) ) ;
		}
	}
	//
	// $(PROGRAM_FILES)
	//
	if ( !key.OpenKey
		( HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion" ) )
	{
		EString	strPath = key.GetString( "ProgramFilesDir", NULL ) ;
		if ( !strPath.IsEmpty() )
		{
			if ( strPath.Right(1) == "\\" )
			{
				strPath = strPath.Left( strPath.GetLength() - 1 ) ;
			}
			m_staEnvDirPath.SetAs( L"PROGRAM_FILES", new EWideString( strPath ) ) ;
		}
	}
}

// 設定ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::LoadEnvironment
	( ESLFileObject & file, HINSTANCE hInstance )
{
	//
	// ファイルを読み込む
	//
	EStreamXMLString	xmlConfig ;
	xmlConfig.ReadTextFile( file ) ;
	ReadDescription( xmlConfig, dftXML ) ;
	//
	// タグを順次処理
	//
	m_hInstance = hInstance ;
	EDescription *	pdscScript = GetContentTagAs( 0, L"script" ) ;
	if ( pdscScript == NULL )
	{
		return	eslErrGeneral ;
	}
	m_strScriptFile = pdscScript->GetAttrString( L"src", NULL ) ;
	//
	for ( int i = 0; i < pdscScript->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = pdscScript->GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"save_dir" )
		{
			#if	_MSC_VER < 1800
			OSVERSIONINFO	osvi ;
			osvi.dwOSVersionInfoSize = sizeof(osvi) ;
			::GetVersionEx( &osvi ) ;
			#endif
			//
			bool	fVersion = true, fPlatform = true ;
			EStreamWideString	swsVersion =
				pdscTag->GetAttrString( L"version", NULL ) ;
			if ( !swsVersion.IsEmpty() )
			{
				EObjArray<EWideString>	lstParam ;
				EStreamWideString	swsUsage = L"(%s)[.(%s)]\\" ;
				EString				strErrMsg ;
				if ( !swsVersion.IsMatchUsage( swsUsage, strErrMsg, &lstParam ) )
				{
					#if	_MSC_VER >= 1800
					WORD	wMajorVersion = (WORD) lstParam[0].AsInteger() ;
					WORD	wMinorVersion = 0 ;
					if ( !lstParam[1].IsEmpty() )
					{
						wMinorVersion = (WORD) lstParam[1].AsInteger() ;
					}
					if ( !IsWindowsVersionOrGreater( wMajorVersion, wMinorVersion, 0 ) )
					{
						fVersion = false ;
					}
					#else
					if ( (DWORD) lstParam[0].AsInteger() != osvi.dwMajorVersion )
					{
						fVersion = false ;
					}
					else if ( !lstParam[1].IsEmpty() )
					{
						if ( (DWORD) lstParam[1].AsInteger() != osvi.dwMinorVersion )
						{
							fVersion = false ;
						}
					}
					#endif
				}
			}
			#if	_MSC_VER < 1800
			if ( (DWORD) pdscTag->GetAttrInteger
					( L"platform", osvi.dwPlatformId ) != osvi.dwPlatformId )
			{
				fPlatform = false ;
			}
			#endif
			if ( fVersion && fPlatform )
			{
				m_strSaveDir =
					FilterFilePath( pdscTag->GetAttrString( L"path", NULL ) ) ;
				m_fAcceptOtherSaveDir =
					(pdscTag->GetAttrString
						( L"accept_other_dir", L"false" ) == L"true") ;
			}
		}
		else if ( pdscTag->Tag() == L"icon" )
		{
			EString	strIconID = pdscTag->GetAttrString( L"id", NULL ) ;
			EString	strIconFile =
				FilterFilePath( pdscTag->GetAttrString( L"src", NULL ) ) ;
			//
			AddIcon( strIconID, strIconFile ) ;
		}
		else if ( pdscTag->Tag() == L"cursor" )
		{
			EString	strCursorID =
				pdscTag->GetAttrString( L"id", NULL ) ;
			EString	strCursorFile =
				FilterFilePath( pdscTag->GetAttrString( L"src", NULL ) ) ;
			//
			AddCursor( strCursorID, strCursorFile ) ;
		}
		else if ( pdscTag->Tag() == L"file" )
		{
			AddFileDirectory
				( pdscTag->GetAttrString( L"path", NULL ),
					pdscTag->GetAttrString( L"id", NULL ) ) ;
		}
		else if ( pdscTag->Tag() == L"archive" )
		{
			AddFileArchive
				( pdscTag->GetAttrString( L"path", NULL ),
					EString( pdscTag->GetAttrString( L"key", NULL ) ),
					pdscTag->GetAttrString( L"id", NULL ) ) ;
		}
		else if ( pdscTag->Tag() == L"display" )
		{
			EWideString	wstrCooperation ;
			static const wchar_t *	pwszCooperations[] =
			{
				L"window", L"normal", L"fullscreen", L"exclusive", NULL
			} ;
			static const EGameWindow::CooperationLevel	clCooperations[] =
			{
				EGameWindow::levelWindow,
				EGameWindow::levelNormal,
				EGameWindow::levelFullScreen,
				EGameWindow::levelExclusive
			} ;
			m_strCaption =
				pdscTag->GetAttrString( L"caption", L"詞葉" ) ;
			m_strBootName =
				pdscTag->GetAttrString
					( L"boot_name", EWideString( m_strCaption ) ) ;
			m_sizeDisplay.w = pdscTag->GetAttrInteger( L"width", 640 ) ;
			m_sizeDisplay.h = pdscTag->GetAttrInteger( L"height", 480 ) ;
			m_nDisplayDepth = pdscTag->GetAttrInteger( L"depth", 0 ) ;
			m_nFrequency = pdscTag->GetAttrInteger( L"frequency", 0 ) ;
			wstrCooperation =
				pdscTag->GetAttrString( L"CooperationLevel", L"normal" ) ;
			for ( int i = 0; pwszCooperations[i]; i ++ )
			{
				if ( wstrCooperation == pwszCooperations[i] )
				{
					m_clCooperation = clCooperations[i] ;
					break ;
				}
			}
			m_fNoChangeMode =
				(pdscTag->GetAttrString
					( L"change_mode",
						(m_fNoChangeMode ? L"false" : L"true") ) != L"true") ;
		}
		else if ( pdscTag->Tag() == L"envvar" )
		{
			static const wchar_t *	pwszParentKeys[] =
			{
				L"HKEY_CLASSES_ROOT",
				L"HKEY_CURRENT_USER",
				L"HKEY_LOCAL_MACHINE",
				L"HKEY_USERS",
				L"HKEY_PERFORMANCE_DATA",
				L"HKEY_CURRENT_CONFIG",
				L"HKEY_DYN_DATA",
				NULL
			} ;
			static const HKEY	hParentKeys[] =
			{
				HKEY_CLASSES_ROOT,
				HKEY_CURRENT_USER,
				HKEY_LOCAL_MACHINE,
				HKEY_USERS,
				HKEY_PERFORMANCE_DATA,
				HKEY_CURRENT_CONFIG,
				HKEY_DYN_DATA
			} ;
			EWideString	wstrValue = pdscTag->GetAttrString( L"def", L"" ) ;
			EWideString	wstrName = pdscTag->GetAttrString( L"name", L"" ) ;
			EWideString	wstrRegPath = pdscTag->GetAttrString( L"reg", NULL ) ;
			int		iKeySep = wstrRegPath.Find( L'\\' ) ;
			HKEY	hKey = NULL ;
			if ( iKeySep >= 0 )
			{
				for ( int i = 0; pwszParentKeys[i]; i ++ )
				{
					EWideString	wstrKey = pwszParentKeys[i] ;
					if ( !wstrKey.CompareNoCase( wstrRegPath.Left( iKeySep ) ) )
					{
						hKey = hParentKeys[i] ;
						wstrRegPath = wstrRegPath.Middle( iKeySep + 1 ) ;
						break ;
					}
				}
			}
			if ( hKey != NULL )
			{
				ERegistryKey	key ;
				if ( !key.OpenKey( hKey, EString( wstrRegPath ) ) )
				{
					wstrValue =
						key.GetString
							( EString( pdscTag->GetAttrString( L"key", NULL ) ),
														EString( wstrValue ) ) ;
				}
			}
			m_staEnvDirPath.SetAs( wstrName, new EWideString( wstrValue ) ) ;
		}
		else if ( pdscTag->Tag() == L"module" )
		{
			AddModule( pdscTag->GetAttrString( L"file", NULL ) ) ;
		}
		else if ( pdscTag->Tag() == L"vm" )
		{
			m_nMaxHeapBlock =
				pdscTag->GetAttrInteger( L"max_heap_block", m_nMaxHeapBlock ) ;
			m_nDefaultHeapSize =
				pdscTag->GetAttrInteger
					( L"heap_size", m_nDefaultHeapSize / 1024 ) * 1024 ;
			m_nDefaultStackSize =
				pdscTag->GetAttrInteger
					( L"init_stack_size", m_nDefaultStackSize / 1024 ) * 1024 ;
			m_fCompileToNative =
				(pdscTag->GetAttrString( L"jit_compiler", L"true" ) == L"true") ;
			m_fNativeBoundary =
				(pdscTag->GetAttrString( L"jit_boundary", L"false" ) == L"true") ;
		}
		else if ( pdscTag->Tag() == L"fonts" )
		{
			for ( int j = 0; j < pdscTag->GetContentTagCount(); j ++ )
			{
				EDescription *	pdscFontTag = pdscTag->GetContentTagAt( j ) ;
				if ( pdscFontTag == NULL )
				{
					continue ;
				}
				if ( pdscFontTag->Tag() == L"file" )
				{
					AddFont
						( pdscFontTag->GetAttrString( L"name", NULL ),
							pdscFontTag->GetAttrString( L"path", NULL ) ) ;
				}
			}
		}
	}
	//
	return	eslErrSuccess ;
}

// 書き出し可能ディレクトリ設定
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::SetSaveDirectory
	( const char * pszSaveDir, bool fAcceptOtherSaveDir )
{
	m_strSaveDir = pszSaveDir ;
	m_fAcceptOtherSaveDir = fAcceptOtherSaveDir ;
}

// 読み込みアーカイブファイル追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddFileArchive
	( const wchar_t * pwszFilePath,
		const char * pszPassword, const wchar_t * pwszID )
{
	EFile *	pfile = new EFile ;
	pfile->m_strDirPath = FilterFilePath( pwszFilePath ) ;
	pfile->m_fArchive = true ;
	pfile->m_strPassword = pszPassword ;
	pfile->m_wstrID = pwszID ;
	//
	for ( unsigned int i = 0; i < m_lstFiles.GetSize(); i ++ )
	{
		EFile *	pfe = m_lstFiles.GetAt( i ) ;
		if ( pfe && pfe->m_fArchive
			&& !pfe->m_strDirPath.CompareNoCase( pfile->m_strDirPath ) )
		{
			delete	pfile ;
			return	eslErrSuccess ;
		}
	}
	m_lstFiles.Add( pfile ) ;
	//
	if ( pfile->m_file.Open
		( pfile->m_strDirPath, ESLFileObject::modeRead
								| ESLFileObject::shareRead ) )
	{
		return	eslErrGeneral ;
	}
	else if ( pfile->Open( &(pfile->m_file) ) )
	{
		pfile->Close( ) ;
		pfile->m_file.Close( ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

ESLError ECSEnvironment::AddFileArchive
	( EMemoryFile * pmemfile,
		const char * pszPassword, const wchar_t * pwszID )
{
	EFile *	pfile = new EFile ;
	pfile->m_fArchive = true ;
	pfile->m_pOwnFile = pmemfile ;
	pfile->m_strPassword = pszPassword ;
	pfile->m_wstrID = pwszID ;
	//
	if ( pfile->Open( pmemfile ) )
	{
		pfile->Close( ) ;
		delete	pfile ;
		return	eslErrGeneral ;
	}
	m_lstFiles.Add( pfile ) ;
	return	eslErrSuccess ;
}

// 読み込みディレクトリパス追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddFileDirectory
		( const wchar_t * pwszFileDir, const wchar_t * pwszID )
{
	EWideString	wstrFilePath = FilterFilePath( pwszFileDir ) ;
	EFile *	pfile = new EFile ;
	if ( wstrFilePath.Right(1) == L"\\" )
	{
		pfile->m_strDirPath =
			wstrFilePath.Left( wstrFilePath.GetLength() - 1 ) ;
	}
	else
	{
		pfile->m_strDirPath = wstrFilePath ;
	}
	pfile->m_wstrID = pwszID ;
	m_lstFiles.Add( pfile ) ;
	return	eslErrSuccess ;
}

// パス有効設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::EnableFilePath( const wchar_t * pwszID, bool fEnable )
{
	for ( unsigned int i = 0; i < m_lstFiles.GetSize(); i ++ )
	{
		EFile *	pfile = m_lstFiles.GetAt( i ) ;
		if ( pfile == NULL )
		{
			continue ;
		}
		if ( pfile->m_wstrID == pwszID )
		{
			pfile->m_fDisabled = !fEnable ;
			return	eslErrSuccess ;
		}
	}
	return	eslErrFailed ;
}

// アイコン追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddIcon( const char * pszID, const char * pszFile )
{
	EString	strIconID = pszID ;
	EString	strIconFile = pszFile ;
	if ( (m_hInstance != NULL)
		&& (m_strIconID.IsEmpty() || (strIconID == "IDI_MAIN")) )
	{
		m_strIconID = strIconID ;
		m_hMainIcon = ::LoadIcon( m_hInstance, strIconID ) ;
	}
	if ( !strIconFile.IsEmpty() && (m_hMainIcon == NULL) )
	{
		m_hMainIcon = (HICON) ::LoadImage
			( NULL, strIconFile, IMAGE_ICON,
				0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE ) ;
	}
	m_lstIconID.Add( new EString( strIconID ) ) ;
	m_lstIconFile.Add( new EString( strIconFile ) ) ;
	return	eslErrSuccess ;
}

// カーソル追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddCursor( const char * pszID, const char * pszFile )
{
	EString	strCursorID = pszID ;
	EString	strCursorFile = pszFile ;
	HCURSOR	hCursor = ::LoadCursor( m_hInstance, strCursorID ) ;
	if ( hCursor == NULL )
	{
		hCursor = (HCURSOR) ::LoadImage
			( NULL, strCursorFile, IMAGE_CURSOR,
					0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE ) ;
	}
	m_lstCursor.Add( hCursor ) ;
	m_lstCursorFile.Add( new EString( strCursorFile ) ) ;
	m_staCursor.Add( ECSWideString( strCursorID ) ) ;
	//
	return	(hCursor != NULL) ? eslErrSuccess : eslErrGeneral ;
}

// モジュール追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddModule
	( const wchar_t * pwszModuleName, ECSContext * pContext )
{
	EString	strModulePath = FilterFilePath( pwszModuleName ) ;
	for ( unsigned int i = 0; i < m_lstModule.GetSize(); i ++ )
	{
		EPlugin *	ppi = m_lstModule.GetAt( i ) ;
		if ( ppi && !ppi->m_strModuleName.CompareNoCase( strModulePath ) )
		{
			return	eslErrSuccess ;
		}
	}
	//
	HMODULE	hModule = ::LoadLibrary( strModulePath ) ;
	if ( hModule == NULL )
	{
		return	eslErrGeneral ;
	}
	EPlugin *	ppi = new EPlugin ;
	ppi->m_strModuleName = strModulePath ;
	ppi->m_hModule = hModule ;
	m_lstModule.Add( ppi ) ;
	//
	PECS_PLUGIN_ENTRYPOINT	pfnEntryPoint =
		(PECS_PLUGIN_ENTRYPOINT) ::GetProcAddress
				( hModule, "ECS_PLUGIN_ENTRYPOINT" ) ;
	if ( pfnEntryPoint != NULL )
	{
		ppi->m_ppiet = pfnEntryPoint( ) ;
		//
		if ( pContext == NULL )
		{
			pContext = m_pContext ;
		}
		if ( pContext != NULL )
		{
			ppi->m_ppiet->pfnStartup( pContext->GetContextInterface() ) ;
			ppi->m_fStartup = true ;
		}
	}
	return	eslErrSuccess ;
}

// フォント追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSEnvironment::AddFont
	( const wchar_t * pwszFontName, const wchar_t * pwszFontFile )
{
	SFileInterface *	pFile =
		NewOpenFile( pwszFontFile, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	eslErrFailed ;
	}
	SGLBitmapFontLoader *	pFont = new SGLBitmapFontLoader ;
	if ( pFont->OpenFontFile( pFile ) )
	{
		delete	pFont ;
		return	eslErrFailed ;
	}
	SGLFont::RegisterStockFont( pwszFontName, pFont ) ;
	return	eslErrSuccess ;
}

// ファイルパスフィルタリング
//////////////////////////////////////////////////////////////////////////////
EWideString ECSEnvironment::FilterFilePath( const wchar_t * pwszPath )
{
	EStreamWideString	swsPath = pwszPath ;
	unsigned int	iLast = 0 ;
	EWideString		wstrPath ;
	while ( !swsPath.IsIndexOverflow() )
	{
		unsigned int	iCurrent = swsPath.GetIndex( ) ;
		wchar_t	wch = swsPath.GetCharacter( ) ;
		if ( wch != L'$' )
			continue ;
		if ( swsPath.GetCharacter() != L'(' )
			continue ;
		//
		EWideString	wstrName = swsPath.GetEnclosedString( L')', FALSE ) ;
		EWideString *	pwstrVar = m_staEnvDirPath.GetAs( wstrName ) ;
		if ( pwstrVar != NULL )
		{
			wstrPath += swsPath.Middle( iLast, iCurrent - iLast ) ;
			wstrPath += *pwstrVar ;
			iLast = swsPath.GetIndex( ) ;
		}
	}
	wstrPath += swsPath.Middle( iLast ) ;
	return	wstrPath ;
}

// カーソル取得
//////////////////////////////////////////////////////////////////////////////
HCURSOR ECSEnvironment::GetCursorAs( const wchar_t * pwszID )
{
	int	iFind = m_staCursor.FindIndex( pwszID ) ;
	if ( iFind < 0 )
	{
		return	NULL ;
	}
	return	m_lstCursor.GetAt( iFind ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSEnvironment::OpenFileObject
	( const char * pszFileName, long int nOpenFlag )
{
	if ( !(nOpenFlag & ESLFileObject::modeWrite) )
	{
		//
		// 読み込み用ファイル
		//
		EString	strFilePath = pszFileName ;
/*		if ( !strFilePath.CompareLeft( "http://" ) )
		{
			ESyncHttpFile *	phttpf = new ESyncHttpFile ;
			if ( phttpf->OpenURL
				( strFilePath, 10000, false, "CotophaScript" ) )
			{
				delete	phttpf ;
				return	NULL ;
			}
			return	phttpf ;
		}
		else*/ if ( !strFilePath.CompareLeft( "http://" )
					|| !strFilePath.CompareLeft( "https://" ) )
		{
			OpenInternetSession( ) ;
			EString	strHeader = "User-Agent: CotophaScript" ;
			EInternetHttpFile *	phttpf = new EInternetHttpFile ;
			if ( phttpf->OpenURL
				( *m_pisSession, strFilePath, NULL,
						strHeader, strHeader.GetLength() ) )
			{
				delete	phttpf ;
				return	NULL ;
			}
			return	phttpf ;
		}
		else if ( !strFilePath.CompareLeft( "ftp://" ) )
		{
			ESyncFtpFile *	pftpf = new ESyncFtpFile ;
			if ( pftpf->OpenURL( strFilePath, NULL, NULL ) )
			{
				delete	pftpf ;
				return	NULL ;
			}
			return	pftpf ;
		}
		for ( unsigned int i = 0; i < m_lstFiles.GetSize(); i ++ )
		{
			EFile *	pfile = m_lstFiles.GetAt( i ) ;
			if ( pfile == NULL )
			{
				continue ;
			}
			if ( pfile->m_fDisabled )
			{
				continue ;
			}
			if ( pfile->m_fArchive )
			{
				pfile->m_cs.Lock() ;
				if ( (pfile->m_pOwnFile == NULL)
					&& !pfile->m_file.IsFileOpened() )
				{
					if ( !pfile->m_file.Open
						( pfile->m_strDirPath, ESLFileObject::modeRead
												| ESLFileObject::shareRead ) )
					{
						if ( pfile->Open( &(pfile->m_file) ) )
						{
							pfile->Close( ) ;
							pfile->m_file.Close( ) ;
						}
					}
				}
				if ( (pfile->m_pOwnFile != NULL)
					|| pfile->m_file.IsFileOpened() )
				{
					if ( !pfile->OpenFile( pszFileName, pfile->m_strPassword ) )
					{
						ESLFileObject *	p = pfile->Duplicate( ) ;
						pfile->AscendFile( ) ;
						p->AttachFileOpener( pfile ) ;
						pfile->m_cs.Unlock() ;
						return	p ;
					}
				}
				pfile->m_cs.Unlock() ;
			}
			else
			{
				strFilePath = pszFileName ;
				if ( !strFilePath.IsEmpty()
					&& (strFilePath.GetAt(0) != '\\')
					&& (strFilePath.Find( ':' ) < 0) )
				{
					if ( pfile->m_strDirPath.Right(1) != "\\" )
					{
						strFilePath = pfile->m_strDirPath + "\\" + strFilePath ;
					}
					else
					{
						strFilePath = pfile->m_strDirPath + strFilePath ;
					}
				}
				ERawFile *	p = new ERawFile ;
				if ( p->Open( strFilePath, nOpenFlag ) )
				{
					delete	p ;
				}
				else
				{
					return	p ;
				}
			}
		}
	}
	//
	// セーブ用ディレクトリのファイル
	//
	EString	strSaveDir = m_strSaveDir ;
	EString	strFilePath = pszFileName ;
	if ( !m_fAcceptOtherSaveDir )
	{
		if ( (strFilePath.GetAt(0) == '\\') || (strFilePath.Find( ':' ) >= 0) )
		{
			strFilePath = EString( strFilePath.GetFileNamePart() ) ;
		}
	}
	if ( (strSaveDir.Right(1) != "\\") && !strSaveDir.IsEmpty() )
	{
		strSaveDir += '\\' ;
	}
	strFilePath = strSaveDir.OffsetFilePath( strFilePath ) ;
	//
	if ( nOpenFlag & ESLFileObject::modeCreateFlag )
	{
		//
		// ディレクトリ作成
		//
		CreateDirectory( strFilePath ) ;
	}
	ERawFile *	p = new ERawFile ;
	if ( p->Open( strFilePath, nOpenFlag ) )
	{
		delete	p ;
	}
	else
	{
		return	p ;
	}
	return	NULL ;
}

// ディレクトリ作成
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::CreateDirectory( const char * pszFilePath )
{
	EWideString	wstrFilePath = pszFilePath ;
	int	i = wstrFilePath.Find( ':' ) ;
	if ( i < 0 )
	{
		i = 2 ;
	}
	else
	{
		i += 2 ;
	}
	while ( i < (int) wstrFilePath.GetLength() )
	{
		wchar_t	wch = wstrFilePath.GetAt( i ) ;
		if ( (wch == '\\') || (wch == '/') )
		{
			EString	strDirPath = wstrFilePath.Left( i ) ;
			WIN32_FIND_DATA	wfd ;
			HANDLE	hFind = ::FindFirstFile( strDirPath, &wfd ) ;
			if ( hFind == INVALID_HANDLE_VALUE )
			{
				::CreateDirectory( strDirPath, NULL ) ;
			}
			else
			{
				::FindClose( hFind ) ;
			}
		}
		i ++ ;
	}
}

// インターネットセッション作成
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::OpenInternetSession( void )
{
	if ( m_pisSession == NULL )
	{
		m_pisSession = new EInternetSession ;
		m_pisSession->Open( ) ;
		//
		EString	strTempBase = m_strSaveDir.OffsetFilePath( "temp" ) ;
		CreateDirectory( strTempBase ) ;
		m_pisSession->SetTemporaryFileInfo( strTempBase, 0x100000 ) ;
	}
}

// DLL 関数を検索する
//////////////////////////////////////////////////////////////////////////////
FARPROC ECSEnvironment::FindPluginedFunction( const char * pszFuncName )
{
	FARPROC	pfnFunc = ::GetProcAddress( m_hInstance, pszFuncName ) ;
	if ( pfnFunc != NULL )
	{
		return	pfnFunc ;
	}
	for ( int i = 0; i < (int) m_lstModule.GetSize(); i ++ )
	{
		ECSEnvironment::EPlugin *	ppi = m_lstModule.GetAt( i ) ;
		if ( (ppi != NULL) && (ppi->m_hModule != NULL) )
		{
			pfnFunc = ::GetProcAddress( ppi->m_hModule, pszFuncName ) ;
			if ( pfnFunc != NULL )
			{
				break ;
			}
		}
	}
	return	pfnFunc ;
}

// 設定情報取得
//////////////////////////////////////////////////////////////////////////////
bool ECSEnvironment::GetEnvironmentString
	( SString& strValue, const wchar_t * pszValuePath )
{
	SString	strValuePath = pszValuePath ;
	SString	strPath = strValuePath.GetFileDirectoryPart() ;
	SString	strAttr = strValuePath.GetFileNamePart() ;
	strPath.Replace( L'/', L'\\' ) ;
	if ( strPath.GetLastAt(0) == L'\\' )
	{
		strPath.ChopRight( 1 ) ;
	}
	strValue = GetStringAt( strPath, strAttr, NULL ) ;
	return	!strValue.IsEmpty() ;
}

// アプリケーション名
//////////////////////////////////////////////////////////////////////////////
void ECSEnvironment::GetApplicationName( SString& strAppName )
{
	strAppName = m_strCaption ;
}

void ECSEnvironment::SetApplicationName( const wchar_t * pszAppName )
{
	m_strCaption = pszAppName ;
}

// Sakura2 JIT Compiler
//////////////////////////////////////////////////////////////////////////////
bool ECSEnvironment::IsEnabledSakura2JITCompiler( void )
{
	return	m_fCompileToNative ;
}

void ECSEnvironment::EnableSakura2JITCompiler( bool fJIT )
{
	m_fCompileToNative = fJIT ;
}

bool ECSEnvironment::IsEnabledSakura2JITBoundary( void )
{
	return	m_fNativeBoundary ;
}

void ECSEnvironment::EnableSakura2JITBoundary( bool fBoundary )
{
	m_fNativeBoundary = fBoundary ;
}

uint64_t ECSEnvironment::GetSakura2JITCpuFeatures( void )
{
	return	-1 ;
}

// ヒープメモリ
//////////////////////////////////////////////////////////////////////////////
size_t ECSEnvironment::GetHeapBlockMaxSize( void )
{
	return	m_nMaxHeapBlock ;
}

void ECSEnvironment::SetHeapBlockMaxSize( size_t nSize )
{
	m_nMaxHeapBlock = nSize ;
}

size_t ECSEnvironment::GetDefaultHeapSize( void )
{
	return	m_nDefaultHeapSize ;
}

void ECSEnvironment::SetDefaultHeapSize( size_t nSize )
{
	m_nDefaultHeapSize = nSize ;
}

// 初期スタックサイズ
//////////////////////////////////////////////////////////////////////////////
size_t ECSEnvironment::GetDefaultStackSize( void )
{
	return	m_nDefaultStackSize ;
}

void ECSEnvironment::SetDefaultStackSize( size_t nSize )
{
	m_nDefaultStackSize = nSize ;
}

// ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////
SFileInterface * ECSEnvironment::NewOpenFile
		( const wchar_t * pszFilePath, long int nOpenFlags )
{
	EString	strFilePath = pszFilePath ;
	ESLFileObject *	pfile = OpenFileObject( strFilePath, nOpenFlags ) ;
	if ( pfile == NULL )
	{
		return	NULL ;
	}
	return	new SESLFileInterface( pfile, true ) ;
}

bool ECSEnvironment::IsExistingFile( const wchar_t * pszFilePath )
{
	EString	strFilePath = pszFilePath ;
	ESLFileObject *	pfile = OpenFileObject( strFilePath, 0 ) ;
	if ( pfile == NULL )
	{
		return	false ;
	}
	delete	pfile ;
	return	true ;
}

SError ECSEnvironment::QueryFileState
		( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	return	errFailed ;
}

size_t ECSEnvironment::GetFileOpenerCount( void )
{
	return	m_lstFiles.GetSize() ;
}

ssize_t ECSEnvironment::FindFileOpenerAs( const wchar_t * pwszID )
{
	for ( unsigned int i = 0; i < m_lstFiles.GetSize(); i ++ )
	{
		EFile *	pFile = m_lstFiles.GetAt( i ) ;
		if ( pFile != NULL )
		{
			if ( pFile->m_wstrID == pwszID )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}

SFileOpener * ECSEnvironment::GetFileOpenerAt( size_t iOpener )
{
	EFile *	pFile = m_lstFiles.GetAt( iOpener ) ;
	if ( pFile != NULL )
	{
		if ( pFile->m_pOpener == NULL )
		{
			pFile->m_pOpener = new EFileOpener( pFile, false ) ;
		}
		return	pFile->m_pOpener ;
	}
	return	NULL ;
}

bool ECSEnvironment::GetFileOpenerIDAt( SString& strID, size_t iOpener )
{
	EFile *	pFile = m_lstFiles.GetAt( iOpener ) ;
	if ( pFile != NULL )
	{
		if ( !pFile->m_wstrID.IsEmpty() )
		{
			strID = pFile->m_wstrID ;
			return	true ;
		}
	}
	return	false ;
}

void ECSEnvironment::AddFileOpener
	( SFileOpener * pOpener,
		const wchar_t * pwszID, const wchar_t * pwszDefaultDir )
{
	delete	pOpener ;
}

void ECSEnvironment::RemoveFileOpener( const wchar_t * pwszID )
{
}

void ECSEnvironment::EnableFileOpener
			( const wchar_t * pwszID, bool fEnable )
{
}

// 書き込み可能ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////
bool ECSEnvironment::CanOpenAllFileForWriting( void )
{
	return	m_fAcceptOtherSaveDir ;
}

void ECSEnvironment::AcceptAllFileForWriting( bool fAllWriting )
{
	m_fAcceptOtherSaveDir = fAllWriting ;
}

SFileOpener * ECSEnvironment::GetWritableFileOpener( void )
{
	return	NULL ;
}

void ECSEnvironment::SetWritableFileOpener( SFileOpener * pOpener )
{
	delete	pOpener ;
}

// ファイル・パス
//////////////////////////////////////////////////////////////////////////////
const SString & ECSEnvironment::GetBaseFilePath( void ) const
{
	return	m_strBaseFilePath ;
}

void ECSEnvironment::SetBaseFilePath( const wchar_t * pszFilePath )
{
	m_strBaseFilePath = pszFilePath ;
}

SString ECSEnvironment::OffsetFilePath( const wchar_t * pszFilePath )
{
	return	m_strBaseFilePath.OffsetFilePath( pszFilePath ) ;
}

