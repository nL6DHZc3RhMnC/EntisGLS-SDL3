
#pragma	comment( lib, "winmm.lib" )
#pragma	comment( lib, "msacm32.lib" )
#pragma	comment( lib, "ws2_32.lib" )
#pragma	comment( lib, "comctl32.lib" )
#pragma	comment( lib, "vfw32.lib" )
#pragma	comment( lib, "ole32.lib" )
#pragma	comment( lib, "imm32.lib" )
#pragma	comment( lib, "wininet.lib" )
#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )
//#pragma	comment( lib, "strmbase.lib" )
#pragma	comment( lib, "strmiids.lib" )
#pragma comment( lib, "iphlpapi.lib" )
//#pragma	comment( lib, "dxerr9.lib" )
#pragma	comment( lib, "dxerr.lib" )
#pragma	comment( lib, "dsound.lib" )

#if	_MSC_VER >= 1700
#pragma comment( lib, "sensorsapi.lib" )
#endif

#include <loquaty_lib.h>

#if	!defined(DISABLE_ENTIS_GLS3)
//	#if	!defined(_MFC_VER)
		#if	defined(_DEBUG)
			#if	defined(DISABLE_ENTIS_GLS4_EXPORTS)
				#pragma	comment( lib, "gls_nxp_db.lib" )
			#else
				#pragma	comment( lib, "gls_db.lib" )
			#endif
			#if	!defined(_DISABLE_ESL_NEW)
				#pragma	comment( lib, "esl_new_db.lib" )
			#endif
		#else
			#if	defined(DISABLE_ENTIS_GLS4_EXPORTS)
				#pragma	comment( lib, "gls_nxp.lib" )
			#else
				#pragma	comment( lib, "gls.lib" )
			#endif
			#if	!defined(_DISABLE_ESL_NEW)
				#pragma	comment( lib, "esl_new.lib" )
			#endif
		#endif
/*
	#else
		#if	defined(_DEBUG)
			#if	defined(DISABLE_ENTIS_GLS4_EXPORTS)
				#pragma	comment( lib, "gls_nxp_mfcdb.lib" )
			#else
				#pragma	comment( lib, "gls_mfcdb.lib" )
			#endif
		#else
			#if	defined(DISABLE_ENTIS_GLS4_EXPORTS)
				#pragma	comment( lib, "gls_nxp_mfc.lib" )
			#else
				#pragma	comment( lib, "gls_mfc.lib" )
			#endif
		#endif
	#endif
*/
#else
	#include <linkgls4.h>
#endif

