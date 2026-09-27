
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_progress_bar.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// バースタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteProgressBar::BarStyle::BarStyle( void )
{
	typeBar = barHorz ;
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteProgressBar::BarStyle::BarStyle( const BarStyle& style )
{
	typeBar = style.typeBar ;
	ptBarOffset = style.ptBarOffset ;
	for ( size_t i = 0; i < typeCount; i ++ )
	{
		imgdscParts[i] = style.imgdscParts[i] ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteProgressBar::BarStyle&
	SGLSpriteProgressBar::BarStyle::operator =
			( const SGLSpriteProgressBar::BarStyle& style )
{
	typeBar = style.typeBar ;
	ptBarOffset = style.ptBarOffset ;
	for ( size_t i = 0; i < typeCount; i ++ )
	{
		imgdscParts[i] = style.imgdscParts[i] ;
	}
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 進捗状況バー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteProgressBar, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteProgressBar::SGLSpriteProgressBar( void )
{
	m_rangeBar = 0x100 ;
	m_posBar = 0 ;
}

SGLSpriteProgressBar::SGLSpriteProgressBar( const SGLSpriteProgressBar& src )
	: SGLSprite( src ),
		m_styleBar( src.m_styleBar ),
		m_sizeBar( src.m_sizeBar ),
		m_rangeBar( src.m_rangeBar ), m_posBar( src.m_posBar )
{
	for ( size_t i = 0; i < typeCount; i ++ )
	{
		m_refFrame[i] = src.m_refFrame[i] ;
		m_styleBar.imgdscParts[i].pImage = m_refFrame[i] ;
	}
	UpdateBarImage() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteProgressBar::~SGLSpriteProgressBar( void )
{
	DetachSyncTimeout( 100 ) ;
}

// スプライト画像の描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteProgressBar::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	SGLSprite::DrawSprite( render, pp, image ) ;
	//
	SGLImageObject *	pBarImage = m_pBarImage ;
	if ( (m_posBar > 0) && (pBarImage != NULL) )
	{
		render.PushTransformation() ;
		if ( pp.pAffine != NULL )
		{
			render.AppendTransformation
				( *(pp.pAffine) + pp.ptPaint, pp.nTransparency ) ;
		}
		SGLPaintParam	ppBar ;
		ppBar.ptPaint = m_styleBar.ptBarOffset ;
		//
		if ( (m_rangeBar <= 0) || (m_posBar >= m_rangeBar) )
		{
			render.DrawImage( ppBar, pBarImage ) ;
		}
		else
		{
			SGLImageRect	rectBar ;
			rectBar.x = 0 ;
			rectBar.y = 0 ;
			if ( m_styleBar.typeBar == barHorz )
			{
				rectBar.w = m_styleBar.imgdscParts[barLeft].rectImage.w ;
				rectBar.h = pBarImage->GetImageHeight() ;
				rectBar.w +=
					(pBarImage->GetImageWidth()
						- m_styleBar.imgdscParts[barLeft].rectImage.w
						- m_styleBar.imgdscParts[barRight].rectImage.w)
					* m_posBar / m_rangeBar ;
			}
			else
			{
				rectBar.w = pBarImage->GetImageWidth() ;
				rectBar.h = m_styleBar.imgdscParts[barRight].rectImage.h ;
				rectBar.h +=
					(pBarImage->GetImageHeight()
						- m_styleBar.imgdscParts[barLeft].rectImage.h
						- m_styleBar.imgdscParts[barRight].rectImage.h)
					* m_posBar / m_rangeBar ;
				rectBar.y = pBarImage->GetImageHeight() - rectBar.h ;
				ppBar.ptPaint.y += rectBar.y ;
			}
			render.DrawImage( ppBar, pBarImage, &rectBar ) ;
		}
		render.PopTransformation() ;
	}
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteProgressBar::GetRectangle( SGLRect& rectExt ) const
{
	bool				fRect = SGLSprite::GetRectangle( rectExt ) ;
	SGLImageObject *	pBar = m_pBarImage ;
	if ( pBar != NULL )
	{
		SGLImageInfo	imginf ;
		if ( !pBar->GetImageInfo( imginf ) )
		{
			SGLRect	rectBar ;
			rectBar.left = m_styleBar.ptBarOffset.x ;
			rectBar.top = m_styleBar.ptBarOffset.y ;
			rectBar.SetWidth( imginf.width ) ;
			rectBar.SetHeight( imginf.height ) ;
			//
			if ( LocalToGlobalRect( rectBar ) )
			{
				if ( fRect )
				{
					rectExt |= rectBar ;
				}
				else
				{
					rectExt = rectBar ;
				}
				fRect = true ;
			}
		}
	}
	return	fRect ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteProgressBar::GetScrollPos
	( SGLSprite::ScrollDirection scrlDir )
{
	return	m_posBar ;
}

void SGLSpriteProgressBar::SetScrollPos
	( int nPos, SGLSprite::ScrollDirection scrlDir )
{
	Lock() ;
	if ( nPos < 0 )
	{
		nPos = 0 ;
	}
	else if ( nPos >= m_rangeBar )
	{
		nPos = m_rangeBar ;
	}
	if ( nPos != m_posBar )
	{
		m_posBar = nPos ;
		NotifyUpdate() ;
	}
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSpriteProgressBar::GetScrollRange
	( SGLSprite::ScrollDirection scrlDir )
{
	return	m_rangeBar ;
}

void SGLSpriteProgressBar::SetScrollRange
	( int nRange, SGLSprite::ScrollDirection scrlDir )
{
	Lock() ;
	m_rangeBar = nRange ;
	NotifyUpdate() ;
	Unlock() ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteProgressBar::InvokeCommand
	( const SSystem::SXMLDocument& xmlCmd,
		SSystem::SXMLDocument * pxmlResult )
{
	if ( xmlCmd.GetTag() == L"bar" )
	{
		Lock() ;
		m_posBar = (int) xmlCmd.GetAttrRichIntegerAs
									( L"pos", m_posBar ) ;
		m_rangeBar = (int) xmlCmd.GetAttrRichIntegerAs
									( L"range", m_rangeBar ) ;
		NotifyUpdate() ;
		Unlock() ;
		return	sglErrSuccess ;
	}
	return	SGLSprite::InvokeCommand( xmlCmd, pxmlResult ) ;
}

// フレームスタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteProgressBar::SetBarStyle( const SGLSpriteProgressBar::BarStyle& style )
{
	Lock() ;
	m_styleBar = style ;
	for ( size_t i = 0; i < typeCount; i ++ )
	{
		m_refFrame[i] = m_styleBar.imgdscParts[i].pImage ;
	}
	UpdateBarImage() ;
	Unlock() ;
}

// フレーム矩形設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteProgressBar::SetBarSize( const SGLSize& size )
{
	Lock() ;
	m_sizeBar = size ;
	UpdateBarImage() ;
	Unlock() ;
}

// 文字画像を更新する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteProgressBar::UpdateBarImage( void )
{
	AttachImage( NULL ) ;
	//
	for ( size_t i = 0; i < typeCount; i ++ )
	{
		m_styleBar.imgdscParts[i].pImage = m_refFrame[i] ;
	}
	m_pFrameImage = CreateFrameImage( m_styleBar, m_sizeBar ) ;
	if ( m_pFrameImage != NULL )
	{
		AttachImage( m_pFrameImage ) ;
	}
	m_pBarImage = CreateBarImage( m_styleBar, m_sizeBar ) ;
}

// フレーム画像を生成する
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteProgressBar::CreateFrameImage
	( const SGLSpriteProgressBar::BarStyle& style, const SGLSize& size )
{
	const int	widthLeftFrame = style.imgdscParts[frameLeft].rectImage.w ;
	const int	widthRightFrame = style.imgdscParts[frameRight].rectImage.w ;
	const int	heightUpperFrame = style.imgdscParts[frameLeft].rectImage.h ;
	const int	heightUnderFrame = style.imgdscParts[frameRight].rectImage.h ;
	if ( style.typeBar == barHorz )
	{
		//
		// 最小サイズ判定
		//
		if ( size.w < widthLeftFrame + widthRightFrame )
		{
			return	NULL ;
		}
		//
		// ループ回数計算
		//
		const int	widthWay = style.imgdscParts[frameWay].rectImage.w ;
		const int	heightWay = style.imgdscParts[frameWay].rectImage.h ;
		size_t	countHorzWay = 0 ;
		if ( widthWay != 0 )
		{
			countHorzWay =
				(size.w - widthLeftFrame - widthRightFrame) / widthWay ;
		}
		//
		// 画像バッファ生成
		//
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->CreateImage
			( (uint32_t) (widthLeftFrame + widthRightFrame
								+ countHorzWay * widthWay),
				(uint32_t) heightWay, formatImageARGB, 32 ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		//
		// フレーム描画
		//
		SGLPaintContext	paint ;
		SGLPaintParam	ppParam ;
		size_t			i ;
		paint.AttachTargetImage( pImage, NULL ) ;
		//
		ppParam.ptPaint.x = 0 ;
		ppParam.ptPaint.y = 0 ;
		if ( style.imgdscParts[frameLeft].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[frameLeft].pImage ) ;
		}
		ppParam.ptPaint.x += widthLeftFrame ;
		//
		for ( i = 0; i < countHorzWay; i ++ )
		{
			if ( style.imgdscParts[frameWay].pImage != NULL )
			{
				paint.DrawImage( ppParam, style.imgdscParts[frameWay].pImage ) ;
			}
			ppParam.ptPaint.x += widthWay ;
		}
		if ( style.imgdscParts[frameRight].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[frameRight].pImage ) ;
		}
		paint.Flush() ;
		//
		return	pImage ;
	}
	else
	{
		//
		// 最小サイズ判定
		//
		if ( size.h < heightUpperFrame + heightUnderFrame )
		{
			return	NULL ;
		}
		//
		// ループ回数計算
		//
		const int	widthWay = style.imgdscParts[frameWay].rectImage.w ;
		const int	heightWay = style.imgdscParts[frameWay].rectImage.h ;
		size_t	countVertWay = 0 ;
		if ( heightWay != 0 )
		{
			countVertWay =
				(size.h - heightUpperFrame - heightUnderFrame) / heightWay ;
		}
		//
		// 画像バッファ生成
		//
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->CreateImage
			( widthWay,
				(uint32_t) (heightUpperFrame + heightUnderFrame
										+ countVertWay * heightWay),
				formatImageARGB, 32 ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		//
		// フレーム描画
		//
		SGLPaintContext	paint ;
		SGLPaintParam	ppParam ;
		size_t			i ;
		paint.AttachTargetImage( pImage, NULL ) ;
		//
		ppParam.ptPaint.x = 0 ;
		ppParam.ptPaint.y = 0 ;
		if ( style.imgdscParts[frameLeft].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[frameLeft].pImage ) ;
		}
		ppParam.ptPaint.y += heightUpperFrame ;
		//
		for ( i = 0; i < countVertWay; i ++ )
		{
			if ( style.imgdscParts[frameWay].pImage != NULL )
			{
				paint.DrawImage( ppParam, style.imgdscParts[frameWay].pImage ) ;
			}
			ppParam.ptPaint.y += heightWay ;
		}
		if ( style.imgdscParts[frameRight].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[frameRight].pImage ) ;
		}
		paint.Flush() ;
		//
		return	pImage ;
	}
}

// バー画像を生成する
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteProgressBar::CreateBarImage
	( const SGLSpriteProgressBar::BarStyle& style, const SGLSize& size )
{
	const int	widthLeftFrame = style.imgdscParts[frameLeft].rectImage.w ;
	const int	widthRightFrame = style.imgdscParts[frameRight].rectImage.w ;
	const int	heightUpperFrame = style.imgdscParts[frameLeft].rectImage.h ;
	const int	heightUnderFrame = style.imgdscParts[frameRight].rectImage.h ;
	const int	widthLeftBar = style.imgdscParts[barLeft].rectImage.w ;
	const int	widthRightBar = style.imgdscParts[barRight].rectImage.w ;
	const int	heightUpperBar = style.imgdscParts[barLeft].rectImage.h ;
	const int	heightUnderBar = style.imgdscParts[barRight].rectImage.h ;
	if ( style.typeBar == barHorz )
	{
		//
		// 最小サイズ判定
		//
		if ( size.w < widthLeftFrame + widthRightFrame )
		{
			return	NULL ;
		}
		//
		// ループ回数計算
		//
		const int	widthWay = style.imgdscParts[barWay].rectImage.w ;
		const int	heightWay = style.imgdscParts[barWay].rectImage.h ;
		size_t	countHorzWay = 0 ;
		if ( widthWay != 0 )
		{
			countHorzWay =
				(size.w - widthLeftFrame - widthRightFrame) / widthWay ;
		}
		//
		// 画像バッファ生成
		//
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->CreateImage
			( (uint32_t) (widthLeftBar + widthRightBar
								+ countHorzWay * widthWay),
				heightWay, formatImageARGB, 32 ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		//
		// フレーム描画
		//
		SGLPaintContext	paint ;
		SGLPaintParam	ppParam ;
		size_t			i ;
		paint.AttachTargetImage( pImage, NULL ) ;
		//
		ppParam.ptPaint.x = 0 ;
		ppParam.ptPaint.y = 0 ;
		if ( style.imgdscParts[barLeft].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[barLeft].pImage ) ;
		}
		ppParam.ptPaint.x += widthLeftBar ;
		//
		for ( i = 0; i < countHorzWay; i ++ )
		{
			if ( style.imgdscParts[barWay].pImage != NULL )
			{
				paint.DrawImage( ppParam, style.imgdscParts[barWay].pImage ) ;
			}
			ppParam.ptPaint.x += widthWay ;
		}
		if ( style.imgdscParts[barRight].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[barRight].pImage ) ;
		}
		paint.Flush() ;
		//
		return	pImage ;
	}
	else
	{
		//
		// 最小サイズ判定
		//
		if ( size.h < heightUpperFrame + heightUnderFrame )
		{
			return	NULL ;
		}
		//
		// ループ回数計算
		//
		const int	widthWay = style.imgdscParts[barWay].rectImage.w ;
		const int	heightWay = style.imgdscParts[barWay].rectImage.h ;
		size_t	countVertWay = 0 ;
		if ( heightWay != 0 )
		{
			countVertWay =
				(size.h - heightUpperFrame - heightUnderFrame) / heightWay ;
		}
		//
		// 画像バッファ生成
		//
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->CreateImage
			( widthWay,
				(uint32_t) (heightUpperBar + heightUnderBar
										+ countVertWay * heightWay),
				formatImageARGB, 32 ) )
		{
			delete	pImage ;
			return	NULL ;
		}
		//
		// フレーム描画
		//
		SGLPaintContext	paint ;
		SGLPaintParam	ppParam ;
		size_t			i ;
		paint.AttachTargetImage( pImage, NULL ) ;
		//
		ppParam.ptPaint.x = 0 ;
		ppParam.ptPaint.y = 0 ;
		if ( style.imgdscParts[barLeft].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[barLeft].pImage ) ;
		}
		ppParam.ptPaint.y += heightUpperBar ;
		//
		for ( i = 0; i < countVertWay; i ++ )
		{
			if ( style.imgdscParts[barWay].pImage != NULL )
			{
				paint.DrawImage( ppParam, style.imgdscParts[barWay].pImage ) ;
			}
			ppParam.ptPaint.y += heightWay ;
		}
		if ( style.imgdscParts[barRight].pImage != NULL )
		{
			paint.DrawImage( ppParam, style.imgdscParts[barRight].pImage ) ;
		}
		paint.Flush() ;
		//
		return	pImage ;
	}
}

// バースタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteProgressBar::ParseBarStyle
	( SGLSkinManager& skin,
		SGLSpriteProgressBar::BarStyle& style,
		const SSystem::SXMLDocument& xmlStyle )
{
	SXMLDocument *	pxmlArrange = xmlStyle.GetElementTagAs( L"arrange" ) ;
	if ( pxmlArrange != NULL )
	{
		SString *	pstrType = pxmlArrange->GetAttributeAs( L"type" ) ;
		if ( pstrType != NULL )
		{
			if ( *pstrType == L"horz" )
			{
				style.typeBar = barHorz ;
			}
			else if ( *pstrType == L"vert" )
			{
				style.typeBar = barVert ;
			}
		}
		style.ptBarOffset.x =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
							( L"bar_x", style.ptBarOffset.x ) ;
		style.ptBarOffset.y =
			(int32_t) pxmlArrange->GetAttrRichIntegerAs
							( L"bar_y", style.ptBarOffset.y ) ;
	}
	SXMLDocument *	pxmlFrame = xmlStyle.GetElementTagAs( L"frame" ) ;
	if ( pxmlFrame != NULL )
	{
		skin.GetRichImageAs
			( style.imgdscParts[frameLeft],
					pxmlFrame->GetAttrStringAs( L"left" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameRight],
					pxmlFrame->GetAttrStringAs( L"right" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[frameWay],
					pxmlFrame->GetAttrStringAs( L"way" ) ) ;
	}
	SXMLDocument *	pxmlBar = xmlStyle.GetElementTagAs( L"bar" ) ;
	if ( pxmlBar != NULL )
	{
		skin.GetRichImageAs
			( style.imgdscParts[barLeft],
					pxmlBar->GetAttrStringAs( L"left" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[barRight],
					pxmlBar->GetAttrStringAs( L"right" ) ) ;
		skin.GetRichImageAs
			( style.imgdscParts[barWay],
					pxmlBar->GetAttrStringAs( L"way" ) ) ;
	}
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteProgressBar::DuplicateObject( void )
{
	return	new SGLSpriteProgressBar( *this ) ;
}
