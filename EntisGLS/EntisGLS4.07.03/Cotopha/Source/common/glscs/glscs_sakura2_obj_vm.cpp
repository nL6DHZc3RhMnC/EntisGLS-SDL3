
/*****************************************************************************
					Sakura2 仮想マシンオブジェクト
 *****************************************************************************/

#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_erisa_lib.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// Sakura2 仮想マシンオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::Sakura2VMObject, EnvironmentVM )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
Sakura2VMObject::Sakura2VMObject( void )
{
	m_typePackage = packageInvalid ;
	m_signalExit.Initialize( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
Sakura2VMObject::~Sakura2VMObject( void )
{
	m_signalExit.Delete() ;
}

// 仮想マシンを起動する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError Sakura2VMObject::OpenVM
	( const wchar_t * pwszFilePath,
			const wchar_t * pwszArg,
			Sakura2VMObject::PackageType typePackage )
{
	//
	// 仮想マシンを読み込む
	//
	SError	err = LoadVM( pwszFilePath, typePackage ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// main 関数スレッド開始
	//
	return	BeginVMMain( pwszArg ) ;
}

// 仮想マシンを読み込む（main を実行はしない）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError Sakura2VMObject::LoadVM
	( const wchar_t * pwszFilePath,
		Sakura2VMObject::PackageType typePackage )
{
	ReleaseVM() ;
	//
	// 環境を設定する
	//
	SSmartPointer<SFileInterface>	file =
		SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::shareRead ) ;
	if ( file == NULL )
	{
		return	errFailed ;
	}
	SString	strEnvCurrent ;
	if ( typePackage == packageFile )
	{
		SString	strBaseDir = SString(pwszFilePath).GetFileDirectoryPart() ;
		AddFileOpener
			( new SOffsetFileOpener
				( strBaseDir, L'\\',
					new SStandardFileOpener, true ), pwszFilePath ) ;
		strEnvCurrent = strBaseDir ;
	}
	else if ( typePackage == packageArchive )
	{
		ERISA::SGLArchiveFile *	pArchive = new ERISA::SGLArchiveFile ;
		if ( pArchive->OpenArchive
				( file.Detach(), true, SFile::shareRead ) )
		{
			delete	pArchive ;
			return	errFailed ;
		}
		AddFileOpener( pArchive, pwszFilePath ) ;
		//
		file = pArchive->NewOpenFile( L"cotopha.xml", SFile::shareRead ) ;
		if ( file == NULL )
		{
			return	errFailed ;
		}
	}
	else
	{
		return	errFailed ;
	}
	SError	err ;
	err = LoadEnvironment( *file ) ;
	if ( err )
	{
		return	err ;
	}
	RegisterEnvironmentString( L"CURRENT", strEnvCurrent ) ;
	//
	// モジュール読み込み
	//
	err = LoadPrimaryModule() ;
	if ( err )
	{
		return	err ;
	}
	//
	// StaticInitialize 関数実行
	//
	return	RunStaticInitialize() ;
}

// main 関数スレッド開始
//////////////////////////////////////////////////////////////////////////////
SSystem::SError Sakura2VMObject::BeginVMMain( const wchar_t * pwszArg )
{
	m_strMainArg = pwszArg ;
	if ( SThread::BeginStockThread
		( &Sakura2VMObject::VMMainThreadProc, this ) == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// main 関数スレッド継続
//////////////////////////////////////////////////////////////////////////////
SSystem::SError Sakura2VMObject::ContinueVMMain( void )
{
	if ( SThread::BeginStockThread
		( &Sakura2VMObject::VMMainContinueThreadProc, this ) == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 仮想マシンを閉じる
//////////////////////////////////////////////////////////////////////////////
void Sakura2VMObject::ReleaseVM( void )
{
	EnvironmentVM::ReleaseVM() ;
	m_typePackage = packageInvalid ;
	m_strPackageFile.FreeArray() ;
	m_signalExit.ResetSignal() ;
}

// 仮想マシンの終了コードを取得する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError Sakura2VMObject::GetExitCode
	( int64_t& codeExit, int64_t msecTimeout )
{
	SError	err = m_signalExit.Wait( msecTimeout ) ;
	if ( !err )
	{
		codeExit = m_codeExit ;
	}
	return	err ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void Sakura2VMObject::VMMainThreadProc( void * pInstance )
{
	((Sakura2VMObject*)pInstance)->VMMainProc() ;
}

void Sakura2VMObject::VMMainContinueThreadProc( void * pInstance )
{
	((Sakura2VMObject*)pInstance)->VMMainContinueProc() ;
}

void Sakura2VMObject::VMMainProc( void )
{
	m_codeExit = -1 ;
	//
	ThreadObject *	pThread = GetMainThread() ;
	if ( pThread != NULL )
	{
		SError	err = RunMain( m_strMainArg ) ;
		if ( !err )
		{
			m_codeExit = pThread->m_regset[regAcc].i ;
		}
		if ( !pThread->IsThreadAborting() )
		{
			UnloadPrimaryModule() ;
		}
		m_statusLoaded = finishedModule ;
	}
	m_signalExit.SetSignal() ;
}

void Sakura2VMObject::VMMainContinueProc( void )
{
	m_codeExit = -1 ;
	//
	ThreadObject *	pThread = GetMainThread() ;
	if ( pThread != NULL )
	{
		const wchar_t *	pwszErr = pThread->ExecuteShell() ;
		if ( pwszErr == NULL )
		{
			m_codeExit = pThread->m_regset[regAcc].i ;
		}
		if ( !pThread->IsThreadAborting() )
		{
			UnloadPrimaryModule() ;
		}
	}
	m_signalExit.SetSignal() ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError Sakura2VMObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SError	err = SaveVMPackageFile( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	EnvironmentVM::SaveStatic( file, vm, context ) ;
}

SError Sakura2VMObject::SaveDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SError	err = SaveVMPackageFile( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	EnvironmentVM::SaveDynamic( file, vm, context ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError Sakura2VMObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SError	err = LoadVMPackageFile( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	EnvironmentVM::LoadStatic( file, vm, context ) ;
}

SError Sakura2VMObject::LoadDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SError	err = LoadVMPackageFile( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	EnvironmentVM::LoadDynamic( file, vm, context ) ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError Sakura2VMObject::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	SError	err = EnvironmentVM::OnLoadedDynamic( vm, context ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_statusLoaded == initializingModule )
	{
		ThreadObject *	pThread = GetMainThread() ;
		if ( pThread != NULL )
		{
			const wchar_t *	pwszErr = pThread->ExecuteShell() ;
			if ( (pwszErr != NULL)
				|| (pThread->m_regset[regAcc].i != errSuccess) )
			{
				m_statusLoaded = loadedModule ;
				return	errSuccess ;
			}
			if ( pThread->m_status == Context::xsHalt )
			{
				pThread->m_regset[regSP].l32 += sizeof(Register) * 1 ;
			}
		}
		m_statusLoaded = initializedModule ;
	}
	if ( m_statusLoaded == initializedModule )
	{
		BeginVMMain( NULL ) ;
	}
	else if ( m_statusLoaded == runningModule )
	{
		ContinueVMMain() ;
	}
	else if ( m_statusLoaded == finishedModule )
	{
		m_signalExit.SetSignal() ;
	}
	return	errSuccess ;
}

// パッケージ保存処理
//////////////////////////////////////////////////////////////////////////////
SError Sakura2VMObject::SaveVMPackageFile( SFileInterface * file )
{
	int32_t	typePackage = (int32_t) m_typePackage ;
	if ( file->Write( &typePackage, sizeof(int32_t) ) < sizeof(int32_t) )
	{
		return	errFailed ;
	}
	return	file->WriteString( m_strPackageFile ) ;
}

// パッケージ復元処理
//////////////////////////////////////////////////////////////////////////////
SError Sakura2VMObject::LoadVMPackageFile( SFileInterface * file )
{
	int32_t	typePackage ;
	if ( file->Read( &typePackage, sizeof(int32_t) ) < sizeof(int32_t) )
	{
		return	errFailed ;
	}
	SString	strPackageFile ;
	SError	err = file->ReadString( strPackageFile ) ;
	if ( err )
	{
		return	err ;
	}
	if ( typePackage != packageInvalid )
	{
		return	LoadVM( strPackageFile, (PackageType) typePackage ) ;
	}
	return	err ;
}


#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Sakura2VM
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Sakura2VM, context, cls_id )
{
	return	new Sakura2VMObject ;
}

// SSystem::SError SSystem::Sakura2VM::OpenVM
//	( const char * pszFilePath,
//		const char * pszArg = NULL,
//		PackageType typePackage = packageFile ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Sakura2VM_OpenVM, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, Sakura2VMObject, pVM, arg, Window::OpenVM ) ;
	const uint16_t *	pszFilePath =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	const uint16_t *	pszArg =
		(const uint16_t*) context->AtomicTranslateAddress( arg[2].i ) ;
	//
	context->m_regset[regAcc].i =
		pVM->OpenVM
			( SString(pszFilePath), SString(pszArg),
				(Sakura2VMObject::PackageType) arg[3].i ) ;
	//
	return	NULL ;
}

// void SSystem::Sakura2VM::ReleaseVM( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Sakura2VM_ReleaseVM, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, Sakura2VMObject, pVM, arg, Window::ReleaseVM ) ;
	//
	pVM->ReleaseVM() ;
	//
	return	NULL ;
}

// SSystem::SError SSystem::Sakura2VM::GetExitCode
//  ( int64_t& codeExit,
//		int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Sakura2VM_GetExitCode, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, Sakura2VMObject, pVM, arg, Window::GetExitCode ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, int64_t, pCodeExit,
				arg[1].i, codeExit at Window::GetExitCode ) ;
	//
	int64_t	msecTimeout = arg[2].i ;
	SError	err = errFailed ;
	if ( msecTimeout != Synchronism::Infinite )
	{
		int64_t	msecStart = CurrentMilliSec() ;
		for ( ; ; )
		{
			if ( context->m_status != Context::xsExecution )
			{
				err = errAbort ;
				break ;
			}
			int64_t	msecPast = CurrentMilliSec() - msecStart ;
			if ( msecPast >= msecTimeout )
			{
				err = errTimeout ;
				break ;
			}
			int64_t	msecLeft = msecTimeout - msecPast ;
			ESLAssert( msecLeft > 0 ) ;
			if ( msecLeft > 10 )
			{
				msecLeft = 10 ;
			}
			err = pVM->GetExitCode( *pCodeExit, msecLeft ) ;
			if ( err == errSuccess )
			{
				break ;
			}
		}
	}
	else
	{
		for ( ; ; )
		{
			if ( context->m_status != Context::xsExecution )
			{
				err = errAbort ;
				break ;
			}
			err = pVM->GetExitCode( *pCodeExit, 30 ) ;
			if ( err == errSuccess )
			{
				break ;
			}
		}
	}
	context->m_regset[regAcc].i = err ;
	//
	return	NULL ;
}

#endif
