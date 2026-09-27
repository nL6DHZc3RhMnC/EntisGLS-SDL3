
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2014 Leshade Entis. All rights reserved.
 ****************************************************************************/


#if	!defined(__ENTIS_GLS_H__)
#define	__ENTIS_GLS_H__	1

#if	!defined(DISABLE_ENTIS_GLS3)

//////////////////////////////////////////////////////////////////////////////
// 環境プリプロセッサ
//////////////////////////////////////////////////////////////////////////////

#define	__ENTIS_GLS__	1

#if	defined(_M_IX86) && !defined(GLS_INTEL_X86)
#define	GLS_INTEL_X86	1
#endif


//////////////////////////////////////////////////////////////////////////////
// ヘッダ
//////////////////////////////////////////////////////////////////////////////

#if	defined(_WINDOWS_)
#if	!defined(_WIN32_WINNT)
#define	_WIN32_WINNT	0x0400
#endif
#define	STRICT	1
#include <windows.h>
#include <winsock2.h>
#endif


#include <stddef.h>
#include <limits.h>

#include <egl.h>

#include <vfw.h>
#include <imm.h>

//#include <ddraw.h>


//////////////////////////////////////////////////////////////////////////////
// 初期化関数
//////////////////////////////////////////////////////////////////////////////

extern	"C"
{
	GLSEXPORT void glsInitializeLibrary( void ) ;
	GLSEXPORT void glsCloseLibrary( void ) ;
	GLSEXPORT void glsInitializeTask( void ) ;
	GLSEXPORT void glsCloseTask( void ) ;
	GLSEXPORT void glsEnableUnhandledExceptionFilter( bool flagEnabled ) ;
} ;

extern	"C"
{
	GLSEXPORT DWORD glsGetLibraryVersion( void ) ;
	GLSEXPORT HESLHEAP glsGetImageGlobalHeap( void ) ;
	GLSEXPORT void glsEnableProcessorType( DWORD dwForceEnable = 0 ) ;
	GLSEXPORT void glsDisableProcessorType( DWORD dwForceDisable = -1 ) ;
	GLSEXPORT DWORD glsGetEnabledProcessorType( void ) ;
} ;

#define	GLS_USE_MMX_PENTIUM	0x0002
#define	GLS_USE_XMM_P3		0x0008
#define	GLS_USE_XMM_SSE		0x0008
#define	GLS_USE_XMM_SSE2	0x0010
#define	GLS_USE_XMM_SSE3	0x0020


enum	EntisGLSCpuFamily
{
	GLS_CPU_FAMILY_UNKNOWN	= -1,
	GLS_CPU_FAMILY_X86		= 0,
	GLS_CPU_FAMILY_ARM		= 1,
	GLS_CPU_FAMILY_MIPS		= 2,
} ;

enum	EntisGLSCpuX86Features
{
	GLS_X86_MMX_PENTIUM	= 0x0002,
	GLS_X86_XMM_P3		= 0x0008,
	GLS_X86_XMM_SSE		= 0x0008,
	GLS_X86_XMM_SSE2	= 0x0010,
	GLS_X86_XMM_SSE3	= 0x0020,
} ;

enum	EntisGLSCpuArmFeatures
{
	GLS_ARM_FEATURE_ARMv7	= 0x0001,
	GLS_ARM_FEATURE_VFPv3	= 0x0002,
	GLS_ARM_FEATURE_NEON	= 0x0004,
} ;


//////////////////////////////////////////////////////////////////////////////
// クラス
//////////////////////////////////////////////////////////////////////////////

class	EMidiMusic ;

class	ESocket ;
	class	EHttpConnection ;
	class	ESyncHttpFile ;
	class	EGctpConnection ;

class	EWaveOutDevice ;
	class	EWaveMixingServer ;
class	EWaveStreamBuffer ;
	class	EWaveSound ;
		class	E3DSoundEffect ;
		class	MIOSoundStream ;

class	EWindow ;
class	EInputFilter ;
class	EWindowInterface ;
class	EGLSThread ;
class	EDisplayMode ;
class	ERegistryKey ;
class	EGameWindow ;
class	ERIAnimationPlayer ;
class	EGLDrawImage ;
class	E3DStereoDisplayInterface ;
	class	E3DStereoDisplayAnaglyphView ;

class	ESpriteInterface ;
	class	EWindowSpriteInterface ;
	class	EAnimationSprite ;
	class	EStaticTextSprite ;
	class	EProgressBarSprite ;
	class	EButtonSprite ;
	class	EScrollBarSprite ;
	class	ETextEditSprite ;
class	EFormResourceManager ;

class	EXEImageFileObject ;
class	EWin32IconFile ;

#include <glsmidimusic.h>
#include <glssockbase.h>
#include <glssound.h>
#include <glswndbase.h>
#include <stereoview_interface.h>
#include <glssurfdesc.h>
#include <glspeimagefile.h>
#include <glsscript.h>
#include <glsctpsprite.h>
#include <glsctprender.h>
#include <glsctpsetup.h>
#include <sakuragl/sakuragl.h>


#else

#include <sakura/sakura.h>
#include <sakuragl/sakuragl.h>


#endif
#endif

