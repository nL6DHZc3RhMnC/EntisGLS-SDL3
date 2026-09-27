
#include <loquaty.h>
#include "EntisGLS4_SceneController.h"

using namespace Loquaty ;


// EntisGLS4.SceneItem getOwnerItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneController_getOwnerItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneController, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getOwnerItem(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneController_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneController, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEditMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



