
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_CRC32_CONTEXT_H__)
#define	__SAKURA_ERISA_CRC32_CONTEXT_H__

namespace	SakuraCL
{
	//////////////////////////////////////////////////////////////////////////
	// CRC32 計算コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	CRC32Context
	{
	protected:
		uint32_t	m_crc ;
	public:
		static const uint32_t	m_CRC32Table[256] ;
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( CRC32Context )
		// 構築関数
		CRC32Context( void ) : m_crc(0xFFFFFFFF) {}
		CRC32Context( const CRC32Context& crc32 ) : m_crc(crc32.m_crc) {}
		// 初期化
		void Initialize( void )
		{
			m_crc = 0xFFFFFFFF ;
		}
		// CRC32 値取得
		uint32_t GetCRC32( void ) const
		{
			return	~m_crc ;
		}
		// CRC32 値更新
		void Stream( const uint8_t * pbytBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// XOR 計算コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	XOR32Context
	{
	protected:
		uint32_t	m_xor ;
		size_t		m_index ;
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( XOR32Context )
		// 構築関数
		XOR32Context( void ) : m_xor(0), m_index(0) {}
		XOR32Context( const XOR32Context& xor32 )
				: m_xor(xor32.m_xor), m_index(xor32.m_index) {}
		// 初期化
		void Initialize( void )
		{
			m_xor = 0 ;
			m_index = 0 ;
		}
		// XOR32 値取得
		uint32_t GetXOR32( void ) const
		{
			return	m_xor ;
		}
		// XOR32 値更新
		void Stream( const uint8_t * pbytBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// CRC32 計算入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	CRC32InputStream
				: public SSystem::SInputStream,
					public CRC32Context, public XOR32Context
	{
	protected:
		SSystem::SInputStream *	m_pStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( CRC32InputStream, SInputStream, CRC32Context, XOR32Context )
		// 構築関数
		CRC32InputStream( SSystem::SInputStream * pStream )
									: m_pStream( pStream ) {}
		// 消滅関数
		virtual ~CRC32InputStream( void ) ;

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// CRC32 計算出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	CRC32OutputStream
				: public SSystem::SOutputStream,
					public CRC32Context, public XOR32Context
	{
	protected:
		SSystem::SOutputStream *	m_pStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( CRC32OutputStream, SOutputStream, CRC32Context, XOR32Context )
		// 構築関数
		CRC32OutputStream( SSystem::SOutputStream * pStream )
									: m_pStream( pStream ) {}
		// 消滅関数
		virtual ~CRC32OutputStream( void ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	} ;


} ;

#endif
