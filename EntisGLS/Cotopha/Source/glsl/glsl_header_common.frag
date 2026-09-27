
#ifdef GL_ES
	#define	MAX_LIGHT_COUNT		5
	#define	MAX_SHADOWMAP_COUNT	6
#else
	#define	MAX_LIGHT_COUNT		8
	#define	MAX_SHADOWMAP_COUNT	12
#endif

#ifndef	_LIMIT_SHADOWMAP_COUNT
	#define	_LIMIT_SHADOWMAP_COUNT	MAX_SHADOWMAP_COUNT
#endif

#define	LIGHT_NULL		0
#define	LIGHT_AMBIENT	1
#define	LIGHT_VECTOR	2
#define	LIGHT_POINT		4
#define	LIGHT_SPOT		6
#define	LIGHT_FOG		8

#define	ENV_MAPPING_NOTHING		0
#define	ENV_MAPPING_HEMISPHERE	1
#define	ENV_MAPPING_SPHERE		2
#define	ENV_MAPPING_CUBE		3
#define	ENV_MAPPING_VIEWPORT	4

#define	ALPHA_NO_TRIM	0
#define	ALPHA_TRIMING	1
#define	ALPHA_DITHERING	2

#ifndef	MAX_BONE_PALETTE
	#ifdef	_DISABLE_BONE_OVER_2
		#define	MAX_BONE_PALETTE	2
	#else
		#ifdef	_DISABLE_BONE_OVER_4
			#define	MAX_BONE_PALETTE	4
		#else
			#ifdef	_DISABLE_BONE_OVER_6
				#define	MAX_BONE_PALETTE	6
			#else
				#ifdef	_DISABLE_BONE_OVER_8
					#define	MAX_BONE_PALETTE	8
				#else
					#ifdef	_DISABLE_BONE_OVER_12
						#define	MAX_BONE_PALETTE	12
					#else
						#define	MAX_BONE_PALETTE	16
					#endif
				#endif
			#endif
		#endif
	#endif
#endif



///////////////////////////////////////////////////////////////////////////////
// varying 頂点情報
///////////////////////////////////////////////////////////////////////////////

#ifdef	_VERTEX_SHADER
	#ifdef	_LINK_GEOMETRY_SHADER
		#define	v_vPosition			g_vPosition
		#define	v_vNormal			g_vNormal
		#define	v_vTextureAxisX		g_vTextureAxisX
		#define	v_vTextureAxisX		g_vTextureAxisX
		#define	v_vTextureCoord		g_vTextureCoord
		#define	v_vVertexMulColor	g_vVertexMulColor
		#define	v_vVertexAddColor	g_vVertexAddColor

		#ifdef	_ENABLE_EXTEND_ATTR_
			#define	v_vExAttrElement0	g_vExAttrElement0
		#endif
	#endif
#endif

// 頂点情報
varying highp vec4		v_vPosition ;			// 頂点座標（グローバル）
varying mediump vec3	v_vNormal ;				// 法線
#ifdef	_PHONG_SHADER
varying mediump vec3	v_vTextureAxisX ;		// テクスチャマッピングｘ基底
varying mediump vec3	v_vTextureAxisY ;		// テクスチャマッピングｙ基底
#endif
varying mediump vec2	v_vTextureCoord ;		// UV座標
varying mediump vec4	v_vVertexMulColor ;		// 頂点色
varying mediump vec4	v_vVertexAddColor ;

#ifdef	_ENABLE_EXTEND_ATTR_
varying highp vec4		v_vExAttrElement0 ;
#endif


#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
	#define	tex_coord_t		mediump vec3
	#ifdef	_ENABLE_MATERIAL_TEXTURE_ARRAY
		#define	tex_sampler_t	sampler2DArray
		#define	tex_sample_px	texture2DArray
	#else
		#define	tex_sampler_t	sampler3D
		#define	tex_sample_px	texture3D
	#endif
	vec3 make_tex_coord( vec2 uv )
	{
		return	vec3( uv, v_vVertexAddColor.a ) ;
	}
	vec3 tex_coord_map( tex_coord_t uv, vec2 vScale, vec2 vOffset )
	{
		return uv * vec3( vScale, 1.0 ) + vec3( vOffset, 0.0 ) ;
	}
#else
	#define	tex_coord_t		mediump vec2
	#define	tex_sampler_t	sampler2D
	#define	tex_sample_px	texture2D
	vec2 make_tex_coord( vec2 uv )
	{
		return	uv ;
	}
	vec2 tex_coord_map( tex_coord_t uv, vec2 vScale, vec2 vOffset )
	{
		return uv * vScale + vOffset ;
	}
#endif



///////////////////////////////////////////////////////////////////////////////
// uniform 情報
///////////////////////////////////////////////////////////////////////////////

// 変換行列
uniform highp mat4		u_mat4PerspectiveView ;		// 透視変換行列
uniform highp mat4		u_mat4CameraView ;			// カメラ変換行列（回転と移動）
uniform highp mat3		u_mat3CameraViewForNormal ;	// カメラ変換行列（回転のみ）
uniform highp mat4		u_mat4ICameraView ;			// カメラ逆変換行列（回転と移動）
uniform highp mat3		u_mat3ICameraViewForNormal ;// カメラ変換逆行列（回転のみ）
uniform highp mat4		u_mat4ModelView ;			// 変換行列（回転と移動）（カメラを含む）
uniform highp mat3		u_mat3ModelViewForNormal ;	// 変換行列（回転のみ）（カメラを含む）
uniform mediump float	u_fpInverseNormal ;			// 法線反転用
uniform mediump float	u_fpBorderOffset ;			// 輪郭オフセット値

#ifndef	_ENABLE_VT_INSTANCING
#ifndef	_DISABLE_MORPHING
uniform highp float		u_fpMorphApplication ;		// モーフィング適用率
#endif
#endif

// ボーン行列
#ifndef	_ENABLE_VT_INSTANCING
#ifdef	_VERTEX_SHADER
uniform int				u_nBoneCount ;
uniform highp mat3		u_mat3BoneRotation[MAX_BONE_PALETTE] ;
uniform highp vec3		u_vBoneTranslate[MAX_BONE_PALETTE] ;
#endif
#endif

// 疑似フォッグ
uniform bool			u_bEnableFog ;
uniform lowp vec3		u_rgbFogColor ;
uniform mediump float	u_zFogNear ;
uniform mediump float	u_zFogDistance ;

// シャドウマッピング
#ifndef	_DISABLE_SHADOWMAPPING
uniform int				u_iEnableShadowmap[MAX_SHADOWMAP_COUNT] ;
#ifdef GL_ES
uniform mediump sampler2DArray	u_samplerShadowmap ;								// depth texture array
#else
uniform sampler2DArray	u_samplerShadowmap[MAX_SHADOWMAP_COUNT] ;			// depth texture array
#endif
uniform int				u_iShadowmapDepthLayer[MAX_SHADOWMAP_COUNT] ;		// 深度バッファのレイヤー番号
uniform highp mat4		u_mat4PerspectiveShadowmap[MAX_SHADOWMAP_COUNT] ;	// 光源からの透視変換
uniform highp mat4		u_mat4ModelViewShadowmap[MAX_SHADOWMAP_COUNT] ;		// 変換行列
uniform highp vec2		u_vShadowmapUnit[MAX_SHADOWMAP_COUNT] ;				// 1px 相当の UV 値
uniform highp float		u_fpShadowmapFixErrorGap[MAX_SHADOWMAP_COUNT] ;		// 1.0 - z 演算誤差固定比率（1/4096 等）
uniform highp float		u_fpShadowmapVarErrorGap[MAX_SHADOWMAP_COUNT] ;		// 角度に応じた z 演算誤差比率（1/512 等）
#endif

// 色効果
uniform lowp vec3		u_vEffectMulColor ;				// 色効果（積）
uniform lowp vec3		u_vEffectAddColor ;				// 色効果（加）
uniform lowp float		u_fpEffectAlpha ;				// 不透明度効果

// 表面属性
uniform bool			u_bMaterialShading ;
#ifdef	_PHONG_SHADER
uniform bool			u_bMaterialToon ;
#endif
uniform bool			u_bMaterialDoubleSide ;
uniform int				u_iMaterialTriming ;
uniform bool			u_bMaterialNoFogEffect ;
uniform bool			u_bMaterialVertexAlpha ;
uniform lowp vec3		u_vMaterialMulColor ;			// 基本色 - 影色
uniform lowp vec3		u_vMaterialAddColor ;
uniform lowp vec3		u_vMaterialMulShade ;			// 影色
uniform lowp vec3		u_vMaterialAddShade ;
uniform lowp vec3		u_vMaterialSpecularColor ;		// 鏡面反射色
uniform mediump float	u_fMaterialAmbient ;			// 環境光強度
uniform mediump float	u_fMaterialDiffusion ;			// 拡散反射光強度
uniform mediump float	u_fMaterialBackDiffusion ;		// 裏拡散反射（疑似AO）
uniform mediump float	u_fMaterialSpecular ;			// 鏡面反射光強度
uniform mediump float	u_fMaterialSpecularPow ;		// 鏡面反射光の鋭さ
uniform mediump float	u_fMaterialAlpha ;				// 不透明度
uniform mediump float	u_fMaterialDeepness ;			// 透明深度係数
uniform mediump float	u_fMaterialDeepnessPow ;		// 透明深度指数
#ifdef	_PHONG_SHADER
uniform mediump float	u_fMaterialReflection ;			// 反射率
#endif
uniform mediump float	u_fMaterialEmission ;			// 発光度
uniform lowp vec3		u_vMaterialBackLightMul ;		// バックライト
uniform lowp vec3		u_vMaterialBackLightAdd ;
uniform mediump float	u_cosShadeCoefficient[2] ;		// shade = a*cosθ + b : [0]=a, [1]=b
#ifdef	_PHONG_SHADER
uniform mediump float	u_fpToonShadeThreshold ;		// トゥーンシェーダー影閾値
uniform mediump float	u_fpToonShadeBrightness ;		// トゥーンシェーダー影輝度
uniform mediump float	u_fMaterialRimLight ;			// リムライト輝度
uniform mediump float	u_fMaterialRimDeepness ;		// リムライト厚み
uniform lowp vec3		u_vMaterialRimColor ;			// リムライト色
#endif

// 通常テクスチャ
uniform bool			u_bMaterialTexture ;			// テクスチャ有効
uniform tex_sampler_t	u_samplerMaterialTexture ;		// テクスチャ（拡散反射成分）
uniform highp vec2		u_vMaterialTextureScale ;		// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vMaterialTextureBase ;		// テクスチャUV変換行列（平行移動）

// 発光テクスチャ
#ifndef	_DISABLE_LUMINOUS_TEXTURE
uniform mediump float	u_fpLuminousTexture ;			// 発光テクスチャ適用度
uniform tex_sampler_t	u_samplerLuminousTexture ;		// 発光テクスチャ（拡散反射成分）
uniform highp vec2		u_vLuminousTextureScale ;		// 発光テクスチャUV変換行列（拡大）
uniform highp vec2		u_vLuminousTextureBase ;		// 発光テクスチャUV変換行列（平行移動）
#endif

// 環境マッピング
#ifndef	_DISABLE_ENVIRONMENT_MAPPING
#ifdef	_PHONG_SHADER
uniform int				u_typeEnvironmentMapping ;		// 環境マッピング
#ifndef	_DISABLE_REFRACTION_MAPPING
uniform int				u_typeEnvironmentRefraction ;	// 屈折反映用
uniform mediump float	u_fpRefractionRatio ;			// 屈折率
uniform mediump float	u_fpRefractionParam ;			// 屈折適用度
#endif
#ifndef	_DISABLE_ENVIRONMENT_CUBMAP
uniform samplerCube		u_samplerEnvironmentCube ;		// キューブマッピング
#endif
#ifndef	_DISABLE_ENVIRONMENT_SPHERE
uniform sampler2D		u_samplerEnvironmentMapping ;	// 環境マッピング（球投影）
uniform highp vec2		u_vEnvMapingTextureScale ;		// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vEnvMapingTextureBase ;		// テクスチャUV変換行列（平行移動）
#endif
#ifndef	_DISABLE_ENVIRONMENT_VIEWPORT
uniform sampler2D		u_samplerViewportMapping ;		// 屈折反映マッピング（前段レンダリング）
uniform sampler2D		u_samplerViewportDepth ;		// 前段レンダリング深度バッファ
uniform highp vec2		u_vViewportUnit ;				// 1px 相当の UV 値
#endif
uniform highp mat3		u_mat3EnvironmentMapping ;		// 変換行列（回転のみ）
#endif
#endif

// 法線テクスチャ
#ifndef	_DISABLE_NORMAL_TEXTURE
#ifdef	_PHONG_SHADER
uniform mediump float	u_fpNormalTexture ;				// 法線テクスチャ適用度
uniform tex_sampler_t	u_samplerNormalTexture ;		// 法線テクスチャ（r,g,b に x,y,z +0.5 を格納）
uniform highp vec2		u_vNormalTextureScale ;			// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vNormalTextureBase ;			// テクスチャUV変換行列（平行移動）
#endif
#endif

// 標高テクスチャ
#ifndef _DISABLE_HEIGHT_TEXTURE
#ifdef	_PHONG_SHADER
uniform mediump float	u_fpBumpHeight ;				// 標高テクスチャ高さ
uniform tex_sampler_t	u_samplerHeightTexture ;		// 標高テクスチャ
uniform highp vec2		u_vHeightTextureScale ;			// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vHeightTextureBase ;			// テクスチャUV変換行列（平行移動）
#endif
#endif

// αテクスチャマッピング
#ifndef	_DISABLE_ALPHA_MAPPING
uniform bool			u_bMaterialAlphaTexture ;		// αテクスチャ有効
uniform mediump float	u_fpAlphaCoefficient ;			// 係数 : x
uniform mediump float	u_fpAlphaBase ;					// α基準値 : y
														// a' = (a - y) * x
uniform tex_sampler_t	u_samplerAlphaMapping ;			// テクスチャ（αマッピング）
uniform highp vec2		u_vAlphaMapingTextureScale ;	// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vAlphaMapingTextureBase ;		// テクスチャUV変換行列（平行移動）
#endif

// スペキュラ・粗さ・反射率テクスチャ
#ifndef _DISABLE_SPECULAR_TEXTURE
#ifdef	_PHONG_SHADER
uniform bool			u_bMaterialSpecularTexture ;	// 反射率テクスチャ有効
uniform tex_sampler_t	u_samplerSpecularTexture ;		// 反射率テクスチャ
uniform highp vec2		u_vSpecularTextureScale ;		// テクスチャUV変換行列（拡大）
uniform highp vec2		u_vSpecularTextureBase ;		// テクスチャUV変換行列（平行移動）
#endif
#endif

// 大域ライトマップAOテクスチャ
#ifndef _DISABLE_LIGHTMAP_AO_TEXTURE
uniform bool			u_bMaterialGlobalAOTexture ;	// AOテクスチャ有効
uniform sampler2D		u_samplerGlobalAOTexture ;		// 反射率テクスチャ
#endif



///////////////////////////////////////////////////////////////////////////////
// 共通ユーティリティ関数
///////////////////////////////////////////////////////////////////////////////

// カメラビュー座標からの変換関数
vec4 utilCameraViewToPerspectiveView( in highp vec3 vCVPosition )
{
	return	u_mat4PerspectiveView * vec4( vCVPosition, 1.0 ) ;
}

vec3 utilCameraViewToGlobalPosition( in highp vec3 vCVPosition )
{
	return	(u_mat4ICameraView * vec4( vCVPosition, 1.0 )).xyz ;
}

vec3 utilCameraViewToGlobalDirection( in highp vec3 vCVDirection )
{
	return	u_mat3ICameraViewForNormal * vCVDirection ;
}

// 大域座標からカメラビュー座標へ変換
vec3 utilGlobalPositionToCameraView( in highp vec3 vGPosition )
{
	return	(u_mat4CameraView * vec4( vGPosition, 1.0 )).xyz ;
}

vec3 utilGlobalDirectionToCameraView( in highp vec3 vGDirection )
{
	return	u_mat3CameraViewForNormal * vGDirection ;
}



#ifdef	_FRAGMENT_SHADER

// 頂点法線（カメラビュー）
vec3 utilGetVertexNormalCV( void )
{
	return	v_vNormal ;
}

vec3 utilGetVertexNormal( void )
{
	return	u_mat3ICameraViewForNormal * v_vNormal ;
}


// パターン・ディザリング
// | 0 2 |
// | 3 1 |
// |  0  8  2 10 |
// | 12  4 14  6 |
// |  3 11  1  9 |
// | 15  7 13  5 |
float utilPatternDithering( void )
{
	float	x = floor(gl_FragCoord.x) ;
	float	y = floor(gl_FragCoord.y) ;

	float	dx2 = fract( x * 0.5 ) * 2.0 ;
	float	dy2 = fract( y * 0.5 ) * 2.0 ;
	float	dt2 = dx2 + (dy2 + fract((dx2 + dy2) * 0.5) - (dx2 * dy2)) * 2.0 ;

	float	dx4 = floor( fract( x * 0.25 ) * 2.0 ) ;
	float	dy4 = floor( fract( y * 0.25 ) * 2.0 ) ;
	float	dt4 = dx4 + (dy4 + fract((dx4 + dy4) * 0.5) - (dx4 * dy4)) * 2.0 ;

	return	(dt2 * 4.0 + dt4) * 0.0625 + 0.03125 ;
}



#endif


