void main( void )
{
mediump vec3 vVertexAddColor = v_vVertexAddColor.rgb ;
mediump vec3 vVertexMulColor = v_vVertexMulColor.rgb ;
mediump vec3 vEmissionColor ;
mediump float fpVertexAlpha = 1.0 ;
if ( u_bMaterialTexture )
{
vVertexAddColor += vVertexMulColor ;
}
else
{
vVertexAddColor += u_vMaterialAddColor * vVertexMulColor ;
}
vVertexAddColor = vVertexAddColor * u_vEffectMulColor + u_vEffectAddColor ;
vEmissionColor = vVertexAddColor * u_fMaterialEmission ;
write_frag6( vec4( vVertexAddColor, fpVertexAlpha ),
vEmissionColor, v_vNormal, vVertexAddColor,
vec3( 0.0, 0.0, 0.0 ), vec3( 0.0, 0.0, 0.0 ) ) ;
}
