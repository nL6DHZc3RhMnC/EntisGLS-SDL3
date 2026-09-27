
#if	!defined(__SAKURA2_BIT_ARRAY_H__)
#define	__SAKURA2_BIT_ARRAY_H__

//////////////////////////////////////////////////////////////////////////////
// ただのビット配列
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	class	SBitArray 
	{
	protected:
		SArray<uint8_t>	m_bufArray ;
		size_t			m_nLength ;

	public:
		// 構築関数
		SBitArray( void )
		{
			m_nLength = 0 ;
		}
		SBitArray( const SBitArray& src )
			: m_bufArray( src.m_bufArray ), m_nLength( src.m_nLength )
		{
		}
		// 内部バッファを解放する
		void FreeArray( void )
		{
			m_bufArray.FreeArray() ;
			m_nLength = 0 ;
		}
		// 配列の長さを取得
		size_t GetLength( void ) const
		{
			return m_nLength ;
		}
		// 配列長設定
		void SetLength( size_t nLength )
		{
			m_bufArray.SetLength( (nLength + 0x07) >> 3 ) ;
			m_nLength = nLength ;
		}
		// 複製
		const SBitArray& operator = ( const SBitArray& src )
		{
			m_bufArray = src.m_bufArray ;
			m_nLength = src.m_nLength ;
			return	*this ;
		}
		// 内部バッファ
		const uint8_t * GetConstArray( void ) const
		{
			return	m_bufArray.GetConstArray() ;
		}
		uint8_t * GetArray( void ) const
		{
			return	m_bufArray.GetArray() ;
		}
		void FinidhArray( void )
		{
			m_bufArray.FinishArray() ;
		}
		// 要素取得
		bool GetAt( size_t nIndex ) const
		{
			if ( nIndex < m_nLength )
			{
				if ( m_bufArray.At( nIndex >> 3 )
								& (0x80 >> (nIndex & 0x07)) )
				{
					return	true ;
				}
			}
			return	false ;
		}
		// 要素設定
		void SetAt( size_t nIndex, bool fBit ) const
		{
			if ( nIndex < m_nLength )
			{
				uint8_t&	nBits = m_bufArray.At( nIndex >> 3 ) ;
				uint8_t		nMask = (uint8_t) (0x80 >> (nIndex & 0x07)) ;
				if ( fBit )
				{
					nBits |= nMask ;
				}
				else
				{
					nBits &= ~nMask ;
				}
			}
		}
		// フィル
		void Fill( bool fBit )
		{
			size_t	nBytesLength = m_bufArray.GetLength() ;
			if ( nBytesLength > 0 )
			{
				eslFillMemory
					( m_bufArray.GetArray(),
						(fBit ? 0xFF : 0), nBytesLength ) ;
				m_bufArray.FinishArray() ;
			}
		}
		void Clear( bool fBit = false )
		{
			Fill( fBit ) ;
		}
		// 範囲
		void FillRange( size_t nIndex, size_t nCount, bool fBit ) ;
		// 全て 0 か？
		bool IsEmpty( void ) const ;
		// 全て 1 か？
		bool IsFull( void ) const ;
		// 1 の個数
		size_t BitCount( void ) const ;
		// 交差テスト
		bool Test( size_t nIndex, const SBitArray& bitTest ) const ;
		// 論理積
		void And( size_t nIndex, const SBitArray& bitSrc ) ;
		// 否定論理積 (*this &= ~bitSrc)
		void NotAnd( size_t nIndex, const SBitArray& bitSrc ) ;
		// 論理和
		void Or( size_t nIndex, const SBitArray& bitSrc ) ;
		// 排他的論理和
		void Xor( size_t nIndex, const SBitArray& bitSrc ) ;
		// 論理否定
		void Not( void ) ;
		// 演算子
		const SBitArray& operator &= ( const SBitArray& bitSrc )
		{
			And( 0, bitSrc ) ;
			return	*this ;
		}
		const SBitArray& operator |= ( const SBitArray& bitSrc )
		{
			Or( 0, bitSrc ) ;
			return	*this ;
		}
		const SBitArray& operator ^= ( const SBitArray& bitSrc )
		{
			Xor( 0, bitSrc ) ;
			return	*this ;
		}
		SBitArray operator & ( const SBitArray& bitSrc ) const
		{
			SBitArray	baTemp = *this ;
			baTemp.And( 0, bitSrc ) ;
			return	baTemp ;
		}
		SBitArray operator | ( const SBitArray& bitSrc ) const
		{
			SBitArray	baTemp = *this ;
			baTemp.Or( 0, bitSrc ) ;
			return	baTemp ;
		}
		SBitArray operator ^ ( const SBitArray& bitSrc ) const
		{
			SBitArray	baTemp = *this ;
			baTemp.Xor( 0, bitSrc ) ;
			return	baTemp ;
		}
		SBitArray operator ~ ( void ) const
		{
			SBitArray	baTemp = *this ;
			baTemp.Not() ;
			return	baTemp ;
		}

	} ;

}

#endif
