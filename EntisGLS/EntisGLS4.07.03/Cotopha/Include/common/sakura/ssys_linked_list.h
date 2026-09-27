
#if	!defined(__SAKURA2_LINKED_LIST_H__)
#define	__SAKURA2_LINKED_LIST_H__

//////////////////////////////////////////////////////////////////////////////
// 結合リスト配列
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	class	SLinkedListConnection
	{
	public:
		SLinkedListConnection *	m_pPrev ;
		SLinkedListConnection *	m_pNext ;
	public:
		// 構築関数
		SLinkedListConnection( void )
		{
			m_pPrev = NULL ;
			m_pNext = NULL ;
		}
		// リストへ挿入
		void InsertAfter( SLinkedListConnection * pPrev )
		{
			SLinkedListConnection *	pNext = pPrev->m_pNext ;
			m_pPrev = pPrev ;
			m_pNext = pNext ;
			pPrev->m_pNext = this ;
			if ( pNext != NULL )
			{
				pNext->m_pPrev = this ;
			}
		}
		void InsertBefore( SLinkedListConnection * pNext )
		{
			SLinkedListConnection *	pPrev = pNext->m_pPrev ;
			m_pPrev = pPrev ;
			m_pNext = pNext ;
			pNext->m_pPrev = this ;
			if ( pPrev != NULL )
			{
				pPrev->m_pNext = this ;
			}
		}
		// リストから分離
		void Detach( void )
		{
			SLinkedListConnection *	pPrev = m_pPrev ;
			SLinkedListConnection *	pNext = m_pNext ;
			if ( pPrev != NULL )
			{
				pPrev->m_pNext = pNext ;
			}
			if ( pNext != NULL )
			{
				pNext->m_pPrev = pPrev ;
			}
			m_pPrev = NULL ;
			m_pNext = NULL ;
		}
		// 次のリスト
		SLinkedListConnection * GetNext( void ) const
		{
			return	m_pNext ;
		}
		// 前のリスト
		SLinkedListConnection * GetPrev( void ) const
		{
			return	m_pPrev ;
		}
	} ;

	template <class T> class SLinkedListEntry
				: public SLinkedListConnection, public T
	{
	public:
		SLinkedListEntry( void )
		{
		}
		SLinkedListEntry( const T & t ) : T( t )
		{
		}
		SLinkedListEntry<T> * GetPrev( void ) const
		{
			return	(SLinkedListEntry<T>*)
							SLinkedListConnection::m_pPrev ;
		}
		SLinkedListEntry<T> * GetNext( void ) const
		{
			return	(SLinkedListEntry<T>*)
							SLinkedListConnection::m_pNext ;
		}
	} ;

	template <class T> class SLinkedList
	{
	protected:
		SLinkedListEntry<T> *	m_pFirst ;
		SLinkedListEntry<T> *	m_pLast ;

	public:
		// 構築関数
		SLinkedList( void )
		{
			m_pFirst = NULL ;
			m_pLast = NULL ;
		}
		// 消滅関数
		~SLinkedList( void )
		{
			DeleteAllEntries() ;
		}
		// 先頭アイテム取得
		SLinkedListEntry<T> * GetFirst( void ) const
		{
			return	m_pFirst ;
		}
		// 先頭からの n 番目のリストエントリ取得
		SLinkedListEntry<T> * GetEntryAt( size_t n ) const
		{
			SLinkedListEntry<T> *	pNext = m_pFirst ;
			for ( size_t i = 0; (i < n) & (pNext != NULL); i ++ )
			{
				pNext = pNext->GetNext() ;
			}
			return	pNext ;
		}
		// 先頭からの n 番目のアイテム取得
		SLinkedListEntry<T> * GetAt( size_t n ) const
		{
			return	GetEntryAt(n) ;
		}
		// 終端アイテム取得
		SLinkedListEntry<T> * GetLast( void ) const
		{
			return	m_pLast ;
		}
		// 終端から n 番目アイテム取得
		SLinkedListEntry<T> * GetLastEntryAt( size_t n ) const
		{
			SLinkedListEntry<T> *	pLast = m_pLast ;
			for ( size_t i = 0; (i < n) & (pLast != NULL); i ++ )
			{
				pLast = pLast->GetPrev() ;
			}
			return	pLast ;
		}
		// 終端アイテム取得
		SLinkedListEntry<T> * GetLastAt( size_t n ) const
		{
			return	GetLastEntryAt(n) ;
		}
		// 先頭へアイテム挿入
		void InsertFirstEntry( SLinkedListEntry<T> * pFirst )
		{
			if ( m_pFirst != NULL )
			{
				pFirst->InsertBefore( m_pFirst ) ;
				m_pFirst = pFirst ;
			}
			else
			{
				ESLAssert( m_pLast == NULL ) ;
				m_pFirst = pFirst ;
				m_pLast = pFirst ;
			}
		}
		// 終端へアイテム挿入
		void InsertLastEntry( SLinkedListEntry<T> * pLast )
		{
			if ( m_pLast != NULL )
			{
				pLast->InsertAfter( m_pLast ) ;
				m_pLast = pLast ;
			}
			else
			{
				ESLAssert( m_pFirst == NULL ) ;
				m_pFirst = pLast ;
				m_pLast = pLast ;
			}
		}
		// アイテムをリストから分離
		SLinkedListEntry<T> * DetachEntry( SLinkedListEntry<T> * pEntry )
		{
			if ( m_pFirst == pEntry )
			{
				m_pFirst = pEntry->GetNext() ;
			}
			if ( m_pLast == pEntry )
			{
				m_pLast = pEntry->GetPrev() ;
			}
			pEntry->SLinkedListConnection::Detach() ;
			return	pEntry ;
		}
		// アイテムをリストから分離し削除
		void DeleteEntry( SLinkedListEntry<T> * pEntry )
		{
			delete	DetachEntry( pEntry ) ;
		}
		// 全てのアイテムを削除
		void DeleteAllEntries( void )
		{
			SLinkedListEntry<T> *	pEntry = m_pFirst ;
			m_pFirst = NULL ;
			m_pLast = NULL ;
			while ( pEntry != NULL )
			{
				SLinkedListEntry<T> *	pNext = pEntry->GetNext() ;
				delete	pEntry ;
				pEntry = pNext ;
			}
		}
		// 全てのアイテムを分離
		void DetachAllEntries( void )
		{
			SLinkedListEntry<T> *	pEntry = m_pFirst ;
			m_pFirst = NULL ;
			m_pLast = NULL ;
			while ( pEntry != NULL )
			{
				SLinkedListEntry<T> *	pNext = pEntry->GetNext() ;
				pEntry->SLinkedListConnection::Detach() ;
				pEntry = pNext ;
			}
		}
	} ;
}

#endif

