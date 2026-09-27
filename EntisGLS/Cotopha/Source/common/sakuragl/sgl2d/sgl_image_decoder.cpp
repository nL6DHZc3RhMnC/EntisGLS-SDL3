
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuraglx/extra/psdlib.h>
#include <sakuragl/sgl_media.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_win_avi_composer.h>
#include <sakuragl/sgl_gdiplus_image_decoder.h>
#endif

#if	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_image_decoder.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// 画像デコーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageDecoderInterface, SObject )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLImageDecoderInterface::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	false ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLImageDecoderInterface::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	false ;
}

// 画像読み込み（縮小）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageDecoderInterface::ReadPreviewImage
	( SGLImageObject & image, SSystem::SFileInterface & file )
{
	return	sglErrNotSupported ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像デコーダー管理
//////////////////////////////////////////////////////////////////////////////

// 画像デコーダー配列
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( SSystem::SObjectArray<SGLImageDecoderInterface> *
					SGLImageDecoderManager::m_arrayImageDecoder = NULL ) ;

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoderManager::Initialzie( void )
{
	if ( m_arrayImageDecoder == NULL )
	{
		m_arrayImageDecoder =
			new SSystem::SObjectArray<SGLImageDecoderInterface> ;

		#if	defined(__COTOPHA__)
		m_arrayImageDecoder->Add( new SGLDefaultImageDecoder ) ;

		#elif	defined(__PLATFORM_WINDOWS__)
		SGLGdiplusImageDecoder *	pGdiplus = new SGLGdiplusImageDecoder ;
		if ( pGdiplus->IsGDIplusInstalled() )
		{
			m_arrayImageDecoder->Add( pGdiplus ) ;
		}
		else
		{
			delete	pGdiplus ;
		}
		m_arrayImageDecoder->Add( new SGLAVIImageDecoder ) ;

		#elif	defined(__PLATFORM_ANDROID__)
		m_arrayImageDecoder->Add( new SGLAndroidImageDecoder ) ;

		#endif

		#if	!defined(__COTOPHA__)
		m_arrayImageDecoder->Add( new SGLERImageDecoder ) ;
		m_arrayImageDecoder->Add( new SGLWindowsBitmapDecoder ) ;
		m_arrayImageDecoder->Add( new SGLTGAImageDecoder ) ;
		m_arrayImageDecoder->Add( new SGLPSDImageDecoder ) ;
		#endif
	}
}

// 終了
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoderManager::Finalize( void )
{
	delete	m_arrayImageDecoder ;
	m_arrayImageDecoder = NULL ;
}

// デコーダー追加登録
//////////////////////////////////////////////////////////////////////////////
void SGLImageDecoderManager::RegisterDecoder( SGLImageDecoderInterface * pDecoder )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageDecoder == NULL )
	{
		m_arrayImageDecoder =
			new SSystem::SObjectArray<SGLImageDecoderInterface> ;
	}
	m_arrayImageDecoder->InsertAt( 0, pDecoder ) ;
	SSystem::QuickUnlock() ;
}

// 拡張子が合致するデコーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoderInterface *
	SGLImageDecoderManager::FindDecoder( const wchar_t * pszExt )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	const size_t	nCount = m_arrayImageDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageDecoderInterface *
					pDecoder = m_arrayImageDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			if ( pDecoder->IsMatchableFileExtension( pszExt ) )
			{
				SSystem::QuickUnlock() ;
				return	pDecoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

// MIME が合致するデコーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageDecoderInterface *
	SGLImageDecoderManager::FindDecoderAsMIME( const wchar_t * pszMIME )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	const size_t	nCount = m_arrayImageDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageDecoderInterface *
					pDecoder = m_arrayImageDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			if ( pDecoder->IsMatchableMIMEType( pszMIME ) )
			{
				SSystem::QuickUnlock() ;
				return	pDecoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageDecoderManager::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	SFileInterface *	pFile = &file ;
	SSmartBuffer		bufFile ;
	if ( !file.IsSeekable() )
	{
		bufFile.ReadFromStream( file ) ;
		pFile = &bufFile ;
	}
	SSystem::QuickLock() ;
	if ( m_arrayImageDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	sglErrFailed ;
	}
	const size_t	nCount = m_arrayImageDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageDecoderInterface *
					pDecoder = m_arrayImageDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			int64_t	posStart = pFile->GetPosition() ;
			SSystem::QuickUnlock() ;
			if ( !pDecoder->ReadImage( image, *pFile, nLimitFrames ) )
			{
				return	sglErrSuccess ;
			}
			SSystem::QuickLock() ;
			pFile->Seek( posStart ) ;
		}
	}
	SSystem::QuickUnlock() ;
	return	sglErrFailed ;
}

// 画像読み込み（縮小）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageDecoderManager::ReadPreviewImage
	( SGLImageObject & image, SSystem::SFileInterface & file )
{
	SFileInterface *	pFile = &file ;
	SSmartBuffer		bufFile ;
	if ( !file.IsSeekable() )
	{
		bufFile.ReadFromStream( file ) ;
		pFile = &bufFile ;
	}
	SSystem::QuickLock() ;
	if ( m_arrayImageDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	sglErrFailed ;
	}
	const size_t	nCount = m_arrayImageDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageDecoderInterface *
					pDecoder = m_arrayImageDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			int64_t	posStart = pFile->GetPosition() ;
			SSystem::QuickUnlock() ;
			if ( !pDecoder->ReadPreviewImage( image, *pFile ) )
			{
				return	sglErrSuccess ;
			}
			SSystem::QuickLock() ;
			pFile->Seek( posStart ) ;
		}
	}
	SSystem::QuickUnlock() ;
	return	sglErrFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// Sakura2VM ネイティブ実装デコーダー
//////////////////////////////////////////////////////////////////////////////

#if	defined(__COTOPHA__)

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDefaultImageDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLDefaultImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	Image::IsLoadableFileExtension( pszExt ) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLDefaultImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	Image::IsLoadableMIMEType( pszMIME ) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDefaultImageDecoder::ReadImage
	( SGLImageObject & image, SSystem::SFileInterface & file, size_t nLimitFrames )
{
	//
	// ファイル読み込み
	//
	SGLError	err ;
	Image *		pImage = new Image ;
	File *		pFile = file.GetFileObject() ;
	if ( pFile != NULL )
	{
		err = pImage->ReadImage( pFile, NULL, nLimitFrames ) ;
	}
	else
	{
		SByteBuffer	sbuf ;
		sbuf.ReadFromFile( file ) ;
		err = pImage->ReadImage( sbuf.GetFileObject(), NULL, nLimitFrames ) ;
	}
	if ( err )
	{
		delete	pImage ;
		return	err ;
	}
	//
	// データ複製
	//
	SGLImageInfo	imginf ;
	if ( pImage->GetImageInfo( imginf ) )
	{
		delete	pImage ;
		return	sglErrFailed ;
	}
	size_t	countFrame = pImage->GetFrameCount() ;
	image.CreateBuffer
		( imginf, SGLImageObject::bufferOnMemory,
					countFrame, pImage->GetTotalTime() ) ;
	//
	SArray<uint32_t>	tblSeq ;
	size_t	countSeq = pImage->GetSequenceLength() ;
	if ( countSeq > 0 )
	{
		tblSeq.SetLength( countSeq ) ;
		countSeq = pImage->GetSequenceTable( tblSeq.GetArray(), countSeq ) ;
		tblSeq.FinishArray() ;
		image.SetSequenceTable( tblSeq.GetConstArray(), countSeq ) ;
	}
	//
	for ( size_t i = 0; i < countFrame; i ++ )
	{
		pImage->SelectFrame( i ) ;
		image.SelectFrame( i ) ;
		//
		if ( imginf.format & formatImageFlagPalette )
		{
			SArray<SGLPalette>	tblPalette ;
			tblPalette.SetLength( 0x100 ) ;
			//
			size_t	countPalette =
				pImage->GetPaletteTable( tblPalette.GetArray(), 0x100 ) ;
			tblPalette.FinishArray() ;
			image.SetPaletteTable( tblPalette.GetConstArray(), countPalette ) ;
		}
		uint8_t *	pbytImage = image.LockBuffer( imginf ) ;
		pImage->ReadFrameBuffer( imginf, pbytImage, i ) ;
		image.UnlockBuffer() ;
	}
	image.SelectFrame( 0 ) ;
	delete	pImage ;
	return	sglErrSuccess ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// ERI 画像デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLERImageDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLERImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"eri" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLERImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/x-eri" ) == 0) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLERImageDecoder::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	//
	// ファイルヘッダ読み込み
	//
	SGLMediaFile	media ;
	SGLError	err =
		(SGLError) media.OpenMediaFile( &file, SGLMediaFile::openStream ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !(media.m_flagsRead & SGLMediaFile::readFileHeader)
		| !(media.m_flagsRead & SGLMediaFile::readImageInfo) )
	{
		return	sglErrFailed ;
	}
	//
	// ヘッダ情報取得
	//
	SGLImageInfo	imginf ;
	SGLSize			sizeClip ;
	bool			fClipBuffer = false ;
	imginf.format = media.m_eriInfoHeader.fdwFormatType ;
	imginf.depth = media.m_eriInfoHeader.dwBitsPerPixel ;
	imginf.width = media.m_eriInfoHeader.nImageWidth ;
	imginf.height = media.m_eriInfoHeader.nImageHeight ;
	if ( media.m_eriInfoHeader.nImageHeight < 0 )
	{
		imginf.height = - media.m_eriInfoHeader.nImageHeight ;
	}
	imginf.colorClip = media.m_eriInfoHeader.dwClippedPixel ;
	if ( imginf.depth == 24 )
	{
		imginf.depth = 32 ;
	}
	if ( (media.m_eriInfoHeader.fdwTransformation == eriTransformationDCT)
		|| (media.m_eriInfoHeader.fdwTransformation == eriTransformationLOT) )
	{
		if ( (imginf.width & 0x0F) | (imginf.height & 0x0F) )
		{
			sizeClip.w = (int32_t) imginf.width ;
			sizeClip.h = (int32_t) imginf.height ;
			imginf.width = (imginf.width + 0x0F) & ~0x0F ;
			imginf.height = (imginf.height + 0x0F) & ~0x0F ;
			fClipBuffer = true ;
		}
	}
	imginf.pitchPixel = (imginf.depth + 0x07) >> 3 ;
	imginf.pitchLine = (imginf.width * imginf.pitchPixel + 0x03) & ~0x03 ;
	//
	SString	strRefFile ;
	if ( media.m_flagsRead & SGLMediaFile::readDescription )
	{
		SGLMediaFile::STagInfo	taginf ;
		taginf.ParseTagInfo( media.m_strDescription ) ;
		taginf.GetHotSpot( imginf.ptOrigin ) ;
		//
		strRefFile = taginf.GetTagContents( SGLMediaFile::tagReferenceFile ) ;
	}
	//
	// バッファ生成
	//
	size_t	nFrameCount = media.m_eriFileHeader.dwFrameCount ;
	if ( (nLimitFrames != 0) && (nFrameCount > nLimitFrames) )
	{
		nFrameCount = nLimitFrames ;
	}
	err = image.CreateBuffer
		( imginf, SGLImageObject::bufferOnMemory,
			nFrameCount,
			media.m_eriFileHeader.dwAllFrameTime
				* nFrameCount / media.m_eriFileHeader.dwFrameCount ) ;
	if ( err )
	{
		return	err ;
	}
	if ( fClipBuffer )
	{
		image.NormalizeFormat
			( 0, 0, SGLImageObject::formatCastSize,
				(uint32_t) sizeClip.w, (uint32_t) sizeClip.h ) ;
	}
	//
	// シーケンステーブル設定
	//
	if ( media.m_flagsRead & SGLMediaFile::readSequenceTable )
	{
		const size_t	nLength = media.m_tableSequence.GetLength() ;
		if ( nLength >= 1 )
		{
			size_t		i ;
			uint32_t	nQuantumTime = media.m_tableSequence.At(0).nDuration ;
			for ( i = 1; i < nLength; i ++ )
			{
				uint32_t	nDuration = media.m_tableSequence.At(i).nDuration ;
				if ( (nQuantumTime == 0) | (nDuration == 0) )
				{
					break ;
				}
				nQuantumTime =
					SakuraCL::ComputeGCD<uint32_t>( nQuantumTime, nDuration ) ;
			}
			if ( nQuantumTime < 8 )
			{
				nQuantumTime = 8 ;
			}
			SArray<uint32_t>	tableSeq ;
			for ( i = 0; i < nLength; i ++ )
			{
				SGLMediaFile::SEQUENCE_DELTA&
							seqDelta = media.m_tableSequence.At(i) ;
				uint32_t	nDuration = seqDelta.nDuration ;
				const size_t
					nCount = (nDuration + (nQuantumTime - 1)) / nQuantumTime ;
				for ( size_t j = 0; j < nCount; j ++ )
				{
					tableSeq.Add( seqDelta.nFrame ) ;
				}
			}
			image.SetSequenceTable
				( tableSeq.GetConstArray(), tableSeq.GetLength() ) ;
		}
	}
	//
	// 画像デコーダ準備
	//
	SGLImageDecoder		decoder ;
	SGLDecodeBitStream	bstream( 0x10000 ) ;
	err = (SGLError) decoder.Initialize( media.m_eriInfoHeader ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 画像順次読み込み
	//
	SArray<SGLPalette>	tablePalette ;
	size_t	iFrame = 0 ;
	while ( iFrame < nFrameCount )
	{
		if ( media.DescendChunk() )
		{
			break ;
		}
		if ( media.IsEqualCurrentChunkID( "ImageFrm" )
			|| media.IsEqualCurrentChunkID( "DiffeFrm" ) )
		{
			// 画像読み込み
			err = image.SelectFrame( iFrame, stereoImageBoth ) ;
			if ( err )
			{
				return	err ;
			}
			if ( imginf.format & formatImageFlagPalette )
			{
				image.SetPaletteTable
					( tablePalette.GetConstArray(), tablePalette.GetLength() ) ;
			}
			SGLImageInfo	infFrame ;
			uint8_t *	ptrFrame = image.LockBuffer( infFrame ) ;
			if ( ptrFrame == NULL )
			{
				return	sglErrFailed ;
			}
			uint32_t flagsDecoding = SGLImageDecoder::flagTopDown ;
			if ( (iFrame > 0) && media.IsEqualCurrentChunkID( "DiffeFrm" ) )
			{
				image.ReadFrameBuffer
					( infFrame, ptrFrame, iFrame - 1, stereoImageBoth ) ;
				flagsDecoding |= SGLImageDecoder::flagDifferential ;
			}
			bstream.AttachInputStream( &media ) ;
			err = (SGLError) decoder.DecodeImage
						( infFrame, ptrFrame, bstream, flagsDecoding ) ;
			if ( err )
			{
				return	err ;
			}
			image.UnlockBuffer( ) ;
			iFrame ++ ;
		}
		else if ( media.IsEqualCurrentChunkID( "Palette " ) )
		{
			// パレットテーブル読み込み
			size_t	nPaletteLength =
				(size_t) media.GetCurrentChunkLength() / sizeof(SGLPalette) ;
			tablePalette.SetLength( nPaletteLength ) ;
			media.Read( tablePalette.GetArray(),
					nPaletteLength * sizeof(SGLPalette) ) ;
			tablePalette.FinishArray() ;
		}
		media.AscendChunk( ) ;
	}
	//
	// 参照画像処理
	//
	strRefFile.TrimRight() ;
	if ( !strRefFile.IsEmpty() )
	{
		SSmartPointer<SFileInterface>
			pfile = file.NewOpenFile
						( strRefFile, SFileOpener::shareRead ) ;
		if ( pfile != NULL )
		{
			SGLImage	imgRef ;
			if ( !imgRef.ReadImage( pfile ) )
			{
				for ( iFrame = 0; iFrame < media.m_eriFileHeader.dwFrameCount; iFrame ++ )
				{
					if ( image.SelectFrame( iFrame, stereoImageBoth )
						|| imgRef.SelectFrame( iFrame, stereoImageBoth) )
					{
						continue ;
					}
					SGLImageBuffer	infDst ;
					SGLImageBuffer	infRef ;
					infDst.ptrBuffer = image.LockBuffer( infDst ) ;
					infRef.ptrBuffer =
						imgRef.LockBuffer( infRef, SGLImageObject::lockRead ) ;
					if ( infDst.ptrBuffer && infRef.ptrBuffer )
					{
						ERISA::eriWrapAroundAddImageBuffer( infDst, infRef ) ;
					}
					image.UnlockBuffer() ;
					imgRef.UnlockBuffer( SGLImageObject::lockRead ) ;
				}
			}
		}
	}
	image.SelectFrame( 0, stereoImageBoth ) ;
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// Windows Bitmap 画像デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowsBitmapDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsBitmapDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"bmp" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsBitmapDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/bmp" ) == 0) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsBitmapDecoder::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	int64_t	fposStart = file.GetPosition() ;
	//
	// ファイルヘッダ読み込み
	//
	BYTE	bytSig[2] ;
	if ( file.Read( &bytSig[0], 2 ) < 2 )
	{
		return	sglErrFailed ;
	}
	if ( (bytSig[0] != 'B') | (bytSig[1] != 'M') )
	{
		return	sglErrFailed ;
	}
	BITMAPFILEHEADER	bmFileHeader ;
	if ( file.Read
		( &bmFileHeader, sizeof(BITMAPFILEHEADER) )
								< sizeof(BITMAPFILEHEADER) )
	{
		return	sglErrFailed ;
	}
	if ( (bmFileHeader.bfReserved1 != 0)
		|| (bmFileHeader.bfReserved2 != 0)
		|| (bmFileHeader.bfOffBits
				< 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) )
	{
		return	sglErrFailed ;
	}
	//
	// 画像情報ヘッダ読み込み
	//
	BITMAPINFOHEADER	bmInfoHeader ;
	if ( file.Read
		( &bmInfoHeader, sizeof(BITMAPINFOHEADER) )
								< sizeof(BITMAPINFOHEADER) )
	{
		return	sglErrFailed ;
	}
	if ( (bmInfoHeader.biSize != sizeof(BITMAPINFOHEADER))
		| (bmInfoHeader.biCompression != 0) )
	{
		return	sglErrFailed ;
	}
	if ( (bmInfoHeader.biBitCount != 8) & (bmInfoHeader.biBitCount != 16)
		& (bmInfoHeader.biBitCount != 24) & (bmInfoHeader.biBitCount != 32) )
	{
		return	sglErrFailed ;
	}
	SGLImageInfo	imginf ;
	imginf.format = formatImageRGB ;
	imginf.depth = bmInfoHeader.biBitCount ;
	imginf.width = bmInfoHeader.biWidth ;
	imginf.height = bmInfoHeader.biHeight ;
	imginf.pitchPixel = (imginf.depth + 0x07) >> 3 ;
	imginf.pitchLine = (imginf.width * imginf.pitchPixel + 0x03) & ~0x03 ;
	//
	// パレットテーブル読み込み
	//
	RGBQUAD	tblPalette[0x100] ;
	size_t	nPalLength = 0 ;
	if ( bmInfoHeader.biBitCount <= 8 )
	{
		nPalLength = (size_t) 1 << bmInfoHeader.biBitCount ;
		if ( bmInfoHeader.biClrUsed != 0 )
		{
			if ( nPalLength < bmInfoHeader.biClrUsed )
			{
				return	sglErrFailed ;
			}
			nPalLength = bmInfoHeader.biClrUsed ;
		}
		if ( file.Read
			( &tblPalette[0],
				nPalLength * sizeof(RGBQUAD) )
						< nPalLength * sizeof(RGBQUAD) )
		{
			return	sglErrFailed ;
		}
		if ( nPalLength == 0x100 )
		{
			bool	flagGray = true ;
			for ( size_t i = 0; i < 0x100; i ++ )
			{
				if ( (tblPalette[i].rgbBlue != i)
					| (tblPalette[i].rgbGreen != i)
					| (tblPalette[i].rgbRed != i) )
				{
					flagGray = false ;
					break ;
				}
			}
			if ( flagGray )
			{
				imginf.format = formatImageGray ;
			}
		}
	}
	//
	// バッファ生成
	//
	SGLError	err =
		image.CreateBuffer
			( imginf, SGLImageObject::bufferOnMemory ) ;
	if ( err )
	{
		return	err ;
	}
	if ( nPalLength > 0 )
	{
		image.SetPaletteTable
			( (SGLPalette*) &tblPalette[0], nPalLength ) ;
	}
	//
	// 画像データ読み込み
	//
	file.Seek( fposStart + bmFileHeader.bfOffBits ) ;
	//
	SGLImageInfo	infFrame ;
	uint8_t *		ptrFrame = image.LockBuffer( infFrame ) ;
	if ( ptrFrame == NULL )
	{
		return	sglErrFailed ;
	}
	ptrFrame += infFrame.pitchLine * (infFrame.height - 1) ;
	for ( size_t y = 0; y < infFrame.height; y ++ )
	{
		if ( file.Read( ptrFrame, imginf.pitchLine )
								< (uint32_t) imginf.pitchLine )
		{
			break ;
		}
		ptrFrame -= infFrame.pitchLine ;
	}
	image.UnlockBuffer( ) ;
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// TGA 画像デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLTGAImageDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLTGAImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"tga" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLTGAImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/tga" ) == 0) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLTGAImageDecoder::ReadImage
	( SGLImageObject & image,	
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	int64_t	fposStart = file.GetPosition() ;
	//
	// ファイルヘッダ読み込み
	//
	FILEHEADER	tgaHeader ;
	if ( file.Read
		( &tgaHeader, sizeof(FILEHEADER) ) < sizeof(FILEHEADER) )
	{
		return	sglErrFailed ;
	}
	if ( (tgaHeader.typeFormat != tgaIndexed)
		&& (tgaHeader.typeFormat != tgaRGB)
		&& (tgaHeader.typeFormat != tgaIndexed_RLE)
		&& (tgaHeader.typeFormat != tgaRGB_RLE) )
	{
		return	sglErrFailed ;
	}
	if ( (tgaHeader.depth != 8)
		&& (tgaHeader.depth != 16)
		&& (tgaHeader.depth != 24)
		&& (tgaHeader.depth != 32) )
	{
		return	sglErrFailed ;
	}
	//
	// パレットテーブル読み込み
	//
	SArray<SGLPalette>	tblPalette ;
	if ( tgaHeader.typeFormat & tgaIndexed )
	{
		SArray<uint8_t>	bufTemp ;
		size_t	nBytes =
				(size_t) (tgaHeader.lenColormap.GetWord()
								* tgaHeader.depthColormap / 8) ;
		uint8_t *	pbytBuf = bufTemp.GetArray( nBytes ) ;
		if ( file.Read( pbytBuf, nBytes ) < nBytes )
		{
			bufTemp.FinishArray() ;
			return	sglErrFailed ;
		}
		bufTemp.FinishArray() ;
		//
		size_t	nPitch = tgaHeader.depthColormap / 8 ;
		size_t	nCount = tgaHeader.lenColormap.GetWord() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLPalette	rgb ;
			rgb.argb.Blue = pbytBuf[0] ;
			if ( nPitch >= 2 )
			{
				rgb.argb.Green = pbytBuf[1] ;
				rgb.argb.Red = pbytBuf[2] ;
				if ( nPitch >= 3 )
				{
					rgb.argb.Alpha = pbytBuf[3] ;
				}
			}
			tblPalette.Add( rgb ) ;
		}
	}
	//
	// 画像データ読み込み
	//
	SArray<uint8_t>	bufPixels ;
	const size_t	nWidth = tgaHeader.width.GetWord() ;
	const size_t	nHeight = tgaHeader.width.GetWord() ;
	const size_t	nPixelBytes = tgaHeader.depth / 8 ;
	const size_t	nTotalPixels = nWidth * nHeight ;
	const size_t	nTotalBytes = nTotalPixels * nPixelBytes ;
	uint8_t *		pbytPixels = bufPixels.GetArray( nTotalBytes ) ;
	//
	if ( !(tgaHeader.typeFormat & tgaRLE_Flag) )
	{
		if ( file.Read( pbytPixels, nTotalBytes ) < nTotalBytes )
		{
			bufPixels.FinishArray() ;
			return	sglErrFailed ;
		}
	}
	else
	{
		//
		// RLE 圧縮
		//
		SArray<uint8_t>	bufRLE ;
		const size_t	nRLBytes = (size_t) (file.GetLength()
											- file.GetPosition()) ;
		uint8_t *		pbytRLE = bufRLE.GetArray( nRLBytes ) ;
		//
		if ( file.Read( pbytRLE, nRLBytes ) < nRLBytes )
		{
			bufRLE.FinishArray() ;
			return	sglErrFailed ;
		}
		uint8_t *	pbytDst = pbytPixels ;
		size_t		iDst = 0 ;
		size_t		iSrc = 0 ;
		while ( (iSrc < nRLBytes) && (iDst < nTotalPixels) )
		{
			uint8_t	rl = pbytRLE[iSrc ++] ;
			size_t	len = (rl & 0x7F) + 1 ;
			if ( iDst + len > nTotalPixels )
			{
				len = nTotalPixels - iDst ;
			}
			if ( rl & 0x80 )
			{
				for ( size_t i = 0; i < len; i ++ )
				{
					for ( size_t j = 0; j < nPixelBytes; j ++ )
					{
						pbytDst[j] = pbytRLE[j] ;
					}
					pbytDst += nPixelBytes ;
				}
				pbytRLE += nPixelBytes ;
			}
			else
			{
				size_t	nRlBytes = nPixelBytes * len ;
				for ( size_t i = 0; i < nRlBytes; i ++ )
				{
					pbytDst[i] = pbytRLE[i] ;
				}
				pbytDst += nRlBytes ;
				iSrc += nRlBytes ;
			}
			iDst += len ;
		}
		bufRLE.FinishArray() ;
	}
	//
	// バッファ生成
	//
	SGLImageInfo	imginf ;
	imginf.format = formatImageRGB ;
	if ( nPixelBytes == 4 )
	{
		imginf.format |= formatImageFlagAlpha ;
	}
	if ( tgaHeader.typeFormat & tgaIndexed )
	{
		imginf.format |= formatImageFlagPalette ;
	}
	imginf.depth = tgaHeader.depth ;
	imginf.width = (uint32_t) nWidth ;
	imginf.height = (uint32_t) nHeight ;
	imginf.pitchPixel = (int32_t) nPixelBytes ;
	imginf.pitchLine = (int32_t) (nPixelBytes * nWidth) ;
	//
	SGLError	err =
		image.CreateBuffer
			( imginf, SGLImageObject::bufferOnMemory ) ;
	if ( err )
	{
		bufPixels.FinishArray() ;
		return	err ;
	}
	if ( tblPalette.GetLength() > 0 )
	{
		image.SetPaletteTable
			( tblPalette.GetConstArray(), tblPalette.GetLength() ) ;
	}
	//
	// 画像ビルドアップ
	//
	uint8_t *	pbytLocked = image.LockBuffer( imginf ) ;
	size_t		nSrcLineBytes = nWidth * nPixelBytes ;
	for ( size_t y = 0; y < imginf.height; y ++ )
	{
		size_t	iy = (imginf.height - 1) - y ;
		eslCopyMemory
			( pbytLocked + y * imginf.pitchLine,
				pbytPixels + iy * nSrcLineBytes, nSrcLineBytes ) ;
	}
	image.UnlockBuffer() ;
	bufPixels.FinishArray() ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// PSD ファイルデコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLPSDImageDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLPSDImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"psd" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLPSDImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/x-photoshop" ) == 0) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPSDImageDecoder::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	PSD::FileReader	psdfr ;
	if ( psdfr.Open( &file, false ) )
	{
		return	sglErrFailed ;
	}
	PSD::FILE_HEADER	fhHeader ;
	psdfr.GetFileHeader( fhHeader ) ;
	size_t	nChannels = fhHeader.wChannels.GetUInt16() ;
	//
	SGLError	err ;
	if ( psdfr.GetLayerCount() >= 1 )
	{
		size_t	nFrameCount = psdfr.GetLayerCount() ;
		if ( (nLimitFrames != 0) && (nFrameCount > nLimitFrames) )
		{
			nFrameCount = nLimitFrames ;
		}
		image.CreateImage
			( fhHeader.dwWidth.GetUInt32(),
				fhHeader.dwHeight.GetUInt32(),
				formatImageARGB, 32,
				SGLImageObject::bufferOnMemory,
				nFrameCount, nFrameCount * 33 ) ;
		//
		for ( size_t i = 0; i < nFrameCount; i ++ )
		{
			SGLImage	imgLayer ;
			if ( psdfr.LoadLayerImage( imgLayer, i ) )
			{
				err = sglErrFailed ;
			}
			else
			{
				PSD::FileReader::LayerInfo	liLayer ;
				if ( psdfr.GetLayerInfo( liLayer, i ) )
				{
					liLayer.rectImage.x = 0 ;
					liLayer.rectImage.y = 0 ;
				}
				image.SelectFrame( i ) ;
				image.ConvertImage
					( &imgLayer, liLayer.rectImage.x, liLayer.rectImage.y ) ;
			}
		}
		image.SelectFrame( 0 ) ;
	}
	else
	{
		err = (SGLError) psdfr.LoadBaseImage( image ) ;
	}
	return	err ;
}


/////////////////////////////////////////////////////////////////////////////
// AVI ファイルデコーダー
/////////////////////////////////////////////////////////////////////////////

#if	defined(__PLATFORM_WINDOWS__)

// クラス情報
/////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAVIImageDecoder, SGLImageDecoderInterface )

// ファイル拡張子判定
/////////////////////////////////////////////////////////////////////////////
bool SGLAVIImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"avi" ) == 0) ;
}

// MIME 判定
/////////////////////////////////////////////////////////////////////////////
bool SGLAVIImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"video/avi" ) == 0) ;
}

// 画像読み込み
/////////////////////////////////////////////////////////////////////////////
SGLError SGLAVIImageDecoder::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	SGLWindowsAVIReader	avir ;
	if ( avir.Open( &file, false ) )
	{
		return	sglErrFailed ;
	}
	size_t	nFrameCount = (size_t) avir.GetVideoLength() ;
	int64_t	msecDuration= avir.GetVideoDuration() ;
	if ( (nLimitFrames != 0) && (nFrameCount > nLimitFrames) )
	{
		nFrameCount = nLimitFrames ;
	}
	SGLImageInfo	imginf ;
	avir.GetImageFormat( imginf ) ;
	image.CreateImage
		( imginf.width, imginf.height,
			imginf.format, imginf.depth,
			SGLImageObject::bufferOnMemory,
			nFrameCount, msecDuration ) ;
	//
	for ( size_t i = 0; i < nFrameCount; i ++ )
	{
		image.SelectFrame( i ) ;
		//
		SGLImageInfo	infFrame ;
		uint8_t*	pbytBuf =
			image.LockBuffer( infFrame, SGLImageObject::lockWrite ) ;
		//
		avir.ReadFrame( infFrame, pbytBuf ) ;
		//
		image.UnlockBuffer( SGLImageObject::lockWrite ) ;
	}
	image.SelectFrame( 0 ) ;
	//
	return	sglErrSuccess ;
}

#endif
