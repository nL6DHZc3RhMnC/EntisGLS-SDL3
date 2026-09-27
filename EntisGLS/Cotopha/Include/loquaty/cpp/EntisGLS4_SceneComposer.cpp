
#include <loquaty.h>
#include "EntisGLS4_SceneComposer.h"

using namespace Loquaty ;


// SceneComposer( EntisGLS4.SceneManager manager )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneComposer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneManager, manager ) ;
	LQT_VERIFY_NULL_PTR( manager ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneComposer>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadComposeFile( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_loadComposeFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadComposeFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.ModelBuffer getAssetModelAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getAssetModelAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBuffer) ) ) ;
	// valRet = pThis->getAssetModelAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelBuffer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelBuffer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.AudioPlayer getAssetAudioAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getAssetAudioAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.AudioPlayer) ) ) ;
	// valRet = pThis->getAssetAudioAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_AudioPlayer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_AudioPlayer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.TextureLibrary getTextureLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getTextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	// valRet = pThis->getTextureLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_TextureLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.MaterialLibrary getMaterialLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getMaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	// valRet = pThis->getMaterialLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_MaterialLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getModelPoseLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_getModelPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	// valRet = pThis->getModelPoseLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPoseLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneController createController( EntisGLS4.SceneItem item, ulong index, String typeId, String ctrlId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createController)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_STRING( ctrlId ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneController) ) ) ;
	// valRet = pThis->createController(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneController> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneItem createItemeChildOf( EntisGLS4.SceneItem space, String typeId, String itemId )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createItemeChildOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, space ) ;
	LQT_VERIFY_NULL_PTR( space ) ;
	LQT_FUNC_ARG_STRING( typeId ) ;
	LQT_FUNC_ARG_STRING( itemId ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->createItemeChildOf(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposition createComposition( String compId, Object objInstance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_createComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_STRING( compId ) ;
	LQT_FUNC_ARG_OBJECT( LObject, objInstance ) ;
	LQT_VERIFY_NULL_PTR( objInstance ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposition) ) ) ;
	// valRet = pThis->createComposition(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneComposition> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneComposition>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void outputLog( String msg )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneComposer_outputLog)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneComposer, pThis ) ;
	LQT_FUNC_ARG_STRING( msg ) ;

	// pThis->outputLog(...) ;

	LQT_RETURN_VOID() ;
}



