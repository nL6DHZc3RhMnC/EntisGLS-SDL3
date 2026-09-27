
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/EntisGLS4_SceneItemInstanceRef.h>


// SceneItemInstanceRef( const EntisGLS4.SceneItemInstanceRef ref )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneItemInstanceRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;
	S3DItemInstanceRef *	pRef = ref->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pRef ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneItemInstanceRef>
			( new SSmartObject( new S3DItemInstanceRef( *pRef ) ) ) ) ;

	LQT_RETURN_VOID() ;
}

// SceneItemInstanceRef( EntisGLS4.SceneItem item, ulong instance )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneItemInstanceRef,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	S3DSceneComposer::ItemSerializer *	pItem = item->GetRef<S3DSceneComposer::ItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pItem ) ;
	LQT_FUNC_ARG_ULONG( instance ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneItemInstanceRef>
			( new SSmartObject( new S3DItemInstanceRef( pItem, (size_t) instance ) ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItemInstanceRef operator :=( const EntisGLS4.SceneItemInstanceRef ref )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_operator_smov)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;
	S3DItemInstanceRef *	pRef = ref->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pRef ) ;

	*pThisRef = *pRef ;

	LQT_RETURN_OBJECT( LQT_ARG_OBJECT(0) ) ;
}

// boolean isEmpty( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isEmpty)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	LQT_RETURN_BOOL( pThisRef->IsEmpty() ) ;
}

// boolean isEqual( const EntisGLS4.SceneItemInstanceRef ref ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isEqual)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;
	S3DItemInstanceRef *	pRef = ref->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pRef ) ;

	LQT_RETURN_BOOL( pThisRef->IsEqual( *pRef ) ) ;
}

// void releaseRef( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_releaseRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	// pThis->releaseRef(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getRefItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getRefItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	S3DSceneComposer::ItemSerializer *	pItem = pThisRef->GetRefItem() ;
	if ( pItem == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pItem) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isInstance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	LQT_RETURN_BOOL( pThisRef->IsInstance() ) ;
}

// long getInstanceIndex( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getInstanceIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	LQT_RETURN_LONG( pThisRef->GetInstanceIndex() ) ;
}

// boolean correctInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_correctInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	LQT_RETURN_BOOL( pThisRef->CorrectInstance() ) ;
}

// boolean getInstanceMatrix( Matrix4* matrix, EntisGLS4.ColorMulAdd* color )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getInstanceMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	LQT_RETURN_BOOL( pThisRef->GetInstanceMatrix( *matrix, *color ) ) ;
}

// boolean deleteInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_deleteInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	S3DItemInstanceRef *	pThisRef = pThis->GetRef<S3DItemInstanceRef>() ;
	LQT_VERIFY_NULL_PTR( pThisRef ) ;

	LQT_RETURN_BOOL( pThisRef->DeleteInstance() ) ;
}



