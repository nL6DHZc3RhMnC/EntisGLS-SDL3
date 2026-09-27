
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (c) 2002-2014 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#endif


/*****************************************************************************
						GDI フォントオブジェクト
 ****************************************************************************/

// Implement class information
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EFontObject, ESLObject )

// Constructer
//////////////////////////////////////////////////////////////////////////////
EFontObject::EFontObject( void )
	: m_hFont( NULL )
{
	m_LogFont.lfHeight = 0 ;
	m_LogFont.lfWidth = 0 ;
	m_LogFont.lfEscapement = 0 ;
	m_LogFont.lfOrientation = 0 ;
	m_LogFont.lfWeight = FW_DONTCARE ;
	m_LogFont.lfItalic = FALSE ;
	m_LogFont.lfUnderline = FALSE ;
	m_LogFont.lfStrikeOut = FALSE ;
	m_LogFont.lfCharSet = DEFAULT_CHARSET ;
	m_LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS ;
	m_LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS ;
	m_LogFont.lfQuality = DEFAULT_QUALITY ;
	m_LogFont.lfPitchAndFamily = (DEFAULT_PITCH | FF_DONTCARE) ;
	::eslFillMemory( m_LogFont.lfFaceName, 0, sizeof(m_LogFont.lfFaceName) ) ;
}

EFontObject::EFontObject
( int nHeight, int nWidth, int nEscapement, int nOrientation,
	int nWeight, bool fItalic, bool fUnderline, bool fStrikeOut,
	BYTE fCharSet, BYTE fOutPrecision, BYTE fClipPrecision,
	BYTE fQuality, BYTE fPitchAndFamily, const char * pszFaceName )
	: m_hFont( NULL )
{
	m_LogFont.lfHeight = nHeight ;
	m_LogFont.lfWidth = nWidth ;
	m_LogFont.lfEscapement = nEscapement ;
	m_LogFont.lfOrientation = nOrientation ;
	m_LogFont.lfWeight = nWeight ;
	m_LogFont.lfItalic = fItalic ;
	m_LogFont.lfUnderline = fUnderline ;
	m_LogFont.lfStrikeOut = fStrikeOut ;
	m_LogFont.lfCharSet = fCharSet ;
	m_LogFont.lfOutPrecision = fOutPrecision ;
	m_LogFont.lfClipPrecision = fClipPrecision ;
	m_LogFont.lfQuality = fQuality ;
	m_LogFont.lfPitchAndFamily = fPitchAndFamily ;
	if ( pszFaceName != NULL )
	{
#if	_MSC_VER >= 1400
		::strcpy_s
			( m_LogFont.lfFaceName,
				sizeof(m_LogFont.lfFaceName), pszFaceName ) ;
#else
		::strcpy( m_LogFont.lfFaceName, pszFaceName ) ;
#endif
	}
	else
	{
		::eslFillMemory
			( m_LogFont.lfFaceName, 0, sizeof(m_LogFont.lfFaceName) ) ;
	}
}

EFontObject::EFontObject( const LOGFONT & logfont )
	: m_hFont( NULL )
{
	m_LogFont = logfont ;
}

// Destructer
//////////////////////////////////////////////////////////////////////////////
EFontObject::~EFontObject( void )
{
	Delete( ) ;
}

// Font size
//////////////////////////////////////////////////////////////////////////////
int EFontObject::GetSize( int * pWidth ) const
{
	if ( pWidth != NULL )
		*pWidth = m_LogFont.lfWidth ;
	return	m_LogFont.lfHeight ;
}

void EFontObject::SetSize( int nHeight, int nWidth )
{
	m_LogFont.lfHeight = nHeight ;
	m_LogFont.lfWidth = nWidth ;
}

// Weight
//////////////////////////////////////////////////////////////////////////////
int EFontObject::GetWeight( void ) const
{
	return	m_LogFont.lfWeight ;
}

void EFontObject::SetWeight( int nWeight )
{
	m_LogFont.lfWeight = nWeight ;
}

// Italic
//////////////////////////////////////////////////////////////////////////////
bool EFontObject::IsItalic( void ) const
{
	return	(m_LogFont.lfItalic != FALSE) ;
}

void EFontObject::SetItalic( bool fItalic )
{
	m_LogFont.lfItalic = fItalic ;
}

// Underline
//////////////////////////////////////////////////////////////////////////////
bool EFontObject::IsUnderline( void ) const
{
	return	(m_LogFont.lfUnderline != FALSE) ;
}

void EFontObject::SetUnderline( bool fUnderline )
{
	m_LogFont.lfUnderline = fUnderline ;
}

// Strike out
//////////////////////////////////////////////////////////////////////////////
bool EFontObject::IsStrikeOut( void ) const
{
	return	(m_LogFont.lfStrikeOut != FALSE) ;
}

void EFontObject::SetStrikeOut( bool fStrikeOut )
{
	m_LogFont.lfStrikeOut = fStrikeOut ;
}

// Character set
//////////////////////////////////////////////////////////////////////////////
BYTE EFontObject::GetCharSet( void ) const
{
	return	m_LogFont.lfCharSet ;
}

void EFontObject::SetCharSet( BYTE fCharSet )
{
	m_LogFont.lfCharSet = fCharSet ;
}

// Pitch and family
//////////////////////////////////////////////////////////////////////////////
BYTE EFontObject::GetPitchAndFamily( void ) const
{
	return	m_LogFont.lfPitchAndFamily ;
}

void EFontObject::SetPitchAndFamily( BYTE fPitchAndFamily )
{
	m_LogFont.lfPitchAndFamily = fPitchAndFamily ;
}

// Font face name
//////////////////////////////////////////////////////////////////////////////
const char * EFontObject::GetFaceName( void ) const
{
	return	m_LogFont.lfFaceName ;
}

void EFontObject::SetFaceName( const char * pszFaceName )
{
	if( pszFaceName != NULL )
	{
#if	_MSC_VER >= 1400
		::strcpy_s
			( m_LogFont.lfFaceName,
				sizeof(m_LogFont.lfFaceName), pszFaceName ) ;
#else
		::strcpy( m_LogFont.lfFaceName, pszFaceName ) ;
#endif
	}
	else
	{
		::eslFillMemory
			( m_LogFont.lfFaceName, 0, sizeof(m_LogFont.lfFaceName) ) ;
	}
}

// Create font object
//////////////////////////////////////////////////////////////////////////////
HFONT EFontObject::Create( void )
{
	Delete( ) ;
	return	m_hFont = ::CreateFontIndirect( &m_LogFont ) ;
}

// Delete font object
//////////////////////////////////////////////////////////////////////////////
void EFontObject::Delete( void )
{
	if ( m_hFont != NULL )
	{
		if ( ::DeleteObject( m_hFont ) )
			m_hFont = NULL ;
	}
}


/*****************************************************************************
						文字描画オブジェクト
 ****************************************************************************/

// Implement class information
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ERealFontImage, ESLObject )

bool	ERealFontImage::m_fEnableSmoothing = true ;

// Constructer
//////////////////////////////////////////////////////////////////////////////
ERealFontImage::ERealFontImage( void )
	: m_pFont( NULL ),
		m_hDC( NULL ), m_fontText( NULL ), m_nLineHeight( 16 ),
		m_nIndentWidth( 0 ), m_nFontPitch( 0 ),
		m_minHyphening( 6 ), m_maxWordWrapping( 3 )
{
	m_rectView.left = 0 ;
	m_rectView.top = 0 ;
	m_rectView.right = 640 ;
	m_rectView.bottom = 480 ;
	m_colorText.dwPixelCode = 0x00FFFFFF ;
	m_nTransparency = 0 ;
	m_posCursor.x = 0 ;
	m_posCursor.y = 0 ;
	//
	m_listCharacters = new EObjArray<EImageSprite> ;
	//
	// Get OS type (Windows or NT)
	//
#if	_MSC_VER < 1800
	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	::GetVersionEx( &osvi );
#endif
	//
	// Whether it can use Unicode APIs?
	//
#if	_MSC_VER >= 1800
	if ( IsWindowsXPOrGreater() )
#else
	if( osvi.dwPlatformId != VER_PLATFORM_WIN32_NT )
#endif
	{
		m_fCanUseUnicode = false ;
		m_apiGetGlyphOutLine = &::GetGlyphOutlineA ;
		m_apiGetCharWidth = &::GetCharWidthA ;
		m_apiGetCharABCWidths = &::GetCharABCWidthsA ;
	}
	else
	{
		m_fCanUseUnicode = true ;
		m_apiGetGlyphOutLine = &::GetGlyphOutlineW ;
		m_apiGetCharWidth = &::GetCharWidthW ;
		m_apiGetCharABCWidths = &::GetCharABCWidthsW ;
	}
	//
	SetProhibitChar( L",.!?;:)]，．、。！？；：」】）〕｝〉》』"
						L"ぁぃぅぇぉっゃゅょァィゥェォッャュョー～" ) ;
	//
	m_fVerticalWriting = false ;
	m_fSmoothing = true ;
}

// Destructer
//////////////////////////////////////////////////////////////////////////////
ERealFontImage::~ERealFontImage( void )
{
	delete	m_pFont ;
	if ( m_hDC != NULL )
	{
		::DeleteDC( m_hDC ) ;
		m_hDC = NULL ;
	}
	delete	m_listCharacters ;
}

// Get text width
//////////////////////////////////////////////////////////////////////////////
unsigned int ERealFontImage::GetTextWidth( const wchar_t * pwszText )
{
	if ( pwszText == NULL )
	{
		return	0 ;
	}
	if ( m_pFont != NULL )
	{
		unsigned int	nTextWidth = 0 ;
		for ( int i = 0; pwszText[i]; i ++ )
		{
			SakuraGL::SGLFontMetrics	metrics ;
			m_pFont->GetMetrics( NULL, 0, metrics, pwszText[i] ) ;
			nTextWidth += metrics.nWidth ;
		}
		return	nTextWidth ;
	}
	//
	// If can't use Unicode API, convert to Shift-JIS code
	//
	EString			strJiscodeBuf ;
	EWideString		wstrJiscodeBuf ;
	if ( !m_fCanUseUnicode )
	{
		//
		// Convert to Shift-JIS code
		//
		strJiscodeBuf = pwszText ;
		const char *	pszSrcText = strJiscodeBuf ;
		wchar_t *	pwszBuffer =
			wstrJiscodeBuf.GetBuffer( strJiscodeBuf.GetLength() + 1 );
		//
		unsigned char	chrNext ;
		for ( ; ; )
		{
			chrNext = *(pszSrcText ++) ;
			if( chrNext == '\0' )	break ;
			if( ::IsDBCSLeadByte( chrNext ) )
			{
				// double byte character code
				*(pwszBuffer ++) =
					(((wchar_t)chrNext) << 8)
						| *((unsigned char*)(pszSrcText ++)) ;
			}
			else
			{
				// single byte character code
				*(pwszBuffer ++) = chrNext ;
			}
		}
		*(pwszBuffer ++) = L'\0' ;
		//
		wstrJiscodeBuf.ReleaseBuffer( ) ;
		pwszText = wstrJiscodeBuf ;
	}
	//
	// Create Device-Context
	if ( m_hDC == NULL )
	{
		m_hDC = ::CreateCompatibleDC( NULL ) ;
	}
	HFONT	hOldFont =
		(HFONT)::SelectObject( m_hDC, (HGDIOBJ)m_fontText ) ;
	//
	// Get size of all character
	unsigned int	nTextWidth = 0 ;
	MAT2			mat2 = { { 1, 0 }, { 0, 0 }, { 0, 0 }, { 1, 0 } } ;
	for ( int i = 0; pwszText[i] != L'\0'; i ++ )
	{
		int	nCharWidth ;
		(*m_apiGetCharWidth)( m_hDC, pwszText[i], pwszText[i], &nCharWidth ) ;
		nTextWidth += nCharWidth ;
	}
	//
	// Finish the function
	::SelectObject( m_hDC, (HGDIOBJ)hOldFont ) ;

	return	nTextWidth ;
}

// Scaling of pixel value of glyph outline
//////////////////////////////////////////////////////////////////////////////
static void ScalingOfGlyphOutline( PEGL_IMAGE_INFO pImageInf )
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
	void *	ptrImageArray = pImageInf->ptrImageArray ;
	DWORD	dwBytesPerLine = pImageInf->dwBytesPerLine ;
	DWORD	dwWidth = pImageInf->dwImageWidth ;
	DWORD	dwHeight = pImageInf->dwImageHeight ;
	if ( (dwWidth == 0) || (dwHeight == 0) )
		return ;
	//
	__asm
	{
		mov		esi, ptrImageArray ;
		mov		edi, dwHeight
LoopVert:	push	esi
			;
			mov		ecx, dwWidth
			shr		ecx, 1
			jz		LoopHorzExit
LoopHorz:		movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [esi + 1]
				mov		al, BYTE PTR bytTone[eax]
				mov		dl, BYTE PTR bytTone[edx]
				mov		BYTE PTR [esi], al
				mov		BYTE PTR [esi + 1], dl
				add		esi, 2
				dec		ecx
			jnz		LoopHorz
LoopHorzExit:
			test	dwWidth, 0001H
			jz		Label1
				movzx	eax, BYTE PTR [esi]
				mov		al, BYTE PTR bytTone[eax]
				mov		BYTE PTR [esi], al
Label1:		;
			pop		esi
			add		esi, dwBytesPerLine
			dec		edi
		jnz		LoopVert
	}
}

// Revolve image
//////////////////////////////////////////////////////////////////////////////
EImageSprite * ERealFontImage::RevolveCharacter( EImageSprite * pis )
{
	PEGL_IMAGE_INFO	pSrc = *pis ;
	EImageSprite *		pisRev = CreateNewCharacter( ) ;
	pisRev->CreateImage
		( pSrc->fdwFormatType,
			pSrc->dwImageHeight, pSrc->dwImageWidth, 8, NULL ) ;
	PEGL_IMAGE_INFO	pDst = *pisRev ;
	//
	for ( DWORD x = 0; x < pSrc->dwImageWidth; x ++ )
	{
		const BYTE *	pbytSrcLine =
			((const BYTE *) pSrc->ptrImageArray) + x ;
		BYTE *	pbytDstLine =
			((BYTE*) pDst->ptrImageArray) + x * pDst->dwBytesPerLine ;
		//
		DWORD	dwHeight = pSrc->dwImageHeight ;
		DWORD	dwBytesPerLine = pSrc->dwBytesPerLine ;
		//
		for ( int y = dwHeight - 1; y >= 0; y -- )
		{
			pbytDstLine[y] = *pbytSrcLine ;
			pbytSrcLine += dwBytesPerLine ;
		}
	}
	//
	delete	pis ;
	return	pisRev ;
}

// Create image from bitmap
//////////////////////////////////////////////////////////////////////////////
EImageSprite * ERealFontImage::CreateFromBitmap
	( const BITMAPINFO * pbmi, const void * ptrBitmap )
{
	EImageSprite *	pis = CreateNewCharacter( ) ;
	pis->CreateImage
		( EIF_GRAY_BITMAP,
			pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biHeight, 8 ) ;
	PEGL_IMAGE_INFO	pDst = *pis ;
	//
	PBYTE	ptrDstLine ;
	const BYTE *	ptrSrcLine ;
	DWORD	dwDstLineBytes, dwSrcLineBytes ;
	ptrDstLine = (PBYTE) pDst->ptrImageArray ;
	dwDstLineBytes = pDst->dwBytesPerLine ;
	ptrSrcLine = (const BYTE *) ptrBitmap ;
	if ( pbmi->bmiHeader.biSizeImage != 0 )
	{
		dwSrcLineBytes =
			pbmi->bmiHeader.biSizeImage / pbmi->bmiHeader.biHeight ;
	}
	else
	{
		dwSrcLineBytes =
			((pbmi->bmiHeader.biWidth
				* pbmi->bmiHeader.biBitCount + 0x1F) >> 3) & (~0x03) ;
	}
	DWORD	dwHeight, dwWidth ;
	dwHeight = pDst->dwImageHeight ;
	dwWidth = pDst->dwImageWidth ;
	for ( DWORD y = 0; y < dwHeight; y ++ )
	{
		if ( pbmi->bmiHeader.biBitCount == 1 )
		{
			int		nOffset = 0 ;
			const BYTE *	ptrSrcNext = ptrSrcLine ;
			BYTE	bytSrc = *(ptrSrcNext ++) ;
			for ( DWORD x = 0; x < dwWidth; x ++ )
			{
				ptrDstLine[x] = ((SBYTE) bytSrc) >> 7 ;
				bytSrc <<= 1 ;
				if ( ++ nOffset >= 8 )
				{
					bytSrc = *(ptrSrcNext ++) ;
					nOffset = 0 ;
				}
			}
		}
		else
		{
			for ( DWORD x = 0; x < dwWidth; x ++ )
			{
				ptrDstLine[x] = ptrSrcLine[x] ;
			}
		}
		ptrDstLine += dwDstLineBytes ;
		ptrSrcLine += dwSrcLineBytes ;
	}
	//
	return	pis ;
}

// Drawt text (multi-line text)
//////////////////////////////////////////////////////////////////////////////
unsigned int ERealFontImage::DrawText( const wchar_t * pwszText )
{
	if ( pwszText == NULL )
	{
		return	0 ;
	}
	//
	// If it can't use Unicode API, convert to Shift-JIS code
	//
	EString		strJiscodeBuf ;
	EWideString	wstrJiscodeBuf ;
	if ( !m_fCanUseUnicode && (m_pFont == NULL) )
	{
		//
		// Convert to Shift-JIS code
		//
		strJiscodeBuf = pwszText ;
		const char *	pszSrcText = strJiscodeBuf ;
		wchar_t *	pwszBuffer =
			wstrJiscodeBuf.GetBuffer( strJiscodeBuf.GetLength() + 1 ) ;
		//
		unsigned char	chrNext ;
		for ( ; ; )
		{
			chrNext = *(pszSrcText ++) ;
			if( chrNext == '\0' )	break ;
			if( ::IsDBCSLeadByte( chrNext ) )
			{
				// double byte character code
				*(pwszBuffer ++) =
					(((wchar_t)chrNext) << 8)
						| *((unsigned char*)(pszSrcText ++)) ;
			}
			else
			{
				// single byte character code
				*(pwszBuffer ++) = chrNext ;
			}
		}
		*(pwszBuffer ++) = L'\0' ;
		//
		wstrJiscodeBuf.ReleaseBuffer( ) ;
		pwszText = wstrJiscodeBuf ;
	}
	//
	//	Preparing the function
	//
	unsigned int	nNullLineCount = 0 ;
	//
	// Create Device-Context
	HFONT	hOldFont = NULL ;
	if ( m_pFont == NULL )
	{
		if ( m_hDC == NULL )
		{
			m_hDC = ::CreateCompatibleDC( NULL ) ;
		}
		hOldFont =
			(HFONT)::SelectObject( m_hDC, (HGDIOBJ)m_fontText ) ;
	}
	//
	// Get font metrix
	unsigned int	nFontDescent = 0 ;
	if ( m_pFont != NULL )
	{
		SakuraGL::SGLFontMetrics	metrics ;
		m_pFont->GetMetrics( NULL, 0, metrics, pwszText[0] ) ;
		nFontDescent = metrics.nDescent ;
	}
	else
	{
		TEXTMETRIC	TextMat ;
		if( ::GetTextMetrics( m_hDC, &TextMat ) )
		{
			nFontDescent = TextMat.tmDescent ;
		}
	}
	//
	// Get string length
	int		i, nTextLen = 0 ;
	while( pwszText[nTextLen] != L'\0' )	nTextLen ++ ;
	//
	// Get size of all character
	EGL_SIZE *	pCharSizes = new EGL_SIZE[nTextLen] ;
	DWORD		dwMaxBufferSize = 0 ;
	MAT2		mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
	if ( m_pFont != NULL )
	{
		for ( i = 0; i < nTextLen; i ++ )
		{
			SakuraGL::SGLFontMetrics	metrics ;
			pCharSizes[i].w = 0 ;
			pCharSizes[i].h = 0 ;
			if ( !m_pFont->GetMetrics( NULL, 0, metrics, pwszText[i] ) )
			{
				DWORD	dwBufSize =
					metrics.rctExterior.w * metrics.rctExterior.h ;
				if( dwBufSize > dwMaxBufferSize )
				{
					dwMaxBufferSize = dwBufSize ;
				}
				pCharSizes[i].w = metrics.nWidth ;
				pCharSizes[i].h = metrics.nHeight ;
			}
		}
	}
	else
	{
		for ( i = 0; i < nTextLen; i ++ )
		{
			GLYPHMETRICS	gm ;
			DWORD	dwBufSize = (*m_apiGetGlyphOutLine)
				( m_hDC, pwszText[i], GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat2 ) ;
			if ( (signed int)dwBufSize <= 0 )
			{
				dwBufSize = (*m_apiGetGlyphOutLine)
					( m_hDC, pwszText[i], GGO_BITMAP, &gm, 0, NULL, &mat2 ) ;
			}
			DWORD	dwLineBytes =
						((gm.gmBlackBoxX * 8 + 0x1F) & ~0x1F) >> 3 ;
			if ( dwBufSize < dwLineBytes * gm.gmBlackBoxY )
			{
				dwBufSize = dwLineBytes * gm.gmBlackBoxY ;
			}
			if( (int)dwBufSize > (int)dwMaxBufferSize )
				dwMaxBufferSize = dwBufSize ;
			(*m_apiGetCharWidth)
				( m_hDC, pwszText[i], pwszText[i], (int*)&pCharSizes[i].w ) ;
			pCharSizes[i].h = gm.gmBlackBoxY ;
		}
	}
	//
	// Get hyphen("-") size
	EGL_SIZE	sizeHyphen ;
	if ( m_pFont != NULL )
	{
		SakuraGL::SGLFontMetrics	metrics ;
		sizeHyphen.w = 0 ;
		sizeHyphen.h = 0 ;
		if ( !m_pFont->GetMetrics( NULL, 0, metrics, L'-' ) )
		{
			DWORD	dwBufSize =
				metrics.rctExterior.w * metrics.rctExterior.h ;
			if( dwBufSize > dwMaxBufferSize )
			{
				dwMaxBufferSize = dwBufSize ;
			}
			sizeHyphen.w = metrics.nWidth ;
			sizeHyphen.h = metrics.nHeight ;
		}
	}
	else
	{
		GLYPHMETRICS	gm ;
		DWORD	dwBufSize = (*m_apiGetGlyphOutLine)
			( m_hDC, '-', GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat2 ) ;
		if ( (signed int)dwBufSize <= 0 )
		{
			dwBufSize = (*m_apiGetGlyphOutLine)
				( m_hDC, '-', GGO_BITMAP, &gm, 0, NULL, &mat2 ) ;
		}
		if( (int)dwBufSize > (int)dwMaxBufferSize )
			dwMaxBufferSize = dwBufSize ;
		(*m_apiGetCharWidth)
			( m_hDC, pwszText[i], pwszText[i], (int*)&sizeHyphen.w ) ;
		sizeHyphen.h = gm.gmBlackBoxY ;
	}
	//
	// Allocate memory to get glyph outline
	void *	ptrGlyphBuf = ::eslHeapAllocate( NULL, dwMaxBufferSize, 0 ) ;
	//
	//	Drawing loop
	//
	i = 0;
	while( i < nTextLen )
	{
		//
		// Check whether the line overflow, or not.
		//
		if ( !IsVerticalWriting() )
		{
//			if( (int)(m_posCursor.y + m_nLineHeight) > (m_rectView.bottom + 1) )
			if( m_posCursor.y > m_rectView.bottom )
				break ;
		}
		else
		{
			if( (int) (m_posCursor.x + m_nLineHeight) < 0 )
				break ;
		}
		//
		//	Get word wrap position
		//
		int	nCursorPos, nWordWrapPos = i ;
		if ( !IsVerticalWriting() )
			nCursorPos = GetCursorPos().x ;
		else
			nCursorPos = GetCursorPos().y ;
		bool	fForceReturn = false ;
		while ( nWordWrapPos < nTextLen )
		{
			if ( pwszText[nWordWrapPos] < L' ' )
			{
				if ( pwszText[nWordWrapPos] == L'\n' )
				{
					nWordWrapPos ++ ;
					fForceReturn = true ;
					break ;
				}
			}
			else
			{
				nCursorPos += pCharSizes[nWordWrapPos].w ;
				if ( !IsVerticalWriting() )
				{
					if ( nCursorPos > (m_rectView.right + 1) )
					{
						nCursorPos -= pCharSizes[nWordWrapPos].w ;
						break ;
					}
				}
				else
				{
					if ( nCursorPos > (m_rectView.bottom + 1) )
					{
						nCursorPos -= pCharSizes[nWordWrapPos].w ;
						break ;
					}
				}
			}
			nWordWrapPos ++ ;
		}
		//
		// Check Prohibit characters
		bool	bHyphening = false ;
		if ( !fForceReturn && (nWordWrapPos < nTextLen) )
		{
			//
			//	Check Prohibit characters
			//
			int				nOrgWordWrapPos = nWordWrapPos ;
			int				nOrgCursorPos = nCursorPos ;
			unsigned int	nProhibitChars = 0 ;
			while ( nWordWrapPos > i )
			{
				if ( !IsProhibitChar( pwszText[nWordWrapPos] ) )
					break ;
				if ( pwszText[nWordWrapPos] < L' ' )
					break ;
				if ( ++ nProhibitChars > m_maxWordWrapping )
				{
					nWordWrapPos = nOrgWordWrapPos ;
					nCursorPos = nOrgCursorPos ;
					break ;
				}
				nCursorPos -= pCharSizes[-- nWordWrapPos].w ;
			}
/*			if ( (nWordWrapPos > i)
				&& IsProhibitChar( pwszText[nWordWrapPos] )
				&& (pwszText[nWordWrapPos] >= L' ') )
			{
				nCursorPos -= pCharSizes[-- nWordWrapPos].w ;
			}*/
			if ( nWordWrapPos <= i )
			{
				nWordWrapPos = nOrgWordWrapPos ;
				nCursorPos = nOrgCursorPos ;
			}
			//
			//	Perform hyphning
			//
			// Get right alphabet count
			int	j ;
			int	nRightAlphabet = 0 ;
			for ( j = nWordWrapPos; j < nTextLen; j ++ )
			{
				if ( !IsAlphabetChar( pwszText[j] ) )
					break ;
				nRightAlphabet ++ ;
			}
			// Get left alphabet count
			int	nLeftAlphabet = 0 ;
			for ( j = nWordWrapPos - 1; j >= i; j -- )
			{
				if ( !IsAlphabetChar( pwszText[j] ) )
					break ;
				nLeftAlphabet ++ ;
			}
			//
			if ( (nLeftAlphabet <= 2) ||
				((nRightAlphabet + nLeftAlphabet) < (int)m_minHyphening) )
			{
				// without hyphening
				nWordWrapPos -= nLeftAlphabet ;
			}
			else
			{
				// Get hyphening alphabet position
				j = 0 ;
				nCursorPos += sizeHyphen.w ;
				while ( j < nLeftAlphabet )
				{
					nCursorPos -= pCharSizes[nWordWrapPos - (++ j)].w ;
					if ( !IsVerticalWriting() )
					{
						if ( nCursorPos <= m_rectView.right )
							break ;
					}
					else
					{
						if ( nCursorPos <= m_rectView.bottom )
							break ;
					}
				}
				if ( j < nLeftAlphabet )
				{
					// with hyphening
					bHyphening = true ;
					nWordWrapPos -= j ;
				}
				else
				{
					// without hyphening
					nWordWrapPos -= nLeftAlphabet ;
				}
			}
		}
		//
		//	Excepted text format
		//
		if ( !fForceReturn && (nWordWrapPos == i) )
		{
			if ( nNullLineCount ++ )
				nWordWrapPos ++ ;
		}
		else
		{
			nNullLineCount = 0 ;
		}
		//
		//	Line text drawing
		//
		EWideString	linewstr( (pwszText + i), (nWordWrapPos - i) ) ;
		if ( bHyphening )
			linewstr += L'-' ;
		for ( i = 0; i < (int)linewstr.GetLength(); i ++ )
		{
			//
			// Get glyph outline
			//
			GLYPHMETRICS				gm ;
			SakuraGL::SGLFontMetrics	metrics ;
			wchar_t	wchNext = linewstr.GetAt( i ) ;
			EImageSprite *	pis = NULL ;
			memset( &metrics, 0, sizeof(SakuraGL::SGLFontMetrics) ) ;
			//
			DWORD	dwBufBytes ;
			if( (wchNext <= L' ')
				|| (!m_fCanUseUnicode && (wchNext == 0x8140))
				|| (m_fCanUseUnicode && (wchNext == L'　')) )
			{
				if ( m_pFont != NULL )
				{
					m_pFont->GetMetrics( NULL, 0, metrics, wchNext ) ;
				}
			}
			else if ( m_pFont != NULL )
			{
				if ( !m_pFont->GetMetrics
					( (uint8_t*) ptrGlyphBuf,
						dwMaxBufferSize, metrics, wchNext ) )
				{
					//
					// Convert to DIB from glyph outline
					//
					BITMAPINFO	bminf ;
					bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
					bminf.bmiHeader.biWidth = metrics.rctExterior.w ;
					bminf.bmiHeader.biHeight = metrics.rctExterior.h ;
					bminf.bmiHeader.biPlanes = 1 ;
					bminf.bmiHeader.biBitCount = 8 ;
					bminf.bmiHeader.biCompression = 0 ;
					bminf.bmiHeader.biSizeImage =
							metrics.rctExterior.w * metrics.rctExterior.h ;
					bminf.bmiHeader.biXPelsPerMeter = 0 ;
					bminf.bmiHeader.biYPelsPerMeter = 0 ;
					bminf.bmiHeader.biClrUsed = 0 ;
					bminf.bmiHeader.biClrImportant = 0 ;
					//
					// Convert to EImageSpriteObject
					//
					pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
				}
			}
			else if ( IsFontSmoothing() &&
				(dwBufBytes = (*m_apiGetGlyphOutLine)
					( m_hDC, wchNext, GGO_GRAY8_BITMAP, &gm,
						dwMaxBufferSize, ptrGlyphBuf, &mat2 )) != GDI_ERROR )
			{
				//
				// Convert to DIB from glyph outline
				//
				BITMAPINFO	bminf ;
				bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
				bminf.bmiHeader.biWidth = gm.gmBlackBoxX ;
				bminf.bmiHeader.biHeight = gm.gmBlackBoxY ;
				bminf.bmiHeader.biPlanes = 1 ;
				bminf.bmiHeader.biBitCount = 8 ;
				bminf.bmiHeader.biCompression = 0 ;
				bminf.bmiHeader.biSizeImage = 0 ;
				bminf.bmiHeader.biXPelsPerMeter = 0 ;
				bminf.bmiHeader.biYPelsPerMeter = 0 ;
				bminf.bmiHeader.biClrUsed = 0 ;
				bminf.bmiHeader.biClrImportant = 0 ;
				//
				DWORD	dwLineBytes =
							((gm.gmBlackBoxX * 8 + 0x1F) & ~0x1F) >> 3 ;
				if ( dwLineBytes * (gm.gmBlackBoxY + 1) == dwBufBytes )
				{
					bminf.bmiHeader.biHeight = gm.gmBlackBoxY + 1 ;
				}
				//
				// Convert to EImageSpriteObject
				//
				pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
				::ScalingOfGlyphOutline( *pis ) ;
			}
			else if ( (dwBufBytes = (*m_apiGetGlyphOutLine)
				( m_hDC, wchNext, GGO_BITMAP,
					&gm, dwMaxBufferSize, ptrGlyphBuf, &mat2 )) != GDI_ERROR )
			{
				//
				// Convert to DIB from glyph outline
				//
				BITMAPINFO	bminf ;
				bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
				bminf.bmiHeader.biWidth = gm.gmBlackBoxX ;
				bminf.bmiHeader.biHeight = gm.gmBlackBoxY ;
				bminf.bmiHeader.biPlanes = 1 ;
				bminf.bmiHeader.biBitCount = 1 ;
				bminf.bmiHeader.biCompression = 0 ;
				bminf.bmiHeader.biSizeImage = 0 ;
				bminf.bmiHeader.biXPelsPerMeter = 0 ;
				bminf.bmiHeader.biYPelsPerMeter = 0 ;
				bminf.bmiHeader.biClrUsed = 0 ;
				bminf.bmiHeader.biClrImportant = 0 ;
				if ( (((gm.gmBlackBoxX + 0x1F) & ~0x1F) >> 3)
							* (gm.gmBlackBoxY + 1) == dwBufBytes )
				{
					bminf.bmiHeader.biHeight = gm.gmBlackBoxY + 1 ;
				}
				//
				// Convert to EImageSpriteObject
				//
				pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
			}
			if ( pis != NULL )
			{
				//
				// Set parameters
				//
				EGL_POINT	posNextOut = m_posCursor ;
				if ( m_pFont != NULL )
				{
					if ( !IsVerticalWriting() )
					{
						posNextOut.x += metrics.rctExterior.x ;
						posNextOut.y +=
							(m_nLineHeight + metrics.rctExterior.y
												- metrics.nAscent - 1) ;
					}
					else
					{
						posNextOut.y += metrics.rctExterior.x ;
//						posNextOut.x += metrics.rctExterior.y ;
						posNextOut.x +=
							(m_nLineHeight - metrics.rctExterior.y
									- pis->GetInfo()->dwImageHeight) ;
						//
						pis = RevolveCharacter( pis ) ;
					}
				}
				else
				{
					ABC	abc ;
					(*m_apiGetCharABCWidths)( m_hDC,
						linewstr.GetAt(i), linewstr.GetAt(i), &abc ) ;
					if ( !IsVerticalWriting() )
					{
						posNextOut.x += gm.gmptGlyphOrigin.x ;
						posNextOut.y +=
							(m_nLineHeight - gm.gmptGlyphOrigin.y - 1) ;
					}
					else
					{
						posNextOut.y += gm.gmptGlyphOrigin.x ;
						posNextOut.x +=
							gm.gmptGlyphOrigin.y - pis->GetInfo()->dwImageHeight ;
						//
						pis = RevolveCharacter( pis ) ;
					}
				}
				EImageSprite::PARAMETER	prm ;
				prm.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
				prm.ptDstPos = posNextOut ;
				prm.ptRevCenter.x = 0 ;
				prm.ptRevCenter.y = 0 ;
				prm.rHorzUnit = 1.0F ;
				prm.rVertUnit = 1.0F ;
				prm.rRevAngle = 0.0F ;
				prm.rCrossingAngle = 90.0F ;
				prm.rgbDimColor = m_colorText ;
				prm.rgbLightColor = m_colorText ;
				prm.nTransparency = m_nTransparency ;
				pis->SetParameter( prm ) ;
				//
				// Add character's list
				//
				OnDrewCharacter( pis ) ;
			}
			//
			// Progress cursor position
			//
			if ( wchNext >= L' ' )
			{
				int	nCharWidth = m_nFontPitch ;
				if ( m_nFontPitch == 0 )
				{
					if ( m_pFont != NULL )
					{
						nCharWidth = metrics.nWidth ;
					}
					else
					{
						(*m_apiGetCharWidth)
							( m_hDC, linewstr.GetAt(i), linewstr.GetAt(i), &nCharWidth ) ;
					}
				}
				if ( !IsVerticalWriting() )
				{
					m_posCursor.x += nCharWidth ;
				}
				else
				{
					m_posCursor.y += nCharWidth ;
				}
			}
		}
		//
		// Next line
		//
		if( fForceReturn || (nWordWrapPos < nTextLen) )
		{
			MoveToNextLine( m_nIndentWidth ) ;
		}
		i = nWordWrapPos ;
	}
	//
	// Finish the function
	//
	::eslHeapFree( NULL, ptrGlyphBuf ) ;
	delete []	pCharSizes ;
	if ( m_pFont == NULL )
	{
		::SelectObject( m_hDC, (HGDIOBJ)hOldFont ) ;
	}

	return	i ;
}

// Draw text with fitting (single-line text)
//////////////////////////////////////////////////////////////////////////////
ESLError ERealFontImage::FitTextToWidth
	( const wchar_t * pwszText,
		int nPosX, int nPosY, unsigned int nWidth )
{
	if ( pwszText == NULL )
	{
		return	eslErrSuccess ;
	}
	//
	// If it can't use Unicode API, convert to Shift-JIS code
	//
	EString		strJiscodeBuf;
	EWideString	wstrJiscodeBuf;
	if ( !m_fCanUseUnicode && (m_pFont == NULL) )
	{
		//
		// Convert to Shift-JIS code
		//
		strJiscodeBuf = pwszText ;
		const char *	pszSrcText = strJiscodeBuf ;
		wchar_t *	pwszBuffer =
			wstrJiscodeBuf.GetBuffer( strJiscodeBuf.GetLength() + 1 ) ;
		//
		unsigned char	chrNext ;
		for ( ; ; )
		{
			chrNext = *(pszSrcText ++) ;
			if( chrNext == '\0' )	break ;
			if( ::IsDBCSLeadByte( chrNext ) )
			{
				// double byte character code
				*(pwszBuffer ++) =
					(((wchar_t)chrNext) << 8)
						| *((unsigned char*)(pszSrcText ++)) ;
			}
			else
			{
				// single byte character code
				*(pwszBuffer ++) = chrNext ;
			}
		}
		*(pwszBuffer ++) = L'\0' ;
		//
		wstrJiscodeBuf.ReleaseBuffer( ) ;
		pwszText = wstrJiscodeBuf ;
	}
	//
	//	Preparing the function
	//
	// Create Device-Context
	HFONT	hOldFont = NULL ;
	if ( m_pFont == NULL )
	{
		if ( m_hDC == NULL )
		{
			m_hDC = ::CreateCompatibleDC( NULL ) ;
		}
		hOldFont =
			(HFONT)::SelectObject( m_hDC, (HGDIOBJ)m_fontText ) ;
	}
	//
	// Get font metrix
	unsigned int	nFontDescent = 0 ;
	if ( m_pFont != NULL )
	{
		SakuraGL::SGLFontMetrics	metrics ;
		m_pFont->GetMetrics( NULL, 0, metrics, pwszText[0] ) ;
		nFontDescent = metrics.nDescent ;
	}
	else
	{
		TEXTMETRIC	TextMat ;
		if( ::GetTextMetrics( m_hDC, &TextMat ) )
		{
			nFontDescent = TextMat.tmDescent ;
		}
	}
	//
	// Get string length
	int		i, nTextLen = 0 ;
	while( pwszText[nTextLen] != L'\0' )	nTextLen ++ ;
	//
	// Get size of all character
	int		nAllTextSize = 0 ;
	DWORD	dwMaxBufferSize = 0 ;
	MAT2	mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
	if ( m_pFont != NULL )
	{
		for( i = 0; i < nTextLen; i ++ )
		{
			SakuraGL::SGLFontMetrics	metrics ;
			if ( !m_pFont->GetMetrics( NULL, 0, metrics, pwszText[i] ) )
			{
				DWORD	dwBufSize =
					metrics.rctExterior.w * metrics.rctExterior.h ;
				if( dwBufSize > dwMaxBufferSize )
				{
					dwMaxBufferSize = dwBufSize ;
				}
				nAllTextSize += metrics.nWidth ;
			}
		}
	}
	else
	{
		for( i = 0; i < nTextLen; i ++ )
		{
			GLYPHMETRICS	gm ;
			DWORD	dwBufSize = (*m_apiGetGlyphOutLine)
				( m_hDC, pwszText[i], GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat2 ) ;
			if ( (signed int)dwBufSize < 0 )
			{
				dwBufSize = (*m_apiGetGlyphOutLine)
					( m_hDC, pwszText[i], GGO_BITMAP, &gm, 0, NULL, &mat2 ) ;
			}
			if( (int)dwBufSize > (int)dwMaxBufferSize )
				dwMaxBufferSize = dwBufSize ;
			int		nCharWidth ;
			(*m_apiGetCharWidth)( m_hDC, pwszText[i], pwszText[i], &nCharWidth ) ;
			nAllTextSize += nCharWidth ;
		}
	}
	//
	// Calculate kerning width
	int	nKerningWidth = 0 ;
	if ( nTextLen > 0 )
	{
		nKerningWidth = (int)(nWidth - nAllTextSize) / nTextLen ;
	}
	nPosX += (nKerningWidth >> 1) ;
	//
	// Allocate memory to get glyph outline
	void *	ptrGlyphBuf = ::eslHeapAllocate( NULL, dwMaxBufferSize, 0 ) ;
	//
	//	Drawing loop
	//
	for ( i = 0; i < nTextLen; i ++ )
	{
		//
		// Get glyph outline
		//
		EImageSprite *	pis = NULL ;
		DWORD			dwBufBytes ;
		GLYPHMETRICS	gm ;
		SakuraGL::SGLFontMetrics	metrics ;
		memset( &metrics, 0, sizeof(SakuraGL::SGLFontMetrics) ) ;
		//
		if( (!m_fCanUseUnicode && (pwszText[i] != 0x8140))
			|| (m_fCanUseUnicode && (pwszText[i] != L'　')) )
		{
		if ( m_pFont != NULL )
		{
			if ( !m_pFont->GetMetrics
				( (uint8_t*) ptrGlyphBuf,
					dwMaxBufferSize, metrics, pwszText[i] ) )
			{
				//
				// Convert to DIB from glyph outline
				//
				BITMAPINFO	bminf ;
				bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
				bminf.bmiHeader.biWidth = metrics.rctExterior.w ;
				bminf.bmiHeader.biHeight = metrics.rctExterior.h ;
				bminf.bmiHeader.biPlanes = 1 ;
				bminf.bmiHeader.biBitCount = 8 ;
				bminf.bmiHeader.biCompression = 0 ;
				bminf.bmiHeader.biSizeImage =
						metrics.rctExterior.w * metrics.rctExterior.h ;
				bminf.bmiHeader.biXPelsPerMeter = 0 ;
				bminf.bmiHeader.biYPelsPerMeter = 0 ;
				bminf.bmiHeader.biClrUsed = 0 ;
				bminf.bmiHeader.biClrImportant = 0 ;
				//
				// Convert to EImageSpriteObject
				//
				pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
			}
		}
		if ( IsFontSmoothing() &&
			((dwBufBytes = (*m_apiGetGlyphOutLine)
				( m_hDC, pwszText[i], GGO_GRAY8_BITMAP,
					&gm, dwMaxBufferSize, ptrGlyphBuf, &mat2 )) != GDI_ERROR) )
		{
			//
			// Convert to DIB from glyph outline
			//
			BITMAPINFO	bminf ;
			bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
			bminf.bmiHeader.biWidth = gm.gmBlackBoxX ;
			bminf.bmiHeader.biHeight = gm.gmBlackBoxY ;
			bminf.bmiHeader.biPlanes = 1 ;
			bminf.bmiHeader.biBitCount = 8 ;
			bminf.bmiHeader.biCompression = 0 ;
			bminf.bmiHeader.biSizeImage = 0 ;
			bminf.bmiHeader.biXPelsPerMeter = 0 ;
			bminf.bmiHeader.biYPelsPerMeter = 0 ;
			bminf.bmiHeader.biClrUsed = 0 ;
			bminf.bmiHeader.biClrImportant = 0 ;
			if ( ((gm.gmBlackBoxX + 0x03) & ~0x03)
						* (gm.gmBlackBoxY + 1) == dwBufBytes )
			{
				bminf.bmiHeader.biHeight = gm.gmBlackBoxY + 1 ;
			}
			//
			// Convert to EImageSpriteObject
			//
			pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
			::ScalingOfGlyphOutline( *pis ) ;
		}
		else if ( (*m_apiGetGlyphOutLine)
			( m_hDC, pwszText[i], GGO_BITMAP,
				&gm, dwMaxBufferSize, ptrGlyphBuf, &mat2 ) != GDI_ERROR )
		{
			//
			// Convert to DIB from glyph outline
			//
			BITMAPINFO	bminf ;
			bminf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
			bminf.bmiHeader.biWidth = gm.gmBlackBoxX ;
			bminf.bmiHeader.biHeight = gm.gmBlackBoxY ;
			bminf.bmiHeader.biPlanes = 1 ;
			bminf.bmiHeader.biBitCount = 1 ;
			bminf.bmiHeader.biCompression = 0 ;
			bminf.bmiHeader.biSizeImage = 0 ;
			bminf.bmiHeader.biXPelsPerMeter = 0 ;
			bminf.bmiHeader.biYPelsPerMeter = 0 ;
			bminf.bmiHeader.biClrUsed = 0 ;
			bminf.bmiHeader.biClrImportant = 0 ;
			//
			// Convert to EImageSpriteObject
			//
			pis = CreateFromBitmap( &bminf, ptrGlyphBuf ) ;
		}
		if ( pis != NULL )
		{
			//
			// Set parameters
			//
			EGL_POINT	posNextOut = { nPosX, nPosY } ;
			if ( m_pFont != NULL )
			{
				posNextOut.x += metrics.rctExterior.x ;
				posNextOut.y +=
						(m_nLineHeight + metrics.rctExterior.y
											- metrics.nAscent - 1) ;
			}
			else
			{
				ABC	abc ;
				(*m_apiGetCharABCWidths)( m_hDC, pwszText[i], pwszText[i], &abc ) ;
				posNextOut.x += abc.abcA ;
				posNextOut.y +=
						(m_nLineHeight - gm.gmptGlyphOrigin.y - 1) ;
			}
			//
			EImageSprite::PARAMETER	prm ;
			prm.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
			prm.ptDstPos = posNextOut ;
			prm.ptRevCenter.x = 0 ;
			prm.ptRevCenter.y = 0 ;
			prm.rHorzUnit = 1.0F ;
			prm.rVertUnit = 1.0F ;
			prm.rRevAngle = 0.0F ;
			prm.rCrossingAngle = 90.0F ;
			prm.rgbDimColor = m_colorText ;
			prm.rgbLightColor = m_colorText ;
			prm.nTransparency = m_nTransparency ;
			pis->SetParameter( prm ) ;
			//
			// Add character's list
			//
			OnDrewCharacter( pis ) ;
		}
		}
		else
		{
			if ( m_pFont != NULL )
			{
				m_pFont->GetMetrics( NULL, 0, metrics, pwszText[i] ) ;
			}
		}
		//
		// Progress cursor position
		//
		int	nCharWidth ;
		if ( m_pFont != NULL )
		{
			nCharWidth = metrics.nWidth ;
		}
		else
		{
			(*m_apiGetCharWidth)
				( m_hDC, pwszText[i], pwszText[i], &nCharWidth ) ;
		}
		nPosX += (nCharWidth + nKerningWidth) ;
	}
	//
	// Finish the function
	//
	::eslHeapFree( NULL, ptrGlyphBuf ) ;
	if ( m_pFont == NULL )
	{
		::SelectObject( m_hDC, (HGDIOBJ)hOldFont ) ;
	}

	return	eslErrSuccess ;
}

// Drawing text callback function
//////////////////////////////////////////////////////////////////////////////
EImageSprite * ERealFontImage::CreateNewCharacter( void )
{
	return	new EImageSprite ;
}

//////////////////////////////////////////////////////////////////////////////
void ERealFontImage::OnDrewCharacter( EImageSprite * pCharObj )
{
	m_listCharacters->Add( pCharObj ) ;
}

// Get character's list length
//////////////////////////////////////////////////////////////////////////////
unsigned int ERealFontImage::GetCharacterCount( void ) const
{
	return	m_listCharacters->GetSize( ) ;
}

// Get character at the index
//////////////////////////////////////////////////////////////////////////////
EImageSprite * ERealFontImage::GetCharacterAt( unsigned int nIndex )
{
	return	m_listCharacters->GetAt( nIndex ) ;
}

// Remove all character's list
//////////////////////////////////////////////////////////////////////////////
void ERealFontImage::RemoveAllCharacter( void )
{
	m_listCharacters->RemoveAll( ) ;
}

// Drawing characters
//////////////////////////////////////////////////////////////////////////////
ESLError ERealFontImage::DrawCharacter
	( HEGL_RENDER_POLYGON hRender, int iFirst, int iEnd )
{
	if ( iEnd < 0 )
	{
		iEnd = GetCharacterCount( ) ;
	}
	for ( int i = iFirst; i < iEnd; i ++ )
	{
		EImageSprite *	pis = GetCharacterAt( i ) ;
		if ( pis != NULL )
		{
			pis->Draw( hRender ) ;
		}
	}
	return	eslErrSuccess ;
}

// Cursor position
//////////////////////////////////////////////////////////////////////////////
void ERealFontImage::MoveToNextLine( unsigned int nIndent )
{
	if ( !IsVerticalWriting() )
	{
		m_posCursor.y += m_nLineHeight ;
		m_posCursor.x = m_rectView.left + nIndent ;
	}
	else
	{
		m_posCursor.x -= m_nLineHeight ;
		m_posCursor.y = m_rectView.top + nIndent ;
	}
}

// Set prohibit characters
//////////////////////////////////////////////////////////////////////////////
void ERealFontImage::SetProhibitChar( const wchar_t * pwszProhibit )
{
	if ( m_fCanUseUnicode )
	{
		m_wstrProhibit = pwszProhibit ;
	}
	else
	{
		int		i = 0 ;
		m_wstrProhibit = L"" ;
		while ( pwszProhibit[i] != L'\0' )
		{
			EString	strCharCode( pwszProhibit + i, 1 ) ;
			if ( strCharCode.GetLength() == 2 )
			{
				m_wstrProhibit +=
					(wchar_t) (((wchar_t)strCharCode.GetAt(0) << 8)
								| ((wchar_t)strCharCode.GetAt(1) & 0xFF)) ;
			}
			else
			{
				m_wstrProhibit += (wchar_t) (strCharCode.GetAt(0)) ;
			}
			++ i ;
		}
	}
}

// Prohibit characters
//////////////////////////////////////////////////////////////////////////////
bool ERealFontImage::IsProhibitChar( wchar_t wchar ) const
{
	unsigned int	nProhibitLen = m_wstrProhibit.GetLength() ;
	for( unsigned int i = 0; i < nProhibitLen; i ++ )
	{
		if( m_wstrProhibit.GetAt(i) == wchar )
			return	true ;
	}
	return	false ;
}

