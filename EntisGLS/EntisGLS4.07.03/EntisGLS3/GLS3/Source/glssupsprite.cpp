
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2004-2007 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// 特殊効果用パーティクル
//////////////////////////////////////////////////////////////////////////////

// パーティクルパラメータ計算
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::EParticleObject::CalculateParticle
			( const EFFECT_PARAM & efprm, long int nMilliSec )
{
	nMilliSec -= m_nDelayMilliSec ;
	if ( nMilliSec < 0 )
	{
		nMilliSec = 0 ;
	}
	double	sec = (double) nMilliSec / 1000.0 ;
	double	v = sec ;
	if ( (nMilliSec != 0) && (efprm.rDeceleration != 0) )
	{
		double	a = 1.0 - efprm.rDeceleration ;
		if ( a < 1.0 )
		{
			v = (1.0 - pow( a, sec )) / ((1 - pow( a, 0.01 )) * 100.0) ;
		}
		else
		{
			v = 0 ;
		}
	}
	m_vCurrentPos =
		m_vPosition + m_vVelocity * v + efprm.vGravity * (sec * sec) ;
	m_vCurrentRev = m_vRevSpeed * sec ;
}

// 部分画像取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO
	ECSSuperSprite::EParticleObject::GetPortionImage( void )
{
	if ( (m_imgMask.GetInfo() == NULL) || (m_imgEffect.GetInfo() == NULL) )
	{
		return	m_imgPortion ;
	}
	return	m_imgEffect ;
}

//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::EParticleObject::UpdatePortionImage
		( const EFFECT_PARAM & efprm, unsigned int nDegree )
{
	if ( (m_imgMask.GetInfo() != NULL) && (m_imgEffect.GetInfo() != NULL) )
	{
		long int	nImageDegree = nDegree ;
		if ( efprm.nMilliSecPerDegree != 0 )
		{
			nImageDegree -=
				(long int) ((INT64) m_nDelayMilliSec
								* 0x100 / efprm.nMilliSecPerDegree) ;
		}
		if ( nImageDegree < 0 )
		{
			nImageDegree = 0 ;
		}
		else if ( nImageDegree > 0x100 )
		{
			nImageDegree = 0x100 ;
		}
		nImageDegree = 0x100 - nImageDegree ;
		nImageDegree = 0x100 - (nImageDegree * nImageDegree / 0x100) ;
		//
		const DWORD	dwFlags =
			EGL_BAC_MULTIPLY | EGL_BAC_ADD_ALPHA | EGL_BAC_MULTIPLY_ALPHA ;
		SDWORD	nAlphaRange = efprm.nAlphaRange * 0x10 ;
		SDWORD	nAlphaBase =
			0x100 - (nAlphaRange * 0x10 + 0x100) * nImageDegree / 0x100 ;
		m_imgEffect.BlendAlphaChannel
			( m_imgPortion, m_imgMask, dwFlags, nAlphaBase, nAlphaRange ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 特殊効果用スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSSuperSprite, ECSSprite )

const ECSSuperSprite::PFUNC_BeforeDraw
	ECSSuperSprite::m_pfnBeforeDraw[etMax] =
{
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_ShadingOff,
	&ECSSuperSprite::before_draw_ShadingOff,
	&ECSSuperSprite::before_draw_Normal,
	&ECSSuperSprite::before_draw_Normal,
} ;

const ECSSuperSprite::PFUNC_Draw
	ECSSuperSprite::m_pfnDraw[ECSSuperSprite::etMax] =
{
	&ECSSuperSprite::draw_Normal,
	&ECSSuperSprite::draw_TiledImage,
	&ECSSuperSprite::draw_FilteredImage,
	&ECSSuperSprite::draw_FilteredImage,
	&ECSSuperSprite::draw_FilteredImage,
	&ECSSuperSprite::draw_FilteredImage,
	&ECSSuperSprite::draw_RasterScroll,
	&ECSSuperSprite::draw_MeshTransformation,
	&ECSSuperSprite::draw_MeshTransformation,
	&ECSSuperSprite::draw_MeshTransformation,
	&ECSSuperSprite::draw_ShadingOff,
	&ECSSuperSprite::draw_ShadingOff,
	&ECSSuperSprite::draw_Particle2D,
	&ECSSuperSprite::draw_Particle3D,
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSSuperSprite::ECSSuperSprite( void )
{
	m_eftType = etNothing ;
	m_fdwFlags = 0 ;
	m_sizeView.w = 0 ;
	m_sizeView.h = 0 ;
	m_ptScroll.x = 0 ;
	m_ptScroll.y = 0 ;
	m_nInterval = 0 ;
	m_nIntervalCounter = 0 ;
	m_pBaseMesh = NULL ;
	m_pMesh = NULL ;
	m_nMeshListSize = 0 ;
	m_pMeshList = NULL ;
	m_ptfToneBuffer = NULL ;
	m_fdwShadingUpdate = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSuperSprite::~ECSSuperSprite( void )
{
	delete	m_ptfToneBuffer ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::Release( void )
{
	ECSSprite::Release( ) ;
	//
	EFFECT_PARAM	effprm ;
	::eslFillMemory( &effprm, 0, sizeof(effprm) ) ;
	effprm.eftType = etNothing ;
	SetEffectParameter( effprm ) ;
	//
	return	eslErrSuccess ;
}

// 外接矩形を取得
//////////////////////////////////////////////////////////////////////////////
EGL_RECT ECSSuperSprite::GetRectangle( void )
{
	EGL_RECT	rect ;
	if ( m_eftType == etNothing )
	{
		rect = ECSSprite::GetRectangle( ) ;
	}
	else
	{
		EGLSize	size = GetSize( ) ;
		rect.left = m_ispParam.ptDstPos.x - m_ispParam.ptRevCenter.x ;
		rect.top = m_ispParam.ptDstPos.y - m_ispParam.ptRevCenter.y ;
		rect.right = rect.left + size.w - 1 ;
		rect.bottom = rect.top + size.h - 1 ;
		//
		switch ( m_eftType )
		{
		case	etNothing:
		case	etShadingOff:
		case	etShadingLight:
		case	etWaveCircle:
		case	etShimmer:
			rect = ECSSprite::GetRectangle( ) ;
			break ;
		case	etTileImage:
			rect.right = rect.left + m_sizeView.w - 1 ;
			rect.bottom = rect.top + m_sizeView.h - 1 ;
			break ;
		case	etFilterWhite:
		case	etFilterLight:
		case	etFilterBlack:
		case	etFilterDark:
			break ;
		case	etMeshWarp:
			rect = m_rctMeshExt ;
			rect = LocalToGlobal( rect ) ;
			break ;
		case	efSmashParticle2D:
		case	efSmashParticle3D:
			break ;
		case	etRasterScroll:
			rect.left -= m_nShakingWidth ;
			rect.right += m_nShakingWidth ;
			break ;
		}
	}
	return	rect ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool ECSSuperSprite::GetHiddenRectangle( EGL_RECT & rect )
{
	switch ( m_eftType )
	{
	case	etNothing:
	case	etFilterWhite:
	case	etFilterLight:
	case	etFilterBlack:
	case	etFilterDark:
		return	ECSSprite::GetHiddenRectangle( rect ) ;
	case	etTileImage:
	case	etRasterScroll:
	case	etWaveCircle:
	case	etShimmer:
	case	etShadingOff:
	case	etShadingLight:
	case	efSmashParticle2D:
	case	efSmashParticle3D:
		break ;
	}
	return	false ;
}

// マルチスレッド描画前準備処理
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::BeforeMTDraw( void )
{
	Refresh( ) ;
	//
	ESLAssert( (m_eftType >= 0) && (m_eftType < etMax) ) ;
	(this->*m_pfnBeforeDraw[m_eftType])() ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
	ESLAssert( (m_eftType >= 0) && (m_eftType < etMax) ) ;
	(this->*m_pfnDraw[m_eftType])( hRenderPoly ) ;
}

// スプライト上の指定領域の更新通知
//////////////////////////////////////////////////////////////////////////////
bool ECSSuperSprite::UpdateRect( EGL_RECT * pUpdateRect )
{
	bool	fUpdate = ECSSprite::UpdateRect( pUpdateRect ) ;
	if ( pUpdateRect != NULL )
	{
		m_fdwShadingUpdate = 0 ;
		//
		if ( m_fdwFlags & effTransformation )
		{
			EImageSprite::UpdateRect( NULL ) ;
		}
	}
	return	fUpdate ;
}

// アニメーション進行
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::OnAdvanceAnimation( unsigned int nTime )
{
	m_nIntervalCounter += nTime ;
	if ( (m_nInterval > 0) && (m_nIntervalCounter >= m_nInterval) )
	{
		int	nPastTime = m_nIntervalCounter ;
		int	nModTime = nPastTime % m_nInterval ;
		int	nPastCount = (nPastTime - nModTime) / m_nInterval ;
		m_nIntervalCounter = nModTime ;
		//
		if ( !(m_dwAnimationFlags & animeEffect) )
		{
			nPastCount = 0 ;
		}
		//
		if ( m_eftType == etTileImage )
		{
			//
			// スクロール処理
			//
			if ( (m_dwAnimationFlags & animeEffect) )
			{
				EGLSize		sizeImage = GetSize( ) ;
				m_ptScroll.x += m_ptSpeed.x * nPastCount ;
				m_ptScroll.y += m_ptSpeed.y * nPastCount ;
				if ( m_ptSpeed.x > 0 )
				{
					while ( m_ptScroll.x > 0 )
					{
						m_ptScroll.x -= sizeImage.w ;
					}
				}
				else if ( m_ptScroll.x < 0 )
				{
					while ( m_ptScroll.x <= - sizeImage.w )
					{
						m_ptScroll.x += sizeImage.w ;
					}
				}
				if ( m_ptSpeed.y > 0 )
				{
					while ( m_ptScroll.y > 0 )
					{
						m_ptScroll.y -= sizeImage.h ;
					}
				}
				else if ( m_ptScroll.y < 0 )
				{
					while ( m_ptScroll.y <= - sizeImage.h )
					{
						m_ptScroll.y += sizeImage.h ;
					}
				}
				EImageSprite::UpdateRect( NULL ) ;
			}
		}
		else
		{
			//
			// エフェクト変化
			//
			int	nDegree = m_nBlendDegree + nPastCount * m_nDegreeStep ;
			while ( nDegree >= 0x300 )
			{
				nDegree -= 0x100 ;
			}
			if ( m_dwAnimationFlags & animeEffect )
			{
				SetBlendDegree( nDegree ) ;
			}
			else if ( nDegree != 0 )
			{
				SetBlendDegree( 0 ) ;
			}
		}
	}
	return	ECSSprite::OnAdvanceAnimation( nTime ) ;
}

// 現在の合成マスクの度合いを取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSSuperSprite::GetBlendDegree( void )
{
	if ( m_eftType == etNothing )
	{
		return	ECSSprite::GetBlendDegree() ;
	}
	else
	{
		return	m_nBlendDegree ;
	}
}

// 変形の度合いを設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetBlendDegree( unsigned int nDegree )
{
	if ( m_eftType == etNothing )
	{
		ECSSprite::SetBlendDegree( nDegree ) ;
	}
	else
	{
		switch ( m_eftType )
		{
		case	etNothing:
		case	etTileImage:
			break ;
		case	etFilterWhite:
		case	etFilterLight:
		case	etFilterBlack:
		case	etFilterDark:
			SetDegreeOnColorFilter( m_eftType, nDegree ) ;
			break ;
		case	etRasterScroll:
			break ;
		case	etWaveCircle:
			SetDegreeOnWaveCircle( nDegree ) ;
			break ;
		case	etShimmer:
			break ;
		case	etMeshWarp:
			SetDegreeOnMeshWarp( nDegree ) ;
			break ;
		case	etShadingOff:
		case	etShadingLight:
			break ;
		case	efSmashParticle2D:
		case	efSmashParticle3D:
			SetDegreeOnSmashParticle( nDegree ) ;
			break ;
		}
		m_nBlendDegree = nDegree ;
		EImageSprite::UpdateRect( NULL ) ;
	}
}

// エフェクトを設定する
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetEffectParameter( const EFFECT_PARAM & param )
{
	//
	// 既存のエフェクト用リソースを解放する
	//
	m_pMesh = NULL ;
	m_bufMesh.Delete( ) ;
	m_bufBaseMesh.Delete( ) ;
	m_pBaseMesh = NULL ;
	m_pMesh = NULL ;
	if ( m_pMeshList != NULL )
	{
		::eslHeapFree( NULL, m_pMeshList ) ;
		m_pMeshList = NULL ;
	}
	m_nMeshListSize = 0 ;
	//
	delete	m_ptfToneBuffer ;
	m_ptfToneBuffer = NULL ;
	m_imgViewBuf.DeleteImage( ) ;
	m_lstShadeOff.RemoveAll( ) ;
	m_fdwShadingUpdate = 0 ;
	m_refParticleImage.SetReference( NULL ) ;
	m_lstParticle.RemoveAll( ) ;
	//
	// パラメータ設定
	//
	m_efprm = param ;
	m_eftType = param.eftType ;
	m_fdwFlags = param.fdwFlags ;
	//
	m_nInterval = param.nInterval ;
	m_nIntervalCounter = 0 ;
	m_nDegreeStep = param.nDegreeStep ;
	//
	m_nShakingWidth = param.nShakingWidth ;
	m_nMeshSize = param.nMeshSize ;
	m_nFrequency = param.nFrequency ;
	//
	m_sizeView = param.sizeView ;
	m_ptScroll.x = 0 ;
	m_ptScroll.y = 0 ;
	m_ptSpeed = param.ptSpeed ;
	//
	switch ( m_eftType )
	{
	case	etNothing:
		m_fdwFlags = 0 ;
		break ;
	case	etTileImage:
		m_fdwFlags &= ~effTransition ;
		m_fdwFlags |= effTransformation ;
		break ;
	case	etFilterWhite:
	case	etFilterLight:
	case	etFilterBlack:
	case	etFilterDark:
		m_fdwFlags &= ~effLoopPlaying ;
		m_fdwFlags |= effTransition ;
		InitializeColorFilter( ) ;
		break ;
	case	etRasterScroll:
		m_fdwFlags |= effTransformation ;
		break ;
	case	etWaveCircle:
	case	etShimmer:
		m_fdwFlags |= effTransformation ;
		InitializeMeshTransformation( ) ;
		break ;
	case	etMeshWarp:
		m_fdwFlags |= effTransformation ;
		break ;
	case	etShadingOff:
	case	etShadingLight:
		m_fdwFlags |= effTransformation ;
		InitializeShadingBuffer( ) ;
		break ;
	case	efSmashParticle2D:
	case	efSmashParticle3D:
		m_fdwFlags |= effTransformation ;
		InitializeSmashParticle( ) ;
		break ;
	}
}

// メッシュワープエフェクトを設定する
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetMeshWarpEffect
	( const E3D_VECTOR_2D * pvMesh, int nMeshListCount,
		const EGL_SIZE & sizeInMesh, const E3D_VECTOR_2D * pvBaseMesh )
{
	EFFECT_PARAM	efprm ;
	::eslFillMemory( &efprm, 0, sizeof(efprm) ) ;
	efprm.eftType = etMeshWarp ;
	efprm.fdwFlags |= effTransition ;
	SetEffectParameter( efprm ) ;
	//
	EGLSize	sizeImage = GetSize( ) ;
	m_sizeInMesh = sizeInMesh ;
	m_vMeshSize.x = (REAL32) ((double) sizeImage.w / m_sizeInMesh.w) ;
	m_vMeshSize.y = (REAL32) ((double) sizeImage.h / m_sizeInMesh.h) ;
	m_sizeInMesh.w += 1 ;
	m_sizeInMesh.h += 1 ;
	//
	long int	nMeshSize = m_sizeInMesh.w * m_sizeInMesh.h ;
	m_pBaseMesh = (E3D_VECTOR_2D*)
		m_bufBaseMesh.PutBuffer( nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
	if ( pvBaseMesh != NULL )
	{
		::eslMoveMemory
			( m_pBaseMesh, pvBaseMesh, nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
	}
	else
	{
		for ( int y = 0; y < m_sizeInMesh.h; y ++ )
		{
			int	iy = y * m_sizeInMesh.w ;
			for ( int x = 0; x < m_sizeInMesh.w; x ++ )
			{
				int	i = iy + x ;
				m_pBaseMesh[i].x = (REAL32) (x * m_vMeshSize.x) ;
				m_pBaseMesh[i].y = (REAL32) (y * m_vMeshSize.y) ;
			}
		}
	}
	//
	m_pMesh = (E3D_VECTOR_2D*)
		m_bufMesh.PutBuffer( nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
	::eslMoveMemory
		( m_pMesh, pvMesh, nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
	//
	m_nMeshListSize = nMeshListCount ;
	m_pMeshList = (E3D_VECTOR_2D*)
		::eslHeapAllocate
			( NULL, nMeshSize * nMeshListCount * sizeof(E3D_VECTOR_2D), 0 ) ;
	::eslMoveMemory
		( m_pMeshList, pvMesh,
			nMeshSize * nMeshListCount * sizeof(E3D_VECTOR_2D) ) ;
	//
	SetDegreeOnMeshWarp( GetBlendDegree() ) ;
}

// メッシュワープ変形
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetDegreeOnMeshWarp( unsigned int nDegree )
{
	long int	i, nMeshSize = m_sizeInMesh.w * m_sizeInMesh.h ;
	//
	EImageSprite::UpdateRect( NULL ) ;
	//
	if ( m_nMeshListSize >= 2 )
	{
		//
		// メッシュ補完処理
		//
		int		nDegreeStep = 0x100 / (m_nMeshListSize - 1) ;
		int		iMeshIndex = nDegree * (m_nMeshListSize - 1) / 0x100 ;
		double	t = (double) (nDegree - iMeshIndex * nDegreeStep) / nDegreeStep ;
		if ( t > 1.0 )
		{
			t = 1.0 ;
		}
		if ( iMeshIndex >= m_nMeshListSize )
		{
			::eslMoveMemory
				( m_pMesh, m_pMeshList + (nMeshSize * (m_nMeshListSize - 1)),
					nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
		}
		else
		{
			E3D_VECTOR_2D *	pvList1 = m_pMeshList + (iMeshIndex * nMeshSize) ;
			E3D_VECTOR_2D *	pvList2 = pvList1 + nMeshSize ;
			for ( i = 0; i < nMeshSize; i ++ )
			{
				m_pMesh[i] = pvList1[i] + (pvList2[i] - pvList1[i]) * t ;
			}
		}
	}
	//
	// メッシュ外接矩形取得
	//
	m_rctMeshExt.left = 0x7FFF ;
	m_rctMeshExt.top = 0x7FFF ;
	m_rctMeshExt.right = -0x7FFF ;
	m_rctMeshExt.bottom = -0x7FFF ;
	//
	ESLAssert( m_pMesh != NULL ) ;
	if ( m_pMesh != NULL )
	for ( i = 0; i < nMeshSize; i ++ )
	{
		if ( m_rctMeshExt.left >= m_pMesh[i].x )
		{
			m_rctMeshExt.left = ::eriRoundR32ToInt( m_pMesh[i].x ) - 1 ;
		}
		if ( m_rctMeshExt.right <= m_pMesh[i].x )
		{
			m_rctMeshExt.right = ::eriRoundR32ToInt( m_pMesh[i].x ) + 1 ;
		}
		if ( m_rctMeshExt.top >= m_pMesh[i].y )
		{
			m_rctMeshExt.top = ::eriRoundR32ToInt( m_pMesh[i].y ) - 1 ;
		}
		if ( m_rctMeshExt.bottom <= m_pMesh[i].y )
		{
			m_rctMeshExt.bottom = ::eriRoundR32ToInt( m_pMesh[i].y ) + 1 ;
		}
	}
}

// 色フィルタの初期設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::InitializeColorFilter( void )
{
	if ( m_ptfToneBuffer == NULL )
	{
		m_ptfToneBuffer = new ECSToneFilter ;
		SetDegreeOnColorFilter( m_eftType, m_nBlendDegree ) ;
	}
}

// 色フィルタ設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetDegreeOnColorFilter
	( EffectType eftType, unsigned int nDegree )
{
	ESLAssert( m_ptfToneBuffer != NULL ) ;
	if ( m_ptfToneBuffer != NULL )
	{
		int	nTone = nDegree, nFlag = EGL_TONE_BRIGHTNESS ;
		switch ( eftType )
		{
		case	etFilterLight:
			nFlag = EGL_TONE_LIGHT ;
			break ;
		case	etFilterBlack:
			nTone = - (int) nDegree ;
			break ;
		case	etFilterDark:
			nTone = - (int) nDegree ;
			nFlag = EGL_TONE_LIGHT ;
			break ;
		}
		m_ptfToneBuffer->SetGeneralTone
			( nTone, nFlag, nTone, nFlag,
				nTone, nFlag, 0, EGL_TONE_BRIGHTNESS, 0 ) ;
	}
}

// メッシュの初期設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::InitializeMeshTransformation( void )
{
	if ( m_nMeshSize <= 0 )
	{
		m_nMeshSize = 16 ;
	}
	EGLSize	sizeImage = GetSize( ) ;
	m_sizeInMesh.w =
		(sizeImage.w + m_nMeshSize - 1) / m_nMeshSize ;
	m_sizeInMesh.h =
		(sizeImage.h + m_nMeshSize - 1) / m_nMeshSize ;
	m_vMeshSize.x = (REAL32) ((double) sizeImage.w / m_sizeInMesh.w) ;
	m_vMeshSize.y = (REAL32) ((double) sizeImage.h / m_sizeInMesh.h) ;
	m_sizeInMesh.w += 1 ;
	m_sizeInMesh.h += 1 ;
	m_pBaseMesh = (E3D_VECTOR_2D*) m_bufBaseMesh.PutBuffer
		( m_sizeInMesh.w * m_sizeInMesh.h * sizeof(E3D_VECTOR_2D) ) ;
	m_pMesh = (E3D_VECTOR_2D*) m_bufMesh.PutBuffer
		( m_sizeInMesh.w * m_sizeInMesh.h * sizeof(E3D_VECTOR_2D) ) ;
	//
	for ( int y = 0; y < m_sizeInMesh.h; y ++ )
	{
		int	iy = y * m_sizeInMesh.w ;
		for ( int x = 0; x < m_sizeInMesh.w; x ++ )
		{
			int	i = iy + x ;
			m_pBaseMesh[i].x = (REAL32) (x * m_vMeshSize.x) ;
			m_pBaseMesh[i].y = (REAL32) (y * m_vMeshSize.y) ;
			m_pMesh[i] = m_pBaseMesh[i] ;
		}
	}
	//
	SetDegreeOnWaveCircle( GetBlendDegree() ) ;
}

// 波紋変形メッシュ生成
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetDegreeOnWaveCircle( unsigned int nDegree )
{
	E3D_VECTOR_2D	vCenter ;
	double	t, rad_r, r, p ;
	vCenter.x = (REAL32) ((m_sizeInMesh.w - 1) * 0.5) ;
	vCenter.y = (REAL32) ((m_sizeInMesh.h - 1) * 0.5) ;
	t = (double) nDegree / 0x100 * m_nFrequency ;
	rad_r = 3.14159265358979 / 4 ;
	//
	for ( int y = 0; y < m_sizeInMesh.h; y ++ )
	{
		int	iy = y * m_sizeInMesh.w ;
		for ( int x = 0; x < m_sizeInMesh.w; x ++ )
		{
			E3DVector2D	vPos( x - vCenter.x, y - vCenter.y ) ;
			double		dist = vPos.x * vPos.x + vPos.y * vPos.y ;
			int			i = iy + x ;
			m_pMesh[i].x = (REAL32) (x * m_vMeshSize.x) ;
			m_pMesh[i].y = (REAL32) (y * m_vMeshSize.y) ;
			if ( (dist == 0) )
			{
				continue ;
			}
			dist = sqrt( dist ) ;			// dist := abs(vPos)
			if ( dist > t )
			{
				continue ;
			}
			vPos *= (1.0 / dist) ;			// abs(vPos) == 1.0
			r = (t - dist) * rad_r ;
			p = sin( r ) ;
			if ( (p < 0.0) & ((y == 0) | (x == 0)
				| (y == m_sizeInMesh.h - 1) | (x == m_sizeInMesh.w - 1)) )
			{
				p = 0 ;
			}
			m_pMesh[i].x += (REAL32) (vPos.x * p * m_nShakingWidth) ;
			m_pMesh[i].y += (REAL32) (vPos.y * p * m_nShakingWidth) ;
		}
	}
}

// ぼかしバッファの初期設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::InitializeShadingBuffer( int iBuffering )
{
	//
	// バッファ生成
	//
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( pInfo == NULL )
	{
		return ;
	}
	Refresh( ) ;
	//
	PEGL_IMAGE_INFO	pViewInf = m_imgViewBuf.GetInfo() ;
	if ( (pViewInf == NULL)
		|| ((pViewInf->fdwFormatType != pInfo->fdwFormatType)
			|| (pViewInf->dwImageWidth != pInfo->dwImageWidth)
			|| (pViewInf->dwImageHeight != pInfo->dwImageHeight)) )
	{
		m_imgViewBuf.CreateImage
			( pInfo->fdwFormatType,
				pInfo->dwImageWidth, pInfo->dwImageHeight, 32 ) ;
		//
		for ( int i = 0; i <= 4; i ++ )
		{
			m_lstShadeOff[i].CreateImage
				( pInfo->fdwFormatType,
					pInfo->dwImageWidth, pInfo->dwImageHeight, 32 ) ;
		}
	}
	//
	// ぼかし画像生成
	//
	EGL_DRAW_PARAM	dp ;
	PEGL_IMAGE_INFO	pLastBuf = m_lstShadeOff[0] ;
	ESLAssert( pLastBuf != NULL ) ;
	if ( !(m_fdwShadingUpdate & 0x0001) )
	{
		HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
		hDraw->Initialize( pLastBuf, NULL, NULL ) ;
		::eslFillMemory( &dp, 0, sizeof(dp) ) ;
		dp.pSrcImage = pInfo ;
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
		hDraw->Release( ) ;
		//
		m_fdwShadingUpdate = 0x0001 ;
	}
	if ( iBuffering <= 0 )
	{
		iBuffering = 4 ;
	}
	for ( int i = 1; i <= iBuffering; i ++ )
	{
		PEGL_IMAGE_INFO	pShadingBuf = m_lstShadeOff[i] ;
		ESLAssert( pShadingBuf != NULL ) ;
		if ( !(m_fdwShadingUpdate & (1 << i)) )
		{
			::eriImageFilterLoop421( *pShadingBuf, pLastBuf ) ;
			for ( int j = 0; j < 15; j ++ )
			{
				::eriImageFilterLoop421( *pShadingBuf ) ;
			}
			m_fdwShadingUpdate |= (1 << i) ;
		}
		pLastBuf = pShadingBuf ;
	}
}

// 粉砕パーティクル初期化
//////////////////////////////////////////////////////////////////////////////
static inline long int MakeRandom( DWORD & dwRandomSeed, long int nLimit )
{
	dwRandomSeed = dwRandomSeed * 5 + 0x76A1E871 ;
	if ( nLimit == 0 )
	{
		return	0 ;
	}
	return	(long int) (dwRandomSeed >> 16) % nLimit ;
}
void ECSSuperSprite::InitializeSmashParticle( void )
{
	m_lstParticle.RemoveAll( ) ;
	//
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( pInfo == NULL )
	{
		return ;
	}
	EGLSize	sizeMesh( m_efprm.nMeshSize, m_efprm.nMeshSize ) ;
	EGLSize	sizeMeshStep ;
	if ( m_efprm.pImageInf != NULL )
	{
		sizeMesh.w = m_efprm.pImageInf->dwImageWidth ;
		sizeMesh.h = m_efprm.pImageInf->dwImageHeight ;
	}
	if ( (sizeMesh.w | sizeMesh.h) == 0 )
	{
		return ;
	}
	EGLSize		sizeImage( pInfo->dwImageWidth, pInfo->dwImageHeight ) ;
	EGLPoint	ptSmashPos( sizeImage.w / 2, sizeImage.h / 2 ) ;
	DWORD		dwRandomSeed = ::timeGetTime( ) ;
	ptSmashPos += m_efprm.ptSmashPoint ;
	sizeMeshStep = sizeMesh ;
	//
	if ( m_efprm.nMeshDivision == 1 )
	{
		sizeMeshStep.w >>= 1 ;
		sizeMeshStep.h >>= 1 ;
		if ( sizeMeshStep.w == 0 )
		{
			sizeMeshStep.w = 1 ;
		}
		if ( sizeMeshStep.h == 0 )
		{
			sizeMeshStep.h = 1 ;
		}
	}
	//
	int	xBaseOffset = 0 ;
	for ( int y = 0; y < sizeImage.h; y += sizeMeshStep.h )
	{
		EGLRect	rctPortion ;
		rctPortion.top = y ;
		rctPortion.bottom = y + sizeMesh.h - 1 ;
		if ( rctPortion.bottom >= sizeImage.h )
		{
			rctPortion.bottom = sizeImage.h - 1 ;
			if ( xBaseOffset > 0 )
			{
				break ;
			}
		}
		for ( int x = xBaseOffset; x < sizeImage.w; x += sizeMesh.w )
		{
			//
			// パーティクル矩形取得
			//
			rctPortion.left = x ;
			rctPortion.right = x + sizeMesh.w - 1 ;
			if ( rctPortion.right >= sizeImage.w )
			{
				rctPortion.right = sizeImage.w - 1 ;
			}
			//
			// パーティクル画像設定
			//
			EParticleObject *	pParticle = new EParticleObject ;
			m_lstParticle.Add( pParticle ) ;
			pParticle->m_imgPortion.SetImageView( pInfo, &rctPortion ) ;
			if ( m_efprm.pImageInf != NULL )
			{
				EGLRect	rctMask ;
				rctMask.left = 0 ;
				rctMask.top = 0 ;
				rctMask.right = rctPortion.right - rctPortion.left ;
				rctMask.bottom = rctPortion.bottom - rctPortion.top ;
				pParticle->m_imgMask.
					SetImageView( m_efprm.pImageInf, &rctMask ) ;
				pParticle->m_imgEffect.
					CreateImage( EIF_RGBA_BITMAP,
						rctMask.right + 1, rctMask.bottom + 1, 32 ) ;
			}
			//
			// パラメータ設定
			//
			pParticle->m_ptRevCenter.x =
				(rctPortion.right - rctPortion.left) / 2 ;
			pParticle->m_ptRevCenter.y =
				(rctPortion.bottom - rctPortion.top) / 2 ;
			//
			pParticle->m_vPosition.x =
				(REAL32) (rctPortion.left + pParticle->m_ptRevCenter.x) ;
			pParticle->m_vPosition.y =
				(REAL32) (rctPortion.top + pParticle->m_ptRevCenter.y) ;
			pParticle->m_vPosition.z = 0 ;
			//
			pParticle->m_vVelocity.x =
				(REAL32) (pParticle->m_vPosition.x - ptSmashPos.x) ;
			pParticle->m_vVelocity.y =
				(REAL32) (pParticle->m_vPosition.y - ptSmashPos.y) ;
			pParticle->m_vVelocity.z = 0 ;
			//
			double	r = pParticle->m_vVelocity.Absolute( ) ;
			pParticle->m_vVelocity *=
				(1.0 + 1.0 / sqrt(r)) * m_efprm.rSmashPower / r ;
			pParticle->m_vVelocity += m_efprm.vVelocity ;
			pParticle->m_vVelocity.x +=
				(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
									/ 1000.0) * m_efprm.rRandomPower) ;
			pParticle->m_vVelocity.y +=
				(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
									/ 1000.0) * m_efprm.rRandomPower) ;
			if ( m_eftType == efSmashParticle3D )
			{
				pParticle->m_vVelocity.z +=
					(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
										/ 1000.0) * m_efprm.rRandomPower) ;
			}
			pParticle->m_nDelayMilliSec =
				(long int) ::eriRoundR64ToLInt( m_efprm.rSmashDelay * r ) ;
			//
			pParticle->m_vRevSpeed = m_efprm.vRevSpeed ;
			pParticle->m_vRevSpeed.z +=
				(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
									/ 1000.0) * m_efprm.vRevRandom.z) ;
			if ( m_eftType == efSmashParticle3D )
			{
				pParticle->m_vRevSpeed.x +=
					(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
										/ 1000.0) * m_efprm.vRevRandom.x) ;
				pParticle->m_vRevSpeed.y +=
					(REAL32) (((MakeRandom( dwRandomSeed, 2000 ) - 1000)
										/ 1000.0) * m_efprm.vRevRandom.y) ;
			}
			//
			pParticle->CalculateParticle( m_efprm, 0 ) ;
			pParticle->UpdatePortionImage( m_efprm, 0 ) ;
		}
		xBaseOffset += sizeMeshStep.w ;
		if ( xBaseOffset >= sizeMesh.w )
		{
			xBaseOffset = 0 ;
		}
	}
}

// 粉砕パーティクル設定
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::SetDegreeOnSmashParticle( unsigned int nDegree )
{
	long int	nMilliSec = (long int)
		( (INT64) m_efprm.nMilliSecPerDegree * nDegree / 0x100 ) ;
	for ( int i = 0; i < (int) m_lstParticle.GetSize(); i ++ )
	{
		EParticleObject *	pParticle = m_lstParticle.GetAt( i ) ;
		ESLAssert( pParticle != NULL ) ;
		if ( pParticle != NULL )
		{
			pParticle->CalculateParticle( m_efprm, nMilliSec ) ;
			pParticle->UpdatePortionImage( m_efprm, nDegree ) ;
		}
	}
}

// 描画前準備処理（何もしない）
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::before_draw_Normal( void )
{
}

// 通常スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_Normal( HEGL_RENDER_POLYGON hRenderPoly )
{
	ECSSprite::MTDraw( hRenderPoly ) ;
}

// タイル状表示
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_TiledImage( HEGL_RENDER_POLYGON hRenderPoly )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( pInfo == NULL )
	{
		return ;
	}
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	EGLRect	rectView = GetRectangle( ) ;
	EGLSize	sizeImage( pInfo->dwImageWidth, pInfo->dwImageHeight ) ;
	//
	EGLPoint	ptScroll = m_ptScroll ;
	while ( ptScroll.x > 0 )
	{
		ptScroll.x -= sizeImage.w ;
	}
	while ( ptScroll.y > 0 )
	{
		ptScroll.y -= sizeImage.h ;
	}
	//
	EGL_DRAW_PARAM	dp ;
	EGL_RECT		rectClip ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.dwFlags = m_eglParam.dwFlags & EGL_DRAW_BLEND_ALPHA ;
	dp.pSrcImage = pInfo ;
	dp.pViewRect = &rectClip ;
	dp.nTransparency = m_ispParam.nTransparency ;
	//
	for ( int y = ptScroll.y; y < m_sizeView.h; y += sizeImage.h )
	{
//		dp.ptBasePos.y = y ;
		if ( y < 0 )
		{
			dp.ptBasePos.y = rectView.top ;
			rectClip.top = - y ;
		}
		else
		{
			dp.ptBasePos.y = rectView.top + y ;
			rectClip.top = 0 ;
		}
		rectClip.bottom = sizeImage.h - 1 ;
		if ( y + (sizeImage.h - rectClip.top) > m_sizeView.h )
		{
			rectClip.bottom +=
				m_sizeView.h - (y + (sizeImage.h - rectClip.top)) ;
		}
		for ( int x = ptScroll.x; x < m_sizeView.w; x += sizeImage.w )
		{
//			dp.ptBasePos.x = x ;
			if ( x < 0 )
			{
				dp.ptBasePos.x = rectView.left ;
				rectClip.left = - x ;
			}
			else
			{
				dp.ptBasePos.x = rectView.left + x ;
				rectClip.left = 0 ;
			}
			rectClip.right = sizeImage.w - 1 ;
			if ( x + (sizeImage.w - rectClip.left) > m_sizeView.w )
			{
				rectClip.right +=
					m_sizeView.w - (x + (sizeImage.w - rectClip.left)) ;
			}
			if ( !hDraw->PrepareDraw( &dp ) )
			{
				hDraw->DrawImage( ) ;
			}
		}
	}
}

// 色フィルタ描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_FilteredImage( HEGL_RENDER_POLYGON hRenderPoly )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( (pInfo == NULL) && (m_ptfToneBuffer != NULL) )
	{
		return ;
	}
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	EGL_DRAW_DEST	ddst ;
	hDraw->GetDestination( &ddst ) ;
	//
	EGL_POINT		ptDstPos ;
	EGL_IMAGE_RECT	irClip ;
	ptDstPos.x = m_ispParam.ptDstPos.x - m_ispParam.ptRevCenter.x ;
	ptDstPos.y = m_ispParam.ptDstPos.y - m_ispParam.ptRevCenter.y ;
	irClip.x = 0 ;
	irClip.y = 0 ;
	irClip.w = pInfo->dwImageWidth ;
	irClip.h = pInfo->dwImageHeight ;
	//
	if ( ptDstPos.x < ddst.rectDstClip.left )
	{
		irClip.x = ddst.rectDstClip.left - ptDstPos.x ;
		irClip.w -= irClip.x ;
		ptDstPos.x = ddst.rectDstClip.left ;
	}
	if ( ptDstPos.y < ddst.rectDstClip.top )
	{
		irClip.y = ddst.rectDstClip.top - ptDstPos.y ;
		irClip.h -= irClip.y ;
		ptDstPos.y = ddst.rectDstClip.top ;
	}
	if ( ptDstPos.x + irClip.w > ddst.rectDstClip.right + 1 )
	{
		irClip.w = (ddst.rectDstClip.right + 1) - ptDstPos.x ;
	}
	if ( ptDstPos.y + irClip.h > ddst.rectDstClip.bottom + 1 )
	{
		irClip.h = (ddst.rectDstClip.bottom + 1) - ptDstPos.y ;
	}
	if ( (irClip.w <= 0) || (irClip.h <= 0) )
	{
		return ;
	}
	//
	EGL_IMAGE_INFO	iiSrc, iiDst ;
	if ( ::eglGetClippedImageInfo( &iiSrc, pInfo, &irClip ) )
	{
		return ;
	}
	irClip.x = ptDstPos.x ;
	irClip.y = ptDstPos.y ;
	if ( ::eglGetClippedImageInfo( &iiDst, ddst.pDstImage, &irClip ) )
	{
		return ;
	}
	::eglApplyToneTable
		( &iiDst, &iiSrc,
			m_ptfToneBuffer->m_bytBlue, m_ptfToneBuffer->m_bytGreen,
			m_ptfToneBuffer->m_bytRed, m_ptfToneBuffer->m_bytAlpha ) ;
}

// ラスタスクロール
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_RasterScroll( HEGL_RENDER_POLYGON hRenderPoly )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	if ( pInfo == NULL )
	{
		return ;
	}
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	//
	const double	rPI = 3.14159265 ;
	int		nDegree = m_nBlendDegree ;
	double	rDegree = (double) nDegree / 0x100 ;
	double	rShakingWidth = m_nShakingWidth ;
	double	rPhase = (rPI * m_nFrequency) * rDegree ;
	double	rWaveLength = rPI / (double) m_nMeshSize ;
	if ( nDegree < 0x100 )
	{
		rShakingWidth *= rDegree ;
	}
	//
	EGL_DRAW_PARAM	dp ;
	EGL_RECT		rectView ;
	EGL_POINT		ptDstPos ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.pSrcImage = pInfo ;
	dp.pViewRect = &rectView ;
	if ( m_fdwFlags & effTransition )
	{
		dp.nTransparency = m_nBlendDegree * m_nBlendDegree / 0x100 ;
	}
	rectView.left = 0 ;
	rectView.right = pInfo->dwImageWidth - 1 ;
	ptDstPos.x = m_ispParam.ptDstPos.x - m_ispParam.ptRevCenter.x ;
	ptDstPos.y = m_ispParam.ptDstPos.y - m_ispParam.ptRevCenter.y ;
	//
	for ( int y = 0; y < (int) pInfo->dwImageHeight; y ++ )
	{
		dp.ptBasePos.x =
			ptDstPos.x + (int) ::eriRoundR64ToLInt
				( rShakingWidth * sin( y * rWaveLength + rPhase ) ) ;
		dp.ptBasePos.y = ptDstPos.y + y ;
		rectView.top = y ;
		rectView.bottom = y ;
		//
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
}

// 変形描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_MeshTransformation( HEGL_RENDER_POLYGON hRenderPoly )
{
	EGL_DRAW_PARAM	dp ;
	EGL_IMAGE_AXES	iax ;
	EGL_IMAGE_AXES	iaxParent ;
	E3D_VECTOR_2D	vParentBase ;
	E3D_VECTOR_2D	vBasePos ;
	E3D_VECTOR_2D	vp[3] ;
	E3D_VECTOR_2D	uv[3] ;
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	//
	if ( m_eglParam.pImageAxes != NULL )
	{
		iaxParent = *(m_eglParam.pImageAxes) ;
	}
	else
	{
		iaxParent.xAxis.x = 1 ;
		iaxParent.xAxis.y = 0 ;
		iaxParent.yAxis.x = 0 ;
		iaxParent.yAxis.y = 1 ;
	}
	if ( m_eglParam.dwFlags & EGL_FIXED_POSITION )
	{
		const double	r = 1.0 / 0x10000 ;
		vParentBase.x = (REAL32) (m_eglParam.ptBasePos.x * r) ;
		vParentBase.y = (REAL32) (m_eglParam.ptBasePos.y * r) ;
	}
	else
	{
		vParentBase.x = (REAL32) m_eglParam.ptBasePos.x ;
		vParentBase.y = (REAL32) m_eglParam.ptBasePos.y ;
	}
	//
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp = m_eglParam ;
	dp.dwFlags |= EGL_FIXED_POSITION | EGL_POLYGON_SHAPED ;
	dp.pSrcImage = GetInfo( ) ;
	dp.pImageAxes = &iax ;
	dp.nVertexCount = 3 ;
	dp.pVertexPos = vp ;
	if ( m_fdwFlags & effTransition )
	{
		dp.nTransparency = m_nBlendDegree * m_nBlendDegree / 0x100 ;
	}
	//
	for ( int y = 1; y < m_sizeInMesh.h; y ++ )
	{
		int	iy = y * m_sizeInMesh.w ;
		int	jy = iy - m_sizeInMesh.w ;
		//
		for ( int x = 1; x < m_sizeInMesh.w; x ++ )
		{
			uv[0] = m_pBaseMesh[jy + x - 1] ;
			uv[1] = m_pBaseMesh[jy + x] ;
			uv[2] = m_pBaseMesh[iy + x - 1] ;
			//
			vp[0] = m_pMesh[jy + x - 1] ;
			vp[1] = m_pMesh[jy + x] ;
			vp[2] = m_pMesh[iy + x - 1] ;
			//
			::eglMapping2DVectors
				( &iaxParent, &vParentBase, vp, vp, 3 ) ;
			if ( !::eglBaseVectorFromMapping2D( &iax, &vBasePos, vp, uv ) )
			{
				dp.ptBasePos.x = ::eriRoundR32ToInt( vBasePos.x * 0x10000 ) ;
				dp.ptBasePos.y = ::eriRoundR32ToInt( vBasePos.y * 0x10000 ) ;
				if ( !hDraw->PrepareDraw( &dp ) )
				{
					hDraw->DrawImage( ) ;
				}
			}
			//
			uv[0] = m_pBaseMesh[iy + x] ;
			vp[0] = m_pMesh[iy + x] ;
			vp[1] = m_pMesh[jy + x] ;
			vp[2] = m_pMesh[iy + x - 1] ;
			//
			::eglMapping2DVectors
				( &iaxParent, &vParentBase, vp, vp, 3 ) ;
			if ( !::eglBaseVectorFromMapping2D( &iax, &vBasePos, vp, uv ) )
			{
				dp.ptBasePos.x = ::eriRoundR32ToInt( vBasePos.x * 0x10000 ) ;
				dp.ptBasePos.y = ::eriRoundR32ToInt( vBasePos.y * 0x10000 ) ;
				if ( !hDraw->PrepareDraw( &dp ) )
				{
					hDraw->DrawImage( ) ;
				}
			}
		}
	}
}

// ぼかし描画前準備処理
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::before_draw_ShadingOff( void )
{
	int	nDegree = m_nBlendDegree ;
	int	nTrans = 0x100 - (nDegree * nDegree / 0x100) ;
	if ( nTrans > 0x100 )
	{
		nTrans = 0x100 ;
	}
	if ( nDegree < 0 )
	{
		nDegree = 0 ;
	}
	else if ( nDegree > 0xFF )
	{
		nDegree = 0xFF ;
	}
	int		nIndex = (nDegree >> 6) ;
	int		nOffset = (nDegree & 0x3F) * 0x100 / 0x3F ;
	//
	if ( !(m_fdwShadingUpdate & (2 << nIndex)) )
	{
		InitializeShadingBuffer( nIndex + 1 ) ;
	}
	EGL_DRAW_PARAM	dp ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
	hDraw->Initialize( m_imgViewBuf, NULL, NULL ) ;
	//
	dp.pSrcImage = m_lstShadeOff[nIndex] ;
	if ( !hDraw->PrepareDraw( &dp ) )
	{
		hDraw->DrawImage( ) ;
	}
	//
	dp.dwFlags = EGL_DRAW_A_MOVE ;
	dp.pSrcImage = m_lstShadeOff[nIndex + 1] ;
	dp.nTransparency = 0x100 - nOffset ;
	if ( !hDraw->PrepareDraw( &dp ) )
	{
		hDraw->DrawImage( ) ;
	}
	//
	if ( m_eftType == etShadingLight )
	{
		EGL_RECT	rect ;
		EGLPalette	rgbWhite( 0x00FFFFFF ) ;
		rect.left = 0 ;
		rect.top = 0 ;
		rect.right = dp.pSrcImage->dwImageWidth - 1 ;
		rect.bottom = dp.pSrcImage->dwImageHeight - 1 ;
		if ( !hDraw->PrepareFillRect
			( &rect, rgbWhite, nTrans, EGL_DRAW_BLEND_ALPHA ) )
		{
			hDraw->FillRegion( ) ;
		}
	}
	hDraw->Release( ) ;
}

// ぼかし描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_ShadingOff( HEGL_RENDER_POLYGON hRenderPoly )
{
	//
	// ぼかし画像描画
	//
	HEGL_DRAW_IMAGE	hDraw ;
	EGL_DRAW_PARAM	dp ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	hDraw = hRenderPoly->GetDrawImage( ) ;
	dp = m_eglParam ;
	dp.pSrcImage = m_imgViewBuf ;
	if ( (m_eftType == etShadingOff) && (m_fdwFlags & effTransition) )
	{
		dp.nTransparency = m_nBlendDegree * m_nBlendDegree / 0x100 ;
	}
	if ( !hDraw->PrepareDraw( &dp ) )
	{
		hDraw->DrawImage( ) ;
	}
}

// パーティクル 2D 描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_Particle2D( HEGL_RENDER_POLYGON hRenderPoly )
{
	EGL_DRAW_PARAM	dp ;
	EGL_IMAGE_AXES	iax ;
	HEGL_DRAW_IMAGE	hDraw = hRenderPoly->GetDrawImage( ) ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.dwFlags = m_ispParam.dwFlags ;
	dp.nTransparency = m_nBlendDegree * m_nBlendDegree / 0x100 ;
	dp.pImageAxes = &iax ;
	//
	for ( int i = 0; i < (int) m_lstParticle.GetSize(); i ++ )
	{
		EParticleObject *	pParticle = m_lstParticle.GetAt( i ) ;
		if ( pParticle == NULL )
		{
			continue ;
		}
		if ( pParticle->m_imgMask.GetInfo() != NULL )
		{
			dp.nTransparency = 0 ;
		}
		dp.pSrcImage = pParticle->GetPortionImage( ) ;
		dp.ptBasePos.x = ::eriRoundR32ToInt( pParticle->m_vCurrentPos.x ) ;
		dp.ptBasePos.y = ::eriRoundR32ToInt( pParticle->m_vCurrentPos.y ) ;
		::eglGetRevolvedAxes
			( &iax, &(dp.ptBasePos), &(pParticle->m_ptRevCenter),
						1, 1, pParticle->m_vCurrentRev.z, 90, 0 ) ;
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
}

// パーティクル 3D 描画
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::draw_Particle3D( HEGL_RENDER_POLYGON hRenderPoly )
{
	E3D_RENDER_PARAM	rp ;
	E3D_REV_MATRIX		revmat ;
	E3DVector			vUnit( 1, 1, 1 ) ;
	const double		rPIby180 = 3.1415926535897932384626433832795 /180.0 ;
	::eslFillMemory( &rp, 0, sizeof(rp) ) ;
	rp.dwFlags =
		E3DRP_RENDER_REV_IMAGE | (m_ispParam.dwFlags & EGL_WITH_Z_ORDER) ;
	rp.dwTransparency = m_nBlendDegree * m_nBlendDegree / 0x100 ;
	rp.rgbaColor.rgbAdd = m_ispParam.rgbDimColor ;
	rp.rgbaColor.rgbMul = m_ispParam.rgbLightColor ;
	rp.rev.pRevMatrix = &revmat ;
	//
	for ( int i = 0; i < (int) m_lstParticle.GetSize(); i ++ )
	{
		EParticleObject *	pParticle = m_lstParticle.GetAt( i ) ;
		if ( pParticle == NULL )
		{
			continue ;
		}
		if ( pParticle->m_imgMask.GetInfo() != NULL )
		{
			rp.dwTransparency = 0 ;
		}
		rp.pSrcImage = pParticle->GetPortionImage( ) ;
		rp.rev.vRenderPos = pParticle->m_vCurrentPos ;
		rp.rev.vRenderPos.z += hRenderPoly->GetScreenPos().z ;
		rp.rev.vRevCenter.x = (REAL32) pParticle->m_ptRevCenter.x ;
		rp.rev.vRevCenter.y = (REAL32) pParticle->m_ptRevCenter.y ;
		//
		double	rad ;
		revmat.InitializeMatrix( vUnit ) ;
		rad = pParticle->m_vCurrentRev.z * rPIby180 ;
		revmat.RevolveOnZ( sin( rad ), cos( rad ) ) ;
		rad = pParticle->m_vCurrentRev.x * rPIby180 ;
		revmat.RevolveOnX( sin( rad ), cos( rad ) ) ;
		rad = pParticle->m_vCurrentRev.y * rPIby180 ;
		revmat.RevolveOnY( sin( rad ), cos( rad ) ) ;
		//
		if ( !hRenderPoly->PrepareRenderParam( &rp ) )
		{
			hRenderPoly->RenderPolygon( ) ;
		}
	}
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSuperSprite::GetTypeName( void ) const
{
	return	L"SuperSprite" ;
}

ECSObject * ECSSuperSprite::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"SuperSprite" ) )
	{
		return	this ;
	}
	return	ECSSprite::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSSuperSprite::Duplicate( void )
{
	ECSSuperSprite *	pSprite = new ECSSuperSprite ;
	if ( GetInfo() != NULL )
	{
		pSprite->DuplicateImage( GetInfo(), 0 ) ;
	}
	PARAMETER	param ;
	GetParameter( param ) ;
	pSprite->SetParameter( param ) ;
	return	pSprite ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::GetFunction
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
ESLError ECSSuperSprite::CallFunction
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
			( "定義されていない SuperSprite 型のメンバ関数を呼び出しています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSSuperSprite::IndexAllMember( void )
{
	ECSSprite::IndexAllMember( ) ;
	m_refParticleImage.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::CommitAllReference( ECSContext & context )
{
	ESLError	err = ECSSprite::CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	err = m_refParticleImage.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	ECSResource *	pMaskImage =
		ESLTypeCast<ECSResource>( m_refParticleImage.m_pRef ) ;
	if ( pMaskImage != NULL )
	{
		m_refParticleImage.SetReference( pMaskImage, &context ) ;
		EGLAnimation *	pImage = pMaskImage->GetImage( ) ;
		if ( pImage != NULL )
		{
			m_efprm.pImageInf = *pImage ;
		}
		else
		{
			ECSSprite *	pSprite = ESLTypeCast<ECSSprite>( pMaskImage ) ;
			if ( pSprite != NULL )
			{
				m_efprm.pImageInf = *pSprite ;
			}
		}
	}
	if ( m_efprm.eftType != etMeshWarp )
	{
		EFFECT_PARAM	efprm = m_efprm ;
		SetEffectParameter( efprm ) ;
	}
	else
	{
		EGL_SIZE	sizeInMesh = m_sizeInMesh ;
		int			nMeshBytes =
			m_sizeInMesh.w * m_sizeInMesh.h * sizeof(E3D_VECTOR_2D) ;
		EStreamBuffer	bufMeshList, bufBaseMesh ;
		sizeInMesh.w -= 1 ;
		sizeInMesh.h -= 1 ;
		bufBaseMesh.Write( m_pBaseMesh, nMeshBytes ) ;
		bufMeshList.Write( m_pMeshList, nMeshBytes * m_nMeshListSize ) ;
		//
		SetMeshWarpEffect
			( (const E3D_VECTOR_2D *) bufMeshList.GetBuffer().GetBuffer(),
				m_nMeshListSize, sizeInMesh,
				(const E3D_VECTOR_2D *) bufBaseMesh.GetBuffer().GetBuffer() ) ;
	}
	return	err ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwBytes = sizeof(m_efprm) ;
	file.Write( &dwBytes, sizeof(dwBytes) ) ;
	if ( file.Write( &m_efprm, dwBytes ) < dwBytes )
	{
		return	eslErrGeneral ;
	}
	err = m_refParticleImage.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_efprm.eftType == etMeshWarp )
	{
		int	nMeshSize = m_sizeInMesh.w * m_sizeInMesh.h ;
		file.Write( &m_sizeInMesh, sizeof(m_sizeInMesh) ) ;
		file.Write( &m_nMeshListSize, sizeof(m_nMeshListSize) ) ;
		file.Write( m_pBaseMesh, nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
		file.Write( m_pMeshList, nMeshSize * m_nMeshListSize * sizeof(E3D_VECTOR_2D) ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSSprite::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwBytes ;
	if ( file.Read( &dwBytes, sizeof(dwBytes) ) != sizeof(dwBytes) )
	{
		return	eslErrGeneral ;
	}
	if ( dwBytes > sizeof(EFFECT_PARAM) )
	{
		return	eslErrGeneral ;
	}
	::eslFillMemory( &m_efprm, 0, sizeof(m_efprm) ) ;
	if ( file.Read( &m_efprm, dwBytes ) != dwBytes )
	{
		return	eslErrGeneral ;
	}
	//
	err = m_refParticleImage.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_efprm.eftType == etMeshWarp )
	{
		int	nMeshSize ;
		file.Read( &m_sizeInMesh, sizeof(m_sizeInMesh) ) ;
		nMeshSize = m_sizeInMesh.w * m_sizeInMesh.h ;
		file.Read( &m_nMeshListSize, sizeof(m_nMeshListSize) ) ;
		//
		m_bufBaseMesh.Delete( ) ;
		m_pBaseMesh = (E3D_VECTOR_2D*)
			m_bufBaseMesh.PutBuffer( nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
		file.Read( m_pBaseMesh, nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
		//
		m_pMeshList = (E3D_VECTOR_2D*)
			::eslHeapReallocate( NULL, m_pMeshList,
					nMeshSize * m_nMeshListSize * sizeof(E3D_VECTOR_2D), 0 ) ;
		file.Read( m_pMeshList, nMeshSize * m_nMeshListSize * sizeof(E3D_VECTOR_2D) ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	ECSSprite::DumpObject( buf, nIndent, context ) ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSSuperSprite::m_staFuncName = NULL ;
const wchar_t *	ECSSuperSprite::m_pwszFuncName[3] =
{
	L"SetEffectParameter",
	L"SetMeshWarpEffect",
	NULL
} ;
const ECSSuperSprite::PFUNC_CALL	ECSSuperSprite::m_pfnCallFunc[2] =
{
	&ECSSuperSprite::Call_SetEffectParameter,
	&ECSSuperSprite::Call_SetMeshWarpEffect,
} ;

// メンバ関数 : SetEffectParameter( EffectParam efprm[, Reference rMaskImage] )
//	EffectParam;
//		String	strType
//		Integer	nFlags
//		Integer	nInterval, nDegreeStep
//		Integer	nShakingWidth, nMeshSize, nMeshDivision, nFrequency
//		Size	sizeView
//		Point	ptSpeed
//		Integer	nAlphaRange, nMilliSecPerDegree
//		Point	ptSmashPoint
//		Real	rSmashDelay, rSmashPower, rRandomPower, rDeceleration
//		Vector	vVelocity, vGravity, vRevSpeed, vRevRandom
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::Call_SetEffectParameter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSStructure *	pefprm =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"EffectParam" ) ) ;
	if ( pefprm == NULL )
	{
		return	ESLErrorMsg( "引数に EffectParam が指定されていません。" ) ;
	}
	static const wchar_t *	pwszTypes[] =
	{
		L"Nothing", L"TileImage",
		L"FilterWhite", L"FilterLight", L"FilterBlack", L"FilterDark",
		L"RasterScroll", L"WaveCircle", L"Shimmer", L"MeshWarp",
		L"ShadingOff", L"ShadingLight",
		L"SmashParticle2D", L"SmashParticle3D",
		NULL
	} ;
	EFFECT_PARAM	efprm ;
	::eslFillMemory( &efprm, 0, sizeof(efprm) ) ;
	EWideString	wstrType = pefprm->GetMemberAsStr( L"strType", NULL ) ;
	for ( int i = 0; pwszTypes[i]; i ++ )
	{
		if ( !wstrType.CompareNoCase( pwszTypes[i] ) )
		{
			efprm.eftType = (EffectType) i ;
			break ;
		}
	}
	efprm.fdwFlags = pefprm->GetMemberAsInt( L"nFlags", 0 ) ;
	efprm.nInterval = pefprm->GetMemberAsInt( L"nInterval", 0 ) ;
	efprm.nDegreeStep = pefprm->GetMemberAsInt( L"nDegreeStep", 0 ) ;
	efprm.nShakingWidth = pefprm->GetMemberAsInt( L"nShakingWidth", 0 ) ;
	efprm.nMeshSize = pefprm->GetMemberAsInt( L"nMeshSize", 0 ) ;
	efprm.nMeshDivision = pefprm->GetMemberAsInt( L"nMeshDivision", 0 ) ;
	efprm.nFrequency = pefprm->GetMemberAsInt( L"nFrequency", 0 ) ;
	//
	ECSStructureInterface *	pSizeView =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"sizeView" ) ) ;
	if ( pSizeView != NULL )
	{
		efprm.sizeView.w = pSizeView->GetMemberAsInt( L"w", 0 ) ;
		efprm.sizeView.h = pSizeView->GetMemberAsInt( L"h", 0 ) ;
	}
	ECSStructureInterface *	pptSpeed =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"ptSpeed" ) ) ;
	if ( pptSpeed != NULL )
	{
		efprm.ptSpeed.x = pptSpeed->GetMemberAsInt( L"x", 0 ) ;
		efprm.ptSpeed.y = pptSpeed->GetMemberAsInt( L"y", 0 ) ;
	}
	//
	efprm.nAlphaRange = pefprm->GetMemberAsInt( L"nAlphaRange", 1 ) ;
	efprm.nMilliSecPerDegree =
		pefprm->GetMemberAsInt( L"nMilliSecPerDegree", 1000 ) ;
	//
	ECSStructureInterface *	pptSmashPoint =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"ptSmashPoint" ) ) ;
	if ( pptSmashPoint != NULL )
	{
		efprm.ptSmashPoint.x = pptSmashPoint->GetMemberAsInt( L"x", 0 ) ;
		efprm.ptSmashPoint.y = pptSmashPoint->GetMemberAsInt( L"y", 0 ) ;
	}
	efprm.rSmashDelay =
		(REAL32) pefprm->GetMemberAsReal( L"rSmashDelay", 0 ) ;
	efprm.rSmashPower =
		(REAL32) pefprm->GetMemberAsReal( L"rSmashPower", 0 ) ;
	efprm.rRandomPower =
		(REAL32) pefprm->GetMemberAsReal( L"rRandomPower", 0 ) ;
	efprm.rDeceleration =
		(REAL32) pefprm->GetMemberAsReal( L"rDeceleration", 0 ) ;
	//
	ECSStructureInterface *	pvVelocity =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"vVelocity" ) ) ;
	if ( pvVelocity != NULL )
	{
		efprm.vVelocity.x = (REAL32) pvVelocity->GetMemberAsReal( L"x", 0 ) ;
		efprm.vVelocity.y = (REAL32) pvVelocity->GetMemberAsReal( L"y", 0 ) ;
		efprm.vVelocity.z = (REAL32) pvVelocity->GetMemberAsReal( L"z", 0 ) ;
	}
	ECSStructureInterface *	pvGravity =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"vGravity" ) ) ;
	if ( pvGravity != NULL )
	{
		efprm.vGravity.x = (REAL32) pvGravity->GetMemberAsReal( L"x", 0 ) ;
		efprm.vGravity.y = (REAL32) pvGravity->GetMemberAsReal( L"y", 0 ) ;
		efprm.vGravity.z = (REAL32) pvGravity->GetMemberAsReal( L"z", 0 ) ;
	}
	ECSStructureInterface *	pvRevSpeed =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"vRevSpeed" ) ) ;
	if ( pvRevSpeed != NULL )
	{
		efprm.vRevSpeed.x = (REAL32) pvRevSpeed->GetMemberAsReal( L"x", 0 ) ;
		efprm.vRevSpeed.y = (REAL32) pvRevSpeed->GetMemberAsReal( L"y", 0 ) ;
		efprm.vRevSpeed.z = (REAL32) pvRevSpeed->GetMemberAsReal( L"z", 0 ) ;
	}
	ECSStructureInterface *	pvRevRandom =
		ESLTypeCast<ECSStructureInterface>( pefprm->GetMemberAs( L"vRevRandom" ) ) ;
	if ( pvRevRandom != NULL )
	{
		efprm.vRevRandom.x = (REAL32) pvRevRandom->GetMemberAsReal( L"x", 0 ) ;
		efprm.vRevRandom.y = (REAL32) pvRevRandom->GetMemberAsReal( L"y", 0 ) ;
		efprm.vRevRandom.z = (REAL32) pvRevRandom->GetMemberAsReal( L"z", 0 ) ;
	}
	//
	ECSResource *	pMaskImage =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 2, L"Resource" ) ) ;
	if ( pMaskImage != NULL )
	{
		m_refParticleImage.SetReference( pMaskImage, &context ) ;
		EGLAnimation *	pImage = pMaskImage->GetImage( ) ;
		if ( pImage != NULL )
		{
			efprm.pImageInf = *pImage ;
		}
		else
		{
			ECSSprite *	pSprite = ESLTypeCast<ECSSprite>( pMaskImage ) ;
			if ( pSprite != NULL )
			{
				efprm.pImageInf = *pSprite ;
			}
		}
	}
	//
	bool	fLock = (GetParent() != NULL) ;
	if ( fLock )
	{
		QuickLock( ) ;
	}
	SetEffectParameter( efprm ) ;
	if ( pMaskImage != NULL )
	{
		m_refParticleImage.SetReference( pMaskImage, &context ) ;
	}
	if ( fLock )
	{
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer SetMeshWarpEffect
//		( Array aMeshList, Integer nMeshListCount,
//			Integer nMeshWidth, Integer nMeshHeight [, Array aBaseMesh] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSuperSprite::Call_SetMeshWarpEffect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 5, 6 ) ;
	if ( err )
	{
		return	err ;
	}
	int	i, nMeshListCount, nMeshSize, nMeshWidth, nMeshHeight ;
	EStreamBuffer	bufMeshList, bufBaseMesh ;
	E3D_VECTOR_2D *	pvMeshList ;
	E3D_VECTOR_2D *	pvBaseMesh = NULL ;
	ECSPointer *	pMeshListPtr =
		ESLTypeCast<ECSPointer>
			( context.GetArgumentObjectAs( lstArg, 1, L"Pointer" ) ) ;
	ECSArray *	pMeshList = NULL ;
	if ( pMeshListPtr == NULL )
	{
		pMeshList = ESLTypeCast<ECSArray>
				( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
		if ( pMeshList == NULL )
		{
			return	ESLErrorMsg( "メッシュリストが指定されていません。" ) ;
		}
	}
	err = context.GetArgumentAsInt( nMeshListCount, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nMeshWidth, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nMeshHeight, lstArg, 4, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	nMeshSize = (nMeshWidth + 1) * (nMeshHeight + 1) ;
	pvMeshList = (E3D_VECTOR_2D*) bufMeshList.PutBuffer
		( nMeshSize * nMeshListCount * sizeof(E3D_VECTOR_2D) ) ;
	if ( pMeshListPtr != NULL )
	{
		const DWORD	dwMeshListElements = nMeshSize * nMeshListCount ;
		const DWORD	dwMeshListBytes = dwMeshListElements * sizeof(double) * 2 ;
		double *	pMeshListData =
			(double*) pMeshListPtr->GetBuffer( 0, dwMeshListBytes, true ) ;
		if ( pMeshListData != NULL )
		{
			for ( i = 0; i < (int) dwMeshListElements; i ++ )
			{
				pvMeshList[i].x = (REAL32) pMeshListData[i+i] ;
				pvMeshList[i].y = (REAL32) pMeshListData[i+i+1] ;
			}
			pMeshListPtr->FlushBuffer
					( 0, dwMeshListBytes, pMeshListData, true ) ;
		}
	}
	else
	{
		if ( (int) pMeshList->m_varArray.GetSize() < nMeshSize * nMeshListCount * 2 )
		{
			return	context.PushObject( context.new_CSInteger( eslErrGeneral ) ) ;
		}
		for ( i = 0; i < nMeshSize * nMeshListCount; i ++ )
		{
			ECSObject *	pX = pMeshList->m_varArray.GetAt( i * 2 ) ;
			if ( (pX != NULL) && (pX->m_vtType == csvtReal) )
			{
				pvMeshList[i].x = (REAL32) ((ECSReal*)pX)->m_varReal ;
			}
			ECSObject *	pY = pMeshList->m_varArray.GetAt( i * 2 + 1 ) ;
			if ( (pY != NULL) && (pY->m_vtType == csvtReal) )
			{
				pvMeshList[i].y = (REAL32) ((ECSReal*)pY)->m_varReal ;
			}
		}
	}
	ECSPointer *	pBaseMeshPtr =
		ESLTypeCast<ECSPointer>
			( context.GetArgumentObjectAs( lstArg, 5, L"Pointer" ) ) ;
	if ( pBaseMeshPtr != NULL )
	{
		const DWORD	dwMeshBytes = nMeshSize * sizeof(double) * 2 ;
		double *	pMeshBaseData =
			(double*) pBaseMeshPtr->GetBuffer( 0, dwMeshBytes, true ) ;
		if ( pMeshBaseData != NULL )
		{
			pvBaseMesh = (E3D_VECTOR_2D*)
				bufBaseMesh.PutBuffer( nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
			for ( i = 0; i < nMeshSize; i ++ )
			{
				pvBaseMesh[i].x = (REAL32) pMeshBaseData[i+i] ;
				pvBaseMesh[i].y = (REAL32) pMeshBaseData[i+i+1] ;
			}
			pBaseMeshPtr->FlushBuffer
					( 0, dwMeshBytes, pMeshBaseData, true ) ;
		}
	}
	else
	{
		ECSArray *	pBaseMesh =
			ESLTypeCast<ECSArray>
				( context.GetArgumentObjectAs( lstArg, 5, L"Array" ) ) ;
		if ( pBaseMesh != NULL )
		{
			if ( (int) pBaseMesh->m_varArray.GetSize() < nMeshSize * 2 )
			{
				return	context.PushObject( context.new_CSInteger( eslErrGeneral ) ) ;
			}
			pvBaseMesh = (E3D_VECTOR_2D*)
				bufBaseMesh.PutBuffer( nMeshSize * sizeof(E3D_VECTOR_2D) ) ;
			for ( i = 0; i < nMeshSize; i ++ )
			{
				ECSObject *	pX = pBaseMesh->m_varArray.GetAt( i * 2 ) ;
				if ( (pX != NULL) && (pX->m_vtType == csvtReal) )
				{
					pvBaseMesh[i].x = (REAL32) ((ECSReal*)pX)->m_varReal ;
				}
				ECSObject *	pY = pBaseMesh->m_varArray.GetAt( i * 2 + 1 ) ;
				if ( (pY != NULL) && (pY->m_vtType == csvtReal) )
				{
					pvBaseMesh[i].y = (REAL32) ((ECSReal*)pY)->m_varReal ;
				}
			}
		}
	}
	bool	fLock = (GetParent() != NULL) ;
	if ( fLock )
	{
		QuickLock( ) ;
	}
	SetMeshWarpEffect
		( pvMeshList, nMeshListCount,
			EGLSize( nMeshWidth, nMeshHeight ), pvBaseMesh ) ;
	if ( fLock )
	{
		QuickUnlock( ) ;
	}
	return	context.PushObject( context.new_CSInteger( eslErrSuccess ) ) ;
}
