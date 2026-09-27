
#if	!defined(__SAKURAGL_OPENGL_WINDOW_PRODUCER_H__)
#define	__SAKURAGL_OPENGL_WINDOW_PRODUCER_H__	1

#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/sgl_opengl_render_context.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// OpenGL ウィンドウ・表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLWindowProducer	: public SGLOpenGLContext,
										public SGLWindowViewProducer
	{
	public:
	#if	defined(__PLATFORM_WINDOWS__)
		DECLARE_HANDLE(HPBUFFERARB);

		struct	OGLRenderingContext
		{
			HWND		hWnd ;
			HDC			hDC ;
			HGLRC		hGLRC ;
			uint64_t	capsFlags ;
			int			nPixelFormat ;
			HPBUFFERARB	hPBuffer ;
			SGLSize		sizePBuffer ;

			OGLRenderingContext( void ) 
				: hWnd( nullptr ), hDC( nullptr ), hGLRC( nullptr ),
					capsFlags( 0 ), nPixelFormat( 0 ),
					hPBuffer( nullptr ), sizePBuffer( 0, 0 ) {}
		} ;
		enum	FormaDescriptiontFlag
		{
			formatStereo	= 0x0001,		// Stereo3D QuadBuffer
			formatSRGB		= 0x0002,		// sRGB
		} ;
		struct	FormatDescription
		{
			uint32_t	nFlags ;
			uint32_t	nColorBits ;
			uint32_t	nColorChannelBits ;
			uint32_t	nAlphaBits ;
			uint32_t	nDepthBits ;
			uint32_t	nStencilBits ;

			FormatDescription( void )
				: nFlags( 0 ), nColorBits( 32 ),
					nColorChannelBits( 8 ), nAlphaBits( 8 ),
					nDepthBits( 32 ), nStencilBits( 0 ) { }
		} ;
	#endif

	protected:
	#if	defined(__PLATFORM_WINDOWS__)
		// メインコンテキスト
		OGLRenderingContext	m_glrc ;

		// 非同期用コンテキスト
		bool				m_flagAsyncNoRender ;
		OGLRenderingContext	m_glrcAsyncNoRender ;

		bool				m_supports_ARB_pbuffer ;
		bool				m_supports_ARB_pixel_format ;

		enum	WGL_ARB_pbuffer
		{
			WGL_DRAW_TO_PBUFFER_ARB		= 0x202D,
			WGL_MAX_PBUFFER_HEIGHT_ARB	= 0x2030,
			WGL_MAX_PBUFFER_PIXELS_ARB	= 0x202E,
			WGL_MAX_PBUFFER_WIDTH_ARB	= 0x202F,
			WGL_PBUFFER_HEIGHT_ARB		= 0x2035,
			WGL_PBUFFER_LARGEST_ARB		= 0x2033,
			WGL_PBUFFER_LOST_ARB		= 0x2036,
			WGL_PBUFFER_WIDTH_ARB		= 0x2034,
		} ;

		typedef BOOL (WINAPI * API_DESTROYPBUFFERARBPROC) ( HPBUFFERARB hPbuffer ) ;
		typedef BOOL (WINAPI * API_QUERYPBUFFERARBPROC) ( HPBUFFERARB hPbuffer, int iAttribute, int *piValue ) ;
		typedef HDC (WINAPI * API_GETPBUFFERDCARBPROC) ( HPBUFFERARB hPbuffer ) ;
		typedef HPBUFFERARB (WINAPI * API_CREATEPBUFFERARBPROC) ( HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList ) ;
		typedef int (WINAPI * API_RELEASEPBUFFERDCARBPROC) ( HPBUFFERARB hPbuffer, HDC hDC ) ;

		API_DESTROYPBUFFERARBPROC	wglDestroyPbufferARB ;
		API_QUERYPBUFFERARBPROC		wglQueryPbufferARB ;
		API_GETPBUFFERDCARBPROC		wglGetPbufferDCARB ;
		API_CREATEPBUFFERARBPROC	wglCreatePbufferARB ;
		API_RELEASEPBUFFERDCARBPROC	wglReleasePbufferDCARB ;


		enum	WGL_ARB_pixel_format
		{
			WGL_ACCELERATION_ARB			= 0x2003,
			WGL_ACCUM_ALPHA_BITS_ARB		= 0x2021,
			WGL_ACCUM_BITS_ARB				= 0x201D,
			WGL_ACCUM_BLUE_BITS_ARB			= 0x2020,
			WGL_ACCUM_GREEN_BITS_ARB		= 0x201F,
			WGL_ACCUM_RED_BITS_ARB			= 0x201E,
			WGL_ALPHA_BITS_ARB				= 0x201B,
			WGL_ALPHA_SHIFT_ARB				= 0x201C,
			WGL_AUX_BUFFERS_ARB				= 0x2024,
			WGL_BLUE_BITS_ARB				= 0x2019,
			WGL_BLUE_SHIFT_ARB				= 0x201A,
			WGL_COLOR_BITS_ARB				= 0x2014,
			WGL_DEPTH_BITS_ARB				= 0x2022,
			WGL_DOUBLE_BUFFER_ARB			= 0x2011,
			WGL_DRAW_TO_BITMAP_ARB			= 0x2002,
			WGL_DRAW_TO_WINDOW_ARB			= 0x2001,
			WGL_FULL_ACCELERATION_ARB		= 0x2027,
			WGL_GENERIC_ACCELERATION_ARB	= 0x2026,
			WGL_GREEN_BITS_ARB				= 0x2017,
			WGL_GREEN_SHIFT_ARB				= 0x2018,
			WGL_NEED_PALETTE_ARB			= 0x2004,
			WGL_NEED_SYSTEM_PALETTE_ARB		= 0x2005,
			WGL_NO_ACCELERATION_ARB			= 0x2025,
			WGL_NUMBER_OVERLAYS_ARB			= 0x2008,
			WGL_NUMBER_PIXEL_FORMATS_ARB	= 0x2000,
			WGL_NUMBER_UNDERLAYS_ARB		= 0x2009,
			WGL_PIXEL_TYPE_ARB				= 0x2013,
			WGL_RED_BITS_ARB				= 0x2015,
			WGL_RED_SHIFT_ARB				= 0x2016,
			WGL_SHARE_ACCUM_ARB				= 0x200E,
			WGL_SHARE_DEPTH_ARB				= 0x200C,
			WGL_SHARE_STENCIL_ARB			= 0x200D,
			WGL_STENCIL_BITS_ARB			= 0x2023,
			WGL_STEREO_ARB					= 0x2012,
			WGL_SUPPORT_GDI_ARB				= 0x200F,
			WGL_SUPPORT_OPENGL_ARB			= 0x2010,
			WGL_SWAP_COPY_ARB				= 0x2029,
			WGL_SWAP_EXCHANGE_ARB			= 0x2028,
			WGL_SWAP_LAYER_BUFFERS_ARB		= 0x2006,
			WGL_SWAP_METHOD_ARB				= 0x2007,
			WGL_SWAP_UNDEFINED_ARB			= 0x202A,
			WGL_TRANSPARENT_ALPHA_VALUE_ARB	= 0x203A,
			WGL_TRANSPARENT_ARB				= 0x200A,
			WGL_TRANSPARENT_BLUE_VALUE_ARB	= 0x2039,
			WGL_TRANSPARENT_GREEN_VALUE_ARB	= 0x2038,
			WGL_TRANSPARENT_INDEX_VALUE_ARB	= 0x203B,
			WGL_TRANSPARENT_RED_VALUE_ARB	= 0x2037,
			WGL_TYPE_COLORINDEX_ARB			= 0x202C,
			WGL_TYPE_RGBA_ARB				= 0x202B,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB	= 0x20A9,
		} ;

		typedef BOOL (WINAPI * API_CHOOSEPIXELFORMATARBPROC) ( HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats ) ;
		typedef BOOL (WINAPI * API_GETPIXELFORMATATTRIBFVARBPROC) ( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues ) ;
		typedef BOOL (WINAPI * API_GETPIXELFORMATATTRIBIVARBPROC) ( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues ) ;

		API_CHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB ;
		API_GETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfvARB ;
		API_GETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribivARB ;

//		HPBUFFERARB		m_hPBuffer ;
//		SGLSize			m_sizePBuffer ;
	#endif

		class	ANRAttacherProc	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
			SGLAbstractWindow *			m_pWnd ;
		public:
			ESL_DECLARE_CLASS_INFO( ANRAttacherProc, SProcedure ) ;
			ANRAttacherProc
				( SGLOpenGLWindowProducer * pOpenGL, SGLAbstractWindow * pWnd ) ;
			virtual void Run( void ) ;
		} ;

		class	ANRDetacherProc	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
			SGLAbstractWindow *			m_pWnd ;
			SSystem::SSignalEvent		m_signalDone ;
		public:
			ESL_DECLARE_CLASS_INFO( ANRDetacherProc, SProcedure ) ;
			ANRDetacherProc
				( SGLOpenGLWindowProducer * pOpenGL, SGLAbstractWindow * pWnd ) ;
			virtual void Run( void ) ;
			SSystem::SError WaitDone
				( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		} ;

		SSystem::SSmartPointer<ANRAttacherProc>	m_pANRAttacherProc ;
		SSystem::SSmartPointer<ANRDetacherProc>	m_pANRDetacherProc ;

		SSystem::SSmartReference<SGLAbstractWindow>	m_refWindow ;

		atomic_int_t				m_countAttached ;
		SSystem::SThread::IdType	m_tidThreadID ;
		SSystem::SMutex				m_mutexGLThread ;

		atomic_int_t				m_countAttachedANR ;
		SSystem::SThread::IdType	m_tidThreadIDANR ;
		SSystem::SCriticalSection	m_mutexGLThreadANR ;

		SSystem::SSignalEvent		m_signalAsyncProcedure ;
		atomic_int_t				m_countAsyncProcesures ;

		SSystem::SSmartPointer<S3DOpenGLBufferedRenderer>
									m_pRenderer ;
		SSystem::SSmartPointer<S3DOpenGLBufferedRenderer>
									m_pDirectRenderer ;
		S3DOpenGLDirectlyRenderer *	m_pglRenderer ;
		S3DOpenGLDirectlyRenderer *	m_pglDirectRenderer ;
		bool						m_flagQuadBuffer ;
		bool						m_flagZBuffer ;
		bool						m_flagLayeredWindow ;

		SSystem::SSmartPointer<S3DOpenGLBufferedRenderer>
									m_pFrameRenderer ;
		SGLSmartImage				m_imgFrameColor ;
		SGLSmartImage				m_imgFrameDepth ;
		bool						m_flagFlipFrame ;

		enum	SoftwareStereoMode
		{
			stereoMonoView,
			stereoAnaglyphView,
			stereoSideBySide,
			stereoInterleave,
		} ;
		SoftwareStereoMode			m_modeStereoView ;
		uint32_t					m_nStereoViewParam ;
		uint32_t					m_nStereoDrawFlags ;
		SGLSmartImage				m_imgRightFrameColor ;
		SGLSmartImage				m_imgLeftFrameColor ;
		SGLSmartImage				m_imgRightFrameDepth ;
		SGLSmartImage				m_imgLeftFrameDepth ;
		S3DMaterial					m_mtrStereoView ;
		SGLOpenGLCustomShader *		m_pglStereoViewShader ;
		SGLSize						m_sizeSideBySideDstView ;
		SGLPoint					m_ptSideBySideDstView ;
		float32_t					m_fpLensScale ;
		float32_t					m_fpLensOffsetX ;
		float32_t					m_fpLensDistortion[4] ;
		SGLSize						m_sizeDistortionMesh ;
		SSystem::SArray<S2DVector>	m_aLensDistortionSrc ;
		SSystem::SArray<S2DVector>	m_aLensDistortionDstLeft ;
		SSystem::SArray<S2DVector>	m_aLensDistortionDstRight ;


		class	GLSyncProcedure	: public SSystem::SSyncProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( GLSyncProcedure, SSyncProcedure )
			// 構築関数
			GLSyncProcedure
				( SGLOpenGLWindowProducer * pOpenGL,
							SSystem::SProcedure * pProc ) ;
			// 開始前の処理
			virtual void Prepare( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;
		friend class GLSyncProcedure ;

		class	GLAsyncProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
			SSystem::SProcedure *		m_pProc ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( GLAsyncProcedure, SProcedure )
			// 構築関数
			GLAsyncProcedure
				( SGLOpenGLWindowProducer * pOpenGL,
							SSystem::SProcedure * pProc ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 開始前の処理
			virtual void Prepare( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;
		friend class GLSyncProcedure ;

		class	GLAsyncNoRenderProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
			SGLAbstractWindow *			m_pWnd ;
			SSystem::SProcedure *		m_pProc ;
			SSystem::STimeCounter		m_timer ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( GLAsyncNoRenderProcedure, SProcedure )
			// 構築関数
			GLAsyncNoRenderProcedure
				( SGLOpenGLWindowProducer * pOpenGL,
					SGLAbstractWindow * pWnd,
					SSystem::SProcedure * pProc ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 開始前の処理
			virtual void Prepare( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;
		friend class GLSyncProcedure ;

		class	GLInitializeProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOpenGLWindowProducer *	m_pOpenGL ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( GLInitializeProcedure, SProcedure )
			// 構築関数
			GLInitializeProcedure( SGLOpenGLWindowProducer * pOpenGL ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;

		SSystem::SSmartPointer<SSystem::SBufferedFile>	m_pErrorLog ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLWindowProducer, SGLOpenGLContext, SGLWindowViewProducer )
		// 構築関数
		SGLOpenGLWindowProducer( void ) ;
		// 消滅関数
		virtual ~SGLOpenGLWindowProducer( void ) ;

	public:
		// OpenGL コンテキスト生成
		SGLError CreateGLContext( void ) ;
		// 拡張 API 準備
		SGLError InitializeGLEX( void ) ;
		// レイヤードウィンドウ用コンテキスト生成
		SGLError CreateARBBuffer( int nWidth, int nHeight ) ;
	#if	defined(__PLATFORM_WINDOWS__)
		SGLError CreateARBBuffer
			( OGLRenderingContext& glrc, int nWidth, int nHeight ) ;
	#endif
		// OpenGL コンテキスト破棄
		SGLError DeleteGLContext( void ) ;
	#if	defined(__PLATFORM_WINDOWS__)
		void DeleteGLContext( OGLRenderingContext& glrc ) const ;
	#endif
		// OpenGL コンテキストをスレッドへ関連付け
		SGLError AttachGLCurrent( void ) ;
		// OpenGL コンテキストをスレッドから解除
		SGLError DetachGLCurrent( void ) ;

		// 非レンダリング用 OpenGL コンテキストをスレッドへ関連付け
		SGLError AttachGLCurrentANR( void ) ;
		// 非レンダリング用 OpenGL コンテキストをスレッドから解除
		SGLError DetachGLCurrentANR( void ) ;

	public:
		// 初期処理
		virtual void OnCreateGLContext( void ) ;
		// 破棄前処理
		virtual void OnDestroyGLContext( void ) ;

	public:
	#if	defined(__PLATFORM_WINDOWS__)
		// ウィンドウ用コンテキスト生成
		SGLError CreateWindowGLContext
			( OGLRenderingContext& glrc,
				HWND hWnd, const FormatDescription& fmtDesc ) const ;
		SGLError CreateWindowGLContextAs
			( OGLRenderingContext& glrc,
				int nPixelFormat, const PIXELFORMATDESCRIPTOR * ppfd ) const ;
		// ARGB ピクセルフォーマット選択
		int ChooseWindowPixelFormatARB
			( HDC hdc, int * pFormats = NULL, size_t nCount = 0 ) const ;
		int ChooseWindowPixelFormatARBAs
			( HDC hdc, int * pFormats, size_t nCount,
								const FormatDescription& fmtDesc ) const ;
		// 別ウィンドウ用コンテキスト生成
		SGLError CreateSecondaryGLContext
			( OGLRenderingContext& glrc,
				HWND hWnd, const FormatDescription& fmtDesc ) const ;
		// ピクセルフォーマット設定
		SGLError SetPixelFormatGLContext
			( OGLRenderingContext& glrc,
				int nFormat, const PIXELFORMATDESCRIPTOR * ppfd ) const ;
		// 別ウィンドウ用コンテキスト破棄
		SGLError DestroySecondaryGLContext( OGLRenderingContext& glrc ) const ;
		// OpenGL コンテキストをスレッドへ関連付け
		SGLError AttachSecondaryGLCurrent( const OGLRenderingContext& glrc ) ;
		// OpenGL コンテキストをスレッドから解除
		SGLError DetachSecondaryGLCurrent( const OGLRenderingContext& glrc ) ;
		// コンテキスト取得
		const OGLRenderingContext& GetRenderingContext( void ) const ;
	#endif

	public:
		// ソフトウェアステレオビュー用バッファサイズ
		void ResizeStereoViewBuffer( void ) ;
		// ステレオ立体視モード終了処理
		void FinalizeStereoDisplayMode( void ) ;

	public:	// SGLOpenGLContext オーバーライド
		// OpenGL スレッドか判定
		virtual bool IsOnRenderThread( void ) ;
		virtual bool IsOnAsyncNoRenderThread( void ) const ;
		// OpenGL スレッドで実行する
		virtual SGLError Procedure
			( SSystem::SProcedure* pProc, ProcedurePriority priority ) ;
		// レンダリングスレッドでの遅延実行が全て完了するまで待機
		virtual SGLError WaitUntilAsyncAllProcedures
					( int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;

	public:	// SGLWindowViewProducer 実装
		// 対応機能フラグ
		virtual uint64_t GetCapacityFlags( void ) const ;
		// 論理ビューサイズ通知
		virtual void OnChangeVirtualViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 論理ビュー表示座標取得
		virtual void GetInternalViewPosition( SGLImageRect& rctVirtualView ) ;
		// 物理ビュー表示領域取得
		virtual bool GetExternalViewPosition( SGLImageRect& rctPhysicalView ) ;
		// 論理座標→物理ビュー座標変換行列取得
		virtual void GetAffineVirtualToPhysical( SGLAffine& affine ) ;
		// 論理座標→物理ビュー座標変換
		virtual void VirtualToPhysicalPosition( S2DDVector& vPos ) ;
		// 物理ビュー座標→論理座標変換
		virtual void PhysicalToVirtualPosition( S2DDVector& vPos ) ;
		// 描画スレッドの関連付け
		virtual SGLError AttachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画スレッドの関連付け解除
		virtual SGLError DetachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 直接描画ハンドラ開始
		virtual RenderContext * BeginDirectView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 直接描画ハンドラ終了
		virtual void EndDirectView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer
			( SGLAbstractWindow * pWnd, bool flagZBuffer ) ;
		// レイヤードウィンドウ設定
		virtual SGLError EnableLayeredWindow
			( SGLAbstractWindow * pWnd, bool flagLayeredWindow ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
		// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
		virtual SGLSize GetStandardDisplaySize( void ) const ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;

	public:
		// ウィンドウUIスレッド排他処理用同期
		SSystem::SError Lock( void ) const ;
		void Unlock( void ) const ;
		// Side by side 表示用レンズパラメータ設定
		void SetLensDistortion
			( const float32_t * pDist, size_t nCount,
					float32_t fpScale, float32_t fpOffsetX ) ;

	protected:
		// Side by side 表示での表示領域パラメータ更新
		void UpdateSideBySideViewParam( void ) ;

	public:
		// エラーログファイルを開く
		SGLError OpenErrorLogFile( const wchar_t * pwszFileName = L"opengl_error.log" ) ;
		// エラーログ出力
		void WriteErrorLog( const wchar_t * pwszFormat, ... ) ;
		// エラーログファイルが既に開かれているか？
		bool IsOpenedErrorLog( void ) const ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// OpenGL ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLWindow	: public SGLGenericWindow
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLWindow, SGLGenericWindow )
		// 構築関数
		SGLOpenGLWindow
			( SSystem::SEnvironmentInterface * env = NULL ) ;

	public:
		enum	StandardShaderID
		{
			shaderNull,
			shaderGaussianBlur,
			shaderDepthBlender,
			shaderSimpleMosaic,
			shaderSimpleWater,
			shaderShadowmapFilter,
		} ;

	public:
		// 標準シェーダーのコンパイル
		bool InvokeCompileDefaultShader
			( SGLOpenGLContext::DefaultShaderIndex dsIndex,
				S3DCustomShader::CompileListener * pListener = NULL ) ;
		// 拡張標準シェーダーのコンパイル
		bool InvokeCompileStandardShader
			( StandardShaderID idShader,
				SGLOpenGLShaderProgram::CompileListener * pListener = NULL ) ;
		// カスタムシェーダーのコンパイル
		S3DCustomShader * InvokeCompileCustomShader
			( const wchar_t * pwszID,
				const S3DRenderDevice::ShaderSourceInfo& source,
				S3DCustomShader * pCustomShader = NULL,
				S3DCustomShader::CompileListener * pListener = NULL ) ;
		// すべての拡張標準シェーダーをコンパイル
		void CompileAllStandardShader
			( S3DCustomShader::CompileListener * pListener = NULL ) ;
	} ;


}

#endif
