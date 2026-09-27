
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_text.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// テキスト表示スプライト・スタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteText::TextStyle::TextStyle( void )
{
	boxAlign = 0 ;
	//
	font.nStyles = 0 ;
	font.nSize = 16 ;
	font.pszFace = SGLFontStyle::StandardFont ;
	//
	decoration.rgbaBody.ui32 = 0xFFFFFFFF ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteText::TextStyle::TextStyle( const SGLSpriteText::TextStyle& style )
{
	eslCopyMemory( this, &style, sizeof(TextStyle) ) ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteText::TextStyle&
	SGLSpriteText::TextStyle::operator =
		( const SGLSpriteText::TextStyle& style )
{
	eslCopyMemory( this, &style, sizeof(TextStyle) ) ;
	return	*this ;
}

// シリアライズ（ポインタを除く）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteText::TextStyle::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	file.Write( &boxAlign, sizeof(uint32_t) ) ;
	font.SaveWithoutPointer( file ) ;
	context.SaveWithoutPointer( file ) ;
	file.Write( &decoration, sizeof(SGLLetterer::Decoration) ) ;
	return	sglErrSuccess ;
}

SGLError SGLSpriteText::TextStyle::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	file.Read( &boxAlign, sizeof(uint32_t) ) ;
	font.LoadWithoutPointer( file ) ;
	context.LoadWithoutPointer( file ) ;
	file.Read( &decoration, sizeof(SGLLetterer::Decoration) ) ;
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// テキスト表示スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteText, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteText::SGLSpriteText( void )
{
	m_flagsUI |= uiUnclickable ;
}

SGLSpriteText::SGLSpriteText( const SGLSpriteText& src )
	: SGLSprite( src ), m_strText( src.m_strText )
{
	m_flagsUI |= uiUnclickable ;
	SetTextStyle( src.m_styleText ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteText::~SGLSpriteText( void )
{
	DetachSyncTimeout( 100 ) ;
}

// テキストスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::SetTextStyle( const SGLSpriteText::TextStyle& style )
{
	Lock() ;
	m_styleText = style ;
	m_strFontFace = style.font.pszFace ;
	m_styleText.font.pszFace = m_strFontFace ;
	//
	UpdateTextImage() ;
	Unlock() ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteText::GetRectangle( SGLRect& rectExt ) const
{
/*
	SGLRect	rectText = m_styleText.context.rectWritable ;
	if ( LocalToGlobalRect( rectText ) )
	{
		if ( SGLSprite::GetRectangle( rectExt ) )
		{
			return	true ;
		}
		else
		{
			rectExt = rectText ;
			return	true ;
		}
	}
	else
*/
	{
		return	SGLSprite::GetRectangle( rectExt ) ;
	}
}

// 文書表示領域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteText::GetTextRectangle( SGLRect& rectExt ) const
{
	rectExt = m_styleText.context.rectWritable ;
	return	LocalToGlobalRect( rectExt ) ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteText::GetText( void ) const
{
	Lock() ;
	SString	strText = m_strText ;
	Unlock() ;
	return	strText ;
}

void SGLSpriteText::SetText( const wchar_t * pwszText )
{
	Lock() ;
	m_strText = pwszText ;
	Unlock() ;

	UpdateTextImage() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::SetTextFont( const wchar_t * pwszFont, int nSize )
{
	Lock() ;
	if ( nSize != 0 )
	{
		m_styleText.font.nSize = nSize ;
	}
	m_strFontFace = pwszFont ;
	SelectValidFont( m_strFontFace ) ;
	m_styleText.font.pszFace = m_strFontFace ;
	Unlock() ;

	UpdateTextImage() ;
}

// 文字画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::UpdateTextImage( void )
{
	Lock() ;
	SString		strText =m_strText ;
	TextStyle	style = m_styleText ;
	Unlock() ;
	//
	if ( !strText.IsEmpty() )
	{
		SGLImageObject *	pTextImage =
			CreateTextImage( m_ptTextUpperLeft, m_styleText, strText ) ;
		//
		Lock() ;
		AttachImage( NULL ) ;
		//
		m_pTextImage = pTextImage ;
		if ( m_pTextImage != NULL )
		{
			AttachImage( m_pTextImage ) ;
			m_paramView.vCenter.x = - m_ptTextUpperLeft.x ;
			m_paramView.vCenter.y = - m_ptTextUpperLeft.y ;
			//
			SGLSize	sizeText = m_pTextImage->GetImageSize() ;
			switch ( m_styleText.boxAlign & alignBoxHorzMask )
			{
			case	alignBoxRight:
				m_paramView.vCenter.x -=
					(m_styleText.context.rectWritable.GetWidth() - sizeText.w) ;
				break ;
			case	alignBoxCenter:
				m_paramView.vCenter.x -=
					(m_styleText.context.rectWritable.GetWidth() - sizeText.w) * 0.5 ;
				break ;
			}
			switch ( m_styleText.boxAlign & alignBoxVertMask )
			{
			case	alignBoxBottom:
				m_paramView.vCenter.y -=
					(m_styleText.context.rectWritable.GetHeight() - sizeText.h) ;
				break ;
			case	alignBoxVCenter:
				m_paramView.vCenter.y -=
					(m_styleText.context.rectWritable.GetHeight() - sizeText.h) * 0.5 ;
				break ;
			}
			//
			NotifyUpdate() ;
		}
		Unlock() ;
	}
	else
	{
		AttachImage( NULL ) ;
	}
}

// 画像化したテキストを生成する
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteText::CreateTextImage
	( SGLPoint& ptUpperLeft,
		const SGLSpriteText::TextStyle& style, const wchar_t * pwszText )
{
	SGLFont	font ;
	font.SetStyle( style.font ) ;
	//
	SGLLetteringContext	context = style.context ;
	context.ptStartWriting.x = context.rectWritable.left ;
	context.ptStartWriting.y = context.rectWritable.top ;
	//
	SGLLetterer	letterer ;
	letterer.WriteLetter( font, context, pwszText ) ;
	if ( letterer.CombineLetter() )
	{
		return	NULL ;
	}
	letterer.DecorateLetter( style.decoration ) ;
	//
	SGLLetterer::Character *	pChar = letterer.GetCharacterAt(0) ;
	if ( (pChar == NULL) || (pChar->pImage == NULL) )
	{
		return	NULL ;
	}
	ptUpperLeft = pChar->ptWriting ;
	ptUpperLeft += pChar->ptOffset ;
	//
	SGLImage *			pImage = new SGLImage ;
	SGLImageBuffer *	pTextImage = pChar->pImage ;
	// ※OpenGL で直接描画する場合、条件によっては
	// 　右辺・下辺が欠ける場合があるため１ピクセル余白を付けておく
	pImage->CreateImage
		( pTextImage->width + 1, pTextImage->height + 1,
				pTextImage->format, pTextImage->depth ) ;
	//
	SGLImageBuffer	imgbuf ;
	imgbuf.ptrBuffer = pImage->LockBuffer( imgbuf ) ;
	sglCopyImageBuffer( imgbuf, *pTextImage ) ;
	pImage->UnlockBuffer() ;
	//
	return	pImage ;
}

// テキストスタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::ParseTextStyle
	( SGLSpriteText::TextStyle& style, SString& strFontFace,
						const SSystem::SXMLDocument& xmlStyle )
{
	// <arrange>
	SXMLDocument *	pxmlArrange = xmlStyle.GetElementTagAs( L"arrange" ) ;
	if ( pxmlArrange != NULL )
	{
		SString *	pstrBoxAlign = pxmlArrange->GetAttributeAs( L"box_align" ) ;
		if ( pstrBoxAlign != NULL )
		{
			static const wchar_t *	pwszBoxAligns[] =
			{
				L"left", L"right", L"center",
				L"top", L"bottom", L"vcenter",
				NULL,
			} ;
			static const uint32_t	nBoxAlignFlags[] =
			{
				alignBoxLeft, alignBoxRight, alignBoxCenter,
				alignBoxTop, alignBoxBottom, alignBoxVCenter,
			} ;
			SStringParser	sparsBoxAlign ;
			SString			strAlign ;
			sparsBoxAlign.AttachString( *pstrBoxAlign ) ;
			style.boxAlign = 0 ;
			while ( sparsBoxAlign.PassSpace() )
			{
				if ( sparsBoxAlign.NextString( strAlign ) )
				{
					for ( size_t i = 0; pwszBoxAligns[i]; i ++ )
					{
						if ( strAlign == pwszBoxAligns[i] )
						{
							style.boxAlign |= nBoxAlignFlags[i] ;
						}
					}
				}
			}
		}
		SString *	pstrAlign = pxmlArrange->GetAttributeAs( L"align" ) ;
		if ( pstrAlign != NULL )
		{
			if ( *pstrAlign == L"left" )
			{
				style.context.typeAlignment = SGLLetteringContext::alignLeft ;
				style.context.flagVertical = SGLLetteringContext::writingHorizontal ;
			}
			else if ( *pstrAlign == L"right" )
			{
				style.context.typeAlignment = SGLLetteringContext::alignRight ;
				style.context.flagVertical = SGLLetteringContext::writingHorizontal ;
			}
			else if ( *pstrAlign == L"center" )
			{
				style.context.typeAlignment = SGLLetteringContext::alignCenter ;
				style.context.flagVertical = SGLLetteringContext::writingHorizontal ;
			}
			else if ( *pstrAlign == L"accordance" )
			{
				style.context.typeAlignment = SGLLetteringContext::alignLong ;
				style.context.flagVertical = SGLLetteringContext::writingHorizontal ;
			}
			else if ( *pstrAlign == L"top" )
			{
				style.context.typeAlignment = SGLLetteringContext::alignLeft ;
				style.context.flagVertical = SGLLetteringContext::writingVertical ;
			}
		}
		style.context.pitchLine =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"line_height", style.context.pitchLine ) ;
		style.context.widthIndent =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"indent", style.context.widthIndent ) ;
		style.context.pitchChar =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"pitch", style.context.pitchChar ) ;
		style.context.pitchTab =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"tab_pitch", style.context.pitchTab ) ;
		style.context.rectWritable.left =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"left", style.context.rectWritable.left ) ;
		style.context.rectWritable.top =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
						( L"top", style.context.rectWritable.top ) ;
		style.context.rectWritable.SetWidth
			( (int) pxmlArrange->GetAttrRichIntegerAs
						( L"width", style.context.rectWritable.GetWidth() ) ) ;
		style.context.rectWritable.SetHeight
			( (int) pxmlArrange->GetAttrRichIntegerAs
						( L"height", style.context.rectWritable.GetHeight() ) ) ;
	}
	// <font>
	SXMLDocument *	pxmlFont = xmlStyle.GetElementTagAs( L"font" ) ;
	if ( pxmlFont != NULL )
	{
		ParseFontStyle( style.font, strFontFace, *pxmlFont ) ;
	}
	// <text>, <shadow>, <border>
	ParseTextDecoration( style.decoration, xmlStyle ) ;
}

// フォントスタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::ParseFontStyle
	( SGLFontStyle& style, SSystem::SString& strFontFace,
					const SSystem::SXMLDocument& xmlFont )
{
	strFontFace = xmlFont.GetAttrStringAs( L"face", style.pszFace ) ;
	SelectValidFont( strFontFace ) ;
	style.pszFace = strFontFace ;
	//
	style.nSize =
		(uint32_t) xmlFont.GetAttrRichIntegerAs( L"size", style.nSize ) ;
	//
	const wchar_t *	pwszDefItalic = L"false" ;
	const wchar_t *	pwszDefBold = L"false" ;
	if ( style.nStyles & SGLFontStyle::styleItalic )
	{
		pwszDefItalic = L"true" ;
	}
	if ( style.nStyles & SGLFontStyle::styleBold )
	{
		pwszDefBold = L"true" ;
	}
	style.nStyles &=
			~(SGLFontStyle::styleItalic | SGLFontStyle::styleBold) ;
	if ( xmlFont.GetAttrStringAs( L"bold", pwszDefBold ) == L"true" )
	{
		style.nStyles |= SGLFontStyle::styleBold ;
	}
	if ( xmlFont.GetAttrStringAs( L"italic", pwszDefItalic ) == L"true" )
	{
		style.nStyles |= SGLFontStyle::styleItalic ;
	}
}

// 文字装飾スタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::ParseTextDecoration
	( SGLLetterer::Decoration& deco, const SSystem::SXMLDocument& xmlStyle )
{
	// <text>
	SXMLDocument *	pxmlText = xmlStyle.GetElementTagAs( L"text" ) ;
	if ( pxmlText != NULL )
	{
		ParseTextColor( deco.rgbaBody, *pxmlText ) ;
	}
	// <shadow>
	SXMLDocument *	pxmlShadow = xmlStyle.GetElementTagAs( L"shadow" ) ;
	if ( pxmlShadow != NULL )
	{
		ParseTextColor( deco.rgbaShadow, *pxmlShadow ) ;
		//
		deco.ptShadow.x =
			(int32_t) pxmlShadow->GetAttrRichIntegerAs
								( L"x", deco.ptShadow.x ) ;
		deco.ptShadow.y =
			(int32_t) pxmlShadow->GetAttrRichIntegerAs
								( L"y", deco.ptShadow.y ) ;
		//
		deco.nFlags &= ~SGLLetterer::flagShadow ;
		if ( deco.rgbaShadow.argb.Alpha != 0 )
		{
			deco.nFlags |= SGLLetterer::flagShadow ;
		}
	}
	// <border>
	SXMLDocument *	pxmlBorder = xmlStyle.GetElementTagAs( L"border" ) ;
	if ( pxmlBorder != NULL )
	{
		ParseTextColor( deco.rgbaBorder, *pxmlBorder ) ;
		//
		deco.nFlags &= ~SGLLetterer::flagBorder ;
		deco.widthBorder = 0 ;
		if ( deco.rgbaBorder.argb.Alpha != 0 )
		{
			deco.nFlags |= SGLLetterer::flagBorder ;
			deco.widthBorder = 1 ;
		}
	}
}

// 文字色を解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::ParseTextColor
	( SGLPalette& rgbaText, const SSystem::SXMLDocument& xmlText )
{
	uint32_t	rgb = rgbaText.ui32 & 0x00FFFFFF ;
	rgbaText.ui32 &= 0xFF000000 ;
	rgbaText.ui32 |=
		(uint32_t) xmlText.GetAttrRichIntegerAs( L"color", rgb ) & 0x00FFFFFF ;
	//
	int64_t	nTransparency = (0xFF - rgbaText.argb.Alpha) * 0x100 / 0xFF ;
	nTransparency =
		xmlText.GetAttrRichIntegerAs( L"transparency", nTransparency ) ;
	if ( nTransparency < 0 )
	{
		nTransparency = 0 ;
	}
	else if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	rgbaText *= (unsigned int) (0x100 - nTransparency) ;
	rgbaText.argb.Alpha = (uint8_t) ((0x100 - nTransparency) * 0xFF / 0x100) ;
}

// セミコロンで区切られた複数のフォントから有効なフォントを選ぶ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteText::SelectValidFont( SSystem::SString& strFontList )
{
	ssize_t	iSep = strFontList.Find( L';' ) ;
	if ( iSep < 0 )
	{
		return ;
	}
	SObjectArray<SString>	listFonts ;
	SGLFont::EnumerateFonts( listFonts ) ;
	//
	SString	strTemp ;
	for ( ; ; )
	{
		strTemp = strFontList.Left( iSep ) ;
		if ( SGLFont::RemappedFontOf( strTemp ) != NULL )
		{
			strFontList = strTemp ;
			return ;
		}
		size_t	nCount = listFonts.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SString *	pstrFont = listFonts.GetAt( i ) ;
			if ( (pstrFont != NULL) && (*pstrFont == strTemp) )
			{
				strFontList = strTemp ;
				return ;
			}
		}
		strFontList = strFontList.Middle( iSep + 1 ) ;
		iSep = strFontList.Find( L';' ) ;
		if ( iSep < 0 )
		{
			return ;
		}
	}
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteText::GetLQClassName( void ) const
{
	return	L"EntisGLS4.TextSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteText::DuplicateObject( void )
{
	return	new SGLSpriteText( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteText::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	m_styleText.SaveWithoutPointer( file ) ;
	file.WriteString( m_strFontFace ) ;
	file.WriteString( m_strText ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteText::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	m_styleText.LoadWithoutPointer( file ) ;
	file.ReadString( m_strFontFace ) ;
	file.ReadString( m_strText ) ;
	m_styleText.context.pwszProhibition = SGLLetteringContext::pwszDefProhibition ;
	m_styleText.font.pszFace = m_strFontFace ;
	UpdateTextImage() ;
	return	sglErrSuccess ;
}

