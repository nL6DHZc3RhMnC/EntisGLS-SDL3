
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// 音声デコーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLSoundDecoder, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundDecoder::SGLSoundDecoder( void )
	: m_ctxGamma( NULL ), m_ctxHuffman( NULL ), m_ctxNemesis( NULL )
{
	m_nBufLength = 0 ;
	m_ptrBuffer1 = NULL ;
	m_ptrBuffer2 = NULL ;
	m_ptrBuffer3 = NULL ;
	m_ptrDivisionTable = NULL ;
	m_ptrRevolveCode = NULL ;
	m_ptrWeightCode = NULL ;
	m_ptrCoefficient = NULL ;
	m_ptrMatrixBuf = NULL ;
	m_ptrInternalBuf = NULL ;
	m_ptrWorkBuf = NULL ;
	m_ptrWorkBuf2 = NULL ;
	m_ptrWeightTable = NULL ;
	m_ptrLastDCT = NULL ;
	m_pRevolveParam = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundDecoder::~SGLSoundDecoder( void )
{
	SGLSoundDecoder::Delete( ) ;
}

// 初期化（パラメータの設定）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::Initialize( const ERISA::MIO_INFO_HEADER & infhdr )
{
	//
	// 以前のリソースを解放
	//
	Delete( ) ;
	//
	// 音声情報ヘッダをコピー
	//
	m_mioih = infhdr ;
	//
	// パラメータのチェック
	//
	if ( m_mioih.fdwTransformation == eriTransformationLossless )
	{
		if ( m_mioih.dwArchitecture != eriRunlengthHuffman )
		{
			return	errFailed ;		// エラー（未対応の圧縮フォーマット）
		}
		if ( (m_mioih.dwChannelCount != 1) && (m_mioih.dwChannelCount != 2) )
		{
			return	errFailed ;		// エラー（未対応のチャネル数）
		}
		if ( (m_mioih.dwBitsPerSample != 8) && (m_mioih.dwBitsPerSample != 16) )
		{
			return	errFailed ;		// エラー（未対応のサンプリング分解能）
		}
	}
	else if ( (m_mioih.fdwTransformation == eriTransformationLOT)
			|| (m_mioih.fdwTransformation == eriTransformationLOT_MSS) )
	{
		if ( (m_mioih.dwArchitecture != eriRunlengthGamma)
			&& (m_mioih.dwArchitecture != eriRunlengthHuffman)
			&& (m_mioih.dwArchitecture != erisaNemesisCode)
			&& (m_mioih.dwArchitecture != erisaRunlengthGamma) )
		{
			return	errFailed ;		// エラー（未対応の圧縮フォーマット）
		}
		if ( (m_mioih.dwChannelCount != 1) && (m_mioih.dwChannelCount != 2) )
		{
			return	errFailed ;		// エラー（未対応のチャネル数）
		}
		if ( m_mioih.dwBitsPerSample != 16 )
		{
			return	errFailed ;		// エラー（未対応のサンプリング分解能）
		}
		if ( (m_mioih.dwSubbandDegree < 8) ||
				(m_mioih.dwSubbandDegree > ERISA::MAX_DCT_DEGREE) )
		{
			return	errFailed ;		// エラー（未対応のDCT次数）
		}
		if ( m_mioih.dwLappedDegree != 1 )
		{
			return	errFailed ;		// エラー（未対応の重複係数）
		}
		//
		// DCT 用バッファ確保
		//
		const size_t	nBlockBufSize =
							(sizeof(float32_t) << m_mioih.dwSubbandDegree) ;
		const size_t	nChBlockBufSize =
							nBlockBufSize * m_mioih.dwChannelCount ;
		size_t			i, nBlocksetSamples, nLappedSamples ;
		nBlocksetSamples = m_mioih.dwChannelCount << m_mioih.dwSubbandDegree ;
		nLappedSamples = nBlocksetSamples * m_mioih.dwLappedDegree ;
		//
		m_bufWork.SetLength
			( nChBlockBufSize * 3
				+ nBlockBufSize * 3
				+ nLappedSamples * sizeof(float32_t) ) ;
		uint8_t *	ptrWorkNext = m_bufWork.GetArray() ;
		//
		m_ptrBuffer1 = ptrWorkNext ;
		ptrWorkNext += nChBlockBufSize ;
		//
		m_ptrMatrixBuf = (float32_t*) ptrWorkNext ;
		ptrWorkNext += nChBlockBufSize ;
		//
		m_ptrInternalBuf = (float32_t*) ptrWorkNext ;
		ptrWorkNext += nChBlockBufSize ;
		//
		m_ptrWorkBuf = (float32_t*) ptrWorkNext ;
		ptrWorkNext += nBlockBufSize ;
		//
		m_ptrWorkBuf2 = (float32_t*) ptrWorkNext ;
		ptrWorkNext += nBlockBufSize ;
		//
		// 重みテーブルを確保
		//
		m_ptrWeightTable = (float32_t*) ptrWorkNext ;
		ptrWorkNext += nBlockBufSize ;
		//
		// LOT 用バッファ確保
		//
		if ( nLappedSamples > 0 )
		{
			m_ptrLastDCT = (float32_t*) ptrWorkNext ;
			for ( i = 0; i < nLappedSamples; i ++ )
			{
				m_ptrLastDCT[i] = 0.0F ;
			}
		}
		//
		// パラメータ初期化
		//
		InitializeWithDegree( m_mioih.dwSubbandDegree ) ;
	}
	else
	{
		return	errFailed ;		// （未対応の圧縮フォーマット）
	}
	//
	// 正常終了
	//
	return	errSuccess ;
}

// 終了（メモリの解放など）
//////////////////////////////////////////////////////////////////////////////
void SGLSoundDecoder::Delete( void )
{
	m_bufWork.FreeArray() ;
	m_ptrBuffer1 = NULL ;
	m_ptrMatrixBuf = NULL ;
	m_ptrInternalBuf = NULL ;
	m_ptrWorkBuf = NULL ;
	m_ptrWorkBuf2 = NULL ;
	m_ptrWeightTable = NULL ;
	m_ptrLastDCT = NULL ;
	//
	m_bufWork2.FreeArray() ;
	m_nBufLength = 0 ;
	m_ptrBuffer2 = NULL ;
	m_ptrBuffer3 = NULL ;
	m_ptrDivisionTable = NULL ;
	m_ptrRevolveCode = NULL ;
	m_ptrWeightCode = NULL ;
	m_ptrCoefficient = NULL ;
}

// 音声を圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeSound
	( ERISA::SGLDecodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf )
{
	bstream.FlushBuffer() ;
	//
	if ( m_mioih.fdwTransformation == eriTransformationLossless )
	{
		if ( m_mioih.dwBitsPerSample == 8 )
		{
			return	DecodeSoundPCM8( bstream, datahdr, ptrWaveBuf ) ;
		}
		else if ( m_mioih.dwBitsPerSample == 16 )
		{
			return	DecodeSoundPCM16( bstream, datahdr, ptrWaveBuf ) ;
		}
	}
	else if ( (m_mioih.fdwTransformation == eriTransformationLOT)
			|| (m_mioih.fdwTransformation == eriTransformationLOT_MSS) )
	{
		if ( (m_mioih.dwChannelCount != 2) ||
				(m_mioih.fdwTransformation == eriTransformationLOT) )
		{
			return	DecodeSoundDCT( bstream, datahdr, ptrWaveBuf ) ;
		}
		else
		{
			return	DecodeSoundDCT_MSS( bstream, datahdr, ptrWaveBuf ) ;
		}
	}
	return	errFailed ;			// エラー
}

// 8ビットのPCMを展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeSoundPCM8
	( ERISA::SGLDecodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf )
{
	//
	// 演算用バッファを確保
	//
	size_t	nSampleCount = datahdr.dwSampleCount ;
	if ( nSampleCount > m_nBufLength )
	{
		m_bufWork2.FreeArray() ;
		m_bufWork2.SetLength( nSampleCount * m_mioih.dwChannelCount ) ;
		m_ptrBuffer1 = m_bufWork2.GetArray() ;
		m_nBufLength = nSampleCount ;
	}
	//
	// ハフマン符号を復号
	//
	m_ctxHuffman.AttachBitStream( &bstream ) ;
	if ( datahdr.bytFlags & mioDataLeadBlock )
	{
		m_ctxHuffman.PrepareToDecodeERINACode( ) ;
	}
	size_t	nBytes = nSampleCount * m_mioih.dwChannelCount ;
	if ( m_ctxHuffman.DecodeERINACodeBytes
			( (SBYTE*) m_ptrBuffer1, nBytes ) < nBytes )
	{
		return	errFailed ;			// エラー
	}
	//
	// 差分処理を施して出力
	//
	uint8_t *	ptrSrcBuf = (uint8_t*) m_ptrBuffer1 ;
	uint8_t *	ptrDstBuf ;
	size_t		nStep = m_mioih.dwChannelCount ;
	size_t		i, j ;
	for ( i = 0; i < m_mioih.dwChannelCount; i ++ )
	{
		ptrDstBuf = (PBYTE) ptrWaveBuf ;
		ptrDstBuf += i ;
		//
		uint8_t	bytValue = 0 ;
		for ( j = 0; j < nSampleCount; j ++ )
		{
			bytValue += *(ptrSrcBuf ++) ;
			*ptrDstBuf = bytValue ;
			ptrDstBuf += nStep ;
		}
	}
	return	errSuccess ;
}

// 16ビットのPCMを展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeSoundPCM16
	( ERISA::SGLDecodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf )
{
	//
	// 演算用バッファを確保
	//
	size_t	i, j ;
	size_t	nSampleCount = datahdr.dwSampleCount ;
	size_t	nChannelCount = m_mioih.dwChannelCount ;
	size_t	nAllSampleCount = nSampleCount * nChannelCount ;
	//
	if ( nSampleCount > m_nBufLength )
	{
		m_bufWork2.FreeArray() ;
		m_bufWork2.SetLength( nAllSampleCount * (sizeof(int16_t) * 2) ) ;
		//
		uint8_t *	ptrWorkNext = m_bufWork2.GetArray() ;
		m_ptrBuffer1 = ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int16_t) ;
		//
		m_ptrBuffer2 = ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int16_t) ;
		//
		m_nBufLength = nSampleCount ;
	}
	//
	// ハフマン符号を復号
	//
	m_ctxHuffman.AttachBitStream( &bstream ) ;
	if ( datahdr.bytFlags & mioDataLeadBlock )
	{
		m_ctxHuffman.PrepareToDecodeERINACode( ) ;
	}
	size_t	nBytes = nSampleCount * nChannelCount * sizeof(int16_t) ;
	if ( m_ctxHuffman.DecodeERINACodeBytes
			( (SBYTE*) m_ptrBuffer1, nBytes ) < nBytes )
	{
		return	errFailed ;			// エラー
	}
	//
	// 上位バイトと下位バイトをパッキング
	//
	uint8_t *	pbytSrcBuf1 ;
	uint8_t *	pbytSrcBuf2 ;
	uint8_t *	pbytDstBuf ;
	for ( i = 0; i < m_mioih.dwChannelCount; i ++ )
	{
		size_t	nOffset = i * nSampleCount * sizeof(int16_t) ;
		pbytSrcBuf1 = ((uint8_t*)m_ptrBuffer1) + nOffset ;
		pbytSrcBuf2 = pbytSrcBuf1 + nSampleCount ;
		pbytDstBuf = ((uint8_t*)m_ptrBuffer2) + nOffset ;
		//
		for ( j = 0; j < nSampleCount; j ++ )
		{
			int8_t	bytLow = (int8_t) pbytSrcBuf2[j] ;
			int8_t	bytHigh = pbytSrcBuf1[j] ;
			pbytDstBuf[j * sizeof(int16_t)]     = bytLow ;
			pbytDstBuf[j * sizeof(int16_t) + 1] = bytHigh ^ (bytLow >> 7) ;
		}
	}
	//
	// 差分処理を施して出力
	//
	int16_t *	ptrSrcBuf = (int16_t*) m_ptrBuffer2 ;
	int16_t *	ptrDstBuf ;
	size_t		nStep = m_mioih.dwChannelCount ;
	for ( i = 0; i < m_mioih.dwChannelCount; i ++ )
	{
		ptrDstBuf = (int16_t*) ptrWaveBuf ;
		ptrDstBuf += i ;
		//
		int16_t	wValue = 0 ;
		int16_t	wDelta = 0 ;
		for ( j = 0; j < nSampleCount; j ++ )
		{
			wDelta +=  *(ptrSrcBuf ++) ;
			wValue += wDelta ;
			*ptrDstBuf = wValue ;
			ptrDstBuf += nStep ;
		}
	}
	return	errSuccess ;
}

// 行列サイズの変更に伴うパラメータの再計算
//////////////////////////////////////////////////////////////////////////////
void SGLSoundDecoder::InitializeWithDegree( size_t nSubbandDegree )
{
	//
	// 回転パラメータ生成
	//
	m_pRevolveParam = ERISA::sclfGetRevolveParameter( nSubbandDegree ) ;
	//
	// 逆量子化用パラメータ生成
	//
	static const int	freq_width[7] =
	{
		-6, -6, -5, -4, -3, -2, -1
	} ;
	for ( int i = 0, j = 0; i < 7; i ++ )
	{
		int	nFrequencyWidth = 1 << (nSubbandDegree + freq_width[i]) ;
		m_nFrequencyPoint[i] = j + (nFrequencyWidth / 2) ;
		j += nFrequencyWidth ;
	}
	//
	// ローカルパラメータを設定
	//
	m_nSubbandDegree = nSubbandDegree ;
	m_nDegreeNum = ((size_t) 1 << nSubbandDegree) ;
}

// 16ビットの非可逆展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeSoundDCT
	( ERISA::SGLDecodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf )
{
	//
	// バッファを確保
	//
	const size_t	nDegreeWidth = ((size_t) 1 << m_mioih.dwSubbandDegree) ;
	const size_t	nSampleCount =
						(datahdr.dwSampleCount
							+ nDegreeWidth - 1) & ~(nDegreeWidth - 1) ;
	const size_t	nSubbandCount = (nSampleCount >> m_mioih.dwSubbandDegree) ;
	const size_t	nChannelCount = m_mioih.dwChannelCount ;
	const size_t	nAllSampleCount = nSampleCount * nChannelCount ;
	const size_t	nAllSubbandCount = nSubbandCount * nChannelCount ;
	size_t			i, j, k ;
	//
	if ( nSampleCount > m_nBufLength )
	{
		m_bufWork2.FreeArray() ;
		m_bufWork2.SetLength
			( nAllSampleCount * sizeof(int)
				+ nAllSampleCount * sizeof(int16_t)
				+ nAllSubbandCount * sizeof(uint8_t)
				+ nAllSubbandCount * 5 * sizeof(int32_t)
				+ nAllSubbandCount * 5 * sizeof(int) ) ;
		//
		uint8_t *	ptrWorkNext = m_bufWork2.GetArray() ;
		m_ptrBuffer2 = ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int) ;
		//
		m_ptrBuffer3 = (int8_t*) ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int16_t) ;
		//
		m_ptrDivisionTable = (uint8_t*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * sizeof(uint8_t) ;
		//
		m_ptrWeightCode = (int32_t*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * 5 * sizeof(int32_t) ;
		//
		m_ptrCoefficient = (int*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * 5 * sizeof(int) ;
		//
		m_nBufLength = nSampleCount ;
	}
	//
	// 量子化係数テーブルを復号
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;
	}
	ESLAssert( nChannelCount <= 0x10 ) ;
	unsigned int	nLastDivision[0x10] ;
	//
	m_ptrNextDivision = m_ptrDivisionTable ;
	m_ptrNextWeight = m_ptrWeightCode ;
	m_ptrNextCoefficient = m_ptrCoefficient ;
	//
	for ( i = 0; i < nChannelCount; i ++ )
	{
		nLastDivision[i] = (unsigned int) -1 ;
	}
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		for ( j = 0; j < nChannelCount; j ++ )
		{
			unsigned int	nDivisionCode = bstream.GetNBits( 2 ) ;
			*(m_ptrNextDivision ++) = (BYTE) nDivisionCode ;
			//
			if ( nDivisionCode != nLastDivision[j] )
			{
				if ( i != 0 )
				{
					*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
					*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
				}
				nLastDivision[j] = nDivisionCode ;
			}
			//
			size_t	nDivisionCount = ((size_t) 1 << nDivisionCode) ;
			for ( k = 0; k < nDivisionCount; k ++ )
			{
				*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
				*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
			}
		}
	}
	if ( nSubbandCount > 0 )
	{
		for ( i = 0; i < nChannelCount; i ++ )
		{
			*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
			*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
		}
	}
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;
	}
	SGLAbstractDecodeContext *	context = NULL ;
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		context = &m_ctxGamma ;
		m_ctxGamma.AttachBitStream( &bstream ) ;
		m_ctxGamma.InitGammaContext( ) ;
	}
	else if ( datahdr.bytFlags & mioDataLeadBlock )
	{
		if ( m_mioih.dwArchitecture == erisaNemesisCode )
		{
			m_ctxNemesis.PrepareToDecodeERISACode( ) ;
			context = &m_ctxNemesis ;
		}
		else
		{
			m_ctxHuffman.PrepareToDecodeERINACode( ) ;
			context = &m_ctxHuffman ;
		}
	}
	else if ( m_mioih.dwArchitecture == erisaNemesisCode )
	{
		m_ctxNemesis.InitializeERISACode( ) ;
		context = &m_ctxNemesis ;
	}
	else
	{
		context = &m_ctxHuffman ;
	}
	context->AttachBitStream( &bstream ) ;
	//
	// 復号して逆インターリブ
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		if ( m_ctxGamma.DecodeGammaCodeWords
				( (SWORD*) m_ptrBuffer3, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		for ( i = 0; i < nAllSampleCount; i ++ )
		{
			((int*)m_ptrBuffer2)[i] = ((int16_t*)m_ptrBuffer3)[i] ;
		}
	}
	else if ( m_mioih.dwArchitecture != erisaNemesisCode )
	{
		if ( context->Read
			( m_ptrBuffer3, nAllSampleCount * 2 ) < nAllSampleCount * 2 )
		{
			return	errFailed ;			// エラー
		}
		//
		int8_t *	ptrHBuf = m_ptrBuffer3 ;
		int8_t *	ptrLBuf = m_ptrBuffer3 + nAllSampleCount ;
		//
		for ( i = 0; i < nDegreeWidth; i ++ )
		{
			int *	ptrQuantumized = ((int*) m_ptrBuffer2) + i ;
			//
			for ( j = 0; j < nAllSubbandCount; j ++ )
			{
				int	nLow = *(ptrLBuf ++) ;
				int	nHigh = *(ptrHBuf ++) ^ (nLow >> 8) ;
				*ptrQuantumized = (nLow & 0xFF) | (nHigh << 8) ;
				ptrQuantumized += nDegreeWidth ;
			}
		}
	}
	else
	{
		if ( m_ctxNemesis.DecodeERISACodeWords
				( (SWORD*) m_ptrBuffer3, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		for ( i = 0; i < nAllSampleCount; i ++ )
		{
			((int*)m_ptrBuffer2)[i] = ((int16_t*)m_ptrBuffer3)[i] ;
		}
	}
	//
	// サブバンド単位で逆 DCT 変換を施す
	//
	ESLAssert( nChannelCount <= 0x10 ) ;
	size_t		nRestSamples[0x10] ;
	int16_t *	ptrDstBuf[0x10] ;
	size_t		nSamples ;
	//
	m_ptrNextDivision = m_ptrDivisionTable ;
	m_ptrNextWeight = m_ptrWeightCode ;
	m_ptrNextCoefficient = m_ptrCoefficient ;
	m_ptrNextSource = (int*) m_ptrBuffer2 ;
	//
	for ( i = 0; i < nChannelCount; i ++ )
	{
		nLastDivision[i] = -1 ;
		nRestSamples[i] = datahdr.dwSampleCount ;
		ptrDstBuf[i] = ((int16_t*) ptrWaveBuf) + i ;
	}
	size_t	nCurrentDivision = (size_t) -1 ;
	//
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		for ( j = 0; j < nChannelCount; j ++ )
		{
			//
			// 分解コードを取得
			//
			unsigned int	nDivisionCode = *(m_ptrNextDivision ++) ;
			size_t			nDivisionCount = ((size_t) 1 << nDivisionCode) ;
			//
			// 重複処理用バッファを取得
			//
			size_t	nChannelStep = nDegreeWidth * m_mioih.dwLappedDegree * j ;
			m_ptrLastDCTBuf = m_ptrLastDCT + nChannelStep ;
			//
			// 行列サイズが変化する際の処理
			//
			bool	fLeadBlock = false ;
			if ( nLastDivision[j] != nDivisionCode )
			{
				//
				// 直前までの行列を完成させる
				//
				if ( i != 0 )
				{
					if ( nCurrentDivision != nLastDivision[j] )
					{
						InitializeWithDegree
							( m_mioih.dwSubbandDegree - nLastDivision[j] ) ;
						nCurrentDivision = nLastDivision[j] ;
					}
					nSamples = nRestSamples[j] ;
					if ( nSamples > m_nDegreeNum )
					{
						nSamples = m_nDegreeNum ;
					}
					if ( DecodePostBlock( ptrDstBuf[j], nSamples ) )
					{
						return	errFailed ;
					}
					nRestSamples[j] -= nSamples ;
					ptrDstBuf[j] += nSamples * nChannelCount ;
				}
				//
				// 行列サイズを変化させるためのパラメータをセットアップ
				//
				nLastDivision[j] = nDivisionCode ;
				fLeadBlock = true ;
			}
			if ( nCurrentDivision != nDivisionCode )
			{
				InitializeWithDegree
					( m_mioih.dwSubbandDegree - nDivisionCode ) ;
				nCurrentDivision = nDivisionCode ;
			}
			//
			// 順次逆 LOT 変換を施す
			//
			for ( k = 0; k < nDivisionCount; k ++ )
			{
				if ( fLeadBlock )
				{
					//
					// リードブロックを復号する
					//
					if ( DecodeLeadBlock() )
					{
						return	errFailed ;
					}
					fLeadBlock = false ;
				}
				else
				{
					//
					// 通常ブロックを復号する
					//
					nSamples = nRestSamples[j] ;
					if ( nSamples > m_nDegreeNum )
					{
						nSamples = m_nDegreeNum ;
					}
					if ( DecodeInternalBlock( ptrDstBuf[j], nSamples ) )
					{
						return	errFailed ;
					}
					nRestSamples[j] -= nSamples ;
					ptrDstBuf[j] += nSamples * nChannelCount ;
				}
			}
		}
	}
	//
	// 行列を完成させる
	//
	if ( nSubbandCount > 0 )
	{
		for ( i = 0; i < nChannelCount; i ++ )
		{
			size_t	nChannelStep = nDegreeWidth * m_mioih.dwLappedDegree * i ;
			m_ptrLastDCTBuf = m_ptrLastDCT + nChannelStep ;
			//
			if ( nCurrentDivision != nLastDivision[i] )
			{
				InitializeWithDegree
					( m_mioih.dwSubbandDegree - nLastDivision[i] ) ;
				nCurrentDivision = nLastDivision[i] ;
			}
			nSamples = nRestSamples[i] ;
			if ( nSamples > m_nDegreeNum )
			{
				nSamples = m_nDegreeNum ;
			}
			if ( DecodePostBlock( ptrDstBuf[i], nSamples ) )
			{
				return	errFailed ;
			}
			nRestSamples[i] -= nSamples ;
			ptrDstBuf[i] += nSamples * nChannelCount ;
		}
	}
	return	errSuccess ;
}

// 通常のブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeInternalBlock
	( int16_t * ptrDst, size_t nSamples )
{
	//
	// 逆量子化
	//
	int32_t	nWeightCode = *(m_ptrNextWeight ++) ;
	int		nCoefficient = *(m_ptrNextCoefficient ++) ;
	IQuantumize
		( m_ptrMatrixBuf, m_ptrNextSource,
			m_nDegreeNum, nWeightCode, nCoefficient ) ;
	m_ptrNextSource += m_nDegreeNum ;
	//
	// 逆 LOT を施す
	//
	ERISA::sclfOddGivensInverseMatrix
		( m_ptrMatrixBuf, m_pRevolveParam, m_nSubbandDegree ) ;
	ERISA::sclfFastIPLOT
		( m_ptrMatrixBuf, m_nSubbandDegree ) ;
	ERISA::sclfFastILOT
		( m_ptrWorkBuf, m_ptrLastDCTBuf,
			m_ptrMatrixBuf, m_nSubbandDegree ) ;
	//
	for ( size_t i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrLastDCTBuf[i] = m_ptrMatrixBuf[i] ;
		m_ptrMatrixBuf[i] = m_ptrWorkBuf[i] ;
	}
	//
	// 逆 DCT 変換を施す
	//
	ERISA::sclfFastIDCT
		( m_ptrInternalBuf, m_ptrMatrixBuf,
			1, m_ptrWorkBuf, m_nSubbandDegree ) ;
	//
	// ストア
	//
	if ( nSamples != 0 )
	{
		ERISA::sclfRoundR32ToWordArray
			( ptrDst, m_mioih.dwChannelCount,
						m_ptrInternalBuf, nSamples ) ;
	}
	return	errSuccess ;
}

// リードブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeLeadBlock( void )
{
	//
	// 逆量子化
	//
	int32_t	nWeightCode = *(m_ptrNextWeight ++) ;
	int		nCoefficient = *(m_ptrNextCoefficient ++) ;
	size_t	i ;
	size_t	nHalfDegree = m_nDegreeNum / 2 ;
	int *	ptrSrcBuf = (int*) m_ptrBuffer1 ;
	int *	ptrNextSrc = ptrSrcBuf ;
	for ( i = 0; i < nHalfDegree; i ++ )
	{
		ptrNextSrc[0] = 0 ;
		ptrNextSrc[1] = *(m_ptrNextSource ++) ;
		ptrNextSrc += 2 ;
	}
	IQuantumize
		( m_ptrLastDCTBuf, ptrSrcBuf,
			m_nDegreeNum, nWeightCode, nCoefficient ) ;
	//
	// 重複パラメータを設定する
	//
	ERISA::sclfOddGivensInverseMatrix
		( m_ptrLastDCTBuf, m_pRevolveParam, m_nSubbandDegree ) ;
	//
	float32_t *	ptrNextDCTBuf = m_ptrLastDCTBuf ;
	for ( i = 0; i < m_nDegreeNum; i += 2 )
	{
		ptrNextDCTBuf[0] = ptrNextDCTBuf[1] ;
		ptrNextDCTBuf += 2 ;
	}
	//
	ERISA::sclfFastIPLOT
		( m_ptrLastDCTBuf, m_nSubbandDegree ) ;
	//
	return	errSuccess ;
}

// ポストブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodePostBlock
	( int16_t * ptrDst, size_t nSamples )
{
	//
	// 逆量子化
	//
	int32_t	nWeightCode = *(m_ptrNextWeight ++) ;
	int		nCoefficient = *(m_ptrNextCoefficient ++) ;
	size_t	i ;
	size_t	nHalfDegree = m_nDegreeNum / 2 ;
	int *	ptrSrcBuf = (int*) m_ptrBuffer1 ;
	int *	ptrNextSrc = ptrSrcBuf ;
	for ( i = 0; i < nHalfDegree; i ++ )
	{
		ptrNextSrc[0] = 0 ;
		ptrNextSrc[1] = *(m_ptrNextSource ++) ;
		ptrNextSrc += 2 ;
	}
	IQuantumize
		( m_ptrMatrixBuf, ptrSrcBuf,
			m_nDegreeNum, nWeightCode, nCoefficient ) ;
	//
	// 逆 LOT を施す
	//
	ERISA::sclfOddGivensInverseMatrix
		( m_ptrMatrixBuf, m_pRevolveParam, m_nSubbandDegree ) ;
	//
	float32_t *	ptrNextMatrixBuf = m_ptrMatrixBuf ;
	for ( i = 0; i < m_nDegreeNum; i += 2 )
	{
		ptrNextMatrixBuf[0] = - ptrNextMatrixBuf[1] ;
		ptrNextMatrixBuf += 2 ;
	}
	//
	ERISA::sclfFastIPLOT
		( m_ptrMatrixBuf, m_nSubbandDegree ) ;
	ERISA::sclfFastILOT
		( m_ptrWorkBuf, m_ptrLastDCTBuf,
			m_ptrMatrixBuf, m_nSubbandDegree ) ;
	//
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrMatrixBuf[i] = m_ptrWorkBuf[i] ;
	}
	//
	// 逆 DCT 変換を施す
	//
	ERISA::sclfFastIDCT
		( m_ptrInternalBuf, m_ptrMatrixBuf,
			1, m_ptrWorkBuf, m_nSubbandDegree ) ;
	//
	// ストア
	//
	if ( nSamples != 0 )
	{
		ERISA::sclfRoundR32ToWordArray
			( ptrDst, m_mioih.dwChannelCount,
				m_ptrInternalBuf, nSamples ) ;
	}
	return	errSuccess ;
}

// 逆量子化
//////////////////////////////////////////////////////////////////////////////
void SGLSoundDecoder::IQuantumize
	( float32_t * ptrDestination,
		const int * ptrQuantumized, size_t nDegreeNum,
		int32_t nWeightCode, int nCoefficient )
{
	//
	// 関数初期設定
	//
	size_t	i, j ;
	//
	// 係数を算出
	//
	double	rMatrixScale = sqrt( 2.0 / nDegreeNum ) ;
	double	rCoefficient = rMatrixScale * nCoefficient ;
	//
	// 重みテーブルを生成する
	//
	double	rAvgRatio[7] ;
	for ( i= 0; i < 6; i++ )
	{
		rAvgRatio[i] =
			1.0 / pow( 2.0, (((nWeightCode >> (i * 5)) & 0x1F) - 15) * 0.5 ) ;
	}
	rAvgRatio[6] = 1.0 ;
	//
	for ( i = 0; i < m_nFrequencyPoint[0]; i ++ )
	{
		m_ptrWeightTable[i] = (float32_t) rAvgRatio[0] ;
	}
	for ( j = 1; j < 7; j ++ )
	{
		double	a = rAvgRatio[j - 1] ;
		double	k = (rAvgRatio[j] - a)
					/ (m_nFrequencyPoint[j] - m_nFrequencyPoint[j - 1]) ;
		while ( i < m_nFrequencyPoint[j] )
		{
			m_ptrWeightTable[i] =
				(float32_t) (k * (i - m_nFrequencyPoint[j - 1]) + a) ;
			i ++ ;
		}
	}
	while ( i < nDegreeNum )
	{
		m_ptrWeightTable[i ++] = (float32_t) rAvgRatio[6] ;
	}
	//
	float32_t	rOddWeight =
		(float32_t) ((((nWeightCode >> 30) & 0x03) + 0x02) * 0.5) ;
	for ( i = 15; i < nDegreeNum; i += 16 )
	{
		m_ptrWeightTable[i] *= rOddWeight ;
	}
	m_ptrWeightTable[nDegreeNum-1] = (float32_t) nCoefficient ;
	//
	for ( i = 0; i < nDegreeNum; i ++ )
	{
		m_ptrWeightTable[i] = 1.0f / m_ptrWeightTable[i] ;
	}
	//
	// 逆量子化
	//
	for ( i = 0; i < nDegreeNum; i ++ )
	{
		ptrDestination[i] =
			(float32_t) (rCoefficient * m_ptrWeightTable[i] * ptrQuantumized[i]) ;
	}
}

// 16ビットの非可逆展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeSoundDCT_MSS
	( ERISA::SGLDecodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, void * ptrWaveBuf )
{
	//
	// バッファを確保
	//
	const size_t	nDegreeWidth = ((size_t) 1 << m_mioih.dwSubbandDegree) ;
	const size_t	nSampleCount =
						(datahdr.dwSampleCount
								+ nDegreeWidth - 1) & ~(nDegreeWidth - 1) ;
	const size_t	nSubbandCount = (nSampleCount >> m_mioih.dwSubbandDegree) ;
	const size_t	nChannelCount = m_mioih.dwChannelCount ;	// 常に２
	const size_t	nAllSampleCount = nSampleCount * nChannelCount ;
	const size_t	nAllSubbandCount = nSubbandCount ;
	size_t			i, j, k ;
	//
	if ( nSampleCount > m_nBufLength )
	{
		m_bufWork2.FreeArray() ;
		m_bufWork2.SetLength
			( nAllSampleCount * sizeof(int)
				+ nAllSampleCount * sizeof(int16_t)
				+ nAllSubbandCount * sizeof(uint8_t)
				+ nAllSubbandCount * 10 * sizeof(uint8_t)
				+ nAllSubbandCount * 10 * sizeof(int32_t)
				+ nAllSubbandCount * 10 * sizeof(int) ) ;
		//
		uint8_t *	ptrWorkNext = m_bufWork2.GetArray() ;
		m_ptrBuffer2 = ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int) ;
		//
		m_ptrBuffer3 = (int8_t*) ptrWorkNext ;
		ptrWorkNext += nAllSampleCount * sizeof(int16_t) ;
		//
		m_ptrDivisionTable = (uint8_t*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * sizeof(uint8_t) ;
		//
		m_ptrRevolveCode = (uint8_t*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * 10 * sizeof(uint8_t) ;
		//
		m_ptrWeightCode = (int32_t*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * 10 * sizeof(int32_t) ;
		//
		m_ptrCoefficient = (int*) ptrWorkNext ;
		ptrWorkNext += nAllSubbandCount * 10 * sizeof(int) ;
		//
		m_nBufLength = nSampleCount ;
	}
	//
	// 量子化係数テーブルを復号
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;
	}
	unsigned int	nLastDivision = (unsigned int) -1 ;
	m_ptrNextDivision = m_ptrDivisionTable ;
	m_ptrNextRevCode = m_ptrRevolveCode ;
	m_ptrNextWeight = m_ptrWeightCode ;
	m_ptrNextCoefficient = m_ptrCoefficient ;
	//
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		unsigned int	nDivisionCode = bstream.GetNBits( 2 ) ;
		*(m_ptrNextDivision ++) = (uint8_t) nDivisionCode ;
		//
		bool	fLeadBlock = false ;
		if ( nDivisionCode != nLastDivision )
		{
			if ( i != 0 )
			{
				*(m_ptrNextRevCode ++) = (uint8_t) bstream.GetNBits( 2 ) ;
				*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
				*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
			}
			fLeadBlock = true ;
			nLastDivision = nDivisionCode ;
		}
		//
		size_t	nDivisionCount = ((size_t) 1 << nDivisionCode) ;
		for ( k = 0; k < nDivisionCount; k ++ )
		{
			if ( fLeadBlock )
			{
				*(m_ptrNextRevCode ++) = (uint8_t) bstream.GetNBits( 2 ) ;
				fLeadBlock = false ;
			}
			else
			{
				*(m_ptrNextRevCode ++) = (uint8_t) bstream.GetNBits( 4 ) ;
			}
			*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
			*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
		}
	}
	if ( nSubbandCount > 0 )
	{
		*(m_ptrNextRevCode ++) = (uint8_t) bstream.GetNBits( 2 ) ;
		*(m_ptrNextWeight ++) = bstream.GetNBits( 32 ) ;
		*(m_ptrNextCoefficient ++) = bstream.GetNBits( 16 ) ;
	}
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;
	}
	SGLAbstractDecodeContext *	context = NULL ;
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		context = &m_ctxGamma ;
		m_ctxGamma.AttachBitStream( &bstream ) ;
		m_ctxGamma.InitGammaContext( ) ;
	}
	else if ( datahdr.bytFlags & mioDataLeadBlock )
	{
		if ( m_mioih.dwArchitecture != erisaNemesisCode )
		{
			m_ctxHuffman.PrepareToDecodeERINACode( ) ;
			context = &m_ctxHuffman ;
		}
		else
		{
			m_ctxNemesis.PrepareToDecodeERISACode( ) ;
			context = &m_ctxNemesis ;
		}
	}
	else if ( m_mioih.dwArchitecture == erisaNemesisCode )
	{
		m_ctxNemesis.InitializeERISACode( ) ;
		context = &m_ctxNemesis ;
	}
	else
	{
		context = &m_ctxHuffman ;
	}
	context->AttachBitStream( &bstream ) ;
	//
	// 復号して逆インターリブ
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		if ( m_ctxGamma.DecodeGammaCodeWords
				( (SWORD*) m_ptrBuffer3, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		for ( i = 0; i < nAllSampleCount; i ++ )
		{
			((int*)m_ptrBuffer2)[i] = ((int16_t*)m_ptrBuffer3)[i] ;
		}
	}
	else if ( m_mioih.dwArchitecture != erisaNemesisCode )
	{
		if ( context->Read
			( m_ptrBuffer3, nAllSampleCount * 2 ) < nAllSampleCount * 2 )
		{
			return	errFailed ;			// エラー
		}
		//
		int8_t *	ptrHBuf = m_ptrBuffer3 ;
		int8_t *	ptrLBuf = m_ptrBuffer3 + nAllSampleCount ;
		//
		const size_t	nDegreeWidthX2 = nDegreeWidth * 2 ;
		for ( i = 0; i < nDegreeWidthX2; i ++ )
		{
			int *	ptrQuantumized = ((int*) m_ptrBuffer2) + i ;
			//
			for ( j = 0; j < nAllSubbandCount; j ++ )
			{
				int	nLow = *(ptrLBuf ++) ;
				int	nHigh = *(ptrHBuf ++) ^ (nLow >> 8) ;
				*ptrQuantumized = (nLow & 0xFF) | (nHigh << 8) ;
				ptrQuantumized += nDegreeWidthX2 ;
			}
		}
	}
	else
	{
		if ( m_ctxNemesis.DecodeERISACodeWords
				( (SWORD*) m_ptrBuffer3, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		for ( i = 0; i < nAllSampleCount; i ++ )
		{
			((int*)m_ptrBuffer2)[i] = ((int16_t*)m_ptrBuffer3)[i] ;
		}
	}
	//
	// サブバンド単位で逆 DCT 変換を施す
	//
	size_t		nSamples ;
	size_t		nRestSamples = datahdr.dwSampleCount ;
	int16_t *	ptrDstBuf = (int16_t*) ptrWaveBuf ;
	//
	nLastDivision = -1 ;
	m_ptrNextDivision = m_ptrDivisionTable ;
	m_ptrNextRevCode = m_ptrRevolveCode ;
	m_ptrNextWeight = m_ptrWeightCode ;
	m_ptrNextCoefficient = m_ptrCoefficient ;
	m_ptrNextSource = (PINT) m_ptrBuffer2 ;
	//
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		//
		// 分解コードを取得
		//
		unsigned int	nDivisionCode = *(m_ptrNextDivision ++) ;
		size_t			nDivisionCount = ((size_t) 1 << nDivisionCode) ;
		//
		// 行列サイズが変化する際の処理
		//
		bool	fLeadBlock = false ;
		if ( nLastDivision != nDivisionCode )
		{
			//
			// 直前までの行列を完成させる
			//
			if ( i != 0 )
			{
				nSamples = nRestSamples ;
				if ( nSamples > m_nDegreeNum )
				{
					nSamples = m_nDegreeNum ;
				}
				if ( DecodePostBlock_MSS( ptrDstBuf, nSamples ) )
				{
					return	errFailed ;
				}
				nRestSamples -= nSamples ;
				ptrDstBuf += nSamples * nChannelCount ;
			}
			//
			// 行列サイズを変化させるためのパラメータをセットアップ
			//
			InitializeWithDegree
				( m_mioih.dwSubbandDegree - nDivisionCode ) ;
			nLastDivision = nDivisionCode ;
			fLeadBlock = true ;
		}
		//
		// 順次逆 LOT 変換を施す
		//
		for ( k = 0; k < nDivisionCount; k ++ )
		{
			if ( fLeadBlock )
			{
				//
				// リードブロックを復号する
				//
				if ( DecodeLeadBlock_MSS( ) )
				{
					return	errFailed ;
				}
				fLeadBlock = false ;
			}
			else
			{
				//
				// 通常ブロックを復号する
				//
				nSamples = nRestSamples ;
				if ( nSamples > m_nDegreeNum )
				{
					nSamples = m_nDegreeNum ;
				}
				if ( DecodeInternalBlock_MSS( ptrDstBuf, nSamples ) )
				{
					return	errFailed ;
				}
				nRestSamples -= nSamples ;
				ptrDstBuf += nSamples * nChannelCount ;
			}
		}
	}
	//
	// 行列を完成させる
	//
	if ( nSubbandCount > 0 )
	{
		nSamples = nRestSamples ;
		if ( nSamples > m_nDegreeNum )
		{
			nSamples = m_nDegreeNum ;
		}
		if ( DecodePostBlock_MSS( ptrDstBuf, nSamples ) )
		{
			return	errFailed ;
		}
		nRestSamples -= nSamples ;
		ptrDstBuf += nSamples * nChannelCount ;
	}
	return	errSuccess ;
}

// 通常のブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeInternalBlock_MSS
	( int16_t * ptrDst, size_t nSamples )
{
	size_t		i, j ;
	float32_t *	ptrSrcBuf = m_ptrMatrixBuf ;
	float32_t *	ptrLapBuf = m_ptrLastDCT ;
	//
	int32_t	nWeightCode = *(m_ptrNextWeight ++) ;
	int		nCoefficient = *(m_ptrNextCoefficient ++) ;
	//
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 逆量子化
		//
		IQuantumize
			( ptrSrcBuf, m_ptrNextSource,
				m_nDegreeNum, nWeightCode, nCoefficient ) ;
		m_ptrNextSource += m_nDegreeNum ;
		ptrSrcBuf += m_nDegreeNum ;
	}
	//
	// 回転変換
	//
	float32_t	rSin, rCos ;
	int			nRevCode = *(m_ptrNextRevCode ++) ;
	int			nRevCode1 = (nRevCode >> 2) & 0x03 ;
	int			nRevCode2 = (nRevCode & 0x03) ;
	//
	float32_t *	ptrSrcBuf1 = m_ptrMatrixBuf ;
	float32_t *	ptrSrcBuf2 = m_ptrMatrixBuf + m_nDegreeNum ;
	//
	double	rPIby8 = SSystem::PI * 0.125 ;
	rSin = (float32_t) sin( nRevCode1 * rPIby8 ) ;
	rCos = (float32_t) cos( nRevCode1 * rPIby8 ) ;
	ERISA::sclfRevolve2x2
		( ptrSrcBuf1, ptrSrcBuf2, rSin, rCos, 2, m_nDegreeNum / 2 ) ;
	//
	rSin = (float32_t) sin( nRevCode2 * rPIby8 ) ;
	rCos = (float32_t) cos( nRevCode2 * rPIby8 ) ;
	ERISA::sclfRevolve2x2
		( ptrSrcBuf1 + 1, ptrSrcBuf2 + 1, rSin, rCos, 2, m_nDegreeNum / 2 ) ;
	//
	ptrSrcBuf = m_ptrMatrixBuf ;
	//
	const size_t	nDegreeNum = m_nDegreeNum ;
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 逆 LOT を施す
		//
		ERISA::sclfOddGivensInverseMatrix
			( ptrSrcBuf, m_pRevolveParam, m_nSubbandDegree ) ;
		ERISA::sclfFastIPLOT
			( ptrSrcBuf, m_nSubbandDegree ) ;
		ERISA::sclfFastILOT
			( m_ptrWorkBuf, ptrLapBuf, ptrSrcBuf, m_nSubbandDegree ) ;
		//
		for ( j = 0; j < nDegreeNum; j ++ )
		{
			ptrLapBuf[j] = ptrSrcBuf[j] ;
			ptrSrcBuf[j] = m_ptrWorkBuf[j] ;
		}
		//
		// 逆 DCT 変換を施す
		//
		ERISA::sclfFastIDCT
			( m_ptrInternalBuf, ptrSrcBuf,
				1, m_ptrWorkBuf, m_nSubbandDegree ) ;
		//
		// ストア
		//
		if ( nSamples != 0 )
		{
			ERISA::sclfRoundR32ToWordArray
				( ptrDst + i, 2, m_ptrInternalBuf, nSamples ) ;
		}
		ptrSrcBuf += nDegreeNum ;
		ptrLapBuf += nDegreeNum ;
	}
	return	errSuccess ;
}

// リードブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodeLeadBlock_MSS( void )
{
	size_t		i, j ;
	size_t		nHalfDegree = m_nDegreeNum / 2 ;
	int32_t		nWeightCode = *(m_ptrNextWeight ++) ;
	int			nCoefficient = *(m_ptrNextCoefficient ++) ;
	float32_t *	ptrLapBuf = m_ptrLastDCT ;
	//
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 逆量子化
		//
		int *	ptrSrcBuf = (int*) m_ptrBuffer1 ;
		int *	ptrNextSrc = ptrSrcBuf ;
		for ( j = 0; j < nHalfDegree; j ++ )
		{
			ptrNextSrc[0] = 0 ;
			ptrNextSrc[1] = *(m_ptrNextSource ++) ;
			ptrNextSrc += 2 ;
		}
		IQuantumize
			( ptrLapBuf, ptrSrcBuf,
				m_nDegreeNum, nWeightCode, nCoefficient ) ;
		//
		ptrLapBuf += m_nDegreeNum ;
	}
	//
	// 回転変換
	//
	float32_t	rSin, rCos ;
	int			nRevCode = *(m_ptrNextRevCode ++) ;
	//
	float32_t *	ptrLapBuf1 = m_ptrLastDCT ;
	float32_t *	ptrLapBuf2 = m_ptrLastDCT + m_nDegreeNum ;
	//
	double	rPIby8 = SSystem::PI * 0.125 ;
	rSin = (float32_t) sin( nRevCode * rPIby8 ) ;
	rCos = (float32_t) cos( nRevCode * rPIby8 ) ;
	ERISA::sclfRevolve2x2
		( ptrLapBuf1, ptrLapBuf2, rSin, rCos, 1, m_nDegreeNum ) ;
	//
	ptrLapBuf = m_ptrLastDCT ;
	//
	const size_t	nDegreeNum = m_nDegreeNum ;
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 重複パラメータを設定する
		//
		ERISA::sclfOddGivensInverseMatrix
			( ptrLapBuf, m_pRevolveParam, m_nSubbandDegree ) ;
		//
		float32_t *	ptrNextLapBuf = ptrLapBuf ;
		for ( j = 0; j < nDegreeNum; j += 2 )
		{
			ptrNextLapBuf[0] = ptrNextLapBuf[1] ;
			ptrNextLapBuf += 2 ;
		}
		//
		ERISA::sclfFastIPLOT( ptrLapBuf, m_nSubbandDegree ) ;
		//
		ptrLapBuf += nDegreeNum ;
	}
	return	errSuccess ;
}

// ポストブロックを復号する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundDecoder::DecodePostBlock_MSS
	( int16_t * ptrDst, size_t nSamples )
{
	float32_t *	ptrLapBuf = m_ptrLastDCT ;
	float32_t *	ptrSrcBuf = m_ptrMatrixBuf ;
	//
	size_t	i, j ;
	size_t	nHalfDegree = m_nDegreeNum / 2 ;
	int32_t	nWeightCode = *(m_ptrNextWeight ++) ;
	int		nCoefficient = *(m_ptrNextCoefficient ++) ;
	//
	const size_t	nDegreeNum = m_nDegreeNum ;
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 逆量子化
		//
		int *	ptrTempBuf = (int*) m_ptrBuffer1 ;
		int *	ptrNextTemp = ptrTempBuf ;
		for ( j = 0; j < nHalfDegree; j ++ )
		{
			ptrNextTemp[0] = 0 ;
			ptrNextTemp[1] = *(m_ptrNextSource ++) ;
			ptrNextTemp += 2 ;
		}
		IQuantumize
			( ptrSrcBuf, ptrTempBuf,
				nDegreeNum, nWeightCode, nCoefficient ) ;
		//
		ptrSrcBuf += nDegreeNum ;
	}
	//
	// 回転変換
	//
	float32_t	rSin, rCos ;
	int			nRevCode = *(m_ptrNextRevCode ++) ;
	//
	float32_t *	ptrSrcBuf1 = m_ptrMatrixBuf ;
	float32_t *	ptrSrcBuf2 = m_ptrMatrixBuf + nDegreeNum ;
	//
	double	rPIby8 = SSystem::PI * 0.125 ;
	rSin = (float32_t) sin( nRevCode * rPIby8 ) ;
	rCos = (float32_t) cos( nRevCode * rPIby8 ) ;
	ERISA::sclfRevolve2x2
		( ptrSrcBuf1, ptrSrcBuf2, rSin, rCos, 1, nDegreeNum ) ;
	//
	ptrSrcBuf = m_ptrMatrixBuf ;
	//
	for ( i = 0; i < 2; i ++ )
	{
		//
		// 逆 LOT を施す
		//
		ERISA::sclfOddGivensInverseMatrix
			( ptrSrcBuf, m_pRevolveParam, m_nSubbandDegree ) ;
		//
		float32_t *	ptrNextSrc = ptrSrcBuf ;
		for ( j = 0; j < nDegreeNum; j += 2 )
		{
			ptrNextSrc[0] = - ptrNextSrc[1] ;
			ptrNextSrc += 2 ;
		}
		//
		ERISA::sclfFastIPLOT
			( ptrSrcBuf, m_nSubbandDegree ) ;
		ERISA::sclfFastILOT
			( m_ptrWorkBuf, ptrLapBuf, ptrSrcBuf, m_nSubbandDegree ) ;
		//
		for ( j = 0; j < nDegreeNum; j ++ )
		{
			ptrSrcBuf[j] = m_ptrWorkBuf[j] ;
		}
		//
		// 逆 DCT 変換を施す
		//
		ERISA::sclfFastIDCT
			( m_ptrInternalBuf, ptrSrcBuf,
				1, m_ptrWorkBuf, m_nSubbandDegree ) ;
		//
		// ストア
		//
		if ( nSamples != 0 )
		{
			ERISA::sclfRoundR32ToWordArray
				( ptrDst + i, 2, m_ptrInternalBuf, nSamples ) ;
		}
		ptrLapBuf += nDegreeNum ;
		ptrSrcBuf += nDegreeNum ;
	}
	return	errSuccess ;
}


