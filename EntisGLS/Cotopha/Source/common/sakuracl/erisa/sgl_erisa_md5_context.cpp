
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuracl/erisa/sgl_erisa_md5_context.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace SakuraCL ;


//////////////////////////////////////////////////////////////////////////////
// MD5 ダイジェスト計算コンテキスト (※リトルエンディアン依存実装)
//////////////////////////////////////////////////////////////////////////////

const uint32_t SakuraCL::MD5Context::m_MD5Table[64] =
{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,  //0
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,  //4
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,  //8
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,  //12
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,  //16
    0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8,  //20
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,  //24
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,  //28
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,  //32
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,  //36
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085,  0x4881d05,  //40
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,  //44
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,  //48
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,  //52
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,  //56
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391   //60
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraCL::MD5Context )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
MD5Context::MD5Context( void )
{
	m_msgDigest[0] = 0x67452301 ;
	m_msgDigest[1] = 0xefcdab89 ;
	m_msgDigest[2] = 0x98badcfe ;
	m_msgDigest[3] = 0x10325476 ;
	m_pX = NULL ;
	m_padLength = 0 ;
	m_nTotalLength = 0 ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void MD5Context::Initialize( void )
{
	m_msgDigest[0] = 0x67452301 ;
	m_msgDigest[1] = 0xefcdab89 ;
	m_msgDigest[2] = 0x98badcfe ;
	m_msgDigest[3] = 0x10325476 ;
	m_pX = NULL ;
	m_padLength = 0 ;
	m_nTotalLength = 0 ;
}

// MD5 値更新
//////////////////////////////////////////////////////////////////////////////
void MD5Context::Stream( const uint8_t * pbytBuf, size_t nBytes )
{
	uint32_t &	A = m_msgDigest[0] ;	// RFCに則ったメッセージダイジェスト
	uint32_t &	B = m_msgDigest[1] ;
	uint32_t &	C = m_msgDigest[2] ;
	uint32_t &	D = m_msgDigest[3] ;
	//
	while ( nBytes > 0 )
	{
		// 512bit, 64byte 処理
		size_t	nLeftBytes = 64 - m_padLength ;
		if ( nLeftBytes <= nBytes )
		{
			eslMoveMemory( &m_padMessage[m_padLength], pbytBuf, nLeftBytes ) ;
			nBytes -= nLeftBytes ;
			pbytBuf += nLeftBytes ;
			m_nTotalLength += nLeftBytes ;
			//
			Round_Calculate( &m_padMessage[0], A, B, C, D ) ;
			//
			m_padLength = 0 ;
		}
		else
		{
			eslMoveMemory( &m_padMessage[m_padLength], pbytBuf, nBytes ) ;
			m_padLength += nBytes ;
			m_nTotalLength += nBytes ;
			break ;
		}
	}
}

// 確定
//////////////////////////////////////////////////////////////////////////////
void MD5Context::Flush( void )
{
	uint32_t &	A = m_msgDigest[0] ;	// RFCに則ったメッセージダイジェスト
	uint32_t &	B = m_msgDigest[1] ;
	uint32_t &	C = m_msgDigest[2] ;
	uint32_t &	D = m_msgDigest[3] ;
	//
	// 終端ブロック
	//
	ESLAssert( m_padLength < 64 ) ;
	eslFillMemory( &m_padMessage[m_padLength], 0, 64 - m_padLength ) ;
	m_padMessage[m_padLength] |= 0x80 ;
	//
	if ( 56 <= m_padLength )
	{
		Round_Calculate( &m_padMessage[0], A, B, C, D ) ;
		eslFillMemory( &m_padMessage[0], 0, 56 ) ;
	}
	//
	// 長さ情報の追加
	//
	uint64_t	nTotalBitCount = m_nTotalLength * 8 ;
	eslMoveMemory( &m_padMessage[56], &nTotalBitCount, 8 ) ;
	//
	Round_Calculate( &m_padMessage[0], A, B, C, D ) ;
}

// MD5 ダイジェスト取得
//////////////////////////////////////////////////////////////////////////////
void MD5Context::GetMD5Digest( uint32_t * digest ) const
{
	digest[0] = m_msgDigest[0] ;
	digest[1] = m_msgDigest[1] ;
	digest[2] = m_msgDigest[2] ;
	digest[3] = m_msgDigest[3] ;
}

const wchar_t * MD5Context::GetMD5DigestHex( SSystem::SString& strHexDigest ) const
{
	const uint8_t *	pbytDigest = (uint8_t*) &m_msgDigest[0] ;
	uint16_t *	pwHexDigest = strHexDigest.LockBuffer( 32 ) ;
	for ( size_t i = 0, j = 0; i < 16; i ++, j += 2 )
	{
		uint8_t	bytHigh = (pbytDigest[i] >> 4) & 0x0F ;
		uint8_t	bytLow = pbytDigest[i] & 0x0F ;
		if ( bytHigh < 10 )
		{
			pwHexDigest[j] = L'0' + bytHigh ;
		}
		else
		{
			pwHexDigest[j] = (L'A' - 10) + bytHigh ;
		}
		if ( bytLow < 10 )
		{
			pwHexDigest[j + 1] = L'0' + bytLow ;
		}
		else
		{
			pwHexDigest[j + 1] = (L'A' - 10) + bytLow ;
		}
	}
	strHexDigest.UnlockBuffer( 32 ) ;
	return	strHexDigest ;
}

// 512bit, 64byte 処理
//////////////////////////////////////////////////////////////////////////////
void MD5Context::Round_Calculate
	( const uint8_t * block,
		uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D )
{
	//
	// 計算用バッファ
	//
	uint32_t	buf[16] ;  // 512bit 64byte
	m_pX = &buf[0] ;
	//
	// Copy block(padding_message) i into X
	//
	for ( int j = 0, k = 0; j < 64; j += 4, k ++ )
	{
		buf[k] = ((uint32_t) block[j])
				| (((uint32_t) block[j+1]) << 8)
				| (((uint32_t) block[j+2]) << 16)
				| (((uint32_t) block[j+3]) << 24) ;
	}
	//
	// Save A as AA, B as BB, C as CC, and D as DD (A,B,C,Dの保存)
	//
	uint32_t	AA = A, BB = B, CC = C, DD = D ;
	//
	//Round 1
	//
	Round1(A,B,C,D,  0,  7,  0);  Round1(D,A,B,C,  1, 12,  1);  Round1(C,D,A,B,  2, 17,  2);  Round1(B,C,D,A,  3, 22,  3);
	Round1(A,B,C,D,  4,  7,  4);  Round1(D,A,B,C,  5, 12,  5);  Round1(C,D,A,B,  6, 17,  6);  Round1(B,C,D,A,  7, 22,  7);
	Round1(A,B,C,D,  8,  7,  8);  Round1(D,A,B,C,  9, 12,  9);  Round1(C,D,A,B, 10, 17, 10);  Round1(B,C,D,A, 11, 22, 11);
	Round1(A,B,C,D, 12,  7, 12);  Round1(D,A,B,C, 13, 12, 13);  Round1(C,D,A,B, 14, 17, 14);  Round1(B,C,D,A, 15, 22, 15);
	//
	//Round 2
	//
	Round2(A,B,C,D,  1,  5, 16);  Round2(D,A,B,C,  6,  9, 17);  Round2(C,D,A,B, 11, 14, 18);  Round2(B,C,D,A,  0, 20, 19);
	Round2(A,B,C,D,  5,  5, 20);  Round2(D,A,B,C, 10,  9, 21);  Round2(C,D,A,B, 15, 14, 22);  Round2(B,C,D,A,  4, 20, 23);
	Round2(A,B,C,D,  9,  5, 24);  Round2(D,A,B,C, 14,  9, 25);  Round2(C,D,A,B,  3, 14, 26);  Round2(B,C,D,A,  8, 20, 27);
	Round2(A,B,C,D, 13,  5, 28);  Round2(D,A,B,C,  2,  9, 29);  Round2(C,D,A,B,  7, 14, 30);  Round2(B,C,D,A, 12, 20, 31);
	//
	//Round 3
	//
	Round3(A,B,C,D,  5,  4, 32);  Round3(D,A,B,C,  8, 11, 33);  Round3(C,D,A,B, 11, 16, 34);  Round3(B,C,D,A, 14, 23, 35);
	Round3(A,B,C,D,  1,  4, 36);  Round3(D,A,B,C,  4, 11, 37);  Round3(C,D,A,B,  7, 16, 38);  Round3(B,C,D,A, 10, 23, 39);
	Round3(A,B,C,D, 13,  4, 40);  Round3(D,A,B,C,  0, 11, 41);  Round3(C,D,A,B,  3, 16, 42);  Round3(B,C,D,A,  6, 23, 43);
	Round3(A,B,C,D,  9,  4, 44);  Round3(D,A,B,C, 12, 11, 45);  Round3(C,D,A,B, 15, 16, 46);  Round3(B,C,D,A,  2, 23, 47);
	//
	//Round 4
	//
	Round4(A,B,C,D,  0,  6, 48);  Round4(D,A,B,C,  7, 10, 49);  Round4(C,D,A,B, 14, 15, 50);  Round4(B,C,D,A,  5, 21, 51);
	Round4(A,B,C,D, 12,  6, 52);  Round4(D,A,B,C,  3, 10, 53);  Round4(C,D,A,B, 10, 15, 54);  Round4(B,C,D,A,  1, 21, 55);
	Round4(A,B,C,D,  8,  6, 56);  Round4(D,A,B,C, 15, 10, 57);  Round4(C,D,A,B,  6, 15, 58);  Round4(B,C,D,A, 13, 21, 59);
	Round4(A,B,C,D,  4,  6, 60);  Round4(D,A,B,C, 11, 10, 61);  Round4(C,D,A,B,  2, 15, 62);  Round4(B,C,D,A,  9, 21, 63);
	//
	// Then perform the following additions.
	//
	A = A + AA ;
	B = B + BB ;
	C = C + CC ;
	D = D + DD ;
	//
	//機密情報のクリア
	//
	memset( buf, 0, 16 * sizeof(uint32_t) ) ;
	m_pX = NULL ;
}

