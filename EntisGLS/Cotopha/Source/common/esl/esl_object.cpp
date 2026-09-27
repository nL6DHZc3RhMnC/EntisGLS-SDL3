
#include <sakura/sakura.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////////
// 基底クラス
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)
ESL_DLL_EXPORT const ESLRuntimeClass	ESLObject::m_RuntimeClass = { "ESLObject", NULL } ;
#endif

#if	defined(__DEBUG__)
// 構築関数
ESLObject::ESLObject( void )
{
}

ESLObject::ESLObject( const ESLObject& obj )
{
}

// 消滅関数
ESLObject::~ESLObject( void )
{
	ESLAssert( *((void**)this) != NULL ) ;
	*((void**)this) = NULL ;
}

// 有効判定
bool ESLObject::IsValidESLObject( void ) const
{
	ESLAssert( this != NULL ) ;
	if ( this != NULL )
	{
		#if	defined(_MSC_VER)
		__try
		{
			if ( *((void**)this) != NULL )
			{
				return	(GetESLRuntimeClass() != NULL) ;
			}
		}
		__except( EXCEPTION_CONTINUE_EXECUTION )
		{
		}
		#else
			if ( *((void**)this) != NULL )
			{
				return	(GetESLRuntimeClass() != NULL) ;
			}
		#endif
	}
	return	false ;
}

#endif

// ランタイムクラス情報取得
const ESLRuntimeClass * ESLObject::GetESLRuntimeClass( void ) const
{
	return	&(ESLObject::m_RuntimeClass) ;
}

// クラス名取得
const char * ESLObject::GetESLClassName( void ) const
{
	return	ESLObject::m_RuntimeClass.pszClassName ;
}

// クラス判定
bool ESLObject::IsKindOf( const ESLRuntimeClass & rtClass ) const
{
	return	(&rtClass == &ESLObject::m_RuntimeClass) ;
}

// 動的変換
void * ESLObject::DynamicCast( const ESLRuntimeClass & rtClass ) const
{
	if ( &rtClass == &ESLObject::m_RuntimeClass )
	{
		return	(void*) this ;
	}
	return	NULL ;
}

// 同一クラス判定
bool ESLObject::IsSameClassAs( ESLObject * pObj ) const
{
	ESLAssert( this != NULL ) ;
	if ( pObj != NULL )
	{
		return	(GetESLClassName() == pObj->GetESLClassName()) ;
	}
	return	(this == pObj) ;
}

// 派生クラス動的判定
bool ESLObject::IsKindOfDynamically
		( ESLObject * pObj, const char * pszClassName )
{
	if ( (pObj == NULL) | (pszClassName == NULL) )
	{
		return	false ;
	}
	#if	defined(__DEBUG__)
	if ( !pObj->IsValidESLObject() )
	{
		return	false ;
	}
	#endif
	const ESLRuntimeClass *	pRTC = pObj->GetESLRuntimeClass() ;
	ESLAssert( pRTC != NULL ) ;
	while ( pRTC != NULL )
	{
		const char *	pszThisClass = pRTC->pszClassName ;
		size_t	i ;
		for ( i = 0; pszThisClass[i]; i ++ )
		{
			if ( pszThisClass[i] != pszClassName[i] )
			{
				break ;
			}
		}
		if ( pszThisClass[i] == pszClassName[i] )
		{
			return	true ;
		}
		pRTC = pRTC->pParentClass ;
	}
	return	false ;
}

bool ESLObject::IsKindOfDynamically( const char * pszClassName ) const
{
	if ( pszClassName == NULL )
	{
		return	false ;
	}
	#if	defined(__DEBUG__)
	if ( !IsValidESLObject() )
	{
		return	false ;
	}
	#endif
	const ESLRuntimeClass *	pRTC = GetESLRuntimeClass() ;
	ESLAssert( pRTC != NULL ) ;
	while ( pRTC != NULL )
	{
		const char *	pszThisClass = pRTC->pszClassName ;
		size_t	i ;
		for ( i = 0; pszThisClass[i]; i ++ )
		{
			if ( pszThisClass[i] != pszClassName[i] )
			{
				break ;
			}
		}
		if ( pszThisClass[i] == pszClassName[i] )
		{
			return	true ;
		}
		pRTC = pRTC->pParentClass ;
	}
	return	false ;
}


#if	defined(__ENTIS_GLS__)

void * ESLObject::operator new ( size_t stObj )
{
	return	::eslHeapAllocate( NULL, (DWORD) stObj, 0 ) ;
}

void * ESLObject::operator new ( size_t stObj, void * ptrObj )
{
	return	ptrObj ;
}

void * ESLObject::operator new
	( size_t stObj, const char * pszFileName, int nLine )
{
	return	::eslHeapAllocate( NULL, (DWORD) stObj, 0 ) ;
}

void ESLObject::operator delete ( void * ptrObj )
{
	::eslHeapFree( NULL, ptrObj, 0 ) ;
}

#elif !defined(ENTISGLS4_DLL_IMPORT)

ESL_DLL_DECL(void *) ESLObject::operator new ( size_t stObj )
{
//	return	::malloc( stObj ) ;
	return	esl_malloc( stObj ) ;
}

ESL_DLL_DECL(void *) ESLObject::operator new ( size_t stObj, void * ptrObj )
{
	return	ptrObj ;
}

ESL_DLL_DECL(void *) ESLObject::operator new
	( size_t stObj, const char * pszFileName, int nLine )
{
//	return	::malloc( stObj ) ;
	return	esl_malloc( stObj ) ;
}

ESL_DLL_DECL(void) ESLObject::operator delete ( void * ptrObj )
{
//	::free( ptrObj ) ;
	esl_free( ptrObj ) ;
}

#endif

