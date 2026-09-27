
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像フィルタ
//////////////////////////////////////////////////////////////////////////////

// αチャネルで RGB を積算
//////////////////////////////////////////////////////////////////////////////
void SakuraGL::sglMultiplyRGBAlpha( SGLPalette* pPixels, size_t nCount )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	argb = pPixels->ui32 ;
		uint32_t	a = (argb >> 24) + 1 ;
		uint32_t	rb = ((argb & 0x00FF00FF) * a) >> 8 ;
		uint32_t	g = ((argb & 0x0000FF00) * a) >> 8 ;
		pPixels->ui32 =
			(argb & 0xFF000000) | (rb & 0x00FF00FF) | (g & 0x0000FF00) ;
		++ pPixels ;
	}
	while ( -- nCount ) ;
}

#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglMultiplyImageRGBAlpha( const SGLImageBuffer& imgbuf )
{
	if ( (imgbuf.depth != 32) || (imgbuf.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint32_t	width = imgbuf.width ;
	uint32_t	height = imgbuf.height ;
	int32_t		pitchLine = imgbuf.pitchLine ;
	uint8_t *	pbytNextLine = imgbuf.ptrBuffer ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		sglMultiplyRGBAlpha
			( (SGLPalette*) pbytNextLine, (size_t) width ) ;
		pbytNextLine += pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 透明部分に色を流し込む（背景色に描画した結果を生成）
//////////////////////////////////////////////////////////////////////////////
void SakuraGL::sglBlendBackgroundColor
	( SGLPalette* pPixels, size_t nCount, const SGLPalette argbBackColor )
{
	if ( nCount > 0 )
	do
	{
		uint32_t	argb = pPixels->ui32 ;
		if ( argb == 0 )
		{
			*pPixels = argbBackColor ;
		}
		else if ( (argb & 0xFF000000) != 0xFF000000 )
		{
			pPixels->ui32 =
				sglPackedColorBlend( argbBackColor, pPixels->ui32 ) ;
		}
		++ pPixels ;
	}
	while ( -- nCount ) ;
}

#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglBlendImageBackgroundColor
	( const SGLImageBuffer& imgbuf, const SGLPalette argbBackColor )
{
	if ( (imgbuf.depth != 32) || (imgbuf.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint32_t	width = imgbuf.width ;
	uint32_t	height = imgbuf.height ;
	int32_t		pitchLine = imgbuf.pitchLine ;
	uint8_t *	pbytNextLine = imgbuf.ptrBuffer ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		sglBlendBackgroundColor
			( (SGLPalette*) pbytNextLine, (size_t) width, argbBackColor ) ;
		pbytNextLine += pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// RGB チャネルを Gray 化
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglMakeGrayImageFromRGB( const SGLImageBuffer& imgbuf )
{
	if ( (imgbuf.depth != 32) || (imgbuf.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint32_t	width = imgbuf.width ;
	uint32_t	height = imgbuf.height ;
	int32_t		pitchLine = imgbuf.pitchLine ;
	uint8_t *	pbytNextLine = imgbuf.ptrBuffer ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		SGLPalette *	pPixels = (SGLPalette*) pbytNextLine ;
		uint32_t	nCount = width ;
		if ( nCount > 0 )
		do
		{
			uint32_t	nGray = (uint32_t) pPixels->argb.Red * 75
								+ (uint32_t) pPixels->argb.Green * 149
								+ ((uint32_t) pPixels->argb.Blue << 5) ;
			nGray |= - (int32_t) (nGray >> 16) ;
			//
			uint8_t	nGray8 = (uint8_t) (nGray >> 8) ;
			pPixels->argb.Blue = nGray8 ;
			pPixels->argb.Green = nGray8 ;
			pPixels->argb.Red = nGray8 ;
			//
			++ pPixels ;
		}
		while ( -- nCount ) ;
		//
		pbytNextLine += pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif


// 画像の任意チャネルを転送
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglPutImageChannelTo
	( const SGLImageBuffer& imgDst, int iDstChannel,
		const SGLImageBuffer& imgSrc, int iSrcChannel,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (iSrcChannel >= imgSrc.pitchPixel) | (iDstChannel >= imgDst.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	pDstNextLine += iDstChannel ;
	pSrcNextLine += iSrcChannel ;
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint8_t *	pDstNextPixel = pDstNextLine ;
		uint8_t *	pSrcNextPixel = pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			*pDstNextPixel = *pSrcNextPixel ;
			pSrcNextPixel += infSrc.pitchPixel ;
			pDstNextPixel += infDst.pitchPixel ;
		}
		pSrcNextLine += infSrc.pitchLine ;
		pDstNextLine += infDst.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 指定 RGBA との積和を任意チャネルに設定
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglPutImageMAddChannelTo
	( const SGLImageBuffer& imgDst, int iDstChannel,
		const SGLImageBuffer& imgSrc, const SGLPalette& rgbaMul,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgSrc.depth < 32) | (iDstChannel >= imgDst.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	pDstNextLine += iDstChannel ;
	//
	const int	bMul = (int) rgbaMul.argb.Blue + 1 ;
	const int	gMul = (int) rgbaMul.argb.Green + 1 ;
	const int	rMul = (int) rgbaMul.argb.Red + 1 ;
	const int	aMul = (int) rgbaMul.argb.Alpha + 1 ;
	//
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint8_t *	pDstNextPixel = pDstNextLine ;
		uint8_t *	pSrcNextPixel = pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			int	v = (int) pSrcNextPixel[0] * bMul
					+ (int) pSrcNextPixel[1] * gMul
					+ (int) pSrcNextPixel[2] * rMul
					+ (int) pSrcNextPixel[3] * aMul ;
			//
			*pDstNextPixel = (uint8_t) esl_min( (v >> 8), 0xFF ) ;
			//
			pSrcNextPixel += infSrc.pitchPixel ;
			pDstNextPixel += infDst.pitchPixel ;
		}
		pSrcNextLine += infSrc.pitchLine ;
		pDstNextLine += infDst.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// トーンフィルタを適用する
//////////////////////////////////////////////////////////////////////////////
void SakuraGL::sglApplyToneFilter
	( SGLPalette* pPixels, size_t nCount,
		const uint8_t* pRedTone, const uint8_t* pGreenTone,
		const uint8_t* pBlueTone, const uint8_t* pAlphaTone )
{
	if ( nCount > 0 )
	do
	{
		pPixels->argb.Blue  = pBlueTone[pPixels->argb.Blue] ;
		pPixels->argb.Green = pGreenTone[pPixels->argb.Green] ;
		pPixels->argb.Red   = pRedTone[pPixels->argb.Red] ;
		pPixels->argb.Alpha = pAlphaTone[pPixels->argb.Alpha] ;
		++ pPixels ;
	}
	while ( -- nCount ) ;
}

#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglApplyToneImageFilter
	( const SGLImageBuffer& imgbuf,
		const SGLImageRect * pRect,
		const uint8_t * pRedTone, const uint8_t * pGreenTone,
		const uint8_t * pBlueTone, const uint8_t * pAlphaTone )
{
	const SGLImageBuffer *	pDst = &imgbuf ;
	SGLImageBuffer			imgTemp ;
	if ( pRect != NULL )
	{
		if ( !imgTemp.GetClippedBuffer( imgbuf, *pRect ) )
		{
			return	sglErrFailed ;
		}
		pDst = &imgTemp ;
	}
	if ( (pDst->depth != 32) || (pDst->pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint8_t	toneStr[0x100] ;
	if ( pRedTone == NULL )
	{
		sglMakeBrightnessToneFilter( &toneStr[0], 0 ) ;
		pRedTone = &toneStr[0] ;
	}
	if ( pGreenTone == NULL )
	{
		sglMakeBrightnessToneFilter( &toneStr[0], 0 ) ;
		pGreenTone = &toneStr[0] ;
	}
	if ( pBlueTone == NULL )
	{
		sglMakeBrightnessToneFilter( &toneStr[0], 0 ) ;
		pBlueTone = &toneStr[0] ;
	}
	if ( pAlphaTone == NULL )
	{
		sglMakeBrightnessToneFilter( &toneStr[0], 0 ) ;
		pAlphaTone = &toneStr[0] ;
	}
	uint32_t	width = pDst->width ;
	uint32_t	height = pDst->height ;
	int32_t		pitchLine = pDst->pitchLine ;
	uint8_t *	pbytNextLine = pDst->ptrBuffer ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		sglApplyToneFilter
			( (SGLPalette*) pbytNextLine, (size_t) width,
				pRedTone, pGreenTone, pBlueTone, pAlphaTone ) ;
		pbytNextLine += pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// トーンテーブルを生成する
//////////////////////////////////////////////////////////////////////////////
void SakuraGL::sglMakeToneFilter( uint8_t* pTone, int nValue, int nType )
{
	switch ( nType )
	{
	case	toneBrightness:
	default:
		sglMakeBrightnessToneFilter( pTone, nValue ) ;
		break ;
	case	toneMultiple:
		sglMakeMultipleToneFilter( pTone, nValue ) ;
		break ;
	case	toneAdditional:
		sglMakeAdditionalToneFilter( pTone, nValue ) ;
		break ;
	case	toneOffsetMultiple:
		sglMakeOffsetMultipleToneFilter( pTone, nValue ) ;
		break ;
	case	toneGamma:
		sglMakeGammaToneFilter( pTone, nValue ) ;
		break ;
	}
}

// 輝度 : nValue = [-256,256]
void SakuraGL::sglMakeBrightnessToneFilter( uint8_t* pTone, int nValue )
{
	if ( nValue > 0 )
	{
		nValue = 256 - nValue ;
		if ( nValue < 0 )
		{
			nValue = 0 ;
		}
		int	v = (255 - nValue) << 8 ;
		for ( int i = 0; i < 0x100; ++ i )
		{
			pTone[i] = (uint8_t) (v >> 8) ;
			v += nValue ;
		}
	}
	else
	{
		nValue += 256 ;
		if ( nValue < 0 )
		{
			nValue = 0 ;
		}
		int	v = 0 ;
		for ( int i = 0; i < 0x100; ++ i )
		{
			pTone[i] = (uint8_t) (v >> 8) ;
			v += nValue ;
		}
	}
}

// 積算
void SakuraGL::sglMakeMultipleToneFilter( uint8_t* pTone, int nValue )
{
	if ( nValue < 0 )
	{
		nValue = 0 ;
	}
	int	v = 0 ;
	for ( int i = 0; i < 0x100; ++ i )
	{
		pTone[i] = (uint8_t) (v >> 8) ;
		v += nValue ;
		if ( v > (255 << 8) )
		{
			v = (255 << 8) ;
		}
	}
}

// 加算
void SakuraGL::sglMakeAdditionalToneFilter( uint8_t* pTone, int nValue )
{
	int	v = nValue ;
	int	i = 0 ;
	while ( (v < 0) & (i < 0x100) )
	{
		pTone[i ++] = 0 ;
		++ v ;
	}
	while ( (v <= 255) & (i < 0x100) )
	{
		pTone[i ++] = (uint8_t) v ;
		++ v ;
	}
	while ( i < 0x100 )
	{
		pTone[i ++] = 0xFF ;
	}
}

// 積算（0x80をゼロとする）
void SakuraGL::sglMakeOffsetMultipleToneFilter( uint8_t* pTone, int nValue )
{
	for ( int i = 0; i < 0x100; ++ i )
	{
		int	v = (((i - 0x80) * nValue) >> 8) + 0x80 ;
		if ( v < 0 )
		{
			v = 0 ;
		}
		if ( v > 255 )
		{
			v = 255 ;
		}
		pTone[i] = (uint8_t) v ;
	}
}

// ガンマ補正
void SakuraGL::sglMakeGammaToneFilter( uint8_t* pTone, int nValue )
{
	double	s = 1.0 / 255.0 ;
	double	r = 1.0 / ((double) nValue / 256.0) ;
	pTone[0] = 0 ;
	for ( int i = 1; i < 0x100; ++ i )
	{
		pTone[i] =
			(uint8_t) eslRoundR32ToInt
						( (float) (pow( s * i, r ) * 255.0) ) ;
	}
}

// 50% 合成（単純処理）
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglHalfBlendImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4)
			| (imgDst.pitchPixel != imgSrc.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
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
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint32_t *	pSrcNextPixel = (uint32_t*) pSrcNextLine ;
		#if	defined(__COTOPHA__)
			asm
			{
				REG ALLOC	nWidth : uint32
				REG ALLOC	nWidthX4 : uint32
				REG ALLOC	mmx(10) : int64
				REG LOAD	pDstNextPixel
				REG LOAD	pSrcNextPixel
				//
				move	nWidth, infDst.width
				srl		nWidthX4, nWidth, 2
				move	acc, 3
				and		nWidth, acc
				//
				move	mmx(6), 0x0101010101010101
				move	mmx(7), mmx(6)
				move	mmx(8), 0x7F7F7F7F7F7F7F7F
				move	mmx(9), mmx(8)
				//
				prefetch.tlb0	pSrcNextPixel
				prefetch.tlb1	pSrcNextPixel
				//
				.WHILE	(uint32) nWidthX4 != (uint32) #zero
					load.64	mmx(0), [pSrcNextPixel]
					load.64	mmx(1), [pSrcNextPixel+8]
					load.64	mmx(2), [pDstNextPixel]
					load.64	mmx(3), [pDstNextPixel+8]
					move	mmx(4), mmx(0)
					move	mmx(5), mmx(1)
					psrld	mmx(0), 1
					psrld	mmx(1), 1
					and		mmx(4), mmx(2)
					and		mmx(5), mmx(3)
					psrld	mmx(2), 1
					psrld	mmx(3), 1
					and		mmx(0), mmx(8)
					and		mmx(1), mmx(9)
					and		mmx(2), mmx(8)
					and		mmx(3), mmx(9)
					and		mmx(4), mmx(6)
					and		mmx(5), mmx(7)
					paddd	mmx(0), mmx(2)
					paddd	mmx(1), mmx(3)
					paddd	mmx(0), mmx(4)
					paddd	mmx(1), mmx(5)
					store.64	[pDstNextPixel], mmx(0)
					store.64	[pDstNextPixel+8], mmx(0)
					//
					dec		nWidthX4
					add		pSrcNextPixel, 4 * 4
					add		pDstNextPixel, 4 * 4
				.ENDW
				//
				.WHILE	(uint32) nWidth != (uint32) #zero
					load.uint32	mmx(0), [pSrcNextPixel]
					load.uint32	mmx(2), [pDstNextPixel]
					move	mmx(4), mmx(0)
					psrld	mmx(0), 1
					and		mmx(4), mmx(2)
					psrld	mmx(2), 1
					and		mmx(0), mmx(8)
					and		mmx(2), mmx(8)
					and		mmx(4), mmx(6)
					paddd	mmx(0), mmx(2)
					paddd	mmx(0), mmx(4)
					store.uint32	[pDstNextPixel], mmx(0)
					//
					dec		nWidth
					add		pSrcNextPixel, 4
					add		pDstNextPixel, 4
				.ENDW
				//
				unfetch.tlb0	pSrcNextPixel
				unfetch.tlb1	pDstNextPixel
			}
		#else
			for ( uint32_t x = 0; x < infDst.width; x ++ )
			{
				uint32_t	argbDst = *pDstNextPixel ;
				uint32_t	argbSrc = *(pSrcNextPixel ++) ;
				*(pDstNextPixel ++) =
					((argbDst >> 1) & 0x7F7F7F7F)
						+ ((argbSrc >> 1) & 0x7F7F7F7F)
						+ ((argbDst & argbSrc) & 0x01010101) ;
			}
		#endif
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}

// αチャネル合成
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglBlendWithAlphaChannel
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgAlpha,
		int32_t fxAlphaCoefficient,
		int32_t fxAlphaIntercept,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4) || (imgAlpha.pitchPixel != 1) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infAlpha ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infAlpha, imgDst, imgAlpha, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infAlpha.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	if ( fxAlphaCoefficient >= 0x10000 )
	{
		fxAlphaCoefficient = 0xFFFF ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pdwDstPixel = (uint32_t*) pDstNextLine ;
		uint8_t *	pSrcPixel = pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; ++ x )
		{
			int32_t	alpha = *(pSrcPixel ++) ;
			alpha = alpha * fxAlphaCoefficient + fxAlphaIntercept ;
			if ( alpha <= 0 )
			{
				*pdwDstPixel = 0 ;
			}
			else if ( alpha < 0xFF )
			{
				*pdwDstPixel =
					sglPackedColorMul
						( *pdwDstPixel, (uint32_t) alpha + 1 ) ;
			}
			pdwDstPixel ++ ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infAlpha.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// ガウスぼかし
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglGaussianBlur
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		const SGLImageBuffer& imgTemp,
		float32_t fpGaussianValue, size_t nBlurWidth )
{
	//
	// 重み係数計算
	//
	if ( nBlurWidth > 126 )
	{
		nBlurWidth = 126 ;
	}
	uint32_t	nWeight[256] ;
	uint32_t *	pWeight = &nWeight[0] ;
	eslFillMemory
		( pWeight, 0, (nBlurWidth * 2 + 1) * sizeof(uint32_t) ) ;
	//
	uint32_t	nSum = 0 ;
	double		c = fpGaussianValue /*fpGaussianValue * fpGaussianValue*/ ;
	size_t		iValidWidth = nBlurWidth ;
	for ( size_t i = 0; i <= nBlurWidth; i ++ )
	{
		double		x = 1.0 + i * 2.0 ;
		double		g = exp( -0.5 * (x * x) / c ) ;
		uint32_t	w = eslRoundR32ToInt( (float32_t) (g * 0x100) ) ;
		if ( w <= 1 )
		{
			w = 1 ;
		}
		pWeight[nBlurWidth + i] = w ;
		nSum += w ;
		if ( i != 0 )
		{
			nSum += w ;
		}
	}
	for ( size_t i = 0; i <= nBlurWidth; i ++ )
	{
		uint32_t	w = (pWeight[nBlurWidth + i] * 0x100) / nSum ;
		pWeight[nBlurWidth - i] = w ;
		pWeight[nBlurWidth + i] = w ;
		if ( w == 0 )
		{
			if ( i > 0 )
			{
				iValidWidth = i - 1 ;
			}
			else
			{
				iValidWidth = i ;
			}
			break ;
		}
	}
	//
	// ブラー処理
	//
	SGLError	err ;
	err = sglGaussianBlurHorizontal
		( imgTemp, imgSrc,
			pWeight + (nBlurWidth - iValidWidth), iValidWidth ) ;
	if ( err )
	{
		return	err ;
	}
	err = sglGaussianBlurVertical
		( imgDst, imgTemp,
			pWeight + (nBlurWidth - iValidWidth), iValidWidth ) ;
	return	err ;
}

SGLError SakuraGL::sglGaussianBlurHorizontal
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		const uint32_t * pWeight, size_t nBlurWidth )
{
	if ( (imgDst.pitchPixel != 4)
		|| (imgSrc.pitchPixel != 4) )
	{
		return	sglErrNotSupported ;
	}
	if ( (imgDst.width != imgSrc.width)
		|| (imgDst.height!= imgSrc.height) )
	{
		return	sglErrInvalidParam ;
	}
	if ( SSystem::g_cpuLogicalCount >= 2 )
	{
		GaussianBlurHorizontalProc::Instance	instance[16] ;
		void *	pInstance[16] ;
		size_t	nThreadCount = SSystem::g_cpuLogicalCount ;
		if ( nThreadCount > 16 )
		{
			nThreadCount = 16 ;
		}
		for ( size_t i = 0; i < nThreadCount; i ++ )
		{
			pInstance[i] = &instance[i] ;
		}
		GaussianBlurHorizontalProc
			gbhp( imgDst, imgSrc, pWeight, nBlurWidth ) ;
		gbhp.Start( &pInstance[0], nThreadCount ) ;
	}
	else
	{
		int32_t		pitchSrcLine = imgSrc.pitchLine ;
		int32_t		pitchDstLine = imgDst.pitchLine ;
		uint8_t *	pbytSrcBuf = imgSrc.ptrBuffer ;
		uint8_t *	pbytDstBuf = imgDst.ptrBuffer ;
		uint32_t	nWidth = imgSrc.width ;
		uint32_t	nHeight = imgSrc.height ;
		size_t		iEndWeight = nBlurWidth * 2 + 1 ;
		for ( uint32_t x = 0; x < nWidth; x ++ )
		{
			uint8_t *	pbytSrcNext = pbytSrcBuf ;
			uint8_t *	pbytDstNext = pbytDstBuf ;
			size_t		iBlurFirst = 0 ;
			size_t		iBlurEnd = iEndWeight ;
			if ( x <= nBlurWidth )
			{
				iBlurFirst = nBlurWidth - x ;
			}
			if ( x + nBlurWidth + 1 >= nWidth )
			{
				iBlurEnd = nWidth - x + nBlurWidth ;
			}
			//
			for ( uint32_t y = 0; y < nHeight; y ++ )
			{
				uint32_t *	pdwSrcNext = (uint32_t*) pbytSrcNext ;
				uint32_t *	pdwDstNext = (uint32_t*) pbytDstNext ;
				//
				size_t		i = 0 ;
				uint32_t	pxSum = 0 ;
				pdwSrcNext -= nBlurWidth - iBlurFirst ;
				while ( i < iBlurFirst )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul( pdwSrcNext[0], pWeight[i] ) ) ;
					i ++ ;
				}
				while ( i < iBlurEnd )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul( *pdwSrcNext, pWeight[i] ) ) ;
					pdwSrcNext ++ ;
					i ++ ;
				}
				while ( i < iEndWeight )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul( pdwSrcNext[-1], pWeight[i] ) ) ;
					i ++ ;
				}
				*pdwDstNext = pxSum ;
				//
				pbytSrcNext += pitchSrcLine ;
				pbytDstNext += pitchDstLine ;
			}
			//
			pbytSrcBuf += 4 ;
			pbytDstBuf += 4 ;
		}
	}
	return	sglErrSuccess ;
}

// 構築関数
GaussianBlurHorizontalProc::GaussianBlurHorizontalProc
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		const uint32_t * pWeight, size_t nBlurWidth )
{
	m_pitchSrcLine = imgSrc.pitchLine ;
	m_pitchDstLine = imgDst.pitchLine ;
	m_pbytSrcBuf = imgSrc.ptrBuffer ;
	m_pbytDstBuf = imgDst.ptrBuffer ;
	m_nWidth = imgSrc.width ;
	m_nHeight = imgSrc.height ;
	m_pWeight = pWeight ;
	m_nBlurWidth = nBlurWidth ;
	m_iEndWeight = nBlurWidth * 2 + 1 ;
	m_xNext = 0 ;
}

// ループ処理／終了判定関数
bool GaussianBlurHorizontalProc::Continue( void * pInstance )
{
	if ( m_xNext >= m_nWidth )
	{
		return	false ;
	}
	Instance *	p = (Instance*) pInstance ;
	p->pbytSrcBuf = m_pbytSrcBuf ;
	p->pbytDstBuf = m_pbytDstBuf ;
	p->x = m_xNext ;
	m_pbytSrcBuf += 4 ;
	m_pbytDstBuf += 4 ;
	m_xNext ++ ;
	return	true ;
}

// 並列処理関数
void GaussianBlurHorizontalProc::RunParallel( void * pInstance )
{
	Instance *			p = (Instance*) pInstance ;
	uint8_t *			pbytSrcNext = p->pbytSrcBuf ;
	uint8_t *			pbytDstNext = p->pbytDstBuf ;
	const uint32_t *	pWeight = m_pWeight ;
	size_t				nBlurWidth = m_nBlurWidth ;
	size_t				iEndWeight = m_iEndWeight ;
	size_t				iBlurFirst = 0 ;
	size_t				iBlurEnd = m_iEndWeight ;
	if ( p->x <= m_nBlurWidth )
	{
		iBlurFirst = m_nBlurWidth - p->x ;
	}
	if ( p->x + m_nBlurWidth + 1 >= m_nWidth )
	{
		iBlurEnd = m_nWidth - p->x + m_nBlurWidth ;
	}
	uint32_t	nHeight = m_nHeight ;
	for ( uint32_t y = 0; y < nHeight; y ++ )
	{
		uint32_t *	pdwSrcNext = (uint32_t*) pbytSrcNext ;
		uint32_t *	pdwDstNext = (uint32_t*) pbytDstNext ;
		//
		size_t		i = 0 ;
		uint32_t	pxSum = 0 ;
		pdwSrcNext -= nBlurWidth - iBlurFirst ;
		while ( i < iBlurFirst )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul( pdwSrcNext[0], pWeight[i] ) ) ;
			i ++ ;
		}
		while ( i < iBlurEnd )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul( *pdwSrcNext, pWeight[i] ) ) ;
			pdwSrcNext ++ ;
			i ++ ;
		}
		while ( i < iEndWeight )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul( pdwSrcNext[-1], pWeight[i] ) ) ;
			i ++ ;
		}
		*pdwDstNext = pxSum ;
		//
		pbytSrcNext += m_pitchSrcLine ;
		pbytDstNext += m_pitchDstLine ;
	}
}

SGLError SakuraGL::sglGaussianBlurVertical
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		const uint32_t * pWeight, size_t nBlurWidth )
{
	if ( (imgDst.pitchPixel != 4)
		|| (imgSrc.pitchPixel != 4) )
	{
		return	sglErrNotSupported ;
	}
	if ( (imgDst.width != imgSrc.width)
		|| (imgDst.height!= imgSrc.height) )
	{
		return	sglErrInvalidParam ;
	}
	if ( SSystem::g_cpuLogicalCount >= 2 )
	{
		GaussianBlurVerticalProc::Instance	instance[16] ;
		void *	pInstance[16] ;
		size_t	nThreadCount = SSystem::g_cpuLogicalCount ;
		if ( nThreadCount > 16 )
		{
			nThreadCount = 16 ;
		}
		for ( size_t i = 0; i < nThreadCount; i ++ )
		{
			pInstance[i] = &instance[i] ;
		}
		GaussianBlurVerticalProc
			gbvp( imgDst, imgSrc, pWeight, nBlurWidth ) ;
		gbvp.Start( &pInstance[0], nThreadCount ) ;
	}
	else
	{
		int32_t		pitchSrcLine = imgSrc.pitchLine ;
		int32_t		pitchDstLine = imgDst.pitchLine ;
		uint8_t *	pbytSrcBuf = imgSrc.ptrBuffer ;
		uint8_t *	pbytDstBuf = imgDst.ptrBuffer ;
		uint32_t	nWidth = imgSrc.width ;
		uint32_t	nHeight = imgSrc.height ;
		size_t		iEndWeight = nBlurWidth * 2 + 1 ;
		for ( uint32_t y = 0; y < nHeight; y ++ )
		{
			uint8_t *	pbytSrcNext = pbytSrcBuf ;
			uint8_t *	pbytDstNext = pbytDstBuf ;
			size_t		iBlurFirst = 0 ;
			size_t		iBlurEnd = iEndWeight ;
			if ( y <= nBlurWidth )
			{
				iBlurFirst = nBlurWidth - y ;
			}
			if ( y + nBlurWidth + 1 >= nHeight )
			{
				iBlurEnd = nHeight - y + nBlurWidth ;
			}
			//
			for ( uint32_t x = 0; x < nWidth; x ++ )
			{
				uint8_t *	pbytSrc = pbytSrcNext ;
				uint8_t *	pbytDst = pbytDstNext ;
				//
				size_t		i = 0 ;
				uint32_t	pxSum = 0 ;
				pbytSrc -= (nBlurWidth - iBlurFirst) * pitchSrcLine ;
				while ( i < iBlurFirst )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul
								( *((uint32_t*)pbytSrc), pWeight[i] ) ) ;
					i ++ ;
				}
				while ( i < iBlurEnd )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul
								( *((uint32_t*)pbytSrc), pWeight[i] ) ) ;
					pbytSrc += pitchSrcLine ;
					i ++ ;
				}
				while ( i < iEndWeight )
				{
					pxSum = sglPackedColorAdd
						( pxSum, sglPackedColorMul
							( *((uint32_t*)(pbytSrc-pitchSrcLine)), pWeight[i] ) ) ;
					i ++ ;
				}
				*((uint32_t*)pbytDst) = pxSum ;
				//
				pbytSrcNext += 4 ;
				pbytDstNext += 4 ;
			}
			//
			pbytSrcBuf += pitchSrcLine ;
			pbytDstBuf += pitchDstLine ;
		}
	}
	return	sglErrSuccess ;
}

// 構築関数
GaussianBlurVerticalProc::GaussianBlurVerticalProc
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		const uint32_t * pWeight, size_t nBlurWidth )
{
	m_pitchSrcLine = imgSrc.pitchLine ;
	m_pitchDstLine = imgDst.pitchLine ;
	m_pbytSrcBuf = imgSrc.ptrBuffer ;
	m_pbytDstBuf = imgDst.ptrBuffer ;
	m_nWidth = imgSrc.width ;
	m_nHeight = imgSrc.height ;
	m_pWeight = pWeight ;
	m_nBlurWidth = nBlurWidth ;
	m_iEndWeight = nBlurWidth * 2 + 1 ;
	m_yNext = 0 ;
}

// ループ処理／終了判定関数
bool GaussianBlurVerticalProc::Continue( void * pInstance )
{
	if ( m_yNext >= m_nHeight )
	{
		return	false ;
	}
	Instance *	p = (Instance*) pInstance ;
	p->pbytSrcBuf = m_pbytSrcBuf ;
	p->pbytDstBuf = m_pbytDstBuf ;
	p->y = m_yNext ;
	m_pbytSrcBuf += m_pitchSrcLine ;
	m_pbytDstBuf += m_pitchDstLine ;
	m_yNext ++ ;
	return	true ;
}

// 並列処理関数
void GaussianBlurVerticalProc::RunParallel( void * pInstance )
{
	Instance *			p = (Instance*) pInstance ;
	uint8_t *			pbytSrcNext = p->pbytSrcBuf ;
	uint8_t *			pbytDstNext = p->pbytDstBuf ;
	const uint32_t *	pWeight = m_pWeight ;
	size_t				nBlurWidth = m_nBlurWidth ;
	size_t				iEndWeight = m_iEndWeight ;
	size_t				iBlurFirst = 0 ;
	size_t				iBlurEnd = m_iEndWeight ;
	if ( p->y <= nBlurWidth )
	{
		iBlurFirst = nBlurWidth - p->y ;
	}
	if ( p->y + nBlurWidth + 1 >= m_nHeight )
	{
		iBlurEnd = m_nHeight - p->y + nBlurWidth ;
	}
	//
	int32_t		pitchSrcLine = m_pitchSrcLine ;
	uint32_t	nWidth = m_nWidth ;
	for ( uint32_t x = 0; x < nWidth; x ++ )
	{
		uint8_t *	pbytSrc = pbytSrcNext ;
		uint8_t *	pbytDst = pbytDstNext ;
		//
		size_t		i = 0 ;
		uint32_t	pxSum = 0 ;
		pbytSrc -= (nBlurWidth - iBlurFirst) * pitchSrcLine ;
		while ( i < iBlurFirst )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul
						( *((uint32_t*)pbytSrc), pWeight[i] ) ) ;
			i ++ ;
		}
		while ( i < iBlurEnd )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul
						( *((uint32_t*)pbytSrc), pWeight[i] ) ) ;
			pbytSrc += pitchSrcLine ;
			i ++ ;
		}
		while ( i < iEndWeight )
		{
			pxSum = sglPackedColorAdd
				( pxSum, sglPackedColorMul
					( *((uint32_t*)(pbytSrc-pitchSrcLine)), pWeight[i] ) ) ;
			i ++ ;
		}
		*((uint32_t*)pbytDst) = pxSum ;
		//
		pbytSrcNext += 4 ;
		pbytDstNext += 4 ;
	}
}

#endif
