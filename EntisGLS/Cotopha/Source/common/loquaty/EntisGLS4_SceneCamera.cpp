
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneCamera.h>


// SceneCamera( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneCamera, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
						new S3DSceneComposer::CameraSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;

	LVector3d	valRet = pCamera->GetCameraPosition() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraPosition( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	pCamera->SetCameraPosition( *vPos ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraTarget( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;

	LVector3d	valRet = pCamera->GetCameraTarget() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraTarget( const Vector3d* vTarget )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTarget ) ;
	LQT_VERIFY_NULL_PTR( vTarget ) ;

	pCamera->SetCameraTarget( *vTarget ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getCameraTop( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_getCameraTop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;

	LVector3d	valRet = pCamera->GetCameraTop() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setCameraTop( const Vector3d* vTop )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneCamera_setCameraTop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneCamera, pThis ) ;
	S3DScene::Camera *	pCamera = pThis->GetRef<S3DScene::Camera>() ;
	LQT_VERIFY_NULL_PTR( pCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTop ) ;
	LQT_VERIFY_NULL_PTR( vTop ) ;

	pCamera->SetCameraTop( *vTop ) ;

	LQT_RETURN_VOID() ;
}



