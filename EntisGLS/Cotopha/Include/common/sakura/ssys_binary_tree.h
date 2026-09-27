
#if	!defined(__SAKURA2_BINARY_TREE_H__)
#define	__SAKURA2_BINARY_TREE_H__

//////////////////////////////////////////////////////////////////////////////
// 二分木
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	template <class T>	class	SBinaryTreeNode	: public T
	{
	public:
		SBinaryTreeNode<T> *	m_pbtnParent ;
		SBinaryTreeNode<T> *	m_pbtnPrevChild ;
		SBinaryTreeNode<T> *	m_pbtnNextChild ;

	public:
		// 構築関数
		SBinaryTreeNode( void )
			: m_pbtnParent(NULL),
				m_pbtnPrevChild(NULL), m_pbtnNextChild(NULL) {}
		// ノード分離
		void Detach( void )
		{
			if ( m_pbtnParent != NULL )
			{
				if ( m_pbtnParent->m_pbtnPrevChild == this )
				{
					m_pbtnParent->m_pbtnPrevChild = NULL ;
				}
				else if ( m_pbtnParent->m_pbtnNextChild == this )
				{
					m_pbtnParent->m_pbtnNextChild = NULL ;
				}
				m_pbtnParent = NULL ;
			}
		}
		// ノード計数
		size_t CountOfNodes( void ) const
		{
			size_t	countNodes = 0 ;
			if ( this != NULL )
			{
				countNodes ++ ;
				countNodes += m_pbtnPrevChild->CountOfNodes() ;
				countNodes += m_pbtnNextChild->CountOfNodes() ;
			}
			return	countNodes ;
		}
		// ノード列挙
		void ListNode( SPointerArray< SBinaryTreeNode<T> > & listNodes )
		{
			if ( m_pbtnPrevChild != NULL )
			{
				m_pbtnPrevChild->ListNode( listNodes ) ;
			}
			listNodes.Add( this ) ;
			if ( m_pbtnNextChild != NULL )
			{
				m_pbtnNextChild->ListNode( listNodes ) ;
			}
		}
	} ;

	template <class T>	class	SBinaryTree
	{
	protected:
		SBinaryTreeNode<T> *	m_pRoot ;

	public:
		// 構築関数
		SBinaryTree( void ) : m_pRoot( NULL ) {}
		// ルートノード取得
		SBinaryTreeNode<T> * GetRoot( void ) const
		{
			return	m_pRoot ;
		}
		// ノード分離
		void DetachNode( SBinaryTreeNode<T> * pNode )
		{
			pNode->Detach() ;
			if ( pNode == m_pRoot )
			{
				m_pRoot = NULL ;
			}
		}
		// ノード検索
		SBinaryTreeNode<T> * SearchNode( const T & t ) const
		{
			SBinaryTreeNode<T> *	pTarget = m_pRoot ;
			while ( pTarget != NULL )
			{
				int	c = pTarget->Compare( t ) ;
				if ( c == 0 )
				{
					return	pTarget ;
				}
				if ( c < 0 )
				{
					pTarget = pTarget->m_pbtnNextChild ;
				}
				else
				{
					pTarget = pTarget->m_pbtnPrevChild ;
				}
			}
			return	NULL ;
		}
		// ノード追加
		SBinaryTreeNode<T> * AddNode( SBinaryTreeNode<T> * pNode )
		{
			pNode->m_pbtnPrevChild = NULL ;
			pNode->m_pbtnNextChild = NULL ;
			//
			if ( m_pRoot != NULL )
			{
				SBinaryTreeNode<T> *	pTarget = m_pRoot ;
				for ( ; ; )
				{
					int	c = pTarget->Compare( *pNode ) ;
					if ( c < 0 )
					{
						if ( pTarget->m_pbtnNextChild == NULL )
						{
							pTarget->m_pbtnNextChild = pNode ;
							pNode->m_pbtnParent = pTarget ;
							break ;
						}
						pTarget = pTarget->m_pbtnNextChild ;
					}
					else if ( c > 0 )
					{
						if ( pTarget->m_pbtnPrevChild == NULL )
						{
							pTarget->m_pbtnPrevChild = pNode ;
							pNode->m_pbtnParent = pTarget ;
							break ;
						}
						pTarget = pTarget->m_pbtnPrevChild ;
					}
					else
					{
						return	pTarget ;
					}
				}
			}
			else
			{
				m_pRoot = pNode ;
				pNode->m_pbtnParent = NULL ;
			}
			return	pNode ;
		}
		// 全ノード計数
		size_t CountOfAllNodes( void ) const
		{
			return	m_pRoot->CountOfNodes() ;
		}
		// 全ノード列挙
		void ListAllNode
			( SPointerArray< SBinaryTreeNode<T> > & listNodes ) const
		{
			if ( m_pRoot != NULL )
			{
				m_pRoot->ListNode( listNodes ) ;
			}
		}
		// ノードを再構築
		void BuildNodeList
			( const SPointerArray< SBinaryTreeNode<T> > & listNodes )
		{
			m_pRoot = NULL ;
			m_pRoot = BuildNodeListRangeOf
						( listNodes, 0, listNodes.GetLength() ) ;
			if ( m_pRoot != NULL )
			{
				m_pRoot->m_pbtnParent = NULL ;
			}
		}
		static SBinaryTreeNode<T> * BuildNodeListRangeOf
			( const SPointerArray< SBinaryTreeNode<T> > & listNodes,
											size_t iFirst, size_t iEnd )
		{
			if ( iFirst >= iEnd )
			{
				return	NULL ;
			}
			const size_t	iMiddle = (iFirst + iEnd) >> 1 ;
			SBinaryTreeNode<T> *
				pMiddle = listNodes.GetAt( iMiddle ) ;
			ESLAssert( pMiddle != NULL ) ;
			SBinaryTreeNode<T> *
				pPrev = BuildNodeListRangeOf( listNodes, iFirst, iMiddle ) ;
			SBinaryTreeNode<T> *
				pNext = BuildNodeListRangeOf( listNodes, iMiddle + 1, iEnd ) ;
			pMiddle->m_pbtnPrevChild = pPrev ;
			pMiddle->m_pbtnNextChild = pNext ;
			if ( pPrev != NULL )
			{
				pPrev->m_pbtnParent = pMiddle ;
			}
			if ( pNext != NULL )
			{
				pNext->m_pbtnParent = pMiddle ;
			}
			return	pMiddle ;
		}

	} ;

}

#endif
