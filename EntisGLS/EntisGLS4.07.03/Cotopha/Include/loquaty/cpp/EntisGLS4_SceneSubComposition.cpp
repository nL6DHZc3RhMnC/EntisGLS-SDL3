
#include <loquaty.h>
#include "EntisGLS4_SceneSubComposition.h"

using namespace Loquaty ;


// SceneSubComposition( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSubComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneSubComposition, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneComposition createInstance( Object objUser )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_createInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LObject, objUser ) ;
	LQT_VERIFY_NULL_PTR( objUser ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposition) ) ) ;
	// valRet = pThis->createInstance(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneComposition> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneComposition>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void releaseInstance( EntisGLS4.SceneComposition comp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_releaseInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;

	// pThis->releaseInstance(...) ;

	LQT_RETURN_VOID() ;
}

// void delayReleaseInstance( EntisGLS4.SceneComposition comp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_delayReleaseInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;

	// pThis->delayReleaseInstance(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isValidInstance( EntisGLS4.SceneComposition comp ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_isValidInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;

	LBoolean	valRet ;
	// valRet = pThis->isValidInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



