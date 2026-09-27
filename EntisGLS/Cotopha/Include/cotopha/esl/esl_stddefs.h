
#if	!defined(__ESL_STD_DEFS_H__)
#define	__ESL_STD_DEFS_H__

//////////////////////////////////////////////////////////////////////////////
// Windows / Entis 標準ライブラリ互換用
//////////////////////////////////////////////////////////////////////////////

typedef	uint8		BYTE ;
typedef	uint16		WORD ;
typedef	uint32		DWORD ;
typedef	uint64		QWORD ;
typedef	uint64		UINT64 ;
typedef	int64		INT64 ;

typedef	uint8*		PBYTE ;

typedef	signed int			INT ;
typedef	signed int *		PINT ;
typedef	unsigned int		UINT ;
typedef	signed long int		LONG ;
typedef	signed long int *	PLONG ;
typedef	unsigned long int	ULONG ;

typedef	uint64		DWORD_PTR ;
typedef	uint64		ULONG_PTR ;
typedef	int64		LONG_PTR ;
typedef	uint64		UINT_PTR ;
typedef	int64		INT_PTR ;


//////////////////////////////////////////////////////////////////////////////
// Entis 標準ライブラリ互換用
//////////////////////////////////////////////////////////////////////////////

typedef	int8		SBYTE ;
typedef	int16		SWORD ;
typedef	int32		SDWORD ;
typedef	int64		SQWORD ;
typedef	float		REAL32 ;
typedef	double		REAL64 ;


//////////////////////////////////////////////////////////////////////////////
// MS-C++ 互換用
//////////////////////////////////////////////////////////////////////////////

typedef	uint8		__uint8 ;
typedef	int8		__int8 ;
typedef	uint16		__uint16 ;
typedef	int16		__int16 ;
typedef	uint32		__uint32 ;
typedef	int32		__int32 ;
typedef	uint64		__uint64 ;
typedef	int64		__int64 ;


//////////////////////////////////////////////////////////////////////////////
// 一般 / POSIX 系互換用
//////////////////////////////////////////////////////////////////////////////

typedef	uint16		wchar_t ;
typedef	uint32		size_t ;
typedef	int32		ssize_t ;
typedef	int64		atomic_int_t ;	// 詞葉固有型
typedef	float		float32_t ;		// 詞葉固有型
typedef	double		float64_t ;		// 詞葉固有型

typedef	uint8		uint8_t ;
typedef	int8		int8_t ;
typedef	uint16		uint16_t ;
typedef	int16		int16_t ;
typedef	uint32		uint32_t ;
typedef	int32		int32_t ;
typedef	uint64		uint64_t ;
typedef	int64		int64_t ;

typedef	unsigned int	uint_t ;
typedef	unsigned long	ulong_t ;


typedef	uint64		ulong_ptr_t ;
typedef	int64		long_ptr_t ;
typedef	uint64		uint_ptr_t ;
typedef	int64		int_ptr_t ;


#if	!defined(NULL)
#define	NULL	0
#endif

#if	!defined(offsetof)
#define	offsetof(T,M)	((size_t)&(((T*)0)->M))
#endif

typedef	int64*	va_list ;

#define va_start(ap,v)  (ap = (va_list)&v + 1)
#define va_arg(ap,t)    (*((t*)(ap ++)))
#define va_end(ap)      (ap = (va_list)0)



//////////////////////////////////////////////////////////////////////////////
// ESLs 関数
//////////////////////////////////////////////////////////////////////////////

// 名前スコープを伴わないので記述に注意
#define	eslFillMemory	memset
#define	eslMoveMemory	memmove

#define	eslRoundR64ToLInt(x)	((int64)((double)x))
#define	eslRoundR32ToInt(x)		((int)((float)x))


//////////////////////////////////////////////////////////////////////////////
// デバッグサポート関数
//////////////////////////////////////////////////////////////////////////////

#define	EXTERN_C

#if	defined(__DEBUG__)

#define	ESLTrace	SSystem::Trace
#define	ESLVerify(expr)		\
	if ( !(expr) )	SSystem::Assert( #expr, __FILE__, __LINE__ )
#define	ESLAssert(expr)		\
	if ( !(expr) )	SSystem::Assert( #expr, __FILE__, __LINE__ )

#else

inline void ESLTrace( const char * pszTrace, ... ) { }

#define	ESLVerify(expr)	(expr)
#define	ESLAssert(expr)	((void)0)

#endif


//////////////////////////////////////////////////////////////////////////////
// 共通エラーコード
//////////////////////////////////////////////////////////////////////////////

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
	eslErrDummy			= 0xFFFFFFFFFFFFFFFF,
} ;

inline ESLError ESLErrorMsg( const char * pszMsg )
{
	return	(ESLError) (ulong_ptr_t) pszMsg ;
}

const char * GetESLErrorMsg( ESLError err ) ;


//////////////////////////////////////////////////////////////////////////////
// メモリ関数
//////////////////////////////////////////////////////////////////////////////

native void * memmove( void * dst, const void * src, size_t bytes ) naked ;
native void * memset( void * dst, int c, size_t bytes ) naked ;
native void * malloc( size_t bytes ) naked ;
native void * shared_malloc( size_t bytes ) naked ;
native void * realloc( void * memblock, size_t bytes ) naked ;
native void free( void * memblock ) naked ;

native int sprintf_s( char * buffer, size_t size, const char * format, ... ) ;
native int vsprintf_s( char * buffer, size_t size, const char * format, va_list argptr ) ;


//////////////////////////////////////////////////////////////////////////////
// 算術関数
//////////////////////////////////////////////////////////////////////////////

native double fabs( double x ) naked ;
native double log( double x ) naked ;
native double log10( double x ) naked ;
native double pow( double x, double y ) naked ;
native double sqrt( double x ) naked ;
native double sin( double x ) naked ;
native double cos( double x ) naked ;
native double tan( double x ) naked ;
native double asin( double x ) naked ;
native double acos( double x ) naked ;
native double atan( double x ) naked ;
native double atan2( double x, double y ) naked ;
native int64_t round( double x ) naked ;
native int64_t floor( double x ) naked ;

#define	min(x,y)	(((x)<(y))?(x):(y))
#define	max(x,y)	(((x)>(y))?(x):(y))
#define	esl_fmin(x,y)	(((x)<(y))?(double)(x):(double)(y))
#define	esl_fmax(x,y)	(((x)>(y))?(double)(x):(double)(y))
#define	esl_fminf(x,y)	(((x)<(y))?(float)(x):(float)(y))
#define	esl_fmaxf(x,y)	(((x)>(y))?(float)(x):(float)(y))

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



//////////////////////////////////////////////////////////////////////////////
// ESL 基底オブジェクト
//////////////////////////////////////////////////////////////////////////////

#include <esl/esl_object.h>


//////////////////////////////////////////////////////////////////////////////
// スクリプト用関数エクスポート宣言
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ECS_EXPORT)
#define	ECS_EXPORT	extern "COTOPHA"
#endif

#define	__fastcall


#endif


