#ifdef GL_ES
#define MAX_LIGHT_COUNT 5
#define MAX_SHADOWMAP_COUNT 6
#else
#define MAX_LIGHT_COUNT 8
#define MAX_SHADOWMAP_COUNT 12
#endif
#ifndef _LIMIT_SHADOWMAP_COUNT
#define _LIMIT_SHADOWMAP_COUNT MAX_SHADOWMAP_COUNT
#endif
#define LIGHT_NULL 0
#define LIGHT_AMBIENT 1
#define LIGHT_VECTOR 2
#define LIGHT_POINT 4
#define LIGHT_SPOT 6
#define LIGHT_FOG 8
#define ENV_MAPPING_NOTHING 0
#define ENV_MAPPING_HEMISPHERE 1
#define ENV_MAPPING_SPHERE 2
#define ENV_MAPPING_CUBE 3
#define ENV_MAPPING_VIEWPORT 4
#define ALPHA_NO_TRIM 0
#define ALPHA_TRIMING 1
#define ALPHA_DITHERING 2
#ifndef MAX_BONE_PALETTE
#ifdef _DISABLE_BONE_OVER_2
#define MAX_BONE_PALETTE 2
#else
#ifdef _DISABLE_BONE_OVER_4
#define MAX_BONE_PALETTE 4
#else
#ifdef _DISABLE_BONE_OVER_6
#define MAX_BONE_PALETTE 6
#else
#ifdef _DISABLE_BONE_OVER_8
#define MAX_BONE_PALETTE 8
#else
#ifdef _DISABLE_BONE_OVER_12
#define MAX_BONE_PALETTE 12
#else
#define MAX_BONE_PALETTE 16
#endif
#endif
#endif
#endif
#endif
#endif
#ifdef _VERTEX_SHADER
#ifdef _LINK_GEOMETRY_SHADER
#define v_vPosition g_vPosition
#define v_vNormal g_vNormal
#define v_vTextureAxisX g_vTextureAxisX
#define v_vTextureAxisX g_vTextureAxisX
#define v_vTextureCoord g_vTextureCoord
#define v_vVertexMulColor g_vVertexMulColor
#define v_vVertexAddColor g_vVertexAddColor
#ifdef _ENABLE_EXTEND_ATTR_
#define v_vExAttrElement0 g_vExAttrElement0
#endif
#endif
#endif
varying highp vec4 v_vPosition ;
varying mediump vec3 v_vNormal ;
#ifdef _PHONG_SHADER
varying mediump vec3 v_vTextureAxisX ;
varying mediump vec3 v_vTextureAxisY ;
#endif
varying mediump vec2 v_vTextureCoord ;
varying mediump vec4 v_vVertexMulColor ;
varying mediump vec4 v_vVertexAddColor ;
#ifdef _ENABLE_EXTEND_ATTR_
varying highp vec4 v_vExAttrElement0 ;
#endif
#ifdef _ENABLE_MATERIAL_TEXTURE_3D
#define tex_coord_t mediump vec3
#ifdef _ENABLE_MATERIAL_TEXTURE_ARRAY
#define tex_sampler_t sampler2DArray
#define tex_sample_px texture2DArray
#else
#define tex_sampler_t sampler3D
#define tex_sample_px texture3D
#endif
vec3 make_tex_coord( vec2 uv )
{
return vec3( uv, v_vVertexAddColor.a ) ;
}
vec3 tex_coord_map( tex_coord_t uv, vec2 vScale, vec2 vOffset )
{
return uv * vec3( vScale, 1.0 ) + vec3( vOffset, 0.0 ) ;
}
#else
#define tex_coord_t mediump vec2
#define tex_sampler_t sampler2D
#define tex_sample_px texture2D
vec2 make_tex_coord( vec2 uv )
{
return uv ;
}
vec2 tex_coord_map( tex_coord_t uv, vec2 vScale, vec2 vOffset )
{
return uv * vScale + vOffset ;
}
#endif
uniform highp mat4 u_mat4PerspectiveView ;
uniform highp mat4 u_mat4CameraView ;
uniform highp mat3 u_mat3CameraViewForNormal ;
uniform highp mat4 u_mat4ICameraView ;
uniform highp mat3 u_mat3ICameraViewForNormal ;
uniform highp mat4 u_mat4ModelView ;
uniform highp mat3 u_mat3ModelViewForNormal ;
uniform mediump float u_fpInverseNormal ;
uniform mediump float u_fpBorderOffset ;
#ifndef _ENABLE_VT_INSTANCING
#ifndef _DISABLE_MORPHING
uniform highp float u_fpMorphApplication ;
#endif
#endif
#ifndef _ENABLE_VT_INSTANCING
#ifdef _VERTEX_SHADER
uniform int u_nBoneCount ;
uniform highp mat3 u_mat3BoneRotation[MAX_BONE_PALETTE] ;
uniform highp vec3 u_vBoneTranslate[MAX_BONE_PALETTE] ;
#endif
#endif
uniform bool u_bEnableFog ;
uniform lowp vec3 u_rgbFogColor ;
uniform mediump float u_zFogNear ;
uniform mediump float u_zFogDistance ;
#ifndef _DISABLE_SHADOWMAPPING
uniform int u_iEnableShadowmap[MAX_SHADOWMAP_COUNT] ;
#ifdef GL_ES
uniform mediump sampler2DArray u_samplerShadowmap ;
#else
uniform sampler2DArray u_samplerShadowmap[MAX_SHADOWMAP_COUNT] ;
#endif
uniform int u_iShadowmapDepthLayer[MAX_SHADOWMAP_COUNT] ;
uniform highp mat4 u_mat4PerspectiveShadowmap[MAX_SHADOWMAP_COUNT] ;
uniform highp mat4 u_mat4ModelViewShadowmap[MAX_SHADOWMAP_COUNT] ;
uniform highp vec2 u_vShadowmapUnit[MAX_SHADOWMAP_COUNT] ;
uniform highp float u_fpShadowmapFixErrorGap[MAX_SHADOWMAP_COUNT] ;
uniform highp float u_fpShadowmapVarErrorGap[MAX_SHADOWMAP_COUNT] ;
#endif
uniform lowp vec3 u_vEffectMulColor ;
uniform lowp vec3 u_vEffectAddColor ;
uniform lowp float u_fpEffectAlpha ;
uniform bool u_bMaterialShading ;
#ifdef _PHONG_SHADER
uniform bool u_bMaterialToon ;
#endif
uniform bool u_bMaterialDoubleSide ;
uniform int u_iMaterialTriming ;
uniform bool u_bMaterialNoFogEffect ;
uniform bool u_bMaterialVertexAlpha ;
uniform lowp vec3 u_vMaterialMulColor ;
uniform lowp vec3 u_vMaterialAddColor ;
uniform lowp vec3 u_vMaterialMulShade ;
uniform lowp vec3 u_vMaterialAddShade ;
uniform lowp vec3 u_vMaterialSpecularColor ;
uniform mediump float u_fMaterialAmbient ;
uniform mediump float u_fMaterialDiffusion ;
uniform mediump float u_fMaterialBackDiffusion ;
uniform mediump float u_fMaterialSpecular ;
uniform mediump float u_fMaterialSpecularPow ;
uniform mediump float u_fMaterialAlpha ;
uniform mediump float u_fMaterialDeepness ;
uniform mediump float u_fMaterialDeepnessPow ;
#ifdef _PHONG_SHADER
uniform mediump float u_fMaterialReflection ;
#endif
uniform mediump float u_fMaterialEmission ;
uniform lowp vec3 u_vMaterialBackLightMul ;
uniform lowp vec3 u_vMaterialBackLightAdd ;
uniform mediump float u_cosShadeCoefficient[2] ;
#ifdef _PHONG_SHADER
uniform mediump float u_fpToonShadeThreshold ;
uniform mediump float u_fpToonShadeBrightness ;
uniform mediump float u_fMaterialRimLight ;
uniform mediump float u_fMaterialRimDeepness ;
uniform lowp vec3 u_vMaterialRimColor ;
#endif
uniform bool u_bMaterialTexture ;
uniform tex_sampler_t u_samplerMaterialTexture ;
uniform highp vec2 u_vMaterialTextureScale ;
uniform highp vec2 u_vMaterialTextureBase ;
#ifndef _DISABLE_LUMINOUS_TEXTURE
uniform mediump float u_fpLuminousTexture ;
uniform tex_sampler_t u_samplerLuminousTexture ;
uniform highp vec2 u_vLuminousTextureScale ;
uniform highp vec2 u_vLuminousTextureBase ;
#endif
#ifndef _DISABLE_ENVIRONMENT_MAPPING
#ifdef _PHONG_SHADER
uniform int u_typeEnvironmentMapping ;
#ifndef _DISABLE_REFRACTION_MAPPING
uniform int u_typeEnvironmentRefraction ;
uniform mediump float u_fpRefractionRatio ;
uniform mediump float u_fpRefractionParam ;
#endif
#ifndef _DISABLE_ENVIRONMENT_CUBMAP
uniform samplerCube u_samplerEnvironmentCube ;
#endif
#ifndef _DISABLE_ENVIRONMENT_SPHERE
uniform sampler2D u_samplerEnvironmentMapping ;
uniform highp vec2 u_vEnvMapingTextureScale ;
uniform highp vec2 u_vEnvMapingTextureBase ;
#endif
#ifndef _DISABLE_ENVIRONMENT_VIEWPORT
uniform sampler2D u_samplerViewportMapping ;
uniform sampler2D u_samplerViewportDepth ;
uniform highp vec2 u_vViewportUnit ;
#endif
uniform highp mat3 u_mat3EnvironmentMapping ;
#endif
#endif
#ifndef _DISABLE_NORMAL_TEXTURE
#ifdef _PHONG_SHADER
uniform mediump float u_fpNormalTexture ;
uniform tex_sampler_t u_samplerNormalTexture ;
uniform highp vec2 u_vNormalTextureScale ;
uniform highp vec2 u_vNormalTextureBase ;
#endif
#endif
#ifndef _DISABLE_HEIGHT_TEXTURE
#ifdef _PHONG_SHADER
uniform mediump float u_fpBumpHeight ;
uniform tex_sampler_t u_samplerHeightTexture ;
uniform highp vec2 u_vHeightTextureScale ;
uniform highp vec2 u_vHeightTextureBase ;
#endif
#endif
#ifndef _DISABLE_ALPHA_MAPPING
uniform bool u_bMaterialAlphaTexture ;
uniform mediump float u_fpAlphaCoefficient ;
uniform mediump float u_fpAlphaBase ;
uniform tex_sampler_t u_samplerAlphaMapping ;
uniform highp vec2 u_vAlphaMapingTextureScale ;
uniform highp vec2 u_vAlphaMapingTextureBase ;
#endif
#ifndef _DISABLE_SPECULAR_TEXTURE
#ifdef _PHONG_SHADER
uniform bool u_bMaterialSpecularTexture ;
uniform tex_sampler_t u_samplerSpecularTexture ;
uniform highp vec2 u_vSpecularTextureScale ;
uniform highp vec2 u_vSpecularTextureBase ;
#endif
#endif
#ifndef _DISABLE_LIGHTMAP_AO_TEXTURE
uniform bool u_bMaterialGlobalAOTexture ;
uniform sampler2D u_samplerGlobalAOTexture ;
#endif
vec4 utilCameraViewToPerspectiveView( in highp vec3 vCVPosition )
{
return u_mat4PerspectiveView * vec4( vCVPosition, 1.0 ) ;
}
vec3 utilCameraViewToGlobalPosition( in highp vec3 vCVPosition )
{
return (u_mat4ICameraView * vec4( vCVPosition, 1.0 )).xyz ;
}
vec3 utilCameraViewToGlobalDirection( in highp vec3 vCVDirection )
{
return u_mat3ICameraViewForNormal * vCVDirection ;
}
vec3 utilGlobalPositionToCameraView( in highp vec3 vGPosition )
{
return (u_mat4CameraView * vec4( vGPosition, 1.0 )).xyz ;
}
vec3 utilGlobalDirectionToCameraView( in highp vec3 vGDirection )
{
return u_mat3CameraViewForNormal * vGDirection ;
}
#ifdef _FRAGMENT_SHADER
vec3 utilGetVertexNormalCV( void )
{
return v_vNormal ;
}
vec3 utilGetVertexNormal( void )
{
return u_mat3ICameraViewForNormal * v_vNormal ;
}
float utilPatternDithering( void )
{
float x = floor(gl_FragCoord.x) ;
float y = floor(gl_FragCoord.y) ;
float dx2 = fract( x * 0.5 ) * 2.0 ;
float dy2 = fract( y * 0.5 ) * 2.0 ;
float dt2 = dx2 + (dy2 + fract((dx2 + dy2) * 0.5) - (dx2 * dy2)) * 2.0 ;
float dx4 = floor( fract( x * 0.25 ) * 2.0 ) ;
float dy4 = floor( fract( y * 0.25 ) * 2.0 ) ;
float dt4 = dx4 + (dy4 + fract((dx4 + dy4) * 0.5) - (dx4 * dy4)) * 2.0 ;
return (dt2 * 4.0 + dt4) * 0.0625 + 0.03125 ;
}
#endif
