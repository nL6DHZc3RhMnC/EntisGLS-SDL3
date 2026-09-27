uniform sampler2D u_samplerDepth ;
uniform sampler2D u_samplerNormal ;
uniform sampler2D u_samplerEmission ;
uniform sampler2D u_samplerDiffusion ;
uniform sampler2D u_samplerSpecular ;
uniform highp mat4 u_mat4SamplePers ;
uniform bool u_bWith3WayMapping ;
uniform sampler2D u_sampler3WayComposed ;
uniform sampler2D u_sampler3WayEmission ;
uniform sampler2D u_sampler3WayDepth ;
uniform highp mat4 u_mat4Sample3WayPers ;
uniform highp float u_fpReachAO ;
uniform highp float u_fpReachAObyZ ;
uniform int u_nAOSamplingCount ;
uniform int u_nGISamplingCount ;
uniform float u_fpDiffusionLuminousness ;
float initRandomizer( in vec2 uv )
{
float a = fract( dot( uv, vec2( 2.067390879775102, 12.451168662908249 ) ) ) - 0.5 ;
float s = a * ( 6.182785114200511 + a*a * (-38.026512460676566 + a*a * 53.392573080032137) ) ;
return fract( s * 43758.5453 ) ;
}
float genRandom( inout float rnd )
{
rnd = fract( rnd * 15.6875 + 9.23670482635498046875 ) ;
return rnd ;
}
float attenuatedRandom( inout float rnd, in float fpReach )
{
float x = genRandom( rnd ) ;
return (2.0 - 2.0 * sqrt( 1.0 - x )) * (fpReach * 0.5) ;
}
vec3 GetDepthPosition( in vec2 uvSample )
{
float m00 = u_mat4SamplePers[0].x ;
float m03 = u_mat4SamplePers[3].x ;
float m11 = u_mat4SamplePers[1].y ;
float m13 = u_mat4SamplePers[3].y ;
float m22 = u_mat4SamplePers[2].z ;
float m32 = u_mat4SamplePers[2].w ;
float m23 = u_mat4SamplePers[3].z ;
float m33 = u_mat4SamplePers[3].w ;
float zDepthSample = texture2D( u_samplerDepth, uvSample ).x ;
float zDepth = zDepthSample * 2.0 - 1.0 ;
float zValue = (m23 - m33 * zDepth) / (m32 * zDepth - m22) ;
float wPers = m32 * zValue + m33 ;
float xSpace = ((uvSample.x * 2.0 - 1.0) * wPers - m03) / m00 ;
float ySpace = ((uvSample.y * 2.0 - 1.0) * wPers - m13) / m11 ;
return vec3( xSpace, ySpace, zValue ) ;
}
float SampleGlobalIllumination
( inout vec3 vDiffColor, inout float giDiffTotal,
inout vec3 vSpecColor, inout vec3 vReflColor, inout float giSpecTotal,
in vec3 vSamplePos, in vec2 vSamplePorjPos,
in vec3 vOriginPos, in vec3 vNormal,
in vec3 vReflectDir, float fpSpecularPower )
{
vec3 vSampleEmis = texture2D( u_samplerEmission, vSamplePorjPos.xy ).rgb ;
vec3 vSampleDiff = texture2D( u_samplerMaterialTexture, vSamplePorjPos.xy ).rgb ;
vec3 vSampleIllm = vSampleEmis + (vSampleDiff - vSampleEmis)
* u_fpDiffusionLuminousness ;
vec3 vDelta = vSamplePos - vOriginPos ;
vec3 vDeltaDir = normalize( vDelta ) ;
float cosDiff = dot( vDeltaDir, vNormal ) ;
float cosRefl = dot( vDeltaDir, vReflectDir ) ;
if ( cosDiff > 0.01 )
{
giDiffTotal += 1.0 ;
vDiffColor += vSampleIllm * cosDiff ;
if ( cosRefl > 0.01 )
{
float fpRefl = pow( cosRefl, fpSpecularPower ) ;
giSpecTotal += fpRefl ;
vSpecColor += vSampleIllm * fpRefl ;
vReflColor += vSampleDiff * fpRefl ;
return cosRefl ;
}
}
return 0.0 ;
}
float Sample3WayGlobalIllumination
( inout float aoVisible, inout float aoTotal,
inout vec3 vDiffColor, inout float giDiffTotal,
inout vec3 vSpecColor, inout vec3 vReflColor, inout float giSpecTotal,
in vec3 vSamplePos, in vec3 vOriginPos, in float fpReachAO,
in vec3 vNormal, in vec3 vReflectDir, float fpSpecularPower )
{
float r = vSamplePos.x * vSamplePos.x + vSamplePos.z * vSamplePos.z ;
float iWay = 1.0 ;
float cosWay = vSamplePos.z / sqrt( r ) ;
float sinWay = 0.0 ;
vec3 vTransPos = vSamplePos ;
if ( cosWay < 0.5 )
{
iWay = (vSamplePos.x < 0.0) ? 0.0 : 2.0 ;
cosWay = -0.5 ;
sinWay = (iWay - 1.0) * 0.86602540378444 ;
vTransPos.x = cosWay * vSamplePos.x - sinWay * vSamplePos.z ;
vTransPos.z = cosWay * vSamplePos.z + sinWay * vSamplePos.x ;
}
vec4 vPersPos = u_mat4Sample3WayPers * vec4( vTransPos, 1.0 ) ;
vec3 vProjPos = vPersPos.xyz * (0.5 / vPersPos.w) + 0.5 ;
float xMapPos = (iWay + max( min( vProjPos.x, 1.0 ), 0.0 )) / 3.0 ;
vec2 uvSample = vec2( xMapPos, vProjPos.y ) ;
float m00 = u_mat4Sample3WayPers[0].x ;
float m03 = u_mat4Sample3WayPers[3].x ;
float m11 = u_mat4Sample3WayPers[1].y ;
float m13 = u_mat4Sample3WayPers[3].y ;
float m22 = u_mat4Sample3WayPers[2].z ;
float m32 = u_mat4Sample3WayPers[2].w ;
float m23 = u_mat4Sample3WayPers[3].z ;
float m33 = u_mat4Sample3WayPers[3].w ;
float zDepthSample = texture2D( u_sampler3WayDepth, uvSample ).x ;
float zDepth = zDepthSample * 2.0 - 1.0 ;
float zValue = (m23 - m33 * zDepth) / (m32 * zDepth - m22) ;
float wPers = m32 * zValue + m33 ;
float xSpace = ((vProjPos.x * 2.0 - 1.0) * wPers - m03) / m00 ;
float ySpace = ((vProjPos.y * 2.0 - 1.0) * wPers - m13) / m11 ;
vec3 vSpace ;
vSpace.x = cosWay * xSpace + sinWay * zValue ;
vSpace.y = ySpace ;
vSpace.z = cosWay * zValue - sinWay * xSpace ;
vec3 vDelta = vSamplePos - vOriginPos ;
vec3 vDeltaDir = normalize( vDelta ) ;
r = length( vDelta ) ;
if ( r < fpReachAO )
{
float ao = (1.0 - r / fpReachAO) * dot( vDeltaDir, vNormal ) ;
aoTotal += ao ;
float zSample = length( vSamplePos ) ;
float zSpace = length( vSpace ) ;
if ( zSample <= zSpace * 1.001 )
{
aoVisible += ao ;
}
else
{
float dz = abs( zSample - zSpace ) ;
aoVisible += ao * min( 1.0, dz * 0.25 / u_fpReachAO ) ;
}
}
vec3 vSampleDiff = texture2D( u_sampler3WayComposed, uvSample ).rgb ;
vec3 vSampleEmis = texture2D( u_sampler3WayEmission, uvSample ).rgb ;
vec3 vSampleIllm = vSampleEmis + (vSampleDiff - vSampleEmis)
* u_fpDiffusionLuminousness ;
float cosDiff = dot( vDeltaDir, vNormal ) ;
float cosRefl = dot( vDeltaDir, vReflectDir ) ;
if ( cosDiff > 0.01 )
{
giDiffTotal += 1.0 ;
vDiffColor += vSampleIllm * cosDiff ;
if ( cosRefl > 0.01 )
{
float fpRefl = pow( cosRefl, fpSpecularPower ) ;
giSpecTotal += fpRefl ;
vSpecColor += vSampleIllm * fpRefl ;
vReflColor += vSampleDiff * fpRefl ;
return cosRefl ;
}
}
return 0.0 ;
}
void main( void )
{
vec2 uvSample =
v_vTextureCoord * u_vMaterialTextureScale + u_vMaterialTextureBase ;
float rndSeed = initRandomizer( uvSample ) ;
vec3 vSpace = GetDepthPosition( uvSample ) ;
vec3 vNormal = normalize
( texture2D( u_samplerNormal, uvSample ).xyz * 2.0 - 1.0 ) ;
vec3 vPosDir = normalize( vSpace ) ;
if ( dot( vNormal, vPosDir ) > 0.0 )
{
vNormal = - vNormal ;
}
float fpReachAO = min( vSpace.z * u_fpReachAObyZ,
u_fpReachAO + vSpace.z * 0.05 ) ;
vec3 vReflectDir =
vPosDir - vNormal * (dot( vPosDir, vNormal ) * 2.0) ;
vec4 vDiffusion = texture2D( u_samplerDiffusion, uvSample ) ;
vec4 vEmission = texture2D( u_samplerEmission, uvSample ) ;
vec4 vSpecular = texture2D( u_samplerSpecular, uvSample ) ;
float fpSpecular = vSpecular.x ;
float fpReflection = vSpecular.y ;
float fpSpecularPower = 1.0 / max( vSpecular.z, 0.00390625 ) ;
float aoTotal = 0.0 ;
float aoVisible = 0.0 ;
float giDiffTotal = 0.0 ;
float giSpecTotal = 0.0 ;
vec3 vDiffColor = vec3( 0.0, 0.0, 0.0 ) ;
vec3 vSpecColor = vec3( 0.0, 0.0, 0.0 ) ;
vec3 vReflColor = vec3( 0.0, 0.0, 0.0 ) ;
vec3 vBaseX, vBaseY ;
if ( abs( vNormal.x ) < abs( vNormal.y ) )
{
vBaseX = cross( vNormal, vec3( 1.0, 0.0, 0.0 ) ) ;
vBaseY = cross( vNormal, vBaseX ) ;
}
else
{
vBaseX = cross( vNormal, vec3( 0.0, 1.0, 0.0 ) ) ;
vBaseY = cross( vNormal, vBaseX ) ;
}
int i ;
for ( i = 0; i < u_nAOSamplingCount; i ++ )
{
float r = attenuatedRandom( rndSeed, 0.99 ) + 0.01 ;
float c = attenuatedRandom( rndSeed, 1.0 ) ;
float s = sqrt( 1.0 - c*c ) ;
float x = genRandom( rndSeed ) * 2.0 - 1.0 ;
float y = genRandom( rndSeed ) * 2.0 - 1.0 ;
float d = fpReachAO * r * c / sqrt( x*x + y*y ) ;
float ao = (1.0 - r) * s ;
vec3 vRnd = vSpace + vNormal * (fpReachAO * r * s)
+ vBaseX * (x * d) + vBaseY * (y * d) ;
vec4 vRndPers = u_mat4SamplePers
* vec4( vRnd.x, vRnd.y, vRnd.z, 1.0 ) ;
vec3 vRndProj = vRndPers.xyz * (0.5 / vRndPers.w) + 0.5 ;
if ( !u_bWith3WayMapping
|| ((vRnd.z > 0.0)
&& (abs( vRndProj.x - 0.5) <= 0.5)
&& (abs( vRndProj.y - 0.5) <= 0.5)) )
{
vec3 vSample = GetDepthPosition( vRndProj.xy ) ;
aoTotal += ao ;
if ( (vRnd.z <= 0.0) || (vRnd.z <= vSample.z * 1.001) )
{
aoVisible += ao ;
}
else
{
float dz = abs( vRnd.z - vSample.z ) ;
aoVisible += ao * min( 1.0, dz * 0.25 / u_fpReachAO ) ;
}
SampleGlobalIllumination
( vDiffColor, giDiffTotal,
vSpecColor, vReflColor, giSpecTotal,
vSample, vRndProj.xy,
vSpace, vNormal, vReflectDir, fpSpecularPower ) ;
}
else
{
Sample3WayGlobalIllumination
( aoVisible, aoTotal,
vDiffColor, giDiffTotal,
vSpecColor, vReflColor, giSpecTotal,
vRnd, vSpace, fpReachAO,
vNormal, vReflectDir, fpSpecularPower ) ;
}
}
if ( abs( vReflectDir.x ) < abs( vReflectDir.y ) )
{
vBaseX = cross( vReflectDir, vec3( 1.0, 0.0, 0.0 ) ) ;
vBaseY = cross( vReflectDir, vBaseX ) ;
}
else
{
vBaseX = cross( vReflectDir, vec3( 0.0, 1.0, 0.0 ) ) ;
vBaseY = cross( vReflectDir, vBaseX ) ;
}
float fpGIReach = vSpace.z ;
if ( u_bWith3WayMapping )
{
fpGIReach *= 2.0 ;
}
float rBase = 0.01 ;
float xBase = 0.0 ;
float rScale = 0.99 ;
for ( i = 0; i < u_nGISamplingCount; i ++ )
{
float r = genRandom( rndSeed ) * rScale + rBase ;
float c = attenuatedRandom( rndSeed, 0.5 ) ;
float s = sqrt( 1.0 - c*c ) ;
float x = (genRandom( rndSeed ) * 2.0 - 1.0) * rScale + xBase ;
float d = fpGIReach * r * c ;
vec3 vRnd = vSpace + vReflectDir * (fpGIReach * r * s)
+ vNormal * (x * d) ;
vec4 vRndPers = u_mat4SamplePers
* vec4( vRnd.x, vRnd.y, vRnd.z, 1.0 ) ;
vec3 vRndProj = vRndPers.xyz * (0.5 / vRndPers.w) + 0.5 ;
float cosReflCur = 0.0 ;
if ( !u_bWith3WayMapping
)
{
vec3 vSample = GetDepthPosition( vRndProj.xy ) ;
cosReflCur = SampleGlobalIllumination
( vDiffColor, giDiffTotal,
vSpecColor, vReflColor, giSpecTotal,
vSample, vRndProj.xy,
vSpace, vNormal, vReflectDir, fpSpecularPower ) ;
}
else
{
cosReflCur = Sample3WayGlobalIllumination
( aoVisible, aoTotal,
vDiffColor, giDiffTotal,
vSpecColor, vReflColor, giSpecTotal,
vRnd, vSpace, fpReachAO,
vNormal, vReflectDir, fpSpecularPower ) ;
}
}
aoTotal = max( aoTotal, 0.001 ) ;
giDiffTotal = max( giDiffTotal, 0.001 ) ;
giSpecTotal = max( giSpecTotal, 0.001 ) ;
float ao = aoVisible / aoTotal ;
vec3 vGIColor = vDiffColor * vDiffusion.rgb / giDiffTotal
+ vSpecColor * (fpSpecular / giSpecTotal)
+ vReflColor * (fpReflection / giSpecTotal) ;
vGIColor = max( vGIColor - vEmission.rgb, vec3( 0.0, 0.0, 0.0 ) ) ;
write_frag1( vec4( vGIColor, ao ) ) ;
}
