
#if	!defined(__ESL_STD_DEFS_WIN32_H__)
#define	__ESL_STD_DEFS_WIN32_H__

//////////////////////////////////////////////////////////////////////////////
// Entis 標準ライブラリ互換用
//////////////////////////////////////////////////////////////////////////////

#if	!defined(_CRTDBG_MAP_ALLOC) && defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif

#if	!defined(__ENTIS_GLS__)

#include <SDKDDKVer.h>
/*
#if	!defined(_WIN32_WINNT)
	#define	_WIN32_WINNT	0x0501
#endif
#if	!defined(STRICT)
	#define	STRICT	1
#endif
#if	!defined(WINVER)
	#define WINVER 0x0501
#endif
*/
#include <windows.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

// INTxx の定義は DirectX SDK 等の basetsd.h のヘッダ参照

typedef	INT8		SBYTE ;		// 詞葉固有型
typedef	INT16		SWORD ;

#if	!defined(__SQLTYPES)
typedef	INT32		SDWORD ;	// 詞葉固有型
#endif

typedef	UINT64		QWORD ;
typedef	INT64 		SQWORD ;	// 詞葉固有型
typedef	float		REAL32 ;	// 詞葉固有型
typedef	double		REAL64 ;	// 詞葉固有型

#endif

#if	defined(__PROCESSOR_INTEL_X86_SSE__)
	#if	__PROCESSOR_INTEL_X86_SSE__ >= 1
		#include <xmmintrin.h>
		#if	__PROCESSOR_INTEL_X86_SSE__ >= 2
			#include <emmintrin.h>
			#include <intrin.h>
		#endif
	#endif
#endif


//////////////////////////////////////////////////////////////////////////////
// 一般 / POSIX 系互換用
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ssize_t)
typedef	ptrdiff_t	ssize_t ;
#endif

#if	defined(__PROCESSOR_INTEL_X86_64__)
typedef	long int	atomic_int_t ;	// 詞葉固有型
#else
typedef	int			atomic_int_t ;	// 詞葉固有型
#endif

typedef	float		float32_t ;		// 詞葉固有型
typedef	double		float64_t ;		// 詞葉固有型

#if	!defined(_NODEF_EXACT_WIDTH_INT_IN_ESL_)
typedef	UINT8		uint8_t ;
typedef	INT8		int8_t ;
typedef	UINT16		uint16_t ;
typedef	INT16		int16_t ;
typedef	UINT32		uint32_t ;
typedef	INT32		int32_t ;
typedef	UINT64		uint64_t ;
typedef	INT64		int64_t ;
#endif

typedef	unsigned int	uint_t ;
typedef	unsigned long	ulong_t ;

typedef	ULONG_PTR	ulong_ptr_t ;
typedef	LONG_PTR	long_ptr_t ;
typedef	UINT_PTR	uint_ptr_t ;
typedef	INT_PTR		int_ptr_t ;

#if	!defined(offsetof)
#define	offsetof(T,M)	((size_t)&(((T*)0)->M))
#endif


//////////////////////////////////////////////////////////////////////////////
// スクリプト用関数エクスポート宣言
//////////////////////////////////////////////////////////////////////////////

#if	!defined(EXTERN_C)
#define	EXTERN_C	extern "C"
#endif

#if	!defined(ESL_DLL_EXPORT)
	#if	defined(ENTISGLS4_DLL_IMPORT)
		#define	ESL_DLL_EXPORT	__declspec( dllimport )
		#define	ESL_DLL_DECL(x)
	#else
		#define	ESL_DLL_EXPORT	__declspec( dllexport )
		#define	ESL_DLL_DECL(x)	ESL_DLL_EXPORT x
	#endif
#endif

#if	!defined(ECS_EXPORT)
	#define	ECS_EXPORT	extern "C" __declspec( dllexport )
	#define	ECS_IMPORT	extern "C" __declspec( dllimport )

	#if	defined(ENTISGLS4_DLL_IMPORT)
		#define	ECS_LIB_EXPORT	ECS_IMPORT
	#elif	defined(DISABLE_ENTIS_GLS4_EXPORTS)
		#define	ECS_LIB_EXPORT	extern "C"
	#else
		#define	ECS_LIB_EXPORT	ECS_EXPORT
	#endif
#endif


//////////////////////////////////////////////////////////////////////////////
// ESLs 関数
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__ENTIS_GLS__)

#define	eslFillMemory	memset
#define	eslMoveMemory	memmove

#endif

#define	eslCompareMemory	memcmp
#define	eslCopyMemory		memcpy

inline INT64 eslRoundR64ToLInt( REAL64 r )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_mm_cvtsd_si64( _mm_set_sd( r ) ) ;
#else
	INT64	n ;
	__asm
	{
		fld		r
		fistp	n
	}
	return	n ;
#endif
}

inline int eslRoundR32ToInt( REAL32 r )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_mm_cvt_ss2si( _mm_set_ss( r ) ) ;
#else
	int		n ;
	__asm
	{
		fld		r
		fistp	n
	}
	return	n ;
#endif
}

inline int64_t esl_lroundfi( double r )
{
	return	eslRoundR64ToLInt( r ) ;
}

inline int esl_roundfi( float32_t r )
{
	return	eslRoundR32ToInt( r ) ;
}

inline int esl_abs( int v )
{
	return	(v < 0) ? -v : v ;
}

inline int esl_clampi( int v, int low, int high )
{
	v -= low ;
	high -= low ;
	if ( (unsigned int) v >= (unsigned int) high )
	{
		v = high & ~(v >> (sizeof(int) * 8 - 1)) ;
	}
	return	v + low ;
}

inline int esl_min( int a, int b )
{
	return	(a <= b) ? a : b ;
}

inline int esl_max( int a, int b )
{
	return	(a >= b) ? a : b ;
}

inline double esl_fclamp( double v, double low, double high )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 2)
	double	r ;
	_mm_store_sd
		( &r, _mm_min_sd
				( _mm_load_sd( &high ), _mm_max_sd
						( _mm_load_sd( &low ), _mm_load_sd( &v ) ) ) ) ;
	return	r ;
#else
	return	(v <= low) ? low : ((v >= high) ? high : v) ;
#endif
}

inline double esl_fmin( double a, double b )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 2)
	double	r ;
	_mm_store_sd
		( &r, _mm_min_sd( _mm_load_sd( &a ), _mm_load_sd( &b ) ) ) ;
	return	r ;
#else
	return	(a <= b) ? a : b ;
#endif
}

inline double esl_fmax( double a, double b )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 2)
	double	r ;
	_mm_store_sd
		( &r, _mm_max_sd( _mm_load_sd( &a ), _mm_load_sd( &b ) ) ) ;
	return	r ;
#else
	return	(a >= b) ? a : b ;
#endif
}

inline float esl_fclampf( float v, float low, float high )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 1)
	float	r ;
	_mm_store_ss
		( &r, _mm_min_ss
				( _mm_load_ss( &high ), _mm_max_ss
						( _mm_load_ss( &low ), _mm_load_ss( &v ) ) ) ) ;
	return	r ;
#else
	return	(v <= low) ? low : ((v >= high) ? high : v) ;
#endif
}

inline float esl_fminf( float a, float b )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 1)
	float	r ;
	_mm_store_ss
		( &r, _mm_min_ss( _mm_load_ss( &a ), _mm_load_ss( &b ) ) ) ;
	return	r ;
#else
	return	(a <= b) ? a : b ;
#endif
}

inline float esl_fmaxf( float a, float b )
{
#if	defined(__PROCESSOR_INTEL_X86_SSE__) && (__PROCESSOR_INTEL_X86_SSE__ >= 1)
	float	r ;
	_mm_store_ss
		( &r, _mm_max_ss( _mm_load_ss( &a ), _mm_load_ss( &b ) ) ) ;
	return	r ;
#else
	return	(a >= b) ? a : b ;
#endif
}

#include <esl/esl_object.h>


//////////////////////////////////////////////////////////////////////////////
// デバッグサポート関数
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__ENTIS_GLS__)
#if	defined(__DEBUG__)

#define	ESLTrace	SSystem::Trace
#define	ESLVerify(expr)		\
	if ( !(expr) )	SSystem::Assert( #expr, __FILE__, __LINE__ )
#define	ESLAssert(expr)		\
	if ( !(expr) )	SSystem::Assert( #expr, __FILE__, __LINE__ )

#else

inline void ESLTrace( const char * pszTrace, ... ) { }

#define	ESLVerify(expr)	((void)(expr))
#define	ESLAssert(expr)	((void)0)

#endif
#endif


//////////////////////////////////////////////////////////////////////////////
// 共通エラーコード
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__ENTIS_GLS__)

enum	ESLError
{
	eslErrSuccess		= 0,
	eslErrNotSupported	= -1,
	eslErrFailed		= 1,
	eslErrAbort			= 2,
	eslErrInvalidParam	= 3,
	eslErrTimeout		= 4,
	eslErrPending		= 5,
	eslErrContinue		= 6,
	eslErrDummy			= 0xFFFFFFFF,
} ;

inline ESLError ESLErrorMsg( const char * pszMsg )
{
	return	(ESLError) ((ulong_ptr_t) pszMsg) ;
}

const char * GetESLErrorMsg( ESLError err ) ;

#endif



#endif


