
#if	!defined(__SAKURAGL_SGL2D_STDDEF_H__)
#define	__SAKURAGL_SGL2D_STDDEF_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 型宣言
	//////////////////////////////////////////////////////////////////////////

	enum	SGLError
	{
		sglErrSuccess			= SSystem::errSuccess,
		sglErrNotSupported		= SSystem::errNotSupported,
		sglErrFailed			= SSystem::errFailed,
		sglErrAbort				= SSystem::errAbort,
		sglErrInvalidParam		= SSystem::errInvalidParam,
		sglErrTimeout			= SSystem::errTimeout,
		sglErrPending			= SSystem::errPending,
		sglErrContinue			= SSystem::errContinue,
	} ;

	union	SGLPalette ;
	struct	SGLPoint ;
	struct	SGLSize ;
	struct	SGLRect ;
	struct	SGLImageRect ;
	struct	SGLAffine ;

	struct	S2DVector ;
	struct	S2DDVector ;
	struct	S3DVector ;
	struct	S3DDVector ;
	struct	S3DVector4 ;
	struct	S3DWVector4 ;
	struct	S3DMatrix ;
	struct	S3DDMatrix ;
	struct	S3DQuaternion ;
	struct	S3DDQuaternion ;


	//////////////////////////////////////////////////////////////////////////
	// packed 4 bytes operations
	//////////////////////////////////////////////////////////////////////////

	inline uint32_t sglPackedColorMul( uint32_t argbPixel, uint32_t x )
	{
	#if	defined(__COTOPHA__)
		asm
		{
			REG LOAD	x
			REG LOAD	argbPixel
			pshuf.w		x, x, 0
			punpack.lbw	argbPixel, #zero
			pmul.lw		argbPixel, x
			psrl.w		argbPixel, argbPixel, 8
			pcvt.uswb	argbPixel, #zero
			REG FLUSH	argbPixel
		}
		return	argbPixel ;
	#else
		uint32_t	aria = (argbPixel & 0x00FF00FF) * x ;			// 0 <= x <= 0x100
		uint32_t	maria = ((argbPixel >> 8) & 0x00FF00FF) * x ;
		return	((aria & 0xFF00FF00) >> 8) | (maria & 0xFF00FF00) ;
	#endif
	}

	inline uint32_t sglPackedColorAdd( uint32_t argbPixel1, uint32_t argbPixel2 )
	{
	#if	defined(__COTOPHA__)
		asm
		{
			REG LOAD	argbPixel1
			REG LOAD	argbPixel2
			padd.ub		argbPixel1, argbPixel2
			REG FLUSH	argbPixel1
		}
		return	argbPixel1 ;
	#elif	defined(__GNUC__) && defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		uint32_t	argbResult ;
		__asm__(
			"uqadd8	%[argbResult], %[spixel0], %[spixel1]\n"
			: [argbResult] "=r" (argbResult)
			: [spixel0] "r" (argbPixel1),
				[spixel1] "r" (argbPixel2)
			: ) ;
		return	argbResult ;
	#else
		uint32_t	aria = (argbPixel1 & 0x00FF00FF)
							+ (argbPixel2 & 0x00FF00FF) ;
		uint32_t	maria = ((argbPixel1 >> 8) & 0x00FF00FF)
							+ ((argbPixel2 >> 8) & 0x00FF00FF) ;
		uint32_t	arisa = (((0x00FF00FF - aria) & 0xFF00FF00) >> 8)
							| ((0x00FF00FF - maria) & 0xFF00FF00) ;
		return	(aria & 0x00FF00FF) | ((maria & 0x00FF00FF) << 8) | arisa ;
	#endif
	}

	inline uint32_t sglPackedColorBlend( uint32_t argbDstPixel, uint32_t argbSrcPixel )
	{
	#if	defined(__COTOPHA__)
		asm
		{
			REG LOAD	argbDstPixel
			REG LOAD	argbSrcPixel
			REG ALLOC	dalpha : uint32
			REG ALLOC	argbDst : uint32
			REG ALLOC	rgbMask : uint32
			psrl.d		dalpha, argbSrcPixel, 24
			xor			dalpha, #ff
			move		rgbMask, 0x00FFFFFF
			padd.d		dalpha, #one
			move		argbDst, 0xFF000000
			and			argbSrcPixel, rgbMask
			xor			argbDst, argbDstPixel
			pshuf.w		dalpha, dalpha, 0
			punpack.lbw	argbDst, #zero
			pmul.lw		argbDst, dalpha
			move		argbDstPixel, 0xFF000000
			psrl.w		argbDst, argbDst, 8
			pcvt.uswb	argbDst, #zero
			padd.ub		argbDst, argbSrcPixel
			xor			argbDstPixel, argbDst
			REG FLUSH	argbDstPixel
		}
		return	argbDstPixel ;
	#elif	defined(__GNUC__) && defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7) && !defined(__PROCESSOR_ARM64__)
		uint32_t	argbResult ;
		__asm__(
			"lsr	r4, %[spixel], #24\n"
			"mov	r5, %[spixel]\n"
			"eor	r4, r4, #0xff\n"
			"ldr	r10, =0xFF000000\n"
			"ldr	r6, =0x00FFFFFF\n"
			"add	r4, r4, #1\n"
			"eor	r10, %[dpixel], r10\n"
			"ldr	r8, =0x00FF00FF\n"
			"and	r5, r5, r6\n"
			"and	r2, r10, r8\n"
			"and	r3, r8, r10, lsr #8\n"
			"mul	r2, r2, r4\n"
			"mul	r3, r3, r4\n"
			"ldr	r10, =0xFF000000\n"
			"and	r2, r8, r2, lsr #8\n"
			"and	r3, r3, r8, lsl #8\n"
			"orr	r2, r2, r3\n"
			"eor	r2, r2, r10\n"
			"uqadd8	%[argbResult], r2, r5\n"
			: [argbResult] "=r" (argbResult)
			: [dpixel] "r" (argbDstPixel),
				[spixel] "r" (argbSrcPixel)
			: "r2", "r3", "r4", "r5", "r6", "r10", "r8" ) ;
		return	argbResult ;
	#else
		uint32_t	dalpha = ((argbSrcPixel >> 24) ^ 0xFF) + 1 ;
		uint32_t	argbDst = argbDstPixel ^ 0xFF000000 ;
		uint32_t	rb = (((uint32_t)((argbDst & 0x00FF00FF)
								* dalpha) >> 8) & 0x00FF00FF)
										+ (argbSrcPixel & 0x00FF00FF) ;
		uint32_t	ag = ((((argbDst >> 8) & 0x00FF00FF)
							* dalpha) & 0xFF00FF00)
										+ (argbSrcPixel & 0x0000FF00) ;
		uint32_t	ff = ((uint32_t)(((0x00FF00FF - rb) & 0xFF00FF00)
							| ((0x0000FF00 - ag) & 0x00FF0000)) >> 8) ;
		//
		return	((ag & 0xFF00FF00) ^ 0xFF000000) | (rb & 0x00FF00FF) | ff ;
	#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// パレット／カラー・コンポジション
	//////////////////////////////////////////////////////////////////////////

	struct	SGLPaletteARGB
	{
		uint8_t	Blue ;
		uint8_t	Green ;
		uint8_t	Red ;
		uint8_t	Alpha ;
	} ;
	struct	SGLPaletteABGR
	{
		uint8_t	Red ;
		uint8_t	Green ;
		uint8_t	Blue ;
		uint8_t	Alpha ;
	} ;
	struct	SGLPaletteYUV
	{
		uint8_t	Y ;
		uint8_t	U ;
		uint8_t	V ;
		uint8_t	Alpha ;
	} ;
	struct	SGLPaletteHSB
	{
		uint8_t	Brightness ;
		uint8_t	Saturation ;
		uint8_t	Hue ;
		uint8_t	Alpha ;
	} ;
	struct	SGLPaletteB4
	{
		uint8_t	ui8[4] ;
	} ;
	union	SGLPalette
	{
		uint32_t		ui32 ;
		float32_t		z32 ;
		SGLPaletteARGB	argb ;
		SGLPaletteABGR	abgr ;
		SGLPaletteYUV	yuv ;
		SGLPaletteHSB	hsb ;
		SGLPaletteB4	b4 ;

		#if	!defined(__COTOPHA__)
		SGLPalette( void ) { ui32 = 0 ; }
		#endif
		SGLPalette( const SGLPalette & pal )
		{
			ui32 = pal.ui32 ;
		}
		SGLPalette( uint32_t uiRGB )
		{
			ui32 = uiRGB ;
		}
		SGLPalette( int nBlue, int nGreen, int nRed, int nAlpha = 0 )
		{
			argb.Blue = (uint8_t) nBlue ;
			argb.Green = (uint8_t) nGreen ;
			argb.Red = (uint8_t) nRed ;
			argb.Alpha = (uint8_t) nAlpha ;
		}
		const SGLPalette & operator = ( const SGLPalette & pal )
		{
			ui32 = pal.ui32 ;
			return	*this ;
		}
		const SGLPalette & operator = ( uint32_t argb )
		{
			ui32 = argb ;
			return	*this ;
		}
		operator uint32_t ( void ) const
		{
			return	ui32 ;
		}
		const SGLPalette & operator += ( const SGLPalette & rgb )
		{
			ui32 = sglPackedColorAdd( ui32, rgb.ui32 ) ;
			return	*this ;
		}
		SGLPalette add( uint32_t rgb ) const
		{
			return	SGLPalette( sglPackedColorAdd( ui32, rgb ) ) ;
		}
		SGLPalette imul( uint32_t x ) const
		{
			ESLAssert( x <= 0x100 ) ;
			return	SGLPalette( sglPackedColorMul( ui32, x ) ) ;
		}
		const SGLPalette & operator *= ( unsigned int x )
		{
			ESLAssert( x <= 0x100 ) ;
			ui32 = sglPackedColorMul( ui32, x ) ;
			return	*this ;
		}
		const SGLPalette & operator *= ( double y )
		{
			uint32_t	x = (uint32_t) (y * 256.0) ;
			ESLAssert( x <= 0x100 ) ;
			ui32 = sglPackedColorMul( ui32, x ) ;
			return	*this ;
		}
		const SGLPalette & operator *= ( const SGLPalette & rgb )
		{
			argb.Blue = (uint8_t) ((argb.Blue * ((int) rgb.argb.Blue + 1)) >> 8) ;
			argb.Green = (uint8_t) ((argb.Green * ((int) rgb.argb.Green + 1)) >> 8) ;
			argb.Red = (uint8_t) ((argb.Red * ((int) rgb.argb.Red + 1)) >> 8) ;
			argb.Alpha = (uint8_t) ((argb.Alpha * ((int) rgb.argb.Alpha + 1)) >> 8) ;
			return	*this ;
		}
		SGLPalette operator + ( const SGLPalette & rgb ) const
		{
			SGLPalette	t = *this ;
			t += rgb ;
			return	t ;
		}
		SGLPalette operator * ( unsigned int x ) const
		{
			SGLPalette	t = *this ;
			t *= x ;
			return	t ;
		}
		SGLPalette operator * ( double x ) const
		{
			SGLPalette	t = *this ;
			t *= x ;
			return	t ;
		}
		SGLPalette operator * ( const SGLPalette & rgb ) const
		{
			SGLPalette	t = *this ;
			t *= rgb ;
			return	t ;
		}
	} ;

	struct	S3DColor
	{
		SGLPalette	rgbMul ;
		SGLPalette	rgbAdd ;

		S3DColor( void )
			: rgbMul( 0x00FFFFFF ), rgbAdd( 0 ) {}
		S3DColor( const S3DColor& clrSrc )
			: rgbMul( clrSrc.rgbMul ), rgbAdd( clrSrc.rgbAdd ) {}
		S3DColor( uint32_t srcMul, uint32_t srcAdd )
			: rgbMul( srcMul ), rgbAdd( srcAdd ) {}
		const S3DColor & operator = ( const S3DColor& clrSrc )
		{
			rgbMul = clrSrc.rgbMul ;
			rgbAdd = clrSrc.rgbAdd ;
			return	*this ;
		}
		SGLPalette operator * ( const SGLPalette& rgbSrc ) const
		{
			uint32_t	b = (((rgbMul.argb.Blue + 1)
								* rgbSrc.argb.Blue) >> 8) + rgbAdd.argb.Blue ;
			uint32_t	g = (((rgbMul.argb.Green + 1)
								* rgbSrc.argb.Green) >> 8) + rgbAdd.argb.Green ;
			uint32_t	r = (((rgbMul.argb.Red + 1)
								* rgbSrc.argb.Red) >> 8) + rgbAdd.argb.Red ;
			uint32_t	a = (((rgbMul.argb.Alpha + 1) * rgbSrc.argb.Alpha) >> 8) ;
			b |= - (int32_t) (b >> 8) ;
			g |= - (int32_t) (g >> 8) ;
			r |= - (int32_t) (r >> 8) ;
			return	SGLPalette( b, g, r, a ) ;
		}
		const S3DColor& operator *= ( const S3DColor& rgbSrc )
		{
			rgbAdd = *this * rgbSrc.rgbAdd ;
			rgbMul.argb.Blue =
				(uint8_t) (((rgbMul.argb.Blue + 1) * rgbSrc.rgbMul.argb.Blue) >> 8) ;
			rgbMul.argb.Green =
				(uint8_t) (((rgbMul.argb.Green + 1) * rgbSrc.rgbMul.argb.Green) >> 8) ;
			rgbMul.argb.Red =
				(uint8_t) (((rgbMul.argb.Red + 1) * rgbSrc.rgbMul.argb.Red) >> 8) ;
			rgbMul.argb.Alpha =
				(uint8_t) (((rgbMul.argb.Alpha + 1) * rgbSrc.rgbMul.argb.Alpha) >> 8) ;
			return	*this ;
		}
		const S3DColor& operator *= ( unsigned int x )
		{
			rgbMul = rgbMul.imul( x ) ;
			rgbAdd = rgbAdd.imul( x ) ;
			return	*this ;
		}
		S3DColor imul( uint32_t x ) const
		{
			ESLAssert( x <= 0x100 ) ;
			return	S3DColor( sglPackedColorMul( rgbMul, x ),
								sglPackedColorMul( rgbAdd, x ) ) ;
		}
		S3DColor operator * ( const S3DColor& rgbSrc ) const
		{
			S3DColor	clrDst = *this ;
			clrDst *= rgbSrc ;
			return	clrDst ;
		}
		S3DColor operator * ( unsigned int x ) const
		{
			S3DColor	clrDst = *this ;
			clrDst.rgbMul = clrDst.rgbMul.imul( x ) ;
			clrDst.rgbAdd = clrDst.rgbAdd.imul( x ) ;
			return	clrDst ;
		}
		S3DColor operator * ( double x ) const
		{
			S3DColor	clrDst = *this ;
			clrDst.rgbMul *= x ;
			clrDst.rgbAdd *= x ;
			return	clrDst ;
		}
		const S3DColor& operator += ( const S3DColor& rgbSrc )
		{
			rgbMul += rgbSrc.rgbMul ;
			rgbAdd += rgbSrc.rgbAdd ;
			return	*this ;
		}
		S3DColor operator + ( const S3DColor& rgbSrc ) const
		{
			S3DColor	clrDst = *this ;
			clrDst.rgbMul += rgbSrc.rgbMul ;
			clrDst.rgbAdd += rgbSrc.rgbAdd ;
			return	clrDst ;
		}
		bool operator == ( const S3DColor& clr ) const
		{
			return	(rgbMul.ui32 == clr.rgbMul.ui32)
					&& (rgbAdd.ui32 == clr.rgbAdd.ui32) ;
		}
		bool operator != ( const S3DColor& clr ) const
		{
			return	(rgbMul.ui32 != clr.rgbMul.ui32)
					|| (rgbAdd.ui32 != clr.rgbAdd.ui32) ;
		}
		bool IsTransparent( void ) const
		{
			// 無色透明
			return	((rgbMul.ui32 & 0x00FFFFFF) == 0x00FFFFFF)
					& ((rgbAdd.ui32 & 0x00FFFFFF) == 0x00000000) ;
		}
		bool IsTranslucent( void ) const
		{
			// 半透明
			return	(((rgbMul.ui32 & 0x00FFFFFF) != 0x00FFFFFF)
						& ((rgbMul.ui32 & 0x00FFFFFF) != 0x00000000))
					| ((rgbAdd.ui32 & 0x00FFFFFF) != 0x00000000) ;
		}
		bool IsOpaque( void ) const
		{
			// 不透明
			return	((rgbMul.ui32 & 0x00FFFFFF) == 0) ;
		}
	} ;

	// 色配列積加算
	void AddProductedColorArray
		( SGLPalette * pDst,
			const SGLPalette * pvSrc, float32_t fpWeight, size_t nCount ) ;
	void AddProductedColorPairArray
		( S3DColor * pDst,
			const S3DColor * pvSrc, float32_t fpWeight, size_t nCount ) ;
	void AddProductedColorPairArrayWithWeight
		( S3DColor * pDst,
			const S3DColor * pvSrc,
			const float32_t * pfpWeightMap,
			float32_t fpWeight, size_t nCount ) ;
	void AddProductedColorPairArrayWithNegWeight
		( S3DColor * pDst,
			const S3DColor * pvSrc,
			const float32_t * pfpWeightMap, size_t nCount ) ;


	//////////////////////////////////////////////////////////////////////////
	// saturated 16bit integer conversion
	//////////////////////////////////////////////////////////////////////////

	inline int16_t sglCvtIntToI16( int n )
	{
		return	(int16_t) esl_clampi( n, -0x8000, 0x7FFF ) ;
	}

	inline uint8_t sglCvtIntToUI8( int n )
	{
		return	(uint8_t) esl_clampi( n, 0, 0xFF ) ;
	}


	//////////////////////////////////////////////////////////////////////////
	// カラー・コンポジション（符号あり16ビット）
	//////////////////////////////////////////////////////////////////////////

	struct	SGLWPaletteARGB
	{
		int16_t	Blue ;
		int16_t	Green ;
		int16_t	Red ;
		int16_t	Alpha ;

		SGLWPaletteARGB( void ) : Blue(0), Green(0), Red(0), Alpha(0) {}
		SGLWPaletteARGB( const SGLWPaletteARGB& argb )
			: Blue(argb.Blue), Green(argb.Green), Red(argb.Red), Alpha(argb.Alpha) {}
		SGLWPaletteARGB( const SGLPalette& src )
			: Blue(src.argb.Blue), Green(src.argb.Green),
				Red(src.argb.Red), Alpha(src.argb.Alpha) {}
		SGLWPaletteARGB( int nBlue, int nGreen, int nRed, int nAlpha )
			: Blue(sglCvtIntToI16(nBlue)),
				Green(sglCvtIntToI16(nGreen)),
				Red(sglCvtIntToI16(nRed)),
				Alpha(sglCvtIntToI16(nAlpha)) {}

		const SGLWPaletteARGB& operator = ( const SGLWPaletteARGB& argb )
		{
			Blue = argb.Blue ;
			Green = argb.Green ;
			Red = argb.Red ;
			Alpha = argb.Alpha ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator = ( const SGLPalette& src )
		{
			Blue = src.argb.Blue ;
			Green = src.argb.Green ;
			Red = src.argb.Red ;
			Alpha = src.argb.Alpha ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator *= ( int x )
		{
			Blue *= x ;
			Green *= x ;
			Red *= x ;
			Alpha *= x ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator >>= ( int x )
		{
			Blue >>= x ;
			Green >>= x ;
			Red >>= x ;
			Alpha >>= x ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator <<= ( int x )
		{
			Blue <<= x ;
			Green <<= x ;
			Red <<= x ;
			Alpha <<= x ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator += ( const SGLWPaletteARGB& argb )
		{
			Blue = sglCvtIntToI16( (int) Blue + argb.Blue ) ;
			Green = sglCvtIntToI16( (int) Green + argb.Green ) ;
			Red = sglCvtIntToI16( (int) Red + argb.Red ) ;
			Alpha = sglCvtIntToI16( (int) Alpha + argb.Alpha ) ;
			return	*this ;
		}
		const SGLWPaletteARGB& operator -= ( const SGLWPaletteARGB& argb )
		{
			Blue = sglCvtIntToI16( (int) Blue - argb.Blue ) ;
			Green = sglCvtIntToI16( (int) Green - argb.Green ) ;
			Red = sglCvtIntToI16( (int) Red - argb.Red ) ;
			Alpha = sglCvtIntToI16( (int) Alpha - argb.Alpha ) ;
			return	*this ;
		}
		operator SGLPalette ( void ) const
		{
			SGLPalette	dst ;
			dst.argb.Blue = sglCvtIntToUI8(Blue) ;
			dst.argb.Green = sglCvtIntToUI8(Green) ;
			dst.argb.Red = sglCvtIntToUI8(Red) ;
			dst.argb.Alpha = sglCvtIntToUI8(Alpha) ;
			return	dst ;
		}
		SGLWPaletteARGB operator - ( void ) const
		{
			SGLWPaletteARGB	dst ;
			dst.Blue = - Blue ;
			dst.Green = - Green ;
			dst.Red = - Red ;
			dst.Alpha = - Alpha ;
			return	dst ;
		}
		SGLWPaletteARGB operator * ( int x ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst *= x ;
			return	dst ;
		}
		SGLWPaletteARGB operator * ( double x ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst *= esl_clampi( (int) esl_lroundfi( x * 0x80 ), -0x80, 0x80 ) ;
			dst >>= 7 ;
			return	dst ;
		}
		SGLWPaletteARGB operator >> ( int x ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst >>= x ;
			return	dst ;
		}
		SGLWPaletteARGB operator << ( int x ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst <<= x ;
			return	dst ;
		}
		SGLWPaletteARGB operator + ( const SGLWPaletteARGB& argb ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst += argb ;
			return	dst ;
		}
		SGLWPaletteARGB operator - ( const SGLWPaletteARGB& argb ) const
		{
			SGLWPaletteARGB	dst = *this ;
			dst -= argb ;
			return	dst ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二次元ベクトル・テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T, class S> struct	SGL2DVector
	{
		T	x ;
		T	y ;

		#if	!defined(__COTOPHA__)
		SGL2DVector( void ) : x(0), y(0) {}
		#endif
		SGL2DVector( const SGL2DVector<T,S> & v ) : x(v.x), y(v.y) {}
		SGL2DVector( T xInit, T yInit ) : x(xInit), y(yInit) {}
		bool operator == ( const SGL2DVector<T,S> & v ) const
		{
			return	(x == v.x) & (y == v.y) ;
		}
		bool operator != ( const SGL2DVector<T,S> & v ) const
		{
			return	(x != v.x) | (y != v.y) ;
		}
		bool IsEqual( const SGL2DVector<T,S> & v, double err = 1.0e-7 ) const
		{
			return	(fabs(x - v.x) <= err) & (fabs(y - v.y) <= err) ;
		}
		bool IsNaN( void ) const
		{
			return	(x != x) || (y != y) ;
		}
		const SGL2DVector<T,S> & operator = ( const SGL2DVector<T,S> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const SGL2DVector<T,S> & operator += ( const SGL2DVector<T,S> & v )
		{
			x += v.x ;
			y += v.y ;
			return	*this ;
		}
		const SGL2DVector<T,S> & operator -= ( const SGL2DVector<T,S> & v )
		{
			x -= v.x ;
			y -= v.y ;
			return	*this ;
		}
		const SGL2DVector<T,S> & operator *= ( const SGL2DVector<T,S> & v )
		{
			return	*this = (*this * v) ;
		}
		const SGL2DVector<T,S> & operator *= ( S s )
		{
			x *= (T) s ;
			y *= (T) s ;
			return	*this ;
		}
		const SGL2DVector<T,S> & operator /= ( const SGL2DVector<T,S> & v )
		{
			return	*this = (*this / v) ;
		}
		const SGL2DVector<T,S> & operator /= ( S s )
		{
			x /= (T) s ;
			y /= (T) s ;
			return	*this ;
		}
		SGL2DVector<T,S> operator + ( const SGL2DVector<T,S> & v ) const
		{
			return	SGL2DVector<T,S>( x + v.x, y + v.y ) ;
		}
		SGL2DVector<T,S> operator - ( const SGL2DVector<T,S> & v ) const
		{
			return	SGL2DVector<T,S>( x - v.x, y - v.y ) ;
		}
		SGL2DVector<T,S> operator - ( void ) const
		{
			return	SGL2DVector<T,S>( - x, - y ) ;
		}
		SGL2DVector<T,S> operator * ( const SGL2DVector<T,S> & v ) const
		{
			return	SGL2DVector<T,S>( x * v.x - y * v.y, x * v.y + y * v.x ) ;
		}
		SGL2DVector<T,S> operator * ( S s ) const
		{
			return	SGL2DVector<T,S>( (T)(x * s), (T)(y * s) ) ;
		}
		SGL2DVector<T,S> operator / ( S s ) const
		{
			return	SGL2DVector<T,S>( (T)(x / s), (T)(y / s) ) ;
		}
		SGL2DVector<T,S> operator / ( const SGL2DVector<T,S> & v ) const
		{
			T	d = v.x * v.x + v.y * v.y ;
			return	SGL2DVector<T,S>
				( (x * v.x + y * v.y) / d, (y * v.x - y * v.x) / d ) ;
		}
		T operator | ( const SGL2DVector<T,S> & v ) const
		{
			return	x * v.x + y * v.y ;
		}
		T InnerProduct( const SGL2DVector<T,S> & v ) const
		{
			return	x * v.x + y * v.y ;
		}
		T Absolute( void ) const
		{
			return	(T) sqrt( x * x + y * y ) ;
		}
		SGL2DVector<T,S>& Normalize( void )
		{
			T	d = (T) sqrt( x * x + y * y ) ;
			if ( d > 0.0 )
			{
				*this *= (T) (1.0 / d) ;
			}
			return	*this ;
		}
		SGL2DVector<T,S> Normalized( void ) const
		{
			T	d = (T) sqrt( x * x + y * y ) ;
			if ( d > 0.0 )
			{
				return	*this * (T) (1.0 / d) ;
			}
			return	*this ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Point
	//////////////////////////////////////////////////////////////////////////

	struct	SGLPoint	: public SGL2DVector<int32_t,int>
	{
		#if	!defined(__COTOPHA__)
		SGLPoint( void ) {}
		#endif
		SGLPoint( const SGL2DVector<int32_t,int> & point )
		{
			x = point.x ;
			y = point.y ;
		}
		SGLPoint( int xInit, int yInit )
		{
			x = xInit ;
			y = yInit ;
		}
		const SGLPoint & operator %= ( int s )
		{
			x %= s ;
			y %= s ;
			return	*this ;
		}
		SGLPoint operator % ( int s ) const
		{
			return	SGLPoint( x % s, y % s ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Size
	//////////////////////////////////////////////////////////////////////////

	struct	SGLSize
	{
		int32_t	w ;
		int32_t	h ;

		#if	!defined(__COTOPHA__)
		SGLSize( void ) : w(0), h(0) {}
		#endif
		SGLSize( const SGLSize & size )
		{
			w = size.w ;
			h = size.h ;
		}
		SGLSize( int wInit, int hInit )
		{
			w = wInit ;
			h = hInit ;
		}
		bool operator == ( const SGLSize & size ) const
		{
			return	(w == size.w) & (h == size.h) ;
		}
		bool operator != ( const SGLSize & size ) const
		{
			return	(w != size.w) | (h != size.h) ;
		}
		const SGLSize & operator += ( const SGLSize & size )
		{
			w += size.w ;
			h += size.h ;
			return	*this ;
		}
		const SGLSize & operator -= ( const SGLSize & size )
		{
			w -= size.w ;
			h -= size.h ;
			return	*this ;
		}
		const SGLSize & operator |= ( const SGLSize & size )
		{
			if ( w < size.w )
			{
				w = size.w ;
			}
			if ( h < size.h )
			{
				h = size.h ;
			}
			return	*this ;
		}
		const SGLSize & operator *= ( int s )
		{
			w *= s ;
			h *= s ;
			return	*this ;
		}
		const SGLSize & operator /= ( int s )
		{
			w /= s ;
			h /= s ;
			return	*this ;
		}
		const SGLSize & operator %= ( int s )
		{
			w %= s ;
			h %= s ;
			return	*this ;
		}
		SGLSize operator + ( const SGLSize & size ) const
		{
			return	SGLSize( w + size.w, h + size.h ) ;
		}
		SGLSize operator - ( const SGLSize & size ) const
		{
			return	SGLSize( w - size.w, h - size.h ) ;
		}
		SGLSize operator - ( void ) const
		{
			return	SGLSize( - w, - h ) ;
		}
		SGLSize operator | ( const SGLSize & size ) const
		{
			return	SGLSize
						( ((w > size.w) ? w : size.w),
							((h > size.h) ? h : size.h) ) ;
		}
		SGLSize operator * ( int s ) const
		{
			return	SGLSize( w * s, h * s ) ;
		}
		SGLSize operator / ( int s ) const
		{
			return	SGLSize( w / s, h / s ) ;
		}
		SGLSize operator % ( int s ) const
		{
			return	SGLSize( w % s, h % s ) ;
		}
		bool IsEmpty( void ) const
		{
			return	(w == 0) | (h == 0) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Rect
	//////////////////////////////////////////////////////////////////////////

	struct	SGLRect
	{
		#if	defined(__COTOPHA__)
		int32_t	left = 0 ;
		int32_t	top = 0 ;
		int32_t	right = -1 ;
		int32_t	bottom = -1 ;

		#else
		int32_t	left ;
		int32_t	top ;
		int32_t	right ;
		int32_t	bottom ;

		SGLRect( void ) : left(0), top(0), right(-1), bottom(-1) {}
		#endif
		SGLRect( const SGLRect & rct )
		{
			left = rct.left ;
			top = rct.top ;
			right = rct.right ;
			bottom = rct.bottom ;
		}
		SGLRect( int l, int t, int r, int b )
		{
			left = l ;
			top = t ;
			right = r ;
			bottom = b ;
		}
		inline SGLRect( const SGLImageRect & irct ) ;
		bool operator == ( const SGLRect & rct ) const
		{
			return	(left == rct.left) & (top == rct.top)
						& (right == rct.right) & (bottom == rct.bottom) ;
		}
		bool operator != ( const SGLRect & rct ) const
		{
			return	(left != rct.left) | (top != rct.top)
						| (right != rct.right) | (bottom != rct.bottom) ;
		}
		inline const SGLRect & operator = ( const SGLImageRect & irct ) ;
		const SGLRect & operator += ( const SGLPoint & pos )
		{
			left += pos.x ;
			top += pos.y ;
			right += pos.x ;
			bottom += pos.y ;
			return	*this ;
		}
		const SGLRect & operator += ( const SGLSize & size )
		{
			right += size.w ;
			bottom += size.h ;
			return	*this ;
		}
		const SGLRect & operator -= ( const SGLPoint & pos )
		{
			left -= pos.x ;
			top -= pos.y ;
			right -= pos.x ;
			bottom -= pos.y ;
			return	*this ;
		}
		const SGLRect & operator -= ( const SGLSize & size )
		{
			right -= size.w ;
			bottom -= size.h ;
			return	*this ;
		}
		SGLRect operator + ( const SGLPoint & pos ) const
		{
			return	SGLRect( left + pos.x, top + pos.y,
							right + pos.x, bottom + pos.y ) ;
		}
		SGLRect operator + ( const SGLSize & size ) const
		{
			return	SGLRect( left, top,
							right + size.w, bottom + size.h ) ;
		}
		SGLRect operator - ( const SGLPoint & pos ) const
		{
			return	SGLRect( left - pos.x, top - pos.y,
							right - pos.x, bottom - pos.y ) ;
		}
		SGLRect operator - ( const SGLSize & size ) const
		{
			return	SGLRect( left, top,
							right - size.w, bottom - size.h ) ;
		}
		SGLRect operator - ( void ) const
		{
			return	SGLRect( - left, - top, - right, - bottom ) ;
		}
		bool operator &= ( const SGLRect & rct )
		{
			if ( (left > rct.right) | (top > rct.bottom)
					| (right < rct.left) | (bottom < rct.top) )
			{
				left = top = 0 ;
				right = bottom = -1 ;
				return	false ;
			}
			else
			{
				if ( left < rct.left )
					left = rct.left ;
				if ( top < rct.top )
					top = rct.top ;
				if ( right > rct.right )
					right = rct.right ;
				if ( bottom > rct.bottom )
					bottom = rct.bottom ;
				return	true ;
			}
		}
		SGLRect operator & ( const SGLRect & rct )
		{
			SGLRect	t = *this ;
			t &= rct ;
			return	t ;
		}
		void operator |= ( const SGLRect & rct )
		{
			if ( left > rct.left )
			{
				left = rct.left ;
			}
			if ( top > rct.top )
			{
				top = rct.top ;
			}
			if ( right < rct.right )
			{
				right = rct.right ;
			}
			if ( bottom < rct.bottom )
			{
				bottom = rct.bottom ;
			}
		}
		SGLRect operator | ( const SGLRect & rct )
		{
			SGLRect	t = *this ;
			t |= rct ;
			return	t ;
		}
		SGLPoint GetPosition( void ) const
		{
			return	SGLPoint( left, top ) ;
		}
		SGLSize GetSize( void ) const
		{
			return	SGLSize( right - left + 1, bottom - top + 1 ) ;
		}
		void SetPosition( const SGLPoint& pos )
		{
			left = pos.x ;
			top = pos.y ;
		}
		void SetSize( const SGLSize& size )
		{
			right = left + size.w - 1 ;
			bottom = top + size.h - 1 ;
		}
		void SetWidth( int width )
		{
			right = left + width - 1 ;
		}
		void SetHeight( int height )
		{
			bottom = top + height - 1 ;
		}
		int32_t GetWidth( void ) const
		{
			return	right - left + 1 ;
		}
		int32_t GetHeight( void ) const
		{
			return	bottom - top + 1 ;
		}
		bool IsEmpty( void ) const
		{
			return	(left > right) | (top > bottom) ;
		}
		void Clear( void )
		{
			left = top = 0 ;
			right = bottom = -1 ;
		}
		bool IsBounds( int x, int y ) const
		{
			return	(left <= x) & (x <= right)
					& (top <= y) & (y <= bottom) ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ImageRect
	//////////////////////////////////////////////////////////////////////////

	struct	SGLImageRect
	{
		int32_t	x ;
		int32_t	y ;
		int32_t	w ;
		int32_t	h ;

		#if	!defined(__COTOPHA__)
		SGLImageRect( void ) : x(0), y(0), w(0), h(0) {}
		#endif
		SGLImageRect( const SGLImageRect & rct )
		{
			x = rct.x ;
			y = rct.y ;
			w = rct.w ;
			h = rct.h ;
		}
		SGLImageRect( const SGLPoint & pos, const SGLSize & size )
		{
			x = pos.x ;
			y = pos.y ;
			w = size.w ;
			h = size.h ;
		}
		SGLImageRect( int xPos, int yPos, int width, int height )
		{
			x = xPos ;
			y = yPos ;
			w = width ;
			h = height ;
		}
		SGLImageRect( const SGLRect & rct )
		{
			x = rct.left ;
			y = rct.top ;
			w = rct.right - rct.left + 1 ;
			h = rct.bottom - rct.top + 1 ;
		}
		bool operator == ( const SGLImageRect & rct ) const
		{
			return	(x == rct.x) & (y == rct.y)
						& (w == rct.w) & (h == rct.h) ;
		}
		bool operator != ( const SGLImageRect & rct ) const
		{
			return	(x != rct.x) | (y != rct.y)
						| (w != rct.w) | (h != rct.h) ;
		}
		const SGLImageRect & operator = ( const SGLRect & rct )
		{
			x = rct.left ;
			y = rct.top ;
			w = rct.right - rct.left + 1 ;
			h = rct.bottom - rct.top + 1 ;
			return	*this ;
		}
		const SGLImageRect & operator += ( const SGLPoint & pos )
		{
			x += pos.x ;
			y += pos.y ;
			return	*this ;
		}
		const SGLImageRect & operator += ( const SGLSize & size )
		{
			w += size.w ;
			h += size.h ;
			return	*this ;
		}
		const SGLImageRect & operator -= ( const SGLPoint & pos )
		{
			x -= pos.x ;
			y -= pos.y ;
			return	*this ;
		}
		const SGLImageRect & operator -= ( const SGLSize & size )
		{
			w -= size.w ;
			h -= size.h ;
			return	*this ;
		}
		SGLImageRect operator + ( const SGLPoint & pos ) const
		{
			return	SGLImageRect( x + pos.x, y + pos.y, w, h ) ;
		}
		SGLImageRect operator + ( const SGLSize & size ) const
		{
			return	SGLImageRect( x, y, w + size.w, h + size.h ) ;
		}
		SGLImageRect operator - ( const SGLPoint & pos ) const
		{
			return	SGLImageRect( x - pos.x, y - pos.y, w, h ) ;
		}
		SGLImageRect operator - ( const SGLSize & size ) const
		{
			return	SGLImageRect( x, y, w - size.w, h - size.h ) ;
		}
		SGLImageRect operator - ( void ) const
		{
			return	SGLImageRect( - x, - y, - w, - h ) ;
		}
		SGLPoint GetPosition( void ) const
		{
			return	SGLPoint( x, y ) ;
		}
		SGLSize GetSize( void ) const
		{
			return	SGLSize( w, h ) ;
		}
		void SetPosition( const SGLPoint& pos )
		{
			x = pos.x ;
			y = pos.y ;
		}
		void SetSize( const SGLSize& size )
		{
			w = size.w ;
			h = size.h ;
		}
		bool IsEmpty( void ) const
		{
			return	(w <= 0) | (h <= 0) ;
		}
		void Clear( void )
		{
			x = y = 0 ;
			w = h = 0 ;
		}
		bool IsBounds( int xPos, int yPos ) const
		{
			return	(x <= xPos) & (xPos < x + w)
					& (y <= yPos) & (yPos < y + h) ;
		}
	} ;
}

inline SakuraGL::SGLRect::SGLRect( const SakuraGL::SGLImageRect & irct )
{
	left = irct.x ;
	top = irct.y ;
	right = irct.x + irct.w - 1 ;
	bottom = irct.y + irct.h - 1 ;
}

inline const SakuraGL::SGLRect &
		SakuraGL::SGLRect::operator = ( const SakuraGL::SGLImageRect & irct )
{
	left = irct.x ;
	top = irct.y ;
	right = irct.x + irct.w - 1 ;
	bottom = irct.y + irct.h - 1 ;
	return	*this ;
}

namespace SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 三次元ベクトル・テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T> struct	SGL3DVector
	{
		T	x ;
		T	y ;
		T	z ;

		#if	!defined(__COTOPHA__)
		SGL3DVector( void ) : x(0), y(0), z(0) {}
		#endif
		SGL3DVector( const SGL3DVector<T> & v ) : x(v.x), y(v.y), z(v.z) {}
		SGL3DVector( T xInit, T yInit, T zInit ) : x(xInit), y(yInit), z(zInit) {}
		bool operator == ( const SGL3DVector<T> & v ) const
		{
			return	(x == v.x) & (y == v.y) & (z == v.z) ;
		}
		bool operator != ( const SGL3DVector<T> & v ) const
		{
			return	(x != v.x) | (y != v.y) | (z != v.z) ;
		}
		bool IsEqual( const SGL3DVector<T> & v, double err = 1.0e-7 ) const
		{
			return	(fabs(x - v.x) <= err)
						& (fabs(y - v.y) <= err)
						 & (fabs(z - v.z) <= err) ;
		}
		bool IsNaN( void ) const
		{
			return	(x != x) || (y != y) || (z != z) ;
		}
		const SGL3DVector<T> & operator += ( const SGL3DVector<T> & v )
		{
			x += v.x ;
			y += v.y ;
			z += v.z ;
			return	*this ;
		}
		const SGL3DVector<T> & operator -= ( const SGL3DVector<T> & v )
		{
			x -= v.x ;
			y -= v.y ;
			z -= v.z ;
			return	*this ;
		}
		const SGL3DVector<T> & operator *= ( const SGL3DVector<T> & v )
		{
			return	*this = (*this * v) ;
		}
		const SGL3DVector<T> & operator *= ( T s )
		{
			x *= s ;
			y *= s ;
			z *= s ;
			return	*this ;
		}
		const SGL3DVector<T> & operator /= ( T s )
		{
			x /= s ;
			y /= s ;
			z /= s ;
			return	*this ;
		}
		SGL3DVector<T> operator + ( const SGL3DVector<T> & v ) const
		{
			return	SGL3DVector<T>( x + v.x, y + v.y, z + v.z ) ;
		}
		SGL3DVector<T> operator - ( const SGL3DVector<T> & v ) const
		{
			return	SGL3DVector<T>( x - v.x, y - v.y, z - v.z ) ;
		}
		SGL3DVector<T> operator - ( void ) const
		{
			return	SGL3DVector<T>( - x, - y, - z ) ;
		}
		SGL3DVector<T> operator * ( const SGL3DVector<T> & v ) const
		{
			return	SGL3DVector<T>
					( (T) (y * v.z - z * v.y),
						(T) (z * v.x - x * v.z),
						(T) (x * v.y - y * v.x) ) ;
		}
		SGL3DVector<T> operator * ( double s ) const
		{
			return	SGL3DVector<T>( (T) (x * s), (T) (y * s), (T) (z * s) ) ;
		}
		SGL3DVector<T> operator / ( double s ) const
		{
			return	SGL3DVector<T>( (T) (x / s), (T) (y / s), (T) (z / s) ) ;
		}
		T operator | ( const SGL3DVector<T> & v ) const
		{
			return	x * v.x + y * v.y + z * v.z ;
		}
		double Absolute( void ) const
		{
			return	sqrt( x * x + y * y + z * z ) ;
		}
		const SGL3DVector<T> & ExteriorProduct( const SGL3DVector<T> & v )
		{
			return	*this = (*this * v) ;
		}
		T InnerProduct( const SGL3DVector<T> & v ) const
		{
			return	x * v.x + y * v.y + z * v.z ;
		}
		const SGL3DVector<T>& Normalize( void )
		{
			double	d = sqrt( x * x + y * y + z * z ) ;
			if ( d > 0.0 )
			{
				*this *= (T) (1.0 / d) ;
			}
			return	*this ;
		}
		SGL3DVector<T> Normalized( void ) const
		{
			double	d = sqrt( x * x + y * y + z * z ) ;
			if ( d > 0.0 )
			{
				return	*this * (T) (1.0 / d) ;
			}
			return	*this ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 三次元変換行列・テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T, int N> struct	SGL3DMatrix
	{
		T	m[3][N] ;

		// 構築
		#if	!defined(__COTOPHA__)
		SGL3DMatrix( void )
		{
			eslFillMemory( &m[0][0], 0, sizeof(T) * 3 * N ) ;
		}
		#endif
		SGL3DMatrix( T m11, T m12, T m13,
						T m21, T m22, T m23,
						T m31, T m32, T m33 )
		{
			m[0][0] = m11 ;  m[0][1] = m12 ;  m[0][2] = m13 ;
			m[1][0] = m21 ;  m[1][1] = m22 ;  m[1][2] = m23 ;
			m[2][0] = m31 ;  m[2][1] = m32 ;  m[2][2] = m33 ;
		}
		SGL3DMatrix( const SGL3DVector<T> & v0,
						const SGL3DVector<T> & v1,
						const SGL3DVector<T> & v2 )
		{
			m[0][0] = v0.x ;  m[0][1] = v1.x ;  m[0][2] = v2.x ;
			m[1][0] = v0.y ;  m[1][1] = v1.y ;  m[1][2] = v2.y ;
			m[2][0] = v0.z ;  m[2][1] = v1.z ;  m[2][2] = v2.z ;
		}
		explicit SGL3DMatrix( const SGL3DVector<T> & v )
		{
			m[0][0] = v.x ;  m[0][1] = 0 ;    m[0][2] = 0 ;
			m[1][0] = 0 ;    m[1][1] = v.y ;  m[1][2] = 0 ;
			m[2][0] = 0 ;    m[2][1] = 0 ;    m[2][2] = v.z ;
		}
		SGL3DMatrix( T m11, T m22, T m33 )
		{
			m[0][0] = m11 ;  m[0][1] = 0 ;    m[0][2] = 0 ;
			m[1][0] = 0 ;    m[1][1] = m22 ;  m[1][2] = 0 ;
			m[2][0] = 0 ;    m[2][1] = 0 ;    m[2][2] = m33 ;
		}
		// 対角行列初期化
		void InitializeMatrix( const SGL3DVector<T> & v )
		{
			eslFillMemory( &m[0][0], 0, sizeof(T) * 3 * N ) ;
			m[0][0] = v.x ;
			m[1][1] = v.y ;
			m[2][2] = v.z ;
		}
		// 比較
		bool operator == ( const SGL3DMatrix<T,N> & mat ) const
		{
			return	(m[0][0] == mat.m[0][0])
						& (m[0][1] == mat.m[0][1])
						& (m[0][2] == mat.m[0][2])
					&& (m[1][0] == mat.m[1][0])
						& (m[1][1] == mat.m[1][1])
						& (m[1][2] == mat.m[1][2])
					&& (m[2][0] == mat.m[2][0])
						& (m[2][1] == mat.m[2][1])
						& (m[2][2] == mat.m[2][2]) ;
		}
		bool operator != ( const SGL3DMatrix<T,N> & mat ) const
		{
			return	(m[0][0] != mat.m[0][0])
						| (m[0][1] != mat.m[0][1])
						| (m[0][2] != mat.m[0][2])
					|| (m[1][0] != mat.m[1][0])
						| (m[1][1] != mat.m[1][1])
						| (m[1][2] != mat.m[1][2])
					|| (m[2][0] != mat.m[2][0])
						| (m[2][1] != mat.m[2][1])
						| (m[2][2] != mat.m[2][2]) ;
		}
		// 列
		SGL3DVector<T> GetColumn( size_t i ) const
		{
			ESLAssert( i < 3 ) ;
			return	SGL3DVector<T>( m[0][i], m[1][i], m[2][i] ) ;
		}
		void SetColumn( size_t i, const SGL3DVector<T>& v )
		{
			ESLAssert( i < 3 ) ;
			m[0][i] = v.x ;  m[1][i] = v.y ;  m[2][i] = v.z ;
		}
		SGL3DVector<T> GetLine( size_t i ) const
		{
			ESLAssert( i < 3 ) ;
			return	SGL3DVector<T>( m[i][0], m[i][1], m[i][2] ) ;
		}
		void SetLine( size_t i, const SGL3DVector<T>& v )
		{
			ESLAssert( i < 3 ) ;
			m[i][0] = v.x ;  m[i][1] = v.y ;  m[i][2] = v.z ;
		}
		// 加算
		const SGL3DMatrix<T,N> & operator += ( const SGL3DMatrix<T,N> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] += mat.m[i][0] ;
				m[i][1] += mat.m[i][1] ;
				m[i][2] += mat.m[i][2] ;
			}
			return	*this ;
		}
		// 減算
		const SGL3DMatrix<T,N> & operator -= ( const SGL3DMatrix<T,N> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] -= mat.m[i][0] ;
				m[i][1] -= mat.m[i][1] ;
				m[i][2] -= mat.m[i][2] ;
			}
			return	*this ;
		}
		// 積
		const SGL3DMatrix<T,N> & operator *= ( const SGL3DMatrix<T,N> & mat )
		{
			return	RevolveByMatrix( mat ) ;
		}
		const SGL3DMatrix<T,N> & operator *= ( T s )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] *= s ;
				m[i][1] *= s ;
				m[i][2] *= s ;
			}
			return	*this ;
		}
		const SGL3DMatrix<T,N> & operator /= ( T s )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] /= s ;
				m[i][1] /= s ;
				m[i][2] /= s ;
			}
			return	*this ;
		}
		// 加算
		SGL3DMatrix<T,N> operator + ( const SGL3DMatrix<T,N> & mat ) const
		{
			return	SGL3DMatrix<T,N>
				( m[0][0] + mat.m[0][0],
						m[0][1] + mat.m[0][1],
						m[0][2] + mat.m[0][2],
					m[1][0] + mat.m[1][0],
						m[1][1] + mat.m[1][1],
						m[1][2] + mat.m[1][2],
					m[2][0] + mat.m[2][0],
						m[2][1] + mat.m[2][1],
						m[2][2] + mat.m[2][2] ) ;
		}
		// 減算
		SGL3DMatrix<T,N> operator - ( const SGL3DMatrix<T,N> & mat ) const
		{
			return	SGL3DMatrix<T,N>
				( m[0][0] - mat.m[0][0],
						m[0][1] - mat.m[0][1],
						m[0][2] - mat.m[0][2],
					m[1][0] - mat.m[1][0],
						m[1][1] - mat.m[1][1],
						m[1][2] - mat.m[1][2],
					m[2][0] - mat.m[2][0],
						m[2][1] - mat.m[2][1],
						m[2][2] - mat.m[2][2] ) ;
		}
		SGL3DMatrix<T,N> operator - ( void ) const
		{
			return	SGL3DMatrix<T,N>
				( - m[0][0], - m[0][1], - m[0][2],
					- m[1][0], - m[1][1], - m[1][2],
					- m[2][0], - m[2][1], - m[2][2] ) ;
		}
		// 積
		SGL3DMatrix<T,N> operator * ( const SGL3DMatrix<T,N> & mat ) const
		{
			SGL3DMatrix<T,N>	t = *this ;
			return	t.RevolveByMatrix( mat ) ;
		}
		SGL3DVector<T> operator * ( const SGL3DVector<T> & v ) const
		{
			const T	x = v.x, y = v.y, z = v.z ;
			return	SGL3DVector<T>
				( m[0][0] * x + m[0][1] * y + m[0][2] * z,
					m[1][0] * x + m[1][1] * y + m[1][2] * z,
					m[2][0] * x + m[2][1] * y + m[2][2] * z ) ;
		}
		SGL3DMatrix<T,N> operator * ( T s ) const
		{
			return	SGL3DMatrix<T,N>
				( m[0][0] * s, m[0][1] * s, m[0][2] * s,
					m[1][0] * s, m[1][1] * s, m[1][2] * s,
					m[2][0] * s, m[2][1] * s, m[2][2] * s ) ;
		}
		SGL3DMatrix<T,N> operator / ( T s ) const
		{
			return	SGL3DMatrix<T,N>
				( m[0][0] / s, m[0][1] / s, m[0][2] / s,
					m[1][0] / s, m[1][1] / s, m[1][2] / s,
					m[2][0] / s, m[2][1] / s, m[2][2] / s ) ;
		}
		// 転置行列
		const SGL3DMatrix<T,N> & TransposeOf( const SGL3DMatrix<T,N> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = mat.m[0][i] ;
				m[i][1] = mat.m[1][i] ;
				m[i][2] = mat.m[2][i] ;
			}
			return	*this ;
		}
		SGL3DMatrix<T,N> Transpose( void ) const
		{
			SGL3DMatrix<T,N>	m ;
			m.TransposeOf( *this ) ;
			return	m ;
		}
		// 行列式
		T Determinant( void ) const
		{
			return	m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
					+ m[0][1] * (m[1][2] * m[2][0] - m[2][2] * m[1][0])
					+ m[0][2] * (m[1][0] * m[2][1] - m[2][0] * m[1][1]) ;
		}
		// 逆行列
		const SGL3DMatrix<T,N> & InverseOf( const SGL3DMatrix<T,N> & mat )
		{
			T	d = (T) (1.0 / mat.Determinant()) ;
			if ( isinf(d) )	d = 0.0 ;
			m[0][0] = d * (mat.m[1][1] * mat.m[2][2]
								- mat.m[1][2] * mat.m[2][1]) ;
			m[0][1] = d * (mat.m[2][1] * mat.m[0][2]
								- mat.m[2][2] * mat.m[0][1]) ;
			m[0][2] = d * (mat.m[0][1] * mat.m[1][2]
								- mat.m[0][2] * mat.m[1][1]) ;
			m[1][0] = d * (mat.m[1][2] * mat.m[2][0]
								- mat.m[1][0] * mat.m[2][2]) ;
			m[1][1] = d * (mat.m[2][2] * mat.m[0][0]
								- mat.m[2][0] * mat.m[0][2]) ;
			m[1][2] = d * (mat.m[0][2] * mat.m[1][0]
								- mat.m[0][0] * mat.m[1][2]) ;
			m[2][0] = d * (mat.m[1][0] * mat.m[2][1]
								- mat.m[1][1] * mat.m[2][0]) ;
			m[2][1] = d * (mat.m[2][0] * mat.m[0][1]
								- mat.m[2][1] * mat.m[0][0]) ;
			m[2][2] = d * (mat.m[0][0] * mat.m[1][1]
								- mat.m[0][1] * mat.m[1][0]) ;
			return	*this ;
		}
		SGL3DMatrix<T,N> Inverse( void ) const
		{
			SGL3DMatrix<T,N>	m ;
			m.InverseOf( *this ) ;
			return	m ;
		}
		// 軸回り回転（軸方向＋∞へ向かって反時計回り）
		void RevolveOnX( double rSin, double rCos )
		{
			// ( a11 a12 a13 )   ( 1    0       0    )
			// ( a21 a22 a23 ) X ( 0  cos(x)  sin(x) )
			// ( a31 a32 a33 )   ( 0 -sin(x)  cos(x) )
			for ( int i = 0; i < 3; i ++ )
			{
				T	r1, r2 ;
				r1 = m[i][1] ;
				r2 = m[i][2] ;
				m[i][1] = (T) (rCos * r1 - rSin * r2) ;
				m[i][2] = (T) (rCos * r2 + rSin * r1) ;
			}
		}
		void RevolveOnY( double rSin, double rCos )
		{
			// ( a11 a12 a13 )   (  cos(y)  0  sin(y) )
			// ( a21 a22 a23 ) X (    0     1    0    )
			// ( a31 a32 a33 )   ( -sin(y)  0  cos(y) )
			for ( int i = 0; i < 3; i ++ )
			{
				T	r1, r2 ;
				r1 = m[i][0] ;
				r2 = m[i][2] ;
				m[i][0] = (T) (rCos * r1 - rSin * r2) ;
				m[i][2] = (T) (rSin * r1 + rCos * r2) ;
			}
		}
		void RevolveOnZ( double rSin, double rCos )
		{
			// ( a11 a12 a13 )   ( cos(z) -sin(z)  0 )
			// ( a21 a22 a23 ) X ( sin(z)  cos(z)  0 )
			// ( a31 a32 a33 )   (   0       0     1 )
			for ( int i = 0; i < 3; i ++ )
			{
				T	r1, r2 ;
				r1 = m[i][0] ;
				r2 = m[i][1] ;
				m[i][0] = (T) (rCos * r1 + rSin * r2) ;
				m[i][1] = (T) (rCos * r2 - rSin * r1) ;
			}
		}
		// ベクトル a が {0,0,z} になる回転
		void RevolveByAngleOn( const SGL3DVector<T> & a )
		{
			T	xx_zz = a.x * a.x + a.z * a.z ;
			T	sqr_xz = (T) sqrt( xx_zz ) ;
			T	sqrt_xyz = (T) sqrt( xx_zz + a.y * a.y ) ;
			if ( sqrt_xyz > 1.0e-10 )
			{
				RevolveOnX( (- a.y / sqrt_xyz), (sqr_xz / sqrt_xyz) ) ;
			}
			if ( sqr_xz > 1.0e-10 )
			{
				RevolveOnY( (- a.x / sqr_xz), (a.z / sqr_xz) ) ;
			}
		}
		// {0,0,z} がベクトル a と平行になる回転
		void RevolveForAngle( const SGL3DVector<T> & a )
		{
			T	xx_zz = a.x * a.x + a.z * a.z ;
			T	sqrt_xz = (T) sqrt( xx_zz ) ;
			T	sqrt_xyz = (T) sqrt( xx_zz + a.y * a.y ) ;
			if ( sqrt_xz > 1.0e-10 )
			{
				RevolveOnY( (a.x / sqrt_xz), (a.z / sqrt_xz) ) ;
			}
			if ( sqrt_xyz > 1.0e-10 )
			{
				RevolveOnX( (a.y / sqrt_xyz), (sqrt_xz / sqrt_xyz) ) ;
			}
		}
		// ベクトル v 回り回転行列（ベクトル方向に向かって反時計回り）
		void RotationOnVectorOf( const SGL3DVector<T> & v, double rSin, double rCos )
		{
			SGL3DMatrix<T,N>	matR( 1, 1, 1 ) ;
			matR.RevolveByAngleOn( v ) ;
			//
			InverseOf( matR ) ;
			RevolveOnZ( rSin, rCos ) ;
			RevolveByMatrix( matR ) ;
		}
		// ベクトル v0 を回転してベクトル v1 と平行にする回転行列
		void VectorRotationOf( const SGL3DVector<T> & v0, const SGL3DVector<T> & v1 )
		{
			SGL3DVector<T>	vx = v0 * v1 ;
			if ( vx.Absolute() > 1.0e-8 )
			{
				SGL3DVector<T>	vn0 = v0 ;
				SGL3DVector<T>	vn1 = v1 ;
				vx.Normalize() ;
				vn0.Normalize() ;
				vn1.Normalize() ;
				//
				SGL3DMatrix<T,N>	matR( 1, 1, 1 ) ;
				matR.RevolveByAngleOn( vx ) ;
				InverseOf( matR ) ;
				//
				matR.RevolveVector( vn0 ) ;
				matR.RevolveVector( vn1 ) ;
				//
				T	c = (T) (vn0.x * vn1.x + vn0.y * vn1.y) ;
				T	s = (T) (vn0.x * vn1.y - vn0.y * vn1.x) ;
				//
				SGL3DMatrix<T,N>	matS( c, -s, 0,  s, c, 0,  0, 0, 1 ) ;
				//
				RevolveByMatrix( matS ) ;
				RevolveByMatrix( matR ) ;
			}
			else if ( v0.InnerProduct( v1 ) < 0 )
			{
				SGL3DMatrix<T,N>	matR( 1, 1, 1 ) ;
				matR.RevolveByAngleOn( v0 ) ;
				InverseOf( matR ) ;
				//
				SGL3DMatrix<T,N>	matS( -1, 1, -1 ) ;
				RevolveByMatrix( matS ) ;
				RevolveByMatrix( matR ) ;
			}
			else
			{
				eslFillMemory( &m[0][0], 0, sizeof(T) * 3 * N ) ;
				m[0][0] = (T) 1 ;
				m[1][1] = (T) 1 ;
				m[2][2] = (T) 1 ;
			}
		}
		// カメラ変換行列
		SGL3DVector<T> CameraAngleOf
			( const SGL3DVector<T>& vTarget,
				const SGL3DVector<T>& vView, const SGL3DVector<T>& vAngleTop )
		{
			eslFillMemory( &m[0][0], 0, sizeof(T) * 3 * N ) ;
			m[0][0] = (T) 1.0 ;
			m[1][1] = (T) 1.0 ;
			m[2][2] = (T) 1.0 ;
			//
			SGL3DVector<T>	vAngle = vTarget - vView ;
			RevolveByAngleOn( vAngle ) ;
			//
			SGL3DVector<T>	vTop = *this * vAngleTop ;
			double		xx_yy = sqrt( vTop.x * vTop.x + vTop.y * vTop.y ) ;
			if ( xx_yy > 1.0e-8 )
			{
				SGL3DMatrix<T,N>	matTop( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
				matTop.RevolveOnZ( - vTop.x / xx_yy, - vTop.y / xx_yy ) ;
				*this = matTop * *this ;
			}
			return	*this * vView ;
		}
		// 拡大
		void MagnifyByVector( const SGL3DVector<T> & v )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = m[i][0] * v.x ;
				m[i][1] = m[i][1] * v.y ;
				m[i][2] = m[i][2] * v.z ;
			}
		}
		// 拡大成分を抽出して除去
		void ExtractMagnification( SGL3DVector<T> & v )
		{
			v.x = (T) sqrt( m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0] ) ;
			v.y = (T) sqrt( m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1] ) ;
			v.z = (T) sqrt( m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2] ) ;
			if ( v.x > 1.0e-8 )
			{
				m[0][0] = m[0][0] / v.x ;
				m[1][0] = m[1][0] / v.x ;
				m[2][0] = m[2][0] / v.x ;
			}
			else
			{
				m[0][0] = 1.0 ;
				m[1][0] = 0.0 ;
				m[2][0] = 0.0 ;
			}
			if ( v.y > 1.0e-8 )
			{
				m[0][1] = m[0][1] / v.y ;
				m[1][1] = m[1][1] / v.y ;
				m[2][1] = m[2][1] / v.y ;
			}
			else
			{
				m[0][1] = 0.0 ;
				m[1][1] = 1.0 ;
				m[2][1] = 0.0 ;
			}
			if ( v.z > 1.0e-8 )
			{
				m[0][2] = m[0][2] / v.z ;
				m[1][2] = m[1][2] / v.z ;
				m[2][2] = m[2][2] / v.z ;
			}
			else
			{
				m[0][2] = 0.0 ;
				m[1][2] = 0.0 ;
				m[2][2] = 1.0 ;
			}
		} ;
		// v 方向に拡大
		void MagnifyOnVectorOf( const SGL3DVector<T> & v, T m )
		{
			if ( v.Absolute() > 1.0e-8 )
			{
				SGL3DMatrix<T,N>	matR( 1, 1, 1 ) ;
				matR.RevolveByAngleOn( v ) ;
				InverseOf( matR ) ;
				//
				SGL3DMatrix<T,N>	matM( 1, 0, 0,  0, 1, 0,  0, 0, m ) ;
				//
				RevolveByMatrix( matM ) ;
				RevolveByMatrix( matR ) ;
			}
		}
		// 回転
		void RevolveMatrix( SGL3DMatrix<T,N> & matDst ) const
		{
			SGL3DMatrix<T,N>	matTemp = matDst ;
			matDst = *this ;
			matDst.RevolveByMatrix( matTemp ) ;
		}
		const SGL3DMatrix<T,N> & RevolveByMatrix( const SGL3DMatrix<T,N> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				T	r1, r2, r3 ;
				r1 = m[i][0] ;
				r2 = m[i][1] ;
				r3 = m[i][2] ;
				m[i][0] = (r1 * mat.m[0][0] + r2 * mat.m[1][0] + r3 * mat.m[2][0]) ;
				m[i][1] = (r1 * mat.m[0][1] + r2 * mat.m[1][1] + r3 * mat.m[2][1]) ;
				m[i][2] = (r1 * mat.m[0][2] + r2 * mat.m[1][2] + r3 * mat.m[2][2]) ;
			}
			return	*this ;
		}
		void RevolveVector( SGL3DVector<T> & v ) const
		{
			const T	x = v.x, y = v.y, z = v.z ;
			v.x = m[0][0] * x + m[0][1] * y + m[0][2] * z ;
			v.y = m[1][0] * x + m[1][1] * y + m[1][2] * z ;
			v.z = m[2][0] * x + m[2][1] * y + m[2][2] * z ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// クォータニオン・テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T,class M> struct SGL3DQuaternion
	{
		T	q[4] ;

		// 構築
		#if	!defined(__COTOPHA__)
		SGL3DQuaternion( void )
		{
			q[0] = 0 ;  q[1] = 0 ;  q[2] = 0 ;  q[3] = 0 ;
		}
		#endif
		SGL3DQuaternion( T q1, T q2, T q3, T q4 )
		{
			q[0] = q1 ;  q[1] = q2 ;  q[2] = q3 ;  q[3] = q4 ;
		}
		// 比較
		bool operator == ( const SGL3DQuaternion<T,M> & qt ) const
		{
			return	(q[0] == qt.q[0]) & (q[1] == qt.q[1])
						& (q[2] == qt.q[2]) & (q[3] == qt.q[3]) ;
		}
		bool operator != ( const SGL3DQuaternion<T,M> & qt ) const
		{
			return	(q[0] != qt.q[0]) | (q[1] != qt.q[1])
						| (q[2] != qt.q[2]) | (q[3] != qt.q[3]) ;
		}
		// 行列へ変換
		void ToMatrix( M & mat ) const
		{
			mat.m[0][0] = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3] ;
			mat.m[0][1] = 2 * (q[1] * q[2] - q[0] * q[3]) ;
			mat.m[0][2] = 2 * (q[1] * q[3] + q[0] * q[2]) ;
			//
			mat.m[1][0] = 2 * (q[1] * q[2] + q[0] * q[3]) ;
			mat.m[1][1] = q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3] ;
			mat.m[1][2] = 2 * (q[2] * q[3] - q[0] * q[1]) ;
			//
			mat.m[2][0] = 2 * (q[1] * q[3] - q[0] * q[2]) ;
			mat.m[2][1] = 2 * (q[2] * q[3] + q[0] * q[1]) ;
			mat.m[2][2] = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] ;
		}
		// 行列から変換
		void FromMatrix( const M & mat )
		{
			q[0] = (T) (( mat.m[0][0] + mat.m[1][1] + mat.m[2][2] + 1) * 0.25) ;
			q[1] = (T) (( mat.m[0][0] - mat.m[1][1] - mat.m[2][2] + 1) * 0.25) ;
			q[2] = (T) ((-mat.m[0][0] + mat.m[1][1] - mat.m[2][2] + 1) * 0.25) ;
			q[3] = (T) ((-mat.m[0][0] - mat.m[1][1] + mat.m[2][2] + 1) * 0.25) ;
			//
			if ( q[0] < 0 )	q[0] = 0 ;
			if ( q[1] < 0 )	q[1] = 0 ;
			if ( q[2] < 0 )	q[2] = 0 ;
			if ( q[3] < 0 )	q[3] = 0 ;
			q[0] = (T) sqrt( q[0] ) ;
			q[1] = (T) sqrt( q[1] ) ;
			q[2] = (T) sqrt( q[2] ) ;
			q[3] = (T) sqrt( q[3] ) ;
			//
			if ( (q[0] < 1.0e-10) && (q[1] < 1.0e-10) )
			{
				q[0] = 0 ;
				q[1] = 0 ;
				if ( mat.m[2][1] + mat.m[1][2] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			else if ( q[0] < 1.0e-10 )
			{
				q[0] = 0 ;
				if ( mat.m[1][0] + mat.m[0][2] < 0 )
				{
					q[2] = - q[2] ;
				}
				if ( mat.m[0][2] + mat.m[2][0] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			else
			{
				if ( mat.m[2][1] - mat.m[1][2] < 0 )
				{
					q[1] = - q[1] ;
				}
				if ( mat.m[0][2] - mat.m[2][0] < 0 )
				{
					q[2] = - q[2] ;
				}
				if ( mat.m[1][0] - mat.m[0][1] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			Normalize( ) ;
		}
		// 回転軸と角度[rad]を取得
		double GetRodriguesRotaion( SGL3DVector<T> & v ) const
		{
			T	cosR = q[0] ;
			T	sinR = (T) ((cosR < 1.0) ? sqrt( 1.0 - cosR * cosR ) : 0.0) ;
			if ( sinR < 1.0e-8 )
			{
				v.x = 1.0 ;
				v.y = 0.0 ;
				v.z = 0.0 ;
				return	0.0 ;
			}
			T	rcpR = (T) (1.0 / sinR) ;
			v.x = q[1] * rcpR ;
			v.y = q[2] * rcpR ;
			v.z = q[3] * rcpR ;
			return	atan2( sinR, cosR ) * 2.0 ;
		}
		// 回転軸と角度[rad]を設定
		void SetRodriguesRotaion( const SGL3DVector<T> & v, T rad )
		{
			T	R = rad * (T) 0.5 ;
			T	sinR = (T) sin( R ) ;
			SGL3DVector<T>	vn = v ;
			vn.Normalize() ;
			q[0] = (T) cos( R ) ;
			q[1] = vn.x * sinR ;
			q[2] = vn.y * sinR ;
			q[3] = vn.z * sinR ;
		}
		// 加算
		const SGL3DQuaternion<T,M> & operator += ( const SGL3DQuaternion<T,M> & qt )
		{
			q[0] += qt.q[0] ;
			q[1] += qt.q[1] ;
			q[2] += qt.q[2] ;
			q[3] += qt.q[3] ;
			return	*this ;
		}
		SGL3DQuaternion<T,M> operator + ( const SGL3DQuaternion<T,M> & qt ) const
		{
			return	SGL3DQuaternion<T,M>
				( q[0] + qt.q[0], q[1] + qt.q[1],
						q[2] + qt.q[2], q[3] + qt.q[3] ) ;
		}
		// 減算
		const SGL3DQuaternion<T,M> & operator -= ( const SGL3DQuaternion<T,M> & qt )
		{
			q[0] -= qt.q[0] ;
			q[1] -= qt.q[1] ;
			q[2] -= qt.q[2] ;
			q[3] -= qt.q[3] ;
			return	*this ;
		}
		SGL3DQuaternion<T,M> operator - ( const SGL3DQuaternion<T,M> & qt ) const
		{
			return	SGL3DQuaternion<T,M>
				( q[0] - qt.q[0], q[1] - qt.q[1],
						q[2] - qt.q[2], q[3] - qt.q[3] ) ;
		}
		SGL3DQuaternion<T,M> operator - ( void ) const
		{
			return	SGL3DQuaternion<T,M>( - q[0], - q[1], - q[2], - q[3] ) ;
		}
		// 積
		const SGL3DQuaternion<T,M> & operator *= ( const SGL3DQuaternion<T,M> & qt )
		{
			*this = (*this * qt) ;
			return	*this ;
		}
		const SGL3DQuaternion<T,M> & operator *= ( T s )
		{
			q[0] *= s ;
			q[1] *= s ;
			q[2] *= s ;
			q[3] *= s ;
			return	*this ;
		}
		SGL3DQuaternion<T,M> operator * ( T s ) const
		{
			return	SGL3DQuaternion<T,M>
				( q[0] * s, q[1] * s, q[2] * s, q[3] * s ) ;
		}
		SGL3DQuaternion<T,M> operator * ( const SGL3DQuaternion<T,M> & qt ) const
		{
			SGL3DQuaternion<T,M>	t ;
			t.q[0] = q[0] * qt.q[0] - q[1] * qt.q[1]
							- q[2] * qt.q[2] - q[3] * qt.q[3] ;
			t.q[1] = q[0] * qt.q[1] + q[1] * qt.q[0]
							+ q[2] * qt.q[3] - q[3] * qt.q[2] ;
			t.q[2] = q[0] * qt.q[2] - q[1] * qt.q[3]
							+ q[2] * qt.q[0] + q[3] * qt.q[1] ;
			t.q[3] = q[0] * qt.q[3] + q[1] * qt.q[2]
							- q[2] * qt.q[1] + q[3] * qt.q[0] ;
			return	t ;
		}
		// 内積
		T InnerProduct( const SGL3DQuaternion<T,M> & qt ) const
		{
			return	q[0] * qt.q[0] + q[1] * qt.q[2]
						+ q[2] * qt.q[2] + q[3] * qt.q[3] ;
		}
		// 逆数
		SGL3DQuaternion<T,M> Inverse( void ) const
		{
			SGL3DQuaternion<T,M>	inv = *this ;
			SGL3DQuaternion<T,M>	i( 0, 1, 0, 0 ) ;
			SGL3DQuaternion<T,M>	j( 0, 0, 1, 0 ) ;
			SGL3DQuaternion<T,M>	k( 0, 0, 0, 1 ) ;
			inv += i * *this * i ;
			inv += j * *this * j ;
			inv += k * *this * k ;
			return	inv * (T) (-0.5 / Norm()) ;
		}
		// 線形補完 : q1 -> q2
		void Lerp( const SGL3DQuaternion<T,M> & q1,
					const SGL3DQuaternion<T,M> & q2, T t )
		{
			T	qr = q1.q[0] * q2.q[0] + q1.q[1] * q2.q[1]
							+ q1.q[2] * q2.q[2] + q1.q[3] * q2.q[3] ;
			T	nt = (T) 1.0 - t ;
			if ( qr < 0.0 )
			{
				nt = - nt ;
			}
			q[0] = q1.q[0] * nt + q2.q[0] * t ;
			q[1] = q1.q[1] * nt + q2.q[1] * t ;
			q[2] = q1.q[2] * nt + q2.q[2] * t ;
			q[3] = q1.q[3] * nt + q2.q[3] * t ;
			Normalize( ) ;
		}
		// 曲面補間 : q1 -> q2
		void Slerp( const SGL3DQuaternion<T,M> & q1,
					const SGL3DQuaternion<T,M> & q2, double t )
		{
			double	qr = q1.q[0] * q2.q[0] + q1.q[1] * q2.q[1]
							+ q1.q[2] * q2.q[2] + q1.q[3] * q2.q[3] ;
			if ( qr * qr >= 1.0 - 1.0e-10 )		// 1.0 - qr * qr != 0.0
			{
				q[0] = q1.q[0] ;
				q[1] = q1.q[1] ;
				q[2] = q1.q[2] ;
				q[3] = q1.q[3] ;
			}
			else
			{
				double	r = 1.0 ;
				if ( qr < 0.0 )
				{
					qr = - qr ;
					r = -1 ;
				}
				double	rad = acos( qr ) ;
				double	sr = 1.0 / sin( rad ) ;
				double	st1 = sin( rad * (1 - t) ) * sr * r ;
				double	st2 = sin( rad * t ) * sr ;
				//
				q[0] = (T) (q1.q[0] * st1 + q2.q[0] * st2) ;
				q[1] = (T) (q1.q[1] * st1 + q2.q[1] * st2) ;
				q[2] = (T) (q1.q[2] * st1 + q2.q[2] * st2) ;
				q[3] = (T) (q1.q[3] * st1 + q2.q[3] * st2) ;
				//
				Normalize( ) ;
			}
		}
		// ノルム
		T Norm( void ) const
		{
			return	(T) sqrt( q[0] * q[0] + q[1] * q[1]
								+ q[2] * q[2] + q[3] * q[3] ) ;
		}
		// 正規化
		void Normalize( void )
		{
			T	r = (T) sqrt( q[0] * q[0] + q[1] * q[1]
								+ q[2] * q[2] + q[3] * q[3] ) ;
			if ( r >= 1.0e-10 )
			{
				r = (T) (1.0 / r) ;
				q[0] *= r ;
				q[1] *= r ;
				q[2] *= r ;
				q[3] *= r ;
			}
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二次元ベクトル
	//////////////////////////////////////////////////////////////////////////

	struct	S2DVector	: public SGL2DVector<float32_t,double>
	{
		#if	!defined(__COTOPHA__)
		S2DVector( void ) {}
		#endif
		S2DVector( double x, double y )
			: SGL2DVector<float32_t,double>( (float32_t) x, (float32_t) y ) {}
		S2DVector( const SGL2DVector<float32_t,double> & v )
			: SGL2DVector<float32_t,double>( v ) {}
		S2DVector( const SGL2DVector<double,double> & v )
			: SGL2DVector<float32_t,double>( (float32_t) v.x, (float32_t) v.y ) {}
		S2DVector( const SGL3DVector<float32_t> & v )
			: SGL2DVector<float32_t,double>( v.x, v.y ) {}
		S2DVector( const SGL3DVector<double> & v )
			: SGL2DVector<float32_t,double>( (float32_t) v.x, (float32_t) v.y ) {}
		const S2DVector & operator = ( const SGL2DVector<float32_t,double> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const S2DVector & operator = ( const SGL2DVector<double,double> & v )
		{
			x = (float32_t) v.x ;
			y = (float32_t) v.y ;
			return	*this ;
		}
		const S2DVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const S2DVector & operator = ( const SGL3DVector<double> & v )
		{
			x = (float32_t) v.x ;
			y = (float32_t) v.y ;
			return	*this ;
		}
	} ;

	struct	S2DDVector	: public SGL2DVector<double,double>
	{
		#if	!defined(__COTOPHA__)
		S2DDVector( void ) {}
		#endif
		S2DDVector( double x, double y )
			: SGL2DVector<double,double>( x, y ) {}
		S2DDVector( const SGL2DVector<double,double> & v )
			: SGL2DVector<double,double>( v ) {}
		S2DDVector( const SGL2DVector<float32_t,double> & v )
			: SGL2DVector<double,double>( v.x, v.y ) {}
		S2DDVector( const SGL3DVector<double> & v )
			: SGL2DVector<double,double>( v.x, v.y ) {}
		S2DDVector( const SGL3DVector<float32_t> & v )
			: SGL2DVector<double,double>( v.x, v.y ) {}
		const S2DDVector & operator = ( const SGL2DVector<double,double> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const S2DDVector & operator = ( const SGL2DVector<float32_t,double> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const S2DDVector & operator = ( const SGL3DVector<double> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
		const S2DDVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;
			y = v.y ;
			return	*this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二次元変換行列・テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T> struct	SGL2DMatrix
	{
		T	m[2][2] ;

		// 構築
		SGL2DMatrix( void )
		{
			m[0][0] = 0 ;  m[0][1] = 0 ;
			m[1][0] = 0 ;  m[1][1] = 0 ;
		}
		SGL2DMatrix( T m11, T m12, T m21, T m22 )
		{
			m[0][0] = m11 ;  m[0][1] = m12 ;
			m[1][0] = m21 ;  m[1][1] = m22 ;
		}
		SGL2DMatrix( T m11, T m22 )
		{
			m[0][0] = m11 ;  m[0][1] = 0 ;
			m[1][0] = 0 ;    m[1][1] = m22 ;
		}
		SGL2DMatrix( const SGL2DVector<T,double>& v )
		{
			m[0][0] = v.x ;  m[0][1] = 0 ;
			m[1][0] = 0 ;    m[1][1] = v.y ;
		}
		SGL2DMatrix( const SGL2DMatrix<T>& mat )
		{
			m[0][0] = mat.m[0][0] ;  m[0][1] = mat.m[0][1] ;
			m[1][0] = mat.m[1][0] ;  m[1][1] = mat.m[1][1] ;
		}
		// 比較
		bool operator == ( const SGL2DMatrix<T> & mat ) const
		{
			return	(m[0][0] == mat.m[0][0])
						& (m[0][1] == mat.m[0][1])
					&& (m[1][0] == mat.m[1][0])
						& (m[1][1] == mat.m[1][1]) ;
		}
		bool operator != ( const SGL2DMatrix<T> & mat ) const
		{
			return	(m[0][0] != mat.m[0][0])
						| (m[0][1] != mat.m[0][1])
					|| (m[1][0] != mat.m[1][0])
						| (m[1][1] != mat.m[1][1]) ;
		}
		// 加算
		const SGL2DMatrix<T> & operator += ( const SGL2DMatrix<T> & mat )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				m[i][0] += mat.m[i][0] ;
				m[i][1] += mat.m[i][1] ;
			}
			return	*this ;
		}
		// 減算
		const SGL2DMatrix<T> & operator -= ( const SGL2DMatrix<T> & mat )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				m[i][0] -= mat.m[i][0] ;
				m[i][1] -= mat.m[i][1] ;
			}
			return	*this ;
		}
		// 積
		const SGL2DMatrix<T> & operator *= ( const SGL2DMatrix<T> & mat )
		{
			return	RevolveByMatrix( mat ) ;
		}
		const SGL2DMatrix<T> & operator *= ( T s )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				m[i][0] *= s ;
				m[i][1] *= s ;
			}
			return	*this ;
		}
		const SGL2DMatrix<T> & operator /= ( T s )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				m[i][0] /= s ;
				m[i][1] /= s ;
			}
			return	*this ;
		}
		// 加算
		SGL2DMatrix<T> operator + ( const SGL2DMatrix<T> & mat ) const
		{
			return	SGL2DMatrix<T>
				( m[0][0] + mat.m[0][0],
						m[0][1] + mat.m[0][1],
					m[1][0] + mat.m[1][0],
						m[1][1] + mat.m[1][1] ) ;
		}
		// 減算
		SGL2DMatrix<T> operator - ( const SGL2DMatrix<T> & mat ) const
		{
			return	SGL2DMatrix<T>
				( m[0][0] - mat.m[0][0],
						m[0][1] - mat.m[0][1],
					m[1][0] - mat.m[1][0],
						m[1][1] - mat.m[1][1] ) ;
		}
		SGL2DMatrix<T> operator - ( void ) const
		{
			return	SGL2DMatrix<T>
				( - m[0][0], - m[0][1],
					- m[1][0], - m[1][1] ) ;
		}
		// 積
		SGL2DMatrix<T> operator * ( const SGL2DMatrix<T> & mat ) const
		{
			SGL2DMatrix<T>	t = *this ;
			return	t.RevolveByMatrix( mat ) ;
		}
		SGL2DVector<T,double> operator * ( const SGL2DVector<T,double> & v ) const
		{
			const T	x = v.x, y = v.y ;
			return	SGL2DVector<T,double>
				( m[0][0] * x + m[0][1] * y,
					m[1][0] * x + m[1][1] * y ) ;
		}
		SGL2DMatrix<T> operator * ( T s ) const
		{
			return	SGL2DMatrix<T>
				( m[0][0] * s, m[0][1] * s,
					m[1][0] * s, m[1][1] * s ) ;
		}
		SGL2DMatrix<T> operator / ( T s ) const
		{
			return	SGL2DMatrix<T>
				( m[0][0] / s, m[0][1] / s,
					m[1][0] / s, m[1][1] / s ) ;
		}
		const SGL2DMatrix<T> & RevolveByMatrix( const SGL2DMatrix<T> & mat )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				T	r1, r2 ;
				r1 = m[i][0] ;
				r2 = m[i][1] ;
				m[i][0] = (r1 * mat.m[0][0] + r2 * mat.m[1][0]) ;
				m[i][1] = (r1 * mat.m[0][1] + r2 * mat.m[1][1]) ;
			}
			return	*this ;
		}
		// 転置行列
		const SGL2DMatrix<T> & TransposeOf( const SGL2DMatrix<T> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = mat.m[0][i] ;
				m[i][1] = mat.m[1][i] ;
			}
			return	*this ;
		}
		SGL2DMatrix<T> Transpose( void ) const
		{
			SGL2DMatrix<T>	m ;
			m.TransposeOf( *this ) ;
			return	m ;
		}
		// 行列式
		T Determinant( void ) const
		{
			return	m[0][0] * m[1][1] - m[0][1] * m[1][0] ;
		}
		// 逆行列
		const SGL2DMatrix<T> & InverseOf( const SGL2DMatrix<T> & mat )
		{
			T	d = (T) (1.0 / mat.Determinant()) ;
			if ( isinf(d) )	d = 0.0 ;
			m[0][0] =   d * mat.m[1][1] ;
			m[0][1] = - d * mat.m[0][1] ;
			m[1][0] = - d * mat.m[1][0] ;
			m[1][1] =   d * mat.m[0][0] ;
			return	*this ;
		}
		SGL2DMatrix<T> Inverse( void ) const
		{
			SGL2DMatrix<T>	m ;
			m.InverseOf( *this ) ;
			return	m ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二次元変換行列
	//////////////////////////////////////////////////////////////////////////

	struct	SGLAffine
	{
		#if	defined(__COTOPHA__)
		float32_t	a11 = 1.0f, a21 = 0.0f ;	// ｘ基底ベクトル
		float32_t	a12 = 0.0f, a22 = 1.0f ;	// ｙ基底ベクトル
		float32_t	a13 = 0.0f, a23 = 0.0f ;	// 原点

		#else
		float32_t	a11, a21 ;
		float32_t	a12, a22 ;
		float32_t	a13, a23 ;

		SGLAffine( void )
			: a11(1.0f), a12(0.0f), a13(0.0f),
				a21(0.0f), a22(1.0f), a23(0.0f) {}
		#endif
		SGLAffine( const SGLAffine & af )
			: a11(af.a11), a12(af.a12), a13(af.a13),
				a21(af.a21), a22(af.a22), a23(af.a23) {}
		SGLAffine( float32_t m11, float32_t m12, float32_t m13,
					float32_t m21, float32_t m22, float32_t m23 )
			: a11(m11), a12(m12), a13(m13),
				a21(m21), a22(m22), a23(m23) {}
		const SGLAffine & operator = ( const SGLAffine & af )
		{
			a11 = af.a11 ;  a12 = af.a12 ;  a13 = af.a13 ;
			a21 = af.a21 ;  a22 = af.a22 ;  a23 = af.a23 ;
			return	*this ;
		}
		bool operator == ( const SGLAffine & af ) const
		{
			return	(a11 == af.a11) & (a12 == af.a12) & (a13 == af.a13)
					& (a21 == af.a21) & (a22 == af.a22) & (a23 == af.a23) ;
		}
		bool operator != ( const SGLAffine & af ) const
		{
			return	(a11 != af.a11) | (a12 != af.a12) | (a13 != af.a13)
					| (a21 != af.a21) | (a22 != af.a22) | (a23 != af.a23) ;
		}
		const SGLAffine & operator += ( const SGLAffine & af )
		{
			a11 += af.a11 ;  a12 += af.a12 ;  a13 += af.a13 ;
			a21 += af.a21 ;  a22 += af.a22 ;  a23 += af.a23 ;
			return	*this ;
		}
		const SGLAffine & operator += ( const SGL2DVector<float32_t,double> & v )
		{
			a13 += v.x ;
			a23 += v.y ;
			return	*this ;
		}
		const SGLAffine & operator += ( const SGL2DVector<int32_t,int> & v )
		{
			a13 += (float32_t) v.x ;
			a23 += (float32_t) v.y ;
			return	*this ;
		}
		const SGLAffine & operator -= ( const SGLAffine & af )
		{
			a11 -= af.a11 ;  a12 -= af.a12 ;  a13 -= af.a13 ;
			a21 -= af.a21 ;  a22 -= af.a22 ;  a23 -= af.a23 ;
			return	*this ;
		}
		const SGLAffine & operator -= ( const SGL2DVector<float32_t,double> & v )
		{
			a13 -= v.x ;
			a23 -= v.y ;
			return	*this ;
		}
		const SGLAffine & operator -= ( const SGL2DVector<int32_t,double> & v )
		{
			a13 -= (float32_t) v.x ;
			a23 -= (float32_t) v.y ;
			return	*this ;
		}
		const SGLAffine & operator *= ( const SGLAffine & af )
		{
			return	*this = (*this * af) ;
		}
		const SGLAffine & operator *= ( double s )
		{
			a11 *= (float32_t) s ;  a12 *= (float32_t) s ;  a13 *= (float32_t) s ;
			a21 *= (float32_t) s ;  a22 *= (float32_t) s ;  a23 *= (float32_t) s ;
			return	*this ;
		}
		const SGLAffine & operator /= ( double s )
		{
			a11 /= (float32_t) s ;  a12 /= (float32_t) s ;  a13 /= (float32_t) s ;
			a21 /= (float32_t) s ;  a22 /= (float32_t) s ;  a23 /= (float32_t) s ;
			return	*this ;
		}
		SGLAffine operator * ( const SGLAffine & af ) const
		{
			return	SGLAffine
				( (a11 * af.a11 + a12 * af.a21),
						(a11 * af.a12 + a12 * af.a22),
						a13 + (a11 * af.a13 + a12 * af.a23),
					(a21 * af.a11 + a22 * af.a21),
						(a21 * af.a12 + a22 * af.a22),
						a23 + (a21 * af.a13 + a22 * af.a23) ) ;
		}
		SGLAffine operator * ( double s ) const
		{
			float32_t	f = (float32_t) s ;
			return	SGLAffine
				( a11 * f, a12 * f, a13 * f,
					a21 * f, a22 * f, a23 * f ) ;
		}
		SGLAffine operator / ( double s ) const
		{
			float32_t	f = (float32_t) s ;
			return	SGLAffine
				( a11 / f, a12 / f, a13 / f,
					a21 / f, a22 / f, a23 / f ) ;
		}
		SGL2DVector<float32_t,double> operator * ( const SGL2DVector<float32_t,double> & v ) const
		{
			return	SGL2DVector<float32_t,double>
						( a11 * v.x + a12 * v.y + a13,
							a21 * v.x + a22 * v.y + a23 ) ;
		}
		SGL2DVector<double,double> operator * ( const SGL2DVector<double,double> & v ) const
		{
			return	SGL2DVector<double,double>
						( a11 * v.x + a12 * v.y + a13,
							a21 * v.x + a22 * v.y + a23 ) ;
		}
		SGLAffine operator + ( const SGL2DVector<float32_t,double> & v ) const
		{
			return	SGLAffine( a11, a12, a13 + v.x, a21, a22, a23 + v.y ) ;
		}
		SGLAffine operator + ( const SGL2DVector<int32_t,int> & v ) const
		{
			return	SGLAffine( a11, a12, a13 + (float32_t) v.x,
								a21, a22, a23 + (float32_t) v.y ) ;
		}
		SGLAffine operator - ( const SGL2DVector<float32_t,double> & v ) const
		{
			return	SGLAffine( a11, a12, a13 - v.x, a21, a22, a23 - v.y ) ;
		}
		SGLAffine operator - ( const SGL2DVector<int32_t,int> & v ) const
		{
			return	SGLAffine( a11, a12, a13 - (float32_t) v.x,
								a21, a22, a23 - (float32_t) v.y ) ;
		}
		void TransformVectors
			( SGL2DVector<float32_t,double> * pvDst,
				const SGL2DVector<float32_t,double> * pvSrc, size_t nCount ) const
		{
			for ( size_t i = 0; i < nCount; i ++ )
			{
				float32_t	xSrc = pvSrc->x ;
				float32_t	ySrc = pvSrc->y ;
				pvDst->x = a11 * xSrc + a12 * ySrc + a13 ;
				pvDst->y = a21 * xSrc + a22 * ySrc + a23 ;
				++ pvSrc ;
				++ pvDst ;
			}
		}
		void TransformVectors
			( SGL2DVector<double,double> * pvDst,
				const SGL2DVector<double,double> * pvSrc, size_t nCount ) const
		{
			for ( size_t i = 0; i < nCount; i ++ )
			{
				double	xSrc = pvSrc->x ;
				double	ySrc = pvSrc->y ;
				pvDst->x = a11 * xSrc + a12 * ySrc + a13 ;
				pvDst->y = a21 * xSrc + a22 * ySrc + a23 ;
				++ pvSrc ;
				++ pvDst ;
			}
		}
		float32_t Determinant( void ) const
		{
			return	a11 * a22 - a12 * a21 ;
		}
		const SGLAffine & InverseOf( const SGLAffine & af )
		{
			float32_t	d = af.Determinant() ;
			if ( (d > 1.0e-8) | (d < -1.0e-8) )
			{
				d = 1.0f / d ;
			}
			a11 =   af.a22 * d ;
			a12 = - af.a12 * d ;
			a21 = - af.a21 * d ;
			a22 =   af.a11 * d ;
			a13 = - (a11 * af.a13 + a12 * af.a23) ;
			a23 = - (a21 * af.a13 + a22 * af.a23) ;
			return	*this ;
		}
		SGLAffine Inverse( void ) const
		{
			SGLAffine	affTemp ;
			affTemp.InverseOf( *this ) ;
			return	affTemp ;
		}
		const SGLAffine & MappingOf
			( const S2DVector * pvDst, const S2DVector * pvSrc )
		{
			S2DVector	uv1 = pvSrc[1] - pvSrc[0] ;
			S2DVector	uv2 = pvSrc[2] - pvSrc[0] ;
			S2DVector	v1 = pvDst[1] - pvDst[0] ;
			S2DVector	v2 = pvDst[2] - pvDst[0] ;
			float32_t	d = uv1.x * uv2.y - uv1.y * uv2.x ;
			if ( (d > 1.0e-8) | (d < -1.0e-8) )
			{
				d = 1.0f / d ;
			}
			a11 = (v1.x * uv2.y - v2.x * uv1.y) * d ;
			a21 = (v1.y * uv2.y - v2.y * uv1.y) * d ;
			a12 = (v1.x * uv2.x - v2.x * uv1.x) * -d ;
			a22 = (v1.y * uv2.x - v2.y * uv1.x) * -d ;
			a13 = pvDst[0].x - (pvSrc[0].x * a11 + pvSrc[0].y * a12) ;
			a23 = pvDst[0].y - (pvSrc[0].x * a21 + pvSrc[0].y * a22) ;
			return	*this ;
		}
		S2DVector GetPosition( void ) const
		{
			return	S2DVector( a13, a23 ) ;
		}
		void SetPosition( double x, double y )
		{
			a13 = (float32_t) x ;
			a23 = (float32_t) y ;
		}
		bool IsRotation( double err = 1.0e-5 ) const
		{
			return	(fabs(a11 - 1.0) > err) | (fabs(a12) > err)
					| (fabs(a21) > err) | (fabs(a22 - 1.0) > err) ;
		}
		double GetRotation( void ) const
		{
			return	atan2( a21, a22 ) ;
		}
		void SetRotation( double rad )
		{
			a11 = (float32_t) cos( rad ) ;
			a12 = (float32_t) - sin( rad ) ;
			a21 = (float32_t) sin( rad ) ;
			a22 = (float32_t) cos( rad ) ;
		}
		static bool MeshMapping
			( const S2DVector * pvMesh,
				int wMesh, int hMesh,
				S2DVector& vDstPos, const S2DVector& vSrcPos ) ;
		static bool InverseMeshMapping
			( const S2DVector * pvMesh,
				int wMesh, int hMesh,
				S2DVector& vSrcPos, const S2DVector& vDstPos ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 三次元ベクトル
	//////////////////////////////////////////////////////////////////////////

	struct	S3DVector	: public SGL3DVector<float32_t>
	{
		#if	!defined(__COTOPHA__)
		S3DVector( void ) {}
		#endif
		S3DVector( double xInit, double yInit, double zInit )
		{
			x = (float32_t) xInit ;  y = (float32_t) yInit ;  z = (float32_t) zInit ;
		}
		S3DVector( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
		}
		S3DVector( const SGL3DVector<double> & v )
		{
			x = (float32_t) v.x ;  y = (float32_t) v.y ;  z = (float32_t) v.z ;
		}
		const S3DVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;
			y = v.y ;
			z = v.z ;
			return	*this ;
		}
		const S3DVector & operator = ( const SGL3DVector<double> & v )
		{
			x = (float32_t) v.x ;
			y = (float32_t) v.y ;
			z = (float32_t) v.z ;
			return	*this ;
		}
	} ;

	struct	S3DDVector	: public SGL3DVector<double>
	{
		#if	!defined(__COTOPHA__)
		S3DDVector( void ) {}
		#endif
		S3DDVector( double xInit, double yInit, double zInit )
		{
			x = xInit ;  y = yInit ;  z = zInit ;
		}	
		S3DDVector( const SGL3DVector<double> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
		}
		S3DDVector( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
		}
		const S3DDVector & operator = ( const SGL3DVector<double> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
			return	*this ;
		}
		const S3DDVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
			return	*this ;
		}
	} ;

	struct	S3DVector4	: public S3DVector
	{
		float32_t	d ;

		#if	!defined(__COTOPHA__)
		S3DVector4( void ) : d(0) {}
		#endif
		S3DVector4( double xInit, double yInit, double zInit, double dInit = 0.0 )
		{
			x = (float32_t) xInit ;  y = (float32_t) yInit ;  z = (float32_t) zInit ;
			d = (float32_t) dInit ;
		}
		S3DVector4( const SGL3DVector<float32_t> & v, double dInit = 0.0 )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
			d = (float32_t) dInit ;
		}
		S3DVector4( const SGL3DVector<double> & v, double dInit = 0.0 )
		{
			x = (float32_t) v.x ;  y = (float32_t) v.y ;  z = (float32_t) v.z ;
			d = (float32_t) dInit ;
		}
		S3DVector4( const S3DVector4 & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;  d = v.d ;
		}
		const S3DVector4 & operator = ( const S3DVector4 & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;  d = v.d ;
			return	*this ;
		}
		const S3DVector4 & operator = ( const SGL3DVector<float32_t> & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
			return	*this ;
		}
		const S3DVector4 & operator = ( const SGL3DVector<double> & v )
		{
			x = (float32_t) v.x ;
			y = (float32_t) v.y ;
			z = (float32_t) v.z ;
			return	*this ;
		}
		bool operator == ( const S3DVector4 & v ) const
		{
			return	(x == v.x) & (y == v.y) & (z == v.z) & (d == v.d) ;
		}
		bool operator != ( const S3DVector4 & v ) const
		{
			return	(x != v.x) | (y != v.y) | (z != v.z) | (d != v.d) ;
		}
	} ;

	// ベクトル配列最小最大値
	void MinMaxVector4DArray
		( S3DVector4& vMin, S3DVector4& vMax,
			const S3DVector4 * pvSrc, size_t nCount ) ;
	// ベクトル配列積加算
	void AddProductedVector1DArray
		( float32_t * pfpDst,
			const float32_t * pfpSrc, float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector2DArray
		( S2DVector * pvDst,
			const S2DVector * pvSrc, float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector4DArray
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc, float32_t fpWeight, size_t nCount ) ;
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	void AddProductedVector1DArray_NEON
		( float32_t * pfpDst,
			const float32_t * pfpSrc, float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector4DArray_NEON
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc, float32_t fpWeight, size_t nCount ) ;
	#endif
	// ベクトル配列積加算（ウェイトマップ付き）
	void AddProductedVector2DArrayWithWeight
		( S2DVector * pvDst,
			const S2DVector * pvSrc,
			const float32_t * pfpWeightMap,
			float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector4DArrayWithWeight
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc,
			const float32_t * pfpWeightMap,
			float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector2DArrayWithNegWeight
		( S2DVector * pvDst,
			const S2DVector * pvSrc,
			const float32_t * pfpWeightMap, size_t nCount ) ;
	void AddProductedVector4DArrayWithNegWeight
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc,
			const float32_t * pfpWeightMap, size_t nCount ) ;
	#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
	void AddProductedVector4DArrayWithWeight_NEON
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc,
			const float32_t * pfpWeightMap,
			float32_t fpWeight, size_t nCount ) ;
	void AddProductedVector4DArrayWithNegWeight_NEON
		( S3DVector4 * pvDst,
			const S3DVector4 * pvSrc,
			const float32_t * pfpWeightMap, size_t nCount ) ;
	#endif
	void AddRevolvedVectorsWithIndexedWeight
		( S3DVector4 * pvDst,
			const S3DMatrix * pMatrics,
			const S3DVector * pvTranslate,
			const S3DVector4 * pvSrc,
			const uint32_t * pIndexedMap,
			const float32_t * pfpWeightMap, size_t nCount ) ;

	struct	S3DWVector4	: public SGL3DVector<int16_t>
	{
		int16_t	d ;

		#if	!defined(__COTOPHA__)
		S3DWVector4( void ) : d(0) {}
		#endif
		S3DWVector4( int xInit, int yInit, int zInit, int dInit = 0 )
		{
			x = (int16_t) xInit ;  y = (int16_t) yInit ;  z = (int16_t) zInit ;
			d = (int16_t) dInit ;
		}
		S3DWVector4( const SGL3DVector<int16_t> & v, int dInit = 0 )
		{
			x = v.x ;  y = v.y ;  z = v.z ;
			d = (int16_t) dInit ;
		}
		const S3DWVector4 & operator = ( const S3DWVector4 & v )
		{
			x = v.x ;  y = v.y ;  z = v.z ;  d =  v.d ;
			return	*this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 三次元変換行列
	//////////////////////////////////////////////////////////////////////////

	struct	S3DMatrix	: public SGL3DMatrix<float32_t,4>
	{
		#if	!defined(__COTOPHA__)
		S3DMatrix( void ) {}
		#endif
		S3DMatrix( double m11, double m12, double m13,
						double m21, double m22, double m23,
						double m31, double m32, double m33 )
		{
			m[0][0] = (float32_t) m11 ;  m[0][1] = (float32_t) m12 ;  m[0][2] = (float32_t) m13 ;
			m[1][0] = (float32_t) m21 ;  m[1][1] = (float32_t) m22 ;  m[1][2] = (float32_t) m23 ;
			m[2][0] = (float32_t) m31 ;  m[2][1] = (float32_t) m32 ;  m[2][2] = (float32_t) m33 ;
		}
		S3DMatrix( const SGL3DVector<float32_t> & v0,
						const SGL3DVector<float32_t> & v1,
						const SGL3DVector<float32_t> & v2 )
			: SGL3DMatrix<float32_t,4>( v0, v1, v2 ) { }
		explicit S3DMatrix( const SGL3DVector<float32_t> & v )
			: SGL3DMatrix<float32_t,4>( v ) { }
		S3DMatrix( double m11, double m22, double m33 )
		{
			m[0][0] = (float32_t) m11 ;  m[0][1] = 0.0f ;  m[0][2] = 0.0f ;
			m[1][0] = 0.0f ;  m[1][1] = (float32_t) m22 ;  m[1][2] = 0.0f ;
			m[2][0] = 0.0f ;  m[2][1] = 0.0f ;  m[2][2] = (float32_t) m33 ;
		}
		S3DMatrix( const SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> > & qt )
		{
			qt.ToMatrix( *this ) ;
		}
		S3DMatrix( const SGL3DMatrix<float32_t,4> & mat )
		{
			eslMoveMemory
				( &m[0][0], &mat.m[0][0], sizeof(float32_t) * 3 * 4 ) ;
		}
		S3DMatrix( const SGL3DMatrix<double,3> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = (float32_t) mat.m[i][0] ;
				m[i][1] = (float32_t) mat.m[i][1] ;
				m[i][2] = (float32_t) mat.m[i][2] ;
			}
		}
		const S3DMatrix & operator = ( const SGL3DMatrix<float32_t,4> & mat )
		{
			eslMoveMemory
				( &m[0][0], &mat.m[0][0], sizeof(float32_t) * 3 * 4 ) ;
			return	*this ;
		}
		const S3DMatrix & operator = ( const SGL3DMatrix<double,3> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = (float32_t) mat.m[i][0] ;
				m[i][1] = (float32_t) mat.m[i][1] ;
				m[i][2] = (float32_t) mat.m[i][2] ;
			}
			return	*this ;
		}
		const S3DMatrix & operator =
			( const SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> > & qt )
		{
			qt.ToMatrix( *this ) ;
			return	*this ;
		}
		void RevolveVectors
			( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
					size_t nCount, const S3DVector & vOffset ) const ;
		#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		void RevolveVectors_NEON
			( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
					size_t nCount, const S3DVector & vOffset ) const ;
		#endif
		void AddRevolvedVectorsWithWeight
			( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
				const float32_t * pWeight,
				size_t nCount, const S3DVector & vOffset ) const ;
		#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		void AddRevolvedVectorsWithWeight_NEON
			( S3DVector4 * pvDst, const S3DVector4 * pvSrc,
				const float32_t * pWeight,
				size_t nCount, const S3DVector & vOffset ) const ;
		#endif
	} ;

	struct	S3DDMatrix	: public SGL3DMatrix<double,3>
	{
		#if	!defined(__COTOPHA__)
		S3DDMatrix( void ) {}
		#endif
		S3DDMatrix( double m11, double m12, double m13,
						double m21, double m22, double m23,
						double m31, double m32, double m33 )
		{
			m[0][0] = m11 ;  m[0][1] = m12 ;  m[0][2] = m13 ;
			m[1][0] = m21 ;  m[1][1] = m22 ;  m[1][2] = m23 ;
			m[2][0] = m31 ;  m[2][1] = m32 ;  m[2][2] = m33 ;
		}
		S3DDMatrix( const SGL3DVector<double> & v0,
						const SGL3DVector<double> & v1,
						const SGL3DVector<double> & v2 )
			: SGL3DMatrix<double,3>( v0, v1, v2 ) { }
		explicit S3DDMatrix( const SGL3DVector<double> & v )
			: SGL3DMatrix<double,3>( v ) { }
		S3DDMatrix( double m11, double m22, double m33 )
		{
			m[0][0] = m11 ;  m[0][1] = 0.0 ;  m[0][2] = 0.0 ;
			m[1][0] = 0.0 ;  m[1][1] = m22 ;  m[1][2] = 0.0 ;
			m[2][0] = 0.0 ;  m[2][1] = 0.0 ;  m[2][2] = m33 ;
		}
		S3DDMatrix( const SGL3DQuaternion< double, SGL3DMatrix<double,3> > & qt )
		{
			qt.ToMatrix( *this ) ;
		}
		S3DDMatrix( const SGL3DMatrix<double,3> & mat )
		{
			eslMoveMemory
				( &m[0][0], &mat.m[0][0], sizeof(double) * 3 * 3 ) ;
		}
		S3DDMatrix( const SGL3DMatrix<float32_t,4> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = mat.m[i][0] ;
				m[i][1] = mat.m[i][1] ;
				m[i][2] = mat.m[i][2] ;
			}
		}
		const S3DDMatrix & operator = ( const SGL3DMatrix<double,3> & mat )
		{
			eslMoveMemory
				( &m[0][0], &mat.m[0][0], sizeof(double) * 3 * 3 ) ;
			return	*this ;
		}
		const S3DDMatrix & operator = ( const SGL3DMatrix<float32_t,4> & mat )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = mat.m[i][0] ;
				m[i][1] = mat.m[i][1] ;
				m[i][2] = mat.m[i][2] ;
			}
			return	*this ;
		}
		const S3DDMatrix & operator =
			( const SGL3DQuaternion< double, SGL3DMatrix<double,3> > & qt )
		{
			qt.ToMatrix( *this ) ;
			return	*this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// クォータニオン
	//////////////////////////////////////////////////////////////////////////

	struct	S3DQuaternion
				: public SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> >
	{
		#if	!defined(__COTOPHA__)
		S3DQuaternion( void ) {}
		#endif
		S3DQuaternion( double q0, double q1, double q2, double q3 )
		{
			q[0] = (float32_t) q0 ;
			q[1] = (float32_t) q1 ;
			q[2] = (float32_t) q2 ;
			q[3] = (float32_t) q3 ;
		}
		S3DQuaternion
			( const SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> > & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
		S3DQuaternion
			( const SGL3DQuaternion< double, SGL3DMatrix<double,3> > & qt )
		{
			q[0] = (float32_t) qt.q[0] ;
			q[1] = (float32_t) qt.q[1] ;
			q[2] = (float32_t) qt.q[2] ;
			q[3] = (float32_t) qt.q[3] ;
		}
		S3DQuaternion( const SGL3DMatrix<float32_t,4> & mat )
		{
			FromMatrix( mat ) ;
		}
		const S3DQuaternion & operator =
			( const SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> > & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
			return	*this ;
		}
		const S3DQuaternion & operator = ( const SGL3DMatrix<float32_t,4> & mat )
		{
			FromMatrix( mat ) ;
			return	*this ;
		}

	} ;

	struct	S3DDQuaternion
				: public SGL3DQuaternion< double, SGL3DMatrix<double,3> >
	{
		#if	!defined(__COTOPHA__)
		S3DDQuaternion( void ) {}
		#endif
		S3DDQuaternion( double q0, double q1, double q2, double q3 )
		{
			q[0] = q0 ;
			q[1] = q1 ;
			q[2] = q2 ;
			q[3] = q3 ;
		}
		S3DDQuaternion
			( const SGL3DQuaternion< double, SGL3DMatrix<double,3> > & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
		S3DDQuaternion
			( const SGL3DQuaternion< float32_t, SGL3DMatrix<float32_t,4> > & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
		S3DDQuaternion( const SGL3DMatrix<double,3> & mat )
		{
			FromMatrix( mat ) ;
		}
		const S3DDQuaternion & operator =
			( const SGL3DQuaternion< double, SGL3DMatrix<double,3> > & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
			return	*this ;
		}
		const S3DDQuaternion & operator = ( const SGL3DMatrix<double,3> & mat )
		{
			FromMatrix( mat ) ;
			return	*this ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ベジェ曲線
	//////////////////////////////////////////////////////////////////////////

	template <class _T, class _S = double>
		class	SGLBezierCurves : public SSystem::SArray<_T>
	{
	public:
		// 構築関数
		SGLBezierCurves( void ) { }
		SGLBezierCurves( const SGLBezierCurves<_T>& bzSrc )
			: SSystem::SArray<_T>( bzSrc ) {}
		// 配列複製
		const SGLBezierCurves<_T> & operator = ( const SSystem::SArray<_T> & bzSrc )
		{
			using namespace SSystem ;
			SArray<_T>::operator = ( bzSrc ) ;
			return	*this ;
		}
		// ベジェ曲線分割位置取得
		size_t GetDividedPosition( _S & t ) const
		{
			using namespace SSystem ;
			size_t	n = (SArray<_T>::m_nLength - 1) / 3 ;
			size_t	m ;
			for ( m = 0; m < (n - 1); m ++ )
			{
				if ( t <= (_S) (m + 1) / n )
				{
					break ;
				}
			}
			if ( n > 0 )
			{
				t = (t - (_S) m / n) * n ;
			}
			return	m ;
		}
		// 直線設定
		void SetLine( const _T p0, const _T p1, _S v0 = 1.0, _S v1 = 1.0 )
		{
			using namespace SSystem ;
			SArray<_T>::SetLength( 4 ) ;
			_T*	cp = SArray<_T>::m_ptrArray ;
			_T	d = (p1 - p0) * (_S) (1.0 / 3.0) ;
			cp[0] = p0 ;
			cp[1] = p0 + d * v0 ;
			cp[2] = p1 - d * v1 ;
			cp[3] = p1 ;
		}
		void AddLine( const _T px, _S v0, _S v1 )
		{
			using namespace SSystem ;
			ESLAssert( SArray<_T>::m_nLength >= 4 ) ;
			size_t	m = SArray<_T>::m_nLength - 1 ;
			SArray<_T>::SetLength( SArray<_T>::m_nLength + 3 ) ;
			//
			_T*	cp = SArray<_T>::m_ptrArray ;
			_T	d = (px - cp[m]) * (_S) (1.0 / 3.0) ;
			cp[m + 1] = cp[m] + d * v0 ;
			cp[m + 2] = px - d * v1 ;
			cp[m + 3] = px ;
		}
		// 曲線設定
		void SetCurveUnsmoothSpeed
			( const _T p0, const _T p1, const _T p2,
				_S v0, _S v1, _S v2, _S v3 )
		{
			using namespace SSystem ;
			SArray<_T>::SetLength( 7 ) ;
			const _S	r = (_S) (2.0 / 3.0) ;
			_T*	cp = SArray<_T>::m_ptrArray ;
			_T	a = (p2 - p0) ;
			_T	b = p1 - a * (v1 * (_S) 0.5) ;
			_T	c = p1 + a * (v2 * (_S) 0.5) ;
			cp[0] = p0 ;
			cp[3] = p1 ;
			cp[6] = p2 ;
			cp[2] = p1 - a * (v1 * (r * (_S) 0.375)) ;
			cp[4] = p1 + a * (v2 * (r * (_S) 0.375)) ;
			cp[1] = p0 + (b - p0) * (v0 * r) ;
			cp[5] = p2 + (c - p2) * (v3 * r) ;
		}
		void SetCurve
			( const _T p0, const _T p1, const _T p2,
				_S v0 = 1.0, _S v1 = 1.0, _S v2 = 1.0 )
		{
			SetCurveUnsmoothSpeed( p0, p1, p2, v0, v1, v1, v2 ) ;
		}
		void AddCurveUnsmoothSpeed2
			( const _T p1, const _T p2,
				_S v0, _S v1, _S v2, _S v3 )
		{
			using namespace SSystem ;
			ESLAssert( SArray<_T>::m_nLength >= 4 ) ;
			size_t	m = SArray<_T>::m_nLength - 1 ;
			SArray<_T>::SetLength( SArray<_T>::m_nLength + 6 ) ;
			//
			const _S	r = (_S) (2.0 / 3.0) ;
			_T*	cp = SArray<_T>::m_ptrArray ;
			_T	p0 = cp[m] ;
			_T	a = (p2 - p0) ;
			_T	b = p1 - a * (v1 * (_S) 0.5) ;
			_T	c = p1 + a * (v2 * (_S) 0.5) ;
			cp[m + 3] = p1 ;
			cp[m + 6] = p2 ;
			cp[m + 2] = p1 - a * (v1 * (r * (_S) 0.375)) ;
			cp[m + 4] = p1 + a * (v2 * (r * (_S) 0.375)) ;
			cp[m + 1] = p0 + (b - p0) * (v0 * r) ;
			cp[m + 5] = p2 + (c - p2) * (v3 * r) ;
		}
		void AddCurveUnsmoothSpeed
			( const _T px, _S v0, _S v1 )
		{
			using namespace SSystem ;
			ESLAssert( SArray<_T>::m_nLength >= 4 ) ;
			size_t	m = SArray<_T>::m_nLength - 1 ;
			SArray<_T>::SetLength( SArray<_T>::m_nLength + 3 ) ;
			//
			_T*	cp = SArray<_T>::m_ptrArray ;
			_T	d = cp[m] - cp[m - 1] ;
			_T	a = cp[m] + d * (_S) 1.5 ;
			cp[m + 1] = cp[m] + d * v0 ;
			cp[m + 2] = px + (a - px) * (v1 * (_S) (2.0 / 3.0)) ;
			cp[m + 3] = px ;
		}
		void AddCurve( const _T px, _S vx )
		{
			AddCurveUnsmoothSpeed( px, 1.0, vx ) ;
		}
		// 制御点計算
		_T & PointAt( _T & p, _S t, size_t n ) const
		{
			using namespace SSystem ;
			size_t	m = n * 3 ;
			if ( SArray<_T>::m_nLength >= m + 4 )
			{
				_S	ct = (_S) 1.0 - t ;
				_T *	cp = SArray<_T>::m_ptrArray ;
				p = cp[m] * (ct * ct * ct) ;
				p += cp[m + 1] * ((_S) 3.0 * t * ct * ct) ;
				p += cp[m + 2] * ((_S) 3.0 * t * t * ct) ;
				p += cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
		_T & PointAt( _T & p, _S t ) const
		{
			size_t	m = GetDividedPosition( t ) ;
			return	PointAt( p, t, m ) ;
		}
		_T PointAt( _S t, size_t n ) const
		{
			using namespace SSystem ;
			_T		p ;
			size_t	m = n * 3 ;
			if ( SArray<_T>::m_nLength >= m + 4 )
			{
				_S		ct = (_S) 1.0 - t ;
				_T *	cp = SArray<_T>::m_ptrArray ;
				p = cp[m] * (ct * ct * ct) ;
				p += cp[m + 1] * ((_S) 3.0 * t * ct * ct) ;
				p += cp[m + 2] * ((_S) 3.0 * t * t * ct) ;
				p += cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
		_T PointAt( _S t ) const
		{
			using namespace SSystem ;
			_T	p ;
			if ( SArray<_T>::m_nLength >= 4 )
			{
				size_t	m = GetDividedPosition( t ) * 3 ;
				_S		ct = (_S) 1.0 - t ;
				_T *	cp = SArray<_T>::m_ptrArray ;
				p = cp[m] * (ct * ct * ct) ;
				p += cp[m + 1] * ((_S) 3.0 * t * ct * ct) ;
				p += cp[m + 2] * ((_S) 3.0 * t * t * ct) ;
				p += cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
		// 接線
		void Tangent( _T & p, _S t, size_t n ) const
		{
			using namespace SSystem ;
			size_t	m = n * 3 ;
			if ( SArray<_T>::m_nLength >= m + 4 )
			{
				_S		ct = (_S) 1.0 - t ;
				_T *	cp = SArray<_T>::m_ptrArray ;
				_S		u = (_S) 1.0 - t ;
				_T	P = cp[m] * u + cp[m + 1] * t ;
				_T	Q = cp[m + 1] * u + cp[m + 2] * t ;
				_T	R = cp[m + 2] * u + cp[m + 3] * t ;
				_T	S = P * u + Q * t ;
				_T	T = Q * u + R * t ;
				p = T - S ;
			}
		}
		// ベジェ曲線分割
		void DivideBezier
			( _S t, SGLBezierCurves<_T> & bzFirst,
							SGLBezierCurves<_T> & bzLast ) const
		{
			using namespace SSystem ;
			if ( SArray<_T>::m_nLength >= 4 )
			{
				size_t	m = GetDividedPosition( t ) * 3 ;
				//
				bzFirst.SetLength( m + 4 ) ;
				bzLast.SetLength( SArray<_T>::m_nLength - m ) ;
				//
				size_t	i ;
				_T *	cp = SArray<_T>::m_ptrArray ;
				_T *	cpFirst = bzFirst.GetArray() ;
				for ( i = 0; i <= m; i ++ )
				{
					cpFirst[i] = cp[i] ;
				}
				_T *	cpLast = bzLast.GetArray() ;
				for ( i = m + 3; i < SArray<_T>::m_nLength; i ++ )
				{
					cpLast[i - m] = cp[i] ;
				}
				_S	u = 1.0 - t ;
				_T	P = cp[m] * u + cp[m + 1] * t ;
				_T	Q = cp[m + 1] * u + cp[m + 2] * t ;
				_T	R = cp[m + 2] * u + cp[m + 3] * t ;
				_T	S = P * u + Q * t ;
				_T	T = Q * u + R * t ;
				_T	U = S * u + T * t ;
				cpFirst[m + 1] = P ;
				cpFirst[m + 2] = S ;
				cpFirst[m + 3] = U ;
				cpLast[0] = U ;
				cpLast[1] = T ;
				cpLast[2] = R ;
				bzFirst.FinishArray() ;
				bzLast.FinishArray() ;
			}
		}

	} ;

}

#endif

