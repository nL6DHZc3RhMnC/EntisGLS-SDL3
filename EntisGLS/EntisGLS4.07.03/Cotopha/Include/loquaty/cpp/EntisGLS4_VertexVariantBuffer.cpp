
#include <loquaty.h>
#include "EntisGLS4_VertexVariantBuffer.h"

using namespace Loquaty ;


// boolean setBoneMatrix( ulong iMesh, ulong nCount, const Matrix3* pMatrix, const Vector3* pTrans )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setBoneMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3, pMatrix ) ;
	LQT_VERIFY_NULL_PTR( pMatrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, pTrans ) ;
	LQT_VERIFY_NULL_PTR( pTrans ) ;

	LBoolean	valRet ;
	// valRet = pThis->setBoneMatrix(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getBoneMatrix( ulong iMesh, ulong nCount, Matrix3* pMatrix, Vector3* pTrans )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getBoneMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3, pMatrix ) ;
	LQT_VERIFY_NULL_PTR( pMatrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, pTrans ) ;
	LQT_VERIFY_NULL_PTR( pTrans ) ;

	LUint64	valRet ;
	// valRet = pThis->getBoneMatrix(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean setMorphingApplication( ulong iMesh, const long* pTargetMesh, const float* pApplication, ulong nTargetMeshCount )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setMorphingApplication)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pTargetMesh ) ;
	LQT_VERIFY_NULL_PTR( pTargetMesh ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pApplication ) ;
	LQT_VERIFY_NULL_PTR( pApplication ) ;
	LQT_FUNC_ARG_ULONG( nTargetMeshCount ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMorphingApplication(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getMorphingApplication( ulong iMesh, long* pTargetMesh, float* pApplication, ulong iTargetMeshIndex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getMorphingApplication)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pTargetMesh ) ;
	LQT_VERIFY_NULL_PTR( pTargetMesh ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pApplication ) ;
	LQT_VERIFY_NULL_PTR( pApplication ) ;
	LQT_FUNC_ARG_ULONG( iTargetMeshIndex ) ;

	LBoolean	valRet ;
	// valRet = pThis->getMorphingApplication(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean enableToRenderMesh( ulong iFirst, long iEnd, boolean fEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_enableToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;
	LQT_FUNC_ARG_BOOL( fEnable ) ;

	LBoolean	valRet ;
	// valRet = pThis->enableToRenderMesh(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isEnabledToRenderMesh( ulong iMesh ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_isEnabledToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEnabledToRenderMesh(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMaterialToRenderMesh( ulong iMesh, EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_setMaterialToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMaterialToRenderMesh(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Material getMaterialToRenderMesh( ulong iMesh ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexVariantBuffer_getMaterialToRenderMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexVariantBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	// valRet = pThis->getMaterialToRenderMesh(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Material> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



