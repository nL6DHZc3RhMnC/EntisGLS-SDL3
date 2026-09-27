
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_frame.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// フレームスタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrame::FrameStyle::FrameStyle( void )
{
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrame::FrameStyle::FrameStyle
	( const SGLSpriteFrame::FrameStyle& style )
{
	for ( size_t i = 0; i < frameCount; i ++ )
	{
		imgdscParts[i] = style.imgdscParts[i] ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteFrame::FrameStyle&
	SGLSpriteFrame::FrameStyle::operator =
		( const SGLSpriteFrame::FrameStyle& style )
{
	for ( size_t i = 0; i < frameCount; i ++ )
	{
		imgdscParts[i] = style.imgdscParts[i] ;
	}
	return	*this ;
}

// フレームサイズ（太さ）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrame::FrameStyle::GetFrameThickness( SGLRect& rect ) const
{
	rect.left = imgdscParts[frameLeft].rectImage.w ;
	rect.top = imgdscParts[frameUpper].rectImage.h ;
	rect.right = imgdscParts[frameRight].rectImage.w ;
	rect.bottom= imgdscParts[frameUnder].rectImage.h ;
}


//////////////////////////////////////////////////////////////////////////////
// フレーム表示スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFrame, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrame::SGLSpriteFrame( void )
{
}

SGLSpriteFrame::SGLSpriteFrame( const SGLSpriteFrame& src )
	: SGLSprite( src ),
		m_styleFrame( src.m_styleFrame ), m_sizeFrame( src.m_sizeFrame )
{
	for ( size_t i = 0; i < frameCount; i ++ )
	{
		m_refFrame[i] = src.m_refFrame[i] ;
		m_styleFrame.imgdscParts[i].pImage = m_refFrame[i] ;
	}
	UpdateFrameImage() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrame::~SGLSpriteFrame( void )
{
	DetachSyncTimeout( 100 ) ;
}

// フレームスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrame::SetFrameStyle( const SGLSpriteFrame::FrameStyle& style )
{
	Lock() ;
	m_styleFrame = style ;
	for ( size_t i = 0; i < frameCount; i ++ )
	{
		m_refFrame[i] = m_styleFrame.imgdscParts[i].pImage ;
	}
	UpdateFrameImage() ;
	Unlock() ;
}

// フレーム矩形設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrame::SetFrameSize( const SGLSize& size )
{
	Lock() ;
	m_sizeFrame = size ;
	UpdateFrameImage() ;
	Unlock() ;
}


// 文字画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrame::UpdateFrameImage( void )
{
	AttachImage( NULL ) ;
	//
	for ( size_t i = 0; i < frameCount; i ++ )
	{
		m_styleFrame.imgdscParts[i].pImage = m_refFrame[i] ;
	}
	m_pFrameImage =
		CreateFrameImage( m_styleFrame, m_sizeFrame, m_rectInner ) ;
	if ( m_pFrameImage != NULL )
	{
		AttachImage( m_pFrameImage ) ;
	}
}
// 画像化したフレームを生成する
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteFrame::CreateFrameImage
	( const SGLSpriteFrame::FrameStyle& style,
			const SGLSize& size, SGLImageRect& rectInner )
{
	//
	// 最小サイズ判定
	//
	const int	widthLeftFrame = style.imgdscParts[frameLeft].rectImage.w ;
	const int	widthRightFrame = style.imgdscParts[frameRight].rectImage.w ;
	const int	heightUpperFrame = style.imgdscParts[frameUpper].rectImage.h ;
	const int	heightUnderFrame = style.imgdscParts[frameUnder].rectImage.h ;
	if ( (size.w < widthLeftFrame + widthRightFrame)
		|| (size.h < heightUpperFrame + heightUnderFrame) )
	{
		rectInner.Clear() ;
		return	NULL ;
	}
	//
	// ループ回数計算
	//
	const int	widthPane = style.imgdscParts[framePane].rectImage.w ;
	const int	heightPane = style.imgdscParts[framePane].rectImage.h ;
	size_t	countHorzPane = 0 ;
	if ( widthPane != 0 )
	{
		countHorzPane =
			(size.w - widthLeftFrame - widthRightFrame) / widthPane ;
	}
	size_t	countVertPane = 0 ;
	if ( heightPane != 0 )
	{
		countVertPane =
			(size.h - heightUpperFrame - heightUnderFrame) / heightPane ;
	}
	//
	// 画像バッファ生成
	//
	rectInner.x = widthLeftFrame ;
	rectInner.y = heightUpperFrame ;
	rectInner.w = (int32_t) (countHorzPane * widthPane) ;
	rectInner.h = (int32_t) (countVertPane * heightPane) ;
	//
	SGLImage *	pImage = new SGLImage ;
	if ( pImage->CreateImage
		( (uint32_t) (widthLeftFrame + widthRightFrame + rectInner.w),
			(uint32_t) (heightUpperFrame + heightUnderFrame + rectInner.h),
			formatImageARGB, 32 ) )
	{
		delete	pImage ;
		return	NULL ;
	}
	//
	// 上辺描画
	//
	SGLPaintContext	paint ;
	SGLPaintParam	ppParam ;
	size_t			i, j ;
	paint.AttachTargetImage( pImage, NULL ) ;
	//
	ppParam.ptPaint.x = 0 ;
	ppParam.ptPaint.y = 0 ;
	if ( style.imgdscParts[frameUpperLeft].pImage != NULL )
	{
		paint.DrawImage
			( ppParam, style.imgdscParts[frameUpperLeft].pImage,
							style.imgdscParts[frameUpperLeft].pRect ) ;
	}
	ppParam.ptPaint.x += widthLeftFrame ;
	//
	for ( i = 0; i < countHorzPane; i ++ )
	{
		if ( style.imgdscParts[frameUpper].pImage != NULL )
		{
			paint.DrawImage
				( ppParam, style.imgdscParts[frameUpper].pImage,
								style.imgdscParts[frameUpper].pRect ) ;
		}
		ppParam.ptPaint.x += widthPane ;
	}
	if ( style.imgdscParts[frameUpperRight].pImage != NULL )
	{
		paint.DrawImage
			( ppParam, style.imgdscParts[frameUpperRight].pImage,
							style.imgdscParts[frameUpperRight].pRect ) ;
	}
	ppParam.ptPaint.y += heightUpperFrame ;
	//
	// 中央描画
	//
	for ( j = 0; j < countVertPane; j ++ )
	{
		ppParam.ptPaint.x = 0 ;
		if ( style.imgdscParts[frameLeft].pImage != NULL )
		{
			paint.DrawImage
				( ppParam, style.imgdscParts[frameLeft].pImage,
								style.imgdscParts[frameLeft].pRect ) ;
		}
		ppParam.ptPaint.x += widthLeftFrame ;
		//
		for ( i = 0; i < countHorzPane; i ++ )
		{
			if ( style.imgdscParts[framePane].pImage != NULL )
			{
				paint.DrawImage
					( ppParam, style.imgdscParts[framePane].pImage,
									style.imgdscParts[framePane].pRect ) ;
			}
			ppParam.ptPaint.x += widthPane ;
		}
		if ( style.imgdscParts[frameRight].pImage != NULL )
		{
			paint.DrawImage
				( ppParam, style.imgdscParts[frameRight].pImage,
								style.imgdscParts[frameRight].pRect ) ;
		}
		ppParam.ptPaint.y += heightPane ;
	}
	//
	// 下辺描画
	//
	ppParam.ptPaint.x = 0 ;
	if ( style.imgdscParts[frameUnderLeft].pImage != NULL )
	{
		paint.DrawImage
			( ppParam, style.imgdscParts[frameUnderLeft].pImage,
							style.imgdscParts[frameUnderLeft].pRect ) ;
	}
	ppParam.ptPaint.x += widthLeftFrame ;
	//
	for ( i = 0; i < countHorzPane; i ++ )
	{
		if ( style.imgdscParts[frameUnder].pImage != NULL )
		{
			paint.DrawImage
				( ppParam, style.imgdscParts[frameUnder].pImage,
								style.imgdscParts[frameUnder].pRect ) ;
		}
		ppParam.ptPaint.x += widthPane ;
	}
	if ( style.imgdscParts[frameUnderRight].pImage != NULL )
	{
		paint.DrawImage
			( ppParam, style.imgdscParts[frameUnderRight].pImage,
							style.imgdscParts[frameUnderRight].pRect ) ;
	}
	paint.Flush() ;
	//
	return	pImage ;
}

// フレームスタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrame::ParseFrameStyle
	( SGLSkinManager& skin,
		SGLSpriteFrame::FrameStyle& style,
		const SSystem::SXMLDocument& xmlStyle )
{
	SXMLDocument *	pxmlImage = xmlStyle.GetElementTagAs( L"image" ) ;
	if ( pxmlImage != NULL )
	{
		skin.GetRichImageAs
			( style.imgdscParts[frameUpperLeft],
				pxmlImage->GetAttrStringAs( L"upper_left" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameUpper],
				pxmlImage->GetAttrStringAs( L"upper" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameUpperRight],
				pxmlImage->GetAttrStringAs( L"upper_right" ) ) ;
		//
		skin.GetRichImageAs
			( style.imgdscParts[frameLeft],
				pxmlImage->GetAttrStringAs( L"left" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[framePane],
				pxmlImage->GetAttrStringAs( L"pane" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameRight],
				pxmlImage->GetAttrStringAs( L"right" ) ) ;
		//
		skin.GetRichImageAs
			( style.imgdscParts[frameUnderLeft],
				pxmlImage->GetAttrStringAs( L"under_left" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameUnder],
				pxmlImage->GetAttrStringAs( L"under" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameUnderRight],
				pxmlImage->GetAttrStringAs( L"under_right" ) ) ;
	}
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFrame::DuplicateObject( void )
{
	return	new SGLSpriteFrame( *this ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 可変クライアントビュー・スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteLayoutView, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteLayoutView::SGLSpriteLayoutView( void )
{
}

SGLSpriteLayoutView::SGLSpriteLayoutView( const SGLSpriteLayoutView& src )
	: SGLSprite( src ),
		m_refClient( src.m_refClient ), m_sizeLayout( src.m_sizeLayout )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteLayoutView::~SGLSpriteLayoutView( void )
{
}

// クライアント設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteLayoutView::AttachClientView( SGLSpriteLayoutView * pView )
{
	Lock() ;
	//
	SGLSpriteLayoutView *	pLastView = m_refClient.GetReference() ;
	if ( pLastView != NULL )
	{
		DetachChild( pLastView ) ;
		OnDettachedClientView( pLastView ) ;
	}
	m_refClient.SetReference( pView ) ;
	//
	SGLImageRect	rect ;
	CalculateClientRect( rect ) ;
	pView->SetPosition( rect.x, rect.y ) ;
	pView->OnChangeSize( rect.GetSize() ) ;
	//
	AddChild( pView ) ;
	OnAttachedClientView( pView ) ;
	//
	Unlock() ;
}

void SGLSpriteLayoutView::AttachSmartClientView( SGLSpriteLayoutView * pView )
{
	Lock() ;
	//
	SGLSpriteLayoutView *	pLastView = m_refClient.GetReference() ;
	if ( pLastView != NULL )
	{
		DetachChild( pLastView ) ;
		OnDettachedClientView( pLastView ) ;
	}
	m_refClient.SetSmartReference( pView ) ;
	//
	SGLImageRect	rect ;
	CalculateClientRect( rect ) ;
	pView->SetPosition( rect.x, rect.y ) ;
	pView->OnChangeSize( rect.GetSize() ) ;
	//
	AddChild( pView ) ;
	OnAttachedClientView( pView ) ;
	//
	Unlock() ;
}

// クライアント取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteLayoutView * SGLSpriteLayoutView::GetClientView( void ) const
{
	return	m_refClient.GetReference() ;
}

// クライアント分離
//////////////////////////////////////////////////////////////////////////////
SGLSpriteLayoutView * SGLSpriteLayoutView::DetachClientView( void )
{
	SGLSpriteLayoutView *	pLastView = m_refClient.DetachReference() ;
	if ( pLastView != NULL )
	{
		DetachChild( pLastView ) ;
		OnDettachedClientView( pLastView ) ;
	}
	return	pLastView ;
}

// レイアウトサイズ取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteLayoutView::GetViewLayoutSize( SGLSize& sizeLayout ) const
{
	sizeLayout = GetImageSize() ;
	if ( sizeLayout.w < m_sizeView.w )
	{
		sizeLayout.w = m_sizeView.w ;
	}
	if ( sizeLayout.h < m_sizeView.h )
	{
		sizeLayout.h = m_sizeView.h ;
	}
	return	!sizeLayout.IsEmpty() ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteLayoutView::GetRectangle( SGLRect& rectExt ) const
{
	SGLRect	rect ;
	SGLSize	sizeLayout ;
	rect.SetPosition( SGLPoint( 0, 0 ) ) ;
	if ( GetViewLayoutSize( sizeLayout ) )
	{
		rect.SetSize( sizeLayout ) ;
	}
	else
	{
		rect.SetSize( GetImageSize() ) ;
	}
	if ( !LocalToGlobalRect( rect ) )
	{
		return	SGLSprite::GetRectangle( rectExt ) ;
	}
	if ( SGLSprite::GetRectangle( rectExt ) )
	{
		rectExt |= rect ;
	}
	else
	{
		rectExt = rect ;
	}
	return	true ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteLayoutView::IsHitSprite( double x, double y ) const
{
	if ( (x >= 0.0) && (x < m_sizeLayout.w)
				&& (y >= 0.0) && (y < m_sizeLayout.h) )
	{
		return	true ;
	}
	return	SGLSprite::IsHitSprite( x, y ) ;
}

// サイズ変更通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteLayoutView::OnChangeSize( const SGLSize& sizeView )
{
	m_sizeLayout = sizeView ;
	m_sizeView = sizeView ;
	//
	SGLSpriteLayoutView *	pView = m_refClient.GetReference() ;
	if ( pView != NULL )
	{
		SGLImageRect	rect ;
		CalculateClientRect( rect ) ;
		pView->SetPosition( rect.x, rect.y ) ;
		pView->OnChangeSize( rect.GetSize() ) ;
	}
}

// クライアントサイズ計算
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteLayoutView::CalculateClientRect( SGLImageRect& rectClient ) const
{
	rectClient.x = 0 ;
	rectClient.y = 0 ;
	rectClient.SetSize( m_sizeLayout ) ;
}

// クライアントが設定された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteLayoutView::OnAttachedClientView( SGLSpriteLayoutView * pView )
{
}

// クライアントが分離された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteLayoutView::OnDettachedClientView( SGLSpriteLayoutView * pView )
{
}


//////////////////////////////////////////////////////////////////////////////
// スクロールビュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteScrollView, SGLSpriteLayoutView )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollView::SGLSpriteScrollView( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollView::~SGLSpriteScrollView( void )
{
}

// スクロール領域更新
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollView::UpdateScrollRange( void )
{
	SGLSpriteLayoutView *	pView = m_refClient.GetReference() ;
	if ( pView != NULL )
	{
		SGLSize	sizeLayout ;
		if ( pView->GetViewLayoutSize( sizeLayout ) )
		{
			SGLImageRect	rectViewPort ;
			rectViewPort.SetPosition( SGLPoint( 0, 0 ) ) ;
			rectViewPort.SetSize( m_sizeLayout ) ;
			m_scroller.SetScrollViewPort( sizeLayout, rectViewPort ) ;
		}
	}
}

// サイズ変更通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollView::OnChangeSize( const SGLSize& sizeView )
{
	SGLSpriteLayoutView *	pView = m_refClient.GetReference() ;
	S2DDVector	vViewPos ;
	if ( pView != NULL )
	{
		vViewPos = pView->GetPosition() ;
	}
	SGLSpriteLayoutView::OnChangeSize( sizeView ) ;
	//
	if ( pView != NULL )
	{
		pView->SetPosition( vViewPos.x, vViewPos.y ) ;
	}
	//
	SGLSize sizeImage = GetImageSize() ;
	if ( sizeImage != sizeView )
	{
		CreateBuffer( sizeView.w, sizeView.h ) ;
	}
	UpdateScrollRange() ;
}

// クライアントが設定された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollView::OnAttachedClientView( SGLSpriteLayoutView * pView )
{
	m_scroller.AttachScrollerTo( *pView, true ) ;
}

// クライアントが分離された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollView::OnDettachedClientView( SGLSpriteLayoutView * pView )
{
	m_scroller.DetachScrollerFrom( *pView ) ;
}


//////////////////////////////////////////////////////////////////////////////
// リストビュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteListView, SGLSpriteLayoutView )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteListView::SGLSpriteListView( void )
{
	m_yListStep = 16 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteListView::~SGLSpriteListView( void )
{
}

// リスト間隔設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::SetListLineHeight( int yStep )
{
	m_yListStep = yStep ;
	UpdateChildrenPosition() ;
	UpdateChildrenSize() ;
}

// リスト間隔取得
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteListView::GetListLineHeight( void ) const
{
	return	m_yListStep ;
}

// アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteListView::GetListItemCount( void ) const
{
	return	m_lstItems.GetLength() ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteLayoutView * SGLSpriteListView::GetListItemAt( size_t nIndex ) const
{
	return	m_lstItems.GetAt( nIndex ) ;
}

// アイテム追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteListView::AddListItem( SGLSpriteLayoutView * pItem )
{
	ESLAssert( pItem != NULL ) ;
	size_t	iItem ;
	Lock() ;
	SGLSize	sizeView( m_sizeLayout.w, m_yListStep ) ;
	pItem->OnChangeSize( sizeView ) ;
	iItem = m_lstItems.Add( pItem ) ;
	AddChild( pItem ) ;
	UpdateChildrenPosition() ;
	NotifyUpdate() ;
	Unlock() ;
	return	iItem ;
}

size_t SGLSpriteListView::InsertListItem( size_t nIndex, SGLSpriteLayoutView * pItem )
{
	ESLAssert( pItem != NULL ) ;
	Lock() ;
	SGLSize	sizeView( m_sizeLayout.w, m_yListStep ) ;
	pItem->OnChangeSize( sizeView ) ;
	m_lstItems.InsertAt( nIndex, pItem ) ;
	AddChild( pItem ) ;
	UpdateChildrenPosition() ;
	Unlock() ;
	return	nIndex ;
}

// アイテム削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::RemoveListItem( size_t nIndex )
{
	Lock() ;
	DetachChild( m_lstItems.GetAt( nIndex ) ) ;
	m_lstItems.RemoveAt( nIndex ) ;
	UpdateChildrenPosition() ;
	Unlock() ;
}

SGLSpriteLayoutView * SGLSpriteListView::DetachListItem( size_t nIndex )
{
	Lock() ;
	SGLSpriteLayoutView *	pView = m_lstItems.DetachAt( nIndex ) ;
	if ( pView != NULL )
	{
		DetachChild( pView ) ;
	}
	UpdateChildrenPosition() ;
	Unlock() ;
	return	pView ;
}

// アイテム全削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::RemoveAllListItems( void )
{
	Lock() ;
	NotifyUpdate() ;
	//
	size_t	nCount = m_lstItems.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DetachChild( m_lstItems.GetAt( i ) ) ;
	}
	m_lstItems.RemoveAll() ;
	m_sizeView.h = 0 ;
	Unlock() ;
}

// アイテム検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteListView::FindListItem( SGLSpriteLayoutView * pItem ) const
{
	return	m_lstItems.FindPtr( pItem ) ;
}

// 子ビュー（リストアイテム）座標更新
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::UpdateChildrenPosition( void )
{
	Lock() ;
	size_t	nCount = m_lstItems.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSpriteLayoutView *	pView = m_lstItems.GetAt( i ) ;
		if ( pView != NULL )
		{
			pView->SetPosition( 0, (double) (i * m_yListStep) ) ;
		}
	}
	m_sizeView.h = (int32_t) (nCount * m_yListStep) ;
	Unlock() ;
}

// 子ビューサイズ更新
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::UpdateChildrenSize( void )
{
	Lock() ;
	SGLSize	sizeView( m_sizeLayout.w, m_yListStep ) ;
	size_t	nCount = m_lstItems.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSpriteLayoutView *	pView = m_lstItems.GetAt( i ) ;
		if ( pView != NULL )
		{
			pView->OnChangeSize( sizeView ) ;
		}
	}
	m_sizeView.h = (int32_t) (nCount * m_yListStep) ;
	PostUpdate() ;
	Unlock() ;
}

// レイアウトサイズ取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteListView::GetViewLayoutSize( SGLSize& sizeLayout ) const
{
	sizeLayout.w = m_sizeLayout.w ;
	sizeLayout.h = (int32_t) (m_lstItems.GetLength() * m_yListStep) ;
	return	!sizeLayout.IsEmpty() ;
}

// サイズ変更通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteListView::OnChangeSize( const SGLSize& sizeView )
{
	SGLSpriteLayoutView::OnChangeSize( sizeView ) ;
	UpdateChildrenSize() ;
}


//////////////////////////////////////////////////////////////////////////////
// フレームビュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFrameView, SGLSpriteLayoutView )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrameView::SGLSpriteFrameView( void )
{
	m_frame.ChangePriority( 0x7FFFFFFF ) ;
	AddChild( &m_frame ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFrameView::~SGLSpriteFrameView( void )
{
	DetachChild( &m_frame ) ;
}

// フレーム作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFrameView::CreateFrame
	( const SGLSpriteFrame::FrameStyle& style, const SGLSize& size )
{
	m_frame.SetFrameStyle( style ) ;
	SetFrameSize( size ) ;
	return	sglErrSuccess ;
}

// フレーム矩形設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrameView::SetFrameSize( const SGLSize& size )
{
	m_frame.SetFrameSize( size ) ;
	//
	const SGLImageRect&		rectInner = m_frame.GetInnerRect() ;
	SGLSpriteLayoutView *	pView = m_refClient.GetReference() ;
	if ( pView != NULL )
	{
		pView->SetPosition( rectInner.x, rectInner.y ) ;
		pView->OnChangeSize( rectInner.GetSize() ) ;
	}
}

// サイズ変更通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrameView::OnChangeSize( const SGLSize& sizeView )
{
	SGLSpriteLayoutView::OnChangeSize( sizeView ) ;
	SetFrameSize( sizeView ) ;
}

// クライアントサイズ計算
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFrameView::CalculateClientRect( SGLImageRect& rectClient ) const
{
	rectClient = m_frame.GetInnerRect() ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易ボタンビュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteTouchableView, SGLSpriteLayoutView )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteTouchableView::SGLSpriteTouchableView( void )
{
	m_status = statusNormal ;
	m_transDisabled = 0x80 ;
	m_flagSelected = false ;
	m_flagSelectable = true ;
	m_pAudioFocus = NULL ;
	m_pAudioPushed = NULL ;
}

// 背景色設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::SetNormalViewColor( uint32_t rgbaNormal )
{
	m_rgbaBackColor[statusNormal] = rgbaNormal ;
}

void SGLSpriteTouchableView::SetFocusViewColor( uint32_t rgbaFocus )
{
	m_rgbaBackColor[statusFocus] = rgbaFocus ;
}

void SGLSpriteTouchableView::SetPushedViewColor( uint32_t rgbaPushed )
{
	m_rgbaBackColor[statusPushed] = rgbaPushed ;
}

void SGLSpriteTouchableView::SetSelectedViewColor( uint32_t rgbaSelected )
{
	m_rgbaBackColor[statusSelected] = rgbaSelected ;
}

// 効果音設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::SetFocusSE( SGLAudioPlayer * pAudio )
{
	Lock() ;
	m_pAudioFocus = pAudio ;
	Unlock() ;
}

void SGLSpriteTouchableView::SetPushedSE( SGLAudioPlayer * pAudio )
{
	Lock() ;
	m_pAudioPushed = pAudio ;
	Unlock() ;
}

// 禁止状態透明度
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::SetDisabledTransparency( uint32_t nTransparency )
{
	m_transDisabled = nTransparency ;
}

// ボタン属性
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteTouchableView::IsButtonChecked( void )
{
	return	m_flagSelected ;
}

void SGLSpriteTouchableView::CheckButton( bool fCheck )
{
	Lock() ;
	m_flagSelected = fCheck ;
	if ( m_status != statusFocus )
	{
		m_status = (TouchStatus) (fCheck ? statusSelected : statusNormal) ;
		PostUpdate() ;
	}
	Unlock() ;
}

// 選択可能状態
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::SetSelectable( bool fSelectable )
{
	Lock() ;
	m_flagSelectable = fSelectable ;
	if ( !fSelectable )
	{
		m_status = statusNormal ;
	}
	PostUpdate() ;
	Unlock() ;
}

bool SGLSpriteTouchableView::IsSelectable( void ) const
{
	return	m_flagSelectable ;
}

// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	SGLAffine	af ;
	pp.GetAffine( af ) ;
	render.PushTransformation() ;
	render.AppendTransformation( af, pp.nTransparency ) ;
	render.FillRectangle
		( 0, 0, m_sizeLayout.w, m_sizeLayout.h, m_rgbaBackColor[m_status] ) ;
	render.PopTransformation() ;
	//
	SGLSpriteLayoutView::DrawSprite( render, pp, image ) ;
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::DrawChildren
	( S3DRenderContextInterface& render,
			SGLSprite::Stereo3DView s3dView ) const
{
	if ( IsEnabled() && m_flagSelectable )
	{
		SGLSpriteLayoutView::DrawChildren( render, s3dView ) ;
	}
	else
	{
		SGLAffine	afI ;
		render.PushTransformation() ;
		render.AppendTransformation( afI, m_transDisabled ) ;
		SGLSpriteLayoutView::DrawChildren( render, s3dView ) ;
		render.PopTransformation() ;
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteTouchableView::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( pChild != NULL )
	{
		pChild->OnMouseMove( vPos.x, vPos.y, nFlags ) ;
		//
		m_status =
			(TouchStatus) (m_flagSelected ? statusSelected : statusNormal) ;
		PostUpdate() ;
		return	true ;
	}
	if ( m_flagSelectable )
	{
		if ( (m_status == statusNormal)
			|| (m_status == statusSelected) )
		{
			if ( m_pAudioFocus != NULL )
			{
				m_pAudioFocus->Play() ;
			}
			m_status = statusFocus ;
			PostUpdate() ;
		}
	}
	SGLSpriteLayoutView::OnMouseMove( xPos, yPos, nFlags ) ;
	return	true ;
}

void SGLSpriteTouchableView::OnMouseLeave( int64_t nFlags )
{
	SGLSpriteLayoutView::OnMouseLeave( nFlags ) ;
	//
	m_status = (TouchStatus) (m_flagSelected ? statusSelected : statusNormal) ;
	PostUpdate() ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteTouchableView::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( pChild != NULL )
	{
		return	false ;
	}
	if ( SGLSpriteLayoutView::OnLButtonDown( xPos, yPos, nFlags ) )
	{
		return	true ;
	}
	if ( m_flagSelectable )
	{
		m_status = statusPushed ;
		PostUpdate() ;
	}
	return	true ;
}

bool SGLSpriteTouchableView::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( pChild != NULL )
	{
		return	false ;
	}
	if ( SGLSpriteLayoutView::OnLButtonUp( xPos, yPos, nFlags ) )
	{
		m_status = statusFocus ;
		PostUpdate() ;
		return	true ;
	}
	if ( m_flagSelectable )
	{
		if ( m_status == statusPushed )
		{
			OnButtonPushed() ;
		}
		m_status = statusFocus ;
		PostUpdate() ;
	}
	return	true ;
}

// 押下時処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteTouchableView::OnButtonPushed( void )
{
	if ( !GetID().IsEmpty() )
	{
		SGLSprite *	pParent = GetParent() ;
		if ( pParent != NULL )
		{
			pParent->OnCommand( GetID(), 0, 0 ) ;
		}
	}
	if ( m_pAudioPushed != NULL )
	{
		m_pAudioPushed->Play() ;
	}
}

