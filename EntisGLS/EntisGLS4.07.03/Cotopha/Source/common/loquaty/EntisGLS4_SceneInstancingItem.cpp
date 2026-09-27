
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/EntisGLS4_SceneInstancingItem.h>


// const Quaterniond* getBaseRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getBaseRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;

	LQuaterniond	valRet = pInstancing->GetBaseRotation() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBaseRotation( const Quaterniond* rot )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_setBaseRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LQuaterniond, rot ) ;
	LQT_VERIFY_NULL_PTR( rot ) ;

	pInstancing->SetBaseRotation( *rot ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBaseZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getBaseZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;

	LVector3d	valRet = pInstancing->GetBaseZoom() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBaseZoom( const Vector3d* zoom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_setBaseZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, zoom ) ;
	LQT_VERIFY_NULL_PTR( zoom ) ;

	pInstancing->SetBaseZoom( *zoom ) ;

	LQT_RETURN_VOID() ;
}

// void addTemporaryInstances( const Matrix4* matrixes, const EntisGLS4.ColorMulAdd* colors, ulong count, EntisGLS4.SceneItem itemFrom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_addTemporaryInstances)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancing = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix4, matrixes, LQT_ARG_LONG(3) ) ;
	LQT_VERIFY_NULL_PTR( matrixes ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, colors, LQT_ARG_LONG(3) ) ;
	LQT_VERIFY_NULL_PTR( colors ) ;
	LQT_FUNC_ARG_ULONG( count ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, itemFrom ) ;
	S3DSceneComposer::ItemSerializer *	pItemFrom = nullptr ;
	if ( itemFrom != nullptr )
	{
		pItemFrom = itemFrom->GetRef<S3DSceneComposer::ItemSerializer>() ;
	}

	pInstancing->AddDynamicInstancingEntries
		( matrixes, colors, (size_t) count, pItemFrom ) ;

	LQT_RETURN_VOID() ;
}

// ulong getStaticInstanceCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getStaticInstanceCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;

	LQT_RETURN_ULONG( pInstancing->GetStaticInstanceCount() ) ;
}

// boolean getStaticInstanceAt( ulong index, Matrix4* matrix, EntisGLS4.ColorMulAdd* color ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
	LQT_RETURN_BOOL( pInstancing->GetStaticInstanceAt( (size_t) index, *matrix, *color ) ) ;
}

// void insertStaticInstanceAt( ulong index, const Matrix4* matrix, const EntisGLS4.ColorMulAdd* color )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_insertStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
	pInstancing->InsertStaticInstanceAt
		( (size_t) index, *matrix, *color,
			ESLTypeCast<S3DSceneComposer::ItemSerializer>(pInstancingItem) ) ;

	LQT_RETURN_VOID() ;
}

// void removeStaticInstanceAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_removeStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
	pInstancing->RemoveStaticInstanceAt
		( (size_t) index, 
			ESLTypeCast<S3DSceneComposer::ItemSerializer>(pInstancingItem) ) ;

	LQT_RETURN_VOID() ;
}

// void lockInstancing( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_lockInstancing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;

	pInstancing->Lock() ;

	LQT_RETURN_VOID() ;
}

// void unlockInstancing( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_unlockInstancing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	S3DInstancingItemInterface *	pInstancingItem = pThis->GetRef<S3DInstancingItemInterface>() ;
	LQT_VERIFY_NULL_PTR( pInstancingItem ) ;
	S3DItemInstancingSerializer *	pInstancing = pInstancingItem->GetInstancing() ;
	LQT_VERIFY_NULL_PTR( pInstancing ) ;

	pInstancing->Unlock() ;

	LQT_RETURN_VOID() ;
}


