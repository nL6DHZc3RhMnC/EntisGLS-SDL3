
/*****************************************************************************
                          EntisGLS4 Library
 ****************************************************************************/

#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl_opengl_render_context.h>

using namespace SSystem ;
using namespace SakuraGL ;

#include <sakuragl/glsl_src_bin.h>
#include <sakuragl/glsl_src_bin.hpp>


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL カスタムシェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLOpenGLCustomShader, SGLOpenGLDefaultShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLCustomShader::SGLOpenGLCustomShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLDefaultShader( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLCustomShader::~SGLOpenGLCustomShader( void )
{
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLCustomShader::InitializeProgram
	( const S3DRenderDevice::ShaderSourceInfo & features,
		SGLOpenGLShaderProgram::CompileListener * pListener )
{
	if ( !OpenGLExtension::g_supports_opengl_1_3
		|| !OpenGLExtension::g_supports_opengl_1_5
		|| !OpenGLExtension::g_supports_opengl_2_0 )
	{
		return	sglErrNotSupported ;
	}
	ESLAssert( features.typeProgram == S3DRenderDevice::programShader ) ;
	ESLAssert( features.typeSource == S3DRenderDevice::shaderGLSL ) ;
	if ( (features.typeProgram != S3DRenderDevice::programShader)
		|| (features.typeSource != S3DRenderDevice::shaderGLSL) )
	{
		return	sglErrNotSupported ;
	}
	//
	// シェーダープログラム・共通ヘッダ
	//
	SArray<char>	bufCommonHeader ;
	SArray<char>	bufFragHeader ;
	SString			strCommonHeader = L"" ;
	SString			strFragOptHeader = L"" ;
	SString			strFragHeader = L"" ;
	//
	bool			fDisableShading = false ;
	bool			fDisableUVAxisAttr = false ;
	bool			fEnableExAttr = false ;
	//
	m_maxLightCount = MAX_LIGHT_COUNT ;
	m_maxBonePalette = MAX_BONE_PALETTE ;
	m_maxShadowmapping = MAX_SHADOWMAPPING ;
	m_enabledMorphing = true ;
	m_enabledInstancing = false ;
	m_enabledVTInstancing = false ;
	m_preparedDepthBuffer = false ;
	//
	if ( m_pOpenGL->m_flagAvailableMultiSVB
		&& !(features.flagsFeature & S3DRenderDevice::shaderDisableMultiShapedDraw) )
	{
		m_enabledInstancing = true ;
		m_enabledVTInstancing = true ;
	}
	//
	const int		versionGLSL = m_pOpenGL->GetMaxGLSLVersion() ;
	int				versionType = features.versionType ;
	if ( versionType == 0 )
	{
		if ( (features.flagsFeature & S3DRenderDevice::shaderMultiRenderTarget)
			|| (features.flagsFeature & S3DRenderDevice::shaderComputeDepth) )
		{
			versionType = 130 ;
		}
		if ( !(features.flagsFeature & S3DRenderDevice::shaderDisableMultiShapedDraw) )
		{
			versionType = 140 ;
		}
		if ( (features.flagsFeature & S3DRenderDevice::shaderWithGeometry)
			|| (((features.nBaseShader & shadingMethodMask) == shadingMethodPhong)
				&& !(features.flagsFeature & S3DRenderDevice::shaderWithoutStdGeometry)) )
		{
			versionType = 150 ;
		}
	}
	if ( versionType > versionGLSL )
	{
		versionType = versionGLSL ;
	}
	//
	if ( versionType > 100 )
	{
		SString	strMRTCommonHdr, strMRTFragHdr ;
		int	nMRT = esl_min( m_pOpenGL->m_maxDrawBuffers, features.nAvailableMRT ) ;
		if ( (versionType >= 120) && (nMRT < 2) )
		{
			nMRT = esl_min( m_pOpenGL->m_maxDrawBuffers, 2 ) ;
		}
		if ( nMRT >= 2 )
		{
			const wchar_t *	pwszPrecision = L"" ;
			#if	defined(__API_OPEN_GL_ES__)
			pwszPrecision = L"lowp" ;
			#endif
			strMRTCommonHdr.Format
				( L"#define _MULTIPLE_RENDER_TARGET %d\r\n", nMRT ) ;
			strMRTFragHdr.Format
				( L"out %s vec4 fr_FragData[%d] ;\r\n", pwszPrecision, nMRT ) ;
		}
		//
		strCommonHeader = L"#version " ;
		#if	defined(__API_OPEN_GL_ES__)
		if ( versionType >= 130 )
		{
			if ( versionType >= 150 )
			{
				strCommonHeader += L"320 es\r\n" ;
			}
			else
			{
				strCommonHeader += L"300 es\r\n" ;
			}
			if ( features.flagsFeature & S3DRenderDevice::shaderEnableTextureArray )
			{
				strCommonHeader += L"#extension GL_EXT_texture_array : enable\r\n" ;
			}
			strCommonHeader += L"#define _GLSL_VERSION_130_LATER_ 1\r\n" ;
			if ( nMRT >= 2 )
			{
				strFragOptHeader += strMRTCommonHdr ;
				strFragOptHeader += strMRTFragHdr ;
			}
			else
			{
				strMRTCommonHdr = L"#define _MULTIPLE_RENDER_TARGET 1\r\n" ;
				strMRTFragHdr = L"out lowp vec4 fr_FragData[1] ;\r\n" ;
				strFragOptHeader += strMRTCommonHdr ;
				strFragOptHeader += strMRTFragHdr ;
			}
		}
		else
		{
			strCommonHeader += L"100\r\n" ;
		}
		#else
		strCommonHeader += SString( versionType ) ;
		strCommonHeader += L"\r\n" ;
		if ( features.flagsFeature & S3DRenderDevice::shaderEnableTextureArray )
		{
			strCommonHeader += L"#extension GL_EXT_texture_array : enable\r\n" ;
		}
		if ( versionType >= 130 )
		{
			strCommonHeader += L"#define _GLSL_VERSION_130_LATER_ 1\r\n" ;
			if ( nMRT >= 2 )
			{
				strCommonHeader += strMRTCommonHdr ;
				strFragOptHeader += strMRTFragHdr ;
			}
		}
		#endif
		if ( (versionType >= 130)
			&& (features.flagsFeature & S3DRenderDevice::shaderComputeDepth) )
		{
			strCommonHeader += L"#define _ENABLE_COMPUTE_DEPTH_ 1\r\n" ;
		}
	}
	else
	{
		strCommonHeader = L"#version 100\r\n" ;
	}
	//
	if ( features.flagsFeature & S3DRenderDevice::shaderDisableMorphing )
	{
//		m_enabledMorphing = false ;
//		strCommonHeader += L"#define _DISABLE_MORPHING 1\r\n" ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseLuminousTexture) )
	{
		strCommonHeader += L"#define _DISABLE_LUMINOUS_TEXTURE 1\r\n" ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseNormalTexture) )
	{
		strCommonHeader += L"#define _DISABLE_NORMAL_TEXTURE 1\r\n" ;
		strCommonHeader += L"#define _DISABLE_UV_AXIS_ATTRIBUTE 1\r\n" ;
		fDisableUVAxisAttr = true ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseAlphaTexture) )
	{
		strCommonHeader += L"#define _DISABLE_ALPHA_MAPPING 1\r\n" ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseHeightTexture) )
	{
		strCommonHeader += L"#define _DISABLE_HEIGHT_TEXTURE 1\r\n" ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseSpecularTexture) )
	{
		strCommonHeader += L"#define _DISABLE_SPECULAR_TEXTURE 1\r\n" ;
	}
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseGlobalAOTexture) )
	{
		strCommonHeader += L"#define _DISABLE_LIGHTMAP_AO_TEXTURE 1\r\n" ;
	}
	else if ( m_enabledVTInstancing )
	{
		strCommonHeader += L"#define _ENABLE_EXTEND_ATTR_ 1\r\n" ;
		fEnableExAttr = true ;
	}
	if ( m_enabledVTInstancing
		&& (features.flagsFeature & S3DRenderDevice::shaderExAttrElements) )
	{
		if ( !fEnableExAttr )
		{
			strCommonHeader += L"#define _ENABLE_EXTEND_ATTR_ 1\r\n" ;
			fEnableExAttr = true ;
		}
	}
	//
	if ( features.flagsFeature & S3DRenderDevice::shaderLimitBone )
	{
		SetBoneLimitForShaderHeader
			( strCommonHeader, features.nBoneLimit ) ;
	}
	//
	#if	defined(__API_OPEN_GL_ES__)
	if ( features.flagsFeature & S3DRenderDevice::shaderLimitLight )
	{
		if ( m_maxLightCount > features.nLightLimit )
		{
			m_maxLightCount = features.nLightLimit ;
		}
		if ( features.nLightLimit <= 1 )
		{
			strCommonHeader += L"#define _DISABLE_LIGHT_OVER_1 1\r\n" ;
		}
		else if ( features.nLightLimit < 4 )
		{
			strCommonHeader += L"#define _DISABLE_LIGHT_OVER_" ;
			strCommonHeader += SString( features.nLightLimit ) ;
			strCommonHeader += L" 1\r\n" ;
		}
	}
	#endif
	//
	if ( !(features.flagsFeature & S3DRenderDevice::shaderUseStandardShader) )
	{
		strCommonHeader += L"#define _DISABLE_SHADING 1\r\n" ;
		strCommonHeader += L"#define _DISABLE_SHADOWMAPPING 1\r\n" ;
		strCommonHeader += L"#define _LIMIT_SHADOWMAP_COUNT 0\r\n" ;
		m_maxShadowmapping = 0 ;
		fDisableShading = true ;
	}
	else
	{
		if ( !(features.flagsFeature & S3DRenderDevice::shaderUseEnvMapping) )
		{
			strCommonHeader += L"#define _DISABLE_ENVIRONMENT_MAPPING 1\r\n" ;
		}
		else
		{
			if ( !(features.flagsFeature & S3DRenderDevice::shaderUseRefraction) )
			{
				strCommonHeader += L"#define _DISABLE_REFRACTION_MAPPING 1\r\n" ;
			}
			if ( !(features.flagsFeature & S3DRenderDevice::shaderUseCubemap)
				|| !m_pOpenGL->m_flagCubemapTexture )
			{
				strCommonHeader += L"#define _DISABLE_ENVIRONMENT_CUBMAP 1\r\n" ;
			}
			if ( !(features.flagsFeature & S3DRenderDevice::shaderUseEnvSphere) )
			{
				strCommonHeader += L"#define _DISABLE_ENVIRONMENT_SPHERE 1\r\n" ;
			}
			if ( !(features.flagsFeature & S3DRenderDevice::shaderUseRefViewport) )
			{
				strCommonHeader += L"#define _DISABLE_ENVIRONMENT_VIEWPORT 1\r\n" ;
			}
		}
		if ( features.flagsFeature & S3DRenderDevice::shaderLimitShadowmap )
		{
			if ( features.nShadowmapLimit == 0 )
			{
				strCommonHeader += L"#define _DISABLE_SHADOWMAPPING 1\r\n" ;
				m_maxShadowmapping = 0 ;
			}
			if ( features.nShadowmapLimit < m_maxShadowmapping )
			{
				m_maxShadowmapping = features.nShadowmapLimit ;
			}
			strCommonHeader += L"#define _LIMIT_SHADOWMAP_COUNT " ;
			strCommonHeader += SString( m_maxShadowmapping ) ;
			strCommonHeader += L"\r\n" ;
		}
		if ( m_pOpenGL->m_flagSupportedComputeShader )
		{
			strCommonHeader += L"#define _PREPARED_SHADOW_DEPTH_BUFFER 1\r\n" ;
			m_preparedDepthBuffer = true ;
		}
		if ( (features.srcLighting.pszPlaneSrc != nullptr)
			|| (features.srcLighting.pbytEncodedSrc != nullptr) )
		{
			if ( features.nUserLighting & S3DRenderDevice::lightingUserLighting )
			{
				strCommonHeader += L"#define _USER_SUB_LIGHTING 1\r\n" ;
			}
			if ( features.nUserLighting & S3DRenderDevice::lightingUserShading )
			{
				strCommonHeader += L"#define _USER_SUB_SHADING 1\r\n" ;
			}
			if ( features.nUserLighting & S3DRenderDevice::lightingUserFogging )
			{
				strCommonHeader += L"#define _USER_SUB_FOGGING 1\r\n" ;
			}
			if ( features.nUserLighting & S3DRenderDevice::samplingUserDiffusion )
			{
				strCommonHeader += L"#define _USER_SUB_DIFFUSION 1\r\n" ;
			}
			if ( features.nUserLighting & S3DRenderDevice::samplingUserEmission )
			{
				strCommonHeader += L"#define _USER_SUB_EMISSION 1\r\n" ;
			}
			if ( features.nUserLighting & S3DRenderDevice::samplingUserNormal )
			{
				strCommonHeader += L"#define _USER_SUB_NORMAL 1\r\n" ;
			}
		}
	}
	if ( features.flagsFeature & S3DRenderDevice::shaderEnableTextureArray )
	{
		strCommonHeader += L"#define _ENABLE_MATERIAL_TEXTURE_3D 1\r\n"
							L"#define _ENABLE_MATERIAL_TEXTURE_ARRAY 1\r\n" ;
	}
	else if ( features.flagsFeature & S3DRenderDevice::shaderEnableTexture3D )
	{
		strCommonHeader += L"#define _ENABLE_MATERIAL_TEXTURE_3D 1\r\n" ;
	}
	if ( features.flagsFeature & S3DRenderDevice::shaderEnableInstancedDraw )
	{
		m_enabledInstancing = OpenGLExtension::g_supports_instanced_draw ;
	}
	//
	SetAttributeLimitForShaderHeader( strCommonHeader ) ;
	//
	strFragHeader = strCommonHeader + strFragOptHeader ;
	strCommonHeader.EncodeDefaultTo( bufCommonHeader ) ;
	strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
	//
	// シェーダープログラム生成
	//
	Source			srcVertex[16] ;
	Source			srcFragment[16] ;
	Source			srcGeometry[16] ;
	const Source *	psrcGeometry = nullptr ;
	size_t			nVertex = 0 ;
	size_t			nFragment = 0 ;
	size_t			nGeometry = 0 ;
	//
	if ( (features.nBaseShader & shadingMethodMask) == shadingMethodPhong )
	{
		// フォンシェーディング・シェーダーベース
		bool	fWithGeometryShader = false ;
		if ( m_pOpenGL->m_flagSupportedGeometry && (versionType >= 150)
			&& !(features.flagsFeature & S3DRenderDevice::shaderWithoutStdGeometry) )
		{
			if ( !fDisableUVAxisAttr )
			{
				strCommonHeader += L"#define _DISABLE_UV_AXIS_ATTRIBUTE 1\r\n" ;
				fDisableUVAxisAttr = true ;
			}
			strCommonHeader += L"#define _LINK_GEOMETRY_SHADER 1\r\n" ;
			strCommonHeader.EncodeDefaultTo( bufCommonHeader ) ;
			fWithGeometryShader = true ;
		}
		srcVertex[0].pszPlaneSrc = bufCommonHeader.GetConstArray() ;
		srcVertex[0].pbytEncodedSrc = nullptr ;
		srcVertex[0].nEncodedBytes = 0 ;
		srcVertex[1].pszPlaneSrc = nullptr ;
		srcVertex[1].pbytEncodedSrc = &g_glsl_header_phong_h[0] ;
		srcVertex[1].nEncodedBytes = sizeof(g_glsl_header_phong_h) ;
		srcVertex[2].pszPlaneSrc = nullptr ;
		srcVertex[2].pbytEncodedSrc = &g_glsl_header_vert[0] ;
		srcVertex[2].nEncodedBytes = sizeof(g_glsl_header_vert) ;
		srcVertex[3].pszPlaneSrc = nullptr ;
		srcVertex[3].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcVertex[3].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcVertex[4].pszPlaneSrc = nullptr ;
		srcVertex[4].pbytEncodedSrc = &g_glsl_vertex_common_vert[0] ;
		srcVertex[4].nEncodedBytes = sizeof(g_glsl_vertex_common_vert) ;
		if ( features.flagsFeature & S3DRenderDevice::shaderUseStandardShader )
		{
			srcVertex[5].pszPlaneSrc = nullptr ;
			srcVertex[5].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
			srcVertex[5].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
		}
		else
		{
			srcVertex[5].pszPlaneSrc = nullptr ;
			srcVertex[5].pbytEncodedSrc = nullptr ;
		}
		if ( features.srcVertex.pszPlaneSrc
			|| features.srcVertex.pbytEncodedSrc )
		{
			srcVertex[6] = features.srcVertex ;
		}
		else
		{
			srcVertex[6].pszPlaneSrc = nullptr ;
			srcVertex[6].pbytEncodedSrc = &g_glsl_main_phong_vert[0] ;
			srcVertex[6].nEncodedBytes = sizeof(g_glsl_main_phong_vert) ;
		}
		nVertex = 7 ;
		//
		srcFragment[0].pszPlaneSrc = bufFragHeader.GetConstArray() ;
		srcFragment[0].pbytEncodedSrc = nullptr ;
		srcFragment[0].nEncodedBytes = 0 ;
		srcFragment[1].pszPlaneSrc = nullptr ;
		srcFragment[1].pbytEncodedSrc = &g_glsl_header_phong_h[0] ;
		srcFragment[1].nEncodedBytes = sizeof(g_glsl_header_phong_h) ;
		srcFragment[2].pszPlaneSrc = nullptr ;
		srcFragment[2].pbytEncodedSrc = &g_glsl_header_frag[0] ;
		srcFragment[2].nEncodedBytes = sizeof(g_glsl_header_frag) ;
		srcFragment[3].pszPlaneSrc = nullptr ;
		srcFragment[3].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcFragment[3].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcFragment[4].pszPlaneSrc = nullptr ;
		srcFragment[4].pbytEncodedSrc = &g_glsl_texture_mapping_frag[0] ;
		srcFragment[4].nEncodedBytes = sizeof(g_glsl_texture_mapping_frag) ;
		if ( features.flagsFeature & S3DRenderDevice::shaderUseStandardShader )
		{
			srcFragment[5].pszPlaneSrc = nullptr ;
			srcFragment[5].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
			srcFragment[5].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
			srcFragment[6].pszPlaneSrc = nullptr ;
			srcFragment[6].pbytEncodedSrc = &g_glsl_shader_shadowmap_frag[0] ;
			srcFragment[6].nEncodedBytes = sizeof(g_glsl_shader_shadowmap_frag) ;
			srcFragment[7].pszPlaneSrc = nullptr ;
			srcFragment[7].pbytEncodedSrc = nullptr ;
			if ( features.srcLighting.pszPlaneSrc
				|| features.srcLighting.pbytEncodedSrc )
			{
				srcFragment[7] = features.srcLighting ;
			}
			srcFragment[8].pszPlaneSrc = nullptr ;
			srcFragment[8].pbytEncodedSrc = &g_glsl_shader_common_frag[0] ;
			srcFragment[8].nEncodedBytes = sizeof(g_glsl_shader_common_frag) ;
		}
		else
		{
			srcFragment[5].pszPlaneSrc = nullptr ;
			srcFragment[5].pbytEncodedSrc = nullptr ;
			srcFragment[6].pszPlaneSrc = nullptr ;
			srcFragment[6].pbytEncodedSrc = nullptr ;
			srcFragment[7].pszPlaneSrc = nullptr ;
			srcFragment[7].pbytEncodedSrc = nullptr ;
			srcFragment[8].pszPlaneSrc = nullptr ;
			srcFragment[8].pbytEncodedSrc = nullptr ;
		}
		srcFragment[9].pszPlaneSrc = nullptr ;
		srcFragment[9].pbytEncodedSrc = &g_glsl_custom_util_frag[0] ;
		srcFragment[9].nEncodedBytes = sizeof(g_glsl_custom_util_frag) ;
		if ( features.srcFragment.pszPlaneSrc
			|| features.srcFragment.pbytEncodedSrc )
		{
			srcFragment[10] = features.srcFragment ;
		}
		else
		{
			srcFragment[10].pszPlaneSrc = nullptr ;
			srcFragment[10].pbytEncodedSrc = &g_glsl_main_phong_frag[0] ;
			srcFragment[10].nEncodedBytes = sizeof(g_glsl_main_phong_frag) ;
		}
		nFragment = 11 ;
		//
		if ( fWithGeometryShader )
		{
			srcGeometry[0].pszPlaneSrc = bufCommonHeader.GetConstArray() ;
			srcGeometry[0].pbytEncodedSrc = nullptr ;
			srcGeometry[0].nEncodedBytes = 0 ;
			srcGeometry[1].pszPlaneSrc = nullptr ;
			srcGeometry[1].pbytEncodedSrc = &g_glsl_main_phong_geom[0] ;
			srcGeometry[1].nEncodedBytes = sizeof(g_glsl_main_phong_geom) ;
			psrcGeometry = srcGeometry ;
			nGeometry = 2 ;
		}
	}
	else if ( (features.nBaseShader & shadingMethodMask) == shadingMethodGouraud )
	{
		// グーローシェーディング・シェーダー
		if ( (features.flagsFeature & S3DRenderDevice::shaderWithGeometry)
			&& m_pOpenGL->m_flagSupportedGeometry && (versionType >= 150) )
		{
			strCommonHeader += L"#define _LINK_GEOMETRY_SHADER 1\r\n" ;
			strCommonHeader.EncodeDefaultTo( bufCommonHeader ) ;
		}
		srcVertex[0].pszPlaneSrc = bufCommonHeader.GetConstArray() ;
		srcVertex[0].pbytEncodedSrc = nullptr ;
		srcVertex[0].nEncodedBytes = 0 ;
		srcVertex[1].pszPlaneSrc = nullptr ;
		srcVertex[1].pbytEncodedSrc = &g_glsl_header_vert[0] ;
		srcVertex[1].nEncodedBytes = sizeof(g_glsl_header_vert) ;
		srcVertex[2].pszPlaneSrc = nullptr ;
		srcVertex[2].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcVertex[2].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		if ( features.flagsFeature & S3DRenderDevice::shaderUseStandardShader )
		{
			srcVertex[3].pszPlaneSrc = nullptr ;
			srcVertex[3].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
			srcVertex[3].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
			srcVertex[4].pszPlaneSrc = nullptr ;
			srcVertex[4].pbytEncodedSrc = nullptr ;
			if ( features.srcLighting.pszPlaneSrc
				|| features.srcLighting.pbytEncodedSrc )
			{
				srcVertex[4] = features.srcLighting ;
			}
			srcVertex[5].pszPlaneSrc = nullptr ;
			srcVertex[5].pbytEncodedSrc = &g_glsl_shader_common_frag[0] ;
			srcVertex[5].nEncodedBytes = sizeof(g_glsl_shader_common_frag) ;
		}
		else
		{
			srcVertex[3].pszPlaneSrc = nullptr ;
			srcVertex[3].pbytEncodedSrc = nullptr ;
			srcVertex[4].pszPlaneSrc = nullptr ;
			srcVertex[4].pbytEncodedSrc = nullptr ;
			srcVertex[5].pszPlaneSrc = nullptr ;
			srcVertex[5].pbytEncodedSrc = nullptr ;
		}
		srcVertex[6].pszPlaneSrc = nullptr ;
		srcVertex[6].pbytEncodedSrc = &g_glsl_vertex_common_vert[0] ;
		srcVertex[6].nEncodedBytes = sizeof(g_glsl_vertex_common_vert) ;
		if ( features.srcVertex.pszPlaneSrc
			|| features.srcVertex.pbytEncodedSrc )
		{
			srcVertex[7] = features.srcVertex ;
		}
		else
		{
			srcVertex[7].pszPlaneSrc = nullptr ;
			srcVertex[7].pbytEncodedSrc = &g_glsl_main_gouraud_vert[0] ;
			srcVertex[7].nEncodedBytes = sizeof(g_glsl_main_gouraud_vert) ;
		}
		nVertex = 8 ;
		//
		srcFragment[0].pszPlaneSrc = bufFragHeader.GetConstArray() ;
		srcFragment[0].pbytEncodedSrc = nullptr ;
		srcFragment[0].nEncodedBytes = 0 ;
		srcFragment[1].pszPlaneSrc = nullptr ;
		srcFragment[1].pbytEncodedSrc = &g_glsl_header_frag[0] ;
		srcFragment[1].nEncodedBytes = sizeof(g_glsl_header_frag) ;
		srcFragment[2].pszPlaneSrc = nullptr ;
		srcFragment[2].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcFragment[2].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcFragment[3].pszPlaneSrc = nullptr ;
		srcFragment[3].pbytEncodedSrc = &g_glsl_texture_mapping_frag[0] ;
		srcFragment[3].nEncodedBytes = sizeof(g_glsl_texture_mapping_frag) ;
		srcFragment[4].pszPlaneSrc = nullptr ;
		srcFragment[4].pbytEncodedSrc = &g_glsl_custom_util_frag[0] ;
		srcFragment[4].nEncodedBytes = sizeof(g_glsl_custom_util_frag) ;
		if ( features.srcFragment.pszPlaneSrc
			|| features.srcFragment.pbytEncodedSrc )
		{
			srcFragment[5].pszPlaneSrc = nullptr ;
			srcFragment[5].pbytEncodedSrc = nullptr ;
			srcFragment[6].pszPlaneSrc = nullptr ;
			srcFragment[6].pbytEncodedSrc = nullptr ;
			srcFragment[7] = features.srcFragment ;
		}
		else
		{
			srcFragment[5].pszPlaneSrc = nullptr ;
			srcFragment[5].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
			srcFragment[5].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
			srcFragment[6].pszPlaneSrc = nullptr ;
			srcFragment[6].pbytEncodedSrc = &g_glsl_shader_shadowmap_frag[0] ;
			srcFragment[6].nEncodedBytes = sizeof(g_glsl_shader_shadowmap_frag) ;
			srcFragment[7].pszPlaneSrc = nullptr ;
			srcFragment[7].pbytEncodedSrc = &g_glsl_main_gouraud_frag[0] ;
			srcFragment[7].nEncodedBytes = sizeof(g_glsl_main_gouraud_frag) ;
		}
		nFragment = 8 ;
	}
	else
	{
		if ( (features.flagsFeature & S3DRenderDevice::shaderWithGeometry)
			&& m_pOpenGL->m_flagSupportedGeometry && (versionType >= 150) )
		{
			strCommonHeader += L"#define _LINK_GEOMETRY_SHADER 1\r\n" ;
			strCommonHeader.EncodeDefaultTo( bufCommonHeader ) ;
		}
		if ( features.flagsFeature & S3DRenderDevice::shaderUseStandardShader )
		{
			if ( !fDisableShading )
			{
				strCommonHeader += L"#define _DISABLE_SHADING 1\r\n" ;
				fDisableShading = true ;
			}
			strFragHeader = strCommonHeader + strFragOptHeader ;
			strCommonHeader.EncodeDefaultTo( bufCommonHeader ) ;
			strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
		}
		srcVertex[0].pszPlaneSrc = bufCommonHeader.GetConstArray() ;
		srcVertex[0].pbytEncodedSrc = nullptr ;
		srcVertex[0].nEncodedBytes = 0 ;
		srcVertex[1].pszPlaneSrc = nullptr ;
		srcVertex[1].pbytEncodedSrc = &g_glsl_header_vert[0] ;
		srcVertex[1].nEncodedBytes = sizeof(g_glsl_header_vert) ;
		srcVertex[2].pszPlaneSrc = nullptr ;
		srcVertex[2].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcVertex[2].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcVertex[3].pszPlaneSrc = nullptr ;
		srcVertex[3].pbytEncodedSrc = &g_glsl_vertex_common_vert[0] ;
		srcVertex[3].nEncodedBytes = sizeof(g_glsl_vertex_common_vert) ;
		if ( features.srcVertex.pszPlaneSrc
			|| features.srcVertex.pbytEncodedSrc )
		{
			srcVertex[4] = features.srcVertex ;
		}
		else
		{
			srcVertex[4].pszPlaneSrc = nullptr ;
			srcVertex[4].pbytEncodedSrc = &g_glsl_main_gouraud_vert[0] ;
			srcVertex[4].nEncodedBytes = sizeof(g_glsl_main_gouraud_vert) ;
		}
		nVertex = 5 ;
		//
		srcFragment[0].pszPlaneSrc = bufFragHeader.GetConstArray() ;
		srcFragment[0].pbytEncodedSrc = nullptr ;
		srcFragment[0].nEncodedBytes = 0 ;
		srcFragment[1].pszPlaneSrc = nullptr ;
		srcFragment[1].pbytEncodedSrc = &g_glsl_header_frag[0] ;
		srcFragment[1].nEncodedBytes = sizeof(g_glsl_header_frag) ;
		srcFragment[2].pszPlaneSrc = nullptr ;
		srcFragment[2].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcFragment[2].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcFragment[3].pszPlaneSrc = nullptr ;
		srcFragment[3].pbytEncodedSrc = &g_glsl_texture_mapping_frag[0] ;
		srcFragment[3].nEncodedBytes = sizeof(g_glsl_texture_mapping_frag) ;
		srcFragment[4].pszPlaneSrc = nullptr ;
		srcFragment[4].pbytEncodedSrc = &g_glsl_custom_util_frag[0] ;
		srcFragment[4].nEncodedBytes = sizeof(g_glsl_custom_util_frag) ;
		if ( features.srcFragment.pszPlaneSrc
			|| features.srcFragment.pbytEncodedSrc )
		{
			srcFragment[5] = features.srcFragment ;
			nFragment = 6 ;
		}
		else
		{
			srcFragment[5].pszPlaneSrc = nullptr ;
			srcFragment[5].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
			srcFragment[5].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
			srcFragment[6].pszPlaneSrc = nullptr ;
			srcFragment[6].pbytEncodedSrc = &g_glsl_shader_shadowmap_frag[0] ;
			srcFragment[6].nEncodedBytes = sizeof(g_glsl_shader_shadowmap_frag) ;
			srcFragment[7].pszPlaneSrc = nullptr ;
			srcFragment[7].pbytEncodedSrc = &g_glsl_main_gouraud_frag[0] ;
			srcFragment[7].nEncodedBytes = sizeof(g_glsl_main_gouraud_frag) ;
			nFragment = 8 ;
		}
	}
	if ( features.flagsFeature & S3DRenderDevice::shaderWithGeometry )
	{
		if ( m_pOpenGL->m_flagSupportedGeometry && (versionType >= 150) )
		{
			srcGeometry[0].pszPlaneSrc = bufCommonHeader.GetConstArray() ;
			srcGeometry[0].pbytEncodedSrc = nullptr ;
			srcGeometry[0].nEncodedBytes = 0 ;
			srcGeometry[1] = features.srcGeometry ;
			psrcGeometry = srcGeometry ;
			nGeometry = 2 ;
		}
	}
	SGLError	err =
		CreateProgram
			( srcVertex, nVertex,
				psrcGeometry, nGeometry,
				srcFragment, nFragment, pListener ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// シェーダー属性／ユニフォーム位置取得
	//
	GetShaderVariableLocations() ;
	OnInitCustomUniform() ;
	//
	return	sglErrSuccess ;
}

// シェーダープログラム復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLCustomShader::LoadProgramBinary
	( const S3DShaderBinary& bin,
		SGLOpenGLShaderProgram::CompileListener * pListener )
{
	SGLError	err =
		SGLOpenGLDefaultShader::LoadProgramBinary( bin, pListener ) ;
	if ( err )
	{
		return	err ;
	}
	OnInitCustomUniform() ;
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// GLSL Compute Shader
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLComputeShader,
		SGLOpenGLShaderProgram, S3DComputeShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLComputeShader::SGLOpenGLComputeShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLShaderProgram( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLComputeShader::~SGLOpenGLComputeShader( void )
{
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLComputeShader::InitializeProgram
	( const S3DRenderDevice::ShaderSourceInfo & features,
		SGLOpenGLShaderProgram::CompileListener * pListener )
{
	if ( !OpenGLExtension::g_supports_compute_shader )
	{
		return	sglErrNotSupported ;
	}
	ESLAssert( features.typeProgram == S3DRenderDevice::programCompute ) ;
	ESLAssert( features.typeSource == S3DRenderDevice::shaderGLSL ) ;
	if ( (features.typeProgram != S3DRenderDevice::programCompute)
		|| (features.typeSource != S3DRenderDevice::shaderGLSL) )
	{
		return	sglErrNotSupported ;
	}
	//
	// ヘッダ生成
	//
	SArray<char>	bufComputeHeader ;
	SString			strVerionHeader = L"#version " ;
#if	defined(__API_OPEN_GL_ES__)
	if ( features.versionType >= 420 )
	{
		strVerionHeader += L"320 es\r\n" ;
	}
	else
	{
		strVerionHeader += L"310 es\r\n" ;
	}
	strVerionHeader += L"precision highp float;\r\n" ;
#else
	strVerionHeader += L"430 core\r\n" ;
#endif
	SString			strLocalSizeHeader ;
	strLocalSizeHeader.Format
		( L"layout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\r\n",
			features.dimLocalSize.x, features.dimLocalSize.y, features.dimLocalSize.z ) ;
	//
	SString	strHeader = strVerionHeader + strLocalSizeHeader ;
	strHeader.EncodeDefaultTo( bufComputeHeader ) ;
	//
	// プログラム生成
	//
	Source	srcCompute[2] ;
	srcCompute[0].pszPlaneSrc = bufComputeHeader.GetConstArray() ;
	srcCompute[1] = features.srcCompute ;
	//
	SGLError	err =
		CreateCumputeShader
			( srcCompute, 2, features.dimLocalSize, pListener ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ユニフォーム位置取得
	//
	OnInitCustomUniform() ;
	//
	return	sglErrSuccess ;
}

// シェーダープログラム保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLComputeShader::SaveProgramBinary( S3DShaderBinary& bin )
{
	SGLError	err = SGLOpenGLShaderProgram::SaveProgramBinary( bin ) ;
	if ( err )
	{
		return	err ;
	}
	bin.SetAttrIntegerAs( L"local_size_x", m_dimComputeLocalSize.x ) ;
	bin.SetAttrIntegerAs( L"local_size_y", m_dimComputeLocalSize.y ) ;
	bin.SetAttrIntegerAs( L"local_size_z", m_dimComputeLocalSize.z ) ;
	return	sglErrSuccess ;
}

// シェーダープログラム復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLComputeShader::LoadProgramBinary
	( const S3DShaderBinary& bin,
		SGLOpenGLShaderProgram::CompileListener * pListener )
{
	SGLError	err =
		SGLOpenGLShaderProgram::LoadProgramBinary( bin, pListener ) ;
	if ( err )
	{
		return	err ;
	}
	m_dimComputeLocalSize.x =
		(size_t) bin.GetAttrIntegerAs( L"local_size_x", m_dimComputeLocalSize.x ) ;
	m_dimComputeLocalSize.y =
		(size_t) bin.GetAttrIntegerAs( L"local_size_y", m_dimComputeLocalSize.y ) ;
	m_dimComputeLocalSize.z =
		(size_t) bin.GetAttrIntegerAs( L"local_size_z", m_dimComputeLocalSize.z ) ;
	//
	OnInitCustomUniform() ;
	//
	return	sglErrSuccess ;
}

// ローカルサイズ取得
//////////////////////////////////////////////////////////////////////////////
S3DComputeShaderInterface::DimSize
	SGLOpenGLComputeShader::GetWorkLocalSize( void ) const
{
	return	m_dimComputeLocalSize ;
}

// 実行（レンダリングスレッド上で）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLComputeShader::Execute
	( S3DRenderDevice * pDevice,
		const S3DComputeShaderInterface::ExecuteParam& param )
{
	if ( !OpenGLExtension::g_supports_compute_shader )
	{
		return	sglErrNotSupported ;
	}

	SSmartLock<const SCriticalSection>	lock( GetUniformMutex() ) ;

	SGLOpenGLShaderProgram *	pglSaveShader = m_pOpenGL->GetCurrentShaderProgram() ;
	m_pOpenGL->AttachShaderProgram( this ) ;

	if ( param.pUniforms != nullptr )
	{
		SetCustomUniformSet( *(param.pUniforms) ) ;
	}
	glDispatchCompute
		( (GLuint) param.dimWorkGroups.x,
			(GLuint) param.dimWorkGroups.y,
			(GLuint) param.dimWorkGroups.z ) ;
	if ( !SGLOpenGLContext::VerifyError( "glDispatchCompute" ) )
	{
		m_pOpenGL->AttachShaderProgram( pglSaveShader ) ;
		return	sglErrFailed ;
	}
	GLbitfield	bitsBarriers = 0 ;
	if ( param.nBarrierFlags & barrierImage )
	{
		bitsBarriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT ;
	}
	if ( param.nBarrierFlags & barrierVertex )
	{
		bitsBarriers |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT ;
	}
	if ( param.nBarrierFlags & barrierElement )
	{
		bitsBarriers |= GL_ELEMENT_ARRAY_BARRIER_BIT ;
	}
	if ( param.nBarrierFlags & barrierUniform )
	{
		bitsBarriers |= GL_UNIFORM_BARRIER_BIT ;
	}
	if ( param.nBarrierFlags & barrierTexture )
	{
		bitsBarriers |= GL_TEXTURE_FETCH_BARRIER_BIT ;
	}
	if ( bitsBarriers != 0 )
	{
		glMemoryBarrier( bitsBarriers ) ;
		SGLOpenGLContext::VerifyError( "glMemoryBarrier" ) ;
	}
	m_pOpenGL->AttachShaderProgram( pglSaveShader ) ;
	return	sglErrSuccess ;
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLComputeShader::SetPerspectiveMatrix( const S4DMatrix& mat4 )
{
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLComputeShader::SetProjectionScreen
	( float32_t xScreen, float32_t yScreen, float32_t zScreen )
{
}

// モデル変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLComputeShader::SetModelViewMatrix
		( const S4DMatrix& mat4, bool fInverseNormal )
{
}



//////////////////////////////////////////////////////////////////////////
// 三角ポリゴン→ワイヤーフレーム描画シェーダ―
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLSimpleTriangle2LineShader, SGLOpenGLCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleTriangle2LineShader::SGLOpenGLSimpleTriangle2LineShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleTriangle2LineShader::~SGLOpenGLSimpleTriangle2LineShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleTriangle2LineShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderUseStandardShader
						| S3DRenderDevice::shaderLimitBone
						| S3DRenderDevice::shaderLimitLight
						| S3DRenderDevice::shaderWithGeometry ;
	source.nBoneLimit = 16 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcGeometry.pszPlaneSrc = nullptr ;
	source.srcGeometry.pbytEncodedSrc = &g_glsl_main_triangle2line_geom[0] ;
	source.srcGeometry.nEncodedBytes = sizeof(g_glsl_main_triangle2line_geom) ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_simple_fill_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_simple_fill_frag) ;
}



//////////////////////////////////////////////////////////////////////////////
// UV→空間座標／三角ポリゴン→ワイヤーフレーム描画シェーダ―
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLSimpleTriangle2UVLineShader, SGLOpenGLCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleTriangle2UVLineShader::SGLOpenGLSimpleTriangle2UVLineShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleTriangle2UVLineShader::~SGLOpenGLSimpleTriangle2UVLineShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleTriangle2UVLineShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderUseStandardShader
						| S3DRenderDevice::shaderLimitBone
						| S3DRenderDevice::shaderLimitLight
						| S3DRenderDevice::shaderWithGeometry ;
	source.nBoneLimit = 16 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = &g_glsl_main_uv_vertex_vert[0] ;
	source.srcVertex.nEncodedBytes = sizeof(g_glsl_main_uv_vertex_vert) ;
	source.srcGeometry.pszPlaneSrc = nullptr ;
	source.srcGeometry.pbytEncodedSrc = &g_glsl_main_triangle2line_geom[0] ;
	source.srcGeometry.nEncodedBytes = sizeof(g_glsl_main_triangle2line_geom) ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_simple_fill_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_simple_fill_frag) ;
}



//////////////////////////////////////////////////////////////////////////////
// DrawWithDepth シェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLDrawWithDepthShader,
		SGLOpenGLCustomShader, S3DDrawWithDepthShaderInterface )

	// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDrawWithDepthShader::SGLOpenGLDrawWithDepthShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		u_samplerDepth( -1 ), u_mat4SamplePers( -1 ),
		m_samplerDepth( nullptr ), m_mat4SamplePers( 1, 1, 1, 1 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDrawWithDepthShader::~SGLOpenGLDrawWithDepthShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_draw_wz_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_draw_wz_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_samplerDepth", S3DCustomShader::uniformTexture, 1	},
		{	L"u_mat4SamplePers", S3DCustomShader::uniformMatrix4x4, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_samplerDepth = FindCustomUniform( L"u_samplerDepth" ) ;
	ESLAssert( u_samplerDepth >= 0 ) ;
	//
	u_mat4SamplePers = FindCustomUniform( L"u_mat4SamplePers" ) ;
	ESLAssert( u_mat4SamplePers >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformTexture( L"u_samplerDepth", m_samplerDepth ) ;
	render.SetCustomShaderUniformMatrix4x4( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataTextureAs( L"u_samplerDepth", m_samplerDepth ) ;
	unis.SetDataMatrix4x4As( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::SetDepthBuffer( SGLImageObject * pDepthBuf )
{
	ESLAssert( pDepthBuf != nullptr ) ;
	m_samplerDepth = pDepthBuf ;
	SetCustomUniformImage( u_samplerDepth, m_samplerDepth ) ;
}

// ｚバッファ透視変換行列
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDrawWithDepthShader::SetPerspective( const S4DMatrix& matPers )
{
	m_mat4SamplePers = matPers ;
	SetCustomUniformMatrix4x4( u_mat4SamplePers, &m_mat4SamplePers, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL ガウスぼかしシェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLOpenGLGaussianBlurShader::SHADER_ID = L"GAUSSIAN_BLUR_1X9" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLGaussianBlurShader,
		SGLOpenGLCustomShader, S3DGaussianBlurShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLGaussianBlurShader::SGLOpenGLGaussianBlurShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_vSampleDirection( 1, 0 )
{
	u_fpGaussianWeight = -1 ;
	u_vSamplingDirection = -1 ;
	m_fpGauss = 1.0 ;
	m_fpBrightness = 1.0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLGaussianBlurShader::~SGLOpenGLGaussianBlurShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::GetSourceInfo( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_gaussian_blur_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_gaussian_blur_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_fpGaussianWeight", S3DCustomShader::uniformFloat, WeightCount	},
		{	L"u_vSamplingDirection", S3DCustomShader::uniformVector2D, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_fpGaussianWeight = FindCustomUniform( L"u_fpGaussianWeight" ) ;
	ESLAssert( u_fpGaussianWeight >= 0 ) ;
	//
	u_vSamplingDirection = FindCustomUniform( L"u_vSamplingDirection" ) ;
	ESLAssert( u_vSamplingDirection >= 0 ) ;
	//
	SetGauss( m_fpGauss ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	render.SetCustomShaderUniformFloat( L"u_fpGaussianWeight", fpGauss, WeightCount ) ;
	render.SetCustomShaderUniformVector2D( L"u_vSamplingDirection", &m_vSampleDirection, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	unis.SetDataFloatAs( L"u_fpGaussianWeight", fpGauss, WeightCount ) ;
	unis.SetDataVector2DAs( L"u_vSamplingDirection", &m_vSampleDirection, 1 ) ;
}

// ガウス係数計算
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::CalcGauss( float32_t fpGauss[] ) const
{
	float32_t	g = (float32_t) m_fpGauss ;
	float32_t	sum = 0.0f ;
	float32_t	c = (float32_t) (g * g) ;
	for ( int i = 0; i < WeightCount; i ++ )
	{
		float32_t	x = (float32_t) (1.0f + i * 2.0f) ;
		float32_t	w = (float32_t) exp( -0.5 * (x * x) / c ) ;
		fpGauss[i] = w ;
		sum += w ;
		if ( i > 0 )
		{
			sum += w ;
		}
	}
	if ( sum <= 0.0 )
	{
		fpGauss[0] = 1.0f ;
	}
	else
	{
		float32_t	r = (float32_t) (m_fpBrightness / sum) ;
		for ( int i = 0; i < WeightCount; i ++ )
		{
			fpGauss[i] *= r ;
		}
	}
}

// ガウス係数設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::SetGauss( double g )
{
	m_fpGauss = g ;

	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	SetCustomUniformFloat
		( (size_t) u_fpGaussianWeight, fpGauss, WeightCount ) ;
}

// サンプリング方向設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::SetDirection( double x, double y )
{
	S2DVector	vDir( x, y ) ;
	SetCustomUniformVector2D
		( (size_t) u_vSamplingDirection, &vDir, 1 ) ;
	//
	m_vSampleDirection = vDir ;
}

// 輝度設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianBlurShader::SetBrightness( double b )
{
	if ( m_fpBrightness != b )
	{
		m_fpBrightness = b ;
		SetGauss( m_fpGauss ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL ガウス放射状ブラーシェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLOpenGLGaussianRadialBlurShader::SHADER_ID = L"GAUSSIAN_RADIAL_BLUR_X9" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SGLOpenGLGaussianRadialBlurShader,
		SGLOpenGLCustomShader, S3DGaussianRadialBlurShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLGaussianRadialBlurShader::SGLOpenGLGaussianRadialBlurShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_vRadialCenter( 0, 0 )
{
	u_fpGaussianWeight = -1 ;
	u_vRadialCenter = -1 ;
	u_fpSamplingUnit = -1 ;
	m_fpGauss = 1.0 ;
	m_fpBrightness = 1.0 ;
	m_fpSamplingUnit = 1.0f ;
	m_fpBlurScale = 1.0f ;
	m_fpBlurPower = 0.0f ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLGaussianRadialBlurShader::~SGLOpenGLGaussianRadialBlurShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_gaussian_radial_blur_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_gaussian_radial_blur_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_fpGaussianWeight", S3DCustomShader::uniformFloat, WeightCount	},
		{	L"u_vRadialCenter", S3DCustomShader::uniformVector2D, 1	},
		{	L"u_fpSamplingUnit", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpBlurScale", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpBlurPower", S3DCustomShader::uniformFloat, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_fpGaussianWeight = FindCustomUniform( L"u_fpGaussianWeight" ) ;
	ESLAssert( u_fpGaussianWeight >= 0 ) ;
	//
	u_vRadialCenter = FindCustomUniform( L"u_vRadialCenter" ) ;
	ESLAssert( u_vRadialCenter >= 0 ) ;
	//
	u_fpSamplingUnit = FindCustomUniform( L"u_fpSamplingUnit" ) ;
	ESLAssert( u_fpSamplingUnit >= 0 ) ;
	//
	u_fpBlurScale = FindCustomUniform( L"u_fpBlurScale" ) ;
	ESLAssert( u_fpBlurScale >= 0 ) ;
	//
	u_fpBlurPower = FindCustomUniform( L"u_fpBlurPower" ) ;
	ESLAssert( u_fpBlurPower >= 0 ) ;
	//
	SetGauss( m_fpGauss ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	render.SetCustomShaderUniformFloat( L"u_fpGaussianWeight", fpGauss, WeightCount ) ;
	render.SetCustomShaderUniformVector2D( L"u_vRadialCenter", &m_vRadialCenter, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpBlurScale", &m_fpBlurScale, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpBlurPower", &m_fpBlurPower, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	unis.SetDataFloatAs( L"u_fpGaussianWeight", fpGauss, WeightCount ) ;
	unis.SetDataVector2DAs( L"u_vRadialCenter", &m_vRadialCenter, 1 ) ;
	unis.SetDataFloatAs( L"u_fpBlurScale", &m_fpBlurScale, 1 ) ;
	unis.SetDataFloatAs( L"u_fpBlurPower", &m_fpBlurPower, 1 ) ;
}

// ガウス係数計算
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::CalcGauss( float32_t fpGauss[] ) const
{
	float32_t	g = (float32_t) m_fpGauss ;
	float32_t	sum = 0.0f ;
	float32_t	c = (GLfloat) (g * g) ;
	for ( int i = 0; i < WeightCount; i ++ )
	{
		GLfloat	x = (GLfloat) (1.0f + i * 2.0f) ;
		GLfloat	w = (GLfloat) exp( -0.5 * (x * x) / c ) ;
		fpGauss[i] = w ;
		sum += w ;
		if ( i > 0 )
		{
			sum += w ;
		}
	}
	if ( sum <= 0.0 )
	{
		fpGauss[0] = 1.0f ;
	}
	else
	{
		float32_t	r = (float32_t) (m_fpBrightness / sum) ;
		for ( int i = 0; i < WeightCount; i ++ )
		{
			fpGauss[i] *= r ;
		}
	}
}

// ガウス係数設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetGauss( double g )
{
	m_fpGauss = g ;

	float32_t	fpGauss[WeightCount] ;
	CalcGauss( fpGauss ) ;

	SetCustomUniformFloat
		( (size_t) u_fpGaussianWeight, fpGauss, WeightCount ) ;
}

// 放射中心設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetCenter( double x, double y )
{
	S2DVector	vCenter( x, y ) ;
	SetCustomUniformVector2D
		( (size_t) u_vRadialCenter, &vCenter, 1 ) ;
	//
	m_vRadialCenter = vCenter ;
}

// 輝度設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetBrightness( double b )
{
	if ( m_fpBrightness != b )
	{
		m_fpBrightness = b ;
		SetGauss( m_fpGauss ) ;
	}
}

// サンプリング単位距離設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetSamplingUnit( double d )
{
	m_fpSamplingUnit = (float32_t) d ;
	SetCustomUniformFloat( u_fpSamplingUnit, &m_fpSamplingUnit, 1 ) ;
}

// ブラースケール : k = (r / x) ^ p,  r : 放射中心からの変位
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLGaussianRadialBlurShader::SetBlueScale( double x, double p )
{
	m_fpBlurScale = (float32_t) (1.0 / x) ;
	SetCustomUniformFloat( u_fpBlurScale, &m_fpBlurScale, 1 ) ;
	//
	m_fpBlurPower = (float32_t) p ;
	SetCustomUniformFloat( u_fpBlurPower, &m_fpBlurPower, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL 被写界深度シェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLOpenGLDepthBlenderShader::SHADER_ID = L"DEPTH_BLENDER" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLDepthBlenderShader,
		SGLOpenGLCustomShader, S3DDepthBlenderShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDepthBlenderShader::SGLOpenGLDepthBlenderShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_pDepthBuf( nullptr ), m_vDepthScale( 1, 1 ),
		m_fpFocusDepth( 1.0f ),
		m_fpFocusNearRange( 10.0f ), m_fpFocusFarRange( 0.01f ),
		m_fpPersM22( 1.0f ), m_fpPersM23( 1.0f )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDepthBlenderShader::~SGLOpenGLDepthBlenderShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::GetSourceInfo( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_depth_blender_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_depth_blender_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_samplerDepth", S3DCustomShader::uniformTexture, 1	},
		{	L"u_vDepthTextureScale", S3DCustomShader::uniformVector2D, 1	},
		{	L"u_fpFocusDepth", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpFocusNearRange", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpFocusFarRange", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpPersM22", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpPersM23", S3DCustomShader::uniformFloat, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_samplerDepth = FindCustomUniform( L"u_samplerDepth" ) ;
	ESLAssert( u_samplerDepth >= 0 ) ;
	//
	u_vDepthTextureScale = FindCustomUniform( L"u_vDepthTextureScale" ) ;
	ESLAssert( u_vDepthTextureScale >= 0 ) ;
	//
	u_fpFocusDepth = FindCustomUniform( L"u_fpFocusDepth" ) ;
	ESLAssert( u_fpFocusDepth >= 0 ) ;
	//
	u_fpFocusNearRange = FindCustomUniform( L"u_fpFocusNearRange" ) ;
	ESLAssert( u_fpFocusNearRange >= 0 ) ;
	//
	u_fpFocusFarRange = FindCustomUniform( L"u_fpFocusFarRange" ) ;
	ESLAssert( u_fpFocusFarRange >= 0 ) ;
	//
	u_fpPersM22 = FindCustomUniform( L"u_fpPersM22" ) ;
	ESLAssert( u_fpPersM22 >= 0 ) ;
	//
	u_fpPersM23 = FindCustomUniform( L"u_fpPersM23" ) ;
	ESLAssert( u_fpPersM23 >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformTexture( L"u_samplerDepth", m_pDepthBuf ) ;
	render.SetCustomShaderUniformVector2D( L"u_vDepthTextureScale", &m_vDepthScale, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpFocusDepth", &m_fpFocusDepth, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpFocusNearRange", &m_fpFocusNearRange, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpFocusFarRange", &m_fpFocusFarRange, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpPersM22", &m_fpPersM22, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpPersM23", &m_fpPersM23, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataTextureAs( L"u_samplerDepth", m_pDepthBuf ) ;
	unis.SetDataVector2DAs( L"u_vDepthTextureScale", &m_vDepthScale, 1 ) ;
	unis.SetDataFloatAs( L"u_fpFocusDepth", &m_fpFocusDepth, 1 ) ;
	unis.SetDataFloatAs( L"u_fpFocusNearRange", &m_fpFocusNearRange, 1 ) ;
	unis.SetDataFloatAs( L"u_fpFocusFarRange", &m_fpFocusFarRange, 1 ) ;
	unis.SetDataFloatAs( L"u_fpPersM22", &m_fpPersM22, 1 ) ;
	unis.SetDataFloatAs( L"u_fpPersM23", &m_fpPersM23, 1 ) ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetDepthBuffer( SGLImageObject * pDepthBuf )
{
	ESLAssert( pDepthBuf != nullptr ) ;
	SetCustomUniformImage( u_samplerDepth, pDepthBuf ) ;
	//
	SGLSize		sizeBuf = pDepthBuf->GetImageSize() ;
	S2DVector	vScale( 1.0f / (float32_t) sizeBuf.w,
						1.0f / (float32_t) sizeBuf.h ) ;
	SetCustomUniformVector2D( u_vDepthTextureScale, &vScale, 1 ) ;
	//
	m_pDepthBuf = pDepthBuf ;
	m_vDepthScale = vScale ;
}

// 焦点深度値設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetFocusDepth( double zFocus )
{
	m_fpFocusDepth = (float32_t) zFocus ;
	SetCustomUniformFloat( u_fpFocusDepth, &m_fpFocusDepth, 1 ) ;
}

// ぼかし深度幅設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetDepthRange( double zNearRange, double zFarRange )
{
	m_fpFocusNearRange = 1.0f / (float32_t) zNearRange ;
	m_fpFocusFarRange = 1.0f / (float32_t) zFarRange ;
	SetCustomUniformFloat( u_fpFocusNearRange, &m_fpFocusNearRange, 1 ) ;
	SetCustomUniformFloat( u_fpFocusFarRange, &m_fpFocusFarRange, 1 ) ;
}

// ｚ値変換用係数設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDepthBlenderShader::SetPersParameter( double m22, double m23 )
{
	m_fpPersM22 = (float32_t) m22 ;
	m_fpPersM23 = (float32_t) m23 ;
	SetCustomUniformFloat( u_fpPersM22, &m_fpPersM22, 1 ) ;
	SetCustomUniformFloat( u_fpPersM23, &m_fpPersM23, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL 遅延シェーダー光源
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLDelayLightShader,
		SGLOpenGLCustomShader, S3DDelayLightShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDelayLightShader::SGLOpenGLDelayLightShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_samplerDepth( nullptr ),
		m_samplerNormal( nullptr ),
		m_samplerSpecular( nullptr ),
		m_mat4SamplePers( 1, 1, 1, 1 ),
		m_fpDiffusion( 1.0f ), m_fpSpecular( 0.0f ),
		m_fpAirScattering( 0.0f ), m_fpAirRcpUnit( 0.0f ),
		m_typeDLighting( 0 ),
		m_vDLPosition( 0, 0, 0 ),
		m_vDLDirection( 0, 1, 0 ),
		m_vDLColor( 1, 1, 1 ),
		m_fpDLBrightness( 1.0f ),
		m_fpDLAttenuationPower( 1.0f ),
		m_fpDLAngle( 1.0f ), m_fpDLGradation( 1.0f )
{
	u_samplerDepth = -1 ;
	u_samplerNormal = -1 ;
	u_samplerSpecular = -1 ;
	u_mat4SamplePers = -1 ;
	u_fpDiffusion = -1 ;
	u_fpSpecular = -1 ;
	u_fpAirScattering = -1 ;
	u_fpAirRcpUnit = -1 ;
	u_typeDLighting = -1 ;
	u_vDLPosition = -1 ;
	u_vDLDirection = -1 ;
	u_vDLColor = -1 ;
	u_fpDLBrightness = -1 ;
	u_fpDLAngle = -1 ;
	u_fpDLGradation = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDelayLightShader::~SGLOpenGLDelayLightShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderMultiRenderTarget
							| S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight
							| S3DRenderDevice::shaderLimitShadowmap ;
	source.nAvailableMRT = 2 ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.nShadowmapLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_delay_point_light_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_delay_point_light_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_samplerDepth", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerNormal", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerSpecular", S3DCustomShader::uniformTexture, 1	},
		{	L"u_mat4SamplePers", S3DCustomShader::uniformMatrix4x4, 1	},
		{	L"u_fpDiffusion", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpSpecular", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpAirScattering", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpAirRcpUnit", S3DCustomShader::uniformFloat, 1	},
		{	L"u_typeDLighting", S3DCustomShader::uniformInt, 1	},
		{	L"u_vDLPosition", S3DCustomShader::uniformVector3D, 1	},
		{	L"u_vDLDirection", S3DCustomShader::uniformVector3D, 1	},
		{	L"u_vDLColor", S3DCustomShader::uniformVector3D, 1	},
		{	L"u_fpDLBrightness", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpDLAttenuationPower", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpDLAngle", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpDLGradation", S3DCustomShader::uniformFloat, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_samplerDepth = FindCustomUniform( L"u_samplerDepth" ) ;
	ESLAssert( u_samplerDepth >= 0 ) ;
	//
	u_samplerNormal = FindCustomUniform( L"u_samplerNormal" ) ;
	ESLAssert( u_samplerNormal >= 0 ) ;
	//
	u_samplerSpecular = FindCustomUniform( L"u_samplerSpecular" ) ;
	ESLAssert( u_samplerSpecular >= 0 ) ;
	//
	u_mat4SamplePers = FindCustomUniform( L"u_mat4SamplePers" ) ;
	ESLAssert( u_mat4SamplePers >= 0 ) ;
	//
	u_fpDiffusion = FindCustomUniform( L"u_fpDiffusion" ) ;
	ESLAssert( u_fpDiffusion >= 0 ) ;
	//
	u_fpSpecular = FindCustomUniform( L"u_fpSpecular" ) ;
	ESLAssert( u_fpSpecular >= 0 ) ;
	//
	u_fpAirScattering = FindCustomUniform( L"u_fpAirScattering" ) ;
	ESLAssert( u_fpAirScattering >= 0 ) ;
	//
	u_fpAirRcpUnit = FindCustomUniform( L"u_fpAirRcpUnit" ) ;
	ESLAssert( u_fpAirRcpUnit >= 0 ) ;
	//
	u_typeDLighting = FindCustomUniform( L"u_typeDLighting" ) ;
	ESLAssert( u_typeDLighting >= 0 ) ;
	//
	u_vDLPosition = FindCustomUniform( L"u_vDLPosition" ) ;
	ESLAssert( u_vDLPosition >= 0 ) ;
	//
	u_vDLDirection = FindCustomUniform( L"u_vDLDirection" ) ;
	ESLAssert( u_vDLDirection >= 0 ) ;
	//
	u_vDLColor = FindCustomUniform( L"u_vDLColor" ) ;
	ESLAssert( u_vDLColor >= 0 ) ;
	//
	u_fpDLBrightness = FindCustomUniform( L"u_fpDLBrightness" ) ;
	ESLAssert( u_fpDLBrightness >= 0 ) ;
	//
	u_fpDLAttenuationPower = FindCustomUniform( L"u_fpDLAttenuationPower" ) ;
	ESLAssert( u_fpDLAttenuationPower >= 0 ) ;
	//
	u_fpDLAngle = FindCustomUniform( L"u_fpDLAngle" ) ;
	ESLAssert( u_fpDLAngle >= 0 ) ;
	//
	u_fpDLGradation = FindCustomUniform( L"u_fpDLGradation" ) ;
	ESLAssert( u_fpDLGradation >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformTexture( L"u_samplerDepth", m_samplerDepth ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerNormal", m_samplerNormal ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerSpecular", m_samplerSpecular ) ;
	render.SetCustomShaderUniformMatrix4x4( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDiffusion", &m_fpDiffusion, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpSpecular", &m_fpSpecular, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpAirScattering", &m_fpAirScattering, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpAirRcpUnit", &m_fpAirRcpUnit, 1 ) ;
	render.SetCustomShaderUniformInt( L"u_typeDLighting", &m_typeDLighting, 1 ) ;
	render.SetCustomShaderUniformVector3D( L"u_vDLPosition", &m_vDLPosition, 1 ) ;
	render.SetCustomShaderUniformVector3D( L"u_vDLDirection", &m_vDLDirection, 1 ) ;
	render.SetCustomShaderUniformVector3D( L"u_vDLColor", &m_vDLColor, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDLBrightness", &m_fpDLBrightness, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDLAttenuationPower", &m_fpDLAttenuationPower, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDLAngle", &m_fpDLAngle, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDLGradation", &m_fpDLGradation, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataTextureAs( L"u_samplerDepth", m_samplerDepth ) ;
	unis.SetDataTextureAs( L"u_samplerNormal", m_samplerNormal ) ;
	unis.SetDataTextureAs( L"u_samplerSpecular", m_samplerSpecular ) ;
	unis.SetDataMatrix4x4As( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDiffusion", &m_fpDiffusion, 1 ) ;
	unis.SetDataFloatAs( L"u_fpSpecular", &m_fpSpecular, 1 ) ;
	unis.SetDataFloatAs( L"u_fpAirScattering", &m_fpAirScattering, 1 ) ;
	unis.SetDataFloatAs( L"u_fpAirRcpUnit", &m_fpAirRcpUnit, 1 ) ;
	unis.SetDataIntAs( L"u_typeDLighting", &m_typeDLighting, 1 ) ;
	unis.SetDataVector3DAs( L"u_vDLPosition", &m_vDLPosition, 1 ) ;
	unis.SetDataVector3DAs( L"u_vDLDirection", &m_vDLDirection, 1 ) ;
	unis.SetDataVector3DAs( L"u_vDLColor", &m_vDLColor, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDLBrightness", &m_fpDLBrightness, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDLAttenuationPower", &m_fpDLAttenuationPower, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDLAngle", &m_fpDLAngle, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDLGradation", &m_fpDLGradation, 1 ) ;
}

// 光源情報設定（座標は透視空間）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetLightParam( const S3DLightEntry & light )
{
	m_typeDLighting = light.typeLight & lightTypeMask ;
	SetCustomUniformInt( u_typeDLighting, &m_typeDLighting, 1 ) ;
	//
	m_vDLColor.x = (float32_t) light.rgbColor.argb.Red / 255.0f ;
	m_vDLColor.y = (float32_t) light.rgbColor.argb.Green / 255.0f ;
	m_vDLColor.z = (float32_t) light.rgbColor.argb.Blue / 255.0f ;
	SetCustomUniformVector3D( u_vDLColor, &m_vDLColor, 1 ) ;
	//
	m_fpDLBrightness = light.fpBrightness ;
	SetCustomUniformFloat( u_fpDLBrightness, &m_fpDLBrightness, 1 ) ;
	//
	m_fpDLAttenuationPower = light.fpAttenuationPower ;
	SetCustomUniformFloat( u_fpDLAttenuationPower, &m_fpDLAttenuationPower, 1 ) ;
	//
	m_vDLPosition = light.vecPosition ;
	SetCustomUniformVector3D( u_vDLPosition, &m_vDLPosition, 1 ) ;
	//
	m_vDLDirection = light.vecDirection ;
	m_vDLDirection.Normalize() ;
	SetCustomUniformVector3D( u_vDLDirection, &m_vDLDirection, 1 ) ;
	//
	m_fpDLAngle = light.fpAngle + light.fpGradation ;
	SetCustomUniformFloat( u_fpDLAngle, &m_fpDLAngle, 1 ) ;
	//
	m_fpDLGradation = light.fpGradation ;
	SetCustomUniformFloat( u_fpDLGradation, &m_fpDLGradation, 1 ) ;
}

// 光源描画効果
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetLightApplication( float32_t fpDiffusion, float32_t fpSpecular )
{
	m_fpDiffusion = fpDiffusion ;
	m_fpSpecular = fpSpecular ;
	SetCustomUniformFloat( u_fpDiffusion, &m_fpDiffusion, 1 ) ;
	SetCustomUniformFloat( u_fpSpecular, &m_fpSpecular, 1 ) ;
}

// 大気散乱効果
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetAirScattering
			( float32_t fpScattering, float32_t fpDistanceUnit )
{
	m_fpAirScattering = fpScattering ;
	m_fpAirRcpUnit = 1.0f / fpDistanceUnit ;
	SetCustomUniformFloat( u_fpAirScattering, &m_fpAirScattering, 1 ) ;
	SetCustomUniformFloat( u_fpAirRcpUnit, &m_fpAirRcpUnit, 1 ) ;
}

// 透視変換行列
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetPerspective( const S4DMatrix& matPers )
{
	m_mat4SamplePers = matPers ;
	SetCustomUniformMatrix4x4( u_mat4SamplePers, &m_mat4SamplePers, 1 ) ;
}

// レンダリング済みバッファ
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDelayLightShader::SetSourceBuffer
	( SGLImageObject*const* ppColorBufs,
			size_t nColorBufCount, SGLImageObject * pDepthBuf )
{
	ESLAssert( pDepthBuf != nullptr ) ;
	m_samplerDepth = pDepthBuf ;
	m_samplerNormal = nullptr ;
	m_samplerSpecular = nullptr ;
	if ( nColorBufCount > S3DRenderDevice::renderTargetNormal )
	{
		m_samplerNormal = ppColorBufs[S3DRenderDevice::renderTargetNormal] ;
	}
	if ( nColorBufCount > S3DRenderDevice::renderTargetSpecular )
	{
		m_samplerSpecular = ppColorBufs[S3DRenderDevice::renderTargetSpecular] ;
	}
	//
	SetCustomUniformImage( u_samplerDepth, m_samplerDepth ) ;
	SetCustomUniformImage( u_samplerNormal, m_samplerNormal ) ;
	SetCustomUniformImage( u_samplerSpecular, m_samplerSpecular ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL 画面空間大域照明
//////////////////////////////////////////////////////////////////////////////

// SSGISampling クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLSSGISamplingShader,
		SGLOpenGLCustomShader, S3DSSGISamplerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSSGISamplingShader::SGLOpenGLSSGISamplingShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_samplerDepth( nullptr ),
		m_samplerNormal( nullptr ),
		m_samplerEmission( nullptr ),
		m_samplerDiffusion( nullptr ),
		m_samplerSpecular( nullptr ),
		m_mat4SamplePers( 1, 1, 1, 1 ),
		m_fpReachAO( 1.0f ), m_fpReachAObyZ( 1.0f ),
		m_nAOSamplingCount( 8 ), m_nGISamplingCount( 8 ),
		m_fpDiffusionLuminousness( 0.0f ),
		m_bWith3WayMapping( 0 ),
		m_sampler3WayComposed( nullptr ),
		m_sampler3WayEmission( nullptr ),
		m_sampler3WayDepth( nullptr ),
		m_mat4Sample3WayPers( 1, 1, 1, 1 )
{
	u_samplerDepth = -1 ;
	u_samplerNormal = -1 ;
	u_samplerEmission = -1 ;
	u_samplerDiffusion = -1 ;
	u_samplerSpecular = -1 ;
	u_mat4SamplePers = -1 ;
	u_fpReachAO = -1 ;
	u_fpReachAObyZ = -1 ;
	u_nAOSamplingCount = -1 ;
	u_nGISamplingCount = -1 ;
	u_fpDiffusionLuminousness = -1 ;
	u_bWith3WayMapping = -1 ;
	u_sampler3WayComposed = -1 ;
	u_sampler3WayEmission = -1 ;
	u_sampler3WayDepth = -1 ;
	u_mat4Sample3WayPers = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSSGISamplingShader::~SGLOpenGLSSGISamplingShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight
							| S3DRenderDevice::shaderLimitShadowmap ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.nShadowmapLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_ssgi_sampling_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_ssgi_sampling_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_samplerDepth", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerNormal", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerEmission", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerDiffusion", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerSpecular", S3DCustomShader::uniformTexture, 1	},
		{	L"u_mat4SamplePers", S3DCustomShader::uniformMatrix4x4, 1	},
		{	L"u_fpReachAO", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpReachAObyZ", S3DCustomShader::uniformFloat, 1	},
		{	L"u_nAOSamplingCount", S3DCustomShader::uniformInt, 1	},
		{	L"u_nGISamplingCount", S3DCustomShader::uniformInt, 1	},
		{	L"u_fpDiffusionLuminousness", S3DCustomShader::uniformFloat, 1	},
		{	L"u_bWith3WayMapping", S3DCustomShader::uniformInt, 1	},
		{	L"u_sampler3WayComposed", S3DCustomShader::uniformTexture, 1	},
		{	L"u_sampler3WayEmission", S3DCustomShader::uniformTexture, 1	},
		{	L"u_sampler3WayDepth", S3DCustomShader::uniformTexture, 1	},
		{	L"u_mat4Sample3WayPers", S3DCustomShader::uniformMatrix4x4, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_samplerDepth = FindCustomUniform( L"u_samplerDepth" ) ;
	ESLAssert( u_samplerDepth >= 0 ) ;
	//
	u_samplerNormal = FindCustomUniform( L"u_samplerNormal" ) ;
	ESLAssert( u_samplerNormal >= 0 ) ;
	//
	u_samplerEmission = FindCustomUniform( L"u_samplerEmission" ) ;
	ESLAssert( u_samplerEmission >= 0 ) ;
	//
	u_samplerDiffusion = FindCustomUniform( L"u_samplerDiffusion" ) ;
	ESLAssert( u_samplerDiffusion >= 0 ) ;
	//
	u_samplerSpecular = FindCustomUniform( L"u_samplerSpecular" ) ;
	ESLAssert( u_samplerSpecular >= 0 ) ;
	//
	u_mat4SamplePers = FindCustomUniform( L"u_mat4SamplePers" ) ;
	ESLAssert( u_mat4SamplePers >= 0 ) ;
	//
	u_fpReachAO = FindCustomUniform( L"u_fpReachAO" ) ;
	ESLAssert( u_fpReachAO >= 0 ) ;
	//
	u_fpReachAObyZ = FindCustomUniform( L"u_fpReachAObyZ" ) ;
	ESLAssert( u_fpReachAObyZ >= 0 ) ;
	//
	u_nAOSamplingCount = FindCustomUniform( L"u_nAOSamplingCount" ) ;
	ESLAssert( u_nAOSamplingCount >= 0 ) ;
	//
	u_nGISamplingCount = FindCustomUniform( L"u_nGISamplingCount" ) ;
	ESLAssert( u_nGISamplingCount >= 0 ) ;
	//
	u_fpDiffusionLuminousness = FindCustomUniform( L"u_fpDiffusionLuminousness" ) ;
	ESLAssert( u_fpDiffusionLuminousness >= 0 ) ;
	//
	u_bWith3WayMapping = FindCustomUniform( L"u_bWith3WayMapping" ) ;
	ESLAssert( u_bWith3WayMapping >= 0 ) ;
	//
	u_sampler3WayComposed = FindCustomUniform( L"u_sampler3WayComposed" ) ;
	ESLAssert( u_sampler3WayComposed >= 0 ) ;
	//
	u_sampler3WayEmission = FindCustomUniform( L"u_sampler3WayEmission" ) ;
	ESLAssert( u_sampler3WayEmission >= 0 ) ;
	//
	u_sampler3WayDepth = FindCustomUniform( L"u_sampler3WayDepth" ) ;
	ESLAssert( u_sampler3WayDepth >= 0 ) ;
	//
	u_mat4Sample3WayPers = FindCustomUniform( L"u_mat4Sample3WayPers" ) ;
	ESLAssert( u_mat4Sample3WayPers >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformTexture( L"u_samplerDepth", m_samplerDepth ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerNormal", m_samplerNormal ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerEmission", m_samplerEmission ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerDiffusion", m_samplerDiffusion ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerSpecular", m_samplerSpecular ) ;
	render.SetCustomShaderUniformMatrix4x4( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpReachAO", &m_fpReachAO, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpReachAObyZ", &m_fpReachAObyZ, 1 ) ;
	render.SetCustomShaderUniformInt( L"u_nAOSamplingCount", &m_nAOSamplingCount, 1 ) ;
	render.SetCustomShaderUniformInt( L"u_nGISamplingCount", &m_nGISamplingCount, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpDiffusionLuminousness", &m_fpDiffusionLuminousness, 1 ) ;
	render.SetCustomShaderUniformInt( L"u_bWith3WayMapping", &m_bWith3WayMapping, 1 ) ;
	render.SetCustomShaderUniformTexture( L"u_sampler3WayComposed", m_sampler3WayComposed ) ;
	render.SetCustomShaderUniformTexture( L"u_sampler3WayDepth", m_sampler3WayDepth ) ;
	render.SetCustomShaderUniformTexture( L"u_sampler3WayEmission", m_sampler3WayEmission ) ;
	render.SetCustomShaderUniformMatrix4x4( L"u_mat4Sample3WayPers", &m_mat4Sample3WayPers, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataTextureAs( L"u_samplerDepth", m_samplerDepth ) ;
	unis.SetDataTextureAs( L"u_samplerNormal", m_samplerNormal ) ;
	unis.SetDataTextureAs( L"u_samplerEmission", m_samplerEmission ) ;
	unis.SetDataTextureAs( L"u_samplerDiffusion", m_samplerDiffusion ) ;
	unis.SetDataTextureAs( L"u_samplerSpecular", m_samplerSpecular ) ;
	unis.SetDataMatrix4x4As( L"u_mat4SamplePers", &m_mat4SamplePers, 1 ) ;
	unis.SetDataFloatAs( L"u_fpReachAO", &m_fpReachAO, 1 ) ;
	unis.SetDataFloatAs( L"u_fpReachAObyZ", &m_fpReachAObyZ, 1 ) ;
	unis.SetDataIntAs( L"u_nAOSamplingCount", &m_nAOSamplingCount, 1 ) ;
	unis.SetDataIntAs( L"u_nGISamplingCount", &m_nGISamplingCount, 1 ) ;
	unis.SetDataFloatAs( L"u_fpDiffusionLuminousness", &m_fpDiffusionLuminousness, 1 ) ;
	unis.SetDataIntAs( L"u_bWith3WayMapping", &m_bWith3WayMapping, 1 ) ;
	unis.SetDataTextureAs( L"u_sampler3WayComposed", m_sampler3WayComposed ) ;
	unis.SetDataTextureAs( L"u_sampler3WayDepth", m_sampler3WayDepth ) ;
	unis.SetDataTextureAs( L"u_sampler3WayEmission", m_sampler3WayEmission ) ;
	unis.SetDataMatrix4x4As( L"u_mat4Sample3WayPers", &m_mat4Sample3WayPers, 1 ) ;
}

// AO 到達距離
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetAOReachDistance
			( float32_t fpReach, float32_t fpZProportion )
{
	m_fpReachAO = fpReach ;
	m_fpReachAObyZ = fpZProportion ;
	SetCustomUniformFloat( u_fpReachAO, &m_fpReachAO, 1 ) ;
	SetCustomUniformFloat( u_fpReachAObyZ, &m_fpReachAObyZ, 1 ) ;
}

// AO サンプリング数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetAOSamplingCount( size_t nCount )
{
	m_nAOSamplingCount = (int32_t) nCount ;
	SetCustomUniformInt( u_nAOSamplingCount, &m_nAOSamplingCount, 1 ) ;
}

// GI サンプリング数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetGISamplingCount( size_t nCount )
{
	m_nGISamplingCount = (int32_t) nCount ;
	SetCustomUniformInt( u_nGISamplingCount, &m_nGISamplingCount, 1 ) ;
}

// GI 輝度
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetGILuminousness( float32_t fpDiffusion )
{
	m_fpDiffusionLuminousness = fpDiffusion ;
	SetCustomUniformFloat( u_fpDiffusionLuminousness, &m_fpDiffusionLuminousness, 1 ) ;
}

// 透視変換行列
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetPerspective( const S4DMatrix& matPers )
{
	m_mat4SamplePers = matPers ;
	SetCustomUniformMatrix4x4( u_mat4SamplePers, &m_mat4SamplePers, 1 ) ;
}

// レンダリング済みバッファ
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::SetSourceBuffer
	( SGLImageObject*const* ppColorBufs,
		size_t nColorBufCount, SGLImageObject * pDepthBuf )
{
	ESLAssert( pDepthBuf != nullptr ) ;
	m_samplerDepth = pDepthBuf ;
	m_samplerNormal = nullptr ;
	m_samplerEmission = nullptr ;
	m_samplerDiffusion = nullptr ;
	m_samplerSpecular = nullptr ;

	if ( nColorBufCount > S3DRenderDevice::renderTargetNormal )
	{
		m_samplerNormal = ppColorBufs[S3DRenderDevice::renderTargetNormal] ;
	}
	if ( nColorBufCount > S3DRenderDevice::renderTargetEmission )
	{
		m_samplerEmission = ppColorBufs[S3DRenderDevice::renderTargetEmission] ;
	}
	if ( nColorBufCount > S3DRenderDevice::renderTargetDiffusion )
	{
		m_samplerDiffusion = ppColorBufs[S3DRenderDevice::renderTargetDiffusion] ;
	}
	if ( nColorBufCount > S3DRenderDevice::renderTargetSpecular )
	{
		m_samplerSpecular = ppColorBufs[S3DRenderDevice::renderTargetSpecular] ;
	}

	SetCustomUniformImage( u_samplerDepth, m_samplerDepth ) ;
	SetCustomUniformImage( u_samplerNormal, m_samplerNormal ) ;
	SetCustomUniformImage( u_samplerEmission, m_samplerEmission ) ;
	SetCustomUniformImage( u_samplerDiffusion, m_samplerDiffusion ) ;
	SetCustomUniformImage( u_samplerSpecular, m_samplerSpecular ) ;
}

// 3面パノラマフレーム有効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::Enable3WayPanoramaBuffer( bool fEnable )
{
	m_bWith3WayMapping = fEnable ;
	SetCustomUniformInt( u_bWith3WayMapping, &m_bWith3WayMapping, 1 ) ;
}

// 3面パノラマ透視変換行列
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::Set3WayPerspective( const S4DMatrix& matPers )
{
	m_mat4Sample3WayPers = matPers ;
	SetCustomUniformMatrix4x4( u_mat4Sample3WayPers, &m_mat4Sample3WayPers, 1 ) ;
}

// レンダリング済み3面パノラマバッファ
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGISamplingShader::Set3WayPanoramaBuffer
	( SGLImageObject*const* ppColorBufs,
		size_t nColorBufCount, SGLImageObject * pDepthBuf )
{
	ESLAssert( ppColorBufs != nullptr ) ;
	ESLAssert( pDepthBuf != nullptr ) ;
	m_sampler3WayComposed = ppColorBufs[S3DRenderDevice::renderTargetComposed] ;
	m_sampler3WayEmission = nullptr ;
	m_sampler3WayDepth = pDepthBuf ;

	if ( nColorBufCount > S3DRenderDevice::renderTargetEmission )
	{
		m_sampler3WayEmission = ppColorBufs[S3DRenderDevice::renderTargetEmission] ;
	}

	SetCustomUniformImage( u_sampler3WayComposed, m_sampler3WayComposed ) ;
	SetCustomUniformImage( u_sampler3WayDepth, m_sampler3WayDepth ) ;
	SetCustomUniformImage( u_sampler3WayEmission, m_sampler3WayEmission ) ;
}


// SSGIComposer クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLSSGIComposerShader,
		SGLOpenGLCustomShader, S3DSSGIComposerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSSGIComposerShader::SGLOpenGLSSGIComposerShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_samplerComposed( nullptr ),
		m_samplerAmbient( nullptr ),
		m_vSamplingScale( 1, 1  ),
		m_fpBlendAO( 0.5f ),
		m_fpBlendGI( 1.0f ),
		m_rgbAOShadeColor( 1, 1, 1  )
{
	u_samplerComposed = -1 ;
	u_samplerAmbient = -1 ;
	u_vSamplingScale = -1 ;
	u_fpBlendAO = -1 ;
	u_fpBlendGI = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSSGIComposerShader::~SGLOpenGLSSGIComposerShader( void )
{
}

// ShaderSourceInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderMultiRenderTarget
							| S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight
							| S3DRenderDevice::shaderLimitShadowmap ;
	source.nAvailableMRT = 2 ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.nShadowmapLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_ssgi_compose_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_ssgi_compose_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_samplerComposed", S3DCustomShader::uniformTexture, 1	},
		{	L"u_samplerAmbient", S3DCustomShader::uniformTexture, 1	},
		{	L"u_vSamplingScale", S3DCustomShader::uniformVector2D, 1	},
		{	L"u_fpBlendAO", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpBlendGI", S3DCustomShader::uniformFloat, 1	},
		{	L"u_rgbAOShadeColor", S3DCustomShader::uniformVector3D, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_samplerComposed = FindCustomUniform( L"u_samplerComposed" ) ;
	ESLAssert( u_samplerComposed >= 0 ) ;
	//
	u_samplerAmbient = FindCustomUniform( L"u_samplerAmbient" ) ;
	ESLAssert( u_samplerAmbient >= 0 ) ;
	//
	u_vSamplingScale = FindCustomUniform( L"u_vSamplingScale" ) ;
	ESLAssert( u_vSamplingScale >= 0 ) ;
	//
	u_fpBlendAO = FindCustomUniform( L"u_fpBlendAO" ) ;
	ESLAssert( u_fpBlendAO >= 0 ) ;
	//
	u_fpBlendGI = FindCustomUniform( L"u_fpBlendGI" ) ;
	ESLAssert( u_fpBlendGI >= 0 ) ;
	//
	u_rgbAOShadeColor = FindCustomUniform( L"u_rgbAOShadeColor" ) ;
	ESLAssert( u_rgbAOShadeColor >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformTexture( L"u_samplerComposed", m_samplerComposed ) ;
	render.SetCustomShaderUniformTexture( L"u_samplerAmbient", m_samplerAmbient ) ;
	render.SetCustomShaderUniformVector2D( L"u_vSamplingScale", &m_vSamplingScale, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpBlendAO", &m_fpBlendAO, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpBlendGI", &m_fpBlendGI, 1 ) ;
	render.SetCustomShaderUniformVector3D( L"u_rgbAOShadeColor", &m_rgbAOShadeColor, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataTextureAs( L"u_samplerComposed", m_samplerComposed ) ;
	unis.SetDataTextureAs( L"u_samplerAmbient", m_samplerAmbient ) ;
	unis.SetDataVector2DAs( L"u_vSamplingScale", &m_vSamplingScale, 1 ) ;
	unis.SetDataFloatAs( L"u_fpBlendAO", &m_fpBlendAO, 1 ) ;
	unis.SetDataFloatAs( L"u_fpBlendGI", &m_fpBlendGI, 1 ) ;
	unis.SetDataVector3DAs( L"u_rgbAOShadeColor", &m_rgbAOShadeColor, 1 ) ;
}

// サンプリングスケール
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetSamplingScale( const S2DVector& vScale )
{
	m_vSamplingScale = vScale ;
	SetCustomUniformVector2D( u_vSamplingScale, &m_vSamplingScale, 1 ) ;
}

// AO/GI 適用度
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetBlendRatio( float32_t fpAO, float32_t fpGI )
{
	m_fpBlendAO = fpAO ;
	m_fpBlendGI = fpGI ;
	SetCustomUniformFloat( u_fpBlendAO, &m_fpBlendAO, 1 ) ;
	SetCustomUniformFloat( u_fpBlendGI, &m_fpBlendGI, 1 ) ;
}

// AO 影加算色
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetAOShadeColor( const SGLPalette& rgbShade )
{
	S3DVector	vColor ;
	vColor.x = (float32_t) rgbShade.argb.Red * (1.0f / 255.0f) ;
	vColor.y = (float32_t) rgbShade.argb.Green * (1.0f / 255.0f) ;
	vColor.z = (float32_t) rgbShade.argb.Blue * (1.0f / 255.0f) ;
	//
	m_rgbAOShadeColor = vColor ;
	SetCustomUniformVector3D( u_rgbAOShadeColor, &m_rgbAOShadeColor, 1 ) ;
}

// レンダリング済みバッファ
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSSGIComposerShader::SetSourceBuffer
	( SGLImageObject*const* ppColorBufs, size_t nColorBufCount )
{
	m_samplerComposed = ppColorBufs[S3DRenderDevice::renderTargetComposed] ;
	m_samplerAmbient = nullptr ;
	if ( nColorBufCount > S3DRenderDevice::renderTargetAmbient )
	{
		m_samplerAmbient = ppColorBufs[S3DRenderDevice::renderTargetAmbient] ;
	}

	SetCustomUniformImage( u_samplerComposed, m_samplerComposed ) ;
	SetCustomUniformImage( u_samplerAmbient, m_samplerAmbient ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL モザイク描画シェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLSimpleMosaicShader, SGLOpenGLCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleMosaicShader::SGLOpenGLSimpleMosaicShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_vMosaicSize( 0.05f, 0.05f )
{
	u_vMosaicSize = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleMosaicShader::~SGLOpenGLSimpleMosaicShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleMosaicShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = shadingMethodPhong ;
	source.flagsFeature = S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight
							| S3DRenderDevice::shaderLimitShadowmap
							| S3DRenderDevice::shaderUseEnvMapping
							| S3DRenderDevice::shaderUseRefraction
							| S3DRenderDevice::shaderUseRefViewport ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 1 ;
	source.nShadowmapLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_mosaic_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_mosaic_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleMosaicShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_vMosaicSize", S3DCustomShader::uniformVector2D, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_vMosaicSize = FindCustomUniform( L"u_vMosaicSize" ) ;
	ESLAssert( u_vMosaicSize >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleMosaicShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformVector2D( L"u_vMosaicSize", &m_vMosaicSize, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleMosaicShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataVector2DAs( L"u_vMosaicSize", &m_vMosaicSize, 1 ) ;
}

// モザイクサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleMosaicShader::SetMosaicSize( double x, double y )
{
	S2DVector	vSize( x, y ) ;
	m_vMosaicSize = vSize ;
	SetCustomUniformVector2D( u_vMosaicSize, &m_vMosaicSize, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL 水面描画シェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLSimpleWaterShader,
		SGLOpenGLCustomShader, S3DSimpleWaterShaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleWaterShader::SGLOpenGLSimpleWaterShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL ),
		m_fpAmplitude( 1.0f ),
		m_fpNormalAmp( 1.0f ),
		m_vAmplitude( 0, 0, 0, 0 ),
		m_vBumpAmplitude( 0, 0, 0, 0 ),
		m_vFrequency( 1, 1, 1, 1 ),
		m_vBumpFrequency( 1, 1, 1, 1 ),
		m_vTime( 0, 0, 0, 0 ),
		m_vBumpTime( 0, 0, 0, 0 ),
		m_vWaterAxisX( 1, 0, 0 ),
		m_vWaterAxisY( 0, 0, 1 ),
		m_fpCascadeFarZ( 300.0f ),
		m_fpCascadePhase1( 0.05f ),
		m_fpCascadeAmplitude1( 0.5f )
{
	u_vAmplitude = -1 ;
	u_vBumpAmplitude = -1 ;
	u_vFrequency = -1 ;
	u_vBumpFrequency = -1 ;
	u_vTime = -1 ;
	u_vBumpTime = -1 ;
	u_vDirection = -1 ;
	u_vBumpDirection = -1 ;
	u_vWaterAxisX = -1 ;
	u_vWaterAxisY = -1 ;
	u_fpCascadeFarZ = -1 ;
	u_fpCascadePhase1 = -1 ;
	u_fpCascadeAmplitude1 = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLSimpleWaterShader::~SGLOpenGLSimpleWaterShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	GetDefaultShaderFeatures( source ) ;
	//
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = shadingMethodPhong ;
	source.flagsFeature |= S3DRenderDevice::shaderWithoutStdGeometry
							| S3DRenderDevice::shaderMultiRenderTarget
							| S3DRenderDevice::shaderUseRefViewport
							| S3DRenderDevice::shaderUseRefraction ;
	source.nAvailableMRT = 4 ;
	source.nBoneLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = &g_glsl_main_simple_water_vert[0] ;
	source.srcVertex.nEncodedBytes = sizeof(g_glsl_main_simple_water_vert) ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_simple_water_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_simple_water_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_fpAmplitude", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpNormalAmp", S3DCustomShader::uniformFloat, 1	},
		{	L"u_vAmplitude", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vBumpAmplitude", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vFrequency", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vBumpFrequency", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vTime", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vBumpTime", S3DCustomShader::uniformVector4D, 1	},
		{	L"u_vDirection", S3DCustomShader::uniformVector2D, 4	},
		{	L"u_vBumpDirection", S3DCustomShader::uniformVector2D, 4	},
		{	L"u_vWaterAxisX", S3DCustomShader::uniformVector3D, 1	},
		{	L"u_vWaterAxisY", S3DCustomShader::uniformVector3D, 1	},
		{	L"u_fpCascadeFarZ", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpCascadePhase1", S3DCustomShader::uniformFloat, 1	},
		{	L"u_fpCascadeAmplitude1", S3DCustomShader::uniformFloat, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_fpAmplitude = FindCustomUniform( L"u_fpAmplitude" ) ;
	ESLAssert( u_fpAmplitude >= 0 ) ;
	//
	u_fpNormalAmp = FindCustomUniform( L"u_fpNormalAmp" ) ;
	ESLAssert( u_fpNormalAmp >= 0 ) ;
	//
	u_vAmplitude = FindCustomUniform( L"u_vAmplitude" ) ;
	ESLAssert( u_vAmplitude >= 0 ) ;
	//
	u_vBumpAmplitude = FindCustomUniform( L"u_vBumpAmplitude" ) ;
	ESLAssert( u_vBumpAmplitude >= 0 ) ;
	//
	u_vFrequency = FindCustomUniform( L"u_vFrequency" ) ;
	ESLAssert( u_vFrequency >= 0 ) ;
	//
	u_vBumpFrequency = FindCustomUniform( L"u_vBumpFrequency" ) ;
	ESLAssert( u_vBumpFrequency >= 0 ) ;
	//
	u_vTime = FindCustomUniform( L"u_vTime" ) ;
	ESLAssert( u_vTime >= 0 ) ;
	//
	u_vBumpTime = FindCustomUniform( L"u_vBumpTime" ) ;
	ESLAssert( u_vBumpTime >= 0 ) ;
	//
	u_vDirection = FindCustomUniform( L"u_vDirection" ) ;
	ESLAssert( u_vDirection >= 0 ) ;
	//
	u_vBumpDirection = FindCustomUniform( L"u_vBumpDirection" ) ;
	ESLAssert( u_vBumpDirection >= 0 ) ;
	//
	u_vWaterAxisX = FindCustomUniform( L"u_vWaterAxisX" ) ;
	ESLAssert( u_vWaterAxisX >= 0 ) ;
	//
	u_vWaterAxisY = FindCustomUniform( L"u_vWaterAxisY" ) ;
	ESLAssert( u_vWaterAxisY >= 0 ) ;
	//
	u_fpCascadeFarZ = FindCustomUniform( L"u_fpCascadeFarZ" ) ;
	ESLAssert( u_fpCascadeFarZ >= 0 ) ;
	//
	u_fpCascadePhase1 = FindCustomUniform( L"u_fpCascadePhase1" ) ;
	ESLAssert( u_fpCascadePhase1 >= 0 ) ;
	//
	u_fpCascadeAmplitude1 = FindCustomUniform( L"u_fpCascadeAmplitude1" ) ;
	ESLAssert( u_fpCascadeAmplitude1 >= 0 ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
	render.SetCustomShaderUniformFloat( L"u_fpAmplitude", &m_fpAmplitude, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpNormalAmp", &m_fpNormalAmp, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vAmplitude", &m_vAmplitude, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vBumpAmplitude", &m_vBumpAmplitude, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vFrequency", &m_vFrequency, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vBumpFrequency", &m_vBumpFrequency, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vTime", &m_vTime, 1 ) ;
	render.SetCustomShaderUniformVector4D( L"u_vBumpTime", &m_vBumpTime, 1 ) ;
	render.SetCustomShaderUniformVector2D( L"u_vDirection", &m_vDirection[0], 4 ) ;
	render.SetCustomShaderUniformVector2D( L"u_vBumpDirection", &m_vBumpDirection[0], 4 ) ;
	render.SetCustomShaderUniformVector3D( L"u_vWaterAxisX", &m_vWaterAxisX, 1 ) ;
	render.SetCustomShaderUniformVector3D( L"u_vWaterAxisY", &m_vWaterAxisY, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpCascadeFarZ", &m_fpCascadeFarZ, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpCascadePhase1", &m_fpCascadePhase1, 1 ) ;
	render.SetCustomShaderUniformFloat( L"u_fpCascadeAmplitude1", &m_fpCascadeAmplitude1, 1 ) ;
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataFloatAs( L"u_fpAmplitude", &m_fpAmplitude, 1 ) ;
	unis.SetDataFloatAs( L"u_fpNormalAmp", &m_fpNormalAmp, 1 ) ;
	unis.SetDataVector4DAs( L"u_vAmplitude", &m_vAmplitude, 1 ) ;
	unis.SetDataVector4DAs( L"u_vBumpAmplitude", &m_vBumpAmplitude, 1 ) ;
	unis.SetDataVector4DAs( L"u_vFrequency", &m_vFrequency, 1 ) ;
	unis.SetDataVector4DAs( L"u_vBumpFrequency", &m_vBumpFrequency, 1 ) ;
	unis.SetDataVector4DAs( L"u_vTime", &m_vTime, 1 ) ;
	unis.SetDataVector4DAs( L"u_vBumpTime", &m_vBumpTime, 1 ) ;
	unis.SetDataVector2DAs( L"u_vDirection", &m_vDirection[0], 4 ) ;
	unis.SetDataVector2DAs( L"u_vBumpDirection", &m_vBumpDirection[0], 4 ) ;
	unis.SetDataVector3DAs( L"u_vWaterAxisX", &m_vWaterAxisX, 1 ) ;
	unis.SetDataVector3DAs( L"u_vWaterAxisY", &m_vWaterAxisY, 1 ) ;
	unis.SetDataFloatAs( L"u_fpCascadeFarZ", &m_fpCascadeFarZ, 1 ) ;
	unis.SetDataFloatAs( L"u_fpCascadePhase1", &m_fpCascadePhase1, 1 ) ;
	unis.SetDataFloatAs( L"u_fpCascadeAmplitude1", &m_fpCascadeAmplitude1, 1 ) ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::SetParameter
	( const SGLOpenGLSimpleWaterShader::Parameter& param )
{
	m_vAmplitude.x = param.waterVertex[0].fpAmplitude ;
	m_vAmplitude.y = param.waterVertex[1].fpAmplitude ;
	m_vAmplitude.z = param.waterVertex[2].fpAmplitude ;
	m_vAmplitude.w = param.waterVertex[3].fpAmplitude ;
	m_vFrequency.x = param.waterVertex[0].fpFrequency ;
	m_vFrequency.y = param.waterVertex[1].fpFrequency ;
	m_vFrequency.z = param.waterVertex[2].fpFrequency ;
	m_vFrequency.w = param.waterVertex[3].fpFrequency ;
	m_vTime.x = param.waterVertex[0].radTime ;
	m_vTime.y = param.waterVertex[1].radTime ;
	m_vTime.z = param.waterVertex[2].radTime ;
	m_vTime.w = param.waterVertex[3].radTime ;
	m_vDirection[0] = param.waterVertex[0].vDirection ;
	m_vDirection[1] = param.waterVertex[1].vDirection ;
	m_vDirection[2] = param.waterVertex[2].vDirection ;
	m_vDirection[3] = param.waterVertex[3].vDirection ;
	SetCustomUniformVector4D( u_vAmplitude, &m_vAmplitude, 1 ) ;
	SetCustomUniformVector4D( u_vFrequency, &m_vFrequency, 1 ) ;
	SetCustomUniformVector4D( u_vTime, &m_vTime, 1 ) ;
	SetCustomUniformVector2D( u_vDirection, &m_vDirection[0], 4 ) ;
	//
	m_vBumpAmplitude.x = param.waterBump[0].fpAmplitude ;
	m_vBumpAmplitude.y = param.waterBump[1].fpAmplitude ;
	m_vBumpAmplitude.z = param.waterBump[2].fpAmplitude ;
	m_vBumpAmplitude.w = param.waterBump[3].fpAmplitude ;
	m_vBumpFrequency.x = param.waterBump[0].fpFrequency ;
	m_vBumpFrequency.y = param.waterBump[1].fpFrequency ;
	m_vBumpFrequency.z = param.waterBump[2].fpFrequency ;
	m_vBumpFrequency.w = param.waterBump[3].fpFrequency ;
	m_vBumpTime.x = param.waterBump[0].radTime ;
	m_vBumpTime.y = param.waterBump[1].radTime ;
	m_vBumpTime.z = param.waterBump[2].radTime ;
	m_vBumpTime.w = param.waterBump[3].radTime ;
	m_vBumpDirection[0] = param.waterBump[0].vDirection ;
	m_vBumpDirection[1] = param.waterBump[1].vDirection ;
	m_vBumpDirection[2] = param.waterBump[2].vDirection ;
	m_vBumpDirection[3] = param.waterBump[3].vDirection ;
	SetCustomUniformVector4D( u_vBumpAmplitude, &m_vBumpAmplitude, 1 ) ;
	SetCustomUniformVector4D( u_vBumpFrequency, &m_vBumpFrequency, 1 ) ;
	SetCustomUniformVector4D( u_vBumpTime, &m_vBumpTime, 1 ) ;
	SetCustomUniformVector2D( u_vBumpDirection, &m_vBumpDirection[0], 4 ) ;
	//
	m_vWaterAxisX = param.vLevelAxisX ;
	m_vWaterAxisY = param.vLevelAxisY ;
	SetCustomUniformVector3D( u_vWaterAxisX, &m_vWaterAxisX, 1 ) ;
	SetCustomUniformVector3D( u_vWaterAxisY, &m_vWaterAxisY, 1 ) ;
	//
	m_fpAmplitude = param.fpAmplitude ;
	m_fpNormalAmp = param.fpNormalAmp ;
	SetCustomUniformFloat( u_fpAmplitude, &m_fpAmplitude, 1 ) ;
	SetCustomUniformFloat( u_fpNormalAmp, &m_fpNormalAmp, 1 ) ;
	//
	m_fpCascadeFarZ = param.zCascadeFar ;
	m_fpCascadePhase1 = param.fpCascadePhase[0] ;
	m_fpCascadeAmplitude1 = param.fpCascadeAmp[0] ;
	SetCustomUniformFloat( u_fpCascadeFarZ, &m_fpCascadeFarZ, 1 ) ;
	SetCustomUniformFloat( u_fpCascadePhase1, &m_fpCascadePhase1, 1 ) ;
	SetCustomUniformFloat( u_fpCascadeAmplitude1, &m_fpCascadeAmplitude1, 1 ) ;
}

// 時間を更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLSimpleWaterShader::SetTimeParameter
	( const float32_t * pTime, const float32_t * pBumpTime )
{
	m_vTime.x = pTime[0] ;
	m_vTime.y = pTime[1] ;
	m_vTime.z = pTime[2] ;
	m_vTime.w = pTime[3] ;
	SetCustomUniformVector4D( u_vTime, &m_vTime, 1 ) ;
	//
	m_vBumpTime.x = pBumpTime[0] ;
	m_vBumpTime.y = pBumpTime[1] ;
	m_vBumpTime.z = pBumpTime[2] ;
	m_vBumpTime.w = pBumpTime[3] ;
	SetCustomUniformVector4D( u_vBumpTime, &m_vBumpTime, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL Depth 用 3x3 ループフィルタ (Compute Shader)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DOpenGLShadowmapDepthFilter5x5Shader,
		SGLOpenGLComputeShader, S3DShadowmapDepthFilter5x5Interface )

const float32_t	S3DOpenGLShadowmapDepthFilter5x5Shader::s_fpDefaultKernel[KernelSize] =
{
	1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f,
	1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f,
	1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f,
	1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f,
	1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f, 1.0f/25.0f,
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////
S3DOpenGLShadowmapDepthFilter5x5Shader::S3DOpenGLShadowmapDepthFilter5x5Shader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLComputeShader( pOpenGL ),
		m_pInputDepth( nullptr ),
		m_pOutputDepth( nullptr )
{
	for ( int i = 0; i < KernelSize; i ++ )
	{
		m_fpFilterKernel[i] = s_fpDefaultKernel[i] ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
S3DOpenGLShadowmapDepthFilter5x5Shader::~S3DOpenGLShadowmapDepthFilter5x5Shader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::GetSourceInfo
		( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	//
	source.typeProgram = S3DRenderDevice::programCompute ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 420 ;
	source.srcCompute.pszPlaneSrc = nullptr ;
	source.srcCompute.pbytEncodedSrc = &g_glsl_shadowmap_depth_filter_5x5_el2_comp[0] ;
	source.srcCompute.nEncodedBytes = sizeof(g_glsl_shadowmap_depth_filter_5x5_el2_comp) ;
	source.dimLocalSize.x = 16 ;
	source.dimLocalSize.y = 16 ;
	source.dimLocalSize.z = 1 ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_imageOutput", S3DCustomShader::uniformImageWrite, 1	},
		{	L"u_imageInput", S3DCustomShader::uniformTexture, 1	},
		{	L"u_nImageWidth", S3DCustomShader::uniformInt, 1	},
		{	L"u_nImageHeight", S3DCustomShader::uniformInt, 1	},
		{	L"u_nImageLayers", S3DCustomShader::uniformInt, 1	},
		{	L"u_filterKernel", S3DCustomShader::uniformFloat, KernelSize	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
	//
	u_imageInput = FindCustomUniform( L"u_imageInput" ) ;
	ESLAssert( u_imageInput >= 0 ) ;
	//
	u_imageOutput = FindCustomUniform( L"u_imageOutput" ) ;
	ESLAssert( u_imageOutput >= 0 ) ;
	//
	u_nImageWidth = FindCustomUniform( L"u_nImageWidth" ) ;
	ESLAssert( u_nImageWidth >= 0 ) ;
	//
	u_nImageHeight = FindCustomUniform( L"u_nImageHeight" ) ;
	ESLAssert( u_nImageHeight >= 0 ) ;
	//
	u_nImageLayers = FindCustomUniform( L"u_nImageLayers" ) ;
	ESLAssert( u_nImageLayers >= 0 ) ;
	//
	u_filterKernel = FindCustomUniform( L"u_filterKernel" ) ;
	ESLAssert( u_filterKernel >= 0 ) ;

	SetGaussianFilter( 3.0f ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
	unis.SetDataImageAs( L"u_imageOutput", uniformImageWrite, m_pOutputDepth ) ;
	unis.SetDataImageAs( L"u_imageInput", uniformTexture, m_pInputDepth ) ;
	unis.SetDataFloatAs( L"u_filterKernel", m_fpFilterKernel, KernelSize ) ;

	if ( m_pInputDepth != nullptr )
	{
		SGLSize	sizeImage = m_pInputDepth->GetImageSize() ;
		int32_t	nLayers = (int32_t) m_pInputDepth->GetFrameCount() ;
		unis.SetDataIntAs( L"u_nImageWidth", &(sizeImage.w), 1 ) ;
		unis.SetDataIntAs( L"u_nImageHeight", &(sizeImage.h), 1 ) ;
		unis.SetDataIntAs( L"u_nImageLayers", &(nLayers), 1 ) ;
	}
}

// ワークグループサイズ計算
//////////////////////////////////////////////////////////////////////////
S3DComputeShaderInterface::DimSize
	S3DOpenGLShadowmapDepthFilter5x5Shader::CalcWorkGroupSize( void ) const
{
	if ( m_pOutputDepth == nullptr )
	{
		return	DimSize( 0, 0, 0 ) ;
	}
	DimSize	dimLocal = GetWorkLocalSize() ;
	SGLSize	sizeImage = m_pOutputDepth->GetImageSize() ;
	size_t	nLayers = m_pOutputDepth->GetFrameCount() ;
	return	DimSize( (sizeImage.w + dimLocal.x - 1) / dimLocal.x,
						(sizeImage.h + dimLocal.y - 1) / dimLocal.y, nLayers ) ;
}

// 入力画像
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::SetInputDepth( SGLImageObject * pInput )
{
	m_pInputDepth = pInput ;
	SetCustomUniformImage( u_imageInput, uniformTexture/*uniformImageRead*/, pInput ) ;

	if ( m_pInputDepth != nullptr )
	{
		SGLSize	sizeImage = m_pInputDepth->GetImageSize() ;
		int32_t	nLayers = (int32_t) m_pInputDepth->GetFrameCount() ;
		SetCustomUniformInt( u_nImageWidth, &(sizeImage.w), 1 ) ;
		SetCustomUniformInt( u_nImageHeight, &(sizeImage.h), 1 ) ;
		SetCustomUniformInt( u_nImageLayers, &(nLayers), 1 ) ;
	}
}

// 出力画像
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::SetOutputDepth( SGLImageObject * pOutput )
{
	m_pOutputDepth = pOutput ;
	SetCustomUniformImage( u_imageOutput, uniformImageWrite, pOutput ) ;
}

// ループフィルター行列
//////////////////////////////////////////////////////////////////////////
void S3DOpenGLShadowmapDepthFilter5x5Shader::SetFilterKernel( const float32_t * pKernel )
{
	for ( int i = 0; i < KernelSize; i ++ )
	{
		m_fpFilterKernel[i] = pKernel[i] ;
	}
	SetCustomUniformFloat( u_filterKernel, m_fpFilterKernel, KernelSize ) ;
}



//////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL アナグリフ表示シェーダー
//////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLOpenGLAnaglyphShader::SHADER_ID = L"ANAGLYPH" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLAnaglyphShader, SGLOpenGLCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLOpenGLAnaglyphShader::SGLOpenGLAnaglyphShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLOpenGLAnaglyphShader::~SGLOpenGLAnaglyphShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLAnaglyphShader::GetSourceInfo( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderUseLuminousTexture
							| S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_anaglyph_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_anaglyph_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLAnaglyphShader::OnInitCustomUniform( void )
{
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLAnaglyphShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLAnaglyphShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL インターリーブ表示シェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLOpenGLInterleaveShader::SHADER_ID = L"INTERLEAVE" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLInterleaveShader, SGLOpenGLCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLInterleaveShader::SGLOpenGLInterleaveShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLCustomShader( pOpenGL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLInterleaveShader::~SGLOpenGLInterleaveShader( void )
{
}

// FeatureInfo 取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLInterleaveShader::GetSourceInfo( S3DRenderDevice::ShaderSourceInfo & source )
{
	eslFillMemory
		( &source, 0, sizeof(S3DRenderDevice::ShaderSourceInfo) ) ;
	source.typeProgram = S3DRenderDevice::programShader ;
	source.typeSource = S3DRenderDevice::shaderGLSL ;
	source.versionType = 0 ;
	source.nBaseShader = 0 ;
	source.flagsFeature = S3DRenderDevice::shaderUseLuminousTexture
							| S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitLight ;
	source.nBoneLimit = 0 ;
	source.nLightLimit = 0 ;
	source.srcVertex.pszPlaneSrc = nullptr ;
	source.srcVertex.pbytEncodedSrc = nullptr ;
	source.srcFragment.pszPlaneSrc = nullptr ;
	source.srcFragment.pbytEncodedSrc = &g_glsl_main_interleave_frag[0] ;
	source.srcFragment.nEncodedBytes = sizeof(g_glsl_main_interleave_frag) ;
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLInterleaveShader::OnInitCustomUniform( void )
{
	static const S3DCustomShader::UniformEntry	uniforms[] =
	{
		{	L"u_bVertical", S3DCustomShader::uniformInt, 1	},
		{	L"u_bOddLine", S3DCustomShader::uniformInt, 1	},
	} ;
	RegisterCustomUniforms
		( &uniforms[0], sizeof(uniforms)/sizeof(uniforms[0]) ) ;
}

// シェーダーパラメータをレンダラに設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLInterleaveShader::SetShaderUniformsTo( S3DRenderContextInterface& render )
{
}

// シェーダーパラメータを UniformSet に設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLInterleaveShader::SetShaderUniformsTo( S3DCustomShader::UniformSet& unis )
{
}

