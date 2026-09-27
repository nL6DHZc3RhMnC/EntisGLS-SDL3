
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

#if	!defined(__COTOPHA__)
#include <sakuragl/sgl_opengl_render_context.h>
#include <sakuragl/sgl3d/sgl_render_buffer.h>
#if	!defined(__PLATFORM_WINDOWS__) || !defined(__ENTIS_GLS__)
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl3d/sgl_hybrid_renderer.h>
#endif
#endif

#include <sakuragl/sgl3d/sgl_render_software_renderer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 表面属性
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)
// シェーディングフラグ
/*
const uint64_t	SakuraGL::shadingMethodRayTracingByPixel	= 0x0000000100000000 ;
const uint64_t	SakuraGL::shadingHintNoZSort				= 0x0000000200000000 ;
const uint64_t	SakuraGL::shadingMeshSurfaceOffset			= 0x0000000400000000 ;
const uint64_t	SakuraGL::shadingMakeBlendAdd				= 0x0000000800000000 ;
const uint64_t	SakuraGL::shadingEmisiveTarget				= 0x0000001000000000 ;
const uint64_t	SakuraGL::shadingNoDrawOffsetBorder			= 0x0000002000000000 ;
const uint64_t	SakuraGL::shadingRefractEnvMapping			= 0x0000004000000000 ;
const uint64_t	SakuraGL::shadingSpecularMapping			= 0x0000008000000000 ;
const uint64_t	SakuraGL::shadingAllLocalTextureMask		= shadingTextureMapping
																| shadingEnvironmentMapping
																| shadingNormalTexture
																| shadingLuminousTexture
																| shadingAlphaTexture
																| shadingHeightTexture
																| 0x0000008000000000
																| 0x0000100000000000;
const uint64_t	SakuraGL::shadingNormalizedUVScale			= 0x0000010000000000 ;
const uint64_t	SakuraGL::shadingDisableColorEffect			= 0x0000020000000000 ;
const uint64_t	SakuraGL::shadingGlobalAOLightMap			= 0x0000100000000000 ;
const uint64_t	SakuraGL::shadingAppExtension1				= 0x0001000000000000 ;
const uint64_t	SakuraGL::shadingAppExtension2				= 0x0002000000000000 ;
const uint64_t	SakuraGL::shadingAppExtension3				= 0x0004000000000000 ;
const uint64_t	SakuraGL::shadingAppExtension4				= 0x0008000000000000 ;
*/
#endif


// XML デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSurfaceAttribute::ParseXML( SSystem::SXMLDocument& xmlAttr )
{
	SXMLDocument::AttrInteger	xaiFlagPairs[] =
	{
		{ L"no_shading", shadingMethodNothing },		// = 0
		{ L"gouraud_shading", shadingMethodGouraud },
		{ L"phong_shading", shadingMethodPhong },
		{ L"toon_shading", shadingMethodToon },
		{ L"ray_shadowing", shadingMethodRayShadowing },
		{ L"ray_reflecting", shadingMethodRayReflecting },
		{ L"ray_refracting", shadingMethodRayRefracting },
		{ L"texture_tiling", shadingTextureTiling },
		{ L"texture_triming", shadingTextureTriming },
		{ L"texture_smoothing", shadingTextureSmoothing },
		{ L"texture_dithering", shadingTextureDithering },
		{ L"texture_mapping", shadingTextureMapping },
		{ L"environment_mapping", shadingEnvironmentMapping },
		{ L"global_environment_mapping", shadingGEnvironmentMapping },
		{ L"normal_texture", shadingNormalTexture },
		{ L"luminous_texture", shadingLuminousTexture },
		{ L"alpha_texture", shadingAlphaTexture },
		{ L"height_texture", shadingHeightTexture },
		{ L"refract_env_mapping", shadingRefractEnvMapping },
		{ L"specular_mapping", shadingSpecularMapping },
		{ L"global_ao_lightmap", shadingGlobalAOLightMap },
		{ L"normalized_uv", shadingNormalizedUVScale },
		{ L"single_side", shadingSingleSidePlane },
		{ L"no_zbuffer", shadingNoZBuffer },
		{ L"no_write_zbuffer", shadingZBufferNoWrite },
		{ L"offset_border", shadingDrawOffsetBorder },
		{ L"surface_offset", shadingMeshSurfaceOffset },
		{ L"hint_of_unknown", shadingHintOfUnknown },		// = 0
		{ L"hint_of_full_alpha", shadingHintOfFullAlpha },
		{ L"hint_of_alpha", shadingHintOfAlpha },
		{ L"hint_of_half_alpha", shadingHintOfHalfAlpha },	// = shadingHintOfFullAlpha | shadingHintOfAlpha
		{ L"hint_no_z_soft", shadingHintNoZSort },
		{ L"no_shadow_object", shadingNoShadowObject },
		{ L"no_drop_shadow", shadingNoDropShadow },
		{ L"no_reflect_object", shadingNoReflectObject },
		{ L"global_reflect_object", shadingGlobalReflectObject },
		{ L"no_fog_effect", shadingNoFogEffect },
		{ L"vertex_alpha", shadingVertexAlpha },
		{ L"blend_add", shadingMakeBlendAdd },
		{ L"emisive_target", shadingEmisiveTarget },
		{ L"no_offset_border", shadingNoDrawOffsetBorder },
		{ L"no_color_effect", shadingDisableColorEffect },
		{ L"app_extension1", shadingAppExtension1 },
		{ L"app_extension2", shadingAppExtension2 },
		{ L"app_extension3", shadingAppExtension3 },
		{ L"app_extension4", shadingAppExtension4 },
		{ NULL, 0 },
	} ;
	SXMLDocument::AttrInteger	xaiExFlagPairs[] =
	{
		{ L"variety_shade", shadingExVarietyShade },
		{ L"back_diffusion", shadingExBackDiffusion },
		{ L"specular_color", shadingExSpecularColor },
		{ L"border_param", shadingExBorderParam },
		{ L"back_light", shadingExBackLight },
		{ L"rim_light", shadingExRimLight },
		{ NULL, 0 },
	} ;
	flagsShading =
			xmlAttr.GetAttrComplexIntegerAs( L"flags", xaiFlagPairs ) ;
	nExFlags =
		(uint32_t) xmlAttr.GetAttrComplexIntegerAs( L"ex_flags", xaiExFlagPairs ) ;
	colorBase.rgbMul.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"color_mul", 0xFFFFFF ) ;
	colorBase.rgbAdd.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"color_add", 0 ) ;
	colorShade.rgbMul.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"shade_mul", 0 ) ;
	colorShade.rgbAdd.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"shade_add", 0 ) ;
	nAmbient =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"ambient", 0 ) ;
	nDiffusion =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"diffusion", 0 ) ;
	nSpecular =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"specular", 0 ) ;
	nSpecularSize =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"specular_size", 0 ) ;
	nTransparency =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"transparency", 0 ) ;
	nDeepness =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"deepness", 0 ) ;
	nDeepnessPower =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"deepness_pow", 0x100 ) ;
	nReflection =
		(uint32_t) xmlAttr.GetAttrIntegerAs( L"reflection", 0 ) ;
	fpRefraction =
		(float32_t) xmlAttr.GetAttrRealAs( L"refraction", 0 ) ;
	nEmission =
		(uint32_t) xmlAttr.GetAttrIntegerAs( L"emission", 0 ) ;
	cosShadeThreshold =
		(float32_t) xmlAttr.GetAttrRealAs( L"shade_threshold", 0.0 ) ;
	fpToonShadeThreshold =
		(float32_t) xmlAttr.GetAttrRealAs( L"toon_shade_threshold", 0.5 ) ;
	fpToonShadeBrightness =
		(float32_t) xmlAttr.GetAttrRealAs( L"toon_shade_brightness", 0.5 ) ;
	nBackDiffusion =
		(int32_t) xmlAttr.GetAttrIntegerAs( L"back_diffusion", 0 ) ;
	rgbSpecularColor.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"specular_color", 0xFFFFFF ) ;
	rgbBorderColor.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"border_color", 0 ) ;
	fpBorderThicknessA =
		(float32_t) xmlAttr.GetAttrRealAs( L"border_thickness_a", 1.0 ) ;
	fpBorderThicknessB =
		(float32_t) xmlAttr.GetAttrRealAs( L"border_thickness_b", 0.0 ) ;
	fpBackLight =
		(float32_t) xmlAttr.GetAttrRealAs( L"back_light", 0.0 ) ;
	colorBackLight.rgbMul.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"back_light_mul", 0xFFFFFF ) ;
	colorBackLight.rgbAdd.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"back_light_add", 0 ) ;
	fpRimLight =
		(float32_t) xmlAttr.GetAttrRealAs( L"rim_light", 0.0 ) ;
	fpRimLightDeepness =
		(float32_t) xmlAttr.GetAttrRealAs( L"rim_light_deepness", 0.1 ) ;
	rgbRimLightColor.ui32 =
		(uint32_t) xmlAttr.GetAttrHexIntegerAs( L"rim_color", 0xFFFFFF ) ;
	return	sglErrSuccess ;
}

// XML シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSurfaceAttribute::FormatXML( SSystem::SXMLDocument& xmlAttr ) const
{
	SXMLDocument::AttrInteger	xaiFlagPairs[] =
	{
		{ L"no_shading", shadingMethodNothing },		// = 0
		{ L"gouraud_shading", shadingMethodGouraud },
		{ L"phong_shading", shadingMethodPhong },
		{ L"toon_shading", shadingMethodToon },
		{ L"ray_shadowing", shadingMethodRayShadowing },
		{ L"ray_reflecting", shadingMethodRayReflecting },
		{ L"ray_refracting", shadingMethodRayRefracting },
		{ L"texture_tiling", shadingTextureTiling },
		{ L"texture_triming", shadingTextureTriming },
		{ L"texture_smoothing", shadingTextureSmoothing },
		{ L"texture_dithering", shadingTextureDithering },
		{ L"texture_mapping", shadingTextureMapping },
		{ L"environment_mapping", shadingEnvironmentMapping },
		{ L"global_environment_mapping", shadingGEnvironmentMapping },
		{ L"normal_texture", shadingNormalTexture },
		{ L"luminous_texture", shadingLuminousTexture },
		{ L"alpha_texture", shadingAlphaTexture },
		{ L"height_texture", shadingHeightTexture },
		{ L"refract_env_mapping", shadingRefractEnvMapping },
		{ L"specular_mapping", shadingSpecularMapping },
		{ L"global_ao_lightmap", shadingGlobalAOLightMap },
		{ L"normalized_uv", shadingNormalizedUVScale },
		{ L"single_side", shadingSingleSidePlane },
		{ L"no_zbuffer", shadingNoZBuffer },
		{ L"no_write_zbuffer", shadingZBufferNoWrite },
		{ L"offset_border", shadingDrawOffsetBorder },
		{ L"surface_offset", shadingMeshSurfaceOffset },
		{ L"hint_of_unknown", shadingHintOfUnknown },		// = 0
		{ L"hint_of_full_alpha", shadingHintOfFullAlpha },
		{ L"hint_of_alpha", shadingHintOfAlpha },
		{ L"hint_of_half_alpha", shadingHintOfHalfAlpha },	// = shadingHintOfFullAlpha | shadingHintOfAlpha
		{ L"hint_no_z_soft", shadingHintNoZSort },
		{ L"no_shadow_object", shadingNoShadowObject },
		{ L"no_drop_shadow", shadingNoDropShadow },
		{ L"no_reflect_object", shadingNoReflectObject },
		{ L"global_reflect_object", shadingGlobalReflectObject },
		{ L"no_fog_effect", shadingNoFogEffect },
		{ L"vertex_alpha", shadingVertexAlpha },
		{ L"blend_add", shadingMakeBlendAdd },
		{ L"emisive_target", shadingEmisiveTarget },
		{ L"no_offset_border", shadingNoDrawOffsetBorder },
		{ L"no_color_effect", shadingDisableColorEffect },
		{ L"app_extension1", shadingAppExtension1 },
		{ L"app_extension2", shadingAppExtension2 },
		{ L"app_extension3", shadingAppExtension3 },
		{ L"app_extension4", shadingAppExtension4 },
		{ NULL, 0 },
	} ;
	SXMLDocument::AttrInteger	xaiExFlagPairs[] =
	{
		{ L"variety_shade", shadingExVarietyShade },
		{ L"back_diffusion", shadingExBackDiffusion },
		{ L"specular_color", shadingExSpecularColor },
		{ L"border_param", shadingExBorderParam },
		{ L"back_light", shadingExBackLight },
		{ L"rim_light", shadingExRimLight },
		{ NULL, 0 },
	} ;
	xmlAttr.SetAttrComplexIntegerAs
			( L"flags", xaiFlagPairs, flagsShading ) ;
	xmlAttr.SetAttrComplexIntegerAs
			( L"ex_flags", xaiExFlagPairs, nExFlags ) ;
	xmlAttr.SetAttrHexIntegerAs( L"color_mul", colorBase.rgbMul.ui32 ) ;
	xmlAttr.SetAttrHexIntegerAs( L"color_add", colorBase.rgbAdd.ui32 ) ;
	xmlAttr.SetAttrHexIntegerAs( L"shade_mul", colorShade.rgbMul.ui32 ) ;
	xmlAttr.SetAttrHexIntegerAs( L"shade_add", colorShade.rgbAdd.ui32 ) ;
	xmlAttr.SetAttrIntegerAs( L"ambient", nAmbient ) ;
	xmlAttr.SetAttrIntegerAs( L"diffusion", nDiffusion ) ;
	xmlAttr.SetAttrIntegerAs( L"specular", nSpecular ) ;
	xmlAttr.SetAttrIntegerAs( L"specular_size", nSpecularSize ) ;
	xmlAttr.SetAttrIntegerAs( L"transparency", nTransparency ) ;
	xmlAttr.SetAttrIntegerAs( L"deepness", nDeepness ) ;
	xmlAttr.SetAttrIntegerAs( L"deepness_pow", nDeepnessPower ) ;
	xmlAttr.SetAttrIntegerAs( L"reflection", nReflection ) ;
	xmlAttr.SetAttrRealAs( L"refraction", fpRefraction ) ;
	xmlAttr.SetAttrIntegerAs( L"emission", nEmission ) ;
	xmlAttr.SetAttrRealAs( L"shade_threshold", cosShadeThreshold ) ;
	xmlAttr.SetAttrRealAs( L"toon_shade_threshold", fpToonShadeThreshold ) ;
	xmlAttr.SetAttrRealAs( L"toon_shade_brightness", fpToonShadeBrightness ) ;
	xmlAttr.SetAttrIntegerAs( L"back_diffusion", nBackDiffusion ) ;
	xmlAttr.SetAttrHexIntegerAs( L"specular_color", rgbSpecularColor.ui32 ) ;
	xmlAttr.SetAttrHexIntegerAs( L"border_color", rgbBorderColor.ui32 ) ;
	xmlAttr.SetAttrRealAs( L"border_thickness_a", fpBorderThicknessA ) ;
	xmlAttr.SetAttrRealAs( L"border_thickness_b", fpBorderThicknessB ) ;
	xmlAttr.SetAttrRealAs( L"back_light", fpBackLight ) ;
	xmlAttr.SetAttrHexIntegerAs( L"back_light_mul", colorBackLight.rgbMul.ui32 ) ;
	xmlAttr.SetAttrHexIntegerAs( L"back_light_add", colorBackLight.rgbAdd.ui32 ) ;
	xmlAttr.SetAttrRealAs( L"rim_light", fpRimLight ) ;
	xmlAttr.SetAttrRealAs( L"rim_light_deepness", fpRimLightDeepness ) ;
	xmlAttr.SetAttrHexIntegerAs( L"rim_color", rgbRimLightColor.ui32 ) ;
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 表面属性
//////////////////////////////////////////////////////////////////////////////

S3DMaterial S3DMaterial::m_materialDefault[S3DMaterial::defaultMaterialCount] =
{
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud | shadingSingleSidePlane,
				0, 0xFFFFFF, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud | shadingSingleSidePlane,
				0, 0, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud | shadingSingleSidePlane,
				0xFFFFFF, 0, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud,
				0, 0xFFFFFF, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud,
				0, 0, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
	S3DMaterial
		( S3DSurfaceAttribute
			( shadingMethodGouraud,
				0xFFFFFF, 0, 0, 0, 0, 0x100, 0, 0, 0, 0, 0, 0.0f, 0 ) ),
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMaterialBuffer, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMaterial, SObject )

// 構築関数
S3DMaterial::S3DMaterial( void )
	: m_zTexture(1024.0f), m_flagBack(false), m_pMaterialBuf(NULL)
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		m_pTexture[i] = NULL ;
		m_flagTexture[i] = textureDiffusion ;
		m_fpTextureApply[i] = 1.0f ;
		m_fpTextureParam1[i] = 0.0f ;
		m_pBackTexture[i] = NULL ;
		m_flagBackTexture[i] = textureDiffusion ;
		m_fpBackTextureApply[i] = 1.0f ;
		m_fpBackTextureParam1[i] = 0.0f ;
	}
	m_flagTexture[0] = textureMain ;
	m_flagBackTexture[0] = textureMain ;
}

S3DMaterial::S3DMaterial( const S3DMaterial& material )
	: m_zTexture(1024.0f), m_flagBack(false), m_pMaterialBuf(NULL)
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		m_pTexture[i] = NULL ;
		m_flagTexture[i] = textureDiffusion ;
		m_fpTextureApply[i] = 1.0f ;
		m_fpTextureParam1[i] = 0.0f ;
		m_pBackTexture[i] = NULL ;
		m_flagBackTexture[i] = textureDiffusion ;
		m_fpBackTextureApply[i] = 1.0f ;
		m_fpBackTextureParam1[i] = 0.0f ;
	}
	CopyFrom( material ) ;
}

S3DMaterial::S3DMaterial( const S3DSurfaceAttribute& attr )
	: m_attrSurface( attr ),
		m_zTexture(1024.0f), m_flagBack(false), m_pMaterialBuf(NULL)
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		m_pTexture[i] = NULL ;
		m_flagTexture[i] = textureDiffusion ;
		m_fpTextureApply[i] = 1.0f ;
		m_fpTextureParam1[i] = 0.0f ;
		m_pBackTexture[i] = NULL ;
		m_flagBackTexture[i] = textureDiffusion ;
		m_fpBackTextureApply[i] = 1.0f ;
		m_fpBackTextureParam1[i] = 0.0f ;
	}
	m_flagTexture[0] = textureMain ;
	m_flagBackTexture[0] = textureMain ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMaterial::~S3DMaterial( void )
{
	S3DMaterial::RemoveAllMaterialBuffer() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::CopyFrom( const S3DMaterial& material )
{
	m_attrSurface = material.m_attrSurface ;
	m_zTexture = material.m_zTexture ;
	m_flagBack = material.m_flagBack ;
	m_attrBack = material.m_attrBack ;

	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		m_pTexture[i] = material.m_pTexture[i] ;
		m_idTexture[i] = material.m_idTexture[i] ;
		m_flagTexture[i] = material.m_flagTexture[i] ;
		m_fpTextureApply[i] = material.m_fpTextureApply[i] ;
		m_fpTextureParam1[i] = material.m_fpTextureParam1[i] ;
		m_pBackTexture[i] = material.m_pBackTexture[i] ;
		m_idBackTexture[i] = material.m_idBackTexture[i] ;
		m_flagBackTexture[i] = material.m_flagBackTexture[i] ;
		m_fpBackTextureApply[i] = material.m_fpBackTextureApply[i] ;
		m_fpBackTextureParam1[i] = material.m_fpBackTextureParam1[i] ;
	}
}

static const SXMLDocument::AttrInteger	g_xaiFlagPairs[] =
{
	{ L"diffusion", S3DMaterial::textureDiffusion },
	{ L"main", S3DMaterial::textureMain },
	{ L"normal", S3DMaterial::textureNormal },
	{ L"luminous", S3DMaterial::textureLuminous },
	{ L"environment", S3DMaterial::textureEnvironment },
	{ L"alpha", S3DMaterial::textureAlpha },
	{ L"height", S3DMaterial::textureHeight },
	{ L"specular", S3DMaterial::textureSpecular },
	{ L"global_ao", S3DMaterial::textureGlobalAO },
	{ NULL, 0 },
} ;

// XML デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterial::ParseXML
	( const SSystem::SXMLDocument & xmlMaterial,
			const S3DTextureLibraryReferencer& libTexture )
{
	SGLError	err ;
	//
	// 表面
	//
	SXMLDocument *	pxmlFace = xmlMaterial.GetElementTagAs( L"face" ) ;
	if ( pxmlFace == NULL )
	{
		Trace( "not found <face> tag at <materials><material>.\n" ) ;
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlAttr = pxmlFace->GetElementTagAs( L"attribute" ) ;
	if ( pxmlAttr == NULL )
	{
		Trace( "not found <attribute> tag at <materials><material><face>.\n" ) ;
		return	sglErrFailed ;
	}
	S3DSurfaceAttribute	attr ;
	err = attr.ParseXML( *pxmlAttr ) ;
	if ( err )
	{
		return	err ;
	}
	SetSurfaceAttribute( attr ) ;
	//
	size_t	j, k ;
	for ( j = 0, k = 0; j < pxmlFace->GetElementsCount(); j ++ )
	{
		SXMLDocument *	pxmlSub = pxmlFace->GetElementAt( j ) ;
		if ( pxmlSub == NULL )
		{
			continue ;
		}
		if ( pxmlSub->GetTag() != L"texture" )
		{
			continue ;
		}
		SString		strTxtID = pxmlSub->GetAttrStringAs( L"id" ) ;
		uint32_t	nFlags =
						(uint32_t) pxmlSub->GetAttrSymbolizedIntegerAs
											( L"flags", g_xaiFlagPairs ) ;
		float32_t	fpApply =
						(float32_t) pxmlSub->GetAttrRealAs( L"apply", 1.0 ) ;
		float32_t	fpParam1 =
						(float32_t) pxmlSub->GetAttrRealAs( L"param1", 0.0 ) ;
		SetTexture
			( libTexture.GetTextureAs( strTxtID ),
					(int) (k ++), nFlags, fpApply, fpParam1, strTxtID ) ;
	}
	//
	// 裏面
	//
	SXMLDocument *	pxmlBack = xmlMaterial.GetElementTagAs( L"back" ) ;
	if ( pxmlBack != NULL )
	{
		pxmlAttr = pxmlBack->GetElementTagAs( L"attribute" ) ;
		if ( pxmlAttr == NULL )
		{
			Trace( "not found <attribute> tag at <materials><material><back>.\n" ) ;
			return	sglErrFailed ;
		}
		err = attr.ParseXML( *pxmlAttr ) ;
		if ( err )
		{
			return	err ;
		}
		SetBackSurfaceAttribute( attr ) ;
		EnableBackSurfaceAttribute( true ) ;
		//
		for ( j = 0, k = 0; j < pxmlBack->GetElementsCount(); j ++ )
		{
			SXMLDocument *	pxmlSub = pxmlBack->GetElementAt( j ) ;
			if ( pxmlSub == NULL )
			{
				continue ;
			}
			if ( pxmlSub->GetTag() != L"texture" )
			{
				continue ;
			}
			SString		strTxtID = pxmlSub->GetAttrStringAs( L"id" ) ;
			uint32_t	nFlags =
							(uint32_t) pxmlSub->GetAttrSymbolizedIntegerAs
												( L"flags", g_xaiFlagPairs ) ;
			float32_t	fpApply =
							(float32_t) pxmlSub->GetAttrRealAs( L"apply", 1.0 ) ;
			float32_t	fpParam1 =
							(float32_t) pxmlSub->GetAttrRealAs( L"param1", 0.0 ) ;
			SetBackTexture
				( libTexture.GetTextureAs( strTxtID ),
						(int) (k ++), nFlags, fpApply, fpParam1, strTxtID ) ;
		}
	}
	return	sglErrSuccess ;
}

// XML シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterial::FormatXML
	( SSystem::SXMLDocument & xmlMaterial,
			const S3DTextureLibraryReferencer& libTexture )
{
	//
	// 表面
	//
	SXMLDocument *	pxmlFace = xmlMaterial.CreateElementTagAs( L"face" ) ;
	SXMLDocument *	pxmlAttr = pxmlFace->CreateElementTagAs( L"attribute" ) ;
	//
	S3DSurfaceAttribute	attr ;
	GetSurfaceAttribute( attr ) ;
	attr.FormatXML( *pxmlAttr ) ;
	//
	size_t	j ;
	for ( j = 0; j < S3DMaterial::textureMaxCount; j ++ )
	{
		SString	strTextureID = m_idTexture[j] ;
		SGLImageObject *	pImage = GetTexture( (int) j ) ;
		if ( pImage != NULL )
		{
			ssize_t	iTexture = libTexture.FindTexturePtr( pImage ) ;
			if ( iTexture >= 0 )
			{
				strTextureID = libTexture.GetTextureIdentityAt( (size_t) iTexture ) ;
			}
		}
		else if ( strTextureID.IsEmpty() )
		{
			continue ;
		}
		SXMLDocument *	pxmlSub = new SXMLDocument ;
		pxmlSub->SetTag( L"texture" ) ;
		pxmlFace->AddElement( pxmlSub ) ;
		//
		pxmlSub->SetAttributeAs
			( L"id", strTextureID ) ;
		pxmlSub->SetAttrSymbolizedIntegerAs
			( L"flags", g_xaiFlagPairs, GetTextureFlags( (int) j ) ) ;
		pxmlSub->SetAttrRealAs
			( L"apply", GetTextureApplication( (int) j ) ) ;
		pxmlSub->SetAttrRealAs
			( L"param1", GetTextureParameter( (int) j ) ) ;
	}
	//
	// 裏面
	//
	if ( IsEnabledBackSurfaceAttribute() )
	{
		SXMLDocument *	pxmlBack = xmlMaterial.CreateElementTagAs( L"back" ) ;
		//
		pxmlAttr = pxmlBack->CreateElementTagAs( L"attribute" ) ;
		GetBackSurfaceAttribute( attr ) ;
		attr.FormatXML( *pxmlAttr ) ;
		//
		for ( j = 0; j < S3DMaterial::textureMaxCount; j ++ )
		{
			SString	strTextureID = m_idBackTexture[j] ;
			SGLImageObject *	pImage = GetBackTexture( (int) j ) ;
			if ( pImage != NULL )
			{
				ssize_t	iTexture = libTexture.FindTexturePtr( pImage ) ;
				if ( iTexture >= 0 )
				{
					strTextureID = libTexture.GetTextureIdentityAt( (size_t) iTexture ) ;
				}
			}
			else if ( strTextureID.IsEmpty() )
			{
				continue ;
			}
			SXMLDocument *	pxmlSub = new SXMLDocument ;
			pxmlSub->SetTag( L"texture" ) ;
			pxmlBack->AddElement( pxmlSub ) ;
			//
			pxmlSub->SetAttributeAs
				( L"id", strTextureID ) ;
			pxmlSub->SetAttrSymbolizedIntegerAs
				( L"flags", g_xaiFlagPairs, GetBackTextureFlags( (int) j ) ) ;
			pxmlSub->SetAttrRealAs
				( L"apply", GetBackTextureApplication( (int) j ) ) ;
			pxmlSub->SetAttrRealAs
				( L"param1", GetBackTextureParameter( (int) j ) ) ;
		}
	}
	return	sglErrSuccess ;
}

// テクスチャタイプ
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMaterial::GetTextureTypeString( uint32_t nType )
{
	return	SXMLDocument::GetSymbolAsIntegerOf( g_xaiFlagPairs, nType ) ;
}

uint32_t S3DMaterial::GetTextureTypeFlag( const wchar_t * pwszType )
{
	return	(uint32_t) SXMLDocument::GetIntegerAsSymbolOf
							( g_xaiFlagPairs, pwszType, textureDiffusion ) ;
}

// テクスチャ参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::UpdateTextureReference
	( const S3DTextureLibraryReferencer& libTexture )
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( !m_idTexture[i].IsEmpty() )
		{
			m_pTexture[i] = libTexture.GetTextureAs( m_idTexture[i] ) ;
		}
		if ( !m_idBackTexture[i].IsEmpty() )
		{
			m_pBackTexture[i] = libTexture.GetTextureAs( m_idBackTexture[i] ) ;
		}
	}
}

// テクスチャ参照有効判定
//////////////////////////////////////////////////////////////////////////////
bool S3DMaterial::IsValidTextureReferenceFor
	( const S3DTextureLibraryReferencer& libTexture ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pTexture[i] != NULL )
		{
			if ( libTexture.FindTexturePtr( m_pTexture[i] ) < 0 )
			{
				return	false ;
			}
		}
		if ( m_pBackTexture[i] != NULL )
		{
			if ( libTexture.FindTexturePtr( m_pBackTexture[i] ) < 0 )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// パラメータ取得
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::GetSurfaceAttribute( S3DSurfaceAttribute& attr ) const
{
	attr = m_attrSurface ;
}

void S3DMaterial::GetBackSurfaceAttribute( S3DSurfaceAttribute& attr ) const
{
	attr = m_attrBack ;
}

bool S3DMaterial::IsEnabledBackSurfaceAttribute( void ) const
{
	return	m_flagBack ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::SetSurfaceAttribute( const S3DSurfaceAttribute& attr )
{
	m_attrSurface = attr ;
	SetUpdateMaterialBuffer() ;
}

void S3DMaterial::SetBackSurfaceAttribute( const S3DSurfaceAttribute& attr )
{
	m_attrBack = attr ;
	SetUpdateMaterialBuffer() ;
}

void S3DMaterial::EnableBackSurfaceAttribute( bool flagBack )
{
	m_flagBack = flagBack ;
	SetUpdateMaterialBuffer() ;
}

// テクスチャ取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DMaterial::GetTexture( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_pTexture[iTexture] ;
	}
	return	NULL ;
}

SGLImageObject * S3DMaterial::GetBackTexture( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_pBackTexture[iTexture] ;
	}
	return	NULL ;
}

// テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::SetTexture
	( SGLImageObject * pImage, int iTexture,	
		uint32_t nFlags, float32_t nApply, float32_t nParam1,
		const wchar_t * pwszTextureID )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_pTexture[iTexture] = pImage ;
		m_flagTexture[iTexture] = nFlags ;
		m_fpTextureApply[iTexture] = nApply ;
		m_fpTextureParam1[iTexture] = nParam1 ;
		m_idTexture[iTexture] = pwszTextureID ;
		//
		SetUpdateMaterialBuffer() ;
	}
}

void S3DMaterial::SetBackTexture
	( SGLImageObject * pImage, int iTexture,	
		uint32_t nFlags, float32_t nApply, float32_t nParam1,
		const wchar_t * pwszTextureID )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_pBackTexture[iTexture] = pImage ;
		m_flagBackTexture[iTexture] = nFlags ;
		m_fpBackTextureApply[iTexture] = nApply ;
		m_fpBackTextureParam1[iTexture] = nParam1 ;
		m_idBackTexture[iTexture] = pwszTextureID ;
		//
		SetUpdateMaterialBuffer() ;
	}
}

void S3DMaterial::SetTextureApplication( int iTexture, float32_t nApply )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_fpTextureApply[iTexture] = nApply ;
		SetUpdateMaterialBuffer() ;
	}
}

void S3DMaterial::SetBackTextureApplication( int iTexture, float32_t nApply )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_fpBackTextureApply[iTexture] = nApply ;
		SetUpdateMaterialBuffer() ;
	}
}

void S3DMaterial::SetTextureParameter( int iTexture, float32_t nParam1 )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_fpTextureParam1[iTexture] = nParam1 ;
		SetUpdateMaterialBuffer() ;
	}
}

void S3DMaterial::SetBackTextureParameter( int iTexture, float32_t nParam1 )
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		m_fpBackTextureParam1[iTexture] = nParam1 ;
		SetUpdateMaterialBuffer() ;
	}
}

uint32_t S3DMaterial::GetTextureFlags( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_flagTexture[iTexture] ;
	}
	return	0 ;
}

uint32_t S3DMaterial::GetTextureType( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_flagTexture[iTexture] & textureTypeMask ;
	}
	return	0 ;
}

uint32_t S3DMaterial::GetBackTextureFlags( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_flagBackTexture[iTexture] ;
	}
	return	0 ;
}

uint32_t S3DMaterial::GetBackTextureType( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_flagBackTexture[iTexture] & textureTypeMask ;
	}
	return	0 ;
}

float32_t S3DMaterial::GetTextureApplication( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_fpTextureApply[iTexture] ;
	}
	return	0.0f ;
}

float32_t S3DMaterial::GetBackTextureApplication( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_fpBackTextureApply[iTexture] ;
	}
	return	0.0f ;
}

float32_t S3DMaterial::GetTextureParameter( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_fpTextureParam1[iTexture] ;
	}
	return	0.0f ;
}

float32_t S3DMaterial::GetBackTextureParameter( int iTexture ) const
{
	if ( (iTexture >= 0) & (iTexture < textureMaxCount) )
	{
		return	m_fpBackTextureParam1[iTexture] ;
	}
	return	0.0f ;
}

// 特定種類のテクスチャ検索
//////////////////////////////////////////////////////////////////////////////
int S3DMaterial::FindTextureTypeOf( uint32_t nType ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( (m_pTexture[i] != NULL)
			&& ((m_flagTexture[i] & textureTypeMask) == nType) )
		{
			return	i ;
		}
	}
	return	-1 ;
}

int S3DMaterial::FindBackTextureTypeOf( uint32_t nType ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( (m_pBackTexture[i] != NULL)
			&& ((m_flagBackTexture[i] & textureTypeMask) == nType) )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// 未使用テクスチャ番号取得
//////////////////////////////////////////////////////////////////////////////
int S3DMaterial::FindEmptyTextureIndex( void ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pTexture[i] == NULL )
		{
			return	i ;
		}
	}
	return	-1 ;
}

int S3DMaterial::FindEmptyBackTextureIndex( void ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pBackTexture[i] == NULL )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// 指定テクスチャ検索
//////////////////////////////////////////////////////////////////////////////
int S3DMaterial::FindTextureOf( SGLImageObject * pImage ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pTexture[i] == pImage )
		{
			return	i ;
		}
	}
	return	-1 ;
}

int S3DMaterial::FindBackTextureOf( SGLImageObject * pImage ) const
{
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pBackTexture[i] == pImage )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// テクスチャ切替用の基準ｚ座標を取得
//////////////////////////////////////////////////////////////////////////////
double S3DMaterial::GetSubTextureZ( void ) const
{
	return	m_zTexture ;
}

// テクスチャ切替用の基準ｚ座標を設定
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::SetSubTextureZ( double zTexture )
{
	m_zTexture = (float32_t) zTexture ;
}

// マテリアル・バッファ更新設定
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::SetUpdateMaterialBuffer( void )
{
	m_csBufLock.Lock() ;
	S3DMaterialBuffer *	pNext = m_pMaterialBuf ;
	while ( pNext != NULL )
	{
		pNext->m_flagUpdate = true ;
		pNext = pNext->m_ptrNext ;
	}
	m_csBufLock.Unlock() ;
}

// マテリアル・バッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterialBuffer * S3DMaterial::GetMaterialBuffer( uint32_t idType ) const
{
	m_csBufLock.Lock() ;
	S3DMaterialBuffer *	pNext = m_pMaterialBuf ;
	while ( pNext != NULL )
	{
		if ( pNext->m_typeMaterial == idType )
		{
			pNext->UpdateMaterial( this ) ;
			m_csBufLock.Unlock() ;
			return	pNext ;
		}
		pNext = pNext->m_ptrNext ;
	}
	m_csBufLock.Unlock() ;
	return	NULL ;
}

// マテリアル・バッファ追加
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::AddMaterialBuffer( S3DMaterialBuffer * pBuf )
{
	m_csBufLock.Lock() ;
	pBuf->m_ptrNext = m_pMaterialBuf ;
	m_pMaterialBuf = pBuf ;
	m_csBufLock.Unlock() ;
}

// 全てのマテリアル・バッファ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMaterial::RemoveAllMaterialBuffer( void )
{
	m_csBufLock.Lock() ;
	S3DMaterialBuffer *	pBuf = m_pMaterialBuf ;
	m_pMaterialBuf = NULL ;
	while ( pBuf != NULL )
	{
		S3DMaterialBuffer *	pNext = pBuf->m_ptrNext ;
		pBuf->m_ptrNext = NULL ;
		delete	pBuf ;
		pBuf = pNext ;
	}
	m_csBufLock.Unlock() ;
}

// テクスチャタイプフラグ（TextureFlags）
// - シェーディングフラグ（S3DShadingFlags）変換
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DMaterial::ShadingFlagOfTextureType( uint32_t nType )
{
	switch ( nType )
	{
	case	textureDiffusion:
		return	shadingTextureMapping ;
	case	textureNormal:
		return	shadingNormalTexture ;
	case	textureLuminous:
		return	shadingLuminousTexture ;
	case	textureEnvironment:
		return	shadingEnvironmentMapping ;
	case	textureAlpha:
		return	shadingAlphaTexture ;
	case	textureHeight:
		return	shadingHeightTexture ;
	case	textureSpecular:
		return	shadingSpecularMapping ;
	case	textureGlobalAO:
		return	shadingGlobalAOLightMap ;
	}
	return	0 ;
}

S3DMaterial::TextureFlags
		S3DMaterial::TextureTypeOfShadingFlag( uint64_t nTextueFlag )
{
	if ( nTextueFlag & textureDiffusion )
	{
		return	textureDiffusion ;
	}
	else if ( nTextueFlag & shadingNormalTexture )
	{
		return	textureNormal ;
	}
	else if ( nTextueFlag & shadingLuminousTexture )
	{
		return	textureLuminous ;
	}
	else if ( nTextueFlag & shadingEnvironmentMapping )
	{
		return	textureEnvironment ;
	}
	else if ( nTextueFlag & shadingAlphaTexture )
	{
		return	textureAlpha ;
	}
	else if ( nTextueFlag & shadingHeightTexture )
	{
		return	textureHeight ;
	}
	else if ( nTextueFlag & shadingSpecularMapping )
	{
		return	textureSpecular ;
	}
	else if ( nTextueFlag & shadingGlobalAOLightMap )
	{
		return	textureGlobalAO ;
	}
	return	textureMain ;
}

// アトラステクスチャへの画像参照領域の取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMaterial::GetImageReferenceRect
	( SGLImageRect& rectRef, SGLSize& sizeAtlas,
		SGLImageObject * pImage, ssize_t iFrame ) const
{
	if ( pImage == NULL )
	{
		return	false ;
	}
	SGLImageObject *
		pAtlas = pImage->GetImageReference( rectRef, iFrame ) ;
	if ( pAtlas == NULL )
	{
		return	false ;
	}
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pTexture[i] == pAtlas )
		{
			sizeAtlas = pAtlas->GetImageSize() ;
			return	true ;
		}
	}
	return	false ;
}

// テクスチャ・サンプリング
//////////////////////////////////////////////////////////////////////////////
bool S3DMaterial::SampleDiffusionTexture
	( SGLPalette& rgbaTexture, float32_t x, float32_t y ) const
{
	SGLImageObject *	pImage =
		GetTexture( (size_t) FindTextureTypeOf
								( S3DMaterial::textureDiffusion ) ) ;
	if ( pImage == NULL )
	{
		rgbaTexture = m_attrSurface.colorShade.rgbAdd ;
		rgbaTexture.argb.Alpha =
			(uint8_t) ~(((int) m_attrSurface.colorShade.rgbMul.argb.Blue
						+ m_attrSurface.colorShade.rgbMul.argb.Green
						+ m_attrSurface.colorShade.rgbMul.argb.Red) / 3) ;
		return	true ;
	}
	return	SampleDiffusionTexture( rgbaTexture, x, y, m_attrSurface, pImage ) ;
}

bool S3DMaterial::SampleLuminousTexture
	( SGLPalette& rgbaTexture, float32_t x, float32_t y ) const
{
	rgbaTexture = 0 ;
	//
	SGLImageObject *	pImage =
		GetTexture( (size_t) FindTextureTypeOf
								( S3DMaterial::textureLuminous ) ) ;
	if ( pImage == NULL )
	{
		return	true ;
	}
	return	SampleDiffusionTexture( rgbaTexture, x, y, m_attrSurface, pImage ) ;
}

void S3DMaterial::SampleTextures
	( S3DMaterial::ColorAttribute& clrAttr, float32_t x, float32_t y ) const
{
	clrAttr.Clear() ;
	//
	for ( int i = 0; i < textureMaxCount; i ++ )
	{
		if ( m_pTexture[i] == NULL )
		{
			continue ;
		}
		switch ( m_flagTexture[i] & textureTypeMask )
		{
		case	textureDiffusion:
			if ( SampleDiffusionTexture
					( clrAttr.argbDiffusion,
						x, y, m_attrSurface, m_pTexture[i] ) )
			{
				clrAttr.nTextureFlags |= (1 << textureDiffusion) ;
			}
			break ;
		case	textureLuminous:
			if ( SampleDiffusionTexture
					( clrAttr.argbLuminous,
						x, y, m_attrSurface, m_pTexture[i] ) )
			{
				clrAttr.nTextureFlags |= (1 << textureLuminous) ;
				clrAttr.fpLuminousApply = m_fpTextureApply[i] ;
			}
			break ;
		case	textureNormal:
			if ( SampleDiffusionTexture
					( clrAttr.argbNormal,
						x, y, m_attrSurface, m_pTexture[i] ) )
			{
				clrAttr.nTextureFlags |= (1 << textureNormal) ;
			}
			break ;
		case	textureHeight:
			if ( SampleDiffusionTexture
					( clrAttr.argbHeight,
						x, y, m_attrSurface, m_pTexture[i] ) )
			{
				clrAttr.nTextureFlags |= (1 << textureHeight) ;
			}
			break ;
		case	textureSpecular:
			if ( SampleDiffusionTexture
					( clrAttr.argbSpecular,
						x, y, m_attrSurface, m_pTexture[i] ) )
			{
				clrAttr.nTextureFlags |= (1 << textureSpecular) ;
			}
			break ;
		}
	}
}

bool S3DMaterial::SampleDiffusionTexture
	( SGLPalette& rgbaTexture, float32_t x, float32_t y,
		const S3DSurfaceAttribute& attr, SGLImageObject * pImage )
{
	SGLImageInfo	infTexture ;
	pImage->GetImageInfo( infTexture ) ;
	if ( infTexture.depth != 32 )
	{
		return	false ;
	}
	if ( attr.flagsShading & shadingTextureSmoothing )
	{
		int	x0 = (int) floor( x ) ;
		int	y0 = (int) floor( y ) ;
		int	x1 = x0 + 1 ;
		int	y1 = y0 + 1 ;
		int	dx = eslRoundR32ToInt( (x - (float32_t) x0) * 256.0f ) ;
		int	dy = eslRoundR32ToInt( (y - (float32_t) y0) * 256.0f ) ;
		if ( attr.flagsShading & shadingTextureTiling )
		{
			int	maskWidth =
					(int) sglNormalizeScalePowerBy2( infTexture.width ) - 1 ;
			int	maskHeight =
					(int) sglNormalizeScalePowerBy2( infTexture.height ) - 1 ;
			x0 &= maskWidth ;
			y0 &= maskHeight ;
			x1 &= maskWidth ;
			y1 &= maskHeight ;
		}
		else
		{
			x0 = esl_clampi( x0, 0, (int) infTexture.width - 1 ) ;
			y0 = esl_clampi( y0, 0, (int) infTexture.height - 1 ) ;
			x1 = esl_clampi( x0, 0, (int) infTexture.width - 1 ) ;
			y1 = esl_clampi( y0, 0, (int) infTexture.height - 1 ) ;
		}
		SGLPalette	sp00, sp01, sp10, sp11 ;
		SGLImageInfo	infSample ;
		uint8_t *	pbytPixels =
			pImage->LockBuffer( infSample, SGLImageObject::lockRead ) ;
		if ( pbytPixels == NULL )
		{
			return	false ;
		}
		SGLPalette *	prgbaLine0 =
				(SGLPalette*) (pbytPixels + infSample.pitchLine * y0) ;
		SGLPalette *	prgbaLine1 =
				(SGLPalette*) (pbytPixels + infSample.pitchLine * y1) ;
		sp00 = prgbaLine0[x0] ;
		sp01 = prgbaLine0[x1] ;
		sp10 = prgbaLine1[x0] ;
		sp11 = prgbaLine1[x1] ;
		pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		//
		uint32_t	udx = (uint32_t) esl_clampi( dx, 0, 0x100 ) ;
		uint32_t	udy = (uint32_t) esl_clampi( dy, 0, 0x100 ) ;
		SGLPalette	sp0 = sp00.imul(0x100 - udx) + sp01.imul(udx) ;
		SGLPalette	sp1 = sp10.imul(0x100 - udx) + sp10.imul(udx) ;
		rgbaTexture = sp0.imul(0x100 - udy) + sp1.imul(udy) ;
		return	true ;
	}
	else
	{
		int	xTexture = eslRoundR32ToInt( x ) ;
		int	yTexture = eslRoundR32ToInt( y ) ;
		if ( attr.flagsShading & shadingTextureTiling )
		{
			xTexture &= sglNormalizeScalePowerBy2( infTexture.width ) - 1 ;
			yTexture &= sglNormalizeScalePowerBy2( infTexture.height ) - 1 ;
		}
		else
		{
			xTexture = esl_clampi( xTexture, 0, (int) infTexture.width - 1 ) ;
			yTexture = esl_clampi( yTexture, 0, (int) infTexture.height - 1 ) ;
		}
		SGLImageRect	rect( xTexture, yTexture, 1, 1 ) ;
		SGLImageInfo	infSample ;
		uint8_t *	pbytPixels =
			pImage->LockBuffer( infSample, SGLImageObject::lockRead, &rect ) ;
		if ( pbytPixels == NULL )
		{
			return	false ;
		}
		rgbaTexture = *((SGLPalette*) pbytPixels) ;
		pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		return	true ;
	}
}

// Material 取得
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
Material * S3DMaterial::GetMaterialObject( void )
{
	if ( this == NULL )
	{
		return	NULL ;
	}
	SGLMaterialBuffer *	pMaterialBuf =
		ESLTypeCast<SGLMaterialBuffer>
			( GetMaterialBuffer( materialObjectEntisGLS4Material ) ) ;
	if ( pMaterialBuf == NULL )
	{
		pMaterialBuf = new SGLMaterialBuffer ;
		AddMaterialBuffer( pMaterialBuf ) ;
		pMaterialBuf->UpdateMaterial( this ) ;
	}
	return	pMaterialBuf->m_material ;
}
#endif


//////////////////////////////////////////////////////////////////////////
// SGLImageObject - S3DMaterial 変換 SGLImageBufferInterface
// ※2D描画を3D描画へ変換するためのシェーディング無しの表面属性
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
( SakuraGL::SGLImageNoShadeMaterialInterface, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLImageNoShadeMaterialInterface::SGLImageNoShadeMaterialInterface( SGLImageObject * pImage )
{
	m_typeObject = imageObjectEntisGLS4NoShadeMaterial ;
	m_pImage = pImage ;
	//
	uint64_t	nShadingFlags[indexCount] =
	{
		shadingTextureSmoothing,
			shadingTextureSmoothing | shadingNoZBuffer,
			shadingTextureSmoothing | shadingZBufferNoWrite,
		0, shadingNoZBuffer, shadingZBufferNoWrite,
		shadingTextureSmoothing | shadingVertexAlpha,
			shadingTextureSmoothing | shadingVertexAlpha | shadingNoZBuffer,
			shadingTextureSmoothing | shadingVertexAlpha | shadingZBufferNoWrite,
		shadingTextureSmoothing | shadingMakeBlendAdd | shadingVertexAlpha,
			shadingTextureSmoothing | shadingMakeBlendAdd | shadingVertexAlpha |shadingNoZBuffer,
			shadingTextureSmoothing | shadingMakeBlendAdd | shadingVertexAlpha | shadingZBufferNoWrite,
		shadingTextureSmoothing | shadingTextureTriming
				| shadingTextureDithering | shadingVertexAlpha,
			shadingTextureSmoothing | shadingTextureTriming
				| shadingTextureDithering | shadingVertexAlpha |shadingNoZBuffer,
			shadingTextureSmoothing | shadingTextureTriming
				| shadingTextureDithering | shadingVertexAlpha | shadingZBufferNoWrite,
	} ;
	for ( int i = 0; i < indexCount; i ++ )
	{
		m_pMaterial[i] = new S3DMaterial ;
		//
		S3DSurfaceAttribute	attr ;
		attr.flagsShading =
			nShadingFlags[i]
				| shadingMethodNothing
				| shadingTextureMapping
				| shadingNoShadowObject | shadingNoReflectObject ;
		attr.colorBase.rgbMul.ui32 = 0x00FFFFFF ;
		attr.colorBase.rgbAdd.ui32 = 0 ;
		//
		m_pMaterial[i]->SetSurfaceAttribute( attr ) ;
		if ( pImage != NULL )
		{
			#if	defined(__COTOPHA__)
			m_pMaterial[i]->SetTexture( pImage ) ;
			#else
			m_pMaterial[i]->SetTexture( pImage->GetImageObject() ) ;
			#endif
		}
		m_pMaterial[i]->EnableBackSurfaceAttribute( false ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLImageNoShadeMaterialInterface::~SGLImageNoShadeMaterialInterface( void )
{
	for ( int i = 0; i < indexCount; i ++ )
	{
		delete	m_pMaterial[i] ;
		m_pMaterial[i] = NULL ;
	}
}

// 更新通知
//////////////////////////////////////////////////////////////////////////
SGLError SGLImageNoShadeMaterialInterface::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////
SGLError SGLImageNoShadeMaterialInterface::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	return	sglErrSuccess ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////
SGLError SGLImageNoShadeMaterialInterface::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////
SGLError SGLImageNoShadeMaterialInterface::MakeMipmap( void )
{
	return	sglErrSuccess ;
}

// 画像バッファの再確保通知
//////////////////////////////////////////////////////////////////////////
bool SGLImageNoShadeMaterialInterface::OnImageReBuffered( SGLImageBuffer * pImageBuf )
{
	return	false ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////
bool SGLImageNoShadeMaterialInterface::OnDestroyObject( ESLObject * pObj )
{
	return	((ESLObject*)m_pImage == pObj) ;
}

// SGLImageObject からシェーディング無しの表面属性取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * SGLImageNoShadeMaterialInterface::GetMaterialBy
	( SGLImageObject * pImage,
		uint64_t nShadingFlags, SGLImageRect * pRefRect )
{
	SGLImageNoShadeMaterialInterface *
			pNoShadeMaterial = GetNoShadeMaterialOf( pImage, pRefRect ) ;
	if ( pNoShadeMaterial == NULL )
	{
		return	NULL ;
	}
	size_t	iMaterial = indexNormal ;
	if ( nShadingFlags & shadingTextureDithering )
	{
		iMaterial = indexDither ;
	}
	else if ( nShadingFlags & shadingMakeBlendAdd )
	{
		iMaterial = indexDrawAdd ;
	}
	else if ( nShadingFlags & shadingVertexAlpha )
	{
		iMaterial = indexVertexAlpha ;
	}
	else if ( !(nShadingFlags & shadingTextureSmoothing) )
	{
		iMaterial = indexNonSmooth ;
	}
	if ( nShadingFlags & shadingNoZBuffer )
	{
		return	pNoShadeMaterial->m_pMaterial[iMaterial + indexNormalNoZ] ;
	}
	else if ( nShadingFlags & shadingZBufferNoWrite )
	{
		return	pNoShadeMaterial->m_pMaterial[iMaterial + indexNormalReadZ] ;
	}
	return	pNoShadeMaterial->m_pMaterial[iMaterial] ;
}

S3DMaterial * SGLImageNoShadeMaterialInterface::GetMaterialOf
	( SGLImageObject * pImage, SGLImageRect * pRefRect )
{
	SGLImageNoShadeMaterialInterface *
			pNoShadeMaterial = GetNoShadeMaterialOf( pImage, pRefRect ) ;
	if ( pNoShadeMaterial == NULL )
	{
		return	NULL ;
	}
	return	pNoShadeMaterial->m_pMaterial[indexNormal] ;
}

S3DMaterial * SGLImageNoShadeMaterialInterface::GetMaterialNoZOf
	( SGLImageObject * pImage, SGLImageRect * pRefRect )
{
	SGLImageNoShadeMaterialInterface *
			pNoShadeMaterial = GetNoShadeMaterialOf( pImage, pRefRect ) ;
	if ( pNoShadeMaterial == NULL )
	{
		return	NULL ;
	}
	return	pNoShadeMaterial->m_pMaterial[indexNormalNoZ] ;
}

S3DMaterial * SGLImageNoShadeMaterialInterface::GetMaterialNoWriteZOf
	( SGLImageObject * pImage, SGLImageRect * pRefRect )
{
	SGLImageNoShadeMaterialInterface *
			pNoShadeMaterial = GetNoShadeMaterialOf( pImage, pRefRect ) ;
	if ( pNoShadeMaterial == NULL )
	{
		return	NULL ;
	}
	return	pNoShadeMaterial->m_pMaterial[indexNormalReadZ] ;
}

SGLImageNoShadeMaterialInterface *
	SGLImageNoShadeMaterialInterface::GetNoShadeMaterialOf
					( SGLImageObject * pImage, SGLImageRect * pRefRect )
{
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SGLImageRect	rectRef ;
	SGLImageBufferInterface *
		pObject = pImage->CommitImageObject
					( imageObjectEntisGLS4NoShadeMaterial, rectRef, true ) ;
	if ( pObject == NULL )
	{
		pImage->AddImageObject
			( new SGLImageNoShadeMaterialInterface(pImage), false ) ;
		pObject = pImage->CommitImageObject
			( imageObjectEntisGLS4NoShadeMaterial, rectRef, true ) ;
	}
	if ( pRefRect != NULL )
	{
		*pRefRect = rectRef ;
	}
	return	ESLTypeCast<SGLImageNoShadeMaterialInterface>( pObject ) ;
}


//////////////////////////////////////////////////////////////////////////////
// シャドウマップ情報
//////////////////////////////////////////////////////////////////////////////

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadowMapInfo::SetShadowMappingInfo
	( S3DRenderContextInterface * renderShadowMap,
		const S3DLightEntry& lightShadowMap,
		const S3DDVector& vCameraTarget,
		const SGLSize& sizeDepthMap,
		double zScreen, double zDistance, double zScale,
		double zNear, double zFar,
		double zErrorPrec, double zErroSubPrec,
		double degAngleVarX, double degAngleVarY )
{
	if ( renderShadowMap == NULL )
	{
		return	sglErrInvalidParam ;
	}
	eslFillMemory( this, 0, sizeof(S3DShadowMapInfo) ) ;
	//
	S3DVector	vScreen ;
	float32_t	zCameraOffset = 0.0f ;
	vScreen.x = (float32_t) (sizeDepthMap.w * 0.5) ;
	vScreen.y = (float32_t) (sizeDepthMap.h * 0.5) ;
	vScreen.z = (float32_t) zScreen ;
	//
	S3DDVector	vLightPos, vLightArrow ;
	switch ( lightShadowMap.typeLight & lightTypeMask )
	{
	case	lightTypeVector:
		vLightArrow = lightShadowMap.vecDirection ;
		vLightArrow.Normalize() ;
		vLightPos = vCameraTarget - vLightArrow * zDistance ;
		//
		vScreen.z = 0.0f ;
		this->zPersScreen = 0.0f ;
		this->zPersScale = 1.0f / (float32_t) zScale ;
		zCameraOffset = vScreen.z ;
		break ;

	case	lightTypePoint:
	case	lightTypeSpot:
		vLightPos = lightShadowMap.vecPosition ;
		if ( (lightShadowMap.typeLight & lightTypeMask) == lightTypePoint )
		{
			vLightArrow = vCameraTarget - vLightPos ;
		}
		else
		{
			vLightArrow = lightShadowMap.vecDirection ;
		}
		vLightArrow.Normalize() ;
		//
		this->zPersScreen = vScreen.z ;
		this->zPersScale = 1.0f ;
		break ;

	default:
		return	sglErrFailed ;
	}
	S3DDMatrix	matCamera( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	if ( (degAngleVarX != 0.0) || (degAngleVarY != 0.0) )
	{
		matCamera.RevolveOnY
			( sin(degAngleVarY * PI / 180.0),
				cos(degAngleVarY * PI / 180.0) ) ;
		matCamera.RevolveOnX
			( sin(degAngleVarX * PI / 180.0),
				cos(degAngleVarX * PI / 180.0) ) ;
	}
	matCamera.RevolveByAngleOn( vLightArrow ) ;
	//
	//
	S3DDMatrix	matICamera ;
	matICamera.InverseOf( matCamera ) ;
	//
	S3DDVector	vCenter = vLightPos + vLightArrow * zCameraOffset ;
	S3DDVector	vX( 1, 0, 0 ), vY( 0, 1, 0 ), vZ( 0, 0, 1 ) ;
	matICamera.RevolveVector( vX ) ;
	matICamera.RevolveVector( vY ) ;
	matICamera.RevolveVector( vZ ) ;
	//
	this->vLight = vLightPos ;
	this->vRay = vZ ;
	this->vTarget = vCenter - (vX * vScreen.x + vY * vScreen.y) ;
	this->vAxisX = vX ;
	this->vAxisY = vY ;
	this->fpFixErrorGap = (float32_t) pow( 2.0, zErrorPrec ) ;
	this->fpVarErrorGap = (float32_t) pow( 2.0, zErroSubPrec ) ;
	this->zPersNear = esl_fminf( (float32_t) zNear, vScreen.z * 0.25f ) ;
	this->zPersFar = (float32_t) zFar ;
	//
	double	fpDistance = (vCameraTarget - vLightPos).Absolute() ;
	if ( fpDistance * 0.25 < zNear )
	{
		zNear = esl_fmax( fpDistance * 0.25, zFar * 0.000001 ) ;
		this->zPersNear = (float32_t) zNear ;
	}
	//
	renderShadowMap->SetProjectionScreen( vScreen, this->zPersScale ) ;
	renderShadowMap->SetZClipRange( this->zPersNear, this->zPersFar ) ;
	//
	matCamera.RevolveVector( vLightPos ) ;
	renderShadowMap->SetCamera( matCamera, vLightPos ) ;
	//
	return	sglErrSuccess ;
}

// カメラ行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DShadowMapInfo::GetCameraMatrix( S3DDMatrix& matCamera ) const
{
	S3DDMatrix	matICamera
		( this->vAxisX.x, this->vAxisY.x, this->vRay.x,
			this->vAxisX.y, this->vAxisY.y, this->vRay.y,
			this->vAxisX.z, this->vAxisY.z, this->vRay.z ) ;
	matCamera.InverseOf( matICamera ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 表面属性インターフェース
//////////////////////////////////////////////////////////////////////////////

#if	defined(__COTOPHA__)

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLMaterialBuffer, S3DMaterialBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMaterialBuffer::SGLMaterialBuffer( void )
{
	m_typeMaterial = materialObjectEntisGLS4Material ;
	m_flagUpdate = true ;
	m_material = new Material ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMaterialBuffer::~SGLMaterialBuffer( void )
{
	delete	m_material ;
	m_material = NULL ;
}

// 属性更新処理
//////////////////////////////////////////////////////////////////////////////
void SGLMaterialBuffer::UpdateMaterial( const S3DMaterial * pMaterial )
{
	if ( m_flagUpdate )
	{
		S3DSurfaceAttribute	sufattr ;
		pMaterial->GetSurfaceAttribute( sufattr ) ;
		m_material->SetSurfaceAttribute( sufattr ) ;
		//
		SGLImageObject *	pImage ;
		int	i ;
		for ( i = 0; i < 4; i ++ )
		{
			SGLImageObject *	pImage = pMaterial->GetTexture(i) ;
			if ( pImage != NULL )
			{
				m_material->SetTexture
					( pImage->GetImageObject(),
						i, pMaterial->m_flagTexture[i] ) ;
			}
			else
			{
				m_material->SetTexture( NULL, i ) ;
			}
		}
		m_material->SetSubTextureZ( pMaterial->GetSubTextureZ() ) ;
		//
		if ( pMaterial->IsEnabledBackSurfaceAttribute() )
		{
			m_material->EnableBackSurfaceAttribute( true ) ;
			pMaterial->GetBackSurfaceAttribute( sufattr ) ;
			m_material->SetBackSurfaceAttribute( sufattr ) ;
			//
			for ( i = 0; i < 4; i ++ )
			{
				pImage = pMaterial->GetBackTexture(i) ;
				if ( pImage != NULL )
				{
					m_material->SetBackTexture
						( pImage->GetImageObject(),
							i, pMaterial->m_flagBackTexture[i] ) ;
				}
				else
				{
					m_material->SetBackTexture( NULL, i ) ;
				}
			}
		}
		else
		{
			m_material->EnableBackSurfaceAttribute( false ) ;
		}
		m_flagUpdate = false ;
	}
}

#endif



// プリミティブ形状頂点数
//////////////////////////////////////////////////////////////////////////////
static const size_t	s_countPrimitiveVertex[primitiveCount] =
{
	1, 1, 2, 2, 3, 3,
} ;

size_t SakuraGL::GetPrimitiveVertexCount( S3DPrimitiveType typePrimitive )
{
	ESLAssert( typePrimitive >= 0 ) ;
	ESLAssert( typePrimitive < primitiveCount ) ;
	return	s_countPrimitiveVertex[typePrimitive] ;
}


//////////////////////////////////////////////////////////////////////////////
// 抽象 3D 頂点バッファ・デバイスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DVertexDeviceBufferInterface, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DVertexDeviceBufferInterface::~S3DVertexDeviceBufferInterface( void )
{
	if ( m_ptrNext != NULL )
	{
		delete	m_ptrNext ;
	}
}

// 指定タイプの S3DVertexDeviceBufferInterface 取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexDeviceBufferInterface *
	S3DVertexDeviceBufferInterface::GetDeviceBufferTypeOf( SGLImageBufferObjectType type )
{
	if ( m_typeObject == type )
	{
		return	this ;
	}
	if ( m_ptrNext != NULL )
	{
		return	m_ptrNext->GetDeviceBufferTypeOf( type ) ;
	}
	return	NULL ;
}

S3DVertexDeviceBufferInterface *
	S3DVertexDeviceBufferInterface::GetDeviceBufferAs( const ESLRuntimeClass& rtClass )
{
	if ( IsKindOf( rtClass ) )
	{
		return	this ;
	}
	if ( m_ptrNext != NULL )
	{
		return	m_ptrNext->GetDeviceBufferAs( rtClass ) ;
	}
	return	NULL ;
}

// リストの次に追加
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::AddNextVertexDeviceBuffer
					( S3DVertexDeviceBufferInterface * pVDB )
{
	ESLAssert( m_ptrNext == NULL ) ;
	m_ptrNext = pVDB ;
}

// リストの次の S3DVertexDeviceBufferInterface を分離
//////////////////////////////////////////////////////////////////////////////
S3DVertexDeviceBufferInterface *
	S3DVertexDeviceBufferInterface::DetachNextVertexDeviceBuffer( void )
{
	S3DVertexDeviceBufferInterface *	pvdbNext = m_ptrNext ;
	if ( pvdbNext != NULL )
	{
		m_ptrNext = pvdbNext->m_ptrNext ;
		pvdbNext->m_ptrNext = NULL ;
	}
	else
	{
		ESLAssert( m_ptrNext == NULL ) ;
	}
	return	pvdbNext ;
}

// 描画の確定時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnFlush( S3DVertexBufferInterface * pVB )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnFlush( pVB ) ;
	}
}

// プリミティブリストを更新時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnUpdateIndexedPrimitiveList
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnUpdateIndexedPrimitiveList
			( pVB, iMesh, nFlags, countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
}

// サブメッシュ（ポリゴンリスト）を更新時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnUpdateSubIndexedTriangleList
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t iSubMesh, uint32_t nFlags,
		size_t countPolygon, const uint32_t * pIndexedList )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnUpdateSubIndexedTriangleList
			( pVB, iMesh, iSubMesh, nFlags, countPolygon, pIndexedList ) ;
	}
}

// 追加的な頂点属性を設定時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnSetExtendVertexAttribute
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t countElements,
		size_t countVertex, const float32_t * pfpAttrElements )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnSetExtendVertexAttribute
			( pVB, iMesh, countElements, countVertex, pfpAttrElements ) ;
	}
}

// メッシュにウェイトマップを設定時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnSetBoneWeightMap
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnSetBoneWeightMap( pVB, iMesh, nCount, ppWeightMaps ) ;
	}
}

// メッシュにモーフターゲット枠を確保時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnAllocateMorphing
	( S3DVertexBufferInterface * pVB, size_t iMesh, size_t nCount )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnAllocateMorphing( pVB, iMesh, nCount ) ;
	}
}

// メッシュにモーフターゲットを設定時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnSetMorphingTargetMesh
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t iMorph, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnSetMorphingTargetMesh
			( pVB, iMesh, iMorph, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
}

// バッファ消去時処理
//////////////////////////////////////////////////////////////////////////////
void S3DVertexDeviceBufferInterface::OnClearBuffer( S3DVertexBufferInterface * pVB )
{
	if ( m_ptrNext != NULL )
	{
		m_ptrNext->OnClearBuffer( pVB ) ;
	}
}




//////////////////////////////////////////////////////////////////////////////
// 抽象 3D 頂点バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DVertexVariantBuffer, SObject )
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DVertexBufferInterface,
		S3DRenderBufferInterface, S3DVertexVariantBuffer )

// 頂点オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface *
	S3DVertexBufferInterface::NewBuffer( SGLPaintContextType type )
{
	if ( type == typePaintDefault )
	{
		type = S3DRenderContextInterface::GetDefaultRenderType() ;
	}
#if	defined(__COTOPHA__)
	return	new S3DVertexBuffer
				( VertexBuffer::NewBuffer( type ), true ) ;
#else
	switch ( type )
	{
	case	typePaintOpenGL:
		{
			S3DRenderVertexBuffer *	pVB = new S3DRenderVertexBuffer ;
//			SGLOpenGLVertexBuffer *	pglVB = new SGLOpenGLVertexBuffer( pVB ) ;
//			pVB->AttachDeviceBuffer( pglVB ) ;
			return	pVB ;
		}

	default:
		break ;
	}
	return	new SakuraGL::S3DRenderVertexBuffer ;
#endif
}

// バッファ制御フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DVertexBufferInterface::GetBufferControlFlags( void ) const
{
	return	0 ;
}

void S3DVertexBufferInterface::SetBufferControlFlags( uint32_t nFlags )
{
}

// デフォルトマテリアル
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DVertexBufferInterface::GetDefaultMaterial( void ) const
{
	return	S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ) ;
}

void S3DVertexBufferInterface::AttachDefaultMaterial( S3DMaterial * pMaterial )
{
}

// プリミティブを追加するためのバッファを確保する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::AllocatePrimitiveBuffer
	( S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	S3DVector4 *	pvBuf =
		(S3DVector4*) esl_malloc
			( countVertex * (sizeof(S3DVector4) * 2
							+ sizeof(S2DVector) + sizeof(S3DColor)) ) ;
	prmbuf.pvVertex = pvBuf ;
	prmbuf.pvNormal = pvBuf + countVertex ;
	prmbuf.pvUVMap = (S2DVector*) (pvBuf + countVertex * 2) ;
	prmbuf.pColor = (S3DColor*) (prmbuf.pvUVMap + countVertex) ;
	//
	if ( countIndex > 0 )
	{
		prmbuf.pIndexedList =
			(uint32_t*) esl_malloc( countIndex * sizeof(uint32_t) ) ;
	}
	else
	{
		prmbuf.pIndexedList = NULL ;
	}
	return	sglErrSuccess ;
}

// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::AddPrimitiveBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		size_t countIndex, size_t countVertex )
{
	SGLError	err =
		AddIndexedPrimitiveList
			( pMaterial, nFlags, typePrimitive,
				countIndex, countVertex,
				prmbuf.pvVertex, prmbuf.pvNormal,
				prmbuf.pvUVMap, prmbuf.pColor, prmbuf.pIndexedList ) ;
	FreePrimitiveBuffer( prmbuf ) ;
	return	err ;
}

// プリミティブを追加せずにバッファを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::FreePrimitiveBuffer
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf )
{
	if ( prmbuf.pvVertex != NULL )
	{
		esl_free( prmbuf.pvVertex ) ;
	}
	if ( prmbuf.pIndexedList != NULL )
	{
		esl_free( prmbuf.pIndexedList ) ;
	}
	return	sglErrSuccess ;
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::RenderBufferTo
	( S3DRenderBufferInterface * render,
		uint64_t flagsExclusion, size_t iFrist, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing ) const
{
	return	sglErrFailed ;
}

// モデル描画が表示範囲にあるか見積もる
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBufferInterface::IsModelIntoView
	( S3DRenderContextInterface * render,
			float32_t fpScaleMargin, float32_t fpModelMargin )
{
	S3DVector	vCenter ;
	double	r = GetCircumscribedSphere( vCenter ) ;
	//
	S3DDVector	vdCenter = vCenter ;
	return	render->IsSphereIntoView
				( vdCenter, r * (1.0 + fpScaleMargin) + fpModelMargin ) ;
}

// バッファを消去
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBufferInterface::ClearBuffer( void )
{
}

// デバイスリソースを解放
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBufferInterface::ReleaseAllDeviceResources( void )
{
}

// バッファのメモリブロックサイズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBufferInterface::SetBufferUnitSize( size_t nBytes )
{
}

// メッシュ外接直方体取得
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBufferInterface::GetCircumscribedParallelepiped
								( S3DVector& vMin, S3DVector& vMax )
{
	SArray<S3DVector4>	bufTemp ;
	size_t	nCount = GetMeshCount() ;
	bool	fParallelepiped = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MeshInfo	info ;
		eslFillMemory( &info, 0, sizeof(MeshInfo) ) ;
		if ( !GetMeshInfoAt( info, i, 0 ) )
		{
			info.pvVertex = bufTemp.GetArray( info.countVertex ) ;
			if ( (info.countVertex > 0)
				&& !GetMeshInfoAt( info, i, info.countVertex ) )
			{
				S3DVector4	vMinTemp, vMaxTemp ;
				MinMaxVector4DArray
					( vMinTemp, vMaxTemp, info.pvVertex, info.countVertex ) ;
				if ( !fParallelepiped )
				{
					vMin = vMinTemp ;
					vMax = vMaxTemp ;
					fParallelepiped = true ;
				}
				else
				{
					vMin.x = esl_fminf( vMin.x, vMinTemp.x ) ;
					vMin.y = esl_fminf( vMin.y, vMinTemp.y ) ;
					vMin.z = esl_fminf( vMin.z, vMinTemp.z ) ;
					vMax.x = esl_fmaxf( vMax.x, vMaxTemp.x ) ;
					vMax.y = esl_fmaxf( vMax.y, vMaxTemp.y ) ;
					vMax.z = esl_fmaxf( vMax.z, vMaxTemp.z ) ;
				}
			}
			bufTemp.FinishArray() ;
		}
	}
	return	fParallelepiped ;
}

// 総ポリゴン数集計
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBufferInterface::CountOfTotalPolygons( void ) const
{
	size_t	nTotal = 0 ;
	size_t	nCount = GetMeshCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MeshInfo	info ;
		eslFillMemory( &info, 0, sizeof(MeshInfo) ) ;
		if ( !GetMeshInfoAt( info, i, 0 ) )
		{
			nTotal += info.countPrimitive ;
		}
	}
	return	nTotal ;
}

// 総頂点数集計
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBufferInterface::CountOfTotalVertices( void ) const
{
	size_t	nTotal = 0 ;
	size_t	nCount = GetMeshCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MeshInfo	info ;
		eslFillMemory( &info, 0, sizeof(MeshInfo) ) ;
		if ( !GetMeshInfoAt( info, i, 0 ) )
		{
			nTotal += info.countVertex ;
		}
	}
	return	nTotal ;
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::EnableMultiInstancingMode( bool flagEnable )
{
	return	sglErrNotSupported ;
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBufferInterface::IsMultiInstancingMode( void ) const
{
	return	false ;
}

// インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBufferInterface::GetInstancingCount( void ) const
{
	return	0 ;
}

// インスタンス取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBufferInterface::GetInstancingEntries
	( S3DVertexVariantBuffer** ppVVB,
		S4DMatrix * pmatInstance,
		S3DColor * pcolorInstance,
		size_t iFirst, size_t nCount ) const
{
	return	0 ;
}

// インスタンス全消去
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::ClearAllInstance( void )
{
	return	sglErrNotSupported ;
}

// インスタンス追加設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBufferInterface::AddInstanceVariant
	( S3DVertexVariantBuffer * pVVB,
		const S4DMatrix & matInstance,
		const S3DColor & colorInstance )
{
	return	sglErrNotSupported ;
}

// VertexBuffer 取得
//////////////////////////////////////////////////////////////////////////////
VertexBuffer * S3DVertexBufferInterface::GetVertexBufferObject( void ) const
{
#if	defined(__COTOPHA__)
	return	NULL ;
#else
	return	(VertexBuffer*) this ;
#endif
}


//////////////////////////////////////////////////////////////////////////////
// 抽象 3D レンダリング・コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DRenderBufferInterface, SObject )
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRenderContextInterface,
		SGLPaintContextInterface, S3DRenderBufferInterface )

SGLPaintContextType	S3DRenderContextInterface::m_typeDefaultRender =
#if	defined(DISABLE_ENTIS_GLS3)
	typePaintOpenGL ;
#else
	typePaintEntisGLS ;
#endif

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferInterface::SetCustomShaderUniformInt
	( const wchar_t * pszUniform,
		const int32_t * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformInt, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformFloat
	( const wchar_t * pszUniform,
		const float32_t * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformFloat, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformVector2D
	( const wchar_t * pszUniform,
		const S2DVector * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformVector2D, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformVector3D
	( const wchar_t * pszUniform,
		const S3DVector * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformVector3D, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformVector4D
	( const wchar_t * pszUniform,
		const S4DVector * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformVector4D, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformMatrix3x3
	( const wchar_t * pszUniform,
		const S3DMatrix * pData, size_t nCount )
{
	SArray<float32_t>	mat3 ;
	float32_t *			pmat3 = mat3.GetArray( nCount * (3 * 3) ) ;
	for ( size_t j = 0; j < nCount; j ++ )
	{
		for ( size_t i = 0; i < 3; i ++ )
		{
			pmat3[0] = pData->m[i][0] ;
			pmat3[1] = pData->m[i][1] ;
			pmat3[2] = pData->m[i][2] ;
			pmat3 += 3 ;
		}
		pData ++ ;
	}
	mat3.FinishArray() ;
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformMatrix3x3,
								mat3.GetConstArray(), nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformMatrix4x4
	( const wchar_t * pszUniform,
		const S4DMatrix * pData, size_t nCount )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformMatrix4x4, pData, nCount ) ;
}

SGLError S3DRenderBufferInterface::SetCustomShaderUniformTexture
	( const wchar_t * pszUniform, SGLImageObject * pTexture )
{
	return	SetCustomShaderUniform
		( pszUniform, S3DCustomShader::uniformTexture, &pTexture, 1 ) ;
}


// 描画オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface *
	S3DRenderContextInterface::NewContext( SGLPaintContextType type )
{
	if ( type == typePaintDefault )
	{
		type = m_typeDefaultRender ;
	}
#if	defined(__COTOPHA__)
	return	new S3DRenderContext
				( RenderContext::NewContext( type ), true ) ;
#else
	switch ( type )
	{
	case	typePaintOpenGL:
		return	new S3DOpenGLBufferedRenderer
						( SGLOpenGLContext::GetDefault() ) ;

	case	typePaintEntisGLS:
		if ( m_typeDefaultRender != typePaintEntisGLS4 )
		{
		#if	!defined(__PLATFORM_WINDOWS__) || !defined(__ENTIS_GLS__)
			return	new S3DHybridRenderContext
				( new SakuraGL::SGLStandardRenderContext, new SGLPaintBuffer ) ;
		#else
			return	new SakuraGL::SGLStandardRenderContext ;
		#endif
		}

	case	typePaintEntisGLS4:
		return	new SakuraGL::S3DSoftwareBufferedRenderer ;

	default:
		break ;
	}
	return	new SakuraGL::SGLStandardRenderContext ;
#endif
}

// 注視点と視点指定による標準的なカメラの設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContextInterface::SetCameraAngle
	( const S3DDVector& posTarget,
		const S3DDVector& posView, double zAngle )
{
	S3DDMatrix	matCamera( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posCamera = posView ;
	S3DDVector	vAngle = posTarget - posView ;
	//
	zAngle *= SSystem::PI / 180.0 ;
	matCamera.RevolveOnZ( sin( zAngle ), cos( zAngle ) ) ;
	matCamera.RevolveByAngleOn( vAngle ) ;
	//
	matCamera.RevolveVector( posCamera ) ;
	SetCamera( matCamera, posCamera ) ;
}

// 注視点と視点指定、上ベクトルによるカメラの設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContextInterface::SetCameraAngleVector
	( const S3DDVector& posTarget,
			const S3DDVector& posView, const S3DDVector& vAngleTop )
{
	S3DDMatrix	matCamera ;
	S3DDVector	posCamera ;
	posCamera = matCamera.CameraAngleOf( posTarget, posView, vAngleTop ) ;
	SetCamera( matCamera, posCamera ) ;
}

// カメラの逆変換行列を現在の座標空間に設定する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContextInterface::SetInverseCameraTransformation
	( const S3DColor * color, unsigned int nTransparency )
{
	S3DDMatrix	matCamera ;
	S3DDVector	vCameraPos ;
	GetCamera( matCamera, vCameraPos ) ;
	//
	S3DDMatrix	matICamera ;
	S3DDVector	vPos ;
	matICamera.InverseOf( matCamera ) ;
	vPos = matICamera * vCameraPos ;
	SetMatrixTransformation( matICamera, vPos, color, nTransparency ) ;
}

// 球（ローカル座標）が視界に収まるか判定する
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderContextInterface::IsSphereIntoView( const S3DDVector& vPos, double radius ) const
{
	//
	// パラメータ収集
	//
	SGLImageRect	rectView ;
	S3DVector		vScreen ;
	double			zScale, fpPixelAspect ;
	S3DDMatrix		matCamera, matModel ;
	S3DDVector		vCamera, vModel ;
	//
	GetViewPort( rectView ) ;
	GetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
	GetCamera( matCamera, vCamera ) ;
	GetMatrixTransformation( matModel, vModel ) ;
	//
	// 座標変換
	//
	S3DDVector	vProjPos = vPos ;
	matModel.RevolveVector( vProjPos ) ;
	vProjPos += vModel ;
	matCamera.RevolveVector( vProjPos ) ;
	vProjPos -= vCamera ;
	//
	if ( vScreen.z != 0.0f )
	{
		//
		// 視界境界面法線
		//
		S3DDVector	vFrame[5] ;		// 背上下左右枠
		vScreen.z *= (float32_t) zScale ;
		vFrame[0] = S3DDVector( 0, 0, -1 ) ;
		vFrame[1] =
			S3DDVector( 0, - vScreen.z, (rectView.y - vScreen.y) ) ;
		vFrame[2] =
			S3DDVector( 0, vScreen.z, - (rectView.y + rectView.h - vScreen.y) ) ;
		vFrame[3] =
			S3DDVector( - vScreen.z, 0,
						(rectView.x - vScreen.x) * fpPixelAspect ) ;
		vFrame[4] =
			S3DDVector( vScreen.z, 0,
						- (rectView.x + rectView.w - vScreen.x) * fpPixelAspect ) ;
		//
		// 視界からのはみ出しを判定する
		//
		S3DDMatrix	matISpace ;
		bool		flagISpace = false ;
		double		r ;
		for ( int i = 0; i < 5; i ++ )
		{
			vFrame[i].Normalize() ;
			r = vFrame[i].InnerProduct( vProjPos ) ;
			if ( r > 0 )
			{
				//
				// はみ出し距離に相当するベクトルをローカル空間に変換し
				// ローカル空間でのはみ出し距離を算出し比較する
				//
				S3DDVector	vLocal = vFrame[i] * r ;
				if ( !flagISpace )
				{
					matISpace.InverseOf( matCamera * matModel ) ;
					flagISpace = true ;
				}
				matISpace.RevolveVector( vLocal ) ;
				r = vLocal.Absolute() ;
				if ( r > radius )
				{
					// 枠外
					return	false ;
				}
			}
		}
	}
	else
	{
		double	xProj = vProjPos.x * zScale / fpPixelAspect + vScreen.x ;
		double	yProj = vProjPos.y * zScale + vScreen.y ;
		double	wRadius = radius * zScale / fpPixelAspect ;
		double	hRadius = radius * zScale ;
		if ( (xProj + wRadius < rectView.x)
			|| (rectView.x + rectView.w < xProj - wRadius)
			|| (yProj + hRadius < rectView.y)
			|| (rectView.y + rectView.h < yProj - hRadius) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 座標（ローカル）の透視変換
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderContextInterface::GetProjectedPosition
	( S2DDVector& vProjPos, const S3DDVector& vPos, double fpLimit ) const
{
	//
	// パラメータ収集
	//
	SGLImageRect	rectView ;
	S3DVector		vScreen ;
	double			zScale, fpPixelAspect ;
	S3DDMatrix		matCamera, matModel ;
	S3DDVector		vCamera, vModel ;
	//
	GetViewPort( rectView ) ;
	GetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
	GetCamera( matCamera, vCamera ) ;
	GetMatrixTransformation( matModel, vModel ) ;
	//
	// 座標変換
	//
	S3DDVector	vTranPos = vPos ;
	matModel.RevolveVector( vTranPos ) ;
	vTranPos += vModel ;
	matCamera.RevolveVector( vTranPos ) ;
	vTranPos -= vCamera ;
	//
	if ( vScreen.z != 0.0f )
	{
		if ( (vTranPos.z <= 0)
			|| (fabs(vTranPos.x * vScreen.z) > fpLimit * vTranPos.z)
			|| (fabs(vTranPos.y * vScreen.z) > fpLimit * vTranPos.z) )
		{
			return	false ;
		}
		zScale *= vScreen.z / vTranPos.z ;
	}
	vProjPos.x = vTranPos.x * zScale / fpPixelAspect + vScreen.x ;
	vProjPos.y = vTranPos.y * zScale + vScreen.y ;
	return	true ;
}

#if	defined(__COTOPHA__)
// RenderContext 取得
//////////////////////////////////////////////////////////////////////////////
RenderContext * S3DRenderContextInterface::GetRenderContextObject( void ) const
{
	return	NULL ;
}
#else

// ハードウェア描画オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * S3DRenderContextInterface::GetRenderDeviceObject( uint64_t nFlags )
{
	return	NULL ;
}

// ハードウェア描画オブジェクト変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContextInterface::SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags )
{
	return	sglErrFailed ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// VertexBuffer - S3DVertexBufferInterface インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DVertexBuffer, S3DVertexBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DVertexBuffer::S3DVertexBuffer( void )
{
	m_buffer = VertexBuffer::NewBuffer
					( S3DRenderContextInterface::GetDefaultRenderType() ) ;
	m_flagOwner = true ;
}

S3DVertexBuffer::S3DVertexBuffer( VertexBuffer * buffer, bool flagOwner )
{
	m_buffer = buffer ;
	m_flagOwner = flagOwner ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DVertexBuffer::~S3DVertexBuffer( void )
{
	if ( m_flagOwner )
	{
		delete	m_buffer ;
		m_buffer = NULL ;
		m_flagOwner = false ;
	}
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::AttachVertexBuffer( VertexBuffer * buffer, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_buffer ;
		m_buffer = NULL ;
		m_flagOwner = false ;
	}
	m_buffer = buffer ;
	m_flagOwner = flagOwner ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::SetShadingFlag( uint64_t nShadingMethod )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->SetShadingFlag( nShadingMethod ) ;
}

// シェーディング取得
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DVertexBuffer::GetShadingFlag( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetShadingFlag() ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AttachCustomShader( S3DCustomShader * pShader )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AttachCustomShader( pShader ) ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DVertexBuffer::GetCustomShader( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetCustomShader() ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AppendMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AppendMatrixTransformation( mat, pos, color, nTransparency ) ;
}

SGLError S3DVertexBuffer::SetMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetMatrixTransformation( mat, pos, color, nTransparency ) ;
}

SGLError S3DVertexBuffer::GetMatrixTransformation
	( S3DDMatrix& mat, S3DDVector& pos,
		S3DColor * color, unsigned int * pTransparency ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetMatrixTransformation( mat, pos, color, pTransparency ) ;
}

SGLError S3DVertexBuffer::PushTransformation( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->PushTransformation() ;
}

SGLError S3DVertexBuffer::PopTransformation( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->PopTransformation() ;
}

SGLError S3DVertexBuffer::ResetTransformation( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->ResetTransformation() ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
		S3DCustomShader::UniformType type,
			const void * pData, size_t nCount )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetCustomShaderUniform
					( pwszUniformId, type, pData, nCount ) ;
}

SGLError S3DVertexBuffer::ResetCustomShaderUniform( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->ResetCustomShaderUniform() ;
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::SetOffsetBorderColor( uint32_t rgbBorder )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->SetOffsetBorderColor( rgbBorder ) ;
}

// 輪郭描画オフセット係数設定 (ax+b)
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->SetOffsetBorderCoefficient( a, b ) ;
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetOptionalFeature
	( FeatureType feature, int32_t nParam1,
				const void * pParam2, size_t sizeOfParam2 )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetOptionalFeature
				( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// オプショナル機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::GetOptionalFeature
	( FeatureType feature, int32_t nParam1,
				void * pParam2, size_t sizeOfParam2 ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetOptionalFeature
				( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddIndexedTriangleList
		( pMaterial->GetMaterialObject(), nFlags,
			countPolygon, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddTriangleStrip
		( pMaterial->GetMaterialObject(), nFlags,
			countTriangleStrip,
			pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddIndexedPrimitiveList
		( pMaterial->GetMaterialObject(), nFlags,
			typePrimitive, countIndex, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// プリミティブを追加するためのバッファを確保する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AllocatePrimitiveBuffer
	( S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AllocatePrimitiveBuffer
				( prmbuf, typePrimitive, countIndex, countVertex ) ;
}

// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddPrimitiveBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
		size_t countIndex, size_t countVertex )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddPrimitiveBuffer
				( pMaterial, nFlags, typePrimitive, prmbuf, countIndex, countVertex ) ;
}

// プリミティブを追加せずにバッファを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::FreePrimitiveBuffer
	( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->FreePrimitiveBuffer( prmbuf ) ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddVertexBuffer
		( pMaterial->GetMaterialObject(), nFlags,
			pBuffer->GetVertexBufferObject(), iFirst, iEnd,
			nInstancing, pmatInstancing, pColorInstancing ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::Flush( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->Flush() ;
}

// 遅延削除オブジェクト追加（Flush 時に削除）
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::AddTemporaryObject( ESLObject * pObj )
{
	m_arrayTemporary.Add( pObj ) ;
}

// バッファ制御フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DVertexBuffer::GetBufferControlFlags( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetBufferControlFlags() ;
}

void S3DVertexBuffer::SetBufferControlFlags( uint32_t nFlags )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->SetBufferControlFlags( nFlags ) ;
}

// デフォルトマテリアル
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DVertexBuffer::GetDefaultMaterial( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetDefaultMaterial() ;
}

void S3DVertexBuffer::AttachDefaultMaterial( S3DMaterial * pMaterial )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->AttachDefaultMaterial( pMaterial ) ;
}

// メッシュ数を取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBuffer::GetMeshCount( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetMeshCount() ;
}

// メッシュ情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::GetMeshInfoAt
	( S3DVertexBufferInterface::MeshInfo& info, size_t iMesh,
		size_t nCopyVertices, size_t iFirstVertex, uint32_t nFlags ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetMeshInfoAt
			( *((VertexBuffer::MeshInfo*)&info),
					iMesh, nCopyVertices, iFirstVertex, nFlags ) ;
}

// ポリゴンリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::UpdateIndexedTriangleList
	( size_t iMesh, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->UpdateIndexedTriangleList
				( iMesh, nFlags, countPolygon, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::UpdateTriangleStrip
	( size_t iMesh, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->UpdateTriangleStrip
				( iMesh, nFlags, countTriangleStrip,
					pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::UpdateIndexedPrimitiveList
	( size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->UpdateIndexedPrimitiveList
				( iMesh, nFlags, countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// サブメッシュ（ポリゴンリスト）を更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::UpdateSubIndexedTriangleList
	( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
		size_t countPolygon, const uint32_t * pIndexedList )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->UpdateSubIndexedTriangleList
				( iMesh, iSubMesh, nFlags, countPolygon, pIndexedList ) ;
}

// サブメッシュ切り替えｚ座標比を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetSubMeshDensity
	( size_t iMesh, float32_t fpDensity, ssize_t iSelector )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetSubMeshDensity( iMesh, fpDensity, iSelector ) ;
}

// 追加的な頂点属性を設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetExtendVertexAttribute
	( size_t iMesh, size_t countElements,
		size_t countVertex, const float32_t * pfpAttrElements )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetExtendVertexAttribute
				( iMesh, countElements, countVertex, pfpAttrElements ) ;
}

// メッシュにウェイトマップを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetBoneWeightMap
	( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetBoneWeightMap( iMesh, nCount, ppWeightMaps ) ;
}

// メッシュにジョイントマップを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetBoneJointMap
	( size_t iMesh, size_t nBoneCount,
		size_t nJointCount, const uint32_t ** ppJointMaps )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetBoneJointMap
				( iMesh, nBoneCount, nJointCount, ppJointMaps ) ;
}

// メッシュにボーン行列設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetBoneMatrix
	( size_t iMesh, size_t nCount,
		const S3DMatrix * pMatrix, const S3DVector * pTrans )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetBoneMatrix( iMesh, nCount, pMatrix, pTrans ) ;
}

// メッシュのボーン行列取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBuffer::GetBoneMatrix
	( size_t iMesh, size_t nCount,
		S3DMatrix * pMatrix, S3DVector * pTrans )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetBoneMatrix( iMesh, nCount, pMatrix, pTrans ) ;
}

// メッシュにモーフターゲット枠を確保
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AllocateMorphing( size_t iMesh, size_t nCount )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AllocateMorphing( iMesh, nCount ) ;
}

// メッシュにモーフターゲットを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetMorphingTargetMesh
	( size_t iMesh, size_t iMorph, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetMorphingTargetMesh
				( iMesh, iMorph, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// メッシュのモーフターゲットにウェイトを設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetMorphingTargetWeight
	( size_t iMesh, size_t iMorph,
		size_t countVertex, const float32_t * pfpWeight )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetMorphingTargetWeight
				( iMesh, iMorph, countVertex, pfpWeight ) ;
}

// モーフィング設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetMorphingApplication
	( size_t iMesh, const ssize_t * pTargetMesh,
			const float32_t * pApplication, size_t nTargetMeshCount )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetMorphingApplication
				( iMesh, pTargetMesh, pApplication, nTargetMeshCount ) ;
}

// モーフィング設定取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::GetMorphingApplication
	( size_t iMesh, ssize_t& iTargetMesh,
			float32_t& fpApplication, size_t iTargetMeshIndex )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetMorphingApplication
				( iMesh, iTargetMesh, fpApplication, iTargetMeshIndex ) ;
}

// メッシュ表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::EnableToRenderMesh
	( size_t iFirst, ssize_t iEnd, bool fEnable )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->EnableToRenderMesh( iFirst, iEnd, fEnable ) ;
}

// メッシュ表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBuffer::IsEnabledToRenderMesh( size_t iMesh ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->IsEnabledToRenderMesh( iMesh ) ;
}

// メッシュマテリアル設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::SetMaterialToRenderMesh
	( size_t iMesh, S3DMaterial * pMaterial )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->SetMaterialToRenderMesh( iMesh, pMaterial ) ;
}

// メッシュマテリアル取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DVertexBuffer::GetMaterialToRenderMesh( size_t iMesh ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetMaterialToRenderMesh( iMesh ) ;
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
		uint64_t flagsExclusion, size_t iFrist, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing ) const
{
	ESLAssert( m_buffer != NULL ) ;
#if	defined(__COTOPHA__)
	S3DRenderContextInterface *	prci =
			ESLTypeCast<S3DRenderContextInterface>( render ) ;
	if ( prci != NULL )
	{
		RenderContext *	prc = prci->GetRenderContextObject() ;
		if ( prc != NULL )
		{
			return	m_buffer->RenderBufferTo
				( prc, flagsExclusion, iFrist, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
		}
	}
	return	sglErrFailed ;
#else
	return	m_buffer->RenderBufferTo
				( render, flagsExclusion, iFrist, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
#endif
}

// バッファを消去
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::ClearBuffer( void )
{
	if ( m_buffer != NULL )
	{
		m_buffer->ClearBuffer() ;
	}
	m_arrayTemporary.RemoveAll() ;
}

// デバイスリソースを解放
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::ReleaseAllDeviceResources( void )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->ReleaseAllDeviceResources() ;
}

// バッファのメモリブロックサイズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::SetBufferUnitSize( size_t nBytes )
{
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->SetBufferUnitSize( nBytes ) ;
}

// メッシュ外接球取得
//////////////////////////////////////////////////////////////////////////////
double S3DVertexBuffer::GetCircumscribedSphere( S3DVector& vCenter )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetCircumscribedSphere( vCenter ) ;
}

// メッシュ外接直方体取得
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBuffer::GetCircumscribedParallelepiped
					( S3DVector& vMin, S3DVector& vMax )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetCircumscribedParallelepiped( vMin, vMax ) ;
}

// 現在の設定に適合する S3DVertexVariantBuffer を生成
//////////////////////////////////////////////////////////////////////////////
S3DVertexVariantBuffer * S3DVertexBuffer::CreateVariantBuffer( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->CreateVariantBuffer() ;
}

// S3DVertexVariantBuffer のパラメータを VertexBuffer へ反映
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::UpdateVertexVariant
	( S3DVertexVariantBuffer * pVVB, size_t iFirst, ssize_t iEnd )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->UpdateVertexVariant( pVVB, iFirst, iEnd ) ;
}

// 参照バリアントを生成
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DVertexBuffer::NewReferenceVariantBuffer( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->NewReferenceVariantBuffer() ;
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::EnableMultiInstancingMode( bool flagEnable )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->EnableMultiInstancingMode( flagEnable ) ;
}

// マルチインスタンス描画モード設定
//////////////////////////////////////////////////////////////////////////////
bool S3DVertexBuffer::IsMultiInstancingMode( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->IsMultiInstancingMode() ;
}

// インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBuffer::GetInstancingCount( void ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetInstancingCount() ;
}

// インスタンス取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DVertexBuffer::GetInstancingEntries
	( S3DVertexVariantBuffer** ppVVB,
		S4DMatrix * pmatInstance,
		S3DColor * pcolorInstance,
		size_t iFirst, size_t nCount ) const
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetInstancingEntries
				( ppVVB, pmatInstance, pcolorInstance, iFirst, nCount ) ;
}

// インスタンス全消去
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::ClearAllInstance( void )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->ClearAllInstance() ;
}

// インスタンス追加設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVertexBuffer::AddInstanceVariant
	( S3DVertexVariantBuffer * pVVB,
		const S4DMatrix & matInstance,
		const S3DColor & colorInstance )
{
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->AddInstanceVariant( pVVB, matInstance, colorInstance ) ;
}

// VertexBuffer 取得
//////////////////////////////////////////////////////////////////////////////
VertexBuffer * S3DVertexBuffer::GetVertexBufferObject( void ) const
{
	return	m_buffer ;
}

// S3DVertexDeviceBufferInterface 取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexDeviceBufferInterface *
	S3DVertexBuffer::GetDeviceBufferTypeOf( SGLImageBufferObjectType type )
{
#if	defined(__COTOPHA__)
	return	NULL ;
#else
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetDeviceBufferTypeOf( type ) ;
#endif
}

S3DVertexDeviceBufferInterface *
	S3DVertexBuffer::GetDeviceBufferAs( const ESLRuntimeClass& rtClass )
{
#if	defined(__COTOPHA__)
	return	NULL ;
#else
	ESLAssert( m_buffer != NULL ) ;
	return	m_buffer->GetDeviceBufferAs( rtClass ) ;
#endif
}

// S3DVertexDeviceBufferInterface 追加
//////////////////////////////////////////////////////////////////////////////
void S3DVertexBuffer::AttachDeviceBuffer( S3DVertexDeviceBufferInterface * pDevBuf )
{
#if	!defined(__COTOPHA__)
	ESLAssert( m_buffer != NULL ) ;
	m_buffer->AttachDeviceBuffer( pDevBuf ) ;
#endif
}




//////////////////////////////////////////////////////////////////////////////
// RenderContext - S3DRenderContextInterface インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DRenderContext, S3DRenderContextInterface )
#else
ESL_IMPLEMENT_CLASS_INFO_CAST( SakuraGL::S3DRenderContext, S3DRenderContextInterface, m_render )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderContext::S3DRenderContext( void )
{
	m_render = RenderContext::NewContext( GetDefaultRenderType() ) ;
	m_flagOwner = true ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

S3DRenderContext::S3DRenderContext( RenderContext * render, bool flagOwner )
{
	m_render = render ;
	m_flagOwner = flagOwner ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderContext::~S3DRenderContext( void )
{
	if ( m_flagOwner )
	{
		delete	m_render ;
		m_render = NULL ;
		m_flagOwner = false ;
	}
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::AttachRenderContext( RenderContext * render, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_render ;
		m_render = NULL ;
		m_flagOwner = false ;
	}
	m_render = render ;
	m_flagOwner = flagOwner ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DRenderContext::GetTargetImage( void )
{
	if ( m_pTarget != NULL )
	{
		return	m_pTarget ;
	}
	ESLAssert( m_render != NULL ) ;
	m_imgTarget.SetImageObject( m_render->GetTargetImage(), false ) ;
	return	&m_imgTarget ;
}

SGLImageObject * S3DRenderContext::GetTargetZBuffer( void )
{
	if ( m_pZBuffer != NULL )
	{
		return	m_pZBuffer ;
	}
	ESLAssert( m_render != NULL ) ;
	m_imgZBuffer.SetImageObject( m_render->GetTargetZBuffer(), false ) ;
	return	&m_imgZBuffer ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::GetViewPort( SGLImageRect & rctView ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetViewPort( rctView ) ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pTarget = pImage ;
	m_pZBuffer = pZBuffer ;
	//
	Image *	imgTarget = NULL ;
	Image *	imgZBuffer = NULL ;
	if ( pImage != NULL )
	{
		imgTarget = pImage->GetImageObject() ;
		if ( imgTarget == NULL )
		{
			Trace( "failed to GetImageObject for target "
					"at S3DRenderContext::AttachTargetImage\n" ) ;
		}
	}
	if ( pZBuffer != NULL )
	{
		imgZBuffer = pZBuffer->GetImageObject() ;
		if ( imgZBuffer == NULL )
		{
			Trace( "failed to GetImageObject for z buffer "
					"at S3DRenderContext::AttachTargetImage\n" ) ;
		}
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->AttachTargetImage( imgTarget, imgZBuffer, pView ) ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::DetachTargetImage( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	//
	ESLAssert( m_render != NULL ) ;
	return	m_render->DetachTargetImage() ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AppendTransformation( af, nTransparency ) ;
}

SGLError S3DRenderContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetTransformation( af, nTransparency ) ;
}

SGLError S3DRenderContext::CurrentAffine( SGLAffine & af )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->CurrentAffine( af ) ;
}

unsigned int S3DRenderContext::CurrentTransparency( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->CurrentTransparency() ;
}

SGLError S3DRenderContext::PushTransformation( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->PushTransformation() ;
}

SGLError S3DRenderContext::PopTransformation( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->PopTransformation() ;
}

SGLError S3DRenderContext::ResetTransformation( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->ResetTransformation() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetPaintFlags( int64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetPaintFlags( nFlags ) ;
}

int64_t S3DRenderContext::GetPaintFlags( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetPaintFlags() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::FillClearTarget( uint32_t argb, int64_t flags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->FillClearTarget( argb, flags ) ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::FillRectangle
	( int x, int y, int width, int height,
				uint32_t argb, double z, uint32_t flags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->FillRectangle( x, y, width, height, argb, z, flags ) ;
}

SGLError S3DRenderContext::FillPolygon
	( const S2DVector * vertices, size_t count,
				uint32_t argb, double z, uint32_t flags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->FillPolygon( vertices, count, argb, z, flags ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	Image *	imgSrcImage = NULL ;
	if ( pSrcImage != NULL )
	{
		imgSrcImage = pSrcImage->GetImageObject() ;
		if ( imgSrcImage == NULL )
		{
			Trace( "failed to GetImageObject"
					" at S3DRenderContext::DrawImage\n" ) ;
			return	sglErrFailed ;
		}
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->DrawImage( ppPaint, imgSrcImage, pSrcClip ) ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	Image *	imgSrcImage = NULL ;
	if ( pSrcImage != NULL )
	{
		imgSrcImage = pSrcImage->GetImageObject() ;
		if ( imgSrcImage == NULL )
		{
			Trace( "failed to GetImageObject"
					"at S3DRenderContext::DrawMesh\n" ) ;
		}
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->DrawMesh
		( pDstMesh, pSrcMesh, widthMesh, heightMesh,
						ppPaint, imgSrcImage, pSrcClip ) ;
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->DrawMultiImages
				( nCount, pParams, ppSrcImages, pSrcClips ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::Flush( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->Flush() ;
}

SGLError S3DRenderContext::Finish( void )
{
	ESLAssert( m_render != NULL ) ;
	SGLError	err = m_render->Finish() ;
	#if	defined(__COTOPHA__)
	m_arrayTemporary.RemoveAll() ;
	#endif
	return	err ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->CopyBufferFrom( renderSrc, nFlags, xDst, yDst, pSrcRect ) ;
}

// マルチターゲット（2つ目以降）描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AttachMultiTargetImages
	( SGLImageObject *const* ppTargets, size_t nCount )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AttachMultiTargetImages( ppTargets, nCount ) ;
}

// マルチターゲット（2つ目以降）取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject*const* S3DRenderContext::GetMultiTargetImages( size_t& nCount ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetMultiTargetImages( nCount ) ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AttachCustomShader( S3DCustomShader * pShader )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AttachCustomShader( pShader ) ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderContext::GetCustomShader( void ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetCustomShader() ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
		S3DCustomShader::UniformType type,
			const void * pData, size_t nCount )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetCustomShaderUniform
				( pwszUniformId, type, pData, nCount ) ;
}

SGLError S3DRenderContext::ResetCustomShaderUniform( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->ResetCustomShaderUniform() ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal, const S2DVector * pvUVMap,
		const S3DColor * pColor, const uint32_t * pIndexedList )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AddIndexedTriangleList
		( pMaterial->GetMaterialObject(), nFlags,
			countPolygon, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AddTriangleStrip
		( pMaterial->GetMaterialObject(), nFlags,
			countTriangleStrip,
			pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AddIndexedPrimitiveList
		( pMaterial->GetMaterialObject(), nFlags,
			typePrimitive, countIndex, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AddVertexBuffer
		( pMaterial->GetMaterialObject(), nFlags,
			pBuffer->GetVertexBufferObject(), iFirst, iEnd,
			nInstancing, pmatInstancing, pColorInstancing ) ;
}

// 遅延削除オブジェクト追加（Flush 時に削除）
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::AddTemporaryObject( ESLObject * pObj )
{
	ESLAssert( m_render != NULL ) ;
#if	!defined(__COTOPHA__)
	m_render->AddTemporaryObject( pObj ) ;
#else
	m_arrayTemporary.Add( pObj ) ;
#endif
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AppendMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AppendMatrixTransformation( mat, pos, color, nTransparency ) ;
}

SGLError S3DRenderContext::SetMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetMatrixTransformation( mat, pos, color, nTransparency ) ;
}

SGLError S3DRenderContext::GetMatrixTransformation
	( S3DDMatrix& mat, S3DDVector& pos,
		S3DColor * color, unsigned int * pTransparency ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetMatrixTransformation( mat, pos, color, pTransparency ) ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
}

// 投影スクリーン座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::GetProjectionScreen
	( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
}

// 透視変換行列取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderContext::GetPerspectiveMatrix( S4DMatrix& matPers ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetPerspectiveMatrix( matPers ) ;
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetPerspectiveMatrix
	( S3DRenderContextInterface::StereoViewIndex sviView,
					const S4DMatrix& matPers, bool fPersMatrix )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetPerspectiveMatrix( sviView, matPers, fPersMatrix ) ;
}

void S3DRenderContext::EnablePerspectiveMatrix( bool fPersMatrix )
{
	ESLAssert( m_render != NULL ) ;
	m_render->EnablePerspectiveMatrix( fPersMatrix ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetCamera( matCamera, posCamera ) ;
}

// カメラ取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::GetCamera
	( S3DDMatrix& matCamera, S3DDVector& posCamera ) const
{
	ESLAssert( m_render != NULL ) ;
	m_render->GetCamera( matCamera, posCamera ) ;
}

// 球（ローカル座標）が視界に収まるか判定する
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderContext::IsSphereIntoView( const S3DDVector& vPos, double radius ) const
{
#if	defined(__COTOPHA__)
	return	S3DRenderContextInterface::IsSphereIntoView( vPos, radius ) ;
#else
	ESLAssert( m_render != NULL ) ;
	return	m_render->IsSphereIntoView( vPos, radius ) ;
#endif
}

// 立体視視差設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
}

// 立体視視差取得
//////////////////////////////////////////////////////////////////////////////
double S3DRenderContext::GetParallax( void ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetParallax() ;
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetZClipRange( double zMin, double zMax )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetZClipRange( zMin, zMax ) ;
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetLightEntries( pLights, countLight ) ;
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMap,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	ESLAssert( m_render != NULL ) ;
	Image *	pimgShadowMap = NULL ;
	Image *	pimgShadowColor = NULL ;
	if ( pShadowMap != NULL )
	{
		pimgShadowMap = pShadowMap->GetImageObject() ;
	}
	if ( pShadowMapColor != NULL )
	{
		pimgShadowColor = pShadowMapColor->GetImageObject() ;
	}
	m_render->SetShadowMap
		( idLight, pimgShadowMap,
			infShadowMap, pimgShadowColor ) ;
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetFog( rgbFog, zFogNear, zFogFar ) ;
}

void S3DRenderContext::EnableFog( bool fFog )
{
	ESLAssert( m_render != NULL ) ;
	m_render->EnableFog( fFog ) ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetShadingFlag( uint64_t nShadingMethod )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetShadingFlag( nShadingMethod ) ;
}

// シェーディング取得
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DRenderContext::GetShadingFlag( void )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetShadingFlag() ;
}

// レイトレーシング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetRayTracingParameter
			( const S3DRenderRayTracingParam& rrtp )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetRayTracingParameter( rrtp ) ;
}

// グローバル環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetEnvironmentMappingImage
			( SGLImageObject * pImage, uint32_t nFlags )
{
	Image *	imgTexture = NULL ;
	if ( pImage != NULL )
	{
		imgTexture = pImage->GetImageObject() ;
	}
	ESLAssert( m_render != NULL ) ;
	m_render->SetEnvironmentMappingImage( imgTexture, nFlags ) ;
}

// グローバル環境マッピング変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetEnvironmentMappingMatrix( const S3DMatrix& matMapping )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetEnvironmentMappingMatrix( matMapping ) ;
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetOffsetBorderColor( uint32_t rgbBorder )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetOffsetBorderColor( rgbBorder ) ;
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SetOffsetBorderCoefficient( a, b ) ;
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
			int32_t nParam1, const void * pParam2, size_t sizeOfParam2 )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// オプショナル機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::GetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
					int32_t nParam1, void * pParam2, size_t sizeOfParam2 ) const
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetOptionalFeature( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	ESLAssert( m_render != NULL ) ;
	m_render->GetRenderingCapacity( caps ) ;
}

// 選択中の立体視用バッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface::StereoViewIndex
	S3DRenderContext::CurrentParallaxView( void )
{
	ESLAssert( m_render != NULL ) ;
	return	(S3DRenderContextInterface::StereoViewIndex)
					((int) m_render->CurrentParallaxView()) ;
}

// 立体視用バッファ選択
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SelectParallaxView
		( S3DRenderContextInterface::StereoViewIndex sviView )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SelectParallaxView
				( (RenderContext::StereoViewIndex) ((int) sviView) ) ;
}

// 内部バッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SetRenderingBufferSize( uint32_t countVertex )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetRenderingBufferSize( countVertex ) ;
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::Begin3DRenderer( uint64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->Begin3DRenderer( nFlags ) ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::End3DRenderer( uint64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->End3DRenderer( nFlags ) ;
}

// 非同期レンダリング開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::AsyncFlush
	( uint32_t nFlags, SSystem::SSignalEvent * pSignal )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->AsyncFlush( nFlags, pSignal ) ;
}

// 非同期レンダリング完了待機
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::WaitFlush( int64_t msecTimeout )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->WaitFlush( msecTimeout ) ;
}

// 非同期レンダリングに適したスレッドで実行
//////////////////////////////////////////////////////////////////////////////
void S3DRenderContext::SuitableProcedure
	( S3DRenderContextInterface::PROCEDURE_RENDERING pfnRendering, void * pInstance )
{
	ESLAssert( m_render != NULL ) ;
	m_render->SuitableProcedure( pfnRendering, pInstance ) ;
}

#if	defined(__COTOPHA__)
// PaintContext 取得
//////////////////////////////////////////////////////////////////////////////
PaintContext * S3DRenderContext::GetPaintContextObject( void ) const
{
	return	m_render ;
}

// RenderContext 取得
//////////////////////////////////////////////////////////////////////////////
RenderContext * S3DRenderContext::GetRenderContextObject( void ) const
{
	return	m_render ;
}

#else
// ハードウェア描画オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * S3DRenderContext::GetRenderDeviceObject( uint64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	return	m_render->GetRenderDeviceObject( nFlags ) ;
}

// ハードウェア描画オブジェクト変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderContext::SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	if ( (m_render == nullptr) && (pDevice != nullptr) )
	{
		m_render = pDevice->NewRenderer() ;
		return	(m_render != nullptr) ? sglErrSuccess : sglErrFailed ;
	}
	SGLError	err = m_render->SetRenderDeviceObject( pDevice, nFlags ) ;
	if ( err && (pDevice != nullptr) )
	{
		AttachRenderContext( pDevice->NewRenderer(), true ) ;
		return	(m_render != nullptr) ? sglErrSuccess : sglErrFailed ;
	}
	return	err ;
}

#endif

