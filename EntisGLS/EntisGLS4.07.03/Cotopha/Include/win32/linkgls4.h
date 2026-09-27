
#pragma comment( lib, "winmm.lib" )
#pragma	comment( lib, "imm32.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma	comment( lib, "wininet.lib" )
#pragma comment( lib, "msacm32.lib" )
#pragma comment( lib, "vfw32.lib" )
#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )
#pragma	comment( lib, "strmiids.lib" )
#pragma comment( lib, "iphlpapi.lib" )
#pragma	comment( lib, "dsound.lib" )
#pragma	comment( lib, "dxerr.lib" )

#if	_MSC_VER >= 1700
#pragma comment( lib, "sensorsapi.lib" )
#endif

#include <loquaty_lib.h>

#if	defined(_DEBUG) || defined(__DEBUG__)
	#if	defined(ENTISGLS4_DLL_IMPORT)
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			#pragma	comment( lib, "gls4_w64_ogl_dll_db.lib" )
		#else
			#pragma	comment( lib, "gls4_ogl_dll_db.lib" )
		#endif
	#else
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			#if	defined(_DLL)
				#pragma	comment( lib, "gls4_w64_ogl_mdd.lib" )
			#else
				#pragma	comment( lib, "gls4_w64_ogl_db.lib" )
			#endif
		#else
			#if	defined(_DLL)
				#pragma	comment( lib, "gls4_ogl_mdd.lib" )
			#else
				#pragma	comment( lib, "gls4_ogl_db.lib" )
			#endif
		#endif
	#endif
#else
	#if	defined(ENTISGLS4_DLL_IMPORT)
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			#pragma	comment( lib, "gls4_w64_ogl_dll.lib" )
		#else
			#pragma	comment( lib, "gls4_ogl_dll.lib" )
		#endif
	#else
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			#if	defined(_DLL)
				#pragma	comment( lib, "gls4_w64_ogl_md.lib" )
			#else
				#pragma	comment( lib, "gls4_w64_ogl.lib" )
			#endif
		#else
			#if	defined(_DLL)
				#pragma	comment( lib, "gls4_ogl_md.lib" )
			#else
				#pragma	comment( lib, "gls4_ogl.lib" )
			#endif
		#endif
	#endif
#endif


