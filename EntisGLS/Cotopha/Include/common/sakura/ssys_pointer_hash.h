
#if	!defined(__SAKURA2_POINTER_HASH_H__)
#define	__SAKURA2_POINTER_HASH_H__

//////////////////////////////////////////////////////////////////////////////
// ポインタをキーとするハッシュテーブル
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class S,class T> class SPointerHash
	{
	public:
		struct	Element
		{
			S *	key ;
			T	obj ;
		} ;
		Element *	m_hash ;
		size_t		m_size ;
		size_t		m_mask ;
		size_t		m_used ;

	public:
		// 構築関数
		SPointerHash( void )
		{
			m_hash = NULL ;
			m_size = 0 ;
			m_mask = 0 ;
			m_used = 0 ;
		}
		// 消滅関数
		~SPointerHash( void )
		{
			delete []	m_hash ;
			m_hash = NULL ;
			m_size = 0 ;
			m_mask = 0 ;
			m_used = 0 ;
		}
		// 要素追加
		T * Add( S * key, const T obj )
		{
			if ( (m_used << 1) >= m_size )
			{
				ExpandTable() ;
			}
			size_t		i = IndexFromPointer( key ) ;
			ESLAssert( i < m_size ) ;
			Element &	el = m_hash[i] ;
			m_used += (int) (el.key != NULL) & 0x01 ;
			el.key = key ;
			el.obj = obj ;
			return	&(el.obj) ;
		}
		// 要素取得
		T * Get( S * key ) const
		{
			size_t	i = IndexFromPointer( key ) ;
			if ( i < m_size )
			{
				Element &	el = m_hash[i] ;
				if ( el.key == key )
				{
					return	&(el.obj) ;
				}
			}
			return	NULL ;
		}
		// テーブル拡張
		void ExpandTable( void )
		{
			const size_t	sizeOld = m_size ;
			const size_t	sizeNew =
								(sizeOld >= 0x100)
									? (sizeOld << 1) : 0x100 ;
			Element *		pHashOld = m_hash ;
			Element *		pHashNew = new Element[sizeNew] ;
			//
			m_hash = pHashNew ;
			m_size = sizeNew ;
			m_mask = sizeNew - 1 ;
			//
			size_t	i ;
			for ( i = 0; i < sizeNew; i ++ )
			{
				pHashNew[i].key = NULL ;
			}
			for ( i = 0; i < sizeOld; i ++ )
			{
				Element&	elOld = pHashOld[i] ;
				S *			key = elOld.key ;
				if ( key != NULL )
				{
					size_t	j = IndexFromPointer( key ) ;
					ESLAssert( j < m_size ) ;
					pHashNew[j] = elObj ;
				}
			}
			delete[]	pHashOld ;
		}
		// インデックス変換
		size_t IndexFromPointer( S * key ) const
		{
			ulong_ptr_t	i = (ulong_ptr_t) key ;
			i ^= (i >> 16) ;
			i ^= (i >> 8) ;
			i &= m_mask ;
			//
			Element *	hash = m_hash + i ;
			while ( i < m_size )
			{
				if ( (hash->key == NULL)
					|| (hash->key == key) )
				{
					return	i ;
				}
				i ++ ;
				hash ++ ;
			}
			hash = m_hash ;
			for ( i = 0; i < m_size; i ++ )
			{
				if ( (hash->key == NULL)
					|| (hash->key == key) )
				{
					return	i ;
				}
				i ++ ;
				hash ++ ;
			}
			return	(size_t) -1 ;
		}

	} ;

}

#endif

