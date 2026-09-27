
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_environment.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;

#if	!defined(ENTISGLS4_DLL_IMPORT)

// static native bool Environment::GetEnvironmentString
//	( SArray<uint16_t>& strValue, const wchar_t * pszValuePath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_GetEnvironmentString, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pValue,
				arg[0].i, strValue at Environment::GetEnvironmentString ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszValuePath,
				arg[1].i, pszValuePath at Environment::GetEnvironmentString ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	bool	fSuccessed = false ;
	if ( env != NULL )
	{
		SString	strValuePath = pszValuePath ;
		SString	strValue ;
		fSuccessed = env->GetEnvironmentString( strValue, strValuePath ) ;
		if ( fSuccessed )
		{
			size_t	nCount = strValue.GetLength() ;
			uint16_t *	pStrArray =
				(uint16_t*) pValue->AllocateArray
						( nCount + 1, sizeof(uint16_t), vm ) ;
			if ( pStrArray != NULL )
			{
				const uint16_t *	pszValue = strValue.GetConstArray() ;
				pValue->m_nLength = (DWORD) nCount ;
				for ( size_t i = 0; i <= nCount; i ++ )
				{
					pStrArray[i] = pszValue[i] ;
				}
			}
		}
	}
	context->m_regset[regAcc].i = fSuccessed ? -1 : 0 ;
	//
	return	NULL ;
}

// static native void Environment::GetApplicationName( SArray<uint16_t> & strAppName ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_GetApplicationName, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pAppName,
				arg[0].i, strAppName at Environment::GetApplicationName ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		SString	strAppName ;
		env->GetApplicationName( strAppName ) ;
		//
		size_t	nCount = strAppName.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pAppName->AllocateArray
					( nCount + 1, sizeof(uint16_t), vm ) ;
		if ( pStrArray != NULL )
		{
			const uint16_t *	pszAppName = strAppName.GetConstArray() ;
			pAppName->m_nLength = (DWORD) nCount ;
			for ( size_t i = 0; i <= nCount; i ++ )
			{
				pStrArray[i] = pszAppName[i] ;
			}
		}
	}
	return	NULL ;
}

// static native void Environment::SetApplicationName( const wchar_t * pszAppName ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_SetApplicationName, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszAppName,
				arg[0].i, pszAppName at Environment::SetApplicationName ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		SString	strAppName = pszAppName ;
		env->SetApplicationName( strAppName ) ;
	}
	return	NULL ;
}

// static native bool Environment::IsEnabledSakura2JITCompiler( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_IsEnabledSakura2JITCompiler, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	bool	fEnabled = false ;
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		fEnabled = env->IsEnabledSakura2JITCompiler() ;
	}
	context->m_regset[regAcc].i = fEnabled ? -1 : 0 ;
	return	NULL ;
}

// static native void Environment::EnableSakura2JITCompiler( bool fJIT ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_EnableSakura2JITCompiler, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		env->EnableSakura2JITCompiler( arg[0].i != 0 ) ;
	}
	return	NULL ;
}

// static native bool Environment::IsEnabledSakura2JITBoundary( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_IsEnabledSakura2JITBoundary, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	bool	fEnabled = false ;
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		fEnabled = env->IsEnabledSakura2JITBoundary() ;
	}
	context->m_regset[regAcc].i = fEnabled ? -1 : 0 ;
	return	NULL ;
}

// static native void Environment::EnableSakura2JITBoundary( bool fBoundary ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_EnableSakura2JITBoundary, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		env->EnableSakura2JITBoundary( arg[0].i != 0 ) ;
	}
	return	NULL ;
}

// static native bool Environment::CanOpenAllFileForWriting( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_CanOpenAllFileForWriting, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	bool	fResult = false ;
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		fResult = env->CanOpenAllFileForWriting() ;
	}
	context->m_regset[regAcc].i = fResult ? -1 : 0 ;
	return	NULL ;
}

// static native void Environment::AcceptAllFileForWriting( bool fAllWriting ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Environment_AcceptAllFileForWriting, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	SEnvironmentInterface *	env = vm->GetEnvironment() ;
	if ( env != NULL )
	{
		env->AcceptAllFileForWriting( arg[0].i != 0 ) ;
	}
	return	NULL ;
}

#endif
