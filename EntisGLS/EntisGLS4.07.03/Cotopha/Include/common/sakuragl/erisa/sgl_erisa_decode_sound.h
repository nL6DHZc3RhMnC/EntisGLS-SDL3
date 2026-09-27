
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_DECODE_SOUND_H__)
#define	__SAKURA_ERISA_DECODE_SOUND_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// MIO 音声ファイル構造体
	//////////////////////////////////////////////////////////////////////////

	struct	MIO_INFO_HEADER
	{
		DWORD	dwVersion ;
		DWORD	fdwTransformation ;		// enum ERITransformation
		DWORD	dwArchitecture ;		// enum ERIArchitecture
		DWORD	dwChannelCount ;
		DWORD	dwSamplesPerSec ;
		DWORD	dwBlocksetCount ;
		DWORD	dwSubbandDegree ;
		DWORD	dwAllSampleCount ;
		DWORD	dwLappedDegree ;
		DWORD	dwBitsPerSample ;
	} ;

	struct	MIO_DATA_HEADER
	{
		BYTE	bytVersion ;
		BYTE	bytFlags ;			// complex enum MIODataFlag
		BYTE	bytReserved1 ;
		BYTE	bytReserved2 ;
		DWORD	dwSampleCount ;
	} ;

	enum	MIODataFlag
	{
		mioDataLeadBlock	= 0x01,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 音声展開オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundDecoder	: public	ESLObject
	{
	protected:
		ERISA::MIO_INFO_HEADER		m_mioih ;			// 音声情報ヘッダ

		size_t						m_nBufLength ;			// バッファ長（サンプル数）
		SSystem::SArray<uint8_t>	m_bufWork ;
		SSystem::SArray<uint8_t>	m_bufWork2 ;
		void *						m_ptrBuffer1 ;			// 差分処理バッファ
		void *						m_ptrBuffer2 ;			// 並び替えバッファ
		int8_t *					m_ptrBuffer3 ;			// インターリーブ用バッファ
		uint8_t *					m_ptrDivisionTable ;	// 分解コードテーブル
		uint8_t *					m_ptrRevolveCode ;		// 回転コードテーブル
		int32_t *					m_ptrWeightCode ;		// 量子化係数テーブル
		int *						m_ptrCoefficient ;		//
		float32_t *					m_ptrMatrixBuf ;		// 行列演算用バッファ
		float32_t *					m_ptrInternalBuf ;		// 中間バッファ
		float32_t *					m_ptrWorkBuf ;			// DCT 演算用ワークエリア
		float32_t *					m_ptrWorkBuf2 ;
		float32_t *					m_ptrWeightTable ;		// 各周波数成分の重みテーブル
		float32_t *					m_ptrLastDCT ;			// 直前の DCT 係数

		uint8_t *					m_ptrNextDivision ;		// 次の分解コード
		uint8_t *					m_ptrNextRevCode ;		// 次の回転コード
		int32_t *					m_ptrNextWeight ;		// 次の量子化係数
		int *						m_ptrNextCoefficient ;	//
		int *						m_ptrNextSource ;		// 次の入力信号
		float32_t *					m_ptrLastDCTBuf ;		// 重複演算用バッファ
		size_t						m_nSubbandDegree ;		// 行列のサイズ
		size_t						m_nDegreeNum ;
		const ERISA::SFP_SIN_COS *	m_pRevolveParam ;
		size_t						m_nFrequencyPoint[7] ;	// 各周波数帯の中心位置

		SGLGammaDecodeContext		m_ctxGamma ;
		SGLHuffmanDecodeContext		m_ctxHuffman ;
		SGLERISADecodeContext		m_ctxNemesis ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundDecoder, ESLObject )
		// 構築関数
		SGLSoundDecoder( void ) ;
		// 消滅関数
		virtual ~SGLSoundDecoder( void ) ;

		// 初期化（パラメータの設定）
		virtual SSystem::SError Initialize( const ERISA::MIO_INFO_HEADER & infhdr ) ;
		// 終了（メモリの解放など）
		virtual void Delete( void ) ;
		// 音声を圧縮
		virtual SSystem::SError DecodeSound
			( ERISA::SGLDecodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf ) ;

	protected:		// 可逆圧縮
		// 8ビットのPCMを展開
		SSystem::SError DecodeSoundPCM8
			( ERISA::SGLDecodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf ) ;
		// 16ビットのPCMを展開
		SSystem::SError DecodeSoundPCM16
			( ERISA::SGLDecodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf ) ;

	protected:		// モノラル・ステレオ
		// 行列サイズの変更に伴うパラメータの再計算
		void InitializeWithDegree( size_t nSubbandDegree ) ;
		// 16ビットの非可逆展開
		SSystem::SError DecodeSoundDCT
			( ERISA::SGLDecodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf ) ;
		// 通常のブロックを復号する
		SSystem::SError DecodeInternalBlock
			( int16_t * ptrDst, size_t nSamples ) ;
		// リードブロックを復号する
		SSystem::SError DecodeLeadBlock( void ) ;
		// ポストブロックを復号する
		SSystem::SError DecodePostBlock
			( int16_t * ptrDst, size_t nSamples ) ;
		// 逆量子化
		void IQuantumize
			( float32_t * ptrDestination,
				const int * ptrQuantumized, size_t nDegreeNum,
				int32_t nWeightCode, int nCoefficient ) ;

	protected:		// ミドルサイドステレオ
		// 16ビットの非可逆展開
		SSystem::SError DecodeSoundDCT_MSS
			( ERISA::SGLDecodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf ) ;
		// 通常のブロックを復号する
		SSystem::SError DecodeInternalBlock_MSS
			( int16_t * ptrDst, size_t nSamples ) ;
		// リードブロックを復号する
		SSystem::SError DecodeLeadBlock_MSS( void ) ;
		// ポストブロックを復号する
		SSystem::SError DecodePostBlock_MSS
			( int16_t * ptrDst, size_t nSamples ) ;
	} ;

}

#endif

