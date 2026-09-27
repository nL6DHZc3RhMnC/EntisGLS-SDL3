
#include "app/sakura2vm.h"


EnvironmentVM *	g_vm = NULL ;
bool			g_flagAbort = false ;


int sglMain( const wchar_t * pwszArg )
{
	Trace( "sglMain \'%s\'", pwszArg ) ;
	//
	// 環境設定ファイル読み込み
	//
	SSmartPointer<SFileInterface>	pEnvFile =
		SFileOpener::DefaultNewOpenFile
			( L"assets://cotopha.xml", SFileOpener::shareRead ) ;
	if ( pEnvFile == NULL )
	{
		Trace( "failed to open cotopha.xml" ) ;
		return	1 ;
	}
	EnvironmentVM	vm ;
	Trace( "LoadEnvironment" ) ;
	if ( vm.LoadEnvironment( *pEnvFile ) )
	{
		Trace( "LoadEnvironment error: %s",
				vm.GetErrorMessage().ToCharArray().GetArray() ) ;
		return	1 ;
	}
	pEnvFile = NULL ;
	//
	// モジュール読み込み
	//
	Trace( "LoadPrimaryModule" ) ;
	{
		SProgressiveDialog	dlg ;
		SString	strAppName ;
		vm.GetApplicationName( strAppName ) ;
		dlg.SetCaption( strAppName ) ;
		dlg.SetMessage( L"起動しています…" ) ;
		dlg.Create( SProgressiveDialog::flagStyleSpinner ) ;
		//
		if ( vm.LoadPrimaryModule() )
		{
			Trace( "LoadPrimaryModule error: %s",
					vm.GetErrorMessage().ToCharArray().GetArray() ) ;
			return	1 ;
		}
	}
	QuickLock() ;
	if ( g_flagAbort )
	{
		g_flagAbort = false ;
		QuickUnlock() ;
		return	-1 ;
	}
	g_vm = &vm ;
	QuickUnlock() ;
	//
	// 実行
	//
	Trace( "run Sakura2VM" ) ;
	vm.Run( L"" ) ;
	Trace( "finished Sakura2VM" ) ;

	int	codeExit = 0 ;
	SSystem::UnlockAll() ;
	QuickLock() ;
	if ( g_flagAbort )
	{
		codeExit = -1 ;
	}
	g_vm = NULL ;
	g_flagAbort = false ;
	QuickUnlock() ;

	if ( codeExit < 0 )
	{
		vm.ReleaseVM() ;
	}
	else
	{
		vm.UnloadPrimaryModule() ;
	}

	return	codeExit ;
}

SGLError sglStaticInitialize( void )
{
	Trace( "sglStaticInitialize" ) ;
	return	sglErrSuccess ;
}

SGLError sglStaticFinalize( void )
{
	Trace( "sglStaticFinalize" ) ;
	return	sglErrSuccess ;
}

void sglAbortVM( void )
{
	Trace( "sglAbortVM" ) ;
	QuickLock() ;
	g_flagAbort = true ;
	if ( g_vm != NULL )
	{
		ThreadObject *	pThread = g_vm->GetMainThread() ;
		if ( pThread != NULL )
		{
			Trace( "halt main thread of Sakura2VM" ) ;
			pThread->ChangeExecutionStatus( Context::xsHalt ) ;
		}
	}
	QuickUnlock() ;
}


