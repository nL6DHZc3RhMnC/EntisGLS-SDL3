#ifdef GL_ES
precision highp float ;
#ifdef _DISABLE_FLOAT_PRECISION_
#define lowp
#define highp
#define mediump
#endif
#else
#define lowp
#define highp
#define mediump
#endif
#ifdef _GLSL_VERSION_130_LATER_
#define attribute in
#define varying out
#define texture2D texture
#define texture3D texture
#define textureCube texture
#define texture2DArray texture
#endif
#define _VERTEX_SHADER 1
attribute highp vec3 a_vVertexPosition ;
attribute mediump vec3 a_vVertexNormal ;
#ifdef _PHONG_SHADER
#ifndef _DISABLE_UV_AXIS_ATTRIBUTE
attribute mediump vec3 a_vVertexMappingX ;
attribute mediump vec3 a_vVertexMappingY ;
#endif
#endif
attribute mediump vec2 a_vTextureCoord ;
attribute mediump vec4 a_vVertexMulColor ;
attribute mediump vec4 a_vVertexAddColor ;
#ifdef _ENABLE_VT_INSTANCING
uniform sampler2D u_samplerVertex ;
uniform highp vec2 u_vVertexTextureScale ;
uniform bool u_bVertexMorphing ;
uniform bool u_bVertexBoneInstance ;
uniform highp float u_yVertexMorphingFirst ;
uniform highp float u_yNormalMorphingFirst ;
uniform highp float u_yVertexMorphingStride ;
uniform highp float u_yVertexMorphingInstance ;
uniform highp float u_xVertexMorphingInstanceStride ;
uniform highp float u_yVertexBoneWeight ;
uniform highp float u_yVertexBoneIndex ;
uniform highp float u_yVertexBoneMatrix ;
uniform highp float u_yVertexBoneMatrixStride ;
uniform highp float u_yVertexExAttrElements ;
uniform highp float u_xVertexExAttrStride ;
#else
#ifndef _DISABLE_MORPHING
attribute highp vec3 a_vMorphPosition ;
attribute mediump vec3 a_vMorphNormal ;
#ifdef _PHONG_SHADER
#ifndef _DISABLE_UV_AXIS_ATTRIBUTE
attribute mediump vec3 a_vMorphMappingX ;
attribute mediump vec3 a_vMorphMappingY ;
#endif
#endif
attribute mediump vec2 a_vMorphTextureCoord ;
attribute mediump vec4 a_vMorphMulColor ;
attribute mediump vec4 a_vMorphAddColor ;
#endif
#ifndef _DISABLE_BONE_OVER_0
attribute mediump vec4 a_vVertexBoneWeight0 ;
#ifndef _DISABLE_BONE_OVER_4
attribute mediump vec4 a_vVertexBoneWeight4 ;
#ifndef _DISABLE_BONE_OVER_8
attribute mediump vec4 a_vVertexBoneWeight8 ;
#ifndef _DISABLE_BONE_OVER_12
attribute mediump vec4 a_vVertexBoneWeight12 ;
#endif
#endif
#endif
#endif
#endif
#ifdef _ENABLE_INSTANCING
attribute highp vec4 a_matInstancingModelView0 ;
attribute highp vec4 a_matInstancingModelView1 ;
attribute highp vec4 a_matInstancingModelView2 ;
attribute mediump vec4 a_vInstancingMulColor ;
attribute mediump vec4 a_vInstancingAddColor ;
#endif
