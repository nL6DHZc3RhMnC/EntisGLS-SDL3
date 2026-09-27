
#include <loquaty.h>
#include "EntisGLS4_SceneComposition.h"

using namespace Loquaty ;


// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEditMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void applySceneParameters( EntisGLS4.Scene scene, const Size* sizeView ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_applySceneParameters)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	LQT_VERIFY_NULL_PTR( scene ) ;
	LQT_FUNC_ARG_STRUCT( LSize, sizeView ) ;
	LQT_VERIFY_NULL_PTR( sizeView ) ;

	// pThis->applySceneParameters(...) ;

	LQT_RETURN_VOID() ;
}

// void prepareToRender( EntisGLS4.RenderDevice device, EntisGLS4.Scene scene, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_prepareToRender)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	LQT_VERIFY_NULL_PTR( scene ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	// pThis->prepareToRender(...) ;

	LQT_RETURN_VOID() ;
}

// void initializeItems( EntisGLS4.Scene scene )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_initializeItems)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	LQT_VERIFY_NULL_PTR( scene ) ;

	// pThis->initializeItems(...) ;

	LQT_RETURN_VOID() ;
}

// void playComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_playComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	// pThis->playComposition(...) ;

	LQT_RETURN_VOID() ;
}

// void stopCompositoin( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_stopCompositoin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	// pThis->stopCompositoin(...) ;

	LQT_RETURN_VOID() ;
}

// void restartComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_restartComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	// pThis->restartComposition(...) ;

	LQT_RETURN_VOID() ;
}

// void pauseCompositoin( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_pauseCompositoin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	// pThis->pauseCompositoin(...) ;

	LQT_RETURN_VOID() ;
}

// void finishComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_finishComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	// pThis->finishComposition(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isPlayingComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isPlayingComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isPlayingComposition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isPausedComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isPausedComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isPausedComposition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isCompositionFinished( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isCompositionFinished)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isCompositionFinished(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// double getCurrentPlayingFrame( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_getCurrentPlayingFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getCurrentPlayingFrame(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void postTimelineFrame( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_postTimelineFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	// pThis->postTimelineFrame(...) ;

	LQT_RETURN_VOID() ;
}

// boolean removeSpaceChild( EntisGLS4.SceneItem space, EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_removeSpaceChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeSpaceChild(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void postDelayRemoveItem( EntisGLS4.SceneItem space, EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_postDelayRemoveItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	// pThis->postDelayRemoveItem(...) ;

	LQT_RETURN_VOID() ;
}

// Object getUserObject( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_getUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getUserObject(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setUserObject( Object obj )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_setUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LObject, obj ) ;
	LQT_VERIFY_NULL_PTR( obj ) ;

	// pThis->setUserObject(...) ;

	LQT_RETURN_VOID() ;
}



