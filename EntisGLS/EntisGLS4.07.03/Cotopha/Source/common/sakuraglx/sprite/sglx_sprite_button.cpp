
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スプライトボタンリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteButtonListener, SObject )

// ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButtonListener::OnButtonPushed( SGLSpriteButton& button, bool fRepeat )
{
	return	false ;
}

// ボタンのステータスが変化した
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButtonListener::OnChangedButtonStatus( SGLSpriteButton& button )
{
	return	false ;
}

// ドラッグが開始した
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButtonListener::OnBeginDrag
	( SGLSpriteButton& button, double xOffset, double yOffset )
{
}


//////////////////////////////////////////////////////////////////////////////
// ボタンスタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::ButtonStyle::ButtonStyle( void )
{
	typeButton = typeNormal ;
	flagHitRect = false ;
	maskStatus = 0 ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::ButtonStyle::ButtonStyle
	( const SGLSpriteButton::ButtonStyle& style )
{
	typeButton = style.typeButton ;
	flagHitRect = style.flagHitRect ;
	maskStatus = style.maskStatus ;
	imgdscMask = style.imgdscMask ;

	for ( size_t i = 0; i < statusCount; i ++ )
	{
		imgdscButton[i] = style.imgdscButton[i] ;
		textStyle[i] = style.textStyle[i] ;
		rgbaBackColor[i] = style.rgbaBackColor[i] ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteButton::ButtonStyle&
	SGLSpriteButton::ButtonStyle::operator =
		( const SGLSpriteButton::ButtonStyle& style )
{
	typeButton = style.typeButton ;
	flagHitRect = style.flagHitRect ;
	maskStatus = style.maskStatus ;
	imgdscMask = style.imgdscMask ;

	for ( size_t i = 0; i < statusCount; i ++ )
	{
		imgdscButton[i] = style.imgdscButton[i] ;
		textStyle[i] = style.textStyle[i] ;
		rgbaBackColor[i] = style.rgbaBackColor[i] ;
	}

	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// ボタンスプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteButton, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::SGLSpriteButton( void )
	: m_statusButton( statusNormal ),
		m_statusView( statusInvalid ),
		m_flagChecked( false ),
		m_flagButtonActive( false ), m_idActiveMouse( 0 ),
		m_flagKeyActive( false ), m_flagPushRepeat( false ),
		m_msecBeforeRepeat( 500 ), m_msecRepeatInterval( 100 ),
		m_flagRightClick( false ), m_nRightClickParam( 0 ),
		m_flagStatusNotification( false ), m_nStatusNotifyParam( 0 ),
		m_flagDraggable( false ), m_flagDragged( false ),
		m_thresholdDrag( 8 )
{
	m_flagsUI |= uiFocusable ;
}

SGLSpriteButton::SGLSpriteButton( const SGLSpriteButton& src )
	: SGLSprite( src ),
		m_refMask( src.m_refMask ),
		m_refFocusSE( src.m_refFocusSE ), m_refPushedSE( src.m_refPushedSE ),
		m_statusButton( src.m_statusButton ), m_statusView( src.m_statusView ),
		m_styleButton( src.m_styleButton ),
		m_sizeButton( src.m_sizeButton ),
		m_strText( src.m_strText ),
		m_flagChecked( src.m_flagChecked ),
		m_flagButtonActive( false ), m_idActiveMouse( 0 ),
		m_flagKeyActive( false ), m_flagPushRepeat( src.m_flagPushRepeat ),
		m_msecBeforeRepeat( src.m_msecBeforeRepeat ),
		m_msecRepeatInterval( src.m_msecRepeatInterval ),
		m_flagRightClick( src.m_flagRightClick ),
		m_nRightClickParam( src.m_nRightClickParam ),
		m_flagStatusNotification( src.m_flagStatusNotification ),
		m_nStatusNotifyParam( src.m_nStatusNotifyParam ),
		m_flagDraggable( src.m_flagDraggable ),
		m_flagDragged( src.m_flagDragged ),
		m_thresholdDrag( src.m_thresholdDrag )
{
	m_styleButton.imgdscMask.pImage = m_refMask ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		m_refButton[i] = src.m_refButton[i] ;
		m_strFontFace[i] = src.m_strFontFace[i] ;
		m_styleButton.imgdscButton[i].pImage = m_refButton[i] ;
		m_styleButton.textStyle[i].font.pszFace = m_strFontFace[i] ;
	}
	UpdateButtonImage() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::~SGLSpriteButton( void )
{
	DetachSyncTimeout( 100 ) ;
}

// ボタンリスナを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::AttachButtonListener
	( SGLSpriteButtonListener * pListener )
{
	Lock() ;
	m_refButtonListener = pListener ;
	Unlock() ;
}

void SGLSpriteButton::SetSmartButtonListener
		( SGLSpriteButtonListener * pListener )
{
	Lock() ;
	m_refButtonListener.SetSmartReference( pListener ) ;
	Unlock() ;
}

// SE を設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::AttachSoundEffect
	( SGLAudioPlayerInterface * pFocusSE,
			SGLAudioPlayerInterface * pPushedSE )
{
	m_refFocusSE = pFocusSE ;
	m_refPushedSE = pPushedSE ;
}

// ボタンを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButton::CreateSimpleButton
	( SGLImageObject** ppImages,
		bool flagHitRect, SGLImageObject* pHitMask,
		const SGLSize& sizeButton,
		const SGLSpriteText::TextStyle& textStyle,
		const SGLPalette* pTextColors,
		const SGLPalette* pBackColors,
		uint32_t maskStatus, SGLSpriteButton::ButtonType typeButton )
{
	ButtonStyle	styleButton ;
	styleButton.typeButton = typeButton ;
	styleButton.flagHitRect = flagHitRect ;
	styleButton.maskStatus = maskStatus ;
	styleButton.imgdscMask.pImage = pHitMask ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		if ( maskStatus & (1 << i) )
		{
			if ( ppImages != NULL )
			{
				styleButton.imgdscButton[i].pImage = ppImages[i] ;
			}
			styleButton.textStyle[i] = textStyle ;
			if ( pTextColors != NULL )
			{
				styleButton.textStyle[i].decoration.rgbaBody = pTextColors[i] ;
			}
			if ( pBackColors != NULL )
			{
				styleButton.rgbaBackColor[i] = pBackColors[i] ;
			}
		}
	}
	SetButtonSize( sizeButton ) ;
	SetButtonStyle( styleButton ) ;
	return	sglErrSuccess ;
}

// シンプルな画像ボタンを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButton::CreateSimpleImageButton
	( SGLImageObject** ppImages,
		bool flagHitRect, SGLImageObject* pHitMask,
		uint32_t maskStatus, SGLSpriteButton::ButtonType typeButton )
{
	SGLSpriteText::TextStyle	textStyle ;
	return	CreateSimpleButton
		( ppImages, flagHitRect, pHitMask, SGLSize(0,0),
			textStyle, NULL, NULL, maskStatus, typeButton ) ;
}

// シンプルなテキストボタンを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButton::CreateSimpleTextButton
	( const SGLSpriteText::TextStyle& textStyle,
		const SGLSize& sizeTextExt,
		const SGLPalette* pTextColors,
		const SGLPalette* pBackColors,
		uint32_t maskStatus, SGLSpriteButton::ButtonType typeButton )
{
	return	CreateSimpleButton
		( NULL, true, NULL, sizeTextExt,
			textStyle, pTextColors, pBackColors, maskStatus, typeButton ) ;
}

// シンプルな矩形ボタンを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButton::CreateSimpleRectButton
	( const SGLSize& sizeRect,
		const SGLPalette* pColors,
		uint32_t maskStatus, SGLSpriteButton::ButtonType typeButton )
{
	SGLSpriteText::TextStyle	textStyle ;
	return	CreateSimpleButton
		( NULL, true, NULL, sizeRect,
			textStyle, NULL, pColors, maskStatus, typeButton ) ;
}

// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	if ( !m_sizeButton.IsEmpty()
		&& (m_styleButton.rgbaBackColor[m_statusView].ui32 != 0) )
	{
		// 背景色の塗りつぶし
		SGLPoint	ptBase( 0, 0 ) ;
		render.PushTransformation() ;
		if ( pp.pAffine != NULL )
		{
			render.AppendTransformation
				( *(pp.pAffine) + pp.ptPaint, pp.nTransparency ) ;
		}
		else
		{
			ptBase = pp.ptPaint ;
		}
		render.FillRectangle
			( ptBase.x, ptBase.y,
				m_sizeButton.w, m_sizeButton.h,
				m_styleButton.rgbaBackColor[m_statusView].ui32, 0, 0 ) ;
		render.PopTransformation() ;
	}
	//
	// ボタン画像の描画
	//
	SGLSprite::DrawSprite( render, pp, image ) ;
	//
	// 文字画像の描画
	//
	SGLImageObject *	pTextImage = m_pTextImage[m_statusView] ;
	if ( pTextImage != NULL )
	{
		SGLPaintParam	ppText ;
		render.PushTransformation() ;
		if ( pp.pAffine != NULL )
		{
			render.AppendTransformation
				( *(pp.pAffine) + pp.ptPaint, pp.nTransparency ) ;
		}
		else
		{
			ppText.ptPaint = pp.ptPaint ;
		}
		ppText.ptPaint += m_ptTextOffset[m_statusView] ;
		render.DrawImage( ppText, pTextImage ) ;
		render.PopTransformation() ;
	}
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::GetRectangle( SGLRect& rectExt ) const
{
	bool	fRect = SGLSprite::GetRectangle( rectExt ) ;
	SGLImageObject *	pTextImage = m_pTextImage[m_statusView] ;
	if ( pTextImage != NULL )
	{
		SGLImageInfo	imginf ;
		if ( !pTextImage->GetImageInfo( imginf ) )
		{
			SGLRect	rectText ;
			rectText.left = m_ptTextOffset[m_statusView].x ;
			rectText.top = m_ptTextOffset[m_statusView].y ;
			rectText.SetWidth( imginf.width ) ;
			rectText.SetHeight( imginf.height ) ;
			//
			if ( LocalToGlobalRect( rectText ) )
			{
				if ( fRect )
				{
					rectExt |= rectText ;
				}
				else
				{
					rectExt = rectText ;
				}
				fRect = true ;
			}
		}
	}
	if ( !m_sizeButton.IsEmpty() )
	{
		SGLRect	rectButton ;
		rectButton.left = 0 ;
		rectButton.top = 0 ;
		rectButton.SetWidth( m_sizeButton.w ) ;
		rectButton.SetHeight( m_sizeButton.h ) ;
		//
		if ( LocalToGlobalRect( rectButton ) )
		{
			if ( fRect )
			{
				rectExt |= rectButton ;
			}
			else
			{
				rectExt = rectButton ;
			}
			fRect = true ;
		}
	}
	return	fRect ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::IsHitSprite( double x, double y ) const
{
	if ( m_styleButton.flagHitRect )
	{
		//
		// 矩形判定
		//
		SGLSize	sizeButton = m_sizeButton ;
		if ( sizeButton.IsEmpty() )
		{
			SGLImageObject *	pImage = m_refMask ;
			if ( pImage == NULL )
			{
				pImage = GetAttachedImage() ;
			}
			if ( pImage != NULL )
			{
				sizeButton = pImage->GetImageSize() ;
			}
		}
		if ( !sizeButton.IsEmpty() )
		{
			if ( (x >= 0) & (y >= 0)
				& (x < sizeButton.w) & (y < sizeButton.h) )
			{
				return	true ;
			}
		}
	}
	else
	{
		//
		// マスク画像判定
		//
		SGLImageObject *	pImage = m_refMask ;
		bool				fAlphaImage = true ;
		if ( pImage == NULL )
		{
			pImage = GetAttachedImage() ;
			fAlphaImage = false ;
		}
		if ( SGLSprite::IsHitSpriteImage( pImage, x, y, fAlphaImage ) )
		{
			return	true ;
		}
	}
	//
	// テキスト領域判定
	//
	SGLImageObject *	pTextImage = m_pTextImage[m_statusView] ;
	if ( pTextImage != NULL )
	{
		SGLSize	sizeText = pTextImage->GetImageSize() ;
		if ( !sizeText.IsEmpty() )
		{
			SGLPoint	ptTextOffset = m_ptTextOffset[m_statusView] ;
			if ( (x >= ptTextOffset.x) & (y >= ptTextOffset.y)
				& (x < ptTextOffset.x + sizeText.w)
				& (y < ptTextOffset.y + sizeText.h) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// ボタンスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetButtonStyle
	( const SGLSpriteButton::ButtonStyle& style )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_styleButton = style ;
	if ( style.imgdscMask.pImage != NULL )
	{
		m_refMask.SetSmartReference
			( style.imgdscMask.pImage->NewReference( style.imgdscMask.pRect ) ) ;
	}
	else
	{
		m_refMask = NULL ;
	}
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		if ( style.maskStatus & (1 << i) )
		{
			m_refButton[i] = style.imgdscButton[i].pImage ;
			m_strFontFace[i] = style.textStyle[i].font.pszFace ;
		}
		else
		{
			m_refButton[i].ReleaseReference() ;
			m_strFontFace[i].FreeArray() ;
		}
		m_styleButton.textStyle[i].font.pszFace = m_strFontFace[i] ;
	}
	m_statusView = statusInvalid ;
	UpdateButtonImage() ;
	Unlock() ;
}

// ボタンサイズ（当たり判定・背景色塗りつぶし矩形）設定
// 空の場合には画像・テキスト矩形が使用される
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetButtonSize( const SGLSize& sizeButton )
{
	LockTrace( __FILE__, __LINE__ ) ;
	NotifyUpdate() ;
	m_sizeButton = sizeButton ;
	NotifyUpdate() ;
	Unlock() ;
}

// ボタンステータス設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetButtonStatus( SGLSpriteButton::ButtonStatus status )
{
	ButtonStatus	statusEffect = EffectStatus( status ) ;
	if ( m_statusButton == statusEffect )
	{
		return ;
	}
	m_statusButton = statusEffect ;
	UpdateButtonView() ;
	//
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSpriteButtonListener *	pListener = m_refButtonListener ;
	if ( pListener != NULL )
	{
		pListener->OnChangedButtonStatus( *this ) ;
	}
	if ( m_flagStatusNotification && !m_strID.IsEmpty() )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pParent->OnCommand
				( m_strID, m_nStatusNotifyParam,
						m_statusButton, commandNormal, true ) ;
		}
	}
	Unlock() ;
}

// ボタンのステータス画像を取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject *
	SGLSpriteButton::NewButtonImageReference
			( SGLSpriteButton::ButtonStatus status ) const
{
	status = ValidStatusView( m_styleButton.maskStatus, status ) ;
	if ( m_styleButton.imgdscButton[status].pImage == NULL )
	{
		return	NULL ;
	}
	return	m_styleButton.imgdscButton[status].pImage->
				NewReference( m_styleButton.imgdscButton[status].pRect ) ;
}

// チェック・キーフォーカス・禁止状態をステータスに加味する
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::ButtonStatus
	SGLSpriteButton::EffectStatus( SGLSpriteButton::ButtonStatus status )
{
	if ( IsEnabled() )
	{
		// 有効状態
		if ( status == statusDisabled )
		{
			status = statusNormal ;
		}
		else if ( status == statusPushDisabled )
		{
			status = statusPushed ;
		}
		if ( m_flagKeyActive && HasKeyFocus() )
		{
			// フォーカス有
			if ( status == statusNormal )
			{
				status = statusFocus ;
			}
			else if ( status == statusPushed )
			{
				status = statusPushedFocus ;
			}
		}
		if ( m_flagChecked )
		{
			// チェック状態
			if ( status == statusNormal )
			{
				status = statusPushed ;
			}
			else if ( status == statusFocus )
			{
				status = statusPushedFocus ;
			}
			else if ( status == statusActive )
			{
				status = statusPushedActive ;
			}
		}
		else
		{
			// 非チェック状態
			if ( status == statusPushed )
			{
				status = statusNormal ;
			}
			else if ( status == statusPushedFocus )
			{
				status = statusFocus ;
			}
			else if ( status == statusPushedActive )
			{
				status = statusActive ;
			}
		}
		if ( m_flagButtonActive )
		{
			// クリック押下状態
			if ( (status == statusPushed)
				| (status == statusPushedFocus) )
			{
				status = statusPushedActive ;
			}
			else
			{
				status = statusActive ;
			}
		}
	}
	else
	{
		// 禁止状態
		if ( m_flagChecked )
		{
			return	statusPushDisabled ;
		}
		else
		{
			return	statusDisabled ;
		}
	}
	return	status ;
}

// 有効な表示用ステータスを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButton::ButtonStatus
	SGLSpriteButton::ValidStatusView
		( uint32_t maskStatus, SGLSpriteButton::ButtonStatus status )
{
	while ( (status != statusNormal) & !(maskStatus & (1 << status)) )
	{
		if ( status == statusPushDisabled )
		{
			if ( maskStatus & flagDisabled )
			{
				status = statusDisabled ;
				break ;
			}
			else
			{
				status = statusPushed ;
			}
		}
		else if ( status == statusDisabled )
		{
			status = statusNormal ;
			break ;
		}
		else if ( status == statusPushedActive )
		{
			if ( maskStatus & flagActive )
			{
				status = statusActive ;
				break ;
			}
			else
			{
				status = statusPushedFocus ;
			}
		}
		else if ( status == statusActive )
		{
			if ( maskStatus & flagPushedActive )
			{
				status = statusPushedActive ;
				break ;
			}
			else
			{
				status = statusPushedFocus ;
			}
		}
		else if ( status == statusPushedFocus )
		{
			status = statusPushed ;
		}
		else if ( status == statusPushed )
		{
			status = statusFocus ;
		}
		else if ( status == statusFocus )
		{
			if ( maskStatus & flagPushed )
			{
				status = statusPushed ;
				break ;
			}
			else
			{
				status = statusNormal ;
				break ;
			}
		}
	}
	return	status ;
}

// ボタン画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::UpdateButtonImage( void )
{
	//
	// 文字画像を更新する
	//
	size_t	i ;
	Lock() ;
	NotifyUpdate() ;
	for ( i = 0; i < statusCount; i ++ )
	{
		m_pTextImage[i] = NULL ;
	}
	Unlock() ;
	//
	if ( !m_strText.IsEmpty() )
	{
		for ( i = 0; i < statusCount; i ++ )
		{
			if ( m_styleButton.maskStatus & (1 << i) )
			{
				SGLPoint			ptUpperLeft ;
				SGLImageObject *	pTextImage =
					SGLSpriteText::CreateTextImage
						( ptUpperLeft, m_styleButton.textStyle[i], m_strText ) ;
				if ( pTextImage != NULL )
				{
					Lock() ;
					m_pTextImage[i] = pTextImage ;
					m_ptTextOffset[i] = ptUpperLeft ;
					Unlock() ;
				}
			}
		}
	}
	//
	// 画像表示を更新する
	//
	UpdateButtonView() ;
}

// ボタンの表示を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::UpdateButtonView( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ButtonStatus
		statusView =
			ValidStatusView( m_styleButton.maskStatus, m_statusButton ) ;
	if ( m_statusView != statusView )
	{
ESLTrace( "button view status: %d\n", statusView ) ;
		m_statusView = statusView ;
		AttachAnimation
			( m_styleButton.imgdscButton[statusView].pImage,
					m_styleButton.imgdscButton[statusView].pRect ) ;
		NotifyUpdate() ;
	}
	Unlock() ;
}

// ボタンスタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::ParseButtonStyle
	( SGLSkinManager& skin,
		SGLSpriteButton::ButtonStyle& style,
		SSystem::SString* pstrFontFace,
		const SSystem::SXMLDocument& xmlStyle )
{
	SXMLDocument *	pxmlArrange = xmlStyle.GetElementTagAs( L"arrange" ) ;
	if ( pxmlArrange != NULL )
	{
		SString *	pstrType = pxmlArrange->GetAttributeAs( L"type" ) ;
		if ( pstrType != NULL )
		{
			if ( *pstrType == L"button" )
			{
				style.typeButton = typeNormal ;
			}
			else if ( *pstrType == L"check" )
			{
				style.typeButton = typeCheck ;
			}
			else if ( *pstrType == L"radio" )
			{
				style.typeButton = typeRadio ;
			}
		}
	}
	SXMLDocument *	pxmlMask = xmlStyle.GetElementTagAs( L"mask" ) ;
	if ( pxmlMask != NULL )
	{
		SString *	pstrImage = pxmlMask->GetAttributeAs( L"image" ) ;
		if ( (pstrImage != NULL) && !pstrImage->IsEmpty() )
		{
			skin.GetRichImageAs( style.imgdscMask, *pstrImage ) ;
		}
		SString *	pstrHitRect = pxmlMask->GetAttributeAs( L"rect" ) ;
		if ( pstrHitRect != NULL )
		{
			style.flagHitRect = (*pstrHitRect == L"true") ;
		}
	}
	const wchar_t *	pwszStatusType[statusCount] ;
	pwszStatusType[statusNormal]		= L"normal" ;
	pwszStatusType[statusFocus]			= L"focus" ;
	pwszStatusType[statusPushed]		= L"pushed" ;
	pwszStatusType[statusPushedFocus]	= L"pushed_focus" ;
	pwszStatusType[statusActive]		= L"active" ;
	pwszStatusType[statusPushedActive]	= L"active_pushed" ;
	pwszStatusType[statusDisabled]		= L"disabled" ;
	pwszStatusType[statusPushDisabled]	= L"push_disabled" ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		SXMLDocument *	pxmlTag =
				xmlStyle.GetElementTagAs( pwszStatusType[i] ) ;
		if ( pxmlTag != NULL )
		{
			SString *	pstrImage = pxmlTag->GetAttributeAs( L"image" ) ;
			if ( pstrImage != NULL )
			{
				skin.GetRichImageAs( style.imgdscButton[i], *pstrImage ) ;
			}
			style.rgbaBackColor[i].ui32 =
				(uint32_t) pxmlTag->GetAttrRichIntegerAs
							( L"color", style.rgbaBackColor[i].ui32 ) ;
			style.maskStatus |= (1 << i) ;
			//
			SGLSpriteText::ParseTextStyle
				( style.textStyle[i], pstrFontFace[i], *pxmlTag ) ;
		}
	}
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteButton::GetText( void ) const
{
	Lock() ;
	SString	strText = m_strText ;
	Unlock() ;
	return	strText ;
}

void SGLSpriteButton::SetText( const wchar_t * pwszText )
{
	Lock() ;
	m_strText = pwszText ;
	Unlock() ;
	//
	UpdateButtonImage() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetTextFont
	( const wchar_t * pwszFont, int nSize )
{
	Lock() ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		m_strFontFace[i] = pwszFont ;
		m_styleButton.textStyle[i].font.pszFace = m_strFontFace[i] ;
		if ( nSize != 0 )
		{
			m_styleButton.textStyle[i].font.nSize = nSize ;
		}
	}
	Unlock() ;
	//
	UpdateButtonImage() ;
}

// 入力禁止状態
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetEnable( bool fEnable )
{
	SGLSprite::SetEnable( fEnable ) ;
	SetButtonStatus( m_statusButton ) ;
}

// ボタン属性
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::IsButtonChecked( void )
{
	return	m_flagChecked ;
}

void SGLSpriteButton::CheckButton( bool fCheck )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_flagChecked = fCheck ;
	SetButtonStatus( m_statusButton ) ;
	//
	if ( fCheck && (m_styleButton.typeButton == typeRadio) )
	do
	{
		SGLSprite *	pParent = GetParent() ;
		if ( pParent == NULL )
		{
			break ;
		}
		ssize_t	iThis = pParent->FindChildSprite( this ) ;
		if ( iThis < 0 )
		{
			break ;
		}
		size_t	iFirst = (size_t) iThis ;
		size_t	iEnd = (size_t) iThis + 1 ;
		while ( iFirst > 0 )
		{
			SGLSprite *	pChild = pParent->GetChildAt( iFirst ) ;
			if ( (pChild != NULL)
				&& !(pChild->GetUIFlag() & uiGroupMember) )
			{
				break ;
			}
			iFirst -- ;
		}
		size_t	nCount = pParent->GetChildCount() ;
		while ( iEnd < nCount )
		{
			SGLSprite *	pChild = pParent->GetChildAt( iEnd ) ;
			if ( (pChild != NULL)
				&& !(pChild->GetUIFlag() & uiGroupMember) )
			{
				break ;
			}
			iEnd ++ ;
		}
		for ( size_t i = iFirst; i < iEnd; i ++ )
		{
			SGLSprite *	pChild = pParent->GetChildAt( i ) ;
			if ( (pChild != NULL) & (pChild != this) )
			{
				pChild->CheckButton( false ) ;
			}
		}
	}
	while ( false ) ;
	Unlock() ;
}

bool SGLSpriteButton::IsButtonPushing( void )
{
	return	m_flagButtonActive ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButton::InvokeCommand
	( const SSystem::SXMLDocument& xmlCmd, SSystem::SXMLDocument * pxmlResult )
{
	if ( xmlCmd.GetTag() == L"right_click" )
	{
		m_flagRightClick = true ;
		m_nRightClickParam = xmlCmd.GetAttrRichIntegerAs
							( L"parameter", m_nRightClickParam ) ;
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"push_repeat" )
	{
		m_flagPushRepeat = true ;
		m_msecBeforeRepeat =
			(uint32_t) xmlCmd.GetAttrRichIntegerAs
							( L"before_repeat", m_msecBeforeRepeat ) ;
		m_msecRepeatInterval =
			(uint32_t) xmlCmd.GetAttrRichIntegerAs
							( L"interval", m_msecRepeatInterval ) ;
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"status_reflection" )
	{
		SGLSpriteButtonStatusReflectionListener *
			pListener = new SGLSpriteButtonStatusReflectionListener ;
		if ( !pListener->InvokeCommand( *this, xmlCmd ) )
		{
			SetSmartButtonListener( pListener ) ;
		}
		else
		{
			delete	pListener ;
		}
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"status_notification" )
	{
		m_flagStatusNotification = true ;
		m_nStatusNotifyParam =
			xmlCmd.GetAttrRichIntegerAs( L"parameter", m_nStatusNotifyParam ) ;
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"scroll" )
	{
		SGLSpriteScrollButtonListener *
			pListener = new SGLSpriteScrollButtonListener ;
		if ( !pListener->InvokeCommand( *this, xmlCmd ) )
		{
			SetSmartButtonListener( pListener ) ;
		}
		else
		{
			delete	pListener ;
		}
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"drag" )
	{
		m_flagDraggable =
			(xmlCmd.GetAttrStringAs( L"enable", L"true" ) == L"true") ;
		m_thresholdDrag =
			(int32_t) xmlCmd.GetAttrRichIntegerAs
							( L"threshold", m_thresholdDrag ) ;
		return	sglErrSuccess ;
	}
	return	SGLSprite::InvokeCommand( xmlCmd, pxmlResult ) ;
}

// リピート機能設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetButtonRepeat
	( bool flagRepeat,
		uint32_t msecBefore, uint32_t msecInterval )
{
	m_flagPushRepeat = flagRepeat ;
	m_msecBeforeRepeat = msecBefore ;
	m_msecRepeatInterval = msecInterval ;
}

// 右クリック設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetRightClickNotify
	( bool flagRightClick, int64_t nRightClickParam )
{
	m_flagRightClick = flagRightClick ;
	m_nRightClickParam = nRightClickParam ;
}

// ステータス通知設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::SetStatusNotification
	( bool flagNotify, int64_t nNotifyParam )
{
	m_flagStatusNotification = flagNotify ;
	m_nStatusNotifyParam = nNotifyParam ;
}

// ドラッグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::EnableDrag( bool flagDraggable, int32_t nThreshold )
{
	m_flagDraggable = flagDraggable ;
	m_thresholdDrag = nThreshold ;
}

// フォーカスが設定された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::OnSetKeyFocus( void )
{
	m_flagKeyActive = true ;
	SGLSprite::OnSetKeyFocus() ;
	SetButtonStatus( statusFocus ) ;
}

// キーフォーカスが解除された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::OnKillKeyFocus( void )
{
	SGLSprite::OnKillKeyFocus() ;
	m_flagKeyActive = false ;
	if ( !IsButtonPushing() )
	{
		SetButtonStatus( statusNormal ) ;
	}
}

// ボタン押下処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::OnButtonPushed( bool fRepeat )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLAudioPlayerInterface *	pPlayer = m_refPushedSE ;
	if ( pPlayer != NULL )
	{
		pPlayer->Play() ;
	}
	if ( m_styleButton.typeButton == typeCheck )
	{
		CheckButton( !m_flagChecked ) ;
	}
	else if ( m_styleButton.typeButton == typeRadio )
	{
		CheckButton( true ) ;
	}
	SGLSpriteButtonListener *	pListener = m_refButtonListener ;
	if ( pListener != NULL )
	{
		if ( pListener->OnButtonPushed( *this, fRepeat ) )
		{
			Unlock() ;
			return ;
		}
	}
	if ( !m_strID.IsEmpty() )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pParent->OnCommand( m_strID, 0, 0 ) ;
		}
	}
	Unlock() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButton::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	if ( m_flagButtonActive & m_flagPushRepeat
			& (m_msecPastLastClicked != (uint32_t) -1) )
	{
		uint32_t	msecInterval = m_msecRepeatInterval ;
		m_msecPastLastClicked += msecPast ;
		if ( m_countPushRepeat == 0 )
		{
			msecInterval = m_msecBeforeRepeat ;
		}
		if ( m_msecPastLastClicked >= msecInterval )
		{
			m_msecPastLastClicked -= msecInterval ;
			m_countPushRepeat ++ ;
			//
			OnButtonPushed( true ) ;
		}
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( IsEnabled() )
	{
		if ( (m_statusButton == statusNormal)
			| (m_statusButton == statusPushed) )
		{
			SGLAudioPlayerInterface *	pPlayer = m_refFocusSE ;
			if ( pPlayer != NULL )
			{
				pPlayer->Play() ;
			}
		}
		if ( m_flagDraggable && m_flagButtonActive && !m_flagDragged )
		{
			S2DDVector	vDelta( xPos - m_vDragStart.x, yPos - m_vDragStart.y ) ;
			if ( vDelta.Absolute() >= m_thresholdDrag )
			{
				m_flagDragged = true ;
				m_flagButtonActive = false ;
				//
				SGLSpriteButtonListener *	pListener = m_refButtonListener ;
				if ( pListener != NULL )
				{
					pListener->OnBeginDrag
						( *this, m_vDragStart.x, m_vDragStart.y ) ;
				}
			}
		}
		m_idActiveMouse = GetMouseID(nFlags) ;
		SetButtonStatus( statusFocus ) ;
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteButton::OnMouseLeave( int64_t nFlags )
{
	if ( GetMouseID(nFlags) == m_idActiveMouse )
	{
		m_flagButtonActive = false ;
		m_flagKeyActive = false ;
		m_flagDragged = false ;
		SetButtonStatus( statusNormal ) ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	bool	fResult = SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
	if ( IsEnabled() )
	{
		m_flagButtonActive = true ;
		m_idActiveMouse = GetMouseID(nFlags) ;
		m_countPushRepeat = 0 ;
		m_msecPastLastClicked = 0 ;
		//
		SetButtonStatus( statusFocus ) ;
		//
		m_vDragStart.x = xPos ;
		m_vDragStart.y = yPos ;
		//
		return	true ;
	}
	return	fResult ;
}

bool SGLSpriteButton::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( GetMouseID(nFlags) == m_idActiveMouse )
	{
		m_flagDragged = false ;
		//
		if ( m_flagButtonActive )
		{
			m_flagButtonActive = false ;
			OnButtonPushed( false ) ;
			SetButtonStatus( statusFocus ) ;
		}
	}
	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
	return	true ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::OnRButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagRightClick && !m_strID.IsEmpty() )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pParent->OnCommand( m_strID, m_nRightClickParam, 0 ) ;
			return	true ;
		}
	}
	return	SGLSprite::OnRButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLSpriteButton::OnRButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	return	SGLSprite::OnRButtonUp( xPos, yPos, nFlags ) ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButton::OnKeyDown
	( int64_t nVirtKey, int64_t nFlags )
{
	if ( (nVirtKey == vkeySpace) | (nVirtKey == vkeyReturn) )
	{
		m_flagButtonActive = true ;
		m_flagKeyActive = true ;
		m_countPushRepeat = 0 ;
		m_msecPastLastClicked = (uint32_t) -1 ;
		//
		SetButtonStatus( statusFocus ) ;
		return	true ;
	}
	return	SGLSprite::OnKeyDown( nVirtKey, nFlags ) ;
}

bool SGLSpriteButton::OnKeyUp
	( int64_t nVirtKey, int64_t nFlags )
{
	if ( (nVirtKey == vkeySpace) | (nVirtKey == vkeyReturn) )
	{
		if ( m_flagButtonActive )
		{
			m_flagButtonActive = false ;
			OnButtonPushed( false ) ;
			SetButtonStatus( statusFocus ) ;
		}
		return	true ;
	}
	return	SGLSprite::OnKeyUp( nVirtKey, nFlags ) ;
}

// Rosetta 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteButton::GetRSClassName( void ) const
{
	return	L"ButtonSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteButton::DuplicateObject( void )
{
	return	new SGLSpriteButton( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ボタンステータス反映ボタンリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
( SakuraGL::SGLSpriteButtonStatusReflectionListener, SGLSpriteButtonListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButtonStatusReflectionListener::SGLSpriteButtonStatusReflectionListener( void )
{
	m_maskStatus = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteButtonStatusReflectionListener::~SGLSpriteButtonStatusReflectionListener( void )
{
}

// ステータス反映設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteButtonStatusReflectionListener::AttachStatusReflection
	( SGLSprite * pTarget,
		uint32_t maskStatus, SGLImageObject** ppImages )
{
	m_refTargetSprite = pTarget ;
	m_maskStatus = maskStatus ;
	for ( size_t i = 0; i < SGLSpriteButton::statusCount; i ++ )
	{
		if ( maskStatus & (1 << i) )
		{
			m_refImage[i] = ppImages[i] ;
			m_imgdscImage[i].pImage = ppImages[i] ;
		}
		else
		{
			m_refImage[i] = NULL ;
			m_imgdscImage[i].pImage = NULL ;
		}
		m_imgdscImage[i].pRect = NULL ;
	}
}

// <status_reflection> コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteButtonStatusReflectionListener::InvokeCommand
			( SGLSprite& sprite, const SSystem::SXMLDocument& xmlCmd )
{
	SGLSpriteFormed *	pParent =
		ESLTypeCast<SGLSpriteFormed>( sprite.GetParent() ) ;
	if ( pParent == NULL )
	{
		return	sglErrFailed ;
	}
	SGLSkinManager *	pSkin = pParent->GetAttachedSkin() ;
	if ( pSkin == NULL )
	{
		return	sglErrFailed ;
	}
	SGLSprite *	pTarget =
		pParent->GetItemAs( xmlCmd.GetAttrStringAs( L"target" ) ) ;
	if ( pTarget == NULL )
	{
		return	sglErrFailed ;
	}
	m_refTargetSprite = pTarget ;
	//
	const wchar_t *	pwszStatusType[SGLSpriteButton::statusCount] ;
	pwszStatusType[SGLSpriteButton::statusNormal]		= L"normal" ;
	pwszStatusType[SGLSpriteButton::statusFocus]		= L"focus" ;
	pwszStatusType[SGLSpriteButton::statusPushed]		= L"pushed" ;
	pwszStatusType[SGLSpriteButton::statusPushedFocus]	= L"pushed_focus" ;
	pwszStatusType[SGLSpriteButton::statusActive]		= L"active" ;
	pwszStatusType[SGLSpriteButton::statusPushedActive]	= L"active_pushed" ;
	pwszStatusType[SGLSpriteButton::statusDisabled]		= L"disabled" ;
	pwszStatusType[SGLSpriteButton::statusPushDisabled]	= L"push_disabled" ;
	for ( size_t i = 0; i < SGLSpriteButton::statusCount; i ++ )
	{
		SXMLDocument *	pxmlTag =
			xmlCmd.GetElementTagAs( pwszStatusType[i] ) ;
		if ( pxmlTag != NULL )
		{
			pSkin->GetRichImageAs
				( m_imgdscImage[i], pxmlTag->GetAttrStringAs( L"image" ) ) ;
			m_refImage[i] = m_imgdscImage[i].pImage ;
			if ( m_imgdscImage[i].pImage != NULL )
			{
				m_maskStatus |= (1 << i) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// ボタンのステータスが変化した
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteButtonStatusReflectionListener::OnChangedButtonStatus( SGLSpriteButton& button )
{
	SGLSprite *	pTarget = m_refTargetSprite ;
	if ( pTarget != NULL )
	{
		SGLSpriteButton::ButtonStatus status =
			SGLSpriteButton::ValidStatusView
				( m_maskStatus, button.GetButtonStatus() ) ;
		pTarget->AttachAnimation
			( m_refImage[status], m_imgdscImage[status].pRect ) ;
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// スクロールボタンリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteScrollButtonListener, SGLSpriteButtonListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollButtonListener::SGLSpriteScrollButtonListener( void )
{
	m_offsetScroll = 1 ;
	m_scrollDirection = SGLSprite::scrollDefault ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollButtonListener::~SGLSpriteScrollButtonListener( void )
{
}

// スクロールターゲット設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollButtonListener::AttachScrollTarget
	( SGLSprite * pTarget,
		int offsetScroll, SGLSprite::ScrollDirection scrlDir )
{
	m_refTargetSprite = pTarget ;
	m_offsetScroll = offsetScroll ;
	m_scrollDirection = scrlDir ;
}

// <scroll> コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteScrollButtonListener::InvokeCommand
		( SGLSprite& sprite, const SSystem::SXMLDocument& xmlCmd )
{
	SGLSpriteFormed *	pParent =
		ESLTypeCast<SGLSpriteFormed>( sprite.GetParent() ) ;
	if ( pParent == NULL )
	{
		return	sglErrFailed ;
	}
	SGLSprite *	pTarget =
		pParent->GetItemAs( xmlCmd.GetAttrStringAs( L"target" ) ) ;
	if ( pTarget == NULL )
	{
		return	sglErrFailed ;
	}
	m_refTargetSprite = pTarget ;
	//
	m_offsetScroll =
		(int) xmlCmd.GetAttrRichIntegerAs( L"offset", m_offsetScroll ) ;
	SString	strType = xmlCmd.GetAttrStringAs( L"type" ) ;
	if ( strType == L"vert" )
	{
		m_scrollDirection = SGLSprite::scrollVert ;
	}
	else if ( strType == L"horz" )
	{
		m_scrollDirection = SGLSprite::scrollHorz ;
	}
	return	sglErrSuccess ;
}

// ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollButtonListener::OnButtonPushed
					( SGLSpriteButton& button, bool fRepeat )
{
	SGLSprite *	pTarget = m_refTargetSprite ;
	if ( pTarget != NULL )
	{
		pTarget->SetScrollPos
			( pTarget->GetScrollPos(m_scrollDirection)
								+ m_offsetScroll, m_scrollDirection ) ;
	}
	return	false ;
}

