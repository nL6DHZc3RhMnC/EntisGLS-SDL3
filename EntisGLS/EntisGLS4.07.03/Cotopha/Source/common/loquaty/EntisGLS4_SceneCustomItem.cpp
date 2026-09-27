
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneCustomItem.h>


// SceneCustomItem( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneCustomItem,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneCustomItem>
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
					new S3DSceneCustomItem
						( _context.VM(), pThis, typeId.c_str() ) ) ) ) ;

	LQT_RETURN_VOID() ;
}



