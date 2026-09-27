
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>

#if	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_font.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// フォントスタイル
//////////////////////////////////////////////////////////////////////////////

// デフォルト・フォント名
//////////////////////////////////////////////////////////////////////////////
const wchar_t *	SGLFontStyle::StandardFont = L"Default" ;
const wchar_t *	SGLFontStyle::FixedPitchFont = L"FixedDefault" ;

#if	defined(__PLATFORM_WINDOWS__)

// 構造体変換
//////////////////////////////////////////////////////////////////////////////
void SGLFontStyle::ToLogFont( LOGFONT& lf ) const
{
	::eslFillMemory( &lf, 0, sizeof(LOGFONT) ) ;
	//
	lf.lfHeight = nSize ;
	lf.lfWeight = FW_DONTCARE ;
	if ( nStyles & styleBold )
	{
		lf.lfWeight = FW_BOLD ;
	}
	if ( nStyles & styleItalic )
	{
		lf.lfItalic = TRUE ;
	}
	lf.lfCharSet = DEFAULT_CHARSET ;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS ;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS ;
	lf.lfQuality = DEFAULT_QUALITY ;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE ;
	//
	SString	wstrFontFace = SGLFont::RemappedFontOf( pszFace ) ;
	SArray<char>	bufFontFace ;
	const char *	pszFontFace =
			wstrFontFace.EncodeDefaultTo( bufFontFace ) ;
	size_t	nFontFaceLen = bufFontFace.GetLength() ;
	if ( nFontFaceLen > LF_FACESIZE )
	{
		nFontFaceLen = LF_FACESIZE ;
	}
	::eslMoveMemory
		( &(lf.lfFaceName[0]),
			pszFontFace, nFontFaceLen ) ;
}

void SGLFontStyle::FromLogFont( const LOGFONT& lf, SString& strFace )
{
	strFace = lf.lfFaceName ;
	nStyles = 0 ;
	nSize = lf.lfHeight ;
	if ( lf.lfHeight < 0 )
	{
		HDC	hdc = ::CreateCompatibleDC( NULL ) ;
		int	nLPx = ::GetDeviceCaps( hdc, LOGPIXELSY ) ;
		nSize = (- lf.lfHeight * nLPx + 72/2) / 72 ;
		::DeleteDC( hdc ) ;
	}
	pszFace = strFace ;
	if ( lf.lfWeight >= FW_BOLD )
	{
		nStyles |= styleBold ;
	}
	if ( lf.lfItalic )
	{
		nStyles |= styleItalic ;
	}
}

#endif

// シリアライズ（ポインタを除く）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLFontStyle::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	uint32_t	ptrNull = 0 ;
	file.Write( &nStyles, sizeof(uint32_t) ) ;
	file.Write( &nSize, sizeof(uint32_t) ) ;
	file.Write( &ptrNull, sizeof(uint32_t) ) ;
	return	errSuccess ;
}

SSystem::SError SGLFontStyle::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	uint32_t	ptrNull = 0 ;
	file.Read( &nStyles, sizeof(uint32_t) ) ;
	file.Read( &nSize, sizeof(uint32_t) ) ;
	file.Read( &ptrNull, sizeof(uint32_t) ) ;
	pszFace = NULL ;
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// フォント抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLFontObject, SObject )

// フォントを列挙
//////////////////////////////////////////////////////////////////////////////
void SGLFontObject::EnumerateFonts
		( SSystem::SObjectArray<SSystem::SString>& listFonts )
{
	SGLFont::EnumerateFonts( listFonts ) ;
}

// フォントオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SGLFontObject * SGLFontObject::NewFont( const SGLFontStyle& style )
{
	SGLFont *	pFont = new SGLFont ;
	pFont->SetStyle( style ) ;
	return	pFont ;
}

// 文字画像取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLFontObject::GetFontImage
	( SGLImageObject * pCharImage,
		SGLFontMetrics& metrics, uint32_t wch, uint32_t rgbaColor )
{
	SGLError	err = GetMetrics( NULL, 0, metrics, wch ) ;
	if ( err )
	{
		return	err ;
	}
	SArray<uint8_t>	bufChar ;
	size_t		nBufBytes = metrics.rctExterior.w * metrics.rctExterior.h ;
	uint8_t *	pbytBuf = bufChar.GetArray( nBufBytes ) ;
	err = GetMetrics( pbytBuf, nBufBytes, metrics, wch ) ;
	if ( err )
	{
		bufChar.FinishArray() ;
		return	err ;
	}
	uint32_t	nWidth = (uint32_t) metrics.rctExterior.w ;
	uint32_t	nHeight = (uint32_t) metrics.rctExterior.h ;
	pCharImage->CreateImage
		( nWidth, nHeight, formatImageARGB, 32 ) ;
	SGLImageInfo	imginf ;
	uint8_t *	pbytLock =
		pCharImage->LockBuffer( imginf, SGLImageObject::lockWrite ) ;
	//
	for ( uint32_t y = 0; y < nHeight; y ++ )
	{
		uint32_t *	pdwDst = (uint32_t*) pbytLock ;
		for ( uint32_t x = 0; x < nWidth; x ++ )
		{
			*(pdwDst ++) =
				sglPackedColorMul( rgbaColor, *(pbytBuf ++) + 1 ) ;
		}
		pbytLock += imginf.pitchLine ;
	}
	//
	pCharImage->UnlockBuffer( SGLImageObject::lockWrite ) ;
	bufChar.FinishArray() ;
	//
	return	sglErrSuccess ;
}

// 文字列描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLFontObject::DrawText
	( SGLImageObject * pDstImage, int xPos, int yPos,
			const wchar_t * pwszText, uint32_t rgbaColor )
{
	SGLImageBuffer	bufDst ;
	bufDst.ptrBuffer =
		pDstImage->LockBuffer( bufDst, SGLImageObject::lockReadWrite ) ;
	//
	SArray<uint8_t>		bufChar ;
	SGLFontMetrics		metrics ;
	SArray<uint32_t>	bufTemp ;
	SGLImageBuffer		bufImage ;
	//
	SGLError	errAny = sglErrSuccess ;
	for ( size_t i = 0; pwszText[i]; i ++ )
	{
		SGLError	err = GetMetrics( NULL, 0, metrics, pwszText[i] ) ;
		if ( err )
		{
			errAny = err ;
			continue ;
		}
		size_t		nBufBytes = metrics.rctExterior.w * metrics.rctExterior.h ;
		uint8_t *	pbytBuf = bufChar.GetArray( nBufBytes ) ;
		err = GetMetrics( pbytBuf, nBufBytes, metrics, pwszText[i] ) ;
		if ( err )
		{
			xPos += metrics.nWidth ;
			errAny = err ;
			continue ;
		}
		uint32_t *	pdwTemp = bufTemp.GetArray( nBufBytes ) ;
		for ( size_t i = 0; i < nBufBytes; i ++ )
		{
			pdwTemp[i] = sglPackedColorMul( rgbaColor, pbytBuf[i] + 1 ) ;
		}
		bufChar.FinishArray() ;
		//
		bufImage.format = formatImageARGB ;
		bufImage.width = metrics.rctExterior.w ;
		bufImage.height = metrics.rctExterior.h ;
		bufImage.depth = 32 ;
		bufImage.pitchPixel = 4 ;
		bufImage.pitchLine = metrics.rctExterior.w * 4 ;
		bufImage.ptrBuffer = (uint8_t*) pdwTemp ;
		//
		sglBlendImageBuffer
			( bufDst, bufImage,
				xPos + metrics.rctExterior.x,
				yPos + metrics.rctExterior.y ) ;
		//
		bufTemp. FinishArray() ;
		xPos += metrics.nWidth ;
	}
	//
	pDstImage->UnlockBuffer( SGLImageObject::lockReadWrite ) ;
	return	errAny ;
}


//////////////////////////////////////////////////////////////////////////////
// フォント実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO_CAST( SakuraGL::SGLFont, SGLFontObject, m_pFont )
#else
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLFont, SGLFontObject )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLFont::SGLFont( void )
{
	m_pFont = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLFont::~SGLFont( void )
{
	if ( m_flagOwner )
	{
		delete	m_pFont ;
	}
	m_pFont = NULL ;
	m_flagOwner = false ;
}

// フォントを列挙
//////////////////////////////////////////////////////////////////////////////
void SGLFont::EnumerateFonts
		( SSystem::SObjectArray<SSystem::SString>& listFonts )
{
	listFonts.RemoveAll() ;

#if	defined(__COTOPHA__)
	Font::EnumerateFonts( listFonts ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	SGLWindowsFont::EnumerateFonts( listFonts ) ;

#else

#endif

	SSystem::QuickLock() ;
	if ( m_pFontRemap != NULL )
	{
		const size_t	nCount = m_pFontRemap->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			const SString *	pstrName = m_pFontRemap->GetTagAt( i ) ;
			if ( pstrName != NULL )
			{
				listFonts.Add( new SString( *pstrName ) ) ;
			}
		}
	}
	if ( m_pFontStock != NULL )
	{
		const size_t	nCount = m_pFontStock->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			const SString *	pstrName = m_pFontStock->GetTagAt( i ) ;
			if ( pstrName != NULL )
			{
				listFonts.Add( new SString( *pstrName ) ) ;
			}
		}
	}
	SSystem::QuickUnlock() ;
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLFont::SetStyle( const SGLFontStyle& style )
{
#if	defined(__COTOPHA__)
	if ( m_pFont == NULL )
	{
		m_pFont = new Font ;
		m_flagOwner = true ;
	}
	return	m_pFont->SetStyle( style ) ;

#else
	if ( m_flagOwner )
	{
		delete	m_pFont ;
		m_pFont = NULL ;
		m_flagOwner = false ;
	}
	SSystem::QuickLock() ;
	if ( m_pFontStock != NULL )
	{
		SGLFontObject *	pFont = m_pFontStock->GetAs( style.pszFace ) ;
		if ( pFont != NULL )
		{
			SSystem::QuickUnlock() ;
			m_pFont = pFont->NewFont( style ) ;
			if ( m_pFont != NULL )
			{
				m_flagOwner = true ;
				return	sglErrSuccess ;
			}
		}
	}
	SSystem::QuickUnlock() ;

	#if	defined(__PLATFORM_WINDOWS__)
		m_pFont = new SGLWindowsFont ;
		m_flagOwner = true ;
		return	m_pFont->SetStyle( style ) ;
	#elif	defined(__PLATFORM_ANDROID__)
		m_pFont = new SGLAndroidFont ;
		m_flagOwner = true ;
		return	m_pFont->SetStyle( style ) ;
	#else
		#warning no implement font rasterizer
		return	sglErrFailed ;
	#endif
#endif
}

// フォント情報取得・ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLFont::GetMetrics
	( uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, uint32_t wch )
{
	if ( m_pFont == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pFont->GetMetrics
				( pbytRasterized, nBufBytes, metrics, wch ) ;
}


// フォント・リマップ・テーブル
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( SSystem::SStrSortObjectArray<SSystem::SString> *	SGLFont::m_pFontRemap = NULL ) ;
ESL_DLL_DECL( SSystem::SStrSortObjectArray<SGLFontObject> *		SGLFont::m_pFontStock = NULL ) ;

// フォント・リマップ初期化
//////////////////////////////////////////////////////////////////////////////
void SGLFont::InitializeRemapFontTable( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	RegisterRemapFont( SGLFontStyle::StandardFont, L"ＭＳ Ｐゴシック" ) ;
	RegisterRemapFont( SGLFontStyle::FixedPitchFont, L"ＭＳ ゴシック" ) ;
#endif
}

// フォント・リマップ全削除
//////////////////////////////////////////////////////////////////////////////
void SGLFont::FinalizeRemapFontTable( void )
{
	SSystem::QuickLock() ;
	delete	m_pFontRemap ;
	delete	m_pFontStock ;
	m_pFontRemap = NULL ;
	m_pFontStock = NULL ;
	SSystem::QuickUnlock() ;
}

// フォント・リマップ登録
//////////////////////////////////////////////////////////////////////////////
void SGLFont::RegisterRemapFont
	( const wchar_t * pwszName, const wchar_t * pwszRemapped )
{
	SSystem::QuickLock() ;
	if ( m_pFontRemap == NULL )
	{
		m_pFontRemap = new SSystem::SStrSortObjectArray<SSystem::SString> ;
	}
	m_pFontRemap->SetAs( pwszName, new SString(pwszRemapped) ) ;
	SSystem::QuickUnlock() ;
}

// リマップ・フォント取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLFont::RemappedFontOf( const wchar_t * pwszName )
{
	SSystem::QuickLock() ;
	if ( m_pFontRemap != NULL )
	{
		for ( ; ; )
		{
			SString *	pRemapped = m_pFontRemap->GetAs( pwszName ) ;
			if ( pRemapped != NULL )
			{
				pwszName = *pRemapped ;
			}
			else
			{
				break ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	pwszName ;
}

// 特殊フォント登録
//////////////////////////////////////////////////////////////////////////////
void SGLFont::RegisterStockFont
	( const wchar_t * pwszName, SGLFontObject * pFontGenerator )
{
	SSystem::QuickLock() ;
	if ( m_pFontStock == NULL )
	{
		m_pFontStock = new SSystem::SStrSortObjectArray<SGLFontObject> ;
	}
	m_pFontStock->SetAs( pwszName, pFontGenerator ) ;
	SSystem::QuickUnlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// Windows API フォント実装
//////////////////////////////////////////////////////////////////////////////

#if	defined(__PLATFORM_WINDOWS__)

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowsFont, SGLFontObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsFont::SGLWindowsFont( void )
{
	m_hDC = ::CreateCompatibleDC( NULL ) ;
	m_hFont = NULL ;
	m_hDefFont = NULL ;
	m_flagAntialias = true ;
	m_flagMultiSampling = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsFont::~SGLWindowsFont( void )
{
	if ( m_hFont && m_hDC )
	{
		::SelectObject( m_hDC, m_hDefFont ) ;
		::DeleteObject( m_hFont ) ;
	}
	if ( m_hDC != NULL )
	{
		::DeleteDC( m_hDC ) ;
	}
	m_hDC = NULL ;
	m_hFont = NULL ;
	m_hDefFont = NULL ;
}

// フォントを列挙
//////////////////////////////////////////////////////////////////////////////
void SGLWindowsFont::EnumerateFonts
		( SSystem::SObjectArray<SSystem::SString>& listFonts )
{
	HDC		hDC = ::CreateCompatibleDC( NULL ) ;
	LOGFONT	lfLogFont ;
	eslFillMemory( &lfLogFont, 0, sizeof(LOGFONT) ) ;
	lfLogFont.lfCharSet = DEFAULT_CHARSET ;
	//
	::EnumFontFamiliesEx
		( hDC, &lfLogFont,
			&SGLWindowsFont::EnumFontFamExProc, (LPARAM) &listFonts, 0 ) ;
	//
	::DeleteDC( hDC ) ;
}

int CALLBACK SGLWindowsFont::EnumFontFamExProc
		( const LOGFONT *lplfe,
			const TEXTMETRIC *lptme,
			DWORD FontType, LPARAM lParam )
{
	SObjectArray<SString> *
		pListFont = (SObjectArray<SString>*) lParam ;

	SString *	pstrFaceName = new SString( lplfe->lfFaceName ) ;
	for ( size_t i = 0; i < pListFont->GetLength(); i ++ )
	{
		SString *	pstrFont = pListFont->GetAt( i ) ;
		if ( (pstrFont != nullptr)
			&& (*pstrFont == *pstrFaceName) )
		{
			delete	pstrFaceName ;
			return	TRUE ;
		}
	}
	pListFont->Add( pstrFaceName ) ;
	return	TRUE ;
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsFont::SetStyle( const SGLFontStyle& style )
{
	if ( m_hFont != NULL )
	{
		::SelectObject( m_hDC, m_hDefFont ) ;
		::DeleteObject( m_hFont ) ;
	}
	m_flagAntialias = !(style.nStyles & SGLFontStyle::styleNoSmooth) ;
	m_flagMultiSampling = ((style.nStyles & SGLFontStyle::styleHighDefinition) != 0) ;
	//
	LOGFONT	lfLogFont ;
	style.ToLogFont( lfLogFont ) ;
	if ( m_flagAntialias && m_flagMultiSampling )
	{
		lfLogFont.lfHeight *= 2 ;
		lfLogFont.lfWidth *= 2 ;
		lfLogFont.lfQuality = CLEARTYPE_QUALITY ;
	}
	m_hFont = ::CreateFontIndirect( &lfLogFont ) ;
	m_hDefFont = (HFONT) ::SelectObject( m_hDC, m_hFont ) ;
	::GetTextMetrics( m_hDC, &m_tmMetrics ) ;
	//
	return	sglErrSuccess ;
}

// フォント情報取得・ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsFont::GetMetrics
	( uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, uint32_t wch )
{
	if ( m_hFont == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// フォントメトリクス取得
	//
	metrics.nFlags = 0 ;
	metrics.nAscent = m_tmMetrics.tmAscent ;
	metrics.nDescent = m_tmMetrics.tmDescent ;
	metrics.nLeading = m_tmMetrics.tmExternalLeading ;
	metrics.nHeight = m_tmMetrics.tmHeight ;
	//
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		int	nCharWidth = 0 ;
		GetCharWidth32W( m_hDC, wch, wch, &nCharWidth ) ;
		metrics.nWidth = nCharWidth ;
	}
	else
	{
		if ( wch >= 0x80 )
		{
			wch = ESLCharset::ShiftJISCodeFromJIS
					( (WORD) ESLCharset::JISCodeFromUnicode( wch ) ) ;
		}
		int	nCharWidth = 0 ;
		GetCharWidth32A( m_hDC, wch, wch, &nCharWidth ) ;
		metrics.nWidth = nCharWidth ;
	}
	//
	// 文字の外接矩形取得
	//
	DWORD (WINAPI *apiGetGlyphOutline)
		(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2 *);
	const MAT2		mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
	GLYPHMETRICS	gm ;
	DWORD			dwGlyphBufSize ;
	int				nGlyphDepth = 8 ;
	if ( g_infoPlatform.runtimeOS == platformOS_WindowsNT )
	{
		apiGetGlyphOutline = &GetGlyphOutlineW ;
	}
	else
	{
		apiGetGlyphOutline = &GetGlyphOutlineA ;
	}
	dwGlyphBufSize = GDI_ERROR ;
	if ( m_flagAntialias )
	{
		dwGlyphBufSize =
			apiGetGlyphOutline
				( m_hDC, wch, GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat2 ) ;
	}
	if ( dwGlyphBufSize == GDI_ERROR )
	{
		nGlyphDepth = 1 ;
		dwGlyphBufSize =
			apiGetGlyphOutline
				( m_hDC, wch, GGO_BITMAP, &gm, 0, NULL, &mat2 ) ;
		if ( dwGlyphBufSize == GDI_ERROR )
		{
			metrics.rctExterior.x = 0 ;
			metrics.rctExterior.y = 0 ;
			metrics.rctExterior.w = 0 ;
			metrics.rctExterior.h = 0 ;
			return	sglErrSuccess ;
		}
	}
	metrics.rctExterior.x = gm.gmptGlyphOrigin.x ;
	metrics.rctExterior.y = metrics.nAscent - gm.gmptGlyphOrigin.y ;
	metrics.rctExterior.w = gm.gmBlackBoxX ;
	metrics.rctExterior.h = gm.gmBlackBoxY ;
	//
	DWORD	dwLineBytes =
				((gm.gmBlackBoxX * nGlyphDepth + 0x1F) & ~0x1F) >> 3 ;
	if ( dwLineBytes * gm.gmBlackBoxY < dwGlyphBufSize )
	{
		metrics.rctExterior.h = dwGlyphBufSize / dwLineBytes ;
	}
	if ( dwGlyphBufSize < dwLineBytes * gm.gmBlackBoxY )
	{
		dwGlyphBufSize = dwLineBytes * gm.gmBlackBoxY ;
	}
	if ( m_flagAntialias && m_flagMultiSampling )
	{
		metrics.nAscent /= 2 ;
		metrics.nDescent /= 2 ;
		metrics.nLeading /= 2 ;
		metrics.nHeight /= 2 ;
		metrics.nWidth /= 2 ;
	}
	//
	// ラスタライズ画像取得
	//
	if ( pbytRasterized != NULL )
	{
		if ( m_bufGlyphOutline.GetLength() < dwGlyphBufSize )
		{
			m_bufGlyphOutline.SetLength( dwGlyphBufSize ) ;
		}
		uint8_t *	pbytBuf = m_bufGlyphOutline.GetArray() ;
		if ( m_flagAntialias )
		{
			dwGlyphBufSize =
				apiGetGlyphOutline
					( m_hDC, wch, GGO_GRAY8_BITMAP, &gm,
						(DWORD) m_bufGlyphOutline.GetLength(), pbytBuf, &mat2 ) ;
			if ( dwGlyphBufSize != GDI_ERROR )
			{
				if ( m_flagMultiSampling )
				{
					size_t	nWidth = ((metrics.rctExterior.w + metrics.rctExterior.x + 1) >> 1)
										- (metrics.rctExterior.x >> 1) ;
					size_t	nHeight = ((metrics.rctExterior.h + metrics.rctExterior.y + 1) >> 1)
										- (metrics.rctExterior.y >> 1) ;
					NormalizeGlyphOutlineGray16
						( pbytRasterized, nWidth, nHeight, pbytBuf, metrics.rctExterior ) ;
					m_bufGlyphOutline.FinishArray() ;
					//
					metrics.rctExterior.x >>= 1 ;
					metrics.rctExterior.y >>= 1 ;
					metrics.rctExterior.w = (int32_t) nWidth ;
					metrics.rctExterior.h = (int32_t) nHeight ;
				}
				else
				{
					NormalizeGlyphOutlineGray8
						( pbytRasterized, pbytBuf,
							metrics.rctExterior.w, metrics.rctExterior.h ) ;
					m_bufGlyphOutline.FinishArray() ;
				}
				return	sglErrSuccess ;
			}
		}
		dwGlyphBufSize =
			apiGetGlyphOutline
				( m_hDC, wch, GGO_BITMAP, &gm,
					(DWORD) m_bufGlyphOutline.GetLength(), pbytBuf, &mat2 ) ;
		if ( dwGlyphBufSize != GDI_ERROR )
		{
			NormalizeGlyphOutlineGray1
				( pbytRasterized, pbytBuf,
					metrics.rctExterior.w, metrics.rctExterior.h ) ;
			m_bufGlyphOutline.FinishArray() ;
			return	sglErrSuccess ;
		}
		m_bufGlyphOutline.FinishArray() ;
		return	sglErrFailed ;
	}
	else
	{
		if ( m_flagAntialias && m_flagMultiSampling )
		{
			size_t	nWidth = ((metrics.rctExterior.w + metrics.rctExterior.x + 1) >> 1)
								- (metrics.rctExterior.x >> 1) ;
			size_t	nHeight = ((metrics.rctExterior.h + metrics.rctExterior.y + 1) >> 1)
								- (metrics.rctExterior.y >> 1) ;
			metrics.rctExterior.x >>= 1 ;
			metrics.rctExterior.y >>= 1 ;
			metrics.rctExterior.w = (int32_t) nWidth ;
			metrics.rctExterior.h = (int32_t) nHeight ;
		}
	}
	return	sglErrSuccess ;
}


void SGLWindowsFont::NormalizeGlyphOutlineGray16
	( uint8_t * pbytDst,
		size_t nWidth, size_t nHeight,
		const uint8_t * pbytSrc, const SGLImageRect& rectSrcBuf )
{
	const int		xOffset = - (rectSrcBuf.x & 1) ;
	const int		yOffset = - (rectSrcBuf.y & 1) ;
	const uint8_t * pbytSrcLine = pbytSrc ;
	const ssize_t	pitchSrcLine = (rectSrcBuf.w + 0x03) & ~0x03 ;
	const SGLSize	sizeSrcBuf = rectSrcBuf.GetSize() ;
	for ( int y = 0; y < (int) nHeight; y ++ )
	{
		const uint8_t * pbytSrcNext = pbytSrcLine + yOffset * pitchSrcLine + xOffset ;
		const int	y2 = y * 2 + yOffset ;
		for ( int x = 0; x < (int) nWidth; x ++ )
		{
			const int	x2 = x * 2 + xOffset ;
			int	g = 0 ;
			if ( x2 >= 0 )
			{
				ESLAssert( x2 < sizeSrcBuf.w ) ;
				if ( y2 >= 0 )
				{
					ESLAssert( y2 < sizeSrcBuf.h ) ;
					g += pbytSrcNext[0] ;
				}
				if ( y2 + 1 < sizeSrcBuf.h )
				{
					g += pbytSrcNext[pitchSrcLine] ;
				}
			}
			if ( x2 + 1 < sizeSrcBuf.w )
			{
				if ( y2 >= 0 )
				{
					ESLAssert( y2 < sizeSrcBuf.h ) ;
					g += pbytSrcNext[1] ;
				}
				if ( y2 + 1 < sizeSrcBuf.h )
				{
					g += pbytSrcNext[pitchSrcLine + 1] ;
				}
			}
			pbytSrcNext += 2 ;
			*(pbytDst ++) = (uint8_t) esl_clampi( g, 0, 0xFF ) ;
		}
		pbytSrcLine += pitchSrcLine * 2 ;
	}
}

void SGLWindowsFont::NormalizeGlyphOutlineGray8
	( uint8_t * pbytDst, const uint8_t * pbytSrc,
					size_t nWidth, size_t nHeight )
{
	static const BYTE	bytTone[0x100] =
	{
		0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
		0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C,
		0x40, 0x44, 0x48, 0x4C, 0x50, 0x55, 0x59, 0x5D,
		0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
		0x81, 0x85, 0x89, 0x8D, 0x91, 0x95, 0x99, 0x9D,
		0xA1, 0xA5, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
		0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD6, 0xDA, 0xDE,
		0xE2, 0xE6, 0xEA, 0xEE, 0xF2, 0xF6, 0xFA, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	} ;
	size_t	nLineBytes = (nWidth + 0x03) & ~0x03 ;
	for ( size_t y = 0; y < nHeight; y ++ )
	{
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			*(pbytDst ++) = bytTone[pbytSrc[x]] ;
		}
		pbytSrc += nLineBytes ;
	}
}

void SGLWindowsFont::NormalizeGlyphOutlineGray1
	( uint8_t * pbytDst, const uint8_t * pbytSrc,
					size_t nWidth, size_t nHeight )
{
	size_t	nLineBytes = ((nWidth + 0x1F) & ~0x1F) >> 3 ;
	for ( size_t y = 0; y < nHeight; y ++ )
	{
		uint8_t	nMask = 0x80 ;
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			*(pbytDst ++) =
				((-(int)(pbytSrc[x >> 3] & nMask)) >> 8) & 0xFF ;
			nMask = (nMask << 7) | (nMask >> 1) ;
		}
		pbytSrc += nLineBytes ;
	}
}

#endif

