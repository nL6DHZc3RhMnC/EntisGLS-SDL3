
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>
#include <sakuragl/sgl2d/sgl_image_buf_interface.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuraglx/sprite/sglx_sprite_cursor.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 抽象画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageObject, SObject )

// フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::GetFrameCount( void ) const
{
	return	0 ;
}

// アニメーション・シーケンス全長取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::GetSequenceLength( void ) const
{
	return	0 ;
}

// アニメーション・シーケンス取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::GetSequenceTable( uint32_t * pSeq, size_t nCount ) const
{
	return	0 ;
}

// アニメーション全長（時間）取得 [msec]
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLImageObject::GetTotalTime( void ) const
{
	return	0 ;
}

// 時間からフレーム番号へ変換
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::FrameFromMilliSec( uint64_t msec )
{
	return	0 ;
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::SelectFrame( size_t iFrame, int iSide )
{
	return	sglErrFailed ;
}

// 選択中フレーム取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::GetSelectedFrame( int * pSide ) const
{
	if ( pSide != NULL )
	{
		*pSide = stereoImageRight ;
	}
	return	0 ;
}

// 画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::GetImageInfo( SGLImageInfo & imginf ) const
{
	return	sglErrFailed ;
}

// 画像サイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLImageObject::GetImageSize( void ) const
{
	SGLImageInfo	imginf ;
	if ( GetImageInfo( imginf ) )
	{
		return	SGLSize( 0, 0 ) ;
	}
	return	SGLSize( imginf.width, imginf.height ) ;
}

uint32_t SGLImageObject::GetImageWidth( void ) const
{
	SGLImageInfo	imginf ;
	if ( GetImageInfo( imginf ) )
	{
		return	0 ;
	}
	return	imginf.width ;
}

uint32_t SGLImageObject::GetImageHeight( void ) const
{
	SGLImageInfo	imginf ;
	if ( GetImageInfo( imginf ) )
	{
		return	0 ;
	}
	return	imginf.height ;
}

// 画像フォーマット取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLImageObject::GetImageFormat( void ) const
{
	SGLImageInfo	imginf ;
	if ( GetImageInfo( imginf ) )
	{
		return	0 ;
	}
	return	imginf.format ;
}

// パレット・テーブル取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::GetPaletteTable( SGLPalette * pPalette, size_t nCount )
{
	return	0 ;
}

// 画像バッファ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLImageObject::LockBuffer
	( SGLImageInfo & imginf, int flags, const SGLImageRect * pRect )
{
	return	NULL ;
}

SGLError SGLImageObject::FlushBuffer( int flags )
{
	return	sglErrFailed ;
}

SGLError SGLImageObject::UnlockBuffer( int flags )
{
	return	sglErrFailed ;
}

// 画像バッファ読み出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::ReadFrameBuffer
	( const SGLImageInfo & imginf, uint8_t * ptrBuffer, size_t iFrame, int iSide )
{
	return	sglErrFailed ;
}

// テクスチャのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::NormalizeToTexture( uint32_t nFlags )
{
	return	sglErrFailed ;
}

SGLError SGLImageObject::NormalizeToMipmapTexture( uint32_t nFlags )
{
	return	sglErrFailed ;
}

// レンダリング・ターゲットのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::NormalizeToRenderTarget( uint32_t nFlags )
{
	return	sglErrFailed ;
}

// テクスチャのための正規化フラグを除去する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::DenormalizeForTexture( uint32_t nFlags )
{
	return	sglErrFailed ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateImage
	( uint32_t width, uint32_t height,
		uint32_t format, uint32_t depth,
		uint64_t nFlags, size_t countFrame, uint64_t msecLong )
{
	SGLImageInfo	imginf ;
	imginf.format = format ;
	imginf.depth = depth ;
	imginf.width = width ;
	imginf.height = height ;
	//
	return	CreateBuffer( imginf, nFlags, countFrame, msecLong ) ;
}

SGLError SGLImageObject::CreateCloneImage( SGLImageObject& imgObj, uint64_t nFlags )
{
	SGLImageInfo	imginf ;
	size_t	countFrame = imgObj.GetFrameCount() ;
	imgObj.GetImageInfo( imginf ) ;
	//
	SGLError	err =
		CreateBuffer( imginf, nFlags, countFrame, imgObj.GetTotalTime() ) ;
	if ( err )
	{
		return	err ;
	}
	//
	for ( size_t i = 0; i < countFrame; i ++ )
	{
		SGLImageInfo	infLock ;
		uint8_t *		pbytLock ;
		if ( imginf.format & formatImageFlagSideBySide )
		{
			SelectFrame( i, stereoImageRight ) ;
			pbytLock = LockBuffer( infLock ) ;
			imgObj.ReadFrameBuffer( infLock, pbytLock, i, stereoImageRight ) ;
			UnlockBuffer() ;
			//
			SelectFrame( i, stereoImageLeft ) ;
			pbytLock = LockBuffer( infLock ) ;
			imgObj.ReadFrameBuffer( infLock, pbytLock, i, stereoImageLeft ) ;
			UnlockBuffer() ;
		}
		else
		{
			SelectFrame( i ) ;
			pbytLock = LockBuffer( infLock ) ;
			imgObj.ReadFrameBuffer( infLock, pbytLock, i ) ;
			UnlockBuffer() ;
		}
	}
	SelectFrame( 0 ) ;
	return	sglErrSuccess ;
}

SGLError SGLImageObject::CreateCloneBuffer
			( const SGLImageBuffer& imgbuf, uint64_t nFlags )
{
	SGLError	err = CreateBuffer( imgbuf, nFlags ) ;
	if ( !err )
	{
		if ( (imgbuf.format & formatImageFlagPalette)
						&& (imgbuf.ptrPalette != NULL) )
		{
			SetPaletteTable( imgbuf.ptrPalette, 0x100 ) ;
		}
		SGLImageBuffer	imgDst ;
		imgDst.ptrBuffer =
			LockBuffer( imgDst, SGLImageObject::lockWrite ) ;
		sglCopyImageBuffer( imgDst, imgbuf ) ;
		UnlockBuffer( SGLImageObject::lockWrite ) ;
		SetImageOrigin( imgbuf.ptOrigin.x, imgbuf.ptOrigin.y ) ;
	}
	return	err ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::ReleaseBuffer( void )
{
}

// パレット・テーブル設定
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageObject::SetPaletteTable
	( const SGLPalette * pPalette, size_t nCount )
{
	return	0 ;
}

// アニメーション・シーケンス・テーブルの設定
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::SetSequenceTable( const uint32_t * pSeq, size_t nCount )
{
}

// 画像フォーマット正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::NormalizeFormat
	( uint32_t format, uint32_t depth,
		uint32_t nFlags, uint32_t width, uint32_t height )
{
	return	sglErrFailed ;
}

// NormalizeFormat で結合されたアニメーション画像への参照を生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImageObject::NewAnimationReference
	( SGLImageRect* pFrameRects, size_t nRectsCount )
{
	return	NULL ;
}

// アトラス化処理
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImageObject::BuildAtlas
	( SGLImageObject*const* ppImages,
		size_t nSrcImageCount,
		const SGLImageObject::BuildAtlasParam& param,
		size_t * pUsedCount, size_t * pUsedIndexes )
{
	//
	// 対象フレーム収集
	//
	SArray<ImageFrame>	aImageFrames ;
	SArray<SGLSize>		aImageSizes ;
	CollectImageFrames( aImageFrames, aImageSizes, ppImages, nSrcImageCount ) ;
	//
	SGLSize *		pImageSizes = aImageSizes.GetArray() ;
	const size_t	nImageCount = aImageSizes.GetLength() ;
	//
	const uint32_t	nMargin = param.nMargin ;
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		pImageSizes[i].w += nMargin * 2 ;
		pImageSizes[i].h += nMargin * 2 ;
	}
	//
	// 配置決定
	//
	SPointerArray<SGLImageRect>	aTxtMapRects ;
	SGLImageRect **	ppTxtMapRects = aTxtMapRects.GetArray( nImageCount ) ;
	//
	SGLAreaAllocator	aalcTexture ;
	aalcTexture.SetInitialSize
		( param.sizeAtlasMin.w, param.sizeAtlasMin.h ) ;
	aalcTexture.SetLimitSize
		( param.sizeAtlasMax.w, param.sizeAtlasMax.h ) ;
	aalcTexture.SetFlags
		( (param.nFlags & atlasFlagExpandPOT)
			? SGLAreaAllocator::flagSizePoweredBy2 : 0 ) ;
	size_t	nAllocCount =
		aalcTexture.BatchAllocate
			( ppTxtMapRects, pImageSizes, nImageCount, pUsedIndexes ) ;
	if ( pUsedCount != nullptr )
	{
		*pUsedCount = nAllocCount ;
	}
	aImageSizes.FinishArray() ;
	//
	// アトラス画像生成
	//
	const SGLSize	sizeAtlas = aalcTexture.GetTotalSize() ;
	SGLImage *	pAtlasImage = new SGLImage ;
	pAtlasImage->CreateImage( sizeAtlas.w, sizeAtlas.h, formatImageARGB, 32 ) ;
	//
	// 画像結合
	//
	SGLImageBuffer *	pAtlasBuf = pAtlasImage->GetImageBuffer() ;
	if ( pAtlasBuf == nullptr )
	{
		return	pAtlasImage ;
	}
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		if ( ppTxtMapRects[i] == nullptr )
		{
			continue ;
		}
		ImageFrame *	pSrcFrame = aImageFrames.GetAt( i ) ;
		if ( (pSrcFrame == nullptr)
			|| (pSrcFrame->pImage == nullptr) )
		{
			continue ;
		}
		pSrcFrame->pImage->SelectFrame( pSrcFrame->iFrame ) ;
		//
		SGLImageBuffer *	pSrcImage = pSrcFrame->pImage->GetImageBuffer() ;
		SGLImageRect		rect = *(ppTxtMapRects[i]) ;
		if ( pSrcImage == nullptr )
		{
			continue ;
		}
		//
		// 画像複製
		//
		sglConvertImageBuffer
			( *pAtlasBuf, *pSrcImage,
				rect.x + nMargin, rect.y + nMargin ) ;
		//
		if ( !(pSrcImage->format & formatImageFlagAlpha)
			&& (param.nMargin >= 1) )
		{
			// 上辺
			SGLImageRect	rectSrc ;
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = (int32_t) pSrcImage->width ;
			rectSrc.h = 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + nMargin,
					rect.y + nMargin - 1, &rectSrc ) ;
			//
			// 下辺
			rectSrc.y = (int32_t) pSrcImage->height - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + nMargin,
					rect.y + rect.h + nMargin, &rectSrc ) ;
			//
			// 左辺
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = 1 ;
			rectSrc.h = (int32_t) pSrcImage->height ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + nMargin - 1,
					rect.y + nMargin, &rectSrc ) ;
			//
			// 右辺
			rectSrc.x = (int32_t) pSrcImage->width - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w + nMargin,
					rect.y + nMargin, &rectSrc ) ;
			//
			// 四隅
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = 1 ;
			rectSrc.h = 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + nMargin - 1,
					rect.y + nMargin - 1, &rectSrc ) ;
			//
			rectSrc.x = (int32_t) pSrcImage->width - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w + nMargin,
					rect.y + nMargin - 1, &rectSrc ) ;
			//
			rectSrc.y = (int32_t) pSrcImage->height - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w + nMargin,
					rect.y + rect.h + nMargin, &rectSrc ) ;
			//
			rectSrc.x = 0 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + nMargin - 1,
					rect.y + rect.h + nMargin, &rectSrc ) ;
		}
		//
		// バッファ参照
		//
		SGLImageRect	rectRef ;
		rectRef.x = rect.x + nMargin ;
		rectRef.y = rect.y + nMargin ;
		rectRef.w = rect.w - nMargin * 2 ;
		rectRef.h = rect.h - nMargin * 2 ;
		//
		pSrcFrame->pImage->MakeImageReference
			( pSrcFrame->iFrame, pAtlasImage, rectRef ) ;
	}
	aTxtMapRects.FinishArray() ;
	//
	return	pAtlasImage ;
}

size_t SGLImageObject::EstimateAtlasSize
	( SGLSize& sizeEstimated,
		SGLImageObject*const* ppImages,
		size_t nSrcImageCount,
		bool flagMakePOT, uint32_t nMargin,
		uint32_t nSizeLimit, size_t * pUsedIndexes )
{
	SArray<ImageFrame>	aImageFrames ;
	SArray<SGLSize>		aImageSizes ;
	CollectImageFrames( aImageFrames, aImageSizes, ppImages, nSrcImageCount ) ;
	//
	SGLSize *		pImageSizes = aImageSizes.GetArray() ;
	const size_t	nImageCount = aImageSizes.GetLength() ;
	//
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		pImageSizes[i].w += nMargin * 2 ;
		pImageSizes[i].h += nMargin * 2 ;
	}
	//
	SPointerArray<SGLImageRect>	aTxtMapRects ;
	SGLImageRect **	ppTxtMapRects = aTxtMapRects.GetArray( nImageCount ) ;
	//
	SGLAreaAllocator	aalcTexture ;
	aalcTexture.SetInitialSize( 16, 16 ) ;
	if ( nSizeLimit != 0 )
	{
		aalcTexture.SetLimitSize( (int) nSizeLimit, (int) nSizeLimit ) ;
	}
	aalcTexture.SetFlags
		( flagMakePOT ? SGLAreaAllocator::flagSizePoweredBy2 : 0 ) ;
	size_t	nAllocCount =
		aalcTexture.BatchAllocate
			( ppTxtMapRects, pImageSizes, nImageCount, pUsedIndexes ) ;
	//
	aImageSizes.FinishArray() ;
	//
	sizeEstimated = aalcTexture.GetTotalSize() ;
	return	nAllocCount ;
}

void SGLImageObject::CollectImageFrames
	( SSystem::SArray<SGLImageObject::ImageFrame>& aImageFrames,
		SSystem::SArray<SGLSize>& aImageSizes,
		SGLImageObject*const* ppImages, size_t nImageCount )
{
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		if ( ppImages[i] == nullptr )
		{
			continue ;
		}
		SGLSize		sizeImage = ppImages[i]->GetImageSize() ;
		size_t		nFrameCount = ppImages[i]->GetFrameCount() ;
		for ( size_t j = 0; j < nFrameCount; j ++ )
		{
			ImageFrame	imgfrm ;
			imgfrm.pImage = ppImages[i] ;
			imgfrm.iFrame = j ;
			//
			aImageFrames.Add( imgfrm ) ;
			aImageSizes.Add( sizeImage ) ;
		}
	}
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::LoadImage
	( const wchar_t * pszFilePath,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	SSmartPointer<SFileInterface>	file =
		SFileOpener::DefaultNewOpenFile( pszFilePath, SFileOpener::shareRead ) ;
	if ( file == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (pszMIME == NULL) || (pszMIME[0] == 0) )
	{
		size_t	iExt = 0 ;
		for ( size_t i = 0; pszFilePath[i] != 0; i ++ )
		{
			if ( pszFilePath[i] == L'.' )
			{
				iExt = i + 1 ;
			}
		}
		SGLImageDecoderInterface *	pDecoder =
			SGLImageDecoderManager::FindDecoder( pszFilePath + iExt ) ;
		if ( pDecoder != NULL )
		{
			if ( !pDecoder->ReadImage( *this, *file, nLimitFrames ) )
			{
				return	sglErrSuccess ;
			}
			file->Seek( 0 ) ;
		}
	}
	return	ReadImage( file, pszMIME, nLimitFrames ) ;
}

SGLError SGLImageObject::ReadImage
	( SSystem::SFileInterface * file,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	SGLImageDecoderInterface *	pDecoder =
		SGLImageDecoderManager::FindDecoderAsMIME( pszMIME ) ;
	if ( pDecoder != NULL )
	{
		if ( !pDecoder->ReadImage( *this, *file, nLimitFrames ) )
		{
			return	sglErrSuccess ;
		}
		file->Seek( 0 ) ;
	}
	return	SGLImageDecoderManager::ReadImage( *this, *file, nLimitFrames ) ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::SaveImage
	( const wchar_t * pszFilePath, const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	SSmartPointer<SFileInterface>	file =
		SFileOpener::DefaultNewOpenFile( pszFilePath, SFileOpener::modeCreate ) ;
	if ( file == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (pszMIME == NULL) || (pszMIME[0] == 0) )
	{
		size_t	iExt = 0 ;
		for ( size_t i = 0; pszFilePath[i] != 0; i ++ )
		{
			if ( pszFilePath[i] == L'.' )
			{
				iExt = i + 1 ;
			}
		}
		SString	strMIME ;
		SGLImageEncoderInterface *	pEncoder =
			SGLImageEncoderManager::FindEncoderAsExt( pszFilePath + iExt, strMIME ) ;
		if ( pEncoder != NULL )
		{
			return	pEncoder->WriteImage( *file,  *this, strMIME, pOpt ) ;
		}
	}
	return	SGLImageEncoderManager::WriteImage
							( *file, *this, pszMIME, pOpt ) ;
}

SGLError SGLImageObject::WriteImage
	( SSystem::SFileInterface * file,
		const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	return	SGLImageEncoderManager::WriteImage
							( *file, *this, pszMIME, pOpt ) ;
}

// ピクセル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::SetPixel( int xPos, int yPos, const SGLPalette& pxcmp )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	if ( ((uint32_t) xPos >= imgDst.width)
		|| ((uint32_t) yPos >= imgDst.height) )
	{
		return	sglErrFailed ;
	}
	uint8_t *	pbytPixel =
					imgDst.ptrBuffer + imgDst.pitchLine * yPos
									+ imgDst.pitchPixel * xPos ;
	switch ( imgDst.depth )
	{
	case	32:
		*((uint32_t*)pbytPixel) = pxcmp.ui32 ;
		break ;
	case	24:
		pbytPixel[0] = (uint8_t) pxcmp.ui32 ;
		pbytPixel[1] = (uint8_t) (pxcmp.ui32 >> 8) ;
		pbytPixel[2] = (uint8_t) (pxcmp.ui32 >> 16) ;
		break ;
	case	16:
		*((uint16_t*)pbytPixel) = (uint16_t) pxcmp.ui32 ;
		break ;
	case	8:
		pbytPixel[0] = (uint8_t) pxcmp.ui32 ;
		break ;
	default:
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError SGLImageObject::SetPixelRGBA( int xPos, int yPos, const SGLPalette& pxRGBA )
{
	SGLImageInfo	infImage ;
	SGLError		err = GetImageInfo( infImage ) ;
	if ( err )
	{
		return	err ;
	}
	SGLPalette	rgbaColor = pxRGBA ;
	PROC_CONVERT_COLOR_FORMAT
		pfnColorFmt = sglGetColorFormatConvertor
							( infImage.format, formatImageARGB ) ;
	if ( pfnColorFmt != NULL )
	{
		pfnColorFmt( &rgbaColor, &pxRGBA, 1 ) ;
	}
	SGLPalette	rgbaDst = rgbaColor ;
	PROC_ENCODE_PIXEL_COMPOSITION
		pfnPixelComp = sglGetPixelCompositionEncoder
							( infImage.format, infImage.depth ) ;
	if ( pfnPixelComp != NULL )
	{
		pfnPixelComp( (uint8_t*) &rgbaDst, &rgbaColor, 1 ) ;
	}
	return	SetPixel( xPos, yPos, rgbaDst ) ;
}

// ピクセル取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::GetPixel( SGLPalette& pxcmp, int xPos, int yPos )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	if ( ((uint32_t) xPos >= imgDst.width)
		|| ((uint32_t) yPos >= imgDst.height) )
	{
		return	sglErrFailed ;
	}
	uint8_t *	pbytPixel =
					imgDst.ptrBuffer + imgDst.pitchLine * yPos
									+ imgDst.pitchPixel * xPos ;
	switch ( imgDst.depth )
	{
	case	32:
		pxcmp.ui32 = *((uint32_t*)pbytPixel) ;
		break ;
	case	24:
		pxcmp.ui32 = pbytPixel[0]
					| ((uint32_t) pbytPixel[1] << 8)
					| ((uint32_t) pbytPixel[2] << 16) ;
		break ;
	case	16:
		pxcmp.ui32 = *((uint16_t*)pbytPixel) ;
		break ;
	case	8:
		pxcmp.ui32 = pbytPixel[0] ;
		break ;
	default:
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError SGLImageObject::GetPixelRGBA( SGLPalette& pxRGBA, int xPos, int yPos )
{
	SGLPalette	rgbaTemp ;
	SGLError	err = GetPixel( rgbaTemp, xPos, yPos ) ;
	if ( err )
	{
		return	err ;
	}
	SGLImageInfo	infImage ;
	err = GetImageInfo( infImage ) ;
	if ( err )
	{
		return	err ;
	}
	SGLPalette	rgbaSrc = rgbaTemp ;
	PROC_DECODE_PIXEL_COMPOSITION
		pfnPixelComp = sglGetPixelCompositionDecoder
							( infImage.format, infImage.depth ) ;
	if ( pfnPixelComp != NULL )
	{
		pfnPixelComp( &rgbaSrc, (const uint8_t*) &rgbaTemp, 1 ) ;
	}
	PROC_CONVERT_COLOR_FORMAT
		pfnColorFmt = sglGetColorFormatConvertor
							( formatImageARGB, infImage.format ) ;
	if ( pfnColorFmt != NULL )
	{
		pfnColorFmt( &pxRGBA, &rgbaSrc, 1 ) ;
	}
	else
	{
		pxRGBA = rgbaSrc ;
	}
	return	sglErrSuccess ;
}

// 画像フィル
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::FillImage
	( const SGLPalette& pxcmp, const SGLImageRect * pRect )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	return	sglFillImageBuffer( imgDst, pxcmp, pRect ) ;
}

// 画像複製
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CopyImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglCopyImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglBlendImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 画像描画（逆順）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendBackImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglBlendBackImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 画像描画（全チャネル加算）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendAddImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglAdditionalBlendImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 画像描画（全チャネル乗算）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendMulImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglMultiplierBlendImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 半透明描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::HalfBlendImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglHalfBlendImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// フォーマット変換
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::ConvertImage
	( SGLImageObject * pSrcImage,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglConvertImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
}

// 背景色合成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendImageBackgroundColor( const SGLPalette& argbBackColor )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	return	sglBlendImageBackgroundColor( imgDst, argbBackColor ) ;
}

// RGB をαチャネルで積算
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::MultiplyImageRGBAlpha( void )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	return	sglMultiplyImageRGBAlpha( imgDst ) ;
}

// チャネル転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::PutImageChannelTo
	( int iDstChannel,
		SGLImageObject * pSrcImage, int iSrcChannel,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglPutImageChannelTo
		( imgDst, iDstChannel, imgSrc, iSrcChannel, xDst, yDst, pSrcRect ) ;
}

// チャネル積和転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::PutImageMAddChannelTo
	( int iDstChannel,
		SGLImageObject * pSrcImage, const SGLPalette& rgbaMul,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglPutImageMAddChannelTo
		( imgDst, iDstChannel, imgSrc, rgbaMul, xDst, yDst, pSrcRect ) ;
}

// αチャネルを一次関数を伴って積算合成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::BlendWithAlphaChannel
	( SGLImageObject * pAlphaImage,
		int32_t fxAlphaCoefficient,
		int32_t fxAlphaIntercept,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	SGLImageBuffer		imgDst, imgAlpha ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgAlpha, pAlphaImage, lockRead ) ;
	//
	return	sglBlendWithAlphaChannel
		( imgDst, imgAlpha,
			fxAlphaCoefficient, fxAlphaIntercept, xDst, yDst, pSrcRect ) ;
}

// トーンカーブ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::ApplyToneFilter
	( const uint8_t * pRedTone, const uint8_t * pGreenTone,
		const uint8_t * pBlueTone, const uint8_t * pAlphaTone,
		const SGLImageRect * pDstRect )
{
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	//
	return	sglApplyToneImageFilter
		( imgDst, pDstRect, pRedTone, pGreenTone, pBlueTone, pAlphaTone ) ;
}

// 輝度トーンカーブ : (1 - x) * v [v > 0], x * (v + 1) [v < 0]
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::MakeBrightnessTone( uint8_t * pTone, float32_t v )
{
	sglMakeBrightnessToneFilter
		( pTone, esl_clampi( eslRoundR32ToInt( v * 0x100 ), -0x100, 0x100 ) ) ;
}

// 積算トーンカーブ : x * v
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::MakeMultipleTone( uint8_t * pTone, float32_t v )
{
	sglMakeMultipleToneFilter
		( pTone, esl_clampi( eslRoundR32ToInt( v * 0x100 ), 0, 0x10000 ) ) ;
}

// 加算トーンカーブ : x + v
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::MakeAdditionalTone( uint8_t * pTone, float32_t v )
{
	sglMakeAdditionalToneFilter
		( pTone, eslRoundR32ToInt( v * 0x100 ) ) ;
}

// 積算トーンカーブ : (x - 0.5) * v + 0.5
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::MakeOffsetMultipleTone( uint8_t * pTone, float32_t v )
{
	sglMakeOffsetMultipleToneFilter
		( pTone, eslRoundR32ToInt( v * 0x100 ) ) ;
}

// ガンマカーブ : x ^ (1 / v)
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::MakeGammaTone( uint8_t * pTone, float32_t v )
{
	sglMakeGammaToneFilter
		( pTone, eslRoundR32ToInt( v * 0x100 ) ) ;
}

// 1/2 縮小
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::EnlargeHalfImage( SGLImageObject * pSrcImage )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglEnlargeHalfImageBuffer( imgDst, imgSrc ) ;
}

// 90度単位回転（反時計回り）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::OrthogonalRotate
	( SGLImageObject * pSrcImage, int degRotateAngle )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglOrthogonalRotateImageBuffer( imgDst, imgSrc, degRotateAngle ) ;
}

// グレイスケールを ARGB に拡張変換する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateColorImageFromGrayscale
	( SGLImageObject * pGrayscale, const SGLPalette& argbColor )
{
	if ( pGrayscale == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	SGLSize	sizeImage = pGrayscale->GetImageSize() ;
	CreateImage( sizeImage.w, sizeImage.h, formatImageARGB, 32 ) ;
	FillImage( argbColor ) ;
	//
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pGrayscale, lockRead ) ;
	return	sglMultiplierARGBxGrayBuffer( imgDst, imgSrc ) ;
}

// ビットマスク画像を 1/8 縮小して Grayscale 画像に変換する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::MultisampleGrayscaleFromBitmask( SGLImageObject * pSrcImage )
{
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, this ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, lockRead ) ;
	//
	return	sglMultisampleBitmaskToGrayscale( imgDst, imgSrc ) ;
}

// 多角形（凹形状可能）グレイスケール画像生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateFilledPolygonShape
	( S2DVector& vMakedOffset,
		const S2DVector * pVertices, size_t nCount, float32_t fpUnit )
{
	if ( nCount < 3 )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 領域計算
	//
	S2DVector	vMin = pVertices[0] ;
	S2DVector	vMax = vMin ;
	for ( size_t i = 1; i < nCount; i ++ )
	{
		vMin.x = esl_fminf( vMin.x, pVertices[i].x ) ;
		vMin.y = esl_fminf( vMin.y, pVertices[i].y ) ;
		vMax.x = esl_fmaxf( vMax.x, pVertices[i].x ) ;
		vMax.y = esl_fmaxf( vMax.y, pVertices[i].y ) ;
	}
	vMakedOffset.x = (float32_t) floor( vMin.x / fpUnit ) * fpUnit ;
	vMakedOffset.y = (float32_t) floor( vMin.y / fpUnit ) * fpUnit ;
	//
	// 座標修正
	//
	SArray<S2DVector>	bufVertex ;
	S2DVector *	pVertexBuf = bufVertex.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pVertexBuf[i] = (pVertices[i] - vMakedOffset) * 8.0f ;
	}
	//
	// ビットマスクバッファ作成
	//
	SGLSize		sizeGrayscale( esl_roundfi( vMax.x - vMakedOffset.x + 1.0f ),
								esl_roundfi( vMax.y - vMakedOffset.y + 1.0f ) ) ;
	CreateImage( sizeGrayscale.w, sizeGrayscale.h, formatImageGray, 8 ) ;
	//
	SGLImage	imgBitmask ;
	imgBitmask.CreateImage
		( sizeGrayscale.w * 8, sizeGrayscale.h * 8, formatImageGray, 1 ) ;
	//
	// ビットマスク生成
	//
	SGLImageBuffer		imgbufBitmask ;
	SGLImageSmartBuffer	isbBitmask( imgbufBitmask, &imgBitmask ) ;
	//
	sglFillPolygonBitmask( imgbufBitmask, pVertexBuf, nCount ) ;
	//
	// ビットマスクからグレイスケールへ変換
	//
	SGLImageBuffer		imgbufGrayscale ;
	SGLImageSmartBuffer	isbGrayscale( imgbufGrayscale, this ) ;
	//
	SGLError	err = sglMultisampleBitmaskToGrayscale( imgbufGrayscale, imgbufBitmask ) ;
	//
	bufVertex.FinishArray() ;
	//
	return	err ;
}

// ベジェ曲線閉鎖領域グレイスケール画像生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateFilledBezierShape
	( S2DVector& vMakedOffset,
		const S2DVector * pVertices, size_t nCount, float32_t fpUnit )
{
	if ( nCount < 4 )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 領域計算
	//
	S2DVector	vMin = pVertices[0] ;
	S2DVector	vMax = vMin ;
	for ( size_t i = 1; i < nCount; i ++ )
	{
		vMin.x = esl_fminf( vMin.x, pVertices[i].x ) ;
		vMin.y = esl_fminf( vMin.y, pVertices[i].y ) ;
		vMax.x = esl_fmaxf( vMax.x, pVertices[i].x ) ;
		vMax.y = esl_fmaxf( vMax.y, pVertices[i].y ) ;
	}
	vMakedOffset.x = (float32_t) floor( vMin.x / fpUnit ) * fpUnit ;
	vMakedOffset.y = (float32_t) floor( vMin.y / fpUnit ) * fpUnit ;
	//
	// 座標修正
	//
	SArray<S2DVector>	bufVertex ;
	S2DVector *	pVertexBuf = bufVertex.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pVertexBuf[i] = (pVertices[i] - vMakedOffset) * 8.0f ;
	}
	//
	// ビットマスクバッファ作成
	//
	SGLSize		sizeGrayscale( esl_roundfi( vMax.x - vMakedOffset.x + 1.0f ),
								esl_roundfi( vMax.y - vMakedOffset.y + 1.0f ) ) ;
	CreateImage( sizeGrayscale.w, sizeGrayscale.h, formatImageGray, 8 ) ;
	//
	SGLImage	imgBitmask ;
	imgBitmask.CreateImage
		( sizeGrayscale.w * 8, sizeGrayscale.h * 8, formatImageGray, 1 ) ;
	//
	// ビットマスク生成
	//
	SGLImageBuffer		imgbufBitmask ;
	SGLImageSmartBuffer	isbBitmask( imgbufBitmask, &imgBitmask ) ;
	//
	sglHatchBezierBitmask( imgbufBitmask, pVertexBuf, nCount ) ;
	//
	// ビットマスクからグレイスケールへ変換
	//
	SGLImageBuffer		imgbufGrayscale ;
	SGLImageSmartBuffer	isbGrayscale( imgbufGrayscale, this ) ;
	//
	SGLError	err = sglMultisampleBitmaskToGrayscale( imgbufGrayscale, imgbufBitmask ) ;
	//
	bufVertex.FinishArray() ;
	//
	return	err ;
}

// 参照元画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImageObject::GetImageReference
							( SGLImageRect& rectRef, ssize_t iFrame )
{
	rectRef.w = 0 ;
	rectRef.h = 0 ;
	return	NULL ;
}

// 参照先画像変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::MakeImageReference
	( size_t iFrame,
		SGLImageObject * pAtlasImage, const SGLImageRect& rectRef )
{
	return	sglErrFailed ;
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBufferInterface *
	SGLImageObject::CommitImageObject
			( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage )
{
	return	NULL ;
}

// 画像オブジェクト追加登録
//////////////////////////////////////////////////////////////////////////////
bool SGLImageObject::AddImageObject
	( SGLImageBufferInterface * pObject, bool fOriginalImage )
{
	return	false ;
}

// 画像オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
bool SGLImageObject::DetachImageObject( SGLImageBufferInterface * pObject )
{
	return	false ;
}

// 画像オブジェクト更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::UpdateImageObject( const SGLImageRect * pUpdateRect )
{
	return	sglErrFailed ;
}

// 画像オブジェクト更新確定
//////////////////////////////////////////////////////////////////////////////
void SGLImageObject::FlushImageObject( void )
{
}

// 画像オブジェクト更新反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::ReflectImageObject( uint32_t idType )
{
	return	sglErrFailed ;
}

// 画像オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::DeleteImageObjectTypeOf( uint32_t idType )
{
	return	sglErrFailed ;
}

SGLError SGLImageObject::DeleteAllImageObjects( void )
{
	return	sglErrFailed ;
}

// 画像オブジェクトクリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::NotifyClearImageObject( SGLPalette pxClear )
{
	return	sglErrFailed ;
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
Image * SGLImageObject::GetImageObject( void ) const
{
#if	defined(__COTOPHA__)
	return	NULL ;
#else
	return	(SGLImageObject*) this ;
#endif
}

// 内部バッファ取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SGLImageObject::GetImageBuffer( void ) const
{
	return	NULL ;
}

// 画像識別子
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLImageObject::GetImageIdentity( void ) const
{
	SGLImageBuffer *	pImageBuf = GetImageBuffer() ;
	if ( pImageBuf == nullptr )
	{
		return	nullptr ;
	}
	const wchar_t *	pwszID = nullptr ;
	QuickLock() ;
	for ( ; ; )
	{
		pwszID = pImageBuf->pwszIdentity ;
		if ( pwszID != nullptr )
		{
			break ;
		}
		pImageBuf = pImageBuf->ptrRefOriginal ;
	}
	QuickUnlock() ;
	return	pwszID ;
}

void SGLImageObject::SetImageIdentity( const wchar_t * pwszID )
{
	SGLImageBuffer *	pImageBuf = GetImageBuffer() ;
	QuickLock() ;
	if ( pImageBuf != nullptr )
	{
		SetImageBufferIdentity( pImageBuf, pwszID ) ;
		//
		wchar_t *	pwszBufID = pImageBuf->pwszIdentity ;
		if ( pwszBufID != nullptr )
		{
			for ( ; ; )
			{
				pImageBuf = pImageBuf->ptrRefOriginal ;
				if ( pImageBuf == nullptr )
				{
					break ;
				}
				if ( pImageBuf->pwszIdentity == nullptr )
				{
					sglAddRefMemory( (uint8_t*) pwszBufID ) ;
					pImageBuf->pwszIdentity = pwszBufID ;
				}
			}
		}
	}
	QuickUnlock() ;
}

#if	defined(__PLATFORM_WINDOWS__)
// 画像をデバイスコンテキストへ描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::DrawToDC
	( HDC hDC, int xPos, int yPos,
		const SGLSize * pDstSize, const SGLImageRect * pSrcView, DWORD dwROP )
{
	SGLImageWin32DIBitmap *	pDIB = SGLImageWin32DIBitmap::CommitDIB( this ) ;
	if ( pDIB == NULL )
	{
		return	sglErrFailed ;
	}
	SGLSize			sizeDst = GetImageSize() ;
	SGLImageRect	rectSrc( 0, 0, sizeDst.w, sizeDst.h ) ;
	if ( pDstSize != NULL )
	{
		sizeDst = *pDstSize ;
	}
	if ( pSrcView != NULL )
	{
		SGLRect	rectTemp = rectSrc ;
		if ( !(rectTemp &= SGLRect( *pSrcView )) )
		{
			return	sglErrFailed ;
		}
		rectSrc = rectTemp ;
	}
	if ( sizeDst != rectSrc.GetSize() )
	{
		if ( ::StretchBlt
			( hDC, xPos, yPos, sizeDst.w, sizeDst.h,
				pDIB->m_hDC, rectSrc.x, rectSrc.y,
							rectSrc.w, rectSrc.h, dwROP ) )
		{
			return	sglErrSuccess ;
		}
	}
	else
	{
		if ( ::BitBlt
			( hDC, xPos, yPos,
				sizeDst.w, sizeDst.h,
				pDIB->m_hDC, rectSrc.x, rectSrc.y, dwROP ) )
		{
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

// HBITMAP から画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateFromHBITMAP( HBITMAP hBitmap, int nFlags )
{
	SGLImageBuffer *	pImageBuf =
			SGLSpriteCursor::ConvertHBITMAPtoImageBuffer( hBitmap ) ;
	if ( pImageBuf == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = CreateCloneBuffer( *pImageBuf, nFlags ) ;
	sglReleaseImageBuffer( pImageBuf ) ;
	return	err ;
}

// HCURSOR から画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateFromHCURSOR( HCURSOR hCursor, int nFlags )
{
	SGLImageBuffer *	pImageBuf =
		SGLSpriteCursor::ConvertHCURSORtoImageBuffer( hCursor ) ;
	if ( pImageBuf == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = CreateCloneBuffer( *pImageBuf, nFlags ) ;
	sglReleaseImageBuffer( pImageBuf ) ;
	return	err ;
}

// HICON から画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObject::CreateFromHICON( HICON hIcon, int nFlags )
{
	SGLImageBuffer *	pImageBuf =
		SGLSpriteCursor::ConvertHCURSORtoImageBuffer( (HCURSOR) hIcon ) ;
	if ( pImageBuf == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = CreateCloneBuffer( *pImageBuf, nFlags ) ;
	sglReleaseImageBuffer( pImageBuf ) ;
	return	err ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト・ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImage, SGLImageObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImage::SGLImage( const SGLImage& img )
{
	m_pImage = NULL ;
	m_flagOwner = false ;
	if ( img.m_pImage != NULL )
	{
		m_pImage = img.m_pImage->NewReference() ;
		m_flagOwner = true ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImage::~SGLImage( void )
{
	if ( m_flagOwner )
	{
		delete	m_pImage ;
	}
	m_pImage = NULL ;
	m_flagOwner = false ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
Image * SGLImage::SetImageObject( Image * pImage, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pImage ;
	}
	m_pImage = pImage ;
	m_flagOwner = flagOwner ;
	return	m_pImage ;
}

// フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::GetFrameCount( void ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->GetFrameCount() ;
	}
	return	0 ;
}

// アニメーション・シーケンス全長取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::GetSequenceLength( void ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->GetSequenceLength() ;
	}
	return	0 ;
}

// アニメーション・シーケンス取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::GetSequenceTable( uint32_t * pSeq, size_t nCount ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->GetSequenceTable( pSeq, nCount ) ;
	}
	return	0 ;
}

// アニメーション全長（時間）取得 [msec]
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLImage::GetTotalTime( void ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->GetTotalTime() ;
	}
	return	0 ;
}

// 時間からフレーム番号へ変換
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::FrameFromMilliSec( uint64_t msec )
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->FrameFromMilliSec( msec ) ;
	}
	return	0 ;
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::SelectFrame( size_t iFrame, int iSide )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->SelectFrame( iFrame, iSide ) ;
}

// 選択中フレーム取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::GetSelectedFrame( int * pSide ) const
{
	if ( m_pImage == NULL )
	{
		if ( pSide != NULL )
		{
			*pSide = stereoImageBoth ;
		}
		return	-1 ;
	}
	return	m_pImage->GetSelectedFrame( pSide ) ;
}

// 画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::GetImageInfo( SGLImageInfo & imginf ) const
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->GetImageInfo( imginf ) ;
}

// パレット・テーブル取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::GetPaletteTable( SGLPalette * pPalette, size_t nCount )
{
	if ( m_pImage == NULL )
	{
		return	0 ;
	}
	return	m_pImage->GetPaletteTable( pPalette, nCount ) ;
}

// 画像バッファ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLImage::LockBuffer
	( SGLImageInfo & imginf, int flags, const SGLImageRect * pRect )
{
	if ( m_pImage == NULL )
	{
		return	NULL ;
	}
	return	m_pImage->LockBuffer( imginf, flags, pRect ) ;
}

SGLError SGLImage::FlushBuffer( int flags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->FlushBuffer( flags ) ;
}

SGLError SGLImage::UnlockBuffer( int flags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->UnlockBuffer( flags ) ;
}

// 画像バッファ読み出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::ReadFrameBuffer
	( const SGLImageInfo & imginf, uint8_t * ptrBuffer, size_t iFrame, int iSide )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->ReadFrameBuffer( imginf, ptrBuffer, iFrame, iSide ) ;
}

// 画像バッファへの参照生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImage::NewReference
	( const SGLImageRect * pClip, ssize_t iFrame, int iSide )
{
	if ( m_pImage == NULL )
	{
		return	NULL ;
	}
	#if	defined(__COTOPHA__)
		Image *	pImage = m_pImage->NewReference( pClip, iFrame, iSide )  ;
		if ( pImage == NULL )
		{
			return	NULL ;
		}
		return	new SGLImage( pImage, true ) ;
	#else
		return	m_pImage->NewReference( pClip, iFrame, iSide ) ;
	#endif
}

// テクスチャのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::NormalizeToTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->NormalizeToTexture( nFlags ) ;
}

SGLError SGLImage::NormalizeToMipmapTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->NormalizeToMipmapTexture( nFlags ) ;
}

// レンダリング・ターゲットのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::NormalizeToRenderTarget( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->NormalizeToRenderTarget( nFlags ) ;
}

// テクスチャのための正規化フラグを除去する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::DenormalizeForTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->DenormalizeForTexture( nFlags ) ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::CreateBuffer
	( const SGLImageInfo& imginf,
		uint64_t nFlags, size_t countFrame, uint64_t msecLong )
{
	if ( m_pImage == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pImage = new Image ;
		#else
			m_pImage = new SGLMultiImage ;
		#endif
		m_flagOwner = true ;
	}
	return	m_pImage->CreateBuffer( imginf, nFlags, countFrame, msecLong ) ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void SGLImage::ReleaseBuffer( void )
{
	if ( m_pImage == NULL )
	{
		return ;
	}
	m_pImage->ReleaseBuffer() ;
}

// バッファフラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLImage::GetBufferFlags( void )
{
	if ( m_pImage == NULL )
	{
		return	0 ;
	}
	return	m_pImage->GetBufferFlags() ;
}

// パレット・テーブル設定
//////////////////////////////////////////////////////////////////////////////
size_t SGLImage::SetPaletteTable
	( const SGLPalette * pPalette, size_t nCount )
{
	if ( m_pImage == NULL )
	{
		return	0 ;
	}
	return	m_pImage->SetPaletteTable( pPalette, nCount ) ;
}

// アニメーション・シーケンス・テーブルの設定
//////////////////////////////////////////////////////////////////////////////
void SGLImage::SetSequenceTable( const uint32_t * pSeq, size_t nCount )
{
	if ( m_pImage == NULL )
	{
		return ;
	}
	m_pImage->SetSequenceTable( pSeq, nCount ) ;
}

// 中心座標情報設定
//////////////////////////////////////////////////////////////////////////////
void SGLImage::SetImageOrigin( int x, int y )
{
	if ( m_pImage == NULL )
	{
		return ;
	}
	m_pImage->SetImageOrigin( x, y ) ;
}

// アニメーション全長時間設定
//////////////////////////////////////////////////////////////////////////////
void SGLImage::SetAnimationDuration( uint64_t nDuration )
{
	if ( m_pImage == NULL )
	{
		return ;
	}
	m_pImage->SetAnimationDuration( nDuration ) ;
}

// 画像フォーマット正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::NormalizeFormat
	( uint32_t format, uint32_t depth,
		uint32_t nFlags, uint32_t width, uint32_t height )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->NormalizeFormat( format, depth, nFlags, width, height ) ;
}

// NormalizeFormat で結合されたアニメーション画像への参照を生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImage::NewAnimationReference
	( SGLImageRect* pFrameRects, size_t nRectsCount )
{
	if ( m_pImage == NULL )
	{
		return	0 ;
	}
	#if	defined(__COTOPHA__)
		Image *	pImage = m_pImage->NewAnimationReference
									( pFrameRects, nRectsCount )  ;
		if ( pImage == NULL )
		{
			return	NULL ;
		}
		return	new SGLImage( pImage, true ) ;
	#else
		return	m_pImage->NewAnimationReference( pFrameRects, nRectsCount ) ;
	#endif
}

// アトラス化された画像の参照矩形を取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::GetReferenceRectOfAtlas
		( SGLImageRect& rect, ssize_t iFrame ) const
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->GetReferenceRectOfAtlas( rect, iFrame ) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::LoadImage
	( const wchar_t * pszFilePath,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	if ( m_pImage == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pImage = new Image ;
		#else
			m_pImage = new SGLMultiImage ;
		#endif
		m_flagOwner = true ;
	}
	return	m_pImage->LoadImage( pszFilePath, pszMIME, nLimitFrames ) ;
}

SGLError SGLImage::ReadImage
	( SSystem::SFileInterface * file,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	if ( m_pImage == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pImage = new Image ;
		#else
			m_pImage = new SGLMultiImage ;
		#endif
		m_flagOwner = true ;
	}
	#if	defined(__COTOPHA__)
		File *	pFile = file->GetFileObject() ;
		if ( pFile != NULL )
		{
			return	m_pImage->ReadImage( pFile, pszMIME, nLimitFrames ) ;
		}
		return	SGLImageObject::ReadImage( file, pszMIME, nLimitFrames ) ;
	#else
		return	m_pImage->ReadImage( file, pszMIME, nLimitFrames ) ;
	#endif
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::WriteImage
	( SSystem::SFileInterface * file,
		const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )

{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	defined(__COTOPHA__)
		return	SGLImageObject::WriteImage( file, pszMIME, pOpt ) ;
	#else
		return	m_pImage->WriteImage( file, pszMIME, pOpt ) ;
	#endif
}

// 参照画像を作成する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::DuplicateOf( Image * pImage )
{
	if ( m_flagOwner )
	{
		delete	m_pImage ;
	}
	if ( pImage != NULL )
	{
		m_pImage = pImage->NewReference() ;
		m_flagOwner = true ;
	}
	else
	{
		m_pImage = NULL ;
		m_flagOwner = false ;
	}
	return	sglErrSuccess ;
}

// 参照先画像変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::MakeImageReference
	( size_t iFrame,
		SGLImageObject * pAtlasImage, const SGLImageRect& rectRef )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pImage->MakeImageReference( iFrame, pAtlasImage, rectRef ) ;
}

// 参照元画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImage::GetImageReference
						( SGLImageRect& rectRef, ssize_t iFrame )
{
	if ( m_pImage == NULL )
	{
		rectRef.w = 0 ;
		rectRef.h = 0 ;
		return	NULL ;
	}
	#if	defined(__COTOPHA__)
		return	this ;
	#else
		return	m_pImage->GetImageReference( rectRef, iFrame ) ;
	#endif
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBufferInterface *
	SGLImage::CommitImageObject
		( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage )
{
	if ( m_pImage == NULL )
	{
		return	NULL ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->CommitImageObject( idType, rectRef, fNoRefImage ) ;
	#else
		return	NULL ;
	#endif
}

// 画像オブジェクト追加登録
//////////////////////////////////////////////////////////////////////////////
bool SGLImage::AddImageObject
		( SGLImageBufferInterface * pObject, bool fOriginalImage )
{
	if ( m_pImage == NULL )
	{
		return	false ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->AddImageObject( pObject, fOriginalImage ) ;
	#else
		return	false ;
	#endif
}

// 画像オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
bool SGLImage::DetachImageObject( SGLImageBufferInterface * pObject )
{
	if ( m_pImage == NULL )
	{
		return	false ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->DetachImageObject( pObject ) ;
	#else
		return	false ;
	#endif
}

// 画像オブジェクト更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::UpdateImageObject( const SGLImageRect * pUpdateRect )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->UpdateImageObject( pUpdateRect ) ;
	#else
		return	sglErrFailed ;
	#endif
}

// 画像オブジェクト更新確定
//////////////////////////////////////////////////////////////////////////////
void SGLImage::FlushImageObject( void )
{
	if ( m_pImage == NULL )
	{
		return ;
	}
	m_pImage->FlushImageObject() ;
}

// 画像オブジェクト更新反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::ReflectImageObject( uint32_t idType )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->ReflectImageObject( idType ) ;
	#else
		return	sglErrFailed ;
	#endif
}

// 画像オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::DeleteImageObjectTypeOf( uint32_t idType )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->DeleteImageObjectTypeOf( idType ) ;
	#else
		return	sglErrFailed ;
	#endif
}

SGLError SGLImage::DeleteAllImageObjects( void )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->DeleteAllImageObjects() ;
	#else
		return	sglErrFailed ;
	#endif
}

// 画像オブジェクトクリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImage::NotifyClearImageObject( SGLPalette pxClear )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	#if	!defined(__COTOPHA__)
		return	m_pImage->NotifyClearImageObject( pxClear ) ;
	#else
		return	sglErrFailed ;
	#endif
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
Image * SGLImage::GetImageObject( void ) const
{
	return	m_pImage ;
}

// 内部バッファ取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SGLImage::GetImageBuffer( void ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage->GetImageBuffer() ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSmartImage, SGLImageObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSmartImage::~SGLSmartImage( void )
{
	if ( m_pImage != NULL )
	{
		m_pImage->NotifyObjectDestroy( this ) ;
		if ( m_flagOwnBuffer )
		{
			sglReleaseImageBuffer( m_pImage ) ;
		}
	}
	m_pImage = NULL ;
	m_flagOwnBuffer = false ;
	//
	if ( m_pbytLocked != NULL )
	{
		sglFreeMemory( m_pbytLocked ) ;
		m_pbytLocked = NULL ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SGLSmartImage::SetImageBuffer( SGLImageBuffer * pImage )
{
	if ( m_flagOwnBuffer && (m_pImage != NULL) )
	{
		sglReleaseImageBuffer( m_pImage ) ;
	}
	m_pImage = pImage ;
	m_flagOwnBuffer = true ;
	return	m_pImage ;
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSmartImage::AttachImageBuffer( SGLImageBuffer * pImage )
{
	if ( m_flagOwnBuffer && (m_pImage != NULL) )
	{
		sglReleaseImageBuffer( m_pImage ) ;
	}
	m_pImage = pImage ;
	m_flagOwnBuffer = false ;
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::SelectFrame( size_t iFrame, int iSide )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer * pImage =
		sglCreateReferenceImageBuffer( m_pImage, NULL, 0, iSide ) ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SetImageBuffer( pImage ) ;
	return	sglErrSuccess ;
}

// 画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::GetImageInfo( SGLImageInfo & imginf ) const
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	imginf = *m_pImage ;
	return	sglErrSuccess ;
}

// パレット・テーブル取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSmartImage::GetPaletteTable( SGLPalette * pPalette, size_t nCount )
{
	if ( m_pImage != NULL )
	{
		if ( (m_pImage->format & formatImageFlagPalette)
			&& (m_pImage->ptrPalette != NULL) )
		{
			if ( nCount > 0x100 )
			{
				nCount = 0x100 ;
			}
			eslMoveMemory
				( pPalette, m_pImage->ptrPalette,
						nCount * sizeof(SGLPalette) ) ;
			return	nCount ;
		}
		else if ( (m_pImage->format & formatImageTypeMask) == formatImageGray )
		{
			if ( nCount > 0x100 )
			{
				nCount = 0x100 ;
			}
			for ( size_t i = 0; i < nCount; i ++ )
			{
				pPalette[i].argb.Red = (uint8_t) i ;
				pPalette[i].argb.Green = (uint8_t) i ;
				pPalette[i].argb.Blue = (uint8_t) i ;
				pPalette[i].argb.Alpha = (uint8_t) i ;
			}
			return	nCount ;
		}
	}
	return	0 ;
}

// 画像バッファ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLSmartImage::LockBuffer
	( SGLImageInfo & imginf, int flags, const SGLImageRect * pRect )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SGLImageRect	irctLock = pImage->GetImageRect() ;
	if ( pRect != NULL )
	{
		irctLock = SGLRect( irctLock ) & SGLRect( *pRect ) ;
		if ( irctLock.IsEmpty() )
		{
			return	NULL ;
		}
		m_rectLocked = *pRect ;
	}
	else
	{
		m_rectLocked.x = 0 ;
		m_rectLocked.y = 0 ;
		m_rectLocked.w = m_pImage->width ;
		m_rectLocked.h = m_pImage->height ;
	}
	SGLImageInfo *	pRefImage ;
	uint8_t *	pbytBuf =
			SGLImageSystemMemory::GetMemoryOf( pImage, pRefImage ) ;
	imginf.format = pImage->format ;
	imginf.depth = pImage->depth ;
	imginf.width = irctLock.w ;
	imginf.height = irctLock.h ;
	if ( pbytBuf == NULL )
	{
		if ( flags & lockWrite )
		{
			pbytBuf =
				SGLImageSystemMemory::CommitMemoryOf( pImage, pRefImage ) ;
		}
		if ( pbytBuf == NULL )
		{
			return	NULL ;
		}
		imginf.pitchPixel = pRefImage->pitchPixel ;
		imginf.pitchLine = pRefImage->pitchLine ;
	}
	uint8_t *	pbytLocked = LockImageBuffer( pImage ) ;
	QuickLock() ;
	if ( m_pbytLocked != NULL )
	{
		ESLAssert( m_pbytLocked == pbytLocked ) ;
		sglFreeMemory( m_pbytLocked ) ;
	}
	m_pbytLocked = pbytLocked ;
	AtomicAdd( &m_countLocked, 1 ) ;
	QuickUnlock() ;
	//
	imginf.ptOrigin = pImage->ptOrigin ;
	imginf.pitchPixel = pRefImage->pitchPixel ;
	imginf.pitchLine = pRefImage->pitchLine ;
	return	pbytBuf + (imginf.pitchPixel * irctLock.x
						+ imginf.pitchLine * irctLock.y) ;
}

SGLError SGLSmartImage::FlushBuffer( int flags )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	if ( flags & lockWrite )
	{
		return	pImage->UpdateImageObject( &m_rectLocked ) ;
//		return	pImage->CommitImageObject() ;
	}
	return	sglErrSuccess ;
}

SGLError SGLSmartImage::UnlockBuffer( int flags )
{
	ESLAssert( m_countLocked > 0 ) ;
	SGLImageBuffer *	pImage = m_pImage ;
	if ( (m_countLocked <= 0) || (pImage == NULL) )
	{
		return	sglErrFailed ;
	}
	QuickLock() ;
	AtomicSub( &m_countLocked, 1 ) ;
	if ( m_pbytLocked != NULL )
	{
		sglFreeMemory( m_pbytLocked ) ;
		m_pbytLocked = NULL ;
	}
	QuickUnlock() ;
	if ( flags & lockWrite )
	{
		pImage->UpdateImageObject( &m_rectLocked ) ;
		return	pImage->CommitImageObject() ;
	}
	return	sglErrSuccess ;
}

// 画像バッファ読み出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::ReadFrameBuffer
	( const SGLImageInfo & imginf, uint8_t * ptrBuffer, size_t iFrame, int iSide )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError		err ;
	SGLImageBuffer	imgDstBuf = imginf ;
	imgDstBuf.ptrBuffer = ptrBuffer ;
	//
	if ( pImage->format & formatImageFlagSideBySide )
	{
		SGLImageBuffer *	pRefImage =
			sglCreateReferenceImageBuffer( pImage, NULL, 0, iSide ) ;
		if ( pRefImage == NULL )
		{
			return	sglErrFailed ;
		}
		err = sglCopyImageBuffer( imgDstBuf, *pRefImage ) ;
		sglReleaseImageBuffer( pRefImage ) ;
	}
	else
	{
		err = sglCopyImageBuffer( imgDstBuf, *pImage ) ;
	}
	return	err ;
}

// 画像バッファへの参照生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSmartImage::NewReference
	( const SGLImageRect * pClip, ssize_t iFrame, int iSide )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new SGLSmartImage
				( sglCreateReferenceImageBuffer( pImage, pClip, 0, iSide ) ) ;
}

// テクスチャのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::NormalizeToTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	uint64_t	nBufFlags = ConvertFlagsToBufferFlags(nFlags)
							| flagImageNormalDepth ;
	if ( nFlags & bufferNonPowerOf2 )
	{
		nBufFlags |= flagImageNormalPitch ;
	}
	else
	{
		nBufFlags |= flagImageNormalSize ;
	}
	return	sglNormalizeImageBuffer( m_pImage, nBufFlags ) ;
}

SGLError SGLSmartImage::NormalizeToMipmapTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	uint64_t	nBufFlags = ConvertFlagsToBufferFlags(nFlags)
							| flagImageNormalDepth | flagImageMipmap ;
	if ( nFlags & bufferNonPowerOf2 )
	{
		nBufFlags |= flagImageNormalPitch ;
	}
	else
	{
		nBufFlags |= flagImageNormalSize ;
	}
	return	sglNormalizeImageBuffer( m_pImage, nBufFlags ) ;
}

// レンダリング・ターゲットのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::NormalizeToRenderTarget( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	sglNormalizeImageBuffer
		( m_pImage, (flagImageNormalDepth | flagImageNormalSize) ) ;
}

// テクスチャのための正規化フラグを除去する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::DenormalizeForTexture( uint32_t nFlags )
{
	if ( m_pImage == NULL )
	{
		return	sglErrFailed ;
	}
	nFlags &= GetBufferFlags() ;
	if ( nFlags != 0 )
	{
		int	nBufFlags =
				ConvertFlagsToBufferFlags( nFlags )
					& (flagImageMipmap
						| flagImageCubemap | flagImageTexture3D
						| flagImageTextureArray
						| flagImageTextureMultisample
						| flagImageRenderBufferStorage
						| flagImageCompressedTexture
						| flagImageNeedSampleNoSmooth
						| flagImageNeedSampleTiling) ;
		if ( nBufFlags != 0 )
		{
			SGLImageBuffer *	pOrg = m_pImage ;
			QuickLock() ;
			while ( pOrg->ptrRefOriginal != NULL )
			{
				pOrg = pOrg->ptrRefOriginal ;
			}
			QuickUnlock() ;
			//
			m_pImage->flagsBuffer &= ~nBufFlags ;
			m_pImage->DeleteAllImageObject() ;
			//
			if ( m_pImage != pOrg )
			{
				pOrg->flagsBuffer &= ~nBufFlags ;
				pOrg->DeleteAllImageObject() ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::CreateBuffer
	( const SGLImageInfo& imginf,
		uint64_t nFlags, size_t countFrame, uint64_t msecLong )
{
	ReleaseBuffer() ;
	//
	int	sglFlags = 0 ;
	if ( nFlags & (bufferForTexture | bufferForRenderTarget) )
	{
		if ( nFlags & (bufferNonPowerOf2 | bufferForRenderTarget) )
		{
			sglFlags |= flagImageNormalDepth | flagImageNormalPitch ;
		}
		else
		{
			sglFlags |= flagImageNormalDepth | flagImageNormalSize ;
		}
	}
	if ( nFlags & bufferOnDeviceOnly )
	{
		sglFlags |= flagImageNoBuffer
					| flagImageNoReadBuffer | flagImageNoWriteBuffer ;
	}
	sglFlags |= ConvertFlagsToBufferFlags( nFlags ) ;
	//
	SetImageBuffer( sglCreateImageBuffer( imginf, sglFlags ) ) ;
	return	sglErrSuccess ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void SGLSmartImage::ReleaseBuffer( void )
{
	if ( m_flagOwnBuffer && (m_pImage != NULL) )
	{
		sglReleaseImageBuffer( m_pImage ) ;
	}
	m_pImage = NULL ;
	m_flagOwnBuffer = false ;
}

// バッファフラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSmartImage::GetBufferFlags( void )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	0 ;
	}
	int	nFlags = 0 ;
	if ( pImage->ptrBuffer == NULL )
	{
		nFlags |= bufferOnDeviceOnly ;
	}
	//
	nFlags |= ConvertFlagsFromBufferFlags( pImage->flagsBuffer ) ;
	//
	if ( pImage->flagsBuffer & flagImageNormalDepth )
	{
		nFlags |= bufferForTexture ;
		if ( !(pImage->flagsBuffer & flagImageNormalSize) )
		{
			nFlags |= bufferNonPowerOf2 ;
		}
	}
	return	nFlags ;
}

// パレット・テーブル設定
//////////////////////////////////////////////////////////////////////////////
size_t SGLSmartImage::SetPaletteTable
	( const SGLPalette * pPalette, size_t nCount )
{
	const SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	0 ;
	}
	if ( (pImage->format & formatImageFlagPalette)
		&& (pImage->ptrPalette != NULL) )
	{
		if ( nCount > 0x100 )
		{
			nCount = 0x100 ;
		}
		eslMoveMemory
			( pImage->ptrPalette,
				pPalette, nCount * sizeof(SGLPalette) ) ;
	}
	else
	{
		return	0 ;
	}
	return	nCount ;
}

// 中心座標情報設定
//////////////////////////////////////////////////////////////////////////////
void SGLSmartImage::SetImageOrigin( int x, int y )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage != NULL )
	{
		pImage->ptOrigin.x = x ;
		pImage->ptOrigin.y = y ;
	}
}

// アニメーション全長時間設定
//////////////////////////////////////////////////////////////////////////////
void SGLSmartImage::SetAnimationDuration( uint64_t nDuration )
{
}

// SGLImageObject::BufferTypeFlag -> SGLBufferCreationFlag 変換
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSmartImage::ConvertFlagsToBufferFlags( uint64_t nTypeFlags )
{
	uint64_t	sglFlags = 0 ;
	if ( nTypeFlags & bufferNoReadable )
	{
		sglFlags |= flagImageNoReadBuffer ;
	}
	if ( nTypeFlags & bufferNoWritable )
	{
		sglFlags |= flagImageNoWriteBuffer ;
	}
	if ( nTypeFlags & bufferForMipmapTexture )
	{
		sglFlags |= flagImageMipmap ;
	}
	if ( nTypeFlags & bufferCubeMapTexture )
	{
		sglFlags |= flagImageCubemap ;
	}
	if ( nTypeFlags & bufferTexture3D )
	{
		sglFlags |= flagImageTexture3D ;
	}
	if ( nTypeFlags & bufferTextureArray )
	{
		sglFlags |= flagImageTextureArray ;
	}
	if ( nTypeFlags & bufferTextureStorage )
	{
		sglFlags |= flagImageStorage ;
	}
	if ( nTypeFlags & bufferMultisample )
	{
		sglFlags |= flagImageTextureMultisample ;
	}
	if ( nTypeFlags & bufferDeviceRenderBuffer )
	{
		sglFlags |= flagImageRenderBufferStorage ;
	}
	if ( nTypeFlags & bufferSampleNoSmooth )
	{
		sglFlags |= flagImageNeedSampleNoSmooth ;
	}
	if ( nTypeFlags & bufferSampleTiling )
	{
		sglFlags |= flagImageNeedSampleTiling ;
	}
	if ( nTypeFlags & bufferCompressedTexture )
	{
		sglFlags |= flagImageCompressedTexture ;
		sglFlags |= ((nTypeFlags & bufferCompressionFormatMask)
						>> bufferCompressionFormatShifter)
						<< flagImageCompressionFormatShifter ;
	}
	return	sglFlags ;
}

// SGLBufferCreationFlag -> SGLImageObject::BufferTypeFlag 変換
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSmartImage::ConvertFlagsFromBufferFlags( uint64_t nBufFlags )
{
	uint64_t	nFlags = 0 ;
	if ( nBufFlags & flagImageNoReadBuffer )
	{
		nFlags |= bufferNoReadable ;
	}
	if ( nBufFlags & flagImageNoWriteBuffer )
	{
		nFlags |= bufferNoWritable ;
	}
	if ( nBufFlags & flagImageMipmap )
	{
		nFlags |= bufferForMipmapTexture ;
	}
	if ( nBufFlags & flagImageCubemap )
	{
		nFlags |= bufferCubeMapTexture ;
	}
	if ( nBufFlags & flagImageTexture3D )
	{
		nFlags |= bufferTexture3D ;
	}
	if ( nBufFlags & flagImageTextureArray )
	{
		nFlags |= bufferTextureArray ;
	}
	if ( nBufFlags & flagImageStorage )
	{
		nFlags |= bufferTextureStorage ;
	}
	if ( nBufFlags & flagImageTextureMultisample )
	{
		nFlags |= bufferMultisample ;
	}
	if ( nBufFlags & flagImageRenderBufferStorage )
	{
		nFlags |= bufferDeviceRenderBuffer ;
	}
	if ( nBufFlags & flagImageNeedSampleNoSmooth )
	{
		nFlags |= bufferSampleNoSmooth ;
	}
	if ( nBufFlags & flagImageNeedSampleTiling )
	{
		nFlags |= bufferSampleTiling ;
	}
	if ( nBufFlags & flagImageCompressedTexture )
	{
		nFlags |= bufferCompressedTexture ;
		nFlags |= ((nBufFlags & flagImageCompressionFormatMask)
						>> flagImageCompressionFormatShifter)
						<< bufferCompressionFormatShifter ;
	}
	return	nFlags ;
}

// 画像フォーマット正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::NormalizeFormat
	( uint32_t format, uint32_t depth,
		uint32_t nFlags, uint32_t width, uint32_t height )
{
	SGLImageBuffer *	pNewImage =
			NormalizeBufferFormat
				( m_pImage, format, depth, nFlags, width, height ) ;
	if ( pNewImage != m_pImage )
	{
		SetImageBuffer( pNewImage ) ;
	}
	return	sglErrSuccess ;
}

SGLImageBuffer * SGLSmartImage::NormalizeBufferFormat
	( SGLImageBuffer * pImageBuf,
		uint32_t format, uint32_t depth,
		uint32_t nFlags, uint32_t width, uint32_t height )
{
	if ( pImageBuf == NULL )
	{
		return	NULL ;
	}
	if ( format == 0 )
	{
		format = pImageBuf->format ;
	}
	if ( depth == 0 )
	{
		depth = pImageBuf->depth ;
	}
	if ( (format & formatImageFlagS3TC)
		&& !(pImageBuf->format & formatImageFlagS3TC)
		&& ((pImageBuf->width % 4) == 0)
		&& ((pImageBuf->height % 4) == 0) )
	{
		// S3TC 圧縮
		SGLImageBuffer *	pTempImageBuf = nullptr ;
		if ( pImageBuf->depth != 32 )
		{
			// RGB-32 / RGBA-32 へ変換
			SGLImageInfo	imginf = *pImageBuf ;
			imginf.depth = 32 ;
			//
			pTempImageBuf =
				sglCreateImageBuffer( imginf, pImageBuf->flagsBuffer ) ;
			sglConvertImageBuffer( *pTempImageBuf, *pImageBuf ) ;
			pImageBuf = pTempImageBuf ;
		}

		// 圧縮
		SGLImageInfo	imginf = *pImageBuf ;
		imginf.format = format ;
		imginf.depth = sizeof(S3TC_DXT1_Block) * 8 / 16 ;
		imginf.pitchPixel = 1 ;
		imginf.pitchLine = imginf.depth * imginf.width / 8 ;
		//
		S3TC_DitheringMethod	dithering = s3tcErrorDiffusion ;
		if ( nFlags & formatNoDithering )
		{
			dithering = s3tcNoDithering ;
		}
		//
		SGLImageBuffer *	pNewImage =
			sglCreateImageBuffer
				( imginf, pImageBuf->flagsBuffer
							& ~(flagImageNormalDepth | flagImageNormalSize
								| flagImageNormalPitch | flagImageNoBuffer) ) ;
		SetImageBufferIdentity( pNewImage, pImageBuf->pwszIdentity ) ;
		//
		sglCompressARGBtoS3TC_DXT1( *pNewImage, *pImageBuf, dithering ) ;
		pImageBuf = pNewImage ;
		//
		if ( pTempImageBuf != nullptr )
		{
			sglReleaseImageBuffer( pTempImageBuf ) ;
		}
	}
	else if ( (format != pImageBuf->format) || (depth != pImageBuf->depth) )
	{
		// フォーマット変換
		if ( (nFlags & formatStraightPixel) && (depth == pImageBuf->depth) )
		{
			pImageBuf->format = format ;
		}
		else
		{
			SGLImageInfo	imginf = *pImageBuf ;
			imginf.format = format ;
			imginf.depth = depth ;
			//
			SGLImageBuffer *	pNewImage =
					sglCreateImageBuffer( imginf, pImageBuf->flagsBuffer ) ;
			SetImageBufferIdentity( pNewImage, pImageBuf->pwszIdentity ) ;
			//
			sglConvertImageBuffer( *pNewImage, *pImageBuf ) ;
			pImageBuf = pNewImage ;
		}
	}
	if ( nFlags & formatCastSize )
	{
		// 部分参照（サイズ縮小）
		if ( pImageBuf->width < width )
		{
			width = pImageBuf->width ;
		}
		if ( pImageBuf->height < height )
		{
			height = pImageBuf->height ;
		}
		if ( (pImageBuf->width != width) || (pImageBuf->height != height) )
		{
			SGLImageRect	rectCast ;
			rectCast.x = 0 ;
			rectCast.y = 0 ;
			rectCast.w = width ;
			rectCast.h = height ;
			//
			if ( nFlags & formatCastSizeBottom )
			{
				rectCast.y += pImageBuf->height - height ;
			}
			SGLImageBuffer *	pNewImage =
				sglCreateReferenceImageBuffer( pImageBuf, &rectCast ) ;
			pImageBuf = pNewImage ;
		}
	}
	return	pImageBuf ;
}

// NormalizeFormat で結合されたアニメーション画像への参照を生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSmartImage::NewAnimationReference
	( SGLImageRect* pFrameRects, size_t nRectsCount )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	if ( nRectsCount != 1 )
	{
		return	NULL ;
	}
	SGLImageRect	rect( 0, 0, pImage->width, pImage->height ) ;
	while ( pImage->ptrRefOriginal != NULL )
	{
		rect.x += pImage->rctRefOriginal.x ;
		rect.y += pImage->rctRefOriginal.y ;
		pImage = pImage->ptrRefOriginal ;
	}
	pFrameRects[0] = rect ;
	//
	sglAddReferenceImageBuffer( pImage ) ;
	return	new SGLSmartImage( pImage ) ;
}

// アトラス化された画像の参照矩形を取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::GetReferenceRectOfAtlas
			( SGLImageRect& rect, ssize_t iFrame ) const
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		rect.Clear() ;
		return	sglErrFailed ;
	}
	rect.x = 0 ;
	rect.y = 0 ;
	rect.w = (int32_t) pImage->width ;
	rect.h = (int32_t) pImage->height ;
	//
	while ( pImage->ptrRefOriginal != NULL )
	{
		rect.x += pImage->rctRefOriginal.x ;
		rect.y += pImage->rctRefOriginal.y ;
		pImage = pImage->ptrRefOriginal ;
	}
	return	sglErrSuccess ;
}

// 参照元画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSmartImage::GetImageReference
				( SGLImageRect& rectRef, ssize_t iFrame )
{
	return	GetImageReferenceOf( rectRef, m_pImage ) ;
}

SGLImageObject * SGLSmartImage::GetImageReferenceOf
			( SGLImageRect& rectRef, SGLImageBuffer * pImage )
{
	rectRef.x = 0 ;
	rectRef.y = 0 ;
	if ( pImage == NULL )
	{
		rectRef.w = 0 ;
		rectRef.h = 0 ;
		return	NULL ;
	}
	if ( pImage->ptrRefOriginal == NULL )
	{
		rectRef.w = pImage->width ;
		rectRef.h = pImage->height ;
		return	this ;
	}
	do
	{
		rectRef.x += pImage->rctRefOriginal.x ;
		rectRef.y += pImage->rctRefOriginal.y ;
		rectRef.w = pImage->rctRefOriginal.w ;
		rectRef.h = pImage->rctRefOriginal.h ;
		pImage = pImage->ptrRefOriginal ;
		ESLAssert( pImage != NULL ) ;
	}
	while ( pImage->ptrRefOriginal != NULL ) ;
	//
	return	CommitImageReferenceOf( pImage ) ;
}

SGLImageObject *
	SGLSmartImage::CommitImageReferenceOf( SGLImageBuffer * pImage )
{
	SGLImageBuffer *	pImageRef ;
	SGLImageRect		rectRefTemp ;
	SGLImageObjectBufferInterface *	pObjBuf =
		ESLTypeCast<SGLImageObjectBufferInterface>
			( pImage->GetImageObject
				( imageObjectEntisGLS4Temporary, pImageRef, rectRefTemp, true ) ) ;
	//
	if ( pObjBuf == NULL )
	{
		SGLSmartImage *	pImageObj = new SGLSmartImage ;
		pImageObj->AttachImageBuffer( pImage ) ;
		//
		pObjBuf = new SGLImageObjectBufferInterface ;
		pObjBuf->SetSmartImageReference( pImageObj ) ;
		pImage->AddImageObject( pObjBuf ) ;
		//
		return	pImageObj ;
	}
	else
	{
		ESLAssert( pImageRef == pImage ) ;
		SGLImageObject *	pImageObj = pObjBuf->GetImageReference() ;
		if ( pImageObj == NULL )
		{
			SGLSmartImage *	pImageObjTemp = new SGLSmartImage ;
			pImageObjTemp->AttachImageBuffer( pImage ) ;
			pObjBuf->SetSmartImageReference( pImageObjTemp ) ;
			pImageObj = pImageObjTemp ;
		}
		return	pImageObj ;
	}
}

// 参照先画像変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::MakeImageReference
	( size_t iFrame,
		SGLImageObject * pAtlasImage, const SGLImageRect& rectRef )
{
	if ( (iFrame != 0) || (pAtlasImage == nullptr) )
	{
		return	sglErrInvalidParam ;
	}
	if ( m_pImage == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer *	pAtlasBuf = pAtlasImage->GetImageBuffer() ;
	if ( pAtlasBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( pAtlasBuf == m_pImage )
	{
		return	sglErrFailed ;
	}
	return	sglMakeReferenceImageBuffer( m_pImage, pAtlasBuf, &rectRef ) ;
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBufferInterface *
	SGLSmartImage::CommitImageObject
		( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SGLImageBuffer *	pImageRef ;
	SGLImageBufferInterface *
		pObject = pImage->GetImageObject
					( idType, pImageRef, rectRef, fNoRefImage ) ;
	if ( pObject == NULL )
	{
		return	NULL ;
	}
	pObject->CommitBuffer( pImageRef ) ;
	return	pObject ;
}

// 画像オブジェクト追加登録
//////////////////////////////////////////////////////////////////////////////
bool SGLSmartImage::AddImageObject
	( SGLImageBufferInterface * pObject, bool fOriginalImage )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	false ;
	}
	QuickLock() ;
	if ( fOriginalImage )
	{
		while ( pImage->ptrRefOriginal != NULL )
		{
			pImage = pImage->ptrRefOriginal ;
		}
	}
	pImage->AddImageObject( pObject ) ;
	QuickUnlock() ;
	return	true ;
}

// 画像オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
bool SGLSmartImage::DetachImageObject( SGLImageBufferInterface * pObject )
{
	SGLImageBuffer *	pImage = m_pImage ;
	QuickLock() ;
	while ( pImage != NULL )
	{
		if ( pImage->DetachImageObject( pObject ) )
		{
			QuickUnlock() ;
			return	true ;
		}
		pImage = pImage->ptrRefOriginal ;
	}
	QuickUnlock() ;
	return	false ;
}

// 画像オブジェクト更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::UpdateImageObject( const SGLImageRect * pUpdateRect )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	pImage->UpdateImageObject( pUpdateRect ) ;
}

// 画像オブジェクト更新確定
//////////////////////////////////////////////////////////////////////////////
void SGLSmartImage::FlushImageObject( void )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return ;
	}
	pImage->CommitImageObject() ;
}

// 画像オブジェクト更新反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::ReflectImageObject( uint32_t idType )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer *	pImageRef ;
	SGLImageRect		rectRef ;
	SGLImageBufferInterface *
		pObject = pImage->GetImageObject
					( idType, pImageRef, rectRef, false ) ;
	if ( pObject == NULL )
	{
		return	sglErrFailed ;
	}
	pImage->UpdateImageObject( NULL ) ;
	pObject->ReflectBuffer( pImageRef ) ;
	return	sglErrSuccess ;
}

// 画像オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::DeleteImageObjectTypeOf( uint32_t idType )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	pImage->DeleteImageObject( idType ) ;
	return	sglErrSuccess ;
}

SGLError SGLSmartImage::DeleteAllImageObjects( void )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	pImage->DeleteAllImageObject() ;
	return	sglErrSuccess ;
}

// 画像オブジェクトクリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSmartImage::NotifyClearImageObject( SGLPalette pxClear )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	pImage->NotifyClearImageObject( pxClear ) ;
}

// 内部バッファ取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SGLSmartImage::GetImageBuffer( void ) const
{
	return	m_pImage ;
}

#if	defined(__COTOPHA__)
// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
Image * SGLSmartImage::GetImageObject( void ) const
{
	SGLImageBuffer *	pImageBuf = m_pImage ;
	if ( pImageBuf == NULL )
	{
		return	NULL ;
	}
	SGLImageBuffer *	pImageRef ;
	SGLImageRect		rectRef ;
	SGLImageBufferInterface *
		pObject = pImageBuf->GetImageObject
			( imageObjectEntisGLS4Image, pImageRef, rectRef, true ) ;
	if ( pObject != NULL )
	{
		EntisGLS4ImageBufferInterface *	pBufInterface =
			ESLTypeCast<EntisGLS4ImageBufferInterface>( pObject ) ;
		if ( pBufInterface != NULL )
		{
			pBufInterface->CommitBuffer( pImageRef ) ;
			return	pBufInterface->m_pImage ;
		}
		return	NULL ;
	}
	EntisGLS4ImageBufferInterface *
			pBufInterface = new EntisGLS4ImageBufferInterface ;
	pBufInterface->Initialize( pImageBuf ) ;
	pImageBuf->AddImageObject( pBufInterface ) ;
	return	pBufInterface->m_pImage ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// 複数画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLMultiImage, SGLSmartImage )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMultiImage::SGLMultiImage( void )
{
	m_msecLong = 0 ;
	m_iSelFrame = -1 ;
	m_iSelFrameSide = stereoImageBoth ;
}

SGLMultiImage::SGLMultiImage
	( const SGLMultiImage& img, const SGLImageRect * pClip, int iSide )
{
	CreateReferenceFrom( img, pClip, iSide ) ;
	m_iSelFrame = -1 ;
	m_iSelFrameSide = stereoImageBoth ;
	SelectFrame( img.GetSelectedFrame(), iSide ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMultiImage::~SGLMultiImage( void )
{
	const size_t			nCount = m_paImages.GetLength() ;
	SGLImageBuffer*const*	ppImages = m_paImages.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = ppImages[i] ;
		if ( pImage != NULL )
		{
			pImage->NotifyObjectDestroy( this ) ;
			sglReleaseImageBuffer( pImage ) ;
		}
	}
	m_paImages.RemoveAll() ;
}

// 参照複製
//////////////////////////////////////////////////////////////////////////////
void SGLMultiImage::CreateReferenceFrom
	( const SGLMultiImage& img,
		const SGLImageRect * pClip, int iSide )
{
	m_tblSequence = img.m_tblSequence ;
	m_msecLong = img.m_msecLong ;
	//
	const size_t	nCount = img.m_paImages.GetLength() ;
	m_paImages.SetLimit( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = img.m_paImages.GetAt( i ) ;
		if ( (pClip == NULL)
			&& ((iSide == stereoImageBoth)
				|| !(pImage->format & formatImageFlagSideBySide)) )
		{
			sglAddReferenceImageBuffer( pImage ) ;
		}
		else
		{
			pImage = sglCreateReferenceImageBuffer
						( pImage, pClip, 0, iSide ) ;
		}
		m_paImages.SetAt( i, pImage ) ;
	}
	SelectFrame( 0 ) ;
}

// フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMultiImage::GetFrameCount( void ) const
{
	return	m_paImages.GetLength() ;
}

// アニメーション・シーケンス全長取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMultiImage::GetSequenceLength( void ) const
{
	return	m_tblSequence.GetLength() ;
}

// アニメーション・シーケンス取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMultiImage::GetSequenceTable( uint32_t * pSeq, size_t nCount ) const
{
	if ( nCount > m_tblSequence.GetLength() )
	{
		nCount = m_tblSequence.GetLength() ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pSeq[i] = m_tblSequence.At(i) ;
	}
	return	nCount ;
}

// アニメーション全長（時間）取得 [msec]
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMultiImage::GetTotalTime( void ) const
{
	return	m_msecLong ;
}

// 時間からフレーム番号へ変換
//////////////////////////////////////////////////////////////////////////////
size_t SGLMultiImage::FrameFromMilliSec( uint64_t msec )
{
	if ( m_msecLong == 0 )
	{
		return	0 ;
	}
	if ( msec >= m_msecLong )
	{
		msec = m_msecLong - 1 ;
	}
	if ( m_tblSequence.GetLength() > 0 )
	{
		return	m_tblSequence.At
			( (size_t) (msec * m_tblSequence.GetLength() / m_msecLong) ) ;
	}
	else
	{
		return	(size_t) (msec * m_paImages.GetLength() / m_msecLong) ;
	}
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::SelectFrame( size_t iFrame, int iSide )
{
	SGLImageBuffer *	pImage = m_paImages.GetAt( iFrame ) ;
	if ( (pImage != NULL)
		&& (pImage->format & formatImageFlagSideBySide) && (iSide != 0) )
	{
		pImage = sglCreateReferenceImageBuffer( pImage, NULL, 0, iSide ) ;
		if ( pImage == NULL )
		{
			return	sglErrFailed ;
		}
	}
	else
	{
		sglAddReferenceImageBuffer( pImage ) ;
	}
	SetImageBuffer( pImage ) ;
	m_iSelFrame = iFrame ;
	m_iSelFrameSide = iSide ;
	return	sglErrSuccess ;
}

// 選択中フレーム取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMultiImage::GetSelectedFrame( int * pSide ) const
{
	if ( pSide != NULL )
	{
		*pSide = m_iSelFrameSide ;
	}
	return	m_iSelFrame ;
}

// 画像バッファ読み出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::ReadFrameBuffer
	( const SGLImageInfo & imginf, uint8_t * ptrBuffer, size_t iFrame, int iSide )
{
	SGLImageBuffer *	pImage = m_paImages.GetAt( iFrame ) ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError		err ;
	SGLImageBuffer	imgDstBuf = imginf ;
	imgDstBuf.ptrBuffer = ptrBuffer ;
	//
	if ( pImage->format & formatImageFlagSideBySide )
	{
		SGLImageBuffer *	pRefImage =
			sglCreateReferenceImageBuffer( pImage, NULL, 0, iSide ) ;
		if ( pRefImage == NULL )
		{
			return	sglErrFailed ;
		}
		err = sglCopyImageBuffer( imgDstBuf, *pRefImage ) ;
		if ( err )
		{
			err = sglConvertImageBuffer( imgDstBuf, *pRefImage ) ;
		}
		sglReleaseImageBuffer( pRefImage ) ;
	}
	else
	{
		err = sglCopyImageBuffer( imgDstBuf, *pImage ) ;
		if ( err )
		{
			err = sglConvertImageBuffer( imgDstBuf, *pImage ) ;
		}
	}
	return	err ;
}

// 画像バッファへの参照生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLMultiImage::NewReference
	( const SGLImageRect * pClip, ssize_t iFrame, int iSide )
{
	if ( iFrame < 0 )
	{
		return	new SGLMultiImage( *this, pClip, iSide ) ;
	}
	else
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( iFrame ) ;
		if ( pImage == NULL )
		{
			return	NULL ;
		}
		return	new SGLSmartImage
					( sglCreateReferenceImageBuffer
								( pImage, pClip, 0, iSide ) ) ;
	}
}

// テクスチャのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::NormalizeToTexture( uint32_t nFlags )
{
	uint64_t	nBufFlags = ConvertFlagsToBufferFlags(nFlags)
							| flagImageNormalDepth ;
	if ( nFlags & bufferNonPowerOf2 )
	{
		nBufFlags |= flagImageNormalPitch ;
	}
	else
	{
		nBufFlags |= flagImageNormalSize ;
	}
	bool	flagCubeMap = false ;
	bool	flag3Dtexture = false ;
	bool	flagArrayTexture = false ;
	size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( pImage != NULL )
		{
			if ( pImage->ptrRefOriginal == NULL )
			{
				sglNormalizeImageBuffer( pImage, nBufFlags ) ;
			}
			if ( pImage->flagsBuffer & flagImageCubemap )
			{
				flagCubeMap = true ;
			}
			if ( pImage->flagsBuffer & flagImageTexture3D )
			{
				flag3Dtexture = true ;
			}
			if ( pImage->flagsBuffer & flagImageTextureArray )
			{
				flagArrayTexture = true ;
			}
		}
	}
	if ( !flagCubeMap && (nFlags & bufferCubeMapTexture) )
	{
		if ( m_pImage != NULL )
		{
			size_t countFrame = nCount ;
			if ( countFrame < 6 )
			{
				countFrame = 6 ;
			}
			SGLImageInfo	imginf = *m_pImage ;
			SGLImageInfo	imginfCube = imginf ;
			//
			if ( m_pImage->ptrBuffer == NULL )
			{
				nBufFlags |= flagImageNoBuffer
							| flagImageNoReadBuffer | flagImageNoWriteBuffer ;
			}
			else
			{
				imginfCube.height *= (uint32_t) countFrame ;
			}
			SGLImageBuffer *	pCubeMap =
				sglCreateImageBuffer( imginfCube, nBufFlags ) ;
			SetImageBufferIdentity( pCubeMap, m_pImage->pwszIdentity ) ;
			//
			m_paImages.SetLength( countFrame ) ;
			for ( size_t i = 0; i < countFrame; i ++ )
			{
				SGLImageRect	rectFace
					( 0, (int) i * imginf.height, imginf.width, imginf.height ) ;
				if ( m_pImage->ptrBuffer == NULL )
				{
					rectFace.y = 0 ;
				}
				SGLImageBuffer *	pCubeFace =
					sglCreateReferenceImageBuffer( pCubeMap, &rectFace ) ;
				if ( i < 6 )
				{
					pCubeFace->flagsBuffer |= (i << flagImageCubeIndexShifter) ;
				}
				SGLImageBuffer *	pOldFrame = m_paImages.GetAt( i ) ;
				if ( pOldFrame != NULL )
				{
					if ( pCubeFace->ptrBuffer && pOldFrame->ptrBuffer )
					{
						sglCopyImageBuffer( *pCubeFace, *pOldFrame ) ;
					}
					sglReleaseImageBuffer( pOldFrame ) ;
				}
				m_paImages.SetAt( i, pCubeFace ) ;
			}
			sglReleaseImageBuffer( pCubeMap ) ;
		}
		SelectFrame( m_iSelFrame ) ;
	}
	else if ( (!flag3Dtexture && (nFlags & bufferTexture3D))
			|| (!flagArrayTexture && (nFlags & bufferTextureArray)) )
	{
		if ( m_pImage != NULL )
		{
			SGLImageInfo	imginf = *m_pImage ;
			SGLImageInfo	imginf3d = imginf ;
			//
			if ( m_pImage->ptrBuffer == NULL )
			{
				nBufFlags |= flagImageNoBuffer
							| flagImageNoReadBuffer | flagImageNoWriteBuffer ;
			}
			else
			{
				imginf3d.height *= (uint32_t) nCount ;
			}
			SGLImageBuffer *	pImage3D =
				sglCreateImageBuffer( imginf3d, nBufFlags ) ;
			SetImageBufferIdentity( pImage3D, m_pImage->pwszIdentity ) ;
			//
			pImage3D->nFrameCount = nCount ;
			pImage3D->sizeFrame.w = (int32_t) imginf.width ;
			pImage3D->sizeFrame.h = (int32_t) imginf.height ;
			//
			for ( size_t i = 0; i < nCount; i ++ )
			{
				SGLImageRect	rectLayer
					( 0, (int) i * imginf.height, imginf.width, imginf.height ) ;
				if ( m_pImage->ptrBuffer == NULL )
				{
					rectLayer.y = 0 ;
				}
				SGLImageBuffer *	pLayer2d =
					sglCreateReferenceImageBuffer( pImage3D, &rectLayer ) ;
				pLayer2d->flagsBuffer |= (((uint64_t)i) << flagImageLayerIndexShifter) ;
				//
				SGLImageBuffer *	pOldFrame = m_paImages.GetAt( i ) ;
				if ( pOldFrame != NULL )
				{
					if ( pLayer2d->ptrBuffer && pOldFrame->ptrBuffer )
					{
						sglCopyImageBuffer( *pLayer2d, *pOldFrame ) ;
					}
					sglReleaseImageBuffer( pOldFrame ) ;
				}
				m_paImages.SetAt( i, pLayer2d ) ;
			}
			sglReleaseImageBuffer( pImage3D ) ;
		}
		SelectFrame( m_iSelFrame ) ;
	}
	return	sglErrSuccess ;
}

SGLError SGLMultiImage::NormalizeToMipmapTexture( uint32_t nFlags )
{
	uint64_t	nBufFlags = ConvertFlagsToBufferFlags(nFlags)
							| flagImageNormalDepth | flagImageMipmap ;
	if ( nFlags & bufferNonPowerOf2 )
	{
		nBufFlags |= flagImageNormalPitch ;
	}
	else
	{
		nBufFlags |= flagImageNormalSize ;
	}
	const size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( pImage != NULL )
		{
			if ( pImage->ptrRefOriginal == NULL )
			{
				sglNormalizeImageBuffer( pImage, nBufFlags ) ;
				pImage->MakeMipmap() ;
			}
		}
	}
	return	sglErrSuccess ;
}

// レンダリング・ターゲットのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::NormalizeToRenderTarget( uint32_t nFlags )
{
	const size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( pImage != NULL )
		{
			sglNormalizeImageBuffer
				( pImage, (flagImageNormalDepth | flagImageNormalSize) ) ;
		}
	}
	return	sglErrSuccess ;
}

// テクスチャのための正規化フラグを除去する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::DenormalizeForTexture( uint32_t nFlags )
{
	uint32_t	nDenormFlags =
					ConvertFlagsToBufferFlags( nFlags ) 
						& (flagImageMipmap
							| flagImageCubemap | flagImageTexture3D
							| flagImageTextureArray
							| flagImageTextureMultisample
							| flagImageRenderBufferStorage
							| flagImageCompressedTexture
							| flagImageNeedSampleNoSmooth
							| flagImageNeedSampleTiling) ;
	const size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( (pImage != NULL) && (pImage->flagsBuffer & nDenormFlags) )
		{
			SGLImageBuffer *	pOrg = pImage ;
			QuickLock() ;
			while ( pOrg->ptrRefOriginal != NULL )
			{
				pOrg = pOrg->ptrRefOriginal ;
			}
			QuickUnlock() ;
			//
			pImage->flagsBuffer &= ~ (uint64_t) nDenormFlags ;
			pImage->DeleteAllImageObject() ;
			//
			if ( pImage != pOrg )
			{
				pOrg->flagsBuffer &= ~ (uint64_t) nDenormFlags ;
				pOrg->DeleteAllImageObject() ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::CreateBuffer
	( const SGLImageInfo& imginf,
		uint64_t nFlags, size_t countFrame, uint64_t msecLong )
{
	ReleaseBuffer() ;
	//
	int	sglFlags = 0 ;
	if ( nFlags & (bufferForTexture | bufferForRenderTarget) )
	{
		if ( nFlags & (bufferNonPowerOf2 | bufferForRenderTarget) )
		{
			sglFlags |= flagImageNormalDepth | flagImageNormalPitch ;
		}
		else
		{
			sglFlags |= flagImageNormalDepth | flagImageNormalSize ;
		}
	}
	if ( nFlags & bufferOnDeviceOnly )
	{
		sglFlags |= flagImageNoBuffer
					| flagImageNoReadBuffer | flagImageNoWriteBuffer ;
	}
	sglFlags |= ConvertFlagsToBufferFlags( nFlags ) ;
	//
	if ( nFlags & bufferCubeMapTexture )
	{
		//
		// Cubmap 用画像バッファ
		//
		sglFlags |= flagImageCubemap ;
		//
		if ( countFrame < 6 )
		{
			countFrame = 6 ;
		}
		SGLImageInfo	imginfCube = imginf ;
		if ( !(nFlags & bufferOnDeviceOnly) )
		{
			imginfCube.height *= (uint32_t) countFrame ;
		}
		//
		SGLImageBuffer *	pCubeMap =
			sglCreateImageBuffer( imginfCube, sglFlags ) ;
		pCubeMap->nFrameCount = countFrame ;
		pCubeMap->sizeFrame.w = (int32_t) imginf.width ;
		pCubeMap->sizeFrame.h = (int32_t) imginf.height ;
		//
		m_paImages.SetLength( countFrame ) ;
		for ( size_t i = 0; i < countFrame; i ++ )
		{
			SGLImageRect	rectFace
				( 0, (int) i * imginf.height, imginf.width, imginf.height ) ;
			if ( nFlags & bufferOnDeviceOnly )
			{
				rectFace.y = 0 ;
			}
			SGLImageBuffer *	pCubeFace =
				sglCreateReferenceImageBuffer( pCubeMap, &rectFace ) ;
			if ( i < 6 )
			{
				pCubeFace->flagsBuffer |= (i << flagImageCubeIndexShifter) ;
			}
			m_paImages.SetAt( i, pCubeFace ) ;
		}
		sglReleaseImageBuffer( pCubeMap ) ;
	}
	else if ( nFlags & (bufferTexture3D | bufferTextureArray) )
	{
		//
		// 2D 配列 / 3D テクスチャ用画像バッファ
		//
		if ( nFlags & bufferTexture3D )
		{
			sglFlags |= flagImageTexture3D ;
		}
		else
		{
			sglFlags |= flagImageTextureArray ;
		}
		//
		SGLImageInfo	imginf3D = imginf ;
		if ( !(nFlags & bufferOnDeviceOnly) )
		{
			imginf3D.height *= (uint32_t) countFrame ;
		}
		SGLImageBuffer *	pImage3D =
			sglCreateImageBuffer( imginf3D, sglFlags ) ;
		pImage3D->nFrameCount = countFrame ;
		pImage3D->sizeFrame.w = (int32_t) imginf.width ;
		pImage3D->sizeFrame.h = (int32_t) imginf.height ;
		//
		m_paImages.SetLength( countFrame ) ;
		for ( size_t i = 0; i < countFrame; i ++ )
		{
			SGLImageRect	rect2D
				( 0, (int) i * imginf.height, imginf.width, imginf.height ) ;
			if ( nFlags & bufferOnDeviceOnly )
			{
				rect2D.y = 0 ;
			}
			SGLImageBuffer *	pImage2D =
				sglCreateReferenceImageBuffer( pImage3D, &rect2D ) ;
			pImage2D->flagsBuffer |= (((uint64_t) i) << flagImageLayerIndexShifter) ;
			m_paImages.SetAt( i, pImage2D ) ;
		}
		sglReleaseImageBuffer( pImage3D ) ;
	}
	else if ( nFlags & bufferMultisample )
	{
		SGLImageBuffer *	pImageMulti =
			sglCreateImageBuffer( imginf, sglFlags ) ;
		pImageMulti->nFrameCount = countFrame ;
		pImageMulti->sizeFrame.w = (int32_t) imginf.width ;
		pImageMulti->sizeFrame.h = (int32_t) imginf.height ;
		//
		m_paImages.SetLength( 1 ) ;
		m_paImages.SetAt( 0, pImageMulti ) ;
	}
	else
	{
		//
		// 通常画像バッファ
		//
		m_paImages.SetLength( countFrame ) ;
		for ( size_t i = 0; i < countFrame; i ++ )
		{
			m_paImages.SetAt
				( i, sglCreateImageBuffer( imginf, sglFlags ) ) ;
		}
	}
	m_msecLong = msecLong ;
	return	SelectFrame( 0 ) ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void SGLMultiImage::ReleaseBuffer( void )
{
	SGLSmartImage::ReleaseBuffer() ;
	//
	const size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( pImage != NULL )
		{
			sglReleaseImageBuffer( pImage ) ;
		}
	}
	m_paImages.RemoveAll() ;
	m_tblSequence.RemoveAll() ;
	m_msecLong = 0 ;
}

// アニメーション・シーケンス・テーブルの設定
//////////////////////////////////////////////////////////////////////////////
void SGLMultiImage::SetSequenceTable( const uint32_t * pSeq, size_t nCount )
{
	m_tblSequence.SetLength( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_tblSequence.SetAt( i, pSeq[i] ) ;
	}
}

// アニメーション全長時間設定
//////////////////////////////////////////////////////////////////////////////
void SGLMultiImage::SetAnimationDuration( uint64_t nDuration )
{
	m_msecLong = nDuration ;
}

// 画像フォーマット正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::NormalizeFormat
	( uint32_t format, uint32_t depth,
		uint32_t nFlags, uint32_t width, uint32_t height )
{
	if ( nFlags & formatMergeAnimation )
	do
	{
		const size_t	nCount = m_paImages.GetLength() ;
		ImageRefArray	ira ;
		ira.SetLimit( nCount ) ;
		//
		SGLImageBuffer *	pImage0 = m_paImages.GetAt( 0 ) ;
		if ( (pImage0 == NULL)
			|| (pImage0->width == 0) || (pImage0->height == 0) )
		{
			break ;
		}
		const int	nBufFlags = (int) pImage0->flagsBuffer ;
		if ( (width == 0) && (height == 0) )
		{
			double	w =
				sqrt( (double) (pImage0->width + 2)
								* (pImage0->height + 2) * nCount ) ;
			width = 1 << (uint32_t) eslRoundR64ToLInt( log(w) / log(2.0) + 0.3 ) ;
			//
			size_t	wUnits = width / (pImage0->width + 2) ;
			if ( wUnits == 0 )
			{
				wUnits = 1 ;
			}
			size_t	hUnits = (nCount + wUnits - 1) / wUnits ;
			//
			width = sglNormalizeScalePowerBy2
						( (uint32_t) (wUnits * (pImage0->width + 2)) ) ;
			height = sglNormalizeScalePowerBy2
						( (uint32_t) (hUnits * (pImage0->height + 2)) ) ;
		}
		if ( format == 0 )
		{
			format = pImage0->format ;
		}
		if ( depth == 0 )
		{
			depth = pImage0->depth ;
		}
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
			if ( pImage == NULL )
			{
				continue ;
			}
			if ( (pImage->width >= width - 8)
				|| (pImage->height >= height - 8) )
			{
				SGLImageBuffer *	pNewImage =
					NormalizeBufferFormat( pImage, format, depth, 0, 0, 0 ) ;
				if ( pImage != pNewImage )
				{
					sglReleaseImageBuffer( pImage ) ;
					m_paImages.SetAt( i, pNewImage ) ;
				}
				continue ;
			}
			if ( ira.m_rectCurrent.x
						+ ira.m_rectCurrent.w + pImage->width + 2 > width )
			{
				if ( ira.m_sizeMerge.w
						< ira.m_rectCurrent.x + ira.m_rectCurrent.w )
				{
					ira.m_sizeMerge.w =
							ira.m_rectCurrent.x + ira.m_rectCurrent.w ;
				}
				ira.m_sizeMerge.h = ira.m_rectCurrent.y + ira.m_rectCurrent.h ;
				ira.m_rectCurrent.Clear() ;
				ira.m_rectCurrent.y = ira.m_sizeMerge.h ;
			}
			if ( ira.m_sizeMerge.h + pImage->height + 2 > height )
			{
				FlushMergeAnimationBy( ira, format, depth, nBufFlags ) ;
			}
			//
			ImageRefUnit	iru ;
			iru.rectRef.x = ira.m_rectCurrent.x + 1 ;
			iru.rectRef.y = ira.m_rectCurrent.y + 1 ;
			iru.rectRef.w = pImage->width ;
			iru.rectRef.h = pImage->height ;
			iru.iFrame = i ;
			ira.Add( iru ) ;
			//
			ira.m_rectCurrent.x = iru.rectRef.x + iru.rectRef.w ;
			ira.m_rectCurrent.y = ira.m_sizeMerge.h ;
			ira.m_rectCurrent.w = 0 ;
			ira.m_rectCurrent.h = pImage->height + 2 ;
		}
		FlushMergeAnimationBy( ira, format, depth, nBufFlags ) ;
		SelectFrame( m_iSelFrame, m_iSelFrameSide ) ;
		return	sglErrSuccess ;
	}
	while ( false ) ;
	//
	const size_t	nCount = m_paImages.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		SGLImageBuffer *	pNewImage =
			NormalizeBufferFormat
				( pImage, format, depth, nFlags, width, height ) ;
		if ( pImage != pNewImage )
		{
			sglReleaseImageBuffer( pImage ) ;
			m_paImages.SetAt( i, pNewImage ) ;
		}
	}
	SelectFrame( m_iSelFrame, m_iSelFrameSide ) ;
	return	sglErrSuccess ;
}

void SGLMultiImage::FlushMergeAnimationBy
	( SGLMultiImage::ImageRefArray& ira,
		uint32_t format, uint32_t depth, int nBufFlags )
{
	if ( ira.GetLength() == 0 )
	{
		return ;
	}
	SGLSize	sizeMerge = ira.m_sizeMerge ;
	if ( ira.m_rectCurrent.x + ira.m_rectCurrent.w > sizeMerge.w )
	{
		sizeMerge.w = ira.m_rectCurrent.x + ira.m_rectCurrent.w ;
	}
	if ( ira.m_rectCurrent.y + ira.m_rectCurrent.h > sizeMerge.h )
	{
		sizeMerge.h = ira.m_rectCurrent.y + ira.m_rectCurrent.h ;
	}
	SGLImageInfo	imginf ;
	imginf.format = format ;
	imginf.depth = depth ;
	imginf.width = sglNormalizeScalePowerBy2( (uint32_t) sizeMerge.w ) ;
	imginf.height = sglNormalizeScalePowerBy2( (uint32_t) sizeMerge.h ) ;
	//
	SGLImageBuffer *
		pNewImage = sglCreateImageBuffer( imginf, nBufFlags ) ;
	//
	size_t	nCount = ira.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ImageRefUnit *	piru = ira.GetAt( i ) ;
		ESLAssert( piru != NULL ) ;
		SGLImageBuffer *	pImage = m_paImages.GetAt( piru->iFrame ) ;
		ESLAssert( pImage != NULL ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		ESLAssert( piru->rectRef.x + piru->rectRef.w <= (int) imginf.width ) ;
		ESLAssert( piru->rectRef.y + piru->rectRef.h <= (int) imginf.height ) ;
		sglConvertImageBuffer
			( *pNewImage, *pImage, piru->rectRef.x, piru->rectRef.y ) ;
		if ( (pNewImage->pwszIdentity == nullptr)
			&& (pImage->pwszIdentity != nullptr) )
		{
			SetImageBufferIdentity( pNewImage, pImage->pwszIdentity ) ;
		}
		if ( !(pImage->format & formatImageFlagAlpha) )
		{
			SGLPoint		ptRef( piru->rectRef.x, piru->rectRef.y ) ;
			SGLSize			sizeRef( piru->rectRef.w, piru->rectRef.h ) ;
			//
			SGLImageRect	rectUL( 0, 0, 1, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x - 1, ptRef.y - 1, &rectUL ) ;
			//
			SGLImageRect	rectUP( 0, 0, sizeRef.w, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x, ptRef.y - 1, &rectUP ) ;
			//
			SGLImageRect	rectUR( sizeRef.w - 1, 0, 1, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x + sizeRef.w, ptRef.y - 1, &rectUR ) ;
			//
			SGLImageRect	rectL( 0, 0, 1, sizeRef.h ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x + 1, ptRef.y, &rectL ) ;
			//
			SGLImageRect	rectR( sizeRef.w - 1, 0, 1, sizeRef.h ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x + sizeRef.w, ptRef.y, &rectR ) ;
			//
			SGLImageRect	rectDL( 0, sizeRef.h - 1, 1, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x - 1, ptRef.y + sizeRef.h, &rectDL ) ;
			//
			SGLImageRect	rectDN( 0, sizeRef.h - 1, sizeRef.w, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x, ptRef.y + sizeRef.h, &rectDN ) ;
			//
			SGLImageRect	rectDR( sizeRef.w - 1, sizeRef.h - 1, 1, 1 ) ;
			sglConvertImageBuffer
				( *pNewImage, *pImage,
					ptRef.x + sizeRef.w, ptRef.y + sizeRef.h, &rectDR ) ;
		}
		SGLImageBuffer *	pRefImage =
			sglCreateReferenceImageBuffer
				( pNewImage, &(piru->rectRef), 0, stereoImageBoth ) ;
		sglReleaseImageBuffer( pImage ) ;
		m_paImages.SetAt( piru->iFrame, pRefImage ) ;
	}
	//
	ira.m_sizeMerge.w = 0 ;
	ira.m_sizeMerge.h = 0 ;
	ira.m_rectCurrent.x = 0 ;
	ira.m_rectCurrent.y = 0 ;
	ira.m_rectCurrent.w = 0 ;
	ira.m_rectCurrent.h = 0 ;
	//
	ira.RemoveAll() ;
	sglReleaseImageBuffer( pNewImage ) ;
}

// NormalizeFormat で結合されたアニメーション画像への参照を生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLMultiImage::NewAnimationReference
	( SGLImageRect* pFrameRects, size_t nRectsCount )
{
	if ( nRectsCount > m_paImages.GetLength() )
	{
		return	NULL ;
	}
	SGLImageBuffer *	pMergedImage = NULL ;
	for ( size_t i = 0; i < nRectsCount; i ++ )
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( i ) ;
		if ( pImage == NULL )
		{
			return	NULL ;
		}
		SGLImageRect	rect( 0, 0, pImage->width, pImage->height ) ;
		while ( pImage->ptrRefOriginal != NULL )
		{
			rect.x += pImage->rctRefOriginal.x ;
			rect.y += pImage->rctRefOriginal.y ;
			pImage = pImage->ptrRefOriginal ;
		}
		if ( pMergedImage == NULL )
		{
			pMergedImage = pImage ;
		}
		else if ( pMergedImage != pImage )
		{
			return	NULL ;
		}
		pFrameRects[i] = rect ;
	}
	sglAddReferenceImageBuffer( pMergedImage ) ;
	return	new SGLSmartImage( pMergedImage ) ;
}

// アトラス化された画像の参照矩形を取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::GetReferenceRectOfAtlas
	( SGLImageRect& rect, ssize_t iFrame ) const
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( iFrame >= 0 )
	{
		pImage = m_paImages.GetAt( iFrame ) ;
	}
	if ( pImage == NULL )
	{
		rect.Clear() ;
		return	sglErrFailed ;
	}
	rect.x = 0 ;
	rect.y = 0 ;
	rect.w = (int32_t) pImage->width ;
	rect.h = (int32_t) pImage->height ;
	//
	while ( pImage->ptrRefOriginal != NULL )
	{
		rect.x += pImage->rctRefOriginal.x ;
		rect.y += pImage->rctRefOriginal.y ;
		pImage = pImage->ptrRefOriginal ;
	}
	return	sglErrSuccess ;
}

// 参照元画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLMultiImage::GetImageReference
			( SGLImageRect& rectRef, ssize_t iFrame )
{
	SGLImageBuffer *	pImage = m_pImage ;
	if ( iFrame >= 0 )
	{
		pImage = m_paImages.GetAt( iFrame ) ;
	}
	SGLImageObject *	pImageObj = GetImageReferenceOf( rectRef, pImage ) ;
	if ( pImageObj == this )
	{
		if ( pImage != m_pImage )
		{
			return	CommitImageReferenceOf( pImage ) ;
		}
	}
	return	pImageObj ;
}

// 参照先画像変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMultiImage::MakeImageReference
		( size_t iFrame,
			SGLImageObject * pAtlasImage, const SGLImageRect& rectRef )
{
	if ( pAtlasImage == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer *	pImage = m_paImages.GetAt( iFrame ) ;
	if ( pImage == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer *	pAtlasBuf = pAtlasImage->GetImageBuffer() ;
	if ( pAtlasBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( pAtlasBuf == pImage )
	{
		return	sglErrFailed ;
	}
	return	sglMakeReferenceImageBuffer( pImage, pAtlasBuf, &rectRef ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト・スマート参照バッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageSmartBuffer::SGLImageSmartBuffer( void )
	: m_pImage( NULL ), m_flags( 0 )
{
}

SGLImageSmartBuffer::SGLImageSmartBuffer
	( SGLImageBuffer& imgbuf,
		SGLImageObject * pImage, int flags, const SGLImageRect * pRect )
	: m_pImage( NULL ), m_flags( 0 )
{
	Lock( imgbuf, pImage, flags, pRect ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageSmartBuffer::~SGLImageSmartBuffer( void )
{
	if ( m_pImage != NULL )
	{
		m_pImage->UnlockBuffer( m_flags ) ;
	}
}

// バッファ参照
//////////////////////////////////////////////////////////////////////////////
const SGLImageBuffer& SGLImageSmartBuffer::Lock
	( SGLImageBuffer& imgbuf,
		SGLImageObject * pImage, int flags, const SGLImageRect * pRect )
{
	m_pImage = pImage ;
	m_flags = flags ;
	imgbuf.ptrBuffer = pImage->LockBuffer( imgbuf, flags, pRect ) ;
	return	imgbuf ;
}

// バッファ参照解放
//////////////////////////////////////////////////////////////////////////////
void SGLImageSmartBuffer::Unlock( void )
{
	if ( m_pImage != NULL )
	{
		m_pImage->UnlockBuffer( m_flags ) ;
		m_pImage = NULL ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 二次元領域アロケーター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAreaAllocator, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAreaAllocator::SGLAreaAllocator( void )
	: m_sizeInit( 0, 0 ), m_sizeLimit( 0x7FFFFFFF, 0x7FFFFFFF )
{
	m_nFlags = 0 ;
	m_pRoot = NULL ;
	m_pLast = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAreaAllocator::~SGLAreaAllocator( void )
{
	FreeAll() ;
}

// 領域確保
//////////////////////////////////////////////////////////////////////////////
SGLImageRect * SGLAreaAllocator::Allocate( int w, int h )
{
	Local *	pLocal = m_pLast ;
	if ( pLocal == NULL )
	{
		//
		// はじめの領域
		//
		ESLAssert( m_pRoot == NULL ) ;
		int	wAlloc = esl_max( w, m_sizeInit.w ) ;
		int	hAlloc = esl_max( h, m_sizeInit.h ) ;
		if ( m_nFlags & flagSizePoweredBy2 )
		{
			wAlloc = sglNormalizeScalePowerBy2( w ) ;
			hAlloc = sglNormalizeScalePowerBy2( h ) ;
		}
		pLocal = CreateLocalArea( 0, 0, wAlloc, hAlloc, nullptr ) ;
		AllocateLocalArea( pLocal, w, h ) ;
		//
		m_pRoot = pLocal ;
		m_pLast = pLocal ;
		return	pLocal ;
	}
	//
	// 直前確保領域の隣接領域
	//
	ESLAssert( !m_pLast->IsEmpty() ) ;
	pLocal = AllocateSubLocal( m_pLast, w, h ) ;
	if ( pLocal != NULL )
	{
		m_pLast = pLocal ;
		return	pLocal ;
	}
	//
	// 空き領域検索
	//
	Local *	pLast = m_pLast ;
	Local *	pParent = pLast->m_pParent ;
	while ( pParent != NULL )
	{
		if ( pParent->m_pRight != pLast )
		{
			pLocal = AllocateSubLocal( pParent->m_pRight, w, h ) ;
			if ( pLocal != NULL )
			{
				m_pLast = pLocal ;
				return	pLocal ;
			}
		}
		if ( pParent->m_pUnder != pLast )
		{
			pLocal = AllocateSubLocal( pParent->m_pUnder, w, h ) ;
			if ( pLocal != NULL )
			{
				m_pLast = pLocal ;
				return	pLocal ;
			}
		}
		pLast = pParent ;
		pParent = pLast->m_pParent ;
	}
	//
	// 全体領域拡張
	//
	ESLAssert( m_pRoot != NULL ) ;
	if ( (m_pRoot->m_rectExt.w <= m_pRoot->m_rectExt.h)
			&& (m_pRoot->m_rectExt.w + w <= m_sizeLimit.w) )
	{
		//
		// 右側へ拡張
		//
		int	wAlloc = m_pRoot->m_rectExt.w + w ;
		int	hAlloc = esl_max( m_pRoot->m_rectExt.h, h ) ;
		if ( m_nFlags & flagSizePoweredBy2 )
		{
			wAlloc = sglNormalizeScalePowerBy2( wAlloc ) ;
			hAlloc = sglNormalizeScalePowerBy2( hAlloc ) ;
		}
		pLocal = CreateLocalArea( 0, 0, wAlloc, hAlloc, nullptr ) ;
		//
		pLocal->w = m_pRoot->m_rectExt.w ;
		pLocal->h = m_pRoot->m_rectExt.h ;
		pLocal->m_pChild = m_pRoot ;
		m_pRoot->m_pParent = pLocal ;
		m_pRoot = pLocal ;
		//
		pLocal->m_pRight =
			CreateLocalArea
				( pLocal->w, pLocal->y, wAlloc - pLocal->w, hAlloc, pLocal ) ;
		if ( hAlloc > pLocal->h )
		{
			pLocal->m_pUnder =
				CreateLocalArea
					( pLocal->x, pLocal->y + pLocal->h,
							pLocal->w, hAlloc - pLocal->h, pLocal ) ;
		}
		//
		AllocateLocalArea( pLocal->m_pRight, w, h ) ;
		m_pLast = pLocal->m_pRight ;
		return	m_pLast ;
	}
	else if ( m_pRoot->m_rectExt.h + h <= m_sizeLimit.h )
	{
		//
		// 下側へ拡張
		//
		int	wAlloc = esl_max( m_pRoot->m_rectExt.w, w ) ;
		int	hAlloc = m_pRoot->m_rectExt.h + h ;
		if ( m_nFlags & flagSizePoweredBy2 )
		{
			wAlloc = sglNormalizeScalePowerBy2( wAlloc ) ;
			hAlloc = sglNormalizeScalePowerBy2( hAlloc ) ;
		}
		pLocal = CreateLocalArea( 0, 0, wAlloc, hAlloc, nullptr ) ;
		//
		AllocateLocalArea
			( pLocal, m_pRoot->m_rectExt.w, m_pRoot->m_rectExt.h ) ;
		pLocal->m_pChild = m_pRoot ;
		m_pRoot->m_pParent = pLocal ;
		m_pRoot = pLocal ;
		//
		AllocateLocalArea( pLocal->m_pUnder, w, h ) ;
		m_pLast = pLocal->m_pUnder ;
		return	m_pLast ;
	}
	return	NULL ;
}

// 領域一括確保
//////////////////////////////////////////////////////////////////////////////
size_t SGLAreaAllocator::BatchAllocate
	( SGLImageRect** ppRects,
		const SGLSize * pSizes, size_t nCount,
		size_t * pAllocaedIndexes )
{
	//
	// ソート
	//
	SArray<size_t>	aIndex ;
	size_t *	pIndex = aIndex.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pIndex[i] = i ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSize	sizeMax = pSizes[pIndex[i]] ;
		size_t	iMax = i ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			size_t	k = pIndex[j] ;
			if ( sizeMax.h < pSizes[k].h )
			{
				sizeMax = pSizes[k] ;
				iMax = j ;
			}
			else if ( sizeMax.h == pSizes[k].h )
			{
				if ( sizeMax.w < pSizes[k].w )
				{
					sizeMax = pSizes[k] ;
					iMax = j ;
				}
			}
		}
		if ( iMax != i )
		{
			size_t	k = pIndex[iMax] ;
			pIndex[iMax] = pIndex[i] ;
			pIndex[i] = k ;
		}
	}
	aIndex.FinishArray() ;
	//
	// 順次確保
	//
	size_t	nAllocated = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		size_t			k = pIndex[i] ;
		SGLImageRect *	pRect = Allocate( pSizes[k].w, pSizes[k].h ) ;
		ppRects[k] = pRect ;
		if ( pRect != nullptr )
		{
			if ( pAllocaedIndexes != nullptr )
			{
				pAllocaedIndexes[nAllocated] = k ;
			}
			nAllocated ++ ;
		}
	}
	return	nAllocated ;
}

// 全領域解放
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::FreeAll( void )
{
	if ( m_pRoot != NULL )
	{
		FreeLocal( m_pRoot ) ;
		m_pRoot = NULL ;
		m_pLast = NULL ;
	}
}

// 全体サイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLAreaAllocator::GetTotalSize( void ) const
{
	if ( m_pRoot == NULL )
	{
		return	SGLSize( 0, 0 ) ;
	}
	return	m_pRoot->m_rectExt.GetSize() ;
}

// フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLAreaAllocator::GetFlags( void ) const
{
	return	m_nFlags ;
}

// フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::SetFlags( uint32_t nFlags )
{
	m_nFlags = nFlags ;
}

// 全体初期サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::SetInitialSize( int w, int h )
{
	m_sizeInit.w = w ;
	m_sizeInit.h = h ;
}

// 全体限界サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::SetLimitSize( int w, int h )
{
	m_sizeLimit.w = w ;
	m_sizeLimit.h = h ;
}

// 指定サイズローカル領域生成
//////////////////////////////////////////////////////////////////////////////
SGLAreaAllocator::Local *
	SGLAreaAllocator::CreateLocalArea
		( int x, int y, int w, int h, Local * pParent )
{
	Local *	pLocal = new Local ;
	pLocal->x = x ;
	pLocal->y = y ;
	pLocal->m_rectExt.x = x ;
	pLocal->m_rectExt.y = y ;
	pLocal->m_rectExt.w = w ;
	pLocal->m_rectExt.h = h ;
	pLocal->m_pParent = pParent ;
	pLocal->m_pChild = NULL ;
	pLocal->m_pUnder = NULL ;
	pLocal->m_pRight = NULL ;
	return	pLocal ;
}

// サブ領域確保
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::AllocateLocalArea
	( SGLAreaAllocator::Local * pLocal, int w, int h )
{
	ESLAssert( pLocal->IsEmpty() ) ;
	ESLAssert( (w <= pLocal->m_rectExt.w) && (h <= pLocal->m_rectExt.h) ) ;
	//
	pLocal->w = w ;
	pLocal->h = h ;
	//
	ESLAssert( pLocal->m_pRight == NULL ) ;
	if ( w < pLocal->m_rectExt.w )
	{
		pLocal->m_pRight =
			CreateLocalArea
				( pLocal->x + w, pLocal->y,
					pLocal->m_rectExt.w - w, h, pLocal ) ;
	}
	ESLAssert( pLocal->m_pUnder == NULL ) ;
	if ( h < pLocal->m_rectExt.h )
	{
		pLocal->m_pUnder =
			CreateLocalArea
				( pLocal->x, pLocal->y + h,
					pLocal->m_rectExt.w, pLocal->m_rectExt.h - h, pLocal ) ;
	}
}

// サブ領域確保（反復）
//////////////////////////////////////////////////////////////////////////////
SGLAreaAllocator::Local *
	SGLAreaAllocator::AllocateSubLocal
		( SGLAreaAllocator::Local * pLocal, int w, int h )
{
	if ( pLocal == NULL )
	{
		return	NULL ;
	}
	if ( (w > pLocal->m_rectExt.w)
		| (h > pLocal->m_rectExt.h) )
	{
		return	NULL ;
	}
	if ( pLocal->IsEmpty() )
	{
		AllocateLocalArea( pLocal, w, h ) ;
		return	pLocal ;
	}
	Local *	pAlloc ;
	if ( pLocal->m_pRight != NULL )
	{
		pAlloc = AllocateSubLocal( pLocal->m_pRight, w, h ) ;
		if ( pAlloc != NULL )
		{
			return	pAlloc ;
		}
	}
	if ( pLocal->m_pUnder != NULL )
	{
		pAlloc = AllocateSubLocal( pLocal->m_pUnder, w, h ) ;
		if ( pAlloc != NULL )
		{
			return	pAlloc ;
		}
	}
	return	NULL ;
}

// 領域解放
//////////////////////////////////////////////////////////////////////////////
void SGLAreaAllocator::FreeLocal( SGLAreaAllocator::Local * pLocal )
{
	if ( pLocal->m_pChild != NULL )
	{
		FreeLocal( pLocal->m_pChild ) ;
	}
	if ( pLocal->m_pRight != NULL )
	{
		FreeLocal( pLocal->m_pRight ) ;
	}
	if ( pLocal->m_pUnder != NULL )
	{
		FreeLocal( pLocal->m_pUnder ) ;
	}
	delete	pLocal ;
}


