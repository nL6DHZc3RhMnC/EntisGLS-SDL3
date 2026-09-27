
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_ENVIRONMENT_H__)
#define	__GLSCS_SAKURA2_OBJECT_ENVIRONMENT_H__

// static native bool Environment::GetEnvironmentString
//	( SArray<uint16_t>& strValue, const wchar_t * pszValuePath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_GetEnvironmentString) ;

// static native void Environment::GetApplicationName( SArray<uint16_t> & strAppName ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_GetApplicationName) ;

// static native void Environment::SetApplicationName( const wchar_t * pszAppName ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_SetApplicationName) ;

// static native bool Environment::IsEnabledSakura2JITCompiler( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_IsEnabledSakura2JITCompiler) ;

// static native void Environment::EnableSakura2JITCompiler( bool fJIT ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_EnableSakura2JITCompiler) ;

// static native bool Environment::IsEnabledSakura2JITBoundary( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_IsEnabledSakura2JITBoundary) ;

// static native void Environment::EnableSakura2JITBoundary( bool fBoundary ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_EnableSakura2JITBoundary) ;

// static native bool Environment::CanOpenAllFileForWriting( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_CanOpenAllFileForWriting) ;

// static native void Environment::AcceptAllFileForWriting( bool fAllWriting ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Environment_AcceptAllFileForWriting) ;

#endif
