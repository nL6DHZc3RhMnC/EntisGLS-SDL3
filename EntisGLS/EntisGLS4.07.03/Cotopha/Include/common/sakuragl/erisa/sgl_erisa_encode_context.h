
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_ENCODE_CONTEXT_H__)
#define	__SAKURA_ERISA_ENCODE_CONTEXT_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// 符号化ビットストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLEncodeBitStream	: public ESLObject
	{
	public:
		// ビットストリームバッファ
		size_t	m_nIntBufCount ;	// 中間入力バッファに蓄積されているビット数
		DWORD	m_dwIntBuffer ;		// 中間入力バッファ
		size_t	m_nBufferingSize ;	// バッファリングするバイト数
		size_t	m_nBufCount ;		// バッファに蓄積されているバイト数
		PBYTE	m_ptrBuffer ;		// 出力バッファの先頭へのポインタ

		// 出力ストリーム
		SSystem::SOutputStream *	m_pStream ;

	public:
		// 構築関数
		SGLEncodeBitStream( size_t nBufferingSize )
		{
			m_nIntBufCount = 0 ;
			m_nBufferingSize = (nBufferingSize + 0x03) & ~0x03 ;
			m_nBufCount = 0 ;
			m_ptrBuffer = (PBYTE) esl_malloc( m_nBufferingSize ) ;
			m_pStream = NULL ;
		}
		// 消滅関数
		virtual ~SGLEncodeBitStream( void ) ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLEncodeBitStream, ESLObject )

	public:
		// 入力ストリームを関連付ける
		void AttachOutputStream( SSystem::SOutputStream * pStream )
		{
			m_pStream = pStream ;
		}
		// ｎビット出力する
		SSystem::SError OutNBits( DWORD dwData, size_t nBits ) ;
		// バッファの内容を出力して空にする
		SSystem::SError Flushout( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 符号化コンテキスト基底クラス
	//////////////////////////////////////////////////////////////////////////

	class	SGLAbstractEncodeContext	: public SSystem::SOutputStream
	{
	protected:
		SGLEncodeBitStream *	m_pStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAbstractEncodeContext, SOutputStream )
		// 構築関数
		SGLAbstractEncodeContext( SGLEncodeBitStream * pStream )
							: m_pStream( pStream ) {}

	public:
		// 出力ビットストリームを関連付ける
		void AttachBitStream( SGLEncodeBitStream * pStream )
		{
			m_pStream = pStream ;
		}
		// 符号完了処理
		virtual SSystem::SError FinishEncoding( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ランレングスガンマ符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLGammaEncodeContext	: public SGLAbstractEncodeContext
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGammaEncodeContext, SGLAbstractEncodeContext )
		// 構築関数
		SGLGammaEncodeContext( SGLEncodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLGammaEncodeContext( void ) ;

	protected:
		static const size_t	m_GammaCodeBytesTable[0x100] ;

	public:
		// ガンマ符号の符号化の準備をする
		void PrepareToEncodeGammaCode( void ) ;
		// ガンマコードに符号化した際のビット数を計算
		static size_t EstimateGammaCode( int num ) ;
		// ランレングスガンマ符号に符号化した際のサイズ（ビット数）を計算
		static size_t EstimateGammaCodeBytes( const SBYTE * ptrSrc, size_t nCount ) ;
		// ガンマコードを出力する
		SSystem::SError OutGammaCode( int num ) ;
		// ランレングスガンマ符号に符号化して出力する
		size_t EncodeGammaCodeBytes( const SBYTE * ptrSrc, size_t nCount ) ;
		size_t EncodeGammaCodeWords( const SWORD * ptrSrc, size_t nCount ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERINA（ハフマン）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLHuffmanEncodeContext	: public SGLGammaEncodeContext
	{
	protected:
		DWORD							m_dwERINAFlags ;
		ERISA::ERINA_HUFFMAN_TREE *		m_pLastHuffmanTree ;
		ERISA::ERINA_HUFFMAN_TREE **	m_ppHuffmanTree ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLHuffmanEncodeContext, SGLGammaEncodeContext )
		// 構築関数
		SGLHuffmanEncodeContext( SGLEncodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLHuffmanEncodeContext( void ) ;

	public:
		// 圧縮方式
		enum	ERINAEncodingFlag
		{
			efERINAOrder0	= 0x0000,
			efERINAOrder1	= 0x0001
		} ;
		// ERINA 符号の符号化の準備をする
		void PrepareToEncodeERINACode( DWORD dwFlags = efERINAOrder1 ) ;
		// ハフマン符号で出力する
		SSystem::SError OutHuffmanCode
				( ERISA::ERINA_HUFFMAN_TREE * tree, int num ) ;
		// 長さをハフマン符号で出力する
		SSystem::SError OutLengthHuffman
				( ERISA::ERINA_HUFFMAN_TREE * tree, int nLength ) ;
		// ERINA 符号に符号化して出力する
		size_t EncodeERINACodeBytes( const SBYTE * ptrSrc, size_t nCount ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERISA（算術）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLERISAEncodeContext	: public SGLAbstractEncodeContext
	{
	protected:
		DWORD	m_dwCodeRegister ;			// コードレジスタ（16 bit）
		DWORD	m_dwAugendRegister ;		// オージェンドレジスタ（16 bit）
		DWORD	m_dwCodeBuffer ;			// コードレジスタバッファ
		SDWORD	m_dwBitBufferCount ;		// 出力された'1'の数を蓄積する
				// '0' は、次に '0' が出力された時点で、すぐに送出される。
				// '1' は、'0' が出力されるまで蓄積される。
				// -1 は、空のビット列 '' を、
				// 0 は、ビット列 '0' を、
				// 1 は、ビット列 '01' を、
				// 2 は、ビット列 '011' を表現します。

		ERISA::ERISA_PROB_MODEL **	m_ppTableERISA ;
		ERISA::ERISA_PROB_MODEL *	m_pPhraseLenProb ;
		ERISA::ERISA_PROB_MODEL *	m_pPhraseIndexProb ;
		ERISA::ERISA_PROB_MODEL *	m_pRunLenProb ;
		ERISA::ERISA_PROB_MODEL *	m_pLastERISAProb ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERISAEncodeContext, SGLAbstractEncodeContext )
		// 構築関数
		SGLERISAEncodeContext( SGLEncodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLERISAEncodeContext( void ) ;

	public:
		// ERISA 符号の符号化の準備をする
		void PrepareToEncodeERISACode( void ) ;
		// 指定の統計モデルを使って1つの算術符号を出力
		int EncodeERISACodeSymbol
			( ERISA::ERISA_PROB_MODEL * pModel, SWORD wSymbol ) ;
		int EncodeERISACodeIndex
			( ERISA::ERISA_PROB_MODEL * pModel, int iSym, WORD wFs ) ;
		// ERISA 符号に符号化して出力する
		size_t EncodeERISACodeBytes( const SBYTE * ptrSrc, size_t nCount ) ;
		size_t EncodeERISACodeWords( const SWORD * ptrSrc, size_t nCount ) ;
		// ERISA 符号を完了する
		SSystem::SError FinishERISACode( void ) ;

	public:	// SGLAbstractEncodeContext オーバーライド
		// 符号完了処理
		virtual SSystem::SError FinishEncoding( void ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERISA-N（算術）符号コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLERISANEncodeContext	: public SGLERISAEncodeContext
	{
	protected:
		BYTE	m_bytLastSymbol[4] ;		// 最近の生起シンボル
		int		m_iLastSymbol ;
		DWORD	m_dwERISAFlags ;			// 圧縮方式

		// 統計モデル用ワークメモリ
		ERISA::ERISA_PROB_BASE *		m_pProbERISA ;

		// スライド辞書用バッファ
		BYTE *	m_pNemesisBuf ;
		int		m_nNemesisIndex ;
		ERISA::ERISAN_PHRASE_LOOKUP *	m_pNemesisLookup ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERISANEncodeContext, SGLERISAEncodeContext )
		// 構築関数
		SGLERISANEncodeContext( SGLEncodeBitStream * pStream ) ;
		// 消滅関数
		virtual ~SGLERISANEncodeContext( void ) ;

	public:
		// 圧縮方式
		enum	ERISAEncodingFlag
		{
			efSimple		= 0x0000,
			efNemesis		= 0x0001,
			efRunLength		= 0x0002,
			efRLNemesis		= 0x0003
		} ;
		// ERISA-N 符号の符号化の準備をする
		void PrepareToEncodeERISANCode( DWORD dwFlags = efNemesis ) ;
		// ERISA-N 符号に符号化して出力する
		size_t EncodeERISANCodeBytes( const SBYTE * ptrSrc, size_t nCount ) ;
		// ERISA-N 符号の EOF を出力する
		void EncodeERISANCodeEOF( void ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;


}

#endif
