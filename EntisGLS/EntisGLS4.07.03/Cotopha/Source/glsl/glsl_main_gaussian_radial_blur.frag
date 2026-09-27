
//////////////////////////////////////////////////////////////////////////////
// 繧ｬ繧ｦ繧ｹ縺ｼ縺九＠?域叛蟆?憾??
//////////////////////////////////////////////////////////////////////////////

uniform mediump float	u_fpGaussianWeight[9] ;
uniform mediump vec2	u_vRadialCenter ;
uniform mediump float	u_fpSamplingUnit ;
uniform mediump float	u_fpBlurScale ;
uniform mediump float	u_fpBlurPower ;


void main( void )
{
	vec4	rgbaSum ;
	vec4	rgbaSampling1 ;
	vec4	rgbaSampling2 ;
	//
	utilTextureColor( rgbaSum, vec2( 0.0, 0.0 ) ) ;
	rgbaSum *= u_fpGaussianWeight[0] ;
	//
	vec2	vDelta = v_vTextureCoord - u_vRadialCenter ;
	vec2	vOffset = normalize( vDelta ) * u_fpSamplingUnit
						* pow( length( vDelta ) * u_fpBlurScale, u_fpBlurPower ) ;
	vec2	vDirection = vOffset ;
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[1] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[2] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[3] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[4] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[5] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[6] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += vDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[7] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[8] ;
	//
	write_frag1( rgbaSum * u_fpEffectAlpha ) ;
}


