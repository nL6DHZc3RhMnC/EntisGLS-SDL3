
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_DECODE_CONTEXT_H__)
#define	__SAKURA_ERISA_DECODE_CONTEXT_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// 復号ビットストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLDecodeBitStream	: public ESLObject
	{
	public:
		// ビットストリームバッファ
		size_t	m_nIntBufCount ;	// 中間入力バッファに蓄積されているビット数
		DWORD	m_dwIntBuffer ;		// 中間入力バッファ
		size_t	m_nBufferingSize ;	// バッファリングするバイト数
		size_t	m_nBufCount ;		// バッファの残りバイト数
		PBYTE	m_ptrBuffer ;		// 入力バッファの先頭へのポインタ
		PBYTE	m_ptrNextBuf ;		// 次に読み込むべき入力バッファへのポインタ

		// 入力ストリーム
		SSystem::SInputStream *	m_pStream ;

	public:
		// 構築関数
		SGLDecodeBitStream( size_t nBufferingSize )
		{
			m_nIntBufCount = 0 ;
			m_nBufferingSize = (nBufferingSize + 0x03) & ~0x03 ;
			m_nBufCount = 0 ;
			m_ptrBuffer = (PBYTE) esl_malloc( m_nBufferingSize ) ;
			m_pStream = NULL ;
		}
		// 消滅関数
		virtual ~SGLDecodeBitStream( void ) ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDecodeBitStream, ESLObject )

	public:
		// 入力ストリームを関連付ける
		void AttachInputStream( SSystem::SInputStream * pStream )
		{
			m_pStream = pStream ;
			m_nIntBufCount = 0 ;
			m_nBufCount = 0 ;
		}
		// バッファが空の時、次のデータを読み込む
		SSystem::SError PrefetchBuffer( void ) ;
		// 1ビット取得する（ 0 又は－1を返す ）
		int GetABit( void )
		{
			if ( (m_nIntBufCount == 0) && PrefetchBuffer() )
			{
				return	1 ;
			}
			DWORD	dwIntBuffer = m_dwIntBuffer ;
			int		nValue = - (int) (dwIntBuffer >> 31) ;
			-- m_nIntBufCount ;
			m_dwIntBuffer = dwIntBuffer << 1 ;
			return	nValue ;
		}
		// nビット取得する
		UINT GetNBits( size_t n )
		{
			UINT	nCode = 0 ;
			while ( n != 0 )
			{
				if ( (m_nIntBufCount == 0) && PrefetchBuffer() )
				{
					break ;
				}
				size_t	nCopyBits = n ;
				if ( nCopyBits > m_nIntBufCount )
				{
					nCopyBits = m_nIntBufCount ;
				}
				DWORD	dwIntBuffer = m_dwIntBuffer ;
				if ( nCopyBits < 32 )
				{
					nCode = (nCode << nCopyBits)
								| (dwIntBuffer >> (32 - nCopyBits)) ;
					n -= nCopyBits ;
					m_nIntBufCount -= nCopyBits ;
					m_dwIntBuffer = dwIntBuffer << nCopyBits ;
				}
				else
				{
					nCode = dwIntBuffer ;
					n -= nCopyBits ;
					m_nIntBufCount -= nCopyBits ;
					m_dwIntBuffer = 0 ;
				}
			}
			return	nCode ;
		}
		// バッファをフラッシュする
		void FlushBuffer( void )
		{
			m_nIntBufCount = 0 ;
			m_nBufCount = 0 ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 復号コンテキスト基底クラス
	//////////////////////////////////////////////////////////////////////////

	class	SGLAbstractDecodeContext	: public SSystem::SInputStream
	{
	protected:
		SGLDecodeBitStream *	m_pStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAbstractDecodeContext, SInputStream )
		// 構築関数
		SGLAbstractDecodeContext( SGLDecodeBitStream * pStream )
										: m_pStream( pStream ) {}

	public:
		// 入力ビットストリームを関連付ける
		void AttachBitStream( SGLDecodeBitStream * pStream )
		{
			m_pStream = pStream ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ランレングスガンマ符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLGammaDecodeContext	: public SGLAbstractDecodeContext
	{
	protected:
		bool					m_flgZero ;		// ゼロフラグ
		size_t					m_nLength ;		// ランレングス

		static const DWORD		m_nGammaCodeLookup[0x200] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGammaDecodeContext, SGLAbstractDecodeContext )
		// 構築関数
		SGLGammaDecodeContext( SGLDecodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLGammaDecodeContext( void ) ;

	public:
		// 入力ビットストリームを関連付ける
		void AttachBitStream( SGLDecodeBitStream * pStream )
		{
			m_pStream = pStream ;
		}
		// ランレングスガンマ符号のゼロフラグを読み込む
		void InitGammaContext( void ) ;
		// ガンマコードを取得する
		int GetGammaCode( void ) ;
		// ランレングスガンマ符号を復号する
		size_t DecodeGammaCodeBytes( SBYTE * ptrDst, size_t nCount ) ;
		size_t DecodeGammaCodeWords( SWORD * ptrDst, size_t nCount ) ;

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERINA（ハフマン）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLHuffmanDecodeContext	: public SGLGammaDecodeContext
	{
	protected:
		DWORD							m_dwERINAFlags ;
		ERISA::ERINA_HUFFMAN_TREE *		m_pLastHuffmanTree ;
		ERISA::ERINA_HUFFMAN_TREE **	m_ppHuffmanTree ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLHuffmanDecodeContext, SGLGammaDecodeContext )
		// 構築関数
		SGLHuffmanDecodeContext( SGLDecodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLHuffmanDecodeContext( void ) ;

	public:
		// 圧縮方式
		enum	ERINAEncodingFlag
		{
			efERINAOrder0	= 0x0000,
			efERINAOrder1	= 0x0001
		} ;
		// ERINA 符号の復号の準備をする
		void PrepareToDecodeERINACode( DWORD dwFlags = efERINAOrder1 ) ;
		// ハフマン符号を取得する
		int GetHuffmanCode( ERISA::ERINA_HUFFMAN_TREE * tree ) ;
		// 長さのハフマン符号を取得する
		int GetLengthHuffman( ERISA::ERINA_HUFFMAN_TREE * tree ) ;
		// ERINA 符号を復号する
		size_t DecodeERINACodeBytes( SBYTE * ptrDst, size_t nCount ) ;

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERISA（算術）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLERISADecodeContext	: public SGLAbstractDecodeContext
	{
	protected:
		DWORD		m_dwCodeRegister ;		// コードレジスタ（16 bit）
		DWORD		m_dwAugendRegister ;	// オージェンドレジスタ（16 bit）
		size_t		m_nLength ;				// ランレングス
		size_t		m_nPostBitCount ;		// 終端ビットバッファカウンタ

		ERISA::ERISA_PROB_MODEL **	m_ppTableERISA ;
		ERISA::ERISA_PROB_MODEL *	m_pPhraseLenProb ;
		ERISA::ERISA_PROB_MODEL *	m_pPhraseIndexProb ;
		ERISA::ERISA_PROB_MODEL *	m_pRunLenProb ;
		ERISA::ERISA_PROB_MODEL *	m_pLastERISAProb ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERISADecodeContext, SGLAbstractDecodeContext )
		// 構築関数
		SGLERISADecodeContext( SGLDecodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLERISADecodeContext( void ) ;

	public:
		// 入力ビットストリームを関連付ける
		void AttachBitStream( SGLDecodeBitStream * pStream )
		{
			m_pStream = pStream ;
		}

	public:
		// ERISA 符号の復号の準備をする
		void PrepareToDecodeERISACode( void ) ;
		// 算術符号の復号の初期化を行う
		void InitializeERISACode( void ) ;
		// 指定の統計モデルを使って1つの算術符号を復号
		int DecodeERISACode( ERISA_PROB_MODEL * pModel ) ;
		int DecodeERISACodeIndex( ERISA_PROB_MODEL * pModel ) ;
		// ERISA 符号を復号する
		size_t DecodeERISACodeBytes( SBYTE * ptrDst, size_t nCount ) ;
		size_t DecodeERISACodeWords( SWORD * ptrDst, size_t nCount ) ;

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERISA-N（算術）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLERISANDecodeContext	: public SGLERISADecodeContext
	{
	protected:
		BYTE		m_bytLastSymbol[4] ;		// 最近の生起シンボル
		int			m_iLastSymbol ;
		int			m_nNemesisLeft ;			// スライド辞書復号カウンタ
		int			m_nNemesisNext ;
		BYTE *		m_pNemesisBuf ;				// スライド辞書用バッファ
		int			m_nNemesisIndex ;
		bool		m_flagEOF ;

		ERISA::ERISA_PROB_BASE *		m_pProbERISA ;
		ERISA::ERISAN_PHRASE_LOOKUP *	m_pNemesisLookup ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERISANDecodeContext, SGLERISADecodeContext )
		// 構築関数
		SGLERISANDecodeContext( SGLDecodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLERISANDecodeContext( void ) ;

	public:
		// ERISA-N 符号の復号の準備をする
		void PrepareToDecodeERISANCode( void ) ;
		// ERISA-N 符号を復号する
		size_t DecodeERISANCodeBytes( SBYTE * ptrDst, size_t nCount ) ;
		// EOF フラグを取得する
		bool GetEOFFlag( void ) const
		{
			return	m_flagEOF ;
		}

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
	} ;

}

#endif
