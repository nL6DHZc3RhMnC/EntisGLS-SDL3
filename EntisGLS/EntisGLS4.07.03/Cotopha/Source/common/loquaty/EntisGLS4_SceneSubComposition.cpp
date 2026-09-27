
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/EntisGLS4_SceneSubComposition.h>


// SceneSubComposition( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSubComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneSubComposition, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
						new S3DSubCompositionSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneComposition createInstance( Object objUser )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_createInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	S3DSubCompositionSerializer *	pSubComp = pThis->GetRef<S3DSubCompositionSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSubComp ) ;
	LQT_FUNC_ARG_OBJECT( LObject, objUser ) ;

	S3DSceneComposer::Composition *	pComp =
		pSubComp->CreateInstance( S3DSceneComposer::ScriptObject( objUser ) ) ;
	if ( pComp == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposition) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_SceneComposition>
			( (S3DSceneComposer::ItemSerializer*) pComp) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void releaseInstance( EntisGLS4.SceneComposition comp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_releaseInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	S3DSubCompositionSerializer *	pSubComp = pThis->GetRef<S3DSubCompositionSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSubComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;
	S3DSceneComposer::Composition *	pComp = comp->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pSubComp->ReleaseInstance( pComp ) ;

	LQT_RETURN_VOID() ;
}

// void delayReleaseInstance( EntisGLS4.SceneComposition comp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_delayReleaseInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	S3DSubCompositionSerializer *	pSubComp = pThis->GetRef<S3DSubCompositionSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSubComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;
	S3DSceneComposer::Composition *	pComp = comp->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	pSubComp->DelayReleaseInstance( pComp ) ;

	LQT_RETURN_VOID() ;
}

// boolean isValidInstance( EntisGLS4.SceneComposition comp ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSubComposition_isValidInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSubComposition, pThis ) ;
	S3DSubCompositionSerializer *	pSubComp = pThis->GetRef<S3DSubCompositionSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSubComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneComposition, comp ) ;
	LQT_VERIFY_NULL_PTR( comp ) ;
	S3DSceneComposer::Composition *	pComp = comp->GetRef<S3DSceneComposer::Composition>() ;
	LQT_VERIFY_NULL_PTR( pComp ) ;

	LQT_RETURN_BOOL( pSubComp->IsValidInstance( pComp ) ) ;
}



