
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (c) 2002-2013 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>



//////////////////////////////////////////////////////////////////////////////
// スプライト抽象クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESprite, ESLObject )

// 表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool ESprite::IsVisible( void )
{
	return	m_visible ;
}

// 表示状態を取得（副作用なし）
//////////////////////////////////////////////////////////////////////////////
bool ESprite::IsVisibleMT( void ) const
{
	return	m_visible ;
}

// 表示状態設定
//////////////////////////////////////////////////////////////////////////////
void ESprite::SetVisible( bool fVisible )
{
	if ( (m_visible && !fVisible) || (!m_visible && fVisible) )
	{
		EGL_RECT	rect = GetRectangle( ) ;
		if ( m_parent != NULL )
		{
			m_parent->UpdateRect( &rect ) ;
		}
		m_visible = fVisible ;
		//
		rect = GetRectangle( ) ;
		if ( m_parent != NULL )
		{
			m_parent->UpdateRect( &rect ) ;
		}
	}
}

// 外接(最小)矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ESprite::GetRectangle3DView( const VIEW3D_INFO & v3dInfo )
{
	return	GetRectangle() ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool ESprite::GetHiddenRectangle( EGL_RECT & rect )
{
	return	false ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ESprite::Draw( HEGL_RENDER_POLYGON hRenderPoly )
{
	BeforeMTDraw() ;
	MTDraw( hRenderPoly ) ;
}

// スプライト描画（立体視表示用）
//////////////////////////////////////////////////////////////////////////////
void ESprite::DrawTo3DView
	( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo )
{
	BeforeMTDraw() ;
	MTDrawTo3DView( hRenderPoly, v3dInfo ) ;
}

// マルチスレッド描画前準備処理
//////////////////////////////////////////////////////////////////////////////
void ESprite::BeforeMTDraw( void )
{
}

// スプライト描画（立体視表示用・マルチスレッド）
//////////////////////////////////////////////////////////////////////////////
void ESprite::MTDrawTo3DView
	( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo )
{
	MTDraw( hRenderPoly ) ;
}

// スプライト上の指定領域の更新通知
//////////////////////////////////////////////////////////////////////////////
bool ESprite::UpdateRect( EGL_RECT * pUpdateRect )
{
	if ( m_parent && IsVisible() )
	{
		return	m_parent->UpdateChild( this, pUpdateRect ) ;
/*
		EGL_RECT	rect ;
		if ( pUpdateRect == NULL )
		{
			rect = GetRectangle( ) ;
		}
		else
		{
			rect = *pUpdateRect ;
		}
		return	m_parent->UpdateRect( &rect ) ;
*/	}
	return	false ;
}

// 子スプライト領域を更新領域に設定
//////////////////////////////////////////////////////////////////////////////
bool ESprite::UpdateChild( ESprite * pChild, EGL_RECT * pUpdateRect )
{
	EGL_RECT	rect = pChild->GetRectangle() ;
	if ( pUpdateRect == NULL )
	{
		rect = pChild->GetRectangle() ;
	}
	else
	{
		rect = *pUpdateRect ;
		pChild->LocalRectToGlocal3DView( rect, NULL ) ;
	}
	return	UpdateRect( &rect ) ;
}

// 立体視用ローカル→大域座標変換
//////////////////////////////////////////////////////////////////////////////
void ESprite::LocalRectToGlocal3DView
	( EGL_RECT & rect, const VIEW3D_INFO * pv3dInfo )
{
}

// 3D投影スクリーン座標取得
//////////////////////////////////////////////////////////////////////////////
const E3D_VECTOR * ESprite::GetScreen3DPosition( void ) const
{
	if ( m_parent != NULL )
	{
		return	m_parent->GetScreen3DPosition() ;
	}
	return	NULL ;
}

// 表示優先度変更
//////////////////////////////////////////////////////////////////////////////
void ESprite::ChangePriority( int nPriority )
{
	if ( m_parent != NULL )
	{
		m_parent->ChangedChildPriority( this, nPriority ) ;
	}
	else
	{
		m_priority = nPriority ;
	}
}

// 子スプライトのプライオリティ変更
//////////////////////////////////////////////////////////////////////////////
void ESprite::ChangedChildPriority( ESprite * pChild, int nPriority )
{
	pChild->m_priority = nPriority ;
}

// アニメーションを進める
//////////////////////////////////////////////////////////////////////////////
ESLError ESprite::OnAdvanceAnimation( unsigned int nTime )
{
	return	eslErrSuccess ;
}

// ｚ座標取得
//////////////////////////////////////////////////////////////////////////////
REAL32 ESprite::GetZPosition( void ) const
{
	return	0.0 ;
}

// カメラを取得する
//////////////////////////////////////////////////////////////////////////////
bool ESprite::Get3DViewCamera
	( E3D_VECTOR & vViewPos, E3D_VECTOR & vTargetPos, double & rRevAngleZ )
{
	if ( m_parent != NULL )
	{
		return	m_parent->Get3DViewCamera
					( vViewPos, vTargetPos, rRevAngleZ ) ;
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( EImageSprite, ESprite, EGLImage )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EImageSprite::EImageSprite( void )
{
	m_ispParam.dwFlags = 0 ;
	m_ispParam.ptDstPos.x = 0 ;
	m_ispParam.ptDstPos.y = 0 ;
	m_ispParam.ptRevCenter.x = 0 ;
	m_ispParam.ptRevCenter.y = 0 ;
	m_ispParam.rHorzUnit = 1 ;
	m_ispParam.rVertUnit = 1 ;
	m_ispParam.rRevAngle = 0 ;
	m_ispParam.rCrossingAngle = 90 ;
	m_ispParam.rgbDimColor.dwPixelCode = 0x00000000 ;
	m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
	m_ispParam.nTransparency = 0 ;
	m_ispParam.rZOrder = 0 ;
	//
	SetParameterToDraw( ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EImageSprite::~EImageSprite( void )
{
}

// スプライトパラメータから描画用パラメータセットアップ
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetParameterToDraw( void )
{
	EGL_SIZE	sizeImage ;
	if ( m_pImage == NULL )
	{
		sizeImage.w = 0 ;
		sizeImage.h = 0 ;
	}
	else
	{
		sizeImage.w = m_pImage->dwImageWidth ;
		sizeImage.h = m_pImage->dwImageHeight ;
	}
	//
	EGL_POINT	ptRevCenter = m_ispParam.ptRevCenter ;
	m_eglParam.dwFlags = m_ispParam.dwFlags & maskEGLFlags ;
	m_eglParam.ptBasePos = m_ispParam.ptDstPos ;
	//
	REAL32	xZoom = m_ispParam.rHorzUnit ;
	REAL32	yZoom = m_ispParam.rVertUnit ;
	REAL32	rZOrder = m_ispParam.rZOrder ;
	REAL32	rRevAngle = m_ispParam.rRevAngle ;
	//
	if ( m_ispParam.dwFlags & flagZScale )
	{
		m_f3d = true ;
		//
		E3D_VECTOR	vPos ;
		vPos.x = (REAL32) m_eglParam.ptBasePos.x ;
		vPos.y = (REAL32) m_eglParam.ptBasePos.y ;
		vPos.z = rZOrder ;
		//
		if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
		{
			vPos.x *= (REAL32) (1.0 / 0x10000) ;
			vPos.y *= (REAL32) (1.0 / 0x10000) ;
		}
		//
		const E3D_VECTOR *	pScreenPos = ESprite::GetScreen3DPosition() ;
		if ( pScreenPos != NULL )
		{
			vPos.x -= pScreenPos->x ;
			vPos.y -= pScreenPos->y ;
			//
			E3D_VECTOR	vViewPos, vTargetPos ;
			double		rCameraAngleZ ;
			if ( Get3DViewCamera( vViewPos, vTargetPos, rCameraAngleZ ) )
			{
				E3D_REV_MATRIX	matCamera ;
				matCamera.InitializeMatrix( E3DVector( 1, 1, 1 ) ) ;
				matCamera.RevolveOnZ( sin( rCameraAngleZ ), cos( rCameraAngleZ ) ) ;
				matCamera.RevolveByAngleOn( vTargetPos - vViewPos ) ;
				//
				vPos -= vViewPos ;
				matCamera.RevolveVector( vPos ) ;
				//
				rRevAngle -=
					(REAL32) (rCameraAngleZ
								* (180.0 / 3.1415926535897932384626433832795)) ;
			}
			else
			{
				ESLTrace( "Failed to Get3DViewCamera\n" ) ;
			}
		}
		//
		rZOrder = vPos.z ;
		//
		if ( vPos.z >= 1 )
		{
			REAL32	rZScale = (REAL32) (m_ispParam.rZScale / vPos.z) ;
			xZoom *= rZScale ;
			yZoom *= rZScale ;
			//
			if ( pScreenPos != NULL )
			{
				rZScale = pScreenPos->z / vPos.z ;
				vPos.x = vPos.x * rZScale + pScreenPos->x ;
				vPos.y = vPos.y * rZScale + pScreenPos->y ;
			}
			if ( (fabs(vPos.x) < 0x4000) && (fabs(vPos.y) < 0x4000) )
			{
				if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
				{
					m_eglParam.ptBasePos.x =
						eriRoundR32ToInt( (REAL32) (vPos.x * 0x10000) ) ;
					m_eglParam.ptBasePos.y =
						eriRoundR32ToInt( (REAL32) (vPos.y * 0x10000) ) ;
				}
				else
				{
					m_eglParam.ptBasePos.x = eriRoundR32ToInt( (REAL32) vPos.x ) ;
					m_eglParam.ptBasePos.y = eriRoundR32ToInt( (REAL32) vPos.y ) ;
				}
			}
			else
			{
				xZoom = 0 ;
				yZoom = 0 ;
			}
		}
		else
		{
			xZoom = 0 ;
			yZoom = 0 ;
		}
	}
	else
	{
		m_f3d = false ;
	}
	//
	::eglGetRevolvedAxes
		( &m_eglAxes, &m_eglParam.ptBasePos, &ptRevCenter,
			xZoom, yZoom, rRevAngle, m_ispParam.rCrossingAngle, 0 ) ;
	//
	if ( !(m_eglParam.dwFlags & EGL_FIXED_POSITION) )
	{
		m_eglParam.dwFlags |= EGL_FIXED_POSITION ;
		m_eglParam.ptBasePos.x <<= 16 ;
		m_eglParam.ptBasePos.y <<= 16 ;
	}
	m_eglParam.pSrcImage = m_pImage ;
	m_eglParam.pViewRect = NULL ;
	m_eglParam.rgbDimColor = m_ispParam.rgbDimColor ;
	m_eglParam.rgbLightColor = m_ispParam.rgbLightColor ;
	m_eglParam.nTransparency = m_ispParam.nTransparency ;
	m_eglParam.rZOrder = rZOrder ;
	m_eglParam.pImageAxes = &m_eglAxes ;
	m_eglParam.rgbColorParam1 = m_ispParam.rgbColorParam1 ;
	//
	if ( (fabs( m_eglAxes.xAxis.x - m_eglAxes.yAxis.y ) < 1.0e-5)
		&& (fabs( m_eglAxes.xAxis.y - m_eglAxes.yAxis.x ) < 1.0e-5)
		&& (fabs( m_eglAxes.xAxis.x - 1 ) < 1.0e-5)
		&& (fabs( m_eglAxes.xAxis.y ) < 1.0e-5)	)
	{
		if ( !(m_eglParam.dwFlags & EGL_SMOOTH_STRETCH) )
		{
			m_eglParam.ptBasePos.x &= ~0xFFFF ;
			m_eglParam.ptBasePos.y &= ~0xFFFF ;
		}
		m_eglParam.pImageAxes = NULL ;
		//
		m_rectExt.left = (m_eglParam.ptBasePos.x >> 16) ;
		m_rectExt.top = (m_eglParam.ptBasePos.y >> 16) ;
		m_rectExt.right =
			((m_eglParam.ptBasePos.x + 0xFFFF) >> 16) + sizeImage.w ;
		m_rectExt.bottom =
			((m_eglParam.ptBasePos.y + 0xFFFF) >> 16) + sizeImage.h ;
		//
		m_rectHidden.left = ((m_eglParam.ptBasePos.x + 0xFFFF) >> 16) ;
		m_rectHidden.top = ((m_eglParam.ptBasePos.y + 0xFFFF) >> 16) ;
		m_rectHidden.right = (m_eglParam.ptBasePos.x >> 16) + sizeImage.w - 1 ;
		m_rectHidden.bottom = (m_eglParam.ptBasePos.y >> 16) + sizeImage.h - 1 ;
	}
	else
	{
		EGL_RECT	rectExt ;
		rectExt.left = 0 ;
		rectExt.top = 0 ;
		rectExt.right = sizeImage.w ;
		rectExt.bottom = sizeImage.h ;
		m_rectExt = LocalToGlobal( rectExt ) ;
	}
	//
//	EImageSprite::UpdateRect( NULL ) ;
}

// 画像バッファ関連付け
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::AttachImage( PEGL_IMAGE_INFO pImage )
{
	UpdateRect( NULL ) ;
	EGLImage::AttachImage( pImage ) ;
	if ( pImage != NULL )
	{
		if ( pImage->fdwFormatType == EIF_RGBA_BITMAP )
		{
			m_ispParam.dwFlags =
				(m_ispParam.dwFlags
					& ~EGL_DRAW_GLOW_LIGHT) | EGL_DRAW_BLEND_ALPHA ;
		}
		else if ( pImage->fdwFormatType == EIF_GRAY_BITMAP )
		{
			m_ispParam.dwFlags |=
				EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
//			m_ispParam.rgbDimColor.dwPixelCode = 0x00FFFFFF ;
//			m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
		}
		else
		{
//			m_ispParam.dwFlags = 0 ;
		}
	}
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// 画像バッファの参照を設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetImageView
	( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect )
{
	UpdateRect( NULL ) ;
	EGLImage::SetImageView( pImage, pViewRect ) ;
	if ( pImage != NULL )
	{
		if ( pImage->fdwFormatType == EIF_RGBA_BITMAP )
		{
			m_ispParam.dwFlags |=
				(m_ispParam.dwFlags
					& ~EGL_DRAW_GLOW_LIGHT) | EGL_DRAW_BLEND_ALPHA ;
		}
		else if ( pImage->fdwFormatType == EIF_GRAY_BITMAP )
		{
			m_ispParam.dwFlags |=
				EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
	//		m_ispParam.rgbDimColor.dwPixelCode = 0x00FFFFFF ;
	//		m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
		}
		else
		{
	//		m_ispParam.dwFlags = 0 ;
		}
	}
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// 画像バッファ作成
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EImageSprite::CreateImage
	( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
			DWORD dwBitsPerPixel, DWORD dwFlags )
{
	EGLImage::CreateImage
		( fdwFormat, dwWidth, dwHeight, dwBitsPerPixel, dwFlags ) ;
	m_ispParam.dwFlags &= ~maskEGLFlags ;
	if ( fdwFormat == EIF_RGBA_BITMAP )
	{
		m_ispParam.dwFlags |= EGL_DRAW_BLEND_ALPHA ;
	}
	else if ( fdwFormat == EIF_GRAY_BITMAP )
	{
		m_ispParam.dwFlags |=
			EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
		m_ispParam.rgbDimColor.dwPixelCode = 0x00FFFFFF ;
		m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
	}
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
	return	GetInfo( ) ;
}

// 画像バッファ複製
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EImageSprite::DuplicateImage
	( PCEGL_IMAGE_INFO pImage, DWORD dwFlags )
{
	EGLImage::DuplicateImage( pImage, dwFlags ) ;
	m_ispParam.dwFlags &= ~maskEGLFlags ;
	if ( GetInfo()->fdwFormatType == EIF_RGBA_BITMAP )
	{
		m_ispParam.dwFlags |= EGL_DRAW_BLEND_ALPHA ;
	}
	else if ( GetInfo()->fdwFormatType == EIF_GRAY_BITMAP )
	{
		m_ispParam.dwFlags |=
			EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
		m_ispParam.rgbDimColor.dwPixelCode = 0x00FFFFFF ;
		m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
	}
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
	return	GetInfo( ) ;
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EImageSprite::ReadImageFile( ESLFileObject & file )
{
	ESLError	err = EGLImage::ReadImageFile( file ) ;
	if ( !err )
	{
		if ( GetInfo()->fdwFormatType == EIF_RGBA_BITMAP )
		{
			m_ispParam.dwFlags = EGL_DRAW_BLEND_ALPHA ;
		}
		else if ( GetInfo()->fdwFormatType == EIF_GRAY_BITMAP )
		{
			m_ispParam.dwFlags =
				EGL_DRAW_BLEND_ALPHA | EGL_DRAW_GLOW_LIGHT ;
			m_ispParam.rgbDimColor.dwPixelCode = 0x00FFFFFF ;
			m_ispParam.rgbLightColor.dwPixelCode = 0x00FFFFFF ;
		}
		else
		{
			m_ispParam.dwFlags = 0 ;
		}
	}
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
	return	err ;
}

// パラメータ取得
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::GetParameter( EImageSprite::PARAMETER & param ) const
{
	param = m_ispParam ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetParameter( const EImageSprite::PARAMETER & param )
{
	EImageSprite::UpdateRect( NULL ) ;
	//
	m_ispParam = param ;
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// パラメータ複製
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::CopyParameters( const EImageSprite * pSrc )
{
	EImageSprite::UpdateRect( NULL ) ;
	//
	m_ispParam = pSrc->m_ispParam ;
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// 表示状態を取得
//////////////////////////////////////////////////////////////////////////////
bool EImageSprite::IsVisible( void )
{
	if ( m_ispParam.dwFlags & flagZScale )
	{
		SetParameterToDraw() ;
		if ( m_eglParam.rZOrder < 1 )
		{
			return	false ;
		}
	}
	return	m_visible && (m_ispParam.nTransparency < 0x100) ;
}

// 表示状態を取得（副作用なし）
//////////////////////////////////////////////////////////////////////////////
bool EImageSprite::IsVisibleMT( void ) const
{
	return	m_visible && (m_ispParam.nTransparency < 0x100) ;
}

// 表示基準座標取得
//////////////////////////////////////////////////////////////////////////////
EGL_POINT EImageSprite::GetPosition( void )
{
	return	m_ispParam.ptDstPos ;
}

// 表示基準座標設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::MovePosition( EGL_POINT ptDstPos )
{
	EImageSprite::UpdateRect( NULL ) ;
	//
	m_ispParam.ptDstPos = ptDstPos ;
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// 透明度取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EImageSprite::GetTransparency( void ) const
{
	return	m_ispParam.nTransparency ;
}

// 透明度設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetTransparency( unsigned int nTransparency )
{
	if ( nTransparency == 0x100 )
	{
		EImageSprite::UpdateRect( NULL ) ;
	}
	m_eglParam.nTransparency =
		m_ispParam.nTransparency = nTransparency ;
	EImageSprite::UpdateRect( NULL ) ;
}

// ｚ座標取得
//////////////////////////////////////////////////////////////////////////////
REAL32 EImageSprite::GetZPosition( void ) const
{
	return	m_ispParam.rZOrder ;
}

// ｚ座標設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetZPosition( REAL32 zPos )
{
	EImageSprite::UpdateRect( NULL ) ;
	m_ispParam.rZOrder = zPos ;
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// ｚスケール取得
//////////////////////////////////////////////////////////////////////////////
REAL32 EImageSprite::GetZScale( void ) const
{
	return	m_ispParam.rZScale ;
}

// ｚスケール設定
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::SetZScale( REAL32 zScale )
{
	EImageSprite::UpdateRect( NULL ) ;
	m_ispParam.rZScale = zScale ;
	SetParameterToDraw( ) ;
	EImageSprite::UpdateRect( NULL ) ;
}

// ローカル座標からグローバル座標に変換
//////////////////////////////////////////////////////////////////////////////
EGL_POINT EImageSprite::LocalToGlobal
	( EGL_POINT ptLocal, const EGL_DRAW_PARAM & eglParam )
{
	EGL_POINT	point ;
	if ( eglParam.dwFlags & EGL_FIXED_POSITION )
	{
		if ( eglParam.pImageAxes == NULL )
		{
			point.x = ptLocal.x + (eglParam.ptBasePos.x >> 16) ;
			point.y = ptLocal.y + (eglParam.ptBasePos.y >> 16) ;
		}
		else
		{
			point.x = (eglParam.ptBasePos.x >> 16)
				+ ::eriRoundR32ToInt
					( ptLocal.x * eglParam.pImageAxes->xAxis.x
								+ ptLocal.y * eglParam.pImageAxes->yAxis.x ) ;
			point.y = (eglParam.ptBasePos.y >> 16)
				+ ::eriRoundR32ToInt
					( ptLocal.x * eglParam.pImageAxes->xAxis.y
								+ ptLocal.y * eglParam.pImageAxes->yAxis.y ) ;
		}
	}
	else
	{
		if ( eglParam.pImageAxes == NULL )
		{
			point.x = ptLocal.x + eglParam.ptBasePos.x ;
			point.y = ptLocal.y + eglParam.ptBasePos.y ;
		}
		else
		{
			point.x = eglParam.ptBasePos.x
				+ ::eriRoundR32ToInt
					( ptLocal.x * eglParam.pImageAxes->xAxis.x
								+ ptLocal.y * eglParam.pImageAxes->yAxis.x ) ;
			point.y = eglParam.ptBasePos.y
				+ ::eriRoundR32ToInt
					( ptLocal.x * eglParam.pImageAxes->xAxis.y
								+ ptLocal.y * eglParam.pImageAxes->yAxis.y ) ;
		}
	}
	return	point ;
}

EGL_RECT EImageSprite::LocalToGlobal
	( const EGL_RECT & rectLocal, const EGL_DRAW_PARAM & eglParam )
{
	EGL_RECT	rect ;
	if ( eglParam.pImageAxes == NULL )
	{
		if ( eglParam.dwFlags & EGL_FIXED_POSITION )
		{
			rect.left = rectLocal.left + (eglParam.ptBasePos.x >> 16) ;
			rect.top = rectLocal.top + (eglParam.ptBasePos.y >> 16) ;
			rect.right = rectLocal.right
							+ ((eglParam.ptBasePos.x + 0xFFFF) >> 16) ;
			rect.bottom = rectLocal.bottom
							+ ((eglParam.ptBasePos.y + 0xFFFF) >> 16) ;
		}
		else
		{
			rect.left = rectLocal.left + eglParam.ptBasePos.x ;
			rect.top = rectLocal.top + eglParam.ptBasePos.y ;
			rect.right = rectLocal.right + eglParam.ptBasePos.x ;
			rect.bottom = rectLocal.bottom + eglParam.ptBasePos.y ;
		}
	}
	else
	{
		EGL_POINT	point[4] ;
		point[0].x = rectLocal.left - 1 ;
		point[0].y = rectLocal.top - 1 ;
		point[1].x = rectLocal.right + 1 ;
		point[1].y = rectLocal.top - 1 ;
		point[2].x = rectLocal.left - 1 ;
		point[2].y = rectLocal.bottom + 1 ;
		point[3].x = rectLocal.right + 1 ;
		point[3].y = rectLocal.bottom + 1 ;
		//
		int	i ;
		for ( i = 0; i < 4; i ++ )
		{
			point[i] = LocalToGlobal( point[i], eglParam ) ;
		}
		//
		EGL_POINT	ptMin = point[0], ptMax = point[0] ;
		for ( i = 1; i < 4; i ++ )
		{
			if ( ptMin.x > point[i].x )
				ptMin.x = point[i].x ;
			else if ( ptMax.x < point[i].x )
				ptMax.x = point[i].x ;
			//
			if ( ptMin.y > point[i].y )
				ptMin.y = point[i].y ;
			else if ( ptMax.y < point[i].y )
				ptMax.y = point[i].y ;
		}
		//
		rect.left = ptMin.x - 1 ;
		rect.top = ptMin.y - 1 ;
		rect.right = ptMax.x + 2 ;
		rect.bottom = ptMax.y + 2 ;
	}
	return	rect ;
}

// グローバル座標からローカル座標に変換
//////////////////////////////////////////////////////////////////////////////
EGL_POINT EImageSprite::GlobalToLocal( EGL_POINT ptGlobal ) const
{
	double	d = (m_eglAxes.xAxis.x * m_eglAxes.yAxis.y
						- m_eglAxes.xAxis.y * m_eglAxes.yAxis.x) ;
	if ( fabs(d) > 1.0e-5 )
	{
		d = 1.0 / d ;
	}
	else
	{
		d = 0.0 ;
	}
	//
	E3D_VECTOR_2D	vDeltaX, vDeltaY ;
	vDeltaX.x = (REAL32)(m_eglAxes.yAxis.y * d) ;
	vDeltaX.y = (REAL32)(- m_eglAxes.xAxis.y * d) ;
	vDeltaY.x = (REAL32)(- m_eglAxes.yAxis.x * d) ;
	vDeltaY.y = (REAL32)(m_eglAxes.xAxis.x * d) ;
	//
	EGL_POINT	point ;
	if ( m_ispParam.dwFlags & flagZScale )
	{
		if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
		{
			ptGlobal.x -= (m_eglParam.ptBasePos.x >> 16) ;
			ptGlobal.y -= (m_eglParam.ptBasePos.y >> 16) ;
		}
		else
		{
			ptGlobal.x -= m_eglParam.ptBasePos.x ;
			ptGlobal.y -= m_eglParam.ptBasePos.y ;
		}
		point.x = 0 ;
		point.y = 0 ;
	}
	else if ( m_ispParam.dwFlags & EGL_FIXED_POSITION )
	{
		ptGlobal.x -= (m_ispParam.ptDstPos.x >> 16) ;
		ptGlobal.y -= (m_ispParam.ptDstPos.y >> 16) ;
		point.x = (m_ispParam.ptRevCenter.x >> 16) ;
		point.y = (m_ispParam.ptRevCenter.y >> 16) ;
	}
	else
	{
		ptGlobal.x -= m_ispParam.ptDstPos.x ;
		ptGlobal.y -= m_ispParam.ptDstPos.y ;
		point.x = m_ispParam.ptRevCenter.x ;
		point.y = m_ispParam.ptRevCenter.y ;
	}
	point.x +=
		(int) ::eriRoundR64ToLInt( vDeltaX.x * ptGlobal.x
									+ vDeltaY.x * ptGlobal.y ) ;
	point.y +=
		(int) ::eriRoundR64ToLInt( vDeltaX.y * ptGlobal.x
									+ vDeltaY.y * ptGlobal.y ) ;
	return	point ;
}

// 画像描画パラメータ取得
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::GetDrawParameter3DView
	( EGL_DRAW_PARAM & dp, const VIEW3D_INFO & v3dInfo ) const
{
	//
	// 描画パラメータ強制固定小数点
	//
	dp = m_eglParam ;
	if ( !(dp.dwFlags & EGL_FIXED_POSITION) )
	{
		dp.dwFlags |= EGL_FIXED_POSITION ;
		dp.ptBasePos.x = (dp.ptBasePos.x << 16) ;
		dp.ptBasePos.y = (dp.ptBasePos.y << 16) ;
	}
	//
	// xp, yp = 投影座標, z = ｚ座標
	// p = ｘ視差, r = 焦点距離, s = 投影スクリーンｚ座標
	// x = xp * z / s
	// x' = x - p + p * z / r
	// xp' = x' * s / z
	//     = xp + p * s * ((z - r)/(z * r))
	//
	double	zScreen = v3dInfo.zFocus ;
	const E3D_VECTOR *	pScreenPos = GetScreen3DPosition() ;
	if ( pScreenPos != NULL )
	{
		zScreen = pScreenPos->z ;
	}
	double	z = m_ispParam.rZOrder + v3dInfo.zOffset ;
	double	zMinLimit = fabs(v3dInfo.xParallax) * 2 ;
	if ( z < zMinLimit )
	{
		z = zMinLimit ;
	}
	double	xParallax =
		v3dInfo.xParallax * zScreen
			* ((z - v3dInfo.zFocus) / (z * v3dInfo.zFocus)) ;
	int	fxDeltaX = eriRoundR32ToInt( (REAL32) (xParallax * 0x10000) ) ;
	dp.ptBasePos.x += fxDeltaX ;
	//
	if ( !(m_ispParam.dwFlags & EGL_FIXED_POSITION) )
	{
		dp.ptBasePos.x &= ~0xFFFF ;
	}
}

// 外接矩形を取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT EImageSprite::GetRectangle( void )
{
	return	m_rectExt ;
}

EGL_RECT EImageSprite::GetRectangle3DView( const VIEW3D_INFO & v3dInfo )
{
	if ( m_pImage == NULL )
	{
		return	EGLRect( 0, 0, -1, -1 ) ;
	}
	//
	EGL_DRAW_PARAM	dp ;
	GetDrawParameter3DView( dp, v3dInfo ) ;
	//
	EGL_RECT	rect ;
	rect.left = 0 ;
	rect.top = 0 ;
	rect.right = m_pImage->dwImageWidth ;
	rect.bottom = m_pImage->dwImageHeight ;
	//
	return	LocalToGlobal( rect, dp ) ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool EImageSprite::GetHiddenRectangle( EGL_RECT & rect )
{
	if ( (m_pImage == NULL)
		|| (m_eglParam.dwFlags & 0xFFFF0000) 
		|| (m_eglParam.pImageAxes != NULL)
		|| (m_eglParam.nTransparency > 0) || !m_visible )
	{
		return	false ;
	}
	ESLAssert( m_pImage != NULL ) ;
	if ( (m_pImage->fdwFormatType & (EIF_WITH_ALPHA | EIF_WITH_CLIPPING))
					|| (m_pImage->fdwFormatType == EIF_GRAY_BITMAP) )
	{
		return	false ;
	}
	rect = m_rectHidden ;
	return	true ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::Draw( HEGL_RENDER_POLYGON hRenderPoly )
{
	BeforeMTDraw() ;
	MTDraw( hRenderPoly ) ;
}

// スプライト描画（立体視表示用）
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::DrawTo3DView
	( HEGL_RENDER_POLYGON hRenderPoly, const ESprite::VIEW3D_INFO & v3dInfo )
{
	BeforeMTDraw() ;
	MTDrawTo3DView( hRenderPoly, v3dInfo ) ;
}

// マルチスレッド描画前準備処理
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::BeforeMTDraw( void )
{
	if ( Is3DSprite() )
	{
		SetParameterToDraw() ;
	}
}

// スプライト描画（マルチスレッド）
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	if ( m_pImage != NULL )
	{
		HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
		if ( !hDraw->PrepareDraw( &m_eglParam ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
}

// スプライト描画（立体視表示用・マルチスレッド）
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::MTDrawTo3DView
	( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo )
{
	if ( m_pImage != NULL )
	{
		if ( m_pImage->fdwFormatType & EIF_SIDE_BY_SIDE )
		{
			HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
			EGL_DRAW_PARAM	dp = m_eglParam ;
			EGL_IMAGE_INFO	infLeft ;
			if ( v3dInfo.nViewIndex == sviStereoViewRight )
			{
				dp.pSrcImage = m_pImage ;
			}
			else
			{
				::eglGetStereoLeftImageBuffer( m_pImage, &infLeft ) ;
				dp.pSrcImage = &infLeft ;
			}
			if ( !hDraw->PrepareDraw( &dp ) )
			{
				hDraw->DrawImage( ) ;
			}
		}
		else
		{
			EGL_DRAW_PARAM	dp ;
			GetDrawParameter3DView( dp, v3dInfo ) ;
			//
			HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
			if ( !hDraw->PrepareDraw( &dp ) )
			{
				hDraw->DrawImage( ) ;
			}
		}
	}
}

// スプライト上の指定領域の更新通知
//////////////////////////////////////////////////////////////////////////////
/*
bool EImageSprite::UpdateRect( EGL_RECT * pUpdateRect )
{
	if ( pUpdateRect == NULL )
	{
		return	ESprite::UpdateRect( NULL ) ;
	}
	else
	{
		EGL_RECT	rect = LocalToGlobal( *pUpdateRect ) ;
		return	ESprite::UpdateRect( &rect ) ;
	}
}
*/

// ローカル→大域座標変換
//////////////////////////////////////////////////////////////////////////////
void EImageSprite::LocalRectToGlocal3DView
	( EGL_RECT & rect, const VIEW3D_INFO * pv3dInfo )
{
	EGL_RECT	rctTemp ;
	if ( pv3dInfo == NULL )
	{
		rctTemp = LocalToGlobal( rect ) ;
	}
	else
	{
		EGL_DRAW_PARAM	dp ;
		GetDrawParameter3DView( dp, *pv3dInfo ) ;
		//
		rctTemp = LocalToGlobal( rect, dp ) ;
	}
	rect = rctTemp ;
}


//////////////////////////////////////////////////////////////////////////////
// フィルタスプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EFilterSprite, ESprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EFilterSprite::EFilterSprite( void )
{
	m_rectEffect.left = 0 ;
	m_rectEffect.top = 0 ;
	m_rectEffect.right = -1 ;
	m_rectEffect.bottom = -1 ;
	//
	SetBrightness( 0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EFilterSprite::~EFilterSprite( void )
{
}

// 矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT EFilterSprite::GetRectangle( void )
{
	return	m_rectEffect ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	//
	// 出力先を取得
	//
	HEGL_DRAW_IMAGE	hDrawImage = hRenderPoly->GetDrawImage( ) ;
	EGL_DRAW_DEST	eglDest ;
	hDrawImage->GetDestination( &eglDest ) ;
	//
	// 出力先画像を設定
	//
	EGLRect	rectDst = m_rectEffect ;
	if ( rectDst &= eglDest.rectDstClip )
	{
		EGL_IMAGE_INFO	eglImage ;
		EGLImageRect	rectClip = rectDst ;
		if ( !::eglGetClippedImageInfo
				( &eglImage, eglDest.pDstImage, &rectClip ) )
		{
			//
			// フィルタ処理
			//
			::eglApplyToneTable
				( &eglImage, &eglImage,
					m_bytBlue, m_bytGreen, m_bytRed, m_bytAlpha ) ;
		}
	}
}

// 矩形設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetRectangle( const EGL_RECT & rect )
{
	UpdateRect( NULL ) ;
	m_rectEffect = rect ;
	UpdateRect( NULL ) ;
}

// 輝度を設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetBrightness( int nBrightness )
{
	SetColorTone( nBrightness, nBrightness, nBrightness, nBrightness ) ;
}

// チャネルごとの輝度を設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetColorTone
	( int nRed, int nGreen, int nBlue, int nAlpha )
{
	::eglCalculateToneTable( m_bytBlue, nBlue, EGL_TONE_BRIGHTNESS ) ;
	::eglCalculateToneTable( m_bytGreen, nGreen, EGL_TONE_BRIGHTNESS ) ;
	::eglCalculateToneTable( m_bytRed, nRed, EGL_TONE_BRIGHTNESS ) ;
	::eglCalculateToneTable( m_bytAlpha, nAlpha, EGL_TONE_BRIGHTNESS ) ;
	UpdateRect( NULL ) ;
}

// 反転フィルタを設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetInversionTone
	( int nRed, int nGreen, int nBlue, int nAlpha )
{
	::eglCalculateToneTable( m_bytBlue, nBlue, EGL_TONE_INVERSION ) ;
	::eglCalculateToneTable( m_bytGreen, nGreen, EGL_TONE_INVERSION ) ;
	::eglCalculateToneTable( m_bytRed, nRed, EGL_TONE_INVERSION ) ;
	::eglCalculateToneTable( m_bytAlpha, nAlpha, EGL_TONE_INVERSION ) ;
	UpdateRect( NULL ) ;
}

// ライトフィルタを設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetLightTone
	( int nRed, int nGreen, int nBlue, int nAlpha )
{
	::eglCalculateToneTable( m_bytBlue, nBlue, EGL_TONE_LIGHT ) ;
	::eglCalculateToneTable( m_bytGreen, nGreen, EGL_TONE_LIGHT ) ;
	::eglCalculateToneTable( m_bytRed, nRed, EGL_TONE_LIGHT ) ;
	::eglCalculateToneTable( m_bytAlpha, nAlpha, EGL_TONE_LIGHT ) ;
	UpdateRect( NULL ) ;
}

// 複雑なフィルタを設定
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetGeneralTone
	( int nRedTone, int nRedFlag, int nGreenTone, int nGreenFlag,
		int nBlueTone, int nBlueFlag, int nAlphaTone, int nAlphaFlag )
{
	::eglCalculateToneTable( m_bytBlue, nBlueTone, nBlueFlag ) ;
	::eglCalculateToneTable( m_bytGreen, nGreenTone, nGreenFlag ) ;
	::eglCalculateToneTable( m_bytRed, nRedTone, nRedFlag ) ;
	::eglCalculateToneTable( m_bytAlpha, nAlphaTone, nAlphaFlag ) ;
	UpdateRect( NULL ) ;
}

// トーンテーブルを複製
//////////////////////////////////////////////////////////////////////////////
void EFilterSprite::SetToneTables
	( const void * pRed, const void * pGreen,
		const void * pBlue, const void * pAlpha )
{
	::memmove( m_bytBlue, pBlue, sizeof(m_bytBlue) ) ;
	::memmove( m_bytGreen, pGreen, sizeof(m_bytGreen) ) ;
	::memmove( m_bytRed, pRed, sizeof(m_bytRed) ) ;
	::memmove( m_bytAlpha, pAlpha, sizeof(m_bytAlpha) ) ;
	UpdateRect( NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 形状スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EShapeSprite, ESprite )

// 表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool EShapeSprite::IsVisible( void )
{
	return	m_visible && (m_nTransparency < 0x100) ;
}

// 描画色取得
//////////////////////////////////////////////////////////////////////////////
void EShapeSprite::SetColor( EGL_PALETTE rgbaColor )
{
	m_rgbaColor.m_palette = rgbaColor ;
	UpdateRect( NULL ) ;
}

// 透明度取得
//////////////////////////////////////////////////////////////////////////////
void EShapeSprite::SetTransparency( unsigned int nTransparency )
{
	m_nTransparency = nTransparency ;
	UpdateRect( NULL ) ;
}

// 描画フラグ取得
//////////////////////////////////////////////////////////////////////////////
void EShapeSprite::SetDrawFlag( DWORD dwFlags )
{
	m_dwFlags = dwFlags ;
	UpdateRect( NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 直線スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ELineSprite, EShapeSprite )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ELineSprite::~ELineSprite( void )
{
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ELineSprite::GetRectangle( void )
{
	return	m_rectExt ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ELineSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	HEGL_DRAW_IMAGE	hDrawImage = hRenderPoly->GetDrawImage( ) ;
	//
	unsigned int	i, nCount ;
	nCount = m_lines.GetSize( ) ;
	for ( i = 1; i < nCount; i ++ )
	{
		EGL_POINT *	p1 = m_lines.GetAt( i - 1 ) ;
		EGL_POINT *	p2 = m_lines.GetAt( i ) ;
		ESLAssert( p1 && p2 ) ;
		if ( !hDrawImage->PrepareLine
			( p1->x, p1->y, p2->x, p2->y,
				m_rgbaColor, m_nTransparency, m_dwFlags ) )
		{
			hDrawImage->FillRegion( ) ;
		}
	}
}

// 直線を設定
//////////////////////////////////////////////////////////////////////////////
void ELineSprite::SetLines( const EGL_POINT * pLines, unsigned int nCount )
{
	UpdateRect( NULL ) ;
	m_lines.RemoveAll( ) ;
	//
	if ( nCount >= 1 )
	{
		m_rectExt.left = m_rectExt.right = pLines[0].x ;
		m_rectExt.top = m_rectExt.bottom = pLines[0].y ;
		//
		for ( unsigned int i = 0; i < nCount; i ++ )
		{
			EGL_POINT *	pt = new EGL_POINT( pLines[i] ) ;
			m_lines.Add( pt ) ;
			//
			if ( pt->x < m_rectExt.left )
				m_rectExt.left = pt->x ;
			else if ( pt->x > m_rectExt.right )
				m_rectExt.right = pt->x ;
			//
			if ( pt->y < m_rectExt.top )
				m_rectExt.top = pt->y ;
			else if ( pt->y > m_rectExt.bottom )
				m_rectExt.bottom = pt->y ;
		}
		UpdateRect( NULL ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 矩形スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ERectangleSprite, EShapeSprite )

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ERectangleSprite::GetRectangle( void )
{
	return	m_rectFill ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ERectangleSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	HEGL_DRAW_IMAGE	hDrawImage = hRenderPoly->GetDrawImage( ) ;
	if ( !hDrawImage->PrepareFillRect
		( &m_rectFill, m_rgbaColor, m_nTransparency, m_dwFlags ) )
	{
		hDrawImage->FillRegion( ) ;
	}
}

// 直線を設定
//////////////////////////////////////////////////////////////////////////////
void ERectangleSprite::SetRectangle( const EGL_RECT & rectFill )
{
	UpdateRect( NULL ) ;
	m_rectFill = rectFill ;
	UpdateRect( NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 楕円スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EEllipseSprite, EShapeSprite )

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT EEllipseSprite::GetRectangle( void )
{
	EGL_RECT	rect ;
	if ( m_sizeRadius.w >= 0 )
	{
		rect.left = m_ptCenter.x - m_sizeRadius.w ;
		rect.right = m_ptCenter.x + m_sizeRadius.w ;
	}
	else
	{
		rect.left = m_ptCenter.x + m_sizeRadius.w ;
		rect.right = m_ptCenter.x - m_sizeRadius.w ;
	}
	if ( m_sizeRadius.h >= 0 )
	{
		rect.top = m_ptCenter.y - m_sizeRadius.h ;
		rect.bottom = m_ptCenter.y + m_sizeRadius.h ;
	}
	else
	{
		rect.top = m_ptCenter.y + m_sizeRadius.h ;
		rect.bottom = m_ptCenter.y - m_sizeRadius.h ;
	}
	return	rect ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void EEllipseSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	HEGL_DRAW_IMAGE	hDrawImage = hRenderPoly->GetDrawImage( ) ;
	if ( !hDrawImage->PrepareFillEllipse
		( &m_ptCenter, &m_sizeRadius,
			m_rgbaColor, m_nTransparency, m_dwFlags ) )
	{
		hDrawImage->FillRegion( ) ;
	}
}

// 中心点を設定
//////////////////////////////////////////////////////////////////////////////
void EEllipseSprite::MovePosition( EGL_POINT ptCenter )
{
	UpdateRect( NULL ) ;
	m_ptCenter = ptCenter ;
	UpdateRect( NULL ) ;
}

// 半径を設定
//////////////////////////////////////////////////////////////////////////////
void EEllipseSprite::SetRadiusSize( EGL_SIZE sizeRadius )
{
	UpdateRect( NULL ) ;
	m_sizeRadius = sizeRadius ;
	UpdateRect( NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 多角形スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EPolygonSprite, EShapeSprite )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EPolygonSprite::~EPolygonSprite( void )
{
	delete []	m_pPolygon ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT EPolygonSprite::GetRectangle( void )
{
	return	m_rectExt ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void EPolygonSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	if ( m_nVertexes >= 3 )
	{
		HEGL_DRAW_IMAGE	hDrawImage = hRenderPoly->GetDrawImage( ) ;
		if ( !hDrawImage->PrepareFillPolygon
			( m_pPolygon, m_nVertexes,
				m_rgbaColor, m_nTransparency, m_dwFlags ) )
		{
			hDrawImage->FillRegion( ) ;
		}
	}
}

// 多角形を設定
//////////////////////////////////////////////////////////////////////////////
void EPolygonSprite::SetPolygon
	( const EGL_POINT * pPolygon, unsigned int nCount )
{
	UpdateRect( NULL ) ;
	//
	if ( m_pPolygon != NULL )
	{
		delete []	m_pPolygon ;
		m_pPolygon = NULL ;
	}
	m_nVertexes = nCount ;
	//
	m_rectExt.left = 0x7FFFFFFF ;
	m_rectExt.top = 0x7FFFFFFF ;
	m_rectExt.right = 0x80000000 ;
	m_rectExt.bottom = 0x80000000 ;
	//
	if ( (nCount >= 3) && (pPolygon != NULL) )
	{
		m_pPolygon = new EGL_POINT[nCount] ;
		for ( unsigned int i = 0; i < nCount; i ++ )
		{
			m_pPolygon[i] = pPolygon[i] ;
			//
			if ( m_rectExt.left > pPolygon[i].x )
				m_rectExt.left = pPolygon[i].x - 1 ;
			if ( m_rectExt.right < pPolygon[i].x )
				m_rectExt.right = pPolygon[i].x + 1 ;
			if ( m_rectExt.top > pPolygon[i].y )
				m_rectExt.top = pPolygon[i].y - 1 ;
			if ( m_rectExt.bottom < pPolygon[i].y )
				m_rectExt.bottom = pPolygon[i].y + 1 ;
		}
		UpdateRect( NULL ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 3D 表示スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( E3DRenderSprite, ESprite, E3DRenderPolygon )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DRenderSprite::E3DRenderSprite( void )
{
	m_rectExt.left = m_rectExt.top = 0 ;
	m_rectExt.right = m_rectExt.bottom = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DRenderSprite::~E3DRenderSprite( void )
{
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT E3DRenderSprite::GetRectangle( void )
{
	return	m_rectExt ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void E3DRenderSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	RenderAllPolygon( hRenderPoly ) ;
}

// レンダリング準備
//////////////////////////////////////////////////////////////////////////////
ESLError E3DRenderSprite::PrepareRendering( void )
{
	ESLError	errResult ;
	UpdateRect( NULL ) ;
	errResult = E3DRenderPolygon::PrepareRendering( ) ;
	if ( !errResult )
	{
		if ( m_hRenderPoly != NULL )
		{
			errResult = GetExternalRect( &m_rectExt ) ;
		}
		else
		{
			m_rectExt.left = m_rectExt.top = 0 ;
			m_rectExt.right = m_rectExt.bottom = -1 ;
		}
		UpdateRect( NULL ) ;
	}
	return	errResult ;
}


//////////////////////////////////////////////////////////////////////////////
// スプライトサーバー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESpriteServer, EImageSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESpriteServer::ESpriteServer( void )
{
	m_fStereoView = false ;
	//
	m_vScreen.x = 0 ;
	m_vScreen.y = 0 ;
	m_vScreen.z = 1024 ;
	//
	m_hRenderPoly = NULL ;
	m_dwRenderFlags =
		(E3D_FLAG_ANTIALIAS_SIDE_EDGE | E3D_FLAG_TEXTURE_SMOOTHING) ;
	m_dwDrawFlags = EGL_SMOOTH_STRETCH ;
	m_usStatus = usEmpty ;
	//
	m_fEnableDynamicMode = false ;
	m_fFillBack = true ;
	m_rgbBackColor.dwPixelCode = 0 ;
	//
	m_rectDynamic.left = 0 ;
	m_rectDynamic.top = 0 ;
	m_rectDynamic.right = -1 ;
	m_rectDynamic.bottom = -1 ;
	//
	m_fCamera = false ;
	m_vCameraPos.x = 0 ;
	m_vCameraPos.y = 0 ;
	m_vCameraPos.z = 0 ;
	m_vTargetPos.x = 0 ;
	m_vTargetPos.y = 0 ;
	m_vTargetPos.z = 1024.0 ;
	m_zRevAngle = 0 ;
	//
	for ( int i = 0; i < countMaxParallelThreads; i ++ )
	{
		m_hParallelRender[i] = NULL ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESpriteServer::~ESpriteServer( void )
{
	if ( m_hRenderPoly != NULL )
	{
		m_hRenderPoly->Release( ) ;
		m_hRenderPoly = NULL ;
	}
	for ( int i = 0; i < countMaxParallelThreads; i ++ )
	{
		if ( m_hParallelRender[i] != NULL )
		{
			m_hParallelRender[i]->Release() ;
			m_hParallelRender[i] = NULL ;
		}
	}
	RemoveAllSprite( ) ;
}

// 画像バッファ作成
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO ESpriteServer::CreateImage
	( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
					DWORD dwBitsPerPixel, DWORD dwFlags )
{
	REAL32	w ;
	m_vScreen.x = (REAL32) ((int) dwWidth * 0.5) ;
	m_vScreen.y = (REAL32) ((int) dwHeight * 0.5) ;
	w = m_vScreen.x ;
	if ( w > m_vScreen.y )
	{
		w = m_vScreen.y ;
	}
	m_vScreen.z = (REAL32) (w * 2.0) ;
	//
	PEGL_IMAGE_INFO	pImageInf =
		EImageSprite::CreateImage
			( fdwFormat, dwWidth, dwHeight, dwBitsPerPixel, dwFlags ) ;
	if ( pImageInf != NULL )
	{
		if ( pImageInf->fdwFormatType & EIF_SIDE_BY_SIDE )
		{
			EGL_IMAGE_INFO	infLeft ;
			if ( !::eglGetStereoLeftImageBuffer( pImageInf, &infLeft ) )
			{
				m_imgLeftBuf.SetImageView( &infLeft ) ;
			}
		}
	}
	return	pImageInf ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::DeleteImage( void )
{
	if ( m_hRenderPoly != NULL )
	{
		m_hRenderPoly->Release( ) ;
		m_hRenderPoly = NULL ;
	}
	EImageSprite::DeleteImage( ) ;
}

// 総スプライト数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ESpriteServer::GetSpriteCount( void ) const
{
	return	m_itaSprite.GetSize( ) ;
}

// スプライトを取得
//////////////////////////////////////////////////////////////////////////////
ESprite * ESpriteServer::GetSpriteAt( unsigned int index )
{
	ETaggedElement<int,ESprite> *	pElement = m_itaSprite.GetAt( index ) ;
	if ( pElement != NULL )
	{
		return	pElement->GetObject( ) ;
	}
	return	NULL ;
}

// 指定スプライトの指標を取得する
//////////////////////////////////////////////////////////////////////////////
int ESpriteServer::GetSpriteIndex( ESprite * pSprite )
{
	if ( pSprite == NULL )
	{
		return	-1 ;
	}
	unsigned int	iFind ;
	int				nPriority = pSprite->GetPriority() ;
	ESprite	*	pObj = m_itaSprite.GetAs( nPriority, &iFind ) ;
	if ( pObj != NULL )
	{
		ETaggedElement<int,ESprite> *	pElement ;
		if ( pObj == pSprite )
		{
			return	iFind ;
		}
		int		i = iFind - 1 ;
		while ( i >= 0 )
		{
			pElement = m_itaSprite.GetAt( i ) ;
			ESLAssert( pElement != NULL ) ;
			pObj = pElement->GetObject( ) ;
			if ( pObj == pSprite )
			{
				return	i ;
			}
			if ( pObj->GetPriority() != nPriority )
			{
				break ;
			}
			i -- ;
		}
		i = iFind + 1 ;
		while ( i < (int) m_itaSprite.GetSize() )
		{
			pElement = m_itaSprite.GetAt( i ) ;
			ESLAssert( pElement != NULL ) ;
			pObj = pElement->GetObject( ) ;
			if ( pObj == pSprite )
			{
				return	i ;
			}
			if ( pObj->GetPriority() != nPriority )
			{
				break ;
			}
			i ++ ;
		}
	}
	return	-1 ;
}

// スプライトを追加
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::AddSprite( int nPriority, ESprite * pSprite )
{
	if ( (pSprite != NULL) && (this != pSprite) )
	{
		if ( GetSpriteIndex( pSprite ) >= 0 )
		{
			ChangedChildPriority( pSprite, nPriority ) ;
		}
		else
		{
			ESLAssert( pSprite->GetParent() == NULL ) ;
			m_itaSprite.Add( nPriority, pSprite ) ;
			SetParentAs( pSprite ) ;
			pSprite->SetSpritePriority( nPriority ) ;
			UpdateChild( pSprite, NULL ) ;
		}
	}
}

// スプライトを分離
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteServer::DetachSprite( ESprite * pSprite )
{
	int	iSprite = GetSpriteIndex( pSprite ) ;
	if ( iSprite >= 0 )
	{
		ETaggedElement<int,ESprite> *	pElement ;
		pElement = m_itaSprite.GetAt( iSprite ) ;
		ESLAssert( pElement != NULL ) ;
		ESLAssert( pElement->GetObject() == pSprite ) ;
		if ( pElement->GetObject() != NULL )
		{
			UpdateChild( pSprite, NULL ) ;
			pSprite->SetSpriteParent( NULL ) ;
			pElement->DetachObject( ) ;
		}
		m_itaSprite.RemoveAt( iSprite ) ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 全てのスプライトを分離
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::DetachAllSprite( void )
{
	unsigned int	i, nCount ;
	nCount = m_itaSprite.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<int,ESprite> *	pElement ;
		pElement = m_itaSprite.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement->GetObject() != NULL )
		{
			pElement->GetObject()->SetSpriteParent( NULL ) ;
			pElement->DetachObject( ) ;
		}
	}
	m_itaSprite.RemoveAll( ) ;
}

// スプライトを削除
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteServer::RemoveSprite( ESprite * pSprite )
{
	if ( !DetachSprite( pSprite ) )
	{
		delete	pSprite ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// 全てのスプライトを削除
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::RemoveAllSprite( void )
{
	unsigned int	i, nCount ;
	EObjArray<ESprite>	lstTemp ;
	nCount = m_itaSprite.GetSize( ) ;
	lstTemp.SetLimit( nCount ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<int,ESprite> *	pElement ;
		pElement = m_itaSprite.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		ESprite *	pChild = pElement->GetObject() ;
		if ( pChild != NULL )
		{
			pChild->SetSpriteParent( NULL ) ;
			pElement->DetachObject( ) ;
			lstTemp.Add( pChild ) ;
		}
	}
	m_itaSprite.RemoveAll( ) ;
}

// 外接矩形を取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ESpriteServer::GetRectangle( void )
{
	if ( !IsDynamicSpriteMode() )
	{
		return	EImageSprite::GetRectangle( ) ;
	}
	EGL_RECT	rect = m_rectDynamic ;
	EGL_POINT	ptBasePos = m_eglParam.ptBasePos ;
	if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
	{
		ptBasePos.x >>= 16 ;
		ptBasePos.y >>= 16 ;
	}
	rect.left += ptBasePos.x ;
	rect.top += ptBasePos.y ;
	rect.right += ptBasePos.x ;
	rect.bottom += ptBasePos.y ;
	return	rect ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::GetHiddenRectangle( EGL_RECT & rect )
{
	if ( (m_pImage == NULL)
		|| (m_eglParam.dwFlags & 0xFFFF0000) 
		|| (m_eglParam.pImageAxes != NULL)
		|| (m_eglParam.nTransparency > 0)
		|| !m_visible || IsDynamicSpriteMode() )
	{
		return	false ;
	}
	ESLAssert( m_pImage != NULL ) ;
	if ( !(m_pImage->fdwFormatType & (EIF_WITH_ALPHA | EIF_WITH_CLIPPING))
					&& (m_pImage->fdwFormatType != EIF_GRAY_BITMAP) )
	{
		rect = m_rectHidden ;
		return	true ;
	}
	if ( m_iofOwnerFlag != iofOwnBuffer )
	{
		return	false ;
	}
	ESprite *	pChild =
		m_itaSprite.GetObjectAt( m_itaSprite.GetSize() - 1 ) ;
	if ( pChild == NULL )
	{
		return	false ;
	}
	if ( !pChild->GetHiddenRectangle( rect ) )
	{
		return	false ;
	}
//	rect = LocalToGlobal( rect ) ;
	ESLAssert( m_eglParam.pImageAxes == NULL ) ;
	if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
	{
		rect.left = rect.left + ((m_eglParam.ptBasePos.x + 0xFFFF) >> 16) ;
		rect.top = rect.top + ((m_eglParam.ptBasePos.y + 0xFFFF) >> 16) ;
		rect.right = rect.right + (m_eglParam.ptBasePos.x >> 16) ;
		rect.bottom = rect.bottom + (m_eglParam.ptBasePos.y >> 16) ;
	}
	else
	{
		rect.left += m_eglParam.ptBasePos.x ;
		rect.top += m_eglParam.ptBasePos.y ;
		rect.right += m_eglParam.ptBasePos.x ;
		rect.bottom += m_eglParam.ptBasePos.y ;
	}
	return	true ;
}

// マルチスレッド描画前準備処理
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::BeforeMTDraw( void )
{
	EImageSprite::BeforeMTDraw() ;
	//
	if ( !m_fStereoView || !IsDynamicSpriteMode() )
	{
		Refresh() ;
	}
}
// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	if ( !IsDynamicSpriteMode() )
	{
		//
		// 静的描画
		//
		EImageSprite::MTDraw( hRenderPoly ) ;
	}
	else
	{
		//
		// 動的描画
		//
		DrawDynamicMode( hRenderPoly, NULL ) ;
	}
}

// スプライト描画（立体視表示用）
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::MTDrawTo3DView
	( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo )
{
	if ( !m_fStereoView )
	{
		EImageSprite::MTDrawTo3DView( hRenderPoly, v3dInfo ) ;
		return ;
	}
	if ( !IsDynamicSpriteMode() )
	{
		//
		// 静的立体視描画
		//
		PEGL_IMAGE_INFO	pImage = NULL ;
		if ( v3dInfo.nViewIndex == sviStereoViewRight )
		{
			pImage = m_pImage ;
		}
		else
		{
			pImage = m_imgLeftBuf ;
		}
		if ( pImage != NULL )
		{
			HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
			EGL_DRAW_PARAM	dp = m_eglParam ;
			dp.pSrcImage = pImage ;
			if ( !hDraw->PrepareDraw( &dp ) )
			{
				hDraw->DrawImage( ) ;
			}
		}
	}
	else
	{
		//
		// 動的立体視描画
		//
		DrawDynamicMode( hRenderPoly, &v3dInfo ) ;
	}
}


// スプライト描画（動的モード）
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::DrawDynamicMode
	( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO * pv3dInfo )
{
	EGLPoint	ptBasePos = m_eglParam.ptBasePos ;
	if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
	{
		ptBasePos.x >>= 16 ;
		ptBasePos.y >>= 16 ;
	}
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	EGL_POINT		ptSaveOffset = hDraw->GetDrawingOffset( ) ;
	ptBasePos += ptSaveOffset ;
	hDraw->SetDrawingOffset( ptBasePos ) ;
	//
	if ( (pv3dInfo == NULL) || (pv3dInfo->nViewIndex == 0) )
	{
		m_rectDynamic.left = 0x7FFF ;
		m_rectDynamic.top = 0x7FFF ;
		m_rectDynamic.right = -1 ;
		m_rectDynamic.bottom = -1 ;
	}
	//
	for ( int i = m_itaSprite.GetSize() - 1; i >= 0; i -- )
	{
		ESprite *	pChild = m_itaSprite.GetObjectAt( i ) ;
		ESLAssert( pChild != NULL ) ;
		if ( pChild == NULL )
		{
			continue ;
		}
		if ( pChild->IsVisible() )
		{
			if ( pv3dInfo == NULL )
			{
				pChild->Draw( hRenderPoly ) ;
			}
			else
			{
				pChild->DrawTo3DView( hRenderPoly, *pv3dInfo ) ;
			}
		}
		EGL_RECT	rect ;
		if ( pv3dInfo == NULL )
		{
			rect = pChild->GetRectangle( ) ;
		}
		else
		{
			rect = pChild->GetRectangle3DView( *pv3dInfo ) ;
		}
		if ( rect.left < m_rectDynamic.left )
		{
			m_rectDynamic.left = rect.left ;
		}
		if ( rect.top < m_rectDynamic.top )
		{
			m_rectDynamic.top = rect.top ;
		}
		if ( m_rectDynamic.right < rect.right )
		{
			m_rectDynamic.right = rect.right ;
		}
		if ( m_rectDynamic.bottom < rect.bottom )
		{
			m_rectDynamic.bottom = rect.bottom ;
		}
	}
	hDraw->SetDrawingOffset( ptSaveOffset ) ;
}

// スプライト上の指定領域の更新通知
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::UpdateRect( EGL_RECT * pUpdateRect )
{
	if ( pUpdateRect == NULL )
	{
		m_usStatus = usFull ;
		m_listUpdateRect.RemoveAll( ) ;
		EImageSprite::UpdateRect( NULL ) ;
		return	true ;
	}
	EGL_RECT	rectUpdate = *pUpdateRect ;
	if ( (rectUpdate.left > rectUpdate.right)
		|| (rectUpdate.top > rectUpdate.bottom) )
	{
		return	false ;
	}
	if ( !IsDynamicSpriteMode() && (m_pImage != NULL) )
	{
		if ( m_usStatus == usFull )
		{
			return	EImageSprite::UpdateRect( pUpdateRect ) ;
		}
		if ( (rectUpdate.right < 0)
				|| (rectUpdate.left >= (SDWORD) m_pImage->dwImageWidth)
			|| (rectUpdate.bottom < 0)
				|| (rectUpdate.top >= (SDWORD) m_pImage->dwImageHeight) )
		{
			return	false ;
		}
		if ( rectUpdate.left < 0 )
		{
			rectUpdate.left = 0 ;
		}
		if ( rectUpdate.right >= (SDWORD) m_pImage->dwImageWidth )
		{
			rectUpdate.right = m_pImage->dwImageWidth - 1 ;
		}
		if ( rectUpdate.top < 0 )
		{
			rectUpdate.top = 0 ;
		}
		if ( rectUpdate.bottom >= (SDWORD) m_pImage->dwImageHeight )
		{
			rectUpdate.bottom = m_pImage->dwImageHeight - 1 ;
		}
	}
	else
	{
		if ( rectUpdate.left < m_rectDynamic.left )
		{
			m_rectDynamic.left = rectUpdate.left ;
		}
		if ( rectUpdate.top < m_rectDynamic.top )
		{
			m_rectDynamic.top = rectUpdate.top ;
		}
		if ( m_rectDynamic.right < rectUpdate.right )
		{
			m_rectDynamic.right = rectUpdate.right ;
		}
		if ( m_rectDynamic.bottom < rectUpdate.bottom )
		{
			m_rectDynamic.bottom = rectUpdate.bottom ;
		}
		EImageSprite::UpdateRect( &rectUpdate ) ;
		return	true ;
	}
	int		i = m_listUpdateRect.GetSize() - 1 ;
	while ( i >= 0 )
	{
		EGL_RECT *	pRect = m_listUpdateRect.GetAt( i ) ;
		ESLAssert( pRect != NULL ) ;
		if ( (pRect->left <= rectUpdate.left)
			&& (pRect->top <= rectUpdate.top)
			&& (pRect->right >= rectUpdate.right)
			&& (pRect->bottom >= rectUpdate.bottom) )
		{
			return	false ;
		}
		i -- ;
	}
	for ( ; ; )
	{
		bool	flagMergeRect = false ;
		i = m_listUpdateRect.GetSize() - 1 ;
		while ( i >= 0 )
		{
			EGL_RECT *	pRect = m_listUpdateRect.GetAt( i ) ;
			ESLAssert( pRect != NULL ) ;
			if ( (pRect->left >= rectUpdate.left)
				&& (pRect->top >= rectUpdate.top)
				&& (pRect->right <= rectUpdate.right)
				&& (pRect->bottom <= rectUpdate.bottom) )
			{
				m_listUpdateRect.RemoveAt( i ) ;
			}
			else if ( (((rectUpdate.left <= pRect->left)
						&& (pRect->left <= rectUpdate.right))
					|| ((rectUpdate.left <= pRect->right)
						&& (pRect->right <= rectUpdate.right)))
				&& (((rectUpdate.top <= pRect->top)
						&& (pRect->top <= rectUpdate.bottom))
					|| ((rectUpdate.top <= pRect->bottom)
						&& (pRect->bottom <= rectUpdate.bottom))) )
			{
				if ( rectUpdate.left > pRect->left )
					rectUpdate.left = pRect->left ;
				if ( rectUpdate.top > pRect->top )
					rectUpdate.top = pRect->top ;
				if ( rectUpdate.right < pRect->right )
					rectUpdate.right = pRect->right ;
				if ( rectUpdate.bottom < pRect->bottom )
					rectUpdate.bottom = pRect->bottom ;
				//
				m_listUpdateRect.RemoveAt( i ) ;
				flagMergeRect = true ;
			}
			i -- ;
		}
		if ( !flagMergeRect )
		{
			break ;
		}
	}
	if ( m_pImage != NULL )
	{
		if ( (rectUpdate.left <= 0)
				&& (rectUpdate.right >= (SDWORD) m_pImage->dwImageWidth - 1)
			&& (rectUpdate.top <= 0)
				&& (rectUpdate.bottom >= (SDWORD) m_pImage->dwImageHeight - 1) )
		{
			m_usStatus = usFull ;
			m_listUpdateRect.RemoveAll( ) ;
			EImageSprite::UpdateRect( NULL ) ;
			ESLAssert( pUpdateRect != NULL ) ;
			*pUpdateRect = rectUpdate ;
			return	true ;
		}
	}
	if ( m_listUpdateRect.GetSize() >= 32 )
	{
		i = m_listUpdateRect.GetSize() - 1 ;
		while ( i >= 0 )
		{
			EGL_RECT *	pRect = m_listUpdateRect.GetAt( i -- ) ;
			ESLAssert( pRect != NULL ) ;
			if ( pRect->left < rectUpdate.left )
				rectUpdate.left = pRect->left ;
			if ( pRect->top < rectUpdate.top )
				rectUpdate.top = pRect->top ;
			if ( pRect->right > rectUpdate.right )
				rectUpdate.right = pRect->right ;
			if ( pRect->bottom > rectUpdate.bottom )
				rectUpdate.bottom = pRect->bottom ;
		}
		m_listUpdateRect.RemoveAll( ) ;
	}
	m_usStatus = usMulti ;
	EImageSprite::UpdateRect( &rectUpdate ) ;
	ESLAssert( pUpdateRect != NULL ) ;
	*pUpdateRect = rectUpdate ;
	m_listUpdateRect.Add( new EGL_RECT( rectUpdate ) ) ;
	return	true ;
}

// 子スプライト領域を更新領域に設定
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::UpdateChild( ESprite * pChild, EGL_RECT * pUpdateRect )
{
	if ( m_fStereoView )
	{
		VIEW3D_INFO	v3dInfo	= m_v3dInfo ;
		v3dInfo.nViewIndex = sviStereoViewRight ;
		EGLRect	rect ;
		if ( pUpdateRect == NULL )
		{
			rect = pChild->GetRectangle3DView( v3dInfo ) ;
		}
		else
		{
			rect = *pUpdateRect ;
			pChild->LocalRectToGlocal3DView( rect, &v3dInfo ) ;
		}
		//
		v3dInfo.nViewIndex = sviStereoViewLeft ;
		v3dInfo.xParallax = - v3dInfo.xParallax ;
		if ( pUpdateRect == NULL )
		{
			rect |= pChild->GetRectangle3DView( v3dInfo ) ;
		}
		else
		{
			EGL_RECT	rectTemp = *pUpdateRect ;
			pChild->LocalRectToGlocal3DView( rectTemp, &v3dInfo ) ;
			rect |= rectTemp ;
		}
		return	UpdateRect( &rect ) ;
	}
	return	EImageSprite::UpdateChild( pChild ) ;
}

// 更新領域を再描画
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::Refresh( void )
{
	if ( (m_iofOwnerFlag == iofOwnBuffer) && !IsDynamicSpriteMode() )
	{
		if ( m_usStatus == usFull )
		{
			m_listUpdateRect.RemoveAll() ;
			m_listUpdateRect.Add
				( new EGLRect( 0, 0,
					m_pImage->dwImageWidth-1, m_pImage->dwImageHeight-1 ) ) ;
		}
		//
		// スプライト描画順更新
		//
		int	i, nCount, nLimit ;
		nCount = m_itaSprite.GetSize( ) ;
		m_lstViewSprite.SetSize( nCount, 0 ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			m_lstViewSprite.SetAt( i, m_itaSprite.GetObjectAt( i ) ) ;
		}
		for ( i = 0; i < nCount; i ++ )
		{
			ESprite *	pSprite = m_lstViewSprite.GetAt( i ) ;
			ESLAssert( pSprite != NULL ) ;
			if ( pSprite->Is3DSprite() )
			{
				REAL32	zPos = pSprite->GetZPosition() ;
				//
				int	j, k = i ;
				for ( j = i + 1; j < nCount; j ++ )
				{
					ESprite *	pCompare = m_lstViewSprite.GetAt( j ) ;
					ESLAssert( pCompare != NULL ) ;
					if ( pCompare->Is3DSprite() )
					{
						if ( pCompare->GetZPosition() < zPos )
						{
							k = j ;
						}
					}
					else
					{
						break ;
					}
				}
				m_lstViewSprite.Swap( i, k ) ;
			}
		}
		//
		// 陰領域の取得
		//
		nCount = m_lstViewSprite.GetSize( ) ;
		nLimit = nCount - HideTestLimit ;
		if ( nLimit < 0 )
		{
			nLimit = 0 ;
		}
		for ( i = nCount - 1; i >= 0; i -- )
		{
			ESprite *	pSprite = m_lstViewSprite.GetAt( i ) ;
			EGL_RECT	rectHide ;
			ESLAssert( pSprite != NULL ) ;
			if ( pSprite->GetHiddenRectangle( rectHide ) )
			{
				m_listHideRect[i] = rectHide ;
			}
			else
			{
				m_listHideRect.SetAt( i, NULL ) ;
			}
		}
		while ( i >= 0 )
		{
			m_listHideRect.SetAt( i --, NULL ) ;
		}
		//
		// マルチスレッド用準備処理
		//
		if ( m_ispParam.dwFlags & flagMultiThreading )
		{
			BeforeMTDrawProc::PARAM	params[8] ;
			void*	pInstance[8] =
			{
				&params[0], &params[1], &params[2], &params[3],
				&params[4], &params[5], &params[6], &params[7],
			} ;
			size_t countThread = SSystem::g_cpuLogicalCount ;
			if ( countThread > 8 )
			{
				countThread = 8 ;
			}
			BeforeMTDrawProc	bmtdProc( this ) ;
			bmtdProc.Start( &pInstance[0], countThread ) ;
		}
		//
		// 各更新領域の描画
		//
		nCount = m_listUpdateRect.GetSize( ) ;
		/*
		if ( (m_lstViewSprite.GetSize() == 0) && !m_fFillBack )
		{
			FlushUpdatedRect( ) ;
			return ;
		}
		*/
		for ( i = 0; i < nCount; i ++ )
		{
			EGL_RECT *	pRect = m_listUpdateRect.GetAt( i ) ;
			ESLAssert( pRect != NULL ) ;
			if ( pRect != NULL )
			{
				RefreshRect( *pRect ) ;
			}
		}
	}
	//
	FlushUpdatedRect( ) ;
}

// HEGL_RENDER_POLYGON 生成
//////////////////////////////////////////////////////////////////////////////
HEGL_RENDER_POLYGON ESpriteServer::GetRenderPolygon( void )
{
	if ( m_hRenderPoly == NULL )
	{
		m_hRenderPoly = ::eglCreateRenderPolygon( ) ;
	}
	return	m_hRenderPoly ;
}

// 領域再描画
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::RefreshRect( const EGL_RECT & rectRefresh )
{
	if ( m_fStereoView && (m_imgLeftBuf.GetInfo() != NULL) )
	{
		VIEW3D_INFO	v3dInfo = m_v3dInfo ;
		v3dInfo.nViewIndex = sviStereoViewRight ;
		RefreshRectWith3DView( m_pImage, rectRefresh, &v3dInfo ) ;
		//
		v3dInfo.nViewIndex = sviStereoViewLeft ;
		v3dInfo.xParallax = - v3dInfo.xParallax ;
		RefreshRectWith3DView( m_imgLeftBuf, rectRefresh, &v3dInfo ) ;
	}
	else
	{
		RefreshRectWith3DView( m_pImage, rectRefresh, NULL ) ;
	}
}

void ESpriteServer::RefreshRectWith3DView
	( PEGL_IMAGE_INFO pImage,
		const EGL_RECT & rectRefresh, const VIEW3D_INFO * pv3dInfo )
{
	if ( pImage == NULL )
	{
		return ;
	}
	int	wRect = rectRefresh.right - rectRefresh.left + 1 ;
	int	hRect = rectRefresh.bottom - rectRefresh.top + 1 ;
	if ( (wRect * hRect >= 0x10000)
		&& (m_ispParam.dwFlags & flagMultiThreading) )
	{
		MTDrawProc::PARAM	params[countMaxParallelThreads] ;
		void*	pInstance[countMaxParallelThreads] ;
		size_t countThread = SSystem::g_cpuLogicalCount ;
		if ( countThread > countMaxParallelThreads )
		{
			countThread = countMaxParallelThreads ;
		}
		for ( size_t i = 0; i < countThread; i ++ )
		{
			if ( m_hParallelRender[i] == NULL )
			{
				m_hParallelRender[i] = ::eglCreateRenderPolygon( ) ;
			}
			pInstance[i] = &params[i] ;
			params[i].hRender = m_hParallelRender[i] ;
		}
		int hBlock = (int) ((hRect + countThread - 1) / countThread) ;
		if ( hBlock == 0 )
		{
			hBlock = 1 ;
		}
		MTDrawProc	mtdProc( this, pImage, rectRefresh, pv3dInfo, hBlock ) ;
		mtdProc.Start( &pInstance[0], countThread ) ;
	}
	else
	{
		if ( m_hRenderPoly == NULL )
		{
			m_hRenderPoly = ::eglCreateRenderPolygon( ) ;
		}
		RefreshRectWith3DViewST
			( m_hRenderPoly, pImage, rectRefresh, pv3dInfo, false ) ;
	}
	AfterRefreshRectWith3DView( m_hRenderPoly, pv3dInfo ) ;
}

void ESpriteServer::RefreshRectWith3DViewST
	( HEGL_RENDER_POLYGON hRender, PEGL_IMAGE_INFO pImage,
		const EGL_RECT & rectRefresh,
		const VIEW3D_INFO * pv3dInfo, bool flagOnMultiThread )
{
	//
	// 情報整理
	//
	int			i, j, nSprites, nLimit ;
	ESprite *	pSprite ;
	EGL_RECT *	pRectHide ;
	nSprites = m_lstViewSprite.GetSize( ) ;
	nLimit = nSprites - HideTestLimit ;
	if ( nLimit < 0 )
	{
		nLimit = 0 ;
	}
	//
	// 描画オブジェクト準備
	//
	HEGL_DRAW_IMAGE	hDrawImage = hRender->GetDrawImage( ) ;
	//
	// ｚバッファ初期化
	//
	if ( m_imgZBuf.GetInfo() && m_fFillBack )
	{
		EGLPalette	rZInitValue( 0x7F000000UL ) ;
		hRender->Initialize
			( m_imgZBuf, &rectRefresh, NULL, &m_vScreen ) ;
		if ( !hDrawImage->PrepareFillRect( &rectRefresh, rZInitValue, 0, 0 ) )
		{
			hDrawImage->FillRegion( ) ;
		}
	}
	//
	// 描画準備
	//
	hRender->Initialize
		( pImage, &rectRefresh, m_imgZBuf, &m_vScreen ) ;
	hRender->SetFunctionFlags( m_dwRenderFlags ) ;
	hDrawImage->SetFunctionFlags( m_dwDrawFlags ) ;
	//
	// 背景色塗りつぶし
	//
	if ( m_fFillBack )
	{
		bool	fFillBack = true ;
		if ( pv3dInfo == NULL )
		{
			for ( i = nLimit; i < nSprites; i ++ )
			{
				pRectHide = m_listHideRect.GetAt( i ) ;
				if ( pRectHide != NULL )
				{
					if ( (pRectHide->left <= rectRefresh.left)
						& (pRectHide->right >= rectRefresh.right)
						& (pRectHide->top <= rectRefresh.top)
						& (pRectHide->bottom >= rectRefresh.bottom) )
					{
						nSprites = i + 1 ;
						fFillBack = false ;
						break ;
					}
				}
			}
		}
		if ( fFillBack )
		{
			if ( !hDrawImage->PrepareFillRect
					( &rectRefresh, m_rgbBackColor, 0, 0 ) )
			{
				hDrawImage->FillRegion( ) ;
			}
		}
	}
	//
	// スプライト描画
	//
	for ( i = nSprites - 1; i >= 0; i -- )
	{
		pSprite = m_lstViewSprite.GetAt( i ) ;
		ESLAssert( pSprite != NULL ) ;
		if ( pSprite == NULL )
		{
			continue ;
		}
		if ( pSprite->IsVisibleMT() )
		{
			if ( flagOnMultiThread )
			{
				if ( pv3dInfo != NULL )
				{
					pSprite->MTDrawTo3DView( hRender, *pv3dInfo ) ;
				}
				else
				{
					pSprite->MTDraw( hRender ) ;
				}
			}
			else
			{
				bool	fHide = false ;
				if ( (i >= nLimit) && (pv3dInfo == NULL) )
				{
					EGL_RECT	rectSprite = pSprite->GetRectangle() ;
					if ( rectSprite.left < rectRefresh.left )
					{
						rectSprite.left = rectRefresh.left ;
					}
					if ( rectSprite.right > rectRefresh.right )
					{
						rectSprite.right = rectRefresh.right ;
					}
					if ( rectSprite.top < rectRefresh.top )
					{
						rectSprite.top = rectRefresh.top ;
					}
					if ( rectSprite.bottom > rectRefresh.bottom )
					{
						rectSprite.bottom = rectRefresh.bottom ;
					}
					for ( j = i - 1; j >= nLimit; j -- )
					{
						pRectHide = m_listHideRect.GetAt( j ) ;
						if ( pRectHide != NULL )
						{
							if ( (pRectHide->left <= rectSprite.left)
								& (pRectHide->right >= rectSprite.right)
								& (pRectHide->top <= rectSprite.top)
								& (pRectHide->bottom >= rectSprite.bottom) )
							{
								fHide = true ;
								break ;
							}
						}
					}
				}
				if ( !fHide )
				{
					if ( pv3dInfo != NULL )
					{
						pSprite->DrawTo3DView( hRender, *pv3dInfo ) ;
					}
					else
					{
						pSprite->Draw( hRender ) ;
					}
				}
			}
		}
	}
}

void ESpriteServer::AfterRefreshRectWith3DView
	( HEGL_RENDER_POLYGON hRender, const VIEW3D_INFO * pv3dInfo )
{
}

// マルチスレッド準備処理
//////////////////////////////////////////////////////////////////////////////

// ループ処理／終了判定関数
bool ESpriteServer::BeforeMTDrawProc::Continue( void * pInstance )
{
	if ( m_iNext >= m_pServer->m_lstViewSprite.GetSize() )
	{
		return	false ;
	}
	PARAM *	pParam = (PARAM*) pInstance ;
	pParam->pChild = m_pServer->m_lstViewSprite.GetAt( m_iNext ++ ) ;
	return	true ;
}

// 並列処理関数
void ESpriteServer::BeforeMTDrawProc::RunParallel( void * pInstance )
{
	PARAM *	pParam = (PARAM*) pInstance ;
	pParam->pChild->BeforeMTDraw() ;
}


// マルチスレッド描画準備処理
//////////////////////////////////////////////////////////////////////////////

// ループ処理／終了判定関数
bool ESpriteServer::MTDrawProc::Continue( void * pInstance )
{
	if ( m_yNext > m_rectTarget.bottom )
	{
		return	false ;
	}
	PARAM *	pParam = (PARAM*) pInstance ;
	pParam->rectBlock.left = m_rectTarget.left ;
	pParam->rectBlock.right = m_rectTarget.right ;
	pParam->rectBlock.top = m_yNext ;
	m_yNext += m_hBlock ;
	if ( m_yNext - 1 > m_rectTarget.bottom )
	{
		pParam->rectBlock.bottom = m_rectTarget.bottom ;
	}
	else
	{
		pParam->rectBlock.bottom = m_yNext - 1 ;
	}
	return	true ;
}

// 並列処理関数
void ESpriteServer::MTDrawProc::RunParallel( void * pInstance )
{
	PARAM *	pParam = (PARAM*) pInstance ;
	m_pServer->RefreshRectWith3DViewST
		( pParam->hRender, m_pImage, pParam->rectBlock, m_pv3dInfo, true ) ;
}


// 更新領域を削除
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::FlushUpdatedRect( void )
{
	m_listUpdateRect.RemoveAll( ) ;
	m_usStatus = usEmpty ;
}

// 子スプライトのプライオリティ変更
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::ChangedChildPriority( ESprite * pChild, int nPriority )
{
	if ( pChild->GetPriority() != nPriority )
	{
		int	iChild = GetSpriteIndex( pChild ) ;
		if ( iChild >= 0 )
		{
			int		i, j = iChild ;
			ETaggedElement<int,ESprite> *	pteChild ;
			ETaggedElement<int,ESprite> *	pteTarget ;
			pteChild = m_itaSprite.GetAt( iChild ) ;
			ESLAssert( (pteChild != NULL)
					&& (pteChild->GetObject() == pChild) ) ;
			//
			if ( pChild->GetPriority() > nPriority )
			{
				for ( i = iChild - 1; i >= 0; i -- )
				{
					pteTarget = m_itaSprite.GetAt( i ) ;
					ESLAssert( pteTarget != NULL ) ;
					if ( pteTarget->Tag() <= nPriority )
					{
						break ;
					}
					m_itaSprite.Swap( i, i + 1 ) ;
					j = i ;
				}
			}
			else
			{
				for ( i = iChild + 1; i < (int) m_itaSprite.GetSize(); i ++ )
				{
					pteTarget = m_itaSprite.GetAt( i ) ;
					ESLAssert( pteTarget != NULL ) ;
					if ( pteTarget->Tag() >= nPriority )
					{
						break ;
					}
					m_itaSprite.Swap( i, i - 1 ) ;
					j = i ;
				}
			}
			//
			pteChild->Tag() = nPriority ;
			pChild->SetSpritePriority( nPriority ) ;
			//
			if ( j != iChild )
			{
				UpdateChild( pChild, NULL ) ;
			}
		}
	}
}

// アニメーションを進める
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteServer::OnAdvanceAnimation( unsigned int nTime )
{
	unsigned int	i, nCount ;
	nCount = GetSpriteCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ESprite *	pSprite = GetSpriteAt( i ) ;
		if ( pSprite != NULL )
		{
			pSprite->OnAdvanceAnimation( nTime ) ;
		}
	}
	return	eslErrSuccess ;
}

// 動的スプライトモード設定
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::EnableDynamicMode( bool fDynamicMode )
{
	bool	fOldDynamic = m_fEnableDynamicMode ;
	UpdateRect( NULL ) ;
	m_fEnableDynamicMode = fDynamicMode ;
	UpdateRect( NULL ) ;
	return	fOldDynamic ;
}

// 動的スプライトモード化取得
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::IsDynamicSpriteMode( void )
{
	return	(m_pImage == NULL)
		|| (m_fEnableDynamicMode
				&& (m_eglParam.nTransparency == 0)
				&& (m_eglParam.pImageAxes == NULL)) ;
}

// 背景色取得
//////////////////////////////////////////////////////////////////////////////
EGL_PALETTE ESpriteServer::GetBackColor( void ) const
{
	return	m_rgbBackColor ;
}

// 背景色設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::SetBackColor( EGL_PALETTE rgbBack, bool fEnableBack )
{
	m_rgbBackColor = rgbBack ;
	m_fFillBack = fEnableBack ;
}

// 背景色は有効か？
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::IsEnabledFillBack( void ) const
{
	return	m_fFillBack ;
}

// 背景色を有効にする
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::EnableFillBack( bool fEnableBack )
{
	m_fFillBack = fEnableBack ;
}

// スクリーン座標を取得
//////////////////////////////////////////////////////////////////////////////
const E3D_VECTOR * ESpriteServer::GetScreen3DPosition( void ) const
{
	return	&m_vScreen ;
}

const E3D_VECTOR & ESpriteServer::GetScreenPosition( void ) const
{
	return	m_vScreen ;
}

// スクリーン座標を設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::SetScreenPosition( const E3D_VECTOR & vScreen )
{
	m_vScreen = vScreen ;
}

// 描画機能フラグを取得する
//////////////////////////////////////////////////////////////////////////////
DWORD ESpriteServer::GetDrawFunctionFlags( void ) const
{
	return	m_dwDrawFlags ;
}

DWORD ESpriteServer::GetRenderFunctionFlags( void ) const
{
	return	m_dwRenderFlags ;
}

// 描画機能フラグを設定する
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::SetDrawFunctionFlags( DWORD dwFlags )
{
	m_dwDrawFlags = dwFlags ;
}

void ESpriteServer::SetRenderFunctionFlags( DWORD dwFlags )
{
	m_dwRenderFlags = dwFlags ;
}

// Z バッファを作成
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::CreateZBuffer( void )
{
	ESLAssert( m_pImage != NULL ) ;
	if ( m_pImage != NULL )
	{
		m_imgZBuf.CreateImage
			( EIF_Z_BUFFER_R4, m_pImage->dwImageWidth,
								m_pImage->dwImageHeight, 32, 0 ) ;
	}
}

// Z バッファを削除
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::DeleteZBuffer( void )
{
	m_imgZBuf.DeleteImage( ) ;
}

// Z バッファを取得
//////////////////////////////////////////////////////////////////////////////
EGLImage & ESpriteServer::GetZBuffer( void )
{
	return	m_imgZBuf ;
}

// ステレオバッファ（左目視点）を作成
//////////////////////////////////////////////////////////////////////////////
ESLError ESpriteServer::CreateStereoBuffer( const VIEW3D_INFO & v3dInfo )
{
	ESLAssert( m_pImage != NULL ) ;
	if ( m_pImage != NULL )
	{
		if ( m_imgLeftBuf.CreateImage
			( m_pImage->fdwFormatType,
				m_pImage->dwImageWidth,
				m_pImage->dwImageHeight, m_pImage->dwBitsPerPixel, 0 ) )
		{
			m_imgLeftBuf.ReverseVertically() ;
			m_fStereoView = true ;
			m_v3dInfo = v3dInfo ;
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}

// ステレオ立体視パラメータを設定
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::SetStereoViewInfo( const VIEW3D_INFO & v3dInfo )
{
	m_v3dInfo = v3dInfo ;
}

// ステレオ立体視パラメータを取得
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::GetStereoViewInfo( VIEW3D_INFO & v3dInfo )
{
	v3dInfo = m_v3dInfo ;
}

// ステレオバッファを削除
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::DeleteStereoBuffer( void )
{
	m_fStereoView = false ;
	m_imgLeftBuf.DeleteImage() ;
}

// ステレオバッファ（左目視点）を取得
//////////////////////////////////////////////////////////////////////////////
EGLImage & ESpriteServer::GetStereoLeftBuffer( void )
{
	return	m_imgLeftBuf ;
}

// カメラ（3Dスプライト用）を設定する
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::Set3DViewCamera
	( const E3D_VECTOR vViewPos,
		const E3D_VECTOR & vTargetPos, double rRevAngleZ )
{
	m_vCameraPos = vViewPos ;
	m_vTargetPos = vTargetPos ;
	m_zRevAngle = rRevAngleZ ;
	m_fCamera = true ;
	//
	UpdateRect( NULL ) ;
}

// カメラを有効・無効化する
//////////////////////////////////////////////////////////////////////////////
void ESpriteServer::Enable3DViewCamera( bool fCamera )
{
	m_fCamera = fCamera ;
	UpdateRect( NULL ) ;
}

// カメラを取得する
//////////////////////////////////////////////////////////////////////////////
bool ESpriteServer::Get3DViewCamera
	( E3D_VECTOR & vViewPos,
		E3D_VECTOR & vTargetPos, double & rRevAngleZ )
{
	if ( m_fCamera )
	{
		vViewPos = m_vCameraPos ;
		vTargetPos = m_vTargetPos ;
		rRevAngleZ = m_zRevAngle ;
		return	true ;
	}
	return	EImageSprite::Get3DViewCamera
				( vViewPos, vTargetPos, rRevAngleZ ) ;
}
