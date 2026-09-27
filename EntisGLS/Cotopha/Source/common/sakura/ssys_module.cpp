
#include <sakura/sakura.h>
#include <sakura/ssys_module.h>

using namespace SSystem ;

#if	!defined(__COTOPHA__)
using namespace ECSSakura2 ;
#endif


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SModule, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SModule::SModule( void )
{
#if	defined(__COTOPHA__)
	m_pModule = new Module ;
#else
	m_fLoaded = false ;
	m_pVM = NULL ;
	m_pOwnerVM = NULL ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SModule::~SModule( void )
{
#if	defined(__COTOPHA__)
	delete	m_pModule ;
	m_pModule = NULL ;
#else
	if ( m_fLoaded )
	{
		ESLAssert( m_pVM != NULL ) ;
		m_pVM->UnloadModuleByEpilogueOnSysThread( &m_module ) ;
		m_module.DeleteModule() ;
		m_fLoaded = false ;
	}
	delete	m_pOwnerVM ;
	m_pVM = NULL ;
	m_pOwnerVM = NULL ;
#endif
}

// モジュールを読み込む
//////////////////////////////////////////////////////////////////////////////
SError SModule::LoadModule( const wchar_t * pszFileName )
{
#if	defined(__COTOPHA__)
	return	m_pModule->LoadModule( pszFileName ) ;
#else
	if ( m_fLoaded )
	{
		ESLAssert( m_pVM != NULL ) ;
		m_pVM->UnloadModuleByEpilogueOnSysThread( &m_module ) ;
		m_module.DeleteModule() ;
		m_fLoaded = false ;
	}
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
					( pszFileName, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	if ( m_module.ReadModule( pFile ) )
	{
		return	errFailed ;
	}
	m_module.CompileToNativeCode( false ) ;
	//
	m_pVM = ESLTypeCast<StandardVM>
					( SEnvironmentInterface::GetInstance() ) ;
	if ( m_pVM == NULL )
	{
		if ( m_pOwnerVM == NULL )
		{
			m_pOwnerVM = new StandardVM ;
			m_pOwnerVM->InitializeVM() ;
			m_pOwnerVM->AttachEnvironment
					( SEnvironmentInterface::GetInstance() ) ;
		}
		m_pVM = m_pOwnerVM ;
	}
	if ( m_pVM->LoadModuleByPrologueOnSysThread( &m_module ) != NULL )
	{
		return	errFailed ;
	}
	m_fLoaded = true ;
	return	errSuccess ;
#endif
}

SError SModule::ReadModule( SFileInterface * pFile )
{
#if	defined(__COTOPHA__)
	File *	pFileObj = pFile->GetFileObject() ;
	if ( pFileObj == NULL )
	{
		return	errFailed ;
	}
	return	m_pModule->ReadModule( pFileObj ) ;
#else
	if ( m_fLoaded )
	{
		ESLAssert( m_pVM != NULL ) ;
		m_pVM->UnloadModuleByEpilogueOnSysThread( &m_module ) ;
		m_module.DeleteModule() ;
		m_fLoaded = false ;
	}
	if ( m_module.ReadModule( pFile ) )
	{
		return	errFailed ;
	}
	m_module.CompileToNativeCode( false ) ;
	//
	m_pVM = ESLTypeCast<StandardVM>
					( SEnvironmentInterface::GetInstance() ) ;
	if ( m_pVM == NULL )
	{
		if ( m_pOwnerVM == NULL )
		{
			m_pOwnerVM = new StandardVM ;
			m_pOwnerVM->InitializeVM() ;
			m_pOwnerVM->AttachEnvironment
					( SEnvironmentInterface::GetInstance() ) ;
		}
		m_pVM = m_pOwnerVM ;
	}
	if ( m_pVM->LoadModuleByPrologueOnSysThread( &m_module ) != NULL )
	{
		return	errFailed ;
	}
	m_fLoaded = true ;
	return	errSuccess ;
#endif
}

// 関数アドレスを取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SModule::FindFunction
	( const wchar_t * pszFuncName, const void * ptrReserved ) const
{
#if	defined(__COTOPHA__)
	return	m_pModule->FindFunction( pszFuncName, ptrReserved ) ;
#else
	ExecutableModule::FUNC_ENTRY *
			pFunc = m_module.GetFunctionEntry( pszFuncName ) ;
	if ( pFunc == NULL )
	{
		return	0 ;
	}
	const DWORD	dwHighIP =
			(VirtualMachine::roasCode << 24)
						| (m_module.m_iModule & 0x00FFFFFF) ;
	return	(((uint64_t)dwHighIP) << 32) | pFunc->dwAddress ;
#endif
}

// 変数アドレスを取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SModule::FindVariable
	( const wchar_t * pszVarName, const void * ptrReserved ) const
{
#if	defined(__COTOPHA__)
	return	m_pModule->FindVariable( pszVarName, ptrReserved ) ;
#else
	ExecutableModule::SYMBOL_INFO *
			pVar = m_module.GetVariableEntry( pszVarName ) ;
	if ( pVar == NULL )
	{
		return	0 ;
	}
	return	pVar->nAddress ;
#endif
}

// 関数を呼び出す
//////////////////////////////////////////////////////////////////////////////
int64_t SModule::CallFunction
	( uint64_t pfnFuncAddr,
		const int64_t * pArg, size_t nArgCount )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	nArgCount
		//
		move	r1, pArg
		.WHILE	nArgCount != #zero
			dec		nArgCount
			load.64	acc, [r1 + nArgCount*8]
			push	acc
		.ENDW
		//
		REG LOAD	pfnFuncAddr
		call	pfnFuncAddr
		//
		REG RELOAD	nArgCount
		sll		nArgCount, 3
		add		sp, nArgCount
		ret
	}
	return	0 ;
#else
	if ( !m_fLoaded )
	{
		return	0 ;
	}
	ESLAssert( m_pVM != NULL ) ;
	return	m_pVM->CallFunctionOnSysThread
				( pfnFuncAddr, (const Register*) pArg, (int) nArgCount ) ;
#endif
}

