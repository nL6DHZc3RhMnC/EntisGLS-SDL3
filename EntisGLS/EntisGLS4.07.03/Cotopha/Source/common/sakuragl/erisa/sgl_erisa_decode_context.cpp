
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
// 復号ビットストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLDecodeBitStream, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDecodeBitStream::~SGLDecodeBitStream( void )
{
	if ( m_ptrBuffer != NULL )
	{
		esl_free( m_ptrBuffer ) ;
		m_ptrBuffer = NULL ;
	}
}

// バッファが空の時、次のデータを読み込む
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLDecodeBitStream::PrefetchBuffer( void )
{
	if ( m_nIntBufCount == 0 )
	{
		if ( m_nBufCount == 0 )
		{
			m_ptrNextBuf = m_ptrBuffer ;
			m_nBufCount = m_pStream->Read( m_ptrBuffer, m_nBufferingSize ) ;
			if ( m_nBufCount == 0 )
			{
				return	SSystem::errFailed ;
			}
			if ( m_nBufCount & 0x03 )
			{
				size_t	i = m_nBufCount ;
				m_nBufCount += 4 - (m_nBufCount & 0x03) ;
				while ( i < m_nBufCount )
				{
					m_ptrBuffer[i ++] = 0x00 ;
				}
			}
		}
		m_nIntBufCount = 32 ;
		m_dwIntBuffer =
			((DWORD)m_ptrNextBuf[0] << 24) | ((DWORD)m_ptrNextBuf[1] << 16)
				| ((DWORD)m_ptrNextBuf[2] << 8) | ((DWORD)m_ptrNextBuf[3]) ;
		m_ptrNextBuf += 4 ;
		m_nBufCount -= 4 ;
	}
	return	SSystem::errSuccess ;
}

#if	defined(__COTOPHA__)
#define	SGLDecoder_PrefetchBuffer(ps,regIntBufCount,regIntBuffer)	\
	REG LOAD	m_nBufCount : [ps].m_nBufCount ;	\
	.IF		(uint32) m_nBufCount != (uint32) #zero ;	\
		REG LOAD	m_ptrNextBuf : [ps].m_ptrNextBuf ;	\
		move		regIntBufCount, 32 ;	\
		load.uint32	acc, [m_ptrNextBuf] ;	\
		add			m_ptrNextBuf, 4 ;	\
		psll.d		regIntBuffer, acc, 24 ;	\
		psrl.d		r1, acc, 24 ;	\
		psll.d		r2, #ff, 8 ;	\
		psrl.d		acc, 8 ;	\
		or			regIntBuffer, r1 ;	\
		and			r2, acc ;	\
		and			acc, #ff ;	\
		or			regIntBuffer, r2 ;	\
		psll.d		acc, 16 ;	\
		or			regIntBuffer, acc ;	\
		add			m_nBufCount, -4 ;	\
		REG FLUSH	m_ptrNextBuf, m_nBufCount ; \
		REG FREE	m_ptrNextBuf, m_nBufCount ;	\
	.ELSE ;	\
		REG FLUSH	regIntBufCount, regIntBuffer ;	\
		INVOKE	ERISA::SGLDecodeBitStream::PrefetchBuffer, ps ; \
		.IF		acc != #zero ;	\
			move	acc, #zero ;	\
			ret ;	\
		.ENDIF ;	\
		REG RELOAD	ps ;	\
		REG RELOAD	regIntBufCount,regIntBuffer ;	\
	.ENDIF
#endif


//////////////////////////////////////////////////////////////////////////////
// 復号コンテキスト基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLAbstractDecodeContext, SInputStream )



//////////////////////////////////////////////////////////////////////////////
// ランレングスガンマ符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

#define	_dmy	(DWORD) -1
const DWORD SGLGammaDecodeContext::m_nGammaCodeLookup[0x200] =
{
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,   2,  2,
   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,
   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,   4,  4,
   8,  6,   8,  6,   8,  6,   8,  6,  16,  8,  _dmy, _dmy,  17,  8,  _dmy, _dmy,
   9,  6,   9,  6,   9,  6,   9,  6,  18,  8,  _dmy, _dmy,  19,  8,  _dmy, _dmy,
   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,
   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,   5,  4,
  10,  6,  10,  6,  10,  6,  10,  6,  20,  8,  _dmy, _dmy,  21,  8,  _dmy, _dmy,
  11,  6,  11,  6,  11,  6,  11,  6,  22,  8,  _dmy, _dmy,  23,  8,  _dmy, _dmy,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,   3,  2,
   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,
   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,   6,  4,
  12,  6,  12,  6,  12,  6,  12,  6,  24,  8,  _dmy, _dmy,  25,  8,  _dmy, _dmy,
  13,  6,  13,  6,  13,  6,  13,  6,  26,  8,  _dmy, _dmy,  27,  8,  _dmy, _dmy,
   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,
   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,   7,  4,
  14,  6,  14,  6,  14,  6,  14,  6,  28,  8,  _dmy, _dmy,  29,  8,  _dmy, _dmy,
  15,  6,  15,  6,  15,  6,  15,  6,  30,  8,  _dmy, _dmy,  31,  8,  _dmy, _dmy
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLGammaDecodeContext, SGLAbstractDecodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGammaDecodeContext::SGLGammaDecodeContext( SGLDecodeBitStream * pStream )
	: SGLAbstractDecodeContext( pStream )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLGammaDecodeContext::~SGLGammaDecodeContext( void )
{
}

// ランレングスガンマ符号のゼロフラグを読み込む
//////////////////////////////////////////////////////////////////////////////
void SGLGammaDecodeContext::InitGammaContext( void )
{
	m_flgZero = (m_pStream->GetABit() != 0) ;
	m_nLength = 0 ;
}

// ガンマコードを取得する
//////////////////////////////////////////////////////////////////////////////
int SGLGammaDecodeContext::GetGammaCode( void )
{
//#if	defined(__COTOPHA__)
#if	0
	uint32_t	nCode = 0, nBase = 2 ;
	asm
	{
		REG LOAD	ps : [tp].m_pStream
		REG LOAD	m_nIntBufCount : [ps].m_nIntBufCount
		//
		// １の判定
		//
		REG LOAD	m_dwIntBuffer : [ps].m_dwIntBuffer
		.IF	(uint32) m_nIntBufCount == (uint32) #zero
			SGLDecoder_PrefetchBuffer(ps,m_nIntBufCount,m_dwIntBuffer)
		.ENDIF
		psub.d		m_nIntBufCount, #one
		psrl.d		acc, m_dwIntBuffer, 31
		psll.d		m_dwIntBuffer, 1
		.IF			!acc
			REG FLUSH	m_nIntBufCount, m_dwIntBuffer
			move		acc, #one
			ret
		.ENDIF
		//
		// 符号終端の判定
		//
		.IF	(uint32) m_nIntBufCount == (uint32) #zero
			SGLDecoder_PrefetchBuffer(ps,m_nIntBufCount,m_dwIntBuffer)
		.ENDIF
		move	r1, m_dwIntBuffer
		move	acc, 0x55000000
		not		r1
		and		acc, r1
		.IF		((uint32) acc != (uint32) #zero) & (m_nIntBufCount >= 8)
			REG ALLOC	i : uint32
			REG ALLOC	nBitCount : uint32
			move		acc, ERISA::SGLGammaDecodeContext::m_nGammaCodeLookup
			psrl.d		i, m_dwIntBuffer, 24
			load.uint32	nBitCount, [acc+i*8+4]
			load.uint32	acc, [acc+i*8]
			psub.d		m_nIntBufCount, nBitCount
			psll.d		m_dwIntBuffer, nBitCount
			REG FLUSH	m_nIntBufCount, m_dwIntBuffer
			REG FREE	i, nBitCount
			ret
		.ENDIF
		REG FLUSH	m_nIntBufCount, m_dwIntBuffer
		//
		// 汎用ルーチン
		//
		REG LOAD	nCode
		REG LOAD	nBase
Loop_Begin:
		.IF		(uint32) m_nIntBufCount >= (uint32) 2
			//
			// 2 ビット一括処理
			//
			move	acc, m_dwIntBuffer
			psll.d	m_dwIntBuffer, 2
			psll.d	nCode, 1
			psrl.d	r1, acc, 31
			or		nCode, r1
			psub.d	m_nIntBufCount, #one
			psub.d	m_nIntBufCount, #one
			psrl.d	acc, 30
			.IF		!acc
				REG FLUSH	m_nIntBufCount, m_dwIntBuffer
				move	acc, nCode
				padd.d	acc, nBase
				ret
			.ENDIF
			psll.d	nBase, 1
			jump	Loop_Begin
		.ELSE
			//
			// 1 ビット取り出し
			//
			.IF	(uint32) m_nIntBufCount == (uint32) #zero
				REG FLUSH	nCode, nBase
				SGLDecoder_PrefetchBuffer(ps,m_nIntBufCount,m_dwIntBuffer)
				REG RELOAD	nCode, nBase
			.ENDIF
			move	acc, m_dwIntBuffer
			psll.d	nCode, 1
			psrl.d	acc, 31
			psub.d	m_nIntBufCount, #one
			or		nCode, acc
			psll.d	m_dwIntBuffer, 1
			//
			// 符号終端判定
			//
			.IF	(uint32) m_nIntBufCount == (uint32) #zero
				REG FLUSH	nCode, nBase
				SGLDecoder_PrefetchBuffer(ps,m_nIntBufCount,m_dwIntBuffer)
				REG RELOAD	nCode, nBase
			.ENDIF
			psrl.d	acc, m_dwIntBuffer, 31
			psub.d	m_nIntBufCount, #one
			psll.d	m_dwIntBuffer, 1
			.IF		!acc
				REG FLUSH	m_nIntBufCount, m_dwIntBuffer
				move	acc, nCode
				padd.d	acc, nBase
				ret
			.ENDIF
			psll.d	nBase, 1
		.ENDIF
		jump	Loop_Begin
	}
#else
	SGLDecodeBitStream *	ps = m_pStream ;
	//
	// １の判定
	//
	if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
	{
		return	0 ;
	}
	DWORD	dwIntBuf ;
	ps->m_nIntBufCount -- ;
	dwIntBuf = ps->m_dwIntBuffer ;
	ps->m_dwIntBuffer <<= 1 ;
	if ( !(dwIntBuf & 0x80000000) )
	{
		return	1 ;
	}
	//
	// 符号終端の判定
	//
	if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
	{
		return	0 ;
	}
	if ( (~ps->m_dwIntBuffer & 0x55000000)
					&& (ps->m_nIntBufCount >= 8) )
	{
		DWORD	i = (ps->m_dwIntBuffer >> 24) << 1 ;
		DWORD	nCode = m_nGammaCodeLookup[i] ;
		DWORD	nBitCount = m_nGammaCodeLookup[i + 1] ;
		ESLAssert( nBitCount <= ps->m_nIntBufCount ) ;
		ESLAssert( nCode > 0 ) ;
		ps->m_nIntBufCount -= nBitCount ;
		ps->m_dwIntBuffer <<= nBitCount ;
		return	nCode ;
	}
	//
	// 汎用ルーチン
	//
	int	nCode = 0, nBase = 2 ;
	for ( ; ; )
	{
		if ( ps->m_nIntBufCount >= 2 )
		{
			//
			// 2 ビット一括処理
			//
			dwIntBuf = ps->m_dwIntBuffer ;
			ps->m_dwIntBuffer <<= 2 ;
			nCode = (nCode << 1) | (dwIntBuf >> 31) ;
			ps->m_nIntBufCount -= 2 ;
			if ( !(dwIntBuf & 0x40000000) )
			{
				return	nCode + nBase ;
			}
			nBase <<= 1 ;
		}
		else
		{
			//
			// 1 ビット取り出し
			//
			if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
			{
				return	0 ;
			}
			nCode = (nCode << 1) | (ps->m_dwIntBuffer >> 31) ;
			ps->m_nIntBufCount -- ;
			ps->m_dwIntBuffer <<= 1 ;
			//
			// 符号終端判定
			//
			if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
			{
				return	0 ;
			}
			dwIntBuf = ps->m_dwIntBuffer ;
			ps->m_nIntBufCount -- ;
			ps->m_dwIntBuffer <<= 1 ;
			if ( !(dwIntBuf & 0x80000000) )
			{
				return	nCode + nBase ;
			}
			nBase <<= 1 ;
		}
	}
#endif
	return	0 ;
}

// ランレングスガンマ符号を復号する
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaDecodeContext::DecodeGammaCodeBytes( SBYTE * ptrDst, size_t nCount )
{
	size_t	nDecoded = 0, nRepeat ;
	SBYTE	nSign, nCode ;
	//
	if ( m_nLength == 0 )
	{
		m_nLength = (size_t) GetGammaCode() ;
		if ( m_nLength == 0 )
		{
			return	nDecoded ;
		}
	}
	//
	for ( ; ; )
	{
		//
		// 出力シンボル数を算出
		//
		nRepeat = m_nLength ;
		if ( nRepeat > nCount )
		{
			nRepeat = nCount ;
		}
		ESLAssert( nRepeat > 0 ) ;
		m_nLength -= nRepeat ;
		nCount -= nRepeat ;
		//
		// シンボルを出力
		//
		if ( !m_flgZero )
		{
			nDecoded += nRepeat ;
			do
			{
				*(ptrDst ++) = 0 ;
			}
			while ( -- nRepeat ) ;
		}
		else
		{
			do
			{
				nSign = (SBYTE) m_pStream->GetABit() ;
				nCode = (SBYTE) GetGammaCode() ;
				if ( nCode == 0 )
				{
					return	nDecoded ;
				}
				nDecoded ++ ;
				*(ptrDst ++) = (nCode ^ nSign) - nSign ;
			}
			while ( -- nRepeat ) ;
		}
		//
		// 終了か？
		//
		if ( nCount == 0 )
		{
			if ( m_nLength == 0 )
			{
				m_flgZero = !m_flgZero ;
			}
			return	nDecoded ;
		}
		//
		// レングスコードを取得
		//
		m_flgZero = !m_flgZero ;
		m_nLength = (size_t) GetGammaCode() ;
		if ( m_nLength == 0 )
		{
			return	nDecoded ;
		}
	}
	return	nDecoded ;
}

size_t SGLGammaDecodeContext::DecodeGammaCodeWords( SWORD * ptrDst, size_t nCount )
{
	size_t	nDecoded = 0, nRepeat ;
	SWORD	nSign, nCode ;
	//
	if ( m_nLength == 0 )
	{
		m_nLength = (size_t) GetGammaCode() ;
		if ( m_nLength == 0 )
		{
			return	nDecoded ;
		}
	}
	//
	for ( ; ; )
	{
		//
		// 出力シンボル数を算出
		//
		nRepeat = m_nLength ;
		if ( nRepeat > nCount )
		{
			nRepeat = nCount ;
		}
		ESLAssert( nRepeat > 0 ) ;
		m_nLength -= nRepeat ;
		nCount -= nRepeat ;
		//
		// シンボルを出力
		//
		if ( !m_flgZero )
		{
			nDecoded += nRepeat ;
			do
			{
				*(ptrDst ++) = 0 ;
			}
			while ( -- nRepeat ) ;
		}
		else
		{
			do
			{
				nSign = (SWORD) m_pStream->GetABit() ;
				nCode = (SWORD) GetGammaCode() ;
				if ( nCode == 0 )
				{
					return	nDecoded ;
				}
				nDecoded ++ ;
				*(ptrDst ++) = (nCode ^ nSign) - nSign ;
			}
			while ( -- nRepeat ) ;
		}
		//
		// 終了か？
		//
		if ( nCount == 0 )
		{
			if ( m_nLength == 0 )
			{
				m_flgZero = !m_flgZero ;
			}
			return	nDecoded ;
		}
		//
		// レングスコードを取得
		//
		m_flgZero = !m_flgZero ;
		m_nLength = (size_t) GetGammaCode() ;
		if ( m_nLength == 0 )
		{
			return	nDecoded ;
		}
	}
	return	nDecoded ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLGammaDecodeContext::Read( void * ptrBuf, size_t nBytes )
{
	return	DecodeGammaCodeBytes( (SBYTE*) ptrBuf, nBytes ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ハフマン符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLHuffmanDecodeContext, SGLGammaDecodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLHuffmanDecodeContext::SGLHuffmanDecodeContext( SGLDecodeBitStream * pStream )
	: SGLGammaDecodeContext( pStream )
{
	m_ppHuffmanTree = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLHuffmanDecodeContext::~SGLHuffmanDecodeContext( void )
{
	if ( m_ppHuffmanTree != NULL )
	{
		esl_free( m_ppHuffmanTree ) ;
		m_ppHuffmanTree = NULL ;
	}
}

// ERINA 符号の復号の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLHuffmanDecodeContext::PrepareToDecodeERINACode( DWORD dwFlags )
{
	//
	// メモリを確保
	//
	int		i ;
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
	m_nLength = 0 ;
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

// ハフマン符号を取得する
//////////////////////////////////////////////////////////////////////////////
int SGLHuffmanDecodeContext::GetHuffmanCode( ERISA::ERINA_HUFFMAN_TREE * tree )
{
	SGLDecodeBitStream *	ps = m_pStream ;
	int						nCode ;
	if ( tree->m_iEscape != ERINA_HUFFMAN_NULL )
	{
		DWORD	iEntry = ERINA_HUFFMAN_ROOT ;
		DWORD	iChild = tree->m_hnTree[ERINA_HUFFMAN_ROOT].m_child_code ;
		//
		// 符号を復号
		//
		do
		{
			if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
			{
				return	ERINA_HUFFMAN_ESCAPE ;
			}
			//
			// 1ビット取り出す
			//
			iEntry = iChild + (ps->m_dwIntBuffer >> 31) ;
			-- ps->m_nIntBufCount ;
			iChild = tree->m_hnTree[iEntry].m_child_code ;
			ps->m_dwIntBuffer <<= 1 ;
		}
		while ( !(iChild & ERINA_CODE_FLAG) ) ;
		//
		// 符号の出現頻度を加算
		//
		if ( (m_dwERINAFlags != efERINAOrder0) ||
			(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
							m_weight < ERINA_HUFFMAN_MAX-1) )
		{
			tree->IncreaseOccuedCount( iEntry ) ;
		}
		//
		// エスケープコードか判別
		//
		nCode = (int) (iChild & ~ERINA_CODE_FLAG) ;
		if ( nCode != ERINA_HUFFMAN_ESCAPE )
		{
			return	nCode ;
		}
	}
	//
	// エスケープコードのときは8ビット固定長
	//
	nCode = ps->GetNBits( 8 ) ;
	tree->AddNewEntry( nCode ) ;
	//
	return	nCode ;
}

// 長さのハフマン符号を取得する
//////////////////////////////////////////////////////////////////////////////
int SGLHuffmanDecodeContext::GetLengthHuffman( ERISA::ERINA_HUFFMAN_TREE * tree )
{
	SGLDecodeBitStream *	ps = m_pStream ;
	int						nCode ;
	if ( tree->m_iEscape != ERINA_HUFFMAN_NULL )
	{
		DWORD	iEntry = ERINA_HUFFMAN_ROOT ;
		DWORD	iChild = tree->m_hnTree[ERINA_HUFFMAN_ROOT].m_child_code ;
		//
		// 符号を復号
		//
		do
		{
			if ( (ps->m_nIntBufCount == 0) && ps->PrefetchBuffer() )
			{
				return	ERINA_HUFFMAN_ESCAPE ;
			}
			//
			// 1ビット取り出す
			//
			iEntry = iChild + (ps->m_dwIntBuffer >> 31) ;
			-- ps->m_nIntBufCount ;
			iChild = tree->m_hnTree[iEntry].m_child_code ;
			ps->m_dwIntBuffer <<= 1 ;
		}
		while ( !(iChild & ERINA_CODE_FLAG) ) ;
		//
		// 符号の出現頻度を加算
		//
		if ( (m_dwERINAFlags != efERINAOrder0) ||
			(tree->m_hnTree[ERINA_HUFFMAN_ROOT].
							m_weight < ERINA_HUFFMAN_MAX-1) )
		{
			tree->IncreaseOccuedCount( iEntry ) ;
		}
		//
		// エスケープコードか判別
		//
		nCode = (int) (iChild & ~ERINA_CODE_FLAG) ;
		if ( nCode != ERINA_HUFFMAN_ESCAPE )
		{
			return	nCode ;
		}
	}
	//
	// エスケープコードのときはガンマ符号
	nCode = GetGammaCode( ) ;
	if ( nCode == -1 )
	{
		return	ERINA_HUFFMAN_ESCAPE ;
	}
	tree->AddNewEntry( nCode ) ;
	//
	return	nCode ;
}

// ERINA 符号を復号する
//////////////////////////////////////////////////////////////////////////////
size_t SGLHuffmanDecodeContext::DecodeERINACodeBytes( SBYTE * ptrDst, size_t nCount )
{
	ERINA_HUFFMAN_TREE *	tree = m_pLastHuffmanTree ;
	int		nSymbol ;
	size_t	nLength ;
	size_t	i = 0 ;
	if ( m_nLength > 0 )
	{
		nLength = m_nLength ;
		if ( nLength > (int) nCount )
		{
			nLength = nCount ;
		}
		m_nLength -= nLength ;
		do
		{
			ptrDst[i ++] = 0 ;
		}
		while ( -- nLength ) ;
	}
	while ( i < nCount )
	{
		nSymbol = GetHuffmanCode( tree ) ;
		if ( nSymbol == ERINA_HUFFMAN_ESCAPE )
		{
			break ;
		}
		ptrDst[i ++] = (SBYTE) nSymbol ;
		//
		if ( nSymbol == 0 )
		{
			nLength = GetLengthHuffman( m_ppHuffmanTree[0x100] ) ;
			if ( nLength == ERINA_HUFFMAN_ESCAPE )
			{
				break ;
			}
			if ( -- nLength )
			{
				m_nLength = nLength ;
				if ( i + nLength > nCount )
				{
					nLength = nCount - i ;
				}
				m_nLength -= nLength ;
				if ( nLength > 0 )
				{
					do
					{
						ptrDst[i ++] = 0 ;
					}
					while ( -- nLength ) ;
				}
			}
		}
		tree = m_ppHuffmanTree[nSymbol & 0xFF] ;
	}
	m_pLastHuffmanTree = tree ;
	//
	return	i ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLHuffmanDecodeContext::Read( void * ptrBuf, size_t nBytes )
{
	return	DecodeERINACodeBytes( (SBYTE*) ptrBuf, nBytes ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ERISA（算術）符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLERISADecodeContext, SGLAbstractDecodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLERISADecodeContext::SGLERISADecodeContext( SGLDecodeBitStream * pStream )
	: SGLAbstractDecodeContext( pStream )
{
	m_ppTableERISA = NULL ;
	m_pPhraseLenProb = NULL ;
	m_pPhraseIndexProb = NULL ;
	m_pRunLenProb = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLERISADecodeContext::~SGLERISADecodeContext( void )
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

// ERISA 符号の復号の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLERISADecodeContext::PrepareToDecodeERISACode( void )
{
	//
	// メモリを確保
	//
	int		i ;
	if ( m_ppTableERISA == NULL )
	{
		size_t	nBytes =
			sizeof(ERISA_PROB_MODEL*) * 0x104
					+ sizeof(ERISA_PROB_MODEL) * 0x101 ;
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
	InitializeERISACode() ;
}

// 算術符号の復号の初期化を行う
//////////////////////////////////////////////////////////////////////////////
void SGLERISADecodeContext::InitializeERISACode( void )
{
	m_dwCodeRegister = m_pStream->GetNBits( 32 ) ;
	m_dwAugendRegister = 0xFFFF ;
	m_nLength = 0 ;
	m_nPostBitCount = 0 ;
}

// 指定の統計モデルを使って1つの算術符号を復号
//////////////////////////////////////////////////////////////////////////////
int SGLERISADecodeContext::DecodeERISACode( ERISA::ERISA_PROB_MODEL * pModel )
{
	int	iSym = DecodeERISACodeIndex( pModel ) ;
	int	nSymbol = ERISA_ESC_CODE ;
	if ( iSym >= 0 )
	{
		nSymbol = pModel->acsSymTable[iSym].wSymbol ;
		pModel->IncreaseSymbol( iSym ) ;
	}
	return	nSymbol ;
}

int SGLERISADecodeContext::DecodeERISACodeIndex( ERISA::ERISA_PROB_MODEL * pModel )
{
	//
	// 指標を復号して検索
	//
	DWORD	dwAcc = m_dwCodeRegister
					* pModel->dwTotalCount
					/ m_dwAugendRegister ;
//	if ( dwAcc >= ERISA_TOTAL_LIMIT )
//	{
//		return	-1 ;			// エラー
//	}
	//
	int		iSym = 0 ;
	WORD	wAcc = (WORD) dwAcc ;
	WORD	wFs = 0 ;
	WORD	wOccured ;
	for ( ; ; )
	{
		wOccured = pModel->acsSymTable[iSym].wOccured ;
		if ( wAcc < wOccured )
		{
			break ;
		}
		wAcc -= wOccured ;
		wFs += wOccured ;
		if ( (DWORD) ++ iSym >= pModel->dwSymbolSorts )
		{
			return	-1 ;		// エラー
		}
	}
	//
	// コードレジスタとオージェンドレジスタを更新
	//
	m_dwCodeRegister -=
		(m_dwAugendRegister * wFs
			+ pModel->dwTotalCount - 1) / pModel->dwTotalCount ;
	m_dwAugendRegister =
		m_dwAugendRegister * wOccured / pModel->dwTotalCount ;
	ESLAssert( m_dwAugendRegister != 0 ) ;
	//
	// オージェントレジスタを正規化し、コードレジスタに符号を読み込む
	//
	SGLDecodeBitStream *	ps = m_pStream ;
	while ( !(m_dwAugendRegister & 0x8000) )
	{
		//
		// コードレジスタにシフトイン
		//
		int	nNextBit = ps->GetABit( ) ;
		if ( nNextBit == 1 )
		{
			if ( (++ m_nPostBitCount) >= 256 )
			{
				return	-1 ;		// エラー
			}
			nNextBit = 0 ;
		}
		m_dwCodeRegister =
			(m_dwCodeRegister << 1) | (nNextBit & 0x01) ;
		//
		m_dwAugendRegister <<= 1 ;
	}
	ESLAssert( m_dwAugendRegister & 0x8000 ) ;
	m_dwCodeRegister &= 0xFFFF ;
	//
	return	iSym ;
}

// ERISA 符号を復号する
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISADecodeContext::DecodeERISACodeBytes( SBYTE * ptrDst, size_t nCount )
{
	ERISA_PROB_MODEL *	pProb = m_pLastERISAProb ;
	int		nSymbol, iSym ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		if ( m_nLength > 0 )
		{
			//
			// 零ランレングス
			//
			size_t	nCurrent = nCount - i ;
			if ( nCurrent > m_nLength )
			{
				nCurrent = m_nLength ;
			}
			m_nLength -= nCurrent ;
			for ( size_t j = 0; j < nCurrent; j ++ )
			{
				ptrDst[i ++] = 0 ;
			}
			continue ;
		}
		//
		// 次の算術符号を復号
		//
		iSym = DecodeERISACodeIndex( pProb ) ;
		if ( iSym < 0 )
		{
			break ;
		}
		nSymbol = pProb->acsSymTable[iSym].wSymbol ;
		pProb->IncreaseSymbol( iSym ) ;
		ptrDst[i ++] = (BYTE) nSymbol ;
		//
		if ( nSymbol == 0 )
		{
			//
			// 零レングスを取得
			//
			iSym = DecodeERISACodeIndex( m_pRunLenProb ) ;
			if ( iSym < 0 )
			{
				break ;
			}
			m_nLength = m_pRunLenProb->acsSymTable[iSym].wSymbol ;
			m_pRunLenProb->IncreaseSymbol( iSym ) ;
		}
		pProb = m_ppTableERISA[(nSymbol & 0xFF)] ;
	}
	m_pLastERISAProb = pProb ;
	//
	return	i ;
}

size_t SGLERISADecodeContext::DecodeERISACodeWords( SWORD * ptrDst, size_t nCount )
{
	ERISA_PROB_MODEL *	pProb = m_pLastERISAProb ;
	int		nSymbol, iSym ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		if ( m_nLength > 0 )
		{
			//
			// 零ランレングス
			//
			size_t	nCurrent = nCount - i ;
			if ( nCurrent > m_nLength )
			{
				nCurrent = m_nLength ;
			}
			m_nLength -= nCurrent ;
			for ( size_t j = 0; j < nCurrent; j ++ )
			{
				ptrDst[i ++] = 0 ;
			}
			continue ;
		}
		//
		// 次の算術符号を復号
		//
		iSym = DecodeERISACodeIndex( pProb ) ;
		if ( iSym < 0 )
		{
			break ;
		}
		nSymbol = pProb->acsSymTable[iSym].wSymbol ;
		pProb->IncreaseSymbol( iSym ) ;
		//
		if ( nSymbol == ERISA_ESC_CODE )
		{
			iSym = DecodeERISACodeIndex( m_pPhraseIndexProb ) ;
			if ( iSym < 0 )
			{
				break ;
			}
			nSymbol = m_pPhraseIndexProb->acsSymTable[iSym].wSymbol ;
			m_pPhraseIndexProb->IncreaseSymbol( iSym ) ;
			//
			iSym = DecodeERISACodeIndex( m_pPhraseLenProb ) ;
			if ( iSym < 0 )
			{
				break ;
			}
			nSymbol = (nSymbol << 8)
				| (m_pPhraseLenProb->acsSymTable[iSym].wSymbol & 0xFF) ;
			m_pPhraseLenProb->IncreaseSymbol( iSym ) ;
			//
			ptrDst[i ++] = (WORD) nSymbol ;
			pProb = m_ppTableERISA[0x100] ;
		}
		else
		{
			ptrDst[i ++] = (SBYTE) nSymbol ;
			pProb = m_ppTableERISA[(nSymbol & 0xFF)] ;
			//
			if ( nSymbol == 0 )
			{
				//
				// 零レングスを取得
				//
				iSym = DecodeERISACodeIndex( m_pRunLenProb ) ;
				if ( iSym < 0 )
				{
					break ;
				}
				m_nLength = m_pRunLenProb->acsSymTable[iSym].wSymbol ;
				m_pRunLenProb->IncreaseSymbol( iSym ) ;
			}
		}
	}
	m_pLastERISAProb = pProb ;
	//
	return	i ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISADecodeContext::Read( void * ptrBuf, size_t nBytes )
{
	return	DecodeERISACodeBytes( (SBYTE*) ptrBuf, nBytes ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ERISA-N（算術）符号コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SGLERISANDecodeContext, SGLERISADecodeContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLERISANDecodeContext::SGLERISANDecodeContext( SGLDecodeBitStream * pStream )
	: SGLERISADecodeContext( pStream )
{
	m_pProbERISA = NULL ;
	m_pNemesisBuf = NULL ;
	m_pNemesisLookup = NULL ;
	m_flagEOF = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLERISANDecodeContext::~SGLERISANDecodeContext( void )
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
	m_pNemesisLookup = NULL ;
}

// ERISA-N 符号の復号の準備をする
//////////////////////////////////////////////////////////////////////////////
void SGLERISANDecodeContext::PrepareToDecodeERISANCode( void )
{
	//
	// メモリを確保
	//
	int		i ;
	if ( m_pProbERISA == NULL )
	{
		size_t	nBytes =
			sizeof(ERISA_PROB_BASE)
				+ sizeof(ERISA_PROB_MODEL) * ERISA_PROB_SLOT_MAX ;
		nBytes = (nBytes + 0x0F) & (~0x0F) ;
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
	InitializeERISACode() ;
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
	m_nNemesisLeft = 0 ;
	//
	m_flagEOF = false ;
}

// ERISA-N 符号を復号する
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISANDecodeContext::DecodeERISANCodeBytes( SBYTE * ptrDst, size_t nCount )
{
	//
	//	指定された数のシンボルを復号し終えるまで繰り返す
	//
	size_t				nDecoded = 0 ;
	ERISA_PROB_BASE *	pBase = m_pProbERISA ;
	if ( m_flagEOF )
	{
		return	0 ;
	}
	while ( nDecoded < nCount )
	{
		//
		// スライド辞書から復号
		//////////////////////////////////////////////////////////////////////
		if ( m_nNemesisLeft > 0 )
		{
			size_t	nNemesisCount = m_nNemesisLeft ;
			if ( nNemesisCount > nCount - nDecoded )
			{
				nNemesisCount = nCount - nDecoded ;
			}
			BYTE	bytLastSymbol =
				m_pNemesisBuf[(m_nNemesisIndex - 1) & NEMESIS_BUF_MASK] ;
			//
			for ( size_t i = 0; i < nNemesisCount; i ++ )
			{
				BYTE	bytSymbol = bytLastSymbol ;
				if ( m_nNemesisNext >= 0 )
				{
					bytSymbol = m_pNemesisBuf[m_nNemesisNext ++] ;
					m_nNemesisNext &= NEMESIS_BUF_MASK ;
				}
				//
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
				//
				*(ptrDst ++) = bytSymbol ;
			}
			//
			m_nNemesisLeft -= (int) nNemesisCount ;
			nDecoded += nNemesisCount ;
			continue ;
		}
		//
		//	有効な統計モデルを取得する
		//////////////////////////////////////////////////////////////////////
		int	iDeg ;
		ERISA_PROB_MODEL *	pModel ;
		pModel = &(pBase->epmBaseModel) ;
		for ( iDeg = 0; iDeg < 4; iDeg ++ )
		{
			int	iLast =
				m_bytLastSymbol[(m_iLastSymbol + 0x03 - iDeg) & 0x03]
							>> ERISA_PROB_BASE::m_nShiftCount[iDeg] ;
			if ( pModel->acsSubModel[iLast].wSymbol < 0 )
			{
				break ;
			}
			ESLAssert( (DWORD) pModel->acsSubModel[iLast].wSymbol < pBase->dwWorkUsed ) ;
			pModel = pBase->ptrProbWork + pModel->acsSubModel[iLast].wSymbol ;
		}
		//
		//	算術符号を復号
		//////////////////////////////////////////////////////////////////////
		//
		// 算術符号を復号
		//
		int	iSym = DecodeERISACodeIndex( pModel ) ;
		if ( iSym < 0 )
		{
			return	nDecoded ;
		}
		//
		// 現在の統計モデルを更新
		//
		int	nSymbol = pModel->acsSymTable[iSym].wSymbol ;
		int	iSymIndex = pModel->IncreaseSymbol( iSym ) ;
		//
		bool	fNemesis = false ;
		if ( nSymbol == ERISA_ESC_CODE )
		{
			if ( pModel != &(pBase->epmBaseModel) )
			{
				iSym = DecodeERISACodeIndex( &(pBase->epmBaseModel) ) ;
				if ( iSym < 0 )
				{
					return	nDecoded ;
				}
				nSymbol = pBase->epmBaseModel.acsSymTable[iSym].wSymbol ;
				pBase->epmBaseModel.IncreaseSymbol( iSym ) ;
				if ( nSymbol != ERISA_ESC_CODE )
				{
					pModel->AddSymbol( (SWORD) nSymbol ) ;
					iSym = -1 ;
				}
				else
				{
					fNemesis = true ;
				}
			}
			else
			{
				fNemesis = true ;
			}
		}
		if ( fNemesis )
		{
			//
			// スライド辞書を使って復号
			//
			int	nLength, nPhraseIndex ;
			nPhraseIndex = DecodeERISACode( m_pPhraseIndexProb ) ;
			if ( nPhraseIndex == ERISA_ESC_CODE )
			{
				m_flagEOF = true ;
				return	nDecoded ;
			}
			if ( nPhraseIndex == 0 )
			{
				nLength = DecodeERISACode( m_pRunLenProb ) ;
			}
			else
			{
				nLength = DecodeERISACode( m_pPhraseLenProb ) ;
			}
			if ( nLength == ERISA_ESC_CODE )
			{
				return	nDecoded ;
			}
			BYTE	bytLastSymbol =
				m_pNemesisBuf[(m_nNemesisIndex - 1) & NEMESIS_BUF_MASK] ;
			ERISAN_PHRASE_LOOKUP *	ppl = &(m_pNemesisLookup[bytLastSymbol]) ;
			m_nNemesisLeft = nLength ;
			if ( nPhraseIndex == 0 )
			{
				m_nNemesisNext = -1 ;
			}
			else
			{
				m_nNemesisNext =
					ppl->index[(ppl->first - nPhraseIndex) & NEMESIS_INDEX_MASK] ;
				ESLAssert( m_pNemesisBuf[m_nNemesisNext] == bytLastSymbol ) ;
				m_nNemesisNext = (m_nNemesisNext + 1) & NEMESIS_BUF_MASK ;
			}
			continue ;
		}
		//
		// データを出力
		//////////////////////////////////////////////////////////////////////
		//
		// 復号されたシンボルを出力
		//
		BYTE	bytSymbol = (BYTE) nSymbol ;
		m_bytLastSymbol[m_iLastSymbol ++] = bytSymbol ;
		m_iLastSymbol &= 0x03 ;
		//
		ERISAN_PHRASE_LOOKUP *	ppl = &(m_pNemesisLookup[bytSymbol]) ;
		ppl->index[ppl->first] = m_nNemesisIndex ;
		ppl->first = (ppl->first + 1) & NEMESIS_INDEX_MASK ;
		m_pNemesisBuf[m_nNemesisIndex ++] = bytSymbol ;
		m_nNemesisIndex &= NEMESIS_BUF_MASK ;
		//
		*(ptrDst ++) = bytSymbol ;
		nDecoded ++ ;
		//
		// 統計モデルのツリーを拡張
		//
		if ( (pBase->dwWorkUsed < ERISA_PROB_SLOT_MAX) && (iDeg < 4) )
		{
			int	iSymbol =
				((BYTE) nSymbol) >> ERISA_PROB_BASE::m_nShiftCount[iDeg] ;
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
	return	nDecoded ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLERISANDecodeContext::Read( void * ptrBuf, size_t nBytes )
{
	return	DecodeERISANCodeBytes( (SBYTE*) ptrBuf, nBytes ) ;
}

