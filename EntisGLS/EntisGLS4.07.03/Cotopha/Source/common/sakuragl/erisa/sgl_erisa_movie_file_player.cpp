
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// レコード先読みオブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::PreloadBuffer::PreloadBuffer( size_t nLength )
{
	SetLength( nLength ) ;
}

SGLMovieFilePlayer::PreloadBuffer::PreloadBuffer
		( const SGLMovieFilePlayer::PreloadBuffer& src )
	: SByteBuffer( src ),
		m_iFrameIndex( src.m_iFrameIndex ),
		m_ui64ChunkType( src.m_ui64ChunkType )
{
}


//////////////////////////////////////////////////////////////////////////////
// MEI 動画ファイルストリーム再生オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLMovieFilePlayer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::SGLMovieFilePlayer( void )
{
	m_flagWaveOutput = false ;
	m_flagWaveStreaming = false ;
	m_iDstBufIndex = 0 ;
	for ( size_t i = 0; i < bufferCount; i ++ )
	{
		m_pDstImage[i] = NULL ;
		m_iDstFrame[i] = -1 ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::~SGLMovieFilePlayer( void )
{
	SGLMovieFilePlayer::Close( ) ;
}

// アニメーションファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMovieFilePlayer::OpenMovieFile
	( SSystem::SFileInterface * pFile,
		bool flagOwner, uint32_t flagsDecode )
{
	Close( ) ;
	//
	// ファイルを開く（ストリームレコードまで開く）
	//////////////////////////////////////////////////////////////////////////
	if ( m_erif.OpenMediaFile( pFile, SGLMediaFile::openStream, flagOwner ) )
	{
		return	errFailed ;
	}
	//
	// 展開オブジェクトを初期化する
	//////////////////////////////////////////////////////////////////////////
	if ( m_decoderImage.Initialize( m_erif.m_eriInfoHeader ) )
	{
		return	errFailed ;
	}
	m_flagWaveOutput = false ;
	if ( m_erif.m_flagsRead & SGLMediaFile::readSoundInfo )
	{
		//
		// サウンドが含まれている
		//
		if ( m_decoderSound.Initialize( m_erif.m_mioInfoHeader ) == errSuccess )
		{
			//
			// 音声出力要求
			//
			if ( RequestWaveOut
				( m_erif.m_mioInfoHeader.dwChannelCount,
					m_erif.m_mioInfoHeader.dwSamplesPerSec,
					m_erif.m_mioInfoHeader.dwBitsPerSample ) )
			{
				// 音声出力要求が受け入れられた
				m_flagWaveOutput = true ;
			}
		}
	}
	m_bstream = new SGLDecodeBitStream( 0x4000 ) ;
	//
	// 画像バッファを生成
	//////////////////////////////////////////////////////////////////////////
	m_flagTopDown = ((flagsDecode & SGLImageDecoder::flagTopDown) != 0) ;
	m_flagsDecode = flagsDecode ;
	//
	uint32_t	nImageWidth, nImageHeight ;
	uint32_t	nClipWidth, nClipHeight ;
	uint32_t	nCastSizeFlags = 0 ;
	bool		fClipImageBuf = false ;
	nImageWidth = m_erif.m_eriInfoHeader.nImageWidth ;
	nImageHeight = m_erif.m_eriInfoHeader.nImageHeight ;
	if ( m_erif.m_eriInfoHeader.nImageHeight < 0 )
	{
		nImageHeight = - m_erif.m_eriInfoHeader.nImageHeight ;
	}
	if ( (m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationDCT)
		|| (m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationLOT) )
	{
		if ( (nImageWidth & 0x0F) | (nImageHeight & 0x0F) )
		{
			nClipWidth = nImageWidth ;
			nClipHeight = nImageHeight ;
			nImageWidth = (nImageWidth + 0x0F) & ~0x0F ;
			nImageHeight = (nImageHeight + 0x0F) & ~0x0F ;
			fClipImageBuf = true ;
			nCastSizeFlags = SGLImageObject::formatCastSize ;
			if ( m_erif.m_eriInfoHeader.nImageHeight >= 0 )
			{
				nCastSizeFlags |= SGLImageObject::formatCastSizeBottom ;
			}
		}
	}
	uint32_t	nBitsPerPixel = m_erif.m_eriInfoHeader.dwBitsPerPixel ;
	if ( nBitsPerPixel == 24 )
	{
		nBitsPerPixel = 32 ;
	}
	size_t	i, nCount = bufferCount ;
	if ( m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
	{
		nCount = 1 ;
	}
	for ( i = 0; i < nCount; i ++ )
	{
		m_pDstImage[i] = CreateImageBuffer
			( m_erif.m_eriInfoHeader.fdwFormatType,
				nImageWidth, nImageHeight, nBitsPerPixel ) ;
		if ( m_pDstImage[i] == NULL )
		{
			return	errFailed ;
		}
		if ( fClipImageBuf )
		{
			m_pDstImage[i]->NormalizeFormat
				( 0, 0, nCastSizeFlags, nClipWidth, nClipHeight ) ;
		}
		m_bufDstImage[i].ptrBuffer =
				m_pDstImage[i]->LockBuffer( m_bufDstImage[i] ) ;
		m_iDstFrame[i] = -1 ;
	}
	for ( i = nCount; i < 5; i ++ )
	{
		m_bufDstImage[i].ptrBuffer = NULL ;
		m_iDstFrame[i] = -1 ;
	}
	//
	// 先読みバッファ配列を確保
	//////////////////////////////////////////////////////////////////////////
	m_nPreloadLimit = 30 ;
	m_queueImage.SetLimit( 30 ) ;
	//
	// フレームシーク用キーポイント配列確保
	//////////////////////////////////////////////////////////////////////////
	m_arrayKeyFrame.SetLimit( m_erif.m_eriFileHeader.dwFrameCount ) ;
	m_arrayKeyWave.SetLimit( m_erif.m_eriFileHeader.dwFrameCount ) ;
	//
	// 先読み準備
	//////////////////////////////////////////////////////////////////////////
	m_iCurrentFrame = 0 ;
	m_iDstBufIndex = 0 ;
	m_nCacheBFrames = 0 ;
	if ( m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
	{
		m_nCacheBFrames = -1 ;
	}
	m_iPreloadFrame = 0 ;
	m_nPreloadWaveSamples = 0 ;
	//
	KeyPoint	keypoint ;
	keypoint.m_iKeyFrame = 0 ;
	keypoint.m_nSubSample = 0 ;
	keypoint.m_nRecOffset = m_erif.GetPosition() ;
	AddKeyPoint( m_arrayKeyFrame, keypoint ) ;
	//
	PreloadBuffer *	pBuffer ;
	do
	{
		pBuffer = LoadMovieStream( m_iPreloadFrame ) ;
		if ( pBuffer == NULL )
		{
			break ;
		}
		AddPreloadBuffer( pBuffer ) ;
	}
	while ( m_queueImage.GetLength() < m_nPreloadLimit ) ;
	//
	// 第1フレームを展開する
	//////////////////////////////////////////////////////////////////////////
	if ( SeekToBegin( ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// アニメーションファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::Close( void )
{
	//
	// 先読みキューをクリアする
	//
	m_queueImage.RemoveAll( ) ;
	//
	// キーフレームポイント配列をクリアする
	//
	m_arrayKeyFrame.RemoveAll( ) ;
	m_arrayKeyWave.RemoveAll( ) ;
	//
	// 画像バッファを削除する
	//
	for ( size_t i = 0; i < bufferCount; i ++ )
	{
		if ( m_pDstImage[i] != NULL )
		{
			m_pDstImage[i]->UnlockBuffer() ;
			delete	m_pDstImage[i] ;
			m_bufDstImage[i].ptrBuffer = NULL ;
		}
		m_pDstImage[i] = NULL ;
	}
	//
	// 展開オブジェクトを削除する
	//
	m_decoderImage.Delete() ;
	m_decoderSound.Delete() ;
	//
	// 音声出力の終了
	//
	if ( m_flagWaveOutput )
	{
		CloseWaveOut( ) ;
		m_flagWaveOutput = false ;
	}
	//
	// ファイルを閉じる
	//
	m_erif.Close( ) ;
}

// 先頭フレームへ移動
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMovieFilePlayer::SeekToBegin( void )
{
	//
	// シーク
	//
	PreloadBuffer *	pBuffer ;
	SeekKeyPoint( m_arrayKeyFrame, 0, m_iPreloadFrame ) ;
	while ( m_queueImage.GetLength() < m_nPreloadLimit )
	{
		pBuffer = LoadMovieStream( m_iPreloadFrame ) ;
		if ( pBuffer == NULL )
		{
			break ;
		}
		AddPreloadBuffer( pBuffer ) ;
	}
	//
	// 先頭フレームを展開
	//
	if ( SeekToNextFrame() )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 次のフレームへ移動
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMovieFilePlayer::SeekToNextFrame( size_t nSkipFrame )
{
	//
	// スキップする分だけ読み飛ばす
	//////////////////////////////////////////////////////////////////////////
	SObjectArray<PreloadBuffer>	queSkipFrames ;
	PreloadBuffer *	pBuf ;
	size_t	i ;
	for ( i = 0; i < nSkipFrame; i ++ )
	{
		//
		// スキップするフレームを取り出す
		// ※ GetPreloadBuffer 関数は展開すべき順番に
		//    再配列してフレームを返すことに注意！
		//
		pBuf = GetPreloadBuffer() ;
		if ( pBuf != NULL )
		{
			FrameType	typeFrame = GetFrameBufferType( pBuf ) ;
			if ( (typeFrame == typeIntraFrame)
					| (typeFrame == typePredictionalFrame) )
			{
				// I, P ピクチャは一旦確保しておく
				queSkipFrames.Add( pBuf ) ;
			}
			else if ( typeFrame == typeBidirectionalFrame )
			{
				// B ピクチャは破棄しても構わない
				delete	pBuf ;
			}
			else
			{
				ApplyPaletteTable( pBuf ) ;
				delete	pBuf ;
				i -- ;
			}
		}
	}
	//
	// 展開するべきターゲットフレームを取得する
	//////////////////////////////////////////////////////////////////////////
	PreloadBuffer *	pNextFrame ;
	for ( ; ; )
	{
		pNextFrame = GetPreloadBuffer( ) ;
		if ( pNextFrame == NULL )
		{
			// ファイル終端処理
			m_iCurrentFrame = m_iDstFrame[m_iDstBufIndex]
								= m_erif.m_eriFileHeader.dwFrameCount - 1 ;
			return	errSuccess ;
		}
		if ( GetFrameBufferType( pNextFrame ) >= typeIntraFrame )
		{
			break ;
		}
		ApplyPaletteTable( pNextFrame ) ;
		delete	pNextFrame ;
	}
	//
	// I ピクチャを挟んでいる場合にはそこまでのデータを破棄する
	//////////////////////////////////////////////////////////////////////////
	if ( m_nCacheBFrames == -1 )
	{
		if ( GetFrameBufferType( pNextFrame ) == typeIntraFrame )
		{
			queSkipFrames.RemoveAll( ) ;
		}
	}
	for ( i = 0; i < queSkipFrames.GetLength(); i ++ )
	{
		pBuf = queSkipFrames.GetLastAt( i ) ;
		if ( (int) GetFrameBufferType( pBuf ) == (int) bufferIFrame )
		{
			if ( m_nCacheBFrames == -1 )
			{
				queSkipFrames.Remove( 0, i ) ;
			}
			else
			{
				if ( (i >= 1) ||
					(GetFrameBufferType( pNextFrame )
									!= typeBidirectionalFrame) )
				{
					DecodeFrame( pBuf, SGLImageDecoder::flagQuickDecode ) ;
					queSkipFrames.Remove( 0, i + 1 ) ;
				}
			}
			break ;
		}
	}
	//
	// 指定フレームまで順次展開
	//////////////////////////////////////////////////////////////////////////
	SSystem::SError	errResult = errSuccess ;
	for ( i = 0; i < queSkipFrames.GetLength(); i ++ )
	{
		pBuf = queSkipFrames.GetAt( i ) ;
		if ( pBuf != NULL )
		{
			DecodeFrame( pBuf, SGLImageDecoder::flagQuickDecode ) ;
		}
	}
	//
	uint32_t	flagsDecode = 0 ;
	if ( nSkipFrame > 0 )
	{
		flagsDecode |= SGLImageDecoder::flagQuickDecode ;
	}
	errResult = DecodeFrame( pNextFrame, flagsDecode ) ;
	//
	if ( m_nCacheBFrames == -1 )
	{
		m_iCurrentFrame = m_iDstFrame[m_iDstBufIndex] ;
	}
	else if ( GetFrameBufferType( pNextFrame ) == typeBidirectionalFrame )
	{
		m_iCurrentFrame = m_iDstFrame[bufferBFrame] ;
	}
	else
	{
		m_iCurrentFrame = m_iDstFrame[(m_iDstBufIndex ^ 0x01)] ;
	}
	delete	pNextFrame ;

	return	errSuccess ;
}

// 指定のフレームに移動
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMovieFilePlayer::SeekToFrame( uint64_t iFrameIndex )
{
	//
	// 特殊条件
	//////////////////////////////////////////////////////////////////////////
	if ( m_iCurrentFrame == iFrameIndex )
	{
		return	errSuccess ;
	}
	if ( m_iCurrentFrame <= iFrameIndex )
	{
		uint32_t	nKeyFrame = GetKeyFrameCount( ) ;
		if ( nKeyFrame == 0 )
		{
			return	SeekToNextFrame
						( (size_t) (iFrameIndex - m_iCurrentFrame - 1) ) ;
		}
		if ( (iFrameIndex / nKeyFrame) == (m_iCurrentFrame / nKeyFrame) )
		{
			return	SeekToNextFrame
						( (size_t) (iFrameIndex - m_iCurrentFrame - 1) ) ;
		}
	}
	//
	// ストリーミング停止
	//////////////////////////////////////////////////////////////////////////
	if ( m_flagWaveStreaming )
	{
		EndWaveStreaming( ) ;
		m_flagWaveStreaming = false ;
	}
	//
	// シーク
	//////////////////////////////////////////////////////////////////////////
	uint64_t	iKeyFrame = 0 ;
	if ( GetKeyFrameCount() > 0 )
	{
		iKeyFrame = iFrameIndex / GetKeyFrameCount() ;
		iKeyFrame *= GetKeyFrameCount() ;
	}
	SeekKeyPoint( m_arrayKeyFrame, iKeyFrame, m_iPreloadFrame ) ;
	//
	// キーフレームを展開
	//////////////////////////////////////////////////////////////////////////
	if ( SeekToNextFrame( ) )
	{
		return	errFailed ;
	}
	//
	// 差分フレームを展開
	//////////////////////////////////////////////////////////////////////////
	if ( iFrameIndex > iKeyFrame )
	{
		if ( SeekToNextFrame( (size_t) (iFrameIndex - iKeyFrame - 1) ) )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// 指定のフレームはキーフレームか？
//////////////////////////////////////////////////////////////////////////////
bool SGLMovieFilePlayer::IsKeyFrame( uint64_t iFrameIndex )
{
	if ( m_erif.m_eriFileHeader.dwKeyFrameCount == 1 )
	{
		return	true ;
	}
	if ( iFrameIndex == 0 )
	{
		return	true ;
	}
	if ( m_erif.m_eriFileHeader.dwKeyFrameCount == 0 )
	{
		return	false ;
	}
	return	(iFrameIndex % m_erif.m_eriFileHeader.dwKeyFrameCount) == 0 ;
}

// 最適なフレームスキップ数を取得する
//////////////////////////////////////////////////////////////////////////////
size_t SGLMovieFilePlayer::GetBestSkipFrames( uint64_t nCurrentTime )
{
	uint64_t	iFrameIndex = TimeToFrameIndex( nCurrentTime ) ;
	if ( iFrameIndex <= m_iCurrentFrame )
	{
		return	0 ;
	}
	ssize_t	i = (ssize_t) (iFrameIndex - m_iCurrentFrame) ;
	ssize_t	nDefSkipCount = 0 ;
	if ( m_nCacheBFrames != -1 )
	{
		//
		// B フレームを考慮する
		//
		nDefSkipCount = (ssize_t) i ;
		if ( nDefSkipCount == m_nCacheBFrames )
		{
			return	m_nCacheBFrames ;
		}
		if ( nDefSkipCount < m_nCacheBFrames )
		{
			return	nDefSkipCount - 1 ;
		}
		nDefSkipCount = m_nCacheBFrames ;
	}
/*
	//
	// I フレームを検索する
	//
	if ( i > (ssize_t) m_queueImage.GetLength() )
	{
		i = m_queueImage.GetLength() ;
	}
	while ( (-- i) >= 0 )
	{
		PreloadBuffer *	pPreloaded = m_queueImage.GetAt( i ) ;
		if ( pPreloaded == NULL )
		{
			continue ;
		}
		if ( SChunkFile::IsEqualChunkID
				( pPreloaded->m_ui64ChunkType, "ImageFrm" ) )
		{
			int	iSkipFrames = 0 ;
			while ( (-- i) >= 0 )
			{
				pPreloaded = m_queueImage.GetAt( i ) ;
				if ( pPreloaded == NULL )
				{
					continue ;
				}
				UINT64	recid = pPreloaded->m_ui64ChunkType ;
				if ( SChunkFile::IsEqualChunkID( recid, "ImageFrm" )
					|| SChunkFile::IsEqualChunkID( recid, "DiffeFrm" ) )
				{
					iSkipFrames ++ ;
				}
			}
			return	iSkipFrames ;
		}
	}
*/
	return	nDefSkipCount ;
}

// 画像展開出力バッファ要求
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageObject * SGLMovieFilePlayer::CreateImageBuffer
	( uint32_t format, uint32_t width, uint32_t height, uint32_t bpp )
{
	SGLImageInfo	imginf ;
	imginf.format = format ;
	imginf.width = width ;
	imginf.height = height ;
	imginf.depth = bpp ;
	if ( bpp == 24 )
	{
		imginf.depth = 32 ;
	}
	imginf.pitchPixel = imginf.depth >> 3 ;
	imginf.pitchLine = imginf.width * imginf.pitchPixel ;
	//
	SGLSmartImage *	pImage = new SGLSmartImage ;
	pImage->CreateBuffer
		( imginf, SGLImageObject::bufferOnMemory
					| SGLImageObject::bufferNonPowerOf2
					| SGLImageObject::bufferForTexture ) ;
	return	pImage ;
}

// 音声出力要求
//////////////////////////////////////////////////////////////////////////////
bool SGLMovieFilePlayer::RequestWaveOut
	( uint32_t channels, uint32_t frequency, uint32_t bps )
{
	return	false ;
}

// 音声出力終了
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::CloseWaveOut( void )
{
}

// 音声データ出力
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::PushWaveBuffer( const void * ptrWaveBuf, size_t nBytes )
{
}

// 音声ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::BeginWaveStreaming( void )
{
	if ( m_flagWaveOutput )
	{
		m_flagWaveStreaming = true ;
		SeekKeyWave( m_arrayKeyWave, m_iCurrentFrame ) ;
	}
}

// 音声ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::EndWaveStreaming( void )
{
	m_flagWaveStreaming = false ;
}

// フレームを展開する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMovieFilePlayer::DecodeFrame
	( SGLMovieFilePlayer::PreloadBuffer * pFrame, uint32_t flagsDecode )
{
	SSystem::SError	errResult = errSuccess ;
	FrameType	typeFrame = GetFrameBufferType( pFrame ) ;
	if ( typeFrame >= typeIntraFrame )
	{
		//
		// 差分フレームの判定
		//
		size_t		iDstIndex ;
		uint32_t	flagsDecMask = 0xFFFFFFFF ;
		if ( m_flagTopDown )
		{
			flagsDecode |= SGLImageDecoder::flagTopDown ;
		}
		if ( m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
		{
			if ( typeFrame == typePredictionalFrame )
			{
				flagsDecode |= SGLImageDecoder::flagDifferential ;
			}
			else
			{
				flagsDecode &= ~SGLImageDecoder::flagDifferential ;
				m_flagsDecode &= ~SGLImageDecoder::flagDifferential ;
				flagsDecMask = ~SGLImageDecoder::flagQuickDecode ;
			}
			ESLAssert( m_iDstBufIndex == bufferIFrame ) ;
			iDstIndex = m_iDstBufIndex ;
		}
		else if ( typeFrame == typeBidirectionalFrame )
		{
			const size_t	iPrevBuf = m_iDstBufIndex ^ 0x01 ;
			flagsDecode |= SGLImageDecoder::flagDifferential ;
			m_pDstImage[iPrevBuf]->
					GetImageInfo( m_bufDstImage[iPrevBuf] ) ;
			m_decoderImage.SetRefPreviousFrame
				( &m_bufDstImage[iPrevBuf],
					m_bufDstImage[iPrevBuf].ptrBuffer,
					&m_bufDstImage[m_iDstBufIndex],
					m_bufDstImage[m_iDstBufIndex].ptrBuffer ) ;
			iDstIndex = bufferBFrame ;
		}
		else
		{
			if ( typeFrame == typePredictionalFrame )
			{
				flagsDecode |= SGLImageDecoder::flagDifferential ;
				m_pDstImage[m_iDstBufIndex]->
						GetImageInfo( m_bufDstImage[m_iDstBufIndex] ) ;
				m_decoderImage.SetRefPreviousFrame
					( &m_bufDstImage[m_iDstBufIndex],
						m_bufDstImage[m_iDstBufIndex].ptrBuffer ) ;
			}
			else
			{
				flagsDecode &= ~SGLImageDecoder::flagDifferential ;
				m_flagsDecode &= ~SGLImageDecoder::flagDifferential ;
				flagsDecMask = ~SGLImageDecoder::flagQuickDecode ;
			}
			m_iDstBufIndex ^= 0x01 ;
			iDstIndex = m_iDstBufIndex ;
		}
		//
		// 展開実行
		//
		size_t	iDstFiltered = iDstIndex ;
		if ( iDstIndex != bufferBFrame )
		{
			iDstFiltered = bufferFilter0 + iDstIndex ;
		}
		flagsDecode |= m_flagsDecode ;
		m_flagsDecode &= flagsDecMask ;
		//
		pFrame->Seek( 0 ) ;
		m_bstream->AttachInputStream( pFrame ) ;
		//
		if ( m_pDstImage[iDstFiltered] != NULL )
		{
			m_pDstImage[iDstFiltered]->
					GetImageInfo( m_bufDstImage[iDstFiltered] ) ;
			m_decoderImage.SetFilteredImageBuffer
				( &m_bufDstImage[iDstFiltered],
					m_bufDstImage[iDstFiltered].ptrBuffer ) ;
		}
		m_pDstImage[iDstIndex]->
				GetImageInfo( m_bufDstImage[iDstIndex] ) ;
		errResult = m_decoderImage.DecodeImage
			( m_bufDstImage[iDstIndex],
				m_bufDstImage[iDstIndex].ptrBuffer,
								*m_bstream, flagsDecode ) ;
		//
		if ( m_decoderImage.GetFilteredImageBuffer() != NULL )
		{
			m_iDstFrame[iDstFiltered] = pFrame->m_iFrameIndex ;
			m_pDstImage[iDstFiltered]->FlushBuffer() ;
		}
		else
		{
			m_pDstImage[iDstIndex]->FlushBuffer() ;
		}
		m_iDstFrame[iDstIndex] = pFrame->m_iFrameIndex ;
	}
	else
	{
		ApplyPaletteTable( pFrame ) ;
	}
	return	errResult ;
}

// パレットテーブルを適用する
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::ApplyPaletteTable
		( SGLMovieFilePlayer::PreloadBuffer * pBuffer )
{
	if ( pBuffer == NULL )
	{
		return ;
	}
	if ( SChunkFile::IsEqualChunkID( pBuffer->m_ui64ChunkType, "Palette " ) )
	{
		size_t	nBytes = (size_t) pBuffer->GetLength() ;
		if ( nBytes > sizeof(SGLPalette) * 0x100 )
		{
			nBytes = sizeof(SGLPalette) * 0x100 ;
		}
		m_erif.m_tablePalette.SetLength( nBytes / sizeof(SGLPalette) ) ;
		pBuffer->Read( m_erif.m_tablePalette.GetArray(), nBytes ) ;
		m_erif.m_tablePalette.FinishArray() ;
		//
		for ( size_t i = 0; i < bufferCount; i ++ )
		{
			if ( m_pDstImage[i] != NULL )
			{
				m_pDstImage[i]->SetPaletteTable
					( m_erif.m_tablePalette.GetConstArray(),
								nBytes / sizeof(SGLPalette) ) ;
			}
		}
	}
}

// 先読みバッファを取得する
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::PreloadBuffer *
		SGLMovieFilePlayer::GetPreloadBuffer( void )
{
	PreloadBuffer *	pBuffer = NULL ;
	//
	// 先読みバッファにフレームを読み込む
	//
	if ( m_queueImage.GetLength() < m_nPreloadLimit )
	{
		PreloadBuffer *	pPreload = LoadMovieStream( m_iPreloadFrame ) ;
		if ( pPreload != NULL )
		{
			AddPreloadBuffer( pPreload ) ;
		}
	}
	if ( m_nCacheBFrames != 0 )
	{
		//
		// B フレームキャッシュがある場合には
		// （又はその必要がない場合には）
		// 素直に直後の先読みデータを返す
		//
		if ( m_queueImage.GetLength() != 0 )
		{
			pBuffer = m_queueImage.GetAt( 0 ) ;
			m_queueImage.DetachAt( 0 ) ;
			if ( m_nCacheBFrames != -1 )
			{
				m_nCacheBFrames -- ;
			}
		}
	}
	else
	{
		//
		// B フレームキャッシュがない場合には
		// （B フレームをキャッシュする必要がある場合には）
		// 次の I, 又は P フレームを返す
		//
		for ( ; ; )
		{
			//
			// B フレーム以外を探す
			//
			size_t	i ;
			for ( i = 0; i < m_queueImage.GetLength(); i ++ )
			{
				FrameType	typeFrame =
					GetFrameBufferType( m_queueImage.GetAt( i ) ) ;
				if ( typeFrame != typeBidirectionalFrame )
				{
					pBuffer = m_queueImage.GetAt( i ) ;
					break ;
				}
			}
			if ( pBuffer != NULL )
			{
				ESLAssert( pBuffer == m_queueImage.GetAt( i ) ) ;
				m_queueImage.DetachAt( (int) i ) ;
				m_nCacheBFrames = (ssize_t) i ;
				break ;
			}
			//
			// B フレームしか見つからなかった場合には
			// 先読みバッファに見つかるまで読み込む
			//
			PreloadBuffer *	pPreload = LoadMovieStream( m_iPreloadFrame ) ;
			if ( pPreload != NULL )
			{
				AddPreloadBuffer( pPreload ) ;
			}
			else
			{
				break ;
			}
		}
	}
	//
	// バッファを先読みする
	//
	if ( m_queueImage.GetLength() < m_nPreloadLimit )
	{
		PreloadBuffer *	pPreload = LoadMovieStream( m_iPreloadFrame ) ;
		if ( pPreload != NULL )
		{
			AddPreloadBuffer( pPreload ) ;
		}
	}
	return	pBuffer ;
}

// 先読みバッファに追加する
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::AddPreloadBuffer
			( SGLMovieFilePlayer::PreloadBuffer * pBuffer )
{
	m_queueImage.Add( pBuffer ) ;
}

// 指定のフレームが I, P, B ピクチャか判定する
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::FrameType
	SGLMovieFilePlayer::GetFrameBufferType
			( SGLMovieFilePlayer::PreloadBuffer * pBuffer )
{
	if ( pBuffer == NULL )
	{
		return	typeOther ;
	}
	uint64_t	recid = pBuffer->m_ui64ChunkType ;
	if ( SChunkFile::IsEqualChunkID( recid, "ImageFrm" ) )
	{
		return	typeIntraFrame ;					// I ピクチャ
	}
	if ( SChunkFile::IsEqualChunkID( recid, "DiffeFrm" ) )
	{
		if ( m_erif.m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
		{
			return	typePredictionalFrame ;		// P ピクチャ
		}
		if ( (pBuffer->GetLength() >= 2)
			&& ((pBuffer->At(1) & 0x02) != 0) )
		{
			return	typeBidirectionalFrame ;		// B ピクチャ
		}
		return	typePredictionalFrame ;
	}
	return	typeOther ;						// その他（パレット等）
}

// SGLMediaFile オブジェクトを取得する
//////////////////////////////////////////////////////////////////////////////
const SGLMediaFile & SGLMovieFilePlayer::GetMediaFile( void ) const
{
	return	m_erif ;
}

// カレントフレームのインデックスを取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMovieFilePlayer::CurrentIndex( void ) const
{
	return	m_iCurrentFrame ;
}

// カレントフレームの画像を取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageObject * SGLMovieFilePlayer::CurrentFrame( void ) const
{
	size_t	 i = bufferCount - 1 ;
	do
	{
		if ( m_iDstFrame[i] == (int64_t) m_iCurrentFrame )
		{
			return	m_pDstImage[i] ;
		}
	}
	while ( -- i ) ;
	return	m_pDstImage[i] ;
}

// パレットテーブル取得
//////////////////////////////////////////////////////////////////////////////
const SakuraGL::SGLPalette * SGLMovieFilePlayer::GetPaletteEntries( void ) const
{
	if ( m_pDstImage[0] == NULL )
	{
		return	NULL ;
	}
	if ( m_erif.m_tablePalette.GetLength() < 0x100 )
	{
		return	NULL ;
	}
	return	m_erif.m_tablePalette.GetConstArray() ;
}

// キーフレームを取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLMovieFilePlayer::GetKeyFrameCount( void ) const
{
	return	m_erif.m_eriFileHeader.dwKeyFrameCount ;
}

// 全フレーム数を取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMovieFilePlayer::GetAllFrameCount( void ) const
{
	return	m_erif.m_eriFileHeader.dwFrameCount ;
}

// 全アニメーション時間を取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMovieFilePlayer::GetTotalTime( void ) const
{
	return	m_erif.m_eriFileHeader.dwAllFrameTime ;
}

// フレーム番号から時間へ変換
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMovieFilePlayer::FrameIndexToTime( uint64_t iFrameIndex ) const
{
	if ( m_erif.m_eriFileHeader.dwFrameCount == 0 )
	{
		return	0 ;
	}
	return	iFrameIndex
				* m_erif.m_eriFileHeader.dwAllFrameTime
				/ m_erif.m_eriFileHeader.dwFrameCount ;
}

// 時間からフレーム番号へ変換
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMovieFilePlayer::TimeToFrameIndex( uint64_t nMilliSec ) const
{
	if ( m_erif.m_eriFileHeader.dwAllFrameTime == 0 )
	{
		return	0 ;
	}
	return	nMilliSec
				* m_erif.m_eriFileHeader.dwFrameCount
				/ m_erif.m_eriFileHeader.dwAllFrameTime ;
}

// 動画像ストリームを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::PreloadBuffer *
	SGLMovieFilePlayer::LoadMovieStream( uint64_t & iCurrentFrame )
{
	KeyPoint	keypoint ;
	for ( size_t i = 0; i < 0x400; i ++ )
	{
		//
		// レコードを開く
		//////////////////////////////////////////////////////////////////////
		uint64_t	nRecPosition = m_erif.GetPosition() ;
		if ( m_erif.DescendChunk() )
		{
			// 1つも画像レコードが無い場合はエラー
			return	NULL ;
			/*
			//
			// レコードの終端に到達したら
			// 自動的に先頭に移動
			iCurrentFrame = 0 ;
			m_nPreloadWaveSamples = 0 ;
			m_erif.Seek( 0, ESLFileObject::FromBegin ) ;
			continue ;
			*/
		}
		//
		// レコードの種類を判別
		//////////////////////////////////////////////////////////////////////
		PreloadBuffer *	pBuffer = NULL ;
		if ( m_erif.IsEqualCurrentChunkID( "ImageFrm" )
			|| m_erif.IsEqualCurrentChunkID( "DiffeFrm" ) )
		{
			//
			// 画像データレコード
			//
			size_t	nDataBytes = (size_t) m_erif.GetCurrentChunkLength( ) ;
			pBuffer = new PreloadBuffer( nDataBytes ) ;
			pBuffer->m_iFrameIndex = iCurrentFrame ;
			pBuffer->m_ui64ChunkType = m_erif.GetCurrentChunkID() ;
			pBuffer->ReadFromFile( m_erif ) ;
			m_erif.AscendChunk( ) ;
			//
			++ iCurrentFrame ;
			if ( IsKeyFrame( iCurrentFrame ) )
			{
				KeyPoint *	pKeyPoint =
					SearchKeyPoint( m_arrayKeyFrame, iCurrentFrame ) ;
				if ( pKeyPoint == NULL )
				{
					keypoint.m_iKeyFrame = iCurrentFrame ;
					keypoint.m_nSubSample = m_nPreloadWaveSamples ;
					keypoint.m_nRecOffset = m_erif.GetPosition( ) ;
					AddKeyPoint( m_arrayKeyFrame, keypoint ) ;
				}
			}
			return	pBuffer ;
		}
		else if ( m_erif.IsEqualCurrentChunkID( "Palette " ) )
		{
			//
			// パレットテーブルレコード
			//
			size_t	nDataBytes = (size_t) m_erif.GetCurrentChunkLength( ) ;
			pBuffer = new PreloadBuffer( nDataBytes ) ;
			pBuffer->m_iFrameIndex = iCurrentFrame ;
			pBuffer->m_ui64ChunkType = m_erif.GetCurrentChunkID() ;
			pBuffer->ReadFromFile( m_erif ) ;
			m_erif.AscendChunk( ) ;
			return	pBuffer ;
		}
		else if ( m_erif.IsEqualCurrentChunkID( "SoundStm" ) )
		{
			//
			// 音声ストリームレコード
			//
			ERISA::MIO_DATA_HEADER	miodh ;
			m_erif.Read( &miodh, sizeof(ERISA::MIO_DATA_HEADER) ) ;
			if ( miodh.bytFlags & mioDataLeadBlock )
			{
				KeyPoint *	pKeyPoint = SearchKeyPoint
					( m_arrayKeyWave, m_nPreloadWaveSamples ) ;
				if ( pKeyPoint == NULL )
				{
					keypoint.m_iKeyFrame = m_nPreloadWaveSamples ;
					keypoint.m_nSubSample = iCurrentFrame ;
					keypoint.m_nRecOffset = nRecPosition ;
					AddKeyPoint( m_arrayKeyWave, keypoint ) ;
				}
			}
			m_nPreloadWaveSamples += miodh.dwSampleCount ;
			//
			if ( m_flagWaveStreaming & m_flagWaveOutput )
			{
				//
				// 音声ストリーミングモード
				//		→ デコードして出力
				//
				SArray<uint8_t>	bufWave ;
				size_t	nBytes = miodh.dwSampleCount
							* m_erif.m_mioInfoHeader.dwChannelCount
							* (m_erif.m_mioInfoHeader.dwBitsPerSample / 8) ;
				bufWave.SetLength( nBytes ) ;
				//
				m_bstream->AttachInputStream( &m_erif ) ;
				if ( m_decoderSound.DecodeSound
					( *m_bstream, miodh, bufWave.GetArray() ) == errSuccess )
				{
					PushWaveBuffer( bufWave.GetConstArray(), nBytes ) ;
				}
				bufWave.FinishArray() ;
			}
			m_erif.AscendChunk( ) ;
		}
		else
		{
			//
			// レコードを閉じる
			//
			m_erif.AscendChunk( ) ;
		}
	}
	return	NULL ;
}

// キーフレームポイントを追加する
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::AddKeyPoint
	( SSystem::SArray<SGLMovieFilePlayer::KeyPoint> & arrayKeyPoint,
							const SGLMovieFilePlayer::KeyPoint & key )
{
	arrayKeyPoint.Add( key ) ;
}

// 指定のキーフレームを検索する
//////////////////////////////////////////////////////////////////////////////
SGLMovieFilePlayer::KeyPoint *
	SGLMovieFilePlayer::SearchKeyPoint
		( SSystem::SArray<SGLMovieFilePlayer::KeyPoint> & arrayKeyPoint, uint64_t iKeyFrame )
{
	ssize_t		iFirst, iMiddle, iEnd ;
	KeyPoint *	pFoundKey ;
	//
	iFirst = 0 ;
	iEnd = (ssize_t) arrayKeyPoint.GetLength() - 1 ;
	//
	for ( ; ; )
	{
		if ( iFirst > iEnd )
		{
			pFoundKey = NULL ;
			break ;
		}
		//
		iMiddle = (iFirst + iEnd) / 2 ;
		pFoundKey = arrayKeyPoint.GetAt( iMiddle ) ;
		//
		if ( pFoundKey->m_iKeyFrame == iKeyFrame )
		{
			break ;
		}
		if ( pFoundKey->m_iKeyFrame > iKeyFrame )
		{
			iEnd = iMiddle - 1 ;
		}
		else
		{
			iFirst = iMiddle + 1 ;
		}
	}
	return	pFoundKey ;
}

// 指定のフレームにシークする
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::SeekKeyPoint
	( SSystem::SArray<SGLMovieFilePlayer::KeyPoint> & arrayKeyPoint,
						uint64_t iFrame, uint64_t & iCurrentFrame )
{
	//
	// 先読みキューに読み込まれているか判断
	//
	bool			fHaveSeeked = false ;
	PreloadBuffer *	pBuffer ;
	for ( ; ; )
	{
		if ( m_queueImage.GetLength() == 0 )
		{
			break ;
		}
		pBuffer = m_queueImage.GetAt( 0 ) ;
		if ( (pBuffer != NULL)
				&& (pBuffer->m_iFrameIndex == iFrame) )
		{
			if ( GetFrameBufferType( pBuffer ) == typeIntraFrame )
			{
				fHaveSeeked = true ;
				break ;
			}
			else
			{
				ApplyPaletteTable( pBuffer ) ;
			}
		}
		m_queueImage.RemoveAt( 0 ) ;
	}
	if ( !fHaveSeeked )
	{
		//
		// リストに指定フレームのポインタが
		// 登録されているかどうかを調べる
		//
		KeyPoint *	pKeyPoint = SearchKeyPoint( m_arrayKeyFrame, iFrame ) ;
		//
		if ( pKeyPoint != NULL )
		{
			//
			// ポインタにシーク
			//
			m_erif.Seek( pKeyPoint->m_nRecOffset ) ;
			iCurrentFrame = pKeyPoint->m_iKeyFrame ;
			m_nPreloadWaveSamples = pKeyPoint->m_nSubSample ;
		}
		else
		{
			//
			// 指定のフレームを探す
			//
			do
			{
				//
				// 次のレコードを開く
				//
				uint64_t	nRecPosition = m_erif.GetPosition( ) ;
				if ( m_erif.DescendChunk( ) )
				{
					iCurrentFrame = 0 ;
					m_erif.Seek( 0 ) ;
					break ;
				}
				//
				// レコードの種類を判別
				//
				if ( m_erif.IsEqualCurrentChunkID( "ImageFrm" )
					|| m_erif.IsEqualCurrentChunkID( "DiffeFrm" ) )
				{
					//
					// 画像データレコード
					//
					m_erif.AscendChunk( ) ;
					++ iCurrentFrame ;
					if ( IsKeyFrame( iCurrentFrame ) )
					{
						KeyPoint *	pKeyPoint =
							SearchKeyPoint( m_arrayKeyFrame, iCurrentFrame ) ;
						if ( pKeyPoint == NULL )
						{
							KeyPoint	keypoint ;
							keypoint.m_iKeyFrame = iCurrentFrame ;
							keypoint.m_nSubSample = m_nPreloadWaveSamples ;
							keypoint.m_nRecOffset = m_erif.GetPosition( ) ;
							AddKeyPoint( m_arrayKeyFrame, keypoint ) ;
						}
					}
					if ( iCurrentFrame == iFrame )
					{
						fHaveSeeked = true ;
					}
				}
				else if ( m_erif.IsEqualCurrentChunkID( "SoundStm" ) )
				{
					//
					// 音声ストリームレコード
					//
					ERISA::MIO_DATA_HEADER	miodh ;
					m_erif.Read( &miodh, sizeof(ERISA::MIO_DATA_HEADER) ) ;
					if ( miodh.bytFlags & mioDataLeadBlock )
					{
						KeyPoint *	pKeyPoint =
							SearchKeyPoint
								( m_arrayKeyWave, m_nPreloadWaveSamples ) ;
						if ( pKeyPoint == NULL )
						{
							KeyPoint	keypoint ;
							keypoint.m_iKeyFrame = m_nPreloadWaveSamples ;
							keypoint.m_nSubSample = iCurrentFrame ;
							keypoint.m_nRecOffset = nRecPosition ;
							AddKeyPoint( m_arrayKeyWave, keypoint ) ;
						}
					}
					m_nPreloadWaveSamples += miodh.dwSampleCount ;
					m_erif.AscendChunk( ) ;
				}
				else
				{
					m_erif.AscendChunk( ) ;
				}
			}
			while ( !fHaveSeeked ) ;
		}
	}
	//
	// B ピクチャに対応したフォーマットでは
	// 先頭の I ピクチャはあらかじめ展開しておく
	//
	if ( m_nCacheBFrames != -1 )
	{
		m_nCacheBFrames = 0 ;
		pBuffer = GetPreloadBuffer( ) ;
		DecodeFrame( pBuffer ) ;
		delete	pBuffer ;
		ESLAssert( m_nCacheBFrames == 0 ) ;
	}
}

// 指定の音声データまでシークしてストリーミング出力する
//////////////////////////////////////////////////////////////////////////////
void SGLMovieFilePlayer::SeekKeyWave
	( SSystem::SArray<SGLMovieFilePlayer::KeyPoint> & arrayKeyPoint, uint64_t iFrame )
{
	if ( !m_flagWaveOutput | !m_flagWaveStreaming )
	{
		return ;
	}
	//
	// フレーム番号を音声サンプル数へ変換
	//
	uint64_t	nSeekTime = FrameIndexToTime( iFrame ) ;
	uint64_t	nWaveSamples =
					nSeekTime * m_erif.m_mioInfoHeader.dwSamplesPerSec / 1000 ;
	//
	// シークポイントより手間で、最も近いキーポイントを検索する
	//
	ssize_t		iFirst, iMiddle, iEnd ;
	KeyPoint *	pKeyPoint ;
	//
	iFirst = 0 ;
	iEnd = (ssize_t) arrayKeyPoint.GetLength() - 1 ;
	//
	for ( ; ; )
	{
		if ( iFirst > iEnd )
		{
			pKeyPoint = arrayKeyPoint.GetAt( iEnd ) ;
			break ;
		}
		//
		iMiddle = (iFirst + iEnd) / 2 ;
		pKeyPoint = arrayKeyPoint.GetAt( iMiddle ) ;
		//
		if ( pKeyPoint->m_iKeyFrame == nWaveSamples )
		{
			break ;
		}
		if ( pKeyPoint->m_iKeyFrame > nWaveSamples )
		{
			iEnd = iMiddle - 1 ;
		}
		else
		{
			iFirst = iMiddle + 1 ;
		}
	}
	//
	// ファイルポインタを移動
	//
	int64_t	nOriginalPointer, iCurrentFrame ;
	nOriginalPointer = m_erif.GetPosition( ) ;
	m_nPreloadWaveSamples = pKeyPoint->m_iKeyFrame ;
	iCurrentFrame = pKeyPoint->m_nSubSample ;
	//
	m_erif.Seek( pKeyPoint->m_nRecOffset ) ;
	//
	// 指定の場所を探す
	//
	do
	{
		//
		// 次のレコードを開く
		//
		uint64_t	nRecPosition = m_erif.GetPosition( ) ;
		if ( m_erif.DescendChunk() )
		{
			m_erif.Seek( 0 ) ;
			break ;
		}
		//
		// レコードの種類を判別
		//
		if ( m_erif.IsEqualCurrentChunkID( "ImageFrm" )
			|| m_erif.IsEqualCurrentChunkID( "DiffeFrm" ) )
		{
			//
			// 画像データレコード
			//
			m_erif.AscendChunk( ) ;
			++ iCurrentFrame ;
			if ( IsKeyFrame( iCurrentFrame ) )
			{
				KeyPoint *	pKeyPoint =
					SearchKeyPoint( m_arrayKeyFrame, iCurrentFrame ) ;
				if ( pKeyPoint == NULL )
				{
					KeyPoint	keypoint ;
					keypoint.m_iKeyFrame = iCurrentFrame ;
					keypoint.m_nSubSample = m_nPreloadWaveSamples ;
					keypoint.m_nRecOffset = m_erif.GetPosition( ) ;
					AddKeyPoint( m_arrayKeyFrame, keypoint ) ;
				}
			}
		}
		else if ( m_erif.IsEqualCurrentChunkID( "SoundStm" ) )
		{
			//
			// 音声ストリームレコード
			//
			ERISA::MIO_DATA_HEADER	miodh ;
			m_erif.Read( &miodh, sizeof(MIO_DATA_HEADER) ) ;
			if ( miodh.bytFlags & mioDataLeadBlock )
			{
				KeyPoint *	pKeyPoint =
					SearchKeyPoint
						( m_arrayKeyWave, m_nPreloadWaveSamples ) ;
				if ( pKeyPoint == NULL )
				{
					KeyPoint	keypoint ;
					keypoint.m_iKeyFrame = iCurrentFrame ;
					keypoint.m_nSubSample = m_nPreloadWaveSamples ;
					keypoint.m_nRecOffset = nRecPosition ;
					AddKeyPoint( m_arrayKeyWave, keypoint ) ;
				}
			}
			//
			// 音声ストリーミングモード
			//		→ デコードして出力
			//
			SArray<uint8_t>	bufWave ;
			size_t	nBytes = miodh.dwSampleCount
						* m_erif.m_mioInfoHeader.dwChannelCount
						* (m_erif.m_mioInfoHeader.dwBitsPerSample / 8) ;
			bufWave.SetLength( nBytes ) ;
			//
			m_bstream->AttachInputStream( &m_erif ) ;
			if ( m_decoderSound.DecodeSound
				( *m_bstream, miodh, bufWave.GetArray() ) == errSuccess )
			{
				if ( m_nPreloadWaveSamples
							+ miodh.dwSampleCount > nWaveSamples )
				{
					int64_t	nOffsetSamples
								= nWaveSamples - m_nPreloadWaveSamples ;
					if ( nOffsetSamples <= 0 )
					{
						PushWaveBuffer( bufWave.GetConstArray(), nBytes ) ;
					}
					else
					{
						size_t	nBlockAlign =
							m_erif.m_mioInfoHeader.dwChannelCount
								* m_erif.m_mioInfoHeader.dwBitsPerSample / 8 ;
						size_t	nOffsetBytes =
									(size_t) nOffsetSamples * nBlockAlign ;
						size_t	nSubBytes = nBytes - nOffsetBytes ;
						//
						PushWaveBuffer
							( bufWave.GetConstArray() + nOffsetBytes, nSubBytes ) ;
					}
				}
			}
			bufWave.FinishArray() ;
			//
			m_nPreloadWaveSamples += miodh.dwSampleCount ;
			//
			m_erif.AscendChunk( ) ;
		}
		else
		{
			m_erif.AscendChunk( ) ;
		}
	}
	while ( m_erif.GetPosition() < nOriginalPointer ) ;
}

