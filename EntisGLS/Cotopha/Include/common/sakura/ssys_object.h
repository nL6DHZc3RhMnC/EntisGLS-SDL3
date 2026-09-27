

#if	!defined(__SAKURA2_OBJECT_H__)
#define	__SAKURA2_OBJECT_H__

namespace	SSystem
{
	class	SReference ;
	class	SSyncReference ;

	//////////////////////////////////////////////////////////////////////////
	// バックリンク付きオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SObject	: public ESLObject
	{
	protected:
		SReference *	m_pRefBackLink ;	// 参照バックリング

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO_THIS_ECP( SObject, ESLObject )
		// 構築関数（デフォルト）
		SObject( void ) : m_pRefBackLink(NULL) {}
		SObject( const SObject& obj ) : m_pRefBackLink(NULL) {}
		// 消滅関数
		virtual ~SObject( void )
		{
			if ( m_pRefBackLink != NULL )
			{
				DetachFromReference() ;
			}
		}
		// 参照を解除
		void DetachFromReference( void ) ;
		// 参照されているか？
		bool IsObjectReferenced( void ) const
		{
			return	(m_pRefBackLink != NULL) ;
		}

	protected:
		// 全ての参照が解除された
		virtual void OnReleaseFromReference( void ) ;

		friend class SReference ;
		friend class SSyncReference ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 自動消滅オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SSmartObject	: public SObject
	{
	protected:
		ESLObject *	m_pObject ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSmartObject, SObject )

	public:
		// 構築関数
		SSmartObject( void ) : m_pObject(NULL) {}
		SSmartObject( ESLObject * pObj ) : m_pObject(pObj) {}
		// 消滅関数
		virtual ~SSmartObject( void ) ;

	public:
		// オブジェクト取得
		ESLObject * GetObject( void ) const
		{
			return	m_pObject ;
		}
		// オブジェクト設定
		void SetObject( ESLObject * pObj )
		{
			delete	m_pObject ;
			m_pObject = pObj ;
		}
		// オブジェクト分離
		ESLObject * DetachObject( void )
		{
			ESLObject *	pObj = m_pObject ;
			m_pObject = NULL ;
			return	pObj ;
		}

	protected:
		// 全ての参照が解除された
		virtual void OnReleaseFromReference( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 参照オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SReference	: public SObject
	{
	protected:
		SObject *		m_pReference ;	// 参照先
		SReference *	m_pPrevRef ;	// 同じオブジェクトを参照する前の SReference
		SReference *	m_pNextRef ;	// 同じオブジェクトを参照する次の SReference

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SReference, SObject )
		// 構築関数
		SReference( void )
			: m_pReference(NULL), m_pPrevRef(NULL), m_pNextRef(NULL) {}
		SReference( SObject * pObj )
			: m_pReference(NULL), m_pPrevRef(NULL), m_pNextRef(NULL)
		{
			SReference::SetReference( pObj ) ;
		}
		SReference( const SReference& refSrc )
			: m_pReference(NULL), m_pPrevRef(NULL), m_pNextRef(NULL)
		{
			SReference::SetReference( refSrc.m_pReference ) ;
		}
		// 消滅関数
		virtual ~SReference( void )
		{
			if ( m_pReference != NULL )
			{
				SReference::ReleaseReference() ;
			}
		}

	public:
		// 参照先オブジェクトを取得
		SObject * GetReference( void ) const
		{
			return	m_pReference ;
		}
		template <class T> T * GetRef( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		bool operator == ( const SObject * pObj ) const
		{
			return	(m_pReference == pObj) ;
		}
		bool operator != ( const SObject * pObj ) const
		{
			return	(m_pReference != pObj) ;
		}
		// 参照を設定
		virtual void SetReference( SObject * pObj ) ;
		void SetSmartReference( ESLObject * pObj )
		{
			SetReference( new SSmartObject( pObj ) ) ;
		}
		const SReference & operator = ( const SReference & ref )
		{
			SetReference( ref.m_pReference ) ;
			return	*this ;
		}
		SObject * operator = ( SObject * pObj )
		{
			SetReference( pObj ) ;
			return	pObj ;
		}
		// スマート（自動削除）参照か？
		bool IsSmartReference( void ) const
		{
			return	(GetRef<SSmartObject>() != nullptr) ;
		}
		// 参照を解除
		virtual void ReleaseReference( void ) ;
		// 参照を解除（すべての参照が解除された場合のみ参照先オブジェクトを返却）
		SObject * GetReleaseReference( void ) ;

	protected:
		// SObject 側から参照の解除要求
		virtual void DetachAllReference( void ) ;

		friend class SObject ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 同期参照オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SSyncReference	: public SReference
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSyncReference, SReference )
		// 構築関数
		SSyncReference( void ) {}
		SSyncReference( const SSyncReference& refSrc ) ;
		SSyncReference( SObject * pObj )
		{
			SSyncReference::SetReference( pObj ) ;
		}
		// 消滅関数
		virtual ~SSyncReference( void )
		{
			if ( m_pReference != NULL )
			{
				SSyncReference::ReleaseReference() ;
			}
		}

	public:
		// 参照を設定
		virtual void SetReference( SObject * pObj ) ;
		const SSyncReference & operator = ( const SSyncReference & ref ) ;
		SObject * operator = ( SObject * pObj )
		{
			SetReference( pObj ) ;
			return	pObj ;
		}
		// 参照を分離
		ESLObject * DetachReference( void )
		{
			SSmartObject *
				pSObj = ESLTypeCast<SSmartObject>( m_pReference ) ;
			ESLObject *	pObj = nullptr ;
			if ( pSObj != nullptr )
			{
				pObj = pSObj->DetachObject() ;
			}
			else
			{
				pObj = m_pReference ;
				ReleaseReference() ;
			}
			return	pObj ;
		}
		// 参照を解除
		virtual void ReleaseReference( void ) ;

	protected:
		// SObject 側から参照の解除要求
		virtual void DetachAllReference( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 型付き同期参照オブジェクト
	//////////////////////////////////////////////////////////////////////////

	template <class T> class SSmartReference	: public SSyncReference
	{
	public:
		// 構築関数
		SSmartReference( void ) {}
		SSmartReference( const SSmartReference<T>& refSrc )
			: SSyncReference( refSrc ) { }
		SSmartReference( T * pObj )
			: SSyncReference( ESLTypeCast<SObject>( pObj ) ) { }
	public:
		// 参照先オブジェクトを取得
		T * GetReference( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		T * operator -> ( void ) const
		{
			ESLAssert( m_pReference != NULL ) ;
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		operator T * ( void ) const
		{
			return	ESLTypeCast<T>( m_pReference ) ;
		}
		bool operator == ( const T * pObj ) const
		{
			return	(ESLTypeCast<T>( m_pReference ) == pObj) ;
		}
		bool operator != ( const T * pObj ) const
		{
			return	(ESLTypeCast<T>( m_pReference ) != pObj) ;
		}
		// 参照を設定
		void SetReference( T * pObj )
		{
			SSyncReference::SetReference( T::GetESLPointer( pObj ) ) ;
		}
		void SetSmartReference( T * pObj )
		{
			SSyncReference::SetReference
				( new SSmartObject( (typename T::ESLClassPointer) pObj ) ) ;
		}
		const SSmartReference<T> & operator = ( const SSmartReference<T> & ref )
		{
			SSyncReference::SetReference( ref ) ;
			return	*this ;
		}
		T * operator = ( T * pObj )
		{
			SetReference( pObj ) ;
			return	pObj ;
		}
		// 参照を分離
		T * DetachReference( void )
		{
			return	ESLTypeCast<T>( SSyncReference::DetachReference() ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 詞葉・揮発性オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SVolatileInterface
	{
	public:
		// コンテキスト復元後の処理
		virtual void OnLoaded( void ) = 0 ;
	} ;

	class	__native VolatileObject
	{
	public:
		// 揮発性オブジェクト処理用インターフェース関連付け
		__native void AttachVolatileInterface
						( SVolatileInterface * pInterface ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 依存オブジェクト通知インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SDependentNotificationServer ;
	class	SDependentNotification	: public SObject
	{
	protected:
		SReference					m_refDependent ;
		SDependentNotification *	m_pdnPrev ;
		SDependentNotification *	m_pdnNext ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SDependentNotification, SObject )
		// 構築関数
		SDependentNotification( void ) ;
		// 消滅関数
		virtual ~SDependentNotification( void ) ;

	public:
		// このオブジェクトの前に挿入する
		void InsertBeforeNotification( SDependentNotification * pBefore ) ;
		// このオブジェクトの後に挿入する
		void InsertAfterNotification( SDependentNotification * pAfter ) ;
		// このオブジェクトを分離する
		void DetachNotification( void ) ;

	public:
		// 依存先オブジェクト生成／再生成後に呼び出される
		virtual void OnCreateObject( SDependentNotificationServer * pObject, bool fInitialize ) ;
		// 依存先のリソースが解放される／リセットされる前等に呼び出される
		virtual void OnReleaseObject( SDependentNotificationServer * pObject ) ;
		// 依存先オブジェクトが削除される前に呼び出される
		virtual void OnFinalizeObject( SDependentNotificationServer * pObject ) ;

		friend class SDependentNotificationServer ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 依存オブジェクト通知サーバ
	//////////////////////////////////////////////////////////////////////////

	class	SDependentNotificationServer	: public SDependentNotification
	{
	public:
		SDependentNotification *	m_pdnFirstNotify ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SDependentNotificationServer, SDependentNotification )
		// 構築関数
		SDependentNotificationServer( void ) ;
		// 消滅関数
		virtual ~SDependentNotificationServer( void ) ;

	public:
		// 通知オブジェクトを追加する
		void AddNotification( SDependentNotification * pdnNotify ) ;
		// 通知を解除する
		void DetachNotification( SDependentNotification * pdnNotify ) ;

	public:
		// OnCreateObject を通知する
		void NotifyAllOnCreate( bool fInitialize ) ;
		// OnReleaseObject を通知する
		void NotifyAllOnRelease( void ) ;
		// OnFinalizeObject を通知する
		void NotifyAllOnFinalize( void ) ;

	public:
		// 依存先オブジェクト生成／再生成後に呼び出される
		virtual void OnCreateObject( SDependentNotificationServer * pObject, bool fInitialize ) ;
		// 依存先のリソースが解放される／リセットされる前等に呼び出される
		virtual void OnReleaseObject( SDependentNotificationServer * pObject ) ;
		// 依存先オブジェクトが削除される前に呼び出される
		virtual void OnFinalizeObject( SDependentNotificationServer * pObject ) ;
	} ;

	extern ESL_DLL_EXPORT SDependentNotificationServer	g_dnsLibServ ;
}


#endif

