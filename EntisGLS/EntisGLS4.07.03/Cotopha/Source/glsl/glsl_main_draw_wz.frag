
///////////////////////////////////////////////////////////////////////////////
// ｚ深度マッピング付き画像描画
///////////////////////////////////////////////////////////////////////////////

uniform sampler2D	u_samplerDepth ;		// ｚバッファ
uniform highp mat4	u_mat4SamplePers ;		// ソースの透視変換行列

void main( void )
{
	float			fpVertexAlpha = v_vVertexMulColor.a ;
	mediump vec3	vVertexAddColor = v_vVertexAddColor.rgb ;
	mediump vec3	vVertexMulColor = v_vVertexMulColor.rgb ;
	mediump vec3	vEmissionColor = vec3( 0.0, 0.0, 0.0 ) ;
	mediump vec3	vAmbientColor = vec3( 0.0, 0.0, 0.0 ) ;
	mediump vec3	vDiffusionColor = vec3( 0.0, 0.0, 0.0 ) ;
	mediump vec3	vDstSpecular = vec3( 0.0, 0.0, 0.0 ) ;
	mediump vec3	vDstNormal = v_vNormal ;

	// シェーディング無し
	vVertexAddColor += u_vMaterialAddColor * vVertexMulColor ;
	vVertexMulColor = u_vMaterialMulColor * vVertexMulColor ;
	fpVertexAlpha =
		texture_mapping_shader
			( vVertexAddColor, vEmissionColor,
				vVertexAddColor, vVertexMulColor,
				vec2( v_vTextureCoord.s, v_vTextureCoord.t ),
				u_fpEffectAlpha * fpVertexAlpha ) ;

	// ソース depth -> z 座標
	vec2	uvSample =
				v_vTextureCoord * u_vMaterialTextureScale + u_vMaterialTextureBase ;
	float	m22 = u_mat4SamplePers[2].z ;
	float	m32 = u_mat4SamplePers[2].w ;
	float	m23 = u_mat4SamplePers[3].z ;
	float	m33 = u_mat4SamplePers[3].w ;
	float	zDepthSample = texture2D( u_samplerDepth, uvSample ).x ;
	float	zDepth = zDepthSample * 2.0 - 1.0 ;
	float	zValue = (m23 - m33 * zDepth) / (m32 * zDepth - m22) ;

	// 出力値
	vec4	vPers = u_mat4PerspectiveView * vec4( 0.0, 0.0, zValue, 1.0 ) ;
	write_depth( vPers.z * 0.5 / vPers.w + 0.5 ) ;

	write_frag6
		( vec4( vVertexAddColor, fpVertexAlpha ), vEmissionColor,
				vDstNormal, vDiffusionColor, vAmbientColor, vDstSpecular ) ;
}


