
// 光源情報
uniform lowp vec3		u_vLightAmbientColor ;							// 環境光
uniform lowp vec3		u_vLightAmbientColorMul ;						// 環境光（乗算）
uniform mediump int		u_countLight ;									// 通常光源数
uniform mediump int		u_typeLighting[MAX_LIGHT_COUNT] ;				// 光源種別
uniform lowp vec3		u_vLightColor[MAX_LIGHT_COUNT] ;				// 光源色
uniform mediump float	u_fpLightBrightness[MAX_LIGHT_COUNT] ;			// 輝度
uniform mediump float	u_fpLightAttenuationPower[MAX_LIGHT_COUNT] ;	// 距離減衰力
uniform highp vec3		u_vLightPosition[MAX_LIGHT_COUNT] ;				// 光源座標（グローバル）
uniform mediump vec3	u_vLightDirection[MAX_LIGHT_COUNT] ;			// 光源ベクトル（グローバル・正規化済み）
uniform mediump float	u_fpLightAngle[MAX_LIGHT_COUNT] ;				// 範囲角 cosθ
uniform mediump float	u_fpLightGradation[MAX_LIGHT_COUNT] ;			// ぼかし範囲 -cosθ


// 標準シェーディング光源効果
void utilDefaultLighting
	( out mediump vec3 vDstMulColor, out mediump vec3 vDstAddColor,
		in mediump vec3 vMulColor, in mediump vec3 vAddColor,
		in mediump vec3 vLightColor,
		in mediump vec3 vLightDirection, in float aShadow,
		in float fpDiffusion, in float fpSpecular, in float fpSpecularPow,
		in bool flagDoubleSide, in float fpFocusParam,
		in highp vec3 vPosition, in highp vec3 vPosDir, in mediump vec3 vNormal )
{
	//
	// 拡散反射光効果計算
	//
	float	fpNormalLight =
				dot( vNormal, vLightDirection )
					* u_cosShadeCoefficient[0] + u_cosShadeCoefficient[1] ;
	if ( ((fpNormalLight < 0.0) && !flagDoubleSide)
		|| (flagDoubleSide && (fpFocusParam * fpNormalLight >= 0.0)) )
	{
		fpNormalLight = abs( fpNormalLight ) ;
		#ifdef	_FRAGMENT_SHADER
		#ifdef	_PHONG_SHADER
		if ( u_bMaterialToon )
		{
			fpNormalLight =
				(fpNormalLight > u_fpToonShadeThreshold)
							? 1.0 : u_fpToonShadeBrightness ;
		}
		#endif
		#endif
		vMulColor *= 1.0 + fpNormalLight
							* u_fMaterialBackDiffusion * (1.0 - aShadow) ;
		vMulColor +=
			vLightColor * (fpNormalLight * fpDiffusion * aShadow) ;
		//
		// 鏡面反射光計算
		//
		if ( fpSpecular != 0.0 )
		{
			vec3	vRefLight = vLightDirection ;
			vRefLight -= vNormal * (2.0 * dot( vLightDirection, vNormal )) ;
			//
			float	cosRefLight = - dot( vRefLight, vPosDir ) ;
			if ( cosRefLight > 0.0 )
			{
				cosRefLight = pow( cosRefLight, fpSpecularPow ) ;
				#ifdef	_FRAGMENT_SHADER
				#ifdef	_PHONG_SHADER
				if ( u_bMaterialToon )
				{
					if ( cosRefLight > 0.5 )
					{
						cosRefLight = 1.0 ;
					}
				}
				#endif
				#endif
				vAddColor +=
					vLightColor * (u_vMaterialSpecularColor
								* (cosRefLight * fpSpecular * aShadow)) ;
			}
		}
	}
	else
	{
		vMulColor *= 1.0 - fpNormalLight * u_fMaterialBackDiffusion ;
		//
		vec3	vBackLightColor = vLightColor * (abs( fpNormalLight ) * aShadow) ;
		vMulColor += vBackLightColor * u_vMaterialBackLightMul ;
		vAddColor += vBackLightColor * u_vMaterialBackLightAdd ;
	}
	#ifdef	_FRAGMENT_SHADER
	#ifdef	_PHONG_SHADER
	float	cosRimLight = - dot( vPosDir, vLightDirection ) ;
	if ( (u_fMaterialRimLight > 0.0) && (cosRimLight > 0.0) )
	{
		float	cosFocus = sqrt( 1.0 - min( fpFocusParam * fpFocusParam, 1.0 ) ) ;
		float	fpLength = cosFocus / max( abs( fpFocusParam ), 0.00001 ) ;
		float	fpRimLight = fpLength * cosRimLight * u_fMaterialRimDeepness ;
		fpRimLight = min( fpRimLight * fpRimLight, 2.0 ) * u_fMaterialRimLight ;
		//
		vec3	vRimColor =
					vLightColor * (u_vMaterialRimColor
										* (fpRimLight * aShadow * 0.5)) ;
		vMulColor += vLightColor * vRimColor ;
		vAddColor += vLightColor * vRimColor ;
	}
	#endif
	#endif
	vDstMulColor = vMulColor ;
	vDstAddColor = vAddColor ;
}


// 標準シェーディング
void utilDefaultShading
	( out mediump vec3 vDstColor,
		out mediump vec3 vDstEmissionColor,
		out mediump vec3 vDstAmbientColor,
		in vec4 vDiffusionTexture,
		in lowp vec3 vMaterialMulColor,
		in mediump vec3 vLightMulColor,
		in mediump vec3 vSrcAmbientColor,
		in vec3 vMaterialLightShade )
{
	vec3	vMulColor = vMaterialMulColor * vLightMulColor + vMaterialLightShade ;
	vec3	vAmbientColor = vMaterialMulColor * vSrcAmbientColor + vMaterialLightShade ;
	//
	float	nalpha = 1.0 - vDiffusionTexture.a ;
	vec3	vDiffusion = vDiffusionTexture.rgb * vMulColor ;
	vec3	vSDiffusion = min( vDiffusion, vec3( 1.0, 1.0, 1.0 ) ) ;
	vec3	vOFDiffusion = vDiffusion - vSDiffusion ;
	float	fpOFDiffusion =
				(vOFDiffusion.r + vOFDiffusion.g + vOFDiffusion.b) * 0.5 ;
	vDstColor = vSDiffusion + vec3( fpOFDiffusion, fpOFDiffusion, fpOFDiffusion ) ;
	vDstEmissionColor = vOFDiffusion ;
	vDstAmbientColor = vDiffusionTexture.rgb * vAmbientColor ;
}


// 拡散反射サンプリング
vec4 utilSampleDiffusion( in tex_coord_t uv, in highp vec3 vCVPosition )
{
	if ( u_bMaterialTexture )
	{
		return	tex_sample_px( u_samplerMaterialTexture, uv ) ;
	}
	else
	{
		return	vec4( 0.0, 0.0, 0.0, 0.0 ) ;
	}
}


// 発光サンプリング
vec4 utilSampleEmission( in tex_coord_t uv, in highp vec3 vCVPosition )
{
#ifndef	_DISABLE_LUMINOUS_TEXTURE
#ifdef	_FRAGMENT_SHADER
	if ( u_fpLuminousTexture > 0.00390625 )
	{
		return	tex_sample_px( u_samplerLuminousTexture, uv ) * u_fpLuminousTexture ;
	}
#endif
#endif
	return	vec4( 0.0, 0.0, 0.0, 0.0 ) ;
}


// 法線サンプリング
vec4 utilSampleNormal( in tex_coord_t uv, in highp vec3 vCVPosition )
{
#ifndef	_DISABLE_NORMAL_TEXTURE
#ifdef	_PHONG_SHADER
#ifdef	_FRAGMENT_SHADER
	if ( u_fpNormalTexture > 0.0 )
	{
		return	(tex_sample_px( u_samplerNormalTexture, uv ) - 0.5) * 2.0 ;
	}
#endif
#endif
#endif
	return	vec4( 0.0, 0.0, 0.0, 0.0 ) ;
}


// 標準フォッグ処理
vec3 utilDefaultFogging
	( in mediump vec3 vSrcColor, in vec3 rgbFog, in float zFog )
{
	return	vSrcColor * (1.0 - zFog) + rgbFog * zFog ;
}


