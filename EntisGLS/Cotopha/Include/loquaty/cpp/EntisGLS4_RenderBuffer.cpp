
#include <loquaty.h>
#include "EntisGLS4_RenderBuffer.h"

using namespace Loquaty ;


// void setShadingFlag( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setShadingFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	// pThis->setShadingFlag(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getShadingFlag( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getShadingFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getShadingFlag(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean appendMatrixTransformation( const Matrix3d* mat, const Vector3d* pos, const EntisGLS4.ColorMulAdd* cmadd, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_appendMatrixTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, cmadd ) ;
	LQT_VERIFY_NULL_PTR( cmadd ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LBoolean	valRet ;
	// valRet = pThis->appendMatrixTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMatrixTransformation( const Matrix3d* mat, const Vector3d* pos, const EntisGLS4.ColorMulAdd* cmadd, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setMatrixTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, cmadd ) ;
	LQT_VERIFY_NULL_PTR( cmadd ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMatrixTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean pushTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_pushTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->pushTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean popTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_popTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->popTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean resetTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_resetTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->resetTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean attachCustomShader( EntisGLS4.CustomShader shader )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_attachCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_CustomShader, shader ) ;
	LQT_VERIFY_NULL_PTR( shader ) ;

	LBoolean	valRet ;
	// valRet = pThis->attachCustomShader(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.CustomShader getCustomShader( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	// valRet = pThis->getCustomShader(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_CustomShader> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean setCustomShaderUniform( String id, EntisGLS4.CustomShader.UniformType type, const void* data, uint count )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setCustomShaderUniform)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_POINTER( void, data ) ;
	LQT_VERIFY_NULL_PTR( data ) ;
	LQT_FUNC_ARG_UINT( count ) ;

	LBoolean	valRet ;
	// valRet = pThis->setCustomShaderUniform(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setCustomShaderUniformImage( String id, int type, EntisGLS4.Image image )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setCustomShaderUniformImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;

	LBoolean	valRet ;
	// valRet = pThis->setCustomShaderUniformImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean resetCustomShaderUniform( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_resetCustomShaderUniform)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->resetCustomShaderUniform(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addIndexedPrimitive( EntisGLS4.Material material, EntisGLS4.RenderBuffer.AddRenderFlag flags, EntisGLS4.PrimitiveType typePrimitive, uint countIndex, uint countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor, const uint* pIndexedList )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_addIndexedPrimitive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_UINT( countIndex ) ;
	LQT_FUNC_ARG_UINT( countVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvVertex ) ;
	LQT_VERIFY_NULL_PTR( pvVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvNormal ) ;
	LQT_VERIFY_NULL_PTR( pvNormal ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pvUVMap ) ;
	LQT_VERIFY_NULL_PTR( pvUVMap ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pColor ) ;
	LQT_VERIFY_NULL_PTR( pColor ) ;
	LQT_FUNC_ARG_POINTER( LUint32, pIndexedList ) ;
	LQT_VERIFY_NULL_PTR( pIndexedList ) ;

	LBoolean	valRet ;
	// valRet = pThis->addIndexedPrimitive(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addVertexBuffer( EntisGLS4.RenderBuffer.AddRenderFlag flags, EntisGLS4.VertexBuffer vb, uint iFirst, int iEnd, uint nInstancing, const Matrix4* pmatInstancing, const EntisGLS4.ColorMulAdd* pColorInstancing )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_addVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	LQT_FUNC_ARG_UINT( iFirst ) ;
	LQT_FUNC_ARG_INT( iEnd ) ;
	LQT_FUNC_ARG_UINT( nInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, pmatInstancing ) ;
	LQT_VERIFY_NULL_PTR( pmatInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pColorInstancing ) ;
	LQT_VERIFY_NULL_PTR( pColorInstancing ) ;

	LBoolean	valRet ;
	// valRet = pThis->addVertexBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean flush( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_flush)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->flush(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setOptionalFeature( EntisGLS4.RenderBuffer.FeatureType feature, int param1, const void* param2, ulong sizeOfParam2, Object param3 )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setOptionalFeature)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_INT( feature ) ;
	LQT_FUNC_ARG_INT( param1 ) ;
	LQT_FUNC_ARG_POINTER( void, param2 ) ;
	LQT_VERIFY_NULL_PTR( param2 ) ;
	LQT_FUNC_ARG_ULONG( sizeOfParam2 ) ;
	LQT_FUNC_ARG_OBJECT( LObject, param3 ) ;
	LQT_VERIFY_NULL_PTR( param3 ) ;

	LBoolean	valRet ;
	// valRet = pThis->setOptionalFeature(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getOptionalFeature( EntisGLS4.RenderBuffer.FeatureType feature, int param1, void* param2, ulong sizeOfParam2, Object param3 ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getOptionalFeature)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	LQT_FUNC_ARG_INT( feature ) ;
	LQT_FUNC_ARG_INT( param1 ) ;
	LQT_FUNC_ARG_POINTER( void, param2 ) ;
	LQT_VERIFY_NULL_PTR( param2 ) ;
	LQT_FUNC_ARG_ULONG( sizeOfParam2 ) ;
	LQT_FUNC_ARG_OBJECT( LObject, param3 ) ;
	LQT_VERIFY_NULL_PTR( param3 ) ;

	LBoolean	valRet ;
	// valRet = pThis->getOptionalFeature(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



