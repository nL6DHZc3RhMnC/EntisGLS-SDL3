

#include <sakura/sakura.h>
#include <sakura/ssys_bit_array.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ただのビット配列
//////////////////////////////////////////////////////////////////////////////

// 範囲
//////////////////////////////////////////////////////////////////////////////
void SBitArray::FillRange( size_t nIndex, size_t nCount, bool fBit )
{
	if ( nIndex > m_nLength )
	{
		return ;
	}
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	if ( nCount == 0 )
	{
		return ;
	}
	uint8_t *	pArray = m_bufArray.GetArray() ;
	size_t		nEnd = nIndex + nCount - 1 ;
	size_t		iLeft = (nIndex >> 3) ;
	size_t		iRight = (nEnd >> 3) ;
	uint8_t		bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t		bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		if ( fBit )
		{
			pArray[iLeft] |= (bitLeft & bitRight) ;
		}
		else
		{
			pArray[iLeft] &= ~(bitLeft & bitRight) ;
		}
	}
	else if ( fBit )
	{
		size_t	nCount = iRight - iLeft - 1 ;
		pArray += iLeft ;
		*(pArray ++) |= bitLeft ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			*(pArray ++) = 0xFF ;
		}
		*(pArray ++) |= bitRight ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		pArray += iLeft ;
		*(pArray ++) &= ~bitLeft ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			*(pArray ++) = 0 ;
		}
		*(pArray ++) &= ~bitRight ;
	}
	m_bufArray.FinishArray() ;
}

// 全て 0 か？
//////////////////////////////////////////////////////////////////////////////
bool SBitArray::IsEmpty( void ) const
{
	size_t			nCount = (m_nLength >> 3) ;
	uint8_t			bitOdd = (uint8_t) ~(0xFF >> (m_nLength & 0x07)) ;
	const uint8_t *	pArray = m_bufArray.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pArray[i] != 0 )
		{
			return	false ;
		}
	}
	return	(bitOdd == 0) || ((pArray[nCount] & bitOdd) == 0) ;
}

// 全て 1 か？
//////////////////////////////////////////////////////////////////////////////
bool SBitArray::IsFull( void ) const
{
	size_t			nCount = (m_nLength >> 3) ;
	uint8_t			bitOdd = (uint8_t) ~(0xFF >> (m_nLength & 0x07)) ;
	const uint8_t *	pArray = m_bufArray.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pArray[i] != 0xFF )
		{
			return	false ;
		}
	}
	return	(bitOdd == 0) || ((pArray[nCount] & bitOdd) == bitOdd) ;
}

// 1 の個数
//////////////////////////////////////////////////////////////////////////////
size_t SBitArray::BitCount( void ) const
{
	size_t			nCount = (m_nLength >> 3) ;
	uint8_t			bitOdd = (uint8_t) ~(0xFF >> (m_nLength & 0x07)) ;
	const uint8_t *	pArray = m_bufArray.GetConstArray() ;
	size_t		nBitCount = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint8_t	bits = pArray[i] ;
		bits = ((bits >> 1) & 0x55) + (bits & 0x55) ;
		bits = ((bits >> 2) & 0x33) + (bits & 0x33) ;
		bits = ((bits >> 4) & 0x0F) + (bits & 0x0F) ;
		nBitCount += bits ;
	}
	if ( bitOdd != 0 )
	{
		uint8_t	bits = pArray[nCount] & bitOdd ;
		bits = ((bits >> 1) & 0x55) + (bits & 0x55) ;
		bits = ((bits >> 2) & 0x33) + (bits & 0x33) ;
		bits = ((bits >> 4) & 0x0F) + (bits & 0x0F) ;
		nBitCount += bits ;
	}
	return	nBitCount ;
}

// 交差テスト
//////////////////////////////////////////////////////////////////////////////
bool SBitArray::Test( size_t nIndex, const SBitArray& bitTest ) const
{
	if ( nIndex >= m_nLength )
	{
		return	false ;
	}
	size_t	nCount = bitTest.m_nLength ;
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	const uint8_t *	pDst = m_bufArray.GetConstArray() ;
	const uint8_t *	pSrc = bitTest.m_bufArray.GetConstArray() ;
	size_t			nEnd = nIndex + nCount - 1 ;
	size_t			iLeft = (nIndex >> 3) ;
	size_t			iRight = (nEnd >> 3) ;
	size_t			nShifter = (nIndex & 0x07) ;
	size_t			nCShifter = 8 - nShifter ;
	uint8_t			bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t			bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		return	(pDst[iLeft] & (pSrc[0] >> nShifter)
								& (bitLeft & bitRight)) != 0 ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		size_t	iSrc = 0 ;
		uint8_t	bitCarry = 0 ;
		uint8_t	bitTemp ;
		//
		pDst += iLeft ;
		//
		bitTemp = pSrc[iSrc ++] ;
		bitCarry = bitTemp << nCShifter ;
		if ( *(pDst ++) & (bitTemp >> nShifter) )
		{
			return	true ;
		}
		for ( size_t i = 0; i < nCount; i ++ )
		{
			bitTemp = pSrc[iSrc ++] ;
			if ( *(pDst ++) & (bitCarry | (bitTemp >> nShifter)) )
			{
				return	true ;
			}
			bitCarry = bitTemp << nCShifter ;
		}
		if ( *(pDst ++) & bitCarry )
		{
			return	true ;
		}
	}
	return	false ;
}

// 論理積
//////////////////////////////////////////////////////////////////////////////
void SBitArray::And( size_t nIndex, const SBitArray& bitSrc )
{
	if ( nIndex >= m_nLength )
	{
		return ;
	}
	size_t	nCount = bitSrc.m_nLength ;
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	uint8_t *		pDst = m_bufArray.GetArray() ;
	const uint8_t *	pSrc = bitSrc.m_bufArray.GetConstArray() ;
	size_t			nEnd = nIndex + nCount - 1 ;
	size_t			iLeft = (nIndex >> 3) ;
	size_t			iRight = (nEnd >> 3) ;
	size_t			nShifter = (nIndex & 0x07) ;
	size_t			nCShifter = 8 - nShifter ;
	uint8_t			bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t			bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		uint8_t	bitMask = (bitLeft & bitRight) ;
		pDst[iLeft] &= ((pSrc[0] >> nShifter) & bitMask) | ~bitMask ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		size_t	iSrc = 0 ;
		uint8_t	bitCarry = 0 ;
		uint8_t	bitTemp ;
		//
		pDst += iLeft ;
		//
		bitTemp = pSrc[iSrc ++] ;
		*(pDst ++) &= (bitTemp >> nShifter) | ~bitLeft ;
		bitCarry = bitTemp << nCShifter ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			bitTemp = pSrc[iSrc ++] ;
			*(pDst ++) &= (bitCarry | (bitTemp >> nShifter)) ;
			bitCarry = bitTemp << nCShifter ;
		}
		*(pDst ++) &= bitCarry | ~bitRight ;
	}
	m_bufArray.FinishArray() ;
}

// 否定論理積 (*this &= ~bitSrc)
//////////////////////////////////////////////////////////////////////////////
void SBitArray::NotAnd( size_t nIndex, const SBitArray& bitSrc )
{
	if ( nIndex >= m_nLength )
	{
		return ;
	}
	size_t	nCount = bitSrc.m_nLength ;
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	uint8_t *		pDst = m_bufArray.GetArray() ;
	const uint8_t *	pSrc = bitSrc.m_bufArray.GetConstArray() ;
	size_t			nEnd = nIndex + nCount - 1 ;
	size_t			iLeft = (nIndex >> 3) ;
	size_t			iRight = (nEnd >> 3) ;
	size_t			nShifter = (nIndex & 0x07) ;
	size_t			nCShifter = 8 - nShifter ;
	uint8_t			bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t			bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		uint8_t	bitMask = (bitLeft & bitRight) ;
		pDst[iLeft] &= ((~pSrc[0] >> nShifter) & bitMask) | ~bitMask ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		size_t	iSrc = 0 ;
		uint8_t	bitCarry = 0 ;
		uint8_t	bitTemp ;
		//
		pDst += iLeft ;
		//
		bitTemp = pSrc[iSrc ++] ;
		*(pDst ++) &= (~bitTemp >> nShifter) | ~bitLeft ;
		bitCarry = bitTemp << nCShifter ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			bitTemp = pSrc[iSrc ++] ;
			*(pDst ++) &= ~(bitCarry | (bitTemp >> nShifter)) ;
			bitCarry = bitTemp << nCShifter ;
		}
		*(pDst ++) &= ~bitCarry | ~bitRight ;
	}
	m_bufArray.FinishArray() ;
}

// 論理和
//////////////////////////////////////////////////////////////////////////////
void SBitArray::Or( size_t nIndex, const SBitArray& bitSrc )
{
	if ( nIndex >= m_nLength )
	{
		return ;
	}
	size_t	nCount = bitSrc.m_nLength ;
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	uint8_t *		pDst = m_bufArray.GetArray() ;
	const uint8_t *	pSrc = bitSrc.m_bufArray.GetConstArray() ;
	size_t			nEnd = nIndex + nCount - 1 ;
	size_t			iLeft = (nIndex >> 3) ;
	size_t			iRight = (nEnd >> 3) ;
	size_t			nShifter = (nIndex & 0x07) ;
	size_t			nCShifter = 8 - nShifter ;
	uint8_t			bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t			bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		uint8_t	bitMask = (bitLeft & bitRight) ;
		pDst[iLeft] |= (pSrc[0] >> nShifter) & bitMask ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		size_t	iSrc = 0 ;
		uint8_t	bitCarry = 0 ;
		uint8_t	bitTemp ;
		//
		pDst += iLeft ;
		//
		bitTemp = pSrc[iSrc ++] ;
		*(pDst ++) |= (bitTemp >> nShifter) ;
		bitCarry = bitTemp << nCShifter ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			bitTemp = pSrc[iSrc ++] ;
			*(pDst ++) |= (bitCarry | (bitTemp >> nShifter)) ;
			bitCarry = bitTemp << nCShifter ;
		}
		*(pDst ++) |= bitCarry ;
	}
	m_bufArray.FinishArray() ;
}

// 排他的論理和
//////////////////////////////////////////////////////////////////////////////
void SBitArray::Xor( size_t nIndex, const SBitArray& bitSrc )
{
	if ( nIndex >= m_nLength )
	{
		return ;
	}
	size_t	nCount = bitSrc.m_nLength ;
	if ( nIndex + nCount > m_nLength )
	{
		nCount = m_nLength - nIndex ;
	}
	uint8_t *		pDst = m_bufArray.GetArray() ;
	const uint8_t *	pSrc = bitSrc.m_bufArray.GetConstArray() ;
	size_t			nEnd = nIndex + nCount - 1 ;
	size_t			iLeft = (nIndex >> 3) ;
	size_t			iRight = (nEnd >> 3) ;
	size_t			nShifter = (nIndex & 0x07) ;
	size_t			nCShifter = 8 - nShifter ;
	uint8_t			bitLeft = (uint8_t) (0xFF >> (nIndex & 0x07)) ;
	uint8_t			bitRight = (uint8_t) (0xFF << (7 - (nEnd & 0x07))) ;
	if ( iLeft == iRight )
	{
		uint8_t	bitMask = (bitLeft & bitRight) ;
		pDst[iLeft] ^= (pSrc[0] >> nShifter) & bitMask ;
	}
	else
	{
		size_t	nCount = iRight - iLeft - 1 ;
		size_t	iSrc = 0 ;
		uint8_t	bitCarry = 0 ;
		uint8_t	bitTemp ;
		//
		pDst += iLeft ;
		//
		bitTemp = pSrc[iSrc ++] ;
		*(pDst ++) ^= (bitTemp >> nShifter) ;
		bitCarry = bitTemp << nCShifter ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			bitTemp = pSrc[iSrc ++] ;
			*(pDst ++) ^= (bitCarry | (bitTemp >> nShifter)) ;
			bitCarry = bitTemp << nCShifter ;
		}
		*(pDst ++) ^= bitCarry ;
	}
	m_bufArray.FinishArray() ;
}

// 論理否定
//////////////////////////////////////////////////////////////////////////////
void SBitArray::Not( void )
{
	size_t		nCount = (m_nLength >> 3) ;
	uint8_t		bitOdd = (uint8_t) ~(0xFF >> (m_nLength & 0x07)) ;
	uint8_t *	pArray = m_bufArray.GetArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pArray[i] = ~pArray[i] ;
	}
	if ( bitOdd != 0 )
	{
		pArray[nCount] ^= bitOdd ;
	}
	m_bufArray.FinishArray() ;
}
