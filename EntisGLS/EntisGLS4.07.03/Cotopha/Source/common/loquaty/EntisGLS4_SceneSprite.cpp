
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneSprite.h>
#include <sakuraglx/render/sglx3d_scene_sprite.h>


// SceneSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneSprite, pThis,
			( new SSmartObject( (SGLSprite*) new S3DSceneSprite ) ) ) ;

	LQT_RETURN_VOID() ;
}



