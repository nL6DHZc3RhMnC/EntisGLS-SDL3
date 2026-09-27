
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ModelBone.h>


// void calcBoneTransformation( Matrix3d* matrix, Vector3d* translate ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, translate ) ;
	LQT_VERIFY_NULL_PTR( translate ) ;

	pBone->CalcBoneTransformation( *matrix, *translate ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* calcBoneBasePosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneBasePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;

	LVector3d	valRet ;
	pBone->CalcBoneBasePosition( valRet ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* calcBoneNormalizedPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcBoneNormalizedPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;

	LVector3d	valRet = pBone->CalcBoneNormalizedPosition() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void calcGlobalTransformation( Matrix3d* matrix, Vector3d* pos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcGlobalTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;

	pBone->CalcGlobalTransformation( *matrix, *pos ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* calcGlobalPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_calcGlobalPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;

	LVector3d	valRet ;
	pBone->CalcGlobalPosition( valRet ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.ModelBone getParentBone( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getParentBone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;

	S3DModelBoneSpace *	pParent = pThisBone->GetParentBone() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBone) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelBone>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelBone.BoneFlag getBoneFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;

	LQT_RETURN_UINT( pThisBone->GetBoneFlags() ) ;
}

// void setTransformation( const Matrix3d* matBone )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_setTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matBone ) ;
	LQT_VERIFY_NULL_PTR( matBone ) ;

	pThisBone->SetLocalTransformation( *matBone ) ;

	LQT_RETURN_VOID() ;
}

// void applyTransformation( const Matrix3d* matModel, const Vector3d* vModelPos )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_applyTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matModel ) ;
	LQT_VERIFY_NULL_PTR( matModel ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vModelPos ) ;
	LQT_VERIFY_NULL_PTR( vModelPos ) ;

	pThisBone->ApplyGlobalTransformation( *matModel, *vModelPos ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBoneOffset( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneOffset)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;

	LVector3d	valRet = pThisBone->GetBoneOffset() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBoneOffset( const Vector3d* vOffset )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_setBoneOffset)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vOffset ) ;
	LQT_VERIFY_NULL_PTR( vOffset ) ;

	pThisBone->SetBoneOffset( *vOffset ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBoneHandle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_getBoneHandle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;

	LVector3d	valRet = pThisBone->GetBoneHandle() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void operateInverseKinematics( const Vector3d* vPos, const Matrix3d* matRotate, const Vector3d* vLocalTip, double fpBendingWeight, double zGimbalWeight, double fpTipBoneWeight, ulong nEffectParents )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBone_operateInverseKinematics)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBone, pThis ) ;
	S3DModelBoneSpace *	pThisBone = pThis->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pThisBone ) ;
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

	pThisBone->OperateInverseKinematics
		( *vPos, *matRotate, *vLocalTip,
			fpBendingWeight, zGimbalWeight,
			fpTipBoneWeight, (size_t) nEffectParents ) ;

	LQT_RETURN_VOID() ;
}



