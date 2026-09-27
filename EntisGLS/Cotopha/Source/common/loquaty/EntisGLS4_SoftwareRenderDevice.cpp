
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SoftwareRenderDevice.h>
#include <sakuragl/sgl3d/sgl_render_software_renderer.h>


// SoftwareRenderDevice( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SoftwareRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SoftwareRenderDevice, pThis,
			( new SSmartObject( new SGLSoftwareRenderDevice ) ) ) ;

	LQT_RETURN_VOID() ;
}



