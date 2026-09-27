#!/usr/bin/env python3
"""Correct GL capability probes in generated SDL SDK sources, never the SDK.

``transform_context(text)`` accepts the original or OS-macro-normalized
sgl_opengl_context.cpp. Replacements are deliberately exact: a different SDK
revision must be reviewed rather than silently receiving a partial patch.
"""


def _replace(text: str, old: str, new: str) -> str:
    count = text.count(old)
    if count != 1:
        raise ValueError(f"SDK GL capability anchor changed ({count} matches): {old[:96]!r}")
    return text.replace(old, new)


def transform_context(text: str) -> str:
    # GL_MAJOR/MINOR_VERSION were introduced in GL/ES 3.0. The SDK's existing
    # GL_VERSION parser works on 2.1 too, without deliberately raising GL errors.
    start = text.index("\tGLint\tverGL[2] ;")
    end = text.index("\t\tconst GLubyte *\tpbytVer = glGetString( GL_VERSION ) ;", start)
    expected = text[start:end]
    if not all(token in expected for token in
               ("GL_MAJOR_VERSION", "GL_MINOR_VERSION", "if ( !fGotVersion )")):
        raise ValueError("SDK GL version probe changed")
    text = _replace(text, expected, "\t// GL_VERSION is valid on desktop GL 2.1 and every supported GLES context.\n\t{\n")

    # GLES multisampling is determined by the framebuffer; GL_MULTISAMPLE is
    # not an Enable/Disable capability (ES 3.0 reference, section 4.1.3).
    # Retain desktop calls, FBO allocation/blits and SDK state bookkeeping.
    # These are the five reviewed calls in initialization, attachment and blit.
    for indent, action, before_semicolon in (
            ("\t\t", "Disable", " "), ("\t\t\t", "Enable", ""),
            ("\t\t\t", "Disable", ""), ("\t", "Disable", " "),
            ("\t\t", "Enable", " ")):
        old = (f'{indent}gl{action}( GL_MULTISAMPLE ){before_semicolon};\n'
               f'{indent}SGLOpenGLContext::VerifyError( "gl{action}(GL_MULTISAMPLE)" ) ;')
        text = _replace(text, old, f'{indent}#if !defined(__API_OPEN_GL_ES__)\n'
                        + old + f'\n{indent}#endif')

    old = '''\t\tglGetIntegerv( GL_MAX_TEXTURE_UNITS, &m_maxMultiTextureUnits ) ;
\t\tSGLOpenGLContext::VerifyError( "glGetIntegerv(GL_MAX_TEXTURE_UNITS)" ) ;'''
    text = _replace(text, old, '''\t\t#if !defined(__API_OPEN_GL_ES__)
''' + old + '''
\t\t#endif''')
    old = '''\tif ( m_versionGL[0] >= 3 )
\t{
\t\tglGetIntegerv( GL_MAX_IMAGE_UNITS, &m_maxImageUnits ) ;'''
    text = _replace(text, old, '''\t#if defined(__API_OPEN_GL_ES__)
\t// Image load/store is GLES 3.1, not GLES 3.0.
\tconst bool supportsImageUnits = GetMaxGLSLVersion() >= 310 ;
\t#else
\tconst bool supportsImageUnits = GetMaxGLSLVersion() >= 420
\t\t|| IsExtensionSupported( L"GL_ARB_shader_image_load_store" ) ;
\t#endif
\tif ( supportsImageUnits )
\t{
\t\tglGetIntegerv( GL_MAX_IMAGE_UNITS, &m_maxImageUnits ) ;''')

    # SDK limits are measured in vec4 slots. Desktop GL 2.1 exposes components
    # instead of the later ES2-compatibility *_VECTORS aliases.
    for field, vectors, components in (
            ("m_maxVertexVaryings", "GL_MAX_VARYING_VECTORS", "GL_MAX_VARYING_FLOATS"),
            ("m_maxVertexUniforms", "GL_MAX_VERTEX_UNIFORM_VECTORS", "GL_MAX_VERTEX_UNIFORM_COMPONENTS"),
            ("m_maxFragmentUniforms", "GL_MAX_FRAGMENT_UNIFORM_VECTORS", "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS")):
        old = f'''\tglGetIntegerv( {vectors}, &{field} ) ;
\tSGLOpenGLContext::VerifyError( "glGetIntegerv({vectors})" ) ;'''
        text = _replace(text, old, '''\t#if defined(__API_OPEN_GL_ES__)
''' + old + f'''
\t#else
\tglGetIntegerv( {components}, &{field} ) ;
\tSGLOpenGLContext::VerifyError( "glGetIntegerv({components})" ) ;
\t{field} /= 4 ;
\t#endif''')

    old = '''\tm_flagSupportedGeometry =
\t\tIsExtensionSupported( L"GL_EXT_geometry_shader" )
\t\t\t|| (m_versionGL[0] >= 4)
\t\t\t|| ((m_versionGL[0] == 3) && (m_versionGL[1] >= 2)) ;'''
    text = _replace(text, old, '''\t// The SDK emits core geometry shaders, not extension-specific syntax.
\t#if defined(__API_OPEN_GL_ES__)
\tm_flagSupportedGeometry = GetMaxGLSLVersion() >= 320 ;
\t#else
\tm_flagSupportedGeometry = GetMaxGLSLVersion() >= 150 ;
\t#endif''')

    old = '''\tm_flagAvailableMultiSVB =
\t\tOpenGLExtension::g_supports_instanced_draw'''
    text = _replace(text, old, '''\t// Instanced attribute arrays can exist on GL 2.1 without the core
\t// gl_InstanceID/gl_VertexID used by the SDK's vertex-texture shader.
\t#if defined(__API_OPEN_GL_ES__)
\tconst bool supportsVertexTextureShader = GetMaxGLSLVersion() >= 300 ;
\t#else
\tconst bool supportsVertexTextureShader = GetMaxGLSLVersion() >= 140 ;
\t#endif
\tm_flagAvailableMultiSVB = supportsVertexTextureShader
\t\t&& OpenGLExtension::g_supports_instanced_draw''')

    old = '''\tglGetIntegerv( GL_NUM_PROGRAM_BINARY_FORMATS, &numProgramBinaryFormats ) ;
\tSGLOpenGLContext::VerifyError( "glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS)" ) ;'''
    text = _replace(text, old, '''\t#if defined(__API_OPEN_GL_ES__)
\tconst bool supportsProgramBinary = m_versionGL[0] >= 3
\t\t|| IsExtensionSupported( L"GL_OES_get_program_binary" ) ;
\t#else
\tconst bool supportsProgramBinary = GetMaxGLSLVersion() >= 410
\t\t|| IsExtensionSupported( L"GL_ARB_get_program_binary" ) ;
\t#endif
\tif ( supportsProgramBinary && OpenGLExtension::g_supports_program_binary )
\t{
\t\tglGetIntegerv( GL_NUM_PROGRAM_BINARY_FORMATS, &numProgramBinaryFormats ) ;
\t\tSGLOpenGLContext::VerifyError( "glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS)" ) ;
\t}''')

    old = '''int SGLOpenGLContext::GetMaxGLSLVersion( void ) const
{
'''
    text = _replace(text, old, old + '''#if defined(__API_OPEN_GL_ES__)
\tif ( m_versionGL[0] < 2 ) return 0 ;
\tif ( m_versionGL[0] == 2 ) return 100 ;
\treturn m_versionGL[0] * 100 + m_versionGL[1] * 10 ;
#else
''')
    old = "\treturn\tm_versionGL[0] * 100 + nMinor ;\n}"
    text = _replace(text, old, "\treturn\tm_versionGL[0] * 100 + nMinor ;\n#endif\n}")

    old = "\tint\t\t\t\tnVersion = 0 ;"
    text = _replace(text, old, old + '''
\t#if !defined(__API_OPEN_GL_ES__)
\t// Keep the desktop 2.1 shader path explicit, including its GLSL 1.20 features.
\tnVersion = m_pOpenGL->GetMaxGLSLVersion() >= 120 ? 120 : 110 ;
\tstrVerHeader.Format( L"#version %d\\r\\n", nVersion ) ;
\t#endif''')
    old = '''\tif ( m_enabledVTInstancing )
\t{
\t\tif ( nVersion < 140 )'''
    text = _replace(text, old, '''\tif ( m_enabledVTInstancing )
\t{
\t\t#if defined(__API_OPEN_GL_ES__)
\t\tconst int vertexTextureGLSLVersion = 300 ;
\t\t#else
\t\tconst int vertexTextureGLSLVersion = 140 ;
\t\t#endif
\t\tif ( nVersion < vertexTextureGLSLVersion )''')
    old = '''\t\t\tnVersion = 310 ;
\t\t\tstrVerHeader = L"#version 310 es\\r\\n" ;'''
    text = _replace(text, old, '''\t\t\t// Vertex/instance IDs and vertex texture sampling are GLES 3.0 core.
\t\t\tnVersion = 300 ;
\t\t\tstrVerHeader = L"#version 300 es\\r\\n" ;''')
    return text
