void main( void )
{
morph_vertex() ;
transform_vertex
( v_vPosition, v_vNormal, v_vPosition.xyz, v_vNormal ) ;
gl_Position = u_mat4PerspectiveView
* (u_mat4ModelView
* vec4( v_vTextureCoord.x, v_vTextureCoord.y, 0.0, 1.0 )) ;
utilDefaultVertexGouraud() ;
}
