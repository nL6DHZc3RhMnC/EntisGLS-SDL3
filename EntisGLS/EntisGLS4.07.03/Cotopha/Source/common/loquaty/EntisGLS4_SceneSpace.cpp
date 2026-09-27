
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneSpace.h>


// SceneSpace( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSpace)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneSpace, pThis,
			( new SSmartObject
				( (S3DSceneComposer::CommonSerializer*)
					new S3DSceneComposer::SpaceSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean addChild( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSpace_addChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSpace, pThis ) ;
	S3DSceneComposer::SpaceSerializer *
		pSpace = pThis->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSpace ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LBoolean	valRet = false ;
	S3DSceneComposer::SpaceSerializer *
		pSpaceItem = item->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	if ( pSpaceItem != nullptr )
	{
		pSpace->AddChild( pSpaceItem ) ;
		valRet = true ;
	}
	else
	{
		S3DSceneComposer::ItemCommonSerializer *
			pCmnItem = item->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
		if ( (pCmnItem != nullptr)
			&& (pCmnItem->GetSceneItem() != nullptr) )
		{
			pSpace->AddItem( pCmnItem->GetSceneItem() ) ;
			valRet = true ;
		}
	}

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean removeChild( EntisGLS4.SceneItem item )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSpace_removeChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSpace, pThis ) ;
	S3DSceneComposer::SpaceSerializer *
		pSpace = pThis->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSpace ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LBoolean	valRet = false ;
	S3DSceneComposer::SpaceSerializer *
		pSpaceItem = item->GetRef<S3DSceneComposer::SpaceSerializer>() ;
	if ( pSpaceItem != nullptr )
	{
		valRet = pSpace->RemoveChild( pSpaceItem ) ;
	}
	else
	{
		S3DSceneComposer::ItemCommonSerializer *
			pCmnItem = item->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
		if ( (pCmnItem != nullptr)
			&& (pCmnItem->GetSceneItem() != nullptr) )
		{
			valRet = pSpace->RemoveItem( pCmnItem->GetSceneItem() ) ;
		}
	}

	LQT_RETURN_BOOL( valRet ) ;
}




