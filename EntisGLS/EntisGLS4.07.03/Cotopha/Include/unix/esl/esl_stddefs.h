
#if	!defined(__ESL_STD_DEFS_H__)
#define	__ESL_STD_DEFS_H__

#if	ANDROID_NDK_VER >= 15
	#include <stdatomic.h>
#else
	#if	defined(__POINTER64__)
	#include <stdatomic.h>
	#else
	#include <sys/atomics.h>
	#endif
#endif

#include <unistd.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#if	defined(__PLATFORM_ANDROID__)
#include <android/log.h>
#include <jni.h>
extern JavaVM *	g_JavaVM ;
#endif

#if	!defined(__stdcall)
#define	__stdcall
#endif

#if	!defined(__fastcall)
#define	__fastcall
#endif


//////////////////////////////////////////////////////////////////////////////
// Entis 標準ライブラリ互換用
//////////////////////////////////////////////////////////////////////////////

typedef	void *					PVOID ;
typedef	uint8_t					BYTE, * PBYTE ;
typedef	uint16_t				WORD, * PWORD ;
typedef	uint32_t				DWORD ;
typedef	signed int				INT, * PINT ;
typedef	unsigned int			UINT ;
typedef	signed long int			LONG, * PLONG ;
typedef	unsigned long int		ULONG ;

typedef	int8_t					SBYTE ;
typedef	int16_t					SWORD ;
typedef	int32_t					SDWORD ;
typedef	float					REAL32 ;
typedef	double					REAL64 ;

typedef	BYTE					UINT8 ;
typedef	SBYTE					INT8 ;
typedef	WORD					UINT16 ;
typedef	SWORD					INT16 ;
typedef	DWORD					UINT32 ;
typedef	SDWORD					INT32 ;
typedef long long				INT64 ;
typedef unsigned long long		UINT64 ;


#if	!defined(NULL)
#define	NULL	0
#endif

#if	defined(__POINTER64__)

#if	!defined(DWORD_PTR)
typedef	UINT64		DWORD_PTR ;
#endif

#if	!defined(ULONG_PTR)
typedef	UINT64		ULONG_PTR ;
#endif

#if	!defined(LONG_PTR)
typedef	INT64		LONG_PTR ;
#endif

#if	!defined(UINT_PTR)
typedef	UINT64		UINT_PTR ;
#endif

#if	!defined(INT_PTR)
typedef	INT64		INT_PTR ;
#endif

#else

#if	!defined(DWORD_PTR)
typedef	unsigned long		DWORD_PTR ;
#endif

#if	!defined(ULONG_PTR)
typedef	unsigned long		ULONG_PTR ;
#endif

#if	!defined(LONG_PTR)
typedef	long				LONG_PTR ;
#endif

#if	!defined(UINT_PTR)
typedef	unsigned int		UINT_PTR ;
#endif

#if	!defined(INT_PTR)
typedef	int					INT_PTR ;
#endif

#endif


//////////////////////////////////////////////////////////////////////////////
// 一般 / POSIX 系互換用
//////////////////////////////////////////////////////////////////////////////

typedef	int			atomic_int_t ;	// 詞葉固有型

typedef	float		float32_t ;		// 詞葉固有型
typedef	double		float64_t ;		// 詞葉固有型

/*
typedef	UINT8		uint8_t ;
typedef	INT8		int8_t ;
typedef	UINT16		uint16_t ;
typedef	INT16		int16_t ;
typedef	UINT32		uint32_t ;
typedef	INT32		int32_t ;
typedef	UINT64		uint64_t ;
typedef	INT64		int64_t ;
*/

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

#define	EXTERN_C	extern "C"

#if	!defined(ESL_DLL_EXPORT)
	#define	ESL_DLL_EXPORT
	#define	ESL_DLL_EXTERN extern
	#define	ESL_DLL_DECL(x)	x
#endif

#if	!defined(ECS_EXPORT)
	#define	ECS_LIB_EXPORT	extern "C"
	#define	ECS_IMPORT	extern "C"
	#define	ECS_EXPORT	extern "C"
#endif


//////////////////////////////////////////////////////////////////////////////
// ESLs 関数
//////////////////////////////////////////////////////////////////////////////

// 名前スコープを伴わないので記述に注意
#define	eslFillMemory		memset
#define	eslMoveMemory		memmove
#define	eslCompareMemory	memcmp
#define	eslCopyMemory		memcpy

#define	eslRoundR64ToLInt	llround
#define	eslRoundR32ToInt	lroundf
#define	esl_lroundfi		llround
#define	esl_roundfi			lroundf

#define	esl_fmin	fmin
#define	esl_fmax	fmax
#define	esl_fminf	fminf
#define	esl_fmaxf	fmaxf

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
		v = high & ~(v >> 31) ;
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

inline double esl_fclamp( double v, double low, double hight )
{
	return	(v <= low) ? low : ((v >= hight) ? hight : v) ;
}

inline float esl_fclampf( float v, float low, float hight )
{
	return	(v <= low) ? low : ((v >= hight) ? hight : v) ;
}



//////////////////////////////////////////////////////////////////////////////
// デバッグサポート関数
//////////////////////////////////////////////////////////////////////////////


#if	!defined(__ENTIS_GLS__)
#if	defined(__DEBUG__)

#if	defined(__PLATFORM_ANDROID__)
	#define	ESLTrace(...)	__android_log_print(ANDROID_LOG_DEBUG,"EntisGLS",__VA_ARGS__)
#else
	#define	ESLTrace	SSystem::Trace
#endif

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
	return	(ESLError) (ulong_ptr_t) pszMsg ;
}

const char * GetESLErrorMsg( ESLError err ) ;

#endif


//////////////////////////////////////////////////////////////////////////////
// ESL 基底オブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__ENTIS_GLS__)

#include <esl/esl_object.h>

#endif


#endif


