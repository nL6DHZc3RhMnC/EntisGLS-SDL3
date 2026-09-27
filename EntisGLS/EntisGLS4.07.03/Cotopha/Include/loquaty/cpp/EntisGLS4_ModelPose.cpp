
#include <loquaty.h>
#include "EntisGLS4_ModelPose.h"

using namespace Loquaty ;


// ModelPose( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelPose)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ModelPose, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.ModelPose.MetaInfo* getMetaInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_getMetaInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;

	LEntisGLS4_ModelPose_MetaInfo	valRet ;
	// valRet = pThis->getMetaInfo(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void resetPoseTarget( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_resetPoseTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;

	// pThis->resetPoseTarget(...) ;

	LQT_RETURN_VOID() ;
}

// void applyPoseTo( EntisGLS4.ModelBuffer model, double w, double t )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_applyPoseTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;
	LQT_FUNC_ARG_DOUBLE( w ) ;
	LQT_FUNC_ARG_DOUBLE( t ) ;

	// pThis->applyPoseTo(...) ;

	LQT_RETURN_VOID() ;
}

// void productPoseTo( EntisGLS4.ModelBuffer model, double w, double t )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPose_productPoseTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPose, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;
	LQT_FUNC_ARG_DOUBLE( w ) ;
	LQT_FUNC_ARG_DOUBLE( t ) ;

	// pThis->productPoseTo(...) ;

	LQT_RETURN_VOID() ;
}



