
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


/*****************************************************************************
                         ハフマン・ツリー・構造体
 *****************************************************************************/

// ツリーの初期化
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::Initialize( void )
{
	int	i ;
	for ( i = 0; i < 0x100; i ++ )
	{
		m_iSymLookup[i] = ERINA_HUFFMAN_NULL ;
	}
	m_iEscape = ERINA_HUFFMAN_NULL ;
	m_iTreePointer = ERINA_HUFFMAN_ROOT ;
	m_hnTree[ERINA_HUFFMAN_ROOT].m_weight = 0 ;
	m_hnTree[ERINA_HUFFMAN_ROOT].m_parent = (WORD) ERINA_HUFFMAN_NULL ;
	m_hnTree[ERINA_HUFFMAN_ROOT].m_child_code = ERINA_HUFFMAN_NULL ;
}

// 発生頻度をインクリメント
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::IncreaseOccuedCount( int iEntry )
{
	m_hnTree[iEntry].m_weight ++ ;
	Normalize( iEntry ) ;
	//
	if ( m_hnTree[ERINA_HUFFMAN_ROOT].m_weight >= ERINA_HUFFMAN_MAX )
	{
		HalfAndRebuild( ) ;
	}
}

// 親の重みを再計算する
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::RecountOccuredCount( int iParent )
{
	int	iChild = m_hnTree[iParent].m_child_code ;
	m_hnTree[iParent].m_weight =
		m_hnTree[iChild].m_weight
			+ m_hnTree[iChild + 1].m_weight ;
}

// ツリーの正規化
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::Normalize( int iEntry )
{
	while ( iEntry < ERINA_HUFFMAN_ROOT )
	{
		//
		// 入れ替えるエントリを検索
		int		iSwap = iEntry + 1 ;
		WORD	weight = m_hnTree[iEntry].m_weight ;
		while ( iSwap < ERINA_HUFFMAN_ROOT )
		{
			if ( m_hnTree[iSwap].m_weight >= weight )
				break ;
			++ iSwap ;
		}
		if ( iEntry == -- iSwap )
		{
			iEntry = m_hnTree[iEntry].m_parent ;
			RecountOccuredCount( iEntry ) ;
			continue ;
		}
		//
		// 入れ替え
		int		iChild, nCode ;
		if ( !(m_hnTree[iEntry].m_child_code & ERINA_CODE_FLAG) )
		{
			iChild = m_hnTree[iEntry].m_child_code ;
			m_hnTree[iChild].m_parent = (WORD) iSwap ;
			m_hnTree[iChild + 1].m_parent = (WORD) iSwap ;
		}
		else
		{
			nCode = (int) (m_hnTree[iEntry].m_child_code & ~ERINA_CODE_FLAG) ;
			if ( nCode != ERINA_HUFFMAN_ESCAPE )
				m_iSymLookup[nCode & 0xFF] = iSwap ;
			else
				m_iEscape = iSwap ;
		}
		if ( !(m_hnTree[iSwap].m_child_code & ERINA_CODE_FLAG) )
		{
			int	iChild = m_hnTree[iSwap].m_child_code ;
			m_hnTree[iChild].m_parent = (WORD) iEntry ;
			m_hnTree[iChild+1].m_parent = (WORD) iEntry ;
		}
		else
		{
			int	nCode = (int) (m_hnTree[iSwap].m_child_code & ~ERINA_CODE_FLAG) ;
			if ( nCode != ERINA_HUFFMAN_ESCAPE )
				m_iSymLookup[nCode & 0xFF] = iEntry ;
			else
				m_iEscape = iEntry ;
		}
		ERINA_HUFFMAN_NODE	node ;
		WORD	iEntryParent = m_hnTree[iEntry].m_parent ;
		WORD	iSwapParent = m_hnTree[iSwap].m_parent ;
		node = m_hnTree[iSwap] ;
		m_hnTree[iSwap] = m_hnTree[iEntry] ;
		m_hnTree[iEntry] = node ;
		m_hnTree[iSwap].m_parent = iSwapParent ;
		m_hnTree[iEntry].m_parent = iEntryParent ;
		//
		// 親の重みを再計算する
		RecountOccuredCount( iSwapParent ) ;
		iEntry = iSwapParent ;
	}
}

// 新しいフリーエントリを作成して追加
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::AddNewEntry( int nNewCode )
{
	if ( m_iTreePointer > 0 )
	{
		//
		// 2つの領域を確保する
		int		i = m_iTreePointer = m_iTreePointer - 2 ;
		//
		// 新しいエントリを初期設定
		ERINA_HUFFMAN_NODE *	phnNew = &m_hnTree[i] ;
		phnNew->m_weight = 1 ;
		phnNew->m_child_code = (DWORD) (ERINA_CODE_FLAG | nNewCode) ;
		m_iSymLookup[nNewCode & 0xFF] = i ;
		//
		ERINA_HUFFMAN_NODE *	phnRoot = &m_hnTree[ERINA_HUFFMAN_ROOT] ;
		if ( phnRoot->m_child_code != ERINA_HUFFMAN_NULL )
		{
			//
			// 新しいエントリをツリーの末端に追加
			ERINA_HUFFMAN_NODE *	phnParent = &m_hnTree[i + 2] ;
			ERINA_HUFFMAN_NODE *	phnChild = &m_hnTree[i + 1] ;
			m_hnTree[i + 1] = m_hnTree[i + 2] ;
			//
			if ( phnChild->m_child_code & ERINA_CODE_FLAG )
			{
				int	nCode = (int) (phnChild->m_child_code & ~ERINA_CODE_FLAG) ;
				if ( nCode != ERINA_HUFFMAN_ESCAPE )
			 		m_iSymLookup[nCode & 0xFF] = i + 1 ;
				else
					m_iEscape = i + 1 ;
			}
			//
			phnParent->m_weight =
				phnNew->m_weight + phnChild->m_weight ;
			phnParent->m_parent = phnChild->m_parent ;
			phnParent->m_child_code = i ;
			//
			phnNew->m_parent = phnChild->m_parent = (WORD) (i + 2) ;
			//
			// 親エントリの正規化
			Normalize( i + 2 ) ;
		}
		else
		{
			//
			// 初期状態のツリーを構築
			phnNew->m_parent = ERINA_HUFFMAN_ROOT ;
			//
			ERINA_HUFFMAN_NODE *
				phnEscape = &m_hnTree[m_iEscape = i + 1] ;
			phnEscape->m_weight = 1 ;
			phnEscape->m_parent = ERINA_HUFFMAN_ROOT ;
			phnEscape->m_child_code =
					(DWORD) (ERINA_CODE_FLAG | ERINA_HUFFMAN_ESCAPE) ;
			//
			phnRoot->m_weight = 2 ;
			phnRoot->m_child_code = i ;
		}
	}
	else
	{
		//
		// 最も出現頻度の低いシンボルを新しいシンボルで置き換える
		int		i = m_iTreePointer ;
		ERINA_HUFFMAN_NODE *	phnEntry = &m_hnTree[i] ;
		if ( phnEntry->m_child_code
				== (ERINA_CODE_FLAG | ERINA_HUFFMAN_ESCAPE) )
		{
			phnEntry = &m_hnTree[i + 1] ;
		}
		phnEntry->m_child_code = (DWORD) (ERINA_CODE_FLAG | nNewCode) ;
	}
}

// 各出現数を2分の1にして木を再構成
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERINA_HUFFMAN_TREE::HalfAndRebuild( void )
{
	//
	// 出現頻度を2分の1にする
	int		i ;
	int		iNextEntry = ERINA_HUFFMAN_ROOT ;
	for ( i = ERINA_HUFFMAN_ROOT - 1; i >= m_iTreePointer; i -- )
	{
		if ( m_hnTree[i].m_child_code & ERINA_CODE_FLAG )
		{
			m_hnTree[i].m_weight = (m_hnTree[i].m_weight + 1) >> 1 ;
			m_hnTree[iNextEntry --] = m_hnTree[i] ;
		}
	}
	++ iNextEntry ;
	//
	// ツリーを再構築
	int		iChild, nCode ;
	i = m_iTreePointer ;
	for ( ; ; )
	{
		//
		// 最も重みの小さい2つのエントリをハフマン木に組み込む
		m_hnTree[i] = m_hnTree[iNextEntry] ;
		m_hnTree[i + 1] = m_hnTree[iNextEntry + 1] ;
		iNextEntry += 2 ;
		ERINA_HUFFMAN_NODE *	phnChild1 = &m_hnTree[i] ;
		ERINA_HUFFMAN_NODE *	phnChild2 = &m_hnTree[i + 1] ;
		//
		if ( !(phnChild1->m_child_code & ERINA_CODE_FLAG) )
		{
			iChild = phnChild1->m_child_code ;
			m_hnTree[iChild].m_parent = (WORD) i ;
			m_hnTree[iChild + 1].m_parent = (WORD) i ;
		}
		else
		{
			nCode = (int) (phnChild1->m_child_code & ~ERINA_CODE_FLAG) ;
			if ( nCode == ERINA_HUFFMAN_ESCAPE )
				m_iEscape = i ;
			else
				m_iSymLookup[nCode & 0xFF] = i ;
		}
		//
		if ( !(phnChild2->m_child_code & ERINA_CODE_FLAG) )
		{
			iChild = phnChild2->m_child_code ;
			m_hnTree[iChild].m_parent = (WORD) (i + 1) ;
			m_hnTree[iChild + 1].m_parent = (WORD) (i + 1) ;
		}
		else
		{
			nCode = (int) (phnChild2->m_child_code & ~ERINA_CODE_FLAG) ;
			if ( nCode == ERINA_HUFFMAN_ESCAPE )
				m_iEscape = i + 1 ;
			else
				m_iSymLookup[nCode & 0xFF] = i + 1 ;
		}
		//
		WORD	weight = phnChild1->m_weight + phnChild2->m_weight ;
		//
		// 親エントリをリストに組み込む
		if ( iNextEntry <= ERINA_HUFFMAN_ROOT )
		{
			int		j = iNextEntry ;
			for ( ; ; )
			{
				if ( weight <= m_hnTree[j].m_weight )
				{
					m_hnTree[j - 1].m_weight = weight ;
					m_hnTree[j - 1].m_child_code = i ;
					break ;
				}
				m_hnTree[j - 1] = m_hnTree[j] ;
				if ( ++ j > ERINA_HUFFMAN_ROOT )
				{
					m_hnTree[ERINA_HUFFMAN_ROOT].m_weight = weight ;
					m_hnTree[ERINA_HUFFMAN_ROOT].m_child_code = i ;
					break ;
				}
			}
			-- iNextEntry ;
		}
		else
		{
			m_hnTree[ERINA_HUFFMAN_ROOT].m_weight = weight ;
			m_hnTree[ERINA_HUFFMAN_ROOT].m_parent = (WORD) ERINA_HUFFMAN_NULL ;
			m_hnTree[ERINA_HUFFMAN_ROOT].m_child_code = i ;
			phnChild1->m_parent = ERINA_HUFFMAN_ROOT ;
			phnChild2->m_parent = ERINA_HUFFMAN_ROOT ;
			break ;
		}
		//
		i += 2 ;
	}
}


/*****************************************************************************
                             算術符号統計モデル
 *****************************************************************************/

const int	ERISA::ERISA_PROB_BASE::m_nShiftCount[4] = { 1, 3, 4, 5 } ;
const int	ERISA::ERISA_PROB_BASE::m_nNewProbLimit[4] = { 0x01, 0x08, 0x10, 0x20 } ;


// 統計情報の初期化
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERISA_PROB_MODEL::Initialize( void )
{
	dwTotalCount = ERISA_SYMBOL_SORTS ;
	dwSymbolSorts = ERISA_SYMBOL_SORTS ;
	//
	int	i ;
	for ( i = 0; i < 0x100; i ++ )
	{
		acsSymTable[i].wOccured = 1 ;
		acsSymTable[i].wSymbol = (SWORD) (BYTE) i ;
	}
	acsSymTable[0x100].wOccured = 1 ;
	acsSymTable[0x100].wSymbol = (SWORD) ERISA_ESC_CODE ;
	//
	for ( i = 0; i < ERISA_SUB_SORT_MAX; i ++ )
	{
		acsSubModel[i].wOccured = 0 ;
		acsSubModel[i].wSymbol = (SWORD) -1 ;
	}
}

// 生起確率を累積する
//////////////////////////////////////////////////////////////////////////////
int ERISA::ERISA_PROB_MODEL::AccumulateProb( SWORD wSymbol )
{
	int	iSym = FindSymbol( wSymbol ) ;
	ESLAssert( iSym >= 0 ) ;
	DWORD	dwOccured = acsSymTable[iSym].wOccured ;
	int		i = 0 ;
	while ( dwOccured < dwTotalCount )
	{
		dwOccured <<= 1 ;
		i ++ ;
	}
	return	i ;
}

// シンボルの生起数を 1/2 にする
//////////////////////////////////////////////////////////////////////////////
void ERISA::ERISA_PROB_MODEL::HalfOccuredCount( void )
{
	DWORD	 i ;
	dwTotalCount = 0 ;
	for ( i = 0; i < dwSymbolSorts; i ++ )
	{
		dwTotalCount +=
			acsSymTable[i].wOccured =
				((acsSymTable[i].wOccured + 1) >> 1) ;
	}
	for ( i = 0; i < ERISA_SUB_SORT_MAX; i ++ )
	{
		acsSubModel[i].wOccured >>= 1 ;
	}
}

// 指定のシンボルの生起数をインクリメントする
//////////////////////////////////////////////////////////////////////////////
int ERISA::ERISA_PROB_MODEL::IncreaseSymbol( int index )
{
	WORD	wOccured = ++ acsSymTable[index].wOccured ;
	SWORD	wSymbol = acsSymTable[index].wSymbol ;
	//
	while ( -- index >= 0 )
	{
		if ( acsSymTable[index].wOccured >= wOccured )
			break ;
		acsSymTable[index + 1] = acsSymTable[index] ;
	}
	acsSymTable[++ index].wOccured = wOccured ;
	acsSymTable[index].wSymbol = wSymbol ;
	//
	if ( ++ dwTotalCount >= ERISA_TOTAL_LIMIT )
	{
		HalfOccuredCount( ) ;
	}
	//
	return	index ;
}

// 指定のシンボルの指標を取得する
//////////////////////////////////////////////////////////////////////////////
int ERISA::ERISA_PROB_MODEL::FindSymbol( SWORD wSymbol )
{
	int		iSym = 0 ;
	while ( acsSymTable[iSym].wSymbol != wSymbol )
	{
		if ( (DWORD) ++ iSym >= dwSymbolSorts )
			return	-1 ;
	}
	return	iSym ;
}

// 指定のシンボルを追加する
//////////////////////////////////////////////////////////////////////////////
int ERISA::ERISA_PROB_MODEL::AddSymbol( SWORD wSymbol )
{
	int		iSym = dwSymbolSorts ++ ;
	dwTotalCount ++ ;
	acsSymTable[iSym].wSymbol = wSymbol ;
	acsSymTable[iSym].wOccured = 1 ;
	return	iSym ;
}



