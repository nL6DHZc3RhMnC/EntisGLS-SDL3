
///////////////////////////////////////////////////////////////////////////////
// 繧ｫ繧ｹ繧ｿ繝繧ｷ繧ｧ繝ｼ繝??繝ｻ繝ｦ繝ｼ繝?ぅ繝ｪ繝?ぅ
///////////////////////////////////////////////////////////////////////////////

// 繝?ヵ繧ｩ繝ｫ繝医ˇ繧ｷ繧ｧ繝ｼ繝??
///////////////////////////////////////////////////////////////////////////////

#ifndef	_DISABLE_SHADING
#ifdef	_PHONG_SHADER

void utilCustomShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in vec3 vPosition, in vec3 vNormal,
		in tex_coord_t vCoordOffset,
		in mediump vec3 vMaterialMulColor, in mediump vec3 vMaterialAddColor,
		in mediump vec3 vMaterialMulShade, in mediump vec3 vMaterialAddShade )
{
	float			fpVertexAlpha = v_vVertexMulColor.a ;
	mediump vec3	vVertexAddColor = v_vVertexAddColor.rgb ;
	mediump vec3	vVertexMulColor = v_vVertexMulColor.rgb ;
	mediump vec3	vEmissionTemp = vec3( 0.0, 0.0, 0.0 ) ;
	#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
	mediump vec3	vTextureCoord = make_tex_coord( v_vTextureCoord ) + vCoordOffset ;
	#else
	mediump vec2	vTextureCoord = v_vTextureCoord + vCoordOffset ;
	#endif
	if ( u_bMaterialShading )
	{
		highp vec3	vRenderPosition ;
		fpVertexAlpha *= effect_lighting
			( vVertexMulColor,
				vVertexAddColor, vEmissionTemp,
				vAmbientColor, vDiffusionColor,
				vDstSpecular, vDstNormal,
				vRenderPosition,
				vPosition, vNormal, vTextureCoord,
				vMaterialMulColor, vMaterialAddColor,
				vMaterialMulShade, vMaterialAddShade,
				vVertexMulColor, vVertexAddColor ) ;
		vVertexAddColor *= v_vVertexMulColor.a ;
	}
	else
	{
		fpVertexAlpha =
			texture_mapping_shader
				( vVertexAddColor, vEmissionTemp,
					vVertexAddColor, vVertexMulColor,
					vTextureCoord, u_fpEffectAlpha * fpVertexAlpha ) ;
		vAmbientColor = vec3( 0.0, 0.0, 0.0 ) ;
		vDiffusionColor = vec3( 0.0, 0.0, 0.0 ) ;
		vDstSpecular = vec3( 0.0, 0.0, 0.0 ) ;
		vDstNormal = vec3( 0.0, 0.0, 0.0 ) ;
	}
	vDstColor = vec4( vVertexAddColor, fpVertexAlpha ) ;
	vEmissionColor = vec4( vEmissionTemp, fpVertexAlpha ) ;
}

void utilDefaultShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in tex_coord_t vCoordOffset )
{
	utilCustomShader
		( vDstColor, vEmissionColor,
			vAmbientColor, vDiffusionColor,
			vDstSpecular, vDstNormal,
			v_vPosition.xyz, v_vNormal, vCoordOffset,
			u_vMaterialMulColor, u_vMaterialAddColor,
			u_vMaterialMulShade, u_vMaterialAddShade ) ;
}

#else

void utilDefaultShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in tex_coord_t vCoordOffset )
{
	mediump vec3	vVertexAddColor = v_vVertexAddColor.rgb ;
	mediump vec3	vEmissionTemp ;
	float	fpVertexAlpha =
		texture_mapping_shader
			( vVertexAddColor, vEmissionTemp,
				vVertexAddColor, v_vVertexMulColor.rgb,
				vec2( v_vTextureCoord.s + vCoordOffset.s,
						v_vTextureCoord.t + vCoordOffset.t ), v_vVertexMulColor.a ) ;
	vDstColor = vec4( vVertexAddColor, fpVertexAlpha ) ;
	vEmissionColor = vec4( vEmissionTemp, 1.0 ) ;
	vAmbientColor = vec3( 0.0, 0.0, 0.0 ) ;
	vDiffusionColor = vec3( 0.0, 0.0, 0.0 ) ;
	vDstSpecular = vec3( 0.0, 0.0, 0.0 ) ;
	vDstNormal = v_vNormal ;
}

void utilCustomShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in tex_coord_t vCoordOffset,
		in mediump vec3 vMaterialMulColor, in mediump vec3 vMaterialAddColor,
		in mediump vec3 vMaterialMulShade, in mediump vec3 vMaterialAddShade )
{
	utilDefaultShader
		( vDstColor, vEmissionColor,
			vAmbientColor, vDiffusionColor,
			vDstSpecular, vDstNormal, vCoordOffset ) ;
}

#endif

#else

void utilDefaultShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in tex_coord_t vCoordOffset )
{
	mediump vec3	vVertexAddColor = v_vVertexAddColor.rgb ;
	mediump vec3	vEmissionTemp ;
	float	fpVertexAlpha =
		texture_mapping_shader
			( vVertexAddColor, vEmissionTemp,
				vVertexAddColor, v_vVertexMulColor.rgb,
				vec2( v_vTextureCoord.s + vCoordOffset.s,
						v_vTextureCoord.t + vCoordOffset.t ), v_vVertexMulColor.a ) ;
	vDstColor = vec4( vVertexAddColor, fpVertexAlpha ) ;
	vEmissionColor = vec4( vEmissionTemp, fpVertexAlpha ) ;
	vAmbientColor = vec3( 0.0, 0.0, 0.0 ) ;
	vDiffusionColor = vec3( 0.0, 0.0, 0.0 ) ;
	vDstSpecular = vec3( 0.0, 0.0, 0.0 ) ;
	vDstNormal = v_vNormal ;
}

void utilCustomShader
	( out mediump vec4 vDstColor,
		out mediump vec4 vEmissionColor,
		out mediump vec3 vAmbientColor,
		out mediump vec3 vDiffusionColor,
		out mediump vec3 vDstSpecular,
		out mediump vec3 vDstNormal,
		in tex_coord_t vCoordOffset,
		in mediump vec3 vMaterialMulColor, in mediump vec3 vMaterialAddColor,
		in mediump vec3 vMaterialMulShade, in mediump vec3 vMaterialAddShade )
{
	utilDefaultShader
		( vDstColor, vEmissionColor,
			vAmbientColor, vDiffusionColor,
			vDstSpecular, vDstNormal, vCoordOffset ) ;
}

#endif


// 繝?ヵ繧ｩ繝ｫ繝医ˇ濶ｲ蜉ｹ譫?
///////////////////////////////////////////////////////////////////////////////

void utilDefaultColorEffect
		( out mediump vec4 vDstColor, in mediump vec4 vSrcColor )
{
	vDstColor =
		vec4( vSrcColor.rgb * v_vVertexMulColor.rgb
								+ v_vVertexAddColor.rgb, vSrcColor.a ) ;
}


// 騾壼ｸｸ繝?け繧ｹ繝√Ε
///////////////////////////////////////////////////////////////////////////////

void utilTextureColor( out mediump vec4 vDstTexture, in tex_coord_t vCoordOffset )
{
	#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
	mediump vec3	vTextureCoord = make_tex_coord( v_vTextureCoord ) + vCoordOffset ;
	#else
	mediump vec2	vTextureCoord = v_vTextureCoord + vCoordOffset ;
	#endif
	vDstTexture =
		tex_sample_px
			( u_samplerMaterialTexture,
				tex_coord_map
					( vTextureCoord,
						u_vMaterialTextureScale,
						u_vMaterialTextureBase ) ) ;
}


// 逋ｺ蜈峨ユ繧ｯ繧ｹ繝√Ε
///////////////////////////////////////////////////////////////////////////////

void utilTextureLuminous( out mediump vec4 vDstTexture, in tex_coord_t vCoordOffset )
{
#ifndef	_DISABLE_LUMINOUS_TEXTURE
	if ( u_fpLuminousTexture > 0.0 )
	{
		#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
		mediump vec3	vTextureCoord = make_tex_coord( v_vTextureCoord ) + vCoordOffset ;
		#else
		mediump vec2	vTextureCoord = v_vTextureCoord + vCoordOffset ;
		#endif
		vDstTexture =
			tex_sample_px
				( u_samplerLuminousTexture,
					tex_coord_map
						( vTextureCoord,
							u_vLuminousTextureScale,
							u_vLuminousTextureBase ) ) ;
	}
	else
	{
		vDstTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
	}
#else
	vDstTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
#endif
}


// 豕慕ｷ壹ユ繧ｯ繧ｹ繝√Ε
///////////////////////////////////////////////////////////////////////////////

void utilTextureNormal( out mediump vec4 vDstTexture, in tex_coord_t vCoordOffset )
{
#ifndef	_DISABLE_NORMAL_TEXTURE
	if ( u_fpNormalTexture > 0.0 )
	{
		#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
		mediump vec3	vTextureCoord = make_tex_coord( v_vTextureCoord ) + vCoordOffset ;
		#else
		mediump vec2	vTextureCoord = v_vTextureCoord + vCoordOffset ;
		#endif
		vDstTexture =
			tex_sample_px
				( u_samplerNormalTexture,
					tex_coord_map
						( vTextureCoord,
							u_vNormalTextureScale,
							u_vNormalTextureBase ) ) ;
	}
	else
	{
		vDstTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
	}
#else
	vDstTexture = vec4( 0.0, 0.0, 0.0, 0.0 ) ;
#endif
}


// ﾎｱ繝?け繧ｹ繝√Ε
///////////////////////////////////////////////////////////////////////////////

float utilTextureAlpha( in tex_coord_t vCoordOffset )
{
#ifndef	_DISABLE_ALPHA_MAPPING
	if ( u_bMaterialAlphaTexture )
	{
		#ifdef	_ENABLE_MATERIAL_TEXTURE_3D
		mediump vec3	vTextureCoord = make_tex_coord( v_vTextureCoord ) + vCoordOffset ;
		#else
		mediump vec2	vTextureCoord = v_vTextureCoord + vCoordOffset ;
		#endif
		return	tex_sample_px
					( u_samplerAlphaMapping,
						tex_coord_map
							( vTextureCoord,
								u_vAlphaMapingTextureScale,
								u_vAlphaMapingTextureBase ) ).r ;
	}
#endif
	return	0.0 ;
}




