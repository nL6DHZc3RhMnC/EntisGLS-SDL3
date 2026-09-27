
///////////////////////////////////////////////////////////////////////////////
// 頂点シェーダー（フォン・シェーディング：水面変位）
///////////////////////////////////////////////////////////////////////////////

uniform mediump float	u_fpAmplitude ;			// 振幅比
//uniform mediump float	u_fpNormalAmp ;			// 法線振幅比
uniform mediump vec4	u_vAmplitude ;			// 振幅
uniform mediump vec4	u_vFrequency ;			// 周波数 [Hz/2π]
uniform mediump vec4	u_vTime ;				// 時間 [rad]
uniform mediump vec2	u_vDirection[4] ;		// 方向
uniform mediump vec3	u_vWaterAxisX ;
uniform mediump vec3	u_vWaterAxisY ;

varying mediump vec2	v_vWaveCoord ;


void main( void )
{
	// ボーン処理
	morph_vertex() ;

	// 座標変換・ボーン処理
	transform_vertex
		( v_vPosition, v_vNormal, v_vPosition.xyz, v_vNormal ) ;

	// 水面オフセット計算
	vec4	vWavePosition = v_vPosition - u_mat4CameraView * vec4( 0.0, 0.0, 0.0, 1.0 ) ;
	vec3	vWaterAxisX = u_mat3CameraViewForNormal * u_vWaterAxisX ;
	vec3	vWaterAxisY = u_mat3CameraViewForNormal * u_vWaterAxisY ;
	vec3	vAxisCross = normalize( cross( vWaterAxisX, vWaterAxisY ) ) ;
	vec2	vWaveCoord =
				vec2( dot( cross( vWavePosition.xyz, vWaterAxisY ), vAxisCross ),
						dot( cross( vWaterAxisX, vWavePosition.xyz ), vAxisCross ) ) ;
	vec2	a2 = u_vDirection[0] ;
	vec2	b2 = u_vDirection[1] ;
	vec2	c2 = u_vDirection[2] ;
	vec2	d2 = u_vDirection[3] ;
	vec4	phase2 = u_vFrequency
						* vec4( dot( a2, vWaveCoord ),
								dot( b2, vWaveCoord ),
								dot( c2, vWaveCoord ),
								dot( d2, vWaveCoord ) ) + u_vTime ;
//	vec4	vCos2 = cos( phase2 ) * u_vAmplitude ;
	vec4	vSin2 = sin( phase2 ) ;
//	vec4	xABCD2 = vec4( a2.x, b2.x, c2.x, d2.x ) ;
//	vec4	yABCD2 = vec4( a2.y, b2.y, c2.y, d2.y ) ;
	vec3	vOffset2 =
				vec3( 0.0 /*dot( vCos2, xABCD2 )*/,
						dot( vSin2, u_vAmplitude ),
						0.0 /*dot( vCos2, yABCD2 )*/ ) ;
	//
	vec3	vNormal = normalize( v_vNormal ) ;
	v_vPosition += vec4( vNormal * (vOffset2.y * u_fpAmplitude), 0.0 ) ;
	//
//	vOffset2 = normalize( vOffset2 ) ;
//	vNormal += normalize( vWaterAxisX ) * (vOffset2.x * 0.5 * u_fpNormalAmp)
//				+ normalize( vWaterAxisY ) * (vOffset2.z * 0.5 * u_fpNormalAmp) ;
//	v_vNormal = normalize( vNormal ) ;

	// 透視変換
	gl_Position = u_mat4PerspectiveView * v_vPosition ;

	v_vWaveCoord = vWaveCoord ;
	v_vVertexMulColor.a *= u_fpEffectAlpha ;
}


