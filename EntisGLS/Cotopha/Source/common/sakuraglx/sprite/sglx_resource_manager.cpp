
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_audio_decoding_player.h>
#include <sakuraglx/sprite/sglx_resource_manager.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>
#include <sakuraglx/sprite/sglx_sprite_frame.h>
#include <sakuraglx/sprite/sglx_sprite_text.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuraglx/sprite/sglx_sprite_edit.h>
#include <sakuraglx/sprite/sglx_sprite_progress_bar.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>
#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// リソース管理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLResourceProducer, SObject )
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::SGLResourceManager, SGLObject, SGLResourceProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLResourceManager::SGLResourceManager( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLResourceManager::~SGLResourceManager( void )
{
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLResourceManager::AddResourceAs
		( const wchar_t * pwszID, SObject * pRsrc )
{
	m_resources.SetAs( pwszID, pRsrc ) ;
	return	sglErrSuccess ;
}

// リソース削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLResourceManager::RemoveResourceAs( const wchar_t * pwszID )
{
	m_resources.RemoveAs( pwszID ) ;
	return	sglErrSuccess ;
}

// 全リソース削除
//////////////////////////////////////////////////////////////////////////////
void SGLResourceManager::RemoveAllResource( void )
{
	m_resources.RemoveAll() ;
}

// 参照されていないリソースを削除する
//////////////////////////////////////////////////////////////////////////////
void SGLResourceManager::CleanupResource( void )
{
	const size_t	nCount = m_resources.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const size_t	j = nCount - i - 1 ;
		SObject *	pObj = m_resources.GetAt( j ) ;
		if ( (pObj != NULL) && !pObj->IsObjectReferenced() )
		{
			m_resources.RemoveAt( j ) ;
		}
	}
}

// リソース取得
//////////////////////////////////////////////////////////////////////////////
SObject * SGLResourceManager::GetResourceAs( const wchar_t * pwszID )
{
	return	m_resources.GetAs( pwszID ) ;
}

// 画像リソース取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLResourceManager::GetImageAs( const wchar_t * pwszID )
{
	return	ESLTypeCast<SGLImageObject>( m_resources.GetAs( pwszID ) ) ;
}

// 音声リソース取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayer * SGLResourceManager::GetAudioAs( const wchar_t * pwszID )
{
	return	ESLTypeCast<SGLAudioPlayer>( m_resources.GetAs( pwszID ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// スキン管理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSkinManager, SGLResourceManager )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSkinManager::SGLSkinManager( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSkinManager::~SGLSkinManager( void )
{
}

// スキンファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::LoadSkinFile
	( const wchar_t * pwszFilePath, bool fStaticResource )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadSkinFile( *pFile, fStaticResource ) ;
}

SGLError SGLSkinManager::ReadSkinFile
	( SSystem::SFileInterface & file, bool fStaticResource )
{
	RemoveSkinResource() ;
	//
	// 書庫ファイルを開く
	//
	SFileInterface *	pFile = &file ;
	SSmartBuffer	sbuf ;
	if ( !file.IsSeekable() )
	{
		sbuf.ReadFromStream( file ) ;
		pFile = &sbuf ;
	}
	ERISA::SGLArchiveFile	arcfile ;
	if ( arcfile.OpenArchive( pFile, false ) )
	{
		return	sglErrFailed ;
	}
	//
	// system.inf を読み込む
	//
	SSmartPointer<SFileInterface>
		pFileSysInf = arcfile.NewOpenFile
						( L"system.inf", SFileOpener::shareRead ) ;
	if ( pFileSysInf == NULL )
	{
		return	sglErrFailed ;
	}
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( *pFileSysInf, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	pFileSysInf = NULL ;
	SXMLDocument *	pxmlSkin = xmlDoc.GetElementTagAs( L"skin" ) ;
	if ( pxmlSkin == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// <resource>
	//
	SXMLDocument *	pxmlResource = pxmlSkin->GetElementTagAs( L"resource" ) ;
	if ( pxmlResource != NULL )
	{
		const size_t	nCount = pxmlResource->GetElementsCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SXMLDocument *	pxmlTag = pxmlResource->GetElementAt( i ) ;
			if ( (pxmlTag == NULL)
				|| (pxmlTag->GetType() != SXMLDocument::typeTag) )
			{
				continue ;
			}
			SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			SString *	pstrSrc = pxmlTag->GetAttributeAs( L"src" ) ;
			if ( (pstrID == NULL) || (pstrSrc == NULL) )
			{
				continue ;
			}
			SSmartPointer<SFileInterface>
				pFileRsrc = arcfile.NewOpenFile
								( *pstrSrc, SFileOpener::shareRead ) ;
			if ( pFileRsrc == NULL )
			{
				continue ;
			}
			if ( fStaticResource )
			{
				SObject *	pObj = RealizeResource( *pxmlTag, *pFileRsrc ) ;
				if ( pObj != NULL )
				{
					AddResourceAs( *pstrID, pObj ) ;
				}
			}
			else
			{
				SByteBuffer *	pbufFile = new SByteBuffer ;
				pbufFile->ReadFromFile( *pFileRsrc ) ;
				//
				m_ssoaResource.SetAs( *pstrID, new SXMLDocument( *pxmlTag ) ) ;
				m_ssoaFile.SetAs( *pstrSrc, pbufFile ) ;
			}
		}
	}
	//
	// <declare_style>
	//
	SXMLDocument *	pxmlDeclStyle =
				pxmlSkin->GetElementTagAs( L"declare_style" ) ;
	if ( pxmlDeclStyle != NULL )
	{
		const size_t	nCount = pxmlDeclStyle->GetElementsCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SXMLDocument *	pxmlTag = pxmlDeclStyle->GetElementAt( i ) ;
			if ( (pxmlTag == NULL)
				|| (pxmlTag->GetType() != SXMLDocument::typeTag)
				|| (pxmlTag->GetTag() != L"style") )
			{
				continue ;
			}
			SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == NULL )
			{
				continue ;
			}
			m_ssoaStyle.SetAs( *pstrID, new SXMLDocument( *pxmlTag ) ) ;
		}
	}
	//
	// <page> or <window>
	//
	const size_t	nCount = pxmlSkin->GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = pxmlSkin->GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetType() != SXMLDocument::typeTag)
			|| ((pxmlTag->GetTag() != L"page")
					&& (pxmlTag->GetTag() != L"window")) )
		{
			continue ;
		}
		SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
		if ( pstrID == NULL )
		{
			continue ;
		}
		m_ssoaForm.SetAs( *pstrID, new SXMLDocument( *pxmlTag ) ) ;
	}
	return	sglErrSuccess ;
}

// スキンデータを削除する
//////////////////////////////////////////////////////////////////////////////
void SGLSkinManager::RemoveSkinResource( void )
{
	RemoveAllResource() ;
	m_ssoaResource.RemoveAll() ;
	m_ssoaFile.RemoveAll() ;
	m_ssoaStyle.RemoveAll() ;
	m_ssoaForm.RemoveAll() ;
}

// フォームを生成する
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateFormedSprite( const wchar_t * pwszID )
{
	SXMLDocument *	pxmlForm = GetFormAs( pwszID ) ;
	if ( pxmlForm == NULL )
	{
		return	NULL ;
	}
	SGLSpriteFormed *	pForm = new SGLSpriteFormed ;
	if ( PrepareFormedPage( *pForm, *pxmlForm ) )
	{
		delete	pForm ;
		return	NULL ;
	}
	if ( AddFormedPageItems( *pForm, *pxmlForm ) )
	{
		delete	pForm ;
		return	NULL ;
	}
	return	pForm ;
}

// フォームを構築する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::BuildFormedPage
	( SGLSpriteFormed& sprPage, const wchar_t * pwszID )
{
	SXMLDocument *	pxmlForm = GetFormAs( pwszID ) ;
	if ( pxmlForm == NULL )
	{
		return	sglErrFailed ;
	}
	if ( PrepareFormedPage( sprPage, *pxmlForm ) )
	{
		return	sglErrFailed ;
	}
	if ( AddFormedPageItems( sprPage, *pxmlForm ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// フォーム準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::PrepareFormedPage
	( SGLSpriteFormed& sprPage, SSystem::SXMLDocument& xmlForm )
{
	sprPage.DetachAllChildren() ;
	//
	if ( xmlForm.GetTag() == L"window" )
	{
		SString *	pstrBG = xmlForm.GetAttributeAs( L"bg" ) ;
		if ( pstrBG == NULL )
		{
			return	sglErrFailed ;
		}
		ImageDescription	imgdscBG ;
		if ( GetRichImageAs( imgdscBG, *pstrBG ) )
		{
			return	sglErrFailed ;
		}
		SGLImageInfo	imginfBG ;
		if ( (imgdscBG.pImage == NULL)
			|| imgdscBG.pImage->GetImageInfo( imginfBG ) )
		{
			return	sglErrFailed ;
		}
		sprPage.CreateBuffer
			( imgdscBG.rectImage.w,
				imgdscBG.rectImage.h, imginfBG.format ) ;
		//
		SGLSprite *	pSpriteBG = new SGLSprite ;
		pSpriteBG->SetID( L"ID_BG" ) ;
		pSpriteBG->ChangePriority( priorityBG ) ;
		pSpriteBG->AttachAnimation( imgdscBG.pImage, imgdscBG.pRect ) ;
		sprPage.AddSmartChild( pSpriteBG ) ;
	}
	else
	{
		if ( xmlForm.GetAttrStringAs( L"buffered", L"true" ) == L"true" )
		{
			uint32_t	width =
				(uint32_t) xmlForm.GetAttrRichIntegerAs( L"width", 0 ) ;
			uint32_t	height =
				(uint32_t) xmlForm.GetAttrRichIntegerAs( L"height", 0 ) ;
			if ( (width == 0) | (height == 0) )
			{
				return	sglErrFailed ;
			}
			sprPage.CreateBuffer( width, height ) ;
		}
		else
		{
			sprPage.ReleaseBuffer() ;
		}
		sprPage.SetPosition
			( (double) xmlForm.GetAttrRichIntegerAs( L"x", 0 ), 
				(double) xmlForm.GetAttrRichIntegerAs( L"y", 0 ) ) ;
	}
	sprPage.AttachSkin( this ) ;
	return	sglErrSuccess ;
}

// <button> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateButtonItemOf( const wchar_t * pwszStyleID )
{
	SXMLDocument *	pxmlStyle = GetStyleAs( pwszStyleID ) ;
	if ( pxmlStyle == NULL )
	{
		return	NULL ;
	}
	SGLSpriteButton::ButtonStyle	style ;
	SString							strFont[SGLSpriteButton::statusCount] ;
	SGLSpriteButton::ParseButtonStyle
			( *this, style, &strFont[0], *pxmlStyle ) ;
	//
	SGLSize	sizeButton =
		style.imgdscButton[SGLSpriteButton::statusNormal].rectImage.GetSize() ;
	//
	SGLAudioPlayer *	pFocusSE = GetAudioAs( L"IDSE_BUTTON_FOCUS" ) ;
	SGLAudioPlayer *	pPushedSE = GetAudioAs( L"IDSE_BUTTON_PUSHED" ) ;
	if ( pFocusSE != NULL )
	{
		pFocusSE->SetVolumeLine( SGLAudioPlayer::lineSystem ) ;
	}
	if ( pPushedSE != NULL )
	{
		pPushedSE->SetVolumeLine( SGLAudioPlayer::lineSystem ) ;
	}
	//
	SGLSpriteButton *	pButton = new SGLSpriteButton ;
	pButton->SetButtonStyle( style ) ;
	pButton->SetButtonSize( sizeButton ) ;
	pButton->AttachSoundEffect( pFocusSE, pPushedSE ) ;
	return	pButton ;
}

// フォームアイテム追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::AddFormedPageItems
	( SGLSpriteFormed& sprPage, SSystem::SXMLDocument& xmlForm )
{
	size_t	nCount = xmlForm.GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = xmlForm.GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetType() != SXMLDocument::typeTag) )
		{
			continue ;
		}
		SGLSprite *	pItem = NULL ;
		if ( pxmlTag->GetTag() == L"image" )
		{
			pItem = CreateImageItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"rectangle" )
		{
			pItem = CreateRectangleItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"static_frame" )
		{
			pItem = CreateStaticFrameItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"static_text" )
		{
			pItem = CreateStaticTextItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"smart_text" )
		{
			pItem = CreateSmartTextItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"edit_text" )
		{
			pItem = CreateEditTextItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"progress_bar" )
		{
			pItem = CreateProgressBarItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"button" )
		{
			pItem = CreateButtonItem( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"scroll_bar" )
		{
			pItem = CreateScrollBarItem( *pxmlTag ) ;
		}
		if ( pItem == NULL )
		{
			continue ;
		}
		const S3DDVector &	vPos = pItem->GetPosition() ;
		pItem->SetPosition
			( (double) pxmlTag->GetAttrRichIntegerAs
							( L"x", (int64_t) vPos.x ),
				(double) pxmlTag->GetAttrRichIntegerAs
							( L"y", (int64_t) vPos.y ) ) ;
		pItem->ChangePriority( priorityFirst + (int32_t) i * priorityStep ) ;
		pItem->SetID( pxmlTag->GetAttrStringAs( L"id", NULL ) ) ;
		if ( pxmlTag->GetAttrStringAs( L"group", L"true" ) != L"true" )
		{
			pItem->ModifyUIFlag( SGLSprite::uiGroupMember ) ;
		}
		if ( pxmlTag->GetAttrStringAs( L"tab_stop", NULL ) == L"true" )
		{
			pItem->ModifyUIFlag( SGLSprite::uiFocusable ) ;
		}
		sprPage.AddSmartChild( pItem ) ;
		//
		if ( pItem->GetUIFlag() & SGLSprite::uiFocusable )
		{
			sprPage.ModifyUIFlag( SGLSprite::uiFocusable ) ;
		}
		//
		const SString *	pstrScrollBar = pxmlTag->GetAttributeAs( L"scroll_bar" ) ;
		if ( pstrScrollBar != NULL )
		{
			SGLSprite *	psprScrollBar = sprPage.GetItemAs( *pstrScrollBar ) ;
			if ( psprScrollBar != NULL )
			{
				pItem->AttachScrollBar( psprScrollBar ) ;
				//
				SGLSpriteScrollBar *		pScrollBar =
					ESLTypeCast<SGLSpriteScrollBar>( psprScrollBar ) ;
				SGLSpriteScrollListener *	pListener =
					ESLTypeCast<SGLSpriteScrollListener>( pItem ) ;
				if ( (pScrollBar != NULL) && (pListener != NULL) )
				{
					pScrollBar->AttachScrollListener( pListener ) ;
				}
			}
		}
		//
		SXMLDocument *	pxmlCommands =
					pxmlTag->GetElementTagAs( L"command" ) ;
		if ( pxmlCommands != NULL )
		{
			size_t	nCommands = pxmlCommands->GetElementsCount() ;
			for ( size_t i = 0; i < nCommands; i ++ )
			{
				SXMLDocument *	pxmlCommand = pxmlCommands->GetElementAt( i ) ;
				if ( (pxmlCommand != NULL)
					&& (pxmlCommand->GetType() == SXMLDocument::typeTag) )
				{
					pItem->InvokeCommand( *pxmlCommand ) ;
				}
			}
		}
	}
	return	sglErrSuccess ;
}

// <rectangle> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateRectangleItem
						( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteRectangle *	pSprite = new SGLSpriteRectangle ;
	SGLSpriteRectangle::RectStyle	style ;
	SGLSpriteRectangle::ParseRectStyle( style, xmlItem ) ;
	pSprite->SetRectangleStyle( style ) ;
	return	pSprite ;
}

// <image> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateImageItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSprite *	pSprite = new SGLSprite ;
	SString *	pstrRsrc = xmlItem.GetAttributeAs( L"rsrc" ) ;
	if ( pstrRsrc != NULL )
	{
		ImageDescription	imgdsc ;
		if ( !GetRichImageAs( imgdsc, *pstrRsrc ) )
		{
			pSprite->AttachAnimation( imgdsc.pImage, imgdsc.pRect ) ;
		}
	}
	return	pSprite ;
}

// <static_frame> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateStaticFrameItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteFrame::FrameStyle	style ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteFrame::ParseFrameStyle( *this, style, *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteFrame::ParseFrameStyle( *this, style, *pxmlStyle ) ;
	}
	//
	SGLSize	sizeFrame ;
	sizeFrame.w = (int32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ;
	sizeFrame.h = (int32_t) xmlItem.GetAttrRichIntegerAs( L"height", 0 ) ;
	if ( sizeFrame.IsEmpty() )
	{
		return	NULL ;
	}
	//
	SGLSpriteFrame *	pFrame = new SGLSpriteFrame ;
	pFrame->SetFrameStyle( style ) ;
	pFrame->SetFrameSize( sizeFrame ) ;
	return	pFrame ;
}

// <static_text> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateStaticTextItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteText::TextStyle	style ;
	SString						strFontFace ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteText::ParseTextStyle( style, strFontFace, *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteText::ParseTextStyle( style, strFontFace, *pxmlStyle ) ;
	}
	//
	SGLSize	sizeText ;
	sizeText.w = (int32_t) xmlItem.GetAttrRichIntegerAs
					( L"width", style.context.rectWritable.GetWidth() ) ;
	sizeText.h = (int32_t) xmlItem.GetAttrRichIntegerAs
					( L"height", style.context.rectWritable.GetHeight() ) ;
	if ( sizeText.IsEmpty() )
	{
		return	NULL ;
	}
	style.context.rectWritable.SetWidth( sizeText.w ) ;
	style.context.rectWritable.SetHeight( sizeText.h ) ;
	//
	SGLSpriteText *	pText = new SGLSpriteText ;
	pText->SetTextStyle( style ) ;
	pText->SetText( xmlItem.GetAttrStringAs( L"text" ) ) ;
	return	pText ;
}

// <smart_text> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateSmartTextItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteSmartTextView::TextStyle	style ;
	SString								strFontFace, strRubyFont ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteSmartTextView::ParseRichTextStyle
			( style, strFontFace, strRubyFont, *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteSmartTextView::ParseRichTextStyle
			( style, strFontFace, strRubyFont, *pxmlStyle ) ;
	}
	//
	SGLSize	sizeText ;
	sizeText.w = (int32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ;
	sizeText.h = (int32_t) xmlItem.GetAttrRichIntegerAs( L"height", 0 ) ;
	if ( sizeText.IsEmpty() )
	{
		return	NULL ;
	}
	style.context.rectWritable.SetWidth( sizeText.w ) ;
	style.context.rectWritable.SetHeight( 0x7FFF ) ;
	//
	SGLSpriteSmartTextView *	pText = new SGLSpriteSmartTextView ;
	pText->CreateView( (uint32_t) sizeText.w, (uint32_t) sizeText.h ) ;
	pText->SetTextStyle( style ) ;
	//
	SString *	pstrText = xmlItem.GetAttributeAs( L"text" ) ;
	if ( pstrText != NULL )
	{
		pText->SetText( *pstrText ) ;
	}
	else
	{
		SXMLDocument *	pxmlText = xmlItem.GetElementTagAs( L"text" ) ;
		if ( pxmlText != NULL )
		{
			SSmartBuffer	sbuf ;
			pxmlText->FormatXMLElements( sbuf, 0, Charset::encodingUTF8 ) ;
			//
			SStringParser	sparsXML ;
			sbuf.Seek( 0 ) ;
			sparsXML.ReadTextFile( sbuf, Charset::encodingUTF8 ) ;
			//
			pText->SetText( sparsXML ) ;
		}
	}
	return	pText ;
}

// <edit_text> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateEditTextItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteEdit::EditStyle	style ;
	SString						strFontFace, strIMEFontFace ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		if ( pxmlStyle->GetAttrStringAs( L"type", L"edit_text2" ) == L"edit_text" )
		{
			SGLSpriteEdit::ParseTextStyle_CompatibleGLS3
				( style, strFontFace, strIMEFontFace, *pxmlStyle ) ;
		}
		else
		{
			SGLSpriteEdit::ParseTextStyle
				( style, strFontFace, strIMEFontFace, *pxmlStyle ) ;
		}
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteEdit::ParseTextStyle
			( style, strFontFace, strIMEFontFace, *pxmlStyle ) ;
	}
	//
	SGLSize	sizeText ;
	sizeText.w = (int32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ;
	sizeText.h = (int32_t) xmlItem.GetAttrRichIntegerAs( L"height", 0 ) ;
	if ( sizeText.IsEmpty() )
	{
		return	NULL ;
	}
	style.context.rectWritable.SetWidth( sizeText.w ) ;
	style.context.rectWritable.SetHeight( sizeText.h ) ;
	//
	SGLSpriteEdit *	pEdit = new SGLSpriteEdit ;
	pEdit->CreateBuffer( sizeText.w, sizeText.h ) ;
	pEdit->SetEditStyle( style ) ;
	pEdit->SetEditItemName( xmlItem.GetAttrStringAs( L"item_name" ) ) ;
	pEdit->SetText( xmlItem.GetAttrStringAs( L"text" ) ) ;
	return	pEdit ;
}

// <progress_bar> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateProgressBarItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteProgressBar::BarStyle	style ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteProgressBar::ParseBarStyle( *this, style, *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteProgressBar::ParseBarStyle( *this, style, *pxmlStyle ) ;
	}
	//
	SGLSize	sizeBar ;
	sizeBar.w = (int32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ;
	sizeBar.h = sizeBar.w ;
	//
	SGLSpriteProgressBar *	pProgress = new SGLSpriteProgressBar ;
	pProgress->SetBarStyle( style ) ;
	pProgress->SetBarSize( sizeBar ) ;
	return	pProgress ;
}

// <button> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateButtonItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteButton::ButtonStyle	style ;
	SString							strFont[SGLSpriteButton::statusCount] ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteButton::ParseButtonStyle
				( *this, style, &strFont[0], *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteButton::ParseButtonStyle
				( *this, style, &strFont[0], *pxmlStyle ) ;
	}
	//
	SGLSize	sizeButton ;
	sizeButton.w = (int32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ;
	sizeButton.h = (int32_t) xmlItem.GetAttrRichIntegerAs( L"height", 0 ) ;
	//
	SGLAudioPlayer *	pFocusSE =
		GetAudioAs( xmlItem.GetAttrStringAs
						( L"focus_se", L"IDSE_BUTTON_FOCUS" ) ) ;
	SGLAudioPlayer *	pPushedSE =
		GetAudioAs( xmlItem.GetAttrStringAs
						( L"pushed_se", L"IDSE_BUTTON_PUSHED" ) ) ;
	if ( pFocusSE != NULL )
	{
		pFocusSE->SetVolumeLine( SGLAudioPlayer::lineSystem ) ;
	}
	if ( pPushedSE != NULL )
	{
		pPushedSE->SetVolumeLine( SGLAudioPlayer::lineSystem ) ;
	}
	//
	SGLSpriteButton *	pButton = new SGLSpriteButton ;
	pButton->SetButtonStyle( style ) ;
	pButton->SetButtonSize( sizeButton ) ;
	pButton->SetText( xmlItem.GetAttrStringAs( L"text" ) ) ;
	pButton->AttachSoundEffect( pFocusSE, pPushedSE ) ;
	return	pButton ;
}

// <scroll_bar> アイテム生成
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSkinManager::CreateScrollBarItem( SSystem::SXMLDocument& xmlItem )
{
	SGLSpriteScrollBar::BarStyle	style ;
	//
	SXMLDocument *	pxmlStyle =
		GetStyleAs( xmlItem.GetAttrStringAs( L"style" ) ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteScrollBar::ParseScrollBarStyle( *this, style, *pxmlStyle ) ;
	}
	pxmlStyle = xmlItem.GetElementTagAs( L"style" ) ;
	if ( pxmlStyle != NULL )
	{
		SGLSpriteScrollBar::ParseScrollBarStyle( *this, style, *pxmlStyle ) ;
	}
	//
	SGLSpriteScrollBar *	pScroll = new SGLSpriteScrollBar ;
	pScroll->SetScrollBarStyle( style ) ;
	pScroll->SetScrollBarSize
		( (uint32_t) xmlItem.GetAttrRichIntegerAs( L"width", 0 ) ) ;
	return	pScroll ;
}

// リソースをリアライズする
//////////////////////////////////////////////////////////////////////////////
SObject * SGLSkinManager::RealizeResource
	( SSystem::SXMLDocument & xmlRsrc, SSystem::SFileInterface & file )
{
	SFileInterface *	pFile = &file ;
#if	defined(__COTOPHA__)
	if ( file.GetFileObject() == NULL )
	{
		SByteBuffer	sbuf ;
		sbuf.ReadFromFile( file ) ;
		pFile = &sbuf ;
	}
#endif
	if ( xmlRsrc.GetTag() == L"sound" )
	{
		uint64_t nFlags = SGLAudioPlayerInterface::modeOpenAuto ;
		SString	strWay = xmlRsrc.GetAttrStringAs( L"way" ) ;
		if ( strWay == L"static" )
		{
			nFlags = SGLAudioPlayerInterface::modeOpenStatic ;
		}
		else if ( strWay == L"auto" )
		{
			nFlags = SGLAudioPlayerInterface::modeOpenAutoStatic ;
		}
		SGLAudioPlayer *	pPlayer = new SGLAudioPlayer ;
		if ( pPlayer->Create( pFile, false, nFlags ) )
		{
			delete	pPlayer ;
			return	NULL ;
		}
		return	pPlayer ;
	}
	else if ( xmlRsrc.GetTag() == L"image" )
	{
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->ReadImage( pFile ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		return	pImage ;
	}
	return	NULL ;
}

// スタイルを取得する
//////////////////////////////////////////////////////////////////////////////
SSystem::SXMLDocument *
		SGLSkinManager::GetStyleAs( const wchar_t * pwszID ) const
{
	return	m_ssoaStyle.GetAs( pwszID ) ;
}

// フォームを取得する
//////////////////////////////////////////////////////////////////////////////
SSystem::SXMLDocument *
		SGLSkinManager::GetFormAs( const wchar_t * pwszID ) const
{
	return	m_ssoaForm.GetAs( pwszID ) ;
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::AddResourceAs
		( const wchar_t * pwszID, SObject * pRsrc )
{
	m_csSync.Lock() ;
	m_resources.SetAs( pwszID, pRsrc ) ;
	m_csSync.Unlock() ;
	return	sglErrSuccess ;
}

// リソース削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::RemoveResourceAs( const wchar_t * pwszID )
{
	m_csSync.Lock() ;
	m_resources.RemoveAs( pwszID ) ;
	m_csSync.Unlock() ;
	return	sglErrSuccess ;
}

// 全リソース削除
//////////////////////////////////////////////////////////////////////////////
void SGLSkinManager::RemoveAllResource( void )
{
	m_csSync.Lock() ;
	m_resources.RemoveAll() ;
	m_csSync.Unlock() ;
}

// 参照されていないリソースを削除する
//////////////////////////////////////////////////////////////////////////////
void SGLSkinManager::CleanupResource( void )
{
	m_csSync.Lock() ;
	SGLResourceManager::CleanupResource() ;
	m_csSync.Unlock() ;
}

// リソース取得
//////////////////////////////////////////////////////////////////////////////
SObject * SGLSkinManager::GetResourceAs( const wchar_t * pwszID )
{
	SObject *	pObj ;
	m_csSync.Lock() ;
	pObj = m_resources.GetAs( pwszID ) ;
	if ( pObj == NULL )
	{
		SXMLDocument *	pxmlRsrc = m_ssoaResource.GetAs( pwszID ) ;
		if ( pxmlRsrc != NULL )
		{
			SString *	pstrSrc = pxmlRsrc->GetAttributeAs( L"src" ) ;
			if ( pstrSrc != NULL )
			{
				SFileInterface *	pFile = m_ssoaFile.GetAs( *pstrSrc ) ;
				if ( pFile != NULL )
				{
					pFile->Seek( 0 ) ;
					//
					pObj = RealizeResource( *pxmlRsrc, *pFile ) ;
					if ( pObj != NULL )
					{
						m_resources.SetAs( pwszID, pObj ) ;
					}
				}
			}
		}
	}
	m_csSync.Unlock() ;
	return	pObj ;
}

// 画像リソース取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSkinManager::GetImageAs( const wchar_t * pwszID )
{
	return	ESLTypeCast<SGLImageObject>( GetResourceAs( pwszID ) ) ;
}

// 音声リソース取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayer * SGLSkinManager::GetAudioAs( const wchar_t * pwszID )
{
	return	ESLTypeCast<SGLAudioPlayer>( GetResourceAs( pwszID ) ) ;
}

// 画像リソース取得（矩形指定を含む）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSkinManager::GetRichImageAs
	( SGLSkinManager::ImageDescription& imgdsc, const wchar_t * pwszID )
{
	SStringParser	sparsDesc = pwszID ;
	SString	strImageID ;
	sparsDesc.NextToken( strImageID ) ;
	imgdsc.pImage = GetImageAs( strImageID ) ;
	imgdsc.pRect = NULL ;
	imgdsc.rectImage.Clear() ;
	if ( imgdsc.pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImageInfo	imginf ;
	if ( !imgdsc.pImage->GetImageInfo( imginf ) )
	{
		imgdsc.rectImage.x = 0 ;
		imgdsc.rectImage.y = 0 ;
		imgdsc.rectImage.w = imginf.width ;
		imgdsc.rectImage.h = imginf.height ;
	}
	if ( (sparsDesc.HasToComeChar( L":" ) == L':')
		&& sparsDesc.HasToComeToken( L"RECT" )
		&& (sparsDesc.HasToComeChar( L"(" ) == L'(') )
	do
	{
		int	typeNum ;
		typeNum = sparsDesc.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			break ;
		}
		imgdsc.rectImage.x =
				(int32_t) sparsDesc.NextInteger( typeNum ) ;
		//
		if ( sparsDesc.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		typeNum = sparsDesc.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			break ;
		}
		imgdsc.rectImage.y =
				(int32_t) sparsDesc.NextInteger( typeNum ) ;
		//
		if ( sparsDesc.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		typeNum = sparsDesc.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			break ;
		}
		imgdsc.rectImage.w =
				(int32_t) sparsDesc.NextInteger( typeNum ) ;
		//
		if ( sparsDesc.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		typeNum = sparsDesc.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			break ;
		}
		imgdsc.rectImage.h =
				(int32_t) sparsDesc.NextInteger( typeNum ) ;
		//
		if ( !imgdsc.rectImage.IsEmpty() )
		{
			imgdsc.pRect = &imgdsc.rectImage ;
		}
	}
	while ( false ) ;
	return	sglErrSuccess ;
}

SGLSkinManager::ImageDescription::ImageDescription
	( const SGLSkinManager::ImageDescription& src )
{
	pImage = src.pImage ;
	pRect = src.pRect ;
	rectImage = src.rectImage ;
	if ( pRect != NULL )
	{
		pRect = &rectImage ;
	}
}

const SGLSkinManager::ImageDescription&
	SGLSkinManager::ImageDescription::operator =
		( const SGLSkinManager::ImageDescription& src )
{
	pImage = src.pImage ;
	pRect = src.pRect ;
	rectImage = src.rectImage ;
	if ( pRect != NULL )
	{
		pRect = &rectImage ;
	}
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// 簡易フォーム
//////////////////////////////////////////////////////////////////////////////

// 画像セット
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLBasicForm::ImageSet::FindFrameAs( const wchar_t * pwszID ) const
{
	return	m_aFrameIDs.FindIndex( pwszID ) ;
}


// インタラクティブ・リスナ
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::ItemInteractive, ESLObject )

bool SGLBasicForm::ItemInteractive::OnPostMessage
		( Item * pItem, int nParam, const wchar_t * pwszOpt )
{
	return	false ;
}

void SGLBasicForm::ItemInteractive::OnMouseMove
		( Item * pItem, const S2DVector& vLocal )
{
}

void SGLBasicForm::ItemInteractive::OnMouseLeave( Item * pItem )
{
}

bool SGLBasicForm::ItemInteractive::OnMouseWheel
		( Item * pItem, const S2DVector& vLocal, float32_t zDelta )
{
	return	false ;
}

bool SGLBasicForm::ItemInteractive::OnClickDown
		( Item * pItem, const S2DVector& vLocal, MouseButton button )
{
	return	false ;
}

bool SGLBasicForm::ItemInteractive::OnClickUp
		( Item * pItem, const S2DVector& vLocal, MouseButton button )
{
	return	false ;
}


// スクロール・リスナ
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::ScrollListener, SObject )


// タイマー・リスナ
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::TimerListener, SObject )

SGLBasicForm::TimerListener::Result
	SGLBasicForm::TimerListener::OnTimer
			( SGLBasicForm * pForm, uint32_t msecPast )
{
	return	resultTerminate ;
}

SGLBasicForm::TimerListener::Result
	SGLBasicForm::TimerListener::OnCancel( SGLBasicForm * pForm )
{
	return	resultTerminate ;
}

SGLBasicForm::TimerListener::Result
	SGLBasicForm::TimerListener::OnFinish( SGLBasicForm * pForm )
{
	return	resultTerminate ;
}


// アニメーション
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::ItemAnimation, TimerListener )

SGLBasicForm::ItemAnimation::ItemAnimation( SGLBasicForm::Item * pItem )
	: m_refItem( pItem ), m_flagLoop( false ), m_iInterval( 0 ), m_msecCurrent( 0 )
{
	ESLAssert( pItem != NULL ) ;
	m_bzMove.Add( pItem->GetPosition() ) ;
	m_bzZoom.Add( pItem->GetZoom() ) ;
	m_bzRotation.Add( pItem->GetRotation() ) ;
	m_bzTransparency.Add( (float32_t) pItem->GetTransparency() / 256.0f ) ;
}

SGLBasicForm::Item * SGLBasicForm::ItemAnimation::GetItem( void ) const
{
	return	m_refItem.GetRef<SGLBasicForm::Item>() ;
}

bool SGLBasicForm::ItemAnimation::GetLoopFlag( void ) const
{
	return	m_flagLoop ;
}

void SGLBasicForm::ItemAnimation::SetLoopFlag( bool flagLoop )
{
	m_flagLoop = flagLoop ;
}

void SGLBasicForm::ItemAnimation::AddLinear
	( uint32_t msecDuration,
		const S2DVector* pMove,
		const uint32_t * pTransparency,
		const S2DVector * pZoom,
		const float32_t * pRotation,
		float32_t fpSpeed0, float32_t fpSpeed1 )
{
	size_t	nCount = m_bzMove.GetLength() ;
	ESLAssert( nCount >= 1 ) ;
	ESLAssert( m_aDurations.GetLength() * 3 + 1 == nCount ) ;
	ESLAssert( m_bzZoom.GetLength() == nCount ) ;
	ESLAssert( m_bzRotation.GetLength() == nCount ) ;
	ESLAssert( m_bzTransparency.GetLength() == nCount ) ;
	//
	m_aDurations.Add( msecDuration ) ;
	//
	if ( pMove != NULL )
	{
		if ( m_bzMove.GetLength() == 1 )
		{
			m_bzMove.SetLine
				( m_bzMove.At(0), *pMove, fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzMove.AddLine( *pMove, fpSpeed0, fpSpeed1 ) ;
		}
	}
	else
	{
		if ( m_bzMove.GetLength() == 1 )
		{
			m_bzMove.SetLine
				( m_bzMove.At(0), m_bzMove.At(0), fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzMove.AddLine
				( m_bzMove.At(nCount - 1), fpSpeed0, fpSpeed1 ) ;
		}
	}
	if ( pZoom != NULL )
	{
		if ( m_bzZoom.GetLength() == 1 )
		{
			m_bzZoom.SetLine
				( m_bzZoom.At(0), *pZoom, fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzZoom.AddLine( *pZoom, fpSpeed0, fpSpeed1 ) ;
		}
	}
	else
	{
		if ( m_bzZoom.GetLength() == 1 )
		{
			m_bzZoom.SetLine
				( m_bzZoom.At(0), m_bzZoom.At(0), fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzZoom.AddLine
				( m_bzZoom.At(nCount - 1), fpSpeed0, fpSpeed1 ) ;
		}
	}
	if ( pRotation != NULL )
	{
		if ( m_bzRotation.GetLength() == 1 )
		{
			m_bzRotation.SetLine
				( m_bzRotation.At(0), *pRotation, fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzRotation.AddLine( *pRotation, fpSpeed0, fpSpeed1 ) ;
		}
	}
	else
	{
		if ( m_bzRotation.GetLength() == 1 )
		{
			m_bzRotation.SetLine
				( m_bzRotation.At(0), m_bzRotation.At(0), fpSpeed0, fpSpeed1 ) ;
		}
		else
		{
			m_bzRotation.AddLine
				( m_bzRotation.At(nCount - 1), fpSpeed0, fpSpeed1 ) ;
		}
	}
	if ( pTransparency != NULL )
	{
		if ( m_bzTransparency.GetLength() == 1 )
		{
			m_bzTransparency.SetLine
				( m_bzTransparency.At(0),
					(float32_t) *pTransparency / 256.0f, 1.0f, 1.0f ) ;
		}
		else
		{
			m_bzTransparency.AddLine
				( (float32_t) *pTransparency / 256.0f, 1.0f, 1.0f ) ;
		}
	}
	else
	{
		if ( m_bzTransparency.GetLength() == 1 )
		{
			m_bzTransparency.SetLine
				( m_bzTransparency.At(0),
					m_bzTransparency.At(0), 1.0f, 1.0f ) ;
		}
		else
		{
			m_bzTransparency.AddLine
				( m_bzTransparency.At(nCount - 1), 1.0f, 1.0f ) ;
		}
	}
}

void SGLBasicForm::ItemAnimation::AddBezier3Points
	( uint32_t msecDuration,
		const S2DVector* pMove,
		const float32_t * pTransparency,
		const S2DVector * pZoom,
		const float32_t * pRotation )
{
	size_t	nCount = m_bzMove.GetLength() ;
	ESLAssert( nCount >= 1 ) ;
	ESLAssert( m_aDurations.GetLength() * 3 + 1 == nCount ) ;
	ESLAssert( m_bzZoom.GetLength() == nCount ) ;
	ESLAssert( m_bzRotation.GetLength() == nCount ) ;
	ESLAssert( m_bzTransparency.GetLength() == nCount ) ;
	//
	m_aDurations.Add( msecDuration ) ;
	//
	if ( pMove != NULL )
	{
		m_bzMove.AddArray( pMove, 3 ) ;
	}
	else
	{
		m_bzMove.AddLine( m_bzMove.At(nCount - 1), 0.0, 0.0 ) ;
	}
	if ( pZoom != NULL )
	{
		m_bzZoom.AddArray( pZoom, 3 ) ;
	}
	else
	{
		m_bzZoom.AddLine( m_bzZoom.At(nCount - 1), 0.0, 0.0 ) ;
	}
	if ( pRotation != NULL )
	{
		m_bzRotation.AddArray( pRotation, 3 ) ;
	}
	else
	{
		m_bzRotation.AddLine( m_bzRotation.At(nCount - 1), 0.0, 0.0 ) ;
	}
	if ( pTransparency != NULL )
	{
		m_bzTransparency.AddArray( pTransparency, 3 ) ;
	}
	else
	{
		m_bzTransparency.AddLine( m_bzTransparency.At(nCount - 1), 1.0f, 1.0f ) ;
	}
}

SGLBasicForm::TimerListener::Result
	SGLBasicForm::ItemAnimation::OnTimer( SGLBasicForm * pForm, uint32_t msecPast )
{
	SGLBasicForm::Item *	pItem = GetItem() ;
	if ( pItem == NULL )
	{
		return	resultTerminate ;
	}
	bool	flagLoop = false ;
	m_msecCurrent += msecPast ;
	for ( ; ; )
	{
		if ( m_iInterval >= m_aDurations.GetLength() )
		{
			if ( !m_flagLoop || (m_aDurations.GetLength() == 0) )
			{
				OnFinish( pForm ) ;
				return	resultTerminate ;
			}
			if ( flagLoop )
			{
				break ;
			}
			m_iInterval = 0 ;
			flagLoop = true ;
		}
		if ( m_msecCurrent < m_aDurations.At(m_iInterval) )
		{
			break ;
		}
		m_msecCurrent -= m_aDurations.At(m_iInterval ++) ;
	}
	const float32_t	t = (float32_t) m_msecCurrent
						/ (float32_t) m_aDurations.At(m_iInterval) ;
	pItem->SetPosition( m_bzMove.PointAt( t, m_iInterval ) ) ;
	pItem->SetZoom( m_bzZoom.PointAt( t, m_iInterval ) ) ;
	pItem->SetRotation( m_bzRotation.PointAt( t, m_iInterval ) ) ;
	pItem->SetTransparency
		( (uint32_t) esl_roundfi( esl_fmaxf
			( m_bzTransparency.PointAt( t, m_iInterval ), 0.0f ) * 256.0f ) ) ;
	return	resultContinue ;
}

SGLBasicForm::TimerListener::Result
	SGLBasicForm::ItemAnimation::OnFinish( SGLBasicForm * pForm )
{
	SGLBasicForm::Item *	pItem = GetItem() ;
	if ( pItem != NULL )
	{
		pItem->SetPosition( *m_bzMove.GetLastAt(0) ) ;
		pItem->SetZoom( *m_bzZoom.GetLastAt(0) ) ;
		pItem->SetRotation( *m_bzRotation.GetLastAt(0) ) ;
		pItem->SetTransparency
			( (uint32_t) esl_roundfi( esl_fmaxf
				( *m_bzTransparency.GetLastAt(0) * 256.0f, 0.0f ) ) ) ;
	}
	return	resultTerminate ;
}

SSystem::SArray<uint32_t>& SGLBasicForm::ItemAnimation::Intervals( void )
{
	return	m_aDurations ;
}

SGLBezierCurves<S2DVector>& SGLBasicForm::ItemAnimation::PositionBezier( void )
{
	return	m_bzMove ;
}

SGLBezierCurves<S2DVector>& SGLBasicForm::ItemAnimation::ZoomingBezier( void )
{
	return	m_bzZoom ;
}

SGLBezierCurves<float32_t,float32_t>& SGLBasicForm::ItemAnimation::RotationBezier( void )
{
	return	m_bzRotation ;
}

SGLBezierCurves<float32_t,float32_t>& SGLBasicForm::ItemAnimation::TransparencyBezier( void )
{
	return	m_bzTransparency ;
}


// アイテム基底
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::Item, SObject )

SGLBasicForm::Item::Item( void )
	: m_pParentForm(NULL), m_pInteractive(NULL),
		m_id(NULL), m_fVisible(true), m_fDisabled(false),
		m_nTransparency(0),
		m_colorEffect(colorNoEffect), m_rgbEffect(0),
		m_vPos(0.0f, 0.0f), m_vCenter(0.0f, 0.0f),
		m_vZoom(1.0f, 1.0f), m_degRotation(0.0f)
{
}

// フォーム
SGLBasicForm * SGLBasicForm::Item::GetParentForm( void ) const
{
	return	m_pParentForm ;
}

// リスナ
SGLBasicForm::ItemInteractive * SGLBasicForm::Item::GetInteractive( void )
{
	return	m_pInteractive ;
}

void SGLBasicForm::Item::AttachInteractive( SGLBasicForm::ItemInteractive * pInteractive )
{
	m_pInteractive = pInteractive ;
}

// アイテム ID
const wchar_t * SGLBasicForm::Item::GetID( void ) const
{
	return	m_id ;
}

void SGLBasicForm::Item::AttachID( const wchar_t * pwszID )
{
	m_id = pwszID ;
}

// 元のアイテム位置とサイズ
const SGLImageRect& SGLBasicForm::Item::GetItemOrgRect( void ) const
{
	return	m_rectOrg ;
}

void SGLBasicForm::Item::SetItemOrgRect( const SGLImageRect& rect )
{
	m_rectOrg = rect ;
}

// 座標
const S2DVector& SGLBasicForm::Item::GetPosition( void ) const
{
	return	m_vPos ;
}

const S2DVector& SGLBasicForm::Item::GetCenter( void ) const
{
	return	m_vCenter ;
}

void SGLBasicForm::Item::SetPosition( const S2DVector& vPos )
{
	m_vPos = vPos ;
}

void SGLBasicForm::Item::SetCenter( const S2DVector& vCenter )
{
	m_vCenter = vCenter ;
}

// 拡大と回転
const S2DVector& SGLBasicForm::Item::GetZoom( void ) const
{
	return	m_vZoom ;
}

float32_t SGLBasicForm::Item::GetRotation( void ) const
{
	return	m_degRotation ;
}

void SGLBasicForm::Item::SetZoom( const S2DVector& vZoom )
{
	m_vZoom = vZoom ;
}

void SGLBasicForm::Item::SetRotation( float32_t degRot )
{
	m_degRotation = degRot ;
}

// アフィン行列
SGLAffine SGLBasicForm::Item::GetAffine( void ) const
{
	SGLAffine	affineRot( 1.0f, 0.0f, m_vPos.x,
							0.0f, 1.0f, m_vPos.y ) ;
	SGLAffine	affineZoom( m_vZoom.x, 0.0f, - m_vCenter.x * m_vZoom.x,
							0.0f, m_vZoom.y, - m_vCenter.y * m_vZoom.y ) ;
	affineRot.SetRotation( m_degRotation * PI / 180.0 ) ;
	return	affineRot * affineZoom ;
}

SGLAffine SGLBasicForm::Item::GetIAffine( void ) const
{
	return	GetAffine().Inverse() ;
}

S2DVector SGLBasicForm::Item::LocalToGlobal( const S2DVector& vLocal ) const
{
	return	GetAffine() * vLocal ;
}

S2DVector SGLBasicForm::Item::GlobalToLocal( const S2DVector& vGlobal ) const
{
	return	GetIAffine() * vGlobal ;
}

// 表示状態
bool SGLBasicForm::Item::IsVisible( void ) const
{
	return	m_fVisible ;
}

void SGLBasicForm::Item::SetVisible( bool fVisible )
{
	m_fVisible = fVisible ;
}

// 禁止状態
bool SGLBasicForm::Item::IsDisabled( void ) const
{
	return	m_fDisabled ;
}

void SGLBasicForm::Item::SetDisable( bool fDisable )
{
	m_fDisabled = fDisable ;
}

// 透明度
uint32_t SGLBasicForm::Item::GetTransparency( void ) const
{
	return	m_nTransparency ;
}

void SGLBasicForm::Item::SetTransparency( uint32_t nTransparency )
{
	m_nTransparency = nTransparency ;
}

// 色効果
SGLBasicForm::Item::ColorEffect
	SGLBasicForm::Item::GetColorEffect( SGLPalette& rgbEffect ) const
{
	rgbEffect = m_rgbEffect ;
	return	m_colorEffect ;
}

void SGLBasicForm::Item::SetColorEffect
	( ColorEffect colorEffect, const SGLPalette& rgbEffect )
{
	m_colorEffect = colorEffect ;
	m_rgbEffect = rgbEffect ;
}

// バー位置
uint32_t SGLBasicForm::Item::GetBarPos( void ) const
{
	return	0 ;
}

void SGLBasicForm::Item::SetBarPos( uint32_t nPos )
{
}

// バー値範囲
uint32_t SGLBasicForm::Item::GetBarRange( void ) const
{
	return	0 ;
}

void SGLBasicForm::Item::SetBarRange( uint32_t nRange )
{
}

// 描画パラメータ取得
void SGLBasicForm::Item::GetPaintParam( SGLPaintParam& pp, SGLAffine& affine )
{
	affine = GetAffine() ;
	//
	pp.nFlags = paintSmoothStretch | paintDelayable ;
	pp.pAffine = &affine ;
	pp.nTransparency = m_nTransparency ;
	//
	switch ( m_colorEffect )
	{
	case	colorEffectAdd:
		pp.nFlags |= paintApplyColorAdd ;
		pp.rgbColorParam = m_rgbEffect ;
		break ;
	case	colorEffectMul:
		pp.nFlags |= paintApplyColorMul ;
		pp.rgbColorParam = m_rgbEffect ;
		break ;
	case	colorNoEffect:
	default:
		break ;
	}
}

// 描画パラメータ
void SGLBasicForm::Item::AppendDrawParam( SGLDrawImageParamList& dipl )
{
}

// タイマー処理
bool SGLBasicForm::Item::OnTimer( uint32_t msecPast )
{
	return	false ;
}

// マウス当たり判定
bool SGLBasicForm::Item::TestHitCursor( const S2DVector& vGlobal ) const
{
	return	false ;
}

SGLBasicForm::Item *
	SGLBasicForm::Item::GetHitItem
		( S2DVector& vHitLocal, const S2DVector& vGlobal )
{
	if ( TestHitCursor( vGlobal ) )
	{
		vHitLocal = GlobalToLocal( vGlobal ) ;
		return	this ;
	}
	return	NULL ;
}

// マウスキャプチャー
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::Item::SetMouseCapture( void )
{
	if ( m_pParentForm != NULL )
	{
		m_pParentForm->SetMouseCapture( this ) ;
	}
}

void SGLBasicForm::Item::ReleaseMouseCapture( void )
{
	if ( m_pParentForm != NULL )
	{
		m_pParentForm->ReleaseMouseCapture() ;
	}
}

// インタラクティブ
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::Item::OnMouseMove( const S2DVector& vLocal )
{
	if ( m_pInteractive != NULL )
	{
		m_pInteractive->OnMouseMove( this, vLocal ) ;
	}
}

void SGLBasicForm::Item::OnMouseLeave( void )
{
	if ( m_pInteractive != NULL )
	{
		m_pInteractive->OnMouseLeave( this ) ;
	}
}

bool SGLBasicForm::Item::OnMouseWheel( const S2DVector& vLocal, float32_t zDelta )
{
	if ( m_pInteractive != NULL )
	{
		return	m_pInteractive->OnMouseWheel( this, vLocal, zDelta ) ;
	}
	return	false ;
}

bool SGLBasicForm::Item::OnClickDown( const S2DVector& vLocal, MouseButton button )
{
	if ( m_pInteractive != NULL )
	{
		return	m_pInteractive->OnClickDown( this, vLocal, button ) ;
	}
	return	false ;
}

bool SGLBasicForm::Item::OnClickUp( const S2DVector& vLocal, MouseButton button )
{
	if ( m_pInteractive != NULL )
	{
		return	m_pInteractive->OnClickUp( this, vLocal, button ) ;
	}
	return	false ;
}

// 描画更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::Item::NotifyUpdate( void )
{
	if ( m_pParentForm != NULL )
	{
		m_pParentForm->SetUpdateFlag() ;
	}
}

// メッセージ通知
//////////////////////////////////////////////////////////////////////////////
bool SGLBasicForm::Item::NotifyMessage
	( SGLBasicForm::Item * pItem, int nParam, const wchar_t * pwszOpt )
{
	bool	fProcessed = false ;
	if ( m_pInteractive != nullptr )
	{
		fProcessed = m_pInteractive->OnPostMessage( pItem, nParam, pwszOpt ) ;
	}
	if ( !fProcessed && (m_pParentForm != nullptr) )
	{
		fProcessed = m_pParentForm->OnPostMessage( pItem, nParam, pwszOpt ) ;
	}
	return	fProcessed ;
}


// 画像アイテム
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::Image, Item )

SGLBasicForm::Image::Image( void )
	: m_pImage(NULL), m_msecAnime(0),
		m_fClickable(false), m_fTestAlpha(false)
{
}

// 画像
SGLImageObject * SGLBasicForm::Image::GetImage( void ) const
{
	return	m_pImage ;
}

void SGLBasicForm::Image::AttachImage( SGLImageObject * pImage )
{
	if ( m_pImage != pImage )
	{
		m_pImage = pImage ;
		m_msecAnime = 0 ;
		m_aTempFrames.RemoveAll() ;
		NotifyUpdate() ;
	}
}

// マウス当たり判定有効化
bool SGLBasicForm::Image::IsClickable( void ) const
{
	return	m_fClickable ;
}

bool SGLBasicForm::Image::IsTestAlpha( void ) const
{
	return	m_fTestAlpha ;
}

void SGLBasicForm::Image::SetClickable( bool fClickable )
{
	m_fClickable = fClickable ;
}

void SGLBasicForm::Image::SetTestAlpha( bool fTestAlpha )
{
	m_fTestAlpha = fTestAlpha ;
}

// 描画パラメータ
void SGLBasicForm::Image::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	if ( m_fVisible && (m_pImage != NULL) && (m_nTransparency < 0x100) )
	{
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		GetPaintParam( pp, affine ) ;
		//
		SGLImageObject *	pImage = m_pImage ;
		const size_t		nFrameCount = pImage->GetFrameCount() ;
		if ( nFrameCount > 1 )
		{
			size_t	iFrame = m_pImage->FrameFromMilliSec( m_msecAnime ) ;
			if ( iFrame < nFrameCount )
			{
				pImage = m_aTempFrames.GetAt( iFrame ) ;
				if ( pImage == NULL )
				{
					pImage = m_pImage->NewReference( NULL, (ssize_t) iFrame ) ;
					m_aTempFrames.SetAt( iFrame, pImage ) ;
				}
			}
		}
		dipl.AddDrawParam( pp, pImage, NULL ) ;
	}
}

// タイマー処理
bool SGLBasicForm::Image::OnTimer( uint32_t msecPast )
{
	if ( (m_pImage != NULL) && (m_pImage->GetFrameCount() >= 2) )
	{
		uint32_t	nTotalTime = (uint32_t) m_pImage->GetTotalTime() ;
		if ( nTotalTime > 0 )
		{
			size_t	iLastFrame = m_pImage->FrameFromMilliSec( m_msecAnime ) ;
			//
			m_msecAnime += msecPast ;
			m_msecAnime %= nTotalTime ;
			//
			size_t	iNextFrame = m_pImage->FrameFromMilliSec( m_msecAnime ) ;
			if ( iLastFrame != iNextFrame )
			{
				NotifyUpdate() ;
			}
			return	true ;
		}
	}
	return	false ;
}

// マウス当たり判定
bool SGLBasicForm::Image::TestHitCursor( const S2DVector& vGlobal ) const
{
	if ( m_fVisible && m_fClickable
		&& (m_pImage != NULL) && (m_nTransparency < 0x100) )
	{
		S2DVector	vLocal = GlobalToLocal( vGlobal ) ;
		return	TestHitImage( vLocal ) ;
	}
	return	false ;
}

bool SGLBasicForm::Image::TestHitImage( const S2DVector& vLocal ) const
{
	if ( (m_pImage == NULL) || !m_fClickable )
	{
		return	false ;
	}
	SGLSize	size = m_pImage->GetImageSize() ;
	if ( (vLocal.x < 0.0f) || (vLocal.y < 0.0f)
		|| (vLocal.x >= size.w) || (vLocal.y >= size.h) )
	{
		return	false ;
	}
	if ( m_fTestAlpha )
	{
		SGLPalette	pixel( 0 ) ;
		if ( m_pImage->GetPixel
			( pixel, esl_roundfi(vLocal.x), esl_roundfi(vLocal.y) ) )
		{
			return	false ;
		}
		if ( pixel.argb.Alpha < 0x80 )
		{
			return	false ;
		}
	}
	return	true ;
}


// 画像セレクタアイテム
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::ImageSelector, Image )

SGLBasicForm::ImageSelector::ImageSelector( void )
	: m_pImageSet( NULL ), m_iSelector( 0 )
{
}

// 画像
const SGLBasicForm::ImageSet * SGLBasicForm::ImageSelector::GetImageSet( void ) const
{
	return	m_pImageSet ;
}

void SGLBasicForm::ImageSelector::AttachImageSet( const SGLBasicForm::ImageSet * pImageSet )
{
	m_pImageSet = pImageSet ;
}

// 値
size_t SGLBasicForm::ImageSelector::GetSelector( void ) const
{
	return	m_iSelector ;
}

void SGLBasicForm::ImageSelector::SetSelector( size_t iSel )
{
	m_iSelector = iSel ;
	//
	if ( m_pImageSet != NULL )
	{
		AttachImage( m_pImageSet->m_aImages.GetAt(iSel) ) ;
	}
}

// 画像選択（IDで）
bool SGLBasicForm::ImageSelector::SelectImageAs
	( const wchar_t * pwszID, size_t iDefault,
		const wchar_t *const* ppwszCandidates, size_t nCandidates )
{
	if ( m_pImageSet == nullptr )
	{
		return	false ;
	}
	ssize_t	iImage = m_pImageSet->FindFrameAs( pwszID ) ;
	if ( iImage >= 0 )
	{
		SetSelector( (size_t) iImage ) ;
		return	true ;
	}
	for ( size_t i = 0; i < nCandidates; i ++ )
	{
		iImage = m_pImageSet->FindFrameAs( ppwszCandidates[i] ) ;
		if ( iImage >= 0 )
		{
			SetSelector( (size_t) iImage ) ;
			return	false ;
		}
	}
	SetSelector( iDefault ) ;
	return	false ;
}


// ボタンアイテム
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::Button, ImageSelector )

const wchar_t *const
	SGLBasicForm::Button::s_pwszStatusID
					[SGLBasicForm::Button::statusIndexCount] =
{
	L"normal",
	L"focus",
	L"pushed",
	L"pushed_focus",
	L"pushing",
	L"disable",
	L"pushed_disable",
} ;

const SGLBasicForm::Button::StatusIndex
	SGLBasicForm::Button::s_statusSubstitute[SGLBasicForm::Button::statusIndexCount][3] =
{
	{ statusNormal, statusNormal, statusNormal },
	{ statusFocus, statusNormal, statusNormal },
	{ statusPushed, statusFocus, statusNormal },
	{ statusPushedFocus, statusPushed, statusFocus },
	{ statusPushing, statusFocus, statusPushed },
	{ statusDisable, statusNormal, statusNormal },
	{ statusPushedDisable, statusDisable, statusPushed },
} ;

SGLBasicForm::Button::Button( void )
	: m_flagFocus( false ), m_flagPushing( false ),
		m_flagPushed( false ), m_flagToggle( false ), m_nClicked( 0 )
{
	m_fClickable = true ;
//	m_fTestAlpha = true ;
}

// 禁止状態
void SGLBasicForm::Button::SetDisable( bool flagDisable )
{
	ImageSelector::SetDisable( flagDisable ) ;

	if ( flagDisable )
	{
		m_flagFocus = false ;
		m_flagPushing = false ;
	}
	SelectStatusImage( GetStatus() ) ;
}

// トグルボタン
bool SGLBasicForm::Button::IsToggleButton( void ) const
{
	return	m_flagToggle ;
}

void SGLBasicForm::Button::SetToggleButton( bool flagToggle )
{
	m_flagToggle = flagToggle ;
}

void SGLBasicForm::Button::SetTogglePushed( bool flagPushed )
{
	if ( m_flagToggle )
	{
		m_flagPushed = flagPushed ;
		SelectStatusImage( GetStatus() ) ;
	}
}

// ステータス取得
bool SGLBasicForm::Button::IsPushing( void ) const
{
	return	m_flagPushing ;
}

bool SGLBasicForm::Button::IsPushed( void ) const
{
	return	m_flagPushed ;
}

SGLBasicForm::Button::StatusIndex
	SGLBasicForm::Button::GetStatus( void ) const
{
	if ( m_fDisabled )
	{
		return	m_flagPushed ? statusPushedDisable : statusDisable ;
	}
	else if ( m_flagPushing )
	{
		return	statusPushing ;
	}
	else if ( m_flagPushed )
	{
		return	m_flagFocus ? statusPushedFocus : statusPushed ;
	}
	else
	{
		return	m_flagFocus ? statusFocus : statusNormal ;
	}
}

// クリック回数
size_t SGLBasicForm::Button::GetClickedCount( void ) const
{
	return	m_nClicked ;
}

void SGLBasicForm::Button::ResetClickedCount( void )
{
	m_nClicked = 0 ;
}

// ステータスに対応する画像設定
void SGLBasicForm::Button::SelectStatusImage( SGLBasicForm::Button::StatusIndex status )
{
	if ( m_pImageSet != NULL )
	{
		ssize_t	iImage =
					m_pImageSet->FindFrameAs
						( s_pwszStatusID[GetSubstituteStatus( status )] ) ;
		if ( iImage >= 0 )
		{
			SetSelector( (size_t) iImage ) ;
		}
		else
		{
			SetSelector( 0 ) ;
		}
	}
}

SGLBasicForm::Button::StatusIndex
	SGLBasicForm::Button::GetSubstituteStatus
		( SGLBasicForm::Button::StatusIndex status ) const
{
	if ( m_pImageSet == NULL )
	{
		return	status ;
	}
	for ( int i = 0; i < 3; i ++ )
	{
		StatusIndex	siSub = s_statusSubstitute[status][i] ;
		if ( siSub == statusNormal )
		{
			break ;
		}
		ssize_t	iImage = m_pImageSet->FindFrameAs( s_pwszStatusID[siSub] ) ;
		if ( (iImage >= 0)
			&& (m_pImageSet->m_aImages.GetAt((size_t)iImage) != NULL) )
		{
			return	siSub ;
		}
	}
	return	statusNormal ;
}

// インタラクティブ
void SGLBasicForm::Button::OnMouseMove( const S2DVector& vLocal )
{
	if ( !m_fDisabled )
	{
		m_flagFocus = true ;
		SelectStatusImage( GetStatus() ) ;
	}
	ImageSelector::OnMouseMove( vLocal ) ;
}

void SGLBasicForm::Button::OnMouseLeave( void )
{
	m_flagFocus = false ;
	m_flagPushing = false ;
	SelectStatusImage( GetStatus() ) ;

	ImageSelector::OnMouseLeave() ;
}

bool SGLBasicForm::Button::OnClickDown( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = ImageSelector::OnClickDown( vLocal, button ) ;

	if ( !m_fDisabled && (button == mouseLeft) )
	{
		m_flagPushing = true ;
		m_nClicked ++ ;
		//
		if ( m_flagToggle )
		{
			m_flagPushed = !m_flagPushed ;
		}
		SelectStatusImage( GetStatus() ) ;
		//
		NotifyMessage( this, 1 ) ;
		return	true ;
	}
	return	fProcessed ;
}

bool SGLBasicForm::Button::OnClickUp( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = ImageSelector::OnClickUp( vLocal, button ) ;

	if ( !m_fDisabled && (button == mouseLeft) )
	{
		m_flagPushing = false ;
		if ( !m_flagToggle )
		{
			m_flagPushed = false ;
		}
		SelectStatusImage( GetStatus() ) ;
		//
//		NotifyMessage( this, 0 ) ;
		return	true ;
	}
	return	fProcessed ;
}


// ゲージ・バー（プログレス・バー）
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::GaugeBar, Image )

SGLBasicForm::GaugeBar::GaugeBar( void )
	: m_flagVertical(false), m_flagInverse(false), m_nPos(0), m_nRange(0x100)
{
}

// 描画パラメータ
void SGLBasicForm::GaugeBar::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	if ( m_fVisible && (m_pImage != NULL)
		&& (m_nTransparency < 0x100) && (m_nPos > 0) )
	{
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		GetPaintParam( pp, affine ) ;
		//
		SGLSize			size = m_pImage->GetImageSize() ;
		SGLImageRect	rect( 0, 0, size.w, size.h )  ;
		if ( (m_nRange > 0) && (m_nPos < m_nRange) )
		{
			if ( m_flagVertical )
			{
				rect.h = size.h * m_nPos / m_nRange ;
				if ( m_flagInverse )
				{
					rect.y += size.h - rect.h ;
					pp.ptPaint.y += rect.y ;
				}
			}
			else
			{
				rect.w = size.w * m_nPos / m_nRange ;
				if ( m_flagInverse )
				{
					rect.x += size.w - rect.w ;
					pp.ptPaint.x += rect.x ;
				}
			}
		}
		//
		dipl.AddDrawParam( pp, m_pImage, &rect ) ;
	}
}

// 表示スタイル
bool SGLBasicForm::GaugeBar::IsVerticalBar( void ) const
{
	return	m_flagVertical ;
}

bool SGLBasicForm::GaugeBar::IsInverseBar( void ) const
{
	return	m_flagInverse ;
}

void SGLBasicForm::GaugeBar::SetBarStyle( bool flagVert, bool flagInverse )
{
	m_flagVertical = flagVert ;
	m_flagInverse = flagInverse ;
	NotifyUpdate() ;
}

// バー位置
uint32_t SGLBasicForm::GaugeBar::GetBarPos( void ) const
{
	return	m_nPos ;
}

void SGLBasicForm::GaugeBar::SetBarPos( uint32_t nPos )
{
	m_nPos = nPos ;
	NotifyUpdate() ;
}

// バー値範囲
uint32_t SGLBasicForm::GaugeBar::GetBarRange( void ) const
{
	return	m_nRange ;
}

void SGLBasicForm::GaugeBar::SetBarRange( uint32_t nRange )
{
	m_nRange = nRange ;
	NotifyUpdate() ;
}


// トラック・バー（スクロール・バー）
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::TrackBar, ImageSelector )

const wchar_t *const
	SGLBasicForm::TrackBar::s_pwszStatusID
					[SGLBasicForm::TrackBar::statusIndexCount] =
{
	L"normal",
	L"focus",
	L"disable",
} ;

SGLBasicForm::TrackBar::TrackBar( void )
	: m_flagVertical(false), m_flagTracking(false),
		m_nPos(0), m_nRange(0x100)
{
	m_fClickable = true ;
}

// 描画パラメータ
void SGLBasicForm::TrackBar::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	if ( m_fVisible && (m_pImage != nullptr)
		&& (m_nTransparency < 0x100) )
	{
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		GetPaintParam( pp, affine ) ;
		//
		S2DVector	vBar = LocalPosFromTrackPos( m_nPos ) ;
		affine.a13 += affine.a11 * vBar.x + affine.a12 * vBar.y ;
		affine.a23 += affine.a21 * vBar.x + affine.a22 * vBar.y ;
		//
		dipl.AddDrawParam( pp, m_pImage, nullptr ) ;
	}
}

// 禁止状態
void SGLBasicForm::TrackBar::SetDisable( bool flagDisable )
{
	ImageSelector::SetDisable( flagDisable ) ;

	SelectStatusImage( flagDisable ? statusDisable : statusNormal ) ;
	if ( flagDisable && m_flagTracking )
	{
		m_flagTracking = false ;
		ReleaseMouseCapture() ;
	}
}

// 表示スタイル
bool SGLBasicForm::TrackBar::IsVerticalBar( void ) const
{
	return	m_flagVertical ;
}

void SGLBasicForm::TrackBar::SetBarStyle( bool flagVert )
{
	m_flagVertical = flagVert ;
}

// バー位置
uint32_t SGLBasicForm::TrackBar::GetBarPos( void ) const
{
	return	m_nPos ;
}

void SGLBasicForm::TrackBar::SetBarPos( uint32_t nPos )
{
	m_nPos = nPos ;
	NotifyUpdate() ;
	//
	GaugeBar *	pGauge = m_refGauge ;
	if ( pGauge != nullptr )
	{
		pGauge->SetBarPos( nPos ) ;
	}
}

// バー値範囲
uint32_t SGLBasicForm::TrackBar::GetBarRange( void ) const
{
	return	m_nRange ;
}

void SGLBasicForm::TrackBar::SetBarRange( uint32_t nRange )
{
	m_nRange = nRange ;
	NotifyUpdate() ;
}

// 背景トラックアイテム設定
void SGLBasicForm::TrackBar::AttachTrackItem
				( SGLBasicForm::ImageSelector * pTrack )
{
	m_refTrack = pTrack ;
}

void SGLBasicForm::TrackBar::AttachGaugeBar( GaugeBar * pGauge )
{
	m_refGauge = pGauge ;
}

// マウス当たり判定
bool SGLBasicForm::TrackBar::TestHitCursor( const S2DVector& vGlobal ) const
{
	if ( m_fVisible && m_fClickable && (m_nTransparency < 0x100) )
	{
		S2DVector	vLocal = GlobalToLocal( vGlobal ) ;
		return	(vLocal.x >= 0) && (vLocal.x < m_rectOrg.w)
				&& (vLocal.y >= 0) && (vLocal.y < m_rectOrg.h) ;
	}
	return	false ;
}

// リスナ設定
void SGLBasicForm::TrackBar::AttachScrollListener
			( SGLBasicForm::ScrollListener * pListener )
{
	ESLAssert( m_aListener.FindPtr( pListener ) < 0 ) ;
	m_aListener.Add( pListener ) ;
}

void SGLBasicForm::TrackBar::DetachScrollListener
			( SGLBasicForm::ScrollListener * pListener )
{
	ssize_t	i = m_aListener.FindPtr( pListener ) ;
	if ( i >= 0 )
	{
		m_aListener.RemoveAt( (size_t) i ) ;
	}
}

// ステータス画像設定
void SGLBasicForm::TrackBar::SelectStatusImage( StatusIndex status )
{
	SelectStatusImage( *this, status ) ;
	//
	ImageSelector *	pTrack = m_refTrack ;
	if ( pTrack != nullptr )
	{
		SelectStatusImage( *pTrack, status ) ;
	}
}

void SGLBasicForm::TrackBar::SelectStatusImage
		( ImageSelector& imgsel, StatusIndex status )
{
	imgsel.SelectImageAs( s_pwszStatusID[status], 0, s_pwszStatusID, 1 ) ;
}

// インタラクティブ
void SGLBasicForm::TrackBar::OnMouseMove( const S2DVector& vLocal )
{
	if ( !m_fDisabled )
	{
		SelectStatusImage( statusFocus ) ;
		//
		if ( m_flagTracking )
		{
			S2DVector	vPos = vLocal - m_vTrackingOffset ;
			uint32_t	nPos = TrackPosFromLocalPos( vPos ) ;
			OnTrackMoved( nPos ) ;
		}
	}
	ImageSelector::OnMouseMove( vLocal ) ;
}

void SGLBasicForm::TrackBar::OnMouseLeave( void )
{
	if ( !m_fDisabled )
	{
		SelectStatusImage( statusNormal ) ;
	}
	m_flagTracking = false ;
	ImageSelector::OnMouseLeave() ;
}

bool SGLBasicForm::TrackBar::OnClickDown( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( !m_fDisabled && (button == mouseLeft) )
	{
		SGLSize		sizeBar( 0, 0 ) ;
		if ( m_pImage != nullptr )
		{
			sizeBar = m_pImage->GetImageSize() ;
		}
		S2DVector	vBar = LocalPosFromTrackPos( m_nPos ) ;
		if ( (vLocal.x >= vBar.x) && (vLocal.x < vBar.x + sizeBar.w)
			&& (vLocal.y >= vBar.y) && (vLocal.y < vBar.y + sizeBar.h) )
		{
			m_flagTracking = true ;
			m_vTrackingOffset = vLocal - vBar ;
			SetMouseCapture() ;
		}
		else
		{
			S2DVector	vTemp( vLocal.x - sizeBar.w / 2, vLocal.y - sizeBar.h / 2 ) ;
			OnTrackMoved( TrackPosFromLocalPos( vTemp ) ) ;
		}
		fProcessed = true ;
	}
	return	ImageSelector::OnClickDown( vLocal, button ) || fProcessed ;
}

bool SGLBasicForm::TrackBar::OnClickUp( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( !m_fDisabled )
	{
		if ( m_flagTracking )
		{
			m_flagTracking = false ;
			ReleaseMouseCapture() ;
			fProcessed = true ;
		}
	}
	return	ImageSelector::OnClickDown( vLocal, button ) || fProcessed ;
}

void SGLBasicForm::TrackBar::OnTrackMoved( uint32_t nPos )
{
	if ( nPos != m_nPos )
	{
		SetBarPos( nPos ) ;
		//
		SReferenceArray<ScrollListener>::Iterator	iter( &m_aListener, 0 ) ;
		while ( iter.HasNext() )
		{
			ScrollListener *	pListener = iter.Next() ;
			if ( pListener != nullptr )
			{
				pListener->OnScrollPos( m_nPos ) ;
			}
		}
		NotifyMessage( this, nPos, L"overwitable=\"true\"" ) ;
	}
}

uint32_t SGLBasicForm::TrackBar::TrackPosFromLocalPos( const S2DVector& vLocal ) const
{
	SGLSize		sizeBar( 0, 0 ) ;
	if ( m_pImage != nullptr )
	{
		sizeBar = m_pImage->GetImageSize() ;
	}
	int	nPos = (int) m_nPos ;
	if ( m_flagVertical && (m_rectOrg.h - sizeBar.h > 0) )
	{
		nPos = (int) esl_lroundfi( vLocal.y * m_nRange / (m_rectOrg.h - sizeBar.h) ) ;
	}
	else if ( !m_flagVertical && (m_rectOrg.w - sizeBar.w > 0) )
	{
		nPos = (int) esl_lroundfi( vLocal.x * m_nRange / (m_rectOrg.w - sizeBar.w) ) ;
	}
	return	(uint32_t) esl_clampi( nPos, 0, (int) m_nRange ) ;
}

S2DVector SGLBasicForm::TrackBar::LocalPosFromTrackPos( uint32_t nPos ) const
{
	SGLSize		sizeBar( 0, 0 ) ;
	if ( m_pImage != nullptr )
	{
		sizeBar = m_pImage->GetImageSize() ;
	}
	S2DVector	vPos( 0, 0 ) ;
	SGLImageRect	rect( 0, 0, sizeBar.w, sizeBar.h )  ;
	if ( m_nRange > 0 )
	{
		float32_t	fpRange = (float32_t) m_nRange ;
		float32_t	fpPos = (float32_t) esl_clampi( (int) nPos, 0, (int) m_nRange ) ;
		if ( m_flagVertical )
		{
			vPos.y = (float32_t) (m_rectOrg.h - sizeBar.h) * fpPos / fpRange ;
		}
		else
		{
			vPos.x = (float32_t) (m_rectOrg.w - sizeBar.w) * fpPos / fpRange ;
		}
	}
	return	vPos ;
}


// 伸縮フレーム
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::StretchFrame, Item )

const wchar_t * SGLBasicForm::StretchFrame::s_pwszPartsID
					[SGLBasicForm::StretchFrame::partsCount] =
{
	L"upper_left",
	L"upper_center_l",
	L"upper_center",
	L"upper_center_r",
	L"upper_right",
	L"left_upper",
	L"left",
	L"left_under",
	L"right_upper",
	L"right",
	L"right_under",
	L"under_left",
	L"under_center_l",
	L"under_center",
	L"under_center_r",
	L"under_right",
	L"back_frame",
} ;

const SGLBasicForm::StretchFrame::PartsIndex
	SGLBasicForm::StretchFrame::s_partsInOrders
		[SGLBasicForm::StretchFrame::partsCount]
		[SGLBasicForm::StretchFrame::partsCount] =
{
	{ partsUpperLeft, partsUpperCenterLeft, partsUpperCenter, partsUpperCenterRight, partsUpperRight },
	{ partsLeftUpper, partsInvalid, partsInvalid, partsInvalid, partsRightUpper },
	{ partsLeft, partsInvalid, partsInvalid, partsInvalid, partsRight },
	{ partsLeftUnder, partsInvalid, partsInvalid, partsInvalid, partsRightUnder },
	{ partsUnderLeft, partsUnderCenterLeft, partsUnderCenter, partsUnderCenterRight, partsUnderRight },
} ;

const SGLBasicForm::StretchFrame::PartsOrder
	SGLBasicForm::StretchFrame::s_orderHorzParts
		[SGLBasicForm::StretchFrame::partsCount] =
{
	orderLeft, orderCenterLeft, orderCenter, orderCenterRight, orderRight,
	orderLeft, orderLeft, orderLeft,
	orderRight, orderRight, orderRight,
	orderLeft, orderCenterLeft, orderCenter, orderCenterRight, orderRight,
	orderInvalid,
} ;

const SGLBasicForm::StretchFrame::PartsOrder
	SGLBasicForm::StretchFrame::s_orderVertParts
		[SGLBasicForm::StretchFrame::partsCount] =
{
	orderUpper, orderUpper, orderUpper, orderUpper, orderUpper,
	orderCenterUpper, orderCenter, orderCenterUnder,
	orderCenterUpper, orderCenter, orderCenterUnder,
	orderUnder, orderUnder, orderUnder, orderUnder, orderUnder,
	orderInvalid,
} ;

SGLBasicForm::StretchFrame::StretchFrame( void )
	: m_pImageSet( nullptr ),
		m_rectInnerMargin( 0, 0, 0, 0 ),
		m_rectBackMargin( 0, 0, 0, 0 ),
		m_sizeFrame( 0, 0 )
{
	for ( int i = 0; i < orderCount; i ++ )
	{
		m_wHorz[i] = 0 ;
		m_hVert[i] = 0 ;
	}
}

// 画像
const SGLBasicForm::ImageSet *
	SGLBasicForm::StretchFrame::GetImageSet( void ) const
{
	return	m_pImageSet ;
}

void SGLBasicForm::StretchFrame::AttachImageSet( const SGLBasicForm::ImageSet * pImageSet )
{
	m_pImageSet = pImageSet ;
	//
	for ( int i = 0; i < orderCount; i ++ )
	{
		m_wHorz[i] = 0 ;
		m_hVert[i] = 0 ;
	}
	for ( int i = 0; i < partsCount; i ++ )
	{
		m_pImageParts[i] = nullptr ;
		//
		if ( pImageSet != nullptr )
		{
			ssize_t	iImage = pImageSet->FindFrameAs( s_pwszPartsID[i] ) ;
			if ( iImage >= 0 )
			{
				m_pImageParts[i] =
					pImageSet->m_aImages.GetAt( (size_t) iImage ) ;
				if ( m_pImageParts[i] != nullptr )
				{
					SGLSize	sizeImage = m_pImageParts[i]->GetImageSize() ;
					if ( (s_orderHorzParts[i] != orderInvalid)
						&& (sizeImage.w > m_wHorz[ s_orderHorzParts[i] ]) )
					{
						m_wHorz[ s_orderHorzParts[i] ] = sizeImage.w ;
					}
					if ( (s_orderVertParts[i] != orderInvalid)
						&& (sizeImage.h > m_hVert[ s_orderVertParts[i] ]) )
					{
						m_hVert[ s_orderVertParts[i] ] = sizeImage.h ;
					}
				}
			}
		}
	}
}

// クライアント領域マージン
const SGLRect& SGLBasicForm::StretchFrame::GetInnerMargin( void ) const
{
	return	m_rectInnerMargin ;
}

void SGLBasicForm::StretchFrame::SetInnerMargin( const SGLRect& rectMargin )
{
	m_rectInnerMargin = rectMargin ;
}

// 背景画像マージン
const SGLRect& SGLBasicForm::StretchFrame::GetBackFrameMargin( void ) const
{
	return	m_rectBackMargin ;
}

void SGLBasicForm::StretchFrame::SetBackFrameMargin( const SGLRect& rectMargin )
{
	m_rectBackMargin = rectMargin ;
}

// 表示サイズ（外接矩形）
const SGLSize& SGLBasicForm::StretchFrame::GetFrameSize( void ) const
{
	return	m_sizeFrame ;
}

void SGLBasicForm::StretchFrame::SetFrameSize( const SGLSize& sizeFrame )
{
	m_sizeFrame = sizeFrame ;
}

// サイズ計算（内接→外接矩形）
SGLSize SGLBasicForm::StretchFrame::CalcExFrameSize( const SGLSize& sizeInner ) const
{
	return	SGLSize( sizeInner.w + m_rectInnerMargin.left + m_rectInnerMargin.right,
						sizeInner.h + m_rectInnerMargin.top + m_rectInnerMargin.bottom ) ;
}

// 内接矩形取得（ローカル座標）
SGLImageRect SGLBasicForm::StretchFrame::GetInnerFrameRect( void ) const
{
	int	wMin = ((m_wHorz[orderCenterLeft] + m_wHorz[orderCenterRight]) > 0)
												? m_wHorz[orderCenter] : 0 ;
	int	hMin = ((m_hVert[orderCenterUpper] + m_hVert[orderCenterUnder]) > 0)
												? m_hVert[orderCenter] : 0 ;
	return	SGLImageRect
		( m_wHorz[orderLeft], m_hVert[orderUpper],
			esl_max( m_sizeFrame.w - (m_rectInnerMargin.left + m_rectInnerMargin.right), wMin ),
			esl_max( m_sizeFrame.h - (m_rectInnerMargin.top + m_rectInnerMargin.bottom), hMin ) ) ;
}

SGLImageRect SGLBasicForm::StretchFrame::GetInnerFrameImageRect( void ) const
{
	int	wMin = ((m_wHorz[orderCenterLeft] + m_wHorz[orderCenterRight]) > 0)
												? m_wHorz[orderCenter] : 0 ;
	int	hMin = ((m_hVert[orderCenterUpper] + m_hVert[orderCenterUnder]) > 0)
												? m_hVert[orderCenter] : 0 ;
	return	SGLImageRect
		( m_wHorz[orderLeft], m_hVert[orderUpper],
			esl_max( m_sizeFrame.w - (m_wHorz[orderLeft] + m_wHorz[orderRight]), wMin ),
			esl_max( m_sizeFrame.h - (m_hVert[orderUpper] + m_hVert[orderUnder]), hMin ) ) ;
}

// 最小サイズ計算（外接）
int32_t SGLBasicForm::StretchFrame::GetMinFrameWidth( void ) const
{
	return	m_wHorz[orderLeft] + m_wHorz[orderCenter] + m_wHorz[orderRight] ;
}

int32_t SGLBasicForm::StretchFrame::GetMinFrameHeight( void ) const
{
	return	m_hVert[orderUpper] + m_hVert[orderCenter] + m_hVert[orderUnder] ;
}

// 描画パラメータ
void SGLBasicForm::StretchFrame::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	const SGLImageRect	rectInner = GetInnerFrameImageRect() ;
	SGLPaintParam		ppItem ;
	SGLAffine			affineItem ;
	GetPaintParam( ppItem, affineItem ) ;

	// 拡大率計算
	float32_t	wScale = 1.0f ;
	float32_t	hScale = 1.0f ;
	bool		fHorzStretch[orderCount] =
	{
		false, false, false, false, false,
	} ;
	bool		fVertStretch[orderCount] =
	{
		false, false, false, false, false,
	} ;
	if ( m_wHorz[orderCenterLeft] + m_wHorz[orderCenterRight] > 0 )
	{
		fHorzStretch[orderCenterLeft] = true ;
		fHorzStretch[orderCenterRight] = true ;
		wScale = (float32_t) (rectInner.w - m_wHorz[orderCenter])
				/ (float32_t) (m_wHorz[orderCenterLeft] + m_wHorz[orderCenterRight]) ;
	}
	else if ( m_wHorz[orderCenter] > 0 )
	{
		fHorzStretch[orderCenter] = true ;
		wScale = (float32_t) rectInner.w / (float32_t) m_wHorz[orderCenter] ;
	}
	if ( m_hVert[orderCenterUpper] + m_hVert[orderCenterUnder] > 0 )
	{
		fVertStretch[orderCenterUpper] = true ;
		fVertStretch[orderCenterUnder] = true ;
		hScale = (float32_t) (rectInner.h - m_hVert[orderCenter])
				/ (float32_t) (m_hVert[orderCenterUpper] + m_hVert[orderCenterUnder]) ;
	}
	else if ( m_hVert[orderCenter] > 0 )
	{
		fVertStretch[orderCenter] = true ;
		hScale = (float32_t) rectInner.h / (float32_t) m_hVert[orderCenter] ;
	}
	wScale = esl_fmaxf( wScale, 0.0f ) ;
	hScale = esl_fmaxf( hScale, 0.0f ) ;

	// 背景フレーム描画
	if ( m_pImageParts[partsBackFrame] != nullptr )
	{
		SGLSize		sizeBack = m_pImageParts[partsBackFrame]->GetImageSize() ;
		SGLAffine	affineBack = affineItem ;
		S2DVector	vBackOffset = affineItem
							* S2DVector( m_rectBackMargin.left, m_rectBackMargin.top ) ;
		float32_t	wBackScale =
			(float32_t) (esl_max( m_sizeFrame.w, GetMinFrameWidth() )
						- (m_rectBackMargin.left
							+ m_rectBackMargin.right)) / (float32_t) sizeBack.w ;
		float32_t	hBackScale =
			(float32_t) (esl_max( m_sizeFrame.h, GetMinFrameHeight() )
						- (m_rectBackMargin.top
							+ m_rectBackMargin.bottom)) / (float32_t) sizeBack.h ;
		affineBack.a11 *= wBackScale ;
		affineBack.a21 *= wBackScale ;
		affineBack.a12 *= hBackScale ;
		affineBack.a22 *= hBackScale ;
		affineBack.a13 = vBackOffset.x ;
		affineBack.a23 = vBackOffset.y ;
		//
		ppItem.pAffine = &affineBack ;
		dipl.AddDrawParam( ppItem, m_pImageParts[partsBackFrame] ) ;
	}
	 
	// 外枠フレーム描画
	float32_t	yPos = 0.0f ;
	for ( int yOrder = 0; yOrder < orderCount; yOrder ++ )
	{
		float32_t	xPos = 0.0f ;
		float32_t	yScale = (fVertStretch[yOrder] ? hScale : 1.0f) ;
		for ( int xOrder = 0; xOrder < orderCount; xOrder ++ )
		{
			float32_t	xScale = (fHorzStretch[xOrder] ? wScale : 1.0f) ;
			if ( (s_partsInOrders[yOrder][xOrder] != partsInvalid)
				&& (m_pImageParts[ s_partsInOrders[yOrder][xOrder] ] != nullptr) )
			{
				S2DVector	vOffset = affineItem * S2DVector( xPos, yPos ) ;
				SGLAffine	affine = affineItem ;
				affine.a11 *= xScale ;
				affine.a21 *= xScale ;
				affine.a12 *= yScale ;
				affine.a22 *= yScale ;
				affine.a13 = vOffset.x ;
				affine.a23 = vOffset.y ;
				//
				ppItem.pAffine = &affine ;
				dipl.AddDrawParam
					( ppItem, m_pImageParts[ s_partsInOrders[yOrder][xOrder] ] ) ;
			}
			xPos += (float32_t) m_wHorz[xOrder] * xScale ;
		}
		yPos += (float32_t) m_hVert[yOrder] * yScale ;
	}
}



// フォーム
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::SubForm, Item )

SGLBasicForm::SubForm::SubForm( void )
	: m_pAlpha( nullptr ),
		m_rectMargin( 0, 0, 0, 0 ),
		m_fpScrollDelta( 32.0f ),
		m_vScroll( 0.0f, 0.0f ),
		m_fBuffered( false ), m_fAlphaMask( false ),
		m_fScrollable( false ), m_fVertScroll( true ),
		m_fSwipping( false )
{
}

// フォーム
SGLBasicForm * SGLBasicForm::SubForm::GetForm( void ) const
{
	return	m_pForm ;
}

void SGLBasicForm::SubForm::SetForm( SGLBasicForm * pForm )
{
	m_pForm = pForm ;
	if ( pForm != NULL )
	{
		pForm->m_pOwnerItem = this ;
	}
}

// バッファ作成
SGLError SGLBasicForm::SubForm::CreateBuffer
	( uint32_t nWidth, uint32_t nHeight, uint64_t nBufFlags )
{
	SGLError	err =
		m_imgBuffer.CreateImage
			( nWidth, nHeight, formatImageDefaultRGBA, 32, nBufFlags ) ;
	if ( err )
	{
		return	err ;
	}
	m_fBuffered = true ;
	return	sglErrSuccess ;
}

// αマスク設定
SGLError SGLBasicForm::SubForm::SetAlphaMask
	( SGLImageObject * pAlpha, uint64_t nBufFlags )
{
	ESLAssert( pAlpha != nullptr ) ;
	m_pAlpha = pAlpha ;
	if ( pAlpha == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	SGLSize		sizeAlpha = pAlpha->GetImageSize() ;
	SGLError	err = CreateBuffer( sizeAlpha.w, sizeAlpha.h, nBufFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_imgMasked.CreateImage
			( sizeAlpha.w, sizeAlpha.h, formatImageDefaultRGBA, 32, nBufFlags ) ;
	if ( err )
	{
		return	err ;
	}
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodNothing | shadingTextureSmoothing
						| shadingTextureMapping | shadingAlphaTexture
						| shadingNormalizedUVScale ;
	m_material.SetSurfaceAttribute( attr ) ;
	m_material.SetTexture
		( &m_imgBuffer, 0, S3DMaterial::textureDiffusion ) ;
	m_material.SetTexture
		( m_pAlpha, 1, S3DMaterial::textureAlpha, 1.0f, 0.0f ) ;
	//
	m_fAlphaMask = true ;
	return	sglErrSuccess ;
}

// バッファ解放
void SGLBasicForm::SubForm::ReleaseBuffer( void )
{
	if ( m_fAlphaMask )
	{
		m_imgMasked.ReleaseBuffer() ;
		m_pAlpha = nullptr ;
		m_fAlphaMask = false ;
	}
	if ( m_fBuffered )
	{
		m_imgBuffer.ReleaseBuffer() ;
		m_fBuffered = false ;
	}
}

// 描画パラメータ
void SGLBasicForm::SubForm::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	if ( m_fVisible && (m_nTransparency < 0x100) && (m_pForm != NULL) )
	{
		if ( m_fBuffered && m_pForm->GetUpdateFlag() )
		{
			RenderBuffer() ;
			m_pForm->ResetUpdateFlag() ;
		}
		SGLAffine	affSave = dipl.GetAffine() ;
		uint32_t	nTransSave = dipl.GetTransparency() ;
		//
		dipl.AppendAffine( GetAffine() ) ;
		dipl.AppendTransparency( m_nTransparency ) ;
		//
		if ( m_fBuffered )
		{
			SGLPaintParam	pp ;
			SGLImageObject *
				pSrcImage = m_fAlphaMask ? &m_imgMasked : &m_imgBuffer ;
			//
			dipl.AddDrawParam( pp, pSrcImage, nullptr ) ;
		}
		else
		{
			m_pForm->DrawForm( dipl ) ;
		}
		//
		dipl.SetAffine( affSave ) ;
		dipl.SetTransparency( nTransSave ) ;
	}
}

void SGLBasicForm::SubForm::RenderBuffer( void )
{
	ESLAssert( m_pForm != nullptr ) ;
	ESLAssert( m_fBuffered ) ;
	SGLAffine	affine( 1, 0, (float32_t) m_rectMargin.left,
						0, 1, (float32_t) m_rectMargin.top ) ;
	if ( m_fScrollable )
	{
		affine.a13 -= m_vScroll.x ;
		affine.a23 -= m_vScroll.y ;
	}
	SGLDrawImageParamList	dipl ;
	dipl.AppendAffine( affine ) ;
	m_pForm->DrawForm( dipl ) ;
	//
	m_render.AttachTargetImage( &m_imgBuffer, nullptr, nullptr ) ;
	m_render.FillClearTarget( 0 ) ;
	dipl.Draw( m_render ) ;
	m_render.Finish() ;
	m_render.DetachTargetImage() ;
	//
	if ( m_fAlphaMask )
	{
		SGLSize		sizeImage = m_imgMasked.GetImageSize() ;
		S3DVector	vScreen( (float32_t) sizeImage.w * 0.5f,
								(float32_t) sizeImage.h * 0.5f,
								(float32_t) sizeImage.w ) ;
		S3DDMatrix	matCamera( 1, 1, 1 ) ;
		S3DDVector	vCamera( 0, 0, - vScreen.z ) ;
		S3DVector4	vVertex[4] =
		{
			S3DVector4( -vScreen.x, -vScreen.y, 0.0f ),
			S3DVector4( vScreen.x, -vScreen.y, 0.0f ),
			S3DVector4( -vScreen.x, vScreen.y, 0.0f ),
			S3DVector4( vScreen.x, vScreen.y, 0.0f ),
		} ;
		S3DVector4	vNormal[4] =
		{
			S3DVector4( 0.0, 0.0, -1.0f ),
			S3DVector4( 0.0, 0.0, -1.0f ),
			S3DVector4( 0.0, 0.0, -1.0f ),
			S3DVector4( 0.0, 0.0, -1.0f ),
		} ;
		S2DVector	vUVMap[4] =
		{
			S2DVector( 0.0f, 0.0f ),
			S2DVector( 1.0f, 0.0f ),
			S2DVector( 0.0f, 1.0f ),
			S2DVector( 1.0f, 1.0f ),
		} ;
		uint32_t	nIndex[6] =
		{
			0, 2, 3,  0, 3, 1
		} ;
		//
		m_render.AttachTargetImage( &m_imgMasked, nullptr, nullptr ) ;
		m_render.FillClearTarget( 0 ) ;
		m_render.SetProjectionScreen( vScreen ) ;
		m_render.SetCamera( matCamera, vCamera ) ;
		m_render.AddIndexedPrimitiveList
			( &m_material, 0, primitiveTriangle, 6, 4,
				vVertex, vNormal, vUVMap, nullptr, nIndex ) ;
		m_render.Finish() ;
		m_render.DetachTargetImage() ;
	}
}

// タイマー処理
bool SGLBasicForm::SubForm::OnTimer( uint32_t msecPast )
{
	if ( m_pForm != NULL )
	{
		return	m_pForm->OnTimer( msecPast ) ;
	}
	return	false ;
}

// マウス当たり判定
bool SGLBasicForm::SubForm::TestHitCursor( const S2DVector& vGlobal ) const
{
	if ( !m_fVisible || (m_nTransparency >= 0x100) )
	{
		return	false ;
	}
	S2DVector	vLocal = GlobalToLocal( vGlobal ) ;
	if ( IsScrollable() )
	{
		SGLSize	sizeBuffer = m_imgBuffer.GetImageSize() ;
		if ( (vLocal.x >= 0) && (vLocal.x < sizeBuffer.w)
			&& (vLocal.y >= 0) && (vLocal.y < sizeBuffer.h) )
		{
			return	true ;
		}
	}
	if ( m_pForm != nullptr )
	{
		return	m_pForm->TestHitCursor( LocalFormPosFrom( vLocal ) ) ;
	}
	return	false ;
}

SGLBasicForm::Item *
	SGLBasicForm::SubForm::GetHitItem
		( S2DVector& vHitLocal, const S2DVector& vGlobal )
{
	if ( !m_fVisible || (m_nTransparency >= 0x100) )
	{
		return	nullptr ;
	}
	S2DVector	vLocal = GlobalToLocal( vGlobal ) ;
	if ( m_pForm != nullptr )
	{
		S2DVector	vTemp = LocalFormPosFrom( vLocal ) ;
		SGLBasicForm::Item *
			pHitItem =	m_pForm->GetHitItem( vHitLocal, vTemp ) ;
		if ( pHitItem != nullptr )
		{
			return	pHitItem ;
		}
	}
	if ( IsScrollable() )
	{
		SGLSize	sizeBuffer = m_imgBuffer.GetImageSize() ;
		if ( (vLocal.x >= 0) && (vLocal.x < sizeBuffer.w)
			&& (vLocal.y >= 0) && (vLocal.y < sizeBuffer.h) )
		{
			vHitLocal = vLocal ;
			return	this ;
		}
	}
	return	nullptr ;
}

// インタラクティブ
void SGLBasicForm::SubForm::OnMouseMove( const S2DVector& vLocal )
{
	if ( m_fSwipping )
	{
		S2DVector	vDelta = LocalFormPosFrom( vLocal ) - m_vSwipeGrip ;
		UpdateScrollPos( m_vScroll - vDelta, true ) ;
	}
	if ( m_pForm != NULL )
	{
		S2DVector	vTemp = LocalFormPosFrom( vLocal ) ;
		m_pForm->OnMouseMove( vTemp ) ;
	}
	Item::OnMouseMove( vLocal ) ;
}

void SGLBasicForm::SubForm::OnMouseLeave( void )
{
	if ( m_fSwipping )
	{
		m_fSwipping = false ;
		ReleaseMouseCapture() ;
	}
	if ( m_pForm != NULL )
	{
		m_pForm->OnMouseLeave() ;
	}
	Item::OnMouseLeave() ;
}

bool SGLBasicForm::SubForm::OnMouseWheel( const S2DVector& vLocal, float32_t zDelta )
{
	bool	fProcessed = false ;
	if ( IsScrollable() )
	{
		S2DVector	vScroll = m_vScroll ;
		if ( m_fVertScroll )
		{
			vScroll.y += zDelta * m_fpScrollDelta ;
		}
		else
		{
			vScroll.x += zDelta * m_fpScrollDelta ;
		}
		UpdateScrollPos( vScroll, true ) ;
		fProcessed = true ;
	}
	if ( m_pForm != NULL )
	{
		S2DVector	vTemp = LocalFormPosFrom( vLocal ) ;
		fProcessed |= m_pForm->OnMouseWheel( vTemp, zDelta ) ;
	}
	return	Item::OnMouseWheel( vLocal, zDelta ) || fProcessed ;
}

bool SGLBasicForm::SubForm::OnClickDown( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( IsScrollable() && (button == mouseLeft) )
	{
		m_fSwipping = true ;
		m_vSwipeGrip = LocalFormPosFrom( vLocal ) ;
		SetMouseCapture() ;
		fProcessed = true ;
	}
	return	Item::OnClickDown( vLocal, button ) || fProcessed ;
}

bool SGLBasicForm::SubForm::OnClickUp( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( m_fSwipping )
	{
		m_fSwipping = false ;
		ReleaseMouseCapture() ;
		fProcessed = true ;
	}
	return	Item::OnClickUp( vLocal, button ) || fProcessed ;
}

// ローカル座標からフォーム上の座標へ
S2DVector SGLBasicForm::SubForm::LocalFormPosFrom( const S2DVector& vLocal ) const
{
	S2DVector	vTemp = vLocal ;
	if ( IsScrollable() )
	{
		vTemp += m_vScroll ;
	}
	vTemp.x -= (float32_t) m_rectMargin.left ;
	vTemp.y -= (float32_t) m_rectMargin.top ;
	return	vTemp ;
}

// スクロールが有効か？
bool SGLBasicForm::SubForm::IsScrollable( void ) const
{
	return	m_fBuffered && m_fScrollable ;
}

void SGLBasicForm::SubForm::SetScrollable( bool fScrollable )
{
	m_fScrollable = fScrollable ;
}

void SGLBasicForm::SubForm::SetScrollVertical( bool fVert )
{
	m_fVertScroll = fVert ;
}

// ホイールスクロール単位
float32_t SGLBasicForm::SubForm::GetWheelScrollDelta( void ) const
{
	return	m_fpScrollDelta ;
}

void SGLBasicForm::SubForm::SetWheelScrollDelta( float32_t fpDelta )
{
	m_fpScrollDelta = fpDelta ;
}

// 表示マージン設定
void SGLBasicForm::SubForm::SetFormMargin( const SGLRect& rectMargin )
{
	m_rectMargin = rectMargin ;
	if ( IsScrollable() )
	{
		UpdateScrollPos( m_vScroll, true ) ;
	}
}

// スクロール用トラックバー関連付け
void SGLBasicForm::SubForm::AttachTarckBar( SGLBasicForm::TrackBar * pBar )
{
	m_refTrack = pBar ;
	m_fScrollable |= (pBar != nullptr) ;
	//
	if ( IsScrollable() )
	{
		UpdateScrollPos( m_vScroll, true ) ;
	}
}

// スクロール位置更新
void SGLBasicForm::SubForm::UpdateScrollPos( const S2DVector& vScroll, bool flagTrackBar )
{
	ESLAssert( IsScrollable() ) ;
	SGLSize	sizeForm = m_pForm->GetFormRect().GetSize() ;
	SGLSize	sizeBuf = m_imgBuffer.GetImageSize() ;
	SGLSize	sizeView = sizeBuf ;
	sizeView.w -= m_rectMargin.left + m_rectMargin.right ;
	sizeView.h -= m_rectMargin.top + m_rectMargin.bottom ;
	//
	SGLPoint	ptMaxPos( esl_max( 0, sizeForm.w - sizeView.w ),
							esl_max( 0, sizeForm.h - sizeView.h ) ) ;
	if ( m_fVertScroll )
	{
		m_vScroll.x = 0.0f ;
		m_vScroll.y = esl_fclampf( vScroll.y, 0.0f, (float) ptMaxPos.y ) ;
	}
	else
	{
		m_vScroll.x = esl_fclampf( vScroll.x, 0.0f, (float) ptMaxPos.x ) ;
		m_vScroll.y = 0.0f ;
	}
	TrackBar *	pTrackBar = m_refTrack ;
	if ( flagTrackBar && (pTrackBar != nullptr) )
	{
		if ( m_fVertScroll )
		{
			pTrackBar->SetBarRange( (uint32_t) ptMaxPos.y ) ;
			pTrackBar->SetBarPos( (uint32_t) esl_roundfi( m_vScroll.y ) ) ;
		}
		else
		{
			pTrackBar->SetBarRange( (uint32_t) ptMaxPos.x ) ;
			pTrackBar->SetBarPos( (uint32_t) esl_roundfi( m_vScroll.x ) ) ;
		}
	}
	NotifyUpdate() ;
}

// スクロールバーからの通知
void SGLBasicForm::SubForm::OnScrollPos( uint32_t nPos )
{
	S2DVector	vScroll = m_vScroll ;
	if ( m_fVertScroll )
	{
		vScroll.y = (float32_t) nPos ;
	}
	else
	{
		vScroll.x = (float32_t) nPos ;
	}
	if ( IsScrollable() )
	{
		UpdateScrollPos( vScroll, false ) ;
	}
}


// Sprite
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::Sprite, Item )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm::Sprite::StubSprite, SGLSprite )

SGLBasicForm::Sprite::StubSprite::StubSprite( Sprite * pOwner )
	: m_pOwner( pOwner )
{
}

SGLBasicForm::Sprite * SGLBasicForm::Sprite::StubSprite::GetOwnerItem( void ) const
{
	return	m_pOwner ;
}

void SGLBasicForm::Sprite::StubSprite::PostUpdate( SGLRect* pUpdate )
{
	SGLSprite::PostUpdate( pUpdate ) ;
	m_pOwner->NotifyUpdate() ;
}

bool SGLBasicForm::Sprite::StubSprite::NotifyCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	SGLBasicForm *	pParentForm = m_pOwner->GetParentForm() ;
	while ( pParentForm != nullptr )
	{
		SGLSprite *	pOwnerSprite = pParentForm->GetOwnerSprite() ;
		if ( pOwnerSprite != nullptr )
		{
			return	pOwnerSprite->OnCommand
						( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
		}
		SGLBasicForm::SubForm *	pOwnerItem = pParentForm->GetOwnerItem() ;
		if ( pOwnerItem == nullptr )
		{
			break ;
		}
		pParentForm = pOwnerItem->GetParentForm() ;
	}
	return	SGLSprite::NotifyCommand( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
}

SGLError SGLBasicForm::Sprite::StubSprite::SetMouseCapture( void )
{
	m_pOwner->SetMouseCapture() ;
	return	SGLSprite::SetMouseCapture() ;
}

SGLError SGLBasicForm::Sprite::StubSprite::ReleaseMouseCapture( void )
{
	m_pOwner->ReleaseMouseCapture() ;
	return	SGLSprite::ReleaseMouseCapture() ;
}


SGLBasicForm::Sprite::Sprite( void )
{
}

// Sprite
SGLSprite * SGLBasicForm::Sprite::GetSprite( void ) const
{
	return	m_pSprite ;
}

void SGLBasicForm::Sprite::SetSprite( SGLSprite * pSprite )
{
	if ( m_pStub == NULL )
	{
		m_pStub = new StubSprite( this ) ;
	}
	if ( m_pSprite != NULL )
	{
		m_pStub->DetachChild( m_pSprite ) ;
	}
	m_pSprite = pSprite ;
	m_pStub->AddChild( pSprite ) ;
}

// 描画パラメータ
void SGLBasicForm::Sprite::AppendDrawParam( SGLDrawImageParamList& dipl )
{
	if ( m_fVisible && (m_nTransparency < 0x100) && (m_pSprite != NULL) )
	{
		SGLAffine	affSave = dipl.GetAffine() ;
		uint32_t	nTransSave = dipl.GetTransparency() ;
		//
		dipl.AppendAffine( GetAffine() ) ;
		dipl.AppendTransparency( m_nTransparency ) ;
		//
		m_pSprite->PrepareDrawFrame() ;
		m_pSprite->BeforeDraw() ;
		m_pSprite->DrawImageList( dipl ) ;
		m_pSprite->AfterDraw() ;
		//
		dipl.SetAffine( affSave ) ;
		dipl.SetTransparency( nTransSave ) ;
	}
}

// タイマー処理
bool SGLBasicForm::Sprite::OnTimer( uint32_t msecPast )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->AdvanceTime( msecPast ) ;
		return	m_pSprite->HasUpdate() ;
	}
	return	false ;
}

// マウス当たり判定
bool SGLBasicForm::Sprite::TestHitCursor( const S2DVector& vGlobal ) const
{
	if ( m_pSprite != NULL )
	{
		S2DVector	vLocal = GlobalToLocal( vGlobal ) ;
		return	m_pSprite->IsHitSprite( vLocal.x, vLocal.y ) ;
	}
	return	false ;
}

// インタラクティブ
void SGLBasicForm::Sprite::OnMouseMove( const S2DVector& vLocal )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->OnMouseMove( vLocal.x, vLocal.y, 0 ) ;
	}
	Item::OnMouseMove( vLocal ) ;
}

void SGLBasicForm::Sprite::OnMouseLeave( void )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->OnMouseLeave( 0 ) ;
	}
	Item::OnMouseLeave() ;
}

bool SGLBasicForm::Sprite::OnMouseWheel( const S2DVector& vLocal, float32_t zDelta )
{
	bool	fProcessed = false ;
	if ( m_pSprite != NULL )
	{
		fProcessed =
			m_pSprite->OnMouseWheel
				( (int32_t) esl_roundfi
					(zDelta * SGLSprite::WheelDeltaUnit), vLocal.x, vLocal.y, 0 ) ;
	}
	return	Item::OnMouseWheel( vLocal, zDelta ) || fProcessed ;
}

bool SGLBasicForm::Sprite::OnClickDown( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( m_pSprite != NULL )
	{
		fProcessed =
			m_pSprite->OnButtonDown
				( vLocal.x, vLocal.y, MouseFlagFromButton(button) ) ;
	}
	return	Item::OnClickDown( vLocal, button ) || fProcessed ;
}

bool SGLBasicForm::Sprite::OnClickUp( const S2DVector& vLocal, MouseButton button )
{
	bool	fProcessed = false ;
	if ( m_pSprite != NULL )
	{
		fProcessed =
			m_pSprite->OnButtonUp
				( vLocal.x, vLocal.y, MouseFlagFromButton(button) ) ;
	}
	return	Item::OnClickUp( vLocal, button ) || fProcessed ;
}

int64_t SGLBasicForm::Sprite::MouseFlagFromButton( SGLBasicForm::MouseButton button )
{
	switch ( button )
	{
	case	SGLBasicForm::mouseLeft:
		return	SGLSprite::LeftButtonID << SGLSprite::ButtonIDShifter ;

	case	SGLBasicForm::mouseRight:
		return	SGLSprite::RightButtonID << SGLSprite::ButtonIDShifter ;

	case	SGLBasicForm::mouseMiddle:
		return	SGLSprite::MiddleButtonID << SGLSprite::ButtonIDShifter ;

	default:
		break ;
	}
	return	0 ;
}



// SGLBasicForm クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicForm, SObject )

// SGLBasicForm 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::SGLBasicForm( void )
	: m_pOwnerItem( NULL ), m_pOwnerSprite( NULL ), m_flagUpdate( false )
{
}

// SGLBasicForm 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::~SGLBasicForm( void )
{
	CancelAllTimers() ;
}

// SGLBasicFormParser 取得
//////////////////////////////////////////////////////////////////////////////
const SGLBasicFormParser * SGLBasicForm::GetFormParser( void ) const
{
	return	m_refFormParser.GetRef<SGLBasicFormParser>() ;
}

void SGLBasicForm::AttachFormParser( const SGLBasicFormParser * pFormParser )
{
	m_refFormParser = (SGLBasicFormParser*) pFormParser ;
}

// オーナーアイテム取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::SubForm * SGLBasicForm::GetOwnerItem( void ) const
{
	return	m_pOwnerItem ;
}

SGLSprite * SGLBasicForm::GetOwnerSprite( void ) const
{
	return	m_pOwnerSprite ;
}

// オーナースプライト設定
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::AttachOwnerSprite( SGLSprite * pOwner )
{
	m_pOwnerSprite = pOwner ;
}

// 矩形
//////////////////////////////////////////////////////////////////////////////
const SGLImageRect& SGLBasicForm::GetFormRect( void ) const
{
	return	m_rectForm ;
}

void SGLBasicForm::SetFormRect( const SGLImageRect& rect )
{
	m_rectForm = rect ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::Item * SGLBasicForm::GetItemAs( const wchar_t * pwszID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	ssize_t	iSep = -1 ;
	for ( size_t i = 0; pwszID[i] != 0; i ++)
	{
		if ( pwszID[i] == L'\\' )
		{
			iSep = (ssize_t) i ;
			break ;
		}
	}
	if ( iSep <= 0 )
	{
		return	m_ssoaItems.GetAs( pwszID + (iSep + 1) ) ;
	}
	SubForm *	pSubForm =
		ESLTypeCast<SubForm>( m_ssoaItems.GetAs( SString( pwszID, iSep ) ) ) ;
	if ( pSubForm != NULL )
	{
		SGLBasicForm *	pForm = pSubForm->GetForm() ;
		if ( pForm != NULL )
		{
			return	pForm->GetItemAs( pwszID + (iSep + 1) ) ;
		}
	}
	return	NULL ;
}

SGLBasicForm::Item * SGLBasicForm::GetItemAt( size_t nIndex ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	return	m_aItems.GetAt( nIndex ) ;
}

// アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLBasicForm::GetItemCount( void ) const
{
	return	m_aItems.GetLength() ;
}

// アイテム追加
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::AddItem( Item * pItem, const wchar_t * pwszID )
{
	ESLAssert( pItem != nullptr ) ;
	if ( pItem == nullptr )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( (pwszID != NULL) && (pwszID[0] != 0) )
	{
		size_t	i = m_ssoaItems.Add( pwszID, pItem ) ;
		const SString *	pstrID = m_ssoaItems.GetTagAt( i ) ;
		if ( pstrID != NULL )
		{
			pItem->m_id = *pstrID ;
		}
	}
	else
	{
		m_aNoNamed.Add( pItem ) ;
	}
	pItem->m_pParentForm = this ;
	m_aItems.Add( pItem ) ;
}

void SGLBasicForm::InsertItem
	( size_t nIndex, Item * pItem, const wchar_t * pwszID )
{
	ESLAssert( pItem != nullptr ) ;
	if ( pItem == nullptr )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( (pwszID != NULL) && (pwszID[0] != 0) )
	{
		size_t	i = m_ssoaItems.Add( pwszID, pItem ) ;
		const SString *	pstrID = m_ssoaItems.GetTagAt( i ) ;
		if ( pstrID != NULL )
		{
			pItem->m_id = *pstrID ;
		}
	}
	else
	{
		m_aNoNamed.Add( pItem ) ;
	}
	pItem->m_pParentForm = this ;
	m_aItems.InsertAt( nIndex, pItem ) ;
}

// アイテム順序取得
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLBasicForm::FindItemIndex( Item * pItem ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	return	m_aItems.FindPtr( pItem ) ;
}

// アイテム削除
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::RemoveItem( SGLBasicForm::Item * pItem )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ssize_t	i = m_aItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_aItems.RemoveAt( (size_t) i ) ;
	}
	i = m_aNoNamed.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_aNoNamed.RemoveAt( (size_t) i ) ;
	}
	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_ssoaItems.RemoveAt( (size_t) i ) ;
	}
}

void SGLBasicForm::RemoveAllItems( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_aItems.RemoveAll() ;
	m_aNoNamed.RemoveAll() ;
	m_ssoaItems.RemoveAll() ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::DrawForm( SGLDrawImageParamList& dipl )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aItems.GetLength(); i ++ )
	{
		Item *	pItem = m_aItems.GetLastAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			pItem->AppendDrawParam( dipl ) ;
		}
	}
}

// マウス当たり判定
//////////////////////////////////////////////////////////////////////////////
bool SGLBasicForm::TestHitCursor( const S2DVector& vGlobal ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aItems.GetLength(); i ++ )
	{
		Item *	pItem = m_aItems.GetAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			if ( pItem->TestHitCursor( vGlobal ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

SGLBasicForm::Item *
	SGLBasicForm::GetHitItem( S2DVector& vHitLocal, const S2DVector& vGlobal )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	Item *	pHitItem = NULL ;
	for ( size_t i = 0; i < m_aItems.GetLength(); i ++ )
	{
		Item *	pItem = m_aItems.GetAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			pHitItem = pItem->GetHitItem( vHitLocal, vGlobal ) ;
			if ( pHitItem != NULL )
			{
				break ;
			}
		}
	}
	return	pHitItem ;
}

// 描画更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::SetUpdateFlag( void )
{
	m_flagUpdate = true ;

	SubForm *	pSubForm = GetOwnerItem() ;
	if ( pSubForm != NULL )
	{
		pSubForm->NotifyUpdate() ;
	}
}

void SGLBasicForm::ResetUpdateFlag( void )
{
	m_flagUpdate = false ;
}

bool SGLBasicForm::GetUpdateFlag( void ) const
{
	return	m_flagUpdate ;
}

// マウスキャプチャー
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::SetMouseCapture( SGLBasicForm::Item * pItem )
{
	m_refCaptured = pItem ;
	//
	if ( m_pOwnerItem != NULL )
	{
		m_pOwnerItem->SetMouseCapture() ;
	}
	if ( m_pOwnerSprite != NULL )
	{
		m_pOwnerSprite->SetMouseCapture() ;
	}
}

void SGLBasicForm::ReleaseMouseCapture( void )
{
	m_refCaptured = NULL ;
	//
	if ( m_pOwnerItem != NULL )
	{
		m_pOwnerItem->ReleaseMouseCapture() ;
	}
	if ( m_pOwnerSprite != NULL )
	{
		m_pOwnerSprite->ReleaseMouseCapture() ;
	}
}

SGLBasicForm::Item * SGLBasicForm::GetMouseCapture( void ) const
{
	return	m_refCaptured.GetRef<Item>() ;
}

// インタラクティブ
//////////////////////////////////////////////////////////////////////////////
bool SGLBasicForm::OnMouseMove( const S2DVector& vGlobal )
{
	S2DVector	vHitLocal ;
	Item *	pHitItem = GetHitItemOnMouse( vHitLocal, vGlobal ) ;
	if ( pHitItem != NULL )
	{
		pHitItem->OnMouseMove( vHitLocal ) ;
		return	true ;
	}
	return	false ;
}

void SGLBasicForm::OnMouseLeave( void )
{
	Item *	pLastHit = m_refLastHit.GetRef<Item>() ;
	if ( pLastHit != NULL )
	{
		pLastHit->OnMouseLeave() ;
	}
	m_refLastHit = NULL ;
}

bool SGLBasicForm::OnMouseWheel( const S2DVector& vGlobal, float32_t zDelta )
{
	S2DVector	vHitLocal ;
	Item *	pHitItem = GetHitItemOnMouse( vHitLocal, vGlobal ) ;
	if ( pHitItem != NULL )
	{
		return	pHitItem->OnMouseWheel( vHitLocal, zDelta ) ;
	}
	return	false ;
}

bool SGLBasicForm::OnClickDown( const S2DVector& vGlobal, MouseButton button )
{
	S2DVector	vHitLocal ;
	Item *	pHitItem = GetHitItemOnMouse( vHitLocal, vGlobal ) ;
	if ( pHitItem != NULL )
	{
		return	pHitItem->OnClickDown( vHitLocal, button ) ;
	}
	return	false ;
}

bool SGLBasicForm::OnClickUp( const S2DVector& vGlobal, MouseButton button )
{
	S2DVector	vHitLocal ;
	Item *	pHitItem = GetHitItemOnMouse( vHitLocal, vGlobal ) ;
	if ( pHitItem != NULL )
	{
		return	pHitItem->OnClickUp( vHitLocal, button ) ;
	}
	return	false ;
}

SGLBasicForm::Item *
	SGLBasicForm::GetHitItemOnMouse
		( S2DVector& vHitLocal, const S2DVector& vGlobal )
{
	Item *	pHitItem = m_refCaptured.GetRef<Item>() ;
	if ( pHitItem != NULL )
	{
		vHitLocal = pHitItem->GlobalToLocal( vGlobal ) ;
	}
	else
	{
		pHitItem = GetHitItem( vHitLocal, vGlobal ) ;
	}
	Item *	pLastHit = m_refLastHit.GetRef<Item>() ;
	if ( pHitItem != pLastHit )
	{
		if ( pLastHit != NULL )
		{
			pLastHit->OnMouseLeave() ;
		}
		m_refLastHit = pHitItem ;
	}
	return	pHitItem ;
}

// 通知受け取り
//////////////////////////////////////////////////////////////////////////////
bool SGLBasicForm::OnPostMessage
	( SGLBasicForm::Item * pItem, int nParam, const wchar_t * pwszOpt )
{
	bool	fProcessed = false ;
	if ( m_pOwnerItem != nullptr )
	{
		fProcessed = m_pOwnerItem->NotifyMessage( pItem, nParam, pwszOpt ) ;
	}
	if ( !fProcessed
		&& (m_pOwnerSprite != nullptr)
		&& (SString::Compare( pItem->GetID(), L"" ) != 0) )
	{
		int64_t	nCode = 0 ;
		int		nPriority = 0 ;
		bool	fOverwritable = false ;
		if ( pwszOpt != nullptr )
		{
			SStrSortObjectArray<SString>	ssiaDTD ;
			SParserErrorTracer	perrTrace ;
			SXMLDocument		xmlOpt ;
			SStringParser		sparsOpt = pwszOpt ;
			xmlOpt.ParseTagAttributes( sparsOpt, ssiaDTD, perrTrace ) ;
			//
			nCode = xmlOpt.GetAttrIntegerAs( L"code", nCode ) ;
			nPriority = (int) xmlOpt.GetAttrIntegerAs( L"priority", nPriority ) ;
			fOverwritable = (xmlOpt.GetAttrStringAs( L"overwitable", L"false" ) == L"true") ;
		}
		fProcessed =
			m_pOwnerSprite->NotifyCommand
				( pItem->GetID(), nParam, nCode, nPriority, fOverwritable ) ;
	}
	return	fProcessed ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool SGLBasicForm::OnTimer( uint32_t msecPast )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	bool	flagTimer = false ;
	for ( size_t i = 0; i < m_aItems.GetLength(); i ++ )
	{
		Item *	pItem = m_aItems.GetLastAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			flagTimer |= pItem->OnTimer( msecPast ) ;
		}
	}
	for ( size_t i = 0; i < m_aTimers.GetLength(); i ++ )
	{
		TimerListener *	pTimer = m_aTimers.GetAt( i ) ;
		if ( pTimer != NULL )
		{
			TimerListener::Result	result = pTimer->OnTimer( this, msecPast ) ;
			ProcessTimerResult( i, pTimer, result ) ;
			flagTimer = true ;
		}
	}
	m_aTimers.TrimEmpty() ;
	if ( flagTimer )
	{
		SetUpdateFlag() ;
	}
	return	flagTimer ;
}

// タイマー追加
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::AddTimer( SGLBasicForm::TimerListener * pTimer )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ESLAssert( m_aTimers.FindPtr( pTimer ) < 0 ) ;
	m_aTimers.Add( pTimer ) ;
}

// タイマーキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::CancelTimer( SGLBasicForm::TimerListener * pTimer )
{
	if ( pTimer != NULL )
	{
		SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
		ssize_t	i = m_aTimers.FindPtr( pTimer ) ;
		if ( i >= 0 )
		{
			TimerListener::Result	result = pTimer->OnCancel( this ) ;
			ProcessTimerResult( (size_t) i, pTimer, result ) ;
		}
	}
}

void SGLBasicForm::CancelAllTimers( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aTimers.GetLength(); i ++ )
	{
		TimerListener *	pTimer = m_aTimers.GetAt( i ) ;
		if ( pTimer != NULL )
		{
			TimerListener::Result	result = pTimer->OnCancel( this ) ;
			ProcessTimerResult( i, pTimer, result ) ;
		}
	}
	m_aTimers.RemoveAll() ;
}

// タイマー処理強制終了
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::FinishTimer( SGLBasicForm::TimerListener * pTimer )
{
	if ( pTimer != NULL )
	{
		SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
		ssize_t	i = m_aTimers.FindPtr( pTimer ) ;
		if ( i >= 0 )
		{
			TimerListener::Result	result = pTimer->OnFinish( this ) ;
			ProcessTimerResult( (size_t) i, pTimer, result ) ;
		}
	}
}

void SGLBasicForm::FinishAllTimers( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aTimers.GetLength(); i ++ )
	{
		TimerListener *	pTimer = m_aTimers.GetAt( i ) ;
		if ( pTimer != NULL )
		{
			TimerListener::Result	result = pTimer->OnFinish( this ) ;
			ProcessTimerResult( i, pTimer, result ) ;
		}
	}
	m_aTimers.RemoveAll() ;
}

void SGLBasicForm::ProcessTimerResult
	( size_t iTimer,
		SGLBasicForm::TimerListener * pTimer,
		SGLBasicForm::TimerListener::Result result )
{
	if ( result != TimerListener::resultContinue )
	{
		if ( result == TimerListener::resultDetach )
		{
			m_aTimers.ExchangeAt( iTimer, NULL ) ;
		}
		else
		{
			m_aTimers.SetAt( iTimer, NULL ) ;
		}
	}
}

// アイテムのアニメーション追加
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::ItemAnimation * SGLBasicForm::AddItemAnimation
	( SGLBasicForm::Item * pItem,
		uint32_t msecDuration,
		const S2DVector * pMove,
		const uint32_t * pTransparency,
		const S2DVector * pZoom,
		const float32_t * pRotation,
		float32_t fpSpeed0, float32_t fpSpeed1 )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ItemAnimation *	pAnimation = FindItemAnimation( pItem ) ;
	if ( pAnimation == NULL )
	{
		pAnimation = new ItemAnimation( pItem ) ;
		AddTimer( pAnimation ) ;
	}
	pAnimation->AddLinear
		( msecDuration, pMove, pTransparency,
			pZoom, pRotation, fpSpeed0, fpSpeed1 ) ;
	return	pAnimation ;
}

SGLBasicForm::ItemAnimation * SGLBasicForm::AddItemMoveAnimation
	( SGLBasicForm::Item * pItem, uint32_t msecDuration,
		const S2DVector& vMove,
		const uint32_t nTransparency,
		const S2DVector * pZoom,
		const float32_t * pRotation,
		float32_t fpSpeed0, float32_t fpSpeed1 )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ItemAnimation *	pAnimation = FindItemAnimation( pItem ) ;
	if ( pAnimation == NULL )
	{
		pAnimation = new ItemAnimation( pItem ) ;
		AddTimer( pAnimation ) ;
	}
	pAnimation->AddLinear
		( msecDuration, &vMove, &nTransparency,
			pZoom, pRotation, fpSpeed0, fpSpeed1 ) ;
	return	pAnimation ;
}

// アイテムのアニメーション取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::ItemAnimation *
	SGLBasicForm::FindItemAnimation( SGLBasicForm::Item * pItem ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aTimers.GetLength(); i ++ )
	{
		ItemAnimation *	pAnimation =
			ESLTypeCast<ItemAnimation>( m_aTimers.GetAt( i ) ) ;
		if ( pAnimation != NULL )
		{
			if ( pAnimation->GetItem() == pItem )
			{
				return	pAnimation ;
			}
		}
	}
	return	NULL ;
}

// アイテムのアニメーションキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::CancelAnimation( SGLBasicForm::Item * pItem )
{
	ItemAnimation *	pAnimation = FindItemAnimation( pItem ) ;
	if ( pAnimation != NULL )
	{
		CancelTimer( pAnimation ) ;
	}
}

// アイテムのアニメーション即時終了
//////////////////////////////////////////////////////////////////////////////
void SGLBasicForm::FinishAnimation( SGLBasicForm::Item * pItem )
{
	ItemAnimation *	pAnimation = FindItemAnimation( pItem ) ;
	if ( pAnimation != NULL )
	{
		FinishTimer( pAnimation ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 簡易フォーム・パーサー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBasicFormParser, SGLResourceProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLBasicFormParser::SGLBasicFormParser( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLBasicFormParser::~SGLBasicFormParser( void )
{
}

// パッケージ（書庫）ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBasicFormParser::OpenPackage( const wchar_t * pwszArcFile )
{
	ClosePackage() ;
	//
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile( pwszArcFile, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		delete	pFile ;
		return	sglErrFailed ;
	}
	m_pArcFile = new ERISA::SGLArchiveFile ;
	if ( m_pArcFile->OpenArchive( pFile, true ) )
	{
		m_pArcFile = NULL ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// パッケージ（書庫）ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLBasicFormParser::ClosePackage( void )
{
	m_pArcFile = NULL ;
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBasicFormParser::LoadForm
	( const wchar_t * pwszFilePath, SGLResourceProducer * pRsrcProducer )
{
	SSmartPointer<SFileInterface>	pFile = NULL ;
	if ( m_pArcFile != NULL )
	{
		pFile = m_pArcFile->NewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	}
	if ( pFile == NULL )
	{
		pFile = SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::shareRead ) ;
	}
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadForm( *pFile, pRsrcProducer ) ;
}

SGLError SGLBasicFormParser::ReadForm
	( SSystem::SFileInterface& file, SGLResourceProducer * pRsrcProducer )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( file, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	return	ParseForm( xmlDoc, pRsrcProducer ) ;
}

// 解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBasicFormParser::ParseForm
	( const SSystem::SXMLDocument& xmlDoc, SGLResourceProducer * pRsrcProducer )
{
	const SXMLDocument *	pxmlBasicForm = &xmlDoc ;
	if ( xmlDoc.GetType() == SXMLDocument::typeRoot )
	{
		pxmlBasicForm = xmlDoc.GetElementTagAs( L"gls4_basic_form" ) ;
		if ( pxmlBasicForm == NULL )
		{
			return	sglErrFailed ;
		}
	}
	else if ( xmlDoc.GetTag() != L"gls4_basic_form" )
	{
		return	sglErrFailed ;
	}
	//
	// 矩形情報
	//
	const SXMLDocument *
		pxmlRects = pxmlBasicForm->GetElementTagAs( L"rectangles" ) ;
	if ( pxmlRects != NULL )
	{
		for ( size_t i = 0; i < pxmlRects->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlTag = pxmlRects->GetElementAt(i) ;
			if ( (pxmlTag == NULL)
				|| (pxmlTag->GetTag() != L"rect") )
			{
				continue ;
			}
			const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == NULL )
			{
				continue ;
			}
			SGLImageRect	rect ;
			rect.x = (int32_t) pxmlTag->GetAttrIntegerAs( L"x" ) ;
			rect.y = (int32_t) pxmlTag->GetAttrIntegerAs( L"y" ) ;
			rect.w = (int32_t) pxmlTag->GetAttrIntegerAs( L"width" ) ;
			rect.h = (int32_t) pxmlTag->GetAttrIntegerAs( L"height" ) ;
			m_ssaRects.SetAs( *pstrID, rect ) ;
		}
	}
	//
	// リソース情報
	//
	const SXMLDocument *
		pxmlRsrcs = pxmlBasicForm->GetElementTagAs( L"resources" ) ;
	if ( pxmlRsrcs != nullptr )
	{
		for ( size_t i = 0; i < pxmlRsrcs->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlTag = pxmlRsrcs->GetElementAt(i) ;
			if ( (pxmlTag == nullptr)
				|| (pxmlTag->GetTag() == L"") )
			{
				continue ;
			}
			const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == nullptr )
			{
				continue ;
			}
			ResourceInfo *	pRsrcInf = new ResourceInfo ;
			pRsrcInf->m_strType = pxmlTag->GetTag() ;
			pRsrcInf->m_strSrc = pxmlTag->GetAttrStringAs( L"src" ) ;
			pRsrcInf->m_refRsrc = GetResourceAs( *pstrID, pRsrcProducer ) ;
			if ( pRsrcInf->m_refRsrc == nullptr )
			{
				SObject *	pRsrc =
					LoadResource( pRsrcInf->m_strType, pRsrcInf->m_strSrc ) ;
				if ( pRsrc != nullptr )
				{
					pRsrcInf->m_refRsrc = new SSmartObject( pRsrc ) ;
				}
			}
			m_ssoaRsrcs.SetAs( *pstrID, pRsrcInf ) ;
		}
	}
	//
	// 画像セット情報
	//
	const SXMLDocument *
		pxmlImageSets = pxmlBasicForm->GetElementTagAs( L"image_sets" ) ;
	if ( pxmlImageSets != NULL )
	{
		for ( size_t i = 0; i < pxmlImageSets->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlSetTag = pxmlImageSets->GetElementAt(i) ;
			if ( (pxmlSetTag == NULL)
				|| (pxmlSetTag->GetTag() != L"image_set") )
			{
				continue ;
			}
			const SString *	pstrID = pxmlSetTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == NULL )
			{
				continue ;
			}
			SGLBasicForm::ImageSet *	pImageSet = new SGLBasicForm::ImageSet ;
			pImageSet->m_msecDuration =
					(uint32_t) pxmlSetTag->GetAttrIntegerAs( L"duration" ) ;
			for ( size_t j = 0; j < pxmlSetTag->GetElementsCount(); j ++ )
			{
				const SXMLDocument *	pxmlTag = pxmlSetTag->GetElementAt(j) ;
				if ( (pxmlTag == NULL)
					|| (pxmlTag->GetTag() != L"image") )
				{
					continue ;
				}
				const SString *	pstrRefID = pxmlTag->GetAttributeAs( L"ref_id" ) ;
				if ( pstrRefID == NULL )
				{
					continue ;
				}
				ssize_t	iRsrc = m_ssoaRsrcs.FindAs( *pstrRefID ) ;
				if ( iRsrc >= 0 )
				{
					const SString *	pstrRefID = m_ssoaRsrcs.GetTagAt( (size_t) iRsrc ) ;
					ResourceInfo *	pRsrcInf = m_ssoaRsrcs.GetAt( (size_t) iRsrc ) ;
					ESLAssert( pstrRefID != NULL ) ;
					ESLAssert( pRsrcInf != NULL ) ;
					pImageSet->m_aImageIDs.Add( *pstrRefID ) ;
					pImageSet->m_aImages.Add
						( pRsrcInf->m_refRsrc.GetRef<SGLImageObject>() ) ;
				}
				else
				{
					pImageSet->m_aImageIDs.Add( *pstrRefID ) ;
					pImageSet->m_aImages.Add( NULL ) ;
				}
				const SString *	pstrFrameID = pxmlTag->GetAttributeAs( L"frame_id" ) ;
				if ( pstrFrameID != NULL )
				{
					pImageSet->m_aFrameIDs.Add( new SString(*pstrFrameID) ) ;
				}
				else
				{
					pImageSet->m_aFrameIDs.Add( new SString() ) ;
				}
			}
			m_ssoaImageSets.SetAs( *pstrID, pImageSet ) ;
		}
	}
	//
	// スタイル情報
	//
	const SXMLDocument *
		pxmlStyles = pxmlBasicForm->GetElementTagAs( L"styles" ) ;
	if ( pxmlStyles != NULL )
	{
		for ( size_t i = 0; i < pxmlStyles->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlTag = pxmlStyles->GetElementAt(i) ;
			if ( pxmlTag == nullptr )
			{
				continue ;
			}
			const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == nullptr )
			{
				continue ;
			}
			m_ssoaStyles.Add( *pstrID, new SXMLDocument( *pxmlTag ) ) ;
		}
	}
	//
	// フォーム情報
	//
	const SXMLDocument *
		pxmlForms = pxmlBasicForm->GetElementTagAs( L"forms" ) ;
	if ( pxmlForms != NULL )
	{
		for ( size_t i = 0; i < pxmlForms->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlTag = pxmlForms->GetElementAt(i) ;
			if ( (pxmlTag == NULL)
				|| (pxmlTag->GetTag() != L"form") )
			{
				continue ;
			}
			const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID == NULL )
			{
				continue ;
			}
			m_ssoaForms.SetAs( *pstrID, new SXMLDocument( *pxmlTag ) ) ;
		}
	}
	return	sglErrSuccess ;
}

// リソース参照更新
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBasicFormParser::UpdateResourceRef( SGLResourceProducer * pRsrcProducer )
{
	//
	// 画像リソース参照
	//
	for ( size_t i = 0; i < m_ssoaRsrcs.GetLength(); i ++ )
	{
		const SString *	pstrID = m_ssoaRsrcs.GetTagAt( i ) ;
		ResourceInfo *	pRsrcInf = m_ssoaRsrcs.GetAt( i ) ;
		if ( (pstrID == nullptr) || (pRsrcInf == nullptr) )
		{
			continue ;
		}
		pRsrcInf->m_refRsrc = GetResourceAs( *pstrID, pRsrcProducer ) ;
		/*
		if ( pRsrcInf->m_refRsrc == nullptr )
		{
			SObject *	pRsrc =
				LoadResource( pRsrcInf->m_strType, pRsrcInf->m_strSrc ) ;
			if ( pRsrc != nullptr )
			{
				pRsrcInf->m_refRsrc = new SSmartObject( pRsrc ) ;
			}
		}
		*/
	}
	//
	// 画像セット情報
	//
	for ( size_t i = 0; i < m_ssoaImageSets.GetLength(); i ++ )
	{
		SGLBasicForm::ImageSet *	pImageSet = m_ssoaImageSets.GetAt( i ) ;
		if ( pImageSet == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pImageSet->m_aImageIDs.GetLength(); j ++ )
		{
			ResourceInfo *	pRsrcInf =
				m_ssoaRsrcs.GetAs( pImageSet->m_aImageIDs.GetAt(j) ) ;
			if ( pRsrcInf != nullptr )
			{
				pImageSet->m_aImages.SetAt
					( j, pRsrcInf->m_refRsrc.GetRef<SGLImageObject>() ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 画像リソースのアトラス化
//////////////////////////////////////////////////////////////////////////////
size_t SGLBasicFormParser::BuildImageAtlas
	( size_t nMaxAtlasSize, size_t nReqBatchCount )
{
	//
	// 画像を収集
	//
	SPointerArray<SGLImageObject>	aImages ;
	for ( size_t i = 0; i < m_ssoaRsrcs.GetLength(); i ++ )
	{
		ResourceInfo *	pRsrcInf = m_ssoaRsrcs.GetAt( i ) ;
		if ( (pRsrcInf == nullptr)
			|| pRsrcInf->m_refRsrc.IsSmartReference() )
		{
			continue ;
		}
		SGLImageObject *	pImage = pRsrcInf->m_refRsrc.GetRef<SGLImageObject>() ;
		if ( pImage != nullptr )
		{
			aImages.Add( pImage ) ;
		}
	}
	//
	// 順次アトラス化
	//
	size_t	nAtlasedCount = 0 ;
	while ( aImages.GetLength() >= nReqBatchCount )
	{
		SArray<size_t>	aUsedIndexes ;
		const size_t	nImageCount =  aImages.GetLength() ;
		size_t *		pUsedIndexes = aUsedIndexes.GetArray( nImageCount ) ;
		SGLSize			sizeAtlas ;
		size_t			nUsedImages =
			SGLImageObject::EstimateAtlasSize
				( sizeAtlas, aImages.GetConstArray(), aImages.GetLength(),
					false, 1, (uint32_t) nMaxAtlasSize, pUsedIndexes ) ;
		if ( nUsedImages < nReqBatchCount )
		{
			break ;
		}
		SGLImageObject::BuildAtlasParam	param ;
		param.nFlags = 0 ;
		param.nMargin = 1 ;
		param.sizeAtlasMax.w = (int32_t) nMaxAtlasSize ;
		param.sizeAtlasMax.h = (int32_t) nMaxAtlasSize ;
		//
		SGLImageObject *	pAtlasImage =
			SGLImageObject::BuildAtlas
				( aImages.GetConstArray(), aImages.GetLength(),
					param, &nUsedImages, pUsedIndexes ) ;
		for ( size_t i = 0; i < nUsedImages; i ++ )
		{
			aImages.RemoveAt( pUsedIndexes[i] ) ;
		}
		delete	pAtlasImage ;
		//
		nAtlasedCount ++ ;
		//
		if ( nUsedImages < nReqBatchCount )
		{
			break ;
		}
	}
	return	nAtlasedCount ;
}

// 矩形情報取得
//////////////////////////////////////////////////////////////////////////////
const SGLImageRect * SGLBasicFormParser::GetRectAs( const wchar_t * pwszID ) const
{
	return	m_ssaRects.GetAs( pwszID ) ;
}

// リソース情報取得
//////////////////////////////////////////////////////////////////////////////
const SGLBasicFormParser::ResourceInfo *
	SGLBasicFormParser::GetResourceInfoAs( const wchar_t * pwszID ) const
{
	return	m_ssoaRsrcs.GetAs( pwszID ) ;
}

SGLImageObject * SGLBasicFormParser::GetRsrcImageAs( const wchar_t * pwszID ) const
{
	const SGLBasicFormParser::ResourceInfo *
					pRsrcInf = GetResourceInfoAs( pwszID ) ;
	if ( pRsrcInf != NULL )
	{
		return	pRsrcInf->m_refRsrc.GetRef<SGLImageObject>() ;
	}
	return	NULL ;
}

// 画像セット取得
//////////////////////////////////////////////////////////////////////////////
const SGLBasicForm::ImageSet *
	SGLBasicFormParser::GetImageSetAs( const wchar_t * pwszID ) const
{
	return	m_ssoaImageSets.GetAs( pwszID ) ;
}

// 画像セットID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLBasicFormParser::GetImageSetIDOf
			( const SGLBasicForm::ImageSet * pImageSet ) const
{
	ssize_t	i = m_ssoaImageSets.FindPtr( (SGLBasicForm::ImageSet*) pImageSet ) ;
	if ( i >= 0 )
	{
		const SString *	pstrTag = m_ssoaImageSets.GetTagAt( (size_t) i ) ;
		if ( pstrTag != nullptr )
		{
			return	*pstrTag ;
		}
	}
	return	nullptr ;
}

// スタイル取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SXMLDocument *
	SGLBasicFormParser::GetStyleAs( const wchar_t * pwszID ) const
{
	return	m_ssoaStyles.GetAs( pwszID ) ;
}

// フォーム情報取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SXMLDocument *
	SGLBasicFormParser::GetFormAs( const wchar_t * pwszID ) const
{
	return	m_ssoaForms.GetAs( pwszID ) ;
}

// フォームID列挙
//////////////////////////////////////////////////////////////////////////////
void SGLBasicFormParser::EnumerateFormIDs
		( SSystem::SObjectArray<SSystem::SString>& aIDs ) const
{
	for ( size_t i = 0; i < m_ssoaForms.GetLength(); i ++ )
	{
		const SString *	pstrID = m_ssoaForms.GetTagAt( i ) ;
		if ( pstrID != NULL )
		{
			aIDs.Add( new SString( *pstrID ) ) ;
		}
	}
}

// リソース・プロデューサー
//////////////////////////////////////////////////////////////////////////////
SGLResourceProducer *
	SGLBasicFormParser::GetResourceProducer( void ) const
{
	return	m_refRsrcProducer.GetReference() ;
}

void SGLBasicFormParser::AttachResourceProducer
	( SGLResourceProducer * pRsrcProducer )
{
	m_refRsrcProducer = pRsrcProducer ;
}

// リソース読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SObject* SGLBasicFormParser::LoadResource
	( const SSystem::SString& strType, const SSystem::SString& strFile )
{
	if ( strType == L"image" )
	{
		SGLImage *	pImage = new SGLImage ;
		if ( m_pArcFile != NULL )
		{
			SSmartPointer<SFileInterface>
				pFile = m_pArcFile->NewOpenFile( strFile, SFileOpener::shareRead ) ;
			if ( (pFile != NULL)
				&& (pImage->ReadImage( pFile ) == sglErrSuccess) )
			{
				return	pImage ;
			}
		}
		if ( pImage->LoadImage( strFile ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		return	pImage ;
	}
	else if ( strType == L"sound" )
	{
		SGLAudioPlayer *	pSound = new SGLAudioPlayer ;
		if ( m_pArcFile != NULL )
		{
			SSmartPointer<SFileInterface>
				pFile = m_pArcFile->NewOpenFile( strFile, SFileOpener::shareRead ) ;
			if ( (pFile != nullptr)
				&& (pSound->Create
					( pFile.Detach(), true,
						SGLAudioPlayerInterface::modeOpenAutoStatic ) == sglErrSuccess) )
			{
				return	pSound ;
			}
		}
	}
	return	NULL ;
}

// リソース取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SObject* SGLBasicFormParser::GetResourceAs( const wchar_t * pwszID )
{
	const ResourceInfo *	prsInf = GetResourceInfoAs( pwszID ) ;
	if ( (prsInf != nullptr)
		&& (prsInf->m_refRsrc.GetReference() != nullptr) )
	{
		return	prsInf->m_refRsrc.GetReference() ;
	}
	if ( m_refRsrcProducer.GetReference() != nullptr )
	{
		return	m_refRsrcProducer.GetReference()->GetResourceAs( pwszID ) ;
	}
	return	nullptr ;
}

SSystem::SObject* SGLBasicFormParser::GetResourceAs
	( const wchar_t * pwszID, SGLResourceProducer * pRsrcProducer ) const
{
	const ResourceInfo *	prsInf = GetResourceInfoAs( pwszID ) ;
	if ( (prsInf != nullptr)
		&& (prsInf->m_refRsrc.GetReference() != nullptr) )
	{
		return	prsInf->m_refRsrc.GetReference() ;
	}
	if ( m_refRsrcProducer.GetReference() != nullptr )
	{
		return	m_refRsrcProducer.GetReference()->GetResourceAs( pwszID ) ;
	}
	if ( pRsrcProducer != nullptr )
	{
		return	pRsrcProducer->GetResourceAs( pwszID ) ;
	}
	return	nullptr ;
}

SGLImageObject * SGLBasicFormParser::GetImageAs
	( const wchar_t * pwszID, SGLResourceProducer * pRsrcProducer ) const
{
	SGLImageObject *	pImage =
		ESLTypeCast<SGLImageObject>( GetResourceAs( pwszID, nullptr ) ) ;
	if ( (pImage == nullptr) && (pRsrcProducer != nullptr) )
	{
		pImage = ESLTypeCast<SGLImageObject>
					( pRsrcProducer->GetResourceAs( pwszID ) ) ;
	}
	return	pImage ;
}

SGLAudioPlayer * SGLBasicFormParser::GetSoundAs
	( const wchar_t * pwszID, SGLResourceProducer * pRsrcProducer ) const
{
	SGLAudioPlayer *	pSound =
		ESLTypeCast<SGLAudioPlayer>( GetResourceAs( pwszID, nullptr ) ) ;
	if ( (pSound == nullptr) && (pRsrcProducer != nullptr) )
	{
		pSound = ESLTypeCast<SGLAudioPlayer>
					( pRsrcProducer->GetResourceAs( pwszID ) ) ;
	}
	return	pSound ;
}

// フォーム構築
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBasicFormParser::BuildFormAs
	( SGLBasicForm& form, const wchar_t * pwszID,
		SGLResourceProducer * pRsrcProducer ) const
{
	const SXMLDocument *	pxmlForm = GetFormAs( pwszID ) ;
	if ( pxmlForm == NULL )
	{
		return	sglErrFailed ;
	}
	return	BuildForm( form, pxmlForm, pRsrcProducer ) ;
}

SGLError SGLBasicFormParser::BuildForm
	( SGLBasicForm& form,
		const SSystem::SXMLDocument * pxmlForm,
		SGLResourceProducer * pRsrcProducer ) const
{
	if ( pxmlForm == NULL )
	{
		return	sglErrFailed ;
	}
	form.AttachFormParser( this ) ;
	//
	SGLImageRect	rect ;
	rect.x = (int32_t) pxmlForm->GetAttrIntegerAs( L"x" ) ;
	rect.y = (int32_t) pxmlForm->GetAttrIntegerAs( L"y" ) ;
	rect.w = (int32_t) pxmlForm->GetAttrIntegerAs( L"width" ) ;
	rect.h = (int32_t) pxmlForm->GetAttrIntegerAs( L"height" ) ;
	form.SetFormRect( rect ) ;
	//
	for ( size_t i = 0; i < pxmlForm->GetElementsCount(); i ++ )
	{
		const SXMLDocument *	pxmlItem = pxmlForm->GetElementAt( i ) ;
		if ( pxmlItem == NULL )
		{
			continue ;
		}
		SGLImageRect	rectItem ;
		rectItem.x = (int32_t) pxmlItem->GetAttrIntegerAs( L"x" ) ;
		rectItem.y = (int32_t) pxmlItem->GetAttrIntegerAs( L"y" ) ;
		rectItem.w = (int32_t) pxmlItem->GetAttrIntegerAs( L"width" ) ;
		rectItem.h = (int32_t) pxmlItem->GetAttrIntegerAs( L"height" ) ;
		//
		const SXMLDocument *	pxmlStyle = pxmlItem ;
		const SString *	pstrStyle = pxmlItem->GetAttributeAs( L"style" ) ;
		if ( pstrStyle != nullptr )
		{
			pxmlStyle = m_ssoaStyles.GetAs( *pstrStyle ) ;
			if ( pxmlStyle == nullptr )
			{
				pxmlStyle = pxmlItem ;
			}
		}
		//
		SGLBasicForm::Item *	pItem =
			BuildItem( rectItem, pxmlStyle, &form, pRsrcProducer ) ;
		if ( pItem != NULL )
		{
			const SString *	pstrID = pxmlItem->GetAttributeAs( L"id" ) ;
			if ( pstrID != NULL )
			{
				form.AddItem( pItem, *pstrID ) ;
			}
			else
			{
				form.AddItem( pItem, NULL ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// アイテム構築
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::Item *
	SGLBasicFormParser::BuildItemAs
		( const SGLImageRect& rectItem,
			const wchar_t * pwszStyleID,
			const SGLBasicForm * pParentForm,
			SGLResourceProducer * pRsrcProducer ) const
{
	return	BuildItem( rectItem, GetStyleAs(pwszStyleID), pParentForm, pRsrcProducer ) ;
}

SGLBasicForm::Item *
	SGLBasicFormParser::BuildItem
		( const SGLImageRect& rectItem,
			const SSystem::SXMLDocument * pxmlItem,
			const SGLBasicForm * pParentForm,
			SGLResourceProducer * pRsrcProducer ) const
{
	if ( pxmlItem == nullptr )
	{
		return	nullptr ;
	}
	SGLBasicForm::Item *	pItem = NULL ;
	if ( pxmlItem->GetTag() == L"image" )
	{
		SGLBasicForm::Image *	pImageItem = new SGLBasicForm::Image ;
		pImageItem->AttachImage
			( GetImageAs( pxmlItem->GetAttrStringAs( L"image" ), pRsrcProducer ) ) ;
		pItem = pImageItem ;
	}
	else if ( pxmlItem->GetTag() == L"imgsel" )
	{
		SGLBasicForm::ImageSelector *	pSelItem = new SGLBasicForm::ImageSelector ;
		pSelItem->AttachImageSet
			( GetImageSetAs( pxmlItem->GetAttrStringAs( L"imgset" ) ) ) ;
		pSelItem->SetSelector( 0 ) ;
		//
		const SString *	pstrTrackBar = pxmlItem->GetAttributeAs( L"track_bar" ) ;
		if ( (pstrTrackBar != nullptr) && (pParentForm != nullptr) )
		{
			SGLBasicForm::TrackBar *	pTrackBar =
				pParentForm->GetItem<SGLBasicForm::TrackBar>( *pstrTrackBar ) ;
			if ( pTrackBar != nullptr )
			{
				pTrackBar->AttachTrackItem( pSelItem ) ;
			}
		}
		pItem = pSelItem ;
	}
	else if ( (pxmlItem->GetTag() == L"button")
			|| (pxmlItem->GetTag() == L"check") )
	{
		SGLBasicForm::Button *	pButtonItem = new SGLBasicForm::Button ;
		pButtonItem->AttachImageSet
			( GetImageSetAs( pxmlItem->GetAttrStringAs( L"imgset" ) ) ) ;
		pButtonItem->SelectStatusImage( SGLBasicForm::Button::statusNormal ) ;
		pButtonItem->SetToggleButton( pxmlItem->GetTag() == L"check" ) ;
		pItem = pButtonItem ;
	}
	else if ( pxmlItem->GetTag() == L"gauge_bar" )
	{
		SGLBasicForm::GaugeBar *	pBarItem = new SGLBasicForm::GaugeBar ;
		pBarItem->SetBarStyle
			( pxmlItem->GetAttrIntegerAs( L"vertical", 0 ) != 0,
				pxmlItem->GetAttrIntegerAs( L"inverse", 0 ) != 0 ) ;
		pBarItem->AttachImage
			( GetImageAs( pxmlItem->GetAttrStringAs( L"image" ), pRsrcProducer ) ) ;
		pItem = pBarItem ;
	}
	else if ( pxmlItem->GetTag() == L"track_bar" )
	{
		SGLBasicForm::TrackBar *	pBarItem = new SGLBasicForm::TrackBar ;
		pBarItem->SetBarStyle
			( pxmlItem->GetAttrIntegerAs( L"vertical", 0 ) != 0 ) ;
		pBarItem->AttachImageSet
			( GetImageSetAs( pxmlItem->GetAttrStringAs( L"imgset" ) ) ) ;
		pBarItem->SelectStatusImage( SGLBasicForm::TrackBar::statusNormal ) ;
		pItem = pBarItem ;
	}
	else if ( pxmlItem->GetTag() == L"text" )
	{
		SGLBasicForm::Sprite *	pSpriteItem = new SGLBasicForm::Sprite ;
		SGLSpriteText *			pSpriteText = new SGLSpriteText ;
		pSpriteItem->SetSprite( pSpriteText ) ;
		//
		SGLSpriteText::TextStyle	styleText ;
		SSystem::SString			strFontFace ;
		SGLSpriteText::ParseTextStyle( styleText, strFontFace, *pxmlItem ) ;
		pSpriteText->SetTextStyle( styleText ) ;
		//
		pItem = pSpriteItem ;
	}
	else if ( pxmlItem->GetTag() == L"message" )
	{
		SGLBasicForm::Sprite *	pSpriteItem = new SGLBasicForm::Sprite ;
		SGLSpriteMessage *		pSpriteMsg = new SGLSpriteMessage ;
		pSpriteItem->SetSprite( pSpriteMsg ) ;
		//
		SGLSpriteMessage::MessageStyle	styleMsg ;
		SSystem::SString				strFontFace, strRubyFont ;
		SGLSpriteMessage::ParseMessageStyle
				( styleMsg, strFontFace, strRubyFont, *pxmlItem ) ;
		pSpriteMsg->SetMessageStyle( styleMsg ) ;
		//
		if ( pxmlItem->GetAttrIntegerAs( L"buffered", 0 ) != 0 )
		{
			pSpriteMsg->CreateBuffer
				( (uint32_t) pxmlItem->GetAttrIntegerAs( L"width" ),
					(uint32_t) pxmlItem->GetAttrIntegerAs( L"height" ),
					formatImageARGB, 32, SGLImageObject::bufferOnMemory ) ;
		}
		else
		{
			const uint32_t	nWidth =
				sglNormalizeScalePowerBy2
					( (uint32_t) styleMsg.context.rectWritable.GetWidth() ) ;
			const uint32_t	nHeight =
				sglNormalizeScalePowerBy2
					( (uint32_t) styleMsg.context.rectWritable.GetHeight() ) ;
			pSpriteMsg->SetCharacterAtlas( nWidth, nHeight ) ;
		}
		//
		pItem = pSpriteItem ;
	}
	else if ( pxmlItem->GetTag() == L"rich_text" )
	{
		SGLBasicForm::Sprite *		pSpriteItem = new SGLBasicForm::Sprite ;
		SGLSpriteSmartTextView *	pSpriteText = new SGLSpriteSmartTextView ;
		pSpriteItem->SetSprite( pSpriteText ) ;
		//
		SGLSpriteSmartTextView::TextStyle	styleText ;
		SSystem::SString					strFontFace, strRubyFont ;
		SGLSpriteSmartTextView::ParseRichTextStyle
				( styleText, strFontFace, strRubyFont, *pxmlItem ) ;
		pSpriteText->SetTextStyle( styleText ) ;
		pSpriteText->CreateView
			( (uint32_t) pxmlItem->GetAttrIntegerAs( L"width" ),
				(uint32_t) pxmlItem->GetAttrIntegerAs( L"height" ),
				(pxmlItem->GetAttrIntegerAs( L"buffered", 1 ) != 0) ) ;
		//
		const SString *	pstrScrollBar = pxmlItem->GetAttributeAs( L"scroll_bar" ) ;
		if ( (pstrScrollBar != nullptr) && (pParentForm != nullptr) )
		{
			SGLBasicForm::TrackBar *	pScrollBar =
				pParentForm->GetItem<SGLBasicForm::TrackBar>( *pstrScrollBar ) ;
			if ( pScrollBar != nullptr )
			{
				pSpriteText->AttachTrackBar( pScrollBar ) ;
			}
		}
		//
		pItem = pSpriteItem ;
	}
	else if ( pxmlItem->GetTag() == L"form" )
	{
		SGLBasicForm::SubForm *	pFormItem = new SGLBasicForm::SubForm ;
		const SSystem::SXMLDocument *
			pxmlSubForm = GetFormAs( pxmlItem->GetAttrStringAs( L"form" ) ) ;
		if ( pxmlSubForm != NULL )
		{
			SGLBasicForm *	pForm = new SGLBasicForm ;
			BuildForm( *pForm, pxmlSubForm, pRsrcProducer ) ;
			pFormItem->SetForm( pForm ) ;
			//
			SGLRect	rectMargin( 0, 0, 0, 0 ) ;
			rectMargin.left = (int32_t) pxmlItem->GetAttrIntegerAs( L"margin_left", 0 ) ;
			rectMargin.top = (int32_t) pxmlItem->GetAttrIntegerAs( L"margin_top", 0 ) ;
			rectMargin.right = (int32_t) pxmlItem->GetAttrIntegerAs( L"margin_right", 0 ) ;
			rectMargin.bottom = (int32_t) pxmlItem->GetAttrIntegerAs( L"margin_bottom", 0 ) ;
			//
			const SString *	pstrAlpha = pxmlItem->GetAttributeAs( L"alpha" ) ;
			if ( pstrAlpha != nullptr )
			{
				SGLImageObject *	pAlpha = GetImageAs( *pstrAlpha );
				if ( pAlpha != nullptr )
				{
					pFormItem->SetAlphaMask( pAlpha ) ;
					pFormItem->SetFormMargin( rectMargin ) ;
				}
			}
			else if ( pxmlItem->GetAttrIntegerAs( L"buffered", 0 ) != 0 )
			{
				SGLSize	sizeForm = pForm->GetFormRect().GetSize() ;
				if ( !sizeForm.IsEmpty() )
				{
					pFormItem->CreateBuffer
						( sizeForm.w + rectMargin.left + rectMargin.right,
							sizeForm.h + rectMargin.top + rectMargin.bottom ) ;
					pFormItem->SetFormMargin( rectMargin ) ;
				}
			}
			const SString *	pstrScrollable = pxmlItem->GetAttributeAs( L"scrollable" ) ;
			if ( pstrScrollable != nullptr )
			{
				pFormItem->SetScrollable( true ) ;
				pFormItem->SetScrollVertical( *pstrScrollable != L"horz" ) ;
			}
			const SString *	pstrScrollBar = pxmlItem->GetAttributeAs( L"scroll_bar" ) ;
			if ( (pstrScrollBar != nullptr) && (pParentForm != nullptr) )
			{
				SGLBasicForm::TrackBar *	pScrollBar =
					pParentForm->GetItem<SGLBasicForm::TrackBar>( *pstrScrollBar ) ;
				if ( pScrollBar != nullptr )
				{
					pFormItem->AttachTarckBar( pScrollBar ) ;
				}
			}
		}
		pItem = pFormItem ;
	}
	else if ( pxmlItem->GetTag() == L"stretch_frame" )
	{
		SGLBasicForm::StretchFrame *	pFrameItem = new SGLBasicForm::StretchFrame ;
		pFrameItem->AttachImageSet
			( GetImageSetAs( pxmlItem->GetAttrStringAs( L"imgset" ) ) ) ;
		//
		SGLRect	rectInnerMargin( 0, 0, 0, 0 ) ;
		rectInnerMargin.left = (int32_t) pxmlItem->GetAttrIntegerAs( L"inner_margin_left", 0 ) ;
		rectInnerMargin.top = (int32_t) pxmlItem->GetAttrIntegerAs( L"inner_margin_top", 0 ) ;
		rectInnerMargin.right = (int32_t) pxmlItem->GetAttrIntegerAs( L"inner_margin_right", 0 ) ;
		rectInnerMargin.bottom = (int32_t) pxmlItem->GetAttrIntegerAs( L"inner_margin_bottom", 0 ) ;
		pFrameItem->SetInnerMargin( rectInnerMargin ) ;
		//
		SGLRect	rectBackMargin( 0, 0, 0, 0 ) ;
		rectBackMargin.left = (int32_t) pxmlItem->GetAttrIntegerAs( L"back_margin_left", 0 ) ;
		rectBackMargin.top = (int32_t) pxmlItem->GetAttrIntegerAs( L"back_margin_top", 0 ) ;
		rectBackMargin.right = (int32_t) pxmlItem->GetAttrIntegerAs( L"back_margin_right", 0 ) ;
		rectBackMargin.bottom = (int32_t) pxmlItem->GetAttrIntegerAs( L"back_margin_bottom", 0 ) ;
		pFrameItem->SetBackFrameMargin( rectBackMargin ) ;
		//
		SGLSize	sizeFrame = pFrameItem->CalcExFrameSize( SGLSize( 0, 0 ) ) ;
		sizeFrame.w = (int32_t) pxmlItem->GetAttrIntegerAs( L"width", sizeFrame.w ) ;
		sizeFrame.h = (int32_t) pxmlItem->GetAttrIntegerAs( L"height", sizeFrame.h ) ;
		pFrameItem->SetFrameSize( sizeFrame ) ;
		//
		pItem = pFrameItem ;
	}
	//
	// 共通パラメータ
	//
	if ( pItem != NULL )
	{
		pItem->SetItemOrgRect( rectItem ) ;
		pItem->SetPosition( S2DVector( rectItem.x, rectItem.y ) ) ;
	}
	return	pItem ;
}


