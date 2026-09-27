
#ifdef GL_ES
	#ifdef	_DISABLE_FLOAT_PRECISION_
		precision mediump float ;
		#define	lowp
		#define	highp
		#define	mediump
	#else
		#ifdef	GL_FRAGMENT_PRECISION_HIGH
			precision highp float ;
		#else
			precision mediump float ;
		#endif
	#endif
#else
	#define	lowp
	#define	highp
	#define	mediump
#endif

#ifdef	_GLSL_VERSION_130_LATER_
// OpenGL 3.0, GLSL 1.3 以降の構文への対応
#define	varying			in
#define	texture2D		texture
#define	texture3D		texture
#define	textureCube		texture
#define	texture2DArray	texture
#endif

#define	_FRAGMENT_SHADER	1


void write_frag1( in mediump vec4 vColor )
{
	#ifndef	_MULTIPLE_RENDER_TARGET
		gl_FragColor = vColor ;
	#else
		fr_FragData[0] = vColor ;
	#endif
}

void write_frag2( in mediump vec4 vColor, in mediump vec3 vEmission )
{
	#ifndef	_MULTIPLE_RENDER_TARGET
		gl_FragColor = vColor ;
	#else
		fr_FragData[0] = vColor ;
		#if	_MULTIPLE_RENDER_TARGET >= 2
			fr_FragData[1] = vec4( vEmission, vColor.a ) ;
		#endif
	#endif
}

void write_frag3( in mediump vec4 vColor,
					in mediump vec3 vEmission, in mediump vec3 vNormal )
{
	#ifndef	_MULTIPLE_RENDER_TARGET
		gl_FragColor = vColor ;
	#else
		fr_FragData[0] = vColor ;
		#if	_MULTIPLE_RENDER_TARGET >= 2
			fr_FragData[1] = vec4( vEmission, vColor.a ) ;
		#endif
		#if	_MULTIPLE_RENDER_TARGET >= 3
			fr_FragData[2] =
				vec4( (normalize(vNormal) + 1.0) * (0.5 * vColor.a), vColor.a ) ;
		#endif
	#endif
}

void write_frag6( in mediump vec4 vColor, in mediump vec3 vEmission,
					in mediump vec3 vNormal, in mediump vec3 vDiffusion,
					in mediump vec3 vAmbient, in mediump vec3 vSpecular )
{
	#ifndef	_MULTIPLE_RENDER_TARGET
		gl_FragColor = vColor ;
	#else
		fr_FragData[0] = vColor ;
		#if	_MULTIPLE_RENDER_TARGET >= 2
			fr_FragData[1] = vec4( vEmission, vColor.a ) ;
		#endif
		#if	_MULTIPLE_RENDER_TARGET >= 3
			fr_FragData[2] =
				vec4( (normalize(vNormal) + 1.0) * (0.5 * vColor.a), vColor.a ) ;
		#endif
		#if	_MULTIPLE_RENDER_TARGET >= 4
			fr_FragData[3] = vec4( vDiffusion * vColor.a, vColor.a ) ;
		#endif
		#if	_MULTIPLE_RENDER_TARGET >= 5
			fr_FragData[4] = vec4( vAmbient * vColor.a, vColor.a ) ;
		#endif
		#if	_MULTIPLE_RENDER_TARGET >= 6
			fr_FragData[5] = vec4( vSpecular * vColor.a, vColor.a ) ;
		#endif
	#endif
}

void write_depth( in highp float fpDepth )
{
	#ifdef	_GLSL_VERSION_130_LATER_
		#ifdef	_ENABLE_COMPUTE_DEPTH_
			gl_FragDepth  = fpDepth ;
		#endif
	#endif
}


