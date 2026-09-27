
#if	!defined(__ESL_STD_DEFS_H__)
#define	__ESL_STD_DEFS_H__

//////////////////////////////////////////////////////////////////////////////
// Entis 標準ライブラリ互換用
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__ENTIS_GLS__) && !defined(DISABLE_ENTIS_GLS3)

#define	__ENTIS_GLS__	1

#if	defined(_M_IX86) && !defined(GLS_INTEL_X86)
#define	GLS_INTEL_X86	1
#endif


#if	!defined(_WIN32_WINNT)
	#define	_WIN32_WINNT	0x0501
#endif
#if	!defined(WINVER)
	#define WINVER 0x0501
#endif
#if	!defined(STRICT)
	#define	STRICT	1
#endif

#include <windows.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>


#include <eritypes.h>
#include <esl.h>

#endif


#include <esl/esl_stddefs_win32.h>

#endif


