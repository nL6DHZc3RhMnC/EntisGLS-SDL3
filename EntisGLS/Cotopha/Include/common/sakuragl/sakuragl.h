
#if	!defined(__SAKURAGL_H__)
#define	__SAKURAGL_H__	1

#include <sakura/sakura.h>

#if	!defined(SGL)
#define	SGL	SakuraGL
#endif

namespace	SakuraGL
{
	// 大域変数
	extern ESL_DLL_EXPORT atomic_int_t	g_countRefSakuraGL ;
	extern ESL_DLL_EXPORT SSystem::SSyncBufferedFile	cout ;
	extern ESL_DLL_EXPORT SSystem::SSyncBufferedFile	cin ;

	// 初期化
	void Initialize( void ) ;
	void Finalize( void ) ;
}

#include <sakuragl/sgl2d/sgl2d_stddef.h>

int sglMain( const wchar_t * pwszArg ) ;
SakuraGL::SGLError sglStaticInitialize( void ) ;
SakuraGL::SGLError sglStaticFinalize( void ) ;

#include <sakuragl/sgl3d_image.h>


#endif

