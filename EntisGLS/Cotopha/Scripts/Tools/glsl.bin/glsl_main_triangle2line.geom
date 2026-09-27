layout(triangles) in ;
layout(line_strip, max_vertices = 4) out ;
in highp vec4 g_vPosition[3] ;
in mediump vec3 g_vNormal[3] ;
in mediump vec2 g_vTextureCoord[3] ;
in mediump vec4 g_vVertexMulColor[3] ;
in mediump vec4 g_vVertexAddColor[3] ;
in mediump float g_fpVertexAlpha[3] ;
#ifdef _ENABLE_EXTEND_ATTR_
in highp vec4 g_vExAttrElement0[3] ;
#endif
out highp vec4 v_vPosition ;
out mediump vec3 v_vNormal ;
out mediump vec3 v_vTextureAxisX ;
out mediump vec3 v_vTextureAxisY ;
out mediump vec2 v_vTextureCoord ;
out mediump vec4 v_vVertexMulColor ;
out mediump vec4 v_vVertexAddColor ;
#ifdef _ENABLE_EXTEND_ATTR_
out highp vec4 v_vExAttrElement0 ;
#endif
void main(void)
{
vec3 v1 = g_vPosition[1].xyz - g_vPosition[0].xyz ;
vec3 v2 = g_vPosition[2].xyz - g_vPosition[0].xyz ;
vec2 uv1 = g_vTextureCoord[1] - g_vTextureCoord[0] ;
vec2 uv2 = g_vTextureCoord[2] - g_vTextureCoord[0] ;
float d = uv1.x * uv2.y - uv2.x * uv1.y ;
vec3 vx = vec3( 0.0, 0.0, 0.0 ) ;
vec3 vy = vec3( 0.0, 0.0, 0.0 ) ;
if ( d != 0.0 )
{
d = 1.0 / d ;
vx = (v1 * uv2.y - v2 * uv1.y) * d ;
vy = - (v1 * uv2.x - v2 * uv1.x) * d ;
}
for ( int i = 0; i < 3; i ++ )
{
gl_Position = gl_in[i].gl_Position ;
v_vPosition = g_vPosition[i] ;
v_vNormal = g_vNormal[i] ;
v_vTextureAxisX = vx ;
v_vTextureAxisY = vy ;
v_vTextureCoord = g_vTextureCoord[i] ;
v_vVertexMulColor = g_vVertexMulColor[i] ;
v_vVertexAddColor = g_vVertexAddColor[i] ;
#ifdef _ENABLE_EXTEND_ATTR_
v_vExAttrElement0 = g_vExAttrElement0[i] ;
#endif
EmitVertex() ;
}
gl_Position = gl_in[0].gl_Position ;
v_vPosition = g_vPosition[0] ;
v_vNormal = g_vNormal[0] ;
v_vTextureAxisX = vx ;
v_vTextureAxisY = vy ;
v_vTextureCoord = g_vTextureCoord[0] ;
v_vVertexMulColor = g_vVertexMulColor[0] ;
v_vVertexAddColor = g_vVertexAddColor[0] ;
#ifdef _ENABLE_EXTEND_ATTR_
v_vExAttrElement0 = g_vExAttrElement0[0] ;
#endif
EmitVertex() ;
EndPrimitive() ;
}
