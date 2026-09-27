
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2015 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_ENCODE_SOUND_H__)
#define	__SAKURA_ERISA_ENCODE_SOUND_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// 音声圧縮オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundEncoder	: public ESLObject
	{
	public:
		enum	PresetParameter
		{
			ppVBR235kbps, ppVBR176kbps, ppVBR156kbps,
			ppVBR141kbps, ppVBR128kbps, ppVBR117kbps,
			ppVBR94kbps, ppVBR78kbps, ppVBR70kbps,
			ppMax
		} ;
		struct	Parameter
		{
			double	fpLowWeight ;			// 低周波成分の重み
			double	fpMiddleWeight ;		// 中周波成分の重み
			double	fpPowerScale ;			// 量子化の基準ビット数
			int		nOddWeight ;			// ブロック歪対策係数
			int		nPreEchoThreshold ;		// プリエコー対策閾値

			// 構築関数
			Parameter( void ) ;
			// プリセット値取得
			void LoadPresetParam
				( PresetParameter ppIndex, ERISA::MIO_INFO_HEADER & infhdr ) ;
		} ;

	protected:
		ERISA::MIO_INFO_HEADER		m_mioih ;				// 音声情報ヘッダ

		size_t						m_nBufLength ;			// バッファ長（サンプル数）
		uint8_t *					m_ptrBuffer1 ;			// 差分処理バッファ
		SSystem::SArray<uint8_t>	m_bufBuffer1 ;
		uint8_t *					m_ptrBuffer2 ;			// 並び替えバッファ
		SSystem::SArray<uint8_t>	m_bufBuffer2 ;
		int8_t *					m_ptrBuffer3 ;			// インターリーブ用バッファ
		SSystem::SArray<int8_t>		m_bufBuffer3 ;
		float32_t *					m_ptrSamplingBuf ;		// サンプリング用バッファ
		SSystem::SArray<float32_t>	m_bufSamplingBuf ;
		float32_t *					m_ptrInternalBuf ;		// 中間バッファ
		SSystem::SArray<float32_t>	m_bufInternalBuf ;
		float32_t *					m_ptrDstBuf ;			// 出力用バッファ
		SSystem::SArray<float32_t>	m_bufDstBuf ;
		float32_t *					m_ptrWorkBuf ;			// DCT 演算用ワークエリア
		SSystem::SArray<float32_t>	m_bufWorkBuf ;
		float32_t *					m_ptrWeightTable ;		// 各周波数成分の重みテーブル
		SSystem::SArray<float32_t>	m_bufWeightTable ;
		float32_t *					m_ptrLastDCT ;			// 直前ブロックの DCT 係数
		SSystem::SArray<float32_t>	m_bufLastDCT ;

		Parameter					m_parameter ;

		int16_t *					m_ptrNextDstBuf ;		// 出力バッファアドレス
		float32_t *					m_ptrLastDCTBuf ;		// 重複演算用バッファ
		size_t						m_nSubbandDegree ;		// 行列のサイズ
		size_t						m_nDegreeNum ;
		size_t						m_nFrequencyWidth[7] ;	// 各周波数帯の幅
		size_t						m_nFrequencyPoint[7] ;	// 各周波数帯の中心位置
		const ERISA::SFP_SIN_COS *	m_pRevolveParam ;

		SGLGammaEncodeContext		m_ctxGamma ;
		SGLHuffmanEncodeContext		m_ctxHuffman ;
		SGLERISAEncodeContext		m_ctxNemesis ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundEncoder, ESLObject )
		// 構築関数
		SGLSoundEncoder( void ) ;
		// 消滅関数
		virtual ~SGLSoundEncoder( void ) ;

	public:
		// 初期化（パラメータの設定）
		virtual SSystem::SError Initialize
			( const ERISA::MIO_INFO_HEADER & infhdr ) ;
		// 終了（メモリの解放など）
		virtual void Delete( void ) ;
		// 音声を圧縮
		virtual SSystem::SError EncodeSound
			( ERISA::SGLEncodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf ) ;
		// 圧縮オプションを設定
		void SetCompressionParameter( const Parameter & parameter ) ;

	protected:		// 可逆圧縮
		// 8ビットのPCMを圧縮
		SSystem::SError EncodeSoundPCM8
			( ERISA::SGLEncodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf ) ;
		// 16ビットのPCMを圧縮
		SSystem::SError EncodeSoundPCM16
			( ERISA::SGLEncodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf ) ;

	protected:		// 非可逆圧縮
		// 行列サイズの変更に伴うパラメータの再計算
		void InitializeWithDegree( size_t nSubbandDegree ) ;
		// 指定サンプル列の音量を求める
		double EvaluateVolume( const float32_t * ptrWave, size_t nCount ) ;
		// 分解コードを取得する
		size_t GetDivisionCode( const float32_t * ptrSamples ) ;

	protected:	// モノラル・ステレオ
		// 16ビットの非可逆圧縮
		SSystem::SError EncodeSoundDCT
			( ERISA::SGLEncodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf ) ;
		// LOT 変換を施す
		void PerformLOT
			( ERISA::SGLEncodeBitStream & bstream,
				float32_t * ptrSamples, float32_t fpPowerScale ) ;
		// 通常のブロックを符号化して出力する
		SSystem::SError EncodeInternalBlock
			( ERISA::SGLEncodeBitStream & bstream,
				float32_t * ptrSamples, float32_t fpPowerScale ) ;
		// リードブロックを符号化して出力する
		SSystem::SError EncodeLeadBlock
			( ERISA::SGLEncodeBitStream & bstream,
				float32_t * ptrSamples, float32_t fpPowerScale ) ;
		// ポストブロックを符号化して出力する
		SSystem::SError EncodePostBlock
			( ERISA::SGLEncodeBitStream & bstream, float32_t fpPowerScale ) ;
		// 量子化
		void Quantumize
			( int32_t * ptrQuantumized, const float32_t * ptrSource,
				size_t nDegreeNum, float32_t fpPowerScale,
				DWORD * ptrWeightCode, int * ptrCoefficient ) ;

	protected:		// ミドルサイドステレオ
		// 16ビットの非可逆圧縮
		SSystem::SError EncodeSoundDCT_MSS
			( ERISA::SGLEncodeBitStream & bstream,
				const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf ) ;
		// 回転パラメータを取得する
		int GetRevolveCode
			( const float32_t * ptrBuf1, const float32_t * ptrBuf2 ) ;
		// LOT 変換を施す
		void PerformLOT_MSS
			( float32_t * ptrDst, float32_t * ptrLapBuf, float32_t * ptrSrc ) ;
		// 通常のブロックを符号化して出力する
		SSystem::SError EncodeInternalBlock_MSS
			( ERISA::SGLEncodeBitStream & bstream,
				float32_t * ptrSrc1, float32_t * ptrSrc2, float32_t fpPowerScale ) ;
		// リードブロックを符号化して出力する
		SSystem::SError EncodeLeadBlock_MSS
			( ERISA::SGLEncodeBitStream & bstream,
				float32_t * ptrSrc1, float32_t * ptrSrc2, float32_t fpPowerScale ) ;
		// ポストブロックを符号化して出力する
		SSystem::SError EncodePostBlock_MSS
			( ERISA::SGLEncodeBitStream & bstream, float32_t fpPowerScale ) ;
		// 量子化
		void Quantumize_MSS
			( int32_t * ptrQuantumized, const float32_t * ptrSource,
				size_t nDegreeNum, float32_t fpPowerScale,
				DWORD * ptrWeightCode, int * ptrCoefficient ) ;
	} ;

}

#endif
