
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2011 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// メッセージ出力スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSMessageSprite::EMsgImageSprite, EImageSprite )
IMPLEMENT_CLASS_INFO( ECSMessageSprite, ECSSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSMessageSprite::EMsgImageSprite::EMsgImageSprite( void )
{
	m_nTime = 0 ;
	m_nFadeSpeed = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSMessageSprite::EMsgImageSprite::~EMsgImageSprite( void )
{
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSMessageSprite::ECSMessageSprite( void )
{
	m_dwFlags = ffGroup | ffTimer ;
	//
	m_pfrmStyle = NULL ;
	m_pdscDefStyle = NULL ;
	m_fFontBold = false ;
	m_fFontBorder = false ;
	m_fMessageInSize = false ;
	//
	m_nDefCharSpeed = 0 ;
	m_nCharSpeed = 0 ;
	m_nCharSpeedRatio = 0x100 ;
	m_nDefFadeSpeed = 0 ;
	m_nFadeSpeed = 0 ;
	//
	m_hRenderPoly = NULL ;
	//
	m_hOutputEvent = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
	m_nOutputCount = true ;
	m_yScrollOffset = 0 ;
	//
	m_rMsgEffectX = 0 ;
	m_rMsgEffectY = 0 ;
	m_rMsgEffectHorz = 1 ;
	m_rMsgEffectVert = 1 ;
	m_rMsgEffectRev = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSMessageSprite::~ECSMessageSprite( void )
{
	Release( ) ;
	//
	if ( m_hRenderPoly != NULL )
	{
		m_hRenderPoly->Release( ) ;
	}
	if ( m_hOutputEvent != NULL )
	{
		::CloseHandle( m_hOutputEvent ) ;
	}
}

// アニメーション進行
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::OnAdvanceAnimation( unsigned int nTime )
{
	EImageSprite::PARAMETER	param ;
	bool			fFixedSprite = true ;
	bool			fScroll = false ;
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	EGL_SIZE		sizeMsg = { 1, 0x7FFF } ;
	if ( pImage != NULL )
	{
		sizeMsg.w = pImage->dwImageWidth ;
		sizeMsg.h = pImage->dwImageHeight ;
	}
	int		i ;
	m_dwCurMessageTime += nTime ;
	if ( m_dwCurMessageTime > 0x7FFFFFFF )
	{
		m_dwCurMessageTime = 0x7FFFFFFF ;
	}
	//
	for ( i = m_nOutputCount; i < (int) m_lstMsgChar.GetSize(); i ++ )
	{
		//
		// 文字に透明度設定
		//
		EMsgImageSprite *	pSprite = m_lstMsgChar.GetAt( i ) ;
		if ( pSprite == NULL )
			continue ;
		//
		SDWORD	nOffsetTime = m_dwCurMessageTime - pSprite->m_nTime ;
		if ( nOffsetTime < 0 )
			continue ;
		//
		unsigned int	nTransparency = 0 ;
		pSprite->GetParameter( param ) ;
		if ( nOffsetTime < pSprite->m_nFadeSpeed )
		{
			if ( pSprite->m_nFadeSpeed > 0 )
			{
				nTransparency =
					0x100 - nOffsetTime * 0x100 / pSprite->m_nFadeSpeed ;
				//
				double	t =
					1.0 - (double) nOffsetTime / pSprite->m_nFadeSpeed ;
				t *= t ;
				param.ptDstPos.x =
					pSprite->m_ptTarget.x
						+ eriRoundR32ToInt( (REAL32) (m_rMsgEffectX * t) ) ;
				param.ptDstPos.y =
					pSprite->m_ptTarget.y - m_yScrollOffset
						+ eriRoundR32ToInt( (REAL32) (m_rMsgEffectY * t) ) ;
				param.rRevAngle = (REAL32) (m_rMsgEffectRev * t) ;
				param.rHorzUnit = (REAL32) ((m_rMsgEffectHorz - 1) * t + 1) ;
				param.rVertUnit = (REAL32) ((m_rMsgEffectVert - 1) * t + 1) ;
			}
		}
		else
		{
			param.ptDstPos = pSprite->m_ptTarget ;
			param.ptDstPos.y -= m_yScrollOffset ;
			param.rRevAngle = 0 ;
			param.rHorzUnit = 1 ;
			param.rVertUnit = 1 ;
		}
		param.nTransparency = nTransparency ;
		pSprite->SetParameter( param ) ;
		//
		// スクロール判定
		//
		if ( nTransparency == 0 )
		{
			EGL_RECT	rect = pSprite->GetRectangle( ) ;
			if ( rect.bottom >= sizeMsg.h )
			{
				fScroll = !m_fMessageInSize ;
			}
		}
		//
		// 終了判定
		//
		if ( nOffsetTime >= pSprite->m_nFadeSpeed )
		{
			if ( fFixedSprite && !fScroll )
			{
				m_nOutputCount = i + 1 ;
				if ( m_nOutputCount >= m_lstMsgChar.GetSize() )
				{
					if ( m_hOutputEvent != NULL )
					{
						::SetEvent( m_hOutputEvent ) ;
					}
				}
			}
		}
		else
		{
			fFixedSprite = false ;
		}
	}
	//
	// スクロール処理
	//
	if ( fScroll && !m_fMessageInSize )
	{
		for ( i = 0; i < (int) m_lstMsgChar.GetSize(); i ++ )
		{
			EMsgImageSprite *	pSprite = m_lstMsgChar.GetAt( i ) ;
			if ( pSprite == NULL )
				continue ;
			EGL_POINT	ptMsg = pSprite->GetPosition( ) ;
			ptMsg.y -= 1 ;
			pSprite->MovePosition( ptMsg ) ;
			//
			EGL_RECT	rect = pSprite->GetRectangle( ) ;
			if ( rect.bottom < 0 )
			{
				if ( m_nOutputCount >= (unsigned int) i )
				{
					m_nOutputCount -- ;
				}
				RemoveSprite( pSprite ) ;
				m_lstMsgChar.RemoveAt( i -- ) ;
			}
		}
		m_yScrollOffset ++ ;
	}
	return	ECSSprite::OnAdvanceAnimation( nTime ) ;
}

// メッセージスプライト削除
//////////////////////////////////////////////////////////////////////////////
void ECSMessageSprite::DeleteImage( void )
{
	for ( int i = 0; i < (int) m_lstMsgChar.GetSize(); i ++ )
	{
		EMsgImageSprite *	pSprite = m_lstMsgChar.GetAt( i ) ;
		if ( pSprite != NULL )
		{
			RemoveSprite( pSprite ) ;
		}
	}
	m_lstMsgChar.RemoveAll( ) ;
	ECSSprite::DeleteImage( ) ;
}

// リソース開放
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Release( void )
{
	ClearMessage( ) ;
	ECSSprite::Release( ) ;
	DeleteImage( ) ;
	//
	m_refRsrcManager.SetReference( NULL ) ;
	//
	return	eslErrSuccess ;
}

// 全てのスプライトを分離
//////////////////////////////////////////////////////////////////////////////
void ECSMessageSprite::DetachAllSprite( void )
{
	m_lstMsgChar.RemoveAll( ) ;
	ESpriteInterface::DetachAllSprite( ) ;
}

// 全てのスプライトを削除
//////////////////////////////////////////////////////////////////////////////
void ECSMessageSprite::RemoveAllSprite( void )
{
	ClearMessage( ) ;
	ESpriteInterface::RemoveAllSprite( ) ;
}

// 文字フォント設定
//////////////////////////////////////////////////////////////////////////////
void ECSMessageSprite::SetSpriteFontFace( const wchar_t * pwszFont )
{
	if ( m_pdscDefStyle != NULL )
	{
		EDescription *	pdscFont =
			m_pdscDefStyle->CreateContentTagAs( 0, L"font" ) ;
		pdscFont->SetAttrString( L"face", pwszFont ) ;
		SetFontFaceName( pwszFont ) ;
	}
}

// メッセージスプライト作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::CreateMessage
	( DWORD dwWidth, DWORD dwHeight,
		const EGL_RECT * pMsgRect, bool fOutputInSize )
{
//	Release( ) ;
	ClearMessage( ) ;
	//
	if ( CreateImage
		( EIF_RGBA_BITMAP, dwWidth, dwHeight, 32, 0 ) == NULL )
	{
		return	eslErrGeneral ;
	}
	m_fdwFormatImage = EIF_RGBA_BITMAP ;
	m_dwWidthImage = dwWidth ;
	m_dwHeightImage = dwHeight ;
	//
	EGLRect	rctMsgView( 0, 0, dwWidth - 1, 0x7FFF ) ;
	if ( pMsgRect != NULL )
	{
		rctMsgView = *pMsgRect ;
		//
		if ( !fOutputInSize )
		{
			rctMsgView.bottom = 0x7FFF ;
		}
	}
	m_fMessageInSize = fOutputInSize ;
	m_rfiText.SetViewRect( rctMsgView ) ;
	//
	if ( m_hRenderPoly == NULL )
	{
		m_hRenderPoly = ::eglCreateRenderPolygon( ) ;
	}
	//
	UpdateCurrentStyle( ) ;
	ClearMessage( ) ;
	//
	return	eslErrSuccess ;
}

// メッセージ出力
//////////////////////////////////////////////////////////////////////////////
int ECSMessageSprite::OutputMessage( const wchar_t * pwszMsg )
{
	//
	// 現在出力中ならば完了させる
	//
	FlushMessage( ) ;
	//
	// メッセージをフォーマットして設定
	//
	bool	fEscape = false ;
	EStreamWideString	swsMsg = pwszMsg ;
	m_nMsgTimeCount = 0 ;
	while ( !swsMsg.IsIndexOverflow() && !fEscape )
	{
		//
		// \ 記号までを通常の書式で出力
		//
		unsigned int	iLast = swsMsg.GetIndex( ) ;
		swsMsg.PassEnclosedString( L'\\' ) ;
		EWideString	wstrText =
			swsMsg.Middle( iLast, swsMsg.GetIndex() - iLast ) ;
		int	nOutCount = DrawMessage( wstrText ) ;
		if ( (int) wstrText.GetLength() > nOutCount )
		{
			swsMsg.MoveIndex( iLast + nOutCount ) ;
			break ;
		}
		//
		// \ 記号シーケンスを取得
		//
		if ( swsMsg.GetCharacter() != L'\\' )
		{
			break ;
		}
		iLast = swsMsg.GetIndex( ) ;
		wchar_t	wchFirst = swsMsg.GetCharacter( ) ;
		wchar_t	wchNext ;
		bool	fEffectColor = false ;
		switch ( wchFirst )
		{
		case	L'\\':
			//
			// \ 記号出力
			//
			if ( DrawMessage( L"\\" ) == 0 )
			{
				swsMsg.MoveIndex( iLast - 1 ) ;
				fEscape = true ;
			}
			break ;

		case	L'n':
			//
			// 書式解除
			//
			iLast = swsMsg.GetIndex( ) ;
			switch ( swsMsg.GetCharacter() )
			{
			case	L'b':
				SetFontBold( false ) ;
				break ;
			case	L'i':
				SetFontItalic( false ) ;
				break ;
			case	L':':
				DrawMessage( L"\n" ) ;
				break ;
			case	L';':
				{
					int	nOutCharCount = m_lstMsgChar.GetSize( ) ;
					iLast = swsMsg.GetIndex( ) ;
					while ( !swsMsg.IsIndexOverflow() )
					{
						wchar_t	wch = swsMsg.GetCharacter( ) ;
						if ( (wch == L';') || (wch == L':') )
						{
							wstrText =
								swsMsg.Middle( iLast, swsMsg.GetIndex() - iLast - 1 ) ;
							iLast = swsMsg.GetIndex() ;
							nOutCount = DrawWordWithoutWrapping( wstrText ) ;
							if ( nOutCount < (int) wstrText.GetLength() )
							{
								m_lstMsgChar.SetSize( nOutCharCount ) ;
								swsMsg.MoveIndex( iLast - 1 ) ;
								fEscape = true ;
								break ;
							}
							if ( wch == L':' )
							{
								break ;
							}
						}
					}
				}
				break ;
			default:
				swsMsg.MoveIndex( iLast ) ;
				break ;
			}
			break ;

		case	L'f':
			//
			// フォントフェース指定
			//
			if ( swsMsg.GetCharacter() == L';' )
			{
				SetFontFaceName( swsMsg.GetEnclosedString( L':' ) ) ;
			}
			else
			{
				SetFontFaceName( NULL ) ;
			}
			break ;

		case	L'b':
			//
			// 太字指定
			//
			SetFontBold( true ) ;
			break ;

		case	L'i':
			//
			// 斜体指定
			//
			SetFontItalic( true ) ;
			break ;

		case	L's':
			if ( swsMsg.CurrentCharacter() == L':' )
			{
				//
				// デフォルトフォントサイズ指定
				//
				SetFontSize( -1, false ) ;
				swsMsg.GetCharacter( ) ;
			}
			else if ( swsMsg.GetCharacter() == L';' )
			{
				//
				// フォントサイズ指定
				//
				wchFirst = swsMsg.CurrentCharacter( ) ;
				SetFontSize
					( swsMsg.GetInteger(),
						((wchFirst == L'+') || (wchFirst == L'-')) ) ;
				swsMsg.GetCharacter( ) ;
			}
			else if ( swsMsg.Middle(iLast,6) == L"style;" )
			{
				//
				// スタイル設定
				//
				swsMsg.MoveIndex( iLast + 6 ) ;
				SetFontStyle( swsMsg.GetEnclosedString( L':' ) ) ;
			}
			else
			{
				swsMsg.MoveIndex( iLast ) ;
			}
			break ;

		case	L'c':
			if ( (swsMsg.CurrentCharacter() == L';')
				|| (swsMsg.CurrentCharacter() == L':') )
			{
				//
				// 文字色指定
				//
				EGL_PALETTE	rgbText, rgbShadow ;
				bool	fText = false, fShadow = false ;
				rgbText.dwPixelCode = 0 ;
				rgbShadow.dwPixelCode = 0 ;
				if ( swsMsg.GetCharacter() == L';' )
				{
					rgbText.dwPixelCode = swsMsg.GetInteger( 16 ) ;
					fText = true ;
					//
					if ( swsMsg.GetCharacter() == L';' )
					{
						rgbShadow.dwPixelCode = swsMsg.GetInteger( 16 ) ;
						fShadow = true ;
						swsMsg.GetCharacter( ) ;
					}
				}
				SetFontColor( rgbText, fText, rgbShadow, fShadow ) ;
			}
			else if ( swsMsg.Middle(iLast,7) == L"center:" )
			{
				//
				// 文字列中央揃え
				//
				m_tsMsgStyle.taAlign = EStaticTextSprite::taCenter ;
				swsMsg.MoveIndex( iLast + 7 ) ;
			}
			else
			{
				swsMsg.MoveIndex( iLast ) ;
			}
			break ;

		case	L'r':
			if ( swsMsg.GetCharacter() == L';' )
			{
				//
				// ルビ指定
				//
				EWideString	wstrWord = swsMsg.GetEnclosedString( L';' ) ;
				EWideString	wstrReading = swsMsg.GetEnclosedString( L':' ) ;
				if ( DrawWordWithReading( wstrWord, wstrReading ) )
				{
					swsMsg.MoveIndex( iLast - 1 ) ;
					fEscape = true ;
				}
			}
			else if ( swsMsg.Middle(iLast,6) == L"right:" )
			{
				//
				// 文字列右揃え
				//
				m_tsMsgStyle.taAlign = EStaticTextSprite::taRight ;
				swsMsg.MoveIndex( iLast + 6 ) ;
			}
			else
			{
				swsMsg.MoveIndex( iLast ) ;
			}
			break ;

		case	L'x':
			//
			// 外字指定
			//
			wchNext = swsMsg.GetCharacter() ;
			if ( wchNext == L'c' )
			{
				wchNext = swsMsg.GetCharacter() ;
				fEffectColor = true ;
			}
			if ( wchNext == L';' )
			{
				EWideString	wstrID = swsMsg.GetEnclosedString( L':' ) ;
				if ( DrawExtensionChar( wstrID, fEffectColor, fEffectColor ) )
				{
					swsMsg.MoveIndex( iLast - 1 ) ;
					fEscape = true ;
				}
			}
			break ;

		case	L'v':
			//
			// メッセージ速度設定
			//
			if ( swsMsg.GetCharacter() == L';' )
			{
				m_nCharSpeed = swsMsg.GetInteger( ) ;
				if ( swsMsg.GetCharacter() == L';' )
				{
					m_nFadeSpeed = swsMsg.GetInteger( ) ;
					swsMsg.GetCharacter( ) ;
				}
				else
				{
					m_nFadeSpeed = m_nDefFadeSpeed ;
				}
			}
			else
			{
				m_nCharSpeed = m_nDefCharSpeed * m_nCharSpeedRatio / 0x100 ;
				m_nFadeSpeed = m_nDefFadeSpeed ;
			}
			break ;

		case	L'h':
			//
			// 行間指定
			//
			if ( swsMsg.GetCharacter() == L';' )
			{
				wchFirst = swsMsg.CurrentCharacter( ) ;
				SetLineHeight
					( swsMsg.GetInteger(),
						((wchFirst == L'+') || (wchFirst == L'-')) ) ;
				swsMsg.GetCharacter( ) ;
			}
			else
			{
				SetLineHeight( -1, false ) ;
			}
			break ;
		}
	}
	//
	// 文字画像を設定
	//
	for ( unsigned int i = m_nOutputCount; i < m_lstMsgChar.GetSize(); i ++ )
	{
		EMsgImageSprite *	pmisChar = m_lstMsgChar.GetAt( i ) ;
		if ( pmisChar == NULL )
			continue ;
		//
		EGL_POINT	ptChar = pmisChar->GetPosition( ) ;
		ptChar.y -= m_yScrollOffset ;
		pmisChar->MovePosition( ptChar ) ;
		pmisChar->SetTransparency( 0x100 ) ;
		AddSprite( 0, pmisChar ) ;
		pmisChar->SetVisible( true ) ;
	}
	m_dwCurMessageTime = 0 ;
	//
	// ログに追加
	//
	m_wstrTextLog += pwszMsg ;
	//
	return	swsMsg.GetIndex() ;
}

// 文字列を出力
//////////////////////////////////////////////////////////////////////////////
int ECSMessageSprite::DrawMessage( const wchar_t * pwszMsg )
{
	unsigned int	i, nCount ;
	if ( m_tsMsgStyle.taAlign != EStaticTextSprite::taLeft )
	{
		int	nTextWidth = m_rfiText.GetTextWidth( pwszMsg ) ;
		EGL_POINT	ptCur = m_rfiText.GetCursorPos( ) ;
		EGL_RECT	rectMsg = m_rfiText.GetViewRect( ) ;
		if ( nTextWidth <= rectMsg.right - ptCur.x )
		{
			if ( m_tsMsgStyle.taAlign == EStaticTextSprite::taCenter )
			{
				ptCur.x += ((rectMsg.right - ptCur.x) - nTextWidth) / 2 ;
			}
			else
			{
				ptCur.x = rectMsg.right - nTextWidth ;
			}
			m_rfiText.MoveCursorPos( ptCur ) ;
		}
	}
	//
	int	nOutCount = m_rfiText.DrawText( pwszMsg ) ;
	nCount = m_rfiText.GetCharacterCount( ) ;
	if ( nCount > 0 )
	{
		m_tsMsgStyle.taAlign = EStaticTextSprite::taLeft ;
	}
	//
	for ( i = 0; i < nCount; i ++ )
	{
		EImageSprite *	pisChar = m_rfiText.GetCharacterAt( i ) ;
		if ( pisChar == NULL )
			continue ;
		//
		EMsgImageSprite *	pmisChar =
			CreateMsgCharFromSprite( pisChar ) ;
		if ( pmisChar == NULL )
			continue ;
		//
		pmisChar->m_nTime = m_nMsgTimeCount ;
		m_nMsgTimeCount += m_nCharSpeed ;
		m_lstMsgChar.Add( pmisChar ) ;
	}
	m_rfiText.RemoveAllCharacter( ) ;
	return	nOutCount ;
}

// 外字を出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::DrawExtensionChar
	( const wchar_t * pwszID, bool fEffectColor, bool fEffectDeco )
{
	if ( m_pfrmStyle != NULL )
	{
		EGLImage *	pImage =
			ESLTypeCast<EGLImage>
				( m_pfrmStyle->GetResourceAs( pwszID ) ) ;
		if ( (pImage != NULL) && (pImage->GetInfo() != NULL) )
		{
			EMsgImageSprite *	pmis = new EMsgImageSprite ;
			if ( pImage->GetFormatType() == EIF_GRAY_BITMAP )
			{
				pmis->AttachImage( *pImage ) ;
				//
				EMsgImageSprite *
					pmisNew = CreateMsgCharFromSprite( pmis ) ;
				if ( pmisNew != NULL )
				{
					delete	pmis ;
					pmis = pmisNew ;
				}
			}
			else if ( !fEffectColor && !fEffectDeco )
			{
				pmis->AttachImage( *pImage ) ;
			}
			else
			{
				EGLSize	sizeImage = pImage->GetSize() ;
				pmis->CreateImage
					( EIF_RGBA_BITMAP, sizeImage.w, sizeImage.h, 32 ) ;
				//
				m_hRenderPoly->Initialize
						( *pmis, NULL, NULL, &E3DVector(0,0,1) ) ;
				HEGL_DRAW_IMAGE	hDrawImage = m_hRenderPoly->GetDrawImage( ) ;
				//
				EGL_DRAW_PARAM	edp ;
				::eslFillMemory( &edp, 0, sizeof(edp) ) ;
				edp.dwFlags = EGL_APPLY_C_MUL ;
				edp.pSrcImage = pImage->GetInfo() ;
				edp.rgbColorParam1.dwPixelCode =
						m_tsMsgStyle.rgbColor.dwPixelCode | 0xFF000000 ;
				if ( !hDrawImage->PrepareDraw( &edp ) )
				{
					hDrawImage->DrawImage() ;
				}
				//
				if ( fEffectDeco )
				{
					EMsgImageSprite *
						pmisNew = CreateMsgCharFromSprite( pmis ) ;
					if ( pmisNew != NULL )
					{
						delete	pmis ;
						pmis = pmisNew ;
					}
				}
			}
			if ( DrawMsgImage( pmis ) )
			{
				delete	pmis ;
				return	eslErrFailed ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 文字を修飾して新たにスプライトを作成
//////////////////////////////////////////////////////////////////////////////
ECSMessageSprite::EMsgImageSprite *
	ECSMessageSprite::CreateMsgCharFromSprite( EImageSprite * pisChar )
{
	if ( pisChar == NULL )
		return	NULL ;
	PEGL_IMAGE_INFO	pCharInf = *pisChar ;
	if ( pCharInf == NULL )
		return	NULL ;
	if ( m_fMessageInSize )
	{
		EGL_POINT	ptChar = pisChar->GetPosition() ;
		EGL_RECT	rectText = m_rfiText.GetViewRect( ) ;
		if ( !m_rfiText.IsVerticalWriting() )
		{
			if ( ptChar.y > rectText.bottom )
			{
				return	NULL ;
			}
		}
		else
		{
			if ( ptChar.x < 0 )
			{
				return	NULL ;
			}
		}
	}
	//
	// 画像サイズ計算
	//
	EGLSize		sizeChar ;
	EGLPoint	ptOffset( 0, 0 ) ;
	sizeChar.w = pCharInf->dwImageWidth ;
	sizeChar.h = pCharInf->dwImageHeight ;
	if ( m_tsMsgStyle.ptShadowOffset.x < 0 )
	{
		ptOffset.x = - m_tsMsgStyle.ptShadowOffset.x ;
		sizeChar.w += ptOffset.x ;
	}
	else
	{
		sizeChar.w += m_tsMsgStyle.ptShadowOffset.x ;
	}
	if ( m_tsMsgStyle.ptShadowOffset.y < 0 )
	{
		ptOffset.y = - m_tsMsgStyle.ptShadowOffset.y ;
		sizeChar.h += ptOffset.y ;
	}
	else
	{
		sizeChar.h += m_tsMsgStyle.ptShadowOffset.y ;
	}
	if ( m_fFontBold )
	{
		sizeChar.w ++ ;
		sizeChar.h ++ ;
	}
	if ( m_fFontBorder
		|| (m_tsMsgStyle.nFlags & EStaticTextSprite::txfBordering) )
	{
		ptOffset.x += 2 ;
		ptOffset.y += 2 ;
		sizeChar.w += 4 ;
		sizeChar.h += 4 ;
	}
	//
	// 画像バッファ作成
	//
	EGL_POINT	ptCharPos = pisChar->GetPosition() ;
	EMsgImageSprite *	pmisChar = new EMsgImageSprite ;
	pmisChar->m_nFadeSpeed = m_nFadeSpeed ;
	pmisChar->CreateImage
		( EIF_RGBA_BITMAP, sizeChar.w, sizeChar.h, 32, 0 ) ;
	ptCharPos.x -= ptOffset.x ;
	ptCharPos.y -= ptOffset.y ;
	//
	m_hRenderPoly->Initialize( *pmisChar, NULL, NULL, &E3DVector(0,0,1) ) ;
	HEGL_DRAW_IMAGE	hDrawImage = m_hRenderPoly->GetDrawImage( ) ;
	//
	// 文字描画
	//
	EGL_DRAW_PARAM	edp ;
	::eslFillMemory( &edp, 0, sizeof(edp) ) ;
	edp.dwFlags = EGL_DRAW_GLOW_LIGHT | EGL_DRAW_BLEND_ALPHA ;
	edp.ptBasePos = ptOffset + m_tsMsgStyle.ptShadowOffset ;
	edp.pSrcImage = pCharInf ;
	edp.rgbDimColor = edp.rgbLightColor = m_tsMsgStyle.rgbShadow ;
	edp.nTransparency = m_tsMsgStyle.nShadowTrans ;
	if ( !hDrawImage->PrepareDraw( &edp ) )
	{
		hDrawImage->DrawImage( ) ;
	}
	if ( m_fFontBold )
	{
		edp.ptBasePos.x ++ ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
	}
	//
	edp.ptBasePos = ptOffset ;
	edp.rgbDimColor = edp.rgbLightColor = m_tsMsgStyle.rgbColor ;
	edp.nTransparency = m_tsMsgStyle.nTransparency ;
	if ( !hDrawImage->PrepareDraw( &edp ) )
	{
		hDrawImage->DrawImage( ) ;
	}
	if ( m_fFontBold )
	{
		edp.ptBasePos.x ++ ;
		if ( !hDrawImage->PrepareDraw( &edp ) )
		{
			hDrawImage->DrawImage( ) ;
		}
	}
	//
	// 文字縁取り処理
	//
	if ( m_fFontBorder
		|| (m_tsMsgStyle.nFlags & EStaticTextSprite::txfBordering) )
	{
		PEGL_IMAGE_INFO	pInfo = pmisChar->GetInfo( ) ;
		ESLAssert( pInfo != NULL ) ;
		ESLAssert( pInfo->ptrImageArray != NULL ) ;
		ESLAssert( pInfo->dwBitsPerPixel == 32 ) ;
		ESLAssert( sizeof(EGL_PALETTE) == 4 ) ;
		int			x, y, w, h ;
		unsigned int	nAlpha ;
		EGL_PALETTE	rgbBorder = m_tsMsgStyle.rgbShadow ;
		EGL_PALETTE	rgbBorder2 = m_tsMsgStyle.rgbShadow ;
		if ( m_tsMsgStyle.nFlags & EStaticTextSprite::txfBordering )
		{
			rgbBorder = m_tsMsgStyle.rgbBorder ;
			rgbBorder2 = m_tsMsgStyle.rgbBorder ;
		}
		EGL_PALETTE	rgbTemp ;
		PEGL_IMAGE_INFO	pTempSrc = ::eglDuplicateImageBuffer( pInfo ) ;
		BYTE *	pbytDstLine = (BYTE*) pInfo->ptrImageArray ;
		BYTE *	pbytSrcLine = (BYTE*) pTempSrc->ptrImageArray ;
		SDWORD	dwDstLineBytes = pInfo->dwBytesPerLine ;
		SDWORD	dwSrcLineBytes = pTempSrc->dwBytesPerLine ;
		h = pInfo->dwImageHeight - 1 ;
		w = pInfo->dwImageWidth - 1 ;
		pbytDstLine += dwDstLineBytes ;
		pbytSrcLine += dwSrcLineBytes ;
		//
		rgbBorder.dwPixelCode |= 0xFF000000 ;
		//
		for ( y = 1; y < h; y ++ )
		{
			EGL_PALETTE *	pxDstPixel = (EGL_PALETTE*) (pbytDstLine + 4) ;
			BYTE *	pbytSrcPixel = pbytSrcLine + 7 ;
			for ( x = 1; x < w; x ++ )
			{
				if ( pxDstPixel->rgba.Alpha != 0xFF )
				{
					BYTE	px0 = *pbytSrcPixel ;
					BYTE	px1 = *(pbytSrcPixel - dwSrcLineBytes) ;
					BYTE	px2 = *(pbytSrcPixel + dwSrcLineBytes) ;
					BYTE	px3 = *(pbytSrcPixel - 4) ;
					BYTE	px4 = *(pbytSrcPixel + 4) ;
					BYTE	px12msk = (BYTE) (((int) px1 - (int) px2) >> 8) ;
					BYTE	px34msk = (BYTE) (((int) px3 - (int) px4) >> 8) ;
					px1 = (px1 & ~px12msk) | (px2 & px12msk) ;
					px3 = (px3 & ~px34msk) | (px4 & px34msk) ;
					BYTE	px13msk = (BYTE) (((int) px1 - (int) px3) >> 8) ;
					px1 = (px1 & ~px13msk) | (px3 & px13msk) ;
					BYTE	px01msk = (BYTE) (((int) px0 - (int) px1) >> 8) ;
					nAlpha = (px0 & ~px01msk) | (px1 & px01msk) ;
					//
					rgbTemp = *pxDstPixel ;
					if ( rgbBorder.dwPixelCode & 0x00FFFFFF )
					{
						if ( pxDstPixel->rgba.Alpha == 0 )
						{
							*pxDstPixel = rgbBorder * nAlpha ;
						}
						else
						{
							rgbBorder2 = rgbBorder * nAlpha ;
							rgbBorder2.dwPixelCode ^= 0xFF000000 ;
							rgbBorder2 *= (unsigned int) (0x100 - rgbTemp.rgba.Alpha) ;
							rgbTemp.dwPixelCode &= 0x00FFFFFF ;
							rgbBorder2 += rgbTemp ;
							pxDstPixel->dwPixelCode =
								rgbBorder2.dwPixelCode^ 0xFF000000 ;
						}
					}
					else
					{
						pxDstPixel->rgba.Alpha = (BYTE) nAlpha ;
					}
				}
				pbytSrcPixel += 4 ;
				pxDstPixel ++ ;
			}
			pbytDstLine += dwDstLineBytes ;
			pbytSrcLine += dwSrcLineBytes ;
		}
		::eglDeleteImageBuffer( pTempSrc ) ;
	}
	//
	EImageSprite::PARAMETER	param ;
	pmisChar->GetParameter( param ) ;
	param.ptRevCenter.x = sizeChar.w / 2 ;
	param.ptRevCenter.y = sizeChar.h / 2 ;
	param.ptDstPos.x = ptCharPos.x + param.ptRevCenter.x ;
	param.ptDstPos.y = ptCharPos.y + param.ptRevCenter.y ;
	pmisChar->m_ptTarget = param.ptDstPos ;
	pmisChar->SetParameter( param ) ;
	//
	return	pmisChar ;
}

// 文字列を行末を折り返さないように出力
//////////////////////////////////////////////////////////////////////////////
int ECSMessageSprite::DrawWordWithoutWrapping( const wchar_t * pwszWord )
{
	//
	// 文字描画
	//
	int	nTextWidth = m_rfiText.GetTextWidth( pwszWord ) ;
	EGL_POINT	ptCur = m_rfiText.GetCursorPos( ) ;
	EGL_RECT	rectMsg = m_rfiText.GetViewRect( ) ;
	if ( !m_rfiText.IsVerticalWriting() )
	{
		if ( nTextWidth > rectMsg.right - ptCur.x )
		{
			DrawMessage( L"\n" ) ;
		}
	}
	else
	{
		if ( nTextWidth > rectMsg.bottom - ptCur.y )
		{
			DrawMessage( L"\n" ) ;
		}
	}
	return	DrawMessage( pwszWord ) ;
}

// ルビ文字列を画像化してスプライトを作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::DrawWordWithReading
	( const wchar_t * pwszWord, const wchar_t * pwszReading )
{
	//
	// 文字描画
	//
	int	nTextWidth = m_rfiText.GetTextWidth( pwszWord ) ;
	m_rfiText.RemoveAllCharacter( ) ;
	m_rfiText.FitTextToWidth( pwszWord, 0, 0, nTextWidth ) ;
	int	yReading = - m_fontMsg.GetSize() ;
	if ( yReading + (int) m_rfiText.GetLineHeight()
							- (int) m_fontReading.GetSize() < 0 )
	{
		yReading = m_fontReading.GetSize() - m_rfiText.GetLineHeight() ;
	}
	SakuraGL::SGLFont *		pFontReading = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	style ;
	SSystem::SString		strFont ;
	style.FromLogFont( m_fontReading.LogFont(), strFont ) ;
	pFontReading->SetStyle( style ) ;
	m_rfiText.SetSGLFont( pFontReading ) ;
//	m_rfiText.SetFont( m_fontReading ) ;
	m_rfiText.FitTextToWidth( pwszReading, 0, yReading, nTextWidth ) ;
	//
	SakuraGL::SGLFont *		pFontMsg = new SakuraGL::SGLFont ;
	style.FromLogFont( m_fontMsg.LogFont(), strFont ) ;
	pFontMsg->SetStyle( style ) ;
	m_rfiText.SetSGLFont( pFontMsg ) ;
//	m_rfiText.SetFont( m_fontMsg ) ;
	//
	// 文字画像バッファ作成
	//
	int		i, nCharCount = m_rfiText.GetCharacterCount() ;
	EGLRect	rectWords( 0, 0, 0, 0 ) ;
	if ( nCharCount == 0 )
	{
		return	eslErrSuccess ;
	}
	EObjArray<EMsgImageSprite>	lstChar ;
	for ( i = 0; i < nCharCount; i ++ )
	{
		EMsgImageSprite *	pmisChar =
			CreateMsgCharFromSprite( m_rfiText.GetCharacterAt(i) ) ;
		if ( pmisChar == NULL )
		{
			continue ;
		}
		lstChar.Add( pmisChar ) ;
		//
		EGL_RECT	rectChar = pmisChar->GetRectangle() ;
		rectWords |= rectChar ;
	}
	if ( rectWords.IsEmpty() )
	{
		return	eslErrSuccess ;
	}
	EMsgImageSprite *	pmisWord = new EMsgImageSprite ;
	pmisWord->CreateImage
		( EIF_RGBA_BITMAP,
			rectWords.right - rectWords.left + 1,
			rectWords.bottom - rectWords.top + 1, 32, 0 ) ;
	//
	nCharCount = lstChar.GetSize() ;
	for ( i = 0; i < nCharCount; i ++ )
	{
		EMsgImageSprite *	pmisChar = lstChar.GetAt( i ) ;
		EGL_POINT	ptCharPos = pmisChar->GetPosition() ;
		ptCharPos.x -= rectWords.left ;
		ptCharPos.y -= rectWords.top ;
		pmisChar->MovePosition( ptCharPos ) ;
		pmisChar->SetVisible( true ) ;
		//
		m_hRenderPoly->Initialize
				( *pmisWord, NULL, NULL, &E3DVector(0,0,1) ) ;
		pmisChar->Draw( m_hRenderPoly ) ;
	}
	m_rfiText.RemoveAllCharacter( ) ;
	//
	// 画像設定
	//
	if ( DrawMsgImage
		( pmisWord, EWideString(pwszWord).GetLength(),
				true, rectWords.left, rectWords.top, nTextWidth ) )
	{
		delete	pmisWord ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 文字画像スプライトを出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::DrawMsgImage
	( ECSMessageSprite::EMsgImageSprite * pmis, int nWordCount,
			bool fCharOffset, int xOffset, int yOffset, int nTextPitch )
{
	if ( pmis == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pmis->GetInfo() == NULL )
	{
		delete	pmis ;
		return	eslErrSuccess ;
	}
	pmis->m_nTime = m_nMsgTimeCount ;
	pmis->m_nFadeSpeed = m_nFadeSpeed ;
	m_nMsgTimeCount += m_nCharSpeed * nWordCount ;
	//
	EImageSprite::PARAMETER	param ;
	EGL_POINT		ptCur = m_rfiText.GetCursorPos( ) ;
	EGL_RECT		rectText = m_rfiText.GetViewRect( ) ;
	PEGL_IMAGE_INFO	pInfo = *pmis ;
	if ( !m_rfiText.IsVerticalWriting() )
	{
		if ( ptCur.x + (int) pInfo->dwImageWidth > rectText.right )
		{
			m_rfiText.MoveToNextLine( ) ;
			ptCur = m_rfiText.GetCursorPos( ) ;
		}
	}
	else
	{
		if ( ptCur.y + (int) pInfo->dwImageHeight > rectText.bottom )
		{
			m_rfiText.MoveToNextLine( ) ;
			ptCur = m_rfiText.GetCursorPos( ) ;
		}
	}
	if ( m_fMessageInSize )
	{
		if ( !m_rfiText.IsVerticalWriting() )
		{
			if ( ptCur.y > rectText.bottom )
			{
				return	eslErrGeneral ;
			}
		}
		else
		{
			if ( ptCur.x < 0 )
			{
				return	eslErrGeneral ;
			}
		}
	}
	EGL_POINT	ptImage = ptCur ;
	if ( fCharOffset )
	{
		ptImage.x += xOffset ;
		ptImage.y += yOffset ;
	}
	else
	{
		if ( m_rfiText.GetLineHeight() > pInfo->dwImageHeight )
		{
			ptImage.y += (int) m_rfiText.GetLineHeight()
								- (int) pInfo->dwImageHeight ;
		}
	}
	pmis->GetParameter( param ) ;
	param.ptRevCenter.x = pInfo->dwImageWidth / 2 ;
	param.ptRevCenter.y = pInfo->dwImageHeight / 2 ;
	param.ptDstPos.x = ptImage.x + param.ptRevCenter.x ;
	param.ptDstPos.y = ptImage.y + param.ptRevCenter.y ;
	pmis->m_ptTarget = param.ptDstPos ;
	pmis->SetParameter( param ) ;
	m_lstMsgChar.Add( pmis ) ;
	//
	if ( !m_rfiText.IsVerticalWriting() )
	{
		if ( fCharOffset )
		{
			ptCur.x += nTextPitch ;
		}
		else
		{
			ptCur.x += pInfo->dwImageWidth + xOffset ;
		}
	}
	else
	{
		ptCur.y += pInfo->dwImageHeight + yOffset ;
	}
	m_rfiText.MoveCursorPos( ptCur ) ;
	return	eslErrSuccess ;
}

// メッセージ出力を即時完了させる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::FlushMessage( void )
{
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	EGL_SIZE	sizeMsg = { 1, 0x7FFF } ;
	int		i, yNeedScroll = 0 ;
	if ( pImage != NULL )
	{
		sizeMsg.w = pImage->dwImageWidth ;
		sizeMsg.h = pImage->dwImageHeight ;
	}
	for ( i = m_nOutputCount; i < (int) m_lstMsgChar.GetSize(); i ++ )
	{
		EMsgImageSprite *	pmis = m_lstMsgChar.GetAt( i ) ;
		if ( pmis == NULL )
			continue ;
		//
		EImageSprite::PARAMETER	param ;
		pmis->GetParameter( param ) ;
		param.ptDstPos.x = pmis->m_ptTarget.x ;
		param.ptDstPos.y = pmis->m_ptTarget.y - m_yScrollOffset ;
		param.nTransparency = 0 ;
		param.rRevAngle = 0 ;
		param.rHorzUnit = 1 ;
		param.rVertUnit = 1 ;
		pmis->SetParameter( param ) ;
		pmis->SetVisible( true ) ;
		//
		EGL_RECT	rect = pmis->GetRectangle( ) ;
		if ( rect.bottom + 1 - sizeMsg.h >  yNeedScroll )
		{
			yNeedScroll = rect.bottom + 1 - sizeMsg.h ;
		}
	}
	if ( yNeedScroll > 0 )
	{
		for ( i = 0; i < (int) m_lstMsgChar.GetSize(); i ++ )
		{
			EMsgImageSprite *	pSprite = m_lstMsgChar.GetAt( i ) ;
			if ( pSprite == NULL )
				continue ;
			EGL_POINT	ptMsg = pSprite->GetPosition( ) ;
			ptMsg.y -= yNeedScroll ;
			pSprite->MovePosition( ptMsg ) ;
			//
			EGL_RECT	rect = pSprite->GetRectangle( ) ;
			if ( rect.bottom < 0 )
			{
				RemoveSprite( pSprite ) ;
				m_lstMsgChar.RemoveAt( i -- ) ;
			}
		}
		m_yScrollOffset += yNeedScroll ;
	}
	m_nOutputCount = m_lstMsgChar.GetSize( ) ;
	::SetEvent( m_hOutputEvent ) ;
	return	eslErrSuccess ;
}

// メッセージをクリアする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::ClearMessage( void )
{
	for ( int i = 0; i < (int) m_lstMsgChar.GetSize(); i ++ )
	{
		EMsgImageSprite *	pChar = m_lstMsgChar.GetAt( i ) ;
		if ( pChar != NULL )
		{
			pChar->SetVisible( false ) ;
			RemoveSprite( pChar ) ;
		}
	}
	m_lstMsgChar.RemoveAll( ) ;
	if ( m_rfiText.IsVerticalWriting() )
	{
		EGL_RECT	rectView = m_rfiText.GetViewRect( ) ;
		m_rfiText.MoveCursorPos
			( EGLPoint( rectView.right, rectView.top ) ) ;
	}
	else
	{
		EGL_RECT	rectView = m_rfiText.GetViewRect( ) ;
		m_rfiText.MoveCursorPos
			( EGLPoint( rectView.left, rectView.top ) ) ;
	}
	m_nOutputCount = 0 ;
	m_yScrollOffset = 0 ;
	m_wstrTextLog = L"" ;
	::SetEvent( m_hOutputEvent ) ;
	return	eslErrSuccess ;
}

// 現在メッセージが出力中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSMessageSprite::IsMessagePending( void )
{
	return	(m_nOutputCount < m_lstMsgChar.GetSize()) ;
}

// メッセージの出力が完了するまで待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::WaitUntilOutputMessage( DWORD dwTimeout )
{
	DWORD	dwWaitResult ;
	dwWaitResult = ::WaitForSingleObject( m_hOutputEvent, dwTimeout ) ;
	if ( dwWaitResult == WAIT_OBJECT_0 )
	{
		return	eslErrSuccess ;
	}
	else if ( dwWaitResult == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	return	eslErrGeneral ;
}

// スタイルを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::AttachMessageStyle
	( EFormResourceManager * pfrmStyle,
			const wchar_t * pwszDefaultStyle )
{
	m_pfrmStyle = pfrmStyle ;
	m_wstrDefStyle = pwszDefaultStyle ;
	m_pdscDefStyle = m_pfrmStyle->GetStyleAs( m_wstrDefStyle ) ;
	if ( m_pdscDefStyle == NULL )
	{
		return	eslErrGeneral ;
	}
	m_dscMsgStyle = *m_pdscDefStyle ;
	m_dscMsgStyle.SetTag( L"" ) ;
	UpdateCurrentStyle( ) ;
	return	eslErrSuccess ;
}

// デフォルトメッセージ速度を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetDefaultMsgSpeed
	( long int nCharSpeed, long int nFadeSpeed, long int nSpeedRatio )
{
	m_nCharSpeedRatio = nSpeedRatio ;
	m_nCharSpeed = (m_nDefCharSpeed = nCharSpeed) * m_nCharSpeedRatio / 0x100 ;
	m_nFadeSpeed = m_nDefFadeSpeed = nFadeSpeed ;
	return	eslErrSuccess ;
}

// 文字出力エフェクトを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetMessageEffect
	( double x, double y, double rHorz, double rVert, double rRev )
{
	m_rMsgEffectX = x ;
	m_rMsgEffectY = y ;
	m_rMsgEffectHorz = rHorz ;
	m_rMsgEffectVert = rVert ;
	m_rMsgEffectRev = rRev ;
	return	eslErrSuccess ;
}

// フォントフェース設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontFaceName( const wchar_t * pwszFaceList )
{
	EWideString	wstrFaceList ;
	if ( pwszFaceList == NULL )
	{
		if ( m_pdscDefStyle != NULL )
		{
			EDescription *	pdscFont =
				m_pdscDefStyle->CreateContentTagAs( 0, L"font" ) ;
			wstrFaceList = pdscFont->GetAttrString( L"face", NULL ) ;
		}
	}
	else
	{
		wstrFaceList = pwszFaceList ;
	}
	EDescription *	pdscFont =
		m_dscMsgStyle.CreateContentTagAs( 0, L"font" ) ;
	pdscFont->SetAttrString( L"face", wstrFaceList ) ;
	return	UpdateCurrentStyle( ) ;
}

// 太字設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontBold( bool fBold )
{
	EDescription *	pdscFont =
		m_dscMsgStyle.CreateContentTagAs( 0, L"font" ) ;
	pdscFont->SetAttrString( L"bold", (fBold ? L"true" : L"false") ) ;
	return	UpdateCurrentStyle( ) ;
}

// 斜体設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontItalic( bool fItalic )
{
	EDescription *	pdscFont =
		m_dscMsgStyle.CreateContentTagAs( 0, L"font" ) ;
	pdscFont->SetAttrString( L"italic", (fItalic ? L"true" : L"false") ) ;
	return	UpdateCurrentStyle( ) ;
}

// フォントサイズ設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontSize( int nFontSize, bool fOffset )
{
	EDescription *	pdscFont =
		m_dscMsgStyle.CreateContentTagAs( 0, L"font" ) ;
	if ( fOffset )
	{
		nFontSize += pdscFont->GetAttrInteger( L"size", 16 ) ;
	}
	else if ( nFontSize == -1 )
	{
		if ( m_pdscDefStyle != NULL )
		{
			nFontSize = m_pdscDefStyle->
				CreateContentTagAs( 0, L"font" )->
						GetAttrInteger( L"size", 16 ) ;
		}
		else
		{
			nFontSize = 16 ;
		}
	}
	pdscFont->SetAttrInteger( L"size", nFontSize ) ;
	return	UpdateCurrentStyle( ) ;
}

// 文字色設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontColor
	( EGL_PALETTE rgbText, bool fText,
		EGL_PALETTE rgbShadow, bool fShadow )
{
	if ( fText )
	{
		m_dscMsgStyle.CreateContentTagAs( 0, L"text" )->SetAttrString
			( L"color", L"0" + EWideString(rgbText.dwPixelCode) + L"H" ) ;
		if ( fShadow )
		{
			m_dscMsgStyle.CreateContentTagAs( 0, L"shadow" )->SetAttrString
				( L"color", L"0" + EWideString(rgbShadow.dwPixelCode) + L"H" ) ;
		}
	}
	else
	{
		if ( m_pdscDefStyle != NULL )
		{
			*(m_dscMsgStyle.CreateContentTagAs( 0, L"text" ))
				= *(m_pdscDefStyle->CreateContentTagAs( 0, L"text" )) ;
			*(m_dscMsgStyle.CreateContentTagAs( 0, L"shadow" ))
				= *(m_pdscDefStyle->CreateContentTagAs( 0, L"shadow" )) ;
		}
	}
	return	UpdateCurrentStyle( ) ;
}

// 文字の影の透明度を設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetShadowTransparency( unsigned int nTransparency )
{
	m_dscMsgStyle.SetIntegerAt( L"shadow", L"transparency", nTransparency ) ;
	return	UpdateCurrentStyle( ) ;
}

// 文字の縁取りを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontBordering( bool fBorder )
{
	m_fFontBorder = fBorder ;
	return	eslErrSuccess ;
}

// 行間を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetLineHeight( int nLineHeight, bool fOffset )
{
	EDescription *	pdscAlign =
		m_dscMsgStyle.CreateContentTagAs( 0, L"arrange" ) ;
	if ( fOffset )
	{
		if ( m_pdscDefStyle != NULL )
		{
			nLineHeight += m_pdscDefStyle->
				CreateContentTagAs( 0, L"arrange" )->
					GetAttrInteger( L"line_height", 16 ) ;
		}
		else
		{
			nLineHeight += 16 ;
		}
	}
	else if ( nLineHeight == -1 )
	{
		if ( m_pdscDefStyle != NULL )
		{
			nLineHeight = m_pdscDefStyle->
				CreateContentTagAs( 0, L"arrange" )->
					GetAttrInteger( L"line_height", 16 ) ;
		}
		else
		{
			nLineHeight = 16 ;
		}
	}
	pdscAlign->SetAttrInteger( L"line_height", nLineHeight ) ;
	return	UpdateCurrentStyle( ) ;
}

// スタイルを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::SetFontStyle( const wchar_t * pwszStyleID )
{
	if ( pwszStyleID == NULL )
	{
		pwszStyleID = m_wstrDefStyle ;
	}
	EDescription *	pdscStyle = m_pfrmStyle->GetStyleAs( pwszStyleID ) ;
	if ( pdscStyle == NULL )
	{
		return	eslErrGeneral ;
	}
	m_dscMsgStyle = *pdscStyle ;
	m_dscMsgStyle.SetTag( L"" ) ;
	return	UpdateCurrentStyle( ) ;
}

// 表示文字の外接矩形を取得する
//////////////////////////////////////////////////////////////////////////////
bool ECSMessageSprite::GetMessageRect( EGL_RECT& rect ) const
{
	bool	fRect = false ;
	EGLRect	rectMsg ;
	for ( int i = 0; i < (int) m_lstMsgChar.GetSize(); i ++ )
	{
		EMsgImageSprite *	pSprite = m_lstMsgChar.GetAt( i ) ;
		if ( pSprite == NULL )
		{
			continue ;
		}
		EGLRect	rectChar = pSprite->GetRectangle( ) ;
		if ( !rectChar.IsEmpty() )
		{
			if ( fRect )
			{
				rectMsg |= rectChar ;
			}
			else
			{
				rectMsg = rectChar ;
				fRect = true ;
			}
		}
	}
	rect = rectMsg ;
	return	fRect ;
}

// フォントに太字が存在するか？
//////////////////////////////////////////////////////////////////////////////
static int CALLBACK EMessageSprite_EnumFontFamProc
	( ENUMLOGFONTEX *lpelfe,
		NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )
{
	DWORD *	pdwFlags = (DWORD*) lParam ;
	*pdwFlags |= (lpntme->ntmTm.ntmFlags & NTM_BOLD) ;
	return	1 ;
}

// スタイルを適用する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::UpdateCurrentStyle( void )
{
	EFormResourceManager::GetStaticTextStyle( m_tsMsgStyle, m_dscMsgStyle ) ;
	SakuraGL::SGLFont *		pFont = new SakuraGL::SGLFont ;
	SakuraGL::SGLFontStyle	style ;
	SSystem::SString		strFont ;
	m_fontMsg.LogFont() = m_tsMsgStyle.lfFont ;
	style.FromLogFont( m_tsMsgStyle.lfFont, strFont ) ;
	pFont->SetStyle( style ) ;
	m_rfiText.SetSGLFont( pFont ) ;
//	m_rfiText.SetFont( m_fontMsg.Create() ) ;
	m_rfiText.SetLineHeight( m_tsMsgStyle.nLineHeight ) ;
	m_rfiText.SetIndentWidth( m_tsMsgStyle.nIndent ) ;
	m_rfiText.SetVerticalWriting
		( m_tsMsgStyle.taAlign == EStaticTextSprite::taTop ) ;
	m_fontReading.LogFont() = m_tsMsgStyle.lfFont ;
	m_fontReading.SetSize( m_fontMsg.GetSize() * 2 / 5 ) ;
//	m_fontReading.Create( ) ;
	//
	m_fFontBold = false ;
	if ( m_fontMsg.GetWeight() > FW_NORMAL )
	{
		DWORD	dwFlags = 0 ;
		HDC		hdc = ::CreateCompatibleDC( NULL ) ;
		LOGFONT	lfEnum ;
		::eslFillMemory( &lfEnum, 0, sizeof(lfEnum) ) ;
		lfEnum.lfCharSet = DEFAULT_CHARSET ;
		::eslMoveMemory
			( lfEnum.lfFaceName, m_fontMsg.LogFont().lfFaceName, LF_FACESIZE ) ;
		::EnumFontFamiliesEx
			( hdc, &lfEnum,
				(FONTENUMPROCA) &EMessageSprite_EnumFontFamProc,
											(LPARAM) &dwFlags, 0 ) ;
		::DeleteDC( hdc ) ;
		m_fFontBold = (dwFlags == 0) ;
	}
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSMessageSprite::GetTypeName( void ) const
{
	return	L"MessageSprite" ;
}

ECSObject * ECSMessageSprite::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"MessageSprite" ) )
	{
		return	this ;
	}
	return	ECSSprite::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSMessageSprite::Duplicate( void )
{
	ECSMessageSprite *	pSprite = new ECSMessageSprite ;
	if ( GetInfo() != NULL )
	{
		pSprite->DuplicateImage( GetInfo(), 0 ) ;
	}
	PARAMETER	param ;
	GetParameter( param ) ;
	pSprite->SetParameter( param ) ;
	return	pSprite ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		nIndex += ECSResource::m_staFuncName->GetSize()
				+ ECSSprite::m_staFuncName->GetSize() ;
		return	eslErrSuccess ;
	}
	if ( ECSSprite::GetFunction( context, nIndex, pwszName ) )
	{
		return	ESLErrorMsg
			( "MessageSprite 型の定義されていない"
					"メンバ関数を呼び出そうとしています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	const int	nBase =
		ECSResource::m_staFuncName->GetSize()
			+ ECSSprite::m_staFuncName->GetSize() ;
	if ( nIndex >= nBase )
	{
		nIndex -= nBase ;
		if ( nIndex < (int) m_staFuncName->GetSize() )
		{
			return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
		}
		return	ESLErrorMsg
			( "MessageSprite 型の定義されていない"
					"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	ECSSprite::CallFunction( context, nIndex, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSMessageSprite::IndexAllMember( void )
{
	ECSSprite::IndexAllMember( ) ;
	//
	m_refRsrcManager.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::CommitAllReference( ECSContext & context )
{
	ECSSprite::CommitAllReference( context ) ;
	//
	// リソース設定復元
	//
	m_refRsrcManager.CommitAllReference( context ) ;
	m_pfrmStyle = ESLTypeCast<EFormResourceManager>( m_refRsrcManager.m_pRef ) ;
	//
	if ( (m_fdwFormatImage != (DWORD) -1)
			&& m_dwWidthImage && m_dwHeightImage && m_pfrmStyle )
	{
		EDescription	dscCurStyle = m_dscMsgStyle ;
		ECSWideString	wstrLog = m_wstrTextLog ;
		EGL_RECT		rctMsgRect = m_rfiText.GetViewRect( ) ;
		CreateMessage
			( m_dwWidthImage, m_dwHeightImage,
					&rctMsgRect, m_fMessageInSize ) ;
		if ( m_pfrmStyle != NULL )
		{
			AttachMessageStyle( m_pfrmStyle, EWideString(m_wstrDefStyle) ) ;
		}
		m_dscMsgStyle = dscCurStyle ;
		UpdateCurrentStyle( ) ;
		//
		ClearMessage( ) ;
		OutputMessage( wstrLog ) ;
		FlushMessage( ) ;
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Save( ESLFileObject & file, ECSContext & context )
{
	//
	// スプライトの基本設定保存
	//
	ESLError	err ;
	err = ECSSprite::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッセージ保存
	//
	DWORD	dwLength ;
	err = m_refRsrcManager.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	dwLength = m_wstrDefStyle.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	file.Write( m_wstrDefStyle.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	//
	EStreamFileBuffer	fbuf ;
	m_dscMsgStyle.WriteDescription( fbuf ) ;
	EPtrBuffer	ptrbuf = fbuf.GetBuffer( ) ;
	dwLength = ptrbuf.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	file.Write( ptrbuf, ptrbuf.GetLength() ) ;
	fbuf.Release( ptrbuf.GetLength() ) ;
	//
	file.Write( &m_nDefCharSpeed, sizeof(m_nDefCharSpeed) ) ;
	file.Write( &m_nCharSpeed, sizeof(m_nCharSpeed) ) ;
	file.Write( &m_nDefFadeSpeed, sizeof(m_nDefFadeSpeed) ) ;
	file.Write( &m_nFadeSpeed, sizeof(m_nFadeSpeed) ) ;
	//
	EGL_RECT	rctMsgRect = m_rfiText.GetViewRect() ;
	int	nFontBorder = m_fFontBorder ;
	int	nMessageInSize = m_fMessageInSize ;
	file.Write( &nFontBorder, sizeof(nFontBorder) ) ;
	file.Write( &nMessageInSize, sizeof(nMessageInSize) ) ;
	file.Write( &rctMsgRect, sizeof(rctMsgRect) ) ;
	file.Write( &m_rMsgEffectX, sizeof(m_rMsgEffectX) ) ;
	file.Write( &m_rMsgEffectY, sizeof(m_rMsgEffectY) ) ;
	file.Write( &m_rMsgEffectHorz, sizeof(m_rMsgEffectHorz) ) ;
	file.Write( &m_rMsgEffectVert, sizeof(m_rMsgEffectVert) ) ;
	file.Write( &m_rMsgEffectRev, sizeof(m_rMsgEffectRev) ) ;
	//
	dwLength = m_wstrTextLog.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	file.Write( m_wstrTextLog.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Load( ESLFileObject & file, ECSContext & context )
{
	//
	// スプライトの基本設定読み込み
	//
	ESLError	err ;
	err = ECSSprite::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メッセージ読み込み
	//
	DWORD	dwLength ;
	err = m_refRsrcManager.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &dwLength, sizeof(DWORD) ) ;
	file.Read( m_wstrDefStyle.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
	m_wstrDefStyle.ReleaseBuffer( dwLength ) ;
	//
	EStreamBuffer	buf ;
	file.Read( &dwLength, sizeof(DWORD) ) ;
	file.Read( buf.PutBuffer(dwLength), dwLength ) ;
	buf.Flush( dwLength ) ;
	m_dscMsgStyle.ReadDescription( buf ) ;
	//
	file.Read( &m_nDefCharSpeed, sizeof(m_nDefCharSpeed) ) ;
	file.Read( &m_nCharSpeed, sizeof(m_nCharSpeed) ) ;
	file.Read( &m_nDefFadeSpeed, sizeof(m_nDefFadeSpeed) ) ;
	file.Read( &m_nFadeSpeed, sizeof(m_nFadeSpeed) ) ;
	//
	int			nFontBorder, nMessageInSize ;
	EGL_RECT	rctMsgRect ;
	file.Read( &nFontBorder, sizeof(nFontBorder) ) ;
	file.Read( &nMessageInSize, sizeof(nMessageInSize) ) ;
	file.Read( &rctMsgRect, sizeof(rctMsgRect) ) ;
	file.Read( &m_rMsgEffectX, sizeof(m_rMsgEffectX) ) ;
	file.Read( &m_rMsgEffectY, sizeof(m_rMsgEffectY) ) ;
	file.Read( &m_rMsgEffectHorz, sizeof(m_rMsgEffectHorz) ) ;
	file.Read( &m_rMsgEffectVert, sizeof(m_rMsgEffectVert) ) ;
	file.Read( &m_rMsgEffectRev, sizeof(m_rMsgEffectRev) ) ;
	m_fFontBorder = (nFontBorder != 0) ;
	m_fMessageInSize = (nMessageInSize != 0) ;
	m_rfiText.SetViewRect( rctMsgRect ) ;
	//
	file.Read( &dwLength, sizeof(DWORD) ) ;
	file.Read( m_wstrTextLog.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
	m_wstrTextLog.ReleaseBuffer( dwLength ) ;
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = m_wstrTextLog ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSMessageSprite::m_staFuncName = NULL ;
const wchar_t *		ECSMessageSprite::m_pwszFuncName[18] =
{
	L"CreateMessage", L"OutputMessage", L"FlushMessage",
	L"ClearMessage", L"IsMessagePending", L"AttachMessageStyle",
	L"SetDefaultMsgSpeed", L"SetMessageEffect",
	L"SetShadowTransparency", L"SetFontBordering",
	L"SetFontStyle", L"SetFontFace", L"SetFontColor",
	L"GetCursorPos", L"MoveCursorPos", L"GetCharacterCount",
	L"GetMessageRect", NULL
} ;
const ECSMessageSprite::PFUNC_CALL	ECSMessageSprite::m_pfnCallFunc[17] =
{
	&ECSMessageSprite::Call_CreateMessage,
	&ECSMessageSprite::Call_OutputMessage,
	&ECSMessageSprite::Call_FlushMessage,
	&ECSMessageSprite::Call_ClearMessage,
	&ECSMessageSprite::Call_IsMessagePending,
	&ECSMessageSprite::Call_AttachMessageStyle,
	&ECSMessageSprite::Call_SetDefaultMsgSpeed,
	&ECSMessageSprite::Call_SetMessageEffect,
	&ECSMessageSprite::Call_SetShadowTransparency,
	&ECSMessageSprite::Call_SetFontBordering,
	&ECSMessageSprite::Call_SetFontStyle,
	&ECSMessageSprite::Call_SetFontFace,
	&ECSMessageSprite::Call_SetFontColor,
	&ECSMessageSprite::Call_GetCursorPos,
	&ECSMessageSprite::Call_MoveCursorPos,
	&ECSMessageSprite::Call_GetCharacterCount,
	&ECSMessageSprite::Call_GetMessageRect,
} ;

// メンバ関数 : Integer CreateMessage
//    ( Integer nWidth, Integer nHeight [,Rect rctMsgView [, Integer fMsgInSize]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_CreateMessage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 5 ) ;
	if ( err )
		return	err ;
	//
	int		nWidth, nHeight, fInSize ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 2, 1 ) ;
	if ( err )
		return	err ;
	//
	EGL_RECT *	pMsgView = NULL ;
	EGL_RECT	rctMsgView ;
	ECSStructureInterface *	pRect =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 3, L"Rect" ) ) ;
	if ( pRect != NULL )
	{
		rctMsgView.left = pRect->GetMemberAsInt( L"left", 0 ) ;
		rctMsgView.top = pRect->GetMemberAsInt( L"top", 0 ) ;
		rctMsgView.right = pRect->GetMemberAsInt( L"right", nWidth - 1 ) ;
		rctMsgView.bottom = pRect->GetMemberAsInt( L"bottom", nHeight - 1 ) ;
		pMsgView = &rctMsgView ;
	}
	err = context.GetArgumentAsInt( fInSize, lstArg, 4, 0 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	err = CreateMessage( nWidth, nHeight, pMsgView, (fInSize != 0) ) ;
	QuickUnlock( ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer OutputMessage( String strMsg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_OutputMessage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrMsg ;
	err = context.GetArgumentAsStr( wstrMsg, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	int	nOutCount ;
	QuickLock( ) ;
	nOutCount = OutputMessage( wstrMsg ) ;
	QuickUnlock( ) ;
	return	context.PushObject( context.new_CSInteger( nOutCount ) ) ;
}

// メンバ関数 : Integer FlushMessage()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_FlushMessage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = FlushMessage( ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer ClearMessage()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_ClearMessage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = ClearMessage( ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsMessagePending()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_IsMessagePending
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	bool	fPending ;
	fPending = IsMessagePending( ) ;
	return	context.PushObject( new ECSInteger( fPending ? -1 : 0 ) ) ;
}

// メンバ関数 :
//	Integer AttachMessageStyle( Reference refStyle, String strDefaultStyle )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_AttachMessageStyle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSResourceManager *	prmManager =
		ESLTypeCast<ECSResourceManager>
			( context.GetArgumentObjectAs( lstArg, 1, L"ResourceManager" ) ) ;
	if ( prmManager == NULL )
	{
		return	ESLErrorMsg
			( "引数に ResourceManager オブジェクトが指定されていません。" ) ;
	}
	ECSWideString	wstrDefStyle ;
	err = context.GetArgumentAsStr( wstrDefStyle, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	m_refRsrcManager.SetReference( prmManager, &context ) ;
	Lock( ) ;
	err = AttachMessageStyle( prmManager, wstrDefStyle ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 :
//	Integer SetDefaultMsgSpeed
//		( Integer nCharSpeed, Integer nFadeSpeed, Integer nSpeedRatio )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetDefaultMsgSpeed
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	int		nCharSpeed, nFadeSpeed, nSpeedRatio ;
	err = context.GetArgumentAsInt( nCharSpeed, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFadeSpeed, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nSpeedRatio, lstArg, 3, 0x100 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = SetDefaultMsgSpeed( nCharSpeed, nFadeSpeed, nSpeedRatio ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 :
//		Integer SetMessageEffect
//			( Real x, Real y, Real mx, Real my, Real rev )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetMessageEffect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 6 ) ;
	if ( err )
		return	err ;
	//
	double	x, y, mx, my, r ;
	err = context.GetArgumentAsReal( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( mx, lstArg, 3, 1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( my, lstArg, 4, 1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( r, lstArg, 5, 0 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = SetMessageEffect( x, y, mx, my, r ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetShadowTransparency( Integer nTransparency )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetShadowTransparency
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nTransparency ;
	err = context.GetArgumentAsInt( nTransparency, lstArg, 1, 0x100 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = SetShadowTransparency( nTransparency ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetFontBordering( Integer fBordering )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetFontBordering
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fBordering ;
	err = context.GetArgumentAsInt( fBordering, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = SetFontBordering( (fBordering != 0) ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetFontStyle( String strMsgStyle )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetFontStyle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrMsgStyle ;
	err = context.GetArgumentAsStr( wstrMsgStyle, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	err = SetFontStyle( wstrMsgStyle ) ;
	Unlock( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetFontFace( String sFontFace )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetFontFace
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFontFace ;
	err = context.GetArgumentAsStr( wstrFontFace, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	Lock() ;
	err = SetFontFaceName( wstrFontFace ) ;
	Unlock() ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetFontColor
//					( Integer rgbColor, Integer rgbShadow := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_SetFontColor
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int	rgbColor, rgbShadow ;
	err = context.GetArgumentAsInt( rgbColor, lstArg, 1, 0xFFFFFFFF ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( rgbShadow, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	Lock() ;
	err = SetFontColor
		( EGLPalette( (DWORD) rgbColor ), true,
			EGLPalette( (DWORD) rgbShadow ), true ) ;
	Unlock() ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Point GetCursorPos()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_GetCursorPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	EGL_POINT	ptCursor = GetCursorPos( ) ;
	ECSStructureInterface *	pPoint = context.CreateUserStructure( L"Point" ) ;
	pPoint->SetMemberAsInt( L"x", ptCursor.x ) ;
	pPoint->SetMemberAsInt( L"y", ptCursor.y ) ;
	//
	return	context.PushObject( *pPoint ) ;
}

// メンバ関数 : MoveCursorPos( Integer x, Integer y )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_MoveCursorPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		x, y ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	MoveCursorPos( EGLPoint( x, y ) ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : GetCharacterCount()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_GetCharacterCount
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( context.new_CSInteger( m_lstMsgChar.GetSize() ) ) ;
}

// メンバ関数 : Rect GetMessageRect()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMessageSprite::Call_GetMessageRect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	EGLRect	rect( 0, 0, -1, -1 ) ;
	Lock() ;
	GetMessageRect( rect ) ;
	Unlock() ;
	//
	ECSStructureInterface *	pRectObj = context.CreateUserStructure( L"Rect" ) ;
	pRectObj->SetMemberAsInt( L"left", rect.left ) ;
	pRectObj->SetMemberAsInt( L"top", rect.top ) ;
	pRectObj->SetMemberAsInt( L"right", rect.right ) ;
	pRectObj->SetMemberAsInt( L"bottom", rect.bottom ) ;
	//
	return	context.PushObject( *pRectObj ) ;
}

