
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像バッファ変換複製
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglConvertImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	//
	// 変換関数取得
	//
	PROC_CONVERT_COLOR_FORMAT	pfnCvtColor =
		sglGetColorFormatConvertor( imgDst.format, imgSrc.format ) ;
	PROC_CONVERT_COLOR_FORMAT	pfnPostCvtColor = NULL ;
	PROC_DECODE_PIXEL_COMPOSITION	pfnDecoder =
		sglGetPixelCompositionDecoder( imgSrc.format, imgSrc.depth ) ;
	PROC_ENCODE_PIXEL_COMPOSITION	pfnEncoder =
		sglGetPixelCompositionEncoder( imgDst.format, imgDst.depth ) ;
	if ( pfnCvtColor == NULL )
	{
		if ( (imgSrc.format & formatImageTypeMask) == formatImageBGR )
		{
			uint32_t	fmtInternal =
				formatImageRGB | (imgDst.format & ~formatImageTypeMask) ;
			pfnCvtColor =
				sglGetColorFormatConvertor( fmtInternal, imgSrc.format ) ;
			pfnPostCvtColor =
				sglGetColorFormatConvertor( imgDst.format, fmtInternal ) ;
		}
		else if ( (imgDst.format & formatImageTypeMask) == formatImageBGR )
		{
			uint32_t	fmtInternal =
				formatImageRGB | (imgDst.format & ~formatImageTypeMask) ;
			pfnCvtColor =
				sglGetColorFormatConvertor( fmtInternal, imgSrc.format ) ;
			pfnPostCvtColor =
				sglGetColorFormatConvertor( imgDst.format, fmtInternal ) ;
		}
		if ( (pfnCvtColor == NULL) | (pfnPostCvtColor == NULL) )
		{
			if ( ((pfnDecoder == NULL) & (pfnEncoder == NULL))
				| (imgDst.pitchPixel == imgSrc.pitchPixel) )
			{
				return	sglCopyImageBuffer
							( imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
			}
		}
	}
	//
	// 交差領域取得
	//
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 中間バッファ確保
	//
	SArray<SGLPalette>	arrayBuffer ;
	arrayBuffer.SetLength( infDst.width * 2 ) ;
	SGLPalette *	pDstBuf = arrayBuffer.GetArray() ;
	SGLPalette *	pSrcBuf = pDstBuf + infDst.width ;
	//
	// バッファポインタ準備
	//
	bool	fSrcPalette =
				((infSrc.format & formatImageFlagPalette) != 0)
								& (infSrc.ptrPalette != NULL) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	//
	if ( pfnCvtColor != NULL )
	{
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			SGLPalette *	pSrcLine = pSrcBuf ;
			if ( pfnDecoder != NULL )
			{
				pfnDecoder( pSrcLine, pSrcNextLine, infDst.width ) ;
				if ( fSrcPalette )
				{
					sglDecodePixelColorIndexed8
						( pSrcLine, infSrc.ptrPalette, infDst.width ) ;
				}
			}
			else
			{
				pSrcLine = (SGLPalette*) pSrcNextLine ;
			}
			if ( pfnEncoder != NULL )
			{
				pfnCvtColor( pDstBuf, pSrcLine, infDst.width ) ;
				if ( pfnPostCvtColor != NULL )
				{
					pfnPostCvtColor( pDstBuf, pDstBuf, infDst.width ) ;
				}
				pfnEncoder( pDstNextLine, pDstBuf, infDst.width ) ;
			}
			else
			{
				pfnCvtColor
					( (SGLPalette*) pDstNextLine,
								pSrcLine, infDst.width ) ;
				if ( pfnPostCvtColor != NULL )
				{
					pfnPostCvtColor
						( (SGLPalette*) pDstNextLine,
							(SGLPalette*) pDstNextLine, infDst.width ) ;
				}
			}
			pDstNextLine += infDst.pitchLine ;
			pSrcNextLine += infSrc.pitchLine ;
		}
	}
	else
	{
		ESLAssert( pfnCvtColor == NULL ) ;
		ESLAssert( (pfnDecoder != NULL) | (pfnEncoder != NULL) ) ;
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			if ( pfnDecoder != NULL )
			{
				if ( pfnEncoder != NULL )
				{
					pfnDecoder( pSrcBuf, pSrcNextLine, infDst.width ) ;
					if ( fSrcPalette )
					{
						sglDecodePixelColorIndexed8
							( pSrcBuf, infSrc.ptrPalette, infDst.width ) ;
					}
					pfnEncoder( pDstNextLine, pSrcBuf, infDst.width ) ;
				}
				else
				{
					pfnDecoder
						( (SGLPalette*) pDstNextLine,
									pSrcNextLine, infDst.width ) ;
				}
			}
			else
			{
				pfnEncoder
					( pDstNextLine,
						(SGLPalette*) pSrcNextLine, infDst.width ) ;
			}
			pDstNextLine += infDst.pitchLine ;
			pSrcNextLine += infSrc.pitchLine ;
		}
	}
	arrayBuffer.FinishArray() ;
	return	sglErrSuccess ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// 特殊変換
//////////////////////////////////////////////////////////////////////////////

// RGB⇔BGR 変換
SGLError SakuraGL::sglFlipCompositionRGBtoBGR( SGLImageBuffer& imgBuf )
{

	uint32_t	format = imgBuf.format ;
	switch ( format & formatImageTypeMask )
	{
	case	formatImageRGB:
		imgBuf.format = (format & ~formatImageTypeMask) | formatImageBGR ;
		break ;
	case	formatImageBGR:
		imgBuf.format = (format & ~formatImageTypeMask) | formatImageRGB ;
		break ;
	default:
		return	sglErrInvalidParam ;
	}
	if ( imgBuf.depth < 24 )
	{
		return	sglErrInvalidParam ;
	}
	uint8_t *	pNextLine = imgBuf.ptrBuffer ;
	if ( pNextLine == NULL )
	{
		return	sglErrInvalidParam ;
	}
	uint32_t	width = imgBuf.width ;
	uint32_t	height = imgBuf.height ;
	int32_t		pitchPixel = imgBuf.pitchPixel ;
	int32_t		pitchLine = imgBuf.pitchLine ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		uint8_t *	pNextPixel = pNextLine ;
		for ( uint32_t x = 0; x < width; x ++ )
		{
			uint8_t	temp = pNextPixel[0] ;
			pNextPixel[0] = pNextPixel[2] ;
			pNextPixel[2] = temp ;
			pNextPixel += pitchPixel ;
		}
		pNextLine += pitchLine ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 色空間変換関数
//////////////////////////////////////////////////////////////////////////////

// 色空間変換関数取得
PROC_CONVERT_COLOR_FORMAT
	SakuraGL::sglGetColorFormatConvertor( uint32_t fmtDst, uint32_t fmtSrc )
{
	bool	fAppendDstAlpha =
					!(fmtSrc & formatImageFlagAlpha)
							& ((fmtDst & formatImageFlagAlpha) != 0) ;
	const uint32_t	fmtOrgDst = fmtDst ;
	const uint32_t	fmtOrgSrc = fmtSrc ;
	fmtDst &= formatImageColorSpaceMask ;
	fmtSrc &= formatImageColorSpaceMask ;
	if ( fmtDst == formatImageRGB )
	{
		if ( fmtSrc == formatImageRGB )
		{
			if ( fAppendDstAlpha )
			{
				return	&SakuraGL::sglConvertFormatRGBtoARGB ;
			}
			if ( (fmtOrgSrc ^ fmtOrgDst) & formatImageFlagNoProductOfAlpha )
			{
				if ( fmtOrgSrc & formatImageFlagNoProductOfAlpha )
				{
					return	&SakuraGL::sglConvertFormatNPARGBtoARGB ;
				}
				else
				{
					return	&SakuraGL::sglConvertFormatARGBtoNPARGB ;
				}
			}
			return	NULL ;
		}
		else if ( fmtSrc == formatImageBGR )
		{
			if ( fAppendDstAlpha )
			{
				return	&SakuraGL::sglConvertFormatBGRtoARGB ;
			}
			else
			{
				return	&SakuraGL::sglConvertFormatRGBtoBGR ;
			}
		}
		else if ( fmtSrc == formatImageGray )
		{
			return	&SakuraGL::sglConvertFormatGraytoRGB ;
		}
		else if ( fmtSrc == formatImageYUV )
		{
			if ( fAppendDstAlpha )
			{
				return	&SakuraGL::sglConvertFormatYUVtoARGB ;
			}
			else
			{
				return	&SakuraGL::sglConvertFormatYUVtoRGB ;
			}
		}
		else if ( fmtSrc == formatImageHSB )
		{
			if ( fAppendDstAlpha )
			{
				return	&SakuraGL::sglConvertFormatHSBtoARGB ;
			}
			else
			{
				return	&SakuraGL::sglConvertFormatHSBtoRGB ;
			}
		}
	}
	else if ( fmtDst == formatImageBGR )
	{
		if ( fmtSrc == formatImageRGB )
		{
			if ( fAppendDstAlpha )
			{
				return	&SakuraGL::sglConvertFormatBGRtoARGB ;
			}
			else
			{
				return	&SakuraGL::sglConvertFormatRGBtoBGR ;
			}
		}
		else if ( fmtSrc == formatImageGray )
		{
			return	&SakuraGL::sglConvertFormatGraytoRGB ;
		}
	}
	else if ( fmtDst == formatImageGray )
	{
		if ( fmtSrc == formatImageRGB )
		{
			return	&SakuraGL::sglConvertFormatRGBtoGray ;
		}
	}
	else if ( fmtDst == formatImageYUV )
	{
		if ( fmtSrc == formatImageRGB )
		{
			return	&SakuraGL::sglConvertFormatRGBtoYUV ;
		}
	}
	else if ( fmtDst == formatImageHSB )
	{
		if ( fmtSrc == formatImageRGB )
		{
			return	&SakuraGL::sglConvertFormatRGBtoHSB ;
		}
	}
	return	NULL ;
}

// パレット展開関数
void __fastcall SakuraGL::sglDecodePixelColorIndexed8
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		pDst->ui32  = pSrc[pDst->ui32 & 0xFF].ui32 ;
		++ pDst ;
	}
	while ( -- nCount ) ;
}

// ARGB <--> ABGR
void __fastcall SakuraGL::sglConvertFormatRGBtoBGR
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	src = pSrc->ui32 ;
		pDst->ui32 =
			(src & 0xFF00FF00)
				| ((src >> 16) & 0xFF) | ((src << 16) & 0x00FF0000) ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// ARGB --> Gray
void __fastcall SakuraGL::sglConvertFormatRGBtoGray
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	nGray = (uint16_t) pSrc->argb.Red * 75
							+ (uint16_t) pSrc->argb.Green * 149
							+ ((uint16_t) pSrc->argb.Blue << 5) ;
		if ( (nGray & 0xFFFF0000) != 0 )
		{
			nGray = 0xFF00 ;
		}
		uint8_t	nGray8 = (uint8_t) (nGray >> 8) ;
		pDst->argb.Blue = nGray8 ;
		pDst->argb.Green = nGray8 ;
		pDst->argb.Red = nGray8 ;
		pDst->argb.Alpha = pSrc->argb.Alpha ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// Gray --> ARGB
void __fastcall SakuraGL::sglConvertFormatGraytoRGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint8_t	nGray = (uint8_t) pSrc->ui32 ;
		pDst->argb.Blue = nGray ;
		pDst->argb.Green = nGray ;
		pDst->argb.Red = nGray ;
		pDst->argb.Alpha = nGray ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// RGB --> YUV
void __fastcall SakuraGL::sglConvertFormatRGBtoYUV
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		int32_t	r = pSrc->argb.Red ;
		int32_t	g = pSrc->argb.Green ;
		int32_t	b = pSrc->argb.Blue ;
		//
		int32_t	y = (r * 75) + (g * 149) + (b << 5) ;
		if ( (y & 0xFFFF0000) != 0 )
		{
			y = 0xFF00 ;
		}
		int32_t	u = (r * -43) + (g * -85) + (b << 7) + 0x8000 ;
		if ( (u & 0xFFFF0000) != 0 )
		{
			u = (~u >> 31) & 0xFF00 ;
		}
		int32_t	v = (r * 121) + (g * -100) + (b * -21) + 0x8000 ;
		if ( (v & 0xFFFF0000) != 0 )
		{
			v = (~v >> 31) & 0xFF00 ;
		}
		pDst->yuv.Y = (uint8_t) (y >> 8) ;
		pDst->yuv.U = (uint8_t) (u >> 8) ;
		pDst->yuv.V = (uint8_t) (v >> 8) ;
		pDst->yuv.Alpha = pSrc->argb.Alpha ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// YUV --> RGB
void __fastcall SakuraGL::sglConvertFormatYUVtoRGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		int32_t	v = pSrc->yuv.V ;
		int32_t	u = pSrc->yuv.U ;
		int32_t	y = pSrc->yuv.Y ;
		//
		int32_t	v3 = ((v + (v << 1) - 0x180) >> 1) ;
		int32_t	u3 = (u + (u << 1) - 0x180) ;
		int32_t	u7 = (u3 + (u << 2) - 0x200) ;
		//
		int32_t	r = y + v3 ;
		if ( (r & 0xFFFFFF00) != 0 )
		{
			r = (~r >> 31) & 0xFF ;
		}
		int32_t	g = y - (v3 >> 1) - (u3 >> 3) ;
		if ( (g & 0xFFFFFF00) != 0 )
		{
			g = (~g >> 31) & 0xFF ;
		}
		int32_t	b = y + (u7 >> 2) ;
		if ( (b & 0xFFFFFF00) != 0 )
		{
			b = (~b >> 31) & 0xFF ;
		}
		pDst->argb.Blue = (uint8_t) b ;
		pDst->argb.Green = (uint8_t) g ;
		pDst->argb.Red = (uint8_t) r ;
		pDst->argb.Alpha = pSrc->argb.Alpha ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// RGB --> HSB
void __fastcall SakuraGL::sglConvertFormatRGBtoHSB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	nBlue = pSrc->argb.Blue ;
		uint32_t	nGreen = pSrc->argb.Green ;
		uint32_t	nRed = pSrc->argb.Red ;
		//
		// 最大値と最小値を取得
		//
		uint32_t	minC = nBlue ;
		uint32_t	maxC = nGreen ;
		if ( maxC < minC )
		{
			minC = nGreen ;
			maxC = nBlue ;
		}
		if ( minC > nRed )
		{
			minC = nRed ;
		}
		if ( maxC < nRed )
		{
			maxC = nRed ;
		}
		//
		// 輝度と彩度
		//
		uint32_t	nBrightness = (maxC + minC + 1) >> 1 ;
		int32_t		nSaturation = 0 ;
		pDst->hsb.Brightness = (uint8_t) nBrightness ;
		//
		// 輝度成分を RGB から除去する
		//
		if ( nBrightness <= 128 )
		{
			if ( nBrightness != 0 )
			{
				// Color = 128 * Color' / Brightness
				uint32_t	rb =
					(128 * 0x100 + (nBrightness >> 1)) / nBrightness ;
				//
				nRed   = (nRed * rb) >> 8 ;
				nGreen = (nGreen * rb) >> 8 ;
				nBlue  = (nBlue * rb) >> 8 ;
				//
				nSaturation = ((maxC * rb) >> 8) - 128 ;
			}
		}
		else
		{
			if ( nBrightness < 256 )
			{
				// Color = (Color' - 255) / (255 - Brightness) * 128 + 256
				int32_t	rb = 256 - nBrightness ;
				rb = (128 * 0x100 + (rb >> 1)) / rb ;
				//
				nRed   = ((((int32_t) nRed - 255) * rb) >> 8) + 256 ;
				nGreen = ((((int32_t) nGreen - 255) * rb) >> 8) + 256 ;
				nBlue  = ((((int32_t) nBlue - 255) * rb) >> 8) + 256 ;
				//
				nSaturation  = -((((int32_t) minC - 255) * rb) >> 8) - 128 ;
			}
		}
		//
		// 彩度
		//
		nSaturation = (int32_t) esl_clampi( nSaturation << 1, 0, 0xFF ) ;
		pDst->hsb.Saturation = (uint8_t) nSaturation ;
		//
		// 彩度成分を RGB から除去する
		//
		if ( nSaturation != 0 )
		{
			// Color = (Color' - 128) / Saturation + 128
			int32_t	rs =
				(int32_t) (255 * 256 + (nSaturation >> 1)) / nSaturation ;
			//
			nRed   = ((((int32_t) nRed - 128) * rs) >> 8) + 128 ;
			nGreen = ((((int32_t) nGreen - 128) * rs) >> 8) + 128 ;
			nBlue  = ((((int32_t) nBlue - 128) * rs) >> 8) + 128 ;
			//
			if ( nRed >= 0x100 )
			{
				nRed = ~(((int32_t) nRed) >> 31) & 0xFF ;
			}
			if ( nGreen >= 0x100 )
			{
				nGreen = ~(((int32_t) nGreen) >> 31) & 0xFF ;
			}
			if ( nBlue >= 0x100 )
			{
				nBlue = ~(((int32_t) nBlue) >> 31) & 0xFF ;
			}
		}
		//
		// 色相を計算する
		//
		//	  0 :  43 :  85 : 128 : 171 : 213 : 256
		//	 R =====----->            <-----===== R
		//	 <------==== G ====------->
		//	             <-----===== B =====------>
		//
		int32_t	c1, c2, c3 ;
		if ( (int32_t) nRed < (int32_t) nGreen )
		{
			if ( (int32_t) nGreen < (int32_t) nBlue )
			{
				c1 = nRed ;
				c2 = nGreen ;
				c3 = 171 ;
			}
			else
			{
				c1 = nBlue ;
				c2 = nRed ;
				c3 = 85 ;
			}
		}
		else
		{
			if ( (int32_t) nRed < (int32_t) nBlue )
			{
				c1 = nRed ;
				c2 = nGreen ;
				c3 = 171 ;
			}
			else
			{
				c1 = nGreen ;
				c2 = nBlue ;
				c3 = 0 ;
			}
		}
		if ( c1 >= c2 )
		{
			c3 += (c1 * 43) >> 8 ;
		}
		else
		{
			c3 -= (c2 * 43) >> 8 ;
		}
		pDst->hsb.Hue = (uint8_t) c3 ;
		pDst->hsb.Alpha = pSrc->argb.Alpha ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// HSB --> RGB
void __fastcall SakuraGL::sglConvertFormatHSBtoRGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		//
		// 色相を RGB へ変換する
		//
		//	  0 :  43 :  85 : 128 : 171 : 213 : 256
		//	 R =====----->            <-----===== R
		//	 <------==== G ====------->
		//	             <-----===== B =====------>
		//
		int32_t		nSignMask ;
		uint32_t	nHue = pSrc->hsb.Hue ;
		//
		// 赤色
		//
		int32_t		nRed = (int32_t) nHue ;
		if ( nHue > 128 )
		{
			nRed = nHue - 256 ;
		}
		nSignMask = (nRed >> 31) ;
		nRed = (nRed ^ nSignMask) - nSignMask ;
		nRed = (86 - nRed) * (0xFF00 / 43) >> 8 ;
		if ( nRed < 0 )
		{
			nRed = 0 ;
		}
		else if ( nRed >= 0x100 )
		{
			nRed = 0xFF ;
		}
		nRed -= 128 ;
		//
		// 緑色
		//
		int32_t	nGreen = (int32_t) nHue - 85 ;
		nSignMask = (nGreen >> 31) ;
		nGreen = (nGreen ^ nSignMask) - nSignMask ;
		nGreen = (86 - nGreen) * (0xFF00 / 43) >> 8 ;
		if ( nGreen < 0 )
		{
			nGreen = 0 ;
		}
		else if ( nGreen >= 0x100 )
		{
			nGreen = 0xFF ;
		}
		nGreen -= 128 ;
		//
		// 青色
		//
		int32_t	nBlue = nHue - 170 ;
		nSignMask = (nBlue >> 31) ;
		nBlue = (nBlue ^ nSignMask) - nSignMask ;
		nBlue = (86 - nBlue) * (0xFF00 / 43) >> 8 ;
		if ( nBlue < 0 )
		{
			nBlue = 0 ;
		}
		else if ( nBlue >= 0x100 )
		{
			nBlue = 0xFF ;
		}
		nBlue -= 128 ;
		//
		// 彩度を適用
		//
		int32_t	nSaturation = pSrc->hsb.Saturation ;
		nRed   = ((nRed * nSaturation) >> 8) + 128 ;
		nGreen = ((nGreen * nSaturation) >> 8) + 128 ;
		nBlue  = ((nBlue * nSaturation) >> 8) + 128 ;
		//
		// 輝度を適用
		//
		int32_t	nBrightness = pSrc->hsb.Brightness ;
		if ( nBrightness <= 128 )
		{
			nRed   = (nRed * nBrightness) >> 7 ;
			nGreen = (nGreen * nBrightness) >> 7 ;
			nBlue  = (nBlue * nBrightness) >> 7 ;
		}
		else
		{
			nBrightness = 255 - nBrightness ;
			nRed   = (((nRed - 255) * nBrightness) >> 7) + 255 ;
			nGreen = (((nGreen - 255) * nBrightness) >> 7) + 255 ;
			nBlue  = (((nBlue - 255) * nBrightness) >> 7) + 255 ;
		}
		pDst->argb.Blue  = (uint8_t) nBlue ;
		pDst->argb.Green = (uint8_t) nGreen ;
		pDst->argb.Red   = (uint8_t) nRed ;
		pDst->argb.Alpha = pSrc->hsb.Alpha ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// RGB --> ARGB
void __fastcall SakuraGL::sglConvertFormatRGBtoARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		pDst->ui32 = pSrc->ui32 | 0xFF000000 ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// ARGB --> No Product of Alpha ARGB
void __fastcall SakuraGL::sglConvertFormatARGBtoNPARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	const uint32_t *	pDivTable = &SGLPaintBuffer::m_tableDivBlend[0] ;
	if ( nCount > 0 )
	do
	{
		uint32_t	a = pSrc->argb.Alpha ;
		if ( (a > 0) && (a < 0xFF) )
		{
			uint32_t	ra = pDivTable[a ^ 0xff] ;
			uint32_t	b = (pSrc->argb.Blue * ra) >> 6 ;
			uint32_t	g = (pSrc->argb.Green * ra) >> 6 ;
			uint32_t	r = (pSrc->argb.Red * ra) >> 6 ;
			if ( b >= 0x100 )
			{
				b = 0xFF ;
			}
			if ( g >= 0x100 )
			{
				g = 0xFF ;
			}
			if ( r >= 0x100 )
			{
				r = 0xFF ;
			}
			pDst->argb.Blue = (uint8_t) b ;
			pDst->argb.Green = (uint8_t) g ;
			pDst->argb.Red = (uint8_t) r ;
			pDst->argb.Alpha = (uint8_t) a ;
		}
		else
		{
			pDst->ui32 = pSrc->ui32 ;
		}
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// No Product of Alpha ARGB --> ARGB
void __fastcall SakuraGL::sglConvertFormatNPARGBtoARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	a = pSrc->argb.Alpha ;
		if ( a > 0 )
		{
			if ( a < 0xFF )
			{
				pDst->ui32 =
					(a << 24)
						| (sglPackedColorMul
								( pSrc->ui32, a + 1 ) & 0x00FFFFFF) ;
			}
			else
			{
				pDst->ui32 = pSrc->ui32 ;
			}
		}
		else
		{
			pDst->ui32 = 0 ;
		}
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// BGR --> ARGB  |  RGB --> ABGR
void __fastcall SakuraGL::sglConvertFormatBGRtoARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	src = pSrc->ui32 ;
		pDst->ui32 =
			(src & 0xFF00FF00) | 0xFF000000
				| ((src >> 16) & 0xFF) | ((src << 16) & 0x00FF0000) ;
		//
		++ pDst ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

void __fastcall SakuraGL::sglConvertFormatYUVtoARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	sglConvertFormatYUVtoRGB( pDst, pSrc, nCount ) ;
	sglConvertFormatRGBtoARGB( pDst, pDst, nCount ) ;
}

void __fastcall SakuraGL::sglConvertFormatHSBtoARGB
	( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount )
{
	sglConvertFormatHSBtoRGB( pDst, pSrc, nCount ) ;
	sglConvertFormatRGBtoARGB( pDst, pDst, nCount ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ピクセルコンポジション変換関数
//////////////////////////////////////////////////////////////////////////////

// ピクセルコンポジション変換関数テーブル [depth/8-1][alpha?]
static const SakuraGL::PROC_DECODE_PIXEL_COMPOSITION
						m_tablePixelCompositionDecoder[4][2] =
{
	// 8 bits
	{
		&SakuraGL::sglDecodePixelCompositionGray,
		&SakuraGL::sglDecodePixelCompositionGray,
	},
	// 16 bits
	{
		&SakuraGL::sglDecodePixelCompositionRGB565,
		&SakuraGL::sglDecodePixelCompositionRGBA4444,
	},
	// 24 bits
	{
		&SakuraGL::sglDecodePixelCompositionRGB24,
		&SakuraGL::sglDecodePixelCompositionRGB24,
	},
	// 32 bits
	{
		NULL,
		NULL,
	},
} ;

static const SakuraGL::PROC_ENCODE_PIXEL_COMPOSITION
						m_tablePixelCompositionEncoder[4][2] =
{
	// 8 bits
	{
		&SakuraGL::sglEncodePixelCompositionGray,
		&SakuraGL::sglEncodePixelCompositionGray,
	},
	// 16 bits
	{
		&SakuraGL::sglEncodePixelCompositionRGB565,
		&SakuraGL::sglEncodePixelCompositionRGBA4444,
	},
	// 24 bits
	{
		&SakuraGL::sglEncodePixelCompositionRGB24,
		&SakuraGL::sglEncodePixelCompositionRGB24,
	},
	// 32 bits
	{
		NULL,
		NULL,
	},
} ;

// ピクセルコンポジション変換関数取得
PROC_DECODE_PIXEL_COMPOSITION
	SakuraGL::sglGetPixelCompositionDecoder( uint32_t fmt, uint32_t depth )
{
	if ( !(fmt & (formatImageTypeMask & ~formatImageColorSpaceMask)) )
	{
		uint32_t	nDepth = (depth >> 3) ;
		if ( (nDepth >= 1) & (nDepth <= 4) )
		{
			return	m_tablePixelCompositionDecoder[nDepth - 1]
							[((fmt & formatImageFlagAlpha) != 0) & 0x01] ;
		}
	}
	else if ( (fmt & formatImageColorSpaceMask) == formatImageYUV )
	{
		switch ( fmt & formatImageTypeMask )
		{
		case	formatImageYUV2:
			return	&SakuraGL::sglDecodePixelCompositionYUV2 ;
		case	formatImageYVYU:
			return	&SakuraGL::sglDecodePixelCompositionYVYU ;
		case	formatImageUYVY:
			return	&SakuraGL::sglDecodePixelCompositionUYVY ;
		case	formatImageYUYV:
			return	&SakuraGL::sglDecodePixelCompositionYUYV ;
		default:
			break ;
		}
	}
	return	NULL ;
}

PROC_ENCODE_PIXEL_COMPOSITION
	SakuraGL::sglGetPixelCompositionEncoder( uint32_t fmt, uint32_t depth )
{
	uint32_t	nDepth = (depth >> 3) ;
	if ( (nDepth >= 1) & (nDepth <= 4) )
	{
		return	m_tablePixelCompositionEncoder[nDepth - 1]
						[((fmt & formatImageFlagAlpha) != 0) & 0x01] ;
	}
	return	NULL ;
}

// ARGB32 --> ARGB32
void __fastcall SakuraGL::sglDecodePixelCompositionARGB32
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	eslMoveMemory( pDst, pSrc, nCount * 4 ) ;
}

// ARGB32 --> ARGB32
void __fastcall SakuraGL::sglEncodePixelCompositionARGB32
	( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount )
{
	eslMoveMemory( pDst, pSrc, nCount * 4 ) ;
}

// RGB24 --> ARGB32
void __fastcall SakuraGL::sglDecodePixelCompositionRGB24
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		pDst->argb.Blue  = pSrc[0] ;
		pDst->argb.Green = pSrc[1] ;
		pDst->argb.Red   = pSrc[2] ;
		pDst->argb.Alpha = 0xFF ;
		++ pDst ;
		pSrc += 3 ;
	}
	while ( -- nCount ) ;
}

// ARGB32 --> RGB24
void __fastcall SakuraGL::sglEncodePixelCompositionRGB24
	( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		pDst[0] = pSrc->argb.Blue ;
		pDst[1] = pSrc->argb.Green ;
		pDst[2] = pSrc->argb.Red ;
		pDst += 3 ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// RGB565 --> ARGB32
void __fastcall SakuraGL::sglDecodePixelCompositionRGB565
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	const uint16_t *	pwSrc = (const uint16_t*) pSrc ;
	if ( nCount > 0 )
	do
	{
		uint32_t	rgb = *(pwSrc ++) ;
		uint32_t	rb = ((rgb & 0xf800) << 8) | ((rgb & 0x001f) << 3) ;
		uint32_t	g = ((rgb & 0x07e0) << 5) ;
		pDst->ui32 = 0xff000000
					| rb | ((rb >> 5) & 0x00070007)
					| g | ((g >> 6) & 0x00000300) ;
		++ pDst ;
	}
	while ( -- nCount ) ;
}

// ARGB32 --> RGB565
void __fastcall SakuraGL::sglEncodePixelCompositionRGB565
	( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount )
{
	uint16_t *	pwDst = (uint16_t*) pDst ;
	if ( nCount > 0 )
	do
	{
		uint32_t	rgb = pSrc->ui32 ;
		*(pwDst ++) =
			(uint16_t) (((rgb & 0x000000f8) >> 3)
						| ((rgb & 0x0000fc00) >> 5)
						| ((rgb & 0x00f80000) >> 8)) ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// RGBA4444 --> ARGB32
void __fastcall SakuraGL::sglDecodePixelCompositionRGBA4444
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	const uint16_t *	pwSrc = (const uint16_t*) pSrc ;
	if ( nCount > 0 )
	do
	{
		uint32_t	rgb = *(pwSrc ++) ;
		rgb = ((rgb & 0xf000) << 4)
				| (rgb & 0x0f00)
				| ((rgb & 0x00f0) >> 4)
				| ((rgb & 0x000f) << 24) ;
		pDst->ui32 = rgb | (rgb << 4) ;
		++ pDst ;
	}
	while ( -- nCount ) ;
}

// ARGB32 --> RGBA4444
void __fastcall SakuraGL::sglEncodePixelCompositionRGBA4444
	( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount )
{
	uint16_t *	pwDst = (uint16_t*) pDst ;
	if ( nCount > 0 )
	do
	{
		uint32_t	rgb = pSrc->ui32 ;
		*(pwDst ++) =
			(uint16_t) ((rgb & 0x000000f0)
					| ((rgb & 0x0000f000) >> 4)
					| ((rgb & 0x00f00000) >> 8)
					| ((rgb & 0xf0000000) >> 28)) ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// Gray --> ARGB32
void __fastcall SakuraGL::sglDecodePixelCompositionGray
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint8_t	v = *(pSrc ++) ;
		pDst->argb.Blue = v ;
		pDst->argb.Green = v ;
		pDst->argb.Red = v ;
		pDst->argb.Alpha = 0xFF ;
		++ pDst ;
	}
	while ( -- nCount ) ;
}

// ARGB32 --> Gray
void __fastcall SakuraGL::sglEncodePixelCompositionGray
	( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		*(pDst ++) = pSrc->argb.Blue ;
		++ pSrc ;
	}
	while ( -- nCount ) ;
}

// YUV2 --> YUV
void __fastcall SakuraGL::sglDecodePixelCompositionYUV2
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	nCount >>= 1 ;
	if ( nCount > 0 )
	do
	{
		pDst[0].yuv.Y = pSrc[0] ;
		pDst[0].yuv.U = pSrc[1] ;
		pDst[0].yuv.V = pSrc[3] ;
		pDst[0].yuv.Alpha = 0xFF ;
		pDst[1].yuv.Y = pSrc[2] ;
		pDst[1].yuv.U = pSrc[1] ;
		pDst[1].yuv.V = pSrc[3] ;
		pDst[1].yuv.Alpha = 0xFF ;
		pSrc += 4 ;
		pDst += 2 ;
	}
	while ( -- nCount ) ;
}

void __fastcall SakuraGL::sglDecodePixelCompositionYVYU
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	nCount >>= 1 ;
	if ( nCount > 0 )
	do
	{
		pDst[0].yuv.Y = pSrc[0] ;
		pDst[0].yuv.U = pSrc[3] ;
		pDst[0].yuv.V = pSrc[1] ;
		pDst[0].yuv.Alpha = 0xFF ;
		pDst[1].yuv.Y = pSrc[2] ;
		pDst[1].yuv.U = pSrc[3] ;
		pDst[1].yuv.V = pSrc[1] ;
		pDst[1].yuv.Alpha = 0xFF ;
		pSrc += 4 ;
		pDst += 2 ;
	}
	while ( -- nCount ) ;
}

void __fastcall SakuraGL::sglDecodePixelCompositionUYVY
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	nCount >>= 1 ;
	if ( nCount > 0 )
	do
	{
		pDst[0].yuv.Y = pSrc[1] ;
		pDst[0].yuv.U = pSrc[0] ;
		pDst[0].yuv.V = pSrc[2] ;
		pDst[0].yuv.Alpha = 0xFF ;
		pDst[1].yuv.Y = pSrc[3] ;
		pDst[1].yuv.U = pSrc[0] ;
		pDst[1].yuv.V = pSrc[2] ;
		pDst[1].yuv.Alpha = 0xFF ;
		pSrc += 4 ;
		pDst += 2 ;
	}
	while ( -- nCount ) ;
}

void __fastcall SakuraGL::sglDecodePixelCompositionYUYV
	( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount )
{
	nCount >>= 1 ;
	if ( nCount > 0 )
	do
	{
		pDst[0].yuv.Y = pSrc[0] ;
		pDst[0].yuv.U = pSrc[1] ;
		pDst[0].yuv.V = pSrc[3] ;
		pDst[0].yuv.Alpha = 0xFF ;
		pDst[1].yuv.Y = pSrc[2] ;
		pDst[1].yuv.U = pSrc[1] ;
		pDst[1].yuv.V = pSrc[3] ;
		pDst[1].yuv.Alpha = 0xFF ;
		pSrc += 4 ;
		pDst += 2 ;
	}
	while ( -- nCount ) ;
}



//////////////////////////////////////////////////////////////////////////////
// S3 Texture Compression DXT1
//////////////////////////////////////////////////////////////////////////////

SGLError SakuraGL::sglCompressARGBtoS3TC_DXT1
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc, S3TC_DitheringMethod dithering )
{
	ESLAssert( (imgDst.width % 4) == 0 ) ;
	ESLAssert( (imgDst.height % 4) == 0 ) ;
	ESLAssert( imgDst.depth == sizeof(S3TC_DXT1_Block) * 8 / 16 ) ;
	ESLAssert( imgDst.pitchLine == (imgDst.width/4) * sizeof(S3TC_DXT1_Block) / 4 ) ;
	ESLAssert( imgSrc.depth == 32 ) ;
	ESLAssert( imgSrc.pitchPixel == 4 ) ;
	if ( (imgSrc.depth != 32)
		|| (imgSrc.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}

	S3TC_SourceParam	src ;
	src.alphaBlack = ((imgDst.format & formatImageFlagAlpha) != 0) ;
	src.dithering = dithering ;

	const int	wBlock = esl_min( imgDst.width/4, imgSrc.width/4 ) ;
	const int	hBlock = esl_min( imgDst.height/4, imgSrc.height/4 ) ;
	for ( int by = 0; by < hBlock; by ++ )
	{
		S3TC_DXT1_Block *	pDstBlock =
			(S3TC_DXT1_Block*) (imgDst.ptrBuffer + imgDst.pitchLine * by * 4) ;
		for ( int bx = 0; bx < wBlock; bx ++ )
		{
			const uint8_t *		pSrcBlock =
				imgSrc.ptrBuffer + imgSrc.pitchLine * by * 4
									+ imgSrc.pitchPixel * bx * 4 ;
			for ( int i = 0; i < 4; i ++ )
			{
				const SGLPalette *	pSrcLine = (const SGLPalette*) pSrcBlock ;
				src.px[i*4]   = pSrcLine[0] ;
				src.px[i*4+1] = pSrcLine[1] ;
				src.px[i*4+2] = pSrcLine[2] ;
				src.px[i*4+3] = pSrcLine[3] ;
				pSrcBlock += imgSrc.pitchLine ;
			}

			sglCompressS3TC_DXT1Block( *pDstBlock, src ) ;

			pDstBlock ++ ;
		}
	}
	return	sglErrSuccess ;
}

void __fastcall SakuraGL::sglCompressS3TC_DXT1Block
	( S3TC_DXT1_Block& dxt1, const S3TC_SourceParam& src )
{
	struct	IntARGB
	{
		int	Blue ;
		int	Green ;
		int	Red ;
		int	Alpha ;
	} ;

	// 入力データ成型
	uint16_t	srcRGB565[16] ;
	S3DVector	vSrcColor[16] ;
	float		fpVar1[16] ;
	bool		fBlack[16] ;

	for ( int i = 0; i < 16; i ++ )
	{
		srcRGB565[i] = sglRGB24toRGB565( src.px[i] ) ;

		vSrcColor[i].x = (float) src.px[i].argb.Blue ;
		vSrcColor[i].y = (float) src.px[i].argb.Green ;
		vSrcColor[i].z = (float) src.px[i].argb.Red ;

		fpVar1[i] = 0.0f ;
		fBlack[i] = false ;

		if ( src.alphaBlack )
		{
			if ( src.px[i].argb.Alpha < 0x80 )
			{
				fBlack[i] = true ;
			}
		}
		else
		{
			if ( (src.px[i].argb.Blue < 8)
				&& (src.px[i].argb.Green < 4)
				&& (src.px[i].argb.Red < 8) )
			{
				fBlack[i] = true ;
			}
		}
	}

	// 最頻色（仮）選択
	float	fpVarMax = 0.0f ;
	int		iVarMax = 0 ;
	float	fpAvgDelta = 0.0 ;
	int		nOpaque = 0 ;

	for ( int i = 0; i < 16; i ++ )
	{
		if ( fBlack[i] )
		{
			continue ;
		}
		S3DVector	vBase = vSrcColor[i] ;
		for ( int j = 0; j < 16; j ++ )
		{
			if ( (i != j) && !fBlack[j] )
			{
				S3DVector	vDelta = vSrcColor[j] - vBase ;
				const float	d2 = vDelta.InnerProduct( vDelta ) ;
				fpAvgDelta += d2 ;
				fpVar1[i] += (float) 0x400 / (d2 + 16.0f) ;
			}
		}
		if ( fpVarMax < fpVar1[i] )
		{
			fpVarMax = fpVar1[i] ;
			iVarMax = i ;
		}
		nOpaque ++ ;
	}
	if ( nOpaque >= 2 )
	{
		fpAvgDelta /= (float) (nOpaque * (nOpaque - 1)) ;
	}
	S3DVector	vColor1 = vSrcColor[iVarMax] ;

	// 次最頻色（仮）選択
	float	fpVarMax2 = 0.0f ;
	int		iVarMax2 = iVarMax ;
	for ( int i = 0; i < 16; i ++ )
	{
		if ( fBlack[i] || (i == iVarMax)
					|| (srcRGB565[i] == srcRGB565[iVarMax]) )
		{
			continue ;
		}
		S3DVector	vDelta = vSrcColor[i] - vColor1 ;
		const float	d2 = vDelta.InnerProduct( vDelta ) ;
		if ( d2 < fpAvgDelta * 0.5f )
		{
			continue ;
		}
		const float	fpVar2 = fpVar1[i] * d2 ;
		if ( fpVarMax2 < fpVar2 )
		{
			fpVarMax2 = fpVar2 ;
			iVarMax2 = i ;
		}
	}
	S3DVector	vColor2 = vSrcColor[iVarMax2] ;

	// （仮）最頻色修正
	fpVarMax = 0.0f ;
	for ( int i = 0; i < 16; i ++ )
	{
		if ( fBlack[i] || (i == iVarMax2)
					|| (srcRGB565[i] == srcRGB565[iVarMax2]) )
		{
			continue ;
		}
		S3DVector	vDelta = vSrcColor[i] - vColor2 ;
		const float	d2 = vDelta.InnerProduct( vDelta ) ;
		if ( d2 < fpAvgDelta * 0.5f )
		{
			continue ;
		}
		const float	fpVar = fpVar1[i] * d2 ;
		if ( fpVarMax < fpVar )
		{
			fpVarMax = fpVar ;
			iVarMax = i ;
		}
	}
	vColor1 = vSrcColor[iVarMax] ;

	// レンジ延伸
	S3DVector	vCenter = (vColor1 + vColor2) * 0.5f ;
	S3DVector	vDeltaDir = (vColor2 - vColor1).Normalized() ;
	float		fpColorLen1 = vDeltaDir.InnerProduct( vColor1 - vCenter ) ;
	float		fpColorLen2 = vDeltaDir.InnerProduct( vColor2 - vCenter ) ;
	for ( int i = 0; i < 16; i ++ )
	{
		if ( fBlack[i] )
		{
			continue ;
		}
		S3DVector	vDelta = vSrcColor[i] - vCenter ;
		const float	fpLen = vDeltaDir.InnerProduct( vDelta ) ;
		const float	cosDir = fpLen / (float) vDelta.Absolute() ;
		if ( (fpLen < fpColorLen1) && (cosDir < -0.966f) )
		{
			fpColorLen1 = fpLen ;
			iVarMax = i ;
		}
		else if ( (fpLen > fpColorLen2) && (cosDir > 0.966f) )
		{
			fpColorLen2 = fpLen ;
			iVarMax2 = i ;
		}
	}

	// 色決定
	uint16_t	rgb16[2] ;
	IntARGB		rgbColor[2] ;

	rgb16[0] = srcRGB565[iVarMax] ;
	rgb16[1] = srcRGB565[iVarMax2] ;

	for ( int i = 0; i < 2; i ++ )
	{
		SGLPalette	rgb32 = sglRGB565toARGB32( rgb16[i] ) ;
		rgbColor[i].Blue = rgb32.argb.Blue ;
		rgbColor[i].Green = rgb32.argb.Green ;
		rgbColor[i].Red = rgb32.argb.Red ;
		rgbColor[i].Alpha = 0xFF ;
	}

	// ピックアップ色
	IntARGB	rgbaIndex[4] ;
	bool	flagWithoutBlack = true ;
	if ( (nOpaque >= 12) && (rgb16[0] != rgb16[1]) )
	{
		if ( rgb16[0] < rgb16[1] )
		{
			uint16_t	temp = rgb16[0] ;
			IntARGB		rgbTemp = rgbColor[0] ;
			rgb16[0] = rgb16[1] ;
			rgbColor[0] = rgbColor[1] ;
			rgb16[1] = temp ;
			rgbColor[1] = rgbTemp ;
		}
		ESLAssert( rgb16[0] > rgb16[1] ) ;

		rgbaIndex[0] = rgbColor[0] ;
		rgbaIndex[1] = rgbColor[1] ;

		rgbaIndex[2].Blue = (rgbColor[0].Blue * 2 + rgbColor[1].Blue) / 3 ;
		rgbaIndex[2].Green = (rgbColor[0].Green * 2 + rgbColor[1].Green) / 3 ;
		rgbaIndex[2].Red = (rgbColor[0].Red * 2 + rgbColor[1].Red) / 3 ;
		rgbaIndex[2].Alpha = 0xFF ;

		rgbaIndex[3].Blue = (rgbColor[0].Blue + rgbColor[1].Blue * 2) / 3 ;
		rgbaIndex[3].Green = (rgbColor[0].Green + rgbColor[1].Green * 2) / 3 ;
		rgbaIndex[3].Red = (rgbColor[0].Red + rgbColor[1].Red * 2) / 3 ;
		rgbaIndex[3].Alpha = 0xFF ;
	}
	else
	{
		if ( rgb16[0] > rgb16[1] )
		{
			uint16_t	temp = rgb16[0] ;
			IntARGB		rgbTemp = rgbColor[0] ;
			rgb16[0] = rgb16[1] ;
			rgbColor[0] = rgbColor[1] ;
			rgb16[1] = temp ;
			rgbColor[1] = rgbTemp ;
		}
		ESLAssert( rgb16[0] <= rgb16[1] ) ;

		rgbaIndex[0] = rgbColor[0] ;
		rgbaIndex[1] = rgbColor[1] ;

		rgbaIndex[2].Blue = (rgbColor[0].Blue + rgbColor[1].Blue) / 3 ;
		rgbaIndex[2].Green = (rgbColor[0].Green + rgbColor[1].Green) / 3 ;
		rgbaIndex[2].Red = (rgbColor[0].Red + rgbColor[1].Red) / 3 ;
		rgbaIndex[2].Alpha = 0xFF ;

		rgbaIndex[3].Blue = 0 ;
		rgbaIndex[3].Green = 0 ;
		rgbaIndex[3].Red = 0 ;
		rgbaIndex[3].Alpha = 0 ;
		flagWithoutBlack = false ;
	}

	dxt1.rgb16[0] = rgb16[0] ;
	dxt1.rgb16[1] = rgb16[1] ;

	// インデックス化
	uint8_t	index[16] ;
	IntARGB	rgbaError ;
	rgbaError.Blue = 0 ;
	rgbaError.Green = 0 ;
	rgbaError.Red = 0 ;
	rgbaError.Alpha = 0 ;

	for ( int i = 0; i < 16; i ++ )
	{
		if ( flagWithoutBlack || !fBlack[i] )
		{
			IntARGB	rgbaSrc ;
			rgbaSrc.Blue = src.px[i].argb.Blue ;
			rgbaSrc.Green = src.px[i].argb.Green ;
			rgbaSrc.Red = src.px[i].argb.Red ;
			rgbaSrc.Alpha = src.px[i].argb.Alpha ;

			int	iNearest = 0 ;
			int	d2Min = 0xffffff ;
			for ( int j = 0; j < 4; j ++ )
			{
				IntARGB	rgbaDelta ;
				rgbaDelta.Blue = rgbaIndex[j].Blue - rgbaSrc.Blue + rgbaError.Blue ;
				rgbaDelta.Green = rgbaIndex[j].Green - rgbaSrc.Green + rgbaError.Green ;
				rgbaDelta.Red = rgbaIndex[j].Red - rgbaSrc.Red + rgbaError.Red ;
				rgbaDelta.Alpha = rgbaIndex[j].Alpha - rgbaSrc.Alpha ;

				int	d2 = rgbaDelta.Blue * rgbaDelta.Blue
						+ rgbaDelta.Green * rgbaDelta.Green
						+ rgbaDelta.Red * rgbaDelta.Red ;
				if ( src.alphaBlack )
				{
					d2 += rgbaDelta.Alpha * rgbaDelta.Alpha ;
				}
				if ( d2 < d2Min )
				{
					iNearest = j ;
					d2Min = d2 ;
				}
			}
			index[i] = (uint8_t) iNearest ;

			if ( src.dithering != s3tcNoDithering )
			{
				rgbaError.Blue += rgbaIndex[iNearest].Blue - rgbaSrc.Blue ;
				rgbaError.Green += rgbaIndex[iNearest].Green - rgbaSrc.Green ;
				rgbaError.Red += rgbaIndex[iNearest].Red - rgbaSrc.Red ;

			}
		}
		else
		{
			index[i] = 3 ;
		}

		if ( (i & 0x03) == 0x03 )
		{
			switch ( src.dithering )
			{
			case	s3tcErrorDiffusionByLine:
				rgbaError.Blue = 0 ;
				rgbaError.Green = 0 ;
				rgbaError.Red = 0 ;
				break ;

			case	s3tcErrorDiffusionByLine2:
				rgbaError.Blue /= 2 ;
				rgbaError.Green /= 2 ;
				rgbaError.Red /= 2 ;
				break ;

			default:
				break ;
			}
		}
	}

	for ( int i = 0, j = 0; i < 4; i ++, j += 4 )
	{
		dxt1.code[i] = index[j] | (index[j+1] << 2)
						| (index[j+2] << 4) | (index[j+3] << 6) ;
	}
}

uint16_t __fastcall SakuraGL::sglRGB24toRGB565( SGLPalette argb32 )
{
	return	(uint16_t) ((esl_min( (((int) argb32.argb.Red) >> 3), 0x1f ) << 11)
						| (esl_min( (((int) argb32.argb.Green) >> 2), 0x3f ) << 5)
						| esl_min( (((int) argb32.argb.Blue) >> 3), 0x1f )) ;
}

SGLPalette __fastcall SakuraGL::sglRGB565toARGB32( uint16_t rgb565 )
{
	SGLPalette	argb32 ;
	argb32.argb.Blue = (uint8_t) (rgb565 & 0x1f) ;
	argb32.argb.Green = (uint8_t) ((rgb565 >> 5) & 0x3f) ;
	argb32.argb.Red = (uint8_t) ((rgb565 >> 11) & 0x1f) ;
	argb32.argb.Alpha = 0xff ;
	argb32.argb.Blue = (argb32.argb.Blue << 3) | (argb32.argb.Blue >> 2) ;
	argb32.argb.Green = (argb32.argb.Green << 2) | (argb32.argb.Green >> 4) ;
	argb32.argb.Red = (argb32.argb.Red << 3) | (argb32.argb.Red >> 2) ;
	return	argb32 ;
}

