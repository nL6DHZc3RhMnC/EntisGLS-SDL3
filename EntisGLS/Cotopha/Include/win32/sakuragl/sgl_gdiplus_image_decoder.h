
#if	!defined(__SAKURAGL_GDIPLUS_IMAGE_DECODER_H__)
#define	__SAKURAGL_GDIPLUS_IMAGE_DECODER_H__	1

#include <gdiplus.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// GDI+ CODEC
	//////////////////////////////////////////////////////////////////////////

	class	SGLGdiplusCodec
	{
	public:
		// GDI+ 関数インターフェース
		typedef	Gdiplus::Status 
			(WINAPI *PGDIP_GdiplusStartup)
				( OUT ULONG_PTR * token,
					const Gdiplus::GdiplusStartupInput * input,
					OUT Gdiplus::GdiplusStartupOutput * output ) ;
		typedef	VOID (WINAPI *PGDIP_GdiplusShutdown)( ULONG_PTR token ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipCreateBitmapFromStream)
				( IStream * stream, Gdiplus::GpBitmap ** bitmap ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipCreateBitmapFromScan0)
				( INT width, INT height, INT stride,
					Gdiplus::PixelFormat format,
					BYTE * scan0, Gdiplus::GpBitmap** bitmap ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipCreateBitmapFromFile)
				( GDIPCONST WCHAR * filename, Gdiplus::GpBitmap ** bitmap) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipDisposeImage)( Gdiplus::GpImage * image ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageFlags)
				( Gdiplus::GpImage * image, UINT * flags ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipImageGetFrameCount)
				( Gdiplus::GpImage * image,
					GDIPCONST GUID * dimensionID, UINT * count ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipImageSelectActiveFrame)
				( Gdiplus::GpImage * image,
					GDIPCONST GUID * dimensionID, UINT frameIndex ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImagePixelFormat)
				( Gdiplus::GpImage * image, Gdiplus::PixelFormat * format ) ;
		typedef Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageThumbnail)
				( Gdiplus::GpImage * image, UINT thumbWidth, UINT thumbHeight,
					Gdiplus::GpImage **thumbImage,
					Gdiplus::GetThumbnailImageAbort callback, VOID * callbackData ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImagePaletteSize)
				( Gdiplus::GpImage * image, INT * size ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImagePalette)
				( Gdiplus::GpImage * image, Gdiplus::ColorPalette * palette, INT size ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipSetImagePalette)
				( Gdiplus::GpImage * image, GDIPCONST Gdiplus::ColorPalette * palette) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageWidth)
				( Gdiplus::GpImage * image, UINT * width ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageHeight)
				( Gdiplus::GpImage * image, UINT * height ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipBitmapLockBits)
				( Gdiplus::GpBitmap * bitmap,
					GDIPCONST Gdiplus::GpRect * rect,
					UINT flags, Gdiplus::PixelFormat format,
					Gdiplus::BitmapData * lockedBitmapData ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipBitmapUnlockBits)
				( Gdiplus::GpBitmap * bitmap, Gdiplus::BitmapData * lockedBitmapData ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PDGIP_GdipSaveImageToFile)
				( Gdiplus::GpImage * image,
					GDIPCONST WCHAR * filename,
					GDIPCONST CLSID * clsidEncoder,
					GDIPCONST Gdiplus::EncoderParameters * encoderParams ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PDGIP_GdipSaveImageToStream)
				( Gdiplus::GpImage * image, IStream * stream,
					GDIPCONST CLSID * clsidEncoder, 
					GDIPCONST Gdiplus::EncoderParameters * encoderParams ) ;
		typedef Gdiplus::GpStatus
			(WINGDIPAPI *PDGIP_GdipGdipSaveAdd)
				( Gdiplus::GpImage *image,
					GDIPCONST Gdiplus::EncoderParameters* encoderParams ) ;
		typedef Gdiplus::GpStatus
			(WINGDIPAPI *PDGIP_GdipGdipSaveAddImage)
				( Gdiplus::GpImage *image, Gdiplus::GpImage* newImage,
					GDIPCONST Gdiplus::EncoderParameters* encoderParams ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageDecodersSize)
				( UINT * numDecoders, UINT * size ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageDecoders)
				( UINT numDecoders, UINT size, Gdiplus::ImageCodecInfo * decoders ) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageEncodersSize)
				( UINT * numEncoders, UINT * size) ;
		typedef	Gdiplus::GpStatus
			(WINGDIPAPI *PGDIP_GdipGetImageEncoders)
				( UINT numEncoders, UINT size, Gdiplus::ImageCodecInfo * encoders ) ;

		HMODULE			m_hGDIPlus ;		// GDI+ モジュール
		ULONG_PTR		m_gdiplusToken ;

		PGDIP_GdiplusStartup				m_pfnGdiplusStartup ;
		PGDIP_GdiplusShutdown				m_pfnGdiplusShutdown ;
		PGDIP_GdipCreateBitmapFromStream	m_pfnCreateBitmapFromStream ;
		PGDIP_GdipCreateBitmapFromFile		m_pfnCreateBitmapFromFile ;
		PGDIP_GdipCreateBitmapFromScan0		m_pfnCreateBitmapFromScan0 ;
		PGDIP_GdipDisposeImage				m_pfnDisposeImage ;
		PGDIP_GdipGetImageFlags				m_pfnGetImageFlags ;
		PGDIP_GdipImageGetFrameCount		m_pfnImageGetFrameCount ;
		PGDIP_GdipImageSelectActiveFrame	m_pfnImageSelectActiveFrame ;
		PGDIP_GdipGetImagePixelFormat		m_pfnGetImagePixelFormat ;
		PGDIP_GdipGetImageThumbnail			m_pfnGetImageThumbnail ;
		PGDIP_GdipGetImagePaletteSize		m_pfnGetImagePaletteSize ;
		PGDIP_GdipGetImagePalette			m_pfnGetImagePalette ;
		PGDIP_GdipSetImagePalette			m_pfnSetImagePalette ;
		PGDIP_GdipGetImageWidth				m_pfnGetImageWidth ;
		PGDIP_GdipGetImageHeight			m_pfnGetImageHeight ;
		PGDIP_GdipBitmapLockBits			m_pfnBitmapLockBits ;
		PGDIP_GdipBitmapUnlockBits			m_pfnBitmapUnlockBits ;
		PDGIP_GdipSaveImageToFile			m_pfnSaveImageToFile ;
		PDGIP_GdipSaveImageToStream			m_pfnSaveImageToStream ;
		PDGIP_GdipGdipSaveAdd				m_pfnSaveAdd ;
		PDGIP_GdipGdipSaveAddImage			m_pfnSaveAddImage ;
		PGDIP_GdipGetImageDecodersSize		m_pfnGetImageDecodersSize ;
		PGDIP_GdipGetImageDecoders			m_pfnGetImageDecoders ;
		PGDIP_GdipGetImageEncodersSize		m_pfnGetImageEncodersSize ;
		PGDIP_GdipGetImageEncoders			m_pfnGetImageEncoders ;

		// GUID
		static const GUID	GUID_GDIP_FrameDimensionTime ;
		static const GUID	GUID_GDIP_FrameDimensionResolution ;
		static const GUID	GUID_GDIP_FrameDimensionPage ;
		static const GUID	GUID_GDIP_EncoderQuality ;
		static const GUID	GUID_GDIP_EncoderSaveFlag ;

	public:
		// 構築関数
		SGLGdiplusCodec( void ) ;
		// 消滅関数
		~SGLGdiplusCodec( void ) ;

	public:
		// 画像読み込み
		SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
		// 画像書き出し
		SGLError WriteImage
			( SGLImageObject & image, SSystem::SFileInterface & file,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
		// GDI+ オブジェクトから変換する
		SGLError ConvertFromGDIplus
			( SGLImageObject & image,
				Gdiplus::GpBitmap * pbitmap, size_t nLimitFrames ) ;
		// GDI+ オブジェクトへ変換する
		SGLError ConvertToGDIplus
			( Gdiplus::GpBitmap *& pbitmap, SGLImageObject & image ) ;
		// デコーダー列挙
		Gdiplus::ImageCodecInfo *
			EnumerateDecoderInfo
				( SSystem::SArray<uint8_t>& bufEnumDecoders, UINT& nDecoderCount ) ;
		// エンコーダー列挙
		Gdiplus::ImageCodecInfo *
			EnumerateEncoderInfo
				( SSystem::SArray<uint8_t>& bufEnumEncoders, UINT& nEncoderCount ) ;
		// デコーダー拡張子判定
		bool IsMatchableDecoderFileExtension
				( const wchar_t * pszExt, CLSID& clsidDecoder ) ;
		// エンコーダー拡張子判定
		bool IsMatchableEncoderFileExtension
				( const wchar_t * pszExt,
					SSystem::SString& strMIME, CLSID& clsidEncoder ) ;
		// デコーダー MIME 判定
		bool IsMatchableDecoderMIMEType
				( const wchar_t * pszMIME, CLSID& clsidDecoder ) ;
		// エンコーダー MIME 判定
		bool IsMatchableEncoderMIMEType
				( const wchar_t * pszMIME, CLSID& clsidEncoder ) ;
		// GDI+ インストールチェック
		bool IsGDIplusInstalled( void ) const
		{
			return	(m_hGDIPlus != NULL) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// GDI+ デコーダ
	//////////////////////////////////////////////////////////////////////////

	class	SGLGdiplusImageDecoder	: public SGLImageDecoderInterface
	{
	protected:
		SGLGdiplusCodec	m_gdipCodec ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGdiplusImageDecoder, SGLImageDecoderInterface )
		// 構築関数
		SGLGdiplusImageDecoder( void ) ;
		// 消滅関数
		virtual ~SGLGdiplusImageDecoder( void ) ;
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
		// GDI+ インストールチェック
		bool IsGDIplusInstalled( void ) const
		{
			return	m_gdipCodec.IsGDIplusInstalled() ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// GDI+ エンコーダ
	//////////////////////////////////////////////////////////////////////////

	class	SGLGdiplusImageEncoder	: public SGLImageEncoderInterface
	{
	protected:
		SGLGdiplusCodec	m_gdipCodec ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGdiplusImageEncoder, SGLImageEncoderInterface )
		// 構築関数
		SGLGdiplusImageEncoder( void ) ;
		// 消滅関数
		virtual ~SGLGdiplusImageEncoder( void ) ;
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension
			( const wchar_t * pszExt, SSystem::SString& strMIME ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像書き出し
		virtual SGLError WriteImage
			( SSystem::SFileInterface & file,
				SGLImageObject & image,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
		// GDI+ インストールチェック
		bool IsGDIplusInstalled( void ) const
		{
			return	m_gdipCodec.IsGDIplusInstalled() ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// IStream 実装
	//////////////////////////////////////////////////////////////////////////

	class	SStreamCOMInterface	: public IStream
	{
	protected:
		atomic_int_t				m_nRef ;
		SSystem::SFileInterface *	m_pFile ;
		bool						m_flagOwner ;

	public:
		// IStream 作成
		static IStream * CreateStream
			( SSystem::SFileInterface * pFile, bool flagOwner ) ;

	public:
		// 構築関数
		SStreamCOMInterface
			( SSystem::SFileInterface * pFile, bool flagOwner ) ;
		// 消滅関数
		virtual ~SStreamCOMInterface( void ) ;

	public:
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
		virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
		virtual ULONG STDMETHODCALLTYPE Release( void ) ;

		// ISequentialStream
		virtual HRESULT STDMETHODCALLTYPE
			Read( void * ptrBuf, ULONG nBytes, ULONG * pReadBytes ) ;
		virtual HRESULT STDMETHODCALLTYPE
			Write( const void * ptrBuf, ULONG nBytes, ULONG * pWrittenBytes ) ;

		// IStream
		virtual HRESULT STDMETHODCALLTYPE
			Seek( LARGE_INTEGER posDst, DWORD dwOrg, ULARGE_INTEGER * pMovedPos ) ;
		virtual HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER nNewSize ) ;
		virtual HRESULT STDMETHODCALLTYPE
			CopyTo( IStream * pStream, ULARGE_INTEGER nBytes,
					ULARGE_INTEGER * pRead, ULARGE_INTEGER * pWritten ) ;
		virtual HRESULT STDMETHODCALLTYPE Commit( DWORD dwFlags ) ;
		virtual HRESULT STDMETHODCALLTYPE Revert( void ) ;
		virtual HRESULT STDMETHODCALLTYPE
			LockRegion( ULARGE_INTEGER nOffset, ULARGE_INTEGER nBytes, DWORD dwLockType ) ;
		virtual HRESULT STDMETHODCALLTYPE
			UnlockRegion( ULARGE_INTEGER nOffset, ULARGE_INTEGER nBytes, DWORD dwLockType ) ;
		virtual HRESULT STDMETHODCALLTYPE Stat( STATSTG * pStats, DWORD dwStatFlag ) ;
		virtual HRESULT STDMETHODCALLTYPE Clone( IStream ** ppStream ) ;

	} ;

}

#endif
