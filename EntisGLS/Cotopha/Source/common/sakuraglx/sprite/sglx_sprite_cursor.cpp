
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_cursor.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// マウスカーソル表示用スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteCursor, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteCursor::SGLSpriteCursor( void )
{
	ModifyUIFlag( uiUnclickable, 0 ) ;
	//
	m_nCursorFlags =
		cursorDefaultArrow | cursorPositionPolling
				| cursorImagePolling | cursorAutoHidding ;
	m_pWindow = NULL ;
	m_msecStaying = 0 ;
	m_flagAutoHidding = false ;
	m_msecHiddenTime = 3000 ;
#if	defined(__PLATFORM_WINDOWS__)
	m_hCursor = NULL ;
	m_hArrowCursor = ::LoadCursor( NULL, IDC_ARROW ) ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteCursor::~SGLSpriteCursor( void )
{
	DetachSyncTimeout( 100 ) ;
}

// 動作フラグ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::SetCursorControlFlags( uint32_t nFlags )
{
	m_nCursorFlags = nFlags ;
}

uint32_t SGLSpriteCursor::ModifyCursorControlFlags
			( uint32_t nAddFlags, uint32_t nRemoveFlags )
{
	m_nCursorFlags = (m_nCursorFlags | nAddFlags) & ~nRemoveFlags ;
	return	m_nCursorFlags ;
}

// ターゲットウィンドウ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::AttachTargetWindow( SGLAbstractWindow * pWindow )
{
	m_pWindow = pWindow ;
}

// 自動非表示時間
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::SetAutoHiddenTime( uint32_t msecTime )
{
	m_msecHiddenTime = msecTime ;
}

// ウィンドウローカル座標をスプライト空間に変換する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::WindowPointToCursorSpace
	( S2DDVector& vCursor, const SGLPoint& ptCursor ) const
{
	vCursor.x = ptCursor.x ;
	vCursor.y = ptCursor.y ;
	if ( m_pWindow != NULL )
	{
		WindowPointToCursorSpace( GetParent(), vCursor ) ;
	}
}

void SGLSpriteCursor::WindowPointToCursorSpace
	( SGLSprite * pParentSpace, S2DDVector& vCursor ) const
{
	if ( (pParentSpace == NULL)
		|| (pParentSpace == ESLTypeCast<SGLSprite>( m_pWindow )) )
	{
		return ;
	}
	SGLSprite *	pParent = pParentSpace->GetParent() ;
	if ( pParent != NULL )
	{
		WindowPointToCursorSpace( pParent, vCursor ) ;
	}
	pParentSpace->GlobalToLocal( vCursor ) ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::AdvanceTime( uint32_t msecPast )
{
	m_msecStaying += msecPast ;
	//
	SGLPoint	ptCursor( 0, 0 ) ;
	bool		flagCursorPos = false ;
	if ( m_pWindow != NULL )
	{
		if ( !m_pWindow->GetCursorPosition( ptCursor ) )
		{
			flagCursorPos = true ;
		}
	}
	#if	defined(__PLATFORM_WINDOWS__)
	else
	{
		POINT	ptCursorPos ;
		::GetCursorPos( &ptCursorPos ) ;
		ptCursor.x = ptCursorPos.x ;
		ptCursor.y = ptCursorPos.y ;
		flagCursorPos = true ;
	}
	#endif
	if ( flagCursorPos )
	{
		if ( m_ptCursorPos != ptCursor )
		{
			if ( m_flagAutoHidding )
			{
				m_flagAutoHidding = false ;
				SetVisible( true ) ;
			}
			if ( m_nCursorFlags & cursorPositionPolling )
			{
				S2DDVector	vCursor ;
				WindowPointToCursorSpace( vCursor, ptCursor ) ;
				SetPosition( vCursor.x, vCursor.y ) ;
			}
			m_msecStaying = 0 ;
			m_ptCursorPos = ptCursor ;
		}
		if ( !m_flagAutoHidding
			&& (m_nCursorFlags & cursorAutoHidding)
			&& (m_msecStaying >= m_msecHiddenTime) )
		{
			m_flagAutoHidding = true ;
			SetVisible( false ) ;
		}
	}
	#if	defined(__PLATFORM_WINDOWS__)
	if ( m_nCursorFlags & cursorImagePolling )
	{
		CURSORINFO	ci ;
		eslFillMemory( &ci, 0, sizeof(CURSORINFO) ) ;
		ci.cbSize = sizeof(CURSORINFO) ;
		if ( ::GetCursorInfo( &ci ) )
		{
			if ( (ci.hCursor == NULL)
				&& (m_nCursorFlags & cursorDefaultArrow) )
			{
				ci.hCursor = m_hArrowCursor ;
			}
			if ( m_hCursor != ci.hCursor )
			{
				m_hCursor = ci.hCursor ;
				//
				if ( m_nCursorFlags & cursorImagePolling )
				{
					AttachImage( NULL ) ;
					//
					SGLImageBuffer *	pimgColor =
						ConvertHCURSORtoImageBuffer( m_hCursor ) ;
					if ( pimgColor != NULL )
					{
						m_imgCursor.CreateCloneBuffer( *pimgColor ) ;
						AttachImage( &m_imgCursor ) ;
						sglReleaseImageBuffer( pimgColor ) ;
					}
				}
			}
		}
	}
	#endif

	SGLSprite::AdvanceTime( msecPast ) ;
}


#if	defined(__PLATFORM_WINDOWS__)

// HCURSOR を画像データに変換
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer *
	SGLSpriteCursor::ConvertHCURSORtoImageBuffer( HCURSOR hCursor )
{
	ICONINFO	ii ;
	if ( !::GetIconInfo( (HICON) hCursor, &ii ) )
	{
		return	NULL ;
	}
	SGLImageBuffer *
		pimgColor = ConvertHBITMAPtoImageBuffer( ii.hbmColor ) ;
	SGLImageBuffer *
		pimgMask = ConvertHBITMAPtoImageBuffer( ii.hbmMask ) ;
	if ( ii.hbmColor != NULL )
	{
		::DeleteObject( ii.hbmColor ) ;
	}
	if ( ii.hbmMask != NULL )
	{
		::DeleteObject( ii.hbmMask ) ;
	}
	if ( (pimgColor != NULL) && (pimgMask != NULL) )
	{
		MakeBlendCursorImage( pimgColor, pimgMask ) ;
	}
	else if ( (pimgColor == NULL) && (pimgMask != NULL) )
	{
		SGLImageInfo	imginf ;
		imginf.format = formatImageRGB ;
		imginf.depth = 32 ;
		imginf.width = pimgMask->width ;
		imginf.height = pimgMask->height / 2 ;
		imginf.pitchPixel = 4 ;
		imginf.pitchLine = imginf.width * 4 ;
		//
		pimgColor = sglCreateImageBuffer( imginf ) ;
		//
		SGLPalette	pxFill( 0xFF000000 ) ;
		sglFillImageBuffer( *pimgColor, pxFill ) ;
		//
		MakeBlendCursorImage( pimgColor, pimgMask ) ;
	}
	if ( pimgMask != NULL )
	{
		sglReleaseImageBuffer( pimgMask ) ;
	}
	if ( pimgColor != NULL )
	{
		pimgColor->ptOrigin.x = ii.xHotspot ;
		pimgColor->ptOrigin.y = ii.yHotspot ;
	}
	return	pimgColor ;
}

// HBITMAP を画像データに変換
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer *
	SGLSpriteCursor::ConvertHBITMAPtoImageBuffer( HBITMAP hBitmap )
{
	if ( hBitmap == NULL )
	{
		return	NULL ;
	}
	SGLImageBuffer *	pimgBitmap = NULL ;
	//
	HDC	hdc = ::CreateCompatibleDC( NULL ) ;
	//
	BITMAPINFO	bmi ;
	eslFillMemory( &bmi, 0, sizeof(BITMAPINFO) ) ;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
	//
	if ( ::GetDIBits( hdc, hBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS ) > 0 )
	{
		SGLImageInfo	imginf ;
		imginf.format = formatImageRGB ;
		imginf.depth = 32 ;
		imginf.width = (uint32_t) bmi.bmiHeader.biWidth ;
		imginf.height = (uint32_t) bmi.bmiHeader.biHeight ;
		imginf.pitchPixel = 4 ;
		imginf.pitchLine = imginf.width * 4 ;
		//
		pimgBitmap = sglCreateImageBuffer( imginf ) ;
		//
		if ( pimgBitmap != NULL )
		{
			if ( bmi.bmiHeader.biHeight > 0 )
			{
				bmi.bmiHeader.biHeight = - bmi.bmiHeader.biHeight ;
			}
			bmi.bmiHeader.biPlanes = 1 ;
			bmi.bmiHeader.biBitCount = 32 ;
			bmi.bmiHeader.biCompression = BI_RGB ;
			bmi.bmiHeader.biSizeImage = imginf.width * imginf.height * 4 ;
			//
			::GetDIBits
				( hdc, hBitmap, 0, imginf.height,
					pimgBitmap->ptrBuffer, &bmi, DIB_RGB_COLORS ) ;
		}
	}
	//
	::DeleteDC( hdc ) ;
	//
	return	pimgBitmap ;
}

// カーソルのカラー画像にマスク画像を合成
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCursor::MakeBlendCursorImage
	( SGLImageBuffer * pimgColor, SGLImageBuffer * pimgMask )
{
	if ( (pimgColor == NULL) || (pimgMask == NULL) )
	{
		return ;
	}
	if ( (pimgColor->depth != 32) || (pimgMask->depth != 32) )
	{
		return ;
	}
	//
	// マスク画像合成
	//
	uint8_t *	pbytDstLine = pimgColor->ptrBuffer ;
	uint8_t *	pbytMaskLine = pimgMask->ptrBuffer ;
	size_t		width = pimgColor->width ;
	size_t		height = pimgColor->height ;
	if ( width > pimgMask->width )
	{
		width = pimgMask->width ;
	}
	if ( height > pimgMask->height )
	{
		height = pimgMask->height ;
	}
	for ( size_t y = 0; y < height; y ++ )
	{
		uint8_t *	pbytDst = pbytDstLine ;
		uint8_t *	pbytSrc = pbytMaskLine ;
		for ( size_t x = 0; x < width; x ++ )
		{
			uint32_t	a = (uint32_t) (pbytSrc[0] ^ 0xFF) + 1 ;
			pbytDst[0] = (uint8_t) (((uint32_t) pbytDst[0] * a) >> 8) ;
			pbytDst[1] = (uint8_t) (((uint32_t) pbytDst[1] * a) >> 8) ;
			pbytDst[2] = (uint8_t) (((uint32_t) pbytDst[2] * a) >> 8) ;
			pbytDst[3] = (uint8_t) (a - 1) ;
			pbytDst += 4 ;
			pbytSrc += 4 ;
		}
		pbytDstLine += pimgColor->pitchLine ;
		pbytMaskLine += pimgMask->pitchLine ;
	}
	//
	// 反転画像合成
	//
	if ( pimgMask->height <= pimgColor->height )
	{
		return ;
	}
	height = pimgMask->height - pimgColor->height ;
	if ( height > pimgColor->height )
	{
		height = pimgColor->height ;
	}
	uint8_t *	pbytSrcLine = pbytMaskLine ;
	pbytMaskLine = pimgMask->ptrBuffer ;
	pbytDstLine = pimgColor->ptrBuffer ;
	for ( size_t y = 0; y < height; y ++ )
	{
		uint32_t *	pdwDst = (uint32_t*) pbytDstLine ;
		uint32_t *	pdwSrc = (uint32_t*) pbytSrcLine ;
		uint32_t *	pdwMask = (uint32_t*) pbytMaskLine ;
		for ( size_t x = 0; x < width; x ++ )
		{
			if ( (*pdwSrc & 0x00FFFFFF) && (pdwMask[0] & 0x00808080) )
			{
				*pdwDst |= 0xFF000000 ;
			}
			else
			{
				*pdwDst ^= *pdwSrc & 0x00FFFFFF ;
			}
			pdwDst ++ ;
			pdwSrc ++ ;
			pdwMask ++ ;
		}
		pbytDstLine += pimgColor->pitchLine ;
		pbytSrcLine += pimgMask->pitchLine ;
		pbytMaskLine += pimgMask->pitchLine ;
	}
}

#endif
