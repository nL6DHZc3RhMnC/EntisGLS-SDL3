
//////////////////////////////////////////////////////////////////////////////
// 繧ｬ繧ｦ繧ｹ縺ｼ縺九＠
//////////////////////////////////////////////////////////////////////////////

uniform mediump float	u_fpGaussianWeight[9] ;
uniform mediump vec2	u_vSamplingDirection ;


void main( void )
{
	vec4	rgbaSum ;
	vec4	rgbaSampling1 ;
	vec4	rgbaSampling2 ;
	//
	utilTextureColor( rgbaSum, vec2( 0.0, 0.0 ) ) ;
	rgbaSum *= u_fpGaussianWeight[0] ;
	//
	vec2	vOffset = u_vSamplingDirection ;
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[1] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[2] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[3] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[4] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[5] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[6] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	vOffset += u_vSamplingDirection ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[7] ;
	//
	utilTextureColor( rgbaSampling1, - vOffset ) ;
	utilTextureColor( rgbaSampling2, vOffset ) ;
	rgbaSum += (rgbaSampling1 + rgbaSampling2) * u_fpGaussianWeight[8] ;
	//
	write_frag1( rgbaSum * u_fpEffectAlpha ) ;
}


