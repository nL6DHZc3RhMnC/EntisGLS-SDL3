
#include <loquaty.h>
#include "EntisGLS4_SceneDynamicModel.h"

using namespace Loquaty ;


// SceneDynamicModel( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneDynamicModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneDynamicModel, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void attachModelRef( EntisGLS4.ModelBuffer model )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_attachModelRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;

	// pThis->attachModelRef(...) ;

	LQT_RETURN_VOID() ;
}

// void setModelID( String modelId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setModelID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_STRING( modelId ) ;

	// pThis->setModelID(...) ;

	LQT_RETURN_VOID() ;
}

// String getModelID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getModelID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getModelID(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void attachCollisionModel( EntisGLS4.VertexBuffer model )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_attachCollisionModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, model ) ;
	LQT_VERIFY_NULL_PTR( model ) ;

	// pThis->attachCollisionModel(...) ;

	LQT_RETURN_VOID() ;
}

// void setCollisionID( String modelId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setCollisionID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_STRING( modelId ) ;

	// pThis->setCollisionID(...) ;

	LQT_RETURN_VOID() ;
}

// String getCollisionID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getCollisionID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getCollisionID(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// boolean isDynamicCollision( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_isDynamicCollision)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isDynamicCollision(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setDynamicCollisionFlag( boolean flagDynamic )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setDynamicCollisionFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagDynamic ) ;

	// pThis->setDynamicCollisionFlag(...) ;

	LQT_RETURN_VOID() ;
}

// void setVariantDrawTaregt( String targetItemId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setVariantDrawTaregt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	LQT_FUNC_ARG_STRING( targetItemId ) ;

	// pThis->setVariantDrawTaregt(...) ;

	LQT_RETURN_VOID() ;
}

// String getVariantDrawTarget( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getVariantDrawTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getVariantDrawTarget(...) ;

	LQT_RETURN_STRING( valRet ) ;
}



