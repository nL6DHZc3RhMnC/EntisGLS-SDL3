void main( void )
{
float fpVertexAlpha = v_vVertexMulColor.a ;
mediump vec3 vVertexAddColor = v_vVertexAddColor.rgb ;
mediump vec3 vVertexMulColor = v_vVertexMulColor.rgb ;
mediump vec3 vEmissionColor = vec3( 0.0, 0.0, 0.0 ) ;
mediump vec3 vAmbientColor = vec3( 0.0, 0.0, 0.0 ) ;
mediump vec3 vDiffusionColor = vec3( 0.0, 0.0, 0.0 ) ;
mediump vec3 vDstSpecular = vec3( 0.0, 0.0, 0.0 ) ;
mediump vec3 vDstNormal = v_vNormal ;
highp vec3 vRenderPosition = v_vPosition.xyz ;
if ( u_bMaterialShading )
{
float fpShadeAlpha = effect_lighting
( vVertexMulColor,
vVertexAddColor, vEmissionColor,
vAmbientColor, vDiffusionColor,
vDstSpecular, vDstNormal,
vRenderPosition,
v_vPosition.xyz, v_vNormal,
make_tex_coord( v_vTextureCoord ),
u_vMaterialMulColor, u_vMaterialAddColor,
u_vMaterialMulShade, u_vMaterialAddShade,
vVertexMulColor, vVertexAddColor ) ;
vVertexAddColor *= v_vVertexMulColor.a ;
vEmissionColor *= v_vVertexMulColor.a ;
fpVertexAlpha *= fpShadeAlpha ;
}
else
{
vVertexAddColor += u_vMaterialAddColor * vVertexMulColor ;
vVertexMulColor = u_vMaterialMulColor * vVertexMulColor ;
fpVertexAlpha =
texture_mapping_shader
( vVertexAddColor, vEmissionColor,
vVertexAddColor, vVertexMulColor,
make_tex_coord( v_vTextureCoord ),
u_fpEffectAlpha * fpVertexAlpha ) ;
}
#ifdef _ENABLE_COMPUTE_DEPTH_
vec4 vPers = u_mat4PerspectiveView * vec4( vRenderPosition, 1.0 ) ;
write_depth( vPers.z * 0.5 / vPers.w + 0.5 ) ;
#endif
if ( vVertexAddColor.r + vVertexAddColor.g
+ vVertexAddColor.b + fpVertexAlpha < 0.0078125 )
{
discard ;
}
else
{
write_frag6( vec4( vVertexAddColor, fpVertexAlpha ), vEmissionColor,
vDstNormal, vDiffusionColor, vAmbientColor, vDstSpecular ) ;
}
}
