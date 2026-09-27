
#include <loquaty.h>
#include "EntisGLS4_ModelBone.h"

using namespace Loquaty ;


// void calcBoneTransformation( Matrix3d* matrix, Vector3d* translate ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, translate ) ;
	LQT_VERIFY_NULL_PTR( translate ) ;

	// pThis->calcBoneTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* calcBoneBasePosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneBasePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->calcBoneBasePosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* calcBoneNormalizedPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneNormalizedPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->calcBoneNormalizedPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void calcGlobalTransformation( Matrix3d* matrix, Vector3d* pos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcGlobalTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;

	// pThis->calcGlobalTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* calcGlobalPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcGlobalPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->calcGlobalPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.ModelBone getParentBone( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getParentBone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBone) ) ) ;
	// valRet = pThis->getParentBone(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelBone> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelBone>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelBone.BoneFlag getBoneFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getBoneFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setTransformation( const Matrix3d* matBone )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_setTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matBone ) ;
	LQT_VERIFY_NULL_PTR( matBone ) ;

	// pThis->setTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// void applyTransformation( const Matrix3d* matModel, const Vector3d* vModelPos )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_applyTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matModel ) ;
	LQT_VERIFY_NULL_PTR( matModel ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vModelPos ) ;
	LQT_VERIFY_NULL_PTR( vModelPos ) ;

	// pThis->applyTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBoneOffset( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneOffset)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getBoneOffset(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBoneOffset( const Vector3d* vOffset )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_setBoneOffset)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vOffset ) ;
	LQT_VERIFY_NULL_PTR( vOffset ) ;

	// pThis->setBoneOffset(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBoneHandle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneHandle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getBoneHandle(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void operateInverseKinematics( const Vector3d* vPos, const Matrix3d* matRotate, const Vector3d* vLocalTip, double fpBendingWeight, double zGimbalWeight, double fpTipBoneWeight, ulong nEffectParents )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_operateInverseKinematics)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matRotate ) ;
	LQT_VERIFY_NULL_PTR( matRotate ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vLocalTip ) ;
	LQT_VERIFY_NULL_PTR( vLocalTip ) ;
	LQT_FUNC_ARG_DOUBLE( fpBendingWeight ) ;
	LQT_FUNC_ARG_DOUBLE( zGimbalWeight ) ;
	LQT_FUNC_ARG_DOUBLE( fpTipBoneWeight ) ;
	LQT_FUNC_ARG_ULONG( nEffectParents ) ;

	// pThis->operateInverseKinematics(...) ;

	LQT_RETURN_VOID() ;
}



