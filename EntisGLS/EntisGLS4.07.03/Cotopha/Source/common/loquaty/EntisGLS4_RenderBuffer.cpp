
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RenderBuffer.h>


// void setShadingFlag( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setShadingFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	pRender->SetShadingFlag( flags ) ;

	LQT_RETURN_VOID() ;
}

// ulong getShadingFlag( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getShadingFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_ULONG( pRender->GetShadingFlag() ) ;
}

// boolean appendMatrixTransformation( const Matrix3d* mat, const Vector3d* pos, const EntisGLS4.ColorMulAdd* cmadd, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_appendMatrixTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, cmadd ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LQT_RETURN_BOOL
		( pRender->AppendMatrixTransformation
			( *mat, *pos, cmadd, transparency ) == sglErrSuccess ) ;
}

// boolean setMatrixTransformation( const Matrix3d* mat, const Vector3d* pos, const EntisGLS4.ColorMulAdd* cmadd, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setMatrixTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, cmadd ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LQT_RETURN_BOOL
		( pRender->SetMatrixTransformation
			( *mat, *pos, cmadd, transparency ) == sglErrSuccess ) ;
}

// boolean pushTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_pushTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_BOOL( pRender->PushTransformation() == sglErrSuccess ) ;
}

// boolean popTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_popTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_BOOL( pRender->PopTransformation() == sglErrSuccess ) ;
}

// boolean resetTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_resetTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_BOOL( pRender->ResetTransformation() == sglErrSuccess ) ;
}

// boolean attachCustomShader( EntisGLS4.CustomShader shader )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_attachCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_CustomShader, shader ) ;
	S3DCustomShader *	pShader = nullptr ;
	if ( shader != nullptr )
	{
		pShader = shader->GetRef<S3DCustomShader>() ;
	}

	LQT_RETURN_BOOL( pRender->AttachCustomShader( pShader ) == sglErrSuccess ) ;
}

// EntisGLS4.CustomShader getCustomShader( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getCustomShader)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	S3DCustomShader *	pShader = pRender->GetCustomShader() ;
	if ( pShader == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.CustomShader) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_CustomShader>(pShader) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean setCustomShaderUniform( String id, int type, const void* data, uint count )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setCustomShaderUniform)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_POINTER_N
		( uint8_t, data,
			S3DCustomShader::UniformData::GetDataBytes
				( (S3DCustomShader::UniformType) type, (size_t) LQT_ARG_LONG(4) ) ) ;
	LQT_VERIFY_NULL_PTR( data ) ;
	LQT_FUNC_ARG_UINT( count ) ;

	LQT_RETURN_BOOL
		( pRender->SetCustomShaderUniform
			( id.c_str(), (S3DCustomShader::UniformType) type, data, count ) == sglErrSuccess ) ;
}

// boolean setCustomShaderUniformImage( String id, int type, EntisGLS4.Image image )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setCustomShaderUniformImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_BOOL
		( pRender->SetCustomShaderUniform
			( id.c_str(), (S3DCustomShader::UniformType) type, &pImage, 1 ) == sglErrSuccess ) ;
}

// boolean resetCustomShaderUniform( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_resetCustomShaderUniform)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_BOOL( pRender->ResetCustomShaderUniform() == sglErrSuccess ) ;
}

// boolean addIndexedPrimitive( EntisGLS4.Material material, uint flags, int typePrimitive, uint countIndex, uint countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor, const uint* pIndexedList )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_addIndexedPrimitive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	S3DMaterial *	pMaterial = nullptr ;
	if ( material != nullptr )
	{
		pMaterial = material->GetRef<S3DMaterial>() ;
	}
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_UINT( countIndex ) ;
	LQT_FUNC_ARG_UINT( countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvVertex, countVertex ) ;
	LQT_VERIFY_NULL_PTR( pvVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvNormal, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pvUVMap, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pColor, countVertex ) ;
	LQT_FUNC_ARG_POINTER_N( LUint32, pIndexedList, countIndex  ) ;

	LQT_RETURN_BOOL
		( pRender->AddIndexedPrimitiveList
			( pMaterial, flags, (S3DPrimitiveType) typePrimitive,
				countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) == sglErrSuccess  ) ;
}

// boolean addVertexBuffer( uint flags, EntisGLS4.VertexBuffer vb, uint iFirst, int iEnd, uint nInstancing, const Matrix4* pmatInstancing, const EntisGLS4.ColorMulAdd* pColorInstancing )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_addVertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, vb ) ;
	LQT_VERIFY_NULL_PTR( vb ) ;
	S3DVertexBufferInterface *	pVB = vb->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_UINT( iFirst ) ;
	LQT_FUNC_ARG_INT( iEnd ) ;
	LQT_FUNC_ARG_UINT( nInstancing ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix4, pmatInstancing, nInstancing ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pColorInstancing, nInstancing ) ;

	LQT_RETURN_BOOL
		( pRender->AddVertexBuffer
			( nullptr, flags, pVB, iFirst, iEnd,
				nInstancing, pmatInstancing, pColorInstancing ) == sglErrSuccess ) ;
}

// boolean flush( )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_flush)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;

	LQT_RETURN_BOOL( pRender->Flush() == sglErrSuccess ) ;
}

// boolean setOptionalFeature( int feature, int param1, const void* param2, ulong sizeOfParam2, Object param3 )
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_setOptionalFeature)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_INT( feature ) ;
	LQT_FUNC_ARG_INT( param1 ) ;
	LQT_FUNC_ARG_POINTER_N( uint8_t, param2, LQT_ARG_LONG(4) ) ;
	LQT_FUNC_ARG_ULONG( sizeOfParam2 ) ;
	LQT_FUNC_ARG_OBJECT( LObject, param3 ) ;

	S3DRenderContextInterface::EnvMappingParam	envMapParam ;
	if ( param3 != nullptr )
	{
		if ( feature == S3DRenderBufferInterface::featureEnvMap )
		{
			std::shared_ptr<LEntisGLS4_Image>
				image = param3->GetElementNativeAs<LEntisGLS4_Image>( L"image" ) ;
			envMapParam.pImage =
				(image != nullptr)
					? image->GetRef<SGLImageObject>() : nullptr ;
			envMapParam.typeMap = (uint32_t) param3->GetElementLongAs( L"typeMap" ) ;
			//
			LMatrix3 *	pMat3 =
				(LMatrix3*) param3->GetElementPointerAs( L"matMap", sizeof(LMatrix3) ) ;
			if ( pMat3 != nullptr )
			{
				envMapParam.matMap = pMat3->ToS3DMatrix() ;
			}
			param2 = (uint8_t*) &envMapParam ;
			sizeOfParam2 = sizeof(envMapParam) ;
		}
	}

	LQT_RETURN_BOOL
		( pRender->SetOptionalFeature
			( (S3DRenderBufferInterface::FeatureType) feature,
				param1, param2, (size_t) sizeOfParam2 ) == sglErrSuccess ) ;
}

// boolean getOptionalFeature( int feature, int param1, void* param2, ulong sizeOfParam2, Object param3 ) const
IMPL_LOQUATY_FUNC(EntisGLS4_RenderBuffer_getOptionalFeature)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_RenderBuffer, pThis ) ;
	S3DRenderBufferInterface *	pRender = pThis->GetRef<S3DRenderBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_INT( feature ) ;
	LQT_FUNC_ARG_INT( param1 ) ;
	LQT_FUNC_ARG_POINTER_N( uint8_t, param2, LQT_ARG_LONG(4) ) ;
	LQT_FUNC_ARG_ULONG( sizeOfParam2 ) ;
	LQT_FUNC_ARG_OBJECT( LObject, param3 ) ;

	S3DRenderContextInterface::EnvMappingParam	envMapParam ;
	if ( param3 != nullptr )
	{
		if ( feature == S3DRenderBufferInterface::featureEnvMap )
		{
			param2 = (uint8_t*) &envMapParam ;
			sizeOfParam2 = sizeof(envMapParam) ;
		}
	}

	LBoolean	valRet =
		(pRender->GetOptionalFeature
			( (S3DRenderBufferInterface::FeatureType) feature,
				param1, param2, (size_t) sizeOfParam2 ) == sglErrSuccess) ;
	if ( valRet && (param3 != nullptr) )
	{
		if ( feature == S3DRenderBufferInterface::featureEnvMap )
		{
			if ( envMapParam.pImage != nullptr )
			{
				LPtr<LNativeObj>	pImage( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
				pImage->SetNative
					( std::make_shared<LEntisGLS4_Image>( envMapParam.pImage ) ) ;
				LObject::ReleaseRef( param3->SetElementAs( L"image", pImage.Get() ) ) ;
			}
			else
			{
				LObject::ReleaseRef( param3->SetElementAs( L"image", nullptr ) ) ;
			}
			param3->SetElementLongAs( L"typeMap", envMapParam.typeMap ) ;

			LMatrix3 *	pMat3 =
				(LMatrix3*) param3->GetElementPointerAs( L"matMap", sizeof(LMatrix3) ) ;
			if ( pMat3 != nullptr )
			{
				*pMat3 = LMatrix3( envMapParam.matMap ) ;
			}
		}
	}

	LQT_RETURN_BOOL( valRet ) ;
}



