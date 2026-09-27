

#if	!defined(__SAKURA2_POINTER_ARRAY_H__)
#define	__SAKURA2_POINTER_ARRAY_H__


//////////////////////////////////////////////////////////////////////////////
// ポインタ配列
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SPointerArray	: public SArray<T*>
	{
	public:
		// 構築関数
		SPointerArray( void ) {}
		SPointerArray( const SPointerArray<T>& src )
			: SArray<T*>( (const SArray<T*>&) src ) { }
		// 要素取得
		T * GetAt( size_t nIndex ) const
		{
			if ( (nIndex >= 0)
					& (nIndex < SArray<T*>::m_nLength) )
			{
				return	SArray<T*>::m_ptrArray[nIndex] ;
			}
			return	NULL ;
		}
		T * GetLastAt( size_t nIndex = 0 ) const
		{
			if ( (nIndex >= 0)
					& (nIndex < SArray<T*>::m_nLength) )
			{
				return	SArray<T*>::m_ptrArray
							[SArray<T*>::m_nLength - nIndex - 1] ;
			}
			return	NULL ;
		}
		T & At( size_t nIndex ) const
		{
			ESLAssert( (nIndex >= 0)
					&& (nIndex < SArray<T*>::m_nLength) ) ;
			ESLAssert( SArray<T*>::m_ptrArray[nIndex] != NULL ) ;
			return	*(SArray<T*>::m_ptrArray[nIndex]) ;
		}
		T & LastAt( size_t nIndex = 0 ) const
		{
			ESLAssert( (nIndex >= 0)
					&& (nIndex < SArray<T*>::m_nLength) ) ;
			ESLAssert( SArray<T*>::m_ptrArray[SArray<T*>::m_nLength - nIndex - 1] != NULL ) ;
			return	*(SArray<T*>::m_ptrArray[SArray<T*>::m_nLength - nIndex - 1]) ;
		}
		// 要素を置き換える
		T * ExchangeAt( size_t nIndex, T * pObj )
		{
			if ( nIndex >= SArray<T*>::m_nLength )
			{
				SArray<T*>::SetLength( nIndex + 1 ) ;
			}
			T *	pOld = SArray<T*>::m_ptrArray[nIndex] ;
			SArray<T*>::m_ptrArray[nIndex] = pObj ;
			return	pOld ;
		}
		// 末尾要素を削除し返却
		T * Pop( void )
		{
			if ( SArray<T*>::m_nLength > 0 )
			{
				return	SArray<T*>::m_ptrArray[-- SArray<T*>::m_nLength] ;
			}
			return	NULL ;
		}
		// 要素を検索
		ssize_t FindPtr( T * pObj, size_t nFirst = 0 ) const
		{
			const size_t	nLength = SArray<T*>::m_nLength ;
			T**				pArray = SArray<T*>::m_ptrArray ;
			for ( size_t i = nFirst; i < nLength; i ++ )
			{
				if ( pArray[i] == pObj )
				{
					return	(ssize_t) i ;
				}
			}
			return	-1 ;
		}
		ssize_t Find( const T& obj1, bool (*pfnCmp)(const T& obj1, const T& obj2), size_t nFirst = 0 ) const
		{
			const size_t	nLength = SArray<T*>::m_nLength ;
			T**				pArray = SArray<T*>::m_ptrArray ;
			for ( size_t i = nFirst; i < nLength; i ++ )
			{
				if ( (pArray[i] != nullptr)
					&& pfnCmp( obj1, *(pArray[i]) ) )
				{
					return	(ssize_t) i ;
				}
			}
			return	-1 ;
		}
		// 選択ソート
		void Sort( bool (*pfnCmp)(const T& obj1, const T& obj2) )
		{
			const size_t	nLength = SArray<T*>::m_nLength ;
			T**				pArray = SArray<T*>::m_ptrArray ;
			for ( size_t i = 0; i + 1 < nLength; i ++ )
			{
				T*	pPrimary = pArray[i] ;
				if ( pPrimary == nullptr )
				{
					continue ;
				}
				size_t	iPrimary = i ;
				for ( size_t j = i + 1; j < nLength; j ++ )
				{
					if ( (pArray[j] != nullptr)
						&& pfnCmp( *(pArray[j]), *pPrimary ) )
					{
						iPrimary = j ;
						pPrimary = pArray[j] ;
					}
				}
				if ( iPrimary != i )
				{
					SArray<T*>::Swap( i, iPrimary ) ;
				}
			}
		}
		// ヌル要素を削除して詰める
		void TrimEmpty( void )
		{
			const uint32_t	nLength = SArray<T*>::m_nLength ;
			T**				pArray = SArray<T*>::m_ptrArray ;
			uint32_t		i = 0, j = 0 ;
			while ( i < nLength )
			{
				if ( pArray[i] != NULL )
				{
					pArray[j ++] = pArray[i] ;
				}
				i ++ ;
			}
			SArray<T*>::m_nLength = j ;
		}
	} ;

} ;

#endif

