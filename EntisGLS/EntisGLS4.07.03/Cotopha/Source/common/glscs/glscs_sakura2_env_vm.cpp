
/*****************************************************************************
						詞葉環境・Sakura2 仮想マシン
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl2d/sgl_bitmap_font.h>
#include <sakuragl/sgl_opengl_context.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_direct_sound_player.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_font.h>

#endif

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// Sakura2 環境＆仮想マシン・プラグインインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::EnvironmentVM::PluginInterface, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::PluginInterface::PluginInterface
	( EnvironmentVM * vm,
		EnvironmentVM::PluginEntry * entry, const wchar_t * pwszArg )
{
	m_entry = entry ;
	//
	if ( entry != NULL )
	{
		(*(m_entry->pfnStartup))( vm, pwszArg ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::PluginInterface::~PluginInterface( void )
{
	if ( m_entry != NULL )
	{
		(*(m_entry->pfnShutdown))() ;
	}
}

// エクスポート関数取得
//////////////////////////////////////////////////////////////////////////////
void * EnvironmentVM::PluginInterface::GetModuleExportFunction
										( const char * pszFuncName )
{
	return	NULL ;
}

// モジュール通知
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::PluginInterface::OnModuleAttached
	( EnvironmentVM * vm, int iModule, ExecutableModule * module )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnModuleAttached != NULL) )
	{
		(*(m_entry->pfnOnModuleAttached))( vm, iModule, module ) ;
	}
}

void EnvironmentVM::PluginInterface::OnModuleDetached
	( EnvironmentVM * vm, int iModule, ExecutableModule * module )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnModuleDetached != NULL) )
	{
		(*(m_entry->pfnOnModuleDetached))( vm, iModule, module ) ;
	}
}

// スレッド通知
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::PluginInterface::OnThreadAttached
	( EnvironmentVM * vm, ThreadObject * thread )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnThreadAttached != NULL) )
	{
		(*(m_entry->pfnOnThreadAttached))( vm, thread ) ;
	}
}

void EnvironmentVM::PluginInterface::OnThreadDetached
	( EnvironmentVM * vm, ThreadObject * thread )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnThreadDetached != NULL) )
	{
		(*(m_entry->pfnOnThreadDetached))( vm, thread ) ;
	}
}

// デバッグ用例外エラー処理
//////////////////////////////////////////////////////////////////////////////
DWORD EnvironmentVM::PluginInterface::OnExceptionEscape
	( EnvironmentVM * vm, Context * context, DWORD maskException )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnExceptionEscape != NULL) )
	{
		return	(*(m_entry->pfnOnExceptionEscape))( vm, context, maskException ) ;
	}
	return	maskException ;
}

// 処理されない例外エラー処理
//////////////////////////////////////////////////////////////////////////////
bool EnvironmentVM::PluginInterface::OnHandleExceptionError
	( EnvironmentVM * vm, Context * context, const wchar_t * pwszErr )
{
	if ( (m_entry != NULL) && (m_entry->pfnOnHandleExceptionError != NULL) )
	{
		return	(*(m_entry->pfnOnHandleExceptionError))( vm, context, pwszErr ) ;
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// Sakura2 環境＆仮想マシン・プラグイン (Windows)
//////////////////////////////////////////////////////////////////////////////

#if	defined(__PLATFORM_WINDOWS__)

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::EnvironmentVM::PluginObject, PluginInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::PluginObject::PluginObject
		( HMODULE hModule, EnvironmentVM * vm,
			EnvironmentVM::PluginEntry * entry, const wchar_t * pwszArg )
	: PluginInterface( vm, entry, pwszArg )
{
	m_hModule = hModule ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::PluginObject::~PluginObject( void )
{
	if ( m_entry != NULL )
	{
		(*(m_entry->pfnShutdown))() ;
		m_entry = NULL ;
	}
	if ( m_hModule != NULL )
	{
		::FreeLibrary( m_hModule ) ;
		m_hModule = NULL ;
	}
}

// エクスポート関数取得
//////////////////////////////////////////////////////////////////////////////
void * EnvironmentVM::PluginObject::GetModuleExportFunction
									( const char * pszFuncName )
{
	if ( m_hModule != NULL )
	{
		return	::GetProcAddress( m_hModule, pszFuncName ) ;
	}
	return	NULL ;
}

// プラグイン・ロード
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::PluginObject *
	EnvironmentVM::PluginObject::LoadPlugin
		( EnvironmentVM * vm, const char * pszDLL, const wchar_t * pwszArg )
{
	HMODULE	hModule = ::LoadLibrary( pszDLL ) ;
	if ( hModule != NULL )
	{
		SVM_PLUGIN_ENTRYPOINT	pfnEntryPoint =
			(SVM_PLUGIN_ENTRYPOINT)
				::GetProcAddress( hModule, "_SVM_PLUGIN_ENTRYPOINT@4" ) ;
		if ( pfnEntryPoint == NULL )
		{
			pfnEntryPoint =
				(SVM_PLUGIN_ENTRYPOINT)
					::GetProcAddress( hModule, "SVM_PLUGIN_ENTRYPOINT" ) ;
		}
		PluginEntry *	entry = NULL ;
		if ( pfnEntryPoint != NULL )
		{
			entry = pfnEntryPoint( numVersion1 ) ;
		}
		return	new PluginObject( hModule, vm, entry, pwszArg ) ;
	}
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// Sakura2 環境＆仮想マシン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( ECSSakura2::EnvironmentVM, StandardVM, SEnvironment, SParserErrorInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::EnvironmentVM( void )
{
	m_statusLoaded = loadedNothing ;
	m_plibShaderBinary = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EnvironmentVM::~EnvironmentVM( void )
{
	if ( SEnvironmentInterface::GetInstance() == (SEnvironmentInterface*) this )
	{
		SEnvironmentInterface::AttachInstance( NULL ) ;
	}
	if ( S3DShaderBinaryLibrary::GetInstance() == m_plibShaderBinary )
	{
		S3DShaderBinaryLibrary::SetInstance( NULL ) ;
	}
	delete	m_plibShaderBinary ;
	//
	EnvironmentVM::UnloadPrimaryModule() ;
}

// 環境初期設定
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::InitEnvironment( void )
{
	if ( SEnvironmentInterface::GetInstance() == NULL )
	{
		SEnvironmentInterface::AttachInstance( this ) ;
	}
	AttachEnvironment( this ) ;
	//
	RegisterDefaultEnvironmentString() ;
	return	errSuccess ;
}

// 環境読み込み
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::LoadEnvironment
	( SFileInterface& file, SSystem::SProgressiveUserInterface * pUI )
{
	m_strErrMsg.FreeArray() ;
	//
	// 環境変数初期設定
	//
	InitEnvironment() ;
	//
	// 環境設定ファイル読み込み・解釈
	//
	SError	err = SEnvironment::ReadDocument( file, *this ) ;
	if ( err )
	{
		m_strErrMsg = L"仮想マシン環境の読み込みに失敗しました" ;
		return	err ;
	}
	if ( m_flagAutoCheckUpdate
		&& (SEnvironment::DoCheckAppUpdate( pUI ) == errAbort) )
	{
		m_strErrMsg.FreeArray() ;
		return	errAbort ;
	}
	if ( SEnvironment::AreDownloadFiles() )
	{
		bool	fNoConfirm = false ;
		SString	strNoConfirm ;
		if ( GetEnvironmentString( strNoConfirm, L"script\\online\\no_confirm" )
			|| GetEnvironmentString( strNoConfirm, L"cotopha\\online\\no_confirm" ) )
		{
			fNoConfirm = (strNoConfirm == L"true") ;
		}
		err = SEnvironment::DoDownloadFiles( fNoConfirm, pUI ) ;
		if ( err )
		{
			m_strErrMsg.FreeArray() ;
			return	err ;
		}
	}
	err = SEnvironment::DoCheckRequirement() ;
	if ( err )
	{
		m_strErrMsg.FreeArray() ;
		return	err ;
	}
	//
	// 仮想マシン初期化
	//
	InitializeVM() ;
	//
	m_statusLoaded = loadedEnvironment ;
	return	errSuccess ;
}

// プライマリ詞葉モジュール読み込み
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::LoadPrimaryModule( SFileInterface* file )
{
	if ( m_statusLoaded != loadedEnvironment )
	{
		return	errFailed ;
	}
	//
	// モジュールファイルを開く
	//
	SSmartPointer<SFileInterface>	pTempFile ;
	if ( file == NULL )
	{
		SString	strSrcScript ;
		if ( !GetEnvironmentString( strSrcScript, L"script\\src" )
			&& !GetEnvironmentString( strSrcScript, L"cotopha\\src" ) )
		{
			m_strErrMsg = L"モジュールの読み込みに失敗しました" ;
			return	errFailed ;
		}
		file = StandardVM::NewOpenFile
			( strSrcScript, SFileOpener::shareRead ) ;
		if ( file == NULL )
		{
			m_strErrMsg = L"モジュールの読み込みに失敗しました" ;
			return	errFailed ;
		}
		pTempFile = file ;
	}
	//
	// モジュールを読み込む
	//
	SError	err = m_module.ReadModule( file ) ;
	if ( err )
	{
		m_strErrMsg = L"モジュールの読み込みに失敗しました" ;
		return	err ;
	}
	//
	// 仮想環境にロード
	//
	const wchar_t *	pwszErr =
				LoadModuleByPrologueOnSysThread( &m_module ) ;
	if ( pwszErr != NULL )
	{
		SArray<char>	strTemp ;
		SString			strErr = pwszErr ;
		Trace( "%s\n", strErr.EncodeDefaultTo( strTemp ) ) ;
		//
		FreeModuleAllocation( &m_module ) ;
		m_strErrMsg = L"モジュールの初期化に失敗しました" ;
		return	errFailed ;
	}
	//
	// JIT コンパイル
	//
	if ( m_flagJITCompiler )
	{
		m_module.CompileToNativeCode
			( !m_flagJITBoundary, m_maskCpuFeatures ) ;
	}
	m_statusLoaded = loadedModule ;
	return	errSuccess ;
}

// プライマリ詞葉モジュールをアンロード
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::UnloadPrimaryModule( void )
{
	if ( m_statusLoaded >= loadedModule )
	{
		UnloadModuleByEpilogueOnSysThread( &m_module ) ;
		m_module.DeleteModule() ;
		m_statusLoaded = loadedEnvironment ;
	}
	return	errSuccess ;
}

// プライマリスレッド実行
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::Run( const wchar_t * pwszArg )
{
	RunStaticInitialize() ;
	return	RunMain( pwszArg ) ;
}

// StaticInitialize 関数実行
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::RunStaticInitialize( void )
{
	if ( m_statusLoaded != loadedModule )
	{
		return	errFailed ;
	}
	ThreadObject *	pThread = GetMainThread() ;
	ESLAssert( pThread != NULL ) ;
	const DWORD	dwHighIP = (roasCode << 24)
							| (m_module.m_iModule & 0x00FFFFFF) ;
	//
	if ( m_module.m_exmHeader.fnStaticInitialize != (DWORD) -1 )
	{
		//
		// StaticInitialize 関数実行
		//
		INT64	addrFunc = (((INT64)dwHighIP) << 32)
							| m_module.m_exmHeader.fnStaticInitialize ;
		m_statusLoaded = initializingModule ;
		const wchar_t *	pwszException =
				pThread->CallFunction( addrFunc, NULL, 0 ) ;
		m_statusLoaded = loadedModule ;
		if ( pwszException != NULL )
		{
			Trace( "exception at StaticInitialize\n" ) ;
			return	errFailed ;
		}
		if ( pThread->m_regset[regAcc].i != errSuccess )
		{
			return	(SError) pThread->m_regset[regAcc].i ;
		}
	}
	m_statusLoaded = initializedModule ;
	return	errSuccess ;
}

// main 関数実行
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::RunMain( const wchar_t * pwszArg )
{
	if ( m_statusLoaded != initializedModule )
	{
		return	errFailed ;
	}
	ThreadObject *	pThread = GetMainThread() ;
	ESLAssert( pThread != NULL ) ;
	const DWORD	dwHighIP = (roasCode << 24)
							| (m_module.m_iModule & 0x00FFFFFF) ;
	//
	if ( m_module.m_exmHeader.fnEntryPoint != (DWORD) -1 )
	{
		//
		// main 関数実行
		//
		int	nPushedCount = 0 ;
		pThread->PushStringOnStack( nPushedCount, pwszArg ) ;
		//
		Register	regArg[1] ;
		regArg[0] = pThread->m_regset[regSP] ;
		//
		INT64	addrFunc = (((INT64)dwHighIP) << 32)
							| m_module.m_exmHeader.fnEntryPoint ;
		m_statusLoaded = runningModule ;
		const wchar_t *	pwszException =
				pThread->CallFunction( addrFunc, regArg, 1 ) ;
		m_statusLoaded = finishedModule ;
		if ( pwszException != NULL )
		{
			Trace( "exception at main\n" ) ;
			return	errFailed ;
		}
		pThread->FreeStack( nPushedCount ) ;
	}
	m_statusLoaded = finishedModule ;
	return	errSuccess ;
}

// コンテキスト保存処理
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::SaveDynamicContext( SFileInterface& file )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err = errSuccess ;
	do
	{
		err = PrepareSave( this, pSysThread ) ;
		if ( err )
		{
			break ;
		}
		err = SaveDynamic( &file, this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// コンテキスト復元処理
//////////////////////////////////////////////////////////////////////////////
SError EnvironmentVM::LoadDynamicContext( SFileInterface& file )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err = errSuccess ;
	do
	{
		err = LoadDynamic( &file, this, pSysThread ) ;
		if ( err )
		{
			break ;
		}
		err = CommitAfterLoad( this, pSysThread ) ;
		if ( err )
		{
			break ;
		}
		err = OnLoadedDynamic( this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}



// StandardVM オーバーライド
//////////////////////////////////////////////////////////////////////////////

// 仮想マシンの解放
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ReleaseVM( void )
{
	StandardVM::ReleaseVM() ;
	SEnvironment::ClearEnvironment() ;
	m_module.DeleteModule() ;
}

// 処理されない例外エラー処理
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::HandleExceptionError
	( Context * context, const wchar_t * pwszErr )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			if ( pPlugin->OnHandleExceptionError( this, context, pwszErr ) )
			{
				return ;
			}
		}
	}
	StandardVM::HandleExceptionError( context, pwszErr ) ;
}

// デバッグ用例外エラー処理
//////////////////////////////////////////////////////////////////////////////
DWORD EnvironmentVM::HandleExceptionEscape
		( Context * context, DWORD maskException )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			maskException =
				pPlugin->OnExceptionEscape( this, context, maskException ) ;
			if ( !(maskException & interruptEscape) )
			{
				break ;
			}
		}
	}
	return	maskException ;
}

// モジュールがアタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::OnModuleAttached( int iModule, ExecutableModule * module )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			pPlugin->OnModuleAttached( this, iModule, module ) ;
		}
	}
	StandardVM::OnModuleAttached( iModule, module ) ;
}

// モジュールがデタッチされる（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::OnModuleDetached( int iModule, ExecutableModule * module )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			pPlugin->OnModuleDetached( this, iModule, module ) ;
		}
	}
	StandardVM::OnModuleDetached( iModule, module ) ;
}

// スレッドがアタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::OnThreadAttached( ThreadObject * thread )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			pPlugin->OnThreadAttached( this, thread ) ;
		}
	}
	StandardVM::OnThreadAttached( thread ) ;
}

// スレッドがデタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::OnThreadDetached( ThreadObject * thread )
{
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			pPlugin->OnThreadDetached( this, thread ) ;
		}
	}
	StandardVM::OnThreadDetached( thread ) ;
}

// エクスポート関数取得
//////////////////////////////////////////////////////////////////////////////
void * EnvironmentVM::GetModuleExportFunction( const wchar_t * pwszFuncName )
{
	SString			strFuncName = pwszFuncName ;
	SArray<char>	bufFuncName ;
	const char *	pszFuncName = strFuncName.EncodeDefaultTo( bufFuncName ) ;
	//
	const size_t	countPlugins = m_plugins.GetLength() ;
	for ( size_t i = 0; i < countPlugins; i ++ )
	{
		PluginInterface *	pPlugin = m_plugins.GetAt( i ) ;
		if ( pPlugin != NULL )
		{
			void *	ptrFunc = pPlugin->GetModuleExportFunction( pszFuncName ) ;
			if ( ptrFunc != NULL )
			{
				return	ptrFunc ;
			}
		}
	}
#if	defined(__PLATFORM_WINDOWS__)
	void *	ptrFunc =
		::GetProcAddress
			( ::GetModuleHandle( NULL ), pszFuncName ) ;
	if ( ptrFunc != NULL )
	{
		return	ptrFunc ;
	}
#endif
	return	StandardVM::GetModuleExportFunction( pwszFuncName ) ;
}


// SEnvironment オーバーライド
//////////////////////////////////////////////////////////////////////////////

// 非標準タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseExtendedEnvironment( const SXMLDocument& xmlTag )
{
	if ( xmlTag.GetTag() == L"module" )
	{
		ParseEnvironmentModuleTag( xmlTag ) ;
	}
	else if ( xmlTag.GetTag() == L"fonts" )
	{
		ParseEnvironmentFontsTag( xmlTag ) ;
	}
	else if ( xmlTag.GetTag() == L"sound" )
	{
		ParseEnvironmentSoundTag( xmlTag ) ;
	}
	else if ( xmlTag.GetTag() == L"opengl" )
	{
		ParseEnvironmentOpenGLTag( xmlTag ) ;
	}
}

// <module> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentModuleTag
		( const SSystem::SXMLDocument& xmlTag )
{
	SString *	pstrDLL = xmlTag.GetAttributeAs( L"file" ) ;
	if ( pstrDLL != NULL )
	{
	#if	defined(__PLATFORM_WINDOWS__)
		SString			strArg = xmlTag.GetAttrStringAs( L"arg", NULL ) ;
		SArray<char>	bufDLL ;
		PluginObject *	pPlugin =
			PluginObject::LoadPlugin
				( this, pstrDLL->EncodeDefaultTo(bufDLL), strArg ) ;
		if ( pPlugin != NULL )
		{
			m_plugins.Add( pPlugin ) ;
		}
	#endif
	}
}

// <fonts> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentFontsTag
		( const SSystem::SXMLDocument& xmlTag )
{
	const size_t	countElement = xmlTag.GetElementsCount() ;
	for ( size_t i = 0; i < countElement; i ++ )
	{
		SXMLDocument *	pxmlFont = xmlTag.GetElementAt( i ) ;
		if ( pxmlFont == NULL )
		{
			continue ;
		}
		if ( pxmlFont->GetTag() == L"filter" )
		{
			ParseEnvironmentFontsFilterTag( *pxmlFont ) ;
		}
		else if ( pxmlFont->GetTag() == L"file" )
		{
			ParseEnvironmentFontsFileTag( *pxmlFont ) ;
		}
	}
}

// <fonts><filter> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentFontsFilterTag
		( const SSystem::SXMLDocument& xmlTag )
{
	SString *	pstrIn = xmlTag.GetAttributeAs( L"in" ) ;
	SString *	pstrOut = xmlTag.GetAttributeAs( L"out" ) ;
	if ( (pstrIn != NULL) && (pstrOut != NULL) )
	{
		SakuraGL::SGLFont::RegisterRemapFont( *pstrIn, *pstrOut ) ;
	}
}

// <fonts><file> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentFontsFileTag
		( const SSystem::SXMLDocument& xmlTag )
{
	SString *	pstrName = xmlTag.GetAttributeAs( L"name" ) ;
	SString *	pstrPath = xmlTag.GetAttributeAs( L"path" ) ;
	if ( (pstrName != NULL) && (pstrPath != NULL) )
	{
		SFileInterface *	pFile =
			NewOpenFile( *pstrPath, SFileOpener::shareRead ) ;
		if ( pFile != NULL )
		{
			SGLBitmapFontLoader *	pFont = new SGLBitmapFontLoader ;
			pFont->SetCacheLimit
				( (size_t) xmlTag.GetAttrIntegerAs( L"cache_kb",
								pFont->GetCacheLimit() / 1024 ) * 1024 ) ;
			if ( !pFont->OpenFontFile( pFile ) )
			{
				ESLTrace( "register bitmap font %s : \'%s\'\n",
							pstrName->ToCharArray().GetConstArray(),
							pstrPath->ToCharArray().GetConstArray() ) ;
				SGLFont::RegisterStockFont( *pstrName, pFont ) ;
				return ;
			}
			else
			{
				Trace( "failed to load bitmap font \'%s\'\n",
								pstrPath->ToCharArray().GetConstArray() ) ;
				delete	pFont ;
			}
		}
		else
		{
			Trace( "failed to open bitmap font \'%s\'\n",
								pstrPath->ToCharArray().GetConstArray() ) ;
		}
		#if	defined(__PLATFORM_ANDROID__)
		if ( pstrPath->CompareLeftNoCase( L"assets://" ) == 0 )
		{
			SGLAndroidFont::LoadFontFromAsset
					( *pstrName, pstrPath->Middle( 9 ) ) ;
		}
		#endif
	}
}

// <sound> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentSoundTag
		( const SSystem::SXMLDocument& xmlTag )
{
#if	defined(__PLATFORM_WINDOWS__)
	SakuraGL::SGLSoundFormat	fmtSound ;
	fmtSound.format = SakuraGL::formatSoundLinearPCM ;
	fmtSound.frequency =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"frequency", 44100 ) ;
	fmtSound.channels =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"channels", 2 ) ;
	fmtSound.bitsPerSample =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"bits_per_sample", 16 ) ;
	//
	SakuraGL::SGLDirectSoundPlayer::Initialize( fmtSound ) ;
#endif
}

// <opengl> タグ解釈
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::ParseEnvironmentOpenGLTag
		( const SSystem::SXMLDocument& xmlTag )
{
	ParseOpenGLDisableSwitch
		( OpenGLExtension::g_disable_run_any_threads,
									xmlTag, L"any_thread" ) ;
	ParseOpenGLDisableSwitch
		( OpenGLExtension::g_disable_texture_non_power_of_2,
									xmlTag, L"non_power_of_2" ) ;
	ParseOpenGLDisableSwitch
		( OpenGLExtension::g_disable_element_index_uint,
									xmlTag, L"element_index_uint" ) ;
	ParseOpenGLDisableSwitch
		( OpenGLExtension::g_disable_opengl_2_0, xmlTag, L"gl2_0" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLContext::m_disable_create_std_gouraud_shader,
									xmlTag, L"std_gouraud_shader" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLContext::m_disable_create_std_phong_shader,
									xmlTag, L"std_phong_shader" ) ;
	ParseOpenGLDisableSwitch
		( OpenGLExtension::g_disable_compute_shader,
									xmlTag, L"compute_shader" ) ;
	//
	SGLOpenGLDefaultShader::m_limit_bone_count =
		(size_t) xmlTag.GetAttrIntegerAs
			( L"limit_bone", SGLOpenGLDefaultShader::m_limit_bone_count ) ;
	SGLOpenGLDefaultShader::m_limit_light_count =
		(size_t) xmlTag.GetAttrIntegerAs
			( L"limit_light", SGLOpenGLDefaultShader::m_limit_light_count ) ;
	SGLOpenGLDefaultShader::m_limit_shadow_map_count =
		(size_t) xmlTag.GetAttrIntegerAs
			( L"limit_shadowmap",
				SGLOpenGLDefaultShader::m_limit_shadow_map_count ) ;
	//
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_environment_mapping,
									xmlTag, L"env_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_environment_cubemapping,
									xmlTag, L"env_cube_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_environment_spheremapping,
									xmlTag, L"env_sphere_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_environment_viewport,
									xmlTag, L"env_viewport_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_environment_refraction,
									xmlTag, L"env_refraction" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_normal_mapping,
									xmlTag, L"normal_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_height_mapping,
									xmlTag, L"height_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_specular_mapping,
									xmlTag, L"specular_mapping" ) ;
	ParseOpenGLDisableSwitch
		( SGLOpenGLDefaultShader::m_disable_global_ao_lightmap,
									xmlTag, L"global_ao_lightmap" ) ;
	//
	SString	strProgramCache = xmlTag.GetAttrStringAs( L"program_cache" ) ;
	if ( !strProgramCache.IsEmpty() )
	{
		if ( m_plibShaderBinary == NULL )
		{
			m_plibShaderBinary = new S3DShaderBinaryLibrary ;
		}
		SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
				( strProgramCache, SFileOpener::shareRead ) ;
		if ( (pFile != NULL) && (pFile->GetLength() < 4 * 1024 * 1024) )
		{
			uint32_t	nProgramVer ;
			nProgramVer =
				(uint32_t) xmlTag.GetAttrIntegerAs( L"program_ver", 0 ) ;
			if ( m_plibShaderBinary->Load( *pFile, nProgramVer ) )
			{
				Trace( "failed to load OpenGL program chache \'%s\'.\n",
						strProgramCache.ToCharArray().GetConstArray() ) ;
			}
			else
			{
				Trace( "loaded OpenGL program chache \'%s\'.\n",
						strProgramCache.ToCharArray().GetConstArray() ) ;
			}
		}
		else
		{
			Trace( "failed to open OpenGL program chache \'%s\'.\n",
					strProgramCache.ToCharArray().GetConstArray() ) ;
		}
		S3DShaderBinaryLibrary::SetInstance( m_plibShaderBinary ) ;
	}
}

void EnvironmentVM::ParseOpenGLDisableSwitch
	( bool& fDisableSwitch,
		const SSystem::SXMLDocument& xmlTag, const wchar_t * pwszAttr )
{
	SString *	pstrTemp = xmlTag.GetAttributeAs( pwszAttr ) ;
	if ( pstrTemp != NULL )
	{
		if ( *pstrTemp == L"disabled" )
		{
			fDisableSwitch = true ;
		}
		else if ( *pstrTemp == L"enabled" )
		{
			fDisableSwitch = false ;
		}
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	EnvironmentVM::NewOpenFile
		( const wchar_t * pwszFilePath, long int nOpenFlags ) const
{
	return	StandardVM::NewOpenFile( pwszFilePath, nOpenFlags ) ;
}

// ファイルは存在するか？
//////////////////////////////////////////////////////////////////////////////
bool EnvironmentVM::IsExistingFile( const wchar_t * pwszFilePath ) const
{
	return	StandardVM::IsExistingFile( pwszFilePath ) ;
}

// SParserErrorInterface 実装
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::OutputError
	( const SStringParser& ss, const wchar_t * pszError )
{
}

void EnvironmentVM::OutputWarning
	( const SStringParser& ss, const wchar_t * pszWarning )
{
}

// 無音音声をループ再生する
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::StartSilentSound
	( uint32_t msec, uint32_t freq, uint32_t ch, uint32_t bits )
{
	SGLSoundFormat	fmt ;
	fmt.format = formatSoundLinearPCM ;
	fmt.frequency = freq ;
	fmt.channels = ch ;
	fmt.bitsPerSample = bits ;
	//
	SGLSoundPlayer *	pPlayer = new SGLSoundPlayer ;
	if ( !pPlayer->Open( fmt ) )
	{
		uint32_t	nSamples = freq * msec / 1000 ;
		//
		SArray<uint8_t>	bufSound ;
		bufSound.SetLength( nSamples * ch * bits / 8 ) ;
		if ( bits == 8 )
		{
			eslFillMemory
				( bufSound.GetArray(), 0x80, bufSound.GetLength() ) ;
			bufSound.FinishArray() ;
		}
		pPlayer->WriteStatic
			( bufSound.GetArray(), bufSound.GetLength() ) ;
		bufSound.FinishArray() ;
		//
		pPlayer->Play( SoundPlayer::flagPlayLoop ) ;
		//
		m_pSilentSound = pPlayer ;
	}
	else
	{
		delete	pPlayer ;
	}
}

// 無音音声のループ再生を停止する
//////////////////////////////////////////////////////////////////////////////
void EnvironmentVM::EndSilentSound( void )
{
	SGLSoundPlayer *	pPlayer =
		ESLTypeCast<SGLSoundPlayer>( m_pSilentSound.Ptr() ) ;
	if ( pPlayer != NULL )
	{
		pPlayer->Stop() ;
	}
	m_pSilentSound = NULL ;
}


