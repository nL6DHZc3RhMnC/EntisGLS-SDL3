
#if	!defined(__SAKURAGL_VR_OPENXR_PRODUCER_H__)
#define	__SAKURAGL_VR_OPENXR_PRODUCER_H__	1

#include <sakura/ssys_array_set.h>
#include <sakuragl/sgl_vr_view_producer.h>
#include <sakuragl/sgl_opengl_render_context.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#if	defined(_DLL)
	#if	defined(__DEBUG__)
	#pragma comment( lib, "openxr_loaderd.lib" )
	#else
	#pragma comment( lib, "openxr_loader.lib" )
	#endif
#else
	#if	defined(__DEBUG__)
	#pragma comment( lib, "openxr_loader_mtd.lib" )
	#else
	#pragma comment( lib, "openxr_loader_mt.lib" )
	#endif
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// OpenXR 出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenXRProducer
				: public SGLVRViewProducer,
					public SSystem::SProcedure,
					public SGLWindowViewSynchronizer
	{
	public:
		// API
	#ifdef XR_USE_GRAPHICS_API_OPENGL
		PFN_xrGetOpenGLGraphicsRequirementsKHR	xrGetOpenGLGraphicsRequirementsKHR ;
	#endif
		PFN_xrGetControllerModelKeyMSFT			xrGetControllerModelKeyMSFT ;
		PFN_xrLoadControllerModelMSFT			xrLoadControllerModelMSFT ;
		PFN_xrGetControllerModelPropertiesMSFT	xrGetControllerModelPropertiesMSFT ;
		PFN_xrGetControllerModelStateMSFT		xrGetControllerModelStateMSFT ;

	public:
		// パス
		enum	InteractionProfileBothHand
		{
			ipbhKhronosSimpleController,		// Khronos Simple Controller
			ipbhDaydreamController,				// Google Daydream Controller
			ipbhViveController,					// HTC Vive Controller
			ipbhMixedRealityMotionController,	// Microsoft Mixed Reality Motion Controller
			ipbhOculusGoController,				// Oculus Go Controller
			ipbhOculusTouchController,			// Oculus Touch Controller
			ipbhValveIndexController,			// Valve Index Controller
			ipbhValveCosmosController,			// HTC VIVE Cosmos Controller
			ipbhValveFocus3Controller,			// HTC VIVE Focus 3 Controller
			ipbhMetaQuestTouchProController,	// Meta Quest Touch Pro Controller
			ipbhBytedancePICONeo3Controller,	// Bytedance PICO Neo3 Controller
			ipbhBytedancePICO4Controller,		// Bytedance PICO 4 Controller
			ipbhControllerTypeCount,
		} ;
		enum	BothHandComponentPathIndex
		{
			bhcpiGripPose,
			bhcpiAimPose,
			bhcpiSelectClick,
			bhcpiMenuClick,
			bhcpiThumbkClick,		// thumb stick or trackpad
			bhcpiThumbTouch,
			bhcpiThumbX,
			bhcpiThumbY,
			bhcpiTrackpadClick,		// 2nd trackpad
			bhcpiTrackpadTouch,
			bhcpiTrackpadX,
			bhcpiTrackpadY,
			bhcpiTriggerClick,
			bhcpiTriggerTouch,
			bhcpiTriggerValue,
			bhcpiSqueezeClick,
			bhcpiSqueezeTouch,
			bhcpiSqueezeValue,
			bhcpiButton1,
			bhcpiButton1Touch,
			bhcpiButton2,
			bhcpiButton2Touch,
			bhcpiHapticOutput,
			bhcpiComponentCount,
			bhcpiEndOfList			= -1,
			bhcpiPoseComponentCount = 2,
		} ;
		enum	ActionStateType
		{
			typeActionBoolean,
			typeActionFloat,
			typeActionPose,
			typeActionVector2,
			typeActionHaptic,
			typeActionTypeCount,
		} ;
		struct	InteractionProfilePath
		{
			const char *	pszBothHandPath ;
			const char *	pszNeedsExtension ;
		} ;
		struct	HandComponentTypeInfo
		{
			ActionStateType	type ;
			const char *	pszActionName ;
			const char *	pszLocalizedName ;
		} ;
		struct	HandComponentPaths
		{
			BothHandComponentPathIndex	iComponent ;// コンポーネント指標
			const char *	pszComponentPath ;		// パス（左右で異なるパスの場合左手パス / 非対応は null）
			const char *	pszRightHandPath ;		// 左右で異なる場合右手用パス / 共通時は null / 片側だけの場合には非対応側を ""
		} ;
		struct	ControllerComponentState
		{
			ActionStateType	type ;
			XrBool32		active ;
			XrBool32		stateBoolean ;
			float			stateFloat ;
			XrPosef			statePose ;

			ControllerComponentState( void )
				: type(typeActionBoolean), active(false),
					stateBoolean(false), stateFloat(0.0f)
			{
				statePose.orientation.w = 1.0f ;
				statePose.orientation.x = 0.0f ;
				statePose.orientation.y = 0.0f ;
				statePose.orientation.z = 0.0f ;
				statePose.position.x = 0.0f ;
				statePose.position.y = 0.0f ;
				statePose.position.z = 0.0f ;
			}
		} ;
		static const char *const			s_pszUserHandPath[handCount] ;
		static const InteractionProfilePath s_InteractionProfilePath[ipbhControllerTypeCount] ;
		static const HandComponentTypeInfo	s_HandComponentTypeInfos[bhcpiComponentCount] ;
		static const XrActionType			s_ActionTypes[typeActionTypeCount] ;
		static const HandComponentPaths		s_HandComponentPaths
												[ipbhControllerTypeCount][bhcpiComponentCount+1] ;

	public:
		// 拡張情報
		class	ExtensionContext
				: public SSystem::SObjectArray< SSystem::SArray<char> >
		{
		public:
			bool	supportsD3D11 ;
			bool	supportsD3D12 ;
			bool	supportsOpenGL ;
			bool	supportsDepthInfo ;
			bool	supportsVisibilityMask ;
			bool	supportsUnboundedSpace ;
			bool	supportsSpatialAnchor ;
			bool	supportsHandInteraction ;
			bool	supportsEyeGazeInteraction ;
			bool	supportsHandJointTracking ;
			bool	supportsHandMeshTracking ;
			bool	supportsSpatialGraphBridge ;
			bool	supportsControllerModel ;
			bool	supportsSecondaryViewConfiguration ;
			bool	supportsAppContainer ;
			bool	supportsHolographicWindowAttachment ;
			bool	supportsSamsungOdysseyController ;
			bool	supportsHPMixedRealityController ;
			bool	supportsSpatialAnchorPersistence ;
			bool	supportsPerceptionAnchorInterop ;
			bool	supportsColorScaleBias ;
			bool	supportsSceneUnderstanding ;
			bool	supportsSceneUnderstandingSerialization ;
			bool	supportsReprojectionConfiguration ;
		public:
			ExtensionContext( void ) ;
			ExtensionContext( const ExtensionContext& xc ) ;
			const ExtensionContext& operator = ( const ExtensionContext& xc ) ;
			void EnumExtentions( const char* pszLayerName = nullptr ) ;
			void TraceAllExtentions( void ) ;
			void ChooseSupported( const char *const * ppszExtensionNames ) ;
			bool IsSupported( const char * pszExtensionName ) const ;
		} ;

		static const char *const	s_pszRequestExtensions[16] ;


		// 名前とバージョン
		class	Version
		{
		public:
			SSystem::SString	m_strName ;
			uint32_t			m_nVersion ;
		public:
			Version( void ) : m_nVersion(0) {}
			Version( const Version& ver )
				: m_strName( ver.m_strName ), m_nVersion( ver.m_nVersion ) {}
			Version( const wchar_t * pwszName, uint32_t nVersion )
				: m_strName( pwszName ), m_nVersion( nVersion ) {}
			const Version& operator = ( const Version& ver )
			{
				m_strName = ver.m_strName ;
				m_nVersion = ver.m_nVersion ;
				return	*this ;
			}
		} ;

		// 初期化コンフィグ
		struct	InitConfig
		{
			size_t						nReqExtentionsCount ;
			const char *const*			ppszReqExtentions ;

			const HandComponentPaths *	pDefHandPaths[ipbhControllerTypeCount] ;	// null 要素はデフォルト設定

			struct	HandComponentPathsDesc
			{
				InteractionProfilePath		profilePath ;
				const HandComponentPaths *	pHandPaths ;	// iComponent==bhcpiEndOfList を終端とする配列
			} ;
			size_t							nExProfileCount ;
			const HandComponentPathsDesc *	pExHandPathsDesc ;

			InitConfig( void )
				: nReqExtentionsCount( 0 ), ppszReqExtentions( nullptr ),
					nExProfileCount( 0 ), pExHandPathsDesc( nullptr )
			{
				for ( int i = 0; i < ipbhControllerTypeCount; i ++ )
				{
					pDefHandPaths[i] = nullptr ;
				}
			}
		} ;

		// インスタンス
		class	InstanceContext
		{
		public:
			bool					m_flagCreated ;
			XrInstance				m_xrInstance ;
			Version					m_verApp ;
			XrInstanceProperties	m_xripProperties ;
			XrPath					m_xrpHandPath[handCount] ;

		public:
			InstanceContext( void ) ;
			~InstanceContext( void ) ;
			bool CreateInstance
				( const Version& verApp, const ExtensionContext& extentions ) ;
			void DestroyInstance( void ) ;
			PFN_xrVoidFunction GetInstanceProcAddr( const char * pszFuncName ) const ;
			XrPath StringToPath( const char* str ) const ;
			SSystem::SArray<char> PathToString( XrPath path ) const ;
		} ;

		// ビュー・プロパティ
		class	ViewProperties
		{
		public:
			XrViewConfigurationType	m_xrvcType ;
			XrBool32				m_xrbFovMutable ;
			XrEnvironmentBlendMode	m_xrBlendMode;
			SSystem::SArray<XrEnvironmentBlendMode>	m_supportedBlendModes ;

		public:
			ViewProperties( void ) ;
			ViewProperties( const ViewProperties& vp ) ;
			bool GetViewProperties
				( XrInstance xrInstance,
					XrSystemId xrSystemId,
					XrViewConfigurationType xrViewConfigType ) ;
		} ;

		static constexpr size_t					s_ViewConfigTypeCount	 = 2 ;
		static const XrViewConfigurationType	s_ViewConfigurationTypes[s_ViewConfigTypeCount] ;

		// システム・コンテキスト
		class	SystemContext
		{
		public:
			XrSystemId			m_xrSystemId ;
			XrFormFactor		m_xrFormFactor ;
			XrSystemProperties	m_xrsProperties ;
			XrSystemHandTrackingPropertiesEXT
								m_xrHandProps ;
			XrSystemHandTrackingMeshPropertiesMSFT
								m_xrHandMeshPross ;
			XrSystemEyeGazeInteractionPropertiesEXT
								m_xrEyeGazePropes ;

			SSystem::SArraySet<XrViewConfigurationType>
								m_supportedPrimaryViewConfig ;
			SSystem::SArraySet<XrViewConfigurationType>
								m_supportedSecondaryViewConfig ;
			SSystem::SObjectArray<ViewProperties>
								m_aViewProperties ;

		public:
			SystemContext( void ) ;
			bool CreateSystemContext
				( const InstanceContext& instance,
					const ExtensionContext& extensions,
					XrFormFactor xrFormFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY ) ;
			const ViewProperties *
				GetViewPropertiesAs( XrViewConfigurationType xrvcType ) const ;
			bool EnumerateViewConfigurationViews
					( SSystem::SArray<XrViewConfigurationView>& aDstViews,
						XrViewConfigurationType xcvType,
						const InstanceContext& instance ) const ;
		
		} ;

		// セッション・コンテキスト
		class	SessionContext
		{
		public:
			bool						m_flagCreated ;
			XrSession					m_xrSession ;
			XrViewConfigurationType		m_xrViewConfigType ;
			XrEnvironmentBlendMode		m_xrBlendMode ;
			SSystem::SArraySet<XrViewConfigurationType>
										m_aSecondaryViewConfigs ;

			SSystem::SArraySet<GLint>	m_supportedColorGLFormats ;
			SSystem::SArray<uint32_t>	m_supportedColorImageFormats ;
			SSystem::SArraySet<GLint>	m_supportedDepthGLFormats ;
			SSystem::SArray<uint32_t>	m_supportedDepthImageFormats ;

			// note: all runtimes must support VIEW and LOCAL reference spaces.
			SSystem::SArraySet<XrReferenceSpaceType>
										m_supportedRefSpaces ;
			bool						m_supportsStageSpace ;
			bool						m_supportsUnboundedSpace ;

		public:
			SessionContext( void ) ;
			~SessionContext( void ) ;
			bool CreateSession
				( const SGLOpenXRProducer& oxr,
					const SystemContext& system,
					const InstanceContext& instance,
					const ExtensionContext& extensions ) ;
			void DestroySession( void ) ;
			SSystem::SArray<XrViewConfigurationType>
					GetAllViewConfigurationTypes( void ) const ;

		} ;

		struct	SupportGLFormatInfo
		{
			GLint			glType ;
			uint32_t		format ;
			const char *	pszGLFormat ;
		} ;
		static constexpr size_t				s_ColorBufferFormatCount = 9 ;
		static constexpr size_t				s_DepthBufferFormatCount = 6 ;
		static const SupportGLFormatInfo	s_ColorBufferFormatTypes[s_ColorBufferFormatCount] ;
		static const SupportGLFormatInfo	s_DepthBufferFormatTypes[s_DepthBufferFormatCount] ;

		// アクション
		class	Action
		{
		public:
			bool					m_flagCreated ;
			XrAction				m_xrAction ;
			SSystem::SArray<XrPath>	m_aSubactPaths ;
		public:
			Action( void ) ;
			~Action( void ) ;
			bool CreateAction
				( const InstanceContext& instance,
					XrActionSet xrActionSet,
					const char* actionName,
					const char* localizedName,
					XrActionType actionType,
					const char *const* ppSubactPaths,
					size_t nSubactPathCount ) ;
			void DestroyAction( void ) ;
			bool GetActionStateBoolean
				( const SessionContext& session,
					XrActionStateBoolean& stateResult, XrPath pathSubaction ) const ;
			bool GetActionStateFloat
				( const SessionContext& session,
					XrActionStateFloat& stateResult, XrPath pathSubaction ) const ;
			bool GetActionStateVector2f
				( const SessionContext& session,
					XrActionStateVector2f& stateResult, XrPath pathSubaction ) const ;
			bool GetActionStatePose
				( const SessionContext& session,
					XrActionStatePose& stateResult, XrPath pathSubaction ) const ;
		} ;

		// アクション・セット
		class	ActionSet
		{
		public:
			const InstanceContext&	m_instance ;
			bool					m_flagCreated ;
			bool					m_flagActive ;
			XrActionSet				m_xrActionSet ;

			SSystem::SObjectArray<Action>	m_actions ;
			SSystem::SArraySet<XrPath>		m_setDeclPaths ;

		public:
			ActionSet( const InstanceContext& instance ) ;
			~ActionSet( void ) ;
			bool CreateActionSet
				( const char* name, const char* localizedName, uint32_t priority = 0 ) ;
			void DestroyActionSet( void ) ;
			Action * CreateAction
				( const char* actionName,
					const char* localizedName,
					XrActionType actionType,
					const char *const* ppSubactPaths,
					size_t nSubactPathCount ) ;
			void SetActive( bool flagActive ) ;
			bool IsActived( void ) const ;
		} ;

		// アクション・バインディング
		struct	ActionBinding
		{
			XrAction		action ;
			const char *	binding ;
		} ;
		class	ActionBindingArray
					: public SSystem::SArray<XrActionSuggestedBinding>
		{
		} ;

		// アクション・コンテキスト
		class	ActionContext
		{
		public:
			const InstanceContext&				m_instance ;
			SSystem::SObjectArray<ActionSet>	m_aActionSets ;
			SSystem::SSortObjectArray
				< SSystem::SSortObjectElement<XrPath,ActionBindingArray> >
												m_mapActionBinding ;

		public:
			ActionContext( const InstanceContext& instance ) ;
			~ActionContext( void ) ;
			ActionSet * CreateActionSet
				( const char* name, const char* localizedName, uint32_t priority = 0 ) ;
			void SuggestInteractionProfileBindings
				( const char* interactionProfile,
					const ActionBinding* pActBindings, size_t nCountBindings ) ;
		} ;

		// リファレンス・スペース
		class	ReferenceSpace
		{
		public:
			bool		m_flagCreated ;
			XrSpace		m_xrRefSpace ;

		public:
			ReferenceSpace( void ) ;
			~ReferenceSpace( void ) ;
			bool CreateSpace
				( const SessionContext& session, XrReferenceSpaceType xrrsType ) ;
			bool CreateActionSpace
				( const SessionContext& session,
					Action * pAction, XrPath xrSubactionPath ) ;
			void DestroySpace( void ) ;
			bool LocateSpace
				( XrPosef& poseDst,
					const ReferenceSpace& spaceApp, const XrTime& time ) const ;
		} ;

		// フレーム設定
		class	FrameConfig
		{
		public:
			XrCompositionLayerFlags	m_flagsLayer ;
			GLint		m_glColorFormat ;
			uint32_t	m_formatColor ;
			GLint		m_glDepthFormat ;
			uint32_t	m_formatDepth ;
			XrExtent2Df	m_extentSizeScale ;		// SwapchainSizeScale is applied to recommended image rect
			XrExtent2Df	m_extentFovScale ;		// SwapchainFovScale is applied to recommended view fov
			XrExtent2Df	m_extentViewportScale ;	// ViewportSizeScale is applied after SwapchainSizeScale applied
			XrOffset2Di	m_offsetViewport ;		// ViewportOffset is relative to (0, 0) of the swapchain
			uint32_t	m_countSwapchainSample ;
			bool		m_modeDoubleWide ;
			bool		m_flagSubmitDepthInfo ;
			bool		m_flagContentProtected ;
			bool		m_flagForceReset ;

			XrCompositionLayerReprojectionInfoMSFT			m_xclri ;
			XrCompositionLayerReprojectionPlaneOverrideMSFT	m_xclrpo ;

		public:
			FrameConfig( void ) ;
			const FrameConfig& operator = ( const FrameConfig& fc ) ;
			bool DoesNeedResetFrameFor( const FrameConfig& fc ) const ;
		} ;

		// スワップチェーン・バッファ
		class	SwapchainBuffer
		{
		public:
			bool			m_flagCreated ;
			bool			m_flagAcquired ;
			XrSwapchain		m_xrSwapchain ;
			GLint			m_glFormat ;
			int32_t			m_width ;
			int32_t			m_height ;
			uint32_t		m_iAcquireSwapchain ;
			SSystem::SArray<XrSwapchainImageOpenGLKHR>	m_aSwapchainOGL ;
			SSystem::SObjectArray<SGLImage>				m_aImages ;

		public:
			SwapchainBuffer( void ) ;
			~SwapchainBuffer( void ) ;
			bool CreateSwapchain
				( SGLOpenGLContext * pOpenGL,
					const SessionContext& session,
					GLint glFormat,  uint32_t format,
					int32_t width, int32_t height,
					uint32_t lengthArray, uint32_t countSample,
					XrSwapchainCreateFlags flagsCreate,
					XrSwapchainUsageFlags flagsUsage,
					const XrViewConfigurationType * pViewcfgType ) ;
			void DestroySwapchain( void ) ;
			bool AcquireSwapchain( void ) ;
			bool WaitSwapchain( void ) ;
			bool ReleaseSwapchain( void ) ;
			SGLImage * GetAcquired( void ) const ;
		} ;

		// ビュー・コンテキスト
		class	ViewContext
		{
		public:
			bool										m_active ;
			bool										m_locateViews ;
			bool										m_locateSpace ;
			XrViewConfigurationType						m_xrvcType ;
			SSystem::SArray<XrViewConfigurationView>	m_viewConfigs ;
			SSystem::SArray<XrView>						m_views ;
			XrPosef										m_xrPoseView ;

			FrameConfig		m_cfgCurrent ;
			FrameConfig		m_cfgPending ;

			XrSpace									m_xrSpace ;
			SSystem::SArray
				<XrCompositionLayerProjectionView>	m_aProjectionViews ;
			SSystem::SArray
				<XrCompositionLayerDepthInfoKHR>	m_aDepthInfos ;
			SSystem::SArray<SGLImageRect>			m_viewports ;
			SSystem::SArray<XrRect2Di>				m_aColorImageRects ;
			SSystem::SArray<XrRect2Di>				m_aDepthImageRects ;

			SwapchainBuffer		m_swapchainColor ;
			SwapchainBuffer		m_swapchainDepth ;
			SGLImage *			m_pLastColor ;
			SGLImage *			m_pLastDepth ;

			SSystem::SSmartPointer<SGLImageObject>	m_pTempColorLeft ;
			SSystem::SSmartPointer<SGLImageObject>	m_pTempColorRight ;
			SSystem::SSmartPointer<SGLImageObject>	m_pTempDepthLeft ;
			SSystem::SSmartPointer<SGLImageObject>	m_pTempDepthRight ;

			SSystem::SSmartPointer<SGLImageObject>	m_pBackColorLeft ;
			SSystem::SSmartPointer<SGLImageObject>	m_pBackColorRight ;
			SSystem::SSmartPointer<SGLImageObject>	m_pBackDepthLeft ;
			SSystem::SSmartPointer<SGLImageObject>	m_pBackDepthRight ;

		public:
			// 構築
			ViewContext( void ) ;
			// ビュー設定（m_viewConfigs）列挙
			bool CreateViewConfiguration
				( XrViewConfigurationType xcvType,
					const InstanceContext& instance,
					const SystemContext& system ) ;
			// ビューのアクティブ状態更新とスワップチェーン再生成判定
			bool UpdateStateActive
				( bool flagActive,
					const InstanceContext& instance,
					const SystemContext& system ) ;
			// ビューの視野角と視差（m_views）取得
			bool LocateViews
				( const SessionContext& session,
					const ReferenceSpace& spaceView, const XrTime& time ) ;
			// ベース空間（m_xrPoseView）取得
			bool LocateSpace
				( const ReferenceSpace& spaceView,
					const ReferenceSpace& spaceApp, const XrTime& time ) ;

		public:
			// ビュー（視差）数取得
			size_t GetViewCount( void ) const ;
			// HMD 姿勢取得
			bool GetHeadPosture
				( const SGLOpenXRProducer& oxr, Posture& postureHead ) const ;
			// 視野情報取得
			bool GetEyeFieldOfView
				( const SGLOpenXRProducer& oxr, EyeFieldOfView& eyeFOV, size_t iEye ) const ;
			// 視差情報取得
			bool GetEyePosture
				( const SGLOpenXRProducer& oxr, Posture& postureEye, size_t iEye ) const ;

		public:
			// スワップチェーン準備処理
			void InitializeFrame( const SessionContext& session ) ;
			// スワップチェーン生成／再生成
			bool PrepareFrame( const SGLOpenXRProducer& oxr ) ;
			// スワップチェーン破棄
			void ReleaseFrame( void ) ;
			// スワップチェーン再生成フラグ設定
			void SetFrameResetFlag( void ) ;

		public:
			// スワップチェーン取得
			bool AcquireSwapchain( void ) ;
			// スワップチェーン取得解放
			void ReleaseSwapchain( void ) ;
			// スワップチェーンをレンダラに関連付け
			bool AttachSwapchainToRenderer
				( const SGLOpenXRProducer& oxr, S3DRenderParameterContext& render ) ;
			// スワップチェーンをレンダラから分離
			void DetachSwapchainFromRenderer( S3DRenderParameterContext& render ) ;

		public:
			// 画面複製用のバッファにレンダラを関連付け
			bool AttachBackBufferToRenderer
				( const SGLOpenXRProducer& oxr, S3DRenderParameterContext& renderBack ) ;
			// バックバッファをスワップチェーンへ転送する
			void BltToSwapchainFromBackBuffer
				( S3DRenderParameterContext& render,
					S3DRenderParameterContext& renderBack ) ;
			// 現在のスワップチェーンを画面フレームバッファへ伸縮コピーする
			void BltPrimaryFramebufferTo
				( S3DRenderContextInterface& renderPrimary,
					S3DRenderParameterContext& renderBack,
					int xDst0, int yDst0, int xDst1, int yDst1 ) ;
		} ;

		// 全ビューコンテキスト
		class	ViewContexts	: public SSystem::SObjectArray<ViewContext>
		{
		public:
			ViewContexts( void ) ;
			~ViewContexts( void ) ;
			void Initialize( const SGLOpenXRProducer& oxr ) ;
			void ReleaseAllFrames( void ) ;
			void SetResetAllFrames( void ) ;
			ViewContext * GetViewContextAs( XrViewConfigurationType type ) const ;
		} ;

		// 表示レイヤー情報
		class	CompositionLayers
		{
		public:
			SSystem::SObjectArray
				<XrCompositionLayerQuad>				m_quads ;
			SSystem::SObjectArray
				<XrCompositionLayerProjection>			m_projections ;
			SSystem::SPointerArray
				<const XrCompositionLayerBaseHeader>	m_headers ;

		public:
			void AppendProjectionLayer( ViewContext& vc ) ;
			XrCompositionLayerQuad* AllocQuadLayer( void ) ;
			XrCompositionLayerProjection*
				AllocProjectionLayer( XrCompositionLayerFlags flags ) ;
			size_t GetLayerCount( void ) const ;
			const XrCompositionLayerBaseHeader *const* GetLayerPtrArray( void ) const ;
		} ;

		// コントローラーモデル
		class	ControllerModel	: public SSystem::SProcedure
		{
		public:
			const SGLOpenXRProducer &	m_oxr ;
			XrPath						m_pathController ;
			XrControllerModelKeyMSFT	m_xrcmKey ;
			S3DVertexBuffer *			m_pModel ;

		protected:
			class	ModelData
			{
			public:
				XrControllerModelKeyMSFT								m_xrcmKey ;
				SSystem::SSmartPointer<S3DVertexBuffer>					m_pModel ;
				SSystem::SArray<size_t>									m_aBoneOrder ;
				SSystem::SPointerArray<class S3DModelBoneSpace>			m_pBones ;
				SSystem::SArray<XrControllerModelNodePropertiesMSFT>	m_aNodeProps ;
				SSystem::SArray<XrControllerModelNodeStateMSFT>			m_aNodeStates ;
			} ;

			SSystem::SCriticalSection						m_csMutex ;
			ModelData *										m_pModelData ;
			SSystem::SSortObjectArray
				< SSystem::SSortObjectElement
					<XrControllerModelKeyMSFT,ModelData> >	m_models ;
			SSystem::SArray<XrControllerModelKeyMSFT>		m_queLoadKeys ;
			SSystem::SSmartPointer<SSystem::SThread>		m_pRunningThread ;
			SSystem::SObjectArray<SSystem::SThread>			m_aDoneThreads ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ControllerModel, SProcedure )
			// 構築関数
			ControllerModel( const SGLOpenXRProducer& oxr, XrPath pathController ) ;
			// 消滅関数
			virtual ~ControllerModel( void ) ;
			// モデル状態のポーリング
			void PollingModel( void ) ;
			// モデル状態更新
			void UpdateModelState( ModelData * pModelData ) ;

		protected:
			// 実行中スレッド削除
			void DeleteAllThreads( void ) ;
			// 終了スレッド後始末
			void CleanupDoneThreads( void ) ;
			// モデル読み込み実行
			bool LoadModel( ModelData& model, XrControllerModelKeyMSFT key ) ;

		public:	// SProcedure 実装
			// スレッド関数
			virtual void Run( void ) ;
		} ;

	protected:
		SGLOpenGLContext *	m_pOpenGL ;
		ExtensionContext	m_supported ;			// 利用可能な全拡張
		ExtensionContext	m_extensions ;			// 使用する拡張
		InstanceContext		m_instance ;			// インスタンス
		SystemContext		m_system ;				// システム
		SessionContext		m_session ;				// セッション

		ReferenceSpace		m_spaceView ;			// ビュースペース
		ReferenceSpace		m_spaceApp ;

		bool				m_flagVisibleView ;		// 表示状態
		ViewContexts		m_viewContexts ;		// 全ビューステート

		ViewSpaceType		m_viewSpaceType ;		// 空間タイプ
		S3DVector			m_vTrackingBasePos ;	// 座標調整用

		SSystem::SObjectArray<ActionContext>
							m_aActionContexts ;		// アクション
		bool				m_flagBoundActionSets ;

		Action *			m_pHandActions[bhcpiComponentCount] ;
		ReferenceSpace		m_spaceHandPose[handCount][bhcpiPoseComponentCount] ;

		SSystem::SObjectArray<ControllerModel>
							m_aControllerModels ;	// コントローラーモデル

		// 現在の状態
		SSystem::SCriticalSection	m_csState ;		// 状態同期用
		XrSessionState		m_stateSession ;		// セッション状態
		bool				m_flagBegunSession ;	// BeginSession ～ EndSession 間

		XrFrameState		m_xfsFrameState ;		// WaitFrame で更新
		SSystem::SArray<XrSecondaryViewConfigurationStateMSFT>
							m_aSecondaryViewCfgStates ;	// WaitFrame で更新
		SSystem::SPointerArray<ViewContext>
							m_aActiveSecondaryView ;	// BeginFrame で更新（アクティのみ）
		SSystem::SArray<XrSecondaryViewConfigurationLayerInfoMSFT>
							m_aSecondaryViewCfgFrameEnd ;	// BeginFrame で更新（アクティのみ）

		SSystem::SObjectArray<CompositionLayers>
							m_aTempCompositionLayers ;
		CompositionLayers *	m_pPrimaryViewLayers ;

		ControllerComponentState
							m_ccStateHands[handCount][bhcpiComponentCount] ;

		// ポーリングスレッド用
		bool					m_flagBegunThread ;
		volatile bool			m_flagAbortedPolling ;
		volatile bool			m_flagQuitThread ;
		SSystem::SThread		m_threadPoll ;
		SSystem::SSignalEvent	m_signalReadyFrame ;
		SSystem::SSignalEvent	m_signalReadyInput ;
		SSystem::SSignalEvent	m_signalDoneFrame ;

		SSystem::STimeCounter	m_timerFlipView ;

		bool					m_flagBegunDrawView ;
		SSystem::SSmartPointer
			<S3DOpenGLBufferedRenderer>	m_pRenderer ;
		SSystem::SSmartPointer
			<S3DOpenGLBufferedRenderer>	m_pBackRenderer ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLOpenXRProducer,
				SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )
		// 構築関数
		SGLOpenXRProducer( S3DRenderDevice * pDevice ) ;
		// 消滅関数
		virtual ~SGLOpenXRProducer( void ) ;
		// 初期化
		SGLError Initialize
			( const Version& verApp,
				ViewSpaceType spaceType = spaceEyeLevel,
				const InitConfig * pConfig = nullptr ) ;
		// 解放
		void Release( void ) ;
		// 拡張関数のアドレス取得
		void CommitXRFunctionsAddress( void ) ;

	public:
		// 拡張情報
		const ExtensionContext& GetXRExtension( void ) const ;
		// インスタンス
		const InstanceContext& GetXRInstance( void ) const ;
		// システム
		const SystemContext& GetXRSystem( void ) const ;
		// セッション
		const SessionContext& GetXRSession( void ) const ;

	public:
		// ViewContext 総数
		size_t GetViewContextCount( void ) const ;
		// ViewContext 取得（iView=0 がプライマリ）
		ViewContext * GetViewContextAt( size_t iView ) const ;
		// XrViewConfigurationType に対応する ViewContext 取得
		ViewContext * GetViewContextAs( XrViewConfigurationType type ) const ;

	public:
		// イベント・ポーリング （返り値 sglErrAbort は終了要請）
		SGLError PollEvents( void ) ;
		// セッション開始
		void BeginSession( void ) ;
		// セッション終了
		void EndSession( void ) ;
		// セッション実行中か？
		bool IsSessionRunning( void ) const ;
		// アクションコンテキスト追加
		ActionContext * NewActionContext( void ) ;
		// セッションにアクション関連付け（一度だけ）
		SGLError BindActionSets( void ) ;
		// アクション関連付け済みか？
		bool HasBoundActionSets( void ) const ;
		// コントローラーモデル更新
		void PollingControllerModels( void ) ;
		// フレーム同期
		SGLError WaitFrame( void ) ;
		// 現在のフレーム時間
		const XrTime& CurrentFrameTime( void ) const ;
		// アクション同期
		SGLError SyncActions( void ) ;
		// 現在フレームのビューの状態更新
		void UpdateAllViewsLocation( void ) ;
		// フレーム描画開始処理
		SGLError BeginFrame( void ) ;
		// フレーム描画完了
		SGLError EndFrame( void ) ;

	public:
		// 現フレームを表示すべきか？
		bool ShouldRenderCurrentFrame( void ) const ;
		// 描画ビューのサブミット登録
		SGLError AppendSubmitView( ViewContext * pvc ) ;

	public:
		// XrPosef から Posture へ変換（スケール変換あり）
		void GetPostureFromXrPosef
			( Posture& postureDst, const XrPosef& poseSrc ) const ;
		// XrPosef から Posture へ変換（空間変換無し）
		static void Matrix4FromXrPosef
			( S4DMatrix& mat4Dst, const XrPosef& poseSrc ) ;

	protected:
		// アクション登録
		void RegisterAllInteractions( const InitConfig * pConfig ) ;
		// アクションの状態を取得
		void SenseAllInteractions( void ) ;

	public:
		// ポーリングスレッド開始
		SGLError BeginPollingThread( void ) ;
		// ポーリングスレッド終了
		void EndPollingThread( void ) ;
		// ポーリングスレッド実行中か？
		bool IsPollingThreadRunning( void ) const ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;

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
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// プライマリウィンドウへの描画も行うか？
		virtual bool DoesDrawToPrimaryWindow( void ) ;
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
		// バイブレーション開始
		virtual SGLError StartVibration
			( ControllerIndex iCtrl, const VibrationParam& vibParam ) ;
		// バイブレーション即時停止
		virtual SGLError StopVibration( ControllerIndex iCtrl ) ;

	public:	// SGLWindowViewSynchronizer 実装
		// 更新タイミング待ち
		virtual SGLError WaitForView( int64_t msecTimeout ) ;

	public:
		// VR が使用可能か？
		static bool IsVRAvailable( void ) ;
		// XrResult 失敗・成功判定
		static bool XRVerify( XrResult xr, const char * pszFunc ) ;

	} ;

}


#endif

