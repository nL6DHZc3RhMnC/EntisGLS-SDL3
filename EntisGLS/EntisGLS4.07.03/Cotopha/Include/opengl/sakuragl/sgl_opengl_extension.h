
#if	!defined(__SAKURAGL_OPENGL_EXTENSION_H__)
#define	__SAKURAGL_OPENGL_EXTENSION_H__

#if	defined(__PLATFORM_ANDROID__)

//////////////////////////////////////////////////////////////////////////////
// Android OpenGL ES
//////////////////////////////////////////////////////////////////////////////

#define	GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>

#if	ANDROID_API_LEVEL >= 8
	#if	ANDROID_API_LEVEL < 18
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
	#else
		#if	ANDROID_API_LEVEL >= 24
			#include <GLES3/gl32.h>
		#elif	ANDROID_API_LEVEL >= 21
			#include <GLES3/gl31.h>
		#else
			#include <GLES3/gl3.h>
		#endif
		#include <GLES3/gl3ext.h>
		#define __gl2_h_
		#include <GLES2/gl2ext.h>
	#endif
#endif

#else

//////////////////////////////////////////////////////////////////////////////
// 一般 OpenGL
//////////////////////////////////////////////////////////////////////////////

/*
// GLEW を使用する場合はこちら
#define	GLEW_STATIC
#include <GL/glew.h>
#if	defined(GLEW_STATIC)
	#if	defined(GLEW_MX)
		#pragma comment( lib, "glew32mxs.lib" )
	#else
		#pragma comment( lib, "glew32s.lib" )
	#endif
#else
	#if	defined(GLEW_MX)
		#pragma comment( lib, "glew32mx.lib" )
	#else
		#pragma comment( lib, "glew32.lib" )
	#endif
#endif
*/

#include <GL/gl.h>
#include <GL/glu.h>

#endif

//
// GLEW を使用せず SakuraGL で使用する関数だけ簡易実装
//
#if	defined(__PLATFORM_WINDOWS__)
#define	GLEX_APICALL	__stdcall
#else
#define	GLEX_APICALL
#endif

namespace	OpenGLExtension
{
	// 初期化関数
	void Initialize( void ) ;
	// 終了関数
	void Finalize( void ) ;

	// 有効関数フラグ
	extern	bool	g_supports_opengl_1_3 ;
	extern	bool	g_supports_opengl_1_5 ;
	extern	bool	g_supports_opengl_es1_0 ;
	extern	bool	g_supports_framebuffer_object ;
	extern	bool	g_supports_opengl_2_0 ;
	extern	bool	g_supports_program_binary ;
	extern	bool	g_supports_multiple_render_target ;
	extern	bool	g_supports_vertex_array_object ;
	extern	bool	g_supports_texture3d ;
	extern	bool	g_supports_texture_multisample ;
	extern	bool	g_supports_instanced_draw ;
	extern	bool	g_supports_compute_shader ;

	// 無効化プロファイル
	extern	bool	g_disable_run_any_threads ;
	extern	bool	g_disable_texture_non_power_of_2 ;
	extern	bool	g_disable_element_index_uint ;
	extern	bool	g_disable_opengl_2_0 ;
	extern	bool	g_disable_program_binary ;
	extern	bool	g_disable_compute_shader ;

	// 定数
	enum	ConstantNumber
	{
#if	defined(__PLATFORM_ANDROID__)
	#ifndef	GL_DEPTH_COMPONENT
		GL_DEPTH_COMPONENT		= GL_DEPTH_COMPONENT32_OES,
	#endif
	#ifndef	GL_DEPTH_COMPONENT16
		GL_DEPTH_COMPONENT16	= GL_DEPTH_COMPONENT16_OES,
	#endif
	#ifndef	GL_DEPTH_COMPONENT24
		GL_DEPTH_COMPONENT24	= GL_DEPTH_COMPONENT24_OES,
	#endif
	#ifndef	GL_DEPTH_COMPONENT32
		GL_DEPTH_COMPONENT32	= GL_DEPTH_COMPONENT32_OES,
	#endif

	#ifndef	GL_FRAMEBUFFER
		GL_FRAMEBUFFER			= GL_FRAMEBUFFER_OES,
	#endif
	#ifndef	GL_RENDERBUFFER
		GL_RENDERBUFFER			= GL_RENDERBUFFER_OES,
	#endif
	#ifndef	GL_COLOR_ATTACHMENT0
		GL_COLOR_ATTACHMENT0	= GL_COLOR_ATTACHMENT0_OES,
	#endif
	#ifndef	GL_DEPTH_ATTACHMENT
		GL_DEPTH_ATTACHMENT		= GL_DEPTH_ATTACHMENT_OES,
	#endif
	#ifndef	GL_FRAMEBUFFER_COMPLETE
		GL_FRAMEBUFFER_COMPLETE	= GL_FRAMEBUFFER_COMPLETE_OES,
	#endif
#else
	#ifndef	GL_COMPRESSED_RGB
		GL_COMPRESSED_RGB			= 0x84ED,
		GL_COMPRESSED_RGBA			= 0x84EE,
		GL_TEXTURE_COMPRESSION_HINT	= 0x84EF,
	#endif
	#ifndef	GL_SRGB
		GL_SRGB					= 0x8C40,
	#endif
	#ifndef	GL_SRGB8
		GL_SRGB8				= 0x8C41,
	#endif
	#ifndef	GL_SRGB_ALPHA
		GL_SRGB_ALPHA			= 0x8C42,
	#endif
	#ifndef	GL_SRGB8_ALPHA8
		GL_SRGB8_ALPHA8			= 0x8C43,
	#endif
	#ifndef	GL_COLOR_ATTACHMENT0
		GL_COLOR_ATTACHMENT0	= 0x8CE0,
	#endif
	#ifndef	GL_DEPTH_ATTACHMENT
		GL_DEPTH_ATTACHMENT		= 0x8D00,
	#endif
	#ifndef	GL_FRAMEBUFFER
		GL_FRAMEBUFFER			= 0x8D40,
	#endif
	#ifndef	GL_READ_FRAMEBUFFER
		GL_READ_FRAMEBUFFER		= 0x8CA8,
	#endif
	#ifndef	GL_DRAW_FRAMEBUFFER
		GL_DRAW_FRAMEBUFFER		= 0x8CA9,
	#endif
	#ifndef	GL_RENDERBUFFER
		GL_RENDERBUFFER			= 0x8D41,
	#endif
	#ifndef	GL_DEPTH_COMPONENT16
		GL_DEPTH_COMPONENT16	= 0x81A5,
		GL_DEPTH_COMPONENT24	= 0x81A6,
		GL_DEPTH_COMPONENT32	= 0x81A7,
	#endif
	#ifndef	GL_DEPTH_COMPONENT32F
		GL_DEPTH_COMPONENT32F	= 0x8CAC,
	#endif
	#ifndef	GL_DEPTH32F_STENCIL8
		GL_DEPTH32F_STENCIL8	= 0x8CAD,
	#endif
	#ifndef	GL_DEPTH24_STENCIL8
		GL_DEPTH24_STENCIL8		= 0x88F0,
	#endif
	#ifndef	GL_FRAMEBUFFER_COMPLETE
		GL_FRAMEBUFFER_COMPLETE	= 0x8CD5,
		GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT			= 0x8CD6,
		GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT	= 0x8CD7,
		GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS			= 0x8CD9,
		GL_FRAMEBUFFER_UNSUPPORTED						= 0x8CDD,
		GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE			= 0x8D56,
	#endif
	#ifndef	GL_MULTISAMPLE
		GL_MULTISAMPLE			= 0x809D,
	#endif
#endif
	#ifndef	GL_RGBA32F
		GL_RGBA32F				= 0x8814,
		GL_RGB32F				= 0x8815,
		GL_RGBA16F				= 0x881A,
		GL_RGB16F				= 0x881B,
	#endif
	#ifndef	GL_R8
		GL_RG					= 0x8227,
		GL_R8					= 0x8229,
		GL_RG8					= 0x822B,
		GL_R16F					= 0x822D,
		GL_R32F					= 0x822E,
		GL_RG32F				= 0x8230,
	#endif
	#ifndef	GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		GL_COMPRESSED_RGB_S3TC_DXT1_EXT		= 0x83F0,
		GL_COMPRESSED_RGBA_S3TC_DXT1_EXT	= 0x83F1,
		GL_COMPRESSED_RGBA_S3TC_DXT3_EXT	= 0x83F2,
		GL_COMPRESSED_RGBA_S3TC_DXT5_EXT	= 0x83F3,
	#endif
	#ifndef	GL_MAJOR_VERSION
		GL_MAJOR_VERSION		= 0x821B,
	#endif
	#ifndef	GL_MINOR_VERSION
		GL_MINOR_VERSION		= 0x821C,
	#endif
	#ifndef	GL_CLAMP_TO_EDGE
		GL_CLAMP_TO_EDGE		= 0x812F,
	#endif
	#ifndef	GL_TEXTURE_DEPTH
		GL_TEXTURE_DEPTH		= 0x8071,
	#endif
	#ifndef	GL_TEXTURE0
		GL_TEXTURE0				= 0x84C0,
		GL_TEXTURE1,
		GL_TEXTURE2,
		GL_TEXTURE3,
		GL_TEXTURE4,
		GL_TEXTURE5,
	#endif
	#ifndef	GL_MAX_TEXTURE_UNITS
		GL_MAX_TEXTURE_UNITS	= 0x84E2,
	#endif
	#ifndef	GL_MAX_TEXTURE_IMAGE_UNITS
			GL_MAX_TEXTURE_IMAGE_UNITS	= 0x8872,
	#endif
	#ifndef	GL_MAX_IMAGE_UNITS
		GL_MAX_IMAGE_UNITS		= 0x8F38,
	#endif
	#ifndef	GL_TEXTURE_MAX_ANISOTROPY_EXT
		GL_TEXTURE_MAX_ANISOTROPY_EXT		= 0x84FE,
		GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	= 0x84FF,
	#endif
	#ifndef	GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS	= 0x8B4C,
	#endif
	#ifndef	GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS	= 0x8B4D,
	#endif
	#ifndef	GL_TEXTURE_CUBE_MAP
		GL_TEXTURE_CUBE_MAP				= 0x8513,
		GL_TEXTURE_BINDING_CUBE_MAP		= 0x8514,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X	= 0x8515,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X	= 0x8516,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y	= 0x8517,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	= 0x8518,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z	= 0x8519,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	= 0x851A,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE	= 0x851C,
	#endif
	#ifndef	GL_TEXTURE_3D
		GL_TEXTURE_3D			= 0x806F,
	#endif
	#ifndef	GL_TEXTURE_BASE_LEVEL
		GL_TEXTURE_BASE_LEVEL	= 0x813C,
	#endif
	#ifndef	GL_TEXTURE_MAX_LEVEL
		GL_TEXTURE_MAX_LEVEL	= 0x813D,
	#endif
	#ifndef	GL_TEXTURE_2D_ARRAY
		GL_TEXTURE_2D_ARRAY		= 0x8C1A,
	#endif
	#ifndef	GL_MAX_3D_TEXTURE_SIZE
		GL_MAX_3D_TEXTURE_SIZE	= 0x8073,
	#endif
	#ifndef	GL_FRAMEBUFFER_SRGB
		GL_FRAMEBUFFER_SRGB		= 0x8DB9,
	#endif
	#ifndef	GL_MAX_VERTEX_ATTRIBS
		GL_MAX_VERTEX_ATTRIBS	= 0x8869,
	#endif
	#ifndef	GL_MAX_VERTEX_UNIFORM_VECTORS
		GL_MAX_VERTEX_UNIFORM_VECTORS	= 0x8DFB,
	#endif
	#ifndef	GL_MAX_VARYING_VECTORS
		GL_MAX_VARYING_VECTORS	= 0x8DFC,
	#endif
	#ifndef	GL_MAX_FRAGMENT_UNIFORM_VECTORS
		GL_MAX_FRAGMENT_UNIFORM_VECTORS	= 0x8DFD,
	#endif
	#ifndef	GL_MAX_DRAW_BUFFERS
		GL_MAX_DRAW_BUFFERS		= 0x8824,
	#endif
	#ifndef	GL_MAX_COLOR_ATTACHMENTS
		GL_MAX_COLOR_ATTACHMENTS= 0x8CDF,
	#endif
	#ifndef	GL_ARRAY_BUFFER
		GL_ARRAY_BUFFER			= 0x8892,
	#endif
	#ifndef	GL_ELEMENT_ARRAY_BUFFER
		GL_ELEMENT_ARRAY_BUFFER	= 0x8893,
	#endif
	#ifndef	GL_READ_ONLY
		GL_READ_ONLY			= 0x88B8,
		GL_WRITE_ONLY			= 0x88B9,
		GL_READ_WRITE			= 0x88BA,
	#endif
	#ifndef	GL_BUFFER_ACCESS
		GL_BUFFER_ACCESS		= 0x88BB,
	#endif
	#ifndef	GL_BUFFER_MAPPED
		GL_BUFFER_MAPPED		= 0x88BC,
		GL_BUFFER_MAP_POINTER	= 0x88BD,
	#endif
	#ifndef	GL_STREAM_DRAW
		GL_STREAM_DRAW			= 0x88E0,
		GL_STREAM_READ			= 0x88E1,
		GL_STREAM_COPY			= 0x88E2,
	#endif
	#ifndef	GL_STATIC_DRAW
		GL_STATIC_DRAW			= 0x88E4,
		GL_STATIC_READ			= 0x88E5,
		GL_STATIC_COPY			= 0x88E6,
	#endif
	#ifndef	GL_DYNAMIC_DRAW
		GL_DYNAMIC_DRAW			= 0x88E8,
		GL_DYNAMIC_READ			= 0x88E9,
		GL_DYNAMIC_COPY			= 0x88EA,
	#endif
	#ifndef	GL_FRAGMENT_SHADER
		GL_FRAGMENT_SHADER		= 0x8B30,
	#endif
	#ifndef	GL_VERTEX_SHADER
		GL_VERTEX_SHADER		= 0x8B31,
	#endif
	#ifndef	GL_GEOMETRY_SHADER
		GL_GEOMETRY_SHADER		= 0x8DD9,
	#endif
	#ifndef	GL_COMPILE_STATUS
		GL_COMPILE_STATUS		= 0x8B81,
	#endif
	#ifndef	GL_LINK_STATUS
		GL_LINK_STATUS			= 0x8B82,
	#endif
	#ifndef	GL_INFO_LOG_LENGTH
		GL_INFO_LOG_LENGTH		= 0x8B84,
	#endif
	#ifndef	GL_PROGRAM_BINARY_RETRIEVABLE_HINT
		GL_PROGRAM_BINARY_RETRIEVABLE_HINT	= 0x8257,
	#endif
	#ifndef	GL_PROGRAM_BINARY_LENGTH
		GL_PROGRAM_BINARY_LENGTH	= 0x8741,
	#endif
	#ifndef	GL_NUM_PROGRAM_BINARY_FORMATS
		GL_NUM_PROGRAM_BINARY_FORMATS	= 0x87FE,
	#endif
	#ifndef	GL_PROGRAM_BINARY_FORMATS
		GL_PROGRAM_BINARY_FORMATS	= 0x87FF,
	#endif
	#ifndef	GL_COMPUTE_SHADER
		GL_COMPUTE_SHADER		= 0x91B9,
	#endif
	#ifndef	GL_TEXTURE_2D_MULTISAMPLE
		GL_TEXTURE_2D_MULTISAMPLE		= 0x9100,
		GL_MAX_COLOR_TEXTURE_SAMPLES	= 0x910E,
		GL_MAX_DEPTH_TEXTURE_SAMPLES	= 0x910F,
	#endif
	#ifndef	GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT	= 0x00000001,
		GL_ELEMENT_ARRAY_BARRIER_BIT		= 0x00000002,
		GL_UNIFORM_BARRIER_BIT				= 0x00000004,
		GL_TEXTURE_FETCH_BARRIER_BIT		= 0x00000008,
		GL_SHADER_IMAGE_ACCESS_BARRIER_BIT	= 0x00000020,
		GL_COMMAND_BARRIER_BIT				= 0x00000040,
		GL_PIXEL_BUFFER_BARRIER_BIT			= 0x00000080,
		GL_TEXTURE_UPDATE_BARRIER_BIT		= 0x00000100,
		GL_BUFFER_UPDATE_BARRIER_BIT		= 0x00000200,
		GL_FRAMEBUFFER_BARRIER_BIT			= 0x00000400,
		GL_TRANSFORM_FEEDBACK_BARRIER_BIT	= 0x00000800,
		GL_ATOMIC_COUNTER_BARRIER_BIT		= 0x00001000,
	#endif

	} ;

	// OpenGL 1.3 (Multi texture) 関数群
#if	!defined(__PLATFORM_ANDROID__)
	typedef void (GLEX_APICALL * API_glActiveTexture)( GLenum texture ) ;
	typedef void (GLEX_APICALL * API_glClientActiveTexture)( GLenum texture ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexImage1D)
			( GLenum target, GLint level, GLenum internalformat,
				GLsizei width, GLint border,
				GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexImage2D)
			( GLenum target, GLint level, GLenum internalformat,
				GLsizei width, GLsizei height, GLint border,
				GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexImage3D)
			( GLenum target, GLint level, GLenum internalformat,
				GLsizei width, GLsizei height, GLsizei depth,
				GLint border, GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexSubImage1D)
			( GLenum target, GLint level, GLint xoffset,
				GLsizei width, GLenum format,
				GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexSubImage2D)
			( GLenum target, GLint level,
				GLint xoffset, GLint yoffset,
				GLsizei width, GLsizei height,
				GLenum format, GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glCompressedTexSubImage3D)
			( GLenum target, GLint level,
				GLint xoffset, GLint yoffset, GLint zoffset,
				GLsizei width, GLsizei height, GLsizei depth,
				GLenum format, GLsizei imageSize, const GLvoid *data ) ;
	typedef void (GLEX_APICALL * API_glGetCompressedTexImage)
						( GLenum target, GLint lod, GLvoid *img ) ;
	typedef void (GLEX_APICALL * API_glLoadTransposeMatrixd)( const GLdouble m[16] ) ;
	typedef void (GLEX_APICALL * API_glLoadTransposeMatrixf)( const GLfloat m[16] ) ;
	typedef void (GLEX_APICALL * API_glMultTransposeMatrixd)( const GLdouble m[16] ) ;
	typedef void (GLEX_APICALL * API_glMultTransposeMatrixf)( const GLfloat m[16] ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1d)( GLenum target, GLdouble s ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1dv)( GLenum target, const GLdouble *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1f)( GLenum target, GLfloat s ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1fv)( GLenum target, const GLfloat *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1i)( GLenum target, GLint s ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1iv)( GLenum target, const GLint *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1s)( GLenum target, GLshort s ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord1sv)( GLenum target, const GLshort *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2d)( GLenum target, GLdouble s, GLdouble t ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2dv)( GLenum target, const GLdouble *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2f)( GLenum target, GLfloat s, GLfloat t ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2fv)( GLenum target, const GLfloat *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2i)( GLenum target, GLint s, GLint t ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2iv)( GLenum target, const GLint *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2s)( GLenum target, GLshort s, GLshort t ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord2sv)( GLenum target, const GLshort *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3d)
					( GLenum target, GLdouble s, GLdouble t, GLdouble r ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3dv)
					( GLenum target, const GLdouble *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3f)
					( GLenum target, GLfloat s, GLfloat t, GLfloat r ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3fv)
					( GLenum target, const GLfloat *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3i)
					( GLenum target, GLint s, GLint t, GLint r ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3iv)
					( GLenum target, const GLint *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3s)
					( GLenum target, GLshort s, GLshort t, GLshort r ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord3sv)
					( GLenum target, const GLshort *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4d)
			( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4dv)
			( GLenum target, const GLdouble *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4f)
			( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4fv)
			( GLenum target, const GLfloat *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4i)
			( GLenum target, GLint s, GLint t, GLint r, GLint q ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4iv)
			( GLenum target, const GLint *v ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4s)
			( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4sv)
			( GLenum target, const GLshort *v ) ;
	typedef void (GLEX_APICALL * API_glSampleCoverage)
			( GLclampf value, GLboolean invert ) ;

	extern	API_glActiveTexture				glActiveTexture ;
	extern	API_glClientActiveTexture		glClientActiveTexture ;
	extern	API_glCompressedTexImage1D		glCompressedTexImage1D ;
	extern	API_glCompressedTexImage2D		glCompressedTexImage2D ;
	extern	API_glCompressedTexImage3D		glCompressedTexImage3D ;
	extern	API_glCompressedTexSubImage1D	glCompressedTexSubImage1D ;
	extern	API_glCompressedTexSubImage2D	glCompressedTexSubImage2D ;
	extern	API_glCompressedTexSubImage3D	glCompressedTexSubImage3D ;
	extern	API_glGetCompressedTexImage		glGetCompressedTexImage ;
	extern	API_glLoadTransposeMatrixd		glLoadTransposeMatrixd ;
	extern	API_glLoadTransposeMatrixf		glLoadTransposeMatrixf ;
	extern	API_glMultTransposeMatrixd		glMultTransposeMatrixd ;
	extern	API_glMultTransposeMatrixf		glMultTransposeMatrixf ;
	extern	API_glMultiTexCoord1d			glMultiTexCoord1d ;
	extern	API_glMultiTexCoord1dv			glMultiTexCoord1dv ;
	extern	API_glMultiTexCoord1f			glMultiTexCoord1f ;
	extern	API_glMultiTexCoord1fv			glMultiTexCoord1fv ;
	extern	API_glMultiTexCoord1i			glMultiTexCoord1i ;
	extern	API_glMultiTexCoord1iv			glMultiTexCoord1iv ;
	extern	API_glMultiTexCoord1s			glMultiTexCoord1s ;
	extern	API_glMultiTexCoord1sv			glMultiTexCoord1sv ;
	extern	API_glMultiTexCoord2d			glMultiTexCoord2d ;
	extern	API_glMultiTexCoord2dv			glMultiTexCoord2dv ;
	extern	API_glMultiTexCoord2f			glMultiTexCoord2f ;
	extern	API_glMultiTexCoord2fv			glMultiTexCoord2fv ;
	extern	API_glMultiTexCoord2i			glMultiTexCoord2i ;
	extern	API_glMultiTexCoord2iv			glMultiTexCoord2iv ;
	extern	API_glMultiTexCoord2s			glMultiTexCoord2s ;
	extern	API_glMultiTexCoord2sv			glMultiTexCoord2sv ;
	extern	API_glMultiTexCoord3d			glMultiTexCoord3d ;
	extern	API_glMultiTexCoord3dv			glMultiTexCoord3dv ;
	extern	API_glMultiTexCoord3f			glMultiTexCoord3f ;
	extern	API_glMultiTexCoord3fv			glMultiTexCoord3fv ;
	extern	API_glMultiTexCoord3i			glMultiTexCoord3i ;
	extern	API_glMultiTexCoord3iv			glMultiTexCoord3iv ;
	extern	API_glMultiTexCoord3s			glMultiTexCoord3s ;
	extern	API_glMultiTexCoord3sv			glMultiTexCoord3sv ;
	extern	API_glMultiTexCoord4d			glMultiTexCoord4d ;
	extern	API_glMultiTexCoord4dv			glMultiTexCoord4dv ;
	extern	API_glMultiTexCoord4f			glMultiTexCoord4f ;
	extern	API_glMultiTexCoord4fv			glMultiTexCoord4fv ;
	extern	API_glMultiTexCoord4i			glMultiTexCoord4i ;
	extern	API_glMultiTexCoord4iv			glMultiTexCoord4iv ;
	extern	API_glMultiTexCoord4s			glMultiTexCoord4s ;
	extern	API_glMultiTexCoord4sv			glMultiTexCoord4sv ;
	extern	API_glSampleCoverage			glSampleCoverage ;

	// OpenGL 1.5 (VertexBuffer) 関数群
	#if	defined(__PROCESSOR_INTEL_X86_64__)
//		typedef int64_t		ptrdiff_t ;
	#else
		typedef int			ptrdiff_t ;
	#endif
	typedef ptrdiff_t GLintptr;
	typedef ptrdiff_t GLsizeiptr;
	typedef void (GLEX_APICALL * API_glBeginQuery)
			( GLenum target, GLuint id ) ;
	typedef void (GLEX_APICALL * API_glBindBuffer)
			( GLenum target, GLuint buffer ) ;
	typedef void (GLEX_APICALL * API_glBufferData)
			( GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage ) ;
	typedef void (GLEX_APICALL * API_glBufferSubData)
			( GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data ) ;
	typedef void (GLEX_APICALL * API_glDeleteBuffers)
			( GLsizei n, const GLuint* buffers ) ;
	typedef void (GLEX_APICALL * API_glDeleteQueries)
			( GLsizei n, const GLuint* ids ) ;
	typedef void (GLEX_APICALL * API_glEndQuery)( GLenum target ) ;
	typedef void (GLEX_APICALL * API_glGenBuffers)
			( GLsizei n, GLuint* buffers ) ;
	typedef void (GLEX_APICALL * API_glGenQueries)
			( GLsizei n, GLuint* ids ) ;
	typedef void (GLEX_APICALL * API_glGetBufferParameteriv)
			( GLenum target, GLenum pname, GLint* params ) ;
	typedef void (GLEX_APICALL * API_glGetBufferPointerv)
			( GLenum target, GLenum pname, GLvoid** params ) ;
	typedef void (GLEX_APICALL * API_glGetBufferSubData)
			( GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data ) ;
	typedef void (GLEX_APICALL * API_glGetQueryObjectiv)
			( GLuint id, GLenum pname, GLint* params ) ;
	typedef void (GLEX_APICALL * API_glGetQueryObjectuiv)
			( GLuint id, GLenum pname, GLuint* params ) ;
	typedef void (GLEX_APICALL * API_glGetQueryiv)
			( GLenum target, GLenum pname, GLint* params ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsBuffer)( GLuint buffer ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsQuery)( GLuint id ) ;
	typedef GLvoid* (GLEX_APICALL * API_glMapBuffer)
			( GLenum target, GLenum access ) ;
	typedef GLboolean (GLEX_APICALL * API_glUnmapBuffer)( GLenum target ) ;

	extern	API_glBeginQuery			glBeginQuery ;
	extern	API_glBindBuffer			glBindBuffer ;
	extern	API_glBufferData			glBufferData ;
	extern	API_glBufferSubData			glBufferSubData ;
	extern	API_glDeleteBuffers			glDeleteBuffers ;
	extern	API_glDeleteQueries			glDeleteQueries ;
	extern	API_glEndQuery				glEndQuery ;
	extern	API_glGenBuffers			glGenBuffers ;
	extern	API_glGenQueries			glGenQueries ;
	extern	API_glGetBufferParameteriv	glGetBufferParameteriv ;
	extern	API_glGetBufferPointerv		glGetBufferPointerv ;
	extern	API_glGetBufferSubData		glGetBufferSubData ;
	extern	API_glGetQueryObjectiv		glGetQueryObjectiv ;
	extern	API_glGetQueryObjectuiv		glGetQueryObjectuiv ;
	extern	API_glGetQueryiv			glGetQueryiv ;
	extern	API_glIsBuffer				glIsBuffer ;
	extern	API_glIsQuery				glIsQuery ;
	extern	API_glMapBuffer				glMapBuffer ;
	extern	API_glUnmapBuffer			glUnmapBuffer ;

	// OpenGL ES1.0 互換関数群
	typedef int	GLclampx ;
	typedef	int GLfixed ;
	typedef void (GLEX_APICALL * API_glAlphaFuncx)( GLenum func, GLclampx ref ) ;
	typedef void (GLEX_APICALL * API_glClearColorx)
			( GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha ) ;
	typedef void (GLEX_APICALL * API_glClearDepthx)( GLclampx depth ) ;
	typedef void (GLEX_APICALL * API_glColor4x)
			( GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha ) ;
	typedef void (GLEX_APICALL * API_glDepthRangex)( GLclampx zNear, GLclampx zFar ) ;
	typedef void (GLEX_APICALL * API_glFogx)( GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glFogxv)( GLenum pname, const GLfixed* params ) ;
	typedef void (GLEX_APICALL * API_glFrustumf)
			( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar ) ;
	typedef void (GLEX_APICALL * API_glFrustumx)
			( GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar ) ;
	typedef void (GLEX_APICALL * API_glLightModelx)( GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glLightModelxv)( GLenum pname, const GLfixed* params ) ;
	typedef void (GLEX_APICALL * API_glLightx)( GLenum light, GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glLightxv)( GLenum light, GLenum pname, const GLfixed* params ) ;
	typedef void (GLEX_APICALL * API_glLineWidthx)( GLfixed width ) ;
	typedef void (GLEX_APICALL * API_glLoadMatrixx)( const GLfixed* m ) ;
	typedef void (GLEX_APICALL * API_glMaterialx)( GLenum face, GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glMaterialxv)( GLenum face, GLenum pname, const GLfixed* params ) ;
	typedef void (GLEX_APICALL * API_glMultMatrixx)( const GLfixed* m ) ;
	typedef void (GLEX_APICALL * API_glMultiTexCoord4x)
			( GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q ) ;
	typedef void (GLEX_APICALL * API_glNormal3x)( GLfixed nx, GLfixed ny, GLfixed nz ) ;
	typedef void (GLEX_APICALL * API_glOrthof)
			( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar ) ;
	typedef void (GLEX_APICALL * API_glOrthox)
			( GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar ) ;
	typedef void (GLEX_APICALL * API_glPointSizex)( GLfixed size ) ;
	typedef void (GLEX_APICALL * API_glPolygonOffsetx)( GLfixed factor, GLfixed units ) ;
	typedef void (GLEX_APICALL * API_glRotatex)( GLfixed angle, GLfixed x, GLfixed y, GLfixed z ) ;
	typedef void (GLEX_APICALL * API_glSampleCoveragex)( GLclampx value, GLboolean invert ) ;
	typedef void (GLEX_APICALL * API_glScalex)( GLfixed x, GLfixed y, GLfixed z ) ;
	typedef void (GLEX_APICALL * API_glTexEnvx)( GLenum target, GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glTexEnvxv)( GLenum target, GLenum pname, const GLfixed* params ) ;
	typedef void (GLEX_APICALL * API_glTexParameterx)( GLenum target, GLenum pname, GLfixed param ) ;
	typedef void (GLEX_APICALL * API_glTranslatex)(GLfixed x, GLfixed y, GLfixed z ) ;

	extern	API_glAlphaFuncx		glAlphaFuncx ;
	extern	API_glClearColorx		glClearColorx ;
	extern	API_glClearDepthx		glClearDepthx ;
	extern	API_glColor4x			glColor4x ;
	extern	API_glDepthRangex		glDepthRangex ;
	extern	API_glFogx				glFogx ;
	extern	API_glFogxv				glFogxv ;
	extern	API_glFrustumf			glFrustumf ;
	extern	API_glFrustumx			glFrustumx ;
	extern	API_glLightModelx		glLightModelx ;
	extern	API_glLightModelxv		glLightModelxv ;
	extern	API_glLightx			glLightx ;
	extern	API_glLightxv			glLightxv ;
	extern	API_glLineWidthx		glLineWidthx ;
	extern	API_glLoadMatrixx		glLoadMatrixx ;
	extern	API_glMaterialx			glMaterialx ;
	extern	API_glMaterialxv		glMaterialxv ;
	extern	API_glMultMatrixx		glMultMatrixx ;
	extern	API_glMultiTexCoord4x	glMultiTexCoord4x ;
	extern	API_glNormal3x			glNormal3x ;
	extern	API_glOrthof			glOrthof ;
	extern	API_glOrthox			glOrthox ;
	extern	API_glPointSizex		glPointSizex ;
	extern	API_glPolygonOffsetx	glPolygonOffsetx ;
	extern	API_glRotatex			glRotatex ;
	extern	API_glSampleCoveragex	glSampleCoveragex ;
	extern	API_glScalex			glScalex ;
	extern	API_glTexEnvx			glTexEnvx ;
	extern	API_glTexEnvxv			glTexEnvxv ;
	extern	API_glTexParameterx		glTexParameterx ;
	extern	API_glTranslatex		glTranslatex ;
#endif

	// Frame Buffer 関数群
#if	defined(__PLATFORM_ANDROID__)
	#if	ANDROID_API_LEVEL < 8
	#define	glBindRenderbuffer			glBindRenderbufferOES
	#define	glDeleteRenderbuffers		glDeleteRenderbuffersOES
	#define	glGenRenderbuffers			glGenRenderbuffersOES
	#define	glRenderbufferStorage		glRenderbufferStorageOES
	#define	glBindFramebuffer			glBindFramebufferOES
	#define	glDeleteFramebuffers		glDeleteFramebuffersOES
	#define	glGenFramebuffers			glGenFramebuffersOES
	#define	glCheckFramebufferStatus	glCheckFramebufferStatusOES
	#define	glFramebufferRenderbuffer	glFramebufferRenderbufferOES
	#define	glFramebufferTexture2D		glFramebufferTexture2DOES
	#endif

	#if	ANDROID_API_LEVEL >= 8
	#define	glFramebufferTexture3D		glFramebufferTexture3DOES
	#endif

#else
	typedef	void (GLEX_APICALL * API_glBindFramebuffer)( GLenum target, GLuint framebuffer ) ;
	typedef void (GLEX_APICALL * API_glBindRenderbuffer)( GLenum target, GLuint renderbuffer ) ;
	typedef void (GLEX_APICALL * API_glBlitFramebuffer)
			( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
				GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) ;
	typedef GLenum (GLEX_APICALL * API_glCheckFramebufferStatus)( GLenum target ) ;
	typedef void (GLEX_APICALL * API_glDeleteFramebuffers)( GLsizei n, const GLuint* framebuffers ) ;
	typedef void (GLEX_APICALL * API_glDeleteRenderbuffers)( GLsizei n, const GLuint* renderbuffers ) ;
	typedef void (GLEX_APICALL * API_glFramebufferRenderbuffer)
			( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ) ;
	typedef void (GLEX_APICALL * API_glFramebufferTexture1D)
			( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) ;
	typedef void (GLEX_APICALL * API_glFramebufferTexture2D)
			( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) ;
	typedef void (GLEX_APICALL * API_glFramebufferTexture3D)
			( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer ) ;
	typedef void (GLEX_APICALL * API_glFramebufferTextureLayer)
			( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) ;
	typedef void (GLEX_APICALL * API_glGenFramebuffers)( GLsizei n, GLuint* framebuffers ) ;
	typedef void (GLEX_APICALL * API_glGenRenderbuffers)( GLsizei n, GLuint* renderbuffers ) ;
	typedef void (GLEX_APICALL * API_glGenerateMipmap)( GLenum target ) ;
	typedef void (GLEX_APICALL * API_glGetFramebufferAttachmentParameteriv)
			( GLenum target, GLenum attachment, GLenum pname, GLint* params ) ;
	typedef void (GLEX_APICALL * API_glGetRenderbufferParameteriv)
			( GLenum target, GLenum pname, GLint* params ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsFramebuffer)( GLuint framebuffer ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsRenderbuffer)( GLuint renderbuffer ) ;
	typedef void (GLEX_APICALL * API_glRenderbufferStorage)
			( GLenum target, GLenum internalformat, GLsizei width, GLsizei height ) ;
	typedef void (GLEX_APICALL * API_glRenderbufferStorageMultisample)
			( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) ;

	extern	API_glBindFramebuffer						glBindFramebuffer ;
	extern	API_glBindRenderbuffer						glBindRenderbuffer ;
	extern	API_glBlitFramebuffer						glBlitFramebuffer ;
	extern	API_glCheckFramebufferStatus				glCheckFramebufferStatus ;
	extern	API_glDeleteFramebuffers					glDeleteFramebuffers ;
	extern	API_glDeleteRenderbuffers					glDeleteRenderbuffers ;
	extern	API_glFramebufferRenderbuffer				glFramebufferRenderbuffer ;
	extern	API_glFramebufferTexture1D					glFramebufferTexture1D ;
	extern	API_glFramebufferTexture2D					glFramebufferTexture2D ;
	extern	API_glFramebufferTexture3D					glFramebufferTexture3D ;
	extern	API_glFramebufferTextureLayer				glFramebufferTextureLayer ;
	extern	API_glGenFramebuffers						glGenFramebuffers ;
	extern	API_glGenRenderbuffers						glGenRenderbuffers ;
	extern	API_glGenerateMipmap						glGenerateMipmap ;
	extern	API_glGetFramebufferAttachmentParameteriv	glGetFramebufferAttachmentParameteriv ;
	extern	API_glIsFramebuffer							glIsFramebuffer ;
	extern	API_glIsRenderbuffer						glIsRenderbuffer ;
	extern	API_glRenderbufferStorage					glRenderbufferStorage ;
	extern	API_glRenderbufferStorageMultisample		glRenderbufferStorageMultisample ;
#endif

	// OpenGL 2.0 (Shader) 関数群
#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8)
#if	defined(__PLATFORM_ANDROID__)
	typedef	double	GLdouble ;
#endif
	typedef char GLchar ;
	typedef void (GLEX_APICALL * API_glAttachShader)( GLuint program, GLuint shader );
	typedef void (GLEX_APICALL * API_glBindAttribLocation)
						( GLuint program, GLuint index, const GLchar* name ) ;
	typedef void (GLEX_APICALL * API_glBlendEquationSeparate)( GLenum, GLenum ) ;
	typedef void (GLEX_APICALL * API_glCompileShader)( GLuint shader ) ;
	typedef GLuint (GLEX_APICALL * API_glCreateProgram)( void ) ;
	typedef GLuint (GLEX_APICALL * API_glCreateShader)( GLenum type ) ;
	typedef void (GLEX_APICALL * API_glDeleteProgram)( GLuint program ) ;
	typedef void (GLEX_APICALL * API_glDeleteShader)( GLuint shader ) ;
	typedef void (GLEX_APICALL * API_glDetachShader)( GLuint program, GLuint shader ) ;
	typedef void (GLEX_APICALL * API_glDisableVertexAttribArray)( GLuint ) ;
	typedef void (GLEX_APICALL * API_glDrawBuffers)( GLsizei n, const GLenum* bufs ) ;
	typedef void (GLEX_APICALL * API_glEnableVertexAttribArray)( GLuint ) ;
	typedef void (GLEX_APICALL * API_glGetActiveAttrib)
			( GLuint program, GLuint index, GLsizei maxLength,
					GLsizei* length, GLint* size, GLenum* type, GLchar* name ) ;
	typedef void (GLEX_APICALL * API_glGetActiveUniform)
			( GLuint program, GLuint index, GLsizei maxLength,
					GLsizei* length, GLint* size, GLenum* type, GLchar* name ) ;
	typedef void (GLEX_APICALL * API_glGetAttachedShaders)
			( GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders ) ;
	typedef GLint (GLEX_APICALL * API_glGetAttribLocation)
							( GLuint program, const GLchar* name ) ;
	typedef void (GLEX_APICALL * API_glGetProgramInfoLog)
			( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog ) ;
	typedef void (GLEX_APICALL * API_glGetProgramiv)
							( GLuint program, GLenum pname, GLint* param ) ;
	typedef void (GLEX_APICALL * API_glGetShaderInfoLog)
			( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog ) ;
	typedef void (GLEX_APICALL * API_glGetShaderSource)
			( GLuint obj, GLsizei maxLength, GLsizei* length, GLchar* source ) ;
	typedef void (GLEX_APICALL * API_glGetShaderiv)
							( GLuint shader, GLenum pname, GLint* param ) ;
	typedef GLint (GLEX_APICALL * API_glGetUniformLocation)
							( GLuint program, const GLchar* name ) ;
	typedef void (GLEX_APICALL * API_glGetUniformfv)
						( GLuint program, GLint location, GLfloat* params ) ;
	typedef void (GLEX_APICALL * API_glGetUniformiv)
						( GLuint program, GLint location, GLint* params ) ;
	typedef void (GLEX_APICALL * API_glGetVertexAttribPointerv)( GLuint, GLenum, GLvoid** ) ;
	typedef void (GLEX_APICALL * API_glGetVertexAttribdv)( GLuint, GLenum, GLdouble* ) ;
	typedef void (GLEX_APICALL * API_glGetVertexAttribfv)( GLuint, GLenum, GLfloat* ) ;
	typedef void (GLEX_APICALL * API_glGetVertexAttribiv)( GLuint, GLenum, GLint* ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsProgram)( GLuint program ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsShader)( GLuint shader ) ;
	typedef void (GLEX_APICALL * API_glLinkProgram)( GLuint program ) ;
	typedef void (GLEX_APICALL * API_glShaderSource)
			( GLuint shader, GLsizei count,
				const GLchar** strings, const GLint* lengths ) ;
	typedef void (GLEX_APICALL * API_glStencilFuncSeparate)
			( GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask ) ;
	typedef void (GLEX_APICALL * API_glStencilMaskSeparate)( GLenum, GLuint ) ;
	typedef void (GLEX_APICALL * API_glStencilOpSeparate)
			( GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass ) ;
	typedef void (GLEX_APICALL * API_glUniform1f)( GLint location, GLfloat v0 ) ;
	typedef void (GLEX_APICALL * API_glUniform1fv)
			( GLint location, GLsizei count, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniform1i)( GLint location, GLint v0 ) ;
	typedef void (GLEX_APICALL * API_glUniform1iv)
					( GLint location, GLsizei count, const GLint* value ) ;
	typedef void (GLEX_APICALL * API_glUniform2f)
					( GLint location, GLfloat v0, GLfloat v1 ) ;
	typedef void (GLEX_APICALL * API_glUniform2fv)
					( GLint location, GLsizei count, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniform2i)( GLint location, GLint v0, GLint v1 ) ;
	typedef void (GLEX_APICALL * API_glUniform2iv)
					( GLint location, GLsizei count, const GLint* value ) ;
	typedef void (GLEX_APICALL * API_glUniform3f)
					( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ) ;
	typedef void (GLEX_APICALL * API_glUniform3fv)
					( GLint location, GLsizei count, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniform3i)
					( GLint location, GLint v0, GLint v1, GLint v2 ) ;
	typedef void (GLEX_APICALL * API_glUniform3iv)
					( GLint location, GLsizei count, const GLint* value ) ;
	typedef void (GLEX_APICALL * API_glUniform4f)
			( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) ;
	typedef void (GLEX_APICALL * API_glUniform4fv)
			( GLint location, GLsizei count, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniform4i)
			( GLint location, GLint v0, GLint v1, GLint v2, GLint v3 ) ;
	typedef void (GLEX_APICALL * API_glUniform4iv)
			( GLint location, GLsizei count, const GLint* value ) ;
	typedef void (GLEX_APICALL * API_glUniformMatrix2fv)
			( GLint location, GLsizei count,
				GLboolean transpose, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniformMatrix3fv)
			( GLint location, GLsizei count,
				GLboolean transpose, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUniformMatrix4fv)
			( GLint location, GLsizei count,
				GLboolean transpose, const GLfloat* value ) ;
	typedef void (GLEX_APICALL * API_glUseProgram)( GLuint program ) ;
	typedef void (GLEX_APICALL * API_glValidateProgram)( GLuint program ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1d)( GLuint index, GLdouble x ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1dv)( GLuint index, const GLdouble* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1f)( GLuint index, GLfloat x ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1fv)( GLuint index, const GLfloat* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1s)( GLuint index, GLshort x ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib1sv)( GLuint index, const GLshort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2d)( GLuint index, GLdouble x, GLdouble y ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2dv)( GLuint index, const GLdouble* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2f)( GLuint index, GLfloat x, GLfloat y ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2fv)( GLuint index, const GLfloat* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2s)( GLuint index, GLshort x, GLshort y ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib2sv)( GLuint index, const GLshort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3d)
					( GLuint index, GLdouble x, GLdouble y, GLdouble z ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3dv)( GLuint index, const GLdouble* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3f)
					( GLuint index, GLfloat x, GLfloat y, GLfloat z ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3fv)( GLuint index, const GLfloat* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3s)
					( GLuint index, GLshort x, GLshort y, GLshort z ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib3sv)( GLuint index, const GLshort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nbv)( GLuint index, const GLbyte* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Niv)( GLuint index, const GLint* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nsv)( GLuint index, const GLshort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nub)
					( GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nubv)( GLuint index, const GLubyte* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nuiv)( GLuint index, const GLuint* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4Nusv)( GLuint index, const GLushort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4bv)( GLuint index, const GLbyte* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4d)
			( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4dv)( GLuint index, const GLdouble* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4f)
			( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4fv)( GLuint index, const GLfloat* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4iv)( GLuint index, const GLint* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4s)
			( GLuint index, GLshort x, GLshort y, GLshort z, GLshort w ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4sv)( GLuint index, const GLshort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4ubv)( GLuint index, const GLubyte* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4uiv)( GLuint index, const GLuint* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttrib4usv)( GLuint index, const GLushort* v ) ;
	typedef void (GLEX_APICALL * API_glVertexAttribPointer)
			( GLuint index, GLint size, GLenum type,
				GLboolean normalized, GLsizei stride, const GLvoid* pointer ) ;

	extern	API_glAttachShader						glAttachShader ;
	extern	API_glBindAttribLocation				glBindAttribLocation ;
	extern	API_glBlendEquationSeparate				glBlendEquationSeparate ;
	extern	API_glCompileShader						glCompileShader ;
	extern	API_glCreateProgram						glCreateProgram ;
	extern	API_glCreateShader						glCreateShader ;
	extern	API_glDeleteProgram						glDeleteProgram ;
	extern	API_glDeleteShader						glDeleteShader ;
	extern	API_glDetachShader						glDetachShader ;
	extern	API_glDisableVertexAttribArray			glDisableVertexAttribArray ;
	extern	API_glDrawBuffers						glDrawBuffers ;
	extern	API_glEnableVertexAttribArray			glEnableVertexAttribArray ;
	extern	API_glGetActiveAttrib					glGetActiveAttrib ;
	extern	API_glGetActiveUniform					glGetActiveUniform ;
	extern	API_glGetAttachedShaders				glGetAttachedShaders ;
	extern	API_glGetAttribLocation					glGetAttribLocation ;
	extern	API_glGetProgramInfoLog					glGetProgramInfoLog ;
	extern	API_glGetProgramiv						glGetProgramiv ;
	extern	API_glGetShaderInfoLog					glGetShaderInfoLog ;
	extern	API_glGetShaderSource					glGetShaderSource ;
	extern	API_glGetShaderiv						glGetShaderiv ;
	extern	API_glGetUniformLocation				glGetUniformLocation ;
	extern	API_glGetUniformfv						glGetUniformfv ;
	extern	API_glGetUniformiv						glGetUniformiv ;
	extern	API_glGetVertexAttribPointerv			glGetVertexAttribPointerv ;
	extern	API_glGetVertexAttribdv					glGetVertexAttribdv ;
	extern	API_glGetVertexAttribfv					glGetVertexAttribfv ;
	extern	API_glGetVertexAttribiv					glGetVertexAttribiv ;
	extern	API_glIsProgram							glIsProgram ;
	extern	API_glIsShader							glIsShader ;
	extern	API_glLinkProgram						glLinkProgram ;
	extern	API_glShaderSource						glShaderSource ;
	extern	API_glStencilFuncSeparate				glStencilFuncSeparate ;
	extern	API_glStencilMaskSeparate				glStencilMaskSeparate ;
	extern	API_glStencilOpSeparate					glStencilOpSeparate ;
	extern	API_glUniform1f							glUniform1f ;
	extern	API_glUniform1fv						glUniform1fv ;
	extern	API_glUniform1i							glUniform1i ;
	extern	API_glUniform1iv						glUniform1iv ;
	extern	API_glUniform2f							glUniform2f ;
	extern	API_glUniform2fv						glUniform2fv ;
	extern	API_glUniform2i							glUniform2i ;
	extern	API_glUniform2iv						glUniform2iv ;
	extern	API_glUniform3f							glUniform3f ;
	extern	API_glUniform3fv						glUniform3fv ;
	extern	API_glUniform3i							glUniform3i ;
	extern	API_glUniform3iv						glUniform3iv ;
	extern	API_glUniform4f							glUniform4f ;
	extern	API_glUniform4fv						glUniform4fv ;
	extern	API_glUniform4i							glUniform4i ;
	extern	API_glUniform4iv						glUniform4iv ;
	extern	API_glUniformMatrix2fv					glUniformMatrix2fv ;
	extern	API_glUniformMatrix3fv					glUniformMatrix3fv ;
	extern	API_glUniformMatrix4fv					glUniformMatrix4fv ;
	extern	API_glUseProgram						glUseProgram ;
	extern	API_glValidateProgram					glValidateProgram ;
	extern	API_glVertexAttrib1d					glVertexAttrib1d ;
	extern	API_glVertexAttrib1dv					glVertexAttrib1dv ;
	extern	API_glVertexAttrib1f					glVertexAttrib1f ;
	extern	API_glVertexAttrib1fv					glVertexAttrib1fv ;
	extern	API_glVertexAttrib1s					glVertexAttrib1s ;
	extern	API_glVertexAttrib1sv					glVertexAttrib1sv ;
	extern	API_glVertexAttrib2d					glVertexAttrib2d ;
	extern	API_glVertexAttrib2dv					glVertexAttrib2dv ;
	extern	API_glVertexAttrib2f					glVertexAttrib2f ;
	extern	API_glVertexAttrib2fv					glVertexAttrib2fv ;
	extern	API_glVertexAttrib2s					glVertexAttrib2s ;
	extern	API_glVertexAttrib2sv					glVertexAttrib2sv ;
	extern	API_glVertexAttrib3d					glVertexAttrib3d ;
	extern	API_glVertexAttrib3dv					glVertexAttrib3dv ;
	extern	API_glVertexAttrib3f					glVertexAttrib3f ;
	extern	API_glVertexAttrib3fv					glVertexAttrib3fv ;
	extern	API_glVertexAttrib3s					glVertexAttrib3s ;
	extern	API_glVertexAttrib3sv					glVertexAttrib3sv ;
	extern	API_glVertexAttrib4Nbv					glVertexAttrib4Nbv ;
	extern	API_glVertexAttrib4Niv					glVertexAttrib4Niv ;
	extern	API_glVertexAttrib4Nsv					glVertexAttrib4Nsv ;
	extern	API_glVertexAttrib4Nub					glVertexAttrib4Nub ;
	extern	API_glVertexAttrib4Nubv					glVertexAttrib4Nubv ;
	extern	API_glVertexAttrib4Nuiv					glVertexAttrib4Nuiv ;
	extern	API_glVertexAttrib4Nusv					glVertexAttrib4Nusv ;
	extern	API_glVertexAttrib4bv					glVertexAttrib4bv ;
	extern	API_glVertexAttrib4d					glVertexAttrib4d ;
	extern	API_glVertexAttrib4dv					glVertexAttrib4dv ;
	extern	API_glVertexAttrib4f					glVertexAttrib4f ;
	extern	API_glVertexAttrib4fv					glVertexAttrib4fv ;
	extern	API_glVertexAttrib4iv					glVertexAttrib4iv ;
	extern	API_glVertexAttrib4s					glVertexAttrib4s ;
	extern	API_glVertexAttrib4sv					glVertexAttrib4sv ;
	extern	API_glVertexAttrib4ubv					glVertexAttrib4ubv ;
	extern	API_glVertexAttrib4uiv					glVertexAttrib4uiv ;
	extern	API_glVertexAttrib4usv					glVertexAttrib4usv ;
	extern	API_glVertexAttribPointer				glVertexAttribPointer ;
#endif

	// GLSL バイナリ関数群
#if	defined(__PLATFORM_ANDROID__) && (ANDROID_API_LEVEL >= 8)
	#if	ANDROID_API_LEVEL < 18
		#define	glGetProgramBinary		glGetProgramBinaryOES
		#define	glProgramBinary			glProgramBinaryOES
	#endif
#else
	typedef void (GLEX_APICALL * API_glGetProgramBinary)
		( GLuint program, GLsizei bufSize,
			GLsizei * length, GLenum * binaryFormat, GLvoid * binary ) ;
	typedef void (GLEX_APICALL * API_glProgramBinary)
		( GLuint program, GLenum binaryFormat,
			const GLvoid * binary, GLsizei length ) ;
	typedef void (GLEX_APICALL * API_glProgramParameteri)
		( GLuint program, GLenum pname, GLint value ) ;

	extern	API_glGetProgramBinary	glGetProgramBinary ;
	extern	API_glProgramBinary		glProgramBinary ;
	extern	API_glProgramParameteri	glProgramParameteri ;
#endif

	// glDrawBuffer, glDrawBuffers 関数
#if	defined(__PLATFORM_ANDROID__)

	extern void GLEX_APICALL glDrawBuffer( GLenum mode ) ;

	#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_API_LEVEL < 18)
	extern void GLEX_APICALL glDrawBuffers( GLsizei n, const GLenum* bufs ) ;
	#endif

#endif

	// Vertex Array Object 関数
#if	defined(__PLATFORM_ANDROID__) && (ANDROID_API_LEVEL >= 8)
	#if	ANDROID_API_LEVEL < 18
		#define	glBindVertexArray		glBindVertexArrayOES
		#define	glDeleteVertexArrays	glDeleteVertexArraysOES
		#define	glGenVertexArrays		glGenVertexArraysOES
		#define	glIsVertexArray			glIsVertexArrayOES
	#endif

#else
	typedef void (GLEX_APICALL * API_glBindVertexArray)( GLuint vao ) ;
	typedef void (GLEX_APICALL * API_glDeleteVertexArrays)( GLsizei n, const GLuint* arrays ) ;
	typedef void (GLEX_APICALL * API_glGenVertexArrays)( GLsizei n, GLuint* arrays ) ;
	typedef GLboolean (GLEX_APICALL * API_glIsVertexArray)( GLuint vao ) ;

	extern API_glBindVertexArray	glBindVertexArray ;
	extern API_glDeleteVertexArrays	glDeleteVertexArrays ;
	extern API_glGenVertexArrays	glGenVertexArrays ;
	extern API_glIsVertexArray		glIsVertexArray ;
#endif

	// 3D テクスチャ関数
#if	defined(__PLATFORM_ANDROID__)
	#if	ANDROID_API_LEVEL < 18
	#define	glTexImage3D		glTexImage3DOES
	#define	glTexSubImage3D		glTexSubImage3DOES
	#endif

#else
	typedef void (GLEX_APICALL * API_glTexImage3D)
			( GLenum target, GLint level, GLint internalFormat,
				GLsizei width, GLsizei height, GLsizei depth,
				GLint border, GLenum format, GLenum type, const GLvoid *pixels ) ;
	typedef void (GLEX_APICALL * API_glTexSubImage3D)
			( GLenum target, GLint level,
				GLint xoffset, GLint yoffset, GLint zoffset,
				GLsizei width, GLsizei height, GLsizei depth,
				GLenum format, GLenum type, const GLvoid *pixels ) ;

	extern	API_glTexImage3D	glTexImage3D ;
	extern	API_glTexSubImage3D	glTexSubImage3D ;

#endif

	// マルチサンプル関数
#if	defined(__PLATFORM_ANDROID__)
#else
	typedef void (GLEX_APICALL * API_glGetMultisamplefv)
			( GLenum pname, GLuint index, GLfloat* val ) ;
	typedef void (GLEX_APICALL * API_glSampleMaski)
			( GLuint index, GLbitfield mask ) ;
	typedef void (GLEX_APICALL * API_glTexImage2DMultisample)
			( GLenum target, GLsizei samples, GLint internalformat,
				GLsizei width, GLsizei height,
				GLboolean fixedsamplelocations ) ;
	typedef void (GLEX_APICALL * API_glTexImage3DMultisample)
			( GLenum target, GLsizei samples, GLint internalformat,
				GLsizei width, GLsizei height, GLsizei depth,
				GLboolean fixedsamplelocations ) ;

	extern	API_glGetMultisamplefv		glGetMultisamplefv ;
	extern	API_glSampleMaski			glSampleMaski ;
	extern	API_glTexImage2DMultisample	glTexImage2DMultisample ;
	extern	API_glTexImage3DMultisample	glTexImage3DMultisample ;

#endif

	// OpenGL 3.1 / 3.3 Instanced Draw 関数
#if	defined(__PLATFORM_ANDROID__)
#else
	typedef void (GLEX_APICALL * API_glDrawArraysInstanced)
			( GLenum mode, GLint first, GLsizei count, GLsizei primcount ) ;
	typedef void (GLEX_APICALL * API_glDrawElementsInstanced)
			( GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount ) ;
	typedef void (GLEX_APICALL * API_glVertexAttribDivisor)
			( GLuint index, GLuint divisor ) ;

	// OpenGL 3.1
	extern	API_glDrawArraysInstanced	glDrawArraysInstanced ;
	extern	API_glDrawElementsInstanced	glDrawElementsInstanced ;
	// OpenGL 3.3
	extern	API_glVertexAttribDivisor	glVertexAttribDivisor ;
#endif

	// OpenGL 4.2 / 4.3 / OpenGL ES 3.1  Compute Shader 関数
#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 21)
	typedef void (GLEX_APICALL * API_glTexStorage1D)
			( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width ) ;
	typedef void (GLEX_APICALL * API_glTexStorage2D)
			( GLenum target, GLsizei levels, GLenum internalformat,
				GLsizei width, GLsizei height ) ;
	typedef void (GLEX_APICALL * API_glTexStorage3D)
			( GLenum target, GLsizei levels, GLenum internalformat,
				GLsizei width, GLsizei height, GLsizei depth ) ;
	typedef void (GLEX_APICALL * API_glBindImageTexture)
			( GLuint unit, GLuint texture, GLint level,
				GLboolean layered, GLint layer, GLenum access, GLenum format ) ;
	typedef void (GLEX_APICALL * API_glMemoryBarrier)( GLbitfield barriers ) ;
	typedef void (GLEX_APICALL * API_glDispatchCompute)
			( GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z ) ;

	// OpenGL 4.2
	extern	API_glTexStorage1D		glTexStorage1D ;
	extern	API_glTexStorage2D		glTexStorage2D ;
	extern	API_glTexStorage3D		glTexStorage3D ;
	extern	API_glBindImageTexture	glBindImageTexture ;
	extern	API_glMemoryBarrier		glMemoryBarrier ;
	// OpenGL 4.3
	extern	API_glDispatchCompute	glDispatchCompute ;
#endif

}

using namespace OpenGLExtension ;

#endif

