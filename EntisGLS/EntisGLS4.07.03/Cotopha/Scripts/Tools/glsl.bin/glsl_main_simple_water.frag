uniform mediump float u_fpAmplitude ;
uniform mediump float u_fpNormalAmp ;
uniform mediump vec4 u_vAmplitude ;
uniform mediump vec4 u_vFrequency ;
uniform mediump vec4 u_vTime ;
uniform mediump vec2 u_vDirection[4] ;
uniform mediump vec4 u_vBumpAmplitude ;
uniform mediump vec4 u_vBumpFrequency ;
uniform mediump vec4 u_vBumpTime ;
uniform mediump vec2 u_vBumpDirection[4] ;
uniform mediump vec3 u_vWaterAxisX ;
uniform mediump vec3 u_vWaterAxisY ;
uniform mediump float u_fpCascadeFarZ ;
uniform mediump float u_fpCascadePhase1 ;
uniform mediump float u_fpCascadeAmplitude1 ;
varying mediump vec2 v_vWaveCoord ;
void main( void )
{
vec2 a1 = u_vBumpDirection[0] ;
vec2 b1 = u_vBumpDirection[1] ;
vec2 c1 = u_vBumpDirection[2] ;
vec2 d1 = u_vBumpDirection[3] ;
vec4 phase1 = u_vBumpFrequency
* vec4( dot( a1, v_vWaveCoord ),
dot( b1, v_vWaveCoord ),
dot( c1, v_vWaveCoord ),
dot( d1, v_vWaveCoord ) ) + u_vBumpTime ;
vec4 vCos1 = cos( phase1 ) * u_vBumpAmplitude ;
vec4 xABCD1 = vec4( a1.x, b1.x, c1.x, d1.x ) ;
vec4 yABCD1 = vec4( a1.y, b1.y, c1.y, d1.y ) ;
vec3 vOffset1 = vec3( dot( vCos1, xABCD1 ) * 0.25,
1.0, dot( vCos1, yABCD1 ) * 0.25 ) ;
vec2 a2 = u_vDirection[0] ;
vec2 b2 = u_vDirection[1] ;
vec2 c2 = u_vDirection[2] ;
vec2 d2 = u_vDirection[3] ;
vec4 phase2 = u_vFrequency
* vec4( dot( a2, v_vWaveCoord ),
dot( b2, v_vWaveCoord ),
dot( c2, v_vWaveCoord ),
dot( d2, v_vWaveCoord ) ) + u_vTime ;
vec4 vCos2 = cos( phase2 ) * u_vAmplitude ;
vec4 vSin2 = sin( phase2 ) ;
vec4 xABCD2 = vec4( a2.x, b2.x, c2.x, d2.x ) ;
vec4 yABCD2 = vec4( a2.y, b2.y, c2.y, d2.y ) ;
vec3 vOffset2 =
vec3( dot( vCos2, xABCD2 ),
dot( vSin2, u_vAmplitude ),
dot( vCos2, yABCD2 ) ) ;
vec4 vCos3 = cos( phase2 * u_fpCascadePhase1 ) ;
vec3 vOffset3 =
vec3( dot( vCos3, xABCD2 ) * 0.25,
1.0, dot( vCos3, yABCD2 ) * 0.25 ) ;
vec3 vWaveAxisX = u_mat3ModelViewForNormal * u_vWaterAxisX ;
vec3 vWaveAxisY = u_mat3ModelViewForNormal * u_vWaterAxisY ;
vec3 vNormal = normalize( v_vNormal ) ;
float cosNormal = abs(dot( normalize(v_vPosition.xyz), vNormal )) ;
float d = length(v_vPosition.xyz) / u_fpCascadeFarZ ;
float denv1 = 1.0 - min( d, 0.99 ) ;
float denv0 = 1.0 - min( d, 1.0 ) ;
vOffset1 = normalize( vOffset1 ) ;
vOffset2 = normalize( vOffset2 ) ;
vOffset3 = normalize( vOffset3 ) ;
float fpNormalAmp2 = 0.5 * u_fpNormalAmp * denv1 ;
float fpCascadeAmp3 = 0.5 * u_fpCascadeAmplitude1 * denv1 ;
vNormal += normalize( vWaveAxisX )
* (vOffset1.x * denv0
+ vOffset2.x * fpNormalAmp2
+ vOffset3.x * fpCascadeAmp3)
+ normalize( vWaveAxisY )
* (vOffset1.z * denv0
+ vOffset2.z * fpNormalAmp2
+ vOffset3.z * fpCascadeAmp3) ;
vNormal = normalize( vNormal ) ;
mediump vec4 vDstColor ;
mediump vec4 vEmissionColor ;
mediump vec3 vAmbientColor ;
mediump vec3 vDiffusionColor ;
mediump vec3 vDstSpecular ;
mediump vec3 vDstNormal ;
utilCustomShader
( vDstColor, vEmissionColor,
vAmbientColor, vDiffusionColor, vDstSpecular, vDstNormal,
v_vPosition.xyz, vNormal, vec2( 0.0, 0.0 ),
u_vMaterialMulColor, u_vMaterialAddColor,
u_vMaterialMulShade, u_vMaterialAddShade ) ;
if ( vDstColor.r + vDstColor.g
+ vDstColor.b + vDstColor.a < 0.0078125 )
{
discard ;
}
else
{
write_frag6( vDstColor, vEmissionColor.rgb,
vDstNormal, vDiffusionColor, vAmbientColor, vDstSpecular ) ;
}
}
