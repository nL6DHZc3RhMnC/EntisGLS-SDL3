
#if	!defined(__SAKURA2_SMART_POINTER_H__)
#define	__SAKURA2_SMART_POINTER_H__

namespace	SSystem
{
	// 自動解放ポインタ
	template <class T>	class	SSmartPointer
	{
	protected:
		T *				m_pObj ;
		atomic_int_t	m_nRef ;

	public:
		SSmartPointer( void )
		{
			m_pObj = NULL ;
			m_nRef = 0 ;
		}
		SSmartPointer( T * pObj )
		{
			m_pObj = pObj ;
			m_nRef = ((pObj != NULL) & 1) ;
		}
		~SSmartPointer( void )
		{
			delete	m_pObj ;
			m_pObj = NULL ;
		}
		T * operator = ( T * pObj )
		{
			delete	m_pObj ;
			m_pObj = pObj ;
			m_nRef = ((pObj != NULL) & 1) ;
			return	pObj ;
		}
		bool operator == ( const T * pObj ) const
		{
			return	(m_pObj == pObj) ;
		}
		bool operator != ( const T * pObj ) const
		{
			return	(m_pObj != pObj) ;
		}
		T * operator -> ( void ) const
		{
			return	m_pObj ;
		}
		T * Ptr( void ) const
		{
			return	m_pObj ;
		}
		T & operator * ( void ) const
		{
			return	*m_pObj ;
		}
		operator const T * ( void ) const
		{
			return	m_pObj ;
		}
		operator T * ( void ) const
		{
			return	m_pObj ;
		}
		T * Lock( void )
		{
			AtomicAdd( &m_nRef, 1 ) ;
			return	m_pObj ;
		}
		void Unlock( void )
		{
			ESLAssert( m_nRef > 0 ) ;
			if ( AtomicSub( &m_nRef, 1 ) == 0 )
			{
				delete	m_pObj ;
				m_pObj = NULL ;
			}
		}
		T * Detach( void )
		{
			T *	pObj = m_pObj ;
			m_pObj = NULL ;
			m_nRef = 0 ;
			return	pObj ;
		}
	} ;

	// 動的型付き（ESLObject 派生型）自動解放ポインタ
	template <class T>	class	SSmartPtr	: protected SSmartObject
	{
	public:
		// 構築関数
		SSmartPtr( void ) {}
		SSmartPtr( SSmartPtr<T>& ptr )
			: SSmartObject( T::GetESLPointer(  ptr.Detach() ) ) {}
		SSmartPtr( T * pObj )
			: SSmartObject( T::GetESLPointer( pObj ) ) {}

	public:
		// オブジェクト取得
		T * Ptr( void ) const
		{
			return	ESLTypeCast<T>( SSmartObject::m_pObject ) ;
		}
		operator T * ( void ) const
		{
			return	ESLTypeCast<T>( SSmartObject::m_pObject ) ;
		}
		T * operator -> ( void ) const
		{
			return	ESLTypeCast<T>( SSmartObject::m_pObject ) ;
		}
		T& operator * ( void ) const
		{
			return	*(ESLTypeCast<T>( SSmartObject::m_pObject )) ;
		}
		bool IsNull( void ) const
		{
			return	(SSmartObject::m_pObject == nullptr) ;
		}
		bool operator == ( const T * pObj ) const
		{
			return	ESLTypeCast<T>( SSmartObject::m_pObject ) == pObj ;
		}
		bool operator != ( const T * pObj ) const
		{
			return	ESLTypeCast<T>( SSmartObject::m_pObject ) != pObj ;
		}
		// オブジェクト設定
		void Set( T * pObj )
		{
			SSmartObject::SetObject( T::GetESLPointer( pObj ) ) ;
		}
		const SSmartPtr<T>& operator = ( SSmartPtr<T>& obj )
		{
			Set( obj.Detach() ) ;
			return	*this ;
		}
		const SSmartPtr<T>& operator = ( T * pObj )
		{
			SSmartObject::SetObject( T::GetESLPointer( pObj ) ) ;
			return	*this ;
		}
		// オブジェクト分離
		T * Detach( void )
		{
			return	ESLSmartCast<T>( SSmartObject::DetachObject() ) ;
		}
	} ;

	// 型付き同期参照
	template <class T> class	SSmartRef	: protected SSyncReference
	{
	public:
		// 構築関数
		SSmartRef( void ) {}
		SSmartRef( const SSmartRef<T>& ref ) : SSyncReference( ref ) {}
		SSmartRef( SSmartPtr<T>* ptr ) : SSyncReference( ptr ) {}
		SSmartRef( SSmartReference<T>& ref ) : SSyncReference( ref ) {}
		SSmartRef( T * pObj ) : SSyncReference( pObj ) {}

	public:
		// 参照取得
		T * Ref( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		SSmartPtr<T> * OwnerPtr( void ) const
		{
			return	(SSmartPtr<T>*) ESLTypeCast<SSmartObject>( m_pReference ) ;
		}
		operator T * ( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		T * operator -> ( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		T& operator * ( void ) const
		{
			return	*(ESLTypeCast<T>( m_pReference )) ;
		}
		bool IsNull( void ) const
		{
			return	(m_pReference == nullptr) ;
		}
		bool operator == ( const T * pObj ) const
		{
			return	ESLTypeCast<T>( m_pReference ) == pObj ;
		}
		bool operator == ( T * pObj ) const
		{
			return	ESLTypeCast<T>( m_pReference ) == pObj ;
		}
		bool operator != ( const T * pObj ) const
		{
			return	ESLTypeCast<T>( m_pReference ) != pObj ;
		}
		bool operator != ( T * pObj ) const
		{
			return	ESLTypeCast<T>( m_pReference ) != pObj ;
		}
		// 参照設定
		const SSmartRef<T>& SetRef( T * pObj )
		{
			SSyncReference::SetReference( T::GetESLPointer( pObj ) ) ;
			return	*this ;
		}
		const SSmartRef<T>& SetOwnRef( T * pObj )
		{
			SSyncReference::SetReference( new SSmartPtr<T>( pObj ) ) ;
			return	*this ;
		}
		const SSmartRef<T>& operator = ( const SSmartRef<T>& ref )
		{
			SSyncReference::SetReference( ref ) ;
			return	*this ;
		}
		const SSmartRef<T>& operator = ( T * pObj )
		{
			SSyncReference::SetReference( T::GetESLPointer( pObj ) ) ;
			return	*this ;
		}
		// 分離
		T * Detach( void )
		{
			T *	pObj = nullptr ;
			QuickLock() ;
			SSmartPtr<T> *	pOwner = OwnerPtr() ;
			if ( pOwner != nullptr )
			{
				pObj = pOwner->Detach() ;
			}
			else
			{
				pObj = ESLTypeCast<T>( m_pReference ) ;
			}
			QuickUnlock() ;
			ReleaseReference() ;
			return	pObj ;
		}
	} ;

	template <class T>	class	SDependentPointer
			: public SDependentNotification, public SSmartPointer<T>
	{
	public:
		T * operator = ( T * pObj )
		{
			SSmartPointer<T>::operator = ( pObj ) ;
			if ( SSmartPointer<T>::m_pObj != NULL )
			{
				g_dnsLibServ.AddNotification( this ) ;
			}
			else
			{
				g_dnsLibServ.DetachNotification( this ) ;
			}
			return	pObj ;
		}
		T * Detach( void )
		{
			T *	p = SSmartPointer<T>::Detach() ;
			g_dnsLibServ.DetachNotification( this ) ;
			return	p ;
		}
		virtual void OnFinalizeObject( SDependentNotificationServer * pObject )
		{
			delete	SSmartPointer<T>::m_pObj ;
			SSmartPointer<T>::m_pObj = NULL ;
		}
	} ;

} ;


#endif

