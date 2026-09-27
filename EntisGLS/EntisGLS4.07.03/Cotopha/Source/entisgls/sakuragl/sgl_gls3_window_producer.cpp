
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/sgl_gls3_window_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// SGLImageBufferInterface の EGL_IMAGE_INFO 実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLImageE3DTexture, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageE3DTexture::SGLImageE3DTexture( void )
{
	m_typeObject = imageObjectEntisGLS3Texture ;
	m_pTexture = NULL ;
	m_flagUpdate = true ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageE3DTexture::~SGLImageE3DTexture( void )
{
	if ( m_pTexture != NULL )
	{
		eglDeleteImageBuffer( m_pTexture ) ;
		m_pTexture = NULL ;
	}
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageE3DTexture::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	m_flagUpdate = true ;
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageE3DTexture::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	m_flagUpdate = false ;
	if ( m_pTexture == NULL )
	{
		CreateTexture( pImageBuf ) ;
	}
	return	sglErrSuccess ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageE3DTexture::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageE3DTexture::MakeMipmap( void )
{
	return	sglErrSuccess ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool SGLImageE3DTexture::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}

// テクスチャ生成
//////////////////////////////////////////////////////////////////////////////
void SGLImageE3DTexture::CreateTexture( SGLImageBuffer * pImageBuf )
{
	if ( m_pTexture == NULL )
	{
		EGL_IMAGE_INFO	eglInf ;
		SGLImageBuffer2EGL( eglInf, *pImageBuf ) ;
		if ( pImageBuf->ptrBuffer != NULL )
		{
			m_pTexture = eglCreateTextureInfo( &eglInf ) ;
		}
		else
		{
			m_pTexture =
				eglCreateImageBuffer
					( eglInf.fdwFormatType,
						eglInf.dwImageWidth,
						eglInf.dwImageHeight,
						eglInf.dwBitsPerPixel ) ;
		}
	}
}

// SGLImageInfo から EGL_IMAGE_INFO へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageE3DTexture::SGLImageInfo2EGL
	( EGL_IMAGE_INFO& eglInf, const SGLImageInfo& sglInf )
{
	eglInf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
	eglInf.fdwFormatType = sglInf.format ;
	eglInf.ptrOffsetPixel = 0 ;
	eglInf.ptrImageArray = NULL ;
	eglInf.pPaletteEntries = NULL ;
	eglInf.dwPaletteCount = 0 ;
	eglInf.dwImageWidth = sglInf.width ;
	eglInf.dwImageHeight = sglInf.height ;
	eglInf.dwBitsPerPixel = sglInf.depth ;
	eglInf.dwBytesPerLine = sglInf.pitchLine ;
	eglInf.dwSizeOfImage = sglInf.pitchLine * sglInf.height ;
	eglInf.dwClippedPixel = 0 ;
}

// SGLImageBuffer から EGL_IMAGE_INFO へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLImageE3DTexture::SGLImageBuffer2EGL
	( EGL_IMAGE_INFO& eglInf, const SGLImageBuffer& sglBuf )
{
	SGLImageInfo2EGL( eglInf, sglBuf ) ;
	//
	eglInf.ptrImageArray = sglBuf.ptrBuffer ;
	eglInf.pPaletteEntries = (PEGL_PALETTE) sglBuf.ptrPalette ;
	if ( (eglInf.pPaletteEntries != NULL) && (sglBuf.depth <= 8) )
	{
		eglInf.dwPaletteCount = (1 << sglBuf.depth) ;
	}
}

// SGLImageObject から PEGL_IMAGE_INFO テクスチャ取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO
	SGLImageE3DTexture::CommitE3DTexture( SGLImageObject * pImage )
{
	SGLImageRect	rectRef ;
	SGLImageBufferInterface *
		pObject = pImage->CommitImageObject
						( imageObjectEntisGLS3Texture, rectRef, true ) ;
	if ( pObject == NULL )
	{
		pImage->AddImageObject( new SGLImageE3DTexture, false ) ;
		pObject = pImage->CommitImageObject
					( imageObjectEntisGLS3Texture, rectRef, true ) ;
	}
	SGLImageE3DTexture *
		pTexture = ESLTypeCast<SGLImageE3DTexture>( pObject ) ;
	if ( pTexture != NULL )
	{
		return	pTexture->m_pTexture ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// S3DMaterialBuffer の E3D_SURFACE_ATTRIBUTE 実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DMaterialE3DSurfaceAttribute, S3DMaterialBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMaterialE3DSurfaceAttribute::S3DMaterialE3DSurfaceAttribute( void )
{
	m_flagBack = false ;
	m_typeMaterial = imageObjectEntisGLS3Texture ;
	eslFillMemory( &m_sufAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
	eslFillMemory( &m_sufBackAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
}

// 情報変換
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialE3DSurfaceAttribute::UpdateMaterial( const S3DMaterial * pMaterial )
{
	if ( m_flagUpdate )
	{
		S3DSurfaceAttribute	sglAttr ;
		pMaterial->GetSurfaceAttribute( sglAttr ) ;
		SGLSurfaceAttribute2EGL( m_sufAttr, sglAttr ) ;
		//
		SGLImageObject *	pImage ;
		if ( sglAttr.flagsShading & shadingEnvironmentMapping )
		{
			m_sufAttr.envmap.pUpperImage = NULL ;
			m_sufAttr.envmap.pUnderImage = NULL ;
			m_sufAttr.envmap.dwFlags = 0 ;
			//
			pImage = pMaterial->GetTexture() ;
			if ( pImage != NULL )
			{
				m_sufAttr.envmap.pUpperImage =
					SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
				m_sufAttr.envmap.pUnderImage =
						m_sufAttr.envmap.pUpperImage ;
			}
		}
		else if ( sglAttr.flagsShading & shadingTextureMapping )
		{
			m_sufAttr.txmap.pTextureImage = NULL ;
			m_sufAttr.txmap.pSmallImage = NULL ;
			m_sufAttr.txmap.pLuminousImage = NULL ;
			m_sufAttr.txmap.pSmallLuminous = NULL ;
			m_sufAttr.txmap.nSmallScale = 0 ;
			m_sufAttr.txmap.rThresholdZ = 0 ;
			//
			pImage = pMaterial->GetTexture() ;
			if ( pImage != NULL )
			{
				m_sufAttr.txmap.pTextureImage =
					SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
				m_sufAttr.txmap.pSmallImage =
						m_sufAttr.txmap.pTextureImage ;
			}
			for ( int i = 1; i < 4; i ++ )
			{
				pImage = pMaterial->GetTexture( i ) ;
				if ( (pImage != NULL) &&
					(pMaterial->GetTextureType(i) == S3DMaterial::textureSub) )
				{
					m_sufAttr.txmap.pSmallImage =
						SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
					m_sufAttr.txmap.nSmallScale = i ;
					m_sufAttr.txmap.rThresholdZ =
						(REAL32) (pMaterial->GetSubTextureZ()
									* pow( 2, (double) (i - 1) )) ;
					break ;
				}
			}
			int	iLuminous =
					pMaterial->FindTextureTypeOf
						( S3DMaterial::textureLuminous ) ;
			if ( iLuminous >= 0 )
			{
				pImage = pMaterial->GetTexture( iLuminous ) ;
				if ( pImage != NULL )
				{
					m_sufAttr.txmap.pLuminousImage =
						SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
					m_sufAttr.txmap.pSmallLuminous = m_sufAttr.txmap.pLuminousImage ;
					m_sufAttr.nLuminousApply =
						eslRoundR32ToInt( 0x100 * pMaterial->GetTextureApplication(iLuminous)) ;
				}
			}
		}
		m_flagBack = pMaterial->IsEnabledBackSurfaceAttribute() ;
		if ( m_flagBack )
		{
			pMaterial->GetBackSurfaceAttribute( sglAttr ) ;
			SGLSurfaceAttribute2EGL( m_sufBackAttr, sglAttr ) ;
			//
			if ( sglAttr.flagsShading & shadingEnvironmentMapping )
			{
				m_sufBackAttr.envmap.pUpperImage = NULL ;
				m_sufBackAttr.envmap.pUnderImage = NULL ;
				m_sufBackAttr.envmap.dwFlags = 0 ;
				//
				pImage = pMaterial->GetBackTexture() ;
				if ( pImage != NULL )
				{
					m_sufBackAttr.envmap.pUpperImage =
						SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
					m_sufBackAttr.envmap.pUnderImage =
							m_sufBackAttr.envmap.pUpperImage ;
				}
			}
			else if ( sglAttr.flagsShading & shadingTextureMapping )
			{
				m_sufBackAttr.txmap.pTextureImage = NULL ;
				m_sufBackAttr.txmap.pSmallImage = NULL ;
				m_sufBackAttr.txmap.pLuminousImage = NULL ;
				m_sufBackAttr.txmap.pSmallLuminous = NULL ;
				m_sufBackAttr.txmap.nSmallScale = 0 ;
				m_sufBackAttr.txmap.rThresholdZ = 0 ;
				//
				pImage = pMaterial->GetBackTexture() ;
				if ( pImage != NULL )
				{
					m_sufBackAttr.txmap.pTextureImage =
						SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
					m_sufBackAttr.txmap.pSmallImage =
							m_sufBackAttr.txmap.pTextureImage ;
				}
				for ( int i = 1; i < 4; i ++ )
				{
					pImage = pMaterial->GetBackTexture( i ) ;
					if ( pImage != NULL )
					{
						m_sufBackAttr.txmap.pSmallImage =
							SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
						m_sufBackAttr.txmap.nSmallScale = i ;
						m_sufBackAttr.txmap.rThresholdZ =
							(REAL32) (pMaterial->GetSubTextureZ()
										* pow( 2, (double) (i - 1) )) ;
						break ;
					}
				}
			}
		}
		m_flagUpdate = false ;
	}
}

// S3DSurfaceAttribute から E3D_SURFACE_ATTRIBUTE へ変換
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialE3DSurfaceAttribute::SGLSurfaceAttribute2EGL
	( E3D_SURFACE_ATTRIBUTE& eglAttr, const S3DSurfaceAttribute& sglAttr )
{
	eglAttr.dwShadingFlags = (DWORD) sglAttr.flagsShading ;
	eglAttr.dwReserved = 0 ;
	eglAttr.nLuminousApply = 0 ;
	eglAttr.rgbaColor.rgbMul.dwPixelCode = sglAttr.colorBase.rgbMul.ui32 ;
	eglAttr.rgbaColor.rgbAdd.dwPixelCode = sglAttr.colorBase.rgbAdd.ui32 ;
	eglAttr.rgbaShade.rgbMul.dwPixelCode = sglAttr.colorShade.rgbMul.ui32 ;
	eglAttr.rgbaShade.rgbAdd.dwPixelCode = sglAttr.colorShade.rgbAdd.ui32 ;
	eglAttr.nAmbient = sglAttr.nAmbient ;
	eglAttr.nDiffusion = sglAttr.nDiffusion ;
	eglAttr.nSpecular = sglAttr.nSpecular ;
	eglAttr.nSpecularSize = sglAttr.nSpecularSize ;
	eglAttr.nTransparency = sglAttr.nTransparency ;
	eglAttr.nTransparency = sglAttr.nTransparency ;
	eglAttr.nDeepness = sglAttr.nDeepness ;
	eglAttr.nReflection = sglAttr.nReflection ;
}


//////////////////////////////////////////////////////////////////////////////
// HEGL_RENDER_POLYGON S3DRenderContextInterface 変換
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLRenderPolygonInterface, S3DRenderParameterContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLRenderPolygonInterface::SGLRenderPolygonInterface( HEGL_RENDER_POLYGON render )
{
	m_render.SetViewPoint( E3DVector( 0, 0, 0 ), E3DVector( 0, 0, 1 ), 0 ) ;
	m_flagAnyPaint = false ;
	m_flagAnyPaintZ = false ;
	m_flagQueuePolygon = false ;
	AttachRenderPolygon( render ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLRenderPolygonInterface::~SGLRenderPolygonInterface( void )
{
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::AttachRenderPolygon( HEGL_RENDER_POLYGON render )
{
	m_hRender = render ;
	m_hDraw = NULL ;
	//
	m_pinfTarget = NULL ;
	m_pinfZBuf = NULL ;
	m_pDstViewRect = NULL ;
	//
	if ( render != NULL )
	{
		m_hDraw = render->GetDrawImage() ;
		//
		m_vScreenPos = m_hRender->GetScreenPos() ;
		m_vProjectionScreen.x = m_vScreenPos.x ;
		m_vProjectionScreen.y = m_vScreenPos.y ;
		m_vProjectionScreen.z = m_vScreenPos.z ;
		m_zProjectionScale = 1.0 ;
		//
		DWORD	dwRenderParam = render->GetFunctionFlags() ;
		DWORD	dwDrawParam = m_hDraw->GetFunctionFlags() ;
		if ( (dwDrawParam & EGL_SMOOTH_STRETCH)
			| (dwRenderParam & E3D_FLAG_TEXTURE_SMOOTHING) )
		{
			m_flagsDefPaint |= paintSmoothStretch ;
		}
		if ( dwDrawParam & EGL_UNSMOOTH_STRETCH )
		{
			m_flagsDefPaint |= paintUnsmoothStretch ;
		}
		m_optContext.nShadingFlags &= ~shadingMethodMask ;
		if ( dwRenderParam & E3D_FLAG_PHONG_SHADING )
		{
			m_optContext.nShadingFlags |= shadingMethodPhong ;
		}
		if ( dwRenderParam & E3D_FLAG_RAY_SHADOWING )
		{
			m_optContext.nShadingFlags |= shadingMethodRayShadowing ;
		}
		if ( dwRenderParam & E3D_FLAG_RAY_REFLECTING )
		{
			m_optContext.nShadingFlags |= shadingMethodRayReflecting ;
		}
		if ( dwRenderParam & E3D_FLAG_RAY_REFRACTING )
		{
			m_optContext.nShadingFlags |= shadingMethodRayRefracting ;
		}
		//
		EGL_DRAW_DEST	ddst ;
		if ( !m_hDraw->GetDestination( &ddst ) )
		{
			if ( ddst.pDstImage != NULL )
			{
				m_render.Initialize
					( ddst.pDstImage, &ddst.rectDstClip , ddst.pZBuffer,
						&m_vScreenPos, 0x100000, 0x1000, E3DRenderPolygon::mrfAuto ) ;
			}
		}
	}
	m_render.SetSortingFlags( E3D_SORT_TRANSPARENT ) ;
}

// 描画機能フラグを更新
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::ReflectFunctionFlags( void )
{
	DWORD	dwDrawParam = 0 ;
	DWORD	dwRenderParam = 0 ;
	if ( m_flagsDefPaint & paintUnsmoothStretch )
	{
		dwDrawParam = EGL_UNSMOOTH_STRETCH ;
	}
	else if ( m_flagsDefPaint & paintSmoothStretch )
	{
		dwDrawParam = EGL_SMOOTH_STRETCH ;
		dwRenderParam = E3D_FLAG_TEXTURE_SMOOTHING ;
	}
	if ( m_optContext.nShadingFlags & shadingMethodPhong )
	{
		dwRenderParam |= E3D_FLAG_PHONG_SHADING ;
	}
	if ( m_optContext.nShadingFlags & shadingMethodRayShadowing )
	{
		dwRenderParam |= E3D_FLAG_RAY_SHADOWING ;
	}
	if ( m_optContext.nShadingFlags & shadingMethodRayReflecting )
	{
		dwRenderParam |= E3D_FLAG_RAY_REFLECTING ;
	}
	if ( m_optContext.nShadingFlags & shadingMethodRayRefracting )
	{
		dwRenderParam |= E3D_FLAG_RAY_REFRACTING ;
	}
	m_hDraw->SetFunctionFlags( dwDrawParam ) ;
	m_hRender->SetFunctionFlags( dwRenderParam ) ;
	if ( m_render.CreateRenderingObject() != NULL )
	{
		m_render.SetFunctionFlags( dwRenderParam ) ;
	}
}

// 光源設定を反映
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::ReflectLightEntries( void )
{
	m_bufLightEntries.SetLength( 0 ) ;
	m_bufShadowMap.SetLength( 0 ) ;
	//
	const size_t	countVector = m_arrayVectorLights.GetLength() ;
	size_t	i ;
	for ( i = 0; i < countVector; i ++ )
	{
		E3D_LIGHT_ENTRY			eglLight ;
		const S3DLightEntry&	light = m_arrayVectorLights.At( i ) ;
		S3DVector	vLight =
			m_matCamera * S3DDVector( light.vecDirection ) ;
		eglLight.dwLightType = light.typeLight ;
		eglLight.rgbColor.dwPixelCode = light.rgbColor.ui32 ;
		eglLight.rBrightness = light.fpBrightness ;
		eglLight.vecLight.x = vLight.x ;
		eglLight.vecLight.y = vLight.y ;
		eglLight.vecLight.z = vLight.z ;
		//
		if ( light.typeLight & lightShadowMapping )
		{
			ReflectShadowMapInfo( eglLight, light ) ;
		}
		m_bufLightEntries.Add( eglLight ) ;
	}
	const size_t	countPoints = m_arrayPointLights.GetLength() ;
	for ( i = 0; i < countPoints; i ++ )
	{
		E3D_LIGHT_ENTRY			eglLight ;
		const S3DLightEntry&	light = m_arrayPointLights.At( i ) ;
		S3DVector	vLight =
			m_matCamera * S3DDVector( light.vecPosition ) - m_vCameraPos ;
		eglLight.dwLightType = light.typeLight ;
		eglLight.rgbColor.dwPixelCode = light.rgbColor.ui32 ;
		eglLight.rBrightness = light.fpBrightness ;
		eglLight.vecLight.x = vLight.x ;
		eglLight.vecLight.y = vLight.y ;
		eglLight.vecLight.z = vLight.z ;
		//
		if ( light.typeLight & lightShadowMapping )
		{
			ReflectShadowMapInfo( eglLight, light ) ;
		}
		m_bufLightEntries.Add( eglLight ) ;
	}
	if ( m_rgbAmbient.ui32 & 0x00FFFFFF )
	{
		E3D_LIGHT_ENTRY	eglLight ;
		eglLight.dwLightType = E3D_AMBIENT_LIGHT ;
		eglLight.rgbColor.dwPixelCode = m_rgbAmbient.ui32 ;
		eglLight.rBrightness = 1.0f ;
		//
		m_bufLightEntries.Add( eglLight ) ;
	}
	if ( m_flagFog )
	{
		E3D_LIGHT_ENTRY	eglLight ;
		eglLight.dwLightType = E3D_FOG_LIGHT ;
		eglLight.rgbColor.dwPixelCode = m_rgbFogColor.ui32 ;
		eglLight.rFogDeepness = (REAL32) (m_zFogFar - m_zFogNear) ;
		eglLight.rFogDistance = (REAL32) m_zFogNear ;
		//
		m_bufLightEntries.Add( eglLight ) ;
	}
	m_hRender->PrepareLight
		( m_render.GetStackHeap(),
			m_bufLightEntries.GetLength(),
						m_bufLightEntries.GetConstArray() ) ;
	m_render.SetLightEntries
		( m_bufLightEntries.GetLength(),
						m_bufLightEntries.GetConstArray() ) ;
}

// シャドウマッピングを設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::ReflectShadowMapInfo
		( E3D_LIGHT_ENTRY& eglLight, const S3DLightEntry& light )
{
	if ( light.typeLight & lightShadowMapping )
	{
		ShadowMapEntry *	psmeMatch = NULL ;
		for ( size_t i = 0; i < m_arrayShadowMapInf.GetLength(); i ++ )
		{
			ShadowMapEntry *	psme = m_arrayShadowMapInf.GetAt( i ) ;
			if ( psme && (psme->idLight == light.nReserved2[0]) )
			{
				psmeMatch = psme ;
				break ;
			}
		}
		if ( psmeMatch && psmeMatch->pDepth )
		{
			SGLImageObject *	pImage = psmeMatch->pDepth ;
			S3DShadowMapInfo *	pShadowMap = &(psmeMatch->smiMapInfo) ;
			//
			E3D_SHADOW_MAP_INFO *	pMapInfo = new E3D_SHADOW_MAP_INFO ;
			m_bufShadowMap.Add( pMapInfo ) ;
			//
			S3DVector	vLightPos =
				m_matCamera * S3DDVector( pShadowMap->vLight ) - m_vCameraPos ;
			S3DVector	vLightRay =
				m_matCamera * S3DDVector( pShadowMap->vRay ) ;
			S3DVector	vOriginPos =
				m_matCamera * S3DDVector( pShadowMap->vTarget ) - m_vCameraPos ;
			S3DVector	vAxisX =
				m_matCamera * S3DDVector( pShadowMap->vAxisX ) ;
			S3DVector	vAxisY =
				m_matCamera * S3DDVector( pShadowMap->vAxisY ) ;
			//
			vLightRay.Normalize() ;
			//
			eglLight.dwLightType |= E3D_LIGHT_SHADOW_MAP ;
			eglLight.ShadowMap.pMapInfo = pMapInfo ;
			//
			pMapInfo->vLightPos.x = vLightPos.x ;
			pMapInfo->vLightPos.y = vLightPos.y ;
			pMapInfo->vLightPos.z = vLightPos.z ;
			//
			pMapInfo->vOriginPos.x = vOriginPos.x ;
			pMapInfo->vOriginPos.y = vOriginPos.y ;
			pMapInfo->vOriginPos.z = vOriginPos.z ;
			//
			pMapInfo->vLightRay.x = vLightRay.x ;
			pMapInfo->vLightRay.y = vLightRay.y ;
			pMapInfo->vLightRay.z = vLightRay.z ;
			//
			pMapInfo->vAxisX.x = vAxisX.x ;
			pMapInfo->vAxisX.y = vAxisX.y ;
			pMapInfo->vAxisX.z = vAxisX.z ;
			//
			pMapInfo->vAxisY.x = vAxisY.x ;
			pMapInfo->vAxisY.y = vAxisY.y ;
			pMapInfo->vAxisY.z = vAxisY.z ;
			//
			SGLImageInfo	imginf ;
			pImage->GetImageInfo( imginf ) ;
			//
			pMapInfo->pShadowMap =
					SGLImageE3DTexture::CommitE3DTexture( pImage ) ;
			//
			pMapInfo->rFixErrorGap = pShadowMap->fpFixErrorGap ;
			pMapInfo->rVarErrorGap = pShadowMap->fpVarErrorGap ;
		}
		else
		{
			eglLight.dwLightType &= ~E3D_LIGHT_SHADOW_MAP ;
			eglLight.ShadowMap.pMapInfo = NULL ;
		}
	}
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::GetViewPort( SGLImageRect & rctView ) const
{
	EGL_DRAW_DEST	ddst ;
	if ( m_hDraw->GetDestination( &ddst ) )
	{
		return	sglErrFailed ;
	}
	rctView.x = ddst.rectDstClip.left ;
	rctView.y = ddst.rectDstClip.top ;
	rctView.w = ddst.rectDstClip.right - ddst.rectDstClip.left + 1 ;
	rctView.h = ddst.rectDstClip.bottom - ddst.rectDstClip.top + 1 ;
	return	sglErrSuccess ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AttachTargetImage
	( SGLImageObject * pImage,
		SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pinfTarget = NULL ;
	m_pinfZBuf = NULL ;
	m_pDstViewRect = NULL ;
	//
	if ( pImage != NULL )
	{
		SGLImageInfo	sglinf ;
		uint8_t *	pbytBuf = pImage->LockBuffer( sglinf ) ;
		SGLImageE3DTexture::SGLImageInfo2EGL( m_infTarget, sglinf ) ;
		m_infTarget.ptrImageArray = pbytBuf ;
		m_pinfTarget = &m_infTarget ;
	}
	if ( pZBuffer != NULL )
	{
		SGLImageInfo	sglinf ;
		uint8_t *	pbytBuf = pZBuffer->LockBuffer( sglinf ) ;
		SGLImageE3DTexture::SGLImageInfo2EGL( m_infZBuffer, sglinf ) ;
		m_infZBuffer.ptrImageArray = pbytBuf ;
		m_pinfZBuf = &m_infZBuffer ;
	}
	if ( pView != NULL )
	{
		m_rctDstView.left = pView->x ;
		m_rctDstView.top = pView->y ;
		m_rctDstView.right = pView->x + pView->w - 1 ;
		m_rctDstView.bottom = pView->y + pView->h - 1 ;
		m_pDstViewRect = &m_rctDstView ;
	}
	if ( m_pinfTarget != NULL )
	{
		m_hRender->Initialize
			( m_pinfTarget, m_pDstViewRect, m_pinfZBuf, &m_vScreenPos ) ;
		m_render.Initialize
			( m_pinfTarget, m_pDstViewRect , m_pinfZBuf,
				&m_vScreenPos, 0x100000, 0x1000, E3DRenderPolygon::mrfAuto ) ;
	}
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	m_flagAnyPaint = false ;
	m_flagAnyPaintZ = false ;
	return	S3DRenderParameterContext::AttachTargetImage( pImage, pZBuffer, pView ) ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::DetachTargetImage( void )
{
	if ( m_pTarget != NULL )
	{
		const int	flagsUnlock =
			m_flagUpdateTarget ? SGLImageObject::lockReadWrite
									: SGLImageObject::lockRead ;
		m_pTarget->UnlockBuffer( flagsUnlock ) ;
		m_flagUpdateTarget = false ;
	}
	if ( m_pZBuffer != NULL )
	{
		const int	flagsUnlock =
			m_flagUpdateZBuf ? SGLImageObject::lockReadWrite
									: SGLImageObject::lockRead ;
		m_pZBuffer->UnlockBuffer( flagsUnlock ) ;
		m_flagUpdateZBuf = false ;
	}
	return	S3DRenderParameterContext::DetachTargetImage() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetPaintFlags( int64_t nFlags )
{
	S3DRenderParameterContext::SetPaintFlags( nFlags ) ;
	ReflectFunctionFlags() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::FillClearTarget
	( uint32_t argb, int64_t flags )
{
	EGL_DRAW_DEST	ddst ;
	if ( m_hDraw->GetDestination( &ddst ) )
	{
		return	sglErrFailed ;
	}
	if ( (flags == 0) || (flags & clearTargetColor) )
	{
		EGLPalette	colorFill( argb ) ;
		if ( !m_hDraw->PrepareFillRect
			( &(ddst.rectDstClip), colorFill, 0, 0 ) )
		{
			m_hDraw->FillRegion() ;
			m_flagUpdateTarget = true ;
		}
	}
	if ( (ddst.pZBuffer != NULL)
		&& ((flags == 0) || (flags & clearTargetZBuffer)) )
	{
		SGLImageBuffer	imgZBuf ;
		imgZBuf.format = ddst.pZBuffer->fdwFormatType ;
		imgZBuf.depth = ddst.pZBuffer->dwBitsPerPixel ;
		imgZBuf.width = ddst.pZBuffer->dwImageWidth ;
		imgZBuf.height = ddst.pZBuffer->dwImageHeight ;
		imgZBuf.colorClip = ddst.pZBuffer->dwClippedPixel ;
		imgZBuf.pitchPixel = imgZBuf.depth >> 3 ;
		imgZBuf.pitchLine = ddst.pZBuffer->dwBytesPerLine ;
		imgZBuf.ptrBuffer = (uint8_t*) ddst.pZBuffer->ptrImageArray ;
		//
		SGLPalette		zbufFill( 0x7F000000 ) ;
		SGLImageRect	rectFill
			( ddst.rectDstClip.left,
				ddst.rectDstClip.top,
				ddst.rectDstClip.right - ddst.rectDstClip.left + 1,
				ddst.rectDstClip.bottom - ddst.rectDstClip.top + 1 ) ;
		//
		sglFillImageBuffer( imgZBuf, zbufFill, &rectFill ) ;
		m_flagUpdateZBuf = true ;
	}
	return	sglErrSuccess ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::FillRectangle
	( int x, int y, int width, int height,
				uint32_t argb, double z, uint32_t flags )
{
	SGLAffine	af ;
	if ( GetAffineTransformation( af ) )
	{
		S2DVector	vertices[4] ;
		const float32_t	fpBias = (float32_t) 0x40 / (float32_t) 0x10000 ;
		if ( (flags | m_flagsDefPaint) & paintFixedPosition )
		{
			const float32_t	fpScale = (float32_t) (1.0 / 0x10000) ;
			vertices[0].x = (float32_t) x * fpScale - fpBias ;
			vertices[0].y = (float32_t) y * fpScale - fpBias ;
			vertices[2].x = (float32_t) (x + width) * fpScale - fpBias ;
			vertices[2].y = (float32_t) (y + height) * fpScale - fpBias ;
		}
		else
		{
			vertices[0].x = (float32_t) x - fpBias ;
			vertices[0].y = (float32_t) y - fpBias ;
			vertices[2].x = (float32_t) (x + width) - fpBias ;
			vertices[2].y = (float32_t) (y + height) - fpBias ;
		}
		vertices[1].x = vertices[2].x ;
		vertices[1].y = vertices[0].y ;
		vertices[3].x = vertices[0].x ;
		vertices[3].y = vertices[2].y ;
		//
		return	FillPolygon( vertices, 4, argb, z, flags ) ;
	}
	else
	{
		unsigned int	nTransparency = EffectTransparency( 0 ) ;
		if ( nTransparency != 0 )
		{
			argb = sglPackedColorMul( argb, 0x100 - nTransparency ) ;
		}
		EGL_RECT	rectFill ;
		if ( (flags | m_flagsDefPaint) & paintFixedPosition )
		{
			x += eslRoundR32ToInt( af.a13 * 0x10000 ) ;
			y += eslRoundR32ToInt( af.a23 * 0x10000 ) ;
			rectFill.left = ((x + 0xFF80) >> 16) ;
			rectFill.top = ((y + 0xFF80) >> 16) ;
			rectFill.right = ((x + width - 0x80) >> 16) ;
			rectFill.bottom = ((y + height - 0x80) >> 16) ;
		}
		else
		{
			x += eslRoundR32ToInt( af.a13 ) ;
			y += eslRoundR32ToInt( af.a23 ) ;
			rectFill.left = x ;
			rectFill.top = y ;
			rectFill.right = x + width - 1 ;
			rectFill.bottom = y + height - 1 ;
		}
		DWORD	dwFlags = EGL_DRAW_BLEND_ALPHA ;
		if ( (flags | m_flagsDefPaint) & paintNoBlendAlpha )
		{
			dwFlags = 0 ;
		}
		if ( m_hDraw->PrepareFillRect
			( &rectFill, EGLPalette( argb ), 0, dwFlags ) )
		{
			return	sglErrSuccess ;
		}
	}
	if ( m_hDraw->FillRegion() )
	{
		return	sglErrFailed ;
	}
	m_flagAnyPaint = true ;
	m_flagAnyPaintZ |= ((flags & paintWithZOrder) != 0) ;
	return	sglErrSuccess ;
}

SGLError SGLRenderPolygonInterface::FillPolygon
	( const S2DVector * vertices, size_t count,
				uint32_t argb, double z, uint32_t flags )
{
	unsigned int	nTransparency = EffectTransparency( 0 ) ;
	if ( nTransparency != 0 )
	{
		argb = sglPackedColorMul( argb, 0x100 - nTransparency ) ;
	}
	S2DVector *	pVertices ;
	m_bufPolygonVertices.SetLength( count ) ;
	pVertices = m_bufPolygonVertices.GetArray() ;
	//
	SGLAffine	af ;
	GetAffineTransformation( af ) ;
	af.TransformVectors( pVertices, vertices, count ) ;
	//
	EGL_POINT *	pPoints ;
	m_bufPolygonPoints.SetLength( count ) ;
	pPoints = m_bufPolygonPoints.GetArray() ;
	//
	for ( size_t i = 0; i < count; i ++ )
	{
		pPoints[i].x = eslRoundR32ToInt( pVertices[i].x ) ;
		pPoints[i].y = eslRoundR32ToInt( pVertices[i].y ) ;
	}
	m_bufPolygonVertices.FinishArray() ;
	m_bufPolygonPoints.FinishArray() ;
	//
	DWORD	dwFlags = EGL_DRAW_BLEND_ALPHA ;
	if ( (flags | m_flagsDefPaint) & paintNoBlendAlpha )
	{
		dwFlags = 0 ;
	}
	if ( m_hDraw->PrepareFillPolygon
			( pPoints, count, EGLPalette( argb ), 0, dwFlags ) )
	{
		return	sglErrSuccess ;
	}
	if ( m_hDraw->FillRegion() )
	{
		return	sglErrFailed ;
	}
	m_flagAnyPaint = true ;
	m_flagAnyPaintZ |= ((flags & paintWithZOrder) != 0) ;
	return	sglErrSuccess ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	EGL_DRAW_PARAM	dd ;
	EGL_RECT		rctSrcRect ;
	EGL_IMAGE_AXES	iaxAxes ;
	EGL_IMAGE_INFO	eglinfSrcImage ;
	SGLImageInfo	sglinfSrcImage ;
	SGLPalette		rgbPalette[0x100] ;
	//
	if ( pSrcImage == NULL )
	{
		return	sglErrFailed ;
	}
	uint8_t *	pbytSrcBuf =
		pSrcImage->LockBuffer
			( sglinfSrcImage, SGLImageObject::lockRead ) ;
	SGLImageE3DTexture::SGLImageInfo2EGL( eglinfSrcImage, sglinfSrcImage ) ;
	eglinfSrcImage.ptrImageArray = pbytSrcBuf ;
	//
	if ( sglinfSrcImage.depth <= 8 )
	{
		eglinfSrcImage.pPaletteEntries = (PEGL_PALETTE) &rgbPalette[0] ;
		eglinfSrcImage.dwPaletteCount =
				pSrcImage->GetPaletteTable
					( rgbPalette, (1 << sglinfSrcImage.depth) ) ;
	}
	//
	eslFillMemory( &dd, 0, sizeof(EGL_DRAW_PARAM) ) ;
	dd.dwFlags = ppPaint.nFlags ;
	if ( !(ppPaint.nFlags & paintNoBlendAlpha)
		&& (sglinfSrcImage.format & formatImageFlagAlpha) )
	{
		dd.dwFlags = EGL_DRAW_BLEND_ALPHA ;
	}
	dd.ptBasePos.x = ppPaint.ptPaint.x ;
	dd.ptBasePos.y = ppPaint.ptPaint.y ;
	if ( !(dd.dwFlags & EGL_FIXED_POSITION) )
	{
		dd.dwFlags |= EGL_FIXED_POSITION ;
		dd.ptBasePos.x <<= 16 ;
		dd.ptBasePos.y <<= 16 ;
	}
	dd.pSrcImage = &eglinfSrcImage ;
	dd.nTransparency = EffectTransparency( ppPaint.nTransparency ) ;
	dd.rZOrder = ppPaint.zOrder ;
	dd.rgbColorParam1.dwPixelCode = ppPaint.rgbColorParam.ui32 ;
	if ( pSrcClip != NULL )
	{
		rctSrcRect.left = pSrcClip->x ;
		rctSrcRect.top = pSrcClip->y ;
		rctSrcRect.right = pSrcClip->x + pSrcClip->w - 1 ;
		rctSrcRect.bottom = pSrcClip->y + pSrcClip->h - 1 ;
		dd.pViewRect = &rctSrcRect ;
	}
	SGLAffine	af ;
	GetAffineTransformation( af ) ;
	if ( ppPaint.pAffine != NULL )
	{
		af *= *ppPaint.pAffine ;
	}
	if ( af.IsRotation() )
	{
		iaxAxes.xAxis.x = af.a11 ;
		iaxAxes.xAxis.y = af.a21 ;
		iaxAxes.yAxis.x = af.a12 ;
		iaxAxes.yAxis.y = af.a22 ;
		dd.pImageAxes = &iaxAxes ;
	}
	dd.ptBasePos.x += eslRoundR32ToInt( af.a13 * 0x10000 ) ;
	dd.ptBasePos.y += eslRoundR32ToInt( af.a23 * 0x10000 ) ;
	//
	dd.nVertexCount = ppPaint.countVertex ;
	dd.pVertexPos = (PCE3D_VECTOR_2D) ppPaint.pVertices ;
	//
	if ( m_hDraw->PrepareDraw( &dd ) )
	{
		pSrcImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		return	sglErrSuccess ;
	}
	ESLError	err = m_hDraw->DrawImage() ;
	pSrcImage->UnlockBuffer( SGLImageObject::lockRead ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	m_flagAnyPaint = true ;
	m_flagAnyPaintZ |= ((ppPaint.nFlags & paintWithZOrder) != 0) ;
	return	sglErrSuccess ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::Flush( void )
{
	if ( m_flagQueuePolygon )
	{
		bool	fRayTracing = m_render.IsRayTracing() ;
		int		nThreadLines = 0 ;
		if ( fRayTracing )
		{
			if ( !(m_optContext.nShadingFlags & shadingMethodRayTracingByPixel) )
			{
				fRayTracing = false ;
				nThreadLines = 8 ;
			}
		}
		m_render.PrepareRendering() ;
		m_render.RenderAllPolygon( m_render, fRayTracing, nThreadLines ) ;
		m_render.FlushAllPolygon() ;
		m_flagQueuePolygon = false ;
		m_flagAnyPaint = true ;
		m_flagAnyPaintZ = true ;
		m_flagUpdateTarget = true ;
		m_flagUpdateZBuf = true ;
	}
	if ( m_flagUpdateTarget && (m_pTarget != NULL) )
	{
		m_pTarget->FlushBuffer() ;
		m_flagUpdateTarget = false ;
	}
	if ( m_flagUpdateZBuf && (m_pZBuffer != NULL) )
	{
		m_pZBuffer->FlushBuffer() ;
		m_flagUpdateZBuf = false ;
	}
	return	sglErrSuccess ;
}

SGLError SGLRenderPolygonInterface::Finish( void )
{
	return	SGLRenderPolygonInterface::Flush() ;
}

// 非同期レンダリング開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AsyncFlush
	( uint32_t nFlags, SSystem::SSignalEvent * pSignal )
{
	SGLError	err = SGLRenderPolygonInterface::Flush() ;
	if ( pSignal != NULL )
	{
		pSignal->SetSignal() ;
	}
	return	err ;
}

// 非同期レンダリング完了待機
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::WaitFlush( int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	m_vScreenPos.x = (REAL32) vScreen.x ;
	m_vScreenPos.y = (REAL32) vScreen.y ;
	m_vScreenPos.z = (REAL32) (vScreen.z * zScale) ;
	//
	if ( m_pinfTarget != NULL )
	{
		m_hRender->Initialize
			( m_pinfTarget, m_pDstViewRect, m_pinfZBuf, &m_vScreenPos ) ;
		m_render.Initialize
			( m_pinfTarget, m_pDstViewRect , m_pinfZBuf,
				&m_vScreenPos, 0x100000, 0x1000, E3DRenderPolygon::mrfAuto ) ;
	}
	else
	{
		EGL_DRAW_DEST	ddst ;
		if ( !m_hDraw->GetDestination( &ddst ) )
		{
			if ( ddst.pDstImage != NULL )
			{
				m_hRender->Initialize
					( ddst.pDstImage, &ddst.rectDstClip,
									ddst.pZBuffer, &m_vScreenPos ) ;
				m_render.Initialize
					( ddst.pDstImage, &ddst.rectDstClip, ddst.pZBuffer,
						&m_vScreenPos, 0x100000, 0x1000, E3DRenderPolygon::mrfAuto ) ;
			}
		}
	}
	return	S3DRenderParameterContext::SetProjectionScreen( vScreen, zScale ) ;
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetZClipRange( double zMin, double zMax )
{
	m_hRender->SetZClipRange( (REAL32) zMin, (REAL32) zMax ) ;
	m_render.SetZClipRange( (REAL32) zMin, (REAL32) zMax ) ;
	//
	S3DRenderParameterContext::SetZClipRange( zMin, zMax ) ;
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	S3DRenderParameterContext::SetLightEntries( pLights, countLight ) ;
	//
	ReflectLightEntries() ;
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMap,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	S3DRenderParameterContext::SetShadowMap
		( idLight, pShadowMap, infShadowMap, pShadowMapColor ) ;
	//
	ReflectLightEntries() ;
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	S3DRenderParameterContext::SetFog( rgbFog, zFogNear, zFogFar ) ;
	//
	ReflectLightEntries() ;
}

void SGLRenderPolygonInterface::EnableFog( bool fFog )
{
	S3DRenderParameterContext::EnableFog( fFog ) ;
	//
	ReflectLightEntries() ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetShadingFlag( uint64_t nShadingMethod )
{
	S3DRenderParameterContext::SetShadingFlag( nShadingMethod ) ;
	//
	ReflectFunctionFlags() ;
}

// レイトレーシング設定
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::SetRayTracingParameter
					( const S3DRenderRayTracingParam& rrtp )
{
	EGL_RENDER_RAY_TRACE_PARAM	eglrrtp ;
	eglrrtp.dwFlags = rrtp.nFlags ;
	eglrrtp.rShadowingDistance = rrtp.fpShadowingDistance ;
	eglrrtp.rRayTracingDistance = rrtp.fpRayTracingDistance ;
	eglrrtp.dwRayReflectCount = rrtp.nRayReflectionCount ;
	eglrrtp.dwAppendShadowingCount = rrtp.nAppendShadowingCount ;
	//
	m_hRender->SetRayTracingParameter( &eglrrtp ) ;
	m_render.SetRayTracingParameter( eglrrtp ) ;
	//
	S3DRenderParameterContext::SetRayTracingParameter( rrtp ) ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void SGLRenderPolygonInterface::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	eslFillMemory( &caps, 0, sizeof(S3DRenderingCapacity) ) ;
	caps.flagsRendering = S3DRenderingCapacity::renderingSortBuffered ;
	caps.flagsShading = S3DRenderingCapacity::shadingGouraud
						| S3DRenderingCapacity::shadingPhong
						| S3DRenderingCapacity::shadingShadowMapping
						| S3DRenderingCapacity::shadingRayTracing ;
	caps.typeHeadware = S3DRenderingCapacity::softwareEntisGLS3 ;
	caps.maxTextureSize = 0x4000 ;
	caps.maxTextureUnit = 1 ;
	caps.maxLightCount = 0x10000 ;
	caps.maxShadowmapCount = 0x10000 ;
	caps.flagsExtensions1 = S3DRenderingCapacity::extFramebuffer
						| S3DRenderingCapacity::extTextureNonPowerOf2 ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	S3DMaterialBuffer *	pMaterialBuf =
			pMaterial->GetMaterialBuffer( imageObjectEntisGLS3Texture ) ;
	if ( pMaterialBuf == NULL )
	{
		pMaterialBuf = new S3DMaterialE3DSurfaceAttribute ;
		pMaterial->AddMaterialBuffer( pMaterialBuf ) ;
	}
	S3DMaterialE3DSurfaceAttribute *	pSufAttr =
			ESLTypeCast<S3DMaterialE3DSurfaceAttribute>( pMaterialBuf ) ;
	if ( pSufAttr == NULL )
	{
		return	sglErrFailed ;
	}
	pSufAttr->UpdateMaterial( pMaterial ) ;
	//
	HSTACKHEAP	hHeap = m_render.GetStackHeap() ;
	DWORD		dwBytes =
		sizeof(E3D_PRIMITIVE_POLYGON) + sizeof(E3D_PRIMITIVE_MESH)
			+ (countVertex - 1) * sizeof(E3D_COLOR)
			+ sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (countPolygon - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	E3D_PRIMITIVE_POLYGON *
		pppMesh = (E3D_PRIMITIVE_POLYGON*)
						eslStackHeapAllocate( hHeap, dwBytes ) ;
	pppMesh->dwTypeFlag = E3D_SMOOTH_POLYGON | E3D_MESH_POLYGON ;
	pppMesh->pSurfaceAttr = &(pSufAttr->m_sufAttr) ;
	pppMesh->dwVertexCount = countVertex ;
	pppMesh->dwDataSize = dwBytes - 0x10 ;
	pppMesh->mesh.vertices =
		(PE3D_VECTOR4) eslStackHeapAllocate
							( hHeap, countVertex * sizeof(E3D_VECTOR4) ) ;
	pppMesh->mesh.normals =
		(PE3D_VECTOR4) eslStackHeapAllocate
							( hHeap, countVertex * sizeof(E3D_VECTOR4) ) ;
	//
	E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
		(E3D_PRIMITIVE_MESH_LIST*) &(pppMesh->mesh.uv_map[countVertex]) ;
	E3D_PRIMITIVE_MESH_POLY *	ppmpNext = &(ppmlMesh->mpEntries[0]) ;
	const uint32_t *			pNextList = pIndexedList ;
	ppmlMesh->dwMeshBytes =
		sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (countPolygon - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	ppmlMesh->dwPolyCount = countPolygon ;
	for ( size_t i = 0; i < countPolygon; i ++ )
	{
		ppmpNext->dwVertexCount = 3 ;
		ppmpNext->dwIndex[0] = pNextList[0] ;
		ppmpNext->dwIndex[1] = pNextList[1] ;
		ppmpNext->dwIndex[2] = pNextList[2] ;
		ppmpNext = (E3D_PRIMITIVE_MESH_POLY*) &(ppmpNext->dwIndex[3]) ;
		pNextList += 3 ;
	}
	//
	SGLError	err =
		AddPolygonMeshPrimitive
			( pppMesh, false, nFlags, pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	if ( pSufAttr->m_flagBack )
	{
		pppMesh->pSurfaceAttr = &(pSufAttr->m_sufBackAttr) ;
		err = AddPolygonMeshPrimitive
				( pppMesh, true, nFlags, pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	return	err ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	S3DMaterialBuffer *	pMaterialBuf =
			pMaterial->GetMaterialBuffer( imageObjectEntisGLS3Texture ) ;
	if ( pMaterialBuf == NULL )
	{
		pMaterialBuf = new S3DMaterialE3DSurfaceAttribute ;
		pMaterial->AddMaterialBuffer( pMaterialBuf ) ;
	}
	S3DMaterialE3DSurfaceAttribute *	pSufAttr =
			ESLTypeCast<S3DMaterialE3DSurfaceAttribute>( pMaterialBuf ) ;
	if ( pSufAttr == NULL )
	{
		return	sglErrFailed ;
	}
	pSufAttr->UpdateMaterial( pMaterial ) ;
	//
	HSTACKHEAP	hHeap = m_render.GetStackHeap() ;
	size_t		countVertex = countTriangleStrip + 2 ;
	DWORD		dwBytes =
		sizeof(E3D_PRIMITIVE_POLYGON) + sizeof(E3D_PRIMITIVE_MESH)
			+ (countVertex - 1) * sizeof(E3D_COLOR)
			+ sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (countTriangleStrip - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	E3D_PRIMITIVE_POLYGON *
		pppMesh = (E3D_PRIMITIVE_POLYGON*)
						eslStackHeapAllocate( hHeap, dwBytes ) ;
	pppMesh->dwTypeFlag = E3D_SMOOTH_POLYGON | E3D_MESH_POLYGON ;
	pppMesh->pSurfaceAttr = &(pSufAttr->m_sufAttr) ;
	pppMesh->dwVertexCount = countVertex ;
	pppMesh->dwDataSize = dwBytes - 0x10 ;
	pppMesh->mesh.vertices =
		(PE3D_VECTOR4) eslStackHeapAllocate
							( hHeap, countVertex * sizeof(E3D_VECTOR4) ) ;
	pppMesh->mesh.normals =
		(PE3D_VECTOR4) eslStackHeapAllocate
							( hHeap, countVertex * sizeof(E3D_VECTOR4) ) ;
	//
	E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
		(E3D_PRIMITIVE_MESH_LIST*) &(pppMesh->mesh.uv_map[countVertex]) ;
	E3D_PRIMITIVE_MESH_POLY *	ppmpNext = &(ppmlMesh->mpEntries[0]) ;
	ppmlMesh->dwMeshBytes =
		sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (countTriangleStrip - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	ppmlMesh->dwPolyCount = countTriangleStrip ;
	for ( size_t i = 0; i < countTriangleStrip; i ++ )
	{
		ppmpNext->dwVertexCount = 3 ;
		ppmpNext->dwIndex[0] = i ;
		ppmpNext->dwIndex[1] = i + 1 + (i & 0x01) ;
		ppmpNext->dwIndex[2] = i + 2 - (i & 0x01) ;
		ppmpNext = (E3D_PRIMITIVE_MESH_POLY*) &(ppmpNext->dwIndex[3]) ;
	}
	//
	SGLError	err =
		AddPolygonMeshPrimitive
			( pppMesh, false, nFlags, pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	if ( pSufAttr->m_flagBack )
	{
		pppMesh->pSurfaceAttr = &(pSufAttr->m_sufBackAttr) ;
		err = AddPolygonMeshPrimitive
				( pppMesh, true, nFlags, pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	return	err ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( typePrimitive == primitiveTriangle )
	{
		return	AddIndexedTriangleList
			( pMaterial, nFlags, countIndex / 3, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
	else if ( typePrimitive == primitiveTriangleStrip )
	{
		return	AddTriangleStrip
			( pMaterial, nFlags, countVertex - 2,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	return	sglErrInvalidParam ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	return	pBuffer->RenderBufferTo
				( this, 0, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// 座標要素が未確定の E3D_PRIMITIVE_POLYGON を追加する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLRenderPolygonInterface::AddPolygonMeshPrimitive
	( E3D_PRIMITIVE_POLYGON * pppMesh,
		bool fBackSurface, uint32_t nFlags,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	if ( pvVertex == NULL )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 頂点座標変換
	//
	S3DDMatrix	matdTransform ;
	S3DDVector	posdTransform ;
	GetTransformMatrix( matdTransform, posdTransform ) ;
	//
	S3DMatrix	matTransform = matdTransform ;
	S3DVector	posTransform = posdTransform ;
	const size_t	countVertex = pppMesh->dwVertexCount ;
	matTransform.RevolveVectors
		( (S3DVector4*) pppMesh->mesh.vertices,
			pvVertex, countVertex, posTransform ) ;
	//
	// 法線変換
	//
	if ( pvNormal != NULL )
	{
		S3DVector	vZero( 0, 0, 0 ) ;
		matTransform.RevolveVectors
			( (S3DVector4*) pppMesh->mesh.normals,
				pvNormal, countVertex, vZero ) ;
	}
	else
	{
		eslFillMemory
			( pppMesh->mesh.normals,
				0, countVertex * sizeof(S3DVector4) ) ;
		//
		E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
			(E3D_PRIMITIVE_MESH_LIST*) &(pppMesh->mesh.uv_map[countVertex]) ;
		E3D_PRIMITIVE_MESH_POLY *	ppmpNext = &(ppmlMesh->mpEntries[0]) ;
		const size_t	countPolygon = ppmlMesh->dwPolyCount ;
		S3DVector4 *	pNormalBuf = (S3DVector4*) pppMesh->mesh.normals ;
		//
		for ( size_t iPoly = 0; iPoly < countPolygon; ++ iPoly )
		{
			const DWORD	nPolyVertex = ppmpNext->dwVertexCount ;
			const DWORD	iIndex1 = ppmpNext->dwIndex[0] ;
			const DWORD	iIndex2 = ppmpNext->dwIndex[1] ;
			const DWORD	iIndex3 = ppmpNext->dwIndex[2] ;
			//
			// 表から見て頂点は反時計回り
			S3DVector	vNormal =
				(pNormalBuf[iIndex2] - pNormalBuf[iIndex1])
					* (pNormalBuf[iIndex3] - pNormalBuf[iIndex1]) ;
			vNormal.Normalize() ;
			//
			pNormalBuf[iIndex1] += vNormal ;
			pNormalBuf[iIndex2] += vNormal ;
			pNormalBuf[iIndex3] += vNormal ;
			//
			for ( DWORD i = 3; i < nPolyVertex; ++ i )
			{
				pNormalBuf[ppmpNext->dwIndex[i]] += vNormal ;
			}
		}
		S3DVector4 *	pNormals = pNormalBuf ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pNormals->Normalize() ;
			++ pNormals ;
		}
	}
	if ( fBackSurface )
	{
		PE3D_VECTOR4	pNormals = pppMesh->mesh.normals ;
		for ( size_t i = 0; i < countVertex; i ++ )
		{
			pNormals->x = - pNormals->x ;
			pNormals->y = - pNormals->y ;
			pNormals->z = - pNormals->z ;
			++ pNormals ;
		}
	}
	//
	// UV 座標と頂点色複製
	//
	if ( (pppMesh->pSurfaceAttr->dwShadingFlags
							& E3DSAF_TEXTURE_MAPPING) && (pvUVMap != NULL) )
	{
		pppMesh->dwTypeFlag |= E3D_TEXTURE_POLYGON ;
		eslMoveMemory
			( &(pppMesh->mesh.uv_map[0]),
				pvUVMap, countVertex * sizeof(S2DVector) ) ;
	}
	else if ( pColor != NULL )
	{
		pppMesh->dwTypeFlag |= E3D_VERTEX_COLOR_POLYGON ;
		eslMoveMemory
			( &(pppMesh->mesh.color[0]),
				pColor, countVertex * sizeof(S3DColor) ) ;
		pColor = NULL ;
	}
	//
	// 追加処理
	//
	unsigned int	nTransparency = 0 ;
	E3D_COLOR		colorEffect ;
	E3D_COLOR *		pcolorEffect = NULL ;
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		nTransparency = pTransform->nTransparency ;
		colorEffect.rgbMul.dwPixelCode = pTransform->colorEffect.rgbMul.ui32 ;
		colorEffect.rgbAdd.dwPixelCode = pTransform->colorEffect.rgbAdd.ui32 ;
		if ( ((colorEffect.rgbMul.dwPixelCode & 0x00FFFFFF) != 0xFFFFFF)
			| ((colorEffect.rgbAdd.dwPixelCode & 0x00FFFFFF) != 0) )
		{
			pcolorEffect = &colorEffect ;
		}
	}
	if ( m_render.AddPrimitive
		( pppMesh, pcolorEffect, nTransparency,
					0, (const E3D_COLOR *) pColor ) )
	{
		return	sglErrFailed ;
	}
	m_flagQueuePolygon = true ;
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS3 レンダラ・ウィンドウ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLEntisGLS3WindowProducer, SGLWindowViewProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLEntisGLS3WindowProducer::SGLEntisGLS3WindowProducer( void )
{
	m_hWnd = NULL ;
	m_flagZBuffer = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLEntisGLS3WindowProducer::~SGLEntisGLS3WindowProducer( void )
{
}

// 論理ビューポート取得
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::GetVirtualViewPort( SGLImageRect& rectView )
{
	if ( m_sizePhysical.w * m_sizeVirtual.h
					<= m_sizePhysical.h * m_sizeVirtual.w )
	{
		rectView.w = m_sizePhysical.w ;
		rectView.h = m_sizeVirtual.h * m_sizePhysical.w / m_sizeVirtual.w ;
		rectView.x = 0 ;
		rectView.y = (m_sizePhysical.h - rectView.h) / 2 ;
	}
	else
	{
		rectView.w = m_sizeVirtual.w * m_sizePhysical.h / m_sizeVirtual.h ;
		rectView.h = m_sizePhysical.h ;
		rectView.x = (m_sizePhysical.w - rectView.w) / 2 ;
		rectView.y = 0 ;
	}
}

// 対応機能フラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLEntisGLS3WindowProducer::GetCapacityFlags( void ) const
{
	return	renderableAnyThread ;
}

// 論理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnChangeVirtualViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	if ( ((uint32_t) m_sizeVirtual.w != nWidth)
		|| ((uint32_t) m_sizeVirtual.h != nHeight) )
	{
		m_imgVirtScreen.CreateImage( nWidth, nHeight, formatImageRGB, 32 ) ;
		m_imgVirtZBuf.CreateImage( nWidth, nHeight, formatImageZ, 32 ) ;
		m_sizeVirtual.w = (int32_t) nWidth ;
		m_sizeVirtual.h = (int32_t) nHeight ;
		GetVirtualViewPort( m_rectViewPort ) ;
	}
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	if ( ((uint32_t) m_sizePhysical.w != nWidth)
		|| ((uint32_t) m_sizePhysical.h != nHeight) )
	{
		m_imgPhysScreen.CreateImage( nWidth, nHeight, formatImageRGB, 32 ) ;
		m_imgPhysZBuf.CreateImage( nWidth, nHeight, formatImageZ, 32 ) ;
		m_sizePhysical.w = (int32_t) nWidth ;
		m_sizePhysical.h = (int32_t) nHeight ;
		GetVirtualViewPort( m_rectViewPort ) ;
	}
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnMovedWindow( SGLAbstractWindow * pWnd )
{
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	m_refWindow = pWnd ;
	m_hWnd = pWnd->GetWindowHandle() ;
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
	m_refWindow = NULL ;
	m_hWnd = NULL ;
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLEntisGLS3WindowProducer::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
}

// 論理ビュー表示座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::GetInternalViewPosition( SGLImageRect& rctVirtualView )
{
	rctVirtualView = m_rectViewPort ;
}

// 物理ビュー表示領域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLEntisGLS3WindowProducer::GetExternalViewPosition( SGLImageRect& rctPhysicalView )
{
	return	false ;
}

// 論理座標→物理ビュー座標変換行列取得
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::GetAffineVirtualToPhysical( SGLAffine& affine )
{
	affine.a11 = (float32_t) m_rectViewPort.w / (float32_t) m_sizeVirtual.w ;
	affine.a12 = 0.0f ;
	affine.a13 = (float32_t) m_rectViewPort.x ;
	affine.a21 = 0.0f ;
	affine.a22 = (float32_t) m_rectViewPort.h / (float32_t) m_sizeVirtual.h ;
	affine.a23 = (float32_t) m_rectViewPort.y ;
}

// 描画スレッドの関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLEntisGLS3WindowProducer::AttachViewThread( SGLAbstractWindow * pWnd )
{
	return	sglErrSuccess ;
}

// 描画スレッドの関連付け解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLEntisGLS3WindowProducer::DetachViewThread( SGLAbstractWindow * pWnd )
{
	return	sglErrSuccess ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLEntisGLS3WindowProducer::BeginDrawView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage,
			SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft,
			SGLImageObject * pZBufferLeft )
{
	if ( pImageLeft == NULL )
	{
		if ( pImage == NULL )
		{
			m_render.AttachTargetImage
				( &m_imgVirtScreen,
					(m_flagZBuffer ? &m_imgVirtZBuf : NULL), pWindow ) ;
		}
		else
		{
			m_render.AttachTargetImage( pImage, pZBuffer, pWindow ) ;
		}
		m_render.ResetTransformation() ;
		m_render.SelectParallaxView( RenderContext::stereoViewRight ) ;
	}
	else
	{
		m_render.AttachTargetImage( pImageLeft, pZBufferLeft, pWindow ) ;
		m_render.ResetTransformation() ;
		m_render.SelectParallaxView( RenderContext::stereoViewLeft ) ;
	}
	return	&m_render ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::EndDrawView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	m_render.Finish() ;
	m_render.DetachTargetImage() ;
	//
	if ( m_sizeVirtual != m_sizePhysical )
	{
		SGLPaintContextInterface *	paint = NULL ;
		if ( SSystem::g_cpuLogicalCount >= 4 )
		{
			paint = &m_paint ;
		}
		else
		{
			paint = &m_render ;
		}
		paint->AttachTargetImage( &m_imgPhysScreen, NULL, NULL ) ;
		//
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		pp.pAffine = &affine ;
		GetAffineVirtualToPhysical( affine ) ;
		paint->DrawImage( pp, &m_imgVirtScreen ) ;
		//
		paint->Finish() ;
		paint->DetachTargetImage() ;
	}
}

// 直接描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLEntisGLS3WindowProducer::BeginDirectView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage,
			SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft,
			SGLImageObject * pZBufferLeft )
{
	if ( pImage == NULL )
	{
		if ( m_sizeVirtual != m_sizePhysical )
		{
			m_render.AttachTargetImage
				( &m_imgPhysScreen,
					(m_flagZBuffer ? &m_imgPhysZBuf : NULL), pWindow ) ;
			if ( m_flagLastPhysZPaint )
			{
				m_render.FillClearTarget
					( 0, PaintContext::clearTargetZBuffer ) ;
				m_flagLastPhysZPaint = false ;
			}
		}
		else
		{
			m_render.AttachTargetImage
				( &m_imgVirtScreen, &m_imgVirtZBuf, pWindow ) ;
		}
	}
	else
	{
		m_render.AttachTargetImage( pImage, pZBuffer, pWindow ) ;
	}
	m_render.ResetTransformation() ;
	return	&m_render ;
}

// 直接描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::EndDirectView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	m_flagLastPhysZPaint = m_render.IsAnyPaintZBuffer() ;
	m_render.Finish() ;
	m_render.DetachTargetImage() ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLEntisGLS3WindowProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	SGLImageWin32DIBitmap *	pDIB = NULL ;
	if ( m_sizeVirtual != m_sizePhysical )
	{
		pDIB = SGLImageWin32DIBitmap::CommitDIB( &m_imgPhysScreen ) ;
	}
	else
	{
		pDIB = SGLImageWin32DIBitmap::CommitDIB( &m_imgVirtScreen ) ;
	}
	if ( pDIB != NULL )
	{
		HWND	hwnd = pWnd->GetWindowHandle() ;
		HDC		hdc = ::GetDC( hwnd ) ;
		::BitBlt
			( hdc, 0, 0, pDIB->m_imgbuf.width,
						pDIB->m_imgbuf.height,
							pDIB->m_hDC, 0, 0, SRCCOPY ) ;
		::ReleaseDC( hwnd, hdc ) ;
	}
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLEntisGLS3WindowProducer::EnableZBuffer
	( SGLAbstractWindow * pWnd, bool flagZBuffer )
{
	m_flagZBuffer = true ;
	return	sglErrSuccess ;
}

// レイヤードウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLEntisGLS3WindowProducer::EnableLayeredWindow
	( SGLAbstractWindow * pWnd, bool flagLayeredWindow )
{
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLEntisGLS3WindowProducer::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	return	sglErrSuccess ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLEntisGLS3WindowProducer::IsStereoDisplayMode( void )
{
	return	false ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLEntisGLS3WindowProducer::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::MonoView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}

// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLEntisGLS3WindowProducer::GetStandardDisplaySize( void ) const
{
	return	m_sizeVirtual ;
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLEntisGLS3WindowProducer::GetRenderDevice( void )
{
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS3 描画ウィンドウ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLGenericWindowGLS3View, SGLGenericWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLGenericWindowGLS3View::SGLGenericWindowGLS3View
		( SSystem::SEnvironmentInterface * env )
	: SGLGenericWindow( new SGLEntisGLS3WindowProducer, env )
{
}
