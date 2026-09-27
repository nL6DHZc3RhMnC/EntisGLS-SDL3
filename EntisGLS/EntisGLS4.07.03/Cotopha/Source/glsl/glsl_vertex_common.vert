
///////////////////////////////////////////////////////////////////////////////
// インスタンシング
///////////////////////////////////////////////////////////////////////////////

mat3 getInstanceMatrix3()
{
	#ifdef	_ENABLE_INSTANCING
		vec3	m0 = vec3( a_matInstancingModelView0.x,
							a_matInstancingModelView1.x,
							a_matInstancingModelView2.x ) ;
		vec3	m1 = vec3( a_matInstancingModelView0.y,
							a_matInstancingModelView1.y,
							a_matInstancingModelView2.y ) ;
		vec3	m2 = vec3( a_matInstancingModelView0.z,
							a_matInstancingModelView1.z,
							a_matInstancingModelView2.z ) ;
	#else
		vec3	m0 = vec3( 1.0, 0.0, 0.0 ) ;
		vec3	m1 = vec3( 0.0, 1.0, 0.0 ) ;
		vec3	m2 = vec3( 0.0, 0.0, 1.0 ) ;
	#endif
	return	mat3( m0, m1, m2 ) ;
}

vec3 getInstancePosition()
{
	#ifdef	_ENABLE_INSTANCING
		return	vec3( a_matInstancingModelView0.w,
						a_matInstancingModelView1.w,
						a_matInstancingModelView2.w ) ;
	#else
		return	vec3( 0.0, 0.0, 0.0 ) ;
	#endif
}



///////////////////////////////////////////////////////////////////////////////
// 拡張属性取得
///////////////////////////////////////////////////////////////////////////////

#ifdef	_ENABLE_VT_INSTANCING

vec4 getExAttrElement( int iElement )
{
	float	fVertexID = float( gl_VertexID ) ;
	float	fElement = float( iElement ) ;
	float	xv = (fVertexID * u_xVertexExAttrStride + fElement) * u_vVertexTextureScale.x ;
	float	yv = floor( xv ) ;
	xv -= yv ;
	yv *= u_vVertexTextureScale.y ;
	return	texture2D( u_samplerVertex,
						vec2( xv, yv + u_yVertexExAttrElements ) ) ;
}

#else

vec4 getExAttrElement( int iElement )
{
	return	vec4( 0.0, 0.0, 0.0, 0.0 ) ;
}

#endif



///////////////////////////////////////////////////////////////////////////////
// 頂点モーフィング
///////////////////////////////////////////////////////////////////////////////

#ifdef	_ENABLE_VT_INSTANCING
void morph_vertex_texture( in float xv, in float yv, in float weight, in float index )
{
	float	ym = u_yVertexMorphingStride * index ;
	vec4	vVertex = texture2D( u_samplerVertex,
						vec2( xv, yv + ym + u_yVertexMorphingFirst ) ) ;
	vec4	vNormal = texture2D( u_samplerVertex,
						vec2( xv, yv + ym + u_yNormalMorphingFirst ) ) ;
	v_vPosition += vec4( vVertex.xyz * weight, 0.0 ) ;
	v_vNormal += vNormal.xyz * weight ;
}
#endif

void morph_vertex( void )
{
	v_vPosition = vec4( a_vVertexPosition, 1.0 ) ;
	v_vNormal = a_vVertexNormal ;
//	if ( dot( v_vNormal, v_vNormal ) == 0.0 )
//	{
//		v_vNormal = vec3( 0.0, 0.0, -1.0 ) ;
//	}
	//
#ifdef	_PHONG_SHADER
#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
	v_vTextureAxisX = a_vVertexMappingX ;
	v_vTextureAxisY = a_vVertexMappingY ;
#else
	v_vTextureAxisX = vec3( 1.0, 0.0, 0.0 ) ;
	v_vTextureAxisY = vec3( 0.0, 1.0, 0.0 ) ;
#endif
#endif
	//
	v_vTextureCoord = a_vTextureCoord ;
	v_vVertexAddColor = a_vVertexAddColor.bgra ;
	v_vVertexMulColor = vec4( a_vVertexMulColor.bgr, 1.0 ) ;
	//
	if ( u_bMaterialVertexAlpha )
	{
		v_vVertexMulColor.a = a_vVertexMulColor.a ;
	}
#ifdef	_ENABLE_VT_INSTANCING
	if ( u_bVertexMorphing )
	{
		float	fInstanceID = float( gl_InstanceID ) ;
		float	fVertexID = float( gl_VertexID ) ;
		float	xi = fInstanceID * u_xVertexMorphingInstanceStride ;
		float	yi = floor( xi ) ;
		float	xv = fVertexID * u_vVertexTextureScale.x ;
		float	yv = floor( xv ) ;
		xi -= yi ;
		xv -= yv ;
		yi *= u_vVertexTextureScale.y ;
		yv *= u_vVertexTextureScale.y ;
		//
		vec4	vWeight = texture2D( u_samplerVertex,
							vec2( xi, yi + u_yVertexMorphingInstance ) ) ;
		vec4	vIndex = texture2D( u_samplerVertex,
							vec2( xi + u_vVertexTextureScale.x,
									yi + u_yVertexMorphingInstance ) ) ;
		//
		if ( vWeight.x != 0.0 )
		{
			morph_vertex_texture( xv, yv, vWeight.x, vIndex.x ) ;
		}
		if ( vWeight.y != 0.0 )
		{
			morph_vertex_texture( xv, yv, vWeight.y, vIndex.y ) ;
		}
		if ( vWeight.z != 0.0 )
		{
			morph_vertex_texture( xv, yv, vWeight.z, vIndex.z ) ;
		}
		if ( vWeight.w != 0.0 )
		{
			morph_vertex_texture( xv, yv, vWeight.w, vIndex.w ) ;
		}
	}
#else
#ifndef	_DISABLE_MORPHING
	float	t = u_fpMorphApplication ;
	if ( t > 0.0 )
	{
		v_vPosition += (vec4( a_vMorphPosition, 1.0 ) - v_vPosition) * t ;
		v_vNormal += (a_vMorphNormal - v_vNormal) * t ;
		//
		#ifdef	_PHONG_SHADER
		#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
			v_vTextureAxisX += (a_vMorphMappingX - v_vTextureAxisX) * t ;
			v_vTextureAxisY += (a_vMorphMappingY - v_vTextureAxisY) * t ;
		#endif
		#endif
		//
		v_vTextureCoord += (a_vMorphTextureCoord - v_vTextureCoord) * t ;
		v_vVertexAddColor += vec4( (a_vMorphAddColor.bgr - v_vVertexAddColor.rgb) * t, 0.0 ) ;
		v_vVertexMulColor.rgb += (a_vMorphMulColor.bgr - v_vVertexMulColor.rgb) * t ;
		//
		if ( u_bMaterialVertexAlpha )
		{
			v_vVertexMulColor.a += (a_vMorphMulColor.a - v_vVertexMulColor.a) * t ;
		}
	}
#endif
#endif
#ifdef	_ENABLE_EXTEND_ATTR_
	v_vExAttrElement0 = getExAttrElement( 0 ) ;
#endif
}


///////////////////////////////////////////////////////////////////////////////
// 頂点座標変換
///////////////////////////////////////////////////////////////////////////////

#ifdef	_ENABLE_VT_INSTANCING

void transform_vertex_bone_at
	( inout highp vec3 vDstPosition, inout mediump vec3 vDstNormal,
		in highp vec3 vSrcPosition, in mediump vec3 vSrcNormal,
		in highp float yiMatrix,
		in highp float iBone, in mediump float fpWeight )
{
	float	xiMatrix = iBone * 3.0 * u_vVertexTextureScale.x ;
	vec4	mv0 = texture2D( u_samplerVertex, vec2( xiMatrix, yiMatrix ) ) ;
	vec4	mv1 = texture2D( u_samplerVertex,
						vec2( xiMatrix + u_vVertexTextureScale.x, yiMatrix ) ) ;
	vec4	mv2 = texture2D( u_samplerVertex,
						vec2( xiMatrix + u_vVertexTextureScale.x * 2.0, yiMatrix ) ) ;
	vec3	m0 = vec3( mv0.x, mv1.x, mv2.x ) ;
	vec3	m1 = vec3( mv0.y, mv1.y, mv2.y ) ;
	vec3	m2 = vec3( mv0.z, mv1.z, mv2.z ) ;
	vec3	m3 = vec3( mv0.w, mv1.w, mv2.w ) ;
	mat3	matBone = mat3( m0, m1, m2 ) ;

	vDstPosition += (matBone * vSrcPosition + m3) * fpWeight ;
	vDstNormal += (matBone * vSrcNormal) * fpWeight ;
}

#else

void transform_vertex_bone_at
	( inout highp vec3 vDstPosition, inout mediump vec3 vDstNormal,
		inout mediump vec3 vDstTexAxisX, inout mediump vec3 vDstTexAxisY,
		in highp vec3 vSrcPosition, in mediump vec3 vSrcNormal,
		in int iBone, in mediump float fpWeight )
{
	vDstPosition +=
		(u_mat3BoneRotation[iBone] * vSrcPosition
							+ u_vBoneTranslate[iBone]) * fpWeight ;
	vDstNormal += (u_mat3BoneRotation[iBone] * vSrcNormal) * fpWeight ;

#ifdef	_PHONG_SHADER
#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
	vDstTexAxisX += (u_mat3BoneRotation[iBone] * v_vTextureAxisX) * fpWeight ;
	vDstTexAxisY += (u_mat3BoneRotation[iBone] * v_vTextureAxisY) * fpWeight ;
#endif
#endif
}

#endif


void transform_vertex
	( out highp vec4 vDstPosition, out mediump vec3 vDstNormal,
		in highp vec3 vSrcPosition, in mediump vec3 vSrcNormal )
{
	highp vec3		vPos = vec3( 0.0, 0.0, 0.0 ) ;
	mediump vec3	vNormal = vec3( 0.0, 0.0, 0.0 ) ;

#ifdef	_ENABLE_VT_INSTANCING
	if ( u_bVertexBoneInstance )
	{
		float	fInstanceID = float( gl_InstanceID ) ;
		float	fVertexID = float( gl_VertexID ) ;
		float	yi = fInstanceID * u_yVertexBoneMatrixStride + u_yVertexBoneMatrix ;
		float	xv = fVertexID * u_vVertexTextureScale.x ;
		float	yv = floor( xv ) ;
		xv -= yv ;
		yv *= u_vVertexTextureScale.y ;
		//
		vec4	vWeight = texture2D( u_samplerVertex,
							vec2( xv, yv + u_yVertexBoneWeight ) ) ;
		vec4	vIndex = texture2D( u_samplerVertex,
							vec2( xv, yv + u_yVertexBoneIndex ) ) ;
		//
		if ( vWeight.x != 0.0 )
		{
			transform_vertex_bone_at
				( vPos, vNormal,
					vSrcPosition, vSrcNormal, yi, vIndex.x, vWeight.x ) ;
		}
		if ( vWeight.y != 0.0 )
		{
			transform_vertex_bone_at
				( vPos, vNormal,
					vSrcPosition, vSrcNormal, yi, vIndex.y, vWeight.y ) ;
		}
		if ( vWeight.z != 0.0 )
		{
			transform_vertex_bone_at
				( vPos, vNormal,
					vSrcPosition, vSrcNormal, yi, vIndex.z, vWeight.z ) ;
		}
		if ( vWeight.w != 0.0 )
		{
			transform_vertex_bone_at
				( vPos, vNormal,
					vSrcPosition, vSrcNormal, yi, vIndex.w, vWeight.w ) ;
		}
	}
	else
	{
		vPos = vSrcPosition ;
		vNormal = vSrcNormal ;
	}

#else
	#ifndef	_DISABLE_BONE_OVER_0
	if ( u_nBoneCount > 0 )
	{
		mediump vec3	vTexAxisX = vec3( 0.0, 0.0, 0.0 ) ;
		mediump vec3	vTexAxisY = vec3( 0.0, 0.0, 0.0 ) ;
		transform_vertex_bone_at
			( vPos, vNormal, vTexAxisX, vTexAxisY,
				vSrcPosition, vSrcNormal, 0, a_vVertexBoneWeight0.x ) ;
		if ( u_nBoneCount > 1 )
		{
			transform_vertex_bone_at
				( vPos, vNormal, vTexAxisX, vTexAxisY,
					vSrcPosition, vSrcNormal, 1, a_vVertexBoneWeight0.y ) ;
			if ( u_nBoneCount > 2 )
			{
				transform_vertex_bone_at
					( vPos, vNormal, vTexAxisX, vTexAxisY,
						vSrcPosition, vSrcNormal, 2, a_vVertexBoneWeight0.z ) ;
				if ( u_nBoneCount > 3 )
				{
					transform_vertex_bone_at
						( vPos, vNormal, vTexAxisX, vTexAxisY,
							vSrcPosition, vSrcNormal, 3, a_vVertexBoneWeight0.w ) ;
				}
			}
		}
		#ifndef	_DISABLE_BONE_OVER_4
		if ( u_nBoneCount > 4 )
		{
			transform_vertex_bone_at
				( vPos, vNormal, vTexAxisX, vTexAxisY,
					vSrcPosition, vSrcNormal, 4, a_vVertexBoneWeight4.x ) ;
			if ( u_nBoneCount > 5 )
			{
				transform_vertex_bone_at
					( vPos, vNormal, vTexAxisX, vTexAxisY,
						vSrcPosition, vSrcNormal, 5, a_vVertexBoneWeight4.y ) ;
				if ( u_nBoneCount > 6 )
				{
					transform_vertex_bone_at
						( vPos, vNormal, vTexAxisX, vTexAxisY,
							vSrcPosition, vSrcNormal, 6, a_vVertexBoneWeight4.z ) ;
					if ( u_nBoneCount > 7 )
					{
						transform_vertex_bone_at
							( vPos, vNormal, vTexAxisX, vTexAxisY,
								vSrcPosition, vSrcNormal, 7, a_vVertexBoneWeight4.w ) ;
					}
				}
			}
		}
		#ifndef _DISABLE_BONE_OVER_8
		if ( u_nBoneCount > 8 )
		{
			transform_vertex_bone_at
				( vPos, vNormal, vTexAxisX, vTexAxisY,
					vSrcPosition, vSrcNormal, 8, a_vVertexBoneWeight8.x ) ;
			if ( u_nBoneCount > 9 )
			{
				transform_vertex_bone_at
					( vPos, vNormal, vTexAxisX, vTexAxisY,
						vSrcPosition, vSrcNormal, 9, a_vVertexBoneWeight8.y ) ;
				if ( u_nBoneCount > 10 )
				{
					transform_vertex_bone_at
						( vPos, vNormal, vTexAxisX, vTexAxisY,
							vSrcPosition, vSrcNormal, 10, a_vVertexBoneWeight8.z ) ;
					if ( u_nBoneCount > 11 )
					{
						transform_vertex_bone_at
							( vPos, vNormal, vTexAxisX, vTexAxisY,
								vSrcPosition, vSrcNormal, 11, a_vVertexBoneWeight8.w ) ;
					}
				}
			}
		}
		#ifndef _DISABLE_BONE_OVER_12
		if ( u_nBoneCount > 12 )
		{
			transform_vertex_bone_at
				( vPos, vNormal, vTexAxisX, vTexAxisY,
					vSrcPosition, vSrcNormal, 12, a_vVertexBoneWeight12.x ) ;
			if ( u_nBoneCount > 13 )
			{
				transform_vertex_bone_at
					( vPos, vNormal, vTexAxisX, vTexAxisY,
						vSrcPosition, vSrcNormal, 13, a_vVertexBoneWeight12.y ) ;
				if ( u_nBoneCount > 14 )
				{
					transform_vertex_bone_at
						( vPos, vNormal, vTexAxisX, vTexAxisY,
							vSrcPosition, vSrcNormal, 14, a_vVertexBoneWeight12.z ) ;
					if ( u_nBoneCount > 15 )
					{
						transform_vertex_bone_at
							( vPos, vNormal, vTexAxisX, vTexAxisY,
								vSrcPosition, vSrcNormal, 15, a_vVertexBoneWeight12.w ) ;
					}
				}
			}
		}
		#endif
		#endif
		#endif
		#ifdef	_PHONG_SHADER
		#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
			v_vTextureAxisX = vTexAxisX ;
			v_vTextureAxisY = vTexAxisY ;
		#endif
		#endif
	}
	else
	{
		vPos = vSrcPosition ;
		vNormal = vSrcNormal ;
	}
	#else
		vPos = vSrcPosition ;
		vNormal = vSrcNormal ;
	#endif
#endif

	#ifdef	_ENABLE_INSTANCING
		vec3	m0 = vec3( a_matInstancingModelView0.x,
							a_matInstancingModelView1.x,
							a_matInstancingModelView2.x ) ;
		vec3	m1 = vec3( a_matInstancingModelView0.y,
							a_matInstancingModelView1.y,
							a_matInstancingModelView2.y ) ;
		vec3	m2 = vec3( a_matInstancingModelView0.z,
							a_matInstancingModelView1.z,
							a_matInstancingModelView2.z ) ;
		vec3	m3 = vec3( a_matInstancingModelView0.w,
							a_matInstancingModelView1.w,
							a_matInstancingModelView2.w ) ;
		mat3	matInstancing = mat3( m0, m1, m2 ) ;
		vPos = matInstancing * vPos + m3 ;
		vDstPosition = u_mat4ModelView * vec4( vPos, 1.0 ) ;
		vDstNormal = normalize( u_mat3ModelViewForNormal
									* matInstancing
									* (vNormal * u_fpInverseNormal) ) ;
		v_vVertexAddColor = vec4( v_vVertexAddColor.rgb
									* a_vInstancingMulColor.bgr
										+ a_vInstancingAddColor.bgr,
									v_vVertexAddColor.a + a_vInstancingAddColor.a ) ;
		v_vVertexMulColor.rgb *= a_vInstancingMulColor.bgr ;
		v_vVertexMulColor.a *= a_vInstancingMulColor.a ;

	#else
		vDstPosition = u_mat4ModelView * vec4( vPos, 1.0 ) ;
		vDstNormal = normalize( u_mat3ModelViewForNormal
									* (vNormal * u_fpInverseNormal) ) ;
	#endif

	if ( u_fpBorderOffset != 0.0 )
	{
		vDstPosition += vec4( vDstNormal * u_fpBorderOffset, 0.0 ) ;
	}

#ifdef	_PHONG_SHADER
#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
	v_vTextureAxisX = u_mat3ModelViewForNormal * v_vTextureAxisX ;
	v_vTextureAxisY = u_mat3ModelViewForNormal * v_vTextureAxisY ;
#endif
#endif
}


///////////////////////////////////////////////////////////////////////////////
// デフォルト頂点シェーダー
///////////////////////////////////////////////////////////////////////////////

vec4 utilDefaultVertex()
{
	morph_vertex() ;
	transform_vertex
		( v_vPosition, v_vNormal, v_vPosition.xyz, v_vNormal ) ;

	return	u_mat4PerspectiveView * v_vPosition ;
}


void utilDefaultVertexGouraud()
{
	mediump vec3	vVertexAddColor = v_vVertexAddColor.rgb ;
	mediump vec3	vVertexMulColor = v_vVertexMulColor.rgb ;
	if ( u_bMaterialShading )
	{
	#ifndef	_DISABLE_SHADING
	#ifdef	_IMPLEMENT_DEFAULT_SHADER
		mediump vec3	vEmissionColor ;
		mediump vec3	vAmbientColor ;
		mediump vec3	vDiffusionColor ;
		mediump vec3	vDstSpecular ;
		mediump vec3	vDstNormal ;
		highp vec3		vRenderPosition ;
		v_vVertexMulColor.a *=
			effect_lighting
				( v_vVertexMulColor.rgb,
					v_vVertexAddColor.rgb, vEmissionColor,
					vAmbientColor, vDiffusionColor,
					vDstSpecular, vDstNormal,
					vRenderPosition,
					v_vPosition.xyz, v_vNormal,
					make_tex_coord( v_vTextureCoord ),
					u_vMaterialMulColor, u_vMaterialAddColor,
					u_vMaterialMulShade, u_vMaterialAddShade,
					vVertexMulColor, vVertexAddColor ) ;
	#endif
	#else
		v_vVertexAddColor.rgb =
			vVertexAddColor
				* (u_vMaterialMulColor + u_vMaterialMulShade)
					+ (u_vMaterialAddColor + u_vMaterialAddShade) ;
		v_vVertexMulColor.rgb =
			vVertexMulColor * (u_vMaterialMulColor + u_vMaterialMulShade) ;
		v_vVertexMulColor.a *= u_fpEffectAlpha ;
	#endif
	}
	else
	{
		v_vVertexAddColor.rgb = vVertexAddColor ;
		v_vVertexMulColor.rgb = vVertexMulColor ;
		v_vVertexMulColor.a *= u_fpEffectAlpha ;
	}
}


void utilDefaultVertexPhong()
{
	v_vVertexMulColor.a *= u_fpEffectAlpha ;
}





