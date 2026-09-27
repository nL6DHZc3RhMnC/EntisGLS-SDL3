

#ifndef	_DISABLE_SHADOWMAPPING
#ifdef	_FRAGMENT_SHADER
float effect_shadow_mapping
	( in mediump sampler2DArray samplerShadowmap,
		in int iShadowmapLayer, in highp vec2 vShadowmapUnit,
		in highp float fpFixErrorGap, in highp float fpVarErrorGap,
		in highp vec3 vShadowmap,
		in mediump vec3 vLightDirection, in mediump vec3 vNormal )
{
	float	sinLight = dot( vNormal, vLightDirection ) ;
	float	cos2Light = 1.0 - sinLight * sinLight ;
	float	zThreshold = vShadowmap.z
							* (fpFixErrorGap - fpVarErrorGap * cos2Light) ;
	float	zLayer = float( iShadowmapLayer ) ;
	vec3	vMap1 = vec3( vShadowmap.xy, zLayer ) ;
#ifndef	_PREPARED_SHADOW_DEPTH_BUFFER
	vec3	vMap0 = vMap1 - vec3( vShadowmapUnit, 0.0 ) ;
	vec3	vMap2 = vMap1 + vec3( vShadowmapUnit, 0.0 ) ;
	vec3	zShadow0 ;
	vec3	zShadow1 ;
	vec3	zShadow2 ;
	vec3	zTemp ;
	zShadow0.x = texture2DArray( samplerShadowmap, vMap0 ).x ;
	zShadow0.y = texture2DArray( samplerShadowmap, vec3( vMap1.x, vMap0.y, zLayer ) ).x ;
	zShadow0.z = texture2DArray( samplerShadowmap, vec3( vMap2.x, vMap0.y, zLayer ) ).x ;
	zShadow1.x = texture2DArray( samplerShadowmap, vec3( vMap0.x, vMap1.y, zLayer ) ).x ;
	zShadow1.y = texture2DArray( samplerShadowmap, vMap1 ).x ;
	zShadow1.z = texture2DArray( samplerShadowmap, vec3( vMap2.x, vMap1.y, zLayer ) ).x ;
	zShadow2.x = texture2DArray( samplerShadowmap, vec3( vMap0.x, vMap2.y, zLayer ) ).x ;
	zShadow2.y = texture2DArray( samplerShadowmap, vec3( vMap1.x, vMap2.y, zLayer ) ).x ;
	zShadow2.z = texture2DArray( samplerShadowmap, vMap2 ).x ;
	//
	zTemp = zShadow1 * 0.5 + (zShadow0 + zShadow2) * 0.25 ;
	//
	float	zAvgShadow =  zTemp.y * 0.5 + (zTemp.x + zTemp.z) * 0.25 ;

	zTemp = zShadow1 * zShadow1 * 0.5
				+ (zShadow0 * zShadow0
					+ zShadow2 * zShadow2) * 0.25 ;
	float	zAvgSqrShadow = 
				zTemp.y * 0.5 + (zTemp.x + zTemp.z) * 0.25 ;
#else
	vec4	vShadowBufDepth = texture2DArray( samplerShadowmap, vMap1 ) ;
	float	zAvgShadow = vShadowBufDepth.x ;
	float	zAvgSqrShadow = vShadowBufDepth.y ;
#endif
	float	aShadow = 1.0 ;
	float	zSigPow2 = max( zAvgSqrShadow - zAvgShadow * zAvgShadow, 0.00000001 ) ;

	if ( zThreshold > zAvgShadow )
	{
		//
		// E(x) = zAvgShadow
		// E(x^2) = zAvgSqrShadow
		// σ^2 = E(x^2) - E(x)^2
		//                  σ^2
		// P(x≧t) ≦ ――――――――――
		//            σ^2 + (t - E(x))^2
		//
		float	tu = zThreshold - zAvgShadow ;
		aShadow = zSigPow2 / max( zSigPow2 + tu * tu, 0.00000001 ) ;
	}
	return	aShadow ;
}
#endif
#endif

float effect_light_shadow_mapping
	( in int iLight, in highp vec3 vPosition,
			in mediump vec3 vLightDirection, in mediump vec3 vNormal )
{
	float	aShadow = 1.0 ;
	
	#ifndef	_DISABLE_SHADOWMAPPING
	highp vec4	vTempPos = vec4( vPosition, 1.0 ) ;
	int			nShadowing = 0 ;
	for ( int i = 0; i < _LIMIT_SHADOWMAP_COUNT; i ++ )
	{
		if ( u_iEnableShadowmap[i] == iLight )
		{
			highp vec4	vViewPos = u_mat4ModelViewShadowmap[i] * vTempPos ;
			highp vec4	v4ShadowMap = u_mat4PerspectiveShadowmap[i] * vViewPos ;
			highp vec3	vShadowmap = v4ShadowMap.xyz * (0.5 / v4ShadowMap.w) + 0.5 ;
			if ( (abs(vShadowmap.x - 0.5) < 0.5)
					&& (abs(vShadowmap.y - 0.5) < 0.5)
					&& (vViewPos.z > 0.0) )
			{
				aShadow = min( effect_shadow_mapping(
						#ifdef GL_ES
						u_samplerShadowmap,
						#else
						u_samplerShadowmap[i],
						#endif
						u_iShadowmapDepthLayer[i], u_vShadowmapUnit[i],
						u_fpShadowmapFixErrorGap[i],
						u_fpShadowmapVarErrorGap[i],
						vShadowmap, vLightDirection, vNormal ), aShadow ) ;
				if ( ++ nShadowing >= 2 )
				{
					break ;
				}
			}
		}
	}
	#endif

	return	aShadow ;
}


