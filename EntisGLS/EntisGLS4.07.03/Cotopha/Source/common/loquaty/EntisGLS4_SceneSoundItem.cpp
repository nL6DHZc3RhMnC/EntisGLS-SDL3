
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/EntisGLS4_SceneSoundItem.h>


// SceneSoundItem( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSoundItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneSoundItem, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
					new S3DSoundItemSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneSoundItem.Instance createInstance( EntisGLS4.SceneSoundItem.InstanceFlag nFlags, double fpSubVolume, EntisGLS4.SceneItem refItem, const Matrix3d* matrix, const Vector3d* pos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_createInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;
	LQT_FUNC_ARG_DOUBLE( fpSubVolume ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, refItem ) ;
	S3DSceneComposer::ItemSerializer *	pRefItem = nullptr ;
	if ( refItem != nullptr )
	{
		pRefItem = refItem->GetRef<S3DSceneComposer::ItemSerializer>() ;
	}
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;

	S3DSoundItemSerializer::Instance *	pInstance =
		pSound->CreateInstance( nFlags, fpSubVolume, pRefItem, matrix, pos ) ;
	if ( pSound == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSoundItem.Instance) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSoundItem_Instance>(pInstance) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeInstance( EntisGLS4.SceneSoundItem.Instance instance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_removeInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	S3DSoundItemSerializer::Instance *	pInstance = instance->GetRef<S3DSoundItemSerializer::Instance>() ;
	LQT_VERIFY_NULL_PTR( pInstance ) ;

	pSound->RemoveInstance( pInstance ) ;

	LQT_RETURN_VOID() ;
}

// void playInstance( EntisGLS4.SceneSoundItem.Instance instance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_playInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	S3DSoundItemSerializer::Instance *	pInstance = instance->GetRef<S3DSoundItemSerializer::Instance>() ;
	LQT_VERIFY_NULL_PTR( pInstance ) ;

	pSound->PlayInstance( pInstance ) ;

	LQT_RETURN_VOID() ;
}

// boolean isValidInstance( EntisGLS4.SceneSoundItem.Instance instance ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_isValidInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	S3DSoundItemSerializer::Instance *	pInstance = instance->GetRef<S3DSoundItemSerializer::Instance>() ;
	LQT_VERIFY_NULL_PTR( pInstance ) ;

	LQT_RETURN_BOOL( pSound->IsValidInstance( pInstance ) ) ;
}

// void setInstancePosition( EntisGLS4.SceneSoundItem.Instance instance, const Vector3d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_setInstancePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	S3DSoundItemSerializer::Instance *	pInstance = instance->GetRef<S3DSoundItemSerializer::Instance>() ;
	LQT_VERIFY_NULL_PTR( pInstance ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	pSound->SetInstancePosition( pInstance, *vPos ) ;

	LQT_RETURN_VOID() ;
}

// boolean getPlayingTimeOfInstance( double* secPlaying, EntisGLS4.SceneSoundItem.Instance instance ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_getPlayingTimeOfInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	S3DSoundItemSerializer *	pSound = pThis->GetRef<S3DSoundItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pSound ) ;
	LQT_FUNC_ARG_POINTER( LDouble, secPlaying ) ;
	LQT_VERIFY_NULL_PTR( secPlaying ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	S3DSoundItemSerializer::Instance *	pInstance = instance->GetRef<S3DSoundItemSerializer::Instance>() ;
	LQT_VERIFY_NULL_PTR( pInstance ) ;

	LQT_RETURN_BOOL( pSound->GetPlayingTimeOfInstance( *secPlaying, pInstance ) ) ;
}



