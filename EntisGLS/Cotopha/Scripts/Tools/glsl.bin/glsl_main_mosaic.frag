uniform mediump vec2 u_vMosaicSize ;
void main( void )
{
highp vec4 vPersPos = u_mat4PerspectiveView * v_vPosition ;
highp vec2 vViewPos = vPersPos.xy * (0.5 / vPersPos.w) + 0.5 ;
vViewPos = floor( vViewPos / u_vMosaicSize ) * u_vMosaicSize ;
vec4 vTexture = texture2D( u_samplerViewportMapping, vViewPos ) ;
write_frag2( vTexture, vec3( 0.0, 0.0, 0.0 ) ) ;
}
