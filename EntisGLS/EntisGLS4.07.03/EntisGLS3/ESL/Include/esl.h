
/*****************************************************************************
                    Entis Standard Library declarations
 ----------------------------------------------------------------------------
        Copyright (c) 2002-2014 Leshade Entis. All rights reserved.
 *****************************************************************************/


#if	!defined(__ESL_H__)
#define	__ESL_H__

#if	!defined(GLSEXPORT)
	#if	defined(_IMPORT_GLS)
		#if	!defined(ENTISGLS4_DLL_IMPORT)
			#define	ENTISGLS4_DLL_IMPORT	1
		#endif
		#define	GLSEXPORT	__declspec(dllimport)
		#define	ESL_DLL_DECL(x)
	#else
		#define	GLSEXPORT	__declspec(dllexport)
		#define	ESL_DLL_DECL(x)	GLSEXPORT x
	#endif
	#define	ESL_DLL_EXPORT	GLSEXPORT
#endif

#if	!defined(__ENTIS_GLS__)
#define	__ENTIS_GLS__	1
#endif


//////////////////////////////////////////////////////////////////////////////
// デバッグサポート関数
//////////////////////////////////////////////////////////////////////////////

typedef	void (*PFUNC_OnExceptionDump)( class ESLFileObject * pfile ) ;

extern "C"
{
	void _ESLTrace( const char * pszTrace, ... ) ;
	void ESLOpenTraceOutputFile( const char * pszTraceFile ) ;
	PFUNC_OnExceptionDump eslGetFunctionExceptionDump( void ) ;
	void eslSetFunctionExceptionDump( PFUNC_OnExceptionDump pfnDump ) ;
}

#if	defined(_DEBUG)

extern "C"
{
	void _ESLAssert
		( const char * pszExpr, const char * pszFile, int nLine ) ;
}

#define	ESLTrace	_ESLTrace
#define	ESLVerify(expr)		\
	if ( !(expr) )	_ESLAssert( #expr, __FILE__, __LINE__ )
#define	ESLAssert(expr)		\
	if ( !(expr) )	_ESLAssert( #expr, __FILE__, __LINE__ )

#else

#if	!defined(_OUTPUT_TRACE)
	inline void ESLTrace( const char * pszTrace, ... ) { }
#else
	#define	ESLTrace	_ESLTrace
#endif

#define	ESLVerify(expr)	((void)(expr))
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
	eslErrGeneral		= 1,
	eslErrAbort			= 2,
	eslErrInvalidParam	= 3,
	eslErrTimeout		= 4,
	eslErrPending		= 5,
	eslErrContinue		= 6,
	eslErrDummy			= 0xFFFFFFFF
} ;

inline ESLError ESLErrorMsg( const char * pszMsg )
{
	return	(ESLError) (LONG_PTR) pszMsg ;
}

const char * GetESLErrorMsg( ESLError err ) ;


//////////////////////////////////////////////////////////////////////////////
// ヒープ関数
//////////////////////////////////////////////////////////////////////////////

#include	<eslheap.h>

#if	!defined(_MFC_VER) && !defined(_DISABLE_ESL_NEW)

void * operator new ( size_t stObj ) ;
// void * operator new ( size_t stObj, void * ptrObj ) ;
#if	defined(_DEBUG)
void * operator new ( size_t stObj, const char * pszFileName, int nLine ) ;
#endif
void operator delete ( void * ptrObj ) ;

#endif


//////////////////////////////////////////////////////////////////////////////
// 基底クラス
//////////////////////////////////////////////////////////////////////////////

#include <esl/esl_object.h>

#define	DECLARE_CLASS_INFO(class_name, parent_class)	\
			ESL_DECLARE_CLASS_INFO(class_name, parent_class)
#define	IMPLEMENT_CLASS_INFO(class_name, parent_class)	\
			ESL_IMPLEMENT_CLASS_INFO(class_name, parent_class)
#define	DECLARE_CLASS_INFO2(class_name, parent_class1, parent_class2)	\
			ESL_DECLARE_CLASS_INFO2(class_name, parent_class1, parent_class2)
#define	IMPLEMENT_CLASS_INFO2(class_name, parent_class1, parent_class2)	\
			ESL_IMPLEMENT_CLASS_INFO2(class_name, parent_class1, parent_class2)



//////////////////////////////////////////////////////////////////////////////
// Entis Library 低水準クラスライブラリ
//////////////////////////////////////////////////////////////////////////////

template <class _Type, class _Obj> class	EGenString ;
	class	EString ;
	class	EWideString ;
		class	EStreamWideString ;

class	EPtrBuffer ;
class	EStreamBuffer ;

class	EPtrArray ;
	template <class> class	ENumArray ;
	template <class> class	EPtrObjArray ;
		template <class> class	EObjArray ;
			template <class TagType, class ObjType> class	ETagSortArray ;
				template <class>	class	EIntTagArray ;
				template <class>	class	EStrTagArray ;
				template <class>	class	EWStrTagArray ;
template <class TagType, class ObjType> class	ETaggedElement ;

class	ESLFileObject ;
	class	ERawFile ;
	class	EMemoryFile ;
	class	EStreamFileBuffer ;
	class	ESyncStreamFile ;

class	EDescription ;

#include	<eslthread.h>
#include	<eslarray.h>
#include	<eslstring.h>
#include	<eslfile.h>
#include	<esldesc.h>


#endif

