
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl3d_image.h>
#include <sakuragl/sgl3d/sgl_render_software_shader.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ソフトウェア・シェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderingShader, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderingShader::S3DRenderingShader( void )
{
	m_flagFog = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderingShader::~S3DRenderingShader( void )
{
}

// 光源を設定する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderingShader::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	m_rgbAmbient.Red = 0 ;
	m_rgbAmbient.Green = 0 ;
	m_rgbAmbient.Blue = 0 ;
	m_arrVectorLights.SetLength(0) ;
	m_arrPointLights.SetLength(0) ;
	m_arrFogLights.SetLength(0) ;
	//
	for ( size_t i = 0; i < countLight; i ++ )
	{
		const S3DLightEntry&	le = pLights[i] ;
		LIGHT	light ;
		switch ( le.typeLight & lightTypeMask )
		{
		case	lightTypeVector:
			light.typeLight = le.typeLight & lightTypeMask ;
			light.vPosition = le.vecPosition ;
			light.vDirection = le.vecDirection ;
			light.vDirection.Normalize() ;
			light.rgbColor.Red =
				eslRoundR32ToInt( le.rgbColor.argb.Red * le.fpBrightness ) ;
			light.rgbColor.Green =
				eslRoundR32ToInt( le.rgbColor.argb.Green * le.fpBrightness ) ;
			light.rgbColor.Blue =
				eslRoundR32ToInt( le.rgbColor.argb.Blue * le.fpBrightness ) ;
			if ( light.rgbColor.Red > 0x100 )
			{
				light.rgbColor.Red = 0x100 ;
			}
			if ( light.rgbColor.Green > 0x100 )
			{
				light.rgbColor.Green = 0x100 ;
			}
			if ( light.rgbColor.Blue > 0x100 )
			{
				light.rgbColor.Blue = 0x100 ;
			}
			m_arrVectorLights.Add( light ) ;
			break ;

		case	lightTypePoint:
		case	lightTypeSpot:
			light.typeLight = le.typeLight & lightTypeMask ;
			light.vPosition = le.vecPosition ;
			light.vDirection = le.vecDirection ;
			light.vDirection.Normalize() ;
			light.fpBrightness = le.fpBrightness ;
			light.fpAttenuationPower = le.fpAttenuationPower ;
			light.fpAngle = le.fpAngle ;
			light.fpGradation = le.fpGradation ;
			light.rgbColor.Red = le.rgbColor.argb.Red ;
			light.rgbColor.Green = le.rgbColor.argb.Green ;
			light.rgbColor.Blue = le.rgbColor.argb.Blue ;
			m_arrPointLights.Add( light ) ;
			break ;

		case	lightTypeAmbient:
			m_rgbAmbient.Red += le.rgbColor.argb.Red ;
			m_rgbAmbient.Green += le.rgbColor.argb.Green ;
			m_rgbAmbient.Blue += le.rgbColor.argb.Blue ;
			break ;

		case	lightTypeFog:
			light.typeLight = le.typeLight & lightTypeMask ;
			light.vPosition = le.vecPosition ;
			light.vDirection = le.vecDirection ;
			light.vDirection.Normalize() ;
			light.fpBrightness = 1.0f / le.fpBrightness ;
			light.rgbColor.Red = le.rgbColor.argb.Red ;
			light.rgbColor.Green = le.rgbColor.argb.Green ;
			light.rgbColor.Blue = le.rgbColor.argb.Blue ;
			m_arrPointLights.Add( light ) ;
			break ;
		}
	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderingShader::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	m_rgbFogColor.ui32 = rgbFog ;
	m_zFogNear = (float32_t) zFogNear ;
	m_zFogDistance = (float32_t) (1.0f / (zFogFar - m_zFogNear)) ;
}

void S3DRenderingShader::EnableFog( bool fFog )
{
	m_flagFog = fFog ;
}

// シェーディング
//////////////////////////////////////////////////////////////////////////////
void S3DRenderingShader::ShadeVertexColors
	( S3DColor * pColorLooks,
		const S3DSurfaceAttribute & attr,
		const S3DVector4 * pNormals,
		const S3DVector4 * pVertices,
		const S3DColor * pVertexColors, size_t nCount )
{
	RGB_INT32	rgbMulColor, rgbAddColor ;
	rgbMulColor.Red = attr.colorBase.rgbMul.argb.Red
						- attr.colorShade.rgbMul.argb.Red ;
	rgbMulColor.Green = attr.colorBase.rgbMul.argb.Green
						- attr.colorShade.rgbMul.argb.Green ;
	rgbMulColor.Blue = attr.colorBase.rgbMul.argb.Blue
						- attr.colorShade.rgbMul.argb.Blue ;
	rgbMulColor.Red &= ~(rgbMulColor.Red >> 31) ;
	rgbMulColor.Green &= ~(rgbMulColor.Green >> 31) ;
	rgbMulColor.Blue &= ~(rgbMulColor.Blue >> 31) ;
	//
	rgbAddColor.Red = attr.colorBase.rgbAdd.argb.Red
						- attr.colorShade.rgbAdd.argb.Red ;
	rgbAddColor.Green = attr.colorBase.rgbAdd.argb.Green
						- attr.colorShade.rgbAdd.argb.Green ;
	rgbAddColor.Blue = attr.colorBase.rgbAdd.argb.Blue
						- attr.colorShade.rgbAdd.argb.Blue ;
	rgbAddColor.Red &= ~(rgbAddColor.Red >> 31) ;
	rgbAddColor.Green &= ~(rgbAddColor.Green >> 31) ;
	rgbAddColor.Blue &= ~(rgbAddColor.Blue >> 31) ;
	//
	if ( pVertexColors != NULL )
	{
		if ( (attr.flagsShading & shadingMethodMask) == shadingMethodNothing )
		{
			if ( pColorLooks != pVertexColors )
			{
				eslMoveMemory
					( pColorLooks, pVertexColors, nCount * sizeof(S3DColor) ) ;
			}
			return ;
		}
		//
		SGLPalette	rgbAdd ;
		rgbMulColor.Red ++ ;
		rgbMulColor.Green ++ ;
		rgbMulColor.Blue ++ ;
		rgbAdd.argb.Red = (uint8_t) rgbAddColor.Red ;
		rgbAdd.argb.Green = (uint8_t) rgbAddColor.Green ;
		rgbAdd.argb.Blue = (uint8_t) rgbAddColor.Blue ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			S3DColor&		colorDst = pColorLooks[i] ;
			const S3DColor&	colorSrc = pVertexColors[i] ;
			colorDst.rgbAdd.argb.Red =
				(uint8_t) ((rgbMulColor.Red * colorSrc.rgbAdd.argb.Red) >> 8) ;
			colorDst.rgbAdd.argb.Green =
				(uint8_t) ((rgbMulColor.Green * colorSrc.rgbAdd.argb.Green) >> 8) ;
			colorDst.rgbAdd.argb.Blue =
				(uint8_t) ((rgbMulColor.Blue * colorSrc.rgbAdd.argb.Blue) >> 8) ;
			colorDst.rgbAdd += rgbAdd ;
			//
			colorDst.rgbMul.argb.Red =
				(uint8_t) ((rgbMulColor.Red * colorSrc.rgbMul.argb.Red) >> 8) ;
			colorDst.rgbMul.argb.Green =
				(uint8_t) ((rgbMulColor.Green * colorSrc.rgbMul.argb.Green) >> 8) ;
			colorDst.rgbMul.argb.Blue =
				(uint8_t) ((rgbMulColor.Blue * colorSrc.rgbMul.argb.Blue) >> 8) ;
		}
	}
	else
	{
		S3DColor	color ;
		color.rgbMul.argb.Red = (uint8_t) rgbMulColor.Red ;
		color.rgbMul.argb.Green = (uint8_t) rgbMulColor.Green ;
		color.rgbMul.argb.Blue = (uint8_t) rgbMulColor.Blue ;
		color.rgbAdd.argb.Red = (uint8_t) rgbAddColor.Red ;
		color.rgbAdd.argb.Green = (uint8_t) rgbAddColor.Green ;
		color.rgbAdd.argb.Blue = (uint8_t) rgbAddColor.Blue ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pColorLooks[i] = color ;
		}
	}
	ShadeVectors( attr, pNormals, pVertices, pColorLooks, nCount ) ;
}

void S3DRenderingShader::ShadeVectors
	( const S3DSurfaceAttribute & attr,
		const S3DVector4 * pNormals,
		const S3DVector4 * pVertices,
		S3DColor * pColorLooks, size_t nCount )
{
	const uint64_t	flagsShading = attr.flagsShading ;
	if ( (flagsShading & shadingMethodMask) == shadingMethodNothing )
	{
		return ;
	}
	//
	// 表面属性定数値計算
	//
	const float32_t		scaleBy256 = 1.0f / 256.0f ;
	SHADING_PARAMETER	sdp ;
	bool				fDeepness = (attr.nDeepness != 0) ;
	bool				fFogEffect = ((flagsShading & shadingNoFogEffect) == 0)
												& (m_arrFogLights.GetLength() > 0) ;
	bool				fTransparency =
							(attr.nTransparency != 0)
								& ((flagsShading & shadingTextureMapping) == 0) ;
	S3DColor			colorShade = attr.colorShade ;
	sdp.maskDoubleSide =
		(((flagsShading & shadingSingleSidePlane) == 0) ? (uint32_t) -1 : 0) ;
	sdp.nAmbient = attr.nAmbient ;
	sdp.fpDiffusion = (float32_t) attr.nDiffusion * scaleBy256 ;
	sdp.fpSpecular = (float32_t) attr.nSpecular * scaleBy256 ;
	sdp.fpSpecularSize = (float32_t) attr.nSpecularSize * scaleBy256 ;
	sdp.fpDeepness = (float32_t) attr.nDeepness * scaleBy256 ;
	sdp.nAlpha = 0x100 - attr.nTransparency ;
	if ( sdp.nAlpha < 0 )
	{
		sdp.nAlpha = 0 ;
	}
	else if ( sdp.nAlpha > 0x100 )
	{
		sdp.nAlpha = 0x100 ;
	}
	//
	// 各頂点計算
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DColor &		clrLooks = pColorLooks[i] ;
		S3DVector		vNormal = pNormals[i] ;
		S3DVector		vVertexPos = pVertices[i] ;
		S3DVector		vVertex = vVertexPos ;
		const float32_t	zVertex = vVertex.z ;
		//
		// 光源効果計算
		//
		sdp.rgbLight.Red = 0 ;
		sdp.rgbLight.Green = 0 ;
		sdp.rgbLight.Blue = 0 ;
		sdp.rgbDiffusion.Red = sdp.nAmbient ;
		sdp.rgbDiffusion.Green = sdp.nAmbient ;
		sdp.rgbDiffusion.Blue = sdp.nAmbient ;
		//
		vNormal.Normalize() ;
		vVertex.Normalize() ;
		//
		const float32_t	fpFocusParam = (vVertex | vNormal) ;
		//
		size_t		iLight, countLights ;
		LIGHT *		pLights ;
		S3DVector	vDirection ;
		countLights = m_arrVectorLights.GetLength() ;
		pLights = m_arrVectorLights.GetArray() ;
		for ( iLight = 0; iLight < countLights; iLight ++ )
		{
			CalculateLightEffect
				( sdp, pLights[iLight], vNormal, vVertex, fpFocusParam ) ;
		}
		m_arrVectorLights.FinishArray() ;
		//
		countLights = m_arrPointLights.GetLength() ;
		const LIGHT *	pcLights = m_arrPointLights.GetConstArray() ;
		for ( iLight = 0; iLight < countLights; iLight ++ )
		{
			const LIGHT &	light = pcLights[iLight] ;
			LIGHT	lightEffect = light ;
			vDirection = vVertexPos ;
			vDirection -= light.vPosition ;
			float32_t	fpDistance = (float32_t) vDirection.Absolute() ;
			if ( fpDistance < 1.0e-5 )
			{
				continue ;
			}
			float32_t	r = 1.0f / fpDistance ;
			vDirection *= r ;
			r = (float32_t) pow( r,
						light.fpAttenuationPower ) * light.fpBrightness ;
			if ( lightEffect.typeLight == lightTypeSpot )
			{
				if ( r > 1.0 )
				{
					r = 1.0 ;
				}
				double	cosDir = vDirection | lightEffect.vDirection ;
				if ( cosDir <= lightEffect.fpAngle )
				{
					continue ;
				}
				cosDir -= lightEffect.fpAngle ;
				if ( cosDir <= lightEffect.fpGradation )
				{
					r *= (float32_t) (cosDir / lightEffect.fpGradation) ;
				}
			}
			else
			{
				if ( r > 1.0 )
				{
					r = (float32_t) sqrt( r ) ;
					if ( r > 2.0 )
					{
						r = 2.0 ;
					}
				}
			}
			lightEffect.vDirection = vDirection ;
			//
			lightEffect.rgbColor.Red   = eslRoundR32ToInt( light.rgbColor.Red * r ) ;
			lightEffect.rgbColor.Green = eslRoundR32ToInt( light.rgbColor.Green * r ) ;
			lightEffect.rgbColor.Blue  = eslRoundR32ToInt( light.rgbColor.Blue * r ) ;
			//
			CalculateLightEffect
				( sdp, lightEffect, vNormal, vVertex, fpFocusParam ) ;
		}
		//
		// 光源色効果計算
		//
		RGB_INT32	rgbDiffusion ;
		rgbDiffusion.Red   = sdp.rgbDiffusion.Red + m_rgbAmbient.Red ;
		rgbDiffusion.Green = sdp.rgbDiffusion.Green + m_rgbAmbient.Green ;
		rgbDiffusion.Blue  = sdp.rgbDiffusion.Blue + m_rgbAmbient.Blue ;
		//
		RGB_INT32	rgbLight ;
		rgbLight.Red =
			((clrLooks.rgbAdd.argb.Red * rgbDiffusion.Red) >> 8)
				+ sdp.rgbLight.Red + colorShade.rgbAdd.argb.Red ;
		rgbLight.Green =
			((clrLooks.rgbAdd.argb.Green * rgbDiffusion.Green) >> 8)
				+ sdp.rgbLight.Green + colorShade.rgbAdd.argb.Green ;
		rgbLight.Blue =
			((clrLooks.rgbAdd.argb.Blue * rgbDiffusion.Blue) >> 8)
				+ sdp.rgbLight.Blue + colorShade.rgbAdd.argb.Blue ;
		//
		clrLooks.rgbAdd.argb.Red =
			(uint8_t) (rgbLight.Red | ((0xff - rgbLight.Red) >> 31)) ;
		clrLooks.rgbAdd.argb.Green =
			(uint8_t) (rgbLight.Green | ((0xff - rgbLight.Green) >> 31)) ;
		clrLooks.rgbAdd.argb.Blue =
			(uint8_t) (rgbLight.Blue | ((0xff - rgbLight.Blue) >> 31)) ;
		//
		rgbDiffusion.Red =
			((clrLooks.rgbMul.argb.Red * rgbDiffusion.Red) >> 8)
										+ colorShade.rgbMul.argb.Red ;
		rgbDiffusion.Green =
			((clrLooks.rgbMul.argb.Green * rgbDiffusion.Green) >> 8)
										+ colorShade.rgbMul.argb.Green ;
		rgbDiffusion.Blue =
			((clrLooks.rgbMul.argb.Blue * rgbDiffusion.Blue) >> 8)
										+ colorShade.rgbMul.argb.Blue ;
		//
		clrLooks.rgbMul.argb.Red =
			(uint8_t) (rgbDiffusion.Red | ((0xff - rgbDiffusion.Red) >> 31)) ;
		clrLooks.rgbMul.argb.Green =
			(uint8_t) (rgbDiffusion.Green | ((0xff - rgbDiffusion.Green) >> 31)) ;
		clrLooks.rgbMul.argb.Blue =
			(uint8_t) (rgbDiffusion.Blue | ((0xff - rgbDiffusion.Blue) >> 31)) ;
		//
		// 透明度・透明深度計算
		//
		if ( fTransparency | fDeepness )
		{
			int32_t	nAlpha = sdp.nAlpha ;
			if ( fDeepness )
			{
				nAlpha -=
					eslRoundR32ToInt
						( (float32_t) fabs( fpFocusParam )
											* sdp.fpDeepness * 256.0f ) ;
				nAlpha &= ~(nAlpha >> 31) ;
				nAlpha |= (0xff - nAlpha) >> 31 ;
				nAlpha = (nAlpha & 0xff) + 1 ;
			}
			clrLooks.rgbAdd *= (unsigned int) nAlpha ;
			clrLooks.rgbMul.ui32 = ~clrLooks.rgbMul.ui32 ;
			clrLooks.rgbMul *= (unsigned int) nAlpha ;
			clrLooks.rgbMul.ui32 = ~clrLooks.rgbMul.ui32 ;
		}
		//
		// 擬似フォッグ計算
		//
		if ( fFogEffect )
		{
			countLights = m_arrFogLights.GetLength() ;
			pcLights = m_arrFogLights.GetArray() ;
			for ( iLight = 0; iLight < countLights; iLight ++ )
			{
				const LIGHT &	light = pcLights[iLight] ;
				S3DVector	vDeepness = vVertexPos - light.vPosition ;
				vDeepness.Normalize() ;
				float32_t	fpDeepness = (vDeepness | light.vDirection) ;
				if ( fpDeepness > 0 )
				{
					// 対象点は境界の内側
					if ( (light.vDirection | light.vPosition) < 0 )
					{
						// 視点は境界面の内側
						fpDeepness = zVertex ;
					}
					fpDeepness *= light.fpBrightness ;
					SGLPalette	rgbFog
						( light.rgbColor.Blue, light.rgbColor.Green, light.rgbColor.Red ) ;
					if ( fpDeepness < 1.0f )
					{
						uint32_t	nFog = (uint32_t) eslRoundR32ToInt( fpDeepness * 256.0f ) ;
						uint32_t	nCompFog = 0x100 - nFog ;
						clrLooks.rgbMul = clrLooks.rgbMul.imul( nCompFog ) ;
						clrLooks.rgbAdd = clrLooks.rgbAdd.imul( nCompFog ) ;
						clrLooks.rgbAdd += rgbFog.imul( nFog ) ;
					}
					else
					{
						clrLooks.rgbMul.ui32 = 0 ;
						clrLooks.rgbAdd = rgbFog ;
					}
				}
			}
		}
		if ( m_flagFog )
		{
			float32_t	zOffset = zVertex - m_zFogNear ;
			if ( zOffset > 0.0f )
			{
				float32_t	zFog = zOffset * m_zFogDistance ;
				if ( zFog < 1.0f )
				{
					uint32_t	nFog = eslRoundR32ToInt( zFog * 256.0f ) ;
					uint32_t	nCompFog = 0x100 - nFog ;
					clrLooks.rgbMul = clrLooks.rgbMul.imul(nCompFog) ;
					clrLooks.rgbAdd = clrLooks.rgbAdd.imul(nCompFog) ;
					clrLooks.rgbAdd += m_rgbFogColor.imul(nFog) ;
				}
				else
				{
					clrLooks.rgbMul.ui32 = 0 ;
					clrLooks.rgbAdd = m_rgbFogColor ;
				}
			}
		}
	}
}

// 光源効果計算
//////////////////////////////////////////////////////////////////////////////
void S3DRenderingShader::CalculateLightEffect
	( SHADING_PARAMETER & sdp, const LIGHT & light,
		const S3DVector & vNormal,
		const S3DVector & vVertex, float32_t fpFocusParam )
{
	//
	// 拡散反射光効果計算
	//
	FLOAT_UINT32	fuiNormalLight ;
	FLOAT_UINT32	fuiFocusParam ;
	fuiNormalLight.fp32 = (vNormal | light.vDirection) ;
	fuiFocusParam.fp32 = fpFocusParam ;
	//
	int32_t	flagLightPlane =
				fuiNormalLight.ui32
					| (~(fuiFocusParam.ui32
							^ fuiNormalLight.ui32) & sdp.maskDoubleSide) ;
	fuiNormalLight.ui32 &= (flagLightPlane >> 31) & 0x7FFFFFFF ;
	//
	if ( fuiNormalLight.ui32 != 0 )
	{
		fuiNormalLight.fp32 *= sdp.fpDiffusion ;
		//
		sdp.rgbDiffusion.Red +=
			eslRoundR32ToInt( light.rgbColor.Red * fuiNormalLight.fp32 ) ;
		sdp.rgbDiffusion.Green +=
			eslRoundR32ToInt( light.rgbColor.Green * fuiNormalLight.fp32 ) ;
		sdp.rgbDiffusion.Blue +=
			eslRoundR32ToInt( light.rgbColor.Blue * fuiNormalLight.fp32 ) ;
		//
		// 反射光計算
		//
		if ( sdp.fpSpecular != 0 )
		{
			S3DVector	vRefLight = light.vDirection ;
			vRefLight -= vNormal * (2.0f * (light.vDirection | vNormal)) ;
			//
			float32_t	cosRefLight = (vRefLight | vVertex) ;
			if ( cosRefLight < 0.0f )
			{
				float32_t	cos2RefLight =
								cosRefLight * cosRefLight * sdp.fpSpecular ;
				sdp.rgbLight.Red +=
					eslRoundR32ToInt( light.rgbColor.Red * cos2RefLight ) ;
				sdp.rgbLight.Green +=
					eslRoundR32ToInt( light.rgbColor.Green * cos2RefLight ) ;
				sdp.rgbLight.Blue +=
					eslRoundR32ToInt( light.rgbColor.Blue * cos2RefLight ) ;
			}
		}
	}
}

