
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneParameter.h>



// uint colorFromVector( const Vector3d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_colorFromVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	LQT_RETURN_UINT( S3DSceneComposer::Parameter::ColorFromVector(*vec).ui32 ) ;
}

// const Vector3d* vectorFromColor( uint rgb )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_vectorFromColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_UINT( rgb ) ;

	LVector3d	valRet = S3DSceneComposer::Parameter::VectorFromColor( SGLPalette(rgb) ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Matrix3d* getMatrixParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getMatrixParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LMatrix3d	valRet = pParam->GetMatrixParameter( (size_t) iParam ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* getVectorParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVectorParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector3d	valRet = pParam->GetVectorParameter( (size_t) iParam ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// double getScalarParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getScalarParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_DOUBLE( pParam->GetScalarParameter( (size_t) iParam ) ) ;
}

// int getIntegerParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getIntegerParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_INT( pParam->GetIntegerParameter( (size_t) iParam ) ) ;
}

// boolean getBooleanParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getBooleanParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_BOOL( pParam->GetBooleanParameter( (size_t) iParam ) ) ;
}

// String getCommandParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getCommandParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_STRING( pParam->GetCommandParameter( (size_t) iParam ) ) ;
}

// const Vector2d* getVector2dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVector2dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector2d	valRet ;
	pParam->GetBinaryParameter( &valRet, sizeof(valRet), (size_t) iParam ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector4d* getVector4dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVector4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector4d	valRet ;
	pParam->GetBinaryParameter( &valRet, sizeof(valRet), (size_t) iParam ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Matrix4d* getMatrix4dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getMatrix4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LMatrix4d	valRet ;
	pParam->GetBinaryParameter( &valRet, sizeof(valRet), (size_t) iParam ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getBinaryParameter( void* pDst, ulong nBufBytes, ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getBinaryParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_POINTER_N( uint8_t, pDst, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pDst ) ;
	LQT_FUNC_ARG_ULONG( nBufBytes ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_ULONG
		( pParam->GetBinaryParameter( pDst, (size_t) nBufBytes, (size_t) iParam ) ) ;
}

// void setMatrixParameter( ulong iParam, const Matrix3d* mat )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setMatrixParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;

	pParam->SetMatrixParameter( (size_t) iParam, *mat ) ;

	LQT_RETURN_VOID() ;
}

// void setVectorParameter( ulong iParam, const Vector3d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVectorParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	pParam->SetVectorParameter( (size_t) iParam, *vec ) ;

	LQT_RETURN_VOID() ;
}

// void setScalarParameter( ulong iParam, double s )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setScalarParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_DOUBLE( s ) ;

	pParam->SetScalarParameter( (size_t) iParam, s ) ;

	LQT_RETURN_VOID() ;
}

// void setIntegerParameter( ulong iParam, int n )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setIntegerParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_INT( n ) ;

	pParam->SetIntegerParameter( (size_t) iParam, n ) ;

	LQT_RETURN_VOID() ;
}

// void setBooleanParameter( ulong iParam, boolean b )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setBooleanParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_BOOL( b ) ;

	pParam->SetBooleanParameter( (size_t) iParam, b ) ;

	LQT_RETURN_VOID() ;
}

// void setCommandParameter( ulong iParam, String cmd )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setCommandParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;

	pParam->SetCommandParameter( (size_t) iParam, cmd.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void setVector2dParameter( ulong iParam, const Vector2d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVector2dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	pParam->SetBinaryParameter( (size_t) iParam, vec, sizeof(LVector2d) ) ;

	LQT_RETURN_VOID() ;
}

// void setVector4dParameter( ulong iParam, const Vector4d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVector4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector4d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	pParam->SetBinaryParameter( (size_t) iParam, vec, sizeof(LVector4d) ) ;

	LQT_RETURN_VOID() ;
}

// void setMatrix4dParameter( ulong iParam, const Matrix4d* mat )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setMatrix4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;

	pParam->SetBinaryParameter( (size_t) iParam, mat, sizeof(LMatrix4d) ) ;

	LQT_RETURN_VOID() ;
}

// ulong setBinaryParameter( ulong iParam, const void* pSrc, ulong nBufBytes )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setBinaryParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_POINTER_N( uint8_t, pSrc, LQT_ARG_LONG(3) ) ;
	LQT_VERIFY_NULL_PTR( pSrc ) ;
	LQT_FUNC_ARG_ULONG( nBufBytes ) ;

	LQT_RETURN_ULONG
		( pParam->SetBinaryParameter( (size_t) iParam, pSrc, (size_t) nBufBytes ) ) ;
}

// boolean enumerateStringSet( ulong iParam, String[] strSet )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_enumerateStringSet)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	S3DSceneComposer::Parameter *	pParam = pThis->GetRef<S3DSceneComposer::Parameter>() ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, strSet ) ;
	LQT_VERIFY_NULL_PTR( strSet ) ;

	SStringArray	aStrSet ;
	LBoolean	valRet = pParam->EnumerateStringSet( (size_t) iParam, aStrSet ) ;

	strSet->RemoveAll() ;
	for ( size_t i = 0; i < aStrSet.GetLength(); i ++ )
	{
		LObject::ReleaseRef
			( strSet->SetElementAt( i, _context.new_String( aStrSet.At(i) ) ) ) ;
	}

	LQT_RETURN_BOOL( valRet ) ;
}



