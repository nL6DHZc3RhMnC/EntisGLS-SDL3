
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneCommon.h>


// EntisGLS4.Scene.ItemClass getItemClass( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_getItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	S3DSceneComposer::ItemCommonSerializer *
		pCommon = pThis->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pCommon ) ;

	LUint32	valRet = 0 ;
	S3DScene::Item *	pItem = pCommon->GetSceneItem() ;
	if ( pItem != nullptr )
	{
		valRet = pItem->m_classItem ;
	}

	LQT_RETURN_UINT( valRet ) ;
}

// void setItemClass( EntisGLS4.Scene.ItemClass cls )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_setItemClass)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	S3DSceneComposer::ItemCommonSerializer *
		pCommon = pThis->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pCommon ) ;
	LQT_FUNC_ARG_UINT( cls ) ;

	S3DScene::Item *	pItem = pCommon->GetSceneItem() ;
	if ( pItem != nullptr )
	{
		pItem->m_classItem = (S3DScene::ItemClass) cls ;
	}

	LQT_RETURN_VOID() ;
}

// uint getRenderPriority( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_getRenderPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	S3DSceneComposer::ItemCommonSerializer *
		pCommon = pThis->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pCommon ) ;

	LUint32	valRet = 0 ;
	S3DScene::Item *	pItem = pCommon->GetSceneItem() ;
	if ( pItem != nullptr )
	{
		valRet = pItem->m_nRenderPriority ;
	}

	LQT_RETURN_UINT( valRet ) ;
}

// void setRenderPriority( uint priority )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCommon_setRenderPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCommon, pThis ) ;
	S3DSceneComposer::ItemCommonSerializer *
		pCommon = pThis->GetRef<S3DSceneComposer::ItemCommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pCommon ) ;
	LQT_FUNC_ARG_UINT( priority ) ;

	S3DScene::Item *	pItem = pCommon->GetSceneItem() ;
	if ( pItem != nullptr )
	{
		pItem->m_nRenderPriority = priority ;
	}

	LQT_RETURN_VOID() ;
}



