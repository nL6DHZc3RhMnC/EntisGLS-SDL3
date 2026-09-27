
#if	!defined(__SAKURAGL_IMAGE_FILTER_H__)
#define	__SAKURAGL_IMAGE_FILTER_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像フィルタ
	//////////////////////////////////////////////////////////////////////////

	// αチャネルで RGB を積算
	void sglMultiplyRGBAlpha( SGLPalette* pPixels, size_t nCount ) ;
	__native SGLError sglMultiplyImageRGBAlpha( const SGLImageBuffer& imgbuf ) ;

	// RGB チャネルを Gray 化
	__native SGLError sglMakeGrayImageFromRGB( const SGLImageBuffer& imgbuf ) ;

	// 透明部分に色を流し込む（背景色に描画した結果を生成）
	void sglBlendBackgroundColor
		( SGLPalette* pPixels, size_t nCount, const SGLPalette argbBackColor ) ;
	__native SGLError sglBlendImageBackgroundColor
		( const SGLImageBuffer& imgbuf, const SGLPalette argbBackColor ) ;

	// 画像の任意チャネルを転送
	__native SGLError sglPutImageChannelTo
		( const SGLImageBuffer& imgDst, int iDstChannel,
			const SGLImageBuffer& imgSrc, int iSrcChannel,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// 指定 RGBA との積和を任意チャネルに設定
	__native SGLError sglPutImageMAddChannelTo
		( const SGLImageBuffer& imgDst, int iDstChannel,
			const SGLImageBuffer& imgSrc, const SGLPalette& rgbaMul,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// トーンフィルタを適用する
	void sglApplyToneFilter
		( SGLPalette* pPixels, size_t nCount,
			const uint8_t* pRedTone, const uint8_t* pGreenTone,
			const uint8_t* pBlueTone, const uint8_t* pAlphaTone ) ;
	__native SGLError sglApplyToneImageFilter
		( const SGLImageBuffer& imgbuf,
			const SGLImageRect * pRect,
			const uint8_t * pRedTone, const uint8_t * pGreenTone,
			const uint8_t * pBlueTone, const uint8_t * pAlphaTone ) ;

	// トーンテーブルを生成する
	enum	ToneTilterType
	{
		toneBrightness,
		toneMultiple,
		toneAdditional,
		toneOffsetMultiple,
		toneGamma,
	} ;
	void sglMakeToneFilter( uint8_t* pTone, int nValue, int nType ) ;
	void sglMakeBrightnessToneFilter( uint8_t* pTone, int nValue ) ;
	void sglMakeMultipleToneFilter( uint8_t* pTone, int nValue ) ;
	void sglMakeAdditionalToneFilter( uint8_t* pTone, int nValue ) ;
	void sglMakeOffsetMultipleToneFilter( uint8_t* pTone, int nValue ) ;
	void sglMakeGammaToneFilter( uint8_t* pTone, int nValue ) ;

	// 50% 合成（単純処理）
	SGLError sglHalfBlendImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// αチャネル合成
	__native SGLError sglBlendWithAlphaChannel
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgAlpha,
			int32_t fxAlphaCoefficient = 0x100,
			int32_t fxAlphaIntercept = 0,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// ガウスぼかし
	__native SGLError sglGaussianBlur
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			const SGLImageBuffer& imgTemp,
			float32_t fpGaussianValue /*1.0～*/, size_t nBlurWidth ) ;
	#if	!defined(__COTOPHA__)
	SGLError sglGaussianBlurHorizontal
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			const uint32_t * pWeight, size_t nBlurWidth ) ;
	SGLError sglGaussianBlurVertical
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			const uint32_t * pWeight, size_t nBlurWidth ) ;

	class	GaussianBlurHorizontalProc : public SSystem::SParallelProcedure
	{
	public:
		struct	Instance
		{
			uint8_t *	pbytSrcBuf ;
			uint8_t *	pbytDstBuf ;
			size_t		x ;
		} ;

	protected:
		int32_t				m_pitchSrcLine ;
		int32_t				m_pitchDstLine ;
		uint8_t *			m_pbytSrcBuf ;
		uint8_t *			m_pbytDstBuf ;
		uint32_t			m_nWidth ;
		uint32_t			m_nHeight ;
		const uint32_t *	m_pWeight ;
		size_t				m_nBlurWidth ;
		size_t				m_iEndWeight ;
		size_t				m_xNext ;

	public:
		// 構築関数
		GaussianBlurHorizontalProc
			( const SGLImageBuffer& imgDst,
				const SGLImageBuffer& imgSrc,
				const uint32_t * pWeight, size_t nBlurWidth ) ;
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;

	class	GaussianBlurVerticalProc : public SSystem::SParallelProcedure
	{
	public:
		struct	Instance
		{
			uint8_t *	pbytSrcBuf ;
			uint8_t *	pbytDstBuf ;
			size_t		y ;
		} ;

	protected:
		int32_t				m_pitchSrcLine ;
		int32_t				m_pitchDstLine ;
		uint8_t *			m_pbytSrcBuf ;
		uint8_t *			m_pbytDstBuf ;
		uint32_t			m_nWidth ;
		uint32_t			m_nHeight ;
		const uint32_t *	m_pWeight ;
		size_t				m_nBlurWidth ;
		size_t				m_iEndWeight ;
		size_t				m_yNext ;

	public:
		// 構築関数
		GaussianBlurVerticalProc
			( const SGLImageBuffer& imgDst,
				const SGLImageBuffer& imgSrc,
				const uint32_t * pWeight, size_t nBlurWidth ) ;
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;
	#endif

}

#endif
