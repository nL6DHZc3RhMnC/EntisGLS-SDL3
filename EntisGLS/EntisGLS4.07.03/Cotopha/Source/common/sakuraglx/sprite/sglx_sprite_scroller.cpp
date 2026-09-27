
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_scroller.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ドラッグスクロール用リスナ用リスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteMouseScrollerListener, SGLSpriteMouseListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseScrollerListener::SGLSpriteMouseScrollerListener( void )
{
}

// スクロール処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScrollerListener::OnScrolled( SGLSprite& sprite )
{
}


//////////////////////////////////////////////////////////////////////////////
// ドラッグスクロール用リスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
( SakuraGL::SGLSpriteMouseScroller, SGLSpriteMouseListener, SGLSpriteTimer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseScroller::SGLSpriteMouseScroller( void )
{
	m_nFlags = 0 ;
	m_xMin = 0 ;
	m_yMin = 0 ;
	m_xMax = 0 ;
	m_yMax = 0 ;
	//
	m_msLastMove = CurrentMilliSec() ;
	m_fpDamper = 0.1 ;
	m_vWheelSpeed.x = 0 ;
	m_vWheelSpeed.y = 256 ;
	//
	m_flagAboveMouse = false ;
	m_flagNoMove = false ;
	m_threasholdNoMove = 16 ;
	//
	m_flagTracking = false ;
}

// スクロール用リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::AttachScrollerTo( SGLSprite& sprite, bool flagAbove )
{
	m_flagAboveMouse = flagAbove ;
	sprite.AddReferenceTimer( this ) ;
	sprite.AttachMouseListener( this ) ;
	m_refScrollTarget = &sprite ;
}

// スクロール用リスナ解除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::DetachScrollerFrom( SGLSprite& sprite )
{
	sprite.RemoveTimer( this ) ;
	sprite.DetachMouseListener( this ) ;
}

// スクロール対象設定
//（AttachScrollerTo で設定したものと別のものを設定する場合）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::AttachScrollTarget( SGLSprite * pTarget )
{
	m_refScrollTarget = pTarget ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::AttachListener
		( SGLSpriteMouseScrollerListener * pListener )
{
	m_refListener.SetReference( pListener ) ;
}

// リスナ解除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::DetachListener
		( SGLSpriteMouseScrollerListener * pListener )
{
	if ( m_refListener.GetReference() == pListener )
	{
		m_refListener.ReleaseReference() ;
	}
}

// スクロール座標範囲設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::SetScrollRange
	( uint32_t nFlags,
		double xMin, double yMin, double xMax, double yMax )
{
	m_nFlags = nFlags ;
	m_xMin = xMin ;
	m_yMin = yMin ;
	m_xMax = xMax ;
	m_yMax = yMax ;
}

void SGLSpriteMouseScroller::SetScrollViewPort
	( const SGLSize& sizeTotal, const SGLImageRect& rectViewPort,
			SGLSprite * pHorzScrollBar, SGLSprite * pVertScrollBar )
{
	m_nFlags = 0 ;
	m_xMin = rectViewPort.x ;
	m_yMin = rectViewPort.y ;
	m_xMax = rectViewPort.x ;
	m_yMax = rectViewPort.y ;
	//
	m_refHorzScroll = pHorzScrollBar ;
	m_refVertScroll = pVertScrollBar ;
	//
	if ( sizeTotal.w > rectViewPort.w )
	{
		m_nFlags |= scrollHorizontal ;
		m_xMin = rectViewPort.x ;
		m_xMax = rectViewPort.x + rectViewPort.w - sizeTotal.w ;
		//
		if ( pHorzScrollBar != NULL )
		{
			int	nRange =
				pHorzScrollBar->GetScrollRange( SGLSprite::scrollHorz ) ;
			m_vRateScroll.x = m_xMax - m_xMin ;
			if ( nRange > 0 )
			{
				m_vRateScroll.x /= nRange ;
			}
		}
	}
	if ( sizeTotal.h > rectViewPort.h )
	{
		m_nFlags |= scrollVertical ;
		m_yMin = rectViewPort.y ;
		m_yMax = rectViewPort.y + rectViewPort.h - sizeTotal.h ;
		//
		if ( pVertScrollBar != NULL )
		{
			int	nRange =
				pHorzScrollBar->GetScrollRange( SGLSprite::scrollVert ) ;
			m_vRateScroll.y = m_yMax - m_yMin ;
			if ( nRange > 0 )
			{
				m_vRateScroll.y /= nRange ;
			}
		}
	}
}

// スクロール速度減衰設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::SetScrollDamper( double fpDamper )
{
	m_fpDamper = fpDamper ;
}

// ホイール速度設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::SetWheelSpeed( double xSpeed, double ySpeed )
{
	m_vWheelSpeed.x = xSpeed ;
	m_vWheelSpeed.y = ySpeed ;
}

// 前置リスナでのクリック判定閾値設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::SetThresholdNoMove( double thresholdNoMove )
{
	m_threasholdNoMove = thresholdNoMove ;
}

// 現在のスクロール速度リセット
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::ResetCurrentScrollSpeed( void )
{
	Lock() ;
	m_vSpeed.x = 0 ;
	m_vSpeed.y = 0 ;
	Unlock() ;
}

// 水平スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::AttachHorzScrollBar
	( SGLSprite * pScrollBar, double rateScroll )
{
	m_refHorzScroll = pScrollBar ;
	m_vRateScroll.x = rateScroll ;
	m_vSpeed.x = 0 ;
}

// 垂直スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::AttachVertScrollBar
	( SGLSprite * pScrollBar, double rateScroll )
{
	m_refVertScroll = pScrollBar ;
	m_vRateScroll.y = rateScroll ;
	m_vSpeed.y = 0 ;
}

// スクロール位置反映
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::ReflectScrollPosOf( SGLSprite& sprite )
{
	LockTrace( __FILE__, __LINE__ ) ;
	const S3DDVector&	vPos = sprite.GetPosition() ;
	SGLSprite *	pScrollBar ;
	//
	pScrollBar = m_refHorzScroll ;
	if ( pScrollBar != NULL )
	{
		pScrollBar->SetScrollPos
			( eslRoundR32ToInt
				( (float) ((vPos.x - m_xMin)
							* m_vRateScroll.x) ), SGLSprite::scrollHorz ) ;
	}
	//
	pScrollBar = m_refVertScroll ;
	if ( pScrollBar != NULL )
	{
		pScrollBar->SetScrollPos
			( eslRoundR32ToInt
				( (float) ((vPos.y - m_yMin)
							* m_vRateScroll.y) ), SGLSprite::scrollVert ) ;
	}
	Unlock() ;
}

// スクロール位置設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMouseScroller::SetScrollPositionTo
			( SGLSprite& sprite, double xPos, double yPos )
{
	if ( m_xMin <= m_xMax )
	{
		if ( xPos < m_xMin )
		{
			xPos = m_xMin ;
		}
		else if ( xPos > m_xMax )
		{
			xPos = m_xMax ;
		}
	}
	else
	{
		if ( xPos < m_xMax )
		{
			xPos = m_xMax ;
		}
		else if ( xPos > m_xMin )
		{
			xPos = m_xMin ;
		}
	}
	if ( m_yMin <= m_yMax )
	{
		if ( yPos < m_yMin )
		{
			yPos = m_yMin ;
		}
		else if ( yPos > m_yMax )
		{
			yPos = m_yMax ;
		}
	}
	else
	{
		if ( yPos < m_yMax )
		{
			yPos = m_yMax ;
		}
		else if ( yPos > m_yMin )
		{
			yPos = m_yMin ;
		}
	}
	sprite.SetPosition( xPos, yPos ) ;
	ReflectScrollPosOf( sprite ) ;
	//
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		pListener->OnScrolled( sprite ) ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseScroller::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	if ( !m_flagTracking )
	{
		int64_t	msCurTime = CurrentMilliSec() ;
		if ( msecPast > msCurTime - m_msLastMove )
		{
			msecPast = (uint32_t) (msCurTime - m_msLastMove) ;
		}
		m_msLastMove = msCurTime ;
		//
		double	secPast = (double) msecPast / 1000.0 ;
		if ( m_fpDamper > 0 )
		{
			m_vSpeed *= pow( m_fpDamper, secPast ) ;
			if ( fabs( m_vSpeed.x ) < 0.5 )
			{
				m_vSpeed.x = 0 ;
			}
			if ( fabs( m_vSpeed.y ) < 0.5 )
			{
				m_vSpeed.y = 0 ;
			}
		}
		else
		{
			m_vSpeed.x = 0 ;
			m_vSpeed.y = 0 ;
		}
		SGLSprite *	pTarget = m_refScrollTarget ;
		if ( pTarget == NULL )
		{
			pTarget = &sprite ;
		}
		const S3DDVector&	vPos = pTarget->GetPosition() ;
		double	xPos = vPos.x ;
		double	yPos = vPos.y ;
		bool	fMove = false ;
		if ( (m_nFlags & scrollHorizontal) && (m_vSpeed.x != 0) )
		{
			xPos += m_vSpeed.x * secPast ;
			fMove = true ;
		}
		if ( (m_nFlags & scrollVertical) && (m_vSpeed.y != 0) )
		{
			yPos += m_vSpeed.y * secPast ;
			fMove = true ;
		}
		if ( fMove )
		{
			SetScrollPositionTo( *pTarget, xPos, yPos ) ;
			//
			SGLWindowSprite *
				pWindow = SGLWindowSprite::WindowOf( pTarget ) ;
			if ( pWindow != NULL )
			{
				pWindow->CallMouseMove() ;
			}
		}
	}
	return	false ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseScroller::OnMouseMove
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagTracking )
	{
		S2DDVector	vMousePos( xPos, yPos ) ;
		sprite.LocalToGlobal( vMousePos ) ;
		double	xDelta = vMousePos.x - m_vLastMove.x ;
		double	yDelta = vMousePos.y - m_vLastMove.y ;
		int64_t	msCurTime = CurrentMilliSec() ;
		double	secPast = (double) (msCurTime - m_msLastMove) / 1000.0 ;
		m_msLastMove = msCurTime ;
		//
		SGLSprite *	pTarget = m_refScrollTarget ;
		if ( pTarget == NULL )
		{
			pTarget = &sprite ;
		}
		const S3DDVector&	vPos = pTarget->GetPosition() ;
		double	xPos = vPos.x ;
		double	yPos = vPos.y ;
		if ( m_nFlags & scrollHorizontal )
		{
			xPos += xDelta ;
		}
		if ( m_nFlags & scrollVertical )
		{
			yPos += yDelta ;
		}
		SetScrollPositionTo( *pTarget, xPos, yPos ) ;
		//
		m_vTotalMoved.x += xDelta ;
		m_vTotalMoved.y += yDelta ;
		m_vLastMove = vMousePos ;
		//
		if ( m_vTotalMoved.Absolute() > m_threasholdNoMove )
		{
			if( m_flagNoMove && m_flagAboveMouse )
			{
				sprite.OnMouseLeave
					( nFlags | SGLSpriteMouseListener::NoMouseListener ) ;
			}
			m_flagNoMove = false ;
		}
		if ( secPast > 0 )
		{
			m_vSpeed.x = xDelta / secPast ;
			m_vSpeed.y = yDelta / secPast ;
		}
	}
	else
	{
		SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnMouseMove( sprite, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

void SGLSpriteMouseScroller::OnMouseLeave( SGLSprite& sprite, int64_t nFlags )
{
	if ( m_flagTracking )
	{
		sprite.ReleaseMouseCapture() ;
		m_flagTracking = false ;
	}
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		pListener->OnMouseLeave( sprite, nFlags ) ;
	}
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseScroller::OnMouseWheel
	( SGLSprite& sprite, int32_t zDelta,
		double xPos, double yPos, int64_t nFlags )
{
	m_vSpeed.x = (double) zDelta / WheelDeltaUnit * m_vWheelSpeed.x ;
	m_vSpeed.y = (double) zDelta / WheelDeltaUnit * m_vWheelSpeed.y ;
	//
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		pListener->OnMouseWheel( sprite, zDelta, xPos, yPos, nFlags ) ;
	}
	return	true ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseScroller::OnButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagAboveMouse && (GetButtonID(nFlags) == LeftButtonID) )
	{
		if ( sprite.CanBeginDragOver( xPos, yPos ) )
		{
			return	false ;
		}
		m_flagTracking = true ;
		m_vLastMove.x = xPos ;
		m_vLastMove.y = yPos ;
		m_vTotalMoved.x = 0 ;
		m_vTotalMoved.y = 0 ;
		m_flagNoMove = true ;
		sprite.LocalToGlobal( m_vLastMove ) ;
		sprite.SetMouseCapture() ;
		m_msLastMove = CurrentMilliSec() ;
		//
		sprite.OnButtonDown
			( xPos, yPos,
				nFlags | SGLSpriteMouseListener::NoMouseListener ) ;
		return	true ;
	}
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		return	pListener->OnButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseScroller::OnButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( m_flagTracking )
	{
		sprite.ReleaseMouseCapture() ;
		m_flagTracking = false ;
		m_msLastMove = CurrentMilliSec() ;
		//
		if ( m_flagNoMove && (pListener != NULL) )
		{
			if ( m_flagAboveMouse )
			{
				return	pListener->OnButtonDown( sprite, xPos, yPos, nFlags )
						| pListener->OnButtonUp( sprite, xPos, yPos, nFlags ) ;
			}
			else
			{
				return	pListener->AfterButtonDown( sprite, xPos, yPos, nFlags )
						| pListener->AfterButtonUp( sprite, xPos, yPos, nFlags ) ;
			}
		}
	}
	if ( pListener != NULL )
	{
		return	pListener->OnButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseScroller::OnButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		return	pListener->OnButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseScroller::AfterButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	if ( !m_flagAboveMouse && (GetButtonID(nFlags) == LeftButtonID) )
	{
		m_flagTracking = true ;
		m_vLastMove.x = xPos ;
		m_vLastMove.y = yPos ;
		m_vTotalMoved.x = 0 ;
		m_vTotalMoved.y = 0 ;
		m_flagNoMove = true ;
		sprite.LocalToGlobal( m_vLastMove ) ;
		sprite.SetMouseCapture() ;
		m_msLastMove = CurrentMilliSec() ;
		return	true ;
	}
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		return	pListener->AfterButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseScroller::AfterButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		return	pListener->AfterButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseScroller::AfterButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseScrollerListener *	pListener = m_refListener ;
	if ( pListener != NULL )
	{
		return	pListener->AfterButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

