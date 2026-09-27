
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl_gdiplus_image_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// GDI+ CODEC
//////////////////////////////////////////////////////////////////////////////

// GUID
//////////////////////////////////////////////////////////////////////////////
const GUID	SGLGdiplusCodec::GUID_GDIP_FrameDimensionTime =
	{ 0x6aedbd6d,0x3fb5,0x418a, { 0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72 } } ;
const GUID	SGLGdiplusCodec::GUID_GDIP_FrameDimensionResolution =
	{ 0x84236f7b,0x3bd3,0x428f, { 0x8d,0xab,0x4e,0xa1,0x43,0x9c,0xa3,0x15 } } ;
const GUID	SGLGdiplusCodec::GUID_GDIP_FrameDimensionPage =
	{ 0x7462dc86,0x6180,0x4c7e, { 0x8e,0x3f,0xee,0x73,0x33,0xa7,0xa4,0x83 } } ;
const GUID	SGLGdiplusCodec::GUID_GDIP_EncoderQuality =
	{ 0x1d5be4b5,0xfa4a,0x452d, { 0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb } } ;
const GUID	SGLGdiplusCodec::GUID_GDIP_EncoderSaveFlag =
	{ 0x292266fc,0xac40,0x47bf, { 0x8c,0xfc,0xa8,0x5b,0x89,0xa6,0x55,0xde } } ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGdiplusCodec::SGLGdiplusCodec( void )
{
	m_hGDIPlus = NULL ;
	//
	m_pfnGdiplusStartup = NULL ;
	m_pfnGdiplusShutdown = NULL ;
	m_pfnCreateBitmapFromStream = NULL ;
	m_pfnCreateBitmapFromFile = NULL ;
	m_pfnCreateBitmapFromScan0 = NULL ;
	m_pfnDisposeImage = NULL ;
	m_pfnGetImageFlags = NULL ;
	m_pfnImageGetFrameCount = NULL ;
	m_pfnImageSelectActiveFrame = NULL ;
	m_pfnGetImagePixelFormat = NULL ;
	m_pfnGetImageThumbnail = NULL ;
	m_pfnGetImagePaletteSize = NULL ;
	m_pfnGetImagePalette = NULL ;
	m_pfnSetImagePalette = NULL ;
	m_pfnGetImageWidth = NULL ;
	m_pfnGetImageHeight = NULL ;
	m_pfnBitmapLockBits = NULL ;
	m_pfnBitmapUnlockBits = NULL ;
	m_pfnSaveImageToFile = NULL ;
	m_pfnSaveImageToStream = NULL ;
	m_pfnGetImageDecodersSize = NULL ;
	m_pfnGetImageDecoders = NULL ;
	m_pfnGetImageEncodersSize = NULL ;
	m_pfnGetImageEncoders = NULL ;
	//
	m_hGDIPlus = (HMODULE) ::LoadLibrary( "gdiplus.dll" ) ;
	if ( m_hGDIPlus != NULL )
	{
		m_pfnGdiplusStartup = (PGDIP_GdiplusStartup)
			::GetProcAddress( m_hGDIPlus, "GdiplusStartup" ) ;
		m_pfnGdiplusShutdown = (PGDIP_GdiplusShutdown)
			::GetProcAddress( m_hGDIPlus, "GdiplusShutdown" ) ;
		m_pfnCreateBitmapFromStream = (PGDIP_GdipCreateBitmapFromStream)
			::GetProcAddress( m_hGDIPlus, "GdipCreateBitmapFromStream" ) ;
		m_pfnCreateBitmapFromFile = (PGDIP_GdipCreateBitmapFromFile)
			::GetProcAddress( m_hGDIPlus, "GdipCreateBitmapFromFile" ) ;
		m_pfnCreateBitmapFromScan0 = (PGDIP_GdipCreateBitmapFromScan0)
			::GetProcAddress( m_hGDIPlus, "GdipCreateBitmapFromScan0" ) ;
		m_pfnDisposeImage = (PGDIP_GdipDisposeImage)
			::GetProcAddress( m_hGDIPlus, "GdipDisposeImage" ) ;
		m_pfnGetImageFlags = (PGDIP_GdipGetImageFlags)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageFlags" ) ;
		m_pfnImageGetFrameCount = (PGDIP_GdipImageGetFrameCount)
			::GetProcAddress( m_hGDIPlus, "GdipImageGetFrameCount" ) ;
		m_pfnImageSelectActiveFrame = (PGDIP_GdipImageSelectActiveFrame)
			::GetProcAddress( m_hGDIPlus, "GdipImageSelectActiveFrame" ) ;
		m_pfnGetImagePixelFormat = (PGDIP_GdipGetImagePixelFormat)
			::GetProcAddress( m_hGDIPlus, "GdipGetImagePixelFormat" ) ;
		m_pfnGetImageThumbnail = (PGDIP_GdipGetImageThumbnail)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageThumbnail" ) ;
		m_pfnGetImagePaletteSize = (PGDIP_GdipGetImagePaletteSize)
			::GetProcAddress( m_hGDIPlus, "GdipGetImagePaletteSize" ) ;
		m_pfnGetImagePalette = (PGDIP_GdipGetImagePalette)
			::GetProcAddress( m_hGDIPlus, "GdipGetImagePalette" ) ;
		m_pfnSetImagePalette = (PGDIP_GdipSetImagePalette)
			::GetProcAddress( m_hGDIPlus, "GdipSetImagePalette" ) ;
		m_pfnGetImageWidth = (PGDIP_GdipGetImageWidth)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageWidth" ) ;
		m_pfnGetImageHeight = (PGDIP_GdipGetImageHeight)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageHeight" ) ;
		m_pfnBitmapLockBits = (PGDIP_GdipBitmapLockBits)
			::GetProcAddress( m_hGDIPlus, "GdipBitmapLockBits" ) ;
		m_pfnBitmapUnlockBits = (PGDIP_GdipBitmapUnlockBits)
			::GetProcAddress( m_hGDIPlus, "GdipBitmapUnlockBits" ) ;
		m_pfnSaveImageToFile = (PDGIP_GdipSaveImageToFile)
			::GetProcAddress( m_hGDIPlus, "GdipSaveImageToFile" ) ;
		m_pfnSaveImageToStream = (PDGIP_GdipSaveImageToStream)
			::GetProcAddress( m_hGDIPlus, "GdipSaveImageToStream" ) ;
		m_pfnSaveAdd = (PDGIP_GdipGdipSaveAdd)
			::GetProcAddress( m_hGDIPlus, "GdipSaveAdd" ) ;
		m_pfnSaveAddImage = (PDGIP_GdipGdipSaveAddImage)
			::GetProcAddress( m_hGDIPlus, "GdipSaveAddImage" ) ;
		m_pfnGetImageDecodersSize = (PGDIP_GdipGetImageDecodersSize)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageDecodersSize" ) ;
		m_pfnGetImageDecoders = (PGDIP_GdipGetImageDecoders)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageDecoders" ) ;
		m_pfnGetImageEncodersSize = (PGDIP_GdipGetImageEncodersSize)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageEncodersSize" ) ;
		m_pfnGetImageEncoders = (PGDIP_GdipGetImageEncoders)
			::GetProcAddress( m_hGDIPlus, "GdipGetImageEncoders" ) ;
		//
		if ( m_pfnGdiplusStartup != NULL )
		{
			Gdiplus::GdiplusStartupInput	gdiplusStartupInput ;
			m_pfnGdiplusStartup
				( &m_gdiplusToken, &gdiplusStartupInput, NULL ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLGdiplusCodec::~SGLGdiplusCodec( void )
{
	if ( m_pfnGdiplusShutdown != NULL )
	{
		m_pfnGdiplusShutdown( m_gdiplusToken ) ;
	}
	if ( m_hGDIPlus != NULL )
	{
		::FreeLibrary( m_hGDIPlus ) ;
		m_hGDIPlus = NULL ;
	}
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusCodec::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	Gdiplus::GpBitmap *	pbitmap = NULL ;
	SGLError			err = sglErrSuccess ;
	do
	{
		//
		// ファイルをメモリ上に読み込む
		//
		SFileInterface *	pfile = &file ;
		SSmartBuffer		sbuf ;
		int64_t	nFileSize = file.GetLength() ;
		if ( nFileSize < 0 )
		{
			sbuf.ReadFromStream( file ) ;
			nFileSize = sbuf.GetLength() ;
			pfile = &sbuf ;
		}
		HGLOBAL	hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, (DWORD) nFileSize ) ;
		if ( hGlobal == NULL )
		{
			err = sglErrFailed ;
			break ;
		}
		LPVOID	lpBuffer = ::GlobalLock( hGlobal ) ;
		if ( lpBuffer == NULL )
		{
			::GlobalFree( hGlobal ) ;
			err = sglErrFailed ;
			break ;
		}
		pfile->Read( lpBuffer, (DWORD) nFileSize ) ;
		::GlobalUnlock( hGlobal ) ;
		//
		// IStream オブジェクトを生成する
		//
		LPSTREAM	stream = NULL ;
		if ( ::CreateStreamOnHGlobal( hGlobal, TRUE, &stream ) != S_OK )
		{
			err = sglErrFailed ;
			break ;
		}
		if ( stream == NULL )
		{
			err = sglErrFailed ;
			break ;
		}
		//
		// 画像を読み込む
		//
		if ( (m_pfnCreateBitmapFromStream == NULL)
			|| m_pfnCreateBitmapFromStream( stream, &pbitmap ) )
		{
			stream->Release( ) ;
			err = sglErrFailed ;
			break ;
		}
		stream->Release( ) ;
		//
		// 画像オブジェクトへ変換
		//
		err = ConvertFromGDIplus( image, pbitmap, nLimitFrames ) ;
	}
	while ( false ) ;
	//
	// 終了
	//
	if ( pbitmap != NULL )
	{
		ESLAssert( m_pfnDisposeImage != NULL ) ;
		if ( m_pfnDisposeImage != NULL )
		{
			m_pfnDisposeImage( pbitmap ) ;
		}
	}
	return	err ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusCodec::WriteImage
	( SGLImageObject & image, SSystem::SFileInterface & file,
			const wchar_t * pszMIME,
			const SGLImageEncoderInterface::Options * pOpt )
{
	if ( m_pfnSaveImageToStream == NULL )
	{
		return	sglErrFailed ;
	}
	CLSID	clsidEncoder ;
	if ( !IsMatchableEncoderMIMEType( pszMIME, clsidEncoder ) )
	{
		return	sglErrFailed ;
	}
	//
	// エンコード・パラメータ
	//
	class	EncoderParameters2	: public Gdiplus::EncoderParameters
	{
	public:
		Gdiplus::EncoderParameter Parameter2 ;
	} ;
	Gdiplus::EncoderParameters * pEncParams = NULL ;
	EncoderParameters2			encParams ;
	UINT						iParam = 0 ;
	ULONG						ulQualityParam ;
	ULONG						ulSaveFlag ;
	if ( (pOpt != NULL)
		&& (pOpt->nFlags & SGLImageEncoderInterface::optionQuality) )
	{
		ulQualityParam = (ULONG) (pOpt->nQuality * 100 / 0x100) ;
		if ( ulQualityParam > 100 )
		{
			ulQualityParam = 100 ;
		}
		encParams.Parameter[iParam].Guid = GUID_GDIP_EncoderQuality ;
		encParams.Parameter[iParam].Type = Gdiplus::EncoderParameterValueTypeLong ;
		encParams.Parameter[iParam].NumberOfValues = 1 ;
		encParams.Parameter[iParam].Value = &ulQualityParam ;
		encParams.Count = ++ iParam ;
		//
		pEncParams = &encParams ;
	}
	if ( image.GetFrameCount() >= 2 )
	{
		ulSaveFlag = Gdiplus::EncoderValueMultiFrame ;
		//
		encParams.Parameter[iParam].Guid = GUID_GDIP_EncoderSaveFlag ;
		encParams.Parameter[iParam].Type = Gdiplus::EncoderParameterValueTypeLong ;
		encParams.Parameter[iParam].NumberOfValues = 1 ;
		encParams.Parameter[iParam].Value = &ulSaveFlag ;
		encParams.Count = ++ iParam ;
		//
		pEncParams = &encParams ;
	}
	//
	// 各フレーム書き出し
	//
	IStream *			pStream = nullptr ;
	Gdiplus::GpBitmap *	pbitmap0 = nullptr ;
	Gdiplus::GpBitmap *	pbitmap = nullptr ;
	SGLError			err = sglErrSuccess ;
	//
	for ( size_t iFrame = 0; iFrame < image.GetFrameCount(); iFrame ++ )
	{
		//
		// GDI+ オブジェクトに変換する
		//
		pbitmap = nullptr ;
		image.SelectFrame( iFrame ) ;
		if ( ConvertToGDIplus( pbitmap, image ) )
		{
			err = sglErrFailed ;
			break ;
		}
		//
		// ファイルへ保存する
		//
		if ( iFrame == 0 )
		{
			pStream = SStreamCOMInterface::CreateStream( &file, false ) ;
			ESLAssert( m_pfnSaveImageToStream != nullptr ) ;
			if ( Gdiplus::Ok != m_pfnSaveImageToStream
				( pbitmap, pStream, &clsidEncoder, pEncParams ) )
			{
				err = sglErrFailed ;
				break ;
			}
			pbitmap0 = pbitmap ;
			pbitmap = nullptr ;
		}
		else
		{
			ESLAssert( m_pfnSaveAddImage != nullptr ) ;
			ulSaveFlag = Gdiplus::EncoderValueFrameDimensionPage ;
			if ( Gdiplus::Ok !=
				m_pfnSaveAddImage( pbitmap0, pbitmap, pEncParams ) )
			{
				err = sglErrFailed ;
				break ;
			}
			if ( m_pfnDisposeImage != nullptr )
			{
				m_pfnDisposeImage( pbitmap ) ;
			}
			pbitmap = nullptr ;
		}
	}
	if ( image.GetFrameCount() >= 2 )
	{
		ESLAssert( m_pfnSaveAdd != nullptr ) ;
		ulSaveFlag = Gdiplus::EncoderValueFlush ;
		if ( Gdiplus::Ok != m_pfnSaveAdd( pbitmap0, pEncParams ) )
		{
			err = sglErrFailed ;
		}
	}
	ESLAssert( m_pfnDisposeImage != nullptr ) ;
	if ( m_pfnDisposeImage != nullptr )
	{
		if ( pbitmap0 != nullptr )
		{
			m_pfnDisposeImage( pbitmap0 ) ;
		}
		if ( pbitmap != nullptr )
		{
			m_pfnDisposeImage( pbitmap ) ;
		}
	}
	if ( pStream != nullptr )
	{
		pStream->Release() ;
	}
	return	err ;
}

// GDI+ オブジェクトから変換する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusCodec::ConvertFromGDIplus
	( SGLImageObject & image,
		Gdiplus::GpBitmap * pbitmap, size_t nLimitFrames )
{
	//
	// ピクセルフォーマットを取得
	//
	Gdiplus::PixelFormat	pxfmt ;
	INT						nPaletteSize ;
	Gdiplus::ColorPalette *	pPalette = NULL ;
	SArray<uint8_t>			bufPalette ;
	//
	if ( (m_pfnGetImagePixelFormat == NULL)
		|| m_pfnGetImagePixelFormat( pbitmap, &pxfmt ) )
	{
		return	sglErrFailed ;
	}
	SGLImageInfo	imginf ;
	if ( pxfmt & PixelFormatPAlpha )
	{
		//
		// RGBA フォーマット（積算済み）
		//
		pxfmt = PixelFormat32bppPARGB ;
		imginf.format = formatImageARGB ;
		imginf.depth = 32 ;
	}
	else if ( pxfmt & PixelFormatAlpha )
	{
		//
		// RGBA フォーマット（非積算済み）
		//
		pxfmt = PixelFormat32bppARGB ;
		imginf.format = formatImageARGB | formatImageFlagNoProductOfAlpha ;
		imginf.depth = 32 ;
	}
	else if ( pxfmt & PixelFormatIndexed )
	{
		//
		// 256色 フォーマット
		//
		pxfmt = PixelFormat8bppIndexed ;
		imginf.format = formatImageRGB | formatImageFlagPalette ;
		imginf.depth = 8 ;
		//
		// パレットテーブルを取得する
		//
		if ( (m_pfnGetImagePaletteSize == NULL)
			|| m_pfnGetImagePaletteSize( pbitmap, &nPaletteSize ) )
		{
			return	sglErrFailed ;
		}
		bufPalette.SetLength( nPaletteSize ) ;
		pPalette = (Gdiplus::ColorPalette*) bufPalette.GetArray() ;
		if ( (m_pfnGetImagePalette == NULL)
			|| m_pfnGetImagePalette( pbitmap, pPalette, nPaletteSize ) )
		{
			bufPalette.FinishArray() ;
			return	sglErrFailed ;
		}
		if ( pPalette->Flags & Gdiplus::PaletteFlagsGrayScale )
		{
			imginf.format = formatImageGray ;
		}
		else if ( pPalette->Flags & Gdiplus::PaletteFlagsHasAlpha )
		{
			imginf.format = formatImageARGB | formatImageFlagPalette ;
		}
		bufPalette.FinishArray() ;
	}
	else
	{
		//
		// RGB フォーマット
		//
		pxfmt = PixelFormat32bppPARGB ;
		imginf.format = formatImageRGB ;
		imginf.depth = 32 ;
	}
	//
	// 画像サイズを取得
	//
	UINT	nWidth, nHeight ;
	if ( (m_pfnGetImageWidth == NULL)
		|| m_pfnGetImageWidth( pbitmap, &nWidth ) )
	{
		return	sglErrFailed ;
	}
	if ( (m_pfnGetImageHeight == NULL)
		|| m_pfnGetImageHeight( pbitmap, &nHeight ) )
	{
		return	sglErrFailed ;
	}
	//
	// フレーム数取得
	//
	unsigned int	nFrameCount = 1 ;
	if ( m_pfnImageGetFrameCount != NULL )
	{
		m_pfnImageGetFrameCount
			( pbitmap, &GUID_GDIP_FrameDimensionTime, &nFrameCount ) ;
	}
	if ( (nLimitFrames != 0) && (nFrameCount > nLimitFrames) )
	{
		nFrameCount = (unsigned int) nLimitFrames ;
	}
	//
	// デコード先画像オブジェクト生成
	//
	imginf.width = (uint32_t) nWidth ;
	imginf.height = (uint32_t) nHeight ;
	//
	image.CreateBuffer
		( imginf, SGLImageObject::bufferOnMemory,
				(size_t) nFrameCount, nFrameCount * 66 ) ;
	//
	// フレーム毎に画像データ取得
	//
	for ( unsigned int i = 0; i < nFrameCount; i ++ )
	{
		if ( m_pfnImageSelectActiveFrame != NULL )
		{
			m_pfnImageSelectActiveFrame
				( pbitmap, &GUID_GDIP_FrameDimensionTime, i )  ;
		}
		image.SelectFrame( i ) ;
		//
		// パレットテーブルを取得する
		//
		if ( imginf.format & formatImageFlagPalette )
		{
			if ( (m_pfnGetImagePaletteSize == NULL)
				|| m_pfnGetImagePaletteSize( pbitmap, &nPaletteSize ) )
			{
				return	sglErrFailed ;
			}
			bufPalette.SetLength( nPaletteSize ) ;
			pPalette = (Gdiplus::ColorPalette*) bufPalette.GetArray() ;
			if ( (m_pfnGetImagePalette == NULL)
				|| m_pfnGetImagePalette( pbitmap, pPalette, nPaletteSize ) )
			{
				bufPalette.FinishArray() ;
				return	sglErrFailed ;
			}
			SArray<SGLPalette>	bufPaletteTable ;
			SGLPalette *		pPaletteTable ;
			bufPaletteTable.SetLength( pPalette->Count ) ;
			pPaletteTable = bufPaletteTable.GetArray() ;
			for ( size_t i = 0; i < pPalette->Count; i ++ )
			{
				pPaletteTable[i].ui32 = pPalette->Entries[i] ;
			}
			image.SetPaletteTable( pPaletteTable, pPalette->Count ) ;
			bufPalette.FinishArray() ;
			bufPaletteTable.FinishArray() ;
		}
		//
		// 画像データ配列を取得
		//
		Gdiplus::Rect		rect ;
		Gdiplus::BitmapData	bmdata ;
		rect.X = 0 ;
		rect.Y = 0 ;
		rect.Width = nWidth ;
		rect.Height = nHeight ;
		if ( m_pfnBitmapLockBits == NULL )
		{
			return	sglErrFailed ;
		}
		Gdiplus::Status	status =
			m_pfnBitmapLockBits
				( pbitmap, &rect,
					Gdiplus::ImageLockModeRead, pxfmt, &bmdata ) ;
		if ( status != Gdiplus::Ok )
		{
			return	sglErrFailed ;
		}
		SGLImageInfo	infLocked ;
		uint8_t *		pbytImage =
			image.LockBuffer( infLocked, SGLImageObject::lockWrite ) ;
		if ( pbytImage == NULL )
		{
			return	sglErrFailed ;
		}
		const uint8_t *	pbytSrc = (const uint8_t*) bmdata.Scan0 ;
		uint32_t		widthLocked = infLocked.width ;
		if ( widthLocked > bmdata.Width )
		{
			widthLocked = (uint32_t) bmdata.Width ;
		}
		for ( uint32_t y = 0;
				(y < infLocked.height) && (y < bmdata.Height); y ++ )
		{
			eslMoveMemory
				( pbytImage, pbytSrc,
					(size_t) (widthLocked * infLocked.pitchPixel) ) ;
			//
			pbytImage += infLocked.pitchLine ;
			pbytSrc += bmdata.Stride ;
		}
		//
		image.UnlockBuffer( SGLImageObject::lockWrite ) ;
		//
		if ( m_pfnBitmapUnlockBits != NULL )
		{
			m_pfnBitmapUnlockBits( pbitmap, &bmdata ) ;
		}
	}
	return	sglErrSuccess ;
}

// GDI+ オブジェクトへ変換する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusCodec::ConvertToGDIplus
	( Gdiplus::GpBitmap *& pbitmap, SGLImageObject & image )
{
	if ( m_pfnCreateBitmapFromScan0 == NULL )
	{
		return	sglErrFailed ;
	}
	Gdiplus::PixelFormat	pxfmt ;
	SGLImageBuffer	imgbuf ;
	imgbuf.ptrBuffer =
		image.LockBuffer( imgbuf, SGLImageObject::lockRead ) ;
	//
	switch ( imgbuf.format & formatImageTypeMask )
	{
	case	formatImageRGB:
	case	formatImageGray:
	case	formatImageBGR:
	case	formatImageYUV:
	case	formatImageHSB:
		switch ( imgbuf.depth )
		{
		case	24:
			pxfmt = PixelFormat24bppRGB ;
			break ;
		case	32:
			pxfmt = PixelFormat32bppRGB ;
			if ( imgbuf.format & formatImageFlagAlpha )
			{
				pxfmt = PixelFormat32bppARGB ;
			}
			break ;
		case	8:
			pxfmt = PixelFormat8bppIndexed ;
			break ;
		case	16:
			pxfmt = PixelFormat16bppRGB555 ;
			break ;
		case	1:
			pxfmt = PixelFormat1bppIndexed ;
			break ;
		case	4:
			pxfmt = PixelFormat4bppIndexed ;
			break ;
		}
		break ;
	case	formatImageZ:
	case	formatImageDepth:
		pxfmt = PixelFormat32bppARGB ;
		break ;
	}
	if ( m_pfnCreateBitmapFromScan0
		( imgbuf.width, imgbuf.height,
			imgbuf.pitchLine, pxfmt,
			(BYTE*) imgbuf.ptrBuffer, &pbitmap ) != Gdiplus::Ok )
	{
		image.UnlockBuffer( SGLImageObject::lockRead ) ;
		return	sglErrFailed ;
	}
	image.UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	SGLPalette	palTable[0x100] ;
	size_t	nPalettes = image.GetPaletteTable( &palTable[0], 0x100 ) ;
	if ( nPalettes > 0 )
	{
		if ( m_pfnSetImagePalette != NULL )
		{
			SByteBuffer				bufGdipPal ;
			Gdiplus::ColorPalette *	pClrPal =
				(Gdiplus::ColorPalette*)
					bufGdipPal.GetArray
						( sizeof(Gdiplus::ColorPalette)
							+ nPalettes * sizeof(Gdiplus::ARGB) ) ;
			pClrPal->Flags = 0 ;
			pClrPal->Count = (UINT) nPalettes ;
			if ( imgbuf.format & formatImageFlagClipping )
			{
				pClrPal->Flags |= Gdiplus::PaletteFlagsHasAlpha ;
				for ( size_t i = 0; i < nPalettes; i ++ )
				{
					if ( i != imgbuf.colorClip )
					{
						pClrPal->Entries[i] = palTable[i].ui32 | 0xFF000000 ;
					}
					else
					{
						pClrPal->Entries[i] = 0 ;
					}
				}
			}
			else if ( imgbuf.format & formatImageFlagAlpha )
			{
				pClrPal->Flags |= Gdiplus::PaletteFlagsHasAlpha ;
				for ( size_t i = 0; i < nPalettes; i ++ )
				{
					pClrPal->Entries[i] = palTable[i].ui32 ;
				}
			}
			else
			{
				if ( (imgbuf.format & formatImageTypeMask) == formatImageGray )
				{
					pClrPal->Flags |= Gdiplus::PaletteFlagsGrayScale ;
				}
				for ( size_t i = 0; i < nPalettes; i ++ )
				{
					pClrPal->Entries[i] = palTable[i].ui32 | 0xFF000000 ;
				}
			}
			m_pfnSetImagePalette( pbitmap, pClrPal ) ;
			bufGdipPal.FinishArray() ;
		}
	}
	return	sglErrSuccess ;
}

// デコーダー列挙
//////////////////////////////////////////////////////////////////////////////
Gdiplus::ImageCodecInfo *
	SGLGdiplusCodec::EnumerateDecoderInfo
		( SArray<uint8_t>& bufEnumDecoders, UINT& nDecoderCount )
{
	if ( (m_pfnGetImageDecodersSize == NULL)
		|| (m_pfnGetImageDecoders == NULL) )
	{
		return	NULL ;
	}
	UINT	nDecoderSize ;
	if ( m_pfnGetImageDecodersSize
		( &nDecoderCount, &nDecoderSize ) != Gdiplus::Ok )
	{
		return	NULL ;
	}
	Gdiplus::ImageCodecInfo *	piciDecoders ;
	bufEnumDecoders.SetLength( nDecoderSize ) ;
	piciDecoders = (Gdiplus::ImageCodecInfo*) bufEnumDecoders.GetArray() ;
	if ( m_pfnGetImageDecoders
		( nDecoderCount, nDecoderSize, piciDecoders ) != Gdiplus::Ok )
	{
		bufEnumDecoders.FinishArray() ;
		return	NULL ;
	}
	bufEnumDecoders.FinishArray() ;
	return	piciDecoders ;
}

// エンコーダー列挙
//////////////////////////////////////////////////////////////////////////////
Gdiplus::ImageCodecInfo *
	SGLGdiplusCodec::EnumerateEncoderInfo
		( SSystem::SArray<uint8_t>& bufEnumEncoders, UINT& nEncoderCount )
{
	if ( (m_pfnGetImageEncodersSize == NULL)
		|| (m_pfnGetImageEncoders == NULL) )
	{
		return	NULL ;
	}
	UINT	nEncoderSize ;
	if ( m_pfnGetImageEncodersSize
		( &nEncoderCount, &nEncoderSize ) != Gdiplus::Ok )
	{
		return	NULL ;
	}
	Gdiplus::ImageCodecInfo *	piciEncoders ;
	bufEnumEncoders.SetLength( nEncoderSize ) ;
	piciEncoders = (Gdiplus::ImageCodecInfo*) bufEnumEncoders.GetArray() ;
	if ( m_pfnGetImageEncoders
		( nEncoderCount, nEncoderSize, piciEncoders ) != Gdiplus::Ok )
	{
		bufEnumEncoders.FinishArray() ;
		return	NULL ;
	}
	bufEnumEncoders.FinishArray() ;
	return	piciEncoders ;
}

// デコーダー拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusCodec::IsMatchableDecoderFileExtension
		( const wchar_t * pszExt, CLSID& clsidDecoder )
{
	SArray<uint8_t>	bufEnumDecoders ;
	UINT			nDecoderCount ;
	Gdiplus::ImageCodecInfo *
		piciDecoders = EnumerateDecoderInfo
							( bufEnumDecoders, nDecoderCount ) ;
	if ( piciDecoders == NULL )
	{
		return	false ;
	}
	for ( UINT i = 0; i < nDecoderCount; i ++ )
	{
		SString			strFileExt = piciDecoders[i].FilenameExtension ;
		SStringParser	sparsExt ;
		sparsExt.AttachString( strFileExt ) ;
		do
		{
			SString	strEnumExt ;
			sparsExt.NextEnclosedString( strEnumExt, L';' ) ;
			//
			SString	strExt = strEnumExt.GetFileExtensionPart() ;
			if ( strExt.CompareNoCase( pszExt ) == 0 )
			{
				clsidDecoder = piciDecoders[i].Clsid ;
				return	true ;
			}
		}
		while ( !sparsExt.IsIndexOverflow() ) ;
	}
	return	false ;
}

// エンコーダー拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusCodec::IsMatchableEncoderFileExtension
		( const wchar_t * pszExt, SString& strMIME, CLSID& clsidEncoder )
{
	SArray<uint8_t>	bufEnumEncoders ;
	UINT			nEncoderCount ;
	Gdiplus::ImageCodecInfo *
		piciEncoders = EnumerateEncoderInfo
							( bufEnumEncoders, nEncoderCount ) ;
	if ( piciEncoders == NULL )
	{
		return	false ;
	}
	for ( UINT i = 0; i < nEncoderCount; i ++ )
	{
		SString			strFileExt = piciEncoders[i].FilenameExtension ;
		SStringParser	sparsExt ;
		sparsExt.AttachString( strFileExt ) ;
		do
		{
			SString	strEnumExt ;
			sparsExt.NextEnclosedString( strEnumExt, L';' ) ;
			//
			SString	strExt = strEnumExt.GetFileExtensionPart() ;
			if ( strExt.CompareNoCase( pszExt ) == 0 )
			{
				strMIME = piciEncoders[i].MimeType ;
				clsidEncoder = piciEncoders[i].Clsid ;
				return	true ;
			}
		}
		while ( !sparsExt.IsIndexOverflow() ) ;
	}
	return	false ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusCodec::IsMatchableDecoderMIMEType
		( const wchar_t * pszMIME, CLSID& clsidDecoder )
{
	SArray<uint8_t>	bufEnumDecoders ;
	UINT			nDecoderCount ;
	Gdiplus::ImageCodecInfo *
		piciDecoders = EnumerateDecoderInfo
							( bufEnumDecoders, nDecoderCount ) ;
	if ( piciDecoders == NULL )
	{
		return	false ;
	}
	for ( UINT i = 0; i < nDecoderCount; i ++ )
	{
		if ( SString::CompareNoCase
				( piciDecoders[i].MimeType, pszMIME ) == 0 )
		{
			clsidDecoder = piciDecoders[i].Clsid ;
			return	true ;
		}
	}
	return	false ;
}

// エンコーダー MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusCodec::IsMatchableEncoderMIMEType
		( const wchar_t * pszMIME, CLSID& clsidEncoder )
{
	SArray<uint8_t>	bufEnumEncoders ;
	UINT			nEncoderCount ;
	Gdiplus::ImageCodecInfo *
		piciEncoders = EnumerateEncoderInfo
							( bufEnumEncoders, nEncoderCount ) ;
	if ( piciEncoders == NULL )
	{
		return	false ;
	}
	for ( UINT i = 0; i < nEncoderCount; i ++ )
	{
		if ( SString::CompareNoCase
				( piciEncoders[i].MimeType, pszMIME ) == 0 )
		{
			clsidEncoder = piciEncoders[i].Clsid ;
			return	true ;
		}
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// GDI+ デコーダ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLGdiplusImageDecoder, SGLImageDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGdiplusImageDecoder::SGLGdiplusImageDecoder( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLGdiplusImageDecoder::~SGLGdiplusImageDecoder( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	CLSID	clsidDecoder ;
	return	m_gdipCodec.IsMatchableDecoderFileExtension( pszExt, clsidDecoder ) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLGdiplusImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	CLSID	clsidDecoder ;
	return	m_gdipCodec.IsMatchableDecoderMIMEType( pszMIME, clsidDecoder ) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusImageDecoder::ReadImage
	( SGLImageObject & image,
		SSystem::SFileInterface & file, size_t nLimitFrames )
{
	return	m_gdipCodec.ReadImage( image, file, nLimitFrames ) ;
}


//////////////////////////////////////////////////////////////////////////
// GDI+ エンコーダ
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLGdiplusImageEncoder, SGLImageEncoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLGdiplusImageEncoder::SGLGdiplusImageEncoder( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLGdiplusImageEncoder::~SGLGdiplusImageEncoder( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////
bool SGLGdiplusImageEncoder::IsMatchableFileExtension
		( const wchar_t * pszExt, SString& strMIME )
{
	CLSID	clsidEncoder ;
	return	m_gdipCodec.IsMatchableEncoderFileExtension
								( pszExt, strMIME, clsidEncoder ) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////
bool SGLGdiplusImageEncoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	CLSID	clsidEncoder ;
	return	m_gdipCodec.IsMatchableEncoderMIMEType( pszMIME, clsidEncoder ) ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////
SGLError SGLGdiplusImageEncoder::WriteImage
	( SSystem::SFileInterface & file,
		SGLImageObject & image,
		const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	return	m_gdipCodec.WriteImage( image, file, pszMIME, pOpt ) ;
}


//////////////////////////////////////////////////////////////////////////////
// IStream 実装
//////////////////////////////////////////////////////////////////////////////

// IStream 作成
//////////////////////////////////////////////////////////////////////////////
IStream * SStreamCOMInterface::CreateStream
	( SSystem::SFileInterface * pFile, bool flagOwner )
{
	return	new SStreamCOMInterface( pFile, flagOwner ) ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SStreamCOMInterface::SStreamCOMInterface
		( SSystem::SFileInterface * pFile, bool flagOwner )
	: m_nRef( 1 ), m_pFile( pFile ), m_flagOwner( flagOwner )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SStreamCOMInterface::~SStreamCOMInterface( void )
{
	if ( m_flagOwner )
	{
		delete	m_pFile ;
		m_pFile = NULL ;
		m_flagOwner = false ;
	}
}

// COM インターフェース取得
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_ISequentialStream)
		|| IsEqualIID(riid, IID_IStream) )
	{
        *ppObj = (IStream*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

// 参照カウンタ加算
//////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE SStreamCOMInterface::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

// 参照カウンタ減少
//////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE SStreamCOMInterface::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::Read
		( void * ptrBuf, ULONG nBytes, ULONG * pReadBytes )
{
	size_t	nReadBytes = m_pFile->Read( ptrBuf, (size_t) nBytes ) ;
	if ( pReadBytes != NULL )
	{
		*pReadBytes = (ULONG) nReadBytes ;
	}
	if ( nReadBytes == 0 )
	{
		return	S_FALSE ;
	}
	return	S_OK ;
}

// ファイルへ書き出し
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::Write
	( const void * ptrBuf, ULONG nBytes, ULONG * pWrittenBytes )
{
	size_t	nWrittenBytes = m_pFile->Write( ptrBuf, (size_t) nBytes ) ;
	if ( pWrittenBytes != NULL )
	{
		*pWrittenBytes = (ULONG) nWrittenBytes ;
	}
	if ( nWrittenBytes == 0 )
	{
		return	S_FALSE ;
	}
	return	S_OK ;
}

// シーク
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::Seek
		( LARGE_INTEGER posDst, DWORD dwOrg, ULARGE_INTEGER * pMovedPos )
{
	if ( !m_pFile->IsSeekable() )
	{
		return	E_FAIL ;
	}
	SFileInterface::SeekOrigin	seekOrg = SFileInterface::FromBegin ;
	switch ( dwOrg )
	{
	case	STREAM_SEEK_SET:
		seekOrg = SFileInterface::FromBegin ;
		break ;
	case	STREAM_SEEK_CUR:
		seekOrg = SFileInterface::FromCurrent ;
		break ;
	case	STREAM_SEEK_END:
		seekOrg = SFileInterface::FromEnd ;
		break ;
	}
	int64_t	posNew = m_pFile->Seek( posDst.QuadPart, seekOrg ) ;
	if ( pMovedPos != NULL )
	{
		pMovedPos->QuadPart = posNew ;
	}
	return	S_OK ;
}

// ファイルサイズ変更
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::SetSize( ULARGE_INTEGER nNewSize )
{
	if ( !m_pFile->IsSeekable() )
	{
		return	E_FAIL ;
	}
	int64_t	pos = m_pFile->GetPosition() ;
	m_pFile->Seek( nNewSize.QuadPart ) ;
	m_pFile->SetEndOfFile() ;
	m_pFile->Seek( pos ) ;
	return	S_OK ;
}

// ファイル複製
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::CopyTo
		( IStream * pStream, ULARGE_INTEGER nBytes,
			ULARGE_INTEGER * pRead, ULARGE_INTEGER * pWritten )
{
	return	E_NOTIMPL ;
}

// 変更の反映
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::Commit( DWORD dwFlags )
{
	return	E_NOTIMPL ;
}

// 変更の破棄
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SStreamCOMInterface::Revert( void )
{
	return	E_NOTIMPL ;
}

// アクセス制限
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::LockRegion
		( ULARGE_INTEGER nOffset, ULARGE_INTEGER nBytes, DWORD dwLockType )
{
	return	E_NOTIMPL ;
}

// アクセス制限解除
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::UnlockRegion
		( ULARGE_INTEGER nOffset, ULARGE_INTEGER nBytes, DWORD dwLockType )
{
	return	E_NOTIMPL ;
}

// ファイルステータス取得
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::Stat( STATSTG * pStats, DWORD dwStatFlag )
{
	if ( pStats == NULL )
	{
		return	E_FAIL ;
	}
	pStats->cbSize.QuadPart = m_pFile->GetLength() ;
	return	S_OK ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
	SStreamCOMInterface::Clone( IStream ** ppStream )
{
	return	E_NOTIMPL ;
}

