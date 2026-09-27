
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneController.h>


// EntisGLS4.SceneItem getOwnerItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneController_getOwnerItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneController, pThis ) ;
	S3DSceneComposer::Controller *	pCtrl = pThis->GetRef<S3DSceneComposer::Controller>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;

	S3DSceneComposer::ItemSerializer *	pItem = pCtrl->GetOwnerItem() ;
	if ( pItem == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pItem) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneController_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneController, pThis ) ;
	S3DSceneComposer::Controller *	pCtrl = pThis->GetRef<S3DSceneComposer::Controller>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;

	S3DSceneComposer::Composition *	pComp = pCtrl->GetComposition() ;

	LQT_RETURN_BOOL( (pComp != nullptr) && pComp->IsEditMode() ) ;
}



