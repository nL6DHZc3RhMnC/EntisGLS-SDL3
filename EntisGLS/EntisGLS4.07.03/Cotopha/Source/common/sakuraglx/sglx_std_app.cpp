
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sglx_std_app.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuracl/erisa/sgl_erisa_md5_context.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakura/ssys_win_registry.h>
#include <Lmcons.h>
#include <iphlpapi.h>
#endif

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <stdlib.h>
#include <unistd.h>
#endif

#if	defined(__PLATFORM_ANDROID__)
#include <sakura/ssys_android_file.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// プロファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAppProfile, SXMLDocument )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAppProfile::SGLAppProfile( void )
{
	m_pxmlProfile = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAppProfile::~SGLAppProfile( void )
{
}

// 圧縮・暗号化されたプロファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAppProfile::LoadProfile
	( const wchar_t * pwszFilePath, const wchar_t * pwszPassword )
{
	//
	// ファイルを開く
	//
	SSmartPointer<SFileInterface>
		pfile = SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pfile == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ヘッダ読み込み
	//
	uint32_t	nOrgLength = 0 ;
	uint32_t	nDecrypeKey ;
	if ( pfile->Read
		( &nOrgLength, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	pfile->Read( &nDecrypeKey, sizeof(uint32_t) ) ;
	if ( nOrgLength > 0x10000000 /*256MB*/ )
	{
		return	sglErrFailed ;
	}
	//
	// デコード
	//
	ERISA::SGLDecrypt32InputStream	decrypt( pfile.Ptr() ) ;
	decrypt.Initialize( pwszPassword ) ;
	decrypt.SetDecryptKey( nDecrypeKey ) ;
	//
	ERISA::SGLDecodeBitStream	bstream( 0x1000 ) ;
	bstream.AttachInputStream( &decrypt ) ;
	//
	ERISA::SGLERISANDecodeContext	decoder( &bstream ) ;
	decoder.PrepareToDecodeERISANCode() ;
	//
	SSmartBuffer	sbufFile ;
	if ( sbufFile.ReadFromStream
		( decoder, (ssize_t) nOrgLength ) < nOrgLength )
	{
		return	sglErrFailed ;
	}
	//
	// XML 解釈
	//
	SParserErrorTracer	perrTracer ;
	m_pxmlProfile = NULL ;
	if ( SXMLDocument::ReadDocument( sbufFile, perrTracer ) )
	{
		return	sglErrFailed ;
	}
	m_pxmlProfile = CreateElementTagAs( L"profile" ) ;
	m_strFilePath = pwszFilePath ;
	return	sglErrSuccess ;
}

// プロファイルを保存する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAppProfile::SaveProfile( const wchar_t * pwszPassword )
{
	if ( m_strFilePath.IsEmpty() )
	{
		return	sglErrFailed ;
	}
	return	SaveProfileAs( m_strFilePath, pwszPassword ) ;
}

SGLError SGLAppProfile::SaveProfileAs
	( const wchar_t * pwszFilePath, const wchar_t * pwszPassword )
{
	//
	// XML 文書化
	//
	SSmartBuffer	sbufTemp ;
	SXMLDocument::WriteDocument( sbufTemp ) ;
	//
	// ファイルを開く
	//
	SSmartPointer<SFileInterface>
		pfile = SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pfile == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// エンコード
	//
	ERISA::SGLEncrypt32OutputStream	encrypt( pfile.Ptr() ) ;
	uint32_t	nDecrypeKey ;
	encrypt.Initialize( pwszPassword ) ;
	nDecrypeKey = encrypt.GenerateKey() ;
	//
	ERISA::SGLEncodeBitStream	bstream( 0x1000 ) ;
	bstream.AttachOutputStream( &encrypt ) ;
	//
	ERISA::SGLERISANEncodeContext	encoder( &bstream ) ;
	encoder.PrepareToEncodeERISANCode() ;
	//
	uint32_t	nOrgLength = (uint32_t) sbufTemp.GetLength() ;
	pfile->Write( &nOrgLength, sizeof(uint32_t) ) ;
	pfile->Write( &nDecrypeKey, sizeof(uint32_t) ) ;
	//
	sbufTemp.WriteToStream( encoder, (ssize_t) nOrgLength ) ;
	//
	encoder.EncodeERISANCodeEOF() ;
	encoder.FinishERISACode() ;
	bstream.Flushout() ;
	encrypt.FlushData() ;
	//
	return	sglErrSuccess ;
}

// 新規作成
//////////////////////////////////////////////////////////////////////////////
void SGLAppProfile::CreateProfile( const wchar_t * pwszFilePath )
{
	RemoveAllContents() ;
	m_pxmlProfile = CreateElementTagAs( L"profile" ) ;
	m_strFilePath = pwszFilePath ;
}

// ファイルパス取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& SGLAppProfile::GetFilePath( void ) const
{
	return	m_strFilePath ;
}

// タグ取得／生成 (<profile> 以下のパス)
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SGLAppProfile::GetProfileOf( const wchar_t * pwszPath )
{
	ESLAssert( m_pxmlProfile != NULL ) ;
	if ( pwszPath == NULL )
	{
		return	m_pxmlProfile ;
	}
	SStringParser	sparsPath = pwszPath ;
	SXMLDocument *	pxmlTag = m_pxmlProfile ;
	SString			strTagName ;
	while ( !sparsPath.IsIndexOverflow() )
	{
		wchar_t	wch = sparsPath.NextEnclosedString( strTagName, L'\\' ) ;
		pxmlTag = pxmlTag->CreateElementTagAs( strTagName ) ;
		if ( wch != L'\\' )
		{
			break ;
		}
	}
	return	pxmlTag ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準的なアプリケーション・テンプレート
//////////////////////////////////////////////////////////////////////////////

SGLStdApplication *	SGLStdApplication::m_pApp = NULL ;
wchar_t	SGLStdApplication::m_wszUserUniqueId[257] = { 0 } ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLStdApplication, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLStdApplication::SGLStdApplication( void )
{
	#if	!defined(__COTOPHA__) && defined(__PLATFORM_WINDOWS__)
	m_hMutex = NULL ;
	#elif	defined(__PLATFORM_ANDROID__)
	m_pProgDialog = NULL ;
	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLStdApplication::~SGLStdApplication( void )
{
	#if	!defined(__COTOPHA__) && defined(__PLATFORM_WINDOWS__)
	ReleaseExclusiveBoot() ;
	#endif
}

// 引数解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLStdApplication::ParseCmdLine( const wchar_t * pwszArg )
{
	return	sglErrSuccess ;
}

// 初期化処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLStdApplication::Initialize( void )
{
	PrepareUserUniqueId() ;
	m_pApp = this ;
	//
	// 初期準備
	//
	SGLError	err ;
	err = PrepareStart() ;
	if ( err )
	{
		return	err ;
	}
	//
	// 起動開始
	//
	SString	strAppName ;
	Environment::GetApplicationName( strAppName ) ;
	//
	#if	defined(__PLATFORM_ANDROID__)
	SSmartPointer<SProgressiveUserInterface>
				pDlg = NewProgressiveUserInterface() ;
	if ( pDlg == NULL )
	{
		SProgressiveDialog *	dlg = new SProgressiveDialog ;
		dlg->SetCreationParam( SProgressiveDialog::flagStyleSpinner ) ;
		pDlg = dlg ;
	}
	pDlg->CreateProgressiveDialog() ;
	pDlg->SetProgressiveCaption( strAppName ) ;
	pDlg->SetProgressiveMessage( L"起動しています…" ) ;
	m_pProgDialog = pDlg;
	#endif
	//
	// アプリケーション準備
	//
	err = StartUpApp() ;
	//
	// 起動完了
	//
	#if	defined(__PLATFORM_ANDROID__)
	if ( m_pProgDialog != NULL )
	{
		m_pProgDialog = NULL ;
		pDlg->CloseProgressiveDialog() ;
	}
	#endif

	return	err ;
}

// 初期準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLStdApplication::PrepareStart( void )
{
	//
	// 環境設定
	//
	#if	!defined(__COTOPHA__)
	SSmartPointer<SFileInterface>	pfileEnv ;
	#if	defined(__PLATFORM_WINDOWS__)
	pfileEnv = SFileOpener::DefaultNewOpenFile
			( L"assets://IDR_COTOMI", SFileOpener::shareRead ) ;
	if ( pfileEnv != NULL )
	{
		// Win32 PE に結合済みリソースをデコード
		ERISA::SGLDecodeBitStream	bstream( 0x1000 ) ;
		bstream.AttachInputStream( pfileEnv.Ptr() ) ;
		//
		ERISA::SGLERISANDecodeContext	decoder( &bstream ) ;
		decoder.PrepareToDecodeERISANCode() ;
		//
		SSmartBuffer *	psbufEnv = new SSmartBuffer ;
		psbufEnv->ReadFromStream( decoder ) ;
		//
		pfileEnv = psbufEnv ;
		//
		// リソースから読み込む場合はカレントディレクトリを
		// exe ファイルと同じディレクトリに移動
		#if	!defined(__DEBUG__)
		wchar_t	bufDir[MAX_PATH + 1] ;
		::GetModuleFileNameW( ::GetModuleHandle(NULL), bufDir, MAX_PATH ) ;
		SString	strModulePath = bufDir ;
		SString	strModuleDir = strModulePath.GetFileDirectoryPart() ;
		if ( (strModuleDir.GetLastAt(0) == L'\\')
			|| (strModuleDir.GetLastAt(0) == L'/') )
		{
			strModuleDir.ChopRight( 1 ) ;
		}
		::SetCurrentDirectoryW( strModuleDir ) ;
		#endif
	}
	else
	{
		SString	strEnvFile ;
		pfileEnv = SFileOpener::DefaultNewOpenFile
				( GetEnvironmentFilePath(strEnvFile), SFileOpener::shareRead ) ;
	}
	#else
	SString	strEnvFile ;
	pfileEnv = SFileOpener::DefaultNewOpenFile
			( GetEnvironmentFilePath(strEnvFile), SFileOpener::shareRead ) ;
	#endif
	if ( pfileEnv == NULL )
	{
		MessageBox( L"環境ファイルを開けませんでした", L"エラー" ) ;
		return	sglErrFailed ;
	}
	Trace( "Loading environment...\n" ) ;
	SSmartPointer<SProgressiveUserInterface>
				pDlg = NewProgressiveUserInterface() ;
	if ( m_env.LoadEnvironment( *pfileEnv, pDlg ) )
	{
		if ( !m_env.GetErrorMessage().IsEmpty() )
		{
			MessageBox( m_env.GetErrorMessage(), L"エラー" ) ;
		}
		return	sglErrFailed ;
	}
	pfileEnv = NULL ;
	pDlg = NULL ;
	#endif
	//
	// 二重起動チェック
	//
	#if	!defined(__COTOPHA__) && defined(__PLATFORM_WINDOWS__)
	SString	strMultiBoot ;
	bool	flagMultiBoot = false ;
	if ( m_env.GetEnvironmentString
			( strMultiBoot, L"script\\vm\\multi_boot" ) )
	{
		flagMultiBoot = (strMultiBoot == L"true") ;
	}
	else if ( m_env.GetEnvironmentString
			( strMultiBoot, L"cotopha\\vm\\multi_boot" ) )
	{
		flagMultiBoot = (strMultiBoot == L"true") ;
	}
	if ( !flagMultiBoot )
	{
		if( CheckMultiBoot() )
		{
			return	sglErrAbort ;
		}
	}
	#endif
	//
	// プロファイル読み込み
	//
	bool	fLoadedProfile = false ;
	SString	strProfFile ;
	GetProfileFilePath( strProfFile ) ;
	if ( !strProfFile.IsEmpty() )
	{
		SString	strPassword ;
		GetProfilePassword( strPassword ) ;
		//
		Trace( "Loading profile...\n" ) ;
		if ( !m_profile.LoadProfile( strProfFile, strPassword ) )
		{
			fLoadedProfile = true ;
		}
		else
		{
			SFileOpener *	pOpener =
				SFileOpener::DefaultGetExisting( strProfFile, true ) ;
			if ( pOpener != NULL )
			{
				// ファイルが読み込めない（破損している等）
				// ミラーファイルの読み込みを試行
				if ( !m_profile.LoadProfile
						( strProfFile + L".mir", strPassword ) )
				{
					fLoadedProfile = true ;
				}
				else
				{
					// 破損ファイルをバックアップする
					if ( pOpener->IsExisting( strProfFile + L".bak" ) )
					{
						if ( !pOpener->IsExisting( strProfFile + L".bk2" ) )
						{
							pOpener->RenameSubFile
								( strProfFile + L".bak", strProfFile + L".bk2" ) ;
						}
						else
						{
							pOpener->RemoveSubFile( strProfFile + L".bak" ) ;
						}
					}
					pOpener->RenameSubFile
						( strProfFile, strProfFile + L".bak" ) ;
				}
			}
		}
		if ( !fLoadedProfile )
		{
			Trace( "New profile.\n" ) ;
			m_profile.CreateProfile( strProfFile ) ;
		}
	}
	//
	return	sglErrSuccess ;
}

// 環境設定ファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLStdApplication::GetEnvironmentFilePath( SString& strFilePath ) const
{
#if	defined(__PLATFORM_ANDROID__)
	strFilePath = L"assets://environment.xml" ;
#else
	strFilePath = L"environment.xml" ;
#endif
	return	strFilePath ;
}

// プロファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLStdApplication::GetProfileFilePath( SString& strFilePath ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	HMODULE			hModule = ::GetModuleHandle( NULL ) ;
	SArray<char>	bufModule ;
	bufModule.SetLength( MAX_PATH + 1 ) ;
	::GetModuleFileName( hModule, bufModule.GetArray(), MAX_PATH ) ;
	bufModule.FinishArray() ;
	//
	SString	strModuleFile = bufModule.GetConstArray() ;
	strFilePath = strModuleFile.GetFileTitlePart() + L".profile" ;
#else
	strFilePath = L"gls4app.profile" ;
#endif
	return	strFilePath ;
}

// プロファイルパスワードを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLStdApplication::GetProfilePassword( SSystem::SString& strPassword ) const
{
	strPassword = GetMachineUniqueId() ;
	return	strPassword ;
}

// アプリケーション準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLStdApplication::StartUpApp( void )
{
	return	sglErrSuccess ;
}

// 終了処理
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::Release( int nExitCode )
{
	m_pApp = NULL ;

	if ( nExitCode >= 0 )
	{
		SaveProfile() ;
	}

	#if	!defined(__COTOPHA__)
	if ( nExitCode < 0 )
	{
		m_env.ReleaseVM() ;
	}
	else
	{
		m_env.UnloadPrimaryModule() ;
	}
	SEnvironmentInterface::AttachInstance( NULL ) ;
	#endif
}

// 実行
//////////////////////////////////////////////////////////////////////////////
int SGLStdApplication::Run( void )
{
	return	0 ;
}

#if	!defined(__COTOPHA__)
// ECSSakura2::EnvironmentVM 取得
//////////////////////////////////////////////////////////////////////////////
ECSSakura2::EnvironmentVM & SGLStdApplication::GetEnvironmentVM( void )
{
	return	m_env ;
}
#endif

#if	defined(__PLATFORM_WINDOWS__)
// 二重起動チェック
//////////////////////////////////////////////////////////////////////////////
bool SGLStdApplication::CheckMultiBoot( void )
{
	SString	strAppName ;
	SString	strMutexName = L"EntisGLS4_App_" ;
	if ( !m_env.GetEnvironmentString
			( strAppName, L"cotopha\\display\\boot_name" ) )
	{
		if ( !m_env.GetEnvironmentString
				( strAppName, L"script\\display\\boot_name" ) )
		{
			m_env.GetApplicationName( strAppName ) ;
		}
		strMutexName = L"COTOMI_" ;
	}
	strMutexName += strAppName ;
	//
	if ( m_hMutex != NULL )
	{
		::CloseHandle( m_hMutex ) ;
		m_hMutex = NULL ;
	}
	SArray<char>	bufMutexName ;
	m_hMutex = ::CreateMutex
		( NULL, TRUE, strMutexName.EncodeDefaultTo(bufMutexName) ) ;
	if ( (m_hMutex == NULL)
		|| (::GetLastError() == ERROR_ALREADY_EXISTS) )
	{
		return	true ;
	}
	return	false ;
}

// 二重起動排他オブジェクト解放
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::ReleaseExclusiveBoot( void )
{
	if ( m_hMutex != NULL )
	{
		::CloseHandle( m_hMutex ) ;
		m_hMutex = NULL ;
	}
}
#endif

// 固有IDの取得（準備）
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::PrepareUserUniqueId( void )
{
	const size_t	limUUID = sizeof(m_wszUserUniqueId) / sizeof(m_wszUserUniqueId[0]) ;
	SString			strUUID = GetUserUniqueId() ;
	const wchar_t *	pwszUUID = strUUID ;
	size_t	nLength = strUUID.GetLength() ;
	if ( nLength > limUUID - 1 )
	{
		nLength = limUUID - 1 ;
	}
	for ( size_t i = 0; i <= nLength; i ++ )
	{
		m_wszUserUniqueId[i] = pwszUUID[i] ;
	}
}

void SGLStdApplication::PrepareUserUniqueId( const wchar_t * pwszUUID )
{
	const size_t	limUUID = sizeof(m_wszUserUniqueId) / sizeof(m_wszUserUniqueId[0]) ;
	size_t	i = 0 ;
	while ( pwszUUID[i] && (i <= limUUID - 1) )
	{
		m_wszUserUniqueId[i] = pwszUUID[i] ;
		i ++ ;
	}
	ESLAssert( i < limUUID ) ;
	m_wszUserUniqueId[i] = 0 ;
}

SString SGLStdApplication::GetUserUniqueId( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	SString	strBaseDir = GetUUIDStorageBaseDirectory() ;
	SString	strUUID ;
	if ( !LoadUserUniqueId( strUUID, strBaseDir ) )
	{
		return	strUUID ;
	}
	strUUID = GetBasicUserUniqueId() ;
	if ( !strUUID.IsEmpty() )
	{
		if ( IsSaveUUIDintoStorage() )
		{
			SaveUserUniqueId( strUUID, strBaseDir ) ;
		}
	}
	else
	{
		strUUID = GetUserUniqueId( strBaseDir ) ;
	}
	return	strUUID ;

#elif	defined(__PLATFORM_ANDROID__)
	return	GetUserUniqueId( GetUUIDStorageBaseDirectory() ) ;

#else
	#error	no implement SGLStdApplication::GetUserUniqueId
#endif
}

SSystem::SString SGLStdApplication::GetBasicUserUniqueId( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	SString	strUniqueId ;
	//
	// システムドライブのシリアルを取得する
	//
	SString	strWindowsDir ;
	if ( !SFile::GetDefaultDirectory
		( strWindowsDir, SFile::DefaultDirectory::WindowsDirectory ) )
	{
		SString	strDrv = strWindowsDir.GetFileDrivePart() ;
		strDrv.Replace( L'/', L'\\' ) ;
		if ( strDrv.GetLastAt(0) != L'\\' )
		{
			strDrv += L'\\' ;
		}
		UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
		SArray<char>	bufDrv ;
		DWORD			dwSerialNum ;
		if ( GetVolumeInformation
			( strDrv.EncodeDefaultTo(bufDrv),
				NULL, 0, &dwSerialNum, NULL, NULL, NULL, 0 ) )
		{
			SString	strHex( dwSerialNum, 8, 16 ) ;
			if ( !strUniqueId.IsEmpty() )
			{
				strUniqueId += L"@" ;
			}
			strUniqueId += strHex ;
		}
		::SetErrorMode( nErrorMode ) ;
	}
	//
	// ネットアークアダプタ MAC アドレスを取得する
	//
	IP_ADAPTER_INFO *
			pAdpInf = (IP_ADAPTER_INFO*)
						esl_malloc( sizeof(IP_ADAPTER_INFO) ) ;
	ULONG	ulOutBufLen = sizeof(IP_ADAPTER_INFO) ;
	if ( ::GetAdaptersInfo
		( pAdpInf, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
	{
		pAdpInf = (IP_ADAPTER_INFO*)
					esl_realloc( pAdpInf, (size_t) ulOutBufLen ) ;
	}
	if ( ::GetAdaptersInfo ( pAdpInf, &ulOutBufLen ) == NO_ERROR )
	{
		if ( !strUniqueId.IsEmpty() )
		{
			strUniqueId += L"@" ;
		}
		for ( size_t i = 0; i < pAdpInf->AddressLength; i ++ )
		{
			SString	strHex( pAdpInf->Address[i], 2, 16 ) ;
			strUniqueId += strHex ;
		}
	}
	esl_free( pAdpInf ) ;
	//
	return	strUniqueId ;

#else
	return	SString() ;

#endif
}

#if	defined(__PLATFORM_WINDOWS__)
// UUID のファイルからの読み込み／新規生成
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUserUniqueId( const wchar_t * pwszBaseDirName )
{
	//
	// コンピューター名
	//
	SString	strComputerName = GetComputerName() ;
	//
	// ユーザー名
	//
	SString	strUserName = GetUserName() ;
	strUserName += L"@" ;
	strUserName += strComputerName ;
	//
	// 保存パス
	//
	SString	strStorageUIDPath = GetUUIDStoragePath( pwszBaseDirName ) ;
	if ( strStorageUIDPath.IsEmpty() )
	{
		SArray<uint8_t>	bufUserName ;
		Charset::Encode
			( bufUserName, Charset::encodingUTF8, strUserName ) ;
		//
		SakuraCL::MD5Context	md5 ;
		md5.Initialize() ;
		md5.Stream( bufUserName.GetConstArray(), bufUserName.GetLength() ) ;
		md5.Flush() ;
		//
		SString	strHexId ;
		md5.GetMD5DigestHex( strHexId ) ;
		return	strHexId ;
	}
	//
	// 保存された UUID を読み込む
	//
	SString	strUUID ;
	if ( !LoadUserUniqueId( strUUID, pwszBaseDirName ) )
	{
		return	strUUID ;
	}
	//
	// 新規に UUID を生成する
	//
	uint64_t	nTimeCounter = GetPerformanceCounter() ;
	//
	DATE_TIME	dtCurrent ;
	CurrentLocalDate( dtCurrent ) ;
	nTimeCounter +=
		dtCurrent.GetAccumulatedDayCount() * (24 * 60 * 60 * 1000)
			+ ((uint64_t) dtCurrent.nHour * (60 * 60 * 1000))
			+ ((uint64_t) dtCurrent.nMinute * (60 * 1000))
			+ ((uint64_t) dtCurrent.nSecond * 1000) + dtCurrent.nMilliSec ;
	//
	SString	strTime( nTimeCounter, 16, 16 ) ;
	strTime += strUserName ;
	//
	SArray<uint8_t>	bufUUID ;
	Charset::Encode
		( bufUUID, Charset::encodingUTF8, strTime ) ;
	//
	SakuraCL::MD5Context	md5 ;
	md5.Initialize() ;
	md5.Stream( bufUUID.GetConstArray(), bufUUID.GetLength() ) ;
	md5.Flush() ;
	md5.GetMD5DigestHex( strUUID ) ;
	//
	// UUID を保存する
	//
	SaveUserUniqueId( strUUID, pwszBaseDirName ) ;
	return	strUUID ;
}

#elif	defined(__PLATFORM_ANDROID__)
// UUID のファイルからの読み込み／新規生成
//////////////////////////////////////////////////////////////////////////////
SString SGLStdApplication::GetUserUniqueId( const wchar_t * pwszBaseDirName )
{
	//
	// 保存パス
	//
	SString	strPackageName ;
	JNI::GetAndroidJavaPackageName( strPackageName ) ;
	//
	SString	strLocalDir ;
	JNI::GetAndroidLocalFilesDirectory( strLocalDir ) ;
	//
	SString	strLocalUIDPath =
		strLocalDir.OffsetFilePath( strPackageName + L".uid" ) ;
	//
	SString	strStorageUIDPath = GetUUIDStoragePath( pwszBaseDirName ) ;
	SString	strPassword = GetUUIDCryptyPassword() ;
	//
	// 読み込み試行
	//
	SString	strUUID ;
	if ( !LoadUserUniqueId( strUUID, strLocalUIDPath, strPassword ) )
	{
		if ( !strUUID.IsEmpty()
			&& IsSaveUUIDintoStorage()
			&& !strStorageUIDPath.IsEmpty()
			&& !SFile::IsExistingAbsPath(strStorageUIDPath) )
		{
			SaveUserUniqueId( strUUID, strStorageUIDPath, strPassword ) ;
		}
	}
	if ( strUUID.IsEmpty() )
	{
		if ( !LoadUserUniqueId( strUUID, strStorageUIDPath, strPassword ) )
		{
			if ( !strUUID.IsEmpty()
				&& !SFile::IsExistingAbsPath(strLocalUIDPath) )
			{
				SaveUserUniqueId( strUUID, strLocalUIDPath, strPassword ) ;
			}
		}
	}
	if ( strUUID.IsEmpty() )
	{
		//
		// EntisGLS.generateRandomUUID()
		//
		JNI::JSmartClass	jsclsEntisGLS
			( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
		jmethodID	jmidgGnerateRandomUUID =
			jsclsEntisGLS.GetStaticMethodID
				( "generateRandomUUID", "()L" JAVA_LANG_STRING ";" ) ;
		//
		JNI::JSmartObject	jsobjUUId
			( jsclsEntisGLS.CallStaticObjectMethod( jmidgGnerateRandomUUID ) ) ;
		//
		JNI::JString	jstrUUID( (jstring) jsobjUUId.GetObject() ) ;
		jstrUUID.ToString( strUUID ) ;
		//
		// 保存
		//
		if ( !strStorageUIDPath.IsEmpty() && IsSaveUUIDintoStorage() )
		{
			SaveUserUniqueId( strUUID, strStorageUIDPath, strPassword ) ;
		}
		SaveUserUniqueId( strUUID, strLocalUIDPath, strPassword ) ;
	}
	return	strUUID ;
}

#endif

// UUID 読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLStdApplication::LoadUserUniqueId
	( SSystem::SString& strUUID, const wchar_t * pwszBaseDirName )
{
	SString	strStoragePath = GetUUIDStoragePath( pwszBaseDirName ) ;
	if ( strStoragePath.IsEmpty() )
	{
		return	errFailed ;
	}
	SString	strPassword = GetUUIDCryptyPassword() ;
	return	LoadUserUniqueId( strUUID, strStoragePath, strPassword ) ;
}

SSystem::SError SGLStdApplication::LoadUserUniqueId
		( SSystem::SString& strUUID,
			const wchar_t * pwszFilePath, const wchar_t * pwszPassword )
{
	SFile	file ;
	if ( file.Open( pwszFilePath, SFileOpener::shareRead ) )
	{
		return	errFailed ;
	}
	ERISA::SGLDecrypt32File	dfw ;
	if ( dfw.Open( &file, false, pwszPassword ) )
	{
		return	errFailed ;
	}
	SStringParser	sparsUUID ;
	sparsUUID.ReadTextFile( dfw, Charset::encodingUTF8 ) ;
	//
	dfw.Close() ;
	file.Close() ;
	//
	strUUID = sparsUUID.GetString() ;
	if ( strUUID.IsEmpty() )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// UUID 保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLStdApplication::SaveUserUniqueId
	( const wchar_t * pwszUUID, const wchar_t * pwszBaseDirName )
{
	SString	strStoragePath = GetUUIDStoragePath( pwszBaseDirName ) ;
	if ( strStoragePath.IsEmpty() )
	{
		return	errFailed ;
	}
	SString	strPassword = GetUUIDCryptyPassword() ;
	return	SaveUserUniqueId( pwszUUID, strStoragePath, strPassword ) ;
}

SSystem::SError SGLStdApplication::SaveUserUniqueId
		( const wchar_t * pwszUUID,
			const wchar_t * pwszFilePath, const wchar_t * pwszPassword )
{
	SFile	file ;
	if ( file.Open( pwszFilePath, SFileOpener::modeCreate ) )
	{
		return	errFailed ;
	}
	ERISA::SGLEncrypt32FileWriter	efw ;
	if ( efw.Open( &file, false, pwszPassword ) )
	{
		return	errFailed ;
	}
	efw.WriteEncodedString( pwszUUID, -1, Charset::encodingUTF8 ) ;
	efw.Close() ;
	file.Close() ;
	return	errSuccess ;
}

// UUID ストレージ保存パス取得（空文字列を返すと保存しない）
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUUIDStoragePath( const wchar_t * pwszBaseDirName )
{
#if	defined(__PLATFORM_WINDOWS__)
	SString	strUIDPath = GetUUIDStorageFileName() ;
	SString	strAppDataDir ;
	if ( !SFile::GetDefaultDirectory
		( strAppDataDir, SFile::DefaultDirectory::ApplicationData ) )
	{
		strUIDPath = strAppDataDir.OffsetFilePath( pwszBaseDirName ).
								OffsetFilePath( GetUUIDStorageFileName() ) ;
	}
	return	strUIDPath ;

#elif	defined(__PLATFORM_ANDROID__)
	SString	strPackageName ;
	JNI::GetAndroidJavaPackageName( strPackageName ) ;
	SString	strStorageDir ;
	JNI::GetAndroidStorageDirectory( strStorageDir ) ;
	//
	SString	strUIDPath =
		strStorageDir.OffsetFilePath( pwszBaseDirName ).
								OffsetFilePath( L"data/" )
			+ strPackageName + L"/files/" + GetUUIDStorageFileName() ;
	return	strUIDPath ;

#else
	SString	strUIDPath = GetUUIDStorageFileName() ;
	return	strUIDPath ;

#endif
}

// UUID ストレージ保存用ベースディレクトリ（オフセットパス）
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUUIDStorageBaseDirectory( void )
{
	return	L"EntisGLS4" ;
}

// UUID ストレージ保存用ファイル名
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUUIDStorageFileName( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	return	L"localmachine.uid" ;

#elif	defined(__PLATFORM_ANDROID__)
	SString	strPackageName ;
	JNI::GetAndroidJavaPackageName( strPackageName ) ;
	return	strPackageName + L".uid" ;

#else
	return	L"localmachine.uid" ;
#endif
}

// UUID ストレージ保存用パスワード
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUUIDCryptyPassword( void ) const
{
	return	L"default" ;
}

// UUID をストレージに保存するか？
//////////////////////////////////////////////////////////////////////////////
bool SGLStdApplication::IsSaveUUIDintoStorage( void ) const
{
	return	false ;
}

// 進行状況表示インターフェース生成
//////////////////////////////////////////////////////////////////////////////
SSystem::SProgressiveUserInterface *
				SGLStdApplication::NewProgressiveUserInterface( void )
{
	return	NULL ;
}

// StartUpApp 中の起動中待ちダイアログ表示メッセージ変更
// ※2行目以降に追加。\r から始まる文字列の場合1行目は削除
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::ChangeStartUpSpinnerMessage( const wchar_t * pwszMsg )
{
#if	defined(__PLATFORM_ANDROID__)
	if ( m_pProgDialog != NULL )
	{
		SString	strMsg = L"起動しています…" ;
		if ( pwszMsg && (pwszMsg[0] == L'\r') )
		{
			strMsg = pwszMsg + 1 ;
		}
		else
		{
			strMsg += L"\n" ;
			strMsg += pwszMsg ;
		}
		m_pProgDialog->SetProgressiveMessage( strMsg ) ;
	}
#endif
}

// StartUpApp 中の起動中待ちダイアログを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::CloseStartUpSpinner( void )
{
#if	defined(__PLATFORM_ANDROID__)
	if ( m_pProgDialog != NULL )
	{
		m_pProgDialog->CloseProgressiveDialog() ;
		m_pProgDialog = NULL ;
	}
#endif
}

// プロファイル・タグ取得／生成 (<profile> 以下のパス)
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SGLStdApplication::GetProfileOf( const wchar_t * pwszPath )
{
	if ( !m_profile.GetFilePath().IsEmpty() )
	{
		return	m_profile.GetProfileOf( pwszPath ) ;
	}
	return	NULL ;
}

// プロファイル値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLStdApplication::GetProfileString
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, const wchar_t * pwszDefValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		SString *	pstrValue = pxmlTag->GetAttributeAs( pwszName ) ;
		if ( pstrValue != NULL )
		{
			return	*pstrValue ;
		}
	}
	return	pwszDefValue ;
}

int64_t SGLStdApplication::GetProfileInteger
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, int64_t nDefValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		return	pxmlTag->GetAttrIntegerAs( pwszName, nDefValue ) ;
	}
	return	nDefValue ;
}

int64_t SGLStdApplication::GetProfileHexInteger
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, int64_t nDefValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		return	pxmlTag->GetAttrHexIntegerAs( pwszName, nDefValue ) ;
	}
	return	nDefValue ;
}

double SGLStdApplication::GetProfileNumber
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, double nDefValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		return	pxmlTag->GetAttrRealAs( pwszName, nDefValue ) ;
	}
	return	nDefValue ;
}

// プロファイル値設定
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::SetProfileString
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, const wchar_t * pwszValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		pxmlTag->SetAttributeAs( pwszName, pwszValue ) ;
	}
}

void SGLStdApplication::SetProfileInteger
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, int64_t nValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		pxmlTag->SetAttrIntegerAs( pwszName, nValue ) ;
	}
}

void SGLStdApplication::SetProfileHexInteger
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, int64_t nValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		pxmlTag->SetAttrHexIntegerAs( pwszName, nValue ) ;
	}
}

void SGLStdApplication::SetProfileNumber
	( const wchar_t * pwszPath,
		const wchar_t * pwszName, double nValue )
{
	SXMLDocument *	pxmlTag = GetProfileOf( pwszPath ) ;
	ESLAssert( pxmlTag != NULL ) ;
	if ( pxmlTag != NULL )
	{
		pxmlTag->SetAttrRealAs( pwszName, nValue ) ;
	}
}

// プロファイル保存
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::SaveProfile( void )
{
	SString	strProfFile = m_profile.GetFilePath() ;
	if ( !strProfFile.IsEmpty() )
	{
		SString	strPassword ;
		GetProfilePassword( strPassword ) ;
		//
		m_profile.SaveProfileAs( strProfFile, strPassword ) ;
		m_profile.SaveProfileAs( strProfFile + L".mir", strPassword ) ;
	}
}

// 環境変数取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLStdApplication::GetEnvironmentVariable
		( const wchar_t * pwszName, SSystem::SString& strVar )
{
#if	defined(__COTOPHA__)
	strVar.FreeArray() ;
	return	NULL ;

#elif	defined(__PLATFORM_WINDOWS__)
	SArray<char>	bufName ;
	SString			strName = pwszName ;
	const char *	pszName = strName.EncodeDefaultTo( bufName ) ;

	DWORD	dwLen = ::GetEnvironmentVariable( pszName, NULL, 0 ) ;

	SArray<char>	bufVar ;
	dwLen = ::GetEnvironmentVariable
		( pszName, bufVar.GetArray( dwLen + 0x100 ), dwLen + 0x100 ) ;
	bufVar.FinishArray() ;
	//
	if ( (dwLen != 0) && (dwLen < bufVar.GetLength()) )
	{
		strVar = bufVar.GetConstArray() ;
		return	strVar ;
	}
	else
	{
		strVar.FreeArray() ;
		return	NULL ;
	}

#else
	SArray<char>	bufName ;
	SString			strName = pwszName ;

	char *	pszVar = getenv( strName.EncodeDefaultTo( bufName ) ) ;
	if ( pszVar != NULL )
	{
		strVar = pszVar ;
		return	strVar ;
	}
	else
	{
		strVar.FreeArray() ;
		return	NULL ;
	}

#endif
}

// 環境変数名一覧
//////////////////////////////////////////////////////////////////////////////
void SGLStdApplication::EnumerateEnvironmentVariableNames
	( SSystem::SObjectArray<SSystem::SString>& lstVarNames )
{
	lstVarNames.RemoveAll() ;

#if	defined(__COTOPHA__)

#elif	defined(__PLATFORM_WINDOWS__)
	LPCH	pszEnvStrs = GetEnvironmentStrings() ;
	LPCH	pszEnvBuf = pszEnvStrs ;
	while ( *pszEnvStrs )
	{
		ssize_t	lenName = 0 ;
		size_t	i = 0 ;
		while ( pszEnvStrs[i] )
		{
			if ( (lenName == 0) && (pszEnvStrs[i] == '=') )
			{
				lenName = (ssize_t) i ;
			}
			i ++ ;
		}
		if ( lenName > 0 )
		{
			lstVarNames.Add( new SString( pszEnvStrs, lenName ) ) ;
		}
		pszEnvStrs += i + 1 ;
	}
	FreeEnvironmentStrings( pszEnvBuf ) ;

#else
	for ( size_t i = 0; environ[i] != NULL; i ++ )
	{
		const char *	pszEnv = environ[i] ;
		ssize_t	len = 0 ;
		while ( pszEnv[len] && (pszEnv[len] != '=') )
		{
			len ++ ;
		}
		lstVarNames.Add( new SString( pszEnv, len ) ) ;
	}

#endif
}

// マシン固有値の取得
//////////////////////////////////////////////////////////////////////////////
SString SGLStdApplication::GetMachineUniqueId( void )
{
	if ( m_wszUserUniqueId[0] == 0 )
	{
		if ( m_pApp != NULL )
		{
			m_pApp->PrepareUserUniqueId() ;
		}
		else
		{
			return	GetBasicUserUniqueId() ;
		}
	}
	return	SString( m_wszUserUniqueId ) ;
}

// 表示用マシンの名前／OSバージョン
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetMachineNameAndOSVersion( void )
{
	SString	strNameAndVer ;
	bool	fName = false ;

#if	defined(__PLATFORM_WINDOWS__)
	//
	// Windows コンピュータ名
	//
	SString	strName = GetComputerName() ;
	if ( !strName.IsEmpty() )
	{
		strNameAndVer = strName ;
		fName = true ;
	}
#endif

	if ( fName )
	{
		strNameAndVer += L" (" ;
		strNameAndVer += GetOSVersionString() ;
		strNameAndVer += L")" ;
	}
	else
	{
		strNameAndVer = GetOSVersionString() ;
	}
	return	strNameAndVer ;
}

// Windows コンピューター名取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetComputerName( void )
{
	SString	strComputerName ;
#if	defined(__PLATFORM_WINDOWS__)
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		wchar_t	wszComputerName[MAX_COMPUTERNAME_LENGTH+0x101] ;
		DWORD	dwBufSize = MAX_COMPUTERNAME_LENGTH+0x101 ;
		if ( ::GetComputerNameW( wszComputerName, &dwBufSize ) )
		{
			strComputerName = wszComputerName ;
		}
	}
	else
	{
		char	szComputerName[MAX_COMPUTERNAME_LENGTH+0x101] ;
		DWORD	dwBufSize = MAX_COMPUTERNAME_LENGTH+0x101 ;
		if ( ::GetComputerName( szComputerName, &dwBufSize ) )
		{
			strComputerName = szComputerName ;
		}
	}
#endif
	return	strComputerName ;
}

// Windows ユーザー名取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetUserName( void )
{
	SString	strUserName ;
#if	defined(__PLATFORM_WINDOWS__)
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		wchar_t	wszUserName[UNLEN+0x101] ;
		DWORD	dwBufSize = UNLEN+0x101 ;
		if ( ::GetUserNameW( wszUserName, &dwBufSize ) )
		{
			strUserName = wszUserName ;
		}
	}
	else
	{
		char	szUserName[UNLEN+0x101] ;
		DWORD	dwBufSize = UNLEN+0x101 ;
		if ( ::GetUserName( szUserName, &dwBufSize ) )
		{
			strUserName = szUserName ;
		}
	}
#endif
	return	strUserName ;
}

// 表示用OSバージョン
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLStdApplication::GetOSVersionString( void )
{
	PLATFORM_INFORMATION	pi ;
	GetPlatformInformation( pi ) ;

	SString	strVersion ;
	switch ( pi.runtimeOS )
	{
	case	platformOS_Windows:
		switch ( pi.versionOS )
		{
		case	versionWindows95:
			strVersion = L"Windows95" ;
			break ;
		case	versionWindows98:
			strVersion = L"Windows98" ;
			break ;
		case	versionWindowsME:
			strVersion = L"WindowsME" ;
			break ;
		default:
			strVersion = L"Windows " ;
			strVersion += SString( pi.versionOS >> 16 ) ;
			strVersion += L"." ;
			strVersion += SString( pi.versionOS & 0xFFFF ) ;
			break ;
		}
		break ;

	case	platformOS_WindowsNT:
		switch ( pi.versionOS )
		{
		case	versionWindowsNT4:
			strVersion = L"WindowsNT4" ;
			break ;
		case	versionWindows2000:
			strVersion = L"Windows2000" ;
			break ;
		case	versionWindowsXP:
			strVersion = L"WindowsXP" ;
			break ;
		case	versionWindowsServer2003:
			strVersion = L"Windows Server 2003" ;
			break ;
		case	versionWindowsVista:
			strVersion = L"Windows Vista" ;
			break ;
		case	versionWindows7:
			strVersion = L"Windows7" ;
			break ;
		case	versionWindows8:
			strVersion = L"Windows8" ;
			break ;
		case	versionWindows8_1:
			strVersion = L"Windows8.1" ;
			break ;
		case	versionWindows10:
			strVersion = L"Windows10" ;
			break ;
		default:
			strVersion = L"WindowNT " ;
			strVersion += SString( pi.versionOS >> 16 ) ;
			strVersion += L"." ;
			strVersion += SString( pi.versionOS & 0xFFFF ) ;
			break ;
		}
		break ;

	case	platformOS_WindowsNT_Server:
		switch ( pi.versionOS )
		{
		case	versionWindowsServer2008:
			strVersion = L"Windows Server 2008" ;
			break ;
		case	versionWindowsServer2008R2:
			strVersion = L"Windows Server 2008 R2" ;
			break ;
		case	versionWindowsServer2012:
			strVersion = L"Windows Server 2012" ;
			break ;
		case	versionWindowsServer2012R2:
			strVersion = L"Windows Server 2012 R2" ;
			break ;
		case	versionWindowsServer2016:
			strVersion = L"Windows Server 2016" ;
			break ;
		default:
			strVersion = L"Window Server " ;
			strVersion += SString( pi.versionOS >> 16 ) ;
			strVersion += L"." ;
			strVersion += SString( pi.versionOS & 0xFFFF ) ;
			break ;
		}
		break ;

	case	platformOS_LinuxAndroid:
		{
			#if	defined(__PLATFORM_ANDROID__)
			//
			// android.os.Build.VERSION.RELEASER
			//
			JNI::JSmartClass	jclsEntisGLS
				( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
			jmethodID	jmidGetOSVersionString =
				jclsEntisGLS.GetStaticMethodID
					( "getOSVersionString", "()L" JAVA_LANG_STRING ";" ) ;
			//
			JNI::JSmartObject	jsobjVersion
				( jclsEntisGLS.CallStaticObjectMethod( jmidGetOSVersionString ) ) ;
			//
			if ( jsobjVersion.GetObject() != NULL )
			{
				SString			strAndroidVersion ;
				JNI::JString	jstrVersion( (jstring) jsobjVersion.GetObject() ) ;
				jstrVersion.ToString( strAndroidVersion ) ;
				//
				strVersion = L"Android " ;
				strVersion += strAndroidVersion ;
				break ;
			}
			#endif
			//
			static const wchar_t *	pwszAndroidVer[] =
			{
				L"1.0", L"1.1", L"1.5", L"1.6", L"2.0",
				L"2.0.1", L"2.1", L"2.2", L"2.3", L"2.3",
				L"3.0", L"3.1", L"3.2", L"4.0", L"4.0.3",
				L"4.1", L"4.2", L"4.3", L"4.4", L"4.4",
				L"5.0", L"5.1", L"6.0", L"7.0", L"7.1",
			} ;
			strVersion = L"Android " ;
			if ( pi.versionOS <= sizeof(pwszAndroidVer) / sizeof(pwszAndroidVer[0]) )
			{
				strVersion += pwszAndroidVer[pi.versionOS - 1] ;
			}
			else
			{
				strVersion += L"API level " ;
				strVersion += SString( pi.versionOS ) ;
			}
		}
		break ;

	default:
		strVersion = "unknown" ;
		break ;
	}
	return	strVersion ;
}


//////////////////////////////////////////////////////////////////////////////
// サービス・リスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLServiceListener )

// サービス初期化処理
//////////////////////////////////////////////////////////////////////////////
void SGLServiceListener::OnInitializeService( SGLService * service )
{
}

// サービス開始時処理
//////////////////////////////////////////////////////////////////////////////
void SGLServiceListener::OnStartService( SGLService * service )
{
}

// サービス終了時処理
//////////////////////////////////////////////////////////////////////////////
void SGLServiceListener::OnFinishService( SGLService * service )
{
}

// サービス実行
//////////////////////////////////////////////////////////////////////////////
void SGLServiceListener::OnServiceTask
	( SGLService * service, const wchar_t * pwszAction )
{
}


//////////////////////////////////////////////////////////////////////////////
// サービス
//////////////////////////////////////////////////////////////////////////////

SGLService *	SGLService::m_pService = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLService, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLService::SGLService( void )
{
	#if	defined(__PLATFORM_ANDROID__)
		m_flagStartup = false ;

	#else
		m_proc = NULL ;
		m_flagSchedule = false ;
	#endif
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLService::~SGLService( void )
{
	#if	defined(__PLATFORM_ANDROID__)

	#else
		ShutdownService() ;
		//
		delete	m_proc ;
		m_proc = NULL ;
		//
		m_thread.Delete() ;
		m_sevShutdown.Delete() ;
	#endif

	if ( m_pService == this )
	{
		m_pService = NULL ;
	}
}

// サービス開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLService::StartupService( void )
{
	#if	defined(__PLATFORM_ANDROID__)
		JNI::JSmartClass	jsclsService
			( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisService" ) ) ;
		jmethodID	jmidStartupService =
			jsclsService.GetStaticMethodID( "startupService", "()V" ) ;
		jsclsService.CallStaticVoidMethod( jmidStartupService ) ;
		//
		m_flagStartup = true ;
		m_pService = this ;

	#else
		if ( m_proc != NULL )
		{
			return	sglErrFailed ;
		}
		m_proc = new ServiceProc( this ) ;
		m_sevShutdown.Initialize( false ) ;
		if ( m_thread.BeginThread( m_proc ) )
		{
			return	sglErrFailed ;
		}
		m_pService = this ;
	#endif

	return	sglErrSuccess ;
}

// サービス終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLService::ShutdownService( void )
{
	#if	defined(__PLATFORM_ANDROID__)
		if ( m_flagStartup )
		{
			JNI::JSmartClass	jsclsService
				( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisService" ) ) ;
			jmethodID	jmidShutdownService =
				jsclsService.GetStaticMethodID( "shutdownService", "()V" ) ;
			jsclsService.CallStaticVoidMethod( jmidShutdownService ) ;
			m_flagStartup = false ;
			if ( m_pService == this )
			{
				m_pService = NULL ;
			}
		}

	#else
		if ( m_proc == NULL )
		{
			return	sglErrSuccess ;
		}
		m_sevShutdown.SetSignal() ;
		if ( m_thread.IsCurrentThread() )
		{
			Trace( "call ShutdownService on service thread\n" ) ;
		}
		else if ( m_thread.Wait( 10000 ) == errTimeout )
		{
			Trace( "timeout SGLService::ShutdownService\n" ) ;
		}
		else
		{
			m_thread.Delete() ;
			m_sevShutdown.Delete() ;
			delete	m_proc ;
			m_proc = NULL ;
			m_strTaskAction.FreeArray() ;
		}
	#endif
	return	sglErrSuccess ;
}

// スケジュール
//////////////////////////////////////////////////////////////////////////////
SGLError SGLService::ScheduleServiceTask
	( const SSystem::DATE_TIME& dt, const wchar_t * pwszAction )
{
	#if	defined(__PLATFORM_ANDROID__)
		JNI::JSmartClass	jsclsService
			( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisService" ) ) ;
		jmethodID	jmidScheduleService =
			jsclsService.GetStaticMethodID
				( "scheduleService", "(IIIIIIL" JAVA_LANG_STRING ";)Z" ) ;
		JNI::JavaObject	jobjStrAction ;
		if ( !jsclsService.CallStaticBooleanMethod
			( jmidScheduleService,
				dt.nYear, dt.nMonth, dt.nDay,
				dt.nHour, dt.nMinute, dt.nSecond,
				jobjStrAction.CreateWideString(pwszAction) ) )
		{
			return	sglErrFailed ;
		}

	#else
		m_csSync.Lock() ;
		m_flagSchedule = true ;
		m_dtTaskSchedule = dt ;
		m_strTaskAction = pwszAction ;
		m_csSync.Unlock() ;
	#endif
	return	sglErrSuccess ;
}

// スケジュール解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLService::UnscheduleServiceTask
	( const SSystem::DATE_TIME& dt, const wchar_t * pwszAction )
{
	#if	defined(__PLATFORM_ANDROID__)
		JNI::JSmartClass	jsclsService
			( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisService" ) ) ;
		jmethodID	jmidCancelSchedule =
			jsclsService.GetStaticMethodID( "cancelSchedule", "()V" ) ;
		jsclsService.CallStaticVoidMethod( jmidCancelSchedule ) ;

	#else
		m_csSync.Lock() ;
		if ( m_flagSchedule
			&& (m_dtTaskSchedule == dt)
			&& (m_strTaskAction == pwszAction) )
		{
			m_flagSchedule = false ;
		}
		m_csSync.Unlock() ;
	#endif
	return	sglErrSuccess ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLService::AttachListener( SGLServiceListener * pListener )
{
	m_csSync.Lock() ;
	m_pListener = pListener ;
	if ( m_pService == NULL )
	{
		m_pService = this ;
	}
	m_csSync.Unlock() ;
}

// サービス初期化処理
//////////////////////////////////////////////////////////////////////////////
void SGLService::OnInitializeService( void )
{
	m_csSync.Lock() ;
	if ( m_pListener != NULL )
	{
		m_pListener->OnInitializeService( this ) ;
	}
	m_csSync.Unlock() ;
}

// サービス開始時処理
//////////////////////////////////////////////////////////////////////////////
void SGLService::OnStartService( void )
{
	m_csSync.Lock() ;
	if ( m_pListener != NULL )
	{
		m_pListener->OnStartService( this ) ;
	}
	m_csSync.Unlock() ;
}

// サービス終了時処理
//////////////////////////////////////////////////////////////////////////////
void SGLService::OnFinishService( void )
{
	m_csSync.Lock() ;
	if ( m_pListener != NULL )
	{
		m_pListener->OnFinishService( this ) ;
	}
	m_csSync.Unlock() ;
}

// サービス実行
//////////////////////////////////////////////////////////////////////////////
void SGLService::OnServiceTask( const wchar_t * pwszAction )
{
	m_csSync.Lock() ;
	if ( m_pListener != NULL )
	{
		m_pListener->OnServiceTask( this, pwszAction ) ;
	}
	m_csSync.Unlock() ;
}

#if	!defined(__PLATFORM_ANDROID__)
// サービス処理スレッド
//////////////////////////////////////////////////////////////////////////////
void SGLService::ServiceProc::Run( void )
{
	SakuraGL::Initialize() ;
	m_pService->OnInitializeService() ;
	m_pService->OnStartService() ;
	//
	while ( m_pService->m_sevShutdown.Wait( 1000 ) == errTimeout )
	{
		m_pService->m_csSync.Lock() ;
		if ( m_pService->m_flagSchedule )
		{
			DATE_TIME	dtCurrent ;
			CurrentLocalDate( dtCurrent ) ;
			if ( dtCurrent >= m_pService->m_dtTaskSchedule )
			{
				m_pService->m_flagSchedule = false ;
				if ( m_pService->m_pListener != NULL )
				{
					m_pService->OnServiceTask( m_pService->m_strTaskAction ) ;
				}
			}
		}
		m_pService->m_csSync.Unlock() ;
	}
	//
	m_pService->OnFinishService() ;
//	SakuraGL::Finalize() ;
}
#endif



//////////////////////////////////////////////////////////////////////////////
// サービスイベント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLStdServiceListener::EventTask, SObject ) ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLStdServiceListener::EventTask::EventTask( void )
	: m_nPriority( 0 )
{
}

SGLStdServiceListener::EventTask::EventTask( const wchar_t * pwszID, int nPriority )
	: m_strID( pwszID ), m_nPriority( 0 )
{
}

SGLStdServiceListener::EventTask::EventTask( const SGLStdServiceListener::EventTask& evtask )
	: m_strID( evtask.m_strID ), m_nPriority( evtask.m_nPriority )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLStdServiceListener::EventTask::~EventTask( void )
{
}

// タスク処理
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::EventTask::OnTask( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// 標準的なサービスリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLStdServiceListener, SObject, SGLServiceListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLStdServiceListener::SGLStdServiceListener( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLStdServiceListener::~SGLStdServiceListener( void )
{
}

// ベースファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLStdServiceListener::GetBaseFilePath( SSystem::SString& strFileDir ) const
{
	#if	defined(__PLATFORM_ANDROID__)
		strFileDir = L"local://" ;
	#else
		strFileDir = L"" ;
	#endif
	return	strFileDir ;
}

// プロファイルパスを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLStdServiceListener::GetProfileFilePath( SSystem::SString& strFilePath ) const
{
	SString	strFileDir ;
	GetBaseFilePath( strFileDir ) ;
	//
	strFilePath = strFileDir.OffsetFilePath( L"gls4serice.profile", L'/' ) ;
	return	strFilePath ;
}

// プロファイル保存
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::SaveProfile( void )
{
	SString	strProfFile = m_profile.GetFilePath() ;
	if ( !strProfFile.IsEmpty() )
	{
		m_profile.SaveProfileAs( strProfFile, L"" ) ;
		m_profile.SaveProfileAs( strProfFile + L".mir", L"" ) ;
	}
}

// イベント追加
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::AddEventTask( SGLStdServiceListener::EventTask * pEvent )
{
	QuickLock() ;
	m_queEvent.Add( pEvent ) ;
	QuickUnlock() ;
}

// イベントキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::RemoveEventTask( SGLStdServiceListener::EventTask * pEvent )
{
	QuickLock() ;
	ssize_t	i = m_queEvent.FindPtr( pEvent ) ;
	if ( i >= 0 )
	{
		m_queEvent.RemoveAt( i ) ;
	}
	QuickUnlock() ;
}

void SGLStdServiceListener::RemoveEventTask( const wchar_t * pwszID )
{
	QuickLock() ;
	bool	fRemoved = false ;
	size_t	nCount = m_queEvent.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		EventTask *	pEvent = m_queEvent.GetAt( i ) ;
		if ( (pEvent != NULL)
			&& (pEvent->GetID() == pwszID) )
		{
			m_queEvent.SetAt( i, NULL ) ;
			fRemoved = true ;
		}
	}
	if ( fRemoved )
	{
		m_queEvent.TrimEmpty() ;
	}
	QuickUnlock() ;
}

// イベント取得
//////////////////////////////////////////////////////////////////////////////
SGLStdServiceListener::EventTask *
		SGLStdServiceListener::GetEventTask( void )
{
	QuickLock() ;
	EventTask *	pEvent = m_queEvent.DetachAt( 0 ) ;
	QuickUnlock() ;
	return	pEvent ;
}

// イベント有無判定
//////////////////////////////////////////////////////////////////////////////
bool SGLStdServiceListener::IsAnyEventTasks( void ) const
{
	return	(m_queEvent.GetLength() != 0) ;
}

// サービス初期化処理
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::OnInitializeService( SGLService * service )
{
	bool	fLoadedProfile = false ;
	SString	strProfFile ;
	GetProfileFilePath( strProfFile ) ;
	if ( !strProfFile.IsEmpty() )
	{
		if ( !m_profile.LoadProfile( strProfFile, L"" ) )
		{
			fLoadedProfile = true ;
		}
		else if ( SFile::IsExistingFile( strProfFile ) )
		{
			// ファイルが読み込めない（破損している等）
			// ミラーファイルの読み込みを試行
			if ( !m_profile.LoadProfile( strProfFile + L".mir", L"" ) )
			{
				fLoadedProfile = true ;
			}
		}
		if ( !fLoadedProfile )
		{
			m_profile.CreateProfile( strProfFile ) ;
		}
	}
}

// サービス終了時処理
//////////////////////////////////////////////////////////////////////////////
void SGLStdServiceListener::OnFinishService( SGLService * service )
{
	SaveProfile() ;
}

