void main( void )
{
mediump vec3 vVertexAddColor = v_vVertexAddColor.rgb ;
mediump vec3 vVertexMulColor = v_vVertexMulColor.rgb ;
mediump vec3 vEmissionColor ;
#ifndef _DISABLE_SHADING
#ifndef _DISABLE_SHADOWMAPPING
if ( u_bMaterialShading
&& (u_countLight >= 1)
&& (u_typeLighting[0] == LIGHT_VECTOR)
&& (u_iEnableShadowmap[0] == 0) )
{
float aShadow =
effect_light_shadow_mapping
( 0, v_vPosition.xyz, u_vLightDirection[0], v_vNormal ) ;
mediump vec3 vAmbient =
u_vLightAmbientColor
+ vec3( u_fMaterialAmbient,
u_fMaterialAmbient, u_fMaterialAmbient ) ;
if ( u_bMaterialTexture )
{
vVertexAddColor =
(vVertexAddColor - u_vMaterialAddShade)
* aShadow + u_vMaterialAddShade ;
vVertexMulColor =
(vVertexMulColor - (u_vMaterialMulShade + vAmbient))
* aShadow + (u_vMaterialMulShade + vAmbient) ;
}
else
{
vVertexAddColor =
(vVertexAddColor - (u_vMaterialAddShade + vAmbient))
* aShadow + (u_vMaterialAddShade + vAmbient) ;
}
}
#endif
#endif
if ( !u_bMaterialShading )
{
vVertexAddColor += u_vMaterialAddColor * vVertexMulColor ;
vVertexMulColor = u_vMaterialMulColor * vVertexMulColor ;
}
float fpVertexAlpha =
texture_mapping_shader
( vVertexAddColor, vEmissionColor,
vVertexAddColor, vVertexMulColor,
make_tex_coord( v_vTextureCoord ), v_vVertexMulColor.a ) ;
if ( vVertexAddColor.r + vVertexAddColor.g
+ vVertexAddColor.b + fpVertexAlpha < 0.0078125 )
{
discard ;
}
else
{
write_frag6( vec4( vVertexAddColor, fpVertexAlpha ),
vEmissionColor, v_vNormal, vVertexAddColor,
vec3( 0.0, 0.0, 0.0 ), vec3( 0.0, 0.0, 0.0 ) ) ;
}
}
