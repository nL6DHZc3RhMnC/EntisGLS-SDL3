
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneDynamicModel.h>
#include <sakuraglx/render/sglx3d_scene_item.h>


// SceneDynamicModel( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneDynamicModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneDynamicModel, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
						new S3DDynamicModelSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void attachModelRef( EntisGLS4.ModelBuffer model )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_attachModelRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBuffer, model ) ;
	S3DModelBuffer *	pModel = nullptr ;
	if ( model != nullptr )
	{
		pModel = model->GetRef<S3DModelBuffer>() ;
	}

	pItem->AttachModelReference( pModel ) ;

	LQT_RETURN_VOID() ;
}

// void setModelID( String modelId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setModelID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( modelId ) ;

	pItem->SetModel( modelId.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// String getModelID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getModelID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_STRING( pItem->GetModelID() ) ;
}

// void attachCollisionModel( EntisGLS4.VertexBuffer model )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_attachCollisionModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexBuffer, model ) ;
	S3DVertexBufferInterface *	pModel = nullptr ;
	if ( model != nullptr )
	{
		pModel = model->GetRef<S3DVertexBufferInterface>() ;
	}

	pItem->AttachCollisionModel( pModel ) ;

	LQT_RETURN_VOID() ;
}

// void setCollisionID( String modelId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setCollisionID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( modelId ) ;

	pItem->SetCollision( modelId.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// String getCollisionID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getCollisionID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_STRING( pItem->GetCollisionID() ) ;
}

// boolean isDynamicCollision( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_isDynamicCollision)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_BOOL( pItem->IsDynamicCollision() ) ;
}

// void setDynamicCollisionFlag( boolean flagDynamic )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setDynamicCollisionFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_BOOL( flagDynamic ) ;

	pItem->SetDynamicCollisionFlag( flagDynamic ) ;

	LQT_RETURN_VOID() ;
}

// void setVariantDrawTaregt( String targetItemId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_setVariantDrawTaregt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( targetItemId ) ;

	pItem->SetVariantDrawTaregt( targetItemId.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// String getVariantDrawTarget( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneDynamicModel_getVariantDrawTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneDynamicModel, pThis ) ;
	S3DDynamicModelSerializer *	pItem = pThis->GetRef<S3DDynamicModelSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_STRING( pItem->GetVariantDrawTarget() ) ;
}



