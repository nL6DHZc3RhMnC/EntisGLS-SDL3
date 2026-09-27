uniform bool u_bVertical ;
uniform bool u_bOddLine ;
void main( void )
{
float x ;
if ( u_bVertical )
{
x = floor(gl_FragCoord.x) * 0.5 ;
}
else
{
x = floor(gl_FragCoord.y) * 0.5 ;
}
bool bSelectRight = (x - floor( x ) >= 0.5) ;
if ( u_bOddLine )
{
bSelectRight = !bSelectRight ;
}
vec4 rgbaTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
if ( bSelectRight )
{
utilTextureColor( rgbaTexture, vec2( 0.0, 0.0 ) ) ;
}
else
{
utilTextureLuminous( rgbaTexture, vec2( 0.0, 0.0 ) ) ;
}
write_frag1( rgbaTexture ) ;
}
