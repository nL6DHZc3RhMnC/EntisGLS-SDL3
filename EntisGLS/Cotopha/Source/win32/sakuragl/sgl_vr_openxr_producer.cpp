
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_opengl_window_producer.h>
#include <sakuragl/sgl_vr_openxr_producer.h>
#include <sakuraglx/render/sglx_model_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// パス
//////////////////////////////////////////////////////////////////////////////

const char *const
	SGLOpenXRProducer::s_pszUserHandPath[SGLVRViewProducer::handCount] =
{
	"/user/hand/right",
	"/user/hand/left"
} ;

const SGLOpenXRProducer::InteractionProfilePath
		SGLOpenXRProducer::s_InteractionProfilePath
						[SGLOpenXRProducer::ipbhControllerTypeCount] =
{
	{ "/interaction_profiles/khr/simple_controller", nullptr },
	{ "/interaction_profiles/google/daydream_controller", nullptr },
	{ "/interaction_profiles/htc/vive_controller", nullptr },
	{ "/interaction_profiles/microsoft/motion_controller", nullptr },
	{ "/interaction_profiles/oculus/go_controller", nullptr },
	{ "/interaction_profiles/oculus/touch_controller", nullptr },
	{ "/interaction_profiles/valve/index_controller", nullptr },
	{ "/interaction_profiles/htc/vive_cosmos_controller", XR_HTC_VIVE_COSMOS_CONTROLLER_INTERACTION_EXTENSION_NAME },
	{ "/interaction_profiles/htc/vive_focus3_controller", XR_HTC_VIVE_FOCUS3_CONTROLLER_INTERACTION_EXTENSION_NAME },
	{ "/interaction_profiles/facebook/touch_controller_pro", "XR_FB_touch_controller_pro" },
	{ "/interaction_profiles/bytedance/pico_neo3_controller", "XR_BD_controller_interaction" },
	{ "/interaction_profiles/bytedance/pico4_controller", "XR_BD_controller_interaction" },
} ;

const SGLOpenXRProducer::HandComponentTypeInfo
	SGLOpenXRProducer::s_HandComponentTypeInfos[bhcpiComponentCount] =
{
	{ typeActionPose, "grip_pose", "Grip Pose" },				// bhcpiGripPose
	{ typeActionPose, "aim_pose", "Aim Pose" },					// bhcpiAimPose
	{ typeActionBoolean, "select_click", "Select Click" },		// bhcpiSelectClick
	{ typeActionBoolean, "menu_click", "Menu Click" },			// bhcpiMenuClick
	{ typeActionBoolean, "thumb_click", "Thumb Click" },		// bhcpiThumbkClick
	{ typeActionBoolean, "thumb_touch", "Thumb Touch" },		// bhcpiThumbTouch
	{ typeActionFloat, "thumb_x", "Thumb X" },					// bhcpiThumbX
	{ typeActionFloat, "thumb_y", "Thumb Y" },					// bhcpiThumbY
	{ typeActionBoolean, "trackpad_click", "Trackpad Click" },	// bhcpiTrackpadClick
	{ typeActionBoolean, "trackpad_touch", "Trackpad Touch" },	// bhcpiTrackpadTouch
	{ typeActionFloat, "trackpad_x", "Trackpad X" },			// bhcpiTrackpadX
	{ typeActionFloat, "trackpad_y", "Trackpad Y" },			// bhcpiTrackpadY
	{ typeActionBoolean, "trigger_click", "Trigger Click" },	// bhcpiTriggerClick
	{ typeActionBoolean, "trigger_touch", "Trigger Touch" },	// bhcpiTriggerTouch
	{ typeActionFloat, "trigger_value", "Trigger Value" },		// bhcpiTriggerValue
	{ typeActionBoolean, "squeeze_click", "Squeeze Click" },	// bhcpiSqueezeClick
	{ typeActionBoolean, "squeeze_touch", "Squeeze Touch" },	// bhcpiSqueezeTouch
	{ typeActionFloat, "squeeze_value", "Squeeze Value" },		// bhcpiSqueezeValue
	{ typeActionBoolean, "button1", "Button 1" },				// bhcpiButton1
	{ typeActionBoolean, "button1_touch", "Button 1 Touch" },	// bhcpiButton1Touch
	{ typeActionBoolean, "button2", "Button 2" },				// bhcpiButton2
	{ typeActionBoolean, "button2_touch", "Button 2 Touch" },	// bhcpiButton2Touch
	{ typeActionHaptic, "haptic_output", "Vibration" },			// bhcpiHapticOutput
} ;

const XrActionType	SGLOpenXRProducer::s_ActionTypes[typeActionTypeCount] =
{
	XR_ACTION_TYPE_BOOLEAN_INPUT,
	XR_ACTION_TYPE_FLOAT_INPUT,
	XR_ACTION_TYPE_POSE_INPUT,
	XR_ACTION_TYPE_VECTOR2F_INPUT,
	XR_ACTION_TYPE_VIBRATION_OUTPUT,
} ;

const SGLOpenXRProducer::HandComponentPaths
	SGLOpenXRProducer::s_HandComponentPaths
				[ipbhControllerTypeCount][bhcpiComponentCount+1] =
{
	// ipbhKhronosSimpleController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiSelectClick,		"/input/select/click", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", nullptr },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhDaydreamController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiSelectClick,		"/input/select/click", nullptr },
		{ bhcpiThumbkClick,		"/input/trackpad/click", nullptr },
		{ bhcpiTrackpadTouch,	"/input/trackpad/touch", nullptr },
		{ bhcpiThumbX,			"/input/trackpad/x", nullptr },
		{ bhcpiThumbY,			"/input/trackpad/y", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhViveController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", nullptr },
		{ bhcpiThumbkClick,		"/input/trackpad/click", nullptr },
		{ bhcpiThumbTouch,		"/input/trackpad/touch", nullptr },
		{ bhcpiThumbX,			"/input/trackpad/x", nullptr },
		{ bhcpiThumbY,			"/input/trackpad/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhMixedRealityMotionController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", nullptr },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTrackpadClick,	"/input/trackpad/click", nullptr },
		{ bhcpiTrackpadTouch,	"/input/trackpad/touch", nullptr },
		{ bhcpiTrackpadX,		"/input/trackpad/x", nullptr },
		{ bhcpiTrackpadY,		"/input/trackpad/y", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhOculusGoController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/back/click", nullptr },
		{ bhcpiThumbkClick,		"/input/trackpad/click", nullptr },
		{ bhcpiThumbTouch,		"/input/trackpad/touch", nullptr },
		{ bhcpiThumbX,			"/input/trackpad/x", nullptr },
		{ bhcpiThumbY,			"/input/trackpad/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhOculusTouchController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", "/input/system/click" },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton1Touch,	"/input/x/touch", "/input/a/touch" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiButton2Touch,	"/input/y/touch", "/input/b/touch" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhValveIndexController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
//		{ bhcpiTrackpadClick,	"/input/trackpad/force", nullptr },	// float なので未対応
		{ bhcpiTrackpadTouch,	"/input/trackpad/touch", nullptr },
		{ bhcpiTrackpadX,		"/input/trackpad/x", nullptr },
		{ bhcpiTrackpadY,		"/input/trackpad/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/a/click", nullptr },
		{ bhcpiButton1Touch,	"/input/a/touch", nullptr },
		{ bhcpiButton2,			"/input/b/click", nullptr },
		{ bhcpiButton2Touch,	"/input/b/touch", nullptr },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhValveCosmosController
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", "/input/system/click" },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhValveFocus3Controller
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", "/input/system/click" },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr }, 
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr }, 
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiSqueezeTouch,	"/input/squeeze/touch", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhMetaQuestTouchProController
	// (ipbhOculusTouchController のスーパーセット、但し拡張は使わないので同じ)
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", "/input/system/click" },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton1Touch,	"/input/x/touch", "/input/a/touch" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiButton2Touch,	"/input/y/touch", "/input/b/touch" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhBytedancePICONeo3Controller
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", nullptr },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton1Touch,	"/input/x/touch", "/input/a/touch" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiButton2Touch,	"/input/y/touch", "/input/b/touch" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
	// ipbhBytedancePICO4Controller
	{
		{ bhcpiGripPose,		"/input/grip/pose", nullptr },
		{ bhcpiAimPose,			"/input/aim/pose", nullptr },
		{ bhcpiMenuClick,		"/input/menu/click", "" },
		{ bhcpiThumbkClick,		"/input/thumbstick/click", nullptr },
		{ bhcpiThumbTouch,		"/input/thumbstick/touch", nullptr },
		{ bhcpiThumbX,			"/input/thumbstick/x", nullptr },
		{ bhcpiThumbY,			"/input/thumbstick/y", nullptr },
		{ bhcpiTriggerClick,	"/input/trigger/click", nullptr },
		{ bhcpiTriggerTouch,	"/input/trigger/touch", nullptr },
		{ bhcpiTriggerValue,	"/input/trigger/value", nullptr },
		{ bhcpiSqueezeClick,	"/input/squeeze/click", nullptr },
		{ bhcpiSqueezeValue,	"/input/squeeze/value", nullptr },
		{ bhcpiButton1,			"/input/x/click", "/input/a/click" },
		{ bhcpiButton1Touch,	"/input/x/touch", "/input/a/touch" },
		{ bhcpiButton2,			"/input/y/click", "/input/b/click" },
		{ bhcpiButton2Touch,	"/input/y/touch", "/input/b/touch" },
		{ bhcpiHapticOutput,	"/output/haptic", nullptr },
		{ bhcpiEndOfList,		nullptr, nullptr },
	},
} ;



//////////////////////////////////////////////////////////////////////////////
// 拡張情報 ExtensionContext
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ExtensionContext::ExtensionContext( void )
	: supportsD3D11( false ),
		supportsD3D12( false ),
		supportsOpenGL( false ),
		supportsDepthInfo( false ),
		supportsVisibilityMask( false ),
		supportsUnboundedSpace( false ),
		supportsSpatialAnchor( false ),
		supportsHandInteraction( false ),
		supportsEyeGazeInteraction( false ),
		supportsHandJointTracking( false ),
		supportsHandMeshTracking( false ),
		supportsSpatialGraphBridge( false ),
		supportsControllerModel( false ),
		supportsSecondaryViewConfiguration( false ),
		supportsAppContainer( false ),
		supportsHolographicWindowAttachment( false ),
		supportsSamsungOdysseyController( false ),
		supportsHPMixedRealityController( false ),
		supportsSpatialAnchorPersistence( false ),
		supportsPerceptionAnchorInterop( false ),
		supportsColorScaleBias( false ),
		supportsSceneUnderstanding( false ),
		supportsSceneUnderstandingSerialization( false ),
		supportsReprojectionConfiguration( false )
{
}

SGLOpenXRProducer::ExtensionContext::ExtensionContext( const ExtensionContext& xc )
	: SSystem::SObjectArray< SSystem::SArray<char> >( xc ),
		supportsD3D11( xc.supportsD3D11 ),
		supportsD3D12( xc.supportsD3D12 ),
		supportsOpenGL( xc.supportsOpenGL ),
		supportsDepthInfo( xc.supportsDepthInfo ),
		supportsVisibilityMask( xc.supportsVisibilityMask ),
		supportsUnboundedSpace( xc.supportsUnboundedSpace ),
		supportsSpatialAnchor( xc.supportsSpatialAnchor ),
		supportsHandInteraction( xc.supportsHandInteraction ),
		supportsEyeGazeInteraction( xc.supportsEyeGazeInteraction ),
		supportsHandJointTracking( xc.supportsHandJointTracking ),
		supportsHandMeshTracking( xc.supportsHandMeshTracking ),
		supportsSpatialGraphBridge( xc.supportsSpatialGraphBridge ),
		supportsControllerModel( xc.supportsControllerModel ),
		supportsSecondaryViewConfiguration( xc.supportsSecondaryViewConfiguration ),
		supportsAppContainer( xc.supportsAppContainer ),
		supportsHolographicWindowAttachment( xc.supportsHolographicWindowAttachment ),
		supportsSamsungOdysseyController( xc.supportsSamsungOdysseyController ),
		supportsHPMixedRealityController( xc.supportsHPMixedRealityController ),
		supportsSpatialAnchorPersistence( xc.supportsSpatialAnchorPersistence ),
		supportsPerceptionAnchorInterop( xc.supportsPerceptionAnchorInterop ),
		supportsColorScaleBias( xc.supportsColorScaleBias ),
		supportsSceneUnderstanding( xc.supportsSceneUnderstanding ),
		supportsSceneUnderstandingSerialization( xc.supportsSceneUnderstandingSerialization ),
		supportsReprojectionConfiguration( xc.supportsReprojectionConfiguration )
{
}

const SGLOpenXRProducer::ExtensionContext&
	SGLOpenXRProducer::ExtensionContext::operator = ( const ExtensionContext& xc )
{
	SObjectArray::operator = ( xc ) ;

	supportsD3D11 = xc.supportsD3D11 ;
	supportsD3D12 = xc.supportsD3D12 ;
	supportsOpenGL = xc.supportsOpenGL ;
	supportsDepthInfo = xc.supportsDepthInfo ;
	supportsVisibilityMask = xc.supportsVisibilityMask ;
	supportsUnboundedSpace = xc.supportsUnboundedSpace ;
	supportsSpatialAnchor = xc.supportsSpatialAnchor ;
	supportsHandInteraction = xc.supportsHandInteraction ;
	supportsEyeGazeInteraction = xc.supportsEyeGazeInteraction ;
	supportsHandJointTracking = xc.supportsHandJointTracking ;
	supportsHandMeshTracking = xc.supportsHandMeshTracking ;
	supportsSpatialGraphBridge = xc.supportsSpatialGraphBridge ;
	supportsControllerModel = xc.supportsControllerModel ;
	supportsSecondaryViewConfiguration = xc.supportsSecondaryViewConfiguration ;
	supportsAppContainer = xc.supportsAppContainer ;
	supportsHolographicWindowAttachment = xc.supportsHolographicWindowAttachment ;
	supportsSamsungOdysseyController = xc.supportsSamsungOdysseyController ;
	supportsHPMixedRealityController = xc.supportsHPMixedRealityController ;
	supportsSpatialAnchorPersistence = xc.supportsSpatialAnchorPersistence ;
	supportsPerceptionAnchorInterop = xc.supportsPerceptionAnchorInterop ;
	supportsColorScaleBias = xc.supportsColorScaleBias ;
	supportsSceneUnderstanding = xc.supportsSceneUnderstanding ;
	supportsSceneUnderstandingSerialization = xc.supportsSceneUnderstandingSerialization ;
	supportsReprojectionConfiguration = xc.supportsReprojectionConfiguration ;

	return	*this ;
}

void SGLOpenXRProducer::ExtensionContext::EnumExtentions( const char* pszLayerName )
{
	SArray<XrExtensionProperties>	bufExProps ;
	XrExtensionProperties *			pxrExProps = nullptr ;
	uint32_t	nExCount = 0 ;
	XrResult	xr =
		xrEnumerateInstanceExtensionProperties( pszLayerName, 0, &nExCount, nullptr ) ;
	if ( XRVerify( xr, "xrEnumerateInstanceExtensionProperties" ) )
	{
		pxrExProps = bufExProps.GetArray( nExCount ) ;
		for ( size_t i = 0; i < nExCount; i ++ )
		{
			pxrExProps[i].type = XR_TYPE_EXTENSION_PROPERTIES ;
		}
		xr = xrEnumerateInstanceExtensionProperties
					( pszLayerName, nExCount, &nExCount, pxrExProps ) ;
		if ( !XRVerify( xr, "xrEnumerateInstanceExtensionProperties" ) )
		{
			nExCount = 0 ;
		}
	}

	RemoveAll() ;
	for ( size_t i = 0; i < nExCount; i ++ )
	{
		SArray<char> *	pbufName = new SArray<char>() ;
		size_t	nStrLen = 0 ;
		while ( pxrExProps[i].extensionName[nStrLen] )
		{
			nStrLen ++ ;
		}
		pbufName->AddArray( pxrExProps[i].extensionName, nStrLen ) ;
		pbufName->Add( 0 ) ;

		Add( pbufName ) ;
	}

#ifdef XR_USE_GRAPHICS_API_D3D11
	supportsD3D11 = IsSupported(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
	supportsD3D12 = IsSupported(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
	supportsOpenGL = IsSupported(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
#endif
	supportsAppContainer = IsSupported(XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME);
	supportsHolographicWindowAttachment = IsSupported(XR_MSFT_HOLOGRAPHIC_WINDOW_ATTACHMENT_EXTENSION_NAME);
	supportsPerceptionAnchorInterop = IsSupported(XR_MSFT_PERCEPTION_ANCHOR_INTEROP_EXTENSION_NAME);
	supportsDepthInfo = IsSupported(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
	supportsVisibilityMask = IsSupported(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
	supportsUnboundedSpace = IsSupported(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
	supportsSpatialAnchor = IsSupported(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
	supportsHandInteraction = IsSupported(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
	supportsEyeGazeInteraction = IsSupported(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME);
	supportsSecondaryViewConfiguration = IsSupported(XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME);
	supportsHandJointTracking = IsSupported(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
	supportsHandMeshTracking = IsSupported(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);
	supportsSpatialGraphBridge = IsSupported(XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME);
	supportsControllerModel = IsSupported(XR_MSFT_CONTROLLER_MODEL_EXTENSION_NAME);
	supportsSamsungOdysseyController = IsSupported(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME);
	supportsHPMixedRealityController = IsSupported(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);
	supportsColorScaleBias = IsSupported(XR_KHR_COMPOSITION_LAYER_COLOR_SCALE_BIAS_EXTENSION_NAME);
	supportsSceneUnderstanding = IsSupported(XR_MSFT_SCENE_UNDERSTANDING_EXTENSION_NAME);
	supportsSceneUnderstandingSerialization = IsSupported(XR_MSFT_SCENE_UNDERSTANDING_SERIALIZATION_EXTENSION_NAME);
	supportsReprojectionConfiguration = IsSupported(XR_MSFT_COMPOSITION_LAYER_REPROJECTION_EXTENSION_NAME);
	supportsSpatialAnchorPersistence = IsSupported(XR_MSFT_SPATIAL_ANCHOR_PERSISTENCE_EXTENSION_NAME);
}

void SGLOpenXRProducer::ExtensionContext::TraceAllExtentions( void )
{
	ESLTrace( "OpenXR Extensions;\n" ) ;
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SArray<char> *	pbufName = GetAt( i ) ;
		ESLAssert( pbufName != nullptr ) ;
		if ( pbufName != nullptr )
		{
			ESLTrace( "  %s\n", pbufName->GetConstArray() ) ;
		}
	}
	ESLTrace( ".\n" ) ;
}

void SGLOpenXRProducer::ExtensionContext::ChooseSupported( const char *const * ppszExtensionNames )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SArray<char> *	pName = GetAt( i ) ;
		ESLAssert( pName != nullptr ) ;
		bool	flagChoose = false ;
		for ( size_t j = 0; ppszExtensionNames[j] != nullptr; j ++ )
		{
			if ( strcmp( pName->GetConstArray(), ppszExtensionNames[j] ) == 0 )
			{
				flagChoose = true ;
				break ;
			}
		}
		if ( !flagChoose )
		{
			RemoveAt( i -- ) ;
		}
	}
}

bool SGLOpenXRProducer::ExtensionContext::IsSupported( const char * pszExtensionName ) const
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SArray<char> *	pName = GetAt( i ) ;
		ESLAssert( pName != nullptr ) ;
		if ( strcmp( pName->GetConstArray(), pszExtensionName ) == 0 )
		{
			return	true ;
		}
	}
	return	false ;
}

const char *const	SGLOpenXRProducer::s_pszRequestExtensions[16] =
{
	XR_MSFT_CONTROLLER_MODEL_EXTENSION_NAME,
	XR_EXT_HAND_TRACKING_EXTENSION_NAME,
	XR_EXT_HAND_JOINTS_MOTION_RANGE_EXTENSION_NAME,
	XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME,
	XR_MSFT_HAND_INTERACTION_EXTENSION_NAME,
	XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME,
	XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME,

	XR_KHR_OPENGL_ENABLE_EXTENSION_NAME,
	XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME,
	XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME,
	XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME,
	XR_MSFT_FIRST_PERSON_OBSERVER_EXTENSION_NAME,

	nullptr,
} ;



//////////////////////////////////////////////////////////////////////////////
// インスタンス InstanceContext
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::InstanceContext::InstanceContext( void )
	: m_flagCreated( false ),
		m_xrInstance( XR_NULL_HANDLE )
{
	for ( size_t i = 0; i < handCount; i ++ )
	{
		m_xrpHandPath[i] = XR_NULL_PATH ;
	}
}

SGLOpenXRProducer::InstanceContext::~InstanceContext( void )
{
	DestroyInstance() ;
}

bool SGLOpenXRProducer::InstanceContext::CreateInstance
	( const Version& verApp, const ExtensionContext& extentions )
{
	ESLAssert( !m_flagCreated ) ;

	SPointerArray<const char>	bufNames ;
	for ( size_t i = 0; i < extentions.GetLength(); i ++ )
	{
		SArray<char> *	pName = extentions.GetAt( i ) ;
		ESLAssert( pName != nullptr ) ;
		bufNames.Add( pName->GetConstArray() ) ;
	}
	//
	XrInstanceCreateInfo xici{ XR_TYPE_INSTANCE_CREATE_INFO } ;
	xici.enabledExtensionCount = (uint32_t) bufNames.GetLength() ;
	xici.enabledExtensionNames = bufNames.GetConstArray() ;

	SArray<char>	bufAppName = verApp.m_strName.ToCharArray() ;
	strncpy_s( xici.applicationInfo.applicationName,
				XR_MAX_APPLICATION_NAME_SIZE,
				bufAppName.GetConstArray(), bufAppName.GetLength() ) ;
	xici.applicationInfo.applicationVersion = verApp.m_nVersion ;

	SArray<char>	bufEngineName = SString(L"EntisGLS4").ToCharArray() ;
	strncpy_s( xici.applicationInfo.engineName,
				XR_MAX_ENGINE_NAME_SIZE,
				bufEngineName.GetConstArray(), bufEngineName.GetLength() ) ;
	xici.applicationInfo.engineVersion = SSystem::entisgls4_version ;

	xici.applicationInfo.apiVersion = XR_CURRENT_API_VERSION ;

	XrResult	xr = xrCreateInstance( &xici, &m_xrInstance ) ;
	if ( !XRVerify( xr, "xrCreateInstance" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;

	eslFillMemory( &m_xripProperties, 0, sizeof(m_xripProperties) ) ;
	m_xripProperties.type = XR_TYPE_INSTANCE_PROPERTIES ;
	xr = xrGetInstanceProperties( m_xrInstance, &m_xripProperties ) ;
	if ( !XRVerify( xr, "xrGetInstanceProperties" ) )
	{
		return	false ;
	}

	m_verApp = verApp ;

	for ( size_t i = 0; i < handCount; i ++ )
	{
		m_xrpHandPath[i] = StringToPath( s_pszUserHandPath[i] ) ;
	}

	return	true ;
}

void SGLOpenXRProducer::InstanceContext::DestroyInstance( void )
{
	if ( m_flagCreated )
	{
		XrResult	xr = xrDestroyInstance( m_xrInstance ) ;
		XRVerify( xr, "xrDestroyInstance" ) ;
		m_flagCreated = false ;
	}
}

PFN_xrVoidFunction
	SGLOpenXRProducer::InstanceContext::
		GetInstanceProcAddr( const char * pszFuncName ) const
{
	ESLAssert( m_flagCreated ) ;
	PFN_xrVoidFunction	xrFuncPtr = nullptr;
	XrResult	xr = xrGetInstanceProcAddr
						( m_xrInstance, pszFuncName, &xrFuncPtr ) ;
	if ( XR_FAILED(xr) )
	{
		ESLTrace( "failed to xrGetInstanceProcAddr(%s)\n", pszFuncName ) ;
		return	nullptr ;
	}
	return	xrFuncPtr ;
}

XrPath SGLOpenXRProducer::InstanceContext::StringToPath( const char* str ) const
{
	ESLAssert( m_flagCreated ) ;
	XrPath		path ;
	XrResult	xr = xrStringToPath( m_xrInstance, str, &path ) ;
	XRVerify( xr, "xrStringToPath" ) ;
	return	path ;
}

SSystem::SArray<char>
	SGLOpenXRProducer::InstanceContext::PathToString( XrPath path ) const
{
	ESLAssert( m_flagCreated ) ;
	SArray<char>	bufString ;
	uint32_t	nStringLen ;
	XrResult	xr = xrPathToString( m_xrInstance, path, 0, &nStringLen, nullptr ) ;
	if ( !XRVerify( xr, "xrPathToString" ) )
	{
		return	bufString ;
	}
	xr = xrPathToString
		( m_xrInstance, path, nStringLen,
			&nStringLen, bufString.GetArray(nStringLen) ) ;
	bufString.FinishArray() ;
	XRVerify( xr, "xrPathToString" ) ;
	//
	return	bufString ;
}




//////////////////////////////////////////////////////////////////////////////
// ビュー・プロパティ ViewProperties
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ViewProperties::ViewProperties( void )
{
}

SGLOpenXRProducer::ViewProperties::ViewProperties( const ViewProperties& vp )
	: m_xrvcType( vp.m_xrvcType ),
		m_xrbFovMutable( vp.m_xrbFovMutable ),
		m_xrBlendMode( vp.m_xrBlendMode ),
		m_supportedBlendModes( vp.m_supportedBlendModes )
{
}

bool SGLOpenXRProducer::ViewProperties::GetViewProperties
	( XrInstance xrInstance, XrSystemId xrSystemId,
		XrViewConfigurationType xrViewConfigType )
{
	XrViewConfigurationProperties
		xrViewConfigProp{ XR_TYPE_VIEW_CONFIGURATION_PROPERTIES } ;
	XrResult	xr = xrGetViewConfigurationProperties
		( xrInstance, xrSystemId, xrViewConfigType, &xrViewConfigProp ) ;
	if ( !XRVerify( xr, "xrGetViewConfigurationProperties" ) )
	{
		return	false ;
	}
	m_xrvcType = xrViewConfigType ;
	m_xrbFovMutable = xrViewConfigProp.fovMutable ;

	uint32_t	countBlendModet ;
	xr = xrEnumerateEnvironmentBlendModes
		( xrInstance, xrSystemId, xrViewConfigType, 0, &countBlendModet, nullptr ) ;
	if ( !XRVerify( xr, "xrEnumerateEnvironmentBlendModes" ) )
	{
		return	false ;
	}
	xr = xrEnumerateEnvironmentBlendModes
		( xrInstance, xrSystemId, xrViewConfigType,
			countBlendModet, &countBlendModet,
			m_supportedBlendModes.GetArray(countBlendModet) ) ;
	m_supportedBlendModes.FinishArray() ;
	if ( !XRVerify( xr, "xrEnumerateEnvironmentBlendModes" ) )
	{
		return	false ;
	}
	const size_t					countCandidate  = 3 ;
	const XrEnvironmentBlendMode	candidateBlendModes[countCandidate] =
	{
		XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
		XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
		XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
	} ;
	bool	flagFoundBlendMode = false ;
	for ( size_t i = 0; i < m_supportedBlendModes.GetLength(); i ++ )
	{
		for ( size_t j = 0; j < countCandidate; j ++ )
		{
			if ( m_supportedBlendModes.At(i) == candidateBlendModes[j] )
			{
				m_xrBlendMode = m_supportedBlendModes.At(i) ;
				flagFoundBlendMode = true ;
				break ;
			}
		}
	}
	return	flagFoundBlendMode ;
}

const XrViewConfigurationType
	SGLOpenXRProducer::s_ViewConfigurationTypes
		[SGLOpenXRProducer::s_ViewConfigTypeCount] =
{
	XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
} ;


//////////////////////////////////////////////////////////////////////////////
// システム・コンテキスト SystemContext
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::SystemContext::SystemContext( void )
	: m_xrSystemId( XR_NULL_SYSTEM_ID )
{
	eslFillMemory( &m_xrsProperties, 0, sizeof(m_xrsProperties) ) ;
	eslFillMemory( &m_xrHandProps, 0, sizeof(m_xrHandProps) ) ;
	eslFillMemory( &m_xrHandMeshPross, 0, sizeof(m_xrHandMeshPross) ) ;
	eslFillMemory( &m_xrEyeGazePropes, 0, sizeof(m_xrEyeGazePropes) ) ;
	m_xrsProperties.type = XR_TYPE_SYSTEM_PROPERTIES ;
	m_xrHandProps.type = XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT ;
	m_xrHandMeshPross.type = XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT ;
	m_xrEyeGazePropes.type = XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT ;
}

bool SGLOpenXRProducer::SystemContext::CreateSystemContext
	( const InstanceContext& instance,
		const ExtensionContext& extensions, XrFormFactor xrFormFactor )
{
	XrSystemGetInfo	xrsi{ XR_TYPE_SYSTEM_GET_INFO } ;
	xrsi.formFactor = xrFormFactor ;

	XrResult xr = xrGetSystem( instance.m_xrInstance, &xrsi, &m_xrSystemId ) ;
	if ( xr == XR_ERROR_FORM_FACTOR_UNAVAILABLE )
	{
		ESLTrace( "XR_ERROR_FORM_FACTOR_UNAVAILABLE\n" ) ;
		return	false ;
	}
	else if ( !XRVerify( xr, "xrGetSystem" ) )
	{
		return	false ;
	}
	m_xrFormFactor = xrFormFactor ;

	if ( extensions.supportsHandJointTracking )
	{
		m_xrHandProps.next = m_xrsProperties.next ;
		m_xrsProperties.next = &m_xrHandProps ;
	}
	if ( extensions.supportsHandMeshTracking )
	{
		m_xrHandMeshPross.next = m_xrsProperties.next ;
		m_xrsProperties.next = &m_xrHandMeshPross ;
	}
	if ( extensions.supportsEyeGazeInteraction )
	{
		m_xrEyeGazePropes.next = m_xrsProperties.next ;
		m_xrsProperties.next = &m_xrEyeGazePropes ;
	}
	xr = xrGetSystemProperties
			( instance.m_xrInstance, m_xrSystemId, &m_xrsProperties ) ;
	if ( !XRVerify( xr, "xrGetSystemProperties" ) )
	{
		return	false ;
	}

	// ViewConfiguration 列挙
	SArraySet<XrViewConfigurationType>	aViewConfigs ;
	uint32_t	countViewConfig = 0 ;
	xr = xrEnumerateViewConfigurations
		( instance.m_xrInstance, m_xrSystemId, 0, &countViewConfig, nullptr ) ;
	if ( !XRVerify( xr, "xrEnumerateViewConfigurations" ) )
	{
		return	false ;
	}
	xr = xrEnumerateViewConfigurations
		( instance.m_xrInstance, m_xrSystemId,
			countViewConfig, &countViewConfig,
			aViewConfigs.GetArray(countViewConfig) ) ;
	aViewConfigs.FinishArray() ;
	if ( !XRVerify( xr, "xrEnumerateViewConfigurations" ) )
	{
		return	false ;
	}

	// XrViewConfigurationType 毎の ViewProperties
	for ( size_t i = 0; i < s_ViewConfigTypeCount; i ++ )
	{
		if ( aViewConfigs.Find( s_ViewConfigurationTypes[i] ) < 0 )
		{
			continue ;
		}
		XrViewConfigurationProperties
			xrViewConfigProp{ XR_TYPE_VIEW_CONFIGURATION_PROPERTIES } ;
		xr = xrGetViewConfigurationProperties
			( instance.m_xrInstance,
				m_xrSystemId, s_ViewConfigurationTypes[i], &xrViewConfigProp ) ;
		if ( !XRVerify( xr, "xrGetViewConfigurationProperties" ) )
		{
			continue ;
		}
		ViewProperties	vp ;
		if ( !vp.GetViewProperties
			( instance.m_xrInstance, m_xrSystemId, s_ViewConfigurationTypes[i] ) )
		{
			continue ;
		}
		m_aViewProperties.Add( new ViewProperties( vp ) ) ;
		//
		if ( i == 0 )
		{
			m_supportedPrimaryViewConfig.AddSorted( s_ViewConfigurationTypes[i] ) ;
		}
		else
		{
			m_supportedSecondaryViewConfig.AddSorted( s_ViewConfigurationTypes[i] ) ;
		}
	}
	return	true ;
}

const SGLOpenXRProducer::ViewProperties *
	SGLOpenXRProducer::SystemContext::GetViewPropertiesAs
					( XrViewConfigurationType xrvcType ) const
{
	for ( size_t i = 0; i < m_aViewProperties.GetLength(); i ++ )
	{
		ViewProperties *	pvp = m_aViewProperties.GetAt( i ) ;
		ESLAssert( pvp != nullptr ) ;
		if ( (pvp != nullptr)
			&& (pvp->m_xrvcType == xrvcType) )
		{
			return	pvp ;
		}
	}
	return	nullptr ;
}

bool SGLOpenXRProducer::SystemContext::
			EnumerateViewConfigurationViews
				( SSystem::SArray<XrViewConfigurationView>& aDstViews,
					XrViewConfigurationType xcvType,
					const InstanceContext& instance ) const
{
	uint32_t	countView ;
	XrResult xr = xrEnumerateViewConfigurationViews
					( instance.m_xrInstance,
						m_xrSystemId,
						xcvType, 0, &countView, nullptr ) ;
	if ( !XRVerify( xr, "xrEnumerateViewConfigurationViews" ) )
	{
		return	false ;
	}
	aDstViews.SetLength( countView ) ;
	for ( size_t i = 0; i < countView; i ++ )
	{
		aDstViews.At(i).type = XR_TYPE_VIEW_CONFIGURATION_VIEW ;
	}
	xr = xrEnumerateViewConfigurationViews
			( instance.m_xrInstance,
				m_xrSystemId,
				xcvType, countView, &countView, aDstViews.GetArray() ) ;
	if ( !XRVerify( xr, "xrEnumerateViewConfigurationViews" ) )
	{
		return	false ;
	}
	aDstViews.FinishArray() ;
	aDstViews.SetLength( countView ) ;
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// セッション・コンテキスト SessionContext
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::SessionContext::SessionContext( void )
	: m_flagCreated( false ), m_xrSession( XR_NULL_HANDLE ),
		m_supportsStageSpace( false ), m_supportsUnboundedSpace( false )
{
}

SGLOpenXRProducer::SessionContext::~SessionContext( void )
{
	DestroySession() ;
}

bool SGLOpenXRProducer::SessionContext::CreateSession
	( const SGLOpenXRProducer& oxr,
		const SystemContext& system,
		const InstanceContext& instance,
		const ExtensionContext& extensions )
{
	ESLAssert( !m_flagCreated ) ;
	XrSessionCreateInfo
		xsci{ XR_TYPE_SESSION_CREATE_INFO, nullptr, 0, system.m_xrSystemId } ;

#ifdef XR_USE_GRAPHICS_API_OPENGL
	//
	// OpenGL の必要なバージョンを確認
	//
	ESLAssert( oxr.xrGetOpenGLGraphicsRequirementsKHR != nullptr ) ;
	if ( oxr.xrGetOpenGLGraphicsRequirementsKHR != nullptr )
	{
		XrGraphicsRequirementsOpenGLKHR
				xrRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR } ;
		XrResult xr = oxr.xrGetOpenGLGraphicsRequirementsKHR
				( instance.m_xrInstance, system.m_xrSystemId, &xrRequirements ) ;
		if ( XRVerify( xr, "xrGetOpenGLGraphicsRequirementsKHR" ) )
		{
			if ( xrRequirements.minApiVersionSupported
				> XR_MAKE_VERSION(oxr.m_pOpenGL->m_versionGL[0],
									oxr.m_pOpenGL->m_versionGL[1], 0) )
			{
				Trace( "not supported OpenGL version for OpenXR.\n" ) ;
			}
		}
	}
	//
	// OpenGL バインディング
	//
	SGLOpenGLWindowProducer *	pOpenGLWP =
			ESLTypeCast<SGLOpenGLWindowProducer>( oxr.m_pOpenGL ) ;
	if ( pOpenGLWP == nullptr )
	{
		return	false ;
	}
	XrGraphicsBindingOpenGLWin32KHR
			oglBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR } ;
	oglBinding.hDC = pOpenGLWP->GetRenderingContext().hDC ;
	oglBinding.hGLRC = pOpenGLWP->GetRenderingContext().hGLRC ;

	oglBinding.next = xsci.next ;
	xsci.next = &oglBinding ;
#endif
	//
	// セッション作成
	//
	XrResult xr = xrCreateSession( instance.m_xrInstance, &xsci, &m_xrSession ) ;
	if ( !XRVerify( xr, "xrCreateSession" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;

	m_xrViewConfigType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ;

	const ViewProperties *
		pvpPrimary = system.GetViewPropertiesAs( m_xrViewConfigType ) ;
	ESLAssert( pvpPrimary != nullptr ) ;
	if ( pvpPrimary != nullptr )
	{
		m_xrBlendMode = pvpPrimary->m_xrBlendMode ;
	}

	for ( size_t i = 0; i < s_ViewConfigTypeCount; i ++ )
	{
		if ( system.m_supportedSecondaryViewConfig.
				Find( s_ViewConfigurationTypes[i] ) >= 0 )
		{
			m_aSecondaryViewConfigs.AddSorted( s_ViewConfigurationTypes[i] ) ;
		}
	}

	//
	// スワップチェーン・フォーマット
	//
	SArraySet<int64_t>	aRuntimeFormats ;
	uint32_t	countSwapchainFormats ;
	xr = xrEnumerateSwapchainFormats( m_xrSession, 0, &countSwapchainFormats, nullptr ) ;
	if ( !XRVerify( xr, "xrEnumerateSwapchainFormats" ) )
	{
		return	false ;
	}
	xr = xrEnumerateSwapchainFormats
		( m_xrSession, countSwapchainFormats,
			&countSwapchainFormats,
			aRuntimeFormats.GetArray(countSwapchainFormats) ) ;
	aRuntimeFormats.FinishArray() ;
	if ( !XRVerify( xr, "xrEnumerateSwapchainFormats" ) )
	{
		return	false ;
	}
	ESLTrace( "OpenXR swapchain color formats;\n" ) ;
	for ( size_t i = 0; i < s_ColorBufferFormatCount; i ++ )
	{
		if ( aRuntimeFormats.Find( s_ColorBufferFormatTypes[i].glType ) >= 0 )
		{
			size_t	j = m_supportedColorGLFormats.Add( s_ColorBufferFormatTypes[i].glType ) ;
			m_supportedColorImageFormats.SetAt( j, s_ColorBufferFormatTypes[i].format ) ;
			ESLTrace( "  %s\n", s_ColorBufferFormatTypes[i].pszGLFormat ) ;
		}
	}
	ESLTrace( ".\nOpenXR swapchain depth formats;\n" ) ;
	for ( size_t i = 0; i < s_DepthBufferFormatCount; i ++ )
	{
		if ( aRuntimeFormats.Find( s_DepthBufferFormatTypes[i].glType ) >= 0 )
		{
			size_t	j = m_supportedDepthGLFormats.Add( s_DepthBufferFormatTypes[i].glType ) ;
			m_supportedDepthImageFormats.SetAt( j, s_DepthBufferFormatTypes[i].format ) ;
			ESLTrace( "  %s\n", s_DepthBufferFormatTypes[i].pszGLFormat ) ;
		}
	}
	ESLTrace( ".\n" ) ;

	//
	// リファレンス・スペース
	//
	uint32_t	countRefSpace ;
	xr = xrEnumerateReferenceSpaces( m_xrSession, 0, &countRefSpace, nullptr ) ;
	if ( !XRVerify( xr, "xrEnumerateReferenceSpaces" ) )
	{
		return	false ;
	}
	m_supportedRefSpaces.SetLength( countRefSpace ) ;
	xr = xrEnumerateReferenceSpaces
		( m_xrSession, countRefSpace,
			&countRefSpace, m_supportedRefSpaces.GetArray() ) ;
	m_supportedRefSpaces.FinishArray() ;
	if ( !XRVerify( xr, "xrEnumerateReferenceSpaces" ) )
	{
		return	false ;
	}
	m_supportedRefSpaces.SortArray() ;

	ESLAssert( m_supportedRefSpaces.FindSorted( XR_REFERENCE_SPACE_TYPE_VIEW ) >= 0 ) ;
	ESLAssert( m_supportedRefSpaces.FindSorted( XR_REFERENCE_SPACE_TYPE_LOCAL ) >= 0 ) ;
	m_supportsStageSpace =
		(m_supportedRefSpaces.FindSorted( XR_REFERENCE_SPACE_TYPE_STAGE ) >= 0) ;
	m_supportsUnboundedSpace =
		(m_supportedRefSpaces.FindSorted( XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT ) >= 0) ;

	return	true ;
}

void SGLOpenXRProducer::SessionContext::DestroySession( void )
{
	if ( m_flagCreated )
	{
		XrResult xr = xrDestroySession( m_xrSession ) ;
		XRVerify( xr, "xrDestroySession" ) ;
		m_flagCreated = false ;
	}
}

SSystem::SArray<XrViewConfigurationType>
	SGLOpenXRProducer::SessionContext::GetAllViewConfigurationTypes( void ) const
{
	SArray<XrViewConfigurationType>	aTypes ;
	aTypes.Add( m_xrViewConfigType ) ;
	aTypes.Merge( 1, m_aSecondaryViewConfigs ) ;
	return	aTypes ;
}


const SGLOpenXRProducer::SupportGLFormatInfo
	SGLOpenXRProducer::s_ColorBufferFormatTypes
		[SGLOpenXRProducer::s_ColorBufferFormatCount] =
{
	{ GL_RGBA8, formatImageABGR, "GL_RGBA8" },
	{ GL_BGRA_EXT, formatImageARGB, "GL_BGRA_EXT" },
	{ GL_RGBA4, formatImageABGR, "GL_RGBA4" },
	{ GL_RGBA, formatImageABGR, "GL_RGBA" },
	{ GL_SRGB8_ALPHA8, formatImageABGR, "GL_SRGB8_ALPHA8" },
	{ GL_SRGB_ALPHA, formatImageABGR, "GL_SRGB_ALPHA" },
	{ GL_RGB, formatImageBGR, "GL_RGB" },
	{ GL_SRGB8, formatImageBGR, "GL_SRGB8" },
	{ GL_SRGB, formatImageBGR, "GL_SRGB" },
} ;

const SGLOpenXRProducer::SupportGLFormatInfo
	SGLOpenXRProducer::s_DepthBufferFormatTypes
		[SGLOpenXRProducer::s_DepthBufferFormatCount]
{
	{ GL_DEPTH_COMPONENT32, formatImageDepth, "GL_DEPTH_COMPONENT32" },
	{ GL_DEPTH_COMPONENT32F, formatImageDepth, "GL_DEPTH_COMPONENT32F" },
	{ GL_DEPTH_COMPONENT24, formatImageDepth, "GL_DEPTH_COMPONENT24" },
	{ GL_DEPTH_COMPONENT16, formatImageDepth, "GL_DEPTH_COMPONENT16" },
	{ GL_DEPTH24_STENCIL8, formatImageDepth, "GL_DEPTH24_STENCIL8" },
	{ GL_DEPTH32F_STENCIL8, formatImageDepth, "GL_DEPTH32F_STENCIL8" },
} ;




//////////////////////////////////////////////////////////////////////////////
// アクション
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::Action::Action( void )
	: m_flagCreated( false ), m_xrAction( XR_NULL_HANDLE )
{
}

SGLOpenXRProducer::Action::~Action( void )
{
	DestroyAction() ;
}

bool SGLOpenXRProducer::Action::CreateAction
	( const InstanceContext& instance,
		XrActionSet xrActionSet,
		const char* actionName,
		const char* localizedName,
		XrActionType actionType,
		const char *const* ppSubactPaths,
		size_t nSubactPathCount )
{
	ESLAssert( !m_flagCreated ) ;
	for ( size_t i = 0; i < nSubactPathCount; i ++ )
	{
		m_aSubactPaths.Add( instance.StringToPath( ppSubactPaths[i] ) ) ;
	}

	XrActionCreateInfo	xaci{ XR_TYPE_ACTION_CREATE_INFO } ;
	xaci.actionType = actionType ;
	xaci.countSubactionPaths = static_cast<uint32_t>( m_aSubactPaths.GetLength() ) ;
	xaci.subactionPaths = m_aSubactPaths.GetConstArray() ;
	strcpy_s( xaci.actionName, actionName ) ;
	strcpy_s( xaci.localizedActionName, localizedName ) ;

	XrResult	xr = xrCreateAction( xrActionSet, &xaci, &m_xrAction ) ;
	if ( !XRVerify( xr, "xrCreateAction" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;
	return	true ;

}

void SGLOpenXRProducer::Action::DestroyAction( void )
{
	if ( m_flagCreated )
	{
		XrResult	xr = xrDestroyAction( m_xrAction ) ;
		XRVerify( xr, "xrDestroySpace" ) ;
		m_flagCreated = false ;
	}
}

bool SGLOpenXRProducer::Action::GetActionStateBoolean
	( const SessionContext& session,
		XrActionStateBoolean& stateResult, XrPath pathSubaction ) const
{
	ESLAssert( m_flagCreated ) ;

	XrActionStateGetInfo	xasGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO } ;
	xasGetInfo.action = m_xrAction ;
	xasGetInfo.subactionPath = pathSubaction ;

	XrActionStateBoolean	xaStateBoolean{ XR_TYPE_ACTION_STATE_BOOLEAN } ;

	XrResult	xr = 
		xrGetActionStateBoolean( session.m_xrSession, &xasGetInfo, &xaStateBoolean ) ;
	stateResult = xaStateBoolean ;

	return	XRVerify( xr, "xrGetActionStateBoolean" ) ;
}

bool SGLOpenXRProducer::Action::GetActionStateFloat
	( const SessionContext& session,
		XrActionStateFloat& stateResult, XrPath pathSubaction ) const
{
	ESLAssert( m_flagCreated ) ;

	XrActionStateGetInfo	xasGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO } ;
	xasGetInfo.action = m_xrAction ;
	xasGetInfo.subactionPath = pathSubaction ;

	XrActionStateFloat	xaStateFloat{ XR_TYPE_ACTION_STATE_FLOAT } ;

	XrResult	xr = 
		xrGetActionStateFloat( session.m_xrSession, &xasGetInfo, &xaStateFloat ) ;
	stateResult = xaStateFloat ;

	return	XRVerify( xr, "xrGetActionStateFloat" ) ;
}

bool SGLOpenXRProducer::Action::GetActionStateVector2f
	( const SessionContext& session,
		XrActionStateVector2f& stateResult, XrPath pathSubaction ) const
{
	ESLAssert( m_flagCreated ) ;

	XrActionStateGetInfo	xasGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO } ;
	xasGetInfo.action = m_xrAction ;
	xasGetInfo.subactionPath = pathSubaction ;

	XrActionStateVector2f	xaStateVector2f{ XR_TYPE_ACTION_STATE_VECTOR2F } ;

	XrResult	xr = 
		xrGetActionStateVector2f( session.m_xrSession, &xasGetInfo, &xaStateVector2f ) ;
	stateResult = xaStateVector2f ;

	return	XRVerify( xr, "xrGetActionStateVector2f" ) ;
}

bool SGLOpenXRProducer::Action::GetActionStatePose
	( const SessionContext& session,
		XrActionStatePose& stateResult, XrPath pathSubaction ) const
{
	ESLAssert( m_flagCreated ) ;

	XrActionStateGetInfo	xasGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO } ;
	xasGetInfo.action = m_xrAction ;
	xasGetInfo.subactionPath = pathSubaction ;

	XrActionStatePose	xaStatePose{ XR_TYPE_ACTION_STATE_POSE } ;

	XrResult	xr = 
		xrGetActionStatePose( session.m_xrSession, &xasGetInfo, &xaStatePose ) ;
	stateResult = xaStatePose ;

	XRVerify( xr, "xrGetActionStatePose" ) ;
	return	(xr == XR_SUCCESS) ;
}



//////////////////////////////////////////////////////////////////////////////
// アクション・セット
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ActionSet::ActionSet( const InstanceContext& instance )
	: m_instance( instance ),
		m_flagCreated( false ),
		m_flagActive( true ), m_xrActionSet( XR_NULL_HANDLE )
{
}

SGLOpenXRProducer::ActionSet::~ActionSet( void )
{
	DestroyActionSet() ;
}

bool SGLOpenXRProducer::ActionSet::CreateActionSet
	( const char* name, const char* localizedName, uint32_t priority )
{
	ESLAssert( !m_flagCreated ) ;

	XrActionSetCreateInfo	xasci{ XR_TYPE_ACTION_SET_CREATE_INFO } ;
	strcpy_s( xasci.actionSetName, name ) ;
	strcpy_s( xasci.localizedActionSetName, localizedName ) ;
	xasci.priority = priority ;

	XrResult	xr = xrCreateActionSet
		( m_instance.m_xrInstance, &xasci, &m_xrActionSet ) ;
	if ( !XRVerify( xr, "xrCreateActionSet" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;
	return	true ;
}

void SGLOpenXRProducer::ActionSet::DestroyActionSet( void )
{
	m_actions.RemoveAll() ;
	m_setDeclPaths.RemoveAll() ;

	if ( m_flagCreated )
	{
		XrResult	xr = xrDestroyActionSet( m_xrActionSet ) ;
		XRVerify( xr, "xrDestroyActionSet" ) ;
		m_flagCreated = false ;
	}
}

SGLOpenXRProducer::Action * SGLOpenXRProducer::ActionSet::CreateAction
	( const char* actionName,
		const char* localizedName,
		XrActionType actionType,
		const char *const* ppSubactPaths,
		size_t nSubactPathCount )
{
	ESLAssert( m_flagCreated ) ;

	Action *	pAct = new Action ;
	if ( !pAct->CreateAction
		( m_instance, m_xrActionSet,
			actionName, localizedName,
			actionType, ppSubactPaths, nSubactPathCount ) )
	{
		delete	pAct ;
		return	nullptr ;
	}
	m_actions.Add( pAct ) ;

	for ( size_t i = 0; i < pAct->m_aSubactPaths.GetLength(); i ++ )
	{
		m_setDeclPaths.AddSorted( pAct->m_aSubactPaths.At(i) ) ;
	}
	return	pAct ;
}

void SGLOpenXRProducer::ActionSet::SetActive( bool flagActive )
{
	m_flagActive = flagActive ;
}

bool SGLOpenXRProducer::ActionSet::IsActived( void ) const
{
	return	m_flagActive ;
}



//////////////////////////////////////////////////////////////////////////////
// アクション・コンテキスト
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ActionContext::ActionContext( const InstanceContext& instance )
	: m_instance( instance )
{
}

SGLOpenXRProducer::ActionContext::~ActionContext( void )
{
}

SGLOpenXRProducer::ActionSet *
	SGLOpenXRProducer::ActionContext::CreateActionSet
		( const char* name, const char* localizedName, uint32_t priority )
{
	ActionSet *	pActSet = new ActionSet( m_instance ) ;
	if ( !pActSet->CreateActionSet( name, localizedName, priority ) )
	{
		delete	pActSet ;
		return	nullptr ;
	}
	m_aActionSets.Add( pActSet ) ;
	return	pActSet ;
}

void SGLOpenXRProducer::ActionContext::SuggestInteractionProfileBindings
	( const char* interactionProfile,
		const ActionBinding* pActBindings, size_t nCountBindings )
{
	const XrPath	xrProfile = m_instance.StringToPath( interactionProfile ) ;

	ActionBindingArray *	pActBinding = m_mapActionBinding.GetAs( xrProfile ) ;
	if ( pActBinding == nullptr )
	{
		pActBinding = new ActionBindingArray ;
		m_mapActionBinding.SetAs( xrProfile, pActBinding ) ;
	}

	for ( size_t i = 0; i < nCountBindings; i ++ )
	{
		XrActionSuggestedBinding	xasb ;
		xasb.action = pActBindings[i].action ;
		xasb.binding = m_instance.StringToPath( pActBindings[i].binding ) ;

		pActBinding->Add( xasb ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// リファレンス・スペース
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ReferenceSpace::ReferenceSpace( void )
	: m_flagCreated( false ), m_xrRefSpace( XR_NULL_HANDLE )
{
}

SGLOpenXRProducer::ReferenceSpace::~ReferenceSpace( void )
{
	DestroySpace() ;
}

bool SGLOpenXRProducer::ReferenceSpace::CreateSpace
	( const SessionContext& session, XrReferenceSpaceType xrrsType )
{
	ESLAssert( !m_flagCreated ) ;

	XrReferenceSpaceCreateInfo	xrsci{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO } ;
	xrsci.referenceSpaceType = xrrsType ;
	xrsci.poseInReferenceSpace.orientation.x = 0 ;
	xrsci.poseInReferenceSpace.orientation.y = 0 ;
	xrsci.poseInReferenceSpace.orientation.z = 0 ;
	xrsci.poseInReferenceSpace.orientation.w = 1 ;
	xrsci.poseInReferenceSpace.position.x = 0 ;
	xrsci.poseInReferenceSpace.position.y = 0 ;
	xrsci.poseInReferenceSpace.position.z = 0 ;

	XrResult	xr = 
		xrCreateReferenceSpace
			( session.m_xrSession, &xrsci, &m_xrRefSpace ) ;
	if ( !XRVerify( xr, "xrCreateReferenceSpace" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;
	return	true ;
}

bool SGLOpenXRProducer::ReferenceSpace::CreateActionSpace
	( const SessionContext& session,
		SGLOpenXRProducer::Action * pAction, XrPath xrSubactionPath )
{
	ESLAssert( !m_flagCreated ) ;

	XrActionSpaceCreateInfo	xasci{ XR_TYPE_ACTION_SPACE_CREATE_INFO } ;

	xasci.poseInActionSpace.orientation.x = 0.0f ;
	xasci.poseInActionSpace.orientation.y = 0.0f ;
	xasci.poseInActionSpace.orientation.z = 0.0f ;
	xasci.poseInActionSpace.orientation.w = 1.0f ;
	xasci.poseInActionSpace.position.x = 0.0f ;
	xasci.poseInActionSpace.position.y = 0.0f ;
	xasci.poseInActionSpace.position.z = 0.0f ;

	xasci.subactionPath = xrSubactionPath ;
	xasci.action = pAction->m_xrAction ;

	XrResult	xr =
		xrCreateActionSpace( session.m_xrSession, &xasci, &m_xrRefSpace ) ;
	if ( !XRVerify( xr, "xrCreateReferenceSpace" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;
	return	true ;
}

void SGLOpenXRProducer::ReferenceSpace::DestroySpace( void )
{
	if ( m_flagCreated )
	{
		XrResult	xr = xrDestroySpace( m_xrRefSpace ) ;
		XRVerify( xr, "xrDestroySpace" ) ;
		m_flagCreated = false ;
	}
}

bool SGLOpenXRProducer::ReferenceSpace::LocateSpace
	( XrPosef& poseDst,
		const ReferenceSpace& spaceApp, const XrTime& time ) const
{
	XrSpaceLocation	xsl{ XR_TYPE_SPACE_LOCATION } ;
	XrResult	xr = xrLocateSpace
		( m_xrRefSpace, spaceApp.m_xrRefSpace, time, &xsl ) ;
	if ( !XRVerify( xr, "xrLocateSpace" ) )
	{
		return	false ;
	}
	poseDst = xsl.pose ;
	return	true ;
}




//////////////////////////////////////////////////////////////////////////////
// フレーム設定 FrameConfig
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::FrameConfig::FrameConfig( void )
	: m_flagsLayer( XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT ),
		m_glColorFormat( GL_RGBA8 ),
		m_formatColor( formatImageABGR ),
		m_glDepthFormat( GL_DEPTH_COMPONENT32 ),
		m_formatDepth( formatImageDepth ),
		m_countSwapchainSample( 1 ),
		m_modeDoubleWide( false ),
		m_flagSubmitDepthInfo( true ),
		m_flagContentProtected( false ),
		m_flagForceReset( false )
{
	m_extentSizeScale.width = 1.0f ;
	m_extentSizeScale.height = 1.0f ;
	m_extentFovScale.width = 1.0f ;
	m_extentFovScale.height = 1.0f ;
	m_extentViewportScale.width = 1.0f ;
	m_extentViewportScale.height = 1.0f ;
	m_offsetViewport.x = 0 ;
	m_offsetViewport.y = 0 ;
}

const SGLOpenXRProducer::FrameConfig&
	SGLOpenXRProducer::FrameConfig::operator = ( const FrameConfig& fc )
{
	m_flagsLayer = fc.m_flagsLayer ;
	m_glColorFormat = fc.m_glColorFormat ;
	m_formatColor = fc.m_formatColor ;
	m_glDepthFormat = fc.m_glDepthFormat ;
	m_formatDepth = fc.m_formatDepth ;
	m_extentSizeScale = fc.m_extentSizeScale ;
	m_extentFovScale = fc.m_extentFovScale ;
	m_extentViewportScale = fc.m_extentViewportScale ;
	m_offsetViewport = fc.m_offsetViewport ;
	m_countSwapchainSample = fc.m_countSwapchainSample ;
	m_modeDoubleWide = fc.m_modeDoubleWide ;
	m_flagSubmitDepthInfo = fc.m_flagSubmitDepthInfo ;
	m_flagContentProtected = fc.m_flagContentProtected ;
	m_flagForceReset = fc.m_flagForceReset ;
	m_xclri = fc.m_xclri ;
	m_xclrpo = fc.m_xclrpo ;
	return	*this ;
}

bool SGLOpenXRProducer::FrameConfig::DoesNeedResetFrameFor( const FrameConfig& fc ) const
{
	return	fc.m_flagForceReset
			|| (m_glColorFormat != fc.m_glColorFormat)
			|| (m_glDepthFormat != fc.m_glDepthFormat)
			|| (m_modeDoubleWide != fc.m_modeDoubleWide)
			|| (m_countSwapchainSample != fc.m_countSwapchainSample)
			|| (m_extentSizeScale.width != fc.m_extentSizeScale.width)
			|| (m_extentSizeScale.height != fc.m_extentSizeScale.height)
			|| (m_flagContentProtected != fc.m_flagContentProtected) ;
}



//////////////////////////////////////////////////////////////////////////////
// スワップチェーン・バッファ SwapchainBuffer
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::SwapchainBuffer::SwapchainBuffer( void )
	: m_flagCreated( false ), m_flagAcquired( false ),
		m_xrSwapchain( XR_NULL_HANDLE ),
		m_glFormat(0), m_width(0), m_height(0), m_iAcquireSwapchain(0)
{
}

SGLOpenXRProducer::SwapchainBuffer::~SwapchainBuffer( void )
{
	ESLAssert( !m_flagAcquired ) ;
	DestroySwapchain() ;
}

bool SGLOpenXRProducer::SwapchainBuffer::CreateSwapchain
	( SGLOpenGLContext * pOpenGL,
		const SGLOpenXRProducer::SessionContext& session,
		GLint glFormat, uint32_t format, int32_t width, int32_t height,
		uint32_t lengthArray, uint32_t countSample,
		XrSwapchainCreateFlags flagsCreate,
		XrSwapchainUsageFlags flagsUsage,
		const XrViewConfigurationType * pViewcfgType )
{
	DestroySwapchain() ;

	m_glFormat = glFormat ;
	m_width = width ;
	m_height = height ;

	XrSwapchainCreateInfo	xsci{ XR_TYPE_SWAPCHAIN_CREATE_INFO } ;
	xsci.createFlags = flagsCreate ;
	xsci.usageFlags = flagsUsage ;
	xsci.format = glFormat ;
	xsci.sampleCount = countSample ;
	xsci.width = width ;
	xsci.height = height ;
	xsci.mipCount = 1 ;
	xsci.faceCount = 1 ;
	xsci.arraySize = lengthArray ;

	XrSecondaryViewConfigurationSwapchainCreateInfoMSFT
		xsvcsci{ XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT } ;
	if ( pViewcfgType != nullptr )
	{
		xsvcsci.viewConfigurationType = *pViewcfgType ;
		xsci.next = &xsvcsci ;
	}

	XrResult	xr = xrCreateSwapchain( session.m_xrSession, &xsci, &m_xrSwapchain ) ;
	if ( !XRVerify( xr, "xrCreateSwapchain" ) )
	{
		return	false ;
	}
	m_flagCreated = true ;

	uint32_t	countImages ;
	xr = xrEnumerateSwapchainImages( m_xrSwapchain, 0, &countImages, nullptr ) ;
	if ( XRVerify( xr, "xrEnumerateSwapchainImages" ) )
	{
		m_aSwapchainOGL.SetLength( countImages ) ;
		for ( size_t i = 0; i < countImages; i ++ )
		{
			m_aSwapchainOGL.At(i).type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR ;
		}
		xr = xrEnumerateSwapchainImages
			( m_xrSwapchain, countImages, &countImages,
				reinterpret_cast<XrSwapchainImageBaseHeader*>(m_aSwapchainOGL.GetArray()) ) ;
		if ( XRVerify( xr, "xrEnumerateSwapchainImages" ) )
		{
			m_aSwapchainOGL.FinishArray() ;
			//
			m_aImages.SetLength( countImages ) ;
			for ( size_t i= 0; i < countImages; i ++ )
			{
				SGLImage *	pImage = new SGLImage ;
				m_aImages.SetAt( i, pImage ) ;
				//
				pImage->CreateImage
					( width, height, format, 32,
						SGLImageObject::bufferNonPowerOf2
						| SGLImageObject::bufferOnDeviceOnly
						| SGLImageObject::bufferTextureArray, lengthArray ) ;
				SGLOpenGLTextureBuffer::AttachGLTexture
					( pOpenGL, pImage, m_aSwapchainOGL.At(i).image, false ) ;
			}
		}
		else
		{
			m_aSwapchainOGL.RemoveAll() ;
		}
	}
	return	true ;
}

void SGLOpenXRProducer::SwapchainBuffer::DestroySwapchain( void )
{
	if ( m_flagAcquired )
	{
		ReleaseSwapchain() ;
	}
	if ( m_flagCreated )
	{
		XrResult	xr = xrDestroySwapchain( m_xrSwapchain ) ;
		XRVerify( xr, "xrDestroySwapchain" ) ;
		m_flagCreated = false ;
		m_xrSwapchain = XR_NULL_HANDLE ;
		m_aSwapchainOGL.RemoveAll() ;
		m_aImages.RemoveAll() ;
	}
}

bool SGLOpenXRProducer::SwapchainBuffer::AcquireSwapchain( void )
{
	ESLAssert( m_flagCreated ) ;
	ESLAssert( !m_flagAcquired ) ;

	XrSwapchainImageAcquireInfo	xsiai{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO } ;
	XrResult	xr = 
		xrAcquireSwapchainImage( m_xrSwapchain, &xsiai, &m_iAcquireSwapchain ) ;
	if ( !XRVerify( xr, "xrAcquireSwapchainImage" ) )
	{
		return	false ;
	}
	m_flagAcquired = true ;
	return	true ;
}

bool SGLOpenXRProducer::SwapchainBuffer::WaitSwapchain( void )
{
	ESLAssert( m_flagCreated ) ;
	ESLAssert( m_flagAcquired ) ;

	XrSwapchainImageWaitInfo	xsiwi{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO } ;
	xsiwi.timeout = XR_INFINITE_DURATION ;

	XrResult	xr = xrWaitSwapchainImage( m_xrSwapchain, &xsiwi ) ;
	XRVerify( xr, "xrWaitSwapchainImage" ) ;

	return	(xr == XR_SUCCESS) ;
}

bool SGLOpenXRProducer::SwapchainBuffer::ReleaseSwapchain( void )
{
	ESLAssert( m_flagCreated ) ;
	ESLAssert( m_flagAcquired ) ;

	XrSwapchainImageReleaseInfo	xsiri{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO } ;
	XrResult	xr = xrReleaseSwapchainImage( m_xrSwapchain, &xsiri ) ;

	m_flagAcquired = false ;
	return	XRVerify( xr, "xrReleaseSwapchainImage" ) ;
}

SGLImage * SGLOpenXRProducer::SwapchainBuffer::GetAcquired( void ) const
{
	ESLAssert( m_flagCreated ) ;
	ESLAssert( m_flagAcquired ) ;
	return	m_aImages.GetAt( m_iAcquireSwapchain ) ;
}




//////////////////////////////////////////////////////////////////////////////
// ビュー・コンテキスト ViewContext
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ViewContext::ViewContext( void )
	: m_active ( false ),
		m_locateViews( false ),
		m_locateSpace( false ),
		m_xrSpace(XR_NULL_HANDLE),
		m_pLastColor( nullptr ),
		m_pLastDepth( nullptr )
{
}

// ビュー設定（m_viewConfigs）列挙
bool SGLOpenXRProducer::ViewContext::CreateViewConfiguration
	( XrViewConfigurationType xcvType,
		const InstanceContext& instance, const SystemContext& system )
{
	m_xrvcType = xcvType ;
	m_active = false ;

	if ( !system.EnumerateViewConfigurationViews
					( m_viewConfigs, xcvType, instance ) )
	{
		return	false ;
	}

	m_views.SetLength( m_viewConfigs.GetLength() ) ;
	for ( size_t i = 0; i < m_views.GetLength(); i ++ )
	{
		m_views.At(i).type = XR_TYPE_VIEW ;
	}
	return	true ;
}

// ビューのアクティブ状態更新とスワップチェーン再生成判定
bool SGLOpenXRProducer::ViewContext::UpdateStateActive
	( bool flagActive,
		const InstanceContext& instance, const SystemContext& system )
{
	if ( m_active == flagActive )
	{
		return	false ;
	}
	m_active = flagActive ;

	if ( flagActive )
	{
		SArray<XrViewConfigurationView>	aViewCfgViews ;
		if ( system.EnumerateViewConfigurationViews
					( aViewCfgViews, m_xrvcType, instance ) )
		{
			return	false ;
		}
		// スワップチェイン・サイズ変更判定
		bool	flagSwapchaiunSizeChanged =
					(aViewCfgViews.GetLength() != m_viewConfigs.GetLength()) ;
		if ( !flagSwapchaiunSizeChanged )
		{
			for ( size_t i = 0; i < aViewCfgViews.GetLength(); i ++ )
			{
				const XrViewConfigurationView&	xvcvNew = aViewCfgViews.At(i) ;
				const XrViewConfigurationView&	xvcvOld = m_viewConfigs.At(i) ;
				if ( (xvcvNew.recommendedImageRectWidth != xvcvOld.recommendedImageRectWidth)
					|| (xvcvNew.recommendedImageRectHeight != xvcvOld.recommendedImageRectHeight) )
				{
					flagSwapchaiunSizeChanged = true ;
					break ;
				}
			}
		}
		if ( flagSwapchaiunSizeChanged )
		{
			m_viewConfigs = aViewCfgViews ;
			SetFrameResetFlag() ;
			return	true ;
		}
	}
	return	false ;
}

// ビューの視野角と視差（m_views）取得
bool SGLOpenXRProducer::ViewContext::LocateViews
	( const SessionContext& session,
		const ReferenceSpace& spaceView, const XrTime& time )
{
	// Locate the views in VIEW space to get the per-view offset from the VIEW "camera"
	XrViewState	xvs{ XR_TYPE_VIEW_STATE } ;

	XrViewLocateInfo	xvli{ XR_TYPE_VIEW_LOCATE_INFO } ;
	xvli.viewConfigurationType = m_xrvcType ;
	xvli.displayTime = time ;
	xvli.space = spaceView.m_xrRefSpace ;

	uint32_t	countView = 0 ;
	XrResult	xr = xrLocateViews
		( session.m_xrSession, &xvli, &xvs,
			(uint32_t) m_views.GetLength(), &countView, m_views.GetArray() ) ;
	m_views.FinishArray() ;
	if ( !XRVerify( xr, "xrLocateViews" ) )
	{
		return	false ;
	}
	ESLAssert( countView == m_views.GetLength() ) ;
	m_locateViews = true ;
	return	true ;
}

// ベース空間（m_xrPoseView）取得
bool SGLOpenXRProducer::ViewContext::LocateSpace
	( const ReferenceSpace& spaceView,
		const ReferenceSpace& spaceApp, const XrTime& time )
{
	if ( !spaceView.LocateSpace( m_xrPoseView, spaceApp, time ) )
	{
		return	false ;
	}
	m_locateSpace = true ;
	return	true ;
}

// ビュー（視差）数取得
size_t SGLOpenXRProducer::ViewContext::GetViewCount( void ) const
{
	return	m_views.GetLength() ;
}

// HMD 姿勢取得
bool SGLOpenXRProducer::ViewContext::GetHeadPosture
	( const SGLOpenXRProducer& oxr, Posture& postureHead ) const
{
	ESLAssert( m_locateSpace ) ;
	if ( m_locateSpace )
	{
		oxr.GetPostureFromXrPosef( postureHead, m_xrPoseView ) ;
		postureHead.nFlags = trackedOrientation | trackedPosition ;
	}
	return	m_locateSpace ;
}

// 視野情報取得
bool SGLOpenXRProducer::ViewContext::GetEyeFieldOfView
	( const SGLOpenXRProducer& oxr, EyeFieldOfView& eyeFOV, size_t iEye ) const
{
	ESLAssert( m_locateViews ) ;
	if ( iEye >= m_views.GetLength() )
	{
		return	false ;
	}
	const XrView&	xrView = m_views.At( iEye ) ;

	eyeFOV.sizeOfView.w = m_swapchainColor.m_width ;
	eyeFOV.sizeOfView.h = m_swapchainColor.m_height ;
	//
	float32_t	wFovTex = (float32_t) m_swapchainColor.m_width ;
	float32_t	hFovTex = (float32_t) m_swapchainColor.m_height ;
	float32_t	tanLeft = (float32_t) fabs(tan(xrView.fov.angleLeft)) ;
	float32_t	tanRight = (float32_t) fabs(tan(xrView.fov.angleRight)) ;
	float32_t	tanUp = (float32_t) fabs(tan(xrView.fov.angleUp)) ;
	float32_t	tanDown = (float32_t) fabs(tan(xrView.fov.angleDown)) ;
	if ( !m_locateViews )
	{
		tanLeft = 1.0f ;
		tanRight = 1.0f ;
		tanUp = 1.0f ;
		tanDown = 1.0f ;
	}
	float32_t	wFOV = tanRight + tanLeft ;
	float32_t	hFOV = tanUp + tanDown ;
	float32_t	zScreen = (float32_t) m_swapchainColor.m_height / hFOV ;
	//
	eyeFOV.vScreenPos.x = wFovTex * tanLeft / wFOV ;
	eyeFOV.vScreenPos.y = hFovTex * tanUp / hFOV ;
	eyeFOV.vScreenPos.z = zScreen ;
	eyeFOV.fpPixelAspect = (wFOV * hFovTex) / (hFOV * wFovTex) ;
	//
	return	m_locateViews ;
}

// 視差情報取得
bool SGLOpenXRProducer::ViewContext::GetEyePosture
	( const SGLOpenXRProducer& oxr, Posture& postureEye, size_t iEye ) const
{
	ESLAssert( m_locateViews ) ;
	if ( !m_locateViews || (iEye >= m_views.GetLength()) )
	{
		return	false ;
	}
	const XrView&	xrView = m_views.At( iEye ) ;
	oxr.GetPostureFromXrPosef( postureEye, xrView.pose ) ;
	postureEye.nFlags = trackedOrientation | trackedPosition ;
	return	true ;
}

// スワップチェーン準備処理
void SGLOpenXRProducer::ViewContext::InitializeFrame( const SessionContext& session )
{
	m_cfgPending.m_glColorFormat = session.m_supportedColorGLFormats.At(0) ;
	m_cfgPending.m_formatColor = session.m_supportedColorImageFormats.At(0) ;
	m_cfgPending.m_glDepthFormat = session.m_supportedDepthGLFormats.At(0) ;
	m_cfgPending.m_formatDepth = session.m_supportedDepthImageFormats.At(0) ;

	if ( m_xrvcType == XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT )
	{
		m_cfgPending.m_flagsLayer = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT ;
	}

	SetFrameResetFlag() ;
}

// スワップチェーン生成／再生成
bool SGLOpenXRProducer::ViewContext::PrepareFrame( const SGLOpenXRProducer& oxr )
{
	//
	// スワップチェーン再生成判定
	//
	bool	shouldResetSwapchain =
				m_cfgPending.DoesNeedResetFrameFor( m_cfgCurrent ) ;

	if ( (m_swapchainColor.m_xrSwapchain == XR_NULL_HANDLE)
		|| (m_swapchainDepth.m_xrSwapchain == XR_NULL_HANDLE) )
	{
		shouldResetSwapchain = true ;
	}

	m_cfgPending.m_flagForceReset = false ;
	m_cfgCurrent = m_cfgPending ;

	//
	// 画像サイズ計算
	//
	uint32_t	widthRecommended = 1 ;
	uint32_t	heightRecommended = 1 ;
	for ( size_t i = 0; i < m_viewConfigs.GetLength(); i ++ )
	{
		const XrViewConfigurationView&	xvcv = m_viewConfigs.At(i) ;
		if ( widthRecommended < xvcv.recommendedImageRectWidth )
		{
			widthRecommended = xvcv.recommendedImageRectWidth ;
		}
		if ( heightRecommended < xvcv.recommendedImageRectHeight )
		{
			heightRecommended = xvcv.recommendedImageRectHeight ;
		}
	}
	ESLAssert( widthRecommended > 1 ) ;
	ESLAssert( heightRecommended > 1 ) ;

	const uint32_t	widthSwapchain =
			(uint32_t) esl_roundfi
				( (float32_t) widthRecommended
						* m_cfgCurrent.m_extentSizeScale.width ) ;
	const uint32_t	heightSwapchain =
			(uint32_t) esl_roundfi
				( (float32_t) heightRecommended
						* m_cfgCurrent.m_extentSizeScale.height ) ;
	const uint32_t	countSwapchainSample =
		(m_cfgCurrent.m_countSwapchainSample < 1)
			? m_viewConfigs.At(0).recommendedSwapchainSampleCount
			: m_cfgCurrent.m_countSwapchainSample ;

	//
	// ビューポート計算
	//
	m_viewports.SetLength( m_viewConfigs.GetLength() ) ;
	m_aColorImageRects.SetLength( m_viewConfigs.GetLength() ) ;
	m_aDepthImageRects.SetLength( m_viewConfigs.GetLength() ) ;

	for ( uint32_t i = 0; i < m_viewConfigs.GetLength(); i ++ )
	{
		SGLImageRect&	rectView = m_viewports.At(i) ;
		rectView.x = m_cfgCurrent.m_offsetViewport.x ;
		rectView.y = m_cfgCurrent.m_offsetViewport.y ;
		rectView.w = (int32_t) esl_roundfi
			( (float32_t) widthSwapchain
					* m_cfgCurrent.m_extentViewportScale.width ) ;
		rectView.h = (int32_t) esl_roundfi
			( (float32_t) heightSwapchain
					* m_cfgCurrent.m_extentViewportScale.height ) ;
		//
		XrRect2Di	rectImage ;
		rectImage.offset.x = 0 ;
		rectImage.offset.y = 0 ;
		rectImage.extent.width = widthSwapchain ;
		rectImage.extent.height = heightSwapchain ;
		//
		if ( m_cfgCurrent.m_modeDoubleWide )
		{
			rectView.x += widthSwapchain * i ;
			rectImage.offset.x += widthSwapchain * i ;
		}
		m_aColorImageRects.SetAt( i, rectImage ) ;
		m_aDepthImageRects.SetAt( i, rectImage ) ;
	}

	if ( !shouldResetSwapchain )
	{
		return	true ;
	}

	//
	// スワップチェーン作成
	//
	const uint32_t	nWideScale = m_cfgCurrent.m_modeDoubleWide ? 2 : 1 ;
	const uint32_t	lengthArray = m_cfgCurrent.m_modeDoubleWide
									? 1 : (uint32_t) m_viewConfigs.GetLength() ;
	const XrViewConfigurationType *	pxvcType = nullptr ;
	if ( oxr.GetXRExtension().supportsSecondaryViewConfiguration )
	{
		pxvcType = &m_xrvcType ;
	}

	if ( !m_swapchainColor.CreateSwapchain
			( oxr.m_pOpenGL,
				oxr.GetXRSession(),
				m_cfgCurrent.m_glColorFormat,
				m_cfgCurrent.m_formatColor,
				widthSwapchain * nWideScale,
				heightSwapchain,
				lengthArray,
				countSwapchainSample,
				(m_cfgCurrent.m_flagContentProtected
					? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0),
				XR_SWAPCHAIN_USAGE_SAMPLED_BIT
					| XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
				pxvcType ) )
	{
		return	false ;
	}
	if ( !m_swapchainDepth.CreateSwapchain
			( oxr.m_pOpenGL,
				oxr.GetXRSession(),
				m_cfgCurrent.m_glDepthFormat,
				m_cfgCurrent.m_formatDepth,
				widthSwapchain * nWideScale,
				heightSwapchain,
				lengthArray,
				countSwapchainSample,
				(m_cfgCurrent.m_flagContentProtected
					? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0),
				XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				pxvcType ) )
	{
		return	false ;
	}

	m_aProjectionViews.SetLength( m_viewConfigs.GetLength() ) ;
	m_aDepthInfos.SetLength( m_viewConfigs.GetLength() ) ;

	return	true ;
}

// スワップチェーン破棄
void SGLOpenXRProducer::ViewContext::ReleaseFrame( void )
{
	m_swapchainColor.DestroySwapchain() ;
	m_swapchainDepth.DestroySwapchain() ;
	SetFrameResetFlag() ;
}

// スワップチェーン再生成フラグ設定
void SGLOpenXRProducer::ViewContext::SetFrameResetFlag( void )
{
	m_cfgPending.m_flagForceReset = true ;
}

// スワップチェーン取得
bool SGLOpenXRProducer::ViewContext::AcquireSwapchain( void )
{
	if ( m_swapchainColor.AcquireSwapchain()
		&& m_swapchainDepth.AcquireSwapchain() )
	{
		return	m_swapchainColor.WaitSwapchain()
				&& m_swapchainDepth.WaitSwapchain() ;
	}
	return	false ;
}

// スワップチェーン取得解放
void SGLOpenXRProducer::ViewContext::ReleaseSwapchain( void )
{
	m_swapchainColor.ReleaseSwapchain() ;
	m_swapchainDepth.ReleaseSwapchain() ;
}

// スワップチェーンをレンダラに関連付け
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::ViewContext::AttachSwapchainToRenderer
		( const SGLOpenXRProducer& oxr, S3DRenderParameterContext& render )
{
	SGLImage *	pColor = m_swapchainColor.GetAcquired() ;
	SGLImage *	pDepth = m_swapchainDepth.GetAcquired() ;
	m_pLastColor = pColor ;
	m_pLastDepth = pDepth ;
	if ( (pColor == nullptr) || (pDepth == nullptr) )
	{
		return	false ;
	}
	const size_t	nViews = m_views.GetLength() ;
	ESLAssert( pColor->GetFrameCount() == nViews ) ;
	ESLAssert( pDepth->GetFrameCount() == nViews ) ;
	if ( (nViews == 1)
		|| (pColor->GetFrameCount() <= 1)
		|| (pDepth->GetFrameCount() <= 1) )
	{
		render.AttachTargetImage( pColor, pDepth ) ;
	}
	else
	{
		m_pTempColorLeft = pColor->NewReference( nullptr, 0 ) ;
		m_pTempColorRight = pColor->NewReference( nullptr, 1 ) ;
		m_pTempDepthLeft = pDepth->NewReference( nullptr, 0 ) ;
		m_pTempDepthRight = pDepth->NewReference( nullptr, 1 ) ;
		//
		render.AttachStereoTargetImage
			( m_pTempColorRight, m_pTempColorLeft,
				m_pTempDepthRight, m_pTempDepthLeft ) ;
	}
	double	zMin, zMax ;
	render.GetZClipRange( zMin, zMax ) ;

	ESLAssert( m_aColorImageRects.GetLength() >= nViews ) ;
	ESLAssert( m_aDepthImageRects.GetLength() >= nViews ) ;
	for ( uint32_t i = 0; i < nViews; i ++ )
	{
		const XrView&	xrView = m_views.At(i) ;

		const XrFovf	fov = xrView.fov ;
		const uint32_t	iColorArrayIndex = m_cfgCurrent.m_modeDoubleWide ? 0 : i ;
		const uint32_t	iDepthArrayIndex = m_cfgCurrent.m_modeDoubleWide ? 0 : i ;

		XrCompositionLayerProjectionView
					xclpv{ XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW } ;
		xclpv.pose = xrView.pose ;
		xclpv.fov.angleLeft = fov.angleLeft * m_cfgCurrent.m_extentFovScale.width ;
		xclpv.fov.angleRight = fov.angleRight * m_cfgCurrent.m_extentFovScale.width ;
		xclpv.fov.angleUp = fov.angleUp * m_cfgCurrent.m_extentFovScale.height ;
		xclpv.fov.angleDown = fov.angleDown * m_cfgCurrent.m_extentFovScale.height ;
		xclpv.subImage.swapchain = m_swapchainColor.m_xrSwapchain ;
		xclpv.subImage.imageArrayIndex = iColorArrayIndex ;
		xclpv.subImage.imageRect = m_aColorImageRects.At(i) ;

		XrCompositionLayerDepthInfoKHR
					xcldi{ XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR } ;
		xcldi.minDepth = 0.0f ;
		xcldi.maxDepth = 1.0f ;
		xcldi.nearZ = /*0.02f*/ (float) (zMin / oxr.GetScaleHMDToModel()) ;
		xcldi.farZ = /*1000.0f*/ (float) (zMax / oxr.GetScaleHMDToModel()) ;
		xcldi.subImage.swapchain = m_swapchainDepth.m_xrSwapchain ;
		xcldi.subImage.imageArrayIndex = iDepthArrayIndex ;
		xcldi.subImage.imageRect = m_aDepthImageRects.At(i) ;
		m_aDepthInfos.SetAt( i, xcldi ) ;

		if ( m_cfgCurrent.m_flagSubmitDepthInfo
			&& oxr.GetXRExtension().supportsDepthInfo )
		{
			xclpv.next = m_aDepthInfos.GetAt(i) ;
		}
		else
		{
			xclpv.next = nullptr ;
		}
		m_aProjectionViews.SetAt( i, xclpv ) ;
	}
	return	true ;
}

// スワップチェーンをレンダラから分離
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ViewContext::DetachSwapchainFromRenderer
	( S3DRenderParameterContext& render )
{
	render.Finish() ;
	render.DetachTargetImage() ;

	m_pTempColorLeft = nullptr ;
	m_pTempColorRight = nullptr ;
	m_pTempDepthLeft = nullptr ;
	m_pTempDepthRight = nullptr ;
}

// 画面複製用のバッファにレンダラを関連付け
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::ViewContext::AttachBackBufferToRenderer
	( const SGLOpenXRProducer& oxr, S3DRenderParameterContext& renderBack )
{
	if ( (m_pBackColorRight == nullptr)
		|| (m_pBackColorRight->GetImageWidth() != (uint32_t) m_swapchainColor.m_width)
		|| (m_pBackColorRight->GetImageHeight() != (uint32_t) m_swapchainColor.m_height) )
	{
		m_pBackColorLeft = new SGLImage ;
		m_pBackColorRight = new SGLImage ;
		m_pBackColorLeft->CreateImage
			( m_swapchainColor.m_width, m_swapchainColor.m_height,
				formatImageABGR, 32, SGLImageObject::bufferOnDeviceOnly ) ;
		m_pBackColorRight->CreateImage
			( m_swapchainColor.m_width, m_swapchainColor.m_height,
				formatImageABGR, 32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( (m_pBackDepthRight == nullptr)
		|| (m_pBackDepthRight->GetImageWidth() != (uint32_t) m_swapchainDepth.m_width)
		|| (m_pBackDepthRight->GetImageHeight() != (uint32_t) m_swapchainDepth.m_height) )
	{
		m_pBackDepthLeft = new SGLImage ;
		m_pBackDepthRight = new SGLImage ;
		m_pBackDepthLeft->CreateImage
			( m_swapchainDepth.m_width, m_swapchainDepth.m_height,
				formatImageDepth, 32, SGLImageObject::bufferOnDeviceOnly ) ;
		m_pBackDepthRight->CreateImage
			( m_swapchainDepth.m_width, m_swapchainDepth.m_height,
				formatImageDepth, 32, SGLImageObject::bufferOnDeviceOnly ) ;
	}
	const size_t	nViews = m_views.GetLength() ;
	if ( m_views.GetLength() == 1 )
	{
		renderBack.AttachTargetImage( m_pBackColorRight, m_pBackDepthRight ) ;
	}
	else
	{
		renderBack.AttachStereoTargetImage
			( m_pBackColorRight, m_pBackColorLeft,
				m_pBackDepthRight, m_pBackDepthLeft ) ;
	}
	return	true ;
}

// バックバッファをスワップチェーンへ転送する
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ViewContext::BltToSwapchainFromBackBuffer
	( S3DRenderParameterContext& render,
		S3DRenderParameterContext& renderBack )
{
	renderBack.Finish() ;
	renderBack.DetachTargetImage() ;

	SGLPaintParam	pp ;
	render.SelectParallaxView( S3DRenderContextInterface::stereoViewRight ) ;
	render.DrawImage( pp, m_pBackColorRight, nullptr ) ;
	//
	if ( m_views.GetLength() > 1 )
	{
		render.SelectParallaxView( S3DRenderContextInterface::stereoViewLeft ) ;
		render.DrawImage( pp, m_pBackColorLeft, nullptr ) ;
	}
}

// 現在のスワップチェーンを画面フレームバッファへ伸縮コピーする
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ViewContext::BltPrimaryFramebufferTo
	( S3DRenderContextInterface& renderPrimary,
		S3DRenderParameterContext& renderBack,
		int xDst0, int yDst0, int xDst1, int yDst1 )
{
	SGLImageObject *	pColor = m_pBackColorLeft ;
	SGLImageObject *	pDepth = m_pBackDepthLeft ;
	if ( (pColor == nullptr) || (pDepth == nullptr) )
	{
		return ;
	}
	const XrView&	xrView = m_views.At( 0 ) ;
	const float32_t	wFovTex = (float32_t) m_swapchainColor.m_width ;
	const float32_t	hFovTex = (float32_t) m_swapchainColor.m_height ;
	const float32_t	tanLeft = (float32_t) fabs(tan(xrView.fov.angleLeft)) ;
	const float32_t	tanRight = (float32_t) fabs(tan(xrView.fov.angleRight)) ;
	const float32_t	tanUp = (float32_t) fabs(tan(xrView.fov.angleUp)) ;
	const float32_t	tanDown = (float32_t) fabs(tan(xrView.fov.angleDown)) ;
	//
	SGLImageRect	rectSrc( 0, 0, m_swapchainColor.m_width,
									m_swapchainColor.m_height ) ;
	const float32_t	aspect = (hFovTex / wFovTex)
							/ ((tanUp + tanDown) / (tanLeft + tanRight)) ;
	float32_t		sx = 1.0f, sy = 1.0f ;
	if ( (tanUp + tanDown) / (tanLeft + tanRight)
			>= (double) (yDst1 - yDst0) / (xDst1 - xDst0) )
	{
		// 縦長の上下をカットする
		sx = (float32_t) (xDst1 - xDst0) / wFovTex ;
		sy = sx * aspect ;
		rectSrc.h = esl_roundfi( (float32_t) (yDst1 - yDst0) / sy ) ;
		rectSrc.y = (m_swapchainColor.m_height - rectSrc.h) / 2 ;
	}
	else
	{
		// 横長の左右をカットする
		sy = (float32_t) (yDst1 - yDst0) / hFovTex ;
		sx = sy / aspect ;
		rectSrc.w = esl_roundfi( (float32_t) (xDst1 - xDst0) / sx ) ;
		rectSrc.x = (m_swapchainColor.m_width - rectSrc.w) / 2 ;
	}
	//
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	pp.SetAffine( affine, xDst0, yDst0, 0, 0, sx, sy ) ;
	renderPrimary.DrawImage( pp, pColor, &rectSrc ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 全ビューコンテキスト
//////////////////////////////////////////////////////////////////////////////

SGLOpenXRProducer::ViewContexts::ViewContexts( void )
{
}

SGLOpenXRProducer::ViewContexts::~ViewContexts( void )
{
	ReleaseAllFrames() ;
}

void SGLOpenXRProducer::ViewContexts::Initialize( const SGLOpenXRProducer& oxr )
{
	SArray<XrViewConfigurationType>
		aViewCfgTypes = oxr.m_session.GetAllViewConfigurationTypes() ;

	for ( size_t i = 0; i < aViewCfgTypes.GetLength(); i ++ )
	{
		ViewContext *	pvc = new ViewContext ;
		if ( !pvc->CreateViewConfiguration
			( aViewCfgTypes.At(i), oxr.m_instance, oxr.m_system ) )
		{
			delete	pvc ;
			continue ;
		}
		pvc->m_xrSpace = oxr.m_spaceView.m_xrRefSpace ;
		pvc->InitializeFrame( oxr.m_session ) ;
		pvc->PrepareFrame( oxr ) ;
		//
		SObjectArray<ViewContext>::Add( pvc ) ;
	}
}

void SGLOpenXRProducer::ViewContexts::ReleaseAllFrames( void )
{
	for ( size_t i = 0; i < SObjectArray<ViewContext>::GetLength(); i ++ )
	{
		ViewContext *	pvc = SObjectArray<ViewContext>::GetAt( i ) ;
		ESLAssert( pvc != nullptr ) ;
		if ( pvc != nullptr )
		{
			pvc->ReleaseFrame() ;
		}
	}
}

void SGLOpenXRProducer::ViewContexts::SetResetAllFrames( void )
{
	for ( size_t i = 0; i < SObjectArray<ViewContext>::GetLength(); i ++ )
	{
		ViewContext *	pvc = SObjectArray<ViewContext>::GetAt( i ) ;
		ESLAssert( pvc != nullptr ) ;
		if ( pvc != nullptr )
		{
			pvc->SetFrameResetFlag() ;
		}
	}
}

SGLOpenXRProducer::ViewContext *
	SGLOpenXRProducer::ViewContexts::GetViewContextAs( XrViewConfigurationType type ) const
{
	for ( size_t i = 0; i < SObjectArray<ViewContext>::GetLength(); i ++ )
	{
		ViewContext *	pvc = SObjectArray<ViewContext>::GetAt( i ) ;
		ESLAssert( pvc != nullptr ) ;
		if ( (pvc != nullptr) && (pvc->m_xrvcType == type) )
		{
			return	pvc ;
		}
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// 表示レイヤー情報 CompositionLayers
//////////////////////////////////////////////////////////////////////////////

void SGLOpenXRProducer::CompositionLayers::AppendProjectionLayer
					( SGLOpenXRProducer::ViewContext& vc )
{
	XrCompositionLayerProjection *	pclp =
				AllocProjectionLayer( vc.m_cfgCurrent.m_flagsLayer ) ;
	ESLAssert( vc.m_xrSpace != XR_NULL_HANDLE ) ;
	pclp->space = vc.m_xrSpace ;
	pclp->viewCount = (uint32_t) vc.m_aProjectionViews.GetLength() ;
	pclp->views = vc.m_aProjectionViews.GetConstArray() ;
}

XrCompositionLayerQuad * SGLOpenXRProducer::CompositionLayers::AllocQuadLayer( void )
{
	XrCompositionLayerQuad *	pclq = new XrCompositionLayerQuad ;
	m_quads.Add( pclq ) ;
	//
	eslFillMemory( pclq, 0, sizeof(XrCompositionLayerQuad) ) ;
	pclq->type = XR_TYPE_COMPOSITION_LAYER_QUAD;
	//
	m_headers.Add( reinterpret_cast<const XrCompositionLayerBaseHeader*>(pclq) ) ;
	//
	return	pclq ;
}

XrCompositionLayerProjection *
	SGLOpenXRProducer::CompositionLayers::AllocProjectionLayer( XrCompositionLayerFlags flags )
{
	XrCompositionLayerProjection *	pclp = new XrCompositionLayerProjection ;
	m_projections.Add( pclp ) ;
	//
	eslFillMemory( pclp, 0, sizeof(XrCompositionLayerProjection) ) ;
	pclp->type = XR_TYPE_COMPOSITION_LAYER_PROJECTION ;
	pclp->layerFlags = flags ;
	//
	m_headers.Add( reinterpret_cast<const XrCompositionLayerBaseHeader*>(pclp) ) ;
	//
	return	pclp ;
}

size_t SGLOpenXRProducer::CompositionLayers::GetLayerCount( void ) const
{
	return	m_headers.GetLength() ;
}

const XrCompositionLayerBaseHeader *const*
		SGLOpenXRProducer::CompositionLayers::GetLayerPtrArray( void ) const
{
	return	m_headers.GetConstArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// コントローラーモデル ControllerModel
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLOpenXRProducer::ControllerModel, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::ControllerModel::ControllerModel
		( const SGLOpenXRProducer& oxr, XrPath pathController )
	: m_oxr( oxr ),
		m_pathController( pathController ),
		m_xrcmKey( XR_NULL_CONTROLLER_MODEL_KEY_MSFT ),
		m_pModel( nullptr ), m_pModelData( nullptr )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::ControllerModel::~ControllerModel( void )
{
	DeleteAllThreads() ;
}

// モデル状態のポーリング
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ControllerModel::PollingModel( void )
{
	if ( !m_oxr.GetXRExtension().supportsControllerModel )
	{
		return ;
	}
	CleanupDoneThreads() ;

	//
	// モデル切り替え判定
	//
	XrControllerModelKeyStateMSFT	xcmKeyState{ XR_TYPE_CONTROLLER_MODEL_KEY_STATE_MSFT } ;
	XrResult	xr = m_oxr.xrGetControllerModelKeyMSFT
		( m_oxr.GetXRSession().m_xrSession, m_pathController, &xcmKeyState ) ;
	if ( XRVerify( xr, "xrGetControllerModelKeyMSFT" )
		&& (xcmKeyState.modelKey != XR_NULL_CONTROLLER_MODEL_KEY_MSFT)
		&& (m_xrcmKey != xcmKeyState.modelKey) )
	{
		m_csMutex.Lock() ;
		m_queLoadKeys.RemoveAll() ;

		ModelData *	pModelData = m_models.GetAs( xcmKeyState.modelKey ) ;
		if ( pModelData != nullptr )
		{
			m_oxr.LockForDeviceState() ;
			m_xrcmKey = xcmKeyState.modelKey ;
			m_pModel = pModelData->m_pModel ;
			m_pModelData = pModelData ;
			m_oxr.UnlockForDeviceState() ;
		}
		else
		{
			m_queLoadKeys.Add( xcmKeyState.modelKey ) ;
			//
			if ( m_pRunningThread == nullptr )
			{
				m_pRunningThread = new SThread ;
				m_pRunningThread->BeginThread( this ) ;
			}
		}
		m_csMutex.Unlock() ;
	}
	//
	// モデル状態更新
	//
	UpdateModelState( m_pModelData ) ;
}

// モデル状態更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ControllerModel::UpdateModelState( ModelData * pModelData )
{
	if ( pModelData == nullptr )
	{
		return ;
	}
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( pModelData->m_pModel.Ptr() ) ;
	if ( pModel == nullptr )
	{
		return ;
	}
	//
	// 状態取得
	//
	XrControllerModelStateMSFT	xcmModelState{ XR_TYPE_CONTROLLER_MODEL_STATE_MSFT } ;
	xcmModelState.nodeCapacityInput = 0 ;
	
	XrResult	xr =
		m_oxr.xrGetControllerModelStateMSFT
			( m_oxr.GetXRSession().m_xrSession, pModelData->m_xrcmKey, &xcmModelState ) ;
	if ( !XRVerify( xr, "xrGetControllerModelStateMSFT" ) )
	{
		return ;
	}

	pModelData->m_aNodeStates.SetLength( xcmModelState.nodeCapacityInput ) ;
	for ( size_t i = 0; i < xcmModelState.nodeCapacityInput; i ++ )
	{
		pModelData->m_aNodeStates.At(i).type = XR_TYPE_CONTROLLER_MODEL_STATE_MSFT ;
	}
	xcmModelState.nodeCapacityInput =
			static_cast<uint32_t>( pModelData->m_aNodeStates.GetLength() ) ;
	xcmModelState.nodeStates = pModelData->m_aNodeStates.GetArray() ;

	xr = m_oxr.xrGetControllerModelStateMSFT
			( m_oxr.GetXRSession().m_xrSession, pModelData->m_xrcmKey, &xcmModelState ) ;
	if ( !XRVerify( xr, "xrGetControllerModelStateMSFT" ) )
	{
		return ;
	}
	ESLAssert( pModelData->m_aNodeStates.GetLength() != pModelData->m_pBones.GetLength() ) ;
	S4DMatrix		matCvtSpace( 1, -1, -1, 1 ) ;
	size_t			nOrderCount = pModelData->m_aBoneOrder.GetLength() ;
	const size_t *	pBoneOrder = pModelData->m_aBoneOrder.GetConstArray() ;
	size_t			nNodeCount = pModelData->m_aNodeStates.GetLength() ;
	const XrControllerModelNodeStateMSFT *
					pNodeStates = pModelData->m_aNodeStates.GetConstArray() ;
	Lock() ;
	for ( size_t i = 0; i < nOrderCount; i ++ )
	{
		size_t	iOrder = pBoneOrder[i] ;
		ESLAssert ( iOrder < nNodeCount ) ;
		if ( iOrder >= nNodeCount )
		{
			continue ;
		}
		S3DModelBoneSpace *	pBone = pModelData->m_pBones.GetAt( iOrder ) ;
		if ( pBone == nullptr )
		{
			continue ;
		}
		S4DMatrix	mat4State ;
		Matrix4FromXrPosef( mat4State, pNodeStates[iOrder].nodePose ) ;
		//
		pBone->ApplyModifiedOriginalBoneMatrix( mat4State, matCvtSpace ) ;
	}
	pModel->LockModelData() ;
	pModel->UpdateBoneMatrix() ;
	pModel->UnlockModelData() ;
	Unlock() ;
}

// 実行中スレッド削除
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ControllerModel::DeleteAllThreads( void )
{
	m_csMutex.Lock() ;
	if ( m_pRunningThread != nullptr )
	{
		SThread *	pThread = m_pRunningThread ;
		m_queLoadKeys.RemoveAll() ;
		m_csMutex.Unlock() ;
		pThread->Wait() ;
		m_csMutex.Lock() ;
	}
	for ( size_t i = 0; i < m_aDoneThreads.GetLength(); i ++ )
	{
		SThread *	pThread = m_aDoneThreads.GetAt( i ) ;
		if ( pThread != nullptr )
		{
			m_csMutex.Unlock() ;
			pThread->Wait() ;
			m_csMutex.Lock() ;
		}
	}
	m_aDoneThreads.RemoveAll() ;
	m_csMutex.Unlock() ;
}

// 終了スレッド後始末
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ControllerModel::CleanupDoneThreads( void )
{
	m_csMutex.Lock() ;
	for ( size_t i = 0; i < m_aDoneThreads.GetLength(); i ++ )
	{
		SThread *	pThread = m_aDoneThreads.GetAt( i ) ;
		if ( pThread != nullptr )
		{
			if ( pThread->Wait(0) == errSuccess )
			{
				m_aDoneThreads.RemoveAt( i -- ) ;
			}
		}
	}
	m_csMutex.Unlock() ;
}

// モデル読み込み実行
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::ControllerModel::LoadModel
	( SGLOpenXRProducer::ControllerModel::ModelData& model, XrControllerModelKeyMSFT key )
{
	if ( (m_oxr.xrGetControllerModelKeyMSFT == nullptr)
		|| (m_oxr.xrLoadControllerModelMSFT == nullptr)
		|| (m_oxr.xrGetControllerModelPropertiesMSFT == nullptr)
		|| (m_oxr.xrGetControllerModelStateMSFT == nullptr) )
	{
		return	false ;
	}
	//
	// glTF バイナリデータ取得
	//
	SByteBuffer	bufGLTFData ;
	uint32_t	nBufSize = 0 ;
	XrResult	xr =
		m_oxr.xrLoadControllerModelMSFT
			( m_oxr.GetXRSession().m_xrSession, key, 0, &nBufSize, nullptr ) ;
	if ( !XRVerify( xr, "xrLoadControllerModelMSFT" ) )
	{
		return	false ;
	}
	xr = m_oxr.xrLoadControllerModelMSFT
			( m_oxr.GetXRSession().m_xrSession, key, nBufSize,
				&nBufSize, bufGLTFData.GetArray(nBufSize) ) ;
	bufGLTFData.FinishArray() ;
	if ( !XRVerify( xr, "xrLoadControllerModelMSFT" ) )
	{
		return	false ;
	}
	//
	// glTF 読み込み
	//
	SSmartPointer<S3DModelLoaderInterface>	pModelLoader =
			S3DModelLoaderInterface::NewModelLoaderTypeAs
				( S3DModelLoaderInterface::glTransmissionFormat ) ;
	if ( pModelLoader == nullptr )
	{
		return	false ;
	}
	SSmartPointer<S3DModelBuffer>	pModel = new S3DModelBuffer ;
	bufGLTFData.Seek( 0 ) ;
	if ( pModelLoader->ReadModel( *pModel, bufGLTFData ) )
	{
		return	false ;
	}
	S3DModelBuffer *	pModelBuf = pModel.Detach() ;
	model.m_pModel = pModelBuf ;
	//
	// プロパティ取得
	//
	XrControllerModelPropertiesMSFT	xcmProps{ XR_TYPE_CONTROLLER_MODEL_PROPERTIES_MSFT } ;
	xcmProps.nodeCapacityInput = 0 ;

	xr = m_oxr.xrGetControllerModelPropertiesMSFT
				( m_oxr.GetXRSession().m_xrSession, key, &xcmProps ) ;
	if ( !XRVerify( xr, "xrGetControllerModelPropertiesMSFT" ) )
	{
		return	false ;
	}

	model.m_aNodeProps.SetLength( xcmProps.nodeCapacityInput ) ;
	for ( size_t i = 0; i < xcmProps.nodeCapacityInput; i ++ )
	{
		model.m_aNodeProps.At(i).type = XR_TYPE_CONTROLLER_MODEL_NODE_PROPERTIES_MSFT ;
	}

	xcmProps.nodeProperties = model.m_aNodeProps.GetArray() ;
	xcmProps.nodeCapacityInput = static_cast<uint32_t>( model.m_aNodeProps.GetLength() ) ;

	xr = m_oxr.xrGetControllerModelPropertiesMSFT
				( m_oxr.GetXRSession().m_xrSession, key, &xcmProps ) ;
	model.m_aNodeProps.FinishArray() ;
	if ( !XRVerify( xr, "xrGetControllerModelPropertiesMSFT" ) )
	{
		model.m_aNodeProps.RemoveAll() ;
		return	false ;
	}
	//
	// ボーン列挙
	//
	size_t	nNodeCount = model.m_aNodeProps.GetLength() ;
	model.m_pBones.SetLength( nNodeCount ) ;
	for ( size_t i = 0; i < nNodeCount; i ++ )
	{
		model.m_pBones.SetAt
			( i, pModelBuf->GetBonePropertyAs
					( SString(model.m_aNodeProps.At(i).nodeName) ) ) ;
	}
	model.m_aBoneOrder.RemoveAll() ;
	model.m_aBoneOrder.SetLimit( nNodeCount ) ;
	pModelBuf->GetBoneRoot().AddBoneOrderIndex
		( model.m_aBoneOrder,
			model.m_pBones.GetConstArray(), model.m_pBones.GetLength() ) ;
	return	true ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::ControllerModel::Run( void )
{
	m_csMutex.Lock() ;
	for ( ; ; )
	{
		if ( m_queLoadKeys.GetLength() == 0 )
		{
			ESLAssert( m_pRunningThread.Ptr() == SThread::GetCurrentThread() ) ;
			if ( m_pRunningThread != nullptr )
			{
				m_aDoneThreads.Add( m_pRunningThread.Detach() ) ;
			}
			break ;
		}
		//
		// キューの先頭を取得
		//
		XrControllerModelKeyMSFT	key = m_queLoadKeys.At(0) ;
		m_queLoadKeys.RemoveAt(0) ;
		//
		ModelData *	pModelData = m_models.GetAs( key ) ;
		if ( pModelData != nullptr )
		{
			m_oxr.LockForDeviceState() ;
			m_xrcmKey = key ;
			m_pModel = pModelData->m_pModel ;
			m_pModelData = pModelData ;
			m_oxr.UnlockForDeviceState() ;
			continue ;
		}
		//
		// 新たなモデルをロードする
		//
		m_csMutex.Unlock() ;
		//
		pModelData = new ModelData ;
		LoadModel( *pModelData, key ) ;
		//
		m_csMutex.Lock() ;
		if ( m_models.GetAs( key ) == nullptr )
		{
			m_models.SetAs( key, pModelData ) ;
			//
			m_oxr.LockForDeviceState() ;
			m_xrcmKey = key ;
			m_pModel = pModelData->m_pModel ;
			m_pModelData = pModelData ;
			m_oxr.UnlockForDeviceState() ;
		}
		else
		{
			delete	pModelData ;
		}
	}
	m_csMutex.Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// SGLOpenXRProducer
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLOpenXRProducer,
		SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::SGLOpenXRProducer( S3DRenderDevice * pDevice )
	: xrGetOpenGLGraphicsRequirementsKHR( nullptr ),
		m_pOpenGL( ESLTypeCast<SGLOpenGLContext>( pDevice ) ),
		m_flagVisibleView( false ),
		m_viewSpaceType( spaceEyeLevel ),
		m_vTrackingBasePos( 0, 0, 0 ),
		m_flagBoundActionSets( false ),
		m_stateSession( XR_SESSION_STATE_UNKNOWN ),
		m_flagBegunSession( false ),
		m_pPrimaryViewLayers( nullptr ),
		m_flagBegunThread( false ),
		m_flagAbortedPolling( false ),
		m_flagQuitThread( false ),
		m_flagBegunDrawView( false )
{
	if ( m_pOpenGL != nullptr )
	{
		m_pRenderer = new S3DOpenGLBufferedRenderer( m_pOpenGL ) ;
		m_pBackRenderer = new S3DOpenGLBufferedRenderer( m_pOpenGL ) ;
	}
	for ( size_t i = 0; i < bhcpiComponentCount; i ++ )
	{
		m_pHandActions[i] = nullptr ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::~SGLOpenXRProducer( void )
{
	Release() ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::Initialize
	( const Version& verApp,
		ViewSpaceType spaceType,
		const SGLOpenXRProducer::InitConfig * pConfig )
{
	if ( m_pOpenGL == nullptr )
	{
		return	sglErrNotSupported ;
	}
	//
	// 拡張機能確認
	//
	m_supported.EnumExtentions() ;
	m_supported.TraceAllExtentions() ;

#ifdef XR_USE_GRAPHICS_API_OPENGL
	if ( !m_supported.supportsOpenGL )
	{
		return	sglErrNotSupported ;
	}
#else
	return	sglErrNotSupported ;
#endif

	SPointerArray<const char>	aReqExts ;
	for ( int i = 0; s_pszRequestExtensions[i] != nullptr; i ++ )
	{
		aReqExts.Add( s_pszRequestExtensions[i] ) ;
	}
	for ( int i = 0; i < ipbhControllerTypeCount; i ++ )
	{
		if ( s_InteractionProfilePath[i].pszNeedsExtension != nullptr )
		{
			aReqExts.Add( s_InteractionProfilePath[i].pszNeedsExtension ) ;
		}
	}
	if ( pConfig != nullptr )
	{
		for ( size_t i = 0; i < pConfig->nReqExtentionsCount; i ++ )
		{
			aReqExts.Add( pConfig->ppszReqExtentions[i] ) ;
		}
		for ( size_t i = 0; i < pConfig->nExProfileCount; i ++ )
		{
			if ( pConfig->pExHandPathsDesc[i].profilePath.pszNeedsExtension != nullptr )
			{
				aReqExts.Add( pConfig->pExHandPathsDesc[i].profilePath.pszNeedsExtension ) ;
			}
		}
	}
	aReqExts.Add( nullptr ) ;

	m_extensions = m_supported ;
	m_extensions.ChooseSupported( aReqExts.GetConstArray() ) ;

	//
	// インスタンス作成
	//
	if ( !m_instance.CreateInstance( verApp, m_extensions ) )
	{
		return	sglErrFailed ;
	}

	//
	// 関数アドレス
	//
	CommitXRFunctionsAddress() ;
	//
	if ( xrGetOpenGLGraphicsRequirementsKHR == nullptr )
	{
		return	sglErrNotSupported ;
	}

	//
	// システム・コンテキスト
	//
	if ( !m_system.CreateSystemContext( m_instance, m_extensions ) )
	{
		return	sglErrFailed ;
	}
	if ( m_system.m_supportedPrimaryViewConfig.Find
				( XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ) < 0 )
	{
		return	sglErrNotSupported ;
	}

	//
	// セッション
	//
	if ( !m_session.CreateSession( *this, m_system, m_instance, m_extensions ) )
	{
		return	sglErrFailed ;
	}

	//
	// スペース作成
	//
	if ( !m_spaceView.CreateSpace( m_session, XR_REFERENCE_SPACE_TYPE_VIEW ) )
	{
		return	sglErrFailed ;
	}
	XrReferenceSpaceType	xrAppSpace = XR_REFERENCE_SPACE_TYPE_LOCAL ;
	if ( spaceType != spaceEyeLevel )
	{
		xrAppSpace = m_extensions.supportsUnboundedSpace
							? XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT
							: XR_REFERENCE_SPACE_TYPE_STAGE ;
	}
	if ( !m_spaceApp.CreateSpace( m_session, xrAppSpace ) )
	{
		return	sglErrFailed ;
	}
	m_viewSpaceType = spaceType ;

	//
	// ビュー・コンテキスト
	//
	m_viewContexts.Initialize( *this ) ;
	if ( m_viewContexts.GetLength() == 0 )
	{
		return	sglErrFailed ;
	}

	//
	// アクション設定・バインド
	//
	RegisterAllInteractions( pConfig ) ;
	BindActionSets() ;

	//
	// コントローラーモデル
	//
	ESLAssert( !m_extensions.supportsControllerModel ) ;	// 来てたら気付くように
/* 動作テストできていないので
	if ( m_extensions.supportsControllerModel )
	{
		for ( int i = 0; i < handCount; i ++ )
		{
			m_aControllerModels.SetAt
				( i, new ControllerModel( *this, m_instance.m_xrpHandPath[i] ) ) ;
		}
	}
*/

	//
	// ポーリングスレッド開始
	//
	if ( BeginPollingThread() )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::Release( void )
{
	EndPollingThread() ;

	m_aControllerModels.RemoveAll() ;

	m_viewContexts.ReleaseAllFrames() ;

	for ( size_t iHand = 0; iHand < handCount; iHand ++ )
	{
		for ( size_t iComp = 0; iComp < bhcpiPoseComponentCount; iComp ++ )
		{
			m_spaceHandPose[iHand][iComp].DestroySpace() ;
		}
	}
	m_spaceView.DestroySpace() ;
	m_spaceApp.DestroySpace() ;

	for ( size_t i = 0; i < bhcpiComponentCount; i ++ )
	{
		m_pHandActions[i] = nullptr ;
	}
	m_aActionContexts.RemoveAll() ;

	m_session.DestroySession() ;
	m_instance.DestroyInstance() ;
}

// 拡張関数のアドレス取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::CommitXRFunctionsAddress( void )
{
#ifdef XR_USE_GRAPHICS_API_OPENGL
	xrGetOpenGLGraphicsRequirementsKHR =
		reinterpret_cast<PFN_xrGetOpenGLGraphicsRequirementsKHR>
			( m_instance.GetInstanceProcAddr( "xrGetOpenGLGraphicsRequirementsKHR" ) ) ;
#endif
	//
	xrGetControllerModelKeyMSFT =
		reinterpret_cast<PFN_xrGetControllerModelKeyMSFT>
			( m_instance.GetInstanceProcAddr( "xrGetControllerModelKeyMSFT" ) ) ;
	xrLoadControllerModelMSFT =
		reinterpret_cast<PFN_xrLoadControllerModelMSFT>
			( m_instance.GetInstanceProcAddr( "xrLoadControllerModelMSFT" ) ) ;
	xrGetControllerModelPropertiesMSFT =
		reinterpret_cast<PFN_xrGetControllerModelPropertiesMSFT>
			( m_instance.GetInstanceProcAddr( "xrGetControllerModelPropertiesMSFT" ) ) ;
	xrGetControllerModelStateMSFT =
		reinterpret_cast<PFN_xrGetControllerModelStateMSFT>
			( m_instance.GetInstanceProcAddr( "xrGetControllerModelStateMSFT" ) ) ;
}

// 拡張情報
//////////////////////////////////////////////////////////////////////////////
const SGLOpenXRProducer::ExtensionContext&
	SGLOpenXRProducer::GetXRExtension( void ) const
{
	return	m_extensions ;
}

// インスタンス
//////////////////////////////////////////////////////////////////////////////
const SGLOpenXRProducer::InstanceContext& SGLOpenXRProducer::GetXRInstance( void ) const
{
	return	m_instance ;
}

// システム
//////////////////////////////////////////////////////////////////////////////
const SGLOpenXRProducer::SystemContext& SGLOpenXRProducer::GetXRSystem( void ) const
{
	return	m_system ;
}

// セッション
//////////////////////////////////////////////////////////////////////////////
const SGLOpenXRProducer::SessionContext& SGLOpenXRProducer::GetXRSession( void ) const
{
	return	m_session ;
}

// ViewContext 総数
//////////////////////////////////////////////////////////////////////////////
size_t SGLOpenXRProducer::GetViewContextCount( void ) const
{
	return	m_viewContexts.GetLength() ;
}

// ViewContext 取得（iView=0 がプライマリ）
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::ViewContext * SGLOpenXRProducer::GetViewContextAt( size_t iView ) const
{
	return	m_viewContexts.GetAt( iView ) ;
}

// XrViewConfigurationType に対応する ViewContext 取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::ViewContext *
	SGLOpenXRProducer::GetViewContextAs( XrViewConfigurationType type ) const
{
	return	m_viewContexts.GetViewContextAs( type ) ;
}

// イベント・ポーリング （返り値 sglErrAbort は終了要請）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::PollEvents( void )
{
	XrEventDataBuffer xrEventData ;
	for ( ; ; )
	{
		xrEventData.type = XR_TYPE_EVENT_DATA_BUFFER ;
		xrEventData.next = nullptr ;

		XrResult	xr = xrPollEvent( m_instance.m_xrInstance, &xrEventData ) ;
		if ( xr == XR_EVENT_UNAVAILABLE )
		{
			return	sglErrSuccess ;
		}
		if ( !XRVerify( xr, "xrPollEvent" ) )
		{
			return	sglErrFailed ;
		}
		if ( xrEventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED )
		{
			XrEventDataSessionStateChanged *	pxrSessionState =
				reinterpret_cast<XrEventDataSessionStateChanged*>( &xrEventData ) ;
			if ( pxrSessionState->session == m_session.m_xrSession )
			{
				m_csState.Lock() ;
				m_stateSession = pxrSessionState->state ;
				m_csState.Unlock() ;

				switch ( pxrSessionState->state )
				{
				case XR_SESSION_STATE_EXITING:
					ESLTrace( "OpenXR: XR_SESSION_STATE_EXITING\n" ) ;
					return sglErrAbort ;	// User's intended to quit

				case XR_SESSION_STATE_LOSS_PENDING:
					ESLTrace( "OpenXR: XR_SESSION_STATE_LOSS_PENDING\n" ) ;
					return sglErrAbort;		// Runtime's intend to quit

				case XR_SESSION_STATE_READY:
					BeginSession();
					break;

				case XR_SESSION_STATE_STOPPING:
					EndSession();
					break;
				}
			}
		}
	}
}

// セッション開始
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::BeginSession( void )
{
	XrSessionBeginInfo	xsbi{ XR_TYPE_SESSION_BEGIN_INFO } ;
	xsbi.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ;

	XrSecondaryViewConfigurationSessionBeginInfoMSFT
		xsvcsbi{ XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT } ;
	if ( m_extensions.supportsSecondaryViewConfiguration
		&& (m_session.m_aSecondaryViewConfigs.GetLength() > 0) )
	{
		xsvcsbi.viewConfigurationCount =
			(uint32_t) m_session.m_aSecondaryViewConfigs.GetLength() ;
		xsvcsbi.enabledViewConfigurationTypes =
			m_session.m_aSecondaryViewConfigs.GetConstArray() ;
		//
		xsvcsbi.next = xsbi.next ;
		xsbi.next = &xsvcsbi ;
	}

	XrResult	xr = xrBeginSession( m_session.m_xrSession, &xsbi ) ;
	if ( !XRVerify( xr, "xrBeginSession" ) )
	{
		return ;
	}
	ESLTrace( "OpenXR: BeginSession\n" ) ;
	m_flagBegunSession = true ;
}

// セッション終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::EndSession( void )
{
	ESLAssert( m_flagBegunSession ) ;
	ESLTrace( "OpenXR: EndSession\n" ) ;
	m_flagBegunSession = false ;

	XrResult	xr = xrEndSession( m_session.m_xrSession ) ;
	XRVerify( xr, "xrEndSession" ) ;
}

// セッション実行中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::IsSessionRunning( void ) const
{
	return	m_flagBegunSession ;
}

// アクションコンテキスト追加
//////////////////////////////////////////////////////////////////////////////
SGLOpenXRProducer::ActionContext * SGLOpenXRProducer::NewActionContext( void )
{
	ActionContext *	pac = new ActionContext( m_instance ) ;
	m_aActionContexts.Add( pac ) ;
	return	pac ;
}

// セッションにアクション関連付け（一度だけ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::BindActionSets( void )
{
	ESLAssert( !m_flagBoundActionSets ) ;

	// アクションバインディング収集
	SSortObjectArray
		< SSortObjectElement
			<XrPath,ActionBindingArray> >	mapActionBinding ;
	for ( size_t i = 0; i < m_aActionContexts.GetLength(); i ++ )
	{
		ActionContext *	pActContext = m_aActionContexts.GetAt( i ) ;
		ESLAssert( pActContext != nullptr ) ;
		if ( pActContext == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pActContext->m_mapActionBinding.GetLength(); j ++ )
		{
			const XrPath *	pTag = pActContext->m_mapActionBinding.GetTagAt( j ) ;
			ESLAssert( pTag != nullptr ) ;
			ActionBindingArray *	pabaSrc = pActContext->m_mapActionBinding.GetAt( j ) ;
			ESLAssert( pabaSrc != nullptr ) ;
			if ( (pTag == nullptr) || (pabaSrc == nullptr) )
			{
				continue ;
			}
			ActionBindingArray *	pabaDst = mapActionBinding.GetAs( *pTag ) ;
			if ( pabaDst == nullptr )
			{
				pabaDst = new ActionBindingArray ;
				mapActionBinding.SetAs( *pTag, pabaDst ) ;
			}
			pabaDst->AddArray( pabaSrc->GetConstArray(), pabaSrc->GetLength() ) ;
		}
	}

	// 順次 xrSuggestInteractionProfileBindings
	for ( size_t i = 0; i < mapActionBinding.GetLength(); i ++ )
	{
		const XrPath *	pTag = mapActionBinding.GetTagAt( i ) ;
		ESLAssert( pTag != nullptr ) ;
		ActionBindingArray *	paba = mapActionBinding.GetAt( i ) ;
		ESLAssert( paba != nullptr ) ;
		if ( (pTag == nullptr) || (paba == nullptr) )
		{
			continue ;
		}
		#ifdef	__DEBUG__
		ESLTrace( "%s:\n", m_instance.PathToString(*pTag).GetConstArray() ) ;
		for ( size_t j = 0; j < paba->GetLength(); j ++ )
		{
			ESLTrace( "  %s\n", m_instance.PathToString(paba->At(j).binding).GetConstArray() ) ;
		}
		#endif
		//
		XrInteractionProfileSuggestedBinding	xipsb{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING } ;
		xipsb.interactionProfile = *pTag ;
		xipsb.suggestedBindings = paba->GetConstArray() ;
		xipsb.countSuggestedBindings = static_cast<uint32_t>( paba->GetLength() ) ;
		//
		XrResult	xr =
			xrSuggestInteractionProfileBindings( m_instance.m_xrInstance, &xipsb ) ;
		XRVerify( xr, "xrSuggestInteractionProfileBindings" ) ;
	}

	// xrAttachSessionActionSets
	SArray<XrActionSet>	aActionSets ;
	for ( size_t i = 0; i < m_aActionContexts.GetLength(); i ++ )
	{
		ActionContext *	pActContext = m_aActionContexts.GetAt( i ) ;
		ESLAssert( pActContext != nullptr ) ;
		if ( pActContext == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pActContext->m_aActionSets.GetLength(); j ++ )
		{
			ActionSet *	pActSet = pActContext->m_aActionSets.GetAt( j ) ;
			ESLAssert( pActSet != nullptr ) ;
			if ( pActSet == nullptr )
			{
				continue ;
			}
			aActionSets.Add( pActSet->m_xrActionSet ) ;
		}
	}
	if ( aActionSets.GetLength() > 0 )
	{
		XrSessionActionSetsAttachInfo	xsasai{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO } ;
		xsasai.countActionSets = static_cast<uint32_t>( aActionSets.GetLength() ) ;
		xsasai.actionSets = aActionSets.GetConstArray() ;
		//
		XrResult	xr =
			xrAttachSessionActionSets( m_session.m_xrSession, &xsasai ) ;
		if ( !XRVerify( xr, "xrAttachSessionActionSets" ) )
		{
			return	sglErrFailed ;
		}
	}
	m_flagBoundActionSets = true ;
	return	sglErrSuccess ;
}

// アクション関連付け済みか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::HasBoundActionSets( void ) const
{
	return	m_flagBoundActionSets ;
}

// コントローラーモデル更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::PollingControllerModels( void )
{
	for ( size_t i = 0; i < m_aControllerModels.GetLength(); i ++ )
	{
		ControllerModel *	pCtrlModel = m_aControllerModels.GetAt(i) ;
		ESLAssert( pCtrlModel != nullptr ) ;
		if ( pCtrlModel != nullptr )
		{
			pCtrlModel->PollingModel() ;
		}
	}
}

// フレーム同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::WaitFrame( void )
{
	XrFrameState	xfs{ XR_TYPE_FRAME_STATE } ;

	XrSecondaryViewConfigurationFrameStateMSFT
					xsvcfs{ XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT } ;

	const size_t	nSecondaryViewCfgs = m_session.m_aSecondaryViewConfigs.GetLength() ;
	SArray<XrSecondaryViewConfigurationStateMSFT>
					aSecondaryViewCfgStates ;

	aSecondaryViewCfgStates.SetLength( nSecondaryViewCfgs ) ;
	for ( size_t i = 0; i < nSecondaryViewCfgs; i ++ )
	{
		aSecondaryViewCfgStates.At(i).type =
				XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT ;
	}

	if ( m_extensions.supportsSecondaryViewConfiguration && (nSecondaryViewCfgs > 0) )
	{
		xsvcfs.viewConfigurationCount = (uint32_t) nSecondaryViewCfgs ;
		xsvcfs.viewConfigurationStates = aSecondaryViewCfgStates.GetArray() ;
		//
		xsvcfs.next = xfs.next ;
		xfs.next = &xsvcfs ;
	}

	XrFrameWaitInfo	xfwi{ XR_TYPE_FRAME_WAIT_INFO } ;
	XrResult	xr = xrWaitFrame( m_session.m_xrSession, &xfwi, &xfs ) ;
	if ( !XRVerify( xr, "xrWaitFrame" ) )
	{
		return	sglErrFailed ;
	}

	m_csState.Lock() ;
	m_xfsFrameState = xfs ;
	m_xfsFrameState.next = nullptr ;
	//
	if ( m_extensions.supportsSecondaryViewConfiguration )
	{
		m_aSecondaryViewCfgStates = aSecondaryViewCfgStates ;
	}
	m_flagVisibleView = ShouldRenderCurrentFrame() ;
	m_csState.Unlock() ;
	return	sglErrSuccess ;
}

// 現在のフレーム時間
//////////////////////////////////////////////////////////////////////////////
const XrTime& SGLOpenXRProducer::CurrentFrameTime( void ) const
{
	return	m_xfsFrameState.predictedDisplayTime ;
}

// アクション同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::SyncActions( void )
{
	//
	// アクティブなアクションセット収集
	//
	SArray<XrActiveActionSet>	aActionSets;
	for ( size_t i = 0; i < m_aActionContexts.GetLength(); i ++ )
	{
		ActionContext *	pActContext = m_aActionContexts.GetAt( i ) ;
		ESLAssert( pActContext != nullptr ) ;
		if ( pActContext == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pActContext->m_aActionSets.GetLength(); j ++ )
		{
			ActionSet *	pActSet = pActContext->m_aActionSets.GetAt( j ) ;
			ESLAssert( pActSet != nullptr ) ;
			if ( (pActSet == nullptr)
				|| !pActSet->IsActived() )
			{
				continue ;
			}
			if ( pActSet->m_setDeclPaths.GetLength() == 0 )
			{
				XrActiveActionSet	xas ;
				xas.actionSet = pActSet->m_xrActionSet ;
				xas.subactionPath = XR_NULL_PATH ;
				aActionSets.Add( xas ) ;
			}
			else
			{
				for ( size_t k = 0; k < pActSet->m_setDeclPaths.GetLength(); k ++ )
				{
					XrActiveActionSet	xas ;
					xas.actionSet = pActSet->m_xrActionSet ;
					xas.subactionPath = pActSet->m_setDeclPaths.At(k) ;
					aActionSets.Add( xas ) ;
				}
			}
		}
	}
	//
	// アクション同期実行
	//
	if ( aActionSets.GetLength() > 0 )
	{
		XrActionsSyncInfo	xasi{ XR_TYPE_ACTIONS_SYNC_INFO } ;
		xasi.countActiveActionSets = static_cast<uint32_t>( aActionSets.GetLength() ) ;
		xasi.activeActionSets = aActionSets.GetConstArray() ;
		//
		XrResult	xr = xrSyncActions( m_session.m_xrSession, &xasi ) ;
		if ( !XRVerify( xr, "xrSyncActions" ) )
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// 現在フレームのビューの状態更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::UpdateAllViewsLocation( void )
{
	for ( size_t i = 0; i < m_viewContexts.GetLength(); i ++ )
	{
		ViewContext *	pvc = m_viewContexts.GetAt(i) ;
		ESLAssert( pvc != nullptr ) ;
		if ( pvc == nullptr )
		{
			continue ;
		}
		pvc->LocateViews
			( m_session, m_spaceView, m_xfsFrameState.predictedDisplayTime ) ;
		pvc->LocateSpace
			( m_spaceView, m_spaceApp, m_xfsFrameState.predictedDisplayTime ) ;
		//
		if ( i == 0 )
		{
			LockForDeviceState() ;
			pvc->GetHeadPosture( *this, m_postureHead ) ;
			m_postureHead.vPosition += m_vTrackingBasePos ;
			//
			pvc->GetEyeFieldOfView( *this, m_fovEyes[eyeLeft], 0 ) ;
			pvc->GetEyePosture( *this, m_postureEyes[eyeLeft], 0 ) ;
			//
			pvc->GetEyeFieldOfView( *this, m_fovEyes[eyeRight], 1 ) ;
			pvc->GetEyePosture( *this, m_postureEyes[eyeRight], 1 ) ;
			UnlockForDeviceState() ;
		}
	}
}

// フレーム描画開始処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::BeginFrame( void )
{
	XrFrameBeginInfo	xfbi{ XR_TYPE_FRAME_BEGIN_INFO } ;
	XrResult	xr = xrBeginFrame( m_session.m_xrSession, &xfbi ) ;
	if ( !XRVerify( xr, "xrBeginFrame" ) )
	{
		return	sglErrFailed ;
	}

	SSmartLock<SCriticalSection>	lock( &m_csState ) ;
	if ( m_extensions.supportsSecondaryViewConfiguration )
	{
		// セカンダリビューのアクティブ状態更新判定
		m_aActiveSecondaryView.RemoveAll() ;
		m_aSecondaryViewCfgFrameEnd.RemoveAll() ;
		//
		for ( size_t i = 0; i < m_aSecondaryViewCfgStates.GetLength(); i ++ )
		{
			const XrSecondaryViewConfigurationStateMSFT &
							xsvcState = m_aSecondaryViewCfgStates.At(i) ;
			ViewContext *	pvc = GetViewContextAs( xsvcState.viewConfigurationType ) ;
			if ( pvc == nullptr )
			{
				continue ;
			}
			//
			// アクティブ状態更新
			//
			pvc->UpdateStateActive( xsvcState.active, m_instance, m_system ) ;
			//
			const ViewProperties *	pvp = m_system.GetViewPropertiesAs( pvc->m_xrvcType ) ;
			if ( pvp == nullptr )
			{
				continue ;
			}
			//
			// アクティブなセカンダリビューのエントリを準備
			//
			size_t	iActive = m_aActiveSecondaryView.GetLength() ;
			ESLAssert( m_aSecondaryViewCfgFrameEnd.GetLength() == iActive ) ;
			m_aActiveSecondaryView.Add( pvc ) ;
			//
			XrSecondaryViewConfigurationLayerInfoMSFT
					xsvcli{ XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT } ;
			xsvcli.next = nullptr ;
			xsvcli.viewConfigurationType = pvc->m_xrvcType  ;
			xsvcli.environmentBlendMode = pvp->m_xrBlendMode ;
			xsvcli.layerCount = 0 ;
			xsvcli.layers = nullptr ;
			m_aSecondaryViewCfgFrameEnd.Add( xsvcli ) ;
		}
	}
	else
	{
		m_aActiveSecondaryView.RemoveAll() ;
		m_aSecondaryViewCfgFrameEnd.RemoveAll() ;
	}
	m_aTempCompositionLayers.RemoveAll() ;
	m_pPrimaryViewLayers = nullptr ;

	//
	// 描画準備処理
	//
	for ( size_t i = 0; i < m_viewContexts.GetLength(); i ++ )
	{
		ViewContext *	pvc = m_viewContexts.GetAt(i) ;
		ESLAssert( pvc != nullptr ) ;
		if ( pvc == nullptr )
		{
			continue ;
		}
		if ( (i == 0) || pvc->m_active )
		{
			pvc->PrepareFrame( *this ) ;
		}
	}
	return	sglErrSuccess ;
}

// フレーム描画完了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::EndFrame( void )
{
	XrFrameEndInfo	xfei{ XR_TYPE_FRAME_END_INFO } ;
	xfei.environmentBlendMode = m_session.m_xrBlendMode ;
	xfei.displayTime = m_xfsFrameState.predictedDisplayTime ;

	if ( m_pPrimaryViewLayers != nullptr )
	{
		xfei.layerCount = (uint32_t) m_pPrimaryViewLayers->GetLayerCount() ;
		xfei.layers = m_pPrimaryViewLayers->GetLayerPtrArray() ;
	}

	XrSecondaryViewConfigurationFrameEndInfoMSFT
			xsvcfei{ XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT } ;
	if ( m_extensions.supportsSecondaryViewConfiguration
		&& (m_aSecondaryViewCfgFrameEnd.GetLength() > 0) )
	{
		xsvcfei.viewConfigurationCount = (uint32_t) m_aSecondaryViewCfgFrameEnd.GetLength() ;
		xsvcfei.viewConfigurationLayersInfo = m_aSecondaryViewCfgFrameEnd.GetConstArray() ;
		//
		xsvcfei.next = xfei.next ;
		xfei.next = &xsvcfei ;
	}

	XrResult	xr = xrEndFrame( m_session.m_xrSession, &xfei ) ;
	if ( !XRVerify( xr, "xrEndFrame" ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 現フレームを表示すべきか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::ShouldRenderCurrentFrame( void ) const
{
	return	(m_xfsFrameState.shouldRender != 0) ;
}

// 描画ビューのサブミット登録
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::AppendSubmitView( ViewContext * pvc )
{
	SSmartLock<SCriticalSection>	lock( &m_csState ) ;
	if ( m_viewContexts.GetAt(0) == pvc )
	{
		ESLAssert( m_pPrimaryViewLayers == nullptr ) ;
		m_pPrimaryViewLayers = new CompositionLayers ;
		m_aTempCompositionLayers.Add( m_pPrimaryViewLayers ) ;
		m_pPrimaryViewLayers->AppendProjectionLayer( *pvc ) ;
		return	sglErrSuccess ;
	}
	ssize_t	iActiveSecondaryView = m_aActiveSecondaryView.FindPtr( pvc ) ;
	if ( iActiveSecondaryView >= 0 )
	{
		ESLAssert( (size_t) iActiveSecondaryView < m_aSecondaryViewCfgFrameEnd.GetLength() ) ;
		XrSecondaryViewConfigurationLayerInfoMSFT&
			xsvcli = m_aSecondaryViewCfgFrameEnd.At(iActiveSecondaryView) ;
		//
		CompositionLayers *	pCompLayers = new CompositionLayers ;
		m_aTempCompositionLayers.Add( pCompLayers ) ;
		//
		pCompLayers->AppendProjectionLayer( *pvc ) ;
		xsvcli.layerCount = (uint32_t) pCompLayers->GetLayerCount() ;
		xsvcli.layers = pCompLayers->GetLayerPtrArray() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// XrPosef から Posture へ変換（スケール変換あり）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::GetPostureFromXrPosef
	( SGLVRViewProducer::Posture& postureDst, const XrPosef& poseSrc ) const
{
	S3DQuaternion	qPose ;
	S3DMatrix		matPose ;
	qPose.q[0] =   poseSrc.orientation.w ;
	qPose.q[1] =   poseSrc.orientation.x ;
	qPose.q[2] = - poseSrc.orientation.y ;
	qPose.q[3] = - poseSrc.orientation.z ;
	qPose.ToMatrix( matPose ) ;
	//
	const float32_t	fpScale = (float32_t) GetScaleHMDToModel() ;
	//
	postureDst.matOrientation = matPose ;
	postureDst.vPosition.x =   poseSrc.position.x * fpScale ;
	postureDst.vPosition.y = - poseSrc.position.y * fpScale ;
	postureDst.vPosition.z = - poseSrc.position.z * fpScale ;
}

// XrPosef から Posture へ変換（空間変換無し）
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::Matrix4FromXrPosef
	( S4DMatrix& mat4Dst, const XrPosef& poseSrc )
{
	S3DQuaternion	qPose ;
	S3DMatrix		matPose ;
	qPose.q[0] = poseSrc.orientation.w ;
	qPose.q[1] = poseSrc.orientation.x ;
	qPose.q[2] = poseSrc.orientation.y ;
	qPose.q[3] = poseSrc.orientation.z ;
	qPose.ToMatrix( matPose ) ;
	//
	S3DVector	vPos( poseSrc.position.x, poseSrc.position.y, poseSrc.position.z ) ;
	//
	mat4Dst.SetMatrix3( matPose ) ;
	mat4Dst.SetTranslation( vPos ) ;
}

// アクション登録
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::RegisterAllInteractions
	( const SGLOpenXRProducer::InitConfig * pConfig )
{
	ActionContext *	pContext = NewActionContext() ;

	ActionSet *	pCtrlActionSet =
		pContext->CreateActionSet
			( "controller_action_set", "Controller Action Set" ) ;
	ESLAssert( pCtrlActionSet != nullptr ) ;
	if ( pCtrlActionSet == nullptr )
	{
		return ;
	}

	// 直接 InteractionProfile を指定していて、
	// それが定義済みインタラクションの場合
	// インデックスを予めリストしておく
	SArray<ssize_t>	aExProfDefIndex ;
	ssize_t			iExProfReserved[ipbhControllerTypeCount] ;
	for ( int i = 0; i < ipbhControllerTypeCount; i ++ )
	{
		iExProfReserved[i] = -1 ;
	}
	for ( size_t i = 0; (pConfig != nullptr) && (i < pConfig->nExProfileCount); i ++ )
	{
		ssize_t	iDefType = -1 ;
		for ( size_t j = 0; j < ipbhControllerTypeCount; j ++ )
		{
			if ( strcmp( pConfig->pExHandPathsDesc[i].profilePath.pszBothHandPath,
								s_InteractionProfilePath[j].pszBothHandPath ) == 0 )
			{
				iDefType = (ssize_t) j ;
				iExProfReserved[j] = (ssize_t) i ;
				break ;
			}
		}
		aExProfDefIndex.Add( iDefType ) ;
	}

	// 要素ごとのアクション
	for ( size_t iComponent = 0; iComponent < bhcpiComponentCount; iComponent ++ )
	{
		Action *	pAct =
			pCtrlActionSet->CreateAction
				( s_HandComponentTypeInfos[iComponent].pszActionName,
					s_HandComponentTypeInfos[iComponent].pszLocalizedName,
					s_ActionTypes[s_HandComponentTypeInfos[iComponent].type],
					s_pszUserHandPath, handCount ) ;
		ESLAssert( pAct != nullptr ) ;

		// 定義済み (InteractionProfileBothHand) インタラクション
		for ( size_t iCtrl = 0; iCtrl < ipbhControllerTypeCount; iCtrl ++ )
		{
			if( s_InteractionProfilePath[iCtrl].pszNeedsExtension != nullptr )
			{
				if ( !m_extensions.IsSupported
					( s_InteractionProfilePath[iCtrl].pszNeedsExtension ) )
				{
					continue ;
				}
			}
			const HandComponentPaths *	pDefHandPaths = 
				((pConfig != nullptr) && (pConfig->pDefHandPaths[iCtrl] != nullptr))
					? pConfig->pDefHandPaths[iCtrl] : s_HandComponentPaths[iCtrl] ;
			if ( iExProfReserved[iCtrl] >= 0 )
			{
				pDefHandPaths = pConfig->pExHandPathsDesc[ iExProfReserved[iCtrl] ].pHandPaths ;
			}
			ssize_t	iEntry = -1 ;
			for ( size_t i = 0; (i < bhcpiComponentCount)
								&& (pDefHandPaths[i].iComponent != bhcpiEndOfList); i ++ )
			{
				ESLAssert( pDefHandPaths[i].iComponent < bhcpiComponentCount ) ;
				if ( pDefHandPaths[i].iComponent == iComponent )
				{
					iEntry = (ssize_t) i ;
					break ;
				}
			}
			if ( iEntry < 0 )
			{
				continue ;
			}
			const char *	pszComponentPath = pDefHandPaths[iEntry].pszComponentPath ;
			if ( pszComponentPath == nullptr )
			{
				continue ;
			}
			SArray<char>	bufBindingPath[handCount] ;
			ActionBinding	actBindingBothHandGrip[handCount] ;
			for ( size_t iHand = 0; iHand < handCount; iHand ++ )
			{
				const char *	pszSubPath = pszComponentPath ;
				if ( (iHand == handRight)
					&& (pDefHandPaths[iEntry].pszRightHandPath != nullptr) )
				{
					pszSubPath = pDefHandPaths[iEntry].pszRightHandPath ;
				}
				if ( (pszSubPath == nullptr) || (pszSubPath[0] == 0) )
				{
					continue ;
				}
				bufBindingPath[iHand].AddArray
					( s_pszUserHandPath[iHand], strlen(s_pszUserHandPath[iHand]) ) ;
				bufBindingPath[iHand].AddArray( pszSubPath, strlen(pszSubPath) ) ;
				bufBindingPath[iHand].Add( 0 ) ;
				//
				actBindingBothHandGrip[iHand].action = pAct->m_xrAction ;
				actBindingBothHandGrip[iHand].binding = bufBindingPath[iHand].GetConstArray() ;
			}
			pContext->SuggestInteractionProfileBindings
				( s_InteractionProfilePath[iCtrl].pszBothHandPath,
									actBindingBothHandGrip, handCount ) ;
		}

		// 拡張定義インタラクション
		for ( size_t i = 0; (pConfig != nullptr) && (i < pConfig->nExProfileCount); i ++ )
		{
			if ( aExProfDefIndex.At(i) >= 0 )
			{
				continue ;
			}
			if( pConfig->pExHandPathsDesc[i].profilePath.pszNeedsExtension != nullptr )
			{
				if ( !m_extensions.IsSupported
					( pConfig->pExHandPathsDesc[i].profilePath.pszNeedsExtension ) )
				{
					continue ;
				}
			}
			const HandComponentPaths *
					pHandPaths = pConfig->pExHandPathsDesc[i].pHandPaths ;
			ssize_t	iEntry = -1 ;
			for ( size_t i = 0; (i < bhcpiComponentCount)
								&& (pHandPaths[i].iComponent != bhcpiEndOfList); i ++ )
			{
				ESLAssert( pHandPaths[i].iComponent < bhcpiComponentCount ) ;
				if ( pHandPaths[i].iComponent == iComponent )
				{
					iEntry = (ssize_t) i ;
					break ;
				}
			}
			if ( iEntry < 0 )
			{
				continue ;
			}
			const char *	pszComponentPath = pHandPaths[iEntry].pszComponentPath ;
			if ( pszComponentPath == nullptr )
			{
				continue ;
			}
			SArray<char>	bufBindingPath[handCount] ;
			ActionBinding	actBindingBothHandGrip[handCount] ;
			for ( size_t iHand = 0; iHand < handCount; iHand ++ )
			{
				const char *	pszSubPath = pszComponentPath ;
				if ( (iHand == handRight)
					&& (pHandPaths[iEntry].pszRightHandPath != nullptr) )
				{
					pszSubPath = pHandPaths[iEntry].pszRightHandPath ;
				}
				if ( (pszSubPath == nullptr) || (pszSubPath[0] == 0) )
				{
					continue ;
				}
				bufBindingPath[iHand].AddArray
					( s_pszUserHandPath[iHand], strlen(s_pszUserHandPath[iHand]) ) ;
				bufBindingPath[iHand].AddArray( pszSubPath, strlen(pszSubPath) ) ;
				bufBindingPath[iHand].Add( 0 ) ;
				//
				actBindingBothHandGrip[iHand].action = pAct->m_xrAction ;
				actBindingBothHandGrip[iHand].binding = bufBindingPath[iHand].GetConstArray() ;
			}
			pContext->SuggestInteractionProfileBindings
				( pConfig->pExHandPathsDesc[i].profilePath.pszBothHandPath,
										actBindingBothHandGrip, handCount ) ;
		}

		// 追加
		if ( s_HandComponentTypeInfos[iComponent].type == typeActionPose )
		{
			ESLAssert( iComponent < bhcpiPoseComponentCount ) ;
			for ( size_t iHand = 0; iHand < handCount; iHand ++ )
			{
				m_spaceHandPose[iHand][iComponent].
					CreateActionSpace
						( m_session, pAct,
							m_instance.m_xrpHandPath[iHand] ) ;
			}
		}
		m_pHandActions[iComponent] = pAct ;
	}
}

// アクションの状態を取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::SenseAllInteractions( void )
{
	for ( size_t iHand = 0; iHand < handCount; iHand ++ )
	{
		for ( size_t iComponent = 0; iComponent < bhcpiComponentCount; iComponent ++ )
		{
			ControllerComponentState&	stateHand = m_ccStateHands[iHand][iComponent] ;
			stateHand.type = s_HandComponentTypeInfos[iComponent].type ;
			stateHand.active = false ;
			//
			if (m_pHandActions[iComponent] == nullptr )
			{
				continue ;
			}
			switch ( s_HandComponentTypeInfos[iComponent].type )
			{
			case	typeActionBoolean:
				{
					XrActionStateBoolean	xasBoolean ;
					if ( m_pHandActions[iComponent]->GetActionStateBoolean
						( m_session, xasBoolean,
							m_instance.m_xrpHandPath[iHand] ) )
					{
						stateHand.active = xasBoolean.isActive ;
						stateHand.stateBoolean = xasBoolean.currentState ;
					}
				}
				break ;

			case	typeActionFloat:
				{
					XrActionStateFloat	xasFloat ;
					if ( m_pHandActions[iComponent]->GetActionStateFloat
						( m_session, xasFloat,
							m_instance.m_xrpHandPath[iHand] ) )
					{
						stateHand.active = xasFloat.isActive ;
						stateHand.stateFloat = xasFloat.currentState ;
					}
				}
				break ;

			case	typeActionPose:
				ESLAssert( iComponent < bhcpiPoseComponentCount ) ;
				if ( iComponent < bhcpiPoseComponentCount )
				{
					XrPosef	xrPose ;
					if ( m_spaceHandPose[iHand][iComponent].LocateSpace
								( xrPose, m_spaceApp, CurrentFrameTime() ) )
					{
						XrActionStatePose	xasPose ;
						if ( m_pHandActions[iComponent]->GetActionStatePose
							( m_session, xasPose,
								m_instance.m_xrpHandPath[iHand] ) )
						{
							stateHand.active = xasPose.isActive ;
							stateHand.statePose = xrPose ;
						}
					}
				}
				break ;

			case	typeActionVector2:
			case	typeActionHaptic:
			default:
				break ;
			}
		}
	}

	LockForDeviceState() ;
	for ( size_t iHand = 0; iHand < handCount; iHand ++ )
	{
		Posture&	postureHand = m_postureHands[iHand] ;
		postureHand.nFlags = 0 ;
		//
		ControllerState&	stateCtrl = m_stateControllers[iHand] ;
		stateCtrl.nState = 0 ;
		stateCtrl.nCapacity = capAxisThumbStick | capAxisIndexTrigger | capAxisMiddleTrigger ;
		stateCtrl.maskButtonPressed = 0 ;
		stateCtrl.maskButtonTouched = 0 ;
		for ( int i = 0; i < axisCount; i ++ )
		{
			stateCtrl.vAxis[i].x = 0 ;
			stateCtrl.vAxis[i].y = 0 ;
		}
		//
		// 手の位置
		//
		if ( m_ccStateHands[iHand][bhcpiGripPose].active )
		{
			GetPostureFromXrPosef
				( postureHand,
					m_ccStateHands[iHand][bhcpiGripPose].statePose ) ;
			//
			stateCtrl.nState = controllerConnected ;
			postureHand.nFlags = trackedOrientation | trackedPosition ;
			postureHand.vPosition += m_vTrackingBasePos ;
		}
		//
		// コントローラーの状態
		//
		if ( m_ccStateHands[iHand][bhcpiSelectClick].active
			&& m_ccStateHands[iHand][bhcpiSelectClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis1 ;
		}
		if ( m_ccStateHands[iHand][bhcpiMenuClick].active
			&& m_ccStateHands[iHand][bhcpiMenuClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAppMenu ;
		}
		if ( m_ccStateHands[iHand][bhcpiButton1].active
			&& m_ccStateHands[iHand][bhcpiButton1].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << button1 ;
		}
		if ( m_ccStateHands[iHand][bhcpiButton1Touch].active
			&& m_ccStateHands[iHand][bhcpiButton1Touch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << button1 ;
		}
		if ( m_ccStateHands[iHand][bhcpiButton2].active
			&& m_ccStateHands[iHand][bhcpiButton2].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << button2 ;
		}
		if ( m_ccStateHands[iHand][bhcpiButton2Touch].active
			&& m_ccStateHands[iHand][bhcpiButton2Touch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << button2 ;
		}
		if ( m_ccStateHands[iHand][bhcpiThumbkClick].active
			&& m_ccStateHands[iHand][bhcpiThumbkClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis0 ;
		}
		if ( m_ccStateHands[iHand][bhcpiThumbTouch].active
			&& m_ccStateHands[iHand][bhcpiThumbTouch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << buttonAxis0 ;
		}
		if ( m_ccStateHands[iHand][bhcpiThumbX].active )
		{
			stateCtrl.vAxis[axisThumbStick].x =
					m_ccStateHands[iHand][bhcpiThumbX].stateFloat ;
		}
		if ( m_ccStateHands[iHand][bhcpiThumbY].active )
		{
			stateCtrl.vAxis[axisThumbStick].y =
					m_ccStateHands[iHand][bhcpiThumbY].stateFloat ;
		}
		if ( m_ccStateHands[iHand][bhcpiTrackpadClick].active
			&& m_ccStateHands[iHand][bhcpiTrackpadClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis5 ;
		}
		if ( m_ccStateHands[iHand][bhcpiTrackpadTouch].active
			&& m_ccStateHands[iHand][bhcpiTrackpadTouch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << buttonAxis5 ;
		}
		if ( m_ccStateHands[iHand][bhcpiTrackpadX].active )
		{
			stateCtrl.vAxis[axisJoyStick2].x =
					m_ccStateHands[iHand][bhcpiTrackpadX].stateFloat ;
		}
		if ( m_ccStateHands[iHand][bhcpiTrackpadY].active )
		{
			stateCtrl.vAxis[axisJoyStick2].y =
					m_ccStateHands[iHand][bhcpiTrackpadY].stateFloat ;
		}
		if ( m_ccStateHands[iHand][bhcpiTriggerClick].active
			&& m_ccStateHands[iHand][bhcpiTriggerClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis1 ;
		}
		if ( m_ccStateHands[iHand][bhcpiTriggerTouch].active
			&& m_ccStateHands[iHand][bhcpiTriggerTouch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << buttonAxis1 ;
		}
		if ( m_ccStateHands[iHand][bhcpiTriggerValue].active )
		{
			stateCtrl.vAxis[axisIndexTrigger].x =
					m_ccStateHands[iHand][bhcpiTriggerValue].stateFloat ;
			if ( stateCtrl.vAxis[axisIndexTrigger].x > 0.8 )
			{
				stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis1 ;
			}
		}
		if ( m_ccStateHands[iHand][bhcpiSqueezeClick].active
			&& m_ccStateHands[iHand][bhcpiSqueezeClick].stateBoolean )
		{
			stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis2 ;
		}
		if ( m_ccStateHands[iHand][bhcpiSqueezeTouch].active
			&& m_ccStateHands[iHand][bhcpiSqueezeTouch].stateBoolean )
		{
			stateCtrl.maskButtonTouched |= (uint64_t) 1 << buttonAxis2 ;
		}
		if ( m_ccStateHands[iHand][bhcpiSqueezeValue].active )
		{
			stateCtrl.vAxis[axisMiddleTrigger].x =
					m_ccStateHands[iHand][bhcpiSqueezeValue].stateFloat ;
			if ( stateCtrl.vAxis[axisMiddleTrigger].x > 0.8 )
			{
				stateCtrl.maskButtonPressed |= (uint64_t) 1 << buttonAxis2 ;
			}
		}
	}
	UnlockForDeviceState() ;
}

// ポーリングスレッド開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::BeginPollingThread( void )
{
	if ( m_flagBegunThread )
	{
		return	sglErrContinue ;
	}
	m_flagAbortedPolling = false ;
	m_flagQuitThread = false ;
	m_signalReadyFrame.Initialize( false ) ;
	m_signalReadyInput.Initialize( false ) ;
	m_signalDoneFrame.Initialize( false ) ;

	if ( m_threadPoll.BeginThread( this ) )
	{
		ESLTrace( "failed to begin thread for polling.\n" ) ;
		return	sglErrFailed ;
	}
	else
	{
		ESLTrace( "Begin OpenXR polling thread.\n" ) ;
		m_flagBegunThread = true ;
	}
	return	sglErrSuccess ;
}

// ポーリングスレッド終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::EndPollingThread( void )
{
	if ( m_flagBegunThread )
	{
		m_flagQuitThread = true ;
		m_threadPoll.Wait() ;
		m_threadPoll.Delete() ;
		m_signalReadyFrame.Delete() ;
		m_signalReadyInput.Delete() ;
		m_signalDoneFrame.Delete() ;
		m_flagBegunThread = false ;
	}
}

// ポーリングスレッド実行中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::IsPollingThreadRunning( void ) const
{
	return	m_flagBegunThread && !m_flagAbortedPolling ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::Run( void )
{
	m_timerFlipView.Reset() ;
	while ( !m_flagQuitThread )
	{
		if ( PollEvents() == sglErrAbort )
		{
			break ;
		}
		PollingControllerModels() ;

		if ( IsSessionRunning() )
		{
			if ( WaitFrame() == sglErrSuccess )
			{
				UpdateAllViewsLocation() ;
				SyncActions() ;
				SenseAllInteractions() ;
				//
				// フレーム描画開始可能通知
				//
				LockForDeviceState() ;
				m_signalReadyInput.SetSignal() ;
				m_signalReadyFrame.SetSignal() ;
				UnlockForDeviceState() ;
				//
				// フレーム描画完了待ち
				//
				do
				{
					if ( m_flagQuitThread )
					{
						break ;
					}
				}
				while ( m_signalDoneFrame.Wait( 10 ) == errTimeout ) ;
				m_signalDoneFrame.ResetSignal() ;
			}
			else
			{
				SleepMilliSec( 1 ) ;
			}
		}
		else
		{
			SleepMilliSec( 10 ) ;
		}
	}
	m_flagAbortedPolling = true ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLOpenXRProducer::BeginDrawView
	( SGLAbstractWindow * pPrimaryWnd )
{
	if ( !IsPollingThreadRunning() || !IsSessionRunning() )
	{
		return	nullptr ;
	}
	ViewContext *	pView = GetViewContextAt(0) ;
	if ( pView == nullptr )
	{
		return	nullptr ;
	}
	if ( m_signalReadyFrame.Wait( 100 ) != errSuccess )
	{
		return	nullptr ;
	}
	m_signalReadyFrame.ResetSignal() ;
	//
	if ( BeginFrame() )
	{
		m_signalDoneFrame.SetSignal() ;
		return	nullptr ;
	}
	double	fpHMScale = GetScaleHMDToModel() ;
	m_pRenderer->SetZClipRange( 0.2 * fpHMScale, 2000.0 * fpHMScale ) ;
	//
	if ( !pView->AcquireSwapchain()
		|| !pView->AttachSwapchainToRenderer( *this, *m_pRenderer ) )
	{
		m_signalDoneFrame.SetSignal() ;
		return	nullptr ;
	}
	AppendSubmitView( pView ) ;
	//
	m_pRenderer->GetDirectlyRenderer().
		SetViewVerticalOrder( S3DOpenGLDirectlyRenderer::verticalStright ) ;
	//
	S3DVector	vScreen ;
	vScreen.x = (float32_t) m_fovEyes[0].sizeOfView.w * 0.5f ;
	vScreen.y = (float32_t) m_fovEyes[0].sizeOfView.h * 0.5f ;
	vScreen.z = (vScreen.x + vScreen.y) * 0.5f ;
	m_pRenderer->SetProjectionScreen( vScreen ) ;
	//
	if ( m_flagDrawToPrimary )
	{
		m_pBackRenderer->SetZClipRange( 0.2 * fpHMScale, 2000.0 * fpHMScale ) ;
		pView->AttachBackBufferToRenderer( *this, *m_pBackRenderer ) ;
		m_pBackRenderer->SetProjectionScreen( vScreen ) ;
		m_flagBegunDrawView = true ;
		return	m_pBackRenderer ;
	}
	else
	{
		m_flagBegunDrawView = true ;
		return	m_pRenderer ;
	}
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::EndDrawView
	( SGLAbstractWindow * pPrimaryWnd, RenderContext * render )
{
	ESLAssert( m_flagBegunDrawView ) ;
	ViewContext *	pView = GetViewContextAt(0) ;
	if ( (pView == nullptr) || (render == nullptr) || !m_flagBegunDrawView )
	{
		return ;
	}
	if ( m_flagDrawToPrimary )
	{
		pView->BltToSwapchainFromBackBuffer( *m_pRenderer, *m_pBackRenderer ) ;
	}
	pView->DetachSwapchainFromRenderer( *m_pRenderer ) ;

	if ( m_flagDrawToPrimary )
	{
		SGLImageRect	rectRender, rectDisplay ;
		if ( !pPrimaryWnd->GetInternalDisplayPosition
								( rectRender, rectDisplay ) )
		{
			RenderContext *	pRenderPrimary = pPrimaryWnd->GetRenderContext() ;
			if ( pRenderPrimary != nullptr )
			{
				pView->BltPrimaryFramebufferTo
					( *pRenderPrimary,
						*m_pBackRenderer,
						rectRender.x, rectRender.y,
						rectRender.x + rectRender.w,
						rectRender.y + rectRender.h ) ;
				pPrimaryWnd->ReleaseRenderContext( pRenderPrimary ) ;
			}
		}
	}

	pView->ReleaseSwapchain() ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::FlipView
	( SGLAbstractWindow * pPrimaryWnd, bool fVSync )
{
	if ( !IsPollingThreadRunning() || !IsSessionRunning() || !m_flagBegunDrawView )
	{
		return ;
	}
	EndFrame() ;

	m_flagBegunDrawView = false ;
	m_timerFlipView.Reset() ;
	m_signalDoneFrame.SetSignal() ;
}

// 表示状態か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::IsVisibleView( void ) const
{
	return	IsSessionRunning() ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::IsStereoDisplayMode( void )
{
	ViewContext *	pView = GetViewContextAt(0) ;
	return	(pView != nullptr) && (pView->GetViewCount() >= 2) ;
}

// プライマリウィンドウへの描画も行うか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::DoesDrawToPrimaryWindow( void )
{
	return	m_flagDrawToPrimary ;
}

// デバイス状態更新タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::WaitForPollDeviceState( int64_t msecTimeout )
{
	SGLError	err =
		(SGLError) m_signalReadyInput.Wait( msecTimeout ) ;
	if ( !err )
	{
		LockForDeviceState() ;
		m_signalReadyInput.ResetSignal() ;
		UnlockForDeviceState() ;
	}
	return	err ;
}

// 現在の HMD 姿勢を基準座標・姿勢にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::ResetCurrentHMDPosture( void )
{
	LockForDeviceState() ;
	m_vTrackingBasePos -= S3DVector( GetHeadPosture().vPosition ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// 現在の HMD 座標を基準座標にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::ResetCurrentHMDPosition( void )
{
	LockForDeviceState() ;
	m_vTrackingBasePos -= S3DVector( GetHeadPosture().vPosition ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// HMD 空間タイプ取得
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::ViewSpaceType SGLOpenXRProducer::GetViewSpaceType( void ) const
{
	return	m_viewSpaceType ;
}

// デバイスモデル取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::GetControllerModel
		( SGLVRViewProducer::ModelInfo& mi, ControllerIndex iCtrl )
{
	mi.status = statusModelError ;
	mi.nFlags = 0 ;

	LockForDeviceState() ;
	ControllerModel *	pCtrlModel = m_aControllerModels.GetAt( iCtrl ) ;
	if ( (pCtrlModel != nullptr)
		&& (pCtrlModel->m_pModel != nullptr) )
	{
		mi.status = statusModelCompleted ;
		mi.pVBO = pCtrlModel->m_pModel ;
	}
	if ( m_ccStateHands[iCtrl][bhcpiGripPose].active )
	{
		GetPostureFromXrPosef
			( mi.postureModel, m_ccStateHands[iCtrl][bhcpiGripPose].statePose ) ;
		mi.postureModel.nFlags = trackedOrientation | trackedPosition ;
		mi.nFlags |= modelVisible ;
		//
		if ( m_ccStateHands[iCtrl][bhcpiAimPose].active )
		{
			GetPostureFromXrPosef
				( mi.postureTipLocal, m_ccStateHands[iCtrl][bhcpiAimPose].statePose ) ;
			mi.postureTipLocal.nFlags = trackedOrientation | trackedPosition ;
			//
			S3DMatrix	matIModel = mi.postureModel.matOrientation.Inverse() ;
			mi.postureTipLocal.matOrientation =
					matIModel * mi.postureTipLocal.matOrientation ;
			mi.postureTipLocal.vPosition =
					matIModel * (mi.postureTipLocal.vPosition
										- mi.postureModel.vPosition) ;
		}
		else
		{
			mi.postureTipLocal.matOrientation = S3DMatrix( 1, 1, 1 ) ;
			mi.postureTipLocal.vPosition = S3DVector( 0, 0, 0 ) ;
			mi.postureTipLocal.nFlags = 0 ;
		}
	}
	UnlockForDeviceState() ;

	return	sglErrSuccess ;
}

// HMD スケール
//////////////////////////////////////////////////////////////////////////////
void SGLOpenXRProducer::SetScaleHMDToModel( double fpScale )
{
	LockForDeviceState() ;

	float32_t	fpScaleRate = (float32_t) (fpScale / GetScaleHMDToModel()) ;
	SGLVRViewProducer::SetScaleHMDToModel( fpScale ) ;

	m_postureHead.vPosition *= fpScaleRate ;
	m_postureEyes[eyeRight].vPosition *= fpScaleRate ;
	m_postureEyes[eyeLeft].vPosition *= fpScaleRate ;
	m_postureHands[handRight].vPosition *= fpScaleRate ;
	m_postureHands[handLeft].vPosition *= fpScaleRate ;

	UnlockForDeviceState() ;
}

// バイブレーション開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::StartVibration
	( SGLVRViewProducer::ControllerIndex iCtrl,
		const SGLVRViewProducer::VibrationParam& vibParam )
{
	if ( m_pHandActions[bhcpiHapticOutput] == nullptr )
	{
		return	sglErrNotSupported ;
	}

	XrHapticVibration	xhv{ XR_TYPE_HAPTIC_VIBRATION } ;
	xhv.duration = (vibParam.duration == vibMinDuration)
					? (XrDuration) XR_MIN_HAPTIC_DURATION
					: (((XrDuration) vibParam.duration) * 1000000) ;
	xhv.frequency = (vibParam.frequency == vibDefaultFrequency)
					? XR_FREQUENCY_UNSPECIFIED : vibParam.frequency ;
	xhv.amplitude = vibParam.amplitude ;

	XrHapticActionInfo	xhai{ XR_TYPE_HAPTIC_ACTION_INFO } ;
	xhai.action = m_pHandActions[bhcpiHapticOutput]->m_xrAction ;

	XrResult	xr = xrApplyHapticFeedback
		( m_session.m_xrSession, &xhai, (const XrHapticBaseHeader*) &xhv ) ;
	if ( !XRVerify( xr, "xrApplyHapticFeedback" ) )
	{
		return	sglErrFailed ;
	}
	if ( xr == XR_SESSION_NOT_FOCUSED )
	{
		ESLTrace( "xrApplyHapticFeedback returned XR_SESSION_NOT_FOCUSED.\n" ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// バイブレーション即時停止
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::StopVibration( SGLVRViewProducer::ControllerIndex iCtrl )
{
	if ( m_pHandActions[bhcpiHapticOutput] == nullptr )
	{
		return	sglErrNotSupported ;
	}

	XrHapticActionInfo	xhai{ XR_TYPE_HAPTIC_ACTION_INFO } ;
	xhai.action = m_pHandActions[bhcpiHapticOutput]->m_xrAction ;

	XrResult	xr = xrStopHapticFeedback( m_session.m_xrSession, &xhai ) ;
	if ( !XRVerify( xr, "xrStopHapticFeedback" ) )
	{
		return	sglErrFailed ;
	}
	if ( xr == XR_SESSION_NOT_FOCUSED )
	{
		ESLTrace( "xrStopHapticFeedback returned XR_SESSION_NOT_FOCUSED.\n" ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 更新タイミング待ち
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenXRProducer::WaitForView( int64_t msecTimeout )
{
	if ( !IsSessionRunning() )
	{
		if ( m_timerFlipView.GetTime() > 30 )
		{
			m_timerFlipView.Reset() ;
			return	sglErrSuccess ;
		}
	}
	return	(SGLError) m_signalReadyFrame.Wait( msecTimeout ) ;
}

// VR が使用可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::IsVRAvailable( void )
{
	ExtensionContext	xc ;
	xc.EnumExtentions() ;

#ifdef XR_USE_GRAPHICS_API_OPENGL
	return	xc.supportsOpenGL ;
#else
	return	false ;
#endif
}

// XrResult 失敗・成功判定
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenXRProducer::XRVerify( XrResult xr, const char * pszFunc )
{
	ESLAssert( !XR_FAILED(xr) ) ;
	if ( XR_FAILED(xr) )
	{
		ESLTrace( "OpenXR failed to %s (#%08x)\n", pszFunc, xr ) ;
		return	false ;
	}
	return	true ;
}
