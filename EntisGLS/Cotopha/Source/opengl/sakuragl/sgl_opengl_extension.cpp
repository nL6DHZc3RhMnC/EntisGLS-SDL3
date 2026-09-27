
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_opengl_extension.h>

#define	GLEX_DEFINE_GLAPI(x)	OpenGLExtension::API_##x OpenGLExtension::x = NULL

#if	defined(__PLATFORM_WINDOWS__)
#define	GLEX_LOAD_GLAPI(x)		\
	OpenGLExtension::##x = (OpenGLExtension::API_##x) wglGetProcAddress((LPCSTR)#x) ;	\
	if ( OpenGLExtension::##x == NULL )	api_supported = false

#else
#define	GLEX_LOAD_GLAPI(x)		api_supported = false

#endif

#if	defined(__PLATFORM_ANDROID__)
#if	ANDROID_API_LEVEL < 18

#include <esl/esl_java_object.h>

static JNI::JSmartClass *	g_jni_pGLES30 = NULL ;
static jmethodID			g_jmethodDrawBuffers = NULL ;

#endif
#endif


// 初期化関数
/////////////////////////////////////////////////////////////////////////////.

void OpenGLExtension::Initialize( void )
{
#if	defined(__PLATFORM_ANDROID__)
	g_supports_opengl_1_3 = true ;
	g_supports_opengl_1_5 = true ;
	g_supports_opengl_es1_0 = true ;
	g_supports_framebuffer_object = true ;
	g_supports_vertex_array_object = false ;
	g_supports_texture3d = true ;
	g_supports_texture_multisample = false ;
	#if	ANDROID_API_LEVEL >= 8
		g_supports_opengl_2_0 = !g_disable_opengl_2_0 ;
		g_supports_program_binary = !g_disable_program_binary ;
		g_supports_vertex_array_object = true ;
	#endif

	#if	ANDROID_API_LEVEL < 18
		g_supports_multiple_render_target = false ;
		g_supports_instanced_draw = false ;
		g_supports_compute_shader = false ;
		//
		JNIEnv *	env = JNI::GetJNIEnv() ;
		if ( env != NULL )
		{
			jclass	jclsGLES30 = JNI::FindJavaClass( "android/opengl/GLES30" ) ;
			if ( jclsGLES30 != NULL )
			{
				g_jni_pGLES30 = new JNI::JSmartClass( jclsGLES30 ) ;
				g_jni_pGLES30->MakeGlobalRef() ;
				if ( g_jni_pGLES30->GetObject() != NULL )
				{
					g_jmethodDrawBuffers =
						g_jni_pGLES30->GetStaticMethodID
							( "glDrawBuffers", "(I[II)V" ) ;
					if ( g_jmethodDrawBuffers != NULL )
					{
						g_supports_multiple_render_target = true ;
					}
					else
					{
						env->ExceptionClear() ;
					}
				}
				else
				{
					delete	g_jni_pGLES30 ;
					g_jni_pGLES30 = NULL ;
				}
			}
			else
			{
				env->ExceptionClear() ;
			}
		}
	#else
		g_supports_multiple_render_target = true ;
		g_supports_instanced_draw = true ;
		#if	ANDROID_API_LEVEL >= 21
			g_supports_compute_shader = true ;
		#else
			g_supports_compute_shader = false ;
		#endif
	#endif
#else
	bool	api_supported ;

	// OpenGL 1.3 (Multi texture) 関数群
	api_supported = true ;
	GLEX_LOAD_GLAPI(glActiveTexture) ;
	GLEX_LOAD_GLAPI(glClientActiveTexture) ;
	GLEX_LOAD_GLAPI(glCompressedTexImage1D) ;
	GLEX_LOAD_GLAPI(glCompressedTexImage2D) ;
	GLEX_LOAD_GLAPI(glCompressedTexImage3D) ;
	GLEX_LOAD_GLAPI(glCompressedTexSubImage1D) ;
	GLEX_LOAD_GLAPI(glCompressedTexSubImage2D) ;
	GLEX_LOAD_GLAPI(glCompressedTexSubImage3D) ;
	GLEX_LOAD_GLAPI(glGetCompressedTexImage) ;
	GLEX_LOAD_GLAPI(glLoadTransposeMatrixd) ;
	GLEX_LOAD_GLAPI(glLoadTransposeMatrixf) ;
	GLEX_LOAD_GLAPI(glMultTransposeMatrixd) ;
	GLEX_LOAD_GLAPI(glMultTransposeMatrixf) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1d) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1dv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1f) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1fv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1i) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1iv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1s) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord1sv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2d) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2dv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2f) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2fv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2i) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2iv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2s) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord2sv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3d) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3dv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3f) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3fv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3i) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3iv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3s) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord3sv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4d) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4dv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4f) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4fv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4i) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4iv) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4s) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4sv) ;
	GLEX_LOAD_GLAPI(glSampleCoverage) ;
	g_supports_opengl_1_3 = api_supported ;

	// OpenGL 1.5 (VertexBuffer) 関数群
	api_supported = true ;
	GLEX_LOAD_GLAPI(glBeginQuery) ;
	GLEX_LOAD_GLAPI(glBindBuffer) ;
	GLEX_LOAD_GLAPI(glBufferData) ;
	GLEX_LOAD_GLAPI(glBufferSubData) ;
	GLEX_LOAD_GLAPI(glDeleteBuffers) ;
	GLEX_LOAD_GLAPI(glDeleteQueries) ;
	GLEX_LOAD_GLAPI(glEndQuery) ;
	GLEX_LOAD_GLAPI(glGenBuffers) ;
	GLEX_LOAD_GLAPI(glGenQueries) ;
	GLEX_LOAD_GLAPI(glGetBufferParameteriv) ;
	GLEX_LOAD_GLAPI(glGetBufferPointerv) ;
	GLEX_LOAD_GLAPI(glGetBufferSubData) ;
	GLEX_LOAD_GLAPI(glGetQueryObjectiv) ;
	GLEX_LOAD_GLAPI(glGetQueryObjectuiv) ;
	GLEX_LOAD_GLAPI(glGetQueryiv) ;
	GLEX_LOAD_GLAPI(glIsBuffer) ;
	GLEX_LOAD_GLAPI(glIsQuery) ;
	GLEX_LOAD_GLAPI(glMapBuffer) ;
	GLEX_LOAD_GLAPI(glUnmapBuffer) ;
	g_supports_opengl_1_5 = api_supported ;

	// OpenGL ES1.0 互換
	api_supported = true ;
	GLEX_LOAD_GLAPI(glAlphaFuncx) ;
	GLEX_LOAD_GLAPI(glClearColorx) ;
	GLEX_LOAD_GLAPI(glClearDepthx) ;
	GLEX_LOAD_GLAPI(glColor4x) ;
	GLEX_LOAD_GLAPI(glDepthRangex) ;
	GLEX_LOAD_GLAPI(glFogx) ;
	GLEX_LOAD_GLAPI(glFogxv) ;
	GLEX_LOAD_GLAPI(glFrustumf) ;
	GLEX_LOAD_GLAPI(glFrustumx) ;
	GLEX_LOAD_GLAPI(glLightModelx) ;
	GLEX_LOAD_GLAPI(glLightModelxv) ;
	GLEX_LOAD_GLAPI(glLightx) ;
	GLEX_LOAD_GLAPI(glLightxv) ;
	GLEX_LOAD_GLAPI(glLineWidthx) ;
	GLEX_LOAD_GLAPI(glLoadMatrixx) ;
	GLEX_LOAD_GLAPI(glMaterialx) ;
	GLEX_LOAD_GLAPI(glMaterialxv) ;
	GLEX_LOAD_GLAPI(glMultMatrixx) ;
	GLEX_LOAD_GLAPI(glMultiTexCoord4x) ;
	GLEX_LOAD_GLAPI(glNormal3x) ;
	GLEX_LOAD_GLAPI(glOrthof) ;
	GLEX_LOAD_GLAPI(glOrthox) ;
	GLEX_LOAD_GLAPI(glPointSizex) ;
	GLEX_LOAD_GLAPI(glPolygonOffsetx) ;
	GLEX_LOAD_GLAPI(glRotatex) ;
	GLEX_LOAD_GLAPI(glSampleCoveragex) ;
	GLEX_LOAD_GLAPI(glScalex) ;
	GLEX_LOAD_GLAPI(glTexEnvx) ;
	GLEX_LOAD_GLAPI(glTexEnvxv) ;
	GLEX_LOAD_GLAPI(glTexParameterx) ;
	GLEX_LOAD_GLAPI(glTranslatex) ;
	g_supports_opengl_es1_0 = api_supported ;

	// Frame Buffer 関数群
	api_supported = true ;
	GLEX_LOAD_GLAPI(glBindFramebuffer) ;
	GLEX_LOAD_GLAPI(glBindRenderbuffer) ;
	GLEX_LOAD_GLAPI(glBlitFramebuffer) ;
	GLEX_LOAD_GLAPI(glCheckFramebufferStatus) ;
	GLEX_LOAD_GLAPI(glDeleteFramebuffers) ;
	GLEX_LOAD_GLAPI(glDeleteRenderbuffers) ;
	GLEX_LOAD_GLAPI(glFramebufferRenderbuffer) ;
	GLEX_LOAD_GLAPI(glFramebufferTexture1D) ;
	GLEX_LOAD_GLAPI(glFramebufferTexture2D) ;
	GLEX_LOAD_GLAPI(glFramebufferTexture3D) ;
	GLEX_LOAD_GLAPI(glFramebufferTextureLayer) ;
	GLEX_LOAD_GLAPI(glGenFramebuffers) ;
	GLEX_LOAD_GLAPI(glGenRenderbuffers) ;
	GLEX_LOAD_GLAPI(glGenerateMipmap) ;
	GLEX_LOAD_GLAPI(glGetFramebufferAttachmentParameteriv) ;
	GLEX_LOAD_GLAPI(glIsFramebuffer) ;
	GLEX_LOAD_GLAPI(glIsRenderbuffer) ;
	GLEX_LOAD_GLAPI(glRenderbufferStorage) ;
	GLEX_LOAD_GLAPI(glRenderbufferStorageMultisample) ;
	g_supports_framebuffer_object = api_supported ;

	// OpenGL 2.0 (Shader) 関数群
	api_supported = true ;
	GLEX_LOAD_GLAPI(glAttachShader) ;
	GLEX_LOAD_GLAPI(glBindAttribLocation) ;
	GLEX_LOAD_GLAPI(glBlendEquationSeparate) ;
	GLEX_LOAD_GLAPI(glCompileShader) ;
	GLEX_LOAD_GLAPI(glCreateProgram) ;
	GLEX_LOAD_GLAPI(glCreateShader) ;
	GLEX_LOAD_GLAPI(glDeleteProgram) ;
	GLEX_LOAD_GLAPI(glDeleteShader) ;
	GLEX_LOAD_GLAPI(glDetachShader) ;
	GLEX_LOAD_GLAPI(glDisableVertexAttribArray) ;
	GLEX_LOAD_GLAPI(glDrawBuffers) ;
	GLEX_LOAD_GLAPI(glEnableVertexAttribArray) ;
	GLEX_LOAD_GLAPI(glGetActiveAttrib) ;
	GLEX_LOAD_GLAPI(glGetActiveUniform) ;
	GLEX_LOAD_GLAPI(glGetAttachedShaders) ;
	GLEX_LOAD_GLAPI(glGetAttribLocation) ;
	GLEX_LOAD_GLAPI(glGetProgramInfoLog) ;
	GLEX_LOAD_GLAPI(glGetProgramiv) ;
	GLEX_LOAD_GLAPI(glGetShaderInfoLog) ;
	GLEX_LOAD_GLAPI(glGetShaderSource) ;
	GLEX_LOAD_GLAPI(glGetShaderiv) ;
	GLEX_LOAD_GLAPI(glGetUniformLocation) ;
	GLEX_LOAD_GLAPI(glGetUniformfv) ;
	GLEX_LOAD_GLAPI(glGetUniformiv) ;
	GLEX_LOAD_GLAPI(glGetVertexAttribPointerv) ;
	GLEX_LOAD_GLAPI(glGetVertexAttribdv) ;
	GLEX_LOAD_GLAPI(glGetVertexAttribfv) ;
	GLEX_LOAD_GLAPI(glGetVertexAttribiv) ;
	GLEX_LOAD_GLAPI(glIsProgram) ;
	GLEX_LOAD_GLAPI(glIsShader) ;
	GLEX_LOAD_GLAPI(glLinkProgram) ;
	GLEX_LOAD_GLAPI(glShaderSource) ;
	GLEX_LOAD_GLAPI(glStencilFuncSeparate) ;
	GLEX_LOAD_GLAPI(glStencilMaskSeparate) ;
	GLEX_LOAD_GLAPI(glStencilOpSeparate) ;
	GLEX_LOAD_GLAPI(glUniform1f) ;
	GLEX_LOAD_GLAPI(glUniform1fv) ;
	GLEX_LOAD_GLAPI(glUniform1i) ;
	GLEX_LOAD_GLAPI(glUniform1iv) ;
	GLEX_LOAD_GLAPI(glUniform2f) ;
	GLEX_LOAD_GLAPI(glUniform2fv) ;
	GLEX_LOAD_GLAPI(glUniform2i) ;
	GLEX_LOAD_GLAPI(glUniform2iv) ;
	GLEX_LOAD_GLAPI(glUniform3f) ;
	GLEX_LOAD_GLAPI(glUniform3fv) ;
	GLEX_LOAD_GLAPI(glUniform3i) ;
	GLEX_LOAD_GLAPI(glUniform3iv) ;
	GLEX_LOAD_GLAPI(glUniform4f) ;
	GLEX_LOAD_GLAPI(glUniform4fv) ;
	GLEX_LOAD_GLAPI(glUniform4i) ;
	GLEX_LOAD_GLAPI(glUniform4iv) ;
	GLEX_LOAD_GLAPI(glUniformMatrix2fv) ;
	GLEX_LOAD_GLAPI(glUniformMatrix3fv) ;
	GLEX_LOAD_GLAPI(glUniformMatrix4fv) ;
	GLEX_LOAD_GLAPI(glUseProgram) ;
	GLEX_LOAD_GLAPI(glValidateProgram) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1d) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1dv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1f) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1fv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1s) ;
	GLEX_LOAD_GLAPI(glVertexAttrib1sv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2d) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2dv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2f) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2fv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2s) ;
	GLEX_LOAD_GLAPI(glVertexAttrib2sv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3d) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3dv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3f) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3fv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3s) ;
	GLEX_LOAD_GLAPI(glVertexAttrib3sv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nbv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Niv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nsv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nub) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nubv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nuiv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4Nusv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4bv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4d) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4dv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4f) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4fv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4iv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4s) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4sv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4ubv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4uiv) ;
	GLEX_LOAD_GLAPI(glVertexAttrib4usv) ;
	GLEX_LOAD_GLAPI(glVertexAttribPointer) ;
	g_supports_opengl_2_0 = api_supported && !g_disable_opengl_2_0 ;
	g_supports_multiple_render_target = g_supports_opengl_2_0 ;

	// GLSL バイナリ関数群
	api_supported = true ;
	GLEX_LOAD_GLAPI(glGetProgramBinary) ;
	GLEX_LOAD_GLAPI(glProgramBinary) ;
	GLEX_LOAD_GLAPI(glProgramParameteri) ;
	g_supports_program_binary = api_supported && !g_disable_program_binary ;

	// Vertex Array Object 関数
	api_supported = true ;
	GLEX_LOAD_GLAPI(glBindVertexArray) ;
	GLEX_LOAD_GLAPI(glDeleteVertexArrays) ;
	GLEX_LOAD_GLAPI(glGenVertexArrays) ;
	GLEX_LOAD_GLAPI(glIsVertexArray) ;
	g_supports_vertex_array_object = api_supported ;

	// 3D テクスチャ関数
	api_supported = true ;
	GLEX_LOAD_GLAPI(glTexImage3D) ;
	GLEX_LOAD_GLAPI(glTexSubImage3D) ;
	g_supports_texture3d = api_supported ;

	// マルチサンプル関数
	api_supported = true ;
	GLEX_LOAD_GLAPI(glGetMultisamplefv) ;
	GLEX_LOAD_GLAPI(glSampleMaski) ;
	GLEX_LOAD_GLAPI(glTexImage2DMultisample) ;
	GLEX_LOAD_GLAPI(glTexImage3DMultisample) ;
	g_supports_texture_multisample = api_supported ;

	// OpenGL 3.1 / 3.3 Instanced Draw 関数
	api_supported = true ;
	GLEX_LOAD_GLAPI(glDrawArraysInstanced) ;
	GLEX_LOAD_GLAPI(glDrawElementsInstanced) ;
	GLEX_LOAD_GLAPI(glVertexAttribDivisor) ;
	g_supports_instanced_draw = api_supported ;

	// OpenGL 4.2 / 4.3 / OpenGL ES 3.0  Compute Shader
	api_supported = true ;
	GLEX_LOAD_GLAPI(glTexStorage1D) ;
	GLEX_LOAD_GLAPI(glTexStorage2D) ;
	GLEX_LOAD_GLAPI(glTexStorage3D) ;
	GLEX_LOAD_GLAPI(glBindImageTexture) ;
	GLEX_LOAD_GLAPI(glMemoryBarrier) ;
	GLEX_LOAD_GLAPI(glDispatchCompute) ;
	g_supports_compute_shader = api_supported ;
#endif
}


// 終了関数
/////////////////////////////////////////////////////////////////////////////.

void OpenGLExtension::Finalize( void )
{
#if	defined(__PLATFORM_ANDROID__)
	#if	ANDROID_API_LEVEL < 18
		delete	g_jni_pGLES30 ;
		g_jni_pGLES30 = NULL ;
	#endif
#endif
}


// 有効関数フラグ
/////////////////////////////////////////////////////////////////////////////.

bool	OpenGLExtension::g_supports_opengl_1_3 = false ;
bool	OpenGLExtension::g_supports_opengl_1_5 = false ;
bool	OpenGLExtension::g_supports_opengl_es1_0 = false ;
bool	OpenGLExtension::g_supports_framebuffer_object = false ;
bool	OpenGLExtension::g_supports_opengl_2_0 = false ;
bool	OpenGLExtension::g_supports_program_binary = false ;
bool	OpenGLExtension::g_supports_multiple_render_target = false ;
bool	OpenGLExtension::g_supports_vertex_array_object = false ;
bool	OpenGLExtension::g_supports_texture3d = false ;
bool	OpenGLExtension::g_supports_texture_multisample = false ;
bool	OpenGLExtension::g_supports_instanced_draw = false ;
bool	OpenGLExtension::g_supports_compute_shader = false ;


// 無効化コンフィグレーション
/////////////////////////////////////////////////////////////////////////////.

bool	OpenGLExtension::g_disable_run_any_threads = false ;
bool	OpenGLExtension::g_disable_texture_non_power_of_2 = false ;
bool	OpenGLExtension::g_disable_element_index_uint = false ;
bool	OpenGLExtension::g_disable_opengl_2_0 = false ;
bool	OpenGLExtension::g_disable_program_binary = false ;
bool	OpenGLExtension::g_disable_compute_shader = false ;


#if	!defined(__PLATFORM_ANDROID__)

// OpenGL 1.3 (Multi texture) 関数群
/////////////////////////////////////////////////////////////////////////////.

GLEX_DEFINE_GLAPI(glActiveTexture) ;
GLEX_DEFINE_GLAPI(glClientActiveTexture) ;
GLEX_DEFINE_GLAPI(glCompressedTexImage1D) ;
GLEX_DEFINE_GLAPI(glCompressedTexImage2D) ;
GLEX_DEFINE_GLAPI(glCompressedTexImage3D) ;
GLEX_DEFINE_GLAPI(glCompressedTexSubImage1D) ;
GLEX_DEFINE_GLAPI(glCompressedTexSubImage2D) ;
GLEX_DEFINE_GLAPI(glCompressedTexSubImage3D) ;
GLEX_DEFINE_GLAPI(glGetCompressedTexImage) ;
GLEX_DEFINE_GLAPI(glLoadTransposeMatrixd) ;
GLEX_DEFINE_GLAPI(glLoadTransposeMatrixf) ;
GLEX_DEFINE_GLAPI(glMultTransposeMatrixd) ;
GLEX_DEFINE_GLAPI(glMultTransposeMatrixf) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1d) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1dv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1f) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1fv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1i) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1iv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1s) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord1sv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2d) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2dv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2f) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2fv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2i) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2iv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2s) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord2sv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3d) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3dv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3f) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3fv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3i) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3iv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3s) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord3sv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4d) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4dv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4f) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4fv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4i) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4iv) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4s) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4sv) ;
GLEX_DEFINE_GLAPI(glSampleCoverage) ;


// OpenGL 1.5 (VertexBuffer) 関数群
/////////////////////////////////////////////////////////////////////////////.

GLEX_DEFINE_GLAPI(glBeginQuery) ;
GLEX_DEFINE_GLAPI(glBindBuffer) ;
GLEX_DEFINE_GLAPI(glBufferData) ;
GLEX_DEFINE_GLAPI(glBufferSubData) ;
GLEX_DEFINE_GLAPI(glDeleteBuffers) ;
GLEX_DEFINE_GLAPI(glDeleteQueries) ;
GLEX_DEFINE_GLAPI(glEndQuery) ;
GLEX_DEFINE_GLAPI(glGenBuffers) ;
GLEX_DEFINE_GLAPI(glGenQueries) ;
GLEX_DEFINE_GLAPI(glGetBufferParameteriv) ;
GLEX_DEFINE_GLAPI(glGetBufferPointerv) ;
GLEX_DEFINE_GLAPI(glGetBufferSubData) ;
GLEX_DEFINE_GLAPI(glGetQueryObjectiv) ;
GLEX_DEFINE_GLAPI(glGetQueryObjectuiv) ;
GLEX_DEFINE_GLAPI(glGetQueryiv) ;
GLEX_DEFINE_GLAPI(glIsBuffer) ;
GLEX_DEFINE_GLAPI(glIsQuery) ;
GLEX_DEFINE_GLAPI(glMapBuffer) ;
GLEX_DEFINE_GLAPI(glUnmapBuffer) ;


// OpenGL ES1.0 互換関数群
/////////////////////////////////////////////////////////////////////////////.

GLEX_DEFINE_GLAPI(glAlphaFuncx) ;
GLEX_DEFINE_GLAPI(glClearColorx) ;
GLEX_DEFINE_GLAPI(glClearDepthx) ;
GLEX_DEFINE_GLAPI(glColor4x) ;
GLEX_DEFINE_GLAPI(glDepthRangex) ;
GLEX_DEFINE_GLAPI(glFogx) ;
GLEX_DEFINE_GLAPI(glFogxv) ;
GLEX_DEFINE_GLAPI(glFrustumf) ;
GLEX_DEFINE_GLAPI(glFrustumx) ;
GLEX_DEFINE_GLAPI(glLightModelx) ;
GLEX_DEFINE_GLAPI(glLightModelxv) ;
GLEX_DEFINE_GLAPI(glLightx) ;
GLEX_DEFINE_GLAPI(glLightxv) ;
GLEX_DEFINE_GLAPI(glLineWidthx) ;
GLEX_DEFINE_GLAPI(glLoadMatrixx) ;
GLEX_DEFINE_GLAPI(glMaterialx) ;
GLEX_DEFINE_GLAPI(glMaterialxv) ;
GLEX_DEFINE_GLAPI(glMultMatrixx) ;
GLEX_DEFINE_GLAPI(glMultiTexCoord4x) ;
GLEX_DEFINE_GLAPI(glNormal3x) ;
GLEX_DEFINE_GLAPI(glOrthof) ;
GLEX_DEFINE_GLAPI(glOrthox) ;
GLEX_DEFINE_GLAPI(glPointSizex) ;
GLEX_DEFINE_GLAPI(glPolygonOffsetx) ;
GLEX_DEFINE_GLAPI(glRotatex) ;
GLEX_DEFINE_GLAPI(glSampleCoveragex) ;
GLEX_DEFINE_GLAPI(glScalex) ;
GLEX_DEFINE_GLAPI(glTexEnvx) ;
GLEX_DEFINE_GLAPI(glTexEnvxv) ;
GLEX_DEFINE_GLAPI(glTexParameterx) ;
GLEX_DEFINE_GLAPI(glTranslatex) ;


// Frame Buffer 関数群
/////////////////////////////////////////////////////////////////////////////.

GLEX_DEFINE_GLAPI(glBindFramebuffer) ;
GLEX_DEFINE_GLAPI(glBindRenderbuffer) ;
GLEX_DEFINE_GLAPI(glBlitFramebuffer) ;
GLEX_DEFINE_GLAPI(glCheckFramebufferStatus) ;
GLEX_DEFINE_GLAPI(glDeleteFramebuffers) ;
GLEX_DEFINE_GLAPI(glDeleteRenderbuffers) ;
GLEX_DEFINE_GLAPI(glFramebufferRenderbuffer) ;
GLEX_DEFINE_GLAPI(glFramebufferTexture1D) ;
GLEX_DEFINE_GLAPI(glFramebufferTexture2D) ;
GLEX_DEFINE_GLAPI(glFramebufferTexture3D) ;
GLEX_DEFINE_GLAPI(glFramebufferTextureLayer) ;
GLEX_DEFINE_GLAPI(glGenFramebuffers) ;
GLEX_DEFINE_GLAPI(glGenRenderbuffers) ;
GLEX_DEFINE_GLAPI(glGenerateMipmap) ;
GLEX_DEFINE_GLAPI(glGetFramebufferAttachmentParameteriv) ;
GLEX_DEFINE_GLAPI(glIsFramebuffer) ;
GLEX_DEFINE_GLAPI(glIsRenderbuffer) ;
GLEX_DEFINE_GLAPI(glRenderbufferStorage) ;
GLEX_DEFINE_GLAPI(glRenderbufferStorageMultisample) ;

#endif


// OpenGL 2.0 (Shader) 関数群
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8)

GLEX_DEFINE_GLAPI(glAttachShader) ;
GLEX_DEFINE_GLAPI(glBindAttribLocation) ;
GLEX_DEFINE_GLAPI(glBlendEquationSeparate) ;
GLEX_DEFINE_GLAPI(glCompileShader) ;
GLEX_DEFINE_GLAPI(glCreateProgram) ;
GLEX_DEFINE_GLAPI(glCreateShader) ;
GLEX_DEFINE_GLAPI(glDeleteProgram) ;
GLEX_DEFINE_GLAPI(glDeleteShader) ;
GLEX_DEFINE_GLAPI(glDetachShader) ;
GLEX_DEFINE_GLAPI(glDisableVertexAttribArray) ;
GLEX_DEFINE_GLAPI(glDrawBuffers) ;
GLEX_DEFINE_GLAPI(glEnableVertexAttribArray) ;
GLEX_DEFINE_GLAPI(glGetActiveAttrib) ;
GLEX_DEFINE_GLAPI(glGetActiveUniform) ;
GLEX_DEFINE_GLAPI(glGetAttachedShaders) ;
GLEX_DEFINE_GLAPI(glGetAttribLocation) ;
GLEX_DEFINE_GLAPI(glGetProgramInfoLog) ;
GLEX_DEFINE_GLAPI(glGetProgramiv) ;
GLEX_DEFINE_GLAPI(glGetShaderInfoLog) ;
GLEX_DEFINE_GLAPI(glGetShaderSource) ;
GLEX_DEFINE_GLAPI(glGetShaderiv) ;
GLEX_DEFINE_GLAPI(glGetUniformLocation) ;
GLEX_DEFINE_GLAPI(glGetUniformfv) ;
GLEX_DEFINE_GLAPI(glGetUniformiv) ;
GLEX_DEFINE_GLAPI(glGetVertexAttribPointerv) ;
GLEX_DEFINE_GLAPI(glGetVertexAttribdv) ;
GLEX_DEFINE_GLAPI(glGetVertexAttribfv) ;
GLEX_DEFINE_GLAPI(glGetVertexAttribiv) ;
GLEX_DEFINE_GLAPI(glIsProgram) ;
GLEX_DEFINE_GLAPI(glIsShader) ;
GLEX_DEFINE_GLAPI(glLinkProgram) ;
GLEX_DEFINE_GLAPI(glShaderSource) ;
GLEX_DEFINE_GLAPI(glStencilFuncSeparate) ;
GLEX_DEFINE_GLAPI(glStencilMaskSeparate) ;
GLEX_DEFINE_GLAPI(glStencilOpSeparate) ;
GLEX_DEFINE_GLAPI(glUniform1f) ;
GLEX_DEFINE_GLAPI(glUniform1fv) ;
GLEX_DEFINE_GLAPI(glUniform1i) ;
GLEX_DEFINE_GLAPI(glUniform1iv) ;
GLEX_DEFINE_GLAPI(glUniform2f) ;
GLEX_DEFINE_GLAPI(glUniform2fv) ;
GLEX_DEFINE_GLAPI(glUniform2i) ;
GLEX_DEFINE_GLAPI(glUniform2iv) ;
GLEX_DEFINE_GLAPI(glUniform3f) ;
GLEX_DEFINE_GLAPI(glUniform3fv) ;
GLEX_DEFINE_GLAPI(glUniform3i) ;
GLEX_DEFINE_GLAPI(glUniform3iv) ;
GLEX_DEFINE_GLAPI(glUniform4f) ;
GLEX_DEFINE_GLAPI(glUniform4fv) ;
GLEX_DEFINE_GLAPI(glUniform4i) ;
GLEX_DEFINE_GLAPI(glUniform4iv) ;
GLEX_DEFINE_GLAPI(glUniformMatrix2fv) ;
GLEX_DEFINE_GLAPI(glUniformMatrix3fv) ;
GLEX_DEFINE_GLAPI(glUniformMatrix4fv) ;
GLEX_DEFINE_GLAPI(glUseProgram) ;
GLEX_DEFINE_GLAPI(glValidateProgram) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1d) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1dv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1f) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1fv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1s) ;
GLEX_DEFINE_GLAPI(glVertexAttrib1sv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2d) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2dv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2f) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2fv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2s) ;
GLEX_DEFINE_GLAPI(glVertexAttrib2sv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3d) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3dv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3f) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3fv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3s) ;
GLEX_DEFINE_GLAPI(glVertexAttrib3sv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nbv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Niv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nsv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nub) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nubv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nuiv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4Nusv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4bv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4d) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4dv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4f) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4fv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4iv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4s) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4sv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4ubv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4uiv) ;
GLEX_DEFINE_GLAPI(glVertexAttrib4usv) ;
GLEX_DEFINE_GLAPI(glVertexAttribPointer) ;


#endif


// GLSL バイナリ関数群
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8)

GLEX_DEFINE_GLAPI(glGetProgramBinary) ;
GLEX_DEFINE_GLAPI(glProgramBinary) ;
GLEX_DEFINE_GLAPI(glProgramParameteri) ;

#endif


// glDrawBuffer, glDrawBuffers 関数
/////////////////////////////////////////////////////////////////////////////.

#if	defined(__PLATFORM_ANDROID__)

void GLEX_APICALL OpenGLExtension::glDrawBuffer( GLenum mode )
{
#if	ANDROID_API_LEVEL >= 8
	glDrawBuffers( 1, &mode ) ;
#endif
}

#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_API_LEVEL < 18)
void GLEX_APICALL OpenGLExtension::glDrawBuffers( GLsizei n, const GLenum* bufs )
{
	if ( (g_jni_pGLES30 != NULL) && (g_jmethodDrawBuffers != NULL) )
	{
		JNI::JavaObject	jobjBufs ;
		JNI::JIntArray	jiaBufs( jobjBufs.CreateIntArray( (jsize) n ) ) ;
		jint *			pBufs = jiaBufs.GetBuffer() ;
		for ( GLsizei i = 0; i < n; i ++ )
		{
			pBufs[i] = (jint) bufs[i] ;
		}
		jiaBufs.ReleaseBuffer() ;
		//
		g_jni_pGLES30->CallStaticVoidMethod
			( g_jmethodDrawBuffers, (jint) n, jobjBufs.GetObject(), (jint) 0 ) ;
	}
}
#endif

#endif


// Vertex Array Object 関数
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8)

GLEX_DEFINE_GLAPI(glBindVertexArray) ;
GLEX_DEFINE_GLAPI(glDeleteVertexArrays) ;
GLEX_DEFINE_GLAPI(glGenVertexArrays) ;
GLEX_DEFINE_GLAPI(glIsVertexArray) ;

#endif


// 3D テクスチャ関数
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8)

GLEX_DEFINE_GLAPI(glTexImage3D) ;
GLEX_DEFINE_GLAPI(glTexSubImage3D) ;

#endif


// マルチサンプル関数
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__)

GLEX_DEFINE_GLAPI(glGetMultisamplefv) ;
GLEX_DEFINE_GLAPI(glSampleMaski) ;
GLEX_DEFINE_GLAPI(glTexImage2DMultisample) ;
GLEX_DEFINE_GLAPI(glTexImage3DMultisample) ;

#endif


// OpenGL 3.1 / 3.3 Instanced Draw 関数
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__)

GLEX_DEFINE_GLAPI(glDrawArraysInstanced) ;
GLEX_DEFINE_GLAPI(glDrawElementsInstanced) ;
GLEX_DEFINE_GLAPI(glVertexAttribDivisor) ;

#endif


// OpenGL 4.2 / 4.3 / OpenGL ES 3.0  Compute Shader 関数
/////////////////////////////////////////////////////////////////////////////.

#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 21)

GLEX_DEFINE_GLAPI(glTexStorage1D) ;
GLEX_DEFINE_GLAPI(glTexStorage2D) ;
GLEX_DEFINE_GLAPI(glTexStorage3D) ;
GLEX_DEFINE_GLAPI(glBindImageTexture) ;
GLEX_DEFINE_GLAPI(glMemoryBarrier) ;
GLEX_DEFINE_GLAPI(glDispatchCompute) ;

#endif


