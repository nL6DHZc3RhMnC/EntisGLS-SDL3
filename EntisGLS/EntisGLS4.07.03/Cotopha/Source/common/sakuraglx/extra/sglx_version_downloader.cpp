
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_http_file.h>
#include <sakuraglx/extra/sglx_version_downloader.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_sprite.h>
#include <sakuragl/sgl_erisa_lib.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakura/ssys_win_registry.h>
#include <shlobj.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// インストーラー・ファイルマネージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSetupFileManager, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSetupFileManager::SGLSetupFileManager( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSetupFileManager::~SGLSetupFileManager( void )
{
}

// インストールログファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSetupFileManager::LoadInstallLog( const wchar_t * pwszLogFile )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::GetDefaultOpener()->
			NewOpenFile( pwszLogFile, SFileOpener::shareRead ) ;
	if ( pFile != NULL )
	{
		SError	err = m_xmlSetup.ReadDocument( *pFile, m_xmlSetup ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		SError	err = m_xmlSetup.LoadDocument( pwszLogFile, m_xmlSetup ) ;
		if ( err )
		{
			return	err ;
		}
	}
	m_strLogFile = pwszLogFile ;
	return	errSuccess ;
}

// インストールログファイル書き出し
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSetupFileManager::SaveInstallLog( const wchar_t * pwszLogFile )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::GetDefaultOpener()->
			NewOpenFile( pwszLogFile, SFileOpener::modeCreate ) ;
	if ( pFile != NULL )
	{
		return	m_xmlSetup.WriteDocument( *pFile ) ;
	}
	else
	{
		return	m_xmlSetup.SaveDocument( pwszLogFile ) ;
	}
}

SSystem::SError SGLSetupFileManager::WriteInstallLog( SFileInterface& file )
{
	return	m_xmlSetup.WriteDocument( file ) ;
}

// 内容消去
//////////////////////////////////////////////////////////////////////////////
void SGLSetupFileManager::ClearAll( void )
{
	m_strLogFile.FreeArray() ;
	m_xmlSetup.RemoveAllContents() ;
}

// ファイルとディレクトリを消去
//////////////////////////////////////////////////////////////////////////////
bool SGLSetupFileManager::UninstallFiles( void )
{
	SXMLDocument *	pxmlDir = m_xmlSetup.GetElementTagAs( L"setup" ) ;
	if ( pxmlDir == NULL )
	{
		return	false ;
	}
	return	UninstallDirFiles( pxmlDir, L"" ) ;
}

bool SGLSetupFileManager::UninstallDirFiles
	( SXMLDocument * pxmlDir, const wchar_t * pwszBaseDir )
{
	SString	strBaseDir = pwszBaseDir ;
	bool	fReqReboot = false ;
	for ( size_t i = 0; i < pxmlDir->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = pxmlDir->GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetType() != SXMLDocument::typeTag) )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"directory" )
		{
			SString	strDirPath =
				strBaseDir.OffsetFilePath
					( pxmlTag->GetAttrStringAs( L"path" ) ) ;
			if ( UninstallDirFiles( pxmlTag, strDirPath ) )
			{
				fReqReboot = true ;
			}
			if ( pxmlTag->GetAttrIntegerAs( L"created" ) )
			{
				SFile::RemoveDirectory( strDirPath ) ;
			}
		}
		else if ( pxmlTag->GetTag() == L"file" )
		{
			SString	strFilePath =
				strBaseDir.OffsetFilePath
					( pxmlTag->GetAttrStringAs( L"path" ) ) ;
			if ( SFile::RemoveFile( strFilePath ) )
			{
				#if	defined(__PLATFORM_WINDOWS__)
				if ( g_infoPlatform.runtimeOS != platformOS_Windows )
				{
					::MoveFileExW
						( strFilePath,
							NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ;
				}
				else
				{
					SString	strWinDir ;
					SFile::GetDefaultDirectory
						( strWinDir, SFile::DefaultDirectory::WindowsDirectory ) ;
					SString	strWinIni =
						strWinDir.OffsetFilePath( L"WININIT.INI" ) ;
					//
					SArray<char>	bufShortPath ;
					SArray<char>	bufFilePath ;
					::GetShortPathName
						( strFilePath.EncodeDefaultTo(bufFilePath),
							bufShortPath.GetArray(MAX_PATH+1), MAX_PATH ) ;
					bufShortPath.FinishArray() ;
					//
					SArray<char>	bufWinIni ;
					::WritePrivateProfileString
						( "Rename", "NUL",
							bufShortPath.GetConstArray(),
							strWinIni.EncodeDefaultTo(bufWinIni) ) ;
				}
				#endif
				fReqReboot = true ;
			}
		}
	}
	return	fReqReboot ;
}

// ディレクトリを検索
//////////////////////////////////////////////////////////////////////////////
SSystem::SXMLDocument *
	SGLSetupFileManager::GetDirectoryAs
		( const wchar_t * pwszDirPath, bool flagCreate )
{
	SXMLDocument *	pxmlDir = m_xmlSetup.CreateElementTagAs( L"setup" ) ;
	SString	strDirPath = pwszDirPath ;
	if ( strDirPath.IsEmpty() )
	{
		return	pxmlDir ;
	}
	size_t	i = 0 ;
	while ( i < pxmlDir->GetElementsCount() )
	{
		SXMLDocument *	pxmlTag = pxmlDir->GetElementAt( i ++ ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetType() != SXMLDocument::typeTag)
			|| (pxmlTag->GetTag() != L"directory") )
		{
			continue ;
		}
		SString	strDir = pxmlTag->GetAttrStringAs( L"path" ) ;
		if ( strDir.CompareNoCase( strDirPath ) == 0 )
		{
			// 一致するタグが既に存在している
			return	pxmlTag ;
		}
		size_t	lenDir = strDir.GetLength() ;
		if ( strDir.CompareNoCase( strDirPath.Left( lenDir ) ) != 0 )
		{
			continue ;
		}
		if ( strDirPath.GetLength() < lenDir )
		{
			continue ;
		}
		wchar_t	wchSep = strDirPath.GetAt( lenDir ) ;
		if ( (wchSep == L'\\') || (wchSep == L'/') )
		{
			strDirPath = strDirPath.Middle( lenDir + 1 ) ;
			if ( strDirPath.IsEmpty() )
			{
				// 一致するタグが既に存在している
				return	pxmlTag ;
			}
			pxmlDir = pxmlTag ;
			i = 0 ;
		}
		else
		{
			continue ;
		}
	}
	if ( flagCreate )
	{
		// ディレクトリエントリ追加
		SXMLDocument *	pxmlNewDir = new SXMLDocument ;
		pxmlNewDir->SetTag( L"directory" ) ;
		pxmlNewDir->SetAttributeAs( L"path", strDirPath ) ;
		pxmlNewDir->SetAttributeAs( L"created", L"1" ) ;
		pxmlDir->AddElement( pxmlNewDir ) ;
		return	pxmlNewDir ;
	}
	return	NULL ;
}

// ファイルを検索
//////////////////////////////////////////////////////////////////////////////
SSystem::SXMLDocument *
	SGLSetupFileManager::GetFileAs
		( const wchar_t * pwszFilePath, bool flagCreate )
{
	SString	strFilePath = pwszFilePath ;
	SString	strFileDir = strFilePath.GetFileDirectoryPart() ;
	if ( (strFileDir.GetLastAt(0) == L'\\')
		|| (strFileDir.GetLastAt(0) == L'/') )
	{
		strFileDir = strFileDir.Left( strFileDir.GetLength() - 1 ) ;
	}
	SXMLDocument *	pxmlDir = GetDirectoryAs( strFileDir, flagCreate ) ;
	if ( pxmlDir == NULL )
	{
		return	NULL ;
	}
	SString	strFileName = strFilePath.GetFileNamePart() ;
	SXMLDocument *	pxmlFile = GetFileAs( pxmlDir, strFileName ) ;
	if ( (pxmlFile == NULL) && flagCreate )
	{
		pxmlFile = new SXMLDocument ;
		pxmlFile->SetTag( L"file" ) ;
		pxmlFile->SetAttributeAs( L"path", strFileName ) ;
		pxmlDir->AddElement( pxmlFile ) ;
	}
	return	pxmlFile ;
}

SSystem::SXMLDocument *
	SGLSetupFileManager::GetFileAs
		( SSystem::SXMLDocument * pxmlDir, const wchar_t * pwszFileName ) const
{
	for ( size_t i = 0; i < pxmlDir->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = pxmlDir->GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetType() != SXMLDocument::typeTag)
			|| (pxmlTag->GetTag() != L"file") )
		{
			continue ;
		}
		SString	strFilePath = pxmlTag->GetAttrStringAs( L"path" ) ;
		SString	strFileName = strFilePath.GetFileNamePart() ;
		if ( strFileName.CompareNoCase( pwszFileName ) == 0 )
		{
			return	pxmlTag ;
		}
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// リスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLVersionDownloader::DownloadListener, ESLObject )

// ダウンロード開始
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::DownloadListener::OnBeginDownload
	( const wchar_t * pwszPackageName,
		size_t iFile, size_t nFileCount,
		uint64_t nCurrentTotal, uint64_t nTotalBytes,
		const wchar_t * pwszName,
		const wchar_t * pwszDstPath, const wchar_t * pwszSrcURL )
{
}

// ダウンロード完了
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::DownloadListener::OnFinishDownload( void )
{
}

// ダウンロード進行度
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::DownloadListener::OnDownloading
			( uint64_t nBytes, uint64_t nTotal )
{
	return	errSuccess ;
}

// エラー
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::DownloadListener::OnError( const wchar_t * pwszErrMsg )
{
	SSystem::MessageBox( pwszErrMsg ) ;
}


//////////////////////////////////////////////////////////////////////////////
// バージョンチェック・ダウンローダ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVersionDownloader, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLVersionDownloader::SGLVersionDownloader( void )
{
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLVersionDownloader::~SGLVersionDownloader( void )
{
}

// インストールログファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::LoadInstallLog( const wchar_t * pwszLogFile )
{
	return	m_fmLog.LoadInstallLog( pwszLogFile ) ;
}

// インストールログファイル書き出し
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::SaveInstallLog( const wchar_t * pwszLogFile )
{
	return	m_fmLog.SaveInstallLog( pwszLogFile ) ;
}

// ダウンロードリスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::AttachListener
	( SGLVersionDownloader::DownloadListener * pListener )
{
	m_pListener = pListener ;
}

// 定義ファイルをダウンロード
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	SGLVersionDownloader::DownloadVersionList( const wchar_t * pwszURL )
{
	m_strBaseURL = SString( pwszURL ).GetFileDirectoryPart() ;
	return	m_xmlDoc.LoadDocument( pwszURL, m_xmlDoc ) ;
}

// 条件に一致するバージョンを検索する
//////////////////////////////////////////////////////////////////////////////
size_t SGLVersionDownloader::EnumMatchVersion
	( SSystem::SPointerArray
		<SSystem::SXMLDocument>& aPackages, int64_t nCurVersion )
{
	aPackages.RemoveAll() ;
	//
	SXMLDocument *	pxmlVersions = m_xmlDoc.GetElementTagAs( L"versions" ) ;
	if ( pxmlVersions == NULL )
	{
		return	0 ;
	}
	//
	RSVirtualMachine	vm ;
	vm.Initialize( 0 ) ;
	//
	RSContext *	pContext = vm.GetSystemContext() ;
	vm.CreateMemberIntegerAs( *pContext, L"version", nCurVersion ) ;
	//
	for ( size_t i = 0; i < pxmlVersions->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlPackage = pxmlVersions->GetElementAt( i ) ;
		if ( (pxmlPackage == NULL)
			|| (pxmlPackage->GetType() != SXMLDocument::typeTag)
			|| (pxmlPackage->GetTag() != L"package") )
		{
			continue ;
		}
		SString	strCondition = pxmlPackage->GetAttrStringAs( L"condition" ) ;
		RSSmartPtr	pCondObj
			( pContext->PerformExpression( strCondition ), pContext ) ;
		if ( pContext->IsException() )
		{
			pContext->ClearException() ;
		}
		else if ( (pCondObj != NULL) && pCondObj->AsBoolean() )
		{
			aPackages.Add( pxmlPackage ) ;
		}
	}
	return	aPackages.GetLength() ;
}

// バージョン情報取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::GetPackageInfo
	( SGLVersionDownloader::PackageInfo& pckinf,
					SSystem::SXMLDocument * pxmlPackage ) const
{
	pckinf.m_nVersion = (int) pxmlPackage->GetAttrIntegerAs( L"version" ) ;
	pckinf.m_strDisplayName = pxmlPackage->GetAttrStringAs( L"display_name" ) ;
	pckinf.m_strDescription = pxmlPackage->GetAttrStringAs( L"description" ) ;
	pckinf.m_nFileCount = 0 ;
	pckinf.m_nTotalBytes = 0 ;
	for ( size_t i = 0; i < pxmlPackage->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlFile = pxmlPackage->GetElementAt( i ) ;
		if ( (pxmlFile == NULL)
			|| (pxmlFile->GetTag() != L"file") )
		{
			continue ;
		}
		pckinf.m_nFileCount ++ ;
		pckinf.m_nTotalBytes += pxmlFile->GetAttrIntegerAs( L"size" ) ;
	}
	return	errSuccess ;
}

// ダウンロード実行
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::DoDownloadPackage
	( SSystem::SXMLDocument * pxmlPackage, const wchar_t * pwszInstallDir )
{
	SObjectArray<DownloadFile>	aDownloadFiles ;
	SString	strInstDir = pwszInstallDir ;
	SError	err = errSuccess ;
	//
	PackageInfo	pckinf ;
	GetPackageInfo( pckinf, pxmlPackage ) ;
	//
	uint64_t	nCurrentDownload = 0 ;
	for ( size_t i = 0; i < pxmlPackage->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlFile = pxmlPackage->GetElementAt( i ) ;
		if ( (pxmlFile == NULL)
			|| (pxmlFile->GetTag() != L"file") )
		{
			continue ;
		}
		//
		// 情報収集
		//
		SString	strLocalPath = pxmlFile->GetAttrStringAs( L"path" ) ;
		SString	strURL = pxmlFile->GetAttrStringAs( L"url" ) ;
		SString	strName =
			pxmlFile->GetAttrStringAs
				( L"name", strLocalPath.GetFileTitlePart() ) ;
		uint64_t	nFileSize =
				(uint64_t) pxmlFile->GetAttrIntegerAs( L"size" ) ;
		uint32_t	nCRC32 =
				(uint32_t) pxmlFile->GetAttrHexIntegerAs( L"crc32" ) ;
		//
		if ( m_pListener != NULL )
		{
			m_pListener->OnBeginDownload
				( pckinf.m_strDisplayName,
					i, pckinf.m_nFileCount,
					nCurrentDownload, pckinf.m_nTotalBytes,
					strName, strLocalPath, strURL ) ;
			err = m_pListener->OnDownloading( 0, nFileSize ) ;
			if ( err )
			{
				break ;
			}
		}
		//
		// テンポラリファイル名生成
		//
		SString	strTempFile =
			strInstDir.OffsetFilePath( strLocalPath + L".tmp" ) ;
		if ( SFile::IsExistingFile( strTempFile ) )
		{
			for ( int i = 0; i <= 999; i ++ )
			{
				SString	strNum( i, 3, 10 ) ;
				strTempFile =
					strInstDir.OffsetFilePath
						( strLocalPath + L"." + strNum ) ;
				if ( !SFile::IsExistingFile( strTempFile ) )
				{
					break ;
				}
			}
		}
		//
		// テンポラリファイル生成
		//
		SSmartPointer<SFileInterface>	pDstFile =
				SFile::NewOpen( strTempFile, SFileOpener::modeCreate ) ;
		if ( pDstFile == NULL )
		{
			if ( m_pListener != NULL )
			{
				m_pListener->OnError
					( strTempFile + L" を生成できませんでした" ) ;
				m_pListener->OnFinishDownload() ;
			}
			err = errFailed ;
			break ;
		}
		DownloadFile *	pdf = new DownloadFile ;
		pdf->m_strDstFile = strLocalPath ;
		pdf->m_strTempFile = strTempFile ;
		aDownloadFiles.Add( pdf ) ;
		//
		// ファイルダウンロード開始
		//
		SSmartPointer<SFileInterface>	pSrcFile =
			SFileOpener::DefaultNewOpenFile
				( m_strBaseURL.OffsetFilePath
						( strURL, L'/' ), SFileOpener::shareRead ) ;
		if ( pSrcFile == NULL )
		{
			if ( m_pListener != NULL )
			{
				m_pListener->OnError
					( strURL + L" をダウンロードできませんでした" ) ;
				m_pListener->OnFinishDownload() ;
			}
			err = errFailed ;
			break ;
		}
		//
		// 順次ダウンロード
		//
		SakuraCL::CRC32Context	ctxCRC32 ;
		SByteBuffer	buf ;
		size_t		nBufBytes = 0x10000 ;
		uint8_t *	pbytBuf = buf.GetArray( nBufBytes ) ;
		uint64_t	nDownloaded = 0 ;
		//
		for ( ; ; )
		{
			size_t	nReadBytes = pSrcFile->Read( pbytBuf, nBufBytes ) ;
			if ( nReadBytes == 0 )
			{
				break ;
			}
			ctxCRC32.Stream( pbytBuf, nReadBytes ) ;
			//
			size_t	nWrittenBytes = pDstFile->Write( pbytBuf, nReadBytes ) ;
			if ( nWrittenBytes < nReadBytes )
			{
				m_pListener->OnError
					( L"ファイルへの書き出しが出来ません。\n"
						L"ディスクの空き容量を確認してください。" ) ;
				m_pListener->OnFinishDownload() ;
				err = errFailed ;
				break ;
			}
			nDownloaded += nReadBytes ;
			if ( m_pListener != NULL )
			{
				err = m_pListener->OnDownloading( nDownloaded, nFileSize ) ;
				if ( err )
				{
					break ;
				}
			}
		}
		buf.FinishArray() ;
		//
		if ( !err && (ctxCRC32.GetCRC32() != nCRC32) )
		{
			if ( m_pListener != NULL )
			{
				m_pListener->OnError( L"CRC が一致しません。" ) ;
				m_pListener->OnFinishDownload() ;
			}
			err = errFailed ;
		}
		if ( err )
		{
			break ;
		}
		if ( m_pListener != NULL )
		{
			m_pListener->OnFinishDownload() ;
		}
		nCurrentDownload += nFileSize ;
	}
	if ( err )
	{
		//
		// 一時ファイルを削除
		//
		for ( size_t i = 0; i < aDownloadFiles.GetLength(); i ++ )
		{
			DownloadFile *	pdf = aDownloadFiles.GetAt( i ) ;
			ESLAssert( pdf != NULL ) ;
			SFile::RemoveFile( pdf->m_strTempFile ) ;
		}
	}
	else
	{
		//
		// 一時ファイルを確定させる
		//
		for ( size_t i = 0; i < aDownloadFiles.GetLength(); i ++ )
		{
			DownloadFile *	pdf = aDownloadFiles.GetAt( i ) ;
			ESLAssert( pdf != NULL ) ;
			SString	strDstFile =
					strInstDir.OffsetFilePath( pdf->m_strDstFile ) ;
			//
			if ( SFile::IsExistingFile( strDstFile ) )
			{
				SFile::RemoveFile( strDstFile ) ;
			}
			if ( SFile::RenameFile( pdf->m_strTempFile, strDstFile ) )
			{
				DownloadFile *	pdfDelay = new DownloadFile ;
				pdfDelay->m_strTempFile = pdf->m_strTempFile ;
				pdfDelay->m_strDstFile = strDstFile ;
				m_aDelayRename.Add( pdfDelay ) ;
			}
			else
			{
				m_fmLog.GetFileAs( strDstFile, true ) ;
			}
		}
	}
	return	err ;
}

// 置き換えが必要なファイルの有無
//////////////////////////////////////////////////////////////////////////////
bool SGLVersionDownloader::ShouldRenameFile( void ) const
{
	return	(m_aDelayRename.GetLength() > 0) ;
}

// ファイルの置き換えを実行
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::DoRenameFileList( void )
{
	SError	err = errSuccess ;
	for ( size_t i = 0; i < m_aDelayRename.GetLength(); i ++ )
	{
		DownloadFile *	pdf = m_aDelayRename.GetAt( i ) ;
		ESLAssert( pdf != NULL ) ;
		if ( pdf == NULL )
		{
			continue ;
		}
		if ( !SFile::IsExistingFile( pdf->m_strTempFile ) )
		{
			m_aDelayRename.RemoveAt( i -- ) ;
			continue ;
		}
		if ( SFile::IsExistingFile( pdf->m_strDstFile ) )
		{
			SFile::RemoveFile( pdf->m_strDstFile ) ;
		}
		if ( SFile::RenameFile( pdf->m_strTempFile, pdf->m_strDstFile ) )
		{
			err = errPending ;
		}
		else
		{
			m_fmLog.GetFileAs( pdf->m_strDstFile, true ) ;
			m_aDelayRename.RemoveAt( i -- ) ;
		}
	}
	return	err ;
}

// 置き換え元一時ファイルを削除（書き換えに失敗したファイルを削除するため）
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::DoDeleteTemporaryFileList( void )
{
	for ( size_t i = 0; i < m_aDelayRename.GetLength(); i ++ )
	{
		DownloadFile *	pdf = m_aDelayRename.GetAt( i ) ;
		ESLAssert( pdf != NULL ) ;
		if ( pdf == NULL )
		{
			continue ;
		}
		SFile::RemoveFile( pdf->m_strTempFile ) ;
	}
	m_aDelayRename.RemoveAll() ;
}

// 置き換えが必要なファイルリストをファイルに保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVersionDownloader::SaveFileListToRename( const wchar_t * pwszListFile )
{
	SXMLDocument	xmlRename ;
	FormatFileListToRename( xmlRename ) ;
	//
	xmlRename.SetTag( L"list" ) ;
	return	xmlRename.SaveDocument( pwszListFile ) ;
}

// 置き換えが必要なファイルリストを XML に出力
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::FormatFileListToRename( SSystem::SXMLDocument& xmlRename )
{
	for ( size_t i = 0; i < m_aDelayRename.GetLength(); i ++ )
	{
		DownloadFile *	pdf = m_aDelayRename.GetAt( i ) ;
		ESLAssert( pdf != NULL ) ;
		if ( pdf == NULL )
		{
			continue ;
		}
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		pxmlTag->SetTag( L"rename" ) ;
		pxmlTag->SetAttributeAs( L"old", pdf->m_strTempFile ) ;
		pxmlTag->SetAttributeAs( L"new", pdf->m_strDstFile ) ;
		xmlRename.AddElement( pxmlTag ) ;
	}
}

// 置き換えが必要なファイルリストを XML から入力
//////////////////////////////////////////////////////////////////////////////
void SGLVersionDownloader::ParseFileListToRename( const SSystem::SXMLDocument& xmlRename )
{
	for ( size_t i = 0; i < xmlRename.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlRename.GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetTag() != L"rename") )
		{
			continue ;
		}
		DownloadFile *	pdf = new DownloadFile ;
		pdf->m_strTempFile = pxmlTag->GetAttrStringAs( L"old" ) ;
		pdf->m_strDstFile = pxmlTag->GetAttrStringAs( L"new" ) ;
		m_aDelayRename.Add( pdf ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// ランチャー・アプリケーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLLauncherApplication, SGLStdApplication )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLLauncherApplication::SGLLauncherApplication( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLLauncherApplication::~SGLLauncherApplication( void )
{
}

// プロファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLLauncherApplication::GetProfileFilePath( SSystem::SString& strFilePath ) const
{
	strFilePath.FreeArray() ;
	return	NULL ;
}

// 実行
//////////////////////////////////////////////////////////////////////////////
int SGLLauncherApplication::Run( void )
{
	SString	strRenameList ;
	if ( m_env.GetEnvironmentString
			( strRenameList, L"launcher\\rename\\src" ) )
	{
		SGLVersionDownloader	vdl ;
		SXMLDocument	xmlRename ;
		if ( !xmlRename.LoadDocument( strRenameList, xmlRename ) )
		{
			SXMLDocument *	pxmlList = xmlRename.GetElementTagAs( L"list" ) ;
			if ( pxmlList != NULL )
			{
				vdl.ParseFileListToRename( *pxmlList ) ;
				for ( int i = 0; (i < 8) && vdl.ShouldRenameFile(); i ++ )
				{
					vdl.DoRenameFileList() ;
					SleepMilliSec( 100 ) ;
				}
				if ( vdl.ShouldRenameFile() )
				{
					SString	strErrMsg ;
					if ( !m_env.GetEnvironmentString
							( strErrMsg, L"launcher\\rename\\failed_msg" ) )
					{
						strErrMsg = L"ファイルの更新に失敗しました。" ;
					}
					return	1 ;
				}
				else
				{
					SFile::RemoveFile( strRenameList ) ;
				}
			}
		}
	}
	SString	strCmdLine ;
	if ( m_env.GetEnvironmentString
			( strCmdLine, L"launcher\\default\\cmd_line" ) )
	{
		SStringParser	sparsCmdLine ;
		sparsCmdLine.AttachString( strCmdLine ) ;
		//
		SString	strAppExe, strParam ;
		sparsCmdLine.NextStringTerm( strAppExe ) ;
		sparsCmdLine.PassSpace() ;
		strParam = sparsCmdLine.SubStringFrom( sparsCmdLine.GetIndex() ) ;
		//
		if ( !strAppExe.IsEmpty() )
		{
			SSystem::OpenShellFile( strParam, shellOpenURI, strAppExe ) ;
		}
	}
	return	0 ;
}


#if	!defined(__COTOPHA__)

//////////////////////////////////////////////////////////////////////////////
// アップデーター・アプリケーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLUpdaterApplication, SGLStdApplication, DownloadListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLUpdaterApplication::SGLUpdaterApplication( void )
{
	m_nAppVersion = -1 ;
	m_fPrivilegeWriteServer = false ;

	#if	defined(__PLATFORM_WINDOWS__)
	m_hConnectProcess = NULL ;
	m_hWndServer = NULL ;
	m_procServ = NULL ;
	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLUpdaterApplication::~SGLUpdaterApplication( void )
{
	#if	defined(__PLATFORM_WINDOWS__)
	if ( m_hConnectProcess != NULL )
	{
		::CloseHandle( m_hConnectProcess ) ;
		m_hConnectProcess = NULL ;
	}
	#endif
}

// 引数解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLUpdaterApplication::ParseCmdLine( const wchar_t * pwszArg )
{
	SStringParser	sparsArg = pwszArg ;
	while ( sparsArg.PassSpace() )
	{
		SString	strTerm ;
		sparsArg.NextStringTerm( strTerm ) ;
		if ( strTerm.GetAt(0) == L'/' )
		{
			if ( strTerm.CompareNoCase( L"/version" ) == 0 )
			{
				m_nAppVersion = sparsArg.NextInteger() ;
			}
			else if ( strTerm.CompareNoCase( L"/write_server" ) == 0 )
			{
				m_fPrivilegeWriteServer = true ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 環境設定ファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLUpdaterApplication::GetEnvironmentFilePath( SSystem::SString& strFilePath ) const
{
#if	defined(__PLATFORM_ANDROID__)
	strFilePath = L"assets://updater.xml" ;
#else
	strFilePath = L"updater.xml" ;
#endif
	return	strFilePath ;
}

// プロファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLUpdaterApplication::GetProfileFilePath( SSystem::SString& strFilePath ) const
{
	strFilePath.FreeArray() ;
	return	NULL ;
}

// 実行
//////////////////////////////////////////////////////////////////////////////
int SGLUpdaterApplication::Run( void )
{
	if ( m_fPrivilegeWriteServer )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			return	DoWriteServer() ;
		#else
			return	1 ;
		#endif
	}
	else
	{
		return	DoUpdate() ;
	}
}

// アップデート実行
//////////////////////////////////////////////////////////////////////////////
int SGLUpdaterApplication::DoUpdate( void )
{
	SString	strAppName ;
	m_env.GetApplicationName( strAppName ) ;
	//
	// 起動元アプリの終了を待つ
	//
	STimeCounter	timer ;
	#if	defined(__PLATFORM_WINDOWS__)
	for ( ; ; )
	{
		SleepMilliSec( 100 ) ;
		if ( !CheckMultiBoot() )
		{
			break ;
		}
		if ( timer.GetTime() >= 2000 )
		{
			if ( SSystem::MessageBox
				( L"アプリケーションを終了してください",
					strAppName, msgboxStyleOkCancel ) != msgboxResultOk )
			{
				return	-1 ;
			}
			timer.Reset() ;
		}
	}
	#endif
	//
	// 更新パッケージ取得
	//
	if ( m_nAppVersion < 0 )
	{
		return	-1 ;
	}
	bool	fNoUpdateMsg = true ;
	do
	{
		SleepMilliSec( 500 ) ;
		//
		if ( m_strUpdateURL.IsEmpty() )
		{
			if ( !m_env.GetEnvironmentString
					( m_strUpdateURL, L"cotopha\\update\\url" ) )
			{
				break ;
			}
		}
		if ( m_strInstallLog.IsEmpty() )
		{
			m_env.GetEnvironmentString
					( m_strInstallLog, L"cotopha\\install\\log_file" ) ;
		}
		SGLVersionDownloader	verdl ;
		if ( verdl.DownloadVersionList( m_strUpdateURL ) )
		{
			SSystem::MessageBox
				( L"更新情報を取得できませんでした", strAppName, msgboxStyleOk ) ;
			fNoUpdateMsg = false ;
			break ;
		}
		SPointerArray<SXMLDocument>	lstPackages ;
		if ( verdl.EnumMatchVersion( lstPackages, m_nAppVersion ) == 0 )
		{
			break ;
		}
		//
		if ( !m_strInstallLog.IsEmpty() )
		{
			verdl.LoadInstallLog( m_strInstallLog ) ;
		}
		//
		SString	strCurDir ;
		SFile::GetDefaultDirectory
			( strCurDir, SFile::DefaultDirectory::CurrentDirectory ) ;
		//
		verdl.AttachListener( this ) ;
		m_dlgProgress.SetCaption( strAppName ) ;
		m_dlgProgress.Create( SProgressiveDialog::flagStyleSpinner ) ;
		//
		SError	err = errSuccess ;
		for ( size_t i = 0; i < lstPackages.GetLength(); i ++ )
		{
			SXMLDocument *	pxmlPackage = lstPackages.GetAt( i ) ;
			if ( pxmlPackage == NULL )
			{
				continue ;
			}
			err = verdl.DoDownloadPackage( pxmlPackage, strCurDir ) ;
			if ( err )
			{
				break ;
			}
			size_t	j = 0 ;
			while ( verdl.ShouldRenameFile() )
			{
				SleepMilliSec( 100 ) ;
				if ( !verdl.DoRenameFileList() )
				{
					break ;
				}
				if ( ++ j >= 10 )
				{
					if ( m_dlgProgress.DoMessageBox
						( L"ファイルの置き換えに失敗しました。",
							strAppName, msgboxStyleRetryCancel )
												!= msgboxResultRetry )
					{
						verdl.DoDeleteTemporaryFileList() ;
						break ;
					}
					j = 0 ;
				}
			}
			if ( !m_strInstallLog.IsEmpty() )
			{
				verdl.SaveInstallLog( m_strInstallLog ) ;
			}
			fNoUpdateMsg = false ;
		}
		//
		m_dlgProgress.Close() ;
		if ( err )
		{
			return	-1 ;
		}
	}
	while ( false ) ;
	//
	if ( fNoUpdateMsg )
	{
		SSystem::MessageBox
			( L"更新はありません", strAppName, msgboxStyleOk ) ;
		return	0 ;
	}
	//
	// アプリを起動する
	//
	if ( m_strAppCmdLine.IsEmpty() )
	{
		if ( !m_env.GetEnvironmentString
				( m_strAppCmdLine, L"cotopha\\update\\cmd" ) )
		{
			return	-1 ;
		}
	}
	//
	SStringParser	sparsCmdLine ;
	sparsCmdLine.AttachString( m_strAppCmdLine ) ;
	//
	SString	strExe, strParams ;
	sparsCmdLine.NextStringTerm( strExe ) ;
	if ( sparsCmdLine.PassSpace() )
	{
		strParams = sparsCmdLine.SubString( sparsCmdLine.GetIndex() ) ;
	}
	//
	#if	defined(__PLATFORM_WINDOWS__)
	ReleaseExclusiveBoot() ;
	#endif
	//
	if ( OpenShellFile( strParams, shellOpenURI, strExe, NULL ) )
	{
		return	0 ;
	}
	return	-1 ;
}

// ダウンロード開始
//////////////////////////////////////////////////////////////////////////////
void SGLUpdaterApplication::OnBeginDownload
	( const wchar_t * pwszPackageName,
		size_t iFile, size_t nFileCount,
		uint64_t nCurrentTotal, uint64_t nTotalBytes,
		const wchar_t * pwszName,
		const wchar_t * pwszDstPath, const wchar_t * pwszSrcURL )
{
	SString	strDstPath = pwszDstPath ;
	m_strPackagenName = pwszPackageName ;
	m_strFileName = strDstPath.GetFileNamePart() ;
	m_iCurFile = iFile ;
	m_nFileCount = nFileCount ;
	//
	m_strMsgBase =
		m_strPackagenName + L" [" + SString(m_iCurFile + 1)
								+ L"/" + SString(m_nFileCount) + " files]\n"
		+ m_strFileName ;
	m_dlgProgress.SetMessage( m_strMsgBase ) ;
}

// ダウンロード完了
//////////////////////////////////////////////////////////////////////////////
void SGLUpdaterApplication::OnFinishDownload( void )
{
}

// ダウンロード進行度
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLUpdaterApplication::OnDownloading
			( uint64_t nBytes, uint64_t nTotal )
{
	SString	strMsg =
		m_strMsgBase + L" [" + SString( nBytes >> 10 )
						+ L"/" + SString( nTotal >> 10 ) + L"KB]" ;
	m_dlgProgress.SetMessage( strMsg ) ;
	//
	while ( nTotal >= 0x10000 )
	{
		nBytes >>= 1 ;
		nTotal >>= 1 ;
	}
	m_dlgProgress.SetStatus( (uint32_t) nBytes, (uint32_t) nTotal ) ;
	//
	if ( m_dlgProgress.IsCanceled() )
	{
		return	errAbort ;
	}
	return	errSuccess ;
}

// エラー
//////////////////////////////////////////////////////////////////////////////
void SGLUpdaterApplication::OnError( const wchar_t * pwszErrMsg )
{
	SString	strAppName ;
	m_env.GetApplicationName( strAppName ) ;
	SSystem::MessageBox( pwszErrMsg, strAppName, msgboxStyleOk ) ;
}


#if	defined(__PLATFORM_WINDOWS__)

// 特権ファイル書き込みサーバー実行
//////////////////////////////////////////////////////////////////////////////
int SGLUpdaterApplication::DoWriteServer( void )
{
	//
	// ChangeWindowMessageFilter 設定
	//
	#if	!defined(MSGFLT_ADD)
	enum	ChangeWindowMessageFilterFlags
	{
		MSGFLT_ADD		= 1,
		MSGFLT_REMOVE	= 2,
	} ;
	#endif
	typedef BOOL (WINAPI *API_ChangeWindowMessageFilter)( UINT message, DWORD dwFlag ) ;
	API_ChangeWindowMessageFilter	apiChangeWindowMessageFilter = NULL ;
	//
	HMODULE	hUser32 = ::LoadLibrary( "user32.dll" ) ;
	if ( hUser32 != NULL )
	{
		apiChangeWindowMessageFilter =
			(API_ChangeWindowMessageFilter)
				::GetProcAddress( hUser32, "ChangeWindowMessageFilter" ) ;
		if ( apiChangeWindowMessageFilter != NULL )
		{
			apiChangeWindowMessageFilter( wmShutdown, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmConnect, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmDisonnect, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmOpenFile, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmCloseFile, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmWriteAsync, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmReadAsync, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmSeek, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmGetLength, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmTruncate, MSGFLT_ADD ) ;
			apiChangeWindowMessageFilter( wmDeleteFile, MSGFLT_ADD ) ;
		}
	}
	//
	// 非同期処理用初期化
	//
	m_sigWriteReq.Initialize( false ) ;
	m_sigExitThread.Initialize( false ) ;
	m_procServ = new ServerProc( this ) ;
	m_threadServ.BeginThread( m_procServ ) ;
	//
	// ウィンドウ作成
	//
	const char *	pszClassName = RegisterWindowClass() ;
	m_hWndServer = ::CreateWindowEx
		( 0, pszClassName, "EntisGLS4_PrivilegeWriteServer",
			WS_CAPTION | WS_SYSMENU,
			0, 0, 100, 100, NULL, NULL, ::GetModuleHandle(NULL), this ) ;
	if ( (m_hWndServer == NULL) || !::IsWindow( m_hWndServer ) )
	{
		ESLTrace( "failed to create window.\n" ) ;
		return	1 ;
	}
	//
	// ウィンドウ初期設定
	//
	::SetWindowLongPtr
		( m_hWndServer, GWLP_WNDPROC,
				(LONG_PTR) &SGLUpdaterApplication::WindowCallbackProc ) ;
	::SetWindowLongPtr( m_hWndServer, GWLP_USERDATA, (LONG_PTR) this ) ;
	//
	// メッセージループ
	//
	MSG	msg ;
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		::TranslateMessage( &msg ) ;
		::DispatchMessage( &msg ) ;
	}
	//
	// スレッド終了
	//
	m_sigExitThread.SetSignal() ;
	m_threadServ.Wait() ;
	//
	return	0 ;
}

// ウィンドウクラス登録
//////////////////////////////////////////////////////////////////////////////
const char * SGLUpdaterApplication::RegisterWindowClass( void )
{
	//
	// 登録済みクラスのテスト
	//
	WNDCLASS		wndclass ;
	HMODULE			hModule = ::GetModuleHandle( NULL ) ;
	SString			strClassName = L"EntisGLS4_PrivilegeWriteServer" ;
	const char *	pszClassName =
						strClassName.EncodeDefaultTo(m_bszWndClassName) ;
	if ( ::GetClassInfo( hModule, pszClassName, &wndclass ) )
	{
		if ( wndclass.lpfnWndProc
				== &SGLUpdaterApplication::WindowCallbackProc )
		{
			return	pszClassName ;
		}
		for ( int i = 0; i < 0x10000; i ++ )
		{
			strClassName = L"EntisGLS4_PrivilegeWriteServer" ;
			strClassName += SString(i) ;
			pszClassName = strClassName.EncodeDefaultTo(m_bszWndClassName) ;
			if ( !::GetClassInfo( hModule, pszClassName, &wndclass ) )
			{
				break ;
			}
		}
	}
	//
	// ウィンドウクラス登録
	//
	wndclass.style = 0 ;
	wndclass.lpfnWndProc = &SGLUpdaterApplication::WindowCallbackProc ;
	wndclass.cbClsExtra = 0 ;
	wndclass.cbWndExtra = sizeof(SGLGenericWindow*) ;
	wndclass.hInstance = ::GetModuleHandle( NULL ) ;
	wndclass.hIcon = NULL ;
	wndclass.hCursor = NULL ;
	wndclass.hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH ) ;
	wndclass.lpszMenuName = NULL ;
	wndclass.lpszClassName = pszClassName ;
	//
	if ( ::RegisterClass( &wndclass ) == 0 )
	{
		ESLTrace( "failed to register class for "
					"PrivilegeWriteServer of SGLUpdaterApplication\n" ) ;
	}
	return	pszClassName ;
}

// ウィンドウ・プロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT SGLUpdaterApplication::WindowProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
	case	wmShutdown:
		::DestroyWindow( hWnd ) ;
		::PostQuitMessage( 0 ) ;
		return	0 ;

	case	wmConnect:
		if ( m_hConnectProcess == NULL )
		{
			m_hConnectProcess =
				::OpenProcess( PROCESS_ALL_ACCESS, FALSE, (DWORD) lParam ) ;
			if ( m_hConnectProcess != NULL )
			{
				return	connectSuccessed ;
			}
		}
		return	connectFailed ;

	case	wmDisonnect:
		if ( m_hConnectProcess != NULL )
		{
			::CloseHandle( m_hConnectProcess ) ;
			m_hConnectProcess = NULL ;
			return	(LRESULT) errSuccess ;
		}
		return	(LRESULT) errFailed ;

	case	wmOpenFile:
		{
			OpenFileParam	ofp ;
			if ( ReadConnectedMemory
				( &ofp, (ulong_ptr_t) lParam, sizeof(OpenFileParam) ) )
			{
				SArray<wchar_t>	bufFilePath ;
				wchar_t *	pwszFilePath = bufFilePath.GetArray( ofp.nFileLen + 1 ) ;
				//
				if ( ReadConnectedMemory
					( pwszFilePath, (ulong_ptr_t) (ofp.pwszFilePath),
								(ofp.nFileLen + 1) * sizeof(wchar_t) ) )
				{
					bufFilePath.FinishArray() ;
					return	(LRESULT) m_fileServ.Open
								( pwszFilePath, (long int) ofp.nOpenFlags ) ;
				}
				bufFilePath.FinishArray() ;
			}
		}
		return	(LRESULT) errFailed ;

	case	wmCloseFile:
		m_fileServ.Close() ;
		return	0 ;

	case	wmWriteAsync:
		{
			WriteAsyncEntry *	pwae = new WriteAsyncEntry ;
			if ( ReadConnectedMemory
				( pwae, (ulong_ptr_t) lParam, sizeof(WriteAsyncEntry) ) )
			{
				m_csWriteQue.Lock() ;
				m_queWriteAsync.Add( pwae ) ;
				m_sigWriteReq.SetSignal() ;
				m_csWriteQue.Unlock() ;
				return	(LRESULT) errSuccess ;
			}
			else
			{
				delete	pwae ;
			}
		}
		return	(LRESULT) errFailed ;

	case	wmReadAsync:
		return	(LRESULT) errFailed ;

	case	wmSeek:
		{
			int64_t	posSeek = 0 ;
			if ( ReadConnectedMemory
				( &posSeek, (ulong_ptr_t) lParam, sizeof(int64_t) ) )
			{
				posSeek =
					m_fileServ.Seek
						( posSeek, (SFileInterface::SeekOrigin) wParam ) ;
				if ( WriteConnectedMemory
					( (ulong_ptr_t) lParam, &posSeek, sizeof(int64_t) ) )
				{
					return	(LRESULT) errSuccess ;
				}
			}
		}
		return	(LRESULT) errFailed ;

	case	wmGetLength:
		{
			int64_t	nLength = m_fileServ.GetLength() ;
			if ( WriteConnectedMemory
				( (ulong_ptr_t) lParam, &nLength, sizeof(int64_t) ) )
			{
				return	(LRESULT) errSuccess ;
			}
		}
		return	(LRESULT) errFailed ;

	case	wmTruncate:
		return	(LRESULT) m_fileServ.SetEndOfFile() ;

	case	wmDeleteFile:
		{
			DeleteFileParam	dfp ;
			if ( ReadConnectedMemory
				( &dfp, (ulong_ptr_t) lParam, sizeof(DeleteFileParam) ) )
			{
				SArray<wchar_t>	bufFilePath ;
				wchar_t *	pwszFilePath = bufFilePath.GetArray( dfp.nFileLen + 1 ) ;
				//
				if ( ReadConnectedMemory
					( pwszFilePath, (ulong_ptr_t) (dfp.pwszFilePath),
								(dfp.nFileLen + 1) * sizeof(wchar_t) ) )
				{
					bufFilePath.FinishArray() ;
					return	(LRESULT) SFile::RemoveFile( pwszFilePath ) ;
				}
				bufFilePath.FinishArray() ;
			}
		}
		return	(LRESULT) errFailed ;
	}
	return	::DefWindowProc( hWnd, uMsg, wParam, lParam ) ;
}

LRESULT __stdcall SGLUpdaterApplication::WindowCallbackProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SGLUpdaterApplication *	pApp = NULL ;
	if ( uMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT	pcs = (LPCREATESTRUCT) lParam ;
		pApp = (SGLUpdaterApplication*) pcs->lpCreateParams ;
		::SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR) pApp ) ;
		pApp->m_hWndServer = hWnd ;
	}
	else
	{
		pApp = (SGLUpdaterApplication*) ::GetWindowLongPtr( hWnd, GWLP_USERDATA ) ;
		if ( pApp != NULL )
		{
			if ( pApp->m_hWndServer != hWnd )
			{
				pApp = NULL ;
			}
		}
	}
	if ( pApp != NULL )
	{
		return	pApp->WindowProc( hWnd, uMsg, wParam, lParam ) ;
	}
	return	::DefWindowProc( hWnd, uMsg, wParam, lParam ) ;
}

// プロセスメモリ読み込み
//////////////////////////////////////////////////////////////////////////////
bool SGLUpdaterApplication::ReadConnectedMemory
	( void * ptrDst, ulong_ptr_t addrSrc, size_t nBytes )
{
	ESLAssert( m_hConnectProcess != NULL ) ;
	SIZE_T	nReadBytes = 0 ;
	if ( !::ReadProcessMemory
		( m_hConnectProcess, (LPVOID) addrSrc, ptrDst, nBytes, &nReadBytes ) )
	{
		ESLTrace( "failed to ReadProcessMemory.\n" ) ;
		return	false ;
	}
	return	true ;
}

// プロセスメモリ書き出し
//////////////////////////////////////////////////////////////////////////////
bool SGLUpdaterApplication::WriteConnectedMemory
	( ulong_ptr_t addrDst, const void * ptrSrc, size_t nBytes )
{
	ESLAssert( m_hConnectProcess != NULL ) ;
	SIZE_T	nWrittenBytes = 0 ;
	if ( !::WriteProcessMemory
		( m_hConnectProcess, (LPVOID) addrDst, ptrSrc, nBytes, &nWrittenBytes ) )
	{
		ESLTrace( "failed to WriteProcessMemory.\n" ) ;
		return	false ;
	}
	return	true ;
}

// 非同期読み込み用スレッド関数実行
//////////////////////////////////////////////////////////////////////////////
void SGLUpdaterApplication::ServerProc::Run( void )
{
	HANDLE	hEvents[2] ;
	hEvents[0] = m_app->m_sigWriteReq.GetHandle() ;
	hEvents[1] = m_app->m_sigExitThread.GetHandle() ;
	//
	for ( ; ; )
	{
		DWORD	dwWait =
			::WaitForMultipleObjects( 2, &hEvents[0], FALSE, 100 ) ;
		if ( dwWait == WAIT_OBJECT_0 )
		{
			WriteAsyncEntry *	pwae = NULL ;
			m_app->m_csWriteQue.Lock() ;
			pwae = m_app->m_queWriteAsync.DetachAt( 0 ) ;
			if ( m_app->m_queWriteAsync.GetLength() == 0 )
			{
				m_app->m_sigWriteReq.ResetSignal() ;
			}
			m_app->m_csWriteQue.Unlock() ;
			//
			if ( pwae != NULL )
			{
				SArray<uint8_t>	bufTemp ;
				uint8_t *	pTemp = bufTemp.GetArray( pwae->nBytes ) ;
				size_t		nReadBytes = 0 ;
				if ( m_app->ReadConnectedMemory
					( pTemp, (ulong_ptr_t) pwae->ptrData, pwae->nBytes ) )
				{
					nReadBytes =
						m_app->m_fileServ.Write( pTemp, pwae->nBytes ) ;
				}
				bufTemp.FinishArray() ;
				//
				::PostThreadMessage
					( pwae->idThread, pwae->uMsgDone,
						(WPARAM) nReadBytes, pwae->lParam ) ;
				delete	pwae ;
			}
		}
		else if ( dwWait == WAIT_OBJECT_0 + 1 )
		{
			ESLAssert( m_app->m_sigExitThread.Wait(0) == errSuccess ) ;
			break ;
		}
		if ( ::WaitForSingleObject( m_app->m_hConnectProcess, 0 ) == WAIT_OBJECT_0 )
		{
			::SendMessage( m_app->m_hWndServer, wmShutdown, 0, 0 ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// 特権ファイル書き込みクライアント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLPrivilegeWriteClient, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPrivilegeWriteClient::SGLPrivilegeWriteClient( void )
{
	m_hProcess = NULL ;
	m_hWndServ = NULL ;
	m_uMsgWritten = ::RegisterWindowMessage( "EntisGLS4_WriteAsync_Done" ) ;
	//
	#if	!defined(MSGFLT_ADD)
	enum	ChangeWindowMessageFilterFlags
	{
		MSGFLT_ADD		= 1,
		MSGFLT_REMOVE	= 2,
	} ;
	#endif
	typedef BOOL (WINAPI *API_ChangeWindowMessageFilter)( UINT message, DWORD dwFlag ) ;
	API_ChangeWindowMessageFilter	apiChangeWindowMessageFilter = NULL ;
	//
	HMODULE	hUser32 = ::GetModuleHandle( "user32.dll" ) ;
	if ( hUser32 != NULL )
	{
		apiChangeWindowMessageFilter =
			(API_ChangeWindowMessageFilter)
				::GetProcAddress( hUser32, "ChangeWindowMessageFilter" ) ;
		if ( apiChangeWindowMessageFilter != NULL )
		{
			apiChangeWindowMessageFilter( m_uMsgWritten, MSGFLT_ADD ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLPrivilegeWriteClient::~SGLPrivilegeWriteClient( void )
{
	if ( IsStandServer() )
	{
		ShutdownServer() ;
	}
}

// サーバー起動
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLPrivilegeWriteClient::StartupServer( const wchar_t * pwszServerCmdLine )
{
	if ( IsStandServer() )
	{
		return	errFailed ;
	}
	//
	// サーバープログラム起動
	//
	SStringParser	sparsCmdLine = pwszServerCmdLine ;
	SString			strExeFile ;
	SString			strParameters ;
	SArray<char>	bufExeFile ;
	SArray<char>	bufParameters ;
	//
	strExeFile = sparsCmdLine.GetStringTerm() ;
	sparsCmdLine.PassSpace() ;
	strParameters = sparsCmdLine.SubString( sparsCmdLine.GetIndex() ) ;
	//
	SHELLEXECUTEINFO	sx ;
	eslFillMemory( &sx, 0, sizeof(SHELLEXECUTEINFO) ) ;
	sx.cbSize = sizeof(SHELLEXECUTEINFO) ;
	sx.fMask = SEE_MASK_NOCLOSEPROCESS ;
	sx.lpVerb = "open" ;
	sx.lpFile = strExeFile.EncodeDefaultTo( bufExeFile ) ;
	if ( !strParameters.IsEmpty() )
	{
		sx.lpParameters = strParameters.EncodeDefaultTo( bufParameters ) ;
	}
	sx.nShow = SW_SHOWNORMAL ;
	//
	if ( !ShellExecuteEx( &sx ) )
	{
		return	errFailed ;
	}
	m_hProcess = sx.hProcess ;
	m_hWndServ = NULL ;
	//
	// ウィンドウ起動待ち
	//
	for ( ; ; )
	{
		DWORD	dwWait = ::WaitForInputIdle( m_hProcess, 10 ) ;
		if ( dwWait != WAIT_TIMEOUT )
		{
			EnumWindows
				( &SGLPrivilegeWriteClient::EnumWindowsProc, (LPARAM) this ) ;
			if ( m_hWndServ != NULL )
			{
				break ;
			}
		}
		if ( ::WaitForSingleObject( m_hProcess, 0 ) == WAIT_OBJECT_0 )
		{
			::CloseHandle( m_hProcess ) ;
			m_hProcess = NULL ;
			m_hWndServ = NULL ;
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// ウィンドウ列挙
//////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK SGLPrivilegeWriteClient::EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
	SGLPrivilegeWriteClient *	ppwc = (SGLPrivilegeWriteClient*) lParam ;
	DWORD	dwProcessId = 0 ;
	::GetWindowThreadProcessId( hwnd, &dwProcessId ) ;
	if ( ::GetProcessId( ppwc->m_hProcess ) == dwProcessId )
	{
		if ( ::SendMessage
			( hwnd, SGLUpdaterApplication::wmConnect,
					0, (LPARAM) ::GetCurrentProcessId() )
							== SGLUpdaterApplication::connectSuccessed )
		{
			ppwc->m_dwProcessID = dwProcessId ;
			ppwc->m_hWndServ = hwnd ;
			return	FALSE ;
		}
	}
	return	TRUE ;
}

// サーバー終了
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLPrivilegeWriteClient::ShutdownServer( void )
{
	if ( ::IsWindow( m_hWndServ ) )
	{
		::PostMessage
			( m_hWndServ, SGLUpdaterApplication::wmShutdown, 0, 0 ) ;
		::WaitForSingleObject( m_hProcess, 3000 ) ;
	}
	if ( m_hProcess != NULL )
	{
		::CloseHandle( m_hProcess ) ;
		m_hProcess = NULL ;
	}
	m_hWndServ = NULL ;
	return	errSuccess ;
}

// サーバー起動済み判定
//////////////////////////////////////////////////////////////////////////////
bool SGLPrivilegeWriteClient::IsStandServer( void ) const
{
	return	(m_hWndServ != NULL) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLPrivilegeWriteClient::OpenFile
	( const wchar_t * pwszFilePath, long int nFlags )
{
	if ( !IsStandServer() )
	{
		return	errFailed ;
	}
	SGLUpdaterApplication::OpenFileParam	ofp ;
	ofp.pwszFilePath = pwszFilePath ;
	ofp.nFileLen = SString::GetLength( pwszFilePath ) ;
	ofp.nOpenFlags = nFlags ;
	//
	SError	err = (SError) ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmOpenFile, 0, (LPARAM) &ofp ) ;
	//
	return	err ;
}

SGLPrivilegeWriteClient::File *
	SGLPrivilegeWriteClient::NewOpenFile
		( const wchar_t * pwszFilePath, long int nFlags )
{
	if ( OpenFile( pwszFilePath, nFlags ) )
	{
		return	NULL ;
	}
	return	new File( this ) ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLPrivilegeWriteClient::CloseFile( void )
{
	if ( IsStandServer() )
	{
		::SendMessage
			( m_hWndServ,
				SGLUpdaterApplication::wmCloseFile, 0, 0 ) ;
	}
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLPrivilegeWriteClient::Write( const void * ptrData, size_t nBytes )
{
	if ( !IsStandServer() )
	{
		return	0 ;
	}
	SGLUpdaterApplication::WriteAsyncEntry	wae ;
	wae.ptrData = (void*) ptrData ;
	wae.nBytes = (uint32_t) nBytes ;
	wae.idThread = ::GetCurrentThreadId() ;
	wae.uMsgDone = m_uMsgWritten ;
	wae.lParam = 0 ;
	//
	MSG	msg ;
	while ( ::PeekMessage
			( &msg, NULL, m_uMsgWritten, m_uMsgWritten, PM_REMOVE ) )
	{
	}
	size_t	nWrittenBytes = 0 ;
	if ( ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmWriteAsync, 0, (LPARAM) &wae ) == errSuccess )
	{
		while ( ::GetMessage( &msg, NULL, m_uMsgWritten, m_uMsgWritten ) )
		{
			if ( (msg.hwnd == NULL) && (msg.message == m_uMsgWritten) )
			{
				nWrittenBytes = msg.wParam ;
				break ;
			}
		}
	}
	return	nWrittenBytes ;
}

// ポインタシーク
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::Seek
	( int64_t pos, SFileInterface::SeekOrigin seekFrom )
{
	if ( !IsStandServer() )
	{
		return	0 ;
	}
	if ( ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmSeek,
			(WPARAM) seekFrom, (LPARAM) &pos ) == errSuccess )
	{
		return	pos ;
	}
	return	0 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::GetPosition( void ) const
{
	if ( !IsStandServer() )
	{
		return	0 ;
	}
	int64_t	pos = 0 ;
	if ( ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmSeek,
			(WPARAM) SFileInterface::FromCurrent, (LPARAM) &pos ) == errSuccess )
	{
		return	pos ;
	}
	return	0 ;
}

// ファイル長を取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::GetLength( void ) const
{
	if ( !IsStandServer() )
	{
		return	0 ;
	}
	int64_t	nLength = 0 ;
	if ( ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmGetLength,
			0, (LPARAM) &nLength ) == errSuccess )
	{
		return	nLength ;
	}
	return	0 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SGLPrivilegeWriteClient::SetEndOfFile( void )
{
	if ( !IsStandServer() )
	{
		return	errFailed ;
	}
	return	(SError) ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmTruncate, 0, 0 ) ;
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLPrivilegeWriteClient::RemoveFile( const wchar_t * pwszFilePath )
{
	if ( !IsStandServer() )
	{
		return	errFailed ;
	}
	SGLUpdaterApplication::DeleteFileParam	dfp ;
	dfp.pwszFilePath = pwszFilePath ;
	dfp.nFileLen = SString::GetLength( pwszFilePath ) ;
	//
	SError	err = (SError) ::SendMessage
		( m_hWndServ,
			SGLUpdaterApplication::wmDeleteFile, 0, (LPARAM) &dfp ) ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// ファイル書き出しインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLPrivilegeWriteClient::File, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPrivilegeWriteClient::File::File( SGLPrivilegeWriteClient * ppwc )
	: m_ppwc( ppwc )
{
	ESLAssert( ppwc != NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLPrivilegeWriteClient::File::~File( void )
{
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLPrivilegeWriteClient::File::Duplicate( void ) const
{
	return	new File( m_ppwc ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLPrivilegeWriteClient::File::Read( void * ptrBuf, size_t nBytes )
{
	return	0 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLPrivilegeWriteClient::File::Write( const void * ptrBuf, size_t nBytes )
{
	return	m_ppwc->Write( ptrBuf, nBytes ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SGLPrivilegeWriteClient::File::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::File::GetLength( void ) const
{
	return	m_ppwc->GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::File::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	m_ppwc->Seek( posFile, seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLPrivilegeWriteClient::File::GetPosition( void ) const
{
	return	m_ppwc->GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLPrivilegeWriteClient::File::SetEndOfFile( void )
{
	return	m_ppwc->SetEndOfFile() ;
}




//////////////////////////////////////////////////////////////////////////////
// 再起動移動ファイルマネージャー
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLInstallerApplication::RebootMoveFile::RebootMoveFile( void )
{
	m_flagLoaded = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLInstallerApplication::RebootMoveFile::~RebootMoveFile( void )
{
}

// リストをレジストリから取得
//////////////////////////////////////////////////////////////////////////////
SError SGLInstallerApplication::RebootMoveFile::LoadList( void )
{
	SRegistryKey	key ;
	if ( key.OpenKey
		( HKEY_LOCAL_MACHINE,
			L"SYSTEM\\CurrentControlSet\\Control\\Session Manager" ) )
	{
		return	errFailed ;
	}
	if ( key.GetMultiStrings( m_lstFileSet, L"PendingFileRenameOperations" ) )
	{
		return	errFailed ;
	}
	m_flagLoaded = true ;
	key.CloseKey() ;
	return	errSuccess ;
}

// リストをレジストリへ保存
//////////////////////////////////////////////////////////////////////////////
SError SGLInstallerApplication::RebootMoveFile::SaveList( void )
{
	if ( !m_flagLoaded )
	{
		return	errFailed ;
	}
	SRegistryKey	key ;
	if ( key.OpenKey
		( HKEY_LOCAL_MACHINE,
			L"SYSTEM\\CurrentControlSet\\Control\\Session Manager" ) )
	{
		return	errFailed ;
	}
	if ( key.SetMultiStrings( L"PendingFileRenameOperations", m_lstFileSet ) )
	{
		return	errFailed ;
	}
	key.CloseKey() ;
	return	errSuccess ;
}

// 指定移動元ファイルをリストから除外
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::RebootMoveFile::RemoveMoveSource( const wchar_t * pwszFilePath )
{
	for ( size_t i = 0; i < m_lstFileSet.GetLength(); i ++ )
	{
		SRegistryKey::MultiString *	pms = m_lstFileSet.GetAt( i ) ;
		if ( (pms != NULL) && (pms->GetAt(0) != NULL) )
		{
			SString *	pstr = pms->GetAt(0) ;
			ESLAssert( pstr != NULL ) ;
			if ( pstr->CompareNoCase( pwszFilePath ) == 0 )
			{
				m_lstFileSet.RemoveAt( i ) ;
				return	true ;
			}
			else if ( pstr->CompareLeft( L"\\??\\" ) == 0 )
			{
				if ( SString::CompareNoCase
					( ((const wchar_t*) *pstr) + 4, pwszFilePath ) == 0 )
				{
					m_lstFileSet.RemoveAt( i ) ;
					return	true ;
				}
			}
		}
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// インストーラー・アプリケーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLInstallerApplication, SGLStdApplication, Listener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLInstallerApplication::SGLInstallerApplication( void )
: m_strUUIDBaseDir( SGLStdApplication::GetUUIDStorageBaseDirectory() ),
	m_strUUIDFileName( SGLStdApplication::GetUUIDStorageFileName() ),
	m_strUUIDPassword( SGLStdApplication::GetUUIDCryptyPassword() )
{
	m_mode = modeInstall ;
	m_nTotalFileBytes = 0 ;
	m_pDlg = NULL ;
	m_flagInstalling = false ;
	m_flagCancelDlg = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLInstallerApplication::~SGLInstallerApplication( void )
{
}

// 引数解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLInstallerApplication::ParseCmdLine( const wchar_t * pwszArg )
{
	return	sglErrSuccess ;
}

// 環境設定ファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLInstallerApplication::GetEnvironmentFilePath( SSystem::SString& strFilePath ) const
{
#if	defined(__PLATFORM_ANDROID__)
	strFilePath = L"assets://setup.xml" ;
#else
	strFilePath = L"setup.xml" ;
#endif
	return	strFilePath ;
}

// プロファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLInstallerApplication::GetProfileFilePath( SSystem::SString& strFilePath ) const
{
	strFilePath.FreeArray() ;
	return	NULL ;
}

// アプリケーション準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLInstallerApplication::StartUpApp( void )
{
	if ( !m_env.GetEnvironmentString
		( m_strAppName, L"cotopha\\install\\app_name" ) )
	{
		m_env.GetApplicationName( m_strAppName ) ;
	}
	m_env.GetEnvironmentString
		( m_strBlandName, L"cotopha\\install\\bland_name" ) ;
	if ( !m_env.GetEnvironmentString
		( m_strRegName, L"cotopha\\install\\reg_name" ) )
	{
		m_strRegName = m_strAppName ;
	}
	m_env.GetEnvironmentString
		( m_strInstLogFile, L"cotopha\\install\\log_file" ) ;
	m_env.GetEnvironmentString
		( m_strUninst, L"cotopha\\install\\uninst" ) ;
	//
	m_env.GetEnvironmentString
		( m_strOperation, L"cotopha\\install\\operation" ) ;
	m_env.GetEnvironmentString
		( m_strInlineRSSource, L"cotopha\\install\\script" ) ;
	//
	if ( m_strOperation == L"update" )
	{
		m_mode = modeUpdate ;
	}
	//
	SXMLDocument *	pxmlFileSets =
		m_env.GetXMLDocumnet().GetContentsElement
						( L"cotopha\\install\\file_sets" ) ;
	if ( pxmlFileSets != NULL )
	{
		for ( size_t i = 0; i < pxmlFileSets->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlFileSet = pxmlFileSets->GetElementAt( i ) ;
			if ( (pxmlFileSet == NULL)
				|| (pxmlFileSet->GetTag() != L"file_set") )
			{
				continue ;
			}
			AddInstallFiles
				( pxmlFileSet->GetAttrStringAs( L"dst" ),
					pxmlFileSet->GetAttrStringAs( L"src" ),
					(pxmlFileSet->GetAttrStringAs( L"sub_dir" ) == L"true"),
					(pxmlFileSet->GetAttrStringAs( L"encrypt" ) == L"true"),
					pxmlFileSet->GetAttrStringAs( L"src_pass" ),
					pxmlFileSet->GetAttrStringAs( L"src_arc" ) ) ;
		}
	}
	//
	return	sglErrSuccess ;
}

// 実行
//////////////////////////////////////////////////////////////////////////////
int SGLInstallerApplication::Run( void )
{
	if ( !m_strInlineRSSource.IsEmpty() )
	{
		SParserErrorLogger	perrLog ;
		m_vmRosetta.Initialize() ;
		perrLog.EnableDebugTrace( true ) ;
		if ( (m_vmRosetta.AddScriptSource
				( m_strInlineRSSource, L"<inline>", perrLog ) != NULL)
			&& (perrLog.GetErrorCount() == 0) )
		{
			m_vmRosetta.RegisterNewClass
				( new RSInstallerClass( this, m_vmRosetta.GetClassClass() ) ) ;
			m_vmRosetta.ImplementNewClasses() ;
			//
			RSContext	context( &m_vmRosetta ) ;
			context.ReleaseObjectRef
				( context.PerformExpression( L"main()", NULL, &perrLog ) ) ;
		}
		if ( perrLog.GetErrorCount() > 0 )
		{
			SParserErrorLogger::ErrorLog *
						pLog = perrLog.GetErrorLogAt( 0 ) ;
			if ( pLog != NULL )
			{
				SString	strErrMsg = L"以下のエラーが発生しました：\n" ;
				SString	strLine = pLog->m_strLine ;
				strLine.TrimLeft() ;
				strLine.TrimRight() ;
				strLine.Replace( L'\t', L' ' ) ;
				//
				strErrMsg += pLog->m_strError ;
				strErrMsg += L"\n\n" ;
				strErrMsg += SString( pLog->m_nLineNum ) ;
				strErrMsg += L"行\n" ;
				strErrMsg += strLine ;
				//
				SString	strAppName ;
				m_env.GetApplicationName( strAppName ) ;
				//
				SSystem::MessageBox( strErrMsg, strAppName, msgboxStyleOk ) ;
			}
		}
		m_vmRosetta.Release() ;
	}
	else if ( m_strOperation == L"uninstall" )
	{
		DoUninstall() ;
	}
	else
	{
		DoModal() ;
	}
	return	0 ;
}

// UUID ストレージ保存用ベースディレクトリ（オフセットパス）
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLInstallerApplication::GetUUIDStorageBaseDirectory( void )
{
	return	m_strUUIDBaseDir ;
}

// UUID ストレージ保存用ファイル名
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLInstallerApplication::GetUUIDStorageFileName( void )
{
	return	m_strUUIDFileName ;
}

// UUID ストレージ保存用パスワード
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLInstallerApplication::GetUUIDCryptyPassword( void ) const
{
	return	m_strUUIDPassword ;
}

// UUID をストレージに保存するか？（false では読み込みのみ）
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::IsSaveUUIDintoStorage( void ) const
{
	return	false ;
}

// UUID ストレージ保存用パス設定
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::SetUUIDStoragePath
	( const wchar_t * pwszBaseDir,
		const wchar_t * pwszFileName, const wchar_t * pwszPassword )
{
	m_strUUIDBaseDir = pwszBaseDir ;
	m_strUUIDFileName = pwszFileName ;
	m_strUUIDPassword = pwszPassword ;
}

// インストールファイル追加
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::AddInstallFiles
	( const wchar_t * pwszDstDir,
		const wchar_t * pwszSrcFiles,
		bool flagSubDirectory, bool flagEncrypt,
		const wchar_t * pwszSrcCryptPass,
		const wchar_t * pwszSrcArchive )
{
	SString	strSrcFiles = pwszSrcFiles ;
	SString	strSrcFileNames = strSrcFiles.GetFileNamePart() ;
	//
	InstallFileInfo *	pifi = new InstallFileInfo ;
	pifi->m_flagOnline = false ;
	pifi->m_strDstDir = pwszDstDir ;
	pifi->m_strSrcDir = strSrcFiles.GetFileDirectoryPart() ;
	if ( (pifi->m_strSrcDir.GetLastAt(0) == L'\\')
		|| (pifi->m_strSrcDir.GetLastAt(0) == L'/') )
	{
		pifi->m_strSrcDir.ChopRight( 1 ) ;
	}
	pifi->m_flagEncrypt = flagEncrypt ;
	if ( flagEncrypt && pwszSrcCryptPass && pwszSrcCryptPass[0] )
	{
		pifi->m_flagSrcCrypt = true ;
		pifi->m_strSrcPassword = pwszSrcCryptPass ;
	}
	if ( (pwszSrcArchive != NULL)
		&& (pwszSrcArchive[0] != 0) )
	{
		SFileInterface *	pFile =
			SFileOpener::DefaultNewOpenFile
				( pwszSrcArchive, SFileOpener::shareRead ) ;
		if ( pFile != NULL )
		{
			ERISA::SGLArchiveFile *	pArcFile = new ERISA::SGLArchiveFile ;
			if ( !pArcFile->OpenArchive( pFile, true, SFile::modeRead ) )
			{
				pifi->m_pOpener = pArcFile ;
			}
			else
			{
				ESLTrace( "failed to open archive \'%s\'\n",
						SString(pwszSrcArchive).ToCharArray().GetConstArray() ) ;
				delete	pArcFile ;
			}
		}
		else
		{
			ESLTrace( "failed to open \'%s\'\n",
					SString(pwszSrcArchive).ToCharArray().GetConstArray() ) ;
		}
	}
	if ( (strSrcFiles.CompareLeftNoCase( L"http://" ) == 0)
		|| (strSrcFiles.CompareLeftNoCase( L"https://" ) == 0) )
	{
		pifi->m_flagOnline = true ;
		//
		FileInfo *	pfi = new FileInfo ;
		pfi->m_strFilePath = strSrcFileNames ;
		pfi->m_nFileLength = 0 ;
		pifi->m_aSrcFiles.Add( pfi ) ;
	}
	else
	{
		AddInstallSubFiles( *pifi, strSrcFileNames, flagSubDirectory ) ;
	}
	m_lstInstFiles.Add( pifi ) ;
	m_nTotalFileBytes += pifi->m_nTotalBytes ;
}

void SGLInstallerApplication::AddInstallSubFiles
	( SGLInstallerApplication::InstallFileInfo& ifi,
		const wchar_t * pwszSrcFiles, bool flagSubDirectory )
{
	SString	strSrcFiles = pwszSrcFiles ;
	SString	strSrcDir = strSrcFiles.GetFileDirectoryPart() ;
	if ( (strSrcDir.GetLastAt(0) == L'\\')
		|| (strSrcDir.GetLastAt(0) == L'/') )
	{
		strSrcDir.ChopRight( 1 ) ;
	}
	SString	strSrcFileNames = strSrcFiles.GetFileNamePart() ;
	//
	SFileOpener *	pOpener = ifi.m_pOpener ;
	if ( flagSubDirectory )
	{
		SObjectArray<SString>	lstDirs ;
		if ( pOpener != NULL )
		{
			pOpener->ListSubDirectories
				( lstDirs, ifi.m_strSrcDir.OffsetFilePath( strSrcDir ) ) ;
		}
		else
		{
			SFile::ListDirectories
				( lstDirs, ifi.m_strSrcDir.OffsetFilePath( strSrcDir ) ) ;
		}
		//
		for ( size_t i = 0; i < lstDirs.GetLength(); i ++ )
		{
			SString *	pstrDirName = lstDirs.GetAt( i ) ;
			ESLAssert( pstrDirName != NULL ) ;
			if ( pstrDirName == NULL )
			{
				continue ;
			}
			if ( (*pstrDirName == L".")
				|| (*pstrDirName == L"..") )
			{
				continue ;
			}
			if ( !SFileOpener::IsMatchWildCardTo
						( strSrcFileNames, *pstrDirName ) )
			{
				continue ;
			}
			SString	strSubDirPath =
				strSrcDir.OffsetFilePath( *pstrDirName ).
							OffsetFilePath( L"*.*" ) ;
			AddInstallSubFiles( ifi, strSubDirPath, flagSubDirectory ) ;
		}
	}
	//
	SObjectArray<SString>	lstFiles ;
	if ( pOpener != NULL )
	{
		pOpener->ListSubFiles
			( lstFiles, ifi.m_strSrcDir.OffsetFilePath( strSrcDir ) ) ;
	}
	else
	{
		SFile::ListFiles
			( lstFiles, ifi.m_strSrcDir.OffsetFilePath( strSrcDir ) ) ;
	}
	//
	for ( size_t i = 0; i < lstFiles.GetLength(); i ++ )
	{
		SString *	pstrFileName = lstFiles.GetAt( i ) ;
		ESLAssert( pstrFileName != NULL ) ;
		if ( pstrFileName == NULL )
		{
			continue ;
		}
		if ( !SFileOpener::IsMatchWildCardTo
					( strSrcFileNames, *pstrFileName ) )
		{
			continue ;
		}
		SString	strSrcFilePath =
			ifi.m_strSrcDir.OffsetFilePath( strSrcDir ).
							OffsetFilePath( *pstrFileName ) ;
		SFileOpener::State	state ;
		if ( pOpener != NULL )
		{
			if ( pOpener->QueryState( strSrcFilePath, state ) )
			{
				continue ;
			}
		}
		else
		{
			if ( SFile::QueryFileState( strSrcFilePath, state ) )
			{
				continue ;
			}
		}
		FileInfo *	pfi = new FileInfo ;
		pfi->m_strFilePath = strSrcDir.OffsetFilePath( *pstrFileName ) ;
		pfi->m_nFileLength = state.nFileSize ;
		ifi.m_aSrcFiles.Add( pfi ) ;
		ifi.m_nTotalBytes += state.nFileSize ;
	}
}

// 定義されたショートカット作成
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::InstallShortcutFiles
	( const wchar_t * pwszDstDir, const wchar_t * pwszEnvPath )
{
	SXMLDocument *	pxmlList =
		m_env.GetXMLDocumnet().GetContentsElement( pwszEnvPath ) ;
	if ( pxmlList == NULL )
	{
		return ;
	}
	SString	strDstDir = pwszDstDir ;
	for ( size_t i = 0; i < pxmlList->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlShortcut = pxmlList->GetElementAt( i ) ;
		if ( (pxmlShortcut == NULL)
			|| (pxmlShortcut->GetTag() != L"shortcut") )
		{
			continue ;
		}
		//
		SString	strName = pxmlShortcut->GetAttrStringAs( L"name" ) ;
		SString	strFilePath =
			strDstDir.OffsetFilePath
				( pxmlShortcut->GetAttrStringAs( L"file", strName + L".lnk" ) ) ;
		SString	strLinkPath =
			m_strInstDir.OffsetFilePath
				( pxmlShortcut->GetAttrStringAs( L"link" ) ) ;
		SString	strParams = pxmlShortcut->GetAttrStringAs( L"arg" ) ;
		//
		if ( CreateShortcutFile
			( strFilePath, strName, strLinkPath, strParams ) )
		{
			m_fmLog.GetFileAs( strFilePath, true ) ;
		}
	}
}

// ショートカット作成
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::CreateShortcutFile
	( const wchar_t * pwszFilePath,
		const wchar_t * pwszName, const wchar_t * pwszLinkPath,
		const wchar_t * pwszParameters, const wchar_t * pwszWorkDir )
{
	bool	fSuccessed = false ;
	//
	// ディレクトリ作成
	//
	SString	strDirPath = SString(pwszFilePath).GetFileDirectoryPart() ;
	if ( (strDirPath.GetLastAt(0) == L'\\')
		|| (strDirPath.GetLastAt(0) == L'/') )
	{
		strDirPath.ChopRight( 1 ) ;
	}
	SFile::CreateFullDirectory( strDirPath ) ;
	//
	// ショートカットファイルオブジェクト作成
	//
	IShellLink *	psl ;
	if( SUCCEEDED( ::CoCreateInstance
		( CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl ) ) )
	{
		//
		// ショートカット設定
		//
		SArray<char>	bufPath ;
		SArray<char>	bufName ;
		SString			strWorkDir ;
		SArray<char>	bufWorkDir ;
		SArray<char>	bufParams ;
		if ( pwszWorkDir == NULL )
		{
			strWorkDir = SString(pwszLinkPath).GetFileDirectoryPart() ;
			if ( (strWorkDir.GetLastAt(0) == L'\\')
				|| (strWorkDir.GetLastAt(0) == L'/') )
			{
				strWorkDir.ChopRight( 1 ) ;
			}
			pwszWorkDir = strWorkDir ;
		}
		psl->SetPath( SString(pwszLinkPath).EncodeDefaultTo(bufPath) ) ;
		psl->SetDescription( SString(pwszName).EncodeDefaultTo(bufName) ) ;
		psl->SetWorkingDirectory
				( SString(pwszWorkDir).EncodeDefaultTo(bufWorkDir) ) ;
		if ( (pwszParameters != NULL) && (pwszParameters[0] != 0) )
		{
			psl->SetArguments
				( SString(pwszParameters).EncodeDefaultTo(bufParams) ) ;
		}
		//
		// 書き出しのためのインターフェース取得
		//
		IPersistFile *	ppf ;
		if( SUCCEEDED( psl->QueryInterface
			( IID_IPersistFile, (void**)&ppf ) ) )
		{
			if ( ppf->Save( pwszFilePath, TRUE ) == S_OK )
			{
				fSuccessed = true ;
			}
			ppf->Release( ) ;
		}
		psl->Release( ) ;
	}
	return	fSuccessed ;
}

// インストール情報をレジストリに登録
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::RegisterUninstall
	( const wchar_t * pwszRegName,
		const wchar_t * pwszDispName,
		const wchar_t * pwszCmdLine,
		const wchar_t * pwszInstDir,
		const wchar_t * pwszPublisher )
{
	SString	strRegPath =
		L"SOFTWARE\\Microsoft\\Windows\\"
		L"CurrentVersion\\Uninstall\\" ;
	strRegPath += pwszRegName ;
	//
	SRegistryKey	key ;
	if ( !key.CreateKey( HKEY_LOCAL_MACHINE, strRegPath ) )
	{
		const wchar_t *	pwszValues[] =
		{
			pwszDispName,
			pwszCmdLine,
			pwszInstDir,
			pwszPublisher,
		} ;
		const wchar_t *	pwszNames[] =
		{
			L"DisplayName",
			L"UninstallString",
			L"InstallLocation",
			L"Publisher",
			NULL
		} ;
		for ( size_t i = 0; pwszNames[i]; i ++ )
		{
			if ( pwszValues[i] == NULL )
			{
				continue ;
			}
			key.SetString( pwszNames[i], pwszValues[i] ) ;
		}
	}
}

// インストール先ディレクトリ取得
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::GetInstallLocation
	( SSystem::SString& strInstDir ) const
{
	if ( SFile::GetDefaultDirectory
		( strInstDir,
			SFile::DefaultDirectory::ApplicationInstalled, m_strRegName ) )
	{
		return	false ;
	}
	return	true ;
}

bool SGLInstallerApplication::GetInstallLocation
	( const wchar_t * pwszRegName, SSystem::SString& strInstDir )
{
	if ( SFile::GetDefaultDirectory
		( strInstDir,
			SFile::DefaultDirectory::ApplicationInstalled, pwszRegName ) )
	{
		return	false ;
	}
	return	true ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::ShellOpenFiles( const wchar_t * pwszEnvPath )
{
	SXMLDocument *	pxmlList =
		m_env.GetXMLDocumnet().GetContentsElement( pwszEnvPath ) ;
	if ( pxmlList == NULL )
	{
		return ;
	}
	for ( size_t i = 0; i < pxmlList->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlShortcut = pxmlList->GetElementAt( i ) ;
		if ( (pxmlShortcut == NULL)
			|| (pxmlShortcut->GetTag() != L"open") )
		{
			continue ;
		}
		SString	strFilePath =
			m_strInstDir.OffsetFilePath
				( pxmlShortcut->GetAttrStringAs( L"file" ) ) ;
		OpenShellFile( strFilePath ) ;
	}
}

// アンインストール実行
//////////////////////////////////////////////////////////////////////////////
void SGLInstallerApplication::DoUninstall( Window * pParentWnd )
{
	//
	// 確認メッセージ
	//
	SString	strAppName ;
	m_env.GetApplicationName( strAppName ) ;
	//
	SString	strMsg ;
	if ( GetInstallLocation( m_strRegName, m_strInstDir ) )
	{
		strMsg = L"「" ;
		strMsg += m_strAppName ;
		strMsg += L"」をアンインストールします" ;
		//
		if ( SSystem::MessageBox
			( strMsg, strAppName,
				msgboxStyleOkCancel, pParentWnd ) != msgboxResultOk )
		{
			return ;
		}
	}
	else
	{
		strMsg = L"「" ;
		strMsg += m_strAppName ;
		strMsg += L"」はインストールされていません" ;
		//
		SSystem::MessageBox
			( strMsg, strAppName, msgboxStyleOk, pParentWnd ) ;
		//
		return ;
	}
	//
	// インストールログファイルを読み込む
	//
	SString	strLogFile = m_strInstDir.OffsetFilePath( m_strInstLogFile ) ;
	if ( m_fmLog.LoadInstallLog( strLogFile ) )
	{
		SSystem::MessageBox
			( L"ログファイルの読み込みに失敗しました",
						strAppName, msgboxStyleOk, pParentWnd ) ;
		return ;
	}
	bool	fReqReboot = m_fmLog.UninstallFiles() ;
	//
	SFile::RemoveFile( strLogFile ) ;
	//
	// レジストリを削除する
	//
	SString	strRegPath =
		L"SOFTWARE\\Microsoft\\Windows\\"
		L"CurrentVersion\\Uninstall\\" ;
	strRegPath += m_strRegName ;
	//
	SArray<char>	bufRegPath ;
	::RegDeleteKey
		( HKEY_LOCAL_MACHINE, strRegPath.EncodeDefaultTo(bufRegPath) ) ;
	//
	// 再起動確認
	//
	if ( fReqReboot )
	{
		if ( SSystem::MessageBox
			( L"アンインストールを完了するには再起動が必要です。\n"
				L"今すぐ再起動しますか？",
				strAppName, msgboxStyleYesNo, pParentWnd ) == msgboxResultYes )
		{
			if ( g_infoPlatform.runtimeOS != platformOS_Windows )
			{
				HANDLE hToken ;
				TOKEN_PRIVILEGES tkp ;
				//
				// Get a token for this process. 
				//
 				if ( !OpenProcessToken
					( GetCurrentProcess(), 
						TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
				{
					ESLTrace( "Failed to OpenProcessToken\n" ) ;
					return ;
				}
				//
				// Get the LUID for the shutdown privilege. 
				//
				LookupPrivilegeValue
					( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid ) ;
				//
				tkp.PrivilegeCount = 1;  // one privilege to set
				tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
				//
				// Get the shutdown privilege for this process. 
				//
				AdjustTokenPrivileges
					( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 ) ;
				//
				// Cannot test the return value of AdjustTokenPrivileges. 
				//
				if ( GetLastError() != ERROR_SUCCESS )
				{
					ESLTrace( "Failed to AdjustTokenPrivileges\n" ) ;
					return ;
				}
			}
			if ( !ExitWindowsEx( EWX_REBOOT | EWX_FORCE, 0 ) ) 
			{
				ESLTrace( "Failed to ExitWindowsEx\n" ) ;
			}
		}
	}
}

// インストーラー・ダイアログ入力
//////////////////////////////////////////////////////////////////////////////
int SGLInstallerApplication::DoModal( Window * pParentWnd )
{
	SString	strInstDir ;
	if ( (m_mode == modeUpdate)
		&& !GetInstallLocation( strInstDir ) )
	{
		SString	strAppName ;
		m_env.GetApplicationName( strAppName ) ;
		MessageBox
			( strAppName + L"のインストール情報が見つかりません。",
				L"エラー", msgboxStyleOk, pParentWnd ) ;
		return	0 ;
	}
	SCustomDialog::ElementInfo	elinf[] =
	{
		{ NULL, SCustomDialog::itemGroupBox,
			SCustomDialog::flagEndOfLine
				| SCustomDialog::flagFullWidth, 0, 0,
			_TX(L"\x1b[en]Install destination\0インストール先\0") },
		{ L"ID_EDIT_INST_DIR", SCustomDialog::itemEdit,
			SCustomDialog::flagMinWidth, 0, 320, L"" },
		{ L"ID_BUTTON_BROWSE", SCustomDialog::itemButton,
			SCustomDialog::flagEndOfLine, 0, 0,
			_TX(L"\x1b[en]Browse\0参照\0"), 0, 0, 0,
			&SGLInstallerApplication::BrowseButtonCallback, this },
		{ L"ID_CHECK_SHORTCUT_DESKTOP",
			SCustomDialog::itemCheck,
			SCustomDialog::flagEndOfLine, 0, 0,
			_TX(L"\x1b[en]Create shortcut on the Desktop\0"
					L"デスクトップにショートカットを作成する\0"), 1 },
		{ L"ID_CHECK_START_MENU",
			SCustomDialog::itemCheck,
			SCustomDialog::flagEndOfGroupBox, 0, 0,
			_TX(L"\x1b[en]Create shortcut into Start Menu\0"
					L"スタートメニューに登録する\0"), 1 },
		{ NULL, SCustomDialog::itemNull,
			SCustomDialog::flagEndOfLine, 0, 0, L"" },
		{ NULL, SCustomDialog::itemGroupBox,
			SCustomDialog::flagEndOfLine
				| SCustomDialog::flagFullWidth, 0, 0,
			_TX(L"\x1b[en]Progress\0進行状況\0") },
		{ NULL, SCustomDialog::itemText,
			SCustomDialog::flagEndOfLine, 0, 0,
			_TX(L"\x1b[en]Total progress\0全体の進行状況\0") },
		{ L"ID_PROGRESS_TOTAL", SCustomDialog::itemProgress,
			SCustomDialog::flagEndOfLine
				| SCustomDialog::flagMinWidth, 0, 380, L"", 0, 0, 0x1000 },
		{ L"ID_TEXT_FILE", SCustomDialog::itemText,
			SCustomDialog::flagEndOfLine
				| SCustomDialog::flagMinWidth, 0, 380,
			_TX(L"\x1b[en]Current file\0現在のファイル\0") },
		{ L"ID_PROGRESS_FILE", SCustomDialog::itemProgress,
			SCustomDialog::flagEndOfGroupBox
				| SCustomDialog::flagMinWidth, 0, 380, L"", 0, 0, 0x1000 },
		{ NULL, SCustomDialog::itemNull,
			SCustomDialog::flagEndOfLine, 0, 0, L"" },
		{ L"ID_INSTALL", SCustomDialog::itemButton,
			SCustomDialog::flagLineRight, 0, 0,
			_TX(L"\x1b[en]Install\0インストール\0"), 0, 0, 0,
			&SGLInstallerApplication::InstallButtonCallback, this },
		{ L"ID_CANCEL", SCustomDialog::itemButton,
			SCustomDialog::flagLineRight
				| SCustomDialog::flagNegativeButton
				| SCustomDialog::flagEndOfLine, 0, 0,
			_TX(L"\x1b[en]Cancel\0キャンセル\0"), 0, 0, 0,
			&SGLInstallerApplication::CancelButtonCallback, this },
	} ;
	//
	SString	strAppName ;
	m_env.GetApplicationName( strAppName ) ;
	//
	SString	strProgramFilesDir ;
	SFile::GetDefaultDirectory
		( strProgramFilesDir, SFile::DefaultDirectory::WindowsProgramFiles ) ;
	//
	SCustomDialog	dlg ;
	dlg.SetCaption( strAppName ) ;
	dlg.SetCustomItems( elinf, sizeof(elinf) / sizeof(elinf[0]) ) ;
	if ( m_mode == modeUpdate )
	{
		dlg.SetItemStringAs( L"ID_EDIT_INST_DIR", strInstDir ) ;
		dlg.EnableItemAs( L"ID_EDIT_INST_DIR", false ) ;
		dlg.EnableItemAs( L"ID_BUTTON_BROWSE", false ) ;
		dlg.SetItemIntegerAs( L"ID_CHECK_SHORTCUT_DESKTOP", 0 ) ;
		dlg.SetItemIntegerAs( L"ID_CHECK_START_MENU", 0 ) ;
		dlg.EnableItemAs( L"ID_CHECK_SHORTCUT_DESKTOP", false ) ;
		dlg.EnableItemAs( L"ID_CHECK_START_MENU", false ) ;
	}
	else
	{
		dlg.SetItemStringAs
			( L"ID_EDIT_INST_DIR",
				strProgramFilesDir.
					OffsetFilePath(m_strBlandName).
					OffsetFilePath(m_strAppName) ) ;
	}
	dlg.AttachListener( this ) ;
	dlg.DoModal( 0, pParentWnd ) ;
	//
	return	0 ;
}

// 参照ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::BrowseButtonCallback
	( SCustomDialog& dlg,
		const SCustomDialog::ElementInfo& item, int code, void * instance )
{
	SString	strInstDir = dlg.GetInputStringAs( L"ID_EDIT_INST_DIR" ) ;
	if ( DoBrowseDirectoryDialog
		( strInstDir,
			_TX(L"\x1b[en]Install destination\0インストール先\0"),
			strInstDir, 0, dlg.GetWindowHandle() ) == msgboxResultOk )
	{
		SGLInstallerApplication *	app = (SGLInstallerApplication*) instance ;
		if ( app->m_strBlandName.CompareNoCase
			( strInstDir.Right( app->m_strBlandName.GetLength() ) ) == 0 )
		{
			strInstDir = strInstDir.OffsetFilePath( app->m_strAppName ) ;
		}
		else if ( app->m_strAppName.CompareNoCase
			( strInstDir.Right( app->m_strAppName.GetLength() ) ) != 0 )
		{
			strInstDir =
				strInstDir.OffsetFilePath( app->m_strBlandName ).
							OffsetFilePath( app->m_strAppName ) ;
		}
		dlg.SetItemStringAs( L"ID_EDIT_INST_DIR", strInstDir ) ;
	}
	return	true ;
}

// インストールボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::InstallButtonCallback
	( SCustomDialog& dlg,
		 const SCustomDialog::ElementInfo& item, int code, void * instance )
{
	dlg.EnableItemAs( L"ID_EDIT_INST_DIR", false ) ;
	dlg.EnableItemAs( L"ID_BUTTON_BROWSE", false ) ;
	dlg.EnableItemAs( L"ID_CHECK_SHORTCUT_DESKTOP", false ) ;
	dlg.EnableItemAs( L"ID_CHECK_START_MENU", false ) ;
	dlg.EnableItemAs( L"ID_INSTALL", false ) ;
	//
	SGLInstallerApplication *	app = (SGLInstallerApplication*) instance ;
	app->m_strInstDir = dlg.GetInputStringAs( L"ID_EDIT_INST_DIR" ) ;
	app->m_flagShortcutDesktop =
		(dlg.GetInputIntegerAs( L"ID_CHECK_SHORTCUT_DESKTOP" ) != 0) ;
	app->m_flagShortcutStartMenu =
		(dlg.GetInputIntegerAs( L"ID_CHECK_START_MENU" ) != 0) ;
	app->m_pDlg = &dlg ;
	//
	SThread::BeginStockThread
		( &SGLInstallerApplication::InstallButtonThreadProc, app ) ;
	return	true ;
}

void SGLInstallerApplication::InstallButtonThreadProc( void * pInstance )
{
	((SGLInstallerApplication*)pInstance)->InstallButtonProc() ;
}

void SGLInstallerApplication::InstallButtonProc( void )
{
	RebootMoveFile	rmf ;
	bool			fCancelDelayMoveFile = false ;
	rmf.LoadList() ;
	//
	m_flagInstalling = true ;
	m_flagCancelDlg = false ;
	//
	// インストールログファイル初期化
	//
	m_fmLog.ClearAll() ;
	if ( !m_strInstLogFile.IsEmpty() )
	{
		m_fmLog.LoadInstallLog
			( m_strInstDir.OffsetFilePath( m_strInstLogFile ) ) ;
	}
	m_fmLog.GetDirectoryAs( m_strInstDir, true ) ;
	//
	SString	strMachineID = GetMachineUniqueId() ;
	//
	// ファイルをコピー
	//
	int64_t	nTotalInstalled = 0 ;
	bool	flagCancel = false ;
	bool	flagError = false ;
	for ( size_t i = 0;
			!flagCancel && (i < m_lstInstFiles.GetLength()); i ++ )
	{
		InstallFileInfo *	pifi = m_lstInstFiles.GetAt( i ) ;
		ESLAssert( pifi != NULL ) ;
		if ( pifi == NULL )
		{
			continue ;
		}
		m_fmLog.GetDirectoryAs
			( m_strInstDir.OffsetFilePath(pifi->m_strDstDir), true ) ;
		//
		for ( size_t j = 0;
				!flagCancel && (j < pifi->m_aSrcFiles.GetLength()); j ++ )
		{
			FileInfo *	pfi = pifi->m_aSrcFiles.GetAt( j ) ;
			if ( pfi == NULL )
			{
				continue ;
			}
			//
			// ファイル名表示
			//
			SString	strSrcPath =
				pifi->m_strSrcDir.OffsetFilePath( pfi->m_strFilePath ) ;
			SString	strDstPath =
				pifi->m_strDstDir.OffsetFilePath( pfi->m_strFilePath ) ;
			//
			m_pDlg->SetItemStringAs( L"ID_TEXT_FILE", pfi->m_strFilePath ) ;
			m_pDlg->SetItemIntegerAs( L"ID_PROGRESS_FILE", 0 ) ;
			//
			// 入力ファイルを開く
			//
			SSmartPointer<SFileInterface>	pSrcFile ;
			for ( ; ; )
			{
				if ( pifi->m_pOpener != NULL )
				{
					ERISA::SGLArchiveFile *	pArcFile =
							ESLTypeCast<ERISA::SGLArchiveFile>( pifi->m_pOpener.Ptr() ) ;
					if ( pArcFile != NULL )
					{
						pSrcFile = pArcFile->NewOpenFile
									( strSrcPath, SFileOpener::shareRead
													| SFileOpener::modeStreaming ) ;
					}
					else
					{
						pSrcFile = pifi->m_pOpener->NewOpenFile
										( strSrcPath, SFileOpener::shareRead ) ;
					}
				}
				else
				{
					pSrcFile = SFileOpener::DefaultNewOpenFile
									( strSrcPath, SFileOpener::shareRead ) ;
				}
				if ( pSrcFile != NULL )
				{
					break ;
				}
				SString	strErrMsg = strSrcPath + L" を開けませんでした" ;
				if ( pifi->m_flagOnline )
				{
					strErrMsg = L"オンラインファイルを開けませんでした。" ;
				}
				int	nResult = m_pDlg->DoMessageBox
					( strErrMsg, L"エラー", msgboxStyleRetryCancel ) ;
				if ( nResult != msgboxResultRetry )
				{
					flagCancel = true ;
					flagError = true ;
					break ;
				}
			}
			if ( flagCancel )
			{
				break ;
			}
			SHttpFileInterface *	pHttp =
				ESLTypeCast<SHttpFileInterface>( pSrcFile.Ptr() ) ;
			if ( pHttp != NULL )
			{
				uint32_t	codeStatus ;
				if ( !pHttp->QueryStatusCode( codeStatus )
					&& (codeStatus != 200) )
				{
					m_pDlg->DoMessageBox
						( L"オンラインファイルへ接続できませんでした。",
								L"エラー", msgboxStyleOk ) ;
					flagCancel = true ;
					flagError = true ;
					break ;
				}
			}
			if ( pifi->m_flagSrcCrypt )
			{
				if ( pifi->m_flagOnline || !pSrcFile->IsSeekable() )
				{
					//
					// ダウンロード
					//
					SString	strFileMsg = L"ダウンロードしています…" ;
					strFileMsg += pfi->m_strFilePath ;
					m_pDlg->SetItemStringAs( L"ID_TEXT_FILE", strFileMsg ) ;
					//
					SSmartBuffer *	psbufTemp = new SSmartBuffer ;
					SArray<uint8_t>	bufTemp ;
					size_t			nBufBytes = 0x10000 ;
					uint8_t *		pTempBuf = bufTemp.GetArray( nBufBytes ) ;
					int64_t			nDownloadBytes = 0 ;
					int64_t			nFileLength = pSrcFile->GetLength() ;
					//
					for ( ; ; )
					{
						size_t	nReadBytes = pSrcFile->Read( pTempBuf, nBufBytes ) ;
						if ( nReadBytes == 0 )
						{
							break ;
						}
						psbufTemp->Write( pTempBuf, nReadBytes ) ;
						nDownloadBytes += nReadBytes ;
						//
						if ( nFileLength > 0 )
						{
							m_pDlg->SetItemIntegerAs
								( L"ID_PROGRESS_FILE",
									(int) (nDownloadBytes * 0x1000 / nFileLength) ) ;
						}
						if ( m_flagCancelDlg )
						{
							flagCancel = true ;
							break ;
						}
					}
					bufTemp.FinishArray() ;
					//
					if ( flagCancel )
					{
						delete	psbufTemp ;
						break ;
					}
					if ( (nFileLength > 0) && (nFileLength != nDownloadBytes) )
					{
						m_pDlg->DoMessageBox
							( L"ダウンロードが中断しました",
										L"エラー", msgboxStyleOk ) ;
						flagCancel = true ;
						flagError = true ;
						delete	psbufTemp ;
						break ;
					}
					//
					m_pDlg->SetItemStringAs( L"ID_TEXT_FILE", pfi->m_strFilePath ) ;
					m_pDlg->SetItemIntegerAs( L"ID_PROGRESS_FILE", 0 ) ;
					//
					psbufTemp->Seek( 0 ) ;
					pSrcFile = psbufTemp ;
				}
				ERISA::SGLDecrypt32File *
						pDecrypt = new ERISA::SGLDecrypt32File ;
				if ( pDecrypt->Open
					( pSrcFile.Detach(), true, pifi->m_strSrcPassword ) )
				{
					delete	pDecrypt;
					//
					m_pDlg->DoMessageBox
						( strSrcPath + L" を開けませんでした",
										L"エラー", msgboxStyleOk ) ;
					flagCancel = true ;
					flagError = true ;
					break ;
				}
				pSrcFile = pDecrypt ;
			}
			//
			// 出力ファイルを開く
			//
			SSmartPointer<SFileInterface>	pDstFile ;
			SFile *	pDstTempFile = new SFile ;
			if ( pDstTempFile->Open
				( m_strInstDir.OffsetFilePath( strDstPath ),
										SFileOpener::modeCreate ) )
			{
				delete	pDstTempFile ;
			}
			else
			{
				pDstFile = pDstTempFile ;
			}
			while ( pDstFile == NULL )
			{
				int	nResult = m_pDlg->DoMessageBox
					( strSrcPath + L" を開けませんでした",
						L"エラー", msgboxStyleRetryCancel ) ;
				if ( nResult != msgboxResultRetry )
				{
					flagCancel = true ;
					flagError = true ;
					break ;
				}
				pDstTempFile = new SFile ;
				if ( pDstTempFile->Open
					( m_strInstDir.OffsetFilePath( strDstPath ),
											SFileOpener::modeCreate ) )
				{
					delete	pDstTempFile ;
				}
				else
				{
					pDstFile = pDstTempFile ;
				}
			}
			if ( flagCancel )
			{
				break ;
			}
			if ( pifi->m_flagEncrypt )
			{
				ERISA::SGLEncrypt32FileWriter *
					pEncrypt = new ERISA::SGLEncrypt32FileWriter ;
				pEncrypt->Open( pDstFile.Detach(), true, strMachineID ) ;
				pDstFile = pEncrypt ;
			}
			//
			// ログに追加
			//
			m_fmLog.GetFileAs
				( m_strInstDir.OffsetFilePath( strDstPath ), true ) ;
			//
			// ファイルをコピー
			//
			SArray<uint8_t>	bufTemp ;
			size_t			nBufBytes = 0x10000 ;
			uint8_t *		pTempBuf = bufTemp.GetArray( nBufBytes ) ;
			int64_t			nCopiedBytes = 0 ;
			int64_t			nReadTotalBytes = 0 ;
			int64_t			nFileLength = pSrcFile->GetLength() ;
			while ( nCopiedBytes < nFileLength )
			{
				size_t	nReadBytes = nBufBytes ;
				if ( (int64_t) (nReadTotalBytes + nReadBytes) > nFileLength )
				{
					nReadBytes = (size_t) (nFileLength - nReadTotalBytes) ;
				}
				nReadBytes = pSrcFile->Read( pTempBuf, nReadBytes ) ;
				nReadTotalBytes += nReadBytes ;
				if ( nReadBytes == 0 )
				{
					if ( nCopiedBytes != nFileLength )
					{
						m_pDlg->DoMessageBox
							( L"ファイルの読み込みに失敗しました",
											L"エラー", msgboxStyleOk ) ;
						flagCancel = true ;
						flagError = true ;
					}
					break ;
				}
				size_t	nWrittenBytes =
							pDstFile->Write( pTempBuf, nReadBytes ) ;
				if ( nWrittenBytes < nReadBytes )
				{
					m_pDlg->DoMessageBox
						( L"ファイルの書き出しに失敗しました",
										L"エラー", msgboxStyleOk ) ;
					flagCancel = true ;
					flagError = true ;
					break ;
				}
				nCopiedBytes += nWrittenBytes ;
				nTotalInstalled += nWrittenBytes ;
				//
				if ( nFileLength != 0 )
				{
					m_pDlg->SetItemIntegerAs
						( L"ID_PROGRESS_FILE",
							(int) (nCopiedBytes * 0x1000 / nFileLength) ) ;
				}
				if ( m_nTotalFileBytes != 0 )
				{
					m_pDlg->SetItemIntegerAs
						( L"ID_PROGRESS_TOTAL",
							(int) (nTotalInstalled
									* 0x1000 / m_nTotalFileBytes) ) ;
				}
				if ( m_flagCancelDlg )
				{
					flagCancel = true ;
					break ;
				}
			}
			bufTemp.FinishArray() ;
			//
			// 書き出しファイルを再起動遅延移動から除外
			//
			if ( rmf.RemoveMoveSource
				( m_strInstDir.OffsetFilePath( strDstPath ) ) )
			{
				fCancelDelayMoveFile = true ;
			}
		}
	}
	if ( fCancelDelayMoveFile )
	{
		rmf.SaveList() ;
	}
	m_pDlg->SetItemStringAs( L"ID_TEXT_FILE", L"" ) ;
	m_pDlg->SetItemIntegerAs( L"ID_PROGRESS_FILE", 0 ) ;
	m_pDlg->SetItemIntegerAs( L"ID_PROGRESS_TOTAL", 0 ) ;
	//
	if ( !m_strInstLogFile.IsEmpty() )
	{
		m_fmLog.GetFileAs
			( m_strInstDir.OffsetFilePath( m_strInstLogFile ), true ) ;
		m_fmLog.SaveInstallLog
			( m_strInstDir.OffsetFilePath( m_strInstLogFile ) ) ;
	}
	if ( flagCancel )
	{
		if ( !flagError )
		{
			SString	strAppName ;
			m_env.GetApplicationName( strAppName ) ;
			//
			m_pDlg->DoMessageBox
				( L"インストールはキャンセルされました",
								strAppName, msgboxStyleOk ) ;
		}
		m_pDlg->EndDialog( msgboxResultCancel ) ;
		m_flagInstalling = false ;
		return ;
	}
	//
	// ショートカット作成
	//
	m_pDlg->SetItemStringAs
		( L"ID_TEXT_FILE", L"ショートカットを作成しています…" ) ;
	//
	if ( m_flagShortcutDesktop )
	{
		SString	strDesktopDir ;
		if ( !SFile::GetDefaultDirectory
			( strDesktopDir, SFile::DefaultDirectory::WindowsDesktop ) )
		{
			InstallShortcutFiles
				( strDesktopDir, L"cotopha\\install\\desktop" ) ;
		}
	}
	if ( m_flagShortcutStartMenu )
	{
		SString	strStartMenuDir ;
		if ( !SFile::GetDefaultDirectory
			( strStartMenuDir, SFile::DefaultDirectory::WindowsStartMenu ) )
		{
			SString	strMenuBlandDir =
						strStartMenuDir.OffsetFilePath( m_strBlandName ) ;
			m_fmLog.GetDirectoryAs( strMenuBlandDir, true ) ;
			//
			SString	strMenuAppDir =
						strMenuBlandDir.OffsetFilePath( m_strAppName ) ;
			m_fmLog.GetDirectoryAs( strMenuAppDir, true ) ;
			//
			InstallShortcutFiles
				( strMenuAppDir, L"cotopha\\install\\start_menu" ) ;
		}
	}
	//
	// ログファイル書き出し
	//
	if ( !m_strInstLogFile.IsEmpty() )
	{
		m_fmLog.SaveInstallLog
			( m_strInstDir.OffsetFilePath( m_strInstLogFile ) ) ;
	}
	//
	// レジストリ登録
	//
	if ( m_mode == modeInstall )
	{
		SString	strUninstCmdLine = L"\"" ;
		strUninstCmdLine += m_strInstDir.OffsetFilePath( m_strUninst ) ;
		strUninstCmdLine += L"\"" ;
		//
		RegisterUninstall
			( m_strRegName, m_strAppName,
				strUninstCmdLine, m_strInstDir, m_strBlandName ) ;
	}
	//
	// ファイルを開く
	//
	ShellOpenFiles( L"cotopha\\install\\shell" ) ;
	//
	// 完了
	//
	SString	strAppName ;
	m_env.GetApplicationName( strAppName ) ;
	//
	m_pDlg->DoMessageBox
		( L"インストールが完了しました", strAppName, msgboxStyleOk ) ;
	m_pDlg->EndDialog( msgboxResultOk ) ;
	m_flagInstalling = false ;
}

// キャンセルボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::CancelButtonCallback
	( SSystem::SCustomDialog& dlg,
		const SSystem::SCustomDialog::ElementInfo& item,
		int code, void * instance )
{
	SGLInstallerApplication *	app = (SGLInstallerApplication*) instance ;
	if ( app->m_flagInstalling )
	{
		app->m_flagCancelDlg = true ;
		return	true ;
	}
	return	false ;
}

// ダイアログキャンセル処理
//////////////////////////////////////////////////////////////////////////////
bool SGLInstallerApplication::OnCancel( SCustomDialog& dlg )
{
	if ( m_flagInstalling )
	{
		m_flagCancelDlg = true ;
		return	true ;
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// Installer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::RSInstallerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInstallerClass::RSInstallerClass
		( SGLInstallerApplication * app,
			RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName ), m_app( app )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSInstallerClass::OverrideVirtuals( Rosetta::RSContext& context )
{
	SParserErrorTracer	perr ;
	AddFunctionDescriptiveAs
		( context, perr, L"doInstall", NULL, L"WindowSprite window",
			NULL, &RSInstallerClass::method_doInstall, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"doUninstall", NULL, L"WindowSprite window",
			NULL, &RSInstallerClass::method_doUninstall, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"getInstalledPath", L"String", L"",
			NULL, &RSInstallerClass::method_getInstalledPath, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"prepareUUID",
			NULL, L"String strBaseDir, "
				L"String strFileName, String strPassword = null",
			NULL, &RSInstallerClass::method_prepareUUID, NULL ) ;
}

// void doInstall( WindowSprite window )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInstallerClass::method_doInstall
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLWindowSprite *	pWindow =
			RSWindowSpriteClass::GetThisWindowSprite
							( context, arg.ObjectAt( 0 ) ) ;
	//
	RSInstallerClass *
		pClass = ESLTypeCast<RSInstallerClass>( pThis ) ;
	if ( pClass && pClass->m_app )
	{
		pClass->m_app->DoModal( pWindow ) ;
	}
	return	NULL ;
}

// void doUninstall( WindowSprite window )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInstallerClass::method_doUninstall
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLWindowSprite *	pWindow =
			RSWindowSpriteClass::GetThisWindowSprite
							( context, arg.ObjectAt( 0 ) ) ;
	//
	RSInstallerClass *
		pClass = ESLTypeCast<RSInstallerClass>( pThis ) ;
	if ( pClass && pClass->m_app )
	{
		pClass->m_app->DoUninstall( pWindow ) ;
	}
	return	NULL ;
}

// String getInstalledPath()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInstallerClass::method_getInstalledPath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSInstallerClass *
		pClass = ESLTypeCast<RSInstallerClass>( pThis ) ;
	if ( pClass && pClass->m_app )
	{
		SString	strInstDir ;
		if ( pClass->m_app->GetInstallLocation( strInstDir ) )
		{
			return	context.new_String( strInstDir ) ;
		}
	}
	return	NULL ;
}

// void prepareUUID
//	( String strBaseDir, String strFileName, String strPassword = null ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInstallerClass::method_prepareUUID
	( Rosetta::RSContext& context, void * pInstace,
		Rosetta::RSObject* pThis,
		Rosetta::RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSInstallerClass *
		pClass = ESLTypeCast<RSInstallerClass>( pThis ) ;
	if ( pClass && pClass->m_app )
	{
		SGLInstallerApplication *	app = pClass->m_app ;
		SString	strBaseDir = arg.StringAt
			( 0, app->SGLStdApplication::GetUUIDStorageBaseDirectory() ) ;
		SString	strFileName = arg.StringAt
			( 1, app->SGLStdApplication::GetUUIDStorageFileName() ) ;
		SString	strPassword = arg.StringAt
			( 2, app->SGLStdApplication::GetUUIDCryptyPassword() ) ;
		//
		app->SetUUIDStoragePath( strBaseDir, strFileName, strPassword ) ;
		//
		SString	strUUID ;
		if ( app->LoadUserUniqueId( strUUID, strBaseDir ) )
		{
			strUUID = app->GetUserUniqueId() ;
			app->SaveUserUniqueId( strUUID, strBaseDir ) ;
		}
		SGLStdApplication::PrepareUserUniqueId( strUUID ) ;
	}
	return	NULL ;
}


#endif
#endif

