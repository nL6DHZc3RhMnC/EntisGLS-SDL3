
#if	!defined(__SAKURAGL_SGL_OPENGL_WINDOW_IMPLEMENT_H__)
#define	__SAKURAGL_SGL_OPENGL_WINDOW_IMPLEMENT_H__	1

#include <sakuragl/sgl_opengl_window_producer.h>


namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 標準レンダリング実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLStandardRenderContext	: public S3DOpenGLBufferedRenderer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLStandardRenderContext, S3DOpenGLBufferedRenderer )
		// 構築関数
		SGLStandardRenderContext( void )
			: S3DOpenGLBufferedRenderer( SGLOpenGLContext::GetDefault() ) { }
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindow	: public SGLOpenGLWindow
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindow, SGLOpenGLWindow )
		// 構築関数
		SGLWindow( void ) : SGLOpenGLWindow( NULL ) { }
	} ;

}

#endif

