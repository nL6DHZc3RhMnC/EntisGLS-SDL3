

#if	!defined(__SAKURA2_INDEXED_ARRAY_H__)
#define	__SAKURA2_INDEXED_ARRAY_H__


//////////////////////////////////////////////////////////////////////////////
// オブジェクトポインタ配列（オブジェクトは配列が破棄する）
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T, class S> class SIndexedArray	: public SObjectArray<T>
	{
	protected:
		SArray<int>	m_indexes ;		// 検索用インデックス配列（ソート）
		bool		m_modified ;	// インデックスの更新フラグ

	public:
		// 構築関数
		SIndexedArray( void ) : m_modified(false) {}

	public:
		// 要素を末尾に追加
		size_t Add( T* pObj )
		{
			if ( m_modified )
			{
				UpdateAllIndex() ;
			}
			size_t	nIndex = SObjectArray<T>::Add( pObj ) ;
			m_indexes.InsertAt( OrderIndex( *pObj ), (int) nIndex ) ;
			return	nIndex ;
		}
		size_t FastAdd( T* pObj )
		{
			m_modified = true ;
			return	SObjectArray<T>::Add( pObj ) ;
		}
		size_t Push( T* pObj )
		{
			return	Add( pObj ) ;
		}
		// 要素を削除
		void RemoveAll( void )
		{
			SObjectArray<T>::RemoveAll() ;
			m_indexes.RemoveAll() ;
			m_modified = false ;
		}
		// 要素を分離
		void DetachAll( void )
		{
			SObjectArray<T>::DetachAll() ;
			m_indexes.RemoveAll() ;
			m_modified = false ;
		}

	public:
		// 指標検索
		ssize_t OrderIndex( S tag ) const
		{
			T *			pElement ;
			int			iFirst, iEnd, iMiddle = 0 ;
			const int*	pIndexes = m_indexes.GetConstArray() ;
			iFirst = 0 ;
			iEnd = (int) m_indexes.GetLength() - 1 ;
			//
			while ( iFirst <= iEnd )
			{
				iMiddle = ((iFirst + iEnd) >> 1) ;
				pElement = SArray<T*>::m_ptrArray[pIndexes[iMiddle]] ;
				ESLAssert( pElement != NULL ) ;
				//
				if ( *pElement > tag )
				{
					iEnd = iMiddle - 1 ;
				}
				else if ( *pElement < tag )
				{
					iFirst = iMiddle + 1 ;
				}
				else
				{
					return	iMiddle ;
				}
			}
			return	iFirst ;
		}
		ssize_t FindIndex( S tag ) const
		{
			ssize_t	nIndex = OrderIndex( tag ) ;
			if ( (size_t) nIndex < m_indexes.GetLength() )
			{
				nIndex = m_indexes.At(nIndex) ;
				if ( (size_t) nIndex < SArray<T*>::m_nLength )
				{
					if ( *(SArray<T*>::m_ptrArray[nIndex]) == tag )
					{
						return	nIndex ;
					}
				}
			}
			return	-1 ;
		}
		// インデックスの更新は必要か？
		bool IsNeedsIndexUpdate( void ) const
		{
			return	m_modified ;
		}
		// インデックス更新
		void UpdateAllIndex( void )
		{
			if ( m_modified )
			{
				m_indexes.RemoveAll() ;
				m_modified = false ;
				//
				for ( uint32_t i = 0; i < SArray<T*>::m_nLength; i ++ )
				{
					ESLAssert( SArray<T*>::m_ptrArray[i] != NULL ) ;
					m_indexes.InsertAt
						( OrderIndex( *(SArray<T*>::m_ptrArray[i]) ), i ) ;
				}
			}
		}

	public:
		// 要素設定
		void SetAt( size_t nIndex, T * pObj )
		{
			m_modified = true ;
			SObjectArray<T>::SetAt( nIndex, pObj ) ;
		}
		void InsertAt( size_t nIndex, T * pObj )
		{
			m_modified = true ;
			SObjectArray<T>::InsertAt( nIndex, pObj ) ;
		}
		// 配列のサイズを設定
		void SetLength( size_t nSize )
		{
			m_modified = true ;
			SObjectArray<T>::SetLength( nSize ) ;
		}
		// 末尾要素を削除し返却
		T * Pop( void )
		{
			m_modified = true ;
			return	SObjectArray<T>::Pop() ;
		}
		// 要素を削除
		void Remove( size_t nIndex, size_t nCount )
		{
			SObjectArray<T>::Remove( nIndex, nCount ) ;
			m_modified = true ;
		}
		void RemoveAt( size_t nIndex )
		{
			SObjectArray<T>::RemoveAt( nIndex ) ;
			m_modified = true ;
		}
		// ヌル要素を削除して詰める
		void TrimEmpty( void )
		{
			SObjectArray<T>::TrimEmpty() ;
			m_modified = true ;
		}
		// 要素を分離
		void Detach( size_t nIndex, size_t nCount )
		{
			SObjectArray<T>::Detach( nIndex, nCount ) ;
			m_modified = true ;
		}
		T * DetachAt( size_t nIndex )
		{
			m_modified = true ;
			return	SObjectArray<T>::DetachAt( nIndex ) ;
		}
	} ;

} ;

#endif

