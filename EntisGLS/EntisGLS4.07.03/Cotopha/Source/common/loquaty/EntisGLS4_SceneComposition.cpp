
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneComposition.h>


// void applySceneParameters( EntisGLS4.Scene scene, const Size* sizeView ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_applySceneParameters)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	LQT_VERIFY_NULL_PTR( scene ) ;
	S3DScene *	pScene = scene->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_STRUCT( LSize, sizeView ) ;
	LQT_VERIFY_NULL_PTR( sizeView ) ;

	pComp->ApplySceneParameters( *pScene, *sizeView ) ;

	LQT_RETURN_VOID() ;
}

// void prepareToRender( EntisGLS4.RenderDevice device, EntisGLS4.Scene scene, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_prepareToRender)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;
	S3DRenderDevice *	pDevice = device->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	LQT_VERIFY_NULL_PTR( scene ) ;
	S3DScene *	pScene = scene->GetRef<S3DScene>() ;
	LQT_VERIFY_NULL_PTR( pScene ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	pComp->PrepareToRender( pDevice, pScene, flags ) ;

	LQT_RETURN_VOID() ;
}

// void initializeItems( EntisGLS4.Scene scene )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_initializeItems)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Scene, scene ) ;
	S3DScene *	pScene = nullptr ;
	if ( scene != nullptr )
	{
		pScene = scene->GetRef<S3DScene>() ;
	}

	pComp->InitializeItems( pScene ) ;

	LQT_RETURN_VOID() ;
}

// void playComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_playComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pComp->PlayComposition() ;

	LQT_RETURN_VOID() ;
}

// void stopCompositoin( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_stopCompositoin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pComp->StopCompositoin() ;

	LQT_RETURN_VOID() ;
}

// void restartComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_restartComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pComp->RestartComposition() ;

	LQT_RETURN_VOID() ;
}

// void pauseCompositoin( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_pauseCompositoin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pComp->PauseCompositoin() ;

	LQT_RETURN_VOID() ;
}

// void finishComposition( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_finishComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pComp->FinishComposition() ;

	LQT_RETURN_VOID() ;
}

// boolean isPlayingComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isPlayingComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_BOOL( pComp->IsPlayingComposition() ) ;
}

// boolean isPausedComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isPausedComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_BOOL( pComp->IsPausedComposition() ) ;
}

// boolean isCompositionFinished( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isCompositionFinished)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_BOOL( pComp->IsCompositionFinished() ) ;
}

// double getCurrentPlayingFrame( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_getCurrentPlayingFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_DOUBLE( pComp->GetCurrentPlayingFrame() ) ;
}

// void postTimelineFrame( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_postTimelineFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	pComp->PostTimelineFrame( frame ) ;

	LQT_RETURN_VOID() ;
}

// boolean removeSpaceChild( EntisGLS4.SceneItem space, EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_removeSpaceChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	S3DSceneComposer::SpaceSerializer *	pSpace = space->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSpace ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	S3DSceneComposer::ItemSerializer *	pItem = item->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_BOOL( pComp->RemoveSpaceChild( *pSpace, pItem ) == sglErrSuccess ) ;
}

// void postDelayRemoveItem( EntisGLS4.SceneItem space, EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_postDelayRemoveItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	S3DSceneComposer::SpaceSerializer *	pSpace = space->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSpace ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	S3DSceneComposer::ItemSerializer *	pItem = item->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	pComp->PostDelayRemoveItem( *pSpace, pItem ) ;

	LQT_RETURN_VOID() ;
}

// Object getUserObject( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_getUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LObjPtr	valRet = pComp->GetLoquatyInstance() ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setUserObject( Object obj )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_setUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;
	LQT_FUNC_ARG_OBJECT( LObject, obj ) ;
	LQT_VERIFY_NULL_PTR( obj ) ;

	pComp->SetLoquatyInstance( obj ) ;

	LQT_RETURN_VOID() ;
}

// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposition_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposition, pThis ) ;
	S3DSceneComposer::Composition *	pComp = pThis->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_BOOL( pComp->IsEditMode() ) ;
}




