
#include <loquaty.h>
#include "EntisGLS4_SceneSoundItem.h"

using namespace Loquaty ;


// SceneSoundItem( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSoundItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneSoundItem, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneSoundItem.Instance createInstance( EntisGLS4.SceneSoundItem.InstanceFlag nFlags, double fpSubVolume, EntisGLS4.SceneItem refItem, const Matrix3d* matrix, const Vector3d* pos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_createInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;
	LQT_FUNC_ARG_DOUBLE( fpSubVolume ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, refItem ) ;
	LQT_VERIFY_NULL_PTR( refItem ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSoundItem.Instance) ) ) ;
	// valRet = pThis->createInstance(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneSoundItem_Instance> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSoundItem_Instance>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeInstance( EntisGLS4.SceneSoundItem.Instance instance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_removeInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;

	// pThis->removeInstance(...) ;

	LQT_RETURN_VOID() ;
}

// void playInstance( EntisGLS4.SceneSoundItem.Instance instance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_playInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;

	// pThis->playInstance(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isValidInstance( EntisGLS4.SceneSoundItem.Instance instance ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_isValidInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;

	LBoolean	valRet ;
	// valRet = pThis->isValidInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setInstancePosition( EntisGLS4.SceneSoundItem.Instance instance, const Vector3d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_setInstancePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	// pThis->setInstancePosition(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getPlayingTimeOfInstance( double* secPlaying, EntisGLS4.SceneSoundItem.Instance instance ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSoundItem_getPlayingTimeOfInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSoundItem, pThis ) ;
	LQT_FUNC_ARG_POINTER( LDouble, secPlaying ) ;
	LQT_VERIFY_NULL_PTR( secPlaying ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneSoundItem_Instance, instance ) ;
	LQT_VERIFY_NULL_PTR( instance ) ;

	LBoolean	valRet ;
	// valRet = pThis->getPlayingTimeOfInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



