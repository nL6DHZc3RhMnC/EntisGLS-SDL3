
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2004-2015 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <math.h>
#include <dshow.h>
#include <dsound.h>

#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mei_media_player.h>


//////////////////////////////////////////////////////////////////////////////
// パーティクル用スプライト
//////////////////////////////////////////////////////////////////////////////

ECSStrTagArray *	ECSParticleSprite::m_staFuncName = NULL ;
const wchar_t *	ECSParticleSprite::m_pwszFuncName[8] =
{
	L"SetParticleImageLimit",
	L"SetParticleImage",
	L"SetParticleParameter",
	L"SetParticleGeneratorMask",
	L"SetParticleRectangle",
	L"CreateParticle",
	L"SetParticleGenerator",
	NULL
} ;
const ECSParticleSprite::PFUNC_CALL	ECSParticleSprite::m_pfnCallFunc[7] =
{
	&ECSParticleSprite::Call_SetParticleImageLimit,
	&ECSParticleSprite::Call_SetParticleImage,
	&ECSParticleSprite::Call_SetParticleParameter,
	&ECSParticleSprite::Call_SetParticleGeneratorMask,
	&ECSParticleSprite::Call_SetParticleRectangle,
	&ECSParticleSprite::Call_CreateParticle,
	&ECSParticleSprite::Call_SetParticleGenerator,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSParticleSprite, ECSSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSParticleSprite::ECSParticleSprite( void )
{
	m_nGenCount = 0 ;
	m_dwRandom = ::timeGetTime( ) ;
	SetParticleImageLimit( 1 ) ;
	m_pGeneratorMask = NULL ;
	m_rctValidated.left = 0 ;
	m_rctValidated.top = 0 ;
	m_rctValidated.right = 639 ;
	m_rctValidated.bottom = 479 ;
	m_rctParticle.left = 0 ;
	m_rctParticle.top = 0 ;
	m_rctParticle.right = -1 ;
	m_rctParticle.bottom = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSParticleSprite::~ECSParticleSprite( void )
{
}

// 外接矩形を取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ECSParticleSprite::GetRectangle( void )
{
	EGL_RECT	rctImage = ECSSprite::GetRectangle( ) ;
	if ( (m_rctParticle.left <= m_rctParticle.right)
		&& (m_rctParticle.top <= m_rctParticle.bottom) )
	{
		if ( rctImage.left > m_rctParticle.left )
		{
			rctImage.left = m_rctParticle.left ;
		}
		if ( rctImage.right < m_rctParticle.right )
		{
			rctImage.right = m_rctParticle.right ;
		}
		if ( rctImage.top > m_rctParticle.top )
		{
			rctImage.top = m_rctParticle.top ;
		}
		if ( rctImage.bottom < m_rctParticle.bottom )
		{
			rctImage.bottom = m_rctParticle.bottom ;
		}
	}
	return	rctImage ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool ECSParticleSprite::GetHiddenRectangle( EGL_RECT & rect )
{
	if ( m_lstParticles.GetSize() > 0 )
	{
		return	false ;
	}
	return	ECSSprite::GetHiddenRectangle( rect ) ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	ECSSprite::MTDraw( hRenderPoly ) ;
	//
	EGL_DRAW_PARAM	dp ;
	EGL_IMAGE_AXES	iax ;
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	EGL_POINT		ptHotspot ;
	int				i, nCount ;
	eslFillMemory( &dp, 0, sizeof(dp) ) ;
	//
	nCount = m_lstParticles.GetSize( ) ;
	//
	for ( i = 0; i < nCount; i ++ )
	{
		PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
		if ( pp == NULL )
		{
			continue ;
		}
		EParticleImage *	ppi = m_lstImages.GetAt( pp->iParticleImage ) ;
		if ( ppi == NULL )
		{
			continue ;
		}
		EGLAnimation *	pParticleImage = ppi->m_pParticleImage ;
		if ( pParticleImage == NULL )
		{
			continue ;
		}
		ptHotspot.x = ppi->m_ptHotspot.x * 0x10000 ;
		ptHotspot.y = ppi->m_ptHotspot.x * 0x10000 ;
		//
		PEGL_IMAGE_INFO	pInfo =
			pParticleImage->GetFrameAt
				( pParticleImage->SequenceToFrame
					( pParticleImage->TimeToSequence( pp->nAnimeTime ) ) ) ;
		if ( pInfo == NULL )
		{
			pInfo = pParticleImage->GetInfo() ;
			if ( pInfo == NULL )
			{
				continue ;
			}
		}
		double	rZoom = pp->rZoom ;
		int		nTransparency = m_ispParam.nTransparency ;
		if ( pp->nPastTime < m_ppParam.nFadein )
		{
			nTransparency =
				m_ppParam.nFadeTransparency
					* (m_ppParam.nFadein - pp->nPastTime) / m_ppParam.nFadein ;
			nTransparency =
				0x100 - (0x100 - nTransparency)
							* (0x100 - m_ispParam.nTransparency) / 0x100 ;
			rZoom = rZoom + (m_ppParam.rFadeZoom - rZoom)
								* pp->nPastTime / m_ppParam.nFadein ;
		}
		else if ( (m_ppParam.nFadeout > 0)
			&& (pp->nPastTime > (m_ppParam.nDuration - m_ppParam.nFadeout)) )
		{
			int	nFadeout =
				pp->nPastTime - (m_ppParam.nDuration - m_ppParam.nFadeout) ;
			nTransparency =
				m_ppParam.nFadeTransparency * nFadeout / m_ppParam.nFadeout ;
			nTransparency =
				0x100 - (0x100 - nTransparency)
							* (0x100 - m_ispParam.nTransparency) / 0x100 ;
			rZoom = rZoom + (m_ppParam.rFadeZoom - rZoom)
								* nFadeout / m_ppParam.nFadeout ;
		}
		//
		dp.ptBasePos.x = eriRoundR32ToInt( pp->vShow.x * 0x10000 ) ;
		dp.ptBasePos.y = eriRoundR32ToInt( pp->vShow.y * 0x10000 ) ;
		eglGetRevolvedAxes
			( &iax, &dp.ptBasePos, &ptHotspot,
				(REAL32) rZoom, (REAL32) rZoom, (REAL32) pp->rRevAngle ) ;
		//
		dp.dwFlags = m_ispParam.dwFlags | EGL_FIXED_POSITION ;
		dp.pSrcImage = pInfo ;
		dp.nTransparency = nTransparency ;
		dp.pImageAxes = &iax ;
		//
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
}

// アニメーション進行
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::OnAdvanceAnimation( unsigned int nTime )
{
	if ( m_dwAnimationFlags & animeEffect )
	{
		int		i, nCount, nAnimeLength = 0 ;
		double	rImageRadius = 1 ;
		EGLRect	rctParticle( 0x7FFF, 0x7FFF, -0x7FFF, -0x7FFF ) ;
		//
		nCount = m_lstParticles.GetSize() ;
		//
		EGL_RECT	rctUpdate = m_rctParticle ;
		ESprite::UpdateRect( &rctUpdate ) ;
		//
		// パーティクル生成
		//
		DWORD	dwPastTime = nTime ;
		if ( dwPastTime > 1000 )
		{
			dwPastTime = 1000 ;
		}
		int	nGenCount = (int) ((INT64) m_nGenCount * dwPastTime / 100000) ;
		int	nGenOdd = (int) ((INT64) m_nGenCount * dwPastTime % 100000) ;
		if ( nGenOdd > Random( 100000 ) )
		{
			nGenCount ++ ;
		}
		CreateParticle( nGenCount ) ;
		//
		// パーティクル移動
		//
		for ( i = 0; i < nCount; i ++ )
		{
			PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
			if ( pp != NULL )
			{
				AdvanceParticlePosition( pp, dwPastTime ) ;
				//
				EGLAnimation *	pParticleImage =
					GetParticleImage( pp->iParticleImage ) ;
				nAnimeLength = 0 ;
				if ( pParticleImage != NULL )
				{
					if ( pParticleImage->GetInfo() != NULL )
					{
						double	w = pParticleImage->GetWidth() ;
						double	h = pParticleImage->GetHeight() ;
						rImageRadius = sqrt( w * w + h * h ) ;
					}
					nAnimeLength = pParticleImage->GetTotalTime( ) ;
				}
				//
				EGL_RECT	rct ;
				double		r = rImageRadius * pp->rZoom ;
				rct.left = (int) eriRoundR64ToLInt( pp->vPos.x - r ) ;
				rct.top = (int) eriRoundR64ToLInt( pp->vPos.y - r ) ;
				rct.right = (int) eriRoundR64ToLInt( pp->vPos.x + r ) ;
				rct.bottom = (int) eriRoundR64ToLInt( pp->vPos.y + r ) ;
				//
				if ( rct.left < rctParticle.left )
				{
					rctParticle.left = rct.left ;
				}
				if ( rct.top < rctParticle.top )
				{
					rctParticle.top = rct.top ;
				}
				if ( rct.right > rctParticle.right )
				{
					rctParticle.right = rct.right ;
				}
				if ( rct.bottom > rctParticle.bottom )
				{
					rctParticle.bottom = rct.bottom ;
				}
				//
				if ( (pp->nPastTime >= m_ppParam.nDuration)
								&& (m_ppParam.nDuration > 0) )
				{
					m_lstParticles.SetAt( i, NULL ) ;
				}
				else if ( nAnimeLength != 0 )
				{
					if ( m_ppParam.nFlags & pfAnimationLoop )
					{
						pp->nAnimeTime %= nAnimeLength ;
					}
					else if ( (int) pp->nAnimeTime >= nAnimeLength )
					{
						m_lstParticles.SetAt( i, NULL ) ;
					}
				}
			}
		}
		m_lstParticles.TrimEmpty( ) ;
		//
		rctParticle.left -= 2 ;
		rctParticle.top -= 2 ;
		rctParticle.right += 2 ;
		rctParticle.bottom += 2 ;
		m_rctParticle = rctParticle ;
		//
		rctUpdate = m_rctParticle ;
		ESprite::UpdateRect( &rctUpdate ) ;
	}
	//
	return	ECSSprite::OnAdvanceAnimation( nTime ) ;
}

// 乱数生成
//////////////////////////////////////////////////////////////////////////////
long int ECSParticleSprite::Random( long int nLimit )
{
	m_dwRandom = m_dwRandom * 5 + 0x9A731651 ;
	if ( nLimit <= 0 )
	{
		return	0 ;
	}
	return	(long int) (m_dwRandom >> 8) % nLimit ;
}

// パーティクルの座標更新
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::AdvanceParticlePosition
	( PARTICLE * pp, int nPastTime ) const
{
	double	rPastSec = nPastTime * 0.001 ;
	pp->nPastTime += nPastTime ;
	pp->nAnimeTime +=
		(int) ((INT64) nPastTime * m_ppParam.nAnimationSpeed / 0x100) ;
	//
	if ( m_ppParam.rShrink != 0 )
	{
		double	rShrink = 1.0 - m_ppParam.rShrink ;
		pp->vVelocity *= pow( rShrink, rPastSec ) ;
	}
	//
	pp->vAcceleration += m_ppParam.vGravity * rPastSec ;
	//
	pp->vPos += pp->vVelocity * rPastSec ;
	pp->vPos += pp->vAcceleration ;
	pp->vPos += m_ppParam.vStream * rPastSec ;
	//
	pp->rRevAngle += pp->rRevSpeed * rPastSec ;
	//
	const double	pi_x2 = 3.1415926535897932384626433832795 * 2 / 1000.0 ;
	double	amp = 0 ;
	for ( int j = 0; j < 2; j ++ )
	{
		if ( (pp->pfFlickness[j].rAmplitude != 0)
			&& (pp->pfFlickness[j].rFrequency != 0) )
		{
			amp += pp->pfFlickness[j].rAmplitude
				* sin( pp->nPastTime * pi_x2 / pp->pfFlickness[j].rFrequency
													+ pp->rFlicknessPhase ) ;
		}
	}
	//
	pp->vShow = pp->vPos + pp->vFlickUnit * amp ;
}

// パーティクル画像を設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::SetParticleImageResource
	( ECSResource * pImage, const EGL_POINT * pHotspot, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	m_lstImages[nIndex].m_refImage.SetReference( pImage ) ;
	m_lstImages[nIndex].m_prsImage = pImage ;
	//
	if ( pImage != NULL )
	{
		return	SetParticleImage( pImage->GetImage(), pHotspot, nIndex ) ;
	}
	else
	{
		return	SetParticleImage( NULL, pHotspot, nIndex ) ;
	}
}

ESLError ECSParticleSprite::SetParticleImage
	( EGLAnimation * pImage, const EGL_POINT * pHotspot, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	m_lstImages[nIndex].m_pParticleImage = pImage ;
	//
	if ( pHotspot != NULL )
	{
		m_lstImages[nIndex].m_ptHotspot = *pHotspot ;
	}
	else if ( pImage != NULL )
	{
		m_lstImages[nIndex].m_ptHotspot.x = pImage->GetWidth() / 2 ;
		m_lstImages[nIndex].m_ptHotspot.y = pImage->GetHeight() / 2 ;
	}
	else
	{
		m_lstImages[nIndex].m_ptHotspot.x = 0 ;
		m_lstImages[nIndex].m_ptHotspot.y = 0 ;
	}
	return	eslErrSuccess ;
}

// パーティクル画像取得
//////////////////////////////////////////////////////////////////////////////
EGLAnimation * ECSParticleSprite::GetParticleImage( int nIndex )
{
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	if ( ppi != NULL )
	{
		return	ppi->m_pParticleImage ;
	}
	return	NULL ;
}

// パーティクル画像の最大数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::SetParticleImageLimit( int nLimit )
{
	m_lstImages.SetSize( nLimit ) ;
	//
	for ( int i = 0; i < nLimit; i ++ )
	{
		if ( m_lstImages.GetAt( i ) == NULL )
		{
			m_lstImages.SetAt( i, new EParticleImage ) ;
		}
	}
}

// パーティクルパラメータ設定
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::SetParticleParameter( const PARTICLE_PARAM & param )
{
	m_ppParam = param ;
}

// 画面有効域設定
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::SetParticleRectangle( const EGL_RECT & rctValidated )
{
	m_rctValidated = rctValidated ;
}

// パーティクル発生領域マスク設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::SetGeneratorAreaMaskResource
	( ECSResource * pMask, const GENERATOR_PARAM * pgp )
{
	m_refGeneratorMask.SetReference( pMask ) ;
	//
	PEGL_IMAGE_INFO	pMaskInf = NULL ;
	EGLAnimation *	pImage = pMask->GetImage( ) ;
	if ( pImage != NULL )
	{
		pMaskInf = pImage->GetInfo( ) ;
	}
	return	SetGeneratorAreaMask( pMaskInf, pgp ) ;
}

ESLError ECSParticleSprite::SetGeneratorAreaMask
	( PEGL_IMAGE_INFO pMask, const GENERATOR_PARAM * pgp )
{
	m_pGeneratorMask = pMask ;
	m_lstGeneratorPos.RemoveAll( ) ;
	//
	if ( pgp != NULL )
	{
		m_gpParam = *pgp ;
		//
		if ( (m_gpParam.nFlags & (gfGenerationPoints | gfRaySide))
							&& (m_gpParam.nGenerationPoints > 0) )
		{
			m_lstGeneratorPos.SetLimit( m_gpParam.nGenerationPoints ) ;
			for ( int i = 0; i < m_gpParam.nGenerationPoints; i ++ )
			{
				E3D_VECTOR_2D	vPos ;
				if ( GenerateParticlePosition( vPos ) != NULL )
				{
					m_lstGeneratorPos.Add( new E3DVector2D( vPos ) ) ;
				}
			}
		}
	}
	else
	{
		m_gpParam.ptMaskCenter.x = 0 ;
		m_gpParam.ptMaskCenter.y = 0 ;
		m_gpParam.szMaskZoom.w = 0x10000 ;
		m_gpParam.szMaskZoom.h = 0x10000 ;
		m_gpParam.szPointStep.w = 0x10000 ;
		m_gpParam.szPointStep.h = 0x10000 ;
		m_gpParam.nFlags = 0 ;
		m_gpParam.nGenerationPoints = 0 ;
	}
	return	eslErrSuccess ;
}

// パーティクル発生領域マスク取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO ECSParticleSprite::GetGeneratorMask( GENERATOR_PARAM * pgp )
{
	if ( pgp != NULL )
	{
		*pgp = m_gpParam ;
	}
	return	m_pGeneratorMask ;
}

// パーティクルを生成する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::CreateParticle( int nCount )
{
	E3DVector2D	vPos ;
	if ( m_ispParam.dwFlags & EGL_FIXED_POSITION )
	{
		vPos.x = (REAL32) ((double) m_ispParam.ptDstPos.x / 0x10000) ;
		vPos.y = (REAL32) ((double) m_ispParam.ptDstPos.y / 0x10000) ;
	}
	else
	{
		vPos.x = (REAL32) m_ispParam.ptDstPos.x ;
		vPos.y = (REAL32) m_ispParam.ptDstPos.y ;
	}
	for ( int i = 0; i < nCount; i ++ )
	{
		PARTICLE *	pp = new PARTICLE ;
		m_lstParticles.Add( pp ) ;
		//
		pp->iParticleImage = Random( m_lstImages.GetSize() ) ;
		//
		pp->nPastTime = 0 ;
		pp->nAnimeTime = 0 ;
		//
		E3D_VECTOR_2D	vParticle ;
		if ( m_lstGeneratorPos.GetSize() > 0 )
		{
			E3D_VECTOR_2D *	pvPos =
				m_lstGeneratorPos.GetAt
					( Random( m_lstGeneratorPos.GetSize() ) ) ;
			if ( pvPos == NULL )
			{
				continue ;
			}
			vParticle = *pvPos ;
		}
		else if ( GenerateParticlePosition( vParticle ) == NULL )
		{
			int	k = m_lstParticles.GetSize() ;
			ESLAssert( m_lstParticles.GetLastAt() == pp ) ;
			m_lstParticles.RemoveAt( k - 1 ) ;
			continue ;
		}
		pp->vPos = vPos + vParticle ;
		pp->vShow = pp->vPos ;
		//
		double	v0 = m_ppParam.rGenVelocity
						+ m_ppParam.rGenVelocityRange * Random(0x1000) / 0x1000 ;
		double	va = m_ppParam.rGenAngle
						+ m_ppParam.rGenAngleRange * Random(0x1000) / 0x1000 ;
		va *= 3.1415926535897932384626433832795 / 180 ;
		//
		pp->vVelocity.x = (REAL32) (v0 * cos( va )) ;
		pp->vVelocity.y = (REAL32) (v0 * sin( va )) ;
		pp->vVelocity +=
			m_ppParam.vGenSpeed
				* (1 + m_ppParam.rGenSpeedRange * Random(0x1000) / 0x1000) ;
		//
		pp->vAcceleration.x = 0 ;
		pp->vAcceleration.y = 0 ;
		//
		pp->rRevAngle = 0 ;
		pp->rRevSpeed = m_ppParam.rRevSpeed
						+ m_ppParam.rRevSpeedRange * Random(0x1000) / 0x1000 ;
		pp->rZoom = m_ppParam.rZoom
						+ m_ppParam.rZoomRange * Random(0x1000) / 0x1000 ;
		//
		for ( int j = 0; j < 2; j ++ )
		{
			pp->pfFlickness[j].rAmplitude =
				m_ppParam.pfFlickness[j].rAmplitude
					+ m_ppParam.pfFlickness[j].rAmplitudeRange
											* Random(0x1000) / 0x1000 ;
			pp->pfFlickness[j].rFrequency =
				m_ppParam.pfFlickness[j].rFrequency
					+ m_ppParam.pfFlickness[j].rFrequencyRange
											* Random(0x1000) / 0x1000 ;
		}
		//
		pp->vFlickUnit.x = pp->vVelocity.y ;
		pp->vFlickUnit.y = - pp->vVelocity.x ;
		double	fa =
			pp->vFlickUnit.x * pp->vFlickUnit.x
				+ pp->vFlickUnit.y * pp->vFlickUnit.y ;
		if ( fa < 1.0e-5 )
		{
			double	r = 3.1415926 * 2.0
						* (double) Random(0x1000) / 0x1000 ;
			pp->vFlickUnit.x = (REAL32) cos(r) ;
			pp->vFlickUnit.y = (REAL32) sin(r) ;
		}
		else
		{
			pp->vFlickUnit *= 1.0 / sqrt( fa ) ;
		}
		pp->rFlicknessPhase =
			6.283185307179586476925286766559
				* ((double) Random(0x1000) / 0x1000) ;
	}
}

// パーティクル発生座標を生成する
//////////////////////////////////////////////////////////////////////////////
E3D_VECTOR_2D *
	ECSParticleSprite::GenerateParticlePosition( E3D_VECTOR_2D & vPos )
{
	if ( (m_gpParam.nFlags & gfRaySide) && (m_pGeneratorMask != NULL) )
	{
		int				nWidth = m_pGeneratorMask->dwImageWidth ;
		int				nHeight = m_pGeneratorMask->dwImageHeight ;
		int				i, nCount = nWidth + nHeight ;
		int				nPosIndex = Random( nCount ) ;
		E3D_VECTOR_2D	vPixel ;
		E3D_VECTOR_2D	vRay = m_gpParam.vRay ;
		double	d = vRay.x * vRay.x + vRay.y * vRay.y ;
		if ( d < 1.0e-5 )
		{
			return	NULL ;
		}
		d = 1.0 / sqrt( d ) ;
		vRay *= d ;
		//
		if ( (nPosIndex < (int) m_pGeneratorMask->dwImageWidth)
									&& (fabs( vRay.y ) > 1.0e-5) )
		{
			vPixel.x = (REAL32) nPosIndex ;
			if ( vRay.y > 0 )
			{
				vPixel.y = 0 ;
			}
			else
			{
				vPixel.y = (REAL32) (m_pGeneratorMask->dwImageHeight - 1) ;
			}
		}
		else if ( fabs( vRay.x ) > 1.0e-5 )
		{
			vPixel.y = (REAL32) (nPosIndex - m_pGeneratorMask->dwImageWidth) ;
			if ( vRay.x > 0 )
			{
				vPixel.x = 0 ;
			}
			else
			{
				vPixel.x = (REAL32) (m_pGeneratorMask->dwImageWidth - 1) ;
			}
		}
		else
		{
			return	NULL ;
		}
		BYTE	bytNegativeMask = 0 ;
		if ( m_gpParam.nFlags & gfNegativeMask )
		{
			bytNegativeMask = 0xFF ;
		}
		//
		for ( i = 0; i < nCount; i ++ )
		{
			int	xp = eriRoundR32ToInt( vPixel.x ) ;
			int	yp = eriRoundR32ToInt( vPixel.y ) ;
			if ( (xp < 0) || (xp >= nWidth) || (yp < 0) || (yp >= nHeight) )
			{
				break ;
			}
			vPixel += vRay ;
			//
			EGL_PALETTE	px = eglGetPixel( m_pGeneratorMask, xp, yp ) ;
			int	nValue = 0x100 ;
			if ( m_pGeneratorMask->fdwFormatType == EIF_GRAY_BITMAP )
			{
				nValue = (px.dwPixelCode ^ bytNegativeMask) & 0xFF ;
			}
			else if ( m_pGeneratorMask->fdwFormatType == EIF_RGBA_BITMAP )
			{
				nValue = px.rgba.Alpha ^ bytNegativeMask ;
			}
			if ( nValue > 0x80 )
			{
				vPos.x =
					(REAL32) ((vPixel.x * m_gpParam.szMaskZoom.w
									+ m_gpParam.ptMaskCenter.x) / 0x10000) ;
				vPos.y =
					(REAL32) ((vPixel.y * m_gpParam.szMaskZoom.h
									+ m_gpParam.ptMaskCenter.y) / 0x10000) ;
				return	&vPos ;
			}
		}
		return	NULL ;
	}
	else if ( m_pGeneratorMask != NULL )
	{
		bool			fFindPosition = false ;
		EGL_SIZE		szSizeByStep = { 1, 1 } ;
		E3D_VECTOR_2D	vBasePos ;
		if ( m_gpParam.szPointStep.w != 0 )
		{
			szSizeByStep.w =
				m_pGeneratorMask->dwImageWidth
					* m_gpParam.szMaskZoom.w
						/ m_gpParam.szPointStep.w + 1 ;
		}
		if ( m_gpParam.szPointStep.h != 0 )
		{
			szSizeByStep.h =
				m_pGeneratorMask->dwImageHeight
					* m_gpParam.szMaskZoom.h
						/ m_gpParam.szPointStep.h + 1 ;
		}
		vBasePos.x =
			(REAL32) ((double) m_gpParam.ptMaskCenter.x
								* m_gpParam.szMaskZoom.w / 0x100000000) ;
		vBasePos.y =
			(REAL32) ((double) m_gpParam.ptMaskCenter.y
								* m_gpParam.szMaskZoom.h / 0x100000000) ;
		//
		BYTE	bytNegativeMask = 0 ;
		if ( m_gpParam.nFlags & gfNegativeMask )
		{
			bytNegativeMask = 0xFF ;
		}
		//
		for ( int j = 0; j < 0x100; j ++ )
		{
			long int	x =
				Random( szSizeByStep.w ) * m_gpParam.szPointStep.w ;
			long int	y =
				Random( szSizeByStep.h ) * m_gpParam.szPointStep.h ;
			long int	xp = x ;
			if ( m_gpParam.szMaskZoom.w != 0 )
			{
				xp /= m_gpParam.szMaskZoom.w ;
			}
			long int	yp = x ;
			if ( m_gpParam.szMaskZoom.h != 0 )
			{
				yp /= m_gpParam.szMaskZoom.h ;
			}
			EGL_PALETTE	px = eglGetPixel( m_pGeneratorMask, xp, yp ) ;
			int	nValue = 0x100 ;
			if ( m_pGeneratorMask->fdwFormatType == EIF_GRAY_BITMAP )
			{
				nValue = (px.dwPixelCode ^ bytNegativeMask) & 0xFF ;
			}
			else if ( m_pGeneratorMask->fdwFormatType == EIF_RGBA_BITMAP )
			{
				nValue = px.rgba.Alpha ^ bytNegativeMask ;
			}
			if ( nValue > Random( 0x100 ) )
			{
				vPos.x = (REAL32) (vBasePos.x + (double) x / 0x1000) ;
				vPos.y = (REAL32) (vBasePos.y + (double) y / 0x1000) ;
				fFindPosition = true ;
				break ;
			}
		}
		if ( !fFindPosition )
		{
			return	NULL ;
		}
	}
	else
	{
		vPos.x =
			(REAL32) (m_ppParam.rGenWidth
							* (Random(0x8000) - 0x4000) / 0x4000) ;
		vPos.y =
			(REAL32) (m_ppParam.rGenHeight
							* (Random(0x8000) - 0x4000) / 0x4000) ;
	}
	return	&vPos ;
}

// パーティクル生成数を設定する（/100sec）
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::SetParticleGenerator( int nCount )
{
	m_nGenCount = nCount ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSParticleSprite::GetTypeName( void ) const
{
	return	L"ParticleSprite" ;
}

ECSObject * ECSParticleSprite::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"ParticleSprite" ) )
	{
		return	this ;
	}
	return	ECSSprite::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSParticleSprite::Duplicate( void )
{
	return	new ECSParticleSprite ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		ESLError	err =
			ECSSprite::GetFunction( context, nIndex, pwszName ) ;
		if ( err )
		{
			return	err ;
		}
		nIndex += m_staFuncName->GetSize( ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( nIndex >= (int) m_staFuncName->GetSize() )
	{
		nIndex -= m_staFuncName->GetSize() ;
		return	ECSSprite::CallFunction( context, nIndex, lstArg ) ;
	}
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていない ParticleSprite 型のメンバ関数を呼び出しています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::IndexAllMember( void )
{
	ECSSprite::IndexAllMember( ) ;
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		m_lstImages[i].m_refImage.IndexAllMember( ) ;
	}
	m_refGeneratorMask.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleSprite::CleanupAllReference( ECSContext & context )
{
	ECSSprite::CleanupAllReference( context ) ;
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		m_lstImages[i].m_refImage.CleanupAllReference( context ) ;
		m_lstImages[i].m_prsImage = NULL ;
		m_lstImages[i].m_pParticleImage = NULL ;
	}
	m_refGeneratorMask.CleanupAllReference( context ) ;
	m_pGeneratorMask = NULL ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::CommitAllReference( ECSContext & context )
{
	ESLError	err ;
	err = ECSSprite::CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		err = m_lstImages[i].m_refImage.CommitAllReference( context ) ;
		if ( err )
			return	err ;
		//
		m_lstImages[i].m_prsImage =
			ESLTypeCast<ECSResource>( m_lstImages[i].m_refImage.m_pRef ) ;
		if ( m_lstImages[i].m_prsImage != NULL )
		{
			SetParticleImage
				( m_lstImages[i].m_prsImage->GetImage(),
							&(m_lstImages[i].m_ptHotspot), i ) ;
		}
		else
		{
			SetParticleImage( NULL, &(m_lstImages[i].m_ptHotspot), i ) ;
		}
	}
	//
	err = m_refGeneratorMask.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	//
	m_pGeneratorMask = NULL ;
	if ( m_refGeneratorMask.m_pRef != NULL )
	{
		ECSResource *	pRsrcMask =
			ESLTypeCast<ECSResource>( m_refGeneratorMask.m_pRef ) ;
		if ( pRsrcMask != NULL )
		{
			EGLAnimation *	pImage = pRsrcMask->GetImage( ) ;
			if ( pImage != NULL )
			{
				m_pGeneratorMask = pImage->GetInfo( ) ;
			}
		}
	}
	//
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	int	i ;
	int	nImageCount = m_lstImages.GetSize( ) ;
	file.Write( &nImageCount, sizeof(nImageCount) ) ;
	for ( i = 0; i < nImageCount; i ++ )
	{
		err = m_lstImages[i].m_refImage.Save( file, context ) ;
		if ( err )
		{
			return	err ;
		}
		file.Write( &(m_lstImages[i].m_ptHotspot), sizeof(EGL_POINT) ) ;
	}
	//
	err = m_refGeneratorMask.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_gpParam, sizeof(m_gpParam) ) ;
	//
	file.Write( &m_nGenCount, sizeof(m_nGenCount) ) ;
	file.Write( &m_ppParam, sizeof(m_ppParam) ) ;
	file.Write( &m_rctValidated, sizeof(m_rctValidated) ) ;
	file.Write( &m_rctParticle, sizeof(m_rctParticle) ) ;
	//
	int	nParticles = m_lstParticles.GetSize() ;
	file.Write( &nParticles, sizeof(nParticles) ) ;
	for ( i = 0; i < nParticles; i ++ )
	{
		PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
		ESLAssert( pp != NULL ) ;
		file.Write( pp, sizeof(PARTICLE) ) ;
	}
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	int	i ;
	int	nImageCount ;
	file.Read( &nImageCount, sizeof(nImageCount) ) ;
	SetParticleImageLimit( nImageCount ) ;
	for ( i = 0; i < nImageCount; i ++ )
	{
		err = m_lstImages[i].m_refImage.Load( file, context ) ;
		if ( err )
		{
			return	err ;
		}
		file.Read( &(m_lstImages[i].m_ptHotspot), sizeof(EGL_POINT) ) ;
	}
	//
	m_pGeneratorMask = NULL ;
	err = m_refGeneratorMask.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_gpParam, sizeof(m_gpParam) ) ;
	//
	file.Read( &m_nGenCount, sizeof(m_nGenCount) ) ;
	file.Read( &m_ppParam, sizeof(m_ppParam) ) ;
	file.Read( &m_rctValidated, sizeof(m_rctValidated) ) ;
	file.Read( &m_rctParticle, sizeof(m_rctParticle) ) ;
	//
	int	nParticles ;
	file.Read( &nParticles, sizeof(nParticles) ) ;
	m_lstParticles.RemoveAll( ) ;
	for ( i = 0; i < nParticles; i ++ )
	{
		PARTICLE *	pp = new PARTICLE ;
		file.Read( pp, sizeof(PARTICLE) ) ;
		m_lstParticles.Add( pp ) ;
	}
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	ECSSprite::DumpObject( buf, nIndent, context ) ;
}

// メンバ関数 : SetParticleImageLimit( Integer nLimit )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleImageLimit
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nLimit ;
	err = context.GetArgumentAsInt( nLimit, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetParticleImageLimit( nLimit ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( context.new_CSInteger( 0 ) ) ;
}

// メンバ関数 : SetParticleImage
//		( Resource rsImage [, Point ptHotspot [, Integer nIndex]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
		return	err ;
	//
	EGL_POINT *	pptHotspot = NULL ;
	EGL_POINT	ptHotspot ;
	int			nIndex ;
	ECSResource *	prsImage =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	ECSStructureInterface *	pstptHotspot =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Point" ) ) ;
	if ( pstptHotspot != NULL )
	{
		ptHotspot.x = pstptHotspot->GetMemberAsInt( L"x", 0 ) ;
		ptHotspot.y = pstptHotspot->GetMemberAsInt( L"y", 0 ) ;
		pptHotspot = &ptHotspot ;
	}
	err = context.GetArgumentAsInt( nIndex, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	QuickLock( ) ;
	err = SetParticleImageResource( prsImage, pptHotspot, nIndex ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleParameter( ParticleParam param )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleParameter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pcpp =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"ParticleParam" ) ) ;
	if ( pcpp != NULL )
	{
		PARTICLE_PARAM	pp ;
		::eslFillMemory( &pp, 0, sizeof(pp) ) ;
		//
		pp.nFlags = pcpp->GetMemberAsInt( L"nFlags", 0 ) ;
		pp.nDuration = pcpp->GetMemberAsInt( L"nDuration", 0 ) ;
		pp.nAnimationSpeed = pcpp->GetMemberAsInt( L"nAnimationSpeed", 0 ) ;
		pp.nFadein = pcpp->GetMemberAsInt( L"nFadein", 0 ) ;
		pp.nFadeout = pcpp->GetMemberAsInt( L"nFadeout", 0 ) ;
		pp.nFadeTransparency = pcpp->GetMemberAsInt( L"nFadeTransparency", 0 ) ;
		pp.rFadeZoom = pcpp->GetMemberAsReal( L"rFadeZoom", 0 ) ;
		pp.rGenWidth = pcpp->GetMemberAsReal( L"rGenWidth", 0 ) ;
		pp.rGenHeight = pcpp->GetMemberAsReal( L"rGenHeight", 0 ) ;
		pp.rGenAngle = pcpp->GetMemberAsReal( L"rGenAngle", 0 ) ;
		pp.rGenAngleRange = pcpp->GetMemberAsReal( L"rGenAngleRange", 0 ) ;
		pp.rGenVelocity = pcpp->GetMemberAsReal( L"rGenVelocity", 0 ) ;
		pp.rGenVelocityRange = pcpp->GetMemberAsReal( L"rGenVelocityRange", 0 ) ;
		pp.rShrink = pcpp->GetMemberAsReal( L"rShrink", 0 ) ;
		pp.rRevSpeed = pcpp->GetMemberAsReal( L"rRevSpeed", 0 ) ;
		pp.rRevSpeedRange = pcpp->GetMemberAsReal( L"rRevSpeedRange", 0 ) ;
		pp.rZoom = pcpp->GetMemberAsReal( L"rZoom", 0 ) ;
		pp.rZoomRange = pcpp->GetMemberAsReal( L"rZoomRange", 0 ) ;
		//
		ECSArray *	pfFlickness2 =
			ESLTypeCast<ECSArray>( pcpp->GetMemberAs( L"pfFlickness" ) ) ;
		if ( pfFlickness2 != NULL )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				ECSStructureInterface *	pcpf =
					ESLTypeCast<ECSStructureInterface>( pfFlickness2->m_varArray.GetAt(i) ) ;
				if ( pcpf != NULL )
				{
					pp.pfFlickness[i].rAmplitude =
						pcpf->GetMemberAsReal( L"rAmplitude", 0 ) ;
					pp.pfFlickness[i].rAmplitudeRange =
						pcpf->GetMemberAsReal( L"rAmplitudeRange", 0 ) ;
					pp.pfFlickness[i].rFrequency =
						pcpf->GetMemberAsReal( L"rFrequency", 0 ) ;
					pp.pfFlickness[i].rFrequencyRange =
						pcpf->GetMemberAsReal( L"rFrequencyRange", 0 ) ;
				}
			}
		}
		//
		ECSStructureInterface *	pcv ;
		pcv = ESLTypeCast<ECSStructureInterface>( pcpp->GetMemberAs( L"vGenSpeed" ) ) ;
		if ( pcv != NULL )
		{
			pp.vGenSpeed.x = (REAL32) pcv->GetMemberAsReal( L"x", 0 ) ;
			pp.vGenSpeed.y = (REAL32) pcv->GetMemberAsReal( L"y", 0 ) ;
		}
		pp.rGenSpeedRange = pcpp->GetMemberAsReal( L"rGenSpeedRange", 0 ) ;
		//
		pcv = ESLTypeCast<ECSStructureInterface>( pcpp->GetMemberAs( L"vStream" ) ) ;
		if ( pcv != NULL )
		{
			pp.vStream.x = (REAL32) pcv->GetMemberAsReal( L"x", 0 ) ;
			pp.vStream.y = (REAL32) pcv->GetMemberAsReal( L"y", 0 ) ;
		}
		//
		pcv = ESLTypeCast<ECSStructureInterface>( pcpp->GetMemberAs( L"vGravity" ) ) ;
		if ( pcv != NULL )
		{
			pp.vGravity.x = (REAL32) pcv->GetMemberAsReal( L"x", 0 ) ;
			pp.vGravity.y = (REAL32) pcv->GetMemberAsReal( L"y", 0 ) ;
		}
		//
		QuickLock( ) ;
		SetParticleParameter( pp ) ;
		QuickUnlock( ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleGeneratorMask
//	( Resource rsMask [, Point ptCenter [, Size szZoom [, Size szStep,
//		[, Integer nFlags [, Integer nGenPoints [, Vector2D vRay]]]]]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleGeneratorMask
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 8 ) ;
	if ( err )
		return	err ;
	//
	ECSResource *	pcsMask =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	//
	GENERATOR_PARAM	gp ;
	gp.ptMaskCenter.x = 0 ;
	gp.ptMaskCenter.y = 0 ;
	gp.szMaskZoom.w = 0x10000 ;
	gp.szMaskZoom.h = 0x10000 ;
	gp.szPointStep.w = 0x10000 ;
	gp.szPointStep.h = 0x10000 ;
	gp.nFlags = 0 ;
	gp.nGenerationPoints = 0 ;
	gp.vRay.x = 1 ;
	gp.vRay.y = 0 ;
	//
	ECSStructureInterface *	pcsCenter =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Point" ) ) ;
	if ( pcsCenter != NULL )
	{
		gp.ptMaskCenter.x = pcsCenter->GetMemberAsInt( L"x", 0 ) ;
		gp.ptMaskCenter.y = pcsCenter->GetMemberAsInt( L"y", 0 ) ;
	}
	ECSStructureInterface *	pcsZoom =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 3, L"Size" ) ) ;
	if ( pcsZoom != NULL )
	{
		gp.szMaskZoom.w = pcsZoom->GetMemberAsInt( L"w", 0x10000 ) ;
		gp.szMaskZoom.h = pcsZoom->GetMemberAsInt( L"h", 0x10000 ) ;
	}
	ECSStructureInterface *	pcsStep =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 4, L"Size" ) ) ;
	if ( pcsStep != NULL )
	{
		gp.szPointStep.w = pcsStep->GetMemberAsInt( L"w", 0x10000 ) ;
		gp.szPointStep.h = pcsStep->GetMemberAsInt( L"h", 0x10000 ) ;
	}
	//
	err = context.GetArgumentAsInt( gp.nFlags, lstArg, 5, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( gp.nGenerationPoints, lstArg, 6, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSStructureInterface *	pcsRay =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 7, L"Vector2D" ) ) ;
	if ( pcsRay != NULL )
	{
		gp.vRay.x = (REAL32) pcsRay->GetMemberAsReal( L"x", 1.0 ) ;
		gp.vRay.y = (REAL32) pcsRay->GetMemberAsReal( L"y", 0.0 ) ;
	}
	//
	QuickLock( ) ;
	SetGeneratorAreaMaskResource( pcsMask, &gp ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleRectangle( Rect rctValidated )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleRectangle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	prctValidated =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Rect" ) ) ;
	if ( prctValidated != NULL )
	{
		EGL_RECT	rctValidated ;
		rctValidated.left = prctValidated->GetMemberAsInt( L"left", 0 ) ;
		rctValidated.top = prctValidated->GetMemberAsInt( L"top", 0 ) ;
		rctValidated.right = prctValidated->GetMemberAsInt( L"right", 640 ) ;
		rctValidated.bottom = prctValidated->GetMemberAsInt( L"bottom", 480 ) ;
		//
		QuickLock( ) ;
		SetParticleRectangle( rctValidated ) ;
		QuickUnlock( ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : CreateParticle( Integer nCount )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_CreateParticle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	CreateParticle( nCount ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleGenerator( Integer nCount )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleSprite::Call_SetParticleGenerator
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetParticleGenerator( nCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ムービー再生用スプライト
//////////////////////////////////////////////////////////////////////////////

EGLDrawImage *	ECSMovieSprite::m_pDrawImage = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSMovieSprite, ECSSprite, ERIAnimationPlayer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSMovieSprite::ECSMovieSprite( void )
{
	m_nPlayType = ptfMusic ;
	m_mfsStatus = mfsNotOpened ;
	m_taskPlayer.m_pSprite = this ;
	m_dwMovieFlags = 0 ;
	m_hCancelPlaying = NULL ;
	//
	m_dwLoopStartFrame = 0 ;
	m_dwLoopEndFrame = -1 ;
	//
	m_player = NULL ;
	m_flagInStopPlayer = false ;
	//
	m_pGraphBuilder = NULL ;
	m_pMediaControl = NULL ;
	m_pBasicAudio = NULL ;
	m_pVideoWindow = NULL ;
	m_pMediaPosition = NULL ;
	m_fWindowNullDraw = false ;
	//
	m_pOwnFileObj = NULL ;
	m_dwRestoredStatus = mfsNotOpened ;
	m_dwRestoredFrame = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSMovieSprite::~ECSMovieSprite( void )
{
	CloseMovie( ) ;
	//
	delete	m_player ;
	m_player = NULL ;
}

// 動画ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::OpenMovie( ESLFileObject & file )
{
	CloseMovie( ) ;
	//
	unsigned int nPreloadSize = 0 ;
	if ( ESLThread::GetLogicalProcessorCount() > 1 )
	{
		nPreloadSize = 30 ;
	}
	ESLError	err =
		ERIAnimationPlayer::Open
			( m_pOwnFileObj, GetWaveOutDevice(),
				nPreloadSize, ERISADecoder::dfQualityDecode ) ;
	if ( !err )
	{
		EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
		if ( pWnd != NULL )
		{
			pWnd->Lock( ) ;
		}
		AttachImage( (PEGL_IMAGE_INFO) ERIAnimation::GetImageInfo() ) ;
		if ( pWnd != NULL )
		{
			pWnd->Unlock( ) ;
		}
		m_mfsStatus = mfsOpened ;
	}
	else
	{
		ERawFile *	pfile = ESLTypeCast<ERawFile>( &file ) ;
		if ( pfile != NULL )
		{
			EWideString	wstrFilePath = pfile->GetFilePath() ;
			err = OpenMovieWithDirectShow( wstrFilePath ) ;
		}
	}
	return	err ;
}

ESLError ECSMovieSprite::OpenMovieFile
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	CloseMovie( ) ;
	//
	/*
	if ( SSystem::GetLogicalProcessorCount() >= 4 )
	{
		if ( m_player == NULL )
		{
			m_player = new SakuraGL::SGLMEIMediaPlayer ;
		}
		if ( !m_player->Open
			( pwszFileName, 0,
				(pContext ? pContext->GetEnvironment() : NULL) ) )
		{
			m_player->SetNotificationListener( this ) ;
			GetCurrentFrameOfSGLMeiMediaPlayer() ;
			AttachImage( &m_eiiSGLMeiFrame ) ;
			m_mfsStatus = mfsOpened ;
			m_wstrFileName = pwszFileName ;
			return	eslErrSuccess ;
		}
		delete	m_player ;
		m_player = NULL ;
	}
	*/
	//
	if ( pContext != NULL )
	{
		m_pOwnFileObj = pContext->OpenFileOnScript( pwszFileName ) ;
	}
	if ( m_pOwnFileObj == NULL )
	{
		ERawFile *	pfile = new ERawFile ;
		if ( pfile->Open
			( EString(pwszFileName),
				ESLFileObject::modeRead | ESLFileObject::shareRead ) )
		{
			delete	pfile ;
			return	eslErrGeneral ;
		}
		else
		{
			m_pOwnFileObj = pfile ;
		}
	}
	unsigned int nPreloadSize = 0 ;
	if ( ESLThread::GetLogicalProcessorCount() > 1 )
	{
		nPreloadSize = 30 ;
	}
	ESLError	err =
		ERIAnimationPlayer::Open
			( m_pOwnFileObj, GetWaveOutDevice(),
				nPreloadSize, ERISADecoder::dfQualityDecode ) ;
	if ( !err )
	{
		EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
		if ( pWnd != NULL )
		{
			pWnd->Lock( ) ;
		}
		AttachImage( (PEGL_IMAGE_INFO) ERIAnimation::GetImageInfo() ) ;
		if ( pWnd != NULL )
		{
			pWnd->Unlock( ) ;
		}
		m_mfsStatus = mfsOpened ;
		m_wstrFileName = pwszFileName ;
	}
	else
	{
		ERawFile *	pfile = ESLTypeCast<ERawFile>( m_pOwnFileObj ) ;
		if ( pfile != NULL )
		{
			EWideString	wstrFilePath = pfile->GetFilePath() ;
			delete	m_pOwnFileObj ;
			m_pOwnFileObj = NULL ;
			err = OpenMovieWithDirectShow( wstrFilePath ) ;
		}
	}
	return	err ;
}

ESLError ECSMovieSprite::OpenMovieWithDirectShow
					( const wchar_t * pwszFilePath )
{
	CloseMovie() ;
	//
	CoCreateInstance
		( CLSID_FilterGraph, NULL, CLSCTX_INPROC,
				IID_IGraphBuilder, (void **) &m_pGraphBuilder ) ;
	if ( m_pGraphBuilder == NULL )
	{
		return	eslErrGeneral ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IMediaControl, (void **) &m_pMediaControl ) ;
	if ( m_pMediaControl == NULL )
	{
		return	eslErrGeneral ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IVideoWindow, (void **) &m_pVideoWindow ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicAudio, (void **) &m_pBasicAudio ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaPosition, (void **) &m_pMediaPosition ) ;
	//
	m_wstrFileName = pwszFilePath ;
	HRESULT	hr ;//_pGraphBuilder->RenderFile( pwszFilePath, NULL ) ;
	EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
	if ( pWnd == NULL )
	{
		if ( m_pMainWnd != NULL )
		{
			pWnd = m_pMainWnd->GetWindowInterface() ;
		}
		if ( pWnd == NULL )
		{
			return	eslErrGeneral ;
		}
	}
	if ( pWnd->ProcedureOnWindowThread
		( OpenDShowMovieFileProc, this, &hr, false ) )
	{
		return	eslErrGeneral ;
	}
	if ( hr )
	{
		CloseMovie() ;
		return	eslErrGeneral ;
	}
	m_mfsStatus = mfsOpened ;
	return	eslErrSuccess ;
}

LRESULT __stdcall ECSMovieSprite::OpenDShowMovieFileProc( void * pInstance )
{
	ECSMovieSprite *	pms = (ECSMovieSprite*) pInstance ;
	//
	HRESULT	hr = pms->m_pGraphBuilder->RenderFile
					( pms->m_wstrFileName.m_varStr, NULL ) ;
	return	hr ;
}

// 動画ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::CloseMovie( void )
{
	StopMovie( ) ;
	//
	EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
	if ( pWnd != NULL )
	{
		pWnd->Lock( ) ;
	}
	AttachImage( NULL ) ;
	if ( pWnd != NULL )
	{
		pWnd->Unlock( ) ;
	}
	ERIAnimationPlayer::Close( ) ;
	ECSResource::Release( ) ;
	//
	if ( m_player )
	{
		delete	m_player ;
		m_player = NULL ;
	}
	if ( m_pBasicAudio != NULL )
	{
		m_pBasicAudio->Release() ;
		m_pBasicAudio = NULL ;
	}
	if ( m_pMediaPosition != NULL )
	{
		m_pMediaPosition->Release() ;
		m_pMediaPosition = NULL ;
	}
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->put_Visible(OAFALSE) ;
		m_pVideoWindow->put_Owner(NULL) ;
	}
	if ( m_pMediaControl != NULL )
	{
		m_pMediaControl->Release() ;
		m_pMediaControl = NULL ;
	}
	if ( m_pGraphBuilder != NULL )
	{
		m_pGraphBuilder->Release() ;
		m_pGraphBuilder = NULL ;
	}
	//
	m_wstrFileName.m_varStr.FreeString( ) ;
	if ( m_pOwnFileObj != NULL )
	{
		delete	m_pOwnFileObj ;
		m_pOwnFileObj = NULL ;
	}
	return	eslErrSuccess ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::PlayMovie( DWORD dwFlags, int fPlayType )
{
	if ( m_player != NULL )
	{
		m_dwMovieFlags = dwFlags ;
		if ( dwFlags & mpfLoopPlay )
		{
			m_player->SetLoop
				( true, m_dwLoopStartFrame, m_dwLoopEndFrame ) ;
		}
		m_nPlayType = fPlayType ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
		m_flagInStopPlayer = false ;
		m_player->Play() ;
		m_mfsStatus = mfsPlaying ;
		return	eslErrSuccess ;
	}
	if ( m_pGraphBuilder != NULL )
	{
		return	PlayMovieWithDirectShow() ;
	}
	if ( m_mfsStatus == mfsNotOpened )
	{
		return	eslErrGeneral ;
	}
	if ( m_hCancelPlaying == NULL )
	{
		m_hCancelPlaying = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	}
	m_dwMovieFlags = dwFlags ;
	m_fdwDecFlags &=
		~(ERISADecoder::dfUseLoopFilter | ERISADecoder::dfNoLoopFilter) ;
	if ( m_dwMovieFlags & mpfUseLoopFilter )
	{
		m_fdwDecFlags |= ERISADecoder::dfUseLoopFilter ;
	}
	if ( m_dwMovieFlags & mpfNoLoopFilter )
	{
		m_fdwDecFlags |= ERISADecoder::dfNoLoopFilter ;
	}
	if ( (fPlayType > ptfNothing) && (fPlayType < ptfMax) )
	{
		m_nPlayType = fPlayType ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
	}
	m_taskPlayer.CloseThread( ) ;
	m_fCancelPlaying = false ;
	if ( m_taskPlayer.BeginThread() )
	{
		return	eslErrGeneral ;
	}
	m_mfsStatus = mfsPlaying ;
	return	eslErrSuccess ;
}

ESLError ECSMovieSprite::PlayMovieWithDirectShow( void )
{
	if ( (m_pGraphBuilder == NULL)
		|| (m_pMediaControl == NULL) )
	{
		return	eslErrGeneral ;
	}
	EWindowSpriteInterface *	pWndItf = GetWindowInterface( ) ;
	//
	HRESULT	hr ;
	if ( pWndItf->ProcedureOnWindowThread
		( PlayDShowMovieProc, this, &hr, false ) )
	{
		return	eslErrGeneral ;
	}
	//
	m_mfsStatus = mfsPlaying ;
	return	eslErrSuccess ;
}

LRESULT __stdcall ECSMovieSprite::PlayDShowMovieProc( void * pInstance )
{
	ECSMovieSprite *	pms = (ECSMovieSprite*) pInstance ;
	//
	EWindowSpriteInterface *	pWndItf = pms->GetWindowInterface( ) ;
	if ( pWndItf != NULL )
	{
		pWndItf->AddNullficationDraw() ;
		pms->m_fWindowNullDraw = true ;
		//
		EWindow *	pWnd = pWndItf->GetWindow() ;
		if ( (pWnd != NULL) && (pms->m_pVideoWindow) )
		{
			pms->m_pVideoWindow->put_Owner( (OAHWND) (HWND) *pWnd ) ;
			pms->m_pVideoWindow->put_MessageDrain( (OAHWND) (HWND) *pWnd ) ;
			pms->m_pVideoWindow->put_WindowStyle
				( WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS ) ;
			//
			if ( pWndItf->IsImageStretching() )
			{
				const EGLPoint &
					ptBase = pWndItf->GetImageStretchingBase() ;
				const EGLSize &
					sizeView = pWndItf->GetImageStretchingSize() ;
				pms->m_pVideoWindow->SetWindowPosition
					( ptBase.x, ptBase.y, sizeView.w, sizeView.h ) ;
			}
			else
			{
				RECT	rectClient ;
				pWnd->GetClientRect( &rectClient ) ;
				pms->m_pVideoWindow->SetWindowPosition
					( rectClient.left, rectClient.top,
						rectClient.right - rectClient.left,
						rectClient.bottom - rectClient.top ) ;
			}
			pms->m_pVideoWindow->put_Visible( OATRUE ) ;
		}
	}
	if ( pms->m_pBasicAudio != NULL )
	{
		double	rVolume = m_rTotalVol[ptfMusic] ;
		LONG	lVolume = DSBVOLUME_MIN ;
		lVolume = (LONG) (2000.0 * log10( rVolume )) ;
		if ( lVolume >= DSBVOLUME_MAX )
		{
			lVolume = DSBVOLUME_MAX ;
		}
		else if ( lVolume < DSBVOLUME_MIN )
		{
			lVolume = DSBVOLUME_MIN ;
		}
		pms->m_pBasicAudio->put_Balance( 0 ) ;
		pms->m_pBasicAudio->put_Volume( lVolume ) ;
	}
	pms->m_pMediaControl->Run() ;
	//
	return	0 ;
}

// 再生停止
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::StopMovie( void )
{
	if ( m_player != NULL )
	{
		m_flagInStopPlayer = true ;
		m_player->Stop() ;
		m_flagInStopPlayer = false ;
		//
		if ( m_mfsStatus == mfsPlaying )
		{
			m_mfsStatus = mfsOpened ;
		}
		return	eslErrSuccess ;
	}
	if ( m_pMediaControl != NULL )
	{
		m_pMediaControl->Stop() ;
		//
		if ( m_fWindowNullDraw )
		{
			EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
			if ( pWnd != NULL )
			{
				pWnd->ReleaseNullficationDraw() ;
				pWnd->UpdateRect() ;
			}
			m_fWindowNullDraw = false ;
		}
		return	eslErrSuccess ;
	}
	//
	// 再生を停止する
	//
	if ( m_taskPlayer.Handle() != NULL )
	{
		CancelPlaying( ) ;
		//
		HWND	hWnd = NULL ;
		EWindowSpriteInterface *
				pWndSprite = GetWindowInterface() ;
		if ( pWndSprite != NULL )
		{
			EWindow *	pWnd = pWndSprite->GetWindow() ;
			if ( pWnd != NULL )
			{
				hWnd = *pWnd ;
			}
		}
		while ( ::WaitForSingleObject
				( m_taskPlayer.Handle(), 10 ) == WAIT_TIMEOUT )
		{
			MSG	msg ;
			if ( ::PeekMessage
				( &msg, hWnd, WM_PAINT, WM_PAINT, PM_REMOVE ) )
			{
				::TranslateMessage( &msg ) ;
				::DispatchMessage( &msg ) ;
			}
		}
		m_taskPlayer.CloseThread( ) ;
	}
	EWaveMixingServer *	pWaveDev = GetWaveOutDevice() ;
	if ( pWaveDev != NULL )
	{
		pWaveDev->Stop( this ) ;
	}
	if ( m_hCancelPlaying != NULL )
	{
		::CloseHandle( m_hCancelPlaying ) ;
		m_hCancelPlaying = NULL ;
	}
	//
	// 現在のフレーム画像をスプライトに設定
	//
	EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
	if ( pWnd != NULL )
	{
		pWnd->Lock( ) ;
	}
	AttachImage( (PEGL_IMAGE_INFO) ERIAnimation::GetImageInfo() ) ;
	if ( pWnd != NULL )
	{
		pWnd->Unlock( ) ;
	}
	if ( m_mfsStatus == mfsPlaying )
	{
		m_mfsStatus = mfsOpened ;
	}
	return	eslErrSuccess ;
}

// ループ位置設定
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::SetLoopPosition( DWORD dwStartFrame, SDWORD dwEndFrame )
{
	m_dwLoopStartFrame = dwStartFrame ;
	m_dwLoopEndFrame = dwEndFrame ;
	//
	if ( m_player != NULL )
	{
		if ( dwEndFrame < 0 )
		{
			dwEndFrame = GetAllFrameCount() ;
		}
		m_player->SetLoop
			( ((m_dwMovieFlags & mpfLoopPlay) != 0),
							dwStartFrame, dwEndFrame ) ;
	}
	else if ( m_pMediaPosition != NULL )
	{
	}
	else
	{
		if ( dwEndFrame < 0 )
		{
			dwEndFrame = GetAllFrameCount() ;
		}
		ERIAnimationPlayer::SetPlayEndFrame( dwEndFrame ) ;
	}
}

// 指定のフレームに移動
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::SeekToFrame( unsigned int iFrameIndex )
{
	if ( m_player != NULL )
	{
		m_player->SeekPosition( iFrameIndex ) ;
		return	eslErrSuccess ;
	}
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime = (double) iFrameIndex / 1000.0 ;
		if ( m_pMediaPosition->put_CurrentPosition( rtTime ) == S_OK )
		{
			return	eslErrSuccess ;
		}
		return	eslErrFailed ;
	}
	else
	{
		return	ERIAnimationPlayer::SeekToFrame( iFrameIndex ) ;
	}
}

// アニメーション進行（DirectShow 再生時に表示位置調整）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::OnAdvanceAnimation( unsigned int nTime )
{
	if ( (m_mfsStatus == mfsPlaying) && (m_pVideoWindow != NULL) )
	{
		EWindowSpriteInterface *	pwsiWnd = GetWindowInterface( ) ;
		if ( pwsiWnd != NULL )
		{
			if ( pwsiWnd->IsImageStretching() )
			{
				const EGLPoint &
					ptBase = pwsiWnd->GetImageStretchingBase() ;
				const EGLSize &
					sizeView = pwsiWnd->GetImageStretchingSize() ;
				m_pVideoWindow->SetWindowPosition
					( ptBase.x, ptBase.y, sizeView.w, sizeView.h ) ;
			}
			else
			{
				EWindow *	pWnd = pwsiWnd->GetWindow() ;
				if ( pWnd != NULL )
				{
					RECT	rectClient ;
					pWnd->GetClientRect( &rectClient ) ;
					m_pVideoWindow->SetWindowPosition
						( rectClient.left, rectClient.top,
							rectClient.right - rectClient.left,
							rectClient.bottom - rectClient.top ) ;
				}
			}
			if ( m_dwMovieFlags & mpfLoopPlay )
			{
				unsigned int	iCurrent = CurrentIndex() ;
				if ( (iCurrent >= GetAllFrameCount())
					|| (iCurrent >= (DWORD) m_dwLoopEndFrame) )
				{
					SeekToFrame( (unsigned int) m_dwLoopStartFrame ) ;
					m_pMediaControl->Run() ;
				}
			}
		}
	}
	return	ECSSprite::OnAdvanceAnimation( nTime ) ;
}

// 現在の再生位置を取得する
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSMovieSprite::CurrentIndex( void ) const
{
	if ( m_player != NULL )
	{
		return	(unsigned int) m_player->CurrentIndex() ;
	}
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_CurrentPosition( &rtTime ) == S_OK )
		{
			return	(unsigned int) (rtTime * 1000.0) ;
		}
		return	0 ;
	}
	else
	{
		return	ERIAnimationPlayer::CurrentIndex() ;
	}
}

// 動画全長を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSMovieSprite::GetAllFrameCount( void ) const
{
	if ( m_player != NULL )
	{
		return	(unsigned int) m_player->GetAllFrameCount() ;
	}
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_Duration( &rtTime ) == S_OK )
		{
			return	(unsigned int) (rtTime * 1000.0) ;
		}
		return	0 ;
	}
	else
	{
		return	ERIAnimationPlayer::GetAllFrameCount() ;
	}
}

unsigned int ECSMovieSprite::GetTotalTime( void ) const
{
	if ( m_player != NULL )
	{
		return	(unsigned int) m_player->GetTotalTime() ;
	}
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_Duration( &rtTime ) == S_OK )
		{
			return	(unsigned int) (rtTime * 1000.0) ;
		}
		return	0 ;
	}
	else
	{
		return	ERIAnimationPlayer::GetTotalTime() ;
	}
}

// アニメーション再生を中断する
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::CancelPlaying( void )
{
	if ( m_player != NULL )
	{
		m_player->Stop() ;
	}
	else if ( m_pMediaControl != NULL )
	{
		m_pMediaControl->Stop() ;
	}
	else
	{
		if ( m_hCancelPlaying != NULL )
		{
			::SetEvent( m_hCancelPlaying ) ;
		}
		ERIAnimationPlayer::CancelPlaying( ) ;
	}
}

// アニメーションが再生中か判定する
//////////////////////////////////////////////////////////////////////////////
bool ECSMovieSprite::IsMoviePlaying( void )
{
	if ( m_player != NULL )
	{
		return	m_player->IsPlaying() ;
	}
	else if ( m_pMediaControl != NULL )
	{
		OAFilterState	state ;
		if ( m_pMediaControl->GetState( 1000, &state ) == S_OK )
		{
			if ( state == State_Running )
			{
				return	(CurrentIndex() < GetAllFrameCount()) ;
			}
			return	false ;
		}
		return	true ;
	}
	else if ( m_mfsStatus == mfsPlaying )
	{
		if ( ::WaitForSingleObject
			( m_taskPlayer.Handle(), 0 ) == WAIT_OBJECT_0 )
		{
			m_mfsStatus = mfsOpened ;
			return	false ;
		}
		return	true ;
	}
	return	false ;
}

// 再生ループポイント設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::SetRewindingPortion
	( unsigned int nRewindPos, unsigned int nEndPos, bool fRepeat )
{
	m_dwLoopStartFrame = nRewindPos ;
	m_dwLoopEndFrame = nEndPos ;
	//
	if ( m_player != NULL )
	{
		m_player->SetLoop( fRepeat, nRewindPos, nEndPos ) ;
		return	eslErrSuccess ;
	}
	else if ( m_pMediaPosition != NULL )
	{
		return	eslErrSuccess ;
	}
	else
	{
		ERIAnimationPlayer::SetPlayEndFrame( nEndPos ) ;
		return	ECSSprite::SetRewindingPortion( nRewindPos, nEndPos, fRepeat ) ;
	}
}

// 音量を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::SetVolume( REAL32 rLeftVol, REAL32 rRightVol )
{
	m_rVolume[0] = rLeftVol ;
	m_rVolume[1] = rRightVol ;
	//
	if ( m_player != NULL )
	{
		REAL32	rTotalVol = GetTotalVolume( ptfDevice ) ;
		if ( (m_nPlayType > ptfNothing) && (m_nPlayType < ptfMax) )
		{
			rTotalVol *= m_rTotalVol[m_nPlayType] ;
		}
		REAL32	rVolume[2] =
			{ (REAL32) (rLeftVol * rTotalVol),
				(REAL32) (rRightVol * rTotalVol) } ;
		return	(ESLError) m_player->SetVolume( rVolume, 2 ) ;
	}
	EWaveMixingServer *	pWaveDev = GetWaveOutDevice() ;
	if ( pWaveDev != NULL )
	{
		REAL32	rTotalVol = 1.0 ;
		if ( (m_nPlayType > ptfNothing) && (m_nPlayType < ptfMax) )
		{
			rTotalVol = m_rTotalVol[m_nPlayType] ;
		}
		REAL32	rVolume[2] =
			{ (REAL32) (rLeftVol * rTotalVol),
				(REAL32) (rRightVol * rTotalVol) } ;
		pWaveDev->SetVolume( this, rVolume ) ;
	}
	if ( m_mfsStatus == mfsNotOpened )
	{
		ECSResource::SetVolume( rLeftVol, rRightVol ) ;
	}
	return	eslErrSuccess ;
}

// 再生中か調べる
//////////////////////////////////////////////////////////////////////////////
bool ECSMovieSprite::IsPlaying( void )
{
	if ( m_mfsStatus == mfsNotOpened )
	{
		return	ECSResource::IsPlaying( ) ;
	}
	else
	{
		return	IsMoviePlaying( ) ;
	}
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::OnDrawMovieFrame
	( HWND hwndTarget, int xPos, int yPos,
		const EGL_SIZE * pViewSize,
		EGLDrawImage * pDrawImage,
		PCEGL_IMAGE_INFO pImage, DWORD dwDuringTime )
{
	//
	// 親ウィンドウを捜す
	//
	EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
	if ( (pWnd == NULL) || (pImage == NULL) )
	{
		return	eslErrSuccess ;
	}
	if ( m_dwMovieFlags & mpfDirectDraw )
	{
		//
		// 直接 VRAM に描画する
		//
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		E3DSDisplayPlugin::I3DImageView *
				pivView3D = pWnd->GetAttached3DViewDisplay() ;
		if ( pivView3D != NULL )
		{
			using namespace E3DSDisplayPlugin ;
			ImageBuffer			bufImage[2] ;
			const ImageBuffer *	pbufImages[2] =
			{
				&bufImage[0], NULL
			} ;
			int					nViewCount = 1 ;
			//
			pWnd->ConvertToE3DSDisplayImageBuffer( bufImage[0], pImage ) ;
			//
			if ( pImage->fdwFormatType & EIF_SIDE_BY_SIDE )
			{
				EGL_IMAGE_INFO	infLeft ;
				if ( !::eglGetStereoLeftImageBuffer( pImage, &infLeft ) )
				{
					pWnd->ConvertToE3DSDisplayImageBuffer( bufImage[1], &infLeft ) ;
					nViewCount = 2 ;
				}
			}
			//
			pivView3D->AttachThread() ;
			//
			bool	fSuccessful = false ;
			if ( !pivView3D->DrawBuffer
				( drawDynamic | drawTemporary,
						0, 0, pbufImages, nViewCount ) )
			{
				if ( !pivView3D->PrepareView() )
				{
					if ( !pivView3D->ViewImage( 0 ) )
					{
						fSuccessful = true ;
					}
				}
				else
				{
					ESLTrace( "Failed to I3DImageView::PrepareView\n" ) ;
				}
			}
			else
			{
				ESLTrace( "Failed to I3DImageView::DrawBuffer\n" ) ;
			}
			pivView3D->DetachThread() ;
			//
			DWORD	dwEndTime = ::timeGetTime( ) ;
			DWORD	dwDrawingTime = dwEndTime - dwBeginTime ;
			if ( dwDrawingTime < dwDuringTime )
			{
				return	OnWaitingTime( dwDuringTime - dwDrawingTime ) ;
			}
			if ( fSuccessful )
			{
				return	eslErrSuccess ;
			}
		}
		EWindow *	pTarget = pWnd->GetWindow( ) ;
		if ( pTarget == NULL )
		{
			return	eslErrSuccess ;
		}
		if ( pWnd->Lock( 100 ) )
		{
			return	eslErrSuccess ;
		}
		xPos = m_eglParam.ptBasePos.x ;
		yPos = m_eglParam.ptBasePos.y ;
		if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
		{
			xPos >>= 16 ;
			yPos >>= 16 ;
		}
		EGL_SIZE	sizeDraw ;
		sizeDraw.w = pImage->dwImageWidth ;
		sizeDraw.h = pImage->dwImageHeight ;
		if ( m_eglParam.pImageAxes != NULL )
		{
			sizeDraw.w = (long int) ::eriRoundR64ToLInt
				( sizeDraw.w * m_eglParam.pImageAxes->xAxis.x ) ;
			sizeDraw.h = (long int) ::eriRoundR64ToLInt
				( sizeDraw.h * m_eglParam.pImageAxes->yAxis.y ) ;
		}
		if ( pWnd->IsImageStretching() )
		{
			EGLPoint	ptUpperLeft( xPos, yPos ) ;
			EGLPoint	ptUnderRight( xPos + sizeDraw.w, yPos + sizeDraw.h ) ;
			pWnd->ClientToWindow( ptUpperLeft ) ;
			pWnd->ClientToWindow( ptUnderRight ) ;
			xPos = ptUpperLeft.x ;
			yPos = ptUpperLeft.y ;
			sizeDraw.w = ptUnderRight.x - ptUpperLeft.x ;
			sizeDraw.h = ptUnderRight.y - ptUpperLeft.y ;
		}
		ESLError	err =
			ERIAnimationPlayer::OnDrawMovieFrame
				( *pTarget, xPos, yPos, &sizeDraw,
						pDrawImage, pImage, dwDuringTime ) ;
		pWnd->Unlock() ;
		return	err ;
	}
	else
	{
		//
		// スプライトの表示を更新する
		//
		bool		fUpdate = false ;
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		if ( !pWnd->Lock( 100 ) )
		{
			fUpdate = true ;
			AttachImage( (PEGL_IMAGE_INFO) pImage ) ;
			pWnd->Unlock( ) ;
		}
		if ( fUpdate )
		{
			pWnd->WaitForDonePaint( 10 ) ;
			/*
			EWindow *	pTarget = pWnd->GetWindow( ) ;
			if ( pTarget != NULL )
			{
				pTarget->UpdateWindow( ) ;
			}
			*/
		}
		DWORD	dwEndTime = ::timeGetTime( ) ;
		DWORD	dwDrawingTime = dwEndTime - dwBeginTime ;
		if ( dwDrawingTime < dwDuringTime )
		{
			return	OnWaitingTime( dwDuringTime - dwDrawingTime ) ;
		}
	}
	return	eslErrSuccess ;
}

// SGLMEIMediaPlayer フレーム更新通知
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::OnFrameUpdate
		( SakuraGL::SGLMediaPlayerInterface * player )
{
	EWindowSpriteInterface *	pWnd = GetWindowInterface( ) ;
	if ( pWnd == NULL )
	{
		return ;
	}
	using namespace SakuraGL ;
	while ( pWnd->Lock( 30 ) != eslErrSuccess )
	{
		if ( m_flagInStopPlayer )
		{
			return ;
		}
	}
	//
	GetCurrentFrameOfSGLMeiMediaPlayer() ;
	//
	if ( m_dwMovieFlags & mpfDirectDraw )
	{
		//
		// 直接 VRAM に描画する
		//
		E3DSDisplayPlugin::I3DImageView *
				pivView3D = pWnd->GetAttached3DViewDisplay() ;
		if ( pivView3D != NULL )
		{
			//
			// ステレオ立体視出力
			//
			using namespace E3DSDisplayPlugin ;
			ImageBuffer			bufImage[2] ;
			const ImageBuffer *	pbufImages[2] =
			{
				&bufImage[0], NULL
			} ;
			int					nViewCount = 1 ;
			//
			pWnd->ConvertToE3DSDisplayImageBuffer
							( bufImage[0], &m_eiiSGLMeiFrame ) ;
			//
			if ( m_eiiSGLMeiFrame.fdwFormatType & EIF_SIDE_BY_SIDE )
			{
				EGL_IMAGE_INFO	infLeft ;
				if ( !::eglGetStereoLeftImageBuffer
								( &m_eiiSGLMeiFrame, &infLeft ) )
				{
					pWnd->ConvertToE3DSDisplayImageBuffer
										( bufImage[1], &infLeft ) ;
					nViewCount = 2 ;
				}
			}
			//
			pivView3D->AttachThread() ;
			//
			bool	fSuccessful = false ;
			if ( !pivView3D->DrawBuffer
				( drawDynamic | drawTemporary,
						0, 0, pbufImages, nViewCount ) )
			{
				if ( !pivView3D->PrepareView() )
				{
					if ( !pivView3D->ViewImage( 0 ) )
					{
						fSuccessful = true ;
					}
				}
				else
				{
					ESLTrace( "Failed to I3DImageView::PrepareView\n" ) ;
				}
			}
			else
			{
				ESLTrace( "Failed to I3DImageView::DrawBuffer\n" ) ;
			}
			pivView3D->DetachThread() ;
			//
			if ( fSuccessful )
			{
				pWnd->Unlock() ;
				return ;
			}
		}
		//
		// DirectDraw での描画
		//
		EWindow *	pTarget = pWnd->GetWindow( ) ;
		if ( pTarget == NULL )
		{
			pWnd->Unlock() ;
			return ;
		}
		int	xPos = m_eglParam.ptBasePos.x ;
		int	yPos = m_eglParam.ptBasePos.y ;
		if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
		{
			xPos >>= 16 ;
			yPos >>= 16 ;
		}
		EGL_SIZE	sizeDraw ;
		sizeDraw.w = m_eiiSGLMeiFrame.dwImageWidth ;
		sizeDraw.h = m_eiiSGLMeiFrame.dwImageHeight ;
		if ( m_eglParam.pImageAxes != NULL )
		{
			sizeDraw.w = (long int) ::eriRoundR64ToLInt
				( sizeDraw.w * m_eglParam.pImageAxes->xAxis.x ) ;
			sizeDraw.h = (long int) ::eriRoundR64ToLInt
				( sizeDraw.h * m_eglParam.pImageAxes->yAxis.y ) ;
		}
		if ( pWnd->IsImageStretching() )
		{
			EGLPoint	ptUpperLeft( xPos, yPos ) ;
			EGLPoint	ptUnderRight( xPos + sizeDraw.w, yPos + sizeDraw.h ) ;
			pWnd->ClientToWindow( ptUpperLeft ) ;
			pWnd->ClientToWindow( ptUnderRight ) ;
			xPos = ptUpperLeft.x ;
			yPos = ptUpperLeft.y ;
			sizeDraw.w = ptUnderRight.x - ptUpperLeft.x ;
			sizeDraw.h = ptUnderRight.y - ptUpperLeft.y ;
		}
		if ( m_eiiSGLMeiFrame.dwBytesPerLine > 0 )
		{
			yPos += sizeDraw.h - 1 ;
			sizeDraw.h = - sizeDraw.h ;
		}
		if ( m_pDrawImage != NULL )
		{
			const DWORD	fdwFlags =
				EGLDrawImage::dfDirectDraw | EGLDrawImage::dfWaitVerticalBlank ;
			if ( m_pddsVRAM == NULL )
			{
				m_pddsVRAM =
					m_pDrawImage->CreateSurfaceOnVRAM
						( (int) m_eiiSGLMeiFrame.dwImageWidth,
							(int) m_eiiSGLMeiFrame.dwImageHeight ) ;
			}
			m_pDrawImage->DrawImageToDisplay
				( *pTarget, &m_eiiSGLMeiFrame,
					xPos, yPos, &sizeDraw, NULL, fdwFlags, m_pddsVRAM ) ;
		}
		else
		{
			HDC	hdc = ::GetDC( *pTarget ) ;
			::eglDrawToDC
				( hdc, &m_eiiSGLMeiFrame, xPos, yPos, &sizeDraw, NULL ) ;
			::ReleaseDC( *pTarget, hdc ) ;
		}
		pWnd->Unlock() ;
	}
	else
	{
		AttachImage( &m_eiiSGLMeiFrame ) ;
		pWnd->Unlock() ;
		//
		pWnd->WaitForDonePaint( 10 ) ;
		/*
		EWindow *	pTarget = pWnd->GetWindow( ) ;
		if ( !m_flagInStopPlayer && (pTarget != NULL) )
		{
			pTarget->UpdateWindow( ) ;
		}
		*/
	}
}

// 再生区間終端到達
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::OnEndOfDuration( SakuraGL::SGLMediaPlayerInterface * player )
{
}

// SGLMEIMediaPlayer の現在のフレームを m_eiiSGLMeiFrame に取得
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::GetCurrentFrameOfSGLMeiMediaPlayer( void )
{
	using namespace SakuraGL ;
	SGLImageObject *	pImage = m_player->CurrentFrame() ;
	ESLAssert( pImage != NULL ) ;
	//
	SGLImageInfo	imginf ;
	uint8_t *	pbytImage = pImage->LockBuffer( imginf ) ;
	pImage->UnlockBuffer() ;
	//
	memset( &m_eiiSGLMeiFrame, 0, sizeof(m_eiiSGLMeiFrame) ) ;
	m_eiiSGLMeiFrame.dwInfoSize = sizeof(m_eiiSGLMeiFrame) ;
	m_eiiSGLMeiFrame.fdwFormatType = imginf.format ;
	m_eiiSGLMeiFrame.ptrImageArray = pbytImage ;
	m_eiiSGLMeiFrame.dwImageWidth = imginf.width ;
	m_eiiSGLMeiFrame.dwImageHeight = imginf.height ;
	m_eiiSGLMeiFrame.dwBitsPerPixel = imginf.depth ;
	m_eiiSGLMeiFrame.dwBytesPerLine = imginf.pitchLine ;
	m_eiiSGLMeiFrame.dwSizeOfImage = imginf.pitchLine * imginf.height ;
	//
	if ( imginf.format & formatImageFlagPalette )
	{
		EGL_PALETTE *	pPalette = m_aMeiFramePalette.GetArray( 0x100 ) ;
		m_eiiSGLMeiFrame.dwPaletteCount =
			pImage->GetPaletteTable( (SGLPalette*) pPalette, 0x100 ) ;
		m_aMeiFramePalette.FinishArray() ;
		m_eiiSGLMeiFrame.pPaletteEntries = pPalette ;
	}
}

// 再生中のメッセージ処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::OnDispatchMessage( void )
{
	return	eslErrSuccess ;
}

// 時間待ち
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::OnWaitingTime( DWORD dwTime )
{
	if ( ::WaitForSingleObject
		( m_hCancelPlaying, dwTime ) == WAIT_OBJECT_0 )
	{
		return	eslErrTimeout ;
	}
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSMovieSprite::GetTypeName( void ) const
{
	return	L"MovieSprite" ;
}

ECSObject * ECSMovieSprite::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"MovieSprite" ) )
	{
		return	this ;
	}
	return	ECSSprite::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSMovieSprite::Duplicate( void )
{
	return	new ECSMovieSprite ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		ESLError	err =
			ECSSprite::GetFunction( context, nIndex, pwszName ) ;
		if ( err )
		{
			return	err ;
		}
		nIndex += m_staFuncName->GetSize( ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( nIndex >= (int) m_staFuncName->GetSize() )
	{
		nIndex -= m_staFuncName->GetSize() ;
		return	ECSSprite::CallFunction( context, nIndex, lstArg ) ;
	}
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていない MovieSprite 型のメンバ関数を呼び出しています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSMovieSprite::IndexAllMember( void )
{
	ECSSprite::IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::CommitAllReference( ECSContext & context )
{
	const int	nPlayType = m_nPlayType ;
	ECSSprite::CommitAllReference( context ) ;
	//
	if ( m_dwRestoredFrame >= 0 )
	{
		SeekToFrame( m_dwRestoredFrame ) ;
		AttachImage( (PEGL_IMAGE_INFO) ERIAnimation::GetImageInfo() ) ;
		//
		if ( m_dwRestoredStatus == mfsPlaying )
		{
			PlayMovie( m_dwRestoredPlayFlags, nPlayType ) ;
		}
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_wstrFileName.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwStatus, dwFrameIndex = -1, dwReserved = 0 ;
	dwStatus = m_mfsStatus ;
	if ( m_mfsStatus != mfsNotOpened )
	{
		dwFrameIndex = CurrentIndex( ) ;
	}
	file.Write( &dwStatus, sizeof(DWORD) ) ;
	file.Write( &dwFrameIndex, sizeof(DWORD) ) ;
	file.Write( &m_dwMovieFlags, sizeof(DWORD) ) ;
	file.Write( &m_dwLoopStartFrame, sizeof(DWORD) ) ;
	file.Write( &m_dwLoopEndFrame, sizeof(DWORD) ) ;
	file.Write( &dwReserved, sizeof(DWORD) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	const int	nPlayType = m_nPlayType ;
	err = m_wstrFileName.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwStatus = 0, dwFrameIndex = -1, dwPlayFlags = 0, dwReserved = 0 ;
	m_dwRestoredStatus = mfsNotOpened ;
	m_dwRestoredFrame = -1 ;
	file.Read( &dwStatus, sizeof(DWORD) ) ;
	file.Read( &dwFrameIndex, sizeof(DWORD) ) ;
	file.Read( &dwPlayFlags, sizeof(DWORD) ) ;
	file.Write( &m_dwLoopStartFrame, sizeof(DWORD) ) ;
	file.Write( &m_dwLoopEndFrame, sizeof(DWORD) ) ;
	file.Read( &dwReserved, sizeof(DWORD) ) ;
	//
	if ( !m_wstrFileName.m_varStr.IsEmpty() )
	{
		if ( !OpenMovieFile
				( EWideString(m_wstrFileName.m_varStr), &context ) )
		{
			m_dwRestoredStatus = dwStatus ;
			m_dwRestoredFrame = dwFrameIndex ;
			m_dwRestoredPlayFlags = dwPlayFlags ;
			//
			SetLoopPosition( m_dwLoopStartFrame, m_dwLoopEndFrame ) ;
		}
		m_nPlayType = nPlayType ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	ECSSprite::DumpObject( buf, nIndent, context ) ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray * ECSMovieSprite::m_staFuncName = NULL ;
const wchar_t * ECSMovieSprite::m_pwszFuncName[10] =
{
	L"OpenMovie", L"CloseMovie", L"PlayMovie", L"StopMovie",
	L"IsMoviePlaying", L"SeekFrame", L"GetCurrentFrame", L"GetTotalFrame",
	L"GetTotalTime", NULL
} ;
const ECSMovieSprite::PFUNC_CALL ECSMovieSprite::m_pfnCallFunc[9] =
{
	&ECSMovieSprite::Call_OpenMovie,
	&ECSMovieSprite::Call_CloseMovie,
	&ECSMovieSprite::Call_PlayMovie,
	&ECSMovieSprite::Call_StopMovie,
	&ECSMovieSprite::Call_IsMoviePlaying,
	&ECSMovieSprite::Call_SeekFrame,
	&ECSMovieSprite::Call_GetCurrentFrame,
	&ECSMovieSprite::Call_GetTotalFrame,
	&ECSMovieSprite::Call_GetTotalTime,
} ;

// メンバ関数 : Integer OpenMovie( String sFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_OpenMovie
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	err = OpenMovieFile( wstrFileName, &context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseMovie()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_CloseMovie
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = CloseMovie( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer PlayMovie( Integer nFlags, Integer fPlayType := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_PlayMovie
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags, fPlayType ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fPlayType, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	err = PlayMovie( nFlags, fPlayType ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer StopMovie()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_StopMovie
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = StopMovie( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsMoviePlaying()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_IsMoviePlaying
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (long int) IsMoviePlaying() ) ) ;
}

// メンバ関数 : Integer SeekFrame( Integer nFrame )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_SeekFrame
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nFrameIndex ;
	err = context.GetArgumentAsInt( nFrameIndex, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_mfsStatus != mfsNotOpened )
	{
		StopMovie( ) ;
		err = SeekToFrame( nFrameIndex ) ;
		QuickLock( ) ;
		AttachImage( (PEGL_IMAGE_INFO) ERIAnimation::GetImageInfo() ) ;
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetCurrentFrame()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_GetCurrentFrame
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( CurrentIndex() ) ) ;
}

// メンバ関数 : Integer GetTotalFrame()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_GetTotalFrame
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetAllFrameCount() ) ) ;
}

// メンバ関数 : Integer GetTotalTime()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSMovieSprite::Call_GetTotalTime
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetTotalTime() ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ムービー再生用スレッド
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSMovieSprite::ECSPlayerThread, EGLSThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSMovieSprite::ECSPlayerThread::ECSPlayerThread( void )
{
	m_pSprite = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSMovieSprite::ECSPlayerThread::~ECSPlayerThread( void )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ECSMovieSprite::ECSPlayerThread::ThreadProc( void )
{
	if ( m_pSprite != NULL )
	{
		for ( ; ; )
		{
			DWORD	fdwFlags = 0 ;
			if ( m_pSprite->m_dwMovieFlags & mpfNoSkipFrame )
			{
				fdwFlags = ERIAnimationPlayer::ptfNoSkipFrame ;
			}
			SDWORD	dwEndFrame = m_pSprite->m_dwLoopEndFrame ;
			if ( dwEndFrame < 0 )
			{
				dwEndFrame = m_pSprite->GetAllFrameCount() ;
			}
			m_pSprite->ERIAnimationPlayer::PlayTo
				( dwEndFrame, NULL, 0, 0, NULL,
						fdwFlags, ECSMovieSprite::m_pDrawImage ) ;
			if ( !(m_pSprite->m_dwMovieFlags & mpfLoopPlay) )
			{
				break ;
			}
			if ( ::WaitForSingleObject
				( m_pSprite->m_hCancelPlaying, 0 ) == WAIT_OBJECT_0 )
			{
				break ;
			}
			if ( m_pSprite->m_dwLoopStartFrame == 0 )
			{
				if ( m_pSprite->SeekToBegin( ) )
				{
					break ;
				}
			}
			else
			{
				if ( m_pSprite->SeekToFrame( m_pSprite->m_dwLoopStartFrame ) )
				{
					break ;
				}
			}
		}
	}
	return	0 ;
}

