uniform sampler2D u_samplerComposed ;
uniform sampler2D u_samplerAmbient ;
uniform vec2 u_vSamplingScale ;
uniform float u_fpBlendAO ;
uniform float u_fpBlendGI ;
uniform vec3 u_rgbAOShadeColor ;
void main( void )
{
vec2 uvSample1 =
v_vTextureCoord * u_vMaterialTextureScale + u_vMaterialTextureBase ;
vec2 uvX = vec2( u_vMaterialTextureScale.x, 0.0 ) ;
vec2 uvY = vec2( 0.0, u_vMaterialTextureScale.y ) ;
vec2 uvSample0 = uvSample1 - uvY ;
vec2 uvSample2 = uvSample1 + uvY ;
vec4 vPx00 = texture2D( u_samplerMaterialTexture, uvSample0 - uvX ) ;
vec4 vPx01 = texture2D( u_samplerMaterialTexture, uvSample0 ) ;
vec4 vPx02 = texture2D( u_samplerMaterialTexture, uvSample0 + uvX ) ;
vec4 vPx10 = texture2D( u_samplerMaterialTexture, uvSample1 - uvX ) ;
vec4 vPx11 = texture2D( u_samplerMaterialTexture, uvSample1 ) ;
vec4 vPx12 = texture2D( u_samplerMaterialTexture, uvSample1 + uvX ) ;
vec4 vPx20 = texture2D( u_samplerMaterialTexture, uvSample2 - uvX ) ;
vec4 vPx21 = texture2D( u_samplerMaterialTexture, uvSample2 ) ;
vec4 vPx22 = texture2D( u_samplerMaterialTexture, uvSample2 + uvX ) ;
vPx01 = vPx01 * 0.125 + (vPx00 + vPx02) * 0.0625 ;
vPx11 = vPx11 * 0.25 + (vPx10 + vPx12) * 0.125 ;
vPx21 = vPx21 * 0.125 + (vPx20 + vPx22) * 0.0625 ;
vPx11 = vPx11 + vPx01 + vPx21 ;
vec2 uvScreen = v_vTextureCoord * u_vMaterialTextureScale
* u_vSamplingScale + u_vMaterialTextureBase ;
vec4 vComposed = texture2D( u_samplerComposed, uvScreen ) ;
vec4 vAmbient = texture2D( u_samplerAmbient, uvScreen ) ;
vec3 vLight = vComposed.rgb
+ (u_rgbAOShadeColor - vAmbient.rgb)
* ((1.0 - vPx11.a) * u_fpBlendAO)
+ vPx11.rgb * u_fpBlendGI ;
write_frag2( vec4( vLight.rgb, 1.0 ), vPx11.rgb * u_fpBlendGI ) ;
}
