uniform sampler2D u_samplerDepth ;
uniform highp vec2 u_vDepthTextureScale ;
uniform highp float u_fpFocusDepth ;
uniform highp float u_fpFocusNearRange ;
uniform highp float u_fpFocusFarRange ;
uniform highp float u_fpPersM22 ;
uniform highp float u_fpPersM23 ;
void main( void )
{
vec4 rgbaTexture ;
utilTextureColor( rgbaTexture, vec2( 0.0, 0.0 ) ) ;
rgbaTexture.a = 1.0 ;
float zDepth =
texture2D( u_samplerDepth,
v_vTextureCoord * u_vDepthTextureScale ).r ;
float zValue = u_fpPersM23 / ((zDepth * 2.0 - 1.0) - u_fpPersM22) ;
float aBlend = zValue - u_fpFocusDepth ;
if ( aBlend > 0.0 )
{
aBlend = abs( aBlend * u_fpFocusFarRange ) ;
}
else
{
aBlend = abs( aBlend * u_fpFocusNearRange ) ;
}
write_frag2( rgbaTexture
* (u_fpEffectAlpha * min( aBlend, 1.0 )),
vec3( 0.0, 0.0, 0.0 ) ) ;
}
