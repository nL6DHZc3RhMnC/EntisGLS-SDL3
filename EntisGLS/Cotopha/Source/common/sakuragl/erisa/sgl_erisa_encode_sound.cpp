
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2015 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// SGLSoundEncoder::Parameter
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundEncoder::Parameter::Parameter( void )
{
	fpLowWeight = 4.0 ;
	fpMiddleWeight = 3.0 ;
	fpPowerScale = 0.5 ;
	nOddWeight = 1 ;
	nPreEchoThreshold = 2 ;
}

// プリセット値取得
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::Parameter::LoadPresetParam
	( PresetParameter ppIndex, ERISA::MIO_INFO_HEADER & infhdr )
{
	struct	PRESET_PARAMETER
	{
		double		low_weight ;
		double		middle_weight ;
		int			odd_weight ;
		int			pe_threshold ;
		signed int	power_scale ;
		int			matrix_degree ;
		int			use_mss ;
	} ;
	static const PRESET_PARAMETER	preset[ppMax] =
	{
		{	4.0,	3.0,	0,	2,	256,	 9,	0	},			// 1/6
		{	4.0,	3.0,	0,	2,	140,	10,	0	},			// 1/8
		{	4.0,	3.0,	0,	2,	108,	10,	1	},			// 1/9
		{	4.0,	3.0,	1,	2,	95,		10,	1	},			// 1/10
		{	4.0,	3.0,	1,	2,	85,		10,	1	},			// 1/11
		{	4.0,	3.0,	1,	2,	75,		10,	1	},			// 1/12
		{	4.0,	3.0,	2,	4,	57,		11,	1	},			// 1/15
		{	4.0,	3.0,	2,	6,	49,		11,	1	},			// 1/18
		{	4.0,	3.0,	2,	8,	42,		12,	1	}			// 1/20
	} ;
	//
	fpLowWeight = preset[ppIndex].low_weight ;
	fpMiddleWeight = preset[ppIndex].middle_weight ;
	fpPowerScale = preset[ppIndex].power_scale / 256.0 ;
	nOddWeight = preset[ppIndex].odd_weight ;
	nPreEchoThreshold = preset[ppIndex].pe_threshold ;
	//
	infhdr.dwVersion = 0x00020300 ;
	if ( preset[ppIndex].use_mss && (infhdr.dwChannelCount == 2) )
		infhdr.fdwTransformation = eriTransformationLOT_MSS ;
	else
		infhdr.fdwTransformation = eriTransformationLOT ;
	infhdr.dwArchitecture = eriRunlengthGamma ;
	infhdr.dwBlocksetCount = 0 ;
	infhdr.dwSubbandDegree = preset[ppIndex].matrix_degree ;
	infhdr.dwLappedDegree = 1 ;
}


//////////////////////////////////////////////////////////////////////////////
// 音声圧縮オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLSoundEncoder, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundEncoder::SGLSoundEncoder( void )
	: m_ctxGamma( NULL ), m_ctxHuffman( NULL ), m_ctxNemesis( NULL )
{
	m_nBufLength = 0 ;
	m_ptrBuffer1 = NULL ;
	m_ptrBuffer2 = NULL ;
	m_ptrBuffer3 = NULL ;
	m_ptrSamplingBuf = NULL ;
	m_ptrInternalBuf = NULL ;
	m_ptrDstBuf = NULL ;
	m_ptrWorkBuf = NULL ;
	m_ptrWeightTable = NULL ;
	m_ptrLastDCT = NULL ;
	m_pRevolveParam = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundEncoder::~SGLSoundEncoder( void )
{
	Delete() ;
}

// 初期化（パラメータの設定）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::Initialize
	( const ERISA::MIO_INFO_HEADER & infhdr )
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
		// 圧縮パラメータ初期化
		//
		Parameter	parameter ;
		SetCompressionParameter( parameter ) ;
		//
		// DCT 用バッファ確保
		//
		m_ptrBuffer1 =
			m_bufBuffer1.GetArray
				( m_mioih.dwChannelCount
					* (sizeof(float32_t) << m_mioih.dwSubbandDegree) ) ;
		m_ptrSamplingBuf =
			m_bufSamplingBuf.GetArray
				( m_mioih.dwChannelCount << m_mioih.dwSubbandDegree ) ;
		m_ptrInternalBuf =
			m_bufInternalBuf.GetArray
				( (size_t) 1 << m_mioih.dwSubbandDegree ) ;
		m_ptrDstBuf =
			m_bufDstBuf.GetArray
				( m_mioih.dwChannelCount << m_mioih.dwSubbandDegree ) ;
		m_ptrWorkBuf =
			m_bufWorkBuf.GetArray
				( (size_t) 1 << m_mioih.dwSubbandDegree ) ;
		//
		// 重みテーブルを確保
		//
		m_ptrWeightTable =
			m_bufWeightTable.GetArray
				( (size_t) 1 << m_mioih.dwSubbandDegree ) ;
		//
		// LOT 用バッファ確保
		//
		size_t	i, nBlocksetSamples, nLappedSamples ;
		nBlocksetSamples = m_mioih.dwChannelCount << m_mioih.dwSubbandDegree ;
		nLappedSamples = nBlocksetSamples * m_mioih.dwLappedDegree ;
		if ( nLappedSamples > 0 )
		{
			m_ptrLastDCT = m_bufLastDCT.GetArray( nLappedSamples ) ;
			for ( i = 0; i < nLappedSamples; i ++ )
			{
				m_ptrLastDCT[i] = 0.0F ;
			}
		}
		//
		// パラメータ設定
		//
		InitializeWithDegree( m_mioih.dwSubbandDegree ) ;
	}
	else
	{
		return	errFailed ;		// エラー（未対応の圧縮フォーマット）
	}
	//
	// 正常終了
	//
	return	errSuccess ;
}

// 終了（メモリの解放など）
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::Delete( void )
{
	if ( m_ptrBuffer1 != NULL )
	{
		m_bufBuffer1.FreeArray() ;
		m_ptrBuffer1 = NULL ;
	}
	if ( m_ptrBuffer2 != NULL )
	{
		m_bufBuffer2.FreeArray() ;
		m_ptrBuffer2 = NULL ;
	}
	if ( m_ptrBuffer3 != NULL )
	{
		m_bufBuffer3.FreeArray() ;
		m_ptrBuffer3 = NULL ;
	}
	if ( m_ptrSamplingBuf != NULL )
	{
		m_bufSamplingBuf.FreeArray() ;
		m_ptrSamplingBuf = NULL ;
	}
	if ( m_ptrInternalBuf != NULL )
	{
		m_bufInternalBuf.FreeArray() ;
		m_ptrInternalBuf = NULL ;
	}
	if ( m_ptrDstBuf != NULL )
	{
		m_bufDstBuf.FreeArray() ;
		m_ptrDstBuf = NULL ;
	}
	if ( m_ptrWorkBuf != NULL )
	{
		m_bufWorkBuf.FreeArray() ;
		m_ptrWorkBuf = NULL ;
	}
	if ( m_ptrWeightTable != NULL )
	{
		m_bufWeightTable.FreeArray() ;
		m_ptrWeightTable = NULL ;
	}
	if ( m_ptrLastDCT != NULL )
	{
		m_bufLastDCT.FreeArray() ;
		m_ptrLastDCT = NULL ;
	}
	m_pRevolveParam = NULL ;
	m_nBufLength = 0 ;
}

// 音声を圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeSound
	( ERISA::SGLEncodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf )
{
	if ( m_mioih.fdwTransformation == eriTransformationLossless )
	{
		if ( m_mioih.dwBitsPerSample == 8 )
		{
			return	EncodeSoundPCM8( bstream, datahdr, ptrWaveBuf ) ;
		}
		else if ( m_mioih.dwBitsPerSample == 16 )
		{
			return	EncodeSoundPCM16( bstream, datahdr, ptrWaveBuf ) ;
		}
	}
	else if ( (m_mioih.fdwTransformation == eriTransformationLOT)
			|| (m_mioih.fdwTransformation == eriTransformationLOT_MSS) )
	{
		if ( m_mioih.dwBitsPerSample == 16 )
		{
			if ( (m_mioih.dwChannelCount != 2) ||
					(m_mioih.fdwTransformation == eriTransformationLOT) )
			{
				return	EncodeSoundDCT( bstream, datahdr, ptrWaveBuf ) ;
			}
			else
			{
				return	EncodeSoundDCT_MSS( bstream, datahdr, ptrWaveBuf ) ;
			}
		}
	}
	return	errFailed ;		// エラー
}

// 圧縮オプションを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::SetCompressionParameter
		( const SGLSoundEncoder::Parameter & parameter )
{
	m_parameter = parameter ;
}

// 8ビットのPCMを圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeSoundPCM8
	( ERISA::SGLEncodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf )
{
	//
	// 演算用バッファを確保
	//
	size_t	nSampleCount = datahdr.dwSampleCount ;
	if ( nSampleCount > m_nBufLength )
	{
		m_ptrBuffer1 = m_bufBuffer1.GetArray
							( nSampleCount * m_mioih.dwChannelCount ) ;
		m_nBufLength = nSampleCount ;
	}
	//
	// チャネルごとにサンプリングして差分処理
	//
	uint8_t *	ptrDstBuf = m_ptrBuffer1 ;
	uint8_t *	ptrSrcBuf ;
	size_t		nStep = m_mioih.dwChannelCount ;
	size_t		i, j ;
	for ( i = 0; i < m_mioih.dwChannelCount; i ++ )
	{
		ptrSrcBuf = (uint8_t*) ptrWaveBuf ;
		ptrSrcBuf += i ;
		//
		uint8_t	bytLeftVal = 0 ;
		for ( j = 0; j < nSampleCount; j ++ )
		{
			uint8_t	bytCurVal = *ptrSrcBuf ;
			ptrSrcBuf += nStep ;
			*(ptrDstBuf ++) = bytCurVal - bytLeftVal ;
			bytLeftVal = bytCurVal ;
		}
	}
	//
	// ハフマン符号で符号化
	//
	m_ctxHuffman.AttachBitStream( &bstream ) ;
	if ( datahdr.bytFlags & ERISA::mioDataLeadBlock )
	{
		m_ctxHuffman.PrepareToEncodeERINACode( ) ;
	}
	size_t	nBytes = nSampleCount * m_mioih.dwChannelCount ;
	if ( m_ctxHuffman.EncodeERINACodeBytes
			( (const SBYTE *) m_ptrBuffer1, nBytes ) < nBytes )
	{
		return	errFailed ;			// エラー
	}
	m_ctxHuffman.FinishEncoding( ) ;
	return	errSuccess ;
}

// 16ビットのPCMを圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeSoundPCM16
	( ERISA::SGLEncodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf )
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
		m_ptrBuffer1 =
			m_bufBuffer1.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		m_ptrBuffer2 =
			m_bufBuffer2.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		//
		m_nBufLength = nSampleCount ;
	}
	//
	// チャネルごとにサンプリングして差分処理
	//
	int16_t *	ptrDstBuf = (int16_t*) m_ptrBuffer1 ;
	int16_t *	ptrSrcBuf ;
	size_t		nStep = nChannelCount ;
	for ( i = 0; i < nChannelCount; i ++ )
	{
		ptrSrcBuf = (int16_t*) ptrWaveBuf ;
		ptrSrcBuf += i ;
		//
		int16_t	wLeftVal = 0 ;
		int16_t	wLastDelta = 0 ;
		for ( j = 0; j < nSampleCount; j ++ )
		{
			int16_t	wCurVal = *ptrSrcBuf ;
			ptrSrcBuf += nStep ;
			int16_t	wCurDelta = wCurVal - wLeftVal ;
			ptrDstBuf[j] = wCurDelta - wLastDelta ;
			wLeftVal = wCurVal ;
			wLastDelta = wCurDelta ;
		}
		ptrDstBuf += nSampleCount ;
	}
	//
	// ワードの上位と下位を分離して整列
	//
	uint8_t *	pbytDstBuf1 ;
	uint8_t *	pbytDstBuf2 ;
	uint8_t *	pbytSrcBuf ;
	for ( i = 0; i < nChannelCount; i ++ )
	{
		size_t	nOffset = i * nSampleCount * sizeof(int16_t) ;
		pbytDstBuf1 = m_ptrBuffer2 + nOffset ;
		pbytDstBuf2 = pbytDstBuf1 + nSampleCount ;
		pbytSrcBuf = m_ptrBuffer1 + nOffset ;
		//
		for ( j = 0; j < nSampleCount; j ++ )
		{
			int8_t	bytLow = (int8_t) pbytSrcBuf[j * sizeof(int16_t) + 0] ;
			int8_t	bytHigh = (int8_t) pbytSrcBuf[j * sizeof(int16_t) + 1] ;
			pbytDstBuf2[j] = bytLow ;
			pbytDstBuf1[j] = bytHigh ^ (bytLow >> 7) ;
		}
	}
	//
	// ハフマン符号で符号化
	//
	m_ctxHuffman.AttachBitStream( &bstream ) ;
	if ( datahdr.bytFlags & ERISA::mioDataLeadBlock )
	{
		m_ctxHuffman.PrepareToEncodeERINACode( ) ;
	}
	size_t	nBytes = nSampleCount * nChannelCount * sizeof(int16_t) ;
	if ( m_ctxHuffman.EncodeERINACodeBytes
			( (const SBYTE *) m_ptrBuffer1, nBytes ) < nBytes )
	{
		return	errFailed ;			// エラー
	}
	m_ctxHuffman.FinishEncoding( ) ;
	return	errSuccess ;
}

// 行列サイズの変更に伴うパラメータの再計算
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::InitializeWithDegree( size_t nSubbandDegree )
{
	//
	// 回転パラメータ生成
	//
	m_pRevolveParam = ERISA::sclfGetRevolveParameter( nSubbandDegree ) ;
	//
	// 量子化用パラメータ設定
	//
	static const int	freq_width[7] =
	{
		-6, -6, -5, -4, -3, -2, -1
	} ;
	for ( int i = 0, j = 0; i < 7; i ++ )
	{
		m_nFrequencyWidth[i] = (size_t) 1 << (nSubbandDegree + freq_width[i]) ;
		m_nFrequencyPoint[i] = j + (m_nFrequencyWidth[i] / 2) ;
		j += (int) m_nFrequencyWidth[i] ;
	}
	//
	// ローカルパラメータを設定
	//
	m_nSubbandDegree = nSubbandDegree ;
	m_nDegreeNum = ((size_t) 1 << nSubbandDegree) ;
}

// 指定サンプル列の音量を求める
//////////////////////////////////////////////////////////////////////////////
double SGLSoundEncoder::EvaluateVolume( const float32_t * ptrWave, size_t nCount )
{
	double	fpVolume = 0.0 ;
	for ( size_t i = 1; i < nCount; i ++ )
	{
		fpVolume += fabs( ptrWave[i] - ptrWave[i - 1] ) ;
	}
	fpVolume /= (nCount - 1) ;
	return	fpVolume ;
}

// 分解コードを取得する
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundEncoder::GetDivisionCode( const float32_t * ptrSamples )
{
	double	fpSum = 0.0 ;
	double	fpAvg = 0.0 ;
	size_t	nDegreeWidth = ((size_t) 1 << m_mioih.dwSubbandDegree) ;
	size_t	nCount = nDegreeWidth / 64 ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		double	fpVol = EvaluateVolume( ptrSamples, 64 ) ;
		if ( (i >= 1) && (fpVol >= fpAvg * m_parameter.nPreEchoThreshold) )
		{
			return	2 ;
		}
		ptrSamples += 64 ;
		fpSum += fpVol ;
		fpAvg = fpSum / (i + 1) ;
	}
	return	0 ;
}

// 16ビットの非可逆圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeSoundDCT
	( ERISA::SGLEncodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf )
{
	//
	// バッファを確保
	//
	size_t	nDegreeWidth = ((size_t) 1 << m_mioih.dwSubbandDegree) ;
	size_t	nSampleCount =
				(datahdr.dwSampleCount + nDegreeWidth - 1)
										& ~(nDegreeWidth - 1) ;
	size_t	nSubbandCount =
				(nSampleCount >> m_mioih.dwSubbandDegree) ;
	size_t	nChannelCount = m_mioih.dwChannelCount ;
	size_t	nAllSampleCount = nSampleCount * nChannelCount ;
	//
	if ( nSampleCount > m_nBufLength )
	{
		m_ptrBuffer2 =
			m_bufBuffer2.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		m_ptrBuffer3 =
			m_bufBuffer3.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		m_nBufLength = nSampleCount ;
	}
	//
	m_ptrNextDstBuf = (int16_t*) m_ptrBuffer2 ;
	//
	// 予約ビットを送出
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		m_ctxGamma.AttachBitStream( &bstream ) ;
	}
	else if ( m_mioih.dwArchitecture == erisaNemesisCode )
	{
		m_ctxNemesis.AttachBitStream( &bstream ) ;
		if ( datahdr.bytFlags & mioDataLeadBlock )
		{
			m_ctxNemesis.PrepareToEncodeERISACode( ) ;
		}
	}
	else
	{
		m_ctxHuffman.AttachBitStream( &bstream ) ;
		if ( datahdr.bytFlags & mioDataLeadBlock )
		{
			m_ctxHuffman.PrepareToEncodeERINACode( ) ;
		}
	}
	bstream.OutNBits( 0, 1 ) ;
	//
	// サブバンド単位で分解し、各々DCT変換を施す
	//
	size_t			i, j, k ;
	const int16_t *	ptrSrcBuf = (const int16_t *) ptrWaveBuf ;
	SArray<size_t>	bufLastDiv ;
	size_t *		pLastDivision = bufLastDiv.GetArray( nChannelCount ) ;
	for ( i = 0; i < nChannelCount; i ++ )
	{
		pLastDivision[i] = (size_t) -1 ;
	}
	size_t	nCurrentDivision = (size_t) -1 ;
	//
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		const int16_t *	ptrSubbandHead =
				ptrSrcBuf + (nDegreeWidth * nChannelCount) * i ;
		//
		size_t	nCopySamples =
					datahdr.dwSampleCount - (i * nDegreeWidth) ;
		if ( nCopySamples > nDegreeWidth )
		{
			nCopySamples = nDegreeWidth ;
		}
		//
		for ( j = 0; j < nChannelCount; j ++ )
		{
			//
			// サンプリング
			//
			const int16_t *	ptrSrcHead = ptrSubbandHead + j ;
			//
			for ( k = 0; k < nCopySamples; k ++ )
			{
				m_ptrSamplingBuf[k] = (float32_t) *ptrSrcHead ;
				ptrSrcHead += nChannelCount ;
			}
			while ( k < nDegreeWidth )
			{
				m_ptrSamplingBuf[k ++] = 0.0F ;
			}
			//
			// 重複処理用バッファを取得
			//
			size_t	nChannelStep = nDegreeWidth * m_mioih.dwLappedDegree * j ;
			m_ptrLastDCTBuf = m_ptrLastDCT + nChannelStep ;
			//
			// 分解コードを取得
			//
			float32_t	fpPowerScale = (float32_t) m_parameter.fpPowerScale ;
			size_t		nDivisionCode = GetDivisionCode( m_ptrSamplingBuf ) ;
			size_t		nDivisionCount = ((size_t) 1 << nDivisionCode) ;
			//
			for ( k = 0; k < nDivisionCode; k ++ )
			{
				fpPowerScale *= 1.0625F ;
			}
			//
			bstream.OutNBits( (((DWORD) nDivisionCode) << 30), 2 ) ;
			//
			// 行列サイズが変化する際の処理
			//
			bool	fLeadBlock = false ;
			//
			if ( pLastDivision[j] != nDivisionCode )
			{
				//
				// 直前までの行列を完成させる
				//
				if ( i != 0 )
				{
					if ( nCurrentDivision != pLastDivision[j] )
					{
						InitializeWithDegree
							( m_mioih.dwSubbandDegree - pLastDivision[j] ) ;
						nCurrentDivision = pLastDivision[j] ;
					}
					if ( EncodePostBlock
						( bstream, (float32_t) m_parameter.fpPowerScale ) )
					{
						return	errFailed ;
					}
				}
				//
				// 行列サイズを変化させるためのパラメータをセットアップ
				//
				pLastDivision[j] = nDivisionCode ;
				fLeadBlock = true ;
			}
			if ( nCurrentDivision != nDivisionCode )
			{
				InitializeWithDegree
					( m_mioih.dwSubbandDegree - nDivisionCode ) ;
				nCurrentDivision = nDivisionCode ;
			}
			//
			// 順次 LOT 変換を施す
			//
			float32_t *	ptrNextSamples = m_ptrSamplingBuf ;
			//
			for ( k = 0; k < nDivisionCount; k ++ )
			{
				if ( fLeadBlock )
				{
					//
					// リードブロックを出力する
					//
					if ( EncodeLeadBlock
							( bstream, ptrNextSamples, fpPowerScale ) )
					{
						return	errFailed ;
					}
					fLeadBlock = false ;
				}
				else
				{
					//
					// 通常ブロックを出力する
					//
					if ( EncodeInternalBlock
							( bstream, ptrNextSamples, fpPowerScale ) )
					{
						return	errFailed ;
					}
				}
				//
				// 次へ
				//
				ptrNextSamples += m_nDegreeNum ;
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
			if ( nCurrentDivision != pLastDivision[i] )
			{
				InitializeWithDegree
					( m_mioih.dwSubbandDegree - pLastDivision[i] ) ;
				nCurrentDivision = pLastDivision[i] ;
			}
			if ( EncodePostBlock
				( bstream, (float32_t) m_parameter.fpPowerScale ) )
			{
				return	errFailed ;
			}
		}
	}
	//
	// インターリーブして符号化
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxGamma.EncodeGammaCodeWords
			( (const SWORD *) m_ptrBuffer2, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		m_ctxGamma.FinishEncoding() ;
	}
	else if ( m_mioih.dwArchitecture != erisaNemesisCode )
	{
		int8_t *	ptrHBuf = m_ptrBuffer3 ;
		int8_t *	ptrLBuf = m_ptrBuffer3 + nAllSampleCount ;
		size_t		nAllSubbandCount = nSubbandCount * nChannelCount ;
		//
		for ( i = 0; i < nDegreeWidth; i ++ )
		{
			ptrSrcBuf = ((const int16_t *) m_ptrBuffer2) + i ;
			//
			for ( j = 0; j < nAllSubbandCount; j ++ )
			{
				int16_t	nValue = *ptrSrcBuf ;
				ptrSrcBuf += nDegreeWidth ;
				int8_t	nLow = (int8_t)((nValue << 8) >> 8) ;
				int8_t	nHigh = (int8_t)((nValue >> 8) ^ (nLow >> 7)) ;
				*(ptrHBuf ++) = nHigh ;
				*(ptrLBuf ++) = nLow ;
			}
		}
		//
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxHuffman.EncodeERINACodeBytes
			( m_ptrBuffer3, nAllSampleCount * 2 ) < nAllSampleCount * 2 )
		{
			return	errFailed ;			// エラー
		}
		m_ctxHuffman.FinishEncoding( ) ;
	}
	else
	{
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxNemesis.EncodeERISACodeWords
				( (const SWORD *) m_ptrBuffer2,
								nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		m_ctxNemesis.FinishERISACode( ) ;
	}

	return	errSuccess ;
}

// LOT 変換を施す
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::PerformLOT
	( ERISA::SGLEncodeBitStream & bstream,
		float32_t * ptrSamples, float32_t fpPowerScale )
{
	//
	// DCT 変換を施す
	//
	float32_t	fpMatrixScale = (float32_t) sqrt(2.0 / m_nDegreeNum) ;
	sclfFastDCT
		( m_ptrInternalBuf, 1, ptrSamples,
			m_ptrWorkBuf, m_nSubbandDegree ) ;
	sclfScalarMultiply
		( m_ptrInternalBuf, fpMatrixScale, m_nDegreeNum ) ;
	//
	// LOT を施す
	//
	sclfFastPLOT
		( m_ptrInternalBuf, m_nSubbandDegree ) ;
	sclfFastLOT
		( m_ptrWorkBuf, m_ptrLastDCTBuf,
			m_ptrInternalBuf, m_nSubbandDegree ) ;
	sclfOddGivensMatrix
		( m_ptrWorkBuf, m_pRevolveParam, m_nSubbandDegree ) ;
	//
	for ( size_t i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrLastDCTBuf[i] = m_ptrInternalBuf[i] ;
	}
	//
	// 量子化
	//
	DWORD	nWeightCode ;
	int		nCoefficient ;
	Quantumize
		( (int32_t*) m_ptrBuffer1, m_ptrWorkBuf,
			m_nDegreeNum, fpPowerScale,
			&nWeightCode, &nCoefficient ) ;
	//
	// 量子化係数を出力
	//
	bstream.OutNBits( nWeightCode, 32 ) ;
	bstream.OutNBits( (((DWORD) nCoefficient) << 16), 16 ) ;
}

// 通常のブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeInternalBlock
	( ERISA::SGLEncodeBitStream & bstream,
		float32_t * ptrSamples, float32_t fpPowerScale )
{
	//
	// LOT 変換を施す
	//
	PerformLOT( bstream, ptrSamples, fpPowerScale ) ;
	//
	// 符号化
	//
	for ( size_t i = 0; i < m_nDegreeNum; i ++ )
	{
		*(m_ptrNextDstBuf ++) =
				(int16_t) (((const int32_t *) m_ptrBuffer1)[i]) ;
	}
	return	errSuccess ;
}

// リードブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeLeadBlock
	( ERISA::SGLEncodeBitStream & bstream,
		float32_t * ptrSamples, float32_t fpPowerScale )
{
	//
	// 重複係数を初期化
	//
	size_t	i ;
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrLastDCTBuf[i] = 0.0f ;
	}
	//
	// LOT 変換を施す
	//
	PerformLOT( bstream, ptrSamples, fpPowerScale ) ;
	//
	// 符号化
	//
	int32_t *	ptrSrcBuffer = (int32_t*) m_ptrBuffer1 ;
	size_t		nHalfDegree = m_nDegreeNum / 2 ;
	for ( i = 0; i < nHalfDegree; i ++ )
	{
		*(m_ptrNextDstBuf ++) = (int16_t) (ptrSrcBuffer[i * 2 + 1]) ;
	}
	return	errSuccess ;
}

// ポストブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodePostBlock
	( ERISA::SGLEncodeBitStream & bstream, float32_t fpPowerScale )
{
	//
	// ダミーの入力信号を送信する
	//
	size_t		i ;
	float32_t *	ptrSamples = (float32_t*) m_ptrBuffer1 ;
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		ptrSamples[i] = 0.0F ;
	}
	//
	// LOT 変換を施す
	//
	PerformLOT( bstream, ptrSamples, fpPowerScale ) ;
	//
	// 符号化
	//
	int32_t *	ptrSrcBuffer = (int32_t*) m_ptrBuffer1 ;
	size_t		nHalfDegree = m_nDegreeNum / 2 ;
	for ( i = 0; i < nHalfDegree; i ++ )
	{
		*(m_ptrNextDstBuf ++) = (int16_t) (ptrSrcBuffer[i * 2 + 1]) ;
	}
	return	errSuccess ;
}

// 量子化
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::Quantumize
	( int32_t * ptrQuantumized, const float32_t * ptrSource,
		size_t nDegreeNum, float32_t fpPowerScale,
		DWORD * ptrWeightCode, int * ptrCoefficient )
{
	//
	// 関数初期設定
	//
	size_t	i, j ;
	size_t	nHalfDegree = nDegreeNum / 2 ;
	//
	// 各周波数帯のエネルギ集中度から周波数ごとの量子化係数を算出する
	//
	double	fpPresetWeight[7] ;
	fpPresetWeight[0] = fpPresetWeight[1] =
		m_parameter.fpLowWeight
			* pow( m_parameter.fpMiddleWeight
					/ m_parameter.fpLowWeight, 1.0 / 4.0 ) ;
	fpPresetWeight[2] =
		m_parameter.fpLowWeight
			* pow( m_parameter.fpMiddleWeight
					/ m_parameter.fpLowWeight, 3.0 / 4.0 ) ;
	for ( i = 3; i < 7; i ++ )
	{
		fpPresetWeight[i] =
			pow( m_parameter.fpMiddleWeight, ((6 - i) * 2) / 7.0 ) ;
	}
	//
	double	fpAvgRatio[7] ;
	i = 0 ;
	for ( j = 0; j < 7; j ++ )
	{
		fpAvgRatio[j] = 0.0 ;
		for ( size_t k = 0; k < m_nFrequencyWidth[j]; k ++ )
		{
			fpAvgRatio[j] += sqrt( fabs( ptrSource[i ++] ) ) ;
		}
		fpAvgRatio[j] = fpAvgRatio[j] / m_nFrequencyWidth[j] ;
		fpAvgRatio[j] *= fpAvgRatio[j] ;
	}
	*ptrWeightCode = 0 ;
	double	fpBaseAvg = 0.0 ;
	for ( i = 6; (ssize_t) i >= 0; i -- )
	{
		fpBaseAvg = fpAvgRatio[i] ;
		if ( fpBaseAvg >= 1.0 )
			break ;
	}
	for ( i = 0; i < 6; i ++ )
	{
		int	nLogRatio = 0 ;
		if ( fpBaseAvg >= 1.0 )
		{
			double	fpRatio =
				pow( fpAvgRatio[i] / fpBaseAvg / fpPresetWeight[i], 0.85 ) ;
			if ( fpRatio > 0.0 )
			{
				nLogRatio = eslRoundR32ToInt
					( (float32_t) (log( fpRatio ) / log( 2.0 ) * 2.0) ) ;
				if ( nLogRatio < -15 )
					nLogRatio = -15 ;
				if ( nLogRatio > 16 )
					nLogRatio = 16 ;
			}
		}
		fpAvgRatio[i] = 1.0 / pow( 2.0, nLogRatio * 0.5 ) ;
		*ptrWeightCode |= (nLogRatio + 15) << (i * 5) ;
	}
	fpAvgRatio[6] = 1.0 ;
	//
	for ( i = 0; i < m_nFrequencyPoint[0]; i ++ )
	{
		m_ptrWeightTable[i] = (float32_t) fpAvgRatio[0] ;
	}
	for ( j = 1; j < 7; j ++ )
	{
		double	a = fpAvgRatio[j - 1] ;
		double	k = (fpAvgRatio[j] - a)
					/ (m_nFrequencyPoint[j]
						- m_nFrequencyPoint[j - 1]) ;
		while ( i < m_nFrequencyPoint[j] )
		{
			m_ptrWeightTable[i] =
				(float32_t) (k * (i - m_nFrequencyPoint[j - 1]) + a) ;
			i ++ ;
		}
	}
	while ( i < nDegreeNum )
	{
		m_ptrWeightTable[i ++] = (float32_t) fpAvgRatio[6] ;
	}
	//
	*ptrWeightCode |= ((DWORD) m_parameter.nOddWeight << 30) ;
	//
	float32_t	fpOddWeight =
					(float32_t) ((m_parameter.nOddWeight + 2) * 0.5F) ;
	for ( i = 15; i < nDegreeNum; i += 16 )
	{
		m_ptrWeightTable[i] *= fpOddWeight ;
	}
	//
	// 絶対値の平均を算出
	//
	double	fpAvg ;
	double	fpMax = 0.0 ;
	double	fpSum = 0.0 ;
	//
	for ( i = 0; i < nDegreeNum; i ++ )
	{
		double	r = fabs( ptrSource[i] * m_ptrWeightTable[i] ) ;
		if ( r > fpMax )
		{
			fpMax = r ;
		}
		if ( r >= 1.0 )
		{
			fpSum += sqrt( r ) ;
		}
	}
	fpAvg = (fpSum / nDegreeNum) ;
	fpAvg *= fpAvg ;
	//
	// 係数を算出
	//
	double	fpCoefficient ;
	double	fpMinCoefficient = fpMax / 0x7800 ;
	if ( fpMinCoefficient < 1.0 )
	{
		fpMinCoefficient = 1.0 ;
	}
	fpCoefficient = fpMinCoefficient ;
	if ( fpAvg > fpPowerScale )
	{
		fpCoefficient = fpAvg / fpPowerScale ;
	}
	if ( fpCoefficient < fpMinCoefficient )
	{
		fpCoefficient = fpMinCoefficient ;
	}
	int	nCoefficient = eslRoundR32ToInt( (float32_t) fpCoefficient ) ;
	if ( nCoefficient >= 0x10000 )
	{
		nCoefficient = 0xFFFF ;
	}
	*ptrCoefficient = nCoefficient ;
	//
	fpCoefficient = 1.0 / nCoefficient ;
	//
	m_ptrWeightTable[nDegreeNum-1] = (float32_t) nCoefficient ;
	//
	// 量子化
	//
	for ( i = 0; i < nDegreeNum; i ++ )
	{
		int	nQuantumized =
			eslRoundR32ToInt
				( (float32_t) (ptrSource[i]
							* m_ptrWeightTable[i] * fpCoefficient) ) ;
		if ( nQuantumized < -0x8000 )
		{
			nQuantumized = -0x8000 ;
		}
		else if ( nQuantumized > 0x7FFF )
		{
			nQuantumized = 0x7FFF ;
		}
		ptrQuantumized[i] = nQuantumized ;
	}
}

// 16ビットの非可逆圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeSoundDCT_MSS
	( ERISA::SGLEncodeBitStream & bstream,
		const ERISA::MIO_DATA_HEADER & datahdr, const void * ptrWaveBuf )
{
	//
	// バッファを確保
	//
	size_t	nDegreeWidth = ((size_t) 1 << m_mioih.dwSubbandDegree) ;
	size_t	nSampleCount =
					(datahdr.dwSampleCount + nDegreeWidth - 1)
											& ~(nDegreeWidth - 1) ;
	size_t	nSubbandCount =
					(nSampleCount >> m_mioih.dwSubbandDegree) ;
	size_t	nChannelCount = m_mioih.dwChannelCount ;	// 常に２
	size_t	nAllSampleCount = nSampleCount * nChannelCount ;
	//
	if ( nSampleCount > m_nBufLength )
	{
		m_ptrBuffer2 =
			m_bufBuffer2.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		m_ptrBuffer3 =
			m_bufBuffer3.GetArray( nAllSampleCount * sizeof(int16_t) ) ;
		m_nBufLength = nSampleCount ;
	}
	//
	m_ptrNextDstBuf = (int16_t*) m_ptrBuffer2 ;
	//
	// 予約ビットを送出
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		m_ctxGamma.AttachBitStream( &bstream ) ;
	}
	else if ( m_mioih.dwArchitecture == erisaNemesisCode )
	{
		m_ctxNemesis.AttachBitStream( &bstream ) ;
		if ( datahdr.bytFlags & mioDataLeadBlock )
		{
			m_ctxNemesis.PrepareToEncodeERISACode( ) ;
		}
	}
	else
	{
		m_ctxHuffman.AttachBitStream( &bstream ) ;
		if ( datahdr.bytFlags & mioDataLeadBlock )
		{
			m_ctxHuffman.PrepareToEncodeERINACode( ) ;
		}
	}
	bstream.OutNBits( 0, 1 ) ;
	//
	// サブバンド単位で分解し、各々DCT変換を施す
	//
	size_t			i, j, k ;
	const int16_t *	ptrSrcBuf = (const int16_t *) ptrWaveBuf ;
	size_t			nLastDivision = (size_t) -1 ;
	//
	for ( i = 0; i < nSubbandCount; i ++ )
	{
		const int16_t *	ptrSubbandHead =
					ptrSrcBuf + (nDegreeWidth * nChannelCount) * i ;
		//
		size_t	nCopySamples =
					datahdr.dwSampleCount - (i * nDegreeWidth) ;
		if ( nCopySamples > nDegreeWidth )
		{
			nCopySamples = nDegreeWidth ;
		}
		//
		// サンプリング
		//
		size_t	nDivisionCode = 0 ;
		for ( j = 0; j < nChannelCount; j ++ )
		{
			const int16_t *	ptrSrcHead = ptrSubbandHead + j ;
			float32_t *		ptrSamplingBuf = m_ptrSamplingBuf + j * nDegreeWidth ;
			//
			for ( k = 0; k < nCopySamples; k ++ )
			{
				ptrSamplingBuf[k] = (float32_t) *ptrSrcHead ;
				ptrSrcHead += nChannelCount ;
			}
			while ( k < nDegreeWidth )
			{
				ptrSamplingBuf[k ++] = 0.0F ;
			}
			//
			size_t	nDivCode = GetDivisionCode( ptrSamplingBuf ) ;
			if ( nDivCode > nDivisionCode )
			{
				nDivisionCode = nDivCode ;
			}
		}
		//
		// 分解コードを取得
		//
		float32_t	fpPowerScale = (float32_t) m_parameter.fpPowerScale ;
		size_t		nDivisionCount = ((size_t) 1 << nDivisionCode) ;
		//
		for ( k = 0; k < nDivisionCode; k ++ )
		{
			fpPowerScale *= 1.0625F ;
		}
		//
		bstream.OutNBits( (((DWORD) nDivisionCode) << 30), 2 ) ;
		//
		// 行列サイズが変化する際の処理
		//
		bool	fLeadBlock = false ;
		//
		if ( nLastDivision != nDivisionCode )
		{
			//
			// 直前までの行列を完成させる
			//
			if ( i != 0 )
			{
				if ( EncodePostBlock_MSS
					( bstream, (float32_t) m_parameter.fpPowerScale ) )
				{
					return	errFailed ;
				}
			}
			//
			// 行列サイズを変化させるためのパラメータをセットアップ
			//
			nLastDivision = nDivisionCode ;
			InitializeWithDegree
				( m_mioih.dwSubbandDegree - nDivisionCode ) ;
			fLeadBlock = true ;
		}
		//
		// 順次 LOT 変換を施す
		//
		float32_t *	ptrNextSrc1 = m_ptrSamplingBuf ;
		float32_t *	ptrNextSrc2 = m_ptrSamplingBuf + nDegreeWidth ;
		//
		for ( k = 0; k < nDivisionCount; k ++ )
		{
			if ( fLeadBlock )
			{
				//
				// リードブロックを出力する
				//
				if ( EncodeLeadBlock_MSS
					( bstream, ptrNextSrc1, ptrNextSrc2, fpPowerScale ) )
				{
					return	errFailed ;
				}
				fLeadBlock = false ;
			}
			else
			{
				//
				// 通常ブロックを出力する
				//
				if ( EncodeInternalBlock_MSS
					( bstream, ptrNextSrc1, ptrNextSrc2, fpPowerScale ) )
				{
					return	errFailed ;
				}
			}
			//
			// 次へ
			//
			ptrNextSrc1 += m_nDegreeNum ;
			ptrNextSrc2 += m_nDegreeNum ;
		}
	}
	//
	// 行列を完成させる
	//
	if ( nSubbandCount > 0 )
	{
		if ( EncodePostBlock_MSS
			( bstream, (float32_t) m_parameter.fpPowerScale ) )
		{
			return	errFailed ;
		}
	}
	//
	// インターリーブして符号化
	//
	if ( m_mioih.dwArchitecture == erisaRunlengthGamma )
	{
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxGamma.EncodeGammaCodeWords
			( (const SWORD *) m_ptrBuffer2, nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		m_ctxGamma.FinishEncoding( ) ;
	}
	else if ( m_mioih.dwArchitecture != erisaNemesisCode )
	{
		int8_t *	ptrHBuf = m_ptrBuffer3 ;
		int8_t *	ptrLBuf = m_ptrBuffer3 + nAllSampleCount ;
		size_t		nAllSubbandCount = nSubbandCount ;
		//
		for ( i = 0; i < nDegreeWidth * 2; i ++ )
		{
			ptrSrcBuf = ((const int16_t *) m_ptrBuffer2) + i ;
			//
			for ( j = 0; j < nAllSubbandCount; j ++ )
			{
				int16_t	nValue = *ptrSrcBuf ;
				ptrSrcBuf += nDegreeWidth * 2 ;
				int8_t	nLow = (int8_t)((nValue << 8) >> 8) ;
				int8_t	nHigh = (int8_t)((nValue >> 8) ^ (nLow >> 7)) ;
				*(ptrHBuf ++) = nHigh ;
				*(ptrLBuf ++) = nLow ;
			}
		}
		//
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxHuffman.EncodeERINACodeBytes
			( m_ptrBuffer3, nAllSampleCount * 2 ) < nAllSampleCount * 2 )
		{
			return	errFailed ;			// エラー
		}
		m_ctxHuffman.FinishEncoding( ) ;
	}
	else
	{
		bstream.OutNBits( 0, 1 ) ;
		//
		if ( m_ctxNemesis.EncodeERISACodeWords
				( (const SWORD *) m_ptrBuffer2,
								nAllSampleCount ) < nAllSampleCount )
		{
			return	errFailed ;			// エラー
		}
		m_ctxNemesis.FinishERISACode( ) ;
	}
	return	errSuccess ;
}

// 回転パラメータを取得する
//////////////////////////////////////////////////////////////////////////////
int SGLSoundEncoder::GetRevolveCode
	( const float32_t * ptrBuf1, const float32_t * ptrBuf2 )
{
	float32_t	r1, r2, r3 ;
	double		s1 = 0.0, s2 = 0.0 ;
	//
	for ( size_t i = 0; i < m_nDegreeNum; i += 2 )
	{
		r1 = ptrBuf1[i] ;
		r2 = ptrBuf2[i] ;
		//
		if ( r1 < 0.0F )
		{
			if ( r2 < 0.0F )
			{
				r1 = -r1 ;
				r2 = -r2 ;
			}
			else
			{
				r3 = r1 ;
				r1 = r2 ;
				r2 = -r3 ;
			}
		}
		else
		{
			if ( r2 < 0.0F )
			{
				r3 = r1 ;
				r1 = -r2 ;
				r2 = r3 ;
			}
		}
		//
		s1 += r1 ;
		s2 += r2 ;
	}
	//
	double	rs = atan2( s2, s1 ) ;
	int		nRevCode =
				eslRoundR32ToInt( (float32_t) (rs * 4.0 / (PI * 0.5)) ) ;
	return	(nRevCode & 0x03) ;
}

// LOT 変換を施す
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::PerformLOT_MSS
	( float32_t * ptrDst, float32_t * ptrLapBuf, float32_t * ptrSrc )
{
	//
	// DCT 変換を施す
	//
	float32_t	fpMatrixScale = (float32_t) sqrt(2.0 / m_nDegreeNum) ;
	sclfFastDCT
		( m_ptrInternalBuf, 1, ptrSrc,
			m_ptrWorkBuf, m_nSubbandDegree ) ;
	sclfScalarMultiply
		( m_ptrInternalBuf, fpMatrixScale, m_nDegreeNum ) ;
	//
	// LOT 変換を施す
	//
	sclfFastPLOT
		( m_ptrInternalBuf, m_nSubbandDegree ) ;
	sclfFastLOT
		( ptrDst, ptrLapBuf,
			m_ptrInternalBuf, m_nSubbandDegree ) ;
	sclfOddGivensMatrix
		( ptrDst, m_pRevolveParam, m_nSubbandDegree ) ;
	//
	for ( size_t i = 0; i < m_nDegreeNum; i ++ )
	{
		ptrLapBuf[i] = m_ptrInternalBuf[i] ;
	}
}

// 通常のブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeInternalBlock_MSS
	( ERISA::SGLEncodeBitStream & bstream,
		float32_t * ptrSrc1, float32_t * ptrSrc2, float32_t fpPowerScale )
{
	//
	// LOT 変換を施す
	//
	float32_t *	ptrDstBuf2 = m_ptrDstBuf + m_nDegreeNum ;
	float32_t *	ptrLapBuf2 = m_ptrLastDCT + m_nDegreeNum ;
	//
	PerformLOT_MSS( m_ptrDstBuf, m_ptrLastDCT, ptrSrc1 ) ;
	PerformLOT_MSS( ptrDstBuf2, ptrLapBuf2, ptrSrc2 ) ;
	//
	// 回転パラメータを取得する
	//
	int		nRevCode1 = GetRevolveCode( m_ptrDstBuf, ptrDstBuf2 ) ;
	int		nRevCode2 = GetRevolveCode( m_ptrDstBuf + 1, ptrDstBuf2 + 1 ) ;
	DWORD	dwRevCode = (nRevCode1 << 2) | nRevCode2 ;
	//
	// 回転処理
	//
	float32_t	fpSin, fpCos ;
	//
	fpSin = (float32_t) sin( - nRevCode1 * PI / 8 ) ;
	fpCos = (float32_t) cos( - nRevCode1 * PI / 8 ) ;
	sclfRevolve2x2
		( m_ptrDstBuf, ptrDstBuf2, fpSin, fpCos, 2, m_nDegreeNum / 2 ) ;
	//
	fpSin = (float32_t) sin( - nRevCode2 * PI / 8 ) ;
	fpCos = (float32_t) cos( - nRevCode2 * PI / 8 ) ;
	sclfRevolve2x2
		( m_ptrDstBuf + 1, ptrDstBuf2 + 1,
				fpSin, fpCos, 2, m_nDegreeNum / 2 ) ;
	//
	bstream.OutNBits( (dwRevCode << 28), 4 ) ;
	//
	// 量子化
	//
	DWORD	nWeightCode ;
	int		nCoefficient ;
	Quantumize_MSS
		( (int32_t*) m_ptrBuffer1, m_ptrDstBuf,
			m_nDegreeNum, fpPowerScale,
			&nWeightCode, &nCoefficient ) ;
	//
	// 量子化係数を出力
	//
	bstream.OutNBits( nWeightCode, 32 ) ;
	bstream.OutNBits( (((DWORD) nCoefficient) << 16), 16 ) ;
	//
	// 出力
	//
	const int32_t *	ptrSrcBuf = (const int32_t *) m_ptrBuffer1 ;
	for ( size_t i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrNextDstBuf[0] = (int16_t) ptrSrcBuf[0] ;
		m_ptrNextDstBuf[1] = (int16_t) ptrSrcBuf[1] ;
		m_ptrNextDstBuf += 2 ;
		ptrSrcBuf += 2 ;
	}
	return	errSuccess ;
}

// リードブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodeLeadBlock_MSS
	( ERISA::SGLEncodeBitStream & bstream,
		float32_t * ptrSrc1, float32_t * ptrSrc2, float32_t fpPowerScale )
{
	//
	// 重複係数を初期化
	//
	size_t	i ;
	for ( i = 0; i < m_nDegreeNum * 2; i ++ )
	{
		m_ptrLastDCT[i] = 0.0F ;
	}
	//
	// LOT 変換を施す
	//
	float32_t *	ptrDstBuf2 = m_ptrDstBuf + m_nDegreeNum ;
	float32_t *	ptrLapBuf2 = m_ptrLastDCT + m_nDegreeNum ;
	//
	PerformLOT_MSS( m_ptrDstBuf, m_ptrLastDCT, ptrSrc1 ) ;
	PerformLOT_MSS( ptrDstBuf2, ptrLapBuf2, ptrSrc2 ) ;
	//
	// 回転パラメータを取得する
	//
	int	nRevCode2 = GetRevolveCode( m_ptrDstBuf + 1, ptrDstBuf2 + 1 ) ;
	//
	// 回転処理
	//
	float32_t	fpSin, fpCos ;
	//
	fpSin = (float32_t) sin( - nRevCode2 * PI / 8 ) ;
	fpCos = (float32_t) cos( - nRevCode2 * PI / 8 ) ;
	sclfRevolve2x2
		( m_ptrDstBuf + 1, ptrDstBuf2 + 1, fpSin, fpCos, 2, m_nDegreeNum / 2 ) ;
	//
	for ( i = 0; i < m_nDegreeNum; i += 2 )
	{
		m_ptrDstBuf[i] = m_ptrDstBuf[i + 1] ;
		ptrDstBuf2[i] = ptrDstBuf2[i + 1] ;
	}
	//
	bstream.OutNBits( (nRevCode2 << 30), 2 ) ;
	//
	// 量子化
	//
	DWORD	nWeightCode ;
	int		nCoefficient ;
	Quantumize_MSS
		( (int32_t*) m_ptrBuffer1, m_ptrDstBuf,
			m_nDegreeNum, fpPowerScale,
			&nWeightCode, &nCoefficient ) ;
	//
	// 量子化係数を出力
	//
	bstream.OutNBits( nWeightCode, 32 ) ;
	bstream.OutNBits( (((DWORD) nCoefficient) << 16), 16 ) ;
	//
	// 出力
	//
	const int32_t *	ptrSrcBuf = (const int32_t *) m_ptrBuffer1 ;
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrNextDstBuf[0] = (int16_t) ptrSrcBuf[1] ;
		m_ptrNextDstBuf ++ ;
		ptrSrcBuf += 2 ;
	}
	return	errSuccess ;
}

// ポストブロックを符号化して出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundEncoder::EncodePostBlock_MSS
	( ERISA::SGLEncodeBitStream & bstream, float32_t fpPowerScale )
{
	//
	// ダミーの入力信号を送信する
	//
	size_t		i ;
	float32_t *	ptrSamples1 = (float32_t*) m_ptrBuffer1 ;
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		ptrSamples1[i * 2]     = 0.0F ;
		ptrSamples1[i * 2 + 1] = 0.0F ;
	}
	//
	// LOT 変換を施す
	//
	float32_t *	ptrDstBuf2 = m_ptrDstBuf + m_nDegreeNum ;
	float32_t *	ptrLapBuf2 = m_ptrLastDCT + m_nDegreeNum ;
	float32_t *	ptrSamples2 = ptrSamples1 + m_nDegreeNum ;
	//
	PerformLOT_MSS( m_ptrDstBuf, m_ptrLastDCT, ptrSamples1 ) ;
	PerformLOT_MSS( ptrDstBuf2, ptrLapBuf2, ptrSamples2 ) ;
	//
	// 回転パラメータを取得する
	//
	int	nRevCode2 = GetRevolveCode( m_ptrDstBuf + 1, ptrDstBuf2 + 1 ) ;
	//
	// 回転処理
	//
	float32_t	fpSin, fpCos ;
	//
	fpSin = (float32_t) sin( - nRevCode2 * PI / 8 ) ;
	fpCos = (float32_t) cos( - nRevCode2 * PI / 8 ) ;
	sclfRevolve2x2
		( m_ptrDstBuf + 1, ptrDstBuf2 + 1,
					fpSin, fpCos, 2, m_nDegreeNum / 2 ) ;
	//
	for ( i = 0; i < m_nDegreeNum; i += 2 )
	{
		m_ptrDstBuf[i] = m_ptrDstBuf[i + 1] ;
		ptrDstBuf2[i] = ptrDstBuf2[i + 1] ;
	}
	//
	bstream.OutNBits( (nRevCode2 << 30), 2 ) ;
	//
	// 量子化
	//
	DWORD	nWeightCode ;
	int		nCoefficient ;
	Quantumize_MSS
		( (int32_t*) m_ptrBuffer1, m_ptrDstBuf,
			m_nDegreeNum, fpPowerScale,
			&nWeightCode, &nCoefficient ) ;
	//
	// 量子化係数を出力
	//
	bstream.OutNBits( nWeightCode, 32 ) ;
	bstream.OutNBits( (((DWORD) nCoefficient) << 16), 16 ) ;
	//
	// 出力
	//
	const int32_t *	ptrSrcBuf = (const int32_t *) m_ptrBuffer1 ;
	for ( i = 0; i < m_nDegreeNum; i ++ )
	{
		m_ptrNextDstBuf[0] = (int16_t) ptrSrcBuf[1] ;
		m_ptrNextDstBuf ++ ;
		ptrSrcBuf += 2 ;
	}
	return	errSuccess ;
}

// 量子化
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEncoder::Quantumize_MSS
	( int32_t * ptrQuantumized, const float32_t * ptrSource,
		size_t nDegreeNum, float32_t fpPowerScale,
		DWORD * ptrWeightCode, int * ptrCoefficient )
{
	//
	// 関数初期設定
	//
	size_t	i, j ;
	size_t	nHalfDegree = nDegreeNum / 2 ;
	//
	// 各周波数帯のエネルギ集中度から周波数ごとの量子化係数を算出する
	//
	double	fpPresetWeight[7] ;
	fpPresetWeight[0] = fpPresetWeight[1] =
		m_parameter.fpLowWeight
			* pow( m_parameter.fpMiddleWeight
					/ m_parameter.fpLowWeight, 1.0 / 4.0 ) ;
	fpPresetWeight[2] =
		m_parameter.fpLowWeight
			* pow( m_parameter.fpMiddleWeight
					/ m_parameter.fpLowWeight, 3.0 / 4.0 ) ;
	for ( i = 3; i < 7; i ++ )
	{
		fpPresetWeight[i] =
			pow( m_parameter.fpMiddleWeight, ((6 - i) * 2) / 7.0 ) ;
	}
	//
	double	fpAvgRatio[7] ;
	i = 0 ;
	for ( j = 0; j < 7; j ++ )
	{
		fpAvgRatio[j] = 0.0 ;
		for ( size_t k = 0; k < m_nFrequencyWidth[j]; k ++ )
		{
			fpAvgRatio[j] += sqrt( fabs( ptrSource[i] ) ) ;
			fpAvgRatio[j] += sqrt( fabs( ptrSource[m_nDegreeNum + i] ) ) ;
			++ i ;
		}
		fpAvgRatio[j] = fpAvgRatio[j] * 0.5 / m_nFrequencyWidth[j] ;
		fpAvgRatio[j] *= fpAvgRatio[j] ;
	}
	*ptrWeightCode = 0 ;
	double	fpBaseAvg = 0.0 ;
	for ( i = 6; (ssize_t) i >= 0; i -- )
	{
		fpBaseAvg = fpAvgRatio[i] ;
		if ( fpBaseAvg >= 1.0 )
			break ;
	}
	for ( i = 0; i < 6; i ++ )
	{
		int	nLogRatio = 0 ;
		if ( fpBaseAvg >= 1.0 )
		{
			double	fpRatio =
				pow( fpAvgRatio[i]
						/ fpBaseAvg / fpPresetWeight[i], 0.85 ) ;
			if ( fpRatio > 0.0 )
			{
				nLogRatio = eslRoundR32ToInt
					( (float32_t) (log( fpRatio ) / log( 2.0 ) * 2.0) ) ;
				if ( nLogRatio < -15 )
					nLogRatio = -15 ;
				if ( nLogRatio > 16 )
					nLogRatio = 16 ;
			}
		}
		fpAvgRatio[i] = 1.0 / pow( 2.0, nLogRatio * 0.5 ) ;
		*ptrWeightCode |= (nLogRatio + 15) << (i * 5) ;
	}
	fpAvgRatio[6] = 1.0 ;
	//
	for ( i = 0; i < m_nFrequencyPoint[0]; i ++ )
	{
		m_ptrWeightTable[i] = (float32_t) fpAvgRatio[0] ;
	}
	for ( j = 1; j < 7; j ++ )
	{
		double	a = fpAvgRatio[j - 1] ;
		double	k = (fpAvgRatio[j] - a)
					/ (m_nFrequencyPoint[j]
						- m_nFrequencyPoint[j - 1]) ;
		while ( i < m_nFrequencyPoint[j] )
		{
			m_ptrWeightTable[i] =
				(float32_t) (k * (i - m_nFrequencyPoint[j - 1]) + a) ;
			i ++ ;
		}
	}
	while ( i < nDegreeNum )
	{
		m_ptrWeightTable[i ++] = (float32_t) fpAvgRatio[6] ;
	}
	//
	*ptrWeightCode |= ((DWORD) m_parameter.nOddWeight << 30) ;
	//
	float32_t	fpOddWeight =
			(float32_t) ((m_parameter.nOddWeight + 2) * 0.5F) ;
	for ( i = 15; i < nDegreeNum; i += 16 )
	{
		m_ptrWeightTable[i] *= fpOddWeight ;
	}
	//
	// 絶対値の平均を算出
	//
	double	fpAvg ;
	double	fpMax = 0.0 ;
	double	fpSum = 0.0 ;
	//
	for ( i = 0; i < nDegreeNum; i ++ )
	{
		double	r ;
		r = fabs( ptrSource[i] * m_ptrWeightTable[i] ) ;
		if ( r > fpMax )
		{
			fpMax = r ;
		}
		if ( r >= 1.0 )
		{
			fpSum += sqrt( r ) ;
		}
		//
		r = fabs( ptrSource[m_nDegreeNum + i] * m_ptrWeightTable[i] ) ;
		if ( r > fpMax )
		{
			fpMax = r ;
		}
		if ( r >= 1.0 )
		{
			fpSum += sqrt( r ) ;
		}
	}
	fpAvg = (fpSum / nDegreeNum / 2) ;
	fpAvg *= fpAvg ;
	//
	// 係数を算出
	//
	double	fpCoefficient ;
	double	fpMinCoefficient = fpMax / 0x7800 ;
	if ( fpMinCoefficient < 1.0 )
	{
		fpMinCoefficient = 1.0 ;
	}
	fpCoefficient = fpMinCoefficient ;
	if ( fpAvg > fpPowerScale )
	{
		fpCoefficient = fpAvg / fpPowerScale ;
	}
	if ( fpCoefficient < fpMinCoefficient )
	{
		fpCoefficient = fpMinCoefficient ;
	}
	int	nCoefficient = eslRoundR32ToInt( (float32_t) fpCoefficient ) ;
	if ( nCoefficient >= 0x10000 )
	{
		nCoefficient = 0xFFFF ;
	}
	*ptrCoefficient = nCoefficient ;
	//
	fpCoefficient = 1.0 / nCoefficient ;
	//
	m_ptrWeightTable[nDegreeNum-1] = (float32_t) nCoefficient ;
	//
	// 量子化
	//
	for ( j = 0; j < 2; j ++ )
	{
		for ( i = 0; i < nDegreeNum; i ++ )
		{
			int	nQuantumized =
				eslRoundR32ToInt
					( (float32_t) (*(ptrSource ++)
								* m_ptrWeightTable[i] * fpCoefficient) ) ;
			if ( nQuantumized < -0x8000 )
			{
				nQuantumized = -0x8000 ;
			}
			else if ( nQuantumized > 0x7FFF )
			{
				nQuantumized = 0x7FFF ;
			}
			*(ptrQuantumized ++) = nQuantumized ;
		}
	}
}

