
#if	!defined(__ESL_OBJECT_H__)
#define	__ESL_OBJECT_H__

#if	!defined(__SAKURA_CPP_PRESETS_H__)
#include <sakura/sakura_cpp_presets.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// 実行時型チェック用
//////////////////////////////////////////////////////////////////////////////

class	ESLObject ;

struct	ESLRuntimeClass
{
	const char *			pszClassName ;	// ただの static な char 配列へのポインタ
	const ESLRuntimeClass *	pParentClass ;

	typedef	ESLObject* (*PFUNC_NEW_OBJECT)( void ) ;
	PFUNC_NEW_OBJECT		pfnNewObject ;	// NULL の場合もあり
} ;


//////////////////////////////////////////////////////////////////////////////
// 基底クラス
//////////////////////////////////////////////////////////////////////////////

class	ESLObject
{
public:
	static ESL_DLL_EXPORT const ESLRuntimeClass	m_RuntimeClass ;
	typedef	ESLObject *	ESLClassPointer ;

	#if	defined(__DEBUG__)
	// 構築関数
	ESLObject( void ) ;
	ESLObject( const ESLObject& obj ) ;
	// 消滅関数
	virtual ~ESLObject( void ) ;
	// 有効判定
	bool IsValidESLObject( void ) const ;

	#else

	// 構築関数
	ESLObject( void ) { }
	ESLObject( const ESLObject& obj ) { }
	// 消滅関数
	virtual ~ESLObject( void ) { }

	#endif

	// ランタイムクラス情報取得
	virtual const ESLRuntimeClass * GetESLRuntimeClass( void ) const ;
	// クラス名取得
	virtual const char * GetESLClassName( void ) const ;
	// クラス判定
	virtual bool IsKindOf( const ESLRuntimeClass & rtClass ) const ;
	// 動的変換
	virtual void * DynamicCast( const ESLRuntimeClass & rtClass ) const ;
	// 同一クラス判定
	bool IsSameClassAs( ESLObject * pObj ) const ;
	// 派生クラス動的判定
	static bool IsKindOfDynamically
		( ESLObject * pObj, const char * pszClassName ) ;
	bool IsKindOfDynamically( const char * pszClassName ) const ;
	// キャストの曖昧性排除
	static ESLObject * GetESLPointer( ESLObject * pObj ) { return pObj ; }

	// new/delete オーバーロード
	//（主に DLL 側と EXE でアロケーションを統一するため）
	static ESL_DLL_EXPORT void * operator new ( size_t stObj ) ;
	static ESL_DLL_EXPORT void * operator new ( size_t stObj, void * ptrObj ) ;
	static ESL_DLL_EXPORT void * operator new ( size_t stObj, const char * pszFileName, int nLine ) ;
	static ESL_DLL_EXPORT void operator delete ( void * ptrObj ) ;

} ;

// 動的型キャスト
#if	!defined(__COTOPHA__)
template <class T1, class T2> inline const T1 * ESLTypeCast( const T2 * pObj )
{
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	return	(typename T1::ConstClassPointerType)
				(pObj->DynamicCast( T1::m_RuntimeClass )) ;
}
#endif

#if	!defined(__COTOPHA__)
template <class T1, class T2> inline const T1 * ESLConstTypeCast( const T2 * pObj )
#else
template <class T1> inline const T1 * ESLConstTypeCast( const ESLObject * pObj )
#endif
{
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	return	(typename T1::ConstClassPointerType)
				(pObj->DynamicCast( T1::m_RuntimeClass )) ;
}

#if	!defined(__COTOPHA__)
template <class T1, class T2> inline T1 * ESLTypeCast( T2 * pObj )
#else
template <class T1> inline T1 * ESLTypeCast( ESLObject * pObj )
#endif
{
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	return	(typename T1::ClassPointerType)
				(pObj->DynamicCast( T1::m_RuntimeClass )) ;
}

// 動的型キャスト（失敗した場合には delete し NULL を返す）
template <class T> inline T * ESLSmartCast( ESLObject * pObj )
{
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	T *	pT = (typename T::ClassPointerType)
				(pObj->DynamicCast( T::m_RuntimeClass )) ;
	if ( pT == nullptr )
	{
		delete	pObj ;
	}
	return	pT ;
}

#define	ESL_RUNTIME_CLASS(T)	(T::m_RuntimeClass)

#define	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class )	\
	void * operator new ( size_t stObj )								\
		{	return 	parent_class::operator new ( stObj ) ;	}			\
	void * operator new ( size_t stObj, void * ptrObj )					\
		{	return 	parent_class::operator new ( stObj, ptrObj ) ;	}	\
	void * operator new ( size_t stObj, const char * pszFileName, int nLine )	\
		{	return 	parent_class::operator new ( stObj, pszFileName, nLine ) ;	}	\
	void operator delete ( void * ptrObj )								\
		{	parent_class::operator delete ( ptrObj ) ;	}
#define	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( parent_class )	\
	void * operator new ( size_t stObj )								\
		{	return 	ESLObject::operator new ( stObj ) ;	}			\
	void * operator new ( size_t stObj, void * ptrObj )					\
		{	return 	ESLObject::operator new ( stObj, ptrObj ) ;	}	\
	void * operator new ( size_t stObj, const char * pszFileName, int nLine )	\
		{	return 	ESLObject::operator new ( stObj, pszFileName, nLine ) ;	}	\
	void operator delete ( void * ptrObj )								\
		{	ESLObject::operator delete ( ptrObj ) ;	}

#define	ESL_DECLARE_CLASS_INFO_OPT( export_opt, virtual_opt, class_name, parent_class )				\
	public:	static export_opt const ESLRuntimeClass	m_RuntimeClass ;			\
	typedef	class_name *	ClassPointerType ;							\
	typedef	const class_name *	ConstClassPointerType ;					\
	virtual_opt const ESLRuntimeClass * GetESLRuntimeClass( void ) const ;	\
	virtual_opt const char * GetESLClassName( void ) const ;					\
	virtual_opt bool IsKindOf( const ESLRuntimeClass & rtClass ) const ;	\
	virtual_opt void * DynamicCast( const ESLRuntimeClass & rtClass ) const ;

#define	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class )				\
	ESL_DECLARE_CLASS_INFO_OPT( ESL_DLL_EXPORT, virtual, class_name, parent_class )

#define	ESL_DECLARE_CLASS_INFO_THIS_ECP( class_name, parent_class )				\
	typedef	class_name*	ESLClassPointer ;	\
	static class_name * GetESLPointer( class_name * pObj ) { return pObj ; } \
	ESL_DECLARE_CLASS_INFO_OPT( ESL_DLL_EXPORT, virtual, class_name, parent_class )

#define	ESL_DECLARE_CLASS_INFO( class_name, parent_class )				\
	typedef	parent_class::ESLClassPointer	ESLClassPointer ;	\
	ESL_DECLARE_CLASS_INFO_OPT( ESL_DLL_EXPORT, virtual, class_name, parent_class )

#define	ESL_DECLARE_NV_CLASS_INFO( class_name )							\
	typedef	class_name*	ESLClassPointer ;	\
	static class_name * GetESLPointer( class_name * pObj ) { return pObj ; } \
	ESL_DECLARE_CLASS_INFO_OPT( ESL_DLL_EXPORT, , class_name, parent_class )

#define	ESL_DLL_DECLARE_CLASS_INFO( class_name, parent_class )					\
	typedef	parent_class::ESLClassPointer	ESLClassPointer ;	\
	ESL_DECLARE_CLASS_INFO_OPT( , , class_name, parent_class )

#define	ESL_DLL_DECLARE_CLASS_INFO2( class_name, parent_class1, parent_class2 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_OPT( , , class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DLL_DECLARE_CLASS_INFO3( class_name, parent_class1, parent_class2, parent_class3 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_OPT( , , class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DECLARE_CLASS_INFO2( class_name, parent_class1, parent_class2 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DECLARE_CLASS_INFO2_NONEW( class_name, parent_class1, parent_class2 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )

#define	ESL_DECLARE_CLASS_INFO3( class_name, parent_class1, parent_class2, parent_class3 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DECLARE_CLASS_INFO3_NONEW( class_name, parent_class1, parent_class2, parent_class3 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )

#define	ESL_DECLARE_CLASS_INFO4( class_name, parent_class1, parent_class2, parent_class3, parent_class4 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DECLARE_CLASS_INFO5( class_name, parent_class1, parent_class2, parent_class3, parent_class4, parent_class5 )	\
	typedef	parent_class1::ESLClassPointer	ESLClassPointer ;	\
	static ESLClassPointer GetESLPointer( class_name * pObj ) { return parent_class1::GetESLPointer( (parent_class1*) pObj ) ; } \
	ESL_DECLARE_CLASS_INFO_NPCP( class_name, parent_class1 )	\
	ESL_DECLARE_CLASS_OPERATOR_NEW( parent_class1 )

#define	ESL_DECLARE_CLASS_NEW_OBJECT( class_name )				\
	static class_name* NewESLObject( void ) ;

#define	ESL_IMPLEMENT_CLASS_NEW_OBJECT( class_name )			\
	class_name* class_name::NewESLObject(void) { return new class_name ; }

#if	defined(ENTISGLS4_DLL_IMPORT)
#define	ESL_IMPLEMENT_CLASS_RUNTIME_INSTANCE( class_name, parent_rtc, new_obj_func )
#else
#define	ESL_IMPLEMENT_CLASS_RUNTIME_INSTANCE( class_name, parent_rtc, new_obj_func )	\
	ESL_DLL_EXPORT const ESLRuntimeClass class_name::m_RuntimeClass = { #class_name, parent_rtc, new_obj_func } ;
#endif

#define	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INSTANCE( class_name, parent_rtc, new_obj_func )	\
	const ESLRuntimeClass class_name::m_RuntimeClass = { #class_name, parent_rtc, new_obj_func } ;

#define	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, parent_rtc, new_obj_func )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INSTANCE(class_name, parent_rtc, new_obj_func)	\
	const ESLRuntimeClass * class_name::GetESLRuntimeClass( void ) const	\
		{	return	&(class_name::m_RuntimeClass) ;	}						\
	const char * class_name::GetESLClassName( void ) const						\
		{	return	class_name::m_RuntimeClass.pszClassName ;	}

#define	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, parent_rtc, new_obj_func )	\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INSTANCE(class_name, parent_rtc, new_obj_func)	\
	const ESLRuntimeClass * class_name::GetESLRuntimeClass( void ) const	\
		{	return	&(class_name::m_RuntimeClass) ;	}						\
	const char * class_name::GetESLClassName( void ) const						\
		{	return	class_name::m_RuntimeClass.pszClassName ;	}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST( class_name, parent_class )		\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const		\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			return	parent_class::IsKindOf( rtClass ) ;						\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			return	parent_class::DynamicCast( rtClass ) ;					\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST_OBJ( class_name, parent_class1, cast_member )		\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	(cast_member != NULL) && ((void*)cast_member != (void*)this) && cast_member->IsKindOf( rtClass ) ;	\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( (pCast == NULL) && (cast_member != NULL) && ((void*)cast_member != (void*)this) )	\
				pCast = cast_member->DynamicCast( rtClass ) ;				\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST_OBJ2( class_name, parent_class1, cast_member1, cast_member2 )		\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( (cast_member1 != NULL) && ((void*)cast_member1 != (void*)this) && cast_member1->IsKindOf( rtClass ) )	\
				return	true ;												\
			return	(cast_member2 != NULL) && (void*)cast_member2 != (void*)this) && cast_member2->IsKindOf( rtClass ) ;	\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL ) {											\
				if ( (cast_member1 != NULL) && ((void*)cast_member1 != (void*)this) )	\
					pCast = cast_member1->DynamicCast( rtClass ) ;			\
				if ( (pCast == NULL) && (cast_member2 != NULL) && ((void*)cast_member2 != (void*)this) )	\
					pCast = cast_member2->DynamicCast( rtClass ) ;			\
			}																\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2_OBJ( class_name, parent_class1, parent_class2, cast_member )		\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class2::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	(cast_member != NULL) && ((void*)cast_member != (void*)this) && cast_member->IsKindOf( rtClass ) ;	\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL ) {											\
				pCast = parent_class2::DynamicCast( rtClass ) ;				\
				if ( (pCast == NULL) && (cast_member != NULL) && ((void*)cast_member != (void*)this) ) {	\
					pCast = cast_member->DynamicCast( rtClass ) ; } }		\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2( class_name, parent_class1, parent_class2 )	\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const		\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	parent_class2::IsKindOf( rtClass ) ;					\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL )											\
				pCast = parent_class2::DynamicCast( rtClass ) ;				\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST3( class_name, parent_class1, parent_class2, parent_class3 )	\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class2::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	parent_class3::IsKindOf( rtClass ) ;					\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL ) {											\
				pCast = parent_class2::DynamicCast( rtClass ) ;				\
				if ( pCast == NULL ) {										\
					pCast = parent_class3::DynamicCast( rtClass ) ; } }		\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST4( class_name, parent_class1, parent_class2, parent_class3, parent_class4 )	\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class2::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class3::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	parent_class4::IsKindOf( rtClass ) ;					\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL ) {											\
				pCast = parent_class2::DynamicCast( rtClass ) ;				\
				if ( pCast == NULL ) {										\
					pCast = parent_class3::DynamicCast( rtClass ) ;			\
					if ( pCast == NULL ) {									\
						pCast = parent_class4::DynamicCast( rtClass ) ; } } }	\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST5( class_name, parent_class1, parent_class2, parent_class3, parent_class4, parent_class5 )	\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	true ;												\
			else if ( parent_class1::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class2::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class3::IsKindOf( rtClass ) )					\
				return	true ;												\
			else if ( parent_class4::IsKindOf( rtClass ) )					\
				return	true ;												\
			return	parent_class5::IsKindOf( rtClass ) ;					\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( &rtClass == &class_name::m_RuntimeClass )					\
				return	(void*) this ;										\
			void *	pCast = parent_class1::DynamicCast( rtClass ) ;			\
			if ( pCast == NULL ) {											\
				pCast = parent_class2::DynamicCast( rtClass ) ;				\
				if ( pCast == NULL ) {										\
					pCast = parent_class3::DynamicCast( rtClass ) ;			\
					if ( pCast == NULL ) {									\
						pCast = parent_class4::DynamicCast( rtClass ) ;		\
						if ( pCast == NULL ) {								\
							pCast = parent_class5::DynamicCast( rtClass ) ; } } } }	\
			return	pCast ;													\
		}

#define	ESL_IMPLEMENT_CLASS_INFO( class_name, parent_class )				\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST( class_name, parent_class )

#define	ESL_DLL_IMPLEMENT_CLASS_INFO( class_name, parent_class )				\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST( class_name, parent_class )

#define	ESL_IMPLEMENT_NV_CLASS_INFO( class_name )							\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, NULL, NULL )				\
	bool class_name::IsKindOf( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			return	(&rtClass == &class_name::m_RuntimeClass) ;				\
		}																	\
	void * class_name::DynamicCast( const ESLRuntimeClass & rtClass ) const	\
		{																	\
			if ( rtClass.pszClassName == class_name::m_RuntimeClass.pszClassName )	\
				return	(void*) this ;										\
			return	NULL ;													\
		}

#define	ESL_IMPLEMENT_CLASS_INFO2( class_name, parent_class1, parent_class2 )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2( class_name, parent_class1, parent_class2 )

#define	ESL_DLL_IMPLEMENT_CLASS_INFO2( class_name, parent_class1, parent_class2 )	\
	ESL_DLL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2( class_name, parent_class1, parent_class2 )

#define	ESL_IMPLEMENT_CLASS_INFO_CAST( class_name, parent_class1, cast_member )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST_OBJ( class_name, parent_class1, cast_member )

#define	ESL_IMPLEMENT_CLASS_INFO_CAST2( class_name, parent_class1, cast_member1, cast_member2 )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST_OBJ2( class_name, parent_class1, cast_member1, cast_member2 )

#define	ESL_IMPLEMENT_CLASS_INFO2_CAST( class_name, parent_class1, parent_class2, cast_member )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST2_OBJ( class_name, parent_class1, parent_class2, cast_member )

#define	ESL_IMPLEMENT_CLASS_INFO3( class_name, parent_class1, parent_class2, parent_class3 )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST3( class_name, parent_class1, parent_class2, parent_class3 )

#define	ESL_IMPLEMENT_CLASS_INFO4( class_name, parent_class1, parent_class2, parent_class3, parent_class4 )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST4( class_name, parent_class1, parent_class2, parent_class3, parent_class4 )

#define	ESL_IMPLEMENT_CLASS_INFO5( class_name, parent_class1, parent_class2, parent_class3, parent_class4, parent_class5 )	\
	ESL_IMPLEMENT_CLASS_RUNTIME_INFO( class_name, &parent_class1::m_RuntimeClass, NULL )	\
	ESL_IMPLEMENT_CLASS_DYNAMIC_CAST5( class_name, parent_class1, parent_class2, parent_class3, parent_class4, parent_class5 )

#endif
