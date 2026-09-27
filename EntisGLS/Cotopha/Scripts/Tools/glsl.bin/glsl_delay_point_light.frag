uniform sampler2D u_samplerDepth ;
uniform sampler2D u_samplerNormal ;
uniform sampler2D u_samplerSpecular ;
uniform highp mat4 u_mat4SamplePers ;
uniform float u_fpDiffusion ;
uniform float u_fpSpecular ;
uniform float u_fpAirScattering ;
uniform float u_fpAirRcpUnit ;
uniform int u_typeDLighting ;
uniform highp vec3 u_vDLPosition ;
uniform mediump vec3 u_vDLDirection ;
uniform lowp vec3 u_vDLColor ;
uniform float u_fpDLBrightness ;
uniform float u_fpDLAttenuationPower ;
uniform mediump float u_fpDLAngle ;
uniform mediump float u_fpDLGradation ;
float CalcSpotBrightness( in vec3 vTestPos )
{
vec3 vDirection = vTestPos - u_vDLPosition ;
float fpDistance = length( vDirection ) ;
float fpBrightness = u_fpDLBrightness
* pow( 1.0 / fpDistance, u_fpDLAttenuationPower ) ;
float cosDir = dot( vDirection / fpDistance, u_vDLDirection ) ;
if ( cosDir < u_fpDLAngle )
{
fpBrightness = 0.0 ;
}
if ( cosDir - u_fpDLAngle < u_fpDLGradation )
{
fpBrightness *= (cosDir - u_fpDLAngle) / u_fpDLGradation ;
}
return fpBrightness ;
}
float CalcPointLightGlow( in vec3 vSpace, in float zValue )
{
vec3 vViewDir = normalize( vSpace ) ;
float fpDLDistance = length( u_vDLPosition ) ;
float cosDLDiff = dot( normalize( u_vDLPosition ), vViewDir ) ;
vec3 vNearestPos = vViewDir * (fpDLDistance * cosDLDiff) ;
float fpDiffDistance =
length( vNearestPos - u_vDLPosition ) * u_fpAirRcpUnit ;
float fpGlow = 0.0 ;
if ( vNearestPos.z < zValue )
{
if ( vNearestPos.z >= 0.0 )
{
fpGlow += 1.0 / fpDiffDistance
- 1.0 / (fpDiffDistance + vNearestPos.z * u_fpAirRcpUnit) ;
fpGlow += 1.0 / fpDiffDistance
- 1.0 / (fpDiffDistance
+ (zValue - vNearestPos.z) * u_fpAirRcpUnit) ;
}
else
{
fpGlow += 1.0 / (fpDiffDistance - vNearestPos.z * u_fpAirRcpUnit)
- 1.0 / (fpDiffDistance
+ (zValue - vNearestPos.z) * u_fpAirRcpUnit) ;
}
}
else
{
fpGlow += 1.0 / (fpDiffDistance
+ (vNearestPos.z - zValue) * u_fpAirRcpUnit)
- 1.0 / (fpDiffDistance + vNearestPos.z * u_fpAirRcpUnit) ;
}
return fpGlow ;
}
void main( void )
{
vec2 uvSample =
v_vTextureCoord * u_vMaterialTextureScale + u_vMaterialTextureBase ;
vec3 vDiffusion = texture2D( u_samplerMaterialTexture, uvSample ).rgb ;
vec3 vSpecular = texture2D( u_samplerSpecular, uvSample ).rgb ;
float m00 = u_mat4SamplePers[0].x ;
float m03 = u_mat4SamplePers[3].x ;
float m11 = u_mat4SamplePers[1].y ;
float m13 = u_mat4SamplePers[3].y ;
float m22 = u_mat4SamplePers[2].z ;
float m32 = u_mat4SamplePers[2].w ;
float m23 = u_mat4SamplePers[3].z ;
float m33 = u_mat4SamplePers[3].w ;
float zDepth = texture2D( u_samplerDepth, uvSample ).x * 2.0 - 1.0 ;
float zValue = (m23 - m33 * zDepth) / (m32 * zDepth - m22) ;
float wPers = m32 * zValue + m33 ;
float xSpace = ((uvSample.x * 2.0 - 1.0) * wPers - m03) / m00 ;
float ySpace = ((uvSample.y * 2.0 - 1.0) * wPers - m13) / m11 ;
vec3 vNormal = normalize
( texture2D( u_samplerNormal, uvSample ).xyz
* 2.0 - vec3( 1.0, 1.0, 1.0 ) ) ;
vec3 vSpace = vec3( xSpace, ySpace, zValue ) ;
vec3 vPosDir = normalize( vSpace ) ;
float fpFocusParam = dot( vPosDir, vNormal ) ;
vec3 vDirection = vSpace - u_vDLPosition ;
float fpDistance = max( length( vDirection ), 1.0e-7 ) ;
vDirection /= fpDistance ;
vec3 vShading = vec3( 0.0, 0.0, 0.0 ) ;
float fpBrightness = u_fpDLBrightness
* pow( 1.0 / fpDistance, u_fpDLAttenuationPower ) ;
if ( u_typeDLighting == LIGHT_SPOT )
{
float cosDir = dot( vDirection, u_vDLDirection ) ;
if ( cosDir < u_fpDLAngle )
{
fpBrightness = 0.0 ;
}
fpBrightness = min( fpBrightness, 1.0 ) ;
if ( cosDir - u_fpDLAngle < u_fpDLGradation )
{
fpBrightness *= (cosDir - u_fpDLAngle) / u_fpDLGradation ;
}
if ( u_fpAirScattering > 0.0 )
{
float fpViewLight = CalcSpotBrightness( vec3( 0.0, 0.0, 0.0 ) ) ;
float fpGlow ;
if ( fpViewLight > 0.0 )
{
fpGlow = CalcPointLightGlow( vSpace, zValue ) ;
vShading += u_vDLColor * (u_fpAirScattering * fpGlow * fpViewLight) ;
}
fpGlow = 0.0 ;
vec3 vRayMarching = vec3( 0.0, 0.0, 0.0 ) ;
float cosConeCore = u_fpDLAngle + u_fpDLGradation ;
float sinConeCore = sqrt( 1.0 - min( 1.0, cosConeCore * cosConeCore ) ) ;
for ( int i = 0; i < 8; i ++ )
{
vec3 vDelta = vRayMarching - u_vDLPosition ;
float fpLength = length( vDelta ) ;
float cosDelta = dot( vDelta / fpLength, u_vDLDirection ) ;
if ( cosDelta > 0.0 )
{
float sinDelta = sqrt( 1.0 - min( 1.0, cosDelta * cosDelta ) ) ;
fpLength *= max( 0.0, sinDelta - sinConeCore ) ;
}
vRayMarching += vPosDir * fpLength ;
if ( vRayMarching.z < zValue )
{
fpGlow = max( fpGlow, CalcSpotBrightness( vRayMarching ) ) ;
}
if ( fpLength == 0.0 )
{
break ;
}
}
vShading += u_vDLColor * (u_fpAirScattering * fpGlow) ;
}
else if ( fpBrightness <= 0.0 )
{
discard ;
}
}
else
{
if ( fpBrightness > 1.0 )
{
fpBrightness = min( sqrt( fpBrightness ), 2.0 ) ;
}
if ( u_fpAirScattering > 0.0 )
{
float fpGlow = CalcPointLightGlow( vSpace, zValue ) ;
vShading += u_vDLColor * (u_fpAirScattering * fpGlow) ;
}
}
float fpNormalLight = dot( vNormal, vDirection ) ;
if ( (fpNormalLight < 0.0) || (fpFocusParam * fpNormalLight >= 0.0) )
{
fpNormalLight = abs( fpNormalLight ) ;
vShading += vDiffusion * u_vDLColor
* (fpNormalLight * fpBrightness * u_fpDiffusion) ;
if ( u_fpSpecular != 0.0 )
{
vec3 vRefLight = vDirection ;
vRefLight -= vNormal * (2.0 * dot( vDirection, vNormal )) ;
float cosRefLight = - dot( vRefLight, vPosDir ) ;
if ( cosRefLight > 0.0 )
{
float fpSpecularPow = 1.0 / max( vSpecular.z, 0.00390625 ) ;
float fpSpecular = pow( cosRefLight, fpSpecularPow ) ;
vShading += u_vDLColor * (fpSpecular * fpBrightness * vSpecular.x) ;
}
}
}
vShading *= v_vVertexMulColor.a ;
write_frag2( vec4( vShading, 0.0 ), vShading ) ;
}
