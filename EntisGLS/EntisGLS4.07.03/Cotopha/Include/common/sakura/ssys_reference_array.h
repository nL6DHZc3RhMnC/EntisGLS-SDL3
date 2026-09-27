
#if	!defined(__SAKURA2_REFERENCE_ARRAY_H__)
#define	__SAKURA2_REFERENCE_ARRAY_H__

#include <sakura/ssys_smart_object_array.h>


//////////////////////////////////////////////////////////////////////////////
// オブジェクト参照配列
//（オブジェクトが破棄されると配列要素は自動的にヌルになる）
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SReferenceArray
						: public SSmartObjectArray<SSyncReference>
	{
	public:
		class	Iterator	: public SSmartObjectArray<SSyncReference>::Iterator
		{
		public:
			Iterator( const SReferenceArray<T> * pArray = nullptr, size_t nIndex = 0 )
				: SSmartObjectArray<SSyncReference>::Iterator( pArray, nIndex ) { }
			Iterator( const SReferenceArray<T> & ref, size_t nIndex = 0 )
				: SSmartObjectArray<SSyncReference>::Iterator( ref, nIndex ) { }
			Iterator( const Iterator& iter )
				: SSmartObjectArray<SSyncReference>::Iterator( iter ) { }
			T * Next( void )
			{
				SSyncReference *	pRef =
					SSmartObjectArray<SSyncReference>::Iterator::Next() ;
				return	(pRef != nullptr) ? ESLTypeCast<T>( pRef->GetReference() ) : nullptr ;
			}
			T * Prev( void )
			{
				SSyncReference *	pRef =
					SSmartObjectArray<SSyncReference>::Iterator::Prev() ;
				return	(pRef != nullptr) ? ESLTypeCast<T>( pRef->GetReference() ) : nullptr ;
			}
			T * Element( void ) const
			{
				SSyncReference *	pRef =
					SSmartObjectArray<SSyncReference>::Iterator::Element() ;
				return	(pRef != nullptr) ? ESLTypeCast<T>( pRef->GetReference() ) : nullptr ;
			}
		} ;

	public:
		// 構築関数
		SReferenceArray( void )
		{
		}
		SReferenceArray( const SReferenceArray<T> & src )
		{
			DuplicateArray( src ) ;
		}
		SReferenceArray( const SPointerArray<T> & src )
		{
			DuplicateArray( src ) ;
		}
		// イテレーター
		Iterator Begin( void ) const
		{
			return	Iterator( this, 0 ) ;
		}
		Iterator End( void ) const
		{
			return	Iterator( this, SArray<SSyncReference*>::m_nLength ) ;
		}
		// 要素取得
		T * GetAt( size_t nIndex ) const
		{
			if ( (nIndex >= 0)
				& (nIndex < SArray<SSyncReference*>::m_nLength) )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex] ;
				if ( pRef != NULL )
				{
					return	ESLTypeCast<T>( pRef->GetReference() ) ;
				}
			}
			return	NULL ;
		}
		T * GetLastAt( size_t nIndex = 0 ) const
		{
			if ( (nIndex >= 0)
					& (nIndex < SArray<SSyncReference*>::m_nLength) )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray
								[SArray<SSyncReference*>::m_nLength - nIndex - 1] ;
				if ( pRef != NULL )
				{
					return	ESLTypeCast<T>( pRef->GetReference() ) ;
				}
			}
			return	NULL ;
		}
		T & At( size_t nIndex ) const
		{
			ESLAssert( (nIndex >= 0)
					&& (nIndex < SArray<SSyncReference*>::m_nLength) ) ;
			return	*(ESLTypeCast<T>(SArray<SSyncReference*>::m_ptrArray[nIndex]->GetReference())) ;
		}
		// 末尾要素を削除し返却
		T * Pop( void )
		{
			SSyncReference *	pRef = SSmartObjectArray<SSyncReference>::Pop() ;
			if ( pRef != NULL )
			{
				ESLAssert( ESLTypeCast<SSmartObject>( pRef->GetReference() ) == nullptr ) ;
				T *	pObj = ESLTypeCast<T>( pRef->GetReference() ) ;
				delete	pRef ;
				return	pObj ;
			}
			return	NULL ;
		}

	public:
		// 要素設定
		void SetAt( size_t nIndex, T * pObj )
		{
			if ( nIndex >= SArray<SSyncReference*>::m_nLength )
			{
				SetLength( nIndex + 1 ) ;
			}
			else // if ( nIndex >= 0 )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex] ;
				if ( pRef != NULL )
				{
					pRef->SetReference( (SObject*) pObj ) ;
					return ;
				}
			}
			/*
			else
			{
				return ;
			}
			*/
			SArray<SSyncReference*>::m_ptrArray[nIndex] =
								new SSyncReference( (SObject*) pObj ) ;
		}
		void SmartSetAt( size_t nIndex, T * pObj )
		{
			if ( nIndex >= SArray<SSyncReference*>::m_nLength )
			{
				SetLength( nIndex + 1 ) ;
			}
			else // if ( nIndex >= 0 )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex] ;
				if ( pRef != NULL )
				{
					pRef->SetReference
						( new SSmartObject( (SObject*) pObj ) ) ;
					return ;
				}
			}
			/*
			else
			{
				return ;
			}
			*/
			SArray<SSyncReference*>::m_ptrArray[nIndex] =
				new SSyncReference( new SSmartObject( (SObject*) pObj ) ) ;
		}
		// 要素を挿入
		void InsertAt( size_t nIndex, T * pObj )
		{
			if ( nIndex > m_nLength )
			{
				nIndex = m_nLength ;
			}
			Insert( nIndex, 1 ) ;
			SArray<SSyncReference*>::m_ptrArray[nIndex]
								= new SSyncReference( (SObject*) pObj ) ;
		}
		void SmartInsertAt( size_t nIndex, T * pObj )
		{
			if ( nIndex > m_nLength )
			{
				nIndex = m_nLength ;
			}
			Insert( nIndex, 1 ) ;
			SArray<SSyncReference*>::m_ptrArray[nIndex]
				= new SSyncReference( new SSmartObject( (SObject*) pObj ) ) ;
		}
		// 要素を末尾に追加
		size_t Add( T * pObj )
		{
			uint32_t	nIndex = SArray<SSyncReference*>::m_nLength ;
			SetLength( nIndex + 1 ) ;
			SArray<SSyncReference*>::m_ptrArray[nIndex]
								= new SSyncReference( (SObject*) pObj ) ;
			return	nIndex ;
		}
		size_t Push( T * pObj )
		{
			uint32_t	nIndex = SArray<SSyncReference*>::m_nLength ;
			SetLength( nIndex + 1 ) ;
			SArray<SSyncReference*>::m_ptrArray[nIndex]
								= new SSyncReference( (SObject*) pObj ) ;
			return	nIndex ;
		}
		size_t SmartAdd( T * pObj )
		{
			uint32_t	nIndex = SArray<SSyncReference*>::m_nLength ;
			SetLength( nIndex + 1 ) ;
			SArray<SSyncReference*>::m_ptrArray[nIndex]
				= new SSyncReference( new SSmartObject( (SObject*) pObj ) ) ;
			return	nIndex ;
		}
		// 要素はスマート要素か？
		bool IsSmartElementAt( size_t nIndex ) const
		{
			if ( (nIndex >= 0)
					& (nIndex < SArray<SSyncReference*>::m_nLength) )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex] ;
				if ( pRef != NULL )
				{
					SObject *	pObj = pRef->GetReference() ;
					if ( pObj != NULL )
					{
						return	(pObj->GetESLRuntimeClass()
									== &(ESL_RUNTIME_CLASS(SSmartObject))) ;
					}
				}
			}
			return	false ;
		}
		bool IsSmartElementOf( T * pObj ) const
		{
			return	IsSmartElementAt( (size_t) FindPtr( pObj ) ) ;
		}

	public:
		// 要素を検索
		ssize_t FindPtr( T * pObj, size_t nFirst = 0 ) const
		{
			const size_t
				nLength = SArray<SSyncReference*>::m_nLength ;
			SSyncReference**
				pArray = SArray<SSyncReference*>::m_ptrArray ;
			for ( size_t i = nFirst; i < nLength; i ++ )
			{
				SSyncReference *	pRef = pArray[i] ;
				if ( (pRef != NULL)
					&& (ESLTypeCast<T>(pRef->GetReference()) == pObj) )
				{
					return	(ssize_t) i ;
				}
			}
			return	-1 ;
		}
		// ヌル要素を削除して詰める
		void TrimEmpty( void )
		{
			const size_t
				nLength = SArray<SSyncReference*>::m_nLength ;
			SSyncReference**
				pArray = SArray<SSyncReference*>::m_ptrArray ;
			uint32_t	i = 0, j = 0 ;
			while ( i < nLength )
			{
				SSyncReference *	pRef = pArray[i] ;
				if ( pRef != NULL )
				{
					if ( pRef->GetReference() != NULL )
					{
						pArray[j ++] = pArray[i] ;
					}
					else
					{
						delete	pRef ;
					}
				}
				i ++ ;
			}
			SArray<SSyncReference*>::m_nLength = j ;
		}

	public:
		// 配列複製
		void DuplicateArray( const SReferenceArray<T> & src )
		{
			size_t	nLength = src.GetLength() ;
			SetLength( nLength ) ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				SSyncReference *	pRef = src.m_ptrArray[i] ;
				if ( pRef != NULL )
				{
					SObjectArray<SSyncReference>::SetAt
								( i, new SSyncReference(*pRef) ) ;
				}
			}
		}
		void DuplicateArray( const SPointerArray<T> & src )
		{
			size_t	nLength = src.GetLength() ;
			SetLength( nLength ) ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				T *	pSrc = src.GetAt(i) ;
				if ( pSrc != NULL )
				{
					SetAt( i, pSrc ) ;
				}
			}
		}
		const SReferenceArray<T> & operator = ( const SReferenceArray<T> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}
		const SReferenceArray<T> & operator = ( const SPointerArray<T> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}

	public:
		// 要素を分離
		void Detach( size_t nIndex, size_t nCount )
		{
			if ( nIndex >= SArray<SSyncReference*>::m_nLength )
			{
				return ;
			}
			if ( (nIndex + nCount) > SArray<SSyncReference*>::m_nLength )
			{
				nCount = SArray<SSyncReference*>::m_nLength - nIndex ;
			}
			for ( size_t i = 0; i < nCount; i ++ )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex + i] ;
				if ( pRef != NULL )
				{
					SSmartObject *	pSObj =
						ESLTypeCast<SSmartObject>( pRef->GetReference() ) ;
					if ( pSObj != NULL )
					{
						pSObj->DetachObject() ;
					}
				}
			}
			Remove( nIndex, nCount ) ;
		}
		T * DetachAt( size_t nIndex )
		{
			if ( nIndex < SArray<SSyncReference*>::m_nLength )
			{
				SSyncReference *
					pRef = SArray<SSyncReference*>::m_ptrArray[nIndex] ;
				if ( pRef != NULL )
				{
					T *	pObj ;
					SSmartObject *	pSObj =
						ESLTypeCast<SSmartObject>( pRef->GetReference() ) ;
					if ( pSObj != NULL )
					{
						ESLObject *	pESLObj = pSObj->DetachObject() ;
						pObj = ESLTypeCast<T>( pESLObj ) ;
						if ( pObj == NULL )
						{
							delete	pESLObj ;
						}
					}
					else
					{
						pObj = ESLTypeCast<T>( pRef->GetReference() ) ;
					}
					RemoveAt( nIndex ) ;
					return	pObj ;
				}
				RemoveAt( nIndex ) ;
			}
			return	NULL ;
		}
		void DetachAll( void )
		{
			Detach( 0, SArray<SSyncReference*>::m_nLength ) ;
		}

	} ;

}

#endif
