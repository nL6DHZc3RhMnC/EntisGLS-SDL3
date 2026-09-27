
#if	!defined(__ROSETTA_IMAGE_H__)
#define	__ROSETTA_IMAGE_H__	1

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl3d/sglh3d_stddef.h>
#include <sakuraglx/extra/sglx_image_composition.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Point クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSPointClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSPointClass, RSStructuredPointerClass )
		// 構築関数
		RSPointClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Point" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// void <init>( int x, int y )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Size クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSizeClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSizeClass, RSStructuredPointerClass )
		// 構築関数
		RSSizeClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Size" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// void <init>( int w, int w )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isEmpty()
		static RSObject * method_isEmpty
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Rect クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRectClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRectClass, RSStructuredPointerClass )
		// 構築関数
		RSRectClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Rect" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// Object - SGLImageRect 変換
		static SakuraGL::SGLImageRect * ImageRectFromObject
			( RSContext& context, SakuraGL::SGLImageRect& rect, RSObject * pObj ) ;
		static SakuraGL::SGLImageRect *
			GetThisRect( RSContext& context, RSObject * pThis ) ;

	public:
		// void <init>( int x, int y, int w, int h )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Point getPosition()
		static RSObject * method_getPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Size getSize()
		static RSObject * method_getSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPosition( int x, int y )
		static RSObject * method_setPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSize( int w, int h )
		static RSObject * method_setSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isEmpty()
		static RSObject * method_isEmpty
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void clear()
		static RSObject * method_clear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isBounds( int x, int y )
		static RSObject * method_isBounds
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean test( Rect rect )
		static RSObject * method_test
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect and( Rect rect )
		static RSObject * method_and
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect or( Rect rect )
		static RSObject * method_or
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect copy( Rect rect )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// RGBColor 構造体
	//////////////////////////////////////////////////////////////////////////

	class	RSRGBColorClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRGBColorClass, RSStructuredPointerClass )
		// 構築関数
		RSRGBColorClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RGBColor" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this 実体ポインタを取得
		static SakuraGL::SGLPalette *
			GetThisColor( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( int red, int green, int blue, int alpha )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getInt( void )
		static RSObject * method_getInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getRed( void )
		static RSObject * method_getRed
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getGreen( void )
		static RSObject * method_getGreen
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getBlue( void )
		static RSObject * method_getBlue
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getAlpha( void )
		static RSObject * method_getAlpha
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RGBColor setInt( int argb )
		static RSObject * method_setInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RGBColor copy( RGBColor color )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RGBColor mul( int alpha )
		static RSObject * method_mul
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int blendTo( int argb )
		static RSObject * method_blendTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Vector2D クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVector2DClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVector2DClass, RSStructuredPointerClass )
		// 構築関数
		RSVector2DClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Vector2D" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// Object - S2DVector 変換
		static SakuraGL::S2DVector VectorFromObject
			( RSContext& context, RSObject * pObj ) ;
		static void ObjectFromVector
			( RSContext& context, RSObject * pObj, const SakuraGL::S2DVector& v ) ;

	public:
		// void <init>( double x, double y )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean equals( Vector2D v )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector2D clone()
		static RSObject * method_clone
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D copy( Vector2D v )
		static RSObject * method_copy
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D add( Vector2D v )
		static RSObject * method_add
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D sub( Vector2D v )
		static RSObject * method_sub
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D mul( double s )
		static RSObject * method_mul
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D div( double s )
		static RSObject * method_div
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double innerProduct( Vector2D v )
		static RSObject * method_innerProduct
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double absolute()
		static RSObject * method_absolute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalize()
		static RSObject * method_normalize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Affine クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSAffineClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSAffineClass, RSStructuredPointerClass )
		// 構築関数
		RSAffineClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Affine" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// Object - SGLAffine 変換
		static SakuraGL::SGLAffine AffineFromObject
			( RSContext& context, RSObject * pObj ) ;
		static void ObjectFromAffine
			( RSContext& context, RSObject * pObj, const SakuraGL::SGLAffine& af ) ;

	public:
		// Vector2D transformVector( Vector2D v )
		static RSObject * method_transformVector
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void transformVectors( Vector2D v, int count )
		static RSObject * method_transformVectors
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void inverseOf( Affine af )
		static RSObject * method_inverseOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void mappingOf( Vector2D vDst, Vector2D vSrc )
		static RSObject * method_mappingOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D getPosition()
		static RSObject * method_getPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPosition( double x, double y )
		static RSObject * method_setPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isRotation()
		static RSObject * method_isRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRotation( double rad )
		static RSObject * method_setRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Image クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSImageClass	: public RGenericNativeObjectClass
	{
	public:
		// Image.BufferInfo クラス
		class	BufferInfoClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( BufferInfoClass, RSStructuredPointerClass )
			// 構築関数
			BufferInfoClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"BufferInfo" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;
		// Image.BuildAtlasParam クラス
		class	BuildAtlasParamClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( BuildAtlasParamClass, RSStructuredPointerClass )
			// 構築関数
			BuildAtlasParamClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"BuildAtlasParam" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSImageClass, RGenericNativeObjectClass )
		// 構築関数
		RSImageClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Image" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトの画像を取得
		static SakuraGL::SGLImageObject *
			GetThisImage( RSContext& context, RSObject* pThis ) ;
		// Object -> SGLImageObject 変換
		static SakuraGL::SGLImageObject *
			ImageFromObject( RSContext& context, RSObject* pObject ) ;
		// 新規 Image 生成
		static RSObject * NewImage
			( RSContext& context, SakuraGL::SGLImageObject * pImage ) ;
		// Image[] -> SPointerArray<SGLImageObject>
		static void GetImageArray
			( RSContext& context,
				SSystem::SPointerArray<SakuraGL::SGLImageObject>& aImages,
												RSObject * pImageArray ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getFrameCount()
		static RSObject * method_getFrameCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int[] getSequenceTable()
		static RSObject * method_getSequenceTable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getTotalTime()
		static RSObject * method_getTotalTime
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int frameFromMilliSec( long msec )
		static RSObject * method_frameFromMilliSec
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean selectFrame( int iFrame, int iSide = Image.stereoRight )
		static RSObject * method_selectFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSelectedFrame( int[] side = null )
		static RSObject * method_getSelectedFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Size getImageSize()
		static RSObject * method_getImageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getImageInfo( Image.BufferInfo imginf )
		static RSObject * method_getImageInfo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int[] getPaletteTable()
		static RSObject * method_getPaletteTable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Uint8Pointer lockBuffer
		//	( Image.BufferInfo imginf,
		//		int flags = Image.lockReadWrite, Rect rect = null )
		static RSObject * method_lockBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean unlockBuffer( int flags = Image.lockReadWrite )
		static RSObject * method_unlockBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image newReference
		//	( Rect rectClip = null,
		//		int iFrame = -1, int iSide = Image.stereoRight )
		static RSObject * method_newReference
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalizeFormat
		//	( int format = 0, int depth = 0,
		//		int flags = 0, int width, int height )
		static RSObject * method_normalizeFormat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getReferenceRectOfAtlas( Rect rectRef, int iFrame = -1 )
		static RSObject * method_getReferenceRectOfAtlas
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Image buildAtlas
		//	( Image[] images, Image.BuildAtlasParam param )
		static RSObject * method_buildAtlas
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Size estimateAtlasSize
		//	( Image[] images, boolean flagMakePOT = false, int nMargin = 1 )
		static RSObject * method_estimateAtlasSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalizeToTexture( int nFlags = 0 )
		static RSObject * method_normalizeToTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalizeToMipmapTexture( int nFlags = 0 )
		static RSObject * method_normalizeToMipmapTexture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void normalizeToRenderTarget( int nFlags = 0 )
		static RSObject * method_normalizeToRenderTarget
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createBuffer
		//	( Image.BufferInfo imginf,
		//		int nFlags = Image.bufferOnMemory,
		//		int countFrame = 1, long msecLong = 0 )
		static RSObject * method_createBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createImage
		//	( int width, int height, int format = Image.formatDefaultRGBA,
		//		int depth = 32, long nFlags = Image.bufferOnMemory,
		//		int countFrame = 1, long msecLong = 0 )
		static RSObject * method_createImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createCloneBuffer( Image src, long nFlags = Image.bufferOnMemory )
		static RSObject * method_createCloneBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseBuffer()
		static RSObject * method_releaseBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getBufferFlags()
		static RSObject * method_getBufferFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPaletteTable( int[] palette )
		static RSObject * method_setPaletteTable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSequenceTable( int[] seq )
		static RSObject * method_setSequenceTable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setImageOrigin( int x, int y )
		static RSObject * method_setImageOrigin
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setAnimationDuration( int duration )
		static RSObject * method_setAnimationDuration
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadImage( String file, String mime = null )
		static RSObject * method_loadImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readImage( InputStream is, String mime = null )
		// boolean readImage( RandomAccessFile file, String mime = null )
		static RSObject * method_readImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean saveImage
		//		( String file, String mime = null, int quality = 0x100 )
		static RSObject * method_saveImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean writeImage
		//		( RandomAccessFile file,
		//				String mime = null, int quality = 0x100 )
		static RSObject * method_writeImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillImage( int argb, Rect rect = null )
		static RSObject * method_fillImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean copyImage
		//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
		static RSObject * method_copyImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean blendImage
		//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
		static RSObject * method_blendImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean halfBlendImage
		//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
		static RSObject * method_halfBlendImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean convertImage
		//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
		static RSObject * method_convertImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean multiplyImageRGBAlpha()
		static RSObject * method_multiplyImageRGBAlpha
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean blendWithAlphaChannel
		//	( Image imgAlpha, int fxCoefficient = 0x100,
		//		int fxIntercept = 0, int x = 0, int y = 0, Rect rctSrc = null )
		static RSObject * method_blendWithAlphaChannel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean filterToneTable
		//	( Uint8Pointer pRedTone, Uint8Pointer pGreenTone,
		//		Uint8Pointer pBlueTone, Uint8Pointer pAlphaTone, Rect rect = null )
		static RSObject * method_filterToneTable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void makeToneFilter( Uint8Pointer pTone, int nValue, int nType )
		static RSObject * method_makeToneFilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean enlargeHalfImage( Image imgSrc )
		static RSObject * method_enlargeHalfImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean orthogonalRotate( Image imgSrc, int degAngle )
		static RSObject * method_orthogonalRotate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ImageComposition クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSImageCompositionClass	: public RGenericNativeObjectClass
	{
	public:
		// ImageComposition.Layer クラス
		class	LayerClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( LayerClass, RSClass )
			// 構築関数
			LayerClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Layer" ) ;
			// メンバ初期設定
			virtual void Initialize( RSContext& context ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// this オブジェクトを取得
			static SakuraGL::SGLImageComposition::Layer *
				GetThisLayer( RSContext& context, RSObject* pThis ) ;

		public:
			// void <init>()
			static RSObject * method_init
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// ImageComposition.Layer getParent()
			static RSObject * method_getParent
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// String getName()
			static RSObject * method_getName
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// void setName( String name )
			static RSObject * method_setName
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// int getType()
			static RSObject * method_getType
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// Point getPosition()
			static RSObject * method_getPosition
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// void setPosition( int x, int y )
			static RSObject * method_setPosition
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// int getBlendMode()
			static RSObject * method_getBlendMode
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// void setBlendMode( int blend )
			static RSObject * method_setBlendMode
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// boolean isVisible()
			static RSObject * method_isVisible
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// int getTransparency()
			static RSObject * method_getTransparency
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// int getChildrenCount()
			static RSObject * method_getChildrenCount
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// ImageComposition.Layer getChildAt( int index )
			static RSObject * method_getChildAt
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// Rect layerSizeOf( int nThreshold = 0 )
			static RSObject * method_layerSizeOf
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSImageCompositionClass, RGenericNativeObjectClass )
		// 構築関数
		RSImageCompositionClass
			( RSClass * pClass,
				const wchar_t * pwszClassName = L"ImageComposition" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLImageComposition *
			GetThisImageComposition( RSContext& context, RSObject* pThis ) ;
		// SGLImageComposition::Layer を取得
		static SakuraGL::SGLImageComposition::Layer *
			LayerFromObject( RSContext& context, RSObject* pObject ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadPsdFile( String file )
		static RSObject * method_loadPsdFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readPsdFile( InputStream file )
		// boolean readPsdFile( RandomAccessFile file )
		static RSObject * method_readPsdFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean savePsdFile( String file )
		static RSObject * method_savePsdFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean writePsdFile( RandomAccessFile file )
		static RSObject * method_writePsdFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCanvasInfo( int width, int height, int channels = 3 )
		static RSObject * method_setCanvasInfo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Size getCanvasSize()
		static RSObject * method_getCanvasSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getLayerCount()
		static RSObject * method_getLayerCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ImageComposition.Layer getLayerAt( int index )
		static RSObject * method_getLayerAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getLayerTreeCount()
		static RSObject * method_getLayerTreeCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ImageComposition.Layer getLayerTreeAt( int index )
		static RSObject * method_getLayerTreeAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ImageComposition.Layer appendLayer
		//		( String name, Image image = null, int x = 0, int y = 0,
		//			ImageComposition.Layer parent = null )
		static RSObject * method_appendLayer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ImageComposition.Layer appendGroupLayer
		//		( String name, int blendGroup = blendNormal, ImageComposition.Layer parent = null )
		static RSObject * method_appendGroupLayer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// PaintContext クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSPaintContextClass	: public RGenericNativeObjectClass
	{
	public:
		class	PaintParamClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PaintParamClass, RSClass )
			// 構築関数
			PaintParamClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"PaintParam" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSPaintContextClass, RGenericNativeObjectClass )
		// 構築関数
		RSPaintContextClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"PaintContext" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトの描画オブジェクトを取得
		static SakuraGL::SGLPaintContextInterface *
			GetThisPaintContext( RSContext& context, RSObject* pThis ) ;

	public:
		// Object - SGLPaintParam 変換
		static void PaintParamFromObject
			( RSContext& context, SakuraGL::SGLPaintParam& ppPaint,
				SakuraGL::SGLAffine& af,
				SSystem::SArray<SakuraGL::S2DVector>& vertices, RSObject * pObj ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getTargetImage() ;
		static RSObject * method_getTargetImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getTargetZBuffer() ;
		static RSObject * method_getTargetZBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getViewPort( Rect rctView ) ;
		static RSObject * method_getViewPort
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean attachTargetImage
		//	( Image pImage, Image pZBuffer, Rect pView = null ) ;
		static RSObject * method_attachTargetImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean detachTargetImage()
		static RSObject * method_detachTargetImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addTransformation( Affine af, int nTransparency )
		static RSObject * method_appendTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setTransformation( Affine af, int nTransparency )
		static RSObject * method_setTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean currentAffine( Affine af )
		static RSObject * method_currentAffine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int currentTransparency()
		static RSObject * method_currentTransparency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean pushTransformation()
		static RSObject * method_pushTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean popTransformation()
		static RSObject * method_popTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean resetTransformation()
		static RSObject * method_resetTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPaintFlags( long nFlags )
		static RSObject * method_setPaintFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getPaintFlags()
		static RSObject * method_getPaintFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillClearTarget( int argb = 0xFF000000, long flags = 0 )
		static RSObject * method_fillClearTarget
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawThinLine
		//	( int x0, int y0, int x1, int y1, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_drawThinLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawThinLines
		//	( Float32Pointer pLines, int nLines, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_drawThinLines
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawEllipse
		//	( double xCenter, double yCenter,
		//		double rWidth, double rHeight,
		//		int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_drawEllipse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawArc
		//	( double xCenter, double yCenter,
		//		double rWidth, double rHeight,
		//		double radFirst, double radEnd, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_drawArc
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawBezier
		//	( Vector2D pPoints, int nPoints, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_drawBezier
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillRectangle
		//	( int x, int y, int width, int height,
		//		int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_fillRectangle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillPolygon
		//	( Vector2D vertices, int count, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_fillPolygon
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillEllipse
		//	( double xCenter, double yCenter,
		//		double rWidth, double rHeight,
		//		int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_fillEllipse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean fillArc
		//	( double xCenter, double yCenter,
		//		double rWidth, double rHeight,
		//		double radFirst, double radEnd, int argb, double z = 0.0, int flags = 0 )
		static RSObject * method_fillArc
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawImage
		//	( PaintContext.PaintParam ppPaint,
		//			Image pSrcImage, Rect pSrcClip = null )
		static RSObject * method_drawImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean drawMesh
		//	( Float32Pointer pDstMesh, Float32Pointer pSrcMesh,
		//		int widthMesh, int heightMesh,
		//		PaintContext.PaintParam ppPaint,
		//		Image pSrcImage, Rect pSrcClip = null )
		static RSObject * method_drawMesh
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean flush()
		static RSObject * method_flush
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean finish()
		static RSObject * method_finish
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

