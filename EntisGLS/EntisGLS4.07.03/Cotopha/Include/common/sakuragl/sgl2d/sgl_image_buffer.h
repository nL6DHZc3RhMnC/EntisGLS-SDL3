
#if	!defined(__SAKURAGL_IMAGE_BUFFER_H__)
#define	__SAKURAGL_IMAGE_BUFFER_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像情報
	//////////////////////////////////////////////////////////////////////////

	enum	SGLStereoImageIndex
	{
		stereoImageBoth		= -1,
		stereoImageRight	= 0,
		stereoImageLeft,
	} ;
	enum	SGLImageFormatFlag
	{
		formatImageDefaultRGBA			= 0,
		formatImageRGB					= 0x00000001,	// B:G:R
		formatImageGray					= 0x00000002,
		formatImageBGR					= 0x00000003,	// R:G:B
		formatImageYUV					= 0x00000004,	// Y:U:V
		formatImageYUYV					= 0x00000104,
		formatImageYUV2					= 0x00000204,	// packed Y0, U0, Y1, V0
		formatImageYVYU					= 0x00000304,	// packed Y0, V0, Y1, U0
		formatImageUYVY					= 0x00000404,	// packed U0, Y0, V0, Y1
		formatImageIYU1					= 0x00000504,	// packed U0, Y0, Y1, V0, Y3, Y4
		formatImageIYU2					= 0x00000604,	// packed U0, Y0, V0, U1, Y1, V1
		formatImageHSB					= 0x00000006,	// B:S:H
		formatImageZ					= 0x00002005,
		formatImageDepth				= 0x00012005,
		formatImageARGB					= 0x04000001,	// B:G:R:A
		formatImageABGR					= 0x04000003,	// R:G:B:A
		formatImageFloatBGR				= 0x00004003,	// R:G:B
		formatImageFloatABGR			= 0x04004003,	// R:G:B:A
		formatImageFloatRGBA			= 0x04004003,	// R:G:B:A
		formatImageRGB_S3TC_DXT1		= 0x00001103,	// S3TC DXT1 RGB
		formatImageRGBA_S3TC_DXT1		= 0x04001103,	// S3TC DXT1 RGBA
		formatImageColorSpaceMask		= 0x000000FF,
		formatImageTypeMask				= 0x0000FFFF,
		formatImageFlagS3TC				= 0x00001000,
		formatImageFlagFloat			= 0x00004000,
		formatImageFlagDepth			= 0x00010000,
		formatImageFlagPalette			= 0x01000000,
		formatImageFlagClipping			= 0x02000000,
		formatImageFlagAlpha			= 0x04000000,
		formatImageFlagNoProductOfAlpha	= 0x08000000,
		formatImageFlagSideBySide		= 0x10000000,
	} ;
	struct	SGLImageInfo
	{
		uint32_t	format ;		// ピクセルフォーマット
		uint32_t	depth ;			// ビット深度 [bits/pixel]
		uint32_t	width ;			// 幅 [pixels]
		uint32_t	height ;		// 高さ [pixels]
		SGLPoint	ptOrigin ;		// （描画の際の）原点
		uint32_t	colorClip ;		// 透明色
		int32_t		pitchPixel ;	// 画像バッファのピクセルピッチ [bytes]
		int32_t		pitchLine ;		// 画像バッファのラインピッチ [bytes]
		uint32_t	reserved ;

		#if	!defined(__COTOPHA__)
		SGLImageInfo( void )
			: format(0), depth(0), width(0), height(0),
				colorClip(0), pitchPixel(0), pitchLine(0), reserved(0) {}
		#endif
		SGLImageInfo( const SGLImageInfo & inf )
			: format(inf.format), depth(inf.depth),
				width(inf.width), height(inf.height),
				ptOrigin(inf.ptOrigin), colorClip(inf.colorClip),
				pitchPixel(inf.pitchPixel), pitchLine(inf.pitchLine) {}
		const SGLImageInfo & operator = ( const SGLImageInfo & inf )
		{
			format = inf.format ;
			depth = inf.depth ;
			width = inf.width ;
			height = inf.height ;
			ptOrigin = inf.ptOrigin ;
			colorClip = inf.colorClip ;
			pitchPixel = inf.pitchPixel ;
			pitchLine = inf.pitchLine ;
			return	*this ;
		}
		SGLSize GetImageSize( void ) const
		{
			return	SGLSize( width, height ) ;
		}
		SGLImageRect GetImageRect( void ) const
		{
			return	SGLImageRect( 0, 0, width, height ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像バッファ
	//////////////////////////////////////////////////////////////////////////

	enum	SGLImageBufferObjectType
	{
		imageObjectEntisGLS3Texture			= 0x00000001,	// SGLImageE3DTexture
		imageObjectEntisGLS4Image			= 0x00000002,	// EntisGLS4ImageBufferInterface
		imageObjectEntisGLS4NoShadeMaterial	= 0x00000003,	// SGLImageNoShadeMaterialInterface
		imageObjectEntisGLS4Temporary		= 0x00000004,	// SGLImageObjectBufferInterface
		imageObjectEntisGLS4Memory			= 0x00000010,	// SGLImageSystemMemory
		imageObjectWin32DIBitmap			= 0x00000100,	// SGLImageWin32DIBitmap
		imageObjectGLTexture				= 0x01000000,	// SGLOpenGLTextureBuffer
		imageObjectCUDAMemory				= 0x02000000,
		imageObjectDD1Surface				= 0x03000001,
		imageObjectDD7Surface				= 0x03000007,
		imageObjectD3D9Surface				= 0x03000009,
		imageObjectD3D9Texture				= 0x04000009,
		imageObjectAppExtension				= 0xFF000000,
	} ;

	struct SGLImageBuffer ;

	class	SGLImageBufferInterface	: public ESLObject
	{
	public:
		SGLImageBufferInterface *	m_ptrNext ;
		uint32_t					m_typeObject ;	// enum SGLImageBufferObjectType
		uint32_t					m_nReserved ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageBufferInterface, ESLObject )
		#if	!defined(__COTOPHA__)
		// 構築関数
		SGLImageBufferInterface( void )
			: m_ptrNext(NULL), m_typeObject(0), m_nReserved(0) {}
		#endif
		// 消滅関数
		virtual ~SGLImageBufferInterface( void ) {}
		// クリア通知
		virtual SGLError ClearBuffer
			( SGLImageBuffer * pImageBuf, SGLPalette pxClear ) ;
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) = 0 ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) = 0 ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) = 0 ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) = 0 ;
		// 画像バッファの再確保通知
		virtual bool OnImageReBuffered( SGLImageBuffer * pImageBuf ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) = 0 ;
	} ;

	struct	SGLImageBuffer	: public SGLImageInfo
	{
		SGLPalette *				ptrPalette ;
		uint8_t *					ptrBuffer ;
		uint64_t					flagsBuffer ;	// complex of enum SGLBufferCreationFlag
		SGLImageBufferInterface *	ptrInterface ;
		wchar_t *					pwszIdentity ;	// for debugging
		SGLImageBuffer *			ptrRefOriginal ;
		SGLImageRect				rctRefOriginal ;
		atomic_int_t				countRef ;
		size_t						nFrameCount ;
		SGLSize						sizeFrame ;

		#if	defined(__DEBUG__)
		SGLImageBuffer *			ptrPrevBufChain ;
		SGLImageBuffer *			ptrNextBufChain ;
		#endif

		static ESL_DLL_EXPORT SGLImageBuffer *		ptrFirstBufChain ;

		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )

		// 構築関数
		SGLImageBuffer( void )
			: ptrPalette(NULL), ptrBuffer(NULL),
				flagsBuffer(0), ptrInterface(NULL),
				pwszIdentity(NULL),
				ptrRefOriginal(NULL), countRef(0), nFrameCount(1)
			#if	defined(__DEBUG__)
				,ptrPrevBufChain(NULL), ptrNextBufChain(NULL) {}
			#else
				{}
			#endif
		SGLImageBuffer( const SGLImageBuffer & buf )
			: SGLImageInfo( buf ),
				ptrPalette(buf.ptrPalette), ptrBuffer(buf.ptrBuffer),
				flagsBuffer(buf.flagsBuffer),
				ptrInterface(buf.ptrInterface),
				pwszIdentity(NULL),
				ptrRefOriginal(buf.ptrRefOriginal),
				rctRefOriginal(buf.rctRefOriginal),
				countRef(0),
				nFrameCount(buf.nFrameCount), sizeFrame(buf.sizeFrame)
			#if	defined(__DEBUG__)
				,ptrPrevBufChain(NULL), ptrNextBufChain(NULL) {}
			#else
				{}
			#endif
		SGLImageBuffer( const SGLImageInfo & inf )
			: SGLImageInfo( inf ),
				ptrPalette(NULL), ptrBuffer(NULL),
				flagsBuffer(0), ptrInterface(NULL),
				pwszIdentity(NULL),
				ptrRefOriginal(NULL), countRef(0), nFrameCount(1)
			#if	defined(__DEBUG__)
				,ptrPrevBufChain(NULL), ptrNextBufChain(NULL) {}
			#else
				{}
			#endif
		#if	defined(__DEBUG__)
		// 消滅関数
		~SGLImageBuffer( void )
		{
			DetachFromChain() ;
		}
		// チェーンに追加
		void AddIntoChainFirst( void ) ;
		// チェーンから分離
		void DetachFromChain( void ) ;
		// 画像バッファチェーンをデバッグ出力へダンプ
		static void DumpAllBufferChain( void ) ;
		// 画像バッファチェーンを切る
		static SGLImageBuffer * SaveBufferChain( void ) ;
		// 画像バッファチェーンを復元する
		static void RestoreBufferChain( SGLImageBuffer * pImageBuf ) ;
		#endif
		// 代入
		const SGLImageBuffer & operator = ( const SGLImageBuffer & buf )
		{
			SGLImageInfo::operator = ( buf ) ;
			ptrPalette = buf.ptrPalette ;
			ptrBuffer = buf.ptrBuffer ;
			flagsBuffer = buf.flagsBuffer ;
			ptrRefOriginal = buf.ptrRefOriginal ;
			rctRefOriginal = buf.rctRefOriginal ;
			return	*this ;
		}
		// バッファ矩形参照
		bool GetClippedBuffer
			( const SGLImageBuffer& imgbuf, const SGLImageRect & rect ) ;
		// 画像バッファのクリア通知
		SGLError NotifyClearImageObject( SGLPalette pxClear ) ;
		// 画像バッファの更新通知
		SGLError UpdateImageObject( const SGLImageRect * pRect = NULL ) ;
		// 画像バッファの更新を画像オブジェクトへ反映
		SGLError CommitImageObject( void ) ;
		// 画像オブジェクトの更新を画像バッファへ反映
		SGLError ReflectImageObject( const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化の設定
		SGLError MakeMipmap( void ) ;
		// 画像オブジェクト取得 (SGLImageBufferObjectType)
		SGLImageBufferInterface * GetImageObject
			( uint32_t idType,
				SGLImageBuffer*& pImageRef,
				SGLImageRect& rectRef, bool fNoRefImage ) const ;
		// 画像オブジェクト追加
		void AddImageObject( SGLImageBufferInterface * pObject ) ;
		// 画像オブジェクト分離
		bool DetachImageObject( SGLImageBufferInterface * pObject ) ;
		// 画像オブジェクト削除
		void DeleteImageObject( uint32_t idType ) ;
		void DeleteAllImageObject( void ) ;
		// 画像バッファの更新通知・削除
		void NotifyReBufferedImage( void ) ;
		// オブジェクト削除通知（関連オブジェクトの削除）
		void NotifyObjectDestroy( ESLObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ただの画像メモリ
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageSystemMemory	: public SGLImageBufferInterface
	{
	public:
		SGLImageInfo				m_imginf ;
		SSystem::SArray<uint8_t>	m_bufImage ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageSystemMemory, SGLImageBufferInterface )
		// 構築関数
		SGLImageSystemMemory( void ) ;
		// 消滅関数
		virtual ~SGLImageSystemMemory( void ) ;

	public:
		// クリア通知
		virtual SGLError ClearBuffer
			( SGLImageBuffer * pImageBuf, SGLPalette pxClear ) ;
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;

	public:
		// SGLImageObject からメモリ生成／取得
		static uint8_t * CommitMemoryOf
			( SGLImageBuffer * pImageBuf, SGLImageInfo*& pRefImage ) ;
		// SGLImageBuffer からメモリ取得
		static uint8_t * GetMemoryOf
			( SGLImageBuffer * pImageBuf, SGLImageInfo*& pRefImage ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像バッファ管理関数群
	//////////////////////////////////////////////////////////////////////////

	// 2^n サイズ正規化
	uint32_t sglNormalizeScalePowerBy2( uint32_t nSize ) ;

	// デフォルト RGBA 形式の設定
	extern	ESL_DLL_EXPORT uint32_t	g_defaultImageFormat ;
	extern	ESL_DLL_EXPORT uint32_t	g_defaultImageDepth ;

	SGLError sglSetDefaultImageFormat( uint32_t format, uint32_t depth ) ;

	// 画像バッファに使用されたメモリ情報
	extern	ESL_DLL_EXPORT atomic_int_t	g_countUsedImageBuffer ;
	extern	ESL_DLL_EXPORT atomic_int_t	g_bytesUsedImageBuffer ;
	extern	ESL_DLL_EXPORT atomic_int_t	g_bytesMaxUsedImageBuffer ;

	// 画像バッファメモリ関数
	uint8_t * sglAllocateMemory( size_t nBytes ) ;
	wchar_t * sglAllocateStringMemory( const wchar_t * pwszString ) ;
	void sglFreeMemory( uint8_t * pbytMem ) ;
	void sglAddRefMemory( uint8_t * pbytMem ) ;

	// 画像バッファ生成
	enum	SGLBufferCreationFlag
	{
		flagImageNormalDepth			= 0x00000001,
		flagImageNormalSize				= 0x00000002,
		flagImageNormalPitch			= 0x00000004,
		flagImageNoBuffer				= 0x00000008,
		flagImageNoReadBuffer			= 0x00000010,
		flagImageNoWriteBuffer			= 0x00000020,
		flagImageMipmap					= 0x00000100,
		flagImageCubemap				= 0x00000200,
		flagImageCubeIndexMask			= 0x00007000,
		flagImageCubeIndexShifter		= 12,
		flagImageTexture3D				= 0x00000400,
		flagImageTextureMultisample		= 0x00000800,
		flagImageRenderBufferStorage	= 0x00008000,
		flagImageCompressedTexture		= 0x00010000,
		flagImageCompressionFormatMask	= 0x7F000000,
		flagImageCompressionFormatShifter	= 24,
		flagImageNeedSampleNoSmooth		= 0x00020000,
		flagImageNeedSampleTiling		= 0x00040000,
		flagImageTextureArray			= 0x00080000,
		flagImageStorage				= 0x00100000,
		flagImageLayerIndexShifter		= 32,
	} ;
	static constexpr uint64_t	flagImageLayerIndexMask	= 0x0000FFFF00000000 ;

	SGLImageBuffer * sglCreateImageBuffer
		( const SGLImageInfo& imginf, uint64_t nFlags = 0 ) ;
	// 画像参照生成
	SGLImageBuffer * sglCreateReferenceImageBuffer
		( SGLImageBuffer * pImage,
			const SGLImageRect * pRect,
			int iFrame = 0, int iSide = stereoImageRight ) ;
	// 画像参照先変更
	SGLError sglMakeReferenceImageBuffer
		( SGLImageBuffer * pImage,
			SGLImageBuffer * pRefImage,
			const SGLImageRect * pRect,
			int iFrame = 0, int iSide = stereoImageRight ) ;
	// 画像バッファ正規化
	SGLError sglNormalizeImageBuffer
		( SGLImageBuffer * pImage, uint64_t nFlags ) ;
	// 参照カウンタ加算
	SGLError sglAddReferenceImageBuffer( SGLImageBuffer * pImage ) ;
	// 参照カウンタ減算／画像バッファ削除
	SGLError sglReleaseImageBuffer( SGLImageBuffer * pImage ) ;
	// 画像バッファ（内部）の参照カウンタを加算しポインタを取得する（sglFreeMemory で解放）
	uint8_t * LockImageBuffer( SGLImageBuffer * pImage ) ;
	// 画像バッファ識別子設定
	void SetImageBufferIdentity( SGLImageBuffer * pImage, const wchar_t * pwszID ) ;

	// 交差矩形参照
	SGLError sglGetImageBufferIntersection
		( SGLImageBuffer& imgIsDst,
			SGLImageBuffer& imgIsSrc,
			const SGLImageBuffer& imgDstBuf,
			const SGLImageBuffer& imgSrcBuf,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファフィル
	__native SGLError sglFillImageBuffer
		( const SGLImageBuffer& imgbuf,
			const SGLPalette& pxcmp, const SGLImageRect * pRect = NULL ) ;
	// 画像バッファ複製
	__native SGLError sglCopyImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファ描画（ARGB 標準合成）
	__native SGLError sglBlendImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファ描画（ARGB 標準合成逆順）
	__native SGLError sglBlendBackImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファ加算（ARGB 加算合成）
	__native SGLError sglAdditionalBlendImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファ乗算（ARGBxARGB 乗算合成）
	__native SGLError sglMultiplierBlendImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;
	// 画像バッファ乗算（ARGBxGray 乗算合成）
	__native SGLError sglMultiplierARGBxGrayBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc,
			int xPos = 0, int yPos = 0,
			const SGLImageRect * pSrcRect = NULL ) ;

	// 画像 1/2 拡大 (RGB32, RGBA32, Gray8 対応)
	__native SGLError sglEnlargeHalfImageBuffer
		( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc ) ;
	// 90度単位回転（反時計回り）
	__native SGLError sglOrthogonalRotateImageBuffer
		( const SGLImageBuffer& imgDst,
			const SGLImageBuffer& imgSrc, int degRotateAngle ) ;

	// ビットマスク画像を 1/8 縮小して Grayscale 画像に変換する
	__native SGLError sglMultisampleBitmaskToGrayscale
		( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc ) ;

}

#endif
