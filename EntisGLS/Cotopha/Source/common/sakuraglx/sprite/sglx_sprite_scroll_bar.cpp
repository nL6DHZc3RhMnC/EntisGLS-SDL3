
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スプライトボタンリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteScrollListener, SObject )

// 位置が移動した
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollListener::OnScroll
		( SGLSpriteScrollBar& scroll, int64_t codeNotify )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// スクロールバースタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::BarStyle::BarStyle( void )
{
	typeScroll = typeVertBar ;
	flagBarStretchable = false ;
	flagProgressStretchable = false ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::BarStyle::BarStyle
	( const SGLSpriteScrollBar::BarStyle& style )
{
	typeScroll = style.typeScroll ;
	flagBarStretchable = style.flagBarStretchable ;
	flagProgressStretchable = style.flagProgressStretchable ;
	rectTrackMargin = style.rectTrackMargin ;
	rectBarStretchable = style.rectBarStretchable ;
	rectColStretchable = style.rectColStretchable ;
	rectProgressStretchable = style.rectProgressStretchable ;
	//
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		imgdscBar[i] = style.imgdscBar[i] ;
		imgdscColumn[i] = style.imgdscColumn[i] ;
		imgdscProgress[i] = style.imgdscProgress[i] ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteScrollBar::BarStyle&
	SGLSpriteScrollBar::BarStyle::operator =
		( const SGLSpriteScrollBar::BarStyle& style )
{
	typeScroll = style.typeScroll ;
	flagBarStretchable = style.flagBarStretchable ;
	flagProgressStretchable = style.flagProgressStretchable ;
	rectTrackMargin = style.rectTrackMargin ;
	rectBarStretchable = style.rectBarStretchable ;
	rectColStretchable = style.rectColStretchable ;
	rectProgressStretchable = style.rectProgressStretchable ;
	//
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		imgdscBar[i] = style.imgdscBar[i] ;
		imgdscColumn[i] = style.imgdscColumn[i] ;
		imgdscProgress[i] = style.imgdscProgress[i] ;
	}
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// スクロールバースプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteScrollBar, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::SGLSpriteScrollBar( void )
	: m_statusBar( statusNormal ), m_statusView( statusNormal ),
		m_widthScrollBar( 0 ), m_posScroll( 0 ),
		m_rangeScroll( 0 ), m_pageScroll( 0 ),
		m_flagKeyActive( false ), m_flagTracking( false )
{
	m_flagsUI |= uiFocusable ;
}

SGLSpriteScrollBar::SGLSpriteScrollBar( const SGLSpriteScrollBar& src )
	: SGLSprite( src ),
		m_statusBar( statusNormal ),
		m_statusView( statusNormal ),
		m_styleBar( src.m_styleBar ),
		m_widthScrollBar( src.m_widthScrollBar ),
		m_rectTrack( src.m_rectTrack ),
		m_posScroll( src.m_posScroll ),
		m_rangeScroll( src.m_rangeScroll ),
		m_pageScroll( src.m_pageScroll ),
		m_flagKeyActive( false ), m_flagTracking( false )
{
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		m_refBarImage[i] = src.m_refBarImage[i] ;
		m_refColumnImage[i] = src.m_refColumnImage[i] ;
		m_refProgressImage[i] = src.m_refProgressImage[i] ;
		m_styleBar.imgdscBar[i].pImage = m_refBarImage[i] ;
		m_styleBar.imgdscColumn[i].pImage = m_refColumnImage[i] ;
		m_styleBar.imgdscProgress[i].pImage = m_refProgressImage[i] ;
	}
	UpdateScrollImage() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::~SGLSpriteScrollBar( void )
{
	DetachSyncTimeout( 100 ) ;
}

// ボタンリスナを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::AttachScrollListener
			( SGLSpriteScrollListener * pListener )
{
	m_refScrollListener = pListener ;
}

void SGLSpriteScrollBar::SetSmartScrollListener
		( SGLSpriteScrollListener * pListener )
{
	m_refScrollListener.SetSmartReference( pListener ) ;
}

// シンプルなスクロールバーを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteScrollBar::CreateSimpleScrollBar
	( SGLSpriteScrollBar::BarType typeScroll,
		uint32_t widthScrollBar, SGLImageObject** ppBarKnob )
{
	BarStyle	style ;
	style.typeScroll = typeScroll ;
	style.rectTrackMargin = SGLRect( 0, 0, 0, 0 ) ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		style.imgdscBar[i].pImage = ppBarKnob[i] ;
	}
	m_widthScrollBar = widthScrollBar ;
	//
	SetScrollBarStyle( style ) ;
	//
	return	sglErrSuccess ;
}

// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	//
	// スクロールカラム画像描画
	//
	SGLSprite::DrawSprite( render, pp, image ) ;
	//
	render.PushTransformation() ;
	if ( pp.pAffine != NULL )
	{
		render.AppendTransformation( *(pp.pAffine), pp.nTransparency ) ;
	}
	//
	// スクロール進行カラム画像描画
	//
	SGLImageObject *	pProgress =
		m_pProgressImage
			[ValidStatusView
				( &m_styleBar.imgdscProgress[0], m_statusView )] ;
	if ( pProgress != NULL )
	{
		SGLPaintParam	ppProgress ;
		SGLAffine		affine ;
		SGLSize			sizeView = GetProgressViewSize() ;
		SGLSize			sizeProgress = pProgress->GetImageSize() ;
		SGLImageRect	rectProgress( 0, 0, sizeProgress.w, sizeProgress.h ) ;
		//
		ppProgress.ptPaint = pp.ptPaint ;
		ppProgress.ptPaint.x += m_styleBar.rectTrackMargin.left ;
		ppProgress.ptPaint.y += m_styleBar.rectTrackMargin.top ;
		//
		if ( m_rangeScroll > 0 )
		{
			if ( m_styleBar.flagProgressStretchable )
			{
				SGLPaintParam	pp ;
				pp.ptPaint = pp.ptPaint ;
				pp.ptPaint.x += m_styleBar.rectTrackMargin.left ;
				pp.ptPaint.y += m_styleBar.rectTrackMargin.top ;
				//
				SGLImageRect	rectStretchanle = m_styleBar.rectProgressStretchable ;
				rectProgress = rectStretchanle ;
				if ( m_styleBar.typeScroll == typeVertBar )
				{
					SGLImageRect	rectUpper( 0, 0, sizeProgress.w, rectStretchanle.y ) ;
					render.DrawImage( pp, pProgress, &rectUpper ) ;
					//
					int	yUnder = rectStretchanle.y + rectStretchanle.h ;
					int	hUnder = sizeProgress.h - yUnder ;
					SGLImageRect	rectUnder( 0, yUnder, sizeProgress.w, hUnder ) ;
					//
					int	hStretch =
							(m_rectTrack.GetHeight()
								- (rectStretchanle.y + hUnder))
										* m_posScroll / m_rangeScroll ;
					pp.ptPaint.y += rectUpper.h + hStretch ;
					render.DrawImage( pp, pProgress, &rectUnder ) ;
					//
					affine.a22 = (float32_t) hStretch
								/ (float32_t) (sizeProgress.h
												- (rectUpper.h + rectUnder.h)) ;
					ppProgress.ptPaint.y += rectUpper.h ;
					ppProgress.pAffine = &affine ;
				}
				else
				{
					SGLImageRect	rectLeft( 0, 0, rectStretchanle.x, sizeProgress.h ) ;
					render.DrawImage( pp, pProgress, &rectLeft ) ;
					//
					int	xRight = rectStretchanle.x + rectStretchanle.w ;
					int	wRight = sizeProgress.w - xRight ;
					SGLImageRect	rectRight( xRight, 0, wRight, sizeProgress.h ) ;
					//
					int	wStretch =
							(m_rectTrack.GetWidth()
								- (rectStretchanle.x + wRight))
										* m_posScroll / m_rangeScroll ;
					pp.ptPaint.x += rectLeft.w + wStretch ;
					render.DrawImage( pp, pProgress, &rectRight ) ;
					//
					affine.a11 = (float32_t) wStretch
								/ (float32_t) (sizeProgress.w
												- (rectLeft.w + rectRight.w)) ;
					ppProgress.ptPaint.x += rectLeft.w ;
					ppProgress.pAffine = &affine ;
				}
			}
			else
			{
				if ( m_styleBar.typeScroll == typeVertBar )
				{
					sizeProgress.h =
						sizeProgress.h * m_posScroll / m_rangeScroll ;
					rectProgress.h = sizeProgress.h ;
					//
					if ( sizeView.h != sizeProgress.h )
					{
						affine.a22 = (float32_t) sizeView.h
										/ (float32_t) sizeProgress.h ;
						ppProgress.pAffine = &affine ;
					}
				}
				else
				{
					sizeProgress.w =
						sizeProgress.w * m_posScroll / m_rangeScroll ;
					rectProgress.w = sizeProgress.w ;
					//
					if ( sizeView.w != sizeProgress.w )
					{
						affine.a11 = (float32_t) sizeView.w
										/ (float32_t) sizeProgress.w ;
						ppProgress.pAffine = &affine ;
					}
				}
			}
		}
		render.DrawImage( ppProgress, pProgress, &rectProgress ) ;
	}
	//
	// つまみ画像描画
	//
	BarStatus	statusBarView =
			ValidStatusView( &m_styleBar.imgdscBar[0], m_statusView ) ;
	SGLImageObject *	pBar = m_refBarImage[statusBarView] ;
	if ( pBar != NULL )
	{
		SGLSize			sizeViewBar = GetBarKnobViewSize() ;
		SGLSize			sizeOrgBar = pBar->GetImageSize() ;
		SGLImageRect *	pBarViewRect =
							m_styleBar.imgdscBar[statusBarView].pRect ;
		if ( pBarViewRect != NULL )
		{
			sizeOrgBar = pBarViewRect->GetSize() ;
		}
		float32_t	zx = (float32_t) sizeViewBar.w / (float32_t) sizeOrgBar.w ;
		float32_t	zy = (float32_t) sizeViewBar.h / (float32_t) sizeOrgBar.h ;
		SGLAffine	affBar( zx, 0.0f, 0.0f,  0.0f, zy, 0.0f ) ;
		//
		SGLPaintParam	ppBar ;
		ppBar.ptPaint = pp.ptPaint ;
		ppBar.ptPaint += GetBarKnobPosition() ;
		ppBar.pAffine = &affBar ;
		//
		render.DrawImage( ppBar, pBar, pBarViewRect ) ;
	}
	//
	render.PopTransformation() ;
}

// 進捗バー表示サイズ
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLSpriteScrollBar::GetProgressViewSize( void ) const
{
	SGLSize	sizeTrack = m_rectTrack.GetSize() ;
	if ( m_rangeScroll - m_pageScroll > 0 )
	{
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			sizeTrack.h = sizeTrack.h * m_posScroll
								/ (m_rangeScroll - m_pageScroll) ;
		}
		else
		{
			sizeTrack.w = sizeTrack.w * m_posScroll
								/ (m_rangeScroll - m_pageScroll) ;
		}
	}
	return	sizeTrack ;
}

// つまみ表示位置
//////////////////////////////////////////////////////////////////////////////
SGLPoint SGLSpriteScrollBar::GetBarKnobPosition( void ) const
{
	SGLSize		sizeTrack = m_rectTrack.GetSize() - GetBarKnobViewSize() ;
	SGLPoint	ptBarKnob = m_rectTrack.GetPosition() ;
	if ( m_rangeScroll - m_pageScroll > 0 )
	{
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			ptBarKnob.y += sizeTrack.h * m_posScroll
									/ (m_rangeScroll - m_pageScroll) ;
		}
		else
		{
			ptBarKnob.x += sizeTrack.w * m_posScroll
									/ (m_rangeScroll - m_pageScroll) ;
		}
	}
	return	ptBarKnob ;
}

// つまみ表示サイズ
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLSpriteScrollBar::GetBarKnobViewSize( void ) const
{
	if ( (m_pageScroll == 0) || !(m_styleBar.flagBarStretchable) )
	{
		return	m_sizeBarKnob ;
	}
	SGLSize	sizeBarKnob = m_sizeBarKnob ;
	if ( (m_rangeScroll == 0) || (m_pageScroll >= m_rangeScroll) )
	{
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			sizeBarKnob.h = m_rectTrack.GetHeight() ;
		}
		else
		{
			sizeBarKnob.w = m_rectTrack.GetWidth() ;
		}
		return	sizeBarKnob ;
	}
	if ( m_styleBar.typeScroll == typeVertBar )
	{
		int	h = m_rectTrack.GetHeight() * m_pageScroll / m_rangeScroll ;
		if ( h > sizeBarKnob.h )
		{
			sizeBarKnob.h = h ;
		}
	}
	else
	{
		int	w = m_rectTrack.GetWidth() * m_pageScroll / m_rangeScroll ;
		if ( w > sizeBarKnob.w )
		{
			sizeBarKnob.w = w ;
		}
	}
	return	sizeBarKnob ;
}

// つまみ位置からスクロール位置計算
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteScrollBar::GetScrollPosFromBarKnobPosition( double x, double y ) const
{
	if ( m_rangeScroll - m_pageScroll <= 0 )
	{
		return	0 ;
	}
	SGLSize	sizeBarKnob = GetBarKnobViewSize() ;
	SGLSize	sizeTrack = m_rectTrack.GetSize() - sizeBarKnob ;
	SGLImageObject *	pProgress =
		m_pProgressImage
			[ValidStatusView
				( &m_styleBar.imgdscProgress[0], m_statusView )] ;
	if ( (pProgress != NULL) && m_styleBar.flagProgressStretchable )
	{
		SGLImageRect	rectStretchanle = m_styleBar.rectProgressStretchable ;
		SGLSize			sizeProgImage = pProgress->GetImageSize() ;
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			int	hStretch = sizeTrack.h
							- (sizeProgImage.h - rectStretchanle.h) ;
			if ( y < rectStretchanle.y )
			{
				return	0 ;
			}
			else if ( y > rectStretchanle.y + hStretch )
			{
				return	m_rangeScroll - m_pageScroll ;
			}
			else if ( hStretch > 0 )
			{
				return	(int) ((y - rectStretchanle.y)
								* (m_rangeScroll - m_pageScroll)
										+ (hStretch >> 1)) / hStretch ;
			}
		}
		else
		{
			int	wStretch = sizeTrack.w
							- (sizeProgImage.w - rectStretchanle.w) ;
			if ( x < rectStretchanle.x )
			{
				return	0 ;
			}
			else if ( x > rectStretchanle.x + wStretch )
			{
				return	m_rangeScroll - m_pageScroll ;
			}
			else if ( wStretch > 0 )
			{
				return	(int) ((x - rectStretchanle.x)
								* (m_rangeScroll - m_pageScroll)
										+ (wStretch >> 1)) / wStretch ;
			}
		}
	}
	else
	{
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			if ( sizeTrack.h > 0 )
			{
				return	(int) ((y - m_rectTrack.top)
								* (m_rangeScroll - m_pageScroll)
										+ (sizeTrack.h >> 1)) / sizeTrack.h ;
			}
		}
		else
		{
			if ( sizeTrack.w > 0 )
			{
				return	(int) ((x - m_rectTrack.left)
								* (m_rangeScroll - m_pageScroll)
										+ (sizeTrack.w >> 1)) / sizeTrack.w ;
			}
		}
	}
	return	0 ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::GetRectangle( SGLRect& rectExt ) const
{
	bool	fRect = SGLSprite::GetRectangle( rectExt ) ;
	SGLRect	rectTrack = m_rectTrack ;
	if ( LocalToGlobalRect( rectTrack ) )
	{
		if ( fRect )
		{
			rectExt |= rectTrack ;
		}
		else
		{
			rectExt = rectTrack ;
		}
		fRect = true ;
	}
	return	fRect ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::IsHitSprite( double x, double y ) const
{
	return	(m_rectTrack.left <= x) & (x <= m_rectTrack.right)
				& (m_rectTrack.top <= y) & (y <= m_rectTrack.bottom) ;
}

// ドラッグ／スワイプ処理判定（子スプライトが処理するか？）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::CanBeginDragOver( double x, double y ) const
{
	return	IsHitSprite( x, y ) ;
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::SetScrollBarStyle
		( const SGLSpriteScrollBar::BarStyle& style )
{
	Lock() ;
	m_styleBar = style ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		m_refBarImage[i] = style.imgdscBar[i].pImage ;
		m_refColumnImage[i] = style.imgdscColumn[i].pImage ;
		m_refProgressImage[i] = style.imgdscProgress[i].pImage ;
	}
	UpdateScrollImage() ;
	Unlock() ;
}

// サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::SetScrollBarSize( uint32_t widthScrollBar )
{
	Lock() ;
	m_widthScrollBar = widthScrollBar ;
	UpdateScrollImage() ;
	Unlock() ;
}

// バーステータス設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::SetScrollBarStatus
		( SGLSpriteScrollBar::BarStatus status )
{
	BarStatus	statusEffect = EffectStatus( status ) ;
	if ( m_statusBar == statusEffect )
	{
		return ;
	}
	m_statusBar = statusEffect ;
	UpdateScrollView() ;
}

// キーフォーカス・禁止状態をステータスに加味する
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::BarStatus
	SGLSpriteScrollBar::EffectStatus( SGLSpriteScrollBar::BarStatus status )
{
	if ( IsEnabled() )
	{
		// 有効状態
		if ( status == statusDisabled )
		{
			status = statusNormal ;
		}
		if ( m_flagKeyActive && HasKeyFocus() )
		{
			// フォーカス有
			if ( status == statusNormal )
			{
				status = statusFocus ;
			}
		}
		if ( m_flagTracking )
		{
			status = statusTracking ;
		}
	}
	else
	{
		// 禁止状態
		status = statusDisabled ;
	}
	return	status ;
}

// 有効な表示用ステータスを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSpriteScrollBar::BarStatus
	SGLSpriteScrollBar::ValidStatusView
		( const SGLSkinManager::ImageDescription* pImages,
							SGLSpriteScrollBar::BarStatus status )
{
	while ( (status != statusNormal) & (pImages[status].pImage == NULL) )
	{
		if ( status == statusDisabled )
		{
			status = statusNormal ;
			break ;
		}
		if ( status == statusTracking )
		{
			status = statusFocus ;
		}
		else if ( status == statusFocus )
		{
			status = statusNormal ;
			break ;
		}
	}
	return	status ;
}

// スクロールバー画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::UpdateScrollImage( void )
{
	//
	// 画像参照正規化
	//
	AttachImage( NULL ) ;
	//
	SGLSize	sizeBar( 0, 0 ), sizeColumn( 0, 0 ), sizeProgress( 0, 0 ) ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		m_styleBar.imgdscBar[i].pImage = m_refBarImage[i] ;
		m_styleBar.imgdscColumn[i].pImage = m_refColumnImage[i] ;
		m_styleBar.imgdscProgress[i].pImage = m_refProgressImage[i] ;
		//
		if ( m_styleBar.imgdscBar[i].pImage != NULL )
		{
			if ( m_styleBar.imgdscBar[i].pRect != NULL )
			{
				sizeBar |= m_styleBar.imgdscBar[i].pRect->GetSize() ;
			}
			else
			{
				sizeBar |= m_styleBar.imgdscBar[i].pImage->GetImageSize() ;
			}
		}
		if ( m_styleBar.imgdscColumn[i].pImage != NULL )
		{
			if ( m_styleBar.imgdscColumn[i].pRect != NULL )
			{
				sizeColumn |= m_styleBar.imgdscColumn[i].pRect->GetSize() ;
			}
			else
			{
				sizeColumn |= m_styleBar.imgdscColumn[i].pImage->GetImageSize() ;
			}
		}
		if ( m_styleBar.imgdscProgress[i].pImage != NULL )
		{
			if ( m_styleBar.imgdscProgress[i].pRect != NULL )
			{
				sizeProgress |= m_styleBar.imgdscProgress[i].pRect->GetSize() ;
			}
			else
			{
				sizeProgress |= m_styleBar.imgdscProgress[i].pImage->GetImageSize() ;
			}
		}
		//
		m_pColumnImage[i] = NULL ;
		m_pProgressImage[i] = NULL ;
	}
	//
	// サイズの正規化
	//
	SGLRect	rectStretch = m_styleBar.rectColStretchable ;
	m_sizeBarKnob = sizeBar ;
	//
	m_rectTrack = m_styleBar.rectTrackMargin ;
	m_rectTrack.right = sizeColumn.w - m_rectTrack.right - 1 ;
	m_rectTrack.bottom = sizeColumn.h - m_rectTrack.bottom - 1 ;
	//
	if ( m_rectTrack.IsEmpty() )
	{
		m_rectTrack.left = 0 ;
		m_rectTrack.top = 0 ;
		m_rectTrack.SetWidth( sizeColumn.w ) ;
		m_rectTrack.SetHeight( sizeColumn.h ) ;
	}
	if ( rectStretch.IsEmpty() )
	{
		rectStretch.left = 0 ;
		rectStretch.top = 0 ;
		rectStretch.SetWidth( sizeColumn.w ) ;
		rectStretch.SetHeight( sizeColumn.h ) ;
	}
	if ( m_widthScrollBar != 0 )
	{
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			if ( m_rectTrack.IsEmpty() )
			{
				m_rectTrack.left = 0 ;
				m_rectTrack.top = 0 ;
				m_rectTrack.SetWidth( sizeBar.w ) ;
				m_rectTrack.SetHeight( m_widthScrollBar ) ;
			}
			else
			{
				m_rectTrack.SetHeight
					( m_widthScrollBar
						- m_styleBar.rectTrackMargin.top
						- m_styleBar.rectTrackMargin.bottom ) ;
			}
		}
		else
		{
			if ( m_rectTrack.IsEmpty() )
			{
				m_rectTrack.left = 0 ;
				m_rectTrack.top = 0 ;
				m_rectTrack.SetWidth( m_widthScrollBar ) ;
				m_rectTrack.SetHeight( sizeBar.h ) ;
			}
			else
			{
				m_rectTrack.SetWidth
					( m_widthScrollBar
						- m_styleBar.rectTrackMargin.left
						- m_styleBar.rectTrackMargin.right ) ;
			}
		}
		if ( rectStretch.IsEmpty() )
		{
			rectStretch = m_rectTrack ;
		}
	}
	if ( m_rectTrack.IsEmpty() )
	{
		return ;
	}
	//
	// 画像生成
	//
	SGLPaintContext	paint ;
	if ( m_styleBar.typeScroll == typeVertBar )
	{
		for ( size_t i = 0; i < statusCount; i ++ )
		{
			//
			// カラム画像生成
			//
			SGLImageObject *	pColumn = m_styleBar.imgdscColumn[i].pImage ;
			if ( pColumn != NULL )
			do
			{
				SGLImageInfo	imginf ;
				pColumn = pColumn->NewReference( m_styleBar.imgdscColumn[i].pRect ) ;
				if ( pColumn == NULL )
				{
					break ;
				}
				if ( pColumn->GetImageInfo( imginf ) )
				{
					delete	pColumn ;
					break ;
				}
				if ( (m_widthScrollBar != 0)
					&& (m_widthScrollBar != imginf.height) )
				{
					SGLImage *	pImage = new SGLImage ;
					m_pColumnImage[i] = pImage ;
					imginf.height = m_widthScrollBar ;
					if ( !pImage->CreateBuffer( imginf ) )
					{
						paint.AttachTargetImage( pImage, NULL, NULL ) ;
						if ( rectStretch.top > 0 )
						{
							SGLPaintParam	ppUpper ;
							SGLImageRect	rectUpper
								( 0, 0, imginf.width, rectStretch.top ) ;
							paint.DrawImage( ppUpper, pColumn, &rectUpper ) ;
						}
						int	heightUnder =
								imginf.height - (rectStretch.bottom + 1) ;
						if ( heightUnder > 0 )
						{
							SGLPaintParam	ppUnder ;
							SGLImageRect	rectUnder
								( 0, rectStretch.bottom + 1,
									imginf.width, heightUnder ) ;
							ppUnder.ptPaint.y = m_widthScrollBar - heightUnder ;
							paint.DrawImage( ppUnder, pColumn, &rectUnder ) ;
						}
						if ( !rectStretch.IsEmpty() )
						{
							SGLImageRect	rectBody
								( 0, rectStretch.top,
									imginf.width, rectStretch.GetHeight() ) ;
							float32_t	heightBody =
								(float32_t) (m_widthScrollBar
												- rectStretch.top - heightUnder) ;
							float32_t	heightStretch =
									heightBody / (float32_t) rectBody.h ;
							SGLAffine	affine
								( 1.0f, 0.0f, 0.0f,
									0.0f, heightStretch, (float32_t) rectBody.y ) ;
							SGLPaintParam	ppBody ;
							ppBody.pAffine = &affine ;
							paint.DrawImage( ppBody, pColumn, &rectBody ) ;
						}
						paint.DetachTargetImage() ;
					}
					delete	pColumn ;
				}
				else
				{
					m_pColumnImage[i] = pColumn ;
				}
			}
			while ( false ) ;
			//
			// カラム画像生成
			//
			SGLImageObject *	pProgress = m_styleBar.imgdscProgress[i].pImage ;
			if ( pProgress != NULL )
			{
				m_pProgressImage[i] =
					pProgress->NewReference( m_styleBar.imgdscProgress[i].pRect ) ;
			}
		}
	}
	else
	{
		for ( size_t i = 0; i < statusCount; i ++ )
		{
			//
			// カラム画像生成
			//
			SGLImageObject *	pColumn = m_styleBar.imgdscColumn[i].pImage ;
			if ( pColumn != NULL )
			do
			{
				SGLImageInfo	imginf ;
				pColumn = pColumn->NewReference( m_styleBar.imgdscColumn[i].pRect ) ;
				if ( pColumn == NULL )
				{
					break ;
				}
				if ( pColumn->GetImageInfo( imginf ) )
				{
					delete	pColumn ;
					break ;
				}
				if ( (m_widthScrollBar != 0)
					&& (m_widthScrollBar != imginf.width) )
				{
					SGLImage *	pImage = new SGLImage ;
					m_pColumnImage[i] = pImage ;
					imginf.width = m_widthScrollBar ;
					if ( !pImage->CreateBuffer( imginf ) )
					{
						paint.AttachTargetImage( pImage, NULL, NULL ) ;
						if ( rectStretch.left > 0 )
						{
							SGLPaintParam	ppLeft ;
							SGLImageRect	rectLeft
								( 0, 0, rectStretch.left, imginf.height ) ;
							paint.DrawImage( ppLeft, pColumn, &rectLeft ) ;
						}
						int	widthRight =
								imginf.width - (rectStretch.right + 1) ;
						if ( widthRight > 0 )
						{
							SGLPaintParam	ppRight ;
							SGLImageRect	rectRight
								( 0, rectStretch.right + 1,
									widthRight, imginf.height ) ;
							ppRight.ptPaint.x = m_widthScrollBar - widthRight ;
							paint.DrawImage( ppRight, pColumn, &rectRight ) ;
						}
						if ( !rectStretch.IsEmpty() )
						{
							SGLImageRect	rectBody
								( rectStretch.left, 0,
									rectStretch.GetWidth(), imginf.height ) ;
							float32_t	widthBody =
								(float32_t) (m_widthScrollBar
												- rectStretch.left - widthRight) ;
							float32_t	widthStretch =
									widthBody / (float32_t) rectBody.w ;
							SGLAffine	affine
								( widthStretch, 0.0f, (float32_t) rectBody.x,
									0.0f, 1.0f, 0.0f ) ;
							SGLPaintParam	ppBody ;
							ppBody.pAffine = &affine ;
							paint.DrawImage( ppBody, pColumn, &rectBody ) ;
						}
						paint.DetachTargetImage() ;
					}
					delete	pColumn ;
				}
				else
				{
					m_pColumnImage[i] = pColumn ;
				}
			}
			while ( false ) ;
			//
			// カラム画像生成
			//
			SGLImageObject *	pProgress = m_styleBar.imgdscProgress[i].pImage ;
			if ( pProgress != NULL )
			{
				m_pProgressImage[i] =
					pProgress->NewReference( m_styleBar.imgdscProgress[i].pRect ) ;
			}
		}
	}
	UpdateScrollView() ;
}

// ボタンの表示を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::UpdateScrollView( void )
{
	Lock() ;
	m_statusView = m_statusBar ;
	AttachAnimation
		( m_pColumnImage
			[ValidStatusView
				( &m_styleBar.imgdscColumn[0], m_statusView )] ) ;
	Unlock() ;
}

// スクロール操作を通知する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::NotifyScroll
			( SGLSpriteScrollBar::NotificationCode ncode )
{
	SGLSpriteScrollListener *	pListener = m_refScrollListener ;
	if ( pListener != NULL )
	{
		if ( pListener->OnScroll( *this, ncode ) )
		{
			return ;
		}
	}
	if ( !m_strID.IsEmpty() )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pParent->OnCommand
				( m_strID, m_posScroll, ncode,
						commandNormal, (ncode == ncPosition) ) ;
		}
	}
}

// スクロールバースタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::ParseScrollBarStyle
	( SGLSkinManager& skin,
		SGLSpriteScrollBar::BarStyle& style,
		const SSystem::SXMLDocument& xmlStyle )
{
	SXMLDocument *	pxmlArrange = xmlStyle.GetElementTagAs( L"arrange" ) ;
	if ( pxmlArrange != NULL )
	{
		SString *	pstrType = pxmlArrange->GetAttributeAs( L"type" ) ;
		if ( pstrType != NULL )
		{
			if ( *pstrType == L"vert" )
			{
				style.typeScroll = typeVertBar ;
			}
			else if ( *pstrType == L"horz" )
			{
				style.typeScroll = typeHorzBar ;
			}
		}
	}
	SXMLDocument *	pxmlTrack = xmlStyle.GetElementTagAs( L"track" ) ;
	if ( pxmlTrack != NULL )
	{
		style.rectTrackMargin.left =
			(int32_t) pxmlTrack->GetAttrRichIntegerAs
							( L"left", style.rectTrackMargin.left ) ;
		style.rectTrackMargin.top =
			(int32_t) pxmlTrack->GetAttrRichIntegerAs
							( L"top", style.rectTrackMargin.top ) ;
		style.rectTrackMargin.right =
			(int32_t) pxmlTrack->GetAttrRichIntegerAs
							( L"right", style.rectTrackMargin.right ) ;
		style.rectTrackMargin.bottom =
			(int32_t) pxmlTrack->GetAttrRichIntegerAs
							( L"bottom", style.rectTrackMargin.bottom ) ;
	}
	SXMLDocument *	pxmlStretchable = xmlStyle.GetElementTagAs( L"stretchable" ) ;
	if ( pxmlStretchable != NULL )
	{
		style.rectColStretchable.x =
			(int32_t) pxmlStretchable->GetAttrRichIntegerAs
							( L"x", style.rectColStretchable.x ) ;
		style.rectColStretchable.y =
			(int32_t) pxmlStretchable->GetAttrRichIntegerAs
							( L"y", style.rectColStretchable.y ) ;
		style.rectColStretchable.w =
			(int32_t) pxmlStretchable->GetAttrRichIntegerAs
							( L"width", style.rectColStretchable.w ) ;
		style.rectColStretchable.h =
			(int32_t) pxmlStretchable->GetAttrRichIntegerAs
							( L"height", style.rectColStretchable.h ) ;
	}
	SXMLDocument *	pxmlBarStretchable = xmlStyle.GetElementTagAs( L"bar_stretchable" ) ;
	if ( pxmlBarStretchable != NULL )
	{
		style.flagBarStretchable = true ;
		style.rectBarStretchable.x =
			(int32_t) pxmlBarStretchable->GetAttrRichIntegerAs
							( L"x", style.rectColStretchable.x ) ;
		style.rectBarStretchable.y =
			(int32_t) pxmlBarStretchable->GetAttrRichIntegerAs
							( L"y", style.rectColStretchable.y ) ;
		style.rectBarStretchable.w =
			(int32_t) pxmlBarStretchable->GetAttrRichIntegerAs
							( L"width", style.rectColStretchable.w ) ;
		style.rectBarStretchable.h =
			(int32_t) pxmlBarStretchable->GetAttrRichIntegerAs
							( L"height", style.rectColStretchable.h ) ;
	}
	SXMLDocument *	pxmlPrgStretchable = xmlStyle.GetElementTagAs( L"progress_stretchable" ) ;
	if ( pxmlPrgStretchable != NULL )
	{
		style.flagProgressStretchable = true ;
		style.rectProgressStretchable.x =
			(int32_t) pxmlPrgStretchable->GetAttrRichIntegerAs
							( L"x", style.rectColStretchable.x ) ;
		style.rectProgressStretchable.y =
			(int32_t) pxmlPrgStretchable->GetAttrRichIntegerAs
							( L"y", style.rectColStretchable.y ) ;
		style.rectProgressStretchable.w =
			(int32_t) pxmlPrgStretchable->GetAttrRichIntegerAs
							( L"width", style.rectColStretchable.w ) ;
		style.rectProgressStretchable.h =
			(int32_t) pxmlPrgStretchable->GetAttrRichIntegerAs
							( L"height", style.rectColStretchable.h ) ;
	}
	const wchar_t *	pwszStatusType[statusCount] ;
	pwszStatusType[statusNormal]	= L"normal" ;
	pwszStatusType[statusFocus]		= L"focus" ;
	pwszStatusType[statusTracking]	= L"tracking" ;
	pwszStatusType[statusDisabled]	= L"disabled" ;
	for ( size_t i = 0; i < statusCount; i ++ )
	{
		SXMLDocument *	pxmlTag =
				xmlStyle.GetElementTagAs( pwszStatusType[i] ) ;
		if ( pxmlTag != NULL )
		{
			SString *	pstrBar = pxmlTag->GetAttributeAs( L"bar" ) ;
			if ( pstrBar != NULL )
			{
				skin.GetRichImageAs( style.imgdscBar[i], *pstrBar ) ;
			}
			SString *	pstrColumn = pxmlTag->GetAttributeAs( L"column" ) ;
			if ( pstrColumn != NULL )
			{
				skin.GetRichImageAs( style.imgdscColumn[i], *pstrColumn ) ;
			}
			SString *	pstrProgress = pxmlTag->GetAttributeAs( L"progress" ) ;
			if ( pstrProgress != NULL )
			{
				skin.GetRichImageAs( style.imgdscProgress[i], *pstrProgress ) ;
			}
		}
	}
}

// 入力禁止状態
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::SetEnable( bool fEnable )
{
	Lock() ;
	SGLSprite::SetEnable( fEnable ) ;
	UpdateScrollView() ;
	Unlock() ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteScrollBar::GetScrollPos
	( SGLSprite::ScrollDirection scrlDir )
{
	return	m_posScroll ;
}

void SGLSpriteScrollBar::SetScrollPos
	( int nPos, SGLSprite::ScrollDirection scrlDir )
{
	Lock() ;
	if ( nPos < 0 )
	{
		nPos = 0 ;
	}
	else if ( nPos >= m_rangeScroll )
	{
		nPos = m_rangeScroll ;
	}
	if ( nPos != m_posScroll )
	{
		m_posScroll = nPos ;
		NotifyUpdate() ;
	}
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteScrollBar::GetScrollRange
	( SGLSprite::ScrollDirection scrlDir )
{
	return	m_rangeScroll ;
}

void SGLSpriteScrollBar::SetScrollRange
	( int nRange, SGLSprite::ScrollDirection scrlDir )
{
	Lock() ;
	m_rangeScroll = nRange ;
	NotifyUpdate() ;
	Unlock() ;
}

// スクロール・ページサイズ属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteScrollBar::GetScrollPageSize
	( SGLSprite::ScrollDirection scrlDir )
{
	return	m_pageScroll ;
}

void SGLSpriteScrollBar::SetScrollPageSize
	( int nPageSize, SGLSprite::ScrollDirection scrlDir )
{
	Lock() ;
	m_pageScroll = nPageSize ;
	NotifyUpdate() ;
	Unlock() ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteScrollBar::InvokeCommand
	( const SSystem::SXMLDocument& xmlCmd,
		SSystem::SXMLDocument * pxmlResult )
{
	if ( xmlCmd.GetTag() == L"bar" )
	{
		Lock() ;
		m_posScroll = (int) xmlCmd.GetAttrRichIntegerAs
									( L"pos", m_posScroll ) ;
		m_rangeScroll = (int) xmlCmd.GetAttrRichIntegerAs
									( L"range", m_rangeScroll ) ;
		NotifyUpdate() ;
		Unlock() ;
		return	sglErrSuccess ;
	}
	return	SGLSprite::InvokeCommand( xmlCmd, pxmlResult ) ;
}

// マウスキャプチャーが解放された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::OnReleaseMouseCapture( void )
{
	m_flagTracking = false ;
	UpdateScrollView() ;
}

// フォーカスが設定された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::OnSetKeyFocus( void )
{
	SGLSprite::OnSetKeyFocus() ;

	m_flagKeyActive = true ;
	UpdateScrollView() ;
}

// キーフォーカスが解除された
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteScrollBar::OnKillKeyFocus( void )
{
	SGLSprite::OnSetKeyFocus() ;

	m_flagKeyActive = false ;
	UpdateScrollView() ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagTracking && (GetMouseID(nFlags) == 0) )
	{
		SetScrollPos
			( GetScrollPosFromBarKnobPosition
				( xPos - m_vTrackOffset.x, yPos - m_vTrackOffset.y ) ) ;
		NotifyScroll( ncPosition ) ;
		return	true ;
	}
	else if ( IsEnabled() && (GetMouseID(nFlags) == 0) )
	{
		SetScrollBarStatus( statusFocus ) ;
	}
	return	SGLSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLSpriteScrollBar::OnMouseLeave( int64_t nFlags )
{
	if ( GetMouseID(nFlags) == 0 )
	{
		m_flagTracking = false ;
		m_flagKeyActive = false ;
		SetScrollBarStatus( statusNormal ) ;
	}
	SGLSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	bool	fResult = SGLSprite::OnMouseWheel( zDelta, xPos, yPos, nFlags ) ;
	if ( IsEnabled() )
	{
		int	nStep = 1 ;
		if ( m_styleBar.typeScroll == typeVertBar )
		{
			if ( m_rectTrack.GetHeight() > 0 )
			{
				nStep = m_rangeScroll / m_rectTrack.GetHeight() ;
			}
		}
		else
		{
			if ( m_rectTrack.GetWidth() > 0 )
			{
				nStep = m_rangeScroll / m_rectTrack.GetWidth() ;
			}
		}
		if ( nStep <= 0 )
		{
			nStep = 1 ;
		}
		SetScrollPos( m_posScroll + zDelta * nStep / WheelDeltaUnit ) ;
		//
		NotificationCode	ncode = ncLineDown ;
		if ( zDelta < 0 )
		{
			ncode = ncLineUp ;
		}
		NotifyScroll( ncode ) ;
		return	true ;
	}
	return	fResult ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	bool	fResult = SGLSprite::OnLButtonDown( xPos, yPos, nFlags ) ;
	if ( IsEnabled() && (GetMouseID(nFlags) == 0) )
	{
		SGLPoint	ptKnob = GetBarKnobPosition() ;
		SGLSize		sizeBarKnob = GetBarKnobViewSize() ;
		if ( (ptKnob.x <= xPos) & (xPos < ptKnob.x + sizeBarKnob.w)
			& (ptKnob.y <= yPos) & (yPos < ptKnob.y + sizeBarKnob.h) )
		{
			m_flagTracking = true ;
			m_vTrackOffset.x = xPos - ptKnob.x ;
			m_vTrackOffset.y = yPos - ptKnob.y ;
			SetMouseCapture() ;
			SetScrollBarStatus( statusTracking ) ;
			NotifyScroll( ncTracking ) ;
			return	true ;
		}
		SetScrollPos
			( GetScrollPosFromBarKnobPosition
				( xPos - sizeBarKnob.w / 2,
					yPos - sizeBarKnob.h / 2 ) ) ;
		if ( sizeBarKnob.IsEmpty() )
		{
			m_flagTracking = true ;
			m_vTrackOffset.x = 0 ;
			m_vTrackOffset.y = 0 ;
			SetMouseCapture() ;
			SetScrollBarStatus( statusTracking ) ;
			NotifyScroll( ncTracking ) ;
			return	true ;
		}
		NotifyScroll( ncClickColumn ) ;
		return	true ;
	}
	return	fResult ;
}

bool SGLSpriteScrollBar::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagTracking && (GetMouseID(nFlags) == 0) )
	{
		m_flagTracking = false ;
		ReleaseMouseCapture() ;
		SetScrollBarStatus( statusFocus ) ;
		NotifyScroll( ncEndTracking ) ;
	}
	SGLSprite::OnLButtonUp( xPos, yPos, nFlags ) ;
	return	true ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteScrollBar::OnKeyDown
	( int64_t nVirtKey, int64_t nFlags )
{
	if ( m_styleBar.typeScroll == typeVertBar )
	{
		int	nStep = 1 ;
		if ( m_rectTrack.GetHeight() > 0 )
		{
			nStep = m_rangeScroll / m_rectTrack.GetHeight() ;
			if ( nStep <= 0 )
			{
				nStep = 1 ;
			}
		}
		if ( nVirtKey == vkeyUp )
		{
			m_flagKeyActive = true ;
			UpdateScrollView() ;
			SetScrollPos( m_posScroll - nStep ) ;
			NotifyScroll( ncLineUp ) ;
			return	true ;
		}
		else if ( nVirtKey == vkeyDown )
		{
			m_flagKeyActive = true ;
			UpdateScrollView() ;
			SetScrollPos( m_posScroll + nStep ) ;
			NotifyScroll( ncLineDown ) ;
			return	true ;
		}
	}
	else
	{
		int	nStep = 1 ;
		if ( m_rectTrack.GetWidth() > 0 )
		{
			nStep = m_rangeScroll / m_rectTrack.GetWidth() ;
			if ( nStep <= 0 )
			{
				nStep = 1 ;
			}
		}
		if ( nVirtKey == vkeyLeft )
		{
			m_flagKeyActive = true ;
			UpdateScrollView() ;
			SetScrollPos( m_posScroll - nStep ) ;
			NotifyScroll( ncLineUp ) ;
			return	true ;
		}
		else if ( nVirtKey == vkeyRight )
		{
			m_flagKeyActive = true ;
			UpdateScrollView() ;
			SetScrollPos( m_posScroll + nStep ) ;
			NotifyScroll( ncLineDown ) ;
			return	true ;
		}
	}
	return	SGLSprite::OnKeyDown( nVirtKey, nFlags ) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteScrollBar::DuplicateObject( void )
{
	return	new SGLSpriteScrollBar( *this ) ;
}

