
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuraglx/sglx_std_app.h>
#include <sakuraglx/extra/sglx_version_downloader.h>

#if	defined(__PLATFORM_ANDROID__)
#include <esl/esl_java_object.h>
#include <sakura/ssys_android_file.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// プロセス（仮想マシン）環境設定
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

//////////////////////////////////////////////////////////////////////////////
// Environment インターフェース互換
//////////////////////////////////////////////////////////////////////////////

// 設定情報取得
//////////////////////////////////////////////////////////////////////////////
bool Environment::GetEnvironmentString
	( SArray<uint16_t>& strValue, const wchar_t * pszValuePath )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		SString	strTemp ;
		bool	fResult =
			pEnv->GetEnvironmentString( strTemp, pszValuePath ) ;
		//
		if ( strTemp.IsEmpty() )
		{
			strTemp = L"" ;
		}
		strValue.SetLength( strTemp.GetLength() + 1 ) ;
		eslMoveMemory
			( strValue.GetArray(),
				strTemp.GetConstArray(),
				(strTemp.GetLength() + 1) * sizeof(uint16_t) ) ;
		strValue.FinishArray() ;
		strValue.SetLength( strTemp.GetLength() ) ;
		return	fResult ;
	}
	return	false ;
}

// アプリケーション名
//////////////////////////////////////////////////////////////////////////////
void Environment::GetApplicationName( SArray<uint16_t>& strAppName )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		SString	strTemp ;
		pEnv->GetApplicationName( strTemp ) ;
		//
		if ( strTemp.IsEmpty() )
		{
			strTemp = L"" ;
		}
		strAppName.SetLength( strTemp.GetLength() + 1 ) ;
		eslMoveMemory
			( strAppName.GetArray(),
				strTemp.GetConstArray(),
				(strTemp.GetLength() + 1) * sizeof(uint16_t) ) ;
		strAppName.FinishArray() ;
		strAppName.SetLength( strTemp.GetLength() ) ;
	}
}

void Environment::SetApplicationName( const wchar_t * pszAppName )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		pEnv->SetApplicationName( pszAppName ) ;
	}
}

// Sakura2 JIT Compiler
//////////////////////////////////////////////////////////////////////////////
bool Environment::IsEnabledSakura2JITCompiler( void )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		return	pEnv->IsEnabledSakura2JITCompiler() ;
	}
	return	false ;
}

void Environment::EnableSakura2JITCompiler( bool fJIT )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		pEnv->EnableSakura2JITCompiler( fJIT ) ;
	}
}

bool Environment::IsEnabledSakura2JITBoundary( void )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		return	pEnv->IsEnabledSakura2JITBoundary() ;
	}
	return	false ;
}

void Environment::EnableSakura2JITBoundary( bool fBoundary )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		pEnv->EnableSakura2JITBoundary( fBoundary ) ;
	}
}

// 書き込み可能ファイル
//////////////////////////////////////////////////////////////////////////////
bool Environment::CanOpenAllFileForWriting( void )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		return	pEnv->CanOpenAllFileForWriting() ;
	}
	return	false ;
}

void Environment::AcceptAllFileForWriting( bool fAllWriting )
{
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		pEnv->AcceptAllFileForWriting( fAllWriting ) ;
	}
}
#endif


//////////////////////////////////////////////////////////////////////////////
// SEnvironmentInterface クラス実装
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL(SEnvironmentInterface * SEnvironmentInterface::m_pDefault = NULL) ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SEnvironmentInterface, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SEnvironmentInterface::SEnvironmentInterface( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SEnvironmentInterface::~SEnvironmentInterface( void )
{
}

// ファイル列挙
//////////////////////////////////////////////////////////////////////////////
SError SEnvironmentInterface::ListReadableFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszFilePathWildCard )
{
	listFiles.RemoveAll() ;
	//
	SString	strFilePathWildCard = pszFilePathWildCard ;
	SString	strDirPath = strFilePathWildCard.GetFileDirectoryPart() ;
	SString	strFileWildCard = strFilePathWildCard.GetFileNamePart() ;
	if ( (strDirPath.GetLastAt( 0 ) == L'\\')
		|| (strDirPath.GetLastAt( 0 ) == L'/') )
	{
		strDirPath.ChopRight( 1 ) ;
	}
	//
	SObjectArray<SString>	listTemp ;
	size_t	nCount = GetFileOpenerCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SFileOpener *	pOpener = GetFileOpenerAt( i ) ;
		if ( pOpener == NULL )
		{
			continue ;
		}
		listTemp.RemoveAll() ;
		pOpener->ListSubFiles( listTemp, strDirPath ) ;
		//
		for ( size_t j = 0; j < listTemp.GetLength(); j ++ )
		{
			SString *	pstrFile = listTemp.GetAt( j ) ;
			if ( pstrFile
				&& SFileOpener::IsMatchWildCardTo
							( strFileWildCard, *pstrFile ) )
			{
				if ( strDirPath.IsEmpty() )
				{
					pstrFile = listTemp.ExchangeAt( j, NULL ) ;
					listFiles.Add( pstrFile ) ;
				}
				else
				{
					listFiles.Add
						( new SString( strDirPath.OffsetFilePath( *pstrFile ) ) ) ;
				}
			}
		}
	}
	return	errSuccess ;
}

// グローバル環境設定
//////////////////////////////////////////////////////////////////////////////
SEnvironmentInterface * SEnvironmentInterface::GetInstance( void )
{
	return	m_pDefault ;
}

void SEnvironmentInterface::AttachInstance( SEnvironmentInterface * pEnv )
{
	m_pDefault = pEnv ;
}


#if	!defined(__COTOPHA__)

#include <sakuragl/sgl_erisa_lib.h>
#include <sakura/ssys_fragment_file.h>
#include <sakura/ssys_http_file.h>

//////////////////////////////////////////////////////////////////////////////
// SEnvironment クラス実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
( SSystem::SEnvironment, SEnvironmentInterface, SProgressiveUserInterface )
ESL_IMPLEMENT_CLASS_INFO( SSystem::SProgressiveUserInterface, ESLObject )

// ファイルのダウンロードか？ローカルコピーか？
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::DownloadFile::WillDownloadOnlineURL( void ) const
{
	ssize_t	iScheme = g_defURLOpener.FindScheme( m_urlDownload ) ;
	if ( iScheme >= 0 )
	{
		const SVirtualURLOpener::SCHEME *
				pScheme = g_defURLOpener.GetSchemeAt( (size_t) iScheme ) ;
		if ( pScheme != NULL )
		{
			return	((pScheme->nFlags & SVirtualURLOpener::schemeOverNetwork) != 0) ;
		}
	}
	return	false ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SEnvironment::SEnvironment( void )
{
	ClearEnvironment() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SEnvironment::~SEnvironment( void )
{
}

// デフォルトの環境変数設定
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::RegisterDefaultEnvironmentString( void )
{
#if		defined(__PLATFORM_WINDOWS__)
	wchar_t	bufDir[MAX_PATH + 1] ;
/*
	::GetModuleFileNameW( ::GetModuleHandle(NULL), bufDir, MAX_PATH ) ;
	SString	strModulePath = bufDir ;
	SString	strModuleDir = strModulePath.GetFileDirectoryPart() ;
	if ( (strModuleDir.GetLastAt(0) == L'\\')
		|| (strModuleDir.GetLastAt(0) == L'/') )
	{
		strModuleDir.ChopRight( 1 ) ;
	}
	RegisterEnvironmentString( L"CURRENT", strModuleDir ) ;
*/
	::GetCurrentDirectoryW( MAX_PATH, bufDir ) ;
	RegisterEnvironmentString( L"CURRENT", SString(bufDir) ) ;
	//
	::GetWindowsDirectoryW( bufDir, MAX_PATH ) ;
	RegisterEnvironmentString( L"SYSTEM", SString(bufDir) ) ;
	//
	bufDir[2] = 0 ;
	RegisterEnvironmentString( L"SYSDRV", SString(bufDir) ) ;
	//
	SString	strAppData ;
	SFile::GetDefaultDirectory
		( strAppData, SFile::DefaultDirectory::ApplicationData ) ;
	RegisterEnvironmentString( L"APPDATA", strAppData ) ;
	//
	SString	strMyDoc ;
	SFile::GetDefaultDirectory
		( strMyDoc, SFile::DefaultDirectory::UserDocuments ) ;
	RegisterEnvironmentString( L"DOCUMENTS", strMyDoc ) ;

#elif	defined(__PLATFORM_ANDROID__)
	RegisterEnvironmentString( L"CURRENT", L"assets://" ) ;
	RegisterEnvironmentString( L"ASSETS", L"assets://" ) ;
	RegisterEnvironmentString( L"LOCAL", L"local://" ) ;
	RegisterEnvironmentString( L"DATA", L"data://" ) ;
	RegisterEnvironmentString( L"SDCARD", L"sd://" ) ;
	RegisterEnvironmentString( L"APPDATA", L"local://" ) ;
	RegisterEnvironmentString( L"DOCUMENTS", L"local://" ) ;

	SString	strPackageName ;
	JNI::GetAndroidJavaPackageName( strPackageName ) ;
	RegisterEnvironmentString( L"APPID", strPackageName ) ;

#endif
}

// XML 読み込み
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::ReadDocument
	( SFileInterface& file, SParserErrorInterface& perr )
{
	SError	err = m_xmlEnv.ReadDocument( file, perr ) ;
	InitEnvironment( m_xmlEnv ) ;
	return	err ;
}

// XML ドキュメント取得
//////////////////////////////////////////////////////////////////////////////
const SXMLDocument& SEnvironment::GetXMLDocumnet( void ) const
{
	return	m_xmlEnv ;
}

// 環境設定初期化
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ClearEnvironment( void )
{
	m_xmlEnv.RemoveAllContents() ;
	m_ssoaEnvVar.RemoveAll() ;
	m_vectorOpener.RemoveAll() ;
	m_pWritableOpener = NULL ;
	m_vectorTempOpener.RemoveAll() ;
	//
	m_flagJITCompiler = false ;
	m_flagJITBoundary = true ;
	m_flagAllFileWritable = false ;
	m_maskCpuFeatures = -1 ;
	m_sizeMaxHeapBlock = 0x2000 ;
	m_sizeDefaultHeap = 0xFF00 ;
	m_sizeDefaultStack = 0x1000 ;
	m_sizeReqMemory = 0 ;
	m_maskReqJITFeatures = 0 ;
	//
	m_flagAppUpdate = false ;
	m_flagAutoCheckUpdate = false ;
	m_flagNoConfirmToUpdate = false ;
	m_flagMustUpdate = false ;
	m_nAppVersion = 0 ;
	m_strUpdateURL.FreeArray() ;
	m_strUpdaterCmd.FreeArray() ;
	//
	m_xmlDownloads.RemoveAllContents() ;
	m_arrayDownloads.RemoveAll() ;
	//
	m_strDynamicEnvFile.FreeArray() ;
	m_xmlDynamicEnv.RemoveAllContents() ;
	m_pxmlDynamicEnv = NULL ;
	m_arrayDynamicFiles.RemoveAll() ;
}

// パラメータ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::InitEnvironment( const SXMLDocument& xmlDoc )
{
	//
	// タグを処理
	//
	SXMLDocument *	pxmlScript = xmlDoc.GetElementTagAs( L"script" ) ;
	if ( pxmlScript == NULL )
	{
		pxmlScript = xmlDoc.GetElementTagAs( L"cotopha" ) ;
		if ( pxmlScript == NULL )
		{
			return ;
		}
	}
	const size_t	nCount = pxmlScript->GetElementsCount() ;
	for ( size_t iElement = 0; iElement < nCount; iElement ++ )
	{
		SXMLDocument *	pxmlTag = pxmlScript->GetElementAt( iElement ) ;
		if ( pxmlTag == NULL )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"save_dir" )
		{
			ParseEnvironmentSaveDirTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"file" )
		{
			ParseEnvironmentFileTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"archive" )
		{
			ParseEnvironmentArchiveTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"display" )
		{
			ParseEnvironmentDisplayTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"vm" )
		{
			ParseEnvironmentVMTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"update" )
		{
			ParseEnvironmentUpdateTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"requirement" )
		{
			ParseEnvironmentRequirementTag( *pxmlTag ) ;
		}
		else
		{
			ParseExtendedEnvironment( *pxmlTag ) ;
		}
	}
	//
	// セーブディレクトリを作成
	//
	if ( m_pWritableOpener == NULL )
	{
		SString	strSavePath ;
		#if	defined(__PLATFORM_ANDROID__)
			strSavePath = L"local://" ;
		#else
			strSavePath = L"$(CURRENT)" ;
			FilterEnvironmentString( strSavePath ) ;
		#endif
		m_pWritableOpener = CreateFileOpener( strSavePath, false ) ;
	}
	SOffsetFileOpener *	pOffsetOpener =
			ESLTypeCast<SOffsetFileOpener>( m_pWritableOpener.Ptr() ) ;
	if ( pOffsetOpener != NULL )
	{
		SString	strSaveDir = pOffsetOpener->GetFullBasePath() ;
		ESLTrace( "create save directory \'%s\'\n",
					strSaveDir.ToCharArray().GetConstArray() ) ;
		CreateFullDirectory( strSaveDir ) ;
	}
	//
	// 動的拡張環境ファイル
	//
	m_strDynamicEnvFile = pxmlScript->GetAttrStringAs( L"dynamic_env" ) ;
	if ( !m_strDynamicEnvFile.IsEmpty() )
	{
		if ( m_xmlDynamicEnv.LoadDocument( m_strDynamicEnvFile, m_xmlDynamicEnv ) )
		{
			ESLTrace( "failed to load dynamic environment \'%s\'\n",
						m_strDynamicEnvFile.ToCharArray().GetConstArray() ) ;
		}
		m_pxmlDynamicEnv = m_xmlDynamicEnv.CreateElementTagAs( L"cotopha" ) ;
		//
		const size_t	nElements = pxmlScript->GetElementsCount() ;
		for ( size_t iElement = 0; iElement < nElements; iElement ++ )
		{
			SXMLDocument *	pxmlTag = m_pxmlDynamicEnv->GetElementAt( iElement ) ;
			if ( pxmlTag == NULL )
			{
				continue ;
			}
			if ( pxmlTag->GetTag() == L"archive" )
			{
				ParseEnvironmentArchiveTag( *pxmlTag ) ;
			}
			else
			{
				ParseExtendedEnvironment( *pxmlTag ) ;
			}
		}
	}
}

// <save_dir> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentSaveDirTag( const SXMLDocument& xmlTag )
{
	SString	strPath = xmlTag.GetAttrStringAs( L"path" ) ;
	FilterEnvironmentString( strPath ) ;
	//
	m_flagAllFileWritable =
		(xmlTag.GetAttrStringAs( L"accept_other_dir" ) == L"true") ;
	//
	bool	fVersion = true, fPlatform = true ;
#if	defined(__PLATFORM_WINDOWS__)
	if ( g_infoPlatform.platformFamily == platformFamilyWin32 )
	{
		int64_t	nPlatform = 1 ;
		if ( (g_infoPlatform.runtimeOS == platformOS_WindowsNT)
			|| (g_infoPlatform.runtimeOS == platformOS_WindowsNT_Server) )
		{
			nPlatform = 2 ;
		}
		fPlatform = (nPlatform
						== xmlTag.GetAttrIntegerAs( L"platform", nPlatform )) ;
		//
		SString *	pstrVersion = xmlTag.GetAttributeAs( L"version" ) ;
		if ( pstrVersion != NULL )
		{
			SStringParser	sparsVersion ;
			sparsVersion.AttachString( *pstrVersion ) ;
			if ( sparsVersion.NextInteger()
							== (g_infoPlatform.versionOS >> 16) )
			{
				if ( sparsVersion.HasToComeChar( L"." ) == L'.' )
				{
					if ( sparsVersion.NextInteger()
								!= (g_infoPlatform.versionOS & 0xFFFF) )
					{
						fPlatform = false ;
					}
				}
			}
			else
			{
				fPlatform = false ;
			}
		}
	}
#endif
	if ( fVersion && fPlatform )
	{
		m_pWritableOpener = CreateFileOpener( strPath, false ) ;
		LoadDownloadedInfo() ;
	}
}

// <file> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentFileTag( const SXMLDocument& xmlTag )
{
	SString	strPath = xmlTag.GetAttrStringAs( L"path" ) ;
	SString	strID = xmlTag.GetAttrStringAs( L"id" ) ;
	bool	fFragment = (xmlTag.GetAttrStringAs( L"fragment" ) == L"true") ;
	ssize_t nFragCache = -1 ;
	if ( fFragment )
	{
		nFragCache =
			(ssize_t) xmlTag.GetAttrIntegerAs( L"fragment_cache", -1 ) ;
	}
	FilterEnvironmentString( strPath ) ;
	//
	SFileOpener *
		pOpener = CreateFileOpener( strPath, fFragment, nFragCache ) ;
	if ( pOpener != NULL )
	{
		AddFileOpener( pOpener, strID ) ;
	}
	//
	const size_t	nCount = xmlTag.GetElementsCount() ;
	for ( size_t iElement = 0; iElement < nCount; iElement ++ )
	{
		SXMLDocument *	pxmlFile = xmlTag.GetElementAt( iElement ) ;
		if ( pxmlFile == NULL )
		{
			continue ;
		}
		if ( pxmlFile->GetTag() == L"file" )
		{
			DownloadFile *	pdf = new DownloadFile ;
			m_arrayDownloads.Add( pdf ) ;
			//
			pdf->m_flagIndirect = false ;
			pdf->m_flagUpdatable = false ;
			pdf->m_strID = strID ;
			pdf->m_urlDownload = pxmlFile->GetAttrStringAs( L"path", NULL ) ;
			FilterEnvironmentString( pdf->m_urlDownload ) ;
			//
			pdf->m_strDisplayName =
					pxmlFile->GetAttrStringAs( L"display_name", NULL ) ;
			pdf->m_strLocalPath =
				strPath.OffsetFilePath
					( SString(pdf->m_urlDownload.GetFileNamePart()) ) ;
			pdf->m_crc32 = (uint32_t) pxmlFile->GetAttrHexIntegerAs( L"crc" ) ;
			pdf->m_length = pxmlFile->GetAttrIntegerAs( L"size" ) ;
		}
	}
}

// <file> タグ用オープナー生成
//////////////////////////////////////////////////////////////////////////////
SFileOpener * SEnvironment::CreateFileOpener
	( const wchar_t * pwszPath,  bool fFragment, ssize_t nFragmentCache )
{
	if ( !fFragment )
	{
		return	g_defURLOpener.NewOffsetOpener( pwszPath, L'/' ) ;
	}
	else
	{
		return	new SFragmentFileOpener
			( L"", L'/',
				g_defURLOpener.NewOffsetOpener( pwszPath, L'/' ), true ) ;
	}
}

// <archive> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentArchiveTag( const SXMLDocument& xmlTag )
{
	SString	strPath = xmlTag.GetAttrStringAs( L"path" ) ;
	SString	strID = xmlTag.GetAttrStringAs( L"id" ) ;
	SString	strPassword = xmlTag.GetAttrStringAs( L"key" ) ;
	SString	strDefDir = xmlTag.GetAttrStringAs( L"default_dir" ) ;
	bool	fFragment = (xmlTag.GetAttrStringAs( L"fragment" ) == L"true") ;
	bool	fLoadDynamic =
				!strID.IsEmpty()
					&& (xmlTag.GetAttrStringAs( L"load_dynamic" ) == L"true") ;
	bool	fEncrypt32 =
				(xmlTag.GetAttrStringAs( L"encrypt32" ) == L"true") ;
	ssize_t	nFragCache = -1 ;
	if ( fFragment )
	{
		nFragCache =
			(ssize_t) xmlTag.GetAttrIntegerAs( L"fragment_cache", -1 ) ;
	}
	FilterEnvironmentString( strPath ) ;
	//
	SString *		pstrIndirect = xmlTag.GetAttributeAs( L"download" ) ;
	bool			flagDelay = false ;
	DownloadFile *	pdf = nullptr ;
	if ( pstrIndirect != nullptr )
	{
		SString	strDisplayName =
					xmlTag.GetAttrStringAs( L"display_name", nullptr ) ;
		bool	fUpdatable =
					(xmlTag.GetAttrStringAs( L"updatable", nullptr ) == L"true") ;
		if ( !fLoadDynamic )
		{
			pdf = AddDownloadIndirectArchive
				( strID, strDisplayName,
					strPath, *pstrIndirect,
					strPassword, fUpdatable, fEncrypt32 ) ;
			flagDelay = true ;
		}
		else
		{
			pdf = GetDynamicDownloadFileInfo( strID ) ;
			if ( pdf == nullptr )
			{
				pdf = NewDownloadIndirectArchive
						( strID, strDisplayName, strPath,
							*pstrIndirect,
							strPassword, fUpdatable, fEncrypt32 ) ;
				pdf->m_flagDynamicLoad = true ;
				m_arrayDynamicFiles.Add( pdf ) ;
			}
		}
	}
	else
	{
		SXMLDocument *	pxmlFile = xmlTag.GetElementTagAs( L"file" ) ;
		if ( pxmlFile != nullptr )
		{
			SString	strDisplayName =
						xmlTag.GetAttrStringAs( L"display_name", nullptr ) ;
			SString	strDownloadURL =
						pxmlFile->GetAttrStringAs( L"path", nullptr ) ;
			uint32_t	nCRC32 =
						(uint32_t) pxmlFile->GetAttrHexIntegerAs( L"crc" ) ;
			uint64_t	nFileSize = pxmlFile->GetAttrIntegerAs( L"size" ) ;
			if ( !fLoadDynamic )
			{
				pdf = AddDownloadArchiveFile
					( strID, strDisplayName,
						strPath, strDownloadURL,
						strPassword, nCRC32, nFileSize, fEncrypt32 ) ;
				flagDelay = true ;
			}
			else
			{
				pdf = GetDynamicDownloadFileInfo( strID ) ;
				if ( pdf == nullptr )
				{
					pdf = NewDownloadArchiveFile
							( strID, strDisplayName,
								strPath, strDownloadURL,
								strPassword, nCRC32, nFileSize, fEncrypt32 ) ;
					pdf->m_flagDynamicLoad = true ;
					m_arrayDynamicFiles.Add( pdf ) ;
				}
			}
		}
	}
	if ( pdf != nullptr )
	{
		pdf->m_strDefaultDir = strDefDir ;
	}
	if ( (pdf != nullptr)
		&& CheckDownloadedFile( pdf, true, nullptr ) )
	{
		flagDelay = true ;
	}
	if ( !flagDelay )
	{
		SString	strMachineID ;
		if ( fEncrypt32 )
		{
			strMachineID =
				SakuraGL::SGLStdApplication::GetMachineUniqueId() ;
		}
		SFileOpener *	pOpener =
			CreateArchiveOpener
				( strPath, strPassword,
					fFragment, nFragCache,
					fEncrypt32, strMachineID ) ;
		if ( pOpener != NULL )
		{
			AddFileOpener( pOpener, strID, strDefDir ) ;
		}
	}
}

// <archive> タグ用オープナー生成
//////////////////////////////////////////////////////////////////////////////
SFileOpener * SEnvironment::CreateArchiveOpener
	( const wchar_t * pwszPath,
		const wchar_t * pwszPassword,
		bool fFragment, ssize_t nFragmentCache,
		bool fCrypt32, const wchar_t * pwszDecryptPass )
{
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile( pwszPath, SFile::shareRead ) ;
	if ( pFile != NULL )
	{
		STimeCounter	timer ;
		if ( fFragment )
		{
			SString	strPath = pwszPath ;
			SFragmentFile *	pff = new SFragmentFile ;
			SError	err = pff->Open
				( *pFile, CreateTempFileOpener
							(strPath.GetFileDirectoryPart()) ) ;
			delete	pFile ;
			if ( err )
			{
				delete	pff ;
				return	NULL ;
			}
			if ( nFragmentCache >= 1 )
			{
				pff->SetCacheLimit( (size_t) nFragmentCache ) ;
			}
			pFile = pff ;
		}
		if ( fCrypt32 )
		{
			ERISA::SGLDecrypt32File *
					pDecrypt = new ERISA::SGLDecrypt32File ;
			SString	strMachineID =
				SakuraGL::SGLStdApplication::GetMachineUniqueId() ;
			if ( pDecrypt->Open( pFile, true, strMachineID ) )
			{
				delete	pDecrypt ;
				return	NULL ;
			}
			pFile = pDecrypt ;
		}
		ERISA::SGLArchiveFile *	pArchive = new ERISA::SGLArchiveFile ;
		if ( !pArchive->OpenArchive( pFile, true, SFile::modeRead ) )
		{
			pArchive->SetDefaultPassword( pwszPassword ) ;
			//
			Trace( "open archive %s : %f [ms]\n",
				SString(pwszPath).ToCharArray().GetConstArray(), timer.GetRealTime() ) ;
			return	pArchive ;
		}
		else
		{
			Trace( "failed to open archive %s\n",
						SString(pwszPath).ToCharArray().GetConstArray() ) ;
			delete	pArchive ;
		}
	}
	else
	{
		Trace( "failed to open archive %s\n",
					SString(pwszPath).ToCharArray().GetConstArray() ) ;
	}
	return	NULL ;
}

// 一致する <file> オープナー取得
//////////////////////////////////////////////////////////////////////////////
SOffsetFileOpener *
	SEnvironment::CreateTempFileOpener( const wchar_t * pwszPath )
{
	SOffsetFileOpener *	pOpener  ;
	const size_t	nCount = m_vectorTempOpener.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pOpener = m_vectorTempOpener.GetAt( i ) ;
		if ( (pOpener != NULL)
			&& (pOpener->GetBasePath() == pwszPath) )
		{
			return	pOpener ;
		}
	}
	pOpener = new SOffsetFileOpener
				( pwszPath, L'/', new SStandardFileOpener, true ) ;
	m_vectorTempOpener.Add( pOpener ) ;
	return	pOpener ;
}

// <display> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentDisplayTag( const SXMLDocument& xmlTag )
{
	m_strAppName = xmlTag.GetAttrStringAs( L"caption" ) ;
}

// <vm> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentVMTag( const SXMLDocument& xmlTag )
{
	m_flagJITCompiler =
		(xmlTag.GetAttrStringAs( L"jit_compiler", L"true" ) == L"true") ;
	m_flagJITBoundary =
		(xmlTag.GetAttrStringAs( L"jit_boundary" ) == L"true") ;
	m_sizeMaxHeapBlock =
		(size_t) xmlTag.GetAttrIntegerAs
					( L"max_heap_block", m_sizeMaxHeapBlock ) ;
	m_sizeDefaultHeap =
		(size_t) xmlTag.GetAttrIntegerAs
			( L"heap_size", m_sizeDefaultHeap / 1024 ) * 1024 ;
	m_sizeDefaultStack =
		(size_t) xmlTag.GetAttrIntegerAs
			( L"init_stack_size", m_sizeDefaultStack / 1024 ) * 1024 ;
	//
	if ( m_flagJITCompiler )
	{
		const wchar_t *	pwszCpuFamily[] =
		{
			L"x86", L"arm",
		} ;
		const wchar_t *	pwszFeatures[][8] =
		{
			{ L"486", L"all", L"mmx", L"sse", L"sse2", L"sse3", NULL },
			{ L"armv5", L"all", L"armv7", L"vfpv3", L"neon", NULL },
		} ;
		const uint64_t	flagCpuFeatures[][8] =
		{
			{ 0, (uint64_t) -1,
				cpuX86_Feature_MMX,
				(cpuX86_Feature_MMX | cpuX86_Feature_SSE),
				(cpuX86_Feature_MMX | cpuX86_Feature_SSE
									| cpuX86_Feature_SSE2),
				(cpuX86_Feature_MMX | cpuX86_Feature_SSE
					| cpuX86_Feature_SSE2 | cpuX86_Feature_SSE3), 0, 0 },
			{ 0, (uint64_t) -1,
				cpuARM_Feature_ARMv7, cpuARM_Feature_VFPv3,
				(cpuARM_Feature_VFPv3 | cpuARM_Feature_NEON), 0, 0, 0 },
		} ;
		CPU_Family	cpuFamily = GetCPUFamily() ;
		int			iFamily = -1 ;
		if ( cpuFamily == cpuFamily_X86 )
		{
			iFamily = 0 ;
		}
		else if ( cpuFamily == cpuFamily_ARM )
		{
			iFamily = 1 ;
		}
		m_maskCpuFeatures = -1 ;
		if ( iFamily >= 0 )
		{
			SString *	pstrFeatures =
					xmlTag.GetAttributeAs( pwszCpuFamily[iFamily] ) ;
			if ( pstrFeatures != NULL )
			{
				SStringParser	sparsFeature ;
				SString			strFeature ;
				sparsFeature.AttachString( *pstrFeatures ) ;
				m_maskCpuFeatures = 0 ;
				while ( sparsFeature.PassSpace() )
				{
					if ( !sparsFeature.NextString( strFeature ) )
					{
						break ;
					}
					for ( int i = 0; pwszFeatures[iFamily][i] != NULL; i ++ )
					{
						if ( strFeature.CompareNoCase( pwszFeatures[iFamily][i] ) == 0 )
						{
							m_maskCpuFeatures |= flagCpuFeatures[iFamily][i] ;
							break ;
						}
					}
				}
			}
		}
	}
}

// <update> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentUpdateTag( const SXMLDocument& xmlTag )
{
	SString	strCmdLine ;
	m_nAppVersion = xmlTag.GetAttrIntegerAs( L"version", 0 ) ;
	m_strUpdateURL = xmlTag.GetAttrStringAs( L"url" ) ;
	strCmdLine = xmlTag.GetAttrStringAs( L"cmd" ) ;
	m_flagAppUpdate = !m_strUpdateURL.IsEmpty() ;
	m_flagAutoCheckUpdate =
			(xmlTag.GetAttrStringAs( L"auto_check" ) == L"true") ;
	m_flagNoConfirmToUpdate =
			(xmlTag.GetAttrStringAs( L"no_confirm" ) == L"true") ;
	m_flagMustUpdate =
			(xmlTag.GetAttrStringAs( L"must_update" ) == L"true") ;
	//
	SString	strVersion( m_nAppVersion ) ;
	SString::FILTER_ENTRY	filter[2] =
	{
		{ L"%(url)", m_strUpdateURL },
		{ L"%(version)", strVersion },
	} ;
	SString::PrepareFilter( filter, 2 ) ;
	m_strUpdaterCmd = strCmdLine.MappingFilter( filter, 2 ) ;
	//
	FilterEnvironmentString( m_strUpdateURL ) ;
	FilterEnvironmentString( m_strUpdaterCmd ) ;
}

// <requirement> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseEnvironmentRequirementTag( const SXMLDocument& xmlTag )
{
	m_sizeReqMemory = (size_t) xmlTag.GetAttrIntegerAs( L"memory" ) ;
	//
	SString *	pstrFeatures = xmlTag.GetAttributeAs( L"cpu_features" ) ;
	m_maskReqJITFeatures = 0 ;
	if ( pstrFeatures != NULL )
	{
		const wchar_t *	pwszFeatures[] =
		{
			L"saturation", L"float", L"simd64", L"simd128", NULL,
		} ;
		const uint32_t	flagCpuFeatures[] =
		{
			jitFeature_Saturation, jitFeature_Float,
				jitFeature_SIMD64, jitFeature_SIMD128, 0,
		} ;
		SStringParser	sparsFeature ;
		SString			strFeature ;
		sparsFeature.AttachString( *pstrFeatures ) ;
		while ( sparsFeature.PassSpace() )
		{
			if ( !sparsFeature.NextString( strFeature ) )
			{
				break ;
			}
			for ( int i = 0; pwszFeatures[i] != NULL; i ++ )
			{
				if ( strFeature.CompareNoCase( pwszFeatures[i] ) == 0 )
				{
					m_maskReqJITFeatures |= flagCpuFeatures[i] ;
					break ;
				}
			}
		}
	}
}

// 非標準タグ解釈
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::ParseExtendedEnvironment( const SXMLDocument& xmlTag )
{
}

// 設定情報取得
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::GetEnvironmentString
	( SString& strValue, const wchar_t * pszValuePath )
{
	SString *	pstrValue = m_xmlEnv.GetContentsValue( pszValuePath ) ;
	if ( pstrValue != NULL )
	{
		strValue = *pstrValue ;
		FilterEnvironmentString( strValue ) ;
		return	true ;
	}
	return	false ;
}

// 文字列置き換えフィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::FilterEnvironmentString( SString& strValue )
{
	size_t	iLast = 0 ;
	for ( ; ; )
	{
		ssize_t	iEnvVar = strValue.Find( L"$(", iLast ) ;
		if ( iEnvVar < 0 )
		{
			break ;
		}
		size_t	iEnvName = iEnvVar + 2 ;
		ssize_t	iEndVar = strValue.Find( L')', iEnvName ) ;
		if ( iEndVar >= 0 )
		{
			SString	strEnvName =
				strValue.Middle( iEnvName, (ssize_t) (iEndVar - iEnvName) ) ;
			SString *	pstrEnvVar = m_ssoaEnvVar.GetAs( strEnvName ) ;
			if ( pstrEnvVar != NULL )
			{
				strValue = strValue.Left( iEnvVar )
							+ *pstrEnvVar + strValue.Middle( iEndVar + 1 ) ;
				iLast = iEnvVar + pstrEnvVar->GetLength() ;
			}
			else
			{
				iLast = iEndVar + 1 ;
			}
		}
		else
		{
			iLast = iEnvName ;
		}
	}
}

// 文字列置き換え登録
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::RegisterEnvironmentString
	( const wchar_t * pszVarName, const wchar_t * pszVarValue )
{
	m_ssoaEnvVar.SetAs( pszVarName, new SString( pszVarValue ) ) ;
}

// テキスト・コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
SString SEnvironment::GetTextResourceAs
	( const wchar_t * pwszID, const wchar_t * pwszDef )
{
	SString	strValue ;
	SString	strPath = L"script\\text\\" ;
	strPath += pwszID ;
	if ( GetEnvironmentString( strValue, strPath ) )
	{
		return	strValue ;
	}
	strPath = L"cotopha\\text\\" ;
	strPath += pwszID ;
	if ( GetEnvironmentString( strValue, strPath ) )
	{
		return	strValue ;
	}
	return	pwszDef ;
}

// アプリケーション名
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::GetApplicationName( SString& strAppName )
{
	strAppName = m_strAppName ;
}

void SEnvironment::SetApplicationName( const wchar_t * pszAppName )
{
	m_strAppName = pszAppName ;
}

// Sakura2 JIT Compiler
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::IsEnabledSakura2JITCompiler( void )
{
	return	m_flagJITCompiler ;
}

void SEnvironment::EnableSakura2JITCompiler( bool fJIT )
{
	m_flagJITCompiler = fJIT ;
}

bool SEnvironment::IsEnabledSakura2JITBoundary( void )
{
	return	m_flagJITBoundary ;
}

void SEnvironment::EnableSakura2JITBoundary( bool fBoundary )
{
	m_flagJITBoundary = fBoundary ;
}

uint64_t SEnvironment::GetSakura2JITCpuFeatures( void )
{
	return	m_maskCpuFeatures ;
}

// ヒープメモリ
//////////////////////////////////////////////////////////////////////////////
size_t SEnvironment::GetHeapBlockMaxSize( void )
{
	return	m_sizeMaxHeapBlock ;
}

void SEnvironment::SetHeapBlockMaxSize( size_t nSize )
{
	m_sizeMaxHeapBlock = nSize ;
}

size_t SEnvironment::GetDefaultHeapSize( void )
{
	return	m_sizeDefaultHeap ;
}

void SEnvironment::SetDefaultHeapSize( size_t nSize )
{
	m_sizeDefaultHeap = nSize ;
}

// 初期スタックサイズ
//////////////////////////////////////////////////////////////////////////////
size_t SEnvironment::GetDefaultStackSize( void )
{
	return	m_sizeDefaultStack ;
}

void SEnvironment::SetDefaultStackSize( size_t nSize )
{
	m_sizeDefaultStack = nSize ;
}

// ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SEnvironment::NewOpenFile
		( const wchar_t * pszFilePath, long int nOpenFlags )
{
	//
	// ファイルパスがフルパスか、スキームを含んでいるか検証
	//
	bool	fFullPath = false ;
	bool	fWithScheme = false ;
	size_t	i ;
	if ( pszFilePath != nullptr )
	{
		if ( (pszFilePath[0] == L'/') || (pszFilePath[0] == L'\\') )
		{
			fFullPath = true ;
		}
		else
		{
			for ( i = 0; pszFilePath[i]; i ++ )
			{
				if ( pszFilePath[i] == L':' )
				{
					fFullPath = true ;
					fWithScheme = (pszFilePath[1 + 1] == L'/')
								&& (pszFilePath[1 + 2] == L'/') ;
					break ;
				}
			}
		}
	}
	//
	// 書き込み可能フォルダへ試行
	//
	SFileInterface *	pfile = nullptr ;
	if ( m_pWritableOpener != nullptr )
	{
		SString			strFilePath, strTemp ;
		const wchar_t *	pwszTempFilePath = pszFilePath ;
		if ( !m_flagAllFileWritable && (pwszTempFilePath != nullptr) )
		{
			// ファイル書き出しが書き込みフォルダのみ制限されている場合には
			// ファイルパスからディレクトリを除去する
			strFilePath = pwszTempFilePath ;
			//
			ssize_t	iDir = strFilePath.Find( L':' ) ;
			if ( iDir >= 0 )
			{
				strTemp = strFilePath.Middle( (size_t) iDir + 1 ) ;
				pwszTempFilePath = strTemp ;
			}
			while ( (pwszTempFilePath[0] == L'/')
					|| (pwszTempFilePath[0] == L'\\') )
			{
				pwszTempFilePath ++ ;
			}
		}
		else if ( fFullPath )
		{
			// フルパスが指定されている場合、デフォルトのオープナーで試行する
			SFileOpener *	pDefOpener = SFileOpener::GetDefaultOpener() ;
			if ( pDefOpener != nullptr )
			{
				pfile = pDefOpener->NewOpenFile( pwszTempFilePath, nOpenFlags ) ;
				if ( pfile != nullptr )
				{
					return	pfile ;
				}
			}
			else
			{
				pfile = SFile::NewOpen( pwszTempFilePath, nOpenFlags ) ;
				if ( pfile != nullptr )
				{
					return	pfile ;
				}
			}
		}
		// 書き込みフォルダで試行する
		pfile = m_pWritableOpener->NewOpenFile( pwszTempFilePath, nOpenFlags ) ;
		if ( pfile != nullptr )
		{
			return	pfile ;
		}
	}
	if ( !(nOpenFlags & SFileInterface::modeWrite) )
	{
		if ( !fFullPath )
		{
			// フルパスでないファイル読み込みは、順次オープナーを試行する
			size_t	nCount = m_vectorOpener.GetLength() ;
			for ( i = 0; i < nCount; i ++ )
			{
				FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( i ) ;
				if ( (pfoe != nullptr) && !pfoe->m_fDisabled )
				{
					SFileOpener *	pOpener = pfoe->m_pOpener ;
					if ( pOpener != nullptr )
					{
						pfile = pOpener->NewOpenFile( pszFilePath, nOpenFlags ) ;
						if ( pfile != nullptr )
						{
							return	pfile ;
						}
					}
					if ( !pfoe->m_strDefaultDir.IsEmpty() )
					{
						pfile = pOpener->NewOpenFile
							( pfoe->m_strDefaultDir.OffsetFilePath(pszFilePath), nOpenFlags ) ;
						if ( pfile != nullptr )
						{
							return	pfile ;
						}
					}
				}
			}
		}
		// デフォルトのオープナーで試行する
		SFileOpener *	pDefOpener = SFileOpener::GetDefaultOpener() ;
		if ( pDefOpener != nullptr )
		{
			pfile = pDefOpener->NewOpenFile( pszFilePath, nOpenFlags ) ;
			if ( pfile != nullptr )
			{
				return	pfile ;
			}
		}
		if ( !fWithScheme )
		{
			pfile = SFile::NewOpen( pszFilePath, nOpenFlags ) ;
			if ( pfile != nullptr )
			{
				return	pfile ;
			}
		}
	}
	return	nullptr ;
}

bool SEnvironment::IsExistingFile( const wchar_t * pszFilePath )
{
	size_t	nCount = m_vectorOpener.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( i ) ;
		if ( pfoe == nullptr )
		{
			continue ;
		}
		SFileOpener *	pOpener = pfoe->m_pOpener ;
		if ( pOpener != nullptr )
		{
			if ( pOpener->IsExisting( pszFilePath ) )
			{
				return	true ;
			}
		}
		if ( !pfoe->m_strDefaultDir.IsEmpty() )
		{
			if ( pOpener->IsExisting
				( pfoe->m_strDefaultDir.OffsetFilePath(pszFilePath) ) )
			{
				return	true ;
			}
		}
	}
	if ( m_pWritableOpener != nullptr )
	{
		SString	strFilePath, strTemp ;
		if ( !m_flagAllFileWritable )
		{
			strFilePath = pszFilePath ;
			strTemp = strFilePath.GetFileNamePart() ;
			pszFilePath = strTemp ;
		}
		return	m_pWritableOpener->IsExisting( pszFilePath ) ;
	}
	return	false ;
}

SError SEnvironment::QueryFileState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	size_t	nCount = m_vectorOpener.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( i ) ;
		if ( pfoe == nullptr )
		{
			continue ;
		}
		SFileOpener *	pOpener = pfoe->m_pOpener ;
		if ( pOpener != NULL )
		{
			if ( !pOpener->QueryState( pszFilePath, state ) )
			{
				return	errSuccess ;
			}
		}
		if ( !pfoe->m_strDefaultDir.IsEmpty() )
		{
			if ( !pOpener->QueryState
				( pfoe->m_strDefaultDir.OffsetFilePath(pszFilePath), state ) )
			{
				return	errSuccess ;
			}
		}
	}
	if ( m_pWritableOpener != NULL )
	{
		SString	strFilePath, strTemp ;
		if ( !m_flagAllFileWritable )
		{
			strFilePath = pszFilePath ;
			strTemp = strFilePath.GetFileNamePart() ;
			pszFilePath = strTemp ;
		}
		return	m_pWritableOpener->QueryState( pszFilePath, state ) ;
	}
	return	errFailed ;
}

size_t SEnvironment::GetFileOpenerCount( void )
{
	return	m_vectorOpener.GetLength() ;
}

ssize_t SEnvironment::FindFileOpenerAs( const wchar_t * pwszID )
{
	size_t	nCount = m_vectorOpener.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( i ) ;
		if ( (pfoe != NULL) && (pfoe->m_strID == pwszID) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

SFileOpener * SEnvironment::GetFileOpenerAt( size_t iOpener )
{
	FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( iOpener ) ;
	if ( pfoe != NULL )
	{
		return	pfoe->m_pOpener ;
	}
	return	NULL ;
}

bool SEnvironment::GetFileOpenerIDAt( SString& strID, size_t iOpener )
{
	FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( iOpener ) ;
	if ( pfoe != NULL )
	{
		if ( !pfoe->m_strID.IsEmpty() )
		{
			strID = pfoe->m_strID ;
			return	true ;
		}
	}
	return	false ;
}

void SEnvironment::AddFileOpener
		( SFileOpener * pOpener,
			const wchar_t * pwszID, const wchar_t * pwszDefaultDir )
{
	m_vectorOpener.Add
		( new FileOpenerEntry( pOpener, pwszID, pwszDefaultDir ) ) ;
}

void SEnvironment::RemoveFileOpener( const wchar_t * pwszID )
{
	ssize_t	i = FindFileOpenerAs( pwszID ) ;
	if ( i >= 0 )
	{
		m_vectorOpener.RemoveAt( i ) ;
	}
}

void SEnvironment::EnableFileOpener( const wchar_t * pwszID, bool fEnable )
{
	ssize_t	i = FindFileOpenerAs( pwszID ) ;
	if ( i >= 0 )
	{
		FileOpenerEntry *	pfoe = m_vectorOpener.GetAt( i ) ;
		if ( pfoe != NULL )
		{
			ESLAssert( pfoe->m_strID == pwszID ) ;
			pfoe->m_fDisabled = !fEnable ;
		}
	}
}

// 書き込み可能ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::CanOpenAllFileForWriting( void )
{
	return	m_flagAllFileWritable ;
}

void SEnvironment::AcceptAllFileForWriting( bool fAllWriting )
{
	m_flagAllFileWritable = fAllWriting ;
}

SFileOpener * SEnvironment::GetWritableFileOpener( void )
{
	return	m_pWritableOpener ;
}

void SEnvironment::SetWritableFileOpener( SFileOpener * pOpener )
{
	m_pWritableOpener = pOpener ;
}

// ファイル・パス
//////////////////////////////////////////////////////////////////////////////
const SString & SEnvironment::GetBaseFilePath( void ) const
{
	return	m_strBasePath ;
}

void SEnvironment::SetBaseFilePath( const wchar_t * pszFilePath )
{
	m_strBasePath = pszFilePath ;
}

SString SEnvironment::OffsetFilePath( const wchar_t * pszFilePath )
{
	return	m_strBasePath.OffsetFilePath( pszFilePath ) ;
}

// ディレクトリを生成（途中のディレクトリが存在しない場合にも自動生成）
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::CreateFullDirectory( const wchar_t * pszPath )
{
	if ( pszPath == NULL )
	{
		return	errFailed ;
	}
	SFileOpener *	pOpener = NULL ;
	SStringParser	sparsPath = pszPath ;
	ssize_t	iScheme = sparsPath.Find( L':' ) ;
	size_t	lenScheme = 0 ;
	if ( iScheme >= 0 )
	{
		// スキーム名・ドライブ名に該当する箇所を飛ばす
		sparsPath.SeekIndex( iScheme + 1 ) ;
		//
		if ( sparsPath.HasToComeString( L"//" ) )
		{
			lenScheme = sparsPath.GetIndex() ;
			pOpener = g_defURLOpener.NewOffsetOpener
						( sparsPath.SubString( 0, (ssize_t) lenScheme ), L'/' ) ;
		}
	}
	else if ( sparsPath.HasToComeString( L"//" )
			|| sparsPath.HasToComeString( L"\\\\" ) )
	{
		// ホスト名・デバイス名に該当する箇所を飛ばす
		while ( !sparsPath.IsIndexOverflow() )
		{
			wchar_t	wch = sparsPath.GetCharacter() ;
			if ( (wch == L'\\') | (wch == L'/') )
			{
				break ;
			}
		}
	}
	SError	err = errFailed ;
	while ( !sparsPath.IsIndexOverflow() )
	{
		// ディレクトリ・セパレータを読み飛ばす
		while ( !sparsPath.IsIndexOverflow() )
		{
			wchar_t	wch = sparsPath.GetCharacter() ;
			if ( (wch != L'\\') & (wch != L'/') )
			{
				break ;
			}
		}
		while ( !sparsPath.IsIndexOverflow() )
		{
			wchar_t	wch = sparsPath.CurrentCharacter() ;
			if ( (wch == L'\\') | (wch == L'/') )
			{
				break ;
			}
			sparsPath.GetCharacter() ;
		}
		if ( pOpener != NULL )
		{
			err = pOpener->CreateSubDirectory
				( sparsPath.SubString
					( lenScheme, (ssize_t) (sparsPath.GetIndex() - lenScheme) ) ) ;
		}
		else
		{
			err = SFile::CreateDirectory
				( sparsPath.SubString( 0, (ssize_t) sparsPath.GetIndex() ) ) ;
		}
	}
	delete	pOpener ;
	return	err ;
}

// アプリケーション更新判定
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::DoCheckAppUpdate( SProgressiveUserInterface * pUI )
{
	if ( !m_flagAppUpdate )
	{
		return	errSuccess ;
	}
	if ( pUI == NULL )
	{
		pUI = this ;
	}
	SakuraGL::SGLVersionDownloader	verdl ;
	if ( verdl.DownloadVersionList( m_strUpdateURL ) )
	{
		return	errSuccess ;
	}
	SPointerArray<SXMLDocument>	lstPackages ;
	if ( verdl.EnumMatchVersion( lstPackages, m_nAppVersion ) == 0 )
	{
		return	errSuccess ;
	}
	if ( !m_flagNoConfirmToUpdate )
	{
		SString	strMsgForm ;
		strMsgForm =
			GetTextResourceAs
				( L"ID_CONFIRM_UPDATE_APP",
					_TX( L"\x1b[en]Will you download following files;\n\n%(0)\0"
						L"以下の更新ファイルをダウンロードしますか？\n\n%(0)\0" ) ) ;
		//
		SString	strUpdateFiles ;
		for ( size_t i = 0; i < lstPackages.GetLength(); i ++ )
		{
			SXMLDocument *	pxmlPackage = lstPackages.GetAt( i ) ;
			if ( pxmlPackage == NULL )
			{
				continue ;
			}
			SakuraGL::SGLVersionDownloader::PackageInfo	pckinf ;
			if ( verdl.GetPackageInfo( pckinf, pxmlPackage ) )
			{
				continue ;
			}
			SString	strAppInf ;
			strAppInf.Format
				( L"%s (%dfiles, %dKB)\n",
					(const wchar_t*) pckinf.m_strDisplayName,
					pckinf.m_nFileCount,
					(pckinf.m_nTotalBytes / 1024) ) ;
			strUpdateFiles += strAppInf ;
		}
		//
		SString::FILTER_ENTRY	filter[2] =
		{
			{ L"%(0)", strUpdateFiles },
			{ L"\\n", L"\n" },
		} ;
		SString::PrepareFilter( filter, 2 ) ;
		SString	strMessage = strMsgForm.MappingFilter( filter, 2 ) ;
		//
		if ( pUI->DoMessageBox
			( strMessage, m_strAppName,
						msgboxStyleOkCancel ) != msgboxResultOk )
		{
			if ( m_flagMustUpdate )
			{
				return	errAbort ;
			}
			else
			{
				return	errSuccess ;
			}
		}
	}
	//
	SStringParser	sparsCmdLine ;
	sparsCmdLine.AttachString( m_strUpdaterCmd ) ;
	//
	SString	strExe, strParams ;
	sparsCmdLine.NextStringTerm( strExe ) ;
	if ( sparsCmdLine.PassSpace() )
	{
		strParams = sparsCmdLine.SubString( sparsCmdLine.GetIndex() ) ;
	}
	else
	{
		strParams = strExe ;
		strExe.FreeArray() ;
	}
	//
	if ( OpenShellFile( strParams, shellOpenURI, strExe, NULL ) )
	{
		return	errSuccess ;
	}
	return	errAbort ;
}

// ダウンロードエントリの有無
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::AreDownloadFiles( void ) const
{
	return	(m_arrayDownloads.GetLength() != 0) ;
}

// ファイルのダウンロードを実行
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::DoDownloadFiles
		( bool fNoConfirm, SProgressiveUserInterface * pUI )
{
	if ( !AreDownloadFiles() )
	{
		return	errSuccess ;
	}
	if ( pUI == NULL )
	{
		pUI = this ;
	}
	SError			err = errSuccess ;
	DOWNLOAD_FILES	df ;
	ssize_t			nDownloads ;
	LoadDownloadedInfo() ;
	//
	pUI->CreateProgressiveDialog() ;
	nDownloads = CheckDownloadedFiles( &df, pUI ) ;
	pUI->CloseProgressiveDialog() ;
	if ( nDownloads < 0 )
	{
		return	errFailed ;
	}
	//
	SError	errResult = errSuccess ;
	while ( nDownloads != 0 )
	{
		if ( !fNoConfirm )
		{
			SString	strMsgForm ;
			if ( df.nLocalFiles == 0 )
			{
				strMsgForm =
					GetTextResourceAs
						( L"ID_CONFIRM_DOWNLOAD",
						_TX( L"\x1b[en]Should download %(0) files (%(1)MB)\0"
							L"%(0) 個のファイルをローカルストレージに"
							L"ダウンロードする必要があります (%(1)MB)\0" ) ) ;
			}
			else if ( df.nOnlineFiles != 0 )
			{
				strMsgForm =
					GetTextResourceAs
						( L"ID_CONFIRM_DOWNLOAD_AND_COPY",
						_TX( L"\x1b[en]Should download or copy %(0) files (%(1)MB)\0"
							L"%(0) 個のファイルをローカルストレージに"
							L"ダウンロード及びコピーする必要があります (%(1)MB)\0" ) ) ;
			}
			else
			{
				strMsgForm =
					GetTextResourceAs
						( L"ID_CONFIRM_INIT_COPY",
						_TX( L"\x1b[en]Should copy %(0) files (%(1)MB)\0"
							L"%(0) 個のファイルをローカルストレージに"
							L"コピーする必要があります (%(1)MB)\0" ) ) ;
			}
			SString	strDownloadFiles, strDownloadSize ;
			strDownloadFiles.FromInteger( nDownloads ) ;
			strDownloadSize.FromInteger
					( (df.nOnlineBytes + df.nLocalBytes) / (1024 * 1024) ) ;
			//
			SString::FILTER_ENTRY	filter[3] =
			{
				{ L"%(0)", strDownloadFiles },
				{ L"%(1)", strDownloadSize },
				{ L"\\n", L"\n" },
			} ;
			SString::PrepareFilter( filter, 3 ) ;
			SString	strMessage = strMsgForm.MappingFilter( filter, 3 ) ;
			//
			if ( pUI->DoMessageBox
				( strMessage, m_strAppName,
							msgboxStyleOkCancel ) != msgboxResultOk )
			{
				errResult = errAbort ;
				break ;
			}
		}
		pUI->CreateProgressiveDialog() ;
		err = DownloadAllFiles( pUI ) ;
		if ( err )
		{
			pUI->CloseProgressiveDialog() ;
			return	err ;
		}
		nDownloads = CheckDownloadedFiles( &df, pUI ) ;
		pUI->CloseProgressiveDialog() ;
		if ( nDownloads < 0 )
		{
			return	errFailed ;
		}
	}
	//
	AddDownloadedArchiveOpener() ;
	SaveDownloadedInfo() ;
	m_arrayDownloads.RemoveAll() ;
	//
	if ( m_pxmlDynamicEnv != NULL )
	{
		SaveDynamicEnvironmentFile() ;
	}
	//
	return	errResult ;
}

// システム要求チェック
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::DoCheckRequirement( void )
{
	if ( m_sizeReqMemory != 0 )
	{
		MEMORY_STATUS	mstatus ;
		GetMemoryStatus( mstatus ) ;
		//
		int64_t	nAvailMem = mstatus.nAvailVirtual ;
		if ( nAvailMem == 0 )
		{
			nAvailMem = mstatus.nAvailPhys ;
		}
		if ( (nAvailMem != 0)
			&& (nAvailMem / (1024 * 1024) < (int64_t) m_sizeReqMemory) )
		{
			SString	strMsgForm =
				GetTextResourceAs
					( L"ID_CONFIRM_REQ_MEMORY",
					_TX( L"\x1b[en]Not enough memory to launch.\n"
						L"(recommends : %(0) MB)\n"
						L"Will you launch?\0"
						L"メモリの空き容量が不足しています。\n"
						L"（推奨空きメモリサイズ：%(0) MB）\n起動しますか？\0" ) ) ;
			//
			SString	strReqSize ;
			strReqSize.FromInteger( m_sizeReqMemory ) ;
			//
			SString::FILTER_ENTRY	filter[2] =
			{
				{ L"%(0)", strReqSize },
				{ L"\\n", L"\n" },
			} ;
			SString::PrepareFilter( filter, 2 ) ;
			SString	strMsg = strMsgForm.MappingFilter( filter, 2 ) ;
			if ( SSystem::MessageBox
				( strMsg, m_strAppName,
						msgboxStyleOkCancel ) != msgboxResultOk )
			{
				return	errFailed ;
			}
		}
	}
	uint32_t	jitFeatures =
			ECSSakura2::ExecutableModule::GetJITCompilerFeatures() ;
	if ( m_maskReqJITFeatures & ~jitFeatures )
	{
		SString	strMsg =
			GetTextResourceAs
				( L"ID_CONFIRM_REQ_JIT_FEATURES",
				_TX( L"\x1b[en]Not recommended processor features.\n"
					L"(Will not be enough performance to run)\n"
					L"Will you launch?\0"
					L"CPU が推奨環境の条件を満たしていません。\n"
					L"（実行に十分なパフォーマンスが得られない可能性があります）\n"
					L"起動しますか？" ) ) ;
		if ( SSystem::MessageBox
			( strMsg, m_strAppName,
					msgboxStyleOkCancel ) != msgboxResultOk )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// 要ダウンロード書庫を追加する
//////////////////////////////////////////////////////////////////////////////
SEnvironment::DownloadFile *
	SEnvironment::AddDownloadIndirectArchive
		( const wchar_t * pwszID,
			const wchar_t * pwszDisplayName,
			const wchar_t * pwszLocalPath,
			const wchar_t * pwszIndirectURL,
			const wchar_t * pwszPassword,
			bool fUpdatable, bool fCrypt32 )
{
	if ( (pwszID != NULL) && (pwszID[0] != 0) )
	{
		if ( FindFileOpenerAs( pwszID ) >= 0 )
		{
			// 読み込み済み／又は重複
			return	NULL ;
		}
		if ( GetDownloadScheduleFileInfo( pwszID ) != NULL )
		{
			return	NULL ;
		}
	}
	SEnvironment::DownloadFile *	pdf = 
		NewDownloadIndirectArchive
			( pwszID, pwszDisplayName,
				pwszLocalPath, pwszIndirectURL,
				pwszPassword, fUpdatable, fCrypt32 ) ;
	m_arrayDownloads.Add( pdf ) ;
	return	pdf ;
}

SEnvironment::DownloadFile *
	SEnvironment::NewDownloadIndirectArchive
		( const wchar_t * pwszID,
			const wchar_t * pwszDisplayName,
			const wchar_t * pwszLocalPath,
			const wchar_t * pwszIndirectURL,
			const wchar_t * pwszPassword,
			bool fUpdatable, bool fCrypt32 )
{
	DownloadFile *	pdf = new DownloadFile ;
	pdf->m_flagIndirect = true ;
	pdf->m_flagArchive = true ;
	pdf->m_flagUpdatable = fUpdatable ;
	pdf->m_flagCrypt32 = fCrypt32 ;
	pdf->m_strID = pwszID ;
	pdf->m_strLocalPath = pwszLocalPath ;
	pdf->m_urlIndirect = pwszIndirectURL ;
	pdf->m_strPassword = pwszPassword ;
	pdf->m_strDisplayName = pwszDisplayName ;
	return	pdf ;
}

SEnvironment::DownloadFile *
	SEnvironment::AddDownloadArchiveFile
		( const wchar_t * pwszID,
			const wchar_t * pwszDisplayName,
			const wchar_t * pwszLocalPath,
			const wchar_t * pwszDownloadURL,
			const wchar_t * pwszPassword,
			uint32_t nCRC32, uint64_t nFileSize, bool fCrypt32 )
{
	if ( (pwszID != NULL) && (pwszID[0] != 0) )
	{
		if ( FindFileOpenerAs( pwszID ) >= 0 )
		{
			// 読み込み済み／又は重複
			return	NULL ;
		}
		if ( GetDownloadScheduleFileInfo( pwszID ) != NULL )
		{
			return	NULL ;
		}
	}
	SEnvironment::DownloadFile *	pdf = 
		NewDownloadArchiveFile
			( pwszID, pwszDisplayName, pwszLocalPath,
				pwszDownloadURL,
				pwszPassword, nCRC32, nFileSize, fCrypt32 ) ;
	m_arrayDownloads.Add( pdf ) ;
	return	pdf ;
}

SEnvironment::DownloadFile *
	SEnvironment::NewDownloadArchiveFile
		( const wchar_t * pwszID,
			const wchar_t * pwszDisplayName,
			const wchar_t * pwszLocalPath,
			const wchar_t * pwszDownloadURL,
			const wchar_t * pwszPassword,
			uint32_t nCRC32, uint64_t nFileSize, bool fCrypt32 )
{
	DownloadFile *	pdf = new DownloadFile ;
	//
	pdf->m_flagIndirect = false ;
	pdf->m_flagUpdatable = false ;
	pdf->m_flagCrypt32 = fCrypt32 ;
	pdf->m_flagArchive = true ;
	pdf->m_strID = pwszID ;
	pdf->m_urlDownload = pwszDownloadURL ;
	FilterEnvironmentString( pdf->m_urlDownload ) ;
	//
	pdf->m_strLocalPath = pwszLocalPath ;
	pdf->m_strPassword = pwszPassword ;
	pdf->m_crc32 = nCRC32 ;
	pdf->m_length = nFileSize ;
	pdf->m_strDisplayName = pwszDisplayName ;
	//
	return	pdf ;
}

// 動的ダウンロードファイルを要ダウンロード書庫に追加する
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::AddDynamicDownloadArchive( const wchar_t * pwszID )
{
	ESLAssert( pwszID != NULL ) ;
	ESLAssert( pwszID[0] != 0 ) ;
	if ( FindFileOpenerAs( pwszID ) >= 0 )
	{
		// 読み込み済み／又は重複
		return ;
	}
	DownloadFile *	pdf = GetDynamicDownloadFileInfo( pwszID ) ;
	if ( pdf == NULL )
	{
		return ;
	}
	if ( m_pxmlDynamicEnv == NULL )
	{
		return ;
	}
	if ( pdf->m_flagIndirect )
	{
		AddDownloadIndirectArchive
			( pdf->m_strID, pdf->m_strDisplayName,
				pdf->m_strLocalPath, pdf->m_urlIndirect,
				pdf->m_strPassword,
				pdf->m_flagUpdatable, pdf->m_flagCrypt32 ) ;
	}
	else
	{
		AddDownloadArchiveFile
			( pdf->m_strID, pdf->m_strDisplayName,
				pdf->m_strLocalPath, pdf->m_urlDownload,
				pdf->m_strPassword,
				pdf->m_crc32, pdf->m_length, pdf->m_flagCrypt32 ) ;
	}
}

// ダウンロード予約リストから検索
//////////////////////////////////////////////////////////////////////////////
SEnvironment::DownloadFile *
	SEnvironment::GetDownloadScheduleFileInfo( const wchar_t * pwszID ) const
{
	for ( size_t i = 0; i < m_arrayDownloads.GetLength(); i ++ )
	{
		DownloadFile *	pdf = m_arrayDownloads.GetAt( i ) ;
		if ( pdf != NULL )
		{
			if ( pdf->m_strID == pwszID )
			{
				return	pdf ;
			}
		}
	}
	return	NULL ;
}

// 動的ダウンロード可能情報取得
//////////////////////////////////////////////////////////////////////////////
SEnvironment::DownloadFile *
	SEnvironment::GetDynamicDownloadFileInfo( const wchar_t * pwszID ) const
{
	for ( size_t i = 0; i < m_arrayDynamicFiles.GetLength(); i ++ )
	{
		DownloadFile *	pdf = m_arrayDynamicFiles.GetAt( i ) ;
		if ( pdf != NULL )
		{
			if ( pdf->m_strID == pwszID )
			{
				return	pdf ;
			}
		}
	}
	return	NULL ;
}

// 動的拡張環境ファイル保存
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::SaveDynamicEnvironmentFile( void )
{
	if ( m_strDynamicEnvFile.IsEmpty()
		|| (m_pxmlDynamicEnv == NULL)
		|| (m_pWritableOpener == NULL) )
	{
		return	errFailed ;
	}
	SSmartPointer<SFileInterface>	pFile =
	#if	defined(__PLATFORM_ANDROID__)
		SFileOpener::DefaultNewOpenFile
			( m_strDynamicEnvFile, SFileOpener::modeCreate ) ;
	#else
		m_pWritableOpener->NewOpenFile
			( m_strDynamicEnvFile, SFileOpener::modeCreate ) ;
	#endif
	if ( pFile == NULL )
	{
		Trace( "failed to save %s\n",
					m_strDynamicEnvFile.ToCharArray().GetConstArray() ) ;
		return	errFailed ;
	}
	return	m_pxmlDynamicEnv->WriteDocument( *pFile ) ;
}

// ダウンロード済み CRC チェック実行
// (ダウンロードが必要なファイル数を返却)
//////////////////////////////////////////////////////////////////////////////
ssize_t SEnvironment::CheckDownloadedFiles
	( SEnvironment::DOWNLOAD_FILES * pdf, SProgressiveUserInterface * pUI )
{
	pUI->SetProgressiveCaption( L"ファイルをチェックしています…" ) ;
	pUI->SetProgressiveMessage( L"" ) ;
	pUI->SetProgressiveStatus( 0, 1 ) ;
	//
	DOWNLOAD_FILES	df ;
	df.nOnlineFiles = 0 ;
	df.nLocalFiles = 0 ;
	df.nOnlineBytes = 0 ;
	df.nLocalBytes = 0 ;
	//
	size_t	nCount = m_arrayDownloads.GetLength() ;
	size_t	nDownloads = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DownloadFile *	pdf = m_arrayDownloads.GetAt( i ) ;
		ESLAssert( pdf != NULL ) ;
		if ( (pdf == NULL) || pdf->m_flagDownloaded )
		{
			continue ;
		}
		SString	strFileName = pdf->m_strLocalPath.GetFileNamePart() ;
		if ( !pdf->m_strDisplayName.IsEmpty() )
		{
			strFileName = pdf->m_strDisplayName ;
		}
		pUI->SetProgressiveMessage( strFileName ) ;
		//
		// インダイレクトファイルを取得
		//
		SError	err = GetDownloadFileInfo( pdf, pUI ) ;
		if ( err )
		{
			return	-1 ;
		}
		//
		// ダウンロード済みファイルを開く
		//
		bool	fShouldDownload = CheckDownloadedFile( pdf, false, pUI ) ;
		if ( pUI->IsProgressiveCanceled() )
		{
			return	-1 ;
		}
		if ( fShouldDownload )
		{
			nDownloads ++ ;
			if ( pdf->WillDownloadOnlineURL() )
			{
				df.nOnlineFiles ++ ;
				df.nOnlineBytes += pdf->m_length ;
			}
			else
			{
				df.nLocalFiles ++ ;
				df.nLocalBytes += pdf->m_length ;
			}
		}
	}
	if ( pdf != NULL )
	{
		*pdf = df ;
	}
	return	(ssize_t) nDownloads ;
}

// ファイル更新の必要性チェック
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::CheckDownloadedFile
	( SEnvironment::DownloadFile * pdf,
		bool fNoCheckCRC, SProgressiveUserInterface * pUI )
{
	bool	fShouldDownload = false ;
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile
			( pdf->m_strLocalPath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		fShouldDownload = true ;
	}
	else
	{
		if ( pUI != NULL )
		{
			SString	strFileName = pdf->m_strLocalPath.GetFileNamePart() ;
			if ( !pdf->m_strDisplayName.IsEmpty() )
			{
				strFileName = pdf->m_strDisplayName ;
			}
			pUI->SetProgressiveMessage( strFileName + L" ..." ) ;
		}
		//
		// ファイルサイズチェック
		//
		if ( pdf->m_flagCrypt32 )
		{
			ERISA::SGLDecrypt32File *
					pDecrypt = new ERISA::SGLDecrypt32File ;
			SString	strMachineID =
				SakuraGL::SGLStdApplication::GetMachineUniqueId() ;
			pDecrypt->Open( pFile.Detach(), true, strMachineID ) ;
			pFile = pDecrypt ;
		}
		uint64_t	nTotalBytes = pFile->GetLength() ;
		if ( nTotalBytes == pdf->m_length )
		{
			if ( fNoCheckCRC )
			{
				SXMLDocument *	pxmlInfo =
						GetDownloadedInfo( pdf->m_strLocalPath ) ;
				if ( (pxmlInfo == NULL)
					|| (pdf->m_crc32 !=
							(uint32_t) pxmlInfo->GetAttrHexIntegerAs( L"crc" )) )
				{
					return	true ;
				}
				return	false ;
			}
			if ( !pdf->m_flagCheckCRC32 )
			{
				pdf->m_flagDownloaded = true ;
				return	false ;
			}
			//
			// CRC チェック
			//
			SakuraCL::CRC32Context	crc32 ;
			//
			SArray<uint8_t>	bufTemp ;
			const size_t	nBufSize = 0x10000 ;
			bufTemp.SetLength( nBufSize ) ;
			uint8_t *	pbytTemp = bufTemp.GetArray() ;
			//
			int			nTotalMB = (int) (nTotalBytes / (1024 * 1024)) ;
			uint64_t	nCheckedBytes = 0 ;
			while ( nCheckedBytes < nTotalBytes )
			{
				size_t	nReadBytes = pFile->Read( pbytTemp, nBufSize ) ;
				if ( nReadBytes == 0 )
				{
					break ;
				}
				crc32.Stream( pbytTemp, nReadBytes ) ;
				//
				nCheckedBytes += nReadBytes ;
				//
				if ( pUI != NULL )
				{
					pUI->SetProgressiveStatus
						( (int) (nCheckedBytes / (1024 * 1024)), nTotalMB ) ;
					//
					if ( pUI->IsProgressiveCanceled() )
					{
						return	false ;
					}
				}
			}
			bufTemp.FinishArray() ;
			//
			if ( crc32.GetCRC32() == pdf->m_crc32 )
			{
				pdf->m_flagCheckCRC32 = false ;
				pdf->m_flagDownloaded = true ;
			}
			else
			{
				fShouldDownload = true ;
			}
		}
		else
		{
			fShouldDownload = true ;
		}
	}
	return	fShouldDownload ;
}

// ダウンロード情報ファイルをダウンロード
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::GetDownloadFileInfo
	( SEnvironment::DownloadFile * pdf, SProgressiveUserInterface * pUI )
{
	SXMLDocument *	pxmlInfo ;
	if ( pdf->m_flagIndirect && pdf->m_urlDownload.IsEmpty() )
	{
		if ( !pdf->m_flagUpdatable )
		{
			pxmlInfo = GetDownloadedInfo( pdf->m_strLocalPath ) ;
			if ( pxmlInfo != NULL )
			{
				pdf->m_urlDownload = pxmlInfo->GetAttrStringAs( L"url" ) ;
				pdf->m_crc32 = (uint32_t) pxmlInfo->GetAttrHexIntegerAs( L"crc" ) ;
				pdf->m_length = pxmlInfo->GetAttrIntegerAs( L"size" ) ;
			}
		}
		if ( pdf->m_urlDownload.IsEmpty() )
		{
			//
			// 情報ファイルをダウンロード
			//
			bool	flagError = true ;
			do
			{
				SSmartPointer<SFileInterface>	pSrcFile = 
					SFileOpener::DefaultNewOpenFile
						( pdf->m_urlIndirect, SFileOpener::shareRead ) ;
				if ( pSrcFile == NULL )
				{
					break ;
				}
				SParserErrorInterface	perr ;
				SXMLDocument			xmlIndirect ;
				xmlIndirect.ReadDocument( *pSrcFile, perr ) ;
				//
				SXMLDocument *	pxmlFile = xmlIndirect.GetElementTagAs( L"file" ) ;
				if ( pxmlFile == NULL )
				{
					break ;
				}
				//
				SString	strURLDir = pdf->m_urlIndirect.GetFileDirectoryPart() ;
				pdf->m_urlDownload =
					strURLDir.OffsetFilePath
							( pxmlFile->GetAttrStringAs( L"path" ) ) ;
				pdf->m_crc32 = (uint32_t) pxmlFile->GetAttrHexIntegerAs( L"crc" ) ;
				pdf->m_length = pxmlFile->GetAttrIntegerAs( L"size" ) ;
				//
				flagError = pdf->m_urlDownload.IsEmpty() ;
			}
			while ( false ) ;
			if ( flagError )
			{
				pUI->DoMessageBox
					( L"ファイル情報をインターネットから"
						L"取得出来ませんでした", m_strAppName ) ;
				return	errFailed ;
			}
		}
	}
	pxmlInfo = CreateDownloadedInfo( pdf->m_strLocalPath ) ;
	if ( (pdf->m_crc32 != (uint32_t) pxmlInfo->GetAttrHexIntegerAs( L"crc" ))
		|| (pdf->m_length != (uint64_t) pxmlInfo->GetAttrIntegerAs( L"size" )) )
	{
		ESLTrace( "needs to update %s\n",
					pdf->m_urlDownload.ToCharArray().GetConstArray() ) ;
		pdf->m_flagCheckCRC32 = true ;
		pxmlInfo->SetAttributeAs( L"url", pdf->m_urlDownload ) ;
		pxmlInfo->SetAttrHexIntegerAs( L"crc", pdf->m_crc32 ) ;
		pxmlInfo->SetAttrIntegerAs( L"size", pdf->m_length ) ;
	}
	return	errSuccess ;
}

// ファイルのダウンロードを実行
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::DownloadAllFiles( SProgressiveUserInterface * pUI )
{
	bool	flagSaveAllFileWritable = m_flagAllFileWritable ;
	m_flagAllFileWritable = true ;
	//
	pUI->SetProgressiveCaption( L"ファイルをダウンロードしています…" ) ;
	pUI->SetProgressiveMessage( L"" ) ;
	pUI->SetProgressiveStatus( 0, 1 ) ;
	//
	#if	defined(__PLATFORM_WINDOWS__)
	SakuraGL::SGLPrivilegeWriteClient	pwcServWriter ;
	bool								fServFileWrite = false ;
	SString								strServCmdLine ;
	if ( GetEnvironmentString
		( strServCmdLine, L"cotopha\\update\\serv_cmd_line" ) )
	{
		if ( !pwcServWriter.StartupServer( strServCmdLine ) )
		{
			fServFileWrite = true ;
		}
	}
	#endif
	//
	size_t	nCount = m_arrayDownloads.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DownloadFile *	pdf = m_arrayDownloads.GetAt( i ) ;
		ESLAssert( pdf != NULL ) ;
		if ( (pdf == NULL) || pdf->m_flagDownloaded )
		{
			continue ;
		}
		//
		// ファイルをダウンロード
		//
		SArray<uint8_t>	bufTemp ;
		const size_t	nBufSize = 0x10000 ;
		uint8_t *		pbytTemp ;
		bool			flagError = true ;
		bool			flagOnline = pdf->WillDownloadOnlineURL() ;
		const char *	pszErrorMsg = NULL ;
		//
		bufTemp.SetLength( nBufSize ) ;
		pbytTemp = bufTemp.GetArray() ;
		//
		do
		{
			CreateFullDirectory
				( pdf->m_strLocalPath.GetFileDirectoryPart() ) ;
			//
			SSmartPointer<SFileInterface>	pFile ;
			#if	defined(__PLATFORM_WINDOWS__)
			if ( fServFileWrite )
			{
				pFile = pwcServWriter.NewOpenFile
						( pdf->m_strLocalPath, SFileOpener::modeCreate ) ;
			}
			else
			#endif
			{
				pFile = SFileOpener::DefaultNewOpenFile
						( pdf->m_strLocalPath, SFileOpener::modeCreate ) ;
			}
			if ( pFile == NULL )
			{
				Trace( "failed to create file \'%s\'\n",
						pdf->m_strLocalPath.ToCharArray().GetConstArray() ) ;
				#if	defined(__PLATFORM_ANDROID__)
				pszErrorMsg = "書き出し先ファイルを作成出来ませんでした。\n"
						"ストレージへの書き込みが許可されているか確認してください。" ;
				#else
				pszErrorMsg = "書き出し先ファイルを作成出来ませんでした" ;
				#endif
				break ;
			}
			SSmartPointer<SFileInterface>	pSrcFile =
				SFileOpener::DefaultNewOpenFile
					( pdf->m_urlDownload, SFileOpener::shareRead ) ;
			if ( pSrcFile == NULL )
			{
				if ( flagOnline )
				{
					pszErrorMsg = "ファイルのダウンロードが失敗しました" ;
				}
				else
				{
					pszErrorMsg = "ファイルのコピーが失敗しました" ;
				}
				break ;
			}
			SString	strFileName = pdf->m_strLocalPath.GetFileNamePart() ;
			if ( !pdf->m_strDisplayName.IsEmpty() )
			{
				strFileName = pdf->m_strDisplayName ;
			}
			if ( flagOnline )
			{
				pUI->SetProgressiveCaption
					( SString(L"ファイルをダウンロードしています… [")
						+ SString(i+1) + L"/" + SString(nCount) + "]" ) ;
			}
			else
			{
				pUI->SetProgressiveCaption
					( SString(L"ファイルをコピーしています… [")
						+ SString(i+1) + L"/" + SString(nCount) + "]" ) ;
			}
			pUI->SetProgressiveMessage( strFileName + L"... " ) ;
			//
			if ( pdf->m_flagCrypt32 )
			{
				ERISA::SGLEncrypt32FileWriter *
					pEncrypt = new ERISA::SGLEncrypt32FileWriter ;
				SString	strMachineID =
					SakuraGL::SGLStdApplication::GetMachineUniqueId() ;
				pEncrypt->Open( pFile.Detach(), true, strMachineID ) ;
				pFile = pEncrypt ;
			}
			//
			SakuraCL::CRC32Context	crc32 ;
			uint64_t	nDownloaded = 0 ;
			int			nTotalKB = (int) (pdf->m_length / 1024) ;
			int			nLastDownloadedKB = 0 ;
			for ( ; ; )
			{
				size_t	nReadBytes = pSrcFile->Read( pbytTemp, nBufSize ) ;
				if ( nReadBytes == 0 )
				{
					break ;
				}
				if ( pFile->Write( pbytTemp, nReadBytes ) < nReadBytes )
				{
					pszErrorMsg = "ファイルの書き込みに失敗しました" ;
					break ;
				}
				crc32.Stream( pbytTemp, nReadBytes ) ;
				//
				nDownloaded += nReadBytes ;
				int	nDownloadedKB = (int) (nDownloaded / 1024) ;
				pUI->SetProgressiveStatus( nDownloadedKB, nTotalKB ) ;
				//
				if ( pUI->IsProgressiveCanceled() )
				{
					m_flagAllFileWritable = flagSaveAllFileWritable ;
					return	errFailed ;
				}
				if ( nDownloadedKB > nLastDownloadedKB )
				{
					pUI->SetProgressiveMessage
						( strFileName
							+ L"... " + SString(nDownloadedKB)
							+ L"/" + SString(nTotalKB) + L"[kb]" ) ;
					nLastDownloadedKB = nDownloadedKB ;
				}
			}
			pdf->m_flagCheckCRC32 = true ;
			flagError = (nDownloaded != pdf->m_length)
						|| (crc32.GetCRC32() != pdf->m_crc32) ;
			if ( flagError )
			{
				ESLTrace( "failed to download %s: %d[bytes], CRC:%08X / info=%d[bytes], CRC:%08X\n",
						pdf->m_urlDownload.ToCharArray().GetConstArray(),
						(int) nDownloaded, crc32.GetCRC32(),
						(int) pdf->m_length, pdf->m_crc32 ) ;
			}
		}
		while ( false ) ;
		bufTemp.FinishArray() ;
		//
		if ( flagError )
		{
			#if	defined(__PLATFORM_WINDOWS__)
			if ( fServFileWrite )
			{
				pwcServWriter.RemoveFile( pdf->m_strLocalPath ) ;
			}
			else
			#endif
			{
				SFileOpener::DefaultRemoveFile( pdf->m_strLocalPath ) ;
			}
			if ( pszErrorMsg == NULL )
			{
				if ( flagOnline )
				{
					pszErrorMsg = "ファイルのダウンロードが失敗しました" ;
				}
				else
				{
					pszErrorMsg = "ファイルのコピーが失敗しました" ;
				}
			}
			pUI->DoMessageBox
				( SString(pszErrorMsg), m_strAppName ) ;
			m_flagAllFileWritable = flagSaveAllFileWritable ;
			return	errFailed ;
		}
		SString	strInstLogFile ;
		if ( GetEnvironmentString
			( strInstLogFile, L"cotopha\\install\\log_file" ) )
		{
			SakuraGL::SGLSetupFileManager	sfm ;
			if ( !sfm.LoadInstallLog( strInstLogFile ) )
			{
				sfm.GetFileAs( pdf->m_strLocalPath, true ) ;
				for ( int i = 0; i < 8; i ++ )
				{
					#if	defined(__PLATFORM_WINDOWS__)
					if ( fServFileWrite )
					{
						SSmartPointer<SFileInterface>
							pFile = pwcServWriter.NewOpenFile
								( strInstLogFile, SFileOpener::modeCreate ) ;
						if ( (pFile != NULL)
							&& !sfm.WriteInstallLog( *pFile ) )
						{
							break ;
						}
					}
					else
					#endif
					{
						if ( !sfm.SaveInstallLog( strInstLogFile ) )
						{
							break ;
						}
					}
					SleepMilliSec( 100 ) ;
				}
			}
		}
	}
	m_flagAllFileWritable = flagSaveAllFileWritable ;
	//
	#if	defined(__PLATFORM_WINDOWS__)
	if ( fServFileWrite )
	{
		pwcServWriter.ShutdownServer() ;
	}
	#endif
	return	errSuccess ;
}

// ダウンロードファイルをアーカイブファイルとして追加
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::AddDownloadedArchiveOpener( void )
{
	const size_t	nCount = m_arrayDownloads.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DownloadFile *	pdf = m_arrayDownloads.GetAt( i ) ;
		if ( (pdf != NULL) && pdf->m_flagArchive )
		{
			SString	strMachineID ;
			if ( pdf->m_flagCrypt32 )
			{
				strMachineID =
					SakuraGL::SGLStdApplication::GetMachineUniqueId() ;
			}
			SFileOpener *	pOpener =
				CreateArchiveOpener
					( pdf->m_strLocalPath, pdf->m_strPassword,
						false, -1, pdf->m_flagCrypt32, strMachineID ) ;
			if ( pOpener != NULL )
			{
				// アーカイブファイル追加登録
				AddFileOpener( pOpener, pdf->m_strID ) ;
			}
			if ( pdf->m_flagDynamicLoad && m_pxmlDynamicEnv )
			{
				size_t	i ;
				size_t	nCount = m_pxmlDynamicEnv->GetElementsCount() ;
				bool	fRegistered = false ;
				for ( i = 0; i < nCount; i ++ )
				{
					SXMLDocument *
						pxmlTag = m_pxmlDynamicEnv->GetElementAt( i ) ;
					if ( (pxmlTag == NULL)
						|| (pxmlTag->GetTag() != L"archive") )
					{
						continue ;
					}
					if ( pxmlTag->GetAttrStringAs( L"id" ) == pdf->m_strID )
					{
						fRegistered = true ;
						break ;
					}
				}
				if ( !fRegistered )
				{
					// 動的拡張環境ファイルへ追加
					SXMLDocument *	pxmlTag = new SXMLDocument ;
					pxmlTag->SetTag( L"archive" ) ;
					pxmlTag->SetAttributeAs( L"id", pdf->m_strID ) ;
					pxmlTag->SetAttributeAs( L"path", pdf->m_strLocalPath ) ;
					pxmlTag->SetAttributeAs( L"key", pdf->m_strPassword ) ;
					pxmlTag->SetAttributeAs( L"display_name", pdf->m_strDisplayName ) ;
					if ( pdf->m_flagUpdatable )
					{
						pxmlTag->SetAttributeAs( L"updatable", L"true" ) ;
					}
					if ( pdf->m_flagCrypt32 )
					{
						pxmlTag->SetAttributeAs( L"encrypt32", L"true" ) ;
					}
					if ( pdf->m_flagIndirect )
					{
						pxmlTag->SetAttributeAs
								( L"download", pdf->m_urlIndirect ) ;
					}
					else
					{
						SXMLDocument *	pxmlFile =
								pxmlTag->CreateElementTagAs( L"file" ) ;
						pxmlFile->SetAttributeAs( L"path", pdf->m_urlDownload ) ;
						pxmlFile->SetAttrHexIntegerAs( L"crc", pdf->m_crc32 ) ;
						pxmlFile->SetAttrIntegerAs( L"size", pdf->m_length ) ;
					}
					m_pxmlDynamicEnv->AddElement( pxmlTag ) ;
				}
			}
		}
	}
}

// ダウンロード済みファイル情報 (downloaded.xml) を読み込む
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::LoadDownloadedInfo( void )
{
	if ( m_pWritableOpener == NULL )
	{
		return	errFailed ;
	}
	SSmartPointer<SFileInterface>	pFile =
	#if	defined(__PLATFORM_ANDROID__)
		SFileOpener::DefaultNewOpenFile
			( L"local://downloaded.xml", SFileOpener::shareRead ) ;
	#else
		m_pWritableOpener->NewOpenFile
			( L"downloaded.xml", SFileOpener::shareRead ) ;
	#endif
	if ( pFile == NULL )
	{
		ESLTrace( "failed to open downloaded.xml\n" ) ;
		return	errFailed ;
	}
	SParserErrorInterface	perr ;
	return	m_xmlDownloads.ReadDocument( *pFile, perr ) ;
}

// ダウンロード済みファイル情報 (downloaded.xml) を書き出す
//////////////////////////////////////////////////////////////////////////////
SError SEnvironment::SaveDownloadedInfo( void )
{
	if ( m_pWritableOpener == NULL )
	{
		return	errFailed ;
	}
	SSmartPointer<SFileInterface>	pFile =
	#if	defined(__PLATFORM_ANDROID__)
		SFileOpener::DefaultNewOpenFile
			( L"local://downloaded.xml", SFileOpener::modeCreate ) ;
	#else
		m_pWritableOpener->NewOpenFile
			( L"downloaded.xml", SFileOpener::modeCreate ) ;
	#endif
	if ( pFile == NULL )
	{
		ESLTrace( "failed to save downloaded.xml\n" ) ;
		return	errFailed ;
	}
	return	m_xmlDownloads.WriteDocument( *pFile ) ;
}

// ダウンロード済み情報エントリを取得
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SEnvironment::GetDownloadedInfo( const wchar_t * pwszLocalPath )
{
	SXMLDocument *	pxmlDownload =
		m_xmlDownloads.GetElementTagAs( L"downloaded" ) ;
	if ( pxmlDownload == NULL )
	{
		return	NULL ;
	}
	const size_t	nCount = pxmlDownload->GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlFile = pxmlDownload->GetElementAt( i ) ;
		if ( (pxmlFile == NULL)
			|| (pxmlFile->GetTag() != L"file") )
		{
			continue ;
		}
		if ( pxmlFile->GetAttrStringAs( L"path" ) == pwszLocalPath )
		{
			return	pxmlFile ;
		}
	}
	return	NULL ;
}

// ダウンロード済み情報エントリを生成
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SEnvironment::CreateDownloadedInfo( const wchar_t * pwszLocalPath )
{
	SXMLDocument *	pxmlFile = GetDownloadedInfo( pwszLocalPath ) ;
	if ( pxmlFile != NULL )
	{
		return	pxmlFile ;
	}
	SXMLDocument *	pxmlDownload =
		m_xmlDownloads.CreateElementTagAs( L"downloaded" ) ;
	pxmlFile = new SXMLDocument ;
	pxmlDownload->AddElement( pxmlFile ) ;
	//
	pxmlFile->SetTag( L"file" ) ;
	pxmlFile->SetAttributeAs( L"path", pwszLocalPath ) ;
	//
	return	pxmlFile ;
}

// 進行状況ダイアログ表示
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::CreateProgressiveDialog( void )
{
	m_dlgProgressive.Create() ;
}

// 進行状況ダイアログ消去
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::CloseProgressiveDialog( void )
{
	m_dlgProgressive.Close() ;
}

// 進行状況ダイアログキャプション設定
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::SetProgressiveCaption( const wchar_t * pwszCaption )
{
	m_dlgProgressive.SetCaption( pwszCaption ) ;
}

// 進行状況ダイアログメッセージ設定
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::SetProgressiveMessage( const wchar_t * pwszMessage )
{
	m_dlgProgressive.SetMessage( pwszMessage ) ;
}

// 進行状況ダイアログメッセージ設定
//////////////////////////////////////////////////////////////////////////////
void SEnvironment::SetProgressiveStatus( int nCurrent, int nTotal )
{
	m_dlgProgressive.SetStatus( (uint32_t) nCurrent, (uint32_t) nTotal ) ;
}

// ユーザーがキャンセル操作したか？
//////////////////////////////////////////////////////////////////////////////
bool SEnvironment::IsProgressiveCanceled( void )
{
	return	m_dlgProgressive.IsCanceled() ;
}

// メッセージボックス表示
//////////////////////////////////////////////////////////////////////////////
int SEnvironment::DoMessageBox
	( const wchar_t * pwszMsg,
		const wchar_t * pwszCaption, int nStyles )
{
	return	m_dlgProgressive.DoMessageBox( pwszMsg, pwszCaption, nStyles ) ;
}

#endif
