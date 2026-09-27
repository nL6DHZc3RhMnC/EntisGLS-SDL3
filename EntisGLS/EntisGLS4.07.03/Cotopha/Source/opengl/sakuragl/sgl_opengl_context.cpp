
/*****************************************************************************
                          EntisGLS4 Library
 ****************************************************************************/

#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl_opengl_render_context.h>

using namespace SSystem ;
using namespace SakuraGL ;

#include <sakuragl/glsl_src_bin.h>


//////////////////////////////////////////////////////////////////////////////
// Uniform データ
//////////////////////////////////////////////////////////////////////////////

SGLOpenGLShaderProgram::CustomUniform::CustomUniform( void )
	: m_glIndex( -1 ),
		m_iTexture( 0 ),
		m_glBinding( 0 ),
		m_glAccess( GL_READ_ONLY ),
		m_glFormat( GL_RGBA8 )
{
}

SGLOpenGLShaderProgram::CustomUniform::CustomUniform
	( const SGLOpenGLShaderProgram::CustomUniform& cuni )
	: UniformData( cuni ),
		m_glIndex( cuni.m_glIndex ),
		m_iTexture( cuni.m_iTexture ),
		m_glBinding( cuni.m_glBinding ),
		m_glAccess( cuni.m_glAccess ),
		m_glFormat( cuni.m_glFormat )
{
}

SGLOpenGLShaderProgram::CustomUniform::~CustomUniform( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLOpenGLShaderProgram, S3DCustomShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLShaderProgram::SGLOpenGLShaderProgram( SGLOpenGLContext * pOpenGL )
{
	m_pOpenGL = pOpenGL ;
	m_glProgram = 0 ;
	m_glVertexShader = 0 ;
	m_glGeometryShader = 0 ;
	m_glFragmentShader = 0 ;
	m_glComputeShader = 0 ;
	//
	m_flagComputeShader = false ;
	m_dimComputeLocalSize.x = 1 ;
	m_dimComputeLocalSize.y = 1 ;
	m_dimComputeLocalSize.z = 1 ;
	//
	m_fpAnisotropy = 4.0f ;
	//
	m_flagProgramBinary = false ;
	//
	m_nUseTextures = 0 ;
	m_nUseImages = 0 ;
	m_flagUpdateUniform = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLShaderProgram::~SGLOpenGLShaderProgram( void )
{
//	Release() ;
}

// シェーダープログラム作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::CreateProgram
	( const SGLOpenGLShaderProgram::Source * pVertexSrc, size_t nVertexSrcCount,
		const SGLOpenGLShaderProgram::Source * pGeometrySrc, size_t nGeometrySrcCount,
		const SGLOpenGLShaderProgram::Source * pFragmentSrc, size_t nFragmentSrcCount,
		S3DCustomShader::CompileListener * pListener )
{
	if ( m_glProgram || m_glVertexShader
			|| m_glGeometryShader || m_glFragmentShader )
	{
		return	sglErrFailed ;
	}
	if ( !OpenGLExtension::g_supports_opengl_2_0 )
	{
		return	sglErrNotSupported ;
	}
	//
	// 頂点シェーダーソース準備
	//
	size_t	i, nLength ;
	SString	strVertSrc ;
	for ( i = 0; i < nVertexSrcCount; i ++ )
	{
		strVertSrc += pVertexSrc[i].DecodeSource() ;
	}
	GLchar *		pcVertSrc ;
	GLint			nVertSrcLen ;
	SArray<GLchar>&	aVertSrc = m_aSrcVertex ;
	if ( sizeof(GLchar) == sizeof(BYTE) )
	{
		nVertSrcLen =
			ESLCharset::EncodeToUTF8
				( strVertSrc, (unsigned int) strVertSrc.GetLength(), NULL, 0 ) ;
		aVertSrc.SetLength( nVertSrcLen + 1 ) ;
		ESLCharset::EncodeToUTF8
			( strVertSrc, (unsigned int) strVertSrc.GetLength(),
				(BYTE*) aVertSrc.GetArray(), nVertSrcLen ) ;
	}
	else
	{
		const wchar_t *	pwszVertSrc = strVertSrc ;
		nLength = strVertSrc.GetLength() ;
		aVertSrc.SetLength( nLength + 1 ) ;
		nVertSrcLen = (GLint) nLength ;
		pcVertSrc = aVertSrc.GetArray() ;
		for ( i = 0; i <= nLength; i ++ )
		{
			pcVertSrc[i] = (GLchar) pwszVertSrc[i] ;
		}
		aVertSrc.FinishArray() ;
	}
	ESLAssert( aVertSrc.GetLength() > (size_t) nVertSrcLen ) ;
	pcVertSrc = aVertSrc.GetArray() ;
	pcVertSrc[nVertSrcLen] = 0 ;
	//
	// ジオメトリシェーダーソース準備
	//
	SString	strGeomSrc ;
	for ( i = 0; i < nGeometrySrcCount; i ++ )
	{
		strGeomSrc += pGeometrySrc[i].DecodeSource() ;
	}
	GLchar *		pcGeomSrc = nullptr ;
	GLint			nGeomSrcLen = 0 ;
	SArray<GLchar>&	aGeomSrc = m_aSrcGeometry ;
	const wchar_t *	pwszGeomSrc = strGeomSrc ;
	nLength = strGeomSrc.GetLength() ;
	if ( nLength > 0 )
	{
		if ( sizeof(GLchar) == sizeof(BYTE) )
		{
			nGeomSrcLen =
				ESLCharset::EncodeToUTF8
					( strGeomSrc, (unsigned int) strGeomSrc.GetLength(), NULL, 0 ) ;
			aGeomSrc.SetLength( nGeomSrcLen + 1 ) ;
			ESLCharset::EncodeToUTF8
				( strGeomSrc, (unsigned int) strGeomSrc.GetLength(),
					(BYTE*) aGeomSrc.GetArray(), nGeomSrcLen ) ;
		}
		else
		{
			aGeomSrc.SetLength( nLength + 1 ) ;
			pcGeomSrc = aGeomSrc.GetArray() ;
			nGeomSrcLen = (GLint) nLength ;
			for ( i = 0; i <= nLength; i ++ )
			{
				pcGeomSrc[i] = (GLchar) pwszGeomSrc[i] ;
			}
			aGeomSrc.FinishArray() ;
		}
		ESLAssert( aGeomSrc.GetLength() > (size_t) nGeomSrcLen ) ;
		pcGeomSrc = aGeomSrc.GetArray() ;
		pcGeomSrc[nGeomSrcLen] = 0 ;
	}
	//
	// フラグメントシェーダーソース準備
	//
	SString	strFragSrc ;
	for ( i = 0; i < nFragmentSrcCount; i ++ )
	{
		strFragSrc += pFragmentSrc[i].DecodeSource() ;
	}
	GLchar *		pcFragSrc ;
	GLint			nFragSrcLen ;
	SArray<GLchar>&	aFragSrc = m_aSrcFragment ;
	if ( sizeof(GLchar) == sizeof(BYTE) )
	{
		nFragSrcLen =
			ESLCharset::EncodeToUTF8
				( strFragSrc, (unsigned int) strFragSrc.GetLength(), NULL, 0 ) ;
		aFragSrc.SetLength( nFragSrcLen + 1 ) ;
		ESLCharset::EncodeToUTF8
			( strFragSrc, (unsigned int) strFragSrc.GetLength(),
				(BYTE*) aFragSrc.GetArray(), nFragSrcLen ) ;
	}
	else
	{
		const wchar_t *	pwszFragSrc = strFragSrc ;
		nLength = strFragSrc.GetLength() ;
		aFragSrc.SetLength( nLength + 1 ) ;
		pcFragSrc = aFragSrc.GetArray() ;
		nFragSrcLen = (GLint) nLength ;
		for ( i = 0; i <= nLength; i ++ )
		{
			pcFragSrc[i] = (GLchar) pwszFragSrc[i] ;
		}
		aFragSrc.FinishArray() ;
	}
	ESLAssert( aFragSrc.GetLength() > (size_t) nFragSrcLen ) ;
	pcFragSrc = aFragSrc.GetArray() ;
	pcFragSrc[nFragSrcLen] = 0 ;
	//
	// シェーダー生成
	//
	m_glVertexShader = glCreateShader( GL_VERTEX_SHADER ) ;
	SGLOpenGLContext::VerifyError( "glCreateShader(GL_VERTEX_SHADER)" ) ;
	if ( m_glVertexShader == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compilePrepared, false ) ;
		}
		Trace( "failed to create GLSL vertex shader.\n" ) ;
		return	sglErrFailed ;
	}
	if ( !strGeomSrc.IsEmpty() )
	{
		// OpenGL 3.2 以降
		m_glGeometryShader = glCreateShader( GL_GEOMETRY_SHADER ) ;
		SGLOpenGLContext::VerifyError( "glCreateShader(GL_GEOMETRY_SHADER)" ) ;
		if ( m_glGeometryShader == 0 )
		{
			if ( pListener != nullptr )
			{
				pListener->OnEvent( this, compilePrepared, false ) ;
			}
			Trace( "failed to create GLSL geometry shader.\n" ) ;
			return	sglErrFailed ;
		}
	}
	m_glFragmentShader = glCreateShader( GL_FRAGMENT_SHADER ) ;
	SGLOpenGLContext::VerifyError( "glCreateShader(GL_FRAGMENT_SHADER)" ) ;
	if ( m_glFragmentShader == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compilePrepared, false ) ;
		}
		Trace( "failed to create GLSL fragment shader.\n" ) ;
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compilePrepared, true ) ;
	}
	//
	// 頂点シェーダーコンパイル
	//
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileStartVertex, true ) ;
	}
	ESLTrace( "compiling GLSL vertex shader.\n" ) ;
	glShaderSource
		( m_glVertexShader, 1, (const GLchar**) &pcVertSrc, &nVertSrcLen ) ;
	SGLOpenGLContext::VerifyError( "glShaderSource" ) ;
	glCompileShader( m_glVertexShader ) ;
	SGLOpenGLContext::VerifyError( "glCompileShader" ) ;
	if ( !IsShaderCompiled( m_glVertexShader, m_strErrorLog ) )
	{
		m_strShaderSrc = strVertSrc ;
		//
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compileEndVertex, false ) ;
		}
		Trace( "failed to compile GLSL vertex shader.\n" ) ;
		Trace( "-------\n%s\n-------\n", m_strShaderSrc.ToCharArray().GetConstArray() ) ;
		return	sglErrFailed ;
	}
	ESLTrace( "completed GLSL vertex shader.\n" ) ;
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileEndVertex, true ) ;
	}
	//
	// ジオメトリシェーダーコンパイル
	//
	if ( !strGeomSrc.IsEmpty() )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compileStartGeometry, true ) ;
		}
		ESLTrace( "compiling GLSL geometry shader.\n" ) ;
		glShaderSource
			( m_glGeometryShader, 1, (const GLchar**) &pcGeomSrc, &nGeomSrcLen ) ;
		SGLOpenGLContext::VerifyError( "glShaderSource" ) ;
		glCompileShader( m_glGeometryShader ) ;
		SGLOpenGLContext::VerifyError( "glCompileShader" ) ;
		if ( !IsShaderCompiled( m_glGeometryShader, m_strErrorLog ) )
		{
			m_strShaderSrc = strGeomSrc ;
			//
			if ( pListener != nullptr )
			{
				pListener->OnEvent( this, compileEndGeometry, false ) ;
			}
			Trace( "failed to compile GLSL geometry shader.\n" ) ;
			Trace( "-------\n%s\n-------\n", m_strShaderSrc.ToCharArray().GetConstArray() ) ;
			return	sglErrFailed ;
		}
		ESLTrace( "completed GLSL geometry shader.\n" ) ;
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compileEndGeometry, true ) ;
		}
	}
	//
	// フラグメントシェーダーコンパイル
	//
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileStartFragment, true ) ;
	}
	ESLTrace( "compiling GLSL fragment shader.\n" ) ;
	glShaderSource
		( m_glFragmentShader, 1, (const GLchar**) &pcFragSrc, &nFragSrcLen ) ;
	SGLOpenGLContext::VerifyError( "glShaderSource" ) ;
	glCompileShader( m_glFragmentShader ) ;
	SGLOpenGLContext::VerifyError( "glCompileShader" ) ;
	if ( !IsShaderCompiled( m_glFragmentShader, m_strErrorLog ) )
	{
		m_strShaderSrc = strFragSrc ;
		//
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compileEndFragment, false ) ;
		}
		Trace( "failed to compile GLSL fragment shader.\n" ) ;
		Trace( "-------\n%s\n-------\n", m_strShaderSrc.ToCharArray().GetConstArray() ) ;
		return	sglErrFailed ;
	}
	ESLTrace( "completed GLSL fragment shader.\n" ) ;
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileEndFragment, true ) ;
	}
	//
	// プログラム作成
	//
	m_glProgram = glCreateProgram() ;
	SGLOpenGLContext::VerifyError( "glCreateProgram" ) ;
	if ( m_glProgram == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkStart, false ) ;
		}
		Trace( "failed to create shader program.\n" ) ;
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkStart, true ) ;
	}
	ESLTrace( "linking GLSL shader program.\n" ) ;
	glAttachShader( m_glProgram, m_glVertexShader ) ;
	SGLOpenGLContext::VerifyError( "glAttachShader" ) ;
	if ( m_glGeometryShader != 0 )
	{
		glAttachShader( m_glProgram, m_glGeometryShader ) ;
		SGLOpenGLContext::VerifyError( "glAttachShader" ) ;
	}
	glAttachShader( m_glProgram, m_glFragmentShader ) ;
	SGLOpenGLContext::VerifyError( "glAttachShader" ) ;
	//
#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL < 8) || (ANDROID_API_LEVEL >= 18)
	if ( OpenGLExtension::g_supports_program_binary
						&& m_pOpenGL->m_flagProgramBinary )
	{
		glProgramParameteri
			( m_glProgram, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE ) ;
		SGLOpenGLContext::VerifyError( "glProgramParameteri" ) ;
	}
#endif
	glLinkProgram( m_glProgram ) ;
	SGLOpenGLContext::VerifyError( "glLinkProgram" ) ;
	//
	glFlush() ;
	SGLOpenGLContext::VerifyError( "glFlush" ) ;
	//
	if ( !IsProgramLinked( m_glProgram, m_strErrorLog ) )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkEnd, false ) ;
		}
		Trace( "failed to link GLSL program.\n" ) ;
		return	sglErrFailed ;
	}
	if ( g_supports_program_binary && m_pOpenGL->m_flagProgramBinary )
	{
		m_flagProgramBinary = GetProgramBinary
			( m_glProgram, m_glBinaryFormat, m_bufProgramBinary ) ;
	}
	ESLTrace( "completed GLSL shader program.\n" ) ;
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkEnd, true ) ;
		pListener->OnEvent( this, compiledProgram, true ) ;
	}
	return	sglErrSuccess ;
}

// コンピュートシェーダープログラム作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::CreateCumputeShader
	( const Source * pSrc, size_t nSrcCount,
		const S3DComputeShaderInterface::DimSize& dimLocalSize,
		CompileListener * pListener )
{
	if ( m_glProgram || m_glComputeShader )
	{
		return	sglErrFailed ;
	}
	if ( !OpenGLExtension::g_supports_opengl_2_0
		|| !OpenGLExtension::g_supports_compute_shader
		|| !(m_pOpenGL->m_flagSupportedComputeShader) )
	{
		return	sglErrNotSupported ;
	}
	//
	// シェーダーソース準備
	//
	size_t	i, nLength ;
	SString	strComputeSrc ;
	for ( i = 0; i < nSrcCount; i ++ )
	{
		strComputeSrc += pSrc[i].DecodeSource() ;
	}
	GLchar *		pcComputeSrc ;
	GLint			nComputeSrcLen ;
	SArray<GLchar>&	aComputeSrc = m_aSrcCompute ;
	if ( sizeof(GLchar) == sizeof(BYTE) )
	{
		nComputeSrcLen =
			ESLCharset::EncodeToUTF8
				( strComputeSrc, (unsigned int) strComputeSrc.GetLength(), NULL, 0 ) ;
		aComputeSrc.SetLength( nComputeSrcLen + 1 ) ;
		ESLCharset::EncodeToUTF8
			( strComputeSrc, (unsigned int) strComputeSrc.GetLength(),
				(BYTE*) aComputeSrc.GetArray(), nComputeSrcLen ) ;
	}
	else
	{
		const wchar_t *	pwszComputeSrc = strComputeSrc ;
		nLength = strComputeSrc.GetLength() ;
		aComputeSrc.SetLength( nLength + 1 ) ;
		nComputeSrcLen = (GLint) nLength ;
		pcComputeSrc = aComputeSrc.GetArray() ;
		for ( i = 0; i <= nLength; i ++ )
		{
			pcComputeSrc[i] = (GLchar) pwszComputeSrc[i] ;
		}
		aComputeSrc.FinishArray() ;
	}
	ESLAssert( aComputeSrc.GetLength() > (size_t) nComputeSrcLen ) ;
	pcComputeSrc = aComputeSrc.GetArray() ;
	pcComputeSrc[nComputeSrcLen] = 0 ;
	//
	// シェーダー生成
	//
	m_glComputeShader = glCreateShader( GL_COMPUTE_SHADER ) ;
	SGLOpenGLContext::VerifyError( "glCreateShader(GL_COMPUTE_SHADER)" ) ;
	if ( m_glComputeShader == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compilePrepared, false ) ;
		}
		Trace( "failed to create GLSL compute shader.\n" ) ;
		return	sglErrFailed ;
	}
	//
	// シェーダーコンパイル
	//
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileStartCompute, true ) ;
	}
	Trace( "compiling GLSL compute shader.\n" ) ;
	glShaderSource
		( m_glComputeShader, 1, (const GLchar**) &pcComputeSrc, &nComputeSrcLen ) ;
	SGLOpenGLContext::VerifyError( "glShaderSource" ) ;
	glCompileShader( m_glComputeShader ) ;
	SGLOpenGLContext::VerifyError( "glCompileShader" ) ;
	if ( !IsShaderCompiled( m_glComputeShader, m_strErrorLog ) )
	{
		m_strShaderSrc = strComputeSrc ;
		//
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, compileEndCompute, false ) ;
		}
		Trace( "failed to compile GLSL compute shader.\n\n" ) ;
		Trace( "-------\n%s\n-------\n", m_strShaderSrc.ToCharArray().GetConstArray() ) ;
		return	sglErrFailed ;
	}
	Trace( "completed GLSL compute shader.\n" ) ;
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, compileEndCompute, true ) ;
	}
	//
	// プログラム作成
	//
	m_glProgram = glCreateProgram() ;
	SGLOpenGLContext::VerifyError( "glCreateProgram" ) ;
	if ( m_glProgram == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkStart, false ) ;
		}
		Trace( "failed to create shader program.\n" ) ;
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkStart, true ) ;
	}
	Trace( "linking GLSL shader program.\n" ) ;
	glAttachShader( m_glProgram, m_glComputeShader ) ;
	SGLOpenGLContext::VerifyError( "glAttachShader" ) ;
	//
	if ( OpenGLExtension::g_supports_program_binary
						&& m_pOpenGL->m_flagProgramBinary )
	{
		glProgramParameteri
			( m_glProgram, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE ) ;
		SGLOpenGLContext::VerifyError( "glProgramParameteri" ) ;
	}
	//
	glLinkProgram( m_glProgram ) ;
	SGLOpenGLContext::VerifyError( "glLinkProgram" ) ;
	//
	glFlush() ;
	SGLOpenGLContext::VerifyError( "glFlush" ) ;
	//
	if ( !IsProgramLinked( m_glProgram, m_strErrorLog ) )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkEnd, false ) ;
		}
		Trace( "failed to link GLSL program.\n" ) ;
		return	sglErrFailed ;
	}
	if ( g_supports_program_binary && m_pOpenGL->m_flagProgramBinary )
	{
		m_flagProgramBinary = GetProgramBinary
			( m_glProgram, m_glBinaryFormat, m_bufProgramBinary ) ;
	}
	Trace( "completed GLSL shader program.\n" ) ;
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkEnd, true ) ;
		pListener->OnEvent( this, compiledProgram, true ) ;
	}
	m_flagComputeShader = true ;
	m_dimComputeLocalSize = dimLocalSize ;
	return	sglErrSuccess ;
}

// シェーダープログラム保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::SaveProgramBinary( S3DShaderBinary& bin )
{
	if ( !m_flagProgramBinary )
	{
		return	sglErrFailed ;
	}
	S3DShaderBinary::Format	fmt ;
	eslFillMemory( &fmt, 0, sizeof(S3DShaderBinary::Format) ) ;
	fmt.typeDev = S3DShaderBinary::deviceOpenGL ;
	fmt.idFormat.glType = m_glBinaryFormat ;
	//
	bin.SetBinary
		( &fmt, sizeof(S3DShaderBinary::Format),
			m_bufProgramBinary.GetArray(), m_bufProgramBinary.GetLength() ) ;
	m_bufProgramBinary.FinishArray() ;
	return	sglErrSuccess ;
}

// シェーダープログラム復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::LoadProgramBinary
	( const S3DShaderBinary& bin,
		S3DCustomShader::CompileListener * pListener )
{
	if ( m_glProgram || m_glVertexShader
			|| m_glGeometryShader || m_glFragmentShader || m_glComputeShader )
	{
		return	sglErrFailed ;
	}
	if ( !OpenGLExtension::g_supports_opengl_2_0 )
	{
		return	sglErrNotSupported ;
	}
	if ( !OpenGLExtension::g_supports_program_binary
				|| !(m_pOpenGL->m_flagProgramBinary) )
	{
		return	sglErrNotSupported ;
	}
	size_t	nFmtBytes, nBinBytes ;
	const S3DShaderBinary::Format *
					pFormat = bin.GetFormat( nFmtBytes ) ;
	const void *	pBinary = bin.GetBinrary( nBinBytes ) ;
	if ( (pFormat == nullptr) || (pBinary == nullptr) )
	{
		return	sglErrFailed ;
	}
	m_glProgram = glCreateProgram() ;
	SGLOpenGLContext::VerifyError( "glCreateProgram" ) ;
	if ( m_glProgram == 0 )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, loadPrepared, false ) ;
		}
		Trace( "failed to create shader program in LoadProgramBinary().\n" ) ;
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, loadPrepared, true ) ;
	}
	glProgramBinary
		( m_glProgram,
			(GLenum) pFormat->idFormat.glType,
			pBinary, (GLsizei) nBinBytes ) ;
	if ( !SGLOpenGLContext::VerifyError( "glProgramBinary" ) )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkStart, false ) ;
		}
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkStart, true ) ;
	}
	if ( !IsProgramLinked( m_glProgram, m_strErrorLog ) )
	{
		if ( pListener != nullptr )
		{
			pListener->OnEvent( this, linkEnd, false ) ;
		}
		return	sglErrFailed ;
	}
	if ( pListener != nullptr )
	{
		pListener->OnEvent( this, linkEnd, true ) ;
		pListener->OnEvent( this, compiledProgram, true ) ;
	}
	return	sglErrSuccess ;
}

// シェーダープログラム解放
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::Release( void )
{
	if ( m_glVertexShader != 0 )
	{
		if ( m_glProgram != 0 )
		{
			glDetachShader( m_glProgram, m_glVertexShader ) ;
			SGLOpenGLContext::VerifyError( "glDetachShader" ) ;
		}
		glDeleteShader( m_glVertexShader ) ;
		SGLOpenGLContext::VerifyError( "glDeleteShader" ) ;
		m_glVertexShader = 0 ;
	}
	if ( m_glGeometryShader != 0 )
	{
		if ( m_glProgram != 0 )
		{
			glDetachShader( m_glProgram, m_glGeometryShader ) ;
			SGLOpenGLContext::VerifyError( "glDetachShader" ) ;
		}
		glDeleteShader( m_glGeometryShader ) ;
		SGLOpenGLContext::VerifyError( "glDeleteShader" ) ;
		m_glGeometryShader = 0 ;
	}
	if ( m_glFragmentShader != 0 )
	{
		if ( m_glProgram != 0 )
		{
			glDetachShader( m_glProgram, m_glFragmentShader ) ;
			SGLOpenGLContext::VerifyError( "glDetachShader" ) ;
		}
		glDeleteShader( m_glFragmentShader ) ;
		SGLOpenGLContext::VerifyError( "glDeleteShader" ) ;
		m_glFragmentShader = 0 ;
	}
	if ( m_glComputeShader != 0 )
	{
		if ( m_glProgram != 0 )
		{
			glDetachShader( m_glProgram, m_glComputeShader ) ;
			SGLOpenGLContext::VerifyError( "glDetachShader" ) ;
		}
		glDeleteShader( m_glComputeShader ) ;
		SGLOpenGLContext::VerifyError( "glDeleteShader" ) ;
		m_glComputeShader = 0 ;
	}
	if ( m_glProgram != 0 )
	{
		glDeleteProgram( m_glProgram ) ;
		m_glProgram = 0 ;
	}
}

// シェーダープログラムが設定された（変更された）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::OnChangedProgram( void )
{
	UpdateCustomUniform( true ) ;
}

// シェーダープログラムが別のプログラムに変更される
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::OnChangingProgram( void )
{
	OnFlushContext() ;

	if ( m_nUseTextures > 0 )
	{
		SPointerArray<SGLImageObject>	bufTemp ;
		LockUniform() ;
		size_t	nCount = m_cusUniform.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			CustomUniform *	pcu = m_cusUniform.GetAt( i ) ;
			if ( (pcu != nullptr)
				&& (pcu->m_type == S3DCustomShader::uniformTexture) )
			{
				if ( pcu->m_nLength >= 2 )
				{
					bufTemp.SetLength( pcu->m_nLength ) ;
					pcu->SetData( pcu->m_type, bufTemp.GetConstArray(), pcu->m_nLength ) ;
				}
				else
				{
					SGLImageObject *	pTemp = nullptr ;
					pcu->SetData( pcu->m_type, &pTemp, 1 ) ;
				}
			}
		}
		UnlockUniform() ;

		for ( size_t i = 0; i < m_nUseTextures; i ++ )
		{
			int	iTexture = GLTextureNumAtUserTexture( i ) ;
			glActiveTexture( GL_TEXTURE0 + iTexture ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
			//
			GLenum	glTarget ;
			if ( m_pOpenGL->IsBindingTexture( iTexture, glTarget ) )
			{
				glBindTexture( glTarget, 0 ) ;
				SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
				//
				m_pOpenGL->SetBindTextureInfo( iTexture, nullptr ) ;
			}
			//
//			#if	!defined(__API_OPEN_GL_ES__)
//			glDisable( GL_TEXTURE_2D ) ;
//			SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
//			#endif
		}
		glActiveTexture( GL_TEXTURE0 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	}

	if ( g_supports_compute_shader && (m_nUseImages > 0) )
	{
		SPointerArray<SGLImageObject>	bufTemp ;
		LockUniform() ;
		size_t	nCount = m_cusUniform.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			CustomUniform *	pcu = m_cusUniform.GetAt( i ) ;
			if ( (pcu != nullptr)
				&& ((pcu->m_type == S3DCustomShader::uniformImageRead)
					|| (pcu->m_type == S3DCustomShader::uniformImageWrite)
					|| (pcu->m_type == S3DCustomShader::uniformImageReadWrite)) )
			{
				for ( size_t j = 0; j < pcu->m_nLength; j ++ )
				{
					glBindImageTexture
						( pcu->m_glBinding + (GLuint) j,
							0, 0, GL_FALSE, 0,
							pcu->m_glAccess, pcu->m_glFormat ) ;
					SGLOpenGLContext::VerifyError( "glBindImageTexture" ) ;
				}
				if ( pcu->m_nLength >= 2 )
				{
					bufTemp.SetLength( pcu->m_nLength ) ;
					pcu->SetData( pcu->m_type, bufTemp.GetConstArray(), pcu->m_nLength ) ;
				}
				else
				{
					SGLImageObject *	pTemp = nullptr ;
					pcu->SetData( pcu->m_type, &pTemp, 1 ) ;
				}
			}
		}
		UnlockUniform() ;
	}
}

// ユニフォーム値更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::UpdateCustomUniform( bool flagForceUpdate )
{
	if ( !(m_flagUpdateUniform || flagForceUpdate) )
	{
		return ;
	}
	LockUniform() ;
	size_t	nCount = m_cusUniform.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		CustomUniform *	pcu = m_cusUniform.GetAt( i ) ;
		if ( (pcu != nullptr) && (pcu->m_flagUpdateData || flagForceUpdate) )
		{
			ReflectCustomUniform( pcu ) ;
		}
	}
	m_flagUpdateUniform = false ;
	UnlockUniform() ;
}

// Flush 処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::OnFlushContext( void )
{
}

// 結合されたソース取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SArray<GLchar>&
	SGLOpenGLShaderProgram::GetVertexShaderSource( void ) const
{
	return	m_aSrcVertex ;
}

const SSystem::SArray<GLchar>&
	SGLOpenGLShaderProgram::GetFragmentShaderSource( void ) const
{
	return	m_aSrcFragment ;
}

const SSystem::SArray<GLchar>&
	SGLOpenGLShaderProgram::GetGeometryShaderSource( void ) const
{
	return	m_aSrcGeometry ;
}

const SSystem::SArray<GLchar>&
	SGLOpenGLShaderProgram::GetComputeShaderSource( void ) const
{
	return	m_aSrcCompute ;
}

// コンパイル／リンクエラーログを取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString&
	SGLOpenGLShaderProgram::GetCompileErrorLog( void ) const
{
	return	m_strErrorLog ;
}

// エラーになったシェーダーソースを取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString&
	SGLOpenGLShaderProgram::GetLastErrorShaderSource( void ) const
{
	return	m_strShaderSrc ;
}

// シェーダーのコンパイル結果をチェック
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLShaderProgram::IsShaderCompiled
		( GLuint glShader, SSystem::SString& strErrorLog )
{
	strErrorLog.FreeArray() ;
	//
	GLint	nLogSize = 0 ;
	glGetShaderiv( glShader, GL_INFO_LOG_LENGTH , &nLogSize ) ;
	SGLOpenGLContext::VerifyError( "glGetShaderiv" ) ;
	if ( nLogSize > 1 )
	{
		SArray<GLchar>	aLogBuf ;
		aLogBuf.SetLength( nLogSize + 1 ) ;
		//
		GLsizei	nInfoSize = 0 ;
		glGetShaderInfoLog
			( glShader, nLogSize + 1, &nInfoSize, aLogBuf.GetArray() ) ;
		aLogBuf.FinishArray() ;
		SGLOpenGLContext::VerifyError( "glGetShaderInfoLog" ) ;
		//
		strErrorLog = (const char*) aLogBuf.GetConstArray() ;
		Trace( "GLSL compile log:\n%s\n", aLogBuf.GetConstArray() ) ;
	}
	GLint	nStatus = GL_FALSE ;
	glGetShaderiv( glShader, GL_COMPILE_STATUS, &nStatus ) ;
	SGLOpenGLContext::VerifyError( "glGetShaderiv" ) ;
	if ( nStatus == GL_FALSE )
	{
		return	false ;
	}
	return	true ;
}

// プログラムのリンク結果をチェック
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLShaderProgram::IsProgramLinked
		( GLuint glProgram, SSystem::SString& strErrorLog )
{
	strErrorLog.FreeArray() ;
	//
	GLint	nLogSize = 0 ;
	glGetProgramiv( glProgram, GL_INFO_LOG_LENGTH , &nLogSize ) ;
	SGLOpenGLContext::VerifyError( "glGetProgramiv" ) ;
	if ( nLogSize > 1 )
	{
		SArray<GLchar>	aLogBuf ;
		aLogBuf.SetLength( nLogSize + 1 ) ;
		//
		GLsizei	nInfoSize = 0 ;
		glGetProgramInfoLog
			( glProgram, nLogSize + 1, &nInfoSize, aLogBuf.GetArray() ) ;
		aLogBuf.FinishArray() ;
		SGLOpenGLContext::VerifyError( "glGetProgramInfoLog" ) ;
		//
		strErrorLog = (const char*) aLogBuf.GetConstArray() ;
		Trace( "GLSL link log:\n%s\n", aLogBuf.GetConstArray() ) ;
	}
	GLint	nStatus = GL_FALSE ;
	glGetProgramiv( glProgram, GL_LINK_STATUS, &nStatus ) ;
	SGLOpenGLContext::VerifyError( "glGetProgramiv" ) ;
	if ( nStatus == GL_FALSE )
	{
		return	false ;
	}
	return	true ;
}

// プログラムバイナリを取得
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLShaderProgram::GetProgramBinary
	( GLuint glProgram, GLenum& glFormat, SSystem::SArray<uint8_t>& bufBinary )
{
	GLint	nBinSize = 0 ;
	glGetProgramiv( glProgram, GL_PROGRAM_BINARY_LENGTH, &nBinSize ) ;
	if ( !SGLOpenGLContext::VerifyError( "glGetProgramiv" ) )
	{
		return	false ;
	}
	GLsizei	nGotBinSize = 0 ;
	glGetProgramBinary
		( glProgram, (GLsizei) nBinSize,
			&nGotBinSize, &glFormat,
			bufBinary.GetArray( (size_t) nBinSize ) ) ;
	bufBinary.FinishArray() ;
	if ( !SGLOpenGLContext::VerifyError( "glGetProgramBinary" ) )
	{
		return	false ;
	}
	bufBinary.SetLength( (size_t) nGotBinSize ) ;
	return	(nGotBinSize != 0) ;
}

// 属性ロケーション取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLOpenGLShaderProgram::GetAttributeLocations
	( const Location * pLocations, size_t nCount ) const
{
	size_t	nSuccessed = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		GLint	loc =
			glGetAttribLocation( m_glProgram, pLocations[i].name ) ;
		*(pLocations[i].location) = loc ;
		if ( loc >= 0 )
		{
			nSuccessed ++ ; 
		}
		else
		{
			//ESLTrace( "failed to glGetAttribLocation %s\n", pLocations[i].name ) ;
		}
	}
	return	nSuccessed ;
}

// ユニフォームロケーション取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLOpenGLShaderProgram::GetUniformLocations
	( const Location * pLocations, size_t nCount ) const
{
	size_t	nSuccessed = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		GLint	loc =
			glGetUniformLocation( m_glProgram, pLocations[i].name ) ;
		*(pLocations[i].location) = loc ;
		if ( loc >= 0 )
		{
			nSuccessed ++ ; 
		}
		else
		{
			//ESLTrace( "failed to glGetUniformLocation %s\n", pLocations[i].name ) ;
		}
	}
	return	nSuccessed ;
}

// 頂点属性配列有効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::EnableVertexAttribArray( GLuint iAttr )
{
	if ( (GLint) iAttr >= 0 )
	{
		glEnableVertexAttribArray( iAttr ) ;
		SGLOpenGLContext::VerifyError( "glEnableVertexAttribArray" ) ;
	}
}

// 頂点属性配列無効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::DisableVertexAttribArray( GLuint iAttr )
{
	if ( (GLint) iAttr >= 0 )
	{
		glDisableVertexAttribArray( iAttr ) ;
		SGLOpenGLContext::VerifyError( "glDisableVertexAttribArray" ) ;
	}
}

// 頂点属性配列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::VertexAttribPointer
	( GLuint iAttr, GLint size, GLenum type,
		GLboolean normalized,
		GLsizei stride, const GLvoid* pointer, GLuint divisor )
{
	if ( (GLint) iAttr >= 0 )
	{
		glVertexAttribPointer( iAttr, size, type, normalized, stride, pointer ) ;
		SGLOpenGLContext::VerifyError( "glVertexAttribPointer" ) ;
		//
		#if	defined(__PLATFORM_ANDROID__)
		#if	ANDROID_API_LEVEL >= 18
			glVertexAttribDivisor( iAttr, divisor ) ;
			SGLOpenGLContext::VerifyError( "glVertexAttribDivisor" ) ;
		#endif
		#else
		if ( g_supports_instanced_draw )
		{
			glVertexAttribDivisor( iAttr, divisor ) ;
			SGLOpenGLContext::VerifyError( "glVertexAttribDivisor" ) ;
		}
		#endif
	}
}

// 異方性フィルタ値取得
//////////////////////////////////////////////////////////////////////////////
float SGLOpenGLShaderProgram::GetAnisotropy( void ) const
{
	return	m_fpAnisotropy ;
}

// 異方性フィルタ値設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::SetAnisotropy( float anisotropy )
{
	m_fpAnisotropy = anisotropy ;
}

// 4x4 行列ユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::glUniformMatrix4f
	( GLint location, GLboolean transpose, const S4DMatrix& mat4 )
{
	ESLAssert( sizeof(float32_t) == sizeof(GLfloat) ) ;
#if	defined(__API_OPEN_GL_ES__)
	if ( transpose )
	{
		GLfloat	mat[4][4] ;
		for ( int i = 0; i < 4; i ++ )
		{
			mat[i][0] = mat4.m[0][i] ;
			mat[i][1] = mat4.m[1][i] ;
			mat[i][2] = mat4.m[2][i] ;
			mat[i][3] = mat4.m[3][i] ;
		}
		glUniformMatrix4fv( location, 1, GL_FALSE, &mat[0][0] ) ;
	}
	else
	{
		glUniformMatrix4fv( location, 1, GL_FALSE, &mat4.m[0][0] ) ;
	}
#else
	glUniformMatrix4fv( location, 1, transpose, &mat4.m[0][0] ) ;
#endif
}

void SGLOpenGLShaderProgram::UniformMatrix4fv
	( GLint location, GLsizei count,
		GLboolean transpose, const S4DMatrix * mat4s )
{
	ESLAssert( sizeof(float32_t) == sizeof(GLfloat) ) ;
#if	defined(__API_OPEN_GL_ES__)
	if ( transpose )
	{
		SArray<GLfloat>	mat4Buf ;
		GLfloat *	pmat4Buf = mat4Buf.GetArray( count * 4 * 4 ) ;
		for ( GLsizei j = 0; j < count; j ++ )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				pmat4Buf[i*4+0] = mat4s[j].m[0][i] ;
				pmat4Buf[i*4+1] = mat4s[j].m[1][i] ;
				pmat4Buf[i*4+2] = mat4s[j].m[2][i] ;
				pmat4Buf[i*4+3] = mat4s[j].m[3][i] ;
			}
			pmat4Buf += 4*4 ;
		}
		mat4Buf.FinishArray() ;
		//
		glUniformMatrix4fv( location, count, GL_FALSE, mat4Buf.GetConstArray() ) ;
	}
	else
	{
		glUniformMatrix4fv( location, count, GL_FALSE, &(mat4s->m[0][0]) ) ;
	}
#else
	glUniformMatrix4fv( location, count, transpose, &(mat4s->m[0][0]) ) ;
#endif
}

// カスタムユニフォーム設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::OnInitCustomUniform( void )
{
}

// カスタムユニフォームテクスチャ数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLOpenGLShaderProgram::GetCustomTextureCount( void ) const
{
	return	m_nUseTextures ;
}

// ユニフォーム指標取得（RegisterCustomUniform で変動）
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLOpenGLShaderProgram::FindCustomUniform( const wchar_t * pszUniform )
{
	return	m_cusUniform.FindAs( pszUniform ) ;
}

// ユニフォーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::SetCustomUniform
	( size_t iUniform,
		S3DCustomShader::UniformType type,
		const void * pData, size_t nCount )
{
	CustomUniform *	pcu = m_cusUniform.GetAt( iUniform ) ;
	if ( pcu == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( pcu->m_type != type )
	{
		if ( type == S3DCustomShader::uniformTexture )
		{
			if ( (pcu->m_type != uniformImageRead)
				&& (pcu->m_type != uniformImageWrite)
				&& (pcu->m_type != uniformImageReadWrite) )
			{
				return	sglErrFailed ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	bool	fOldUpdateUniform = m_flagUpdateUniform ;
	switch ( type )
	{
	case	S3DCustomShader::uniformInt:
	case	S3DCustomShader::uniformFloat:
	case	S3DCustomShader::uniformVector2D:
	case	S3DCustomShader::uniformVector3D:
	case	S3DCustomShader::uniformVector4D:
	case	S3DCustomShader::uniformMatrix2x2:
	case	S3DCustomShader::uniformMatrix3x3:
	case	S3DCustomShader::uniformMatrix4x4:
		m_flagUpdateUniform |= pcu->UpdateData( type, pData, nCount ) ;
		break ;
	case	S3DCustomShader::uniformTexture:
	case	S3DCustomShader::uniformImageRead:
	case	S3DCustomShader::uniformImageWrite:
	case	S3DCustomShader::uniformImageReadWrite:
		pcu->UpdateData( type, pData, nCount ) ;
		pcu->m_flagUpdateData = true ;
		m_flagUpdateUniform = true ;
		break ;
	default:
		return	sglErrFailed ;
	}
	if ( (m_pOpenGL->GetCurrentShaderProgram() == this)
							&& m_pOpenGL->IsOnRenderThread() )
	{
		SGLError	err = ReflectCustomUniform( pcu ) ;
		if ( !err )
		{
			m_flagUpdateUniform = fOldUpdateUniform ;
		}
		return	err ;
	}
	return	sglErrSuccess ;
}

// ユニフォーム指標定義
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::RegisterCustomUniform
	( const wchar_t * pszUniformID,
		S3DCustomShader::UniformType type, size_t nCount )
{
	if ( m_cusUniform.GetAs( pszUniformID ) == nullptr )
	{
		ESLAssert( m_pOpenGL->IsOnRenderThread() ) ;
		SString	strUniform = pszUniformID ;
		CustomUniform *	pcu = new CustomUniform ;
		pcu->m_glIndex =
			glGetUniformLocation
				( m_glProgram,
					(const GLchar*) strUniform.ToCharArray().GetConstArray() ) ;
		if ( pcu->m_glIndex >= 0 )
		{
			pcu->SetType( type, nCount ) ;
			m_cusUniform.Add( pszUniformID, pcu ) ;
			//
			switch ( type )
			{
			case	S3DCustomShader::uniformTexture:
				pcu->m_iTexture = m_nUseTextures ;
				m_nUseTextures += nCount ;
				break ;
			case	S3DCustomShader::uniformImageRead:
			case	S3DCustomShader::uniformImageWrite:
			case	S3DCustomShader::uniformImageReadWrite:
				pcu->m_glBinding = (GLuint) m_nUseImages ;
				if ( type == S3DCustomShader::uniformImageRead )
				{
					pcu->m_glAccess = GL_READ_ONLY ;
				}
				else if ( type == S3DCustomShader::uniformImageWrite )
				{
					pcu->m_glAccess = GL_WRITE_ONLY ;
				}
				else
				{
					pcu->m_glAccess = GL_READ_WRITE ;
				}
				m_nUseImages += nCount ;
				break ;
			default:
				break ;
			}
			m_flagUpdateUniform = true ;
		}
		else
		{
			delete	pcu ;
		}
	}
}

// ユニフォーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLOpenGLShaderProgram::GetCustomUniformCount( void ) const
{
	return	m_cusUniform.GetLength() ;
}

// ユニフォーム取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLShaderProgram::CustomUniform *
	SGLOpenGLShaderProgram::GetCustomUniformAt( size_t nIndex ) const
{
	return	m_cusUniform.GetAt( nIndex ) ;
}

// ユニフォーム値反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLShaderProgram::ReflectCustomUniform
	( SGLOpenGLShaderProgram::CustomUniform * pcu )
{
	size_t				i ;
	int					iTexture ;
	SGLImageObject *	pTexture ;
	SGLImageRect		rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
						pglTexture ;
	SArray<GLint>		aglInts ;
	SArray<GLfloat>		aglFloats ;
	//
	if ( pcu->m_nLength == 1 )
	{
		switch ( pcu->m_type )
		{
		case	S3DCustomShader::uniformInt:
			glUniform1i
				( pcu->m_glIndex, (GLint) *((int32_t*) (pcu->m_pData)) ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i" ) ;
			break ;
		case	S3DCustomShader::uniformFloat:
			glUniform1f
				( pcu->m_glIndex, (GLfloat) pcu->m_bufData[0] ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f" ) ;
			break ;
		case	S3DCustomShader::uniformVector2D:
			glUniform2f
				( pcu->m_glIndex, (GLfloat) pcu->m_bufData[0], (GLfloat) pcu->m_bufData[1] ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f" ) ;
			break ;
		case	S3DCustomShader::uniformVector3D:
			glUniform3f
				( pcu->m_glIndex,
					(GLfloat) pcu->m_bufData[0],
					(GLfloat) pcu->m_bufData[1], (GLfloat) pcu->m_bufData[2] ) ;
			SGLOpenGLContext::VerifyError( "glUniform3f" ) ;
			break ;
		case	S3DCustomShader::uniformVector4D:
			glUniform4f
				( pcu->m_glIndex,
					(GLfloat) pcu->m_bufData[0], (GLfloat) pcu->m_bufData[1],
					(GLfloat) pcu->m_bufData[2], (GLfloat) pcu->m_bufData[3] ) ;
			SGLOpenGLContext::VerifyError( "glUniform4f" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix2x2:
			UniformMatrix2fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix2fv" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix3x3:
			UniformMatrix3fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix3fv" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix4x4:
			UniformMatrix4fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix4fv" ) ;
			break ;
		case	S3DCustomShader::uniformTexture:
			iTexture = GLTextureNumAtUserTexture( pcu->m_iTexture ) ;
			glUniform1i( pcu->m_glIndex, iTexture ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i" ) ;
			//
			glActiveTexture( GL_TEXTURE0 + iTexture ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
			//
			pTexture = *((SGLImageObject**)pcu->m_pData) ;
			pglTexture =
				SGLOpenGLTextureBuffer::CommitGLTextureRsrc
						( m_pOpenGL, pTexture, rectRefTexture ) ;
			if ( pglTexture != nullptr )
			{
				#if	!defined(__API_OPEN_GL_ES__)
				if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
				{
					glEnable( pglTexture->m_paramTarget ) ;
					SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
				}
				#endif
				//
				glBindTexture
					( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
				SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
				//
				if ( pglTexture->m_flagMipmapped && pglTexture->m_flagSmoothable )
				{
					SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
					if ( (pOpenGL != nullptr) && pOpenGL->m_flagAnisotropicExt )
					{
						glTexParameterf
							( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
								esl_fminf( pOpenGL->m_maxAnisotropic, m_fpAnisotropy ) ) ;
						SGLOpenGLContext::VerifyError
							( "glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT)" ) ;
					}
					glTexParameteri
						( pglTexture->m_paramTarget,
								GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) ;
					SGLOpenGLContext::VerifyError
						( "glTexParameteri(GL_TEXTURE_2D,GL_LINEAR_MIPMAP_LINEAR)" ) ;
				}
				else
				{
					glTexParameteri
						( pglTexture->m_paramTarget,
								GL_TEXTURE_MIN_FILTER, pglTexture-> m_paramDefFilter ) ;
					SGLOpenGLContext::VerifyError
						( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
				}
				glTexParameteri
					( pglTexture->m_paramTarget,
							GL_TEXTURE_MAG_FILTER, pglTexture->m_paramDefFilter ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
				glTexParameteri
					( pglTexture->m_paramTarget,
							GL_TEXTURE_WRAP_S, pglTexture->m_paramDefWrap ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
				glTexParameteri
					( pglTexture->m_paramTarget,
							GL_TEXTURE_WRAP_T, pglTexture->m_paramDefWrap ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
				//
				m_pOpenGL->SetBindTextureInfo
					( iTexture, pglTexture, pglTexture->m_paramTarget ) ;
			}
			//
			glActiveTexture( GL_TEXTURE0 ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
			break ;
		case	S3DCustomShader::uniformImageRead:
		case	S3DCustomShader::uniformImageWrite:
		case	S3DCustomShader::uniformImageReadWrite:
			if ( g_supports_compute_shader )
			{
				pTexture = *((SGLImageObject**)pcu->m_pData) ;
				pglTexture =
					SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTexture, rectRefTexture ) ;
				if ( pglTexture != nullptr )
				{
					GLboolean	glLayered = IsLayeredGLImage( pTexture ) ;
					pcu->m_glFormat = GetGLImageFormatOf( pTexture ) ;
					//
					glBindImageTexture
						( pcu->m_glBinding, pglTexture->m_glTexture,
							0, glLayered, 0, pcu->m_glAccess, pcu->m_glFormat ) ;
					if ( !SGLOpenGLContext::VerifyError( "glBindImageTexture" ) )
					{
						Trace( " glBinding = %d, glTexture = %d, glLayered = %d, "
								"glAccess = %x, glFormat = %x\n",
								pcu->m_glBinding, pglTexture->m_glTexture,
								glLayered, pcu->m_glAccess, pcu->m_glFormat ) ;
					}
				}
			}
			break ;
		default:
			return	sglErrFailed ;
		}
	}
	else
	{
		switch ( pcu->m_type )
		{
		case	S3DCustomShader::uniformInt:
			for ( i = 0; i < pcu->m_nLength; i ++ )
			{
				aglInts.Add( (GLint) ((int32_t*)pcu->m_pData)[i] ) ;
			}
			glUniform1iv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglInts.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform1iv" ) ;
			break ;
		case	S3DCustomShader::uniformFloat:
			for ( i = 0; i < pcu->m_nLength; i ++ )
			{
				aglFloats.Add( (GLfloat) ((float32_t*)pcu->m_pData)[i] ) ;
			}
			glUniform1fv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglFloats.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform1fv" ) ;
			break ;
		case	S3DCustomShader::uniformVector2D:
			for ( i = 0; i < pcu->m_nLength*2; i ++ )
			{
				aglFloats.Add( (GLfloat) ((float32_t*)pcu->m_pData)[i] ) ;
			}
			glUniform2fv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglFloats.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform2fv" ) ;
			break ;
		case	S3DCustomShader::uniformVector3D:
			for ( i = 0; i < pcu->m_nLength*3; i ++ )
			{
				aglFloats.Add( (GLfloat) ((float32_t*)pcu->m_pData)[i] ) ;
			}
			glUniform3fv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglFloats.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform3fv" ) ;
			break ;
		case	S3DCustomShader::uniformVector4D:
			for ( i = 0; i < pcu->m_nLength*4; i ++ )
			{
				aglFloats.Add( (GLfloat) ((float32_t*)pcu->m_pData)[i] ) ;
			}
			glUniform4fv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglFloats.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform4fv" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix2x2:
			UniformMatrix2fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix2fv" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix3x3:
			UniformMatrix3fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix3fv" ) ;
			break ;
		case	S3DCustomShader::uniformMatrix4x4:
			UniformMatrix4fv
				( pcu->m_glIndex, (GLsizei) pcu->m_nLength,
									(const float32_t*) pcu->m_pData ) ;
			SGLOpenGLContext::VerifyError( "glUniformMatrix4fv" ) ;
			break ;
		case	S3DCustomShader::uniformTexture:
			for ( i = 0; i < pcu->m_nLength; i ++ )
			{
				iTexture = GLTextureNumAtUserTexture( pcu->m_iTexture + i ) ;
				aglInts.Add( iTexture ) ;
				//
				glActiveTexture( GL_TEXTURE0 + iTexture ) ;
				SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
				//
				pTexture = ((SGLImageObject**)pcu->m_pData)[i] ;
				pglTexture =
					SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTexture, rectRefTexture ) ;
				if ( pglTexture != nullptr )
				{
					#if	!defined(__API_OPEN_GL_ES__)
					glEnable( pglTexture->m_paramTarget ) ;
					SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
					#endif
					//
					glBindTexture
						( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
					SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
					//
					if ( pglTexture->m_flagMipmapped && pglTexture->m_flagSmoothable )
					{
						SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
						if ( (pOpenGL != nullptr) && pOpenGL->m_flagAnisotropicExt )
						{
							glTexParameterf
								( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
									esl_fminf( pOpenGL->m_maxAnisotropic, m_fpAnisotropy ) ) ;
							SGLOpenGLContext::VerifyError
								( "glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT)" ) ;
						}
						glTexParameteri
							( pglTexture->m_paramTarget,
									GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) ;
						SGLOpenGLContext::VerifyError
							( "glTexParameteri(GL_TEXTURE_2D,GL_LINEAR_MIPMAP_LINEAR)" ) ;
					}
					else
					{
						glTexParameteri
							( pglTexture->m_paramTarget,
									GL_TEXTURE_MIN_FILTER, pglTexture-> m_paramDefFilter ) ;
						SGLOpenGLContext::VerifyError
							( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
					}
					glTexParameteri
						( pglTexture->m_paramTarget,
								GL_TEXTURE_MAG_FILTER, pglTexture->m_paramDefFilter ) ;
					SGLOpenGLContext::VerifyError
						( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
					glTexParameteri
						( pglTexture->m_paramTarget,
								GL_TEXTURE_WRAP_S, pglTexture-> m_paramDefWrap ) ;
					SGLOpenGLContext::VerifyError
						( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
					glTexParameteri
						( pglTexture->m_paramTarget,
								GL_TEXTURE_WRAP_T, pglTexture->m_paramDefWrap ) ;
					SGLOpenGLContext::VerifyError
						( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
					//
					m_pOpenGL->SetBindTextureInfo
						( iTexture, pglTexture, pglTexture->m_paramTarget ) ;
				}
			}
			glUniform1iv
				( pcu->m_glIndex,
					(GLsizei) pcu->m_nLength, aglInts.GetConstArray() ) ;
			SGLOpenGLContext::VerifyError( "glUniform1iv" ) ;
			//
			glActiveTexture( GL_TEXTURE0 ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
			break ;
		case	S3DCustomShader::uniformImageRead:
		case	S3DCustomShader::uniformImageWrite:
		case	S3DCustomShader::uniformImageReadWrite:
			if ( g_supports_compute_shader )
			{
				for ( i = 0; i < pcu->m_nLength; i ++ )
				{
					pTexture = ((SGLImageObject**)pcu->m_pData)[i] ;
					pglTexture =
						SGLOpenGLTextureBuffer::CommitGLTextureRsrc
								( m_pOpenGL, pTexture, rectRefTexture ) ;
					if ( pglTexture != nullptr )
					{
						GLboolean	glLayered = IsLayeredGLImage( pTexture ) ;
						pcu->m_glFormat = GetGLImageFormatOf( pTexture ) ;
						//
						glBindImageTexture
							( pcu->m_glBinding + (GLuint) i,
								pglTexture->m_glTexture,
								0, glLayered, 0, pcu->m_glAccess, pcu->m_glFormat ) ;
						SGLOpenGLContext::VerifyError( "glBindImageTexture" ) ;
					}
				}
			}
			break ;
		default:
			return	sglErrFailed ;
		}
	}
	pcu->m_flagUpdateData = false ;
	return	sglErrSuccess ;
}

// ユーザーテクスチャ番号 → OpenGL バインドテクスチャ番号
//////////////////////////////////////////////////////////////////////////////
int SGLOpenGLShaderProgram::GLTextureNumAtUserTexture( size_t iUserTexture ) const
{
	return	(int) iUserTexture + 1 ;
}

// 画像フォーマット → GL画像フォーマット
//////////////////////////////////////////////////////////////////////////////
GLenum SGLOpenGLShaderProgram::GetGLImageFormatOf( SGLImageObject * pTexture )
{
	SGLImageInfo	imginf ;
	if ( (pTexture != nullptr)
		&& !pTexture->GetImageInfo( imginf ) )
	{
		switch ( imginf.format & formatImageTypeMask )
		{
		case	formatImageRGB:
		case	formatImageBGR:
			if ( imginf.depth == 32 )
			{
				return	GL_RGBA8 ;
			}
			break ;

		case	formatImageGray:
			if ( imginf.depth == 8 )
			{
				return	GL_R8 ;
			}
			else if ( (imginf.format & formatImageFlagAlpha) && (imginf.depth == 16) )
			{
				return	GL_RG8 ;
			}
			break ;

		case	(formatImageGray | formatImageFlagFloat):
			if ( imginf.depth == 32 )
			{
				return	GL_R32F ;
			}
			else if ( (imginf.format & formatImageFlagAlpha) && (imginf.depth == 32*2) )
			{
				return	GL_RG32F ;
			}
			break ;

		case	formatImageZ:
		case	formatImageDepth:
			if ( imginf.depth == 32 )
			{
				return	GL_R32F ;
			}
			break ;

		case	formatImageFloatBGR:
			return	GL_RGBA32F ;

		default:
			break ;
		}
	}
	return	GL_RGBA8 ;
}

// layered として画像をバインドするか？
//////////////////////////////////////////////////////////////////////////////
GLboolean SGLOpenGLShaderProgram::IsLayeredGLImage( SGLImageObject * pTexture )
{
	if ( pTexture->GetBufferFlags()
		& (SGLImageObject::bufferTexture3D | SGLImageObject::bufferTextureArray) )
	{
		return	GL_TRUE ;
	}
	return	GL_FALSE ;
}

// 2x2 行列ユニフォーム反映
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::UniformMatrix2fv
	( GLint location, GLsizei count, const float32_t* value )
{
	GLfloat *	pTempBuf = m_bufMatrixTranspose.GetArray( count * (2 * 2) ) ;
	GLfloat *	pNext = pTempBuf ;
	for ( GLsizei i = 0; i < count; i ++ )
	{
		pNext[0] = (GLfloat) value[0] ;
		pNext[1] = (GLfloat) value[2] ;
		pNext[2] = (GLfloat) value[1] ;
		pNext[3] = (GLfloat) value[3] ;
		//
		value += 2 * 2 ;
		pNext += 2 * 2 ;
	}
	glUniformMatrix2fv( location, count, GL_FALSE, pTempBuf ) ;
	m_bufMatrixTranspose.FinishArray() ;
}

// 3x3 行列ユニフォーム反映
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::UniformMatrix3fv
	( GLint location, GLsizei count, const float32_t* value )
{
	GLfloat *	pTempBuf = m_bufMatrixTranspose.GetArray( count * (3 * 3) ) ;
	GLfloat *	pNext = pTempBuf ;
	for ( GLsizei i = 0; i < count; i ++ )
	{
		for ( int j = 0; j < 3; j ++ )
		{
			pNext[0 + j] = (GLfloat) value[j * 3 + 0] ;
			pNext[3 + j] = (GLfloat) value[j * 3 + 1] ;
			pNext[6 + j] = (GLfloat) value[j * 3 + 2] ;
		}
		value += 3 * 3 ;
		pNext += 3 * 3 ;
	}
	glUniformMatrix3fv( location, count, GL_FALSE, pTempBuf ) ;
	m_bufMatrixTranspose.FinishArray() ;
}

// 4x4 行列ユニフォーム反映
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLShaderProgram::UniformMatrix4fv
	( GLint location, GLsizei count, const float32_t* value )
{
	GLfloat *	pTempBuf = m_bufMatrixTranspose.GetArray( count * (4 * 4) ) ;
	GLfloat *	pNext = pTempBuf ;
	for ( GLsizei i = 0; i < count; i ++ )
	{
		for ( int j = 0; j < 4; j ++ )
		{
			pNext[0 + j]  = (GLfloat) value[j * 4 + 0] ;
			pNext[4 + j]  = (GLfloat) value[j * 4 + 1] ;
			pNext[8 + j]  = (GLfloat) value[j * 4 + 2] ;
			pNext[12 + j] = (GLfloat) value[j * 4 + 3] ;
		}
		value += 4 * 4 ;
		pNext += 4 * 4 ;
	}
	glUniformMatrix4fv( location, count, GL_FALSE, pTempBuf ) ;
	m_bufMatrixTranspose.FinishArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL デバイス画像コミット関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLContext::CommitImageProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::CommitImageProcedure::CommitImageProcedure
		( SGLOpenGLContext * pOpenGL,
				SGLImageObject * pImage, size_t nUpdatePixels )
	: m_pOpenGL( pOpenGL ),
		m_refImage( pImage ), m_nUpdatePixels( nUpdatePixels )
{
}

// 実行
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CommitImageProcedure::Run( void )
{
	SGLImageObject *	pImage = m_refImage.GetReference() ;
	if ( pImage != nullptr )
	{
		if ( (m_nUpdatePixels == 0) || m_pOpenGL->IsOnAsyncNoRenderThread() )
		{
			SGLImageRect	rectRefTexture ;
			SGLOpenGLTextureBuffer::CommitGLTexture
						( m_pOpenGL, pImage, rectRefTexture ) ;
		}
		else
		{	// 順次
			STimeCounter	timer ;
			SGLError		err =
				SGLOpenGLTextureBuffer::CommitGLTextureProgressively
									( m_pOpenGL, pImage, m_nUpdatePixels ) ;
			//
			if ( err == sglErrContinue )
			{
				int64_t	nCommitTime = timer.GetTime() ;
				size_t	nNextUpdatePixels = m_nUpdatePixels ;
				if ( nCommitTime < 2 )
				{
					nNextUpdatePixels *= 4 ;
				}
				else if ( nCommitTime > 16 )
				{
					nNextUpdatePixels /= (size_t) (nCommitTime / 16 + 1) ;
					if ( nNextUpdatePixels < 128 * 128 )
					{
						nNextUpdatePixels = 128 * 128 ;
					}
				}
				ESLTrace( "continue commit texture\n" ) ;
				SSyncProcedure *	pSyncProc =
					new SSyncProcedure
						( new CommitImageProcedure
							( m_pOpenGL, pImage, nNextUpdatePixels ), true ) ;
				pSyncProc->SetAutoDelete( true ) ;
				//
				m_pOpenGL->Procedure
					( pSyncProc, S3DRenderDevice::procedureNoRender ) ;
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL デバイス VBO コミット関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLContext::CommitVertexBufferProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::CommitVertexBufferProcedure::CommitVertexBufferProcedure
	( SGLOpenGLContext * pOpenGL, SGLOpenGLVertexBuffer * pVBO )
: m_pOpenGL( pOpenGL ), m_refVBO( pVBO )
{
}

// 実行
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CommitVertexBufferProcedure::Run( void )
{
	SGLOpenGLVertexBuffer *	pVBO = m_refVBO.GetReference() ;
	if ( pVBO != nullptr )
	{
		pVBO->CommitResourceAs( m_pOpenGL ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// OpenGL 標準シェーダーコンパイル関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLContext::CompileDefaultShaderProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::CompileDefaultShaderProcedure::CompileDefaultShaderProcedure
	( SGLOpenGLContext * pOpenGL,
		SGLOpenGLContext::DefaultShaderIndex dsIndex )
: m_pOpenGL( pOpenGL ), m_dsIndex( dsIndex ), m_fResult( false )
{
	m_eventDone.Initialize( false ) ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CompileDefaultShaderProcedure::Run( void )
{
	ESLAssert( m_pOpenGL->IsOnRenderThread() ) ;
	m_fResult = m_pOpenGL->CompileDefaultShader( m_dsIndex ) ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CompileDefaultShaderProcedure::Finalize( void )
{
	glFlush() ;
	m_eventDone.SetSignal() ;
}

// 同期
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLOpenGLContext::CompileDefaultShaderProcedure::Wait( int64_t msecTimeout )
{
	return	m_eventDone.Wait( msecTimeout ) ;
}


//////////////////////////////////////////////////////////////////////////////
// OpenGL カスタムシェーダーコンパイル関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLContext::CompileCustomShaderProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::CompileCustomShaderProcedure::CompileCustomShaderProcedure
	( SGLOpenGLContext * pOpenGL,
		S3DCustomShader * pCustomShader,
		S3DCustomShader::CompileListener * pListener,
		const S3DRenderDevice::ShaderSourceInfo& source )
: m_pOpenGL( pOpenGL ), m_pShader( pCustomShader ),
	m_pListener( pListener ), m_source( source ), m_errResult( sglErrFailed )
{
	m_eventDone.Initialize( false ) ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CompileCustomShaderProcedure::Run( void )
{
	ESLAssert( m_pOpenGL->IsOnRenderThread() ) ;
	m_errResult =
		m_pOpenGL->CompileCustomShader
			( m_pShader, m_source, m_pListener ) ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::CompileCustomShaderProcedure::Finalize( void )
{
	glFlush() ;
	m_eventDone.SetSignal() ;
}

// 同期
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLOpenGLContext::CompileCustomShaderProcedure::Wait( int64_t msecTimeout )
{
	return	m_eventDone.Wait( msecTimeout ) ;
}


//////////////////////////////////////////////////////////////////////////////
// OpenGL カスタムシェーダーロード関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLContext::LoadCustomShaderProcedure, SProcedure )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::LoadCustomShaderProcedure::LoadCustomShaderProcedure
	( SGLOpenGLContext * pOpenGL,
		S3DCustomShader * pCustomShader,
		S3DCustomShader::CompileListener * pListener,
		const S3DShaderBinary * pBinary )
: m_pOpenGL( pOpenGL ), m_pShader( pCustomShader ),
	m_pListener( pListener ), m_pBinary( pBinary ), m_errResult( sglErrFailed )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::LoadCustomShaderProcedure::Run( void )
{
	ESLAssert( m_pOpenGL->IsOnRenderThread() ) ;
	m_errResult =
		m_pOpenGL->LoadCustomShader
			( m_pShader, *m_pBinary, m_pListener ) ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL デバイス
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( bool	SGLOpenGLContext::m_disable_create_std_gouraud_shader = true ) ;
ESL_DLL_DECL( bool	SGLOpenGLContext::m_disable_create_std_phong_shader = true ) ;

atomic_int_t		SGLOpenGLContext::m_countGLEXInit = 0 ;
ESL_DLL_DECL( SGLOpenGLContext *	SGLOpenGLContext::m_pChainFirst = nullptr ) ;

const GLenum	SGLOpenGLContext::s_blendModeParam[blendModeCount][2] =
{
	{ GL_ONE, GL_ONE },
	{ GL_ONE, GL_ZERO },
	{ GL_ONE, GL_ONE_MINUS_SRC_ALPHA },
	{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA },
	{ GL_ZERO, GL_SRC_COLOR },
	{ GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA },
} ;

// パフォーマンスログ・操作
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::PerformanceLog::Reset( void )
{
	countDrawCall = 0 ;
	countDrawInstance = 0 ;
	countDrawVertex = 0 ;
	countTransmitVertex = 0 ;
	countTransmitPixel = 0 ;
	countComputeShapeByCPU = 0 ;
	countSwitchRenderer = 0 ;
	countSwitchShader = 0 ;
	countSwitchMaterial = 0 ;
	msecShapeByCPU = 0.0 ;
	msecRenderingFrame = 0.0 ;
}

void SGLOpenGLContext::PerformanceLog::Add( const SGLOpenGLContext::PerformanceLog& pl )
{
	countDrawCall += pl.countDrawCall ;
	countDrawInstance += pl.countDrawInstance ;
	countDrawVertex += pl.countDrawVertex ;
	countTransmitVertex += pl.countTransmitVertex ;
	countTransmitPixel += pl.countTransmitPixel ;
	countComputeShapeByCPU += pl.countComputeShapeByCPU ;
	countSwitchRenderer += pl.countSwitchRenderer ;
	countSwitchShader += pl.countSwitchShader ;
	countSwitchMaterial += pl.countSwitchMaterial ;
	msecShapeByCPU += pl.msecShapeByCPU ;
	msecRenderingFrame += pl.msecRenderingFrame ;
}

void SGLOpenGLContext::PerformanceLog::Max( const SGLOpenGLContext::PerformanceLog& pl )
{
	countDrawCall =
		(size_t) esl_max( (int) countDrawCall, (int) pl.countDrawCall ) ;
	countDrawInstance =
		(size_t) esl_max( (int) countDrawInstance, (int) pl.countDrawInstance ) ;
	countDrawVertex =
		(size_t) esl_max( (int) countDrawVertex, (int) pl.countDrawVertex ) ;
	countTransmitVertex =
		(size_t) esl_max( (int) countTransmitVertex, (int) pl.countTransmitVertex ) ;
	countTransmitPixel =
		(size_t) esl_max( (int) countTransmitPixel, (int) pl.countTransmitPixel ) ;
	countComputeShapeByCPU =
		(size_t) esl_max( (int) countComputeShapeByCPU, (int) pl.countComputeShapeByCPU ) ;
	countSwitchRenderer =
		(size_t) esl_max( (int) countSwitchRenderer, (int) pl.countSwitchRenderer ) ;
	countSwitchShader =
		(size_t) esl_max( (int) countSwitchShader, (int) pl.countSwitchShader ) ;
	countSwitchMaterial =
		(size_t) esl_max( (int) countSwitchMaterial, (int) pl.countSwitchMaterial ) ;
	msecShapeByCPU = esl_fmax( msecShapeByCPU, pl.msecShapeByCPU ) ;
	msecRenderingFrame = esl_fmax( msecRenderingFrame, pl.msecRenderingFrame ) ;
}

void SGLOpenGLContext::PerformanceLog::ToFrameLog
			( S3DRenderDevice::PerformanceFrameLog& pfl ) const
{
	pfl.countDrawCall = countDrawCall ;
	pfl.countDrawInstance = countDrawInstance ;
	pfl.countDrawVertex = countDrawVertex ;
	pfl.countTransmitVertex = countTransmitVertex ;
	pfl.countTransmitPixel = countTransmitPixel ;
	pfl.countComputeShapeByCPU = countComputeShapeByCPU ;
	pfl.msecShapeByCPU = msecShapeByCPU ;
	pfl.msecRenderingFrame = msecRenderingFrame ;
	pfl.countSwitchRenderer = countSwitchRenderer ;
	pfl.countSwitchShader = countSwitchShader ;
	pfl.countSwitchMaterial = countSwitchMaterial ;
}


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::SGLOpenGLContext, S3DRenderDevice, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::SGLOpenGLContext( void )
{
	m_pglView = nullptr ;
	m_pglFrameBuffer = nullptr ;
	m_pglCurShader = nullptr ;
	for ( int i = 0; i < countDefaultShader; i ++ )
	{
		m_pDefShader[i] = nullptr ;
	}
	m_pCurRenderer = nullptr ;
	m_pShdCompileListener = nullptr ;
	//
	m_pfnSuitableRendering = nullptr ;
	m_pSuitableRenderInstance = nullptr ;
	m_flagReadySuitableThread = false ;
	m_flagQuitSuitableThread = false ;
	m_flagInSuitableProcedure = false ;
	m_flagDoneSuitableProc = false ;
	//
	m_versionGL[0] = 1 ;
	m_versionGL[1] = 0 ;
	m_flagTextureNonPowerOf2 = false ;
	m_flagElementIndexUint = false ;
	m_flagDepthTexture = false ;
	m_flagMultisampling = false ;
	m_flagCubemapTexture = false ;
	m_flagCompressionS3TC = false ;
	m_flagSupportedStereo3D = false ;
	m_flagProgramBinary = false ;
	m_flagProgramBinaryOES = false ;
	m_flagAnisotropicExt = false ;
	m_flagSupportedMRT = false ;
	m_flagSupportedGeometry = false ;
	m_flagSupportedVAO = false ;
	m_flagTextureFloat = false ;
	m_flagColorBufferFloat = false ;
	m_flagAvailableMultiSVB = false ;
	m_flagSupportedComputeShader = false ;
	//
	m_maxMultiTextureUnits = 0 ;
	m_maxTextureImages = 0 ;
	m_maxVSTextureImages = 0 ;
	m_maxCombinedTextureImages = 0 ;
	m_maxTextureSize = 0 ;
	m_max3DTextureSize = 0 ;
	m_maxCubemapTextureSize = 0 ;
	m_maxVertexAttributes = 0 ;
	m_maxVertexVaryings = 0 ;
	m_maxVertexUniforms = 0 ;
	m_maxFragmentUniforms = 0 ;
	m_maxDrawBuffers = 0 ;
	m_maxColorAttachments = 0 ;
	//
	m_maxAnisotropic = 1.0f ;
	//
	m_bytesUsedTexture = 0 ;
	m_bytesMaxUsedTexture = 0 ;
	m_bytesUsedVBO = 0 ;
	m_bytesMaxUsedVBO = 0 ;
	//
	m_pflog.Reset() ;
	m_pflogMax.Reset() ;
	m_pflogTotal.Reset() ;
	m_nTotalLogFrameCount = 0 ;
	//
	for ( int i = 0; i < countBindTxtureNum; i ++ )
	{
		m_glBinTexture[i].pglBind = nullptr ;
		m_glBinTexture[i].glTarget = GL_TEXTURE_2D ;
	}
	m_iTempTexture = 0 ;
	//
	m_pBindingVBO = nullptr ;
	m_pRsrcOfVBO = nullptr ;
	//
	m_pChainNext = nullptr ;
	AddToChain() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext::~SGLOpenGLContext( void )
{
	EndSuitableThread() ;
	DetachFromChain() ;
}

// 初期処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::OnCreateGLContext( void )
{
	if ( m_pChainNext== nullptr )
	{
		AddToChain() ;
	}
	//
	// OpenGL 拡張 API 初期化
	//
	QuickLock() ;
	if ( AtomicAdd( &m_countGLEXInit, 1 ) == 1 )
	{
		OpenGLExtension::Initialize() ;
		//
		Trace( "OpenGL version: %s\n", glGetString( GL_VERSION ) ) ;
		//
		if ( !OpenGLExtension::g_supports_opengl_es1_0 )
		{
			ESLTrace( "OpenGL ES 1.0 is not supported.\n" ) ;
		}
		if ( !OpenGLExtension::g_supports_framebuffer_object )
		{
			ESLTrace( "OpenGL framebuffer is not supported.\n" ) ;
		}
		#if	defined(__PLATFORM_ANDROID__)
		if ( OpenGLExtension::g_supports_multiple_render_target )
		{
			Trace( "OpenGL supports mtuiple render target.\n" ) ;
		}
		#endif
	}
	QuickUnlock() ;
	//
	// OpenGL 拡張機能列挙
	//
	const GLubyte *	pbytExt = glGetString( GL_EXTENSIONS ) ;
	m_extension_supported.RemoveAll() ;
	if ( pbytExt != nullptr )
	{
		size_t	lenExt = 0 ;
		while ( pbytExt[lenExt] )
		{
			lenExt ++ ;
		}
		SString		strExt ;
		uint16_t *	pwBuf = strExt.LockBuffer( lenExt ) ;
		for ( size_t i = 0; i < lenExt; i ++ )
		{
			pwBuf[i] = (uint16_t) pbytExt[i] ;
		}
		strExt.UnlockBuffer( (ssize_t) lenExt ) ;
		//
		SStringParser	sparsExt ;
		sparsExt.AttachString( strExt ) ;
		//
		Trace( "extension supported;\n" ) ;
		while ( sparsExt.PassSpace() )
		{
			SString *	pstrExtName = new SString ;
			if ( sparsExt.NextString( *pstrExtName ) )
			{
				m_extension_supported.Add( pstrExtName ) ;
				Trace( "  + %s\n", pstrExtName->ToCharArray().GetConstArray() ) ;
			}
			else
			{
				delete	pstrExtName ;
				break ;
			}
		}
	}
	//
	// バージョン解釈
	//
	GLint	verGL[2] ;
	bool	fGotVersion = false ;
	glGetIntegerv( GL_MAJOR_VERSION, &verGL[0] ) ;
	if ( SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAJOR_VERSION)" ) )
	{
		glGetIntegerv( GL_MINOR_VERSION, &verGL[1] ) ;
		if ( SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MINOR_VERSION)" ) )
		{
			m_versionGL[0] = (uint32_t) verGL[0] ;
			m_versionGL[1] = (uint32_t) verGL[1] ;
			fGotVersion = true ;
		}
	}
	if ( !fGotVersion )
	{
		const GLubyte *	pbytVer = glGetString( GL_VERSION ) ;
		m_versionGL[0] = 1 ;
		m_versionGL[1] = 0 ;
		if ( pbytVer != nullptr )
		{
			int	i, j = 0 ;
			for ( i = 0; pbytVer[i]; i ++ )
			{
				GLubyte	c = pbytVer[i] ;
				if ( ('0' <= c) && (c <= '9') )
				{
					break ;
				}
			}
			m_versionGL[0] = 0 ;
			m_versionGL[1] = 0 ;
			for ( ; pbytVer[i] && (j < 2); i ++ )
			{
				GLubyte	c = pbytVer[i] ;
				if ( (c >= '0') && (c <= '9') )
				{
					m_versionGL[j] = (m_versionGL[j] * 10) + (c - '0') ;
				}
				else if ( c == '.' )
				{
					j ++ ;
				}
				else
				{
					break ;
				}
			}
		}
	}
	Charset::Decode
		( m_strVender, Charset::encodingUTF8,
			(const uint8_t*) glGetString( GL_VENDOR ) ) ;
	Charset::Decode
		( m_strRenderer, Charset::encodingUTF8,
			(const uint8_t*) glGetString( GL_RENDERER ) ) ;
	Charset::Decode
		( m_strVersion, Charset::encodingUTF8,
			(const uint8_t*) glGetString( GL_VERSION ) ) ;
	//
	Trace( "identified OpenGL version: %d.%d\n", m_versionGL[0], m_versionGL[1] ) ;
	//
	// テクスチャ最大ユニット数取得
	//
	m_maxMultiTextureUnits = 1 ;
	m_maxTextureImages = 1 ;
	m_maxVSTextureImages = 0 ;
	m_maxCombinedTextureImages = 1 ;
	m_maxImageUnits = 0 ;
	if ( OpenGLExtension::g_supports_opengl_1_3 )
	{
		glGetIntegerv( GL_MAX_TEXTURE_UNITS, &m_maxMultiTextureUnits ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_TEXTURE_UNITS)" ) ;
		//
		glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureImages ) ;
		if ( !SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS)" ) )
		{
			m_maxTextureImages = m_maxMultiTextureUnits ;
		}
		//
		glGetIntegerv( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_maxVSTextureImages ) ;
		if ( !SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)" ) )
		{
			m_maxVSTextureImages = 0 ;
		}
		//
		glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxCombinedTextureImages ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)" ) ;
	}
	if ( m_versionGL[0] >= 3 )
	{
		glGetIntegerv( GL_MAX_IMAGE_UNITS, &m_maxImageUnits ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_IMAGE_UNITS)" ) ;
	}
	Trace( "max multi texture units: %d\n", m_maxMultiTextureUnits ) ;
	Trace( "max texture images: %d\n", m_maxTextureImages ) ;
	Trace( "max vertex shader texture images: %d\n", m_maxVSTextureImages ) ;
	Trace( "max combined texture images: %d\n", m_maxCombinedTextureImages ) ;
	Trace( "max image units: %d\n", m_maxImageUnits ) ;
	//
	// 最大テクスチャサイズ取得
	//
	m_maxTextureSize = 1 ;
	m_max3DTextureSize = 1 ;
	m_maxCubemapTextureSize = 1 ;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &m_maxTextureSize ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_TEXTURE_SIZE)" ) ;
	glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &m_max3DTextureSize ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE)" ) ;
	glGetIntegerv( GL_MAX_CUBE_MAP_TEXTURE_SIZE, &m_maxCubemapTextureSize ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE)" ) ;
	Trace( "max texture size: %d\n", m_maxTextureSize ) ;
	Trace( "max 3D texture size: %d\n", m_max3DTextureSize ) ;
	Trace( "max Cubemap texture size: %d\n", m_maxCubemapTextureSize ) ;
	//
	// GLSL 最大 attribute 数取得
	//
	m_maxVertexAttributes = 0 ;
	glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &m_maxVertexAttributes ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_VERTEX_ATTRIBS)" ) ;
	Trace( "max vertex attributes: %d\n", m_maxVertexAttributes ) ;
	//
	// GLSL 最大 varying 数取得
	//
	m_maxVertexVaryings = 0 ;
	glGetIntegerv( GL_MAX_VARYING_VECTORS, &m_maxVertexVaryings ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_VARYING_VECTORS)" ) ;
	Trace( "max varying vectors: %d\n", m_maxVertexVaryings ) ;
	//
	// GLSL 最大 uniform 数取得
	//
	m_maxVertexUniforms = 0 ;
	m_maxFragmentUniforms = 0 ;
	glGetIntegerv( GL_MAX_VERTEX_UNIFORM_VECTORS, &m_maxVertexUniforms ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS)" ) ;
	glGetIntegerv( GL_MAX_FRAGMENT_UNIFORM_VECTORS, &m_maxFragmentUniforms ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS)" ) ;
	Trace( "max vertex uniform vectors: %d\n", m_maxVertexUniforms ) ;
	Trace( "max fragment uniform vectors: %d\n", m_maxFragmentUniforms ) ;
	//
	// MRT 対応取得
	//
	m_maxDrawBuffers = 1 ;
	m_maxColorAttachments = 1 ;
	glGetIntegerv( GL_MAX_DRAW_BUFFERS, &m_maxDrawBuffers ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_DRAW_BUFFERS)" ) ;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &m_maxColorAttachments ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS)" ) ;
	Trace( "max draw buffers: %d\n", m_maxDrawBuffers ) ;
	Trace( "max color attachments: %d\n", m_maxColorAttachments ) ;
	//
	if ( OpenGLExtension::g_supports_multiple_render_target
		&& (m_maxDrawBuffers >= 2) )
	{
		#if	defined(__API_OPEN_GL_ES__)
		m_flagSupportedMRT = (m_versionGL[0] >= 3) ;
		#else
		m_flagSupportedMRT = true ;
		#endif
	}
	//
	// GS対応判定 OpenGL 3.2 / OpenGLS ES 3.2 以降
	//
	m_flagSupportedGeometry =
		IsExtensionSupported( L"GL_EXT_geometry_shader" )
			|| (m_versionGL[0] >= 4)
			|| ((m_versionGL[0] == 3) && (m_versionGL[1] >= 2)) ;
	//
	// VAO 対応判定
	//
	m_flagSupportedVAO =
		OpenGLExtension::g_supports_vertex_array_object
			&& (IsExtensionSupported( L"GL_ARB_vertex_array_object" )
				|| IsExtensionSupported( L"GL_OES_vertex_array_object" )) ;
	//
	// VRAM サイズ取得
	//
	if ( IsExtensionSupported( L"GL_ATI_meminfo" ) )
	{
		#if	!defined(GL_TEXTURE_FREE_MEMORY_ATI)
		enum
		{
			VBO_FREE_MEMORY_ATI				= 0x87FB,
			GL_TEXTURE_FREE_MEMORY_ATI		= 0x87FC,
			RENDERBUFFER_FREE_MEMORY_ATI	= 0x87FD,
		} ;
		#endif
		GLint	nATIMemInfo[4] = { 0, 0, 0, 0 } ;
		glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, &nATIMemInfo[0] ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI)" ) ;
		Trace( "ATI texture free memory; total:%d, available:%d, auxiliary:%d, %d\n",
					nATIMemInfo[0], nATIMemInfo[1], nATIMemInfo[2], nATIMemInfo[3] ) ;
	}
	if ( IsExtensionSupported( L"GL_NVX_gpu_memory_info" ) )
	{
		#if	!defined(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX)
		enum
		{
			GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX			= 0x9047,
			GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX		= 0x9048,
			GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX	= 0x9049,
			GPU_MEMORY_INFO_EVICTION_COUNT_NVX				= 0x904A,
			GPU_MEMORY_INFO_EVICTED_MEMORY_NVX				= 0x904B,
		} ;
		#endif
		GLint	nMemTotal, nMemAvail ;
		glGetIntegerv( GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &nMemTotal ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX)" ) ;
		glGetIntegerv( GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nMemAvail ) ;
		SGLOpenGLContext::VerifyError( "glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX)" ) ;
		Trace( "NVIDIA texture free memory; total:%d, available:%d\n", nMemTotal, nMemAvail ) ;
	}
	//
	// テクスチャ拡張機能
	//
	m_flagTextureNonPowerOf2 =
		!g_disable_texture_non_power_of_2
			&& IsExtensionSupported( L"GL_ARB_texture_non_power_of_two" ) ;
	m_flagDepthTexture =
		IsExtensionSupported( L"GL_ARB_depth_texture" )
			|| IsExtensionSupported( L"GL_OES_depth_texture" )
			|| IsExtensionSupported( L"GL_SGIX_depth_texture" ) ;
	m_flagMultisampling =
		IsExtensionSupported( L"GL_ARB_multisample" )
			|| IsExtensionSupported( L"GL_ARB_texture_multisample" )
			|| IsExtensionSupported( L"GL_ARB_texture_storage_multisample" ) ;
	m_flagCubemapTexture =
		(m_maxCubemapTextureSize >= 16)
			|| IsExtensionSupported( L"GL_EXT_texture_cube_map" )
			|| IsExtensionSupported( L"GL_ARB_texture_cube_map" ) ;
	m_flagCompressionS3TC =
		IsExtensionSupported( L"GL_EXT_texture_compression_s3tc" ) ;
	m_flagAnisotropicExt =
		IsExtensionSupported( L"GL_EXT_texture_filter_anisotropic" ) ;
	if ( m_flagAnisotropicExt )
	{
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropic ) ;
		glTexParameterf
			( GL_TEXTURE_2D,
				GL_TEXTURE_MAX_ANISOTROPY_EXT,
				esl_fminf( m_maxAnisotropic, 4.0f ) ) ;
	}
#if	defined(__API_OPEN_GL_ES__)
		m_flagElementIndexUint =
			!g_disable_element_index_uint
				&& (IsExtensionSupported( L"GL_element_index_uint" )
					|| IsExtensionSupported( L"GL_OES_element_index_uint" )) ;
	#else
		m_flagElementIndexUint = !g_disable_element_index_uint ;
	#endif
	//
	if ( !g_disable_texture_non_power_of_2 && !m_flagTextureNonPowerOf2 )
	{
		if ( m_versionGL[0] >= 2 )
		{
			// OpenGL 2.0 以降は常に非２累乗テクスチャをサポート
			// OpenGL ES 2.0 以降も非２累乗テクスチャをサポート
			// 但し CLAMP_TO_EDGE にしなければならないなど制約あり
			// repeat wrap は２の累乗に正規化したテクスチャを使用すること
			m_flagTextureNonPowerOf2 = true ;
		}
	}
	//
	// 浮動小数点テクスチャ対応判定
	//
	m_flagTextureFloat =
		IsExtensionSupported( L"GL_ARB_texture_float" )
		|| IsExtensionSupported( L"GL_OES_texture_float" )
		|| IsExtensionSupported( L"GL_OES_texture_float_linear" ) ;
	m_flagColorBufferFloat =
		IsExtensionSupported( L"GL_ARB_color_buffer_float" )
		|| IsExtensionSupported( L"GL_EXT_color_buffer_float" ) ;
	//
	// multi shape vertex シェーダー可能判定
	//
	m_flagAvailableMultiSVB =
		OpenGLExtension::g_supports_instanced_draw
			&& (m_maxTextureImages >= 4)
			&& m_flagTextureFloat
			&& (m_maxVSTextureImages >= 1)
			&& (m_maxVertexAttributes >= 10) ;
	S3DRenderDevice::m_availableMultiShapeVB |= m_flagAvailableMultiSVB ;
	//
	if ( m_flagAvailableMultiSVB )
	{
		Trace( "multi variant draw: enabled.\n" ) ;
	}
	//
	// OpenGL Quad Buffer テスト
	//
	m_flagSupportedStereo3D = IsSupportedStereo() ;
	if ( m_flagSupportedStereo3D )
	{
		Trace( "OpenGL supports Quad Buffer.\n" ) ;
	}
	//
	// GLSL バイナリ機能
	//
	GLint	numProgramBinaryFormats = 0 ;
	glGetIntegerv( GL_NUM_PROGRAM_BINARY_FORMATS, &numProgramBinaryFormats ) ;
	SGLOpenGLContext::VerifyError( "glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS)" ) ;
	if ( numProgramBinaryFormats != 0 )
	{
		m_flagProgramBinary = true ;
		Trace( "OpenGL supports %d program binary formats.\n", numProgramBinaryFormats ) ;
	}
	#if	defined(__PLATFORM_ANDROID__)
		#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_API_LEVEL < 18)
			m_flagProgramBinaryOES =
				IsExtensionSupported( L"GL_OES_get_program_binary" ) ;
			if ( m_flagProgramBinaryOES )
			{
				m_flagProgramBinary = true ;
			}
		#endif
	#endif
	//
	// シェーダーバイナリ・デバイス登録
	//
	S3DShaderBinaryLibrary *
			plibShader = S3DShaderBinaryLibrary::GetInstance() ;
	if ( plibShader != nullptr )
	{
		const S3DShaderBinaryLibrary::DeviceInfo *
			pdi = plibShader->GetDeviceInfo( S3DShaderBinary::deviceOpenGL ) ;
		if ( pdi != nullptr )
		{
			if ( (pdi->m_strVender != m_strVender)
				|| (pdi->m_strRenderer != m_strRenderer)
				|| (pdi->m_strVersion != m_strVersion) )
			{
				plibShader->RemoveDeviceBinary
						( S3DShaderBinary::deviceOpenGL ) ;
			}
		}
		plibShader->SetDeviceInfo
			( S3DShaderBinary::deviceOpenGL,
				m_strVender, m_strRenderer, m_strVersion ) ;
	}
	//
	// Compute Shader 使用可能
	//
	m_flagSupportedComputeShader = OpenGLExtension::g_supports_compute_shader
									&& !OpenGLExtension::g_disable_compute_shader ;
	if ( m_flagSupportedComputeShader )
	{
		#if	defined(__API_OPEN_GL_ES__)
		// OpenGL ES 3.1 以降
		if ( (m_versionGL[0] >= 4)
				|| ((m_versionGL[0] == 3) && (m_versionGL[1] >= 1)) )
		#else
		// OpenGL 4.3 以降
		if ( (m_versionGL[0] >= 5)
				|| ((m_versionGL[0] == 4) && (m_versionGL[1] >= 3)) )
		#endif
		{
			m_flagSupportedComputeShader = true ;
			Trace( "OpenGL supports compute shader.\n" ) ;
		}
		else
		{
			m_flagSupportedComputeShader = false ;
		}
	}
	//
	// 標準シェーダープログラム準備
	//
	CompileDefaultShader( indexNonShading ) ;
	//
	if ( !m_disable_create_std_gouraud_shader )
	{
		CompileDefaultShader( indexGouraudShading ) ;
	}
	if ( !m_disable_create_std_phong_shader )
	{
		CompileDefaultShader( indexPhongShading ) ;
	}
	//
	// 初期化
	//
	InitMaterialSetting() ;
	//
	// 通知
	//
	NotifyDeviceReset() ;
}

// 破棄前処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::OnDestroyGLContext( void )
{
	//
	// シェーダー削除
	//
	SaveProgramBinary() ;
	//
	RemoveAllCustomShaders() ;
	//
	m_pglCurShader = nullptr ;
	for ( int i = 0; i < countDefaultShader; i ++ )
	{
		m_pDefShader[i] = nullptr ;
	}
	m_pCurRenderer = nullptr ;
	m_pShdCompileListener = nullptr ;
	//
	// 通知
	//
	NotifyDeviceRelease() ;
	//
	DetachFromChain() ;
	//
	// OpenGL 拡張 API 後始末
	//
	QuickLock() ;
	if ( AtomicSub( &m_countGLEXInit, 1 ) == 0 )
	{
		OpenGLExtension::Finalize() ;
	}
	else
	{
		ESLAssert( m_countGLEXInit >= 0 ) ;
	}
	QuickUnlock() ;
}

// プログラムバイナリ保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::SaveProgramBinary( void )
{
	S3DShaderBinaryLibrary *
			plibShader = S3DShaderBinaryLibrary::GetInstance() ;
	if ( plibShader == nullptr )
	{
		return	sglErrSuccess ;
	}
	if ( !plibShader->IsModified() )
	{
		return	sglErrSuccess ;
	}
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != nullptr )
	{
		SString	strProgramCache ;
		if ( pEnv->GetEnvironmentString
				( strProgramCache, L"cotopha\\opengl\\program_cache" )
			|| pEnv->GetEnvironmentString
				( strProgramCache, L"script\\opengl\\program_cache" ) )
		{
			SSmartPointer<SFileInterface>	pFile =
				SFileOpener::DefaultNewOpenFile
					( strProgramCache, SFileOpener::modeCreate ) ;
			if ( pFile != nullptr )
			{
				uint32_t	nProgramVer = 0 ;
				SString		strProgramVer ;
				if ( pEnv->GetEnvironmentString
						( strProgramVer, L"cotopha\\opengl\\program_ver" )
					|| pEnv->GetEnvironmentString
						( strProgramVer, L"script\\opengl\\program_ver" ) )
				{
					nProgramVer = (uint32_t) strProgramVer.AsInteger() ;
				}
				if ( !plibShader->Save( *pFile, nProgramVer ) )
				{
					return	sglErrSuccess ;
				}
			}
		}
	}
	return	sglErrFailed ;
}

// 標準シェーダーのコンパイル
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::CompileDefaultShader
		( SGLOpenGLContext::DefaultShaderIndex dsIndex )
{
	static const int	typeShading[] =
	{
		0,
		SGLOpenGLDefaultShader::shadingGouraud,
		SGLOpenGLDefaultShader::shadingPhong
	} ;
	const wchar_t *	pwszDefShaderName[] =
	{
		S3DRenderDevice::DefaultShaderId::NonShading,
		S3DRenderDevice::DefaultShaderId::Gouraud,
		S3DRenderDevice::DefaultShaderId::Phong,
	} ;
	static const char *	pszDefShaderDisplay[] =
	{
		"non shading",
		"gouraud shading",
		"phong shading",
	} ;
	if ( m_pDefShader[dsIndex] != nullptr )
	{
		return	true ;
	}
	if ( !IsOnRenderThread() )
	{
		CompileDefaultShaderProcedure	proc( this, dsIndex ) ;
		Procedure( &proc, procedureNoRender ) ;
		proc.Wait() ;
		return	proc.m_fResult ;
	}
	SGLOpenGLDefaultShader *	pshdDef = new SGLOpenGLDefaultShader( this ) ;
	S3DShaderBinaryLibrary *
			plibShader = S3DShaderBinaryLibrary::GetInstance() ;
	if ( m_flagProgramBinary && (plibShader != nullptr) )
	{
		S3DShaderBinary *	pBinary =
			plibShader->GetBinary
				( pwszDefShaderName[dsIndex], S3DShaderBinary::deviceOpenGL ) ;
		if ( pBinary != nullptr )
		{
			if ( !pshdDef->LoadProgramBinary( *pBinary, m_pShdCompileListener ) )
			{
				m_pDefShader[dsIndex] = pshdDef ;
				RegisterShaderProgram( pwszDefShaderName[dsIndex], pshdDef ) ;
				Trace( "enabled %s by GLSL.\n", pszDefShaderDisplay[dsIndex] ) ;
				return	true ;
			}
			else
			{
				delete	pshdDef ;
				pshdDef = new SGLOpenGLDefaultShader( this ) ;
			}
		}
	}
	if ( pshdDef->InitializeProgram
		( typeShading[dsIndex], m_pShdCompileListener ) == sglErrSuccess )
	{
		if ( m_flagProgramBinary && (plibShader != nullptr) )
		{
			S3DShaderBinary *	pBinary = new S3DShaderBinary ;
			if ( pshdDef->SaveProgramBinary( *pBinary ) )
			{
				delete	pBinary ;
			}
			else
			{
				pBinary->SetIdentity( pwszDefShaderName[dsIndex] ) ;
				plibShader->AddBinary( pBinary ) ;
			}
		}
		m_pDefShader[dsIndex] = pshdDef ;
		RegisterShaderProgram( pwszDefShaderName[dsIndex], pshdDef ) ;
		Trace( "enabled %s by GLSL.\n", pszDefShaderDisplay[dsIndex] ) ;
		return	true ;
	}
	else
	{
		delete	pshdDef ;
		return	false ;
	}
}

// カスタムシェーダー・オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * SGLOpenGLContext::NewCustomShader( ShaderProgramType type )
{
	if ( type == S3DRenderDevice::programCompute )
	{
		return	new SGLOpenGLComputeShader( this ) ;
	}
	else
	{
		return	new SGLOpenGLCustomShader( this ) ;
	}
}

// 定義済み標準シェーダー生成／コンパイル／取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader *
	SGLOpenGLContext::GetDefaultShaderProgramAs( const wchar_t * pwszID )
{
	S3DCustomShader *	pShader = GetShaderProgramAs( pwszID ) ;
	if ( pShader != nullptr )
	{
		return	pShader ;
	}
	if ( SString::Compare
		( pwszID, S3DRenderDevice::DefaultShaderId::NonShading ) == 0 )
	{
		if ( !CompileDefaultShader( indexNonShading ) )
		{
			return	nullptr ;
		}
		return	GetStandardShaderProgram( 0 ) ;
	}
	else if ( SString::Compare
		( pwszID, S3DRenderDevice::DefaultShaderId::Gouraud ) == 0 )
	{
		if ( !CompileDefaultShader( indexGouraudShading ) )
		{
			return	nullptr ;
		}
		return	GetStandardShaderProgram( shadingMethodGouraud ) ;
	}
	else if ( SString::Compare
		( pwszID, S3DRenderDevice::DefaultShaderId::Phong ) == 0 )
	{
		if ( !CompileDefaultShader( indexPhongShading ) )
		{
			return	nullptr ;
		}
		return	GetStandardShaderProgram( shadingMethodPhong ) ;
	}
	else
	{
		S3DRenderDevice::ShaderSourceInfo	srcShader ;
		if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::DrawWithDepth ) == 0 )
		{
			SGLOpenGLDrawWithDepthShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLDrawWithDepthShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::DelayLight ) == 0 )
		{
			SGLOpenGLDelayLightShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLDelayLightShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SSGISampler ) == 0 )
		{
			SGLOpenGLSSGISamplingShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSSGISamplingShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SSGIComposer ) == 0 )
		{
			SGLOpenGLSSGIComposerShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSSGIComposerShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::GaussianBlur ) == 0 )
		{
			SGLOpenGLGaussianBlurShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLGaussianBlurShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::RadialGaussianBlur ) == 0 )
		{
			SGLOpenGLGaussianRadialBlurShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLGaussianRadialBlurShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::DepthBlender ) == 0 )
		{
			SGLOpenGLDepthBlenderShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLDepthBlenderShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SimpleMosaic ) == 0 )
		{
			SGLOpenGLSimpleMosaicShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSimpleMosaicShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SimpleWater ) == 0 )
		{
			SGLOpenGLSimpleWaterShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSimpleWaterShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::ShadowmapFilter ) == 0 )
		{
			S3DOpenGLShadowmapDepthFilter5x5Shader::GetSourceInfo( srcShader ) ;
			pShader = new S3DOpenGLShadowmapDepthFilter5x5Shader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SimpleWireFrame ) == 0 )
		{
			SGLOpenGLSimpleTriangle2LineShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSimpleTriangle2LineShader( this ) ;
		}
		else if ( SString::Compare
			( pwszID, S3DRenderDevice::DefaultShaderId::SimpleUVWireFrame ) == 0 )
		{
			SGLOpenGLSimpleTriangle2UVLineShader::GetSourceInfo( srcShader ) ;
			pShader = new SGLOpenGLSimpleTriangle2UVLineShader( this ) ;
		}
		if ( pShader != nullptr )
		{
			if ( !MakeCustomShader( pwszID, pShader, srcShader ) )
			{
				return	pShader ;
			}
			delete	pShader ;
		}
	}
	return	nullptr ;
}

// カスタムシェーダーコンパイル
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::CompileCustomShader
		( S3DCustomShader * pShader,
			const ShaderSourceInfo& src,
			S3DCustomShader::CompileListener * pListener )
{
	if ( pListener == nullptr )
	{
		pListener = m_pShdCompileListener ;
	}
	if ( !IsOnRenderThread() )
	{
		CompileCustomShaderProcedure
				proc( this, pShader, pListener, src ) ;
		Procedure( &proc, procedureNoRender ) ;
		proc.Wait() ;
		return	proc.m_errResult ;
	}
	if ( src.typeProgram == S3DRenderDevice::programCompute )
	{
		SGLOpenGLComputeShader *
			pglShader = ESLTypeCast<SGLOpenGLComputeShader>( pShader ) ;
		if ( pglShader == nullptr )
		{
			return	sglErrFailed ;
		}
		return	pglShader->InitializeProgram( src, pListener ) ;
	}
	else
	{
		SGLOpenGLCustomShader *
			pglShader = ESLTypeCast<SGLOpenGLCustomShader>( pShader ) ;
		if ( pglShader == nullptr )
		{
			return	sglErrFailed ;
		}
		return	pglShader->InitializeProgram( src, pListener ) ;
	}
}

// カスタムシェーダー読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::LoadCustomShader
		( S3DCustomShader * pShader,
			const S3DShaderBinary& bin,
			S3DCustomShader::CompileListener * pListener )
{
	if ( pListener == nullptr )
	{
		pListener = m_pShdCompileListener ;
	}
	if ( !IsOnRenderThread() )
	{
		LoadCustomShaderProcedure
				proc( this, pShader, pListener, &bin ) ;
		Procedure( &proc, procedureSync ) ;
		return	proc.m_errResult ;
	}
	SGLOpenGLShaderProgram *
		pglShader = ESLTypeCast<SGLOpenGLShaderProgram>( pShader ) ;
	if ( pglShader == nullptr )
	{
		return	sglErrFailed ;
	}
	return	pglShader->LoadProgramBinary( bin, pListener ) ;
}

// カスタムシェーダー・バイナリの保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::SaveCustomShaderBinary
	( S3DShaderBinary& bin, S3DCustomShader * pShader )
{
	SGLOpenGLCustomShader *
		pglShader = ESLTypeCast<SGLOpenGLCustomShader>( pShader ) ;
	if ( pglShader == nullptr )
	{
		return	sglErrFailed ;
	}
	return	pglShader->SaveProgramBinary( bin ) ;
}

// OpenGL 拡張サポートテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::IsExtensionSupported( const wchar_t * pwszExtension )
{
	const size_t	count = m_extension_supported.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SString *	pstrExtName = m_extension_supported.GetAt(i) ;
		if ( (pstrExtName != nullptr)
			&& (*pstrExtName == pwszExtension) )
		{
			return	true ;
		}
	}
	return	false ;
}

// OpenGL Quad Buffer サポートテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::IsSupportedStereo( void ) const
{
#if	defined(__API_OPEN_GL_ES__)
	return	false ;
#else
	GLboolean	boolTest = false ;
	glGetBooleanv( GL_STEREO, &boolTest ) ;
	if ( !SGLOpenGLContext::VerifyError( "glGetBooleanv(GL_STEREO)" ) )
	{
		return	false ;
	}
	return	(boolTest != 0) ;
#endif
}

// OpenGL バージョンから使用可能な GLSL バージョン番号取得
//////////////////////////////////////////////////////////////////////////////
int SGLOpenGLContext::GetMaxGLSLVersion( void ) const
{
	if ( m_versionGL[0] <= 1 )
	{
		return	0 ;
	}
	else if ( m_versionGL[0] == 2 )
	{
		if ( m_versionGL[1] == 0 )
		{
			return	110 ;			// OpenGL 2.0 / GLSL 1.1
		}
		else
		{
			return	120 ;			// OpenGL 2.1 / GLSL 1.2
		}
	}
	else if ( m_versionGL[0] == 3 )
	{
		if ( m_versionGL[1] == 0 )
		{
			return	130 ;			// OpenGL 3.0 / GLSL 1.3
		}
		else if ( m_versionGL[1] == 1 )
		{
			return	140 ;			// OpenGL 3.1 / GLSL 1.4
		}
		else if ( m_versionGL[1] == 2 )
		{
			return	150 ;			// OpenGL 3.2 / GLSL 1.5
		}
	}
	int	nMinor = m_versionGL[1] ;
	if ( nMinor < 10 )
	{
		nMinor *= 10 ;
	}
	return	m_versionGL[0] * 100 + nMinor ;
}

// OpenGL 描画ステータスを初期設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::InitMaterialSetting( void )
{
	//
	// 描画方法
	//
	glEnable( GL_BLEND ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_BLEND)" ) ;
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ) ;
	SGLOpenGLContext::VerifyError( "glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA)" ) ;
	m_modeBlend = SGLOpenGLContext::blendProducted ;
	//
	// αテスト
	//
	#if	!defined(__API_OPEN_GL_ES__)
	glDisable( GL_ALPHA_TEST ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_ALPHA_TEST)" ) ;
	/*
	glAlphaFunc( GL_GEQUAL, 0.0078125 ) ;
	SGLOpenGLContext::VerifyError( "glAlphaFunc" ) ;
	glEnable( GL_ALPHA_TEST ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_ALPHA_TEST)" ) ;
	*/
	#endif
	m_alphaTestZeroClip = false ;
//	m_alphaTestZeroClip = true ;
	//
	// 表面ポリゴン
	//
	glEnable( GL_CULL_FACE ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_CULL_FACE)" ) ;
	glCullFace( GL_BACK ) ;
	SGLOpenGLContext::VerifyError( "glCullFace(GL_BACK)" ) ;
	m_faceDoubleSide = false ;
	m_faceCullBack = true ;
	//
	// ｚ比較
	//
	glDisable( GL_DEPTH_TEST ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
	glDepthMask( GL_FALSE ) ;
	SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
	m_funcDepthTest = shadingNoZBuffer ;
	//
	// テクスチャ
	//
	if ( g_supports_opengl_1_3 )
	{
		glDisable( GL_MULTISAMPLE ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_MULTISAMPLE)" ) ;
	}
	m_enabledMultiSample = false ;
	//
	if ( g_supports_opengl_1_5 )
	{
		glActiveTexture( GL_TEXTURE0 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	}
}

// SGLOpenGLView の関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AttachGLView( SGLOpenGLView * pView )
{
	m_pglView = pView ;
}

// フレームバッファの関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AttachFrameBuffer
		( SGLOpenGLFrameBuffer * pglFrameBuffer )
{
	if ( OpenGLExtension::g_supports_framebuffer_object )
	{
		if ( pglFrameBuffer != nullptr )
		{
			glBindFramebuffer
				( pglFrameBuffer->m_glBufTarget,
					pglFrameBuffer->m_glFrameBuffer ) ;
			SGLOpenGLContext::VerifyError( "glBindFramebuffer()" ) ;
		}
		else
		{
			glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
			SGLOpenGLContext::VerifyError( "glBindFramebuffer(GL_FRAMEBUFFER,0)" ) ;
		}
	}
	m_pglFrameBuffer = pglFrameBuffer ;
}

// 現在のシェーダーを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AttachShaderProgram( SGLOpenGLShaderProgram * pglShader )
{
	if ( OpenGLExtension::g_supports_opengl_2_0 )
	{
		if ( (m_pglCurShader != nullptr) && (m_pglCurShader != pglShader) )
		{
			m_pglCurShader->OnChangingProgram() ;
		}
		if ( pglShader != nullptr )
		{
			glUseProgram( pglShader->m_glProgram ) ;
			SGLOpenGLContext::VerifyError( "glUseProgram" ) ;
			//
			if ( m_pglCurShader != pglShader )
			{
				pglShader->OnChangedProgram() ;
			}
		}
		else
		{
			glUseProgram( 0 ) ;
			SGLOpenGLContext::VerifyError( "glUseProgram(0)" ) ;
		}
	}
	m_pglCurShader = pglShader ;
}

// デフォルトシェーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLShaderProgram *
		SGLOpenGLContext::GetDefaultShaderProgram( int nShaderType ) const
{
	if ( nShaderType & (shadingMethodPhong | shadingMethodToon) )
	{
		if ( m_pDefShader[indexPhongShading] != nullptr )
		{
			return	m_pDefShader[indexPhongShading] ;
		}
		else if ( m_pDefShader[indexGouraudShading] != nullptr )
		{
			return	m_pDefShader[indexGouraudShading] ;
		}
	}
	else if ( nShaderType & shadingMethodGouraud )
	{
		if ( m_pDefShader[indexGouraudShading] != nullptr )
		{
			return	m_pDefShader[indexGouraudShading] ;
		}
		else if ( m_pDefShader[indexPhongShading] != nullptr )
		{
			return	m_pDefShader[indexPhongShading] ;
		}
	}
	return	m_pDefShader[indexNonShading] ;
}

SGLOpenGLShaderProgram *
		SGLOpenGLContext::GetStandardShaderProgram( int nShaderType ) const
{
	if ( nShaderType & (shadingMethodPhong | shadingMethodToon) )
	{
		return	m_pDefShader[indexPhongShading] ;
	}
	else if ( nShaderType & shadingMethodGouraud )
	{
		return	m_pDefShader[indexGouraudShading] ;
	}
	return	m_pDefShader[indexNonShading] ;
}

// 現在のレンダラを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AttachRenderContext( S3DRenderContextInterface * pRender )
{
	m_pCurRenderer = pRender ;
}

// バインドテクスチャ情報を設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::SetBindTextureInfo
	( int iTexture,
		SGLOpenGLTextureBuffer::GLResource * pglTexture, GLenum glTarget )
{
	if ( (iTexture >= 0) && (iTexture < countBindTxtureNum) )
	{
		m_glBinTexture[iTexture].pglBind = pglTexture ;
		m_glBinTexture[iTexture].glTarget = glTarget ;
	}
}

// バインドテクスチャ情報を取得する
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::IsBindingTexture( int iTexture, GLenum& glTarget ) const
{
	if ( (iTexture >= 0) && (iTexture < countBindTxtureNum) )
	{
		if ( m_glBinTexture[iTexture].pglBind != nullptr )
		{
			glTarget = m_glBinTexture[iTexture].glTarget ;
			return	true ;
		}
	}
	return	false ;
}

// ブレンドモード設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::SetBlendMode( SGLOpenGLContext::BlendMode modeBlend )
{
	if ( m_modeBlend != modeBlend )
	{
		glBlendFunc( s_blendModeParam[modeBlend][0],
						s_blendModeParam[modeBlend][1] ) ;
		SGLOpenGLContext::VerifyError( "glBlendFunc" ) ;
		m_modeBlend = modeBlend ;
	}
}

// ブレンドモード変換
SGLOpenGLContext::BlendMode
	SGLOpenGLContext::BlendModeFromBlendOp
		( S3DRenderBufferInterface::BlendOperation blendOp,
			SGLOpenGLContext::BlendMode modeDefault )
{
	switch ( blendOp )
	{
	case	S3DRenderBufferInterface::blendDefault:
	default:
		break ;

	case	S3DRenderBufferInterface::blendProductedSrc:
		return	blendProducted ;

	case	S3DRenderBufferInterface::blendAdd:
		return	blendAdd ;

	case	S3DRenderBufferInterface::blendUnproductedSrc:
		return	blendUnproducted ;

	case	S3DRenderBufferInterface::blendCopy:
		return	blendCopy ;

	case	S3DRenderBufferInterface::blendDstMasked:
		return	blendDstMasked ;

	case	S3DRenderBufferInterface::blendMulColor:
		return	blendMulColor ;
	}
	return	modeDefault ;
}

// OpenGL スレッドか判定
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::IsOnRenderThread( void )
{
	return	false ;
}

// OpenGL スレッドで実行する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::Procedure
	( SSystem::SProcedure* pProc, ProcedurePriority priority )
{
	return	sglErrFailed ;
}

// レンダラ生成
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLOpenGLContext::NewRenderer( void ) const
{
	return	new S3DOpenGLBufferedRenderer( (SGLOpenGLContext*) this ) ;
}

// レンダリングデバイス用の画像インスタンスを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::CommitDeviceImage
	( SGLImageObject * pImage, int64_t msecTimeout )
{
	ESLAssert( pImage != nullptr ) ;
	if ( IsOnRenderThread() )
	{
		SGLImageRect	rectRefTexture ;
		SGLOpenGLTextureBuffer::CommitGLTexture
						( this, pImage, rectRefTexture ) ;
	}
	else
	{
		if ( msecTimeout == 0 )
		{
			SSyncProcedure *	pSyncProc =
				new SSyncProcedure
					( new CommitImageProcedure( this, pImage ), true ) ;
			pSyncProc->SetAutoDelete( true ) ;
			Procedure( pSyncProc, procedureNoRender ) ;
		}
		else
		{
			SSyncProcedure *	pSyncProc =
				new SSyncProcedure
					( new CommitImageProcedure
						( this, pImage, 1024 * 1024 ), true ) ;
			Procedure( pSyncProc, procedureNoRender ) ;
			if ( pSyncProc->WaitDone( msecTimeout ) == errSuccess )
			{
				delete	pSyncProc ;
			}
			else
			{
				pSyncProc->SetAutoDelete( true ) ;
				return	sglErrTimeout ;
			}
		}
	}
	return	sglErrSuccess ;
}

// レンダリングデバイス用の VBO インスタンスを生成／更新
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::CommitDeviceVertexBuffer
	( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout )
{
	ESLAssert( pVertexBuf != nullptr ) ;
	SGLOpenGLVertexBuffer *	pVBO = SGLOpenGLVertexBuffer::Commit( pVertexBuf ) ;
	if ( pVBO == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( IsOnRenderThread() )
	{
		pVBO->CommitResourceAs( this ) ;
	}
	else
	{
		SSyncProcedure *	pSyncProc =
			new SSyncProcedure
				( new CommitVertexBufferProcedure( this, pVBO ), true ) ;
		if ( msecTimeout == 0 )
		{
			pSyncProc->SetAutoDelete( true ) ;
			Procedure( pSyncProc, procedureNoRender ) ;
		}
		else
		{
			Procedure( pSyncProc, procedureNoRender ) ;
			if ( pSyncProc->WaitDone( msecTimeout ) == errSuccess )
			{
				delete	pSyncProc ;
			}
			else
			{
				pSyncProc->SetAutoDelete( true ) ;
				return	sglErrTimeout ;
			}
		}
	}
	return	sglErrSuccess ;
}

// レンダリングデバイス用の画像インスタンスを解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::ReleaseDeviceImage
	( SGLImageObject * pImage, int64_t msecTimeout )
{
	ESLAssert( pImage != nullptr ) ;
	if ( pImage != nullptr )
	{
		pImage->DeleteImageObjectTypeOf( imageObjectGLTexture ) ;
	}
	return	sglErrSuccess ;
}

// レンダリングデバイス用の VBO インスタンスを解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::ReleaseDeviceVertexBuffer
	( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout )
{
	ESLAssert( pVertexBuf != nullptr ) ;
	pVertexBuf->ReleaseAllDeviceResources() ;
	return	sglErrSuccess ;
}

// デバイス機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::GetDeviceFeatures( S3DRenderDevice::Features& features )
{
	eslFillMemory( &features, 0, sizeof(S3DRenderDevice::Features) ) ;
	//
	features.flagsFeatures[0] |=
		m_flagTextureNonPowerOf2 ? feature0_TextureNonPowerOf2 : 0 ;
	features.flagsFeatures[0] |=
		m_flagDepthTexture ? feature0_DepthTexture : 0 ;
	features.flagsFeatures[0] |=
		m_flagCubemapTexture ? feature0_CubemapTexture : 0 ;
	features.flagsFeatures[0] |=
		m_flagMultisampling ? feature0_MultisampleTexture : 0 ;
	features.flagsFeatures[0] |=
		m_flagCompressionS3TC ? feature0_CompressionS3TC : 0 ;
	features.flagsFeatures[0] |=
		(m_flagProgramBinary || m_flagProgramBinaryOES)
							? feature0_ProgramBinary : 0 ;
	features.flagsFeatures[0] |=
		m_flagSupportedMRT ? feature0_MultiRenderTarget : 0 ;
	features.flagsFeatures[0] |=
		m_flagSupportedGeometry ? feature0_GeometryShader : 0 ;
	features.flagsFeatures[0] |=
		m_flagTextureFloat ? feature0_TextureFloat : 0 ;
	features.flagsFeatures[0] |=
		m_flagColorBufferFloat ? feature0_ColorBufferFloat : 0 ;
	features.flagsFeatures[0] |=
		OpenGLExtension::g_supports_instanced_draw ? feature0_InstancedDraw : 0 ;
	features.flagsFeatures[0] |=
		m_flagAvailableMultiSVB ? feature0_MultiShapeDraw : 0 ;
	features.flagsFeatures[0] |=
		OpenGLExtension::g_supports_opengl_2_0 ? feature0_CustomShader : 0 ;
	features.flagsFeatures[0] |=
		m_flagSupportedComputeShader ? feature0_ComputeShader : 0 ;
	//
	features.maxTextureSize = (uint32_t) m_maxTextureSize ;
	features.max3DTextureSize = (uint32_t) m_max3DTextureSize ;
	features.maxCubeTextureSize = (uint32_t) m_maxCubemapTextureSize ;
	features.maxMultiTextureUnits = (uint32_t) m_maxTextureImages ;
	features.maxVSTexturesUnits = (uint32_t) m_maxVSTextureImages ;
	features.maxCombinedTextureUnits = (uint32_t) m_maxCombinedTextureImages ;
	features.maxVertexAttributes = (uint32_t) m_maxVertexAttributes ;
	features.maxVertexVaryings = (uint32_t) m_maxVertexVaryings ;
	features.maxVertexUniforms = (uint32_t) m_maxVertexUniforms ;
	features.maxFragmentUniforms = (uint32_t) m_maxFragmentUniforms ;
	features.maxMutiRenderTarget = (uint32_t) m_maxDrawBuffers ;
	//
	return	sglErrSuccess ;
}

// SGLOpenGLContext チェーン
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AddToChain( void )
{
	ESLAssert( m_pChainNext == nullptr ) ;
	QuickLock() ;
	SGLOpenGLContext *	pLast = nullptr ;
	SGLOpenGLContext *	pNext = m_pChainFirst ;
	while ( pNext != nullptr )
	{
		if ( pNext == this )
		{
			QuickUnlock() ;
			return ;
		}
		pLast = pNext ;
		pNext = pNext->m_pChainNext ;
	}
	if ( pLast != nullptr )
	{
		pLast->m_pChainNext = this ;
	}
	else
	{
		m_pChainFirst = this ;
	}
	QuickUnlock() ;
}

void SGLOpenGLContext::DetachFromChain( void )
{
	QuickLock() ;
	SGLOpenGLContext *	pLast = nullptr ;
	SGLOpenGLContext *	pNext = m_pChainFirst ;
	while ( pNext != nullptr )
	{
		if ( pNext == this )
		{
			pNext = m_pChainNext ;
			m_pChainNext = nullptr ;
			//
			if ( pLast != nullptr )
			{
				pLast->m_pChainNext = pNext ;
			}
			else
			{
				m_pChainFirst = pNext ;
			}
		}
		else
		{
			pLast = pNext ;
			pNext = pNext->m_pChainNext ;
		}
	}
	QuickUnlock() ;
	ESLAssert( m_pChainNext == nullptr ) ;
}

// デフォルトの SGLOpenGLContext を取得する
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext * SGLOpenGLContext::GetDefault( void )
{
	return	m_pChainFirst ;
}

// デフォルトの SGLOpenGLContext を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::SwitchDefault( SGLOpenGLContext * pOpenGL )
{
	if ( pOpenGL == nullptr )
	{
		return ;
	}
	QuickLock() ;
	SGLOpenGLContext *	pLast = nullptr ;
	SGLOpenGLContext *	pNext = m_pChainFirst ;
	while ( pNext != nullptr )
	{
		if ( pNext == pOpenGL )
		{
			if ( pLast != nullptr )
			{
				pLast->m_pChainNext = pNext->m_pChainNext ;
			}
			else
			{
				m_pChainFirst = pNext->m_pChainNext ;
			}
			pNext = pNext->m_pChainNext ;
			continue ;
		}
		pLast = pNext ;
		pNext = pNext->m_pChainNext ;
	}
	pOpenGL->m_pChainNext = m_pChainFirst ;
	m_pChainFirst = pOpenGL ;
	QuickUnlock() ;
}

// 現在のスレッドの SGLOpenGLContext を取得する
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLContext * SGLOpenGLContext::GetCurrentGLContext( void )
{
	QuickLock() ;
	SGLOpenGLContext *	pNext = m_pChainFirst ;
	while ( pNext != nullptr )
	{
		if ( pNext->IsOnRenderThread() )
		{
			QuickUnlock() ;
			return	pNext ;
		}
		pNext = pNext->m_pChainNext ;
	}
	QuickUnlock() ;
	return	nullptr ;
}

// 現在のスレッドの SGLOpenGLContext に関連付けられている SGLOpenGLView を取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLView * SGLOpenGLContext::GetCurrentGLView( void )
{
	SGLOpenGLContext *	pOpenGL = GetCurrentGLContext() ;
	if ( pOpenGL != nullptr )
	{
		return	pOpenGL->GetGLView() ;
	}
	return	nullptr ;
}

// OpenGL スレッドで実行する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::ProcedureOnGLThread
	( SSystem::SProcedure* pProc, ProcedurePriority priority )
{
	QuickLock() ;
	SGLOpenGLContext *	pNext = m_pChainFirst ;
	while ( pNext != nullptr )
	{
		if ( pNext->IsOnRenderThread() )
		{
			QuickUnlock() ;
			//
			pProc->Prepare() ;
			pProc->Run() ;
			pProc->Finalize() ;
			return	sglErrSuccess ;
		}
		pNext = pNext->m_pChainNext ;
	}
	pNext = m_pChainFirst ;
	QuickUnlock() ;
	//
	if ( pNext == nullptr )
	{
		return	sglErrFailed ;
	}
	return	pNext->Procedure( pProc, priority ) ;
}

// OpenGL のエラーをチェックし、エラーの場合はログに出力する
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLContext::VerifyError( const char * pszFunction )
{
	static const char *	pszLastSucceeded = nullptr ;
	GLenum	err ;
	err = glGetError() ;
	if ( err )
	{
		if ( pszFunction != nullptr )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			const char *	pszErr = nullptr ;
			if ( sizeof(char) == sizeof(GLubyte) )
			{
				pszErr = (const char*) gluErrorString( err ) ;
				Trace( "failed to %s (%08X) \'%s\'\n", pszFunction, err, pszErr ) ;
			}
			else
			#endif
			{
				Trace( "failed to %s (%08X)\n", pszFunction, err ) ;
			}
			if ( pszLastSucceeded != nullptr )
			{
				ESLTrace( "last succeeded: %s\n", pszLastSucceeded ) ;
				pszLastSucceeded = nullptr ;
			}
		}
		return	false ;
	}
	else
	{
		pszLastSucceeded = pszFunction ;
		return	true ;
	}
}

// SuitableRender 実行
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::SuitableProcedure
	( S3DRenderContextInterface::PROCEDURE_RENDERING pfnRendering, void * pInstance )
{
	if ( IsOnRenderThread() )
	{
		if ( !BeginSuitableThread() )
		{
			m_csSuitableRendering.Lock() ;
			if ( !m_flagInSuitableProcedure )
			{
				//
				// 別スレッドで指定関数を実行
				//
				m_queRenderProc.Reset() ;
				m_pfnSuitableRendering = pfnRendering ;
				m_pSuitableRenderInstance = pInstance ;
				m_flagDoneSuitableProc = false ;
				m_csSuitableRendering.Unlock() ;
				m_signalSuitableProc.SetSignal() ;
				//
				// 実行完了までレンダラを割り当てながら待機
				//
				m_queRenderProc.RunAll() ;

				return ;
			}
			else
			{
				m_csSuitableRendering.Unlock() ;
			}
		}
	}
	pfnRendering( pInstance ) ;
}

// SuitableRender 実行中にOpenGL スレッドで実行する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::ProcedureInSuitable( SSystem::SProcedure* pProc, bool fSync )
{
	if ( pProc == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( IsOnRenderThread() )
	{
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
		return	sglErrSuccess ;
	}
	SSyncProcedure *	pSyncProc = nullptr ;
	if ( fSync )
	{
		pSyncProc = new SSyncProcedure( pProc ) ;
		pProc = pSyncProc ;
	}
	m_csSuitableRendering.Lock() ;
	if ( !m_flagInSuitableProcedure )
	{
		m_csSuitableRendering.Unlock() ;
		return	sglErrFailed ;
	}
	m_queRenderProc.AddProcedure( pProc ) ;
	m_csSuitableRendering.Unlock() ;
	//
	if ( fSync )
	{
		pSyncProc->WaitDone() ;
		pSyncProc->SetAutoDelete() ;
	}
	return	sglErrSuccess ;
}

// SuitableRender 用スレッド開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::BeginSuitableThread( void )
{
	SGLError	err = sglErrSuccess ;
	m_csSuitableRendering.Lock() ;
	if ( !m_flagReadySuitableThread )
	{
		m_signalSuitableProc.Initialize( false ) ;
		m_flagQuitSuitableThread = false ;
		m_flagInSuitableProcedure = false ;
		//
		if ( m_threadSuitable.BeginThread( this ) == errSuccess )
		{
			m_flagReadySuitableThread = true ;
		}
		else
		{
			err = sglErrFailed ;
		}
	}
	m_csSuitableRendering.Unlock() ;
	return	err ;
}

// SuitableRender 用スレッド終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLContext::EndSuitableThread( void )
{
	m_csSuitableRendering.Lock() ;
	if ( m_flagReadySuitableThread )
	{
		m_flagQuitSuitableThread = true ;
		m_signalSuitableProc.SetSignal() ;
		m_csSuitableRendering.Unlock() ;
		//
		m_threadSuitable.Wait() ;
		//
		m_csSuitableRendering.Lock() ;
		m_signalSuitableProc.Delete() ;
		m_flagReadySuitableThread = false ;
	}
	m_csSuitableRendering.Unlock() ;
	return	sglErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::Run( void )
{
	for ( ; ; )
	{
		if ( m_signalSuitableProc.Wait() == errSuccess )
		{
			m_csSuitableRendering.Lock() ;
			m_signalSuitableProc.ResetSignal() ;
			if ( m_pfnSuitableRendering != nullptr )
			{
				S3DRenderContextInterface::PROCEDURE_RENDERING
						pfnRendering = m_pfnSuitableRendering ;
				void *	ptrInstance = m_pSuitableRenderInstance ;
				//
				m_pfnSuitableRendering = nullptr ;
				m_pSuitableRenderInstance = nullptr ;
				m_flagInSuitableProcedure = true ;
				m_csSuitableRendering.Unlock() ;
				//
				pfnRendering( ptrInstance ) ;
				//
				m_csSuitableRendering.Lock() ;
				m_queRenderProc.RequestQuit( SProcedure::quitNormally ) ;
				m_flagInSuitableProcedure = false ;
				m_flagDoneSuitableProc = true ;
			}
			if ( m_flagQuitSuitableThread )
			{
				m_csSuitableRendering.Unlock() ;
				break ;
			}
			m_csSuitableRendering.Unlock() ;
		}
	}
}

// テクスチャ転送用一時バッファを取得する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::GetTemporaryImageBuffer
	( SGLImageBuffer& imgTemp,
		uint32_t format, uint32_t depth, uint32_t width, uint32_t height )
{
	imgTemp.format = format ;
	imgTemp.depth = depth ;
	imgTemp.width = width ;
	imgTemp.height = height ;
	imgTemp.pitchPixel = (depth >> 3) ;
	imgTemp.pitchLine = width * imgTemp.pitchPixel ;
	//
	size_t	bytesTemp = imgTemp.pitchLine * height ;
	if ( bytesTemp > m_bufTempTexture.GetLength() )
	{
		size_t	bytesBuf = 0x10000 ;
		while ( bytesBuf < bytesTemp )
		{
			bytesBuf <<= 1 ;
		}
		m_bufTempTexture.FreeArray() ;
		m_bufTempTexture.SetLength( bytesBuf ) ;
	}
	imgTemp.ptrBuffer = m_bufTempTexture.GetArrayPtr() ;
}

// テクスチャ使用容量を加算する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::AddUsedTextureBytes( size_t bytesTeture )
{
	m_bytesUsedTexture += bytesTeture ;
	if ( m_bytesUsedTexture > m_bytesMaxUsedTexture )
	{
		if ( (m_bytesUsedTexture >> 20) > (m_bytesMaxUsedTexture >> 20) )
		{
			ESLTrace( "max used OpenGL texture: %d [MB]\n",
								(int) (m_bytesUsedTexture >> 20) ) ;
		}
		m_bytesMaxUsedTexture = m_bytesUsedTexture ;
	}
}

// パフォーマンスログ・フレーム開始
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::BeginFramePerformanceLog( void )
{
	m_pflog.Reset() ;
	m_timerFrame.Reset() ;
}

// パフォーマンスログ・フレーム終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::EndFramePerformanceLog( void )
{
	m_pflog.msecRenderingFrame = m_timerFrame.GetRealTime() ;
	m_pflogMax.Max( m_pflog ) ;
	m_pflogTotal.Add( m_pflog ) ;
	m_nTotalLogFrameCount ++ ;
}

// パフォーマンスログ・デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLContext::DebugTracePerformanceLog( PerformanceLogInfo * pli )
{
	if ( pli != nullptr )
	{
		pli->nFrameCount = m_nTotalLogFrameCount ;
		pli->secInterval = m_timerLog.GetRealTime() / 1000.0 ;
		//
		m_pflogTotal.ToFrameLog( pli->pflogSum ) ;
		m_pflogMax.ToFrameLog( pli->pflogMax ) ;
		//
		pli->bytesUsedTexture = m_bytesUsedTexture ;
		pli->bytesMaxUsedTexture = m_bytesMaxUsedTexture ;
		pli->bytesUsedVBO = m_bytesUsedVBO ;
		pli->bytesMaxUsedVBO = m_bytesMaxUsedVBO ;
	}
	else
	{
		double	fpDivFrame = 1.0 ;
		if ( m_nTotalLogFrameCount > 0 )
		{
			fpDivFrame /= m_nTotalLogFrameCount ;
		}
		Trace( "OpenGL log during %d frames;\n"
				"  draw call:              %.1f (max %d) [count/frame]\n"
				"  instance count:         %.1f (max %d) [count/frame]\n"
				"  transmit vertex:        %.1f (max %d) [count/frame]\n"
				"  switch render context:  %.1f (max %d) [count/frame]\n"
				"  switch shader:          %.1f (max %d) [count/frame]\n"
				"  switch material:        %.1f (max %d) [count/frame]\n"
				"  shape processed by CPU: %.1f (max %d) [count/frame]\n"
				"  shape processed time:   %.1f (max %f) [ms]\n",
				m_nTotalLogFrameCount,
				m_pflogTotal.countDrawCall * fpDivFrame,
				m_pflogMax.countDrawCall,
				m_pflogTotal.countDrawInstance * fpDivFrame,
				m_pflogMax.countDrawInstance,
				m_pflogTotal.countTransmitVertex * fpDivFrame,
				m_pflogMax.countTransmitVertex,
				m_pflogTotal.countSwitchRenderer * fpDivFrame,
				m_pflogMax.countSwitchRenderer,
				m_pflogTotal.countSwitchShader * fpDivFrame,
				m_pflogMax.countSwitchShader,
				m_pflogTotal.countSwitchMaterial * fpDivFrame,
				m_pflogMax.countSwitchMaterial,
				m_pflogTotal.countComputeShapeByCPU * fpDivFrame,
				m_pflogMax.countComputeShapeByCPU,
				m_pflogTotal.msecShapeByCPU * fpDivFrame,
				m_pflogMax.msecShapeByCPU ) ;
	}
	m_pflogMax.Reset() ;
	m_pflogTotal.Reset() ;
	m_nTotalLogFrameCount = 0 ;
	m_timerLog.Reset() ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL ビュー・インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLView::SGLOpenGLView( void )
	: m_sizePhysical( 800, 480 ), m_sizeVirtual( 640, 480 ),
		m_rectViewPort( 0, 0, 640, 480 ), m_vViewOffset( 0.0f, 0.0f )
{
	m_xViewLeft = 0.0f ;
	m_xViewRight = 640.0f ;
	m_yViewTop = 480.0f ;
	m_yViewBottom = 0.0f ;
	m_flagRotatable = false ;
	m_flagRotation = false ;
	m_flagViewport = false ;
	m_flagZBounds = false ;
	m_flagOrthogonal = false ;
	m_zNear = 10.0f ;
	m_zFar = 1.0e+5 ;
	m_flagMatrixPers = false ;
}

// 物理ビューサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::SetPhysicalViewSize( const SGLSize& size )
{
	m_sizePhysical = size ;
	UpdateViewPort() ;
}

// 論理ビューサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::SetVirtualViewSize( const SGLSize& size )
{
	m_sizeVirtual = size ;
	UpdateViewPort() ;
}

// 論理ビューの回転（縦横比の自動的な最適化）を有効／無効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::EnableViewRotation( bool fRotatable )
{
	m_flagRotatable = fRotatable ;
	UpdateViewPort() ;
}

// ｚ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::SetZBounds
	( float32_t zNear, float32_t zFar, bool fZBounds )
{
	m_flagZBounds = fZBounds ;
	m_zNear = zNear ;
	m_zFar = zFar ;
}

// ビューポート更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::UpdateViewPort( void )
{
	bool	fRotate = false ;
	SGLSize	sizePhysical = m_sizePhysical ;
	if ( m_sizeVirtual.w <= 0 )
	{
		m_sizeVirtual.w = 1 ;
	}
	if ( m_sizeVirtual.h <= 0 )
	{
		m_sizeVirtual.h = 1 ;
	}
	if ( m_flagRotatable )
	{
		// 縦横比で回転判定
		if ( ((m_sizeVirtual.w > m_sizeVirtual.h)
					&& (m_sizePhysical.w < m_sizePhysical.h))
			|| ((m_sizeVirtual.w < m_sizeVirtual.h)
					&& (m_sizePhysical.w > m_sizePhysical.h)) )
		{
			fRotate = true ;
			sizePhysical.w = m_sizePhysical.h ;
			sizePhysical.h = m_sizePhysical.w ;
		}
	}
	int	xViewPort = 0, yViewPort = 0 ;
	int	widthViewPort = sizePhysical.w ;
	int	heightViewPort = sizePhysical.h ;
	if ( sizePhysical.w * m_sizeVirtual.h <= sizePhysical.h * m_sizeVirtual.w )
	{
		// 横の比率に合わせる
		heightViewPort = m_sizeVirtual.h * sizePhysical.w / m_sizeVirtual.w ;
		yViewPort = (sizePhysical.h - heightViewPort) / 2 ;
	}
	else
	{
		// 縦の比率に合わせる
		widthViewPort = m_sizeVirtual.w * sizePhysical.h / m_sizeVirtual.h ;
		xViewPort = (sizePhysical.w - widthViewPort) / 2 ;
	}
	if ( fRotate )
	{
		m_rectViewPort.x = yViewPort ;
		m_rectViewPort.y = xViewPort ;
		m_rectViewPort.w = heightViewPort ;
		m_rectViewPort.h = widthViewPort ;
		//
		float32_t	xScale =
			(float32_t) widthViewPort / (float32_t) m_sizeVirtual.w ;
		float32_t	yScale =
			(float32_t) heightViewPort / (float32_t) m_sizeVirtual.h ;
		//
		m_affineView.a11 = 0.0f ;
		m_affineView.a12 = - yScale ;
		m_affineView.a13 = (float32_t) m_rectViewPort.w ;
		m_affineView.a21 = xScale ;
		m_affineView.a22 = 0.0f ;
		m_affineView.a23 = 0.0f ;
		//
		m_affineRotation.a11 = 0.0f ;
		m_affineRotation.a12 = -1.0f ;
		m_affineRotation.a13 = (float32_t) m_sizeVirtual.h ;
		m_affineRotation.a21 = 1.0f ;
		m_affineRotation.a22 = 0.0f ;
		m_affineRotation.a23 = 0.0f ;
		m_flagRotation = true ;
		//
		m_xViewLeft = 0.0f ;
		m_xViewRight = (float32_t) m_sizeVirtual.h ;
		m_yViewBottom = 0.0f ;
		m_yViewTop = (float32_t) m_sizeVirtual.w ;
	}
	else
	{
		m_rectViewPort.x = xViewPort ;
		m_rectViewPort.y = yViewPort ;
		m_rectViewPort.w = widthViewPort ;
		m_rectViewPort.h = heightViewPort ;
		if ( widthViewPort + 1 == sizePhysical.w )
		{
			m_rectViewPort.x = 0 ;
			m_rectViewPort.w = sizePhysical.w ;
		}
		if ( heightViewPort + 1 == sizePhysical.h )
		{
			m_rectViewPort.y = 0 ;
			m_rectViewPort.h = sizePhysical.h ;
		}
		//
		float32_t	xScale =
			(float32_t) m_rectViewPort.w / (float32_t) m_sizeVirtual.w ;
		float32_t	yScale =
			(float32_t) m_rectViewPort.h / (float32_t) m_sizeVirtual.h ;
		//
		m_affineView.a11 = xScale ;
		m_affineView.a12 = 0.0f ;
		m_affineView.a13 = 0.0f ;
		m_affineView.a21 = 0.0f ;
		m_affineView.a22 = yScale ;
		m_affineView.a23 = 0.0f ;
		//
		m_affineRotation.a11 = 1.0f ;
		m_affineRotation.a12 = 0.0f ;
		m_affineRotation.a13 = 0.0f ;
		m_affineRotation.a21 = 0.0f ;
		m_affineRotation.a22 = 1.0f ;
		m_affineRotation.a23 = 0.0f ;
		m_flagRotation = false ;
		//
		m_xViewLeft = 0.0f ;
		m_xViewRight = (float32_t) m_sizeVirtual.w ;
		m_yViewBottom = 0.0f ;
		m_yViewTop = (float32_t) m_sizeVirtual.h ;
	}
	m_affineIView.InverseOf( m_affineView ) ;
}

// OpenGL ビューポート設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::SetOpenGLViewPort
	( const SGLImageRect * pView, const S2DVector * pvOffset )
{
	if ( (pView == nullptr)
		|| ((pView->x == 0) && (pView->y == 0)
				&& (pView->w == m_sizeVirtual.w)
				&& (pView->h == m_sizeVirtual.h)) )
	{
		glViewport
			( m_rectViewPort.x, m_rectViewPort.y,
					m_rectViewPort.w, m_rectViewPort.h ) ;
		SGLOpenGLContext::VerifyError( "glViewport" ) ;
		//
		m_rectViewClip.x = 0 ;
		m_rectViewClip.y = 0 ;
		m_rectViewClip.w = m_sizeVirtual.w ;
		m_rectViewClip.h = m_sizeVirtual.h ;
		//
		m_xViewLeft = 0.0f ;
		m_xViewRight = (float32_t) m_sizeVirtual.w ;
		m_yViewBottom = 0.0f ;
		m_yViewTop = (float32_t) m_sizeVirtual.h ;
		m_flagViewport = false ;
	}
	else
	{
		/*
		S2DVector	v0 = m_affineView * S2DVector(pView->x, pView->y) ;
		S2DVector	v1 = m_affineView
							* S2DVector(pView->x + pView->w,
										pView->y + pView->h) ;
		if ( v0.x > v1.x )
		{
			float32_t	x = v0.x ;
			v0.x = v1.x ;
			v1.x = x ;
		}
		if ( v0.y > v1.y )
		{
			float32_t	y = v0.y ;
			v0.y = v1.y ;
			v1.y = y ;
		}
		glViewport
			( (GLint) v0.x, (GLint) v0.y,
				(GLsizei) (v1.x - v0.x), (GLsizei) (v1.y - v0.y) ) ;
		*/
		glViewport( pView->x, pView->y, pView->w, pView->h ) ;
		SGLOpenGLContext::VerifyError( "glViewport" ) ;
		//
		m_rectViewClip = *pView ;
		//
		m_xViewLeft = (float32_t) m_rectViewClip.x ;
		m_xViewRight = (float32_t) (m_rectViewClip.x + m_rectViewClip.w) ;
		m_yViewBottom = (float32_t) m_rectViewClip.y ;
		m_yViewTop = (float32_t) (m_rectViewClip.y + m_rectViewClip.h) ;
		//
		m_flagViewport = true ;
		m_affineViewport.a11 = (float32_t) m_sizeVirtual.w
									/ (float32_t) m_rectViewClip.w ;
		m_affineViewport.a12 = 0.0f ;
		m_affineViewport.a21 = 0.0f ;
		m_affineViewport.a22 = (float32_t) m_sizeVirtual.h
									/ (float32_t) m_rectViewClip.h ;
		m_affineViewport.a13 =
				- (float32_t) m_rectViewClip.x * m_affineViewport.a11 ;
		m_affineViewport.a23 =
				- (float32_t) (m_sizeVirtual.h - m_yViewTop) * m_affineViewport.a22 ;
	}
	if ( pvOffset != nullptr )
	{
		m_vViewOffset = *pvOffset ;
	}
	else
	{
		m_vViewOffset.x = 0.0f ;
		m_vViewOffset.y = 0.0f ;
	}
}

void SGLOpenGLView::RestoreOpenGLViewPort( void )
{
	SGLImageRect	rectView = m_rectViewClip ;
	S2DVector		vOffset = m_vViewOffset ;
	SetOpenGLViewPort( &rectView, &vOffset ) ;
}

// 2D 描画用座標系設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::PutOpenGLOrthogonalProjection
	( SGLOpenGLShaderProgram * pShader, bool yReverse )
{
	double	zNear = m_zNear ;
	double	zFar = m_zFar ;
	if ( !m_flagZBounds )
	{
		zNear = -1.0e+5f ;
		zFar = 1.0e+5f ;
	}
	S4DMatrix			matProjection ;
	const SGLAffine *	pAffineRotation = nullptr ;
	SGLAffine			afView ;
	if ( m_flagRotation )
	{
		pAffineRotation = &m_affineRotation ;
	}
	matProjection.OrthogonalProjection
		( m_xViewLeft /*- 0.49f*/, m_xViewRight /*- 0.49f*/,
			m_yViewTop /*- 0.49f*/, m_yViewBottom /*- 0.49f*/,
			zNear, zFar, yReverse,
			-m_vViewOffset.x, -m_vViewOffset.y, pAffineRotation ) ;

	PutOpenGLOrthogonalProjection( pShader, matProjection ) ;
}

void SGLOpenGLView::PutOpenGLOrthogonalProjection
	( SGLOpenGLShaderProgram * pShader,
		double xScreen, double yScreen, double zScale, bool yReverse )
{
	double	zNear = m_zNear ;
	double	zFar = m_zFar ;
	if ( !m_flagZBounds )
	{
		zNear = -1.0e+3f ;
		zFar = 1.0e+5f ;
	}
	S4DMatrix			matProjection ;
	const SGLAffine *	pAffineRotation = nullptr ;
	SGLAffine			afView ;
	if ( m_flagRotation )
	{
		pAffineRotation = &m_affineRotation ;
	}
	matProjection.OrthogonalProjection
		( (m_xViewLeft - xScreen) * zScale,
			(m_xViewRight - xScreen) * zScale,
			(m_yViewTop - yScreen) * zScale,
			(m_yViewBottom - yScreen) * zScale,
			zNear, zFar, yReverse,
			-m_vViewOffset.x, -m_vViewOffset.y, pAffineRotation ) ;

	PutOpenGLOrthogonalProjection( pShader, matProjection ) ;
}

void SGLOpenGLView::PutOpenGLOrthogonalProjection
	( SGLOpenGLShaderProgram * pShader, const S4DMatrix& matProjection )
{
	if ( pShader != nullptr )
	{
		pShader->SetPerspectiveMatrix( matProjection ) ;
	}
	else
	{
		glMatrixMode( GL_PROJECTION ) ;
		SGLOpenGLContext::VerifyError( "glMatrixMode(GL_PROJECTION)" ) ;

		glLoadIdentity() ;
		SGLOpenGLContext::VerifyError( "glLoadIdentity" ) ;

		GLfloat	mat[4][4] ;
		for ( int i = 0; i < 4; i ++ )
		{
			mat[i][0] = matProjection.m[0][i] ;
			mat[i][1] = matProjection.m[1][i] ;
			mat[i][2] = matProjection.m[2][i] ;
			mat[i][3] = matProjection.m[3][i] ;
		}
		glMultMatrixf( &mat[0][0] ) ;
		SGLOpenGLContext::VerifyError( "glMultMatrixf" ) ;
	}
	m_flagOrthogonal = true ;
	m_flagMatrixPers = true ;
	m_matPerspective = matProjection ;
	m_matPerspectM22 = matProjection.m[2][2] ;
	m_matPerspectM23 = matProjection.m[2][3] ;

	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->AttachGLView( this ) ;
	}
}

// 3D 描画用座標系設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::PutOpenGLPerspectiveProjection
	( SGLOpenGLShaderProgram * pShader,
		double xScreen, double yScreen, double zScreen, double fpAspectRatio )
{
	S4DMatrix			matProjection ;
	const SGLAffine *	pAffineRotation = nullptr ;
	if ( m_flagRotation )
	{
		pAffineRotation = &m_affineRotation ;
	}
	S4DMatrix::PerspectiveProjectionType
		persType = m_flagZBounds ? S4DMatrix::persZBoundsN1_1_YUp
									: S4DMatrix::persNoZBounds_YUp ;
	matProjection.PerspectiveProjection
		( xScreen * fpAspectRatio /*+ 0.49f*/,
				yScreen /*+ 0.49f*/, zScreen,
			m_sizeVirtual.w * fpAspectRatio, m_sizeVirtual.h,
			m_zNear, m_zFar, persType,
			m_xViewLeft*0, m_yViewBottom*0,
			m_vViewOffset.x * fpAspectRatio,
			m_vViewOffset.y, pAffineRotation ) ;
	if ( m_flagViewport )
	{
		matProjection.PerspectiveProjection
			( (xScreen * m_affineViewport.a11
							+ m_affineViewport.a13) * fpAspectRatio,
				yScreen * m_affineViewport.a22
							+ m_affineViewport.a23, zScreen,
				m_sizeVirtual.w * fpAspectRatio, m_sizeVirtual.h,
				m_zNear, m_zFar, persType,
				0, 0,
				m_vViewOffset.x * fpAspectRatio,
				m_vViewOffset.y, pAffineRotation ) ;
		S4DMatrix	mat
			( m_affineViewport.a11, m_affineViewport.a12, 0.0, 0.0,
				m_affineViewport.a21, m_affineViewport.a22, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0 ) ;
		matProjection *= mat ;
	}

	if ( pShader != nullptr )
	{
		pShader->SetPerspectiveMatrix( matProjection ) ;
		pShader->SetProjectionScreen
			( (float32_t) xScreen,
				(float32_t) yScreen, (float32_t) zScreen ) ;
	}
	else
	{
		glMatrixMode( GL_PROJECTION ) ;
		SGLOpenGLContext::VerifyError( "glMatrixMode(GL_PROJECTION)" ) ;

		glLoadIdentity() ;
		SGLOpenGLContext::VerifyError( "glLoadIdentity" ) ;

		GLfloat	mat[4][4] ;
		for ( int i = 0; i < 4; i ++ )
		{
			mat[i][0] = matProjection.m[0][i] ;
			mat[i][1] = matProjection.m[1][i] ;
			mat[i][2] = matProjection.m[2][i] ;
			mat[i][3] = matProjection.m[3][i] ;
		}
		glMultMatrixf( &mat[0][0] ) ;
		SGLOpenGLContext::VerifyError( "glMultMatrixf" ) ;
	}
	m_flagOrthogonal = false ;
	m_flagMatrixPers = true ;
	m_matPerspective = matProjection ;
	m_matPerspectM22 = matProjection.m[2][2] ;
	m_matPerspectM23 = matProjection.m[2][3] ;

	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->AttachGLView( this ) ;
	}
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::PutOpenGLPerspectiveMatrix
	( SGLOpenGLShaderProgram * pShader,
		const S4DMatrix& matPers, const S3DVector& vScreenPos )
{
	if ( pShader != nullptr )
	{
		pShader->SetPerspectiveMatrix( matPers ) ;
		pShader->SetProjectionScreen
			( vScreenPos.x, vScreenPos.y, vScreenPos.z ) ;
	}
	else
	{
		glMatrixMode( GL_PROJECTION ) ;
		SGLOpenGLContext::VerifyError( "glMatrixMode(GL_PROJECTION)" ) ;

		glLoadIdentity() ;
		SGLOpenGLContext::VerifyError( "glLoadIdentity" ) ;

		GLfloat	mat[4][4] ;
		for ( int i = 0; i < 4; i ++ )
		{
			mat[i][0] = matPers.m[0][i] ;
			mat[i][1] = matPers.m[1][i] ;
			mat[i][2] = matPers.m[2][i] ;
			mat[i][3] = matPers.m[3][i] ;
		}
		glMultMatrixf( &mat[0][0] ) ;
		SGLOpenGLContext::VerifyError( "glMultMatrixf" ) ;
	}
	m_flagOrthogonal = false ;
	m_flagMatrixPers = true ;
	m_matPerspective = matPers ;
	m_matPerspectM22 = matPers.m[2][2] ;
	m_matPerspectM23 = matPers.m[2][3] ;

	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->AttachGLView( this ) ;
	}
}

// モデル座標系を初期設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::InitializeOpenGLModelView
	( SGLOpenGLShaderProgram * pShader,
		bool xReverse, bool yReverse, bool zReverse, bool fReverseFace )
{
	if ( pShader != nullptr )
	{
		S4DMatrix	mat4 ;
		float	x = 1.0f, y = 1.0f, z = 1.0f ;
		if ( xReverse )
		{
			x = -1.0f ;
			fReverseFace = !fReverseFace ;
		}
		if ( yReverse )
		{
			y = -1.0f ;
			fReverseFace = !fReverseFace ;
		}
		if ( zReverse )
		{
			z = -1.0f ;
			fReverseFace = !fReverseFace ;
		}
		mat4.InitializeMatrix( x, y, z, 1.0f ) ;
		pShader->SetModelViewMatrix( mat4 ) ;
	}
	else
	{
		glMatrixMode( GL_MODELVIEW ) ;
		SGLOpenGLContext::VerifyError( "glMatrixMode(GL_MODELVIEW)" ) ;

		glLoadIdentity() ;
		SGLOpenGLContext::VerifyError( "glLoadIdentity" ) ;

		if ( xReverse | yReverse | zReverse )
		{
			GLfloat	x = 1.0f, y = 1.0f, z = 1.0f ;
			if ( xReverse )
			{
				x = -1.0f ;
				fReverseFace = !fReverseFace ;
			}
			if ( yReverse )
			{
				y = -1.0f ;
				fReverseFace = !fReverseFace ;
			}
			if ( zReverse )
			{
				z = -1.0f ;
				fReverseFace = !fReverseFace ;
			}
			glScalef( x, y, z ) ;
			SGLOpenGLContext::VerifyError( "glScalef" ) ;
		}
	}
	if ( fReverseFace )
	{
		glFrontFace( GL_CW ) ;
		SGLOpenGLContext::VerifyError( "glFrontFace(GL_CW)" ) ;
	}
	else
	{
		glFrontFace( GL_CCW ) ;
		SGLOpenGLContext::VerifyError( "glFrontFace(GL_CCW)" ) ;
	}
}

// OpenGL depth 値 -> z 値変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::ZValueFromDepth
	( float32_t* pzDst, const float32_t* pzSrc, size_t nCount )
{
	float32_t	m22 = m_matPerspectM22 ;
	float32_t	m23 = m_matPerspectM23 ;
	if ( m_flagOrthogonal )
	{
		float32_t	im22 = 1.0f / m22 ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pzDst[i] = (m23 - pzSrc[i]) * im22 ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pzDst[i] = m23 / (pzSrc[i] - m22) ;
		}
	}
}

SGLError SGLOpenGLView::ZBufferFromDepth( SGLImageBuffer& imgbuf )
{
	if ( ((imgbuf.format & formatImageTypeMask) != formatImageZ)
				| (imgbuf.depth != 32) | (imgbuf.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint8_t *	pbytLine = imgbuf.ptrBuffer ;
	if ( pbytLine == nullptr )
	{
		return	sglErrFailed ;
	}
	uint32_t	width = imgbuf.width ;
	uint32_t	height = imgbuf.height ;
	int32_t		pitchLine = imgbuf.pitchLine ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		ZValueFromDepth
			( (float32_t*) pbytLine, (const float32_t*) pbytLine, width ) ;
		pbytLine += pitchLine ;
	}
	return	sglErrSuccess ;
}

// z 値 -> OpenGL depth 値変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::DepthFromZValue
	( float32_t* pzDst, const float32_t* pzSrc, size_t nCount )
{
	float32_t	m22 = m_matPerspectM22 ;
	float32_t	m23 = m_matPerspectM23 ;
	if ( m_flagOrthogonal )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pzDst[i] = m22 * pzSrc[i] + m23 ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			float32_t	z = pzSrc[i] ;
			pzDst[i] = (m22 * z + m23) / z ;
		}
	}
}

SGLError SGLOpenGLView::DepthFromZBuffer( SGLImageBuffer& imgbuf )
{
	if ( ((imgbuf.format & formatImageTypeMask) != formatImageZ)
				| (imgbuf.depth != 32) | (imgbuf.pitchPixel != 4) )
	{
		return	sglErrFailed ;
	}
	uint8_t *	pbytLine = imgbuf.ptrBuffer ;
	if ( pbytLine == nullptr )
	{
		return	sglErrFailed ;
	}
	uint32_t	width = imgbuf.width ;
	uint32_t	height = imgbuf.height ;
	int32_t		pitchLine = imgbuf.pitchLine ;
	for ( uint32_t y = 0; y < height; y ++ )
	{
		DepthFromZValue
			( (float32_t*) pbytLine, (const float32_t*) pbytLine, width ) ;
		pbytLine += pitchLine ;
	}
	return	sglErrSuccess ;
}

// 変換行列取得（論理座標⇔物理座標）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::GetAffineVirtualToPhysics( SGLAffine& affine )
{
	SGLAffine	afViewOffset
		( 1.0f, 0.0f, m_vViewOffset.x,
			0.0f, 1.0f, m_vViewOffset.y ) ;
	SGLAffine	afViewPort
		( 1.0f, 0.0f, (float32_t) m_rectViewPort.x,
			0.0f, 1.0f, (float32_t) m_rectViewPort.y ) ;
	affine = afViewPort * m_affineView * afViewOffset ;
}

// 座標変換（論理座標⇔物理座標）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::VirtualPointFromPhysics( SGL2DVector<double,double>& v )
{
	double	x0 = v.x - m_rectViewPort.x ;
	double	y0 = v.y - m_rectViewPort.y ;
	v.x = x0 * m_affineIView.a11
				+ y0 * m_affineIView.a12
				+ m_affineIView.a13 - m_vViewOffset.x ;
	v.y = x0 * m_affineIView.a21
				+ y0 * m_affineIView.a22
				+ m_affineIView.a23 - m_vViewOffset.y ;
}

void SGLOpenGLView::VirtualPointToPhysics( SGL2DVector<double,double>& v )
{
	double	x0 = v.x + m_vViewOffset.x ;
	double	y0 = v.y + m_vViewOffset.y ;
	v.x = x0 * m_affineView.a11
				+ y0 * m_affineView.a12
					+ (m_affineView.a13 + m_rectViewPort.x) ;
	v.y = x0 * m_affineView.a21
				+ y0 * m_affineView.a22
					+ (m_affineView.a23 + m_rectViewPort.y) ;
}

// 座標変換（論理座標⇔物理ビューポート座標）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLView::VertexFromPhysics( SGL2DVector<double,double>& v )
{
	double	x0 = v.x ;
	double	y0 = v.y ;
	v.x = x0 * m_affineIView.a11
				+ y0 * m_affineIView.a12
				+ m_affineIView.a13 - m_vViewOffset.x ;
	v.y = x0 * m_affineIView.a21
				+ y0 * m_affineIView.a22
				+ m_affineIView.a23 - m_vViewOffset.y ;
}

void SGLOpenGLView::VertexToPhysics( SGL2DVector<double,double>& v )
{
	double	x0 = v.x + m_vViewOffset.x ;
	double	y0 = v.y + m_vViewOffset.y ;
	v.x = x0 * m_affineView.a11
				+ y0 * m_affineView.a12 + m_affineView.a13 ;
	v.y = x0 * m_affineView.a21
				+ y0 * m_affineView.a22 + m_affineView.a23 ;
}



//////////////////////////////////////////////////////////////////////////////
// SGLImageBuffer - OpenGL テクスチャ変換 SGLImageBufferInterface
//////////////////////////////////////////////////////////////////////////////

// OpenGL ピクセルフォーマット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::FromImageInfo( const SGLImageBuffer& imginf )
{
	glTarget = GL_TEXTURE_2D ;
	glLayer = 0 ;
	if ( imginf.flagsBuffer & flagImageCubemap )
	{
		static const GLenum	s_glTarget[8] =
		{
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		} ;
		size_t	iCubeIndex =
					(size_t) ((imginf.flagsBuffer & flagImageCubeIndexMask)
												>> flagImageCubeIndexShifter) ;
		ESLAssert( iCubeIndex < 6 ) ;
		glTarget = s_glTarget[iCubeIndex] ;
	}
	else if ( imginf.flagsBuffer & flagImageTextureArray )
	{
		glTarget = GL_TEXTURE_2D_ARRAY ;
		glLayer = (GLint) ((imginf.flagsBuffer & flagImageLayerIndexMask)
											>> flagImageLayerIndexShifter) ;
	}
	else if ( imginf.flagsBuffer & flagImageTexture3D )
	{
		glTarget = GL_TEXTURE_3D ;
	}
	else if ( imginf.flagsBuffer & flagImageTextureMultisample )
	{
		glTarget = GL_TEXTURE_2D_MULTISAMPLE ;
	}
	glInternalFormat = GL_BGRA_EXT ;
	glFormat = GL_BGRA_EXT ;
	glType = GL_UNSIGNED_BYTE ;
	switch ( imginf.format & formatImageTypeMask )
	{
	case	formatImageRGB:
		if ( !((imginf.width | imginf.height) & 0x03)
			&& (imginf.flagsBuffer & flagImageCompressedTexture) )
		{
			#if	defined(__PLATFORM_ANDROID__)
				#if	ANDROID_API_LEVEL >= 8
					glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ;
				#endif
			#else
				glInternalFormat = GL_COMPRESSED_RGBA ;
			#endif
		}
		break ;
	case	formatImageBGR:
		glInternalFormat = GL_RGBA ;
		glFormat = GL_RGBA ;
		if ( !((imginf.width | imginf.height) & 0x03)
			&& (imginf.flagsBuffer & flagImageCompressedTexture) )
		{
			#if	defined(__PLATFORM_ANDROID__)
				#if	ANDROID_API_LEVEL >= 8
					glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ;
				#endif
			#else
				glInternalFormat = GL_COMPRESSED_RGBA ;
			#endif
		}
		break ;
	case	formatImageGray:
		if ( imginf.depth == 8 )
		{
			glInternalFormat = GL_R8 ;
			glFormat = GL_RED ;
		}
		else if ( (imginf.format & formatImageFlagAlpha) && (imginf.depth == 16) )
		{
			glInternalFormat = GL_RG8 ;
			glFormat = GL_RG ;
		}
		break ;
	case	(formatImageGray | formatImageFlagFloat):
		if ( imginf.depth == 32 )
		{
			glInternalFormat = GL_R32F ;
			glFormat = GL_RED ;
			glType = GL_FLOAT ;
		}
		else if ( (imginf.format & formatImageFlagAlpha) && (imginf.depth == 32*2) )
		{
			glInternalFormat = GL_RG32F ;
			glFormat = GL_RG ;
			glType = GL_FLOAT ;
		}
		break ;
	case	formatImageZ:
	case	formatImageDepth:
		#if	defined(__PLATFORM_ANDROID__)
			glInternalFormat = GL_DEPTH_COMPONENT ;
			glFormat = GL_DEPTH_COMPONENT ;
			glType = GL_UNSIGNED_SHORT ;
		#else
			glInternalFormat = GL_DEPTH_COMPONENT ;
			glFormat = GL_DEPTH_COMPONENT ;
			glType = GL_FLOAT ;
		#endif
		break ;
	case	formatImageFloatBGR:
		if ( imginf.format & formatImageFlagAlpha )
		{
			// formatImageFloatRGBA
			glInternalFormat = GL_RGBA32F ;
			glFormat = GL_RGBA ;
			glType = GL_FLOAT ;
		}
		else
		{
			glInternalFormat = GL_RGB32F ;
			glFormat = GL_RGB ;
			glType = GL_FLOAT ;
		}
		break ;
	case	formatImageRGB_S3TC_DXT1:
		if ( imginf.format & formatImageFlagAlpha )
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ;
			glFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ;
		}
		else
		{
			glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT ;
			glFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT ;
		}
		break ;
	default:
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsTexture2D( void ) const
{
	return	(glTarget == GL_TEXTURE_2D) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsTexture2DArray( void ) const
{
	return	(glTarget == GL_TEXTURE_2D_ARRAY) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsTexture3D( void ) const
{
	return	(glTarget == GL_TEXTURE_3D) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsTextureMultisample( void ) const
{
	return	(glTarget == GL_TEXTURE_2D_MULTISAMPLE) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsCubemap( void ) const
{
	return	(glTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
			| (glTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
			| (glTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
			| (glTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
			| (glTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
			| (glTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatRGB( void ) const
{
	return	((glInternalFormat == GL_BGRA_EXT)
				| (glInternalFormat == GL_RGBA)
				| (glInternalFormat == GL_RGB))
			|| IsFormatCompressedRGB() ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatCompressedRGB( void ) const
{
	#if	defined(__PLATFORM_ANDROID__)
		#if	ANDROID_API_LEVEL >= 8
			return	(glInternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ;
		#else
			return	false ;
		#endif
	#else
		return	(glInternalFormat == GL_COMPRESSED_RGBA)
				|| (glInternalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
				|| (glInternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ;
	#endif
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatCompressedSource( void ) const
{
	return	(glFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
			|| (glFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatFloatRGB( void ) const
{
	return	((glInternalFormat == GL_RGBA32F)
				| (glInternalFormat == GL_RGB32F)
				| (glInternalFormat == GL_RGBA16F)
				| (glInternalFormat == GL_RGB16F)) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatFloat( void ) const
{
	return	(glType == GL_FLOAT) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::MakeUncompressed( void )
{
	#if	defined(__PLATFORM_ANDROID__)
		#if	ANDROID_API_LEVEL >= 8
			if ( glInternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )
			{
				glInternalFormat = GL_RGBA ;
				return	true ;
			}
		#endif
	#else
		if ( glInternalFormat == GL_COMPRESSED_RGBA )
		{
			glInternalFormat = GL_RGBA ;
			return	true ;
		}
	#endif
	return	false ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatGray( void ) const
{
	return	(glFormat == GL_RED)
			|| (glFormat == GL_LUMINANCE) ;
}

bool SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT::IsFormatDepth( void ) const
{
	return	(glInternalFormat == GL_DEPTH_COMPONENT) ;
}

bool	SGLOpenGLTextureBuffer::m_flagNotSupportedBGRA = false ;
size_t	SGLOpenGLTextureBuffer::m_countRetryTextureBGRA = 0 ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLTextureBuffer, SGLImageBufferInterface )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLTextureBuffer::GLResource, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::SGLOpenGLTextureBuffer( void )
{
	m_typeObject = imageObjectGLTexture ;
	m_pFirstGLRsrc = nullptr ;
	m_pImageBuf = nullptr ;
	m_pRsrc = nullptr ;
	m_flagMipmap = false ;
//	m_flagMipmapped = false ;
//	m_glTexture = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::~SGLOpenGLTextureBuffer( void )
{
	delete	m_pFirstGLRsrc ;
	m_pFirstGLRsrc = nullptr ;
}

SGLOpenGLTextureBuffer::GLResource::~GLResource( void )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->DetachNotifyObject( this ) ;
	}
	if ( m_glTexture || m_glRenderBuffer )
	{
		if ( m_flagOwnTexture )
		{
			const size_t	bytesUsed = m_nGLTexBytes ;
			if ( pOpenGL != nullptr )
			{
				pOpenGL->Procedure
					( new TextureDestroyer
						( pOpenGL, m_glTexture, m_glRenderBuffer, bytesUsed ),
						S3DRenderDevice::procedureDelayable ) ;
			}
			else
			{
				SGLOpenGLContext::ProcedureOnGLThread
					( new TextureDestroyer
						( pOpenGL, m_glTexture, m_glRenderBuffer, bytesUsed ),
						S3DRenderDevice::procedureDelayable ) ;
			}
		}
		m_flagOwnTexture = false ;
		m_glTexture = 0 ;
		m_glRenderBuffer = 0 ;
	}
	GLResource *	pNextRsrc = m_pNextRsrc ;
	m_pNextRsrc = nullptr ;
	delete	pNextRsrc ;
}

// 同期
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::Lock( void ) const
{
	m_csSync.Lock() ;
}

void SGLOpenGLTextureBuffer::Unlock( void ) const
{
	m_csSync.Unlock() ;
}

// GLResource 取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLTextureBuffer::GetResourceAs
		( SGLOpenGLTextureBuffer * pTxtBuf, SGLOpenGLContext * pOpenGL )
{
	GLResource *	pRsrc = nullptr ;
	if ( (pTxtBuf != nullptr) & (pOpenGL != nullptr) )
	{
		SSmartLock<SCriticalSection>	lock( &(pTxtBuf->m_csSync) ) ;
		GLResource *	pLastRsrc = nullptr ;
		pRsrc = pTxtBuf->m_pFirstGLRsrc ;
		while ( pRsrc != nullptr )
		{
			if ( pRsrc->m_refOpenGL.GetReference() == pOpenGL )
			{
				break ;
			}
			pLastRsrc = pRsrc ;
			pRsrc = pRsrc->m_pNextRsrc ;
		}
		if ( pRsrc == nullptr )
		{
			pRsrc = new GLResource( pTxtBuf ) ;
			if ( pLastRsrc != nullptr )
			{
				pLastRsrc->m_pNextRsrc = pRsrc ;
			}
			else
			{
				pTxtBuf->m_pFirstGLRsrc = pRsrc ;
			}
			pRsrc->m_refOpenGL = pOpenGL ;
			//
			pOpenGL->AddNotifyObject( pRsrc ) ;
		}
	}
	return	pRsrc ;
}

// GLResource 削除
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::ReleaseResource
			( SGLOpenGLTextureBuffer::GLResource * pRsrc )
{
	if ( m_pFirstGLRsrc != nullptr )
	{
		SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
		if ( m_pFirstGLRsrc == pRsrc )
		{
			m_pFirstGLRsrc = pRsrc->m_pNextRsrc ;
		}
		else
		{
			GLResource *	pLastRsrc = m_pFirstGLRsrc ;
			GLResource *	pNextRsrc = pLastRsrc->m_pNextRsrc ;
			while ( pNextRsrc != nullptr )
			{
				if ( pNextRsrc == pRsrc )
				{
					pLastRsrc->m_pNextRsrc = pRsrc->m_pNextRsrc ;
					break ;
				}
				pLastRsrc = pNextRsrc ;
				pNextRsrc = pNextRsrc->m_pNextRsrc ;
			}
		}
		pRsrc->m_pNextRsrc = nullptr ;
	}
	delete	pRsrc ;
}

// OpenGL テクスチャ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::AttachGLTexture
	( SGLOpenGLContext * pOpenGL,
		SGLImageBuffer * pImageBuf,
		GLuint glTexture, bool fAutoDelete )
{
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	m_pImageBuf = pImageBuf ;
	//
	if ( pOpenGL == nullptr )
	{
		return	sglErrFailed ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SGLError		err ;
	GLResource *	pRsrc ;
	pRsrc = GetResourceAs( this, pOpenGL ) ;
	err = pRsrc->AttachGLTexture( pImageBuf, glTexture, fAutoDelete ) ; 
	//
//	m_imginf = pRsrc->m_imginf ;
//	m_flagMipmapped = pRsrc->m_flagMipmapped ;
//	m_glTexture = pRsrc->m_glTexture ;
	//
	return	err ;
}

// クリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::ClearBuffer
	( SGLImageBuffer * pImageBuf, SGLPalette pxClear )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	GLResource *	pRsrc = m_pFirstGLRsrc ;
	while ( pRsrc != nullptr )
	{
		pRsrc->m_flagUpdate = false ;
		pRsrc->m_flagClear = true ;
		pRsrc->m_colorClear = pxClear ;
		pRsrc = pRsrc->m_pNextRsrc ;
	}
	return	sglErrSuccess ;
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	GLResource *	pRsrc = m_pFirstGLRsrc ;
	while ( pRsrc != nullptr )
	{
		if ( !pRsrc->m_flagUpdate )
		{
			pRsrc->m_flagUpdate = true ;
			if ( pRect != nullptr )
			{
				pRsrc->m_rectUpdate = *pRect ;
			}
		}
		if ( pRect != nullptr )
		{
			pRsrc->m_rectUpdate =
				SGLRect(pRsrc->m_rectUpdate) | SGLRect(*pRect) ;
		}
		else
		{
			pRsrc->m_rectUpdate = pImageBuf->GetImageRect() ;
		}
		pRsrc->m_zUpdateFirst = 0 ;
		pRsrc->m_zUpdateCount = (size_t) -1 ;
		pRsrc->m_flagClear = false ;
		pRsrc = pRsrc->m_pNextRsrc ;
	}
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	// ※OpenGL がアタッチされたスレッド上で実行されること
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	return	CommitBufferProgressively( pOpenGL, pImageBuf, 0 ) ;
}

// 更新確定処理（一度で確定せず少しづつ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::CommitBufferProgressively
	( SGLOpenGLContext * pOpenGL,
		SGLImageBuffer * pImageBuf, size_t nProgressivePixelCount )
{
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// ※OpenGL がアタッチされたスレッド上で実行されること
	//
	if ( pOpenGL == nullptr )
	{
//		Trace( "SGLOpenGLTextureBuffer::CommitBuffer"
//					" is called from non OpenGL thread.\n" ) ;
		return	sglErrFailed ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;

	GLResource *	pRsrc = GetResourceAs( this, pOpenGL ) ;
	//
	m_pImageBuf = pImageBuf ;
	m_pRsrc = pRsrc ;
	//
	SGLError	err = sglErrSuccess ;
	if ( pRsrc->m_flagUpdate || pRsrc->m_flagClear )
	{
		//
		// VRAM への転送が必要
		//
		bool	flagUpdate = pRsrc->m_flagUpdate ;
		pRsrc->m_flagUpdate = false ;
		//
		SGLImageBuffer *	pOrgBuf = pImageBuf ;
		while ( pOrgBuf->ptrRefOriginal != nullptr )
		{
			pOrgBuf = pOrgBuf->ptrRefOriginal ;
		}
		GL_PIXEL_FORMAT	glpf( *pOrgBuf ) ;
		if ( (pRsrc->m_glTexture == 0) & (pRsrc->m_glRenderBuffer == 0) )
		{
			//
			// OpenGL 用テクスチャオブジェクト生成
			//
			CreateGLTexture( pOpenGL, pRsrc, glpf, pOrgBuf ) ;
		}
		bool	fDualBuffer =
			(pRsrc->m_glTexture != 0) & (pRsrc->m_glRenderBuffer != 0) ;
		bool	fWriteBuffer =
					!(pOrgBuf->flagsBuffer & flagImageNoWriteBuffer) ;
		if ( flagUpdate
			&& (pRsrc->m_glTexture != 0)
			&& (pOrgBuf->ptrBuffer != nullptr)
			&& (fDualBuffer | fWriteBuffer) )
		{
			glActiveTexture( (GLenum) (GL_TEXTURE0 + pOpenGL->m_iTempTexture) ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
			//
			SGLImageInfo *	pRefImage ;
			uint8_t *	pbytBuffer =
				SGLImageSystemMemory::GetMemoryOf( pOrgBuf, pRefImage ) ;
			glBindTexture( glpf.glTarget, pRsrc->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
//			pRsrc->m_flagMipmapped = false ;
			/*
			#if	!defined(__API_OPEN_GL_ES__)
			if ( (m_flagMipmap
					|| (pOrgBuf->flagsBuffer & flagImageMipmap))
							&& glpf.IsFormatRGB() )
			{
				// gluBuild2DMipmaps での mipmap 生成
				err = Build2DMipmaps( pOpenGL, pRsrc, glpf, pOrgBuf ) ;
			}
			else
			#endif
			*/
			{
				if ( glpf.IsFormatDepth() && (pOpenGL != nullptr)
					&& fWriteBuffer
					&& !(pOrgBuf->format & formatImageFlagDepth) )
				{
					// z バッファで formatImageFlagDepth フラグが
					// 設定されていない場合、z->depth 変換を行う
					SGLOpenGLView *	pglView = pOpenGL->GetGLView() ;
					if ( pglView != nullptr )
					{
						Trace( "convert Depth from ZBuffer in "
								"SGLOpenGLTextureBuffer::CommitBuffer.\n" ) ;
						pglView->DepthFromZBuffer( *pImageBuf ) ;
					}
				}
				//
				// 画像データを VRAM に転送
				//
				if ( glpf.IsTexture2D() )
				{
					//
					// 2D テクスチャ転送
					//
					if ( glpf.IsFormatCompressedSource() )
					{
						// 圧縮済み画像
						err = UpdateCompressedTexture2D
								( pOpenGL, pRsrc, glpf,
									pOrgBuf, pRefImage,
									pbytBuffer, nProgressivePixelCount ) ;
					}
					else
					{
						// 非圧縮画像
						err = UpdateTexture2D
								( pOpenGL, pRsrc, glpf,
									pOrgBuf, pRefImage,
									pbytBuffer, nProgressivePixelCount ) ;
					}
				}
				else if ( glpf.IsCubemap() )
				{
					//
					// Cube テクスチャ転送
					//
					SGLSize	sizeFace( pImageBuf->width, pImageBuf->height ) ;
					err = UpdateTextureCubemap
							( pOpenGL, pRsrc, glpf, sizeFace, pRefImage, pbytBuffer ) ;
				}
				else if ( glpf.IsTexture3D() || glpf.IsTexture2DArray() )
				{
					//
					// 3D テクスチャ転送
					//
					err = UpdateTexture3D
							( pOpenGL, pRsrc, glpf,
								pOrgBuf, pRefImage,
								pbytBuffer, nProgressivePixelCount ) ;
				}
				else
				{
					ESLTrace( "failed to update texture : invalid texture type.\n" ) ;
				}
			}
			if ( (err == sglErrFailed) && glpf.IsTexture2D() && glpf.IsFormatRGB() )
			{
				//
				// カラーテクスチャで失敗した場合は
				// GL_RGBA フォーマットで生成を試みる
				//
				RetryUpdateTexture2DwithRGBA
					( pOpenGL, pRsrc, glpf, pImageBuf, pOrgBuf, pbytBuffer ) ;
			}
			glBindTexture( glpf.glTarget, 0 ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D,0)" ) ;
		}
	}
	return	err ;
}

// OpenGL テクスチャ生成
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::CreateGLTexture
	( SGLOpenGLContext * pOpenGL,
		GLResource * pRsrc,
		const GL_PIXEL_FORMAT& glpf, SGLImageBuffer * pOrgBuf )
{
	bool	flagRenderBuffer =
				((pOrgBuf->flagsBuffer & flagImageRenderBufferStorage) != 0) ;
	if ( !flagRenderBuffer )
	{
		if ( glpf.IsFormatDepth() )
		{
			// 深度テクスチャに対応していない場合には Renderbuffer
			flagRenderBuffer = !pOpenGL->m_flagDepthTexture ;
		}
		else if ( glpf.IsTextureMultisample() )
		{
			#if	defined(__API_OPEN_GL_ES__)
				// OpenGL ES ではマルチサンプルテクスチャは Renderbuffer
				flagRenderBuffer = true ;
			#endif
		}
		/*
		else if ( !glpf.IsFormatRGB()
				&& !glpf.IsFormatFloatRGB()
				&& (pOrgBuf->ptrBuffer == nullptr) )
		{
			// RGB/RGBA/Depth 形式以外でデバイスメモリ指定
			flagRenderBuffer = true ;
		}
		*/
	}
	if ( pOrgBuf->flagsBuffer & flagImageMipmap )
	{
		m_flagMipmap = true ;
	}
	if ( !flagRenderBuffer )
	{
		SGLImageBuffer *	pInitBuf = pOrgBuf ;
		/*
		#if	!defined(__API_OPEN_GL_ES__)
		if ( m_flagMipmap && glpf.IsFormatRGB() )
		{
			// gluBuild2DMipmaps での mipmap 生成の場合
			pInitBuf = nullptr ;
		}
		#endif
		*/
		glActiveTexture( (GLenum) (GL_TEXTURE0 + pOpenGL->m_iTempTexture) ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		pRsrc->CreateGLTexture( pInitBuf ) ;
	}
	else
	{
		pRsrc->CreateGLRenderbuffer( pOrgBuf ) ;
	}
}

// Mipmap 生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::Build2DMipmaps
	( SGLOpenGLContext * pOpenGL, GLResource * pRsrc,
		const GL_PIXEL_FORMAT& glpf, SGLImageBuffer * pOrgBuf )
{
	bool	fSuccessed = false ;
#if	!defined(__API_OPEN_GL_ES__)
	pRsrc->m_flagMipmapped = true ;
	Trace( "gluBuild2DMipmaps %dx%d in "
			"SGLOpenGLTextureBuffer::CommitBuffer.\n",
						pOrgBuf->width, pOrgBuf->height ) ;
	if ( !m_flagNotSupportedBGRA
		| (glpf.glInternalFormat != GL_BGRA_EXT) )
	{
		gluBuild2DMipmaps
			( glpf.glTarget, glpf.glInternalFormat,
				pOrgBuf->width, pOrgBuf->height,
				glpf.glFormat, glpf.glType, pOrgBuf->ptrBuffer ) ;
		pRsrc->m_imginf = *pOrgBuf ;
		fSuccessed =
			SGLOpenGLContext::VerifyError
					( "gluBuild2DMipmaps(GL_TEXTURE_2D)" ) ;
	}
	if ( !fSuccessed & (glpf.glInternalFormat == GL_BGRA_EXT) )
	{
		m_flagNotSupportedBGRA = true ;
		gluBuild2DMipmaps
			( glpf.glTarget, GL_RGBA,
				pOrgBuf->width, pOrgBuf->height,
				glpf.glFormat, glpf.glType, pOrgBuf->ptrBuffer ) ;
		fSuccessed =
			SGLOpenGLContext::VerifyError
				( "gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA)" ) ;
		if ( fSuccessed && (++ m_countRetryTextureBGRA >= 128) )
		{
			pRsrc->m_flagMipmapped = false ;
			m_flagNotSupportedBGRA = false ;
			m_countRetryTextureBGRA = 0 ;
		}
	}
	pOpenGL->m_pflog.countTransmitPixel += pOrgBuf->width * pOrgBuf->height ;
#endif
	return	fSuccessed ? sglErrSuccess : sglErrFailed ;
}

// Cubemap テクスチャを転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::UpdateTextureCubemap
	( SGLOpenGLContext * pOpenGL, GLResource * pRsrc,
		const GL_PIXEL_FORMAT& glpf,
		const SGLSize& sizeFace,
		SGLImageInfo * pRefImage, uint8_t * pbytBuffer )
{
	static const GLenum	s_glTarget[6] =
	{
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	} ;
	// ※ Cube テクスチャの転送は６面すべて行う
	bool	fSuccessed = true ;
	for ( size_t iFace = 0; iFace < 6; iFace ++ )
	{
		SGLRect	rectFace ;
		rectFace.left = 0 ;
		rectFace.top = (int32_t) (iFace * sizeFace.h) ;
		rectFace.SetWidth( sizeFace.w ) ;
		rectFace.SetHeight( sizeFace.h ) ;
		//
		if ( rectFace &= pRsrc->m_rectUpdate )
		{
			glTexSubImage2D
				( s_glTarget[iFace], 0,
					0, 0, sizeFace.w, sizeFace.h,
					glpf.glFormat, glpf.glType,
					pbytBuffer + (sizeFace.h * iFace) * pRefImage->pitchLine ) ;
			SGLOpenGLContext::VerifyError
				( "glTexSubImage2D(GL_TEXTURE_CUBE_MAP_*)" ) ;
			//
			pOpenGL->m_pflog.countTransmitPixel += sizeFace.w * sizeFace.h ;
		}
	}
	return	fSuccessed ? sglErrSuccess : sglErrFailed ;
}

// 3D テクスチャを転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::UpdateTexture3D
	( SGLOpenGLContext * pOpenGL, GLResource * pRsrc,
		const GL_PIXEL_FORMAT& glpf,
		SGLImageBuffer * pOrgBuf, SGLImageInfo * pRefImage,
		uint8_t * pbytBuffer, size_t nProgressivePixelCount )
{
	bool	fSuccessed = false ;
	bool	fContinue = false ;
	ssize_t	nLayerPitch = pOrgBuf->pitchLine * pOrgBuf->sizeFrame.h ;
	GLint	zOffset = 0 ;
	GLsizei	zDepth = (GLsizei) pOrgBuf->nFrameCount ;
	if ( nProgressivePixelCount != 0 )
	{
		size_t	nFramePixels = pOrgBuf->sizeFrame.w * pOrgBuf->sizeFrame.h ;
		if ( nFramePixels == 0 )
		{
			nFramePixels = 1 ;
		}
		zOffset = (GLint) pRsrc->m_zUpdateFirst ;
		ESLAssert( (size_t) zOffset < pOrgBuf->nFrameCount ) ;
		if ( (size_t) zOffset >= pOrgBuf->nFrameCount )
		{
			return	sglErrSuccess ;
		}
		zDepth = (GLsizei) ((nProgressivePixelCount + nFramePixels - 1) / nFramePixels) ;
		if ( (pRsrc->m_zUpdateCount != (size_t) -1)
				&& ((size_t) zDepth > pRsrc->m_zUpdateCount) )
		{
			zDepth = (GLsizei) pRsrc->m_zUpdateCount ;
		}
		if ( (size_t) (zOffset + zDepth) >= pOrgBuf->nFrameCount )
		{
			zDepth = (GLsizei) (pOrgBuf->nFrameCount - zOffset) ;
			pRsrc->m_zUpdateFirst = 0 ;
			pRsrc->m_zUpdateCount = (size_t) -1 ;
			pRsrc->m_flagUpdate = false ;
			fContinue = false ;
		}
		else
		{
			pRsrc->m_zUpdateFirst += zDepth ;
			pRsrc->m_zUpdateCount = (size_t) -1 ;
			pRsrc->m_flagUpdate = true ;
			fContinue = true ;
		}
	}
	glTexSubImage3D
		( glpf.glTarget /*GL_TEXTURE_3D*/, 0,
			0, 0, zOffset,
			(GLsizei) pOrgBuf->sizeFrame.w,
			(GLsizei) pOrgBuf->sizeFrame.h, zDepth,
			glpf.glFormat, glpf.glType,
			pbytBuffer + nLayerPitch * zOffset ) ;
	fSuccessed =
		SGLOpenGLContext::VerifyError( "glTexSubImage3D(GL_TEXTURE_3D)" ) ;
	//
	if ( m_flagMipmap && fSuccessed )
	{
		ESLTrace( "glGenerateMipmap\n" ) ;
		glGenerateMipmap( glpf.glTarget /*GL_TEXTURE_3D*/ ) ;
		if ( SGLOpenGLContext::VerifyError( "glGenerateMipmap(GL_TEXTURE_3D)" ) )
		{
			pRsrc->m_flagMipmapped = true ;
		}
	}
	pOpenGL->m_pflog.countTransmitPixel
			+= pOrgBuf->sizeFrame.w * pOrgBuf->sizeFrame.h * zDepth ;
	return	fSuccessed ? (fContinue ? sglErrContinue : sglErrSuccess) : sglErrFailed ;
}

// 2D テクスチャを転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::UpdateTexture2D
	( SGLOpenGLContext * pOpenGL,
		GLResource * pRsrc, const GL_PIXEL_FORMAT& glpf,
		SGLImageBuffer * pOrgBuf, SGLImageInfo * pRefImage,
		uint8_t * pbytBuffer, size_t nProgressivePixelCount )
{
	SGLImageRect	rectUpdate =
		SGLRect(pOrgBuf->GetImageRect()) & pRsrc->m_rectUpdate ;
	bool	flagUnderQuarter =
		((uint32_t) rectUpdate.w * 2 < pOrgBuf->width)
			&& (pOrgBuf->width * pOrgBuf->height
				> (uint32_t) (rectUpdate.w * rectUpdate.h * 4)) ;
	bool	flagNeedsConvert =
		(pRsrc->m_fmtNeedsFormat != 0)
			&& (pRsrc->m_fmtNeedsFormat != pOrgBuf->format) ;
	bool	fSuccessed = false ;
	bool	fContinue = false ;
	if ( flagUnderQuarter || flagNeedsConvert )
	{
		// 更新領域が全体の 1/4 以下の場合、更新部分だけ転送
		// ※ 1/4 以下ならメモリ間転送を1回行っても
		// 　トータル処理コストは半分程度以下には減る見込み
		// ※RGBフォーマットをBGRと相互変換が必要な場合も含む
		SGLImageBuffer	imgbufTemp ;
		pOpenGL->GetTemporaryImageBuffer
			( imgbufTemp, pOrgBuf->format,
				pOrgBuf->depth, rectUpdate.w, rectUpdate.h ) ;
		//
		GL_PIXEL_FORMAT	glpfTemp = glpf ; ;
		if ( flagNeedsConvert )
		{
			imgbufTemp.format = pRsrc->m_fmtNeedsFormat ;
			glpfTemp.FromImageInfo( imgbufTemp ) ;
			sglConvertImageBuffer
				( imgbufTemp, *pOrgBuf, 0, 0, &rectUpdate ) ;
		}
		else
		{
			sglCopyImageBuffer
				( imgbufTemp, *pOrgBuf, 0, 0, &rectUpdate ) ;
		}
		//
		glTexSubImage2D
			( glpf.glTarget, 0,
				rectUpdate.x, rectUpdate.y,
				rectUpdate.w, rectUpdate.h,
				glpfTemp.glFormat,
				glpfTemp.glType, imgbufTemp.ptrBuffer ) ;
		fSuccessed =
			SGLOpenGLContext::VerifyError
					( "glTexSubImage2D(GL_TEXTURE_2D) - rect" ) ;
		//
		pOpenGL->m_pflog.countTransmitPixel += rectUpdate.w * rectUpdate.h ;
	}
	else
	{
		// 画像の横幅いっぱい転送する
		if ( nProgressivePixelCount != 0 )
		{
			size_t	nUpdatePixels = pOrgBuf->width * rectUpdate.h ;
			if ( nUpdatePixels > nProgressivePixelCount * 2 )
			{
				// 一部だけ更新し、更新領域を残す
				size_t	hUpdate =
					(nProgressivePixelCount + pOrgBuf->width - 1) / pOrgBuf->width ;
				if ( glpf.IsFormatCompressedRGB() )
				{
					hUpdate &= ~0x03 ;
					if ( hUpdate < 4 )
					{
						hUpdate = esl_min( pRsrc->m_rectUpdate.h, 4 ) ;
					}
				}
				else if ( hUpdate < 0 )
				{
					hUpdate = 1 ;
				}
				pRsrc->m_rectUpdate.y += (int32_t) hUpdate ;
				pRsrc->m_rectUpdate.h -= (int32_t) hUpdate ;
				if ( pRsrc->m_rectUpdate.h > 0 )
				{
					fContinue = true ;
					pRsrc->m_flagUpdate = true ;
					rectUpdate.h = (int32_t) hUpdate ;
				}
			}
		}
		glTexSubImage2D
			( glpf.glTarget, 0,
				0, rectUpdate.y, pOrgBuf->width, rectUpdate.h,
				glpf.glFormat, glpf.glType,
				pbytBuffer + rectUpdate.y * pRefImage->pitchLine ) ;
		fSuccessed =
			SGLOpenGLContext::VerifyError
					( "glTexSubImage2D(GL_TEXTURE_2D) - band" ) ;
		//
		pOpenGL->m_pflog.countTransmitPixel += pOrgBuf->width * rectUpdate.h ;
	}
	if ( !fSuccessed )
	{
		//
		// 失敗した場合は glTexImage2D で生成を試みる
		//
		if ( !m_flagNotSupportedBGRA
			| (glpf.glInternalFormat != GL_BGRA_EXT) )
		{
			Trace( "glTexImage2D %dx%d in "
					"SGLOpenGLTextureBuffer::CommitBuffer.\n",
								pOrgBuf->width, pOrgBuf->height ) ;
			glTexImage2D
				( glpf.glTarget, 0, glpf.glInternalFormat,
					pOrgBuf->width, pOrgBuf->height, 0,
					glpf.glFormat, glpf.glType, pbytBuffer ) ;
			pRsrc->m_imginf = *pOrgBuf ;
			fSuccessed =
				SGLOpenGLContext::VerifyError
						( "glTexImage2D(GL_TEXTURE_2D)" ) ;
			if ( fSuccessed )
			{
				fContinue = false ;
				pRsrc->m_flagUpdate = false ;
			}
		}
	}
	if ( m_flagMipmap && fSuccessed && !fContinue )
	{
		ESLTrace( "glGenerateMipmap\n" ) ;
		glGenerateMipmap( glpf.glTarget /*GL_TEXTURE_2D*/ ) ;
		if ( SGLOpenGLContext::VerifyError( "glGenerateMipmap(GL_TEXTURE_2D)" ) )
		{
			pRsrc->m_flagMipmapped = true ;
		}
	}
	return	fSuccessed ? (fContinue ? sglErrContinue : sglErrSuccess) : sglErrFailed ;
}

// 圧縮済み 2D テクスチャを転送
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::UpdateCompressedTexture2D
	( SGLOpenGLContext * pOpenGL,
		GLResource * pRsrc, const GL_PIXEL_FORMAT& glpf,
		SGLImageBuffer * pOrgBuf, SGLImageInfo * pRefImage,
		uint8_t * pbytBuffer, size_t nProgressivePixelCount )
{
	SGLImageRect	rectUpdate =
		SGLRect(pOrgBuf->GetImageRect()) & pRsrc->m_rectUpdate ;
	bool	fSuccessed = false ;
	bool	fContinue = false ;

	if ( nProgressivePixelCount != 0 )
	{
		size_t	nUpdatePixels = pOrgBuf->width * rectUpdate.h ;
		if ( nUpdatePixels > nProgressivePixelCount * 2 )
		{
			// 一部だけ更新し、更新領域を残す
			size_t	hUpdate =
				(nProgressivePixelCount + pOrgBuf->width - 1) / pOrgBuf->width ;
			hUpdate &= ~0x03 ;
			if ( hUpdate < 4 )
			{
				hUpdate = esl_min( pRsrc->m_rectUpdate.h, 4 ) ;
			}
			pRsrc->m_rectUpdate.y += (int32_t) hUpdate ;
			pRsrc->m_rectUpdate.h -= (int32_t) hUpdate ;
			if ( pRsrc->m_rectUpdate.h > 0 )
			{
				fContinue = true ;
				pRsrc->m_flagUpdate = true ;
				rectUpdate.h = (int32_t) hUpdate ;
			}
		}
	}
	glCompressedTexSubImage2D
		( glpf.glTarget, 0,
			0, rectUpdate.y, pOrgBuf->width, rectUpdate.h,
			glpf.glFormat, rectUpdate.h * pRefImage->pitchLine,
			pbytBuffer + rectUpdate.y * pRefImage->pitchLine ) ;
	fSuccessed =
		SGLOpenGLContext::VerifyError
				( "glCompressedTexSubImage2D(GL_TEXTURE_2D)" ) ;
	//
	pOpenGL->m_pflog.countTransmitPixel += pOrgBuf->width * rectUpdate.h ;
	//
	if ( m_flagMipmap && fSuccessed && !fContinue )
	{
		ESLTrace( "glGenerateMipmap (compressed)\n" ) ;
		glGenerateMipmap( glpf.glTarget ) ;
		if ( SGLOpenGLContext::VerifyError( "glGenerateMipmap(GL_TEXTURE_2D)" ) )
		{
			pRsrc->m_flagMipmapped = true ;
		}
	}
	return	fSuccessed ? (fContinue ? sglErrContinue : sglErrSuccess) : sglErrFailed ;
}

// GL_RGBA フォーマットで 2D テクスチャの生成を再試行
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::RetryUpdateTexture2DwithRGBA
	( SGLOpenGLContext * pOpenGL, GLResource * pRsrc,
		const GL_PIXEL_FORMAT& glpf,
		SGLImageBuffer * pImageBuf,
		SGLImageBuffer * pOrgBuf, uint8_t * pbytBuffer )
{
	Trace( "glTexImage2D GL_RGBA, %d, %d.\n",
				pOrgBuf->width, pOrgBuf->height ) ;
	m_flagNotSupportedBGRA = true ;
	glTexImage2D
		( glpf.glTarget, 0, GL_RGBA,
			pOrgBuf->width, pOrgBuf->height, 0,
			glpf.glFormat, glpf.glType, pbytBuffer ) ;
	pRsrc->m_imginf = *pOrgBuf ;
	if ( !SGLOpenGLContext::VerifyError
				( "glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA)" ) )
	{
		Trace( "try convert RGBA format and glTexImage2D.\n" ) ;
		NormalizePixelFormat( pImageBuf, true ) ;
		//
		glTexImage2D
			( glpf.glTarget, 0, GL_RGBA,
				pOrgBuf->width, pOrgBuf->height, 0,
				GL_RGBA, glpf.glType, pbytBuffer ) ;
		pRsrc->m_imginf = *pOrgBuf ;
		SGLOpenGLContext::VerifyError
				( "glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,,,GL_RGBA)" ) ;
	}
	else
	{
		if ( ++ m_countRetryTextureBGRA >= 128 )
		{
			m_flagNotSupportedBGRA = false ;
			m_countRetryTextureBGRA = 0 ;
		}
	}
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	m_pImageBuf = pImageBuf ;
	//
	// ※描画ターゲットにこのテクスチャが設定されていること
	//
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	ESLAssert( pOpenGL != nullptr ) ;
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	GLResource *	pRsrc = GetResourceAs( this, pOpenGL ) ;
	if ( pImageBuf->ptrBuffer != nullptr )
	{
		SGLImageBuffer *	pOrgBuf = pImageBuf ;
		while ( pOrgBuf->ptrRefOriginal != nullptr )
		{
			pOrgBuf = pOrgBuf->ptrRefOriginal ;
		}
		pRsrc->m_flagUpdate = false ;
		//
		size_t	iLayer = 0 ;
		if ( pImageBuf->flagsBuffer
				& (flagImageTexture3D | flagImageTextureArray) )
		{
			iLayer = (pImageBuf->flagsBuffer & flagImageLayerIndexMask)
											>> flagImageLayerIndexShifter ;
		}
		else if ( pImageBuf->flagsBuffer & flagImageCubemap )
		{
			iLayer = (pImageBuf->flagsBuffer & flagImageCubeIndexMask)
											>> flagImageCubeIndexShifter ;
		}
		if ( iLayer > pOrgBuf->nFrameCount )
		{
			iLayer = 0 ;
		}
		ssize_t	nLayerOffset =
			(ssize_t) iLayer * pOrgBuf->sizeFrame.h * pOrgBuf->pitchLine ;
		//
		bool	fDualBuffer =
			(pRsrc->m_glTexture != 0) & (pRsrc->m_glRenderBuffer != 0) ;
		bool	fReadBuffer =
					!(pOrgBuf->flagsBuffer & flagImageNoReadBuffer) ;
		if ( (pOrgBuf->ptrBuffer != nullptr)
			&& (fDualBuffer | fReadBuffer) )
		{
//			ESLTrace
//				( "glReadPixels %dx%d in "
//					"SGLOpenGLTextureBuffer::ReflectBuffer.\n",
//								pOrgBuf->width, pOrgBuf->height ) ;
			GL_PIXEL_FORMAT	glpf( *pOrgBuf ) ;
			glFlush() ;
			glReadPixels
				( 0, 0, pOrgBuf->width, pOrgBuf->height,
					glpf.glFormat, glpf.glType,
					pOrgBuf->ptrBuffer + nLayerOffset ) ;
			if ( !SGLOpenGLContext::VerifyError( "glReadPixels" ) )
			{
				if ( glpf.glFormat == GL_BGRA_EXT )
				{
					Trace( "glReadPixels %dx%d with conversion BGRA format.\n",
											pOrgBuf->width, pOrgBuf->height ) ;
					SGLImageBuffer	imgbufTemp ;
					pOpenGL->GetTemporaryImageBuffer
						( imgbufTemp,
							(formatImageBGR
								| (pOrgBuf->format & ~formatImageTypeMask)),
							pOrgBuf->depth, pOrgBuf->width, pOrgBuf->height ) ;
					//
					glReadPixels
						( 0, 0, imgbufTemp.width, imgbufTemp.height,
							GL_RGBA, glpf.glType, imgbufTemp.ptrBuffer ) ;
					if ( SGLOpenGLContext::VerifyError( "glReadPixels,,,,GL_RGBA" ) )
					{
						sglConvertImageBuffer( *pOrgBuf, imgbufTemp ) ;
					}
				}
			}
			pOpenGL->m_pflog.countTransmitPixel += pOrgBuf->width * pOrgBuf->height ;
			//
			if ( glpf.IsFormatDepth() && (pOpenGL != nullptr)
				&& fReadBuffer
				&& !(pOrgBuf->format & formatImageFlagDepth) )
			{
				// z バッファで formatImageFlagDepth フラグが
				// 設定されていない場合、depth->z 変換を行う
				SGLOpenGLView *	pglView = pOpenGL->GetGLView() ;
				if ( pglView != nullptr )
				{
					pglView->ZBufferFromDepth( *pImageBuf ) ;
				}
			}
			pRsrc->m_flagUpdate = fDualBuffer ;
		}
	}
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::MakeMipmap( void )
{
	ESLAssert( m_pImageBuf != nullptr ) ;
	SGLOpenGLTextureBuffer::UpdateBuffer( m_pImageBuf, nullptr ) ;
	m_flagMipmap = true ;
	return	sglErrSuccess ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLTextureBuffer::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}

// SGLImageObject から SGLOpenGLTextureBuffer テクスチャ取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer *
	SGLOpenGLTextureBuffer::CommitGLTexture
		( SGLOpenGLContext * pOpenGL,
			SGLImageObject * pImage, SGLImageRect& rectRef )
{
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	if ( (pOpenGL != nullptr) && pOpenGL->m_flagTextureNonPowerOf2 )
	{
		pImage->NormalizeToTexture( SGLImageObject::bufferNonPowerOf2 ) ;
	}
	else
	{
		pImage->NormalizeToTexture( 0 ) ;
	}
	SGLOpenGLTextureBuffer *	pglTexture = nullptr ;
	SGLImageBufferInterface *
		pObject = pImage->CommitImageObject
					( imageObjectGLTexture, rectRef, false ) ;
	if ( pObject == nullptr )
	{
		pglTexture = new SGLOpenGLTextureBuffer ;
		pImage->AddImageObject( pglTexture, true ) ;
		pObject = pImage->CommitImageObject
						( imageObjectGLTexture, rectRef, false ) ;
	}
	pglTexture = ESLTypeCast<SGLOpenGLTextureBuffer>( pObject ) ;
//	if ( pglTexture != nullptr )
//	{
//		pglTexture->m_rectRef = rectRef ;
//	}
	return	pglTexture ;
}

SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLTextureBuffer::CommitGLTextureRsrc
		( SGLOpenGLContext * pOpenGL,
			SGLImageObject * pImage, SGLImageRect& rectRef )
{
	SGLOpenGLTextureBuffer::GLResource *	pRsrc = nullptr ;
	SGLOpenGLTextureBuffer *
		pglTexture = CommitGLTexture( pOpenGL, pImage, rectRef ) ;
	if ( pglTexture != nullptr )
	{
		pglTexture->Lock() ;
		pRsrc = pglTexture->m_pRsrc ;
		if ( (pRsrc == nullptr)
			|| (pRsrc->m_refOpenGL.GetReference() != pOpenGL) )
		{
			pRsrc = SGLOpenGLTextureBuffer::GetResourceAs( pglTexture, pOpenGL ) ;
		}
		pglTexture->Unlock() ;
		ESLAssert( pRsrc != nullptr ) ;
	}
	return	pRsrc ;
}

SGLError SGLOpenGLTextureBuffer::CommitGLTextureProgressively
	( SGLOpenGLContext * pOpenGL,
		SGLImageObject * pImage, size_t nProgressivePixelCount )
{
	if ( pImage == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( (pOpenGL != nullptr) && pOpenGL->m_flagTextureNonPowerOf2 )
	{
		pImage->NormalizeToTexture( SGLImageObject::bufferNonPowerOf2 ) ;
	}
	else
	{
		pImage->NormalizeToTexture( 0 ) ;
	}
	SGLImageBuffer *	pImageBuf = pImage->GetImageBuffer() ;
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer *			pImageRef ;
	SGLImageRect				rectRef ;
	SGLOpenGLTextureBuffer *	pglTexture =
		ESLTypeCast<SGLOpenGLTextureBuffer>
			( pImageBuf->GetImageObject
				( imageObjectGLTexture, pImageRef, rectRef, false ) ) ;
	if ( pglTexture == nullptr )
	{
		pglTexture = new SGLOpenGLTextureBuffer ;
		pImage->AddImageObject( pglTexture, true ) ;
	}
	return	pglTexture->CommitBufferProgressively
				( pOpenGL, pImageBuf, nProgressivePixelCount ) ;
}

// SGLImageObject へ OpenGL テクスチャ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::AttachGLTexture
		( SGLOpenGLContext * pOpenGL,
			SGLImageObject * pImage,
			GLuint glTexture, bool fAutoDelete )
{
	if ( pImage == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( (pOpenGL != nullptr) && pOpenGL->m_flagTextureNonPowerOf2 )
	{
		pImage->NormalizeToTexture( SGLImageObject::bufferNonPowerOf2 ) ;
	}
	else
	{
		pImage->NormalizeToTexture( 0 ) ;
	}
	SGLImageBuffer *	pImageBuf = pImage->GetImageBuffer() ;
	if ( pImageBuf == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLImageRect	rectRef ;
	SGLImageBufferInterface *
		pObject = pImage->CommitImageObject
					( imageObjectGLTexture, rectRef, false ) ;
	if ( pObject != nullptr )
	{
		return	sglErrFailed ;
	}
	SGLOpenGLTextureBuffer *	pglTexture = new SGLOpenGLTextureBuffer ;
	pglTexture->AttachGLTexture
			( pOpenGL, pImageBuf, glTexture, fAutoDelete ) ;
	pImage->AddImageObject( pglTexture, true ) ;
	return	sglErrSuccess ;
}

// RGBA フォーマット正規化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::NormalizePixelFormat
		( SGLImageBuffer * pImageBuf, bool fConvertPixelData )
{
	if ( (pImageBuf->format & formatImageTypeMask) == formatImageRGB )
	{
		QuickLock() ;
		SGLImageBuffer *	pOrgBuf = pImageBuf ;
		while ( pOrgBuf->ptrRefOriginal != nullptr )
		{
			pOrgBuf = pOrgBuf->ptrRefOriginal ;
		}
		if ( (pOrgBuf->format & formatImageTypeMask) != formatImageBGR )
		{
			if ( fConvertPixelData )
			{
				sglFlipCompositionRGBtoBGR( *pOrgBuf ) ;
			}
			SGLImageBuffer *	pNextBuf = pImageBuf ;
			while ( pNextBuf != nullptr )
			{
				pNextBuf->format = pOrgBuf->format ;
				pNextBuf = pNextBuf->ptrRefOriginal ;
			}
		}
		QuickUnlock() ;
	}
}

// テクスチャ生成
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::GLResource::CreateGLTexture( SGLImageBuffer * pImageBuf )
{
	if ( m_glTexture == 0 )
	{
		glGenTextures( 1, &m_glTexture ) ;
		if ( !SGLOpenGLContext::VerifyError( "glGenTextures(1)" ) )
		{
			m_flagUpdate = true ;
		}
		m_flagOwnTexture = true ;
	}
	if ( pImageBuf != nullptr )
	{
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		//
		// テクスチャ用バッファを VRAM 上に確保
		//
		GL_PIXEL_FORMAT	glpf( *pImageBuf ) ;
		uint32_t	width = pImageBuf->width ;
		uint32_t	height = pImageBuf->height ;
		bool		flagStorage =
						((pImageBuf->flagsBuffer & flagImageStorage) != 0)
						&& OpenGLExtension::g_supports_compute_shader ;
		m_imginf = *pImageBuf ;
		//
		static const GLenum	s_glCubeTarget[6] =
		{
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		} ;
		static const GLenum	s_glTxt2DTarget[6] =
		{
			GL_TEXTURE_2D,
		} ;
		const GLenum *	pglTargets = &s_glTxt2DTarget[0] ;
		size_t			nTargets = 1 ;
		GLenum			glParamTarget = GL_TEXTURE_2D ;
		//
		if ( glpf.IsCubemap() )
		{
			pglTargets = &s_glCubeTarget[0] ;
			glParamTarget = GL_TEXTURE_CUBE_MAP ;
			ESLAssert( (int) height / 6 == pImageBuf->sizeFrame.h ) ;
			ESLAssert( pImageBuf->nFrameCount == 6 ) ;
			width = (uint32_t) pImageBuf->sizeFrame.w ;
			height = (uint32_t) pImageBuf->sizeFrame.h ;
			nTargets = 6 ;
		}
		else if ( glpf.IsTexture2DArray() )
		{
			glParamTarget = GL_TEXTURE_2D_ARRAY ;
			width = (uint32_t) pImageBuf->sizeFrame.w ;
			height = (uint32_t) pImageBuf->sizeFrame.h ;
			nTargets = (size_t) pImageBuf->nFrameCount ;
		}
		else if ( glpf.IsTexture3D() )
		{
			glParamTarget = GL_TEXTURE_3D ;
			width = (uint32_t) pImageBuf->sizeFrame.w ;
			height = (uint32_t) pImageBuf->sizeFrame.h ;
			nTargets = (size_t) pImageBuf->nFrameCount ;
		}
		#if	!defined(__API_OPEN_GL_ES__)
		else if ( glpf.IsTextureMultisample()
			&& (pOpenGL != nullptr) && pOpenGL->m_flagMultisampling )
		{
			m_flagMultisample = true ;
			glParamTarget = GL_TEXTURE_2D_MULTISAMPLE ;
			width = (uint32_t) pImageBuf->sizeFrame.w ;
			height = (uint32_t) pImageBuf->sizeFrame.h ;
			nTargets = (size_t) pImageBuf->nFrameCount ;
		}
		#endif
		//
		glBindTexture( glParamTarget, m_glTexture ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		if ( (pOpenGL == nullptr)
			|| !(pOpenGL->m_flagTextureNonPowerOf2) )
		{
			// テクスチャは 2 の累乗サイズでなければならない
			width = sglNormalizeScalePowerBy2( width ) ;
			height = sglNormalizeScalePowerBy2( height ) ;
		}
		#if	defined(__API_OPEN_GL_ES__)
		if ( (pOpenGL == nullptr) || (pOpenGL->m_versionGL[0] < 2) )
		{
			// OpenGL ES 1.x ではテクスチャは正方形でなければならない
			// ※ OpenGL ES 2.0 以降では多分大丈夫？
			if ( width < height )
			{
				width = height ;
			}
			height = width ;
		}
		#endif
		m_flagSmoothable = !(pImageBuf->flagsBuffer & flagImageNeedSampleNoSmooth) ;
		m_flagReqTiling = (pImageBuf->flagsBuffer & flagImageNeedSampleTiling) != 0 ;
		m_imginf.width = width ;
		m_imginf.height = height ;
		m_fmtNeedsFormat = 0 ;
		m_paramDefFilter = m_flagSmoothable ? GL_LINEAR : GL_NEAREST ;
		m_paramDefWrap = m_flagReqTiling ? GL_REPEAT : GL_CLAMP_TO_EDGE ;
		//
		/*
		if ( glpf.IsFormatFloat() )
		{
			m_flagSmoothable = false ;
			m_paramDefFilter = GL_NEAREST ;
		}
		*/
		ESLAssert( width != 0 ) ;
		ESLAssert( height != 0 ) ;
		//
		#if	defined(__DEBUG__)
		if ( pImageBuf->pwszIdentity != nullptr )
		{
			ESLTrace( "create OpenGL texture as image \'%s\' on thread #%08X\n",
					SString(pImageBuf->pwszIdentity).ToCharArray().GetConstArray(),
					SThread::GetCurrentId() ) ;
		}
		#endif
		if ( (glParamTarget == GL_TEXTURE_3D)
			|| (glParamTarget == GL_TEXTURE_2D_ARRAY) )
		{
			ESLTrace( "create OpenGL texture3d "
						"#%d %dx%dx%d, %dbpp, %08X\n",
						m_glTexture, width, height, nTargets,
						pImageBuf->depth, pImageBuf->format ) ;
			if ( flagStorage )
			{
				glTexStorage3D
					( glParamTarget, 1, glpf.glInternalFormat,
						(GLsizei) width, (GLsizei) height, (GLsizei) nTargets ) ;
				SGLOpenGLContext::VerifyError( "glTexStorage3D" ) ;
			}
			else
			{
				glTexImage3D
					( glParamTarget, 0, glpf.glInternalFormat,
						(GLsizei) width, (GLsizei) height, (GLsizei) nTargets, 0,
						glpf.glFormat, glpf.glType, nullptr ) ;
			}
			if ( !SGLOpenGLContext::VerifyError(nullptr) )
			{
				if ( glpf.IsFormatRGB() )
				{
					glTexImage3D
						( glParamTarget, 0, GL_RGBA,
							(GLsizei) width, (GLsizei) height, (GLsizei) nTargets, 0,
							glpf.glFormat, glpf.glType, nullptr ) ;
					if ( !SGLOpenGLContext::VerifyError
								( "glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA)" ) )
					{
						glTexImage3D
							( glParamTarget, 0, GL_RGBA,
								(GLsizei) width, (GLsizei) height, (GLsizei) nTargets, 0,
								GL_RGBA, glpf.glType, nullptr ) ;
						SGLOpenGLContext::VerifyError
							( "glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,,,,GL_RGBA)" ) ;
						m_fmtNeedsFormat =
							formatImageBGR
								| (m_imginf.format & ~formatImageTypeMask) ;
					}
				}
				else
				{
					Trace( "failed to glTexImage3D(GL_TEXTURE_3D,0,%08X,%d,%d,0,%08X,%08X)",
						glpf.glInternalFormat, width, height, glpf.glFormat, glpf.glType ) ;
				}
			}
		}
		#if	!defined(__API_OPEN_GL_ES__)
		else if ( glParamTarget == GL_TEXTURE_2D_MULTISAMPLE )
		{
			ESLTrace( "create OpenGL texture2d multisample "
						"#%d %dx%dx%d, %dbpp, %08X\n",
						m_glTexture, width, height, nTargets,
						pImageBuf->depth, pImageBuf->format ) ;
			glTexImage2DMultisample
				( GL_TEXTURE_2D_MULTISAMPLE, (GLsizei) nTargets,
					glpf.glInternalFormat, width, height, GL_TRUE ) ;
			SGLOpenGLContext::VerifyError
				( "glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE)" ) ;
		}
		#endif
		else
		{
			ESLTrace( "create OpenGL texture2d "
						"#%d %dx%d for %dx%d, %dbpp, %08X\n",
						m_glTexture, width, height,
						pImageBuf->width, pImageBuf->height,
						pImageBuf->depth, pImageBuf->format ) ;
			//
			for ( size_t i = 0; i < nTargets; i ++ )
			{
				if ( glpf.IsFormatCompressedSource() )
				{
					glCompressedTexImage2D
						( pglTargets[i], 0, glpf.glInternalFormat,
							width, height, 0,
							pImageBuf->pitchLine * height, nullptr ) ;
					SGLOpenGLContext::VerifyError( "glCompressedTexImage2D()" ) ;
				}
				else
				{
					if ( flagStorage )
					{
						glTexStorage2D
							( pglTargets[i], 1,
								glpf.glInternalFormat, width, height ) ;
						SGLOpenGLContext::VerifyError( "glTexStorage2D" ) ;
					}
					else
					{
						glTexImage2D
							( pglTargets[i], 0, glpf.glInternalFormat,
									width, height, 0,
									glpf.glFormat, glpf.glType, nullptr ) ;
					}
					if ( !SGLOpenGLContext::VerifyError(nullptr) )
					{
						if ( glpf.IsFormatRGB() )
						{
							glTexImage2D
								( pglTargets[i], 0, GL_RGBA,
									width, height, 0,
									glpf.glFormat, glpf.glType, nullptr ) ;
							if ( !SGLOpenGLContext::VerifyError
										( "glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA)" ) )
							{
								glTexImage2D
									( pglTargets[i], 0, GL_RGBA,
										width, height, 0,
										GL_RGBA, glpf.glType, nullptr ) ;
								SGLOpenGLContext::VerifyError
									( "glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,,,,GL_RGBA)" ) ;
								m_fmtNeedsFormat =
									formatImageBGR
										| (m_imginf.format & ~formatImageTypeMask) ;
							}
						}
						else
						{
							Trace( "failed to glTexImage2D(%08X,0,%08X,%d,%d,0,%08X,%08X)",
								pglTargets[i], glpf.glInternalFormat,
								width, height, glpf.glFormat, glpf.glType ) ;
						}
					}
				}
			}
		}
		if ( glParamTarget != GL_TEXTURE_2D_MULTISAMPLE )
		{
			glTexParameteri( glParamTarget, GL_TEXTURE_MIN_FILTER, m_paramDefFilter ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
			glTexParameteri( glParamTarget, GL_TEXTURE_MAG_FILTER, m_paramDefFilter ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
			glTexParameteri( glParamTarget, GL_TEXTURE_WRAP_S, m_paramDefWrap ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
			glTexParameteri( glParamTarget, GL_TEXTURE_WRAP_T, m_paramDefWrap ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
		}
		//
		glBindTexture( glParamTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D,0)" ) ;
		//
		m_rectUpdate = pImageBuf->GetImageRect() ;
		m_flagUpdate = true ;
		m_paramTarget = glParamTarget ;
		m_nTargetCount = nTargets ;
		//
		if ( pOpenGL != nullptr )
		{
			m_nGLTexBytes = width * height * (pImageBuf->depth >> 3) * nTargets ;
			if ( glpf.IsFormatCompressedRGB() )
			{
				m_nGLTexBytes >>= 3 ;
			}
			if ( pImageBuf->flagsBuffer & flagImageMipmap )
			{
				m_nGLTexBytes <<= 1 ;
			}
			pOpenGL->AddUsedTextureBytes( m_nGLTexBytes ) ;
		}
	}
}

// テクスチャ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLTextureBuffer::GLResource::AttachGLTexture
		( SGLImageBuffer * pImageBuf, GLuint glTexture, bool fAutoDelete )
{
	ESLAssert( m_glTexture == 0 ) ;
	if ( m_glTexture != 0 )
	{
		return	sglErrFailed ;
	}
	m_flagOwnTexture = fAutoDelete ;
	m_glTexture = glTexture ;
	m_fmtNeedsFormat = 0 ;
	//
	m_flagClear = false ;
	m_flagUpdate = false ;
	m_flagMipmapped = ((pImageBuf->flagsBuffer & flagImageMipmap) != 0) ;
	m_rectUpdate = pImageBuf->GetImageRect() ;
	//
	m_imginf = *pImageBuf ;
	m_nTargetCount = 1 ;
	//
	GL_PIXEL_FORMAT	glpf( *pImageBuf ) ;
	if ( glpf.IsCubemap() )
	{
		m_nTargetCount = 6 ;
	}
	else if ( glpf.IsTexture3D() || glpf.IsTexture2DArray() )
	{
		m_nTargetCount = (size_t) pImageBuf->nFrameCount ;
	}
	m_paramTarget = glpf.glTarget ;
//
	if ( fAutoDelete )
	{
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		if ( pOpenGL != nullptr )
		{
			m_nGLTexBytes = m_imginf.width * m_imginf.height
								* (m_imginf.depth >> 3) * m_nTargetCount ;
			pOpenGL->AddUsedTextureBytes( m_nGLTexBytes ) ;
		}
	}
	return	sglErrSuccess ;
}

// テクスチャ破棄
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::GLResource::DeleteGLTexture( void )
{
	if ( m_glTexture != 0 )
	{
		if ( m_flagOwnTexture )
		{
			ESLTrace( "glDeleteTextures #%d\n", m_glTexture ) ;
			glDeleteTextures( 1, &m_glTexture ) ;
			if ( SGLOpenGLContext::VerifyError( "glDeleteTextures(1)" ) )
			{
				m_glTexture = 0 ;
			}
			SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
			if ( pOpenGL != nullptr )
			{
				ESLAssert( m_nGLTexBytes >= pOpenGL->m_bytesUsedTexture ) ;
				pOpenGL->m_bytesUsedTexture -= m_nGLTexBytes ;
			}
		}
		else
		{
			m_flagOwnTexture = false ;
			m_glTexture = 0 ;
		}
	}
}

// レンダーバッファ生成
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::GLResource::CreateGLRenderbuffer( SGLImageBuffer * pImageBuf )
{
	if ( m_glRenderBuffer != 0 )
	{
		return ;
	}
	GL_PIXEL_FORMAT	glpf( *pImageBuf ) ;
	glGenRenderbuffers( 1, &m_glRenderBuffer ) ;
	if ( SGLOpenGLContext::VerifyError( "glGenRenderbuffers(1)" ) )
	{
		glBindRenderbuffer( GL_RENDERBUFFER, m_glRenderBuffer ) ;
		SGLOpenGLContext::VerifyError( "glBindRenderbuffer(GL_RENDERBUFFER)" ) ;
		//
		bool	fSuccessed ;
		#if	defined(__PLATFORM_ANDROID__)
		if ( glpf.IsFormatDepth() )
		{
			glpf.glInternalFormat = GL_DEPTH_COMPONENT16 ;
		}
		else
		{
			glpf.glInternalFormat = GL_RGBA ;
		}
		#endif
		//
		uint32_t	width = pImageBuf->width ;
		uint32_t	height = pImageBuf->height ;
		//
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		if ( (pOpenGL == nullptr)
			|| !(pOpenGL->m_flagTextureNonPowerOf2) )
		{
			// テクスチャは 2 の累乗サイズでなければならない
			// ※テクスチャサイズと合わせるため
			width = sglNormalizeScalePowerBy2( width ) ;
			height = sglNormalizeScalePowerBy2( height ) ;
		}
		#if	defined(__API_OPEN_GL_ES__)
		if ( (pOpenGL == nullptr) || (pOpenGL->m_versionGL[0] < 2) )
		{
			// ※テクスチャサイズと合わせるため
			if ( width < height )
			{
				width = height ;
			}
			height = width ;
		}
		#endif
		//
		bool	flagMultisample =
					glpf.IsTextureMultisample()
					&& (pOpenGL != nullptr) && pOpenGL->m_flagMultisampling ;
		#if	defined(__API_OPEN_GL_ES__)
			#if	ANDROID_API_LEVEL < 18
				flagMultisample = false ;
			#endif
		#endif
		if ( flagMultisample )
		{
			m_flagMultisample = true ;
			m_nTargetCount = pImageBuf->nFrameCount ;
			//
			#if	!defined(__API_OPEN_GL_ES__) || (ANDROID_API_LEVEL >= 18)
			glRenderbufferStorageMultisample
				( GL_RENDERBUFFER,
					(GLsizei) pImageBuf->nFrameCount,
					glpf.glInternalFormat, width, height ) ;
			fSuccessed = SGLOpenGLContext::VerifyError( "glRenderbufferStorageMultisample" ) ;
			#endif
		}
		else
		{
			m_flagMultisample = false ;
			m_nTargetCount = 1 ;
			//
			glRenderbufferStorage
				( GL_RENDERBUFFER, glpf.glInternalFormat, width, height ) ;
			fSuccessed = SGLOpenGLContext::VerifyError( "glRenderbufferStorage" ) ;
			if ( !fSuccessed )
			{
				if ( glpf.IsFormatDepth() )
				{
					Trace( "try to create depth24 render-buffer.\n" ) ;
					glRenderbufferStorage
						( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
									pImageBuf->width, pImageBuf->height ) ;
					fSuccessed =
						SGLOpenGLContext::VerifyError
							( "glRenderbufferStorage(,GL_DEPTH_COMPONENT24)" ) ;
					if ( !fSuccessed )
					{
						Trace( "try to create depth16 render-buffer.\n" ) ;
						glRenderbufferStorage
							( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
										pImageBuf->width, pImageBuf->height ) ;
						fSuccessed =
							SGLOpenGLContext::VerifyError
								( "glRenderbufferStorage(,GL_DEPTH_COMPONENT16)" ) ;
					}
				}
			}
		}
		glBindRenderbuffer( GL_RENDERBUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindRenderbuffer(GL_RENDERBUFFER,0)" ) ;
		//
		if ( !fSuccessed )
		{
			glDeleteRenderbuffers( 1, &m_glRenderBuffer ) ;
			SGLOpenGLContext::VerifyError( "glDeleteRenderbuffers(1)" ) ;
			m_glRenderBuffer = 0 ;
			m_flagUpdate = true ;
		}
		else
		{
			m_imginf = *pImageBuf ;
			m_imginf.width = width ;
			m_imginf.height = height ;
		}
	}
	else
	{
		m_flagUpdate = true ;
	}
	m_flagOwnTexture = true ;
}

// デバイスの削除前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLTextureBuffer::GLResource::OnReleaseDevice( S3DRenderDevice * pDev )
{
	m_pTextureBuf->ReleaseResource( this ) ;
}


// テクスチャ破棄
//////////////////////////////////////////////////////////////////////////////

// 構築関数
SGLOpenGLTextureBuffer::TextureDestroyer::TextureDestroyer
	( SGLOpenGLContext * pOpenGL,
		GLuint glTexture, GLuint glRenderBuffer, size_t bytesUsed )
{
	m_pOpenGL = pOpenGL ;
	m_glTexture = glTexture ;
	m_glRenderBuffer = glRenderBuffer ;
	m_bytesUsed = bytesUsed ;
}

// スレッド関数
void SGLOpenGLTextureBuffer::TextureDestroyer::Run( void )
{
	if ( m_glTexture != 0 )
	{
		ESLTrace( "glDeleteTextures #%d\n", m_glTexture ) ;
		glDeleteTextures( 1, &m_glTexture ) ;
		SGLOpenGLContext::VerifyError( "glDeleteTextures(1)" ) ;
	}
	if ( m_glRenderBuffer != 0 )
	{
		ESLTrace( "glDeleteRenderbuffers #%d\n", m_glRenderBuffer ) ;
		glDeleteRenderbuffers( 1, &m_glRenderBuffer ) ;
		SGLOpenGLContext::VerifyError( "glDeleteRenderbuffers(1)" ) ;
	}
	if ( m_pOpenGL != nullptr )
	{
		ESLAssert( m_pOpenGL->m_bytesUsedTexture >= m_bytesUsed ) ;
		m_pOpenGL->m_bytesUsedTexture -= m_bytesUsed ;
	}
}

// 完了後の処理
void SGLOpenGLTextureBuffer::TextureDestroyer::Finalize( void )
{
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// フレームバッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLFrameBuffer::SGLOpenGLFrameBuffer( void )
{
	m_glFrameBuffer = 0 ;
	m_glBufTarget = GL_FRAMEBUFFER ;
	m_flagMultiSample = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLFrameBuffer::~SGLOpenGLFrameBuffer( void )
{
	if ( m_glFrameBuffer != 0 )
	{
		ReleaseFrameBuffer() ;
	}
}

// フレームバッファターゲット設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::SetFrameBufferTarget( GLenum glTarget )
{
	m_glBufTarget = glTarget ;
}

// フレームバッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLFrameBuffer::CreateFrameBuffer( void )
{
	if ( m_glFrameBuffer == 0 )
	{
		if ( !OpenGLExtension::g_supports_framebuffer_object )
		{
			return	sglErrFailed ;
		}
		glGenFramebuffers( 1, &m_glFrameBuffer ) ;
		if ( !SGLOpenGLContext::VerifyError( "glGenFramebuffers(1)" ) )
		{
			return	sglErrFailed ;
		}
		SGLOpenGLContext *
			pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		m_refOpenGL = pOpenGL ;
		//
		if ( pOpenGL != nullptr )
		{
			pOpenGL->AddNotifyObject( this ) ;
		}
	}
	return	sglErrSuccess ;
}

// フレームバッファ破棄
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::ReleaseFrameBuffer( void )
{
	if ( m_glFrameBuffer != 0 )
	{
		if ( !OpenGLExtension::g_supports_framebuffer_object )
		{
			return ;
		}
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		FrameBufferDestroyer *	pfbd =
				new FrameBufferDestroyer( m_glFrameBuffer ) ;
		m_glFrameBuffer = 0 ;
		if ( pOpenGL != nullptr )
		{
			pOpenGL->DetachNotifyObject( this ) ;
			pOpenGL->Procedure( pfbd, S3DRenderDevice::procedureDelayable ) ;
		}
		else
		{
			SGLOpenGLContext::ProcedureOnGLThread
				( pfbd, S3DRenderDevice::procedureDelayable ) ;
		}
		m_flagMultiSample = false ;
		m_refOpenGL = nullptr ;
	}
}

// フレームバッファを設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::AttachFrameBuffer
	( SGLOpenGLTextureBuffer * pglColor, SGLOpenGLTextureBuffer * pglDepth )
{
	if ( !OpenGLExtension::g_supports_framebuffer_object )
	{
		return ;
	}
	//
	// フレームバッファをバインド
	//
	if ( m_glFrameBuffer == 0 )
	{
		CreateFrameBuffer() ;
		if ( m_glFrameBuffer == 0 )
		{
			return ;
		}
	}
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	ESLAssert( pOpenGL != nullptr ) ;
	ESLAssert( pOpenGL == SGLOpenGLContext::GetCurrentGLContext() ) ;
	if ( pOpenGL == nullptr )
	{
		pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		ESLAssert( pOpenGL != nullptr ) ;
	}
	pOpenGL->AttachFrameBuffer( this ) ;
	m_flagMultiSample = false ;
	//
	SGLOpenGLTextureBuffer::GLResource *	pglrsColor =
		SGLOpenGLTextureBuffer::GetResourceAs( pglColor, pOpenGL ) ;
	SGLOpenGLTextureBuffer::GLResource *	pglrsDepth =
		SGLOpenGLTextureBuffer::GetResourceAs( pglDepth, pOpenGL ) ;
	//
	// 色バッファを設定
	//
	GLenum	txtColorTarget = GL_TEXTURE_2D ;
	GLint	iColorLayer = 0 ;
	if ( pglColor && pglColor->m_pImageBuf )
	{
		SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT
						glpf( *(pglColor->m_pImageBuf) ) ;
		txtColorTarget = glpf.glTarget ;
		iColorLayer = glpf.glLayer ;
		m_flagMultiSample |= glpf.IsTextureMultisample() ;
	}
	AttachToFrameBuffer
		( pglrsColor, GL_COLOR_ATTACHMENT0, txtColorTarget, iColorLayer ) ;
	//
	// 深度バッファを設定
	//
	GLenum	txtDepthTarget = GL_TEXTURE_2D ;
	GLint	iDepthLayer = 0 ;
	if ( pglDepth && pglDepth->m_pImageBuf )
	{
		SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT
						glpf( *(pglDepth->m_pImageBuf) ) ;
		txtDepthTarget = glpf.glTarget ;
		iDepthLayer = glpf.glLayer ;
		m_flagMultiSample |= glpf.IsTextureMultisample() ;
	}
	AttachToFrameBuffer
		( pglrsDepth, GL_DEPTH_ATTACHMENT, txtDepthTarget, iDepthLayer ) ;
	//
	// それ以外をいったん解除
	//
	const GLenum	bufsTarget[] =
	{
		GL_COLOR_ATTACHMENT0,
	} ;
	glDrawBuffers( 1, bufsTarget ) ;
	SGLOpenGLContext::VerifyError( "glDrawBuffers(1)" ) ;
	//
	DetachMultiTarget() ;
	//
	// チェック
	//
	if ( (pglrsColor != nullptr) | (pglrsDepth != nullptr) )
	do
	{
		GLenum	status = glCheckFramebufferStatus( m_glBufTarget ) ;
		if ( status == GL_FRAMEBUFFER_COMPLETE )
		{
			break ;
		}
		Trace( "glCheckFramebufferStatus() => %08X\n", status ) ;
		//
		if ( (pglrsDepth != nullptr)
			&& (pglrsDepth->m_glRenderBuffer == 0)
			&& (pglDepth->m_pImageBuf != nullptr) )
		{
			Trace( "re-create OpenGL renderbuffer for depth.\n" ) ;
//			pglrsDepth->DeleteGLTexture() ;		// ※省メモリ＆高速化のため
			pglrsDepth->CreateGLRenderbuffer( pglDepth->m_pImageBuf ) ;
			//
			AttachToFrameBuffer
				( pglrsDepth, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, iDepthLayer ) ;
			//
			status = glCheckFramebufferStatus( m_glBufTarget ) ;
			if ( status == GL_FRAMEBUFFER_COMPLETE )
			{
				break ;
			}
			Trace( "glCheckFramebufferStatus() => %08X\n", status ) ;
		}
		//
		if ( (pglrsColor != nullptr)
			&& (pglrsColor->m_glRenderBuffer == 0)
			&& (pglColor->m_pImageBuf != nullptr) )
		{
			Trace( "re-create OpenGL renderbuffer for color.\n" ) ;
			pglrsColor->CreateGLRenderbuffer( pglColor->m_pImageBuf ) ;
			//
			AttachToFrameBuffer
				( pglrsColor, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iColorLayer ) ;
			//
			status = glCheckFramebufferStatus( m_glBufTarget ) ;
			if ( status == GL_FRAMEBUFFER_COMPLETE )
			{
				break ;
			}
			Trace( "glCheckFramebufferStatus() => %08X\n", status ) ;
		}
	}
	while ( false ) ;
	//
	if ( pOpenGL->m_enabledMultiSample != m_flagMultiSample )
	{
		if ( m_flagMultiSample )
		{
			glEnable( GL_MULTISAMPLE );
			SGLOpenGLContext::VerifyError( "glEnable(GL_MULTISAMPLE)" ) ;
			pOpenGL->m_enabledMultiSample = true ;
		}
		else
		{
			glDisable( GL_MULTISAMPLE );
			SGLOpenGLContext::VerifyError( "glDisable(GL_MULTISAMPLE)" ) ;
			pOpenGL->m_enabledMultiSample = false ;
		}
	}
	//
	// バッファのクリア
	//
	GLbitfield	bfClear = 0 ;
	if ( (pglrsColor != nullptr) && pglrsColor->m_flagClear )
	{
		bfClear |= GL_COLOR_BUFFER_BIT ;
		pglrsColor->m_flagClear = false ;

		SGLPalette	argbColor( pglrsColor->m_colorClear ) ;
		const float	scaleBy255 = 1.0f / 255.0f ;
		glClearColor
			( (float) argbColor.argb.Red * scaleBy255,
				(float) argbColor.argb.Green * scaleBy255,
				(float) argbColor.argb.Blue * scaleBy255,
				(float) argbColor.argb.Alpha * scaleBy255 ) ;
		SGLOpenGLContext::VerifyError( "glClearColor" ) ;
	}
	if ( (pglrsDepth != nullptr) && pglrsDepth->m_flagClear )
	{
		glDepthMask( GL_TRUE ) ;
		SGLOpenGLContext::VerifyError( "glDepthMask(GL_TRUE)" ) ;
		glEnable( GL_DEPTH_TEST ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
		glDepthFunc( GL_LEQUAL );
		SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
		pOpenGL->m_funcDepthTest = 0 ;
		//
		bfClear |= GL_DEPTH_BUFFER_BIT ;
		pglrsDepth->m_flagClear = false ;
	}
	if ( bfClear != 0 )
	{
		glClear( bfClear ) ;
		SGLOpenGLContext::VerifyError( "glClear" ) ;
	}
}

// マルチレンダーターゲット（2つめ以降）設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::AddRenderTarget
	( SGLOpenGLTextureBuffer ** ppglColor, size_t nCount )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL == nullptr )
	{
		return ;
	}
#if	defined(__DEBUG__)
	if ( pOpenGL != SGLOpenGLContext::GetCurrentGLContext() )
	{
		ESLTrace( "not current OpenGL context.\n" ) ;
//		ESLAssert( pOpenGL == SGLOpenGLContext::GetCurrentGLContext() ) ;
		return ;
	}
	if ( pOpenGL->GetCurrentFrameBuffer() != this )
	{
		ESLTrace( "not current OpenGL frame buffer.\n" ) ;
//		ESLAssert( pOpenGL->GetCurrentFrameBuffer() == this ) ;
		return ;
	}
#endif
	//
	size_t	nLastTargets = m_aMultiTargets.GetLength() ;
	//
	m_aMultiTargets.RemoveAll() ;
	m_aMultiTargets.Add( GL_COLOR_ATTACHMENT0 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLOpenGLTextureBuffer::GLResource *	pglrsColor = nullptr ;
		if ( ppglColor[i] != nullptr )
		{
			pglrsColor =
				SGLOpenGLTextureBuffer::GetResourceAs
									( ppglColor[i], pOpenGL ) ;
		}
		GLenum	attachment = (GL_COLOR_ATTACHMENT0 + 1) + (GLenum) i ;
		if ( pglrsColor == nullptr )
		{
			AttachToFrameBuffer
				( nullptr, attachment, GL_TEXTURE_2D, 0 ) ;
			continue ;
		}
		GLenum	txtColorTarget = GL_TEXTURE_2D ;
		GLint	iColorLayer = 0 ;
		if ( ppglColor[i]->m_pImageBuf )
		{
			SGLOpenGLTextureBuffer::GL_PIXEL_FORMAT
							glpf( *(ppglColor[i]->m_pImageBuf) ) ;
			txtColorTarget = glpf.glTarget ;
			iColorLayer = glpf.glLayer ;
			ESLAssert( m_flagMultiSample == glpf.IsTextureMultisample() ) ;
		}
		AttachToFrameBuffer
			( pglrsColor, attachment, txtColorTarget, iColorLayer ) ;
		//
		m_aMultiTargets.Add( attachment ) ;
	}
	for ( size_t i = nCount; i < nLastTargets; i ++ )
	{
		GLenum	attachment = (GL_COLOR_ATTACHMENT0 + 1) + (GLenum) i ;
		AttachToFrameBuffer( nullptr, attachment, GL_TEXTURE_2D, 0 ) ;
	}
	glDrawBuffers
		( (GLsizei) m_aMultiTargets.GetLength(),
						m_aMultiTargets.GetConstArray() ) ;
	SGLOpenGLContext::VerifyError( "glDrawBuffers" ) ;
	//
	GLenum	status = glCheckFramebufferStatus( m_glBufTarget ) ;
	ESLAssert( status == GL_FRAMEBUFFER_COMPLETE ) ;
	if ( status != GL_FRAMEBUFFER_COMPLETE )
	{
		Trace( "glCheckFramebufferStatus() => %08X\n", status ) ;
	}
}

// マルチレンダーターゲット（2つめ以降）解除
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::DetachMultiTarget( void )
{
	for ( size_t i = 0; i < m_aMultiTargets.GetLength(); i ++ )
	{
		GLenum	attachment = (GL_COLOR_ATTACHMENT0 + 1) + (GLenum) i ;
		AttachToFrameBuffer( nullptr, attachment, GL_TEXTURE_2D, 0 ) ;
	}
	m_aMultiTargets.RemoveAll() ;
}

// フレームバッファを解除
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::DetachFrameBuffer( void )
{
	if ( !OpenGLExtension::g_supports_framebuffer_object
		|| !OpenGLExtension::g_supports_multiple_render_target )
	{
		return ;
	}
	if ( m_aMultiTargets.GetLength() > 0 )
	{
		const GLenum	bufsTarget[] =
		{
			GL_COLOR_ATTACHMENT0,
		} ;
		glDrawBuffers( 1, bufsTarget ) ;
		SGLOpenGLContext::VerifyError( "glDrawBuffers(1)" ) ;
		//
		DetachMultiTarget() ;
	}
	if ( m_glFrameBuffer != 0 )
	{
		AttachFrameBuffer( nullptr, nullptr ) ;
	}
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	ESLAssert( pOpenGL != nullptr ) ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->AttachFrameBuffer( nullptr ) ;
	}
}

// フレームバッファへ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::AttachToFrameBuffer
	( SGLOpenGLTextureBuffer::GLResource * pglBuffer,
			GLenum attachment, GLenum txtarget, GLint iLayer )
{
	if ( (pglBuffer != nullptr) && (pglBuffer->m_glTexture != 0) )
	{
		if ( txtarget == GL_TEXTURE_2D_ARRAY )
		{
			glFramebufferTextureLayer
				( m_glBufTarget, attachment,
					pglBuffer->m_glTexture, 0, iLayer ) ;
			SGLOpenGLContext::VerifyError( "glFramebufferTextureLayer()" ) ;
		}
		else if ( txtarget == GL_TEXTURE_3D )
		{
			glFramebufferTexture3D
				( m_glBufTarget, attachment,
					txtarget, pglBuffer->m_glTexture, 0, iLayer ) ;
			SGLOpenGLContext::VerifyError( "glFramebufferTexture3D()" ) ;
		}
		else
		{
			glFramebufferTexture2D
				( m_glBufTarget, attachment,
					txtarget, pglBuffer->m_glTexture, 0 ) ;
			SGLOpenGLContext::VerifyError( "glFramebufferTexture2D()" ) ;
		}
	}
	else if ( (pglBuffer != nullptr) && (pglBuffer->m_glRenderBuffer != 0) )
	{
		glFramebufferRenderbuffer
			( m_glBufTarget, attachment,
				GL_RENDERBUFFER, pglBuffer->m_glRenderBuffer ) ;
		SGLOpenGLContext::VerifyError
			( "glFramebufferRenderbuffer(,,GL_RENDERBUFFER)" ) ;
	}
	else
	{
		glFramebufferTexture2D
			( m_glBufTarget, attachment, txtarget, 0, 0 ) ;
		SGLOpenGLContext::VerifyError
			( "glFramebufferTexture2D(,,,0)" ) ;
		glFramebufferRenderbuffer
			( m_glBufTarget, attachment, GL_RENDERBUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError
			( "glFramebufferRenderbuffer(,,GL_RENDERBUFFER,0)" ) ;
	}
}

// フレームバッファ間転送
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::BlitFramebufferTo
	( GLuint glDstFBO, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
					GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1 )
{
#if	!defined(__API_OPEN_GL_ES__) || (defined(__PLATFORM_ANDROID__) && (ANDROID_API_LEVEL >= 18))
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	ESLAssert( pOpenGL != nullptr ) ;
	bool	fMultisample = pOpenGL ? pOpenGL->m_enabledMultiSample : false ;
	//
	glDisable( GL_MULTISAMPLE ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_MULTISAMPLE)" ) ;
	//
	glBindFramebuffer( GL_READ_FRAMEBUFFER, m_glFrameBuffer ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_READ_FRAMEBUFFER)" ) ;
	//
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, glDstFBO ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_DRAW_FRAMEBUFFER)" ) ;
	//
	glBlitFramebuffer
		( srcX0, srcY0, srcX1, srcY1,
			dstX0, dstY0, dstX1, dstY1,
			GL_COLOR_BUFFER_BIT, GL_LINEAR ) ;
	SGLOpenGLContext::VerifyError
		( "glBlitFramebuffer(,,,,,,,,GL_COLOR_BUFFER_BIT,GL_NEAREST)" ) ;
	//
	glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_READ_FRAMEBUFFER,0)" ) ;
	//
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_DRAW_FRAMEBUFFER,0)" ) ;
	//
	if ( fMultisample )
	{
		glEnable( GL_MULTISAMPLE ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_MULTISAMPLE)" ) ;
	}
	glBindFramebuffer( m_glBufTarget, m_glFrameBuffer ) ;
	SGLOpenGLContext::VerifyError( "glBindFramebuffer()" ) ;
#endif
}

// デバイスの削除前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLFrameBuffer::OnReleaseDevice( S3DRenderDevice * pDev )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL == pDev )
	{
		ReleaseFrameBuffer() ;
		pOpenGL->DetachNotifyObject( this ) ;
	}
}


// フレームバッファ破棄
//////////////////////////////////////////////////////////////////////////////

// 構築関数
SGLOpenGLFrameBuffer::FrameBufferDestroyer::FrameBufferDestroyer( GLuint glFrameBuffer )
{
	m_glFrameBuffer = glFrameBuffer ;
}

// スレッド関数
void SGLOpenGLFrameBuffer::FrameBufferDestroyer::Run( void )
{
	glDeleteRenderbuffers( 1, &m_glFrameBuffer ) ;
	SGLOpenGLContext::VerifyError( "glDeleteRenderbuffers(1)" ) ;
}

// 完了後の処理
void SGLOpenGLFrameBuffer::FrameBufferDestroyer::Finalize( void )
{
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL VertexBufferObject リソース（SGLOpenGLContext 毎）
//////////////////////////////////////////////////////////////////////////////

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::GLResource::~GLResource( void )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->DetachNotifyObject( this ) ;
	}
	if ( m_glVertexBuffer || m_glElementBuffer
		|| m_glInstancingBuffer || (m_aVAOList.GetLength() > 0) )
	{
		if ( pOpenGL != nullptr )
		{
			pOpenGL->Procedure
				( new BufferDestroyer
					( pOpenGL, this,
						m_glVertexBuffer,
						m_glElementBuffer,
						m_glInstancingBuffer, m_aVAOList ),
					S3DRenderDevice::procedureDelayable ) ;
			pOpenGL->m_bytesUsedVBO -=
						m_bytesAllocVertex
							+ m_bytesAllocElement + m_bytesAllocInstancing ;
		}
		else
		{
			SGLOpenGLContext::ProcedureOnGLThread
				( new BufferDestroyer
					( pOpenGL, this,
							m_glVertexBuffer,
							m_glElementBuffer,
							m_glInstancingBuffer, m_aVAOList ),
					S3DRenderDevice::procedureDelayable ) ;
		}
		m_glVertexBuffer = 0 ;
		m_glElementBuffer = 0 ;
		m_glInstancingBuffer = 0 ;
	}
	GLResource *	pNextRsrc = m_pNextRsrc ;
	m_pNextRsrc = nullptr ;
	delete	pNextRsrc ;
}

// バッファ確保
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLVertexBuffer::GLResource::AllocateBuffer
					( size_t bytesVertex, size_t bytesElement )
{
	if ( m_glVertexBuffer == 0 )
	{
		//
		// バッファ生成
		//
		ESLAssert( m_glElementBuffer == 0 ) ;
		GLuint	glBuffer[2] ;
		glGenBuffers( 2, &glBuffer[0] ) ;
		SGLOpenGLContext::VerifyError( "glGenBuffers(2)" ) ;
		//
		m_glVertexBuffer = glBuffer[0] ;
		m_glElementBuffer = glBuffer[1] ;
	}
	BindBuffer() ;
	//
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	bool				fRealloc = false ;
	if ( bytesVertex > m_bytesAllocVertex )
	{
		glBufferData
			( GL_ARRAY_BUFFER, bytesVertex, nullptr, GL_STREAM_DRAW ) ;
		if ( SGLOpenGLContext::VerifyError( "glBufferData(GL_ARRAY_BUFFER)" ) )
		{
			if ( pOpenGL != nullptr )
			{
				pOpenGL->m_bytesUsedVBO += bytesVertex - m_bytesAllocVertex ;
			}
			m_bytesAllocVertex = bytesVertex ;
		}
		fRealloc = true ;
	}
	if ( bytesElement > m_bytesAllocElement )
	{
		glBufferData
			( GL_ELEMENT_ARRAY_BUFFER, bytesElement, nullptr, GL_STREAM_DRAW ) ;
		if ( SGLOpenGLContext::VerifyError( "glBufferData(GL_ELEMENT_ARRAY_BUFFER)" ) )
		{
			if ( pOpenGL != nullptr )
			{
				pOpenGL->m_bytesUsedVBO += bytesElement - m_bytesAllocElement ;
			}
			m_bytesAllocElement = bytesElement ;
		}
		fRealloc = true ;
	}
	if ( pOpenGL->m_bytesUsedVBO > pOpenGL->m_bytesMaxUsedVBO )
	{
		if ( (pOpenGL->m_bytesUsedVBO >> 20)
					> (pOpenGL->m_bytesMaxUsedVBO >> 20) )
		{
			ESLTrace( "max used OpenGL VBO: %d [MB]\n",
								(int) (pOpenGL->m_bytesUsedVBO >> 20) ) ;
		}
		pOpenGL->m_bytesMaxUsedVBO = pOpenGL->m_bytesUsedVBO ;
	}
	//
	UnbindBuffer() ;
	//
	return	fRealloc ;
}

// バッファ関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::BindBuffer( void )
{
	glBindBuffer( GL_ARRAY_BUFFER, m_glVertexBuffer ) ;
	SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER)" ) ;
	//
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glElementBuffer ) ;
	SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)" ) ;
}

// バッファ分離
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::UnbindBuffer( void )
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
	//
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
}

// 頂点バッファ書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::WriteVertexBuffer
	( size_t iOffset, const void * ptrData, size_t bytesData )
{
	glBufferSubData
		( GL_ARRAY_BUFFER, iOffset, bytesData, (GLvoid*) ptrData ) ;
	SGLOpenGLContext::VerifyError( "glBufferSubData(GL_ARRAY_BUFFER)" ) ;
}

void SGLOpenGLVertexBuffer::GLResource::WriteComposedVertexBuffer
	( size_t ofsVertex, const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		size_t countVertex, size_t iSrcOffset )
{
	ESLAssert( pvVertex != nullptr ) ;
	ESLAssert( pvNormal != nullptr ) ;
	if ( m_flagInterleaved /*&& (pvUVMap != nullptr) && (pColor != nullptr)*/ )
	{
		VertexElementVNTC *	pveBuf = m_bufVertexElementVNTC.GetArray( countVertex ) ;
		size_t	i ;
		VertexElementVNTC *	pveNext = pveBuf ;
		for ( i = 0; i < countVertex; i ++ )
		{
			pveNext->vertex = pvVertex[i + iSrcOffset] ;
			pveNext ++ ;
		}
		pveNext = pveBuf ;
		for ( i = 0; i < countVertex; i ++ )
		{
			pveNext->normal = pvNormal[i + iSrcOffset] ;
			pveNext ++ ;
		}
		if ( pvUVMap != nullptr )
		{
			pveNext = pveBuf ;
			for ( i = 0; i < countVertex; i ++ )
			{
				pveNext->uv = pvUVMap[i + iSrcOffset] ;
				pveNext ++ ;
			}
		}
		if ( pColor != nullptr )
		{
			pveNext = pveBuf ;
			for ( i = 0; i < countVertex; i ++ )
			{
				pveNext->color = pColor[i + iSrcOffset] ;
				pveNext ++ ;
			}
		}
		WriteVertexBuffer
			( ofsVertex, pveBuf, countVertex * sizeof(VertexElementVNTC) ) ;
		m_bufVertexElementVNTC.FinishArray() ;
	}
	else
	{
		WriteVertexBuffer
			( ofsVertex, pvVertex, countVertex * sizeof(S3DVector4) ) ;
		WriteVertexBuffer
			( ofsVertex + countVertex * sizeof(S3DVector4),
						pvNormal, countVertex * sizeof(S3DVector4) ) ;
		if ( pvUVMap != nullptr )
		{
			WriteVertexBuffer
				( ofsVertex + countVertex
								* SGLOpenGLVertexBuffer::OFFSET_UV_MAP,
					pvUVMap, countVertex * sizeof(S2DVector) ) ;
		}
		if ( pColor != nullptr )
		{
			WriteVertexBuffer
				( ofsVertex + countVertex
								* SGLOpenGLVertexBuffer::OFFSET_COLOR_MAP,
					pColor, countVertex * sizeof(S3DColor) ) ;
		}
	}
}

// ボーンウェイトマップ書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::WriteBoneWeightBuffer
	( size_t iOffset, const float32_t * ptrData,
				size_t nVertexCount, size_t nBoneCount )
{
	float32_t *	pfpPackedMap =
					m_bufWeightTempBuf.GetArray( nVertexCount * 4 ) ;
	size_t	nPackedBones = (nBoneCount + 3) / 4 ;
	//
	for ( size_t i = 0; i < nPackedBones; i ++ )
	{
		size_t	nCompCount = nBoneCount - i * 4 ;
		if ( nCompCount >= 4 )
		{
			nCompCount = 4 ;
		}
		else
		{
			eslFillMemory
				( pfpPackedMap, 0, nVertexCount * (sizeof(float32_t)*4) ) ;
		}
		for ( size_t j = 0; j < nCompCount; j ++ )
		{
			float32_t *	pDst = pfpPackedMap + j ;
			for ( size_t k = 0; k < nVertexCount; k ++ )
			{
				*pDst = ptrData[k] ;
				pDst += 4 ;
			}
			ptrData += nVertexCount ;
		}
		WriteVertexBuffer
			( iOffset, pfpPackedMap,
					nVertexCount * (sizeof(float32_t) * 4) ) ;
		iOffset += nVertexCount * (sizeof(float32_t) * 4) ;
	}
	m_bufWeightTempBuf.FinishArray() ;
}

// 指標バッファ書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::WriteElementBuffer
	( size_t iOffset, const void * ptrData, size_t bytesData )
{
	glBufferSubData
		( GL_ELEMENT_ARRAY_BUFFER, iOffset, bytesData, (GLvoid*) ptrData ) ;
	SGLOpenGLContext::VerifyError( "glBufferSubData(GL_ELEMENT_ARRAY_BUFFER)" ) ;
}

// 指標バッファ書き込み（m_typeElementIndex に合わせて自動変換）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::WriteIndexedList
	( size_t iOffset, const uint32_t * pIndexedList, size_t nIndexCount )
{
	if ( m_typeElementIndex == GL_UNSIGNED_SHORT )
	{
		uint16_t *	pDstIndexes = m_bufElementTempBuf.GetArray( nIndexCount ) ;
		for ( size_t i = 0; i < nIndexCount; i ++ )
		{
			ESLAssert( pIndexedList[i] < 0x10000 ) ;
			pDstIndexes[i] = (uint16_t) pIndexedList[i] ;
		}
		WriteElementBuffer
			( iOffset, pDstIndexes, nIndexCount * sizeof(uint16_t) ) ;
		m_bufElementTempBuf.FinishArray() ;
	}
	else
	{
		WriteElementBuffer
			( iOffset, pIndexedList, nIndexCount * sizeof(uint32_t) ) ;
	}
}

// インスタンシングバッファ確保
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLVertexBuffer::GLResource::AllocateInstancingBuffer( size_t nCount )
{
	if ( m_glInstancingBuffer == 0 )
	{
		glGenBuffers( 1, &m_glInstancingBuffer ) ;
		SGLOpenGLContext::VerifyError( "glGenBuffers(1)" ) ;
	}
	BindInstancingBuffer() ;
	//
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	size_t	nBytes = nCount * sizeof(InstanceEntry) ;
	if ( nBytes <= m_bytesAllocInstancing )
	{
		return	false ;
	}
	glBufferData
		( GL_ARRAY_BUFFER, nBytes, nullptr, GL_STREAM_DRAW ) ;
	if ( SGLOpenGLContext::VerifyError( "glBufferData(GL_ARRAY_BUFFER)" ) )
	{
		if ( pOpenGL != nullptr )
		{
			pOpenGL->m_bytesUsedVBO += nBytes - m_glInstancingBuffer ;
		}
		m_bytesAllocInstancing = nBytes ;
	}
	for ( size_t i = 0; i < m_aVAOList.GetLength(); i ++ )
	{
		ArrayBufferList *	pabl = m_aVAOList.GetAt( i ) ;
		if ( pabl != nullptr )
		{
			ArrayBufferEntry *	pabe = pabl->GetArray() ;
			const size_t	nCount = pabl->GetLength() ;
			for ( size_t j = 0; j < nCount; j ++ )
			{
				pabe[j].m_fUpdate = true ;
			}
			pabl->FinishArray() ;
		}
	}
	return	true ;
}

// バッファ関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::BindInstancingBuffer( void )
{
	ESLAssert( m_glInstancingBuffer != 0 ) ;
	glBindBuffer( GL_ARRAY_BUFFER, m_glInstancingBuffer ) ;
	SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER)" ) ;
}

// インスタンシングバッファ書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::WriteInstancingBuffer
	( const S4DMatrix * pMatrixs, const S3DColor * pColors, size_t nCount )
{
	AllocateInstancingBuffer( nCount ) ;
	//
	InstanceEntry *	pEntries = m_bufInstancingTempBuf.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		InstanceEntry&		ie = pEntries[i] ;
		const S4DMatrix&	matSrc = pMatrixs[i] ;
		const float32_t		w = 1.0f / matSrc.m[3][3] ;
		for ( size_t j = 0; j < 3; j ++ )
		{
			ie.matrix[j][0] = matSrc.m[j][0] * w ;
			ie.matrix[j][1] = matSrc.m[j][1] * w ;
			ie.matrix[j][2] = matSrc.m[j][2] * w ;
			ie.matrix[j][3] = matSrc.m[j][3] * w ;
		}
		ie.color = pColors[i] ;
	}
	glBufferSubData
		( GL_ARRAY_BUFFER, 0,
			nCount * sizeof(InstanceEntry), (GLvoid*) pEntries ) ;
	SGLOpenGLContext::VerifyError( "glBufferSubData(GL_ARRAY_BUFFER)" ) ;
	//
	m_bufInstancingTempBuf.FinishArray() ;
}

// 頂点要素をインターリーブするか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLVertexBuffer::GLResource::IsInterleaveVertexElement
	( const S3DRenderBuffer::RENDER_ENTRY * pre ) const
{
	ESLAssert( pre != nullptr ) ;
	return	m_flagInterleaved
				&& pre->pvVertex && pre->pvNormal
				/*&& pre->pvUVMap*/ && pre->pColor ;
}

// VAO エントリ取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::ArrayBufferEntry *
	SGLOpenGLVertexBuffer::GLResource::GetArrayBufferAt
		( S3DCustomShader * pShader, size_t i, size_t nEntryCount )
{
	ArrayBufferList *	pablList = GetArrayBufferListFor( pShader ) ;
	ESLAssert( pablList != nullptr ) ;
	ESLAssert( i < nEntryCount ) ;
	ArrayBufferEntry *	pabe = pablList->GetAt( i ) ;
	if ( pabe == nullptr )
	{
		pablList->SetLength( nEntryCount ) ;
		pabe = pablList->GetAt( i ) ;
	}
	return	pabe ;
}

SGLOpenGLVertexBuffer::ArrayBufferList *
	SGLOpenGLVertexBuffer::GLResource::GetArrayBufferListFor( S3DCustomShader * pShader )
{
	ArrayBufferList **	ppABL = m_aVAOList.GetArray() ;
	size_t				nCount = m_aVAOList.GetLength() ;
	ArrayBufferList *	pablLast = nullptr ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ArrayBufferList *	pabl = ppABL[i] ;
		ESLAssert( pabl != nullptr ) ;
		ppABL[i] = pablLast ;
		pablLast = pabl ;
		//
		if ( pabl->m_pLocShader == pShader )
		{
			ppABL[0] = pabl ;
			m_aVAOList.FinishArray() ;
			return	pabl ;
		}
	}
	m_aVAOList.FinishArray() ;
	//
	if ( nCount < maxVAOShaderLRU )
	{
		if ( pablLast != nullptr )
		{
			m_aVAOList.SetAt( nCount, pablLast ) ;
		}
		ArrayBufferList *	pabl = new ArrayBufferList ;
		pabl->m_pLocShader = pShader ;
		m_aVAOList.SetAt( 0, pabl ) ;
		return	pabl ;
	}
	else
	{
		ESLAssert( pablLast != nullptr ) ;
		m_aVAOList.SetAt( 0, pablLast ) ;
		pablLast->m_pLocShader = pShader ;
		//
		ArrayBufferEntry *	pabeList = pablLast->GetArray() ;
		nCount = pablLast->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pabeList[i].m_fUpdate = true ;
		}
		pablLast->FinishArray() ;
		return	pablLast ;
	}
}

// デバイスの削除前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::GLResource::OnReleaseDevice( S3DRenderDevice * pDev )
{
	m_pVertexBuf->ReleaseResource( this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL VertexBufferObject 破棄
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::BufferDestroyer::BufferDestroyer
	( SGLOpenGLContext * pOpenGL,
		SGLOpenGLVertexBuffer::GLResource * pRsrc,
		GLuint glVertexBuffer, GLuint glElementBuffer,
		GLuint glInstancingBuffer,
		const SSystem::SObjectArray<ArrayBufferList>& lstVAO )
{
	m_pOpenGL = pOpenGL ;
	m_pRsrc = pRsrc ;
	m_glVertexBuffer = glVertexBuffer ;
	m_glElementBuffer = glElementBuffer ;
	m_glInstancingBuffer = glInstancingBuffer ;
	//
	for ( size_t i = 0; i < lstVAO.GetLength(); i ++ )
	{
		ArrayBufferList *	pablList = lstVAO.GetAt( i ) ;
		ESLAssert( pablList != nullptr ) ;
		if ( pablList != nullptr )
		{
			size_t		nLastVAOs = m_aVAOs.GetLength() ;
			size_t		nLength = pablList->GetLength() ;
			const ArrayBufferEntry *
						pabeList = pablList->GetConstArray() ;
			GLuint *	pglVAOs = m_aVAOs.GetArray( nLastVAOs + nLength ) ;
			size_t		nVAO = 0 ;
			for ( size_t j = 0; j < nLength; j ++ )
			{
				if ( pabeList[j].m_glArrayBuffer != 0 )
				{
					pglVAOs[nLastVAOs + nVAO] = pabeList[j].m_glArrayBuffer ;
					nVAO ++ ;
				}
			}
			m_aVAOs.FinishArray() ;
			m_aVAOs.SetLength( nLastVAOs + nVAO ) ;
		}
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::BufferDestroyer::Run( void )
{
	if ( m_pOpenGL != nullptr )
	{
		if ( m_pOpenGL->m_pRsrcOfVBO == m_pRsrc )
		{
			ESLTrace( "destroy current binded VBO.\n" ) ;
			glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
			SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
			//
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
			SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
			//
			m_pOpenGL->m_pBindingVBO = nullptr ;
			m_pOpenGL->m_pRsrcOfVBO = nullptr ;
		}
	}
	if ( m_glVertexBuffer != 0 )
	{
		glDeleteBuffers( 1, &m_glVertexBuffer ) ;
		SGLOpenGLContext::VerifyError( "glDeleteBuffers(1)" ) ;
	}
	if ( m_glElementBuffer != 0 )
	{
		glDeleteBuffers( 1, &m_glElementBuffer ) ;
		SGLOpenGLContext::VerifyError( "glDeleteBuffers(1)" ) ;
	}
	if ( m_glInstancingBuffer != 0 )
	{
		glDeleteBuffers( 1, &m_glInstancingBuffer ) ;
		SGLOpenGLContext::VerifyError( "glDeleteBuffers(1)" ) ;
	}
	if ( m_aVAOs.GetLength() > 0 )
	{
		glDeleteVertexArrays
			( (GLsizei) m_aVAOs.GetLength(), m_aVAOs.GetConstArray() ) ;
		SGLOpenGLContext::VerifyError( "glDeleteVertexArrays()" ) ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::BufferDestroyer::Finalize( void )
{
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL VertexBufferObject
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLVertexBuffer, S3DVertexDeviceBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::SGLOpenGLVertexBuffer( S3DRenderBuffer * prb )
{
	m_pFirstGLRsrc = nullptr ;
	m_pRenderBuf = prb ;
	//
	m_bytesVertex = 0 ;
	m_bytesElement = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::~SGLOpenGLVertexBuffer( void )
{
	delete	m_pFirstGLRsrc ;
	m_pFirstGLRsrc = nullptr ;
}

// VBO を OpenGL 用にコミット
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer *
	SGLOpenGLVertexBuffer::Commit( S3DVertexBufferInterface * pVB )
{
	SGLOpenGLVertexBuffer *
			pglVB = pVB->GetDeviceBuffer<SGLOpenGLVertexBuffer>() ;
	if ( pglVB == nullptr )
	{
		S3DRenderBuffer *	prb = ESLTypeCast<S3DRenderBuffer>( pVB ) ;
		if ( prb == nullptr )
		{
			return	nullptr ;
		}
		pglVB = new SGLOpenGLVertexBuffer( prb ) ;
		pglVB->OnFlush( prb ) ;
		prb->AttachDeviceBuffer( pglVB ) ;
	}
	return	pglVB ;
}

// バッファ更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::NotifyUpdateBuffer
	( size_t iMeshFirst, size_t iMeshEnd, uint32_t maskUpdateElements )
{
	GLResource *	pRsrc ;
	m_csSync.Lock() ;
	pRsrc = m_pFirstGLRsrc ;
	while ( pRsrc != nullptr )
	{
		if ( pRsrc->m_maskUpdateElements )
		{
			pRsrc->m_maskUpdateElements |= maskUpdateElements ;
			//
			if ( iMeshFirst < pRsrc->m_iFirstUpdate )
			{
				pRsrc->m_iFirstUpdate = iMeshFirst ;
			}
			if ( iMeshEnd > pRsrc->m_iEndUpdate )
			{
				pRsrc->m_iEndUpdate = iMeshEnd ;
			}
		}
		else
		{
			pRsrc->m_maskUpdateElements = maskUpdateElements ;
			pRsrc->m_iFirstUpdate = iMeshFirst ;
			pRsrc->m_iEndUpdate = iMeshEnd ;
		}
		pRsrc = pRsrc->m_pNextRsrc ;
	}
	m_csSync.Unlock() ;
}

// GLResource 取得（バッファ更新確定）
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::GLResource *
	SGLOpenGLVertexBuffer::CommitResourceAs( SGLOpenGLContext * pOpenGL )
{
	ESLAssert( m_pRenderBuf != nullptr ) ;
	m_pRenderBuf->Flush() ;
	//
	GLResource *	pRsrc ;
	m_csSync.Lock() ;
	pRsrc = GetResourceAs( this, pOpenGL ) ;
	if ( (pRsrc != nullptr)
		&& (pRsrc->m_maskUpdateElements != 0) )
	{
		size_t		iFirst = pRsrc->m_iFirstUpdate ;
		size_t		iEnd = pRsrc->m_iEndUpdate ;
		uint32_t	maskElements = pRsrc->m_maskUpdateElements ;
		//
		ESLAssert( pOpenGL->IsOnRenderThread() ) ;
		pRsrc->m_maskUpdateElements = 0 ;
		m_csSync.Unlock() ;
		//
		if ( pRsrc->AllocateBuffer( m_bytesVertex, m_bytesElement ) )
		{
			iFirst = 0 ;
			iEnd = m_arrEntryInfo.GetLength() ;
		}
		pRsrc->BindBuffer() ;
		//
		const ENTRY_INFO *	peiNext = m_arrEntryInfo.GetConstArray() ;
		peiNext += iFirst ;
		for ( size_t i = iFirst; i < iEnd; i ++, peiNext ++ )
		{
			RENDER_ENTRY *	pre = peiNext->pre ;
			ESLAssert( pre != nullptr ) ;
			//
			SGLImageObject *	pVTex = m_arrVertexTexture.GetAt(i) ;
			CommitVertexBuffer
				( pRsrc, pVTex, *peiNext, pre, maskElements ) ;
			//
			if ( peiNext->flagVertexTexture && (pVTex != nullptr) )
			{
				pOpenGL->CommitDeviceImage( pVTex, 0 ) ;
			}
			//
			for ( size_t j = 0; j < pRsrc->m_aVAOList.GetLength(); j ++ )
			{
				ArrayBufferList *	pablList = pRsrc->m_aVAOList.GetAt( j ) ;
				ESLAssert( pablList != nullptr ) ;
				ArrayBufferEntry *	pabe = pablList->GetAt( i ) ;
				if ( pabe != nullptr )
				{
					pabe->m_fUpdate = true ;
				}
			}
		}
		pRsrc->UnbindBuffer() ;
		return	pRsrc ;
	}
	m_csSync.Unlock() ;
	return	pRsrc ;
}

void SGLOpenGLVertexBuffer::CommitVertexBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc,
		SGLImageObject * pVTex,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		S3DRenderBuffer::RENDER_ENTRY * pre,
		uint32_t maskUpdateElements )
{
	size_t	ofsVertex = ei.ofsVertex ;
	size_t	ofsElement = ei.ofsElement ;
	size_t	countIndex = pre->countIndex ;
	size_t	countVertex = pre->countVertex ;
	//
	SGLOpenGLContext *	pOpenGL = pRsrc->m_refOpenGL ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->m_pflog.countTransmitVertex += countVertex ;
	}
	//
	if ( pRsrc->IsInterleaveVertexElement( pre ) )
	{
		if ( maskUpdateElements
			& (elementVertex | elementNormal | elementUVMap | elementColor) )
		{
			pRsrc->WriteComposedVertexBuffer
				( ofsVertex, pre->pvVertex,
					pre->pvNormal, pre->pvUVMap, pre->pColor, countVertex ) ;
		}
		ofsVertex += countVertex * sizeof(VertexElementVNTC) ;
	}
	else
	{
		if ( pre->pvVertex != nullptr )
		{
			// 頂点
			if ( maskUpdateElements & elementVertex )
			{
				pRsrc->WriteVertexBuffer
					( ofsVertex, pre->pvVertex,
								countVertex * sizeof(S3DVector4) ) ;
			}
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
		if ( pre->pvNormal != nullptr )
		{
			// 法線
			if ( maskUpdateElements & elementNormal )
			{
				pRsrc->WriteVertexBuffer
					( ofsVertex, pre->pvNormal,
								countVertex * sizeof(S3DVector4) ) ;
			}
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
		if ( pre->pvUVMap != nullptr )
		{
			// UV マップ
			if ( maskUpdateElements & elementUVMap )
			{
				pRsrc->WriteVertexBuffer
					( ofsVertex, pre->pvUVMap,
								countVertex * sizeof(S2DVector) ) ;
			}
			ofsVertex += countVertex * sizeof(S2DVector) ;
		}
		if ( pre->pColor != nullptr )
		{
			// 頂点色
			if ( maskUpdateElements & elementColor )
			{
				pRsrc->WriteVertexBuffer
					( ofsVertex, pre->pColor,
								countVertex * sizeof(S3DColor) ) ;
			}
			ofsVertex += countVertex * sizeof(S3DColor) ;
		}
	}
	if ( pre->pvTexAxisX != nullptr )
	{
		// 表面ｘ基底
		if ( maskUpdateElements & (elementVertex | elementUVMap) )
		{
			pRsrc->WriteVertexBuffer
				( ofsVertex, pre->pvTexAxisX,
							countVertex * sizeof(S3DVector4) ) ;
		}
		ofsVertex += countVertex * sizeof(S3DVector4) ;
	}
	if ( pre->pvTexAxisY != nullptr )
	{
		// 表面ｙ基底
		if ( maskUpdateElements & (elementVertex | elementUVMap) )
		{
			pRsrc->WriteVertexBuffer
				( ofsVertex, pre->pvTexAxisY,
							countVertex * sizeof(S3DVector4) ) ;
		}
		ofsVertex += countVertex * sizeof(S3DVector4) ;
	}
	if ( ei.flagVertexTexture && (pVTex != nullptr) )
	{
		ESLAssert( pVTex->GetImageSize() == ei.sizeVertexTex ) ;
		if ( maskUpdateElements
				& (elementMorphing | elementWeightMap | elementExAttrElements) )
		{
			SGLImageInfo	imginf ;
			SGLImageRect	rect
				( 0, 0, ei.sizeVertexTex.w, (int) ei.yVTMorphInstance ) ;
			uint8_t *		pbytBuf =
				pVTex->LockBuffer( imginf, SGLImageObject::lockWrite, &rect ) ;
			//
			CommitVertexTextureBuffer
				( pRsrc, imginf, pbytBuf, ei, pre, maskUpdateElements ) ;
			//
			pVTex->UnlockBuffer( SGLImageObject::lockWrite ) ;
		}
	}
	else if ( !ei.flagMustShapeByCPU )
	{
		if ( (pre->countMorph != 0) && (ei.ofsMorph != 0) )
		{
			// モーフターゲット
			for ( size_t j = 0; j < pre->countMorph; j ++ )
			{
				size_t	iOffset = j * countVertex ;
				size_t	ofsMorph =
					ei.ofsMorph + iOffset * SGLOpenGLVertexBuffer::SIZEOF_ELEMENT ;
				if ( (pre->pvMorphVertex != nullptr)
					&& (maskUpdateElements & elementMorphVertex) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pvMorphVertex + iOffset,
							countVertex * sizeof(S3DVector4) ) ;
				}
				ofsMorph += countVertex * sizeof(S3DVector4) ;
				//
				if ( (pre->pvMorphNormal != nullptr)
					&& (maskUpdateElements & elementMorphNormal) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pvMorphNormal + iOffset,
							countVertex * sizeof(S3DVector4) ) ;
				}
				ofsMorph += countVertex * sizeof(S3DVector4) ;
				//
				if ( (pre->pvMorphUVMap != nullptr)
					&& (maskUpdateElements & elementMorphUVMap) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pvMorphUVMap + iOffset,
							countVertex * sizeof(S2DVector) ) ;
				}
				ofsMorph += countVertex * sizeof(S2DVector) ;
				//
				if ( (pre->pMorphColor != nullptr)
					&& (maskUpdateElements & elementMorphColor) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pMorphColor + iOffset,
							countVertex * sizeof(S3DColor) ) ;
				}
				ofsMorph += countVertex * sizeof(S3DColor) ;
				//
				if ( (pre->pvMorphTexAxisX != nullptr)
					&& (maskUpdateElements
							& (elementMorphVertex | elementMorphUVMap)) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pvMorphTexAxisX + iOffset,
							countVertex * sizeof(S3DVector4) ) ;
				}
				ofsMorph += countVertex * sizeof(S3DVector4) ;
				//
				if ( (pre->pvMorphTexAxisY != nullptr)
					&& (maskUpdateElements
							& (elementMorphVertex | elementMorphUVMap)) )
				{
					pRsrc->WriteVertexBuffer
						( ofsMorph, pre->pvMorphTexAxisY + iOffset,
							countVertex * sizeof(S3DVector4) ) ;
				}
				ofsMorph += countVertex * sizeof(S3DVector4) ;
			}
		}
		if ( (pre->countBone != 0) && (ei.ofsBone != 0)
			&& (maskUpdateElements & elementWeightMap) )
		{
			// ボーン・ウェイトマップ
			pRsrc->WriteBoneWeightBuffer
				( ei.ofsBone,
					pre->ppWeightMap[0], countVertex, pre->countWeightMap ) ;
		}
	}
	if ( pre->pIndexedList != nullptr )
	{
		// 指標
		if ( maskUpdateElements & elementIndex )
		{
			pRsrc->WriteIndexedList
				( ofsElement, pre->pIndexedList, countIndex ) ;
		}
		ofsElement += countIndex * sizeof(uint32_t) ;
	}
	if ( maskUpdateElements & elementSubIndex )
	{
		for ( int j = 0; j < S3DRenderBuffer::countSubMesh; j ++ )
		{
			if ( (ei.ofsSubElements[j] == 0)
				|| (pre->pSubIndexedList[j] == nullptr) )
			{
				continue ;
			}
			pRsrc->WriteIndexedList
				( ei.ofsSubElements[j],
					pre->pSubIndexedList[j],
					pre->nSubIndexCount[j] ) ;
		}
	}
}

void SGLOpenGLVertexBuffer::CommitVertexTextureBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc,
		const SGLImageInfo& imginf, uint8_t * pbytBuf,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		S3DRenderBuffer::RENDER_ENTRY * pre,
		uint32_t maskUpdateElements )
{
	const size_t	countVertex = pre->countVertex ;
	if ( pre->countMorph > 0 )
	{
		// モーフターゲット
		size_t	yVertex = ei.yVTMorphVertex ;
		size_t	yNormal = ei.yVTMorphNormal ;
		for ( size_t j = 0; j < pre->countMorph; j ++ )
		{
			size_t	iOffset = j * countVertex ;
			if ( pre->flagMorphWithWeight )
			{
				if ( maskUpdateElements & elementMorphVertex )
				{
					WriteMorphVertexBuffer
						( imginf, pbytBuf, yVertex,
							pre->pvMorphVertex + iOffset,
							pre->pfpMorphWeight + iOffset,
							pre->pvVertex, countVertex ) ;
				}
				if ( maskUpdateElements & elementMorphNormal )
				{
					WriteMorphVertexBuffer
						( imginf, pbytBuf, yNormal,
							pre->pvMorphNormal + iOffset,
							pre->pfpMorphWeight + iOffset,
							pre->pvNormal, countVertex ) ;
				}
			}
			else
			{
				if ( maskUpdateElements & elementMorphVertex )
				{
					WriteMorphVertexBuffer
						( imginf, pbytBuf, yVertex,
							pre->pvMorphVertex + iOffset,
							nullptr,
							pre->pvVertex, countVertex ) ;
				}
				if ( maskUpdateElements & elementMorphNormal )
				{
					WriteMorphVertexBuffer
						( imginf, pbytBuf, yNormal,
							pre->pvMorphNormal + iOffset,
							nullptr,
							pre->pvNormal, countVertex ) ;
				}
			}
			yVertex += ei.yVTMorphStride ;
			yNormal += ei.yVTMorphStride ;
		}
	}
	if ( (pre->countBone > 0)
		&& (maskUpdateElements & elementWeightMap) )
	{
		// ボーン・ウェイトマップ
		if ( pre->ppJointMap != nullptr )
		{
			WriteWeightMapBuffer
				( imginf, pbytBuf, ei.yVTBoneWeight,
					pre->ppWeightMap,
					pre->countWeightMap, countVertex ) ;
			WriteJointMapBuffer
				( imginf, pbytBuf, ei.yVTBoneIndex,
					pre->ppJointMap,
					pre->countWeightMap, countVertex ) ;
		}
		else
		{
			WriteWeightMapBuffer
				( imginf, pbytBuf, ei.yVTBoneWeight,
					pre->ppWeightMap,
					pre->countBone, countVertex ) ;
			WriteJointMapBuffer
				( imginf, pbytBuf, ei.yVTBoneIndex,
					nullptr, 0, countVertex ) ;
		}
	}
	if ( (pre->nExAttrElements > 0)
		&& (maskUpdateElements & elementExAttrElements) )
	{
		ESLAssert( pre->pfpExAttrElements != nullptr ) ;
		WriteExAttributeElementsBuffer
			( imginf, pbytBuf, ei.yVTExAttrElements,
				pre->pfpExAttrElements,
				pre->nExAttrElements, countVertex ) ;
	}
}

void SGLOpenGLVertexBuffer::WriteMorphVertexBuffer
	( const SGLImageInfo& imginf, uint8_t * pbytBuf,
		size_t yLine, const S3DVector4 * pvMorph,
		const float32_t * pfpWeight,
		const S3DVector4 * pvOrgVertex, size_t nCount )
{
	ESLAssert( imginf.depth == 32 * 4 ) ;
	ESLAssert( imginf.pitchLine == imginf.width * 16 ) ;
	S3DVector4 *	pvDst = (S3DVector4*) (pbytBuf + yLine * imginf.pitchLine) ;
	if ( pfpWeight != nullptr )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pvDst[i] = (pvMorph[i] - pvOrgVertex[i]) * pfpWeight[i] ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pvDst[i] = pvMorph[i] - pvOrgVertex[i] ;
		}
	}
}

void SGLOpenGLVertexBuffer::WriteWeightMapBuffer
	( const SGLImageInfo& imginf, uint8_t * pbytBuf,
		size_t yLine, float32_t *const* ppWeight,
		size_t nWeightCount, size_t nVertexCount )
{
	ESLAssert( imginf.depth == 32 * 4 ) ;
	ESLAssert( imginf.pitchLine == imginf.width * 16 ) ;
	float32_t *	pDstLine =
					(float32_t*) (pbytBuf + imginf.pitchLine * yLine) ;
	for ( size_t i = 0; (i < nWeightCount) && (i < 4); i ++ )
	{
		const float32_t *	pWeight = ppWeight[i] ;
		float32_t *			pDst = pDstLine + i ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			*pDst = *pWeight ;
			pWeight ++ ;
			pDst += 4 ;
		}
	}
	for ( size_t i = nWeightCount; i < 4; i ++ )
	{
		float32_t *	pDst = pDstLine + i ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			*pDst = 0 ;
			pDst += 4 ;
		}
	}
}

void SGLOpenGLVertexBuffer::WriteJointMapBuffer
	( const SGLImageInfo& imginf, uint8_t * pbytBuf,
		size_t yLine, uint32_t *const* ppJoint,
		size_t nWeightCount, size_t nVertexCount )
{
	ESLAssert( imginf.depth == 32 * 4 ) ;
	ESLAssert( imginf.pitchLine == imginf.width * 16 ) ;
	float32_t *	pDstLine =
					(float32_t*) (pbytBuf + imginf.pitchLine * yLine) ;
	for ( size_t i = 0; (i < nWeightCount) && (i < 4); i ++ )
	{
		const uint32_t *	pJoint = ppJoint[i] ;
		float32_t *			pDst = pDstLine + i ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			*pDst = (float32_t) *pJoint ;
			pJoint ++ ;
			pDst += 4 ;
		}
	}
	for ( size_t i = nWeightCount; i < 4; i ++ )
	{
		float32_t *	pDst = pDstLine + i ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			*pDst = (float32_t) i ;
			pDst += 4 ;
		}
	}
}

void SGLOpenGLVertexBuffer::WriteExAttributeElementsBuffer
	( const SGLImageInfo& imginf, uint8_t * pbytBuf,
		size_t yLine, const float32_t * pfpElements,
		size_t nElementsCount, size_t nVertexCount )
{
	ESLAssert( imginf.depth == 32 * 4 ) ;
	ESLAssert( imginf.pitchLine == imginf.width * 16 ) ;
	float32_t *		pDstLine =
						(float32_t*) (pbytBuf + imginf.pitchLine * yLine) ;
	const size_t	nExAttrVElements = (nElementsCount + 3) / 4 ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		for ( size_t j = 0; j < nElementsCount; j ++ )
		{
			pDstLine[j] = pfpElements[j] ;
		}
		pDstLine += nExAttrVElements * 4 ;
		pfpElements += nElementsCount ;
	}
}

// GLResource 取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::GLResource *
	SGLOpenGLVertexBuffer::GetResourceAs
		( SGLOpenGLVertexBuffer * pVertBuf, SGLOpenGLContext * pOpenGL )
{
	GLResource *	pRsrc = nullptr ;
	if ( (pVertBuf != nullptr) & (pOpenGL != nullptr) )
	{
		ESLAssert( pVertBuf->m_pRenderBuf != nullptr ) ;
		pVertBuf->m_csSync.Lock() ;
		GLResource *	pLastRsrc = nullptr ;
		pRsrc = pVertBuf->m_pFirstGLRsrc ;
		while ( pRsrc != nullptr )
		{
			if ( pRsrc->m_refOpenGL.GetReference() == pOpenGL )
			{
				break ;
			}
			pLastRsrc = pRsrc ;
			pRsrc = pRsrc->m_pNextRsrc ;
		}
		if ( pRsrc == nullptr )
		{
			pRsrc = new GLResource( pVertBuf ) ;
			if ( pLastRsrc != nullptr )
			{
				pLastRsrc->m_pNextRsrc = pRsrc ;
			}
			else
			{
				pVertBuf->m_pFirstGLRsrc = pRsrc ;
			}
			pRsrc->m_refOpenGL = pOpenGL ;
			pRsrc->m_flagInterleaved =
				pOpenGL->m_flagAvailableMultiSVB
					&& !(pVertBuf->m_pRenderBuf->GetBufferControlFlags()
							& S3DVertexBufferInterface::bufferDynamicVertex) ;
			pRsrc->m_maskUpdateElements = elementAll ;
			pRsrc->m_iFirstUpdate = 0 ;
			pRsrc->m_iEndUpdate = pVertBuf->m_pRenderBuf->GetMeshCount() ;
			//
			if ( !(pOpenGL->m_flagElementIndexUint) )
			{
				pRsrc->m_typeElementIndex = GL_UNSIGNED_SHORT ;
			}
			//
			pOpenGL->AddNotifyObject( pRsrc ) ;
		}
		pVertBuf->m_csSync.Unlock() ;
	}
	return	pRsrc ;
}

// GLResource 削除
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::ReleaseResource
		( SGLOpenGLVertexBuffer::GLResource * pRsrc )
{
	if ( m_pFirstGLRsrc != nullptr )
	{
		m_csSync.Lock() ;
		if ( m_pFirstGLRsrc == pRsrc )
		{
			m_pFirstGLRsrc = pRsrc->m_pNextRsrc ;
		}
		else
		{
			GLResource *	pLastRsrc = m_pFirstGLRsrc ;
			GLResource *	pNextRsrc = pLastRsrc->m_pNextRsrc ;
			while ( pNextRsrc != nullptr )
			{
				if ( pNextRsrc == pRsrc )
				{
					pLastRsrc->m_pNextRsrc = pRsrc->m_pNextRsrc ;
					break ;
				}
				pLastRsrc = pNextRsrc ;
				pNextRsrc = pNextRsrc->m_pNextRsrc ;
			}
		}
		pRsrc->m_pNextRsrc = nullptr ;
		m_csSync.Unlock() ;
	}
	delete	pRsrc ;
}

// エントリ情報取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexBuffer::ENTRY_INFO *
	SGLOpenGLVertexBuffer::GetEntryInfoAt( size_t iMesh )
{
	ESLAssert( m_pRenderBuf != nullptr ) ;
	if ( m_pRenderBuf->GetMeshCount() > m_arrEntryInfo.GetLength() )
	{
		m_pRenderBuf->Flush() ;
	}
	return	m_arrEntryInfo.GetAt( iMesh ) ;
}

SGLImageObject * SGLOpenGLVertexBuffer::GetVertexTextureAt( size_t iMesh )
{
	ESLAssert( m_pRenderBuf != nullptr ) ;
	if ( m_pRenderBuf->GetMeshCount() > m_arrEntryInfo.GetLength() )
	{
		m_pRenderBuf->Flush() ;
	}
	return	m_arrVertexTexture.GetAt( iMesh ) ;
}

// モーフィング・インスタンス書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::WriteMorphInstanceToVertexTexture
	( SGLImageObject * pImage,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		const S3DVector4 * pvMorphInstance, size_t nCount )
{
	ESLAssert( nCount <= ei.nMaxInstance ) ;
	if ( nCount > ei.nMaxInstance )
	{
		nCount = ei.nMaxInstance ;
	}
	size_t	wInstance = ei.sizeVertexTex.w / 2 ;
	size_t	hLines = (nCount + wInstance - 1) / wInstance ;
	//
	SGLImageRect	rect( 0, (int) ei.yVTMorphInstance,
							ei.sizeVertexTex.w, (int) hLines ) ;
	SGLImageInfo	imginf ;
	S3DVector4 *	pvBuf =
		(S3DVector4*) pImage->LockBuffer
						( imginf, SGLImageObject::lockWrite, &rect ) ;
	//
	eslCopyMemory( pvBuf, pvMorphInstance, nCount * sizeof(S3DVector4) * 2 ) ;
	//
	pImage->UnlockBuffer( SGLImageObject::lockWrite ) ;
}

void SGLOpenGLVertexBuffer::MakeMorphInstance
	( S3DVector4 * pvMorphInstance,
		S3DVertexVariantBuffer * pvvb, size_t iMesh )
{
	S3DVector4	vWeight( 0, 0, 0, 0 ) ;
	S3DVector4	vIndex( 0, 0, 0, 0 ) ;
	//
	ssize_t		iTargetMesh ;
	float32_t	fpApplication ;
	size_t		iMorphIndex = 0 ;
	//
	while ( !pvvb->GetMorphingApplication
				( iMesh, iTargetMesh, fpApplication, iMorphIndex ++ ) )
	{
		if ( iTargetMesh < 0 )
		{
			continue ;
		}
		if ( fpApplication > vWeight.x )
		{
			if ( vWeight.d > 1.0e-5f )
			{
				ESLTrace( "Caution: too many morphing targets.\n" ) ;
			}
			vWeight.d = vWeight.z ;
			vWeight.z = vWeight.y ;
			vWeight.y = vWeight.x ;
			vWeight.x = fpApplication ;
			//
			vIndex.d = vIndex.z ;
			vIndex.z = vIndex.y ;
			vIndex.y = vIndex.x ;
			vIndex.x = (float32_t) iTargetMesh ;
		}
		else if ( fpApplication > vWeight.y )
		{
			vWeight.d = vWeight.z ;
			vWeight.z = vWeight.y ;
			vWeight.y = fpApplication ;
			//
			vIndex.d = vIndex.z ;
			vIndex.z = vIndex.y ;
			vIndex.y = (float32_t) iTargetMesh ;
		}
		else if ( fpApplication > vWeight.z )
		{
			vWeight.d = vWeight.z ;
			vWeight.z = fpApplication ;
			//
			vIndex.d = vIndex.z ;
			vIndex.z = (float32_t) iTargetMesh ;
		}
		else if ( fpApplication > vWeight.d )
		{
			vWeight.d = fpApplication ;
			vIndex.d = (float32_t) iTargetMesh ;
		}
	}
	//
	pvMorphInstance[0] = vWeight ;
	pvMorphInstance[1] = vIndex ;
}

// ボーンインスタンスを頂点テクスチャに書き込む
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::WriteBoneInstanceToVertexTexture
	( SGLImageObject * pImage, size_t iMesh,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		S3DMatrix * pMatrixBuf, S3DVector * pvTransBuf,
		S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount )
{
	if ( nCount == 0 )
	{
		return ;
	}
	SGLImageRect	rect( 0, (int) ei.yVTBoneInstance,
							ei.sizeVertexTex.w, (int) nCount ) ;
	SGLImageInfo	imginf ;
	uint8_t *		pbytBuf = pImage->LockBuffer
								( imginf, SGLImageObject::lockWrite, &rect ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DVertexVariantBuffer *	pvvb = ppInstancingVVB[i] ;
		ESLAssert( pvvb != nullptr ) ;
		if ( pvvb == nullptr )
		{
			continue ;
		}
		size_t	nBoneCount = pvvb->GetBoneMatrix( iMesh, 0, nullptr, nullptr ) ;
		if ( nBoneCount > (size_t) ei.sizeVertexTex.w )
		{
			nBoneCount = (size_t) ei.sizeVertexTex.w ;
		}
		nBoneCount = pvvb->GetBoneMatrix
						( iMesh, nBoneCount, pMatrixBuf, pvTransBuf ) ;
		//
		S4DVector *	pvBuf = (S4DVector*) (pbytBuf + imginf.pitchLine * i) ;
		S3DMatrix *	pNextMatrix = pMatrixBuf ;
		S3DVector * pvNextTrans = pvTransBuf ;
		//
		for ( size_t j = 0; j < nBoneCount; j ++ )
		{
			pvBuf->x = pNextMatrix->m[0][0] ;
			pvBuf->y = pNextMatrix->m[0][1] ;
			pvBuf->z = pNextMatrix->m[0][2] ;
			pvBuf->w = pvNextTrans->x ;
			pvBuf ++ ;
			//
			pvBuf->x = pNextMatrix->m[1][0] ;
			pvBuf->y = pNextMatrix->m[1][1] ;
			pvBuf->z = pNextMatrix->m[1][2] ;
			pvBuf->w = pvNextTrans->y ;
			pvBuf ++ ;
			//
			pvBuf->x = pNextMatrix->m[2][0] ;
			pvBuf->y = pNextMatrix->m[2][1] ;
			pvBuf->z = pNextMatrix->m[2][2] ;
			pvBuf->w = pvNextTrans->z ;
			pvBuf ++ ;
			//
			pNextMatrix ++ ;
			pvNextTrans ++ ;
		}
	}
	//
	pImage->UnlockBuffer( SGLImageObject::lockWrite ) ;
}

void SGLOpenGLVertexBuffer::AllocVertexTextureSize
	( SGLOpenGLVertexBuffer::ENTRY_INFO& ei, RENDER_ENTRY * pre )
{
	size_t	nBoneVCount = pre->countVertex * 2 ;
	size_t	nMorphVCount = pre->countVertex * 2 * pre->countMorph ;
	size_t	nExAttrVElements = (pre->nExAttrElements + 3) / 4 ;
	size_t	nExAttrVCount = pre->countVertex * nExAttrVElements ;
	size_t	wVSqrt = (size_t) sqrt( (double) (nBoneVCount + nMorphVCount + nExAttrVCount) ) ;
	size_t	wBoneMatrix = pre->countBone * 3 ;
	size_t	wTex = sglNormalizeScalePowerBy2
				( (uint32_t) esl_max( (int) wVSqrt, (int) wBoneMatrix ) ) ;
	wTex = (size_t) esl_clampi( (int) wTex, 4, 2048 ) ;
	//
	size_t	yVertexStride = (pre->countVertex + wTex - 1) / wTex ;
	size_t	yExAttrStride = (nExAttrVCount + wTex - 1) / wTex ;
	//
	ei.sizeVertexTex.w = (int32_t) wTex ;
	ei.yVTMorphVertex = 0 ;
	ei.yVTMorphNormal = yVertexStride * pre->countMorph ;
	ei.yVTMorphStride = yVertexStride ;
	ei.yVTBoneWeight = ei.yVTMorphNormal + yVertexStride * pre->countMorph ;
	ei.yVTBoneIndex = ei.yVTBoneWeight + yVertexStride ;
	ei.yVTExAttrElements = ei.yVTBoneIndex + yVertexStride ;
	ei.xVTExAttrStride = nExAttrVElements ;
	ei.yVTMorphInstance = ei.yVTExAttrElements + yExAttrStride + 1 ;
	//
	size_t	wMorphInstance = wTex / 2 ;
	if ( pre->countBone > 0 )
	{
		size_t	hTex = sglNormalizeScalePowerBy2( (uint32_t) (ei.yVTMorphInstance + 17) ) ;
		ei.sizeVertexTex.h = (int32_t) hTex ;
		ei.yVTBoneInstance = ei.yVTMorphInstance + 1 ;
		//
		size_t	nMorphMax = wMorphInstance ;
		size_t	nBoneMax = hTex - ei.yVTBoneInstance ;
		//
		if ( pre->countMorph > 0 )
		{
			if ( nBoneMax > nMorphMax )
			{
				size_t	x = (nBoneMax - nMorphMax) / (wMorphInstance + 1) ;
				ei.yVTBoneInstance += x ;
				nMorphMax += wMorphInstance * x ;
				nBoneMax -= x ;
			}
			ei.nMaxInstance =
				(size_t) esl_min( (int) nMorphMax, (int) nBoneMax ) ;
		}
		else
		{
			ei.nMaxInstance = nBoneMax ;
		}
	}
	else
	{
		size_t	hTex = sglNormalizeScalePowerBy2( (uint32_t) (ei.yVTMorphInstance + 1) ) ;
		ei.sizeVertexTex.h = (int32_t) hTex ;
		ei.yVTBoneInstance = ei.yVTMorphInstance ;
		ei.nMaxInstance = (hTex - ei.yVTMorphInstance) * wMorphInstance ;
	}
}

void SGLOpenGLVertexBuffer::AllocBufferOffset
	( SGLOpenGLVertexBuffer::ENTRY_INFO& ei, RENDER_ENTRY * pre )
{
	ei.ofsVertex = m_bytesVertex ;
	ei.ofsElement = m_bytesElement ;
	//
	size_t	countIndex = pre->countIndex ;
	size_t	countVertex = pre->countVertex ;
	if ( pre->pvVertex != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S3DVector4) ;
	}
	if ( pre->pvNormal != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S3DVector4) ;
	}
	if ( pre->pvUVMap != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S2DVector) ;
	}
	if ( pre->pColor != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S3DColor) ;
	}
	if ( pre->pIndexedList != nullptr )
	{
		m_bytesElement += countIndex * sizeof(uint32_t) ;
	}
	if ( pre->pvTexAxisX != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S3DVector4) ;
	}
	if ( pre->pvTexAxisY != nullptr )
	{
		m_bytesVertex += countVertex * sizeof(S3DVector4) ;
	}
	for ( int j = 0; j < S3DRenderBuffer::countSubMesh; j ++ )
	{
		if ( (pre->pSubIndexedList[j] != nullptr)
			&& (pre->pSubIndexedList[j] != pre->pIndexedList) )
		{
			ei.ofsSubElements[j] = m_bytesElement ;
			m_bytesElement += pre->nSubIndexCount[j] * sizeof(uint32_t) ;
		}
	}
	if ( !ei.flagMustShapeByCPU && !ei.flagVertexTexture )
	{
		if ( pre->countMorph > 0 )
		{
			ei.ofsMorph = m_bytesVertex ;
			m_bytesVertex +=
				pre->countMorph * countVertex * SIZEOF_ELEMENT ;
		}
		if ( pre->countBone > 0 )
		{
			ei.ofsBone = m_bytesVertex ;
			m_bytesVertex +=
				((pre->countBone + 3) / 4)
					* countVertex * (sizeof(float32_t) * 4) ;
		}
	}
}

// 描画の確定時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnFlush( S3DVertexBufferInterface * pVB )
{
	S3DRenderBuffer *	prb = m_pRenderBuf ;
	ESLAssert( prb == pVB ) ;
	if ( prb != nullptr )
	{
		size_t	countEntry = prb->GetMeshCount() ;
		if ( m_arrEntryInfo.GetLength() < countEntry )
		{
			size_t	iFirst = m_arrEntryInfo.GetLength() ;
			m_arrEntryInfo.SetLength( countEntry ) ;
			//
			ENTRY_INFO *	peiNext = m_arrEntryInfo.GetArray() ;
			peiNext += iFirst ;
			for ( size_t i = iFirst; i < countEntry; i ++, peiNext ++ )
			{
				RENDER_ENTRY *	pre = prb->GetMeshEntryAt( i ) ;
				ESLAssert( pre != nullptr ) ;
				uint32_t	nFlags = S3DRenderBuffer::renderAutoNormal
										| S3DRenderBuffer::renderAutoColor ;
				if ( pre->pMaterial
					&& ((pre->pMaterial->m_attrSurface.flagsShading & shadingNormalTexture)
						|| ((pre->pMaterial->m_flagBack
							&& pre->pMaterial->m_attrBack.flagsShading & shadingNormalTexture))) )
				{
					nFlags |= S3DRenderBuffer::renderAutoTexAxis ;
				}
				prb->NormalizeVertexElements( *pre, nFlags ) ;
				//
				peiNext->pre = pre ;
				peiNext->flagMustShapeByCPU = false ;
				peiNext->flagVertexTexture = false ;
				//
				if ( S3DRenderDevice::m_availableMultiShapeVB
					&& ((pre->countBone >= 1)
						|| (pre->countMorph >= 1)
						|| (pre->nExAttrElements >= 1)) )
				{
					peiNext->flagVertexTexture =
						(pre->countBone <= 4)
						|| ((pre->ppJointMap != nullptr)
								&& (pre->countWeightMap <= 4)) ;
					peiNext->flagMustShapeByCPU = !peiNext->flagVertexTexture ;
					//
					if ( peiNext->flagVertexTexture )
					{
						AllocVertexTextureSize( *peiNext, pre ) ;
						//
						SGLImage *	pImage = new SGLImage ;
						pImage->CreateImage
							( (uint32_t) peiNext->sizeVertexTex.w,
								(uint32_t) peiNext->sizeVertexTex.h,
								formatImageFloatRGBA, 32 * 4,
								SGLImageObject::bufferSampleNoSmooth ) ;
						pImage->SetImageIdentity( L"texture for vertex" ) ;
						m_arrVertexTexture.SetAt( i, pImage ) ;
					}
				}
				AllocBufferOffset( *peiNext, pre ) ;
			}
			m_arrEntryInfo.FinishArray() ;
			//
			NotifyUpdateBuffer( iFirst, countEntry, elementAll ) ;
		}
	}
	S3DVertexDeviceBufferInterface::OnFlush( pVB ) ;
}

// プリミティブリストを更新時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnUpdateIndexedPrimitiveList
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	uint32_t	elementsUpdate = 0 ;
	if ( pvVertex != nullptr )
	{
		elementsUpdate |= elementVertex ;
	}
	if ( pvNormal != nullptr )
	{
		elementsUpdate |= elementNormal ;
	}
	if ( pvUVMap != nullptr )
	{
		elementsUpdate |= elementUVMap ;
	}
	if ( pColor != nullptr )
	{
		elementsUpdate |= elementColor ;
	}
	if ( pIndexedList != nullptr )
	{
		elementsUpdate |= elementIndex ;
	}
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementsUpdate ) ;

	S3DVertexDeviceBufferInterface::OnUpdateIndexedPrimitiveList
		( pVB, iMesh, nFlags, countIndex, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// サブメッシュ（ポリゴンリスト）を更新時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnUpdateSubIndexedTriangleList
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t iSubMesh, uint32_t nFlags,
		size_t countPolygon, const uint32_t * pIndexedList )
{
	ENTRY_INFO *	pei = m_arrEntryInfo.GetAt( iMesh ) ;
	if ( pei != nullptr )
	{
		RENDER_ENTRY *	pre = pei->pre ;
		ESLAssert( pre != nullptr ) ;
		if ( pei->ofsSubElements[iSubMesh] == 0 )
		{
			pei->ofsSubElements[iSubMesh] = m_bytesElement ;
			m_bytesElement += countPolygon * (sizeof(uint32_t) * 3) ;
		}
	}
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementSubIndex ) ;

	S3DVertexDeviceBufferInterface::OnUpdateSubIndexedTriangleList
				( pVB, iMesh, iSubMesh, nFlags, countPolygon, pIndexedList ) ;
}

// 追加的な頂点属性を設定時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnSetExtendVertexAttribute
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t countElements,
		size_t countVertex, const float32_t * pfpAttrElements )
{
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementExAttrElements ) ;

	S3DVertexDeviceBufferInterface::OnSetExtendVertexAttribute
				( pVB, iMesh, countElements, countVertex, pfpAttrElements ) ;
}

// メッシュにウェイトマップを設定時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnSetBoneWeightMap
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps )
{
	ENTRY_INFO *	pei = m_arrEntryInfo.GetAt( iMesh ) ;
	if ( pei != nullptr )
	{
		RENDER_ENTRY *	pre = pei->pre ;
		ESLAssert( pre != nullptr ) ;
		if ( pei->ofsBone == 0 )
		{
			pei->ofsBone = m_bytesVertex ;
			m_bytesVertex +=
				pre->countVertex
					* ((nCount + 3) / 4) * (sizeof(float32_t) * 4) ;
		}
	}
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementWeightMap ) ;

	S3DVertexDeviceBufferInterface::OnSetBoneWeightMap
						( pVB, iMesh, nCount, ppWeightMaps ) ;
}

// メッシュにモーフターゲット枠を確保時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnAllocateMorphing
	( S3DVertexBufferInterface * pVB, size_t iMesh, size_t nCount )
{
	ENTRY_INFO *	pei = m_arrEntryInfo.GetAt( iMesh ) ;
	if ( pei != nullptr )
	{
		RENDER_ENTRY *	pre = pei->pre ;
		ESLAssert( pre != nullptr ) ;
		if ( pei->ofsMorph == 0 )
		{
			pei->ofsMorph = m_bytesVertex ;
			m_bytesVertex +=
				pre->countVertex * nCount * SIZEOF_ELEMENT ;
		}
	}
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementMorphing ) ;

	S3DVertexDeviceBufferInterface::OnAllocateMorphing( pVB, iMesh, nCount ) ;
}

// メッシュにモーフターゲットを設定時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnSetMorphingTargetMesh
	( S3DVertexBufferInterface * pVB,
		size_t iMesh, size_t iMorph, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	uint32_t	elementsUpdate = 0 ;
	if ( pvVertex != nullptr )
	{
		elementsUpdate |= elementMorphVertex ;
	}
	if ( pvNormal != nullptr )
	{
		elementsUpdate |= elementMorphNormal ;
	}
	if ( pvUVMap != nullptr )
	{
		elementsUpdate |= elementMorphUVMap ;
	}
	if ( pColor != nullptr )
	{
		elementsUpdate |= elementMorphColor ;
	}
	NotifyUpdateBuffer( iMesh, iMesh + 1, elementsUpdate ) ;

	S3DVertexDeviceBufferInterface::OnSetMorphingTargetMesh
			( pVB, iMesh, iMorph,
				countVertex, pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// バッファ消去時処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLVertexBuffer::OnClearBuffer( S3DVertexBufferInterface * pVB )
{
	ESLAssert( m_pRenderBuf != nullptr ) ;
	ESLAssert( m_pRenderBuf == pVB ) ;
	if ( !(pVB->GetBufferControlFlags()
				& S3DRenderBuffer::bufferKeepDeviceBuffer) )
	{
		GLResource *	pRsrc = nullptr ;
		m_csSync.Lock() ;
		pRsrc = m_pFirstGLRsrc ;
		m_pFirstGLRsrc = nullptr ;
		m_csSync.Unlock() ;
		delete	pRsrc ;
	}
	//
	m_arrEntryInfo.RemoveAll() ;
	m_arrVertexTexture.RemoveAll() ;
	m_bytesVertex = 0 ;
	m_bytesElement = 0 ;

	S3DVertexDeviceBufferInterface::OnClearBuffer( pVB ) ;
}

// S3DPrimitiveType -> GLenum 変換
//////////////////////////////////////////////////////////////////////////////
const GLenum	SGLOpenGLVertexBuffer::m_glDrawMode[primitiveCount] =
{
	GL_POINTS,
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
} ;

GLenum SGLOpenGLVertexBuffer::PrimitiveTypeToGL( S3DPrimitiveType typePrimitive )
{
	ESLAssert( typePrimitive >= 0 ) ;
	ESLAssert( typePrimitive < primitiveCount ) ;
	return	m_glDrawMode[typePrimitive] ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL VertexVariantBuffer
//////////////////////////////////////////////////////////////////////////////
/*
// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLVertexVariantBuffer, S3DRenderVariantBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexVariantBuffer::SGLOpenGLVertexVariantBuffer( SGLOpenGLVertexBuffer * pvb )
	: S3DRenderVariantBuffer( pvb )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLVertexVariantBuffer::~SGLOpenGLVertexVariantBuffer( void )
{
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLVertexVariantBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
		uint64_t flagsExclusion, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing ) const
{
	S3DRenderBuffer *	prb = ESLTypeCast<S3DRenderBuffer>( m_buffer ) ;
	ESLAssert( prb != nullptr ) ;
	if ( prb == nullptr )
	{
		return	sglErrFailed ;
	}
	S3DRenderBuffer::VariantBuffer *
		pvb = ESLTypeCast<S3DRenderBuffer::VariantBuffer>( m_pvvbVar ) ;
	if ( pvb != nullptr )
	{
		pvb->ReflectMeshVisibleTo( prb ) ;
	}
	// SGLOpenGLVertexVariantBuffer 自身を VertexBuffer として
	// レンダリングキューに追加する
	// 最終的に S3DOpenGLDirectlyRenderer::AddVertexBuffer によって
	// prb->UpdateVertexVariant( m_pvvbVar, iFirst, iEnd ) が呼び出される
	return	prb->RenderVertexBufferTo
				( render, (S3DVertexBufferInterface*) this,
					flagsExclusion, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}
*/



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 GLSL 標準シェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLOpenGLDefaultShader, SGLOpenGLShaderProgram )

// 使用テクスチャの割り当て
const int	SGLOpenGLDefaultShader::m_gl_iMaterialTextures[32] =
{
	SGLOpenGLDefaultShader::glTextureMaterial0,
	SGLOpenGLDefaultShader::glTextureMaterial1,
	SGLOpenGLDefaultShader::glTextureMaterial2,
	SGLOpenGLDefaultShader::glTextureMaterial3,
	SGLOpenGLDefaultShader::glTextureMaterial4,
	SGLOpenGLDefaultShader::glTextureMaterial5,
	SGLOpenGLDefaultShader::glTextureMaterial6,
	SGLOpenGLDefaultShader::glTextureMaterial7,
	SGLOpenGLDefaultShader::glTextureMaterial8,
	SGLOpenGLDefaultShader::glTextureMaterial9,
	SGLOpenGLDefaultShader::glTextureMaterial10,
	SGLOpenGLDefaultShader::glTextureMaterial11,
	SGLOpenGLDefaultShader::glTextureMaterial12,
	SGLOpenGLDefaultShader::glTextureMaterial13,
	SGLOpenGLDefaultShader::glTextureMaterial14,
	SGLOpenGLDefaultShader::glTextureMaterial15,
	SGLOpenGLDefaultShader::glTextureMaterial15+1,
	SGLOpenGLDefaultShader::glTextureMaterial15+2,
	SGLOpenGLDefaultShader::glTextureMaterial15+3,
	SGLOpenGLDefaultShader::glTextureMaterial15+4,
	SGLOpenGLDefaultShader::glTextureMaterial15+5,
	SGLOpenGLDefaultShader::glTextureMaterial15+6,
	SGLOpenGLDefaultShader::glTextureMaterial15+7,
	SGLOpenGLDefaultShader::glTextureMaterial15+8,
	SGLOpenGLDefaultShader::glTextureMaterial15+9,
	SGLOpenGLDefaultShader::glTextureMaterial15+10,
	SGLOpenGLDefaultShader::glTextureMaterial15+11,
	SGLOpenGLDefaultShader::glTextureMaterial15+12,
	SGLOpenGLDefaultShader::glTextureMaterial15+13,
	SGLOpenGLDefaultShader::glTextureMaterial15+14,
	SGLOpenGLDefaultShader::glTextureMaterial15+15,
	SGLOpenGLDefaultShader::glTextureMaterial15+16,
} ;

// ボーンパレット使用制限数（シェーダー作成時最大数制限）
size_t	SGLOpenGLDefaultShader::m_limit_bone_count = 12 ;

// 光源使用制限数（シェーダー作成時最大数制限）
size_t	SGLOpenGLDefaultShader::m_limit_light_count = 2 ;

// シャドウマッピング制限数
size_t	SGLOpenGLDefaultShader::m_limit_shadow_map_count = 6 ;

// 環境マッピング使用不可
#if	defined(__API_OPEN_GL_ES__)
bool	SGLOpenGLDefaultShader::m_disable_environment_mapping = false ;
bool	SGLOpenGLDefaultShader::m_disable_environment_cubemapping = false ;
bool	SGLOpenGLDefaultShader::m_disable_environment_spheremapping = false ;
bool	SGLOpenGLDefaultShader::m_disable_environment_viewport = false ;
bool	SGLOpenGLDefaultShader::m_disable_environment_refraction = false ;
#else
bool	SGLOpenGLDefaultShader::m_disable_environment_mapping = true ;
bool	SGLOpenGLDefaultShader::m_disable_environment_cubemapping = true ;
bool	SGLOpenGLDefaultShader::m_disable_environment_spheremapping = true ;
bool	SGLOpenGLDefaultShader::m_disable_environment_viewport = true ;
bool	SGLOpenGLDefaultShader::m_disable_environment_refraction = true ;
#endif

// 法線テクスチャ使用不可
bool	SGLOpenGLDefaultShader::m_disable_normal_mapping = true ;

// 標高テクスチャ使用不可
#if	defined(__API_OPEN_GL_ES__)
bool	SGLOpenGLDefaultShader::m_disable_height_mapping = false ;
#else
bool	SGLOpenGLDefaultShader::m_disable_height_mapping = true ;
#endif

// スペキュラ・粗さ・反射率テクスチャ使用不可
#if	defined(__API_OPEN_GL_ES__)
bool	SGLOpenGLDefaultShader::m_disable_specular_mapping = false ;
#else
bool	SGLOpenGLDefaultShader::m_disable_specular_mapping = true ;
#endif

// 大域ライトマップAOテクスチャ使用不可
#if	defined(__API_OPEN_GL_ES__)
bool	SGLOpenGLDefaultShader::m_disable_global_ao_lightmap = false ;
#else
bool	SGLOpenGLDefaultShader::m_disable_global_ao_lightmap = true ;
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDefaultShader::SGLOpenGLDefaultShader( SGLOpenGLContext * pOpenGL )
	: SGLOpenGLShaderProgram( pOpenGL ), m_mat4ModelView( 1, 1, 1, 1 )
{
	m_maxLightCount = MAX_LIGHT_COUNT ;
	m_maxBonePalette = MAX_BONE_PALETTE ;
	m_maxShadowmapping = MAX_SHADOWMAPPING ;
	m_enabledMorphing = true ;
	m_enabledInstancing = false ;
	m_enabledVTInstancing = false ;
	m_preparedDepthBuffer = false ;
	//
	eslFillMemory( m_mat4DummyInstancing, 0, sizeof(GLfloat) * 4 * 4 ) ;
	m_mat4DummyInstancing[0][0] = 1.0f ;
	m_mat4DummyInstancing[1][1] = 1.0f ;
	m_mat4DummyInstancing[2][2] = 1.0f ;
	m_mat4DummyInstancing[3][3] = 1.0f ;
	//
	m_colorDummyInstancing.rgbMul = 0xFFFFFFFF ;
	m_colorDummyInstancing.rgbAdd = 0 ;
	//
	m_psaLastMaterial = nullptr ;
	m_flagsLastMaterialShading = 0 ;
	//
	m_fEnableTextureSmoothing = true ;
	m_fDisableWriteDepth = false ;
	m_fShaderForceToon = false ;
	m_fShaderForceBorder = false ;
	m_fShaderNoDrawBorder = false ;
	m_fShaderSurfaceOffset = false ;
	m_fShaderEmisiveTarget = false ;
	m_rgbBorderColor.ui32 = 0 ;
	m_fpBorderCoefficient[0] = 0.0f ;
	m_fpBorderCoefficient[1] = 1.0f ;
	//
	m_faceCulling = S3DRenderBufferInterface::faceCullingDefault ;
	m_depthMask = S3DRenderBufferInterface::depthMaskDefault ;
	m_blendOperation = S3DRenderBufferInterface::blendDefault ;
	//
	for ( int i = 0; i < MAX_SHADOWMAPPING; i ++ )
	{
		m_bufShadowmap.iLights[i] = -1 ;
	}
	m_flagEnabledShadowmap = true ;
//	m_iShadowmapLight = -1 ;
//	m_iShadowmapLight2 = -1 ;
//	m_iEnableShadowmap = -1 ;
//	m_iEnableShadowmap2 = -1 ;
	m_pVertexTexture = nullptr ;
	m_iVertexTexture = -1 ;
	m_pEnvMapping = nullptr ;
	m_nEnvMappingType = ENV_MAPPING_NOTHING ;
	m_matEnvMapping.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	m_pEnvViewportDepth = nullptr ;
	m_pEnvRefraction = nullptr ;
	m_nEnvRefractionType = ENV_MAPPING_NOTHING ;
	m_fpEnvRefractionDeepness = 1.0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLDefaultShader::~SGLOpenGLDefaultShader( void )
{
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::InitializeProgram
	( int typeShading, SGLOpenGLShaderProgram::CompileListener * pListener )
{
	if ( !OpenGLExtension::g_supports_opengl_1_3
		|| !OpenGLExtension::g_supports_opengl_1_5
		|| !OpenGLExtension::g_supports_opengl_2_0 )
	{
		return	sglErrNotSupported ;
	}
	//
	m_maxLightCount = MAX_LIGHT_COUNT ;
	m_maxBonePalette = MAX_BONE_PALETTE ;
	m_maxShadowmapping = MAX_SHADOWMAPPING ;
	m_enabledMorphing = true ;
	m_enabledInstancing = OpenGLExtension::g_supports_instanced_draw ;
	m_enabledVTInstancing = m_pOpenGL->m_flagAvailableMultiSVB ;
	m_preparedDepthBuffer = false ;
	if ( !(typeShading & shadingPhong) )
	{
		m_maxLightCount = 1 ;
		m_maxShadowmapping = 0 ;
	}
	//
	// シェーダープログラム・共通ヘッダ
	//
	SArray<char>	bufVertHeader ;
	SArray<char>	bufFragHeader ;
	SString			strVerHeader = L"" ;
	SString			strCommonHeader = L"" ;
	SString			strFragOptHeader = L"" ;
	SString			strFragHeader = L"" ;
	SString			strVertHeader = L"" ;
	int				nVersion = 0 ;
	//
	bool	fDisableNoramlTexture = false ;
	bool	fDisableHeightTexture = false ;
	bool	fDisableSpecularTexture = false ;
	bool	fDisableGlobalAOTexture = false ;
	bool	fDisableEnvMapping = false ;
	bool	fWithGeometryShader = false ;
	bool	fEnableExAttrElement = false ;
	//
	SetBoneLimitForShaderHeader( strCommonHeader, m_limit_bone_count ) ;
	//
	#if	defined(__API_OPEN_GL_ES__)
	if ( m_limit_light_count <= 1 )
	{
		strCommonHeader += L"#define _DISABLE_LIGHT_OVER_1 1\r\n" ;
	}
	else if ( m_limit_light_count < 4 )
	{
		strCommonHeader += L"#define _DISABLE_LIGHT_OVER_" ;
		strCommonHeader += SString( m_limit_light_count ) ;
		strCommonHeader += L" 1\r\n" ;
	}
	#endif
	if ( m_limit_shadow_map_count < m_maxShadowmapping )
	{
		m_maxShadowmapping = m_limit_shadow_map_count ;
	}
	if ( m_maxShadowmapping == 0 )
	{
		strCommonHeader += L"#define _DISABLE_SHADOWMAPPING 1\r\n" ;
	}
	strCommonHeader += L"#define _LIMIT_SHADOWMAP_COUNT " ;
	strCommonHeader += SString( m_maxShadowmapping ) ;
	strCommonHeader += L"\r\n" ;
	if ( m_pOpenGL->m_flagTextureFloat
		&& m_pOpenGL->m_flagSupportedComputeShader )
	{
		strCommonHeader += L"#define _PREPARED_SHADOW_DEPTH_BUFFER 1\r\n" ;
		m_preparedDepthBuffer = true ;
	}
	if ( !(typeShading & shadingPhong) )
	{
		strCommonHeader += L"#define _DISABLE_NORMAL_TEXTURE 1\r\n" ;
		strCommonHeader += L"#define _DISABLE_HEIGHT_TEXTURE 1\r\n" ;
		strCommonHeader += L"#define _DISABLE_SPECULAR_TEXTURE 1\r\n" ;
		fDisableNoramlTexture = true ;
		fDisableHeightTexture = true ;
		fDisableSpecularTexture = true ;
	}
	if ( !fDisableNoramlTexture
		&& SGLOpenGLDefaultShader::m_disable_normal_mapping )
	{
		strCommonHeader += L"#define _DISABLE_NORMAL_TEXTURE 1\r\n" ;
		fDisableNoramlTexture = true ;
	}
	if ( !fDisableHeightTexture
		&& SGLOpenGLDefaultShader::m_disable_height_mapping )
	{
		strCommonHeader += L"#define _DISABLE_HEIGHT_TEXTURE 1\r\n" ;
		fDisableHeightTexture = true ;
	}
	if ( !fDisableSpecularTexture
		&& SGLOpenGLDefaultShader::m_disable_specular_mapping )
	{
		strCommonHeader += L"#define _DISABLE_SPECULAR_TEXTURE 1\r\n" ;
		fDisableSpecularTexture = true ;
	}
	if ( !fDisableGlobalAOTexture
		&& SGLOpenGLDefaultShader::m_disable_global_ao_lightmap )
	{
		strCommonHeader += L"#define _DISABLE_LIGHTMAP_AO_TEXTURE 1\r\n" ;
		fDisableGlobalAOTexture = true ;
	}
	if ( fDisableNoramlTexture && fDisableHeightTexture )
	{
		strCommonHeader += L"#define _DISABLE_UV_AXIS_ATTRIBUTE 1\r\n" ;
	}
	else if ( m_pOpenGL->m_flagSupportedGeometry
				&& (typeShading & shadingPhong) )
	{
		fWithGeometryShader = true ;
		if ( nVersion < 150 )
		{
			#if	defined(__API_OPEN_GL_ES__)
			nVersion = 320 ;
			strVerHeader = L"#version 320 es\r\n" ;
			#else
			nVersion = 150 ;
			strVerHeader = L"#version 150 core\r\n" ;
			#endif
			strCommonHeader += L"#define _GLSL_VERSION_130_LATER_ 1\r\n" ;
		}
		strCommonHeader += L"#define _DISABLE_UV_AXIS_ATTRIBUTE 1\r\n" ;
		strCommonHeader += L"#define _LINK_GEOMETRY_SHADER 1\r\n" ;
	}
	if ( !fDisableEnvMapping
		&& SGLOpenGLDefaultShader::m_disable_environment_mapping )
	{
		strCommonHeader += L"#define _DISABLE_ENVIRONMENT_MAPPING 1\r\n" ;
		fDisableEnvMapping = true ;
	}
	else if ( !SGLOpenGLDefaultShader::m_disable_environment_mapping )
	{
		if ( SGLOpenGLDefaultShader::m_disable_environment_cubemapping
			|| !m_pOpenGL->m_flagCubemapTexture )
		{
			strCommonHeader += L"#define _DISABLE_ENVIRONMENT_CUBMAP 1\r\n" ;
		}
		if ( SGLOpenGLDefaultShader::m_disable_environment_spheremapping )
		{
			strCommonHeader += L"#define _DISABLE_ENVIRONMENT_SPHERE 1\r\n" ;
		}
		if ( SGLOpenGLDefaultShader::m_disable_environment_viewport )
		{
			strCommonHeader += L"#define _DISABLE_ENVIRONMENT_VIEWPORT 1\r\n" ;
		}
		if ( SGLOpenGLDefaultShader::m_disable_environment_refraction )
		{
			strCommonHeader += L"#define _DISABLE_REFRACTION_MAPPING 1\r\n" ;
		}
	}
	if ( m_pOpenGL->m_flagSupportedMRT && (m_pOpenGL->m_versionGL[0] >= 3) )
	{
		if ( nVersion < 130 )
		{
			#if	defined(__API_OPEN_GL_ES__)
			nVersion = 300 ;
			strVerHeader = L"#version 300 es\r\n" ;
			#else
			nVersion = 130 ;
			strVerHeader = L"#version 130\r\n" ;
			#endif
			strCommonHeader += L"#define _GLSL_VERSION_130_LATER_ 1\r\n" ;
		}
		int	nMRT = esl_min( m_pOpenGL->m_maxDrawBuffers,
							S3DRenderDevice::renderTargetCount ) ;
		if ( nMRT >= 2 )
		{
			SString	strMRTCommonHdr, strMRTFragHdr ;
			const wchar_t *	pwszPrecision = L"" ;
			#if	defined(__API_OPEN_GL_ES__)
			pwszPrecision = L"lowp" ;
			#endif
			strMRTCommonHdr.Format
				( L"#define _MULTIPLE_RENDER_TARGET %d\r\n", nMRT ) ;
			strMRTFragHdr.Format
				( L"out %s vec4 fr_FragData[%d] ;\r\n",
					pwszPrecision, nMRT ) ;
			strCommonHeader += strMRTCommonHdr ;
			strFragOptHeader += strMRTFragHdr ;
		}
	}
	//
//	if ( typeShading == 0 )
//	{
//		m_enabledInstancing = false ;
//	}
	SetAttributeLimitForShaderHeader( strCommonHeader ) ;
	//
	if ( m_enabledVTInstancing )
	{
		if ( nVersion < 140 )
		{
			if ( nVersion < 130 )
			{
				strCommonHeader += L"#define _GLSL_VERSION_130_LATER_ 1\r\n" ;
			}
			#if	defined(__API_OPEN_GL_ES__)
			nVersion = 310 ;
			strVerHeader = L"#version 310 es\r\n" ;
			#else
			nVersion = 140 ;
			strVerHeader = L"#version 140\r\n" ;
			#endif
		}
		strCommonHeader += L"#define _ENABLE_EXTEND_ATTR_ 1\r\n" ;
		fEnableExAttrElement = true ;
	}
	//
	strFragHeader = strVerHeader + strCommonHeader + strFragOptHeader ;
	strVertHeader = strVerHeader + strCommonHeader ;
	strVertHeader.EncodeDefaultTo( bufVertHeader ) ;
	strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
	//
	// シェーダープログラム生成
	//
	Source		srcVertex[10] ;
	Source		srcFragment[10] ;
	Source		srcGeometry[10] ;
	Source *	psrcGeometry = nullptr ;
	size_t		nVertex = 0 ;
	size_t		nFragment = 0 ;
	size_t		nGeometry = 0 ;
	//
	if ( typeShading & shadingPhong )
	{
		// フォンシェーディング・シェーダー
		// （EntisGLS4 標準シェーダーの中で最も汎用性のあるシェーダー）
		srcVertex[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
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
		srcVertex[5].pszPlaneSrc = nullptr ;
		srcVertex[5].pbytEncodedSrc = &g_glsl_main_phong_vert[0] ;
		srcVertex[5].nEncodedBytes = sizeof(g_glsl_main_phong_vert) ;
		nVertex = 6 ;
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
		srcFragment[5].pszPlaneSrc = nullptr ;
		srcFragment[5].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
		srcFragment[5].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
		srcFragment[6].pszPlaneSrc = nullptr ;
		srcFragment[6].pbytEncodedSrc = &g_glsl_shader_shadowmap_frag[0] ;
		srcFragment[6].nEncodedBytes = sizeof(g_glsl_shader_shadowmap_frag) ;
		srcFragment[7].pszPlaneSrc = nullptr ;
		srcFragment[7].pbytEncodedSrc = &g_glsl_shader_common_frag[0] ;
		srcFragment[7].nEncodedBytes = sizeof(g_glsl_shader_common_frag) ;
		srcFragment[8].pszPlaneSrc = nullptr ;
		srcFragment[8].pbytEncodedSrc = &g_glsl_main_phong_frag[0] ;
		srcFragment[8].nEncodedBytes = sizeof(g_glsl_main_phong_frag) ;
		nFragment = 9 ;
		//
		if ( fWithGeometryShader )
		{
			srcGeometry[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
			srcGeometry[0].pbytEncodedSrc = nullptr ;
			srcGeometry[0].nEncodedBytes = 0 ;
			srcGeometry[1].pszPlaneSrc = nullptr ;
			srcGeometry[1].pbytEncodedSrc = &g_glsl_main_phong_geom[0] ;
			srcGeometry[1].nEncodedBytes = sizeof(g_glsl_main_phong_geom) ;
			psrcGeometry = srcGeometry ;
			nGeometry = 2 ;
		}
	}
	else if ( typeShading & shadingGouraud )
	{
		// グーローシェーディング・シェーダー
		// （低スペック GPU のための選択肢としてのシェーダー）
		srcVertex[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
		srcVertex[0].pbytEncodedSrc = nullptr ;
		srcVertex[0].nEncodedBytes = 0 ;
		srcVertex[1].pszPlaneSrc = nullptr ;
		srcVertex[1].pbytEncodedSrc = &g_glsl_header_vert[0] ;
		srcVertex[1].nEncodedBytes = sizeof(g_glsl_header_vert) ;
		srcVertex[2].pszPlaneSrc = nullptr ;
		srcVertex[2].pbytEncodedSrc = &g_glsl_header_common_frag[0] ;
		srcVertex[2].nEncodedBytes = sizeof(g_glsl_header_common_frag) ;
		srcVertex[3].pszPlaneSrc = nullptr ;
		srcVertex[3].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
		srcVertex[3].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
		srcVertex[4].pszPlaneSrc = nullptr ;
		srcVertex[4].pbytEncodedSrc = &g_glsl_shader_common_frag[0] ;
		srcVertex[4].nEncodedBytes = sizeof(g_glsl_shader_common_frag) ;
		srcVertex[5].pszPlaneSrc = nullptr ;
		srcVertex[5].pbytEncodedSrc = &g_glsl_vertex_common_vert[0] ;
		srcVertex[5].nEncodedBytes = sizeof(g_glsl_vertex_common_vert) ;
		srcVertex[6].pszPlaneSrc = nullptr ;
		srcVertex[6].pbytEncodedSrc = &g_glsl_main_gouraud_vert[0] ;
		srcVertex[6].nEncodedBytes = sizeof(g_glsl_main_gouraud_vert) ;
		nVertex = 7 ;
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
		srcFragment[4].pbytEncodedSrc = &g_glsl_light_common_frag[0] ;
		srcFragment[4].nEncodedBytes = sizeof(g_glsl_light_common_frag) ;
		srcFragment[5].pszPlaneSrc = nullptr ;
		srcFragment[5].pbytEncodedSrc = &g_glsl_shader_shadowmap_frag[0] ;
		srcFragment[5].nEncodedBytes = sizeof(g_glsl_shader_shadowmap_frag) ;
		srcFragment[6].pszPlaneSrc = nullptr ;
		srcFragment[6].pbytEncodedSrc = &g_glsl_main_gouraud_frag[0] ;
		srcFragment[6].nEncodedBytes = sizeof(g_glsl_main_gouraud_frag) ;
		nFragment = 7 ;
	}
	else
	{
		// シェーディング無し・シェーダー
		// （低スペック GPU での２Ｄ描画などのための簡易シェーダー）
		strCommonHeader += L"#define _DISABLE_SHADING 1\r\n" ;
		strFragHeader = strVerHeader + strCommonHeader + strFragOptHeader ;
		strVertHeader = strVerHeader + strCommonHeader ;
		strVertHeader.EncodeDefaultTo( bufVertHeader ) ;
		strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
		//
		srcVertex[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
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
		srcVertex[4].pszPlaneSrc = nullptr ;
		srcVertex[4].pbytEncodedSrc = &g_glsl_main_gouraud_vert[0] ;
		srcVertex[4].nEncodedBytes = sizeof(g_glsl_main_gouraud_vert) ;
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
		srcFragment[4].pbytEncodedSrc = &g_glsl_main_gouraud_frag[0] ;
		srcFragment[4].nEncodedBytes = sizeof(g_glsl_main_gouraud_frag) ;
		nFragment = 5 ;
	}
	//
	SGLError	err =
		CreateProgram
			( srcVertex, nVertex,
				psrcGeometry, nGeometry,
				srcFragment, nFragment, pListener ) ;
	if ( err )
	{
		#if	defined(__DEBUG__)
		SString	strBaseShaderID = "default_no_shade" ;
		if ( typeShading & shadingPhong )
		{
			strBaseShaderID = "default_phong" ;
		}
		else if ( typeShading & shadingGouraud )
		{
			strBaseShaderID = "default_gouraud" ;
		}
		if ( GetVertexShaderSource().GetLength() > 0 )
		{
			SSmartPointer<SFileInterface>	pFile =
				SFileOpener::DefaultNewOpenFile
					( strBaseShaderID + L".vert", SFileOpener::modeCreate ) ;
			if ( pFile != nullptr )
			{
				pFile->Write
					( GetVertexShaderSource().GetConstArray(),
						GetVertexShaderSource().GetLength() ) ;
			}
		}
		if ( GetFragmentShaderSource().GetLength() > 0 )
		{
			SSmartPointer<SFileInterface>	pFile =
				SFileOpener::DefaultNewOpenFile
					( strBaseShaderID + L".frag", SFileOpener::modeCreate ) ;
			if ( pFile != nullptr )
			{
				pFile->Write
					( GetFragmentShaderSource().GetConstArray(),
						GetFragmentShaderSource().GetLength() ) ;
			}
		}
		if ( GetGeometryShaderSource().GetLength() > 0 )
		{
			SSmartPointer<SFileInterface>	pFile =
				SFileOpener::DefaultNewOpenFile
					( strBaseShaderID + L".geom", SFileOpener::modeCreate ) ;
			if ( pFile != nullptr )
			{
				pFile->Write
					( GetGeometryShaderSource().GetConstArray(),
						GetGeometryShaderSource().GetLength() ) ;
			}
		}
		#endif
		static bool		s_flagOldTegra = false ;
		const struct	TryOption
		{
			size_t	nLightLimit ;
			size_t	nBoneLimit ;
			bool	flagMorphing ;
		}
		toptParam[] =
		{
			{ 1, 0, false },
			{ 2, 4, false },
			{ 3, 8, false },
		} ;
		const int	nOptCount = sizeof(toptParam)/sizeof(toptParam[0]) ;
		//
		SString	strOrgCommonHeader = strCommonHeader ;
		//
		// シェーダープログラムのサイズの問題があるかもしれないので
		// 減らして行ってみる
		if ( !s_flagOldTegra )
		for ( int i = nOptCount - 1; i >= 0; i -- )
		{
			strCommonHeader = strOrgCommonHeader ;
			#if	defined(__API_OPEN_GL_ES__)
			if ( m_limit_light_count > toptParam[i].nLightLimit )
			{
				ESLTrace( "try to create GLSL shader limited light %d.\n", toptParam[i].nLightLimit ) ;
				strCommonHeader += L"#define _DISABLE_LIGHT_OVER_" ;
				strCommonHeader += SString( toptParam[i].nLightLimit ) ;
				strCommonHeader += L" 1\r\n" ;
			}
			#else
			ESLTrace( "try to create GLSL shader limited bone %d.\n", toptParam[i].nBoneLimit ) ;
			#endif
			if ( m_maxBonePalette > toptParam[i].nBoneLimit )
			{
				strCommonHeader += L"#define _DISABLE_BONE_OVER_" ;
				strCommonHeader += SString( toptParam[i].nBoneLimit ) ;
				strCommonHeader += L" 1\r\n" ;
			}
			if ( !toptParam[i].flagMorphing && m_enabledMorphing )
			{
				strCommonHeader += L"#define _DISABLE_MORPHING 1\r\n" ;
			}
			strFragHeader = strVerHeader + strCommonHeader + strFragOptHeader ;
			strVertHeader = strVerHeader + strCommonHeader ;
			strVertHeader.EncodeDefaultTo( bufVertHeader ) ;
			strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
			srcVertex[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
			srcFragment[0].pszPlaneSrc = bufFragHeader.GetConstArray() ;
			srcGeometry[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
			Release() ;
			err = CreateProgram
				( srcVertex, nVertex,
					psrcGeometry, nGeometry,
					srcFragment, nFragment, pListener ) ;
			if ( !err )
			{
				#if	defined(__API_OPEN_GL_ES__)
				m_maxLightCount = (size_t) toptParam[i].nLightLimit ;
				#endif
				m_maxBonePalette = (size_t) toptParam[i].nBoneLimit ;
				m_enabledMorphing = toptParam[i].flagMorphing ;
				break ;
			}
		}
		#if	defined(__API_OPEN_GL_ES__)
		if ( err )
		{
			// 古い Tegra-2 では関数引数の
			// 精度指定子(lowp,mediump,highp)がエラーになる
			// ドライバが存在するので、外して試す
			m_maxLightCount = MAX_LIGHT_COUNT ;
			m_maxBonePalette = MAX_BONE_PALETTE ;
			m_enabledMorphing = true ;
			ESLTrace( "try to create GLSL shader for old Tegra.\n" ) ;
			for ( int i = nOptCount - 1; i >= 0; i -- )
			{
				strCommonHeader = strOrgCommonHeader ;
				strCommonHeader += L"#define _DISABLE_FLOAT_PRECISION_" ;
				if ( m_limit_light_count > i )
				{
					ESLTrace( "try to create GLSL shader limited light %d.\n", toptParam[i].nLightLimit ) ;
					strCommonHeader += L"#define _DISABLE_LIGHT_OVER_" ;
					strCommonHeader += SString( toptParam[i].nLightLimit ) ;
					strCommonHeader += L" 1\r\n" ;
				}
				if ( m_maxBonePalette > toptParam[i].nBoneLimit )
				{
					strCommonHeader += L"#define _DISABLE_BONE_OVER_" ;
					strCommonHeader += SString( toptParam[i].nBoneLimit ) ;
					strCommonHeader += L" 1\r\n" ;
				}
				if ( !toptParam[i].flagMorphing && m_enabledMorphing )
				{
					strCommonHeader += L"#define _DISABLE_MORPHING 1\r\n" ;
				}
				strFragHeader = strVerHeader + strCommonHeader + strFragOptHeader ;
				strVertHeader = strVerHeader + strCommonHeader ;
				strVertHeader.EncodeDefaultTo( bufVertHeader ) ;
				strFragHeader.EncodeDefaultTo( bufFragHeader ) ;
				srcVertex[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
				srcFragment[0].pszPlaneSrc = bufFragHeader.GetConstArray() ;
				srcGeometry[0].pszPlaneSrc = bufVertHeader.GetConstArray() ;
				Release() ;
				err = CreateProgram
					( srcVertex, nVertex,
						psrcGeometry, nGeometry,
						srcFragment, nFragment, pListener ) ;
				if ( !err )
				{
					m_maxLightCount = (size_t) toptParam[i].nLightLimit ;
					m_maxBonePalette = (size_t) toptParam[i].nBoneLimit ;
					m_enabledMorphing = toptParam[i].flagMorphing ;
					s_flagOldTegra = true ;
					break ;
				}
			}
		}
		#endif
		if ( err )
		{
			return	err ;
		}
	}
	if ( m_maxBonePalette > m_limit_bone_count )
	{
		m_maxBonePalette = m_limit_bone_count ;
	}
	#if	defined(__API_OPEN_GL_ES__)
	if ( m_maxLightCount > m_limit_light_count )
	{
		m_maxLightCount = m_limit_light_count ;
	}
	#endif
	//
	// シェーダー属性／ユニフォーム位置取得
	//
	GetShaderVariableLocations() ;
	//
	return	sglErrSuccess ;
}

// ボーン数制限用ヘッダ追加と m_maxBonePalette 設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetBoneLimitForShaderHeader
	( SSystem::SString& strHeader, size_t nBoneLimit )
{
	if ( m_enabledVTInstancing )
	{
		m_maxBonePalette = 4 ;
	}
	else
	{
		if ( nBoneLimit <= 0 )
		{
			if ( m_maxBonePalette > 0 )
			{
				strHeader += L"#define _DISABLE_BONE_OVER_0 1\r\n" ;
				m_maxBonePalette = 0 ;
			}
		}
		if ( nBoneLimit <= 4 )
		{
			if ( m_maxBonePalette > 4 )
			{
				strHeader += L"#define _DISABLE_BONE_OVER_4 1\r\n" ;
				m_maxBonePalette = 4 ;
			}
		}
		if ( nBoneLimit <= 8 )
		{
			if ( m_maxBonePalette > 8 )
			{
				strHeader += L"#define _DISABLE_BONE_OVER_8 1\r\n" ;
				m_maxBonePalette = 8 ;
			}
		}
		if ( nBoneLimit <= 12 )
		{
			if ( m_maxBonePalette > 12 )
			{
				strHeader += L"#define _DISABLE_BONE_OVER_12 1\r\n" ;
				m_maxBonePalette = 12 ;
			}
		}
	}
}

// GLSL attribute 最大数からヘッダを調整する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetAttributeLimitForShaderHeader( SSystem::SString& strHeader )
{
	if ( m_enabledVTInstancing )
	{
		if ( m_enabledMorphing )
		{
			m_enabledMorphing = false ;
		}
		if ( m_pOpenGL->m_maxVertexAttributes < 10 )
		{
			m_enabledInstancing = false ;
			m_enabledVTInstancing = false ;
			//
			if ( m_maxBonePalette > 0 )
			{
				strHeader += L"#define _DISABLE_BONE_OVER_0 1\r\n" ;
				m_maxBonePalette = 0 ;
			}
			strHeader += L"#define _DISABLE_MORPHING 1\r\n" ;
			ESLTrace( "disable shader multi shape shader by too short OpenGL attribute availability.\n" ) ;
		}
		else
		{
			m_enabledInstancing = true ;
			//
			strHeader += L"#define _ENABLE_INSTANCING 1\r\n" ;
			strHeader += L"#define _ENABLE_VT_INSTANCING 1\r\n" ;
			ESLTrace( "enable OpenGL variant shape instanced draw for shader.\n" ) ;
		}
	}
	else
	{
		const int	nAttr4Vertex = 5 ;
		const int	nAttr4Bone = (int) (m_maxBonePalette + 3) / 4 ;
		const int	nAttrInstacing = 5 ;
		int	nNeedsAttr = nAttr4Vertex
							+ (m_enabledMorphing ? nAttr4Vertex : 0)
							+ nAttr4Bone
							+ (m_enabledInstancing ? nAttrInstacing : 0) ;
		if ( m_pOpenGL->m_maxVertexAttributes < nNeedsAttr )
		{
			if ( m_enabledMorphing )
			{
				nNeedsAttr -= nAttr4Vertex ;
				m_enabledMorphing = false ;
				strHeader += L"#define _DISABLE_MORPHING 1\r\n" ;
				ESLTrace( "disable shader morphing by too short OpenGL attribute availability.\n" ) ;
			}
			//
			if ( m_pOpenGL->m_maxVertexAttributes < nNeedsAttr )
			{
				if ( (m_pOpenGL->m_maxVertexAttributes < 10) && m_enabledInstancing )
				{
					m_enabledInstancing = false ;
					nNeedsAttr -= nAttrInstacing ;
				}
				else
				{
					nNeedsAttr -= nAttr4Bone ;
					//
					int	nAvailableBones =
							(m_pOpenGL->m_maxVertexAttributes - nNeedsAttr) * 4 ;
					if ( (nAvailableBones <= 0) && (m_maxBonePalette > 0) )
					{
						strHeader += L"#define _DISABLE_BONE_OVER_0 1\r\n" ;
						m_maxBonePalette = 0 ;
						ESLTrace( "disable shader bone palette by too short OpenGL attribute availability.\n" ) ;
					}
					else if ( (nAvailableBones <= 4) && (m_maxBonePalette > 4) )
					{
						strHeader += L"#define _DISABLE_BONE_OVER_4 1\r\n" ;
						m_maxBonePalette = 4 ;
						ESLTrace( "limit shader bone palette for 4 by too short OpenGL attribute availability.\n" ) ;
					}
					else if ( (nAvailableBones <= 8) && (m_maxBonePalette > 8) )
					{
						strHeader += L"#define _DISABLE_BONE_OVER_8 1\r\n" ;
						m_maxBonePalette = 8 ;
						ESLTrace( "limit shader bone palette for 8 by too short OpenGL attribute availability.\n" ) ;
					}
					else if ( (nAvailableBones <= 12) && (m_maxBonePalette > 12) )
					{
						strHeader += L"#define _DISABLE_BONE_OVER_12 1\r\n" ;
						m_maxBonePalette = 8 ;
						ESLTrace( "limit shader bone palette for 12 by too short OpenGL attribute availability.\n" ) ;
					}
					nNeedsAttr += (int) m_maxBonePalette / 4 ;
				}
			}
			if ( m_pOpenGL->m_maxVertexAttributes < nNeedsAttr )
			{
				m_enabledInstancing = false ;
			}
		}
		if ( m_enabledInstancing )
		{
			strHeader += L"#define _ENABLE_INSTANCING 1\r\n" ;
			ESLTrace( "enable OpenGL instanced draw for shader.\n" ) ;
		}
	}
}

// シェーダープログラム保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::SaveProgramBinary( S3DShaderBinary& bin )
{
	SGLError	err = SGLOpenGLShaderProgram::SaveProgramBinary( bin ) ;
	if ( err )
	{
		return	err ;
	}
	bin.SetAttrIntegerAs( L"max_light", m_maxLightCount ) ;
	bin.SetAttrIntegerAs( L"max_bone", m_maxBonePalette ) ;
	bin.SetAttrIntegerAs( L"shadowmapping", m_maxShadowmapping ) ;
	bin.SetAttrIntegerAs( L"morphing", m_enabledMorphing ? 1 : 0 ) ;
	bin.SetAttrIntegerAs( L"instancing", m_enabledInstancing ? 1 : 0 ) ;
	return	sglErrSuccess ;
}

// シェーダープログラム復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::LoadProgramBinary
	( const S3DShaderBinary& bin,
		SGLOpenGLShaderProgram::CompileListener * pListener )
{
	SGLError	err =
		SGLOpenGLShaderProgram::LoadProgramBinary( bin, pListener ) ;
	if ( err )
	{
		return	err ;
	}
	m_maxLightCount =
		(size_t) bin.GetAttrIntegerAs( L"max_light", MAX_LIGHT_COUNT ) ;
	m_maxBonePalette =
		(size_t) bin.GetAttrIntegerAs( L"max_bone", MAX_BONE_PALETTE ) ;
	m_maxShadowmapping =
		(size_t) bin.GetAttrIntegerAs( L"shadowmapping", MAX_SHADOWMAPPING ) ;
	m_enabledMorphing =
		(bin.GetAttrIntegerAs( L"morphing", true ) != 0) ;
	m_enabledInstancing =
		(bin.GetAttrIntegerAs( L"instancing", true ) != 0) ;
	//
	GetShaderVariableLocations() ;
	//
	return	sglErrSuccess ;
}

// シェーダー属性／ユニフォーム位置取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::GetShaderVariableLocations( void )
{
	//
	// シェーダー属性位置取得
	//
	Location	locAttr[] =
	{
		{	&a_vVertexPosition,			"a_vVertexPosition"	},
		{	&a_vVertexNormal,			"a_vVertexNormal"	},
		{	&a_vVertexMappingX,			"a_vVertexMappingX"	},
		{	&a_vVertexMappingY,			"a_vVertexMappingY"	},
		{	&a_vTextureCoord,			"a_vTextureCoord"	},
		{	&a_vVertexMulColor,			"a_vVertexMulColor"	},
		{	&a_vVertexAddColor,			"a_vVertexAddColor"	},
		{	&a_vMorphPosition,			"a_vMorphPosition"	},
		{	&a_vMorphNormal,			"a_vMorphNormal"	},
		{	&a_vMorphMappingX,			"a_vMorphMappingX"	},
		{	&a_vMorphMappingY,			"a_vMorphMappingY"	},
		{	&a_vMorphTextureCoord,		"a_vMorphTextureCoord"	},
		{	&a_vMorphMulColor,			"a_vMorphMulColor"	},
		{	&a_vMorphAddColor,			"a_vMorphAddColor"	},
		{	&a_vVertexBoneWeight[0],	"a_vVertexBoneWeight0"	},
		{	&a_vVertexBoneWeight[1],	"a_vVertexBoneWeight4"	},
		{	&a_vVertexBoneWeight[2],	"a_vVertexBoneWeight8"	},
		{	&a_vVertexBoneWeight[3],	"a_vVertexBoneWeight12"	},
		{	&a_matInstancingModelView[0],"a_matInstancingModelView0"	},
		{	&a_matInstancingModelView[1],"a_matInstancingModelView1"	},
		{	&a_matInstancingModelView[2],"a_matInstancingModelView2"	},
		{	&a_vInstancingMulColor,		"a_vInstancingMulColor"	},
		{	&a_vInstancingAddColor,		"a_vInstancingAddColor"	},
	} ;
	const size_t	countAttr = sizeof(locAttr) / sizeof(locAttr[0]) ;
	GetAttributeLocations( &locAttr[0], countAttr ) ;
	//
	// シェーダーユニフォーム位置取得
	//
	Location	locUniform[] =
	{
		// 変換行列
		{	&u_mat4PerspectiveView,		"u_mat4PerspectiveView"	},
		{	&u_mat4CameraView,			"u_mat4CameraView"	},
		{	&u_mat3CameraViewForNormal,	"u_mat3CameraViewForNormal"	},
		{	&u_mat4ICameraView,			"u_mat4ICameraView"	},
		{	&u_mat3ICameraViewForNormal,"u_mat3ICameraViewForNormal"	},
		{	&u_mat4ModelView,			"u_mat4ModelView"	},
		{	&u_mat3ModelViewForNormal,	"u_mat3ModelViewForNormal"	},
		{	&u_fpInverseNormal,			"u_fpInverseNormal"	},
		{	&u_fpBorderOffset,			"u_fpBorderOffset"	},
		{	&u_fpMorphApplication,		"u_fpMorphApplication"	},
		// ボーン
		{	&u_nBoneCount,				"u_nBoneCount"	},
		{	&u_mat3BoneRotation,		"u_mat3BoneRotation"	},
		{	&u_vBoneTranslate,			"u_vBoneTranslate"	},
		// 頂点テクスチャ
		{	&u_samplerVertex,			"u_samplerVertex"	},
		{	&u_vVertexTextureScale,		"u_vVertexTextureScale"	},
		{	&u_bVertexMorphing,			"u_bVertexMorphing"	},
		{	&u_bVertexBoneInstance,		"u_bVertexBoneInstance"	},
		{	&u_yVertexMorphingFirst,	"u_yVertexMorphingFirst"	},
		{	&u_yNormalMorphingFirst,	"u_yNormalMorphingFirst"	},
		{	&u_yVertexMorphingStride,	"u_yVertexMorphingStride"	},
		{	&u_yVertexMorphingInstance,	"u_yVertexMorphingInstance"	},
		{	&u_xVertexMorphingInstanceStride,	"u_xVertexMorphingInstanceStride"	},
		{	&u_yVertexBoneWeight,		"u_yVertexBoneWeight"	},
		{	&u_yVertexBoneIndex,		"u_yVertexBoneIndex"	},
		{	&u_yVertexBoneMatrix,		"u_yVertexBoneMatrix"	},
		{	&u_yVertexBoneMatrixStride,	"u_yVertexBoneMatrixStride"	},
		{	&u_yVertexExAttrElements,	"u_yVertexExAttrElements"	},
		{	&u_xVertexExAttrStride,		"u_xVertexExAttrStride"	},
		// 光源情報
		{	&u_vLightAmbientColor,		"u_vLightAmbientColor"	},
		{	&u_vLightAmbientColorMul,	"u_vLightAmbientColorMul"	},
		{	&u_countLight,				"u_countLight"	},
		{	&u_typeLighting,			"u_typeLighting"	},
		{	&u_vLightColor,				"u_vLightColor"	},
		{	&u_fpLightBrightness,		"u_fpLightBrightness"	},
		{	&u_fpLightAttenuationPower,	"u_fpLightAttenuationPower"	},
		{	&u_vLightPosition,			"u_vLightPosition"	},
		{	&u_vLightDirection,			"u_vLightDirection"	},
		{	&u_fpLightAngle,			"u_fpLightAngle"	},
		{	&u_fpLightGradation,		"u_fpLightGradation"	},
		// 疑似フォッグ
		{	&u_bEnableFog,				"u_bEnableFog"	},
		{	&u_rgbFogColor,				"u_rgbFogColor"	},
		{	&u_zFogNear,				"u_zFogNear"	},
		{	&u_zFogDistance,			"u_zFogDistance"	},
		// シャドウマッピング
		{	&u_iEnableShadowmap,		"u_iEnableShadowmap"	},
		{	&u_mat4PerspectiveShadowmap,"u_mat4PerspectiveShadowmap"	},
		{	&u_mat4ModelViewShadowmap,	"u_mat4ModelViewShadowmap"	},
		{	&u_samplerShadowmap,		"u_samplerShadowmap"	},
		{	&u_iShadowmapDepthLayer,	"u_iShadowmapDepthLayer"	},
		{	&u_vShadowmapUnit,			"u_vShadowmapUnit"	},
		{	&u_fpShadowmapFixErrorGap,	"u_fpShadowmapFixErrorGap"	},
		{	&u_fpShadowmapVarErrorGap,	"u_fpShadowmapVarErrorGap"	},
		// 色効果
		{	&u_vEffectMulColor,			"u_vEffectMulColor"	},
		{	&u_vEffectAddColor,			"u_vEffectAddColor"	},
		{	&u_fpEffectAlpha,			"u_fpEffectAlpha"	},
		// 表面属性
		{	&u_bMaterialShading,		"u_bMaterialShading"	},
		{	&u_bMaterialToon,			"u_bMaterialToon"		},
		{	&u_bMaterialDoubleSide,		"u_bMaterialDoubleSide"	},
		{	&u_iMaterialTriming,		"u_iMaterialTriming"	},
		{	&u_bMaterialNoFogEffect,	"u_bMaterialNoFogEffect"	},
		{	&u_bMaterialVertexAlpha,	"u_bMaterialVertexAlpha"	},
		{	&u_vMaterialMulColor,		"u_vMaterialMulColor"	},
		{	&u_vMaterialAddColor,		"u_vMaterialAddColor"	},
		{	&u_vMaterialMulShade,		"u_vMaterialMulShade"	},
		{	&u_vMaterialAddShade,		"u_vMaterialAddShade"	},
		{	&u_vMaterialSpecularColor,	"u_vMaterialSpecularColor"	},
		{	&u_fMaterialAmbient,		"u_fMaterialAmbient"	},
		{	&u_fMaterialDiffusion,		"u_fMaterialDiffusion"	},
		{	&u_fMaterialBackDiffusion,	"u_fMaterialBackDiffusion"	},
		{	&u_fMaterialSpecular,		"u_fMaterialSpecular"	},
		{	&u_fMaterialSpecularPow,	"u_fMaterialSpecularPow"	},
		{	&u_fMaterialAlpha,			"u_fMaterialAlpha"	},
		{	&u_fMaterialDeepness,		"u_fMaterialDeepness"	},
		{	&u_fMaterialDeepnessPow,	"u_fMaterialDeepnessPow"	},
		{	&u_fMaterialReflection,		"u_fMaterialReflection"	},
		{	&u_fMaterialEmission,		"u_fMaterialEmission"	},
		{	&u_vMaterialBackLightMul,	"u_vMaterialBackLightMul"	},
		{	&u_vMaterialBackLightAdd,	"u_vMaterialBackLightAdd"	},
		{	&u_cosShadeCoefficient,		"u_cosShadeCoefficient"	},
		{	&u_fpToonShadeThreshold,	"u_fpToonShadeThreshold"	},
		{	&u_fpToonShadeBrightness,	"u_fpToonShadeBrightness"	},
		{	&u_fMaterialRimLight,		"u_fMaterialRimLight"	},
		{	&u_fMaterialRimDeepness,	"u_fMaterialRimDeepness"	},
		{	&u_vMaterialRimColor,		"u_vMaterialRimColor"	},
		// 通常テクスチャ
		{	&u_bMaterialTexture,		"u_bMaterialTexture"	},
		{	&u_samplerMaterialTexture,	"u_samplerMaterialTexture"	},
		{	&u_vMaterialTextureScale,	"u_vMaterialTextureScale"	},
		{	&u_vMaterialTextureBase,	"u_vMaterialTextureBase"	},
		// 発光テクスチャ
		{	&u_fpLuminousTexture,		"u_fpLuminousTexture"	},
		{	&u_samplerLuminousTexture,	"u_samplerLuminousTexture"	},
		{	&u_vLuminousTextureScale,	"u_vLuminousTextureScale"	},
		{	&u_vLuminousTextureBase,	"u_vLuminousTextureBase"	},
		// 環境マッピング
		{	&u_typeEnvironmentMapping,		"u_typeEnvironmentMapping"	},
		{	&u_typeEnvironmentRefraction,	"u_typeEnvironmentRefraction"	},
		{	&u_fpRefractionRatio,			"u_fpRefractionRatio"	},
		{	&u_fpRefractionParam,			"u_fpRefractionParam"	},
		{	&u_samplerEnvironmentCube,		"u_samplerEnvironmentCube"	},
		{	&u_samplerEnvironmentMapping,	"u_samplerEnvironmentMapping"	},
		{	&u_samplerViewportMapping,		"u_samplerViewportMapping"	},
		{	&u_samplerViewportDepth,		"u_samplerViewportDepth"	},
		{	&u_vViewportUnit,				"u_vViewportUnit"	},
		{	&u_mat3EnvironmentMapping,		"u_mat3EnvironmentMapping"	},
		{	&u_vEnvMapingTextureScale,		"u_vEnvMapingTextureScale"	},
		{	&u_vEnvMapingTextureBase,		"u_vEnvMapingTextureBase"	},
		// 法線テクスチャ
		{	&u_fpNormalTexture,				"u_fpNormalTexture"	},
		{	&u_samplerNormalTexture,		"u_samplerNormalTexture"	},
		{	&u_vNormalTextureScale,			"u_vNormalTextureScale"	},
		{	&u_vNormalTextureBase,			"u_vNormalTextureBase"	},
		// 標高テクスチャ
		{	&u_fpBumpHeight,				"u_fpBumpHeight"	},
		{	&u_samplerHeightTexture,		"u_samplerHeightTexture"	},
		{	&u_vHeightTextureScale,			"u_vHeightTextureScale"	},
		{	&u_vHeightTextureBase,			"u_vHeightTextureBase"	},
		// αマッピング
		{	&u_bMaterialAlphaTexture,		"u_bMaterialAlphaTexture"	},
		{	&u_fpAlphaCoefficient,			"u_fpAlphaCoefficient"	},
		{	&u_fpAlphaBase,					"u_fpAlphaBase"	},
		{	&u_samplerAlphaMapping,			"u_samplerAlphaMapping"	},
		{	&u_vAlphaMapingTextureScale,	"u_vAlphaMapingTextureScale"	},
		{	&u_vAlphaMapingTextureBase,		"u_vAlphaMapingTextureBase"	},
		// スペキュラー・粗さ・反射率テクスチャ
		{	&u_bMaterialSpecularTexture,	"u_bMaterialSpecularTexture"	},
		{	&u_samplerSpecularTexture,		"u_samplerSpecularTexture"	},
		{	&u_vSpecularTextureScale,		"u_vSpecularTextureScale"	},
		{	&u_vSpecularTextureBase,		"u_vSpecularTextureBase"	},
		// 大域ライトマップAOテクスチャ
		{	&u_bMaterialGlobalAOTexture,	"u_bMaterialGlobalAOTexture"	},
		{	&u_samplerGlobalAOTexture,		"u_samplerGlobalAOTexture"	},
	} ;
	const size_t
		countUniform = sizeof(locUniform) / sizeof(locUniform[0]) ;
	GetUniformLocations( &locUniform[0], countUniform ) ;
}

// プログラムを現在のコンテキストに設定しuniformを初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::InitializeShader( void )
{
	if ( m_glProgram == 0 )
	{
		return	sglErrFailed ;
	}
	SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
	ESLAssert( pOpenGL != nullptr ) ;
	ESLAssert( pOpenGL == SGLOpenGLContext::GetCurrentGLContext() ) ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->AttachShaderProgram( this ) ;
	}
	else
	{
		glUseProgram( m_glProgram ) ;
		if ( !SGLOpenGLContext::VerifyError( "glUseProgram" ) )
		{
			return	sglErrFailed ;
		}
	}
	//
	// 属性ポインタ
	//
	m_enabledVertexAttr = false ;
	m_enabledVertex4Morphing = false ;
	m_enabledInstancingAttr = false ;
	//
	for ( int i = 0; i < BONE_PALETTE_ATTRS; i ++ )
	{
		DisableVertexAttribArray( a_vVertexBoneWeight[i] ) ;
		m_enabledVertexBoneWeight[i] = false ;
	}
	//
	// 変換行列
	//
	S4DMatrix	matI( 1.0, 1.0, 1.0, 1.0 ) ;
	SetPerspectiveMatrix( matI ) ;
	SetCameraViewMatrix( matI ) ;
	SetModelViewMatrix( matI ) ;
	//
	glUniform1f( u_fpBorderOffset, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBorderOffset)" ) ;
	//
	// モーフィング
	//
	glUniform1f( u_fpMorphApplication, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpMorphApplication)" ) ;
	m_fpMorphApplication = 0.0f ;
	//
	// ボーン
	//
	glUniform1i( u_nBoneCount, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_nBoneCount)" ) ;
	m_nBoneCount = 0 ;
	//
	if ( m_maxBonePalette > 0 )
	{
		GLfloat	fpBoneMatrix[MAX_BONE_PALETTE][3][3] ;
		GLfloat	fpBoneTrans[MAX_BONE_PALETTE][3] ;
		eslFillMemory( &fpBoneMatrix[0][0][0], 0, sizeof(fpBoneMatrix) ) ;
		eslFillMemory( &fpBoneTrans[0][0], 0, sizeof(fpBoneTrans) ) ;
		for ( int i = 0; i < MAX_BONE_PALETTE; i ++ )
		{
			fpBoneMatrix[i][0][0] = 1.0f ;
			fpBoneMatrix[i][1][1] = 1.0f ;
			fpBoneMatrix[i][2][2] = 1.0f ;
		}
		glUniformMatrix3fv
			( u_mat3BoneRotation,
				(GLsizei) m_maxBonePalette,
				GL_FALSE, &fpBoneMatrix[0][0][0] ) ;
		SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3BoneRotation)" ) ;
		//
		glUniform3fv
			( u_vBoneTranslate,
				(GLsizei) m_maxBonePalette, &fpBoneTrans[0][0] ) ;
		SGLOpenGLContext::VerifyError( "glUniform3fv(u_vBoneTranslate)" ) ;
	}
	//
	// 頂点テクスチャ
	//
	m_pVertexTexture = nullptr ;
	m_iVertexTexture = -1 ;
	//
	if ( m_enabledVTInstancing )
	{
		glUniform1i( u_samplerVertex, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerVertex)" ) ;
		//
		glUniform1i( u_bVertexMorphing, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexMorphing)" ) ;
		//
		glUniform1i( u_bVertexBoneInstance, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexBoneInstance)" ) ;
	}
	//
	// 色効果
	//
	glUniform3f( u_vEffectMulColor, 1.0f, 1.0f, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectMulColor)" ) ;
	glUniform3f( u_vEffectAddColor, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectAddColor)" ) ;
	m_colorUniformEffect.rgbMul = 0x00FFFFFF ;
	m_colorUniformEffect.rgbAdd = 0 ;
	//
	glUniform1f( u_fpEffectAlpha, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpEffectAlpha)" ) ;
	m_nUniformTransparency = 0 ;
	//
	// 光源
	//
	memset( &m_bufLight, 0, sizeof(LIGHT_BUFFER) ) ;
	glUniform3f( u_vLightAmbientColor, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vLightAmbientColor)" ) ;
	//
	glUniform3f( u_vLightAmbientColorMul, 1.0f, 1.0f, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vLightAmbientColorMul)" ) ;
	//
	glUniform1i( u_countLight, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_countLight)" ) ;
	//
	glUniform1iv( u_typeLighting, MAX_ALL_LIGHT_COUNT, m_bufLight.typeLighting ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_typeLighting)" ) ;
	//
	glUniform1i( u_bEnableFog, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bEnableFog)" ) ;
	//
	for ( int i = 0; i < MAX_SHADOWMAPPING; i ++ )
	{
		m_bufShadowmap.iLights[i] = -1 ;
	}
	glUniform1iv( u_iEnableShadowmap, MAX_SHADOWMAPPING, m_bufShadowmap.iLights ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_iEnableShadowmap)" ) ;
	//
	// シェーディング
	//
	glUniform1i( u_bMaterialShading, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialShading)" ) ;
	m_bMaterialShading = 0 ;
	//
	glUniform1i( u_bMaterialToon, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialToon)" ) ;
	m_bMaterialToon = 0 ;
	//
	glUniform1i( u_iMaterialTriming, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_iMaterialTriming)" ) ;
	m_iMaterialTriming = 0 ;
	//
	glUniform1i( u_bMaterialNoFogEffect, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialNoFogEffect)" ) ;
	m_bMaterialNoFogEffect = 0 ;
	//
	glUniform1i( u_bMaterialVertexAlpha, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialVertexAlpha)" ) ;
	m_bMaterialVertexAlpha = 0 ;
	//
	glUniform1i( u_bMaterialDoubleSide, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialDoubleSide)" ) ;
	m_bMaterialDoubleSide = 0 ;
	//
	// 表面色
	//
	glUniform3f( u_vMaterialMulColor, 1.0f, 1.0f, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialMulColor)" ) ;
	m_saLastMaterial.colorBase.rgbMul = 0x00FFFFFF ;
	//
	glUniform3f( u_vMaterialAddColor, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialAddColor)" ) ;
	m_saLastMaterial.colorBase.rgbAdd = 0 ;
	//
	glUniform3f( u_vMaterialMulShade, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialMulShade)" ) ;
	m_saLastMaterial.colorShade.rgbMul = 0 ;
	//
	glUniform3f( u_vMaterialAddShade, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialAddShade)" ) ;
	m_saLastMaterial.colorShade.rgbAdd = 0 ;
	//
	glUniform3f( u_vMaterialSpecularColor, 1.0f, 1.0f, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialSpecularColor)" ) ;
	m_saLastMaterial.rgbSpecularColor = 0x00FFFFFF ;
	//
	// 表面属性
	//
	glUniform1f( u_fMaterialAmbient, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialAmbient)" ) ;
	m_saLastMaterial.nAmbient = 0 ;
	//
	glUniform1f( u_fMaterialDiffusion, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDiffusion)" ) ;
	m_saLastMaterial.nDiffusion = 0x100 ;
	//
	glUniform1f( u_fMaterialBackDiffusion, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialBackDiffusion)" ) ;
	m_saLastMaterial.nBackDiffusion = 0 ;
	//
	glUniform1f( u_fMaterialSpecular, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialSpecular)" ) ;
	m_saLastMaterial.nSpecular = 0 ;
	//
	glUniform1f( u_fMaterialSpecularPow, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialSpecularPow)" ) ;
	m_saLastMaterial.nSpecularSize = 0x100 ;
	//
	glUniform1f( u_fMaterialAlpha, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialAlpha)" ) ;
	m_saLastMaterial.nTransparency = 0 ;
	//
	glUniform1f( u_fMaterialDeepness, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDeepness)" ) ;
	m_saLastMaterial.nDeepness = 0 ;
	//
	glUniform1f( u_fMaterialDeepnessPow, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDeepnessPow)" ) ;
	m_saLastMaterial.nDeepnessPower = 0x100 ;
	//
	glUniform1f( u_fMaterialReflection, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialReflection)" ) ;
	m_saLastMaterial.nReflection = 0 ;
	//
	glUniform1f( u_fMaterialEmission, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialEmission)" ) ;
	m_saLastMaterial.nEmission = 0 ;
	//
	glUniform3f( u_vMaterialBackLightMul, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_vMaterialBackLightMul)" ) ;
	glUniform3f( u_vMaterialBackLightAdd, 0.0f, 0.0f, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_vMaterialBackLightAdd)" ) ;
	m_saLastMaterial.fpBackLight = 0.0f ;
	m_saLastMaterial.colorBackLight.rgbMul = 0 ;
	m_saLastMaterial.colorBackLight.rgbAdd = 0 ;
	//
	GLfloat	cosShadeCorfficient[2] ;
	cosShadeCorfficient[0] = 1.0f ;
	cosShadeCorfficient[1] = 0.0f ;
	glUniform1fv( u_cosShadeCoefficient, 2, cosShadeCorfficient ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_cosShadeCoefficient)" ) ;
	m_saLastMaterial.cosShadeThreshold = 0.0f ;
	//
	glUniform1f( u_fpToonShadeThreshold, 0.5f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpToonShadeThreshold)" ) ;
	m_saLastMaterial.fpToonShadeThreshold = 0.5f ;
	//
	glUniform1f( u_fpToonShadeBrightness, 0.5f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpToonShadeBrightness)" ) ;
	m_saLastMaterial.fpToonShadeBrightness = 0.5f ;
	//
	glUniform1f( u_fMaterialRimLight, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialRimLight)" ) ;
	m_saLastMaterial.fpRimLight = 0.0f ;
	//
	glUniform1f( u_fMaterialRimDeepness, 0.1f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialRimDeepness)" ) ;
	m_saLastMaterial.fpRimLightDeepness = 0.1f ;
	//
	glUniform3f( u_vMaterialRimColor, 1.0f, 1.0f, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialRimColor)" ) ;
	m_saLastMaterial.rgbRimLightColor = 0x00FFFFFF ;
	//
	// テクスチャ
	//
	glUniform1i( u_bMaterialTexture, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialTexture)" ) ;
	m_txiMaterialTexture.pImage = nullptr ;
	m_txiMaterialTexture.pglTexture = nullptr ;
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	//
	GLenum	glTarget = GL_TEXTURE_2D ;
	m_pOpenGL->IsBindingTexture( 0, glTarget ) ;
	glBindTexture( glTarget, 0 ) ;
	SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
	m_pOpenGL->SetBindTextureInfo( 0, nullptr ) ;
	//
	// 発光テクスチャ
	//
	glUniform1f( u_fpLuminousTexture, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(m_fpLuminousTexture)" ) ;
	m_txiMaterialLuminous.pImage = nullptr ;
	m_txiMaterialLuminous.pglTexture = nullptr ;
	m_fpLuminousTexture = 0.0f ;
	//
	// 環境マッピング
	//
	glUniform1i( u_typeEnvironmentMapping, ENV_MAPPING_NOTHING ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentMapping)" ) ;
	glUniform1i( u_typeEnvironmentRefraction, ENV_MAPPING_NOTHING ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentRefraction)" ) ;
	glUniform1f( u_fpRefractionRatio, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpRefractionRatio)" ) ;
	glUniform1f( u_fpRefractionParam, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpRefractionParam)" ) ;
	m_txiMaterialEnvironment.pImage = nullptr ;
	m_txiMaterialEnvironment.pglTexture = nullptr ;
	m_txiMaterialRefraction.pImage = nullptr ;
	m_txiMaterialRefraction.pglTexture = nullptr ;
	m_txiMaterialViewportDepth.pImage = nullptr ;
	m_txiMaterialViewportDepth.pglTexture = nullptr ;
	m_fpRefractionRatio = 0.0f ;
	m_fpRefractionParam = 1.0f ;
	//
	GLfloat	m3x3[3][3] ;
	m_matEnvironmentMapping.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	for ( int i = 0; i < 3; i ++ )
	{
		m3x3[i][0] = m_matEnvironmentMapping.m[0][i] ;
		m3x3[i][1] = m_matEnvironmentMapping.m[1][i] ;
		m3x3[i][2] = m_matEnvironmentMapping.m[2][i] ;
	}
	glUniformMatrix3fv
		( u_mat3EnvironmentMapping, 1, GL_FALSE, &m3x3[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3EnvironmentMapping)" ) ;
	//
	// 法線テクスチャ
	//
	glUniform1f( u_fpNormalTexture, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpNormalTexture)" ) ;
	m_txiMaterialNormal.pImage = nullptr ;
	m_txiMaterialNormal.pglTexture = nullptr ;
	m_fpNormalTexture = 0.0f ;
	//
	// 標高テクスチャ
	//
	glUniform1f( u_fpBumpHeight, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBumpHeight)" ) ;
	m_txiMaterialHeight.pImage = nullptr ;
	m_txiMaterialHeight.pglTexture = nullptr ;
	m_fpHeightTexture = 0.0f ;
	//
	// αマッピング
	//
	glUniform1i( u_bMaterialAlphaTexture, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialAlphaTexture)" ) ;
	glUniform1f( u_fpAlphaCoefficient, 1.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpAlphaCoefficient)" ) ;
	glUniform1f( u_fpAlphaBase, 0.0f ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpAlphaBase)" ) ;
	//
	m_txiMaterialAlpha.pImage = nullptr ;
	m_txiMaterialAlpha.pglTexture = nullptr ;
	m_fpAlphaCoefficient = 1.0 ;
	m_fpAlphaBase = 0.0 ;
	//
	// スペキュラー・粗さ・反射率テクスチャ
	//
	glUniform1i( u_bMaterialSpecularTexture, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialSpecularTexture)" ) ;
	m_txiMaterialSpecular.pImage = nullptr ;
	m_txiMaterialSpecular.pglTexture = nullptr ;
	//
	// 大域ライトマップAOテクスチャ
	//
	glUniform1i( u_bMaterialGlobalAOTexture, 0 ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialGlobalAOTexture)" ) ;
	m_txiMaterialLightMapAO.pImage = nullptr ;
	m_txiMaterialLightMapAO.pglTexture = nullptr ;
	//
	return	sglErrSuccess ;
}

// デフォルトのシェーダー設定を取得する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::GetDefaultShaderFeatures
	( S3DRenderDevice::ShaderSourceInfo& srcinf )
{
	srcinf.flagsFeature = S3DRenderDevice::shaderUseStandardShader
							| S3DRenderDevice::shaderLimitBone
							| S3DRenderDevice::shaderLimitShadowmap ;
	srcinf.flagsFeature |= S3DRenderDevice::shaderUseLuminousTexture ;
	if ( !m_disable_environment_mapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseEnvMapping ;
	}
	if ( !m_disable_environment_cubemapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseCubemap ;
	}
	if ( !m_disable_environment_spheremapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseEnvSphere ;
	}
	if ( !m_disable_environment_viewport )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseRefViewport ;
	}
	if ( !m_disable_environment_refraction )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseRefraction ;
	}
	if ( !m_disable_normal_mapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseNormalTexture ;
	}
	if ( !m_disable_height_mapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseHeightTexture ;
	}
	if ( !m_disable_specular_mapping )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseSpecularTexture ;
	}
	if ( !m_disable_global_ao_lightmap )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderUseGlobalAOTexture ;
	}
	if ( m_limit_light_count < 8 )
	{
		srcinf.flagsFeature |= S3DRenderDevice::shaderLimitLight ;
	}
	srcinf.nBoneLimit = (uint32_t) m_limit_bone_count ;
	srcinf.nLightLimit = (uint32_t) m_limit_light_count ;
	srcinf.nShadowmapLimit = (uint32_t) m_limit_shadow_map_count ;
}

// テクスチャ補完有効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::EnableTextureSmoothing( bool fEnable )
{
	m_fEnableTextureSmoothing = fEnable ;
}

// テクスチャ補完は有効か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsEnabledTextureSmoothing( void ) const
{
	return	m_fEnableTextureSmoothing ;
}

// Zバッファへの書き込み有効
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::EnableWriteDepth( bool fEnable )
{
	m_fDisableWriteDepth = !fEnable ;
}

// Zバッファへの書き込み有効か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsEnabledWriteDepth( void ) const
{
	return	!m_fDisableWriteDepth ;
}

// トゥーンシェーダーを強制的に有効にする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::ForceToonShader( bool fToon )
{
	m_fShaderForceToon = fToon ;
}

// トゥーンシェーダーが強制されているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsForcedToonShader( void ) const
{
	return	m_fShaderForceToon ;
}

// 表面オフセット描画を有効にする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::EnableMeshSurfaceOffset( bool fOffset )
{
	m_fShaderSurfaceOffset = fOffset ;
}

// 表面オフセット描画が有効に設定されているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsEnabledMeshSurfaceOffset( void ) const
{
	return	m_fShaderSurfaceOffset ;
}

// 輪郭表示（オフセット描画）を強制的に有効にする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::ForceBorderShader( bool fBorder )
{
	m_fShaderForceBorder = fBorder ;
}

// 輪郭表示（オフセット描画）が強制されているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsForcedBorderShader( void ) const
{
	return	m_fShaderForceBorder ;
}

// 輪郭表示（オフセット描画）を強制的に無効にする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::ForceNoBorderShader( bool fNoBorder )
{
	m_fShaderNoDrawBorder = fNoBorder ;
}

// 輪郭表示（オフセット描画）が無効にされているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsForcedNoBorderShader( void ) const
{
	return	m_fShaderNoDrawBorder ;
}

// 輪郭描画の色を設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetOffsetBorderColor( uint32_t rgb )
{
	m_rgbBorderColor.ui32 = rgb ;
}

// 輪郭描画の色を取得する
//////////////////////////////////////////////////////////////////////////////
const SGLPalette& SGLOpenGLDefaultShader::GetOffsetBorderColor( void ) const
{
	return	m_rgbBorderColor ;
}

// 輪郭描画の太さ係数を設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	m_fpBorderCoefficient[0] = a ;
	m_fpBorderCoefficient[1] = b ;
}

// カリング処理の設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetFaceCullingOperation
	( S3DRenderBufferInterface::FaceCullingOperation faceCulling )
{
	if ( m_faceCulling != faceCulling )
	{
		m_psaLastMaterial = nullptr ;
		m_faceCulling = faceCulling ;
	}
}

// ｚバッファ処理の設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetDepthMaskOperation
	( S3DRenderBufferInterface::DepthMaskOperation depthMask )
{
	if ( m_depthMask != depthMask )
	{
		m_psaLastMaterial = nullptr ;
		m_depthMask = depthMask ;
	}
}

// 描画処理の設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetBlendOperation
	( S3DRenderBufferInterface::BlendOperation blendOp )
{
	if ( m_blendOperation != blendOp )
	{
		m_psaLastMaterial = nullptr ;
		m_blendOperation = blendOp ;
	}
}

// 拡散反射光を発光出力する（マルチレンダーターゲット出力用）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetEmisiveTarget( bool fEmisiveTarget )
{
	m_fShaderEmisiveTarget = fEmisiveTarget ;
}

// 拡散反射光を発光出力するか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::IsEmisiveTarget( void ) const
{
	return	m_fShaderEmisiveTarget ;
}

// シェーダープログラムが設定された（変更された）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::OnChangedProgram( void )
{
	m_psaLastMaterial = nullptr ;
	m_flagsLastMaterialShading = 0 ;
	//
	DisableAllVertexPointer() ;
	//
	if ( m_pOpenGL->m_pBindingVBO )
	{
		glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
		//
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
		//
		m_pOpenGL->m_pBindingVBO = nullptr ;
		m_pOpenGL->m_pRsrcOfVBO = nullptr ;
	}

	SGLOpenGLShaderProgram::OnChangedProgram() ;
}

// シェーダープログラムが別のプログラムに変更される
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::OnChangingProgram( void )
{
	SGLOpenGLShaderProgram::OnChangingProgram() ;
}

// ユニフォーム値更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::UpdateCustomUniform( bool flagForceUpdate )
{
	SGLOpenGLShaderProgram::UpdateCustomUniform( flagForceUpdate ) ;
}

// ユーザーテクスチャ番号 → OpenGL バインドテクスチャ番号
//////////////////////////////////////////////////////////////////////////////
int SGLOpenGLDefaultShader::GLTextureNumAtUserTexture( size_t iUserTexture ) const
{
	const size_t	maxUserTexture =
						sizeof(m_gl_iMaterialTextures)/sizeof(m_gl_iMaterialTextures[0]) ;
	ESLAssert( iUserTexture < maxUserTexture ) ;
	return	(iUserTexture < maxUserTexture) ? m_gl_iMaterialTextures[iUserTexture] : 0 ;
}

// ユニフォーム指標定義
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::RegisterCustomUniform
	( const wchar_t * pszUniformID,
		S3DCustomShader::UniformType type, size_t nCount )
{
	SGLOpenGLShaderProgram::RegisterCustomUniform( pszUniformID, type, nCount ) ;
}

// Flush 処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::OnFlushContext( void )
{
	DisableAllVertexPointer() ;
	//
	if ( m_pOpenGL->m_pBindingVBO )
	{
		glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
		//
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
		//
		m_pOpenGL->m_pBindingVBO = nullptr ;
		m_pOpenGL->m_pRsrcOfVBO = nullptr ;
	}
	//
	SetMaterial( nullptr ) ;

	SGLOpenGLShaderProgram::OnFlushContext() ;
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetPerspectiveMatrix( const S4DMatrix& mat4 )
{
	glUniformMatrix4f( u_mat4PerspectiveView, GL_TRUE, mat4 ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4PerspectiveView)" ) ;
	//
	m_mat4Perspective = mat4 ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetProjectionScreen
	( float32_t xScreen, float32_t yScreen, float32_t zScreen )
{
	m_vProjectScreen.x = xScreen ;
	m_vProjectScreen.y = yScreen ;
	m_vProjectScreen.z = zScreen ;
}

// カメラ変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetCameraViewMatrix( const S4DMatrix& mat4 )
{
	glUniformMatrix4f( u_mat4CameraView, GL_TRUE, mat4 ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4CameraView)" ) ;
	//
	GLfloat	m3x3[3][3] ;
	for ( int i = 0; i < 3; i ++ )
	{
		m3x3[i][0] = mat4.m[0][i] ;
		m3x3[i][1] = mat4.m[1][i] ;
		m3x3[i][2] = mat4.m[2][i] ;
	}
	glUniformMatrix3fv( u_mat3CameraViewForNormal, 1, GL_FALSE, &m3x3[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3CameraViewForNormal)" ) ;
	//
	S4DMatrix	mat4Inv = mat4.Inverse() ;
	glUniformMatrix4f( u_mat4ICameraView, GL_TRUE, mat4Inv ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4ICameraView)" ) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		m3x3[i][0] = mat4Inv.m[0][i] ;
		m3x3[i][1] = mat4Inv.m[1][i] ;
		m3x3[i][2] = mat4Inv.m[2][i] ;
	}
	glUniformMatrix3fv( u_mat3ICameraViewForNormal, 1, GL_FALSE, &m3x3[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3ICameraViewForNormal)" ) ;
	//
	m_mat4Camera = mat4 ;
}

// モデル変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetModelViewMatrix
		( const S4DMatrix& mat4, bool fInverseNormal )
{
	glUniformMatrix4f( u_mat4ModelView, GL_TRUE, mat4 ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4ModelView)" ) ;
	m_mat4ModelView = mat4 ;
	//
	GLfloat	m3x3[3][3] ;
	for ( int i = 0; i < 3; i ++ )
	{
		m3x3[i][0] = mat4.m[0][i] ;
		m3x3[i][1] = mat4.m[1][i] ;
		m3x3[i][2] = mat4.m[2][i] ;
	}
	glUniformMatrix3fv( u_mat3ModelViewForNormal, 1, GL_FALSE, &m3x3[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3ModelViewForNormal)" ) ;
	//
	m_fpInverseNormal = (fInverseNormal ? -1.0f : 1.0f) ;
	glUniform1f( u_fpInverseNormal, m_fpInverseNormal ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_fpInverseNormal)" ) ;
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	//
	// 光源情報を正規化
	//
	memset( &m_bufLight, 0, sizeof(LIGHT_BUFFER) ) ;
	m_tableLightID.SetLength( countLight ) ;
	//
	m_bufLight.vAmbientMul[0] = 1.0f ;
	m_bufLight.vAmbientMul[1] = 1.0f ;
	m_bufLight.vAmbientMul[2] = 1.0f ;
	//
	size_t	i ;
	size_t	iLight = 0 ;
	GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
	for ( i = 0; (i < countLight) && (iLight < MAX_LIGHT_COUNT); i ++ )
	{
		const S3DLightEntry&	le = pLights[i] ;
		S3DLightEntry			leTemp ;
		m_tableLightID.SetAt( i, -1 ) ;
		switch ( le.typeLight & lightTypeMask )
		{
		case	lightTypeAmbient:
			m_bufLight.vAmbientColor[0] += (GLfloat) (le.rgbColor.argb.Red * fp1by255) ;
			m_bufLight.vAmbientColor[1] += (GLfloat) (le.rgbColor.argb.Green * fp1by255) ;
			m_bufLight.vAmbientColor[2] += (GLfloat) (le.rgbColor.argb.Blue * fp1by255) ;
			break ;
		case	lightTypeAmbientMul:
			m_bufLight.vAmbientMul[0] *= (GLfloat) (le.rgbColor.argb.Red * fp1by255) ;
			m_bufLight.vAmbientMul[1] *= (GLfloat) (le.rgbColor.argb.Green * fp1by255) ;
			m_bufLight.vAmbientMul[2] *= (GLfloat) (le.rgbColor.argb.Blue * fp1by255) ;
			break ;
		case	lightTypeVector:
		case	lightTypePoint:
		case	lightTypeSpot:
			leTemp = le ;
			leTemp.vecDirection.Normalize() ;
			m_bufLight.SetAt( iLight, leTemp ) ;
			m_tableLightID.SetAt( i, (int) iLight ) ;
			iLight ++ ;
			break ;
		}
	}
	m_bufLight.countLight = (GLint) iLight ;
	//
	#if	defined(__API_OPEN_GL_ES__)
	iLight = INDEX_LIGHT_FOG ;
	#endif
	for ( i = 0; (i < countLight) && (iLight < MAX_ALL_LIGHT_COUNT); i ++ )
	{
		const S3DLightEntry&	le = pLights[i] ;
		S3DLightEntry			leTemp ;
		switch ( le.typeLight & lightTypeMask )
		{
		case	lightTypeFog:
			leTemp = le ;
			leTemp.vecDirection.Normalize() ;
			m_bufLight.SetAt( iLight, leTemp ) ;
			iLight ++ ;
			break ;
		}
	}
	//
	// 光源情報を設定
	//
	glUniform3f
		( u_vLightAmbientColor,
			m_bufLight.vAmbientColor[0],
			m_bufLight.vAmbientColor[1], m_bufLight.vAmbientColor[2] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vLightAmbientColor)" ) ;
	//
	glUniform3f
		( u_vLightAmbientColorMul,
			m_bufLight.vAmbientMul[0],
			m_bufLight.vAmbientMul[1], m_bufLight.vAmbientMul[2] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_vLightAmbientColorMul)" ) ;
	//
	glUniform1i( u_countLight, m_bufLight.countLight ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_countLight)" ) ;
	//
	glUniform1iv( u_typeLighting, MAX_ALL_LIGHT_COUNT, m_bufLight.typeLighting ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_typeLighting)" ) ;
	//
	glUniform3fv( u_vLightColor, MAX_ALL_LIGHT_COUNT, &m_bufLight.vColor[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3fv(u_vLightColor)" ) ;
	//
	glUniform1fv( u_fpLightBrightness, MAX_ALL_LIGHT_COUNT, m_bufLight.fpBrightness ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpLightBrightness)" ) ;
	//
	glUniform1fv( u_fpLightAttenuationPower, MAX_ALL_LIGHT_COUNT, m_bufLight.fpAttenuationPower ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpLightAttenuationPower)" ) ;
	//
	glUniform3fv( u_vLightPosition, MAX_ALL_LIGHT_COUNT, &m_bufLight.vPosition[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3fv(u_vLightPosition)" ) ;
	//
	glUniform3fv( u_vLightDirection, MAX_ALL_LIGHT_COUNT, &m_bufLight.vDirection[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3fv(u_vLightDirection)" ) ;
	//
	glUniform1fv( u_fpLightAngle, MAX_ALL_LIGHT_COUNT, m_bufLight.fpAngle ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpLightAngle)" ) ;
	//
	glUniform1fv( u_fpLightGradation, MAX_ALL_LIGHT_COUNT, m_bufLight.fpGradation ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpLightGradation)" ) ;
	//
	// シャドウマッピング設定初期化
	//
	for ( int i = 0; i < MAX_SHADOWMAPPING; i ++ )
	{
		m_bufShadowmap.iLights[i] = -1 ;
	}
	glUniform1iv( u_iEnableShadowmap, MAX_SHADOWMAPPING, m_bufShadowmap.iLights ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_iEnableShadowmap)" ) ;
	//
	if ( m_pOpenGL->m_maxTextureImages > glTextureShadow1 )
	{
		GLenum	glTarget = GL_TEXTURE_2D ;
		m_pOpenGL->IsBindingTexture( glTextureShadow1, glTarget ) ;
		//
		glActiveTexture( GL_TEXTURE0 + glTextureShadow1 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE1)" ) ;
		//
		glBindTexture( glTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo( glTextureShadow1, nullptr ) ;
		//
		#if	!defined(__API_OPEN_GL_ES__)
//		glDisable( glTarget ) ;
//		SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
		#endif
		//
		glActiveTexture( GL_TEXTURE0 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	}
	if ( m_pOpenGL->m_maxTextureImages > glTextureShadow2 )
	{
		GLenum	glTarget = GL_TEXTURE_2D ;
		m_pOpenGL->IsBindingTexture( glTextureShadow2, glTarget ) ;
		//
		glActiveTexture( GL_TEXTURE0 + glTextureShadow2 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE1)" ) ;
		//
		glBindTexture( glTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo( glTextureShadow2, nullptr ) ;
		//
		#if	!defined(__API_OPEN_GL_ES__)
//		glDisable( glTarget ) ;
//		SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
		#endif
		//
		glActiveTexture( GL_TEXTURE0 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	}
}

// S3DLightEntry 変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::LIGHT_BUFFER::SetAt
					( size_t i, const S3DLightEntry& light )
{
	ESLAssert( i < MAX_ALL_LIGHT_COUNT ) ;
	GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
	typeLighting[i]			= light.typeLight & lightTypeMask ;
	vColor[i][0]			= (GLfloat) (light.rgbColor.argb.Red * fp1by255) ;
	vColor[i][1]			= (GLfloat) (light.rgbColor.argb.Green * fp1by255) ;
	vColor[i][2]			= (GLfloat) (light.rgbColor.argb.Blue * fp1by255) ;
	fpBrightness[i]			= light.fpBrightness ;
	fpAttenuationPower[i]	= light.fpAttenuationPower ;
	vPosition[i][0]			= light.vecPosition.x ;
	vPosition[i][1]			= light.vecPosition.y ;
	vPosition[i][2]			= light.vecPosition.z ;
	vDirection[i][0]		= light.vecDirection.x ;
	vDirection[i][1]		= light.vecDirection.y ;
	vDirection[i][2]		= light.vecDirection.z ;
	fpAngle[i]				= light.fpAngle ;
	fpGradation[i]			= light.fpGradation ;
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetShadowMaps
	( size_t nShadowCount,
		const S4DMatrix * pmat4ICameras,
		const uint32_t * pidLights,
		const S3DShadowMapInfo * pinfShadowMaps,
		SGLImageObject*const* ppShadowMapDepths,
		SGLImageObject*const* ppShadowMapColors )
{
	if ( !g_supports_opengl_1_3 
		|| (m_pOpenGL->m_maxTextureImages <= glTextureShadow1) )
	{
		return ;
	}
	if ( nShadowCount > MAX_SHADOWMAPPING )
	{
		nShadowCount = MAX_SHADOWMAPPING ;
	}
	if ( nShadowCount == 0 )
	{
		return ;
	}
	SGLImageRect	rectRefTexture( 0, 0, 1, 1 ) ;
	SGLOpenGLTextureBuffer::GLResource *
		pglTexture =
			SGLOpenGLTextureBuffer::CommitGLTextureRsrc
				( m_pOpenGL, ppShadowMapDepths[0], rectRefTexture ) ;
	if ( pglTexture == nullptr )
	{
		GLenum	glTarget = GL_TEXTURE_2D ;
		glActiveTexture( GL_TEXTURE0 + glTextureShadow1 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE1)" ) ;
		//
		glBindTexture( glTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo( glTextureShadow1, nullptr ) ;
		//
		glActiveTexture( GL_TEXTURE0 + glTextureShadow2 ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE1)" ) ;
		//
		glBindTexture( glTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo( glTextureShadow2, nullptr ) ;
		return ;
	}
	//
	SGLImageObject *	pLastDepthBuf = nullptr ;
	GLint		glDepthSampler = 0 ;
	GLint		iDepthLayer = 0 ;
	size_t		nDepthBufs = 0 ;
	S4DMatrix	matShadowPars ;
	for ( size_t i = 0; i < nShadowCount; i ++ )
	{
		int *	pIndex = m_tableLightID.GetAt( pidLights[i] ) ;
		if ( pIndex == nullptr )
		{
			m_bufShadowmap.iLights[i] = -1 ;
			continue ;
		}
		if ( pLastDepthBuf != ppShadowMapDepths[i] )
		{
			if ( nDepthBufs == 0 )
			{
				glDepthSampler = glTextureShadow1 ;
			}
			else if ( nDepthBufs == 1 )
			{
				glDepthSampler = glTextureShadow2 ;
			}
			else
			{
				m_bufShadowmap.iLights[i] = -1 ;
				continue ;
			}
			pLastDepthBuf = ppShadowMapDepths[i] ;
			pglTexture =
				SGLOpenGLTextureBuffer::CommitGLTextureRsrc
					( m_pOpenGL, pLastDepthBuf, rectRefTexture ) ;
			if ( pglTexture != nullptr )
			{
				glActiveTexture( GL_TEXTURE0 + glDepthSampler ) ;
				SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE1)" ) ;
				//
				#if	!defined(__API_OPEN_GL_ES__)
//				glEnable( pglTexture->m_paramTarget ) ;
//				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
				#endif
				//
				glBindTexture
					( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
				SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
				//
				m_pOpenGL->SetBindTextureInfo
					( glDepthSampler, pglTexture, pglTexture->m_paramTarget ) ;
				//
				glTexParameteri
					( pglTexture->m_paramTarget,
						GL_TEXTURE_MIN_FILTER, pglTexture->m_paramDefFilter ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
				glTexParameteri
					( pglTexture->m_paramTarget,
						GL_TEXTURE_MAG_FILTER, pglTexture->m_paramDefFilter ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
				//
				glTexParameteri
					( pglTexture->m_paramTarget,
						GL_TEXTURE_WRAP_S, pglTexture->m_paramDefWrap ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
				glTexParameteri
					( pglTexture->m_paramTarget,
						GL_TEXTURE_WRAP_T, pglTexture->m_paramDefWrap ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
			}
			iDepthLayer = 0 ;
			nDepthBufs ++ ;
		}
		else
		{
			iDepthLayer ++ ;
		}
		if ( pglTexture == nullptr )
		{
			m_bufShadowmap.iLights[i] = -1 ;
			continue ;
		}
		m_bufShadowmap.iLights[i] = (GLint) *pIndex ;
		m_bufShadowmap.iSamplers[i] = glDepthSampler ;
		m_bufShadowmap.iLayers[i] = iDepthLayer ;
		//
		S4DMatrix	matShadowPars ;
		double		xProjection = rectRefTexture.w * 0.5 ;
		double		yProjection = rectRefTexture.h * 0.5 ;
		if ( pinfShadowMaps[i].zPersScreen == 0.0f )
		{
			matShadowPars.OrthogonalProjection
				( - xProjection / pinfShadowMaps[i].zPersScale,
					xProjection / pinfShadowMaps[i].zPersScale,
					- yProjection / pinfShadowMaps[i].zPersScale,
					yProjection / pinfShadowMaps[i].zPersScale,
					pinfShadowMaps[i].zPersNear, pinfShadowMaps[i].zPersFar, true ) ;
		}
		else
		{
			matShadowPars.PerspectiveProjection
				( xProjection, yProjection,
					pinfShadowMaps[i].zPersScreen * pinfShadowMaps[i].zPersScale,
					rectRefTexture.w, rectRefTexture.h,
					pinfShadowMaps[i].zPersNear, pinfShadowMaps[i].zPersFar ) ;
		}
		m_bufShadowmap.mat4Perspectiv[i] = matShadowPars ;
		//
		S3DDMatrix	mat3Camera ;
		pinfShadowMaps[i].GetCameraMatrix( mat3Camera ) ;
		//
		S4DMatrix	mat4Camera ;
		S3DDVector	vCameraPos = - S3DDVector(pinfShadowMaps[i].vLight) ;
		mat3Camera.RevolveVector( vCameraPos ) ;
		Matrix4x4From3x3( mat4Camera, mat3Camera, vCameraPos ) ;
		mat4Camera *= pmat4ICameras[i] ;
		//
		mat4Camera.m[1][0] = - mat4Camera.m[1][0] ;	// ※上下反転
		mat4Camera.m[1][1] = - mat4Camera.m[1][1] ;
		mat4Camera.m[1][2] = - mat4Camera.m[1][2] ;
		mat4Camera.m[1][3] = - mat4Camera.m[1][3] ;
		//
		m_bufShadowmap.mat4ModelView[i] = mat4Camera ;
		//
		m_bufShadowmap.vMapUnit[i][0] = (GLfloat) (1.0 / pglTexture->m_imginf.width) ;
		m_bufShadowmap.vMapUnit[i][1] = (GLfloat) (1.0 / pglTexture->m_imginf.height) ;
		m_bufShadowmap.fpFixErrorGap[i] = (GLfloat) (1.0 - pinfShadowMaps[i].fpFixErrorGap) ;
		m_bufShadowmap.fpVarErrorGap[i] = (GLfloat) pinfShadowMaps[i].fpVarErrorGap ;
	}
	glUniform1iv( u_iEnableShadowmap, (GLsizei) nShadowCount, m_bufShadowmap.iLights ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_iEnableShadowmap)" ) ;
	//
	glUniform1iv( u_iShadowmapDepthLayer, (GLsizei) nShadowCount, m_bufShadowmap.iLayers ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_iShadowmapDepthLayer)" ) ;
	//
	#if	defined(__API_OPEN_GL_ES__)
	glUniform1i( u_samplerShadowmap, m_bufShadowmap.iSamplers[0] ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerShadowmap)" ) ;
	#else
	glUniform1iv( u_samplerShadowmap, (GLsizei) nShadowCount, m_bufShadowmap.iSamplers ) ;
	SGLOpenGLContext::VerifyError( "glUniform1iv(u_samplerShadowmap)" ) ;
	#endif
	//
	UniformMatrix4fv
		( u_mat4PerspectiveShadowmap,
			(GLsizei) nShadowCount, GL_TRUE, m_bufShadowmap.mat4Perspectiv ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4PerspectiveShadowmap)" ) ;
	//
	UniformMatrix4fv
		( u_mat4ModelViewShadowmap,
			(GLsizei) nShadowCount, GL_TRUE, m_bufShadowmap.mat4ModelView ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat4PerspectiveShadowmap)" ) ;
	//
	glUniform2fv
		( u_vShadowmapUnit, (GLsizei) nShadowCount, &(m_bufShadowmap.vMapUnit[0][0]) ) ;
	SGLOpenGLContext::VerifyError( "glUniform2fv(u_vShadowmapUnit)" ) ;
	//
	glUniform1fv
		( u_fpShadowmapFixErrorGap, (GLsizei) nShadowCount, m_bufShadowmap.fpFixErrorGap ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpShadowmapFixErrorGap)" ) ;
	//
	glUniform1fv
		( u_fpShadowmapVarErrorGap, (GLsizei) nShadowCount, m_bufShadowmap.fpVarErrorGap ) ;
	SGLOpenGLContext::VerifyError( "glUniform1fv(u_fpShadowmapVarErrorGap)" ) ;
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
	int	r = (rgbFog >> 16) & 0xFF ;
	int	g = (rgbFog >> 8) & 0xFF ;
	int	b = rgbFog & 0xFF ;
	//
	glUniform3f
		( u_rgbFogColor,
			(GLfloat) (r * fp1by255),
			(GLfloat) (g * fp1by255), (GLfloat) (b * fp1by255) ) ;
	SGLOpenGLContext::VerifyError( "glUniform3f(u_rgbFogColor)" ) ;
	//
	glUniform1f( u_zFogNear, (GLfloat) zFogNear ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_zFogNear)" ) ;
	//
	glUniform1f( u_zFogDistance, (GLfloat) (1.0f / (zFogFar - zFogNear)) ) ;
	SGLOpenGLContext::VerifyError( "glUniform1f(u_zFogDistance)" ) ;
}

void SGLOpenGLDefaultShader::EnableFog( bool fFog )
{
	glUniform1i( u_bEnableFog, (fFog ? 1 : 0) ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_bEnableFog)" ) ;
}

// 環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetEnvironmentMapping
	( SGLImageObject * pImage, uint32_t nFlags, S3DMatrix& matMapping )
{
	S3DMatrix	matJ( 1.0f, -1.0f, 1.0f ) ;
	m_matEnvMapping = matJ * matMapping * matJ ;
	//
	m_pEnvMapping = pImage ;
	if ( pImage != nullptr )
	{
		m_nEnvMappingType =
				(nFlags & RenderContext::envMappingTypeMask) + 1 ;
	}
	else
	{
		m_nEnvMappingType = ENV_MAPPING_NOTHING ;
	}
}

void SGLOpenGLDefaultShader::SetRefractionMapping
	( SGLImageObject * pImage, uint32_t nFlags, float32_t nDeepness )
{
	m_pEnvRefraction = pImage ;
	if ( pImage != nullptr )
	{
		m_nEnvRefractionType =
				(nFlags & RenderContext::envMappingTypeMask) + 1 ;
		m_fpEnvRefractionDeepness = nDeepness ;
	}
	else
	{
		m_nEnvRefractionType = ENV_MAPPING_NOTHING ;
	}
}

void SGLOpenGLDefaultShader::SetEnvironmentMappingViewportDepth( SGLImageObject * pDepth )
{
	m_pEnvViewportDepth = pDepth ;
}

// 頂点テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetVertexTexture
	( const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
			SGLImageObject * pImage, bool flagMultiShapeInstance )
{
	ESLAssert( pImage != nullptr ) ;
	size_t	iTxtAlloc = GetCustomTextureCount() ;
	m_pVertexTexture = pImage ;
	m_iVertexTexture = m_gl_iMaterialTextures[iTxtAlloc] ;
	//
	glActiveTexture( GL_TEXTURE0 + m_iVertexTexture ) ;
	//
	GLenum	glTxTarget ;
	if ( m_pOpenGL->IsBindingTexture( m_iVertexTexture, glTxTarget ) )
	{
		glBindTexture( glTxTarget, 0 ) ;
	}
	SGLImageRect		rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
		pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
								( m_pOpenGL, pImage, rectRefTexture ) ;
	if ( pglTexture != nullptr )
	{
		glTxTarget = pglTexture->m_paramTarget ;
		glBindTexture( glTxTarget, pglTexture->m_glTexture ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo
			( m_iVertexTexture, pglTexture, pglTexture->m_paramTarget ) ;
		//
		glTexParameteri
			( glTxTarget,
				GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
		glTexParameteri
			( glTxTarget,
				GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
		//
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
		//
		glUniform1i( u_samplerVertex, m_iVertexTexture ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerVertex)" ) ;
		//
		S2DVector	vTexScale
			( 1.0f / (float32_t) pglTexture->m_imginf.width,
				1.0f / (float32_t) pglTexture->m_imginf.height ) ;
		glUniform2f
			( u_vVertexTextureScale, vTexScale.x, vTexScale.y ) ;
		SGLOpenGLContext::VerifyError( "glUniform2f(u_vVertexTextureScale)" ) ;
		//
		if ( ei.pre->countMorph > 0 )
		{
			glUniform1i( u_bVertexMorphing, 1 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexMorphing)" ) ;
			//
			glUniform1f
				( u_yVertexMorphingFirst,
					(float32_t) ei.yVTMorphVertex * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexMorphingFirst)" ) ;
			//
			glUniform1f
				( u_yNormalMorphingFirst,
					(float32_t) ei.yVTMorphNormal * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yNormalMorphingFirst)" ) ;
			//
			glUniform1f
				( u_yVertexMorphingStride,
					(float32_t) ei.yVTMorphStride * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexMorphingStride)" ) ;
			//
			glUniform1f
				( u_yVertexMorphingInstance,
					(float32_t) ei.yVTMorphInstance * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexMorphingInstance)" ) ;
			//
			glUniform1f
				( u_xVertexMorphingInstanceStride,
					flagMultiShapeInstance ? vTexScale.x * 2.0f : 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_xVertexMorphingInstanceStride)" ) ;
		}
		else
		{
			glUniform1i( u_bVertexMorphing, 0 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexMorphing)" ) ;
		}
		//
		if ( ei.pre->countBone > 0 )
		{
			glUniform1i( u_bVertexBoneInstance, 1 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexBoneInstance)" ) ;
			//
			glUniform1f
				( u_yVertexBoneWeight,
					(float32_t) ei.yVTBoneWeight * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexBoneWeight)" ) ;
			//
			glUniform1f
				( u_yVertexBoneIndex,
					(float32_t) ei.yVTBoneIndex * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexBoneIndex)" ) ;
			//
			glUniform1f
				( u_yVertexBoneMatrix,
					(float32_t) ei.yVTBoneInstance * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexBoneMatrix)" ) ;
			//
			glUniform1f
				( u_yVertexBoneMatrixStride,
					flagMultiShapeInstance ? vTexScale.y : 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_yVertexBoneMatrixStride)" ) ;
		}
		else
		{
			glUniform1i( u_bVertexBoneInstance, 0 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexBoneInstance)" ) ;
		}
		//
		if ( ei.pre->nExAttrElements > 0 )
		{
			glUniform1f( u_yVertexExAttrElements,
						(float32_t) ei.yVTExAttrElements * vTexScale.y ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_yVertexExAttrElements)" ) ;
			//
			glUniform1f( u_xVertexExAttrStride, (float32_t) ei.xVTExAttrStride ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_xVertexExAttrStride)" ) ;
		}
		else
		{
			glUniform1f( u_yVertexExAttrElements, 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_yVertexExAttrElements)" ) ;
			//
			glUniform1f( u_xVertexExAttrStride, 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_xVertexExAttrStride)" ) ;
		}
	}
	else
	{
		m_pOpenGL->SetBindTextureInfo( m_iVertexTexture, nullptr ) ;
	}
	glActiveTexture( GL_TEXTURE0 ) ;
}

void SGLOpenGLDefaultShader::UnsetVertexTexture( void )
{
	if ( m_pVertexTexture != nullptr )
	{
		if ( m_iVertexTexture >= 0 )
		{
			GLenum	glTxTarget ;
			if ( m_pOpenGL->IsBindingTexture( m_iVertexTexture, glTxTarget ) )
			{
				glActiveTexture( GL_TEXTURE0 + m_iVertexTexture ) ;
				glBindTexture( glTxTarget, 0 ) ;
				glActiveTexture( GL_TEXTURE0 ) ;
				//
				m_pOpenGL->SetBindTextureInfo( m_iVertexTexture, nullptr ) ;
			}
		}
		m_pVertexTexture = nullptr ;
		m_iVertexTexture = -1 ;
		//
		glUniform1i( u_bVertexMorphing, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexMorphing)" ) ;
		//
		glUniform1i( u_bVertexBoneInstance, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bVertexBoneInstance)" ) ;
	}
}

// 色効果を設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetColorEffect
	( const S3DColor * pColor, unsigned int nTransparency )
{
	if ( pColor != nullptr )
	{
		if ( (pColor->rgbMul.ui32 & 0x00FFFFFF)
				!= (m_colorUniformEffect.rgbMul.ui32 & 0x00FFFFFF) )
		{
			GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
			GLfloat	r = 1.0f ;
			GLfloat	g = 1.0f ;
			GLfloat	b = 1.0f ;
			if ( (pColor->rgbMul.ui32 & 0x00FFFFFF) != 0x00FFFFFF )
			{
				r = (GLfloat) (pColor->rgbMul.argb.Red * fp1by255) ;
				g = (GLfloat) (pColor->rgbMul.argb.Green * fp1by255) ;
				b = (GLfloat) (pColor->rgbMul.argb.Blue * fp1by255) ;
			}
			glUniform3f( u_vEffectMulColor, r, g, b ) ;
			SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectMulColor)" ) ;
			m_colorUniformEffect.rgbMul = pColor->rgbMul ;
		}
		if ( (pColor->rgbAdd.ui32 & 0x00FFFFFF)
				!= (m_colorUniformEffect.rgbAdd.ui32 & 0x00FFFFFF) )
		{
			GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
			GLfloat	r = 0.0f ;
			GLfloat	g = 0.0f ;
			GLfloat	b = 0.0f ;
			if ( (pColor->rgbAdd.ui32 & 0x00FFFFFF) != 0 )
			{
				r = (GLfloat) (pColor->rgbAdd.argb.Red * fp1by255) ;
				g = (GLfloat) (pColor->rgbAdd.argb.Green * fp1by255) ;
				b = (GLfloat) (pColor->rgbAdd.argb.Blue * fp1by255) ;
			}
			glUniform3f( u_vEffectAddColor, r, g, b ) ;
			SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectAddColor)" ) ;
			m_colorUniformEffect.rgbAdd = pColor->rgbAdd ;
		}
	}
	else
	{
		if ( (m_colorUniformEffect.rgbMul.ui32 & 0x00FFFFFF) != 0x00FFFFFF )
		{
			glUniform3f( u_vEffectMulColor, 1.0f, 1.0f, 1.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectMulColor)" ) ;
			m_colorUniformEffect.rgbMul = 0x00FFFFFF ;
		}
		if ( (m_colorUniformEffect.rgbAdd.ui32 & 0x00FFFFFF) != 0 )
		{
			glUniform3f( u_vEffectAddColor, 0.0f, 0.0f, 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectAddColor)" ) ;
			m_colorUniformEffect.rgbAdd = 0 ;
		}
	}
	if ( m_nUniformTransparency != nTransparency )
	{
		if ( nTransparency >= 0x100 )
		{
			glUniform1f( u_fpEffectAlpha, 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpEffectAlpha)" ) ;
		}
		else if ( nTransparency > 0 )
		{
			glUniform1f
				( u_fpEffectAlpha,
					(GLfloat) ((0x100 - nTransparency) / 256.0) ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpEffectAlpha)" ) ;
		}
		else
		{
			glUniform1f( u_fpEffectAlpha, 1.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpEffectAlpha)" ) ;
		}
		m_nUniformTransparency = nTransparency ;
	}
}

// 表面属性を設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetMaterial
	( S3DMaterial * pMaterial,
		bool fBackFace, uint64_t flagRemove, uint64_t flagAdd )
{
	SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
	if ( pMaterial == nullptr )
	{
		if ( (m_psaLastMaterial != nullptr)
			|| (m_flagsLastMaterialShading != (uint64_t) -1) )
		{
			BindTexture( glTextureDefault, nullptr, nullptr, 0 ) ;
			BindLuminousTexture( glTextureMaterial0, nullptr, nullptr, 0, 0.0f ) ;
			BindEnvironmentTexture
				( glTextureMaterial0, nullptr,
					nullptr, ENV_MAPPING_NOTHING, false, nullptr ) ;
			BindRefractionTexture
				( glTextureMaterial0, nullptr,
					nullptr, ENV_MAPPING_NOTHING, false, nullptr, 0.0f ) ;
			BindViewportDepthTexture( glTextureMaterial0, nullptr, nullptr, false ) ;
			BindNormalTexture( glTextureMaterial0, nullptr, nullptr, 0, 0.0f ) ;
			BindHeightTexture( glTextureMaterial0, nullptr, nullptr, 0, 0.0f ) ;
			BindAlphaTexture( glTextureMaterial0, nullptr, nullptr, 0, 1.0f, 0.0f ) ;
			BindSpecularTexture( glTextureMaterial0, nullptr, nullptr, 0 ) ;
			BindGlobalAOTexture( glTextureMaterial0, nullptr, 0 ) ;
		}
		pOpenGL->m_iTempTexture = 0 ;
		m_psaLastMaterial = nullptr ;
		m_flagsLastMaterialShading = (uint64_t) -1 ;
		return ;
	}
	S3DSurfaceAttribute *	psa = &(pMaterial->m_attrSurface) ;
	if ( fBackFace && pMaterial->m_flagBack )
	{
		psa = &(pMaterial->m_attrBack) ;
	}
	uint64_t	flagsShading = (psa->flagsShading & ~flagRemove) | flagAdd ;
	//
	if ( (psa == m_psaLastMaterial)
			& (flagsShading == m_flagsLastMaterialShading) )
	{
		return ;
	}
	m_psaLastMaterial = psa ;
	m_flagsLastMaterialShading = flagsShading ;
	//
	pOpenGL->m_pflog.countSwitchMaterial ++ ;
	//
	// 描画設定
	//
	GLint	bShading = (((flagsShading & shadingMethodMask)
									!= shadingMethodNothing) ? 1 : 0) ;
	if ( m_bMaterialShading != bShading )
	{
		glUniform1i( u_bMaterialShading, bShading ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialShading)" ) ;
		m_bMaterialShading = bShading ;
	}
	GLint	bToon = ((m_fShaderForceToon
						|| (flagsShading & shadingMethodToon)) ? 1 : 0) ;
	if ( m_bMaterialToon != bToon )
	{
		glUniform1i( u_bMaterialToon, bToon ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialToon)" ) ;
		m_bMaterialToon = bToon ;
	}
	GLint	iTriming = 0 ;
	if ( flagsShading & shadingTextureTriming )
	{
		iTriming = (flagsShading & shadingTextureDithering) ? 2 : 1 ;
	}
	if ( m_iMaterialTriming != iTriming )
	{
		glUniform1i( u_iMaterialTriming, iTriming ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_iMaterialTriming)" ) ;
		m_iMaterialTriming = iTriming ;
	}
	GLint	bNoFogEffect = ((flagsShading & shadingNoFogEffect) ? 1 : 0) ;
	if ( m_bMaterialNoFogEffect != bNoFogEffect )
	{
		glUniform1i( u_bMaterialNoFogEffect, bNoFogEffect ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialNoFogEffect)" ) ;
		m_bMaterialNoFogEffect = bNoFogEffect ;
	}
	GLint	bVertexAlpha = ((flagsShading & shadingVertexAlpha) ? 1 : 0) ;
	if ( m_bMaterialVertexAlpha != bVertexAlpha )
	{
		glUniform1i( u_bMaterialVertexAlpha, bVertexAlpha ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialVertexAlpha)" ) ;
		m_bMaterialVertexAlpha = bVertexAlpha ;
	}
	//
	glEnable( GL_BLEND ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_BLEND)" ) ;
	//
	#if	!defined(__API_OPEN_GL_ES__)
	/*
	if ( !pOpenGL->m_alphaTestZeroClip )
	{
		glAlphaFunc( GL_GEQUAL, 0.0078125 ) ;
		SGLOpenGLContext::VerifyError( "glAlphaFunc" ) ;
		glEnable( GL_ALPHA_TEST ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_ALPHA_TEST)" ) ;
		pOpenGL->m_alphaTestZeroClip = true ;
	}
	*/
	#endif
	//
	// 面カリング
	//
	S3DRenderBufferInterface::FaceCullingOperation
								faceCulling = m_faceCulling ;
	if ( faceCulling == S3DRenderBufferInterface::faceCullingDefault )
	{
		if ( fBackFace || (flagsShading & shadingSingleSidePlane) )
		{
			if ( !fBackFace )
			{
				faceCulling = S3DRenderBufferInterface::faceCullingBack ;
			}
			else
			{
				faceCulling = S3DRenderBufferInterface::faceCullingFront ;
			}
		}
		else
		{
			faceCulling = S3DRenderBufferInterface::faceCullingNo ;
		}
	}
	if ( faceCulling == S3DRenderBufferInterface::faceCullingNo )
	{
		if ( m_bMaterialDoubleSide == 0 )
		{
			glUniform1i( u_bMaterialDoubleSide, 1 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialDoubleSide)" ) ;
			m_bMaterialDoubleSide = 1 ;
		}
		if ( !pOpenGL->m_faceDoubleSide )
		{
			glDisable( GL_CULL_FACE ) ;		// 両面ポリゴン
			SGLOpenGLContext::VerifyError( "glDisable(GL_CULL_FACE)" ) ;
			pOpenGL->m_faceDoubleSide = true ;
		}
	}
	else
	{
		if ( m_bMaterialDoubleSide != 0 )
		{
			glUniform1i( u_bMaterialDoubleSide, 0 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialDoubleSide)" ) ;
			m_bMaterialDoubleSide = 0 ;
		}
		if ( pOpenGL->m_faceDoubleSide )
		{
			glEnable( GL_CULL_FACE ) ;		// 片面ポリゴン
			SGLOpenGLContext::VerifyError( "glEnable(GL_CULL_FACE)" ) ;
			pOpenGL->m_faceDoubleSide = false ;
		}
		if ( faceCulling == S3DRenderBufferInterface::faceCullingBack )
		{
			if ( !pOpenGL->m_faceCullBack )
			{
				glCullFace( GL_BACK ) ;
				SGLOpenGLContext::VerifyError( "glCullFace(GL_BACK)" ) ;
				pOpenGL->m_faceCullBack = true ;
			}
		}
		else
		{
			if ( pOpenGL->m_faceCullBack )
			{
				glCullFace( GL_FRONT ) ;
				SGLOpenGLContext::VerifyError( "glCullFace(GL_FRONT)" ) ;
				pOpenGL->m_faceCullBack = false ;
			}
		}
	}
	//
	// ｚバッファ処理法
	//
	S3DRenderBufferInterface::DepthMaskOperation
								depthMask = m_depthMask ;
	if ( depthMask == S3DRenderBufferInterface::depthMaskDefault )
	{
		if ( flagsShading & shadingNoZBuffer )
		{
			depthMask = S3DRenderBufferInterface::depthMaskNoTest ;
		}
		else
		{
			if ( m_fDisableWriteDepth
				| ((flagsShading & shadingZBufferNoWrite) != 0) )
			{
				depthMask = S3DRenderBufferInterface::depthMaskNoWrite ;
			}
			else
			{
				depthMask = S3DRenderBufferInterface::depthMaskEnable ;
			}
		}
	}
	if ( depthMask == S3DRenderBufferInterface::depthMaskEnable )
	{
		if ( pOpenGL->m_funcDepthTest != 0 )
		{
			glEnable( GL_DEPTH_TEST ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
			glDepthFunc( GL_LEQUAL );
			SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
			glDepthMask( GL_TRUE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_TRUE)" ) ;
			pOpenGL->m_funcDepthTest = 0 ;
		}
	}
	else if ( depthMask == S3DRenderBufferInterface::depthMaskNoWrite )
	{
		if ( pOpenGL->m_funcDepthTest != shadingZBufferNoWrite )
		{
			glEnable( GL_DEPTH_TEST ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
			glDepthFunc( GL_LEQUAL );
			SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
			glDepthMask( GL_FALSE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
			pOpenGL->m_funcDepthTest = shadingZBufferNoWrite ;
		}
	}
	else if ( depthMask == S3DRenderBufferInterface::depthMaskNoWriteGT )
	{
		if ( pOpenGL->m_funcDepthTest != (shadingZBufferNoWrite | shadingNoZBuffer) )
		{
			glEnable( GL_DEPTH_TEST ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
			glDepthFunc( GL_GREATER ) ;
			SGLOpenGLContext::VerifyError( "glDepthFunc(GL_GREATER)" ) ;
			glDepthMask( GL_FALSE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
			pOpenGL->m_funcDepthTest = shadingZBufferNoWrite | shadingNoZBuffer ;
		}
	}
	else
	{
		if ( pOpenGL->m_funcDepthTest != shadingNoZBuffer )
		{
			glDisable( GL_DEPTH_TEST ) ;
			SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
			glDepthMask( GL_FALSE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
			pOpenGL->m_funcDepthTest = shadingNoZBuffer ;
		}
	}
	//
	// 基本色
	//
	GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
	GLfloat	fp1by256 = (GLfloat) (1.0 / 256.0) ;
	int		rMul, gMul, bMul, rAdd, gAdd, bAdd ;
	//
	rMul = (int) psa->colorBase.rgbMul.argb.Red
				- (int) psa->colorShade.rgbMul.argb.Red ;
	gMul = (int) psa->colorBase.rgbMul.argb.Green
				- (int) psa->colorShade.rgbMul.argb.Green ;
	bMul = (int) psa->colorBase.rgbMul.argb.Blue
				- (int) psa->colorShade.rgbMul.argb.Blue ;
	rAdd = (int) psa->colorBase.rgbAdd.argb.Red
				- (int) psa->colorShade.rgbAdd.argb.Red ;
	gAdd = (int) psa->colorBase.rgbAdd.argb.Green
				- (int) psa->colorShade.rgbAdd.argb.Green ;
	bAdd = (int) psa->colorBase.rgbAdd.argb.Blue
				- (int) psa->colorShade.rgbAdd.argb.Blue ;
	//
	if ( (m_saLastMaterial.colorBase.rgbMul.argb.Red != rMul)
		| (m_saLastMaterial.colorBase.rgbMul.argb.Green != gMul)
		| (m_saLastMaterial.colorBase.rgbMul.argb.Blue != bMul) )
	{
		glUniform3f
			( u_vMaterialMulColor,
				(GLfloat) (rMul * fp1by255), 
				(GLfloat) (gMul * fp1by255), (GLfloat) (bMul * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialMulColor)" ) ;
		//
		m_saLastMaterial.colorBase.rgbMul.argb.Red = (uint8_t) rMul ;
		m_saLastMaterial.colorBase.rgbMul.argb.Green = (uint8_t) gMul ;
		m_saLastMaterial.colorBase.rgbMul.argb.Blue = (uint8_t) bMul ;
	}
	if ( (m_saLastMaterial.colorBase.rgbAdd.argb.Red != rAdd)
		| (m_saLastMaterial.colorBase.rgbAdd.argb.Green != gAdd)
		| (m_saLastMaterial.colorBase.rgbAdd.argb.Blue != bAdd) )
	{
		glUniform3f
			( u_vMaterialAddColor,
				(GLfloat) (rAdd * fp1by255), 
				(GLfloat) (gAdd * fp1by255), (GLfloat) (bAdd * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialAddColor)" ) ;
		//
		m_saLastMaterial.colorBase.rgbAdd.argb.Red = (uint8_t) rAdd ;
		m_saLastMaterial.colorBase.rgbAdd.argb.Green = (uint8_t) gAdd ;
		m_saLastMaterial.colorBase.rgbAdd.argb.Blue = (uint8_t) bAdd ;
	}
	if ( (m_saLastMaterial.colorShade.rgbMul.ui32 & 0x00FFFFFF)
			!= ( psa->colorShade.rgbMul.ui32 & 0x00FFFFFF) )
	{
		rMul = (int) psa->colorShade.rgbMul.argb.Red ;
		gMul = (int) psa->colorShade.rgbMul.argb.Green ;
		bMul = (int) psa->colorShade.rgbMul.argb.Blue ;
		//
		glUniform3f
			( u_vMaterialMulShade,
				(GLfloat) (rMul * fp1by255), 
				(GLfloat) (gMul * fp1by255), (GLfloat) (bMul * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialMulSahde)" ) ;
		//
		m_saLastMaterial.colorShade.rgbMul = psa->colorShade.rgbMul ;
	}
	if ( (m_saLastMaterial.colorShade.rgbAdd.ui32 & 0x00FFFFFF)
			!= ( psa->colorShade.rgbAdd.ui32 & 0x00FFFFFF) )
	{
		rAdd = (int) psa->colorShade.rgbAdd.argb.Red ;
		gAdd = (int) psa->colorShade.rgbAdd.argb.Green ;
		bAdd = (int) psa->colorShade.rgbAdd.argb.Blue ;
		//
		glUniform3f
			( u_vMaterialAddShade,
				(GLfloat) (rAdd * fp1by255), 
				(GLfloat) (gAdd * fp1by255), (GLfloat) (bAdd * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialAddSahde)" ) ;
		//
		m_saLastMaterial.colorShade.rgbAdd = psa->colorShade.rgbAdd ;
	}
	SGLPalette	rgbSpecularColor( 0x00FFFFFF ) ;
	if ( psa->nExFlags & shadingExSpecularColor )
	{
		rgbSpecularColor.ui32 = psa->rgbSpecularColor.ui32 & 0x00FFFFFF ;
	}
	if ( m_saLastMaterial.rgbSpecularColor.ui32 != rgbSpecularColor.ui32 )
	{
		rAdd = (int) rgbSpecularColor.argb.Red ;
		gAdd = (int) rgbSpecularColor.argb.Green ;
		bAdd = (int) rgbSpecularColor.argb.Blue ;
		//
		glUniform3f
			( u_vMaterialSpecularColor,
				(GLfloat) (rAdd * fp1by255), 
				(GLfloat) (gAdd * fp1by255), (GLfloat) (bAdd * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialSpecularColor)" ) ;
		//
		m_saLastMaterial.rgbSpecularColor = rgbSpecularColor ;
	}
	//
	// 表面属性
	//
	int32_t	nAmbient = psa->nAmbient + psa->nEmission ;
	if ( m_saLastMaterial.nAmbient != nAmbient )
	{
		glUniform1f( u_fMaterialAmbient, (GLfloat) (nAmbient * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialAmbient)" ) ;
		m_saLastMaterial.nAmbient = nAmbient ;
	}
	if ( m_saLastMaterial.nDiffusion != psa->nDiffusion )
	{
		glUniform1f( u_fMaterialDiffusion, (GLfloat) (psa->nDiffusion * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDiffusion)" ) ;
		m_saLastMaterial.nDiffusion = psa->nDiffusion ;
	}
	int32_t	nBackDiffusion = 0 ;
	if ( psa->nExFlags & shadingExBackDiffusion )
	{
		nBackDiffusion = psa->nBackDiffusion ;
	}
	if ( m_saLastMaterial.nBackDiffusion != nBackDiffusion )
	{
		glUniform1f( u_fMaterialBackDiffusion, (GLfloat) (nBackDiffusion * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialBackDiffusion)" ) ;
		m_saLastMaterial.nBackDiffusion = psa->nBackDiffusion ;
	}
	if ( m_saLastMaterial.nSpecular != psa->nSpecular )
	{
		glUniform1f( u_fMaterialSpecular, (GLfloat) (psa->nSpecular * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialSpecular)" ) ;
		m_saLastMaterial.nSpecular = psa->nSpecular ;
	}
	if ( m_saLastMaterial.nSpecularSize != psa->nSpecularSize )
	{
		glUniform1f
			( u_fMaterialSpecularPow,
				(GLfloat) (256.0f / esl_max(psa->nSpecularSize,1)) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialSpecularPow)" ) ;
		m_saLastMaterial.nSpecularSize = psa->nSpecularSize ;
	}
	if ( m_saLastMaterial.nTransparency != psa->nTransparency )
	{
		glUniform1f( u_fMaterialAlpha, (GLfloat) (1.0f - psa->nTransparency * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialAlpha)" ) ;
		m_saLastMaterial.nTransparency = psa->nTransparency ;
	}
	if ( m_saLastMaterial.nDeepness != psa->nDeepness )
	{
		glUniform1f( u_fMaterialDeepness, (GLfloat) (psa->nDeepness * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDeepness)" ) ;
		m_saLastMaterial.nDeepness = psa->nDeepness ;
	}
	if ( m_saLastMaterial.nDeepnessPower != psa->nDeepnessPower )
	{
		glUniform1f( u_fMaterialDeepnessPow, (GLfloat) (psa->nDeepnessPower * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialDeepnessPow)" ) ;
		m_saLastMaterial.nDeepnessPower = psa->nDeepnessPower ;
	}
	if ( m_saLastMaterial.nReflection != psa->nReflection )
	{
		glUniform1f( u_fMaterialReflection, (GLfloat) (psa->nReflection * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialReflection)" ) ;
		m_saLastMaterial.nReflection = psa->nReflection ;
	}
	int32_t	nEmission = (int32_t) psa->nEmission ;
	if ( m_fShaderEmisiveTarget || (flagsShading & shadingEmisiveTarget) )
	{
		nEmission = 0x100 ;
	}
	if ( m_saLastMaterial.nEmission != nEmission )
	{
		glUniform1f( u_fMaterialEmission, (GLfloat) (nEmission * fp1by256) ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialEmission)" ) ;
		m_saLastMaterial.nEmission = (uint32_t) nEmission ;
	}
	float32_t	cosShadeThreshold = 0.0f ;
	float32_t	fpToonShadeThreshold = 0.5f ;
	float32_t	fpToonShadeBrightness = 0.5f ;
	if ( psa->nExFlags & shadingExVarietyShade )
	{
		cosShadeThreshold = psa->cosShadeThreshold ;
		fpToonShadeThreshold = psa->fpToonShadeThreshold ;
		fpToonShadeBrightness = psa->fpToonShadeBrightness ;
	}
	if ( m_saLastMaterial.cosShadeThreshold != cosShadeThreshold )
	{
		GLfloat	cosShadeCoefficient[2] = { 1.0f, 1.0f } ;
		if ( cosShadeThreshold < 1.0f )
		{
			cosShadeCoefficient[0] = 1.0f / (1.0f - cosShadeThreshold) ;
			cosShadeCoefficient[1] = cosShadeCoefficient[0] * cosShadeThreshold ;
		}
		glUniform1fv( u_cosShadeCoefficient, 2, cosShadeCoefficient ) ;
		SGLOpenGLContext::VerifyError( "glUniform1fv(u_cosShadeCoefficient)" ) ;
		m_saLastMaterial.cosShadeThreshold = cosShadeThreshold ;
	}
	if ( m_saLastMaterial.fpToonShadeThreshold != fpToonShadeThreshold )
	{
		glUniform1f( u_fpToonShadeThreshold, fpToonShadeThreshold ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpToonShadeThreshold)" ) ;
		m_saLastMaterial.fpToonShadeThreshold = fpToonShadeThreshold ;
	}
	if ( m_saLastMaterial.fpToonShadeBrightness != fpToonShadeBrightness )
	{
		glUniform1f( u_fpToonShadeBrightness, fpToonShadeBrightness ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpToonShadeBrightness)" ) ;
		m_saLastMaterial.fpToonShadeBrightness = fpToonShadeBrightness ;
	}
	S3DColor	colorBackLight( 0, 0 ) ;
	if ( psa->nExFlags & shadingExBackLight )
	{
		uint32_t	b =
			esl_clampi( eslRoundR32ToInt
						( psa->fpBackLight * 0x100 ), 0, 0x100 ) ;
		colorBackLight = psa->colorBackLight.imul(b) ;
	}
	if ( m_saLastMaterial.colorBackLight != colorBackLight )
	{
		rMul = (int) colorBackLight.rgbMul.argb.Red ;
		gMul = (int) colorBackLight.rgbMul.argb.Green ;
		bMul = (int) colorBackLight.rgbMul.argb.Blue ;
		//
		rAdd = (int) colorBackLight.rgbAdd.argb.Red ;
		gAdd = (int) colorBackLight.rgbAdd.argb.Green ;
		bAdd = (int) colorBackLight.rgbAdd.argb.Blue ;
		//
		glUniform3f
			( u_vMaterialBackLightMul,
				(GLfloat) (rMul * fp1by255), 
				(GLfloat) (gMul * fp1by255), (GLfloat) (bMul * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialBackLightMul)" ) ;
		glUniform3f
			( u_vMaterialBackLightAdd,
				(GLfloat) (rAdd * fp1by255), 
				(GLfloat) (gAdd * fp1by255), (GLfloat) (bAdd * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialBackLightAdd)" ) ;
		//
		m_saLastMaterial.colorBackLight = colorBackLight ;
	}
	float32_t	fpRimLight = 0.0f ;
	float32_t	fpRimDeepness = 0.1f ;
	SGLPalette	rgbRimColor( 0x00FFFFFF ) ;
	if ( psa->nExFlags & shadingExRimLight )
	{
		fpRimLight = psa->fpRimLight ;
		fpRimDeepness = psa->fpRimLightDeepness ;
		rgbRimColor.ui32 = psa->rgbRimLightColor.ui32 & 0x00FFFFFF ;
	}
	if ( m_saLastMaterial.fpRimLight != fpRimLight )
	{
		glUniform1f( u_fMaterialRimLight, fpRimLight ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialRimLight)" ) ;
		m_saLastMaterial.fpRimLight = fpRimLight ;
	}
	if ( m_saLastMaterial.fpRimLightDeepness != fpRimDeepness )
	{
		glUniform1f( u_fMaterialRimDeepness, fpRimDeepness ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fMaterialRimDeepness)" ) ;
		m_saLastMaterial.fpRimLightDeepness = fpRimDeepness ;
	}
	if ( m_saLastMaterial.rgbRimLightColor.ui32 != rgbRimColor.ui32 )
	{
		rAdd = (int) rgbRimColor.argb.Red ;
		gAdd = (int) rgbRimColor.argb.Green ;
		bAdd = (int) rgbRimColor.argb.Blue ;
		//
		glUniform3f
			( u_vMaterialRimColor,
				(GLfloat) (rAdd * fp1by255), 
				(GLfloat) (gAdd * fp1by255), (GLfloat) (bAdd * fp1by255) ) ; 
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vMaterialRimColor)" ) ;
		//
		m_saLastMaterial.rgbRimLightColor = rgbRimColor ;
	}
	//
	// シャドウマッピング
	//
	if ( flagsShading & shadingNoDropShadow )
	{
		if ( m_flagEnabledShadowmap )
		{
			GLint	iLights[MAX_SHADOWMAPPING] ;
			for ( int i = 0; i < MAX_SHADOWMAPPING; i ++ )
			{
				iLights[i] = -1 ;
			}
			glUniform1iv( u_iEnableShadowmap, MAX_SHADOWMAPPING, iLights ) ;
			SGLOpenGLContext::VerifyError( "glUniform1iv(u_iEnableShadowmap)" ) ;
		}
		m_flagEnabledShadowmap = false ;
	}
	else
	{
		if ( !m_flagEnabledShadowmap )
		{
			glUniform1iv( u_iEnableShadowmap, MAX_SHADOWMAPPING, m_bufShadowmap.iLights ) ;
			SGLOpenGLContext::VerifyError( "glUniform1iv(u_iEnableShadowmap)" ) ;
		}
	}
	//
	// テクスチャ
	//
	size_t				iTxtAlloc = GetCustomTextureCount() ;
	SGLImageObject *	pTextureImage = nullptr ;
	SGLImageObject *	pTextureImage2 = nullptr ;
	if ( flagsShading & shadingTextureMapping )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
								( S3DMaterial::textureDiffusion ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
								( S3DMaterial::textureDiffusion ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
			}
		}
	}
	BindTexture( glTextureDefault, pTextureImage, nullptr, flagsShading ) ;
	//
	if ( m_enabledVTInstancing )
	{
		iTxtAlloc ++ ;			// 頂点テクスチャ用
	}
	//
	// 環境マッピング
	//
	int	typeEnvMapping = ENV_MAPPING_SPHERE ;
	pTextureImage = nullptr ;
	if ( flagsShading & shadingGEnvironmentMapping )
	{
		pTextureImage = m_pEnvMapping ;
		typeEnvMapping = m_nEnvMappingType ;
	}
	else if ( flagsShading & shadingEnvironmentMapping )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
								( S3DMaterial::textureEnvironment ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
								( S3DMaterial::textureEnvironment ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
			}
		}
	}
	bool	fTxtSmoothing = ((flagsShading & shadingTextureSmoothing) != 0) ;
	if ( BindEnvironmentTexture
			( m_gl_iMaterialTextures[iTxtAlloc],
				pTextureImage, nullptr, typeEnvMapping,
				true /*fTxtSmoothing*/, &m_matEnvMapping ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// 屈折反映環境マッピング
	//
	float32_t	fpRefraction = psa->fpRefraction ;
	pTextureImage = nullptr ;
	pTextureImage2 = nullptr ;
	if ( (m_pEnvRefraction != nullptr)
		&& (m_nEnvRefractionType != ENV_MAPPING_NOTHING)
		&& (flagsShading & shadingRefractEnvMapping) )
	{
		pTextureImage = m_pEnvRefraction ;
		pTextureImage2 = (m_nEnvRefractionType == ENV_MAPPING_VIEWPORT)
									? m_pEnvViewportDepth : nullptr ;
//		fpRefraction *= m_fpEnvRefractionDeepness ;
		typeEnvMapping = m_nEnvRefractionType ;
	}
	if ( BindRefractionTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage, nullptr, typeEnvMapping,
			true , &m_matEnvMapping, fpRefraction ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	if ( BindViewportDepthTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage2, nullptr, true ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// 発光マッピング
	//
	float32_t	fpLuminousApply = 0.0f ;
	pTextureImage = nullptr ;
	if ( flagsShading & shadingLuminousTexture )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureLuminous ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
				fpLuminousApply = pMaterial->GetBackTextureApplication(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureLuminous ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
				fpLuminousApply = pMaterial->GetTextureApplication(iTexture) ;
			}
		}
	}
	if ( BindLuminousTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage, nullptr, flagsShading, fpLuminousApply ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// αマッピング
	//
	float32_t	fpAlphaCoefficient = 1.0f ;
	float32_t	fpAlphaBase = 0.0f ;
	pTextureImage = nullptr ;
	if ( flagsShading & shadingAlphaTexture )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureAlpha ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
				fpAlphaCoefficient = pMaterial->GetBackTextureApplication(iTexture) ;
				fpAlphaBase = pMaterial->GetBackTextureParameter(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureAlpha ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
				fpAlphaCoefficient = pMaterial->GetTextureApplication(iTexture) ;
				fpAlphaBase = pMaterial->GetTextureParameter(iTexture) ;
			}
		}
	}
	if ( BindAlphaTexture
			( m_gl_iMaterialTextures[iTxtAlloc],
				pTextureImage, nullptr,
				flagsShading, fpAlphaCoefficient, fpAlphaBase ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// 法線マッピング
	//
	float32_t	fpNormalApply = 0.0f ;
	pTextureImage = nullptr ;
	if ( flagsShading & shadingNormalTexture )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureNormal ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
				fpNormalApply = pMaterial->GetBackTextureApplication(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureNormal ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
				fpNormalApply = pMaterial->GetTextureApplication(iTexture) ;
			}
		}
	}
	if ( BindNormalTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage, nullptr, flagsShading, fpNormalApply ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// 標高マッピング
	//
	float32_t	fpHeightParam = 0.0f ;
	pTextureImage = nullptr ;
	if ( flagsShading & shadingHeightTexture )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureHeight ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
				fpHeightParam =
					pMaterial->GetBackTextureApplication(iTexture)
						* pMaterial->GetBackTextureParameter(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureHeight ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
				fpHeightParam =
					pMaterial->GetTextureApplication(iTexture)
						* pMaterial->GetTextureParameter(iTexture) ;
			}
		}
	}
	if ( BindHeightTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage, nullptr, flagsShading, fpHeightParam ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// スペキュラ・反射率・粗さ（きめ細かさ）テクスチャマッピング
	//
	pTextureImage = nullptr ;
	if ( flagsShading & shadingSpecularMapping )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureSpecular ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureSpecular ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
			}
		}
	}
	if ( BindSpecularTexture
		( m_gl_iMaterialTextures[iTxtAlloc],
			pTextureImage, nullptr, flagsShading ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	//
	// 大域ライトマップAOテクスチャ設定
	//
	pTextureImage = nullptr ;
	if ( flagsShading & shadingGlobalAOLightMap )
	{
		if ( fBackFace && pMaterial->m_flagBack )
		{
			int	iTexture = pMaterial->FindBackTextureTypeOf
									( S3DMaterial::textureGlobalAO ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetBackTexture(iTexture) ;
			}
		}
		else
		{
			int	iTexture = pMaterial->FindTextureTypeOf
									( S3DMaterial::textureGlobalAO ) ;
			if ( iTexture >= 0 )
			{
				pTextureImage = pMaterial->GetTexture(iTexture) ;
			}
		}
	}
	if ( BindGlobalAOTexture
		( m_gl_iMaterialTextures[iTxtAlloc], pTextureImage, flagsShading ) != nullptr )
	{
		iTxtAlloc ++ ;
	}
	ESLAssert( iTxtAlloc < 32 ) ;
	//
	pOpenGL->m_iTempTexture = (size_t) m_gl_iMaterialTextures[iTxtAlloc] ;
}

// 輪郭描画用パラメータを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetBorderOffset
	( bool fBorder, float32_t zTarget,
			const S3DColor& colorEffect,
			const S3DSurfaceAttribute& saMaterial )
{
	if ( fBorder )
	{
		SGLPalette	rgbBorderColor = m_rgbBorderColor ;
		float32_t	fpBorderCoefficient[2] =
		{
			m_fpBorderCoefficient[0] / m_vProjectScreen.z,
			m_fpBorderCoefficient[1]
		} ;
		if ( saMaterial.nExFlags & shadingExBorderParam )
		{
			rgbBorderColor = saMaterial.rgbBorderColor ;
			fpBorderCoefficient[0] =
					saMaterial.fpBorderThicknessA / m_vProjectScreen.z ;
			fpBorderCoefficient[1] = saMaterial.fpBorderThicknessB ;
		}
		//
		S3DColor	clrBorder ;
		clrBorder.rgbMul.ui32 = 0 ;
		clrBorder.rgbAdd = rgbBorderColor ;
		clrBorder = clrBorder * colorEffect ;
		//
		GLfloat	fp1by255 = (GLfloat) (1.0 / 255.0) ;
		GLfloat	r = 1.0f ;
		GLfloat	g = 1.0f ;
		GLfloat	b = 1.0f ;
		if ( (clrBorder.rgbMul.ui32 & 0x00FFFFF) != 0x00FFFFF )
		{
			r = (GLfloat) (clrBorder.rgbMul.argb.Red * fp1by255) ;
			g = (GLfloat) (clrBorder.rgbMul.argb.Green * fp1by255) ;
			b = (GLfloat) (clrBorder.rgbMul.argb.Blue * fp1by255) ;
		}
		glUniform3f( u_vEffectMulColor, r, g, b ) ;
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectMulColor)" ) ;
		m_colorUniformEffect.rgbMul = clrBorder.rgbMul ;
		//
		r = 0.0f ;
		g = 0.0f ;
		b = 0.0f ;
		if ( (clrBorder.rgbAdd.ui32 & 0x00FFFFF) != 0 )
		{
			r = (GLfloat) (clrBorder.rgbAdd.argb.Red * fp1by255) ;
			g = (GLfloat) (clrBorder.rgbAdd.argb.Green * fp1by255) ;
			b = (GLfloat) (clrBorder.rgbAdd.argb.Blue * fp1by255) ;
		}
		glUniform3f( u_vEffectAddColor, r, g, b ) ;
		SGLOpenGLContext::VerifyError( "glUniform3f(u_vEffectAddColor)" ) ;
		m_colorUniformEffect.rgbAdd = clrBorder.rgbAdd ;
		//
		glUniform1f( u_fpInverseNormal, -1.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpInverseNormal)" ) ;
		m_fpInverseNormal = -1.0f ;
		//
		GLfloat	fpBorderOffset =
			(GLfloat) (zTarget * fpBorderCoefficient[0]
									+ fpBorderCoefficient[1]) ;
		glUniform1f( u_fpBorderOffset, - fpBorderOffset ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBorderOffset)" ) ;
	}
	else
	{
		glUniform1f( u_fpInverseNormal, 1.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpInverseNormal)" ) ;
		m_fpInverseNormal = 1.0f ;
		//
		glUniform1f( u_fpBorderOffset, 0.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBorderOffset)" ) ;
	}
}

// テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect, uint64_t flagsShading )
{
	SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
	SGLImageRect		rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
						pglTexture = nullptr ;
	bool				flagChangeTexture = false ;
	if ( flagsShading & shadingTextureMapping )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialTexture, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialTexture.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialTexture.pImage = pTextureImage ;
			m_txiMaterialTexture.pglTexture = pglTexture ;
			m_txiMaterialTexture.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialTexture.pglTexture ;
			rectRefTexture = m_txiMaterialTexture.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialTexture.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialTexture.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialTexture.pImage == nullptr )
	{
		ESLAssert( pTextureImage == nullptr ) ;
		pOpenGL->SetBlendMode
			( SGLOpenGLContext::BlendModeFromBlendOp
				( m_blendOperation, SGLOpenGLContext::blendProducted ) ) ;
		return	nullptr ;
	}
	else
	{
		ESLAssert( pTextureImage == nullptr ) ;
		UnbindGLTexture( m_txiMaterialTexture, iTextureNum ) ;
	}
	SGLOpenGLContext::BlendMode	modeBlend = SGLOpenGLContext::blendProducted ;
	if ( pglTexture != nullptr )
	{
		m_txiMaterialTexture.glTxTarget = pglTexture->m_paramTarget ;
		//
		#if	!defined(__API_OPEN_GL_ES__)
		if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
		{
			glEnable( pglTexture->m_paramTarget ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
		}
		#endif
		//
		glBindTexture( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		m_pOpenGL->SetBindTextureInfo
			( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
		//
		if ( flagChangeTexture )
		{
			glUniform1i( u_bMaterialTexture, 1 ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialTexture)" ) ;
			//
			glUniform1i( u_samplerMaterialTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerMaterialTexture)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vMaterialTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vMaterialTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vMaterialTextureScale)" ) ;
		}
		//
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialTexture.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialTexture.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vMaterialTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vMaterialTextureBase)" ) ;
			//
			m_txiMaterialTexture.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialTexture.ptBaseRef.y = rectRefTexture.y ;
		}
		//
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
		//
		/*
		if ( (m_nUniformTransparency == 0)
			&& !(pglTexture->m_imginf.format & formatImageFlagAlpha) )
		{
			modeBlend = SGLOpenGLContext::blendCopy ;
		}
		*/
	}
	else
	{
		ESLAssert( pTextureImage == nullptr ) ;
		glUniform1i( u_bMaterialTexture, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialTexture)" ) ;
	}
	if ( m_blendOperation == S3DRenderBufferInterface::blendDefault )
	{
		if ( flagsShading & shadingMakeBlendAdd )
		{
			pOpenGL->SetBlendMode( SGLOpenGLContext::blendAdd ) ;
		}
		else if ( modeBlend == SGLOpenGLContext::blendCopy )
		{
			pOpenGL->SetBlendMode( SGLOpenGLContext::blendCopy ) ;
		}
		else
		{
			pOpenGL->SetBlendMode( SGLOpenGLContext::blendProducted ) ;
		}
	}
	else
	{
		pOpenGL->SetBlendMode
			( SGLOpenGLContext::BlendModeFromBlendOp
				( m_blendOperation, SGLOpenGLContext::blendProducted ) ) ;
	}
	return	pglTexture ;
}

// テクスチャ（発光成分）設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindLuminousTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect,
			uint64_t flagsShading, float32_t fpLuminousApply )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( flagsShading & shadingLuminousTexture )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialLuminous, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialLuminous.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialLuminous.pImage = pTextureImage ;
			m_txiMaterialLuminous.pglTexture = pglTexture ;
			m_txiMaterialLuminous.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialLuminous.pglTexture ;
			rectRefTexture = m_txiMaterialLuminous.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialLuminous.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialLuminous.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialLuminous.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialLuminous, iTextureNum ) ;
	}
	//
	if ( pglTexture != nullptr )
	{
		m_txiMaterialLuminous.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glEnable( pglTexture->m_paramTarget ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerLuminousTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerLuminousTexture)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vLuminousTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vLuminousTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vLuminousTextureScale)" ) ;
		}
		if ( (m_fpLuminousTexture != fpLuminousApply)
			&& (fabs( m_fpLuminousTexture - fpLuminousApply ) >= 0.01) )
		{
			glUniform1f( u_fpLuminousTexture, fpLuminousApply ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(m_fpLuminousTexture)" ) ;
			m_fpLuminousTexture = fpLuminousApply ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialLuminous.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialLuminous.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vLuminousTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vLuminousTextureBase)" ) ;
			//
			m_txiMaterialLuminous.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialLuminous.ptBaseRef.y = rectRefTexture.y ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1f( u_fpLuminousTexture, 0.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(m_fpLuminousTexture)" ) ;
		//
		m_fpLuminousTexture = 0.0f ;
	}
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// 環境マッピング・テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindEnvironmentTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect,
			int typeEnvMapping, bool fSmoothing,
			const S3DMatrix * pmat3EnvMap )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		pTextureImage = nullptr ;
		typeEnvMapping = ENV_MAPPING_NOTHING ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (typeEnvMapping != ENV_MAPPING_NOTHING) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialEnvironment, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE2)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialEnvironment.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialEnvironment.pImage = pTextureImage ;
			m_txiMaterialEnvironment.pglTexture = pglTexture ;
			m_txiMaterialEnvironment.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialEnvironment.pglTexture ;
			rectRefTexture = m_txiMaterialEnvironment.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialEnvironment.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialEnvironment.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialEnvironment.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialEnvironment, iTextureNum ) ;
	}
	if ( pglTexture != nullptr )
	{
		m_txiMaterialEnvironment.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glEnable( pglTexture->m_paramTarget ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			#endif
			//
			glBindTexture( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			if ( pglTexture->m_paramTarget == GL_TEXTURE_CUBE_MAP
						/*&& (typeEnvMapping == ENV_MAPPING_CUBE)*/ )
			{
				glUniform1i( u_samplerEnvironmentCube, iTextureNum ) ;
				SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerEnvironmentCube)" ) ;
				//
				typeEnvMapping = ENV_MAPPING_CUBE ;
			}
			else
			{
				glUniform1i( u_samplerEnvironmentMapping, iTextureNum ) ;
				SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerEnvironmentMapping)" ) ;
				//
				SGLSize	sizeImage = pTextureImage->GetImageSize() ;
				glUniform2f
					( u_vEnvMapingTextureScale,
						(GLfloat) ((float) sizeImage.w / pglTexture->m_imginf.width),
						(GLfloat) ((float) sizeImage.h / pglTexture->m_imginf.height) ) ;
				SGLOpenGLContext::VerifyError( "glUniform2f(u_vEnvMapingTextureScale)" ) ;
			}
			//
			glUniform1i( u_typeEnvironmentMapping, typeEnvMapping ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentMapping)" ) ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialEnvironment.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialEnvironment.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vEnvMapingTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vEnvMapingTextureBase)" ) ;
			//
			m_txiMaterialEnvironment.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialEnvironment.ptBaseRef.y = rectRefTexture.y ;
		}
		if ( pmat3EnvMap != nullptr )
		{
			if ( m_matEnvironmentMapping != *pmat3EnvMap )
			{
				GLfloat	m3x3[3][3] ;
				for ( int i = 0; i < 3; i ++ )
				{
					m3x3[i][0] = pmat3EnvMap->m[0][i] ;
					m3x3[i][1] = pmat3EnvMap->m[1][i] ;
					m3x3[i][2] = pmat3EnvMap->m[2][i] ;
				}
				glUniformMatrix3fv
					( u_mat3EnvironmentMapping, 1, GL_FALSE, &m3x3[0][0] ) ;
				SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3EnvironmentMapping)" ) ;
				m_matEnvironmentMapping = *pmat3EnvMap ;
			}
		}
		//
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable && fSmoothing,
				pglTexture->m_flagMipmapped, false,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1i( u_typeEnvironmentMapping, ENV_MAPPING_NOTHING ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentMapping)" ) ;
	}
	return	pglTexture ;
}

SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindRefractionTexture
		( int iTextureNum,
			SGLImageObject * pRefraction,
			const SGLImageRect * pImageRect,
			int typeRefraction, bool fSmoothing,
			const S3DMatrix * pmat3EnvMap, float32_t fpRefraction )
{
	//
	// 屈折反映テクスチャ設定
	//
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		typeRefraction = ENV_MAPPING_NOTHING ;
		pRefraction = nullptr ;
	}
	if ( (pRefraction != nullptr)
		&& (typeRefraction != ENV_MAPPING_NOTHING) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialRefraction, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialRefraction.pImage != pRefraction )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pRefraction, rectRefTexture ) ;
			m_txiMaterialRefraction.pImage = pRefraction ;
			m_txiMaterialRefraction.pglTexture = pglTexture ;
			m_txiMaterialRefraction.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialRefraction.pglTexture ;
			rectRefTexture = m_txiMaterialRefraction.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialRefraction.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialRefraction.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialRefraction.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialRefraction, iTextureNum ) ;
	}
	if ( pglTexture != nullptr )
	{
		m_txiMaterialRefraction.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glEnable( pglTexture->m_paramTarget ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			if ( pglTexture->m_paramTarget == GL_TEXTURE_CUBE_MAP
				/*&& (typeRefraction == ENV_MAPPING_CUBE)*/ )
			{
				glUniform1i( u_samplerEnvironmentCube, iTextureNum ) ;
				SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerEnvironmentCube)" ) ;
			}
			else
			{
				if ( typeRefraction == ENV_MAPPING_VIEWPORT )
				{
					glUniform1i( u_samplerViewportMapping, iTextureNum ) ;
					SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerViewportMapping)" ) ;
				}
				else
				{
					glUniform1i( u_samplerEnvironmentMapping, iTextureNum ) ;
					SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerEnvironmentMapping)" ) ;
					//
					SGLSize	sizeImage = pRefraction->GetImageSize() ;
					glUniform2f
						( u_vEnvMapingTextureScale,
							(GLfloat) ((float) sizeImage.w / pglTexture->m_imginf.width),
							(GLfloat) ((float) sizeImage.h / pglTexture->m_imginf.height) ) ;
					SGLOpenGLContext::VerifyError( "glUniform2f(u_vEnvMapingTextureScale)" ) ;
				}
			}
			glUniform1i( u_typeEnvironmentRefraction, typeRefraction ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentRefraction)" ) ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialRefraction.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialRefraction.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vEnvMapingTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vEnvMapingTextureBase)" ) ;
			//
			m_txiMaterialRefraction.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialRefraction.ptBaseRef.y = rectRefTexture.y ;
		}
		if ( pmat3EnvMap != nullptr )
		{
			if ( m_matEnvironmentMapping != *pmat3EnvMap )
			{
				GLfloat	m3x3[3][3] ;
				for ( int i = 0; i < 3; i ++ )
				{
					m3x3[i][0] = pmat3EnvMap->m[0][i] ;
					m3x3[i][1] = pmat3EnvMap->m[1][i] ;
					m3x3[i][2] = pmat3EnvMap->m[2][i] ;
				}
				glUniformMatrix3fv
					( u_mat3EnvironmentMapping, 1, GL_FALSE, &m3x3[0][0] ) ;
				SGLOpenGLContext::VerifyError( "glUniformMatrix3fv(u_mat3EnvironmentMapping)" ) ;
				m_matEnvironmentMapping = *pmat3EnvMap ;
			}
		}
		if ( m_fpRefractionRatio != fpRefraction )
		{
			glUniform1f( u_fpRefractionRatio, fpRefraction ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpRefractionRatio)" ) ;
			m_fpRefractionRatio = fpRefraction ;
		}
		//
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable && fSmoothing,
				false, false, pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1i( u_typeEnvironmentRefraction, ENV_MAPPING_NOTHING ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_typeEnvironmentRefraction)" ) ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	//
	return	pglTexture ;
}

SGLOpenGLTextureBuffer::GLResource * SGLOpenGLDefaultShader::BindViewportDepthTexture
	( int iTextureNum, SGLImageObject * pDepth,
		const SGLImageRect * pImageRect, bool fSmoothing )
{
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( pDepth != nullptr )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialViewportDepth, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialViewportDepth.pImage != pDepth )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pDepth, rectRefTexture ) ;
			m_txiMaterialViewportDepth.pImage = pDepth ;
			m_txiMaterialViewportDepth.pglTexture = pglTexture ;
			m_txiMaterialViewportDepth.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialViewportDepth.pglTexture ;
			rectRefTexture = m_txiMaterialViewportDepth.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialViewportDepth.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialViewportDepth.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialViewportDepth.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialViewportDepth, iTextureNum ) ;
	}
	if ( pDepth != nullptr )
	{
		m_txiMaterialViewportDepth.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glEnable( pglTexture->m_paramTarget ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerViewportDepth, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerViewportDepth)" ) ;
			//
			glUniform2f
				( u_vViewportUnit,
					(GLfloat) (1.0 / pglTexture->m_imginf.width),
					(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vViewportUnit)" ) ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable && fSmoothing,
				false, false, pglTexture->m_paramTarget ) ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// 法線マッピング・テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource * SGLOpenGLDefaultShader::BindNormalTexture
	( int iTextureNum, SGLImageObject * pTextureImage,
		const SGLImageRect * pImageRect,
		uint64_t flagsShading, float32_t fpNormalApply )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		flagsShading &= ~shadingNormalTexture ;
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (flagsShading & shadingNormalTexture) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialNormal, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialNormal.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialNormal.pImage = pTextureImage ;
			m_txiMaterialNormal.pglTexture = pglTexture ;
			m_txiMaterialNormal.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialNormal.pglTexture ;
			rectRefTexture = m_txiMaterialNormal.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialNormal.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialNormal.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialNormal.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialNormal, iTextureNum ) ;
	}
	if ( pTextureImage != nullptr )
	{
		m_txiMaterialNormal.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
			{
				glEnable( pglTexture->m_paramTarget ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerNormalTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerNormalTexture)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vNormalTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vNormalTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vNormalTextureScale)" ) ;
		}
		if ( (m_fpNormalTexture != fpNormalApply)
			&& (fabs(m_fpNormalTexture - fpNormalApply) >= 0.01) )
		{
			glUniform1f( u_fpNormalTexture, fpNormalApply ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpNormalTexture)" ) ;
			m_fpNormalTexture = fpNormalApply ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialNormal.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialNormal.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vNormalTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vNormalTextureBase)" ) ;
			//
			m_txiMaterialNormal.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialNormal.ptBaseRef.y = rectRefTexture.y ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1f( u_fpNormalTexture, 0.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBumpMapping)" ) ;
		//
		m_fpNormalTexture = 0.0f ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// 標高マッピング・テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindHeightTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect,
			uint64_t flagsShading, float32_t fpOffsetHeight )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		flagsShading &= ~shadingHeightTexture ;
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (flagsShading & shadingHeightTexture) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialHeight, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialHeight.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialHeight.pImage = pTextureImage ;
			m_txiMaterialHeight.pglTexture = pglTexture ;
			m_txiMaterialHeight.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialHeight.pglTexture ;
			rectRefTexture = m_txiMaterialHeight.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialHeight.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialHeight.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialHeight.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialHeight, iTextureNum ) ;
	}
	if ( pTextureImage != nullptr )
	{
		m_txiMaterialHeight.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
			{
				glEnable( pglTexture->m_paramTarget ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerHeightTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerHeightTexture)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vHeightTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vHeightTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vBumpMapingTextureScale)" ) ;
		}
		if ( m_fpHeightTexture != fpOffsetHeight )
		{
			glUniform1f( u_fpBumpHeight, fpOffsetHeight ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBumpHeight)" ) ;
			m_fpHeightTexture = fpOffsetHeight ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialHeight.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialHeight.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vHeightTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vHeightTextureBase)" ) ;
			//
			m_txiMaterialHeight.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialHeight.ptBaseRef.y = rectRefTexture.y ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1f( u_fpBumpHeight, 0.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBumpHeight)" ) ;
		//
		m_fpHeightTexture = 0.0f ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// αマッピング・テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindAlphaTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect, uint64_t flagsShading,
			float32_t fpAlphaCoefficient, float32_t fpAlphaBase )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		flagsShading &= ~shadingAlphaTexture ;
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (flagsShading & shadingAlphaTexture) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialAlpha, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialAlpha.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialAlpha.pImage = pTextureImage ;
			m_txiMaterialAlpha.pglTexture = pglTexture ;
			m_txiMaterialAlpha.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialAlpha.pglTexture ;
			rectRefTexture = m_txiMaterialAlpha.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialAlpha.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialAlpha.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialAlpha.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialAlpha, iTextureNum ) ;
	}
	if ( pTextureImage != nullptr )
	{
		m_txiMaterialAlpha.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
			{
				glEnable( pglTexture->m_paramTarget ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerAlphaMapping, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerBumpMapping)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vAlphaMapingTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vAlphaMapingTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vAlphaMapingTextureScale)" ) ;
		}
		glUniform1i( u_bMaterialAlphaTexture, 1 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialAlphaTexture)" ) ;
		//
		if ( m_fpAlphaCoefficient != fpAlphaCoefficient )
		{
			glUniform1f( u_fpAlphaCoefficient, fpAlphaCoefficient ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpAlphaCoefficient)" ) ;
			m_fpAlphaCoefficient = fpAlphaCoefficient ;
		}
		if ( m_fpAlphaBase != fpAlphaBase )
		{
			glUniform1f( u_fpAlphaBase, fpAlphaBase ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpAlphaBase)" ) ;
			m_fpAlphaBase = fpAlphaBase ;
		}
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialAlpha.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialAlpha.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vAlphaMapingTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vAlphaMapingTextureBase)" ) ;
			//
			m_txiMaterialAlpha.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialAlpha.ptBaseRef.y = rectRefTexture.y ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1i( u_bMaterialAlphaTexture, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialAlphaTexture)" ) ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// スペキュラ・粗さ・反射率テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindSpecularTexture
		( int iTextureNum, SGLImageObject * pTextureImage,
			const SGLImageRect * pImageRect, uint64_t flagsShading )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		flagsShading &= ~shadingSpecularMapping ;
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (flagsShading & shadingSpecularMapping) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialSpecular, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialSpecular.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialSpecular.pImage = pTextureImage ;
			m_txiMaterialSpecular.pglTexture = pglTexture ;
			m_txiMaterialSpecular.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialSpecular.pglTexture ;
			rectRefTexture = m_txiMaterialSpecular.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialSpecular.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialSpecular.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialSpecular.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialSpecular, iTextureNum ) ;
	}
	if ( pTextureImage != nullptr )
	{
		m_txiMaterialSpecular.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
			{
				glEnable( pglTexture->m_paramTarget ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerSpecularTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerSpecularTexture)" ) ;
			//
			if ( flagsShading & shadingNormalizedUVScale )
			{
				glUniform2f( u_vSpecularTextureScale, 1.0f, 1.0f ) ;
			}
			else
			{
				glUniform2f
					( u_vSpecularTextureScale,
						(GLfloat) (1.0 / pglTexture->m_imginf.width),
						(GLfloat) (1.0 / pglTexture->m_imginf.height) ) ;
			}
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vSpecularTextureScale)" ) ;
		}
		glUniform1i( u_bMaterialSpecularTexture, 1 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialSpecularTexture)" ) ;
		//
		if ( pImageRect != nullptr )
		{
			rectRefTexture.x += pImageRect->x ;
			rectRefTexture.y += pImageRect->y ;
		}
		if ( flagChangeTexture
			|| (m_txiMaterialSpecular.ptBaseRef.x != rectRefTexture.x)
			|| (m_txiMaterialSpecular.ptBaseRef.y != rectRefTexture.y) )
		{
			glUniform2f
				( u_vSpecularTextureBase,
					(GLfloat) ((double) rectRefTexture.x
									/ pglTexture->m_imginf.width),
					(GLfloat) ((double) rectRefTexture.y
									/ pglTexture->m_imginf.height) ) ;
			SGLOpenGLContext::VerifyError( "glUniform2f(u_vSpecularTextureBase)" ) ;
			//
			m_txiMaterialSpecular.ptBaseRef.x = rectRefTexture.x ;
			m_txiMaterialSpecular.ptBaseRef.y = rectRefTexture.y ;
		}
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1i( u_bMaterialSpecularTexture, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialSpecularTexture)" ) ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// 大域ライトマップAOテクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLDefaultShader::BindGlobalAOTexture
		( int iTextureNum, SGLImageObject * pTextureImage, uint64_t flagsShading )
{
	if ( iTextureNum >= m_pOpenGL->m_maxTextureImages )
	{
		flagsShading &= ~shadingGlobalAOLightMap ;
		pTextureImage = nullptr ;
	}
	SGLImageRect	rectRefTexture ;
	SGLOpenGLTextureBuffer::GLResource *
					pglTexture = nullptr ;
	bool			flagChangeTexture = false ;
	if ( (pTextureImage != nullptr)
		&& (flagsShading & shadingGlobalAOLightMap) )
	{
		flagChangeTexture =
			UnbindGLTexture( m_txiMaterialLightMapAO, iTextureNum ) ;
		//
		glActiveTexture( GL_TEXTURE0 + iTextureNum ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		m_pOpenGL->m_iTempTexture = (size_t) iTextureNum ;
		//
		if ( m_txiMaterialLightMapAO.pImage != pTextureImage )
		{
			pglTexture = SGLOpenGLTextureBuffer::CommitGLTextureRsrc
							( m_pOpenGL, pTextureImage, rectRefTexture ) ;
			m_txiMaterialLightMapAO.pImage = pTextureImage ;
			m_txiMaterialLightMapAO.pglTexture = pglTexture ;
			m_txiMaterialLightMapAO.rectRef = rectRefTexture ;
			flagChangeTexture = true ;
		}
		else
		{
			pglTexture = m_txiMaterialLightMapAO.pglTexture ;
			rectRefTexture = m_txiMaterialLightMapAO.rectRef ;
			flagChangeTexture |=
				(m_txiMaterialLightMapAO.iTextureNum != iTextureNum) ;
		}
		m_txiMaterialLightMapAO.iTextureNum = iTextureNum ;
	}
	else if ( m_txiMaterialLightMapAO.pImage == nullptr )
	{
		return	nullptr ;
	}
	else
	{
		UnbindGLTexture( m_txiMaterialLightMapAO, iTextureNum ) ;
	}
	if ( pTextureImage != nullptr )
	{
		m_txiMaterialLightMapAO.glTxTarget = pglTexture->m_paramTarget ;
		//
		if ( flagChangeTexture )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			if ( pglTexture->m_paramTarget != GL_TEXTURE_2D_ARRAY )
			{
				glEnable( pglTexture->m_paramTarget ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			glBindTexture
				( pglTexture->m_paramTarget, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			m_pOpenGL->SetBindTextureInfo
				( iTextureNum, pglTexture, pglTexture->m_paramTarget ) ;
			//
			glUniform1i( u_samplerGlobalAOTexture, iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glUniform1i(u_samplerGlobalAOTexture)" ) ;
		}
		glUniform1i( u_bMaterialGlobalAOTexture, 1 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialGlobalAOTexture)" ) ;
		//
		SetGLTextureParameter
			( m_fEnableTextureSmoothing
					&& pglTexture->m_flagSmoothable
					&& ((flagsShading & shadingTextureSmoothing) != 0),
				pglTexture->m_flagMipmapped,
				((flagsShading & shadingTextureTiling) != 0)
					|| pglTexture->m_flagReqTiling,
				pglTexture->m_paramTarget ) ;
	}
	else
	{
		glUniform1i( u_bMaterialGlobalAOTexture, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_bMaterialGlobalAOTexture)" ) ;
	}
	//
	glActiveTexture( GL_TEXTURE0 ) ;
	SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTURE0)" ) ;
	return	pglTexture ;
}

// テクスチャ割り当て解除
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLDefaultShader::UnbindGLTexture
		( SGLOpenGLDefaultShader::TextureInfo& txinf, int iAllocated ) const
{
	bool	flagUnbind = false ;
	if ( txinf.pImage != nullptr )
	{
		if ( iAllocated <= txinf.iTextureNum )
		{
			glActiveTexture( GL_TEXTURE0 + txinf.iTextureNum ) ;
			SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
			//
			glBindTexture( txinf.glTxTarget, 0 ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			#if	!defined(__API_OPEN_GL_ES__)
			if ( txinf.glTxTarget != GL_TEXTURE_2D_ARRAY )
			{
				glDisable( txinf.glTxTarget ) ;
				SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
			}
			#endif
			//
			m_pOpenGL->SetBindTextureInfo( txinf.iTextureNum, nullptr ) ;
			flagUnbind = true ;
		}
	}
	GLenum	glTxTarget ;
	if ( m_pOpenGL->IsBindingTexture( iAllocated, glTxTarget )
							&& (txinf.glTxTarget != glTxTarget) )
	{
		glActiveTexture( GL_TEXTURE0 + iAllocated ) ;
		SGLOpenGLContext::VerifyError( "glActiveTexture(GL_TEXTUREx)" ) ;
		//
		glBindTexture( glTxTarget, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
		//
		#if	!defined(__API_OPEN_GL_ES__)
		if ( glTxTarget != GL_TEXTURE_2D_ARRAY )
		{
			glDisable( glTxTarget ) ;
			SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
		}
		#endif
		//
		m_pOpenGL->SetBindTextureInfo( iAllocated, nullptr ) ;
		flagUnbind = true ;
	}
	txinf.pImage = nullptr ;
	return	flagUnbind ;
}

// テクスチャスムーシング・クリッピング設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetGLTextureParameter
		( bool fSmoothing, bool fMipmap, bool fTiling, GLenum glTxTarget ) const
{
	if ( fSmoothing )
	{
		if ( fMipmap )
		{
			SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
			if ( (pOpenGL != nullptr) && pOpenGL->m_flagAnisotropicExt )
			{
				glTexParameterf
					( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
						esl_fminf( pOpenGL->m_maxAnisotropic, m_fpAnisotropy ) ) ;
				SGLOpenGLContext::VerifyError
					( "glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT)" ) ;
			}
			glTexParameteri
				( glTxTarget,
					GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
			glTexParameteri
				( glTxTarget,
					GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
		}
		else
		{
			glTexParameteri
				( glTxTarget,
					GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
			glTexParameteri
				( glTxTarget,
					GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
			SGLOpenGLContext::VerifyError
				( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
		}
	}
	else
	{
		glTexParameteri
			( glTxTarget,
				GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
		glTexParameteri
			( glTxTarget,
				GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
	}
	if ( fTiling )
	{
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
	}
	else
	{
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
		glTexParameteri
			( glTxTarget,
					GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
	}
}

// プリミティブリストをレンダリング
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::AddIndexedPrimitiveList
	( uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( pvVertex == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	if ( m_pOpenGL->m_pBindingVBO != nullptr )
	{
		DisableAllVertexPointer() ;
		//
		glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
		//
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
		//
		m_pOpenGL->m_pBindingVBO = nullptr ;
		m_pOpenGL->m_pRsrcOfVBO = nullptr ;
	}
	if ( m_fpMorphApplication > 0.0f )
	{
		glUniform1f( u_fpMorphApplication, 0.0f ) ;
		SGLOpenGLContext::VerifyError( "glUniform1f(u_fpMorphApplication)" ) ;
		m_fpMorphApplication = 0.0f ;
	}
	if ( m_nBoneCount != 0 )
	{
		glUniform1i( u_nBoneCount, 0 ) ;
		SGLOpenGLContext::VerifyError( "glUniform1i(u_nBoneCount)" ) ;
		m_nBoneCount = 0 ;
	}
	EnableVertexAttribArray( a_vVertexPosition ) ;
	VertexAttribPointer
		( a_vVertexPosition, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), pvVertex ) ;
	//
	if ( (pvNormal == nullptr)
		&& (GetPrimitiveVertexCount(typePrimitive) == 3) )
	{
		if ( m_tnbNormals.SetForIndexedPrimitiveList
				( typePrimitive,
					countIndex / 3, countVertex,
					pvVertex, pvUVMap, pIndexedList ) )
		{
			pvNormal = m_tnbNormals.GetNormalBuffer() ;
		}
	}
	if ( pvNormal != nullptr )
	{
		EnableVertexAttribArray( a_vVertexNormal ) ;
		VertexAttribPointer
			( a_vVertexNormal, 3, GL_FLOAT,
					GL_TRUE, sizeof(S3DVector4), pvNormal ) ;
	}
	else
	{
		// ダミー
		EnableVertexAttribArray( a_vVertexNormal ) ;
		VertexAttribPointer
			( a_vVertexNormal, 3, GL_FLOAT,
					GL_TRUE, sizeof(S3DVector4), pvVertex ) ;
		pvNormal = pvVertex ;
	}
	//
	const void *	pUVMap = pvUVMap ;
	if ( pvUVMap != nullptr )
	{
		EnableVertexAttribArray( a_vTextureCoord ) ;
		VertexAttribPointer
			( a_vTextureCoord, 2, GL_FLOAT,
					GL_FALSE, sizeof(S2DVector), pvUVMap ) ;
	}
	else
	{
		// ダミー
		EnableVertexAttribArray( a_vTextureCoord ) ;
		VertexAttribPointer
			( a_vTextureCoord, 2, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), pvVertex ) ;
		pUVMap = pvVertex ;
	}
	//
	if ( pColor == nullptr )
	{
		pColor = AllocateDummyVertexColorBuffer( countVertex ) ;
	}
	EnableVertexAttribArray( a_vVertexMulColor ) ;
	VertexAttribPointer
		( a_vVertexMulColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), &(pColor->rgbMul) ) ;
	EnableVertexAttribArray( a_vVertexAddColor ) ;
	VertexAttribPointer
		( a_vVertexAddColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), &(pColor->rgbAdd) ) ;
	//
	S3DTemporaryTextureAxisBuffer	bufTempAxis ;
	const void *	pVertexMappingX = nullptr ;
	const void *	pVertexMappingY = nullptr ;
	if ( (nFlags & S3DRenderBuffer::renderAutoTexAxis)
		&& (pvUVMap != nullptr)
		&& (a_vVertexMappingX >= 0) && (a_vVertexMappingY >= 0) )
	{
		if ( bufTempAxis.SetForIndexedPrimitiveList
			( typePrimitive,
				countIndex / 3, countVertex,
				pvVertex, pvUVMap, pIndexedList ) )
		{
			EnableVertexAttribArray( a_vVertexMappingX ) ;
			VertexAttribPointer
				( a_vVertexMappingX, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4),
					bufTempAxis.GetBufferAxisX() ) ;
			pVertexMappingX = bufTempAxis.GetBufferAxisX() ;
			//
			EnableVertexAttribArray( a_vVertexMappingY ) ;
			VertexAttribPointer
				( a_vVertexMappingY, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4),
					bufTempAxis.GetBufferAxisY() ) ;
			pVertexMappingY = bufTempAxis.GetBufferAxisY() ;
		}
	}
	m_enabledVertexAttr = true ;
	//
	if ( m_enabledMorphing )
	{
		EnableVertexAttribArray( a_vMorphPosition ) ;
		VertexAttribPointer
			( a_vMorphPosition, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), pvVertex ) ;
		EnableVertexAttribArray( a_vMorphNormal ) ;
		VertexAttribPointer
			( a_vMorphNormal, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), pvNormal ) ;
		//
		EnableVertexAttribArray( a_vMorphTextureCoord ) ;
		VertexAttribPointer
			( a_vMorphTextureCoord, 2, GL_FLOAT,
				GL_FALSE, sizeof(S2DVector), pUVMap ) ;
		//
		EnableVertexAttribArray( a_vMorphMulColor ) ;
		VertexAttribPointer
			( a_vMorphMulColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), &(pColor->rgbMul) ) ;
		EnableVertexAttribArray( a_vMorphAddColor ) ;
		VertexAttribPointer
			( a_vMorphAddColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), &(pColor->rgbAdd) ) ;
		if ( pVertexMappingX != nullptr )
		{
			EnableVertexAttribArray( a_vMorphMappingX ) ;
			VertexAttribPointer
				( a_vVertexMappingX, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), pVertexMappingX ) ;
		}
		if ( pVertexMappingY != nullptr )
		{
			EnableVertexAttribArray( a_vMorphMappingY ) ;
			VertexAttribPointer
				( a_vVertexMappingX, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), pVertexMappingY ) ;
		}
		m_enabledVertex4Morphing = true ;
	}
	//
	if ( m_enabledInstancing )
	{
		EnableVertexAttribArray( a_matInstancingModelView[0] ) ;
		VertexAttribPointer
			( a_matInstancingModelView[0], 4, GL_FLOAT,
				GL_FALSE, sizeof(GLfloat)*16, &m_mat4DummyInstancing[0][0], 1 ) ;
		EnableVertexAttribArray( a_matInstancingModelView[1] ) ;
		VertexAttribPointer
			( a_matInstancingModelView[1], 4, GL_FLOAT,
				GL_FALSE, sizeof(GLfloat)*16, &m_mat4DummyInstancing[1][0], 1 ) ;
		EnableVertexAttribArray( a_matInstancingModelView[2] ) ;
		VertexAttribPointer
			( a_matInstancingModelView[2], 4, GL_FLOAT,
				GL_FALSE, sizeof(GLfloat)*16, &m_mat4DummyInstancing[2][0], 1 ) ;
		EnableVertexAttribArray( a_vInstancingMulColor ) ;
		VertexAttribPointer
			( a_vInstancingMulColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor),
					&(m_colorDummyInstancing.rgbMul), 1 ) ;
		EnableVertexAttribArray( a_vInstancingAddColor ) ;
		VertexAttribPointer
			( a_vInstancingAddColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor),
					&(m_colorDummyInstancing.rgbAdd), 1 ) ;
		m_enabledInstancingAttr = true ;
	}
	//
	GLenum	modeDraw = SGLOpenGLVertexBuffer::PrimitiveTypeToGL( typePrimitive ) ;
#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL >= 18)
	if ( m_enabledInstancing )
	{
		if ( pIndexedList != nullptr )
		{
			if ( m_pOpenGL->m_flagElementIndexUint )
			{
				glDrawElementsInstanced
					( modeDraw, (GLsizei) countIndex,
						GL_UNSIGNED_INT, pIndexedList, 1 ) ;
				SGLOpenGLContext::VerifyError
						( "glDrawElementsInstanced(,,GL_UNSIGNED_INT)" ) ;
			}
			else
			{
				glDrawElementsInstanced
					( modeDraw, (GLsizei) countIndex,
						GL_UNSIGNED_SHORT,
						ElementIndexToUint16( pIndexedList, countIndex ), 1 ) ;
				SGLOpenGLContext::VerifyError
						( "glDrawElementsInstanced(,,GL_UNSIGNED_SHORT)" ) ;
			}
		}
		else
		{
			glDrawArraysInstanced( modeDraw, 0, (GLsizei) countVertex, 1 ) ;
			SGLOpenGLContext::VerifyError( "glDrawArraysInstanced(GL_TRIANGLE_STRIP)" ) ;
		}
	}
	else
#endif
	if ( pIndexedList != nullptr )
	{
		if ( m_pOpenGL->m_flagElementIndexUint )
		{
			glDrawElements
				( modeDraw, (GLsizei) countIndex,
					GL_UNSIGNED_INT, pIndexedList ) ;
			SGLOpenGLContext::VerifyError
					( "glDrawElements(,,GL_UNSIGNED_INT)" ) ;
		}
		else
		{
			glDrawElements
				( modeDraw, (GLsizei) countIndex,
					GL_UNSIGNED_SHORT,
					ElementIndexToUint16( pIndexedList, countIndex ) ) ;
			SGLOpenGLContext::VerifyError
					( "glDrawElements(,,GL_UNSIGNED_SHORT)" ) ;
		}
	}
	else
	{
		glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
		SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
	}
	m_pOpenGL->m_pflog.countDrawCall ++ ;
	m_pOpenGL->m_pflog.countDrawInstance ++ ;
	m_pOpenGL->m_pflog.countDrawVertex += countVertex ;
	m_pOpenGL->m_pflog.countTransmitVertex += countVertex ;
	//
	DisableAllVertexPointer() ;
	//
	return	sglErrSuccess ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLDefaultShader::AddVertexBuffer
	( const S4DDMatrix& matBase,
		const S3DColor& colorBase,
		unsigned int nTransparency, uint32_t nFlags,
		S3DRenderBuffer * pRBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing,
		S3DVertexVariantBuffer*const* ppInstancingVVB )
{
	SGLOpenGLVertexBuffer *
			pBuffer = SGLOpenGLVertexBuffer::Commit( pRBuffer ) ;
	ESLAssert( pBuffer != nullptr ) ;
	//
	size_t	nVBOMeshCount = pRBuffer->GetMeshCount() ;
	if ( iEnd < 0 )
	{
		iEnd = (ssize_t) nVBOMeshCount ;
	}
	SGLOpenGLContext *	pOpenGL = m_pOpenGL ;
	ESLAssert( pOpenGL != nullptr ) ;
	ESLAssert( SGLOpenGLContext::GetCurrentGLContext() == pOpenGL ) ;
	//
	if ( pOpenGL->m_pBindingVBO != pBuffer )
	{
		SGLOpenGLVertexBuffer::GLResource *
					pRsrc = pBuffer->CommitResourceAs( pOpenGL ) ;
		if ( pRsrc == nullptr )
		{
			return	sglErrFailed ;
		}
		DisableAllVertexPointer() ;
		pRsrc->BindBuffer() ;
		//
		pOpenGL->m_pBindingVBO = pBuffer ;
		pOpenGL->m_pRsrcOfVBO = pRsrc ;
		pOpenGL->m_typeVBOIndex = pRsrc->m_typeElementIndex ;
		pOpenGL->m_iMeshOfVBO = (size_t) -1 ;
	}
	S3DMaterial *	pLastMaterial = nullptr ;
	for ( size_t i = iFirst; i < (size_t) iEnd; i ++ )
	{
		SGLOpenGLVertexBuffer::ENTRY_INFO *
							pei = pBuffer->GetEntryInfoAt( i ) ;
		if ( pei == nullptr )
		{
			continue ;
		}
		S3DRenderBuffer::RENDER_ENTRY *	pre = pei->pre ;
		ESLAssert( pre != nullptr ) ;
		if ( !pre->flagRenderable )
		{
			continue ;
		}
		//
		// 描画情報
		//
		S3DMaterial *		pMaterial = pre->pMaterial ;
		S3DPrimitiveType	typePrimitive = (S3DPrimitiveType) pre->nType ;
		size_t				nDrawIndexCount = pre->countIndex ;
		size_t				ofsElement = pei->ofsElement ;
		const GLenum		modeDraw = SGLOpenGLVertexBuffer::
										PrimitiveTypeToGL( typePrimitive ) ;
		//
		// 座標変換準備
		//
		S4DDMatrix		matMesh = matBase ;
		S3DColor		colorEffect = colorBase ;
		unsigned int	nTransEffect = nTransparency ;
		if ( pre->pTransform != nullptr )
		{
			S3DRenderBuffer::Transformation *	pTrans = pre->pTransform ;
			S4DDMatrix	matTrans ;
			Matrix4x4From3x3
				( matTrans, pTrans->matTransform, pTrans->vTransform ) ;
			matMesh = matBase * matTrans ;
			SetModelViewMatrix( matMesh ) ;
			//
			colorEffect = colorBase * pTrans->colorEffect ;
			nTransEffect = 0x100 - (0x100 - nTransparency)
								* (0x100 - pTrans->nTransparency) / 0x100 ;
		}
		else
		{
			SetModelViewMatrix( matMesh ) ;
		}
		if ( pMaterial->m_attrSurface.flagsShading & shadingDisableColorEffect )
		{
			colorEffect.rgbMul.ui32 = 0xFFFFFFFF ;
			colorEffect.rgbAdd.ui32 = 0 ;
		}
		//
		// 見かけ上の座標からサブメッシュを選択
		//
		S4DVector	vMeshPos
						( matMesh.m[0][3], matMesh.m[1][3],
							matMesh.m[2][3], matMesh.m[3][3] ) ;
		m_mat4Camera.RevolveVector( vMeshPos ) ;
		//
		SelectSubMesh( pei, pre, nDrawIndexCount, ofsElement, vMeshPos ) ;
		//
		// モーフィング・ボーン適用処理
		//
		SGLOpenGLVertexBuffer::GLResource *
						pRsrc = pOpenGL->m_pRsrcOfVBO ;
		const size_t	countVertex = pre->countVertex ;
		//
		VertexSetupContext	vsc ;
		UpdateShapeMatrix
			( vsc, pOpenGL, pRBuffer, pBuffer, i, nVBOMeshCount,
				pRsrc, pei, pre,
				nInstancing, pmatInstancing,
				pColorInstancing, ppInstancingVVB ) ;
		//
		// 輪郭描画
		//
		if ( !m_fShaderNoDrawBorder
			&& (m_fShaderForceBorder
				|| (pMaterial
					&& (pMaterial->m_attrSurface.flagsShading
										& shadingDrawOffsetBorder))) )
		{
			SetBorderOffset
				( true, (float32_t) matMesh.m[2][3],
						colorEffect, pMaterial->m_attrSurface ) ;
			//
			SetMaterial
				( pMaterial, true,
					shadingMethodMask,
					shadingTextureTriming
						| shadingSingleSidePlane ) ;
			//
			DrawVertexBuffer
				( vsc, pOpenGL, modeDraw, nDrawIndexCount, ofsElement ) ;
			//
			SetBorderOffset
				( false, 0.0f, colorEffect, pMaterial->m_attrSurface ) ;
			pLastMaterial = nullptr ;
		}
		//
		// 裏面描画
		//
		SetColorEffect( &colorBase, nTransEffect ) ;
		//
		if ( pMaterial && pMaterial->m_flagBack
			&& (m_faceCulling == S3DRenderBufferInterface::faceCullingDefault) )
		{
			glUniform1f( u_fpInverseNormal, -1.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpInverseNormal)" ) ;
			m_fpInverseNormal = -1.0f ;
			//
			SetMaterial( pMaterial, true ) ;
			//
			DrawVertexBuffer
				( vsc, pOpenGL, modeDraw, nDrawIndexCount, ofsElement ) ;
			//
			glUniform1f( u_fpInverseNormal, 1.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpInverseNormal)" ) ;
			m_fpInverseNormal = 1.0f ;
			//
			pLastMaterial = nullptr ;
		}
		//
		// 表面描画
		//
		if ( pLastMaterial != pMaterial )
		{
			SetMaterial( pMaterial ) ;
			pLastMaterial = pMaterial ;
		}
		if ( m_fShaderSurfaceOffset )
		{
			GLfloat	fpSurfaceOffset =
				(GLfloat) ((float32_t) matMesh.m[2][3]
									* m_fpBorderCoefficient[0]
										+ m_fpBorderCoefficient[1]) ;
			glUniform1f( u_fpBorderOffset, fpSurfaceOffset ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBorderOffset)" ) ;
		}
		//
		DrawVertexBuffer
			( vsc, pOpenGL, modeDraw, nDrawIndexCount, ofsElement ) ;
		//
		if ( m_fShaderSurfaceOffset )
		{
			glUniform1f( u_fpBorderOffset, 0.0f ) ;
			SGLOpenGLContext::VerifyError( "glUniform1f(u_fpBorderOffset)" ) ;
		}
		//
		FinishShapeMatrix( vsc ) ;
	}
	return	sglErrSuccess ;
}

// モーフィング・ボーン適用・頂点セットアップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::UpdateShapeMatrix
	( VertexSetupContext& vsc,
		SGLOpenGLContext * pOpenGL,
		S3DRenderBuffer * pBuffer,
		SGLOpenGLVertexBuffer * pglBuffer,
		size_t iMesh, size_t nVBOMeshCount,
		SGLOpenGLVertexBuffer::GLResource * pRsrc,
		SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
		S3DRenderBuffer::RENDER_ENTRY * pre,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing,
		S3DVertexVariantBuffer*const* ppInstancingVVB )
{
	const size_t	countVertex = pre->countVertex ;
	ssize_t			iBaseMesh = -1 ;
	ssize_t			iMorphMesh = -1 ;
	float32_t		fpMorphApply = 0.0f ;
	size_t			countBone = pre->countBone ;
	//
	// ボーン・モーフィングの処理判定
	//
	vsc.flagVAO = false ;
	vsc.flagVertexTexture = false ;
	vsc.flagMultiShapeInstance = false ;
	vsc.pRsrc = pRsrc ;
	vsc.pei = pei ;
	vsc.pre = pre ;
	vsc.pVTImage = nullptr ;
	vsc.iMesh = iMesh ;
	vsc.nInstancing = nInstancing ;
	vsc.pmatInstancing = pmatInstancing ;
	vsc.pColorInstancing = pColorInstancing ;
	vsc.ppInstancingVVB = ppInstancingVVB ;
	//
	const size_t	ofsVertex = pei->ofsVertex ;
	//
	if ( m_enabledVTInstancing )
	{
		if ( pei->flagVertexTexture )
		{
			vsc.pVTImage = pglBuffer->GetVertexTextureAt( iMesh ) ;
			ESLAssert( vsc.pVTImage != nullptr ) ;
			if ( vsc.pVTImage != nullptr )
			{
				//
				// 頂点テクスチャの設定
				//
				vsc.flagMultiShapeInstance =
						(nInstancing > 0) && (ppInstancingVVB != nullptr) ;
				SetVertexTexture
					( *pei, vsc.pVTImage, vsc.flagMultiShapeInstance ) ;
				vsc.flagVertexTexture = true ;
				//
				if ( !vsc.flagMultiShapeInstance )
				{
					S3DVertexVariantBuffer *	pvvb[1] =
					{
						pBuffer
					} ;
					WriteMorphInstanceToVertexTexture
						( vsc.pVTImage, iMesh, *pei, pvvb, 1 ) ;
					WriteBoneInstanceToVertexTexture
						( vsc.pVTImage, iMesh, *pei, pvvb, 1 ) ;
				}
			}
		}
		else if ( pei->flagMustShapeByCPU )
		{
			if ( (pre->countMorph > 0)
				&& (pre->flagUpdateMorph || pre->flagUpdateBone) )
			{
				STimeCounter	timer ;
				pBuffer->MorphMeshVertics( *pre ) ;
				WriteDynamicVertexBuffer( pRsrc, ofsVertex, pre ) ;
				pOpenGL->m_pflog.countTransmitVertex += pre->countVertex ;
				pOpenGL->m_pflog.countComputeShapeByCPU += pre->countVertex ;
				pOpenGL->m_pflog.msecShapeByCPU += timer.GetRealTime() ;
			}
			else if ( (pre->countBone > 0) || pre->flagUpdateBone )
			{
				STimeCounter	timer ;
				pBuffer->TransformMeshVerticsByBone( *pre ) ;
				WriteDynamicVertexBuffer( pRsrc, ofsVertex, pre ) ;
				pOpenGL->m_pflog.countTransmitVertex += pre->countVertex ;
				pOpenGL->m_pflog.countComputeShapeByCPU += pre->countVertex ;
				pOpenGL->m_pflog.msecShapeByCPU += timer.GetRealTime() ;
			}
		}
		if ( !vsc.flagVertexTexture )
		{
			UnsetVertexTexture() ;
		}
		countBone = 0 ;
	}
	else
	{
		//
		// バーテックスシェーダーで処理できる条件判定とCPU処理
		//
		if ( (countBone > 0) && (pre->countFullBone > 1) )
		{
			countBone -= (pre->countFullBone - 1) & ~0x03 ;
		}
		bool	flagBonePossibleByGPU =
					!(pei->flagMustShapeByCPU)
					&& (countBone <= m_maxBonePalette)
					&& !((countBone > 0) && (pre->ppJointMap != nullptr)) ;
		if ( pre->countMorph > 0 )
		{
			if ( m_enabledMorphing
				&& (pre->nTargetMeshCount <= 2)
				&& flagBonePossibleByGPU )
			{
				if ( pre->nTargetMeshCount == 2 )
				{
					iBaseMesh = pre->pMorphTargetMesh[0] ;
					iMorphMesh = pre->pMorphTargetMesh[1] ;
					fpMorphApply = pre->pMorphApplication[1] ;
					//
					if ( (iMorphMesh < 0) && (iBaseMesh >= 0) )
					{
						iBaseMesh = pre->pMorphTargetMesh[1] ;
						iMorphMesh = pre->pMorphTargetMesh[0] ;
						fpMorphApply = pre->pMorphApplication[0] ;
					}
					if ( fpMorphApply == 0.0f )
					{
						iMorphMesh = iBaseMesh ;
					}
				}
				else if ( pre->nTargetMeshCount == 1 )
				{
					iBaseMesh = pre->pMorphTargetMesh[0] ;
					iMorphMesh = iBaseMesh ;
					fpMorphApply = 0.0f ;
				}
				if ( pRsrc->m_flagDynamicMesh )
				{
					RestoreDynamicVertexBuffer( pRsrc, ofsVertex, pre ) ;
					pOpenGL->m_pflog.countTransmitVertex += pre->countVertex ;
					pre->flagUpdateMorph = true ;
				}
			}
			else if ( pre->flagUpdateMorph || pre->flagUpdateBone )
			{
				STimeCounter	timer ;
				pBuffer->MorphMeshVertics( *pre ) ;
				WriteDynamicVertexBuffer( pRsrc, ofsVertex, pre ) ;
				countBone = 0 ;
				pOpenGL->m_pflog.countTransmitVertex += pre->countVertex ;
				pOpenGL->m_pflog.countComputeShapeByCPU ++ ;
				pOpenGL->m_pflog.msecShapeByCPU += timer.GetRealTime() ;
			}
			else if ( pRsrc->m_flagDynamicMesh )
			{
				// ※モーフィング・ボーンはCPUで処理済みで更新は無い
				// 　→ボーンの回転行列はシェーダ―に渡さない
				countBone = 0 ;
			}
		}
		else if ( !flagBonePossibleByGPU && pre->flagUpdateBone )
		{
	//		ESLTrace( "too many bone for a mesh.\n" ) ;
			STimeCounter	timer ;
			pBuffer->TransformMeshVerticsByBone( *pre ) ;
			WriteDynamicVertexBuffer( pRsrc, ofsVertex, pre ) ;
			countBone = 0 ;
			pOpenGL->m_pflog.countTransmitVertex += pre->countVertex ;
			pOpenGL->m_pflog.countComputeShapeByCPU ++ ;
			pOpenGL->m_pflog.msecShapeByCPU += timer.GetRealTime() ;
		}
		if ( m_enabledMorphing )
		{
			if ( fpMorphApply != m_fpMorphApplication )
			{
				glUniform1f( u_fpMorphApplication, fpMorphApply ) ;
				SGLOpenGLContext::VerifyError( "glUniform1f(u_fpMorphApplication)" ) ;
				m_fpMorphApplication = fpMorphApply ;
			}
		}
		if ( (countBone > 0) && flagBonePossibleByGPU )
		{
			//
			// ボーン回転行列の設定
			//
			SetBoneMatrixUniform( pre ) ;
		}
		else
		{
			if ( m_nBoneCount != 0 )
			{
				glUniform1i( u_nBoneCount, 0 ) ;
				SGLOpenGLContext::VerifyError( "glUniform1i(u_nBoneCount)" ) ;
				m_nBoneCount = 0 ;
			}
		}
	}
	//
	// 頂点バッファ設定
	//
	if ( m_enabledInstancing )
	{
		// インスタンスバッファの確保と書き込み
		WriteInstancingBuffer
			( pRsrc, nInstancing, pmatInstancing, pColorInstancing ) ;
	}
	if ( pOpenGL->m_flagSupportedVAO )
	{
		// VAO 使用
		SGLOpenGLVertexBuffer::ArrayBufferEntry *
			pabe = pRsrc->GetArrayBufferAt( this, iMesh, nVBOMeshCount ) ;
		ESLAssert( pabe != nullptr ) ;
		if ( pabe->m_glArrayBuffer == 0 )
		{
			glGenVertexArrays( 1, &(pabe->m_glArrayBuffer) ) ;
			SGLOpenGLContext::VerifyError( "glGenVertexArrays(1)" ) ;
			pabe->m_fUpdate = true ;
		}
		glBindVertexArray( pabe->m_glArrayBuffer ) ;
		vsc.flagVAO = true ;
		//
		if ( pabe->m_fUpdate
			|| (pabe->m_iBaseMesh != iBaseMesh)
			|| (pabe->m_iMorphMesh != iMorphMesh)
			|| (pabe->m_nBoneCount != countBone) )
		{
			// VAO に変更のある場合設定
			pRsrc->BindBuffer() ;
			BindVertexBuffer
				( pRsrc, pei, pre, iBaseMesh, iMorphMesh, countBone ) ;
			//
			if ( m_enabledInstancing )
			{
				BindInstancingBuffer( pRsrc ) ;
			}
			//
			pabe->m_fUpdate = false ;
			pabe->m_iBaseMesh = iBaseMesh ;
			pabe->m_iMorphMesh = iMorphMesh ;
			pabe->m_nBoneCount = countBone ;
		}
		else
		{
			EnableVertexBuffer
				( pei, pre, iBaseMesh, iMorphMesh, countBone ) ;
			//
			if ( m_enabledInstancing )
			{
				EnableInstancingBuffer() ;
			}
		}
		pOpenGL->m_iMeshOfVBO = (size_t) -1 ;
	}
	else if ( m_enabledInstancing
		|| (pOpenGL->m_iMeshOfVBO != iMesh)
		|| (pOpenGL->m_iMeshMorph0OfVBO != iBaseMesh)
		|| (pOpenGL->m_iMeshMorph1OfVBO != iMorphMesh) )
	{
		// VAO を使用しない場合で、変更のある場合設定
		pRsrc->BindBuffer() ;
		BindVertexBuffer
			( pRsrc, pei, pre, iBaseMesh, iMorphMesh, countBone ) ;
		//
		if ( m_enabledInstancing )
		{
			BindInstancingBuffer( pRsrc ) ;
		}
		//
		pOpenGL->m_iMeshOfVBO = iMesh ;
		pOpenGL->m_iMeshMorph0OfVBO = iBaseMesh ;
		pOpenGL->m_iMeshMorph1OfVBO = iMorphMesh ;
	}
}

// モーフィング・ボーン適用・頂点セットアップ処理後始末
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::FinishShapeMatrix( VertexSetupContext& vsc )
{
	if ( vsc.flagVertexTexture )
	{
		UnsetVertexTexture() ;
	}
	if ( vsc.flagVAO )
	{
		glBindVertexArray( 0 ) ;
	}
}

// モーフィングインスタンスを頂点テクスチャに書き込む
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::WriteMorphInstanceToVertexTexture
	( SGLImageObject * pImage, size_t iMesh,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount )
{
	S3DVector4 *	pvMorphInstance =
						m_aVTMorphInstance.GetArray( nCount * 2 ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pvMorphInstance[0] = S3DVector4( 0, 0, 0, 0 ) ;
		pvMorphInstance[1] = S3DVector4( 0, 0, 0, 0 ) ;
		//
		S3DVertexVariantBuffer *	pvb = ppInstancingVVB[i] ;
		if ( pvb != nullptr )
		{
			SGLOpenGLVertexBuffer::MakeMorphInstance
							( pvMorphInstance, pvb, iMesh ) ;
		}
		pvMorphInstance += 2 ;
	}
	m_aVTMorphInstance.FinishArray() ;
	//
	SGLOpenGLVertexBuffer::WriteMorphInstanceToVertexTexture
		( pImage, ei, m_aVTMorphInstance.GetConstArray(), nCount ) ;
}

// ボーンインスタンスを頂点テクスチャに書き込む
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::WriteBoneInstanceToVertexTexture
	( SGLImageObject * pImage, size_t iMesh,
		const SGLOpenGLVertexBuffer::ENTRY_INFO& ei,
		S3DVertexVariantBuffer*const* ppInstancingVVB, size_t nCount )
{
	size_t	nBufWidth = (size_t) ei.sizeVertexTex.w ;
	SGLOpenGLVertexBuffer::WriteBoneInstanceToVertexTexture
		( pImage, iMesh, ei,
			m_aVTBoneMatrixBuf.GetArray(nBufWidth),
			m_aVTBoneTransBuf.GetArray(nBufWidth),
			ppInstancingVVB, nCount ) ;
	//
	m_aVTBoneMatrixBuf.FinishArray() ;
	m_aVTBoneTransBuf.FinishArray() ;
}

// 頂点バッファにオリジナル値を書き戻す
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::RestoreDynamicVertexBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc,
		size_t ofsVertex, const S3DRenderBuffer::RENDER_ENTRY * pre )
{
	const size_t	countVertex = pre->countVertex ;
	//
	pRsrc->BindBuffer() ;
	pRsrc->WriteComposedVertexBuffer
		( ofsVertex, pre->pvVertex,
				pre->pvNormal, pre->pvUVMap, pre->pColor, countVertex ) ;
	pRsrc->m_flagDynamicMesh = false ;
}

// 一時的な頂点バッファを書き込む（CPUでボーンやモーフィング処理時）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::WriteDynamicVertexBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc,
		size_t ofsVertex, const S3DRenderBuffer::RENDER_ENTRY * pre )
{
	const size_t	countVertex = pre->countVertex ;
	//
	pRsrc->BindBuffer() ;
	if ( pRsrc->m_flagInterleaved )
	{
		pRsrc->WriteComposedVertexBuffer
			( ofsVertex, pre->pvTempVertex,
					pre->pvTempNormal,
					pre->pvTempUVMap ? pre->pvTempUVMap : pre->pvUVMap,
					pre->pvTempColor ? pre->pvTempColor : pre->pColor, countVertex ) ;
	}
	else
	{
		pRsrc->WriteComposedVertexBuffer
			( ofsVertex, pre->pvTempVertex,
					pre->pvTempNormal,
					pre->pvTempUVMap, pre->pvTempColor, countVertex ) ;
	}
	pRsrc->m_flagDynamicMesh = true ;
}

// サブメッシュを選択
//////////////////////////////////////////////////////////////////////////////
int SGLOpenGLDefaultShader::SelectSubMesh
	( const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
		const S3DRenderBuffer::RENDER_ENTRY * pre,
		size_t& nDrawIndexCount,
		size_t& ofsElement, const S4DVector& vMeshPos ) const
{
	ssize_t	iSubMeshIndex = pre->iSubMeshSelector + 1 ;
	if ( iSubMeshIndex <= 0 )
	{
		iSubMeshIndex =
			(int) floor( vMeshPos.z / (pre->fpSubMeshDensity
								* m_vProjectScreen.z * vMeshPos.w) ) ;
	}
	if ( iSubMeshIndex >= 1 )
	{
		if ( iSubMeshIndex > S3DVertexBufferInterface::countSubMesh )
		{
			iSubMeshIndex = S3DVertexBufferInterface::countSubMesh ;
		}
		while ( pei->ofsSubElements[iSubMeshIndex - 1] == 0 )
		{
			if ( (-- iSubMeshIndex) == 0 )
			{
				break ;
			}
		}
		if ( iSubMeshIndex >= 1 )
		{
			nDrawIndexCount = pre->nSubIndexCount[iSubMeshIndex - 1] ;
			ofsElement = pei->ofsSubElements[iSubMeshIndex - 1] ;
		}
	}
	else
	{
		iSubMeshIndex = 0 ;
	}
	return	(int) iSubMeshIndex ;
}

// 頂点バッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::BindVertexBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc,
		const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
		const S3DRenderBuffer::RENDER_ENTRY * pre,
		ssize_t iBaseMesh, ssize_t iMorphMesh, size_t countBone )
{
	ESLAssert( pei != nullptr ) ;
	ESLAssert( pre != nullptr ) ;
	const size_t	countVertex = pre->countVertex ;
	size_t			ofsVertex = pei->ofsVertex ;
	//
	size_t	ofsVertVertex = ofsVertex ;
	size_t	ofsAttrVertex, ofsAttrNormal = 0, ofsAttrTxtCoord = 0 ;
	size_t	ofsAttrMulColor = 0, ofsAttrAddColor = 0, ofsAttrAlpha = 0 ;
	size_t	ofsAttrMappingX = 0, ofsAttrMappingY = 0 ;
	//
	if ( iBaseMesh < 0 )
	{
		if ( pRsrc->IsInterleaveVertexElement( pre ) )
		{
			const size_t	nElementSize = sizeof(SGLOpenGLVertexBuffer::VertexElementVNTC) ;
			EnableVertexAttribArray( a_vVertexPosition ) ;
			EnableVertexAttribArray( a_vVertexNormal ) ;
			EnableVertexAttribArray( a_vTextureCoord ) ;
			EnableVertexAttribArray( a_vVertexMulColor ) ;
			EnableVertexAttribArray( a_vVertexAddColor ) ;
			VertexAttribPointer
				( a_vVertexPosition, 3, GL_FLOAT,
					GL_FALSE, nElementSize,
					(GLvoid*) (ofsVertex + offsetof(SGLOpenGLVertexBuffer::VertexElementVNTC,vertex)) ) ;
			VertexAttribPointer
				( a_vVertexNormal, 3, GL_FLOAT,
					GL_FALSE, nElementSize,
					(GLvoid*) (ofsVertex + offsetof(SGLOpenGLVertexBuffer::VertexElementVNTC,normal)) ) ;
			VertexAttribPointer
				( a_vTextureCoord, 2, GL_FLOAT,
					GL_FALSE, nElementSize,
					(GLvoid*) (ofsVertex + offsetof(SGLOpenGLVertexBuffer::VertexElementVNTC,uv)) ) ;
			VertexAttribPointer
				( a_vVertexMulColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, nElementSize,
					(GLvoid*) (ofsVertex + offsetof(SGLOpenGLVertexBuffer::VertexElementVNTC,color.rgbMul)) ) ;
			VertexAttribPointer
				( a_vVertexAddColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, nElementSize,
					(GLvoid*) (ofsVertex + offsetof(SGLOpenGLVertexBuffer::VertexElementVNTC,color.rgbAdd)) ) ;
			ofsAttrVertex = ofsVertex ;
			ofsVertex += countVertex * nElementSize ;
		}
		else
		{
			// 頂点
			EnableVertexAttribArray( a_vVertexPosition ) ;
			VertexAttribPointer
				( a_vVertexPosition, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
			ofsAttrVertex = ofsVertex ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
			//
			if ( pre->pvNormal != nullptr )
			{
				// 法線
				EnableVertexAttribArray( a_vVertexNormal ) ;
				VertexAttribPointer
					( a_vVertexNormal, 3, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
				ofsAttrNormal = ofsVertex ;
				ofsVertex += countVertex * sizeof(S3DVector4) ;
			}
			if ( pre->pvUVMap != nullptr )
			{
				// UV 座標
				EnableVertexAttribArray( a_vTextureCoord ) ;
				VertexAttribPointer
					( a_vTextureCoord, 2, GL_FLOAT,
						GL_FALSE, sizeof(S2DVector), (GLvoid*) ofsVertex ) ;
				ofsAttrTxtCoord = ofsVertex ;
				ofsVertex += countVertex * sizeof(S2DVector) ;
			}
			else
			{
				// UV 座標（ダミー）
				EnableVertexAttribArray( a_vTextureCoord ) ;
				VertexAttribPointer
					( a_vTextureCoord, 2, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertVertex ) ;
				ofsAttrTxtCoord = ofsVertVertex ;
			}
			if ( pre->pColor != nullptr )
			{
				// 頂点色
				ofsAttrMulColor = ofsVertex + offsetof(S3DColor,rgbMul) ;
				ofsAttrAddColor = ofsVertex + offsetof(S3DColor,rgbAdd) ;
				ofsAttrAlpha = ofsVertex + offsetof(S3DColor,rgbMul.argb.Alpha) ;
				//
				EnableVertexAttribArray( a_vVertexMulColor ) ;
				VertexAttribPointer
					( a_vVertexMulColor, 4, GL_UNSIGNED_BYTE,
						GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrMulColor ) ;
				EnableVertexAttribArray( a_vVertexAddColor ) ;
				VertexAttribPointer
					( a_vVertexAddColor, 4, GL_UNSIGNED_BYTE,
						GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrAddColor ) ;
				ofsVertex += countVertex * sizeof(S3DColor) ;
			}
		}
		if ( pre->pvTexAxisX != nullptr )
		{
			// テクスチャｘ基底
			EnableVertexAttribArray( a_vVertexMappingX ) ;
			VertexAttribPointer
				( a_vVertexMappingX, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
			ofsAttrMappingX = ofsVertex ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
		if ( pre->pvTexAxisY != nullptr )
		{
			// テクスチャｙ基底
			EnableVertexAttribArray( a_vVertexMappingY ) ;
			VertexAttribPointer
				( a_vVertexMappingY, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
			ofsAttrMappingY = ofsVertex ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
	}
	else
	{
		// モーフィングターゲット頂点
		ofsVertex = pei->ofsMorph
			+ iBaseMesh * countVertex
				* SGLOpenGLVertexBuffer::SIZEOF_ELEMENT ;
		ofsAttrVertex = ofsVertex ;
		//
		EnableVertexAttribArray( a_vVertexPosition ) ;
		VertexAttribPointer
			( a_vVertexPosition, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrVertex ) ;
		ofsVertex += countVertex * sizeof(S3DVector4) ;
		//
		ofsAttrNormal = ofsVertex ;
		EnableVertexAttribArray( a_vVertexNormal ) ;
		VertexAttribPointer
			( a_vVertexNormal, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrNormal ) ;
		ofsVertex += countVertex * sizeof(S3DVector4) ;
		//
		ofsAttrTxtCoord = ofsVertex ;
		EnableVertexAttribArray( a_vTextureCoord ) ;
		VertexAttribPointer
			( a_vTextureCoord, 2, GL_FLOAT,
				GL_FALSE, sizeof(S2DVector), (GLvoid*) ofsAttrTxtCoord ) ;
		ofsVertex += countVertex * sizeof(S2DVector) ;
		//
		ofsAttrMulColor = ofsVertex + offsetof(S3DColor,rgbMul) ;
		ofsAttrAddColor = ofsVertex + offsetof(S3DColor,rgbAdd) ;
		ofsAttrAlpha = ofsVertex + offsetof(S3DColor,rgbMul.argb.Alpha) ;
		//
		EnableVertexAttribArray( a_vVertexMulColor ) ;
		VertexAttribPointer
			( a_vVertexMulColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrMulColor ) ;
		EnableVertexAttribArray( a_vVertexAddColor ) ;
		VertexAttribPointer
			( a_vVertexAddColor, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrAddColor ) ;
		ofsVertex += countVertex * sizeof(S3DColor) ;
		//
		if ( pre->pvTexAxisX != nullptr )
		{
			ofsAttrMappingX = ofsVertex ;
			EnableVertexAttribArray( a_vVertexMappingX ) ;
			VertexAttribPointer
				( a_vVertexMappingX, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrMappingX ) ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
		if ( pre->pvTexAxisY != nullptr )
		{
			ofsAttrMappingY = ofsVertex ;
			EnableVertexAttribArray( a_vVertexMappingY ) ;
			VertexAttribPointer
				( a_vVertexMappingY, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrMappingY ) ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
		}
	}
	m_enabledVertexAttr = true ;
	//
	if ( m_enabledMorphing )
	{
		if ( (iMorphMesh >= 0) && (iMorphMesh != iBaseMesh) )
		{
			// モーフィングターゲット設定
			ofsVertex = pei->ofsMorph
				+ iMorphMesh * countVertex
					* SGLOpenGLVertexBuffer::SIZEOF_ELEMENT ;
			//
			EnableVertexAttribArray( a_vMorphPosition ) ;
			VertexAttribPointer
				( a_vMorphPosition, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
			//
			EnableVertexAttribArray( a_vMorphNormal ) ;
			VertexAttribPointer
				( a_vMorphNormal, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
			ofsVertex += countVertex * sizeof(S3DVector4) ;
			//
			EnableVertexAttribArray( a_vMorphTextureCoord ) ;
			VertexAttribPointer
				( a_vMorphTextureCoord, 2, GL_FLOAT,
					GL_FALSE, sizeof(S2DVector), (GLvoid*) ofsVertex ) ;
			ofsVertex += countVertex * sizeof(S2DVector) ;
			//
			EnableVertexAttribArray( a_vMorphMulColor ) ;
			VertexAttribPointer
				( a_vMorphMulColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor),
					(GLvoid*) (ofsVertex + offsetof(S3DColor,rgbMul)) ) ;
			EnableVertexAttribArray( a_vMorphAddColor ) ;
			VertexAttribPointer
				( a_vMorphAddColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor),
					(GLvoid*) (ofsVertex + offsetof(S3DColor,rgbAdd)) ) ;
			ofsVertex += countVertex * sizeof(S3DColor) ;
			//
			if ( pre->pvTexAxisX != nullptr )
			{
				EnableVertexAttribArray( a_vMorphMappingX ) ;
				VertexAttribPointer
					( a_vMorphMappingX, 3, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
				ofsVertex += countVertex * sizeof(S3DVector4) ;
			}
			if ( pre->pvTexAxisY != nullptr )
			{
				EnableVertexAttribArray( a_vMorphMappingY ) ;
				VertexAttribPointer
					( a_vMorphMappingY, 3, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsVertex ) ;
				ofsVertex += countVertex * sizeof(S3DVector4) ;
			}
		}
		else
		{
			// モーフィング無し
			EnableVertexAttribArray( a_vMorphPosition ) ;
			EnableVertexAttribArray( a_vMorphNormal ) ;
			EnableVertexAttribArray( a_vMorphTextureCoord ) ;
			EnableVertexAttribArray( a_vMorphMulColor ) ;
			EnableVertexAttribArray( a_vMorphAddColor ) ;
			VertexAttribPointer
				( a_vMorphPosition, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrVertex ) ;
			VertexAttribPointer
				( a_vMorphNormal, 3, GL_FLOAT,
					GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrNormal ) ;
			VertexAttribPointer
				( a_vMorphTextureCoord, 2, GL_FLOAT,
					GL_FALSE, sizeof(S2DVector), (GLvoid*) ofsAttrTxtCoord ) ;
			VertexAttribPointer
				( a_vMorphMulColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrMulColor ) ;
			VertexAttribPointer
				( a_vMorphAddColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor), (GLvoid*) ofsAttrAddColor ) ;
			if ( ofsAttrMappingX != 0 )
			{
				EnableVertexAttribArray( a_vMorphMappingX ) ;
				VertexAttribPointer
					( a_vMorphMappingX, 3, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrMappingX ) ;
			}
			if ( ofsAttrMappingY != 0 )
			{
				EnableVertexAttribArray( a_vMorphMappingY ) ;
				VertexAttribPointer
					( a_vMorphMappingY, 3, GL_FLOAT,
						GL_FALSE, sizeof(S3DVector4), (GLvoid*) ofsAttrMappingY ) ;
			}
		}
		m_enabledVertex4Morphing = true ;
	}
	//
	if ( (countBone > 0) && (countBone <= m_maxBonePalette) )
	{
		// ボーン・ウェイトマップの設定
		size_t	ofsBoneWeight = pei->ofsBone ;
		//
		size_t	nBoneCount = pre->countBone ;
		size_t	iSrcBone = 0 ;
		size_t	iDstBone = 0 ;
		if ( pre->countFullBone >= 2 )
		{
			iSrcBone = (pre->countFullBone - 1) & ~0x03 ;
			ofsBoneWeight += iSrcBone * countVertex * sizeof(float32_t) ;
		}
		while ( iSrcBone < nBoneCount )
		{
			ESLAssert( iDstBone < BONE_PALETTE_ATTRS ) ;
			EnableVertexAttribArray( a_vVertexBoneWeight[iDstBone] ) ;
			m_enabledVertexBoneWeight[iDstBone] = true ;
			//
			VertexAttribPointer
				( a_vVertexBoneWeight[iDstBone], 4, GL_FLOAT,
					GL_FALSE, sizeof(float32_t) * 4, (GLvoid*) ofsBoneWeight ) ;
			ofsBoneWeight += countVertex * sizeof(float32_t) * 4 ;
			//
			iSrcBone += 4 ;
			iDstBone ++ ;
		}
	}
}

// 頂点バッファ有効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::EnableVertexBuffer
	( const SGLOpenGLVertexBuffer::ENTRY_INFO * pei,
		const S3DRenderBuffer::RENDER_ENTRY * pre,
		ssize_t iBaseMesh, ssize_t iMorphMesh, size_t countBone )
{
	EnableVertexAttribArray( a_vVertexPosition ) ;
	EnableVertexAttribArray( a_vVertexNormal ) ;
	EnableVertexAttribArray( a_vTextureCoord ) ;
	EnableVertexAttribArray( a_vVertexMulColor ) ;
	EnableVertexAttribArray( a_vVertexAddColor ) ;
	if ( pre->pvTexAxisX != nullptr )
	{
		EnableVertexAttribArray( a_vVertexMappingX ) ;
	}
	if ( pre->pvTexAxisY != nullptr )
	{
		EnableVertexAttribArray( a_vVertexMappingY ) ;
	}
	m_enabledVertexAttr = true ;
	//
	if ( m_enabledMorphing )
	{
		EnableVertexAttribArray( a_vMorphPosition ) ;
		EnableVertexAttribArray( a_vMorphNormal ) ;
		EnableVertexAttribArray( a_vMorphTextureCoord ) ;
		EnableVertexAttribArray( a_vMorphMulColor ) ;
		EnableVertexAttribArray( a_vMorphAddColor ) ;
		if ( pre->pvTexAxisX != nullptr )
		{
			EnableVertexAttribArray( a_vMorphMappingX ) ;
		}
		if ( pre->pvTexAxisY != nullptr )
		{
			EnableVertexAttribArray( a_vMorphMappingY ) ;
		}
		m_enabledVertex4Morphing = true ;
	}
	//
	if ( (countBone > 0) && (countBone <= m_maxBonePalette) )
	{
		size_t	nBoneCount = pre->countBone ;
		size_t	iSrcBone = 0 ;
		size_t	iDstBone = 0 ;
		if ( pre->countFullBone >= 2 )
		{
			iSrcBone = (pre->countFullBone - 1) & ~0x03 ;
		}
		while ( iSrcBone < nBoneCount )
		{
			ESLAssert( iDstBone < BONE_PALETTE_ATTRS ) ;
			EnableVertexAttribArray( a_vVertexBoneWeight[iDstBone] ) ;
			m_enabledVertexBoneWeight[iDstBone] = true ;
			//
			iSrcBone += 4 ;
			iDstBone ++ ;
		}
	}
}

// インスタンシングバッファ書き込み
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::WriteInstancingBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc, size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	pRsrc->AllocateInstancingBuffer
			( (size_t) esl_max( (int) nInstancing, 1 ) ) ;
	//
	if ( nInstancing == 0 )
	{
		S4DMatrix	matI( 1, 1, 1, 1 ) ;
		S3DColor	clrT( 0xFFFFFFFF, 0 ) ;
		pRsrc->WriteInstancingBuffer( &matI, &clrT, 1 ) ;
	}
	else
	{
		pRsrc->WriteInstancingBuffer
			( pmatInstancing, pColorInstancing, nInstancing ) ;
	}
}

// インスタンシングバッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::BindInstancingBuffer
	( SGLOpenGLVertexBuffer::GLResource * pRsrc )
{
	ESLAssert( m_enabledInstancing ) ;
	pRsrc->BindInstancingBuffer() ;
	//
	const GLsizei	stride = sizeof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry) ;
	//
	EnableVertexAttribArray( a_matInstancingModelView[0] ) ;
	VertexAttribPointer
		( a_matInstancingModelView[0], 4, GL_FLOAT, GL_FALSE, stride,
			(GLvoid*) offsetof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry,matrix[0][0]), 1 ) ;
	//
	EnableVertexAttribArray( a_matInstancingModelView[1] ) ;
	VertexAttribPointer
		( a_matInstancingModelView[1], 4, GL_FLOAT, GL_FALSE, stride,
			(GLvoid*) offsetof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry,matrix[1][0]), 1 ) ;
	//
	EnableVertexAttribArray( a_matInstancingModelView[2] ) ;
	VertexAttribPointer
		( a_matInstancingModelView[2], 4, GL_FLOAT, GL_FALSE, stride,
			(GLvoid*) offsetof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry,matrix[2][0]), 1 ) ;
	//
	EnableVertexAttribArray( a_vInstancingMulColor ) ;
	VertexAttribPointer
		( a_vInstancingMulColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
			(GLvoid*) offsetof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry,color.rgbMul), 1 ) ;
	//
	EnableVertexAttribArray( a_vInstancingAddColor ) ;
	VertexAttribPointer
		( a_vInstancingAddColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
			(GLvoid*) offsetof(SGLOpenGLVertexBuffer::GLResource::InstanceEntry,color.rgbAdd), 1 ) ;
	//
	m_enabledInstancingAttr = true ;
}

// インスタンシングバッファ有効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::EnableInstancingBuffer( void )
{
	EnableVertexAttribArray( a_matInstancingModelView[0] ) ;
	EnableVertexAttribArray( a_matInstancingModelView[1] ) ;
	EnableVertexAttribArray( a_matInstancingModelView[2] ) ;
	EnableVertexAttribArray( a_vInstancingMulColor ) ;
	EnableVertexAttribArray( a_vInstancingAddColor ) ;
	//
	m_enabledInstancingAttr = true ;
}

// ボーン回転行列 Uniform 設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetBoneMatrixUniform
	( const S3DRenderBuffer::RENDER_ENTRY * pre )
{
	GLfloat	fpBoneMatrix[MAX_BONE_PALETTE][3][3] ;
	GLfloat	fpBoneTrans[MAX_BONE_PALETTE][3] ;
	//
	size_t	nBoneCount = pre->countBone ;
	size_t	iSrcBone = 0 ;
	size_t	iDstBone = 0 ;
	if ( pre->countFullBone >= 2 )
	{
		iSrcBone = (pre->countFullBone - 1) & ~0x03 ;
	}
	while ( iSrcBone < nBoneCount )
	{
		S3DMatrix&	mat = pre->pBoneMatrix[iSrcBone] ;
		S3DVector&	pos = pre->pBoneTrans[iSrcBone] ;
		for ( size_t j = 0; j < 3; j ++ )
		{
			fpBoneMatrix[iDstBone][j][0] = mat.m[0][j] ;
			fpBoneMatrix[iDstBone][j][1] = mat.m[1][j] ;
			fpBoneMatrix[iDstBone][j][2] = mat.m[2][j] ;
		}
		fpBoneTrans[iDstBone][0] = pos.x ;
		fpBoneTrans[iDstBone][1] = pos.y ;
		fpBoneTrans[iDstBone][2] = pos.z ;
		//
		iSrcBone ++ ;
		iDstBone ++ ;
	}
	glUniform1i( u_nBoneCount, (GLint) iDstBone ) ;
	SGLOpenGLContext::VerifyError( "glUniform1i(u_nBoneCount)" ) ;
	m_nBoneCount = (GLint) iDstBone ;
	//
	// ボーン回転行列の設定
	glUniformMatrix3fv
		( u_mat3BoneRotation,
			(GLsizei) iDstBone,
			GL_FALSE, &fpBoneMatrix[0][0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniformMatrix4fv(u_mat3BoneRotation)" ) ;
	//
	glUniform3fv
		( u_vBoneTranslate,
			(GLsizei) iDstBone, &fpBoneTrans[0][0] ) ;
	SGLOpenGLContext::VerifyError( "glUniform3fv(u_vBoneTranslate)" ) ;
}

// 描画コマンド
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::DrawVertexBuffer
	( VertexSetupContext& vsc,
		SGLOpenGLContext * pOpenGL,
		GLenum modeDraw, size_t nIndexCount, size_t ofsElement )
{
	SGLOpenGLVertexBuffer::GLResource *			pRsrc = vsc.pRsrc ;
	const SGLOpenGLVertexBuffer::ENTRY_INFO *	pei = vsc.pei ;
	S3DRenderBuffer::RENDER_ENTRY *				pre = vsc.pre ;
	const size_t	countVertex = pre->countVertex ;
	//
#if	!defined(__PLATFORM_ANDROID__) || (ANDROID_API_LEVEL >= 18)
	if ( m_enabledVTInstancing
		&& pei->flagVertexTexture && vsc.flagMultiShapeInstance )
	{
		//
		// Multi shaped instanced Draw
		//
		ESLAssert( vsc.nInstancing != 0 ) ;
		ESLAssert( vsc.ppInstancingVVB != nullptr ) ;
		//
		size_t				nInstancing = vsc.nInstancing ;
		const S4DMatrix *	pmatInstancing = vsc.pmatInstancing ;
		const S3DColor *	pColorInstancing = vsc.pColorInstancing ;
		S3DVertexVariantBuffer*const*
							ppInstancingVVB = vsc.ppInstancingVVB ;
		//
		while ( nInstancing != 0 )
		{
			size_t	nPrimCount = nInstancing ;
			if ( nPrimCount > pei->nMaxInstance )
			{
				nPrimCount = pei->nMaxInstance ;
			}
			WriteInstancingBuffer
				( pRsrc, nPrimCount, pmatInstancing, pColorInstancing ) ;
			WriteMorphInstanceToVertexTexture
				( vsc.pVTImage, vsc.iMesh, *pei, ppInstancingVVB, nPrimCount ) ;
			WriteBoneInstanceToVertexTexture
				( vsc.pVTImage, vsc.iMesh, *pei, ppInstancingVVB, nPrimCount ) ;
			//
			if ( pre->pIndexedList != nullptr )
			{
				glDrawElementsInstanced
					( modeDraw,
						(GLsizei) nIndexCount,
						pOpenGL->m_typeVBOIndex,
						(GLvoid*) ofsElement, (GLsizei) nPrimCount ) ;
				SGLOpenGLContext::VerifyError( "glDrawElementsInstanced()" ) ;
			}
			else
			{
				glDrawArraysInstanced
					( modeDraw, 0,
						(GLsizei) countVertex, (GLsizei) nPrimCount ) ;
				SGLOpenGLContext::VerifyError( "glDrawArraysInstanced()" ) ;
			}
			pOpenGL->m_pflog.countDrawCall ++ ;
			pOpenGL->m_pflog.countDrawInstance += nPrimCount ;
			pOpenGL->m_pflog.countDrawVertex += countVertex * nPrimCount ;
			//
			pmatInstancing += nPrimCount ;
			pColorInstancing += nPrimCount ;
			ppInstancingVVB += nPrimCount ;
			nInstancing -= nPrimCount ;
		}
	}
	else if ( m_enabledInstancing )
	{
		//
		// Instanced Draw
		//
		GLsizei	nPrimCount = (vsc.nInstancing == 0) ?
								1 : (GLsizei) vsc.nInstancing ;
		if ( pre->pIndexedList != nullptr )
		{
			glDrawElementsInstanced
				( modeDraw,
					(GLsizei) nIndexCount,
					pOpenGL->m_typeVBOIndex,
					(GLvoid*) ofsElement, nPrimCount ) ;
			SGLOpenGLContext::VerifyError( "glDrawElementsInstanced()" ) ;
		}
		else
		{
			glDrawArraysInstanced
				( modeDraw, 0, (GLsizei) countVertex, nPrimCount ) ;
			SGLOpenGLContext::VerifyError( "glDrawArraysInstanced()" ) ;
		}
		pOpenGL->m_pflog.countDrawCall ++ ;
		pOpenGL->m_pflog.countDrawInstance += nPrimCount ;
		pOpenGL->m_pflog.countDrawVertex += countVertex * nPrimCount ;
	}
	else
#endif
	if ( vsc.nInstancing > 0 )
	{
		S4DMatrix	mat4SaveModelView = m_mat4ModelView ;
		bool		fSaveInvNormal = (m_fpInverseNormal < 0.0f) ;
		S3DColor	clrSaveColor = m_colorUniformEffect ;
		uint32_t	nSaveTransparency = m_nUniformTransparency ;
		//
		// 連続描画
		for ( size_t i = 0; i < vsc.nInstancing; i ++ )
		{
			S4DMatrix	mat4Instance =
							mat4SaveModelView * vsc.pmatInstancing[i] ;
			S3DColor	clrInstance =
							clrSaveColor * vsc.pColorInstancing[i] ;
			uint32_t	nTransparency =
				0x100 - ((0x100 - nSaveTransparency)
					* (vsc.pColorInstancing[i].rgbMul.argb.Alpha + 1)) / 0x100 ;
			//
			SetModelViewMatrix( mat4Instance, fSaveInvNormal ) ;
			SetColorEffect( &clrInstance, nTransparency ) ;
			//
			if ( pre->pIndexedList != nullptr )
			{
				glDrawElements
					( modeDraw,
						(GLsizei) nIndexCount,
						pOpenGL->m_typeVBOIndex, (GLvoid*) ofsElement ) ;
				SGLOpenGLContext::VerifyError( "glDrawElements()" ) ;
			}
			else
			{
				glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
				SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
			}
		}
		//
		SetModelViewMatrix( mat4SaveModelView, fSaveInvNormal ) ;
		SetColorEffect( &clrSaveColor, nSaveTransparency ) ;
		//
		pOpenGL->m_pflog.countDrawCall += vsc.nInstancing ;
		pOpenGL->m_pflog.countDrawInstance += vsc.nInstancing ;
		pOpenGL->m_pflog.countDrawVertex += countVertex * vsc.nInstancing ;
	}
	else
	{
		//
		// 通常の描画
		//
		if ( pre->pIndexedList != nullptr )
		{
			glDrawElements
				( modeDraw,
					(GLsizei) nIndexCount,
					pOpenGL->m_typeVBOIndex, (GLvoid*) ofsElement ) ;
			SGLOpenGLContext::VerifyError( "glDrawElements()" ) ;
		}
		else
		{
			glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
			SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
		}
		pOpenGL->m_pflog.countDrawCall ++ ;
		pOpenGL->m_pflog.countDrawInstance ++ ;
		pOpenGL->m_pflog.countDrawVertex += countVertex ;
	}
}

// 頂点バッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetVertexPointer( const S3DVector4 * pvVertex )
{
	if ( m_pOpenGL->m_pBindingVBO != nullptr )
	{
		DisableAllVertexPointer() ;
		//
		glBindBuffer( GL_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ARRAY_BUFFER,0)" ) ;
		//
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) ;
		SGLOpenGLContext::VerifyError( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0)" ) ;
		//
		m_pOpenGL->m_pBindingVBO = nullptr ;
		m_pOpenGL->m_pRsrcOfVBO = nullptr ;
	}
	if ( pvVertex != nullptr )
	{
		EnableVertexAttribArray( a_vVertexPosition ) ;
		VertexAttribPointer
			( a_vVertexPosition, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), pvVertex ) ;
	}
	else
	{
		DisableVertexAttribArray( a_vVertexPosition ) ;
	}
}

// 法線バッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetNormalPointer( const S3DVector4 * pvNormal )
{
	if ( pvNormal != nullptr )
	{
		EnableVertexAttribArray( a_vVertexNormal ) ;
		VertexAttribPointer
			( a_vVertexNormal, 3, GL_FLOAT,
				GL_FALSE, sizeof(S3DVector4), pvNormal ) ;
	}
	else
	{
		DisableVertexAttribArray( a_vVertexNormal ) ;
	}
}

// UV 座標バッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetTexCoordPointer( const S2DVector * pvUVMap )
{
	if ( pvUVMap != nullptr )
	{
		EnableVertexAttribArray( a_vTextureCoord ) ;
		VertexAttribPointer
			( a_vTextureCoord, 2, GL_FLOAT,
					GL_FALSE, sizeof(S2DVector), pvUVMap ) ;
	}
	else
	{
		DisableVertexAttribArray( a_vTextureCoord ) ;
	}
}

// 頂点色バッファ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::SetColorPointer( const S3DColor * pColor )
{
	if ( pColor != nullptr )
	{
		EnableVertexAttribArray( a_vVertexMulColor ) ;
		VertexAttribPointer
			( a_vVertexMulColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor), &(pColor->rgbMul) ) ;
		EnableVertexAttribArray( a_vVertexAddColor ) ;
		VertexAttribPointer
			( a_vVertexAddColor, 4, GL_UNSIGNED_BYTE,
					GL_TRUE, sizeof(S3DColor), &(pColor->rgbAdd) ) ;
	}
	else
	{
		DisableVertexAttribArray( a_vVertexMulColor ) ;
		DisableVertexAttribArray( a_vVertexAddColor ) ;
	}
}

// 全頂点ポインタを無効化
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLDefaultShader::DisableAllVertexPointer( void )
{
	if ( m_enabledVertexAttr )
	{
		DisableVertexAttribArray( a_vVertexPosition ) ;
		DisableVertexAttribArray( a_vVertexNormal ) ;
		DisableVertexAttribArray( a_vVertexMappingX ) ;
		DisableVertexAttribArray( a_vVertexMappingY ) ;
		DisableVertexAttribArray( a_vTextureCoord ) ;
		DisableVertexAttribArray( a_vVertexMulColor ) ;
		DisableVertexAttribArray( a_vVertexAddColor ) ;
		m_enabledVertexAttr = false ;
	}
	//
	if ( m_enabledVertex4Morphing )
	{
		DisableVertexAttribArray( a_vMorphPosition ) ;
		DisableVertexAttribArray( a_vMorphNormal ) ;
		DisableVertexAttribArray( a_vMorphMappingX ) ;
		DisableVertexAttribArray( a_vMorphMappingY ) ;
		DisableVertexAttribArray( a_vMorphTextureCoord ) ;
		DisableVertexAttribArray( a_vMorphMulColor ) ;
		DisableVertexAttribArray( a_vMorphAddColor ) ;
		m_enabledVertex4Morphing = false ;
	}
	//
	for ( int i = 0; i < BONE_PALETTE_ATTRS; i ++ )
	{
		if ( m_enabledVertexBoneWeight[i] )
		{
			DisableVertexAttribArray( a_vVertexBoneWeight[i] ) ;
			m_enabledVertexBoneWeight[i] = false ;
		}
	}
	//
	if ( m_enabledInstancingAttr )
	{
		DisableVertexAttribArray( a_matInstancingModelView[0] ) ;
		DisableVertexAttribArray( a_matInstancingModelView[1] ) ;
		DisableVertexAttribArray( a_matInstancingModelView[2] ) ;
		DisableVertexAttribArray( a_vInstancingMulColor ) ;
		DisableVertexAttribArray( a_vInstancingAddColor ) ;
		m_enabledInstancingAttr = false ;
	}
}

// 頂点色用ダミーバッファを確保
//////////////////////////////////////////////////////////////////////////////
S3DColor * SGLOpenGLDefaultShader::AllocateDummyVertexColorBuffer( size_t countVertex )
{
	size_t	nLen = m_bufColors.GetLength() ;
	if ( nLen < countVertex )
	{
		m_bufColors.SetLength( countVertex ) ;
		//
		S3DColor *	pColor = m_bufColors.GetArray() ;
		for ( size_t i = nLen; i < countVertex; i ++ )
		{
			pColor[i].rgbMul.ui32 = 0xFFFFFFFF ;
			pColor[i].rgbAdd.ui32 = 0x00000000 ;
		}
		m_bufColors.FinishArray() ;
	}
	return	m_bufColors.GetArrayPtr() ;
}

// 指標バッファを16ビットへ変換
//////////////////////////////////////////////////////////////////////////////
uint16_t * SGLOpenGLDefaultShader::ElementIndexToUint16
	( const uint32_t * pIndexedList, size_t countIndexes )
{
	if ( m_bufIndex.GetLength() < countIndexes )
	{
		m_bufIndex.FreeArray() ;
		m_bufIndex.SetLength( (countIndexes + 0xFF) & ~0xFF ) ;
	}
	uint16_t *	pwIndex = m_bufIndex.GetArray() ;
	for ( size_t i = 0; i < countIndexes; i ++ )
	{
		pwIndex[i] = (uint16_t) pIndexedList[i] ;
	}
	m_bufIndex.FinishArray() ;
	return	pwIndex ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL レンダリング・コンテキスト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLRenderingContext::SGLOpenGLRenderingContext( void )
{
	m_pLastMaterial = nullptr ;
	m_flagBackFace = false ;
	m_flagNeedsMulAlpha = false ;
	m_flagWithoutAlpha = false ;
	m_pglLastTexture = nullptr ;
	//
	m_flagLightingGL = false ;
	m_flagTextureSmoothing = true ;
	//
	m_flagMulColor = false ;
	m_flagAddColor = false ;
	m_colorMulEffect[0] = 1.0f ;
	m_colorMulEffect[1] = 1.0f ;
	m_colorMulEffect[2] = 1.0f ;
	m_colorAddEffect[0] = 0.0f ;
	m_colorAddEffect[1] = 0.0f ;
	m_colorAddEffect[2] = 0.0f ;
	m_alphaEnvEffect = 1.0f ;
	m_alphaEffect = 1.0f ;
}

// 機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::EnableLightingByGL( bool flagLighting )
{
	m_flagLightingGL = flagLighting ;
}

void SGLOpenGLRenderingContext::EnableTextureSmoothing( bool flagSmoothing )
{
	m_flagTextureSmoothing = flagSmoothing ;
}

// 効果を設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::SetColorEffect
	( const S3DColor * colorEffect, unsigned int nTransparency )
{
	m_colorMulEffect[0] = 1.0f ;
	m_colorMulEffect[1] = 1.0f ;
	m_colorMulEffect[2] = 1.0f ;
	m_colorAddEffect[0] = 0.0f ;
	m_colorAddEffect[1] = 0.0f ;
	m_colorAddEffect[2] = 0.0f ;
	//
	if ( colorEffect != nullptr )
	{
		m_flagMulColor = ((colorEffect->rgbMul.ui32 & 0xFFFFFF) != 0xFFFFFF) ;
		if ( m_flagMulColor )
		{
			m_colorMulEffect[0] =
				(GLfloat) colorEffect->rgbMul.argb.Red / 255.0f ;
			m_colorMulEffect[1] =
				(GLfloat) colorEffect->rgbMul.argb.Green / 255.0f ;
			m_colorMulEffect[2] =
				(GLfloat) colorEffect->rgbMul.argb.Blue / 255.0f ;
		}
		m_flagAddColor = ((colorEffect->rgbAdd.ui32 & 0xFFFFFF) != 0xFFFFFF) ;
		if ( m_flagAddColor )
		{
			m_colorAddEffect[0] =
				(GLfloat) colorEffect->rgbAdd.argb.Red / 255.0f ;
			m_colorAddEffect[1] =
				(GLfloat) colorEffect->rgbAdd.argb.Green / 255.0f ;
			m_colorAddEffect[2] =
				(GLfloat) colorEffect->rgbAdd.argb.Blue / 255.0f ;
		}
	}
	else
	{
		m_flagMulColor = false ;
		m_flagAddColor = false ;
	}
	/*
	if ( nTransparency < 0 )
	{
		m_alphaEnvEffect = 1.0f ;
	}
	else
	*/
	if ( nTransparency > 0x100 )
	{
		m_alphaEnvEffect = 0.0f ;
	}
	else
	{
		m_alphaEnvEffect = (GLfloat) (0x100 - nTransparency) / 0x100 ;
	}
	m_alphaEffect = m_alphaEnvEffect ;
}

// 光源を設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	if ( !m_flagLightingGL )
	{
		glDisable( GL_LIGHTING ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_LIGHTING)" ) ;
		return ;
	}
	//
	// OpenGL に各種光源を設定する
	//
	SGLPalette	rgbAmbient ;
	rgbAmbient.ui32 = 0 ;
	//
	const int	glLightCount = 8 ;
	int			iLight = 0 ;
	GLfloat		params[4] ;
	GLfloat		scaleBy255 = 1.0f / 255.0f ;
	//
	size_t	i ;
	for ( i = 0; i < countLight; i ++ )
	{
		const S3DLightEntry &	light = pLights[i] ;
		switch ( light.typeLight & lightTypeMask )
		{
		case	lightTypeVector:
		case	lightTypePoint:
		case	lightTypeSpot:
			glEnable( GL_LIGHT0 + iLight ) ;
			//
			params[0] = (GLfloat) (light.rgbColor.argb.Red * scaleBy255) ;
			params[1] = (GLfloat) (light.rgbColor.argb.Green * scaleBy255) ;
			params[2] = (GLfloat) (light.rgbColor.argb.Blue * scaleBy255) ;
			params[3] = 1.0f ;
			glLightfv( GL_LIGHT0 + iLight, GL_AMBIENT, params ) ;
			SGLOpenGLContext::VerifyError( "glLightfv(GL_AMBIENT)" ) ;
			glLightfv( GL_LIGHT0 + iLight, GL_DIFFUSE, params ) ;
			SGLOpenGLContext::VerifyError( "glLightfv(GL_DIFFUSE)" ) ;
			glLightfv( GL_LIGHT0 + iLight, GL_SPECULAR, params ) ;
			SGLOpenGLContext::VerifyError( "glLightfv(GL_SPECULAR)" ) ;
			//
			if ( (light.typeLight & lightTypeMask) == lightTypeVector )
			{
				params[0] = - light.vecDirection.x ;
				params[1] = - light.vecDirection.y ;
				params[2] = - light.vecDirection.z ;
				params[3] = 0.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_POSITION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_POSITION)" ) ;
				//
				params[0] = light.fpBrightness ;
				params[1] = light.fpBrightness ;
				params[2] = light.fpBrightness ;
				params[3] = 0.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_CONSTANT_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_CONSTANT_ATTENUATION)" ) ;
				//
				params[0] = 0.0f ;
				params[1] = 0.0f ;
				params[2] = 0.0f ;
				params[3] = 0.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_LINEAR_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_LINEAR_ATTENUATION)" ) ;
				glLightfv( GL_LIGHT0 + iLight, GL_QUADRATIC_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_QUADRATIC_ATTENUATION)" ) ;
			}
			else
			{
				params[0] = light.vecPosition.x ;
				params[1] = light.vecPosition.y ;
				params[2] = light.vecPosition.z ;
				params[3] = 1.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_POSITION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_POSITION)" ) ;
				//
				if ( (light.typeLight & lightTypeMask) == lightTypeSpot )
				{
					params[0] = light.vecDirection.x ;
					params[1] = light.vecDirection.y ;
					params[2] = light.vecDirection.z ;
					params[3] = 1.0f ;
					glLightfv( GL_LIGHT0 + iLight, GL_SPOT_DIRECTION, params ) ;
					SGLOpenGLContext::VerifyError( "glLightfv(GL_SPOT_DIRECTION)" ) ;
					//
					params[0] =
						(GLfloat) (acos( light.fpAngle )
										* (180.0 / SSystem::PI)) ;
					params[1] = 0.0f ;
					params[2] = 0.0f ;
					params[3] = 0.0f ;
					glLightfv( GL_LIGHT0 + iLight, GL_SPOT_CUTOFF, params ) ;
					SGLOpenGLContext::VerifyError( "glLightfv(GL_SPOT_CUTOFF)" ) ;
				}
				else
				{
					params[0] = 180.0f ;
					params[1] = 0.0f ;
					params[2] = 0.0f ;
					params[3] = 0.0f ;
					glLightfv( GL_LIGHT0 + iLight, GL_SPOT_CUTOFF, params ) ;
					SGLOpenGLContext::VerifyError( "glLightfv(GL_SPOT_CUTOFF)" ) ;
				}
				//
				params[0] = light.fpBrightness ;
				params[1] = light.fpBrightness ;
				params[2] = light.fpBrightness ;
				params[3] = 0.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_LINEAR_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_LINEAR_ATTENUATION)" ) ;
				//
				params[0] = 0.0f ;
				params[1] = 0.0f ;
				params[2] = 0.0f ;
				params[3] = 0.0f ;
				glLightfv( GL_LIGHT0 + iLight, GL_CONSTANT_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_LINEAR_ATTENUATION)" ) ;
				glLightfv( GL_LIGHT0 + iLight, GL_QUADRATIC_ATTENUATION, params ) ;
				SGLOpenGLContext::VerifyError( "glLightfv(GL_QUADRATIC_ATTENUATION)" ) ;
			}
			//
			iLight ++ ;
			break ;

		case	lightTypeAmbient:
			rgbAmbient += light.rgbColor ;
			break ;
		}
		if ( iLight >= glLightCount )
		{
			break ;
		}
	}
	if ( (rgbAmbient.ui32 != 0) & (iLight < glLightCount) )
	{
		glEnable( GL_LIGHT0 + iLight ) ;
		//
		params[0] = (GLfloat) (rgbAmbient.argb.Red * scaleBy255) ;
		params[1] = (GLfloat) (rgbAmbient.argb.Green * scaleBy255) ;
		params[2] = (GLfloat) (rgbAmbient.argb.Blue * scaleBy255) ;
		params[3] = 1.0f ;
		glLightfv( GL_LIGHT0 + iLight, GL_AMBIENT, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_AMBIENT)" ) ;
		//
		params[0] = 0.0f ;
		params[1] = 0.0f ;
		params[2] = 0.0f ;
		params[3] = 1.0f ;
		glLightfv( GL_LIGHT0 + iLight, GL_DIFFUSE, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_DIFFUSE)" ) ;
		glLightfv( GL_LIGHT0 + iLight, GL_SPECULAR, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_SPECULAR)" ) ;
		//
		glLightfv( GL_LIGHT0 + iLight, GL_POSITION, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_POSITION)" ) ;
		//
		params[0] = 1.0f ;
		params[1] = 1.0f ;
		params[2] = 1.0f ;
		params[3] = 1.0f ;
		glLightfv( GL_LIGHT0 + iLight, GL_CONSTANT_ATTENUATION, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_CONSTANT_ATTENUATION)" ) ;
		//
		params[0] = 0.0f ;
		params[1] = 0.0f ;
		params[2] = 0.0f ;
		params[3] = 0.0f ;
		glLightfv( GL_LIGHT0 + iLight, GL_LINEAR_ATTENUATION, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_LINEAR_ATTENUATION)" ) ;
		glLightfv( GL_LIGHT0 + iLight, GL_QUADRATIC_ATTENUATION, params ) ;
		SGLOpenGLContext::VerifyError( "glLightfv(GL_QUADRATIC_ATTENUATION)" ) ;
		//
		iLight ++ ;
	}
	//
	// 使用しない光源を無効化
	//
	for ( i = iLight; i < glLightCount; i ++ )
	{
		glDisable( (GLenum) (GL_LIGHT0 + i) ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_LIGHT0+x)" ) ;
	}
	if ( iLight > 0 )
	{
		glEnable( GL_LIGHTING ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_LIGHTING)" ) ;
	}
	else
	{
		glDisable( GL_LIGHTING ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_LIGHTING)" ) ;
	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	if ( m_flagLightingGL )
	{
		#if	defined(__API_OPEN_GL_ES__)
			glFogx( GL_FOG_MODE, GL_LINEAR ) ;
		#else
			glFogi( GL_FOG_MODE, GL_LINEAR ) ;
		#endif
		SGLOpenGLContext::VerifyError( "glFogx(GL_FOG_MODE,GL_LINEAR)" ) ;
		glHint( GL_FOG_HINT, GL_DONT_CARE ) ;
		SGLOpenGLContext::VerifyError( "glHint(GL_FOG_HINT,GL_DONT_CARE)" ) ;
		//
		SGLPalette	rgbAmbient ;
		GLfloat		params[4] ;
		GLfloat		scaleBy255 = 1.0f / 255.0f ;
		rgbAmbient.ui32 = rgbFog ;
		params[0] = (GLfloat) (rgbAmbient.argb.Red * scaleBy255) ;
		params[1] = (GLfloat) (rgbAmbient.argb.Green * scaleBy255) ;
		params[2] = (GLfloat) (rgbAmbient.argb.Blue * scaleBy255) ;
		params[3] = 1.0f ;
		glFogfv( GL_FOG_COLOR, params ) ;
		SGLOpenGLContext::VerifyError( "glFogfv(GL_FOG_COLOR)" ) ;
		glFogf( GL_FOG_DENSITY, 1.0f ) ;
		SGLOpenGLContext::VerifyError( "glFogf(GL_FOG_DENSITY)" ) ;
		glFogf( GL_FOG_START, (GLfloat) zFogNear ) ;
		SGLOpenGLContext::VerifyError( "glFogf(GL_FOG_START)" ) ;
		glFogf( GL_FOG_END, (GLfloat) zFogFar ) ;
		SGLOpenGLContext::VerifyError( "glFogf(GL_FOG_END)" ) ;
	}
}

void SGLOpenGLRenderingContext::EnableFog( bool fFog )
{
	if ( fFog && m_flagLightingGL )
	{
		glEnable( GL_FOG ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_FOG)" ) ;
	}
	else
	{
		glDisable( GL_FOG ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_FOG)" ) ;
	}
}

/*
// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLRenderingContext::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	//
	// 表面属性
	//
	PutGLMaterial( pOpenGL, pMaterial, false ) ;
	//
	// 頂点設定
	//
	PutVertexPointer( pvVertex, pvNormal, pvUVMap, countVertex ) ;
	//
	// 頂点色
	//
	bool	fAddColor = PutVertexColors( pColor, countVertex ) ;
	if ( fAddColor )
	{
		ReviseGLMaterialForBaseColor( pOpenGL ) ;
	}
	//
	// 描画（表面 or 両面）
	//
	size_t	countIndex = countPolygon * 3 ;
#if	defined(__API_OPEN_GL_ES__)
	if ( m_bufIndexed16.GetLength() < countIndex )
	{
		m_bufIndexed16.SetLength( countIndex ) ;
	}
	uint16_t *	pIndexedBuf = m_bufIndexed16.GetArray() ;
	for ( size_t i = 0; i < countIndex; i ++ )
	{
		pIndexedBuf[i] = (uint16_t) pIndexedList[i] ;
	}
	glDrawElements
		( GL_TRIANGLES, countIndex, GL_UNSIGNED_SHORT, pIndexedBuf ) ;
#else
	glDrawElements
		( GL_TRIANGLES, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
#endif
	SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;
	//
	if ( fAddColor )
	{
		PutVertexAddColors() ;
		PutGLMaterialForAddColor( pOpenGL ) ;
		//
		#if	defined(__API_OPEN_GL_ES__)
			glDrawElements
				( GL_TRIANGLES, countIndex,
					GL_UNSIGNED_SHORT, pIndexedBuf ) ;
		#else
			glDrawElements
				( GL_TRIANGLES, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
		#endif
		SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;
		//
		PutGLMaterialAfterAddColor( pOpenGL ) ;
	}
	if ( !PutGLMaterial( pOpenGL, pMaterial, true ) )
	{
		//
		// 裏面（別属性の場合）描画
		//
		if ( fAddColor )
		{
			PutVertexMulColors() ;
			ReviseGLMaterialForBaseColor( pOpenGL ) ;
		}
		#if	defined(__API_OPEN_GL_ES__)
			glDrawElements
				( GL_TRIANGLES, countIndex,
					GL_UNSIGNED_SHORT, pIndexedBuf ) ;
		#else
			glDrawElements
				( GL_TRIANGLES, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
		#endif
		SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;
		//
		if ( fAddColor )
		{
			PutVertexAddColors() ;
			PutGLMaterialForAddColor( pOpenGL ) ;
			//
			#if	defined(__API_OPEN_GL_ES__)
				glDrawElements
					( GL_TRIANGLES, countIndex,
						GL_UNSIGNED_SHORT, pIndexedBuf ) ;
			#else
				glDrawElements
					( GL_TRIANGLES, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
			#endif
			SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;
			//
			PutGLMaterialAfterAddColor( pOpenGL ) ;
		}
	}
	//
	FlushVertexPointers() ;
	//
	return	sglErrSuccess ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLRenderingContext::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	size_t	countVertex = countTriangleStrip + 2 ;
	//
	// 表面属性
	//
	PutGLMaterial( pOpenGL, pMaterial, false ) ;
	//
	// 頂点設定
	//
	PutVertexPointer( pvVertex, pvNormal, pvUVMap, countVertex ) ;
	//
	// 頂点色
	//
	bool	fAddColor = PutVertexColors( pColor, countVertex ) ;
	if ( fAddColor )
	{
		ReviseGLMaterialForBaseColor( pOpenGL ) ;
	}
	//
	// 描画（表面 or 両面）
	//
	glDrawArrays( GL_TRIANGLE_STRIP, 0, (GLsizei) countVertex ) ;
	SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_STRIP)" ) ;
	//
	if ( fAddColor )
	{
		PutVertexAddColors() ;
		PutGLMaterialForAddColor( pOpenGL ) ;
		//
		glDrawArrays( GL_TRIANGLE_STRIP, 0, (GLsizei) countVertex ) ;
		SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_STRIP)" ) ;
		//
		PutGLMaterialAfterAddColor( pOpenGL ) ;
	}
	if ( !PutGLMaterial( pOpenGL, pMaterial, true ) )
	{
		//
		// 裏面（別属性の場合）描画
		//
		if ( fAddColor )
		{
			PutVertexMulColors() ;
			ReviseGLMaterialForBaseColor( pOpenGL ) ;
		}
		glDrawArrays( GL_TRIANGLE_STRIP, 0, (GLsizei) countVertex ) ;
		SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_STRIP)" ) ;
		//
		if ( fAddColor )
		{
			PutVertexAddColors() ;
			PutGLMaterialForAddColor( pOpenGL ) ;
			//
			glDrawArrays( GL_TRIANGLE_STRIP, 0, (GLsizei) countVertex ) ;
			SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_STRIP)" ) ;
			//
			PutGLMaterialAfterAddColor( pOpenGL ) ;
		}
	}
	//
	FlushVertexPointers() ;
	//
	return	sglErrSuccess ;
}
*/

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLRenderingContext::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	//
	// 表面属性
	//
	PutGLMaterial( pOpenGL, pMaterial, false ) ;
	//
	// 頂点設定
	//
	PutVertexPointer( pvVertex, pvNormal, pvUVMap, countVertex ) ;
	//
	// 頂点色
	//
	bool	fAddColor = PutVertexColors( pColor, countVertex ) ;
	if ( fAddColor )
	{
		ReviseGLMaterialForBaseColor( pOpenGL ) ;
	}
	//
	// 描画（表面 or 両面）
	//
	const GLenum	modeDraw =
			SGLOpenGLVertexBuffer::PrimitiveTypeToGL( typePrimitive ) ;
	uint16_t *	pIndexedBuf = nullptr ;
	if ( pIndexedList != nullptr )
	{
		#if	defined(__API_OPEN_GL_ES__)
			if ( m_bufIndexed16.GetLength() < countIndex )
			{
				m_bufIndexed16.SetLength( countIndex ) ;
			}
			pIndexedBuf = m_bufIndexed16.GetArray() ;
			for ( size_t i = 0; i < countIndex; i ++ )
			{
				pIndexedBuf[i] = (uint16_t) pIndexedList[i] ;
			}
			m_bufIndexed16.FinishArray() ;
			//
			glDrawElements
				( modeDraw, countIndex,
						GL_UNSIGNED_SHORT, m_bufIndexed16.GetConstArray() ) ;
		#else
			glDrawElements
				( modeDraw, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
		#endif
		SGLOpenGLContext::VerifyError( "glDrawElements()" ) ;
	}
	else
	{
		glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
		SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
	}
	//
	if ( fAddColor )
	{
		PutVertexAddColors() ;
		PutGLMaterialForAddColor( pOpenGL ) ;
		//
		if ( pIndexedList != nullptr )
		{
			#if	defined(__API_OPEN_GL_ES__)
				glDrawElements
					( GL_TRIANGLES, countIndex,
						GL_UNSIGNED_SHORT, pIndexedBuf ) ;
			#else
				glDrawElements
					( modeDraw, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
			#endif
			SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;
		}
		else
		{
			glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
			SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
		}
		PutGLMaterialAfterAddColor( pOpenGL ) ;
	}
	if ( !PutGLMaterial( pOpenGL, pMaterial, true ) )
	{
		//
		// 裏面（別属性の場合）描画
		//
		if ( fAddColor )
		{
			PutVertexMulColors() ;
			ReviseGLMaterialForBaseColor( pOpenGL ) ;
		}
		if ( pIndexedList != nullptr )
		{
			#if	defined(__API_OPEN_GL_ES__)
				glDrawElements
					( modeDraw, countIndex,
						GL_UNSIGNED_SHORT, pIndexedBuf ) ;
			#else
				glDrawElements
					( modeDraw, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
			#endif
			SGLOpenGLContext::VerifyError( "glDrawElements()" ) ;
		}
		else
		{
			glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
			SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
		}
		if ( fAddColor )
		{
			PutVertexAddColors() ;
			PutGLMaterialForAddColor( pOpenGL ) ;
			//
			if ( pIndexedList != nullptr )
			{
				#if	defined(__API_OPEN_GL_ES__)
					glDrawElements
						( modeDraw, countIndex,
							GL_UNSIGNED_SHORT, pIndexedBuf ) ;
				#else
					glDrawElements
						( modeDraw, (GLsizei) countIndex, GL_UNSIGNED_INT, pIndexedList ) ;
				#endif
				SGLOpenGLContext::VerifyError( "glDrawElements()" ) ;
			}
			else
			{
				glDrawArrays( modeDraw, 0, (GLsizei) countVertex ) ;
				SGLOpenGLContext::VerifyError( "glDrawArrays()" ) ;
			}
			PutGLMaterialAfterAddColor( pOpenGL ) ;
		}
	}
	//
	FlushVertexPointers() ;
	//
	return	sglErrSuccess ;
}

// 頂点の設定をクリアする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::FlushVertexPointers( void )
{
	glDisable( GL_NORMALIZE ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_NORMALIZE)" ) ;
	glDisableClientState( GL_VERTEX_ARRAY ) ;
	SGLOpenGLContext::VerifyError( "glDisableClientState(GL_VERTEX_ARRAY)" ) ;
	glDisableClientState( GL_TEXTURE_COORD_ARRAY ) ;
	SGLOpenGLContext::VerifyError( "glDisableClientState(GL_TEXTURE_COORD_ARRAY)" ) ;
	glDisableClientState( GL_NORMAL_ARRAY ) ;
	SGLOpenGLContext::VerifyError( "glDisableClientState(GL_NORMAL_ARRAY)" ) ;
	glDisableClientState( GL_COLOR_ARRAY ) ;
	SGLOpenGLContext::VerifyError( "glDisableClientState(GL_COLOR_ARRAY)" ) ;
}

// 頂点を設定する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutVertexPointer
	( const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal, const S2DVector * pvUVMap, size_t nCount )
{
	//
	// 頂点
	//
	glEnableClientState( GL_VERTEX_ARRAY ) ;
	SGLOpenGLContext::VerifyError( "glEnableClientState(GL_VERTEX_ARRAY)" ) ;
	glVertexPointer( 3, GL_FLOAT, sizeof(S3DVector4), pvVertex ) ;
	SGLOpenGLContext::VerifyError( "glVertexPointer" ) ;
	//
	// 法線
	//
	if ( pvNormal != nullptr )
	{
		glEnable( GL_NORMALIZE ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_NORMALIZE)" ) ;
		glEnableClientState( GL_NORMAL_ARRAY ) ;
		SGLOpenGLContext::VerifyError( "glEnableClientState(GL_NORMAL_ARRAY)" ) ;
		glNormalPointer( GL_FLOAT, sizeof(S3DVector4), pvNormal ) ;
		SGLOpenGLContext::VerifyError( "glNormalPointer" ) ;
	}
	else
	{
		glDisableClientState( GL_NORMAL_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glDisableClientState(GL_NORMAL_ARRAY)" ) ;
	}
	//
	// UV マップ
	//
	if ( (pvUVMap != nullptr) && (m_pglLastTexture != nullptr) )
	{
		if ( m_bufUVMap.GetLength() < nCount )
		{
			m_bufUVMap.SetLength( (nCount + 0xFF) & ~0xFF ) ;
		}
		S2DVector *	pvBufUVMap = m_bufUVMap.GetArray() ;
		m_affineTextureUV.TransformVectors( pvBufUVMap, pvUVMap, nCount ) ;
		//
		glEnableClientState( GL_TEXTURE_COORD_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glEnableClientState(GL_TEXTURE_COORD_ARRAY)" ) ;
		glTexCoordPointer( 2, GL_FLOAT, 0, pvBufUVMap ) ;
		SGLOpenGLContext::VerifyError( "glTexCoordPointer" ) ;
		//
		m_bufUVMap.FinishArray() ;
	}
	else
	{
		glDisableClientState( GL_TEXTURE_COORD_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glDisableClientState(GL_TEXTURE_COORD_ARRAY)" ) ;
	}
}

// 頂点色を設定する
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLRenderingContext::PutVertexColors
			( const S3DColor * pColor, size_t nCount )
{
	bool	fAddColor = false ;
	if ( pColor != nullptr )
	{
		if ( m_bufMulColor.GetLength() < nCount * 4 )
		{
			m_bufMulColor.SetLength( (nCount * 4 + 0xFF) & ~0xFF ) ;
		}
		if ( m_pglLastTexture != nullptr )
		{
			if ( m_bufAddColor.GetLength() < nCount * 4 )
			{
				m_bufAddColor.SetLength( (nCount * 4 + 0xFF) & ~0xFF ) ;
			}
			fAddColor =
				ConvertAddColorToFloat
					( m_bufAddColor.GetArray(), pColor, nCount ) ;
			m_bufAddColor.FinishArray() ;
			//
			ConvertMulColorToFloat
				( m_bufMulColor.GetArray(), pColor, nCount ) ;
			m_bufMulColor.FinishArray() ;
		}
		else
		{
			ConvertColorToFloat
				( m_bufMulColor.GetArray(), pColor, nCount ) ;
			m_bufMulColor.FinishArray() ;
		}
		glEnableClientState( GL_COLOR_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glEnableClientState(GL_COLOR_ARRAY)" ) ;
		glColorPointer( 4, GL_FLOAT, 0, m_bufMulColor.GetConstArray() ) ;
		SGLOpenGLContext::VerifyError( "glColorPointer(GL_FLOAT)" ) ;
	}
	else
	{
		glDisableClientState( GL_COLOR_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glDisableClientState(GL_COLOR_ARRAY)" ) ;
		//
		if ( (m_pLastSufAttr->flagsShading
							& shadingMethodMask) != shadingMethodNothing )
		{
			GLfloat	color[4] ;
			if ( m_pglLastTexture != nullptr )
			{
				ConvertMulColorToFloat
					( color, &(m_pLastSufAttr->colorBase), 1 ) ;
			}
			else
			{
				ConvertColorToFloat
					( color, &(m_pLastSufAttr->colorBase), 1 ) ;
			}
			glColor4f( color[0], color[1], color[2], color[3] ) ;
		}
		else
		{
			glColor4f
				( m_alphaEffect, m_alphaEffect, m_alphaEffect, m_alphaEffect ) ;
		}
		SGLOpenGLContext::VerifyError( "glColor4f" ) ;
	}
	return	fAddColor ;
}

// 頂点色を設定する（加算成分）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutVertexAddColors( void )
{
	glColorPointer( 4, GL_FLOAT, 0, m_bufAddColor.GetConstArray() ) ;
	SGLOpenGLContext::VerifyError( "glColorPointer(GL_FLOAT)" ) ;
}

// 頂点色を設定する（乗算成分）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutVertexMulColors( void )
{
	glColorPointer( 4, GL_FLOAT, 0, m_bufMulColor.GetConstArray() ) ;
	SGLOpenGLContext::VerifyError( "glColorPointer(GL_FLOAT)" ) ;
}

// 頂点色変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::ConvertColorToFloat
	( GLfloat * pFloat, const S3DColor * pColor, size_t nCount )
{
	const GLfloat	scaleBy255 = m_alphaEffect / 255.0f ;
	const GLfloat	scaleBy255By3 = 1.0f / (255.0f * 3.0f) ;
	const GLfloat	alphaTrans = m_alphaEffect ;
	GLfloat *		pDst = pFloat ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const S3DColor &	color = pColor[i] ;
		pDst[0] = (GLfloat) color.rgbAdd.argb.Red * scaleBy255 ;
		pDst[1] = (GLfloat) color.rgbAdd.argb.Green * scaleBy255 ;
		pDst[2] = (GLfloat) color.rgbAdd.argb.Blue * scaleBy255 ;
		pDst[3] = alphaTrans
			* (1.0f - (GLfloat) (color.rgbMul.argb.Red
								+ color.rgbMul.argb.Green
								+ color.rgbMul.argb.Blue) * scaleBy255By3) ;
		pDst += 4 ;
	}
}

void SGLOpenGLRenderingContext::ConvertMulColorToFloat
	( GLfloat * pFloat, const S3DColor * pColor, size_t nCount )
{
	const GLfloat	scaleBy255 = m_alphaEffect / 255.0f ;
	const GLfloat	alphaTrans = m_alphaEffect ;
	GLfloat *		pDst = pFloat ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const S3DColor &	color = pColor[i] ;
		pDst[0] = (GLfloat) color.rgbMul.argb.Red * scaleBy255 ;
		pDst[1] = (GLfloat) color.rgbMul.argb.Green * scaleBy255 ;
		pDst[2] = (GLfloat) color.rgbMul.argb.Blue * scaleBy255 ;
		pDst[3] = alphaTrans ;
		pDst += 4 ;
	}
	if ( m_flagMulColor )
	{
		pDst = pFloat ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pDst[0] *= m_colorMulEffect[0] ;
			pDst[1] *= m_colorMulEffect[1] ;
			pDst[2] *= m_colorMulEffect[2] ;
			pDst += 4 ;
		}
	}
}

bool SGLOpenGLRenderingContext::ConvertAddColorToFloat
	( GLfloat * pFloat, const S3DColor * pColor, size_t nCount )
{
	const GLfloat	scaleBy255 = m_alphaEffect / 255.0f ;
	uint32_t		rgbAdd = 0 ;
	GLfloat *		pDst = pFloat ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const S3DColor &	color = pColor[i] ;
		rgbAdd |= color.rgbAdd.ui32 ;
		pDst[0] = (GLfloat) color.rgbAdd.argb.Red * scaleBy255 ;
		pDst[1] = (GLfloat) color.rgbAdd.argb.Green * scaleBy255 ;
		pDst[2] = (GLfloat) color.rgbAdd.argb.Blue * scaleBy255 ;
		pDst[3] = 1.0f ;
		pDst += 4 ;
	}
	if ( m_flagMulColor | m_flagAddColor )
	{
		pDst = pFloat ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pDst[0] = m_colorMulEffect[0] * pDst[0] + m_colorAddEffect[0] ;
			pDst[1] = m_colorMulEffect[1] * pDst[1] + m_colorAddEffect[1] ;
			pDst[2] = m_colorMulEffect[2] * pDst[2] + m_colorAddEffect[2] ;
			pDst += 4 ;
		}
	}
	return	m_flagAddColor | ((rgbAdd & 0x00FFFFFF) != 0) ;
}

// マテリアル設定をクリアする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::FlushGLMaterial( void )
{
	glDisable( GL_LIGHTING ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_LIGHTING)" ) ;
	//
	glDisable( GL_FOG ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_FOG)" ) ;
	//
	GLenum				glTxTarget = GL_TEXTURE_2D ;
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL != nullptr )
	{
		pOpenGL->IsBindingTexture( 0, glTxTarget ) ;
	}
	glBindTexture( glTxTarget, 0 ) ;
	SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D,0)" ) ;
	//
	if ( pOpenGL != nullptr )
	{
		pOpenGL->SetBindTextureInfo( 0, nullptr ) ;
	}
	//
	glDisable( GL_TEXTURE_2D ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
	//
	glDisable( GL_DEPTH_TEST ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
	//
	m_pLastMaterial = nullptr ;
	m_flagBackFace = false ;
	m_flagNeedsMulAlpha = false ;
	m_flagWithoutAlpha = false ;
	m_pLastSufAttr = nullptr ;
	m_pLastTextureImage = nullptr ;
	m_pglLastTexture = nullptr ;
}

// マテリアルを設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLRenderingContext::PutGLMaterial
	( SGLOpenGLContext * pOpenGL, S3DMaterial * pMaterial, bool fBack )
{
	if ( (m_pLastMaterial == pMaterial) & (m_flagBackFace == fBack) )
	{
		// 変更無し
		return	sglErrSuccess ;
	}
	uint64_t				flagsShading ;
	S3DSurfaceAttribute *	pSufAttr ;
	SGLImageObject *		pTextureImage ;
	SGLOpenGLTextureBuffer::GLResource *	pglTexture = nullptr ;
	if ( !fBack )
	{
		pSufAttr = &(pMaterial->m_attrSurface) ;
		pTextureImage = pMaterial->m_pTexture[0] ;
	}
	else
	{
		if ( !(pMaterial->m_flagBack)
			|| !(pMaterial->m_attrSurface.flagsShading
									& shadingSingleSidePlane) )
		{
			return	sglErrInvalidParam ;
		}
		pSufAttr = &(pMaterial->m_attrBack) ;
		pTextureImage = pMaterial->m_pBackTexture[0] ;
	}
	flagsShading = pSufAttr->flagsShading ;
	//
	m_pLastMaterial = pMaterial ;
	m_flagBackFace = fBack ;
	m_pLastSufAttr = pSufAttr ;
	//
	// テクスチャ設定
	//
	if ( !(flagsShading & shadingTextureMapping) )
	{
		pTextureImage = nullptr ;
	}
	pglTexture = BindGLTexture( pTextureImage ) ;
	//
	if ( pglTexture != nullptr )
	{
		PutGLTextureSmoothing
			( (flagsShading & shadingTextureSmoothing)
								&& m_flagTextureSmoothing ) ;
		PutGLTextureTiling
			( (flagsShading & shadingTextureTiling) != 0 ) ;
	}
	//
	// 描画設定
	//
	glEnable( GL_BLEND ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_BLEND)" ) ;
	//
	#if	!defined(__API_OPEN_GL_ES__)
	/*
	glAlphaFunc( GL_GEQUAL, 0.0078125 ) ;
	SGLOpenGLContext::VerifyError( "glAlphaFunc" ) ;
	glEnable( GL_ALPHA_TEST ) ;
	SGLOpenGLContext::VerifyError( "glEnable(GL_ALPHA_TEST)" ) ;
	pOpenGL->m_alphaTestZeroClip = true ;
	*/
	#endif
	//
	if ( m_flagNeedsMulAlpha )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
		SGLOpenGLContext::VerifyError
			( "glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA)" ) ;
		pOpenGL->m_modeBlend = SGLOpenGLContext::blendUnproducted ;
	}
	else if ( m_flagWithoutAlpha )
	{
		glBlendFunc( GL_ONE, GL_ZERO ) ;
		SGLOpenGLContext::VerifyError( "glBlendFunc(GL_ONE,GL_ZERO)" ) ;
		pOpenGL->m_modeBlend = SGLOpenGLContext::blendCopy ;
	}
	else
	{
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ) ;
		SGLOpenGLContext::VerifyError
			( "glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA)" ) ;
		pOpenGL->m_modeBlend = SGLOpenGLContext::blendProducted ;
	}
	if ( (flagsShading & shadingSingleSidePlane) || fBack )
	{
		glEnable( GL_CULL_FACE ) ;		// 片面ポリゴン
		SGLOpenGLContext::VerifyError( "glEnable(GL_CULL_FACE)" ) ;
		if ( !fBack )
		{
			glCullFace( GL_BACK ) ;
			SGLOpenGLContext::VerifyError( "glCullFace(GL_BACK)" ) ;
			pOpenGL->m_faceCullBack = true ;
		}
		else
		{
			glCullFace( GL_FRONT ) ;
			SGLOpenGLContext::VerifyError( "glCullFace(GL_FRONT)" ) ;
			pOpenGL->m_faceCullBack = false ;
		}
		pOpenGL->m_faceDoubleSide = false ;
	}
	else
	{
		glDisable( GL_CULL_FACE ) ;		// 両面ポリゴン
		SGLOpenGLContext::VerifyError( "glDisable(GL_CULL_FACE)" ) ;
		pOpenGL->m_faceDoubleSide = true ;
	}
	if ( flagsShading & shadingNoZBuffer )
	{
		glDisable( GL_DEPTH_TEST ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
		glDepthMask( GL_FALSE ) ;
		SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
		pOpenGL->m_funcDepthTest = shadingNoZBuffer ;
	}
	else
	{
		glEnable( GL_DEPTH_TEST ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
		glDepthFunc( GL_LEQUAL );
		SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
		//
		if ( flagsShading & shadingZBufferNoWrite )
		{
			glDepthMask( GL_FALSE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
			pOpenGL->m_funcDepthTest = shadingZBufferNoWrite ;
		}
		else
		{
			glDepthMask( GL_TRUE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_TRUE)" ) ;
			pOpenGL->m_funcDepthTest = 0 ;
		}
	}
	//
	// 透明度
	//
	if ( (pSufAttr->nTransparency > 0)
		&& ((pglTexture != nullptr) | m_flagLightingGL) )
	{
		if ( pSufAttr->nTransparency >= 0x100 )
		{
			m_alphaEffect = 0.0f ;
		}
		else
		{
			m_alphaEffect =
				m_alphaEnvEffect
					* (GLfloat) (0x100 - pSufAttr->nTransparency) / 0x100 ;
		}
	}
	//
	// 表面属性
	//
	if ( m_flagLightingGL )
	{
		//
		// 基本色
		//
		const GLfloat	scaleBy255 = 1.0f / 255.0f ;
		const GLfloat	scaleBy256 = 1.0f / 256.0f ;
		GLfloat	rgba[4], rgbaShade[4] ;
		bool	fShading = true ;
		if ( (flagsShading & shadingMethodMask) == shadingMethodNothing )
		{
			rgba[0] = 1.0f ;
			rgba[1] = 1.0f ;
			rgba[2] = 1.0f ;
			rgba[3] = 1.0f ;
			rgbaShade[0] = 0.0f ;
			rgbaShade[1] = 0.0f ;
			rgbaShade[2] = 0.0f ;
			rgbaShade[3] = 1.0f ;
			fShading = false ;
		}
		else if ( pglTexture != nullptr )
		{
			rgba[0] = (GLfloat) pSufAttr->colorBase.rgbMul.argb.Red * scaleBy255 ;
			rgba[1] = (GLfloat) pSufAttr->colorBase.rgbMul.argb.Green * scaleBy255 ;
			rgba[2] = (GLfloat) pSufAttr->colorBase.rgbMul.argb.Blue * scaleBy255 ;
			rgba[3] = 1.0f ;
			//
			rgbaShade[0] = (GLfloat) pSufAttr->colorShade.rgbMul.argb.Red * scaleBy255 ;
			rgbaShade[1] = (GLfloat) pSufAttr->colorShade.rgbMul.argb.Green * scaleBy255 ;
			rgbaShade[2] = (GLfloat) pSufAttr->colorShade.rgbMul.argb.Blue * scaleBy255 ;
			rgbaShade[3] = 1.0f ;
		}
		else
		{
			ConvertColorToFloat( &rgba[0], &(pSufAttr->colorBase), 1 ) ;
			ConvertColorToFloat( &rgbaShade[0], &(pSufAttr->colorShade), 1 ) ;
		}
		//
		// マテリアル設定
		//
		GLfloat	params[4] ;
		if ( fShading )
		{
			rgba[0] -= rgbaShade[0] ;
			rgba[1] -= rgbaShade[1] ;
			rgba[2] -= rgbaShade[2] ;
			//
			params[0] = 0 ;
			params[1] = 0 ;
			params[2] = 0 ;
			params[3] = 0 ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_AMBIENT)" ) ;
			//
			GLfloat	fDiffusion = (GLfloat) pSufAttr->nDiffusion * scaleBy256 ;
			params[0] = rgba[0] * fDiffusion ;
			params[1] = rgba[1] * fDiffusion ;
			params[2] = rgba[2] * fDiffusion ;
			params[3] = rgba[3] ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_DIFFUSE)" ) ;
			//
			GLfloat	fSpecular = (GLfloat) pSufAttr->nSpecular * scaleBy256 ;
			params[0] = rgba[0] * fSpecular ;
			params[1] = rgba[1] * fSpecular ;
			params[2] = rgba[2] * fSpecular ;
			params[3] = rgba[3] ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_SPECULAR)" ) ;
			//
			GLfloat	fShiness = (GLfloat) pSufAttr->nSpecularSize * 0.5f ;
			params[0] = fShiness ;
			params[1] = fShiness ;
			params[2] = fShiness ;
			params[3] = 1.0f ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_SHININESS)" ) ;
			//
			GLfloat	fAmbient = (GLfloat) pSufAttr->nAmbient * scaleBy256 ;
			params[0] = rgba[0] * fAmbient + rgbaShade[0] ;
			params[1] = rgba[1] * fAmbient + rgbaShade[1] ;
			params[2] = rgba[2] * fAmbient + rgbaShade[2] ;
			params[3] = rgba[3] ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_EMISSION)" ) ;
			//
			glShadeModel( GL_SMOOTH ) ;
			SGLOpenGLContext::VerifyError( "glShadeModel(GL_SMOOTH)" ) ;
		}
		else
		{
			params[0] = 0.0f ;
			params[1] = 0.0f ;
			params[2] = 0.0f ;
			params[3] = 1.0f ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_AMBIENT)" ) ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_DIFFUSE)" ) ;
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, params ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_SPECULAR)" ) ;
			//
			glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, rgba ) ;
			SGLOpenGLContext::VerifyError( "glMaterialfv(GL_EMISSION)" ) ;
		}
	}
	else
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ;
		SGLOpenGLContext::VerifyError( "glColor4f" ) ;
	}
	return	sglErrSuccess ;
}

// テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLTextureBuffer::GLResource *
	SGLOpenGLRenderingContext::BindGLTexture( SGLImageObject * pTextureImage )
{
	if ( pTextureImage == m_pLastTextureImage )
	{
		return	m_pglLastTexture ;
	}
	if ( pTextureImage != nullptr )
	{
		SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		SGLImageRect	rectRefTexture ;
		SGLOpenGLTextureBuffer::GLResource *
			pglTexture =
				SGLOpenGLTextureBuffer::CommitGLTextureRsrc
					( pOpenGL, pTextureImage, rectRefTexture ) ;
		if ( pglTexture == m_pglLastTexture )
		{
			return	pglTexture ;
		}
		if ( (pglTexture != nullptr) && (pglTexture->m_glTexture != 0) )
		{
			glEnable( GL_TEXTURE_2D ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
			//
			glBindTexture( GL_TEXTURE_2D, pglTexture->m_glTexture ) ;
			SGLOpenGLContext::VerifyError( "glBindTexture(GL_TEXTURE_2D)" ) ;
			//
			pOpenGL->SetBindTextureInfo( 0, pglTexture, GL_TEXTURE_2D ) ;
			//
			m_affineTextureUV.a11 =
					1.0f / (float32_t) pglTexture->m_imginf.width ;
			m_affineTextureUV.a12 = 0.0f ;
			m_affineTextureUV.a13 =
					(float32_t) rectRefTexture.x
						/ (float32_t) pglTexture->m_imginf.width ;
			m_affineTextureUV.a21 = 0.0f ;
			m_affineTextureUV.a22 =
					1.0f / (float32_t) pglTexture->m_imginf.height ;
			m_affineTextureUV.a23 =
					(float32_t) rectRefTexture.y
						/ (float32_t) pglTexture->m_imginf.height ;
			//
			m_flagNeedsMulAlpha =
				((pglTexture->m_imginf.format
					& formatImageFlagNoProductOfAlpha) != 0) ;
			m_flagWithoutAlpha =
				((pglTexture->m_imginf.format & formatImageFlagAlpha) == 0) ;
		}
		else
		{
			glDisable( GL_TEXTURE_2D ) ;
			SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
			//
			m_flagNeedsMulAlpha = false ;
			m_flagWithoutAlpha = false ;
		}
		m_pglLastTexture = pglTexture ;
	}
	else
	{
		glDisable( GL_TEXTURE_2D ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
		m_pglLastTexture = nullptr ;
		m_flagNeedsMulAlpha = false ;
		m_flagWithoutAlpha = false ;
	}
	m_pLastTextureImage = pTextureImage ;
	return	m_pglLastTexture ;
}

// テクスチャ補完設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutGLTextureSmoothing( bool fSmoothing )
{
	if ( fSmoothing )
	{
		glTexParameteri
			( GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
		glTexParameteri
			( GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
	}
	else
	{
		glTexParameteri
			( GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER)" ) ;
		glTexParameteri
			( GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER)" ) ;
	}
}

// テクスチャ繰り返し設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutGLTextureTiling( bool fTiling )
{
	if ( fTiling )
	{
		glTexParameteri
			( GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
		glTexParameteri
			( GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
	}
	else
	{
		glTexParameteri
			( GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S)" ) ;
		glTexParameteri
			( GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) ;
		SGLOpenGLContext::VerifyError
			( "glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T)" ) ;
	}
}

// ベース色レンダリング用マテリアル補正
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::ReviseGLMaterialForBaseColor( SGLOpenGLContext * pOpenGL )
{
//	glDepthMask( GL_FALSE ) ;
//	SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
}

// 加算色レンダリング用マテリアル設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutGLMaterialForAddColor( SGLOpenGLContext * pOpenGL )
{
	/*
	ESLAssert( m_pLastSufAttr != nullptr ) ;
	if ( !(m_pLastSufAttr->flagsShading
				& (shadingNoZBuffer | shadingZBufferNoWrite)) )
	{
		glDepthMask( GL_TRUE ) ;
		SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
	}
	*/
	glBlendFunc( GL_ONE, GL_ONE ) ;
	SGLOpenGLContext::VerifyError( "glBlendFunc(GL_ONE,GL_ONE)" ) ;
	//
	glDisable( GL_TEXTURE_2D ) ;
	SGLOpenGLContext::VerifyError( "glDisable(GL_TEXTURE_2D)" ) ;
	//
	pOpenGL->m_modeBlend = SGLOpenGLContext::blendAdd ;
}

// 加算色レンダリング用後マテリアル再設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLRenderingContext::PutGLMaterialAfterAddColor( SGLOpenGLContext * pOpenGL )
{
	if ( m_flagNeedsMulAlpha )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
		SGLOpenGLContext::VerifyError
				( "glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA)" ) ;
		pOpenGL->m_modeBlend = SGLOpenGLContext::blendUnproducted ;
	}
	else
	{
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ) ;
		SGLOpenGLContext::VerifyError
				( "glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA)" ) ;
		pOpenGL->m_modeBlend = SGLOpenGLContext::blendProducted ;
	}
	if ( m_pglLastTexture != nullptr )
	{
		glEnable( GL_TEXTURE_2D ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_TEXTURE_2D)" ) ;
	}
}


