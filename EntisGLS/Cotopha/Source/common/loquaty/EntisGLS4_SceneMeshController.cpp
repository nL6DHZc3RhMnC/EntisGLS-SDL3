
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneMeshController.h>



// SceneMeshController( String typeId )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneMeshController,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneMeshController>
			( new SSmartObject
				( (S3DSceneComposer::Controller*)
					new SceneMeshController
						( _context.VM(), pThis, typeId.c_str() ) ) ) ) ;

	LQT_RETURN_VOID() ;
}



