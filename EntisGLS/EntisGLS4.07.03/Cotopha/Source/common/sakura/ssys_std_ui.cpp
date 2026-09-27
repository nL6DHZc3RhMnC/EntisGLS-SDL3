
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>
#include <sakuragl/sgl_window.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <shlobj.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 日本語以外への対応用
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL(LanguageType SSystem::g_languageTarget = languageJapanese) ;

const wchar_t *	SSystem::g_pwszLanguageSignatures[languageTypeCount] =
{
	L"jp", L"en"
} ;

// シグネチャから言語タイプ取得
LanguageType SSystem::GetLanguageTypeBySignature( const wchar_t * pwszLangSig )
{
	for ( int i = 0; i < languageTypeCount; i ++ )
	{
		if ( SString::Compare
			( pwszLangSig, g_pwszLanguageSignatures[i] ) == 0 )
		{
			return	(LanguageType) i ;
		}
	}
	return	languageInvalid ;
}

// 複合テキストからターゲット言語テキストを選択
// format: '\x1b' '[' <lang-sig> ']' <text> '\0' ... '\0' '\0'
// '\x1b' から始まる 0 終端テキストは言語指定ありとして判別
// '\x1b' から始まらない文字列はそのまま返す
const wchar_t * SSystem::MultiLanguageComplexText( const wchar_t * pwszMultiLangText )
{
	if ( pwszMultiLangText == NULL )
	{
		return	NULL ;
	}
	const wchar_t *	pwszDefText = pwszMultiLangText ;
	const wchar_t *	pwszText = pwszMultiLangText ;
	while ( (pwszText[0] == 0x1B) && (pwszText[1] == L'[') )
	{
		pwszText += 2 ;
		//
		if ( (pwszText[0] == L'j')
			&& (pwszText[1] == L'p')
			&& (pwszText[2] == L']') )
		{
			pwszDefText = pwszText + 3 ;
		}
		LanguageType	langTarget = languageInvalid ;
		for ( int i = 0; i < languageTypeCount; i ++ )
		{
			const wchar_t *	pwszLangSig = g_pwszLanguageSignatures[i] ;
			int	j ;
			for ( j = 0; pwszLangSig[j]; j ++ )
			{
				if ( pwszLangSig[j] != pwszText[j] )
				{
					break ;
				}
			}
			if ( (pwszLangSig[j] == 0) && (pwszText[j] == L']') )
			{
				if ( g_languageTarget == (LanguageType) i )
				{
					return	pwszText + (j + 1) ;
				}
			}
		}
		while ( *pwszText )
		{
			pwszText ++ ;
		}
		pwszText ++ ;
	}
	return	pwszText[0] ? pwszText : pwszDefText ;
}



//////////////////////////////////////////////////////////////////////////////
// OS/シェルでファイルを開く
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

SError SSystem::OpenShellFile
	( const wchar_t * pwszURI,
		SSystem::ShellAction actShell,
		const wchar_t * pwszAppPath,
		const wchar_t * pwszAppPlacement,
		uint32_t nFlags, ShellOpenResult * pResult )
{
	if ( pResult != NULL )
	{
		pResult->nFlags = 0 ;
	}
#if	defined(__PLATFORM_WINDOWS__)
	SString			strURI = pwszURI ;
	SString			strDir = pwszAppPlacement ;
	SString			strExe = pwszAppPath ;
	SArray<char>	bufURI ;
	SArray<char>	bufDir ;
	SArray<char>	bufExe ;
	//
	SHELLEXECUTEINFO	sx ;
	eslFillMemory( &sx, 0, sizeof(SHELLEXECUTEINFO) ) ;
	sx.cbSize = sizeof(SHELLEXECUTEINFO) ;
	sx.fMask = SEE_MASK_NOCLOSEPROCESS ;
	sx.lpVerb = "open" ;
	sx.lpDirectory = strDir.EncodeDefaultTo( bufDir ) ;
	sx.nShow = SW_SHOWNORMAL ;
	//
	if ( nFlags & shellNoConsole )
	{
		sx.fMask |= SEE_MASK_NO_CONSOLE ;
	}
	if ( pwszAppPath != NULL )
	{
		sx.lpFile = strExe.EncodeDefaultTo( bufExe ) ;
		sx.lpParameters = strURI.EncodeDefaultTo( bufURI ) ;
	}
	else
	{
		sx.lpFile = strURI.EncodeDefaultTo( bufURI ) ;
	}
	if ( !ShellExecuteEx( &sx ) )
	{
		return	errFailed ;
	}
	if ( pResult != NULL )
	{
		pResult->nFlags |= shellResultSuccess ;
	}
	if ( nFlags & shellOpenSync )
	{
		::WaitForInputIdle( sx.hProcess, INFINITE ) ;
	}
	if ( nFlags & shellExeSync )
	{
		::WaitForSingleObject( sx.hProcess, INFINITE ) ;
		//
		DWORD	dwExitCode ;
		if ( (pResult != NULL)
			&& ::GetExitCodeProcess( sx.hProcess, &dwExitCode ) )
		{
			pResult->nFlags |= shellResultExitCode ;
			pResult->nExitCode = (uint32_t) dwExitCode ;
		}
	}
	::CloseHandle( sx.hProcess ) ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLS.intentFileView() 呼び出し
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidIntentFileView =
		jsclsEntisGLS.GetStaticMethodID
			( "intentFileView",
				"(L" JAVA_LANG_STRING
					";IL" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING ";)Z" ) ;
	ESLAssert( jmidIntentFileView != NULL ) ;
	if ( jmidIntentFileView == NULL )
	{
		return	errFailed ;
	}
	JNI::JavaObject	jobjStrURI ;
	JNI::JavaObject	jobjStrPackage ;
	JNI::JavaObject	jobjStrClass ;
	jstring	jstrURI = NULL ;
	jstring	jstrPackage = NULL ;
	jstring	jstrClass = NULL ;
	if ( pwszURI != NULL )
	{
		jstrURI = jobjStrURI.CreateWideString( pwszURI ) ;
	}
	if ( pwszAppPlacement != NULL )
	{
		jstrPackage = jobjStrPackage.CreateWideString( pwszAppPlacement ) ;
	}
	if ( pwszAppPath != NULL )
	{
		jstrClass = jobjStrClass.CreateWideString( pwszAppPath ) ;
	}
	if ( jsclsEntisGLS.CallStaticBooleanMethod
			( jmidIntentFileView,
				jstrURI, (int) actShell, jstrPackage, jstrClass ) )
	{
		if ( pResult != NULL )
		{
			pResult->nFlags |= shellResultSuccess ;
		}
		return	errSuccess ;
	}
	return	errFailed ;

#else
	#error	no implement SSystem::OpenShellFile
#endif
}


// 特定のウィンドウをフォアグラウンドにする
//////////////////////////////////////////////////////////////////////////////
SError SSystem::ActivateWindow
	( const wchar_t * pwszName, const wchar_t * pwszClass )
{
#if	defined(__PLATFORM_WINDOWS__)
	SString	strClass = pwszClass ;
	if ( strClass.IsEmpty() )
	{
		#if	!defined(__PLATFORM_WINDOWS__) || !defined(__ENTIS_GLS__)
			strClass = SGLGenericWindow::SGL_GENERIC_WINDOW_CLASS ;
		#else
			strClass = EGameWindow::GetWindowClassName() ;
		#endif
	}
	HWND	hWnd = NULL ;
	if ( pwszName != NULL )
	{
		SString	strName = pwszName ;
		hWnd = ::FindWindow
			( strClass.ToCharArray(), strName.ToCharArray() ) ;
	}
	else
	{
		hWnd = ::FindWindow( strClass.ToCharArray(), NULL ) ;
	}
	if ( hWnd == NULL )
	{
		return	errFailed ;
	}
	::SetForegroundWindow( hWnd ) ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	JNIEnv *	env = JNI::GetJNIEnv() ;
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidActivateActivity =
		jsclsEntisGLS.GetStaticMethodID
			( "activateActivity", "(Ljava/lang/Class;)V" ) ;
	ESLAssert( jmidActivateActivity != NULL ) ;
	if ( jmidActivateActivity == NULL )
	{
		return	errFailed ;
	}
	if ( pwszClass != NULL )
	{
		SString	strClass = pwszClass ;
		strClass.Replace( L'.', L'/' ) ;
		//
		JNI::JSmartClass	jsclsActivity
			( JNI::FindJavaClass( strClass.ToCharArray() ) ) ;
		if ( (env->ExceptionOccurred() != NULL)
			|| (jsclsActivity.GetObject() == NULL) )
		{
			env->ExceptionClear() ;
			return	errFailed ;
		}
		jsclsEntisGLS.CallStaticVoidMethod
				( jmidActivateActivity, jsclsActivity.GetObject() ) ;
	}
	else
	{
		jsclsEntisGLS.CallStaticVoidMethod( jmidActivateActivity, NULL ) ;
	}
	return	errSuccess ;

#else
	#error	no implement SSystem::ActivateWindow
#endif
}

#endif


// ディレクトリを選択
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)

#if	defined(__PLATFORM_WINDOWS__)
class	BrowseFolderProcedure	: public SSyncProcedure
{
public:
	bool			m_flagResult ;
	HWND			m_hwndParent ;
	SString			m_strCaption ;
	SString			m_strDirPath ;
public:
	BrowseFolderProcedure( void ) : SSyncProcedure( NULL ) {}
	virtual void Run( void ) ;
} ;

int SSystem::DoBrowseDirectoryDialog
	( SString& strDirPath,
		const wchar_t * pwszCaption,
		const wchar_t * pwszInitDir,
		uint32_t nFlags, HWND hParentWnd )
{
	if ( pwszCaption == NULL )
	{
		pwszCaption = L"ディレクトリ選択" ;
	}
	BrowseFolderProcedure	bf ;
	bf.m_flagResult = false ;
	bf.m_hwndParent = hParentWnd ;
	bf.m_strCaption = pwszCaption ;
	bf.m_strDirPath = pwszInitDir ;
	//
	bf.Run() ;
	//
	if ( bf.m_flagResult )
	{
		strDirPath = bf.m_strDirPath ;
		return	msgboxResultOk ;
	}
	return	msgboxResultCancel ;
}
#endif

int SSystem::BrowseDirectoryDialog
	( SString& strDirPath,
		const wchar_t * pwszCaption,
		const wchar_t * pwszInitDir,
		uint32_t nFlags, SakuraGL::Window * pParentWnd )
{
	if ( pwszCaption == NULL )
	{
		pwszCaption = L"ディレクトリ選択" ;
	}
#if	defined(__PLATFORM_WINDOWS__)
	//
	// Windows 標準ダイアログ呼び出し
	//
	if ( pParentWnd == NULL )
	{
		QuickLock() ;
		pParentWnd = SGLAbstractWindow::GetDefaultWindow() ;
		QuickUnlock() ;
	}
	BrowseFolderProcedure	bf ;
	bf.m_flagResult = false ;
	bf.m_hwndParent = NULL ;
	bf.m_strCaption = pwszCaption ;
	bf.m_strDirPath = pwszInitDir ;
	if ( pParentWnd != NULL )
	{
		bf.m_hwndParent = pParentWnd->GetWindowHandle() ;
		if ( !pParentWnd->PostUIThread( &bf ) )
		{
			bf.WaitDone() ;
		}
		else
		{
			bf.Run() ;
		}
	}
	else
	{
		bf.Run() ;
	}
	if ( bf.m_flagResult )
	{
		strDirPath = bf.m_strDirPath ;
		return	msgboxResultOk ;
	}
	return	msgboxResultCancel ;

#elif	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLS.doBrowseDirectoryDialog() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidDoBrowseDirectoryDialog =
		jsclsEntisGLS.GetStaticMethodID
			( "doBrowseDirectoryDialog",
				"(L" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING
					";I)L" JAVA_LANG_STRING ";" ) ;
	ESLAssert( jmidDoBrowseDirectoryDialog != NULL ) ;
	if ( jmidDoBrowseDirectoryDialog == NULL )
	{
		SSystem::Relock( countLocked ) ;
		return	msgboxResultCancel ;
	}
	JNI::JavaObject	jobjStrCaption ;
	JNI::JavaObject	jobjStrInitDir ;
	jstring	jstrCaption = NULL ;
	jstring	jstrInitDir = NULL ;
	if ( pwszCaption != NULL )
	{
		jstrCaption = jobjStrCaption.CreateWideString( pwszCaption ) ;
	}
	if ( pwszInitDir != NULL )
	{
		jstrInitDir = jobjStrInitDir.CreateWideString( pwszInitDir ) ;
	}
	JNI::JSmartObject	jsobjResult =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidDoBrowseDirectoryDialog,
				jstrCaption, jstrInitDir, nFlags ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	if ( jsobjResult.GetObject() == NULL )
	{
		return	msgboxResultCancel ;
	}
	JNI::JString	jstrResult( (jstring) jsobjResult.GetObject() ) ;
	jstrResult.ToString( strDirPath ) ;
	return	msgboxResultOk ;

#else
	#error	no implement BrowseDirectoryDialog
#endif
}

#if	defined(__PLATFORM_WINDOWS__)

static int CALLBACK BrowseCallbackProc
			( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData ) ;

void BrowseFolderProcedure::Run( void )
{
	BROWSEINFO		bi ;
	SArray<char>	strDispName ;
	SArray<char>	strCaption ;
	SArray<char>	strDirPath ;
	strDispName.SetLength( MAX_PATH ) ;
	//
	::memset( &bi, 0, sizeof(bi) ) ;
	bi.hwndOwner = m_hwndParent ;
	bi.pszDisplayName = strDispName.GetArray() ;
	bi.lpszTitle = m_strCaption.EncodeDefaultTo( strCaption ) ;
	bi.lpfn = &BrowseCallbackProc ;
	bi.lParam = (LPARAM) m_strDirPath.EncodeDefaultTo( strDirPath ) ;
	//
	LPITEMIDLIST	piidl = ::SHBrowseForFolder( &bi ) ;
	if ( piidl != NULL )
	{
		::SHGetPathFromIDList( piidl, bi.pszDisplayName ) ;
		::CoTaskMemFree( piidl ) ;
		m_strDirPath = bi.pszDisplayName ;
		m_flagResult = true ;
	}
	else
	{
		m_flagResult = false ;
	}
	strDispName.FinishArray() ;
	//
	if ( bi.hwndOwner != NULL )
	{
		::EnableWindow( bi.hwndOwner, TRUE ) ;
	}
}

static int CALLBACK BrowseCallbackProc
	( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if ( uMsg == BFFM_INITIALIZED )
	{
		SendMessage( hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, lpData ) ;
	}
	return	0 ;
}

#endif
#endif


// ファイルを選択（既存ファイル）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)

#if	defined(__PLATFORM_WINDOWS__)
class	BrowseOpenFileProcedure	: public SSyncProcedure
{
public:
	BOOL			m_flagResult ;
	OPENFILENAME *	m_pofn ;
public:
	BrowseOpenFileProcedure( void ) : SSyncProcedure( NULL ) {}
	virtual void Run( void ) ;
} ;
static char * ConvertFileDialogFilter
	( SArray<char>& bufFilter, const wchar_t ** ppwszFileFilters ) ;
#elif	defined(__PLATFORM_ANDROID__)
static void EnumFileDialogFilter
	( SObjectArray<SString>& aExtFilter, const wchar_t ** ppwszFileFilters ) ;
#endif

int SSystem::BrowseOpenFileDialog
	( SString& strFilePath,
		const wchar_t * pwszCaption,
		const wchar_t * pwszInitDir,
		const wchar_t ** ppwszFileFilters,
		uint32_t nFlags, SakuraGL::Window * pParentWnd )
{
	if ( pwszCaption == NULL )
	{
		pwszCaption = L"ファイルを開く" ;
	}
#if	defined(__PLATFORM_WINDOWS__)
	//
	// Windows コモンダイアログを呼び出し
	//
	if ( pParentWnd == NULL )
	{
		QuickLock() ;
		pParentWnd = SGLAbstractWindow::GetDefaultWindow() ;
		QuickUnlock() ;
	}
	OPENFILENAME	ofn ;
	SArray<char>	bufFilter ;
	SArray<char>	bufFilePath ;
	SArray<char>	bufInitDir ;
	SArray<char>	bufTitle ;
	eslFillMemory( &ofn, 0, sizeof(OPENFILENAME) ) ;
	ofn.lStructSize = sizeof(OPENFILENAME) ;
	if ( pParentWnd != NULL )
	{
		ofn.hwndOwner = pParentWnd->GetWindowHandle() ;
	}
	ofn.lpstrFilter = ConvertFileDialogFilter( bufFilter, ppwszFileFilters ) ;
	ofn.lpstrFile = bufFilePath.GetArray( 0x400 ) ;
	ofn.nMaxFile = 0x400 ;
	ofn.lpstrInitialDir =
			SString(pwszInitDir).EncodeDefaultTo( bufInitDir ) ;
	ofn.lpstrTitle =
			SString(pwszCaption).EncodeDefaultTo( bufTitle ) ;
	ofn.Flags = OFN_HIDEREADONLY ;

	BrowseOpenFileProcedure	bofp;
	bofp.m_pofn = &ofn ;
	//
	if ( pParentWnd != NULL )
	{
		pParentWnd->PostUIThread( &bofp ) ;
		bofp.WaitDone() ;
	}
	else
	{
		bofp.Run() ;
	}
	bufFilePath.FinishArray() ;
	//
	if ( bofp.m_flagResult )
	{
		strFilePath = ofn.lpstrFile ;
		return	msgboxResultOk ;
	}
	return	msgboxResultCancel ;

#elif	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLS.doOpenFileDialog() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidDoOpenFileDialog =
		jsclsEntisGLS.GetStaticMethodID
			( "doOpenFileDialog",
				"(L" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING
					";[L" JAVA_LANG_STRING
					";I)L" JAVA_LANG_STRING ";" ) ;
	ESLAssert( jmidDoOpenFileDialog != NULL ) ;
	if ( jmidDoOpenFileDialog == NULL )
	{
		SSystem::Relock( countLocked ) ;
		return	msgboxResultCancel ;
	}
	JNI::JavaObject	jobjStrCaption ;
	JNI::JavaObject	jobjStrInitDir ;
	JNI::JavaObject	jobjArrFilter ;
	jstring	jstrCaption = NULL ;
	jstring	jstrInitDir = NULL ;
	if ( pwszCaption != NULL )
	{
		jstrCaption = jobjStrCaption.CreateWideString( pwszCaption ) ;
	}
	if ( pwszInitDir != NULL )
	{
		jstrInitDir = jobjStrInitDir.CreateWideString( pwszInitDir ) ;
	}
	if ( ppwszFileFilters != NULL )
	{
		SObjectArray<SString>	aExtFilter ;
		EnumFileDialogFilter( aExtFilter, ppwszFileFilters ) ;
		//
		JNI::JSmartClass
			jsclsString( JNI::FindJavaClass( JAVA_LANG_STRING ) ) ;
		jobjectArray	jobjArr =
			jobjArrFilter.CreateObjectArray
				( (jsize) aExtFilter.GetLength(), jsclsString.GetObject() ) ;
		JNI::JObjectArray	joarrFilter( jobjArr ) ;
		for ( size_t i = 0; i < aExtFilter.GetLength(); i ++ )
		{
			JNI::JavaObject	jobjStrExt ;
			ESLAssert( aExtFilter.GetAt(i) != NULL ) ;
			joarrFilter.SetAt
				( (jsize) i,
					jobjStrExt.CreateWideString( aExtFilter.At(i) ) ) ;
		}
	}
	JNI::JSmartObject	jsobjResult =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidDoOpenFileDialog,
				jstrCaption, jstrInitDir,
				jobjArrFilter.GetObject(), nFlags ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	if ( jsobjResult.GetObject() == NULL )
	{
		return	msgboxResultCancel ;
	}
	JNI::JString	jstrResult( (jstring) jsobjResult.GetObject() ) ;
	jstrResult.ToString( strFilePath ) ;
	return	msgboxResultOk ;

#else
	#error no implement BrowseOpenFileDialog
#endif
}

#if	defined(__PLATFORM_WINDOWS__)

void BrowseOpenFileProcedure::Run( void )
{
	m_flagResult = ::GetOpenFileName( m_pofn ) ;
}

static char * ConvertFileDialogFilter
	( SArray<char>& bufFilter, const wchar_t ** ppwszFileFilters )
{
	if ( ppwszFileFilters != NULL )
	{
		SString	strFilter ;
		for ( int i = 0; ppwszFileFilters[i] != NULL; i += 2 )
		{
			SString	strExts = ppwszFileFilters[i + 1] ;
			ssize_t	iLast = 0 ;
			strFilter += ppwszFileFilters[i] ;
			strFilter += (wchar_t) 0 ;
			for ( ; ; )
			{
				ssize_t	iNext = strExts.Find( L';', (size_t) iLast ) ;
				if ( iNext < 0 )
				{
					strFilter += L"*." ;
					strFilter += strExts.Middle( (size_t) iLast ) ;
					break ;
				}
				strFilter += L"*." ;
				strFilter +=
					strExts.Middle( (size_t) iLast, iNext - iLast ) ;
				strFilter += L";" ;
				iLast = iNext + 1 ;
			}
			strFilter += (wchar_t) 0 ;
		}
		return	(char*) strFilter.EncodeDefaultTo( bufFilter ) ;
	}
	else
	{
		return	"all files\0*.*\0\0" ;
	}
}

#elif	defined(__PLATFORM_ANDROID__)

static void EnumFileDialogFilter
	( SObjectArray<SString>& aExtFilter, const wchar_t ** ppwszFileFilters )
{
	if ( ppwszFileFilters != NULL )
	{
		for ( int i = 0; ppwszFileFilters[i] != NULL; i += 2 )
		{
			SString	strExts = ppwszFileFilters[i + 1] ;
			ssize_t	iLast = 0 ;
			for ( ; ; )
			{
				ssize_t	iNext = strExts.Find( L';', (size_t) iLast ) ;
				if ( iNext < 0 )
				{
					aExtFilter.Add
						( new SString
							( strExts.Middle( (size_t) iLast ) ) ) ;
					break ;
				}
				aExtFilter.Add
					( new SString
						( strExts.Middle
							( (size_t) iLast, iNext - iLast ) ) ) ;
				iLast = iNext + 1 ;
			}
		}
	}
}

#endif

#endif


// ファイルを選択（既存／新規ファイル）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)

#if	defined(__PLATFORM_WINDOWS__)
class	BrowseSaveFileProcedure	: public SSyncProcedure
{
public:
	BOOL			m_flagResult ;
	OPENFILENAME *	m_pofn ;
public:
	BrowseSaveFileProcedure( void ) : SSyncProcedure( NULL ) {}
	virtual void Run( void ) ;
} ;
#endif

int SSystem::BrowseSaveFileDialog
	( SString& strFilePath,
		const wchar_t * pwszCaption,
		const wchar_t * pwszInitDir,
		const wchar_t ** ppwszFileFilters,
		uint32_t nFlags, SakuraGL::Window * pParentWnd )
{
	if ( pwszCaption == NULL )
	{
		pwszCaption = L"ファイルを保存" ;
	}
#if	defined(__PLATFORM_WINDOWS__)
	//
	// Windows コモンダイアログを呼び出し
	//
	if ( pParentWnd == NULL )
	{
		QuickLock() ;
		pParentWnd = SGLAbstractWindow::GetDefaultWindow() ;
		QuickUnlock() ;
	}
	OPENFILENAME	ofn ;
	SArray<char>	bufFilter ;
	SArray<char>	bufFilePath ;
	SArray<char>	bufInitDir ;
	SArray<char>	bufTitle ;
	eslFillMemory( &ofn, 0, sizeof(OPENFILENAME) ) ;
	ofn.lStructSize = sizeof(OPENFILENAME) ;
	if ( pParentWnd != NULL )
	{
		ofn.hwndOwner = pParentWnd->GetWindowHandle() ;
	}
	ofn.lpstrFilter = ConvertFileDialogFilter( bufFilter, ppwszFileFilters ) ;
	ofn.lpstrFile = bufFilePath.GetArray( 0x400 ) ;
	ofn.nMaxFile = 0x400 ;
	ofn.lpstrInitialDir =
			SString(pwszInitDir).EncodeDefaultTo( bufInitDir ) ;
	ofn.lpstrTitle =
			SString(pwszCaption).EncodeDefaultTo( bufTitle ) ;
	ofn.Flags = OFN_OVERWRITEPROMPT ;

	BrowseSaveFileProcedure	bsfp;
	bsfp.m_pofn = &ofn ;
	//
	if ( pParentWnd != NULL )
	{
		pParentWnd->PostUIThread( &bsfp ) ;
		bsfp.WaitDone() ;
	}
	else
	{
		bsfp.Run() ;
	}
	bufFilePath.FinishArray() ;
	//
	if ( bsfp.m_flagResult )
	{
		strFilePath = ofn.lpstrFile ;
		return	msgboxResultOk ;
	}
	return	msgboxResultCancel ;

#elif	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLS.doSaveFileDialog() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidDoSaveFileDialog =
		jsclsEntisGLS.GetStaticMethodID
			( "doSaveFileDialog",
				"(L" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING
					";[L" JAVA_LANG_STRING
					";I)L" JAVA_LANG_STRING ";" ) ;
	ESLAssert( jmidDoSaveFileDialog != NULL ) ;
	if ( jmidDoSaveFileDialog == NULL )
	{
		SSystem::Relock( countLocked ) ;
		return	msgboxResultCancel ;
	}
	JNI::JavaObject	jobjStrCaption ;
	JNI::JavaObject	jobjStrInitDir ;
	JNI::JavaObject	jobjArrFilter ;
	jstring	jstrCaption = NULL ;
	jstring	jstrInitDir = NULL ;
	if ( pwszCaption != NULL )
	{
		jstrCaption = jobjStrCaption.CreateWideString( pwszCaption ) ;
	}
	if ( pwszInitDir != NULL )
	{
		jstrInitDir = jobjStrInitDir.CreateWideString( pwszInitDir ) ;
	}
	if ( ppwszFileFilters != NULL )
	{
		SObjectArray<SString>	aExtFilter ;
		EnumFileDialogFilter( aExtFilter, ppwszFileFilters ) ;
		//
		JNI::JSmartClass
			jsclsString( JNI::FindJavaClass( JAVA_LANG_STRING ) ) ;
		jobjectArray	jobjArr =
			jobjArrFilter.CreateObjectArray
				( (jsize) aExtFilter.GetLength(), jsclsString.GetObject() ) ;
		JNI::JObjectArray	joarrFilter( jobjArr ) ;
		for ( size_t i = 0; i < aExtFilter.GetLength(); i ++ )
		{
			JNI::JavaObject	jobjStrExt ;
			ESLAssert( aExtFilter.GetAt(i) != NULL ) ;
			joarrFilter.SetAt
				( (jsize) i,
					jobjStrExt.CreateWideString( aExtFilter.At(i) ) ) ;
		}
	}
	JNI::JSmartObject	jsobjResult =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidDoSaveFileDialog,
				jstrCaption, jstrInitDir,
				jobjArrFilter.GetObject(), nFlags ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	if ( jsobjResult.GetObject() == NULL )
	{
		return	msgboxResultCancel ;
	}
	JNI::JString	jstrResult( (jstring) jsobjResult.GetObject() ) ;
	jstrResult.ToString( strFilePath ) ;
	return	msgboxResultOk ;

#else
	#error no implement BrowseSaveFileDialog
#endif
}

#if	defined(__PLATFORM_WINDOWS__)

void BrowseSaveFileProcedure::Run( void )
{
	m_flagResult = ::GetSaveFileName( m_pofn ) ;
}

#endif

#endif


//////////////////////////////////////////////////////////////////////////////
// 入力テキストボックス
//////////////////////////////////////////////////////////////////////////////

#if	defined(__PLATFORM_WINDOWS__)

/*
#define IDD_PROGRESSIVE_DIALOG          130
#define IDC_STATIC_MESSAGE              1004
#define IDC_EDIT_TEXT                   1005

IDD_MSG_EDIT_DIALOG DIALOGEX 0, 0, 293, 70
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Caption"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    DEFPUSHBUTTON   "OK",IDOK,180,48,50,14
    PUSHBUTTON      "キャンセル",IDCANCEL,234,48,50,14
    LTEXT           "Text",IDC_STATIC_MESSAGE,12,6,270,12,SS_CENTERIMAGE
    EDITTEXT        IDC_EDIT_TEXT,12,24,270,12,ES_AUTOHSCROLL
END
*/
const BYTE	SMessageEditBoxDialog::m_bytEditBoxDlgData[232] =
{
	0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0xC8, 0x80, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x01, 
	0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 
	0x61, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00, 
	0x6F, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x08, 0x00, 
	0x90, 0x01, 0x00, 0x01, 0x4D, 0x00, 0x53, 0x00, 
	0x20, 0x00, 0x53, 0x00, 0x68, 0x00, 0x65, 0x00, 
	0x6C, 0x00, 0x6C, 0x00, 0x20, 0x00, 0x44, 0x00, 
	0x6C, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x01, 0x50, 0xB4, 0x00, 0x30, 0x00, 
	0x32, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0x80, 0x00, 0x4F, 0x00, 0x4B, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x50, 
	0xEA, 0x00, 0x30, 0x00, 0x32, 0x00, 0x0E, 0x00, 
	0x02, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x80, 0x00, 
	0xAD, 0x30, 0xE3, 0x30, 0xF3, 0x30, 0xBB, 0x30, 
	0xEB, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x50, 0x0C, 0x00, 0x06, 0x00, 
	0x0E, 0x01, 0x0C, 0x00, 0xEC, 0x03, 0x00, 0x00, 
	0xFF, 0xFF, 0x82, 0x00, 0x54, 0x00, 0x65, 0x00, 
	0x78, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x00, 0x81, 0x50, 0x0C, 0x00, 0x18, 0x00, 
	0x0E, 0x01, 0x0C, 0x00, 0xED, 0x03, 0x00, 0x00, 
	0xFF, 0xFF, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 
} ;

// ダイアログ関数
INT_PTR CALLBACK SMessageEditBoxDialog::EditBoxDialogProc
	( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	#if	!defined(DWLP_USER)
		enum	{ DWLP_USER = DWL_USER ; } ;
	#endif
	SMessageEditBoxDialog *	pdlg =
		(SMessageEditBoxDialog*) ::GetWindowLongPtr( hwndDlg, DWLP_USER ) ;
	SArray<char>	bufText ;
	int				cchMax = 0x100 ;
	switch ( uMsg )
	{
	case	WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case	IDOK:
			while ( cchMax && pdlg )
			{
				bufText.SetLength( cchMax + 1 ) ;
				UINT	nResult = 
					::GetDlgItemText
						( hwndDlg, IDC_EDIT_TEXT, bufText.GetArray(), cchMax ) ;
				bufText.FinishArray() ;
				//
				if ( nResult < (UINT) cchMax )
				{
					pdlg->m_strEdit = bufText.GetConstArray() ;
					break ;
				}
				cchMax <<= 1 ;
			}
			pdlg->m_nResult = msgboxResultOk ;
			::EndDialog( hwndDlg, IDOK ) ;
			return	0 ;
		case	IDCANCEL:
			pdlg->m_nResult = msgboxResultCancel ;
			::EndDialog( hwndDlg, IDCANCEL ) ;
			return	0 ;
		}
		break ;

	case	WM_CLOSE:
		::EndDialog( hwndDlg, IDCANCEL ) ;
		return	0 ;

	case	WM_INITDIALOG:
		pdlg = (SMessageEditBoxDialog*) lParam ;
		ESLAssert( pdlg != NULL ) ;
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam ) ;
		if ( pdlg != NULL )
		{
			pdlg->m_hDialog = hwndDlg ;
			//
			HWND	hwndEdit = ::GetDlgItem( hwndDlg, IDC_EDIT_TEXT ) ;
			LONG	lStyle = ::GetWindowLong( hwndEdit, GWL_STYLE ) ;
			int		yDlgExpand = 0 ;
			RECT	rctEdit ;
			bool	flagChangeEdit = false ;
			::GetWindowRect( hwndEdit, &rctEdit ) ;
			if ( pdlg->m_nStyles & editboxStyleMultiLine )
			{
				lStyle |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL ;
				//
				static const UINT	nItemIDs[2] =
				{
					IDOK, IDCANCEL
				} ;
				yDlgExpand += (rctEdit.bottom - rctEdit.top) * 4 ;
				OffsetDialogItems( hwndDlg, nItemIDs, 2, 0, yDlgExpand ) ;
				//
				rctEdit.bottom += yDlgExpand ;
				flagChangeEdit = true ;
			}
			if ( pdlg->m_nStyles & editboxStyleNumber )
			{
				lStyle |= ES_NUMBER ;
				flagChangeEdit = true ;
			}
			if ( pdlg->m_nStyles & editboxStylePassword )
			{
				lStyle |= ES_PASSWORD ;
				flagChangeEdit = true ;
			}
			if ( flagChangeEdit )
			{
				::DestroyWindow( hwndEdit ) ;
				//
				POINT	ptEdit = { rctEdit.left, rctEdit.top } ;
				OffsetDialogClientPos( hwndDlg, ptEdit ) ;
				hwndEdit = ::CreateWindowEx
					( WS_EX_CLIENTEDGE, "edit", "",
					lStyle, ptEdit.x, ptEdit.y,
					(rctEdit.right - rctEdit.left),
					(rctEdit.bottom - rctEdit.top),
					hwndDlg, (HMENU) IDC_EDIT_TEXT,
					::GetModuleHandle( NULL ), NULL ) ;
			}
			if ( pdlg->m_strMessage.IsEmpty() )
			{
				HWND	hwndMsg = ::GetDlgItem( hwndDlg, IDC_STATIC_MESSAGE ) ;
				RECT	rctMsg ;
				::GetWindowRect( hwndMsg, &rctMsg ) ;
				::ShowWindow( hwndMsg, SW_HIDE ) ;
				//
				static const UINT	nItemIDs[3] =
				{
					IDC_EDIT_TEXT, IDOK, IDCANCEL
				} ;
				yDlgExpand -= (rctMsg.bottom - rctMsg.top) ;
				OffsetDialogItems
					( hwndDlg, nItemIDs, 3, 0, -(rctMsg.bottom - rctMsg.top) ) ;
			}
			//
			RECT	rctParent ;
			HWND	hwndParent = ::GetParent( hwndDlg ) ;
			if ( hwndParent != NULL )
			{
				::GetWindowRect( hwndParent, &rctParent ) ;
			}
			else
			{
				rctParent.left = 0 ;
				rctParent.top = 0 ;
				rctParent.right = ::GetSystemMetrics( SM_CXSCREEN ) ;
				rctParent.bottom = ::GetSystemMetrics( SM_CYSCREEN ) ;
			}
			RECT	rctDlg ;
			::GetWindowRect( hwndDlg, &rctDlg ) ;
			::SetWindowPos
				( hwndDlg, NULL,
					((rctParent.right - rctParent.left)
						- (rctDlg.right - rctDlg.left)) / 2 + rctParent.left,
					((rctParent.bottom - rctParent.top)
						- (rctDlg.bottom - rctDlg.top + yDlgExpand)) / 2 + rctParent.top,
					rctDlg.right - rctDlg.left,
					rctDlg.bottom - rctDlg.top + yDlgExpand, SWP_NOZORDER ) ;
			//
			if ( !pdlg->m_strCaption.IsEmpty() )
			{
				::SetWindowText( hwndDlg, pdlg->m_strCaption.ToCharArray() ) ;
			}
			if ( !pdlg->m_strMessage.IsEmpty() )
			{
				::SetDlgItemText
					( hwndDlg, IDC_STATIC_MESSAGE,
						pdlg->m_strMessage.ToCharArray() ) ;
			}
			if ( !pdlg->m_strEdit.IsEmpty() )
			{
				::SetDlgItemText
					( hwndDlg, IDC_EDIT_TEXT,
						pdlg->m_strEdit.ToCharArray() ) ;
			}
			::SetFocus( hwndEdit ) ;
			//
			pdlg->m_signalCreated.SetSignal() ;
		}
		break ;
	}
	return	0 ;
}

// ダイアログアイテムを移動する
//////////////////////////////////////////////////////////////////////////////
void SMessageEditBoxDialog::OffsetDialogItems
	( HWND hwndDlg, const UINT * pItemIDs,
				size_t nCount, int xOffset, int yOffset )
{
	RECT	rctDialog, rctClient ;
	if ( !::GetWindowRect( hwndDlg, &rctDialog ) )
	{
		return ;
	}
	if ( !::GetClientRect( hwndDlg, &rctClient ) )
	{
		return ;
	}
	POINT	ptClient = { rctClient.left, rctClient.top } ;
	::ClientToScreen( hwndDlg, &ptClient ) ;
	//
	xOffset -= ptClient.x ;
	yOffset -= ptClient.y ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		HWND	hwndItem = ::GetDlgItem( hwndDlg, pItemIDs[i] ) ;
		if ( hwndItem )
		{
			RECT	rctItem ;
			if ( ::GetWindowRect( hwndItem, &rctItem ) )
			{
				::MoveWindow
					( hwndItem,
						rctItem.left + xOffset,
						rctItem.top + yOffset,
						rctItem.right - rctItem.left,
						rctItem.bottom - rctItem.top, TRUE ) ;
			}
		}
	}
}

// ダイアログアイテム座標補正値を取得
//////////////////////////////////////////////////////////////////////////////
void SMessageEditBoxDialog::OffsetDialogClientPos( HWND hwndDlg, POINT& ptOffset )
{
	RECT	rctDialog, rctClient ;
	if ( !::GetWindowRect( hwndDlg, &rctDialog ) )
	{
		return ;
	}
	if ( !::GetClientRect( hwndDlg, &rctClient ) )
	{
		return ;
	}
	POINT	ptClient = { rctClient.left, rctClient.top } ;
	::ClientToScreen( hwndDlg, &ptClient ) ;
	//
	ptOffset.x -= ptClient.x ;
	ptOffset.y -= ptClient.y ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SMessageEditBoxDialog::Run( void )
{
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	//
	::DialogBoxIndirectParam
		( ::GetModuleHandle( NULL ),
			(LPCDLGTEMPLATE) m_bytEditBoxDlgData,
			m_hParentWnd, EditBoxDialogProc, (LPARAM) this ) ;
	//
	if ( m_hParentWnd != NULL )
	{
		::EnableWindow( m_hParentWnd, TRUE ) ;
	}
	m_signalCreated.SetSignal() ;
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SMessageEditBoxDialog, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SMessageEditBoxDialog::SMessageEditBoxDialog( void )
{
	m_hDialog = NULL ;
	m_hParentWnd = NULL ;
	m_nStyles = 0 ;
	m_nResult = msgboxResultCancel ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SMessageEditBoxDialog::~SMessageEditBoxDialog( void )
{
	if ( m_hDialog != NULL )
	{
		::EndDialog( m_hDialog, IDOK ) ;
		m_threadUI.Wait() ;
		m_threadUI.Delete() ;
		m_signalCreated.Delete() ;
		m_hDialog = NULL ;
	}
}

// 実行
//////////////////////////////////////////////////////////////////////////////
SError SMessageEditBoxDialog::DoModal
	( const wchar_t * pwszInitEdit,
		const wchar_t * pwszMsg,
		const wchar_t * pwszCaption,
		int nStyles, SakuraGL::Window * pParentWnd )
{
	if ( m_hDialog != NULL )
	{
		return	errFailed ;
	}
	m_hParentWnd = NULL ;
	if ( pParentWnd == NULL )
	{
		QuickLock() ;
		SGLAbstractWindow *	pDefWindow = SGLAbstractWindow::GetDefaultWindow() ;
		if ( pDefWindow != NULL )
		{
			m_hParentWnd = pDefWindow->GetWindowHandle() ;
		}
		QuickUnlock() ;
	}
	else
	{
		m_hParentWnd = pParentWnd->GetWindowHandle() ;
	}
	m_strCaption = pwszCaption ;
	if ( m_strCaption.IsEmpty() )
	{
		m_strCaption = L"文字列入力" ;
	}
	m_strMessage = pwszMsg ;
	m_strEdit = pwszInitEdit ;
	m_nStyles = nStyles ;
	m_nResult = msgboxResultCancel ;
	//
	m_signalCreated.Initialize( false ) ;
	if ( m_threadUI.BeginThread( this ) )
	{
		return	errFailed ;
	}
	m_signalCreated.Wait() ;
	m_threadUI.Wait() ;
	return	errSuccess ;
}

// 編集文字列取得
//////////////////////////////////////////////////////////////////////////////
const SString& SMessageEditBoxDialog::GetEditString( void ) const
{
	return	m_strEdit ;
}

// 結果取得
//////////////////////////////////////////////////////////////////////////////
int SMessageEditBoxDialog::GetResult( void ) const
{
	return	m_nResult ;
}

#endif



// 文字列入力
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

int SSystem::MessageEditBox
	( SString& strEditText,
		const wchar_t * pwszMsg,
		const wchar_t * pwszCaption,
		int nStyles, SakuraGL::Window * pParentWnd )
{
#if	defined(__PLATFORM_WINDOWS__)
	SMessageEditBoxDialog	dlg ;
	if ( dlg.DoModal
		( strEditText, pwszMsg, pwszCaption, nStyles, pParentWnd ) )
	{
		return	msgboxResultCancel ;
	}
	strEditText = dlg.GetEditString() ;
	return	dlg.GetResult() ;

#elif	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLS.doMessageEditBox() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidDoMessageEditBox =
		jsclsEntisGLS.GetStaticMethodID
			( "doMessageEditBox",
				"(L" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING
					";L" JAVA_LANG_STRING
					";I)L" JAVA_LANG_STRING ";" ) ;
	ESLAssert( jmidDoMessageEditBox != NULL ) ;
	if ( jmidDoMessageEditBox == NULL )
	{
		SSystem::Relock( countLocked ) ;
		return	msgboxResultCancel ;
	}
	//
	JNI::JavaObject	jobjStrTitle ;
	JNI::JavaObject	jobjStrMsg ;
	JNI::JavaObject	jobjStrEdit ;
	jstring	jstrTitle = NULL ;
	jstring	jstrMsg = NULL ;
	jstring	jstrInitEdit = NULL ;
	if ( pwszCaption != NULL )
	{
		jstrTitle = jobjStrTitle.CreateWideString( pwszCaption ) ;
	}
	if ( pwszMsg != NULL )
	{
		jstrMsg = jobjStrMsg.CreateWideString( pwszMsg ) ;
	}
	if ( !strEditText.IsEmpty() )
	{
		jstrInitEdit = jobjStrEdit.CreateWideString( strEditText ) ;
	}
	//
	JNI::JSmartObject	jsobjResult =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidDoMessageEditBox,
				jstrTitle, jstrMsg, jstrInitEdit,
				((nStyles << 8) | msgboxStyleOkCancel) ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	JNI::JString	jstrResult( (jstring) jsobjResult.GetObject() ) ;
	jstrResult.ToString( strEditText ) ;

	if ( jstrInitEdit == (jstring) jsobjResult.GetObject() )
	{
		return	msgboxResultCancel ;
	}
	return	msgboxResultOk ;
#else
	#error	no implement SSystem::MessageEditBox
#endif
}

#endif




//////////////////////////////////////////////////////////////////////////////
// 進行状況ダイアログ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SSystem::SProgressiveDialog, SProgressiveUserInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SProgressiveDialog::SProgressiveDialog( void )
{
#if	defined(__COTOPHA__)
	m_dlg = NULL ;

#elif	defined(__PLATFORM_WINDOWS__)
	m_hDialog = NULL ;
	m_hParentWnd = NULL ;
	m_flagCanceled = false ;

#endif

	m_nCreationFlags = 0 ;
	m_pParentWnd = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SProgressiveDialog::~SProgressiveDialog( void )
{
	SProgressiveDialog::Close() ;
}

// ダイアログ作成
//////////////////////////////////////////////////////////////////////////////
SError SProgressiveDialog::Create
	( uint64_t nFlags, SakuraGL::SGLAbstractWindow * pParentWnd )
{
#if	defined(__COTOPHA__)
	Window *	pParentWndObj = NULL ;
	if ( m_dlg == NULL )
	{
		m_dlg = new ProgressiveDialog ;
	}
	if ( pParentWnd != NULL )
	{
		pParentWndObj = pParentWnd->GetWindowObject() ;
	}
	return	m_dlg->Create( nFlags, pParentWndObj ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog != NULL )
	{
		return	errFailed ;
	}
	m_hParentWnd = NULL ;
	if ( pParentWnd != NULL )
	{
		m_hParentWnd = pParentWnd->GetWindowHandle() ;
	}
	m_flagCanceled = false ;
	m_signalCreated.Initialize( false ) ;
	if ( m_threadUI.BeginThread( this ) )
	{
		return	errFailed ;
	}
	m_signalCreated.Wait() ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() != NULL )
	{
		return	errFailed ;
	}
	if ( m_jobjDialog.CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/UIProgressDialog" ) == NULL )
	{
		return	errFailed ;
	}
	if ( nFlags & flagStyleSpinner )
	{
		jmethodID	jmidSetStyleSpinner =
			m_jobjDialog.GetMethodID( "setStyleSplinner", "()V" ) ;
		m_jobjDialog.CallVoidMethod( jmidSetStyleSpinner ) ;
	}
	m_jmidSetTitle =
		m_jobjDialog.GetMethodID
			( "setTitle", "(L" JAVA_LANG_STRING ";)V" ) ;
	m_jmidSetMessage =
		m_jobjDialog.GetMethodID
			( "setMessage", "(L" JAVA_LANG_STRING ";)V" ) ;
	m_jmidCloseDialog =
		m_jobjDialog.GetMethodID( "closeDialog", "()V" ) ;
	m_jmidIsCanceled =
		m_jobjDialog.GetMethodID( "isCanceled", "()Z" ) ;
	m_jmidSetProgress =
		m_jobjDialog.GetMethodID( "setProgress", "(II)V" ) ;
	//
	if ( !m_strCaption.IsEmpty() )
	{
		SetCaption( m_strCaption ) ;
		m_strCaption.FreeArray() ;
	}
	if ( !m_strMessage.IsEmpty() )
	{
		SetMessage( m_strMessage ) ;
		m_strMessage.FreeArray() ;
	}
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidProcedureOnUIThread =
		jsclsEntisGLS.GetStaticMethodID
			( "procedureOnUIThread", "(L" JAVA_LANG_RUNNABLE ";)Z" ) ;
	ESLAssert( jmidProcedureOnUIThread != NULL ) ;
	if ( jmidProcedureOnUIThread == NULL )
	{
		return	errFailed ;
	}
	if ( !jsclsEntisGLS.CallStaticBooleanMethod
		( jmidProcedureOnUIThread, m_jobjDialog.GetObject() ) )
	{
		return	errFailed ;
	}
	m_jobjDialog.MakeGlobalRef() ;
	return	errSuccess ;

#endif
}

// ダイアログ消去
//////////////////////////////////////////////////////////////////////////////
SError SProgressiveDialog::Close( void )
{
#if	defined(__COTOPHA__)
	if ( m_dlg != NULL )
	{
		m_dlg->Close() ;
		delete	m_dlg ;
		m_dlg = NULL ;
	}
	return	errSuccess ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog != NULL )
	{
		::EndDialog( m_hDialog, IDOK ) ;
		m_threadUI.Wait() ;
		m_threadUI.Delete() ;
		m_signalCreated.Delete() ;
		m_hDialog = NULL ;
	}
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() == NULL )
	{
		return	errFailed ;
	}
	m_jobjDialog.CallVoidMethod( m_jmidCloseDialog ) ;
	m_jobjDialog.DetachJavaObject() ;
	return	errSuccess ;

#endif
}

// キャプション設定
//////////////////////////////////////////////////////////////////////////////
SError SProgressiveDialog::SetCaption( const wchar_t * pwszCaption )
{
#if	defined(__COTOPHA__)
	if ( m_dlg == NULL )
	{
		m_dlg = new ProgressiveDialog ;
	}
	return	m_dlg->SetCaption( pwszCaption ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog == NULL )
	{
		m_strCaption = pwszCaption ;
		return	errSuccess ;
	}
	SString	strCaption = pwszCaption ;
	::SetWindowText( m_hDialog, strCaption.ToCharArray() ) ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() == NULL )
	{
		m_strCaption = pwszCaption ;
		return	errSuccess ;
	}
	JNI::JavaObject	jstrCaption ;
	m_jobjDialog.CallVoidMethod
		( m_jmidSetTitle, jstrCaption.CreateWideString( pwszCaption ) ) ;
	return	errSuccess ;

#endif
}

// メッセージ設定
//////////////////////////////////////////////////////////////////////////////
SError SProgressiveDialog::SetMessage( const wchar_t * pwszMessage )
{
#if	defined(__COTOPHA__)
	if ( m_dlg == NULL )
	{
		m_dlg = new ProgressiveDialog ;
	}
	return	m_dlg->SetMessage( pwszMessage ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog == NULL )
	{
		m_strMessage = pwszMessage ;
		return	errSuccess ;
	}
	SString	strMessage = pwszMessage ;
	::SetDlgItemText
		( m_hDialog, IDC_STATIC_MESSAGE, strMessage.ToCharArray() ) ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() == NULL )
	{
		m_strMessage = pwszMessage ;
		return	errSuccess ;
	}
	JNI::JavaObject	jstrMessage ;
	m_jobjDialog.CallVoidMethod
		( m_jmidSetMessage, jstrMessage.CreateWideString( pwszMessage ) ) ;
	return	errSuccess ;

#endif
}

// 進捗状況設定
//////////////////////////////////////////////////////////////////////////////
SError SProgressiveDialog::SetStatus( uint32_t nCurrent, uint32_t nTotal )
{
#if	defined(__COTOPHA__)
	if ( m_dlg == NULL )
	{
		m_dlg = new ProgressiveDialog ;
	}
	return	m_dlg->SetStatus( nCurrent, nTotal ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog == NULL )
	{
		return	errFailed ;
	}
	HWND	hProgress = ::GetDlgItem( m_hDialog, IDC_PROGRESS ) ;
	while ( nTotal >= 0x8000 )
	{
		nTotal >>= 1 ;
		nCurrent >>= 1 ;
	}
	::SendMessage( hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,nTotal) ) ;
	::SendMessage( hProgress, PBM_SETPOS, nCurrent, 0 ) ;
	return	errSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() == NULL )
	{
		return	errFailed ;
	}
	if ( nTotal >= 0x80000000 )
	{
		nTotal >>= 1 ;
		nCurrent >>= 1 ;
	}
	JNI::JavaObject	jstrCaption ;
	m_jobjDialog.CallVoidMethod
		( m_jmidSetProgress, (jint) nCurrent, (jint) nTotal ) ;
	return	errSuccess ;

#endif
}

// キャンセルが押されたか？
//////////////////////////////////////////////////////////////////////////////
bool SProgressiveDialog::IsCanceled( void )
{
#if	defined(__COTOPHA__)
	if ( m_dlg == NULL )
	{
		return	true ;
	}
	return	m_dlg->IsCanceled() ;

#elif	defined(__PLATFORM_WINDOWS__)
	return	m_flagCanceled ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjDialog.GetObject() == NULL )
	{
		return	true ;
	}
	return	m_jobjDialog.CallBooleanMethod( m_jmidIsCanceled ) ;

#endif
}

// CreateProgressiveDialog での生成パラメータ
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::SetCreationParam
	( uint64_t nFlags, SakuraGL::SGLAbstractWindow * pParentWnd )
{
	m_nCreationFlags = nFlags ;
	m_pParentWnd = pParentWnd ;
}

// 進行状況ダイアログ表示
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::CreateProgressiveDialog( void )
{
	Create( m_nCreationFlags, m_pParentWnd ) ;
}

// 進行状況ダイアログ消去
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::CloseProgressiveDialog( void )
{
	Close() ;
}

// 進行状況ダイアログキャプション設定
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::SetProgressiveCaption( const wchar_t * pwszCaption )
{
	SetCaption( pwszCaption ) ;
}

// 進行状況ダイアログメッセージ設定
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::SetProgressiveMessage( const wchar_t * pwszMessage )
{
	SetMessage( pwszMessage ) ;
}

// 進行状況ダイアログメッセージ設定
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::SetProgressiveStatus( int nCurrent, int nTotal )
{
	SetStatus( (uint32_t) nCurrent, (uint32_t) nTotal ) ;
}

// ユーザーがキャンセル操作したか？
//////////////////////////////////////////////////////////////////////////////
bool SProgressiveDialog::IsProgressiveCanceled( void )
{
	return	IsCanceled() ;
}

// メッセージボックス表示
//////////////////////////////////////////////////////////////////////////////
int SProgressiveDialog::DoMessageBox
	( const wchar_t * pwszMsg,
		const wchar_t * pwszCaption, int nStyles )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( m_hDialog != NULL )
	{
		return	SSystem::DoMessageBox
					( pwszMsg, pwszCaption, nStyles, m_hDialog ) ;
	}
#endif
	return	SSystem::MessageBox( pwszMsg, pwszCaption, nStyles ) ;
}



#if	defined(__PLATFORM_WINDOWS__)

// Windows 用実装
//////////////////////////////////////////////////////////////////////////////

/*
#define IDD_PROGRESSIVE_DIALOG          130
#define IDC_STATIC_MESSAGE              1000
#define IDC_PROGRESS                    1001

IDD_PROGRESSIVE_DIALOG DIALOG DISCARDABLE  0, 0, 295, 83
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 9, "ＭＳ Ｐゴシック"
BEGIN
    PUSHBUTTON      "キャンセル",IDCANCEL,122,62,50,14
    LTEXT           "",IDC_STATIC_MESSAGE,15,10,265,20
    CONTROL         "Progress1",IDC_PROGRESS,"msctls_progress32",WS_BORDER,
                    10,35,275,10
END
*/
const BYTE	SProgressiveDialog::m_bytProgressiveDlgData[184] =
{
	0xC0, 0x00, 0xC8, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x01, 
	0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x09, 0x00, 0x2D, 0xFF, 0x33, 0xFF, 0x20, 0x00, 
	0x30, 0xFF, 0xB4, 0x30, 0xB7, 0x30, 0xC3, 0x30, 
	0xAF, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x7A, 0x00, 0x3E, 0x00, 
	0x32, 0x00, 0x0E, 0x00, 0x02, 0x00, 0xFF, 0xFF, 
	0x80, 0x00, 0xAD, 0x30, 0xE3, 0x30, 0xF3, 0x30, 
	0xBB, 0x30, 0xEB, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x0F, 0x00, 0x0A, 0x00, 0x09, 0x01, 0x14, 0x00, 
	0xE8, 0x03, 0xFF, 0xFF, 0x82, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x23, 0x00, 
	0x13, 0x01, 0x0A, 0x00, 0xE9, 0x03, 0x6D, 0x00, 
	0x73, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6C, 0x00, 
	0x73, 0x00, 0x5F, 0x00, 0x70, 0x00, 0x72, 0x00, 
	0x6F, 0x00, 0x67, 0x00, 0x72, 0x00, 0x65, 0x00, 
	0x73, 0x00, 0x73, 0x00, 0x33, 0x00, 0x32, 0x00, 
	0x00, 0x00, 0x50, 0x00, 0x72, 0x00, 0x6F, 0x00, 
	0x67, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 
	0x73, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 
} ;

// ダイアログ関数
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SProgressiveDialog::ProgressiveDialogProc
	( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	#if	!defined(DWLP_USER)
		enum	{ DWLP_USER = DWL_USER ; } ;
	#endif
	SProgressiveDialog *	pdlg =
		(SProgressiveDialog*) ::GetWindowLongPtr( hwndDlg, DWLP_USER ) ;
	switch ( uMsg )
	{
	case	WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case	IDCANCEL:
			if ( pdlg != NULL )
			{
				pdlg->m_flagCanceled = true ;
				return	0 ;
			}
			break ;
		}
		break ;

	case	WM_CLOSE:
		if ( pdlg != NULL )
		{
			pdlg->m_flagCanceled = true ;
			return	0 ;
		}
		break ;

	case	WM_INITDIALOG:
		pdlg = (SProgressiveDialog*) lParam ;
		ESLAssert( pdlg != NULL ) ;
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam ) ;
		if ( pdlg != NULL )
		{
			pdlg->m_hDialog = hwndDlg ;
			//
			RECT	rctParent ;
			HWND	hwndParent = ::GetParent( hwndDlg ) ;
			if ( hwndParent != NULL )
			{
				::GetWindowRect( hwndParent, &rctParent ) ;
			}
			else
			{
				rctParent.left = 0 ;
				rctParent.top = 0 ;
				rctParent.right = ::GetSystemMetrics( SM_CXSCREEN ) ;
				rctParent.bottom = ::GetSystemMetrics( SM_CYSCREEN ) ;
			}
			RECT	rctDlg ;
			::GetWindowRect( hwndDlg, &rctDlg ) ;
			::SetWindowPos
				( hwndDlg, NULL,
					((rctParent.right - rctParent.left)
						- (rctDlg.right - rctDlg.left)) / 2 + rctParent.left,
					((rctParent.bottom - rctParent.top)
						- (rctDlg.bottom - rctDlg.top)) / 2 + rctParent.top,
					0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;
			//
			if ( !pdlg->m_strCaption.IsEmpty() )
			{
				::SetWindowText( hwndDlg, pdlg->m_strCaption.ToCharArray() ) ;
			}
			if ( !pdlg->m_strMessage.IsEmpty() )
			{
				::SetDlgItemText
					( hwndDlg, IDC_STATIC_MESSAGE,
						pdlg->m_strMessage.ToCharArray() ) ;
			}
			//
			pdlg->m_signalCreated.SetSignal() ;
		}
		break ;
	}
	return	0 ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SProgressiveDialog::Run( void )
{
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	//
	HWND	hwndParent = m_hParentWnd ;
	if ( hwndParent == NULL )
	{
		QuickLock() ;
		SGLAbstractWindow *	pDefWindow = SGLAbstractWindow::GetDefaultWindow() ;
		if ( pDefWindow != NULL )
		{
			hwndParent = pDefWindow->GetWindowHandle() ;
		}
		QuickUnlock() ;
	}
	::DialogBoxIndirectParam
		( ::GetModuleHandle( NULL ),
			(LPCDLGTEMPLATE) m_bytProgressiveDlgData,
			hwndParent, ProgressiveDialogProc, (LPARAM) this ) ;
	//
	if ( hwndParent != NULL )
	{
		::EnableWindow( hwndParent, TRUE ) ;
	}
	m_signalCreated.SetSignal() ;
}

#endif



//////////////////////////////////////////////////////////////////////////////
// 任意ダイアログ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

// SCustomDialog::Listener
// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SCustomDialog::Listener, ESLObject )

// 初期化処理
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::Listener::OnInitDialog( SCustomDialog& dlg )
{
}

// キャンセル処理
//////////////////////////////////////////////////////////////////////////////
bool SCustomDialog::Listener::OnCancel( SCustomDialog& dlg )
{
	return	false ;
}


// ElementData 構築関数
//////////////////////////////////////////////////////////////////////////////
SCustomDialog::ElementData::ElementData( const SCustomDialog::ElementInfo& ei )
{
	SetElementInfo( ei ) ;
}

SCustomDialog::ElementData::ElementData( const SCustomDialog::ElementData& ed )
{
	SetElementInfo( ed.m_info ) ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::SetElementInfo( const SCustomDialog::ElementInfo& ei )
{
	m_info = ei ;
	m_strID = ei.pwszID ;
	m_strText = ei.pwszText ;
	m_info.pwszID = m_strID ;
	m_info.pwszText = m_strText ;
	//
#if	defined(__PLATFORM_ANDROID__)
	m_jobjItem.CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/UICustomDialogInterface$Item" ) ;
	m_jobjItem.MakeGlobalRef() ;
	//
	JNI::JavaObject	jobjInstance ;
	m_jobjItem.SetObjectField
		( m_jobjItem.GetFieldID
				( "m_bufInstance", "L" JAVA_NIO_BYTEBUFFER ";" ),
			jobjInstance.CreateByteBuffer( this, sizeof(ElementData) ) ) ;
	//
	ToJavaObject() ;
#endif
}

#if	defined(__PLATFORM_ANDROID__)
// Java オブジェクトへ更新
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::ToJavaObject( void )
{
	JNI::JavaObject	jobjStrID ;
	m_jobjItem.SetObjectField
		( m_jobjItem.GetFieldID( "m_strID", "L" JAVA_LANG_STRING ";" ),
			jobjStrID.CreateWideString( m_info.pwszID ) ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_type" ), m_info.type ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_nFlags" ), m_info.nFlags ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_optFlags" ), m_info.optFlags ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_nMinWidth" ), m_info.nMinWidth ) ;
	//
	JNI::JavaObject	jobjStrText ;
	m_jobjItem.SetObjectField
		( m_jobjItem.GetFieldID( "m_strText", "L" JAVA_LANG_STRING ";" ),
			jobjStrText.CreateWideString( m_info.pwszText ) ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_nValue" ), m_info.nValue ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_minRange" ), m_info.minRange ) ;
	//
	m_jobjItem.SetIntField
		( m_jobjItem.GetIntFieldID( "m_maxRange" ), m_info.maxRange ) ;
}

// Java オブジェクトから取得
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::FromJavaObject( void )
{
	JNI::JSmartObject	jsoStrText
		( m_jobjItem.GetObjectField( "m_strText", "L" JAVA_LANG_STRING ";" ) ) ;
	JNI::JString	jstrText( (jstring) jsoStrText.GetObject() ) ;
	jstrText.ToString( m_strText ) ;
	m_info.pwszText = m_strText ;
	m_info.nValue = m_jobjItem.GetIntField( "m_nValue" ) ;
}

// 入力値を取得
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::GetInputValue( void )
{
	jmethodID	jmidGetInputValue =
		m_jobjItem.GetMethodID( "getInputValue", "()V" ) ;
	m_jobjItem.CallVoidMethod( jmidGetInputValue ) ;
	//
	FromJavaObject() ;
}

// 数値設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::SetViewInteger( int nValue )
{
	jmethodID	jmidSetViewInteger =
		m_jobjItem.GetMethodID( "setViewInteger", "(I)V" ) ;
	m_jobjItem.CallVoidMethod( jmidSetViewInteger, nValue ) ;
	//
	FromJavaObject() ;
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::SetViewString( const wchar_t * pwszText )
{
	jmethodID	jmidSetViewString =
		m_jobjItem.GetMethodID
			( "setViewString", "(L" JAVA_LANG_STRING ";)V" ) ;
	JNI::JavaObject	jobjStrText ;
	m_jobjItem.CallVoidMethod
		( jmidSetViewString, jobjStrText.CreateWideString( pwszText ) ) ;
	//
	FromJavaObject() ;
}

// 有効状態設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ElementData::SetEnabled( bool flagEnable )
{
	jmethodID	jmidSetEnabled =
		m_jobjItem.GetMethodID( "setEnabled", "(Z)V" ) ;
	m_jobjItem.CallVoidMethod
		( jmidSetEnabled, (jboolean) flagEnable ) ;
}

#endif


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SCustomDialog, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SCustomDialog::SCustomDialog( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	m_hDialog = NULL ;
	m_hParentWnd = NULL ;
	m_hFont = NULL ;
#endif
#if	!defined(__COTOPHA__)
	m_pListener = NULL ;
	m_flagInModal = false ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SCustomDialog::~SCustomDialog( void )
{
}

// フォーム設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::SetCustomItems
	( const SCustomDialog::ElementInfo * pElements, size_t nCount )
{
	m_elements.RemoveAll() ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_elements.Add( new ElementData( pElements[i] ) ) ;
	}
}

// キャプション設定
//////////////////////////////////////////////////////////////////////////////
SError SCustomDialog::SetCaption( const wchar_t * pwszCaption )
{
	m_strCaption = pwszCaption ;
	return	errSuccess ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::AttachListener( SCustomDialog::Listener * pListener )
{
	m_pListener = pListener ;
}

// 表示
//////////////////////////////////////////////////////////////////////////////
int SCustomDialog::DoModal( uint64_t nFlags, SGLAbstractWindow * pParentWnd )
{
#if	defined(__PLATFORM_WINDOWS__)
	bool	fWindowThread = false ;
	m_hParentWnd = NULL ;
	if ( pParentWnd != NULL )
	{
		m_hParentWnd = pParentWnd->GetWindowHandle() ;
		//
		fWindowThread = (::GetCurrentThreadId()
							== ::GetWindowThreadProcessId
										( m_hParentWnd, NULL )) ;
	}
	m_nResultCode = msgboxResultCancel ;
	if ( fWindowThread )
	{
		m_flagInModal = true ;
		Run() ;
		m_flagInModal = false ;
	}
	else
	{
		if ( m_threadUI.BeginThread( this ) )
		{
			return	msgboxResultCancel ;
		}
		m_flagInModal = true ;
		m_threadUI.Wait() ;
		m_flagInModal = false ;
		m_hDialog = NULL ;
		m_threadUI.Delete() ;
	}
	if ( m_hFont != NULL )
	{
		::DeleteObject( m_hFont ) ;
		m_hFont = NULL ;
	}
	return	(int) m_nResultCode ;

#elif	defined(__PLATFORM_ANDROID__)
	m_jobjDialog.CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/UICustomDialog" ) ;
	m_jobjDialog.MakeGlobalRef() ;
	//
	JNI::JavaObject	jobjInstance ;
	m_jobjDialog.SetObjectField
		( m_jobjDialog.GetFieldID( "m_bufInstance", "L" JAVA_NIO_BYTEBUFFER ";" ),
			jobjInstance.CreateByteBuffer( this, sizeof(SCustomDialog) ) ) ;
	//
	// setCaption( m_strCaption ) ;
	//
	JNI::JavaObject	jobjStrCaption ;
	jmethodID		jmidSetTitle =
						m_jobjDialog.GetMethodID
							( "setTitle", "(L" JAVA_LANG_STRING ";)V" ) ;
	m_jobjDialog.CallVoidMethod
		( jmidSetTitle, jobjStrCaption.CreateWideString( m_strCaption ) ) ;
	//
	// Item[] items = new Item[count] ;
	//
	JNI::JSmartClass	jsclsItem
		( JNI::FindJavaClass
			( ENTIS_GLS4_JAVA_PACKAGE "/UICustomDialogInterface$Item" ) ) ;
	JNI::JavaObject	jobjArrItems ;
	jobjectArray	jarrItems =
		jobjArrItems.CreateObjectArray
			( (jsize) m_elements.GetLength(), jsclsItem.GetObject() ) ;
	//
	JNI::JObjectArray	joaItems( jarrItems ) ;
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		ESLAssert( ped != NULL ) ;
		if ( ped == NULL )
		{
			continue ;
		}
		joaItems.SetAt( (jsize) i, ped->m_jobjItem.GetObject() ) ;
	}
	//
	// setCustomItems( items ) ;
	//
	jmethodID	jmidSetCustomItems =
		m_jobjDialog.GetMethodID
			( "setCustomItems",
				"([L" ENTIS_GLS4_JAVA_PACKAGE "/UICustomDialogInterface$Item" ";)V" ) ;
	m_jobjDialog.CallVoidMethod
			( jmidSetCustomItems, jarrItems ) ;
	//
	// int nResult = doModal()
	//
	m_flagInModal = true ;
	//
	jmethodID	jmidDoModal =
					m_jobjDialog.GetMethodID( "doModal", "()I" ) ;
	int	nResult = m_jobjDialog.CallIntMethod( jmidDoModal ) ;
	//
	GetInputResult() ;
	//
	// closeDialog()
	//
	jmethodID	jmidCloseDialog =
					m_jobjDialog.GetMethodID( "closeDialog", "()V" ) ;
	m_jobjDialog.CallVoidMethod( jmidCloseDialog ) ;
	//
	m_flagInModal = false ;

	return	nResult ;
#endif
}

// 入力結果取得
//////////////////////////////////////////////////////////////////////////////
int SCustomDialog::GetInputIntegerAs( const wchar_t * pwszID ) const
{
	ElementData *	ped = GetItemData( pwszID ) ;
	if ( ped == NULL )
	{
		return	0 ;
	}
	if ( m_flagInModal )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		if ( m_hDialog != NULL )
		{
			GetInputResultAs( *ped ) ;
		}
	#elif	defined(__PLATFORM_ANDROID__)
		ped->GetInputValue() ;
	#else
		#error	no implement
	#endif
	}
	switch ( ped->m_info.type )
	{
	case	itemText:
	case	itemEdit:
	case	itemButton:
	case	itemGroupBox:
		ped->m_info.nValue = (int) ped->m_strText.AsInteger() ;
		break ;
	case	itemCheck:
	case	itemRadio:
		break ;
	case	itemProgress:
	case	itemScroll:
	case	itemDropDownList:
		break ;
	default:
		return	0 ;
	}
	return	ped->m_info.nValue ;
}

const wchar_t * SCustomDialog::GetInputStringAs( const wchar_t * pwszID ) const
{
	ElementData *	ped = GetItemData( pwszID ) ;
	if ( ped == NULL )
	{
		return	NULL ;
	}
	if ( m_flagInModal )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		if ( m_hDialog != NULL )
		{
			GetInputResultAs( *ped ) ;
		}
	#elif	defined(__PLATFORM_ANDROID__)
		ped->GetInputValue() ;
	#else
		#error	no implement
	#endif
	}
	switch ( ped->m_info.type )
	{
	case	itemText:
	case	itemEdit:
	case	itemButton:
		case	itemGroupBox:
		return	ped->m_strText ;
	case	itemCheck:
	case	itemRadio:
		break ;
	case	itemProgress:
	case	itemScroll:
		break ;
	default:
		return	NULL ;
	}
	return	NULL ;
}

// アイテム情報取得
//////////////////////////////////////////////////////////////////////////////
SCustomDialog::ElementData *
	SCustomDialog::GetItemData( const wchar_t * pwszID ) const
{
	const size_t	nCount = m_elements.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( ped != NULL )
		{
			if ( ped->m_strID == pwszID )
			{
				return	ped ;
			}
		}
	}
	return	NULL ;
}

// 数値設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::SetItemIntegerAs
	( const wchar_t * pwszID, int nValue )
{
	ElementData *	ped = GetItemData( pwszID ) ;
	if ( ped == NULL )
	{
		return ;
	}
	if ( m_flagInModal )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		switch ( ped->m_info.type )
		{
		case	itemNull:
			break ;
		case	itemText:
		case	itemEdit:
		case	itemButton:
		case	itemGroupBox:
			ped->m_strText.FromInteger( nValue ) ;
			ped->m_info.pwszText = ped->m_strText ;
			::SetWindowText
				( ped->m_hWndCtrl,
					ped->m_strText.ToCharArray().GetConstArray() ) ;
			break ;
		case	itemCheck:
		case	itemRadio:
			ped->m_info.nValue = nValue ;
			::SendMessage
				( ped->m_hWndCtrl, BM_SETCHECK ,
					(nValue ? BST_CHECKED : BST_UNCHECKED), 0 ) ; 
			break ;
		case	itemProgress:
			ped->m_info.nValue = nValue ;
			::SendMessage
				( ped->m_hWndCtrl, PBM_SETPOS, nValue, 0 ) ;
			break ;
		case	itemScroll:
			ped->m_info.nValue = nValue ;
			::SetScrollPos( ped->m_hWndCtrl, SB_CTL, nValue, TRUE ) ;
			break ;
		case	itemDropDownList:
			ped->m_info.nValue = nValue ;
			::SendMessage
				( ped->m_hWndCtrl, CB_SETCURSEL, (WPARAM) nValue, 0 ) ;
			break ;
		}

	#elif	defined(__PLATFORM_ANDROID__)
		ped->SetViewInteger( nValue ) ;

	#else
		#error	no implement
	#endif
	}
	else
	{
		switch ( ped->m_info.type )
		{
		case	itemNull:
			break ;
		case	itemText:
		case	itemEdit:
		case	itemButton:
		case	itemGroupBox:
			ped->m_strText.FromInteger( nValue ) ;
			ped->m_info.pwszText = ped->m_strText ;
			break ;
		case	itemCheck:
		case	itemRadio:
			ped->m_info.nValue = nValue ;
			break ;
		case	itemProgress:
			ped->m_info.nValue = nValue ;
			break ;
		case	itemScroll:
			ped->m_info.nValue = nValue ;
			break ;
		case	itemDropDownList:
			ped->m_info.nValue = nValue ;
			break ;
		}
	}
}

// テキスト設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::SetItemStringAs
	( const wchar_t * pwszID, const wchar_t * pwszText )
{
	ElementData *	ped = GetItemData( pwszID ) ;
	if ( ped == NULL )
	{
		return ;
	}
	if ( m_flagInModal )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		switch ( ped->m_info.type )
		{
		case	itemText:
		case	itemEdit:
		case	itemButton:
		case	itemCheck:
		case	itemRadio:
		case	itemGroupBox:
			ped->m_strText = pwszText ;
			ped->m_info.pwszText = ped->m_strText ;
			::SetWindowText
				( ped->m_hWndCtrl,
					ped->m_strText.ToCharArray().GetConstArray() ) ;
			break ;
		default:
			break ;
		}

	#elif	defined(__PLATFORM_ANDROID__)
		ped->SetViewString( pwszText ) ;

	#else
		#error	no implement
	#endif
	}
	else
	{
		switch ( ped->m_info.type )
		{
		case	itemText:
		case	itemEdit:
		case	itemButton:
		case	itemCheck:
		case	itemRadio:
		case	itemGroupBox:
			ped->m_strText = pwszText ;
			ped->m_info.pwszText = ped->m_strText ;
			break ;
		default:
			break ;
		}
	}
}

// 有効／禁止状態設定
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::EnableItemAs( const wchar_t * pwszID, bool flagEnable )
{
	ElementData *	ped = GetItemData( pwszID ) ;
	if ( ped == NULL )
	{
		return ;
	}
	if ( m_flagInModal )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		::EnableWindow( ped->m_hWndCtrl, flagEnable ) ;

	#elif	defined(__PLATFORM_ANDROID__)
		ped->SetEnabled( flagEnable ) ;

	#else
		#error	no implement
	#endif
	}
}

// ダイアログ終了
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::EndDialog( int nResultCode )
{
#if	defined(__PLATFORM_WINDOWS__)
	GetInputResult() ;
	::EndDialog( m_hDialog, nResultCode ) ;

#elif	defined(__PLATFORM_ANDROID__)
	GetInputResult() ;
	//
	jmethodID	jmidEndDialog =
		m_jobjDialog.GetMethodID( "endDialog", "(I)V" ) ;
	m_jobjDialog.CallVoidMethod( jmidEndDialog, nResultCode ) ;

#else
	#error	no implement
#endif
}

// メッセージボックス表示
//////////////////////////////////////////////////////////////////////////////
int SCustomDialog::DoMessageBox
	( const wchar_t * pwszMsg,
		const wchar_t * pwszCaption, int nStyles )
{
#if	defined(__PLATFORM_WINDOWS__)
	return	SSystem::DoMessageBox( pwszMsg, pwszCaption, nStyles, m_hDialog ) ;
#else
	return	SSystem::MessageBox( pwszMsg, pwszCaption, nStyles ) ;
#endif
}

// ウィンドウハンドル
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
HWND SCustomDialog::GetWindowHandle( void ) const
{
	return	m_hDialog ;
}
#endif


#if	defined(__PLATFORM_WINDOWS__)

/*
IDD_DUMMY_DIALOG DIALOGEX 0, 0, 324, 113
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
END
*/
const BYTE	SCustomDialog::m_bytCustomDlgData[64] =
{
	0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0xC8, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x01, 
	0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x90, 0x01, 0x00, 0x01, 0x4D, 0x00, 
	0x53, 0x00, 0x20, 0x00, 0x53, 0x00, 0x68, 0x00, 
	0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x20, 0x00, 
	0x44, 0x00, 0x6C, 0x00, 0x67, 0x00, 0x00, 0x00, 
} ;

// ダイアログ関数
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SCustomDialog::CustomDialogProc
	( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	#if	!defined(DWLP_USER)
		enum	{ DWLP_USER = DWL_USER ; } ;
	#endif
	SCustomDialog *	pdlg =
		(SCustomDialog*) ::GetWindowLongPtr( hwndDlg, DWLP_USER ) ;
	switch ( uMsg )
	{
	case	WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case	IDCANCEL:
			if ( pdlg != NULL )
			{
				if ( pdlg->m_pListener != NULL )
				{
					if ( pdlg->m_pListener->OnCancel( *pdlg ) )
					{
						return	0 ;
					}
				}
				pdlg->EndDialog( msgboxResultCancel ) ;
				return	0 ;
			}
			break ;
		default:
			pdlg->OnCommand( LOWORD( wParam ), HIWORD( wParam ) ) ;
			break ;
		}
		break ;

	case	WM_HSCROLL:
		pdlg->OnHScroll
			( LOWORD( wParam ),
				(short int) HIWORD( wParam ), (HWND) lParam ) ;
		break ;

	case	WM_CLOSE:
		if ( pdlg != NULL )
		{
			if ( pdlg->m_pListener != NULL )
			{
				if ( pdlg->m_pListener->OnCancel( *pdlg ) )
				{
					return	0 ;
				}
			}
			pdlg->EndDialog( msgboxResultCancel ) ;
			return	0 ;
		}
		break ;

	case	WM_INITDIALOG:
		pdlg = (SCustomDialog*) lParam ;
		ESLAssert( pdlg != NULL ) ;
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam ) ;
		if ( pdlg->m_hFont == NULL )
		{
			pdlg->m_hFont =
				::CreateFont
					( 14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						DEFAULT_PITCH | FF_DONTCARE, "MS UI Gothic" ) ;
			::SendMessage( hwndDlg, WM_SETFONT, (WPARAM) (pdlg->m_hFont), 0 ) ;
		}
		if ( pdlg != NULL )
		{
			pdlg->m_hDialog = hwndDlg ;
			pdlg->OnInitDialog() ;
		}
		RECT	rctParent ;
		HWND	hwndParent = ::GetParent( hwndDlg ) ;
		if ( hwndParent != NULL )
		{
			::GetWindowRect( hwndParent, &rctParent ) ;
		}
		else
		{
			rctParent.left = 0 ;
			rctParent.top = 0 ;
			rctParent.right = ::GetSystemMetrics( SM_CXSCREEN ) ;
			rctParent.bottom = ::GetSystemMetrics( SM_CYSCREEN ) ;
		}
		RECT	rctDlg ;
		::GetWindowRect( hwndDlg, &rctDlg ) ;
		::SetWindowPos
			( hwndDlg, NULL,
				((rctParent.right - rctParent.left)
					- (rctDlg.right - rctDlg.left)) / 2 + rctParent.left,
				((rctParent.bottom - rctParent.top)
					- (rctDlg.bottom - rctDlg.top)) / 2 + rctParent.top,
				0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;
		break ;
	}
	return	0 ;
}

// カスタムアイテムを生成
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::OnInitDialog( void )
{
	m_nNextCtrlID = 100 ;
	m_flagGroupRadio = false ;
	//
	// アイテム配置
	//
	StructItem	si ;
	LayoutCustomItems( si, 0, m_elements.GetLength(), 16, 16 ) ;
	//
	// ダイアログサイズ調整
	//
	RECT	rectWindow, rectClient ;
	if ( ::GetWindowRect( m_hDialog, &rectWindow )
		&& ::GetClientRect( m_hDialog, &rectClient ) )
	{
		rectWindow.right +=
			si.m_ptElements.x + si.m_sizeElements.cx - rectClient.right ;
		rectWindow.bottom +=
			si.m_ptElements.y + si.m_sizeElements.cy + 8 - rectClient.bottom ;
		//
		::MoveWindow
			( m_hDialog,
				rectWindow.left, rectWindow.top,
				rectWindow.right - rectWindow.left,
				rectWindow.bottom - rectWindow.top, TRUE ) ;
	}
	//
	// キャプション設定
	//
	::SetWindowText( m_hDialog, m_strCaption.ToCharArray().GetConstArray() ) ;
	//
	// コールバック
	//
	if ( m_pListener != NULL )
	{
		m_pListener->OnInitDialog( *this ) ;
	}
}

void SCustomDialog::LayoutCustomItems
	( SCustomDialog::StructItem& si,
		size_t iFirst, size_t iEnd, int xPos, int yPos )
{
	SArray<LineItemsInfo>	aLineInfo ;
	size_t		iLine = 0 ;			// current line first index of si.m_aElements
	int			yNext = yPos ;
	int			widthMax = xPos ;
	const int	xRightGap =16 ;
	for ( size_t i = iFirst; i < iEnd; i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( ped == NULL )
		{
			continue ;
		}
		//
		// アイテム生成
		//
		CreateCustomItem( *ped ) ;
		//
		StructItem *	psi = new StructItem ;
		psi->m_pElement = ped ;
		si.m_aElements.Add( psi ) ;
		//
		if ( ped->m_info.type == itemGroupBox )
		{
			//
			// Group Box 処理
			//
			size_t	iEndOfGroup = iEnd ;
			for ( size_t j = i; j < iEnd; j ++ )
			{
				ElementData *	pedTemp = m_elements.GetAt( j ) ;
				if ( pedTemp && (pedTemp->m_info.nFlags & flagEndOfGroupBox) )
				{
					iEndOfGroup = j + 1 ;
					break ;
				}
			}
			//
			SIZE	sizeGroupBoxHeader = SizeofStructItem( *psi ) ;
			LayoutCustomItems
				( *psi, i + 1, iEndOfGroup,
					sizeGroupBoxHeader.cy, 
					sizeGroupBoxHeader.cy + 4 ) ;
			//
			::MoveWindow
				( ped->m_hWndCtrl, 0, 0,
					sizeGroupBoxHeader.cy * 3 / 2
						+ psi->m_sizeElements.cx - xRightGap,
					sizeGroupBoxHeader.cy
						+ psi->m_sizeElements.cy + 16, TRUE ) ;
			//
			i = iEndOfGroup - 1 ;
		}
		//
		if ( ((i + 1) >= iEnd)
			|| (ped->m_info.nFlags & flagEndOfLine) )
		{
			//
			// 同一列の最大高取得
			//
			int	hMax = 8 ;
			for ( size_t j = iLine; j < si.m_aElements.GetLength(); j ++ )
			{
				StructItem *	psiTemp = si.m_aElements.GetAt( j ) ;
				ESLAssert( psiTemp != NULL ) ;
				SIZE	sizeItem = SizeofStructItem( *psiTemp ) ;
				if ( sizeItem.cy > hMax )
				{
					hMax = (int) sizeItem.cy ;
				}
			}
			//
			// 同一列アイテム配置
			//
			int	xNext = xPos ;
			for ( size_t j = iLine; j < si.m_aElements.GetLength(); j ++ )
			{
				StructItem *	psiTemp = si.m_aElements.GetAt( j ) ;
				ESLAssert( psiTemp != NULL ) ;
				SIZE	sizeItem = SizeofStructItem( *psiTemp ) ;
				OffsetMoveStructItem
					( *psiTemp, xNext, yNext + (hMax - sizeItem.cy) / 2 ) ;
				xNext += sizeItem.cx + 8 ;
			}
			xNext += xRightGap ;
			if ( widthMax < xNext )
			{
				widthMax = xNext ;
			}
			//
			// 行情報追加
			//
			LineItemsInfo	lii ;
			lii.iFirst = iLine ;
			lii.nCount = si.m_aElements.GetLength() - iLine ;
			lii.nWidth = xNext ;
			lii.nFlags = ped->m_info.nFlags ;
			aLineInfo.Add( lii ) ;
			//
			// 次の行へ
			//
			yNext += hMax + 6 ;
			iLine = si.m_aElements.GetLength() ;
		}
	}
	//
	// 垂直整列判定
	//
	for ( size_t i = 0; i < 4; i ++ )
	{
		int	xMaxLeft = 0 ;
		for ( size_t j = 0; j < si.m_aElements.GetLength(); j ++ )
		{
			StructItem *	psi = si.m_aElements.GetAt( j ) ;
			if ( (psi == NULL)
				|| (psi->m_pElement == NULL)
				|| !(psi->m_pElement->m_info.nFlags & (flagArrangeCol1 << i)) )
			{
				continue ;
			}
			POINT	ptItem ;
			if ( GetStructItemPosition( ptItem, *psi ) )
			{
				if ( ptItem.x > xMaxLeft )
				{
					xMaxLeft = (int) ptItem.x ;
				}
			}
		}
		int	xOffset = 0 ;
		for ( size_t j = 0; j < si.m_aElements.GetLength(); j ++ )
		{
			StructItem *	psi = si.m_aElements.GetAt( j ) ;
			if ( psi == NULL )
			{
				continue ;
			}
			if ( (psi->m_pElement != NULL)
				&& (psi->m_pElement->m_info.nFlags & (flagArrangeCol1 << i)) )
			{
				POINT	ptItem ;
				if ( GetStructItemPosition( ptItem, *psi ) )
				{
					xOffset = xMaxLeft - ptItem.x ;
				}
			}
			if ( xOffset )
			{
				OffsetMoveStructItem( *psi, xOffset, 0 ) ;
				//
				POINT	ptItem ;
				if ( GetStructItemPosition( ptItem, *psi ) )
				{
					SIZE	sizeItem = SizeofStructItem( *psi ) ;
					if ( ptItem.x + sizeItem.cx + xRightGap > widthMax )
					{
						widthMax = ptItem.x + sizeItem.cx + xRightGap ;
					}
				}
			}
			if ( (psi->m_pElement != NULL)
				&& (psi->m_pElement->m_info.nFlags & flagEndOfLine) )
			{
				xOffset = 0 ;
			}
		}
	}
	for ( size_t i = 0; i < si.m_aElements.GetLength(); i ++ )
	{
		StructItem *	psi = si.m_aElements.GetAt( i ) ;
		if ( (psi == NULL)
			|| (psi->m_pElement == NULL)
			|| !(psi->m_pElement->m_info.nFlags & flagFullWidth) )
		{
			continue ;
		}
		POINT	ptItem ;
		if ( GetStructItemPosition( ptItem, *psi ) )
		{
			if ( ptItem.x < widthMax - xRightGap )
			{
				SIZE	sizeItem = SizeofStructItem( *psi ) ;
				ResizeStructItem
					( *psi, widthMax - xRightGap - ptItem.x, sizeItem.cy ) ;
			}
		}
	}
	si.m_ptElements.x = xPos ;
	si.m_ptElements.y = yPos ;
	si.m_sizeElements.cx = widthMax - xPos ;
	si.m_sizeElements.cy = yNext - yPos ;
	//
	// 行アライン調整
	//
	for ( size_t i = 0; i < aLineInfo.GetLength(); i ++ )
	{
		LineItemsInfo *	plii = aLineInfo.GetAt( i ) ;
		ESLAssert( plii != NULL ) ;
		int	xOffset = 0 ;
		if ( plii->nFlags & flagLineCenter )
		{
			xOffset = (widthMax - plii->nWidth) / 2 ;
		}
		else if ( plii->nFlags & flagLineRight )
		{
			xOffset = (widthMax - plii->nWidth) ;
		}
		if ( xOffset != 0 )
		{
			for ( size_t j = 0; j < plii->nCount; j ++ )
			{
				StructItem *	psi = si.m_aElements.GetAt( plii->iFirst + j ) ;
				ESLAssert( psi != NULL ) ;
				if ( psi != NULL )
				{
					OffsetMoveStructItem( *psi, xOffset, 0 ) ;
				}
			}
		}
	}
}

void SCustomDialog::CreateCustomItem( SCustomDialog::ElementData& ed )
{
	const char *	pszWndClass = NULL ;
	SGLSize			sizeCtrl ;
	DWORD			dwStyle = WS_CHILD | WS_VISIBLE ;
	DWORD			dwStyleEx = 0 ;
	SArray<char>	bufText ;
	switch ( ed.m_info.type )
	{
	case	itemNull:
		return ;
	case	itemText:
	case	itemButton:
	case	itemCheck:
	case	itemRadio:
	case	itemGroupBox:
		{
			SIZE	sizeText ;
			HDC	hdc = ::GetDC( m_hDialog ) ;
			//
			bufText = ed.m_strText.ToCharArray() ;
			//
			if ( ::GetTextExtentPoint
				( hdc, bufText.GetConstArray(),
						(int) bufText.GetLength(), &sizeText ) )
			{
				POINT	pt ;
				pt.x = sizeText.cx ;
				pt.y = sizeText.cy ;
				//
				::LPtoDP( hdc, &pt, 1 ) ;
				//
				sizeCtrl.w = pt.x ;
				sizeCtrl.h = pt.y ;
			}
			else
			{
				sizeCtrl.w = 200 ;
				sizeCtrl.h = 20 ;
			}
			//
			::DeleteDC( hdc ) ;
			//
			switch ( ed.m_info.type )
			{
			case	itemText:
				sizeCtrl.w += 8 ;
				sizeCtrl.h += 2 ;
				pszWndClass = "STATIC" ;
				dwStyle |= SS_LEFT | SS_CENTERIMAGE ;
				break ;
			case	itemButton:
				sizeCtrl.w += 16 ;
				sizeCtrl.h += 8 ;
				pszWndClass = "BUTTON" ;
				dwStyle |= BS_CENTER | WS_TABSTOP ;
				break ;
			case	itemCheck:
			case	itemRadio:
			case	itemGroupBox:
				sizeCtrl.w += 24 ;
				sizeCtrl.h += 4 ;
				pszWndClass = "BUTTON" ;
				if ( ed.m_info.type == itemCheck )
				{
					dwStyle |= BS_AUTOCHECKBOX | WS_TABSTOP ;
				}
				else if ( ed.m_info.type == itemRadio )
				{
					dwStyle |= BS_AUTORADIOBUTTON | WS_TABSTOP ;
					if ( !m_flagGroupRadio )
					{
						dwStyle |= WS_GROUP ;
					}
					m_flagGroupRadio =
						!(ed.m_info.nFlags & flagEndOfRadio) ;
				}
				else
				{
					dwStyle |= BS_GROUPBOX ;
				}
				break ;
			}
		}
		break ;
	case	itemEdit:
		{
			int			nHeight = 16 ;
			TEXTMETRIC	tm ;
			HDC	hdc = ::GetDC( m_hDialog ) ;
			if ( ::GetTextMetrics( hdc, &tm ) )
			{
				nHeight = (int) tm.tmHeight ;
			}
			::DeleteDC( hdc ) ;
			//
			bufText = ed.m_strText.ToCharArray() ;
			sizeCtrl.w = 64 ;
			sizeCtrl.h = nHeight + 4 ;
			pszWndClass = "EDIT" ;
			if ( ed.m_info.optFlags & editboxStyleMultiLine )
			{
				sizeCtrl.h += nHeight * 2 ;
				dwStyle |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL ;
			}
			if ( ed.m_info.optFlags & editboxStyleNumber )
			{
				dwStyle |= ES_NUMBER ;
			}
			if ( ed.m_info.optFlags & editboxStylePassword )
			{
				dwStyle |= ES_PASSWORD ;
			}
			dwStyle |= ES_AUTOHSCROLL | WS_TABSTOP ;
			dwStyleEx |= WS_EX_CLIENTEDGE ;
		}
		break ;
	case	itemProgress:
		sizeCtrl.w = 100 ;
		sizeCtrl.h = 16 ;
		pszWndClass = PROGRESS_CLASS ;
		break ;
	case	itemScroll:
		sizeCtrl.w = 100 ;
		sizeCtrl.h = 16 ;
		pszWndClass = "SCROLLBAR" ;
		dwStyle |= SBS_HORZ ;
		break ;
	case	itemDropDownList:
		sizeCtrl.w = 64 ;
		sizeCtrl.h = 256 ;
		pszWndClass = "COMBOBOX" ;
		dwStyle |= CBS_DROPDOWNLIST  ;
		break ;
		break ;
	default:
		return ;
	}
	bufText.Add( 0 ) ;
	//
	if ( (ed.m_info.nFlags & flagMinWidth)
		&& ((uint32_t) sizeCtrl.w < ed.m_info.nMinWidth) )
	{
		sizeCtrl.w = ed.m_info.nMinWidth ;
	}
	ed.m_nCtrlID = m_nNextCtrlID ++ ;
	ed.m_hWndCtrl =
		CreateWindowEx
			( dwStyleEx, pszWndClass,
				bufText.GetConstArray(), dwStyle,
				0, 0, sizeCtrl.w, sizeCtrl.h,
				m_hDialog, (HMENU) ((uint_ptr_t) ed.m_nCtrlID),
				GetModuleHandle( NULL ), NULL ) ;
	//
	if ( m_hFont && ed.m_hWndCtrl )
	{
		::SendMessage
			( ed.m_hWndCtrl, WM_SETFONT,
				(WPARAM) m_hFont, MAKELPARAM(TRUE,0) ) ;
	}
	//
	const wchar_t *	pwszText ;
	switch ( ed.m_info.type )
	{
	case	itemCheck:
	case	itemRadio:
		::SendMessage
			( ed.m_hWndCtrl, BM_SETCHECK,
				(ed.m_info.nValue ? BST_CHECKED : BST_UNCHECKED), 0 ) ; 
		break ;
	case	itemProgress:
		::SendMessage
			( ed.m_hWndCtrl, PBM_SETRANGE,
				0, MAKELPARAM(ed.m_info.minRange, ed.m_info.maxRange) ) ;
		::SendMessage
			( ed.m_hWndCtrl, PBM_SETPOS, ed.m_info.nValue, 0 ) ;
		break ;
	case	itemScroll:
		::SetScrollRange
			( ed.m_hWndCtrl, SB_CTL,
				ed.m_info.minRange, ed.m_info.maxRange, FALSE ) ;
		::SetScrollPos
			( ed.m_hWndCtrl, SB_CTL, ed.m_info.nValue, TRUE ) ;
		break ;
	case	itemDropDownList:
		pwszText = ed.m_info.pwszText ;
		while ( pwszText[0] )
		{
			ssize_t	nLength = 0 ;
			while ( pwszText[nLength] && (pwszText[nLength] != L'\n') )
			{
				nLength ++ ;
			}
			SString			strText( pwszText, nLength ) ;
			SArray<char>	bufText ;
			::SendMessage
				( ed.m_hWndCtrl, CB_ADDSTRING ,
					0, (LPARAM) strText.EncodeDefaultTo( bufText ) ) ;
			pwszText += nLength ;
			if ( *pwszText == L'\n' )
			{
				pwszText ++ ;
			}
		}
		::SendMessage
			( ed.m_hWndCtrl, CB_SETCURSEL, (WPARAM) ed.m_info.nValue, 0 ) ;
		break ;
	}
}

// アイテムのサイズ取得
//////////////////////////////////////////////////////////////////////////////
SIZE SCustomDialog::SizeofStructItem
	( const SCustomDialog::StructItem& si ) const
{
	if ( si.m_pElement != NULL )
	{
		if ( si.m_pElement->m_hWndCtrl != NULL )
		{
			RECT	rect ;
			if ( ::GetWindowRect( si.m_pElement->m_hWndCtrl, &rect ) )
			{
				SIZE	sizeItem =
					{ rect.right - rect.left, rect.bottom - rect.top } ;
				return	sizeItem ;
			}
		}
	}
	SIZE	sizeItem = { 0, 0 } ;
	return	sizeItem ;
}

// アイテムの位置取得
//////////////////////////////////////////////////////////////////////////////
bool SCustomDialog::GetStructItemPosition
	( POINT& ptItem, const SCustomDialog::StructItem& si ) const
{
	if ( si.m_pElement != NULL )
	{
		if ( si.m_pElement->m_hWndCtrl != NULL )
		{
			RECT	rect ;
			if ( ::GetWindowRect( si.m_pElement->m_hWndCtrl, &rect ) )
			{
				POINT	ptWindow = { rect.left, rect.top } ;
				::ScreenToClient( m_hDialog, &ptWindow ) ;
				//
				ptItem.x = ptWindow.x ;
				ptItem.y = ptWindow.y ;
				return	true ;
			}
		}
	}
	return	false ;
}

// アイテム移動
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::OffsetMoveStructItem
	( SCustomDialog::StructItem& si, int xOffset, int yOffset ) const
{
	if ( si.m_pElement != NULL )
	{
		if ( si.m_pElement->m_hWndCtrl != NULL )
		{
			RECT	rect ;
			if ( ::GetWindowRect( si.m_pElement->m_hWndCtrl, &rect ) )
			{
				POINT	ptWindow = { rect.left, rect.top } ;
				::ScreenToClient( m_hDialog, &ptWindow ) ;
				//
				::SetWindowPos
					( si.m_pElement->m_hWndCtrl, NULL,
						ptWindow.x + xOffset,
						ptWindow.y + yOffset, 0, 0,
						(SWP_NOSIZE | SWP_NOZORDER) ) ;
			}
		}
	}
	for ( size_t i = 0; i < si.m_aElements.GetLength(); i ++ )
	{
		StructItem *	psi = si.m_aElements.GetAt( i ) ;
		ESLAssert( psi != NULL ) ;
		OffsetMoveStructItem( *psi, xOffset, yOffset ) ;
	}
	si.m_ptElements.x += xOffset ;
	si.m_ptElements.y += yOffset ;
}

// アイテムサイズ変更
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::ResizeStructItem
	( SCustomDialog::StructItem& si, int nWidth, int nHeight ) const
{
	if ( si.m_pElement != NULL )
	{
		if ( si.m_pElement->m_hWndCtrl != NULL )
		{
			::SetWindowPos
				( si.m_pElement->m_hWndCtrl, NULL,
					0, 0, nWidth, nHeight,
					(SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED) ) ;
		}
	}
}

// コマンドメッセージ処理
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::OnCommand( WORD wID, WORD wNotifyCode )
{
	ElementData	*	pedItem = NULL ;
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( (ped != NULL) && (ped->m_nCtrlID == wID) )
		{
			pedItem = ped ;
			break ;
		}
	}
	if ( pedItem == NULL )
	{
		return ;
	}
	int	code ;
	switch ( pedItem->m_info.type )
	{
	case	itemEdit:
		if ( wNotifyCode == EN_KILLFOCUS )
		{
			code = codeOnChanged ;
		}
		else
		{
			return ;
		}
		break ;
	case	itemButton:
		code = codeOnPushed ;
		break ;
	case	itemCheck:
	case	itemRadio:
		code = codeOnChanged ;
		break ;
	case	itemDropDownList:
		if ( wNotifyCode == CBN_SELCHANGE )
		{
			code = codeOnChanged ;
		}
		else
		{
			return ;
		}
		break ;
	default:
		return ;
	}
	if ( pedItem->m_info.pfnCallback != NULL )
	{
		GetInputResultAs( *pedItem ) ;
		if ( (pedItem->m_info.pfnCallback)
			( *this, pedItem->m_info, code, pedItem->m_info.pInstance ) )
		{
			return ;
		}
	}
	if ( pedItem->m_info.type == itemButton )
	{
		switch ( pedItem->m_info.nFlags & flagMaskTypeButton )
		{
		case	flagPositiveButton:
			EndDialog( msgboxResultOk ) ;
			break ;
		case	flagNegativeButton:
			EndDialog( msgboxResultCancel ) ;
			break ;
		case	flagNeutralButton:
			EndDialog( msgboxResultUser ) ;
			break ;
		}
	}
}

// スクロールメッセージ処理
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::OnHScroll( WORD wSBCode, short int wPos, HWND hwndScroll )
{
	ElementData	*	pedItem = NULL ;
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( (ped != NULL) && (ped->m_hWndCtrl == hwndScroll) )
		{
			pedItem = ped ;
			break ;
		}
	}
	if ( pedItem == NULL )
	{
		return ;
	}
	switch ( wSBCode )
	{
	case	SB_LEFT:
		pedItem->m_info.nValue = pedItem->m_info.minRange ;
		break ;
	case	SB_RIGHT:
		pedItem->m_info.nValue = pedItem->m_info.maxRange ;
		break ;
	case	SB_LINELEFT:
		pedItem->m_info.nValue -- ;
		break ;
	case	SB_LINERIGHT:
		pedItem->m_info.nValue ++ ;
		break ;
	case	SB_PAGELEFT:
		pedItem->m_info.nValue -=
			(pedItem->m_info.maxRange - pedItem->m_info.minRange) >> 4 ;
		break ;
	case	SB_PAGERIGHT:
		pedItem->m_info.nValue +=
			(pedItem->m_info.maxRange - pedItem->m_info.minRange) >> 4 ;
		break ;
	case	SB_THUMBPOSITION:
		pedItem->m_info.nValue = wPos ;
		break ;
	}
	if ( pedItem->m_info.nValue < pedItem->m_info.minRange )
	{
		pedItem->m_info.nValue = pedItem->m_info.minRange ;
	}
	else if ( pedItem->m_info.nValue > pedItem->m_info.maxRange )
	{
		pedItem->m_info.nValue = pedItem->m_info.maxRange ;
	}
	::SetScrollPos
		( pedItem->m_hWndCtrl, SB_CTL, pedItem->m_info.nValue, TRUE ) ;
	//
	if ( pedItem->m_info.pfnCallback != NULL )
	{
		if ( (pedItem->m_info.pfnCallback)
			( *this, pedItem->m_info, codeOnChanged, pedItem->m_info.pInstance ) )
		{
			return ;
		}
	}
}

// 入力結果を取得する
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::GetInputResult( void ) const
{
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( ped != NULL )
		{
			GetInputResultAs( *ped ) ;
		}
	}
}

void SCustomDialog::GetInputResultAs( SCustomDialog::ElementData& ed ) const
{
	if ( (ed.m_hWndCtrl == NULL) || !::IsWindow( ed.m_hWndCtrl ) )
	{
		return ;
	}
	SArray<char>	bufText ;
	int				nTextLen ;
	switch ( ed.m_info.type )
	{
	case	itemEdit:
		nTextLen = ::GetWindowTextLength( ed.m_hWndCtrl ) ;
		::GetWindowText
			( ed.m_hWndCtrl,
				bufText.GetArray( nTextLen + 0x100 ), nTextLen + 1 ) ;
		bufText.FinishArray() ;
		ed.m_strText = bufText.GetConstArray() ;
		ed.m_info.pwszText = ed.m_strText ;
		break ;
	case	itemCheck:
	case	itemRadio:
		if ( ::SendMessage
				( ed.m_hWndCtrl, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		{
			ed.m_info.nValue = 1 ;
		}
		else
		{
			ed.m_info.nValue = 0 ;
		}
		break ;
	case	itemScroll:
		break ;
	case	itemDropDownList:
		ed.m_info.nValue =
			(int) ::SendMessage( ed.m_hWndCtrl, CB_GETCURSEL , 0, 0 ) ;
		break ;
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::Run( void )
{
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	//
	HWND	hwndParent = m_hParentWnd ;
	if ( hwndParent == NULL )
	{
		QuickLock() ;
		SGLAbstractWindow *	pDefWindow = SGLAbstractWindow::GetDefaultWindow() ;
		if ( pDefWindow != NULL )
		{
			hwndParent = pDefWindow->GetWindowHandle() ;
		}
		QuickUnlock() ;
	}
	m_nResultCode = ::DialogBoxIndirectParam
		( ::GetModuleHandle( NULL ),
			(LPCDLGTEMPLATE) m_bytCustomDlgData,
			hwndParent, CustomDialogProc, (LPARAM) this ) ;
	m_hDialog = NULL ;
	//
	if ( hwndParent != NULL )
	{
		::EnableWindow( hwndParent, TRUE ) ;
	}
}


#elif	defined(__PLATFORM_ANDROID__)

// 入力結果を取得する
//////////////////////////////////////////////////////////////////////////////
void SCustomDialog::GetInputResult( void ) const
{
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ElementData *	ped = m_elements.GetAt( i ) ;
		if ( ped != NULL )
		{
			ped->GetInputValue() ;
		}
	}
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
bool SCustomDialog::OnCallbackItem
		( SCustomDialog::ElementData * ped, int code )
{
	if ( code == codeOnCanceled )
	{
		if ( m_pListener != NULL )
		{
			return	m_pListener->OnCancel( *this ) ;
		}
		return	false ;
	}
	if ( ped->m_info.pfnCallback == NULL )
	{
		return	false ;
	}
	return	(ped->m_info.pfnCallback)
				( *this, ped->m_info, code, ped->m_info.pInstance ) ;
}

#endif


#endif

