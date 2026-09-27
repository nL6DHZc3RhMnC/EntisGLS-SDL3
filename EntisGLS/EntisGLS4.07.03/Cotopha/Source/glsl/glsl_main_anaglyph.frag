
//////////////////////////////////////////////////////////////////////////////
// 繧｢繝翫げ繝ｪ繝?
//////////////////////////////////////////////////////////////////////////////

void main( void )
{
	vec4	rgbaRight ;
	vec4	rgbaLeft ;
	//
	utilTextureColor( rgbaRight, vec2( 0.0, 0.0 ) ) ;
	utilTextureLuminous( rgbaLeft, vec2( 0.0, 0.0 ) ) ;
	//
	write_frag1( vec4( (rgbaLeft.b + rgbaLeft.g) * 0.25 + rgbaLeft.r * 0.5,
						rgbaRight.g, (rgbaRight.b + rgbaRight.b) * 0.5, 1.0 ) ) ;
}


