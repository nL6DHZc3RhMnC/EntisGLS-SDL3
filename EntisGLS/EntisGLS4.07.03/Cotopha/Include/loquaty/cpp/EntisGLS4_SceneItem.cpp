
#include <loquaty.h>
#include "EntisGLS4_SceneItem.h"

using namespace Loquaty ;


// const Matrix3d* getItemRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LMatrix3d	valRet ;
	// valRet = pThis->getItemRotation(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setItemRotation( const Matrix3d* matRot )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matRot ) ;
	LQT_VERIFY_NULL_PTR( matRot ) ;

	// pThis->setItemRotation(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getItemZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getItemZoom(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setItemZoom( const Vector3d* vZoom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vZoom ) ;
	LQT_VERIFY_NULL_PTR( vZoom ) ;

	// pThis->setItemZoom(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getItemPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getItemPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// uint getItemTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getItemTransparency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// const EntisGLS4.ColorMulAdd* getItemColorEffect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemColorEffect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LEntisGLS4_ColorMulAdd	valRet ;
	// valRet = pThis->getItemColorEffect(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean getVisibleParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getVisibleParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->getVisibleParameter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setVisibleParameter( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setVisibleParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	// pThis->setVisibleParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setItemMatrix( const Matrix3d* matrix )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;

	// pThis->setItemMatrix(...) ;

	LQT_RETURN_VOID() ;
}

// void setItemPositioin( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_setItemPositioin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	// pThis->setItemPositioin(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getParentSpaceItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getParentSpaceItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getParentSpaceItem(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposition getComposition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposition) ) ) ;
	// valRet = pThis->getComposition(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneComposition> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneComposition>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneComposer getComposer( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getComposer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneComposer) ) ) ;
	// valRet = pThis->getComposer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneComposer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneComposer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneManager getManager( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getManager)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneManager) ) ) ;
	// valRet = pThis->getManager(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneManager> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneManager>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Scene getScene( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getScene)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Scene) ) ) ;
	// valRet = pThis->getScene(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Scene> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Scene>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneItem getSceneItemAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getSceneItemAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getSceneItemAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void getItemLinkTransformation( Matrix3d* matLink, Vector3d* vLinkPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getItemLinkTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matLink ) ;
	LQT_VERIFY_NULL_PTR( matLink ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vLinkPos ) ;
	LQT_VERIFY_NULL_PTR( vLinkPos ) ;

	// pThis->getItemLinkTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// void getGlobalTransformation( Matrix3d* matGlobal, Vector3d* vGlobalPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getGlobalTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matGlobal ) ;
	LQT_VERIFY_NULL_PTR( matGlobal ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vGlobalPos ) ;
	LQT_VERIFY_NULL_PTR( vGlobalPos ) ;

	// pThis->getGlobalTransformation(...) ;

	LQT_RETURN_VOID() ;
}

// void getGlobalColorEffect( EntisGLS4.ColorMulAdd* clrEffect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getGlobalColorEffect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, clrEffect ) ;
	LQT_VERIFY_NULL_PTR( clrEffect ) ;

	// pThis->getGlobalColorEffect(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getChildItemCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getChildItemCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getChildItemCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.SceneItem getChildItemAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getChildItemAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getChildItemAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long findChildItem( EntisGLS4.SceneItem item ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findChildItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;

	LInt64	valRet ;
	// valRet = pThis->findChildItem(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong getControllerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getControllerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// long findControllerID( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findControllerID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LInt64	valRet ;
	// valRet = pThis->findControllerID(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// long findController( EntisGLS4.SceneController ctrl ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_findController)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneController, ctrl ) ;
	LQT_VERIFY_NULL_PTR( ctrl ) ;

	LInt64	valRet ;
	// valRet = pThis->findController(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// EntisGLS4.SceneController getControllerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneController) ) ) ;
	// valRet = pThis->getControllerAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneController> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneController getControllerAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_getControllerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneController) ) ) ;
	// valRet = pThis->getControllerAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneController> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneController>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean removeControllerAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_removeControllerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeControllerAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean removeControllerAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_removeControllerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeControllerAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isEditMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItem_isEditMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItem, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEditMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



