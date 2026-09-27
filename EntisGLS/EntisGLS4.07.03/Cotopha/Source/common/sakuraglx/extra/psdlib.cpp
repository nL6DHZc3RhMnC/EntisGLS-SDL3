
/*****************************************************************************
                  Adobe(R) Photoshop(R) ファイルローダー
 *****************************************************************************/

#include <sakuragl/sakuragl.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakuraglx/extra/psdlib.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace PSD ;


//////////////////////////////////////////////////////////////////////////////
// PSD ファイル・リーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( PSD::FileReader, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
FileReader::FileReader( void )
{
	m_pfile = NULL ;
	m_flagFileOwner = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
FileReader::~FileReader( void )
{
	FileReader::Close( ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError FileReader::Open( SFileInterface * pfile, bool flagOwnFile )
{
	//
	// 以前のファイルを破棄
	//
	Close( ) ;
	//
	// ファイルヘッダを読み込む
	//
	m_pfile = pfile ;
	m_flagFileOwner = flagOwnFile ;
	//
	if ( pfile->Read( &m_fhHeader, sizeof(m_fhHeader) ) < sizeof(m_fhHeader) )
	{
		return	errFailed ;
	}
	if ( PSD_CHUNK_SIGNATURE('8','B','P','S')
						!= m_fhHeader.dwSignature.GetUInt32() )
	{
		return	errFailed ;
	}
	//
	// Color Mode Data を読み込む
	//
	DWORD_BE	dwLength ;
	if ( pfile->Read( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	if ( dwLength.GetUInt32() > 0 )
	{
		m_bufColorMode.SetLength( dwLength.GetUInt32() ) ;
		pfile->Read( m_bufColorMode.GetArray(), m_bufColorMode.GetLength() ) ;
		m_bufColorMode.FinishArray() ;
	}
	//
	// Image Resource Section を読み飛ばす
	//
	if ( pfile->Read( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	m_dwLayerDataPos =
		(uint32_t) pfile->GetPosition() + dwLength.GetUInt32() ;
	if ( pfile->Seek( m_dwLayerDataPos ) != m_dwLayerDataPos )
	{
		return	errFailed ;
	}
	//
	// Layer and Mask Information Section を読み込む
	//
	if ( pfile->Read( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	m_dwBaseDataPos =
		(uint32_t) pfile->GetPosition() + dwLength.GetUInt32() ;
	if ( dwLength.GetUInt32() != 0 )
	{
		if ( pfile->Read( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
		{
			return	errFailed ;
		}
		int16_be_t	wLayerCount ;
		if ( pfile->Read
			( &wLayerCount, sizeof(int16_be_t) ) < sizeof(int16_be_t) )
		{
			return	errFailed ;
		}
		int	nLayerCount = wLayerCount.GetInt16() ;
		if ( nLayerCount <= 0 )
		{
			nLayerCount = - nLayerCount ;
		}
		for ( int i = 0; i < nLayerCount; i ++ )
		{
			LayerRecord *	plrLayer = new LayerRecord ;
			m_lstLayer.Add( plrLayer ) ;
			//
			// レイヤー領域（矩形）、及びチャネル数
			//
			if ( pfile->Read
				( plrLayer, sizeof(DWORD_BE) * 4 + sizeof(WORD_BE) )
								< sizeof(DWORD_BE) * 4 + sizeof(WORD_BE) )
			{
				return	errFailed ;
			}
			//
			// チャネル長情報、及び残りのデータを読み込む
			//
			if ( plrLayer->wChannels.GetUInt16() > 24 )
			{
				return	errFailed ;
			}
			size_t	nDataLen ;
			nDataLen = sizeof(CHANNEL_LENGTH_INFO)
							* plrLayer->wChannels.GetUInt16() ;
			if ( pfile->Read( &(plrLayer->chlen[0]), nDataLen ) < nDataLen )
			{
				return	errFailed ;
			}
			nDataLen =
				(sizeof(DWORD_BE) * 2 + sizeof(BYTE) * 4 + sizeof(DWORD_BE)) ;
			if ( pfile->Read
				( &(plrLayer->dwSignature), nDataLen ) < nDataLen )
			{
				return	errFailed ;
			}
			//
			// 拡張情報を読み込む
			//
			SByteBuffer	bufExtra ;
			bufExtra.SetLength( plrLayer->dwExtraSize.GetUInt32() ) ;
			pfile->Read
				( bufExtra.GetArray(), (size_t) bufExtra.GetLength() ) ;
			bufExtra.FinishArray() ;
			//
			// Adjustment Layer Data
			//
			bufExtra.Read( &(plrLayer->adjdata.dwSize), sizeof(DWORD_BE) ) ;
			nDataLen = plrLayer->adjdata.dwSize.GetUInt32() ;
			if ( nDataLen != 0 )
			{
				size_t	nAdjustDataBytes =
							sizeof(ADJUST_LAYER_DATA) - sizeof(DWORD_BE) ;
				if ( nDataLen > nAdjustDataBytes )
				{
					bufExtra.Read
						( &(plrLayer->adjdata.dwTop), nAdjustDataBytes ) ;
					bufExtra.Seek
						( nDataLen - nAdjustDataBytes,
								SFileInterface::FromCurrent ) ;
				}
				else
				{
					bufExtra.Read( &(plrLayer->adjdata.dwTop), nDataLen ) ;
				}
			}
			//
			// Layer Blending ranges data
			//
			dwLength = 0 ;
			bufExtra.Read( &dwLength, sizeof(DWORD_BE) ) ;
			if ( dwLength.GetUInt32() != 0 )
			{
				bufExtra.Seek
					( dwLength.GetUInt32(), SFileInterface::FromCurrent ) ;
			}
			//
			// Layer name
			//
			BYTE	bytNameLen = 0 ;
			bufExtra.Read( &bytNameLen, sizeof(BYTE) ) ;
			//
			size_t	nNameBufLen = ((int) bytNameLen + 1 + 0x03) & ~0x03 ;
			SArray<uint8_t>	bufName ;
			bufName.SetLength( nNameBufLen - 1 ) ;
			bufExtra.Read( bufName.GetArray(), nNameBufLen - 1 ) ;
			bufName.FinishArray() ;
			//
			Charset::Decode
				( plrLayer->strLayerName,
					Charset::encodingShiftJIS,
					bufName.GetConstArray(), bytNameLen ) ;
			//
			// Unicode レイヤー名／グループ処理チャンク判別
			//
			plrLayer->typeLayer = LayerRecord::layerNormal ;
			plrLayer->dwGroupBlendMode = 0 ;
			//
			for ( ; ; )
			{
				DWORD_BE	dwChunkHeader[3] ;
				if ( bufExtra.Read
					( &dwChunkHeader[0],
						sizeof(DWORD_BE) * 3 ) < sizeof(DWORD_BE) * 3 )
				{
					break ;
				}
				if ( dwChunkHeader[0].GetUInt32() != 0x3842494D /*'8BIM'*/ )
				{
					break ;
				}
				nDataLen = dwChunkHeader[2].GetUInt32() ;
				int64_t	fposNextChunk = bufExtra.GetPosition() + nDataLen ;
				if ( dwChunkHeader[1].GetUInt32() == 0x6C756E69 /*'luni'*/ )
				{
					if ( bufExtra.Read
						( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
					{
						break ;
					}
					nDataLen = dwLength.GetUInt32() ;
					//
					uint16_t *	pwBufName =
						plrLayer->strLayerName.LockBuffer( nDataLen ) ;
					bufExtra.Read( pwBufName, nDataLen * sizeof(uint16_t) ) ;
					for ( size_t j = 0; j < nDataLen; j ++ )
					{
						pwBufName[j] = wswap( pwBufName[j] ) ;
					}
					plrLayer->strLayerName.UnlockBuffer( (ssize_t) nDataLen ) ;
					//
					bufExtra.Seek( nDataLen, SFileInterface::FromCurrent ) ;
				}
				else if ( dwChunkHeader[1].GetUInt32() == 0x6C6E7372 /*'lnsr'*/ )
				{
					DWORD_BE	dwType ;
					if ( bufExtra.Read
						( &dwType, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
					{
						break ;
					}
					if ( dwType.GetUInt32() == 0x62676E64 /* 'bgnd' */ )
					{
						plrLayer->typeLayer = LayerRecord::layerBackground ;
					}
					else if ( dwType.GetUInt32() == 0x6C736574 /* 'lset' */ )
					{
						plrLayer->typeLayer = LayerRecord::layerGroup ;
//						plrLayer->typeLayer = LayerRecord::layerEndOfGroup ;
					}
				}
				else if ( dwChunkHeader[1].GetUInt32() == 0x6C736374 /*'lsct'*/ )
				{
					if ( nDataLen >= 12 )
					{
						DWORD_BE	dwLSection[3] ;
						if ( bufExtra.Read
							( &dwLSection[0], sizeof(DWORD_BE) * 3 )
													< sizeof(DWORD_BE) * 3 )
						{
							break ;
						}
						if ( dwLSection[1].GetUInt32() == 0x3842494D /* '8BIM' */ )
						{
							plrLayer->typeLayer =
								(dwLSection[0].GetUInt32() == 3)
									? LayerRecord::layerEndOfGroup
									: LayerRecord::layerGroup ;
							plrLayer->dwGroupBlendMode = dwLSection[2] ;
						}
					}
					else
					{
						plrLayer->typeLayer = LayerRecord::layerEndOfGroup ;
					}
				}
				bufExtra.Seek( fposNextChunk ) ;
			}
		}
	}
	//
	// 完了
	//
	m_dwLayerDataPos = (uint32_t) pfile->GetPosition( ) ;
	//
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void FileReader::Close( void )
{
	m_bufColorMode.FreeArray( ) ;
	m_lstLayer.RemoveAll( ) ;
	//
	if ( m_flagFileOwner )
	{
		delete	m_pfile ;
	}
	m_pfile = NULL ;
	m_flagFileOwner = false ;
}

// ヘッダ情報を取得する
//////////////////////////////////////////////////////////////////////////////
void FileReader::GetFileHeader( FILE_HEADER & fhHeader ) const
{
	fhHeader = m_fhHeader ;
}

// カラーモード情報を取得する
//////////////////////////////////////////////////////////////////////////////
void FileReader::GetColorModeData( SArray<uint8_t>& bufColorMode ) const
{
	bufColorMode = m_bufColorMode ;
}

// レイヤーの総数を取得する
//////////////////////////////////////////////////////////////////////////////
size_t FileReader::GetLayerCount( void ) const
{
	return	m_lstLayer.GetLength() ;
}

// レイヤー情報を取得する
//////////////////////////////////////////////////////////////////////////////
SError FileReader::GetLayerInfo
	( FileReader::LayerInfo & liLayer, size_t iLayer ) const
{
	LayerRecord *	plrLayer = m_lstLayer.GetAt( iLayer ) ;
	if ( plrLayer == NULL )
	{
		return	errFailed ;
	}
	//
	liLayer.rectImage.x = (int32_t) plrLayer->dwLeft.GetUInt32() ;
	liLayer.rectImage.y = (int32_t) plrLayer->dwTop.GetUInt32() ;
	liLayer.rectImage.w = (int32_t) plrLayer->dwRight.GetUInt32() - liLayer.rectImage.x ;
	liLayer.rectImage.h = (int32_t) plrLayer->dwBottom.GetUInt32() - liLayer.rectImage.y ;
	//
	if ( plrLayer->adjdata.dwSize.GetUInt32() != 0 )
	{
		liLayer.rectMask.x = (int32_t) plrLayer->adjdata.dwLeft.GetUInt32() ;
		liLayer.rectMask.y = (int32_t) plrLayer->adjdata.dwTop.GetUInt32() ;
		liLayer.rectMask.w =
			(int32_t) plrLayer->adjdata.dwRight.GetUInt32() - liLayer.rectMask.x ;
		liLayer.rectMask.h =
			(int32_t) plrLayer->adjdata.dwBottom.GetUInt32() - liLayer.rectMask.y ;
	}
	else
	{
		liLayer.rectMask = liLayer.rectImage ;
	}
	//
	liLayer.nChannels = plrLayer->wChannels.GetUInt16() ;
	for ( size_t i = 0; i < liLayer.nChannels; i ++ )
	{
		liLayer.chlen[i] = plrLayer->chlen[i] ;
	}
	liLayer.dwBlendMode = plrLayer->dwBlendMode.GetUInt32() ;
	liLayer.nTransparency =
		0x100 - ((uint32_t) plrLayer->bytOpacity + (plrLayer->bytOpacity >> 7)) ;
	liLayer.dwFlags = plrLayer->bytFlags ;
	liLayer.dwAdjFlags = plrLayer->adjdata.Flags ;
	liLayer.dwDefColor = plrLayer->adjdata.bytDefColor ;
	//
	liLayer.typeLayer = plrLayer->typeLayer ;
	liLayer.dwGroupBlendMode = plrLayer->dwGroupBlendMode.GetUInt32() ;
	//
	liLayer.strLayerName = plrLayer->strLayerName ;
	//
	return	errSuccess ;
}

// レイヤー画像を取得する
//////////////////////////////////////////////////////////////////////////////
SError FileReader::LoadLayerImage
	( SGLImageObject & imgBuf, size_t iLayer, FileReadListener * pListener )
{
	if ( m_pfile == NULL )
	{
		return	errFailed ;
	}
	//
	// データのファイル上のアドレスを計算する
	//
	if ( iLayer == (size_t) -1 )
	{
		return	LoadBaseImage( imgBuf ) ;
	}
	DWORD		dwDataPos ;
	LayerInfo	liLayer ;
	dwDataPos = m_dwLayerDataPos ;
	for ( size_t i = 0; i < iLayer; i ++ )
	{
		if ( GetLayerInfo( liLayer, i ) )
		{
			return	errFailed ;
		}
		for ( size_t j = 0; j < liLayer.nChannels; j ++ )
		{
			dwDataPos += liLayer.chlen[j].dwLength.GetUInt32() ;
		}
	}
	if ( GetLayerInfo( liLayer, iLayer ) )
	{
		return	errFailed ;
	}
	//
	// 画像バッファの生成
	//
	uint32_t	format, depth, nPixelStep, nDefLineBytes ;
	size_t		nChannels = liLayer.nChannels ;
	size_t		nMainChannels = 3 ;
	bool		fGrayToRGB = false ;
	uint16_t	wMode = m_fhHeader.wMode.GetUInt16() ;
	if ( (nChannels < 3)
		|| (wMode == modeGrayscale) || (wMode == modeIndexed) )
	{
		if ( nChannels <= 0 )
		{
			return	errFailed ;
		}
		depth = 8 ;
		nPixelStep = 1 ;
		nDefLineBytes = (liLayer.rectImage.w * depth + 0x07) >> 3 ;
		nMainChannels = 1 ;
		if ( m_fhHeader.wMode.GetUInt16() == modeIndexed )
		{
			format = formatImageARGB | formatImageFlagPalette ;
			nChannels = 1 ;
		}
		else
		{
			format = formatImageGray ;
			if ( nChannels >= 2 )
			{
				format = formatImageARGB
						| formatImageFlagNoProductOfAlpha ;
				fGrayToRGB = true ;
				//
				depth = 32 ;
				nPixelStep = 4 ;
				nDefLineBytes = liLayer.rectImage.w ;
			}
		}
	}
	else if ( nChannels == 3 )
	{
		if ( m_fhHeader.wDepth.GetUInt16() != 8 )
		{
			return	errFailed ;
		}
		format = formatImageRGB ;
		depth = 32 ;
		nPixelStep = 4 ;
		nDefLineBytes = liLayer.rectImage.w ;
	}
	else if ( nChannels >= 4 )
	{
		if ( m_fhHeader.wDepth.GetUInt16() != 8 )
		{
			return	errFailed ;
		}
		format = formatImageARGB | formatImageFlagNoProductOfAlpha ;
		depth = 32 ;
		nPixelStep = 4 ;
		nDefLineBytes = liLayer.rectImage.w ;
	}
	imgBuf.CreateImage
		( liLayer.rectImage.w, liLayer.rectImage.h, format, depth ) ;
	//
	SGLImageInfo	imginf ;
	uint8_t *	pbytPixels = imgBuf.LockBuffer( imginf ) ;
	if ( pbytPixels == NULL )
	{
		return	errFailed ;
	}
	//
	// 各チャネルを展開
	//
	bool	fAlphaFlag = false ;
	for ( size_t iChannel = 0; iChannel < nChannels; iChannel ++ )
	{
		//
		// ファイルのシーク
		//
		if ( m_pfile->Seek( dwDataPos ) != dwDataPos )
		{
			return	errFailed ;
		}
		dwDataPos += liLayer.chlen[iChannel].dwLength.GetUInt32() ;
		//
		// 圧縮方式取得
		//
		WORD_BE		wbeCompression ;
		uint16_t	wCompression ;
		if ( m_pfile->Read
			( &wbeCompression, sizeof(WORD_BE) ) < sizeof(WORD_BE) )
		{
			return	errFailed ;
		}
		wCompression = wbeCompression.GetUInt16() ;
		//
		// 各行のバイト数を取得
		//
		SArray<uint8_t>		bufLineBuf ;
		SArray<uint16_t>	bufLineSize ;
		SArray<uint8_t>		bufRLE ;
		uint16_t *			pwLineSize = NULL ;
		uint8_t *			ptrRLE = NULL ;
		size_t				iRLE = 0 ;
		size_t				nTotalBytes = 0 ;
		size_t				nLineBytes ;
		SGLImageRect		irRect ;
		//
		if ( liLayer.chlen[iChannel].wChannelID.GetUInt16()
								== (WORD) chidUserSuppliedMask )
		{
			irRect = liLayer.rectMask ;
			irRect.x -= liLayer.rectImage.x ;
			irRect.y -= liLayer.rectImage.y ;
			nLineBytes = irRect.w ;
			//
			if ( irRect.x + irRect.w > liLayer.rectImage.w )
			{
				irRect.w = liLayer.rectImage.w - irRect.x ;
				if ( irRect.w < 0 )
				{
					irRect.w = 0 ;
				}
			}
		}
		else
		{
			irRect.x = 0 ;
			irRect.y = 0 ;
			irRect.w = liLayer.rectImage.w ;
			irRect.h = liLayer.rectImage.h ;
			nLineBytes = nDefLineBytes ;
		}
		uint8_t *	ptrLineBuf ;
		bufLineBuf.SetLength( nLineBytes ) ;
		ptrLineBuf = bufLineBuf.GetArray() ;
		//
		if ( wCompression == 1 )	// RLE
		{
			size_t	nBytes = irRect.h * sizeof(WORD) ;
			bufLineSize.SetLength( irRect.h ) ;
			pwLineSize = bufLineSize.GetArray() ;
			if ( m_pfile->Read( pwLineSize, nBytes ) < nBytes )
			{
				return	errFailed ;
			}
			for ( int i = 0; i < irRect.h; i ++ )
			{
				pwLineSize[i] = wswap( pwLineSize[i] ) ;
				nTotalBytes += pwLineSize[i] ;
			}
			bufLineSize.FinishArray() ;
			//
			bufRLE.SetLength( nTotalBytes ) ;
			ptrRLE = bufRLE.GetArray() ;
			if ( m_pfile->Read( ptrRLE, nTotalBytes ) < nTotalBytes )
			{
				bufRLE. FinishArray() ;
				return	errFailed ;
			}
			bufRLE. FinishArray() ;
		}
		else if ( wCompression != 0 )
		{
			return	errFailed ;
		}
		//
		// 各ラインを順次読み込み・展開
		//
		for ( int y = 0; y < irRect.h; y ++ )
		{
			if ( wCompression == 0 )	// Non-compress
			{
				if ( m_pfile->Read( ptrLineBuf, nLineBytes ) < nLineBytes )
				{
					return	errFailed ;
				}
			}
			else						// RLE
			{
				UnpackBits
					( ptrLineBuf, nLineBytes,
						ptrRLE + iRLE, pwLineSize[y] ) ;
				iRLE += pwLineSize[y] ;
			}
			//
			uint8_t *	ptrDstLine = pbytPixels ;
			if ( y + irRect.y < 0 )
			{
				continue ;
			}
			else if ( (uint32_t) (y + irRect.y) >= imginf.height )
			{
				break ;
			}
			ptrDstLine += imginf.pitchLine * (y + irRect.y) ;
			//
			size_t	i ;
			bool	fProduct = false ;
			switch ( liLayer.chlen[iChannel].wChannelID.GetUInt16() )
			{
			case	chidRed:
				i = 2 ;
				break ;
			case	chidGreen:
				i = 1 ;
				break ;
			case	chidBlue:
				i = 0 ;
				break ;
			default:
				i = 3 ;
				fProduct = fAlphaFlag ;
				break ;
			}
			if ( i * 8 >= depth )
			{
				i = 0 ;
			}
			i += nPixelStep * irRect.x ;
			//
			if ( fProduct )
			{
				int	w = irRect.w ;
				if ( irRect.x < 0 )
				{
					w += irRect.x ;
					i -= nPixelStep * irRect.x ;
				}
				for ( int x = 0; x < w; x ++ )
				{
					ptrDstLine[i] =
						(uint8_t) (((uint16_t) ptrDstLine[i]
								* ((uint16_t) ptrLineBuf[x] + 1)) >> 8) ;
					i += nPixelStep ;
				}
			}
			else
			{
				for ( int x = 0; x < irRect.w; x ++ )
				{
					ptrDstLine[i] = ptrLineBuf[x] ;
					i += nPixelStep ;
				}
			}
			if ( pListener != NULL )
			{
				SError	err =
					pListener->OnReadImage
						( this, iChannel * irRect.h + y + 1,
											irRect.h * nChannels ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		if ( (liLayer.chlen[iChannel].wChannelID.GetUInt16()
									== (WORD) chidUserSuppliedMask)
											&& (liLayer.dwDefColor == 0) )
		{
			//
			// ユーザーマスクチャネルの領域外の処理
			//
			for ( uint32_t y = 0; y < imginf.height; y ++ )
			{
				uint8_t *	ptrDstLine = pbytPixels ;
				ptrDstLine += imginf.pitchLine * y ;
				//
				if ( ((int32_t) y < irRect.y)
						| (irRect.y + irRect.h <= (int32_t) y) )
				{
					for ( uint32_t x = 0; x < imginf.width; x ++ )
					{
						ptrDstLine[3] = 0 ;
						ptrDstLine += nPixelStep ;
					}
				}
				else
				{
					for ( uint32_t x = 0; x < imginf.width; x ++ )
					{
						if ( ((int32_t) x < irRect.x)
							| (irRect.x + irRect.w <= (int32_t) x) )
						{
							ptrDstLine[3] = 0 ;
						}
						ptrDstLine += nPixelStep ;
					}
				}
			}
		}
		switch ( liLayer.chlen[iChannel].wChannelID.GetUInt16() )
		{
		case	chidRed:
		case	chidGreen:
		case	chidBlue:
			break ;
		default:
			fAlphaFlag = true ;
			break ;
		}
		bufLineBuf.FinishArray() ;
	}
	if ( fGrayToRGB )
	{
		uint8_t *	pbytNextLine = pbytPixels ;
		for ( uint32_t y = 0; y < imginf.height; y ++ )
		{
			uint8_t *	pbytNextPixel = pbytNextLine ;
			for ( uint32_t x = 0; x < imginf.width; x ++ )
			{
				uint8_t	p = pbytNextPixel[2] ;
				pbytNextPixel[0] = p ;
				pbytNextPixel[1] = p ;
				pbytNextPixel += nPixelStep ;
			}
			pbytNextLine += imginf.pitchLine ;
		}
	}
	imgBuf.UnlockBuffer() ;
	return	errSuccess ;
}

// ベース画像を取得する
//////////////////////////////////////////////////////////////////////////////
SError FileReader::LoadBaseImage
	( SGLImageObject & imgBuf, FileReadListener * pListener )
{
	if ( m_pfile == NULL )
	{
		return	errFailed ;
	}
	//
	// ヘッダ情報取得
	//
	FILE_HEADER	fhHeader ;
	GetFileHeader( fhHeader ) ;
	//
	// 画像バッファの生成
	//
	uint32_t	format, depth, nPixelStep, nLineBytes ;
	size_t		nChannels = fhHeader.wChannels.GetUInt16() ;
	uint16_t	wMode = m_fhHeader.wMode.GetUInt16() ;
	if ( (nChannels < 3)
		|| (wMode == modeGrayscale) || (wMode == modeIndexed) )
	{
		if ( nChannels <= 0 )
		{
			return	errFailed ;
		}
		if ( wMode == modeIndexed )
		{
			format = formatImageARGB | formatImageFlagPalette ;
			nChannels = 1 ;
		}
		else
		{
			format = formatImageGray ;
		}
		depth = 8 ;
		nPixelStep = 1 ;
		nLineBytes = fhHeader.dwWidth.GetUInt32() ;
	}
	else if ( nChannels == 3 )
	{
		if ( fhHeader.wDepth.GetUInt16() != 8 )
		{
			return	errFailed ;
		}
		format = formatImageRGB ;
		nLineBytes = fhHeader.dwWidth.GetUInt32() ;
		depth = 32 ;
		nPixelStep = 4 ;
	}
	else if ( nChannels >= 4 )
	{
		if ( fhHeader.wDepth.GetUInt16() != 8 )
		{
			return	errFailed ;
		}
		format = formatImageARGB ;
		depth = 32 ;
		nPixelStep = 4 ;
		nLineBytes = fhHeader.dwWidth.GetUInt32() ;
	}
	imgBuf.CreateImage
		( fhHeader.dwWidth.GetUInt32(),
			fhHeader.dwHeight.GetUInt32(), format, depth ) ;
	//
	SGLImageInfo	imginf ;
	uint8_t *	pbytPixels = imgBuf.LockBuffer( imginf ) ;
	if ( pbytPixels == NULL )
	{
		return	errFailed ;
	}
	//
	// ファイルのシーク
	//
	if ( m_pfile->Seek( m_dwBaseDataPos ) != m_dwBaseDataPos )
	{
		return	errFailed ;
	}
	//
	// 圧縮方式取得
	//
	WORD_BE	wbeCompression ;
	WORD	wCompression ;
	if ( m_pfile->Read( &wbeCompression, sizeof(WORD_BE) ) < sizeof(WORD_BE) )
	{
		return	errFailed ;
	}
	wCompression = wbeCompression.GetUInt16() ;
	//
	// 各行のバイト数を取得
	//
	SArray<uint8_t>		bufLineBuf ;
	SArray<uint16_t>	bufLineSize ;
	SArray<uint8_t>		bufRLE ;
	uint8_t *	ptrLineBuf = NULL ;
	uint16_t *	pwLineSize = NULL ;
	uint8_t *	ptrRLE = NULL ;
	size_t		iRLE = 0 ;
	size_t		nTotalBytes = 0 ;
	size_t		nTotalLines = fhHeader.dwHeight.GetUInt32() * nChannels ;
	//
	bufLineBuf.SetLength( nLineBytes ) ;
	ptrLineBuf = bufLineBuf.GetArray() ;
	//
	if ( wCompression == 1 )	// RLE
	{
		size_t	nBytes = nTotalLines * sizeof(uint16_t) ;
		bufLineSize.SetLength( nTotalLines ) ;
		pwLineSize = bufLineSize.GetArray() ;
		if ( m_pfile->Read( pwLineSize, nBytes ) < nBytes )
		{
			bufLineSize.FinishArray() ;
			return	errFailed ;
		}
		for ( size_t i = 0; i < nTotalLines; i ++ )
		{
			pwLineSize[i] = wswap( pwLineSize[i] ) ;
			nTotalBytes += pwLineSize[i] ;
		}
		bufLineSize.FinishArray() ;
		//
		bufRLE.SetLength( nTotalBytes ) ;
		ptrRLE = bufRLE.GetArray() ;
		if ( m_pfile->Read( ptrRLE, nTotalBytes ) < nTotalBytes )
		{
			bufRLE.FinishArray() ;
			return	errFailed ;
		}
		bufRLE.FinishArray() ;
	}
	else if ( wCompression != 0 )
	{
		return	errFailed ;
	}
	//
	// 各チャネルを展開
	//
	for ( size_t iChannel = 0; iChannel < nChannels; iChannel ++ )
	{
		//
		// 各ラインを順次読み込み
		//
		const uint32_t	nHeight = fhHeader.dwHeight.GetUInt32() ;
		size_t	iChOffset = iChannel ;
		bool	fProduct = (iChannel >= 4) ;
		if ( depth == 32 )
		{
			if ( iChannel < 3 )
			{
				iChOffset = 2 - iChannel ;
			}
			else
			{
				iChOffset = 3 ;
			}
		}
		if ( iChOffset * 8 >= depth )
		{
			iChOffset = 0 ;
			fProduct = true ;
		}
		for ( uint32_t y = 0; y < nHeight; y ++ )
		{
			if ( wCompression == 0 )	// Non-compress
			{
				if ( m_pfile->Read( ptrLineBuf, nLineBytes ) < nLineBytes )
				{
					return	errFailed ;
				}
			}
			else						// RLE
			{
				size_t	i = y + iChannel * nHeight ;
				UnpackBits
					( ptrLineBuf, nLineBytes,
						ptrRLE + iRLE, pwLineSize[i] ) ;
				iRLE += pwLineSize[i] ;
			}
			//
			uint8_t *	ptrDstLine = pbytPixels ;
			ptrDstLine += imginf.pitchLine * y ;
			//
			size_t		i = iChOffset ;
			uint32_t	nWidth = fhHeader.dwWidth.GetUInt32() ;
			if ( !fProduct )
			{
				for ( uint32_t x = 0; x < nWidth; x ++ )
				{
					ptrDstLine[i] = ptrLineBuf[x] ;
					i += nPixelStep ;
				}
			}
			else
			{
				for ( uint32_t x = 0; x < nWidth; x ++ )
				{
					ptrDstLine[i] =
						(uint8_t) (((uint16_t) ptrDstLine[i]
									* ((uint16_t) ptrLineBuf[x] + 1)) >> 8) ;
					i += nPixelStep ;
				}
			}
			if ( pListener != NULL )
			{
				SError	err =
					pListener->OnReadImage
						( this, iChannel * nHeight + y + 1,
											nHeight * nChannels ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
	}
	bufLineBuf.FinishArray() ;
	imgBuf.UnlockBuffer() ;
	return	errSuccess ;
}

// RLE の展開
//////////////////////////////////////////////////////////////////////////////
void FileReader::UnpackBits
	( uint8_t * ptrBuf, size_t nBufSize,
			const uint8_t * ptrRLE, size_t nRLESize )
{
	size_t	i = 0, j = 0 ;
	while ( (j < nBufSize) && (i + 2 <= nRLESize) )
	{
		size_t	nLength ;
		if ( ptrRLE[i]  & 0x80 )
		{
			uint8_t	bytFill ;
			nLength = (size_t) 1 - (int8_t) ptrRLE[i ++] ;
			bytFill = ptrRLE[i ++] ;
			if ( j + nLength > nBufSize )
			{
				nLength = nBufSize - j ;
			}
			size_t	k ;
			for ( k = 0; k < nLength; k ++ )
			{
				ptrBuf[j + k] = bytFill ;
			}
			j += k ;
		}
		else
		{
			nLength = (size_t) 1 + ptrRLE[i ++] ;
			if ( j + nLength > nBufSize )
			{
				nLength = nBufSize - j ;
			}
			if ( i + nLength > nRLESize )
			{
				nLength = nRLESize - i ;
			}
			for ( size_t k = 0; k < nLength; k ++ )
			{
				ptrBuf[j + k] = ptrRLE[i + k] ;
			}
			i += nLength ;
			j += nLength ;
		}
	}
}


// PSD ファイル読み込みリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( PSD::FileReadListener )

// 進行状況
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	FileReadListener::OnReadImage
		( FileReader * pfr, size_t current, size_t total )
{
	return	errSuccess ;
}





//////////////////////////////////////////////////////////////////////////////
// PSD ファイル・ライター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( PSD::FileWriter, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
FileWriter::FileWriter( void )
	: m_pfile( nullptr ), m_flagFileOwner( false )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
FileWriter::~FileWriter( void )
{
	FileWriter::Close( ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileWriter::Open
	( const CanvasInfo& canvasInfo,
		SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	Close( ) ;
	//
	// ファイルヘッダを書き込む
	//
	memset( &m_fhHeader, 0, sizeof(m_fhHeader) ) ;
	m_fhHeader.dwSignature = PSD_CHUNK_SIGNATURE('8','B','P','S') ;
	m_fhHeader.wVersion = 1 ;
	m_fhHeader.wChannels.SetUInt16( (uint16_t) canvasInfo.nChannels ) ;
	m_fhHeader.dwHeight.SetUInt32( canvasInfo.nHeight ) ;
	m_fhHeader.dwWidth.SetUInt32( canvasInfo.nWidth ) ;
	m_fhHeader.wDepth.SetUInt16( (uint16_t) canvasInfo.nDepth ) ;
	m_fhHeader.wMode.SetUInt16( (uint16_t) canvasInfo.mode ) ;
	//
	m_pfile = pfile ;
	m_flagFileOwner = flagOwnFile ;
	//
	if ( PSD_CHUNK_SIGNATURE('8','B','P','S')
						!= m_fhHeader.dwSignature.GetUInt32() )
	{
		return	errFailed ;
	}
	if ( pfile->Write( &m_fhHeader, sizeof(m_fhHeader) ) < sizeof(m_fhHeader) )
	{
		return	errFailed ;
	}
	//
	// Color Mode Data
	//
	DWORD_BE	dwLength = (uint32_t) canvasInfo.nColorDataByes ;
	if ( pfile->Write( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	if ( canvasInfo.nColorDataByes > 0 )
	{
		if ( pfile->Write
			( canvasInfo.pColorModeData,
				canvasInfo.nColorDataByes ) < canvasInfo.nColorDataByes )
		{
			return	errFailed ;
		}
	}
	//
	// Image Resource Section
	//
	dwLength = 0 ;
	if ( pfile->Write( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	//
	// Layer and Mask Information Section
	//
	m_fpLayerMaskInfo = pfile->GetPosition() ;
	dwLength = 0 ;
	if ( pfile->Write( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	if ( pfile->Write( &dwLength, sizeof(DWORD_BE) ) < sizeof(DWORD_BE) )
	{
		return	errFailed ;
	}
	WORD_BE	wLayerCount = 0 ;
	if ( pfile->Write( &wLayerCount, sizeof(WORD_BE) ) < sizeof(WORD_BE) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void FileWriter::Close( void )
{
	if ( m_flagFileOwner )
	{
		delete	m_pfile ;
	}
	m_pfile = nullptr ;
	m_flagFileOwner = false ;
}

// レイヤー情報追加
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileWriter::AppendLayerInfo
	( const wchar_t * pwszLayerName,
		SakuraGL::SGLImageObject * pImage,
		int xOffset, int yOffset,
		PSD::LayerBlendMode blendMode,
		uint32_t nOpacity, bool flagClipping, int nLayerFlags,
		LayerRecord::LayerType typeLayer, PSD::LayerBlendMode blendGroup )
{
	SGLImageInfo	imginf ;
	if ( pImage != nullptr )
	{
		pImage->GetImageInfo( imginf ) ;
	}
	uint32_t	nChannels = imginf.depth / 8 ;
	if ( imginf.format & formatImageFlagAlpha )
	{
		nChannels = (uint32_t) esl_min( (int) nChannels, 4 ) ;
	}
	else
	{
		nChannels = (uint32_t) esl_min( (int) nChannels, 3 ) ;
	}
	uint32_t	nPlaneBytes = imginf.width * imginf.height + sizeof(WORD_BE) ;
	if ( pImage == nullptr )
	{
		nChannels = 4 ;
	}
	//
	// レイヤー情報設定
	//
	const DWORD_BE	dw8BIM = PSD_CHUNK_SIGNATURE('8','B','I','M') ;
	//
	LayerInfo *	pliLayer = new LayerInfo ;
	pliLayer->pImage = pImage ;
	pliLayer->dwTop.SetUInt32( (uint32_t) yOffset ) ;
	pliLayer->dwLeft.SetUInt32( (uint32_t) xOffset ) ;
	pliLayer->dwBottom.SetUInt32( (uint32_t) (imginf.height + yOffset) ) ;
	pliLayer->dwRight.SetUInt32( (uint32_t) (imginf.width + xOffset) ) ;

	pliLayer->wChannels.SetUInt16( (uint16_t) nChannels ) ;
	if ( (imginf.format & formatImageColorSpaceMask) == formatImageBGR )
	{
		pliLayer->chlen[0].wChannelID.SetUInt16( chidRed ) ;
		pliLayer->chlen[1].wChannelID.SetUInt16( chidGreen ) ;
		pliLayer->chlen[2].wChannelID.SetUInt16( chidBlue ) ;
	}
	else
	{
		pliLayer->chlen[0].wChannelID.SetUInt16( chidBlue ) ;
		pliLayer->chlen[1].wChannelID.SetUInt16( chidGreen ) ;
		pliLayer->chlen[2].wChannelID.SetUInt16( chidRed ) ;
	}
	pliLayer->chlen[3].wChannelID.SetUInt16( chidTransparencyMask ) ;
	for ( uint32_t i = 0; i < nChannels; i ++ )
	{
		pliLayer->chlen[i].dwLength.SetUInt32( nPlaneBytes ) ;
	}
	pliLayer->dwSignature.SetUInt32( dw8BIM ) ;
	pliLayer->dwBlendMode.SetUInt32( blendMode ) ;
	pliLayer->bytOpacity = (nOpacity > 0xFF) ? 0xFF : (BYTE) nOpacity ;
	pliLayer->bytClipping = flagClipping ? 1 : 0 ;
	pliLayer->bytFlags = (BYTE) nLayerFlags ;
	pliLayer->bytFilter = 0 ;
	pliLayer->dwExtraSize.SetUInt32( 0 ) ;

	pliLayer->adjdata.dwSize.SetUInt32( sizeof(ADJUST_LAYER_DATA) - sizeof(DWORD_BE) ) ;
	pliLayer->adjdata.dwTop.SetUInt32( (uint32_t) yOffset ) ;
	pliLayer->adjdata.dwLeft.SetUInt32( (uint32_t) xOffset ) ;
	pliLayer->adjdata.dwBottom.SetUInt32( (uint32_t) (imginf.height + yOffset) ) ;
	pliLayer->adjdata.dwRight.SetUInt32( (uint32_t) (imginf.width + xOffset) ) ;
	pliLayer->adjdata.bytDefColor = 0 ;
	pliLayer->adjdata.Flags = 0 ;
	pliLayer->adjdata.wPadding.SetUInt16( 0 ) ;

	m_lstLayer.Add( pliLayer ) ;

	//
	// 拡張情報
	//
	SQueueBuffer	qbuf ;
	qbuf.Write( &(pliLayer->adjdata), sizeof(ADJUST_LAYER_DATA) ) ;
	//
	// Layer Blending ranges data
	DWORD_BE	dwLength = 10 * sizeof(DWORD_BE) ;
	qbuf.Write( &dwLength, sizeof(DWORD_BE) ) ;
	for ( int i = 0; i < 10; i ++ )
	{
		WORD_BE	w0 = 0 ;
		WORD_BE	wf = 0xFFFF ;
		qbuf.Write( &w0, sizeof(WORD_BE) ) ;
		qbuf.Write( &wf, sizeof(WORD_BE) ) ;
	}
	//
	// Layer name (Pascal string)
	SArray<uint8_t>	bufSJISLayerName ;
	Charset::Encode
		( bufSJISLayerName, Charset::encodingShiftJIS, pwszLayerName ) ;
	//
	BYTE	bytSJISNameLen = (BYTE) esl_min( (int) bufSJISLayerName.GetLength(), 0xFF ) ;
	size_t	nSJISNameBufLen = ((int) bytSJISNameLen + 1 + 0x03) & ~0x03 ;
	bufSJISLayerName.SetLength( nSJISNameBufLen ) ;
	qbuf.Write( &bytSJISNameLen, sizeof(BYTE) ) ;
	qbuf.Write( bufSJISLayerName.GetConstArray(), nSJISNameBufLen - 1 ) ;
	//
	// Protected setting (Photoshop 6.0)
	DWORD_BE	dwKey = PSD_CHUNK_SIGNATURE('l','s','p','f') ;
	DWORD_BE	dwProtectionFlags = 0 ;
	dwLength = sizeof(DWORD_BE) ;
	qbuf.Write( &dw8BIM, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwKey, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwLength, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwProtectionFlags, sizeof(DWORD_BE) ) ;
	//
	// Unicode layer name (Photoshop 5.0)
	SString		strLayerName = pwszLayerName ;
	size_t		nStrLen = strLayerName.GetLength() ;
	DWORD_BE	dwStrLen = (uint32_t) nStrLen ;
	dwKey = PSD_CHUNK_SIGNATURE('l','u','n','i') ;
	dwLength = (uint32_t) (strLayerName.GetLength() * 2 + sizeof(DWORD_BE)) ;
	qbuf.Write( &dw8BIM, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwKey, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwLength, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwStrLen, sizeof(DWORD_BE) ) ;
	//
	uint16_t *	pwString = strLayerName.LockBuffer( nStrLen ) ;
	for ( size_t i = 0; i < nStrLen; i ++ )
	{
		pwString[i] = wswap( pwString[i] ) ;
	}
	qbuf.Write( pwString, nStrLen * sizeof(WORD_BE) ) ;
	strLayerName.UnlockBuffer( (ssize_t) nStrLen ) ;
	//
	// Transparency shapes layer (Photoshop 7.0)
	BYTE	bytTransShapes[4] = { 1, 0, 0, 0 } ;
	if ( typeLayer == LayerRecord::layerEndOfGroup )
	{
		bytTransShapes[0] = 0 ;
	}
	dwKey = PSD_CHUNK_SIGNATURE('t','s','l','y') ;
	dwLength = sizeof(bytTransShapes) ;
	qbuf.Write( &dw8BIM, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwKey, sizeof(DWORD_BE) ) ;
	qbuf.Write( &dwLength, sizeof(DWORD_BE) ) ;
	qbuf.Write( &bytTransShapes[0], sizeof(bytTransShapes) ) ;
	//
	// Section divider setting (Photoshop 6.0)
	if ( (typeLayer == LayerRecord::layerGroup)
		|| (typeLayer == LayerRecord::layerEndOfGroup) )
	{
		DWORD_BE	dwLSection[3] = { 1, dw8BIM, (uint32_t) blendGroup } ;
		if ( typeLayer == LayerRecord::layerEndOfGroup )
		{
			dwLSection[0] = 3 ;
		}
		dwKey = PSD_CHUNK_SIGNATURE('l','s','c','t') ;
		dwLength = sizeof(dwLSection) ;
		qbuf.Write( &dw8BIM, sizeof(DWORD_BE) ) ;
		qbuf.Write( &dwKey, sizeof(DWORD_BE) ) ;
		qbuf.Write( &dwLength, sizeof(DWORD_BE) ) ;
		qbuf.Write( dwLSection, sizeof(dwLSection) ) ;
	}

	//
	size_t	nExBytes = (size_t) qbuf.GetLength() ;
	pliLayer->bufExtra.SetLength( nExBytes ) ;
	qbuf.Read( pliLayer->bufExtra.GetArray(), nExBytes ) ;
	pliLayer->bufExtra.FinishArray() ;
	//
	pliLayer->dwExtraSize.SetUInt32( (uint32_t) nExBytes ) ;

	//
	// ファイル出力
	//
	m_pfile->Write
		( &(pliLayer->dwTop), sizeof(DWORD_BE) * 4 + sizeof(WORD_BE) ) ;
	m_pfile->Write
		( &(pliLayer->chlen[0]), sizeof(CHANNEL_LENGTH_INFO) * nChannels ) ;
	m_pfile->Write
		( &(pliLayer->dwSignature),
			sizeof(DWORD_BE) * 2 + sizeof(BYTE) * 4 + sizeof(DWORD_BE) ) ;
	m_pfile->Write
		( pliLayer->bufExtra.GetConstArray(),
				pliLayer->bufExtra.GetLength() ) ;

	return	errSuccess ;
}

SSystem::SError FileWriter::AppendGroupLayer
	( const wchar_t * pwszLayerName, PSD::LayerBlendMode blendGroup )
{
	return	AppendLayerInfo
			( pwszLayerName, nullptr, 0, 0,
				PSD::blendNormal, 255, false, 0,
				PSD::LayerRecord::layerGroup, blendGroup ) ;
}

SSystem::SError FileWriter::AppendEndOfGroup( const wchar_t * pwszLayerName )
{
	return	AppendLayerInfo
			( pwszLayerName, nullptr, 0, 0,
				PSD::blendNormal, 255, false, 0,
				PSD::LayerRecord::layerEndOfGroup, blendNormal ) ;
}

// レイヤー画像データを出力
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileWriter::WriteAllLayerImages( FileWriteListener * pListener )
{
	ESLAssert( m_pfile != nullptr ) ;
	if ( m_pfile == nullptr )
	{
		return	errFailed ;
	}
	//
	// 画像データ出力
	//
	for ( size_t i = 0; i < m_lstLayer.GetLength(); i ++ )
	{
		LayerInfo *	pliLayer = m_lstLayer.GetAt( i ) ;
		ESLAssert( pliLayer != nullptr ) ;
		if ( pListener != nullptr )
		{
			SError	err = pListener->OnWriteLayer
							( this, pliLayer, i, m_lstLayer.GetLength() ) ;
			if ( err )
			{
				return	err ;
			}
		}
		SGLImageInfo	imginf ;
		uint8_t *		pbytPixels = nullptr ;
		if ( pliLayer->pImage != nullptr )
		{
			pbytPixels = pliLayer->pImage->LockBuffer( imginf ) ;
			if ( pbytPixels == nullptr )
			{
				continue ;
			}
		}
		SArray<uint8_t>		bufLineBuf ;
		bufLineBuf.SetLength( imginf.width ) ;
		//
		// 各チャネル
		//
		const size_t	nChannels = pliLayer->wChannels.GetUInt16() ;
		for ( size_t iChannel = 0; iChannel < nChannels; iChannel ++ )
		{
			//
			// 圧縮方式
			WORD_BE	wbeCompression = 0 ;
			m_pfile->Write( &wbeCompression, sizeof(WORD_BE) ) ;
			//
			if ( pliLayer->pImage == nullptr )
			{
				continue ;
			}
			//
			// 各ライン
			for ( uint32_t y = 0; y < imginf.height; y ++ )
			{
				uint8_t *	pbytSrcLine = pbytPixels + (int32_t) y * imginf.pitchLine + iChannel ;
				uint8_t *	pbytDstLine = bufLineBuf.GetArray() ;
				for ( uint32_t x = 0; x < imginf.width; x ++ )
				{
					pbytDstLine[x] = *pbytSrcLine ;
					pbytSrcLine += imginf.pitchPixel ;
				}
				m_pfile->Write( pbytDstLine, imginf.width ) ;
			}
		}
		if ( pliLayer->pImage != nullptr )
		{
			pliLayer->pImage->UnlockBuffer() ;
		}
		if ( pListener != nullptr )
		{
			SError	err = pListener->OnFinishedLayer
							( this, pliLayer, i, m_lstLayer.GetLength() ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	//
	// レイヤー情報を完成する
	//
	int64_t	fpFilePos = m_pfile->GetPosition() ;
	m_pfile->Seek( m_fpLayerMaskInfo + sizeof(DWORD_BE) ) ;
	//
	DWORD_BE	dwLength = (uint32_t) (fpFilePos - (m_fpLayerMaskInfo + sizeof(DWORD_BE)*2)) ;
	m_pfile->Write( &dwLength, sizeof(DWORD_BE) ) ;
	//
	WORD_BE	wLayerCount = (uint16_t) m_lstLayer.GetLength() ;
	m_pfile->Write( &wLayerCount, sizeof(WORD_BE) ) ;
	//
	m_pfile->Seek( fpFilePos ) ;
	//
	// Global layer mask info
	//
	GLOBAL_LAYER_MASK_INFO	glmi ;
	memset( &glmi, 0, sizeof(GLOBAL_LAYER_MASK_INFO) ) ;
	glmi.dwSize.SetUInt32( sizeof(GLOBAL_LAYER_MASK_INFO) - sizeof(DWORD_BE) ) ;
	glmi.bytKind = glmkindUseValueStoredPerLayer ;
	m_pfile->Write( &glmi, sizeof(GLOBAL_LAYER_MASK_INFO) ) ;
	//
	// Additional Layer Information
	//
	DWORD_BE	dwSignature = PSD_CHUNK_SIGNATURE('8','B','I','M') ;
	DWORD_BE	dwKey = PSD_CHUNK_SIGNATURE('P','a','t','t') ;
	dwLength = 0 ;
	m_pfile->Write( &dwSignature, sizeof(DWORD_BE) ) ;
	m_pfile->Write( &dwKey, sizeof(DWORD_BE) ) ;
	m_pfile->Write( &dwLength, sizeof(DWORD_BE) ) ;
	//
	return	errSuccess ;
}

// ベース画像（合成済み）を出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileWriter::WriteBaseImage( SakuraGL::SGLImageObject & imgBuf )
{
	ESLAssert( m_pfile != nullptr ) ;
	if ( m_pfile == nullptr )
	{
		return	errFailed ;
	}
	int64_t			fpFilePos = m_pfile->GetPosition() ;
	SGLImageInfo	imginf ;
	uint8_t *	pbytPixels = imgBuf.LockBuffer( imginf ) ;
	if ( pbytPixels == nullptr )
	{
		return	errFailed ;
	}
	const uint32_t	nWidth = m_fhHeader.dwWidth.GetUInt32() ;
	const uint32_t	nHeight = m_fhHeader.dwHeight.GetUInt32() ;
	const size_t	nChannels = m_fhHeader.wChannels.GetUInt16() ;
	const uint16_t	wMode = m_fhHeader.wMode.GetUInt16() ;
	if ( (nWidth != imginf.width)
		|| (nHeight != imginf.height)
		|| (nChannels > imginf.depth / 8)
		|| (nChannels > 4) )
	{
		imgBuf.UnlockBuffer() ;
		return	errFailed ;
	}
	SArray<uint8_t>		bufLineBuf ;
	bufLineBuf.SetLength( nWidth ) ;
	//
	// 圧縮方式
	//
	WORD_BE	wbeCompression = 0 ;
	if ( m_pfile->Write( &wbeCompression, sizeof(WORD_BE) ) < sizeof(WORD_BE) )
	{
		imgBuf.UnlockBuffer() ;
		return	errFailed ;
	}
	//
	// 各チャネル
	//
	size_t	nChOffset[4] =
	{
		2, 1, 0, 3,
	} ;
	if ( (imginf.format & formatImageColorSpaceMask) == formatImageBGR )
	{
		nChOffset[0] = 0 ;
		nChOffset[1] = 1 ;
		nChOffset[2] = 2 ;
	}
	for ( size_t iChannel = 0; iChannel < nChannels; iChannel ++ )
	{
		// 各ライン
		for ( uint32_t y = 0; y < nHeight; y ++ )
		{
			uint8_t *	pbytSrcLine = pbytPixels + (int32_t) y * imginf.pitchLine
													+ nChOffset[iChannel] ;
			uint8_t *	pbytDstLine = bufLineBuf.GetArray() ;
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				pbytDstLine[x] = *pbytSrcLine ;
				pbytSrcLine += imginf.pitchPixel ;
			}
			m_pfile->Write( pbytDstLine, nWidth ) ;
		}
	}
	imgBuf.UnlockBuffer() ;
	//
	// 参照ポインタ
	//
	int64_t	fpSavePos = m_pfile->GetPosition() ;
	m_pfile->Seek( m_fpLayerMaskInfo ) ;
	//
	DWORD_BE	dwLength = (uint32_t) (fpFilePos - (m_fpLayerMaskInfo + sizeof(DWORD_BE))) ;
	m_pfile->Write( &dwLength, sizeof(DWORD_BE) ) ;
	//
	m_pfile->Seek( fpSavePos ) ;
	return	errSuccess ;
}



// PSD ファイル書き込みリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( PSD::FileWriteListener )

// 進行状況
//////////////////////////////////////////////////////////////////////////////
SSystem::SError FileWriteListener::OnWriteLayer
		( FileWriter * pfr,
			FileWriter::LayerInfo * pli, size_t iLayer, size_t nLayerCount )
{
	return	errSuccess ;
}

SSystem::SError FileWriteListener::OnFinishedLayer
	( FileWriter * pfr,
		FileWriter::LayerInfo * pli, size_t iLayer, size_t nLayerCount )
{
	return	errSuccess ;
}


