
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2009-2015 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_MD5_CONTEXT_H__)
#define	__SAKURA_ERISA_MD5_CONTEXT_H__

namespace	SakuraCL
{
	//////////////////////////////////////////////////////////////////////////
	// MD5 ダイジェスト計算コンテキスト (※リトルエンディアン依存実装)
	//////////////////////////////////////////////////////////////////////////

	class	MD5Context
	{
	protected:
		uint32_t	m_msgDigest[4] ;		// メッセージダイジェスト 128bit
		uint32_t *	m_pX ;
		uint8_t		m_padMessage[64] ;		// 未処理メッセージ 512bit
		size_t		m_padLength ;
		uint64_t	m_nTotalLength ;		// 全長

		static const uint32_t	m_MD5Table[64] ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( MD5Context )
		// 構築関数
		MD5Context( void ) ;

	public:
		// 初期化
		void Initialize( void ) ;
		// MD5 値更新
		void Stream( const uint8_t * pbytBuf, size_t nBytes ) ;
		// 確定
		void Flush( void ) ;
		// MD5 ダイジェスト取得
		void GetMD5Digest( uint32_t * digest ) const ;
		const wchar_t * GetMD5DigestHex( SSystem::SString& strHexDigest ) const ;

	public:
		// 512bit, 64byte 処理
		void Round_Calculate
			( const uint8_t * block,
				uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D ) ;
		// 基本操作関数
		inline static uint32_t ROTATE_LEFT( uint32_t x, uint32_t n )
			{
				return	(((x) << (n)) | ((x) >> (32-(n)))) ;
			}
		inline static uint32_t F( uint32_t X, uint32_t Y, uint32_t Z )
			{
				return	(X & Y) | (~X & Z) ;
			}
		inline static uint32_t G( uint32_t X, uint32_t Y, uint32_t Z )
			{
				return	(X & Z) | (Y & ~Z) ;
			}
		inline static uint32_t H( uint32_t X, uint32_t Y, uint32_t Z )
			{
				return	X ^ Y ^ Z ;
			}
		inline static uint32_t I( uint32_t X, uint32_t Y, uint32_t Z )
			{
				return	Y ^ (X | ~Z) ;
			}
		inline uint32_t Round
			( uint32_t a, uint32_t b, uint32_t FGHI, uint32_t k, uint32_t s, uint32_t i )
			{
				return	b + ROTATE_LEFT( a + FGHI + m_pX[k] + m_MD5Table[i], s ) ;
			}
		inline void Round1
			( uint32_t &a, uint32_t b, uint32_t c,
				uint32_t d, uint32_t k,  uint32_t s, uint32_t i )
			{
				a = Round( a, b, F(b,c,d), k, s, i ) ;
			}
		inline void Round2
			( uint32_t &a, uint32_t b, uint32_t c,
				uint32_t d, uint32_t k, uint32_t s, uint32_t i )
			{
				a = Round( a, b, G(b,c,d), k, s, i ) ;
			}
		inline void Round3
			( uint32_t &a, uint32_t b, uint32_t c,
				uint32_t d, uint32_t k,  uint32_t s, uint32_t i )
			{
				a = Round( a, b, H(b,c,d), k, s, i ) ;
			}
		inline void Round4
			( uint32_t &a, uint32_t b, uint32_t c,
				uint32_t d, uint32_t k,  uint32_t s, uint32_t i )
			{
				 a = Round( a, b, I(b,c,d), k, s, i ) ;
			}
	} ;

} ;

#endif
