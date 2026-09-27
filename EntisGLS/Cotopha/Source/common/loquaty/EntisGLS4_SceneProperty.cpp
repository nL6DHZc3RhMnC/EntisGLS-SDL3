
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneProperty.h>


// String getItemIdentity( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getItemIdentity)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;

	LQT_RETURN_STRING( pProp->GetItemIdentity() ) ;
}

// void setItemIdentity( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_setItemIdentity)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	pProp->SetItemIdentity( id.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// ulong getParameterCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;

	LQT_RETURN_ULONG( pProp->GetParameterCount() ) ;
}

// String getParameterID( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_STRING( pProp->GetParameterID( (size_t) iParam ) ) ;
}

// String getParameterFriendlyName( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterFriendlyName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_STRING( pProp->GetParameterFriendlyName( (size_t) iParam ) ) ;
}

// String getParameterDescription( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_STRING( pProp->GetParameterDescription( (size_t) iParam ) ) ;
}

// long findParameterID( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_findParameterID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_LONG( pProp->FindParameterID( id.c_str() ) ) ;
}

// int getParameterType( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_INT( pProp->GetParameterType( (size_t) iParam ) ) ;
}

// uint getParameterAttributes( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterAttributes)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_UINT( pProp->GetParameterAttributes( (size_t) iParam ) ) ;
}

// boolean getParameterScalarRange( ulong iParam, double* pMin, double* pMax ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterScalarRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_POINTER( LDouble, pMin ) ;
	LQT_VERIFY_NULL_PTR( pMin ) ;
	LQT_FUNC_ARG_POINTER( LDouble, pMax ) ;
	LQT_VERIFY_NULL_PTR( pMax ) ;

	LQT_RETURN_BOOL( pProp->GetParameterScalarRange( (size_t) iParam, *pMin, *pMax ) ) ;
}

// boolean isParameterValidation( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_isParameterValidation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LQT_RETURN_BOOL( pProp->IsParameterValidation( (size_t) iParam ) ) ;
}

// String getParameterCategoryName( ulong iCategory ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterCategoryName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iCategory ) ;

	LQT_RETURN_STRING( pProp->GetParameterCategoryName( (size_t) iCategory ) ) ;
}

// EntisGLS4.SceneSequencer getParameterSequencer( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	S3DSceneComposer::Sequencer *
			pSeq = pProp->GetParameterSequencer( (size_t) iParam ) ;
	if ( pSeq == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSequencer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSequencer>(pSeq) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneSequencer createParameterSequencer( ulong iParam )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_createParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	S3DSceneComposer::Sequencer *
			pSeq = pProp->CreateParameterSequencer( (size_t) iParam ) ;
	if ( pSeq == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSequencer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSequencer>(pSeq) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeParameterSequencer( ulong iParam )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_removeParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	pProp->RemoveParameterSequencer( (size_t) iParam ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.ModelPoseLibrary getPoseLibraryChain( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getPoseLibraryChain)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	S3DSceneComposer::ParameterProperty *	pProp = pThis->GetRef<S3DSceneComposer::ParameterProperty>() ;
	LQT_VERIFY_NULL_PTR( pProp ) ;

	S3DModelPoseLibrary *	pLib = pProp->GetPoseLibraryChain() ;
	if ( pLib == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>(pLib) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



