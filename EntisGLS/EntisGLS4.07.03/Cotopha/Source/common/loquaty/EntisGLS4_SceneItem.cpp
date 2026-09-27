
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneItem.h>


// const Matrix3d* getItemRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LMatrix3d	valRet = pItem->GetItemRotation() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setItemRotation( const Matrix3d* matRot )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matRot ) ;
	LQT_VERIFY_NULL_PTR( matRot ) ;

	pItem->SetItemRotation( *matRot ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getItemZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LVector3d	valRet = pItem->GetItemZoom() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setItemZoom( const Vector3d* vZoom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vZoom ) ;
	LQT_VERIFY_NULL_PTR( vZoom ) ;

	pItem->SetItemZoom( *vZoom ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getItemPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LVector3d	valRet = pItem->GetItemPosition() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// uint getItemTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_UINT( pItem->GetItemTransparency() ) ;
}

// const EntisGLS4.ColorMulAdd* getItemColorEffect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemColorEffect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LEntisGLS4_ColorMulAdd	valRet = pItem->GetItemColorEffect() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean getVisibleParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getVisibleParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_BOOL( pItem->GetVisibleParameter() ) ;
}

// void setVisibleParameter( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setVisibleParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	pItem->SetVisibleParameter( visible ) ;

	LQT_RETURN_VOID() ;
}

// void setItemMatrix( const Matrix3d* matrix )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;

	pItem->SetItemMatrix( *matrix ) ;

	LQT_RETURN_VOID() ;
}

// void setItemPositioin( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemPositioin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	pItem->SetItemPositioin( *vPos ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getParentSpaceItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getParentSpaceItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DSceneComposer::ItemSerializer *	pParent = pItem->GetParentSpaceItem() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pParent ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposition getComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;
	if ( pComp == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pComp ) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_SceneComposition>
			( (S3DSceneComposer::CommonSerializer*) pComp) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposer getComposer( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getComposer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DSceneComposer *	pComposer = pItem->GetComposer() ;
	if ( pComposer == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneComposer>(pComposer) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneManager getManager( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getManager)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DCompositionManager *	pManager = pItem->GetManager() ;
	if ( pManager == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneManager) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneManager>(pManager) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Scene getScene( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::CommonSerializer *	pItem = pThis->GetRef<S3DSceneComposer::CommonSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DScene *	pScene = pItem->GetScene() ;
	if ( pScene == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet
		( new LNativeObj( _context.VM().GetClassPathAs( pScene->GetLQClassName() ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Scene>(pScene) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneItem getSceneItemAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getSceneItemAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	S3DSceneComposer::ItemSerializer *	pSub = pItem->GetSceneItemAs( id.c_str() ) ;
	if ( pSub == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pSub ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pSub) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void getItemLinkTransformation( Matrix3d* matLink, Vector3d* vLinkPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemLinkTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matLink ) ;
	LQT_VERIFY_NULL_PTR( matLink ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vLinkPos ) ;
	LQT_VERIFY_NULL_PTR( vLinkPos ) ;

	pItem->GetItemLinkTransformation( *matLink, *vLinkPos ) ;

	LQT_RETURN_VOID() ;
}

// void getGlobalTransformation( Matrix3d* matGlobal, Vector3d* vGlobalPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getGlobalTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matGlobal ) ;
	LQT_VERIFY_NULL_PTR( matGlobal ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vGlobalPos ) ;
	LQT_VERIFY_NULL_PTR( vGlobalPos ) ;

	pItem->GetGlobalTransformation( *matGlobal, *vGlobalPos ) ;

	LQT_RETURN_VOID() ;
}

// void getGlobalColorEffect( EntisGLS4.ColorMulAdd* clrEffect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getGlobalColorEffect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, clrEffect ) ;
	LQT_VERIFY_NULL_PTR( clrEffect ) ;

	pItem->GetGlobalColorEffect( *clrEffect ) ;

	LQT_RETURN_VOID() ;
}

// ulong getChildItemCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getChildItemCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_ULONG( pItem->GetChildItemCount() ) ;
}

// EntisGLS4.SceneItem getChildItemAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getChildItemAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DSceneComposer::ItemSerializer *	pChild = pItem->GetChildItemAt( (size_t) index ) ;
	if ( pChild == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pChild ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pChild) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long findChildItem( EntisGLS4.SceneItem item ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findChildItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pThisItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pThisItem ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	S3DSceneComposer::ItemSerializer *	pChildItem = item->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pChildItem ) ;

	LQT_RETURN_LONG( pThisItem->FindChildItem( pChildItem ) ) ;
}

// ulong getControllerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	LQT_RETURN_ULONG( pItem->GetControllerCount() ) ;
}

// long findControllerID( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findControllerID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_LONG( pItem->FindControllerID( id.c_str() ) ) ;
}

// long findController( EntisGLS4.SceneController ctrl ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findController)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneController, ctrl ) ;
	LQT_VERIFY_NULL_PTR( ctrl ) ;
	S3DSceneComposer::Controller *	pCtrl = ctrl->GetRef<S3DSceneComposer::Controller>() ;
	LQT_VERIFY_NULL_PTR( pCtrl ) ;

	LQT_RETURN_LONG( pItem->FindController( pCtrl ) ) ;
}

// EntisGLS4.SceneController getControllerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DSceneComposer::Controller *	pCtrl = pItem->GetControllerAt( (size_t) index ) ;
	if ( pCtrl == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pCtrl ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>(pCtrl) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneController getControllerAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	S3DSceneComposer::Controller *	pCtrl = pItem->GetControllerAs( id.c_str() ) ;
	if ( pCtrl == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSceneItemClass( _context.VM(), pCtrl ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>(pCtrl) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean removeControllerAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_removeControllerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_BOOL( pItem->RemoveControllerAt( (size_t) index ) == sglErrSuccess ) ;
}

// boolean removeControllerAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_removeControllerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL( pItem->RemoveControllerAs( id.c_str() ) == sglErrSuccess ) ;
}

// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	S3DSceneComposer::ItemSerializer *	pItem = pThis->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;

	S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;

	LQT_RETURN_BOOL( (pComp != nullptr) && pComp->IsEditMode() ) ;
}


