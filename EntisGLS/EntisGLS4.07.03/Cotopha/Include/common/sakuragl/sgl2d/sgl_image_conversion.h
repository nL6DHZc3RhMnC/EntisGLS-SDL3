
#if	!defined(__SAKURAGL_IMAGE_CONVERSION_H__)
#define	__SAKURAGL_IMAGE_CONVERSION_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像フォーマット変換関数群
	//////////////////////////////////////////////////////////////////////////

	// 画像バッファ変換複製
	__native SGLError sglConvertImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// RGB⇔BGR 変換
	SGLError sglFlipCompositionRGBtoBGR( SGLImageBuffer& imgBuf ) ;

	// 色空間変換関数
	typedef	void (__fastcall *PROC_CONVERT_COLOR_FORMAT)
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;

	// ピクセルコンポジション変換関数
	typedef	void (__fastcall *PROC_DECODE_PIXEL_COMPOSITION)
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	typedef	void (__fastcall *PROC_ENCODE_PIXEL_COMPOSITION)
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;

	// 色空間変換関数取得
	PROC_CONVERT_COLOR_FORMAT
		sglGetColorFormatConvertor( uint32_t fmtDst, uint32_t fmtSrc ) ;

	// ピクセルコンポジション変換関数取得
	PROC_DECODE_PIXEL_COMPOSITION
		sglGetPixelCompositionDecoder( uint32_t fmt, uint32_t depth ) ;
	PROC_ENCODE_PIXEL_COMPOSITION
		sglGetPixelCompositionEncoder( uint32_t fmt, uint32_t depth ) ;

	// パレット展開関数
	void __fastcall sglDecodePixelColorIndexed8
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;

	// 色空間変換関数
	void __fastcall sglConvertFormatRGBtoBGR
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatRGBtoGray
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatGraytoRGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatRGBtoYUV
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatYUVtoRGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatRGBtoHSB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatHSBtoRGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;

	// 特殊変換
	void __fastcall sglConvertFormatRGBtoARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatARGBtoNPARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatNPARGBtoARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatBGRtoARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatYUVtoARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglConvertFormatHSBtoARGB
		( SGLPalette* pDst, const SGLPalette* pSrc, size_t nCount ) ;

	// ピクセルコンポジション変換関数
	void __fastcall sglDecodePixelCompositionARGB32
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglEncodePixelCompositionARGB32
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionRGB24
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglEncodePixelCompositionRGB24
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionRGB565
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglEncodePixelCompositionRGB565
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionRGBA4444
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglEncodePixelCompositionRGBA4444
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionGray
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglEncodePixelCompositionGray
		( uint8_t* pDst, const SGLPalette* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionYUV2
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionYVYU
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionUYVY
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;
	void __fastcall sglDecodePixelCompositionYUYV
		( SGLPalette* pDst, const uint8_t* pSrc, size_t nCount ) ;

	// S3 Texture Compression DXT1
	enum	S3TC_DitheringMethod
	{
		s3tcNoDithering,
		s3tcErrorDiffusion,			// 誤差拡散
		s3tcErrorDiffusionByLine,	// 誤差拡散（ライン毎）
		s3tcErrorDiffusionByLine2,	// 誤差拡散（ライン毎に1/2）
	} ;
	SGLError sglCompressARGBtoS3TC_DXT1
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			S3TC_DitheringMethod dithering ) ;

	struct	S3TC_DXT1_Block
	{
		uint16_t	rgb16[2] ;	// Hi<- R5:G6:B5 ->Low
		uint8_t		code[4] ;	// 2bit index array [16]
	} ;
	struct	S3TC_SourceBlock
	{
		SGLPalette	px[16] ;
	} ;
	struct	S3TC_SourceParam	: public S3TC_SourceBlock
	{
		bool					alphaBlack ;	// Black は透明
		S3TC_DitheringMethod	dithering ;		// ディザリング
	} ;
	void __fastcall sglCompressS3TC_DXT1Block
		( S3TC_DXT1_Block& dxt1, const S3TC_SourceParam& src ) ;
	uint16_t __fastcall sglRGB24toRGB565( SGLPalette argb32 ) ;
	SGLPalette __fastcall sglRGB565toARGB32( uint16_t rgb565 ) ;

}

#endif
