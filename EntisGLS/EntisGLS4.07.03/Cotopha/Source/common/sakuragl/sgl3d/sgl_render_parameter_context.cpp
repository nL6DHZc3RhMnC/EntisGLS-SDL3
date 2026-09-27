
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl3d_image.h>
#include <sakuragl/sgl3d/sgl_render_parameter_context.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 3D レンダリング・コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DRenderParameterContext, S3DRenderContextInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderParameterContext::S3DRenderParameterContext( void )
	: m_vProjectionScreen( 320, 240, 512 ),
		m_zProjectionScale(1.0),
		m_fpPixelAspectRatio(1.0),
		m_xProjectionScreenOrg(320),
		m_matCamera( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_matEnvMapping( 1, 0, 0,  0, 1, 0,  0, 0, 1 )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	m_fStereoView = false ;
	m_pStereoTarget[0] = NULL ;
	m_pStereoTarget[1] = NULL ;
	m_pStereoZBuffer[0] = NULL ;
	m_pStereoZBuffer[1] = NULL ;
	//
	m_pTransformation = NULL ;
	m_pGarbage = NULL ;
	m_flagsDefPaint = paintSmoothStretch ;
	//
	m_flagPersMatrix = false ;
	//
	m_xParallax = 0.0 ;
	m_zParallaxFocus = 1.0 ;
	m_xParallaxScreen = 0.0 ;
	//
	m_zMinClip = 0.1 ;
	m_zMaxClip = 3000.0 ;
	m_rgbAmbient.ui32 = 0 ;
	m_rgbAmbientMul.ui32 = 0x00FFFFFF ;
	//
	m_flagFog = false ;
	//
	DefaultOptionalContext( m_optContext ) ;
	//
	m_pEnvMapping = NULL ;
	m_nEnvMappingFlags = 0 ;
	m_pEnvRefraction = NULL ;
	m_nEnvRefractionFlags = envMappingViewport | envMappingRefraction ;
	//
	m_feature_sRGB = false ;
	//
	m_sviView = stereoViewAuto ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderParameterContext::~S3DRenderParameterContext( void )
{
	S3DRenderParameterContext::ResetTransformation() ;
	//
	TransformationList *	pGarbage = m_pGarbage ;
	while ( pGarbage != NULL )
	{
		TransformationList *	pPrev = pGarbage->pPrev ;
		delete	pGarbage ;
		pGarbage = pPrev ;
	}
	m_pGarbage = NULL ;
}

// 描画ターゲットを複製設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::AttachTargetImagesTo( S3DRenderParameterContext& context )
{
	SGLImageRect *	pRectView = NULL ;
	bool			fStereoView = false ;
	if ( m_pTarget != NULL )
	{
		pRectView = &m_rctView ;
		fStereoView = m_fStereoView ;
	}
	if ( fStereoView )
	{
		context.AttachStereoTargetImage
			( m_pStereoTarget[0], m_pStereoTarget[1],
				m_pStereoZBuffer[0], m_pStereoZBuffer[1], pRectView ) ;
	}
	else
	{
		context.DisableStereoTargetImage() ;
		context.AttachTargetImage( m_pTarget, m_pZBuffer, pRectView ) ;
	}
	//
	size_t					nMultiTargets = 0 ;
	SGLImageObject*const*	ppMultiTargets = GetMultiTargetImages( nMultiTargets ) ;
	context.AttachMultiTargetImages( ppMultiTargets, nMultiTargets ) ;
}

// 描画ターゲット設定が同一か判定
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderParameterContext::IsEqualTargetImages( const S3DRenderParameterContext& context ) const
{
	if ( m_fStereoView )
	{
		if ( (m_pStereoTarget[0] != context.m_pStereoTarget[0])
			|| (m_pStereoTarget[1] != context.m_pStereoTarget[1])
			|| (m_pStereoZBuffer[0] != context.m_pStereoZBuffer[0])
			|| (m_pStereoZBuffer[1] != context.m_pStereoZBuffer[1]) )
		{
			return	false ;
		}
	}
	else
	{
		if ( (m_pTarget != context.m_pTarget)
			|| (m_pZBuffer != context.m_pZBuffer) )
		{
			return	false ;
		}
	}
	if ( m_aMultiTarget.GetLength() != context.m_aMultiTarget.GetLength() )
	{
		return	false ;
	}
	SGLImageObject *const*	ppMultiTargets0 = m_aMultiTarget.GetConstArray() ;
	SGLImageObject *const*	ppMultiTargets1 = context.m_aMultiTarget.GetConstArray() ;
	const size_t	nMultiTargets = m_aMultiTarget.GetLength() ;
	for ( size_t i = 0; i < nMultiTargets; i ++ )
	{
		if ( ppMultiTargets0[i] != ppMultiTargets1[i] )
		{
			return	false ;
		}
	}
	return	true ;
}

// 描画パラメータを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetAllRenderingParameterTo
				( S3DRenderContextInterface * context, uint64_t flags )
{
	context->SetPaintFlags( m_flagsDefPaint ) ;
	context->SetRayTracingParameter( m_rrtpRayTracing ) ;
	//
	S3DMatrix	matEnvMapping ;
	matEnvMapping.InverseOf( m_matEnvMapping ) ;
	context->SetEnvironmentMappingImage( m_pEnvMapping, m_nEnvMappingFlags ) ;
	context->SetEnvironmentMappingImage( m_pEnvViewportDepth, envMappingViewportDepth ) ;
	context->SetEnvironmentMappingMatrix( matEnvMapping ) ;
	//
	EnvMappingParam	envMapParam ;
	envMapParam.pImage = m_pEnvRefraction ;
	envMapParam.typeMap = m_nEnvRefractionFlags ;
	envMapParam.matMap.InverseOf( m_matEnvRefraction ) ;
	context->SetOptionalFeature
		( featureEnvMap, envMappingRefraction,
					&envMapParam, sizeof(EnvMappingParam) ) ;
	//
	context->SetOptionalFeature
		( featureContextSet, 0, &m_optContext, sizeof(OptionalContextSet) ) ;
	//
	context->SetOptionalFeature
		( featureSRGB, m_feature_sRGB, NULL, 0 ) ;
	//
	context->SetProjectionScreen
		( m_vProjectionScreen, m_zProjectionScale, m_fpPixelAspectRatio ) ;
	context->SetCamera( m_matCamera, m_vCameraPos ) ;
	context->SetParallax( m_xParallax, m_zParallaxFocus, m_xParallaxScreen ) ;
	context->SetZClipRange( m_zMinClip, m_zMaxClip ) ;
	//
	if ( m_flagPersMatrix )
	{
		context->SetPerspectiveMatrix
			( stereoViewRight, m_matPerspective[stereoViewRight], true ) ;
		context->SetPerspectiveMatrix
			( stereoViewLeft, m_matPerspective[stereoViewLeft], true ) ;
	}
	else
	{
		context->EnablePerspectiveMatrix( false ) ;
	}
	//
	SArray<S3DLightEntry>	arrayLights ;
	size_t	i, nCount ;
	nCount = m_arrayVectorLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry *	pLight = m_arrayVectorLights.GetAt( i ) ;
		uint32_t	idLight = pLight->nReserved2[0] ;
		arrayLights.SetAt( idLight, *pLight ) ;
	}
	nCount = m_arrayPointLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry *	pLight = m_arrayPointLights.GetAt( i ) ;
		uint32_t	idLight = pLight->nReserved2[0] ;
		arrayLights.SetAt( idLight, *pLight ) ;
	}
	nCount = m_arrayFogLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry *	pLight = m_arrayFogLights.GetAt( i ) ;
		uint32_t	idLight = pLight->nReserved2[0] ;
		arrayLights.SetAt( idLight, *pLight ) ;
	}
	if ( m_rgbAmbient.ui32 != 0 )
	{
		S3DLightEntry	light ;
		light.typeLight = lightTypeAmbient ;
		light.rgbColor = m_rgbAmbient ;
		light.fpBrightness = 1.0f ;
		//
		S3DLightEntry *	pleLights = arrayLights.GetArray() ;
		bool	flagSetAmbLight = false ;
		nCount = arrayLights.GetLength() ;
		for ( i = 0; i < nCount; i ++ )
		{
			if ( pleLights[i].typeLight == 0 )
			{
				pleLights[i] = light ;
				flagSetAmbLight = true ;
				break ;
			}
		}
		arrayLights.FinishArray() ;
		//
		if ( !flagSetAmbLight )
		{
			arrayLights.Add( light ) ;
		}
	}
	if ( (m_rgbAmbientMul.ui32 & 0x00FFFFFF) != 0x00FFFFFF )
	{
		S3DLightEntry	light ;
		light.typeLight = lightTypeAmbientMul ;
		light.rgbColor = m_rgbAmbientMul ;
		light.fpBrightness = 1.0f ;
		arrayLights.Add( light ) ;
	}
	context->SetLightEntries
		( arrayLights.GetConstArray(), arrayLights.GetLength() ) ;
	//
	nCount = m_arrayShadowMapInf.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ShadowMapEntry *	psme = m_arrayShadowMapInf.GetAt( i ) ;
		ESLAssert( psme != NULL ) ;
		if ( psme->pDepth != NULL )
		{
			context->SetShadowMap
				( psme->idLight, psme->pDepth,
						psme->smiMapInfo, psme->pColor ) ;
		}
	}
	//
	context->SetFog( m_rgbFogColor.ui32, m_zFogNear, m_zFogFar ) ;
	context->EnableFog( m_flagFog ) ;
	//
	context->SelectParallaxView( m_sviView ) ;
}

// ステレオ立体視描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AttachStereoTargetImage
	( SGLImageObject * pImageRight, SGLImageObject * pImageLeft,
			SGLImageObject * pZBufferRight, SGLImageObject * pZBufferLeft,
			const SGLImageRect * pView )
{
	m_fStereoView = true ;
	m_pStereoTarget[0] = pImageRight ;
	m_pStereoTarget[1] = pImageLeft ;
	m_pStereoZBuffer[0] = pZBufferRight ;
	m_pStereoZBuffer[1] = pZBufferLeft ;
	//
	size_t	iSide = 0 ;
	if ( m_sviView == RenderContext::stereoViewLeft )
	{
		iSide = 1 ;
	}
	return	AttachTargetImage
		( m_pStereoTarget[iSide], m_pStereoZBuffer[iSide], pView ) ;
}

// ステレオ立体視無効化
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::DisableStereoTargetImage( void )
{
	m_fStereoView = false ;
}

// 一時オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::DeleteAllTemporaryObjects( void )
{
	m_arrayTemporary.RemoveAll() ;
}

// デフォルト値
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::DefaultOptionalContext( OptionalContextSet& optcs )
{
	optcs.nOptionMask = optionContextAll ;
	optcs.nShadingFlags = shadingMethodGouraud | shadingTextureSmoothing ;
	optcs.pShader = NULL ;
	optcs.opbBorder.rgbBorder = 0 ;
	optcs.opbBorder.aThickness = 0.0f ;
	optcs.opbBorder.bThickness = 1.0f ;
	optcs.faceCulling = faceCullingDefault ;
	optcs.depthMask = depthMaskDefault ;
	optcs.blendOp = blendDefault ;
	optcs.pointSize = 1.0f ;
	optcs.lineWidth = 1.0f ;
	optcs.anisotropy = 4.0f ;
}

// 比較
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderParameterContext::IsEqualOptionalContext
	( const OptionalContextSet& optcs1, const OptionalContextSet& optcs2 )
{
	if ( (optcs1.nShadingFlags == optcs2.nShadingFlags)
		& (optcs1.pShader == optcs2.pShader)
		& (optcs1.faceCulling == optcs2.faceCulling)
		& (optcs1.depthMask == optcs2.depthMask)
		& (optcs1.blendOp == optcs2.blendOp)
		& (optcs1.pointSize == optcs2.pointSize)
		& (optcs1.lineWidth == optcs2.lineWidth)
		& (optcs1.anisotropy == optcs2.anisotropy) )
	{
		if ( (optcs1.nShadingFlags & shadingDrawOffsetBorder)
			&& !(optcs1.nShadingFlags & shadingNoDrawOffsetBorder) )
		{
			return	IsEqualBorderParam( optcs1.opbBorder, optcs2.opbBorder ) ;
		}
		return	true ;
	}
	return	false ;
}

bool S3DRenderParameterContext::IsEqualBorderParam
	( const OffsetBorderParam& obp1, const OffsetBorderParam& obp2 )
{
	return	(obp1.rgbBorder == obp2.rgbBorder)
			& (obp1.aThickness == obp2.aThickness)
			& (obp1.bThickness == obp2.bThickness) ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DRenderParameterContext::GetTargetImage( void )
{
	return	m_pTarget ;
}

SGLImageObject * S3DRenderParameterContext::GetTargetZBuffer( void )
{
	return	m_pZBuffer ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::GetViewPort( SGLImageRect & rctView ) const
{
	rctView = m_rctView ;
	return	sglErrSuccess ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pTarget = pImage ;
	m_pZBuffer = pZBuffer ;
	//
	if ( pImage == NULL )
	{
		if ( pView != NULL )
		{
			m_rctView = *pView ;
		}
		return	sglErrSuccess ;
	}
	SGLImageInfo	imginf ;
	pImage->GetImageInfo( imginf ) ;
	m_rctView = imginf.GetImageRect() ;
	//
	if ( pView != NULL )
	{
		m_rctView = SGLRect( m_rctView ) & SGLRect( *pView ) ;
		if ( m_rctView.IsEmpty() )
		{
			return	sglErrSuccess ;
		}
	}
	return	sglErrSuccess ;
}

// マルチターゲット（2つ目以降）描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AttachMultiTargetImages
	( SGLImageObject *const* ppTargets, size_t nCount )
{
	m_aMultiTarget.SetLength( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_aMultiTarget.SetAt( i, ppTargets[i] ) ;
	}
	return	sglErrSuccess ;
}

// マルチターゲット（2つ目以降）取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject*const*
	S3DRenderParameterContext::GetMultiTargetImages( size_t& nCount ) const
{
	nCount = m_aMultiTarget.GetLength() ;
	return	m_aMultiTarget.GetConstArray() ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::DetachTargetImage( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	m_fStereoView = false ;
	m_aMultiTarget.RemoveAll() ;
	//
	DeleteAllTemporaryObjects() ;
	//
	return	sglErrSuccess ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AttachCustomShader( S3DCustomShader * pShader )
{
	m_optContext.pShader = pShader ;
	return	sglErrSuccess ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderParameterContext::GetCustomShader( void ) const
{
	return	m_optContext.pShader ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		return	SetTransformation( af, nTransparency ) ;
	}
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	pTransform->afTransform = pTransform->afTransform * af ;
	pTransform->nTransparency =
		0x100 - (0x100 - nTransparency)
					* (0x100 - pTransform->nTransparency) / 0x100 ;
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		pTransform = m_pGarbage ;
		if ( pTransform != NULL )
		{
			m_pGarbage = pTransform->pPrev ;
			pTransform->Recycle() ;
		}
		else
		{
			pTransform = new TransformationList ;
		}
		m_pTransformation = pTransform ;
	}
	pTransform->afTransform = af ;
	pTransform->nTransparency = nTransparency ;
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::CurrentAffine( SGLAffine & af )
{
	if ( m_pTransformation != NULL )
	{
		af = m_pTransformation->afTransform ;
		return	sglErrSuccess ;
	}
	af.a11 = 1.0f ;
	af.a12 = 0.0f ;
	af.a13 = 0.0f ;
	af.a21 = 0.0f ;
	af.a22 = 1.0f ;
	af.a23 = 0.0f ;
	return	sglErrFailed ;
}

unsigned int S3DRenderParameterContext::CurrentTransparency( void )
{
	if ( m_pTransformation != NULL )
	{
		return	m_pTransformation->nTransparency ;
	}
	return	0 ;
}

SGLError S3DRenderParameterContext::PushTransformation( void )
{
	TransformationList *	pTransform = m_pGarbage ;
	if ( pTransform != NULL )
	{
		m_pGarbage = pTransform->pPrev ;
		pTransform->Recycle() ;
	}
	else
	{
		pTransform = new TransformationList ;
	}
	pTransform->pPrev = m_pTransformation ;
	pTransform->optContext = m_optContext ;
	//
	if ( m_pTransformation != NULL )
	{
		m_pTransformation->optContext = m_optContext ;
		//
		pTransform->afTransform = m_pTransformation->afTransform ;
		pTransform->matTransform = m_pTransformation->matTransform ;
		pTransform->vTransform = m_pTransformation->vTransform ;
		pTransform->colorEffect = m_pTransformation->colorEffect ;
		pTransform->nTransparency = m_pTransformation->nTransparency ;
	}
	m_pTransformation = pTransform ;
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::PopTransformation( void )
{
	if ( m_pTransformation == NULL )
	{
		return	sglErrFailed ;
	}
	TransformationList *	pTransform = m_pTransformation ;
	m_pTransformation = pTransform->pPrev ;
	pTransform->pPrev = m_pGarbage ;
	m_pGarbage = pTransform ;
	//
	if ( m_pTransformation != NULL )
	{
		m_optContext = m_pTransformation->optContext ;
	}
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::ResetTransformation( void )
{
	TransformationList *	pLast = m_pTransformation ;
	if ( pLast != NULL )
	{
		TransformationList *	pFirst = pLast ;
		while ( pFirst->pPrev != NULL )
		{
			pFirst = pFirst->pPrev ;
		}
		ESLAssert( pFirst->pPrev == NULL ) ;
		pFirst->pPrev = m_pGarbage ;
		m_pGarbage = pLast ;
		m_pTransformation = NULL ;
		//
		m_optContext = pFirst->optContext ;
	}
	return	sglErrSuccess ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
		S3DCustomShader::UniformType type,
			const void * pData, size_t nCount )
{
	return	sglErrFailed ;
}

SGLError S3DRenderParameterContext::ResetCustomShaderUniform( void )
{
	return	sglErrFailed ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetPaintFlags( int64_t nFlags )
{
	m_flagsDefPaint = nFlags ;
}

int64_t S3DRenderParameterContext::GetPaintFlags( void )
{
	return	m_flagsDefPaint ;
}

// ２Ｄメッシュ描画（DrawImage 呼び出し）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::DrawMesh
	( const S2DVector * pDstMesh, const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh, const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	const size_t	countWidth = widthMesh + 1 ;
	S2DVector		vVerticesDst[4] ;
	S2DVector		vVerticesSrc[4] ;
	SGLAffine		afMeshHalf ;
	SGLImageInfo	infSrcImage ;
	SGLImageRect	rctSrcImage ;
	SGLPaintParam	ppPaintMesh = ppPaint ;
	S2DVector		vSrcMeshSize ;
	//
	if ( pSrcImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	if ( pSrcClip != NULL )
	{
		rctSrcImage = *pSrcClip ;
	}
	else
	{
		pSrcImage->GetImageInfo( infSrcImage ) ;
		rctSrcImage = infSrcImage.GetImageRect() ;
	}
	vSrcMeshSize.x = (float32_t) rctSrcImage.w / (float32_t) widthMesh ;
	vSrcMeshSize.y = (float32_t) rctSrcImage.h / (float32_t) heightMesh ;
	//
	ppPaintMesh.nFlags |= paintDelayable | paintOrderNoCare ;
	ppPaintMesh.pAffine = &afMeshHalf ;
	ppPaintMesh.pVertices = &vVerticesDst[0] ;
	ppPaintMesh.countVertex = 3 ;
	//
	for ( size_t y = 0, iLine = 0; y <= heightMesh; y ++ )
	{
		for ( size_t x = 0; x <= widthMesh; x ++ )
		{
			vVerticesDst[0] = pDstMesh[x] ;
			vVerticesDst[1] = pDstMesh[x + 1] ;
			vVerticesDst[2] = pDstMesh[x + countWidth] ;
			vVerticesDst[3] = pDstMesh[x + countWidth + 1] ;
			//
			if ( pSrcMesh != NULL )
			{
				vVerticesSrc[0] = pSrcMesh[x] ;
				vVerticesSrc[1] = pSrcMesh[x + 1] ;
				vVerticesSrc[2] = pSrcMesh[x + countWidth] ;
				vVerticesSrc[3] = pSrcMesh[x + countWidth + 1] ;
			}
			else
			{
				vVerticesSrc[0].x =
					(float32_t) rctSrcImage.x
									+ vSrcMeshSize.x * (float32_t) x ;
				vVerticesSrc[0].y =
					(float32_t) rctSrcImage.y
									+ vSrcMeshSize.y * (float32_t) y ;
				vVerticesSrc[1].x = vVerticesSrc[0].x + vSrcMeshSize.x ;
				vVerticesSrc[1].y = vVerticesSrc[0].y ;
				vVerticesSrc[2].x = vVerticesSrc[0].x ;
				vVerticesSrc[2].y = vVerticesSrc[0].y + vSrcMeshSize.y ;
				vVerticesSrc[3].x = vVerticesSrc[1].x ;
				vVerticesSrc[3].y = vVerticesSrc[2].y ;
			}
			ppPaintMesh.pVertices = &vVerticesDst[0] ;
			afMeshHalf.MappingOf( &vVerticesDst[0], &vVerticesSrc[0] ) ;
			DrawImage( ppPaintMesh, pSrcImage ) ;
			//
			if ( (y == heightMesh) & (x == widthMesh) )
			{
				ppPaintMesh.nFlags = ppPaint.nFlags ;
			}
			ppPaintMesh.pVertices = &vVerticesDst[1] ;
			afMeshHalf.MappingOf( &vVerticesDst[1], &vVerticesSrc[1] ) ;
			DrawImage( ppPaintMesh, pSrcImage ) ;
		}
		pDstMesh += countWidth ;
		pSrcMesh += countWidth ;
	}
	return	sglErrSuccess ;
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	SGLError	err = sglErrSuccess ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		err = DrawImage
			( pParams[i], ppSrcImages[i],
				((pSrcClips != NULL) ? pSrcClips + i : NULL) ) ;
	}
	return	err ;
}

// 画像変形描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::DrawTrianglePolygon
	( const S2DVector * pDstVertices,
		const S2DVector * pSrcVertices,
		const SGLPaintParam & ppPaint, SGLImageObject * pSrcImage )
{
	SGLPaintParam	ppPaintPoly = ppPaint ;
	SGLAffine		afMeshHalf ;
	afMeshHalf.MappingOf( pDstVertices, pSrcVertices ) ;
	//
	ppPaintPoly.pAffine = &afMeshHalf ;
	ppPaintPoly.pVertices = pDstVertices ;
	ppPaintPoly.countVertex = 3 ;
	//
	return	DrawImage( ppPaintPoly, pSrcImage ) ;
}

// ｚクリップ範囲を取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::GetZClipRange( double& zMin, double& zMax ) const
{
	zMin = m_zMinClip ;
	zMax = m_zMaxClip ;
}

// 2D 変換行列を取得（カメラは含まない）
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderParameterContext::GetAffineTransformation( SGLAffine& af ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		af.a11 = 1.0f ;
		af.a12 = 0.0f ;
		af.a13 = 0.0f ;
		af.a21 = 0.0f ;
		af.a22 = 1.0f ;
		af.a23 = 0.0f ;
		return	false ;
	}
	af = pTransform->afTransform ;
	return	af.IsRotation() ;
}

// 3D 変換行列を取得（カメラを含む）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::GetTransformMatrix( S3DDMatrix& mat, S3DDVector& pos )
{
	//
	// カメラ×行列変換
	//
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		mat = m_matCamera * pTransform->matTransform ;
		pos = m_matCamera * pTransform->vTransform - m_vCameraPos ;
	}
	else
	{
		mat = m_matCamera ;
		pos = - m_vCameraPos ;
	}
	if ( (m_xParallax != 0)
		&& (m_sviView != S3DRenderContextInterface::stereoViewAuto) )
	{
		//
		// ステレオ立体視・視差
		//
		S3DDMatrix	matParallax( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		S3DDVector	vParallax( m_xParallax, 0, 0 ) ;
		S3DDVector	vViewAngle( 0, 0, m_vProjectionScreen.z * m_zParallaxFocus ) ;
		if ( m_sviView != S3DRenderContextInterface::stereoViewRight )
		{
			vParallax.x = (float32_t) - m_xParallax;
		}
		vViewAngle -= vParallax ;
		//
		if ( m_zParallaxFocus > 1.0e-8 )
		{
			matParallax.RevolveByAngleOn( vViewAngle ) ;
		}
		//
		mat = matParallax * mat ;
		pos = matParallax * pos - vParallax ;
	}
}

// 透明度を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int S3DRenderParameterContext::EffectTransparency( unsigned int nTransparency ) const
{
	if ( nTransparency >= 0x100 )
	{
		return	0x100 ;
	}
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		return	nTransparency ;
	}
	return	0x100 - (0x100 - nTransparency)
						* (0x100 - pTransform->nTransparency) / 0x100 ;
}

// 色効果を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderParameterContext::GetColorEffect( S3DColor& colorEffect ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		colorEffect = pTransform->colorEffect ;
		return	!(((colorEffect.rgbMul.ui32 & 0x00FFFFFF) == 0x00FFFFFF)
							& ((colorEffect.rgbAdd.ui32 & 0x00FFFFFF) == 0)) ;
	}
	else
	{
		colorEffect.rgbMul.ui32 = 0x00FFFFFF ;
		colorEffect.rgbAdd.ui32 = 0 ;
		return	false ;
	}
}

// カメラの逆行列を設定する（カメラを無効化）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetMatrixInverseOfCamera( void )
{
	S3DDMatrix		matICamera ;
	S3DDVector		posICamera ;
	S3DColor		colorEffect ;
	unsigned int	nTransparency ;
	//
	matICamera.InverseOf( m_matCamera ) ;
	posICamera = matICamera * m_vCameraPos ;
	GetColorEffect( colorEffect ) ;
	nTransparency = EffectTransparency( 0 ) ;
	//
	SetMatrixTransformation
		( matICamera, posICamera, &colorEffect, nTransparency ) ;
}

// 3D 変換行列（色・透明度含む）を
// 別の S3DRenderBufferInterface に設定する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetMatrixTransformationTo
							( S3DRenderBufferInterface * render )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		render->SetMatrixTransformation
			( pTransform->matTransform, pTransform->vTransform,
				&(pTransform->colorEffect), pTransform->nTransparency ) ;
	}
	else
	{
		S3DDMatrix	matUnit( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		S3DDVector	vZero( 0, 0, 0 ) ;
		render->SetMatrixTransformation( matUnit, vZero, NULL, 0 ) ;
	}
}

// 2D 変換行列と互換性のある 3D 変換行列（色・透明度含む）を
// 別の S3DRenderBufferInterface に設定する
//（※ターゲットにも同じカメラが設定されていると仮定）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetMatrixTransformationAsAffineTo
	( S3DRenderBufferInterface * render, double z, uint32_t flagsPaint )
{
	S3DDMatrix	mat ;
	S3DDVector	pos ;
	mat.InverseOf( m_matCamera ) ;			// = camera posture
	pos = m_vCameraPos ;					// = m_matCamera * (camera pos)
	//
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		S3DDMatrix	matAffine
			( pTransform->afTransform.a11, pTransform->afTransform.a12, 0,
				pTransform->afTransform.a21, pTransform->afTransform.a22, 0,
				0, 0, 1 ) ;
		double	xOffset = pTransform->afTransform.a13 - m_vProjectionScreen.x ;
		double	yOffset = pTransform->afTransform.a23 - m_vProjectionScreen.y ;
		double	zScreen = m_vProjectionScreen.z * m_zProjectionScale ;
		double	zOffset = zScreen ;
		if ( flagsPaint & (paintWithZOrder | paintWithZOrderNoWrite) )
		{
			double	s = z / zScreen ;
			zOffset = z ;
			xOffset *= s ;
			yOffset *= s ;
			//
			matAffine.m[0][0] *= s ;
			matAffine.m[0][1] *= s ;
			matAffine.m[1][0] *= s ;
			matAffine.m[1][1] *= s ;
		}
		else
		{
			if ( (zScreen < m_zMinClip * 1.1)
				|| (zScreen > m_zMaxClip * 0.9) )
			{
				zOffset = (m_zMinClip + m_zMaxClip) * 0.5 ;
				//
				double	s = zOffset / zScreen ;
				matAffine.m[0][0] *= s ;
				matAffine.m[0][1] *= s ;
				matAffine.m[1][0] *= s ;
				matAffine.m[1][1] *= s ;
				//
				xOffset *= s ;
				yOffset *= s ;
			}
		}
		pos.x += xOffset ;
		pos.y += yOffset ;
		pos.z += zOffset ;
		//
		pos = mat * pos ;
		//
		mat *= matAffine ;
		render->SetMatrixTransformation
			( mat, pos, &(pTransform->colorEffect), pTransform->nTransparency ) ;
	}
	else
	{
		S3DDMatrix	matAffine( 1, 1, 1 ) ;
		double	xOffset = - m_vProjectionScreen.x ;
		double	yOffset = - m_vProjectionScreen.y ;
		double	zScreen = m_vProjectionScreen.z * m_zProjectionScale ;
		double	zOffset = zScreen ;
		//
		if ( !(flagsPaint & (paintWithZOrder | paintWithZOrderNoWrite))
			&& ((zScreen < m_zMinClip * 1.1)
				|| (zScreen > m_zMaxClip * 0.9)) )
		{
			zOffset = (m_zMinClip + m_zMaxClip) * 0.5 ;
			//
			double	s = zOffset / zScreen ;
			matAffine.m[0][0] *= s ;
			matAffine.m[1][1] *= s ;
			//
			xOffset *= s ;
			yOffset *= s ;
		}
		//
		pos.x += xOffset ;
		pos.y += yOffset ;
		pos.z += zOffset ;
		pos = mat * pos ;
		//
		mat *= matAffine ;
		render->SetMatrixTransformation( mat, pos, NULL, 0 ) ;
	}
}

// 視差カメラを別の S3DRenderBufferInterface に設定する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetParallaxCameraTo
	( S3DRenderContextInterface * render, StereoViewIndex sviView )
{
	S3DDMatrix	matParallax( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	double		rSin = 0, rCos = 1 ;
	if ( sviView == stereoViewRight )
	{
		rCos = m_vProjectionScreen.z * m_zParallaxFocus ;
		rSin = - m_xParallax ;
	}
	else if ( sviView == stereoViewLeft )
	{
		rCos = m_vProjectionScreen.z * m_zParallaxFocus ;
		rSin = m_xParallax ;
	}
	double	r = 1.0 / sqrt( rCos * rCos + rSin * rSin ) ;
	if ( m_zParallaxFocus > 1.0e-8 )
	{
		matParallax.RevolveOnY( rSin * r, rCos * r ) ;
	}
	//
	// Mpar * (Mcam * x - Pcam) - Ppar
	//	= (Mpar * Mcam) * x - (Mpar * Pcam + Ppar)
	//
	S3DDVector	vParallax( m_xParallax, 0, 0 ) ;
	if ( sviView == stereoViewLeft )
	{
		vParallax.x = - m_xParallax ;
	}
	vParallax += matParallax * m_vCameraPos ;
	matParallax *= m_matCamera ;
	//
	render->SetCamera( matParallax, vParallax ) ;
	//
	// ＨＭＤ眼球位置補正用
	//
	if ( fabs(m_xParallaxScreen) > 1.0e-5 )
	{
		S3DVector	vScreen
			( m_xProjectionScreenOrg,
				m_vProjectionScreen.y, m_vProjectionScreen.z ) ;
		if ( sviView == stereoViewRight )
		{
			vScreen.x += (float32_t) m_xParallaxScreen ;
		}
		else if ( sviView == stereoViewLeft )
		{
			vScreen.x -= (float32_t) m_xParallaxScreen ;
		}
		render->SetProjectionScreen
			( vScreen, m_zProjectionScale, m_fpPixelAspectRatio ) ;
	}
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	renderSrc.Finish() ;
	//
	if ( nFlags == 0 )
	{
		nFlags = copyBufferColor | copyBufferDepth ;
	}
	SGLError	err = sglErrInvalidParam ;
	if ( nFlags & copyBufferColor )
	{
		SGLImageObject *	pDstImage = GetTargetImage() ;
		SGLImageObject *	pSrcImage = renderSrc.GetTargetImage() ;
		if ( pDstImage && pSrcImage )
		{
			err = pDstImage->CopyImage( pSrcImage, xDst, yDst, pSrcRect ) ;
		}
	}
	if ( nFlags & copyBufferDepth )
	{
		SGLImageObject *	pDstImage = GetTargetZBuffer() ;
		SGLImageObject *	pSrcImage = renderSrc.GetTargetZBuffer() ;
		if ( pDstImage && pSrcImage )
		{
			err = pDstImage->CopyImage( pSrcImage, xDst, yDst, pSrcRect ) ;
		}
	}
	return	err ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AppendMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	if ( m_pTransformation == NULL )
	{
		return	SetMatrixTransformation( mat, pos, color, nTransparency ) ;
	}
	TransformationList *	pTransform = m_pTransformation ;
	pTransform->vTransform += m_pTransformation->matTransform * pos ;
	pTransform->matTransform *= mat ;
	if ( color != NULL )
	{
		pTransform->colorEffect *= *color ;
	}
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	pTransform->nTransparency =
		0x100 - (0x100 - nTransparency)
					* (0x100 - pTransform->nTransparency) / 0x100 ;
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::SetMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform == NULL )
	{
		pTransform = m_pGarbage ;
		if ( pTransform != NULL )
		{
			m_pGarbage = pTransform->pPrev ;
			pTransform->Recycle() ;
		}
		else
		{
			pTransform = new TransformationList ;
		}
		m_pTransformation = pTransform ;
	}
	pTransform->matTransform = mat ;
	pTransform->vTransform = pos ;
	if ( color != NULL )
	{
		pTransform->colorEffect = *color ;
	}
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	pTransform->nTransparency = nTransparency ;
	return	sglErrSuccess ;
}

SGLError S3DRenderParameterContext::GetMatrixTransformation
	( S3DDMatrix& mat, S3DDVector& pos,
		S3DColor * color, unsigned int * pTransparency ) const
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( pTransform != NULL )
	{
		mat = pTransform->matTransform ;
		pos = pTransform->vTransform ;
		if ( color != NULL )
		{
			*color = pTransform->colorEffect ;
		}
		if ( pTransparency != NULL )
		{
			*pTransparency = pTransform->nTransparency ;
		}
	}
	else
	{
		mat.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
		if ( color != NULL )
		{
			color->rgbMul.ui32 = 0x00FFFFFF ;
			color->rgbAdd.ui32 = 0 ;
		}
		if ( pTransparency != NULL )
		{
			*pTransparency = 0 ;
		}
	}
	return	sglErrSuccess ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	m_vProjectionScreen = vScreen ;
	m_zProjectionScale = zScale ;
	m_xProjectionScreenOrg = vScreen.x ;
	m_fpPixelAspectRatio = fpPixelAspect ;
	return	sglErrSuccess ;
}

// 投影スクリーン座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::GetProjectionScreen
	( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) const
{
	vScreen = m_vProjectionScreen ;
	zScale = m_zProjectionScale ;
	fpPixelAspect = m_fpPixelAspectRatio ;
	return	sglErrSuccess ;
}

// 透視変換行列取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderParameterContext::GetPerspectiveMatrix( S4DMatrix& matPers ) const
{
	if ( m_flagPersMatrix )
	{
		matPers = m_matPerspective[0] ;
		return	true ;
	}
	matPers.PerspectiveProjection
		( m_vProjectionScreen.x,
				m_vProjectionScreen.y, m_vProjectionScreen.z,
			m_rctView.w, m_rctView.h, m_zMinClip, m_zMaxClip,
			S4DMatrix::persZBoundsN1_1_YUp,
			0.0, 0.0, m_rctView.x, m_rctView.y, NULL ) ;
	return	true ;
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetPerspectiveMatrix
	( StereoViewIndex sviView, const S4DMatrix& matPers, bool fPersMatrix )
{
	switch ( sviView )
	{
	case	stereoViewAuto:
	default:
		m_matPerspective[0] = matPers ;
		m_matPerspective[1] = matPers ;
		break ;
	case	stereoViewRight:
	case	stereoViewLeft:
		m_matPerspective[sviView] = matPers ;
		break ;
	}
	m_flagPersMatrix = fPersMatrix ;
}

void S3DRenderParameterContext::EnablePerspectiveMatrix( bool fPersMatrix )
{
	m_flagPersMatrix = fPersMatrix ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	m_matCamera = matCamera ;
	m_vCameraPos = posCamera ;
}

// カメラ取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::GetCamera
	( S3DDMatrix& matCamera, S3DDVector& posCamera ) const
{
	matCamera = m_matCamera ;
	posCamera = m_vCameraPos ;
}

// 立体視視差設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	m_xParallax = xParallax ;
	m_zParallaxFocus = zFocusRate ;
	m_xParallaxScreen = xScreenDelta ;
}

// 立体視視差取得
//////////////////////////////////////////////////////////////////////////////
double S3DRenderParameterContext::GetParallax( void ) const
{
	return	m_xParallax ;
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetZClipRange( double zMin, double zMax )
{
	m_zMinClip = zMin ;
	m_zMaxClip = zMax ;
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	m_arrayVectorLights.SetLength( 0 ) ;
	m_arrayPointLights.SetLength( 0 ) ;
	m_arrayFogLights.SetLength( 0 ) ;
	m_arrayShadowMapInf.SetLength( 0 ) ;
	//
	m_rgbAmbient.ui32 = 0 ;
	m_rgbAmbientMul.ui32 = 0x00FFFFFF ;
	//
	for ( size_t i = 0; i < countLight; i ++ )
	{
		S3DLightEntry	light = pLights[i] ;
		light.nReserved2[0] = (uint32_t) i ;
		switch ( light.typeLight & lightTypeMask )
		{
		case	lightTypeVector:
			m_arrayVectorLights.Add( light ) ;
			break ;
		case	lightTypePoint:
		case	lightTypeSpot:
			m_arrayPointLights.Add( light ) ;
			break ;
		case	lightTypeAmbient:
			m_rgbAmbient += light.rgbColor ;
			break ;
		case	lightTypeAmbientMul:
			m_rgbAmbientMul *= light.rgbColor ;
			break ;
		case	lightTypeFog:
			m_arrayFogLights.Add( light ) ;
			break ;
		}
	}
	m_rgbAmbient.ui32 &= 0x00FFFFFF ;
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMap,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	if ( pShadowMap != NULL )
	{
		ShadowMapEntry	sme ;
		sme.idLight = idLight ;
		sme.pDepth = pShadowMap ;
		sme.pColor = pShadowMapColor ;
		sme.smiMapInfo = infShadowMap ;
		m_arrayShadowMapInf.Add( sme ) ;
	}
	else
	{
		for ( size_t i = 0; i < m_arrayShadowMapInf.GetLength(); i ++ )
		{
			ShadowMapEntry *	psme = m_arrayShadowMapInf.GetAt( i ) ;
			ESLAssert( psme != NULL ) ;
			if ( psme && (psme->idLight == idLight) )
			{
				m_arrayShadowMapInf.RemoveAt( i -- ) ;
			}
		}
	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	m_rgbFogColor.ui32 = rgbFog ;
	m_zFogNear = zFogNear ;
	m_zFogFar = zFogFar ;
}

void S3DRenderParameterContext::EnableFog( bool fFog )
{
	m_flagFog = fFog ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetShadingFlag( uint64_t nShadingMethod )
{
	m_optContext.nShadingFlags = nShadingMethod ;
}

// シェーディング取得
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DRenderParameterContext::GetShadingFlag( void )
{
	return	m_optContext.nShadingFlags ;
}

// レイトレーシング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetRayTracingParameter
			( const S3DRenderRayTracingParam& rrtp )
{
	m_rrtpRayTracing = rrtp ;
}

// グローバル環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetEnvironmentMappingImage
			( SGLImageObject * pImage, uint32_t nFlags )
{
	if ( nFlags & envMappingViewportDepth )
	{
		m_pEnvViewportDepth = pImage ;
	}
	else if ( nFlags & envMappingRefraction )
	{
		m_pEnvRefraction = pImage ;
		m_nEnvRefractionFlags = nFlags ;
	}
	else
	{
		m_pEnvMapping = pImage ;
		m_nEnvMappingFlags = nFlags ;
	}
}

// グローバル環境マッピング変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetEnvironmentMappingMatrix( const S3DMatrix& matMapping )
{
	m_matEnvMapping.InverseOf( matMapping ) ;
	m_matEnvRefraction = m_matEnvMapping ;
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetOffsetBorderColor( uint32_t rgbBorder )
{
	m_optContext.opbBorder.rgbBorder = rgbBorder ;
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	m_optContext.opbBorder.aThickness = a ;
	m_optContext.opbBorder.bThickness = b ;
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::SetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
			int32_t nParam1, const void * pParam2, size_t sizeOfParam2 )
{
	switch ( feature )
	{
	case	featureContextSet:
		if ( sizeOfParam2 != sizeof(OptionalContextSet) )
		{
			return	sglErrInvalidParam ;
		}
		else
		{
			const OptionalContextSet *	pocs = (const OptionalContextSet*) pParam2 ;
			if ( pocs->nOptionMask & optionShadingFlag )
			{
				m_optContext.nShadingFlags = pocs->nShadingFlags ;
			}
			if ( pocs->nOptionMask & optionCustomShader )
			{
				m_optContext.pShader = pocs->pShader ;
			}
			if ( pocs->nOptionMask & optionBorderParam )
			{
				m_optContext.opbBorder = pocs->opbBorder ;
			}
			if ( pocs->nOptionMask & optionFaceCulling )
			{
				m_optContext.faceCulling = pocs->faceCulling ;
			}
			if ( pocs->nOptionMask & optionDepthMask )
			{
				m_optContext.depthMask = pocs->depthMask ;
			}
			if ( pocs->nOptionMask & optionBlendOperation )
			{
				m_optContext.blendOp = pocs->blendOp ;
			}
			if ( pocs->nOptionMask & optionPointSize )
			{
				m_optContext.pointSize = pocs->pointSize ;
			}
			if ( pocs->nOptionMask & optionLineWidth )
			{
				m_optContext.lineWidth = pocs->lineWidth ;
			}
			if ( pocs->nOptionMask & optionAnisotropy )
			{
				m_optContext.anisotropy = pocs->anisotropy ;
			}
		}
		break ;

	case	featureEnvMap:
		if ( sizeOfParam2 != sizeof(EnvMappingParam) )
		{
			return	sglErrInvalidParam ;
		}
		if ( nParam1 & envMappingViewportDepth )
		{
			const EnvMappingParam *	pemp = (const EnvMappingParam*) pParam2 ;
			m_pEnvViewportDepth = pemp->pImage ;
		}
		else if ( nParam1 & envMappingRefraction )
		{
			const EnvMappingParam *	pemp = (const EnvMappingParam*) pParam2 ;
			m_pEnvRefraction = pemp->pImage ;
			m_nEnvRefractionFlags = pemp->typeMap | envMappingRefraction ;
			m_matEnvRefraction.InverseOf( pemp->matMap ) ;
		}
		else
		{
			const EnvMappingParam *	pemp = (const EnvMappingParam*) pParam2 ;
			m_pEnvMapping = pemp->pImage ;
			m_nEnvMappingFlags = pemp->typeMap ;
			m_matEnvMapping.InverseOf( pemp->matMap ) ;
		}
		break ;

	case	featureOffsetBorder:
		if ( sizeOfParam2 != sizeof(OffsetBorderParam) )
		{
			return	sglErrInvalidParam ;
		}
		else
		{
			const OffsetBorderParam *	pobp = (const OffsetBorderParam*) pParam2 ;
			m_optContext.opbBorder = *pobp ;
		}
		break ;

	case	featureFaceCulling:
		m_optContext.faceCulling = (FaceCullingOperation) nParam1 ;
		break ;

	case	featureDepthMask:
		m_optContext.depthMask = (DepthMaskOperation) nParam1 ;
		break ;

	case	featureBlendOperation:
		m_optContext.blendOp = (BlendOperation) nParam1 ;
		break ;

	case	featurePointSize:
		if ( sizeOfParam2 != sizeof(float32_t) )
		{
			return	sglErrInvalidParam ;
		}
		m_optContext.pointSize = *((float32_t*)pParam2) ;
		break ;

	case	featureLineWidth:
		if ( sizeOfParam2 != sizeof(float32_t) )
		{
			return	sglErrInvalidParam ;
		}
		m_optContext.lineWidth = *((float32_t*)pParam2) ;
		break ;

	case	featureAnisotropy:
		if ( sizeOfParam2 != sizeof(float32_t) )
		{
			return	sglErrInvalidParam ;
		}
		m_optContext.anisotropy = *((float32_t*)pParam2) ;
		break ;

	case	featureSRGB:
		m_feature_sRGB = (nParam1 != 0) ;
		break ;

	default:
		return	sglErrNotSupported ;
	}
	return	sglErrSuccess ;
}

// オプショナル機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::GetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
		int32_t nParam1, void * pParam2, size_t sizeOfParam2 ) const
{
	switch ( feature )
	{
	case	featureContextSet:
		if ( sizeOfParam2 != sizeof(OptionalContextSet) )
		{
			return	sglErrInvalidParam ;
		}
		else
		{
			OptionalContextSet *	pocs = (OptionalContextSet*) pParam2 ;
			*pocs = m_optContext ;
		}
		break ;

	case	featureEnvMap:
		if ( sizeOfParam2 != sizeof(EnvMappingParam) )
		{
			return	sglErrInvalidParam ;
		}
		if ( nParam1 & envMappingViewportDepth )
		{
			EnvMappingParam *	pemp = (EnvMappingParam*) pParam2 ;
			pemp->pImage = m_pEnvViewportDepth ;
			pemp->typeMap = m_nEnvMappingFlags ;
			pemp->matMap.InverseOf( m_matEnvMapping ) ;
		}
		else if ( nParam1 & envMappingRefraction )
		{
			EnvMappingParam *	pemp = (EnvMappingParam*) pParam2 ;
			pemp->pImage = m_pEnvRefraction ;
			pemp->typeMap = m_nEnvRefractionFlags & ~envMappingRefraction ;
			pemp->matMap.InverseOf( m_matEnvRefraction ) ;
		}
		else
		{
			EnvMappingParam *	pemp = (EnvMappingParam*) pParam2 ;
			pemp->pImage = m_pEnvMapping ;
			pemp->typeMap = m_nEnvMappingFlags ;
			pemp->matMap.InverseOf( m_matEnvMapping ) ;
		}
		break ;

	case	featureOffsetBorder:
		if ( sizeOfParam2 != sizeof(OffsetBorderParam) )
		{
			return	sglErrInvalidParam ;
		}
		else
		{
			OffsetBorderParam *	pobp = (OffsetBorderParam*) pParam2 ;
			*pobp = m_optContext.opbBorder ;
		}
		break ;

	case	featureFaceCulling:
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.faceCulling ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featureDepthMask:
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.depthMask ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featureBlendOperation:
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_optContext.blendOp ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featurePointSize:
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			*((float32_t*)pParam2) = m_optContext.pointSize  ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featureLineWidth:
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			*((float32_t*)pParam2) = m_optContext.lineWidth ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featureAnisotropy:
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			*((float32_t*)pParam2) = m_optContext.anisotropy ;
			break ;
		}
		return	sglErrInvalidParam ;

	case	featureSRGB:
		if ( sizeOfParam2 == sizeof(int32_t) )
		{
			*((int32_t*)pParam2) = m_feature_sRGB ;
			break ;
		}
		if ( sizeOfParam2 == sizeof(bool) )
		{
			*((bool*)pParam2) = m_feature_sRGB ;
			break ;
		}
		return	sglErrInvalidParam ;

	default:
		return	sglErrNotSupported ;
	}
	return	sglErrSuccess ;
}

// 選択中の立体視用バッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface::StereoViewIndex
		S3DRenderParameterContext::CurrentParallaxView( void )
{
	return	m_sviView ;
}

// 立体視用バッファ選択
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::SelectParallaxView
	( S3DRenderContextInterface::StereoViewIndex sviView )
{
	m_sviView = sviView ;
	//
	if ( m_fStereoView )
	{
		size_t	iSide = 0 ;
		if ( sviView == RenderContext::stereoViewLeft )
		{
			iSide = 1 ;
		}
		SGLImageRect	rctView = m_rctView ;
		return	AttachTargetImage
					( m_pStereoTarget[iSide],
							m_pStereoZBuffer[iSide], &rctView ) ;
	}
	return	sglErrSuccess ;
}

// 内部バッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::SetRenderingBufferSize( uint32_t countVertex )
{
	return	sglErrSuccess ;
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::Begin3DRenderer( uint64_t nFlags )
{
	return	sglErrSuccess ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::End3DRenderer( uint64_t nFlags )
{
	return	Flush() ;
}

// 非同期レンダリング開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::AsyncFlush
	( uint32_t nFlags, SSystem::SSignalEvent * pSignal )
{
	SGLError	err = (nFlags & asyncFlushCommitTarget) ? Finish() : Flush() ;
	if ( pSignal != NULL )
	{
		pSignal->SetSignal() ;
	}
	return	err ;
}

// 非同期レンダリング完了待機
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderParameterContext::WaitFlush( int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// 非同期レンダリングに適したスレッドで実行
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::SuitableProcedure
	( S3DRenderContextInterface::PROCEDURE_RENDERING pfnRendering, void * pInstance )
{
	pfnRendering( pInstance ) ;
}

// 遅延削除オブジェクト追加（Flush 時に削除）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderParameterContext::AddTemporaryObject( ESLObject * pObj )
{
	ESLAssert( m_arrayTemporary.FindPtr( pObj ) < 0 ) ;
	m_arrayTemporary.Add( pObj ) ;
	//
#if	defined(__DEBUG__)
	if ( m_arrayTemporary.GetLength() > 0x400 )
	{
		ESLTrace( "too many objects stocked by AddTemporaryObject (%d).\n",
											m_arrayTemporary.GetLength() ) ;
	}
#endif
}
