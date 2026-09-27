
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
// SGLImageEncoder::Parameter
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoder::Parameter::Parameter( void )
{
	m_nFlags = 0 ;
	m_fpYScaleDC = 0.5F ;
	m_fpCScaleDC = 0.3F ;
	m_fpYScaleLow = 0.25F ;
	m_fpCScaleLow = 0.2F ;
	m_fpYScaleHigh = 0.25F ;
	m_fpCScaleHigh = 0.2F ;
	m_nYThreshold = 0 ;
	m_nCThreshold = 0 ;
	m_nYLPFThreshold = 64 ;
	m_nCLPFThreshold = 64 ;
	m_nAMDFThreshold = 0x120 ;
	m_fpPFrameScale = 1.0F ;
	m_fpBFrameScale = 0.9F ;
	m_nMaxFrameSize = 0 ;
	m_nMinFrameSize = 0 ;
}

// プリセット値取得
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::Parameter::LoadPresetParam
	( SGLImageEncoder::PresetParameter ppIndex,
						ERISA::ERI_INFO_HEADER & infhdr )
{
	struct	PRESET_PARAMETER
	{
		uint32_t	nFlags ;
		uint32_t	fTransformation ;
		float32_t	fpYScaleDC ;
		float32_t	fpYScaleLow ;
		float32_t	fpYScaleHigh ;
		float32_t	fpCScaleDC ;
		float32_t	fpCScaleLow ;
		float32_t	fpCScaleHigh ;
		float32_t	fpPFrameScale ;
		float32_t	fpBFrameScale ;
		size_t		nMaxFrameSize ;
		size_t		nMinFrameSize ;
		int			nYThreshold ;
		int			nCThreshold ;
		int			nYLPFThreshold ;
		int			nCLPFThreshold ;
		int			nAMDFThreshold ;
		uint32_t	nSamplingFlag ;
	} ;
	static const PRESET_PARAMETER	preset[ppCount] =
	{
		{	// 準可逆圧縮
			pfUseLoopFilter,
			eriTransformationDCT,
			0.5F, 0.3F, 0.2F,
			0.35F, 0.25F, 0.2F,
			1.5F, 1.0F, 0, 0,
			0, 0, 64, 64,
			0x120, eriSamplingYUV444
		},
		{	// 高画質１
			pfUseLoopFilter,
			eriTransformationDCT,
			0.25F, 0.06F, 0.03F,
			0.2F, 0.05F, 0.025F,
			1.5F, 1.0F, 0, 0,
			0, 0, 64, 64,
			0x120, eriSamplingYUV411
		},
		{	// 高画質２
			pfUseLoopFilter,
			eriTransformationDCT,
			0.2F, 0.05F, 0.025F,
			0.18F, 0.04F, 0.02F,
			1.2F, 0.85F, 0, 0,
			1, 1, 63, 63,
			0x120, eriSamplingYUV411
		},
		{	// 標準１
			pfUseLoopFilter,
			eriTransformationDCT,
			0.15F, 0.04F, 0.02F,
			0.1F, 0.025F, 0.01F,
			1.2F, 0.85F, 8000, 6000,
			1, 1, 43, 43,
			0x140, eriSamplingYUV411
		},
		{	// 標準２
			pfUseLoopFilter,
			eriTransformationDCT,
			0.15F, 0.03F, 0.02F,
			0.1F, 0.02F, 0.01F,
			1.0F, 0.7F, 6000, 4000,
			1, 1, 36, 36,
			0x180, eriSamplingYUV411
		},
		{	// 低画質１
			pfUseLoopFilter,
			eriTransformationLOT,
			0.12F, 0.02F, 0.01F,
			0.09F, 0.02F, 0.01F,
			0.9F, 0.6F, 4000, 3000,
			1, 1, 28, 28,
			0x180, eriSamplingYUV411
		},
		{	// 低画質２
			pfUseLoopFilter,
			eriTransformationLOT,
			0.12F, 0.02F, 0.01F,
			0.09F, 0.02F, 0.01F,
			0.9F, 0.6F, 3000, 2000,
			2, 2, 28, 28,
			0x180, eriSamplingYUV411
		}
	} ;
	//
	m_nFlags = preset[ppIndex].nFlags ;
	m_fpYScaleDC = preset[ppIndex].fpYScaleDC ;
	m_fpYScaleLow = preset[ppIndex].fpYScaleLow ;
	m_fpYScaleHigh = preset[ppIndex].fpYScaleHigh ;
	m_fpCScaleDC = preset[ppIndex].fpCScaleDC ;
	m_fpCScaleLow = preset[ppIndex].fpCScaleLow ;
	m_fpCScaleHigh = preset[ppIndex].fpCScaleHigh ;
	m_nYThreshold = preset[ppIndex].nYThreshold ;
	m_nCThreshold = preset[ppIndex].nCThreshold ;
	m_nYLPFThreshold = preset[ppIndex].nYLPFThreshold ;
	m_nCLPFThreshold = preset[ppIndex].nCLPFThreshold ;
	m_nAMDFThreshold = preset[ppIndex].nAMDFThreshold ;
	m_fpPFrameScale = preset[ppIndex].fpPFrameScale ;
	m_fpBFrameScale = preset[ppIndex].fpBFrameScale ;
	m_nMaxFrameSize = preset[ppIndex].nMaxFrameSize ;
	m_nMinFrameSize = preset[ppIndex].nMinFrameSize ;
	//
	infhdr.dwVersion = 0x00020300 ;
	infhdr.fdwTransformation = preset[ppIndex].fTransformation ;
	infhdr.dwArchitecture = eriRunlengthGamma ;
	infhdr.dwSamplingFlag = preset[ppIndex].nSamplingFlag ;
	infhdr.dwBlockingDegree = 3 ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像圧縮オブジェクト
//////////////////////////////////////////////////////////////////////////////

const SGLImageEncoder::PTR_PROCEDURE
	SGLImageEncoder::m_pfnColorOperation[0x10] =
{
	&SGLImageEncoder::ColorOperation0000,
		&SGLImageEncoder::ColorOperation0000,
		&SGLImageEncoder::ColorOperation0000,
		&SGLImageEncoder::ColorOperation0000,
		&SGLImageEncoder::ColorOperation0000,
	&SGLImageEncoder::ColorOperation0101,
	&SGLImageEncoder::ColorOperation0110,
	&SGLImageEncoder::ColorOperation0111,
		&SGLImageEncoder::ColorOperation0000,
	&SGLImageEncoder::ColorOperation1001,
	&SGLImageEncoder::ColorOperation1010,
	&SGLImageEncoder::ColorOperation1011,
		&SGLImageEncoder::ColorOperation0000,
	&SGLImageEncoder::ColorOperation1101,
	&SGLImageEncoder::ColorOperation1110,
	&SGLImageEncoder::ColorOperation1111
} ;


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLImageEncoder, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoder::SGLImageEncoder( void )
{
	m_ptrColumnBuf = NULL ;
	m_ptrLineBuf = NULL ;
	m_ptrEncodeBuf = NULL ;
	m_ptrArrangeBuf = NULL ;
	m_pArrangeTable[0] = NULL ;
	//
	m_ptrVertBufLOT = NULL ;
	m_ptrHorzBufLOT = NULL ;
	m_ptrBlocksetBuf[0] = NULL ;
	m_ptrMatrixBuf[0] = NULL ;
	m_pQuantumizeScale[0] = NULL ;
	m_pQuantumizeTable = NULL ;
	//
	m_nMovingVector = 0 ;
	m_pMoveVecFlags = NULL ;
	m_pMovingVector = NULL ;
	m_fPredFrameType = 0 ;
	m_nIntraBlockCount = 0 ;
	m_fpDiffDeflectBlock = 0 ;
	m_fpMaxDeflectBlock = 0 ;
	//
	m_ptrCoefficient = NULL ;
	m_ptrImageDst = NULL ;
	m_ptrSignalBuf = NULL ;
	m_pHuffmanTree = NULL ;
	m_pProbERISA = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoder::~SGLImageEncoder( void )
{
	Delete() ;
}

// 初期化（パラメータの設定）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::Initialize( const ERISA::ERI_INFO_HEADER & infhdr )
{
	//
	// 以前のデータを消去
	//
	Delete() ;
	//
	// 画像情報ヘッダをコピー
	//
	m_eihInfo = infhdr ;
	//
	// パラメータのチェック
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLossless )
	{
		//
		// 可逆圧縮
		//////////////////////////////////////////////////////////////////////
		if ( (m_eihInfo.dwArchitecture != eriRunlengthGamma)
				&& (m_eihInfo.dwArchitecture != eriRunlengthHuffman)
				&& (m_eihInfo.dwArchitecture != erisaNemesisCode) )
		{
			return	errFailed ;		// エラー（未対応の圧縮フォーマット）
		}
		//
		switch ( (m_eihInfo.fdwFormatType & formatImageTypeMask) )
		{
		case	formatImageRGB:
		case	formatImageBGR:
			if ( m_eihInfo.dwBitsPerPixel <= 8 )
				m_nChannelCount = 1 ;
			else if ( !(m_eihInfo.fdwFormatType & formatImageFlagAlpha) )
				m_nChannelCount = 3 ;
			else
				m_nChannelCount = 4 ;
			break ;

		case	formatImageGray:
			m_nChannelCount = 1 ;
			break;

		default:
			return	errFailed ;		// エラー（未対応の画像フォーマット）
		}
		//
		// 各定数を計算
		//
		if ( m_eihInfo.dwBlockingDegree == 0 )
		{
			return	errFailed ;		// エラー（未対応の画像フォーマット）
		}
		//
		m_nBlockSize = ((size_t) 1 << (size_t) m_eihInfo.dwBlockingDegree) ;
		m_nBlockArea = ((size_t) 1 << (size_t) (m_eihInfo.dwBlockingDegree * 2)) ;
		m_nBlockSamples = m_nBlockArea * m_nChannelCount ;
		m_nWidthInBlocks =
			(size_t) ((m_eihInfo.nImageWidth + m_nBlockSize - 1)
									>> m_eihInfo.dwBlockingDegree) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			m_nHeightInBlocks = (size_t) - m_eihInfo.nImageHeight ;
		}
		else
		{
			m_nHeightInBlocks = (size_t) m_eihInfo.nImageHeight ;
		}
		m_nHeightInBlocks =
			(m_nHeightInBlocks + m_nBlockSize - 1)
								>> m_eihInfo.dwBlockingDegree ;
		//
		// ワーキングメモリを確保
		//
		m_ptrColumnBuf =
			m_bufColumn.GetArray( m_nBlockSize * m_nChannelCount ) ;
		m_ptrLineBuf =
			m_bufLineBuf.GetArray
				( m_nChannelCount *
					(m_nWidthInBlocks << m_eihInfo.dwBlockingDegree) ) ;
		m_ptrEncodeBuf = m_bufEncodeBuf.GetArray( m_nBlockSamples ) ;
		m_ptrArrangeBuf = m_bufArrangeBuf.GetArray( m_nBlockSamples ) ;
		//
		// バージョンのチェックとサンプリングテーブルの準備
		//
		InitializeSamplingTable( ) ;
		//
		if ( m_eihInfo.dwVersion == 0x00020200 )
		{
			if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
			{
				m_pHuffmanTree = new ERISA::ERINA_HUFFMAN_TREE ;
			}
			else if ( m_eihInfo.dwArchitecture == erisaNemesisCode )
			{
				m_pProbERISA = new ERISA::ERISA_PROB_MODEL ;
			}
		}
		else if ( m_eihInfo.dwVersion != 0x00020100 )
		{
			return	errFailed ;		// エラー（未対応のバージョン）
		}
	}
	else if ( (m_eihInfo.fdwTransformation == eriTransformationDCT)
			|| (m_eihInfo.fdwTransformation == eriTransformationLOT) )
	{
		//
		// 非可逆圧縮
		//////////////////////////////////////////////////////////////////////
		if ( m_eihInfo.dwArchitecture != eriRunlengthGamma )
		{
			return	errFailed ;		// エラー（未対応の圧縮フォーマット）
		}
		//
		switch ( (m_eihInfo.fdwFormatType & formatImageTypeMask) )
		{
		case	formatImageRGB:
		case	formatImageBGR:
			if ( m_eihInfo.dwBitsPerPixel <= 8 )
				m_nChannelCount = 1 ;
			else if ( !(m_eihInfo.fdwFormatType & formatImageFlagAlpha) )
				m_nChannelCount = 3 ;
			else
				m_nChannelCount = 4 ;
			break ;

		case	formatImageGray:
			m_nChannelCount = 1 ;
			break;

		default:
			return	errFailed ;		// エラー（未対応の画像フォーマット）
		}
		//
		// 各定数を計算
		//
		if ( m_eihInfo.dwBlockingDegree != 3 )
		{
			return	errFailed ;		// エラー（未対応の画像フォーマット）
		}
		//
		m_nBlockSize = ((size_t) 1 << (size_t) m_eihInfo.dwBlockingDegree) ;
		m_nBlockArea = ((size_t) 1 << (size_t) (m_eihInfo.dwBlockingDegree * 2)) ;
		m_nBlockSamples = m_nBlockArea * m_nChannelCount ;
		if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
		{
			m_nWidthInBlocks =
				(size_t) ((m_eihInfo.nImageWidth + m_nBlockSize * 2 - 1)
									>> (m_eihInfo.dwBlockingDegree + 1)) ;
			if ( m_eihInfo.nImageHeight < 0 )
			{
				m_nHeightInBlocks = (size_t) - m_eihInfo.nImageHeight ;
			}
			else
			{
				m_nHeightInBlocks = (size_t) m_eihInfo.nImageHeight ;
			}
			m_nHeightInBlocks =
				(m_nHeightInBlocks + m_nBlockSize * 2 - 1)
								>> (m_eihInfo.dwBlockingDegree + 1) ;
			//
			m_nWidthInBlocks += 1  ;
			m_nHeightInBlocks += 1 ;
			//
			if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
			{
				m_nBlocksetCount = m_nChannelCount * 4 ;
			}
			else if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
			{
				switch ( m_nChannelCount )
				{
				case	1:
					m_nBlocksetCount = 4 ;
					break ;
				case	3:
					m_nBlocksetCount = 6 ;
					break ;
				case	4:
					m_nBlocksetCount = 10 ;
					break ;
				default:
					return	errFailed ;
				}
			}
			else
			{
				return	errFailed ;		// エラー（未対応の画像フォーマット）
			}
		}
		else
		{
			m_nWidthInBlocks =
				(size_t) ((m_eihInfo.nImageWidth + (m_nBlockSize * 2 - 1))
									>> (m_eihInfo.dwBlockingDegree + 1)) ;
			if ( m_eihInfo.nImageHeight < 0 )
			{
				m_nHeightInBlocks = (size_t) - m_eihInfo.nImageHeight ;
			}
			else
			{
				m_nHeightInBlocks = (size_t) m_eihInfo.nImageHeight ;
			}
			m_nHeightInBlocks =
				(m_nHeightInBlocks + (m_nBlockSize * 2 - 1))
								>> (m_eihInfo.dwBlockingDegree + 1) ;
			//
			if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
			{
				m_nBlocksetCount = m_nChannelCount * 4 ;
			}
			else if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
			{
				switch ( m_nChannelCount )
				{
				case	1:
					m_nBlocksetCount = 4 ;
					break ;
				case	3:
					m_nBlocksetCount = 6 ;
					break ;
				case	4:
					m_nBlocksetCount = 10 ;
					break ;
				default:
					return	errFailed ;
				}
			}
			else
			{
				return	errFailed ;		// エラー（未対応の画像フォーマット）
			}
		}
		//
		// ワーキングメモリを確保
		//
		m_ptrEncodeBuf = m_bufEncodeBuf.GetArray( m_nBlockSamples * 4 ) ;
		m_ptrVertBufLOT = m_bufVertBufLOT.GetArray( m_nBlockSamples * 2 * m_nWidthInBlocks ) ;
		m_ptrHorzBufLOT = m_bufHorzBufLOT.GetArray( m_nBlockSamples * 2 ) ;
		m_ptrBlocksetBuf[0] = m_bufBlocksetBuf.GetArray( m_nBlockArea * 36 ) ;
		m_ptrMatrixBuf[0] = m_bufMatrixBuf.GetArray( m_nBlockArea * 16 ) ;
		m_pQuantumizeScale[0] = m_bufQuantumizeScale.GetArray( m_nBlockArea * 2 ) ;
		m_pQuantumizeTable = m_bufQuantumizeTable.GetArray( m_nBlockArea * 2 ) ;
		//
		size_t	nTotalBlocks = m_nWidthInBlocks * m_nHeightInBlocks ;
		m_nMovingVector = 0 ;
		m_pMoveVecFlags = m_bufMoveVecFlags.GetArray( nTotalBlocks ) ;
		m_pMovingVector = m_bufMovingVector.GetArray( nTotalBlocks * 4 ) ;
		m_ptrCoefficient = m_bufCoefficient.GetArray( nTotalBlocks * 2 ) ;
		m_ptrImageDst = m_bufImageDst.GetArray( nTotalBlocks * m_nBlockArea * m_nBlocksetCount ) ;
		m_ptrSignalBuf = m_bufSignalBuf.GetArray( nTotalBlocks * m_nBlockArea * m_nBlocksetCount ) ;
		//
		int	i ;
		for ( i = 1; i < 36; i ++ )
		{
			m_ptrBlocksetBuf[i] = m_ptrBlocksetBuf[0] + (m_nBlockArea * i) ;
		}
		for ( i = 1; i < 16; i ++ )
		{
			m_ptrMatrixBuf[i] = m_ptrMatrixBuf[0] + (m_nBlockArea * i) ;
		}
		m_pQuantumizeScale[1] = m_pQuantumizeScale[0] + m_nBlockArea ;
		//
		// サンプリングテーブルの準備
		//
		InitializeZigZagTable( ) ;
		//
		m_pHuffmanTree = new ERINA_HUFFMAN_TREE ;
	}
	else
	{
		return	errFailed ;			// エラー（未対応のフォーマット）
	}
	//
	// 正常終了
	//
	return	errSuccess ;
}

// 終了（メモリの解放など）
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::Delete( void )
{
	m_bufColumn.FreeArray() ;
	m_bufLineBuf.FreeArray() ;
	m_bufEncodeBuf.FreeArray() ;
	m_bufArrangeBuf.FreeArray() ;
	m_bufArrangeTable.FreeArray() ;
	//
	m_bufVertBufLOT.FreeArray() ;
	m_bufHorzBufLOT.FreeArray() ;
	m_bufBlocksetBuf.FreeArray() ;
	m_bufMatrixBuf.FreeArray() ;
	m_bufQuantumizeScale.FreeArray() ;
	m_bufQuantumizeTable.FreeArray() ;
	//
	m_bufMoveVecFlags.FreeArray() ;
	m_bufMovingVector.FreeArray() ;
	//
	m_bufCoefficient.FreeArray() ;
	m_bufImageDst.FreeArray() ;
	m_bufSignalBuf.FreeArray() ;
	//
	m_ptrColumnBuf = NULL ;
	m_ptrLineBuf = NULL ;
	m_ptrEncodeBuf = NULL ;
	m_ptrArrangeBuf = NULL ;
	m_pArrangeTable[0] = NULL ;
	//
	m_ptrVertBufLOT = NULL ;
	m_ptrHorzBufLOT = NULL ;
	m_ptrBlocksetBuf[0] = NULL ;
	m_ptrMatrixBuf[0] = NULL ;
	m_pQuantumizeScale[0] = NULL ;
	m_pQuantumizeTable = NULL ;
	//
	m_nMovingVector = 0 ;
	m_pMoveVecFlags = NULL ;
	m_pMovingVector = NULL ;
	m_fPredFrameType = 0 ;
	m_nIntraBlockCount = 0 ;
	m_fpDiffDeflectBlock = 0 ;
	m_fpMaxDeflectBlock = 0 ;
	//
	m_ptrCoefficient = NULL ;
	m_ptrImageDst = NULL ;
	m_ptrSignalBuf = NULL ;
	if ( m_pHuffmanTree != NULL )
	{
		delete	m_pHuffmanTree ;
		m_pHuffmanTree = NULL ;
	}
	if ( m_pProbERISA != NULL )
	{
		delete	m_pProbERISA ;
		m_pProbERISA = NULL ;
	}
}

// 画像を圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::EncodeImage
	( const SakuraGL::SGLImageInfo & infSrcImage,
		const uint8_t * pSrcBuffer,
		ERISA::SGLEncodeBitStream & bstream, uint32_t flagsDecoding )
{
	SGLImageBuffer	imginf ;
	((SGLImageInfo&)imginf) = infSrcImage ;
	imginf.ptrBuffer = (uint8_t*) pSrcBuffer ;
	//
	if ( imginf.format & formatImageFlagSideBySide )
	{
		imginf.width *= 2 ;
	}
	if ( m_eihInfo.fdwTransformation == eriTransformationLossless )
	{
		// 可逆圧縮フォーマット
		return	EncodeLosslessImage( imginf, bstream, flagsDecoding ) ;
	}
	else if ( (m_eihInfo.fdwTransformation == eriTransformationDCT)
			|| (m_eihInfo.fdwTransformation == eriTransformationLOT) )
	{
		// 非可逆圧縮フォーマット
		return	EncodeLossyImage( imginf, bstream, flagsDecoding ) ;
	}

	return	errFailed ;			// 未対応のフォーマット
}

// 圧縮オプションを設定
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SetCompressionParameter( const SGLImageEncoder::Parameter & prmCmprOpt )
{
	m_prmCmprOpt = prmCmprOpt ;
}

// 展開進行状況通知関数
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::OnEncodedBlock( size_t line, size_t column )
{
	return	errSuccess ;
}

// 動き補償パラメータを計算する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::ProcessMovingVector
	( const SakuraGL::SGLImageBuffer & dstimgbuf,
		const SakuraGL::SGLImageBuffer & previmgbuf,
		int & nAbsMaxDiff,
		const SakuraGL::SGLImageBuffer * ppredimg )
{
	//
	// 画像情報の検証
	//////////////////////////////////////////////////////////////////////////
	if ( (dstimgbuf.width != previmgbuf.width)
		|| (dstimgbuf.height != previmgbuf.height)
		|| (dstimgbuf.depth != previmgbuf.depth) )
	{
		return	errFailed ;
	}
	const uint32_t	nBlockSize = 16 ;
	ESLAssert( m_nBlockSize == 8 ) ;
	uint32_t	nImageWidth = dstimgbuf.width ;
	uint32_t	nImageHeight = dstimgbuf.height ;
	//
	if ( (nImageWidth != m_eihInfo.nImageWidth)
		|| (((int32_t) nImageHeight != m_eihInfo.nImageHeight)
			&& ((int32_t) nImageHeight != - m_eihInfo.nImageHeight)) )
	{
		return	errFailed ;
	}
	//
	// 各ブロック毎の動きベクトルを算出する
	//////////////////////////////////////////////////////////////////////////
	//
	uint32_t	x, y ;
	int8_t *	ptrNextVector = m_pMovingVector ;
	uint8_t *	ptrNextVecFlag = m_pMoveVecFlags ;
	uint32_t	nWidthBlocks = (nImageWidth + (nBlockSize - 1)) / nBlockSize ;
	uint32_t	nHeightBlocks = (nImageHeight + (nBlockSize - 1)) / nBlockSize ;
	double		fpSumDiffDeflect = 0 ;
	m_fPredFrameType = (ppredimg != NULL) ? 2 : 1 ;
	nAbsMaxDiff = 0 ;
	m_nIntraBlockCount = 0 ;
	m_fpMaxDeflectBlock = 0 ;
	//
	SGLImageBuffer	dstimg = dstimgbuf ;		// 動き補償結果画像出力先
	SGLImageBuffer	previmg = previmgbuf ;		// 過去フレーム
	SGLImageBuffer	predimg ;					// 未来フレーム
	if ( ppredimg != NULL )
	{
		predimg = *ppredimg ;
	}
	if ( m_eihInfo.nImageHeight < 0 )
	{
		dstimg.ptrBuffer +=
				(int32_t) (dstimg.height - 1) * dstimg.pitchLine ;
		dstimg.pitchLine = - dstimg.pitchLine ;
		//
		previmg.ptrBuffer +=
				(int32_t) (previmg.height - 1) * previmg.pitchLine ;
		previmg.pitchLine = - previmg.pitchLine ;
		//
		if ( ppredimg != NULL )
		{
			predimg.ptrBuffer +=
					(int32_t) (predimg.height - 1) * predimg.pitchLine ;
			predimg.pitchLine = - predimg.pitchLine ;
			ppredimg = &predimg ;
		}
	}
	//
	for ( y = 0; y < nImageHeight; y += nBlockSize )
	{
		for ( x = 0; x < nImageWidth; x += nBlockSize )
		{
			//
			// 動き補償ベクトルを計算する
			//
			SGLPoint	ptMoveVec[2] ;
			double		fpDeflectBlock ;
			int			nPredType =
				PredictMovingVector
					( dstimg, previmg, x, y,
						&ptMoveVec[0], nAbsMaxDiff, fpDeflectBlock, ppredimg ) ;
			//
			m_nIntraBlockCount += (nPredType == 0) ? 1 : 0 ;
			*(ptrNextVecFlag ++) = nPredType ;
			fpSumDiffDeflect += fpDeflectBlock ;
			if ( m_fpMaxDeflectBlock < fpDeflectBlock )
			{
				m_fpMaxDeflectBlock = (float32_t) fpDeflectBlock ;
			}
			ptrNextVector[0] = (int8_t) ptMoveVec[0].x ;
			ptrNextVector[1] = (int8_t) ptMoveVec[0].y ;
			ptrNextVector[2] = (int8_t) ptMoveVec[1].x ;
			ptrNextVector[3] = (int8_t) ptMoveVec[1].y ;
			ptrNextVector += 4 ;
		}
	}
	//
	// 各ブロックごとの動きベクトルを差分処理する
	//////////////////////////////////////////////////////////////////////////
	//
	// 水平方向差分
	//
	ptrNextVector = m_pMovingVector ;
	for ( y = 0; y < nHeightBlocks; y ++ )
	{
		for ( x = nWidthBlocks - 2; (int32_t) x >= 0; x -- )
		{
			ptrNextVector[x * 4 + 4] -= ptrNextVector[x * 4] ;
			ptrNextVector[x * 4 + 5] -= ptrNextVector[x * 4 + 1] ;
			ptrNextVector[x * 4 + 6] -= ptrNextVector[x * 4 + 2] ;
			ptrNextVector[x * 4 + 7] -= ptrNextVector[x * 4 + 3] ;
		}
		ptrNextVector += nWidthBlocks * 4 ;
	}
	//
	// 垂直方向差分
	//
	int8_t *	ptrLastVector =
		m_pMovingVector + (nHeightBlocks - 1) * nWidthBlocks * 4 ;
	ptrNextVector = ptrLastVector - nWidthBlocks * 4 ;
	for ( y = 1; y < nHeightBlocks; y ++ )
	{
		for ( x = 0; x < nWidthBlocks; x ++ )
		{
			ptrLastVector[x * 4]     -= ptrNextVector[x * 4] ;
			ptrLastVector[x * 4 + 1] -= ptrNextVector[x * 4 + 1] ;
			ptrLastVector[x * 4 + 2] -= ptrNextVector[x * 4 + 2] ;
			ptrLastVector[x * 4 + 3] -= ptrNextVector[x * 4 + 3] ;
		}
		ptrLastVector = ptrNextVector ;
		ptrNextVector -= nWidthBlocks * 4 ;
	}
	//
	m_nMovingVector = nWidthBlocks * nHeightBlocks ;
	m_fpDiffDeflectBlock =
		(float32_t) (fpSumDiffDeflect / (m_nMovingVector - m_nIntraBlockCount + 1)) ;
	//
	if ( m_nIntraBlockCount == m_nMovingVector )
	{
		nAbsMaxDiff = 0x7FFFFFFF ;
	}
	//
	return	errSuccess ;
}

// 動き補償パラメータをクリアする
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::ClearMovingVector( void )
{
	m_nMovingVector = 0 ;
}

// 動き補償ベクトルを計算し、予測画像との差分を計算する
//////////////////////////////////////////////////////////////////////////////
int SGLImageEncoder::PredictMovingVector
	( const SakuraGL::SGLImageBuffer & dstimg,
		const SakuraGL::SGLImageBuffer & previmg,
		int xBlock, int yBlock, SakuraGL::SGLPoint * ptMoveVec,
		int & nAbsMaxDiff, double & fpDeflectBlock,
		const SakuraGL::SGLImageBuffer * ppredimg )
{
	ESLAssert( m_nBlockSize == 8 ) ;
	const int	nBlockSize = 16 ;
	SGLImageBuffer	nextblock ;
	SGLImageBuffer	predblock[2] ;
	uint32_t		imgbuf[3][256] ;
	bool			fDiffBlock[3] = { true, true, false } ;	// 差分ブロックか？
	long int		nSumDeflect, nSumSqrDiff[3] ;			// 偏差値
	long int		nSumAbsDiff[3] ;
	//
	// 過去フレームからの動きベクトルを計算する
	//////////////////////////////////////////////////////////////////////////
	predblock[0].ptrBuffer = (uint8_t*) &(imgbuf[0][0]) ;
	SearchMovingVector
		( nextblock, predblock[0],
			dstimg, previmg, xBlock, yBlock, ptMoveVec[0] ) ;
	//
	// 差分フレームを利用するか独立フレームを利用するか判定
	//
	fpDeflectBlock = 0 ;
	nSumDeflect = CalcSumDeflectBlock( nextblock ) ;
	nSumSqrDiff[0] = CalcSumSqrDifferenceBlock( nextblock, predblock[0] ) ;
	nSumAbsDiff[0] = CalcSumAbsDifferenceBlock( nextblock, predblock[0] ) ;
	//
	fpDeflectBlock +=
		sqrt( (double) nSumSqrDiff[0]
				/ (int) (nextblock.width * nextblock.height) ) ;
	//
	if ( (nSumSqrDiff[0] > 8192*64*3)
		|| ((nSumDeflect <= nSumSqrDiff[0])
				&& (nSumSqrDiff[0] > 4096*64*3)) )
	{
		fDiffBlock[0] = (ppredimg != NULL) ;
	}
	//
	// 未来フレームからの動きベクトルを計算する
	//////////////////////////////////////////////////////////////////////////
	SGLImageBuffer	halfblock ;
	if ( ppredimg != NULL )
	{
		predblock[1].ptrBuffer = (uint8_t*) &(imgbuf[1][0]) ;
		SearchMovingVector
			( nextblock, predblock[1],
				dstimg, *ppredimg, xBlock, yBlock, ptMoveVec[1] ) ;
		//
		// 差分フレームを利用するか独立フレームを利用するか判定
		//
		nSumSqrDiff[1] =
			CalcSumSqrDifferenceBlock( nextblock, predblock[1] ) ;
		nSumAbsDiff[1] =
			CalcSumAbsDifferenceBlock( nextblock, predblock[1] ) ;
		//
		fpDeflectBlock +=
			sqrt( (double) nSumSqrDiff[1]
					/ (int) (nextblock.width * nextblock.height) ) ;
		fpDeflectBlock *= 0.5 ;
		/*
		if ( (nSumSqrDiff[1] > 8192*64*3)
			|| ((nSumDeflect <= nSumSqrDiff[1])
					&& (nSumSqrDiff[1] > 4096*64*3)) )
		{
			fDiffBlock[1] = false ;
		}
		*/
		if ( fDiffBlock[0] && fDiffBlock[1] )
		{
			//
			// 両方向予測ブロック
			//
			eslFillMemory( &halfblock, 0, sizeof(SGLImageBuffer) ) ;
			halfblock.format = nextblock.format ;
			halfblock.width = nextblock.width ;
			halfblock.height = nextblock.height ;
			halfblock.depth = 32 ;
			halfblock.pitchPixel = 4 ;
			halfblock.pitchLine = halfblock.width * 4 ;
			halfblock.ptrBuffer = (uint8_t*) &(imgbuf[2][0]) ;
			//
			BlendBlockHalfImage( halfblock, predblock[0], predblock[1] ) ;
			//
			nSumSqrDiff[2] =
				CalcSumSqrDifferenceBlock( nextblock, halfblock ) ;
			nSumAbsDiff[2] =
				CalcSumAbsDifferenceBlock( nextblock, halfblock ) ;
			//
			if ( !((nSumSqrDiff[2] > 8192*64*3)
				|| ((nSumDeflect <= nSumSqrDiff[2])
						&& (nSumSqrDiff[2] > 4096*64*3)))
				&& (nSumAbsDiff[2] <= nSumAbsDiff[0])
				&& (nSumAbsDiff[2] <= nSumAbsDiff[1])
				&& (nSumAbsDiff[0] * 3 > nSumAbsDiff[1])
				&& (nSumAbsDiff[1] * 3 < nSumAbsDiff[0]) )
			{
				fDiffBlock[0] = false ;
				fDiffBlock[1] = false ;
				fDiffBlock[2] = true ;
			}
			else //if ( fDiffBlock[0] && fDiffBlock[1] )
			{
				if ( nSumAbsDiff[0] <= nSumAbsDiff[1] )
				{
					fDiffBlock[1] = false ;
				}
				else
				{
					fDiffBlock[0] = false ;
				}
			}
		}
	}
	else
	{
		ptMoveVec[1].x = 0 ;
		ptMoveVec[1].y = 0 ;
		fDiffBlock[1] = false ;
	}
	/*
	if ( (xBlock + nBlockSize > (int) nextimg.width)
		|| (yBlock + nBlockSize > (int) nextimg.height) )
	{
		fDiffBlock[0] = false ;
		fDiffBlock[1] = false ;
		fDiffBlock[2] = false ;
	}
	*/
	//
	// 差分画像を生成する
	//////////////////////////////////////////////////////////////////////////
	SGLImageBuffer*	pRefBlock = NULL ;
	int				nPredType = 0 ;
	if ( fDiffBlock[2] )
	{
		// 双方向参照ブロック
		pRefBlock = &halfblock ;
		nPredType = 2 ;
	}
	else if ( fDiffBlock[0] )
	{
		// 前方向参照ブロック
		pRefBlock = &predblock[0] ;
		ptMoveVec[1].x = 0 ;
		ptMoveVec[1].y = 0 ;
		nPredType = 1 ;
	}
	else if ( fDiffBlock[1] )
	{
		// 後方向参照ブロック
		pRefBlock = &predblock[1] ;
		ptMoveVec[0].x = 0 ;
		ptMoveVec[0].y = 0 ;
		nPredType = -1 ;
	}
	else
	{
		// 独立ブロック
		MakeBlockValueHalf( nextblock ) ;
		return	0 ;
	}
	int	nAbsDiff = MakeSubtractionBlock( nextblock, *pRefBlock ) ;
	if ( nAbsDiff > nAbsMaxDiff )
	{
		nAbsMaxDiff = nAbsDiff ;
	}
	return	nPredType ;
}

// 動き補償ベクトルを計算する
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SearchMovingVector
	( SakuraGL::SGLImageBuffer & nextblock,
		SakuraGL::SGLImageBuffer & predblock,
		const SakuraGL::SGLImageBuffer & nextimg,
		const SakuraGL::SGLImageBuffer & predimg,
		int xBlock, int yBlock, SakuraGL::SGLPoint & ptMoveVec )
{
	//
	// 画像情報の取得
	//
	const uint32_t	nBlockSize = 16 ;
	ESLAssert( m_nBlockSize == 8 ) ;
	SGLImageBuffer	prvblock ;
	uint32_t		nImageWidth, nImageHeight, nNextPixelBytes, nPredPixelBytes ;
	nextblock = nextimg ;
	prvblock = predimg ;
	nImageWidth = nextimg.width ;
	nImageHeight = nextimg.height ;
	nNextPixelBytes = nextblock.depth / 8 ;
	nPredPixelBytes = prvblock.depth / 8 ;
	//
	// ブロック領域のアドレスを計算する
	//
	nextblock.width = nBlockSize ;
	nextblock.height = nBlockSize ;
	prvblock.width = nBlockSize ;
	prvblock.height = nBlockSize ;
	nextblock.ptrBuffer =
		nextimg.ptrBuffer
			+ xBlock * nNextPixelBytes
			+ yBlock * nextblock.pitchLine ;
	prvblock.ptrBuffer =
		predimg.ptrBuffer
			+ xBlock * nPredPixelBytes
			+ yBlock * prvblock.pitchLine ;
	//
	if ( (yBlock + nBlockSize > nImageHeight)
		|| (xBlock + nBlockSize > nImageWidth) )
	{
		//
		// 画面端の半端サイズのブロックは処理しない
		//
		ptMoveVec.x = 0 ;
		ptMoveVec.y = 0 ;
		//
		// ブロック情報を正規化する
		//
		if ( yBlock + nBlockSize > nImageHeight )
		{
			nextblock.height = nImageHeight - yBlock ;
			prvblock.height = nextblock.height ;
		}
		if ( xBlock + nBlockSize > nImageWidth )
		{
			nextblock.width = nImageWidth - xBlock ;
			prvblock.width = nextblock.width ;
		}
	}
	else
	{
		//
		// 絶対差の合計を計算する
		//
		long int	nBaseDeflect =
			CalcSumSqrDifferenceBlock( nextblock, prvblock ) ;
		long int	nBaseSAD =
			CalcSumAbsDifferenceBlock( nextblock, prvblock ) ;
		//
		// 領域を狭めながら最適なベクトルの選択を繰り返す
		//
		static const SGLPoint	ptVecList[8] =
		{
			SGLPoint( -1, -1 ), SGLPoint( 0, -1 ), SGLPoint( 1, -1 ),
			SGLPoint( -1,  0 ), SGLPoint( 1,  0 ),
			SGLPoint( -1,  1 ), SGLPoint( 0,  1 ), SGLPoint( 1,  1 )
		} ;
		int			i, x, y ;
		SGLPoint	ptBaseVec( 0, 0 ) ;
		SGLPoint	ptBestVec = ptBaseVec ;
		long int	nBestSAD = nBaseSAD ;
		long int	nBestDeflect = nBaseDeflect ;
		for ( y = -17; y <= 17; y += 2 )
		{
			for ( x = -17; x <= 17; x += 2 )
			{
				//
				// 基準ベクトルと、基底の差分ベクトルとを比較する
				//
				SGLPoint	ptDeltaVec ;
				ptDeltaVec.x = ptBaseVec.x + x ;
				ptDeltaVec.y = ptBaseVec.y + y ;
				//
				SGLPoint	ptBlockPos = ptDeltaVec ;
				ptBlockPos.x += xBlock ;
				ptBlockPos.y += yBlock ;
				if ( ((ptBlockPos.x | ptBlockPos.y) < 0)
					|| (ptBlockPos.x + nBlockSize > nImageWidth)
					|| (ptBlockPos.y + nBlockSize > nImageHeight) )
				{
					continue ;
				}
				prvblock.ptrBuffer =
					predimg.ptrBuffer
						+ ptBlockPos.x * nPredPixelBytes
						+ ptBlockPos.y * prvblock.pitchLine ;
				//
				long int	nDeltaSAD =
					CalcSumAbsDifferenceBlock( nextblock, prvblock ) ;
				//
				if ( nDeltaSAD < nBestSAD )
				{
					ptBestVec = ptDeltaVec ;
					nBestSAD = nDeltaSAD ;
				}
			}
		}
		if ( (ptBestVec.x != ptBaseVec.x)
			|| (ptBestVec.y != ptBaseVec.y) )
		{
			prvblock.ptrBuffer =
				predimg.ptrBuffer
					+ (xBlock + ptBestVec.x) * nPredPixelBytes
					+ (yBlock + ptBestVec.y) * prvblock.pitchLine ;
			long int	nDeltaDiff =
				CalcSumSqrDifferenceBlock( nextblock, prvblock ) ;
			if ( nDeltaDiff <= nBaseDeflect )
			{
				ptBaseVec = ptBestVec ;
				nBaseSAD = nBestSAD ;
				nBaseDeflect = nDeltaDiff ;
			}
		}
		ptMoveVec = ptBaseVec ;
		//
		// 半画素精度の動き検出
		//
		long int	nOptSAD = 0x7FFFFFFF ;
		SGLPoint	ptOptVec( 0, 0 ) ;
		for ( i = 0; i < 8; i ++ )
		{
			SGLPoint	ptBlockPos = ptMoveVec ;
			ptBlockPos.x += ptVecList[i].x + xBlock ;
			ptBlockPos.y += ptVecList[i].y + yBlock ;
			if ( ((ptBlockPos.x | ptBlockPos.y) < 0)
				|| (ptBlockPos.x + nBlockSize > nImageWidth)
				|| (ptBlockPos.y + nBlockSize > nImageHeight) )
			{
				continue ;
			}
			prvblock.ptrBuffer =
				predimg.ptrBuffer
					+ ptBlockPos.x * nPredPixelBytes
					+ ptBlockPos.y * prvblock.pitchLine ;
			//
			long int	nDeltaSAD =
				CalcSumAbsDifferenceBlock( nextblock, prvblock ) ;
			//
			if ( nDeltaSAD < nOptSAD )
			{
				ptOptVec = ptVecList[i] ;
				nOptSAD = nDeltaSAD ;
			}
		}
		ptMoveVec.x *= 2 ;
		ptMoveVec.y *= 2 ;
		if ( nOptSAD < nBaseSAD * 5 / 4 )
		{
			ptMoveVec.x += ptOptVec.x ;
			ptMoveVec.y += ptOptVec.y ;
		}
	}
	//
	// 動きベクトル決定
	//
	if ( ptMoveVec.x < -0x7F )
	{
		ptMoveVec.x = -0x7F ;
	}
	else if ( ptMoveVec.x > 0x7F )
	{
		ptMoveVec.x = 0x7F ;
	}
	if ( ptMoveVec.y < -0x7F )
	{
		ptMoveVec.y = -0x7F ;
	}
	else if ( ptMoveVec.y > 0x7F )
	{
		ptMoveVec.y = 0x7F ;
	}
	ESLAssert( xBlock + (ptMoveVec.x >> 1) >= 0 ) ;
	ESLAssert( yBlock + (ptMoveVec.y >> 1) >= 0 ) ;
	ESLAssert( xBlock + ((ptMoveVec.x + 1) >> 1)
						+ prvblock.width <= nImageWidth ) ;
	ESLAssert( yBlock + ((ptMoveVec.y + 1) >> 1)
						+ prvblock.height <= nImageHeight ) ;
	//
	// 画像生成
	//
	predblock.format = prvblock.format ;
	predblock.width = prvblock.width ;
	predblock.height = prvblock.height ;
	predblock.depth = prvblock.depth ;
	predblock.pitchPixel = nPredPixelBytes ;
	predblock.pitchLine = predblock.width * nPredPixelBytes ;
	//
	prvblock.ptrBuffer =
		predimg.ptrBuffer
			+ (xBlock + (ptMoveVec.x >> 1)) * nPredPixelBytes
			+ (yBlock + (ptMoveVec.y >> 1)) * prvblock.pitchLine ;
	//
	SGLImageBuffer	prvblock2 ;
	if ( ptMoveVec.y & 0x01 )
	{
		if ( ptMoveVec.x & 0x01 )
		{
			SGLImageBuffer	bufblock ;
			uint32_t		buf[256] ;
			ESLAssert( m_nBlockSize == 8 ) ;
			//
			prvblock2 = prvblock ;
			prvblock2.ptrBuffer = prvblock.ptrBuffer + nPredPixelBytes ;
			BlendBlockHalfImage( predblock, prvblock, prvblock2 ) ;
			//
			bufblock = prvblock ;
			bufblock.ptrBuffer = (uint8_t*) &buf[0] ;
			bufblock.pitchLine = bufblock.width * nPredPixelBytes ;
			//
			prvblock.ptrBuffer = prvblock.ptrBuffer + prvblock2.pitchLine ;
			//
			prvblock2.ptrBuffer = prvblock2.ptrBuffer + prvblock2.pitchLine ;
			//
			BlendBlockHalfImage( bufblock, prvblock, prvblock2 ) ;
			//
			BlendBlockHalfImage( predblock, predblock, bufblock ) ;
		}
		else
		{
			prvblock2 = prvblock ;
			prvblock2.ptrBuffer = prvblock.ptrBuffer + prvblock.pitchLine ;
			BlendBlockHalfImage( predblock, prvblock, prvblock2 ) ;
		}
	}
	else if ( ptMoveVec.x & 0x01 )
	{
		prvblock2 = prvblock ;
		prvblock2.ptrBuffer = prvblock.ptrBuffer + nPredPixelBytes ;
		BlendBlockHalfImage( predblock, prvblock, prvblock2 ) ;
	}
	else
	{
		sglCopyImageBuffer( predblock, prvblock ) ;
	}
}

// 画像ブロックの二乗偏差（合計）を求める
//////////////////////////////////////////////////////////////////////////////
long int SGLImageEncoder::CalcSumDeflectBlock( const SakuraGL::SGLImageBuffer & imgblock )
{
	int			n, x, y ;
	int			nSum[3] = { 0, 0, 0 } ;
	int			nAvg[3], nDeflect = 0 ;
	int			nWidth = (int) imgblock.width ;
	int			nHeight = (int) imgblock.height ;
	uint8_t *	ptrNextLine = imgblock.ptrBuffer ;
	int			nPixelBytes = (int) imgblock.depth / 8 ;
	for ( y = 0; y < nHeight; y ++ )
	{
		uint8_t *	ptrNextPixel = ptrNextLine ;
		for ( x = 0; x < nWidth; x ++ )
		{
			nSum[0] += (int) ptrNextPixel[0] ;
			nSum[1] += (int) ptrNextPixel[1] ;
			nSum[2] += (int) ptrNextPixel[2] ;
			ptrNextPixel += nPixelBytes ;
		}
		ptrNextLine += imgblock.pitchLine ;
	}
	//
	nAvg[0] = nSum[0] / (nHeight * nWidth) ;
	nAvg[1] = nSum[1] / (nHeight * nWidth) ;
	nAvg[2] = nSum[2] / (nHeight * nWidth) ;
	ptrNextLine = imgblock.ptrBuffer ;
	//
	for ( y = 0; y < nHeight; y ++ )
	{
		uint8_t *	ptrNextPixel = ptrNextLine ;
		for ( x = 0; x < nWidth; x ++ )
		{
			n = (int) ptrNextPixel[0] - nAvg[0] ;
			nDeflect += n * n ;
			n = (int) ptrNextPixel[1] - nAvg[1] ;
			nDeflect += n * n ;
			n = (int) ptrNextPixel[2] - nAvg[2] ;
			nDeflect += n * n ;
			ptrNextPixel += nPixelBytes ;
		}
		ptrNextLine += imgblock.pitchLine ;
	}
	//
	return	nDeflect ;
}

// 画像ブロックの差の二乗偏差（合計）を求める
//////////////////////////////////////////////////////////////////////////////
long int SGLImageEncoder::CalcSumSqrDifferenceBlock
	( const SakuraGL::SGLImageBuffer & dstimg, const SakuraGL::SGLImageBuffer & srcimg )
{
	int			n, x, y ;
	int			nSum[3] = { 0, 0, 0 } ;
	int			nAvg[3], nDeflect = 0 ;
	int			nWidth = (int) dstimg.width ;
	int			nHeight = (int) dstimg.height ;
	uint8_t *	ptrDstNextLine = dstimg.ptrBuffer ;
	uint8_t *	ptrSrcNextLine = srcimg.ptrBuffer ;
	int			nDstPixelBytes = (int) dstimg.depth / 8 ;
	int			nSrcPixelBytes = (int) srcimg.depth / 8 ;
	//
	for ( y = 0; y < nHeight; y ++ )
	{
		uint8_t *	ptrDstNextPixel = ptrDstNextLine ;
		uint8_t *	ptrSrcNextPixel = ptrSrcNextLine ;
		for ( x = 0; x < nWidth; x ++ )
		{
			nSum[0] += (int) ptrDstNextPixel[0] - (int) ptrSrcNextPixel[0] ;
			nSum[1] += (int) ptrDstNextPixel[1] - (int) ptrSrcNextPixel[1] ;
			nSum[2] += (int) ptrDstNextPixel[2] - (int) ptrSrcNextPixel[2] ;
			ptrDstNextPixel += nDstPixelBytes ;
			ptrSrcNextPixel += nSrcPixelBytes ;
		}
		ptrDstNextLine += dstimg.pitchLine ;
		ptrSrcNextLine += srcimg.pitchLine ;
	}
	//
	nAvg[0] = nSum[0] / (nHeight * nWidth) ;
	nAvg[1] = nSum[1] / (nHeight * nWidth) ;
	nAvg[2] = nSum[2] / (nHeight * nWidth) ;
	ptrDstNextLine = dstimg.ptrBuffer ;
	ptrSrcNextLine = srcimg.ptrBuffer ;
	//
	for ( y = 0; y < nHeight; y ++ )
	{
		BYTE *	ptrDstNextPixel = ptrDstNextLine ;
		BYTE *	ptrSrcNextPixel = ptrSrcNextLine ;
		for ( x = 0; x < nWidth; x ++ )
		{
			n = (int) ptrDstNextPixel[0] - (int) ptrSrcNextPixel[0] - nAvg[0] ;
			nDeflect += n * n ;
			n = (int) ptrDstNextPixel[1] - (int) ptrSrcNextPixel[1] - nAvg[1] ;
			nDeflect += n * n ;
			n = (int) ptrDstNextPixel[2] - (int) ptrSrcNextPixel[2] - nAvg[2] ;
			nDeflect += n * n ;
			ptrDstNextPixel += nDstPixelBytes ;
			ptrSrcNextPixel += nSrcPixelBytes ;
		}
		ptrDstNextLine += dstimg.pitchLine ;
		ptrSrcNextLine += srcimg.pitchLine ;
	}
	return	nDeflect ;
}

// 画像ブロックの絶対差の合計を求める
//////////////////////////////////////////////////////////////////////////////
long int SGLImageEncoder::CalcSumAbsDifferenceBlock
	( const SakuraGL::SGLImageBuffer & dstimg, const SakuraGL::SGLImageBuffer & srcimg )
{
	if ( (dstimg.width != srcimg.width)
		|| (dstimg.height != srcimg.height)
		|| (dstimg.depth != srcimg.depth) )
	{
		return	0x7FFFFFFF ;
	}
	if ( (dstimg.depth != 24) && (dstimg.depth != 32) )
	{
		return	0x7FFFFFFF ;
	}
	uint32_t	nPixelBytes = dstimg.depth >> 3 ;
	int32_t		nDstLineBytes = dstimg.pitchLine ;
	int32_t		nSrcLineBytes = srcimg.pitchLine ;
	uint8_t *	pbytDstLine = dstimg.ptrBuffer ;
	uint8_t *	pbytSrcLine = srcimg.ptrBuffer ;
	uint32_t	nWidth = dstimg.width ;
	uint32_t	nHeight = dstimg.height ;
	int32_t		fr = 0, fg = 0, fb = 0, fa = 0 ;
	long int	nSumAbsDiff = 0 ;
	bool		fWithAlpha = (nPixelBytes == 4)
							&& (dstimg.format & formatImageFlagAlpha) ;
	//
	for ( uint32_t y = 0; y < nHeight; y ++ )
	{
		uint8_t *	pbytDst = pbytDstLine ;
		uint8_t *	pbytSrc = pbytSrcLine ;
		if ( fWithAlpha )
		{
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				int32_t	b = (int32_t) pbytDst[0] - pbytSrc[0] ;
				int32_t	g = (int32_t) pbytDst[1] - pbytSrc[1] ;
				int32_t	r = (int32_t) pbytDst[2] - pbytSrc[2] ;
				int32_t	a = (int32_t) pbytDst[3] - pbytSrc[3] ;
				pbytSrc += 4 ;
				pbytDst += 4 ;
				//
				fb = b >> 31 ;
				fg = g >> 31 ;
				fr = r >> 31 ;
				fa = a >> 31 ;
				//
				nSumAbsDiff += ((b ^ fb) - fb)
								+ ((g ^ fg) - fg)
								+ ((r ^ fr) - fr)
								+ ((a ^ fa) - fa) ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				int32_t	b = (int32_t) pbytDst[0] - pbytSrc[0] ;
				int32_t	g = (int32_t) pbytDst[1] - pbytSrc[1] ;
				int32_t	r = (int32_t) pbytDst[2] - pbytSrc[2] ;
				pbytSrc += nPixelBytes ;
				pbytDst += nPixelBytes ;
				//
				fb = b >> 31 ;
				fg = g >> 31 ;
				fr = r >> 31 ;
				//
				nSumAbsDiff += ((b ^ fb) - fb)
								+ ((g ^ fg) - fg)
								+ ((r ^ fr) - fr) ;
			}
		}
		pbytDstLine += nDstLineBytes ;
		pbytSrcLine += nSrcLineBytes ;
	}
	return	nSumAbsDiff ;
}

// 2つの画像ブロックの 50% 合成画像を生成する
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::BlendBlockHalfImage
	( const SakuraGL::SGLImageBuffer & dstimg,
			const SakuraGL::SGLImageBuffer & srcimg1,
			const SakuraGL::SGLImageBuffer & srcimg2 )
{
	if ( (dstimg.width != srcimg1.width)
		|| (dstimg.height != srcimg1.height)
		|| (dstimg.depth != srcimg1.depth) )
	{
		return ;
	}
	if ( (dstimg.width != srcimg2.width)
		|| (dstimg.height != srcimg2.height)
		|| (dstimg.depth != srcimg2.depth) )
	{
		return ;
	}
	if ( (dstimg.depth != 24) && (dstimg.depth != 32) )
	{
		return ;
	}
	uint32_t	nPixelBytes = dstimg.depth >> 3 ;
	int32_t		nDstLineBytes = dstimg.pitchLine ;
	int32_t		nSrc1LineBytes = srcimg1.pitchLine ;
	int32_t		nSrc2LineBytes = srcimg2.pitchLine ;
	uint8_t *	pbytDstLine = dstimg.ptrBuffer ;
	uint8_t *	pbytSrc1Line = srcimg1.ptrBuffer ;
	uint8_t *	pbytSrc2Line = srcimg2.ptrBuffer ;
	uint32_t	nWidth = dstimg.width ;
	uint32_t	nHeight = dstimg.height ;
	//
	for ( uint32_t y = 0; y < nHeight; y ++ )
	{
		uint8_t *	pbytDst = pbytDstLine ;
		uint8_t *	pbytSrc1 = pbytSrc1Line ;
		uint8_t *	pbytSrc2 = pbytSrc2Line ;
		if ( nPixelBytes == 4 )
		{
			uint32_t *	pdwDst = (uint32_t*) pbytDst ;
			uint32_t *	pdwSrc1 = (uint32_t*) pbytSrc1 ;
			uint32_t *	pdwSrc2 = (uint32_t*) pbytSrc2 ;
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				uint32_t	s1 = *(pdwSrc1 ++) ;
				uint32_t	s2 = *(pdwSrc2 ++) ;
				*(pdwDst ++) =
					((s1 >> 1) & 0x7F7F7F7F)
						+ ((s2 >> 1) & 0x7F7F7F7F)
						+ ((s1 & s2) & 0x01010101) ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				pbytDst[0] =
					(uint8_t) (((int) pbytSrc1[0] + pbytSrc1[0] + 1) >> 1) ;
				pbytDst[1] =
					(uint8_t) (((int) pbytSrc1[1] + pbytSrc1[1] + 1) >> 1) ;
				pbytDst[2] =
					(uint8_t) (((int) pbytSrc1[2] + pbytSrc1[2] + 1) >> 1) ;
				pbytDst += nPixelBytes ;
				pbytSrc1 += nPixelBytes ;
				pbytSrc2 += nPixelBytes ;
			}
		}
		pbytDstLine += nDstLineBytes ;
		pbytSrc1Line += nSrc1LineBytes ;
		pbytSrc2Line += nSrc2LineBytes ;
	}
}

// 2つの画像の差分（飽和）と最大絶対差の取得
//////////////////////////////////////////////////////////////////////////////
int SGLImageEncoder::MakeSubtractionBlock
	( const SakuraGL::SGLImageBuffer & dstimg,
			const SakuraGL::SGLImageBuffer & srcimg )
{
	if ( (dstimg.width != srcimg.width)
		|| (dstimg.height != srcimg.height)
		|| (dstimg.depth != srcimg.depth) )
	{
		return	0x200 ;
	}
	if ( (dstimg.depth != 24) && (dstimg.depth != 32) )
	{
		return	0x200 ;
	}
	//
	uint32_t	nPixelBytes = dstimg.depth >> 3 ;
	int32_t		nDstLineBytes = dstimg.pitchLine ;
	int32_t		nSrcLineBytes = srcimg.pitchLine ;
	uint8_t *	pbytDstLine = dstimg.ptrBuffer ;
	uint8_t *	pbytSrcLine = srcimg.ptrBuffer ;
	uint32_t	nWidth = dstimg.width ;
	uint32_t	nHeight = dstimg.height ;
	int32_t		dr = 0, dg = 0, db = 0, da = 0 ;
	int32_t		lr = 0, lg = 0, lb = 0, la = 0 ;
	int32_t		f ;
	//
	for ( uint32_t y = 0; y < nHeight; y ++ )
	{
		uint8_t *	pbytDst = pbytDstLine ;
		uint8_t *	pbytSrc = pbytSrcLine ;
		if ( nPixelBytes == 4 )
		{
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				int32_t	b = (int32_t) pbytDst[0] - (int32_t) pbytSrc[0] ;
				int32_t	g = (int32_t) pbytDst[1] - (int32_t) pbytSrc[1] ;
				int32_t	r = (int32_t) pbytDst[2] - (int32_t) pbytSrc[2] ;
				int32_t	a = (int32_t) pbytDst[3] - (int32_t) pbytSrc[3] ;
				pbytSrc += 4 ;
				pbytDst[0] = (uint8_t) ((b + ((b >> 8) & 1)) >> 1) ;
				pbytDst[1] = (uint8_t) ((g + ((g >> 8) & 1)) >> 1) ;
				pbytDst[2] = (uint8_t) ((r + ((r >> 8) & 1)) >> 1) ;
				pbytDst[3] = (uint8_t) ((a + ((a >> 8) & 1)) >> 1) ;
				pbytDst += 4 ;
				//
				lb -= b ;
				lg -= g ;
				lr -= r ;
				la -= a ;
				//
				f = lb >> 31 ;
				lb = (lb ^ f) - f ;
				f = lg >> 31 ;
				lg = (lg ^ f) - f ;
				f = lr >> 31 ;
				lr = (lr ^ f) - f ;
				f = la >> 31 ;
				la = (la ^ f) - f ;
				//
				f = (lb - db) >> 31 ;
				db = (db & f) | (lb & ~f) ;
				f = (lg - dg) >> 31 ;
				dg = (dg & f) | (lg & ~f) ;
				f = (lr - dr) >> 31 ;
				dr = (dr & f) | (lr & ~f) ;
				f = (la - da) >> 31 ;
				da = (da & f) | (la & ~f) ;
				//
				lb = b ;
				lg = g ;
				lr = r ;
				la = a ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				int32_t	b = (int32_t) pbytDst[0] - (int32_t) pbytSrc[0] ;
				int32_t	g = (int32_t) pbytDst[1] - (int32_t) pbytSrc[1] ;
				int32_t	r = (int32_t) pbytDst[2] - (int32_t) pbytSrc[2] ;
				pbytSrc += 3 ;
				pbytDst[0] = (uint8_t) ((b + ((b >> 8) & 1)) >> 1) ;
				pbytDst[1] = (uint8_t) ((g + ((g >> 8) & 1)) >> 1) ;
				pbytDst[2] = (uint8_t) ((r + ((r >> 8) & 1)) >> 1) ;
				pbytDst += 3 ;
				//
				lb -= b ;
				lg -= g ;
				lr -= r ;
				//
				f = lb >> 31 ;
				lb = (lb ^ f) - f ;
				f = lg >> 31 ;
				lg = (lg ^ f) - f ;
				f = lr >> 31 ;
				lr = (lr ^ f) - f ;
				//
				f = (lb - db) >> 31 ;
				db = (db & f) | (lb & ~f) ;
				f = (lg - dg) >> 31 ;
				dg = (dg & f) | (lg & ~f) ;
				f = (lr - dr) >> 31 ;
				dr = (dr & f) | (lr & ~f) ;
				//
				lb = b ;
				lg = g ;
				lr = r ;
			}
		}
		pbytDstLine += nDstLineBytes ;
		pbytSrcLine += nSrcLineBytes ;
	}
	//
	if ( db < dg )
	{
		db = dg ;
	}
	if ( db < dr )
	{
		db = dr ;
	}
	if ( dstimg.format & formatImageFlagAlpha )
	{
		if ( db < da )
		{
			db = da ;
		}
	}
	//
	return	db ;
}

// 画像ブロックの輝度を半分にする
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::MakeBlockValueHalf( const SakuraGL::SGLImageBuffer & imgblock )
{
	int			x, y ;
	int			nWidth = (int) imgblock.width ;
	int			nHeight = (int) imgblock.height ;
	uint8_t *	ptrNextLine = imgblock.ptrBuffer ;
	int			nPixelBytes = (int) imgblock.depth / 8 ;
	for ( y = 0; y < nHeight; y ++ )
	{
		if ( nPixelBytes == 32 )
		{
			uint32_t *	pdwNextPixel = (uint32_t*) ptrNextLine ;
			for ( x = 0; x < nWidth; x ++ )
			{
				*pdwNextPixel = (*pdwNextPixel >> 1) & 0x7F7F7F7F ;
				pdwNextPixel ++ ;
			}
		}
		else
		{
			uint8_t *	ptrNextPixel = ptrNextLine ;
			for ( x = 0; x < nWidth; x ++ )
			{
				ptrNextPixel[0] >>= 1 ;
				ptrNextPixel[1] >>= 1 ;
				ptrNextPixel[2] >>= 1 ;
				ptrNextPixel += nPixelBytes ;
			}
		}
		ptrNextLine += imgblock.pitchLine ;
	}
}

// 可逆圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::EncodeLosslessImage
	( const SakuraGL::SGLImageBuffer & imginf,
		ERISA::SGLEncodeBitStream & bstream, uint32_t flagsDecoding )
{
	//
	// 関数の準備
	//////////////////////////////////////////////////////////////////////////
	//
	// サンプリング関数取得
	//
	PTR_PROCEDURE	pfnSamplingFunc ;
	m_nSrcLineBytes = imginf.pitchLine ;
	m_nSrcPixelBytes = imginf.depth >> 3 ;
	pfnSamplingFunc =
		GetLLSamplingFunc( imginf.format, imginf.depth, flagsDecoding ) ;
	if ( pfnSamplingFunc == NULL )
	{
		return	errFailed ;
	}
	//
	// コンテキストを初期化し ERI ヘッダを出力
	//
	SGLAbstractEncodeContext *	pContext = NULL ;
	SGLGammaEncodeContext		ctxGamma( &bstream ) ;
	SGLHuffmanEncodeContext		ctxHuffman( &bstream ) ;
	SGLERISAEncodeContext		ctxNemesis( &bstream ) ;
	if ( m_eihInfo.dwArchitecture == erisaNemesisCode )
	{
		//
		// ERISA フォーマット
		//
		bstream.OutNBits( 0x10000008UL, 32 ) ;
		bstream.OutNBits( 0, 1 ) ;
		m_pProbERISA->Initialize( ) ;
		ctxNemesis.PrepareToEncodeERISACode( ) ;
		pContext = &ctxNemesis ;
	}
	else if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
	{
		//
		// ERINA フォーマット
		//
		bstream.OutNBits( 0x08000008UL, 32 ) ;
		bstream.OutNBits( 0, 1 ) ;
		m_pHuffmanTree->Initialize( ) ;
		ctxHuffman.PrepareToEncodeERINACode( ) ;
		pContext = &ctxHuffman ;
	}
	else
	{
		//
		// ERI 互換フォーマット
		//
		ESLAssert( m_eihInfo.dwArchitecture == eriRunlengthGamma ) ;
		bstream.OutNBits( 0x01000000UL, 32 ) ;
		bstream.OutNBits( 0, 1 ) ;
		ctxGamma.PrepareToEncodeGammaCode( ) ;
		pContext = &ctxGamma ;
	}
	//
	// 各ブロックを圧縮して出力する反復処理
	//////////////////////////////////////////////////////////////////////////
	//
	// ラインバッファをクリア
	//
	size_t	nWidthSamples = m_nChannelCount * m_nWidthInBlocks * m_nBlockSize ;
	eslFillMemory( m_ptrLineBuf, 0, nWidthSamples ) ;
	//
	size_t	nPosX, nPosY ;
	size_t	nAllBlockLines = m_nBlockSize * m_nChannelCount ;
	int32_t	nLeftHeight = (int32_t) imginf.height ;
	//
	for ( nPosY = 0; nPosY < m_nHeightInBlocks; nPosY ++ )
	{
		//
		// カラムバッファをクリア
		//
		size_t	nColumnBufSamples = m_nBlockSize * m_nChannelCount ;
		eslFillMemory( m_ptrColumnBuf, 0, nColumnBufSamples ) ;
		//
		// 行の事前処理
		//
		m_ptrSrcBlock =
			imginf.ptrBuffer
				+ ((int32_t) (nPosY * m_nBlockSize) * imginf.pitchLine) ;
		m_nSrcHeight = m_nBlockSize ;
		if ( (int32_t) m_nSrcHeight > nLeftHeight )
		{
			m_nSrcHeight = nLeftHeight ;
		}
		//
		int32_t		nLeftWidth = (int32_t) imginf.width ;
		int8_t *	ptrNextLineBuf = m_ptrLineBuf ;
		//
		for ( nPosX = 0; nPosX < m_nWidthInBlocks; nPosX ++ )
		{
			//
			// ブロックをサンプリング
			//
			m_nSrcWidth = m_nBlockSize ;
			if ( (int32_t) m_nSrcWidth > nLeftWidth )
			{
				m_nSrcWidth = nLeftWidth ;
			}
			if ( (m_nSrcHeight < m_nBlockSize)
					|| (m_nSrcWidth < m_nBlockSize) )
			{
				eslFillMemory( m_ptrArrangeBuf, 0, m_nBlockSamples ) ;
			}
			(this->*pfnSamplingFunc)() ;
			//
			if ( m_nChannelCount >= 3 )
			{
				//
				// オペレーションコードを取得
				//
				uint32_t	nOpCode =
					DecideOperationCode
						( flagsDecoding, nAllBlockLines, ptrNextLineBuf ) ;
				ptrNextLineBuf += nColumnBufSamples ;
				//
				// オペレーションコードを出力
				//
				if ( m_eihInfo.dwArchitecture == erisaNemesisCode )
				{
					// ERISA フォーマット
					ctxNemesis.EncodeERISACodeSymbol( m_pProbERISA, (BYTE) nOpCode ) ;
				}
				else if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
				{
					// ERINA フォーマット
					ctxHuffman.OutHuffmanCode( m_pHuffmanTree, (BYTE) nOpCode ) ;
				}
				else
				{
					// ERI 互換フォーマット
					ESLAssert( m_eihInfo.dwArchitecture == eriRunlengthGamma ) ;
					bstream.OutNBits( (nOpCode << 28), 4 ) ;
				}
			}
			else
			{
				if ( m_eihInfo.fdwFormatType == formatImageGray )
				{
					DifferentialOperation( nAllBlockLines, ptrNextLineBuf ) ;
					ptrNextLineBuf += nColumnBufSamples ;
				}
			}
			//
			// データを符号化
			//
			if ( pContext->Write
					( m_ptrEncodeBuf, m_nBlockSamples ) < m_nBlockSamples )
			{
				return	errFailed ;		// エンコードに失敗
			}
			//
			// 圧縮の状況を通知
			//
			SError	errContinue ;
			errContinue = OnEncodedBlock( nPosY, nPosX ) ;
			if ( errContinue )
			{
				return	errContinue ;
			}
			//
			// 次のブロックへ
			//
			m_ptrSrcBlock += m_nSrcPixelBytes * m_nBlockSize ;
			nLeftWidth -= (int32_t) m_nBlockSize ;
		}
		//
		nLeftHeight -= (int32_t) m_nBlockSize ;
	}
	//
	// 終了
	//
	if ( pContext->FinishEncoding( ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// サンプリングテーブルの初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::InitializeSamplingTable( void )
{
	uint32_t	i, j, k, l, m, n ;
	//
	// サンプリングテーブル用バッファ確保
	//
	uint32_t *	ptrTable = m_bufArrangeTable.GetArray( m_nBlockSamples * 4 ) ;
	uint32_t *	ptrNext ;
	m_pArrangeTable[0] = ptrTable ;
	m_pArrangeTable[1] = ptrTable + m_nBlockSamples ;
	m_pArrangeTable[2] = ptrTable + m_nBlockSamples * 2 ;
	m_pArrangeTable[3] = ptrTable + m_nBlockSamples * 3 ;
	//
	// 水平方向走査
	//
	ptrNext = m_pArrangeTable[0] ;
	for ( i = 0; i < m_nBlockSamples; i ++ )
	{
		ptrNext[i] = i ;
	}
	//
	// 垂直方向走査
	//
	ptrNext = m_pArrangeTable[1] ;
	l = 0 ;
	for ( i = 0; i < m_nChannelCount; i ++ )
	{
		for ( j = 0; j < m_nBlockSize; j ++ )
		{
			m = l + j ;
			for ( k = 0; k < m_nBlockSize; k ++ )
			{
				*(ptrNext ++) = m ;
				m += (uint32_t) m_nBlockSize ;
			}
		}
		l += (uint32_t) m_nBlockArea ;
	}
	//
	// 水平方向インターリーブ
	//
	ptrNext = m_pArrangeTable[2] ;
	for ( i = 0; i < m_nChannelCount; i ++ )
	{
		k = i ;
		for ( j = 0; j < m_nBlockArea; j ++ )
		{
			*(ptrNext ++) = k ;
			k += (uint32_t) m_nChannelCount ;
		}
	}
	//
	// 垂直方向インターリーブ
	//
	ptrNext = m_pArrangeTable[3] ;
	n = (uint32_t) (m_nBlockSize * m_nChannelCount) ;
	for ( i = 0; i < m_nChannelCount; i ++ )
	{
		l = i ;
		for ( j = 0; j < m_nBlockSize; j ++ )
		{
			m = l ;
			l += (uint32_t) m_nChannelCount ;
			for ( k = 0; k < m_nBlockSize; k ++ )
			{
				*(ptrNext ++) = m ;
				m += n ;
			}
		}
	}
}

// 差分処理
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::DifferentialOperation( size_t nAllBlockLines, int8_t * pNextLineBuf )
{
	size_t		i, j, k ;
	int8_t *	ptrLineBuf ;
	int8_t *	ptrNextBuf ;
	int8_t *	ptrNextColBuf ;
	//
	// 差分処理を実行（垂直方向）
	//
	ptrLineBuf = pNextLineBuf ;
	ptrNextBuf = m_ptrEncodeBuf ;
	for ( k = 0; k < m_nChannelCount; k ++ )
	{
		for ( i = 0; i < m_nBlockSize; i ++ )
		{
			for ( j = 0; j < m_nBlockSize; j ++ )
			{
				int8_t	nCurVal = *ptrNextBuf ;
				*(ptrNextBuf ++) -= ptrLineBuf[j] ;
				ptrLineBuf[j] = nCurVal ;
			}
		}
		ptrLineBuf += m_nBlockSize ;
	}
	//
	// 差分処理を実行（水平方向）
	//
	ptrNextBuf = m_ptrEncodeBuf ;
	ptrNextColBuf = m_ptrColumnBuf ;
	for ( i = 0; i < nAllBlockLines; i ++ )
	{
		int8_t	nRightVal = ptrNextBuf[m_nBlockSize - 1] ;
		j = m_nBlockSize ;
		while ( -- j )
		{
			ptrNextBuf[j] -= ptrNextBuf[j - 1] ;
		}
		ptrNextBuf[0] -= ptrNextColBuf[i] ;
		ptrNextBuf += m_nBlockSize ;
		ptrNextColBuf[i] = nRightVal ;
	}
}

// オペレーションコードを取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLImageEncoder::DecideOperationCode
	( uint32_t flagsDecode, size_t nAllBlockLines, int8_t * pNextLineBuf )
{
	size_t		i, j, k ;
	uint32_t	nBestSize, nTrySize ;
	//
	// 最適な差分処理コードを選択
	//
	uint32_t	iBestDifOp ;
	uint32_t	nCompressMode = flagsDecode & flagCmprModeMask ;
	if ( (nCompressMode >= flagNormalCmpr)
		|| (m_eihInfo.dwArchitecture == eriRunlengthGamma) )
	{
		//
		// 差分処理の選択はしない
		//
		iBestDifOp = 0x03 ;
		DifferentialOperation( nAllBlockLines, pNextLineBuf ) ;
		nBestSize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
									( m_ptrEncodeBuf, m_nBlockSamples ) ;
	}
	else // if ( nCompressMode <= flagHighCmpr )
	{
		//
		// 差分処理を実行（垂直方向）
		//
		int8_t *	ptrLineBuf ;
		int8_t *	ptrNextBuf ;
		int8_t *	ptrNextColBuf ;
		ptrLineBuf = pNextLineBuf ;
		ptrNextBuf = m_ptrEncodeBuf ;
		for ( k = 0; k < m_nChannelCount; k ++ )
		{
			for ( i = 0; i < m_nBlockSize; i ++ )
			{
				for ( j = 0; j < m_nBlockSize; j ++ )
				{
					int8_t	nCurVal = *ptrNextBuf ;
					*(ptrNextBuf ++) -= ptrLineBuf[j] ;
					ptrLineBuf[j] = nCurVal ;
				}
			}
			ptrLineBuf += m_nBlockSize ;
		}
		//
		iBestDifOp = 0x02 ;
		nBestSize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
								( m_ptrEncodeBuf, m_nBlockSamples ) ;
		//
		// 中間状態を保存
		//
		eslMoveMemory( m_ptrArrangeBuf, m_ptrEncodeBuf, m_nBlockSamples ) ;
		//
		// 差分処理を実行（水平方向）
		//
		ptrNextBuf = m_ptrEncodeBuf ;
		ptrNextColBuf = m_ptrColumnBuf ;
		for ( i = 0; i < nAllBlockLines; i ++ )
		{
			int8_t	nRightVal = ptrNextBuf[m_nBlockSize - 1] ;
			j = m_nBlockSize ;
			while ( -- j )
			{
				ptrNextBuf[j] -= ptrNextBuf[j - 1] ;
			}
			ptrNextBuf[0] -= ptrNextColBuf[i] ;
			ptrNextBuf += m_nBlockSize ;
			ptrNextColBuf[i] = nRightVal ;
		}
		//
		nTrySize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
								( m_ptrEncodeBuf, m_nBlockSamples ) ;
		if ( nTrySize < nBestSize )
		{
			iBestDifOp = 0x03 ;
			nBestSize = nTrySize ;
		}
		else
		{
			eslMoveMemory
				( m_ptrEncodeBuf, m_ptrArrangeBuf, m_nBlockSamples ) ;
		}
	}
	//
	if ( nCompressMode == flagLowCmpr )
	{
		// カラーオペレーションは使用しない
		return	iBestDifOp << 6 ;
	}
	//
	// アルファチャネルをコピーする
	//
	if ( m_nChannelCount >= 4 )
	{
		int8_t *	ptrSrc = m_ptrEncodeBuf + m_nBlockArea * 3 ;
		int8_t *	ptrDst = m_ptrArrangeBuf + m_nBlockArea * 3 ;
		eslMoveMemory( ptrDst, ptrSrc, m_nBlockArea ) ;
	}
	//
	// 最適なカラーオペレーションを選択
	//
	static const int	iClrOpFull[9] =
	{
		5, 6, 7, 9, 10, 11, 13, 14, 15
	} ;
	static const int	iClrOpHalf[3] =
	{
		7, 11, 15
	} ;
	uint32_t	iBestClrOp = 0 ;
	//
	const int *		pClrOp ;
	unsigned int	nClrOpCount ;
	if ( nCompressMode >= flagHighCmpr )
	{
		pClrOp = iClrOpHalf ;
		nClrOpCount = 3 ;
	}
	else // if ( nCompressMode == flagBestCmpr )
	{
		pClrOp = iClrOpFull ;
		nClrOpCount = 9 ;
	}
	for ( i = 0; i < nClrOpCount; i ++ )
	{
		(this->*m_pfnColorOperation[pClrOp[i]])( ) ;
		//
		nTrySize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
								( m_ptrArrangeBuf, m_nBlockSamples ) ;
		if ( nTrySize < nBestSize )
		{
			iBestClrOp = pClrOp[i] ;
			nBestSize = nTrySize ;
		}
	}
	//
	// カラーオペレーションを実行
	//
	(this->*m_pfnColorOperation[iBestClrOp])( ) ;
	//
	// 最適なアレンジコードを選ぶ
	//
	uint32_t	iBestArrange = 0 ;
	uint32_t *	pArrange ;
	if ( (nCompressMode >= flagNormalCmpr)
		|| (m_eihInfo.dwArchitecture == eriRunlengthGamma) )
	{
		//
		// アレンジコードは使わない
		//
	}
	else if ( nCompressMode == flagHighCmpr )
	{
		//
		// インターリーブの有効性を検証
		//
		pArrange = m_pArrangeTable[2] ;
		for ( j = 0; j < m_nBlockSamples; j ++ )
		{
			m_ptrEncodeBuf[pArrange[j]] = m_ptrArrangeBuf[j] ;
		}
		nTrySize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
								( m_ptrEncodeBuf, m_nBlockSamples ) ;
		if ( nTrySize < nBestSize )
		{
			nBestSize = nTrySize ;
			iBestArrange |= 0x02 ;
		}
		//
		// 垂直走査の有効性を検証
		//
		pArrange = m_pArrangeTable[iBestArrange | 0x01] ;
		for ( j = 0; j < m_nBlockSamples; j ++ )
		{
			m_ptrEncodeBuf[pArrange[j]] = m_ptrArrangeBuf[j] ;
		}
		nTrySize =
			(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
								( m_ptrEncodeBuf, m_nBlockSamples ) ;
		if ( nTrySize < nBestSize )
		{
			nBestSize = nTrySize ;
			iBestArrange |= 0x01 ;
		}
	}
	else // if ( nCompressMode == flagBestCmpr )
	{
		//
		// 総当たり
		//
		for ( i = 1; i < 4; i ++ )
		{
			pArrange = m_pArrangeTable[i] ;
			for ( j = 0; j < m_nBlockSamples; j ++ )
			{
				m_ptrEncodeBuf[pArrange[j]] = m_ptrArrangeBuf[j] ;
			}
			nTrySize =
				(uint32_t) SGLGammaEncodeContext::EstimateGammaCodeBytes
									( m_ptrEncodeBuf, m_nBlockSamples ) ;
			//
			if ( nTrySize < nBestSize )
			{
				nBestSize = nTrySize ;
				iBestArrange = (uint32_t) i ;
			}
		}
	}
	pArrange = m_pArrangeTable[iBestArrange] ;
	for ( i = 0; i < m_nBlockSamples; i ++ )
	{
		m_ptrEncodeBuf[pArrange[i]] = m_ptrArrangeBuf[i] ;
	}
	//
	// オペレーションコードを返す
	//
	return	(iBestDifOp << 6) | (iBestArrange << 4) | iBestClrOp ;
}

// カラーオペレーション関数群
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::ColorOperation0000( void )
{
	eslMoveMemory( m_ptrArrangeBuf, m_ptrEncodeBuf, m_nBlockArea * 3 ) ;
}

void SGLImageEncoder::ColorOperation0001( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[0] = ptrSrc[0] ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] - nBase ;
		ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] -= ptrDst[nChSamples] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0010( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[nChSamples] -= ptrDst[0] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0011( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[nChSamples * 2] -= ptrDst[0] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0100( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[0] = ptrSrc[0] ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] - nBase ;
		ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] -= ptrDst[nChSamples * 2] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0101( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[0] = ptrSrc[0] ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] - nBase ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0110( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[0] = ptrSrc[0] ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] - nBase ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation0111( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[0] = ptrSrc[0] ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] - nBase ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] - nBase ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1000( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[nChSamples] -= ptrDst[nChSamples * 2] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1001( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1010( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[0] = ptrSrc[0] ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] - nBase ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1011( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		(ptrDst ++)[nChSamples * 2] = (ptrSrc ++)[nChSamples * 2] - nBase ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1100( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		ptrDst[nChSamples] = ptrSrc[nChSamples] ;
		ptrDst[nChSamples * 2] -= ptrDst[nChSamples] ;
		ptrSrc ++ ;
		ptrDst ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1101( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		(ptrDst ++)[nChSamples] = (ptrSrc ++)[nChSamples] ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1110( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] = ptrSrc[0] ;
		(ptrDst ++)[nChSamples] = (ptrSrc ++)[nChSamples] - nBase ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageEncoder::ColorOperation1111( void )
{
	int8_t *	ptrSrc = m_ptrEncodeBuf ;
	int8_t *	ptrDst = m_ptrArrangeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	int8_t		nBase ;
	//
	do
	{
		nBase = ptrDst[nChSamples * 2] = ptrSrc[nChSamples * 2] ;
		ptrDst[0] = ptrSrc[0] - nBase ;
		(ptrDst ++)[nChSamples] = (ptrSrc ++)[nChSamples] - nBase ;
	}
	while ( -- nRepCount ) ;
}

// グレイ画像のサンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SamplingGray8( void )
{
	uint8_t *	ptrSrcLine = m_ptrSrcBlock ;
	int8_t *	ptrDstLine = m_ptrEncodeBuf ;
	//
	for ( size_t y = 0; y < m_nSrcHeight; y ++ )
	{
		for ( size_t x = 0; x < m_nSrcWidth; x ++ )
		{
			ptrDstLine[x] = (int8_t) ptrSrcLine[x] ;
		}
		ptrSrcLine += m_nSrcLineBytes ;
		ptrDstLine += m_nBlockSize ;
	}
}

// RGB 画像(15ビット)のサンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SamplingRGB16( void )
{
	uint8_t *	ptrSrcLine = m_ptrSrcBlock ;
	int8_t *	ptrDstLine = m_ptrEncodeBuf ;
	size_t		nBlockSamples = m_nBlockArea ;
	//
	for ( size_t y = 0; y < m_nSrcHeight; y ++ )
	{
		uint16_t *	ptrSrcNext = (uint16_t*) ptrSrcLine ;
		int8_t *	ptrDstNext = ptrDstLine ;
		//
		for ( size_t x = 0; x < m_nSrcWidth; x ++ )
		{
			uint16_t	wSrcPixel = *(ptrSrcNext ++) ;
			*ptrDstNext = wSrcPixel & 0x1F ;
			ptrDstNext[nBlockSamples] = (wSrcPixel >> 5) & 0x1F ;
			ptrDstNext[nBlockSamples * 2] = (wSrcPixel >> 10) & 0x1F ;
			ptrDstNext ++ ;
		}
		ptrSrcLine += m_nSrcLineBytes ;
		ptrDstLine += m_nBlockSize ;
	}
}

// RGB 画像のサンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SamplingRGB24( void )
{
	uint8_t *	ptrSrcLine = m_ptrSrcBlock ;
	int8_t *	ptrDstLine = m_ptrEncodeBuf ;
	size_t		nBytesPerPixel = m_nSrcPixelBytes ;
	size_t		nBlockSamples = m_nBlockArea ;
	//
	for ( size_t y = 0; y < m_nSrcHeight; y ++ )
	{
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		int8_t *	ptrDstNext = ptrDstLine ;
		//
		for ( size_t x = 0; x < m_nSrcWidth; x ++ )
		{
			ptrDstNext[0] = ptrSrcNext[0] ;
			ptrDstNext[nBlockSamples] = ptrSrcNext[1] ;
			ptrDstNext[nBlockSamples * 2] = ptrSrcNext[2] ;
			ptrSrcNext += nBytesPerPixel ;
			ptrDstNext ++ ;
		}
		ptrSrcLine += m_nSrcLineBytes ;
		ptrDstLine += m_nBlockSize ;
	}
}

// RGBA 画像のサンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SamplingRGBA32( void )
{
	uint8_t *	ptrSrcLine = m_ptrSrcBlock ;
	int8_t *	ptrDstLine = m_ptrEncodeBuf ;
	size_t		nBlockSamples = m_nBlockArea ;
	size_t		nBlockSamplesX3 = nBlockSamples * 3 ;
	//
	for ( size_t y = 0; y < m_nSrcHeight; y ++ )
	{
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		int8_t *	ptrDstNext = ptrDstLine ;
		//
		for ( size_t x = 0; x < m_nSrcWidth; x ++ )
		{
			ptrDstNext[0] = ptrSrcNext[0] ;
			ptrDstNext[nBlockSamples] = ptrSrcNext[1] ;
			ptrDstNext[nBlockSamples * 2] = ptrSrcNext[2] ;
			ptrDstNext[nBlockSamplesX3] = ptrSrcNext[3] ;
			ptrSrcNext += 4 ;
			ptrDstNext ++ ;
		}
		ptrSrcLine += m_nSrcLineBytes ;
		ptrDstLine += m_nBlockSize ;
	}
}

// 画像をサンプリングする関数へのポインタを取得する
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoder::PTR_PROCEDURE SGLImageEncoder::GetLLSamplingFunc
	( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecode )
{
	switch ( nBitsPerPixel )
	{
	case	32:
		if ( m_eihInfo.fdwFormatType & formatImageFlagAlpha )
		{
			return	&SGLImageEncoder::SamplingRGBA32 ;
		}
	case	24:
		return	&SGLImageEncoder::SamplingRGB24 ;
	case	16:
		return	&SGLImageEncoder::SamplingRGB16 ;
	case	8:
		return	&SGLImageEncoder::SamplingGray8 ;
	}
	return	NULL ;
}

// 非可逆圧縮
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageEncoder::EncodeLossyImage
	( const SakuraGL::SGLImageBuffer & imginf,
		ERISA::SGLEncodeBitStream & bstream, uint32_t flagsDecoding )
{
	//
	// 関数の準備
	//////////////////////////////////////////////////////////////////////////
	//
	// LOT 変換・DCT 変換の切り替え
	//
	DWORD	fdwOrgTrans = m_eihInfo.fdwTransformation ;
	CalcImageSizeInBlocks
		( (flagsDecoding & flagDifferential)
					? eriTransformationDCT : fdwOrgTrans ) ;
	//
	// サンプリング関数の取得
	//
	PTR_PROCEDURE	pfnSamplingFunc ;
	m_nSrcLineBytes = imginf.pitchLine ;
	m_nSrcPixelBytes = imginf.depth >> 3 ;
	m_flagsEncode = flagsDecoding ;
	pfnSamplingFunc =
		GetLSSamplingFunc( imginf.format, imginf.depth, flagsDecoding ) ;
	if ( pfnSamplingFunc == NULL )
	{
		return	errFailed ;
	}
	//
	// 量子化テーブルの生成
	//
	InitializeQuantumizeTable( ) ;
	//
	// バッファの初期化
	//
	eslFillMemory
		( m_ptrVertBufLOT, 0,
			m_nBlockSamples * 2 * m_nWidthInBlocks * sizeof(float32_t) ) ;
	//
	// 各ブロックごとの処理
	//////////////////////////////////////////////////////////////////////////
	size_t		i, j, k ;
	size_t		nPosX, nPosY ;
	int32_t		nLeftHeight = imginf.height ;
	int32_t		nBlockStepAddr = imginf.pitchLine * (int32_t) m_nBlockSize ;
	int8_t *	ptrCoefficient = m_ptrCoefficient ;
	uint8_t *	ptrImageDst = m_ptrImageDst ;
	float32_t	fpMatrixScale = (float32_t) (2.0 / (int32_t) m_nBlockSize) ;
	float32_t *	ptrNextSignalBuf = m_ptrSignalBuf ;
	//
	for ( nPosY = 0; nPosY < m_nHeightInBlocks; nPosY ++ )
	{
		//
		// LOT 変換バッファをクリア
		//
		eslFillMemory
			( m_ptrHorzBufLOT, 0, m_nBlockSamples * 2 * sizeof(float32_t) ) ;
		//
		uint8_t *	ptrSrcLineAddr =
			imginf.ptrBuffer + ((int32_t) nPosY * nBlockStepAddr * 2) ;
		int32_t		nLeftWidth = (int32_t) imginf.width ;
		float32_t *	ptrVertBufLOT = m_ptrVertBufLOT ;
		//
		for ( nPosX = 0; nPosX < m_nWidthInBlocks; nPosX ++ )
		{
			//
			// ブロックをサンプリング（4:4:4 形式で）＆色空間変換
			//
			SamplingMacroBlock
				( (int) nPosX, (int) nPosY, nLeftWidth, nLeftHeight,
					nBlockStepAddr, ptrSrcLineAddr, pfnSamplingFunc ) ;
			//
			// スケーリング
			//
			if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
			{
				BlockScaling444( ) ;
			}
			else // if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
			{
				BlockScaling411( ) ;
			}
			//
			// LOT 変換
			//
			if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
			{
				MatrixLOT8x8( ptrVertBufLOT ) ;
			}
			else
			{
				MatrixDCT8x8( ) ;
			}
			sclfScalarMultiply
				( m_ptrBlocksetBuf[0],
					fpMatrixScale, m_nBlockArea * m_nBlocksetCount ) ;
			ptrVertBufLOT += m_nBlockArea * 2 * m_nChannelCount ;
			//
			eslMoveMemory
				( ptrNextSignalBuf, m_ptrBlocksetBuf[0],
					m_nBlockArea * m_nBlocksetCount * sizeof(float32_t) ) ;
			ptrNextSignalBuf += m_nBlockArea * m_nBlocksetCount ;
			//
			// 量子化
			//
			uint32_t	fdwBlockFlags = flagsDecoding ;
			if ( m_nMovingVector > 0 )
			{
				if ( m_pMoveVecFlags[nPosY * m_nWidthInBlocks + nPosX] == 0 )
				{
					fdwBlockFlags &= ~flagDifferential ;
				}
			}
			ArrangeAndQuantumize( ptrCoefficient, fdwBlockFlags ) ;
			for ( i = 0; i < m_nBlockArea; i ++ )
			{
				for ( j = 0; j < m_nBlocksetCount; j ++ )
				{
					ptrImageDst[i * m_nBlocksetCount + j] =
							m_ptrEncodeBuf[i + j * m_nBlockArea] ;
				}
			}
			ptrCoefficient += 2 ;
			ptrImageDst += m_nBlockArea * m_nBlocksetCount ;
			//
			// 圧縮の状況を通知
			//
			SError	errContinue ;
			errContinue = OnEncodedBlock( nPosY, nPosX ) ;
			if ( errContinue )
			{
				return	errContinue ;
			}
			//
			// 次のブロックへ
			//
			nLeftWidth -= (int32_t) m_nBlockSize * 2 ;
		}
		//
		nLeftHeight -= (int32_t) m_nBlockSize * 2 ;
	}
	//
	// レート制御
	//////////////////////////////////////////////////////////////////////////
	size_t	nTotalBlocks = m_nWidthInBlocks * m_nHeightInBlocks ;
	size_t	nTotalSamples = nTotalBlocks * m_nBlockArea * m_nBlocksetCount ;
	//
	double	fpRatio = 1.0 ;
	if ( m_prmCmprOpt.m_nMinFrameSize < m_prmCmprOpt.m_nMaxFrameSize )
	{
		do
		{
			//
			// レート判定
			//
			size_t	nEstimatedSize =
				SGLGammaEncodeContext::EstimateGammaCodeBytes
					( (const SBYTE *) m_ptrImageDst, nTotalSamples ) / 8 ;
			//
			double	r ;
			if ( m_prmCmprOpt.m_nMinFrameSize > nEstimatedSize )
			{
				r = pow( (double) (long int)
						m_prmCmprOpt.m_nMinFrameSize
								/ (long int) nEstimatedSize, 0.85 ) ;
				if ( r >= 32.0 )
				{
					r = 32.0 ;
				}
			}
			else if ( m_prmCmprOpt.m_nMaxFrameSize < nEstimatedSize )
			{
				r = pow( (double) (long int)
						m_prmCmprOpt.m_nMaxFrameSize
								/ (long int) nEstimatedSize, 0.8 ) ;
				if ( r <= 0.125 )
				{
					r = 0.125 ;
				}
			}
			else
			{
				break ;
			}
			//
			// 再量子化ステップ
			//
			fpRatio *= r ;
			ptrCoefficient = m_ptrCoefficient ;
			ptrImageDst = m_ptrImageDst ;
			ptrNextSignalBuf = m_ptrSignalBuf ;
			InitializeQuantumizeTable( fpRatio ) ;
			//
			for ( k = 0; k < nTotalBlocks; k ++ )
			{
				eslMoveMemory
					( m_ptrBlocksetBuf[0], ptrNextSignalBuf,
						m_nBlockArea * m_nBlocksetCount * sizeof(float32_t) ) ;
				ptrNextSignalBuf += m_nBlockArea * m_nBlocksetCount ;
				//
				uint32_t	fdwBlockFlags = flagsDecoding ;
				if ( m_nMovingVector > 0 )
				{
					if ( m_pMoveVecFlags[k] == 0 )
					{
						fdwBlockFlags &= ~flagDifferential ;
					}
				}
				ArrangeAndQuantumize( ptrCoefficient, fdwBlockFlags ) ;
				for ( i = 0; i < m_nBlockArea; i ++ )
				{
					for ( j = 0; j < m_nBlocksetCount; j ++ )
					{
						ptrImageDst[i * m_nBlocksetCount + j] =
								m_ptrEncodeBuf[i + j * m_nBlockArea] ;
					}
				}
				ptrCoefficient += 2 ;
				ptrImageDst += m_nBlockArea * m_nBlocksetCount ;
			}
		}
		while ( false ) ;
	}
	//
	// 符号化して出力
	//////////////////////////////////////////////////////////////////////////
	//
	// コンテキストを初期化し ERI ヘッダを出力
	//
	DWORD	dwHeader = 0 ;
	if ( m_nMovingVector > 0 )
	{
		// 動き補償
		dwHeader |= 0x00010000UL ;
		//
		if ( m_fPredFrameType == 2 )
		{
			// 未来フレームを参照
			dwHeader |= 0x00020000UL ;
		}
	}
	//
	// ループフィルタ判定
	//
	if ( m_prmCmprOpt.m_nFlags & pfUseLoopFilter )
	{
		dwHeader |= 0x00040000UL ;
	}
	if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
	{
		dwHeader |= 0x00000100UL ;
	}
	ESLAssert( m_eihInfo.dwArchitecture == eriRunlengthGamma ) ;
	dwHeader |= 0x28000008UL ;
	bstream.OutNBits( dwHeader, 32 ) ;
	bstream.OutNBits( 0, 1 ) ;
	m_pHuffmanTree->Initialize( ) ;
	//
	SGLHuffmanEncodeContext	ctxHuffman( &bstream ) ;
	ctxHuffman.PrepareToEncodeERINACode
			( SGLHuffmanEncodeContext::efERINAOrder0 ) ;
	//
	// 逆量子化テーブルを出力する
	//
	for ( i = 0; i < m_nBlockArea * 2; i ++ )
	{
		if ( ctxHuffman.OutHuffmanCode
			( m_pHuffmanTree, (uint8_t) m_pQuantumizeTable[i] ) )
		{
			return	errFailed ;
		}
	}
	//
	// ブロックスケーリング係数を出力する
	//
	if ( ctxHuffman.EncodeGammaCodeBytes
			( m_ptrCoefficient, nTotalBlocks * 2 ) < nTotalBlocks * 2 )
	{
		return	errFailed ;
	}
	//
	// 動き補償ベクトルを出力する
	//
	if ( m_nMovingVector > 0 )
	{
		if ( ctxHuffman.EncodeGammaCodeBytes
			( (SBYTE*) m_pMoveVecFlags, m_nMovingVector )
											< m_nMovingVector )
		{
			return	errFailed ;
		}
		if ( ctxHuffman.EncodeGammaCodeBytes
			( m_pMovingVector, m_nMovingVector * 4 )
										< m_nMovingVector * 4 )
		{
			return	errFailed ;
		}
		m_nMovingVector = 0 ;
		m_nIntraBlockCount = 0 ;
		m_fpDiffDeflectBlock = 0 ;
		m_fpMaxDeflectBlock = 0 ;
	}
	//
	// 画像信号を出力する
	//
	if ( ctxHuffman.EncodeERINACodeBytes
		( (const SBYTE *) m_ptrImageDst, nTotalSamples ) < nTotalSamples )
	{
		return	errFailed ;
	}
	if ( ctxHuffman.FinishEncoding( ) )
	{
		return	errFailed ;
	}
	m_eihInfo.fdwTransformation = fdwOrgTrans ;
	//
	return	errSuccess ;
}

// ブロック単位での画面サイズを計算する
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::CalcImageSizeInBlocks( DWORD fdwTransformation )
{
	m_eihInfo.fdwTransformation = fdwTransformation ;
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		m_nWidthInBlocks =
			((m_eihInfo.nImageWidth + m_nBlockSize - 1)
									>> m_eihInfo.dwBlockingDegree) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			m_nHeightInBlocks = (size_t) - m_eihInfo.nImageHeight ;
		}
		else
		{
			m_nHeightInBlocks = (size_t) m_eihInfo.nImageHeight ;
		}
		m_nHeightInBlocks =
			(m_nHeightInBlocks + m_nBlockSize - 1)
									>> m_eihInfo.dwBlockingDegree ;
		//
		if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
		{
			m_nBlocksetCount = m_nChannelCount * 4 ;
			m_nWidthInBlocks = (m_nWidthInBlocks >> 1) + 1  ;
			m_nHeightInBlocks = (m_nHeightInBlocks >> 1) + 1 ;
		}
		else if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
		{
			m_nWidthInBlocks = ((m_nWidthInBlocks + 1) >> 1) + 1  ;
			m_nHeightInBlocks = ((m_nHeightInBlocks + 1) >> 1) + 1 ;
		}
	}
	else
	{
		m_nWidthInBlocks =
			((m_eihInfo.nImageWidth + (m_nBlockSize * 2 - 1))
								>> (m_eihInfo.dwBlockingDegree + 1)) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			m_nHeightInBlocks = (size_t) - m_eihInfo.nImageHeight ;
		}
		else
		{
			m_nHeightInBlocks = (size_t) m_eihInfo.nImageHeight ;
		}
		m_nHeightInBlocks =
			(m_nHeightInBlocks + (m_nBlockSize * 2 - 1))
							>> (m_eihInfo.dwBlockingDegree + 1) ;
	}
}

// サンプリングテーブルの初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::InitializeZigZagTable( void )
{
	uint32_t *	ptrArrange = m_bufArrangeTable.GetArray( m_nBlockArea * 2 ) ;
	m_pArrangeTable[0] = ptrArrange ;
	m_pArrangeTable[1] = ptrArrange + m_nBlockArea ;
	//
	size_t	i = 0 ;
	int		x = 0, y = 0 ;
	for ( ; ; )
	{
		for ( ; ; )
		{
			ptrArrange[i ++] = (uint32_t) (x + y * m_nBlockSize) ;
			if ( i >= m_nBlockArea )
				goto	Label_Next ;
			++ x ;
			-- y ;
			if ( x >= (int) m_nBlockSize )
			{
				-- x ;
				y += 2 ;
				break ;
			}
			else if ( y < 0 )
			{
				y = 0 ;
				break ;
			}
		}
		for ( ; ; )
		{
			ptrArrange[i ++] = (uint32_t) (x + y * m_nBlockSize) ;
			if ( i >= m_nBlockArea )
				goto	Label_Next ;
			++ y ;
			-- x ;
			if ( y >= (int) m_nBlockSize )
			{
				-- y ;
				x += 2 ;
				break ;
			}
			else if ( x < 0 )
			{
				x = 0 ;
				break ;
			}
		}
	}
Label_Next:
	//
	for ( i = 0; i < m_nBlockArea; i ++ )
	{
		m_pArrangeTable[1][ptrArrange[i]] = (uint32_t) i ;
	}
}

// 量子化テーブルの生成
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::InitializeQuantumizeTable( double r )
{
	ESLAssert( m_nBlockSize == 8 ) ;
	double	q[2][3] =
	{
		{
			m_prmCmprOpt.m_fpYScaleDC * r,
			m_prmCmprOpt.m_fpYScaleLow * r,
			m_prmCmprOpt.m_fpYScaleHigh * r
		},
		{
			m_prmCmprOpt.m_fpCScaleDC * r,
			m_prmCmprOpt.m_fpCScaleLow * r,
			m_prmCmprOpt.m_fpCScaleHigh * r
		}
	} ;
	for ( int i = 0; i < 2; i ++ )
	{
		//
		// 低周波成分の量子化パラメータ
		//
		int			j, k ;
		float32_t *	pQScale = m_pQuantumizeScale[i] ;
		uint8_t *	pQTable = m_pQuantumizeTable + i * m_nBlockArea ;
		float32_t	rq ;
		int			iq ;
		for ( j = 0; j < 7; j ++ )
		{
			for ( k = 0; k <= j; k ++ )
			{
				//
				// 量子化パラメータの補完と正規化
				//
				double	r = 1.0 ;
				if ( j != 0 )
				{
					r = pow( (double) (j - k) / j, 2.0 )
								+ pow( (double) k / j, 2.0 ) ;
				}
				rq = (float32_t)
					((q[i][0] + (q[i][1] - q[i][0])
							* pow( (j * 0.16666667), 0.7 )) * r) ;
				if ( rq > 0.0001 )
				{
					iq = eslRoundR32ToInt( 1.0f / rq ) ;
					if ( iq <= 0 )
					{
						iq = 1 ;
					}
					else if ( iq > 0x100 )
					{
						iq = 0x100 ;
					}
				}
				else
				{
					iq = 0x100 ;
				}
				rq = (float32_t) (1.0 / iq) ;
				iq -- ;
				//
				// パラメータ設定
				//
				pQScale[(j - k) * 8 + k] = rq ;
				pQTable[(j - k) * 8 + k] = (uint8_t) iq ;
			}
		}
		//
		// 高周波成分の量子化パラメータ
		//
		for ( j = 7; j < 15; j ++ )
		{
			for ( k = 7; k >= j - 7; k -- )
			{
				//
				// 量子化パラメータの補完と正規化
				//
				double	r = pow( (double) (j - k) / j, 2.0 )
								+ pow( (double) k / j, 2.0 ) ;
				rq = (float32_t)
					((q[i][1] + (q[i][2] - q[i][1]) * (j * 0.125f)) * r) ;
				if ( rq > 0.0001 )
				{
					iq = eslRoundR32ToInt( 1.0f / rq ) ;
					if ( iq <= 0 )
					{
						iq = 1 ;
					}
					else if ( iq > 0x100 )
					{
						iq = 0x100 ;
					}
				}
				else
				{
					iq = 0x100 ;
				}
				rq = (float32_t) (1.0 / iq) ;
				iq -- ;
				//
				// パラメータ設定
				//
				pQScale[(j - k) * 8 + k] = rq ;
				pQTable[(j - k) * 8 + k] = (uint8_t) iq ;
			}
		}
		//
		if ( (m_eihInfo.fdwTransformation == eriTransformationLOT)
									&& (m_pArrangeTable[0] != NULL) )
		{
			uint32_t *	pArrange = m_pArrangeTable[1] ;
			for ( int y = 1; y < 8; y += 2 )
			{
				for ( int x = 0; x < 8; x ++ )
				{
					j = pArrange[y * 8 + x] ;
					float32_t	rq = pQScale[j] ;
					if ( x & 0x01 )
						rq *= 1.75f ;
					else
						rq *= 1.4f ;
					//
					if ( rq > 0.0001 )
					{
						iq = eslRoundR32ToInt( 1.0f / rq ) ;
						if ( iq <= 0 )
						{
							iq = 1 ;
						}
						else if ( iq > 0x100 )
						{
							iq = 0x100 ;
						}
					}
					else
					{
						iq = 0x100 ;
					}
					rq = (float32_t) (1.0 / iq) ;
					iq -- ;
					pQScale[j] = rq ;
					pQTable[j] = (uint8_t) iq ;
				}
			}
		}
	}
}

// マクロブロックのサンプリング（4:4:4 形式）＆色空間変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::SamplingMacroBlock
	( int xBlock, int yBlock,
		int nLeftWidth, int nLeftHeight,
		int32_t nBlockStepAddr, uint8_t*& ptrSrcLineAddr,
		SGLImageEncoder::PTR_PROCEDURE pfnSamplingFunc )
{
	//
	// ブロックをサンプリング（4:4:4 形式で）
	//
	size_t		i, j, k ;
	uint8_t *	ptrSrcBlocks[9] ;
	ssize_t		nWidths[9], nHeights[9] ;
	size_t		nSubBlocks = 9 ;
	size_t		xSub, ySub ;
	size_t		nSubLeftHeight = (size_t) nLeftHeight + m_nBlockSize ;
	size_t		nBlockWidthBytes = m_nBlockSize * m_nSrcPixelBytes ;
	uint8_t *	ptrBlockLine =
					ptrSrcLineAddr - nBlockStepAddr - nBlockWidthBytes ;
	i = 0 ;
	for ( ySub = 0; ySub < 3; ySub ++ )
	{
		uint8_t *	ptrNextBlock = ptrBlockLine ;
		ssize_t		nSubLeftWidth = nLeftWidth + (ssize_t) m_nBlockSize ;
		for ( xSub = 0; xSub < 3; xSub ++ )
		{
			if ( ((xSub + xBlock) == 0) || ((ySub + yBlock) == 0) )
			{
				nWidths[i] = 0 ;
				nHeights[i] = 0 ;
			}
			else
			{
				ptrSrcBlocks[i] = ptrNextBlock ;
				nWidths[i] = nSubLeftWidth ;
				nHeights[i] = (ssize_t) nSubLeftHeight ;
			}
			i ++ ;
			ptrNextBlock += nBlockWidthBytes ;
			nSubLeftWidth -= (ssize_t) m_nBlockSize ;
		}
		ptrBlockLine += nBlockStepAddr ;
		nSubLeftHeight -= m_nBlockSize ;
	}
	ptrSrcLineAddr += (m_nBlockSize * m_nSrcPixelBytes) * 2 ;
	//
	for ( i = 0; i < nSubBlocks; i ++ )
	{
		//
		// 各サブブロックをサンプリング
		//
		m_ptrSrcBlock = ptrSrcBlocks[i] ;
		if ( (nWidths[i] <= 0) || (nHeights[i] <= 0) )
		{
			// 画像領域外：0で初期化
			size_t	nBytes = m_nBlockArea * sizeof(float32_t) ;
			for ( j = 0; j < m_nChannelCount; j ++ )
			{
				eslFillMemory
					( m_ptrBlocksetBuf[i + j * nSubBlocks], 0, nBytes ) ;
			}
			ptrSrcBlocks[i] = NULL ;
			continue ;
		}
		m_nSrcWidth = (size_t) nWidths[i] ;
		m_nSrcHeight = (size_t) nHeights[i] ;
		if ( m_nSrcWidth > m_nBlockSize )
		{
			m_nSrcWidth = m_nBlockSize ;
		}
		if ( m_nSrcHeight > m_nBlockSize )
		{
			m_nSrcHeight = m_nBlockSize ;
		}
		//
		// サンプリング
		(this->*pfnSamplingFunc)( ) ;
		FillBlockOddArea( m_flagsEncode ) ;
		//
		// 浮動小数点形式へ変換
		if ( m_flagsEncode & flagDifferential )
		{
			int8_t *	ptrEncodeBuf = m_ptrEncodeBuf ;
			for ( j = 0; j < m_nChannelCount; j ++ )
			{
				float32_t *	pDstBuf = m_ptrBlocksetBuf[i + j * nSubBlocks] ;
				for ( k = 0; k < m_nBlockArea; k ++ )
				{
					pDstBuf[k] = ptrEncodeBuf[k] ;
				}
				ptrEncodeBuf += m_nBlockArea ;
			}
		}
		else
		{
			uint8_t *	ptrEncodeBuf = (uint8_t*) m_ptrEncodeBuf ;
			for ( j = 0; j < (INT) m_nChannelCount; j ++ )
			{
				float32_t *	pDstBuf = m_ptrBlocksetBuf[i + j * nSubBlocks] ;
				for ( k = 0; k < m_nBlockArea; k ++ )
				{
					pDstBuf[k] = ptrEncodeBuf[k] ;
				}
				ptrEncodeBuf += m_nBlockArea ;
			}
		}
	}
	//
	// LOT 変換時の画面端ブロックのための処理
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		for ( i = 0; i < nSubBlocks; i ++ )
		{
			if ( ptrSrcBlocks[i] != NULL )
			{
				continue ;
			}
			int	x, y ;
			k = i ;
			x = (int) i % 3 ;
			y = (int) i / 3 ;
			if ( ptrSrcBlocks[y * 3 + 1] != NULL )
			{
				k = y * 3 + 1 ;
			}
			else if ( ptrSrcBlocks[3 + x] != NULL )
			{
				k = 3 + x ;
			}
			else if ( ptrSrcBlocks[y * 3] != NULL )
			{
				k = y * 3 ;
			}
			else if ( ptrSrcBlocks[x] != NULL )
			{
				k = x ;
			}
			else if ( ptrSrcBlocks[y * 3 + 2] != NULL )
			{
				k = y * 3 + 2 ;
			}
			else if ( ptrSrcBlocks[6 + x] != NULL )
			{
				k = 6 + x ;
			}
			else if ( ptrSrcBlocks[3 + 1] != NULL )
			{
				k = 3 + 1 ;
			}
			else if ( (ptrSrcBlocks[0] != NULL)
						&& (ptrSrcBlocks[6 + 2] == NULL) )
			{
				k = 0 ;
			}
			else if ( (ptrSrcBlocks[6 + 2] != NULL)
						&& (ptrSrcBlocks[0] == NULL) )
			{
				k = 6 + 2 ;
			}
			else if ( (ptrSrcBlocks[2] != NULL)
						&& (ptrSrcBlocks[6] == NULL) )
			{
				k = 2 ;
			}
			else if ( (ptrSrcBlocks[6] != NULL)
						&& (ptrSrcBlocks[2] == NULL) )
			{
				k = 6 ;
			}
			else
			{
				continue ;
			}
			//
			for ( j = 0; j < m_nChannelCount; j ++ )
			{
				float32_t *	pDstBlock = m_ptrBlocksetBuf[i + j * nSubBlocks] ;
				float32_t *	pSrcBlock = m_ptrBlocksetBuf[k + j * nSubBlocks] ;
				float32_t	rAvgBlock = 0 ;
				for ( x = 0; x < 64; x ++ )
				{
					rAvgBlock += pSrcBlock[x] ;
				}
				rAvgBlock /= 64.0f ;
				//
				for ( x = 0; x < 64; x ++ )
				{
					pDstBlock[x] = rAvgBlock ;
				}
			}
		}
	}
	//
	// RGB->YUV 色空間変換
	//
	if ( m_nChannelCount >= 3 )
	{
		sclfConvertRGBtoYUV
			( m_ptrBlocksetBuf[0],
				m_ptrBlocksetBuf[nSubBlocks],
				m_ptrBlocksetBuf[nSubBlocks*2],
				m_nBlockArea * nSubBlocks ) ;
	}
}

// 半端領域に平均値を設定
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::FillBlockOddArea( uint32_t flagsDecoding )
{
	if ( (m_nSrcWidth >= m_nBlockSize)
		&& (m_nSrcHeight >= m_nBlockSize) )
	{
		return ;
	}
	for ( size_t i = 0; i < m_nChannelCount; i ++ )
	{
		size_t		x, y ;
		uint8_t *	ptrBuf ;
		uint8_t *	ptrBuf1 ;
		uint8_t *	ptrBuf2 ;
		ptrBuf = (uint8_t*) m_ptrEncodeBuf + i * m_nBlockArea ;
		if ( m_nSrcWidth < m_nBlockSize )
		{
			for ( y = 0; y < m_nSrcHeight; y ++ )
			{
				ptrBuf1 = ptrBuf + y * m_nBlockSize ;
				int8_t	p = ptrBuf1[m_nSrcWidth - 1] ;
				for ( x = m_nSrcWidth; x < m_nBlockSize; x ++ )
				{
					ptrBuf1[x] = p ;
				}
			}
		}
		ptrBuf1 = ptrBuf + (m_nSrcHeight - 1) * m_nBlockSize ;
		ptrBuf2 = ptrBuf1 + m_nBlockSize ;
		for ( y = m_nSrcHeight; y < m_nBlockSize; y ++ )
		{
			for ( x = 0; x < m_nBlockSize; x ++ )
			{
				ptrBuf1[x] = ptrBuf2[x] ;
			}
			ptrBuf1 += m_nBlockSize ;
			ptrBuf2 += m_nBlockSize ;
		}
	}
}

// 4:4:4 スケーリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::BlockScaling444( void )
{
	size_t	nBlockSize = m_nBlockSize ;
	size_t	nHalfSize = m_nBlockSize / 2 ;
	size_t	nHalfArea = m_nBlockArea / 2 ;
	size_t	i, j, k, p = 0 ;
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		for ( k = 0; k < m_nChannelCount; k ++ )
		{
			for ( i = 0; i < 2; i ++ )
			{
				for ( j = 0; j < 2; j ++ )
				{
					eslMoveMemory
						( m_ptrBlocksetBuf[p ++],
							m_ptrBlocksetBuf[k * 9 + i * 3 + j],
							m_nBlockArea * sizeof(float32_t) ) ;
				}
			}
		}
	}
	else
	{
		for ( k = 0; k < m_nChannelCount; k ++ )
		{
			for ( i = 0; i < 2; i ++ )
			{
				for ( j = 0; j < 2; j ++ )
				{
					eslMoveMemory
						( m_ptrBlocksetBuf[p ++],
							m_ptrBlocksetBuf[k * 9 + 4 + i * 3 + j],
							m_nBlockArea * sizeof(float32_t) ) ;
				}
			}
		}
	}
}

// 4:1:1 スケーリング
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::BlockScaling411( void )
{
	//
	// 輝度信号を再配列
	//
	size_t	nBlockSize = m_nBlockSize ;
	size_t	nHalfSize = m_nBlockSize / 2 ;
	size_t	nHalfArea = m_nBlockArea / 2 ;
	size_t	i, j ;
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		for ( i = 0, j = 2; i < 2; i ++, j ++ )
		{
			eslMoveMemory
				( m_ptrBlocksetBuf[j],
					m_ptrBlocksetBuf[3 + i],
					m_nBlockArea * sizeof(float32_t) ) ;
		}
	}
	else
	{
		for ( i = 0; i < 2; i ++ )
		{
			for ( j = 0; j < 2; j ++ )
			{
				eslMoveMemory
					( m_ptrBlocksetBuf[i * 2 + j],
						m_ptrBlocksetBuf[4 + i * 3 + j],
						m_nBlockArea * sizeof(float32_t) ) ;
			}
		}
	}
	if ( m_nChannelCount < 3 )
	{
		return ;
	}
	//
	// 色差信号を 1/2 にスケーリング
	//
	for ( i = 0; i < 2; i ++ )
	{
		float32_t *	pDstBuf[4] ;
		pDstBuf[0] = m_ptrBlocksetBuf[4 + i] ;
		pDstBuf[1] = pDstBuf[0] + nHalfSize ;
		pDstBuf[2] = pDstBuf[0] + nHalfArea ;
		pDstBuf[3] = pDstBuf[1] + nHalfArea ;
		//
		for ( j = 0; j < 4; j ++ )
		{
			float32_t *	ptrSrc =
				m_ptrBlocksetBuf[13 + i * 9 + (j >> 1) * 3 + (j & 0x01)] ;
			float32_t *	ptrDst = pDstBuf[j] ;
			//
			for ( size_t y = 0; y < nBlockSize; y += 2 )
			{
				for ( size_t x = 0; x < nHalfSize; x ++ )
				{
					ptrDst[x] =
						(ptrSrc[0] + ptrSrc[1]
							+ ptrSrc[nBlockSize]
							+ ptrSrc[nBlockSize + 1]) * 0.25f ;
					ptrSrc += 2 ;
				}
				ptrDst += nBlockSize ;
				ptrSrc += nBlockSize ;
			}
		}
	}
	//
	// αチャネルを複製
	//
	if ( m_nChannelCount >= 4 )
	{
		for ( i = 0; i < 2; i ++ )
		{
			for ( j = 0; j < 2; j ++ )
			{
				if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
				{
					eslMoveMemory
						( m_ptrBlocksetBuf[6 + i * 2 + j],
							m_ptrBlocksetBuf[27 + i * 3 + j],
							m_nBlockArea * sizeof(float32_t) ) ;
				}
				else
				{
					eslMoveMemory
						( m_ptrBlocksetBuf[6 + i * 2 + j],
							m_ptrBlocksetBuf[31 + i * 3 + j],
							m_nBlockArea * sizeof(float32_t) ) ;
				}
			}
		}
	}
}

// DCT 変換を施す
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::MatrixDCT8x8( void )
{
	for ( size_t i = 0; i < m_nBlocksetCount; i ++ )
	{
		sclfFastDCT8x8( m_ptrBlocksetBuf[i] ) ;
	}
}

// LOT 変換を施す
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::MatrixLOT8x8( float32_t * ptrVertBufLOT )
{
	//
	// 輝度チャネルを処理
	//
	size_t		i, j, k, l = 0 ;
	float32_t *	ptrHorzBufLOT = m_ptrHorzBufLOT ;
	for ( i = 0; i < 2; i ++ )
	{
		for ( j = 0; j < 2; j ++ )
		{
			sclfFastLOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT + j * m_nBlockArea ) ;
			l ++ ;
		}
		ptrHorzBufLOT += m_nBlockArea ;
	}
	ptrVertBufLOT += m_nBlockArea * 2 ;
	//
	// 色差チャネルを処理
	//
	if ( m_nChannelCount < 3 )
	{
		return ;
	}
	if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
	{
		for ( k = 0; k < 2; k ++ )
		{
			for ( i = 0; i < 2; i ++ )
			{
				for ( j = 0; j < 2; j ++ )
				{
					sclfFastLOT8x8
						( m_ptrBlocksetBuf[l],
							ptrHorzBufLOT, ptrVertBufLOT + j * m_nBlockArea ) ;
					l ++ ;
				}
				ptrHorzBufLOT += m_nBlockArea ;
			}
			ptrVertBufLOT += m_nBlockArea * 2 ;
		}
	}
	else if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
	{
		for ( k = 0; k < 2; k ++ )
		{
			sclfFastLOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT ) ;
			l ++ ;
			ptrHorzBufLOT += m_nBlockArea ;
			ptrVertBufLOT += m_nBlockArea ;
		}
	}
	else
	{
		return ;
	}
	//
	// αチャネルを処理
	//
	if ( m_nChannelCount < 4 )
	{
		return ;
	}
	for ( i = 0; i < 2; i ++ )
	{
		for ( j = 0; j < 2; j ++ )
		{
			sclfFastLOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT + j * m_nBlockArea ) ;
			l ++ ;
		}
		ptrHorzBufLOT += m_nBlockArea ;
	}
	ptrVertBufLOT += m_nBlockArea * 2 ;
}

// 量子化を施す
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoder::ArrangeAndQuantumize( int8_t * ptrCoefficient, uint32_t flagsDecoding )
{
	//
	// ジグザグ走査
	//
	size_t		i, j, k ;
	float32_t *	ptrDst ;
	float32_t *	ptrSrc ;
	uint32_t *	pArrange = m_pArrangeTable[0] ;
	for ( i = 0; i < m_nBlocksetCount; i ++ )
	{
		ptrDst = m_ptrMatrixBuf[i] ;
		ptrSrc = m_ptrBlocksetBuf[i] ;
		for ( j = 0; j < m_nBlockArea; j ++ )
		{
			ptrDst[j] = ptrSrc[pArrange[j]] ;
		}
	}
	//
	// 各ブロックの量子化テーブルの指標を取得
	//
	size_t	iParamIndex[16] ;
	int		nThreshold[16] ;
	size_t	nLPFThreshold[16] ;
	iParamIndex[0] = iParamIndex[1] = iParamIndex[2] = iParamIndex[3] = 0 ;
	if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
	{
		for ( i = 4; i < 12; i ++ )
		{
			iParamIndex[i] = 1 ;
		}
		for ( i = 12; i < m_nBlocksetCount; i ++ )
		{
			iParamIndex[i] = 0 ;
		}
	}
	else
	{
		iParamIndex[4] = iParamIndex[5] = 1 ;
		for ( i = 6; i < m_nBlocksetCount; i ++ )
		{
			iParamIndex[i] = 0 ;
		}
	}
	//
	// ブロックの低周波・高周波成分の比率を調べる
	//
	double	fpTAvgDC[2] = { 0, 0 } ;
	double	fpTAvgAC2[2] = { 0, 0 } ;
	double	fpTAvgLow[2] = { 0, 0 } ;
	double	fpTAvgHigh[2] = { 0, 0 } ;
	int		nBlockCount[2] = { 0, 0 } ;
	int		nScalingHint[2] = { 0, 0 } ;
	for ( i = 0; i < m_nBlocksetCount; i ++ )
	{
		nThreshold[i] =
			(iParamIndex[i] == 0) ?
				m_prmCmprOpt.m_nYThreshold
					: m_prmCmprOpt.m_nCThreshold ;
		nLPFThreshold[i] =
			(iParamIndex[i] == 0) ?
				m_prmCmprOpt.m_nYLPFThreshold
					: m_prmCmprOpt.m_nCLPFThreshold ;
		//
		double	fpAvgDC = 0, fpAvgAC2 = 0, fpAvgLow = 0, fpAvgHigh = 0 ;
		ptrSrc = m_ptrMatrixBuf[i] ;
		fpAvgDC = fabs( ptrSrc[0] ) ;
		k = iParamIndex[i] ;
		fpAvgAC2 = (fabs( ptrSrc[1] ) + fabs( ptrSrc[2] )) / 2 ;
		fpTAvgAC2[k] += fpAvgAC2 ;
		for ( j = 1; j < 28; j ++ )
		{
			fpAvgLow += fabs( ptrSrc[j] ) ;
		}
		for ( j = 28; j < 64; j ++ )
		{
			fpAvgHigh += fabs( ptrSrc[j] ) ;
		}
		fpAvgLow /= (28 - 1) ;
		fpAvgHigh /= (64 - 28) ;
		//
		fpTAvgDC[k] += fpAvgDC ;
		fpTAvgLow[k] += fpAvgLow ;
		fpTAvgHigh[k] += fpAvgHigh ;
		nBlockCount[k] ++ ;
		//
		if ( (fpAvgLow >= fpAvgHigh * 4.0)
			&& (fpAvgDC >= fpAvgLow * 64.0) )
		{
			if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
			{
				nThreshold[i] -= (nThreshold[i] > 0) ? 1 : 0 ;
			}
		}
		else if ( fpAvgLow < fpAvgHigh * 2.0 )
		{
			if ( flagsDecoding & flagDifferential )
			{
				if ( fpAvgDC < fpAvgLow * 3.0 )
				{
					nThreshold[i] /= 2 ;
					nLPFThreshold[i] = (nLPFThreshold[i] + 64) / 2 ;
				}
			}
			else if ( (fpAvgDC < fpAvgLow * 8.0)
				|| ((fpAvgDC <= fpAvgAC2 * 8.0)
						&& (fpAvgAC2 < fpAvgLow * 3.0)) )
			{
				nThreshold[i] /= 2 ;
				nLPFThreshold[i] = 64 ;
				nScalingHint[iParamIndex[i]] -- ;
				//
				if ( fpAvgLow < fpAvgHigh * 2.0 )
				{
					nScalingHint[iParamIndex[i]] -- ;
				}
			}
			else if ( (fpAvgDC < fpAvgLow * 16.0)
				|| ((fpAvgDC <= fpAvgAC2 * 16.0)
						&& (fpAvgAC2 < fpAvgLow * 3.0)) )
			{
				nThreshold[i] -= (nThreshold[i] > 0) ;
				nLPFThreshold[i] = (nLPFThreshold[i] + 64) / 2 ;
				//
				if ( fpAvgLow < fpAvgHigh * 2.0 )
				{
					nScalingHint[iParamIndex[i]] -- ;
				}
			}
		}
	}
	//
	// 周波数分布からスケーリング値を調節する
	//
	int			nCoefficient[2] = { 0, 0 } ;
	float32_t	fpCoefficient[2] = { 1.0f, 1.0f } ;
	for ( i = 0; i < 2; i ++ )
	{
		fpTAvgDC[i] /= nBlockCount[i] ;
		fpTAvgAC2[i] /= nBlockCount[i] ;
		fpTAvgLow[i] /= nBlockCount[i] ;
		fpTAvgHigh[i] /= nBlockCount[i] ;
		//
		if ( nScalingHint[i] < 0 )
		{
			nCoefficient[i] = nScalingHint[i] + 1 ;
			if ( nCoefficient[i] < -4 )
			{
				nCoefficient[i] = -4 ;
			}
			fpCoefficient[i] = 1.0f ;
			if ( nCoefficient[i] & 0x01 )
			{
				fpCoefficient[i] = 1.5f ;
			}
			fpCoefficient[i] =
				(float32_t) (1.0 / (fpCoefficient[i]
								* pow( 2.0, (nCoefficient[i] >> 1) ))) ;
		}
		//
		if ( !(flagsDecoding & flagDifferential)
				&& (m_flagsEncode & flagDifferential) )
		{
			// 差分フレームでの独立ブロックの符号化
			nCoefficient[i] = -3 ;
			fpCoefficient[i] = (float32_t) (1.0 / 0.375) ;
		}
	}
	//
	// 量子化係数の正規化：最大値が範囲に収まるように補正
	//
	float32_t	fpMax[2] = { 0.0f, 0.0f } ;
	for ( i = 0; i < m_nBlocksetCount; i ++ )
	{
		ptrSrc = m_ptrMatrixBuf[i] ;
		k = iParamIndex[i] ;
		for ( j = 0; j < m_nBlockArea; j ++ )
		{
			float32_t	y = (float32_t) fabs
				( ptrSrc[j] * m_pQuantumizeScale[k][j] * fpCoefficient[k] ) ;
			if ( y > fpMax[k] )
			{
				fpMax[k] = y ;
			}
		}
	}
	for ( i = 0; i < 2; i ++ )
	{
		if ( fpMax[i] > 128.0 )
		{
			if ( fpMax[i] <= 192.0 )
			{
				nCoefficient[i] ++ ;
			}
			else
			{
				do
				{
					fpMax[i] *= 0.5f ;
					nCoefficient[i] += 2 ;
				}
				while ( fpMax[i] > 128.0 ) ;
				if ( fpMax[i] < 96.0 )
				{
					nCoefficient[i] -- ;
				}
			}
		}
		fpCoefficient[i] = 1.0f ;
		if ( nCoefficient[i] & 0x01 )
		{
			fpCoefficient[i] = 1.5f ;
		}
		fpCoefficient[i] =
			(float32_t) (1.0 / (fpCoefficient[i]
							* pow( 2.0, (nCoefficient[i] >> 1) ))) ;
	}
	//
	// ブロックスケール係数出力
	//
	ptrCoefficient[0] = (int8_t) nCoefficient[0] ;
	ptrCoefficient[1] = (int8_t) nCoefficient[1] ;
	//
	// 量子化実行
	//
	int8_t *	pbytDst = m_ptrEncodeBuf ;
	ptrSrc = m_ptrMatrixBuf[0] ;
	for ( i = 0; i < m_nBlocksetCount; i ++ )
	{
		//
		// 量子化
		//
		size_t	k = iParamIndex[i] ;
		sclfVectorMultiply
			( ptrSrc, m_pQuantumizeScale[k], m_nBlockArea ) ;
		sclfScalarMultiply
			( ptrSrc, fpCoefficient[k], m_nBlockArea ) ;
		for ( j = 0; j < m_nBlockArea; j ++ )
		{
			pbytDst[j] =
				(int8_t) esl_clampi
						( eslRoundR32ToInt( ptrSrc[j] ), -0x80, 0x7F ) ;
		}
		//
		// 閾値処理
		//
		int		nCurThreshold = nThreshold[i] ;
		size_t	nLPF = nLPFThreshold[i] ;
		if ( nLPF >= m_nBlockArea )
		{
			nLPF = m_nBlockArea ;
		}
		if ( nCurThreshold != 0 )
		{
			double	fpThreshold = nCurThreshold * 0.6 ;
			double	fpThresholdStep = nCurThreshold * 0.8 / 64 ;
			if ( flagsDecoding & flagDifferential )
			{
				for ( j = 0; j < 3; j ++ )
				{
					if ( fabs( ptrSrc[j] ) < 0.7 )
					{
						pbytDst[j] = 0 ;
						break ;
					}
				}
				fpThreshold += 0.6 ;
			}
			j = 3 ;
			while ( j < m_nBlockSize )
			{
				if ( fabs( ptrSrc[j] ) < fpThreshold )
				{
					pbytDst[j] = 0 ;
				}
				fpThreshold += fpThresholdStep ;
				j ++ ;
			}
		}
		ESLAssert( m_nBlockSize == 8 ) ;
		for ( j = m_nBlockArea - 1; (int) j >= (int) nLPF; j -- )
		{
			pbytDst[j] = 0 ;
		}
		pbytDst += m_nBlockArea ;
		ptrSrc += m_nBlockArea ;
	}
	//
	// 直流成分差分処理
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
	{
		m_ptrEncodeBuf[m_nBlockArea]   -= m_ptrEncodeBuf[0] ;
		m_ptrEncodeBuf[m_nBlockArea*2] -= m_ptrEncodeBuf[0] ;
		m_ptrEncodeBuf[m_nBlockArea*3] -= m_ptrEncodeBuf[0] ;
		//
		k = m_nBlockArea * 6 ;
		j = 3 ;
		if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
		{
			k = m_nBlockArea * 4 ;
			j = 1 ;
		}
		for ( i = j; i < m_nChannelCount; i ++ )
		{
			m_ptrEncodeBuf[k + m_nBlockArea]   -= m_ptrEncodeBuf[k] ;
			m_ptrEncodeBuf[k + m_nBlockArea*2] -= m_ptrEncodeBuf[k] ;
			m_ptrEncodeBuf[k + m_nBlockArea*3] -= m_ptrEncodeBuf[k] ;
			k += m_nBlockArea * 4 ;
		}
	}
}

// 画像をサンプリングする関数へのポインタを取得する
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoder::PTR_PROCEDURE SGLImageEncoder::GetLSSamplingFunc
	( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecoding )
{
	return	GetLLSamplingFunc( formatImage, nBitsPerPixel, flagsDecoding ) ;
}


