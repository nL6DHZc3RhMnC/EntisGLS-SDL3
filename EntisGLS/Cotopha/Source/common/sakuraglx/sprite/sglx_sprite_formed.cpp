
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スキンページ用スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFormed, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFormed::SGLSpriteFormed( void )
{
}

SGLSpriteFormed::SGLSpriteFormed( const SGLSpriteFormed& src )
	: SGLSprite( src ), m_refSkin( src.m_refSkin )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFormed::~SGLSpriteFormed( void )
{
	DetachSyncTimeout( 100 ) ;
}

// スキンを関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::AttachSkin( SGLSkinManager * pSkin )
{
	m_refSkin = pSkin ;
}

// 関連付けられたスキンを取得
//////////////////////////////////////////////////////////////////////////////
SGLSkinManager * SGLSpriteFormed::GetAttachedSkin( void ) const
{
	return	m_refSkin ;
}

// SGLBasicFormParser 関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::AttachFormParser( SGLBasicFormParser * pFormParser )
{
	m_refFormParser = pFormParser ;
}

// 関連付けられた SGLBasicFormParser を取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicFormParser * SGLSpriteFormed::GetFormParser( void ) const
{
	return	m_refFormParser ;
}

// フォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::SetBasicForm( SGLBasicForm * pForm )
{
	m_pForm = pForm ;

	if ( pForm != nullptr )
	{
		pForm->AttachOwnerSprite( this ) ;
		//
		SGLImageRect	rect = pForm->GetFormRect() ;
		SetCenterPosition( - rect.x, - rect.y ) ;
	}
}

// フォーム取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm * SGLSpriteFormed::GetBasicForm( void ) const
{
	return	m_pForm ;
}

// スキンリソース解放
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::ReleaseForm( void )
{
	m_refSkin = NULL ;
	DetachAllChildren() ;
	//
	m_refFormParser = NULL ;
	m_pForm = NULL ;
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::PrepareDrawFrame( void )
{
	SGLSprite::PrepareDrawFrame() ;

	m_dipList.ClearList() ;
	//
	if ( IsVisible() && (GetTransparency() < 0x100) )
	{
		OnDrawImage( m_dipList ) ;
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::DrawChildren
	( S3DRenderContextInterface& render, Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;
	//
	m_dipList.Draw( render ) ;
}

void SGLSpriteFormed::DrawChildrenImageList
	( SGLDrawImageParamList& dipl, Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildrenImageList( dipl, s3dView ) ;
	//
	m_dipList.DrawToList( dipl ) ;
}

// 可視状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::IsSpriteVisible( const wchar_t * pwszID ) const
{
	bool	fVisible = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		fVisible = pItem->IsVisible() ;
	}
	else
	{
		fVisible = SGLSprite::IsSpriteVisible( pwszID ) ;
	}
	Unlock() ;
	return	fVisible ;
}

void SGLSpriteFormed::SetSpriteVisible( const wchar_t * pwszID, bool fVisible )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->NotifyUpdate() ;
		pItem->SetVisible( fVisible ) ;
		pItem->NotifyUpdate() ;
	}
	SGLSprite::SetSpriteVisible( pwszID, fVisible ) ;
	Unlock() ;
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteFormed::GetSpriteTransparency( const wchar_t * pwszID ) const
{
	uint32_t	nTrans = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		nTrans = pItem->GetTransparency() ;
	}
	else
	{
		nTrans = SGLSprite::GetSpriteTransparency( pwszID ) ;
	}
	Unlock() ;
	return	nTrans ;
}

void SGLSpriteFormed::SetSpriteTransparency
	( const wchar_t * pwszID, uint32_t nTransparency )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->SetTransparency( nTransparency ) ;
		pItem->NotifyUpdate() ;
	}
	SGLSprite::SetSpriteTransparency( pwszID, nTransparency ) ;
	Unlock() ;
}

// 表示領域
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::GetSpriteRectangle
		( const wchar_t * pwszID, SGLRect& rectExt ) const
{
	bool	fHasRect = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		SGLRect		rect = pItem->GetItemOrgRect() ;
		S2DVector	vRect[4] ;
		vRect[0] = S2DVector( rect.left, rect.top ) ;
		vRect[1] = S2DVector( rect.right, rect.top ) ;
		vRect[2] = S2DVector( rect.left, rect.bottom ) ;
		vRect[3] = S2DVector( rect.right, rect.bottom ) ;
		//
		while ( pItem != nullptr )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				vRect[0] = pItem->LocalToGlobal( vRect[i] ) ;
			}
			SGLBasicForm *	pParent = pItem->GetParentForm() ;
			if ( pParent == nullptr )
			{
				break ;
			}
			pItem = pParent->GetOwnerItem() ;
		}
		S2DVector	vMin = vRect[0] ;
		S2DVector	vMax = vRect[0] ;
		for ( int i = 1; i < 4; i ++ )
		{
			vMin.x = esl_fminf( vMin.x, vRect[i].x ) ;
			vMin.y = esl_fminf( vMin.y, vRect[i].y ) ;
			vMax.x = esl_fmaxf( vMax.x, vRect[i].x ) ;
			vMax.y = esl_fmaxf( vMax.y, vRect[i].y ) ;
		}
		rectExt.left = (int32_t) esl_roundfi( vMin.x - 1.0f ) ;
		rectExt.top = (int32_t) esl_roundfi( vMin.y - 1.0f ) ;
		rectExt.right = (int32_t) esl_roundfi( vMax.x + 1.0f ) ;
		rectExt.bottom = (int32_t) esl_roundfi( vMax.y + 1.0f ) ;
		fHasRect = true ;
	}
	else
	{
		fHasRect = SGLSprite::GetSpriteRectangle( pwszID, rectExt ) ;
	}
	Unlock() ;
	return	fHasRect ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLSpriteFormed::GetSpriteText( const wchar_t * pwszID ) const
{
	SString	strText ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Sprite *
		pItem = (m_pForm != nullptr) ?
					m_pForm->GetItem<SGLBasicForm::Sprite>(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		SGLSprite *	pSprite = pItem->GetSprite() ;
		if ( pSprite != nullptr )
		{
			strText = pSprite->GetText() ;
		}
	}
	else
	{
		strText = SGLSprite::GetSpriteText( pwszID ) ;
	}
	Unlock() ;
	return	strText ;
}

void SGLSpriteFormed::SetSpriteText
	( const wchar_t * pwszID, const wchar_t * pwszText )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Sprite *
		pItem = (m_pForm != nullptr) ?
					m_pForm->GetItem<SGLBasicForm::Sprite>(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		SGLSprite *	pSprite = pItem->GetSprite() ;
		if ( pSprite != nullptr )
		{
			pSprite->SetText( pwszText ) ;
		}
	}
	else
	{
		SGLSprite::SetSpriteText( pwszID, pwszText ) ;
	}
	Unlock() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::SetSpriteTextFont
	( const wchar_t * pwszID, const wchar_t * pwszFont, int nSize )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Sprite *
		pItem = (m_pForm != nullptr) ?
					m_pForm->GetItem<SGLBasicForm::Sprite>(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		SGLSprite *	pSprite = pItem->GetSprite() ;
		if ( pSprite != nullptr )
		{
			pSprite->SetTextFont( pwszFont, nSize ) ;
		}
	}
	else
	{
		SGLSprite::SetSpriteTextFont( pwszID, pwszFont, nSize ) ;
	}
	Unlock() ;
}

// 画像属性
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::SetSpriteImage
	( const wchar_t * pwszID, const wchar_t * pwszImageID )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		SGLBasicForm::ImageSelector *
			pImageSel = ESLTypeCast<SGLBasicForm::ImageSelector>( pItem ) ;
		if ( (pImageSel == nullptr)
			|| !pImageSel->SelectImageAs( pwszImageID ) )
		{
			SGLBasicForm::Image *
				pImage = ESLTypeCast<SGLBasicForm::Image>( pItem ) ;
			SGLBasicFormParser *	pFormParser = m_refFormParser ;
			if ( (pImage != nullptr) && (pFormParser != nullptr) )
			{
				pImage->AttachImage( pFormParser->GetImageAs( pwszImageID ) ) ;
			}
		}
	}
	else
	{
		SGLSkinManager *	pSkin = m_refSkin ;
		if ( pSkin != nullptr )
		{
			SGLSkinManager::ImageDescription	imgdsc ;
			pSkin->GetRichImageAs( imgdsc, pwszImageID ) ;
			//
			SGLSprite *	pItem = GetItemAs( pwszID ) ;
			if ( pItem != NULL )
			{
				pItem->AttachAnimation( imgdsc.pImage, imgdsc.pRect ) ;
			}
		}
		else
		{
			SGLSprite::SetSpriteImage( pwszID, pwszImageID ) ;
		}
	}
	Unlock() ;
}

// 入力禁止状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::IsSpriteEnabled( const wchar_t * pwszID ) const
{
	bool	fEnabled = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		fEnabled = !pItem->IsDisabled() ;
	}
	else
	{
		fEnabled = SGLSprite::IsSpriteEnabled( pwszID ) ;
	}
	Unlock() ;
	return	fEnabled ;
}

void SGLSpriteFormed::SetSpriteEnable( const wchar_t * pwszID, bool fEnable )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->SetDisable( !fEnable ) ;
	}
	SGLSprite::SetSpriteEnable( pwszID, fEnable ) ;
	Unlock() ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteFormed::GetSpriteScrollPos
	( const wchar_t * pwszID, ScrollDirection scrlDir ) const
{
	int	nPos = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		nPos = pItem->GetBarPos() ;
	}
	else
	{
		nPos = SGLSprite::GetSpriteScrollPos( pwszID, scrlDir ) ;
	}
	Unlock() ;
	return	nPos ;
}

void SGLSpriteFormed::SetSpriteScrollPos
	( const wchar_t * pwszID, int nPos, ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->SetBarPos( (uint32_t) esl_max( nPos, 0 ) ) ;
	}
	SGLSprite::SetSpriteScrollPos( pwszID, nPos, scrlDir ) ;
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteFormed::GetSpriteScrollRange
	( const wchar_t * pwszID, ScrollDirection scrlDir ) const
{
	int	nRange = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		nRange = pItem->GetBarRange() ;
	}
	else
	{
		nRange = SGLSprite::GetSpriteScrollRange( pwszID, scrlDir ) ;
	}
	Unlock() ;
	return	nRange ;
}

void SGLSpriteFormed::SetSpriteScrollRange
	( const wchar_t * pwszID, int nRange, ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Item *
		pItem = (m_pForm != nullptr) ? m_pForm->GetItemAs(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->SetBarRange( (uint32_t) esl_max( nRange, 0 ) ) ;
	}
	SGLSprite::SetSpriteScrollRange( pwszID, nRange, scrlDir ) ;
	Unlock() ;
}

// ボタン属性
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::IsSpriteButtonChecked( const wchar_t * pwszID ) const
{
	bool	fChecked = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Button *
		pItem = (m_pForm != nullptr) ?
					m_pForm->GetItem<SGLBasicForm::Button>(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		fChecked = pItem->IsPushed() ;
	}
	else
	{
		fChecked = SGLSprite::IsSpriteButtonChecked( pwszID ) ;
	}
	Unlock() ;
	return	fChecked ;
}

void SGLSpriteFormed::CheckSpriteButton( const wchar_t * pwszID, bool fCheck )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLBasicForm::Button *
		pItem = (m_pForm != nullptr) ?
					m_pForm->GetItem<SGLBasicForm::Button>(pwszID) : nullptr ;
	if ( pItem != nullptr )
	{
		pItem->SetTogglePushed( fCheck ) ;
	}
	else
	{
		SGLSprite::CheckSpriteButton( pwszID, fCheck ) ;
	}
	Unlock() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	if ( m_pForm != NULL )
	{
		m_pForm->OnTimer( msecPast ) ;
		if ( m_pForm->GetUpdateFlag() )
		{
			m_pForm->ResetUpdateFlag() ;
			PostUpdate( NULL ) ;
		}
	}
}

// 更新領域通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::PostUpdate( SGLRect* pUpdate )
{
	if ( m_pForm != NULL )
	{
		if ( m_statusUpdate != updateFull )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			m_statusUpdate = updateFull ;
			//
			if ( GetFrameBuffer() != NULL )
			{
				NotifyUpdate() ;
			}
			else
			{
				SGLSprite *	pParent = m_refParent.GetReference() ;
				if ( pParent != NULL )
				{
					pParent->PostUpdate( NULL ) ;
				}
			}
			Unlock() ;
		}
	}
	else
	{
		SGLSprite::PostUpdate( pUpdate ) ;
	}
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::GetRectangle( SGLRect& rectExt ) const
{
	if ( (m_pForm != nullptr) && (GetFrameBuffer() == nullptr) )
	{
		rectExt = m_pForm->GetFormRect() ;
		rectExt.left -= 0x1000 ;
		rectExt.top -= 0x1000 ;
		rectExt.right += 0x1000 ;
		rectExt.bottom += 0x1000 ;
		return	LocalToGlobalRect( rectExt ) ;
	}
	else
	{
		return	SGLSprite::GetRectangle( rectExt ) ;
	}
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::IsHitSprite( double x, double y ) const
{
	if ( SGLSprite::IsHitSprite( x, y ) )
	{
		return	true ;
	}
	if ( m_pForm != NULL )
	{
		S2DVector	vPos( x, y ) ;
		return	m_pForm->TestHitCursor( vPos ) ;
	}
	return	false ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_pForm != NULL )
	{
		S2DVector	vPos( xPos, yPos ) ;
		if ( m_pForm->OnMouseMove( vPos ) )
		{
			PostUpdate() ;
			return	true ;
		}
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteFormed::OnMouseLeave( int64_t nFlags )
{
	if ( m_pForm != NULL )
	{
		m_pForm->OnMouseLeave() ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	if ( m_pForm != NULL )
	{
		S2DVector	vPos( xPos, yPos ) ;
		float32_t	z = (float32_t) zDelta / WheelDeltaUnit ;
		if ( m_pForm->OnMouseWheel( vPos, z ) )
		{
			return	true ;
		}
	}
	return	SGLSprite::OnMouseWheel( zDelta, xPos, yPos, nFlags ) ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFormed::OnButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_pForm != NULL )
	{
		S2DVector	vPos( xPos, yPos ) ;
		if ( m_pForm->OnClickDown( vPos, MouseButtonFromSpriteMouseFlags(nFlags) ) )
		{
			return	true ;
		}
	}
	return	SGLSprite::OnButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLSpriteFormed::OnButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_pForm != NULL )
	{
		S2DVector	vPos( xPos, yPos ) ;
		if ( m_pForm->OnClickUp( vPos, MouseButtonFromSpriteMouseFlags(nFlags) ) )
		{
			return	true ;
		}
	}
	return	SGLSprite::OnButtonUp( xPos, yPos, nFlags ) ;
}

SGLBasicForm::MouseButton
	SGLSpriteFormed::MouseButtonFromSpriteMouseFlags( int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	switch ( id )
	{
	case	LeftButtonID:
	default:
		return	SGLBasicForm::mouseLeft ;

	case	RightButtonID:
		return	SGLBasicForm::mouseRight ;

	case	MiddleButtonID:
		return	SGLBasicForm::mouseMiddle ;
	}
	return	SGLBasicForm::mouseLeft ;
}

// 画像描画リスト
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormed::OnDrawImage( SGLDrawImageParamList& dipl )
{
	if ( m_pForm != NULL )
	{
		m_pForm->DrawForm( dipl ) ;
	}
}

// 描画リスト取得
//////////////////////////////////////////////////////////////////////////////
SGLDrawImageParamList& SGLSpriteFormed::DrawImageParamList( void )
{
	return	m_dipList ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFormed::DuplicateObject( void )
{
	return	new SGLSpriteFormed( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFormed::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	SGLObjectSavingMapper *	pMapper = SGLObjectSavingMapper::GetCurrent() ;
	SString	strSkinName ;
	if ( pMapper != NULL )
	{
		strSkinName = pMapper->GetIdentityOf( (SGLObject*) m_refSkin.GetReference() ) ;
	}
	file.WriteString( strSkinName ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFormed::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	SString	strSkinName ;
	file.ReadString( strSkinName ) ;
	//
	SGLObjectSavingMapper *	pMapper = SGLObjectSavingMapper::GetCurrent() ;
	if ( pMapper != NULL )
	{
		AttachSkin
			( ESLTypeCast<SGLSkinManager>
					( pMapper->GetObjectOf( strSkinName ) ) ) ;
	}
	return	sglErrSuccess ;
}




//////////////////////////////////////////////////////////////////////////////
// ストレッチフレーム付きスプライト
//////////////////////////////////////////////////////////////////////////////

// スタイル
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFormFramed::FrameStyle::FrameStyle( void )
	: pImageSet( nullptr ),
		rectInnerMargin( 0, 0, 0, 0 ),
		rectBackMargin( 0, 0, 0, 0 ),
		rectCaptionMargin( 0, 0, 0, 0 )
{
}

SGLSpriteFormFramed::FrameStyle::FrameStyle( const FrameStyle& style )
	: SGLSpriteMessage::RichTextStyle( style ),
		pImageSet( style.pImageSet ),
		rectInnerMargin( style.rectInnerMargin ),
		rectBackMargin( style.rectBackMargin ),
		rectCaptionMargin( style.rectCaptionMargin )
{
}

const SGLSpriteFormFramed::FrameStyle&
	SGLSpriteFormFramed::FrameStyle::operator = ( const FrameStyle& style )
{
	SGLSpriteMessage::RichTextStyle::operator = ( style ) ;
	pImageSet = style.pImageSet ;
	rectInnerMargin = style.rectInnerMargin ;
	rectBackMargin = style.rectBackMargin ;
	rectCaptionMargin = style.rectCaptionMargin ;
	return	*this ;
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFormFramed, SGLSpriteFormed )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFormFramed::SGLSpriteFormFramed( void )
	: m_sizeFrame( 0, 0 ), m_sizeInner( 0, 0 ), m_flagInnerBuffer( false ),
		m_pFrame( nullptr ), m_pSubForm( nullptr ), m_pSubSprite( nullptr )
{
}

SGLSpriteFormFramed::SGLSpriteFormFramed( const SGLSpriteFormFramed& src )
	: SGLSpriteFormed( src ),
		m_sizeFrame( 0, 0 ), m_sizeInner( 0, 0 ), m_flagInnerBuffer( false ),
		m_pFrame( nullptr ), m_pSubForm( nullptr ), m_pSubSprite( nullptr )
{
	SetFrameStyle( src.GetFrameStyle() ) ;
	//
	if ( src.m_pFrame != nullptr )
	{
		CreateFrame( src.m_strCaption, src.m_sizeInner, src.m_flagInnerBuffer ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFormFramed::~SGLSpriteFormFramed( void )
{
	ReleaseFrame() ;
}

// フレームスタイルIDを指定してスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::AttachFrameStyle
	( SGLBasicFormParser * pFormParser, const wchar_t * pwszStyleID )
{
	AttachFormParser( pFormParser ) ;
	//
	if ( (pFormParser != nullptr) && (pwszStyleID != nullptr) )
	{
		const SSystem::SXMLDocument *
			pxmlStyle = pFormParser->GetStyleAs( pwszStyleID ) ;
		if ( pxmlStyle != nullptr )
		{
			ParseFrameStyle
				( m_styleFrame, GetFormParser(),
					m_strFontFace, m_strRubyFont, *pxmlStyle ) ;
		}
	}
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::SetFrameStyle( const SGLSpriteFormFramed::FrameStyle& style )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_styleFrame = style ;
	//
	m_strFontFace = style.font.pszFace ;
	if ( style.fontRuby.pszFace != nullptr )
	{
		m_strRubyFont = style.fontRuby.pszFace ;
	}
	else
	{
		m_strRubyFont = style.font.pszFace ;
	}
	m_strProhibition = m_styleFrame.context.pwszProhibition ;
	m_styleFrame.font.pszFace = m_strFontFace ;
	m_styleFrame.context.pwszProhibition = m_strProhibition ;
	if ( m_styleFrame.fontRuby.nSize == 0 )
	{
		m_styleFrame.fontRuby.nSize = m_styleFrame.font.nSize * 2 / 5 ;
	}
	m_styleFrame.fontRuby.pszFace = m_strRubyFont ;
	//
	PostUpdate() ;
	Unlock() ;
}

// スタイル取得
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteFormFramed::FrameStyle& SGLSpriteFormFramed::GetFrameStyle( void ) const
{
	return	m_styleFrame ;
}

// スタイル解釈
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::ParseFrameStyle
	( SGLSpriteFormFramed::FrameStyle& style,
		SGLBasicFormParser * pFormParser,
		SSystem::SString& strFontFace,
		SSystem::SString& strRubyFont,
		const SSystem::SXMLDocument& xmlStyle )
{
	SGLSpriteMessage::ParseRichTextStyle
		( style, strFontFace, strRubyFont, xmlStyle ) ;
	//
	style.pImageSet = nullptr ;
	if ( pFormParser != nullptr )
	{
		style.pImageSet =
			pFormParser->GetImageSetAs( xmlStyle.GetAttrStringAs( L"imgset" ) ) ;
	}
	//
	style.rectInnerMargin.left = (int32_t) xmlStyle.GetAttrIntegerAs( L"inner_margin_left", 0 ) ;
	style.rectInnerMargin.top = (int32_t) xmlStyle.GetAttrIntegerAs( L"inner_margin_top", 0 ) ;
	style.rectInnerMargin.right = (int32_t) xmlStyle.GetAttrIntegerAs( L"inner_margin_right", 0 ) ;
	style.rectInnerMargin.bottom = (int32_t) xmlStyle.GetAttrIntegerAs( L"inner_margin_bottom", 0 ) ;
	//
	style.rectBackMargin.left = (int32_t) xmlStyle.GetAttrIntegerAs( L"back_margin_left", 0 ) ;
	style.rectBackMargin.top = (int32_t) xmlStyle.GetAttrIntegerAs( L"back_margin_top", 0 ) ;
	style.rectBackMargin.right = (int32_t) xmlStyle.GetAttrIntegerAs( L"back_margin_right", 0 ) ;
	style.rectBackMargin.bottom = (int32_t) xmlStyle.GetAttrIntegerAs( L"back_margin_bottom", 0 ) ;
	//
	style.rectCaptionMargin.left = (int32_t) xmlStyle.GetAttrIntegerAs( L"caption_left", 0 ) ;
	style.rectCaptionMargin.top = (int32_t) xmlStyle.GetAttrIntegerAs( L"caption_top", 0 ) ;
	style.rectCaptionMargin.right = (int32_t) xmlStyle.GetAttrIntegerAs( L"caption_right", 0 ) ;
	style.rectCaptionMargin.bottom = 0 ;
}

// フレーム作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFormFramed::CreateFrame
	( const wchar_t * pwszCaption,
		const SGLSize& sizeInner, bool flagInnerBuffer )
{
	ReleaseFrame() ;
	//
	LockTrace( __FILE__, __LINE__ ) ;
	m_sizeFrame.w = sizeInner.w + m_styleFrame.rectInnerMargin.left
								+ m_styleFrame.rectInnerMargin.right ;
	m_sizeFrame.h = sizeInner.h + m_styleFrame.rectInnerMargin.top
								+ m_styleFrame.rectInnerMargin.bottom ;
	m_sizeInner = sizeInner ;
	m_flagInnerBuffer = flagInnerBuffer ;
	//
	SGLBasicForm *	pForm = new SGLBasicForm ;
	pForm->AttachFormParser( GetFormParser() ) ;
	pForm->SetFormRect( SGLImageRect( 0, 0, m_sizeFrame.w, m_sizeFrame.h ) ) ;
	SetBasicForm( pForm ) ;
	//
	m_pFrame = new SGLBasicForm::StretchFrame ;
	m_pFrame->AttachImageSet( m_styleFrame.pImageSet ) ;
	m_pFrame->SetInnerMargin( m_styleFrame.rectInnerMargin ) ;
	m_pFrame->SetBackFrameMargin( m_styleFrame.rectBackMargin ) ;
	m_pFrame->SetFrameSize( m_sizeFrame ) ;
	pForm->AddItem( m_pFrame ) ;
	Unlock() ;
	//
	SetCaptionText( pwszCaption ) ;
	//
	return	sglErrSuccess ;
}

// フレーム削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::ReleaseFrame( void )
{
	if ( m_pFrame != nullptr )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		SetBasicForm( nullptr ) ;
		m_pFrame = nullptr ;
		m_pSubForm = nullptr ;
		m_pSubSprite = nullptr ;
		m_imgCaption = nullptr ;
		m_strCaption.FreeArray() ;
		Unlock() ;
	}
}

// フレーム取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm::StretchFrame * SGLSpriteFormFramed::GetFrame( void ) const
{
	return	m_pFrame ;
}

// クライアント領域アイテム取得
//////////////////////////////////////////////////////////////////////////////
SGLBasicForm * SGLSpriteFormFramed::GetClientForm( void )
{
	if ( m_pSubForm != nullptr )
	{
		return	m_pSubForm->GetForm() ;
	}
	if ( m_pFrame != nullptr )
	{
		SGLBasicForm *	pForm = GetBasicForm() ;
		if ( pForm != nullptr )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			m_pSubForm = new SGLBasicForm::SubForm ;
			if ( m_flagInnerBuffer )
			{
				m_pSubForm->CreateBuffer
					( (uint32_t) m_sizeInner.w,
						(uint32_t) m_sizeInner.h,
						SGLImageObject::bufferOnMemory ) ;
			}
			m_pSubForm->SetPosition
				( S2DVector( m_styleFrame.rectInnerMargin.left,
								m_styleFrame.rectInnerMargin.top ) ) ;
			//
			pForm->InsertItem( 0, m_pSubForm ) ;
			Unlock() ;
			//
			return	m_pSubForm->GetForm() ;
		}
	}
	return	nullptr ;
}

SGLSpriteFormed * SGLSpriteFormFramed::GetClientSprite( void )
{
	if ( m_pSubSprite != nullptr )
	{
		return	ESLTypeCast<SGLSpriteFormed>( m_pSubSprite->GetSprite() ) ;
	}
	if ( m_pFrame != nullptr )
	{
		SGLBasicForm *	pForm = GetBasicForm() ;
		if ( pForm != nullptr )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			SGLSpriteFormed *	pFormSprite = new SGLSpriteFormed ;
			pFormSprite->AttachFormParser( GetFormParser() ) ;
			//
			if ( m_flagInnerBuffer )
			{
				pFormSprite->CreateBuffer
					( (uint32_t) m_sizeInner.w,
						(uint32_t) m_sizeInner.h,
						formatImageARGB, 32,
						SGLImageObject::bufferOnMemory ) ;
			}
			m_pSubSprite = new SGLBasicForm::Sprite ;
			m_pSubSprite->SetSprite( pFormSprite ) ;
			m_pSubSprite->SetPosition
				( S2DVector( m_styleFrame.rectInnerMargin.left,
								m_styleFrame.rectInnerMargin.top ) ) ;
			//
			pForm->InsertItem( 0, m_pSubSprite ) ;
			Unlock() ;
			//
			return	pFormSprite ;
		}
	}
	return	nullptr ;
}

// クライアントサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFormFramed::ResizeFrame( const SGLSize& sizeInner )
{
	if ( m_pFrame == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( m_sizeInner == sizeInner )
	{
		return	sglErrSuccess ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	m_sizeFrame.w = sizeInner.w + m_styleFrame.rectInnerMargin.left
								+ m_styleFrame.rectInnerMargin.right ;
	m_sizeFrame.h = sizeInner.h + m_styleFrame.rectInnerMargin.top
								+ m_styleFrame.rectInnerMargin.bottom ;
	m_sizeInner = sizeInner ;
	//
	SGLBasicForm *	pForm = GetBasicForm() ;
	if ( pForm != nullptr )
	{
		pForm->SetFormRect( SGLImageRect( 0, 0, m_sizeFrame.w, m_sizeFrame.h ) ) ;
	}
	m_pFrame->SetFrameSize( m_sizeFrame ) ;
	//
	if ( m_flagInnerBuffer )
	{
		if ( m_pSubForm != nullptr )
		{
			m_pSubForm->CreateBuffer
				( (uint32_t) m_sizeInner.w,
					(uint32_t) m_sizeInner.h,
					SGLImageObject::bufferOnMemory ) ;
		}
		if ( m_pSubSprite != nullptr )
		{
			SGLSprite *	pSprite = m_pSubSprite->GetSprite() ;
			if ( pSprite != nullptr )
			{
				pSprite->CreateBuffer
					( (uint32_t) m_sizeInner.w,
						(uint32_t) m_sizeInner.h,
						formatImageARGB, 32,
						SGLImageObject::bufferOnMemory ) ;
			}
		}
	}
	PostUpdate() ;
	Unlock() ;
	return	sglErrSuccess ;
}

// キャプション変更
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::SetCaptionText( const wchar_t * pwszCaption )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_strCaption != pwszCaption )
	{
		m_strCaption = pwszCaption ;
		UpdateCaption() ;
	}
	Unlock() ;
}

void SGLSpriteFormFramed::UpdateCaption( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_strCaption.IsEmpty() )
	{
		m_imgCaption = nullptr ;
		Unlock() ;
		return ;
	}
	SString		strCaption = m_strCaption ;
	SGLSpriteMessage::RichTextStyle
				style = m_styleFrame ;
	style.context.rectWritable.left = m_styleFrame.rectCaptionMargin.left ;
	style.context.rectWritable.top = m_styleFrame.rectCaptionMargin.top ;
	style.context.rectWritable.right = m_sizeFrame.w - m_styleFrame.rectCaptionMargin.right ;
	style.context.rectWritable.bottom = m_sizeFrame.h ;
	style.context.ptStartWriting.x = style.context.rectWritable.left ;
	style.context.ptStartWriting.y = style.context.rectWritable.top ;
	Unlock() ;
	//
	SGLSpriteMessage	msg ;
	msg.SetRichTextStyle( style ) ;
	msg.AddMessageXML( strCaption ) ;
	//
	SSmartPointer<SGLImageObject>	imgCaption = new SGLImage ;
	SGLPoint	ptOffset ;
	if ( msg.RasterizeTextImage( *imgCaption, ptOffset ) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_imgCaption = nullptr ;
		PostUpdate() ;
		Unlock() ;
	}
	else
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_imgCaption = imgCaption.Detach() ;
		m_ptCaptionOffset = ptOffset ;
		PostUpdate() ;
		Unlock() ;
	}
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFormFramed::PrepareDrawFrame( void )
{
	SGLSpriteFormed::PrepareDrawFrame() ;
	//
	if ( m_imgCaption != nullptr )
	{
		SGLPaintParam	ppCaption ;
		ppCaption.ptPaint = m_ptCaptionOffset ;
		m_dipList.AddDrawParam( ppCaption, m_imgCaption ) ;
	}
}


