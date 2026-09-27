uniform mediump float u_fpAmplitude ;
uniform mediump vec4 u_vAmplitude ;
uniform mediump vec4 u_vFrequency ;
uniform mediump vec4 u_vTime ;
uniform mediump vec2 u_vDirection[4] ;
uniform mediump vec3 u_vWaterAxisX ;
uniform mediump vec3 u_vWaterAxisY ;
varying mediump vec2 v_vWaveCoord ;
void main( void )
{
morph_vertex() ;
transform_vertex
( v_vPosition, v_vNormal, v_vPosition.xyz, v_vNormal ) ;
vec4 vWavePosition = v_vPosition - u_mat4CameraView * vec4( 0.0, 0.0, 0.0, 1.0 ) ;
vec3 vWaterAxisX = u_mat3CameraViewForNormal * u_vWaterAxisX ;
vec3 vWaterAxisY = u_mat3CameraViewForNormal * u_vWaterAxisY ;
vec3 vAxisCross = normalize( cross( vWaterAxisX, vWaterAxisY ) ) ;
vec2 vWaveCoord =
vec2( dot( cross( vWavePosition.xyz, vWaterAxisY ), vAxisCross ),
dot( cross( vWaterAxisX, vWavePosition.xyz ), vAxisCross ) ) ;
vec2 a2 = u_vDirection[0] ;
vec2 b2 = u_vDirection[1] ;
vec2 c2 = u_vDirection[2] ;
vec2 d2 = u_vDirection[3] ;
vec4 phase2 = u_vFrequency
* vec4( dot( a2, vWaveCoord ),
dot( b2, vWaveCoord ),
dot( c2, vWaveCoord ),
dot( d2, vWaveCoord ) ) + u_vTime ;
vec4 vSin2 = sin( phase2 ) ;
vec3 vOffset2 =
vec3( 0.0 ,
dot( vSin2, u_vAmplitude ),
0.0 ) ;
vec3 vNormal = normalize( v_vNormal ) ;
v_vPosition += vec4( vNormal * (vOffset2.y * u_fpAmplitude), 0.0 ) ;
gl_Position = u_mat4PerspectiveView * v_vPosition ;
v_vWaveCoord = vWaveCoord ;
v_vVertexMulColor.a *= u_fpEffectAlpha ;
}
