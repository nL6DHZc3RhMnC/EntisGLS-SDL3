
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// メッセージ表示スプライト／リンク情報
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::LinkInfo::LinkInfo( void )
{
	m_status = statusNormal ;
	for ( int i = 0; i < statusCount; i ++ )
	{
		m_pImage[i] = NULL ;
	}
}

SGLSpriteMessage::LinkInfo::LinkInfo( const LinkInfo& li )
	: m_aHitRects( li.m_aHitRects ), m_strLinkURL( li.m_strLinkURL )
{
	m_status = statusNormal ;
	//
	for ( int i = 0; i < statusCount; i ++ )
	{
		m_pImage[i] = NULL ;
		if ( li.m_pImage[i] != NULL )
		{
			m_pImage[i] = li.m_pImage[i]->NewReference() ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::LinkInfo::~LinkInfo( void )
{
	for ( int i = 0; i < statusCount; i ++ )
	{
		if ( m_pImage[i] != NULL )
		{
			delete	m_pImage[i] ;
			m_pImage[i] = NULL ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// メッセージ表示スプライト／文字パーツ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::Character::Character( void )
{
	m_pLink = NULL ;
	m_pImage = NULL ;
	m_wchCode = 0 ;
	m_msecStart = 0 ;
}

SGLSpriteMessage::Character::Character( const SGLSpriteMessage::Character& chr )
{
	m_pLink = NULL ;
	if ( chr.m_pLink != NULL )
	{
		m_pLink = new LinkInfo( *(chr.m_pLink) ) ;
	}
	m_pImage = NULL ;
	if ( chr.m_pImage != NULL )
	{
		m_pImage = chr.m_pImage->NewReference() ;
	}
	m_ptWriting = chr.m_ptWriting ;
	m_ptOffset = chr.m_ptOffset ;
	m_sizeChar = chr.m_sizeChar ;
	m_wchCode = chr.m_wchCode ;
	m_msecStart = chr.m_msecStart ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::Character::~Character( void )
{
	delete	m_pLink ;
	m_pLink = NULL ;
	delete	m_pImage ;
	m_pImage = NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// メッセージ表示スプライト／表示アクションスタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::ViewActionStyle::ViewActionStyle( void )
{
	msecPerChar = 0 ;
	msecFade = 0 ;
	vMove.x = 0 ;
	vMove.y = 0 ;
	vZoom.x = 1 ;
	vZoom.y = 1 ;
	zRotation = 0 ;
}

SGLSpriteMessage::ViewActionStyle::ViewActionStyle( const SGLSpriteMessage::ViewActionStyle& src )
{
	msecPerChar = src.msecPerChar ;
	msecFade = src.msecFade ;
	vMove = src.vMove ;
	vZoom = src.vZoom ;
	zRotation = src.zRotation ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteMessage::ViewActionStyle&
	SGLSpriteMessage::ViewActionStyle::operator =
				( const SGLSpriteMessage::ViewActionStyle& src )
{
	msecPerChar = src.msecPerChar ;
	msecFade = src.msecFade ;
	vMove = src.vMove ;
	vZoom = src.vZoom ;
	zRotation = src.zRotation ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// スタイル
//////////////////////////////////////////////////////////////////////////////

// SGLSpriteMessage::RichTextStyle 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::RichTextStyle::RichTextStyle( void )
{
	SGLLetterer::Decoration&
		linkNorm = decoLink[SGLSpriteMessage::LinkInfo::statusNormal] ;
	linkNorm.rgbaBody = 0xFF0000FF ;
	//
	SGLLetterer::Decoration&
		linkFocus = decoLink[SGLSpriteMessage::LinkInfo::statusFocus] ;
	linkFocus.rgbaBody = 0xFFFF0000 ;
	//
	SGLLetterer::Decoration&
		linkPush = decoLink[SGLSpriteMessage::LinkInfo::statusPushing] ;
	linkPush.rgbaBody = 0xFFFFFF00 ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::RichTextStyle::RichTextStyle
		( const SGLSpriteMessage::RichTextStyle& style )
	: SGLSpriteText::TextStyle( style ),
		fontRuby( style.fontRuby )
{
	for ( int i = 0; i < SGLSpriteMessage::LinkInfo::statusCount; i ++ )
	{
		decoLink[i] = style.decoLink[i] ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteMessage::RichTextStyle&
	SGLSpriteMessage::RichTextStyle::operator =
		( const SGLSpriteMessage::RichTextStyle& style )
{
	SGLSpriteText::TextStyle::operator = ( style ) ;
	fontRuby = style.fontRuby ;
	for ( int i = 0; i < SGLSpriteMessage::LinkInfo::statusCount; i ++ )
	{
		decoLink[i] = style.decoLink[i] ;
	}
	return	*this ;
}

// シリアライズ（ポインタを除く）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::RichTextStyle::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	SGLSpriteText::TextStyle::SaveWithoutPointer( file ) ;
	fontRuby.SaveWithoutPointer( file ) ;
	file.Write( &decoLink[0], sizeof(decoLink) ) ;
	return	sglErrSuccess ;
}

SGLError SGLSpriteMessage::RichTextStyle::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	SGLSpriteText::TextStyle::LoadWithoutPointer( file ) ;
	fontRuby.LoadWithoutPointer( file ) ;
	file.Read( &decoLink[0], sizeof(decoLink) ) ;
	return	sglErrSuccess ;
}



// SGLSpriteMessage::MessageStyle 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::MessageStyle::MessageStyle( void )
{
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::MessageStyle::MessageStyle( const SGLSpriteMessage::MessageStyle& style )
	: RichTextStyle( style ),
		viewAction( style.viewAction )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteMessage::MessageStyle&
	SGLSpriteMessage::MessageStyle::operator =
		( const SGLSpriteMessage::MessageStyle& style )
{
	RichTextStyle::operator = ( style ) ;
	viewAction = style.viewAction ;
	return	*this ;
}

// シリアライズ（ポインタを除く）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::MessageStyle::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	RichTextStyle::SaveWithoutPointer( file ) ;
	file.Write( &viewAction, sizeof(ViewActionStyle) ) ;
	return	sglErrSuccess ;
}

SGLError SGLSpriteMessage::MessageStyle::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	RichTextStyle::LoadWithoutPointer( file ) ;
	file.Read( &viewAction, sizeof(ViewActionStyle) ) ;
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 文字画像アトラス
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::Atlas::Atlas( void )
	: m_sizeAtlas( 0, 0 ), m_xNext( 0 ), m_yLine( 0 ), m_hLine( 0 )
{
}

// アトラス画像サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::Atlas::SetAtlasSize( uint32_t nWidth, uint32_t nHeight )
{
	if ( (m_sizeAtlas.w != (int32_t) nWidth)
		|| (m_sizeAtlas.h != (int32_t) nHeight) )
	{
		m_imgAtlas.CreateImage( nWidth, nHeight ) ;
		m_sizeAtlas.w = (int32_t) nWidth ;
		m_sizeAtlas.h = (int32_t) nHeight ;
		//
		ClearAll() ;
	}
}

// 画像クリア
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::Atlas::ClearAll( void )
{
	m_imgAtlas.FillImage( SGLPalette(0) ) ;
	//
	m_xNext = 0 ;
	m_yLine = 0 ;
	m_hLine = 0 ;
}

// 文字画像追加
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteMessage::Atlas::Allocate( SGLImageObject& imgChar )
{
	SGLSize	sizeChar = imgChar.GetImageSize() ;
	sizeChar.w += 2 ;
	sizeChar.h += 2 ;
	//
	if ( m_xNext + (size_t) sizeChar.w > (size_t) m_sizeAtlas.w )
	{
		if ( sizeChar.w > m_sizeAtlas.w )
		{
			return	NULL ;
		}
		m_xNext = 0 ;
		m_yLine += m_hLine ;
		m_hLine = 0 ;
	}
	if ( m_yLine + (size_t) sizeChar.h > (size_t) m_sizeAtlas.h )
	{
		return	NULL ;
	}
	if ( m_hLine < (size_t) sizeChar.h )
	{
		m_hLine = (size_t) sizeChar.h ;
	}
	SGLImageRect	rectChar ;
	rectChar.x = (int32_t) m_xNext + 1 ;
	rectChar.y = (int32_t) m_yLine + 1 ;
	rectChar.w = sizeChar.w - 2 ;
	rectChar.h = sizeChar.h - 2 ;
	//
	m_imgAtlas.CopyImage( &imgChar, rectChar.x, rectChar.y ) ;
	//
	m_xNext += (size_t) sizeChar.w ;
	//
	return	m_imgAtlas.NewReference( &rectChar, 0 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// メッセージ表示スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMessage, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::SGLSpriteMessage( void )
{
	m_flagRectChars = false ;
	m_msecDuration = 0 ;
	m_msecLastTiming = 0 ;
	m_msecFading = 0 ;
	m_fxSpeedRatio = 0x100 ;
	//
	m_pFocusLink = NULL ;
	//
	m_nPausedMsg = 0 ;
}

SGLSpriteMessage::SGLSpriteMessage( const SGLSpriteMessage& src )
	: m_characters( src.m_characters ),
		m_flagRectChars( src.m_flagRectChars ),
		m_rectChars( src.m_rectChars ),
		m_msecDuration( src.m_msecDuration ),
		m_msecLastTiming( src.m_msecLastTiming ),
		m_msecFading( src.m_msecFading ),
		m_fxSpeedRatio( src.m_fxSpeedRatio ),
		m_styleMsg( src.m_styleMsg ),
		m_refSkin( src.m_refSkin ),
		m_lcLettering( src.m_lcLettering ),
		m_ltDecoration( src.m_ltDecoration ),
		m_fsFont( src.m_fsFont ),
		m_xmlHistory( src.m_xmlHistory )
{
	m_strFontFace = m_styleMsg.font.pszFace ;
	m_strProhibition = m_styleMsg.context.pwszProhibition ;
	m_strFontCur = m_fsFont.pszFace ;
	m_styleMsg.font.pszFace = m_strFontFace ;
	m_styleMsg.context.pwszProhibition = m_strProhibition ;
	m_fsFont.pszFace = m_strFontCur ;
	m_styleMsg.fontRuby.pszFace = m_strFontFace ;
	m_font.SetStyle( m_fsFont ) ;
	//
	m_pFocusLink = NULL ;
	//
	m_nPausedMsg = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::~SGLSpriteMessage( void )
{
	DetachSyncTimeout( 100 ) ;
}

// テキストスタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteText::TextStyle& SGLSpriteMessage::GetTextStyle( void ) const
{
	return	m_styleMsg ;
}

const SGLSpriteMessage::RichTextStyle& SGLSpriteMessage::GetRichTextStyle( void ) const
{
	return	m_styleMsg ;
}

const SGLSpriteMessage::MessageStyle& SGLSpriteMessage::GetMessageStyle( void ) const
{
	return	m_styleMsg ;
}

// テキストスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetTextStyle( const SGLSpriteText::TextStyle& style )
{
	*((SGLSpriteText::TextStyle*)&m_styleMsg) = style ;
	m_strFontFace = style.font.pszFace ;
	m_strProhibition = m_styleMsg.context.pwszProhibition ;
	m_styleMsg.font.pszFace = m_strFontFace ;
	m_styleMsg.context.pwszProhibition = m_strProhibition ;
	m_lcLettering = m_styleMsg.context ;
	m_ltDecoration = m_styleMsg.decoration ;
	m_fsFont = m_styleMsg.font ;
	m_styleMsg.fontRuby = m_styleMsg.font ;
	m_styleMsg.fontRuby.nSize = m_styleMsg.fontRuby.nSize * 2 / 5 ;
	m_font.SetStyle( m_styleMsg.font ) ;
}

void SGLSpriteMessage::SetRichTextStyle( const SGLSpriteMessage::RichTextStyle& style )
{
	SetTextStyle( style ) ;
	SetRubyFontStyle( style.fontRuby ) ;
	//
	for ( int i = 0; i < SGLSpriteMessage::LinkInfo::statusCount; i ++ )
	{
		m_styleMsg.decoLink[i] = style.decoLink[i] ;
	}
}

void SGLSpriteMessage::SetMessageStyle( const SGLSpriteMessage::MessageStyle& style )
{
	SetRichTextStyle( style ) ;
	SetViewActionStyle( style.viewAction ) ;
}

// ルビ表示フォントスタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLFontStyle& SGLSpriteMessage::GetRubyFontStyle( void ) const
{
	return	m_styleMsg.fontRuby ;
}

// ルビ表示フォントスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetRubyFontStyle( const SGLFontStyle& style )
{
	m_styleMsg.fontRuby = style ;
	//
	m_strRubyFont = style.pszFace ;
	m_styleMsg.fontRuby.pszFace = m_strRubyFont ;
}

// 表示スタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteMessage::ViewActionStyle & SGLSpriteMessage::GetViewActionStyle( void ) const
{
	return	m_styleMsg.viewAction ;
}

// 表示スタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetViewActionStyle( const SGLSpriteMessage::ViewActionStyle& style )
{
	m_styleMsg.viewAction = style ;
}

// 関連スキン取得
//////////////////////////////////////////////////////////////////////////////
SGLSkinManager * SGLSpriteMessage::GetAttachedSkin( void ) const
{
	return	m_refSkin.GetReference() ;
}

// スキンを関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AttachSkin( SGLSkinManager * pSkin )
{
	m_refSkin = pSkin ;
}

// リンク装飾取得
//////////////////////////////////////////////////////////////////////////////
const SGLLetterer::Decoration&
	SGLSpriteMessage::GetLinkDecoration( int nStatus ) const
{
	ESLAssert( (nStatus >= 0) && (nStatus < LinkInfo::statusCount) ) ;
	return	m_styleMsg.decoLink[nStatus] ;
}

// リンク装飾設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetLinkDecoration
	( int nStatus, const SGLLetterer::Decoration& deco )
{
	ESLAssert( (nStatus >= 0) && (nStatus < LinkInfo::statusCount) ) ;
	m_styleMsg.decoLink[nStatus] = deco ;
}

// 文字アトラス化設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetCharacterAtlas( uint32_t nWidth, uint32_t nHeight )
{
	if ( m_pAtlas == NULL )
	{
		m_pAtlas = new Atlas ;
	}
	m_pAtlas->SetAtlasSize( nWidth, nHeight ) ;
}

void SGLSpriteMessage::ReleaseCharacterAtlas( void )
{
	m_pAtlas = NULL ;
}

// 表示文字を全消去
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::ClearMessage( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	NotifyUpdate() ;
	m_characters.RemoveAll() ;
	m_flagRectChars = false ;
	m_msecDuration = 0 ;
	m_msecLastTiming = 0 ;
	m_msecFading = 0 ;
	m_lcLettering = m_styleMsg.context ;
	m_fsFont = m_styleMsg.font ;
	m_font.SetStyle( m_styleMsg.font ) ;
	m_xmlHistory.RemoveAllContents() ;
	m_pFocusLink = NULL ;
	if ( m_pAtlas != nullptr )
	{
		m_pAtlas->ClearAll() ;
	}
	Unlock() ;
}

// 表示中の文字を即座に完了させる
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::FlushMessage( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_msecFading = m_msecDuration ;
	NotifyUpdate() ;
	Unlock() ;
}

// 文字が表示中（フェード中）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::IsMessagePending( void ) const
{
	return	(m_msecFading < m_msecDuration) ;
}

// 出力速度比 [x256] 設定 (0x10000 は一瞬表示)
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetMessageSpeedRatio( uint32_t fxSpeedRatio )
{
	Lock() ;
	m_fxSpeedRatio = fxSpeedRatio ;
	Unlock() ;
}

// スタイル解釈
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::ParseRichTextStyle
	( RichTextStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strRubyFont,
		const SSystem::SXMLDocument& xmlStyle )
{
	SGLSpriteText::ParseTextStyle( style, strFontFace, xmlStyle ) ;
	//
	strRubyFont = style.font.pszFace ;
	style.fontRuby = style.font ;
	style.fontRuby.nSize = style.font.nSize * 2 / 5 ;
	style.fontRuby.pszFace = strRubyFont ;
	//
	// <ruby>
	SXMLDocument *	pxmlRubyFont = xmlStyle.GetElementTagAs( L"ruby" ) ;
	if ( pxmlRubyFont != NULL )
	{
		SGLSpriteText::ParseFontStyle
			( style.fontRuby, strRubyFont, *pxmlRubyFont ) ;
	}
	//
	// <link>
	SXMLDocument *	pxmlLink = xmlStyle.GetElementTagAs( L"link" ) ;
	if ( pxmlLink != NULL )
	{
		static const wchar_t *	pwszStatus[LinkInfo::statusCount] =
		{
			L"normal",
			L"focus",
			L"pushing",
		} ;
		for ( int i = 0; i < LinkInfo::statusCount; i ++ )
		{
			SXMLDocument *	pxmlLinkStyle =
					pxmlLink->GetElementTagAs( pwszStatus[i] ) ;
			if ( pxmlLinkStyle != NULL )
			{
				SGLSpriteText::ParseTextDecoration
						( style.decoLink[i], *pxmlLinkStyle ) ;
			}
		}
	}
}

void SGLSpriteMessage::ParseMessageStyle
	( MessageStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strRubyFont,
		const SSystem::SXMLDocument& xmlStyle )
{
	ParseRichTextStyle( style, strFontFace, strRubyFont, xmlStyle ) ;
	//
	// <action>
	SXMLDocument *	pxmlAction = xmlStyle.GetElementTagAs( L"action" ) ;
	if ( pxmlAction != NULL )
	{
		style.viewAction.msecPerChar =
			(uint32_t) pxmlAction->GetAttrIntegerAs( L"speed" ) ;
		style.viewAction.msecFade =
			(uint32_t) pxmlAction->GetAttrIntegerAs( L"fade" ) ;
		style.viewAction.vMove.x =
			(float32_t) pxmlAction->GetAttrRealAs( L"move_x" ) ;
		style.viewAction.vMove.y =
			(float32_t) pxmlAction->GetAttrRealAs( L"move_y" ) ;
		style.viewAction.vZoom.x =
			(float32_t) pxmlAction->GetAttrRealAs( L"zoom_x" ) ;
		style.viewAction.vZoom.y =
			(float32_t) pxmlAction->GetAttrRealAs( L"zoom_y" ) ;
		style.viewAction.zRotation =
			(float32_t) pxmlAction->GetAttrRealAs( L"rotation" ) ;
	}
}

// 表示文字列追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AddMessageText( const wchar_t * pwszText )
{
	SXMLDocument	xmlText ;
	xmlText.SetText( pwszText ) ;
	AddMessageXML( xmlText ) ;
}

// 表示文字列追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AddMessageXML( const SSystem::SXMLDocument& xmlMsg )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( xmlMsg.GetType() == SXMLDocument::typeRoot )
	{
		size_t	count = xmlMsg.GetElementsCount() ;
		for ( size_t i = 0; i < count; i ++ )
		{
			SXMLDocument *	pxmlElement = xmlMsg.GetElementAt( i ) ;
			if ( pxmlElement != NULL )
			{
				m_xmlHistory.AddElement( new SXMLDocument( *pxmlElement ) ) ;
			}
		}
	}
	else
	{
		m_xmlHistory.AddElement( new SXMLDocument( xmlMsg ) ) ;
	}
	Unlock() ;
	//
	bool	flagPaused = false ;
	if ( m_pAtlas != NULL )
	{
		AtomicAdd( &m_nPausedMsg, 1 ) ;
		flagPaused = true ;
	}
	//
	AddLettering( xmlMsg ) ;
	VerticalAlignmentMessage() ;
	//
	if ( flagPaused )
	{
		ESLVerify( AtomicSub( &m_nPausedMsg, 1 ) >= 0 ) ;
	}
}

void SGLSpriteMessage::AddMessageXML( const wchar_t * pwszTextXML )
{
	SStringParser	sparsXML = pwszTextXML ;
	SXMLDocument	xmlText ;
	SStrSortObjectArray<SString>	ssoaDTD ;
	xmlText.ParseXMLElements( sparsXML, ssoaDTD, xmlText ) ;
	AddMessageXML( xmlText ) ;
}

// 画像追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AddMessageImage
	( SGLImageObject * pImage, int pitchChar, int xOffset, int yOffset )
{
	if ( pImage == NULL )
	{
		return ;
	}
	Character *	pChar = new Character ;
	pChar->m_pImage = pImage->NewReference() ;
	pChar->m_ptWriting = m_lcLettering.ptStartWriting ;
	pChar->m_ptOffset.x = xOffset ;
	pChar->m_ptOffset.y = yOffset ;
	pChar->m_sizeChar.w = pitchChar ;
	pChar->m_sizeChar.h = pitchChar ;
	pChar->m_msecStart = m_msecLastTiming ;
	if ( pitchChar == 0 )
	{
		pChar->m_sizeChar = pImage->GetImageSize() ;
	}
	Lock() ;
	if ( !m_lcLettering.flagVertical )
	{
		m_lcLettering.ptStartWriting.x += pChar->m_sizeChar.w ;
		if ( m_lcLettering.ptStartWriting.x > m_lcLettering.rectWritable.right )
		{
			m_lcLettering.ptStartWriting.x =
				m_lcLettering.rectWritable.left + m_lcLettering.widthIndent ;
			m_lcLettering.ptStartWriting.y += m_lcLettering.pitchLine ;
			pChar->m_ptWriting = m_lcLettering.ptStartWriting ;
			m_lcLettering.ptStartWriting.x += pChar->m_sizeChar.w ;
		}
	}
	else
	{
		m_lcLettering.ptStartWriting.y += pChar->m_sizeChar.h ;
		if ( m_lcLettering.ptStartWriting.y > m_lcLettering.rectWritable.bottom )
		{
			m_lcLettering.ptStartWriting.y =
				m_lcLettering.rectWritable.top + m_lcLettering.widthIndent ;
			m_lcLettering.ptStartWriting.x -= m_lcLettering.pitchLine ;
			pChar->m_ptWriting = m_lcLettering.ptStartWriting ;
			m_lcLettering.ptStartWriting.y += pChar->m_sizeChar.h ;
		}
	}
	m_characters.Add( pChar ) ;
	m_msecLastTiming += m_styleMsg.viewAction.msecPerChar ;
	m_msecDuration = m_msecLastTiming + m_styleMsg.viewAction.msecFade ;
	NotifyUpdate() ;
	Unlock() ;
}

// 垂直アライメント適用
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::VerticalAlignmentMessage( void )
{
	if ( (m_styleMsg.boxAlign
			& SGLSpriteText::alignBoxVertMask) != SGLSpriteText::alignBoxTop )
	{
		SGLRect	rectMsg ;
		if ( GetCircumscribedRect( rectMsg ) )
		{
			int	yOffset = 0 ;
			switch ( m_styleMsg.boxAlign & SGLSpriteText::alignBoxVertMask )
			{
			case	SGLSpriteText::alignBoxBottom:
				yOffset = m_styleMsg.context.rectWritable.GetHeight()
								- rectMsg.GetHeight() - rectMsg.top ;
				break ;
			case	SGLSpriteText::alignBoxVCenter:
				yOffset = (m_styleMsg.context.rectWritable.GetHeight()
									- rectMsg.GetHeight()) / 2 - rectMsg.top ;
				break ;
			}
			for ( size_t i = 0; i < m_characters.GetLength(); i ++ )
			{
				Character *	pChar = m_characters.GetAt( i ) ;
				if ( pChar != nullptr )
				{
					pChar->m_ptWriting.y += (int32_t) yOffset ;
				}
			}
		}
	}
}

// 次の出力位置を取得
//////////////////////////////////////////////////////////////////////////////
const SGLPoint& SGLSpriteMessage::GetNextMessagePoint( void ) const
{
	return	m_lcLettering.ptStartWriting ;
}

// 表示文字の外接矩形（ローカル座標）取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::GetCircumscribedRect( SGLRect& rectMsg ) const
{
	bool	flagRect = false ;
	size_t	nCount = m_characters.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		if ( pChar == nullptr )
		{
			continue ;
		}
		SGLImageObject *	pImage = pChar->m_pImage ;
		SGLPoint			ptOffset = pChar->m_ptOffset ;
		if ( pImage == nullptr )
		{
			LinkInfo *	pLink = pChar->m_pLink ;
			if ( pLink != nullptr )
			{
				SGLImageInfo	imginf ;
				pImage = pLink->m_pImage[pLink->m_status] ;
				if ( (pImage != nullptr)
					&& !(pImage->GetImageInfo( imginf )) )
				{
					ptOffset.x -= imginf.ptOrigin.x ;
					ptOffset.y -= imginf.ptOrigin.y ;
				}
			}
			if ( pImage == nullptr )
			{
				continue ;
			}
		}
		SGLSize	sizeImage = pImage->GetImageSize() ;
		if ( !sizeImage.IsEmpty() )
		{
			SGLPoint	ptChar = pChar->m_ptWriting ;
			ptChar += ptOffset ;
			//
			SGLRect	rect ;
			rect.SetPosition( ptChar ) ;
			rect.SetSize( sizeImage ) ;
			//
			if ( !flagRect )
			{
				flagRect = true ;
				rectMsg = rect ;
			}
			else
			{
				rectMsg |= rect ;
			}
		}
	}
	return	flagRect ;
}

// 表示文字数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMessage::GetMessageCharacterCount( void ) const
{
	return	m_characters.GetLength() ;
}

// 表示文字取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::Character *
	SGLSpriteMessage::GetMessageCharacterAt( size_t iMsg ) const
{
	return	m_characters.GetAt( iMsg ) ;
}

// 表示メッセージのプレーンテキスト取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteMessage::GetPlainText( void ) const
{
	return	m_xmlHistory.ToPlainText() ;
}

// 書式コンテキスト
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SaveLetteringContext
		( SGLSpriteMessage::LetteringContext& context )
{
	SaveLetteringContext
		( context.context, context.decoration, context.font, context.strFontFace ) ;
}

void SGLSpriteMessage::RestoreLetteringContext
		( const SGLSpriteMessage::LetteringContext& context )
{
	RestoreLetteringContext
		( context.context, context.decoration, context.font ) ;
}

// 文字色
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetTextColor( SGLPalette argbText )
{
	m_ltDecoration.rgbaBody = argbText ;
}

void SGLSpriteMessage::SetTextBorderColor( SGLPalette argbBorder )
{
	m_ltDecoration.rgbaBorder = argbBorder ;
}

void SGLSpriteMessage::SetTextBorderWidth( uint32_t nWidth )
{
	m_ltDecoration.widthBorder = nWidth ;
}

void SGLSpriteMessage::SetTextBorder2Color( SGLPalette argbBorder2 )
{
	m_ltDecoration.rgbaBorder2 = argbBorder2 ;
}

void SGLSpriteMessage::SetTextBorder2Width( uint32_t nWidth2 )
{
	m_ltDecoration.widthBorder2 = nWidth2 ;
}

void SGLSpriteMessage::SetTextShadowColor( SGLPalette argbBorder )
{
	m_ltDecoration.rgbaShadow = argbBorder ;
}

// 表示文字追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::AddLettering( const SSystem::SXMLDocument& xmlMsg )
{
	SXMLDocument::DocumentType	typeDoc = xmlMsg.GetType() ;
	if ( typeDoc == SXMLDocument::typeRoot )
	{
		return	AddLetteringElements( xmlMsg ) ;
	}
	else if ( typeDoc == SXMLDocument::typeTag )
	{
		if ( xmlMsg.GetTag() == L"font" )
		{
			//
			// フォント設定変更
			//
			SGLLetteringContext		contextSave ;
			SGLLetterer::Decoration	decoSave ;
			SGLFontStyle			fontSave ;
			SString					strFontSave ;
			SaveLetteringContext
				( contextSave, decoSave, fontSave, strFontSave ) ;
			//
			SString *	pstrSize = xmlMsg.GetAttributeAs( L"size" ) ;
			if ( pstrSize != NULL )
			{
				if ( m_lcLettering.pitchLine == 0 )
				{
					m_lcLettering.pitchLine = m_fsFont.nSize ;
				}
				if ( pstrSize->GetAt(0) == L'+' )
				{
					int64_t	offsetSize = pstrSize->AsInteger() ;
					m_fsFont.nSize += (int32_t) offsetSize ;
					m_lcLettering.pitchLine += (int32_t) offsetSize ;
				}
				else if ( pstrSize->GetAt(0) == L'-' )
				{
					int64_t	offsetSize = pstrSize->AsInteger() ;
					m_fsFont.nSize += (int32_t) offsetSize ;
					m_lcLettering.pitchLine += (int32_t) offsetSize ;
				}
				else if ( pstrSize->GetLastAt(0) == L'%' )
				{
					int32_t	nRate = (int32_t) pstrSize->AsInteger() ;
					m_fsFont.nSize = m_fsFont.nSize * nRate / 100 ;
					m_lcLettering.pitchLine = m_lcLettering.pitchLine * nRate / 100 ;
				}
				else
				{
					m_fsFont.nSize = (uint32_t) pstrSize->AsInteger() ;
					if ( m_lcLettering.pitchLine < (int32_t) m_fsFont.nSize )
					{
						m_lcLettering.pitchLine = m_fsFont.nSize ;
					}
				}
			}
			SString *	pstrFace = xmlMsg.GetAttributeAs( L"face" ) ;
			if ( pstrFace != NULL )
			{
				m_strFontCur = *pstrFace ;
				m_fsFont.pszFace = m_strFontCur ;
			}
			SString *	pstrItalic = xmlMsg.GetAttributeAs( L"italic" ) ;
			if ( pstrItalic != NULL )
			{
				m_fsFont.nStyles &= ~SGLFontStyle::styleItalic ;
				if ( *pstrItalic == L"true" )
				{
					m_fsFont.nStyles |= SGLFontStyle::styleItalic ;
				}
			}
			SString *	pstrBold = xmlMsg.GetAttributeAs( L"bold" ) ;
			if ( pstrBold != NULL )
			{
				m_fsFont.nStyles &= ~SGLFontStyle::styleBold ;
				if ( *pstrBold == L"true" )
				{
					m_fsFont.nStyles |= SGLFontStyle::styleBold ;
				}
			}
			m_font.SetStyle( m_fsFont ) ;
			//
			SString *	pstrLineHeight = xmlMsg.GetAttributeAs( L"line_height" ) ;
			if ( pstrLineHeight != NULL )
			{
				if ( m_lcLettering.pitchLine == 0 )
				{
					m_lcLettering.pitchLine = m_fsFont.nSize ;
				}
				if ( pstrLineHeight->GetAt(0) == L'+' )
				{
					int64_t	offsetSize = pstrLineHeight->AsInteger() ;
					m_lcLettering.pitchLine += (int32_t) offsetSize ;
				}
				else if ( pstrLineHeight->GetAt(0) == L'-' )
				{
					int64_t	offsetSize = pstrLineHeight->AsInteger() ;
					m_lcLettering.pitchLine += (int32_t) offsetSize ;
				}
				else if ( pstrSize->GetLastAt(0) == L'%' )
				{
					int32_t	nRate = (int32_t) pstrSize->AsInteger() ;
					m_lcLettering.pitchLine = m_lcLettering.pitchLine * nRate / 100 ;
				}
				else
				{
					m_lcLettering.pitchLine =
							(int32_t) pstrLineHeight->AsInteger() ;
				}
			}
			//
			m_ltDecoration.rgbaBody =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"body_argb", m_ltDecoration.rgbaBody.ui32 ) ;
			m_ltDecoration.rgbaBorder =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"border_argb", m_ltDecoration.rgbaBorder.ui32 ) ;
			m_ltDecoration.widthBorder =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"border_width", m_ltDecoration.widthBorder ) ;
			m_ltDecoration.rgbaBorder2 =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"border2_argb", m_ltDecoration.rgbaBorder2.ui32 ) ;
			m_ltDecoration.widthBorder2 =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"border2_width", m_ltDecoration.widthBorder2 ) ;
			m_ltDecoration.rgbaShadow =
				(uint32_t) xmlMsg.GetAttrHexIntegerAs
					( L"shadow_argb", m_ltDecoration.rgbaShadow.ui32 ) ;
			//
			size_t		iStart = m_characters.GetLength() ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			//
			SString *	pstrAlign = xmlMsg.GetAttributeAs( L"align" ) ;
			if ( pstrAlign != NULL )
			{
				if ( *pstrAlign == L"center" )
				{
					AlignmentLetter
						( iStart, SGLLetteringContext::alignCenter ) ;
				}
				else if ( *pstrAlign == L"right" )
				{
					AlignmentLetter
						( iStart, SGLLetteringContext::alignRight ) ;
				}
				else if ( *pstrAlign == L"left" )
				{
					AlignmentLetter
						( iStart, SGLLetteringContext::alignLeft ) ;
				}
			}
			RestoreLetteringContext( contextSave, decoSave, fontSave ) ;
			//
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"style" )
		{
			SGLLetteringContext		contextSave ;
			SGLLetterer::Decoration	decoSave ;
			SGLFontStyle			fontSave ;
			SString					strFontSave ;
			SaveLetteringContext
				( contextSave, decoSave, fontSave, strFontSave ) ;
			//
			SGLSkinManager *	pSkin = m_refSkin ;
			if ( pSkin != NULL )
			{
				SXMLDocument *	pxmlStyle =
					pSkin->GetStyleAs( xmlMsg.GetAttrStringAs( L"id" ) ) ;
				if ( pxmlStyle != NULL )
				{
					SGLSpriteText::TextStyle	style ;
					SString	strFontFace ;
					SGLSpriteText::ParseTextStyle
						( style, strFontFace, *pxmlStyle ) ;
					//
					SGLPoint	ptMsg = m_lcLettering.ptStartWriting ;
					SetTextStyle( style ) ;
					m_lcLettering.ptStartWriting = ptMsg ;
				}
			}
			//
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			//
			RestoreLetteringContext( contextSave, decoSave, fontSave ) ;
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"center" )
		{
			size_t		iStart = m_characters.GetLength() ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			AlignmentLetter
				( iStart, SGLLetteringContext::alignCenter ) ;
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"right" )
		{
			size_t		iStart = m_characters.GetLength() ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			AlignmentLetter
				( iStart, SGLLetteringContext::alignRight ) ;
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"left" )
		{
			size_t		iStart = m_characters.GetLength() ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			AlignmentLetter
				( iStart, SGLLetteringContext::alignLeft ) ;
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"phrase" )
		{
			SGLLetteringContext		contextSave ;
			SGLLetterer::Decoration	decoSave ;
			SGLFontStyle			fontSave ;
			SString					strFontSave ;
			SaveLetteringContext
				( contextSave, decoSave, fontSave, strFontSave ) ;
			//
			Lock() ;
			//
			size_t		iStart = m_characters.GetLength() ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			//
			if ( CountLineFeedFrom( iStart ) > 0 )
			{
				m_characters.Remove
					( iStart, m_characters.GetLength() - iStart ) ;
				RestoreLetteringContext( contextSave, decoSave, fontSave ) ;
				m_lcLettering.ptStartWriting = contextSave.ptStartWriting ;
				//
				SGLLetterer	letterer ;
				letterer.WriteLetter( m_font, m_lcLettering, L"\n" ) ;
				//
				err = AddLetteringElements( xmlMsg ) ;
			}
			Unlock() ;
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"br" )
		{
			SGLLetterer	letterer ;
			letterer.WriteLetter( m_font, m_lcLettering, L"\n" ) ;
		}
		else if ( xmlMsg.GetTag() == L"ruby" )
		{
			//
			// 振り仮名
			//
			size_t		iStart = m_characters.GetLength() ;
			SGLPoint	ptRubyStart = m_lcLettering.ptStartWriting ;
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			//
			size_t	i, countChars ;
			int32_t	widthWords = 0 ;
			LockTrace( __FILE__, __LINE__ ) ;
			countChars = m_characters.GetLength() ;
			for ( i = iStart; i < countChars; i ++ )
			{
				Character *	pChar = m_characters.GetAt( i ) ;
				if ( pChar != NULL )
				{
					if ( !m_lcLettering.flagVertical )
					{
						widthWords += pChar->m_sizeChar.w ;
					}
					else
					{
						widthWords += pChar->m_sizeChar.h ;
					}
				}
			}
			Unlock() ;
			//
			SGLLetteringContext	context ;
			context.rectWritable.left = 0 ;
			context.rectWritable.right = 0x7FFFFFFF ;
			context.rectWritable.top = 0 ;
			context.rectWritable.bottom = 0x7FFFFFFF ;
			context.flagVertical = m_lcLettering.flagVertical ;
			context.pitchLine = m_styleMsg.fontRuby.nSize ;
			if ( m_lcLettering.pitchLine != 0 )
			{
				if ( m_lcLettering.pitchLine
						- (int32_t) m_fsFont.nSize > context.pitchLine )
				{
					context.pitchLine =
						m_lcLettering.pitchLine - m_fsFont.nSize ;
				}
			}
			SGLFont	font ;
			font.SetStyle( m_styleMsg.fontRuby ) ;
			//
			SGLLetterer	letterer ;
			SString	strReading = xmlMsg.GetAttrStringAs( L"reading", NULL ) ;
			letterer.WriteLetter( font, context, strReading ) ;
			letterer.DecorateLetter( m_ltDecoration ) ;
			//
			size_t	countRuby = letterer.GetLetterLength() ;
			if ( countRuby > 0 )
			{
				int32_t	pitchRuby = (widthWords + 1) / (int32_t) countRuby ;
				//
				LockTrace( __FILE__, __LINE__ ) ;
				size_t		iNextWord = iStart ;
				size_t		iEndWord = m_characters.GetLength() ;
				SGLPoint	ptCharBase = ptRubyStart ;
				uint32_t	msecFadeBase = 0 ;
				int32_t		widthChar = 0 ;
				int32_t		offsetRuby = pitchRuby / 2 ;
				for ( i = 0; i < countRuby; i ++ )
				{
					while ( (offsetRuby >= widthChar) && (iNextWord < iEndWord) )
					{
						Character *	pChar = m_characters.GetAt( iNextWord ++ ) ;
						if ( pChar != NULL )
						{
							ptCharBase = pChar->m_ptWriting ;
							offsetRuby -= widthChar ;
							widthChar =
								m_lcLettering.flagVertical
									? pChar->m_sizeChar.h : pChar->m_sizeChar.w ;
							msecFadeBase = pChar->m_msecStart ;
						}
					}
					SGLLetterer::Character *
							pLtChar = letterer.GetCharacterAt( i ) ;
					if ( (pLtChar == NULL)
						|| (pLtChar->pImage == NULL) )
					{
						offsetRuby += pitchRuby ;
						continue ;
					}
					SGLSize	sizeImage = pLtChar->pImage->GetImageSize() ;
					Character *	pChar = new Character ;
					pChar->m_pImage = new SGLImage ;
					pChar->m_pImage->CreateCloneBuffer( *(pLtChar->pImage) ) ;
					pChar->m_ptOffset = pLtChar->ptOffset ;
					pChar->m_sizeChar = pLtChar->sizeChar ;
					pChar->m_msecStart = msecFadeBase ;
					//
					if ( !m_lcLettering.flagVertical )
					{
						pChar->m_ptWriting.x =
								ptCharBase.x + offsetRuby - sizeImage.w / 2 ;
						pChar->m_ptWriting.y = ptCharBase.y ;
					}
					else
					{
						pChar->m_ptWriting.x = ptCharBase.x ;
						pChar->m_ptWriting.y =
								ptCharBase.y + offsetRuby - sizeImage.h / 2 ;
					}
					offsetRuby += pitchRuby ;
					//
					if ( m_pAtlas != NULL )
					{
						SGLImageObject *	pAtlased = m_pAtlas->Allocate( *(pChar->m_pImage) ) ;
						if ( pAtlased != NULL )
						{
							delete	pChar->m_pImage ;
							pChar->m_pImage = pAtlased ;
						}
					}
					//
					m_characters.Add( pChar ) ;
					//
					SGLRect	rectChar ;
					rectChar.left = pChar->m_ptOffset.x + pChar->m_ptWriting.x ;
					rectChar.top = pChar->m_ptOffset.y + pChar->m_ptWriting.y ;
					rectChar.SetWidth( pChar->m_sizeChar.w ) ;
					rectChar.SetHeight( pChar->m_sizeChar.h ) ;
					if ( m_flagRectChars )
					{
						m_rectChars |= rectChar ;
					}
					else
					{
						m_rectChars = rectChar ;
					}
				}
				NotifyUpdate() ;
				Unlock() ;
			}
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"a" )
		{
			//
			// リンク
			//
			SString *	pstrText = xmlMsg.GetTextElement() ;
			if ( pstrText == NULL )
			{
				return	sglErrSuccess ;
			}
			LinkInfo *	pLinkInfo = new LinkInfo ;
			pLinkInfo->m_strLinkURL = xmlMsg.GetAttrStringAs( L"href" ) ;
			//
			Character *	pChar = new Character ;
			pChar->m_pLink = pLinkInfo ;
			pChar->m_msecStart = m_msecLastTiming ;
			//
			SArray<SGLImageRect>	aChars ;
			SGLLetterer	letterer ;
			letterer.WriteLetter
				( m_font, m_lcLettering, *pstrText ) ;
			//
			// 文字矩形収集
			//
			size_t	nCharCount = letterer.GetLetterLength() ;
			bool	fCharRectMerge = false ;
			SGLRect	rectCharMerge ;
			for ( size_t i = 0; i < nCharCount; i ++ )
			{
				SGLLetterer::Character *
						pLChar = letterer.GetCharacterAt( i ) ;
				SGLImageRect
						rectChar( pLChar->ptWriting, pLChar->sizeChar ) ;
				if ( (m_lcLettering.flagVertical
							== SGLLetteringContext::writingHorizontal)
					&& (m_lcLettering.pitchLine != 0)
					&& (rectChar.h < m_lcLettering.pitchLine) )
				{
					rectChar.h = m_lcLettering.pitchLine ;
				}
				aChars.Add( rectChar ) ;
				//
				if ( !fCharRectMerge )
				{
					fCharRectMerge = true ;
					rectCharMerge = rectChar ;
				}
				else
				{
					if ( (rectCharMerge.top == rectChar.y)
						&& (rectCharMerge.GetHeight() == rectChar.h) )
					{
						rectCharMerge |= SGLRect( rectChar ) ;
					}
					else
					{
						pLinkInfo->m_aHitRects.Add( rectCharMerge ) ;
						fCharRectMerge = false ;
					}
				}
				if ( m_flagRectChars )
				{
					m_rectChars |= rectChar ;
				}
				else
				{
					m_rectChars = rectChar ;
				}
			}
			if ( fCharRectMerge )
			{
				pLinkInfo->m_aHitRects.Add( rectCharMerge ) ;
				fCharRectMerge = false ;
			}
			//
			// 画像生成
			//
			letterer.CombineLetter() ;
			//
			SGLLetterer::Character *
					pLChar = letterer.GetCharacterAt( 0 ) ;
			if ( pLChar != NULL )
			{
				//
				// 下線を引く
				//
				for ( size_t i = 0; i < pLinkInfo->m_aHitRects.GetLength(); i ++ )
				{
					SGLRect *	pRect = pLinkInfo->m_aHitRects.GetAt( i ) ;
					ESLAssert( pRect != NULL ) ;
					SGLPalette		pxWhite( 0xFFFFFFFF ) ;
					SGLImageRect	rectUnder
						( pRect->left - pLChar->ptWriting.x
										- pLChar->ptOffset.x,
							pRect->bottom - pLChar->ptWriting.y
											- pLChar->ptOffset.y - 1,
							pRect->GetWidth(), 1 ) ;
					SGLImageRect	rectImage
						( 0, 0, pLChar->pImage->width,
									pLChar->pImage->height ) ;
					if ( rectUnder.x < 0 )
					{
						rectImage.x = rectUnder.x ;
						rectImage.w -= rectUnder.x ;
					}
					if ( rectUnder.x + rectUnder.w
									> rectImage.x + rectImage.w )
					{
						rectImage.w = rectUnder.x + rectUnder.w - rectImage.x ;
					}
					if ( rectUnder.y + rectUnder.h
									> rectImage.y + rectImage.h )
					{
						rectImage.h = rectUnder.y + rectUnder.h - rectImage.y ;
					}
					if ( (rectImage.w != pLChar->pImage->width)
						|| (rectImage.h != pLChar->pImage->height) )
					{
						// 画像バッファの拡張
						SGLImageInfo	imginf = *(pLChar->pImage) ;
						imginf.width = rectImage.w ;
						imginf.height = rectImage.h ;
						//
						SGLImageBuffer *
							pTempBuf = sglCreateImageBuffer( imginf ) ;
						sglBlendImageBuffer
							( *pTempBuf, *(pLChar->pImage),
								- rectImage.x, - rectImage.y ) ;
						//
						sglReleaseImageBuffer( pLChar->pImage ) ;
						pLChar->pImage = pTempBuf ;
					}
					sglFillImageBuffer
						( *(pLChar->pImage), pxWhite, &rectUnder ) ;
				}
			}
			//
			for ( int i = 0; i < LinkInfo::statusCount; i ++ )
			{
				SGLLetterer::Character	chChar ;
				if ( letterer.CreateDecoratedCharacterAt
							( chChar, m_styleMsg.decoLink[i], 0 ) )
				{
					continue ;
				}
				if ( i == 0 )
				{
					pChar->m_ptWriting = chChar.ptWriting ;
					pChar->m_ptOffset = chChar.ptOffset ;
					pChar->m_sizeChar = chChar.sizeChar ;
				}
				pLinkInfo->m_pImage[i] = new SGLImage ;
				pLinkInfo->m_pImage[i]->
							CreateCloneBuffer( *(chChar.pImage) ) ;
				sglReleaseImageBuffer( chChar.pImage ) ;
				//
				if ( m_pAtlas != NULL )
				{
					SGLImageObject *	pAtlased = m_pAtlas->Allocate( *(pLinkInfo->m_pImage[i]) ) ;
					if ( pAtlased != NULL )
					{
						delete	pLinkInfo->m_pImage[i] ;
						pLinkInfo->m_pImage[i] = pAtlased ;
					}
				}
				if ( i != 0 )
				{
					pLinkInfo->m_pImage[i]->SetImageOrigin
						( pChar->m_ptOffset.x - chChar.ptOffset.x,
							pChar->m_ptOffset.y - chChar.ptOffset.y ) ;
				}
			}
			Lock() ;
			m_characters.Add( pChar ) ;
			m_msecLastTiming += m_styleMsg.viewAction.msecPerChar ;
			Unlock() ;
		}
		else if ( xmlMsg.GetTag() == L"xfont" )
		{
			//
			// 外字
			//
			SGLSkinManager *	pSkin = m_refSkin.GetReference() ;
			if ( pSkin != NULL )
			{
				SGLImageObject *	pImage =
					pSkin->GetImageAs( xmlMsg.GetAttrStringAs( L"id" ) ) ;
				if ( pImage != NULL )
				{
					AddMessageImage
						( pImage,
							(int) xmlMsg.GetAttrIntegerAs( L"pitch", 0 ),
							(int) xmlMsg.GetAttrIntegerAs( L"offset_x", 0 ),
							(int) xmlMsg.GetAttrIntegerAs( L"offset_y", 0 ) ) ;
				}
			}
		}
		else if ( xmlMsg.GetTag() == L"speed" )
		{
			//
			// 速度
			//
			ViewActionStyle	styleViewAct = m_styleMsg.viewAction ;
			m_styleMsg.viewAction.msecPerChar =
				(uint32_t) xmlMsg.GetAttrIntegerAs
							( L"ms_per_char", m_styleMsg.viewAction.msecPerChar ) ;
			m_styleMsg.viewAction.msecFade =
				(uint32_t) xmlMsg.GetAttrIntegerAs
							( L"fade", m_styleMsg.viewAction.msecFade ) ;
			//
			SGLError	err = AddLetteringElements( xmlMsg ) ;
			//
			m_styleMsg.viewAction = styleViewAct ;
			//
			return	err ;
		}
		else if ( xmlMsg.GetTag() == L"wait" )
		{
			//
			// 一時停止
			//
			m_msecLastTiming +=
				(uint32_t) xmlMsg.GetAttrIntegerAs( L"time", 0 ) ;
		}
		else
		{
			return	AddLetteringElements( xmlMsg ) ;
		}
	}
	else if ( typeDoc == SXMLDocument::typeText )
	{
		//
		// 通常文字列
		//
		const SString&	strText = xmlMsg.GetText() ;
		SGLLetterer		letterer ;
		size_t	countChar =
			letterer.WriteLetter( m_font, m_lcLettering, strText ) ;
		letterer.DecorateLetter( m_ltDecoration ) ;
		//
		SGLImageRect	rectChars ;
		letterer.GetLetterRect( rectChars ) ;
		if ( m_flagRectChars )
		{
			m_rectChars |= SGLRect( rectChars ) ;
		}
		else
		{
			m_rectChars = rectChars ;
			m_flagRectChars = true ;
		}
		//
		size_t	countLetter = letterer.GetLetterLength() ;
		Lock() ;
		for ( size_t i = 0; i < countLetter; i ++ )
		{
			SGLLetterer::Character *
					pLtChar = letterer.GetCharacterAt( i ) ;
			if ( (pLtChar == NULL)
				|| (pLtChar->pImage == NULL) )
			{
				continue ;
			}
			Character *	pChar = new Character ;
			pChar->m_pImage = new SGLImage ;
			pChar->m_pImage->CreateCloneBuffer( *(pLtChar->pImage) ) ;
			pChar->m_ptWriting = pLtChar->ptWriting ;
			pChar->m_ptOffset = pLtChar->ptOffset ;
			pChar->m_sizeChar = pLtChar->sizeChar ;
			pChar->m_msecStart = m_msecLastTiming ;
			//
			if ( m_pAtlas != NULL )
			{
				SGLImageObject *	pAtlased = m_pAtlas->Allocate( *(pChar->m_pImage) ) ;
				if ( pAtlased != NULL )
				{
					delete	pChar->m_pImage ;
					pChar->m_pImage = pAtlased ;
				}
			}
			//
			m_characters.Add( pChar ) ;
			m_msecLastTiming += m_styleMsg.viewAction.msecPerChar ;
		}
		m_msecDuration = m_msecLastTiming + m_styleMsg.viewAction.msecFade ;
		NotifyUpdate() ;
		Unlock() ;
		//
		if ( countChar < strText.GetLength() )
		{
			return	sglErrAbort ;
		}
	}
	return	sglErrSuccess ;
}

// 表示文字追加（XML要素）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::AddLetteringElements( const SSystem::SXMLDocument& xmlMsg )
{
	size_t	count = xmlMsg.GetElementsCount() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SXMLDocument *	pxmlElement = xmlMsg.GetElementAt( i ) ;
		if ( pxmlElement != NULL )
		{
			SGLError	err = AddLettering( *pxmlElement ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 改行判定
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMessage::CountLineFeedFrom( size_t iStart ) const
{
	size_t	countLines = 0 ;
	size_t	countChars = m_characters.GetLength() ;
	while ( iStart < countChars )
	{
		Character *	pLineFirst = m_characters.GetAt( iStart ) ;
		if ( pLineFirst == NULL )
		{
			break ;
		}
		while ( ++ iStart < countChars )
		{
			Character *	pChar = m_characters.GetAt( iStart ) ;
			if ( pChar == NULL )
			{
				continue ;
			}
			if ( !m_lcLettering.flagVertical )
			{
				if ( pChar->m_ptWriting.y != pLineFirst->m_ptWriting.y )
				{
					countLines ++ ;
					break ;
				}
			}
			else
			{
				if ( pChar->m_ptWriting.x != pLineFirst->m_ptWriting.x )
				{
					countLines ++ ;
					break ;
				}
			}
		}
	}
	return	countLines ;
}

// 文字のアライメント処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AlignmentLetter
	( size_t iStart, SGLLetteringContext::AlignmentType typeAlign )
{
	size_t	countChars = m_characters.GetLength() ;
	while ( iStart < countChars )
	{
		Character *	pLineFirst = m_characters.GetAt( iStart ) ;
		if ( pLineFirst == NULL )
		{
			break ;
		}
		int32_t	widthLine =
			!m_lcLettering.flagVertical
				? pLineFirst->m_sizeChar.w : pLineFirst->m_sizeChar.h ;
		size_t	iLineEnd = iStart ;
		while ( ++ iLineEnd < countChars )
		{
			Character *	pChar = m_characters.GetAt( iLineEnd ) ;
			if ( pChar == NULL )
			{
				continue ;
			}
			if ( !m_lcLettering.flagVertical )
			{
				if ( pChar->m_ptWriting.y != pLineFirst->m_ptWriting.y )
				{
					break ;
				}
				widthLine = pChar->m_ptWriting.x
								+ pChar->m_sizeChar.w
									- pLineFirst->m_ptWriting.x ;
			}
			else
			{
				if ( pChar->m_ptWriting.x != pLineFirst->m_ptWriting.x )
				{
					break ;
				}
				widthLine = pChar->m_ptWriting.y
								+ pChar->m_sizeChar.h
									- pLineFirst->m_ptWriting.y ;
			}
		}
		SGLPoint	ptOffset( 0, 0 ) ;
		switch ( typeAlign )
		{
		case	SGLLetteringContext::alignLeft:
			break ;
		case	SGLLetteringContext::alignRight:
			if ( !m_lcLettering.flagVertical )
			{
				ptOffset.x = m_lcLettering.rectWritable.right
								- (pLineFirst->m_ptWriting.x + widthLine) ;
			}
			else
			{
				ptOffset.y = m_lcLettering.rectWritable.bottom
								- (pLineFirst->m_ptWriting.y + widthLine) ;
			}
			break ;
		case	SGLLetteringContext::alignCenter:
			if ( !m_lcLettering.flagVertical )
			{
				ptOffset.x = (m_lcLettering.rectWritable.right
								- (pLineFirst->m_ptWriting.x + widthLine)) / 2 ;
			}
			else
			{
				ptOffset.y = (m_lcLettering.rectWritable.bottom
								- (pLineFirst->m_ptWriting.y + widthLine)) / 2 ;
			}
			break ;
		default:
			break ;
		}
		while ( iStart < iLineEnd )
		{
			Character *	pChar = m_characters.GetAt( iStart ++ ) ;
			if ( pChar != NULL )
			{
				pChar->m_ptWriting += ptOffset ;
			}
		}
		if ( iLineEnd >= countChars )
		{
			m_lcLettering.ptStartWriting += ptOffset ;
		}
	}
}

// 表示属性保存
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SaveLetteringContext
	( SGLLetteringContext& context,
		SGLLetterer::Decoration& decoration,
		SGLFontStyle& font, SSystem::SString& strFontFace )
{
	context = m_lcLettering ;
	strFontFace = m_fsFont.pszFace ;
	decoration = m_ltDecoration ;
	font = m_fsFont ;
	font.pszFace = strFontFace ;
}

// 表示属性復帰
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::RestoreLetteringContext
	( const SGLLetteringContext& context,
		const SGLLetterer::Decoration& decoration, const SGLFontStyle& font )
{
	m_lcLettering.rectWritable = context.rectWritable ;
	m_lcLettering.typeAlignment = context.typeAlignment ;
	m_lcLettering.flagVertical = context.flagVertical ;
	m_lcLettering.pitchChar = context.pitchChar ;
	m_lcLettering.offsetChar = context.offsetChar ;
	m_lcLettering.scalePitch = context.scalePitch ;
	m_lcLettering.pitchLine = context.pitchLine ;
	m_lcLettering.widthIndent = context.widthIndent ;
	m_lcLettering.minHyphening = context.minHyphening ;
	m_lcLettering.maxProhibition = context.maxProhibition ;
	//
	m_ltDecoration = decoration ;
	//
	m_strFontCur = font.pszFace ;
	m_fsFont = font ;
	m_fsFont.pszFace = m_strFontCur ;
	m_font.SetStyle( font ) ;
}

// リンクあたり判定
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMessage::LinkInfo *
	SGLSpriteMessage::HitTestLinkInfo( int x, int y ) const
{
	for ( size_t i = 0; i < m_characters.GetLength(); i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		if ( pChar && pChar->m_pLink )
		{
			LinkInfo *		pLink = pChar->m_pLink ;
			const SGLRect *	pRects = pLink->m_aHitRects.GetConstArray() ;
			const size_t	nRectCount = pLink->m_aHitRects.GetLength() ;
			for ( size_t i = 0; i < nRectCount; i ++ )
			{
				if ( (pRects->left <= x) && (x <= pRects->right)
					&& (pRects->top <= y) && (y <= pRects->bottom) )
				{
					return	pLink ;
				}
				pRects ++ ;
			}
		}
	}
	return	NULL ;
}

// リンククリック時処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::OnClickLinkInfo
	( const SGLSpriteMessage::LinkInfo * pLink )
{
	ESLAssert( pLink != NULL ) ;
	if ( pLink && !(pLink->m_strLinkURL.IsEmpty()) )
	{
		OpenShellFile( pLink->m_strLinkURL ) ;
	}
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteMessage::GetText( void ) const
{
	SSmartBuffer	sbuf ;
	m_xmlHistory.FormatXMLElements( sbuf, 0, Charset::encodingUTF8 ) ;
	//
	SStringParser	sparsXML ;
	sbuf.Seek( 0 ) ;
	sparsXML.ReadTextFile( sbuf, Charset::encodingUTF8 ) ;
	//
	return	(const SString&) sparsXML ;
}

void SGLSpriteMessage::SetText( const wchar_t * pwszText )
{
	ClearMessage() ;
	AddMessageXML( pwszText ) ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::SetTextFont
	( const wchar_t * pwszFont, int nSize )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_strFontFace = pwszFont ;
	SGLSpriteText::SelectValidFont( m_strFontFace ) ;
	m_styleMsg.font.pszFace = m_strFontFace ;
	m_styleMsg.fontRuby.pszFace = m_strFontFace ;
	m_fsFont.pszFace = m_strFontFace ;
	if ( nSize != 0 )
	{
		m_styleMsg.font.nSize = nSize ;
		m_fsFont.nSize = nSize ;
	}
	m_font.SetStyle( m_fsFont ) ;
	Unlock() ;
	//
	ClearMessage() ;
	//
	AddLettering( m_xmlHistory ) ;
	FlushMessage() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	if ( (m_nPausedMsg == 0) && (m_msecFading < m_msecDuration) )
	{
		if ( m_fxSpeedRatio >= 0x10000 )
		{
			m_msecFading = m_msecDuration ;
		}
		else
		{
			m_msecFading += msecPast * m_fxSpeedRatio / 0x100 ;
			if ( m_msecFading > m_msecDuration )
			{
				m_msecFading = m_msecDuration ;
			}
		}
		NotifyUpdate() ;
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMessage::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;
	//
	SGLDrawImageParamList	dipl ;
	DrawCharacterImageList( dipl, s3dView ) ;
	dipl.Draw( render ) ;
}

void SGLSpriteMessage::DrawChildrenImageList
	( SGLDrawImageParamList& dipl, SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildrenImageList( dipl, s3dView ) ;
	//
	DrawCharacterImageList( dipl, s3dView ) ;
}

void SGLSpriteMessage::DrawCharacterImageList
	( SGLDrawImageParamList& dipl, SGLSprite::Stereo3DView s3dView ) const
{
	SGLAffine		affine ;
	const size_t	countChar = m_characters.GetLength() ;
	for ( size_t i = 0; i < countChar; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		if ( pChar == NULL )
		{
			continue ;
		}
		if ( pChar->m_msecStart > m_msecFading )
		{
			continue ;
		}
		SGLImageObject *	pImage = pChar->m_pImage ;
		SGLPoint			ptOffset = pChar->m_ptOffset ;
		if ( pImage == NULL )
		{
			LinkInfo *	pLink = pChar->m_pLink ;
			if ( pLink != NULL )
			{
				SGLImageInfo	imginf ;
				pImage = pLink->m_pImage[pLink->m_status] ;
				if ( (pImage != NULL)
					&& !(pImage->GetImageInfo( imginf )) )
				{
					ptOffset.x -= imginf.ptOrigin.x ;
					ptOffset.y -= imginf.ptOrigin.y ;
				}
			}
			if ( pImage == NULL )
			{
				continue ;
			}
		}
		SGLPaintParam	ppChar ;
		uint32_t	msecOffset = m_msecFading - pChar->m_msecStart ;
		if ( msecOffset < m_styleMsg.viewAction.msecFade )
		{
			SGLSize	sizeImage = pImage->GetImageSize() ;
			double	t = (double) msecOffset
							/ (double) m_styleMsg.viewAction.msecFade ;
			t = (1.0 - t) * (1.0 - t) ;
			//
			double	xDst = pChar->m_ptWriting.x + ptOffset.x ;
			double	yDst = pChar->m_ptWriting.y + ptOffset.y ;
			double	xSrcOrg = sizeImage.w * 0.5 ;
			double	ySrcOrg = sizeImage.h * 0.5 ;
			double	xZoom = 1.0 ;
			double	yZoom = 1.0 ;
			double	zAngle = 0.0 ;
			xDst += xSrcOrg ;
			yDst += ySrcOrg ;
			xDst += m_styleMsg.viewAction.vMove.x * t ;
			yDst += m_styleMsg.viewAction.vMove.y * t ;
			xZoom += (m_styleMsg.viewAction.vZoom.x - xZoom) * t ;
			yZoom += (m_styleMsg.viewAction.vZoom.y - yZoom) * t ;
			zAngle += m_styleMsg.viewAction.zRotation * t ;
			//
			ppChar.SetAffine
				( affine, xDst, yDst, xSrcOrg, ySrcOrg, xZoom, yZoom, zAngle ) ;
			ppChar.nTransparency =
				0x100 - msecOffset * 0x100 / m_styleMsg.viewAction.msecFade ;
			//
			if ( ppChar.nTransparency >= 0x100 )
			{
				continue ;
			}
		}
		else
		{
			ppChar.ptPaint = pChar->m_ptWriting ;
			ppChar.ptPaint += ptOffset ;
		}
		ppChar.nFlags |= paintDelayable ;
		//
		dipl.AddDrawParam( ppChar, pImage ) ;
	}
}

// 画像化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::RasterizeTextImage
	( SGLImageObject& imgText, SGLPoint& ptOffset )
{
	SGLRect	rectMsg ;
	if ( !GetCircumscribedRect( rectMsg ) )
	{
		return	sglErrFailed ;
	}
	//
	// 画像バッファ作成とオフセット計算
	//
	ptOffset = rectMsg.GetPosition() ;
	imgText.CreateImage
		( (uint32_t) rectMsg.GetWidth(),
			(uint32_t) rectMsg.GetHeight(),
			formatImageARGB, 32 ) ;
	//
	// 文字画像描画
	//
	size_t	nCount = m_characters.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		if ( pChar == nullptr )
		{
			continue ;
		}
		SGLImageObject *	pImage = pChar->m_pImage ;
		SGLPoint			ptCharOffset = pChar->m_ptOffset ;
		if ( pImage == nullptr )
		{
			LinkInfo *	pLink = pChar->m_pLink ;
			if ( pLink != nullptr )
			{
				SGLImageInfo	imginf ;
				pImage = pLink->m_pImage[pLink->m_status] ;
				if ( (pImage != nullptr)
					&& !(pImage->GetImageInfo( imginf )) )
				{
					ptCharOffset.x -= imginf.ptOrigin.x ;
					ptCharOffset.y -= imginf.ptOrigin.y ;
				}
			}
			if ( pImage == nullptr )
			{
				continue ;
			}
		}
		SGLPoint	ptChar = pChar->m_ptWriting ;
		ptChar += ptCharOffset ;
		ptChar -= ptOffset ;
		imgText.BlendImage( pImage, ptChar.x, ptChar.y ) ;
	}
	return	sglErrSuccess ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::GetRectangle( SGLRect& rectExt ) const
{
	SGLRect	rectText = m_styleMsg.context.rectWritable ;
	if ( m_flagRectChars )
	{
		int	nFontSize = m_styleMsg.font.nSize ;
		int	nMargin =
			eslRoundR32ToInt
				( (float32_t) ((m_styleMsg.viewAction.vZoom.x - 1.0f) * nFontSize) ) ;
		if ( fabs( m_styleMsg.viewAction.zRotation ) > 1.0e-5 )
		{
			nMargin += nFontSize ;
		}
		rectText.left = m_rectChars.left - nMargin ;
		rectText.top = m_rectChars.top - nMargin ;
		rectText.right = m_rectChars.right + nMargin + 1 ;
		rectText.bottom = m_rectChars.bottom + nMargin + 1 ;
		//
		if ( m_styleMsg.viewAction.vMove.x < 0.0f )
		{
			rectText.left += eslRoundR32ToInt( m_styleMsg.viewAction.vMove.x ) ;
		}
		else if ( m_styleMsg.viewAction.vMove.x > 0.0f )
		{
			rectText.right += eslRoundR32ToInt( m_styleMsg.viewAction.vMove.x ) ;
		}
		if ( m_styleMsg.viewAction.vMove.y < 0.0f )
		{
			rectText.top += eslRoundR32ToInt( m_styleMsg.viewAction.vMove.y ) ;
		}
		else if ( m_styleMsg.viewAction.vMove.y > 0.0f )
		{
			rectText.bottom += eslRoundR32ToInt( m_styleMsg.viewAction.vMove.y ) ;
		}
	}
	else
	{
		rectText.right = rectText.left ;
		rectText.bottom = rectText.top ;
	}
	if ( LocalToGlobalRect( rectText ) )
	{
		if ( SGLSprite::GetRectangle( rectExt ) )
		{
			rectExt |= rectText ;
			return	true ;
		}
		else
		{
			rectExt = rectText ;
			return	true ;
		}
	}
	else
	{
		return	SGLSprite::GetRectangle( rectExt ) ;
	}
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::IsHitSprite( double x, double y ) const
{
	if ( HitTestLinkInfo
		( (int) eslRoundR64ToLInt(x),
			(int) eslRoundR64ToLInt(y) ) != NULL )
	{
		return	true ;
	}
	return	SGLSprite::IsHitSprite( x, y ) ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	LinkInfo *	pHitLink =
		HitTestLinkInfo
			( (int) eslRoundR64ToLInt(xPos),
				(int) eslRoundR64ToLInt(yPos) ) ;
	if ( m_pFocusLink != pHitLink )
	{
		if ( m_pFocusLink != NULL )
		{
			m_pFocusLink->m_status = LinkInfo::statusNormal ;
		}
		if ( pHitLink != NULL )
		{
			pHitLink->m_status = LinkInfo::statusFocus ;
		}
		m_pFocusLink = pHitLink ;
		PostUpdate() ;
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteMessage::OnMouseLeave( int64_t nFlags )
{
	if ( m_pFocusLink != NULL )
	{
		m_pFocusLink->m_status = LinkInfo::statusNormal ;
		m_pFocusLink = NULL ;
		PostUpdate() ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMessage::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	LinkInfo *	pHitLink =
		HitTestLinkInfo
			( (int) eslRoundR64ToLInt(xPos),
				(int) eslRoundR64ToLInt(yPos) ) ;
	if ( m_pFocusLink == pHitLink )
	{
		if ( pHitLink->m_status == LinkInfo::statusFocus )
		{
			pHitLink->m_status = LinkInfo::statusPushing ;
			PostUpdate() ;
			return	true ;
		}
	}
	return	SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLSpriteMessage::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	LinkInfo *	pHitLink =
		HitTestLinkInfo
			( (int) eslRoundR64ToLInt(xPos),
				(int) eslRoundR64ToLInt(yPos) ) ;
	if ( m_pFocusLink == pHitLink )
	{
		if ( pHitLink->m_status == LinkInfo::statusPushing )
		{
			OnClickLinkInfo( pHitLink ) ;
			//
			pHitLink->m_status = LinkInfo::statusFocus ;
			PostUpdate() ;
			return	true ;
		}
	}
	return	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
}

// Rosetta 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteMessage::GetRSClassName( void ) const
{
	return	L"MessageSprite" ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteMessage::GetLQClassName( void ) const
{
	return	L"EntisGLS4.MessageSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteMessage::DuplicateObject( void )
{
	return	new SGLSpriteMessage( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_msecDuration, sizeof(uint32_t) ) ;
	file.Write( &m_msecLastTiming, sizeof(uint32_t) ) ;
	file.Write( &m_msecFading, sizeof(uint32_t) ) ;
	file.Write( &m_fxSpeedRatio, sizeof(uint32_t) ) ;
	//
	file.WriteString( SString( m_styleMsg.font.pszFace ) ) ;
	file.WriteString( SString( m_styleMsg.fontRuby.pszFace ) ) ;
	file.WriteString( m_strProhibition ) ;
	m_styleMsg.SaveWithoutPointer( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	SString	strSkinID ;
	if ( posm != NULL )
	{
		strSkinID = posm->GetIdentityOf( (SGLObject*) m_refSkin.GetReference() ) ;
	}
	file.WriteString( strSkinID ) ;
	//
	m_lcLettering.SaveWithoutPointer( file ) ;
	file.Write( &m_ltDecoration, sizeof(SGLLetterer::Decoration) ) ;
	//
	SSmartBuffer	sbuf ;
	m_xmlHistory.FormatXMLElements( sbuf, 0, Charset::encodingUTF8 ) ;
	//
	SStringParser	sparsXML ;
	sbuf.Seek( 0 ) ;
	sparsXML.ReadTextFile( sbuf, Charset::encodingUTF8 ) ;
	file.WriteString( sparsXML ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMessage::OnRestore( SSystem::SFileInterface& file )
{
	ClearMessage() ;
	//
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	msecDuration, msecLastTiming, msecFading, fxSpeedRatio ;
	file.Read( &msecDuration, sizeof(uint32_t) ) ;
	file.Read( &msecLastTiming, sizeof(uint32_t) ) ;
	file.Read( &msecFading, sizeof(uint32_t) ) ;
	file.Read( &fxSpeedRatio, sizeof(uint32_t) ) ;
	//
	file.ReadString( m_strFontFace ) ;
	file.ReadString( m_strRubyFont ) ;
	file.ReadString( m_strProhibition ) ;
	m_styleMsg.LoadWithoutPointer( file ) ;
	//
	m_styleMsg.font.pszFace = m_strFontFace ;
	m_styleMsg.context.pwszProhibition = m_strProhibition ;
	m_styleMsg.fontRuby.pszFace = m_strRubyFont ;
	//
	SString	strSkinID ;
	file.ReadString( strSkinID ) ;
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm != NULL )
	{
		m_refSkin.SetReference
			( ESLTypeCast<SGLSkinManager>( posm->GetObjectOf(strSkinID) ) ) ;
	}
	//
	SGLLetteringContext	lcLettering ;
	lcLettering.LoadWithoutPointer( file ) ;
	lcLettering.pwszProhibition = m_styleMsg.context.pwszProhibition ;
	//
	file.Read( &m_ltDecoration, sizeof(SGLLetterer::Decoration) ) ;
	//
	SString	strHistoryXML ;
	file.ReadString( strHistoryXML ) ;
	//
	SStrSortObjectArray<SString>	ssoaDTD ;
	SStringParser					sparsXML ;
	sparsXML.AttachString( strHistoryXML ) ;
	m_xmlHistory.RemoveAllContents() ;
	m_xmlHistory.ParseXMLElements( sparsXML, ssoaDTD, m_xmlHistory ) ;
	//
	AddLettering( m_xmlHistory ) ;
	//
	m_msecDuration = msecDuration ;
	m_msecLastTiming = msecLastTiming ;
	m_msecFading = msecFading ;
	m_fxSpeedRatio = fxSpeedRatio ;
	//
	m_lcLettering = lcLettering ;
	//
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// テキスト表示スプライト（テキスト装飾／省メモリスクロール対応）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
( SakuraGL::SGLSpriteSmartTextView, SGLSprite, SGLSpriteScrollListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteSmartTextView::SGLSpriteSmartTextView( void )
	: m_flagSwiping( false )
{
}

SGLSpriteSmartTextView::SGLSpriteSmartTextView( const SGLSpriteSmartTextView& src )
	: m_strFontFace( src.m_strFontFace ),
		m_strProhibition( src.m_strProhibition ),
		m_styleText( src.m_styleText ),
		m_refSkin( src.m_refSkin ),
		m_sizeView( src.m_sizeView ),
		m_flagSwiping( false )
{
	m_styleText.font.pszFace = m_strFontFace ;
	m_styleText.context.pwszProhibition = m_strProhibition ;
	m_styleText.fontRuby.pszFace = m_strFontFace ;
	//
	if ( !src.m_sizeView.IsEmpty() )
	{
		CreateView
			( (uint32_t) src.m_sizeView.w,
				(uint32_t) src.m_sizeView.h, src.IsBuffered() ) ;
	}
	SetText( src.m_strXMLText ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteSmartTextView::~SGLSpriteSmartTextView( void )
{
	DetachSyncTimeout( 100 ) ;
	ReleaseView() ;
}

// 作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteSmartTextView::CreateView
	( uint32_t width, uint32_t height, bool flagBuffered )
{
	ReleaseView() ;
	//
	if ( flagBuffered )
	{
		SGLError	err = SGLSprite::CreateBuffer( width, height ) ;
		if ( err )
		{
			SGLSprite::ReleaseBuffer() ;
			return	err ;
		}
	}
	m_sizeView.w = (int32_t) width ;
	m_sizeView.h = (int32_t) height ;
	//
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::ReleaseView( void )
{
	if ( !m_sizeView.IsEmpty() )
	{
		if ( SGLSprite::IsBuffered() )
		{
			SGLSprite::ReleaseBuffer() ;
		}
		m_sizeView.w = 0 ;
		m_sizeView.h = 0 ;
	}
}

// スクロール高計算（暫定）
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteSmartTextView::EstimateScrollHeight( void ) const
{
	int	nHeight = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	for ( size_t i = 0; i < m_aLines.GetLength(); i ++ )
	{
		TextLine *	pLine = m_aLines.GetLastAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		if ( pLine->m_nLineHeight >= 0 )
		{
			nHeight += pLine->m_yLineTop + pLine->m_nLineHeight ;
			break ;
		}
		else
		{
			if ( m_styleText.context.pitchLine > 0 )
			{
				nHeight += m_styleText.context.pitchLine ;
			}
			else
			{
				nHeight += m_styleText.font.nSize ;
			}
		}
	}
	Unlock() ;
	return	nHeight ;
}

// 現在のスクロール位置をスクロールバーへ反映
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::ReflectToScrollBar( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pScrollBar = m_refScrollBar.GetReference() ;
	if ( pScrollBar != nullptr )
	{
		pScrollBar->SetScrollRange( EstimateScrollHeight() - m_sizeView.h ) ;
		pScrollBar->SetScrollPos( m_ptScroll.y ) ;
	}
	SGLBasicForm::TrackBar * pTrackBar = m_refTrackBar.GetReference() ;
	if ( pTrackBar != nullptr )
	{
		pTrackBar->SetBarRange( EstimateScrollHeight() - m_sizeView.h ) ;
		pTrackBar->SetBarPos( m_ptScroll.y ) ;
	}
	Unlock() ;
}

// テキストスタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteSmartTextView::TextStyle&
	SGLSpriteSmartTextView::GetTextStyle( void ) const
{
	return	m_styleText ;
}

// テキストスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::SetTextStyle( const SGLSpriteSmartTextView::TextStyle& style )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_styleText = style ;
	m_strFontFace = style.font.pszFace ;
	if ( style.fontRuby.pszFace != NULL )
	{
		m_strRubyFont = style.fontRuby.pszFace ;
	}
	else
	{
		m_strRubyFont = style.font.pszFace ;
	}
	m_strProhibition = m_styleText.context.pwszProhibition ;
	m_styleText.font.pszFace = m_strFontFace ;
	m_styleText.context.pwszProhibition = m_strProhibition ;
	if ( m_styleText.fontRuby.nSize == 0 )
	{
		m_styleText.fontRuby.nSize = m_styleText.font.nSize * 2 / 5 ;
	}
	m_styleText.fontRuby.pszFace = m_strRubyFont ;
	//
	UpdateTextView() ;
	PrepareTextDraw() ;
	Unlock() ;
}

// ルビ表示フォントスタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLFontStyle& SGLSpriteSmartTextView::GetRubyFontStyle( void ) const
{
	return	m_styleText.fontRuby ;
}

// ルビ表示フォントスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::SetRubyFontStyle( const SGLFontStyle& style )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_styleText.fontRuby = style ;
	//
	m_strRubyFont = style.pszFace ;
	m_styleText.fontRuby.pszFace = m_strRubyFont ;
	Unlock() ;
}

// 関連スキン取得
//////////////////////////////////////////////////////////////////////////////
SGLSkinManager * SGLSpriteSmartTextView::GetAttachedSkin( void ) const
{
	return	m_refSkin.GetReference() ;
}

// スキンを関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::AttachSkin( SGLSkinManager * pSkin )
{
	m_refSkin = pSkin ;
}

// リンク装飾取得
//////////////////////////////////////////////////////////////////////////////
const SGLLetterer::Decoration& SGLSpriteSmartTextView::GetLinkDecoration( int nStatus ) const
{
	ESLAssert( (nStatus >= 0) && (nStatus < SGLSpriteMessage::LinkInfo::statusCount) ) ;
	return	m_styleText.decoLink[nStatus] ;
}

// リンク装飾設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::SetLinkDecoration
	( int nStatus , const SGLLetterer::Decoration& deco )
{
	ESLAssert( (nStatus >= 0) && (nStatus < SGLSpriteMessage::LinkInfo::statusCount) ) ;
	m_styleText.decoLink[nStatus] = deco ;
}

// 更新処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::UpdateView( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	UpdateTextView() ;
	PrepareTextDraw() ;
	Unlock() ;
}

// スタイル解釈
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::ParseRichTextStyle
	( TextStyle& style,
		SSystem::SString& strFontFace,
		SSystem::SString& strRubyFont,
		const SSystem::SXMLDocument& xmlStyle )
{
	SGLSpriteMessage::ParseRichTextStyle
		( style, strFontFace, strRubyFont, xmlStyle ) ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteSmartTextView::GetRectangle( SGLRect& rectExt ) const
{
	if ( SGLSprite::GetRectangle( rectExt ) )
	{
		if ( !m_sizeView.IsEmpty() )
		{
			rectExt |= SGLRect( 0, 0, m_sizeView.w - 1, m_sizeView.h - 1 ) ;
		}
		return	true ;
	}
	else
	{
		if ( !m_sizeView.IsEmpty() )
		{
			rectExt.left = 0 ;
			rectExt.top = 0 ;
			rectExt.SetSize( m_sizeView ) ;
			return	true ;
		}
	}
	return	false ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteSmartTextView::GetText( void ) const
{
	return	m_strXMLText ;
}

void SGLSpriteSmartTextView::SetText( const wchar_t * pwszText )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_strXMLText = pwszText ;
	//
	UpdateTextView() ;
	PrepareTextDraw() ;
	Unlock() ;
}

void SGLSpriteSmartTextView::UpdateTextView( void )
{
	//
	// 以前のデータを削除する
	//
	for ( size_t i = 0; i < m_aLines.GetLength(); i ++ )
	{
		TextLine *	pLine = m_aLines.GetAt( i ) ;
		ESLAssert( pLine != NULL ) ;
		if ( (pLine != NULL)
			&& (pLine->m_pSprite != NULL) )
		{
			SGLSprite::DetachChild( pLine->m_pSprite ) ;
		}
	}
	m_aLines.RemoveAll() ;
	//
	// 文字列をXML解釈する
	//
	SStringParser	sparsXML ;
	sparsXML.AttachString( m_strXMLText ) ;
	//
	SXMLDocument	xmlText ;
	SStrSortObjectArray<SString>	ssoaDTD ;
	xmlText.ParseXMLElements( sparsXML, ssoaDTD, xmlText ) ;
	//
	// 行に分解する
	//
	TextLine *	pLine = new TextLine ;
	m_aLines.Add( pLine ) ;
	//
	for ( size_t i = 0; i < xmlText.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlText.GetElementAt( i ) ;
		if ( pxmlTag == NULL )
		{
			continue ;
		}
		if ( (pxmlTag->GetType() == SXMLDocument::typeText)
			|| (pxmlTag->GetType() == SXMLDocument::typeCDATA) )
		{
			size_t	iLast = 0 ;
			for ( ; ; )
			{
				ssize_t	iEndOfLine =
					pxmlTag->GetText().Find( L'\n', iLast ) ;
				if ( iEndOfLine < 0 )
				{
					break ;
				}
				SXMLDocument *	pxmlText = new SXMLDocument ;
				pxmlTag->SetText
					( pxmlTag->GetText().Middle
						( iLast, iEndOfLine + 1 - (ssize_t) iLast ) ) ;
				pLine->m_xmlLine.AddElement( pxmlText ) ;
				//
				pLine = new TextLine ;		// 新しい行
				m_aLines.Add( pLine ) ;
				//
				iLast = (size_t) iEndOfLine + 1 ;
			}
			if ( iLast < pxmlTag->GetText().GetLength() )
			{
				SXMLDocument *	pxmlText = new SXMLDocument ;
				pxmlTag->SetText( pxmlTag->GetText().Middle( iLast ) ) ;
				pLine->m_xmlLine.AddElement( pxmlText ) ;
			}
		}
		else
		{
			pLine->m_xmlLine.AddElement( new SXMLDocument( *pxmlTag ) ) ;
			//
			if ( pxmlTag->GetTag() == L"br" )
			{
				pLine = new TextLine ;		// 新しい行
				m_aLines.Add( pLine ) ;
			}
		}
	}
	PostUpdate() ;
}

void SGLSpriteSmartTextView::PrepareTextDraw( bool fAllSize )
{
	int		yNext = 0 ;
	bool	fUnderView = false ;
	for ( size_t i = 0; i < m_aLines.GetLength(); i ++ )
	{
		TextLine *	pLine = m_aLines.GetAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		if ( (pLine->m_nLineHeight < 0) && (!fUnderView || fAllSize) )
		{
			ESLAssert( pLine->m_pSprite == NULL ) ;
			//
			SGLSpriteMessage *	pMsg = new SGLSpriteMessage ;
			pMsg->SetTextStyle( m_styleText ) ;
			pMsg->SetRubyFontStyle( m_styleText.fontRuby ) ;
			pMsg->AttachSkin( m_refSkin.GetReference() ) ;
			for ( int j = 0; j < SGLSpriteMessage::LinkInfo::statusCount; j ++ )
			{
				pMsg->SetLinkDecoration( j, m_styleText.decoLink[j] ) ;
			}
			pMsg->AddMessageXML( pLine->m_xmlLine ) ;
			pMsg->FlushMessage() ;
			pLine->m_pSprite = pMsg ;
			//
			SGLPoint	ptNext = pMsg->GetNextMessagePoint() ;
			//
			pLine->m_yLineTop = yNext ;
			pLine->m_nLineHeight = ptNext.y ;
			//
			if ( ptNext.x > m_styleText.context.rectWritable.left )
			{
				if ( m_styleText.context.pitchLine > 0 )
				{
					pLine->m_nLineHeight += m_styleText.context.pitchLine ;
				}
				else
				{
					pLine->m_nLineHeight += m_styleText.font.nSize ;
				}
			}
			//
			SGLSprite::AddChild( pMsg ) ;
		}
		if ( fUnderView )
		{
			if ( pLine->m_pSprite != NULL )
			{
				SGLSprite::DetachChild( pLine->m_pSprite ) ;
				delete	pLine->m_pSprite ;
				pLine->m_pSprite = NULL ;
			}
		}
		else
		{
			yNext = pLine->m_yLineTop + pLine->m_nLineHeight ;
			if ( pLine->m_pSprite != NULL )
			{
				if ( yNext - m_ptScroll.y > 0 )
				{
					pLine->m_pSprite->SetPosition
						( - m_ptScroll.x, pLine->m_yLineTop - m_ptScroll.y ) ;
				}
				else
				{
					SGLSprite::DetachChild( pLine->m_pSprite ) ;
					delete	pLine->m_pSprite ;
					pLine->m_pSprite = NULL ;
				}
			}
			if ( yNext - m_ptScroll.y >= m_sizeView.h )
			{
				fUnderView = true ;
			}
		}
	}
	PostUpdate() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::SetTextFont
	( const wchar_t * pwszFont, int nSize )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_strFontFace = pwszFont ;
	SGLSpriteText::SelectValidFont( m_strFontFace )  ;
	m_styleText.font.pszFace = m_strFontFace ;
	m_styleText.fontRuby.pszFace = m_strFontFace ;
	//
	UpdateTextView() ;
	PrepareTextDraw() ;
	Unlock() ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteSmartTextView::GetScrollPos
	( SGLSprite::ScrollDirection scrlDir )
{
	if ( scrlDir == scrollHorz )
	{
		return	m_ptScroll.x ;
	}
	else
	{
		return	m_ptScroll.y ;
	}
}

void SGLSpriteSmartTextView::SetScrollPos
	( int nPos, SGLSprite::ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( scrlDir == scrollHorz )
	{
		m_ptScroll.x = (int32_t) nPos ;
	}
	else
	{
		m_ptScroll.y = (int32_t) nPos ;
	}
	PrepareTextDraw() ;
	PostUpdate() ;
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteSmartTextView::GetScrollRange
	( SGLSprite::ScrollDirection scrlDir )
{
	int	nRange = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	if ( scrlDir == scrollHorz )
	{
	}
	else
	{
		nRange = EstimateScrollHeight() ;
	}
	Unlock() ;
	return	nRange ;
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::AttachScrollBar
	( SGLSprite * pScrollBar, SGLSprite::ScrollDirection scrlDir )
{
	if ( (scrlDir == scrollVert)
		|| (scrlDir == scrollDefault) )
	{
		m_refScrollBar = pScrollBar ;
		ReflectToScrollBar() ;
	}
}

void SGLSpriteSmartTextView::AttachTrackBar( SGLBasicForm::TrackBar * pScrollBar )
{
	m_refTrackBar = pScrollBar ;
	ReflectToScrollBar() ;
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteSmartTextView::DetachScrollBar
	( SGLSprite * pScrollBar, SGLSprite::ScrollDirection scrlDir )
{
	if ( (scrlDir == scrollVert)
		|| (scrlDir == scrollDefault) )
	{
		if ( m_refScrollBar.GetReference() == pScrollBar )
		{
			m_refScrollBar = nullptr ;
		}
	}
}

void SGLSpriteSmartTextView::DetachTrackBar( SGLBasicForm::TrackBar * pScrollBar )
{
	if ( m_refTrackBar.GetReference() == pScrollBar )
	{
		m_refTrackBar = nullptr ;
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteSmartTextView::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagSwiping )
	{
		int	dy = (int) eslRoundR64ToLInt( yPos - m_vLastSwipe.y ) ;
		if ( dy != 0 )
		{
			m_ptScroll.y -= dy ;
			//
			int	nRange = EstimateScrollHeight() ;
			if ( m_ptScroll.y + m_sizeView.h > nRange )
			{
				m_ptScroll.y = nRange - m_sizeView.h ;
			}
			if ( m_ptScroll.y < 0 )
			{
				m_ptScroll.y = 0 ;
			}
			//
			PrepareTextDraw() ;
			ReflectToScrollBar() ;
			//
			m_vLastSwipe.x = xPos ;
			m_vLastSwipe.y = yPos ;
		}
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteSmartTextView::OnMouseLeave( int64_t nFlags )
{
	if ( m_flagSwiping )
	{
		ReleaseMouseCapture() ;
		m_flagSwiping = false ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteSmartTextView::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	int	nLinePitch = (int) m_styleText.context.pitchLine ;
	if ( nLinePitch == 0 )
	{
		nLinePitch = (int) m_styleText.font.nSize ;
	}
	m_ptScroll.y += zDelta * nLinePitch / WheelDeltaUnit ;
	//
	int	nRange = EstimateScrollHeight() ;
	if ( m_ptScroll.y + m_sizeView.h > nRange )
	{
		m_ptScroll.y = nRange - m_sizeView.h ;
	}
	if ( m_ptScroll.y < 0 )
	{
		m_ptScroll.y = 0 ;
	}
	//
	PrepareTextDraw() ;
	ReflectToScrollBar() ;
	//
	return	true ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteSmartTextView::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
	//
	m_flagSwiping = true ;
	m_vLastSwipe.x = xPos ;
	m_vLastSwipe.y = yPos ;
	SetMouseCapture() ;
	//
	return	true ;
}

bool SGLSpriteSmartTextView::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
	//
	if ( m_flagSwiping )
	{
		ReleaseMouseCapture() ;
		m_flagSwiping = false ;
	}
	return	true ;
}

// 位置が移動した
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteSmartTextView::OnScroll
	( SGLSpriteScrollBar& scroll, int64_t codeNotify )
{
	LockTrace( __FILE__, __LINE__ ) ;
	int	nPos = scroll.GetScrollPos() ;
	m_ptScroll.y = nPos ;
	PrepareTextDraw() ;
	Unlock() ;
	return	false ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteSmartTextView::DuplicateObject( void )
{
	return	new SGLSpriteSmartTextView( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteSmartTextView::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.WriteString( m_strFontFace ) ;
	file.WriteString( m_strRubyFont ) ;
	file.WriteString( m_strProhibition ) ;
	m_styleText.SaveWithoutPointer( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	SString	strSkinID ;
	if ( posm != NULL )
	{
		strSkinID = posm->GetIdentityOf( (SGLObject*) m_refSkin.GetReference() ) ;
	}
	file.WriteString( strSkinID ) ;
	//
	uint32_t	nFlags = 0 ;
	if ( SGLSprite::IsBuffered() )
	{
		nFlags |= 0x01 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &m_sizeView, sizeof(SGLSize) ) ;
	file.Write( &m_ptScroll, sizeof(SGLPoint) ) ;
	file.WriteString( m_strXMLText ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteSmartTextView::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.ReadString( m_strFontFace ) ;
	file.ReadString( m_strRubyFont ) ;
	file.ReadString( m_strProhibition ) ;
	m_styleText.LoadWithoutPointer( file ) ;
	//
	m_styleText.font.pszFace = m_strFontFace ;
	m_styleText.context.pwszProhibition = m_strProhibition ;
	m_styleText.fontRuby.pszFace = m_strRubyFont ;
	//
	SString	strSkinID ;
	file.ReadString( strSkinID ) ;
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm != NULL )
	{
		m_refSkin.SetReference
			( ESLTypeCast<SGLSkinManager>( posm->GetObjectOf(strSkinID) ) ) ;
	}
	//
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	file.Read( &m_sizeView, sizeof(SGLSize) ) ;
	file.Read( &m_ptScroll, sizeof(SGLPoint) ) ;
	file.ReadString( m_strXMLText ) ;
	//
	if ( m_sizeView.IsEmpty() )
	{
		ReleaseView() ;
	}
	else
	{
		CreateView
			( (uint32_t) m_sizeView.w,
				(uint32_t) m_sizeView.h, ((nFlags & 0x01) != 0) ) ;
	}
	UpdateTextView() ;
	PrepareTextDraw() ;
	//
	return	sglErrSuccess ;
}



