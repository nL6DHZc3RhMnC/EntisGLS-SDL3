#ifdef _USER_SUB_DIFFUSION
vec4 utilSampleDiffusion( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
vec4 subSampleDiffusion( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
#endif
#ifdef _USER_SUB_EMISSION
vec4 utilSampleEmission( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
vec4 subSampleEmission( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
#endif
#ifdef _USER_SUB_NORMAL
vec4 utilSampleNormal( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
vec4 subSampleNormal( in tex_coord_t uv, in highp vec3 vCVPosition ) ;
#endif
float texture_mapping
( out mediump vec3 vDstVertexAddColor,
in mediump vec3 vVertexAddColor,
in mediump vec3 vVertexMulColor,
in tex_coord_t vTextureCoord, in float fpVertexAlpha )
{
#ifdef _USER_SUB_DIFFUSION
vec4 vTexture =
subSampleDiffusion
( tex_coord_map
( vTextureCoord,
u_vMaterialTextureScale,
u_vMaterialTextureBase ), v_vPosition.xyz ) ;
#else
vec4 vTexture =
tex_sample_px
( u_samplerMaterialTexture,
tex_coord_map
( vTextureCoord,
u_vMaterialTextureScale,
u_vMaterialTextureBase ) ) ;
#endif
if ( u_iMaterialTriming != ALPHA_NO_TRIM )
{
float alphaThreshold = 0.5 ;
#ifdef _FRAGMENT_SHADER
if ( u_iMaterialTriming == ALPHA_DITHERING )
{
alphaThreshold = utilPatternDithering() ;
}
#endif
if ( vTexture.a < alphaThreshold )
{
vTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
}
else
{
vTexture *= 1.0 / (vTexture.a + 0.05) ;
vTexture.a = 1.0 ;
}
}
vTexture *= fpVertexAlpha ;
#ifndef _DISABLE_ALPHA_MAPPING
if ( u_bMaterialAlphaTexture )
{
float alpha =
tex_sample_px
( u_samplerAlphaMapping,
tex_coord_map
( vTextureCoord,
u_vAlphaMapingTextureScale,
u_vAlphaMapingTextureBase ) ).r ;
alpha = (alpha - u_fpAlphaBase) * u_fpAlphaCoefficient ;
vTexture *= max( min( alpha, 1.0 ), 0.0 ) ;
}
#endif
vDstVertexAddColor = vVertexAddColor * vTexture.a
+ vTexture.rgb * vVertexMulColor ;
return vTexture.a ;
}
float texture_mapping_shader
( out mediump vec3 vDstVertexAddColor,
out mediump vec3 vDstEmissionColor,
in mediump vec3 vVertexAddColor,
in mediump vec3 vVertexMulColor,
in tex_coord_t vTextureCoord, in float fpVertexAlpha )
{
float fpAlpha = fpVertexAlpha ;
#ifndef _USER_SUB_DIFFUSION
if ( u_bMaterialTexture )
{
#endif
fpAlpha =
texture_mapping
( vVertexAddColor,
vVertexAddColor, vVertexMulColor,
vTextureCoord, fpVertexAlpha ) ;
#ifndef _USER_SUB_DIFFUSION
}
else
{
vVertexAddColor *= fpVertexAlpha ;
fpAlpha *=
max( 1.0 - (vVertexMulColor.r
+ vVertexMulColor.g
+ vVertexMulColor.b) * 0.33334, 0.0 ) ;
}
#endif
#ifndef _DISABLE_LIGHTMAP_AO_TEXTURE
#ifdef _FRAGMENT_SHADER
#ifdef _ENABLE_EXTEND_ATTR_
if ( u_bMaterialGlobalAOTexture )
{
vec4 vAO = texture2D( u_samplerGlobalAOTexture, v_vExAttrElement0.xy ) ;
vVertexAddColor.rgb *= vAO.rgb ;
}
#endif
#else
if ( u_bMaterialGlobalAOTexture )
{
vec4 vExAttr = getExAttrElement( 0 ) ;
vec4 vAO = texture2D( u_samplerGlobalAOTexture, vExAttr.xy ) ;
vVertexAddColor *= vAO.rgb ;
}
#endif
#endif
vDstEmissionColor = vec3( 0.0, 0.0, 0.0 ) ;
#ifndef _DISABLE_LUMINOUS_TEXTURE
#ifdef _USER_SUB_EMISSION
if ( (u_iMaterialTriming == ALPHA_NO_TRIM) || (fpAlpha <= 0.5) )
{
vec4 vTexture =
subSampleEmission
( tex_coord_map
( vTextureCoord,
u_vLuminousTextureScale,
u_vLuminousTextureBase ), v_vPosition.xyz ) ;
vDstEmissionColor = vTexture.rgb * fpVertexAlpha ;
vVertexAddColor += vDstEmissionColor ;
}
#else
#ifndef _DISABLE_SHADING
if ( (u_fpLuminousTexture > 0.0)
&& ((u_iMaterialTriming == ALPHA_NO_TRIM) || (fpAlpha >= 0.5)) )
{
vec4 vTexture =
tex_sample_px
( u_samplerLuminousTexture,
tex_coord_map
( vTextureCoord,
u_vLuminousTextureScale,
u_vLuminousTextureBase ) ) ;
vDstEmissionColor = vTexture.rgb * (u_fpLuminousTexture * fpVertexAlpha) ;
vVertexAddColor += vDstEmissionColor ;
if ( !u_bMaterialTexture )
{
fpAlpha = 0.0 ;
}
}
#endif
#endif
#endif
vDstVertexAddColor = vVertexAddColor * u_vEffectMulColor
+ u_vEffectAddColor * fpAlpha ;
vDstEmissionColor += vVertexAddColor * u_fMaterialEmission ;
return fpAlpha ;
}
