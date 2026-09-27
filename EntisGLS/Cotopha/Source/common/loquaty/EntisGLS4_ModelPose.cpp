
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ModelPose.h>


// ModelPose( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelPose)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_ModelPose, pThis,
			( new SSmartObject( new S3DModelPose ) ) ) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.ModelPose.MetaInfo* getMetaInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_getMetaInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	S3DModelPose *	pPose = pThis->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;

	LEntisGLS4_ModelPose_MetaInfo	valRet = pPose->GetMetaInfo() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void resetPoseTarget( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_resetPoseTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	S3DModelPose *	pPose = pThis->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;

	pPose->ResetPoseTarget() ;

	LQT_RETURN_VOID() ;
}

// void applyPoseTo( EntisGLS4.ModelBuffer model, double w, double t )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_applyPoseTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	S3DModelPose *	pPose = pThis->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;
	S3DModelBuffer *	pModel = model->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_DOUBLE( w ) ;
	LQT_FUNC_ARG_DOUBLE( t ) ;

	pPose->ApplyPoseTo( *pModel, w, t ) ;

	LQT_RETURN_VOID() ;
}

// void productPoseTo( EntisGLS4.ModelBuffer model, double w, double t )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_productPoseTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	S3DModelPose *	pPose = pThis->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;
	S3DModelBuffer *	pModel = model->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_DOUBLE( w ) ;
	LQT_FUNC_ARG_DOUBLE( t ) ;

	pPose->ProductPoseTo( *pModel, w, t ) ;

	LQT_RETURN_VOID() ;
}



