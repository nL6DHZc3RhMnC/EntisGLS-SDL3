
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


//////////////////////////////////////////////////////////////////////////////
// 符号化ビットストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLEncodeBitStream, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLEncodeBitStream::~SGLEncodeBitStream( void )
{
	if ( m_ptrBuffer != NULL )
	{
		esl_free( m_ptrBuffer ) ;
		m_ptrBuffer = NULL ;
	}
}

// ｎビット出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLEncodeBitStream::OutNBits( DWORD dwData, size_t nBits )
{
	while ( nBits > 0 )
	{
		if ( m_nIntBufCount + nBits >= 32 )
		{
			//
			// 中間バッファがいっぱいになる
			//
			size_t	sc = 32 - m_nIntBufCount ;
			if ( m_nIntBufCount == 0 )
			{
				m_dwIntBuffer = dwData ;
			}
			else
			{
				m_dwIntBuffer =
					(m_dwIntBuffer << sc) | (dwData >> m_nIntBufCount) ;
				dwData <<= sc ;
			}
			m_nIntBufCount = 0 ;
			nBits -= sc ;
			//
			// バッファに出力
			//
			m_ptrBuffer[m_nBufCount] = (BYTE) (m_dwIntBuffer >> 24) ;
			m_ptrBuffer[m_nBufCount + 1] = (BYTE) (m_dwIntBuffer >> 16) ;
			m_ptrBuffer[m_nBufCount + 2] = (BYTE) (m_dwIntBuffer >> 8) ;
			m_ptrBuffer[m_nBufCount + 3] = (BYTE) m_dwIntBuffer ;
			m_nBufCount += sizeof(DWORD) ;
			if ( m_nBufCount >= m_nBufferingSize )
			{
				if ( m_pStream->Write
						( m_ptrBuffer, m_nBufCount ) < m_nBufCount )
				{
					return	SSystem::errFailed ;
				}
				m_nBufCount = 0 ;
			}
		}
		else
		{
			//
			// 中間バッファには余裕がある
			//
			m_dwIntBuffer = (m_dwIntBuffer << nBits)
								| (dwData >> (32 - nBits)) ;
			m_nIntBufCount += nBits ;
			break ;
		}
	}
	return	SSystem::errSuccess ;
}

// バッファの内容を出力して空にする
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLEncodeBitStream::Flushout( void )
{
	if ( m_nIntBufCount > 0 )
	{
		m_dwIntBuffer <<= (32 - m_nIntBufCount) ;
		m_nIntBufCount = 0 ;
		m_ptrBuffer[m_nBufCount] = (BYTE) (m_dwIntBuffer >> 24) ;
		m_ptrBuffer[m_nBufCount + 1] = (BYTE) (m_dwIntBuffer >> 16) ;
		m_ptrBuffer[m_nBufCount + 2] = (BYTE) (m_dwIntBuffer >> 8) ;
		m_ptrBuffer[m_nBufCount + 3] = (BYTE) m_dwIntBuffer ;
		m_nBufCount += sizeof(DWORD) ;
	}
	if ( m_pStream->Write( m_ptrBuffer, m_nBufCount ) < m_nBufCount )
	{
		return	SSystem::errFailed ;
	}
	m_nBufCount = 0 ;
	return	SSystem::errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 符号化コンテキスト基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLAbstractEncodeContext, SOutputStream )

// 符号完了処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLAbstractEncodeContext::FinishEncoding( void )
{
	return	m_pStream->Flushout() ;
}


//////////////////////////////////////////////////////////////////////////////
// ランレングスガンマ符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

const size_t	SGLGammaEncodeContext::m_GammaCodeBytesTable[0x100] =
{
	1, 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SGLGammaEncodeContext, SGLAbstractEncodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGammaEncodeContext::SGLGammaEncodeContext( SGLEncodeBitStream * pStream )
	: SGLAbstractEncodeContext( pStream )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLGammaEncodeContext::~SGLGammaEncodeContext( void )
{
}

// ガンマ符号の符号化の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLGammaEncodeContext::PrepareToEncodeGammaCode( void )
{
}

// ガンマコードに符号化した際のビット数を計算
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaEncodeContext::EstimateGammaCode( int num )
{
	ESLAssert( num >= 0 ) ;
	if ( num < 0x100 )
	{
		return	m_GammaCodeBytesTable[num] ;
	}
	int	i, j ;
	i = 0 ;
	j = 1 ;
	while ( j < num )
	{
		j = (j << 1) + 1 ;
		i ++ ;
	}
	return	(i << 1) + 1 ;
}

// ランレングスガンマ符号に符号化した際のサイズ（ビット数）を計算
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaEncodeContext::EstimateGammaCodeBytes( const SBYTE * ptrSrc, size_t nCount )
{
	size_t	c, i = 0, l = 1 ;
	while ( i < nCount )
	{
		if ( ptrSrc[i] == 0 )
		{
			//
			// ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[i] == 0 )
			{
				if ( ++ i >= nCount )
				{
					break ;
				}
			}
			//
			// 符号の長さを加算
			//
			l += EstimateGammaCode( (int) i - (int) c ) ;
		}
		else
		{
			//
			// 非ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[c] != 0 )
			{
				if ( ++ c >= nCount )
				{
					break ;
				}
			}
			//
			// 符号の長さを加算
			//
			l += EstimateGammaCode( (int) c - (int) i ) ;
			//
			// 各符号の長さを加算
			//
			while ( i < c )
			{
				int	num = ptrSrc[i ++] ;
				if ( num < 0 )
				{
					num = - num ;
				}
				l += EstimateGammaCode( num ) + 1 ;
			}
		}
	}
	return	l ;
}

// ガンマコードを出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLGammaEncodeContext::OutGammaCode( int num )
{
	//
	// コードの長さを決定する
	//
	int	b = 0, c = 1 ;
	num -- ;
	while ( num >= c )
	{
		num -= c ;
		c <<= 1 ;
		b ++ ;
	}
	if ( b <= 15 )
	{
		//
		// コードを生成
		//
		DWORD	dwCode = 0xAAAAAAAA ;
		DWORD	dwNum = (DWORD) num << (31 - b) ;
		DWORD	dwMask = 0x40000000 ;
		for ( c = 0; c < b; c ++ )
		{
			dwCode |= (dwNum & dwMask) ;
			dwNum >>= 1 ;
			dwMask >>= 2 ;
		}
		dwCode &= ~((dwMask << 2) - 1) ;
		//
		// コードを出力
		//
		return	m_pStream->OutNBits( dwCode, (b << 1) + 1 ) ;
	}
	SGLEncodeBitStream *	ps = m_pStream ;
	SError	err ;
	DWORD	dwNum = (DWORD) num << (31 - b) ;
	for ( int i = 0; i < b; i ++ )
	{
		DWORD	dwCode = 0x80000000 | (dwNum & 0x40000000) ;
		dwNum <<= 1 ;
		err = ps->OutNBits( dwCode, 2 ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	ps->OutNBits( 0, 1 ) ;
}

// ランレングスガンマ符号に符号化して出力する
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaEncodeContext::EncodeGammaCodeBytes( const SBYTE * ptrSrc, size_t nCount )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	size_t	c, i = 0 ;
	if ( ptrSrc[0] == 0 )
	{
		ps->OutNBits( 0, 1 ) ;
	}
	else
	{
		ps->OutNBits( 0x80000000UL, 1 ) ;
	}
	while ( i < nCount )
	{
		if ( ptrSrc[i] == 0 )
		{
			//
			// ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[i] == 0 )
			{
				if ( ++ i >= nCount )
				{
					break ;
				}
			}
			//
			// ゼロ信号の長さを出力
			//
			if ( OutGammaCode( (int) i - (int) c ) )
			{
				break ;
			}
		}
		else
		{
			//
			// 非ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[c] != 0 )
			{
				if ( ++ c >= nCount )
				{
					break ;
				}
			}
			//
			// 非ゼロ信号の長さを出力
			//
			if ( OutGammaCode( (int) c - (int) i ) )
			{
				break ;
			}
			//
			// 各符号を符号化
			//
			while ( i < c )
			{
				int	num = ptrSrc[i ++] ;
				if ( num < 0 )
				{
					if ( ps->OutNBits( 0x80000000UL, 1 ) )
					{
						return	i ;
					}
					num = - num ;
				}
				else
				{
					if ( ps->OutNBits( 0, 1 ) )
					{
						return	i ;
					}
				}
				if ( OutGammaCode( num ) )
				{
					return	i ;
				}
			}
		}
	}
	return	i ;
}

size_t SGLGammaEncodeContext::EncodeGammaCodeWords( const SWORD * ptrSrc, size_t nCount )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	size_t	c, i = 0 ;
	if ( ptrSrc[0] == 0 )
	{
		ps->OutNBits( 0, 1 ) ;
	}
	else
	{
		ps->OutNBits( 0x80000000UL, 1 ) ;
	}
	while ( i < nCount )
	{
		if ( ptrSrc[i] == 0 )
		{
			//
			// ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[i] == 0 )
			{
				if ( ++ i >= nCount )
				{
					break ;
				}
			}
			//
			// ゼロ信号の長さを出力
			//
			if ( OutGammaCode( (int) i - (int) c ) )
			{
				break ;
			}
		}
		else
		{
			//
			// 非ゼロ信号の連続数を取得
			//
			c = i ;
			while ( ptrSrc[c] != 0 )
			{
				if ( ++ c >= nCount )
				{
					break ;
				}
			}
			//
			// 非ゼロ信号の長さを出力
			//
			if ( OutGammaCode( (int) c - (int) i ) )
			{
				break ;
			}
			//
			// 各符号を符号化
			//
			while ( i < c )
			{
				int	num = ptrSrc[i ++] ;
				if ( num < 0 )
				{
					if ( ps->OutNBits( 0x80000000UL, 1 ) )
					{
						return	i ;
					}
					num = - num ;
				}
				else
				{
					if ( ps->OutNBits( 0, 1 ) )
					{
						return	i ;
					}
				}
				if ( OutGammaCode( num ) )
				{
					return	i ;
				}
			}
		}
	}
	return	i ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaEncodeContext::Write( const void * ptrBuf, size_t nBytes )
{
	return	EncodeGammaCodeBytes( (const SBYTE*) ptrBuf, nBytes ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ERINA（ハフマン）符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLHuffmanEncodeContext, SGLGammaEncodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLHuffmanEncodeContext::SGLHuffmanEncodeContext( SGLEncodeBitStream * pStream )
	: SGLGammaEncodeContext( pStream )
{
	m_ppHuffmanTree = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLHuffmanEncodeContext::~SGLHuffmanEncodeContext( void )
{
	if ( m_ppHuffmanTree != NULL )
	{
		esl_free( m_ppHuffmanTree ) ;
		m_ppHuffmanTree = NULL ;
	}
}

// ERINA 符号の符号化の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLHuffmanEncodeContext::PrepareToEncodeERINACode( DWORD dwFlags )
{
	//
	// メモリを確保する
	//
	int	i ;
	if ( m_ppHuffmanTree == NULL )
	{
		size_t	nSize =
			(sizeof(ERINA_HUFFMAN_TREE*)
				+ sizeof(ERINA_HUFFMAN_TREE)) * 0x101 ;
		nSize = (nSize + 0x0F) & ~0x0F ;
		m_ppHuffmanTree = (ERINA_HUFFMAN_TREE **) esl_malloc( nSize ) ;
		//
		PBYTE	ptrBuf = (PBYTE)(m_ppHuffmanTree + 0x101) ;
		for ( i = 0; i < 0x101; i ++ )
		{
			m_ppHuffmanTree[i] = (ERINA_HUFFMAN_TREE*) ptrBuf ;
			ptrBuf += sizeof(ERINA_HUFFMAN_TREE) ;
		}
	}
	//
	// ハフマンテーブルを初期化
	//
	m_dwERINAFlags = dwFlags ;
	if ( dwFlags == efERINAOrder0 )
	{
		m_ppHuffmanTree[0]->Initialize( ) ;
		m_ppHuffmanTree[0x100]->Initialize( ) ;
		for ( i = 1; i < 0x100; i ++ )
		{
			m_ppHuffmanTree[i] = m_ppHuffmanTree[0] ;
		}
	}
	else
	{
		for ( i = 0; i < 0x101; i ++ )
		{
			m_ppHuffmanTree[i]->Initialize( ) ;
		}
	}
	m_pLastHuffmanTree = m_ppHuffmanTree[0] ;
}

// ハフマン符号で出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLHuffmanEncodeContext::OutHuffmanCode
				( ERISA::ERINA_HUFFMAN_TREE * tree, int num )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	//
	// 指定のツリーエントリを取得
	//
	int	iEntry = tree->m_iSymLookup[num & 0xFF] ;
	if ( iEntry == ERINA_HUFFMAN_NULL )
	{
		//
		// エスケープコードを出力
		//
		if ( tree->m_iEscape != ERINA_HUFFMAN_NULL )
		{
			DWORD	dwCode = 0 ;
			int		nCodeLength = 0 ;
			int		iChild = tree->m_iEscape ;
			int		iParent ;
			do
			{
				iParent = tree->m_hnTree[iChild].m_parent ;
				++ nCodeLength ;
				if ( tree->m_hnTree[iParent].m_child_code == (DWORD) iChild )
				{
					dwCode >>= 1 ;
				}
				else
				{
					dwCode = (dwCode >> 1) | 0x80000000 ;
				}
				iChild = iParent ;
			}
			while ( iParent < ERINA_HUFFMAN_ROOT ) ;
			//
			if ( ps->OutNBits( dwCode, nCodeLength ) )
			{
				return	SSystem::errFailed ;
			}
			if ( (m_dwERINAFlags != efERINAOrder0) ||
				(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
								m_weight < ERINA_HUFFMAN_MAX-1) )
			{
				tree->IncreaseOccuedCount( tree->m_iEscape ) ;
			}
		}
		if ( ps->OutNBits( (DWORD) num << 24, 8 ) )
		{
			return	SSystem::errFailed ;
		}
		tree->AddNewEntry( num ) ;
	}
	else
	{
		//
		// 普通のコードを出力
		//
		DWORD	dwCode = 0 ;
		int		nCodeLength = 0 ;
		int		iChild = iEntry ;
		int		iParent ;
		do
		{
			iParent = tree->m_hnTree[iChild].m_parent ;
			++ nCodeLength ;
			if ( tree->m_hnTree[iParent].m_child_code == (DWORD) iChild )
			{
				dwCode >>= 1 ;
			}
			else
			{
				dwCode = (dwCode >> 1) | 0x80000000 ;
			}
			iChild = iParent ;
		}
		while ( iParent < ERINA_HUFFMAN_ROOT ) ;
		//
		if ( ps->OutNBits( dwCode, nCodeLength ) )
		{
			return	SSystem::errFailed ;
		}
		if ( (m_dwERINAFlags != efERINAOrder0) ||
			(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
							m_weight < ERINA_HUFFMAN_MAX-1) )
		{
			tree->IncreaseOccuedCount( iEntry ) ;
		}
	}
	return	SSystem::errSuccess ;
}

// 長さをハフマン符号で出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLHuffmanEncodeContext::OutLengthHuffman
		( ERISA::ERINA_HUFFMAN_TREE * tree, int nLength )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	ERINA_HUFFMAN_NODE *	phnEntry ;
	int		iChild, iParent, nCodeLength ;
	DWORD	dwCode ;
	//
	// 指定のツリーエントリを取得
	//
	int	iEntry = tree->m_iSymLookup[nLength & 0xFF] ;
	if ( (iEntry == ERINA_HUFFMAN_NULL) ||
		(tree->m_hnTree[iEntry].m_child_code
						!= (DWORD) (ERINA_CODE_FLAG | nLength)) )
	{
		int	iTreePointer = tree->m_iTreePointer ;
		iEntry = ERINA_HUFFMAN_ROOT - 1 ;
		while ( iEntry >= iTreePointer )
		{
			phnEntry = &tree->m_hnTree[iEntry] ;
			if ( (phnEntry->m_child_code & ERINA_CODE_FLAG)
				&& (phnEntry->m_child_code
						== (DWORD) (ERINA_CODE_FLAG | nLength)) )
			{
				break ;
			}
			-- iEntry ;
		}
		if ( iEntry < iTreePointer )
		{
			//
			// エスケープコードを出力
			//
			if ( tree->m_iEscape != ERINA_HUFFMAN_NULL )
			{
				iChild = tree->m_iEscape ;
				nCodeLength = 0 ;
				dwCode = 0 ;
				do
				{
					iParent = tree->m_hnTree[iChild].m_parent ;
					++ nCodeLength ;
					if ( tree->m_hnTree[iParent].m_child_code == (DWORD) iChild )
					{
						dwCode >>= 1 ;
					}
					else
					{
						dwCode = (dwCode >> 1) | 0x80000000 ;
					}
					iChild = iParent ;
				}
				while ( iParent < ERINA_HUFFMAN_ROOT ) ;
				//
				if ( ps->OutNBits( dwCode, nCodeLength ) )
				{
					return	SSystem::errFailed ;
				}
				if ( (m_dwERINAFlags != efERINAOrder0) ||
					(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
									m_weight < ERINA_HUFFMAN_MAX-1) )
				{
					tree->IncreaseOccuedCount( tree->m_iEscape ) ;
				}
			}
			if ( OutGammaCode( nLength ) )
			{
				return	SSystem::errFailed ;
			}
			tree->AddNewEntry( nLength ) ;
			return	SSystem::errSuccess ;
		}
	}
	//
	// 通常のハフマン符号を出力
	//
	iChild = iEntry ;
	dwCode = 0 ;
	nCodeLength = 0 ;
	do
	{
		iParent = tree->m_hnTree[iChild].m_parent ;
		++ nCodeLength ;
		if ( tree->m_hnTree[iParent].m_child_code == (DWORD) iChild )
		{
			dwCode >>= 1 ;
		}
		else
		{
			dwCode = (dwCode >> 1) | 0x80000000 ;
		}
		iChild = iParent ;
	}
	while ( iParent < ERINA_HUFFMAN_ROOT ) ;
	//
	if ( ps->OutNBits( dwCode, nCodeLength ) )
	{
		return	SSystem::errFailed ;
	}
	if ( (m_dwERINAFlags != efERINAOrder0) ||
		(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
						m_weight < ERINA_HUFFMAN_MAX-1) )
	{
		tree->IncreaseOccuedCount( iEntry ) ;
	}
	return	SSystem::errSuccess ;
}

// ERINA 符号に符号化して出力する
//////////////////////////////////////////////////////////////////////////////
size_t SGLHuffmanEncodeContext::EncodeERINACodeBytes( const SBYTE * ptrSrc, size_t nCount )
{
	ERINA_HUFFMAN_TREE *	tree = m_pLastHuffmanTree ;
	int		nSymbol = ERINA_HUFFMAN_ESCAPE ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		nSymbol = ptrSrc[i ++] & 0xFF ;
		if ( OutHuffmanCode( tree, nSymbol ) )
		{
			break ;
		}
		if ( nSymbol == 0 )
		{
			size_t	j = i ;
			while (  i < nCount )
			{
				if ( ptrSrc[i] & 0xFF )
				{
					break ;
				}
				++ i ;
			}
			if ( OutLengthHuffman
				( m_ppHuffmanTree[0x100], (int) (i - j + 1) ) )
			{
				break ;
			}
		}
		tree = m_ppHuffmanTree[nSymbol] ;
	}
	m_pLastHuffmanTree = tree ;
	return	i ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLHuffmanEncodeContext::Write( const void * ptrBuf, size_t nBytes )
{
	return	EncodeERINACodeBytes( (const SBYTE*) ptrBuf, nBytes ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ERISA（算術）符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLERISAEncodeContext, SGLAbstractEncodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLERISAEncodeContext::SGLERISAEncodeContext( SGLEncodeBitStream * pStream )
	: SGLAbstractEncodeContext( pStream )
{
	m_ppTableERISA = NULL ;
	m_pPhraseLenProb = NULL ;
	m_pPhraseIndexProb = NULL ;
	m_pRunLenProb = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLERISAEncodeContext::~SGLERISAEncodeContext( void )
{
	if ( m_ppTableERISA != NULL )
	{
		esl_free( m_ppTableERISA ) ;
		m_ppTableERISA = NULL ;
	}
	delete	m_pPhraseLenProb ;
	delete	m_pPhraseIndexProb ;
	delete	m_pRunLenProb ;
	m_pPhraseLenProb = NULL ;
	m_pPhraseIndexProb = NULL ;
	m_pRunLenProb = NULL ;
}

// ERISA 符号の符号化の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLERISAEncodeContext::PrepareToEncodeERISACode( void )
{
	//
	// メモリを確保
	//
	int		i ;
	if ( m_ppTableERISA == NULL )
	{
		size_t	nBytes =
			sizeof(ERISA_PROB_MODEL*) * 0x104
					+ sizeof(ERISA_PROB_MODEL) * 0x100 ;
		nBytes = (nBytes + 0x100F) & (~0xFFF) ;
		m_ppTableERISA = (ERISA_PROB_MODEL**) esl_malloc( nBytes ) ;
	}
	if ( m_pPhraseLenProb == NULL )
	{
		m_pPhraseLenProb = new ERISA_PROB_MODEL ;
	}
	if ( m_pPhraseIndexProb == NULL )
	{
		m_pPhraseIndexProb = new ERISA_PROB_MODEL ;
	}
	if ( m_pRunLenProb == NULL )
	{
		m_pRunLenProb = new ERISA_PROB_MODEL ;
	}
	//
	// 統計モデルを初期化
	//
	ERISA_PROB_MODEL *	pNextProb =
		(ERISA_PROB_MODEL*) (m_ppTableERISA + 0x104) ;
	m_pLastERISAProb = pNextProb ;
	for ( i = 0; i < 0x101; i ++ )
	{
		pNextProb->Initialize( ) ;
		m_ppTableERISA[i] = pNextProb ;
		pNextProb ++ ;
	}
	m_pPhraseLenProb->Initialize( ) ;
	m_pPhraseIndexProb->Initialize( ) ;
	m_pRunLenProb->Initialize( ) ;
	//
	// レジスタを初期化
	//
	m_dwCodeRegister = 0 ;
	m_dwAugendRegister = 0xFFFF ;
	m_dwCodeBuffer = 0 ;
	m_dwBitBufferCount = -1 ;
}

// 指定の統計モデルを使って1つの算術符号を出力
//////////////////////////////////////////////////////////////////////////////
int SGLERISAEncodeContext::EncodeERISACodeSymbol
	( ERISA::ERISA_PROB_MODEL * pModel, SWORD wSymbol )
{
	//
	// シンボル指標を取得
	//
	int		iSym = 0 ;
	WORD	wFs = 0 ;
	while ( pModel->acsSymTable[iSym].wSymbol != wSymbol )
	{
		wFs += pModel->acsSymTable[iSym ++].wOccured ;
		ESLAssert( (DWORD) iSym < pModel->dwSymbolSorts ) ;
	}
	return	EncodeERISACodeIndex( pModel, iSym, wFs ) ;
}

int SGLERISAEncodeContext::EncodeERISACodeIndex
	( ERISA::ERISA_PROB_MODEL * pModel, int iSym, WORD wFs )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	//
	// コードレジスタとオージェンドレジスタを更新
	//
	m_dwCodeRegister +=
		(m_dwAugendRegister * wFs
			+ pModel->dwTotalCount - 1) / pModel->dwTotalCount ;
	m_dwAugendRegister =
		m_dwAugendRegister
			*  pModel->acsSymTable[iSym].wOccured
			/ pModel->dwTotalCount ;
	//
	// コードレジスタ桁あふれ処理
	//
	if ( m_dwCodeRegister & 0x10000 )
	{
		m_dwCodeBuffer ++ ;
		if ( m_dwCodeBuffer & 0x10000 )
		{
			ESLAssert( m_dwBitBufferCount >= 0 ) ;
			//
			ps->OutNBits( 0x80000000UL, 1 ) ;
			//
			m_dwBitBufferCount -- ;
			while ( m_dwBitBufferCount > 0 )
			{
				DWORD	dwBits = m_dwBitBufferCount ;
				if ( dwBits > 16 )
				{
					dwBits = 16 ;
				}
				ps->OutNBits( 0x00000000, dwBits ) ;
				m_dwBitBufferCount -= dwBits ;
			}
			m_dwCodeBuffer &= 0xFFFF ;
		}
		m_dwCodeRegister &= 0xFFFF ;
	}
	ESLAssert( !(m_dwCodeRegister & 0xFFFF0000) ) ;
	//
	// オージェントレジスタを正規化し、コードレジスタから符号を出力
	//
	while ( !(m_dwAugendRegister & 0x8000) )
	{
		m_dwCodeRegister <<= 1 ;
		m_dwAugendRegister <<= 1 ;
		m_dwCodeBuffer =
			(m_dwCodeBuffer << 1) | ((m_dwCodeRegister >> 16) & 0x0001) ;
		//
		if ( m_dwCodeBuffer & 0x10000 )
		{
			m_dwBitBufferCount ++ ;
		}
		else
		{
			if ( m_dwBitBufferCount >= 0 )
			{
				ps->OutNBits( 0x00000000, 1 ) ;
				while ( m_dwBitBufferCount > 0 )
				{
					DWORD	dwBits = m_dwBitBufferCount ;
					if ( dwBits > 16 )
					{
						dwBits = 16 ;
					}
					ps->OutNBits( 0xFFFFFFFF, dwBits ) ;
					m_dwBitBufferCount -= dwBits ;
				}
				ESLAssert( m_dwBitBufferCount == 0 ) ;
			}
			m_dwBitBufferCount = 0 ;
		}
	}
	m_dwCodeRegister &= 0xFFFF ;
	m_dwCodeBuffer &= 0xFFFF ;
	//
	// 現在の統計モデルを更新
	//
	return	pModel->IncreaseSymbol( iSym ) ;
}

// ERISA 符号に符号化して出力する
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISAEncodeContext::EncodeERISACodeBytes( const SBYTE * ptrSrc, size_t nCount )
{
	ERISA_PROB_MODEL *	prob = m_pLastERISAProb ;
	SWORD	wSymbol ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		wSymbol = ptrSrc[i ++] & 0xFF ;
		EncodeERISACodeSymbol( prob, wSymbol ) ;
		if ( wSymbol == 0 )
		{
			size_t	j = i ;
			while ( i < nCount )
			{
				if ( ptrSrc[i] & 0xFF )
				{
					break ;
				}
				++ i ;
				if ( i - j >= 0xFF )
				{
					break ;
				}
			}
			size_t	len = i - j ;
			ESLAssert( len <= 0xFF ) ;
			EncodeERISACodeSymbol( m_pRunLenProb, (BYTE) len ) ;
		}
		prob = m_ppTableERISA[wSymbol] ;
	}
	m_pLastERISAProb = prob ;
	return	i ;
}

size_t SGLERISAEncodeContext::EncodeERISACodeWords( const SWORD * ptrSrc, size_t nCount )
{
	ERISA_PROB_MODEL *	prob = m_pLastERISAProb ;
	SWORD	wSymbol ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		wSymbol = ptrSrc[i ++] ;
		if ( (wSymbol >= -0x80) && (wSymbol <= 0x7F) )
		{
			EncodeERISACodeSymbol( prob, (wSymbol & 0xFF) ) ;
			prob = m_ppTableERISA[(wSymbol & 0xFF)] ;
		}
		else
		{
			EncodeERISACodeSymbol
				( prob, ERISA_ESC_CODE ) ;
			EncodeERISACodeSymbol
				( m_pPhraseIndexProb, ((wSymbol >> 8) & 0xFF) ) ;
			EncodeERISACodeSymbol
				( m_pPhraseLenProb, (wSymbol & 0xFF) ) ;
			prob = m_ppTableERISA[0x100] ;
		}
		if ( wSymbol == 0 )
		{
			size_t	j = i ;
			while ( i < nCount )
			{
				if ( ptrSrc[i] & 0xFF )
				{
					break ;
				}
				++ i ;
				if ( i - j >= 0xFF )
				{
					break ;
				}
			}
			size_t	len = i - j ;
			ESLAssert( len <= 0xFF ) ;
			EncodeERISACodeSymbol( m_pRunLenProb, (BYTE) len ) ;
		}
	}
	m_pLastERISAProb = prob ;
	return	i ;
}

// ERISA 符号を完了する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLERISAEncodeContext::FinishERISACode( void )
{
	SGLEncodeBitStream *	ps = m_pStream ;
	if ( m_dwBitBufferCount >= 0 )
	{
		ps->OutNBits( 0x00000000, 1 ) ;
		while ( m_dwBitBufferCount > 0 )
		{
			DWORD	dwBits = m_dwBitBufferCount ;
			if ( dwBits > 16 )
			{
				dwBits = 16 ;
			}
			ps->OutNBits( 0xFFFFFFFF, dwBits ) ;
			m_dwBitBufferCount -= dwBits ;
		}
	}
	ps->OutNBits( (m_dwCodeBuffer << 16), 16 ) ;
	ps->OutNBits( (m_dwCodeRegister << 16), 16 ) ;
	//
	return	SSystem::errSuccess ;
}

// 符号完了処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLERISAEncodeContext::FinishEncoding( void )
{
	FinishERISACode() ;
	return	SGLAbstractEncodeContext::FinishEncoding() ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISAEncodeContext::Write( const void * ptrBuf, size_t nBytes )
{
	return	EncodeERISACodeBytes( (const SBYTE*) ptrBuf, nBytes ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ERISA-N（算術）符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLERISANEncodeContext, SGLERISAEncodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLERISANEncodeContext::SGLERISANEncodeContext( SGLEncodeBitStream * pStream )
	: SGLERISAEncodeContext( pStream )
{
	m_pProbERISA = NULL ;
	m_pNemesisBuf = NULL ;
	m_pNemesisLookup = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLERISANEncodeContext::~SGLERISANEncodeContext( void )
{
	if ( m_pProbERISA != NULL )
	{
		esl_free( m_pProbERISA ) ;
		m_pProbERISA = NULL ;
	}
	if ( m_pNemesisBuf != NULL )
	{
		esl_free( m_pNemesisBuf ) ;
		m_pNemesisBuf = NULL ;
	}
	delete []	m_pNemesisLookup ;
}

// ERISA-N 符号の符号化の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLERISANEncodeContext::PrepareToEncodeERISANCode( DWORD dwFlags )
{
	//
	// メモリを確保
	//
	int	i ;
	if ( m_pProbERISA == NULL )
	{
		size_t	nBytes =
			sizeof(ERISA_PROB_BASE)
				+ sizeof(ERISA_PROB_MODEL) * ERISA_PROB_SLOT_MAX ;
		nBytes = (nBytes + 0x100F) & (~0xFFF) ;
		m_pProbERISA = (ERISA_PROB_BASE*) esl_malloc( nBytes ) ;
	}
	//
	BYTE *	pbytNext = (BYTE*) m_pProbERISA ;
	pbytNext += sizeof(ERISA_PROB_BASE) ;
	//
	m_iLastSymbol = 0 ;
	for ( i = 0; i < 4; i ++ )
	{
		m_bytLastSymbol[i] = 0 ;
	}
	//
	// 統計モデルを初期化
	//
	m_pProbERISA->ptrProbWork = (ERISA_PROB_MODEL*) pbytNext ;
	m_pProbERISA->dwWorkUsed = 0 ;
	m_pProbERISA->epmBaseModel.Initialize( ) ;
	//
	for ( i = 0; i < ERISA_PROB_SLOT_MAX; i ++ )
	{
		m_pProbERISA->ptrProbIndex[i] = m_pProbERISA->ptrProbWork + i ;
	}
	//
	// レジスタを初期化
	//
	m_dwCodeRegister = 0 ;
	m_dwAugendRegister = 0xFFFF ;
	m_dwCodeBuffer = 0 ;
	m_dwBitBufferCount = -1 ;
	//
	// スライド辞書セットアップ
	//
	if ( m_pNemesisBuf == NULL )
	{
		m_pNemesisBuf = (BYTE*) esl_malloc( NEMESIS_BUF_SIZE ) ;
	}
	if ( m_pNemesisLookup == NULL )
	{
		m_pNemesisLookup = new ERISAN_PHRASE_LOOKUP[0x100] ;
	}
	eslFillMemory( m_pNemesisBuf, 0, NEMESIS_BUF_SIZE ) ;
	eslFillMemory
		( m_pNemesisLookup, 0, 0x100 * sizeof(ERISAN_PHRASE_LOOKUP) ) ;
	m_nNemesisIndex = 0 ;
	//
	if ( m_pPhraseLenProb == NULL )
	{
		m_pPhraseLenProb = new ERISA_PROB_MODEL ;
	}
	if ( m_pPhraseIndexProb == NULL )
	{
		m_pPhraseIndexProb = new ERISA_PROB_MODEL ;
	}
	if ( m_pRunLenProb == NULL )
	{
		m_pRunLenProb = new ERISA_PROB_MODEL ;
	}
	//
	m_pPhraseLenProb->Initialize( ) ;
	m_pPhraseIndexProb->Initialize( ) ;
	m_pRunLenProb->Initialize( ) ;
	//
	m_dwERISAFlags = dwFlags ;
}

// ERISA-N 符号に符号化して出力する
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISANEncodeContext::EncodeERISANCodeBytes( const SBYTE * ptrSrc, size_t nCount )
{
	//
	//	指定された数のシンボルを復号し終えるまで繰り返す
	//
	size_t				nEncoded = 0 ;
	ERISA_PROB_BASE *	pBase = m_pProbERISA ;
	//
	while ( nEncoded < nCount )
	{
		//
		//	有効な統計モデルを取得する
		//////////////////////////////////////////////////////////////////////
		int	iDeg ;
		ERISA_PROB_MODEL *	pModel ;
		pModel = &(pBase->epmBaseModel) ;
		for ( iDeg = 0; iDeg < 4; iDeg ++ )
		{
			int		iLast =
				m_bytLastSymbol
					[(m_iLastSymbol + 0x03 - iDeg) & 0x03]
						>> ERISA_PROB_BASE::m_nShiftCount[iDeg] ;
			if ( pModel->acsSubModel[iLast].wSymbol < 0 )
			{
				break ;
			}
			ESLAssert( pModel->acsSubModel[iLast].wSymbol != -1 ) ;
			ESLAssert( (int)pModel->acsSubModel[iLast].wSymbol < (int)pBase->dwWorkUsed ) ;
			pModel = pBase->ptrProbWork + pModel->acsSubModel[iLast].wSymbol ;
		}
		//
		// 次のフレーズと一致するフレーズを検索する
		//
		ERISAN_PHRASE_LOOKUP *	ppl ;
		int		nMaxLength = 0 ;
		int		nPhraseIndex = -1 ;
		int		nRunLength = 0 ;
		BYTE	bytLastSymbol =
			m_pNemesisBuf[(m_nNemesisIndex - 1) & NEMESIS_BUF_MASK] ;
		ppl = &(m_pNemesisLookup[bytLastSymbol]) ;
		//
		int		i ;
		size_t	j, nLimit ;
		if ( m_dwERISAFlags & efRunLength )
		{
			nLimit = nCount - nEncoded ;
			if ( nLimit >= 0x100 )
			{
				nLimit = 0xFF ;
			}
			for ( j = 0; j < nLimit; j ++ )
			{
				if ( (BYTE) ptrSrc[j] != bytLastSymbol )
				{
					break ;
				}
			}
			nRunLength = (int) j ;
		}
		if ( m_dwERISAFlags & efNemesis )
		{
			for ( i = 1; i < NEMESIS_INDEX_LIMIT; i ++ )
			{
				int	nIndex =
					ppl->index[(ppl->first - i) & NEMESIS_INDEX_MASK] ;
				if ( m_pNemesisBuf[nIndex] != bytLastSymbol )
				{
					break ;
				}
				nLimit = nCount - nEncoded ;
				if ( nLimit >= 0x100 )
				{
					nLimit = 0xFF ;
				}
				if ( ((m_nNemesisIndex - nIndex - 1)
							& NEMESIS_BUF_MASK) < (int) nLimit )
				{
					nLimit = (m_nNemesisIndex - nIndex - 1) & NEMESIS_BUF_MASK ;
				}
				//
				for ( j = 0; j < nLimit; j ++ )
				{
					nIndex = (nIndex + 1) & NEMESIS_BUF_MASK ;
					if ( (BYTE) ptrSrc[j] != m_pNemesisBuf[nIndex] )
					{
						break ;
					}
				}
				if ( nMaxLength < (int) j )
				{
					nMaxLength = (int) j ;
					nPhraseIndex = i ;
				}
			}
		}
		//
		// スライド辞書を利用するか判定する
		//
		bool	fNemesis = false ;
		if ( (nMaxLength >= 2) || (nRunLength >= 2) )
		{
			int		nEscBits = 0 ;
			int		nPhraseBits = 0 ;
			int		nRunLenBits = 0 ;
			int		nCodeBits = 0 ;
			int		iSym ;
			//
			nEscBits += pModel->AccumulateProb( (SWORD) ERISA_ESC_CODE ) ;
			if ( pModel != &(pBase->epmBaseModel) )
			{
				nEscBits +=
					pBase->epmBaseModel.
						AccumulateProb( (SWORD) ERISA_ESC_CODE ) ;
			}
			if ( nMaxLength >= 2 )
			{
				nPhraseBits +=
					m_pPhraseIndexProb->AccumulateProb( (BYTE) nPhraseIndex ) ;
				nPhraseBits +=
					m_pPhraseLenProb->AccumulateProb( (BYTE) nMaxLength ) ;
				nPhraseBits += nEscBits ;
			}
			else
			{
				nPhraseBits = 0x7FFFFFFF ;
			}
			if ( nRunLength >= 2 )
			{
				nRunLenBits +=
					m_pPhraseIndexProb->AccumulateProb( 0 ) ;
				nRunLenBits +=
					m_pRunLenProb->AccumulateProb( (BYTE) nRunLength ) ;
				nRunLenBits += nEscBits ;
			}
			else
			{
				nRunLenBits = 0x7FFFFFFF ;
			}
			//
			if ( nRunLenBits <= nPhraseBits )
			{
				nPhraseIndex = 0 ;
				nMaxLength = nRunLength ;
				nPhraseBits = nRunLenBits ;
			}
			//
			BYTE	bytLastSymbol[4] ;
			int		iLastSymbol ;
			bytLastSymbol[0] = m_bytLastSymbol[0] ;
			bytLastSymbol[1] = m_bytLastSymbol[1] ;
			bytLastSymbol[2] = m_bytLastSymbol[2] ;
			bytLastSymbol[3] = m_bytLastSymbol[3] ;
			iLastSymbol = m_iLastSymbol ;
			//
			ERISA_PROB_MODEL *	pNext ;
			for ( int j = 0; j < nMaxLength; j ++ )
			{
				pNext = &(pBase->epmBaseModel) ;
				for ( i = 0; i < 4; i ++ )
				{
					int		iLast =
						bytLastSymbol
							[(iLastSymbol + 0x03 - i) & 0x03]
								>> ERISA_PROB_BASE::m_nShiftCount[i] ;
					if ( pNext->acsSubModel[iLast].wSymbol < 0 )
					{
						break ;
					}
					ESLAssert( pNext->acsSubModel[iLast].wSymbol != -1 ) ;
					ESLAssert( (int)pNext->acsSubModel[iLast].wSymbol < (int)pBase->dwWorkUsed ) ;
					pNext = pBase->ptrProbWork + pNext->acsSubModel[iLast].wSymbol ;
				}
				//
				BYTE	bytNextSymbol = ptrSrc[j] ;
				bytLastSymbol[iLastSymbol ++] = bytNextSymbol ;
				iLastSymbol &= 0x03 ;
				//
				iSym = pNext->FindSymbol( bytNextSymbol ) ;
				if ( iSym >= 0 )
				{
					nCodeBits += pNext->AccumulateProb( bytNextSymbol ) ;
				}
				else
				{
					nCodeBits +=
						pNext->AccumulateProb( (SWORD) ERISA_ESC_CODE ) ;
					nCodeBits +=
						pBase->epmBaseModel.AccumulateProb( bytNextSymbol ) ;
				}
			}
			if ( nCodeBits >= nPhraseBits )
			{
				fNemesis = true ;
			}
		}
		if ( fNemesis )
		{
			EncodeERISACodeSymbol( pModel, (SWORD) ERISA_ESC_CODE ) ;
			if ( pModel != &(pBase->epmBaseModel) )
			{
				EncodeERISACodeSymbol
					( &(pBase->epmBaseModel), (SWORD) ERISA_ESC_CODE ) ;
			}
			ESLAssert( !(nMaxLength & ~0xFF) ) ;
			ESLAssert( !(nPhraseIndex & ~0xFF) ) ;
			EncodeERISACodeSymbol
				( m_pPhraseIndexProb, (BYTE) nPhraseIndex ) ;
			if ( nPhraseIndex == 0 )
			{
				EncodeERISACodeSymbol
					( m_pRunLenProb, (BYTE) nMaxLength ) ;
			}
			else
			{
				EncodeERISACodeSymbol
					( m_pPhraseLenProb, (BYTE) nMaxLength ) ;
			}
			//
			// スライド辞書に反映する
			//
			for ( i = 0; i < nMaxLength; i ++ )
			{
				BYTE	bytSymbol = ptrSrc[i] ;
				m_bytLastSymbol[m_iLastSymbol ++] = bytSymbol ;
				m_iLastSymbol &= 0x03 ;
				//
				ERISAN_PHRASE_LOOKUP *	ppl = &(m_pNemesisLookup[bytSymbol]) ;
				ppl->index[ppl->first] = m_nNemesisIndex ;
				ppl->first = (ppl->first + 1) & NEMESIS_INDEX_MASK ;
				bytLastSymbol = bytSymbol ;
				//
				m_pNemesisBuf[m_nNemesisIndex ++] = bytSymbol ;
				m_nNemesisIndex &= NEMESIS_BUF_MASK ;
			}
			//
			ptrSrc += nMaxLength ;
			nEncoded += nMaxLength ;
			//
			continue ;
		}
		//
		BYTE	bytSymbol = *ptrSrc ;
		//
		m_bytLastSymbol[m_iLastSymbol ++] = bytSymbol ;
		m_iLastSymbol &= 0x03 ;
		//
		ppl = &(m_pNemesisLookup[bytSymbol]) ;
		ppl->index[ppl->first] = m_nNemesisIndex ;
		ppl->first = (ppl->first + 1) & NEMESIS_INDEX_MASK ;
		m_pNemesisBuf[m_nNemesisIndex ++] = bytSymbol ;
		m_nNemesisIndex &= NEMESIS_BUF_MASK ;
		//
		ptrSrc ++ ;
		nEncoded ++ ;
		//
		//	算術符号化
		//////////////////////////////////////////////////////////////////////
		int		iSym = 0 ;
		WORD	wFs = 0 ;
		while ( pModel->acsSymTable[iSym].wSymbol != bytSymbol )
		{
			wFs += pModel->acsSymTable[iSym ++].wOccured ;
			if ( (DWORD) iSym >= pModel->dwSymbolSorts )
			{
				break ;
			}
		}
		int		iSymIndex ;
		if ( (DWORD) iSym < pModel->dwSymbolSorts )
		{
			ESLAssert( wFs < pModel->dwTotalCount ) ;
			iSymIndex = EncodeERISACodeIndex( pModel, iSym, wFs ) ;
		}
		else
		{
			EncodeERISACodeSymbol( pModel, (SWORD) ERISA_ESC_CODE ) ;
			EncodeERISACodeSymbol( &(pBase->epmBaseModel), bytSymbol ) ;
			iSymIndex = pModel->AddSymbol( bytSymbol ) ;
		}
		//
		// 統計モデルのツリーを拡張
		//////////////////////////////////////////////////////////////////////
		if ( (pBase->dwWorkUsed < ERISA_PROB_SLOT_MAX) && (iDeg < 4) )
		{
			int	iSymbol =
				bytSymbol >> ERISA_PROB_BASE::m_nShiftCount[iDeg] ;
			ESLAssert( iSymbol < ERISA_SUB_SORT_MAX ) ;
			if ( ++ pModel->acsSubModel[iSymbol].wOccured
					>= ERISA_PROB_BASE::m_nNewProbLimit[iDeg] )
			{
				int	i ;
				ERISA_PROB_MODEL *	pParent = pModel ;
				pModel = &(pBase->epmBaseModel) ;
				for ( i = 0; i <= iDeg; i ++ )
				{
					iSymbol = m_bytLastSymbol
						[(m_iLastSymbol + 0x03 - i) & 0x03]
								>> ERISA_PROB_BASE::m_nShiftCount[i] ;
					if ( pModel->acsSubModel[iSymbol].wSymbol < 0 )
					{
						break ;
					}
					ESLAssert
						( (DWORD) pModel->acsSubModel[iSymbol].
										wSymbol < pBase->dwWorkUsed ) ;
					pModel = pBase->ptrProbWork
								+ pModel->acsSubModel[iSymbol].wSymbol ;
				}
				if ( (i <= iDeg) &&
					(pModel->acsSubModel[iSymbol].wSymbol < 0) )
				{
					ERISA_PROB_MODEL *	pNew ;
					pNew = pBase->ptrProbWork + pBase->dwWorkUsed ;
					pModel->acsSubModel[iSymbol].
								wSymbol = (SWORD) (pBase->dwWorkUsed ++) ;
					//
					pNew->dwTotalCount = 0 ;
					int	j = 0 ;
					for ( i = 0; i < (int) pParent->dwSymbolSorts; i ++ )
					{
						WORD	wOccured =
							(pParent->acsSymTable[i].wOccured >> 4) ;
						if ( (wOccured > 0) &&
							(pParent->acsSymTable[i].wSymbol != ERISA_ESC_CODE) )
						{
							pNew->dwTotalCount += wOccured ;
							pNew->acsSymTable[j].wOccured = wOccured ;
							pNew->acsSymTable[j].wSymbol
								= pParent->acsSymTable[i].wSymbol ;
							j ++ ;
						}
					}
					pNew->dwTotalCount ++ ;
					pNew->acsSymTable[j].wOccured = 1 ;
					pNew->acsSymTable[j].wSymbol = (SWORD) ERISA_ESC_CODE ;
					pNew->dwSymbolSorts = ++ j ;
					//
					for ( i = 0; i < ERISA_SUB_SORT_MAX; i ++ )
					{
						pNew->acsSubModel[i].wOccured = 0 ;
						pNew->acsSubModel[i].wSymbol = (SWORD) -1 ;
					}
				}
			}
		}
	}
	return	nEncoded ;
}

// ERISA-N 符号の EOF を出力する
//////////////////////////////////////////////////////////////////////////////
void SGLERISANEncodeContext::EncodeERISANCodeEOF( void )
{
	//
	//	有効な統計モデルを取得する
	//
	ERISA_PROB_MODEL *	pModel ;
	ERISA_PROB_BASE *	pBase = m_pProbERISA ;
	pModel = &(pBase->epmBaseModel) ;
	int	iDeg ;
	for ( iDeg = 0; iDeg < 4; iDeg ++ )
	{
		int	iLast =
			m_bytLastSymbol
				[(m_iLastSymbol + 0x03 - iDeg) & 0x03]
					>> ERISA_PROB_BASE::m_nShiftCount[iDeg] ;
		if ( pModel->acsSubModel[iLast].wSymbol < 0 )
		{
			break ;
		}
		ESLAssert( pModel->acsSubModel[iLast].wSymbol != -1 ) ;
		ESLAssert( (int)pModel->acsSubModel[iLast].wSymbol < (int)pBase->dwWorkUsed ) ;
		pModel = pBase->ptrProbWork + pModel->acsSubModel[iLast].wSymbol ;
	}
	//
	// ESC 符号を出力する
	//
	EncodeERISACodeSymbol( pModel, (SWORD) ERISA_ESC_CODE ) ;
	if ( pModel != &(pBase->epmBaseModel) )
	{
		EncodeERISACodeSymbol
			( &(pBase->epmBaseModel), (SWORD) ERISA_ESC_CODE ) ;
	}
	EncodeERISACodeSymbol( m_pPhraseIndexProb, (SWORD) ERISA_ESC_CODE ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISANEncodeContext::Write( const void * ptrBuf, size_t nBytes )
{
	return	EncodeERISANCodeBytes( (const SBYTE*) ptrBuf, nBytes ) ;
}

