
#define	_IMPLEMENT_DEFAULT_SHADER	1

void effect_shading
	( out mediump vec3 vDstMulColor, out mediump vec3 vDstAddColor,
		in mediump vec3 vMulColor, in mediump vec3 vAddColor,
		in int iLight, in mediump vec3 vLightColor, in mediump vec3 vLightDirection,
		in float fpDiffusion, in float fpSpecular, in float fpSpecularPow,
		in bool flagDoubleSide, in float fpFocusParam,
		in highp vec3 vPosition, in highp vec3 vPosDir, in mediump vec3 vNormal )
{
	float	aShadow = 1.0 ;
	#ifndef	_DISABLE_SHADOWMAPPING
	#ifdef	_FRAGMENT_SHADER
	//
	// シャドウマップ効果計算
	//
	aShadow = effect_light_shadow_mapping
				( iLight, vPosition, vLightDirection, vNormal ) ;
	#endif
	#endif
	//
	// 拡散反射／鏡面反射／その他光源効果計算
	//
	#ifdef	_USER_SUB_LIGHTING
		subLighting
			( vDstMulColor, vDstAddColor, vMulColor, vAddColor,
				vLightColor, vLightDirection, aShadow,
				fpDiffusion, fpSpecular, fpSpecularPow,
				flagDoubleSide, fpFocusParam, vPosition, vPosDir, vNormal ) ;
	#else
		utilDefaultLighting
			( vDstMulColor, vDstAddColor, vMulColor, vAddColor,
				vLightColor, vLightDirection, aShadow,
				fpDiffusion, fpSpecular, fpSpecularPow,
				flagDoubleSide, fpFocusParam, vPosition, vPosDir, vNormal ) ;
	#endif
}


void effect_shading_light_at
	( out mediump vec3 vDstLightMulColor,
		out mediump vec3 vDstLightAddColor,
		in mediump vec3 vLightMulColor,
		in mediump vec3 vLightAddColor,
		in int iLight, in float fpSpecular, in float fpSpecularPow,
		in highp vec3 vPosition, in mediump vec3 vNormal,
		in float fpFocusParam, in highp vec3 vPosDir )
{
	int	typeLight = u_typeLighting[iLight] ;
	vDstLightMulColor = vLightMulColor ;
	vDstLightAddColor = vLightAddColor ;
	if ( (typeLight == LIGHT_POINT) || (typeLight == LIGHT_SPOT) )
	{
		// 点光源／スポットライト
		vec3	vLightPos =
					(u_mat4CameraView
						* vec4( u_vLightPosition[iLight], 1.0 )).xyz ;
		vec3	vDirection = vPosition - vLightPos ;
		float	fpDistance = max( length( vDirection ), 1.0e-5 ) ;
		vDirection /= fpDistance ;
		//
		float	r = pow( 1.0 / fpDistance,
							u_fpLightAttenuationPower[iLight] ) ;
		r *= u_fpLightBrightness[iLight] ;
		//
		if ( typeLight == LIGHT_SPOT )
		{
			vec3	vLightDir =
						u_mat3CameraViewForNormal * u_vLightDirection[iLight] ;
			r = min( r, 1.0 ) ;
			float	cosDir = dot( vDirection, vLightDir ) ;
			cosDir -= u_fpLightAngle[iLight] ;
			if ( cosDir <= 0.0 )
			{
				return ;
			}
			if ( cosDir < u_fpLightGradation[iLight] )
			{
				r *= cosDir / u_fpLightGradation[iLight] ;
			}
		}
		else
		{
			if ( r > 1.0 )
			{
				r = min( sqrt( r ), 2.0 ) ;
			}
		}
		effect_shading
			( vDstLightMulColor, vDstLightAddColor,
				vLightMulColor, vLightAddColor,
				iLight, u_vLightColor[iLight] * r, vDirection,
				u_fMaterialDiffusion,
				fpSpecular, fpSpecularPow,
				u_bMaterialDoubleSide,
				fpFocusParam, vPosition, vPosDir, vNormal ) ;
	}
	else if ( typeLight == LIGHT_VECTOR )
	{
		// 無限遠光源
		effect_shading
			( vDstLightMulColor, vDstLightAddColor,
				vLightMulColor, vLightAddColor,
				iLight, u_vLightColor[iLight] * u_fpLightBrightness[iLight],
				normalize( u_mat3CameraViewForNormal * u_vLightDirection[iLight] ),
				u_fMaterialDiffusion,
				fpSpecular, fpSpecularPow,
				u_bMaterialDoubleSide,
				fpFocusParam, vPosition, vPosDir, vNormal ) ;
	}
}


void effect_shading_fog_at
	( out mediump vec3 vDstMulColor,
		out mediump vec3 vDstAddColor,
		in mediump vec3 vMulColor,
		in mediump vec3 vAddColor,
		in int iLight, in highp vec3 vPosition )
{
	vec3	vLightPos =
				(u_mat4CameraView
					* vec4( u_vLightPosition[iLight], 1.0 )).xyz ;
	vec3	vLightDir =
				normalize( u_mat3CameraViewForNormal
								* u_vLightDirection[iLight] ) ;
	vec3	vDeepness = vPosition - vLightPos ;
	float	fpDeepness = dot( vDeepness, vLightDir ) ;
	if ( fpDeepness > 0.0 )
	{
		// 対象点は境界の内側
		if ( dot( vLightPos, vLightDir ) < 0.0 )
		{
			// 視点は境界面の内側
			fpDeepness = min( vPosition.z, fpDeepness ) ;
		}
		fpDeepness = min( fpDeepness * u_fpLightBrightness[iLight], 1.0 ) ;
		//
		#ifdef	_FRAGMENT_SHADER
			vec3	rgbFog =
						u_vLightColor[iLight]
							* (vec3(1.0, 1.0, 1.0) - vMulColor) ;
			#ifdef	_USER_SUB_FOGGING
				vAddColor = subFogging( vAddColor, rgbFog, fpDeepness ) ;
			#else
				vAddColor = utilDefaultFogging( vAddColor, rgbFog, fpDeepness ) ;
			#endif
		#else
			vec3	rgbFog = u_vLightColor[iLight] ;
			#ifdef	_USER_SUB_FOGGING
				vAddColor = subFogging( vAddColor, rgbFog, fpDeepness ) ;
			#else
				vAddColor = utilDefaultFogging( vAddColor, rgbFog, fpDeepness ) ;
			#endif
			vMulColor *= (1.0 - fpDeepness) ;
		#endif
	}
	vDstMulColor = vMulColor ;
	vDstAddColor = vAddColor ;
}


float effect_lighting
	( out mediump vec3 vMulColor,
		out mediump vec3 vAddColor,
		out mediump vec3 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		out highp vec3 vLPosition,
		in highp vec3 vPosition, in mediump vec3 vNormal,
		in tex_coord_t vTextureCoord,
		in lowp vec3 vMaterialMulColor, in lowp vec3 vMaterialAddColor,
		in lowp vec3 vMaterialMulShade, in lowp vec3 vMaterialAddShade,
		in mediump vec3 vEffectMulColor, in mediump vec3 vEffectAddColor )
{
	highp vec3		vPosDir = normalize( vPosition ) ;
	tex_coord_t		vLTextureCoord = vTextureCoord ;
	vLPosition = vPosition ;
	vNormal = normalize( vNormal ) ;
	vDstNormal = vNormal ;
	//
	#ifdef	_FRAGMENT_SHADER
	// テクスチャマッピング（拡散反射成分）
	vec4	vDiffusionTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
		#ifdef	_USER_SUB_DIFFUSION
		vDiffusionTexture =
			subSampleDiffusion
				( tex_coord_map
					( vLTextureCoord,
						u_vMaterialTextureScale,
						u_vMaterialTextureBase ), vLPosition ) ;
		#else
		if ( u_bMaterialTexture )
		{
			vDiffusionTexture =
				tex_sample_px
					( u_samplerMaterialTexture,
						tex_coord_map
							( vLTextureCoord,
								u_vMaterialTextureScale,
								u_vMaterialTextureBase ) ) ;
		#endif
		if ( u_iMaterialTriming != ALPHA_NO_TRIM )
		{
			float	alphaThreshold = 0.5 ;
			if ( u_iMaterialTriming == ALPHA_DITHERING )
			{
				alphaThreshold = utilPatternDithering() ;
			}
			if ( vDiffusionTexture.a < alphaThreshold )
			{
				vDiffusionTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
				vMulColor = vec3( 1.0, 1.0, 1.0 ) ;
				vAddColor = vec3( 0.0, 0.0, 0.0 ) ;
				vEmissionColor = vAddColor ;
				vAmbientColor = vAddColor ;
				vDiffusionColor = vAddColor ;
				vDstSpecular = vec3( 0.0, 0.0, 1.0 ) ;
				return	0.0 ;
			}
			else
			{
				vDiffusionTexture *= 1.0 / (vDiffusionTexture.a + 0.05) ;
				vDiffusionTexture.a = 1.0 ;
			}
		}
		#ifndef	_USER_SUB_DIFFUSION
		}
		#endif
	#endif
	
	#ifndef	_DISABLE_HEIGHT_TEXTURE
	#ifdef	_FRAGMENT_SHADER
	if ( u_fpBumpHeight != 0.0 )
	{
		//
		// 標高テクスチャ
		//
		vec3	vPolyNormal = vNormal ;
		vec3	vUVCross = cross( v_vTextureAxisX, v_vTextureAxisY ) ;
		vUVCross *= 1.0 / dot( vUVCross, vUVCross ) ;
		float	cosViewNormal = dot( vPosDir, vPolyNormal ) ;
		float	sinViewNormal = sqrt( 1.0 - cosViewNormal * cosViewNormal ) ;
		if ( (cosViewNormal != 0.0) && (sinViewNormal != 0.0) )
		{
			float	fpShiftLength =
						sinViewNormal * u_fpBumpHeight / - cosViewNormal ;
			vec3	vShiftBase =
						- normalize( vPosDir - vPolyNormal * cosViewNormal ) ;
			//
			#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
			vec3	vShiftUVBase =
						vec3( dot( cross( vShiftBase, v_vTextureAxisY ), vUVCross ),
								dot( cross( v_vTextureAxisX, vShiftBase ), vUVCross ), 0.0 ) ;
			vec3	vHeightTextureScale = vec3( u_vHeightTextureScale, 1.0 ) ;
			vec3	vHeightTextureBase = vec3( u_vHeightTextureBase, 0.0 ) ;
			#else
			vec2	vShiftUVBase =
						vec2( dot( cross( vShiftBase, v_vTextureAxisY ), vUVCross ),
								dot( cross( v_vTextureAxisX, vShiftBase ), vUVCross ) ) ;
			vec2	vHeightTextureScale = u_vHeightTextureScale ;
			vec2	vHeightTextureBase = u_vHeightTextureBase ;
			#endif
			//
			float	a = - cosViewNormal / sinViewNormal ;
			float	x1 = fpShiftLength ;
			vec4	vHeight =
				tex_sample_px
					( u_samplerHeightTexture,
							(vTextureCoord + vShiftUVBase * x1)
								* vHeightTextureScale + vHeightTextureBase ) ;
			float	y1 = vHeight.x * u_fpBumpHeight ;
			float	x = 0.0 ;
			float	y = y1 ;
			//
			for ( int i = 1; i <= 4; i ++ )
			{
				x = fpShiftLength * (1.0 - (i * 0.25)) ;
				vHeight =
					tex_sample_px
						( u_samplerHeightTexture,
							(vTextureCoord + vShiftUVBase * x)
								* vHeightTextureScale + vHeightTextureBase ) ;
				y = vHeight.x * u_fpBumpHeight ;
				if ( y >= a * x * 0.99 )
				{
					break ;
				}
				x1 = x ;
				y1 = y ;
			}
			for ( int i = 0; i < 3; i ++ )
			{
				float	x2 = (x1 + x) * 0.5 ;
				vHeight =
					tex_sample_px
						( u_samplerHeightTexture,
							(vTextureCoord + vShiftUVBase * x2)
								* vHeightTextureScale + vHeightTextureBase ) ;
				float	y2 = vHeight.x * u_fpBumpHeight ;
				//
				if ( y2 >= a * x2 * 0.99 )
				{
					x = x2 ;
					y = y2 ;
				}
				else
				{
					x1 = x2 ;
					y1 = y2 ;
				}
			}
			// 直線 (x1,y1)-(x2,y2)
			// dx= x2 - x1, dy = y2 - y1 : y = (dy * x + dx * y1 - dy * x1) / dy
			// y = a * x との交点の x : (dx * y1 - dy * x1) / (dx * a - dy)
			float	dy = y1 - y ;
			float	dx = x1 - x ;
			x1 = (dx * y - dy * x) / (a * dx - dy) ;
			y1 = a * x1 ;
			//
			vLPosition += vShiftBase * x1 + vNormal * y1 ;
			vLTextureCoord += vShiftUVBase * x1 ;
			vPosDir = normalize( vLPosition ) ;
		}
	}
	#endif
	#endif
	//
	// マテリアル情報取得
	//
	float	fpMaterialSpecular = u_fMaterialSpecular ;
#ifdef	_FRAGMENT_SHADER
	float	fpMaterialReflection = u_fMaterialReflection ;
#else
	float	fpMaterialReflection = 0.0 ;
#endif
	float	fpMaterialSpecularPow = u_fMaterialSpecularPow ;
	//
	#ifndef _DISABLE_SPECULAR_TEXTURE
	#ifdef	_FRAGMENT_SHADER
	if ( u_bMaterialSpecularTexture )
	{
		vec4	vSpecularSet =
					tex_sample_px
						( u_samplerSpecularTexture,
							vLTextureCoord
								* u_vSpecularTextureScale + u_vSpecularTextureBase ) ;
		fpMaterialSpecular *= vSpecularSet.x ;
		fpMaterialReflection *= vSpecularSet.y ;
		fpMaterialSpecularPow *= max( vSpecularSet.z, 0.00390625 ) ;
	}
	#endif
	#endif
	//
	vAmbientColor = u_vLightAmbientColor
							+ vec3( u_fMaterialAmbient,
									u_fMaterialAmbient, u_fMaterialAmbient )  ;
	vDstSpecular.x = fpMaterialSpecular ;
	vDstSpecular.y = fpMaterialReflection ;
	vDstSpecular.z = 1.0 / max( fpMaterialSpecularPow, 0.00390625 ) ;
	//
	// 光源効果計算
	//
	mediump vec3	vLightMulColor = vAmbientColor * u_vLightAmbientColorMul ;
	mediump vec3	vLightAddColor = vec3( 0.0, 0.0, 0.0 ) ;
	float			fpFocusParam ;
	//
	#ifndef	_DISABLE_NORMAL_TEXTURE
	#ifdef	_FRAGMENT_SHADER
	if ( u_fpNormalTexture > 0.0 )
	{
		// 法線テクスチャ（r,g,b 成分に (x,y,z)*0.5+0.5 を格納）
		if ( u_bMaterialDoubleSide && (dot( vPosDir, vNormal ) >= 0.0) )
		{
			vNormal = - vNormal ;
		}
		#ifdef	_USER_SUB_NORMAL
		vec4	vTexture =
			subSampleNormal
				( tex_coord_map
					( vLTextureCoord,
						u_vNormalTextureScale,
						u_vNormalTextureBase ), vLPosition ) ;
		#else
		vec4	vTexture =
					(tex_sample_px
						( u_samplerNormalTexture,
							tex_coord_map
								( vLTextureCoord,
									u_vNormalTextureScale,
									u_vNormalTextureBase ) ) - 0.5) * 2.0 ;
		#endif
		vec3	vNormalTex = normalize( vTexture.rgb ) ;
		vec3	vTempNormal =
					normalize( v_vTextureAxisX ) * vNormalTex.r
					+ normalize( v_vTextureAxisY ) * vNormalTex.g
					+ vNormal * vNormalTex.b ;
		vNormal += (vTempNormal - vNormal) * u_fpNormalTexture ;
		vNormal = normalize( vNormal ) ;
		vDstNormal = vNormal ;
	}
	#endif
	#endif
	//
	fpFocusParam = dot( vPosDir, vNormal ) ;
	//
	#ifdef GL_ES
	if ( u_countLight >= 1 )
	{
		effect_shading_light_at
			( vLightMulColor, vLightAddColor,
				vLightMulColor, vLightAddColor,
				0, fpMaterialSpecular, fpMaterialSpecularPow,
				vLPosition, vNormal, fpFocusParam, vPosDir ) ;
		#ifndef	_DISABLE_LIGHT_OVER_1
		if ( u_countLight >= 2 )
		{
			effect_shading_light_at
				( vLightMulColor, vLightAddColor,
					vLightMulColor, vLightAddColor,
					1, fpMaterialSpecular, fpMaterialSpecularPow,
					vLPosition, vNormal, fpFocusParam, vPosDir ) ;
			#ifndef	_DISABLE_LIGHT_OVER_2
			if ( u_countLight >= 3 )
			{
				effect_shading_light_at
					( vLightMulColor, vLightAddColor,
						vLightMulColor, vLightAddColor,
						2, fpMaterialSpecular, fpMaterialSpecularPow,
						vLPosition, vNormal, fpFocusParam, vPosDir ) ;
				#ifndef	_DISABLE_LIGHT_OVER_3
				if ( u_countLight >= 4 )
				{
					effect_shading_light_at
						( vLightMulColor, vLightAddColor,
							vLightMulColor, vLightAddColor,
							3, fpMaterialSpecular, fpMaterialSpecularPow,
							vLPosition, vNormal, fpFocusParam, vPosDir ) ;
				}
				#endif
			}
			#endif
		}
		#endif
	}
	#else
	for ( int i = 0; (i < u_countLight) && (i < MAX_LIGHT_COUNT); i ++ )
	{
		effect_shading_light_at
			( vLightMulColor, vLightAddColor,
				vLightMulColor, vLightAddColor,
				i, fpMaterialSpecular, fpMaterialSpecularPow,
				vLPosition, vNormal, fpFocusParam, vPosDir ) ;
	}
	#endif
	//
	// 表面属性色効果計算
	//
	#ifdef	_FRAGMENT_SHADER
	vEmissionColor = vLightAddColor * (1.0 - u_fMaterialEmission) ;
	if ( u_bMaterialTexture )
	{
		mediump vec3	vTempDstColor ;
		mediump vec3	vTempEmissionColor ;
		vec3			vTempMulShade =
							vMaterialMulShade * u_vLightAmbientColorMul ;

		#ifdef	_USER_SUB_SHADING
			subShading
				( vTempDstColor, vTempEmissionColor, vAmbientColor,
					vDiffusionTexture,
					vMaterialMulColor, vLightMulColor,
					vAmbientColor, vTempMulShade ) ;
		#else
			utilDefaultShading
				( vTempDstColor, vTempEmissionColor, vAmbientColor,
					vDiffusionTexture,
					vMaterialMulColor, vLightMulColor,
					vAmbientColor, vTempMulShade ) ;
		#endif

		float	nalpha = 1.0 - vDiffusionTexture.a ;
		vAddColor = vMaterialAddColor * vLightMulColor
							+ vLightAddColor + vMaterialAddShade ;
		vAddColor = vAddColor * vDiffusionTexture.a + vTempDstColor ;
		vAddColor = min( vAddColor, vec3( 1.0, 1.0, 1.0 ) ) ;
		vAddColor = vAddColor * vEffectMulColor
						+ vEffectAddColor * vDiffusionTexture.a ;
		vMulColor = vec3( nalpha, nalpha, nalpha ) ;
		vEmissionColor += vTempEmissionColor ;
		//
		// 遅延シェーディング用拡散反射成分
		vDiffusionColor =
			vDiffusionTexture.rgb
				* (vMaterialMulColor + vTempMulShade)
						* vEffectMulColor * u_fMaterialDiffusion ;
	}
	else
	{
		vec3	vTempAddShade = vMaterialAddShade * u_vLightAmbientColorMul ;
		vAddColor = (vMaterialAddColor * vEffectMulColor + vEffectAddColor)
						* vLightMulColor + vLightAddColor + vTempAddShade ;
		vMulColor = vMaterialMulColor * vEffectMulColor + vMaterialMulShade ;
		vAmbientColor =
					(vMaterialAddColor * vAmbientColor
										+ vTempAddShade) * vEffectMulColor ;
		vDiffusionColor =
					(vMaterialAddColor + vTempAddShade)
								* vEffectMulColor * u_fMaterialDiffusion ;
	}
	#ifndef _DISABLE_LIGHTMAP_AO_TEXTURE
	#ifdef	_ENABLE_EXTEND_ATTR_
	if ( u_bMaterialGlobalAOTexture )
	{
		vec4	vAO = texture2D( u_samplerGlobalAOTexture, v_vExAttrElement0.xy ) ;
		vAddColor *= vAO.rgb ;
	}
	#endif
	#endif
	//
	// 発光テクスチャ
	#ifndef	_DISABLE_LUMINOUS_TEXTURE
	if ( (u_iMaterialTriming == ALPHA_NO_TRIM) || (vMulColor.r <= 0.5) )
	{
		#ifdef	_USER_SUB_EMISSION
			vec4	vTexture =
						subSampleEmission
							( tex_coord_map
								( vLTextureCoord,
									u_vLuminousTextureScale,
									u_vLuminousTextureBase ), vLPosition ) ;
			vAddColor += vTexture.rgb ;
			vEmissionColor += vTexture.rgb ;
		#else
		if ( u_fpLuminousTexture > 0.00390625 )
		{
			vec4	vTexture =
				tex_sample_px
					( u_samplerLuminousTexture,
						tex_coord_map
							( vLTextureCoord,
								u_vLuminousTextureScale,
								u_vLuminousTextureBase ) ) ;
			vTexture *= u_fpLuminousTexture ;
			vAddColor += vTexture.rgb ;
			vEmissionColor += vTexture.rgb ;
		}
		#endif
	}
	#endif

	#ifndef	_DISABLE_ENVIRONMENT_MAPPING
	if ( (u_typeEnvironmentMapping != ENV_MAPPING_NOTHING)
							&& (fpMaterialReflection > 0.00390625) )
	{
		//
		// 環境マッピング
		//
		vec3	vRefDir = vPosDir ;
		vRefDir -= vNormal * (2.0 * dot( vPosDir, vNormal )) ;
		vRefDir = normalize( u_mat3EnvironmentMapping * vRefDir ) ;
		//
		vec4	vTexture ;
		#ifndef	_DISABLE_ENVIRONMENT_CUBMAP
		if ( u_typeEnvironmentMapping == ENV_MAPPING_CUBE )
		{
			vTexture =
				textureCube( u_samplerEnvironmentCube, vRefDir ) ;
		}
		else
		#endif
		#ifndef	_DISABLE_ENVIRONMENT_SPHERE
		if ( u_typeEnvironmentMapping == ENV_MAPPING_SPHERE )
		{
			vec2	vTexCoord =
						vec2( vRefDir.x * 0.5 + 0.5,
								vRefDir.z * 0.25 + 0.25 ) ;
			if ( vRefDir.y < 0.0 )
			{
				vTexCoord.y += 0.5 ;
			}
			vTexture =
				texture2D( u_samplerEnvironmentMapping,
							vTexCoord * u_vEnvMapingTextureScale
											+ u_vEnvMapingTextureBase ) ;
		}
		else
		{
			vTexture =
				texture2D( u_samplerEnvironmentMapping,
						vec2( vRefDir.x + 1.0, vRefDir.z + 1.0 )
								* (u_vEnvMapingTextureScale * 0.5)
											+ u_vEnvMapingTextureBase ) ;
		}
		#else
		{
			vTexture = vec4( 0.0, 0.0, 0.0, 1.0 ) ;
		}
		#endif
//		float	fpDstMulTex ;
		vTexture *= fpMaterialReflection ;
//		fpDstMulTex = (1.0 - vTexture.a) ;
//		vMulColor *= fpDstMulTex ;
//		vAddColor = vAddColor * fpDstMulTex + vTexture.rgb ;
		vAddColor += vTexture.rgb ;
	}
	#endif
	#ifndef	_DISABLE_ALPHA_MAPPING
	if ( u_bMaterialAlphaTexture )
	{
		//
		// αテクスチャマッピング
		//
		float	alpha = 
			tex_sample_px
				( u_samplerAlphaMapping,
					tex_coord_map
						( vLTextureCoord,
							u_vAlphaMapingTextureScale,
							u_vAlphaMapingTextureBase ) ).r ;
		alpha = (alpha - u_fpAlphaBase) * u_fpAlphaCoefficient ;
		alpha = max( min( alpha, 1.0 ), 0.0 ) ;
		if ( u_iMaterialTriming != ALPHA_NO_TRIM )
		{
			alpha = (alpha >= 0.5) ? 1.0 : 0.0 ;
		}
		float	nalpha = 1.0 - alpha ;
		vMulColor = vec3( nalpha, nalpha, nalpha ) + vMulColor * alpha ;
		vAddColor *= alpha ;
	}
	#endif

	#else	// ifndef _FRAGMENT_SHADER
	vAddColor = (vMaterialAddColor * vEffectMulColor + vEffectAddColor)
					* vLightMulColor + vLightAddColor + vMaterialAddShade ;
	vMulColor = (vMaterialMulColor * vEffectMulColor + vEffectAddColor)
					* vLightMulColor + vMaterialMulShade ;
	vEmissionColor = vLightAddColor * (1.0 - u_fMaterialEmission) ;
	vAmbientColor =
				(vMaterialAddColor * vAmbientColor
									+ vMaterialAddShade) * vEffectMulColor ;
	//
	#endif
	//
	vEmissionColor += vAddColor * u_fMaterialEmission ;
	//
	// 透明度・透明深度計算
	//
	float	alphaEffect = u_fMaterialAlpha ;
	alphaEffect -= pow( abs( fpFocusParam ),
						u_fMaterialDeepnessPow ) * u_fMaterialDeepness ;
	alphaEffect = clamp( alphaEffect, 0.0, 1.0 ) * u_fpEffectAlpha ;
	//
	#ifdef	_FRAGMENT_SHADER
	vAddColor *= alphaEffect ;
	alphaEffect *= 1.0 - (vMulColor.r + vMulColor.g + vMulColor.b) * 0.333333333 ;

	#ifndef	_DISABLE_ENVIRONMENT_MAPPING
	#ifndef	_DISABLE_REFRACTION_MAPPING
	if ( (u_typeEnvironmentRefraction != ENV_MAPPING_NOTHING)
									&& (u_fpRefractionRatio != 0.0) )
	{
		//
		// 環境マッピング（屈折）
		//
		float	fpRefDelta = dot( vPosDir, vNormal ) * u_fpRefractionRatio ;
		vec3	vRefDir = vNormal * fpRefDelta + vPosDir ;
		//
		vec4	vTexture ;
		#ifndef	_DISABLE_ENVIRONMENT_CUBMAP
		if ( u_typeEnvironmentRefraction == ENV_MAPPING_CUBE )
		{
			vRefDir = normalize( u_mat3EnvironmentMapping * vRefDir ) ;
			vTexture =
				textureCube( u_samplerEnvironmentCube, vRefDir ) ;
		}
		else
		#endif
		#ifndef	_DISABLE_ENVIRONMENT_VIEWPORT
		if ( u_typeEnvironmentRefraction == ENV_MAPPING_VIEWPORT )
		{
			float	m22 = u_mat4PerspectiveView[2].z ;
			float	m32 = u_mat4PerspectiveView[2].w ;
			float	m23 = u_mat4PerspectiveView[3].z ;
			float	m33 = u_mat4PerspectiveView[3].w ;
			vec4	vPersLPos = u_mat4PerspectiveView * vec4( vLPosition, 1.0 ) ;
			vec3	vViewLPos = vPersLPos.xyz * 0.5 / vPersLPos.w + 0.5 ;
			float	zLDepth = texture2D( u_samplerViewportDepth, vViewLPos.xy ).x * 2.0 - 1.0 ;
			float	zValue = (m23 - m33 * zLDepth) / (m32 * zLDepth - m22) ;
			float	zRefDepth = (zValue - vLPosition.z) * u_fpRefractionParam ;
			//
			vec4	vPersRPos =
				u_mat4PerspectiveView
					* vec4( vLPosition + vRefDir * zRefDepth, 1.0 ) ;
			vec2	vViewRPos = vPersRPos.xy * (0.5 / vPersRPos.w) + 0.5 ;
			vec2	vViewDeltaX1 = vec2( u_vViewportUnit.x, 0.0 ) ;
			vec2	vViewDeltaY1 = vec2( 0.0, u_vViewportUnit.y ) ;
			float	zBufDepth = texture2D( u_samplerViewportDepth, vViewRPos ).x ;
			float	zBufDepthDX =
						min( texture2D( u_samplerViewportDepth, vViewRPos - vViewDeltaX1 ).x,
								texture2D( u_samplerViewportDepth, vViewRPos + vViewDeltaX1 ).x ) ;
			float	zBufDepthDY =
						min( texture2D( u_samplerViewportDepth, vViewRPos - vViewDeltaY1 ).x,
								texture2D( u_samplerViewportDepth, vViewRPos + vViewDeltaY1 ).x ) ;
			if ( min( zBufDepth, min( zBufDepthDX, zBufDepthDY ) ) < vViewLPos.z )
			{
				vViewRPos = vViewLPos.xy ;
			}
			vTexture = texture2D( u_samplerViewportMapping, vViewRPos ) ;
		}
		else
		#endif
		#ifndef	_DISABLE_ENVIRONMENT_SPHERE
		if ( u_typeEnvironmentRefraction == ENV_MAPPING_SPHERE )
		{
			vRefDir = normalize( u_mat3EnvironmentMapping * vRefDir ) ;
			vec2	vTexCoord =
						vec2( vRefDir.x * 0.5 + 0.5,
								vRefDir.z * 0.25 + 0.25 ) ;
			if ( vRefDir.y < 0.0 )
			{
				vTexCoord.y += 0.5 ;
			}
			vTexture =
				texture2D( u_samplerEnvironmentMapping,
							vTexCoord * u_vEnvMapingTextureScale
											+ u_vEnvMapingTextureBase ) ;
		}
		else
		{
			vRefDir = normalize( u_mat3EnvironmentMapping * vRefDir ) ;
			vTexture =
				texture2D( u_samplerEnvironmentMapping,
						vec2( vRefDir.x + 1.0, vRefDir.z + 1.0 )
								* (u_vEnvMapingTextureScale * 0.5)
											+ u_vEnvMapingTextureBase ) ;
		}
		#else
		{
			vTexture = vec4( 0.0, 0.0, 0.0, 1.0 ) ;
		}
		#endif
		vTexture *= 1.0 - alphaEffect ;
		alphaEffect = 1.0 ;
		vAddColor += vTexture.rgb ;
	}
	#endif
	#endif
	#endif
	//
	// 疑似フォッグ計算
	//
	if ( !u_bMaterialNoFogEffect )
	{
		if ( u_bEnableFog )
		{
			float	zOffset = vLPosition.z - u_zFogNear ;
			if ( zOffset > 0.0 )
			{
				float	zFog = min( zOffset * u_zFogDistance, 1.0 ) ;
				#ifdef	_FRAGMENT_SHADER
					vec3	rgbFog =
								u_rgbFogColor
									* (vec3(1.0, 1.0, 1.0) - vMulColor) ;
					#ifdef	_USER_SUB_FOGGING
						vAddColor = subFogging( vAddColor, rgbFog, zFog ) ;
					#else
						vAddColor = utilDefaultFogging( vAddColor, rgbFog, zFog ) ;
					#endif
				#else
					vec3	rgbFog = u_rgbFogColor ;
					#ifdef	_USER_SUB_FOGGING
						vAddColor = subFogging( vAddColor, rgbFog, zFog ) ;
					#else
						vAddColor = utilDefaultFogging( vAddColor, rgbFog, zFog ) ;
					#endif
					vMulColor *= (1.0 - zFog) ;
				#endif
			}
		}
		#ifdef GL_ES
		if ( u_typeLighting[4] == LIGHT_FOG )
		{
			effect_shading_fog_at
				( vMulColor, vAddColor, vMulColor, vAddColor, 4, vLPosition ) ;
		}
		#else
		for ( int i = u_countLight; i < MAX_LIGHT_COUNT; i ++ )
		{
			if ( u_typeLighting[i] == LIGHT_FOG )
			{
				effect_shading_fog_at
					( vMulColor, vAddColor, vMulColor, vAddColor, i, vLPosition ) ;
			}
			else
			{
				break ;
			}
		}
		#endif
	}
	vAddColor = vAddColor * u_vEffectMulColor
					+ (vec3( 1.0, 1.0, 1.0 ) - vMulColor) * u_vEffectAddColor ;
	return	alphaEffect ;
}


