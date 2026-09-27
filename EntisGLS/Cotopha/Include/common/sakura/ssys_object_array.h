

#if	!defined(__SAKURA2_OBJECT_ARRAY_H__)
#define	__SAKURA2_OBJECT_ARRAY_H__


//////////////////////////////////////////////////////////////////////////////
// オブジェクトポインタ配列（オブジェクトは配列が破棄する）
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SObjectArray	: public SPointerArray<T>
	{
	public:
		// 構築関数
		SObjectArray( void )
		{
		}
		SObjectArray( const SObjectArray<T> & src )
		{
			DuplicateArray( src ) ;
		}
		// 消滅関数
		~SObjectArray( void )
		{
			if ( SArray<T*>::m_ptrArray != NULL )
			{
				Remove( 0, SArray<T*>::m_nLength ) ;
				#if	defined(__DEBUG__)
					SPointerArray<T>::FreeArray() ;
				#else
					esl_free( SArray<T*>::m_ptrArray ) ;
				#endif
				SArray<T*>::m_ptrArray = NULL ;
			}
		}
		// 要素設定
		void SetAt( size_t nIndex, T * pObj )
		{
			if ( nIndex >= SArray<T*>::m_nLength )
			{
				SetLength( nIndex + 1 ) ;
			}
			delete	SArray<T*>::m_ptrArray[nIndex] ;
			SArray<T*>::m_ptrArray[nIndex] = pObj ;
		}
		// 配列のサイズを設定
		void SetLength( size_t nSize )
		{
			if ( nSize < SArray<T*>::m_nLength )
			{
				uint32_t	nLength = SArray<T*>::m_nLength ;
				for ( size_t i = nSize; i < nLength; i ++ )
				{
					delete SArray<T*>::m_ptrArray[i] ;
				}
				SArray<T*>::m_nLength = (uint32_t) nSize ;
			}
			else
			{
				SPointerArray<T>::SetLength( nSize ) ;
			}
		}
		// 配列複製
		void DuplicateArray( const SArray<T*> & src )
		{
			SetLength( src.GetLength() ) ;
			for ( size_t i = 0; i < SArray<T*>::m_nLength; i ++ )
			{
				T *	pSrc = *(src.GetAt(i)) ;
				if ( pSrc != NULL )
				{
					SetAt( i, new T( *pSrc ) ) ;
				}
			}
		}
		const SObjectArray<T> & operator = ( const SObjectArray<T> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}
		const SObjectArray<T> & operator = ( const SArray<T*> & src )
		{
			DuplicateArray( src ) ;
			return	*this ;
		}
		// 配列要素移動
		void MoveArrayFrom
			( size_t nIndex, SObjectArray<T> & src,
					size_t nFirst = 0, ssize_t nCount = -1 )
		{
			if ( nFirst < src.GetLength() )
			{
				if ( nCount < 0 )
				{
					nCount = (ssize_t) (src.GetLength() - nFirst) ;
				}
				SPointerArray<T>::Merge( nIndex, src, nFirst, (size_t) nCount ) ;
				src.Detach( nFirst, (size_t) nCount ) ;
			}
		}
		// 内部バッファを解放する
		void FreeArray( void )
		{
			RemoveAll() ;
			SPointerArray<T>::FreeArray() ;
		}
		// 要素を削除
		void Remove( size_t nIndex, size_t nCount )
		{
			if ( nIndex >= SArray<T*>::m_nLength )
			{
				return ;
			}
			if ( (nIndex + nCount) > SArray<T*>::m_nLength )
			{
				nCount = SArray<T*>::m_nLength - nIndex ;
			}
			for ( size_t i = 0; i < nCount; i ++ )
			{
				delete SArray<T*>::m_ptrArray[nIndex + i] ;
			}
			size_t	nRight = SArray<T*>::m_nLength - (nIndex + nCount) ;
			if ( nRight > 0 )
			{
				eslMoveMemory
					( SArray<T*>::m_ptrArray + nIndex,
						SArray<T*>::m_ptrArray + (nIndex + nCount),
						nRight * sizeof(T*) ) ;
			}
			SArray<T*>::m_nLength -= (uint32_t) nCount ;
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
			SPointerArray<T>::Remove( nIndex, nCount ) ;
		}
		T * DetachAt( size_t nIndex )
		{
			if ( (nIndex >= 0)
					& ((unsigned int) nIndex < SArray<T*>::m_nLength) )
			{
				T *	pObj = SArray<T*>::m_ptrArray[nIndex] ;
				SPointerArray<T>::Remove( nIndex, 1 ) ;
				return	pObj ;
			}
			return	NULL ;
		}
		void DetachAll( void )
		{
			SPointerArray<T>::Remove( 0, SArray<T*>::m_nLength ) ;
		}

	protected:
		// 配列を末尾に追加
		size_t AddArray( T*const* pArray, size_t nCount )
		{
			return	SPointerArray<T>::AddArray( pArray, nCount ) ;
		}
		// 配列を挿入結合する
		void Merge( size_t nIndex, const SObjectArray<T> & src,
							size_t nFirst = 0, ssize_t nCount = -1 )
		{
			SPointerArray<T>::Merge( nIndex, src, nFirst, nCount ) ;
		}

	public:
		// 配列を挿入結合する
		void MergeDuplicated
			( size_t nIndex, const SObjectArray<T> & src,
							size_t nFirst = 0, ssize_t nCount = -1 )
		{
			if ( nCount <= 0 )
			{
				nCount = (ssize_t) src.GetLength() - (ssize_t) nFirst ;
				if ( nCount <= 0 )
				{
					return ;
				}
			}
			for ( size_t i = 0; i < (size_t) nCount; i ++ )
			{
				T *	pSrc = src.GetAt(i) ;
				if ( pSrc != NULL )
				{
					pSrc = new T( *pSrc ) ;
				}
				SPointerArray<T>::InsertAt( nIndex + i, pSrc ) ;
			}
		}

	} ;


	class SStringArray	: public SObjectArray<SString>
	{
	public:
		// 構築関数
		SStringArray( void )
		{
		}
		SStringArray( const SStringArray & src )
			: SObjectArray<SString>( src )
		{
		}
		// 要素を取得
		const wchar_t * GetStringAt( size_t nIndex ) const
		{
			SString *	pStr = SObjectArray<SString>::GetAt( nIndex ) ;
			return	(pStr != nullptr) ? (const wchar_t*) *pStr : nullptr ;
		}
		// 要素を設定
		void SetStringAt( size_t nIndex, const wchar_t * pwszStr )
		{
			SString *	pStr = SObjectArray<SString>::GetAt( nIndex ) ;
			if ( pStr != nullptr )
			{
				*pStr = pwszStr ;
			}
			else
			{
				SObjectArray<SString>::SetAt( nIndex, new SString( pwszStr ) ) ;
			}
		}
		size_t AppendString( const wchar_t * pwszStr )
		{
			return	SObjectArray<SString>::Add( new SString( pwszStr ) ) ;
		}
		// 要素を検索
		ssize_t FindString( const wchar_t * pwszStr, size_t nFirst = 0 ) const
		{
			const size_t	nLength = SArray<SString*>::m_nLength ;
			SString**		pArray = SArray<SString*>::m_ptrArray ;
			for ( size_t i = nFirst; i < nLength; i ++ )
			{
				if ( (pArray[i] != nullptr)
					&& (*(pArray[i]) == pwszStr) )
				{
					return	(ssize_t) i ;
				}
			}
			return	-1 ;
		}
		// 昇順ソート
		void SortAscending( void )
		{
			SPointerArray<SString>::Sort
				( []( const SString& obj1, const SString& obj2 )
					{ return obj1 < obj2 ; } ) ;
		}

	} ;

} ;

#endif

