
#if	!defined(__SAKURA2_ARRAY_H__)
#define	__SAKURA2_ARRAY_H__

//////////////////////////////////////////////////////////////////////////////
//
// シンプルな配列基底クラス
// 　ただのバッファ的なメモリ配列を保持するクラスです
// 　高速な代りに構築関数や消滅関数、代入演算子は一部無視されます
//
//	※この配列には、コンストラクタ・デストラクタ・代入演算子の
//	　処理が必須なクラスを直接要素にするべきではありません
//
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T> class SArray
	{
	protected:
		T*			m_ptrArray ;
		uint32_t	m_nLength ;
		uint32_t	m_nBufSize ;

	public:
		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

		// 構築関数
		SArray( void )
		{
			m_ptrArray = NULL ;
			m_nLength = 0 ;
			m_nBufSize = 0 ;
		}
		SArray( const SArray<T> & src )
		{
			m_ptrArray = NULL ;
			m_nLength = 0 ;
			m_nBufSize = 0 ;
			//
			if ( src.m_nLength > 0 )
			{
				SetLimit( src.m_nBufSize ) ;
				m_nLength = src.m_nLength ;
				::eslMoveMemory
					( m_ptrArray, src.m_ptrArray, m_nLength * sizeof(T) ) ;
			}
		}
		// 消滅関数
		~SArray( void )
		{
			if ( m_ptrArray != NULL )
			{
				#if	defined(__DEBUG__)
					CheckArrayBoundary() ;
					esl_free( ((uint32_t*)m_ptrArray) - 2 ) ;
				#else
					esl_free( m_ptrArray ) ;
				#endif
				m_ptrArray = NULL ;
			}
			#if	defined(__DEBUG__)
			else
			{
				ESLAssert( m_nLength == 0 ) ;
				ESLAssert( m_nBufSize == 0 ) ;
			}
			#endif
		}
		// 内部バッファを解放する
		void FreeArray( void )
		{
			if ( m_ptrArray != NULL )
			{
				#if	defined(__DEBUG__)
					CheckArrayBoundary() ;
					esl_free( ((uint32_t*)m_ptrArray) - 2 ) ;
				#else
					esl_free( m_ptrArray ) ;
				#endif
				m_ptrArray = NULL ;
			}
			m_nLength = 0 ;
			m_nBufSize = 0 ;
		}
		// 配列へのポインタを取得
		T * GetArray( void ) const
		{
			return	m_ptrArray ;
		}
		T * GetArray( size_t nLength )
		{
			if ( m_nLength < nLength )
			{
				SetLength( nLength ) ;
			}
			return	m_ptrArray ;
		}
		// 取得した配列ポインタの配列内容を変更し終えた時に呼び出す
		// （何度も呼び出し可／GetArray せずに呼び出しも可／FinishArray 後もポインタは有効）
		void FinishArray( void )
		{
			#if	defined(__DEBUG__)
				CheckArrayBoundary() ;
			#endif
		}
		// 配列へのポインタを取得（後で FinishArray を呼び出さない場合）
		T * GetArrayPtr( void ) const
		{
			return	m_ptrArray ;
		}
		// 配列へのポインタを取得（変更不可）
		const T * GetConstArray( void ) const
		{
			return	m_ptrArray ;
		}
		operator const T * ( void ) const
		{
			return	m_ptrArray ;
		}
		// 配列の長さを取得
		size_t GetLength( void ) const
		{
			return m_nLength ;
		}
		// 配列の内部バッファの長さを取得
		size_t GetLimit( void ) const
		{
			return	m_nBufSize ;
		}
		// 配列複製
		const SArray<T> & operator = ( const SArray<T> & src )
		{
			if ( src.m_nLength > 0 )
			{
				SetLimit( src.m_nBufSize ) ;
				m_nLength = src.m_nLength ;
				::eslMoveMemory
					( m_ptrArray, src.m_ptrArray, m_nLength * sizeof(T) ) ;
			}
			else
			{
				FreeArray() ;
			}
			return	*this ;
		}
		// 配列のサイズを設定
		void SetLength( size_t nSize )
		{
			if ( nSize > m_nBufSize )
			{
				uint32_t	nBufSize = m_nBufSize + (m_nBufSize >> 1) ;
				nBufSize = (nBufSize + 0x07) & ~0x07 ;
				if ( nBufSize < nSize )
				{
					nBufSize = (uint32_t) (nSize + 0x07) & ~0x07 ;
				}
				SetLimit( nBufSize ) ;
			}
			if ( nSize > m_nLength )
			{
				::eslFillMemory
					( m_ptrArray + m_nLength,
						0, (nSize - m_nLength) * sizeof(T) ) ;
			}
			m_nLength = (uint32_t) nSize ;
		}
		// 配列の内部バッファのサイズを設定
		void SetLimit( size_t nLimit )
		{
			if ( nLimit > m_nBufSize )
			{
				#if	defined(__DEBUG__)
					uint32_t *	p = (uint32_t*) m_ptrArray ;
					if ( p != NULL )
					{
						CheckArrayBoundary() ;
						p = (uint32_t*) esl_realloc
							( p - 2,
								nLimit * sizeof(T) + sizeof(uint32_t)*4 ) ;
					}
					else
					{
						p = (uint32_t*) esl_malloc
							( nLimit * sizeof(T) + sizeof(uint32_t)*4 ) ;
					}
					p[0] = 0x454E5453 ;
					p[1] = 0xCCCCCCCC ;
					m_ptrArray = (T*) (p + 2) ;
					((uint32_t*)(m_ptrArray + nLimit))[0] = 0xCCCCCCCC ;
					((uint32_t*)(m_ptrArray + nLimit))[1] = 0x53544E45 ;
				#else
					if ( m_ptrArray != NULL )
					{
						m_ptrArray = (T*) esl_realloc( m_ptrArray, nLimit * sizeof(T) ) ;
					}
					else
					{
						m_ptrArray = (T*) esl_malloc( nLimit * sizeof(T) ) ;
					}
				#endif
				m_nBufSize = (uint32_t) nLimit ;
			}
		}
		void CheckArrayBoundary( void ) const
		{
		#if	defined(__DEBUG__)
			if ( m_ptrArray != NULL )
			{
				uint32_t *	p = (uint32_t*) m_ptrArray ;
				ESLAssert( p[-2] == 0x454E5453 ) ;
				ESLAssert( p[-1] == 0xCCCCCCCC ) ;
				ESLAssert( ((uint32_t*)(m_ptrArray + m_nBufSize))[0] == 0xCCCCCCCC ) ;
				ESLAssert( ((uint32_t*)(m_ptrArray + m_nBufSize))[1] == 0x53544E45 ) ;
			}
		#endif
		}
		// 要素取得
		T * GetAt( size_t nIndex ) const
		{
			if ( (nIndex >= 0) & (nIndex < m_nLength) )
			{
				return	m_ptrArray + nIndex ;
			}
			return	NULL ;
		}
		T * GetLastAt( size_t nIndex = 0 ) const
		{
			if ( (nIndex >= 0) & (nIndex < m_nLength) )
			{
				return	m_ptrArray + (m_nLength - nIndex - 1) ;
			}
			return	NULL ;
		}
		// operator [] は operator const T * と曖昧になるため使用しない
		T & At( size_t nIndex ) const
		{
			ESLAssert( (nIndex >= 0) && (nIndex < m_nLength) ) ;
			return	m_ptrArray[nIndex] ;
		}
		T & LastAt( size_t nIndex = 0 ) const
		{
			ESLAssert( (nIndex >= 0) && (nIndex < m_nLength) ) ;
			return	m_ptrArray[m_nLength - nIndex - 1] ;
		}
		// 要素設定
		void SetAt( size_t nIndex, const T e )
		{
			if ( nIndex >= m_nLength )
			{
				SetLength( nIndex + 1 ) ;
			}
			m_ptrArray[nIndex] = e ;
		}
		// 要素を挿入
		void Insert( size_t nIndex, size_t nCount )
		{
			if ( nIndex > m_nLength )
			{
				nIndex = m_nLength ;
			}
			size_t	nRight = m_nLength - nIndex ;
			SetLength ( m_nLength + nCount ) ;
			if ( nRight > 0 )
			{
				::eslMoveMemory
					( m_ptrArray + (nIndex + nCount),
						m_ptrArray + nIndex, nRight * sizeof(T) ) ;
			}
			if ( nCount > 0 )
			{
				::eslFillMemory
					( m_ptrArray + nIndex, 0, nCount * sizeof(T) ) ;
			}
		}
		size_t InsertAt( size_t nIndex, const T e )
		{
			if ( nIndex > m_nLength )
			{
				nIndex = m_nLength ;
			}
			Insert( nIndex, 1 ) ;
			m_ptrArray[nIndex] = e ;
			return	nIndex ;
		}
		// 要素を末尾に追加
		size_t Add( const T e )
		{
			uint32_t	nIndex = m_nLength ;
			SetLength( nIndex + 1 ) ;
			m_ptrArray[nIndex] = e ;
			return	nIndex ;
		}
		size_t Push( const T e )
		{
			uint32_t	nIndex = m_nLength ;
			SetLength( nIndex + 1 ) ;
			m_ptrArray[nIndex] = e ;
			return	nIndex ;
		}
		// 配列を末尾に追加
		size_t AddArray( const T * pArray, size_t nCount )
		{
			uint32_t	nIndex = m_nLength ;
			SetLength( nIndex + nCount ) ;
			::eslMoveMemory
				( m_ptrArray + nIndex, pArray, nCount * sizeof(T) ) ;
			return	nIndex ;
		}
		T * AppendArray( size_t nCount )
		{
			uint32_t	nIndex = m_nLength ;
			SetLength( nIndex + nCount ) ;
			return	m_ptrArray + nIndex ;
		}
		// 要素を入れ替え
		void Swap( size_t nIndex1, size_t nIndex2 )
		{
			ESLAssert( nIndex1 < m_nLength ) ;
			ESLAssert( nIndex2 < m_nLength ) ;
			T	temp ;
			temp = m_ptrArray[nIndex1] ;
			m_ptrArray[nIndex1] = m_ptrArray[nIndex2] ;
			m_ptrArray[nIndex2] = temp ;
		}
		// 末尾要素を削除し返却
		T Pop( void )
		{
			ESLAssert( m_nLength > 0 ) ;
			return	m_ptrArray[-- m_nLength] ;
		}
		// 要素を削除
		void Remove( size_t nIndex, size_t nCount )
		{
			if ( nIndex >= m_nLength )
			{
				return ;
			}
			if ( (nIndex + nCount) > m_nLength )
			{
				nCount = m_nLength - nIndex ;
			}
			size_t	nRight = m_nLength - (nIndex + nCount) ;
			if ( nRight > 0 )
			{
				::eslMoveMemory
					( m_ptrArray + nIndex,
						m_ptrArray + (nIndex + nCount), nRight * sizeof(T) ) ;
			}
			m_nLength -= (uint32_t) nCount ;
		}
		void RemoveAt( size_t nIndex )
		{
			Remove( nIndex, 1 ) ;
		}
		void RemoveAll( void ) 
		{
			m_nLength = 0 ;
		}
		// 配列を挿入結合する
		void Merge( size_t nIndex, const SArray<T> & array,
							size_t nFirst = 0, ssize_t nCount = -1 )
		{
			if ( nCount < 0 )
			{
				nCount = (ssize_t) (array.m_nLength - nFirst) ;
			}
			if ( nCount <= 0 )
			{
				return ;
			}
			if ( nIndex > m_nLength )
			{
				nIndex = m_nLength ;
			}
			if ( nCount > 0 )
			{
				Insert( nIndex, nCount ) ;
				::eslMoveMemory
					( m_ptrArray + nIndex,
						array.m_ptrArray + nFirst, nCount * sizeof(T) ) ;
			}
		}

	} ;


	template <class T> class SArrayPtr
	{
	protected:
		T*		m_ptrArray ;
		size_t	m_nBounds ;
		ssize_t	m_iPointer ;

	public:
		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

		// 構築関数
		SArrayPtr( void )
			: m_ptrArray(nullptr), m_nBounds(0), m_iPointer(0) { }
		SArrayPtr( const SArrayPtr<T> & src, ssize_t iOffset = 0 )
			: m_ptrArray(src.m_ptrArray),
				m_nBounds(src.m_nBounds), m_iPointer(src.m_iPointer + iOffset) { }
		SArrayPtr( const SArray<T> & src, ssize_t iOffset = 0 )
			: m_ptrArray(src.GetArrayPtr()),
				m_nBounds(src.GetLength()), m_iPointer(iOffset) { }
		SArrayPtr( T* ptr, size_t nBounds, ssize_t iOffset = 0 )
			: m_ptrArray(ptr), m_nBounds(nBounds), m_iPointer(iOffset) { }

		// 代入
		const SArrayPtr<T>& operator = ( const SArrayPtr<T>& src )
		{
			m_ptrArray = src.m_ptrArray ;
			m_nBounds = src.m_nBounds ;
			m_iPointer = src.m_iPointer ;
			return	*this ;
		}
		const SArrayPtr<T>& operator = ( const SArray<T>& src )
		{
			m_ptrArray = src.GetArrayPtr() ;
			m_nBounds = src.src.GetLength() ;
			m_iPointer = 0 ;
			return	*this ;
		}

		// 比較
		bool operator == ( const SArrayPtr<T>& ptr ) const
		{
			return	(m_ptrArray == ptr.m_ptrArray)
					&& (m_iPointer == ptr.m_iPointer) ;
		}
		bool operator != ( const SArrayPtr<T>& ptr ) const
		{
			return	(m_ptrArray != ptr.m_ptrArray)
					|| (m_iPointer != ptr.m_iPointer) ;
		}
		bool operator == ( const T* ptr ) const
		{
			return	(m_ptrArray + m_iPointer) == ptr ;
		}
		bool operator != ( const T* ptr ) const
		{
			return	(m_ptrArray + m_iPointer) != ptr ;
		}

		// 加減算
		const SArrayPtr<T>& operator += ( ssize_t iOffset )
		{
			m_iPointer += iOffset ;
			return	*this ;
		}
		const SArrayPtr<T>& operator -= ( ssize_t iOffset )
		{
			m_iPointer -= iOffset ;
			return	*this ;
		}
		SArrayPtr<T> operator + ( ssize_t iOffset )
		{
			return	SArrayPtr<T>( m_ptrArray, m_nBounds, m_iPointer + iOffset ) ;
		}
		SArrayPtr<T> operator - ( ssize_t iOffset )
		{
			return	SArrayPtr<T>( m_ptrArray, m_nBounds, m_iPointer - iOffset ) ;
		}

		// ポインタ取得
		T* Ptr( void ) const
		{
			ESLAssert( (size_t) m_iPointer <= m_nBounds ) ;
			return	m_ptrArray + m_iPointer ;
		}
		operator const T * ( void ) const
		{
			ESLAssert( (size_t) m_iPointer <= m_nBounds ) ;
			return	m_ptrArray + m_iPointer ;
		}

		// 有効要素数取得
		size_t GetLength( void ) const
		{
			ESLAssert( (size_t) m_iPointer <= m_nBounds ) ;
			return	(size_t) (m_nBounds - m_iPointer) ;
		}

		// 要素参照
		T& operator[] ( size_t iOffset ) const
		{
			ESLAssert( (size_t) (m_iPointer + iOffset) < m_nBounds ) ;
			return	m_ptrArray[m_iPointer + iOffset] ;
		}
		T& At( ssize_t iOffset ) const
		{
			ESLAssert( (ssize_t) (m_iPointer + iOffset) < m_nBounds ) ;
			return	m_ptrArray[m_iPointer + iOffset] ;
		}

		// 部分配列
		SArrayPtr<T> Bounds( size_t iStart, size_t nLength ) const
		{
			ESLAssert( m_iPointer + iStart + nLength <= m_nBounds ) ;
			return	SArrayPtr<T>( m_ptrArray + (m_iPointer + iStart), nLength ) ;
		}

		// 境界判定
		bool IsOutOfBounds( ssize_t iOffset = 0 ) const
		{
			return	((size_t) (m_iPointer + iOffset) >= m_nBounds) ;
		}

	} ;

} ;

#endif

