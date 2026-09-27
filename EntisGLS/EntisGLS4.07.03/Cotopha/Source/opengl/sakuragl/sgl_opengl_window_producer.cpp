
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_opengl_window_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 非レンダリング OpenGL スレッド開始プロシージャ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::ANRAttacherProc, SProcedure ) ;

SGLOpenGLWindowProducer::ANRAttacherProc::ANRAttacherProc
		( SGLOpenGLWindowProducer * pOpenGL, SGLAbstractWindow * pWnd )
	: m_pOpenGL( pOpenGL ), m_pWnd( pWnd )
{
}

void SGLOpenGLWindowProducer::ANRAttacherProc::Run( void )
{
	if ( m_pOpenGL->AttachGLCurrentANR() )
	{
		Trace( "Failed to AttachGLCurrentANR.\n" ) ;
	}
	else
	{
		Trace( " AttachGLCurrentANR on thead #%08X.\n", SThread::GetCurrentId() ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 非レンダリング OpenGL スレッド終了プロシージャ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::ANRDetacherProc, SProcedure ) ;

SGLOpenGLWindowProducer::ANRDetacherProc::ANRDetacherProc
		( SGLOpenGLWindowProducer * pOpenGL, SGLAbstractWindow * pWnd )
	: m_pOpenGL( pOpenGL ), m_pWnd( pWnd )
{
	m_signalDone.Initialize( false ) ;
}

void SGLOpenGLWindowProducer::ANRDetacherProc::Run( void )
{
	m_pOpenGL->DetachGLCurrentANR() ;
	//
	m_signalDone.SetSignal() ;
}

SSystem::SError SGLOpenGLWindowProducer::ANRDetacherProc::WaitDone( int64_t msecTimeout )
{
	SError	err = m_signalDone.Wait( msecTimeout ) ;
	if ( err == errSuccess )
	{
		m_signalDone.ResetSignal() ;
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL スレッド実行プロシージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::GLSyncProcedure, SSyncProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::GLSyncProcedure::GLSyncProcedure
			( SGLOpenGLWindowProducer * pOpenGL, SProcedure * pProc )
	: SSyncProcedure( pProc )
{
	m_pOpenGL = pOpenGL ;
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLSyncProcedure::Prepare( void )
{
	if ( m_pOpenGL->AttachGLCurrent() )
	{
		Trace( "Failed to AttachGLCurrent.\n" ) ;
	}
	SSyncProcedure::Prepare() ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLSyncProcedure::Finalize( void )
{
	SGLOpenGLWindowProducer *	pOpenGL = m_pOpenGL ;
	SSyncProcedure::Finalize() ;
	pOpenGL->DetachGLCurrent() ;
}


//////////////////////////////////////////////////////////////////////////////
// OpenGL スレッド非同期実行プロシージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::GLAsyncProcedure, SProcedure )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::GLAsyncProcedure::GLAsyncProcedure
	( SGLOpenGLWindowProducer * pOpenGL,
				SSystem::SProcedure * pProc )
{
	m_pOpenGL = pOpenGL ;
	m_pProc = pProc ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncProcedure::Run( void )
{
	if ( m_pProc != NULL )
	{
		m_pProc->Run() ;
	}
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncProcedure::Prepare( void )
{
	if ( m_pOpenGL->AttachGLCurrent() )
	{
		Trace( "Failed to AttachGLCurrent.\n" ) ;
	}
	if ( m_pProc != NULL )
	{
		m_pProc->Prepare() ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncProcedure::Finalize( void )
{
	SGLOpenGLWindowProducer *	pOpenGL = m_pOpenGL ;
	if ( m_pProc != NULL )
	{
		m_pProc->Finalize() ;
	}
	if ( AtomicSub( &(m_pOpenGL->m_countAsyncProcesures), 1 ) == 0 )
	{
		m_pOpenGL->m_signalAsyncProcedure.SetSignal() ;
	}
	pOpenGL->DetachGLCurrent() ;
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// 非レンダリング OpenGL スレッド非同期実行プロシージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::GLAsyncNoRenderProcedure, SProcedure )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::GLAsyncNoRenderProcedure::GLAsyncNoRenderProcedure
	( SGLOpenGLWindowProducer * pOpenGL,
		SGLAbstractWindow * pWnd, SSystem::SProcedure * pProc )
{
	m_pOpenGL = pOpenGL ;
	m_pWnd = pWnd ;
	m_pProc = pProc ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncNoRenderProcedure::Run( void )
{
	if ( m_pProc != NULL )
	{
		m_pProc->Run() ;
	}
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncNoRenderProcedure::Prepare( void )
{
	SGLWindowMonitorInterface *
		pWndMonitor = ESLTypeCast<SGLWindowMonitorInterface>( m_pWnd ) ;
	if ( pWndMonitor != nullptr )
	{
		double	msecPainting = 0.0 ;
		if ( pWndMonitor->IsWindowPainting( &msecPainting ) )
		{
			SleepMilliSec( 3 ) ;
		}
	}

	m_timer.Reset() ;

	if ( m_pOpenGL->AttachGLCurrentANR() )
	{
		Trace( "Failed to AttachGLCurrent.\n" ) ;
	}
	if ( m_pProc != NULL )
	{
		m_pProc->Prepare() ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLAsyncNoRenderProcedure::Finalize( void )
{
	SGLOpenGLWindowProducer *	pOpenGL = m_pOpenGL ;
	if ( m_pProc != NULL )
	{
		m_pProc->Finalize() ;
	}
	if ( AtomicSub( &(m_pOpenGL->m_countAsyncProcesures), 1 ) == 0 )
	{
		m_pOpenGL->m_signalAsyncProcedure.SetSignal() ;
	}
	pOpenGL->DetachGLCurrentANR() ;

	int64_t	msecPast = esl_lroundfi( m_timer.GetRealTime() ) ;
	if ( msecPast >= 1 )
	{
		SleepMilliSec( esl_min( (int) msecPast, 5 ) ) ;
	}
	//
	SGLWindowMonitorInterface *
		pWndMonitor = ESLTypeCast<SGLWindowMonitorInterface>( m_pWnd ) ;
	if ( pWndMonitor != nullptr )
	{
		double	msecPainting = 0.0 ;
		if ( pWndMonitor->IsWindowPainting( &msecPainting ) )
		{
			SleepMilliSec( 3 ) ;
		}
		else if ( msecPainting > 15.0 )
		{
			SleepMilliSec( 1 ) ;
		}
	}
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL 初期化プロシージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenGLWindowProducer::GLInitializeProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::GLInitializeProcedure::GLInitializeProcedure( SGLOpenGLWindowProducer * pOpenGL )
{
	m_pOpenGL = pOpenGL ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GLInitializeProcedure::Run( void )
{
	m_pOpenGL->OnCreateGLContext() ;
	m_pOpenGL->InitializeGLEX() ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL ウィンドウ・表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLOpenGLWindowProducer,
			SGLOpenGLContext, SGLWindowViewProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::SGLOpenGLWindowProducer( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	m_glrc.hWnd = NULL ;
	m_glrc.hDC = NULL ;
	m_glrc.hGLRC = NULL ;
	m_glrc.capsFlags = !g_disable_run_any_threads ? renderableAnyThread : 0 ;
	m_glrc.hPBuffer = NULL ;
	//
	m_flagAsyncNoRender = false ;
	//
	m_supports_ARB_pbuffer = false ;
	m_supports_ARB_pixel_format = false ;
	//
	wglDestroyPbufferARB = NULL ;
	wglQueryPbufferARB = NULL ;
	wglGetPbufferDCARB = NULL ;
	wglCreatePbufferARB = NULL ;
	wglReleasePbufferDCARB = NULL ;
	//
	wglChoosePixelFormatARB = NULL ;
	wglGetPixelFormatAttribfvARB = NULL ;
	wglGetPixelFormatAttribivARB = NULL ;
	//
#endif
	m_countAttached = 0 ;
	m_tidThreadID = SThread::InvalidId ;
	m_mutexGLThread.Initialize() ;

	m_countAttachedANR = 0 ;
	m_tidThreadIDANR = SThread::InvalidId ;

	m_signalAsyncProcedure.Initialize( true ) ;
	m_countAsyncProcesures = 0 ;

	m_pRenderer = new S3DOpenGLBufferedRenderer( this ) ;
	m_pDirectRenderer = new S3DOpenGLBufferedRenderer( this ) ;
	m_pglRenderer = &(m_pRenderer->GetDirectlyRenderer()) ;
	m_pglDirectRenderer = &(m_pDirectRenderer->GetDirectlyRenderer()) ;
	m_pglRenderer->SetFixViewport( true ) ;
	m_pglDirectRenderer->SetFixViewport( true ) ;

	AttachGLView( &(m_pglRenderer->m_glView) ) ;
	m_flagSupportedStereo3D = false ;
	m_flagQuadBuffer = false ;
	m_flagZBuffer = false ;
	m_flagLayeredWindow = false ;

	m_pFrameRenderer = new S3DOpenGLBufferedRenderer( this ) ;
	m_flagFlipFrame = false ;

	m_modeStereoView = stereoMonoView ;
	m_nStereoViewParam = 0 ;
	m_nStereoDrawFlags = 0 ;
	m_pglStereoViewShader = NULL ;
	m_fpLensScale = 1.0f ;
	m_fpLensOffsetX = 0.0f ;
	m_fpLensDistortion[0] = 1.1f ;
	m_fpLensDistortion[1] = -0.1f ;
	m_fpLensDistortion[2] = 0.0f ;
	m_fpLensDistortion[3] = 0.0f ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindowProducer::~SGLOpenGLWindowProducer( void )
{
	if ( m_refWindow.GetReference() != NULL )
	{
		DeleteGLContext() ;
	}
}

// OpenGL コンテキスト生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::CreateGLContext( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( (m_glrc.hWnd == NULL) || !::IsWindow( m_glrc.hWnd ) )
	{
		return	sglErrFailed ;
	}
	if ( m_glrc.hGLRC != NULL )
	{
		DeleteGLContext() ;
	}
	FormatDescription	fmtDesc ;
	fmtDesc.nFlags = 0 ;
	fmtDesc.nColorBits = 32 ;
	fmtDesc.nColorChannelBits = 8 ;
	fmtDesc.nAlphaBits = 8 ;
	fmtDesc.nDepthBits = 32 ;
	fmtDesc.nStencilBits = 0 ;
	//
	SGLError	err =
		CreateWindowGLContext( m_glrc, m_glrc.hWnd, fmtDesc ) ;
	if ( err )
	{
		WriteErrorLog
			( L"failed to create OpenGL context.\r\n"
				L"(color=%d/%d, alpha=%d, depth=%d, stencil=%d)\r\n\r\n",
				fmtDesc.nColorBits, fmtDesc.nColorChannelBits,
				fmtDesc.nAlphaBits, fmtDesc.nDepthBits, fmtDesc.nStencilBits ) ;
		//
		if ( m_glrc.hGLRC != NULL )
		{
			DeleteGLContext( m_glrc ) ;
		}
		fmtDesc.nColorBits = 16 ;
		fmtDesc.nColorChannelBits = 4 ;
		fmtDesc.nAlphaBits = 4 ;
		fmtDesc.nDepthBits = 16 ;
		SGLError	err =
			CreateWindowGLContext( m_glrc, m_glrc.hWnd, fmtDesc ) ;
		if ( err )
		{
			return	err ;
		}
	}
	if ( IsOpenedErrorLog() )
	{
		WriteErrorLog
			( L"succeeded to create OpenGL context.\r\n"
				L"(color=%d/%d, alpha=%d, depth=%d, stencil=%d)\r\n\r\n",
				fmtDesc.nColorBits, fmtDesc.nColorChannelBits,
				fmtDesc.nAlphaBits, fmtDesc.nDepthBits, fmtDesc.nStencilBits ) ;
		//
		if ( m_glrc.capsFlags & renderableAnyThread )
		{
			WriteErrorLog( L"renderable on any thread.\r\n" ) ;
		}
		else
		{
			WriteErrorLog( L"not renderable on any thread.\r\n" ) ;
		}
	}
#endif

	sglSetDefaultImageFormat( formatImageABGR, 32 ) ;
	S3DRenderContextInterface::SetDefaultRenderType( typePaintOpenGL ) ;

#if	defined(__PLATFORM_ANDROID__)
	GLInitializeProcedure	procInit( this ) ;
	Procedure( &procInit, procedureSync ) ;
#else
	AttachGLCurrent() ;
	InitializeGLEX() ;
	if ( m_supports_ARB_pixel_format )
	{
		// より良いピクセルフォーマットの求めて…
		const int	nEntryCount = 64 ;
		int	formatEntries[nEntryCount] ;
		int	nPixelFormat =
			ChooseWindowPixelFormatARB
				( m_glrc.hDC, formatEntries, nEntryCount ) ;
		if ( (nPixelFormat != 0)
			&& (m_glrc.nPixelFormat != nPixelFormat) )
		{
			const int	nSafePixelFormat = m_glrc.nPixelFormat ;
			DetachGLCurrent() ;
			DestroySecondaryGLContext( m_glrc ) ;
			//
			bool	fSucceeded = false ;
			for ( int i = 0; i < nEntryCount; i ++ )
			{
				if ( formatEntries[i] == 0 )
				{
					break ;
				}
				m_glrc.hDC = ::GetDC( m_glrc.hWnd ) ;
				if ( CreateWindowGLContextAs
						( m_glrc, formatEntries[i], NULL ) )
				{
					DestroySecondaryGLContext( m_glrc ) ;
				}
				else
				{
					fSucceeded = true ;
					break ;
				}
			}
			if ( !fSucceeded )
			{
				m_glrc.hDC = ::GetDC( m_glrc.hWnd ) ;
				if ( CreateWindowGLContextAs( m_glrc, nSafePixelFormat, NULL ) )
				{
					DestroySecondaryGLContext( m_glrc ) ;
					return	sglErrFailed ;
				}
			}
			AttachGLCurrent() ;
			InitializeGLEX() ;
		}
	}
	OnCreateGLContext() ;
	DetachGLCurrent() ;

	if ( !CreateARBBuffer( m_glrcAsyncNoRender, 8, 8 ) )
	{
		if ( !wglShareLists( m_glrc.hGLRC, m_glrcAsyncNoRender.hGLRC ) )
		{
			ESLTrace( "failed to wglShareLists.\n" ) ;
			DeleteGLContext( m_glrcAsyncNoRender ) ;
			//
			if ( IsOpenedErrorLog() )
			{
				WriteErrorLog( L"no secondary GL context.\r\n" ) ;
			}
		}
		else
		{
			if ( IsOpenedErrorLog() )
			{
				WriteErrorLog( L"has secondary GL context.\r\n" ) ;
			}
			m_flagAsyncNoRender = true ;
		}
	}
#endif

	return	sglErrSuccess ;
}

// 拡張 API 準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::InitializeGLEX( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	// WGL_ARB_pbuffer.
	wglDestroyPbufferARB =
		(API_DESTROYPBUFFERARBPROC)
			wglGetProcAddress( "wglDestroyPbufferARB" ) ;
	wglQueryPbufferARB =
		(API_QUERYPBUFFERARBPROC)
			wglGetProcAddress( "wglQueryPbufferARB" ) ;
	wglGetPbufferDCARB =
		(API_GETPBUFFERDCARBPROC)
			wglGetProcAddress( "wglGetPbufferDCARB" ) ;
	wglCreatePbufferARB =
		(API_CREATEPBUFFERARBPROC)
			wglGetProcAddress( "wglCreatePbufferARB" ) ;
	wglReleasePbufferDCARB =
		(API_RELEASEPBUFFERDCARBPROC)
			wglGetProcAddress( "wglReleasePbufferDCARB" ) ;
	m_supports_ARB_pbuffer =
		wglDestroyPbufferARB
			&& wglQueryPbufferARB
			&& wglGetPbufferDCARB
			&& wglCreatePbufferARB
			&& wglReleasePbufferDCARB ;

	// WGL_ARB_pixel_format.
	wglChoosePixelFormatARB =
		(API_CHOOSEPIXELFORMATARBPROC)
			wglGetProcAddress( "wglChoosePixelFormatARB" ) ;
	wglGetPixelFormatAttribfvARB =
		(API_GETPIXELFORMATATTRIBFVARBPROC)
			wglGetProcAddress( "wglGetPixelFormatAttribfvARB" ) ;
	wglGetPixelFormatAttribivARB =
		(API_GETPIXELFORMATATTRIBIVARBPROC )
			wglGetProcAddress( "wglGetPixelFormatAttribivARB" ) ;
	m_supports_ARB_pixel_format =
		wglChoosePixelFormatARB
			&& wglGetPixelFormatAttribfvARB
			&& wglGetPixelFormatAttribivARB ;
#endif
	return	sglErrSuccess ;
}

// レイヤードウィンドウ用コンテキスト生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::CreateARBBuffer( int nWidth, int nHeight )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( (m_glrc.sizePBuffer.w == nWidth) && (m_glrc.sizePBuffer.h == nHeight) )
	{
		return	sglErrSuccess ;
	}
	if ( !m_supports_ARB_pbuffer || !m_supports_ARB_pixel_format )
	{
		return	sglErrNotSupported ;
	}

	OGLRenderingContext	glrc ;
	SGLError	err = CreateARBBuffer( glrc, nWidth, nHeight ) ;
	if ( err )
	{
		return	err ;
	}
	DeleteGLContext() ;
	m_glrc = glrc ;
	//
	AttachGLCurrent() ;
	OnCreateGLContext() ;
	DetachGLCurrent() ;
	//
	return	sglErrSuccess ;
#else
	return	sglErrFailed ;
#endif
}

#if	defined(__PLATFORM_WINDOWS__)
SGLError SGLOpenGLWindowProducer::CreateARBBuffer
	( OGLRenderingContext& glrc, int nWidth, int nHeight )
{
	if ( AttachGLCurrent() )
	{
		return	sglErrFailed ;
	}

	int attribList[] =
	{
		WGL_DRAW_TO_PBUFFER_ARB,	TRUE,	// allow rendering to the pbuffer
		WGL_SUPPORT_OPENGL_ARB,		TRUE,	// associate with OpenGL
		WGL_DOUBLE_BUFFER_ARB,		FALSE,	// single buffered
		WGL_RED_BITS_ARB,			8,		// minimum 8-bits for red channel
		WGL_GREEN_BITS_ARB,			8,		// minimum 8-bits for green channel
		WGL_BLUE_BITS_ARB,			8,		// minimum 8-bits for blue channel
		WGL_ALPHA_BITS_ARB,			8,		// minimum 8-bits for alpha channel
		WGL_DEPTH_BITS_ARB,			16,		// minimum 16-bits for depth buffer
		0
	} ;
	int		format = 0 ;
	UINT	matchingFormats = 0 ;
	if ( !wglChoosePixelFormatARB
		( m_glrc.hDC, attribList, 0, 1, &format, &matchingFormats ) )
	{
		DetachGLCurrent() ;
		return	sglErrFailed ;
	}
	HPBUFFERARB	hPBuf =
		wglCreatePbufferARB( m_glrc.hDC, format, nWidth, nHeight, 0 ) ;
	if ( hPBuf == NULL )
	{
		DetachGLCurrent() ;
		return	sglErrFailed ;
	}
	HDC	hDC = wglGetPbufferDCARB( hPBuf ) ;
	if ( hDC == NULL )
	{
		wglDestroyPbufferARB( hPBuf ) ;
		DetachGLCurrent() ;
		return	sglErrFailed ;
	}
	HGLRC	hGLRC = wglCreateContext( hDC ) ;
	if ( hGLRC == NULL )
	{
		wglReleasePbufferDCARB( hPBuf, hDC ) ;
		wglDestroyPbufferARB( hPBuf ) ;
		DetachGLCurrent() ;
		return	sglErrFailed ;
	}
	DetachGLCurrent() ;

	glrc.hDC = hDC ;
	glrc.hGLRC = hGLRC ;
	glrc.hPBuffer = hPBuf ;
	glrc.sizePBuffer.w = nWidth ;
	glrc.sizePBuffer.h = nHeight ;

	return	sglErrSuccess ;
}
#endif

// OpenGL コンテキスト破棄
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DeleteGLContext( void )
{
	if ( !AttachGLCurrent() )
	{
		OnDestroyGLContext() ;
		DetachGLCurrent() ;
	}
	ESLAssert( m_countAttached == 0 ) ;

#if	defined(__PLATFORM_WINDOWS__)
	if ( m_flagAsyncNoRender )
	{
		DeleteGLContext( m_glrcAsyncNoRender ) ;
		m_flagAsyncNoRender = false ;
	}
	DeleteGLContext( m_glrc ) ;
	return	sglErrSuccess ;
#else
	return	sglErrSuccess ;
#endif
}

#if	defined(__PLATFORM_WINDOWS__)
void SGLOpenGLWindowProducer::DeleteGLContext
		( SGLOpenGLWindowProducer::OGLRenderingContext& glrc ) const
{
	if ( glrc.hGLRC != NULL )
	{
		wglMakeCurrent( glrc.hDC, NULL ) ;
		wglDeleteContext( glrc.hGLRC ) ;
		glrc.hGLRC = NULL ;
	}
	if ( glrc.hPBuffer != NULL )
	{
		if ( glrc.hDC != NULL )
		{
			wglReleasePbufferDCARB( glrc.hPBuffer, glrc.hDC ) ;
			glrc.hDC = NULL ;
		}
		wglDestroyPbufferARB( glrc.hPBuffer ) ;
		glrc.hPBuffer = NULL ;
	}
	else
	{
		if ( glrc.hDC != NULL )
		{
			::ReleaseDC( glrc.hWnd, glrc.hDC ) ;
			glrc.hDC = NULL ;
		}
	}
}
#endif

// OpenGL コンテキストをスレッドへ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::AttachGLCurrent( void )
{
	if ( m_mutexGLThread.LockTrace( __FILE__, __LINE__, 0 ) == errSuccess )
	{
		if ( m_countAttached == 0 )
		{
			#if	defined(__PLATFORM_WINDOWS__)
				if ( (m_glrc.hDC == NULL) || (m_glrc.hGLRC == NULL) )
				{
					m_mutexGLThread.Unlock() ;
					return	sglErrFailed ;
				}
				if ( !wglMakeCurrent( m_glrc.hDC, m_glrc.hGLRC ) )
				{
					m_mutexGLThread.Unlock() ;
					ESLTrace( "failed to wglMakeCurrent as attach to thread.\n" ) ;
					return	sglErrFailed ;
				}
			#endif
			//
			m_tidThreadID = SThread::GetCurrentId() ;
			m_countAttached = 1 ;
			//
//			AttachRenderContext( NULL ) ;
//			InitMaterialSetting() ;
		}
		else
		{
			m_countAttached ++ ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// OpenGL コンテキストをスレッドから解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DetachGLCurrent( void )
{
	if ( m_countAttached != 0 )
	{
		if ( -- m_countAttached == 0 )
		{
			m_tidThreadID = SThread::InvalidId ;
			//
			#if	defined(__PLATFORM_WINDOWS__)
				//
				if ( m_glrc.hDC == NULL )
				{
					m_mutexGLThread.Unlock() ;
					return	sglErrFailed ;
				}
				if ( !wglMakeCurrent( m_glrc.hDC, NULL ) )
				{
					m_mutexGLThread.Unlock() ;
					ESLTrace( "failed to wglMakeCurrent as detach from thread.\n" ) ;
					return	sglErrFailed ;
				}
			#endif
			//
			AttachGLView( NULL ) ;
		}
		m_mutexGLThread.Unlock() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 非レンダリング用 OpenGL コンテキストをスレッドへ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::AttachGLCurrentANR( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( !m_flagAsyncNoRender )
	{
		return	AttachGLCurrent() ;
	}
#endif

	m_mutexGLThreadANR.Lock() ;

	if ( m_countAttachedANR == 0 )
	{
		#if	defined(__PLATFORM_WINDOWS__)
		if ( !wglMakeCurrent( m_glrcAsyncNoRender.hDC, m_glrcAsyncNoRender.hGLRC ) )
		{
			m_mutexGLThreadANR.Unlock() ;
			ESLTrace( "failed to wglMakeCurrent as attach to thread.\n" ) ;
			return	sglErrFailed ;
		}
		#endif

		m_tidThreadIDANR = SThread::GetCurrentId() ;
		m_countAttachedANR = 1 ;
	}
	else
	{
		m_countAttachedANR ++ ;
	}
	return	sglErrSuccess ;
}

// 非レンダリング用 OpenGL コンテキストをスレッドから解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DetachGLCurrentANR( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( !m_flagAsyncNoRender )
	{
		return	DetachGLCurrent() ;
	}
#endif
	if ( m_countAttachedANR == 0 )
	{
		return	sglErrFailed ;
	}
	if ( -- m_countAttachedANR == 0 )
	{
		#if	defined(__PLATFORM_WINDOWS__)
		if ( !wglMakeCurrent( m_glrcAsyncNoRender.hDC, NULL ) )
		{
			m_mutexGLThreadANR.Unlock() ;
			ESLTrace( "failed to wglMakeCurrent as detach from thread.\n" ) ;
			return	sglErrFailed ;
		}
		#endif

		m_tidThreadIDANR = SThread::InvalidId ;
	}
	m_mutexGLThreadANR.Unlock() ;
	return	sglErrSuccess ;
}

// 初期処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnCreateGLContext( void )
{
	SGLOpenGLContext::OnCreateGLContext() ;
	//
	if ( IsOpenedErrorLog() )
	{
		WriteErrorLog( L"\r\nOpenGL version: %d.%d\r\n", m_versionGL[0], m_versionGL[1] ) ;
		WriteErrorLog( L"max multi texture units: %d\r\n", m_maxMultiTextureUnits ) ;
		WriteErrorLog( L"max texture images: %d\r\n", m_maxTextureImages ) ;
		WriteErrorLog( L"max vertex shader texture images: %d\r\n", m_maxVSTextureImages ) ;
		WriteErrorLog( L"max combined texture images: %d\r\n", m_maxCombinedTextureImages ) ;
		WriteErrorLog( L"max texture size: %d\r\n", m_maxTextureSize ) ;
		WriteErrorLog( L"max 3D texture size: %d\r\n", m_max3DTextureSize ) ;
		WriteErrorLog( L"max Cubemap texture size: %d\r\n", m_maxCubemapTextureSize ) ;
		WriteErrorLog( L"max vertex attributes: %d\r\n", m_maxVertexAttributes ) ;
		WriteErrorLog( L"max varying vectors: %d\r\n", m_maxVertexVaryings ) ;
		WriteErrorLog( L"max vertex uniform vectors: %d\r\n", m_maxVertexUniforms ) ;
		WriteErrorLog( L"max fragment uniform vectors: %d\r\n", m_maxFragmentUniforms ) ;
		WriteErrorLog( L"max draw buffers: %d\r\n", m_maxDrawBuffers ) ;
		WriteErrorLog( L"max color attachments: %d\r\n", m_maxColorAttachments ) ;
		WriteErrorLog( L"supported VAO: %d\r\n", m_flagSupportedVAO ) ;
		WriteErrorLog( L"supported texture non power of 2: %d\r\n", m_flagTextureNonPowerOf2 ) ;
		WriteErrorLog( L"supported depth texture: %d\r\n", m_flagDepthTexture ) ;
		WriteErrorLog( L"supported multi sampling: %d\r\n", m_flagMultisampling ) ;
		WriteErrorLog( L"supported cube texture: %d\r\n", m_flagCubemapTexture ) ;
		WriteErrorLog( L"supported texture compression s3tc: %d\r\n", m_flagCompressionS3TC ) ;
		WriteErrorLog( L"supported element index uint: %d\r\n", m_flagElementIndexUint ) ;
		WriteErrorLog( L"supported texture float: %d\r\n", m_flagTextureFloat ) ;
		WriteErrorLog( L"supported color buffer float: %d\r\n", m_flagColorBufferFloat ) ;
		WriteErrorLog( L"supported geometry shader: %d\r\n", m_flagSupportedGeometry ) ;
		WriteErrorLog( L"supported compute shader: %d\r\n", m_flagSupportedComputeShader ) ;
	}
}

// 破棄前処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnDestroyGLContext( void )
{
	SGLOpenGLContext::OnDestroyGLContext() ;
}

#if	defined(__PLATFORM_WINDOWS__)
// ウィンドウ用コンテキスト生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::CreateWindowGLContext
	( SGLOpenGLWindowProducer::OGLRenderingContext& glrc, HWND hWnd,
		const SGLOpenGLWindowProducer::FormatDescription& fmtDesc ) const
{
	if ( m_glrc.hGLRC != NULL )
	{
		return	sglErrFailed ;
	}
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Specifies the size of this data structure
		1,								// Specifies the version of this data structure
		PFD_DRAW_TO_WINDOW |			// ピクセルバッファのビットフラグの設定
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,					// RGBA pixel values
		(BYTE) fmtDesc.nColorBits,		// 32-bitカラーと指定
		(BYTE) fmtDesc.nColorChannelBits, 0,	// Specifies the number of red bitplanes in each RGBA color buffer
		(BYTE) fmtDesc.nColorChannelBits, 0,
		(BYTE) fmtDesc.nColorChannelBits, 0,
		(BYTE) fmtDesc.nAlphaBits, 0,	// Specifies the number of alpha bitplanes in each RGBA color buffer
		0, 0, 0, 0, 0,					// Specifies the total number of bitplanes in the accumulation buffer
		(BYTE) fmtDesc.nDepthBits,		// Specifies the depth(bit) of the depth (z-axis) buffer
		(BYTE) fmtDesc.nStencilBits,	// Specifies the depth of the stencil buffer
		0,								// Specifies the number of auxiliary buffers
		PFD_MAIN_PLANE,					// Layer type　Ignored...
		0,								// Specifies the number of overlay and underlay planes
		0,								// Ignored
		0,								// Specifies the transparent color or index of an underlay plane
		0								// Ignored
	} ;
	if ( fmtDesc.nFlags & formatStereo )
	{
		pfd.dwFlags |= PFD_STEREO ;
	}
	ESLAssert( glrc.hDC == NULL ) ;
	glrc.hDC = ::GetDC( hWnd ) ;
	//
	int	nPixelFormat = ChoosePixelFormat( glrc.hDC, &pfd ) ;
	if ( nPixelFormat == 0 )
	{
		ESLTrace( "failed to OpenGL ChoosePixelFormat.\n" ) ;
		return	sglErrFailed ;
	}
	return	CreateWindowGLContextAs( glrc, nPixelFormat, &pfd ) ;
}

SGLError SGLOpenGLWindowProducer::CreateWindowGLContextAs
	( SGLOpenGLWindowProducer::OGLRenderingContext& glrc,
			int nPixelFormat, const PIXELFORMATDESCRIPTOR * ppfd ) const
{
	if ( glrc.hGLRC != NULL )
	{
		return	sglErrFailed ;
	}
	glrc.hGLRC = NULL ;
	glrc.capsFlags = !g_disable_run_any_threads ? renderableAnyThread : 0 ;
	glrc.nPixelFormat = 0 ;
	//
	SGLError	err =
		SetPixelFormatGLContext( glrc, nPixelFormat, ppfd ) ;
	if ( err )
	{
		return	err ;
	}
	glrc.hGLRC = wglCreateContext( glrc.hDC ) ;
	if ( glrc.hGLRC == NULL )
	{
		ESLTrace( "failed to wglCreateContext.\n" ) ;
		return	sglErrFailed ;
	}
	glrc.nPixelFormat = nPixelFormat ;
	return	sglErrSuccess ;
}

// ARGB ピクセルフォーマット選択
//////////////////////////////////////////////////////////////////////////////
int SGLOpenGLWindowProducer::ChooseWindowPixelFormatARB
	( HDC hdc, int * pFormats, size_t nCount ) const
{
	if ( !m_supports_ARB_pixel_format )
	{
		return	0 ;
	}
	FormatDescription	fmtDesc ;
	fmtDesc.nFlags = formatStereo | formatSRGB ;
	fmtDesc.nColorBits = 32 ;
	fmtDesc.nColorChannelBits = 8 ;
	fmtDesc.nAlphaBits = 8 ;
	fmtDesc.nDepthBits = 32 ;
	fmtDesc.nStencilBits = 0 ;
	//
	int	nPixelFormat = 0 ;
	do
	{
		nPixelFormat =
			ChooseWindowPixelFormatARBAs
				( hdc, pFormats, nCount, fmtDesc ) ;
		if ( nPixelFormat != 0 )
		{
			// QuadBuffer, RGBA 32bit, depth 32bit
			break ;
		}
		fmtDesc.nFlags &= ~formatStereo ;
		nPixelFormat =
			ChooseWindowPixelFormatARBAs
				( hdc, pFormats, nCount, fmtDesc ) ;
		if ( nPixelFormat != 0 )
		{
			// RGBA 32bit, depth 32bit
			break ;
		}
		fmtDesc.nDepthBits = 24 ;
		nPixelFormat =
			ChooseWindowPixelFormatARBAs
				( hdc, pFormats, nCount, fmtDesc ) ;
		if ( nPixelFormat != 0 )
		{
			// RGBA 32bit, depth 24bit
			break ;
		}
		fmtDesc.nDepthBits = 16 ;
		nPixelFormat =
			ChooseWindowPixelFormatARBAs
				( hdc, pFormats, nCount, fmtDesc ) ;
		if ( nPixelFormat != 0 )
		{
			// RGBA 32bit, depth 16bit
			break ;
		}
	}
	while ( false ) ;
	return	nPixelFormat ;
}

int SGLOpenGLWindowProducer::ChooseWindowPixelFormatARBAs
	( HDC hdc, int * pFormats, size_t nCount,
		const SGLOpenGLWindowProducer::FormatDescription& fmtDesc ) const
{
	if ( !m_supports_ARB_pixel_format )
	{
		return	0 ;
	}
	int attribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,		TRUE,	// allow rendering to the pbuffer
		WGL_SUPPORT_OPENGL_ARB,		TRUE,	// associate with OpenGL
		WGL_DOUBLE_BUFFER_ARB,		TRUE,	// single buffered
		WGL_STEREO_ARB,				(fmtDesc.nFlags & formatStereo) ? TRUE : FALSE,
		WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,			(int) fmtDesc.nColorBits,
		WGL_RED_BITS_ARB,			(int) fmtDesc.nColorChannelBits,	// minimum for red channel
		WGL_GREEN_BITS_ARB,			(int) fmtDesc.nColorChannelBits,	// minimum for green channel
		WGL_BLUE_BITS_ARB,			(int) fmtDesc.nColorChannelBits,	// minimum for blue channel
		WGL_ALPHA_BITS_ARB,			(int) fmtDesc.nAlphaBits,			// minimum for alpha channel
		WGL_DEPTH_BITS_ARB,			(int) fmtDesc.nDepthBits,			// minimum for depth buffer
		WGL_STENCIL_BITS_ARB,		(int) fmtDesc.nStencilBits,			// minimum for stencil buffer
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB,
									(fmtDesc.nFlags & formatSRGB) ? TRUE : FALSE,
		0
	} ;
	int		format[64] ;
	UINT	matchingFormats = 0 ;
	eslFillMemory( format, 0, sizeof(format) ) ;
	//
	if ( wglChoosePixelFormatARB
			( m_glrc.hDC, attribList, 0,
				sizeof(format)/sizeof(format[0]),
				&format[0], &matchingFormats )
		&& (format[0] != 0) && (matchingFormats >= 1) )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pFormats[i] = (i < matchingFormats) ? format[i] : 0 ;
		}
		return	format[0] ;
	}
	return	0 ;
}

// ピクセルフォーマット設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::SetPixelFormatGLContext
	( SGLOpenGLWindowProducer::OGLRenderingContext& glrc,
				int nFormat, const PIXELFORMATDESCRIPTOR * ppfd ) const
{
	if ( !SetPixelFormat( glrc.hDC, nFormat, ppfd ) )
	{
		ESLTrace( "Failed to OpenGL SetPixelFormat.\n" ) ;
		return	sglErrFailed ;
	}
	PIXELFORMATDESCRIPTOR	pfd ;
	if ( DescribePixelFormat
		( glrc.hDC, nFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd ) == 0 )
	{
		ESLTrace( "failed to OpenGL DescribePixelFormat.\n" ) ;
		return	sglErrFailed ;
	}
	if ( pfd.dwFlags
		& (PFD_SWAP_EXCHANGE | PFD_SWAP_COPY | PFD_SWAP_LAYER_BUFFERS) )
	{
		glrc.capsFlags |= renderableAnyThread ;
	}
	else
	{
		ESLTrace( "OpenGL no renderable any threads.\n" ) ;
		glrc.capsFlags &= ~renderableAnyThread ;
	}
	return	sglErrSuccess ;
}

// 別ウィンドウ用コンテキスト生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::CreateSecondaryGLContext
	( SGLOpenGLWindowProducer::OGLRenderingContext& glrc, HWND hWnd,
		const SGLOpenGLWindowProducer::FormatDescription& fmtDesc ) const
{
	SGLError	err = CreateWindowGLContext( glrc, hWnd, fmtDesc ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !wglShareLists( m_glrc.hGLRC, glrc.hGLRC ) )
	{
		ESLTrace( "failed to wglShareLists.\n" ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 別ウィンドウ用コンテキスト破棄
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DestroySecondaryGLContext
	( SGLOpenGLWindowProducer::OGLRenderingContext& glrc ) const
{
	if ( glrc.hGLRC != NULL )
	{
		wglMakeCurrent( glrc.hDC, NULL ) ;
		wglDeleteContext( glrc.hGLRC ) ;
		glrc.hGLRC = NULL ;
	}
	if ( glrc.hDC != NULL )
	{
		::ReleaseDC( glrc.hWnd, glrc.hDC ) ;
		glrc.hDC = NULL ;
	}
	return	sglErrSuccess ;
}

// OpenGL コンテキストをスレッドへ関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::AttachSecondaryGLCurrent
	( const SGLOpenGLWindowProducer::OGLRenderingContext& glrc )
{
	if ( m_mutexGLThread.LockTrace( __FILE__, __LINE__, 0 ) == errSuccess )
	{
		if ( m_countAttached == 0 )
		{
			if ( (glrc.hDC == NULL) || (glrc.hGLRC == NULL) )
			{
				m_mutexGLThread.Unlock() ;
				return	sglErrFailed ;
			}
			if ( !wglMakeCurrent( glrc.hDC, glrc.hGLRC ) )
			{
				m_mutexGLThread.Unlock() ;
				ESLTrace( "failed to wglMakeCurrent as attach to thread.\n" ) ;
				return	sglErrFailed ;
			}
			//
			m_tidThreadID = SThread::GetCurrentId() ;
			m_countAttached = 1 ;
			//
			AttachRenderContext( NULL ) ;
			InitMaterialSetting() ;
		}
		else
		{
			m_countAttached ++ ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// OpenGL コンテキストをスレッドから解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DetachSecondaryGLCurrent
	( const SGLOpenGLWindowProducer::OGLRenderingContext& glrc )
{
	if ( m_countAttached != 0 )
	{
		if ( -- m_countAttached == 0 )
		{
			m_tidThreadID = SThread::InvalidId ;
			//
			if ( glrc.hDC == NULL )
			{
				m_mutexGLThread.Unlock() ;
				return	sglErrFailed ;
			}
			if ( !wglMakeCurrent( glrc.hDC, NULL ) )
			{
				m_mutexGLThread.Unlock() ;
				ESLTrace( "failed to wglMakeCurrent as detach from thread.\n" ) ;
				return	sglErrFailed ;
			}
			//
			AttachGLView( NULL ) ;
		}
		m_mutexGLThread.Unlock() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
const SGLOpenGLWindowProducer::OGLRenderingContext&
		SGLOpenGLWindowProducer::GetRenderingContext( void ) const
{
	return	m_glrc ;
}

#endif

// ソフトウェアステレオビュー用バッファサイズ
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::ResizeStereoViewBuffer( void )
{
	Lock() ;
	//
	SGLSize	sizeView = m_pglRenderer->m_glView.m_sizePhysical ;
	if ( m_imgRightFrameColor.GetImageSize() != sizeView )
	{
		m_imgRightFrameColor.CreateImage
			( sizeView.w, sizeView.h, formatImageABGR,
				32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( m_imgLeftFrameColor.GetImageSize() != sizeView )
	{
		m_imgLeftFrameColor.CreateImage
			( sizeView.w, sizeView.h, formatImageABGR,
				32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( m_imgRightFrameDepth.GetImageSize() != sizeView )
	{
		m_imgRightFrameDepth.CreateImage
			( sizeView.w, sizeView.h, formatImageDepth,
				32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( m_imgLeftFrameDepth.GetImageSize() != sizeView )
	{
		m_imgLeftFrameDepth.CreateImage
			( sizeView.w, sizeView.h, formatImageDepth,
				32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( m_modeStereoView == stereoSideBySide )
	{
		UpdateSideBySideViewParam() ;
	}
	//
	Unlock() ;
}

// ステレオ立体視モード終了処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::FinalizeStereoDisplayMode( void )
{
/*
	if ( m_flagQuadBuffer )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			SSystem::LockTrace( __FILE__, __LINE__ ) ;
			if ( (m_hWnd == NULL) || !::IsWindow( m_hWnd ) )
			{
				m_flagQuadBuffer = false ;
			}
			else
			{
				DeleteGLContext() ;
				m_flagQuadBuffer = false ;
				CreateGLContext() ;
			}
			SSystem::Unlock() ;
		#else
			m_flagQuadBuffer = false ;
		#endif
	}
*/
	m_modeStereoView = stereoMonoView ;
	m_pglRenderer->m_glView.SetPhysicalViewSize
			( m_pglDirectRenderer->m_glView.m_sizePhysical ) ;
}

// OpenGL スレッドか判定
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::IsOnRenderThread( void )
{
	if ( m_countAttached && (SThread::GetCurrentId() == m_tidThreadID) )
	{
		return	true ;
	}
	if ( m_countAttachedANR && (SThread::GetCurrentId() == m_tidThreadIDANR) )
	{
		return	true ;
	}
	return	false ;
}

bool SGLOpenGLWindowProducer::IsOnAsyncNoRenderThread( void ) const
{
	if ( m_countAttachedANR && (SThread::GetCurrentId() == m_tidThreadIDANR) )
	{
		return	true ;
	}
	return	false ;
}

// OpenGL スレッドで実行する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::Procedure
	( SSystem::SProcedure* pProc, ProcedurePriority priority )
{
	if ( pProc == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (priority < procedureLater) && IsOnRenderThread() )
	{
		// 既にスレッドに関連付けられている
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
		return	sglErrSuccess ;
	}
	else
	{
		if ( m_flagInSuitableProcedure && (priority < procedureDelayable) )
		{
			if ( !ProcedureInSuitable( pProc, (priority == procedureSync) ) )
			{
				return	sglErrSuccess ;
			}
		}
		SGLAbstractWindow *	pWnd = m_refWindow ;
		if ( (GetCapacityFlags() & renderableAnyThread)
						&& (priority < procedureDelayable) )
		{
			// スレッドに関連付けて実行する
			SError	errLocked ;
			if ( pWnd != NULL )
			{
				errLocked = pWnd->LockTrace( __FILE__, __LINE__, 10 ) ;
			}
			else
			{
				errLocked = SSystem::LockTrace( __FILE__, __LINE__, 10 ) ;
			}
			if ( errLocked == errSuccess )
			{
				if ( !AttachGLCurrent() )
				{
					pProc->Prepare() ;
					pProc->Run() ;
					pProc->Finalize() ;
					//
					DetachGLCurrent() ;
					//
					if ( pWnd != NULL )
					{
						pWnd->Unlock() ;
					}
					else
					{
						SSystem::Unlock() ;
					}
					return	sglErrSuccess ;
				}
				ESLTrace( "failed to SGLOpenGLWindowProducer::AttachGLCurrent.\n" ) ;
				if ( pWnd != NULL )
				{
					pWnd->Unlock() ;
				}
				else
				{
					SSystem::Unlock() ;
				}
			}
			else
			{
				ESLTrace( "timeout SSystem::Lock for call procedure on OpenGL thread.\n" ) ;
			}
		}
		// Window フレームワークから呼び出す必要がある
		if ( pWnd != NULL )
		{
			if ( priority != procedureSync )
			{
				AtomicAdd( &m_countAsyncProcesures, 1 ) ;
				m_signalAsyncProcedure.ResetSignal() ;
				//
				#if	defined(__PLATFORM_WINDOWS__)
				if ( m_flagAsyncNoRender && (priority == procedureNoRender) )
				#else
				if ( priority == procedureNoRender )
				#endif
				{
					return	pWnd->PostRenderingThread
								( new GLAsyncNoRenderProcedure( this, pWnd, pProc ),
										SGLAbstractWindow::postAsyncNoRender ) ;
				}
				else
				{
					return	pWnd->PostRenderingThread
								( new GLAsyncProcedure( this, pProc ),
										((priority >= procedureDelayable)
											? SGLAbstractWindow::postDelay
											: SGLAbstractWindow::postNormal) ) ;
				}
			}
			GLSyncProcedure	proc( this, pProc ) ;
			if ( pWnd->PostRenderingThread
				( &proc, SGLAbstractWindow::postNormal ) == sglErrSuccess )
			{
				if ( proc.WaitDone(100) != errSuccess )
				{
					atomic_int_t	countLocked = pWnd->UnlockAll() ;
					proc.WaitDone() ;
					pWnd->Relock( countLocked ) ;
				}
				return	sglErrSuccess ;
			}
		}
	}
	return	sglErrFailed ;
}

// レンダリングスレッドでの遅延実行が全て完了するまで待機
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::WaitUntilAsyncAllProcedures( int64_t msecTimeout )
{
	return	(SGLError) m_signalAsyncProcedure.Wait( msecTimeout ) ;
}

// 対応機能フラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLOpenGLWindowProducer::GetCapacityFlags( void ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	return	m_glrc.capsFlags ;
#else
	return	0 ;
#endif
}

// 論理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnChangeVirtualViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView( nWidth, nHeight ) ;
	pWnd->Lock() ;
	m_pglRenderer->m_glView.SetVirtualViewSize( sizeView ) ;
	if ( m_modeStereoView == stereoSideBySide )
	{
		m_pglRenderer->m_glView.SetPhysicalViewSize( sizeView ) ;
		ResizeStereoViewBuffer() ;
	}
	pWnd->Unlock() ;
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView( nWidth, nHeight ) ;
	pWnd->Lock() ;
	if ( m_modeStereoView != stereoSideBySide )
	{
		m_pglRenderer->m_glView.SetPhysicalViewSize( sizeView ) ;
	}
	m_pglDirectRenderer->m_glView.SetVirtualViewSize( sizeView ) ;
	m_pglDirectRenderer->m_glView.SetPhysicalViewSize( sizeView ) ;
	//
	if ( m_modeStereoView != stereoMonoView )
	{
		ResizeStereoViewBuffer() ;
	}
	pWnd->Unlock() ;
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	if ( m_refWindow != NULL )
	{
		DeleteGLContext() ;
		AddToChain() ;
	}
	m_refWindow = pWnd ;
	//
#if	defined(__PLATFORM_WINDOWS__)
	m_glrc.hWnd = pWnd->GetWindowHandle() ;
#endif
	CreateGLContext() ;

#if	defined(__PLATFORM_WINDOWS__)
	if ( m_flagAsyncNoRender )
	{
		if ( m_pANRAttacherProc == nullptr )
		{
			m_pANRAttacherProc = new ANRAttacherProc( this, pWnd ) ;
		}
		if ( m_pANRDetacherProc == nullptr )
		{
			m_pANRDetacherProc = new ANRDetacherProc( this, pWnd ) ;
		}
		pWnd->PostRenderingThread
			( m_pANRAttacherProc, SGLAbstractWindow::postAsyncNoRender ) ;
		pWnd->PostRenderingThread
			( m_pANRDetacherProc, SGLAbstractWindow::postAsyncNoRenderFinally ) ;
	}
#endif
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( m_flagAsyncNoRender )
	{
		if ( m_pANRDetacherProc != nullptr )
		{
			if ( m_pANRDetacherProc->WaitDone(0) == errTimeout )
			{
				pWnd->PostRenderingThread
					( m_pANRDetacherProc, SGLAbstractWindow::postAsyncNoRender ) ;
				m_pANRDetacherProc->WaitDone( 1000 ) ;
			}
		}
	}
#endif
	//
	DeleteGLContext() ;
	m_refWindow = NULL ;
	//
#if	defined(__PLATFORM_WINDOWS__)
	m_glrc.hWnd = NULL ;
#endif
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnMovedWindow( SGLAbstractWindow * pWnd )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( m_modeStereoView == stereoInterleave )
	{
		HWND	hWnd = pWnd->GetWindowHandle() ;
		if ( hWnd != NULL )
		{
			POINT	ptClient = { 0, 0 } ;
			::ClientToScreen( hWnd, &ptClient ) ;
			//
			uint32_t	nDrawFlags =
				(m_nStereoViewParam & Window::stereoFlagSwapEyes) ;
			if ( m_nStereoViewParam & Window::stereoFlagVertical )
			{
				if ( ptClient.x & 0x01 )
				{
					nDrawFlags ^= Window::stereoFlagSwapEyes ;
				}
			}
			else
			{
				if ( ptClient.y & 0x01 )
				{
					nDrawFlags ^= Window::stereoFlagSwapEyes ;
				}
			}
			if ( m_nStereoDrawFlags != nDrawFlags )
			{
				m_nStereoDrawFlags = nDrawFlags ;
				//
				if ( m_pglStereoViewShader != NULL )
				{
					int32_t	nFlag =
						(m_nStereoDrawFlags
							& Window::stereoFlagSwapEyes) ? 1 : 0 ;
					m_pglStereoViewShader->
						SetCustomUniformIntAs( L"u_bOddLine", &nFlag, 1 ) ;
				}
				//
				pWnd->PostUpdate() ;
			}
		}
	}
#endif
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
}

// 論理ビュー表示座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GetInternalViewPosition( SGLImageRect& rctVirtualView )
{
	rctVirtualView = m_pglRenderer->m_glView.m_rectViewPort ;
}

// 物理ビュー表示領域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::GetExternalViewPosition( SGLImageRect& rctPhysicalView )
{
	if ( m_modeStereoView == stereoSideBySide )
	{
		rctPhysicalView = m_pglRenderer->m_glView.m_rectViewPort ;
		return	true ;
	}
	return	false ;
}

// 論理座標→物理ビュー座標変換行列取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::GetAffineVirtualToPhysical( SGLAffine& affine )
{
	if ( m_modeStereoView == stereoSideBySide )
	{
		SGLSize	sizeView = m_imgRightFrameColor.GetImageSize() ;
		affine.a11 = (float32_t) m_sizeSideBySideDstView.w
										/ (float32_t) sizeView.w ;
		affine.a12 = 0.0f ;
		affine.a13 = (float32_t) m_ptSideBySideDstView.x ;
		affine.a21 = 0.0f ;
		affine.a22 = (float32_t) m_sizeSideBySideDstView.h
										/ (float32_t) sizeView.h ;
		affine.a23 = (float32_t) m_ptSideBySideDstView.y ;
		//
		if ( m_nStereoViewParam & Window::stereoFlagLensDistortion )
		{
			affine.a13 -= m_fpLensOffsetX ;
		}
	}
	else
	{
		m_pglRenderer->m_glView.GetAffineVirtualToPhysics( affine ) ;
	}
}

// 論理座標→物理ビュー座標変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::VirtualToPhysicalPosition( S2DDVector& vPos )
{
	if ( (m_modeStereoView == stereoSideBySide)
		&& (m_nStereoViewParam & Window::stereoFlagLensDistortion) )
	{
		S2DVector	vDst, vSrc = vPos ;
		SGLSize		sizeView = m_imgRightFrameColor.GetImageSize() ;
		vSrc.x *= (float32_t) m_sizeDistortionMesh.w / (float32_t) sizeView.w ;
		vSrc.y *= (float32_t) m_sizeDistortionMesh.h / (float32_t) sizeView.h ;
		if ( SGLAffine::MeshMapping
			( m_aLensDistortionDstLeft.GetConstArray(),
				m_sizeDistortionMesh.w,
				m_sizeDistortionMesh.h, vDst, vSrc ) )
		{
			vPos = vDst ;
			return ;
		}
	}
	SGLWindowViewProducer::VirtualToPhysicalPosition( vPos ) ;
}

// 物理ビュー座標→論理座標変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::PhysicalToVirtualPosition( S2DDVector& vPos )
{
	SGLSize	sizePhysical =
				m_pglDirectRenderer->
					m_glView.m_rectViewPort.GetSize() ;
	if ( (m_modeStereoView == stereoSideBySide)
		&& (m_nStereoViewParam & Window::stereoFlagLensDistortion) )
	{
		SGLSize				sizeView = m_imgRightFrameColor.GetImageSize() ;
		S2DVector			vDst, vSrc = vPos ;
		const S2DVector *	pvMesh = m_aLensDistortionDstLeft.GetConstArray() ;
		if ( vSrc.x >= sizePhysical.w / 2 )
		{
			vSrc.x -= (float32_t) (sizePhysical.w / 2) ;
			pvMesh = m_aLensDistortionDstRight.GetConstArray() ;
		}
		if ( SGLAffine::InverseMeshMapping
			( pvMesh, m_sizeDistortionMesh.w,
				m_sizeDistortionMesh.h, vDst, vSrc ) )
		{
			vPos.x = vDst.x * (float32_t) sizeView.w / (float32_t) m_sizeDistortionMesh.w ;
			vPos.y = vDst.y * (float32_t) sizeView.h / (float32_t) m_sizeDistortionMesh.h ;
			return ;
		}
	}
	if ( m_modeStereoView == stereoSideBySide )
	{
		if ( vPos.x >= sizePhysical.w / 2 )
		{
			vPos.x -= (float32_t) (sizePhysical.w / 2) ;
		}
	}
	SGLWindowViewProducer::PhysicalToVirtualPosition( vPos ) ;
}

// 描画スレッドの関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::AttachViewThread( SGLAbstractWindow * pWnd )
{
	return	AttachGLCurrent() ;
}

// 描画スレッドの関連付け解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::DetachViewThread( SGLAbstractWindow * pWnd )
{
	return	DetachGLCurrent() ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLOpenGLWindowProducer::BeginDrawView
	( SGLAbstractWindow * pWnd,
		bool fOnWinThread, const SGLImageRect * pWindow,
		SGLImageObject * pImage, SGLImageObject * pZBuffer,
		SGLImageObject * pImageLeft, SGLImageObject * pZBufferLeft )
{
	SGLImageRect	rectView ;
	SGLSize	sizeVirtual = m_pglRenderer->m_glView.m_sizeVirtual ;
	if ( pWindow == NULL )
	{
		rectView.x = 0 ;
		rectView.y = 0 ;
		rectView.w = sizeVirtual.w ;
		rectView.h = sizeVirtual.h ;
		pWindow = &rectView ;
	}
	if ( fOnWinThread )
	{
		AttachGLView( &(m_pglRenderer->m_glView) ) ;
		if ( m_modeStereoView != stereoMonoView )
		{
			m_pRenderer->AttachStereoTargetImage
				( &m_imgRightFrameColor, &m_imgLeftFrameColor,
					&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
		}
		else if ( pImageLeft == NULL )
		{
			m_pRenderer->DisableStereoTargetImage() ;
			m_pRenderer->AttachTargetImage( pImage, pZBuffer, pWindow ) ;
		}
		else
		{
			m_pRenderer->AttachStereoTargetImage
				( pImage, pImageLeft, pZBuffer, pZBufferLeft, pWindow ) ;
		}
		if ( pWindow != NULL )
		{
			m_pglRenderer->m_glView.m_vViewOffset.x = (float32_t) - pWindow->x ;
			m_pglRenderer->m_glView.m_vViewOffset.y = (float32_t) - pWindow->y ;
		}
		m_pRenderer->ResetTransformation() ;
		m_pRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
		//
		if ( m_flagFlipFrame )
		{
			SGLPaintParam	ppPaint ;
			m_pglRenderer->DrawImage( ppPaint, &m_imgFrameColor ) ;
			m_flagFlipFrame = false ;
			return	NULL ;
		}
		return	m_pRenderer ;
	}
	else if ( GetCapacityFlags() & renderableAnyThread )
	{
		SSystem::LockTrace( __FILE__, __LINE__ ) ;
		if ( !AttachGLCurrent() )
		{
			AttachGLView( &(m_pglRenderer->m_glView) ) ;
			if ( m_modeStereoView != stereoMonoView )
			{
				m_pRenderer->AttachStereoTargetImage
					( &m_imgRightFrameColor, &m_imgLeftFrameColor,
						&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
			}
			else if ( pImageLeft == NULL )
			{
				m_pRenderer->DisableStereoTargetImage() ;
				m_pRenderer->AttachTargetImage( pImage, pZBuffer, pWindow ) ;
			}
			else
			{
				m_pRenderer->AttachStereoTargetImage
					( pImage, pImageLeft, pZBuffer, pZBufferLeft, pWindow ) ;
			}
			if ( pWindow != NULL )
			{
				m_pglRenderer->m_glView.m_vViewOffset.x = (float32_t) - pWindow->x ;
				m_pglRenderer->m_glView.m_vViewOffset.y = (float32_t) - pWindow->y ;
			}
			m_pRenderer->ResetTransformation() ;
			m_pRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
			return	m_pRenderer ;
		}
		SSystem::Unlock() ;
	}
	pWnd->Lock() ;
	if ( m_imgFrameColor.GetImageSize() != sizeVirtual )
	{
		// ※ OpenGL ハードによってはフレームバッファに
		//    formatImageARGB (GL_BGRA_EXT) が使用できない場合があるので注意！
		m_imgFrameColor.CreateImage
			( sizeVirtual.w, sizeVirtual.h,
				formatImageABGR, 32, SGLImageObject::bufferForRenderTarget ) ;
	}
	if ( m_imgFrameDepth.GetImageSize() != sizeVirtual )
	{
		m_imgFrameDepth.CreateImage
			( sizeVirtual.w, sizeVirtual.h,
				formatImageDepth, 32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	S3DOpenGLBufferedRenderer *	pRender = m_pFrameRenderer ;
	S3DOpenGLDirectlyRenderer &	gldRender = pRender->GetDirectlyRenderer() ;
	gldRender.m_glView.SetVirtualViewSize( sizeVirtual ) ;
	gldRender.m_glView.SetPhysicalViewSize( sizeVirtual ) ;
	pRender->AttachTargetImage( &m_imgFrameColor, &m_imgFrameDepth ) ;
	pRender->ResetTransformation() ;
	pRender->Begin3DRenderer() ;
	pWnd->Unlock() ;
	return	pRender ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::EndDrawView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	if ( fOnWinThread )
	{
		m_pRenderer->Finish() ;
		m_pRenderer->DetachTargetImage() ;
		return ;
	}
	else if ( GetCapacityFlags() & renderableAnyThread )
	{
		if ( render == (RenderContext*) m_pRenderer.Ptr() )
		{
			m_pRenderer->Finish() ;
			m_pRenderer->DetachTargetImage() ;
			DetachGLCurrent() ;
			SSystem::Unlock() ;
			return ;
		}
	}
	if ( render != NULL )
	{
		render->End3DRenderer() ;
		render->Finish() ;
	}
}

// 直接描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLOpenGLWindowProducer::BeginDirectView
	( SGLAbstractWindow * pWnd,
		bool fOnWinThread, const SGLImageRect * pWindow,
		SGLImageObject * pImage, SGLImageObject * pZBuffer,
		SGLImageObject * pImageLeft, SGLImageObject * pZBufferLeft )
{
	if ( fOnWinThread )
	{
		AttachGLView( &(m_pglDirectRenderer->m_glView) ) ;
		if ( m_modeStereoView == stereoSideBySide )
		{
			m_pRenderer->AttachStereoTargetImage
				( &m_imgRightFrameColor, &m_imgLeftFrameColor,
					&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
			m_pRenderer->ResetTransformation() ;
			m_pRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
			return	m_pRenderer ;
		}
		else if ( m_modeStereoView != stereoMonoView )
		{
			m_pDirectRenderer->AttachStereoTargetImage
				( &m_imgRightFrameColor, &m_imgLeftFrameColor,
					&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
		}
		else if ( pImageLeft == NULL )
		{
			m_pDirectRenderer->DisableStereoTargetImage() ;
			m_pDirectRenderer->AttachTargetImage( pImage, pZBuffer, pWindow ) ;
		}
		else
		{
			m_pDirectRenderer->AttachStereoTargetImage
				( pImage, pImageLeft, pZBuffer, pZBufferLeft, pWindow ) ;
		}
		if ( pWindow != NULL )
		{
			m_pglDirectRenderer->m_glView.m_vViewOffset.x = (float32_t) - pWindow->x ;
			m_pglDirectRenderer->m_glView.m_vViewOffset.y = (float32_t) - pWindow->y ;
		}
		m_pDirectRenderer->ResetTransformation() ;
		m_pDirectRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
		return	m_pDirectRenderer ;
	}
	else if ( GetCapacityFlags() & renderableAnyThread )
	{
		SSystem::LockTrace( __FILE__, __LINE__ ) ;
		if ( !AttachGLCurrent() )
		{
			AttachGLView( &(m_pglDirectRenderer->m_glView) ) ;
			if ( m_modeStereoView == stereoSideBySide )
			{
				m_pRenderer->AttachStereoTargetImage
					( &m_imgRightFrameColor, &m_imgLeftFrameColor,
						&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
				m_pRenderer->ResetTransformation() ;
				m_pRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
				return	m_pRenderer ;
			}
			else if ( m_modeStereoView != stereoMonoView )
			{
				m_pDirectRenderer->AttachStereoTargetImage
					( &m_imgRightFrameColor, &m_imgLeftFrameColor,
						&m_imgRightFrameDepth, &m_imgLeftFrameDepth, pWindow ) ;
			}
			else if ( pImageLeft == NULL )
			{
				m_pDirectRenderer->DisableStereoTargetImage() ;
				m_pDirectRenderer->AttachTargetImage( pImage, pZBuffer, pWindow ) ;
			}
			else
			{
				m_pDirectRenderer->AttachStereoTargetImage
					( pImage, pImageLeft, pZBuffer, pZBufferLeft, pWindow ) ;
			}
			if ( pWindow != NULL )
			{
				m_pglDirectRenderer->m_glView.m_vViewOffset.x = (float32_t) - pWindow->x ;
				m_pglDirectRenderer->m_glView.m_vViewOffset.y = (float32_t) - pWindow->y ;
			}
			m_pDirectRenderer->ResetTransformation() ;
			m_pDirectRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
			return	m_pDirectRenderer ;
		}
		SSystem::Unlock() ;
	}
	return	NULL ;
}

// 直接描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::EndDirectView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	if ( fOnWinThread )
	{
		if ( m_modeStereoView == stereoSideBySide )
		{
			m_pRenderer->Finish() ;
			m_pRenderer->DetachTargetImage() ;
		}
		else
		{
			m_pDirectRenderer->Finish() ;
			m_pDirectRenderer->DetachTargetImage() ;
		}
		return ;
	}
	else if ( GetCapacityFlags() & renderableAnyThread )
	{
		if ( render == (RenderContext*) m_pRenderer.Ptr() )
		{
			m_pRenderer->Finish() ;
			m_pRenderer->DetachTargetImage() ;
			DetachGLCurrent() ;
			SSystem::Unlock() ;
			return ;
		}
		else if ( render == (RenderContext*) m_pDirectRenderer.Ptr() )
		{
			m_pDirectRenderer->Finish() ;
			m_pDirectRenderer->DetachTargetImage() ;
			DetachGLCurrent() ;
			SSystem::Unlock() ;
			return ;
		}
	}
	if ( render != NULL )
	{
		render->Flush() ;
	}
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	if ( fOnWinThread || (GetCapacityFlags() & renderableAnyThread) )
	{
		if ( m_modeStereoView != stereoMonoView )
		{
			if ( !fOnWinThread )
			{
				SSystem::LockTrace( __FILE__, __LINE__ ) ;
				if ( AttachGLCurrent() )
				{
					SSystem::Unlock() ;
					return ;
				}
			}
			SGLSize			sizeView = m_imgRightFrameColor.GetImageSize() ;
			SGLImageRect	rectView( 0, 0, sizeView.w, sizeView.h ) ;
			//
			AttachGLView( &(m_pglDirectRenderer->m_glView) ) ;
			m_pDirectRenderer->DisableStereoTargetImage() ;
			m_pDirectRenderer->AttachTargetImage( NULL, NULL, &rectView ) ;
			m_pDirectRenderer->ResetTransformation() ;
			m_pDirectRenderer->SelectParallaxView( RenderContext::stereoViewAuto ) ;
			m_pDirectRenderer->AttachCustomShader( m_pglStereoViewShader ) ;
			m_pDirectRenderer->FillClearTarget( 0 ) ;
			//
			S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			S3DDVector	vZero( 0, 0, 0 ) ;
			m_pDirectRenderer->SetCamera( matI, vZero ) ;
			//
			S3DVector	vScreen
				( sizeView.w * 0.5, sizeView.h * 0.5, sizeView.w ) ;
			m_pDirectRenderer->SetProjectionScreen( vScreen ) ;
			m_pDirectRenderer->SetZClipRange( vScreen.z / 2, vScreen.z * 2 ) ;
			//
			SGLImageObject *	pLeftView = &m_imgLeftFrameColor ;
			SGLImageObject *	pRightView = &m_imgRightFrameColor ;
			if ( (m_modeStereoView != stereoInterleave)
				&& (m_nStereoViewParam & Window::stereoFlagSwapEyes) )
			{
				pLeftView = &m_imgRightFrameColor ;
				pRightView = &m_imgLeftFrameColor ;
			}
			//
			if ( m_modeStereoView == stereoSideBySide )
			{
				SGLSize	sizePhysical =
							m_pglDirectRenderer->
								m_glView.m_rectViewPort.GetSize() ;
				SGLAffine		affine ;
				SGLPaintParam	pp ;
				pp.nFlags = paintSmoothStretch
								| paintDelayable | paintOrderNoCare ;
				//
				if ( m_nStereoViewParam & Window::stereoFlagLensDistortion )
				{
					pp.SetAffine
						( affine, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 ) ;
					m_pDirectRenderer->DrawMesh
						( m_aLensDistortionDstLeft.GetConstArray(),
							m_aLensDistortionSrc.GetConstArray(),
							(size_t) m_sizeDistortionMesh.w,
							(size_t) m_sizeDistortionMesh.h, pp, pLeftView ) ;
					//
					pp.SetAffine
						( affine, sizePhysical.w / 2, 0.0, 0.0, 0.0, 1.0, 1.0 ) ;
					m_pDirectRenderer->DrawMesh
						( m_aLensDistortionDstRight.GetConstArray(),
							m_aLensDistortionSrc.GetConstArray(),
							(size_t) m_sizeDistortionMesh.w,
							(size_t) m_sizeDistortionMesh.h, pp, pRightView ) ;
				}
				else
				{
					SGLPoint	ptOffset = m_ptSideBySideDstView ;
					SGLSize		sizeDstView = m_sizeSideBySideDstView ;
					pp.SetAffine
						( affine, ptOffset.x, ptOffset.y, 0.0, 0.0,
							(double) sizeDstView.w / sizeView.w,
							(double) sizeDstView.h / sizeView.h ) ;
					m_pDirectRenderer->DrawImage( pp, pLeftView ) ;
					//
					pp.SetAffine
						( affine, sizePhysical.w / 2
									+ ptOffset.x, ptOffset.y, 0.0, 0.0,
							(double) sizeDstView.w / sizeView.w,
							(double) sizeDstView.h / sizeView.h ) ;
					m_pDirectRenderer->DrawImage( pp, pRightView ) ;
				}
			}
			else
			{
				S3DSurfaceAttribute	sufattr ;
				sufattr.flagsShading =
					shadingMethodNothing
						| shadingTextureSmoothing
						| shadingTextureMapping
						| shadingLuminousTexture
						| shadingNoZBuffer ;
				m_mtrStereoView.SetSurfaceAttribute( sufattr ) ;
				m_mtrStereoView.SetTexture
					( pRightView, 0, S3DMaterial::textureDiffusion ) ;
				m_mtrStereoView.SetTexture
					( pLeftView, 1,
						S3DMaterial::textureLuminous, 1.0f, 0.0f ) ;
				//
				S3DVector4	vVertics[4] ;
				vVertics[0].x = - vScreen.x ;
				vVertics[0].y = - vScreen.y ;
				vVertics[0].z = vScreen.z ;
				vVertics[1].x = vScreen.x ;
				vVertics[1].y = - vScreen.y ;
				vVertics[1].z = vScreen.z ;
				vVertics[2].x = vScreen.x ;
				vVertics[2].y = vScreen.y ;
				vVertics[2].z = vScreen.z ;
				vVertics[3].x = - vScreen.x ;
				vVertics[3].y = vScreen.y ;
				vVertics[3].z = vScreen.z ;
				//
				S2DVector	vUVMap[4] ;
				vUVMap[0].x = 0 ;
				vUVMap[0].y = 0 ;
				vUVMap[1].x = (float32_t) (sizeView.w - 1) ;
				vUVMap[1].y = 0 ;
				vUVMap[2].x = (float32_t) (sizeView.w - 1) ;
				vUVMap[2].y = (float32_t) (sizeView.h - 1) ;
				vUVMap[3].x = 0 ;
				vUVMap[3].y = (float32_t) (sizeView.h - 1) ;
				//
				uint32_t	nIndexed[6] =
				{
					0, 1, 2,  0, 2, 3
				} ;
				//
				m_pDirectRenderer->Begin3DRenderer() ;
				m_pDirectRenderer->AddIndexedTriangleList
					( &m_mtrStereoView, 0, 2, 4,
						vVertics, NULL, vUVMap, NULL, nIndexed ) ;
				m_pDirectRenderer->End3DRenderer() ;
			}
			//
			m_pDirectRenderer->Finish() ;
			m_pDirectRenderer->DetachTargetImage() ;
			m_pDirectRenderer->AttachCustomShader( NULL ) ;
			//
			if ( !fOnWinThread )
			{
				DetachGLCurrent() ;
				SSystem::Unlock() ;
			}
		}
		#if	defined(__PLATFORM_WINDOWS__)
			if ( m_glrc.hDC != NULL )
			{
				if ( !wglSwapLayerBuffers( m_glrc.hDC, WGL_SWAP_MAIN_PLANE ) )
				{
					ESLTrace( "failed to wglSwapLayerBuffers.\n" ) ;
					m_glrc.capsFlags &= ~renderableAnyThread ;
				}
			}
		#endif
	}
	else
	{
		m_flagFlipFrame = true ;
		//
		if ( pWnd != NULL )
		{
			pWnd->PostUpdate() ;
		}
	}
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::EnableZBuffer
	( SGLAbstractWindow * pWnd, bool flagZBuffer )
{
	m_flagZBuffer = flagZBuffer ;
	return	sglErrSuccess ;
}

// レイヤードウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::EnableLayeredWindow
	( SGLAbstractWindow * pWnd, bool flagLayeredWindow )
{
	if ( m_flagLayeredWindow != flagLayeredWindow )
	{
		m_flagLayeredWindow = flagLayeredWindow ;

		#if	defined(__PLATFORM_WINDOWS__)
		if ( m_glrc.hGLRC != NULL )
		{
			if ( m_flagLayeredWindow )
			{
				if ( !m_supports_ARB_pbuffer
						| !m_supports_ARB_pixel_format )
				{
					m_flagLayeredWindow = false ;
					return	sglErrNotSupported ;
				}
				SGLSize		sizeView = m_pglDirectRenderer->m_glView.m_sizePhysical ;
				SGLError	err = CreateARBBuffer( sizeView.w, sizeView.h ) ;
				if ( err )
				{
					m_flagLayeredWindow = false ;
				}
				return	err ;
			}
			else
			{
				DeleteGLContext() ;
				CreateGLContext() ;
			}
		}
		#endif
	}
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	SGLError	err = sglErrSuccess ;
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::OpenGLQuadBuffer ) == 0 )
	{
		/*
		if ( !m_flagQuadBuffer )
		{
			FinalizeStereoDisplayMode() ;
			//
			#if	defined(__PLATFORM_WINDOWS__)
				SSystem::LockTrace( __FILE__, __LINE__ ) ;
				if ( (m_hWnd == NULL) || !::IsWindow( m_hWnd ) )
				{
					m_flagQuadBuffer = true ;
				}
				else
				{
					DeleteGLContext() ;
					m_flagQuadBuffer = true ;
					err = CreateGLContext() ;
					if ( err )
					{
						m_flagQuadBuffer = false ;
						CreateGLContext() ;
					}
				}
				SSystem::Unlock() ;
			#else
				m_flagQuadBuffer = true ;
			#endif
		}
		*/
		m_flagQuadBuffer = m_flagSupportedStereo3D ;
		pWnd->PostUpdate() ;
		return	sglErrSuccess ;
	}
	else if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::AnaglyphView ) == 0 )
	{
		FinalizeStereoDisplayMode() ;
		//
		SGLOpenGLAnaglyphShader *	pglAnaglyph =
			ESLTypeCast<SGLOpenGLAnaglyphShader>
				( GetShaderProgramAs
					( SGLOpenGLAnaglyphShader::SHADER_ID ) ) ;
		if ( pglAnaglyph == NULL )
		{
			S3DRenderDevice::ShaderSourceInfo	srcAnaglyph ;
			SGLOpenGLAnaglyphShader::GetSourceInfo( srcAnaglyph ) ;
			pglAnaglyph = new SGLOpenGLAnaglyphShader( this ) ;
			//
			MakeCustomShader
				( SGLOpenGLAnaglyphShader::SHADER_ID,
								pglAnaglyph, srcAnaglyph ) ;
		}
		//
		ResizeStereoViewBuffer() ;
		//
		m_modeStereoView = stereoAnaglyphView ;
		m_nStereoViewParam = (uint32_t) nParam ;
		m_pglStereoViewShader = pglAnaglyph ;
		//
		pWnd->PostUpdate() ;
		return	sglErrSuccess ;
	}
	else if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::InterleavedView ) == 0 )
	{
		FinalizeStereoDisplayMode() ;
		//
		SGLOpenGLInterleaveShader *	pglInterleave =
			ESLTypeCast<SGLOpenGLInterleaveShader>
				( GetShaderProgramAs
					( SGLOpenGLInterleaveShader::SHADER_ID ) ) ;
		if ( pglInterleave == NULL )
		{
			S3DRenderDevice::ShaderSourceInfo	srcInterleave ;
			SGLOpenGLInterleaveShader::GetSourceInfo( srcInterleave ) ;
			pglInterleave = new SGLOpenGLInterleaveShader( this ) ;
			//
			MakeCustomShader
				( SGLOpenGLInterleaveShader::SHADER_ID,
								pglInterleave, srcInterleave) ;
		}
		int32_t	nFlag = (nParam & Window::stereoFlagVertical) ? 1 : 0 ;
		pglInterleave->SetCustomUniformIntAs( L"u_bVertical", &nFlag, 1 ) ;
		//
		nFlag = (nParam & Window::stereoFlagSwapEyes) ? 1 : 0 ;
		pglInterleave->SetCustomUniformIntAs( L"u_bOddLine", &nFlag, 1 ) ;
		//
		ResizeStereoViewBuffer() ;
		//
		m_modeStereoView = stereoInterleave ;
		m_nStereoViewParam = (uint32_t) nParam ;
		m_nStereoDrawFlags = 0 ;
		m_pglStereoViewShader = pglInterleave ;
		//
		OnMovedWindow( pWnd ) ;
		pWnd->PostUpdate() ;
		return	sglErrSuccess ;
	}
	else if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::SideBySide ) == 0 )
	{
		FinalizeStereoDisplayMode() ;
		//
		m_pglRenderer->m_glView.SetPhysicalViewSize
				( m_pglRenderer->m_glView.m_sizeVirtual ) ;
		//
		m_modeStereoView = stereoSideBySide ;
		m_nStereoViewParam = (uint32_t) nParam ;
		m_pglStereoViewShader = NULL ;
		//
		ResizeStereoViewBuffer() ;
		//
		pWnd->PostUpdate() ;
		return	sglErrSuccess ;
	}
	else
	{
		FinalizeStereoDisplayMode() ;
		pWnd->PostUpdate() ;
		//
		if ( SString::Compare
			( pszMethodID, SGLAbstractWindow::Stereo3D::MonoView ) == 0 )
		{
			return	sglErrSuccess ;
		}
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::IsStereoDisplayMode( void )
{
	return	(m_modeStereoView != stereoMonoView)
				|| (m_flagQuadBuffer && m_flagSupportedStereo3D) ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::OpenGLQuadBuffer ) == 0 )
	{
		return	m_flagSupportedStereo3D ;
	}
#endif
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::AnaglyphView ) == 0 )
	{
		return	true ;
	}
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::SideBySide ) == 0 )
	{
		return	true ;
	}
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::InterleavedView ) == 0 )
	{
		return	true ;
	}
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::MonoView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}

// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLOpenGLWindowProducer::GetStandardDisplaySize( void ) const
{
	SGLSize	sizeVirtual = m_pglRenderer->m_glView.m_sizeVirtual ;
	if ( (m_modeStereoView == stereoSideBySide)
		&& (m_nStereoViewParam & Window::stereoFlagPixelAspect1_1) )
	{
		sizeVirtual.w *= 2 ;
	}
	return	sizeVirtual ;
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLOpenGLWindowProducer::GetRenderDevice( void )
{
	return	this ;
}

// ウィンドウUIスレッド排他処理用同期
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLOpenGLWindowProducer::Lock( void ) const
{
	SGLAbstractWindow *	pWnd = m_refWindow ;
	if ( pWnd != NULL )
	{
		return	pWnd->Lock() ;
	}
	else
	{
		return	SSystem::Lock() ;
	}
}

void SGLOpenGLWindowProducer::Unlock( void ) const
{
	SGLAbstractWindow *	pWnd = m_refWindow ;
	if ( pWnd != NULL )
	{
		pWnd->Unlock() ;
	}
	else
	{
		SSystem::Unlock() ;
	}
}

// Side by side 表示用レンズパラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::SetLensDistortion
	( const float32_t * pDist, size_t nCount,
				float32_t fpScale, float32_t fpOffsetX )
{
	m_fpLensScale = fpScale ;
	m_fpLensOffsetX = fpOffsetX ;
	//
	if ( nCount >= 4 )
	{
		nCount = 4 ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_fpLensDistortion[i] = pDist[i] ;
	}
	//
	Lock() ;
	if ( m_modeStereoView == stereoSideBySide )
	{
		UpdateSideBySideViewParam() ;
	}
	Unlock() ;
}

// Side by side 表示での表示領域パラメータ更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::UpdateSideBySideViewParam( void )
{
	if ( m_modeStereoView != stereoSideBySide )
	{
		return ;
	}
	//
	// 表示領域計算
	//
	SGLSize	sizeView = m_imgRightFrameColor.GetImageSize() ;
	SGLSize	sizePhysical =
		m_pglDirectRenderer->m_glView.m_rectViewPort.GetSize() ;
	SGLSize	sizeDstView( sizePhysical.w / 2, sizePhysical.h ) ;
	if ( m_nStereoViewParam & Window::stereoFlagPixelAspect1_1 )
	{
		if ( sizeDstView.w * sizeView.h
						< sizeDstView.h * sizeView.w )
		{
			sizeDstView.h =
				sizeView.h * sizeDstView.w / sizeView.w ;
		}
		else
		{
			sizeDstView.w =
				sizeView.w * sizeDstView.h / sizeView.h ;
		}
	}
	else if ( m_nStereoViewParam & Window::stereoFlagPixelAspect1_2 )
	{
		if ( sizeDstView.w * sizeView.h
						< sizeDstView.h * (sizeView.w / 2) )
		{
			sizeDstView.h =
				sizeView.h * sizeDstView.w / (sizeView.w / 2) ;
		}
		else
		{
			sizeDstView.w =
				(sizeView.w / 2) * sizeDstView.h / sizeView.h ;
		}
	}
	if ( (m_nStereoViewParam & Window::stereoFlagLensScale)
									&& (m_fpLensScale < 1.0f) )
	{
		sizeDstView.w =
			eslRoundR32ToInt
				( (float32_t) sizeDstView.w * m_fpLensScale ) ;
		sizeDstView.h =
			eslRoundR32ToInt
				( (float32_t) sizeDstView.h * m_fpLensScale ) ;
	}
	m_sizeSideBySideDstView = sizeDstView ;
	m_ptSideBySideDstView.x = (sizePhysical.w / 2 - sizeDstView.w) / 2 ;
	m_ptSideBySideDstView.y = (sizePhysical.h - sizeDstView.h) / 2 ;
	//
	// レンズ歪み計算
	//
	if ( m_nStereoViewParam & Window::stereoFlagLensDistortion )
	{
		SGLAffine	affine
			( (float32_t) sizeDstView.w / (float32_t) sizeView.w, 0.0,
									(float32_t) m_ptSideBySideDstView.x,
				0.0, (float32_t) sizeDstView.h / (float32_t) sizeView.h,
									(float32_t) m_ptSideBySideDstView.y ) ;
		m_sizeDistortionMesh.w = (sizeView.w + 15) / 16 ;
		m_sizeDistortionMesh.h = (sizeView.h + 15) / 16 ;
		//
		SGLSize		sizeMesh = m_sizeDistortionMesh ;
		S2DVector *	pvSrc =
			m_aLensDistortionSrc.GetArray
				( (size_t) ((sizeMesh.w + 1) * (sizeMesh.h + 1)) ) ;
		S2DVector *	pvDstLeft =
			m_aLensDistortionDstLeft.GetArray
				( (size_t) ((sizeMesh.w + 1) * (sizeMesh.h + 1)) ) ;
		S2DVector *	pvDstRight =
			m_aLensDistortionDstRight.GetArray
				( (size_t) ((sizeMesh.w + 1) * (sizeMesh.h + 1)) ) ;
		//
		S2DVector	vMeshSize, vDstSize, vDstCenter ;
		vMeshSize.x = (float32_t) sizeView.w / (float32_t) sizeMesh.w ;
		vMeshSize.y = (float32_t) sizeView.h / (float32_t) sizeMesh.h ;
		vDstSize.x = (float32_t) sizePhysical.w * 0.5f ;
		vDstSize.y = (float32_t) sizePhysical.h ;
		vDstCenter.x = vDstSize.x * 0.5f ;
		vDstCenter.y = vDstSize.y * 0.5f ;
		//
		affine.a11 /= vDstCenter.x ;
		affine.a12 /= vDstCenter.x ;
		affine.a13 = affine.a13 / vDstCenter.x - 1.0f ;
		affine.a21 /= vDstCenter.y ;
		affine.a22 /= vDstCenter.y ;
		affine.a23 = affine.a23 / vDstCenter.y - 1.0f ;
		//
		float32_t	fpDist[4] =
		{
			m_fpLensDistortion[0],
			m_fpLensDistortion[1],
			m_fpLensDistortion[2],
			m_fpLensDistortion[3],
		} ;
		float32_t	fpOffsetX = m_fpLensOffsetX ;
		for ( int y = 0; y <= sizeMesh.h; y ++ )
		{
			S2DVector	vSrc ;
			S2DVector	vDst, vDstLeft, vDstRight ;
			vSrc.y = (float32_t) y * vMeshSize.y ;
			//
			for ( int x = 0; x <= sizeMesh.w; x ++ )
			{
				vSrc.x = (float32_t) x * vMeshSize.x ;
				vDst.x = affine.a11 * vSrc.x
							+ affine.a12 * vSrc.y + affine.a13 ;
				vDst.y = affine.a21 * vSrc.x
							+ affine.a22 * vSrc.y + affine.a23 ;
				//
				float32_t	r2 = vDst.x * vDst.x + vDst.y * vDst.y ;
				float32_t	r4 = r2 * r2 ;
				float32_t	r6 = r4 * r2 ;
				float32_t	k = fpDist[0] + fpDist[1] * r2
								 + fpDist[2] * r4 + fpDist[3] * r6 ;
				//
				vDstLeft.x = vDst.x * k - fpOffsetX ;
				vDstLeft.y = vDst.y * k ;
				if ( fabs( vDstLeft.x ) > 1.0f )
				{
					vDstLeft.x = (vDstLeft.x < 0.0f) ? -1.0f : 1.0f ;
				}
				if ( fabs( vDstLeft.y ) > 1.0f )
				{
					vDstLeft.y = (vDstLeft.y < 0.0f) ? -1.0f : 1.0f ;
				}
				vDstLeft.x = vDstLeft.x * vDstCenter.x + vDstCenter.x ;
				vDstLeft.y = vDstLeft.y * vDstCenter.y + vDstCenter.y ;
				//
				vDstRight.x = vDst.x * k + fpOffsetX ;
				vDstRight.y = vDst.y * k ;
				if ( fabs( vDstRight.x ) > 1.0f )
				{
					vDstRight.x = (vDstRight.x < 0.0f) ? -1.0f : 1.0f ;
				}
				if ( fabs( vDstRight.y ) > 1.0f )
				{
					vDstRight.y = (vDstRight.y < 0.0f) ? -1.0f : 1.0f ;
				}
				vDstRight.x = vDstRight.x * vDstCenter.x + vDstCenter.x ;
				vDstRight.y = vDstRight.y * vDstCenter.y + vDstCenter.y ;
				//
				int	i = y * (sizeMesh.w + 1) + x ;
				pvSrc[i] = vSrc ;
				pvDstLeft[i] = vDstLeft ;
				pvDstRight[i] = vDstRight ;
			}
		}
		m_aLensDistortionSrc.FinishArray() ;
		m_aLensDistortionDstLeft.FinishArray() ;
		m_aLensDistortionDstRight.FinishArray() ;
	}
}

// エラーログファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenGLWindowProducer::OpenErrorLogFile( const wchar_t * pwszFileName )
{
	m_pErrorLog = new SBufferedFile ;
	if ( m_pErrorLog->Open( pwszFileName, SFileOpener::modeCreate ) )
	{
		m_pErrorLog = nullptr ;
		return	sglErrFailed ;
	}
	m_pErrorLog->SetCharsetEncoding( Charset::encodingUTF8 ) ;
	//
	DATE_TIME	dt ;
	CurrentLocalDate( dt ) ;
	//
	m_pErrorLog->WriteFormat
		( L"\xfeff\r\n%04d/%02d/%02d %02d:%02d:%02d\r\n\r\n",
			dt.nYear, dt.nMonth, dt.nDay, dt.nHour, dt.nMinute, dt.nSecond ) ;
	return	sglErrSuccess ;
}

// エラーログ出力
//////////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindowProducer::WriteErrorLog( const wchar_t * pwszFormat, ... )
{
	if ( m_pErrorLog == nullptr )
	{
		if ( OpenErrorLogFile( L"opengl_error.log" ) )
		{
			return ;
		}
	}
	va_list	vl ;
	va_start( vl, pwszFormat ) ;
	m_pErrorLog->WriteFormatV( pwszFormat, vl ) ;
	m_pErrorLog->FlushBuffer() ;
}

// エラーログファイルが既に開かれているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindowProducer::IsOpenedErrorLog( void ) const
{
	return	(m_pErrorLog != nullptr) ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL ウィンドウ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLOpenGLWindow, SGLGenericWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenGLWindow::SGLOpenGLWindow
		( SSystem::SEnvironmentInterface * env )
	: SGLGenericWindow( new SGLOpenGLWindowProducer, env )
{
}

// 標準シェーダーのコンパイル
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindow::InvokeCompileDefaultShader
		( SGLOpenGLContext::DefaultShaderIndex dsIndex,
			S3DCustomShader::CompileListener * pListener )
{
	SGLOpenGLWindowProducer *
		poglwp = ESLTypeCast<SGLOpenGLWindowProducer>
								( m_wvfFramework.GetView() ) ;
	if ( poglwp == NULL )
	{
		return	false ;
	}
	bool	fSuccessed ;
	poglwp->AttachShaderCompileListener( pListener ) ;
	fSuccessed = poglwp->CompileDefaultShader( dsIndex ) ;
	poglwp->AttachShaderCompileListener( NULL ) ;
	return	fSuccessed ;
}

// 拡張標準シェーダーのコンパイル
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenGLWindow::InvokeCompileStandardShader
	( SGLOpenGLWindow::StandardShaderID idShader,
		S3DCustomShader::CompileListener * pListener )
{
	SGLOpenGLWindowProducer *
		poglwp = ESLTypeCast<SGLOpenGLWindowProducer>
								( m_wvfFramework.GetView() ) ;
	if ( poglwp == NULL )
	{
		return	false ;
	}
	const wchar_t *	pwszID = NULL ;
	switch ( idShader )
	{
	case	shaderGaussianBlur:
		pwszID = S3DRenderDevice::DefaultShaderId::GaussianBlur ;
		break ;
	case	shaderDepthBlender:
		pwszID = S3DRenderDevice::DefaultShaderId::DepthBlender ;
		break ;
	case	shaderSimpleMosaic:
		pwszID = S3DRenderDevice::DefaultShaderId::SimpleMosaic ;
		break ;
	case	shaderSimpleWater:
		pwszID = S3DRenderDevice::DefaultShaderId::SimpleWater ;
		break ;
	case	shaderShadowmapFilter:
		pwszID = S3DRenderDevice::DefaultShaderId::ShadowmapFilter ;
		break ;
	case	shaderNull:
	default:
		return	false ;
	}
	bool	fSuccessed ;
	poglwp->AttachShaderCompileListener( pListener ) ;
	fSuccessed = (poglwp->GetDefaultShaderProgramAs( pwszID ) != NULL) ;
	poglwp->AttachShaderCompileListener( NULL ) ;
	return	fSuccessed ;
}

// カスタムシェーダーのコンパイル
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * SGLOpenGLWindow::InvokeCompileCustomShader
	( const wchar_t * pwszID,
		const S3DRenderDevice::ShaderSourceInfo& source,
		S3DCustomShader * pCustomShader,
		S3DCustomShader::CompileListener * pListener )
{
	SGLOpenGLWindowProducer *
		poglwp = ESLTypeCast<SGLOpenGLWindowProducer>
								( m_wvfFramework.GetView() ) ;
	if ( poglwp == NULL )
	{
		delete	pCustomShader ;
		return	NULL ;
	}
	if ( poglwp->MakeCustomShader
			( pwszID, pCustomShader, source, pListener ) )
	{
		delete	pCustomShader ;
		pCustomShader = NULL ;
	}
	return	pCustomShader ;
}

// すべての拡張標準シェーダーをコンパイル
//////////////////////////////////////////////////////////////////////////
void SGLOpenGLWindow::CompileAllStandardShader
	( S3DCustomShader::CompileListener * pListener )
{
	InvokeCompileStandardShader( shaderGaussianBlur, pListener ) ;
	InvokeCompileStandardShader( shaderDepthBlender, pListener ) ;
	InvokeCompileStandardShader( shaderSimpleMosaic, pListener ) ;
	InvokeCompileStandardShader( shaderSimpleWater, pListener ) ;
	InvokeCompileStandardShader( shaderShadowmapFilter, pListener ) ;
}



