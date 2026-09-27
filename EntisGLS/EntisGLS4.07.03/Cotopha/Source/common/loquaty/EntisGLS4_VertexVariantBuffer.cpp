
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_VertexVariantBuffer.h>


// boolean setBoneMatrix( ulong iMesh, ulong nCount, const Matrix3* pMatrix, const Vector3* pTrans )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setBoneMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix3, pMatrix3, nCount ) ;
	LQT_VERIFY_NULL_PTR( pMatrix3 ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector3, pTrans, nCount ) ;
	LQT_VERIFY_NULL_PTR( pTrans ) ;

	SArray<S3DMatrix>	bufMatrix ;
	S3DMatrix *	pMatrix = bufMatrix.GetArray( (size_t) nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pMatrix[i] = pMatrix3[i].ToS3DMatrix() ;
	}

	LQT_RETURN_BOOL
		( pVVB->SetBoneMatrix
			( (size_t) iMesh, (size_t) nCount, pMatrix, pTrans ) == sglErrSuccess ) ;
}

// ulong getBoneMatrix( ulong iMesh, ulong nCount, Matrix3* pMatrix, Vector3* pTrans )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getBoneMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix3, pMatrix3, nCount ) ;
	LQT_VERIFY_NULL_PTR( pMatrix3 ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector3, pTrans, nCount ) ;
	LQT_VERIFY_NULL_PTR( pTrans ) ;

	SArray<S3DMatrix>	bufMatrix ;
	S3DMatrix *	pMatrix = bufMatrix.GetArray( (size_t) nCount ) ;

	LUint64	valRet =
		pVVB->GetBoneMatrix( (size_t) iMesh, (size_t) nCount, pMatrix, pTrans ) ;
	for ( size_t i = 0; i < valRet; i ++ )
	{
		pMatrix3[i] = LMatrix3( pMatrix[i] ) ;
	}
	LQT_RETURN_ULONG( valRet ) ;
}

// boolean setMorphingApplication( ulong iMesh, const long* pTargetMesh, const float* pApplication, ulong nTargetMeshCount )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setMorphingApplication)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_POINTER_N( LInt64, pTargetMesh, LQT_ARG_LONG(4) ) ;
	LQT_VERIFY_NULL_PTR( pTargetMesh ) ;
	LQT_FUNC_ARG_POINTER_N( LFloat, pApplication, LQT_ARG_LONG(4) ) ;
	LQT_VERIFY_NULL_PTR( pApplication ) ;
	LQT_FUNC_ARG_ULONG( nTargetMeshCount ) ;

	SArray<ssize_t>	bufTargetMesh ;
	ssize_t *	pTargetMeshBuf = bufTargetMesh.GetArray( (size_t) nTargetMeshCount ) ;
	for ( size_t i = 0; i < nTargetMeshCount; i ++ )
	{
		pTargetMeshBuf[i] = (ssize_t) pTargetMesh[i] ;
	}
	LQT_RETURN_BOOL
		( pVVB->SetMorphingApplication
			( (size_t) iMesh, pTargetMeshBuf,
				pApplication, (size_t) nTargetMeshCount ) == sglErrSuccess ) ;
}

// boolean getMorphingApplication( ulong iMesh, long* pTargetMesh, float* pApplication, ulong iTargetMeshIndex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getMorphingApplication)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pTargetMesh ) ;
	LQT_VERIFY_NULL_PTR( pTargetMesh ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pApplication ) ;
	LQT_VERIFY_NULL_PTR( pApplication ) ;
	LQT_FUNC_ARG_ULONG( iTargetMeshIndex ) ;

	ssize_t	iTargetMesh ;
	LBoolean	valRet =
		(pVVB->GetMorphingApplication
			( (size_t) iMesh, iTargetMesh,
				*pApplication, (size_t) iTargetMeshIndex ) == sglErrSuccess) ;
	*pTargetMesh = iTargetMesh ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean enableToRenderMesh( ulong iFirst, long iEnd, boolean fEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_enableToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;
	LQT_FUNC_ARG_BOOL( fEnable ) ;

	LQT_RETURN_BOOL
		( pVVB->EnableToRenderMesh
			( (size_t) iFirst, (ssize_t) iEnd, fEnable ) == sglErrSuccess ) ;
}

// boolean isEnabledToRenderMesh( ulong iMesh ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_isEnabledToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;

	LQT_RETURN_BOOL( pVVB->IsEnabledToRenderMesh( (size_t) iMesh ) ) ;
}

// boolean setMaterialToRenderMesh( ulong iMesh, EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setMaterialToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	S3DMaterial *	pMaterial = material->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_BOOL
		( pVVB->SetMaterialToRenderMesh
			( (size_t) iMesh, pMaterial ) == sglErrSuccess ) ;
}

// EntisGLS4.Material getMaterialToRenderMesh( ulong iMesh ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getMaterialToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	S3DVertexVariantBuffer *	pVVB = pThis->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;

	S3DMaterial *	pMaterial = pVVB->GetMaterialToRenderMesh( (size_t) iMesh ) ;
	if ( pMaterial == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>(pMaterial) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



