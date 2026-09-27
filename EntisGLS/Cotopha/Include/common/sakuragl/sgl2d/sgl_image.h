
#if	!defined(__SAKURAGL_IMAGE_H__)
#define	__SAKURAGL_IMAGE_H__

namespace	SakuraGL
{
	#if	defined(__COTOPHA__)
	class	native Image ;
	class	SGLImageObject ;
	#else
	class	SGLImageObject ;
	typedef	SGLImageObject	Image ;
	#endif
}

#include <sakuragl/sgl2d/sgl_image_encoder.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 抽象画像オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageObject	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageObject, SSystem::SObject )
		// 構築関数
		SGLImageObject( void ) {}
		// 構築関数（ダミー）
		SGLImageObject( const SGLImageObject& image ) {}
		// フレーム数取得
		virtual size_t GetFrameCount( void ) const ;
		// アニメーション・シーケンス全長取得
		virtual size_t GetSequenceLength( void ) const ;
		// アニメーション・シーケンス取得
		virtual size_t GetSequenceTable( uint32_t * pSeq, size_t nCount ) const ;
		// アニメーション全長（時間）取得 [msec]
		virtual uint64_t GetTotalTime( void ) const ;
		// 時間からフレーム番号へ変換
		virtual size_t FrameFromMilliSec( uint64_t msec ) ;
		// フレーム選択
		virtual SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
		// 選択中フレーム取得
		virtual size_t GetSelectedFrame( int * pSide = nullptr ) const ;
		// 画像情報取得
		virtual SGLError GetImageInfo( SGLImageInfo & imginf ) const ;
		// 画像サイズ取得
		SGLSize GetImageSize( void ) const ;
		uint32_t GetImageWidth( void ) const ;
		uint32_t GetImageHeight( void ) const ;
		// 画像フォーマット取得
		uint32_t GetImageFormat( void ) const ;
		// パレット・テーブル取得
		virtual size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
		// 画像バッファ取得
		enum	LockBufferMethod
		{
			lockRead		= 0x01,
			lockWrite		= 0x02,
			lockReadWrite	= 0x03,
		} ;
		virtual uint8_t * LockBuffer
			( SGLImageInfo & imginf,
				int flags = lockReadWrite,
				const SGLImageRect * pRect = nullptr ) ;
		virtual SGLError FlushBuffer( int flags = lockReadWrite ) ;
		virtual SGLError UnlockBuffer( int flags = lockReadWrite ) ;
		// 画像バッファ読み出し
		virtual SGLError ReadFrameBuffer
			( const SGLImageInfo & imginf, uint8_t * ptrBuffer,
					size_t iFrame = 0, int iSide = stereoImageRight ) ;

	public:
		// CubeMap のフレーム番号
		enum	CubeMapFrameIndex
		{
			cubemapNegativeX,			// Left
			cubemapPositiveY,			// Down
			cubemapNegativeZ,			// Front
			cubemapPositiveX,			// Right
			cubemapNegativeY,			// Up
			cubemapPositiveZ,			// Back
		} ;
		// バッファフラグ
		enum	BufferTypeFlag
		{
			bufferNoWritable			= 0x00000001,	// 書き出し不可ヒント（未使用）
			bufferNoReadable			= 0x00000002,	// 読み出し不可ヒント（未使用）
			bufferNonPowerOf2			= 0x00000008,	// VRAM 上のサイズは２の累乗に限らない
			bufferForTexture			= 0x00000010,	// テクスチャソース使用ヒント（未使用）
			bufferForMipmapTexture		= 0x00000020,	// VRAM 上ではミップマップ化
			bufferForRenderTarget		= 0x00000040,	// レンダリング対象ヒント（未使用）
			bufferCubeMapTexture		= 0x00000080,	// キューブマッピング用
			bufferDeviceRenderBuffer	= 0x00000200,	// レンダリング専用バッファ
			bufferMultisample			= 0x00000400,	// マルチサンプリング（レンダリング対象）
			bufferCompressedTexture		= 0x00000800,	// 可能なら VRAM 上は圧縮形式
			bufferCompressionFormatMask	= 0x7F000000,
			bufferCompressionFormatShifter	= 24,
			bufferTexture3D				= 0x00001000,	// VRAM 上では 3D テクスチャ
			bufferTextureArray			= 0x00002000,	// VRAM 上では テクスチャ配列 ※bufferTexture3D との組み合わせは未対応
			bufferTextureStorage		= 0x00004000,	// 不変テクスチャ（TexStorage）
			bufferSampleNoSmooth		= 0x00010000,	// デフォルトで補完無効
			bufferSampleTiling			= 0x00020000,	// デフォルトでタイリング (WRAP)
			bufferOnMemory				= 0x00000000,	// RAM 上にバッファ保持
			bufferOnDeviceOnly			= 0x00000100,	// VRAM 上のみ
			bufferRenderNonTextureFlags	=
					bufferDeviceRenderBuffer
					| bufferMultisample,	// 描画先にのみ使えるフラグ集合
		} ;
		// 画像バッファへの参照生成
		virtual SGLImageObject * NewReference
			( const SGLImageRect * pClip = nullptr,
					ssize_t iFrame = -1, int iSide = stereoImageRight ) = 0 ;
		// テクスチャのための正規化
		virtual SGLError NormalizeToTexture( uint32_t nFlags = 0 ) ;
		virtual SGLError NormalizeToMipmapTexture( uint32_t nFlags = 0 ) ;
		// レンダリング・ターゲットのための正規化
		virtual SGLError NormalizeToRenderTarget( uint32_t nFlags = 0 ) ;
		// テクスチャのための正規化フラグを除去する
		virtual SGLError DenormalizeForTexture( uint32_t nFlags ) ;
		// 画像バッファ生成
		virtual SGLError CreateBuffer
			( const SGLImageInfo& imginf,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) = 0 ;
		SGLError CreateImage
			( uint32_t width, uint32_t height,
				uint32_t format = formatImageDefaultRGBA, uint32_t depth = 32,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		SGLError CreateCloneImage
			( SGLImageObject& imgObj, uint64_t nFlags = bufferOnMemory ) ;
		SGLError CreateCloneBuffer
			( const SGLImageBuffer& imgbuf, uint64_t nFlags = bufferOnMemory ) ;
		// 保有リソース解放
		virtual void ReleaseBuffer( void ) ;
		// バッファフラグ取得
		virtual uint64_t GetBufferFlags( void ) = 0 ;
		// パレット・テーブル設定
		virtual size_t SetPaletteTable( const SGLPalette * pPalette, size_t nCount ) ;
		// アニメーション・シーケンス・テーブルの設定
		virtual void SetSequenceTable( const uint32_t * pSeq, size_t nCount ) ;
		// 中心座標情報設定
		virtual void SetImageOrigin( int x, int y ) = 0 ;
		// アニメーション全長時間設定
		virtual void SetAnimationDuration( uint64_t nDuration ) = 0 ;

	public:
		// フォーマット正規化フラグ
		enum	NormalizeFormatFlag
		{
			formatMergeAnimation	= 0x00000001,	// アニメーション画像を1枚にまとめる
			formatCastSize			= 0x00000002,	// サイズ変更（縮小）
			formatCastSizeBottom	= 0x00000004,
			formatStraightPixel		= 0x00000010,	// データはそのままフォーマット情報のみ変更
			formatNoDithering		= 0x00010000,	// 減色処理時にディザリングしない
		} ;
		// 画像フォーマット正規化
		virtual SGLError NormalizeFormat
			( uint32_t format = 0, uint32_t depth = 0,
				uint32_t nFlags = 0,
				uint32_t width = 0, uint32_t height = 0 ) ;
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		virtual SGLImageObject * NewAnimationReference
			( SGLImageRect* pFrameRects, size_t nRectsCount ) ;
		// アトラス化された画像の参照矩形を取得
		virtual SGLError GetReferenceRectOfAtlas
			( SGLImageRect& rect, ssize_t iFrame = -1 ) const = 0 ;

	public:
		// アトラス化処理
		enum	BuildAtlasFlag
		{
			atlasFlagExpandPOT	= 0x0001,
		} ;
		struct	BuildAtlasParam
		{
			uint32_t	nFlags ;		// enum BuildAtlasFlag
			uint32_t	nMargin ;
			SGLSize		sizeAtlasMin ;
			SGLSize		sizeAtlasMax ;

			BuildAtlasParam( void )
				: nFlags(0), nMargin(1),
					sizeAtlasMin(16,16),
					sizeAtlasMax(2048,2048) { }
		} ;
		static SGLImageObject * BuildAtlas
			( SGLImageObject*const* ppImages,
				size_t nImageCount,
				const BuildAtlasParam& param,
				size_t * pUsedCount = nullptr,
				size_t * pUsedIndexes = nullptr ) ;
		static size_t EstimateAtlasSize
			( SGLSize& sizeEstimated,
				SGLImageObject*const* ppImages, size_t nImageCount,
				bool flagMakePOT = false, uint32_t nMargin = 1,
				uint32_t nSizeLimit = 0, size_t * pUsedIndexes = nullptr ) ;
	protected:
		struct	ImageFrame
		{
			SGLImageObject *	pImage ;
			size_t				iFrame ;
		} ;
		static void CollectImageFrames
			( SSystem::SArray<ImageFrame>& aImageFrames,
				SSystem::SArray<SGLSize>& aImageSizes,
				SGLImageObject*const* ppImages, size_t nImageCount ) ;

	public:
		// 画像読み込み
		SGLError LoadImage
			( const wchar_t * pszFilePath,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		SGLError ReadImage
			( SSystem::SFileInterface * file,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		// 画像書き出し
		SGLError SaveImage
			( const wchar_t * pszFilePath, const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = nullptr ) ;
		SGLError WriteImage
			( SSystem::SFileInterface * file,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = nullptr ) ;

	public:
		// ピクセル設定
		SGLError SetPixel( int xPos, int yPos, const SGLPalette& pxcmp ) ;
		SGLError SetPixelRGBA( int xPos, int yPos, const SGLPalette& pxRGBA ) ;
		// ピクセル取得
		SGLError GetPixel( SGLPalette& pxcmp, int xPos, int yPos ) ;
		SGLError GetPixelRGBA( SGLPalette& pxRGBA, int xPos, int yPos ) ;
		// 画像フィル
		SGLError FillImage
			( const SGLPalette& pxcmp, const SGLImageRect * pRect = nullptr ) ;
		// 画像複製
		SGLError CopyImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 画像描画
		SGLError BlendImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 画像描画（逆順）
		SGLError BlendBackImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 画像描画（全チャネル加算）
		SGLError BlendAddImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 画像描画（全チャネル乗算）
		SGLError BlendMulImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 半透明描画
		SGLError HalfBlendImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// フォーマット変換
		SGLError ConvertImage
			( SGLImageObject * pSrcImage,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// 背景色合成
		SGLError BlendImageBackgroundColor( const SGLPalette& argbBackColor ) ;
		// RGB をαチャネルで積算
		SGLError MultiplyImageRGBAlpha( void ) ;
		// チャネル転送
		SGLError PutImageChannelTo
			( int iDstChannel,
				SGLImageObject * pSrcImage,
				int iSrcChannel,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// チャネル積和値転送
		SGLError PutImageMAddChannelTo
			( int iDstChannel,
				SGLImageObject * pSrcImage,
				const SGLPalette& rgbaMul,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// αチャネルを一次関数を伴って積算合成
		SGLError BlendWithAlphaChannel
			( SGLImageObject * pAlphaImage,
				int32_t fxAlphaCoefficient = 0x100,
				int32_t fxAlphaIntercept = 0,
				int xDst = 0, int yDst = 0,
				const SGLImageRect * pSrcRect = nullptr ) ;
		// トーンカーブ
		SGLError ApplyToneFilter
			( const uint8_t * pRedTone, const uint8_t * pGreenTone,
				const uint8_t * pBlueTone, const uint8_t * pAlphaTone,
				const SGLImageRect * pDstRect = nullptr ) ;
		// 輝度トーンカーブ : (1 - x) * v [v > 0], x * (v + 1) [v < 0]
		static void MakeBrightnessTone( uint8_t * pTone, float32_t v ) ;
		// 積算トーンカーブ : x * v
		static void MakeMultipleTone( uint8_t * pTone, float32_t v ) ;
		// 加算トーンカーブ : x + v
		static void MakeAdditionalTone( uint8_t * pTone, float32_t v ) ;
		// 積算トーンカーブ : (x - 0.5) * v + 0.5
		static void MakeOffsetMultipleTone( uint8_t * pTone, float32_t v ) ;
		// ガンマカーブ : x ^ (1 / v)
		static void MakeGammaTone( uint8_t * pTone, float32_t v ) ;
		// 1/2 縮小
		SGLError EnlargeHalfImage( SGLImageObject * pSrcImage ) ;
		// 90度単位回転（反時計回り）
		SGLError OrthogonalRotate
			( SGLImageObject * pSrcImage, int degRotateAngle ) ;
		// グレイスケールを ARGB に拡張変換する
		SGLError CreateColorImageFromGrayscale
			( SGLImageObject * pGrayscale, const SGLPalette& argbColor ) ;
		// ビットマスク画像を 1/8 縮小して グレイスケール画像に変換する
		SGLError MultisampleGrayscaleFromBitmask( SGLImageObject * pSrcImage ) ;
		// 多角形（凹形状可能）グレイスケール画像生成
		SGLError CreateFilledPolygonShape
			( S2DVector& vMakedOffset,
				const S2DVector * pVertices,
				size_t nCount, float32_t fpUnit = 1.0f ) ;
		// ベジェ曲線閉鎖領域グレイスケール画像生成
		SGLError CreateFilledBezierShape
			( S2DVector& vMakedOffset,
				const S2DVector * pVertices,
				size_t nCount, float32_t fpUnit = 1.0f ) ;

	public:
		// 参照元画像情報取得
		virtual SGLImageObject * GetImageReference
					( SGLImageRect& rectRef, ssize_t iFrame = -1 ) ;
		// 参照先画像変更
		virtual SGLError MakeImageReference
			( size_t iFrame,
				SGLImageObject * pAtlasImage, const SGLImageRect& rectRef ) ;
		// 画像オブジェクト取得
		virtual SGLImageBufferInterface *
			CommitImageObject
				( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage ) ;
		// 画像オブジェクト追加登録
		virtual bool AddImageObject
			( SGLImageBufferInterface * pObject, bool fOriginalImage ) ;
		// 画像オブジェクト分離
		virtual bool DetachImageObject( SGLImageBufferInterface * pObject ) ;
		// 画像オブジェクト更新通知
		virtual SGLError UpdateImageObject( const SGLImageRect * pUpdateRect = nullptr ) ;
		// 画像オブジェクト更新確定
		virtual void FlushImageObject( void ) ;
		// 画像オブジェクト更新反映
		virtual SGLError ReflectImageObject( uint32_t idType ) ;
		// 画像オブジェクト削除
		virtual SGLError DeleteImageObjectTypeOf( uint32_t idType ) ;
		virtual SGLError DeleteAllImageObjects( void ) ;
		// 画像オブジェクトクリア通知
		virtual SGLError NotifyClearImageObject( SGLPalette pxClear ) ;
		// 画像オブジェクト取得
		virtual Image * GetImageObject( void ) const ;
		// 内部バッファ取得
		virtual SGLImageBuffer * GetImageBuffer( void ) const ;
		// 画像識別子
		virtual const wchar_t * GetImageIdentity( void ) const ;
		virtual void SetImageIdentity( const wchar_t * pwszID ) ;

		#if	defined(__PLATFORM_WINDOWS__)
		// 画像をデバイスコンテキストへ描画
		SGLError DrawToDC
			( HDC hDC, int xPos, int yPos,
				const SGLSize * pDstSize = nullptr,
				const SGLImageRect * pSrcView = nullptr,
				DWORD dwROP = SRCCOPY ) ;
		// HBITMAP から画像バッファ生成
		SGLError CreateFromHBITMAP
			( HBITMAP hBitmap, int nFlags = bufferOnMemory ) ;
		// HCURSOR から画像バッファ生成
		SGLError CreateFromHCURSOR
			( HCURSOR hCursor, int nFlags = bufferOnMemory ) ;
		// HICON から画像バッファ生成
		SGLError CreateFromHICON
			( HICON hIcon, int nFlags = bufferOnMemory ) ;
		#endif
	} ;

	#if	defined(__COTOPHA__)
	class	native Image	: public SSystem::VolatileObject
	{
	public:
		// フレーム数取得
		native size_t GetFrameCount( void ) const ;
		// アニメーション・シーケンス全長取得
		native size_t GetSequenceLength( void ) const ;
		// アニメーション・シーケンス取得
		native size_t GetSequenceTable( uint32_t * pSeq, size_t nCount ) const ;
		// アニメーション全長（時間）取得 [msec]
		native uint64_t GetTotalTime( void ) const ;
		// 時間からフレーム番号へ変換
		native size_t FrameFromMilliSec( uint64_t msec ) ;
		// フレーム選択
		native SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
		// 選択中フレーム取得
		native size_t GetSelectedFrame( int * pSide = nullptr ) const ;
		// 画像情報取得
		native SGLError GetImageInfo( SGLImageInfo & imginf ) const ;
		// パレット・テーブル取得
		native size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
		// 画像バッファ取得
		enum	LockBufferMethod
		{
			bufferRead		= 0x01,
			bufferWrite		= 0x02,
			bufferReadWrite	= 0x03,
		} ;
		native uint8_t * LockBuffer
			( SGLImageInfo & imginf,
				int flags = bufferReadWrite,
				const SGLImageRect * pRect = nullptr ) ;
		native SGLError FlushBuffer( int flags = bufferReadWrite ) ;
		native SGLError UnlockBuffer( int flags = bufferReadWrite ) ;
		// 画像バッファ読み込み
		native SGLError ReadFrameBuffer
			( const SGLImageInfo & imginf, uint8_t * ptrBuffer,
					size_t iFrame = 0, int iSide = stereoImageRight ) ;

	public:
		// バッファフラグ
		enum	BufferTypeFlag
		{
			bufferNoWritable		= 0x0001,
			bufferNoReadable		= 0x0002,
			bufferNonPowerOf2		= 0x0008,
			bufferForTexture		= 0x0010,
			bufferForMipmapTexture	= 0x0020,
			bufferForRenderTarget	= 0x0040,
			bufferOnMemory			= 0x0000,
			bufferOnDeviceOnly		= 0x0100,
		} ;
		// 画像バッファへの参照生成
		native Image * NewReference
			( const SGLImageRect * pClip = nullptr,
					ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
		// テクスチャのための正規化
		native SGLError NormalizeToTexture( uint32_t nFlags = 0 ) ;
		native SGLError NormalizeToMipmapTexture( uint32_t nFlags = 0 ) ;
		// レンダリング・ターゲットのための正規化
		native SGLError NormalizeToRenderTarget( uint32_t nFlags = 0 ) ;
		// テクスチャのための正規化フラグを除去する
		native SGLError DenormalizeForTexture( uint32_t nFlags ) ;
		// 画像バッファ生成
		native SGLError CreateBuffer
			( const SGLImageInfo& imginf,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		// 保有リソース解放
		native void ReleaseBuffer( void ) ;
		// バッファフラグ取得
		native int GetBufferFlags( void ) ;
		// パレット・テーブル設定
		native size_t SetPaletteTable( const SGLPalette * pPalette, size_t nCount ) ;
		// アニメーション・シーケンス・テーブルの設定
		native void SetSequenceTable( const uint32_t * pSeq, size_t nCount ) ;
		// 中心座標情報設定
		native void SetImageOrigin( int x, int y ) ;
		// アニメーション全長時間設定
		native void SetAnimationDuration( uint64_t nDuration ) ;

	public:
		// フォーマット正規化フラグ
		enum	NormalizeFormatFlag
		{
			formatMergeAnimation	= 0x0001,
		} ;
		// 画像フォーマット正規化
		native SGLError NormalizeFormat
			( uint32_t format = 0, uint32_t depth = 0,
				uint32_t nFlags = 0,
				uint32_t width = 0, uint32_t height = 0 ) ;
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		native Image * NewAnimationReference
			( SGLImageRect* pFrameRects, size_t nRectsCount ) ;

	public:
		// 画像読み込み
		native SGLError LoadImage
			( const wchar_t * pszFilePath,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		native SGLError ReadImage
			( SSystem::File * file,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		// 読み込み対応拡張子判定
		static native bool IsLoadableFileExtension( const wchar_t * pszExt ) ;
		// 読み込み対応 MIME 判定
		static native bool IsLoadableMIMEType( const wchar_t * pszMIME ) ;

	public:
		// 画像オブジェクト更新確定
		native void FlushImageObject( void ) ;
	} ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 画像オブジェクト・ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLImage	: public SGLImageObject
	{
	protected:
		Image *	m_pImage ;
		bool	m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImage, SGLImageObject )
		// 構築関数
		#if	!defined(__COTOPHA__)
		SGLImage( void ) : m_pImage(nullptr), m_flagOwner(false) {}
		#endif
		SGLImage( Image * pImage, bool flagOwner = false )
					: m_pImage(pImage), m_flagOwner(flagOwner) {}
		SGLImage( const SGLImage& img ) ;
		// 消滅関数
		virtual ~SGLImage( void ) ;
		// 代入
		Image * operator = ( Image * pImage )
		{
			return	SetImageObject( pImage ) ;
		}
		Image * SetImageObject( Image * pImage, bool flagOwner = false ) ;
		// ポインタ変換
		Image * operator -> ( void ) const
		{
			return	m_pImage ;
		}
		Image * GetImage( void ) const
		{
			return	m_pImage ;
		}
		operator Image * ( void ) const
		{
			return	m_pImage ;
		}

	public:
		// フレーム数取得
		virtual size_t GetFrameCount( void ) const ;
		// アニメーション・シーケンス全長取得
		virtual size_t GetSequenceLength( void ) const ;
		// アニメーション・シーケンス取得
		virtual size_t GetSequenceTable( uint32_t * pSeq, size_t nCount ) const ;
		// アニメーション全長（時間）取得 [msec]
		virtual uint64_t GetTotalTime( void ) const ;
		// 時間からフレーム番号へ変換
		virtual size_t FrameFromMilliSec( uint64_t msec ) ;
		// フレーム選択
		virtual SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
		// 選択中フレーム取得
		virtual size_t GetSelectedFrame( int * pSide = nullptr ) const ;
		// 画像情報取得
		virtual SGLError GetImageInfo( SGLImageInfo & imginf ) const ;
		// パレット・テーブル取得
		virtual size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
		// 画像バッファ取得
		virtual uint8_t * LockBuffer
			( SGLImageInfo & imginf,
				int flags = lockReadWrite,
				const SGLImageRect * pRect = nullptr ) ;
		virtual SGLError FlushBuffer( int flags = lockReadWrite ) ;
		virtual SGLError UnlockBuffer( int flags = lockReadWrite ) ;
		// 画像バッファ読み出し
		virtual SGLError ReadFrameBuffer
			( const SGLImageInfo & imginf, uint8_t * ptrBuffer,
					size_t iFrame = 0, int iSide = stereoImageRight ) ;

	public:
		// 画像バッファへの参照生成
		virtual SGLImageObject * NewReference
			( const SGLImageRect * pClip = nullptr,
					ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
		// テクスチャのための正規化
		virtual SGLError NormalizeToTexture( uint32_t nFlags = 0 ) ;
		virtual SGLError NormalizeToMipmapTexture( uint32_t nFlags = 0 ) ;
		// レンダリング・ターゲットのための正規化
		virtual SGLError NormalizeToRenderTarget( uint32_t nFlags = 0 ) ;
		// テクスチャのための正規化フラグを除去する
		virtual SGLError DenormalizeForTexture( uint32_t nFlags ) ;
		// 画像バッファ生成
		virtual SGLError CreateBuffer
			( const SGLImageInfo& imginf,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		// 保有リソース解放
		virtual void ReleaseBuffer( void ) ;
		// バッファフラグ取得
		virtual uint64_t GetBufferFlags( void ) ;
		// パレット・テーブル設定
		virtual size_t SetPaletteTable
			( const SGLPalette * pPalette, size_t nCount ) ;
		// アニメーション・シーケンス・テーブルの設定
		virtual void SetSequenceTable( const uint32_t * pSeq, size_t nCount ) ;
		// 中心座標情報設定
		virtual void SetImageOrigin( int x, int y ) ;
		// アニメーション全長時間設定
		virtual void SetAnimationDuration( uint64_t nDuration ) ;

	public:
		// 画像フォーマット正規化
		virtual SGLError NormalizeFormat
			( uint32_t format = 0, uint32_t depth = 0,
				uint32_t nFlags = 0,
				uint32_t width = 0, uint32_t height = 0 ) ;
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		virtual SGLImageObject * NewAnimationReference
			( SGLImageRect* pFrameRects, size_t nRectsCount ) ;
		// アトラス化された画像の参照矩形を取得
		virtual SGLError GetReferenceRectOfAtlas
			( SGLImageRect& rect, ssize_t iFrame = -1 ) const ;

	public:
		// 画像読み込み
		SGLError LoadImage
			( const wchar_t * pszFilePath,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		SGLError ReadImage
			( SSystem::SFileInterface * file,
				const wchar_t * pszMIME = nullptr, size_t nLimitFrames = 0 ) ;
		// 画像書き出し
		SGLError WriteImage
			( SSystem::SFileInterface * file,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = nullptr ) ;
		// 参照画像を作成する
		SGLError DuplicateOf( Image * pImage ) ;

	public:
		// 参照先画像変更
		virtual SGLError MakeImageReference
			( size_t iFrame,
				SGLImageObject * pAtlasImage, const SGLImageRect& rectRef ) ;
		// 参照元画像情報取得
		virtual SGLImageObject * GetImageReference
					( SGLImageRect& rectRef, ssize_t iFrame = -1 ) ;
		// 画像オブジェクト取得
		virtual SGLImageBufferInterface *
			CommitImageObject
				( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage ) ;
		// 画像オブジェクト追加登録
		virtual bool AddImageObject
			( SGLImageBufferInterface * pObject, bool fOriginalImage ) ;
		// 画像オブジェクト分離
		virtual bool DetachImageObject( SGLImageBufferInterface * pObject ) ;
		// 画像オブジェクト更新通知
		virtual SGLError UpdateImageObject( const SGLImageRect * pUpdateRect = nullptr ) ;
		// 画像オブジェクト更新確定
		virtual void FlushImageObject( void ) ;
		// 画像オブジェクト更新反映
		virtual SGLError ReflectImageObject( uint32_t idType ) ;
		// 画像オブジェクト削除
		virtual SGLError DeleteImageObjectTypeOf( uint32_t idType ) ;
		virtual SGLError DeleteAllImageObjects( void ) ;
		// 画像オブジェクトクリア通知
		virtual SGLError NotifyClearImageObject( SGLPalette pxClear ) ;
		// 画像オブジェクト取得
		virtual Image * GetImageObject( void ) const ;
		// 内部バッファ取得
		virtual SGLImageBuffer * GetImageBuffer( void ) const ;

	} ;

}

#endif
