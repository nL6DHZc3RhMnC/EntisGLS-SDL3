
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
// 画像展開オブジェクト
//////////////////////////////////////////////////////////////////////////////

const SGLImageDecoder::PTR_PROCEDURE
	SGLImageDecoder::m_pfnColorOperation[0x10] =
{
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation0101,
	&SGLImageDecoder::ColorOperation0110,
	&SGLImageDecoder::ColorOperation0111,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation1001,
	&SGLImageDecoder::ColorOperation1010,
	&SGLImageDecoder::ColorOperation1011,
	&SGLImageDecoder::ColorOperation0000,
	&SGLImageDecoder::ColorOperation1101,
	&SGLImageDecoder::ColorOperation1110,
	&SGLImageDecoder::ColorOperation1111,
} ;

const SGLImageDecoder::PTR_MOVE_BLOCK
		SGLImageDecoder::m_pfnMoveBlockPFrame[2][4] =
{
	{
		&SGLImageDecoder::SamplingRGBMovePBlock0,
		&SGLImageDecoder::SamplingRGBMovePBlock1,
		&SGLImageDecoder::SamplingRGBMovePBlock2,
		&SGLImageDecoder::SamplingRGBMovePBlock3,
	},
	{
		&SGLImageDecoder::SamplingBGRMovePBlock0,
		&SGLImageDecoder::SamplingBGRMovePBlock1,
		&SGLImageDecoder::SamplingBGRMovePBlock2,
		&SGLImageDecoder::SamplingBGRMovePBlock3,
	},
} ;

const SGLImageDecoder::PTR_MOVE_BLOCK
		SGLImageDecoder::m_pfnMoveBlockBFrame[2][4] =
{
	{
		&SGLImageDecoder::SamplingRGBMoveBBlock0,
		&SGLImageDecoder::SamplingRGBMoveBBlock1,
		&SGLImageDecoder::SamplingRGBMoveBBlock2,
		&SGLImageDecoder::SamplingRGBMoveBBlock3,
	},
	{
		&SGLImageDecoder::SamplingBGRMoveBBlock0,
		&SGLImageDecoder::SamplingBGRMoveBBlock1,
		&SGLImageDecoder::SamplingBGRMoveBBlock2,
		&SGLImageDecoder::SamplingBGRMoveBBlock3,
	},
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLImageDecoder, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoder::SGLImageDecoder( void )
{
	m_ptrDstBlock = NULL ;
	m_ptrOperations = NULL ;
	m_ptrColumnBuf = NULL ;
	m_ptrLineBuf = NULL ;
	m_ptrDecodeBuf = NULL ;
	m_ptrArrangeBuf = NULL ;
	m_pArrangeTable[0] = NULL ;
	m_ptrVertBufLOT = NULL ;
	m_ptrHorzBufLOT = NULL ;
	m_ptrBlocksetBuf[0] = NULL ;
	m_ptrMatrixBuf = NULL ;
	m_ptrIQParamBuf = NULL ;
	m_ptrIQParamTable = NULL ;
	m_ptrBlockLineBuf = NULL ;
	m_ptrSrcImageBuf = NULL ;
	m_ptrYUVImage = NULL ;
	m_ptrRGBImage = NULL ;
	m_ptrMovingVector = NULL ;
	m_ptrMoveVecFlags = NULL ;
	m_ptrMovePrevBlocks = NULL ;
	m_pPrevImageInf = NULL ;
	m_pPrevImageBuf = NULL ;
	m_pNextImageInf = NULL ;
	m_pNextImageBuf = NULL ;
	m_pFilterImageInf = NULL ;
	m_pFilterImageBuf = NULL ;
	m_pHuffmanTree = NULL ;
	m_pProbERISA = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoder::~SGLImageDecoder( void )
{
	SGLImageDecoder::Delete( ) ;
}

// 初期化（パラメータの設定）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageDecoder::Initialize
		( const ERISA::ERI_INFO_HEADER & infhdr )
{
	//
	// 以前のデータを消去
	//
	Delete( ) ;
	//
	// 画像情報ヘッダをコピー
	//
	m_eihInfo = infhdr ;
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLossless )
	{
		//
		// パラメータのチェック
		//
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
		m_nBlockSize = ((size_t) 1 << m_eihInfo.dwBlockingDegree) ;
		m_nBlockArea = ((size_t) 1 << (m_eihInfo.dwBlockingDegree * 2)) ;
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
		m_bufOperations.SetLength( m_nWidthInBlocks * m_nHeightInBlocks ) ;
		m_ptrOperations = m_bufOperations.GetArray() ;
		//
		m_bufColumn.SetLength( m_nBlockSize * m_nChannelCount ) ;
		m_ptrColumnBuf = m_bufColumn.GetArray() ;
		//
		m_bufLine.SetLength
			( m_nChannelCount
					* (m_nWidthInBlocks << m_eihInfo.dwBlockingDegree) ) ;
		m_ptrLineBuf = m_bufLine.GetArray() ;
		//
		m_bufDecode.SetLength( m_nBlockSamples ) ;
		m_ptrDecodeBuf = m_bufDecode.GetArray() ;
		//
		m_bufArrange.SetLength( m_nBlockSamples ) ;
		m_ptrArrangeBuf = m_bufArrange.GetArray() ;
		//
		if ( (m_ptrOperations == NULL) | (m_ptrColumnBuf == NULL)
			| (m_ptrLineBuf == NULL)
			| (m_ptrDecodeBuf == NULL) | (m_ptrArrangeBuf == NULL) )
		{
			return	errFailed ;		// エラー（メモリ不足）
		}
		//
		// バージョンのチェック
		//
		if ( m_eihInfo.dwVersion == eriFileStandardVersinon )
		{
			// 標準フォーマット
			m_flagEnhancedMode = 0 ;
			//
			InitializeArrangeTable( ) ;
		}
		else if ( m_eihInfo.dwVersion == eriFileEnhancedVersinon )
		{
			// 拡張フォーマット
			m_flagEnhancedMode = 2 ;
			//
			InitializeArrangeTable( ) ;
			//
			if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
			{
				m_pHuffmanTree = new ERISA::ERINA_HUFFMAN_TREE ;
			}
			else if ( m_eihInfo.dwArchitecture == erisaNemesisCode )
			{
				m_pProbERISA = new ERISA::ERISA_PROB_MODEL ;
			}
		}
		else
		{
			return	errFailed ;		// エラー（未対応のバージョン）
		}
	}
	else if ( (m_eihInfo.fdwTransformation == eriTransformationLOT)
			|| (m_eihInfo.fdwTransformation == eriTransformationDCT) )
	{
		//
		// 非可逆圧縮
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
		if ( m_eihInfo.dwBlockingDegree != 3 )
		{
			return	errFailed ;		// エラー（未対応の画像フォーマット）
		}
		//
		m_nBlockSize = ((size_t) 1 << m_eihInfo.dwBlockingDegree) ;
		m_nBlockArea = ((size_t) 1 << (m_eihInfo.dwBlockingDegree * 2)) ;
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
		m_bufDecode.SetLength( m_nBlockArea * 16 ) ;
		m_ptrDecodeBuf = m_bufDecode.GetArray() ;
		//
		m_bufVertLOT.SetLength( m_nBlockSamples * 2 * m_nWidthInBlocks ) ;
		m_bufHorzLOT.SetLength( m_nBlockSamples * 2 ) ;
		m_ptrVertBufLOT = m_bufVertLOT.GetArray() ;
		m_ptrHorzBufLOT = m_bufHorzLOT.GetArray() ;
		//
		m_bufBlockset.SetLength( m_nBlockArea * 16 ) ;
		m_ptrBlocksetBuf[0] = m_bufBlockset.GetArray() ;
		//
		m_bufMatrix.SetLength( m_nBlockArea ) ;
		m_ptrMatrixBuf = m_bufMatrix.GetArray() ;
		//
		m_bufIQParam.SetLength( m_nBlockArea * 2 ) ;
		m_bufIQParamTable.SetLength( m_nBlockArea * 2 ) ;
		m_ptrIQParamBuf = m_bufIQParam.GetArray() ;
		m_ptrIQParamTable = m_bufIQParamTable.GetArray() ;
		//
		const size_t	nTotalBlocks = m_nWidthInBlocks * m_nHeightInBlocks ;
		m_bufOperations.SetLength( nTotalBlocks * 2 ) ;
		m_ptrOperations = m_bufOperations.GetArray() ;
		//
		m_bufSrcImage.SetLength
			( m_nWidthInBlocks * m_nBlockArea * m_nBlocksetCount ) ;
		m_ptrSrcImageBuf = m_bufSrcImage.GetArray() ;
		//
		m_bufMovingVector.SetLength( nTotalBlocks * 4 ) ;
		m_ptrMovingVector = m_bufMovingVector.GetArray() ;
		//
		m_bufMoveVecFlags.SetLength( nTotalBlocks ) ;
		m_ptrMoveVecFlags = m_bufMoveVecFlags.GetArray() ;
		//
		m_bufMovePrevBlocks.SetLength( nTotalBlocks ) ;
		m_ptrMovePrevBlocks = m_bufMovePrevBlocks.GetArray() ;
		//
		m_pPrevImageInf = NULL ;
		m_pPrevImageBuf = NULL ;
		//
		for ( size_t i = 1; i < 16; i ++ )
		{
			m_ptrBlocksetBuf[i] = m_ptrBlocksetBuf[0] + (m_nBlockArea * i) ;
		}
		//
		// 中間画像バッファを生成
		//
		m_nYUVPixelBytes = (uint32_t) m_nChannelCount ;
		if ( m_nYUVPixelBytes == 3 )
		{
			m_nYUVPixelBytes = 4 ;
		}
		m_nYUVLineBytes =
			((m_nYUVPixelBytes
				* m_nWidthInBlocks * m_nBlockSize * 2) + 0x0F) & (~0x0F) ;
		size_t	nYUVImageSize = m_nYUVLineBytes * m_nBlockSize * 2 ;
		if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
		{
			nYUVImageSize *= m_nHeightInBlocks ;
		}
		//
		m_bufBlockLine.SetLength( m_nYUVLineBytes * 16 ) ;
		m_ptrBlockLineBuf = m_bufBlockLine.GetArray() ;
		//
		m_bufYUVImage.SetLength( nYUVImageSize * 2 ) ;
		m_ptrYUVImage = m_bufYUVImage.GetArray() ;
		//
		m_ptrRGBImage = (uint8_t*) m_ptrYUVImage + nYUVImageSize ;
		m_nRGBLineBytes = m_nYUVLineBytes ;
		m_nRGBPixelBytes = m_nYUVPixelBytes ;
		//
		if ( (m_ptrDecodeBuf == NULL) | (m_ptrVertBufLOT == NULL)
			| (m_ptrHorzBufLOT == NULL) | (m_ptrBlocksetBuf[0] == NULL)
			| (m_ptrMatrixBuf == NULL) | (m_ptrIQParamBuf == NULL)
			| (m_ptrIQParamTable == NULL) | (m_ptrOperations == NULL)
			| (m_ptrSrcImageBuf == NULL)
			| (m_ptrMovingVector == NULL) | (m_ptrMovePrevBlocks == NULL) )
		{
			return	errFailed ;		// エラー（メモリ不足）
		}
		//
		// サンプリングテーブルの準備
		//
		InitializeZigZagTable( ) ;
		//
		// 圧縮コンテキストの初期化
		//
		m_pHuffmanTree = new ERISA::ERINA_HUFFMAN_TREE ;
		m_pProbERISA = new ERISA::ERISA_PROB_MODEL ;
		//
		// 非同期処理用
		//
		m_aLossyDecInstance.RemoveAll() ;
		m_flagAsyncDecoding = false ;
		//
		if ( (m_nHeightInBlocks >= 3)
			&& (SSystem::GetLogicalProcessorCount() >= 2) )
		{
			/*
			m_flagAsyncDecoding = true ;
			m_bufSrcImageNext.SetLength
				( m_nWidthInBlocks * m_nBlockArea * m_nBlocksetCount ) ;
			m_sigReadySrcNext.Initialize( false ) ;
			m_sigDecodeSrcNext.Initialize( false ) ;
			*/
			size_t	nCount = SSystem::GetLogicalProcessorCount() ;
			if ( nCount > m_nHeightInBlocks )
			{
				nCount = m_nHeightInBlocks ;
			}
			for ( size_t i = 0; i < nCount; i ++ )
			{
				LossyDCTDecodingLine *	plddl = new LossyDCTDecodingLine ;
				m_aLossyDecInstance.Add( plddl ) ;
				//
				plddl->nPosY = 0 ;
				plddl->ptrQParam = NULL ;
				plddl->ptrDstBlock = NULL ;
				plddl->pMovePrevBlock = NULL ;
				//
				if ( i == 0 )
				{
					plddl->ptrSrcData = m_bufSrcImage.GetArray() ;
					plddl->ptrIQParamBuf = m_bufIQParam.GetArray() ;
					plddl->ptrBlocksetBuf[0] = m_bufBlockset.GetArray() ;
					plddl->ptrYUVImage = m_bufYUVImage.GetArray() ;
				}
				else
				{
					plddl->ptrSrcData =
						plddl->m_bufSrcImage.GetArray
							( m_nWidthInBlocks * m_nBlockArea * m_nBlocksetCount ) ;
					plddl->ptrIQParamBuf =
						plddl->m_bufIQParam.GetArray( m_nBlockArea * 2 ) ;
					plddl->ptrBlocksetBuf[0] =
						plddl->m_bufBlockset.GetArray( m_nBlockArea * 16 ) ;
					plddl->ptrYUVImage =
						plddl->m_bufYUVImage.GetArray( nYUVImageSize * 2 ) ;
				}
				for ( size_t j = 1; j < 16; j ++ )
				{
					plddl->ptrBlocksetBuf[j] =
						plddl->ptrBlocksetBuf[0] + (m_nBlockArea * j) ;
				}
				plddl->ptrRGBImage =
						(uint8_t*) plddl->ptrYUVImage + nYUVImageSize ;
			}
		}
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
void SGLImageDecoder::Delete( void )
{
	m_bufOperations.FreeArray() ;
	m_bufColumn.FreeArray() ;
	m_bufLine.FreeArray() ;
	m_bufDecode.FreeArray() ;
	m_bufArrange.FreeArray() ;
	m_bufArrangeTable.FreeArray() ;
	m_bufVertLOT.FreeArray() ;
	m_bufHorzLOT.FreeArray() ;
	m_bufBlockset.FreeArray() ;
	m_bufMatrix.FreeArray() ;
	m_bufIQParam.FreeArray() ;
	m_bufIQParamTable.FreeArray() ;
	m_bufBlockLine.FreeArray() ;
	m_bufSrcImage.FreeArray() ;
	m_bufYUVImage.FreeArray() ;
	m_bufMovingVector.FreeArray() ;
	m_bufMoveVecFlags.FreeArray() ;
	m_bufMovePrevBlocks.FreeArray() ;
	//
	m_ptrOperations = NULL ;
	m_ptrColumnBuf = NULL ;
	m_ptrLineBuf = NULL ;
	m_ptrDecodeBuf = NULL ;
	m_ptrArrangeBuf = NULL ;
	m_pArrangeTable[0] = NULL ;
	m_ptrVertBufLOT = NULL ;
	m_ptrHorzBufLOT = NULL ;
	m_ptrBlocksetBuf[0] = NULL ;
	m_ptrMatrixBuf = NULL ;
	m_ptrIQParamBuf = NULL ;
	m_ptrIQParamTable = NULL ;
	m_ptrBlockLineBuf = NULL ;
	m_ptrSrcImageBuf = NULL ;
	m_ptrYUVImage = NULL ;
	m_ptrRGBImage = NULL ;
	m_ptrMovingVector = NULL ;
	m_ptrMoveVecFlags = NULL ;
	m_ptrMovePrevBlocks = NULL ;
	//
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

// 画像を展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageDecoder::DecodeImage
	( const SakuraGL::SGLImageInfo & infDstImage, uint8_t * pDstBuffer,
		ERISA::SGLDecodeBitStream & bstream, uint32_t flagsDecoding )
{
	//
	// 出力画像情報をコピーする
	//
	SGLImageInfo	imginf = infDstImage ;
	bool	fReverse = ((flagsDecoding & flagTopDown) != 0) ;
	if ( m_eihInfo.nImageHeight < 0 )
	{
		fReverse = ! fReverse ;
	}
	if ( fReverse )
	{
		pDstBuffer += (imginf.height - 1) * imginf.pitchLine ;
		imginf.pitchLine = - imginf.pitchLine ;
	}
	if ( imginf.format & formatImageFlagSideBySide )
	{
		imginf.width *= 2 ;
	}
	if ( m_eihInfo.fdwTransformation == eriTransformationLossless )
	{
		// 可逆画像フォーマット
		return	DecodeLosslessImage( imginf, pDstBuffer, bstream, flagsDecoding ) ;
	}
	else if ( (m_eihInfo.fdwTransformation == eriTransformationDCT)
			|| (m_eihInfo.fdwTransformation == eriTransformationLOT) )
	{
		// 非可逆画像フォーマット
		return	DecodeLossyImage( imginf, pDstBuffer, bstream, flagsDecoding ) ;
	}
	return	errFailed ;			// 未対応のフォーマット
}

// 直前フレームへの参照を設定
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SetRefPreviousFrame
	( SakuraGL::SGLImageInfo * pPrevFrame, uint8_t * pPrevFrameBuf,
		SakuraGL::SGLImageInfo * pNextFrame, uint8_t * pNextFrameBuf )
{
	m_pPrevImageInf = pPrevFrame ;
	m_pPrevImageBuf = pPrevFrameBuf ;
	if ( pPrevFrame != NULL )
	{
		m_nPrevLineBytes = pPrevFrame->pitchLine ;
	}
	m_pNextImageInf = pNextFrame ;
	m_pNextImageBuf = pNextFrameBuf ;
	if ( pNextFrame != NULL )
	{
		m_nNextLineBytes = pNextFrame->pitchLine ;
	}
}

// フィルタ処理画像を受け取るバッファを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SetFilteredImageBuffer
	( SakuraGL::SGLImageInfo * pImageInf, uint8_t * pImageBuf )
{
	m_pFilterImageInf = pImageInf ;
	m_pFilterImageBuf = pImageBuf ;
}

// 展開進行状況通知関数
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	SGLImageDecoder::OnDecodedBlock( const SakuraGL::SGLImageRect & rect )
{
	return	errSuccess ;
}

// 可逆画像展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageDecoder::DecodeLosslessImage
	( const SakuraGL::SGLImageInfo & infDstImage, uint8_t * pDstBuffer,
		ERISA::SGLDecodeBitStream & bstream, uint32_t flagsDecoding )
{
	//
	// 画像データヘッダを取得する
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nERIVersion, fOpTable, fEncodeType, nBitCount ;
	//
	// ERI image data header (4 bytes) ;
	//	0000H : nERIVersion : ERI バージョン
	//		= 1 : ERI 互換フォーマット（第１水準）
	//		= 8 : ERINA フォーマット（第２水準）
	//		= 16 : ERISA フォーマット（第５水準）
	//	0001H : fOpTable : オペレーションテーブルの圧縮
	//		= 0 : 常に０
	//	0002H : fEncodeType : エンコード方式
	//		= 0 : ブロック独立型（推奨）
	//		= 1 : ブロック非独立型
	//	0003H : nBitCount : ビット深度
	//		= 0 : ERI 互換フォーマットでは予約（常に０）
	//		= 8 : ERINA, ERISA では符号のビット深度（常に８）
	//
	// 備考 ;
	// 　ERI ヘッダに続けてオペレーションテーブルが続く。
	// 　オペレーションコードは、ERI では固定長符号（4ビット）、
	// 　ERINA ではハフマン符号、ERISA では算術符号となる。
	// 　オペレーションコードの次に 1 ビットの '0' があり、
	// 　その後に実際の画像の符号が続く。
	// 　ただし、ERISA では常にブロック独立型が指定され、
	// 　オペレーションテーブルは存在しない。
	// 　また、ERISA 符号は先頭に 16 ビットの 0 が
	// 　付加されるので、ERI ヘッダの直後 17 ビットは
	// 　常に 0 である。
	//
	m_pFilterImageInf = NULL ;
	m_pFilterImageBuf = NULL ;
	//
	nERIVersion = bstream.GetNBits( 8 ) ;
	fOpTable = bstream.GetNBits( 8 ) ;
	fEncodeType = bstream.GetNBits( 8 ) ;
	nBitCount = bstream.GetNBits( 8 ) ;
	//
	if ( (fOpTable != 0) || (fEncodeType & 0xFE) )
	{
		return	errFailed ;			// 未対応のフォーマット
	}
	if ( nERIVersion == 1 )
	{
		// ERI 互換フォーマット（第１水準）
		if ( nBitCount != 0 )
		{
			return	errFailed ;		// 未対応のフォーマット
		}
	}
	else if ( nERIVersion == 8 )
	{
		// ERINA フォーマット（第２水準）
		if ( nBitCount != 8 )
		{
			return	errFailed ;		// 未対応のフォーマット
		}
	}
	else if ( nERIVersion == 16 )
	{
		// ERISA フォーマット（第５水準）
		if ( (nBitCount != 8) || (fEncodeType != 0) )
		{
			return	errFailed ;		// 未対応のフォーマット
		}
	}
	else
	{
		return	errFailed ;
	}
	if ( (flagsDecoding & flagDifferential)
			& (m_pPrevImageInf != NULL) & (m_pPrevImageBuf != NULL) )
	{
		SGLImageBuffer	bufDst( infDstImage ) ;
		SGLImageBuffer	bufPrev( *m_pPrevImageInf ) ;
		bufDst.ptrBuffer = pDstBuffer ;
		bufPrev.ptrBuffer = m_pPrevImageBuf ;
		//
		bool	fReverse = ((flagsDecoding & flagTopDown) != 0) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			fReverse = !fReverse ;
		}
		if ( fReverse )
		{
			bufPrev.ptrBuffer += (bufPrev.height - 1) * bufPrev.pitchLine ;
			bufPrev.pitchLine = - bufPrev.pitchLine ;
		}
		if ( bufPrev.format & formatImageFlagSideBySide )
		{
			bufPrev.width *= 2 ;
		}
		sglCopyImageBuffer( bufDst, bufPrev ) ;
		m_pPrevImageInf = NULL ;
		m_pPrevImageBuf = NULL ;
	}
	//
	// 関数を準備する
	//////////////////////////////////////////////////////////////////////////
	size_t	i ;
	//
	// ストア関数取得
	//
	PTR_PROCEDURE	pfnRestoreFunc ;
	m_nDstPixelBytes = infDstImage.pitchPixel ;
	m_nDstLineBytes = infDstImage.pitchLine ;
	pfnRestoreFunc = GetLLRestoreFunc
			( infDstImage.format, infDstImage.depth, flagsDecoding ) ;
	if ( pfnRestoreFunc == NULL )
	{
		return	errFailed ;
	}
	//
	// オペレーションテーブルを取得する ;
	//	ERI 互換フォーマット :
	//		4 ビットでカラーオペレーション番号を指定する
	//		上位 4 ビットに 1100 を付加すると拡張フォーマットと同じ意味になる
	//	ERI 拡張フォーマット :
	//		DFC(2):ARC(2):COPN(4) のビットフォーマットで、
	//		DFC には差分コード、ARC にはアレンジコード、
	//		COPN にはカラーオペレーション番号を指定する。
	//
	SGLAbstractDecodeContext *	pContext = NULL ;
	SGLGammaDecodeContext		ctxGamma( &bstream ) ;
	SGLHuffmanDecodeContext		ctxHuffman( &bstream ) ;
	SGLERISADecodeContext		ctxNemesis( &bstream ) ;
	//
	if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
	{
		ESLAssert( m_pHuffmanTree != NULL ) ;
		m_pHuffmanTree->Initialize( ) ;
	}
	else if ( m_eihInfo.dwArchitecture == erisaNemesisCode )
	{
		ESLAssert( m_pProbERISA != NULL ) ;
		m_pProbERISA->Initialize( ) ;
	}
	uint8_t *	ptrNextOperation = m_ptrOperations ;
	if ( (fEncodeType & 0x01) && (m_nChannelCount >= 3) )
	{
		ESLAssert( m_eihInfo.dwArchitecture != erisaNemesisCode ) ;
		size_t	nAllBlockCount = m_nWidthInBlocks * m_nHeightInBlocks ;
		if ( m_eihInfo.dwArchitecture == eriRunlengthGamma )
		{
			for ( i = 0; i < nAllBlockCount; i ++ )
			{
				m_ptrOperations[i] = (uint8_t) (bstream.GetNBits(4) | 0xC0) ;
			}
		}
		else
		{
			ESLAssert( m_eihInfo.dwArchitecture == eriRunlengthHuffman ) ;
			for ( i = 0; i < nAllBlockCount; i ++ )
			{
				m_ptrOperations[i] =
					(uint8_t) ctxHuffman.GetHuffmanCode( m_pHuffmanTree ) ;
			}
		}
	}
	//
	// コンテキストを初期化する
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;			// 不正なフォーマット
	}
	if ( m_eihInfo.dwArchitecture == eriRunlengthGamma )
	{
		// ランレングス・ガンマ符号 (ERI 互換フォーマット)
		pContext = &ctxGamma ;
		if ( fEncodeType & 0x01 )
		{
			ctxGamma.InitGammaContext( ) ;
		}
	}
	else if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
	{
		// ランレングス・適応型１次ハフマン符号 (ERINA フォーマット)
		pContext = &ctxHuffman ;
		ctxHuffman.PrepareToDecodeERINACode( ) ;
	}
	else
	{
		// ERISA-N 符号 (ERISA フォーマット)
		ESLAssert( m_eihInfo.dwArchitecture == erisaNemesisCode ) ;
		pContext = &ctxNemesis ;
		ctxNemesis.PrepareToDecodeERISACode( ) ;
	}
	//
	// ラインバッファをクリアする
	//
	size_t	nWidthSamples = m_nChannelCount * m_nWidthInBlocks * m_nBlockSize ;
	eslFillMemory( m_ptrLineBuf, 0, nWidthSamples ) ;
	//
	// 各ブロックごとに復号して出力する反復処理
	//////////////////////////////////////////////////////////////////////////
	SGLImageRect	irBlockRect ;
	size_t			nPosX, nPosY ;
	size_t			nAllBlockLines = m_nBlockSize * m_nChannelCount ;
	size_t			nLeftHeight = infDstImage.height ;
	irBlockRect.y = 0 ;
	//
	for ( nPosY = 0; nPosY < m_nHeightInBlocks; nPosY ++ )
	{
		//
		// カラムバッファをクリアする
		//
		size_t	nColumnBufSamples = m_nBlockSize * m_nChannelCount ;
		eslFillMemory( m_ptrColumnBuf, 0, nColumnBufSamples ) ;
		//
		// 行の復号準備
		//
		if ( !(flagsDecoding & flagPreviewDecode) )
		{
			m_ptrDstBlock =
				pDstBuffer + (nPosY * infDstImage.pitchLine * m_nBlockSize) ;
			m_nDstHeight = m_nBlockSize ;
			if ( m_nDstHeight > nLeftHeight )
			{
				m_nDstHeight = nLeftHeight ;
			}
		}
		else
		{
			m_ptrDstBlock = pDstBuffer + (nPosY * infDstImage.pitchLine) ;
			m_nDstHeight = 1 ;
		}
		irBlockRect.h = (int32_t) m_nDstHeight ;
		//
		size_t		nLeftWidth = infDstImage.width ;
		int8_t *	ptrNextLineBuf = m_ptrLineBuf ;
		irBlockRect.x = 0 ;
		//
		for ( nPosX = 0; nPosX < m_nWidthInBlocks; nPosX ++ )
		{
			//
			// １つのブロックを復号して出力
			//////////////////////////////////////////////////////////////////
			//
			// ブロックの情報を正規化
			//
			if ( !(flagsDecoding & flagPreviewDecode) )
			{
				m_nDstWidth = m_nBlockSize ;
				if ( m_nDstWidth > nLeftWidth )
				{
					m_nDstWidth = nLeftWidth ;
				}
			}
			else
			{
				m_nDstWidth = 1 ;
			}
			irBlockRect.w = (int32_t) m_nDstWidth ;
			//
			// オペレーションコードを取得
			//
			uint32_t	nOperationCode ;
			if ( m_nChannelCount >= 3 )
			{
				if ( fEncodeType & 0x01 )
				{
					nOperationCode = *(ptrNextOperation ++) ;
				}
				else if ( m_eihInfo.dwArchitecture == eriRunlengthGamma )
				{
					nOperationCode = bstream.GetNBits(4) | 0xC0 ;
					ctxGamma.InitGammaContext( ) ;
				}
				else if ( m_eihInfo.dwArchitecture == eriRunlengthHuffman )
				{
					nOperationCode =
						ctxHuffman.GetHuffmanCode( m_pHuffmanTree ) ;
				}
				else
				{
					ESLAssert( m_eihInfo.dwArchitecture == erisaNemesisCode ) ;
					nOperationCode =
						ctxNemesis.DecodeERISACode( m_pProbERISA ) ;
				}
			}
			else
			{
				if ( m_eihInfo.fdwFormatType == formatImageGray )
				{
					nOperationCode = 0xC0 ;
				}
				else
				{
					nOperationCode = 0x00 ;
				}
				if ( !(fEncodeType & 0x01)
					& (m_eihInfo.dwArchitecture == eriRunlengthGamma) )
				{
					ctxGamma.InitGammaContext( ) ;
				}
			}
			//
			// 符号を復号する
			//
			if ( pContext->Read
				( m_ptrArrangeBuf, m_nBlockSamples ) < m_nBlockSamples )
			{
				return	errFailed ;			// エラー（デコードに失敗）
			}
			//
			// オペレーションを実行する
			//
			PerformOperation
				( nOperationCode, nAllBlockLines, ptrNextLineBuf ) ;
			ptrNextLineBuf += nColumnBufSamples ;
			//
			// 処理結果を出力バッファにストア
			//
			(this->*pfnRestoreFunc)( ) ;
			//
			// 展開の状況を通知
			//
			SError	err ;
			err = OnDecodedBlock( irBlockRect ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 次のブロックへ移動
			//
			if ( !(flagsDecoding & flagPreviewDecode) )
			{
				m_ptrDstBlock += m_nDstPixelBytes * m_nBlockSize ;
			}
			else
			{
				m_ptrDstBlock += m_nDstPixelBytes ;
			}
			irBlockRect.x += (int32_t) m_nBlockSize ;
			nLeftWidth -= m_nBlockSize ;
		}
		//
		irBlockRect.y += (int32_t) m_nBlockSize ;
		nLeftHeight -= m_nBlockSize ;
	}
	return	errSuccess ;			// 正常終了
}

// アレンジテーブルの初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::InitializeArrangeTable( void )
{
	uint32_t	i, j, k, l, m ;
	//
	// サンプリングテーブル用バッファ確保
	//
	m_bufArrangeTable.SetLength( m_nBlockSamples * 4 ) ;
	uint32_t *	ptrTable = m_bufArrangeTable.GetArray() ;
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
	for ( i = 0; i < m_nBlockArea; i ++ )
	{
		k = i ;
		for ( j = 0; j < m_nChannelCount; j ++ )
		{
			*(ptrNext ++) = k ;
			k += (uint32_t) m_nBlockArea ;
		}
	}
	//
	// 垂直方向インターリーブ
	//
	ptrNext = m_pArrangeTable[3] ;
	for ( i = 0; i < m_nBlockSize; i ++ )
	{
		l = i ;
		for ( j = 0; j < m_nBlockSize; j ++ )
		{
			m = l ;
			l += (uint32_t) m_nBlockSize ;
			for ( k = 0; k < m_nChannelCount; k ++ )
			{
				*(ptrNext ++) = m ;
				m += (uint32_t) m_nBlockArea ;
			}
		}
	}
}

// オペレーション実行
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::PerformOperation
	( uint32_t nOpCode, size_t nAllBlockLines, int8_t * pNextLineBuf )
{
	//
	// 再配列実行
	//
	size_t		i, j, k ;
	uint32_t	nArrangeCode, nColorOperation, nDiffOperation ;
	nColorOperation = nOpCode & 0x0F ;
	nArrangeCode = (nOpCode >> 4) & 0x03 ;
	nDiffOperation = (nOpCode >> 6) & 0x03 ;
	//
	if ( nArrangeCode == 0 )
	{
		eslMoveMemory( m_ptrDecodeBuf, m_ptrArrangeBuf, m_nBlockSamples ) ;
		//
		if ( nOpCode == 0 )
		{
			return ;
		}
	}
	else
	{
		const size_t	nBlockSamples = m_nBlockSamples ;
		int8_t *		ptrDecodeBuf = m_ptrDecodeBuf ;
		int8_t *		ptrArrangeBuf = m_ptrArrangeBuf ;
		uint32_t *		pArrange = m_pArrangeTable[nArrangeCode] ;
		for ( i = 0; i < nBlockSamples; i ++ )
		{
			ptrDecodeBuf[pArrange[i]] = ptrArrangeBuf[i] ;
		}
	}
	//
	// カラーオペレーションを実行
	//
	(this->*m_pfnColorOperation[nColorOperation])( ) ;
	//
	// 差分処理を実行（水平方向）
	//
	const size_t	nBlockSize = m_nBlockSize ;
	int8_t *		ptrNextBuf ;
	int8_t *		ptrNextColBuf ;
	int8_t *		ptrLineBuf ;
	if ( nDiffOperation & 0x01 )
	{
		ptrNextBuf = m_ptrDecodeBuf ;
		ptrNextColBuf = m_ptrColumnBuf ;
		for ( i = 0; i < nAllBlockLines; i ++ )
		{
			int8_t	nLastVal = *ptrNextColBuf ;
			for ( j = 0; j < nBlockSize; j ++ )
			{
				nLastVal += *ptrNextBuf ;
				*(ptrNextBuf ++) = nLastVal ;
			}
			*(ptrNextColBuf ++) = nLastVal ;
		}
	}
	else
	{
		ptrNextBuf = m_ptrDecodeBuf ;
		ptrNextColBuf = m_ptrColumnBuf ;
		for ( i = 0; i < nAllBlockLines; i ++ )
		{
			*(ptrNextColBuf ++) = ptrNextBuf[nBlockSize - 1] ;
			ptrNextBuf += nBlockSize ;
		}
	}
	//
	// 差分処理を実行（垂直方向）
	//
	ptrLineBuf = pNextLineBuf ;
	ptrNextBuf = m_ptrDecodeBuf ;
	for ( k = 0; k < (INT) m_nChannelCount; k ++ )
	{
		int8_t *	ptrLastLine = ptrLineBuf ;
		for ( i = 0; i < nBlockSize; i ++ )
		{
			int8_t *	ptrCurrentLine = ptrNextBuf ;
			for ( j = 0; j < nBlockSize; j ++ )
			{
				*(ptrNextBuf ++) += *(ptrLastLine ++) ;
			}
			ptrLastLine = ptrCurrentLine ;
		}
		for ( j = 0; j < nBlockSize; j ++ )
		{
			*(ptrLineBuf ++) = *(ptrLastLine ++) ;
		}
	}
}


// カラーオペレーション関数群
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::ColorOperation0000( void )
{
}

void SGLImageDecoder::ColorOperation0101( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = *ptrNext ;
		ptrNext[nChSamples] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation0110( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea << 1 ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = *ptrNext ;
		ptrNext[nChSamples] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation0111( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = *ptrNext ;
		ptrNext[nChSamples] += nBase ;
		ptrNext[nChSamples << 1] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1001( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples] ;
		*ptrNext += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1010( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples] ;
		ptrNext[nChSamples << 1] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1011( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples] ;
		*ptrNext += nBase ;
		ptrNext[nChSamples << 1] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1101( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea << 1 ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples] ;
		*ptrNext += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1110( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples << 1] ;
		ptrNext[nChSamples] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

void SGLImageDecoder::ColorOperation1111( void )
{
	int8_t		nBase ;
	int8_t *	ptrNext = m_ptrDecodeBuf ;
	size_t		nChSamples = m_nBlockArea ;
	size_t		nRepCount = m_nBlockArea ;
	//
	do
	{
		nBase = ptrNext[nChSamples << 1] ;
		*ptrNext += nBase ;
		ptrNext[nChSamples] += nBase ;
		ptrNext ++ ;
	}
	while ( -- nRepCount ) ;
}

// グレイ画像／256色画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::RestoreGray8( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstLine[x] = ptrSrcLine[x] ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// RGB 画像（15ビット）の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::RestoreRGB16( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint16_t *	ptrDstNext = (uint16_t*) ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			*(ptrDstNext ++) =
				((uint16_t) *ptrSrcNext & 0x1F) |
				(((uint16_t) ptrSrcNext[nBlockSamples] & 0x1F) << 5) |
				(((uint16_t) ptrSrcNext[nBlockSamples * 2] & 0x1F) << 10) ;
			ptrSrcNext ++ ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// RGB 画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::RestoreRGB24( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = *ptrSrcNext ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = ptrSrcNext[nBlockSamplesX2] ;
			ptrSrcNext ++ ;
			ptrDstNext += nBytesPerPixel ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::RestoreBGR24( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = *ptrSrcNext ;
			ptrSrcNext ++ ;
			ptrDstNext += nBytesPerPixel ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::RestoreRGB32( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = *ptrSrcNext ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[3] = 0xFF ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::RestoreBGR32( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = *ptrSrcNext ;
			ptrDstNext[3] = 0xFF ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// RGBA 画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::RestoreRGBA32( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nBlockSamplesX3 = nBlockSamplesX2 + nBlockSamples ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = *ptrSrcNext ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[3] = ptrSrcNext[nBlockSamplesX3] ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::RestoreBGRA32( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nBlockSamplesX3 = nBlockSamplesX2 + nBlockSamples ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] = ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[1] = ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] = *ptrSrcNext ;
			ptrDstNext[3] = ptrSrcNext[nBlockSamplesX3] ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// RGB 画像の差分出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::LL_RestoreDeltaRGB24( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] += *ptrSrcNext ;
			ptrDstNext[1] += ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] += ptrSrcNext[nBlockSamplesX2] ;
			ptrSrcNext ++ ;
			ptrDstNext += nBytesPerPixel ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::LL_RestoreDeltaBGR24( void )
{
	uint8_t *		ptrDstLine = m_ptrDstBlock ;
	uint8_t *		ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] += ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[1] += ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] += *ptrSrcNext ;
			ptrSrcNext ++ ;
			ptrDstNext += nBytesPerPixel ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// RGBA 画像の差分出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::LL_RestoreDeltaRGBA32( void )
{
	uint8_t *	ptrDstLine = m_ptrDstBlock ;
	uint8_t *	ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nBlockSamplesX3 = nBlockSamplesX2 + nBlockSamples ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] += *ptrSrcNext ;
			ptrDstNext[1] += ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] += ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[3] += ptrSrcNext[nBlockSamplesX3] ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

void SGLImageDecoder::LL_RestoreDeltaBGRA32( void )
{
	uint8_t *	ptrDstLine = m_ptrDstBlock ;
	uint8_t *	ptrSrcLine = (uint8_t*) m_ptrDecodeBuf ;
	const size_t	nBytesPerPixel = m_nDstPixelBytes ;
	const size_t	nBlockSamples = m_nBlockArea ;
	const size_t	nBlockSamplesX2 = nBlockSamples << 1 ;
	const size_t	nBlockSamplesX3 = nBlockSamplesX2 + nBlockSamples ;
	const size_t	nDstHeight = m_nDstHeight ;
	const size_t	nDstWidth = m_nDstWidth ;
	//
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		uint8_t *	ptrDstNext = ptrDstLine ;
		uint8_t *	ptrSrcNext = ptrSrcLine ;
		//
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			ptrDstNext[0] += ptrSrcNext[nBlockSamplesX2] ;
			ptrDstNext[1] += ptrSrcNext[nBlockSamples] ;
			ptrDstNext[2] += *ptrSrcNext ;
			ptrDstNext[3] += ptrSrcNext[nBlockSamplesX3] ;
			ptrSrcNext ++ ;
			ptrDstNext += 4 ;
		}
		ptrSrcLine += m_nBlockSize ;
		ptrDstLine += m_nDstLineBytes ;
	}
}

// 画像出力関数取得
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoder::PTR_PROCEDURE SGLImageDecoder::GetLLRestoreFunc
	( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecode )
{
	switch ( nBitsPerPixel )
	{
	case	32:
		if ( formatImage == formatImageABGR )
		{
			if ( !(flagsDecode & flagDifferential) )
				return	&SGLImageDecoder::RestoreBGRA32 ;
			else
				return	&SGLImageDecoder::LL_RestoreDeltaBGRA32 ;
		}
		else if ( formatImage == formatImageARGB )
		{
			if ( !(flagsDecode & flagDifferential) )
				return	&SGLImageDecoder::RestoreRGBA32 ;
			else
				return	&SGLImageDecoder::LL_RestoreDeltaRGBA32 ;
		}
		else if ( !(flagsDecode & flagDifferential) )
		{
			if ( (formatImage & formatImageTypeMask) == formatImageBGR )
				return	&SGLImageDecoder::RestoreBGR32 ;
			else
				return	&SGLImageDecoder::RestoreRGB32 ;
		}
	case	24:
		if ( formatImage == formatImageBGR )
		{
			if ( !(flagsDecode & flagDifferential) )
				return	&SGLImageDecoder::RestoreBGR24 ;
			else
				return	&SGLImageDecoder::LL_RestoreDeltaBGR24 ;
		}
		else
		{
			if ( !(flagsDecode & flagDifferential) )
				return	&SGLImageDecoder::RestoreRGB24 ;
			else
				return	&SGLImageDecoder::LL_RestoreDeltaRGB24 ;
		}
	case	16:
		return	&SGLImageDecoder::RestoreRGB16 ;
	case	8:
		return	&SGLImageDecoder::RestoreGray8 ;
	}
	return	NULL ;
}

// 非可逆画像展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLImageDecoder::DecodeLossyImage
	( const SakuraGL::SGLImageInfo & infDstImage, uint8_t * pDstBuffer,
		ERISA::SGLDecodeBitStream & bstream, uint32_t flagsDecoding )
{
	//
	// 画像データヘッダを取得する
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nERIVersion, fOpTable, fEncodeType, nBitCount ;
	//
	// ERI image data header (4 bytes) ;
	//	0000H : nERIVersion : ERI バージョン
	//		= 21H : ガンマ符号（未使用）
	//		= 28H : ERINA 符号
	//		= 30H : ERISA 符号（未使用）
	//	0001H : fOpTable : オペレーションテーブル
	//		=  0H : デフォルト
	//		|  1H : 動き補償ベクトル（動画用：P フレーム）
	//		|  2H : 動き補償ベクトル（動画用：B フレーム）
	//		|  4H : 4-2-1 ループフィルタ（画面全体）
	//		|  8H : 4-2-1 ループフィルタ（イントラブロックのみ）
	//	0002H : fEncodeType : エンコード方式
	//		= 0 : デフォルト
	//		= 1 : DCT 変換を使用（差分フレーム用）
	//	0003H : nBitCount : ビット深度
	//		= 0 : ガンマ符号では予約（常に０）
	//		= 8 : ERINA, ERISA では符号のビット深度（常に８）
	//
	// 備考；
	//		ERI ヘッダに続けて次のデータ（テーブル）が続く
	//			・逆量子化係数テーブル
	//			・ブロックスケーリング係数
	//			・動き補償ベクトルテーブル
	//			・画像データ
	//		逆量子化テーブルはハフマン符号で、画像データは
	//		ランレングス付ハフマン符号で、それ以外は
	//		ランレングスガンマ符号で符号化される
	//
	nERIVersion = bstream.GetNBits( 8 ) ;
	fOpTable = bstream.GetNBits( 8 ) ;
	fEncodeType = bstream.GetNBits( 8 ) ;
	nBitCount = bstream.GetNBits( 8 ) ;
	//
	// 関数を準備する
	//////////////////////////////////////////////////////////////////////////
	if ( flagsDecoding & flagQualityDecode )
	{
		flagsDecoding &= ~flagQuickDecode ;
	}
	//
	// LOT 変換・DCT 変換の切り替え
	//
	DWORD	fdwOrgTrans = m_eihInfo.fdwTransformation ;
	DWORD	fdwTransformation = fdwOrgTrans ;
	if ( fEncodeType == 1 )
	{
		fdwTransformation = eriTransformationDCT ;
	}
	CalcImageSizeInBlocks( fdwTransformation ) ;
	//
	// 出力先画像バッファを設定
	//
	m_ptrDstBlock = pDstBuffer ;
	m_nDstPixelBytes = infDstImage.pitchPixel ;
	m_nDstLineBytes = infDstImage.pitchLine ;
	m_nDstWidth = infDstImage.width ;
	m_nDstHeight = infDstImage.height ;
	m_flagsDecode = flagsDecoding ;
	//
	// ストア関数取得
	//
	PTR_RESTORE_FUNC	pfnRestoreFunc ;
	pfnRestoreFunc = GetLSRestoreFunc
		( infDstImage.format, infDstImage.depth, flagsDecoding ) ;
	if ( pfnRestoreFunc == NULL )
	{
		return	errFailed ;
	}
	//
	// コンテキストを初期化する
	//
	SGLAbstractDecodeContext *	pContext = NULL ;
	SGLGammaDecodeContext		ctxGamma( &bstream ) ;
	SGLHuffmanDecodeContext		ctxHuffman( &bstream ) ;
	//
	if ( bstream.GetABit() != 0 )
	{
		return	errFailed ;			// 不正なフォーマット
	}
	if ( nERIVersion == 0x28 )
	{
		ESLAssert( m_eihInfo.dwArchitecture == eriRunlengthGamma ) ;
		ESLAssert( m_pHuffmanTree != NULL ) ;
		m_pHuffmanTree->Initialize( ) ;
		ctxHuffman.PrepareToDecodeERINACode
			( SGLHuffmanDecodeContext::efERINAOrder0 ) ;
		pContext = &ctxHuffman ;
	}
	else
	{
		return	errFailed ;			// 未対応のフォーマット
	}
	//
	// 符号を復号
	//////////////////////////////////////////////////////////////////////////
	//
	// 逆量子化テーブルを取得
	//
	const size_t	nBlockArea = m_nBlockArea ;
	const size_t	nBlockAreaX2 = nBlockArea << 1 ;
	size_t	i ;
	for ( i = 0; i < nBlockAreaX2; i ++ )
	{
		m_ptrIQParamTable[i] =
			(uint8_t) ctxHuffman.GetHuffmanCode( m_pHuffmanTree ) ;
	}
	//
	// ブロックスケーリング係数を取得
	//
	const size_t	nTotalBlocks = m_nWidthInBlocks * m_nHeightInBlocks ;
	const size_t	nTotalSamples =
						nTotalBlocks * m_nBlockArea * m_nBlocksetCount ;
	ctxGamma.InitGammaContext( ) ;
	if ( ctxGamma.DecodeGammaCodeBytes
		( (int8_t*) m_ptrOperations, (nTotalBlocks << 1) )
										< (nTotalBlocks << 1) )
	{
		return	errFailed ;
	}
	//
	// 動き補償ベクトルを取得
	//
	ESLAssert( m_nBlockSize == 8 ) ;
#if	defined(__COTOPHA__)
	constant		nBlockSize = 16 ;
#else
	const size_t	nBlockSize = 16 ;
#endif
	const size_t	nWidthDivBlocks =
						(infDstImage.width + (nBlockSize - 1)) / nBlockSize ;
	const size_t	nHeightDivBlocks =
						(infDstImage.height + (nBlockSize - 1)) / nBlockSize ;
	const size_t	nTotalDivBlocks = nWidthDivBlocks * nHeightDivBlocks ;
	size_t			nPosX, nPosY ;
	//
	if ( fOpTable & 0x01 )
	{
		ctxGamma.InitGammaContext( ) ;
		if ( ctxGamma.DecodeGammaCodeBytes
			( m_ptrMoveVecFlags, nTotalDivBlocks ) < nTotalDivBlocks )
		{
			return	errFailed ;
		}
		ctxGamma.InitGammaContext( ) ;
		if ( ctxGamma.DecodeGammaCodeBytes
			( m_ptrMovingVector, (nTotalDivBlocks << 2) )
										< (nTotalDivBlocks << 2) )
		{
			return	errFailed ;
		}
	}
	else if ( flagsDecoding & flagDifferential )
	{
		eslFillMemory( m_ptrMoveVecFlags, 1, nTotalDivBlocks ) ;
		eslFillMemory( m_ptrMovingVector, 0, (nTotalDivBlocks << 2) ) ;
	}
	if ( flagsDecoding & flagDifferential )
	{
		ESLAssert( m_pPrevImageInf != NULL ) ;
		ESLAssert( m_pPrevImageBuf != NULL ) ;
		if ( m_pPrevImageInf == NULL )
		{
			m_pPrevImageInf = (SakuraGL::SGLImageInfo*) &infDstImage ;
		}
		if ( m_pPrevImageBuf == NULL )
		{
			m_pPrevImageBuf = pDstBuffer ;
		}
		SetupMovingVector( ) ;
	}
	//
	// 画像信号の復号準備（ラインバイト数計算）
	//
	const size_t	nLineBlockSamples =
		m_nWidthInBlocks * m_nBlockArea * m_nBlocksetCount ;
	//
	// 各ブロックごとに復号処理
	//////////////////////////////////////////////////////////////////////////
	SGLImageRect	irBlockRect( 0, 0, (int) m_nDstWidth, 0 ) ;
	int8_t *		ptrSrcData = m_ptrSrcImageBuf ;
	int8_t *		ptrQParam = (int8_t*) m_ptrOperations ;
	//
	// バッファを初期化
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		eslFillMemory
			( m_ptrVertBufLOT, 0,
				m_nBlockSamples * 2 * m_nWidthInBlocks * sizeof(int16_t) ) ;
	}
	//
	// 展開アルゴリズムを選択する
	//
	PTR_BLOCK_MATRIX		pfnBlockMatrix ;
	PTR_BLOCK_SCALING		pfnBlockScaling ;
	PTR_BLOCK_SCALING_LINE	pfnBlockScalingLine ;
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		pfnBlockMatrix = &SGLImageDecoder::MatrixILOT8x8 ;
	}
	else
	{
		pfnBlockMatrix = &SGLImageDecoder::MatrixIDCT8x8 ;
	}
	if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
	{
		if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
		{
			if ( !(flagsDecoding & flagDifferential) )
			{
				pfnBlockScaling =
					&SGLImageDecoder::BlockDCTScaling411_IFrame ;
				pfnBlockScalingLine =
					&SGLImageDecoder::BlockDCTScaling411_IFrame_atLine ;
			}
			else
			{
				pfnBlockScaling =
					&SGLImageDecoder::BlockDCTScaling411_PFrame ;
				pfnBlockScalingLine =
					&SGLImageDecoder::BlockDCTScaling411_PFrame_atLine ;
			}
		}
		else
		{
			pfnBlockScaling = &SGLImageDecoder::BlockLOTScaling411 ;
			pfnBlockScalingLine = NULL ;
		}
	}
	else
	{
		pfnBlockScaling = &SGLImageDecoder::BlockScaling444 ;
		pfnBlockScalingLine = &SGLImageDecoder::BlockScaling444_atLine ;
	}
	//
	m_ptrNextPrevBlocks = m_ptrMovePrevBlocks ;
	//
	// 非同期展開処理
	//
	ASYNC_DECODING_THREAD_PARAM *	padtpParam = NULL ;
	if ( m_flagAsyncDecoding )
	{
		padtpParam = new ASYNC_DECODING_THREAD_PARAM ;
		padtpParam->flagExit = false ;
		padtpParam->countLoop = m_nHeightInBlocks - 1 ;
		padtpParam->pContext = pContext ;
		padtpParam->bytesBuffer = nLineBlockSamples ;
		padtpParam->pSrcBuffer = m_bufSrcImageNext.GetArray() ;
		padtpParam->pReadyNext = &m_sigReadySrcNext ;
		padtpParam->pDecodeNext = &m_sigDecodeSrcNext ;
		//
		if ( pContext->Read
			( padtpParam->pSrcBuffer, nLineBlockSamples ) < nLineBlockSamples )
		{
			delete	padtpParam ;
			return	errFailed ;
		}
		m_sigReadySrcNext.SetSignal() ;
		m_sigDecodeSrcNext.ResetSignal() ;
		//
		SThread::BeginStockThread
			( &SGLImageDecoder::AsyncDecodingThreadProc, padtpParam ) ;
	}
	if ( (m_aLossyDecInstance.GetLength() >= 2)
		&& (m_eihInfo.fdwTransformation == eriTransformationDCT) )
	{
		ESLAssert( pfnBlockScalingLine != NULL) ;
		DecodeLossyLineProc	dllp
			( this, flagsDecoding, pContext,
					pfnBlockScalingLine, pfnRestoreFunc ) ;
		dllp.Start( (void**) m_aLossyDecInstance.GetArray(),
								m_aLossyDecInstance.GetLength() ) ;
	}
	else
	for ( nPosY = 0; nPosY < m_nHeightInBlocks; nPosY ++ )
	{
		//
		// LOT 変換バッファをクリア
		//
		size_t	yInBlocks = 0 ;
		if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
		{
			eslFillMemory
				( m_ptrHorzBufLOT, 0,
					m_nBlockSamples * 2 * sizeof(int16_t) ) ;
			yInBlocks = nPosY ;
		}
		int16_t *	ptrVertBufLOT = m_ptrVertBufLOT ;
		m_ptrNextBlockBuf = m_ptrBlockLineBuf ;
		//
		// 画像信号を復号
		//
		if ( padtpParam != NULL )
		{
			m_sigReadySrcNext.Wait() ;
			eslMoveMemory
				( m_ptrSrcImageBuf,
					m_bufSrcImageNext.GetArray(), nLineBlockSamples ) ;
			m_sigReadySrcNext.ResetSignal() ;
			m_sigDecodeSrcNext.SetSignal() ;
		}
		else
		{
			if ( pContext->Read
				( m_ptrSrcImageBuf, nLineBlockSamples ) < nLineBlockSamples )
			{
				return	errFailed ;
			}
		}
		ptrSrcData = m_ptrSrcImageBuf ;
		//
		for ( nPosX = 0; nPosX < m_nWidthInBlocks; nPosX ++ )
		{
			//
			// 逆量子化
			//
			ArrangeAndIQuantumize( ptrSrcData, ptrQParam ) ;
			ptrSrcData += m_nBlockArea * m_nBlocksetCount ;
			ptrQParam += 2 ;
			//
			// 逆 LOT/DCT 変換
			//
			(this->*pfnBlockMatrix)( ptrVertBufLOT ) ;
			ptrVertBufLOT += m_nBlockArea * 2 * m_nChannelCount ;
			//
			// 画像スケーリング
			//
			(this->*pfnBlockScaling)
				( (unsigned int) nPosX,
					(unsigned int) yInBlocks, flagsDecoding ) ;
		}
		//
		// 逆 YUV 変換・画像出力
		//
		if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
		{
			irBlockRect.y = (int32_t) (nPosY * nBlockSize) ;
			irBlockRect.h = infDstImage.height - irBlockRect.y ;
			if ( irBlockRect.h > nBlockSize )
			{
				irBlockRect.h = nBlockSize ;
			}
			m_nDstHeight = irBlockRect.h ;
			//
			if ( flagsDecoding & flagDifferential )
			{
				MoveImageBlockLineWithVector( ) ;
			}
			//
			ConvertImageYUVtoRGB( 1, flagsDecoding ) ;
			//
			(this->*pfnRestoreFunc)
				( m_ptrDstBlock, m_ptrRGBImage, m_nDstWidth, m_nDstHeight ) ;
			//
			m_ptrDstBlock += m_nDstLineBytes * nBlockSize ;
			//
			// 展開の状況を通知
			//
			SError	err ;
			err = OnDecodedBlock( irBlockRect ) ;
			if ( err )
			{
				if ( padtpParam != NULL )
				{
					padtpParam->flagExit = true ;
					m_sigDecodeSrcNext.SetSignal() ;
				}
				return	err ;
			}
		}
		else if ( nPosY != 0 )
		{
			//
			// 展開の状況を通知
			//
			SError	err ;
			irBlockRect.y = (int32_t) ((nPosY - 1) * nBlockSize) ;
			irBlockRect.h = irBlockRect.y ;
			if ( irBlockRect.h > nBlockSize )
			{
				irBlockRect.h = nBlockSize ;
			}
			err = OnDecodedBlock( irBlockRect ) ;
			if ( err )
			{
				if ( padtpParam != NULL )
				{
					padtpParam->flagExit = true ;
					m_sigDecodeSrcNext.SetSignal() ;
				}
				return	err ;
			}
		}
	}
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		m_nDstHeight = infDstImage.height ;
		//
		if ( flagsDecoding & flagDifferential )
		{
			MoveImageAllBlockWithVector( ) ;
		}
		//
		ConvertImageYUVtoRGB( m_nHeightInBlocks - 1, flagsDecoding ) ;
		//
		(this->*pfnRestoreFunc)
			( m_ptrDstBlock, m_ptrRGBImage, m_nDstWidth, m_nDstHeight ) ;
	}
	//
	// 逆 YUV 変換・画像出力
	//
	if ( !(flagsDecoding & flagNoHalfFilter) )
	{
		if ( (fOpTable & 0x04) | (flagsDecoding & flagUseHalfFilter) )
		{
			// 画面全体へのフィルタ
			if ( (m_pFilterImageInf != NULL) & (m_pFilterImageBuf != NULL) )
			{
				SGLImageBuffer	bufFilter = *m_pFilterImageInf ;
				SGLImageBuffer	bufDstImage = infDstImage ;
				bufFilter.ptrBuffer = m_pFilterImageBuf ;
				bufDstImage.ptrBuffer = pDstBuffer ;
				//
				bool	fReverse = ((flagsDecoding & flagTopDown) != 0) ;
				if ( m_eihInfo.nImageHeight < 0 )
				{
					fReverse = !fReverse ;
				}
				if ( fReverse )
				{
					bufFilter.ptrBuffer +=
						(bufFilter.height - 1) * bufFilter.pitchLine ;
					bufFilter.pitchLine = - bufFilter.pitchLine ;
				}
				eriImageFilterHalf1111( bufFilter, bufDstImage ) ;
				//
				if ( (bufFilter.format & formatImageTypeMask)
					 != (bufDstImage.format & formatImageTypeMask) )
				{
					sglFlipCompositionRGBtoBGR( bufFilter ) ;
				}
			}
		}
		else
		{
			m_pFilterImageInf = NULL ;
			m_pFilterImageBuf = NULL ;
		}
	}
	else
	{
		m_pFilterImageInf = NULL ;
		m_pFilterImageBuf = NULL ;
	}
	//
	m_eihInfo.fdwTransformation = fdwOrgTrans ;
	//
	return	errSuccess ;
}

// 非同期符号デコードスレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::AsyncDecodingThreadProc( void * pInstance )
{
	ASYNC_DECODING_THREAD_PARAM *
			padtp = (ASYNC_DECODING_THREAD_PARAM*) pInstance ;
	bool	flagError = false ;
	while ( !(padtp->flagExit) && (padtp->countLoop != 0) )
	{
		padtp->pDecodeNext->Wait() ;
		padtp->pDecodeNext->ResetSignal() ;
		if ( !flagError
			&& (padtp->pContext->Read
				( padtp->pSrcBuffer,
					padtp->bytesBuffer ) < padtp->bytesBuffer) )
		{
			flagError = true ;
		}
		padtp->pReadyNext->SetSignal() ;
		padtp->countLoop -- ;
	}
	delete	padtp ;
}

// ブロック単位での画面サイズを計算する
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::CalcImageSizeInBlocks( DWORD fdwTransformation )
{
	m_eihInfo.fdwTransformation = fdwTransformation ;
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		m_nWidthInBlocks =
			((m_eihInfo.nImageWidth + (m_nBlockSize * 2 - 1))
									>> (m_eihInfo.dwBlockingDegree + 1)) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			m_nHeightInBlocks = - m_eihInfo.nImageHeight ;
		}
		else
		{
			m_nHeightInBlocks = m_eihInfo.nImageHeight ;
		}
		m_nHeightInBlocks =
			(m_nHeightInBlocks + (m_nBlockSize * 2 - 1))
									>> (m_eihInfo.dwBlockingDegree + 1) ;
		//
		m_nWidthInBlocks += 1  ;
		m_nHeightInBlocks += 1 ;
	}
	else
	{
		m_nWidthInBlocks =
			((m_eihInfo.nImageWidth + (m_nBlockSize * 2 - 1))
									>> (m_eihInfo.dwBlockingDegree + 1)) ;
		if ( m_eihInfo.nImageHeight < 0 )
		{
			m_nHeightInBlocks = - m_eihInfo.nImageHeight ;
		}
		else
		{
			m_nHeightInBlocks = m_eihInfo.nImageHeight ;
		}
		m_nHeightInBlocks =
			(m_nHeightInBlocks + (m_nBlockSize * 2 - 1))
								>> (m_eihInfo.dwBlockingDegree + 1) ;
	}
}

// サンプリングテーブルの初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::InitializeZigZagTable( void )
{
	uint32_t *	ptrArrange ;
	m_bufArrangeTable.SetLength( m_nBlockArea ) ;
	ptrArrange = m_bufArrangeTable.GetArray() ;
	m_pArrangeTable[0] = ptrArrange ;
	//
	unsigned int	i = 0 ;
	signed int		x = 0, y = 0 ;
	const ssize_t	nBlockSize = (ssize_t) m_nBlockSize ;
	const ssize_t	nBlockArea = (ssize_t) m_nBlockArea ;
	for ( ; ; )
	{
		for ( ; ; )
		{
			ptrArrange[i ++] = (uint32_t) (x + y * nBlockSize) ;
			if ( i >= (unsigned int) nBlockArea )
				return ;
			++ x ;
			-- y ;
			if ( x >= nBlockSize )
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
			ptrArrange[i ++] = (uint32_t) (x + y * nBlockSize) ;
			if ( i >= (unsigned int) nBlockArea )
				return ;
			++ y ;
			-- x ;
			if ( y >= nBlockSize )
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
}

// 動きベクトルをセットアップする
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SetupMovingVector( void )
{
	//
	// パラメータ計算
	//
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nDstPixelBytes == 4 ) ;
#if	defined(__COTOPHA__)
	constant		nBlockSize = 16 ;
#else
	const size_t	nBlockSize = 16 ;
#endif
	const size_t	nWidthDivBlocks = 
						(m_nDstWidth + (nBlockSize - 1)) / nBlockSize ;
	const size_t	nHeightDivBlocks =
						(m_nDstHeight + (nBlockSize - 1)) / nBlockSize ;
	const size_t	nTotalDivBlocks = nWidthDivBlocks * nHeightDivBlocks ;
	size_t			nPosX, nPosY ;
	//
	// 垂直方向差分の復元
	//
	int8_t *	ptrLastVector = m_ptrMovingVector ;
	int8_t *	ptrNextVector = ptrLastVector + nWidthDivBlocks * 4 ;
	for ( nPosY = 1; nPosY < nHeightDivBlocks; nPosY ++ )
	{
		for ( nPosX = 0; nPosX < nWidthDivBlocks; nPosX ++ )
		{
			ptrNextVector[0] += ptrLastVector[0] ;
			ptrNextVector[1] += ptrLastVector[1] ;
			ptrNextVector[2] += ptrLastVector[2] ;
			ptrNextVector[3] += ptrLastVector[3] ;
			ptrLastVector += 4 ;
			ptrNextVector += 4 ;
		}
	}
	//
	// 水平方向差分の復元
	//
	ptrNextVector = m_ptrMovingVector ;
	for ( nPosY = 0; nPosY < nHeightDivBlocks; nPosY ++ )
	{
		for ( nPosX = 1; nPosX < nWidthDivBlocks; nPosX ++ )
		{
			ptrNextVector[4] += ptrNextVector[0] ;
			ptrNextVector[5] += ptrNextVector[1] ;
			ptrNextVector[6] += ptrNextVector[2] ;
			ptrNextVector[7] += ptrNextVector[3] ;
			ptrNextVector += 4 ;
		}
		ptrNextVector += 4 ;
	}
	//
	// 参照ブロックへのポインタを算出する
	//
	ESLAssert( m_pPrevImageInf != NULL ) ;
	ESLAssert( m_pPrevImageBuf != NULL ) ;
	MOVE_PREV_BLOCK*	ptrNextBlockAddr = m_ptrMovePrevBlocks ;
	uint8_t *			ptrPrevImage = m_pPrevImageBuf ;
	int8_t *			ptrNextVecFlag = m_ptrMoveVecFlags ;
	size_t				nPrevPixelBytes = m_pPrevImageInf->pitchPixel ;
	ssize_t				nPrevLineBytes = m_pPrevImageInf->pitchLine ;
	uint8_t *			ptrNextImage = NULL ;
	size_t				nNextPixelBytes = 0 ;
	ssize_t				nNextLineBytes = 0 ;
	ESLAssert( nPrevPixelBytes == 4 ) ;
	if ( (m_pNextImageInf != NULL) & (m_pNextImageBuf != NULL) )
	{
		ptrNextImage = m_pNextImageBuf ;
		nNextPixelBytes = m_pNextImageInf->pitchPixel ;
		nNextLineBytes = m_pNextImageInf->pitchLine ;
		ESLAssert( nNextPixelBytes == 4 ) ;
	}
	bool	fReverse = ((m_flagsDecode & flagTopDown) != 0) ;
	if ( m_eihInfo.nImageHeight < 0 )
	{
		fReverse = !fReverse ;
	}
	if ( fReverse )
	{
		ptrPrevImage += (m_pPrevImageInf->height - 1) * nPrevLineBytes ;
		nPrevLineBytes = - nPrevLineBytes ;
		//
		if ( m_pNextImageInf != NULL )
		{
			ptrNextImage += (m_pNextImageInf->height - 1) * nNextLineBytes ;
			nNextLineBytes = - nNextLineBytes ;
		}
	}
	m_nPrevLineBytes = (int32_t) nPrevLineBytes ;
	m_iPrevFormat = 0 ;
	m_nNextLineBytes = (int32_t) nNextLineBytes ;
	m_iNextFormat = 0 ;
	ptrNextVector = m_ptrMovingVector ;
	//
	if ( (m_pPrevImageInf->format & formatImageTypeMask) == formatImageBGR )
	{
		m_iPrevFormat = 1 ;
	}
	if ( (m_pNextImageInf != NULL)
		&& ((m_pNextImageInf->format & formatImageTypeMask) == formatImageBGR) )
	{
		m_iNextFormat = 1 ;
	}
	//
	for ( nPosY = 0; nPosY < nHeightDivBlocks; nPosY ++ )
	{
		for ( nPosX = 0; nPosX < nWidthDivBlocks; nPosX ++ )
		{
			SGLPoint	ptRefPos ;
			int			nPrevType = *(ptrNextVecFlag ++) ;
			ptrNextBlockAddr->pPrevFrame = NULL ;
			ptrNextBlockAddr->pNextFrame = NULL ;
			//
			if ( (nPrevType == 1) || (nPrevType == 2) )
			{
				//
				// 直前フレーム参照
				//
				ptRefPos.x = (int32_t) (nPosX * nBlockSize
									+ ((int) ptrNextVector[0] >> 1)) ;
				ptRefPos.y = (int32_t) (nPosY * nBlockSize
									+ ((int) ptrNextVector[1] >> 1)) ;
				if ( ((ptRefPos.x | ptRefPos.y) < 0)
					|| (ptRefPos.y + nBlockSize > m_nDstHeight)
					|| (ptRefPos.x + nBlockSize > m_nDstWidth) )
				{
					ptRefPos.x = (int32_t) (nPosX * nBlockSize) ;
					ptRefPos.y = (int32_t) (nPosY * nBlockSize) ;
				}
				#if	defined(__DEBUG__)
				SGLSize	sizeBlock ;
				sizeBlock.w = (int32_t) (m_nDstWidth - nPosX * nBlockSize) ;
				sizeBlock.h = (int32_t) (m_nDstHeight - nPosY * nBlockSize) ;
				if ( sizeBlock.w > nBlockSize )
				{
					sizeBlock.w = nBlockSize ;
				}
				if ( sizeBlock.h > nBlockSize )
				{
					sizeBlock.h = nBlockSize ;
				}
				ESLAssert( ptRefPos.x >= 0 ) ;
				ESLAssert( ptRefPos.y >= 0 ) ;
				ESLAssert( ptRefPos.x + sizeBlock.w
						+ (ptrNextVector[0] & 0x01) <= (int) m_nDstWidth ) ;
				ESLAssert( ptRefPos.y + sizeBlock.h
						+ (ptrNextVector[1] & 0x01) <= (int) m_nDstHeight ) ;
				#endif
				ptrNextBlockAddr->pPrevFrame =
					ptrPrevImage + ptRefPos.x * nPrevPixelBytes
									+ ptRefPos.y * nPrevLineBytes ;
			}
			if ( (ptrNextImage != NULL)
				&& ((nPrevType == -1) || (nPrevType == 2)) )
			{
				//
				// 直後フレーム参照
				//
				ptRefPos.x = (int32_t) (nPosX * nBlockSize
									+ ((int) ptrNextVector[2] >> 1)) ;
				ptRefPos.y = (int32_t) (nPosY * nBlockSize
									+ ((int) ptrNextVector[3] >> 1)) ;
				if ( ((ptRefPos.x | ptRefPos.y) < 0)
					|| (ptRefPos.y + nBlockSize > m_nDstHeight)
					|| (ptRefPos.x + nBlockSize > m_nDstWidth) )
				{
					ptRefPos.x = (int32_t) (nPosX * nBlockSize) ;
					ptRefPos.y = (int32_t) (nPosY * nBlockSize) ;
				}
				#if	defined(__DEBUG__)
				SGLSize	sizeBlock ;
				sizeBlock.w = (int32_t) (m_nDstWidth - nPosX * nBlockSize) ;
				sizeBlock.h = (int32_t) (m_nDstHeight - nPosY * nBlockSize) ;
				if ( sizeBlock.w > nBlockSize )
					sizeBlock.w = nBlockSize ;
				if ( sizeBlock.h > nBlockSize )
					sizeBlock.h = nBlockSize ;
				ESLAssert( ptRefPos.x >= 0 ) ;
				ESLAssert( ptRefPos.y >= 0 ) ;
				ESLAssert( ptRefPos.x + sizeBlock.w
						+ (ptrNextVector[2] & 0x01) <= (int) m_nDstWidth ) ;
				ESLAssert( ptRefPos.y + sizeBlock.h
						+ (ptrNextVector[3] & 0x01) <= (int) m_nDstHeight ) ;
				#endif
				ptrNextBlockAddr->pNextFrame =
					ptrNextImage + ptRefPos.x * nNextPixelBytes
									+ ptRefPos.y * nNextLineBytes ;
			}
			ptrNextBlockAddr->flagPrevHalf =
				(ptrNextVector[0] & 0x01) | ((ptrNextVector[1] & 0x01) << 1) ;
			ptrNextBlockAddr->flagNextHalf =
				(ptrNextVector[2] & 0x01) | ((ptrNextVector[3] & 0x01) << 1) ;
			ptrNextVector += 4 ;
			ptrNextBlockAddr ++ ;
		}
	}
}

// 逆量子化
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::ArrangeAndIQuantumize
	( const int8_t * ptrSrcData, const int8_t * ptrCoefficient ) const
{
	ArrangeAndIQuantumize_atLine
		( &m_ptrBlocksetBuf[0], m_ptrIQParamBuf, ptrSrcData, ptrCoefficient ) ;
}

void SGLImageDecoder::ArrangeAndIQuantumize_atLine
	( int16_t *const* ppBlocksetBufs, int16_t * ptrIQParamBuf,
		const int8_t * ptrSrcData, const int8_t * ptrCoefficient ) const
{
	//
	// 逆量子化係数を算出
	//
	ESLAssert( m_nBlockSize == 8 ) ;
	const size_t	nChannelCount = m_nChannelCount ;
	const size_t	nBlockArea = m_nBlockArea ;
	const size_t	nBlockAreaX2 = nBlockArea << 1 ;
	const size_t	nBlockAreaX3 = nBlockAreaX2 + nBlockArea ;
	const size_t	nBlocksetCount = m_nBlocksetCount ;
	size_t		i, j, k ;
	int16_t *	pIQParam[16] ;				// fixed
	uint32_t	nIQScale[16] ;				// right shifter for pIQParam
	int16_t *	pNextQParam = ptrIQParamBuf ;
	uint8_t *	pNextQParamTable = m_ptrIQParamTable ;
	for ( i = 0; i < 2; i ++ )
	{
		int		nCoefficient = ptrCoefficient[i] ;
		int		nOddScale = nCoefficient & 0x01 ;
		nCoefficient >>= 1 ;
		if ( nCoefficient >= 0 )
		{
			nIQScale[i] = nOddScale ;
		}
		else
		{
			nIQScale[i] = - nCoefficient + nOddScale ;
			nCoefficient = 0 ;
		}
		//
		pIQParam[i] = pNextQParam ;
		if ( nOddScale != 0 )
		{
			for ( j = 0; j < nBlockArea; j ++ )
			{
				int16_t	nQParam = *(pNextQParamTable ++) + 1 ;
				*(pNextQParam ++) =
					(nQParam + nQParam + nQParam) << nCoefficient ;
			}
		}
		else
		{
			for ( j = 0; j < nBlockArea; j ++ )
			{
				*(pNextQParam ++) =
					(*(pNextQParamTable ++) + 1) << nCoefficient ;
			}
		}
	}
	//
	// インターリーブ
	//
	ESLAssert( nBlockArea == 64 ) ;
	int16_t		bufSrc[64*16] ;
	int16_t *	ptrNextDst = &bufSrc[0] ;
	for ( i = 0; i < nBlocksetCount; i ++ )
	{
		const int8_t *	ptrNextSrc = ptrSrcData + i ;
		for ( j = 0; j < nBlockArea; j ++ )
		{
			*(ptrNextDst ++) = *ptrNextSrc ;
			ptrNextSrc += nBlocksetCount ;
		}
	}
	//
	// 直流成分差分処理
	//
	if ( m_eihInfo.fdwTransformation == eriTransformationDCT )
	{
		bufSrc[nBlockArea]   = (int8_t) (bufSrc[nBlockArea] + bufSrc[0]) ;
		bufSrc[nBlockAreaX2] = (int8_t) (bufSrc[nBlockAreaX2] + bufSrc[0]) ;
		bufSrc[nBlockAreaX3] = (int8_t) (bufSrc[nBlockAreaX3] + bufSrc[0]) ;
		//
		k = nBlockAreaX3 << 1 ;
		j = 3 ;
		if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
		{
			k = nBlockArea << 2 ;
			j = 1 ;
		}
		for ( i = j; i < nChannelCount; i ++ )
		{
			bufSrc[k + nBlockArea]   =
					(int8_t) (bufSrc[k + nBlockArea] + bufSrc[k]) ;
			bufSrc[k + nBlockAreaX2] =
					(int8_t) (bufSrc[k + nBlockAreaX2] + bufSrc[k]) ;
			bufSrc[k + nBlockAreaX3] =
					(int8_t) (bufSrc[k + nBlockAreaX3] + bufSrc[k]) ;
			k += nBlockArea << 2 ;
		}
	}
	//
	// 逆量子化＆ジグザグ走査
	//
	pIQParam[4] = pIQParam[1] ;
	nIQScale[4] = nIQScale[1] ;
	pIQParam[1] = pIQParam[2] = pIQParam[3] = pIQParam[0] ;
	nIQScale[1] = nIQScale[2] = nIQScale[3] = nIQScale[0] ;
	if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV444 )
	{
		for ( i = 5; i < 12; i ++ )
		{
			pIQParam[i] = pIQParam[4] ;
			nIQScale[i] = nIQScale[4] ;
		}
		for ( i = 12; i < nBlocksetCount; i ++ )
		{
			pIQParam[i] = pIQParam[0] ;
			nIQScale[i] = nIQScale[0] ;
		}
	}
	else
	{
		pIQParam[5] = pIQParam[4] ;
		nIQScale[5] = nIQScale[4] ;
		for ( i = 6; i < nBlocksetCount; i ++ )
		{
			pIQParam[i] = pIQParam[0] ;
			nIQScale[i] = nIQScale[0] ;
		}
	}
	int16_t *	ptrNextSrc = &bufSrc[0] ;
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ESLAssert( SSystem::g_cpuFamily == SSystem::cpuFamily_ARM ) ;
	ESLAssert( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_ARMv7 ) ;
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		for ( i = 0; i < nBlocksetCount; i ++ )
		{
			ERISA_sclwShuffleVectorMul8x8_ARM_NEON
				( ppBlocksetBufs[i], m_pArrangeTable[0],
							ptrNextSrc, pIQParam[i], -nIQScale[i] ) ;
			ptrNextSrc += nBlockArea ;
		}
	}
	else
#elif	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		for ( i = 0; i < nBlocksetCount; i ++ )
		{
			int16_t *	ptrDst = ppBlocksetBufs[i] ;
			uint32_t	nCoefficient = nIQScale[i] ;
			uint32_t *	pArrange = m_pArrangeTable[0] ;
			//
			pNextQParam = pIQParam[i] ;
			//
			__asm
			{
				mov		ecx, nBlockArea
				mov		esi, ptrNextSrc
				mov		edx, pNextQParam
				movd	xmm7, nCoefficient
				shr		ecx, 4
			LoopBegin1:
					movdqu	xmm0, [esi]
					movdqu	xmm1, [edx]
					pmullw	xmm0, xmm1
						movdqu	xmm2, [esi + 16]
						movdqu	xmm3, [edx + 16]
						pmullw	xmm2, xmm3
					psraw	xmm0, xmm7
						psraw	xmm2, xmm7
					movdqu	[esi], xmm0
						movdqu	[esi + 16], xmm2
					add		esi, 32
					add		edx, 32
					dec		ecx
				jnz		LoopBegin1
				;
				mov		ecx, nBlockArea
				mov		edi, ptrDst
				mov		esi, ptrNextSrc
				mov		ebx, pArrange
			LoopBegin2:
					mov		eax, DWORD PTR [ebx]
					mov		dx, WORD PTR [esi]
					add		ebx, 4
					add		esi, 2
					mov		WORD PTR [edi + eax * 2], dx
					dec		ecx
				jnz		LoopBegin2
			}
			ptrNextSrc += nBlockArea ;
		}
	}
	else
#endif
	for ( i = 0; i < nBlocksetCount; i ++ )
	{
		int16_t *	ptrDst = ppBlocksetBufs[i] ;
		uint32_t	nCoefficient = nIQScale[i] ;
		uint32_t *	pArrange = m_pArrangeTable[0] ;
		//
		pNextQParam = pIQParam[i] ;
		//
		for ( j = 0; j < nBlockArea; j ++ )
		{
			ptrDst[*(pArrange ++)] =
				(*(ptrNextSrc ++) * *(pNextQParam ++)) >> nCoefficient ;
		}
	}
}

// 逆 DCT 変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::MatrixIDCT8x8( int16_t * ptrVertBufLOT ) const
{
	MatrixIDCT8x8_atLine( &m_ptrBlocksetBuf[0] ) ;
}

void SGLImageDecoder::MatrixIDCT8x8_atLine( int16_t *const* ppBlocksetBufs ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ESLAssert( SSystem::g_cpuFamily == SSystem::cpuFamily_ARM ) ;
	ESLAssert( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_ARMv7 ) ;
	if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
	{
		for ( size_t i = 0; i < m_nBlocksetCount; i ++ )
		{
			ERISA_sclwFastIDCT8x8_ARM_NEON
				( ppBlocksetBufs[i], &ERISA::sclw_Param_IDCT8x8[0][0] ) ;
		}
	}
	else
	{
		for ( size_t i = 0; i < m_nBlocksetCount; i ++ )
		{
			ERISA_sclwFastIDCT8x8_ARMv7A
				( ppBlocksetBufs[i], &ERISA::sclw_Param_IDCT8x8[0][0] ) ;
		}
	}
#else
	for ( size_t i = 0; i < m_nBlocksetCount; i ++ )
	{
		sclwFastIDCT8x8( ppBlocksetBufs[i] ) ;
	}
#endif
}

// 逆 LOT 変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::MatrixILOT8x8( int16_t * ptrVertBufLOT ) const
{
	//
	// 輝度チャネルを処理
	//
	ESLAssert( m_nBlockSize == 8 ) ;
	const size_t	nBlockArea = 64 ;
	size_t			i, j, k, l = 0 ;
	int16_t *		ptrHorzBufLOT = m_ptrHorzBufLOT ;
	for ( i = 0; i < 2; i ++ )
	{
		for ( j = 0; j < 2; j ++ )
		{
			sclwFastILOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT + j * nBlockArea ) ;
			l ++ ;
		}
		ptrHorzBufLOT += nBlockArea ;
	}
	ptrVertBufLOT += nBlockArea * 2 ;
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
					sclwFastILOT8x8
						( m_ptrBlocksetBuf[l],
							ptrHorzBufLOT, ptrVertBufLOT + j * nBlockArea ) ;
					l ++ ;
				}
				ptrHorzBufLOT += nBlockArea ;
			}
			ptrVertBufLOT += nBlockArea * 2 ;
		}
	}
	else if ( m_eihInfo.dwSamplingFlag == eriSamplingYUV411 )
	{
		for ( k = 0; k < 2; k ++ )
		{
			sclwFastILOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT ) ;
			l ++ ;
			ptrHorzBufLOT += nBlockArea ;
			ptrVertBufLOT += nBlockArea ;
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
			sclwFastILOT8x8
				( m_ptrBlocksetBuf[l],
					ptrHorzBufLOT, ptrVertBufLOT + j * nBlockArea ) ;
			l ++ ;
		}
		ptrHorzBufLOT += nBlockArea ;
	}
	ptrVertBufLOT += nBlockArea * 2 ;
}

// 4:4:4 スケーリング (汎用)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::BlockScaling444
	( unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	BlockScaling444_atLine
		( m_ptrYUVImage, &m_ptrBlocksetBuf[0], x, y, flagsDecode ) ;
}

void SGLImageDecoder::BlockScaling444_atLine
	( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
		unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	int	nBlockOffset = 0 ;
	if ( m_eihInfo.fdwTransformation == eriTransformationLOT )
	{
		nBlockOffset = 1 ;
	}
	for ( size_t i = 0; i < 2; i ++ )
	{
		int	yPos = (int) (y * 2 + i) - nBlockOffset ;
		if ( yPos < 0 )
		{
			continue ;
		}
		for ( size_t j = 0; j < 2; j ++ )
		{
			int	xPos = (int) (x * 2 + j) - nBlockOffset ;
			if ( xPos < 0 )
			{
				continue ;
			}
			//
			// 輝度チャネルを出力
			//
			size_t	k = i * 2 + j ;
			if ( flagsDecode & flagDifferential )
			{
				StoreYUVImageChannelSByte_atLine
					( ptrYUVImage, xPos, yPos, 0, ppBlocksetBufs[k] ) ;
			}
			else
			{
				StoreYUVImageChannelByte_atLine
					( ptrYUVImage, xPos, yPos, 0, ppBlocksetBufs[k] ) ;
			}
			if ( m_nChannelCount < 3 )
			{
				continue ;
			}
			//
			// 色差チャネルを出力
			//
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, xPos, yPos, 1, ppBlocksetBufs[k + 4] ) ;
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, xPos, yPos, 2, ppBlocksetBufs[k + 8] ) ;
			//
			if ( m_nChannelCount < 4 )
			{
				continue ;
			}
			//
			// αチャネルを出力
			//
			if ( flagsDecode & flagDifferential )
			{
				StoreYUVImageChannelSByte_atLine
					( ptrYUVImage, xPos, yPos, 3, ppBlocksetBufs[k + 12] ) ;
			}
			else
			{
				StoreYUVImageChannelByte_atLine
					( ptrYUVImage, xPos, yPos, 3, ppBlocksetBufs[k + 12] ) ;
			}
		}
	}
}

// 4:1:1 スケーリング (DCT 独立フレーム)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::BlockDCTScaling411_IFrame
	( unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	BlockDCTScaling411_IFrame_atLine
		( m_ptrYUVImage, &m_ptrBlocksetBuf[0], x, y, flagsDecode ) ;
}

void SGLImageDecoder::BlockDCTScaling411_IFrame_atLine
	( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
		unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	x <<= 1 ;
	y <<= 1 ;
	StoreYUVImageChannelByte_atLine
		( ptrYUVImage, x,     y,     0, ppBlocksetBufs[0] ) ;
	StoreYUVImageChannelByte_atLine
		( ptrYUVImage, x + 1, y,     0, ppBlocksetBufs[1] ) ;
	StoreYUVImageChannelByte_atLine
		( ptrYUVImage, x,     y + 1, 0, ppBlocksetBufs[2] ) ;
	StoreYUVImageChannelByte_atLine
		( ptrYUVImage, x + 1, y + 1, 0, ppBlocksetBufs[3] ) ;
	//
	if ( m_nChannelCount >= 3 )
	{
		StoreYUVImageChannelX2_atLine
			( ptrYUVImage, x, y, 1, ppBlocksetBufs[4] ) ;
		StoreYUVImageChannelX2_atLine
			( ptrYUVImage, x, y, 2, ppBlocksetBufs[5] ) ;
		//
		if ( m_nChannelCount >= 4 )
		{
			StoreYUVImageChannelByte_atLine
				( ptrYUVImage, x,     y,     3, ppBlocksetBufs[6] ) ;
			StoreYUVImageChannelByte_atLine
				( ptrYUVImage, x + 1, y,     3, ppBlocksetBufs[7] ) ;
			StoreYUVImageChannelByte_atLine
				( ptrYUVImage, x,     y + 1, 3, ppBlocksetBufs[8] ) ;
			StoreYUVImageChannelByte_atLine
				( ptrYUVImage, x + 1, y + 1, 3, ppBlocksetBufs[9] ) ;
		}
	}
}

// 4:1:1 スケーリング (DCT 差分フレーム)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::BlockDCTScaling411_PFrame
	( unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	BlockDCTScaling411_PFrame_atLine
		( m_ptrYUVImage, &m_ptrBlocksetBuf[0], x, y, flagsDecode ) ;
}

void SGLImageDecoder::BlockDCTScaling411_PFrame_atLine
	( int8_t * ptrYUVImage, int16_t *const* ppBlocksetBufs,
		unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	x <<= 1 ;
	y <<= 1 ;
	StoreYUVImageChannelSByte_atLine
		( ptrYUVImage, x,     y,     0, ppBlocksetBufs[0] ) ;
	StoreYUVImageChannelSByte_atLine
		( ptrYUVImage, x + 1, y,     0, ppBlocksetBufs[1] ) ;
	StoreYUVImageChannelSByte_atLine
		( ptrYUVImage, x,     y + 1, 0, ppBlocksetBufs[2] ) ;
	StoreYUVImageChannelSByte_atLine
		( ptrYUVImage, x + 1, y + 1, 0, ppBlocksetBufs[3] ) ;
	//
	if ( m_nChannelCount >= 3 )
	{
		StoreYUVImageChannelX2_atLine
			( ptrYUVImage, x, y, 1, ppBlocksetBufs[4] ) ;
		StoreYUVImageChannelX2_atLine
			( ptrYUVImage, x, y, 2, ppBlocksetBufs[5] ) ;
		//
		if ( m_nChannelCount >= 4 )
		{
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, x,     y,     3, ppBlocksetBufs[6] ) ;
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, x + 1, y,     3, ppBlocksetBufs[7] ) ;
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, x,     y + 1, 3, ppBlocksetBufs[8] ) ;
			StoreYUVImageChannelSByte_atLine
				( ptrYUVImage, x + 1, y + 1, 3, ppBlocksetBufs[9] ) ;
		}
	}
}

// 4:1:1 スケーリング (LOT 汎用)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::BlockLOTScaling411
	( unsigned int x, unsigned int y, uint32_t flagsDecode ) const
{
	int	nBlockOffset = 1 ;
	for ( size_t i = 0; i < 2; i ++ )
	{
		int	yPos = (int) (y * 2 + i) - nBlockOffset * 2 ;
		if ( yPos < 0 )
		{
			continue ;
		}
		for ( size_t j = 0; j < 2; j ++ )
		{
			int	xPos = (int) (x * 2 + j) - nBlockOffset * 2 ;
			if ( xPos < 0 )
			{
				continue ;
			}
			//
			// 輝度チャネルを出力
			//
			size_t	k = i * 2 + j ;
			if ( flagsDecode & flagDifferential )
			{
				StoreYUVImageChannelSByte
					( xPos, yPos, 0, m_ptrBlocksetBuf[k] ) ;
			}
			else
			{
				StoreYUVImageChannelByte
					( xPos, yPos, 0, m_ptrBlocksetBuf[k] ) ;
			}
			if ( m_nChannelCount < 4 )
			{
				continue ;
			}
			//
			// αチャネルを出力
			//
			if ( flagsDecode & flagDifferential )
			{
				StoreYUVImageChannelSByte
					( xPos, yPos, 3, m_ptrBlocksetBuf[k + 6] ) ;
			}
			else
			{
				StoreYUVImageChannelSByte
					( xPos, yPos, 3, m_ptrBlocksetBuf[k + 6] ) ;
			}
		}
	}
	//
	// 色差チャネルを出力
	//
	if ( m_nChannelCount < 3 )
	{
		return ;
	}
	y -= nBlockOffset ;
	x -= nBlockOffset ;
	if ( ((int) y < 0) | ((int) x < 0) )
	{
		return ;
	}
	StoreYUVImageChannelX2( x, y, 1, m_ptrBlocksetBuf[4] ) ;
	StoreYUVImageChannelX2( x, y, 2, m_ptrBlocksetBuf[5] ) ;
}

// 中間画像バッファに 1 チャネル書き出す
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::StoreYUVImageChannelByte
	( size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
	StoreYUVImageChannelByte_atLine
		( m_ptrYUVImage, xBlock, yBlock, iChannel, pwSrcChannel ) ;
}

void SGLImageDecoder::StoreYUVImageChannelByte_atLine
	( int8_t * ptrYUVImage,
		size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
	ESLAssert( m_nBlockSize == 8 ) ;
#if	defined(__COTOPHA__)
	constant	nBlockSize = 8 ;
	asm
	{
		//
		// 入力信号を符号無し8ビットに変換
		//
		REG LOAD	pwSrcChannel
		REG ALLOC	mm(10) : int64
		//
		load.64		mm(0), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(1), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.uswb	mm(0), mm(4)
		pcvt.uswb	mm(1), mm(5)
		//
		load.64		mm(2), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(3), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.uswb	mm(2), mm(4)
		pcvt.uswb	mm(3), mm(5)
		//
		load.64		mm(4), [pwSrcChannel]
		load.64		mm(6), [pwSrcChannel + 0x08]
		load.64		mm(5), [pwSrcChannel + 0x10]
		load.64		mm(7), [pwSrcChannel + 0x18]
		pcvt.uswb	mm(4), mm(6)
		pcvt.uswb	mm(5), mm(7)
		//
		load.64		mm(6), [pwSrcChannel]
		load.64		mm(8), [pwSrcChannel + 0x08]
		load.64		mm(7), [pwSrcChannel + 0x10]
		load.64		mm(9), [pwSrcChannel + 0x18]
		pcvt.uswb	mm(6), mm(8)
		pcvt.uswb	mm(7), mm(9)
		//
		REG FREE	pwSrcChannel
		//
		// ptrDstYUV = 
		//		ptrYUVImage
		//			+ (yBlock * nBlockSize * m_nYUVLineBytes)
		//			+ (xBlock * nBlockSize * nPixelBytes)
		//			+ iChannel * 8 ;
		REG LOAD	xBlock
		REG LOAD	yBlock
		REG LOAD	iChannel
		REG ALLOC	ptrDstYUV : int8 *
		REG ALLOC	nYUVLineBytes : int64
		//
		move		acc, [tp].m_nYUVPixelBytes
		move		nYUVLineBytes, [tp].m_nYUVLineBytes
		imul.dq		acc, xBlock
		imul.dq		yBlock, nYUVLineBytes
		move		ptrDstYUV, ptrYUVImage
		add			acc, yBlock
		add			acc, iChannel
		sll			acc, 3
		add			ptrDstYUV, acc
		//
		// 信号を出力
		//
		store.64	[ptrDstYUV], mm(0)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(1)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(2)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(3)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(4)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(5)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(6)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(7)
	}
#else
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		const size_t	nPixelBytes = m_nYUVPixelBytes ;
		const ssize_t	nYUVLineBytes = m_nYUVLineBytes ;
		const size_t	nBlockSize = 8 ;
		int8_t *		ptrDstYUV =
							ptrYUVImage
								+ (yBlock * nBlockSize * nYUVLineBytes)
								+ (xBlock * nBlockSize * nPixelBytes)
								+ iChannel * 8 ;
		__asm
		{
			mov			eax, pwSrcChannel
			mov			edx, ptrDstYUV
			mov			ecx, nYUVLineBytes
			movdqu		xmm0, [eax]
			movdqu		xmm1, [eax + 10H]
			lea			esi, [ecx + ecx*2]
			movdqu		xmm2, [eax + 20H]
			movdqu		xmm3, [eax + 30H]
			packuswb	xmm0, xmm1
			packuswb	xmm2, xmm3
			movq		MMWORD PTR [edx], xmm0
			movq		MMWORD PTR [edx + ecx*2], xmm2
			psrldq		xmm0, 8
			psrldq		xmm2, 8
			movq		MMWORD PTR [edx + ecx], xmm0
			movq		MMWORD PTR [edx + esi], xmm2
			lea			edx, [edx + ecx*4]
			;
			movdqu		xmm0, [eax + 40H]
			movdqu		xmm1, [eax + 50H]
			movdqu		xmm2, [eax + 60H]
			movdqu		xmm3, [eax + 70H]
			packuswb	xmm0, xmm1
			packuswb	xmm2, xmm3
			movq		MMWORD PTR [edx], xmm0
			movq		MMWORD PTR [edx + ecx*2], xmm2
			psrldq		xmm0, 8
			psrldq		xmm2, 8
			movq		MMWORD PTR [edx + ecx], xmm0
			movq		MMWORD PTR [edx + esi], xmm2
		}
	}
	else
	{
	#endif
		const ssize_t	nYUVLineBytes = m_nYUVLineBytes ;
		const size_t	nPixelBytes = m_nYUVPixelBytes ;
		const size_t	nBlockSize = 8 ;
		int8_t *		ptrDstYUV =
							ptrYUVImage
								+ (yBlock * nBlockSize * nYUVLineBytes)
								+ (xBlock * nBlockSize * nPixelBytes)
								+ iChannel * 8 ;
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A
			( (uint8_t*) ptrDstYUV, nYUVLineBytes, pwSrcChannel ) ;
	#else
		uint8_t	bufDecode[64] ;
		ESLAssert( m_nBlockArea <= 64 ) ;
		sclwConvertArraySWordToByte
			( &bufDecode[0], pwSrcChannel, m_nBlockArea ) ;
		//
		const uint8_t *	ptrSrcYUV = &bufDecode[0] ;
		//
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[0] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[1] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[2] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[3] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[4] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[5] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[6] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[7] ;
	#endif
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	}
	#endif
#endif
}

void SGLImageDecoder::StoreYUVImageChannelSByte
	( size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
	StoreYUVImageChannelSByte_atLine
		( m_ptrYUVImage, xBlock, yBlock, iChannel, pwSrcChannel ) ;
}

void SGLImageDecoder::StoreYUVImageChannelSByte_atLine
	( int8_t * ptrYUVImage,
		size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
	ESLAssert( m_nBlockSize == 8 ) ;
#if	defined(__COTOPHA__)
	constant	nBlockSize = 8 ;
	asm
	{
		//
		// 入力信号を符号あり8ビットに変換
		//
		REG LOAD	pwSrcChannel
		REG ALLOC	mm(10) : int64
		//
		load.64		mm(0), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(1), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.swb	mm(0), mm(4)
		pcvt.swb	mm(1), mm(5)
		//
		load.64		mm(2), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(3), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.swb	mm(2), mm(4)
		pcvt.swb	mm(3), mm(5)
		//
		load.64		mm(4), [pwSrcChannel]
		load.64		mm(6), [pwSrcChannel + 0x08]
		load.64		mm(5), [pwSrcChannel + 0x10]
		load.64		mm(7), [pwSrcChannel + 0x18]
		pcvt.swb	mm(4), mm(6)
		pcvt.swb	mm(5), mm(7)
		//
		load.64		mm(6), [pwSrcChannel]
		load.64		mm(8), [pwSrcChannel + 0x08]
		load.64		mm(7), [pwSrcChannel + 0x10]
		load.64		mm(9), [pwSrcChannel + 0x18]
		pcvt.swb	mm(6), mm(8)
		pcvt.swb	mm(7), mm(9)
		//
		REG FREE	pwSrcChannel
		//
		// ptrDstYUV = 
		//		ptrYUVImage
		//			+ (yBlock * nBlockSize * m_nYUVLineBytes)
		//			+ (xBlock * nBlockSize * nPixelBytes)
		//			+ iChannel * 8 ;
		REG LOAD	xBlock
		REG LOAD	yBlock
		REG LOAD	iChannel
		REG ALLOC	ptrDstYUV : int8 *
		REG ALLOC	nYUVLineBytes : int64
		//
		move		acc, [tp].m_nYUVPixelBytes
		move		nYUVLineBytes, [tp].m_nYUVLineBytes
		imul.dq		acc, xBlock
		imul.dq		yBlock, nYUVLineBytes
		move		ptrDstYUV, ptrYUVImage
		add			acc, yBlock
		add			acc, iChannel
		sll			acc, 3
		add			ptrDstYUV, acc
		//
		REG FREE	xBlock
		REG FREE	yBlock
		REG FREE	iChannel
		//
		// 信号を出力
		//
		store.64	[ptrDstYUV], mm(0)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(1)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(2)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(3)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(4)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(5)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(6)
		add			ptrDstYUV, nYUVLineBytes
		store.64	[ptrDstYUV], mm(7)
	}
#else
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		const size_t	nPixelBytes = m_nYUVPixelBytes ;
		const ssize_t	nYUVLineBytes = m_nYUVLineBytes ;
		const size_t	nBlockSize = 8 ;
		int8_t *		ptrDstYUV =
							ptrYUVImage
								+ (yBlock * nBlockSize * nYUVLineBytes)
								+ (xBlock * nBlockSize * nPixelBytes)
								+ iChannel * 8 ;
		__asm
		{
			mov			eax, pwSrcChannel
			mov			edx, ptrDstYUV
			mov			ecx, nYUVLineBytes
			movdqu		xmm0, [eax]
			movdqu		xmm1, [eax + 10H]
			lea			esi, [ecx + ecx*2]
			movdqu		xmm2, [eax + 20H]
			movdqu		xmm3, [eax + 30H]
			packsswb	xmm0, xmm1
			packsswb	xmm2, xmm3
			movq		MMWORD PTR [edx], xmm0
			movq		MMWORD PTR [edx + ecx*2], xmm2
			psrldq		xmm0, 8
			psrldq		xmm2, 8
			movq		MMWORD PTR [edx + ecx], xmm0
			movq		MMWORD PTR [edx + esi], xmm2
			lea			edx, [edx + ecx*4]
			;
			movdqu		xmm0, [eax + 40H]
			movdqu		xmm1, [eax + 50H]
			movdqu		xmm2, [eax + 60H]
			movdqu		xmm3, [eax + 70H]
			packsswb	xmm0, xmm1
			packsswb	xmm2, xmm3
			movq		MMWORD PTR [edx], xmm0
			movq		MMWORD PTR [edx + ecx*2], xmm2
			psrldq		xmm0, 8
			psrldq		xmm2, 8
			movq		MMWORD PTR [edx + ecx], xmm0
			movq		MMWORD PTR [edx + esi], xmm2
		}
	}
	else
	{
	#endif
		const size_t	nPixelBytes = m_nYUVPixelBytes ;
		const ssize_t	nYUVLineBytes = m_nYUVLineBytes ;
		const size_t	nBlockSize = 8 ;
		int8_t *		ptrDstYUV =
							ptrYUVImage
								+ (yBlock * nBlockSize * nYUVLineBytes)
								+ (xBlock * nBlockSize * nPixelBytes)
								+ iChannel * 8 ;
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A
			( ptrDstYUV, nYUVLineBytes, pwSrcChannel ) ;
	#else
		int8_t	bufDecode[64] ;
		ESLAssert( m_nBlockArea <= 64 ) ;
		sclwConvertArraySWordToSByte
			( &bufDecode[0], pwSrcChannel, m_nBlockArea ) ;
		//
		const int8_t *	ptrSrcYUV = &bufDecode[0] ;
		//
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[0] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[1] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[2] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[3] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[4] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[5] ;
		ptrDstYUV += nYUVLineBytes << 1 ;
		*((int64_t*)ptrDstYUV) = ((const int64_t*)ptrSrcYUV)[6] ;
		*((int64_t*)(ptrDstYUV + nYUVLineBytes))
								= ((const int64_t*)ptrSrcYUV)[7] ;
	#endif
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	}
	#endif
#endif
}

// 中間画像バッファに 1 チャネル書き出す（スケーリング）
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::StoreYUVImageChannelX2
	( size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
	StoreYUVImageChannelX2_atLine
		( m_ptrYUVImage, xBlock, yBlock, iChannel, pwSrcChannel ) ;
}

void SGLImageDecoder::StoreYUVImageChannelX2_atLine
	( int8_t * ptrYUVImage,
		size_t xBlock, size_t yBlock,
		size_t iChannel, const int16_t * pwSrcChannel ) const
{
#if	defined(__COTOPHA__)
	constant	nBlockSize = 8 ;
	asm
	{
		//
		// 入力信号を符号あり8ビットに変換
		//
		REG LOAD	pwSrcChannel
		REG ALLOC	mm(10) : int64
		//
		load.64		mm(0), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(1), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.swb	mm(0), mm(4)
		pcvt.swb	mm(1), mm(5)
		//
		load.64		mm(2), [pwSrcChannel]
		load.64		mm(4), [pwSrcChannel + 0x08]
		load.64		mm(3), [pwSrcChannel + 0x10]
		load.64		mm(5), [pwSrcChannel + 0x18]
		pcvt.swb	mm(2), mm(4)
		pcvt.swb	mm(3), mm(5)
		//
		load.64		mm(4), [pwSrcChannel]
		load.64		mm(6), [pwSrcChannel + 0x08]
		load.64		mm(5), [pwSrcChannel + 0x10]
		load.64		mm(7), [pwSrcChannel + 0x18]
		pcvt.swb	mm(4), mm(6)
		pcvt.swb	mm(5), mm(7)
		//
		load.64		mm(6), [pwSrcChannel]
		load.64		mm(8), [pwSrcChannel + 0x08]
		load.64		mm(7), [pwSrcChannel + 0x10]
		load.64		mm(9), [pwSrcChannel + 0x18]
		pcvt.swb	mm(6), mm(8)
		pcvt.swb	mm(7), mm(9)
		//
		REG FREE	pwSrcChannel
		//
		// ptrDstYUV = 
		//		ptrYUVImage
		//			+ (yBlock * nBlockSize * m_nYUVLineBytes)
		//			+ (xBlock * nBlockSize * nPixelBytes)
		//			+ iChannel * 8 ;
		REG LOAD	xBlock
		REG LOAD	yBlock
		REG LOAD	iChannel
		REG ALLOC	prDstYUV : int8 *
		REG ALLOC	nYUVLineBytes : int64
		REG ALLOC	nYUVPixelBytesX8 : int64
		//
		move		acc, [tp].m_nYUVPixelBytes
		move		nYUVLineBytes, [tp].m_nYUVLineBytes
		sll			nYUVPixelBytesX8, acc, 3
		imul.dq		acc, xBlock
		imul.dq		yBlock, nYUVLineBytes
		move		prDstYUV, ptrYUVImage
		add			acc, yBlock
		add			acc, iChannel
		sll			acc, 3
		add			acc, prDstYUV
		//
		REG FREE	xBlock
		REG FREE	yBlock
		REG FREE	iChannel
		REG FREE	prDstYUV
		//
		// 信号を出力
		//
		ASSUME		acc : int64 *
		srl			r2, mm(0), 32
		punpack.lbw	mm(0), mm(0)
		punpack.lbw	r2, r2
		store.64	[acc], mm(0)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(0)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(1), 32
		punpack.lbw	mm(1), mm(1)
		punpack.lbw	r2, r2
		store.64	[acc], mm(1)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(1)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(2), 32
		punpack.lbw	mm(2), mm(2)
		punpack.lbw	r2, r2
		store.64	[acc], mm(2)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(2)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(3), 32
		punpack.lbw	mm(3), mm(3)
		punpack.lbw	r2, r2
		store.64	[acc], mm(3)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(3)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(4), 32
		punpack.lbw	mm(4), mm(4)
		punpack.lbw	r2, r2
		store.64	[acc], mm(4)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(4)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(5), 32
		punpack.lbw	mm(5), mm(5)
		punpack.lbw	r2, r2
		store.64	[acc], mm(5)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(5)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(6), 32
		punpack.lbw	mm(6), mm(6)
		punpack.lbw	r2, r2
		store.64	[acc], mm(6)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(6)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		//
		srl			r2, mm(7), 32
		punpack.lbw	mm(7), mm(7)
		punpack.lbw	r2, r2
		store.64	[acc], mm(7)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
		store.64	[acc], mm(7)
		store.64	[acc + nYUVPixelBytesX8], r2
		add			acc, nYUVLineBytes
	}
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	const size_t	nPixelBytes = m_nYUVPixelBytes ;
	const size_t	nPixelBytesX8 = nPixelBytes << 3 ;
	const ssize_t	nLineBytes = m_nYUVLineBytes ;
	const size_t	nBlockSize = 8 ;
	int8_t *		ptrDstYUV =
						ptrYUVImage
							+ (yBlock * nBlockSize * nLineBytes)
							+ (xBlock * nBlockSize * nPixelBytes)
							+ iChannel * 8 ;

	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A
			( ptrDstYUV, nPixelBytesX8, nLineBytes, pwSrcChannel ) ;
	#else
		#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
		if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
		{
			__asm
			{
				mov		ecx, nBlockSize
				mov		esi, pwSrcChannel
				mov		edi, ptrDstYUV
				mov		edx, nPixelBytesX8
				mov		eax, nLineBytes
				add		edx, edi
				lea		ebx, [eax + eax * 2]
				shr		ecx, 1
			LoopBegin:
					movdqu		xmm0, [esi]
						movdqu		xmm1, [esi + 16]
					add			esi, 32
					packsswb	xmm0, xmm0
						packsswb	xmm1, xmm1
					punpcklbw	xmm0, xmm0
						punpcklbw	xmm1, xmm1
					movq		QWORD PTR [edi], xmm0
					movq		QWORD PTR [edi + eax], xmm0
					movq		QWORD PTR [edi + eax * 2], xmm1
					movq		QWORD PTR [edi + ebx], xmm1
					psrldq		xmm0, 8
					psrldq		xmm1, 8
					movq		QWORD PTR [edx], xmm0
					movq		QWORD PTR [edx + eax], xmm0
					movq		QWORD PTR [edx + eax * 2], xmm1
					movq		QWORD PTR [edx + ebx], xmm1
					lea			edi, [edi + eax * 4]
					lea			edx, [edx + eax * 4]
					dec			ecx
				jnz		LoopBegin
			}
		}
		else
		#endif
		{
			int8_t	bufTemp[64] ;
			ESLAssert( m_nBlockArea == 64 ) ;
			sclwConvertArraySWordToSByte
				( &bufTemp[0], pwSrcChannel, m_nBlockArea ) ;
			//
			const uint8_t *	ptrSrcYUV = (const uint8_t*) &bufTemp[0] ;
			//
			for ( size_t y = 0; y < nBlockSize; y ++ )
			{
				uint32_t	d0 = ptrSrcYUV[0]
								| ((uint32_t) ptrSrcYUV[1] << 16) ;
				uint32_t	d1 = ptrSrcYUV[2]
								| ((uint32_t) ptrSrcYUV[3] << 16) ;
				uint32_t	d2 = ptrSrcYUV[4]
								| ((uint32_t) ptrSrcYUV[5] << 16) ;
				uint32_t	d3 = ptrSrcYUV[6]
								| ((uint32_t) ptrSrcYUV[7] << 16) ;
				ptrSrcYUV += 8 ;
				d0 |= (d0 << 8) ;
				d1 |= (d1 << 8) ;
				d2 |= (d2 << 8) ;
				d3 |= (d3 << 8) ;
				*((uint32_t*)ptrDstYUV) = d0 ;
				*((uint32_t*)(ptrDstYUV + 4)) = d1 ;
				*((uint32_t*)(ptrDstYUV + nPixelBytesX8)) = d2 ;
				*((uint32_t*)(ptrDstYUV + nPixelBytesX8 + 4)) = d3 ;
				ptrDstYUV += nLineBytes ;
				*((uint32_t*)ptrDstYUV) = d0 ;
				*((uint32_t*)(ptrDstYUV + 4)) = d1 ;
				*((uint32_t*)(ptrDstYUV + nPixelBytesX8)) = d2 ;
				*((uint32_t*)(ptrDstYUV + nPixelBytesX8 + 4)) = d3 ;
				ptrDstYUV += nLineBytes ;
			}
		}
	#endif
#endif
}

// 中間バッファを YUV から RGB 形式へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::ConvertImageYUVtoRGB
		( size_t heightInBlockset, uint32_t flagsDecode ) const
{
	ConvertImageYUVtoRGB_atLine
		( m_ptrRGBImage, m_ptrYUVImage, heightInBlockset, flagsDecode ) ;
}

void SGLImageDecoder::ConvertImageYUVtoRGB_atLine
	( uint8_t * ptrRGBLine, int8_t * ptrYUVLine,
		size_t heightInBlockset, uint32_t flagsDecode ) const
{
	if ( m_nChannelCount < 3 )
	{
		return ;
	}
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	widthInPacked = m_nWidthInBlocks * 2 ;
	const size_t	pitchPixel = m_nRGBPixelBytes ;
	const size_t	pitchPacked = pitchPixel * m_nBlockSize ;
	const size_t	nHeight = heightInBlockset * 16 ;
	const ssize_t	pitchLine = m_nYUVLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	const bool		flagSwitchSSE2 =
		(pitchPixel == 4)
			&& ((SSystem::g_cpuFeatures
					& SSystem::cpuX86_Feature_SSE2) != 0) ;
	#endif
	if ( flagsDecode & flagDifferential )
	{
		for ( size_t yPos = 0; yPos < nHeight; yPos ++ )
		{
			int8_t *	ptrNextYUV = ptrYUVLine ;
			uint8_t *	ptrNextRGB = ptrRGBLine ;
			#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
			if ( flagSwitchSSE2 )
			for ( size_t xPacked = 0; xPacked < widthInPacked; xPacked ++ )
			{
				__asm
				{
					mov		eax, ptrNextYUV
					mov		edx, ptrNextRGB
					movq	xmm4, MMWORD PTR [eax]
					pxor	xmm0, xmm0
					movq	xmm5, MMWORD PTR [eax + 8]
					pxor	xmm1, xmm1
					movq	xmm6, MMWORD PTR [eax + 16]
					pxor	xmm2, xmm2
					movq	xmm7, MMWORD PTR [eax + 24]
					pxor	xmm3, xmm3
					punpcklbw	xmm0, xmm4
					punpcklbw	xmm1, xmm5
					punpcklbw	xmm2, xmm6
					punpcklbw	xmm3, xmm7
					psraw	xmm0, 7			; xmm0 := y
					psraw	xmm1, 7			; xmm1 := u3
					psraw	xmm2, 7			; xmm2 := v3
					psraw	xmm3, 7			; xmm3 := a
					;
					movdqa	xmm4, xmm1
					movdqa	xmm5, xmm1
					movdqa	xmm6, xmm2
					psllw	xmm4, 3			; xmm4 := u7 = (u3 << 3) - u3 ;
					psllw	xmm5, 1
					psllw	xmm6, 1
					psubw	xmm4, xmm1
					paddw	xmm1, xmm5
					paddw	xmm2, xmm6
					;
					psraw	xmm4, 2			; xmm4 = y + (u7 >> 2)
					paddw	xmm1, xmm2		; xmm5 = y - ((u3 + v3 + v3) >> 3)
					paddw	xmm4, xmm0
					paddw	xmm1, xmm2
					movdqa	xmm5, xmm0
					psraw	xmm1, 3
					psraw	xmm2, 1			; xmm2 = y + (v3 >> 1)
					psubw	xmm5, xmm1
					paddw	xmm2, xmm0
					;
					movdqa		xmm0, xmm4
					punpcklwd	xmm4, xmm5	; xmm4 = g3:b3:g2:b2:g1:b1:g0:b0
					punpckhwd	xmm0, xmm5	; xmm0 = g7:b7:g6:b6:g5:b5:g4:b4
					movdqa		xmm6, xmm2
					punpcklwd	xmm2, xmm3	; xmm2 = a3:r3:a2:r2:a1:r1:a0:r0
					punpckhwd	xmm6, xmm3	; xmm6 = a7:r7:a6:r6:a5:r5:a4:r4
					;
					movdqa		xmm5, xmm4
					punpckldq	xmm4, xmm2	; xmm4 = a1:r1:g1:b1:a0:r0:g0:b0
					punpckhdq	xmm5, xmm2	; xmm5 = a3:r3:g3:b3:a2:r2:g2:b2
					movq		xmm2, MMWORD PTR [edx]
					movq		xmm1, MMWORD PTR [edx + 8]
					pxor		xmm3, xmm3
					punpcklbw	xmm2, xmm3
					punpcklbw	xmm1, xmm3
					paddw		xmm4, xmm2
					paddw		xmm5, xmm1
					packuswb	xmm4, xmm5
					movdqu		[edx], xmm4
					;
					movdqa		xmm5, xmm0
					punpckldq	xmm0, xmm6	; xmm0 = a5:r5:g5:b5:a4:r4:g4:b4
					punpckhdq	xmm5, xmm6	; xmm5 = a7:r7:g7:b7:a6:r6:g6:b6
					movq		xmm2, MMWORD PTR [edx + 16]
					movq		xmm1, MMWORD PTR [edx + 24]
					pxor		xmm3, xmm3
					punpcklbw	xmm2, xmm3
					punpcklbw	xmm1, xmm3
					paddw		xmm0, xmm2
					paddw		xmm5, xmm1
					packuswb	xmm0, xmm5
					movdqu		[edx + 16], xmm0
				}
				ptrNextRGB += pitchPixel * 8 ;
				ptrNextYUV += pitchPacked ;
			}
			else
			#endif
			#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
			if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
			{
				ERISA_sclbAddYUVtoRGB8x8_ARM_NEON
					( ptrRGBLine, ptrYUVLine, widthInPacked ) ;
			}
			else
			{
				ERISA_sclbAddYUVtoRGB8x8_ARMv7A
					( ptrRGBLine, ptrYUVLine, widthInPacked ) ;
			}
			#else
			for ( size_t xPacked = 0; xPacked < widthInPacked; xPacked ++ )
			{
				for ( size_t i = 0; i < 8; i ++ )
				{
					int16_t	y = ptrNextYUV[i] << 1 ;
					int16_t	u3 = ptrNextYUV[i + 0x08] << 1 ;
					int16_t	v3 = ptrNextYUV[i + 0x10] << 1 ;
					int16_t	u7 = (u3 << 3) - u3 ;
					u3 = (u3 << 1) + u3 ;
					v3 = (v3 << 1) + v3 ;
					int16_t	b = ptrNextRGB[0] + y + (u7 >> 2) ;
					int16_t	g = ptrNextRGB[1] + y - ((u3 + v3 + v3) >> 3) ;
					int16_t	r = ptrNextRGB[2] + y + (v3 >> 1) ;
					int16_t	a = ptrNextRGB[3] + (ptrNextYUV[i + 0x18] << 1) ;
					if ( (uint16_t) b > 0xFF )
					{
						b = (~b >> 15) & 0xFF ;
					}
					if ( (uint16_t) g > 0xFF )
					{
						g = (~g >> 15) & 0xFF ;
					}
					if ( (uint16_t) r > 0xFF )
					{
						r = (~r >> 15) & 0xFF ;
					}
					if ( (uint16_t) a > 0xFF )
					{
						a = (~a >> 15) & 0xFF ;
					}
					ptrNextRGB[0] = (uint8_t) b ;
					ptrNextRGB[1] = (uint8_t) g ;
					ptrNextRGB[2] = (uint8_t) r ;
					ptrNextRGB[3] = (uint8_t) a ;
					ptrNextRGB += pitchPixel ;
				}
				ptrNextYUV += pitchPacked ;
			}
			#endif
			ptrYUVLine += pitchLine ;
			ptrRGBLine += pitchLine ;
		}
	}
	else
	{
		for ( size_t yPos = 0; yPos < nHeight; yPos ++ )
		{
			int8_t *	ptrNextYUV = ptrYUVLine ;
			uint8_t *	ptrNextRGB = ptrRGBLine ;
			#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
			if ( flagSwitchSSE2 )
			for ( size_t xPacked = 0; xPacked < widthInPacked; xPacked ++ )
			{
				__asm
				{
					mov		eax, ptrNextYUV
					mov		edx, ptrNextRGB
					movq	xmm0, MMWORD PTR [eax]
					movq	xmm5, MMWORD PTR [eax + 8]
					pxor	xmm4, xmm4
					pxor	xmm1, xmm1
					pxor	xmm2, xmm2
					movq	xmm6, MMWORD PTR [eax + 16]
					movq	xmm3, MMWORD PTR [eax + 24]
					punpcklbw	xmm0, xmm4	; xmm0 := y
					punpcklbw	xmm1, xmm5	; xmm1 := u3
					punpcklbw	xmm2, xmm6	; xmm2 := v3
											; xmm3 := a
					psraw		xmm1, 8
					psraw		xmm2, 8
					;
					movdqa	xmm4, xmm1
					movdqa	xmm5, xmm1
					movdqa	xmm6, xmm2
					psllw	xmm4, 3			; xmm4 := u7 = (u3 << 3) - u3 ;
					psllw	xmm5, 1
					psllw	xmm6, 1
					psubw	xmm4, xmm1
					paddw	xmm1, xmm5
					paddw	xmm2, xmm6
					;
					psraw	xmm4, 2			; xmm4 = y + (u7 >> 2)
					paddw	xmm1, xmm2		; xmm5 = y - ((u3 + v3 + v3) >> 3)
					paddw	xmm4, xmm0
					paddw	xmm1, xmm2
					movdqa	xmm5, xmm0
					psraw	xmm1, 3
					psraw	xmm2, 1			; xmm2 = y + (v3 >> 1)
					psubw	xmm5, xmm1
					paddw	xmm2, xmm0
					;
					packuswb	xmm4, xmm4
					packuswb	xmm5, xmm5
					packuswb	xmm2, xmm2
					punpcklbw	xmm4, xmm5	; xmm4 = g7:b7:g6:b6:g5:b5:g4:b4:g3:b3:g2:b2:g1:b1:g0:b0
					punpcklbw	xmm2, xmm3	; xmm2 = a7:r7:a6:r6:a5:r5:a4:r4:a3:r3:a2:r2:a1:r1:a0:r0
					movdqa		xmm0, xmm4
					punpcklwd	xmm4, xmm2	; xmm4 = a3:r3:g3:b3:a2:r2:g2:b2:a1:r1:g1:b1:a0:r0:g0:b0
					punpckhwd	xmm0, xmm2	; xmm0 = a7:r7:g7:b7:a6:r6:g6:b6:a5:r5:g5:b5:a4:r4:g4:b4
					;
					movdqu		[edx], xmm4
					movdqu		[edx + 16], xmm0
				}
				ptrNextRGB += pitchPixel * 8 ;
				ptrNextYUV += pitchPacked ;
			}
			else
			#endif
			#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
			if ( SSystem::g_cpuFeatures & SSystem::cpuARM_Feature_NEON )
			{
				ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON
					( ptrRGBLine, ptrYUVLine, widthInPacked ) ;
			}
			else
			{
				ERISA_sclbConvertYUVtoRGB8x8_ARMv7A
					( ptrRGBLine, ptrYUVLine, widthInPacked ) ;
			}
			#else
			for ( size_t xPacked = 0; xPacked < widthInPacked; xPacked ++ )
			{
				for ( size_t i = 0; i < 8; i ++ )
				{
					int16_t	y = *((uint8_t*)ptrNextYUV + i) ;
					int16_t	u3 = ptrNextYUV[i + 0x08] ;
					int16_t	v3 = ptrNextYUV[i + 0x10] ;
					int16_t	u7 = (u3 << 3) - u3 ;
					u3 = (u3 << 1) + u3 ;
					v3 = (v3 << 1) + v3 ;
					int16_t	b = y + (u7 >> 2) ;
					int16_t	g = y - ((u3 + v3 + v3) >> 3) ;
					int16_t	r = y + (v3 >> 1) ;
					if ( (uint16_t) b > 0xFF )
					{
						b = (~b >> 15) & 0xFF ;
					}
					ptrNextRGB[0] = (uint8_t) b ;
					if ( (uint16_t) g > 0xFF )
					{
						g = (~g >> 15) & 0xFF ;
					}
					ptrNextRGB[1] = (uint8_t) g ;
					if ( (uint16_t) r > 0xFF )
					{
						r = (~r >> 15) & 0xFF ;
					}
					ptrNextRGB[2] = (uint8_t) r ;
					ptrNextRGB[3] = ptrNextYUV[i + 0x18] ;
					ptrNextRGB += pitchPixel ;
				}
				ptrNextYUV += pitchPacked ;
			}
			#endif
			ptrYUVLine += pitchLine ;
			ptrRGBLine += pitchLine ;
		}
	}
}

// 動き補償を適用した上で画像を複製する
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::MoveImageAllBlockWithVector( void )
{
	uint8_t *	ptrRGBImage = m_ptrRGBImage ;
	for ( size_t nPosY = 0; nPosY < m_nHeightInBlocks; nPosY ++ )
	{
		MoveImageBlockLineWithVector() ;
		m_ptrRGBImage += m_nRGBLineBytes * 16 ;
	}
	m_ptrRGBImage = ptrRGBImage ;
}

void SGLImageDecoder::MoveImageBlockLineWithVector( void )
{
	MoveImageWithVector_atLine( m_ptrRGBImage, m_ptrNextPrevBlocks ) ;
	m_ptrNextPrevBlocks += m_nWidthInBlocks ;
}

void SGLImageDecoder::MoveImageWithVector_atLine
	( uint8_t * ptrRGBImage,
		SGLImageDecoder::MOVE_PREV_BLOCK * pNextPrevBlocks ) const
{
	const size_t		pitchBlock = m_nRGBPixelBytes * m_nBlockSize * 2 ;
	const size_t		nWidthInBlocks = m_nWidthInBlocks ;
	uint8_t *			ptrDstImage = ptrRGBImage ;
	MOVE_PREV_BLOCK *	pNextPrevBlock = pNextPrevBlocks ;
	for ( size_t i = 0; i < nWidthInBlocks; i ++ )
	{
		PTR_MOVE_BLOCK	pfnMovePBlock =
			m_pfnMoveBlockPFrame[m_iPrevFormat][pNextPrevBlock->flagPrevHalf] ;
		PTR_MOVE_BLOCK	pfnMoveBBlock =
			m_pfnMoveBlockBFrame[m_iNextFormat][pNextPrevBlock->flagNextHalf] ;
		if ( pNextPrevBlock->pPrevFrame != NULL )
		{
			(this->*pfnMovePBlock)
				( ptrDstImage,
					pNextPrevBlock->pPrevFrame, m_nPrevLineBytes ) ;
			if ( pNextPrevBlock->pNextFrame != NULL )
			{
				(this->*pfnMoveBBlock)
					( ptrDstImage,
						pNextPrevBlock->pNextFrame, m_nNextLineBytes ) ;
			}
		}
		else if ( pNextPrevBlock->pNextFrame != NULL )
		{
			pfnMovePBlock =
				m_pfnMoveBlockPFrame[m_iNextFormat][pNextPrevBlock->flagNextHalf] ;
			(this->*pfnMovePBlock)
				( ptrDstImage,
					pNextPrevBlock->pNextFrame, m_nNextLineBytes ) ;
		}
		else
		{
			FillZeroMoveIBlock0( ptrDstImage ) ;
		}
		ptrDstImage += pitchBlock ;
		pNextPrevBlock ++ ;
	}
}

// 前後フレームサンプリング（ゼロフィル）
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::FillZeroMoveIBlock0( uint8_t * pDstImage ) const
{
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint64_t *	pqwDst = (uint64_t*) pDstImage ;
		pqwDst[0] = 0 ;
		pqwDst[1] = 0 ;
		pqwDst[2] = 0 ;
		pqwDst[3] = 0 ;
		pqwDst[4] = 0 ;
		pqwDst[5] = 0 ;
		pqwDst[6] = 0 ;
		pqwDst[7] = 0 ;
		pDstImage += pitchDstLine ;
	}
}

// 前フレームサンプリング (RGB形式)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SamplingRGBMovePBlock0
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint64_t *			pqwDst = (uint64_t*) pDstImage ;
		const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
		//
		pqwDst[0] = pqwSrc[0] ;
		pqwDst[1] = pqwSrc[1] ;
		pqwDst[2] = pqwSrc[2] ;
		pqwDst[3] = pqwSrc[3] ;
		pqwDst[4] = pqwSrc[4] ;
		pqwDst[5] = pqwSrc[5] ;
		pqwDst[6] = pqwSrc[6] ;
		pqwDst[7] = pqwSrc[7] ;
		//
		pDstImage += pitchDstLine ;
		pSrcImage += pitchSrcLine ;
	}
}

// 水平方向半整数画素
void SGLImageDecoder::SamplingRGBMovePBlock1
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMovePBlock1_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
				movdqu	xmm2, [esi + 16]
				movdqu	xmm1, [esi + 4]
				movdqu	xmm3, [esi + 20]
				pavgb	xmm0, xmm1
				pavgb	xmm2, xmm3
				movdqu	xmm4, [esi + 32]
				movdqu	xmm6, [esi + 48]
				movdqu	xmm5, [esi + 36]
				movdqu	xmm7, [esi + 52]
				pavgb	xmm4, xmm5
				pavgb	xmm6, xmm7
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm2
				movdqu	[edi + 32], xmm4
				movdqu	[edi + 48], xmm6
				add		esi, eax
				add		edi, edx
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint32_t *			pdwDst = (uint32_t*) pDstImage ;
			const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
			uint32_t			dwLast = *(pdwSrc ++) ;
			for ( size_t x = 0; x < nBlockSize; x ++ )
			{
				uint32_t	dwNext = *(pdwSrc ++) ;
				*(pdwDst ++) = ((dwLast >> 1) & 0x7F7F7F7F)
								+ ((dwNext >> 1) & 0x7F7F7F7F)
								+ ((dwNext & dwLast) & 0x01010101) ;
				dwLast = dwNext ;
			}
			pDstImage += pitchDstLine ;
			pSrcImage += pitchSrcLine ;
		}
	}
#endif
}

// 垂直方向半整数画素
void SGLImageDecoder::SamplingRGBMovePBlock2
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMovePBlock2_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const size_t	nHalfBlockSize = 8 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
				movdqu	xmm2, [esi + 16]
				movdqu	xmm1, [esi + eax]
				movdqu	xmm3, [esi + eax + 16]
				pavgb	xmm0, xmm1
				pavgb	xmm2, xmm3
				movdqu	xmm4, [esi + 32]
				movdqu	xmm6, [esi + 48]
				movdqu	xmm5, [esi + eax + 32]
				movdqu	xmm7, [esi + eax + 48]
				pavgb	xmm4, xmm5
				pavgb	xmm6, xmm7
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm2
				movdqu	[edi + 32], xmm4
				movdqu	[edi + 48], xmm6
				add		esi, eax
				add		edi, edx
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint64_t *			pqwDst = (uint64_t*) pDstImage ;
			const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
			const uint64_t *	pqwNextSrc ;
			pSrcImage += pitchSrcLine ;
			pqwNextSrc = (const uint64_t*) pSrcImage ;
			for ( size_t x = 0; x < nHalfBlockSize; x ++ )
			{
				uint64_t	qwLast = *(pqwSrc ++) ;
				uint64_t	qwNext = *(pqwNextSrc ++) ;
				*(pqwDst ++) = ((qwLast >> 1) & 0x7F7F7F7F7F7F7F7F)
								+ ((qwNext >> 1) & 0x7F7F7F7F7F7F7F7F)
								+ ((qwNext & qwLast) & 0x0101010101010101) ;
			}
			pDstImage += pitchDstLine ;
		}
	}
#endif
}

// 水平・垂直方向半整数画素
void SGLImageDecoder::SamplingRGBMovePBlock3
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMovePBlock3_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
					movdqu	xmm4, [esi + 16]
				movdqu	xmm1, [esi + eax]
					movdqu	xmm5, [esi + eax + 16]
				movdqu	xmm2, [esi + 4]
					movdqu	xmm6, [esi + 20]
				movdqu	xmm3, [esi + eax + 4]
					movdqu	xmm7, [esi + eax + 20]
				pavgb	xmm0, xmm1
					pavgb	xmm4, xmm5
				pavgb	xmm2, xmm3
					pavgb	xmm6, xmm7
				pavgb	xmm0, xmm2
					pavgb	xmm4, xmm6
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm4
				;
				movdqu	xmm0, [esi + 32]
					movdqu	xmm4, [esi + 48]
				movdqu	xmm1, [esi + eax + 32]
					movdqu	xmm5, [esi + eax + 48]
				movdqu	xmm2, [esi + 36]
					movdqu	xmm6, [esi + 52]
				movdqu	xmm3, [esi + eax + 36]
					movdqu	xmm7, [esi + eax + 52]
				pavgb	xmm0, xmm1
					pavgb	xmm4, xmm5
				pavgb	xmm2, xmm3
					pavgb	xmm6, xmm7
				pavgb	xmm0, xmm2
					pavgb	xmm4, xmm6
				movdqu	[edi + 32], xmm0
				movdqu	[edi + 48], xmm4
				;
				add		esi, eax
				add		edi, edx
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint32_t *			pdwDst = (uint32_t*) pDstImage ;
			const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
			const uint32_t *	pdwNextSrc ;
			pSrcImage += pitchSrcLine ;
			pdwNextSrc = (const uint32_t*) pSrcImage ;
			//
			uint32_t	dwLast0 = *(pdwSrc ++) ;
			uint32_t	dwLast1 = *(pdwNextSrc ++) ;
			uint32_t	dwLast = ((dwLast0 >> 1) & 0x7F7F7F7F)
								+ ((dwLast1 >> 1) & 0x7F7F7F7F)
								+ ((dwLast0 & dwLast1) & 0x01010101) ;
			for ( size_t x = 0; x < nBlockSize; x ++ )
			{
				uint32_t	dwNext0 = *(pdwSrc ++) ;
				uint32_t	dwNext1 = *(pdwNextSrc ++) ;
				uint32_t	dwNext = ((dwNext0 >> 1) & 0x7F7F7F7F)
									+ ((dwNext1 >> 1) & 0x7F7F7F7F)
									+ ((dwNext0 & dwNext1) & 0x01010101) ;
				*(pdwDst ++) = ((dwLast >> 1) & 0x7F7F7F7F)
								+ ((dwNext >> 1) & 0x7F7F7F7F)
								+ ((dwNext & dwLast) & 0x01010101) ;
				dwLast = dwNext ;
			}
			pDstImage += pitchDstLine ;
		}
	}
#endif
}

// 前フレームサンプリング (BGR形式)
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SamplingBGRMovePBlock0
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	SamplingRGBMovePBlock0( pDstImage, pSrcImage, pitchSrcLine ) ;
	FlipBlockPixelRGBtoBGR( pDstImage, m_nRGBLineBytes ) ;
}

void SGLImageDecoder::SamplingBGRMovePBlock1
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	SamplingRGBMovePBlock1( pDstImage, pSrcImage, pitchSrcLine ) ;
	FlipBlockPixelRGBtoBGR( pDstImage, m_nRGBLineBytes ) ;
}

void SGLImageDecoder::SamplingBGRMovePBlock2
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	SamplingRGBMovePBlock2( pDstImage, pSrcImage, pitchSrcLine ) ;
	FlipBlockPixelRGBtoBGR( pDstImage, m_nRGBLineBytes ) ;
}

void SGLImageDecoder::SamplingBGRMovePBlock3
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	SamplingRGBMovePBlock3( pDstImage, pSrcImage, pitchSrcLine ) ;
	FlipBlockPixelRGBtoBGR( pDstImage, m_nRGBLineBytes ) ;
}

void SGLImageDecoder::FlipBlockPixelRGBtoBGR
			( uint8_t * pImage, int32_t pitchLine )
{
	const size_t	nBlockSize = 16 ;
	const size_t	nHalfBlockSize = 8 ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint32_t *	pdwImage = (uint32_t*) pImage ;
		pImage += pitchLine ;
		for ( size_t x = 0; x < nHalfBlockSize; x ++ )
		{
			uint32_t	dwARGB0 = pdwImage[0] ;
			uint32_t	dwARGB1 = pdwImage[1] ;
			*(pdwImage ++) = (dwARGB0 & 0xFF00FF00)
							| ((dwARGB0 & 0x000000FF) << 16)
							| ((dwARGB0 & 0x00FF0000) >> 16) ;
			*(pdwImage ++) = (dwARGB1 & 0xFF00FF00)
							| ((dwARGB1 & 0x000000FF) << 16)
							| ((dwARGB1 & 0x00FF0000) >> 16) ;
		}
	}
}

// 後フレームサンプリング (RGB形式)（出力先に合成）
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SamplingRGBMoveBBlock0
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const size_t	nHalfBlockSize = 8 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [edi]
				movdqu	xmm2, [edi + 16]
				movdqu	xmm4, [edi + 32]
				movdqu	xmm6, [edi + 48]
				movdqu	xmm1, [esi]
				movdqu	xmm3, [esi + 16]
				movdqu	xmm5, [esi + 32]
				movdqu	xmm7, [esi + 48]
				pavgb	xmm0, xmm1
				pavgb	xmm2, xmm3
				pavgb	xmm4, xmm5
				pavgb	xmm6, xmm7
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm2
				movdqu	[edi + 32], xmm4
				movdqu	[edi + 48], xmm6
				add		edi, edx
				add		esi, eax
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint64_t *			pqwDst = (uint64_t*) pDstImage ;
			const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
			for ( size_t x = 0; x < nHalfBlockSize; x ++ )
			{
				uint64_t	qwDst = *pqwDst ;
				uint64_t	qwSrc = *(pqwSrc ++) ;
				*(pqwDst ++) = ((qwDst >> 1) & 0x7F7F7F7F7F7F7F7F)
								+ ((qwSrc >> 1) & 0x7F7F7F7F7F7F7F7F)
								+ ((qwDst & qwSrc) & 0x0101010101010101) ;
			}
			pDstImage += pitchDstLine ;
			pSrcImage += pitchSrcLine ;
		}
	}
#endif
}

// 水平方向半整数画素
void SGLImageDecoder::SamplingRGBMoveBBlock1
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
				movdqu	xmm1, [esi + 16]
				movdqu	xmm4, [edi]
				movdqu	xmm5, [edi + 16]
				movdqu	xmm2, [esi + 4]
				movdqu	xmm3, [esi + 20]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm1
				;
				movdqu	xmm0, [esi + 32]
				movdqu	xmm1, [esi + 48]
				movdqu	xmm4, [edi + 32]
				movdqu	xmm5, [edi + 48]
				movdqu	xmm2, [esi + 36]
				movdqu	xmm3, [esi + 52]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				movdqu	[edi + 32], xmm0
				movdqu	[edi + 48], xmm1
				add		edi, edx
				add		esi, eax
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint32_t *			pdwDst = (uint32_t*) pDstImage ;
			const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
			uint32_t			dwLast = *(pdwSrc ++) ;
			for ( size_t x = 0; x < nBlockSize; x ++ )
			{
				uint32_t	dwNext = *(pdwSrc ++) ;
				uint32_t	dwDst = *pdwDst ;
				uint32_t	dwSrc = ((dwLast >> 1) & 0x7F7F7F7F)
									+ ((dwNext >> 1) & 0x7F7F7F7F)
									+ ((dwNext & dwLast) & 0x01010101) ;
				*(pdwDst ++) = ((dwDst >> 1) & 0x7F7F7F7F)
									+ ((dwSrc >> 1) & 0x7F7F7F7F)
									+ ((dwDst & dwSrc) & 0x01010101) ;
				dwLast = dwNext ;
			}
			pDstImage += pitchDstLine ;
			pSrcImage += pitchSrcLine ;
		}
	}
#endif
}

// 垂直方向半整数画素
void SGLImageDecoder::SamplingRGBMoveBBlock2
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const size_t	nHalfBlockSize = 8 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
				movdqu	xmm1, [esi + 16]
				movdqu	xmm2, [esi + eax]
				movdqu	xmm3, [esi + eax + 16]
				movdqu	xmm4, [edi]
				movdqu	xmm5, [edi + 16]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm1
				;
				movdqu	xmm0, [esi + 32]
				movdqu	xmm1, [esi + 48]
				movdqu	xmm2, [esi + eax + 32]
				movdqu	xmm3, [esi + eax + 48]
				movdqu	xmm4, [edi + 32]
				movdqu	xmm5, [edi + 48]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				movdqu	[edi + 32], xmm0
				movdqu	[edi + 48], xmm1
				add		edi, edx
				add		esi, eax
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint64_t *			pqwDst = (uint64_t*) pDstImage ;
			const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
			const uint64_t *	pqwNextSrc ;
			pSrcImage += pitchSrcLine ;
			pqwNextSrc = (const uint64_t*) pSrcImage ;
			for ( size_t x = 0; x < nHalfBlockSize; x ++ )
			{
				uint64_t	qwLast = *(pqwSrc ++) ;
				uint64_t	qwNext = *(pqwNextSrc ++) ;
				uint64_t	qwSrc = ((qwLast >> 1) & 0x7F7F7F7F7F7F7F7F)
									+ ((qwNext >> 1) & 0x7F7F7F7F7F7F7F7F)
									+ ((qwNext & qwLast) & 0x0101010101010101) ;
				uint64_t	qwDst = *pqwDst ;
				*(pqwDst ++) = ((qwDst >> 1) & 0x7F7F7F7F7F7F7F7F)
									+ ((qwSrc >> 1) & 0x7F7F7F7F7F7F7F7F)
									+ ((qwDst & qwSrc) & 0x0101010101010101) ;
			}
			pDstImage += pitchDstLine ;
		}
	}
#endif
}

// 水平・垂直方向半整数画素
void SGLImageDecoder::SamplingRGBMoveBBlock3
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
	ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A
		( pDstImage, m_nRGBLineBytes, pSrcImage, pitchSrcLine ) ;
#else
	ESLAssert( m_nBlockSize == 8 ) ;
	ESLAssert( m_nRGBPixelBytes == 4 ) ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		__asm
		{
			mov		ecx, nBlockSize
			mov		edi, pDstImage
			mov		esi, pSrcImage
			mov		edx, pitchDstLine
			mov		eax, pitchSrcLine
		LoopBegin1:
				movdqu	xmm0, [esi]
				movdqu	xmm1, [esi + 16]
				movdqu	xmm2, [esi + eax]
				movdqu	xmm3, [esi + eax + 16]
				movdqu	xmm4, [esi + 4]
				movdqu	xmm5, [esi + 20]
				movdqu	xmm6, [esi + eax + 4]
				movdqu	xmm7, [esi + eax + 20]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm4, xmm6
				pavgb	xmm5, xmm7
				movdqu	xmm2, [edi]
				movdqu	xmm3, [edi + 16]
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				movdqu	[edi], xmm0
				movdqu	[edi + 16], xmm1
				;
				movdqu	xmm0, [esi + 32]
				movdqu	xmm1, [esi + 48]
				movdqu	xmm2, [esi + eax + 32]
				movdqu	xmm3, [esi + eax + 48]
				movdqu	xmm4, [esi + 36]
				movdqu	xmm5, [esi + 52]
				movdqu	xmm6, [esi + eax + 36]
				movdqu	xmm7, [esi + eax + 52]
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				pavgb	xmm4, xmm6
				pavgb	xmm5, xmm7
				movdqu	xmm2, [edi + 32]
				movdqu	xmm3, [edi + 48]
				pavgb	xmm0, xmm4
				pavgb	xmm1, xmm5
				pavgb	xmm0, xmm2
				pavgb	xmm1, xmm3
				movdqu	[edi + 32], xmm0
				movdqu	[edi + 48], xmm1
				add		edi, edx
				add		esi, eax
				dec		ecx
			jnz		LoopBegin1
		}
	}
	else
	#endif
	{
		for ( size_t y = 0; y < nBlockSize; y ++ )
		{
			uint32_t *			pdwDst = (uint32_t*) pDstImage ;
			const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
			const uint32_t *	pdwNextSrc ;
			pSrcImage += pitchSrcLine ;
			pdwNextSrc = (const uint32_t*) pSrcImage ;
			//
			uint32_t	dwLast0 = *(pdwSrc ++) ;
			uint32_t	dwLast1 = *(pdwNextSrc ++) ;
			uint32_t	dwLast = ((dwLast0 >> 1) & 0x7F7F7F7F)
								+ ((dwLast1 >> 1) & 0x7F7F7F7F)
								+ ((dwLast0 & dwLast1) & 0x01010101) ;
			for ( size_t x = 0; x < nBlockSize; x ++ )
			{
				uint32_t	dwNext0 = *(pdwSrc ++) ;
				uint32_t	dwNext1 = *(pdwNextSrc ++) ;
				uint32_t	dwNext = ((dwNext0 >> 1) & 0x7F7F7F7F)
									+ ((dwNext1 >> 1) & 0x7F7F7F7F)
									+ ((dwNext0 & dwNext1) & 0x01010101) ;
				uint32_t	dwDst = *pdwDst ;
				uint32_t	dwSrc = ((dwLast >> 1) & 0x7F7F7F7F)
									+ ((dwNext >> 1) & 0x7F7F7F7F)
									+ ((dwNext & dwLast) & 0x01010101) ;
				*(pdwDst ++) = ((dwDst >> 1) & 0x7F7F7F7F)
									+ ((dwSrc >> 1) & 0x7F7F7F7F)
									+ ((dwDst & dwSrc) & 0x01010101) ;
				dwLast = dwNext ;
			}
			pDstImage += pitchDstLine ;
		}
	}
#endif
}

// 後フレームサンプリング (BGR形式)（出力先に合成）
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::SamplingBGRMoveBBlock0
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	uint8_t			bufBlock[16][16][4] ;
	const size_t	nBlockSize = 16 ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint64_t *			pqwDst = (uint64_t*) &(bufBlock[y][0][0]) ;
		const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
		//
		pqwDst[0] = pqwSrc[0] ;
		pqwDst[1] = pqwSrc[1] ;
		pqwDst[2] = pqwSrc[2] ;
		pqwDst[3] = pqwSrc[3] ;
		pqwDst[4] = pqwSrc[4] ;
		pqwDst[5] = pqwSrc[5] ;
		pqwDst[6] = pqwSrc[6] ;
		pqwDst[7] = pqwSrc[7] ;
		//
		pSrcImage += pitchSrcLine ;
	}
	FlipBlockPixelRGBtoBGR( &(bufBlock[0][0][0]), 16*4 ) ;
	SamplingRGBMoveBBlock0( pDstImage, &(bufBlock[0][0][0]), 16*4 ) ;
}

void SGLImageDecoder::SamplingBGRMoveBBlock1
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	uint8_t			bufBlock[16][16][4] ;
	const size_t	nBlockSize = 16 ;
	const ssize_t	pitchDstLine = m_nRGBLineBytes ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint32_t *			pdwDst = (uint32_t*) &(bufBlock[y][0][0]) ;
		const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
		uint32_t			dwLast = *(pdwSrc ++) ;
		for ( size_t x = 0; x < nBlockSize; x ++ )
		{
			uint32_t	dwNext = *(pdwSrc ++) ;
			*(pdwDst ++) = ((dwLast >> 1) & 0x7F7F7F7F)
							+ ((dwNext >> 1) & 0x7F7F7F7F)
							+ ((dwNext & dwLast) & 0x01010101) ;
			dwLast = dwNext ;
		}
		pSrcImage += pitchSrcLine ;
	}
	FlipBlockPixelRGBtoBGR( &(bufBlock[0][0][0]), 16*4 ) ;
	SamplingRGBMoveBBlock0( pDstImage, &(bufBlock[0][0][0]), 16*4 ) ;
}

void SGLImageDecoder::SamplingBGRMoveBBlock2
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	uint8_t			bufBlock[16][16][4] ;
	const size_t	nBlockSize = 16 ;
	const size_t	nHalfBlockSize = 8 ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint64_t *			pqwDst = (uint64_t*) &(bufBlock[y][0][0]) ;
		const uint64_t *	pqwSrc = (const uint64_t*) pSrcImage ;
		const uint64_t *	pqwNextSrc ;
		pSrcImage += pitchSrcLine ;
		pqwNextSrc = (const uint64_t*) pSrcImage ;
		for ( size_t x = 0; x < nHalfBlockSize; x ++ )
		{
			uint64_t	qwLast = *(pqwSrc ++) ;
			uint64_t	qwNext = *(pqwNextSrc ++) ;
			*(pqwDst ++) = ((qwLast >> 1) & 0x7F7F7F7F7F7F7F7F)
							+ ((qwNext >> 1) & 0x7F7F7F7F7F7F7F7F)
							+ ((qwNext & qwLast) & 0x0101010101010101) ;
		}
	}
	FlipBlockPixelRGBtoBGR( &(bufBlock[0][0][0]), 16*4 ) ;
	SamplingRGBMoveBBlock0( pDstImage, &(bufBlock[0][0][0]), 16*4 ) ;
}

void SGLImageDecoder::SamplingBGRMoveBBlock3
	( uint8_t * pDstImage,
		const uint8_t * pSrcImage, int32_t pitchSrcLine ) const
{
	uint8_t			bufBlock[16][16][4] ;
	const size_t	nBlockSize = 16 ;
	for ( size_t y = 0; y < nBlockSize; y ++ )
	{
		uint32_t *			pdwDst = (uint32_t*) &(bufBlock[y][0][0]) ;
		const uint32_t *	pdwSrc = (const uint32_t*) pSrcImage ;
		const uint32_t *	pdwNextSrc ;
		pSrcImage += pitchSrcLine ;
		pdwNextSrc = (const uint32_t*) pSrcImage ;
		//
		uint32_t	dwLast0 = *(pdwSrc ++) ;
		uint32_t	dwLast1 = *(pdwNextSrc ++) ;
		uint32_t	dwLast = ((dwLast0 >> 1) & 0x7F7F7F7F)
							+ ((dwLast1 >> 1) & 0x7F7F7F7F)
							+ ((dwLast0 & dwLast1) & 0x01010101) ;
		for ( size_t x = 0; x < nBlockSize; x ++ )
		{
			uint32_t	dwNext0 = *(pdwSrc ++) ;
			uint32_t	dwNext1 = *(pdwNextSrc ++) ;
			uint32_t	dwNext = ((dwNext0 >> 1) & 0x7F7F7F7F)
								+ ((dwNext1 >> 1) & 0x7F7F7F7F)
								+ ((dwNext0 & dwNext1) & 0x01010101) ;
			*(pdwDst ++) = ((dwLast >> 1) & 0x7F7F7F7F)
							+ ((dwNext >> 1) & 0x7F7F7F7F)
							+ ((dwNext & dwLast) & 0x01010101) ;
			dwLast = dwNext ;
		}
	}
	FlipBlockPixelRGBtoBGR( &(bufBlock[0][0][0]), 16*4 ) ;
	SamplingRGBMoveBBlock0( pDstImage, &(bufBlock[0][0][0]), 16*4 ) ;
}

// グレイ画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::LS_RestoreGray8
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		eslMoveMemory( ptrDstImage, ptrSrcImage, nDstWidth ) ;
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

// RGB 画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::LS_RestoreRGB24
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint8_t *	pbytSrcLine = ptrSrcImage ;
		uint8_t *		pbytDstLine = ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			pbytDstLine[0] = pbytSrcLine[0] ;
			pbytDstLine[1] = pbytSrcLine[1] ;
			pbytDstLine[2] = pbytSrcLine[2] ;
			pbytSrcLine += 4 ;
			pbytDstLine += 3 ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

void SGLImageDecoder::LS_RestoreBGR24
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint8_t *	pbytSrcLine = ptrSrcImage ;
		uint8_t *		pbytDstLine = ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			pbytDstLine[0] = pbytSrcLine[2] ;
			pbytDstLine[1] = pbytSrcLine[1] ;
			pbytDstLine[2] = pbytSrcLine[0] ;
			pbytSrcLine += 4 ;
			pbytDstLine += 3 ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

void SGLImageDecoder::LS_RestoreRGB32
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint32_t *	pdwSrcLine = (const uint32_t*) ptrSrcImage ;
		uint32_t *			pdwDstLine = (uint32_t*) ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			*(pdwDstLine ++) = *(pdwSrcLine ++) | 0xFF000000 ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

void SGLImageDecoder::LS_RestoreBGR32
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint8_t *	pbytSrcLine = ptrSrcImage ;
		uint8_t *		pbytDstLine = ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			pbytDstLine[0] = pbytSrcLine[2] ;
			pbytDstLine[1] = pbytSrcLine[1] ;
			pbytDstLine[2] = pbytSrcLine[0] ;
			pbytDstLine[3] = 0xFF ;
			pbytSrcLine += 4 ;
			pbytDstLine += 4 ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

// RGBA 画像の出力
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::LS_RestoreRGBA32
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint32_t *	pdwSrcLine = (const uint32_t*) ptrSrcImage ;
		uint32_t *			pdwDstLine = (uint32_t*) ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			*(pdwDstLine ++) = *(pdwSrcLine ++) ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

void SGLImageDecoder::LS_RestoreBGRA32
	( uint8_t * ptrDstImage,
		const uint8_t * ptrSrcImage,
		size_t nDstWidth, size_t nDstHeight ) const
{
	const ssize_t	nSrcLineBytes = m_nRGBLineBytes ;
	const ssize_t	nDstLineBytes = m_nDstLineBytes ;
	for ( size_t y = 0; y < nDstHeight; y ++ )
	{
		const uint8_t *	pbytSrcLine = ptrSrcImage ;
		uint8_t *		pbytDstLine = ptrDstImage ;
		for ( size_t x = 0; x < nDstWidth; x ++ )
		{
			pbytDstLine[0] = pbytSrcLine[2] ;
			pbytDstLine[1] = pbytSrcLine[1] ;
			pbytDstLine[2] = pbytSrcLine[0] ;
			pbytDstLine[3] = pbytSrcLine[3] ;
			pbytSrcLine += 4 ;
			pbytDstLine += 4 ;
		}
		ptrSrcImage += nSrcLineBytes ;
		ptrDstImage += nDstLineBytes ;
	}
}

// 画像出力関数取得
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoder::PTR_RESTORE_FUNC SGLImageDecoder::GetLSRestoreFunc
	( uint32_t formatImage, uint32_t nBitsPerPixel, uint32_t flagsDecode ) const
{
	switch ( nBitsPerPixel )
	{
	case	32:
		if ( (formatImage & formatImageTypeMask) == formatImageBGR )
		{
			if ( formatImage & formatImageFlagAlpha )
			{
				return	&SGLImageDecoder::LS_RestoreBGRA32 ;
			}
			else
			{
				return	&SGLImageDecoder::LS_RestoreBGR32 ;
			}
		}
		else
		{
			if ( formatImage & formatImageFlagAlpha )
			{
				return	&SGLImageDecoder::LS_RestoreRGBA32 ;
			}
			else
			{
				return	&SGLImageDecoder::LS_RestoreRGB32 ;
			}
		}

	case	24:
		if ( (formatImage & formatImageTypeMask) == formatImageBGR )
		{
			return	&SGLImageDecoder::LS_RestoreBGR24 ;
		}
		else
		{
			return	&SGLImageDecoder::LS_RestoreRGB24 ;
		}

	case	8:
		return	&SGLImageDecoder::LS_RestoreGray8 ;

	}
	return	NULL ;
}


// 加算（ARGB 加算・可逆圧縮差分復元用）
//////////////////////////////////////////////////////////////////////////////
SGLError ERISA::eriWrapAroundAddImageBuffer
	( const SakuraGL::SGLImageBuffer& imgDst,
		const SakuraGL::SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SakuraGL::SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel < 3) | (imgSrc.pitchPixel < 3) )
	{
		return	sglErrInvalidParam ;
	}
	SakuraGL::SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		SakuraGL::sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	bool	fPixelQuadBytes =
		(infDst.pitchPixel == 4) && (infSrc.pitchPixel == 4) ;
	//
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint8_t *	pDstNextPixel = pDstNextLine ;
		uint8_t *	pSrcNextPixel = pSrcNextLine ;
		if ( fPixelQuadBytes )
		{
			for ( uint32_t x = 0; x < infDst.width; x ++ )
			{
				pDstNextPixel[0] += pSrcNextPixel[0] ;
				pDstNextPixel[1] += pSrcNextPixel[1] ;
				pDstNextPixel[2] += pSrcNextPixel[2] ;
				pDstNextPixel[3] += pSrcNextPixel[3] ;
				pDstNextPixel += 4 ;
				pSrcNextPixel += 4 ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < infDst.width; x ++ )
			{
				pDstNextPixel[0] += pSrcNextPixel[0] ;
				pDstNextPixel[1] += pSrcNextPixel[1] ;
				pDstNextPixel[2] += pSrcNextPixel[2] ;
				pDstNextPixel += infDst.pitchPixel ;
				pSrcNextPixel += infSrc.pitchPixel ;
			}
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}

// 減算（ARGB 減算・可逆圧縮差分用）
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError
	ERISA::eriWrapAroundSubImageBuffer
		( const SakuraGL::SGLImageBuffer& imgDst,
			const SakuraGL::SGLImageBuffer& imgSrc,
			int xPos, int yPos,
			const SakuraGL::SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel < 3) | (imgSrc.pitchPixel < 3) )
	{
		return	sglErrInvalidParam ;
	}
	SakuraGL::SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		SakuraGL::sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	bool	fPixelQuadBytes =
		(infDst.pitchPixel == 4) && (infSrc.pitchPixel == 4) ;
	//
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint8_t *	pDstNextPixel = pDstNextLine ;
		uint8_t *	pSrcNextPixel = pSrcNextLine ;
		if ( fPixelQuadBytes )
		{
			for ( uint32_t x = 0; x < infDst.width; x ++ )
			{
				pDstNextPixel[0] -= pSrcNextPixel[0] ;
				pDstNextPixel[1] -= pSrcNextPixel[1] ;
				pDstNextPixel[2] -= pSrcNextPixel[2] ;
				pDstNextPixel[3] -= pSrcNextPixel[3] ;
				pDstNextPixel += 4 ;
				pSrcNextPixel += 4 ;
			}
		}
		else
		{
			for ( uint32_t x = 0; x < infDst.width; x ++ )
			{
				pDstNextPixel[0] -= pSrcNextPixel[0] ;
				pDstNextPixel[1] -= pSrcNextPixel[1] ;
				pDstNextPixel[2] -= pSrcNextPixel[2] ;
				pDstNextPixel += infDst.pitchPixel ;
				pSrcNextPixel += infSrc.pitchPixel ;
			}
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}

// 画像半画素フィルタ
//////////////////////////////////////////////////////////////////////////////
SGLError ERISA::eriImageFilterHalf1111
	( const SakuraGL::SGLImageBuffer & bufDstImage,
		const SakuraGL::SGLImageBuffer & bufSrcImage )
{
	SGLImageBuffer	imgIsDst, imgIsSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( imgIsDst, imgIsSrc, bufDstImage, bufSrcImage ) ;
	if ( err )
	{
		return	err ;
	}
	ESLAssert( imgIsDst.pitchPixel == 4 ) ;
	ESLAssert( imgIsSrc.pitchPixel == 4 ) ;
	if ( (imgIsDst.pitchPixel != 4)
		| (imgIsSrc.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint8_t *		ptrDstLine = imgIsDst.ptrBuffer ;
	const uint8_t *	ptrSrcLine = imgIsSrc.ptrBuffer ;
	const size_t	widthImage = imgIsDst.width ;
	const size_t	heightImage = imgIsDst.height ;
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	if ( SSystem::g_cpuFeatures & SSystem::cpuX86_Feature_SSE2 )
	{
		const size_t	widthImageM1 = widthImage - 1 ;
		const int32_t	pitchSrcLine = imgIsSrc.pitchLine ;
		const int32_t	pitchDstLine = imgIsDst.pitchLine ;
		__asm
		{
			mov		ecx, heightImage
			test	ecx, ecx
			jz		LOOP_VERT_END
			dec		ecx
			jz		LOOP_VERT_END
			mov		esi, ptrSrcLine
			mov		edi, ptrDstLine
LOOP_VERT_BEGIN:
				push	ecx
				mov		eax, pitchSrcLine
				lea		ebx, [esi + eax]
				;
				mov		ecx, widthImageM1
				shr		ecx, 3
				jz		LOOP_HORZ_ENDx8
LOOP_HORZ_BEGINx8:
					movdqu	xmm0, [esi]
					movdqu	xmm1, [ebx]
					movdqu	xmm2, [esi + 10H]
					movdqu	xmm3, [ebx + 10H]
					movd	xmm4, [esi + 20H]
					movd	xmm5, [ebx + 20H]
					add		esi, 20H
					add		ebx, 20H
					pavgb	xmm0, xmm1
					pavgb	xmm2, xmm3
					pavgb	xmm4, xmm5
					movdqa	xmm1, xmm0
					movdqa	xmm3, xmm2
					movdqa	xmm5, xmm2
					psrldq	xmm1, 4
					psrldq	xmm3, 4
					pslldq	xmm5, 4*3
					pslldq	xmm4, 4*3
					por		xmm1, xmm5
					por		xmm3, xmm4
					pavgb	xmm0, xmm1
					pavgb	xmm2, xmm3
					movdqu	[edi], xmm0
					movdqu	[edi + 10H], xmm2
					add		edi, 20H
					dec		ecx
				jnz		LOOP_HORZ_BEGINx8
LOOP_HORZ_ENDx8:
				mov		ecx, widthImageM1
				and		ecx, 07H
				jz		LOOP_HORZ_ENDx1
LOOP_HORZ_BEGINx1:
					movq	xmm0, MMWORD PTR [esi]
					movq	xmm1, MMWORD PTR [ebx]
					add		esi, 4
					add		ebx, 4
					pavgb	xmm0, xmm1
					pshufd	xmm1, xmm0, 1
					pavgb	xmm0, xmm1
					movd	DWORD PTR [edi], xmm0
					add		edi, 4
					dec		ecx
				jnz		LOOP_HORZ_BEGINx1
LOOP_HORZ_ENDx1:
				mov		eax, DWORD PTR [esi]
				mov		DWORD PTR [edi], eax
				;
				mov		esi, ptrSrcLine
				mov		edi, ptrDstLine
				add		esi, pitchSrcLine
				add		edi, pitchDstLine
				mov		ptrSrcLine, esi
				mov		ptrDstLine, edi
				;
				pop		ecx
				dec		ecx
			jnz		LOOP_VERT_BEGIN
LOOP_VERT_END:
			;
			mov		ecx, widthImage
LOOP_LAST_LINE_BEGIN:
				mov		eax, DWORD PTR [esi]
				mov		DWORD PTR [edi], eax
				add		esi, 4
				add		edi, 4
				dec		ecx
			jnz		LOOP_LAST_LINE_BEGIN
		}
	}
	else
	{
	#endif
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		ERISA_ImageFilterHalf1111_ARMv7A
			( ptrDstLine, imgIsDst.pitchLine,
				ptrSrcLine, imgIsSrc.pitchLine, widthImage, heightImage ) ;
	#else
		for ( size_t y = 1; y < heightImage; y ++ )
		{
			uint32_t *			pdwDst = (uint32_t*) ptrDstLine ;
			const uint32_t *	pdwSrc0 = (const uint32_t*) ptrSrcLine ;
			const uint32_t *	pdwSrc1 ;
			ptrSrcLine += imgIsSrc.pitchLine ;
			pdwSrc1 = (const uint32_t*) ptrSrcLine ;
			//
			uint32_t	dwLast0 = *pdwSrc0 ;
			uint32_t	dwLast1 = *pdwSrc1 ;
			uint32_t	dwLast = ((dwLast0 >> 1) & 0x7F7F7F7F)
								+ ((dwLast1 >> 1) & 0x7F7F7F7F)
								+ ((dwLast0 & dwLast1) & 0x01010101) ;
			//
			for ( size_t x = 1; x < widthImage; x ++ )
			{
				uint32_t	dwNext0 = *(++ pdwSrc0) ;
				uint32_t	dwNext1 = *(++ pdwSrc1) ;
				uint32_t	dwNext = ((dwNext0 >> 1) & 0x7F7F7F7F)
									+ ((dwNext1 >> 1) & 0x7F7F7F7F)
									+ ((dwNext0 & dwNext1) & 0x01010101) ;
				*(pdwDst ++) = ((dwLast >> 1) & 0x7F7F7F7F)
									+ ((dwNext >> 1) & 0x7F7F7F7F)
									+ ((dwLast & dwNext) & 0x01010101) ;
				dwLast = dwNext ;
			}
			*pdwDst = *pdwSrc0 ;
			//
			ptrDstLine += imgIsDst.pitchLine ;
		}
		eslMoveMemory( ptrDstLine, ptrSrcLine, widthImage * 4 ) ;
	#endif
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
	}
	#endif
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 非可逆圧縮・並列展開スレッド関数
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoder::DecodeLossyLineProc::DecodeLossyLineProc
	( SGLImageDecoder * decoder,
		uint32_t flagsDecoding,
		SGLAbstractDecodeContext * pContext,
		SGLImageDecoder::PTR_BLOCK_SCALING_LINE pfnScaling,
		SGLImageDecoder::PTR_RESTORE_FUNC pfnRestore )
{
	m_decoder = decoder ;
	//
	m_flagsDecoding = flagsDecoding ;
	m_nPosY = 0 ;
	m_ptrQParam = (int8_t*) (decoder->m_ptrOperations) ;
	m_ptrDstBlock = decoder->m_ptrDstBlock ;
	m_ptrNextPrevBlocks = decoder->m_ptrNextPrevBlocks ;
	//
	m_pContext = pContext ;
	m_pfnScaling = pfnScaling ;
	m_pfnRestore = pfnRestore ;
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool SGLImageDecoder::DecodeLossyLineProc::Continue( void * pInstance )
{
	if ( m_nPosY >= m_decoder->m_nHeightInBlocks )
	{
		return	false ;
	}
	LOSSY_DCT_DECODING_LINE *	plddl = (LOSSY_DCT_DECODING_LINE*) pInstance ;
	SGLImageDecoder *	decoder = m_decoder ;
	const size_t	nLineBlockSamples =
		decoder->m_nWidthInBlocks
			* decoder->m_nBlockArea * decoder->m_nBlocksetCount ;
	if ( m_pContext->Read
		( plddl->ptrSrcData, nLineBlockSamples ) < nLineBlockSamples )
	{
		return	false ;
	}
	//
	plddl->nPosY = m_nPosY ;
	m_nPosY ++ ;
	//
	plddl->ptrQParam = m_ptrQParam ;
	m_ptrQParam += decoder->m_nWidthInBlocks * 2 ;
	//
	const size_t	nBlockSize = 16 ;
	plddl->ptrDstBlock = m_ptrDstBlock ;
	m_ptrDstBlock += decoder->m_nDstLineBytes * nBlockSize ;
	//
	plddl->pMovePrevBlock = m_ptrNextPrevBlocks ;
	m_ptrNextPrevBlocks += decoder->m_nWidthInBlocks ;
	//
	return	true ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoder::DecodeLossyLineProc::RunParallel( void * pInstance )
{
	LOSSY_DCT_DECODING_LINE *	plddl = (LOSSY_DCT_DECODING_LINE*) pInstance ;
	SGLImageDecoder *	decoder = m_decoder ;
	//
	const size_t	nWidthInBlocks = decoder->m_nWidthInBlocks ;
	int8_t *		ptrSrcData = plddl->ptrSrcData ;
	int8_t *		ptrQParam = plddl->ptrQParam ;
	//
	for ( size_t nPosX = 0; nPosX < nWidthInBlocks; nPosX ++ )
	{
		//
		// 逆量子化
		//
		decoder->ArrangeAndIQuantumize_atLine
			( &(plddl->ptrBlocksetBuf[0]),
				plddl->ptrIQParamBuf, ptrSrcData, ptrQParam ) ;
		ptrSrcData += decoder->m_nBlockArea * decoder->m_nBlocksetCount ;
		ptrQParam += 2 ;
		//
		// 逆 LOT/DCT 変換
		//
		decoder->MatrixIDCT8x8_atLine( &(plddl->ptrBlocksetBuf[0]) ) ;
		//
		// 画像スケーリング
		//
		(decoder->*m_pfnScaling)
			( plddl->ptrYUVImage,
				&(plddl->ptrBlocksetBuf[0]),
				(unsigned int) nPosX, 0, m_flagsDecoding ) ;
	}
	ESLAssert( decoder->m_nBlockSize == 8 ) ;
	const size_t	nBlockSize = 16 ;
	size_t	nDstHeight = decoder->m_nDstHeight - plddl->nPosY * nBlockSize ;
	if ( nDstHeight > nBlockSize )
	{
		nDstHeight = nBlockSize ;
	}
	if ( m_flagsDecoding & flagDifferential )
	{
		decoder->MoveImageWithVector_atLine
			( plddl->ptrRGBImage, plddl->pMovePrevBlock ) ;
	}
	decoder->ConvertImageYUVtoRGB_atLine
		( plddl->ptrRGBImage, plddl->ptrYUVImage, 1, m_flagsDecoding ) ;
	//
	(decoder->*m_pfnRestore)
		( plddl->ptrDstBlock,
			plddl->ptrRGBImage, decoder->m_nDstWidth, nDstHeight ) ;
}


