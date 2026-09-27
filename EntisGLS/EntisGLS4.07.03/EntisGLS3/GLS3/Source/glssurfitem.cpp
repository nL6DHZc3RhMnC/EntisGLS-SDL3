
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2017 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// アニメーション画像スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EAnimationSprite, ESpriteInterface ) ;

DWORD	EAnimationSprite::m_dwAnimationFlags = EAnimationSprite::animeFull ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EAnimationSprite::EAnimationSprite( void )
{
	m_nViewFrame = 0 ;
	m_nLoopCount = (DWORD) -1 ;
	m_nRewindSequence = 0 ;
	m_nTurnSequence = 0 ;
	m_nAnimationDuration = 0 ;
	m_nAnimationOffsetTime = 0 ;
	m_pAnimation = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EAnimationSprite::~EAnimationSprite( void )
{
}

// アニメーション進行
//////////////////////////////////////////////////////////////////////////////
ESLError EAnimationSprite::OnAdvanceAnimation( unsigned int nTime )
{
	if ( (m_pAnimation != NULL) && (m_nLoopCount != 0) )
	{
		if ( m_dwAnimationFlags & animeNormal )
		{
			m_nAnimationOffsetTime += nTime ;
			//
			DWORD				dwDeltaTime = m_nAnimationOffsetTime ;
			unsigned long int	nTotalTime = m_nAnimationDuration ;
			unsigned long int	nTurnSequence = m_nTurnSequence + 1 ;
			if ( nTurnSequence == 0 )
			{
				nTurnSequence = m_pAnimation->GetSequenceLength() ;
			}
			//
			if ( dwDeltaTime >= nTotalTime )
			{
				unsigned long int	nRewindTime = 0 ;
				if ( nTurnSequence > 0 )
				{
					nRewindTime =
						m_nRewindSequence * nTotalTime / nTurnSequence ;
				}
				if ( m_nLoopCount != (unsigned long int) -1 )
				{
					m_nLoopCount -- ;
				}
				if ( m_nLoopCount != 0 )
				{
					if ( nTotalTime > 0 )
					{
						dwDeltaTime -= nTotalTime ;
						if ( nTotalTime > nRewindTime )
						{
							dwDeltaTime %= (nTotalTime - nRewindTime) ;
						}
						dwDeltaTime += nRewindTime ;
					}
					else
					{
						dwDeltaTime = nRewindTime ;
					}
				}
				else
				{
					if ( nTotalTime > 0 )
					{
						dwDeltaTime = nTotalTime - 1 ;
					}
					else
					{
						dwDeltaTime = nRewindTime ;
					}
				}
			}
			m_nAnimationOffsetTime = dwDeltaTime ;
			//
			unsigned long int	nFrame = 0 ;
			if ( nTotalTime != 0 )
			{
				m_nCurrentSequence =
					dwDeltaTime * nTurnSequence / nTotalTime ;
				nFrame = m_pAnimation->SequenceToFrame( m_nCurrentSequence ) ;
			}
			if ( nFrame != m_nViewFrame )
			{
				m_nViewFrame = nFrame ;
				AttachImage( m_pAnimation->GetFrameAt( nFrame ) ) ;
			}
		}
		else if ( m_nLoopCount != (unsigned long int) -1 )
		{
			m_nLoopCount = 0 ;
		}
	}
	return	ESpriteInterface::OnAdvanceAnimation( nTime ) ;
}

// パラメータ複製
//////////////////////////////////////////////////////////////////////////////
void EAnimationSprite::CopyParameters( const EImageSprite * pSrc )
{
	ESpriteInterface::CopyParameters( pSrc ) ;
	//
	if ( m_pAnimation != NULL )
	{
		const EAnimationSprite *
			pAnimeSrc = ESLTypeCast<EAnimationSprite,EImageSprite>( pSrc ) ;
		if ( pAnimeSrc != NULL )
		{
			m_nViewFrame = pAnimeSrc->m_nViewFrame ;
			m_nCurrentSequence = pAnimeSrc->m_nCurrentSequence ;
			m_nLoopCount = pAnimeSrc->m_nLoopCount ;
			m_nRewindSequence = pAnimeSrc->m_nRewindSequence ;
			m_nTurnSequence = pAnimeSrc->m_nTurnSequence ;
			m_nAnimationDuration = pAnimeSrc->m_nAnimationDuration ;
			m_nAnimationOffsetTime = pAnimeSrc->m_nAnimationOffsetTime ;
		}
	}
}

// アニメーション画像スプライトを生成
//////////////////////////////////////////////////////////////////////////////
ESLError EAnimationSprite::CreateAnimation( EGLAnimation * pAnimation )
{
	DeleteImage( ) ;
	//
	m_pAnimation = pAnimation ;
	m_nCurrentSequence = 0 ;
	m_nViewFrame = pAnimation->SequenceToFrame( 0 ) ;
	m_nRewindSequence = 0 ;
	m_nTurnSequence = pAnimation->GetSequenceLength( ) ;
	if ( m_nTurnSequence == 0 )
	{
		m_nTurnSequence = pAnimation->GetTotalFrameCount( ) ;
		if ( m_nTurnSequence > 0 )
		{
			m_nTurnSequence -- ;
		}
	}
	else
	{
		m_nTurnSequence -- ;
	}
	m_nAnimationDuration = pAnimation->GetTotalTime( ) ;
	m_nAnimationOffsetTime = 0 ;
	AttachImage( pAnimation->GetFrameAt( m_nViewFrame ) ) ;
	//
	PARAMETER	param ;
	GetParameter( param ) ;
	param.ptRevCenter = pAnimation->GetHotSpot( ) ;
	if ( param.dwFlags & EGL_FIXED_POSITION )
	{
		param.ptRevCenter.x <<= 16 ;
		param.ptRevCenter.y <<= 16 ;
	}
	SetParameter( param ) ;
	//
	return	eslErrSuccess ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
void EAnimationSprite::DeleteImage( void )
{
	m_pAnimation = NULL ;
	ESpriteInterface::DeleteImage( ) ;
}

// アニメーション任意回数再生
//////////////////////////////////////////////////////////////////////////////
ESLError EAnimationSprite::BeginAnimation
	( unsigned long int nLoopCount,
		unsigned long int nBeginFrame, unsigned long int nAnimationTime,
		unsigned long int nRewindSequence, unsigned long int nTurnSequence )
{
	if ( m_pAnimation == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( nBeginFrame == (unsigned long int) -1 )
	{
		nBeginFrame = m_nCurrentSequence ;
	}
	else
	{
		m_nCurrentSequence = nBeginFrame ;
		m_nViewFrame = m_pAnimation->SequenceToFrame( nBeginFrame ) ;
		AttachImage( m_pAnimation->GetFrameAt( m_nViewFrame ) ) ;
	}
	m_nLoopCount = nLoopCount ;
	m_nRewindSequence = nRewindSequence ;
	m_nTurnSequence = nTurnSequence ;
	if ( nAnimationTime == (unsigned long int) -1 )
	{
		if ( m_nTurnSequence == (unsigned long int) -1 )
		{
			nAnimationTime = m_pAnimation->GetTotalTime( ) ;
		}
		else
		{
			nAnimationTime =
				m_pAnimation->SequenceToTime( m_nTurnSequence + 1 ) ;
		}
	}
	m_nAnimationDuration = nAnimationTime ;
	m_nAnimationOffsetTime = m_pAnimation->SequenceToTime( nBeginFrame ) ;
	return	eslErrSuccess ;
}

// アニメーション停止
//////////////////////////////////////////////////////////////////////////////
ESLError EAnimationSprite::EndAnimation( void )
{
	m_nLoopCount = 0 ;
	return	eslErrSuccess ;
}

// 機能フラグを設定する
//////////////////////////////////////////////////////////////////////////////
void EAnimationSprite::SetFunctionFlags( DWORD dwFlags )
{
	ESpriteInterface::SetFunctionFlags( dwFlags | ffTimer ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 静テキストスプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EStaticTextSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EStaticTextSprite::EStaticTextSprite( void )
{
	m_nScrollRange = 0 ;
	m_nScrollPos = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EStaticTextSprite::~EStaticTextSprite( void )
{
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EStaticTextSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	EDescription *	pdscStyle = dscParam.GetContentTagAs( 0, L"style" ) ;
	if ( pdscStyle != NULL )
	{
		EDescription *	pdscArrange =
			pdscStyle->GetContentTagAs( 0, L"arrange" ) ;
		if ( pdscArrange != NULL )
		{
			static const wchar_t *	pwszAligns[] =
			{
				L"left", L"top", L"right", L"center", L"accordance", NULL
			} ;
			static const TextAlign	taAligns[] =
			{
				taLeft, taTop, taRight, taCenter, taAccordance
			} ;
			EWideString	wstrAlign =
				pdscArrange->GetAttrString( L"align", NULL ) ;
			for ( int i = 0; pwszAligns[i]; i ++ )
			{
				if ( wstrAlign == pwszAligns[i] )
				{
					m_tsStyle.taAlign = taAligns[i] ;
				}
			}
			m_tsStyle.nLineHeight =
				pdscArrange->GetAttrInteger
					( L"line_height", m_tsStyle.nLineHeight ) ;
			m_tsStyle.nIndent =
				pdscArrange->GetAttrInteger
					( L"indent", m_tsStyle.nIndent ) ;
		}
		EDescription *	pdscFont =
			pdscStyle->GetContentTagAs( 0, L"font" ) ;
		if ( pdscFont != NULL )
		{
			EString		strFontFace ;
			EWideString	wstrFontStyle ;
			//
			m_tsStyle.lfFont.lfHeight =
				pdscFont->GetAttrInteger
					( L"size", m_tsStyle.lfFont.lfHeight ) ;
			//
			strFontFace = m_tsStyle.lfFont.lfFaceName ;
			strFontFace =
				GetValidFontFace( EString(
					pdscFont->GetAttrString
						( L"face", EWideString( strFontFace ) ) ) ) ;
			::eslMoveMemory
				( m_tsStyle.lfFont.lfFaceName, strFontFace.CharPtr(),
					__min( strFontFace.GetLength() + 1, LF_FACESIZE ) ) ;
			//
			wstrFontStyle = pdscFont->GetAttrString( L"bold", NULL ) ;
			if ( wstrFontStyle == L"true" )
				m_tsStyle.lfFont.lfWeight = FW_BOLD ;
			else if ( wstrFontStyle == L"false" )
				m_tsStyle.lfFont.lfWeight = FW_NORMAL ;
			//
			wstrFontStyle = pdscFont->GetAttrString( L"italic", NULL ) ;
			if ( wstrFontStyle == L"true" )
				m_tsStyle.lfFont.lfItalic = TRUE ;
			else if ( wstrFontStyle == L"false" )
				m_tsStyle.lfFont.lfItalic = FALSE ;
		}
		EDescription *	pdscText =
			pdscStyle->GetContentTagAs( 0, L"text" ) ;
		if ( pdscText != NULL )
		{
			m_tsStyle.rgbColor.dwPixelCode =
				pdscText->GetAttrInteger
					( L"color", m_tsStyle.rgbColor.dwPixelCode ) ;
			m_tsStyle.nTransparency =
				pdscText->GetAttrInteger
					( L"transparency", m_tsStyle.nTransparency ) ;
		}
		EDescription *	pdscShadow =
			pdscStyle->GetContentTagAs( 0, L"shadow" ) ;
		if ( pdscShadow != NULL )
		{
			m_tsStyle.ptShadowOffset.x =
				pdscShadow->GetAttrInteger
					( L"x", m_tsStyle.ptShadowOffset.x ) ;
			m_tsStyle.ptShadowOffset.y =
				pdscShadow->GetAttrInteger
					( L"y", m_tsStyle.ptShadowOffset.y ) ;
			m_tsStyle.rgbShadow.dwPixelCode =
				pdscShadow->GetAttrInteger
					( L"color", m_tsStyle.rgbShadow.dwPixelCode ) ;
			m_tsStyle.nShadowTrans =
				pdscShadow->GetAttrInteger
					( L"transparency", m_tsStyle.nShadowTrans ) ;
		}
		EWideString	wstrText = m_wstrText ;
		SetText( wstrText ) ;
	}
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// 静テキストオブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticTextSprite::CreateText
	( const TEXT_STYLE & style, int nViewWidth, int nViewHeight )
{
	//
	// 文字画像生成
	//
	ERealFontImage	rfiText ;
	EGL_SIZE		sizeMax ;
	if ( CreateFontImage( rfiText, style, &sizeMax ) )
	{
		return	eslErrGeneral ;
	}
	m_sizeView.w = nViewWidth ;
	m_sizeView.h = nViewHeight ;
	if ( sizeMax.w < nViewWidth )
		sizeMax.w = nViewWidth ;
	if ( sizeMax.h < nViewHeight )
		sizeMax.h = nViewHeight ;
	//
	m_wstrText = style.pwszText ;
	//
	// 画像バッファ生成
	//
	if ( m_imgText.CreateImage
		( EIF_RGBA_BITMAP, sizeMax.w, sizeMax.h, 32 ) == NULL )
	{
		return	ESLErrorMsg( "画像バッファの作成に失敗しました。" ) ;
	}
	//
	// 文字画像描画
	//
	ESLError	err ;
	HEGL_DRAW_IMAGE	hDrawImage ;
	hDrawImage = ::eglCreateDrawImage( ) ;
	hDrawImage->Initialize( m_imgText, NULL, NULL ) ;
	err = DrawFontImage( hDrawImage, rfiText, style ) ;
	hDrawImage->Release( ) ;
	//
	// 表示設定
	//
	m_nScrollRange = sizeMax.h - m_sizeView.h ;
	m_nScrollPos = -1 ;
	SetVertScrollPos( 0 ) ;
	//
	// スタイルを保存する
	//
	m_tsStyle = style ;
	//
	return	err ;
}

// 文字列を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticTextSprite::SetText( const wchar_t * pwszText )
{
	if ( m_imgText.GetInfo() == NULL )
	{
		return	ESLErrorMsg( "静テキストオブジェクトが作成されていません。" ) ;
	}
	ESLError	err ;
	TEXT_STYLE	style = m_tsStyle ;
	style.pwszText = pwszText ;
	err = CreateText( style, m_sizeView.w, m_sizeView.h ) ;
	return	err ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool EStaticTextSprite::IsHitSprite( int xPos, int yPos )
{
	return	false ;
}

// 文字列取得・設定
//////////////////////////////////////////////////////////////////////////////
const wchar_t * EStaticTextSprite::GetSpriteText( void )
{
	return	m_wstrText ;
}

void EStaticTextSprite::SetSpriteText( const wchar_t * pwszText )
{
	SetText( pwszText ) ;
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void EStaticTextSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	EString	strFont = GetValidFontFace( EString( pwszFont ) ) ;
	int		nFaceLen = strFont.GetLength() + 1 ;
	if ( nFaceLen > LF_FACESIZE )
	{
		nFaceLen = LF_FACESIZE ;
	}
	::eslMoveMemory( m_tsStyle.lfFont.lfFaceName, strFont.CharPtr(), nFaceLen ) ;
	//
	EWideString	wstrText = m_wstrText ;
	SetText( wstrText ) ;
}

// スクロールレンジ取得
//////////////////////////////////////////////////////////////////////////////
int EStaticTextSprite::GetVertScrollRange( void ) const
{
	return	m_nScrollRange ;
}

// 表示位置取得
//////////////////////////////////////////////////////////////////////////////
int EStaticTextSprite::GetVertScrollPos( void ) const
{
	return	m_nScrollPos ;
}

// 表示位置設定
//////////////////////////////////////////////////////////////////////////////
void EStaticTextSprite::SetVertScrollPos( int nPos )
{
	if ( nPos > m_nScrollRange )
	{
		nPos = m_nScrollRange ;
	}
	if ( m_nScrollPos != nPos )
	{
		m_nScrollPos = nPos ;
		//
		PEGL_IMAGE_INFO	pImage = m_imgText ;
		if ( pImage != NULL )
		{
			EGL_RECT	rectView ;
			rectView.left = 0 ;
			rectView.right = pImage->dwImageWidth - 1 ;
			rectView.top = nPos ;
			rectView.bottom = nPos + m_sizeView.h - 1 ;
			SetImageView( pImage, &rectView ) ;
		}
	}
}

// テキスト描画関数
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticTextSprite::CreateFontImage
	( ERealFontImage & rfiText,
			const TEXT_STYLE & style, EGL_SIZE * pMaxSize )
{
	//
	// 文字描画オブジェクトに基本的な属性を設定
	//
	EGL_RECT	rectExt = style.rectExt ;
	if ( style.taAlign == taLeft )
	{
		rectExt.bottom = 0x7FFF ;
	}
	SakuraGL::SGLFont *	pFont = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	fsStyle ;
	SSystem::SString		strFont ;
	fsStyle.FromLogFont( style.lfFont, strFont ) ;
	pFont->SetStyle( fsStyle ) ;
	rfiText.SetSGLFont( pFont ) ;
//	EFontObject	font( style.lfFont ) ;
//	rfiText.SetFont( font.Create() ) ;
	rfiText.SetViewRect( rectExt ) ;
	rfiText.SetLineHeight( style.nLineHeight ) ;
	rfiText.SetIndentWidth( style.nIndent ) ;
	rfiText.SetFontPitch( style.nFontPitch ) ;
	//
	// 文字列描画
	//
	if ( (style.taAlign == taLeft) || (style.taAlign == taTop) )
	{
		//
		// 複数行
		//
		EGL_POINT	ptCursor ;
		if ( style.taAlign == taLeft )
		{
			rfiText.SetVerticalWriting( false ) ;
			ptCursor.x = style.rectExt.left ;
			ptCursor.y = style.rectExt.top ;
		}
		else
		{
			rfiText.SetVerticalWriting( true ) ;
			ptCursor.x = style.rectExt.right ;
			ptCursor.y = style.rectExt.top ;
		}
		rfiText.MoveCursorPos( ptCursor ) ;
		//
		rfiText.DrawText( style.pwszText ) ;
	}
	else
	{
		//
		// 単一行
		//
		int		xOffset, nWidth ;
		nWidth = rfiText.GetTextWidth( style.pwszText ) ;
		if ( style.taAlign == taRight )
		{
			xOffset = style.rectExt.right - nWidth + 1 ;
		}
		else if ( style.taAlign == taCenter )
		{
			xOffset =
				style.rectExt.left
				+ ((style.rectExt.right
					- style.rectExt.left + 1) - nWidth) / 2 ;
		}
		else
		{
			xOffset = style.rectExt.left ;
			nWidth = style.rectExt.right - style.rectExt.left + 1 ;
		}
		if ( style.taAlign != taAccordance )
		{
			rfiText.MoveCursorPos
				( EGLPoint( xOffset, style.rectExt.top ) ) ;
			rfiText.DrawText( style.pwszText ) ;
		}
		else
		{
			rfiText.FitTextToWidth
				( style.pwszText, xOffset, style.rectExt.top, nWidth ) ;
		}
	}
	//
	// 最小外接矩形を取得
	//
	if ( pMaxSize != NULL )
	{
		unsigned int	i, nCount ;
		nCount = rfiText.GetCharacterCount( ) ;
		pMaxSize->w = 0 ;
		pMaxSize->h = 0 ;
		//
		for ( i = 0; i < nCount; i ++ )
		{
			EImageSprite *	pis = rfiText.GetCharacterAt( i ) ;
			if ( pis != NULL )
			{
				EGL_RECT	rect = pis->GetRectangle( ) ;
				if ( rect.right >= pMaxSize->w )
					pMaxSize->w = rect.right + 1 ;
				if ( rect.bottom >= pMaxSize->h )
					pMaxSize->h = rect.bottom + 1 ;
			}
		}
	}
	//
	return	eslErrSuccess ;
}

ESLError EStaticTextSprite::DrawFontImage
	( HEGL_DRAW_IMAGE hDrawImage,
			ERealFontImage & rfiText, const TEXT_STYLE & style )
{
	//
	// 文字の影を描画
	//
	unsigned int	i, nCount ;
	EGL_DRAW_PARAM	edp ;
	nCount = rfiText.GetCharacterCount( ) ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	edp.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
	edp.rgbDimColor = edp.rgbLightColor = style.rgbShadow ;
	edp.nTransparency = style.nShadowTrans ;
	if ( style.nShadowTrans < 0x100 )
	{
		for ( i = 0; i < nCount; i ++ )
		{
			EImageSprite *	pis = rfiText.GetCharacterAt( i ) ;
			if ( pis != NULL )
			{
				EGL_POINT	ptChar = pis->GetPosition( ) ;
				edp.pSrcImage = pis->GetInfo( ) ;
				edp.ptBasePos.x = ptChar.x + style.ptShadowOffset.x ;
				edp.ptBasePos.y = ptChar.y + style.ptShadowOffset.y ;
				if ( !hDrawImage->PrepareDraw( &edp ) )
				{
					hDrawImage->DrawImage( ) ;
				}
			}
		}
	}
	//
	// 文字の縁取りを描画
	//
	if ( style.nFlags & txfBordering )
	{
		edp.rgbDimColor = edp.rgbLightColor = style.rgbBorder ;
		edp.nTransparency = style.nBorderTrans ;
		for ( i = 0; i < nCount; i ++ )
		{
			EImageSprite *	pis = rfiText.GetCharacterAt( i ) ;
			if ( pis != NULL )
			{
				EGL_POINT	ptChar = pis->GetPosition( ) ;
				static const EGL_POINT	ptOffset[5] =
				{
					{ 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
				} ;
				edp.pSrcImage = pis->GetInfo( ) ;
				for ( int j = 0; j < 5; j ++ )
				{
					edp.ptBasePos.x = ptChar.x + ptOffset[j].x ;
					edp.ptBasePos.y = ptChar.y + ptOffset[j].y ;
					if ( !hDrawImage->PrepareDraw( &edp ) )
					{
						hDrawImage->DrawImage( ) ;
					}
				}
			}
		}
	}
	//
	// 文字を描画
	//
	edp.rgbDimColor = edp.rgbLightColor = style.rgbColor ;
	edp.nTransparency = style.nTransparency ;
	for ( i = 0; i < nCount; i ++ )
	{
		EImageSprite *	pis = rfiText.GetCharacterAt( i ) ;
		if ( pis != NULL )
		{
			edp.ptBasePos = pis->GetPosition( ) ;
			edp.pSrcImage = pis->GetInfo( ) ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
		}
	}
	//
	return	eslErrSuccess ;
}

// フォント情報取得
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticTextSprite::GetFontInformation( LOGFONT & lfFont, HFONT hFont )
{
	LPOUTLINETEXTMETRIC	lpoltm ;
	UINT	nBufSize, nResult ;
	HDC		hDC = ::CreateCompatibleDC( NULL ) ;
	HFONT	hOrgFont = (HFONT) ::SelectObject( hDC, (HGDIOBJ) hFont ) ;
	nBufSize = ::GetOutlineTextMetrics( hDC, 0, NULL ) ;
	lpoltm = (LPOUTLINETEXTMETRIC) ::eslHeapAllocate( NULL, nBufSize, 0 ) ;
	nResult = ::GetOutlineTextMetrics( hDC, nBufSize, lpoltm ) ;
	::SelectObject( hDC, (HGDIOBJ) hOrgFont ) ;
	::DeleteDC( hDC ) ;
	//
	::eslFillMemory( &lfFont, 0, sizeof(LOGFONT) ) ;
	if ( nResult )
	{
		lfFont.lfHeight = lpoltm->otmTextMetrics.tmHeight ;
		lfFont.lfWeight = lpoltm->otmTextMetrics.tmWeight ;
		lfFont.lfItalic = lpoltm->otmTextMetrics.tmItalic ;
		lfFont.lfUnderline = lpoltm->otmTextMetrics.tmUnderlined ;
		lfFont.lfStrikeOut = lpoltm->otmTextMetrics.tmStruckOut ;
		lfFont.lfCharSet = lpoltm->otmTextMetrics.tmCharSet ;
		lfFont.lfOutPrecision = OUT_DEFAULT_PRECIS ;
		lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS ;
		lfFont.lfQuality = DEFAULT_QUALITY ;
		lfFont.lfPitchAndFamily = lpoltm->otmTextMetrics.tmPitchAndFamily ;
		::lstrcpy( lfFont.lfFaceName,
			(const char *) (((BYTE*)lpoltm) + (int)lpoltm->otmpFaceName) ) ;
	}
	else
	{
		lfFont.lfHeight = 16 ;
	}
	::eslHeapFree( NULL, lpoltm ) ;
	//
	return	eslErrSuccess ;
}

// フォントを列挙する
//////////////////////////////////////////////////////////////////////////////
static int CALLBACK EStaticTextSprite_EnumFontFamProc
	( ENUMLOGFONTEX *lpelfe,
		NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )
{
	DWORD *	pdwFlags = (DWORD*) lParam ;
	*pdwFlags |= (FontType == TRUETYPE_FONTTYPE) ;
	return	1 ;
}

// 有効なフォントフェースを取得
//////////////////////////////////////////////////////////////////////////////
EString EStaticTextSprite::GetValidFontFace( const char * pszFontFaceList )
{
	EString	strFace = pszFontFaceList ;
	int		iLast = 0 ;
	int		iFind = strFace.Find( ';', 0 ) ;
	while ( iFind > 0 )
	{
		EString	strFaceName = strFace.Middle( iLast, iFind - iLast ) ;
		DWORD	dwFlags = 0 ;
		HDC	hdc = ::CreateCompatibleDC( NULL ) ;
		LOGFONT	lfEnum ;
		::eslFillMemory( &lfEnum, 0, sizeof(lfEnum) ) ;
		lfEnum.lfCharSet = DEFAULT_CHARSET ;
		::eslMoveMemory
			( lfEnum.lfFaceName, strFace.CharPtr(),
				__min( strFaceName.GetLength(), LF_FACESIZE ) ) ;
		::EnumFontFamiliesEx
			( hdc, &lfEnum,
				(FONTENUMPROCA) &EStaticTextSprite_EnumFontFamProc,
												(LPARAM) &dwFlags, 0 ) ;
		::DeleteDC( hdc ) ;
		if ( dwFlags )
		{
			strFace = strFaceName ;
			break ;
		}
		iLast = ++ iFind ;
		iFind = strFace.Find( ';', iLast ) ;
		if ( iFind < 0 )
		{
			strFace = strFace.Middle( iLast ) ;
			break ;
		}
	}
	return	strFace ;
}


//////////////////////////////////////////////////////////////////////////////
// 進捗状況バースプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EProgressBarSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EProgressBarSprite::EProgressBarSprite( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EProgressBarSprite::~EProgressBarSprite( void )
{
}

// 固有の処理
//////////////////////////////////////////////////////////////////////////////
long int EProgressBarSprite::SendCommand
	( const EDescription & dscParam, EWideString * pwstrResult )
{
	EDescription *	pdscBar = dscParam.GetContentTagAs( 0, L"bar" ) ;
	if ( pdscBar != NULL )
	{
		SetRange( pdscBar->GetAttrInteger( L"range", 0 ) ) ;
		SetPos( pdscBar->GetAttrInteger( L"pos", 0 ) ) ;
	}
	return	ESpriteInterface::SendCommand( dscParam, pwstrResult ) ;
}

// 進捗状況バー生成
//////////////////////////////////////////////////////////////////////////////
ESLError EProgressBarSprite::CreateProgressBar( const BAR_STYLE & style )
{
	//
	// サイズを計算する
	//
	int		nFrameLeft, nFrameRight, nWayCount ;
	EGL_SIZE	sizeExt, sizeBar ;
	//
	if ( (style.pFrameLeft == NULL) || (style.pFrameRight == NULL)
		|| (style.pFrameWay == NULL) || (style.pBarLeft == NULL)
		|| (style.pBarRight == NULL) || (style.pBarWay == NULL) )
	{
		return	eslErrGeneral ;
	}
	//
	m_pbtType = style.pbtType ;
	m_ptBar = style.ptBarOffset ;
	//
	if ( m_pbtType == pbtVert )
	{
		nFrameLeft = style.pFrameLeft->dwImageHeight ;
		nFrameRight = style.pFrameRight->dwImageHeight ;
		nWayCount =
			(int) (style.sizeExt.h - nFrameLeft - nFrameRight)
							/ (int) style.pFrameWay->dwImageHeight ;
		m_nBarLeft = style.pBarLeft->dwImageHeight ;
		m_nBarRight = style.pBarRight->dwImageHeight ;
		//
		if ( nWayCount <= 0 )
			nWayCount = 1 ;
		//
		sizeExt.w = style.sizeExt.w ;
		sizeExt.h = nFrameLeft + nFrameRight
					+ nWayCount * style.pFrameWay->dwImageHeight ;
	}
	else
	{
		nFrameLeft = style.pFrameLeft->dwImageWidth ;
		nFrameRight = style.pFrameRight->dwImageWidth ;
		nWayCount =
			(int) (style.sizeExt.w - nFrameLeft - nFrameRight)
							/ (int) style.pFrameWay->dwImageWidth ;
		m_nBarLeft = style.pBarLeft->dwImageWidth ;
		m_nBarRight = style.pBarRight->dwImageWidth ;
		//
		if ( nWayCount <= 0 )
			nWayCount = 1 ;
		//
		sizeExt.w = nFrameLeft + nFrameRight
					+ nWayCount * style.pFrameWay->dwImageWidth ;
		sizeExt.h = style.sizeExt.h ;
	}
	//
	// 画像バッファを確保する
	//
	if ( CreateImage
		( EIF_RGBA_BITMAP, sizeExt.w, sizeExt.h, 32 ) == NULL )
	{
		return	eslErrGeneral ;
	}
	EnableFillBack( false ) ;
	//
	if ( m_imgBack.CreateImage
		( EIF_RGBA_BITMAP, sizeExt.w, sizeExt.h, 32 ) == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( m_pbtType == pbtVert )
	{
		sizeBar.w = style.pBarLeft->dwImageWidth ;
		if ( sizeBar.w < (int) style.pBarRight->dwImageWidth )
			sizeBar.w = style.pBarRight->dwImageWidth ;
		if ( sizeBar.w < (int) style.pBarWay->dwImageWidth )
			sizeBar.w = style.pBarWay->dwImageWidth ;
		sizeBar.h = m_nBarLeft + m_nBarRight
					+ nWayCount * style.pBarWay->dwImageHeight ;
	}
	else
	{
		sizeBar.w = m_nBarLeft + m_nBarRight
					+ nWayCount * style.pBarWay->dwImageWidth ;
		sizeBar.h = style.pBarLeft->dwImageHeight ;
		if ( sizeBar.h < (int) style.pBarRight->dwImageHeight )
			sizeBar.h = style.pBarRight->dwImageHeight ;
		if ( sizeBar.h < (int) style.pBarWay->dwImageHeight )
			sizeBar.h = style.pBarWay->dwImageHeight ;
	}
	if ( m_imgBar.CreateImage
		( EIF_RGBA_BITMAP, sizeBar.w, sizeBar.h, 32 ) == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 画像を生成する
	//
	HEGL_DRAW_IMAGE	hDrawImage ;
	EGL_DRAW_PARAM	edp ;
	hDrawImage = ::eglCreateDrawImage( ) ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	//
	if ( m_pbtType == pbtVert )
	{
		//
		// フレーム描画（垂直）
		//
		int	i, y = 0 ;
		hDrawImage->Initialize( m_imgBack, NULL, NULL ) ;
		//
		edp.ptBasePos.y = y ;
		edp.pSrcImage = style.pFrameLeft ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		y += style.pFrameLeft->dwImageHeight ;
		for ( i = 0; i < nWayCount; i ++ )
		{
			edp.ptBasePos.y = y ;
			edp.pSrcImage = style.pFrameWay ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
			y += style.pFrameWay->dwImageHeight ;
		}
		edp.ptBasePos.y = y ;
		edp.pSrcImage = style.pFrameRight ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		//
		// バー描画（垂直）
		//
		hDrawImage->Initialize( m_imgBar, NULL, NULL ) ;
		//
		y = 0 ;
		edp.ptBasePos.y = y ;
		edp.pSrcImage = style.pBarLeft ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		y += style.pBarLeft->dwImageHeight ;
		for ( i = 0; i < nWayCount; i ++ )
		{
			edp.ptBasePos.y = y ;
			edp.pSrcImage = style.pBarWay ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
			y += style.pBarWay->dwImageHeight ;
		}
		edp.ptBasePos.y = y ;
		edp.pSrcImage = style.pBarRight ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
	}
	else
	{
		//
		// フレーム描画（水平）
		//
		int	i, x = 0 ;
		hDrawImage->Initialize( m_imgBack, NULL, NULL ) ;
		//
		edp.ptBasePos.x = x ;
		edp.pSrcImage = style.pFrameLeft ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		x += style.pFrameLeft->dwImageWidth ;
		for ( i = 0; i < nWayCount; i ++ )
		{
			edp.ptBasePos.x = x ;
			edp.pSrcImage = style.pFrameWay ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
			x += style.pFrameWay->dwImageWidth ;
		}
		edp.ptBasePos.x = x ;
		edp.pSrcImage = style.pFrameRight ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		//
		// バー描画（垂直）
		//
		hDrawImage->Initialize( m_imgBar, NULL, NULL ) ;
		//
		x = 0 ;
		edp.ptBasePos.x = x ;
		edp.pSrcImage = style.pBarLeft ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
		x += style.pBarLeft->dwImageWidth ;
		for ( i = 0; i < nWayCount; i ++ )
		{
			edp.ptBasePos.x = x ;
			edp.pSrcImage = style.pBarWay ;
			if ( !hDrawImage->PrepareDraw( &edp ) )
			{
				hDrawImage->DrawImage( ) ;
			}
			x += style.pBarWay->dwImageWidth ;
		}
		edp.ptBasePos.x = x ;
		edp.pSrcImage = style.pBarRight ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
	}
	//
	hDrawImage->Release( ) ;
	//
	// 表示更新
	//
	m_nBarRange = 100 ;
	m_nBarPos = 0 ;
	DrawProgressBar( ) ;
	//
	return	eslErrSuccess ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::DrawProgressBar( void )
{
	if ( (GetInfo() == NULL)
		|| (m_imgBack.GetInfo() == NULL)
		|| (m_imgBar.GetInfo() == NULL) )
	{
		return ;
	}
	//
	HEGL_DRAW_IMAGE	hDrawImage ;
	EGL_DRAW_PARAM	edp ;
	EGL_RECT		rcView ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	hDrawImage = ::eglCreateDrawImage( ) ;
	hDrawImage->Initialize( GetInfo(), NULL, NULL ) ;
	//
	edp.pSrcImage = m_imgBack ;
	if ( !hDrawImage->PrepareDraw( &edp ) )
	{
		hDrawImage->DrawImage( ) ;
	}
	//
	if ( (m_nBarPos > 0) && (m_nBarRange > 0) )
	{
		edp.dwFlags = EGL_DRAW_BLEND_ALPHA ;
		edp.ptBasePos = m_ptBar ;
		edp.pSrcImage = m_imgBar ;
		//
		if ( m_nBarPos < m_nBarRange )
		{
			if ( m_pbtType == pbtVert )
			{
				int		nHeight = m_imgBar.GetInfo()->dwImageHeight ;
				rcView.left = 0 ;
				rcView.bottom = nHeight - 1 ;
				rcView.right = m_imgBar.GetInfo()->dwImageWidth - 1 ;
				rcView.top = nHeight - m_nBarLeft
						- ((nHeight - m_nBarLeft - m_nBarRight)
									* m_nBarPos / m_nBarRange) ;
				edp.ptBasePos.y += rcView.top ;
			}
			else
			{
				int		nWidth = m_imgBar.GetInfo()->dwImageWidth ;
				rcView.left = 0 ;
				rcView.top = 0 ;
				rcView.right = m_nBarLeft
						+ ((nWidth - m_nBarLeft - m_nBarRight)
									* m_nBarPos / m_nBarRange) ;
				rcView.bottom = m_imgBar.GetInfo()->dwImageHeight - 1 ;
			}
			edp.pViewRect = &rcView ;
		}
		//
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
	}
	//
	hDrawImage->Release( ) ;
}

// 全体量を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetRange( int nRange )
{
	if ( nRange < 0 )
	{
		nRange = 0 ;
	}
	if ( m_nBarRange != nRange )
	{
		m_nBarRange = nRange ;
		if ( m_nBarPos > nRange )
		{
			m_nBarPos = nRange ;
		}
		DrawProgressBar( ) ;
		UpdateRect( NULL ) ;
	}
}

// 現在の進捗状況を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetPos( int nPos )
{
	if ( nPos < 0 )
	{
		nPos = 0 ;
	}
	else if ( nPos > m_nBarRange )
	{
		nPos = m_nBarRange ;
	}
	if ( m_nBarPos != nPos )
	{
		m_nBarPos = nPos ;
		DrawProgressBar( ) ;
		UpdateRect( NULL ) ;
	}
}

// 現在の垂直スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EProgressBarSprite::GetVertScrollPos( void ) const
{
	return	GetPos( ) ;
}

// 現在の垂直スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetVertScrollPos( int nPos )
{
	SetPos( nPos ) ;
}

// 垂直スクロールの範囲を取得
//////////////////////////////////////////////////////////////////////////////
int EProgressBarSprite::GetVertScrollRange( void ) const
{
	return	GetRange( ) ;
}

// 垂直スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetVertScrollRange( int nRange )
{
	SetRange( nRange ) ;
}

// 現在の水平スクロール位置を取得
//////////////////////////////////////////////////////////////////////////////
int EProgressBarSprite::GetHorzScrollPos( void ) const
{
	return	GetPos( ) ;
}

// 現在の水平スクロール位置を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetHorzScrollPos( int nPos )
{
	SetPos( nPos ) ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
int EProgressBarSprite::GetHorzScrollRange( void ) const
{
	return	GetRange( ) ;
}

// 水平スクロールの範囲を設定
//////////////////////////////////////////////////////////////////////////////
void EProgressBarSprite::SetHorzScrollRange( int nRange )
{
	SetRange( nRange ) ;
}


//////////////////////////////////////////////////////////////////////////////
// フレーム表示スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EStaticFrameSprite, ESpriteInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EStaticFrameSprite::EStaticFrameSprite( void )
{
	for ( int i = 0; i < ftMax; i ++ )
	{
		m_fsStyle.pFrame[i] = NULL ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EStaticFrameSprite::~EStaticFrameSprite( void )
{
}

// フレーム生成
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticFrameSprite::CreateStaticFrame
	( const FRAME_STYLE & style, int nWidth, int nHeight )
{
	m_fsStyle = style ;
	for ( int i = 0; i < ftMax; i ++ )
	{
		if ( m_fsStyle.pFrame[i] )
		{
			m_imgFrameParts[i].DuplicateImage( m_fsStyle.pFrame[i] ) ;
			m_fsStyle.pFrame[i] = m_imgFrameParts[i] ;
		}
	}
	return	ResizeFrame( nWidth, nHeight ) ;
}

// サイズ変更
//////////////////////////////////////////////////////////////////////////////
ESLError EStaticFrameSprite::ResizeFrame( int nWidth, int nHeight )
{
	//
	// 指定サイズ以下で最大のサイズを算出する
	//
	if ( m_fsStyle.pFrame[ftUpperLeft] != NULL )
	{
		m_sizeLeft.w = m_fsStyle.pFrame[ftUpperLeft]->dwImageWidth ;
		m_sizeLeft.h = m_fsStyle.pFrame[ftUpperLeft]->dwImageHeight ;
	}
	else
	{
		m_sizeLeft.w = m_fsStyle.pFrame[ftLeft] ?
						m_fsStyle.pFrame[ftLeft]->dwImageWidth : 0 ;
		m_sizeLeft.h = m_fsStyle.pFrame[ftUpper] ?
						m_fsStyle.pFrame[ftUpper]->dwImageHeight : 0 ;
	}
	if ( m_fsStyle.pFrame[ftPane] == NULL )
	{
		return	eslErrGeneral ;
	}
	m_sizeMiddle.w = m_fsStyle.pFrame[ftPane]->dwImageWidth ;
	m_sizeMiddle.h = m_fsStyle.pFrame[ftPane]->dwImageHeight ;
	if ( m_fsStyle.pFrame[ftUnderRight] != NULL )
	{
		m_sizeRight.w = m_fsStyle.pFrame[ftUnderRight]->dwImageWidth ;
		m_sizeRight.h = m_fsStyle.pFrame[ftUnderRight]->dwImageHeight ;
	}
	else
	{
		m_sizeRight.w = m_fsStyle.pFrame[ftRight] ?
						m_fsStyle.pFrame[ftRight]->dwImageWidth : 0 ;
		m_sizeRight.h = m_fsStyle.pFrame[ftUnder] ?
						m_fsStyle.pFrame[ftUnder]->dwImageHeight : 0 ;
	}
	m_sizePane.w = (nWidth - (m_sizeLeft.w + m_sizeRight.w)) / m_sizeMiddle.w ;
	m_sizePane.h = (nHeight - (m_sizeLeft.h + m_sizeRight.h)) / m_sizeMiddle.h ;
	if ( m_sizePane.w < 0 )
	{
		m_sizePane.w = 0 ;
	}
	if ( m_sizePane.h < 0 )
	{
		m_sizePane.h = 0 ;
	}
	nWidth = m_sizeLeft.w + m_sizeRight.w + m_sizeMiddle.w * m_sizePane.w ;
	nHeight = m_sizeLeft.h + m_sizeRight.h + m_sizeMiddle.h * m_sizePane.h ;
	//
	// 画像バッファ生成
	//
	int		i ;
	DWORD	fdwFormatType = EIF_RGB_BITMAP ;
	for ( i = 0; i < ftMax; i ++ )
	{
		if ( m_fsStyle.pFrame[i] == NULL )
		{
			continue ;
		}
		if ( m_fsStyle.pFrame[i]->fdwFormatType & EIF_WITH_ALPHA )
		{
			fdwFormatType = EIF_RGBA_BITMAP ;
			break ;
		}
	}
	m_imgFrame.CreateImage( fdwFormatType, nWidth, nHeight, 32 ) ;
	//
	// フレーム画像描画
	//
	HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
	EGL_DRAW_PARAM	dp ;
	hDraw->Initialize( m_imgFrame, NULL, NULL ) ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	//
	for ( int y = 0; y <= m_sizePane.h + 1; y ++ )
	{
		int	iVFrame = ftUpperLeft ;
		dp.ptBasePos.y = 0 ;
		if ( y > 0 )
		{
			dp.ptBasePos.y = m_sizeLeft.h + m_sizeMiddle.h * (y - 1) ;
			if ( y <= m_sizePane.h )
			{
				iVFrame = ftLeft ;
			}
			else
			{
				iVFrame = ftUnderLeft ;
			}
		}
		for ( int x = 0; x <= m_sizePane.w + 1; x ++ )
		{
			int	iFrame = iVFrame ;
			dp.ptBasePos.x = 0 ;
			if ( x > 0 )
			{
				dp.ptBasePos.x = m_sizeLeft.w + m_sizeMiddle.w * (x - 1) ;
				if ( x <= m_sizePane.w )
				{
					iFrame += ftUpper ;
				}
				else
				{
					iFrame += ftUpperRight ;
				}
			}
			dp.pSrcImage = m_fsStyle.pFrame[iFrame] ;
			if ( dp.pSrcImage != NULL )
			{
				if ( !hDraw->PrepareDraw( &dp ) )
				{
					hDraw->DrawImage( ) ;
				}
			}
		}
	}
	hDraw->Release( ) ;
	//
	// 画像関連付け
	//
	AttachImage( m_imgFrame ) ;
	return	eslErrSuccess ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
int EStaticFrameSprite::HitTest( int xLocal, int yLocal )
{
	PEGL_IMAGE_INFO	pFrame = m_imgFrame ;
	if ( (xLocal < 0) || (yLocal < 0) || (pFrame == NULL) )
	{
		return	ftMax ;
	}
	int	iFrame = 0 ;
	if ( xLocal >= m_sizeLeft.w )
	{
		iFrame ++ ;
		if ( xLocal >= m_sizeLeft.w + m_sizeMiddle.w * m_sizePane.w )
		{
			iFrame ++ ;
			if ( xLocal >= (int) pFrame->dwImageWidth )
			{
				return	ftMax ;
			}
		}
	}
	if ( yLocal >= m_sizeLeft.h )
	{
		iFrame += 3 ;
		if ( yLocal >= m_sizeLeft.h + m_sizeMiddle.h * m_sizePane.h )
		{
			iFrame += 3 ;
			if ( yLocal >= (int) pFrame->dwImageHeight )
			{
				return	ftMax ;
			}
		}
	}
	return	iFrame ;
}

