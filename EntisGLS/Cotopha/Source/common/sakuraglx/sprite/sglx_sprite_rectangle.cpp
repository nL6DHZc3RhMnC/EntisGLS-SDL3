
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 矩形スタイル
//////////////////////////////////////////////////////////////////////////////

// 構築関数（デフォルト値）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteRectangle::RectStyle::RectStyle( void )
{
}

// 構築関数（複製）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteRectangle::RectStyle::RectStyle
	( const SGLSpriteRectangle::RectStyle& style )
{
	size = style.size ;
	color = style.color ;
}

// 代入
const SGLSpriteRectangle::RectStyle&
	SGLSpriteRectangle::RectStyle::operator =
			( const SGLSpriteRectangle::RectStyle& style )
{
	size = style.size ;
	color = style.color ;
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// 矩形描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteRectangle::SGLRectDrawer, SGLSpriteDrawer )

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteRectangle::SGLRectDrawer::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( image != NULL )
	{
		SGLSpriteDrawer::Draw( render, pp, image ) ;
	}
	int	x = pp.ptPaint.x ;
	int	y = pp.ptPaint.y ;
	if ( pp.nFlags & paintFixedPosition )
	{
		x >>= 16 ;
		y >>= 16 ;
	}
	uint32_t	argbFill = m_argbFill.ui32 ;
	if ( pp.pAffine != NULL )
	{
		render.PushTransformation() ;
		render.AppendTransformation( *pp.pAffine, pp.nTransparency ) ;
	}
	else if ( pp.nTransparency > 0 )
	{
		if ( pp.nTransparency < 0x100 )
		{
			argbFill =
				sglPackedColorMul
					( argbFill, 0x100 - pp.nTransparency ) ;
		}
		else
		{
			argbFill = 0 ;
		}
	}
	if ( argbFill != 0 )
	{
		render.FillRectangle
			( x, y, m_sizeRect.w, m_sizeRect.h, argbFill, 0, pp.nFlags ) ;
	}
	if ( pp.pAffine != NULL )
	{
		render.PopTransformation() ;
	}
}

// 描画域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteRectangle::SGLRectDrawer::GetRectangle
	( SGLImageRect& rectDraw, SGLImageObject* image ) const
{
	rectDraw.x = 0 ;
	rectDraw.y = 0 ;
	rectDraw.w = m_sizeRect.w ;
	rectDraw.h = m_sizeRect.h ;
	return	true ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteRectangle::SGLRectDrawer::IsHitPointAt
	( SGLImageObject* image, double x, double y ) const
{
	if ( m_argbFill.argb.Alpha < 0x80 )
	{
		return	false ;
	}
	int32_t	xPos = (int32_t) x ;
	int32_t	yPos = (int32_t) y ;
	if ( (xPos < 0) | (yPos < 0) )
	{
		return	false ;
	}
	if ( (xPos >= m_sizeRect.w) | (yPos >= m_sizeRect.h) )
	{
		return	false ;
	}
	return	true ;
}


//////////////////////////////////////////////////////////////////////////////
// 矩形スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteRectangle, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteRectangle::SGLSpriteRectangle( void )
{
}

SGLSpriteRectangle::SGLSpriteRectangle( const SGLSpriteRectangle& src )
	: SGLSprite( src )
{
	SetRectangleStyle( src.m_style ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteRectangle::~SGLSpriteRectangle( void )
{
	DetachSyncTimeout( 100 ) ;
}

// 矩形スタイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteRectangle::SetRectangleStyle
	( const SGLSpriteRectangle::RectStyle& style )
{
	SGLRectDrawer *	pDrawer =
		ESLTypeCast<SGLRectDrawer>( (SGLSpriteDrawer*) m_pDrawer ) ;
	if ( pDrawer == NULL )
	{
		pDrawer = new SGLRectDrawer ;
		m_pDrawer = pDrawer ;
	}
	m_style = style ;
	pDrawer->m_argbFill = style.color ;
	pDrawer->m_sizeRect = style.size ;
}

// 矩形サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteRectangle::SetRectangleSize( ssize_t w, ssize_t h )
{
	SGLRectDrawer *	pDrawer =
		ESLTypeCast<SGLRectDrawer>( (SGLSpriteDrawer*) m_pDrawer ) ;
	if ( pDrawer == NULL )
	{
		pDrawer = new SGLRectDrawer ;
		m_pDrawer = pDrawer ;
	}
	m_style.size.w = (int32_t) w ;
	m_style.size.h = (int32_t) h ;
	pDrawer->m_sizeRect.w = (int32_t) w ;
	pDrawer->m_sizeRect.h = (int32_t) h ;
}

// 矩形塗りつぶし色設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteRectangle::SetRectangleColor( SGLPalette color )
{
	SGLRectDrawer *	pDrawer =
		ESLTypeCast<SGLRectDrawer>( (SGLSpriteDrawer*) m_pDrawer ) ;
	if ( pDrawer == NULL )
	{
		pDrawer = new SGLRectDrawer ;
		m_pDrawer = pDrawer ;
	}
	m_style.color = color ;
	pDrawer->m_argbFill = color ;
}

// 矩形スタイルを解釈する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteRectangle::ParseRectStyle
	( SGLSpriteRectangle::RectStyle& style,
		const SSystem::SXMLDocument& xmlStyle )
{
	style.size.w =
		(int32_t) xmlStyle.GetAttrRichIntegerAs
							( L"width", style.size.w ) ;
	style.size.h =
		(int32_t) xmlStyle.GetAttrRichIntegerAs
							( L"height", style.size.h ) ;
	style.color.ui32 =
		(int32_t) xmlStyle.GetAttrRichIntegerAs
							( L"color", style.color.ui32 ) ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteRectangle::GetLQClassName( void ) const
{
	return	L"EntisGLS4.RectangleSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteRectangle::DuplicateObject( void )
{
	return	new SGLSpriteRectangle( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteRectangle::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_style, sizeof(RectStyle) ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteRectangle::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	RectStyle	style ;
	file.Read( &style, sizeof(RectStyle) ) ;
	SetRectangleStyle( style ) ;
	return	sglErrSuccess ;
}

