
#ifdef GL_ES
	precision highp float ;
	#ifdef	_DISABLE_FLOAT_PRECISION_
		#define	lowp
		#define	highp
		#define	mediump
	#endif
#else
	#define	lowp
	#define	highp
	#define	mediump
#endif

#ifdef	_GLSL_VERSION_130_LATER_
// OpenGL 3.0, GLSL 1.3 以降の構文への対応
#define	attribute		in
#define	varying			out
#define	texture2D		texture
#define	texture3D		texture
#define	textureCube		texture
#define	texture2DArray	texture
#endif

#define	_VERTEX_SHADER	1


///////////////////////////////////////////////////////////////////////////////
// 頂点属性
///////////////////////////////////////////////////////////////////////////////

// 頂点情報
attribute highp vec3	a_vVertexPosition ;		// 頂点座標
attribute mediump vec3	a_vVertexNormal ;		// 法線
#ifdef	_PHONG_SHADER
#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
attribute mediump vec3	a_vVertexMappingX ;		// テクスチャマッピングｘ基底
attribute mediump vec3	a_vVertexMappingY ;		// テクスチャマッピングｙ基底
#endif
#endif
attribute mediump vec2	a_vTextureCoord ;		// UV座標
attribute mediump vec4	a_vVertexMulColor ;		// 頂点色（α付き）
attribute mediump vec4	a_vVertexAddColor ;



#ifdef	_ENABLE_VT_INSTANCING

// 浮動小数点テクスチャでモーフィング・ボーンパラメータ指定

uniform sampler2D	u_samplerVertex ;			// テクスチャ（頂点バッファ）
uniform highp vec2	u_vVertexTextureScale ;		// 個数→テクスチャUV変換（サイズ逆数：サイズは２の累乗）
uniform bool		u_bVertexMorphing ;			// モーフィング有効
uniform bool		u_bVertexBoneInstance ;		// ボーン有効
uniform highp float	u_yVertexMorphingFirst ;	// モーフィングバッファのｙ座標
uniform highp float	u_yNormalMorphingFirst ;	// （※頂点・法線は差分値）
uniform highp float	u_yVertexMorphingStride ;	// １つのモーフィング配列の行数 * u_vVertexTextureScale.y
uniform highp float	u_yVertexMorphingInstance ;	// モーフィング適用度配列のｙ座標
												//（（ウェイト４＋指標４）×インスタンス）
uniform highp float	u_xVertexMorphingInstanceStride ;	// { 0.0 | u_vVertexTextureScale.y * 2 }
uniform highp float	u_yVertexBoneWeight ;		// ボーンウェイト配列（４要素配列）
uniform highp float	u_yVertexBoneIndex ;		// ボーンインデックス配列（４要素配列）
uniform highp float	u_yVertexBoneMatrix ;		// ボーン行列配列（４×３×ボーン数）
												// ※ボーンのインスタンスは必ず１行に収め
												// 　次のインスタンスは次の行に移る
uniform highp float	u_yVertexBoneMatrixStride ;	// { 0.0 | u_vVertexTextureScale.y }
uniform highp float	u_yVertexExAttrElements ;	// 拡張頂点属性のｙ座標 * u_vVertexTextureScale.y
uniform highp float	u_xVertexExAttrStride ;		// 拡張頂点属性の１要素ピクセル数

#else

// モーフィング
#ifndef	_DISABLE_MORPHING
attribute highp vec3	a_vMorphPosition ;		// 頂点座標（モーフターゲット）
attribute mediump vec3	a_vMorphNormal ;		// 法線（モーフターゲット）
#ifdef	_PHONG_SHADER
#ifndef	_DISABLE_UV_AXIS_ATTRIBUTE
attribute mediump vec3	a_vMorphMappingX ;		// テクスチャマッピングｘ基底（モーフターゲット）
attribute mediump vec3	a_vMorphMappingY ;		// テクスチャマッピングｙ基底（モーフターゲット）
#endif
#endif
attribute mediump vec2	a_vMorphTextureCoord ;	// UV座標（モーフターゲット）
attribute mediump vec4	a_vMorphMulColor ;		// 頂点色（モーフターゲット）
attribute mediump vec4	a_vMorphAddColor ;
#endif

// ボーン適用度
#ifndef	_DISABLE_BONE_OVER_0
attribute mediump vec4	a_vVertexBoneWeight0 ;
#ifndef	_DISABLE_BONE_OVER_4
attribute mediump vec4	a_vVertexBoneWeight4 ;
#ifndef	_DISABLE_BONE_OVER_8
attribute mediump vec4	a_vVertexBoneWeight8 ;
#ifndef _DISABLE_BONE_OVER_12
attribute mediump vec4	a_vVertexBoneWeight12 ;
#endif
#endif
#endif
#endif

#endif


// インスタンス情報
#ifdef	_ENABLE_INSTANCING
attribute highp vec4	a_matInstancingModelView0 ;	// 座標変換
attribute highp vec4	a_matInstancingModelView1 ;
attribute highp vec4	a_matInstancingModelView2 ;
attribute mediump vec4	a_vInstancingMulColor ;		// 頂点色（α付き）
attribute mediump vec4	a_vInstancingAddColor ;
#endif

