
#if	!defined(__SAKURA2_SORT_ARRAY_H__)
#define	__SAKURA2_SORT_ARRAY_H__

namespace	SSystem
{

//////////////////////////////////////////////////////////////////////////////
// ソート式辞書配列要素
//	T : ソート・キー
//	S : オブジェクト型
//	F : ソート・キー引数型
//	G : オブジェクト・キャスト型／代入元型
//////////////////////////////////////////////////////////////////////////////

// ※テンプレート引数の名前の長さに注意※
// 識別子が 255 文字を超えないためには S は名前空間を含め32文字以内程度が目安

// 最もシンプルな辞書要素
template <class T, class S> class SSortElement
{
public:
	T		m_tag ;
	S		m_obj ;

	typedef	T			TypeTag ;
	typedef	S			TypeObj ;
	typedef	T			TypeT ;
	typedef	const S &	TypeS ;
	typedef	S &			TypeO ;
	typedef	S *			TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SSortElement( TypeT t ) : m_tag(t)
	{
	}
	SSortElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SSortElement( const SSortElement<T,S>& src )
		: m_tag(src.m_tag), m_obj(src.m_obj)
	{
	}
	operator TypeO ( void )
	{
		return	m_obj ;
	}
	operator TypeP ( void )
	{
		return	&m_obj ;
	}
	void operator = ( TypeS obj )
	{
		m_obj = obj ;
	}
	int Compare( TypeT t ) const
	{
		return	(m_tag < t) ? -1 : ((m_tag > t) ? 1 : 0) ;
	}
} ;

// 汎用的な型の辞書要素
template <class T, class S> class SGenSortElement
{
public:
	T						m_tag ;
	S						m_obj ;

	typedef	T				TypeTag ;
	typedef	S				TypeObj ;
	typedef	T				TypeT ;
	typedef	const S &		TypeS ;
	typedef	S &				TypeO ;
	typedef	S *				TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SGenSortElement( TypeT t ) : m_tag(t)
	{
	}
	SGenSortElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SGenSortElement( const SGenSortElement<T,S>& src )
		: m_tag(src.m_tag), m_obj(src.m_obj)
	{
	}
	operator TypeO ( void )
	{
		return	m_obj ;
	}
	operator TypeP ( void )
	{
		return	&m_obj ;
	}
	void operator = ( TypeS obj )
	{
		m_obj = obj ;
	}
	int Compare( TypeT t ) const
	{
		return	(m_tag < t) ? -1 : ((m_tag > t) ? 1 : 0) ;
	}
} ;

// ポインタ値比較
template <class T> class SPointerComparator
{
public:
	ulong_ptr_t	m_pointer ;
public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SPointerComparator( const T * p )
	{
		m_pointer = (ulong_ptr_t) p ;
	}
	SPointerComparator( const SPointerComparator& pc )
	{
		m_pointer = pc.m_pointer ;
	}
	operator T * ( void ) const
	{
		return	(T*) m_pointer ;
	}
	operator const T * ( void ) const
	{
		return	(const T*) m_pointer ;
	}
	bool operator == ( const T * p ) const
	{
		return	(m_pointer == (ulong_ptr_t) p) ;
	}
	bool operator != ( const T * p ) const
	{
		return	(m_pointer != (ulong_ptr_t) p) ;
	}
	bool operator < ( const T * p ) const
	{
		return	(m_pointer < (ulong_ptr_t) p) ;
	}
	bool operator <= ( const T * p ) const
	{
		return	(m_pointer <= (ulong_ptr_t) p) ;
	}
	bool operator > ( const T * p ) const
	{
		return	(m_pointer > (ulong_ptr_t) p) ;
	}
	bool operator >= ( const T * p ) const
	{
		return	(m_pointer >= (ulong_ptr_t) p) ;
	}
} ;

// ポインタをキーとする辞書要素
template <class T, class S> class SPointerSortElement
{
public:
	SPointerComparator<T>	m_tag ;
	S						m_obj ;

	typedef	SPointerComparator<T>	TypeTag ;
	typedef	S						TypeObj ;
	typedef	const T *				TypeT ;
	typedef	const S &				TypeS ;
	typedef	S &						TypeO ;
	typedef	S *						TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SPointerSortElement( TypeT t ) : m_tag(t)
	{
	}
	SPointerSortElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SPointerSortElement( const SPointerSortElement<T,S>& src )
		: m_tag(src.m_tag), m_obj(src.m_obj)
	{
	}
	operator TypeO ( void )
	{
		return	m_obj ;
	}
	operator TypeP ( void )
	{
		return	&m_obj ;
	}
	void operator = ( TypeS obj )
	{
		m_obj = obj ;
	}
	int Compare( TypeT t ) const
	{
		return	(m_tag < t) ? -1 : ((m_tag > t) ? 1 : 0) ;
	}
} ;

// 文字列をキーとする辞書要素
template <class S> class SStringSortElement
{
public:
	SString		m_tag ;
	S			m_obj ;

	typedef	SString				TypeTag ;
	typedef	S					TypeObj ;
	typedef	const wchar_t *		TypeT ;
	typedef	const S &			TypeS ;
	typedef	S &					TypeO ;
	typedef	S *					TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SStringSortElement( TypeT t ) : m_tag(t)
	{
	}
	SStringSortElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SStringSortElement( const SStringSortElement<S>& src )
			: m_tag(src.m_tag), m_obj(src.m_obj)
	{
	}
	operator TypeO ( void )
	{
		return	m_obj ;
	}
	operator TypeP ( void )
	{
		return	&m_obj ;
	}
	void operator = ( TypeS obj )
	{
		m_obj = obj ;
	}
	int Compare( TypeT t ) const
	{
		return	m_tag.Compare(t) ;
	}
} ;

// オブジェクトポインタを所有する辞書要素
template <class T, class S> class SSortObjectElement
{
public:
	T		m_tag ;
	S*		m_obj ;

	typedef	T		TypeTag ;
	typedef	S*		TypeObj ;
	typedef	T		TypeT ;
	typedef	S *		TypeS ;
	typedef	S &		TypeO ;
	typedef	S *		TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SSortObjectElement( TypeT t ) : m_tag(t), m_obj(NULL)
	{
	}
	SSortObjectElement( TypeT t, TypeS g ) : m_tag(t), m_obj(g)
	{
	}
	SSortObjectElement( const SSortObjectElement<T,S>& src )
			: m_tag(src.m_tag)
	{
		m_obj = new S( *src.m_obj ) ;
	}
	~SSortObjectElement( void )
	{
		delete	m_obj ;
	}
	operator TypeP ( void )
	{
		return	m_obj ;
	}
	void operator = ( TypeS obj )
	{
		delete	m_obj ;
		m_obj = obj ;
	}
	TypeP Detach( void )
	{
		TypeP	obj = m_obj ;
		m_obj = NULL ;
		return	obj ;
	}
	int Compare( TypeT t ) const
	{
		return	(m_tag < t) ? -1 : ((m_tag > t) ? 1 : 0) ;
	}
} ;

// ポインタ値をキーとし、オブジェクトポインタを所有する辞書要素
template <class T,class S> class SPointerSortObjectElement
{
public:
	SPointerComparator<T>	m_tag ;
	S *						m_obj ;

	typedef	SPointerComparator<T>	TypeTag ;
	typedef	S *						TypeObj ;
	typedef	const T *				TypeT ;
	typedef	S *						TypeS ;
	typedef	S &						TypeO ;
	typedef	S *						TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SPointerSortObjectElement( TypeT t ) : m_tag(t), m_obj(NULL)
	{
	}
	SPointerSortObjectElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SPointerSortObjectElement( const SPointerSortObjectElement<T,S>& src )
			: m_tag(src.m_tag)
	{
		m_obj = new S( *src.m_obj ) ;
	}
	~SPointerSortObjectElement( void )
	{
		delete	m_obj ;
	}
	operator TypeP ( void )
	{
		return	m_obj ;
	}
	void operator = ( TypeS obj )
	{
		delete	m_obj ;
		m_obj = obj ;
	}
	TypeP Detach( void )
	{
		TypeP	obj = m_obj ;
		m_obj = NULL ;
		return	obj ;
	}
	int Compare( TypeT t ) const
	{
		return	(m_tag < t) ? -1 : ((m_tag > t) ? 1 : 0) ;
	}
} ;

// 文字列をキーとし、オブジェクトポインタを所有する辞書要素
template <class S> class SStringSortObjectElement
{
public:
	SString		m_tag ;
	S*			m_obj ;

	typedef	SString			TypeTag ;
	typedef	S*				TypeObj ;
	typedef	const wchar_t *	TypeT ;
	typedef	S *				TypeS ;
	typedef	S &				TypeO ;
	typedef	S *				TypeP ;

public:
	ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

	SStringSortObjectElement( TypeT t ) : m_tag(t), m_obj(NULL)
	{
	}
	SStringSortObjectElement( TypeT t, TypeS s ) : m_tag(t), m_obj(s)
	{
	}
	SStringSortObjectElement( const SStringSortObjectElement<S>& src )
			: m_tag(src.m_tag)
	{
		m_obj = new S( *src.m_obj ) ;
	}
	~SStringSortObjectElement( void )
	{
		delete	m_obj ;
	}
	operator TypeP ( void )
	{
		return	m_obj ;
	}
	void operator = ( TypeS obj )
	{
		delete	m_obj ;
		m_obj = obj ;
	}
	TypeP Detach( void )
	{
		TypeP	obj = m_obj ;
		m_obj = NULL ;
		return	obj ;
	}
	int Compare( TypeT t ) const
	{
		return	m_tag.Compare(t) ;
	}
} ;


//////////////////////////////////////////////////////////////////////////////
// ソート式辞書配列
//	T::TypeT : ソート・キー引数／代入元型
//	T::TypeS : オブジェクト／代入元型
//	T::TypeO : オブジェクト・キャスト型
//	T::TypeP : オブジェクト・ポインタ型
//////////////////////////////////////////////////////////////////////////////

template <class T> class SSortArray : public SObjectArray<T>
{
public:
	// 構築関数
	SSortArray( void )
	{
	}
	SSortArray( const SSortArray<T>& src )
	{
		SObjectArray<T>::DuplicateArray( src ) ;
	}
	// 配列複製
	const SSortArray<T> & operator = ( const SSortArray<T> & src )
	{
		SObjectArray<T>::DuplicateArray( src ) ;
		return	*this ;
	}
	// 指標検索
	size_t OrderIndex( typename T::TypeT tag ) const
	{
		T *			pElement ;
		int			cmp ;
		ssize_t		iFirst, iEnd, iMiddle = 0 ;
		T**			ppArray = SArray<T*>::m_ptrArray ;
		iFirst = 0 ;
		iEnd = (ssize_t) SArray<T*>::m_nLength - 1 ;
		//
		while ( iFirst <= iEnd )
		{
			iMiddle = ((iFirst + iEnd) >> 1) ;
			pElement = ppArray[iMiddle] ;
			ESLAssert( pElement != NULL ) ;
			//
			cmp = pElement->Compare( tag ) ;
			if ( cmp > 0 )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( cmp < 0 )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				return	(size_t) iMiddle ;
			}
		}
		return	iFirst ;
	}
	ssize_t FindAs( typename T::TypeT tag ) const
	{
		T *			pElement ;
		int			cmp ;
		ssize_t		iFirst, iEnd, iMiddle = 0 ;
		T**			ppArray = SArray<T*>::m_ptrArray ;
		iFirst = 0 ;
		iEnd = (ssize_t) SArray<T*>::m_nLength - 1 ;
		//
		while ( iFirst <= iEnd )
		{
			iMiddle = ((iFirst + iEnd) >> 1) ;
			pElement = ppArray[iMiddle] ;
			ESLAssert( pElement != NULL ) ;
			//
			cmp = pElement->Compare( tag ) ;
			if ( cmp > 0 )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( cmp < 0 )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				return	iMiddle ;
			}
		}
		return	-1 ;
	}
	ssize_t FindPtr( typename T::TypeP pObj, size_t nFirst = 0 ) const
	{
		const size_t	nLength = SArray<T*>::m_nLength ;
		T**				pArray = SArray<T*>::m_ptrArray ;
		for ( size_t i = nFirst; i < nLength; i ++ )
		{
			T *	p = pArray[i] ;
			if ( p && (((typename T::TypeP) *p) == pObj) )
			{
				return	(ssize_t) i ;
			}
		}
		return	-1 ;
	}
	// 要素アクセス
	typename T::TypeP GetAs( typename T::TypeT tag ) const
	{
		return	GetAt( FindAs( tag ) ) ;
	}
	typename T::TypeP GetAt( size_t nIndex ) const
	{
		if ( nIndex < SArray<T*>::m_nLength )
		{
			T *	p = SArray<T*>::m_ptrArray[nIndex] ;
			if ( p != NULL )
			{
				return	(typename T::TypeP) *p ;
			}
		}
		return	NULL ;
	}
	typename T::TypeO At( size_t nIndex ) const
	{
		ESLAssert( nIndex < SArray<T*>::m_nLength ) ;
		typename T::TypeP	p = GetAt( nIndex ) ;
		ESLAssert( p != NULL ) ;
		return	*p ;
	}
	T* GetElementAt( size_t nIndex ) const
	{
		if ( nIndex < SArray<T*>::m_nLength )
		{
			return	SArray<T*>::m_ptrArray[nIndex] ;
		}
		return	NULL ;
	}
	typename T::TypeO operator [] ( typename T::TypeT tag ) const
	{
		typename T::TypeP	p = GetAs( tag ) ;
		ESLAssert( p != NULL ) ;
		return	*p ;
	}
	const typename T::TypeTag * GetTagAt( size_t nIndex ) const
	{
		if ( nIndex < SArray<T*>::m_nLength )
		{
			T *	p = SArray<T*>::m_ptrArray[nIndex] ;
			if ( p != NULL )
			{
				return	&(p->m_tag) ;
			}
		}
		return	NULL ;
	}
	const typename T::TypeTag & TagAt( size_t nIndex ) const
	{
		ESLAssert( nIndex < SArray<T*>::m_nLength ) ;
		T *	p = SArray<T*>::m_ptrArray[nIndex] ;
		ESLAssert( p != NULL ) ;
		return	p->m_tag ;
	}
	// 要素追加
	size_t Add( typename T::TypeT tag, typename T::TypeS obj )
	{
		size_t	nIndex = OrderIndex( tag ) ;
		SObjectArray<T>::InsertAt( nIndex, new T( tag, obj ) ) ;
		return	nIndex ;
	}
	size_t AddElement( T * pElement )
	{
		return	SObjectArray<T>::Add( pElement ) ;
	}
	// 要素設定
	size_t SetAs( typename T::TypeT tag, typename T::TypeS obj )
	{
		size_t	nIndex = OrderIndex( tag ) ;
		if ( (nIndex >= 0)
				& ((unsigned int) nIndex < SArray<T*>::m_nLength) )
		{
			T *	p = SArray<T*>::m_ptrArray[nIndex] ;
			if ( p->m_tag == tag )
			{
				*p = obj ;
				return	nIndex ;
			}
		}
		SObjectArray<T>::InsertAt( nIndex, new T( tag, obj ) ) ;
		return	nIndex ;
	}
	// 要素削除
	void RemoveAs( typename T::TypeT tag )
	{
		ssize_t	nIndex = FindAs( tag ) ;
		if ( nIndex >= 0 )
		{
			SObjectArray<T>::RemoveAt( nIndex ) ;
		}
	}

} ;



//////////////////////////////////////////////////////////////////////////////
// int キーでソートするコンテナ配列
//////////////////////////////////////////////////////////////////////////////

template <class T> class SIntSortArray
		: public SSortArray< SSortElement<int,T> >
{
public:
	// 構築関数
	SIntSortArray( void )
	{
	}
	SIntSortArray( const SIntSortArray<T>& src )
	{
		SObjectArray< SSortElement<int,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SIntSortArray<T> & operator = ( const SIntSortArray<T> & src )
	{
		SObjectArray< SSortElement<int,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SUIntSortArray
		: public SSortArray< SSortElement<unsigned int,T> >
{
public:
	// 構築関数
	SUIntSortArray( void )
	{
	}
	SUIntSortArray( const SUIntSortArray<T>& src )
	{
		SObjectArray< SSortElement<unsigned int,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SUIntSortArray<T> & operator = ( const SUIntSortArray<T> & src )
	{
		SObjectArray< SSortElement<unsigned int,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SLongSortArray
		: public SSortArray< SSortElement<long,T> >
{
public:
	// 構築関数
	SLongSortArray( void )
	{
	}
	SLongSortArray( const SLongSortArray<T>& src )
	{
		SObjectArray< SSortElement<long,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SLongSortArray<T> & operator = ( const SLongSortArray<T> & src )
	{
		SObjectArray< SSortElement<long,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SULongSortArray
		: public SSortArray< SSortElement<unsigned long,T> >
{
public:
	// 構築関数
	SULongSortArray( void )
	{
	}
	SULongSortArray( const SULongSortArray<T>& src )
	{
		SObjectArray< SSortElement<unsigned long,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SULongSortArray<T> & operator = ( const SULongSortArray<T> & src )
	{
		SObjectArray<SSortElement<unsigned long,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;


//////////////////////////////////////////////////////////////////////////////
// 文字列キーでソートするコンテナ配列
//////////////////////////////////////////////////////////////////////////////

template <class T> class SStrSortArray
		: public SSortArray< SStringSortElement<T> >
{
public:
	// 構築関数
	SStrSortArray( void )
	{
	}
	SStrSortArray( const SStrSortArray<T>& src )
	{
		SObjectArray< SStringSortElement<T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SStrSortArray<T> & operator = ( const SStrSortArray<T> & src )
	{
		SObjectArray<SStringSortElement<T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;


//////////////////////////////////////////////////////////////////////////////
// ポインタ値でソートするコンテナ配列
//////////////////////////////////////////////////////////////////////////////

template <class T, class S> class SPtrSortArray
		: public SSortArray< SPointerSortElement<T,S> >
{
public:
	// 構築関数
	SPtrSortArray( void )
	{
	}
	SPtrSortArray( const SPtrSortArray<T,S>& src )
	{
		SObjectArray< SPointerSortElement<T,S> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SPtrSortArray<T,S> & operator = ( const SPtrSortArray<T,S> & src )
	{
		SObjectArray< SPointerSortElement<T,S> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;



//////////////////////////////////////////////////////////////////////////////
// オブジェクトをポインタで保持するソート式辞書配列
//////////////////////////////////////////////////////////////////////////////

template <class T> class SSortObjectArray : public SSortArray< T >
{
public:
	// 構築関数
	SSortObjectArray( void )
	{
	}
	SSortObjectArray( const SSortObjectArray<T>& src )
	{
		SObjectArray<T>::DuplicateArray( src ) ;
	}
	// 配列複製
	const SSortObjectArray<T> & operator = ( const SSortObjectArray<T> & src )
	{
		SObjectArray<T>::DuplicateArray( src ) ;
		return	*this ;
	}
	// 要素分離
	typename T::TypeP DetachAs( typename T::TypeT tag )
	{
		ssize_t	nIndex = SSortArray<T>::FindAs( tag ) ;
		if ( nIndex >= 0 )
		{
			ESLAssert( SArray<T*>::m_ptrArray[nIndex] != NULL ) ;
			typename T::TypeP	pObj =
					SArray<T*>::m_ptrArray[nIndex]->Detach() ;
			SSortArray<T>::RemoveAt( nIndex ) ;
			return	pObj ;
		}
		return	NULL ;
	}
	typename T::TypeP DetachAt( size_t nIndex )
	{
		if ( nIndex < SArray<T*>::m_nLength )
		{
			ESLAssert( SArray<T*>::m_ptrArray[nIndex] != NULL ) ;
			typename T::TypeP	pObj =
					SArray<T*>::m_ptrArray[nIndex]->Detach() ;
			SSortArray<T>::RemoveAt( nIndex ) ;
			return	pObj ;
		}
		return	NULL ;
	}
	void DetachAll( void )
	{
		const int	nLength = SArray<T*>::m_nLength ;
		for ( int i = 0; i < nLength; i ++ )
		{
			ESLAssert( SArray<T*>::m_ptrArray[i] != NULL ) ;
			SArray<T*>::m_ptrArray[i]->Detach() ;
		}
		SSortArray<T>::RemoveAll() ;
	}

} ;



//////////////////////////////////////////////////////////////////////////////
// int キーでソートするオブジェクト配列
//////////////////////////////////////////////////////////////////////////////

template <class T> class SIntSortObjectArray
		: public SSortObjectArray< SSortObjectElement<int,T> >
{
public:
	// 構築関数
	SIntSortObjectArray( void )
	{
	}
	SIntSortObjectArray( const SIntSortObjectArray<T>& src )
	{
		SObjectArray< SSortObjectElement<int,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SIntSortObjectArray<T> & operator = ( const SIntSortObjectArray<T> & src )
	{
		SObjectArray< SSortObjectElement<int,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SUIntSortObjectArray
		: public SSortObjectArray< SSortObjectElement<unsigned int,T> >
{
public:
	// 構築関数
	SUIntSortObjectArray( void )
	{
	}
	SUIntSortObjectArray( const SUIntSortObjectArray<T>& src )
	{
		SObjectArray< SSortObjectElement<unsigned int,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SUIntSortObjectArray<T> & operator = ( const SUIntSortObjectArray<T> & src )
	{
		SObjectArray< SSortObjectElement<unsigned int,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SLongSortObjectArray
		: public SSortObjectArray< SSortObjectElement<long,T> >
{
public:
	// 構築関数
	SLongSortObjectArray( void )
	{
	}
	SLongSortObjectArray( const SLongSortObjectArray<T>& src )
	{
		SObjectArray< SSortObjectElement<long,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SLongSortObjectArray<T> & operator = ( const SLongSortObjectArray<T> & src )
	{
		SObjectArray< SSortObjectElement<long,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SULongSortObjectArray
		: public SSortObjectArray< SSortObjectElement<unsigned long,T> >
{
public:
	// 構築関数
	SULongSortObjectArray( void )
	{
	}
	SULongSortObjectArray( const SULongSortObjectArray<T>& src )
	{
		SObjectArray< SSortObjectElement<unsigned long,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SULongSortObjectArray<T> & operator = ( const SULongSortObjectArray<T> & src )
	{
		SObjectArray< SSortObjectElement<unsigned long,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;

template <class T> class SULongPtrSortObjectArray
		: public SSortObjectArray< SSortObjectElement<ulong_ptr_t,T> >
{
public:
	// 構築関数
	SULongPtrSortObjectArray( void )
	{
	}
	SULongPtrSortObjectArray( const SULongPtrSortObjectArray<T>& src )
	{
		SObjectArray< SSortObjectElement<unsigned long,T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SULongPtrSortObjectArray<T> & operator = ( const SULongPtrSortObjectArray<T> & src )
	{
		SObjectArray< SSortObjectElement<unsigned long,T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;



//////////////////////////////////////////////////////////////////////////////
// 文字列キーでソートするオブジェクト配列
//////////////////////////////////////////////////////////////////////////////

template <class T> class SStrSortObjectArray
		: public SSortObjectArray< SStringSortObjectElement<T> >
{
public:
	// 構築関数
	SStrSortObjectArray( void )
	{
	}
	SStrSortObjectArray( const SStrSortObjectArray<T>& src )
	{
		SObjectArray< SStringSortObjectElement<T> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SStrSortObjectArray<T> & operator = ( const SStrSortObjectArray<T> & src )
	{
		SObjectArray< SStringSortObjectElement<T> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;



//////////////////////////////////////////////////////////////////////////////
// ポインタ値でソートするオブジェクト配列
//////////////////////////////////////////////////////////////////////////////

template <class T, class S> class SPtrSortObjectArray
		: public SSortObjectArray< SPointerSortObjectElement<T,S> >
{
public:
	// 構築関数
	SPtrSortObjectArray( void )
	{
	}
	SPtrSortObjectArray( const SPtrSortObjectArray<T,S>& src )
	{
		SObjectArray< SPointerSortObjectElement<T,S> >::DuplicateArray( src ) ;
	}
	// 配列複製
	const SPtrSortObjectArray<T,S> & operator = ( const SPtrSortObjectArray<T,S> & src )
	{
		SObjectArray< SPointerSortObjectElement<T,S> >::DuplicateArray( src ) ;
		return	*this ;
	}
} ;


} ;

#endif

