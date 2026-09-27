
#if	!defined(__SAKURAGL_VR_OPENVR_PRODUCER_H__)
#define	__SAKURAGL_VR_OPENVR_PRODUCER_H__	1

#include <sakuragl/sgl_vr_view_producer.h>
#include <sakuragl/sgl_opengl_render_context.h>

#include <openvr.h>

//#pragma comment( lib, "openvr_api.lib" )

#if	defined(_DEBUG) || defined(__DEBUG__)
	#if	defined(__PROCESSOR_INTEL_X86_64__)
		#if	defined(_DLL)
			#pragma comment( lib, "win64\\Debug\\openvr_api64.lib" )
		#else
			#pragma comment( lib, "win64\\Debug\\openvr_api64_mt.lib" )
		#endif
	#else
		#if	defined(_DLL)
			#pragma comment( lib, "win32\\Debug\\openvr_api.lib" )
		#else
			#pragma comment( lib, "win32\\Debug\\openvr_api_mt.lib" )
		#endif
	#endif
#else
	#if	defined(__PROCESSOR_INTEL_X86_64__)
		#if	defined(_DLL)
			#pragma comment( lib, "win64\\Release\\openvr_api64.lib" )
		#else
			#pragma comment( lib, "win64\\Release\\openvr_api64_mt.lib" )
		#endif
	#else
		#if	defined(_DLL)
			#pragma comment( lib, "win32\\Release\\openvr_api.lib" )
		#else
			#pragma comment( lib, "win32\\Release\\openvr_api_mt.lib" )
		#endif
	#endif
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// OpenVR 出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenVRProducer
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
		Status					m_status ;
		vr::IVRSystem *			m_pHMD ;
		vr::IVRRenderModels *	m_pRenderModels ;

		vr::HmdMatrix44_t		m_mat4HMDProjection[eyeCount] ;
		bool					m_flagReverseZ[eyeCount] ;

		vr::TrackedDevicePose_t		m_tdpDevPose[vr::k_unMaxTrackedDeviceCount] ;
		vr::HmdMatrix34_t			m_mat34EyeToHead[2] ;
		vr::VRControllerState_t		m_statController[vr::k_unMaxTrackedDeviceCount] ;
		bool						m_flagControllerState[vr::k_unMaxTrackedDeviceCount] ;
		vr::TrackedDeviceIndex_t	m_tdiHMD ;
		vr::TrackedDeviceIndex_t	m_tdiRightHand ;
		vr::TrackedDeviceIndex_t	m_tdiLeftHand ;
		S3DDVector					m_vPositionBase ;
		bool						m_flagShowControllerModel[controllerCount] ;
		bool						m_flagPreparedAxisIndex[controllerCount] ;
		AxisIndex					m_mapAxisIndex[controllerCount][5] ;
		static const int			m_iVRBurronTransTable[64] ;		// [enum ButtonIndex] -> vr::EVRButtonId

	public:
		// 描画用オブジェクト
		class	Renderer	: public S3DOpenGLBufferedRenderer
		{
		protected:
			SGLOpenVRProducer *	m_pVR ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Renderer, S3DOpenGLBufferedRenderer )
			// 構築関数
			Renderer( SGLOpenVRProducer * vr, SGLOpenGLContext * pOpenGL ) ;
			// 透視変換行列更新
			void UpdatePerspectiveMatrix( void ) ;
		protected:
			// Flush 関数処理
			virtual void OnGLThreadFlush( bool flagFinish ) ;
		} ;
		friend class Renderer ;
	protected:
		SSystem::SSmartPointer<Renderer>	m_pRenderer ;

		// レンダリングターゲットテクスチャ
		SGLImage	m_imgRenderColor[eyeCount] ;
		SGLImage	m_imgResolveColor[eyeCount] ;
		SGLImage	m_imgRenderDepth[eyeCount] ;

		SGLOpenGLFrameBuffer		m_fboResolve[eyeCount] ;
		SGLOpenGLTextureBuffer *	m_pglResolve[eyeCount] ;

		// デバイス表示用モデル
		class	DeviceModel
		{
		public:
			enum	Status
			{
				statusError				= -1,
				statusPending,
				statusLoadingModel,
				statusLoadingTexture,
				statusLoaded,
				statusCompleted,
			} ;
			Status									m_status ;
			SSystem::SSmartPointer<S3DVertexBuffer>	m_pModel ;
			S3DMaterial								m_material ;
			SGLImage								m_imgTexture ;
			vr::TrackedDeviceIndex_t				m_tdiDevice ;
			SSystem::SArray<char>					m_bufModelName ;
			vr::RenderModel_t *						m_pvrSrcModel ;
			vr::RenderModel_TextureMap_t *			m_pvrSrcTexture ;
			bool									m_flagState ;
			vr::VRControllerState_t					m_stateController ;
			vr::RenderModel_ControllerMode_State_t	m_stateCtrlMode ;
			vr::RenderModel_ComponentState_t		m_stateComponentTip ;
		public:
			DeviceModel( void )
				: m_status( statusPending ),
					m_tdiDevice( vr::k_unTrackedDeviceIndexInvalid ),
					m_pvrSrcModel( NULL ),
					m_pvrSrcTexture( NULL ),
					m_flagState( false ) { }
		} ;
		SSystem::SStrSortObjectArray<DeviceModel>	m_ssoaModels ;
		SSystem::SPointerArray<DeviceModel>			m_aPendingModels ;
		SSystem::SCriticalSection					m_csModel ;

		// ポーリング用スレッド
		bool					m_flagBeganThread ;
		volatile bool			m_flagQuitThread ;
		SSystem::SThread		m_threadPoll ;
		SSystem::SSignalEvent	m_signalReadyFrame ;
		SSystem::SSignalEvent	m_signalReadyInput ;
		SSystem::SSignalEvent	m_signalDoneFrame ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLOpenVRProducer,
				SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )
		// 構築関数
		SGLOpenVRProducer( SGLOpenGLContext * pOpenGL ) ;

	public:
		// ランタイム初期化
		SGLError InitializeLib( void ) ;
		// 初期設定
		SGLError PrepareDevice( ViewSpaceType spaceType = spaceEyeLevel ) ;
		// 解放
		SGLError Release( void ) ;
		// VR が使用可能か？
		static bool IsVRAvailable( void ) ;

	protected:
		// コントローラーの軸パラメータ番号変換テーブル生成
		void PrepareControllerAxisIndexTable
			( AxisIndex * pAxisIndex,
					vr::TrackedDeviceIndex_t tdiDevice ) const ;
		// HmdMatrix44_t から透視変換パラメータ逆算
		static void ProjectionParamFromMatrix4
			( EyeFieldOfView& fov, bool& zReverse,
				const vr::HmdMatrix44_t& mat, const SGLSize& sizeView ) ;

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
		// m_tdpDevPose, m_mat34EyeToHead から Head, Eye, Hand の座標取得
		void ConvertHeadEyeHandPosture( void ) ;
		// HmdMatrix34_t から Posture へ変換
		void GetPostureFromHmdMatrix34
			( Posture& postureDst,
				const vr::HmdMatrix34_t& matSrc ) const ;
		// TrackedDevicePose_t から Posture へ変換
		void GetPostureFromTrackedDevicePose
			( Posture& postureDst,
				const vr::TrackedDevicePose_t& poseSrc ) const ;
		// VRControllerState_t から ControllerState へ変換
		void ConvertControllerState
			( ControllerState& stateDst,
				ControllerIndex iController,
				const vr::VRControllerState_t& stateSrc ) const ;
		// SteamVR イベント処理
		void ProcessVREvent( const vr::VREvent_t& vrEvent ) ;
		// TrackedDeviceIndex_t から ControllerIndex へ変換
		ControllerIndex ControllerIndexFromDeviceIndex
							( vr::TrackedDeviceIndex_t tdiDev ) const ;

	protected:
		// デバイス名取得
		SSystem::SString GetDeviceModelNameString
				( vr::TrackedDeviceIndex_t tdiDevice ) const ;
		// デバイスモデル取得
		DeviceModel * GetDeviceModel
				( vr::TrackedDeviceIndex_t tdiDevice ) const ;
		// デバイスモデル取得／読み込み開始
		DeviceModel * LoadDeviceModel
				( vr::TrackedDeviceIndex_t tdiDevice ) ;
		// 読み込み中のモデル処理
		void ProcessPendingModels( void ) ;
		// 読み込み済みのモデル状態取得
		void PollModelComponentState( void ) ;
		// モデル構築
		void BuildDeviceModel( DeviceModel& model ) ;

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

