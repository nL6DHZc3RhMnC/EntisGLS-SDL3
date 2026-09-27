
#if	!defined(__SAKURAGL_VR_OCULUS_PRODUCER_H__)
#define	__SAKURAGL_VR_OCULUS_PRODUCER_H__	1

#include <sakuragl/sgl_vr_view_producer.h>
#include <sakuragl/sgl_opengl_render_context.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

//#pragma comment( lib, "libOVR.lib" )

#if	defined(_DEBUG) || defined(__DEBUG__)
	#if	defined(__PROCESSOR_INTEL_X86_64__)
		#if	defined(_DLL)
			#pragma comment( lib, "x64\\LibOVR_mdd.lib" )
		#else
			#pragma comment( lib, "x64\\LibOVR_mtd.lib" )
		#endif
	#else
		#if	defined(_DLL)
			#pragma comment( lib, "win32\\LibOVR_mdd.lib" )
		#else
			#pragma comment( lib, "win32\\LibOVR_mtd.lib" )
		#endif
	#endif
#else
	#if	defined(__PROCESSOR_INTEL_X86_64__)
		#if	defined(_DLL)
			#pragma comment( lib, "x64\\LibOVR_md.lib" )
		#else
			#pragma comment( lib, "x64\\LibOVR_mt.lib" )
		#endif
	#else
		#if	defined(_DLL)
			#pragma comment( lib, "win32\\LibOVR_md.lib" )
		#else
			#pragma comment( lib, "win32\\LibOVR_mt.lib" )
		#endif
	#endif
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Oculus Rift 出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOculusVRProducer
				: public SGLVRViewProducer,
					public SSystem::SProcedure,
					public SGLWindowViewSynchronizer
	{
	protected:
		SGLOpenGLContext *	m_pOpenGL ;

		enum	Status
		{
			statusUnintialized,
			statusFailedInitialize,
			statusLibInit,
			statusInitialized,
		} ;
		Status				m_status ;
		volatile bool		m_flagDisplayLost ;
		volatile bool		m_flagPostDisplayLost ;
		volatile bool		m_flagVisibleHMD ;
		bool				m_flagVisibleFrame ;
		ovrGraphicsLuid		m_luid ;
		ovrSession			m_session ;					// セッション
		ovrHmdDesc			m_hmdDesc ;					// HMD 情報
		ovrEyeRenderDesc	m_eyeDesc[ovrEye_Count] ;	// 両目レンダリング情報
		ovrTrackingState	m_tsLast ;					// 直前の ovrTrackingState
		ovrLayer_Union		m_layer ;					// 転送情報
		ovrMirrorTexture	m_mirrorTexture ;			// ミラー表示用
		long long			m_indexFrame ;
		ViewSpaceType		m_viewSpaceType ;
		S3DDVector			m_vPositionBase ;

		// テクスチャ生成成功フラグ
		bool	m_fCreatedColorTexture[ovrEye_Count] ;
		bool	m_fCreatedMirrorTexture ;

		// レンダリングターゲットテクスチャ
		SSystem::SObjectArray<SGLImageObject>	m_aRenderColor[ovrEye_Count] ;
		SGLImage								m_imgRenderDepth[ovrEye_Count] ;

		// ミラー用フレームバッファ
		SGLOpenGLFrameBuffer		m_fboMirror ;
		SGLImage					m_imgMirror ;
		SGLOpenGLTextureBuffer *	m_pglMirror ;

		// 描画用オブジェクト
		SGLSize						m_sizeWindow ;
		SSystem::SSmartPointer<S3DOpenGLBufferedRenderer>
									m_pRenderer ;

		// 描画同期用スレッド
		bool					m_flagBeganThread ;
		volatile bool			m_flagQuitThread ;
		SSystem::SThread		m_threadSync ;
		SSystem::SSignalEvent	m_signalReadyFrame ;
		SSystem::SSignalEvent	m_signalEndFrame ;
		SSystem::SSignalEvent	m_signalReadyInput ;
		SSystem::STimeCounter	m_timerSignalFrame ;

		// ライブラリ初期化カウンタ
		static atomic_int_t		m_countLibInit ;

		// ボタン指標変換 [enum ButtonIndex] -> ovrButton
		static const int		m_iVRBurronTransTable[64] ;

	public:
		// ディスプレイロストコマンド
		static const wchar_t *const	ID_OCULUS_DISPLAY_LOST ;

	public:
		enum	OculusButtonIndex
		{
			obuttonA			= button1,
			obuttonB,
			obuttonRThumb,
			obuttonRShoulder,
			obuttonX,
			obuttonY,
			obuttonLThumb,
			obuttonLShoulder,
			obuttonVolUp,
			obuttonVolDown,
			obuttonBack			= buttonSystem,
			obuttonEnter		= buttonAppMenu,
			obuttonHome			= buttonGrip,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLOculusVRProducer,
				SGLVRViewProducer,
					SProcedure, SGLWindowViewSynchronizer )
		// 構築関数
		SGLOculusVRProducer( SGLOpenGLContext * pOpenGL ) ;

	public:
		// 初期化
		SGLError Initialize
			( const SGLSize& sizeWindow, ViewSpaceType spaceType = spaceEyeLevel ) ;
		// 解放
		SGLError Release( void ) ;
		// セッション再生成（デバイスロスト時）
		SGLError RecreateSession( void ) ;
		// HMD 情報
		const ovrHmdDesc& GetHMDDescription( void ) const
		{
			return	m_hmdDesc ;
		}
		SGLSize GetDisplaySizeInEye( void ) const ;
		// DisplayLost 判定
		bool IsDisplayLost( void ) const
		{
			return	m_flagDisplayLost ;
		}
		// 描画タイミング同期
		SGLError WaitFrameSync
			( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;

	public:
		// LibOVR 初期化
		static SGLError InitializeLib( void ) ;
		// LibOVR 終了
		static void ShutdownLib( void ) ;
		// LibOVR 初期化済みか？
		static bool IsInitializedLib( void ) ;

	protected:
		// セッション生成
		SGLError CreateSession
			( const SGLSize& sizeWindow, ViewSpaceType spaceType ) ;
		SGLError CreateSessionWithouThread
			( const SGLSize& sizeWindow, ViewSpaceType spaceType ) ;
		// セッション解放
		SGLError ReleaseSession( void ) ;
		SGLError ReleaseSessionKeepThread( void ) ;
		// セッション再生成（デバイスロスト時スレッド内）
		SGLError RecreateSessionOnThread( void ) ;
		// 表示用 ovrTextureSwapChain を SGLImageObject 配列に関連付け
		void AttachTextureSwapChain
			( SSystem::SObjectArray<SGLImageObject>& aSwapChain,
				ovrTextureSwapChain textureChain,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFormat, uint32_t nBitsPerPixel ) const ;
		// ステータス取得／処理
		void PollSessionStatus( void ) ;
		// 姿勢取得
		void PollEyePosture( void ) ;
		// m_tsLast, m_eyeDesc, から Head, Eye, Hand の座標取得
		void ConvertHeadEyeHandPosture( void ) ;
		// ovrStatusBits -> PostureFlag 変換
		static uint32_t ConvertTrackingStatus( unsigned int nStatusFlags ) ;
		// ovrPosef -> Posture 変換
		void ConvertPosture
			( Posture& postureDst, const ovrPosef& poseSrc ) const ;
		// コントローラー取得
		void PollInputState( void ) ;
		// ovrInputState -> ControllerState 変換
		void ConvertControllerState
			( ControllerState& stateDst,
				ControllerIndex iController,
				const ovrInputState& stateSrc ) const ;

	protected:
		// セッション再生成実行用
		class	RecreateSessionProcedure	: public SSystem::SProcedure
		{
		protected:
			SGLOculusVRProducer *	m_pVR ;
		public:
			RecreateSessionProcedure( SGLOculusVRProducer * pVR ) ;
			virtual void Run( void ) ;
			virtual void Finalize( void ) ;
		} ;
		SSystem::SSmartPointer<RecreateSessionProcedure>	m_pRSProc ;
		SSystem::SSignalEvent								m_signalRSDone ;
		SSystem::SCriticalSection							m_csRSProc ;

		friend class RecreateSessionProcedure ;

	public:	// SGLSecondaryViewProducer 実装
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
			( SGLAbstractWindow * pPrimaryWnd ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pPrimaryWnd, RenderContext * render ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pPrimaryWnd, bool fVSync ) ;
		// 表示状態か？
		virtual bool IsVisibleView( void ) const ;

	public:	// SGLVRViewProducer 実装
		// デバイス状態更新タイミング同期
		virtual SGLError WaitForPollDeviceState( int64_t msecTimeout ) ;
		// 現在の HMD 姿勢を基準座標・姿勢にリセット
		virtual SGLError ResetCurrentHMDPosture( void ) ;
		// 現在の HMD 座標を基準座標にリセット
		virtual SGLError ResetCurrentHMDPosition( void ) ;
		// HMD 空間タイプ取得
		virtual ViewSpaceType GetViewSpaceType( void ) const ;
		// デバイスモデル取得
		virtual SGLError GetControllerModel
				( ModelInfo& mi, ControllerIndex iCtrl ) ;
		// HMD スケール
		virtual void SetScaleHMDToModel( double fpScale ) ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;

	public:	// SGLWindowViewSynchronizer 実装
		// 更新タイミング待ち
		virtual SGLError WaitForView( int64_t msecTimeout ) ;

	protected:
		SSystem::SSmartPointer<SSystem::SBufferedFile>	m_pLogFile ;

	public:
		// ログ出力するファイル設定
		void SetLogFile( SSystem::SBufferedFile * pfile ) ;
		// デバッグ出力
		void LogTrace( const wchar_t * pwszFormat, ... ) const ;

	} ;

}

#endif


