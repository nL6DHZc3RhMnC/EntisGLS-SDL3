
//////////////////////////////////////////////////////////////////////////////
// 豺ｱ蠎ｦ繝舌ャ繝輔ぃ蜷域??域ｷｱ蠎ｦ縺ｼ縺九＠蜷域?逕ｨ??
//////////////////////////////////////////////////////////////////////////////

uniform sampler2D	u_samplerDepth ;			// depth texture
uniform highp vec2	u_vDepthTextureScale ;
uniform highp float	u_fpFocusDepth ;			// 辟ｦ轤ｹ豺ｱ蠎ｦ
uniform highp float	u_fpFocusNearRange ;		// 縺ｼ縺九＠蟷?
uniform highp float	u_fpFocusFarRange ;
uniform highp float	u_fpPersM22 ;				// depth竊抵ｽ夊ｨ育ｮ礼畑
uniform highp float	u_fpPersM23 ;


void main( void )
{
	vec4	rgbaTexture ;
	utilTextureColor( rgbaTexture, vec2( 0.0, 0.0 ) ) ;
	rgbaTexture.a = 1.0 ;
	//
	float	zDepth =
				texture2D( u_samplerDepth,
							v_vTextureCoord * u_vDepthTextureScale ).r ;
	float	zValue = u_fpPersM23 / ((zDepth * 2.0 - 1.0) - u_fpPersM22) ;
	float	aBlend = zValue - u_fpFocusDepth ;
	if ( aBlend > 0.0 )
	{
		aBlend = abs( aBlend * u_fpFocusFarRange ) ;
	}
	else
	{
		aBlend = abs( aBlend * u_fpFocusNearRange ) ;
	}
	//
	write_frag2( rgbaTexture
					* (u_fpEffectAlpha * min( aBlend, 1.0 )),
				vec3( 0.0, 0.0, 0.0 ) ) ;
}


