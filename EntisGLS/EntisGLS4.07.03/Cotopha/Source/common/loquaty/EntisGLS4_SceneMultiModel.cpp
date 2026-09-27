
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/EntisGLS4_SceneMultiModel.h>


// SceneMultiModel( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneMultiModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneMultiModel, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
						new S3DMultiModelSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}



