

#if	!defined(__SAKURA2_SMART_OBJECT_ARRAY_H__)
#define	__SAKURA2_SMART_OBJECT_ARRAY_H__


//////////////////////////////////////////////////////////////////////////////
// イテレーション中の配列操作に対応できるオブジェクトポインタ配列
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SSmartObjectArray	: public SObjectArray<T>
	{
	public:
		class	Iterator	: protected SLinkedListConnection
		{
		protected:
			const SSmartObjectArray<T> *	m_pArray ;
			T *								m_pElement ;
			size_t							m_iElement ;
			size_t							m_iNext ;

		public:
			Iterator( const SSmartObjectArray<T> * pArray = NULL, size_t nIndex = 0 )
				: m_pArray( pArray ),
					m_pElement( NULL ), m_iElement( (size_t) -1 ), m_iNext( nIndex )
			{
				AttachArrayRef() ;
			}
			Iterator( const SSmartObjectArray<T> & ref, size_t nIndex = 0 )
				: m_pArray( &ref ),
					m_pElement( NULL ), m_iElement( (size_t) -1 ), m_iNext( nIndex )
			{
				AttachArrayRef() ;
			}
			Iterator( const Iterator& iter )
				: m_pArray( iter.m_pArray ), m_pElement( iter.m_pElement ),
					m_iElement( iter.m_iElement ), m_iNext( iter.m_iNext )
			{
				AttachArrayRef() ;
			}
			~Iterator( void )
			{
				DetachArrayRef() ;
			}
			const Iterator& operator = ( const Iterator& iter )
			{
				if ( m_pArray != iter.m_pArray )
				{
					DetachArrayRef() ;
					m_pArray = iter.m_pArray ;
					AttachArrayRef() ;
				}
				m_pElement = iter.m_pElement ;
				m_iElement = iter.m_iElement ;
				m_iNext = iter.m_iNext ;
				return	*this ;
			}
			bool operator == ( const Iterator& iter ) const
			{
				return	(m_pArray == iter.m_pArray) && (m_iNext == iter.m_iNext) ;
			}
			bool operator != ( const Iterator& iter ) const
			{
				return	(m_pArray != iter.m_pArray) || (m_iNext != iter.m_iNext) ;
			}
			bool IsExpired( void ) const
			{
				return	(m_pArray == NULL) ;
			}
			bool HasNext( void ) const
			{
				return	(m_pArray != NULL) && (m_iNext < m_pArray->GetLength()) ;
			}
			T * Next( void )
			{
				m_iElement = m_iNext ;
				m_pElement = (m_pArray != NULL) ? m_pArray->GetAt( m_iNext ++ ) : NULL ;
				return	m_pElement ;
			}
			bool HasPrev( void ) const
			{
				return	(m_pArray != NULL) && (m_iNext > 0) ;
			}
			T * Prev( void )
			{
				m_pElement = ((m_pArray != NULL) && (m_iNext > 0))
									? m_pArray->GetAt( -- m_iNext ) : NULL ;
				m_iElement = m_iNext ;
				return	m_pElement ;
			}
			T * Element( void ) const
			{
				return	m_pElement ;
			}
			size_t Index( void ) const
			{
				return	m_iElement ;
			}
			bool IsElementExpired( void ) const
			{
				return	(m_iElement == (size_t) -1) ;
			}

		protected:
			void AttachArrayRef( void )
			{
				if ( m_pArray != NULL )
				{
					const_cast< SSmartObjectArray<T>* >(m_pArray)->
											AttachIteratorRef( this ) ;
				}
			}
			void DetachArrayRef( void )
			{
				if ( m_pArray != NULL )
				{
					const_cast< SSmartObjectArray<T>* >(m_pArray)->
											DetachIteratorRef( this ) ;
				}
			}
			size_t InsertedElementIndex( size_t nIndex, size_t iFirst, size_t nCount ) const
			{
				if ( iFirst < nIndex )
				{
					nIndex += nCount ;
				}
				return	nIndex ;
			}
			void OnInsertedElement( size_t iFirst, size_t nCount )
			{
				m_iNext = InsertedElementIndex( m_iNext, iFirst, nCount ) ;
				m_iElement = InsertedElementIndex( m_iElement, iFirst, nCount ) ;
			}
			size_t RemovedElementIndex( size_t nIndex, size_t iFirst, size_t nCount, size_t iRemoved ) const
			{
				if ( iFirst <= nIndex )
				{
					if ( nIndex < iFirst + nCount )
					{
						nIndex = iRemoved ;
					}
					else
					{
						nIndex -= nCount ;
					}
				}
				return	nIndex ;
			}
			void OnRemovedElement( size_t iFirst, size_t nCount )
			{
				m_iNext = RemovedElementIndex( m_iNext, iFirst, nCount, iFirst ) ;
				m_iElement = RemovedElementIndex( m_iElement, iFirst, nCount, (size_t) -1 ) ;
				if ( m_iElement == (size_t) -1 )
				{
					m_pElement = NULL ;
				}
			}
			Iterator * GetNextLinked( void ) const
			{
				return	(Iterator*) SLinkedListConnection::GetNext() ;
			}
			Iterator * GetPrevLinked( void ) const
			{
				return	(Iterator*) SLinkedListConnection::GetPrev() ;
			}

			friend class SSmartObjectArray<T> ;
		} ;

		friend class Iterator ;

	protected:
		Iterator *	m_pRefFirstIter ;

	public:
		// 構築関数
		SSmartObjectArray( void ) : m_pRefFirstIter( nullptr )
		{
		}
		SSmartObjectArray( const SSmartObjectArray<T> & src )
			: SObjectArray<T>( src ), m_pRefFirstIter( nullptr )
		{
		}
		// 消滅関数
		~SSmartObjectArray( void )
		{
			Iterator *	pIter = m_pRefFirstIter ;
			while ( pIter != NULL )
			{
				pIter->m_pArray = NULL ;
				pIter->m_pElement = NULL ;
				pIter->m_iElement = (size_t) -1 ;
				pIter = pIter->GetNextLinked() ;
			}
		}
		// イテレーター
		Iterator Begin( void ) const
		{
			return	Iterator( this, 0 ) ;
		}
		Iterator End( void ) const
		{
			return	Iterator( this, SArray<T*>::m_nLength ) ;
		}
		// 配列のサイズを設定
		void SetLength( size_t nSize )
		{
			if ( nSize < SObjectArray<T>::GetLength() )
			{
				NotifyRemovedElement( nSize, SArray<T*>::m_nLength - nSize ) ;
			}
			SObjectArray<T>::SetLength( nSize ) ;
		}
		// 配列複製
		void DuplicateArray( const SArray<T*> & src )
		{
			SetLength( src.GetLength() ) ;
			SObjectArray<T>::DuplicateArray( src ) ;
		}
		const SSmartObjectArray<T> & operator = ( const SSmartObjectArray<T> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}
		const SSmartObjectArray<T> & operator = ( const SObjectArray<T> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}
		// 内部バッファを解放する
		void FreeArray( void )
		{
			RemoveAll() ;
			SPointerArray<T>::FreeArray() ;
		}
		// 要素を挿入
		void Insert( size_t nIndex, size_t nCount )
		{
			if ( nIndex > SArray<T*>::m_nLength )
			{
				nIndex = SArray<T*>::m_nLength ;
			}
			NotifyInsertedElement( nIndex, nCount ) ;
			SObjectArray<T>::Insert( nIndex, nCount ) ;
		}
		size_t InsertAt( size_t nIndex, T * e )
		{
			size_t	i = SObjectArray<T>::InsertAt( nIndex, e ) ;
			NotifyInsertedElement( i, 1 ) ;
			return	i ;
		}
		// 要素を末尾に追加
		size_t Add( T * e )
		{
			size_t	i = SObjectArray<T>::Add( e ) ;
			NotifyInsertedElement( i, 1 ) ;
			return	i ;
		}
		size_t Push( T * e )
		{
			size_t	i = SObjectArray<T>::Push( e ) ;
			NotifyInsertedElement( i, 1 ) ;
			return	i ;
		}
		// 配列を末尾に追加
		size_t AddArray( T *const* ppArray, size_t nCount )
		{
			size_t	i = SObjectArray<T>::AddArray( ppArray, nCount ) ;
			NotifyInsertedElement( i, nCount ) ;
			return	i ;
		}
		T * AppendArray( size_t nCount )
		{
			NotifyInsertedElement( SArray<T*>::m_nLength, nCount ) ;
			return	SObjectArray<T>::AppendArray( nCount ) ;
		}
		// 末尾要素を削除し返却
		T * Pop( void )
		{
			if ( SArray<T*>::m_nLength > 0 )
			{
				NotifyRemovedElement( SArray<T*>::m_nLength - 1, 1 ) ;
			}
			return	SObjectArray<T>::Pop() ;
		}
		// 要素設定
		void SetAt( size_t nIndex, T * pObj )
		{
			NotifySwappedElement( nIndex, 1 ) ;
			SObjectArray<T>::SetAt( nIndex, pObj ) ;
		}
		// 要素を置き換える
		T * ExchangeAt( size_t nIndex, T * pObj )
		{
			NotifySwappedElement( nIndex, 1 ) ;
			return	SObjectArray<T>::ExchangeAt( nIndex, pObj ) ;
		}
		// 要素を削除
		void Remove( size_t nIndex, size_t nCount )
		{
			NotifyRemovedElement( nIndex, nCount ) ;
			SObjectArray<T>::Remove( nIndex, nCount ) ;
		}
		void RemoveAt( size_t nIndex )
		{
			Remove( nIndex, 1 ) ;
		}
		void RemoveAll( void )
		{
			Remove( 0, SArray<T*>::m_nLength ) ;
		}
		// 要素を分離
		void Detach( size_t nIndex, size_t nCount )
		{
			NotifyRemovedElement( nIndex, nCount ) ;
			SObjectArray<T>::Detach( nIndex, nCount ) ;
		}
		T * DetachAt( size_t nIndex )
		{
			NotifyRemovedElement( nIndex, 1 ) ;
			return	SObjectArray<T>::DetachAt( nIndex ) ;
		}
		void DetachAll( void )
		{
			NotifyRemovedElement( 0, SArray<T*>::m_nLength ) ;
			SObjectArray<T>::DetachAll() ;
		}
		// 配列を挿入結合する
		void MergeDuplicated
			( size_t nIndex, const SObjectArray<T> & src,
							size_t nFirst = 0, ssize_t nCount = -1 )
		{
			if ( nCount < 0 )
			{
				nCount = (ssize_t) src.GetLength() - (ssize_t) nFirst ;
				if ( nCount <= 0 )
				{
					return ;
				}
			}
			NotifyInsertedElement( nIndex, (size_t) nCount ) ;
			SObjectArray<T>::MergeDuplicated( nIndex, src, nFirst, nCount ) ;
		}

	protected:
		// イテレーターに要素の挿入を通知
		void NotifyInsertedElement( size_t iFirst, size_t nCount )
		{
			Iterator *	pIter = m_pRefFirstIter ;
			while ( pIter != NULL )
			{
				pIter->OnInsertedElement( iFirst, nCount ) ;
				pIter = pIter->GetNextLinked() ;
			}
		}
		// イテレーターに要素の削除を通知
		void NotifyRemovedElement( size_t iFirst, size_t nCount )
		{
			Iterator *	pIter = m_pRefFirstIter ;
			while ( pIter != NULL )
			{
				pIter->OnRemovedElement( iFirst, nCount ) ;
				pIter = pIter->GetNextLinked() ;
			}
		}
		// イテレーターに要素の変更を通知
		void NotifySwappedElement( size_t iFirst, size_t nCount )
		{
			NotifyRemovedElement( iFirst, nCount ) ;
			NotifyInsertedElement( iFirst, nCount ) ;
		}
		// イテレーター参照設定
		void AttachIteratorRef( Iterator * pIter )
		{
			ESLAssert( pIter->GetNextLinked() == NULL ) ;
			ESLAssert( pIter->GetPrevLinked() == NULL ) ;
			if ( m_pRefFirstIter != NULL )
			{
				m_pRefFirstIter->InsertBefore( pIter ) ;
			}
			m_pRefFirstIter = pIter ;
		}
		// イテレーター参照解除
		void DetachIteratorRef( Iterator * pIter )
		{
			if ( m_pRefFirstIter == pIter )
			{
				m_pRefFirstIter = pIter->GetNextLinked() ;
			}
			pIter->Detach() ;
		}

	} ;

}


#endif