
/*****************************************************************************
                  Adobe(R) Photoshop(R) ファイルローダー
 *****************************************************************************/

#include <gls.h>
#include <psdfile.h>


using	namespace PSD ;

//////////////////////////////////////////////////////////////////////////////
// PSD ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( PSD::File, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
File::File( void )
{
	m_pfile = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
File::~File( void )
{
	Close( ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError File::Open( ESLFileObject & file )
{
	ESLError	errCannotRead = ESLErrorMsg( "読み込みに失敗しました。" ) ;
	//
	// 以前のデータを破棄
	//
	Close( ) ;
	//
	// ファイルヘッダを読み込む
	//
	if ( file.Read( &m_fhHeader, sizeof(m_fhHeader) ) < sizeof(m_fhHeader) )
	{
		return	ESLErrorMsg( "ファイルヘッダを読み込めませんでした。" ) ;
	}
	if ( dwswap('8BPS') != m_fhHeader.dwSignature )
	{
		return	ESLErrorMsg( "ファイルシグネチャが一致しません。" ) ;
	}
	//
	// Color Mode Data を読み込む
	//
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errCannotRead ;
	}
	if ( dwLength > 0 )
	{
		dwLength = dwswap( dwLength ) ;
		file.Read( m_bufColorMode.PutBuffer(dwLength), dwLength ) ;
		m_bufColorMode.Flush( dwLength ) ;
	}
	//
	// Image Resource Section を読み飛ばす
	//
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errCannotRead ;
	}
	m_dwLayerDataPos = file.GetPosition() + dwswap(dwLength) ;
	if ( file.Seek( m_dwLayerDataPos, file.FromBegin ) != m_dwLayerDataPos )
	{
		return	errCannotRead ;
	}
	//
	// Layer Mask Information Section を読み込む
	//
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errCannotRead ;
	}
	m_dwBaseDataPos = file.GetPosition() + dwswap(dwLength) ;
	if ( dwLength != 0 )
	{
		if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	errCannotRead ;
		}
		WORD	wLayerCount ;
		if ( file.Read( &wLayerCount, sizeof(WORD) ) < sizeof(WORD) )
		{
			return	errCannotRead ;
		}
		int		nLayerCount = (SWORD) wswap( wLayerCount ) ;
		if ( nLayerCount <= 0 )
		{
			nLayerCount = - nLayerCount ;
		}
		for ( int i = 0; i < nLayerCount; i ++ )
		{
			LAYER_RECORD *	plrLayer = new LAYER_RECORD ;
			m_lstLayer.Add( plrLayer ) ;
			//
			// レイヤー領域（矩形）、及びチャネル数
			//
			if ( file.Read( plrLayer, sizeof(DWORD) * 4 + sizeof(WORD) )
										< sizeof(DWORD) * 4 + sizeof(WORD) )
			{
				return	errCannotRead ;
			}
			//
			// チャネル長情報、及び残りのデータを読み込む
			//
			if ( wswap(plrLayer->wChannels) > 24 )
			{
				return	errCannotRead ;
			}
			dwLength =
				sizeof(CHANNEL_LENGTH_INFO) * wswap(plrLayer->wChannels) ;
			if ( file.Read( plrLayer->chlen, dwLength ) < dwLength )
			{
				return	errCannotRead ;
			}
			dwLength =
				(sizeof(DWORD) * 2 + sizeof(BYTE) * 4 + sizeof(DWORD)) ;
			if ( file.Read( &(plrLayer->dwSignature), dwLength ) < dwLength )
			{
				return	errCannotRead ;
			}
			//
			// 拡張情報を読み込む
			//
			EStreamBuffer	bufExtra ;
			dwLength = dwswap( plrLayer->dwExtraSize ) ;
			file.Read( bufExtra.PutBuffer( dwLength ), dwLength ) ;
			bufExtra.Flush( dwLength ) ;
			//
			// Adjustment Layer Data
			//
			bufExtra.Read( &(plrLayer->adjdata.dwSize), sizeof(DWORD) ) ;
			dwLength = dwswap( plrLayer->adjdata.dwSize ) ;
			if ( dwLength != 0 )
			{
				DWORD	dwReadBytes = dwLength ;
				if ( dwReadBytes > sizeof(ADJUST_LAYER_DATA) - sizeof(DWORD) )
				{
					dwReadBytes = sizeof(ADJUST_LAYER_DATA) - sizeof(DWORD) ;
				}
				bufExtra.Read( &(plrLayer->adjdata.dwTop), dwReadBytes ) ;
				//
				if ( dwLength > dwReadBytes )
				{
					bufExtra.GetBuffer( dwLength - dwReadBytes ) ;
					bufExtra.Release( dwLength - dwReadBytes ) ;
				}
			}
			//
			// Layer Blending ranges data
			//
			dwLength = 0 ;
			bufExtra.Read( &dwLength, sizeof(DWORD) ) ;
			dwLength = dwswap( dwLength ) ;
			bufExtra.GetBuffer( dwLength ) ;
			bufExtra.Release( dwLength ) ;
			//
			// Layer name
			//
			EPtrBuffer	ptrbuf = bufExtra.GetBuffer( ) ;
			if ( ptrbuf.GetLength() > 2 )
			{
				const BYTE *	pchName = (const BYTE *) ptrbuf.GetBuffer() ;
				if ( ((int) pchName[0] + 1) <= (int) ptrbuf.GetLength() )
				{
					plrLayer->strLayerName =
						EString( (const char *) pchName + 1, (int) pchName[0] ) ;
				}
				bufExtra.Release( ((int) pchName[0] + 1 + 0x03) & ~0x03 ) ;
				//
				// Unicode レイヤー名チャンク検索
				//
				for ( ; ; )
				{
					DWORD	dwChunkHeader[3] ;
					if ( bufExtra.Read
						( dwChunkHeader,
							sizeof(DWORD) * 3 ) < sizeof(DWORD) * 3 )
					{
						break ;
					}
					if ( dwChunkHeader[0] != 0x4D494238 /*'8BIM'*/ )
					{
						break ;
					}
					dwLength = dwswap(dwChunkHeader[2]) ;
					if ( dwChunkHeader[1] == 0x696E756C /*'luni'*/ )
					{
						if ( bufExtra.Read
							( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
						{
							break ;
						}
						dwLength = dwswap(dwLength) ;
						//
						ptrbuf = bufExtra.GetBuffer( dwLength * sizeof(WORD) ) ;
						if ( ptrbuf.GetLength() < dwLength * sizeof(WORD) )
						{
							break ;
						}
						EWideString	wstrBuf ;
						wchar_t *	pwBuf = wstrBuf.GetBuffer( dwLength ) ;
						WORD *		pwSrc = (WORD*) ptrbuf.GetBuffer() ;
						for ( DWORD j = 0; j < dwLength; j ++ )
						{
							pwBuf[j] = wswap( pwSrc[j] ) ;
						}
						wstrBuf.ReleaseBuffer( dwLength ) ;
						plrLayer->strLayerName = wstrBuf ;
						break ;
					}
					ptrbuf = bufExtra.GetBuffer( dwLength ) ;
					bufExtra.Release( dwLength ) ;
				}
			}
		}
	}
	//
	// 完了
	//
	m_dwLayerDataPos = file.GetPosition( ) ;
	m_pfile = &file ;
	//
	return	eslErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void File::Close( void )
{
	m_bufColorMode.Delete( ) ;
	m_lstLayer.RemoveAll( ) ;
	m_bufLayerInfo.Delete( ) ;
	m_lstLayerData.RemoveAll( ) ;
	m_bufBaseData.Delete( ) ;
	m_pfile = NULL ;
}

// ヘッダ情報を取得する
//////////////////////////////////////////////////////////////////////////////
void File::GetFileHeader( FILE_HEADER & fhHeader ) const
{
	fhHeader = m_fhHeader ;
	fhHeader.wVersion = wswap( fhHeader.wVersion ) ;
	fhHeader.wChannels = wswap( fhHeader.wChannels ) ;
	fhHeader.dwHeight = dwswap( fhHeader.dwHeight ) ;
	fhHeader.dwWidth = dwswap( fhHeader.dwWidth ) ;
	fhHeader.wDepth = wswap( fhHeader.wDepth ) ;
	fhHeader.wMode = wswap( fhHeader.wMode ) ;
}

// カラーモード情報を取得する
//////////////////////////////////////////////////////////////////////////////
void File::GetColorModeData( EStreamBuffer bufColorMode ) const
{
	EPtrBuffer	ptrbuf = ((File*)this)->m_bufColorMode.GetBuffer() ;
	bufColorMode.Write( ptrbuf, ptrbuf.GetLength() ) ;
	((File*)this)->m_bufColorMode.Release( 0 ) ;
}

// レイヤーの総数を取得する
//////////////////////////////////////////////////////////////////////////////
unsigned int File::GetLayerCount( void ) const
{
	return	m_lstLayer.GetSize() ;
}

// レイヤー情報を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError File::GetLayerInfo
	( File::LayerInfo & liLayer, unsigned int iLayer ) const
{
	LAYER_RECORD *	plrLayer = m_lstLayer.GetAt( iLayer ) ;
	if ( plrLayer == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	liLayer.irRect.x = (int) dwswap( plrLayer->dwLeft ) ;
	liLayer.irRect.y = (int) dwswap( plrLayer->dwTop ) ;
	liLayer.irRect.w = (int) dwswap( plrLayer->dwRight ) - liLayer.irRect.x ;
	liLayer.irRect.h = (int) dwswap( plrLayer->dwBottom ) - liLayer.irRect.y ;
	//
	if ( plrLayer->adjdata.dwSize != 0 )
	{
		liLayer.irMask.x = (int) dwswap( plrLayer->adjdata.dwLeft ) ;
		liLayer.irMask.y = (int) dwswap( plrLayer->adjdata.dwTop ) ;
		liLayer.irMask.w =
			(int) dwswap( plrLayer->adjdata.dwRight ) - liLayer.irMask.x ;
		liLayer.irMask.h =
			(int) dwswap( plrLayer->adjdata.dwBottom ) - liLayer.irMask.y ;
	}
	else
	{
		liLayer.irMask = liLayer.irRect ;
	}
	//
	liLayer.nChannels = wswap( plrLayer->wChannels ) ;
	for ( int i = 0; i < liLayer.nChannels; i ++ )
	{
		liLayer.chlen[i].wChannelID = wswap( plrLayer->chlen[i].wChannelID ) ;
		liLayer.chlen[i].dwLength = dwswap( plrLayer->chlen[i].dwLength ) ;
	}
	liLayer.dwBlendMode = plrLayer->dwBlendMode ;
	liLayer.nTransparency =
		0x100 - ((int) plrLayer->bytOpacity + (plrLayer->bytOpacity >> 7)) ;
	liLayer.dwFlags = plrLayer->bytFlags ;
	liLayer.dwDefColor = plrLayer->adjdata.bytDefColor ;
	liLayer.strLayerName = plrLayer->strLayerName ;
	//
	return	eslErrSuccess ;
}

// レイヤー画像を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError File::LoadLayerImage
	( EGLImage & imgBuf, unsigned int iLayer )
{
	if ( m_pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// データのファイル上のアドレスを計算する
	//
	DWORD		dwDataPos ;
	LayerInfo	liLayer ;
	if ( iLayer == (unsigned int) -1 )
	{
		return	LoadBaseImage( imgBuf ) ;
	}
	else
	{
		dwDataPos = m_dwLayerDataPos ;
		for ( unsigned int i = 0; i < iLayer; i ++ )
		{
			if ( GetLayerInfo( liLayer, i ) )
			{
				return	eslErrGeneral ;
			}
			for ( int j = 0; j < liLayer.nChannels; j ++ )
			{
				dwDataPos += liLayer.chlen[j].dwLength ;
			}
		}
		if ( GetLayerInfo( liLayer, iLayer ) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// 画像バッファの生成
	//
	DWORD	fdwFormat, dwBitsPerPixel, dwDefLineBytes, dwPixelStep ;
	if ( liLayer.nChannels < 3 )
	{
		if ( liLayer.nChannels <= 0 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_GRAY_BITMAP ;
		dwBitsPerPixel = wswap( m_fhHeader.wDepth ) ;
		dwDefLineBytes = (liLayer.irRect.w * dwBitsPerPixel + 0x07) >> 3 ;
		dwPixelStep = 1 ;
	}
	else if ( liLayer.nChannels == 3 )
	{
		if ( wswap( m_fhHeader.wDepth ) != 8 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_RGB_BITMAP ;
		dwDefLineBytes = liLayer.irRect.w ;
		dwBitsPerPixel = 32 ;
		dwPixelStep = 4 ;
	}
	else if ( liLayer.nChannels >= 4 )
	{
		if ( wswap( m_fhHeader.wDepth ) != 8 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_RGBA_BITMAP ;
		dwDefLineBytes = liLayer.irRect.w ;
		dwBitsPerPixel = 32 ;
		dwPixelStep = 4 ;
	}
	PEGL_IMAGE_INFO	pImage =
		imgBuf.CreateImage
			( fdwFormat, liLayer.irRect.w, liLayer.irRect.h, dwBitsPerPixel ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	imgBuf.ReverseVertically( ) ;
	//
	// 各チャネルを展開
	//
	bool	fAlphaFlag = false ;
	for ( int iChannel = 0; iChannel < liLayer.nChannels; iChannel ++ )
	{
		//
		// ファイルのシーク
		//
		if ( m_pfile->Seek
			( dwDataPos, ESLFileObject::FromBegin ) != dwDataPos )
		{
			return	eslErrGeneral ;
		}
		dwDataPos += liLayer.chlen[iChannel].dwLength ;
		//
		// 圧縮方式取得
		//
		WORD	wCompression ;
		if ( m_pfile->Read( &wCompression, sizeof(WORD) ) < sizeof(WORD) )
		{
			return	eslErrGeneral ;
		}
		wCompression = wswap( wCompression ) ;
		//
		// 各行のバイト数を取得
		//
		EStreamBuffer	bufLineBuf ;
		EStreamBuffer	bufLineSize ;
		EStreamBuffer	bufRLE ;
		WORD *			pwLineSize = NULL ;
		BYTE *			ptrRLE = NULL ;
		int				iRLE = 0 ;
		DWORD			dwTotalBytes = 0 ;
		DWORD			dwLineBytes ;
		EGL_IMAGE_RECT	irRect ;
		//
		if ( liLayer.chlen[iChannel].wChannelID
						== (WORD) chidUserSuppliedMask )
		{
			irRect = liLayer.irMask ;
			irRect.x -= liLayer.irRect.x ;
			irRect.y -= liLayer.irRect.y ;
			if ( liLayer.nChannels < 3 )
			{
				dwLineBytes = (irRect.w * dwBitsPerPixel + 0x07) >> 3 ;
			}
			else
			{
				dwLineBytes = irRect.w ;
			}
			if ( irRect.x + irRect.w > liLayer.irRect.w )
			{
				irRect.w = liLayer.irRect.w - irRect.x ;
				if ( irRect.w < 0 )
				{
					irRect.w = 0 ;
				}
			}
			if ( irRect.y + irRect.h > liLayer.irRect.h )
			{
				irRect.h = liLayer.irRect.h - irRect.y ;
				if ( irRect.h < 0 )
				{
					irRect.h = 0 ;
				}
			}
		}
		else
		{
			irRect.x = 0 ;
			irRect.y = 0 ;
			irRect.w = liLayer.irRect.w ;
			irRect.h = liLayer.irRect.h ;
			dwLineBytes = dwDefLineBytes ;
		}
		BYTE *			ptrLineBuf =
			(BYTE*) bufLineBuf.PutBuffer( dwLineBytes ) ;
		//
		if ( wCompression == 1 )	// RLE
		{
			DWORD	dwBytes = irRect.h * sizeof(WORD) ;
			pwLineSize = (WORD*) bufLineSize.PutBuffer( dwBytes ) ;
			if ( m_pfile->Read( pwLineSize, dwBytes ) < dwBytes )
			{
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < irRect.h; i ++ )
			{
				pwLineSize[i] = wswap( pwLineSize[i] ) ;
				dwTotalBytes += pwLineSize[i] ;
			}
			ptrRLE = (BYTE*) bufRLE.PutBuffer( dwTotalBytes ) ;
			if ( m_pfile->Read( ptrRLE, dwTotalBytes ) < dwTotalBytes )
			{
				return	eslErrGeneral ;
			}
		}
		else if ( wCompression != 0 )
		{
			return	eslErrGeneral ;
		}
		//
		// 各ラインを順次読み込み・展開
		//
		for ( int y = 0; y < irRect.h; y ++ )
		{
			if ( wCompression == 0 )	// Non-compress
			{
				if ( m_pfile->Read( ptrLineBuf, dwLineBytes ) < dwLineBytes )
				{
					return	eslErrGeneral ;
				}
			}
			else						// RLE
			{
				UnpackBits
					( ptrLineBuf, (int) dwLineBytes,
						ptrRLE + iRLE, pwLineSize[y] ) ;
				iRLE += pwLineSize[y] ;
			}
			//
			BYTE *	ptrDstLine = (BYTE*) pImage->ptrImageArray ;
			if ( y + irRect.y < 0 )
			{
				continue ;
			}
			ptrDstLine += pImage->dwBytesPerLine * (y + irRect.y) ;
			//
			int		i ;
			bool	fProduct = false ;
			switch ( liLayer.chlen[iChannel].wChannelID )
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
			if ( i * 8 > (int) dwBitsPerPixel )
			{
				i = 0 ;
			}
			i += dwPixelStep * irRect.x ;
			//
			if ( fProduct )
			{
				int	w = irRect.w ;
				if ( irRect.x < 0 )
				{
					w += irRect.x ;
					i -= dwPixelStep * irRect.x ;
				}
				for ( int x = 0; x < w; x ++ )
				{
					ptrDstLine[i] =
						(BYTE) (((int) ptrDstLine[i]
								* ((int) ptrLineBuf[x] + 1)) >> 8) ;
					i += dwPixelStep ;
				}
			}
			else
			{
				for ( int x = 0; x < irRect.w; x ++ )
				{
					ptrDstLine[i] = ptrLineBuf[x] ;
					i += dwPixelStep ;
				}
			}
		}
		if ( (liLayer.chlen[iChannel].wChannelID == (WORD) chidUserSuppliedMask)
											&& (liLayer.dwDefColor == 0) )
		{
			//
			// ユーザーマスクチャネルの領域外の処理
			//
			for ( int y = 0; y < (int) pImage->dwImageHeight; y ++ )
			{
				BYTE *	ptrDstLine = (BYTE*) pImage->ptrImageArray ;
				ptrDstLine += pImage->dwBytesPerLine * y ;
				//
				if ( (y < irRect.y) || (irRect.y + irRect.h <= y) )
				{
					for ( int x = 0; x < (int) pImage->dwImageWidth; x ++ )
					{
						ptrDstLine[3] = 0 ;
						ptrDstLine += dwPixelStep ;
					}
				}
				else
				{
					for ( int x = 0; x < (int) pImage->dwImageWidth; x ++ )
					{
						if ( (x < irRect.x) || (irRect.x + irRect.w <= x) )
						{
							ptrDstLine[3] = 0 ;
						}
						ptrDstLine += dwPixelStep ;
					}
				}
			}
		}
		switch ( liLayer.chlen[iChannel].wChannelID )
		{
		case	chidRed:
		case	chidGreen:
		case	chidBlue:
			break ;
		default:
			fAlphaFlag = true ;
			break ;
		}
	}
	//
	return	eslErrSuccess ;
}

// ベース画像を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError File::LoadBaseImage( EGLImage & imgBuf )
{
	if ( m_pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// ヘッダ情報取得
	//
	FILE_HEADER	fhHeader ;
	GetFileHeader( fhHeader ) ;
	//
	// 画像バッファの生成
	//
	DWORD	fdwFormat, dwBitsPerPixel, dwLineBytes, dwPixelStep ;
	int		nChannels = (int) fhHeader.wChannels ;
	if ( nChannels < 3 )
	{
		if ( nChannels <= 0 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_GRAY_BITMAP ;
		dwBitsPerPixel = fhHeader.wDepth ;
		dwLineBytes = (fhHeader.dwWidth * dwBitsPerPixel + 0x07) >> 3 ;
		dwPixelStep = 1 ;
	}
	else if ( nChannels == 3 )
	{
		if ( fhHeader.wDepth != 8 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_RGB_BITMAP ;
		dwLineBytes = fhHeader.dwWidth ;
		dwBitsPerPixel = 32 ;
		dwPixelStep = 4 ;
	}
	else if ( nChannels >= 4 )
	{
		if ( fhHeader.wDepth != 8 )
		{
			return	eslErrGeneral ;
		}
		fdwFormat = EIF_RGBA_BITMAP ;
		dwLineBytes = fhHeader.dwWidth ;
		dwBitsPerPixel = 32 ;
		dwPixelStep = 4 ;
	}
	PEGL_IMAGE_INFO	pImage =
		imgBuf.CreateImage
			( fdwFormat, fhHeader.dwWidth, fhHeader.dwHeight, dwBitsPerPixel ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	imgBuf.ReverseVertically( ) ;
	//
	// ファイルのシーク
	//
	if ( m_pfile->Seek
		( m_dwBaseDataPos, ESLFileObject::FromBegin ) != m_dwBaseDataPos )
	{
		return	eslErrGeneral ;
	}
	//
	// 圧縮方式取得
	//
	WORD	wCompression ;
	if ( m_pfile->Read( &wCompression, sizeof(WORD) ) < sizeof(WORD) )
	{
		return	eslErrGeneral ;
	}
	wCompression = wswap( wCompression ) ;
	//
	// 各行のバイト数を取得
	//
	EStreamBuffer	bufLineBuf ;
	EStreamBuffer	bufLineSize ;
	EStreamBuffer	bufRLE ;
	BYTE *	ptrLineBuf = (BYTE*) bufLineBuf.PutBuffer( dwLineBytes ) ;
	WORD *	pwLineSize = NULL ;
	BYTE *	ptrRLE = NULL ;
	int		iRLE = 0 ;
	DWORD	dwTotalBytes = 0 ;
	int		nTotalLines = (int) fhHeader.dwHeight * nChannels ;
	//
	if ( wCompression == 1 )	// RLE
	{
		DWORD	dwBytes = nTotalLines * sizeof(WORD) ;
		pwLineSize = (WORD*) bufLineSize.PutBuffer( dwBytes ) ;
		if ( m_pfile->Read( pwLineSize, dwBytes ) < dwBytes )
		{
			return	eslErrGeneral ;
		}
		for ( int i = 0; i < nTotalLines; i ++ )
		{
			pwLineSize[i] = wswap( pwLineSize[i] ) ;
			dwTotalBytes += pwLineSize[i] ;
		}
		ptrRLE = (BYTE*) bufRLE.PutBuffer( dwTotalBytes ) ;
		if ( m_pfile->Read( ptrRLE, dwTotalBytes ) < dwTotalBytes )
		{
			return	eslErrGeneral ;
		}
	}
	else if ( wCompression != 0 )
	{
		return	eslErrGeneral ;
	}
	//
	// 各チャネルを展開
	//
	for ( int iChannel = 0; iChannel < nChannels; iChannel ++ )
	{
		//
		// 各ラインを順次読み込み
		//
		for ( int y = 0; y < (int) fhHeader.dwHeight; y ++ )
		{
			if ( wCompression == 0 )	// Non-compress
			{
				if ( m_pfile->Read( ptrLineBuf, dwLineBytes ) < dwLineBytes )
				{
					return	eslErrGeneral ;
				}
			}
			else						// RLE
			{
				int	i = y + iChannel * fhHeader.dwHeight ;
				UnpackBits
					( ptrLineBuf, (int) dwLineBytes,
						ptrRLE + iRLE, pwLineSize[i] ) ;
				iRLE += pwLineSize[i] ;
			}
			//
			BYTE *	ptrDstLine = (BYTE*) pImage->ptrImageArray ;
			ptrDstLine += pImage->dwBytesPerLine * y ;
			//
			int		i ;
			if ( iChannel < 3 )
			{
				i = 2 - iChannel ;
			}
			else
			{
				i = 3 ;
			}
			if ( i * 8 > (int) dwBitsPerPixel )
			{
				i = 0 ;
			}
			if ( iChannel < 4 )
			{
				for ( int x = 0; x < (int) fhHeader.dwWidth; x ++ )
				{
					ptrDstLine[i] = ptrLineBuf[x] ;
					i += dwPixelStep ;
				}
			}
			else
			{
				for ( int x = 0; x < (int) fhHeader.dwWidth; x ++ )
				{
					ptrDstLine[i] =
						(BYTE) (((int) ptrDstLine[i]
								* ((int) ptrLineBuf[x] + 1)) >> 8) ;
					i += dwPixelStep ;
				}
			}
		}
	}
	//
	return	eslErrSuccess ;
}

// RLE の展開
//////////////////////////////////////////////////////////////////////////////
void File::UnpackBits
	( BYTE * ptrBuf, int nBufSize, const BYTE * ptrRLE, int nRLESize )
{
	int	i = 0, j = 0 ;
	while ( (j < nBufSize) && (i + 2 <= nRLESize) )
	{
		int		nLength ;
		if ( ptrRLE[i]  & 0x80 )
		{
			BYTE	bytFill ;
			nLength = 1 - (SBYTE) ptrRLE[i ++] ;
			bytFill = ptrRLE[i ++] ;
			if ( j + nLength > nBufSize )
			{
				nLength = nBufSize - j ;
			}
			int	k ;
			for ( k = 0; k < nLength; k ++ )
			{
				ptrBuf[j + k] = bytFill ;
			}
			j += k ;
		}
		else
		{
			nLength = ptrRLE[i ++] + 1 ;
			if ( j + nLength > nBufSize )
			{
				nLength = nBufSize - j ;
			}
			if ( i + nLength > nRLESize )
			{
				nLength = nRLESize - i ;
			}
			for ( int k = 0; k < nLength; k ++ )
			{
				ptrBuf[j + k] = ptrRLE[i + k] ;
			}
			i += nLength ;
			j += nLength ;
		}
	}
}

// PSD ファイル書き出しの準備を始める
//////////////////////////////////////////////////////////////////////////////
ESLError File::PrepareToWrite( ESLFileObject & file, PCEGL_IMAGE_INFO pBase )
{
	ESLError	errFaildToWrite =
		ESLErrorMsg( "ファイルへの書き出しに失敗しました" ) ;
	//
	// 以前のデータを破棄
	//
	Close( ) ;
	//
	// ファイルを関連付けてファイルヘッダを準備する
	//
	if ( pBase == NULL )
	{
		return	ESLErrorMsg( "ベース画像がありません" ) ;
	}
	m_pfile = &file ;
	m_fhHeader.dwSignature = dwswap( '8BPS' ) ;
	m_fhHeader.wVersion = wswap( 1 ) ;
	::eslFillMemory
		( m_fhHeader.bytReserved, 0, sizeof(m_fhHeader.bytReserved) ) ;
	switch ( pBase->dwBitsPerPixel )
	{
	case	32:
		if ( pBase->fdwFormatType & EIF_WITH_ALPHA )
		{
			m_fhHeader.wChannels = wswap( 4 ) ;
			m_fhHeader.wMode = wswap( modeRGB ) ;
			break ;
		}
	case	24:
		m_fhHeader.wChannels = wswap( 3 ) ;
		m_fhHeader.wMode = wswap( modeRGB ) ;
		break; 
	case	8:
		m_fhHeader.wChannels = wswap( 1 ) ;
		if ( pBase->fdwFormatType == EIF_GRAY_BITMAP )
		{
			m_fhHeader.wMode = wswap( modeGrayscale ) ;
		}
		else
		{
			m_fhHeader.wMode = wswap( modeIndexed ) ;
		}
		break; 
	default:
		return	ESLErrorMsg( "対応していない画像フォーマットです" ) ;
	}
	m_fhHeader.dwHeight = dwswap( pBase->dwImageHeight ) ;
	m_fhHeader.dwWidth = dwswap( pBase->dwImageWidth ) ;
	m_fhHeader.wDepth = wswap( 8 ) ;
	//
	if ( m_pfile->Write
		( &m_fhHeader, sizeof(m_fhHeader) ) < sizeof(m_fhHeader) )
	{
		return	errFaildToWrite ;
	}
	//
	// Color Mode Data を書き出す
	//
	DWORD	dwLength = 0 ;
	if ( m_pfile->Write
		( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	errFaildToWrite ;
	}
	//
	// Image Resource Section を書き出す
	//
	static const char	cPascalStrResolution[] = "Resolution\0" ;
	static const char	cPascalStrFXGA[] = "FX Global Altitude\0" ;
	BYTE	bytStrLenResolution = (BYTE) strlen(cPascalStrResolution) ;
	BYTE	bytStrLenFXGA = (BYTE) strlen(cPascalStrFXGA) ;
	ULONG	nPaddingLenResolution =
							((bytStrLenResolution + 2) & ~0x01) - 1 ;
	ULONG	nPaddingLenFXGA = ((bytStrLenFXGA + 2) & ~0x01) - 1 ;
	//
	dwLength =
		dwswap( sizeof(DWORD) + sizeof(WORD)
				+ nPaddingLenResolution + 1
				+ sizeof(DWORD) + sizeof(ResolutionInfo)
				+ sizeof(DWORD) + sizeof(WORD)
				+ nPaddingLenFXGA + 1
				+ sizeof(DWORD) + sizeof(DWORD) ) ;
	if ( m_pfile->Write
		( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	errFaildToWrite ;
	}
	//
	// Resolution
	//
	DWORD	dwSignature = dwswap( '8BIM' ) ;
	WORD	wID = wswap( 0x3ED ) ;
	if ( m_pfile->Write
		( &dwSignature, sizeof(dwSignature) ) < sizeof(dwSignature) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write( &wID, sizeof(wID) ) < sizeof(wID) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( &bytStrLenResolution,
			sizeof(bytStrLenResolution) ) < sizeof(bytStrLenResolution) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( cPascalStrResolution,
				nPaddingLenResolution ) < nPaddingLenResolution )
	{
		return	errFaildToWrite ;
	}
	ResolutionInfo	rsiInfo ;
	dwLength = dwswap( sizeof(ResolutionInfo) ) ;
	rsiInfo.fxHorzRes = dwswap( 72 * 0x10000 ) ;
	rsiInfo.wHorzResUnit = wswap( runitPixelPerInch ) ;
	rsiInfo.wWidthUnit = wswap( dunitCM ) ;
	rsiInfo.fxVertRes = dwswap( 72 * 0x10000 ) ;
	rsiInfo.wVertResUnit = wswap( runitPixelPerInch ) ;
	rsiInfo.wHeightUnit = wswap( dunitCM ) ;
	if ( m_pfile->Write
		( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( &rsiInfo, sizeof(rsiInfo) ) < sizeof(rsiInfo) )
	{
		return	errFaildToWrite ;
	}
	//
	// FX Global Altitude 
	//
	wID = wswap( 0x419 ) ;
	//
	if ( m_pfile->Write
		( &dwSignature, sizeof(dwSignature) ) < sizeof(dwSignature) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write( &wID, sizeof(wID) ) < sizeof(wID) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( &bytStrLenFXGA,
			sizeof(bytStrLenFXGA) ) < sizeof(bytStrLenFXGA) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( cPascalStrFXGA, nPaddingLenFXGA ) < nPaddingLenFXGA )
	{
		return	errFaildToWrite ;
	}
	DWORD	dwFXGlobalAltitude = dwswap( 0x1E ) ;
	dwLength = dwswap( sizeof(dwFXGlobalAltitude) ) ;
	if ( m_pfile->Write
		( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	errFaildToWrite ;
	}
	if ( m_pfile->Write
		( &dwFXGlobalAltitude,
			sizeof(dwFXGlobalAltitude) ) < sizeof(dwFXGlobalAltitude) )
	{
		return	errFaildToWrite ;
	}
	//
	// ベース画像を圧縮する
	//
	return	PackBaseImage( m_bufBaseData, pBase ) ;
}

// レイヤーを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError File::AddLayerImage
	( PCEGL_IMAGE_INFO pLayer,
		const char * pszName, int xPos, int yPos,
		unsigned int nTransparency, DWORD dwBlendMode )
{
	if ( pLayer == NULL )
	{
		return	ESLErrorMsg( "レイヤー画像がありません" ) ;
	}
	//
	// レイヤーレコード生成
	//
	LAYER_RECORD *	plrLayer = new LAYER_RECORD ;
	//
	plrLayer->dwTop = dwswap( yPos ) ;
	plrLayer->dwLeft = dwswap( xPos ) ;
	plrLayer->dwBottom = dwswap( yPos + pLayer->dwImageHeight ) ;
	plrLayer->dwRight = dwswap( xPos + pLayer->dwImageWidth ) ;
	plrLayer->wChannels = wswap( (WORD) (pLayer->dwBitsPerPixel / 8) ) ;
	//
	::eslFillMemory( plrLayer->chlen, 0, sizeof(plrLayer->chlen) ) ;
	//
	plrLayer->dwSignature = dwswap( '8BIM' ) ;
	plrLayer->dwBlendMode = dwswap( dwBlendMode ) ;
	plrLayer->bytOpacity =
		(BYTE) (0xFF - (nTransparency - (nTransparency >> 7))) ;
	plrLayer->bytClipping = 0 ;
	plrLayer->bytFlags = 0 ;
	plrLayer->bytFilter = 0 ;
	//
	::eslFillMemory( &(plrLayer->adjdata), 0, sizeof(plrLayer->adjdata) ) ;
/*	plrLayer->adjdata.dwSize =
		dwswap( sizeof(ADJUST_LAYER_DATA) - sizeof(DWORD) ) ;
	plrLayer->adjdata.dwTop = dwswap( plrLayer->dwTop ) ;
	plrLayer->adjdata.dwLeft = dwswap( plrLayer->dwLeft ) ;
	plrLayer->adjdata.dwBottom = dwswap( plrLayer->dwBottom ) ;
	plrLayer->adjdata.dwRight = dwswap( plrLayer->dwRight ) ;
	plrLayer->adjdata.bytDefColor = 0 ;
	plrLayer->adjdata.Flags = 0 ;
	plrLayer->adjdata.wPadding = 0 ;
*/	//
	if ( pszName != NULL )
	{
		plrLayer->strLayerName = pszName ;
	}
	else
	{
		plrLayer->strLayerName =
			"Layer " + EString( (int) m_lstLayer.GetSize() + 1 ) ;
	}
	if ( plrLayer->strLayerName.GetLength() >= 0x100 )
	{
		plrLayer->strLayerName = plrLayer->strLayerName.Left( 0xFF ) ;
	}
	int	nExtraSize =
		sizeof(DWORD) * 2 + 0x28 + plrLayer->strLayerName.GetLength() + 2 ;
	plrLayer->dwExtraSize = dwswap( (nExtraSize + 1) & ~0x01 ) ;
	//
	// レイヤー画像圧縮
	//
	EStreamBuffer *	pbufLayer = new EStreamBuffer ;
	ESLError	err = PackLayerImage( *pbufLayer, *plrLayer, pLayer ) ;
	if ( err )
	{
		delete	plrLayer ;
		delete	pbufLayer ;
		return	err ;
	}
	m_lstLayer.Add( plrLayer ) ;
	m_lstLayerData.Add( pbufLayer ) ;
	//
	// レイヤー情報シリアル化
	//
	m_bufLayerInfo.Write( plrLayer, sizeof(DWORD) * 4 + sizeof(WORD) ) ;
	//
	DWORD	dwBytes =
		sizeof(CHANNEL_LENGTH_INFO) * wswap(plrLayer->wChannels) ;
	m_bufLayerInfo.Write( plrLayer->chlen, dwBytes ) ;
	//
	dwBytes = sizeof(DWORD) * 2 + sizeof(BYTE) * 4 + sizeof(DWORD) ;
	m_bufLayerInfo.Write( &(plrLayer->dwSignature), dwBytes ) ;
	//
	dwBytes = plrLayer->adjdata.dwSize + sizeof(DWORD) ;
	m_bufLayerInfo.Write( &(plrLayer->adjdata), dwBytes ) ;
	//
	DWORD	dwLength = dwswap( 0x28 ) ;
	m_bufLayerInfo.Write( &dwLength, sizeof(DWORD) ) ;
	DWORD	dwLayerBlendingRange = dwswap( 0x0000FFFF ) ;
	for ( int i = 0; i < 10; i ++ )
	{
		m_bufLayerInfo.Write
			( &dwLayerBlendingRange, sizeof(dwLayerBlendingRange) ) ;
	}
	//
	BYTE	bytNull = 0 ;
	dwLength = plrLayer->strLayerName.GetLength( ) ;
	m_bufLayerInfo.Write( &dwLength, sizeof(BYTE) ) ;
	m_bufLayerInfo.Write
		( plrLayer->strLayerName.CharPtr(),
			plrLayer->strLayerName.GetLength() ) ;
	m_bufLayerInfo.Write( &bytNull, sizeof(BYTE) ) ;
	if ( nExtraSize & 0x01 )
	{
		m_bufLayerInfo.Write( &bytNull, sizeof(BYTE) ) ;
	}
	//
	return	eslErrSuccess ;
}

// ファイルへの書き出しを終了する
//////////////////////////////////////////////////////////////////////////////
ESLError File::FinishToWrite( void )
{
	//
	// データサイズの計算
	//
	ESLError	errFaildToWrite = ESLErrorMsg( "書き出しに失敗しました" ) ;
	if ( m_pfile == NULL )
	{
		return	ESLErrorMsg( "書き出しの準備が出来ていません" ) ;
	}
	DWORD	dwLayerInfoSize = m_bufLayerInfo.GetLength() ;
	DWORD	dwLayerDataSize = 0 ;
	int		i, nLayerCount = m_lstLayerData.GetSize() ;
	if ( nLayerCount != (int) m_lstLayer.GetSize() )
	{
		return	ESLErrorMsg( "書き出し情報が不正です" ) ;
	}
	if ( nLayerCount != 0 )
	{
		dwLayerInfoSize += sizeof(WORD) ;
		dwLayerDataSize = dwLayerInfoSize ;
		for ( i = 0; i < nLayerCount; i ++ )
		{
			EStreamBuffer *	pbuf = m_lstLayerData.GetAt( i ) ;
			ESLAssert( pbuf != NULL ) ;
			dwLayerDataSize += pbuf->GetLength( ) ;
		}
	}
	//
	// レイヤー情報書き出し
	//
	DWORD	dwLength ;
	if ( dwLayerDataSize == 0 )
	{
		dwLength = dwswap( sizeof(DWORD) * 2 ) ;
	}
	else
	{
		dwLength = dwswap( dwLayerDataSize + sizeof(DWORD) ) ;
	}
	if ( m_pfile->Write( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFaildToWrite ;
	}
	if ( dwLayerDataSize == 0 )
	{
		const DWORD	dwDummy[2] = { 0, 0 } ;
		if ( m_pfile->Write( dwDummy, sizeof(dwDummy) ) < sizeof(dwDummy) )
		{
			return	errFaildToWrite ;
		}
	}
	if ( nLayerCount != 0 )
	{
		dwLength = dwswap( dwLayerDataSize ) ;
		if ( m_pfile->Write( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	errFaildToWrite ;
		}
		WORD	wLayerCount = wswap( (WORD) nLayerCount ) ;
		if ( m_pfile->Write( &wLayerCount, sizeof(WORD) ) < sizeof(WORD) )
		{
			return	errFaildToWrite ;
		}
		if ( m_bufLayerInfo.WriteToFile( *m_pfile ) )
		{
			return	errFaildToWrite ;
		}
		//
		// レイヤー画像を書き出す
		//
		m_dwLayerDataPos = m_pfile->GetPosition( ) ;
		for ( i = 0; i < nLayerCount; i ++ )
		{
			EStreamBuffer *	pbuf = m_lstLayerData.GetAt( i ) ;
			ESLAssert( pbuf != NULL ) ;
			if ( pbuf->WriteToFile( *m_pfile ) )
			{
				return	errFaildToWrite ;
			}
		}
	}
	//
	// ベース画像を書き出す
	//
	if ( m_bufBaseData.WriteToFile( *m_pfile ) )
	{
		return	errFaildToWrite ;
	}
	//
	// 書き出しのための情報をクリアする
	//
	m_lstLayerData.RemoveAll( ) ;
	//
	return	eslErrSuccess ;
}

// レイヤー画像を圧縮する
//////////////////////////////////////////////////////////////////////////////
ESLError File::PackLayerImage
	( EStreamBuffer & bufImage,
		LAYER_RECORD & lrLayer, PCEGL_IMAGE_INFO pLayer )
{
	if ( pLayer == NULL )
	{
		return	ESLErrorMsg( "レイヤー画像が指定されていません" ) ;
	}
	EStreamBuffer	bufBuf, bufRLE ;
	BYTE *	pbytImage = (BYTE*) bufBuf.PutBuffer( pLayer->dwImageWidth ) ;
	BYTE *	pbytRLE = (BYTE*) bufRLE.PutBuffer( pLayer->dwImageWidth * 2 ) ;
	int		iChannel, nChannelCount = wswap( lrLayer.wChannels ) ;
	//
	for ( iChannel = 0; iChannel < nChannelCount; iChannel ++ )
	{
		DWORD	dwChannelBytes = sizeof(WORD) ;
		WORD	wCompression = wswap( 1 ) ;			// RLE
		bufImage.Write( &wCompression, sizeof(WORD) ) ;
		//
		// 各ラインのバイト数領域を確保
		//
		DWORD	dwChBasePos = bufImage.GetLength( ) ;
		DWORD	dwBytes = pLayer->dwImageHeight * sizeof(WORD) ;
		bufImage.PutBuffer( dwBytes ) ;
		bufImage.Flush( dwBytes ) ;
		//
		// チャネルインデックスを取得する
		//
		int	iCh = iChannel ;
		if ( pLayer->fdwFormatType & EIF_WITH_ALPHA )
		{
			if ( iChannel == 0 )
			{
				iCh = nChannelCount - 1 ;
				lrLayer.chlen[iChannel].wChannelID
								= wswap( chidTransparencyMask ) ;
			}
			else if ( iChannel <= 3 )
			{
				iCh = 3 - iChannel ;
				lrLayer.chlen[iChannel].wChannelID = wswap( iChannel - 1 ) ;
			}
			else
			{
				iCh = iChannel - 1 ;
				lrLayer.chlen[iChannel].wChannelID = wswap( iChannel ) ;
			}
		}
		else if ( nChannelCount >= 3 )
		{
			if ( iChannel <= 2 )
			{
				iCh = 2 - iChannel ;
				lrLayer.chlen[iChannel].wChannelID = wswap( iChannel ) ;
			}
			else
			{
				iCh = iChannel ;
				lrLayer.chlen[iChannel].wChannelID = wswap( iChannel ) ;
			}
		}
		else
		{
			iCh = iChannel ;
			lrLayer.chlen[iChannel].wChannelID = wswap( iChannel ) ;
		}
		if ( iCh * 8 > (int) pLayer->dwBitsPerPixel )
		{
			iCh = 0 ;
		}
		//
		// 各ラインを順次圧縮
		//
		const BYTE *	pbytLineBuf = (const BYTE *) pLayer->ptrImageArray ;
		for ( int y = 0; y < (int) pLayer->dwImageHeight; y ++ )
		{
			//
			// 画像をサンプリング
			//
			DWORD	dwPixelBytes = (pLayer->dwBitsPerPixel + 0x07) >> 3 ;
			int		p = iCh ;
			for ( int x = 0; x < (int) pLayer->dwImageWidth; x ++ )
			{
				pbytImage[x] = pbytLineBuf[p] ;
				p += dwPixelBytes ;
			}
			pbytLineBuf += pLayer->dwBytesPerLine ;
			//
			// RLE で圧縮
			//
			int	nPackedBytes =
				PackBits( pbytRLE, pLayer->dwImageWidth * 2,
							pbytImage, pLayer->dwImageWidth ) ;
			//
			WORD *	pwLineBytes =
				(WORD*) bufImage.ModifyBuffer
					( dwChBasePos + y * sizeof(WORD), sizeof(WORD) ) ;
			*pwLineBytes = wswap( (WORD) nPackedBytes ) ;
			//
			bufImage.Write( pbytRLE, nPackedBytes ) ;
			dwChannelBytes += nPackedBytes ;
		}
		lrLayer.chlen[iChannel].dwLength = dwswap( dwChannelBytes ) ;
	}
	return	eslErrSuccess ;
}

// ベース画像を圧縮する
//////////////////////////////////////////////////////////////////////////////
ESLError File::PackBaseImage
	( EStreamBuffer & bufImage, PCEGL_IMAGE_INFO pBase )
{
	if ( pBase == NULL )
	{
		return	ESLErrorMsg( "ベース画像が指定されていません" ) ;
	}
	EStreamBuffer	bufBuf, bufRLE ;
	BYTE *	pbytImage = (BYTE*) bufBuf.PutBuffer( pBase->dwImageWidth ) ;
	BYTE *	pbytRLE = (BYTE*) bufRLE.PutBuffer( pBase->dwImageWidth * 2 ) ;
	//
	int		iChannel, nChannelCount ;
	if ( (pBase->dwBitsPerPixel == 32)
		&& (pBase->fdwFormatType & EIF_WITH_ALPHA) )
	{
		nChannelCount = 4 ;
	}
	else
	{
		nChannelCount = (pBase->dwBitsPerPixel + 0x07) >> 3 ;
	}
	//
	// データヘッダを設定する
	//
	WORD	wCompression = wswap( 1 ) ;
	bufImage.Write( &wCompression, sizeof(WORD) ) ;
	//
	DWORD	dwLenDataPos = bufImage.GetLength( ) ;
	int		nTotalLines = pBase->dwImageHeight * nChannelCount ;
	DWORD	dwBytes = nTotalLines * sizeof(WORD) ;
	bufImage.PutBuffer( dwBytes ) ;
	bufImage.Flush( dwBytes ) ;
	//
	// 各チャネルを圧縮
	//
	for ( iChannel = 0; iChannel < nChannelCount; iChannel ++ )
	{
		//
		// チャネルインデックスを取得
		//
		int	iCh = iChannel ;
		if ( (nChannelCount >= 3) && (iChannel < 3) )
		{
			iCh = 2 - iChannel ;
		}
		if ( iCh * 8 > (int) pBase->dwBitsPerPixel )
		{
			iCh = 0 ;
		}
		//
		// 各ラインを順次圧縮
		//
		const BYTE *	pbytLineBuf = (const BYTE *) pBase->ptrImageArray ;
		//
		for ( int y = 0; y < (int) pBase->dwImageHeight; y ++ )
		{
			//
			// ラインをサンプリング
			//
			DWORD	dwPixelBytes = (pBase->dwBitsPerPixel + 0x07) >> 3 ;
			int		p = iCh ;
			for ( int x = 0; x < (int) pBase->dwImageWidth; x ++ )
			{
				pbytImage[x] = pbytLineBuf[p] ;
				p += dwPixelBytes ;
			}
			pbytLineBuf += pBase->dwBytesPerLine ;
			//
			// 圧縮
			//
			int	nPackedBytes =
				PackBits( pbytRLE, pBase->dwImageWidth * 2,
							pbytImage, pBase->dwImageWidth ) ;
			//
			DWORD	dwLenDataEntryAddr =
				dwLenDataPos + (iChannel * pBase->dwImageHeight + y) * sizeof(WORD) ;
			WORD *	pwLineBytes =
				(WORD*) bufImage.ModifyBuffer
					( dwLenDataEntryAddr, sizeof(WORD) ) ;
			*pwLineBytes = wswap( (WORD) nPackedBytes ) ;
			//
			bufImage.Write( pbytRLE, nPackedBytes ) ;
		}
	}
	return	eslErrSuccess ;
}

// RLE の圧縮
//////////////////////////////////////////////////////////////////////////////
int File::PackBits
	( BYTE * ptrRLE, int nRLESize, const BYTE * ptrBuf, int nBufSize )
{
	int	i = 0, j = 0 ;
	while ( i < nBufSize )
	{
		//
		// ランレングスを取得
		//
		int		k = i ;
		BYTE	bytCur = ptrBuf[i] ;
		while ( ++ k < nBufSize )
		{
			if ( (bytCur != ptrBuf[k]) || (k - i >= 0x80) )
			{
				break ;
			}
		}
		int	nLen = k - i ;
		if ( nLen >= 2 )
		{
			//
			// ランレングス符号
			//
			if ( j + 2 > nRLESize )
			{
				break ;
			}
			ptrRLE[j]     = (SBYTE) (1 - nLen) ;
			ptrRLE[j + 1] = bytCur ;
			i = k ;
			j += 2 ;
		}
		else
		{
			//
			// 非ランレングスブロック
			//
			BYTE	bytLast = bytCur ;
			for ( nLen = 1; nLen < 0x80; nLen ++ )
			{
				k = i + nLen ;
				if ( k >= nBufSize )
				{
					break ;
				}
				bytCur = ptrBuf[k] ;
				if ( bytCur == bytLast )
				{
					nLen -- ;
					break ;
				}
				bytLast = bytCur ;
			}
			if ( j + nLen + 1 > nRLESize )
			{
				break ;
			}
			ESLAssert( nLen > 0 ) ;
			ptrRLE[j ++] = (BYTE) (nLen - 1) ;
			do
			{
				ptrRLE[j ++] = ptrBuf[i ++] ;
			}
			while ( -- nLen ) ;
		}
	}
	return	j ;
}
