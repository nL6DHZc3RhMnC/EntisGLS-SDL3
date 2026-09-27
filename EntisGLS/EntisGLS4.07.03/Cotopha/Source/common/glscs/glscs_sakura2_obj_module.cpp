
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_module.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ファイル・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ECSSakura2::ModuleObject, ECSVolatileObject, ExecutableModule )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ModuleObject::ModuleObject( void )
{
	m_flagLoaded = false ;
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SError ModuleObject::LoadModule
	( StandardVM * vm, const wchar_t * pszFileName, int iModule )
{
	FreeModuleOnVM( vm ) ;
	if ( vm == NULL )
	{
		return	errFailed ;
	}
	SSmartPointer<SFileInterface>
		pFile = vm->NewOpenFile( pszFileName, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	SError	err = ExecutableModule::ReadModule( pFile ) ;
	if ( !err )
	{
		if ( LoadModuleOnVM( vm, iModule ) )
		{
			return	errFailed ;
		}
		m_strFilePath = pszFileName ;
	}
	return	err ;
}

SError ModuleObject::ReadModule
	( StandardVM * vm, SSystem::SFileInterface * pFile )
{
	FreeModuleOnVM( vm ) ;
	if ( vm == NULL )
	{
		return	errFailed ;
	}
	SError	err = ExecutableModule::ReadModule( pFile ) ;
	if ( err )
	{
		return	errFailed ;
	}
	return	LoadModuleOnVM( vm ) ;
}

// モジュールを仮想マシンにロード
//////////////////////////////////////////////////////////////////////////////
SError ModuleObject::LoadModuleOnVM( StandardVM * vm, int iModule )
{
	const wchar_t *	pwszErr =
				vm->LoadModuleByPrologueOnSysThread( this, iModule ) ;
	if ( pwszErr != NULL )
	{
		SArray<char>	strTemp ;
		SString			strErr = pwszErr ;
		Trace( "%s\n", strErr.EncodeDefaultTo( strTemp ) ) ;
		//
		vm->FreeModuleAllocation( this ) ;
		return	errFailed ;
	}
	SEnvironmentInterface *	pEnv = vm->GetEnvironment() ;
	if ( pEnv != NULL )
	{
		if ( pEnv->IsEnabledSakura2JITCompiler() )
		{
			ExecutableModule::CompileToNativeCode
				( !pEnv->IsEnabledSakura2JITBoundary(),
						pEnv->GetSakura2JITCpuFeatures() ) ;
		}
	}
	else
	{
		ExecutableModule::CompileToNativeCode( false ) ;
	}
	m_flagLoaded = true ;
	return	errSuccess ;
}

// モジュール解放
//////////////////////////////////////////////////////////////////////////////
void ModuleObject::FreeModuleOnVM( StandardVM * vm )
{
	if ( m_flagLoaded )
	{
		if ( vm != NULL )
		{
			vm->UnloadModuleByEpilogueOnSysThread( this ) ;
		}
		DeleteModule() ;
		m_flagLoaded = false ;
	}
	m_strFilePath.FreeArray() ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ModuleObject::GetTypeName( void ) const
{
	return	L"SSystem::Module" ;
}

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void ModuleObject::OnDestruction
	( VirtualMachine * vm, Context * context )
{
	FreeModuleOnVM( ESLTypeCast<StandardVM>( vm ) ) ;
	//
	ECSVolatileObject::OnDestruction( vm, context ) ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError ModuleObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::SaveStatic( file, vm, context ) ;
	file->WriteString( m_strFilePath ) ;
	int32_t	iModule = m_iModule ;
	file->Write( &iModule, sizeof(int32_t) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError ModuleObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::LoadStatic( file, vm, context ) ;
	//
	SString	strFilePath ;
	int32_t	iModule ;
	file->ReadString( strFilePath ) ;
	file->Read( &iModule, sizeof(int32_t) ) ;
	//
	if ( !m_strFilePath.IsEmpty() )
	{
		return	LoadModule
			( ESLTypeCast<StandardVM>( vm ), strFilePath, iModule ) ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// SSystem::Module スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Module
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SSystem_Module,context,cls_id)
{
	return	new ModuleObject ;
}

// SError Module::LoadModule( const wchar_t * pszFileName ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Module_LoadModule,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ModuleObject, pModule, arg, Module::LoadModule ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszFileName,
				arg[1].i, pszFileName at Module::LoadModule ) ;
	//
	SString	strFileName = pszFileName ;
	context->m_regset[regAcc].i =
		pModule->LoadModule( ESLTypeCast<StandardVM>(vm), strFileName ) ;
	//
	return	NULL ;
}

// SError Module::ReadModule( File * pFile ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Module_ReadModule,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ModuleObject, pModule, arg, Module::ReadModule ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, SFileInterface, pFile,
				arg[1].i, pFile at Module::ReadModule ) ;
	//
	context->m_regset[regAcc].i =
		pModule->ReadModule( ESLTypeCast<StandardVM>(vm), pFile ) ;
	//
	return	NULL ;
}

// uint64_t FindFunction
//	( const wchar_t * pszFuncName, const void * ptrReserved = NULL ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Module_FindFunction,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ExecutableModule, pModule, arg, Module::FindFunction ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszFuncName,
				arg[1].i, pszFuncName at Module::FindFunction ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	//
	SString	strFuncName = pszFuncName ;
	ExecutableModule::FUNC_ENTRY *
			pFunc = pModule->GetFunctionEntry( strFuncName ) ;
	if ( pFunc != NULL )
	{
		const DWORD	dwHighIP =
				(VirtualMachine::roasCode << 24)
							| (pModule->m_iModule & 0x00FFFFFF) ;
		context->m_regset[regAcc].i =
				(((uint64_t)dwHighIP) << 32) | pFunc->dwAddress ;
	}
	//
	return	NULL ;
}

// uint64_t FindVariable
//	( const wchar_t * pszVarName, const void * ptrReserved = NULL ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Module_FindVariable,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ExecutableModule, pModule, arg, Module::FindVariable ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszVarName,
				arg[1].i, pszFuncName at Module::FindVariable ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	//
	SString	strVarName = pszVarName ;
	ExecutableModule::SYMBOL_INFO *
			pVar = pModule->GetVariableEntry( strVarName ) ;
	if ( pVar != NULL )
	{
		context->m_regset[regAcc].i = pVar->nAddress ;
	}
	//
	return	NULL ;
}

#endif
