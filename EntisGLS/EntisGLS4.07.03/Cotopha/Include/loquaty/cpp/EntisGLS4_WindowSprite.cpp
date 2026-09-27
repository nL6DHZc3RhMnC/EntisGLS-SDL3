
#include <loquaty.h>
#include "EntisGLS4_WindowSprite.h"

using namespace Loquaty ;


// WindowSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_WindowSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_WindowSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean postUpdate( const ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_postUpdate_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;
	LQT_VERIFY_NULL_PTR( pUpdate ) ;

	LBoolean	valRet ;
	// valRet = pThis->postUpdate(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Sprite getDirectRoot( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_getDirectRoot)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getDirectRoot(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void enableSpriteTimer( boolean flagTimer )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_enableSpriteTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagTimer ) ;

	// pThis->enableSpriteTimer(...) ;

	LQT_RETURN_VOID() ;
}

// void setAutoHideCursor( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_setAutoHideCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	// pThis->setAutoHideCursor(...) ;

	LQT_RETURN_VOID() ;
}



