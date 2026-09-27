
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_WindowSprite.h>


// WindowSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_WindowSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_WindowSprite, pThis,
			( new SSmartObject( (SGLSprite*) new SGLWindowSprite ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean postUpdate( const ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_postUpdate_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;

	if ( pUpdate != nullptr )
	{
		SGLRect	rect = *pUpdate ;
		pSprite->PostUpdate( &rect ) ;
	}
	else
	{
		pSprite->PostUpdate( nullptr ) ;
	}

	LQT_RETURN_BOOL( true ) ;
}

// EntisGLS4.Sprite getDirectRoot( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_getDirectRoot)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	SGLWindowSprite *	pWindow = pThis->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_Sprite>
				( &(pWindow->GetDirectRootSprite()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void enableSpriteTimer( boolean flagTimer )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_enableSpriteTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	SGLWindowSprite *	pWindow = pThis->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_BOOL( flagTimer ) ;

	pWindow->EnableSpriteTimer( flagTimer ) ;

	LQT_RETURN_VOID() ;
}

// void setAutoHideCursor( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_WindowSprite_setAutoHideCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_WindowSprite, pThis ) ;
	SGLWindowSprite *	pWindow = pThis->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	pWindow->SetAutoHideCursor( msecTimeout ) ;

	LQT_RETURN_VOID() ;
}



