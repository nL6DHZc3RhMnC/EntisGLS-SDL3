
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneComposer.h>


// SceneComposer( EntisGLS4.SceneManager manager )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneComposer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneComposer, pThis, () ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneManager, manager ) ;
	LQT_VERIFY_NULL_PTR( manager ) ;
	S3DCompositionManager *	pManager = manager->GetRef<S3DCompositionManager>() ;
	LQT_VERIFY_NULL_PTR( pManager ) ;

	pThis->SetSmartReference( new S3DSceneComposer( pManager ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadComposeFile( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_loadComposeFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pComposer->LoadComposeFile( file.c_str() ) == sglErrSuccess ) ;
}

// EntisGLS4.ModelBuffer getAssetModelAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getAssetModelAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	S3DModelBuffer *	pModel = pComposer->GetAssets().GetModelAs( id.c_str() ) ;
	if ( pModel == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBuffer) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_ModelBuffer>
				( (S3DRenderBufferInterface*) pModel) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.AudioPlayer getAssetAudioAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getAssetAudioAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	SGLAudioPlayer *	pAudio = pComposer->GetAssets().GetAudioAs( id.c_str() ) ;
	if ( pAudio == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.AudioPlayer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_AudioPlayer>(pAudio) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.TextureLibrary getTextureLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getTextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_TextureLibrary>
				( &(pComposer->Assets().TextureLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.MaterialLibrary getMaterialLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getMaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_MaterialLibrary>
				( &(pComposer->Assets().MaterialLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getModelPoseLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getModelPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_ModelPoseLibrary>
					( &(pComposer->Assets().PoseLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneController createController( EntisGLS4.SceneItem item, ulong index, String typeId, String ctrlId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createController)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	S3DSceneComposer::ItemSerializer *	pItem = item->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_STRING( ctrlId ) ;

	S3DSceneComposer::Controller *	pCtrl = pComposer->CreateController( typeId.c_str() ) ;
	if ( pCtrl == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	pItem->InsertController( (size_t) index, pCtrl ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pCtrl ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>(pCtrl) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneItem createItemeChildOf( EntisGLS4.SceneItem space, String typeId, String itemId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createItemeChildOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	S3DSceneComposer::SpaceSerializer *	pSpace = space->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSpace ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_STRING( itemId ) ;

	S3DSceneComposer::Composition *	pComp = pSpace->GetComposition() ;
	if ( pComp == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	S3DSceneComposer::ItemSerializer *
			pItem = pComposer->CreateSceneItem( typeId.c_str() ) ;
	if ( pItem == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	pComp->AddSpaceChild( *pSpace, pItem, itemId.c_str() ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pItem ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pItem) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposition createComposition( String compId, Object objInstance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_STRING( compId ) ;
	LQT_FUNC_ARG_OBJECT( LObject, objInstance ) ;

	S3DSceneComposer::CompositionInfo *	pci = pComposer->GetCompositionAs( compId.c_str() ) ;
	if ( pci == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	S3DSceneComposer::Composition *	pComp = pComposer->CreateComposition( *pci ) ;
	if ( pComp == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	if ( objInstance != nullptr )
	{
		pComp->SetLoquatyInstance( objInstance ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pComp ) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_SceneComposition>
				( new SSmartObject( (S3DSceneComposer::CommonSerializer*) pComp ) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void outputLog( String msg )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_outputLog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	S3DSceneComposer *	pComposer = pThis->GetRef<S3DSceneComposer>() ;
	LQT_VERIFY_NULL_PTR( pComposer ) ;
	LQT_FUNC_ARG_STRING( msg ) ;

	pComposer->OutputTraceLog( msg.c_str() ) ;

	LQT_RETURN_VOID() ;
}

