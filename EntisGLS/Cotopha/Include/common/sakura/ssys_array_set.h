
#if	!defined(__SAKURA2_ARRAY_SET_H__)
#define	__SAKURA2_ARRAY_SET_H__

//////////////////////////////////////////////////////////////////////////////
// シンプルな集合配列クラス
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SArraySet	: public SArray<T>
	{
	protected:
		#if	defined(__DEBUG__)
			bool	m_flagSorted ;
			size_t	m_nSortedLength ;
		#endif

	public:
		// 構築関数
		SArraySet( void )
			#if	defined(__DEBUG__)
				: m_flagSorted( true ), m_nSortedLength( 0 )
			#endif
			 { }
		SArraySet( const SArraySet<T> & src )
			: SArray<T>( (const SArray<T>&) src )
			#if	defined(__DEBUG__)
				, m_flagSorted( src.m_flagSorted ),
					m_nSortedLength( src.m_nSortedLength )
			#endif
			 {  }
		// 要素検索
		ssize_t Find( const T e ) const
		{
			const size_t	nLength = SArray<T>::m_nLength ;
			T*				pArray = SArray<T>::m_ptrArray ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				if ( pArray[i] == e )
				{
					return	(ssize_t) i ;
				}
			}
			return	-1 ;
		}
		// 挿入指標検索
		size_t OrderIndex( const T e ) const
		{
			#if	defined(__DEBUG__)
				ESLAssert( m_flagSorted && (m_nSortedLength == SArray<T>::m_nLength) ) ;
			#endif
			T*		pArray = SArray<T>::m_ptrArray ;
			ssize_t	iFirst = 0 ;
			ssize_t	iEnd = (ssize_t) SArray<T>::m_nLength - 1 ;
			ssize_t	iMiddle ;
			//
			while ( iFirst <= iEnd )
			{
				iMiddle = ((iFirst + iEnd) >> 1) ;
				if ( pArray[iMiddle] == e )
				{
					return	iMiddle ;
				}
				if ( pArray[iMiddle] > e )
				{
					iEnd = iMiddle - 1 ;
				}
				else
				{
					iFirst = iMiddle + 1 ;
				}
			}
			return	iFirst ;
		}
		// 二分探査（昇順ソート済み）
		ssize_t FindSorted( const T e ) const
		{
			#if	defined(__DEBUG__)
				ESLAssert( m_flagSorted && (m_nSortedLength == SArray<T>::m_nLength) ) ;
			#endif
			T*		pArray = SArray<T>::m_ptrArray ;
			ssize_t	iFirst = 0 ;
			ssize_t	iEnd = (ssize_t) SArray<T>::m_nLength - 1 ;
			ssize_t	iMiddle ;
			//
			while ( iFirst <= iEnd )
			{
				iMiddle = ((iFirst + iEnd) >> 1) ;
				if ( pArray[iMiddle] == e )
				{
					return	iMiddle ;
				}
				if ( pArray[iMiddle] > e )
				{
					iEnd = iMiddle - 1 ;
				}
				else
				{
					iFirst = iMiddle + 1 ;
				}
			}
			return	-1 ;
		}
		// 昇順ソート
		void SortArray( void )
		{
			const size_t	nLength = SArray<T>::m_nLength ;
			T*				pArray = SArray<T>::m_ptrArray ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				size_t	iMin = i ;
				for ( size_t j = i + 1; j < nLength; j ++ )
				{
					if ( pArray[iMin] > pArray[j] )
					{
						iMin = j ;
					}
				}
				T	eTemp = pArray[i] ;
				pArray[i] = pArray[iMin] ;
				pArray[iMin] = eTemp ;
			}
			#if	defined(__DEBUG__)
				m_flagSorted = true ;
				m_nSortedLength = SArray<T>::m_nLength ;
			#endif
		}
		// ソート済み配列から重複要素を削除する
		void NormalizeSorted( void )
		{
			#if	defined(__DEBUG__)
				ESLAssert( m_flagSorted && (m_nSortedLength == SArray<T>::m_nLength) ) ;
			#endif
			const size_t	nLength = SArray<T>::m_nLength ;
			T*				pArray = SArray<T>::m_ptrArray ;
			if ( nLength > 0 )
			{
				size_t	iDst = 1 ;
				for ( size_t i = 0; i < nLength; i ++ )
				{
					if ( pArray[i] != pArray[iDst - 1] )
					{
						if ( iDst != i )
						{
							pArray[iDst] = pArray[i] ;
						}
						iDst ++ ;
					}
				}
				SArray<T>::SetLength( iDst ) ;
			}
			#if	defined(__DEBUG__)
				m_nSortedLength = SArray<T>::m_nLength ;
			#endif
		}
		// 要素を追加
		size_t Add( const T e )
		{
			ssize_t	i = Find( e ) ;
			if ( i >= 0 )
			{
				return	(size_t) i ;
			}
			#if	defined(__DEBUG__)
				m_flagSorted = false ;
			#endif
			return	SArray<T>::Add( e ) ;
		}
		size_t QuickAdd( const T e )
		{
			#if	defined(__DEBUG__)
				m_flagSorted = false ;
			#endif
			return	SArray<T>::Add( e ) ;
		}
		// 要素を追加（ソート済み配列へ挿入）
		size_t AddSorted( const T e )
		{
			#if	defined(__DEBUG__)
				ESLAssert( m_flagSorted && (m_nSortedLength == SArray<T>::m_nLength) ) ;
			#endif
			size_t	nIndex = OrderIndex( e ) ;
			if ( (nIndex < SArray<T>::m_nLength)
				&& (SArray<T>::m_ptrArray[nIndex] == e) )
			{
				return	nIndex ;
			}
			SArray<T>::InsertAt( nIndex, e ) ;
			#if	defined(__DEBUG__)
				m_nSortedLength = SArray<T>::m_nLength ;
			#endif
			return	nIndex ;
		}
		// 要素を追加（追加してソート正規化）
		void AppendSetNormalize( const SArraySet<T> & src )
		{
			SArray<T>::AddArray( src.GetConstArray(), src.GetLength() ) ;
			SortArray() ;
			NormalizeSorted() ;
		}
		// 要素を削除
		#if	defined(__DEBUG__)
		void Remove( size_t nIndex, size_t nCount )
		{
			SArray<T>::Remove( nIndex, nCount ) ;
			#if	defined(__DEBUG__)
				m_nSortedLength = SArray<T>::m_nLength ;
			#endif
		}
		void RemoveAt( size_t nIndex )
		{
			Remove( nIndex, 1 ) ;
		}
		void RemoveAll( void ) 
		{
			SArray<T>::m_nLength = 0 ;
			#if	defined(__DEBUG__)
				m_flagSorted = true ;
				m_nSortedLength = 0 ;
			#endif
		}
		#else
		void RemoveAt( size_t nIndex )
		{
			SArray<T>::Remove( nIndex, 1 ) ;
		}
		#endif
		bool RemoveAs( const T e )
		{
			ssize_t	i = Find( e ) ;
			if ( i >= 0 )
			{
				RemoveAt( (size_t) i ) ;
				return	true ;
			}
			return	false ;
		}
		bool RemoveSortedAs( const T e )
		{
			#if	defined(__DEBUG__)
				ESLAssert( m_flagSorted && (m_nSortedLength == SArray<T>::m_nLength) ) ;
			#endif
			ssize_t	i = FindSorted( e ) ;
			if ( i >= 0 )
			{
				RemoveAt( (size_t) i ) ;
				return	true ;
			}
			return	false ;
		}
		// 要素を削除（ソート正規化済み配列の要素を削除）
		void RemoveSetBySorted( const SArraySet<T> & src )
		{
			#if	defined(__DEBUG__)
				ESLAssert( src.m_flagSorted && (src.m_nSortedLength == src.m_nLength) ) ;
			#endif
			const size_t	nLength = SArray<T>::m_nLength ;
			T*				pArray = SArray<T>::m_ptrArray ;
			size_t			iDst = 0 ;
			for ( size_t i = 0; i < nLength; i ++ )
			{
				if ( src.FindSorted( pArray[i] ) < 0 )
				{
					if ( iDst < i )
					{
						pArray[iDst] = pArray[i] ;
					}
					iDst ++ ;
				}
			}
			SArray<T>::SetLength( iDst ) ;
			#if	defined(__DEBUG__)
				m_nSortedLength = SArray<T>::m_nLength ;
			#endif
		}
		// 配列複製
		const SArraySet<T> & operator = ( const SArraySet<T> & src )
		{
			SArray<T>::operator = ( src ) ;
			#if	defined(__DEBUG__)
				m_flagSorted = src.m_flagSorted ;
				m_nSortedLength = src.m_nSortedLength ;
			#endif
			return	*this ;
		}

	} ;
} ;

#endif

