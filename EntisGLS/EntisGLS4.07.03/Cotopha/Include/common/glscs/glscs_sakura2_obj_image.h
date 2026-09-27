
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_IMAGE_H__)
#define	__GLSCS_SAKURA2_OBJECT_IMAGE_H__

#include <sakuragl/sgl2d/sgl_image_buf_object.h>

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 画像オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	ECSImageObject	: public ECSVolatileObject,
								public SakuraGL::SGLMultiImage
	{
	protected:
		enum	ProviderType
		{
			providerNoData,
			providerRefImage,
			providerLoadImage,
			providerCreateBuffer,
		} ;
		struct	REF_IMAGE_PARAM
		{
			DWORD				dwRefImage ;
			DWORD				iFrame ;
			DWORD				iSide ;
			SGL::SGLImageRect	rctClip ;
		} ;
		struct	CREATE_BUFFER_PARAM
		{
			DWORD			dwFlags ;
			DWORD			dwFrames ;
			UINT64			nTimeLong ;
		} ;
		ProviderType		m_typeProvider ;
		ssize_t				m_iSelFrame ;
		int					m_iSelSide ;
		SSystem::SString	m_strFilePath ;
		SSystem::SString	m_strMIMEType ;
		uint32_t			m_nLimitFrames ;
		REF_IMAGE_PARAM		m_ripRef ;
		CREATE_BUFFER_PARAM	m_cbpCreate ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( ECSImageObject,
				ECSVolatileObject, SakuraGL::SGLMultiImage )
		// 構築関数
		ECSImageObject( void ) ;
		ECSImageObject
			( const ECSImageObject& img,
				const SGL::SGLImageRect * pClip = NULL,
				int iSide = SGL::stereoImageBoth ) ;

	public:
		// フレーム選択
		virtual SGL::SGLError SelectFrame
			( size_t iFrame, int iSide = SGL::stereoImageRight ) ;
		// 画像バッファへの参照生成
		virtual SGL::SGLImageObject * NewReference
			( const SGL::SGLImageRect * pClip = NULL,
				ssize_t iFrame = -1, int iSide = SGL::stereoImageRight ) ;
		// テクスチャのための正規化
		virtual SGL::SGLError NormalizeToTexture( uint32_t nFlags ) ;
		virtual SGL::SGLError NormalizeToMipmapTexture( uint32_t nFlags ) ;
		// レンダリング・ターゲットのための正規化
		virtual SGL::SGLError NormalizeToRenderTarget( uint32_t nFlags ) ;
		// 画像バッファ生成
		virtual SGL::SGLError CreateBuffer
			( const SGL::SGLImageInfo& imginf,
				int nFlags = SGL::SGLImageObject::bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		// 保有リソース解放
		virtual void ReleaseBuffer( void ) ;

	public:
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		virtual SGLImageObject * NewAnimationReference
			( SakuraGL::SGLImageRect* pFrameRects, size_t nRectsCount ) ;

	public:
		// 画像ファイルを読み込む
		virtual SGL::SGLError LoadImageFile
			( SSystem::SEnvironmentInterface * pEnv,
				const wchar_t * pszFilePath,
				const wchar_t * pszMIME, size_t nLimitFrames ) ;
		virtual SGL::SGLError ReadImageFile
			( SSystem::SFileInterface * pFile,
				const wchar_t * pszMIME, size_t nLimitFrames ) ;

	public:
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後処理
		virtual SError CommitAfterLoad
			( VirtualMachine * vm, Context * context ) ;
	} ;

}

// new SakuraGL::Image
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_Image) ;

// size_t GetFrameCount( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetFrameCount) ;
// size_t GetSequenceLength( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetSequenceLength) ;
// size_t GetSequenceTable( size_t * pSeq, size_t nCount ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetSequenceTable) ;
// uint64_t GetTotalTime( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetTotalTime) ;
// size_t FrameFromMilliSec( uint64_t msec ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_FrameFromMilliSec) ;
// SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_SelectFrame) ;
// size_t GetSelectedFrame( int * pSide = NULL ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetSelectedFrame) ;
// SGLError GetImageInfo( SGLImageInfo & imginf ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetImageInfo) ;
// size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetPaletteTable) ;
// uint8_t * LockBuffer( SGLImageInfo & imginf,
//	int flags = bufferReadWrite, const SGLImageRect * pRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_LockBuffer) ;
// SGLError FlushBuffer( int flags = bufferReadWrite ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_FlushBuffer) ;
// SGLError UnlockBuffer( int flags = bufferReadWrite ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_UnlockBuffer) ;
// SGLError ReadFrameBuffer
//	( SGLImageInfo & imginf, uint8_t * ptrBuffer,
//			size_t iFrame = 0, int iSide = stereoImageRight ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_ReadFrameBuffer) ;
// Image * NewReference( const SGLImageRect * pClip = NULL,
//			ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NewReference) ;
// SGLError NormalizeToTexture( uint32_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToTexture) ;
// SGLError NormalizeToMipmapTexture( uint32_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToMipmapTexture) ;
// SGLError NormalizeToRenderTarget( uint32_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToRenderTarget) ;
// SGLError DenormalizeForTexture( uint32_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_DenormalizeForTexture) ;
// SGLError CreateBuffer( const SGLImageInfo& imginf,
//	int nFlags = bufferOnMemory, size_t countFrame = 1, uint64_t msecLong = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_CreateBuffer) ;
// void ReleaseBuffer( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_ReleaseBuffer) ;
// int GetBufferFlags( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_GetBufferFlags) ;
// size_t SetPaletteTable( const SGLPalette * pPalette, size_t nCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_SetPaletteTable) ;
// void SetSequenceTable( const size_t * pSeq, size_t nCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_SetSequenceTable) ;
// void SetImageOrigin( int x, int y ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_SetImageOrigin) ;
// void SetAnimationDuration( int nDuration ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_SetAnimationDuration) ;
// SGLError NormalizeFormat( uint32_t format = 0, uint32_t depth = 0,
//		uint32_t nFlags = 0, uint32_t width = 0, uint32_t height = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NormalizeFormat) ;
// Image * NewAnimationReference
//		( SGLImageRect* pFrameRects, size_t nRectsCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_NewAnimationReference) ;
// SGLError LoadImage( const wchar_t * pszFilePath, const wchar_t * pszMIME = NULL, size_t nLimitFrames = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_LoadImage) ;
// SGLError ReadImage( File * file, const wchar_t * pszMIME = NULL, size_t nLimitFrames = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_ReadImage) ;
// static bool IsLoadableFileExtension( const wchar_t * pszExt ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_IsLoadableFileExtension) ;
// static bool IsLoadableMIMEType( const wchar_t * pszMIME ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_IsLoadableMIMEType) ;
// native void FlushImageObject( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Image_FlushImageObject) ;



// 画像バッファ関数
// SGLError sglFillImageBuffer
//	( const SGLImageBuffer& imgbuf,
//		const SGLPalette& pxcmp, const SGLImageRect * pRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglFillImageBuffer) ;

// SGLError sglCopyImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglCopyImageBuffer) ;

// SGLError sglBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglBlendImageBuffer) ;

// SGLError sglBlendBackImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglBlendBackImageBuffer) ;

// SGLError sglAdditionalBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglAdditionalBlendImageBuffer) ;

// SGLError sglMultiplierBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglMultiplierBlendImageBuffer) ;

// SGLError sglEnlargeHalfImageBuffer
//	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglEnlargeHalfImageBuffer) ;

// SGLError sglOrthogonalRotateImageBuffer
//	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc, int degRotateAngle ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglOrthogonalRotateImageBuffer) ;

// SGLError sglConvertImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglConvertImageBuffer) ;

// SGLError sglMultiplyImageRGBAlpha( const SGLImageBuffer& imgbuf ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglMultiplyImageRGBAlpha) ;

// SGLError sglMakeGrayImageFromRGB( const SGLImageBuffer& imgbuf ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglMakeGrayImageFromRGB) ;

// SGLError sglPutImageChannelTo
//	( const SGLImageBuffer& imgDst, int iDstChannel,
//		const SGLImageBuffer& imgSrc, int iSrcChannel,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglPutImageChannelTo) ;

// SGLError sglApplyToneImageFilter
//	( const SGLImageBuffer& imgbuf,
//		const SGLImageRect * pRect,
//		const uint8_t * pRedTone, const uint8_t * pGreenTone,
//		const uint8_t * pBlueTone, const uint8_t * pAlphaTone ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglApplyToneImageFilter) ;

// SGLError sglBlendWithAlphaChannel
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgAlpha,
//		int32_t fxAlphaCoefficient = 0x100,
//		int32_t fxAlphaIntercept = 0,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglBlendWithAlphaChannel) ;

// SGLError sglGaussianBlur
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		const SGLImageBuffer& imgTemp,
//		float32_t fpGaussianValue, size_t nBlurWidth ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_sglGaussianBlur) ;

#endif
