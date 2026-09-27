
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_vr_openvr_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// OpenVR HMD 用描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLOpenVRProducer::Renderer, S3DOpenGLBufferedRenderer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenVRProducer::Renderer::Renderer
		( SGLOpenVRProducer * vr, SGLOpenGLContext * pOpenGL )
	: S3DOpenGLBufferedRenderer( pOpenGL ), m_pVR( vr )
{
}

// 透視変換行列更新
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::Renderer::UpdatePerspectiveMatrix( void )
{
	const vr::EVREye	vrEyeIndexes[2] =
	{
		vr::Eye_Right, vr::Eye_Left
	} ;
	const RenderContext::StereoViewIndex	sviIndexes[2] =
	{
		RenderContext::stereoViewRight,
		RenderContext::stereoViewLeft
	} ;
	for ( int i = 0; i < 2; i ++ )
	{
		vr::HmdMatrix44_t	mat44Proj =
			m_pVR->m_pHMD->GetProjectionMatrix
				( vrEyeIndexes[i], (float) m_zMinClip, (float) m_zMaxClip ) ;
		//
		S4DMatrix	matPers ;
		for ( int j = 0; j < 4; j ++ )
		{
			matPers.m[j][0] = mat44Proj.m[j][0] ;
			matPers.m[j][1] = - mat44Proj.m[j][1] ;
			matPers.m[j][2] = - mat44Proj.m[j][2] ;
			matPers.m[j][3] = mat44Proj.m[j][3] ;
		}
		SetPerspectiveMatrix( sviIndexes[i], matPers, true ) ;
	}
}

// Flush 関数処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::Renderer::OnGLThreadFlush( bool flagFinish )
{
//	UpdatePerspectiveMatrix() ;
	S3DOpenGLBufferedRenderer::OnGLThreadFlush( flagFinish ) ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenVR 出力インターフェース
//////////////////////////////////////////////////////////////////////////////

const int	SGLOpenVRProducer::m_iVRBurronTransTable[64] =
{
	vr::k_EButton_DPad_Up,
	vr::k_EButton_DPad_Down,
	vr::k_EButton_DPad_Left,
	vr::k_EButton_DPad_Right,
	vr::k_EButton_A,    vr::k_EButton_A+1,  vr::k_EButton_A+2,  vr::k_EButton_A+3,
	vr::k_EButton_A+4,  vr::k_EButton_A+5,  vr::k_EButton_A+6,  vr::k_EButton_A+7,
	vr::k_EButton_A+8,  vr::k_EButton_A+9,  vr::k_EButton_A+10, vr::k_EButton_A+11,
	//
	vr::k_EButton_A+12, vr::k_EButton_A+13, vr::k_EButton_A+14, vr::k_EButton_A+15,
	vr::k_EButton_A+16, vr::k_EButton_A+17, vr::k_EButton_A+18, vr::k_EButton_A+19,
	vr::k_EButton_A+20, vr::k_EButton_A+21, vr::k_EButton_A+22, vr::k_EButton_A+23,
	-1, -1, -1, -1,
	//
	vr::k_EButton_System,
	vr::k_EButton_ApplicationMenu,
	vr::k_EButton_Grip, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	//
	vr::k_EButton_Axis0,
	vr::k_EButton_Axis1,
	vr::k_EButton_Axis2,
	vr::k_EButton_Axis3,
	vr::k_EButton_Axis4, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLOpenVRProducer,
		SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOpenVRProducer::SGLOpenVRProducer( SGLOpenGLContext * pOpenGL )
	: m_pOpenGL( pOpenGL ), m_vPositionBase( 0, 0, 0 )
{
	m_status = statusUnintialized ;
	m_pHMD = NULL ;
	m_pRenderModels = NULL ;
	for ( int i = 0; i < vr::k_unMaxTrackedDeviceCount; i ++ )
	{
		m_flagControllerState[i] = false ;
	}
	for ( int i = 0; i < controllerCount; i ++ )
	{
		m_flagShowControllerModel[i] = true ;
		m_flagPreparedAxisIndex[i] = false ;
		for ( int j = 0; j < 5; j ++ )
		{
			m_mapAxisIndex[i][j] = axisInvalid ;
		}
	}
	m_tdiHMD = vr::k_unTrackedDeviceIndexInvalid ;
	m_tdiRightHand = vr::k_unTrackedDeviceIndexInvalid ;
	m_tdiLeftHand = vr::k_unTrackedDeviceIndexInvalid ;
	//
	m_flagBeganThread = false ;
	m_flagQuitThread = false ;
	//
	m_pRenderer = new Renderer( this, pOpenGL ) ;
}

// ランタイム初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::InitializeLib( void )
{
	//
	// SteamVR ランタイム読み込み
	//
	vr::EVRInitError	errInit = vr::VRInitError_None ;
	m_pHMD = vr::VR_Init( &errInit, vr::VRApplication_Scene ) ;
	if ( errInit != vr::VRInitError_None )
	{
		m_pHMD = NULL ;
		m_status = statusFailedInitialize ;
		//
		LogTrace( L"failed to vr::VR_Init: %s\n",
				(const wchar_t*) SString(vr::VR_GetVRInitErrorAsEnglishDescription( errInit )) ) ;
		return	sglErrFailed ;
	}
	m_pRenderModels =
		(vr::IVRRenderModels*)
			vr::VR_GetGenericInterface
				( vr::IVRRenderModels_Version, &errInit ) ;
	if ( m_pRenderModels == NULL )
	{
		m_pHMD = NULL ;
		m_status = statusFailedInitialize ;
		vr::VR_Shutdown() ;
		//
		LogTrace( L"failed to get IVRRenderModels: %s\n",
				(const wchar_t*) SString(vr::VR_GetVRInitErrorAsEnglishDescription( errInit )) ) ;
		return	sglErrFailed ;
	}
	if ( vr::VRCompositor() == NULL )
	{
		LogTrace( L"failed to vr::VRCompositor()\n" ) ;
		return	sglErrFailed ;
	}
	LogTrace( L"initialized OpenVR runtime \'%s\'\n",
				(const wchar_t *) SString(vr::VR_RuntimePath()) ) ;
	//
	m_status = statusLibInit ;
	return	sglErrSuccess ;
}

// 初期設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::PrepareDevice( ViewSpaceType spaceType )
{
	if ( m_status != statusLibInit )
	{
		return	sglErrFailed ;
	}
	vr::VRCompositor()->SetTrackingSpace
		( (spaceType == spaceEyeLevel)
			? vr::TrackingUniverseSeated : vr::TrackingUniverseStanding ) ;
	m_vPositionBase = S3DDVector( 0, 0, 0 ) ;
	//
	// 設定取得
	//
	uint32_t	wRenderTarget, hRenderTarget ;
	m_pHMD->GetRecommendedRenderTargetSize( &wRenderTarget, &hRenderTarget ) ;
	LogTrace( L"OpenVR HMD render target size: %d x %d\n", wRenderTarget, hRenderTarget ) ;
	//
	m_tdiHMD = vr::k_unTrackedDeviceIndex_Hmd ;
	m_tdiRightHand =
		m_pHMD->GetTrackedDeviceIndexForControllerRole
						( vr::TrackedControllerRole_RightHand ) ;
	m_tdiLeftHand =
		m_pHMD->GetTrackedDeviceIndexForControllerRole
						( vr::TrackedControllerRole_LeftHand ) ;
	if ( m_tdiRightHand != vr::k_unTrackedDeviceIndexInvalid )
	{
		LogTrace( L"OpenVR right hand device : #%d\n", m_tdiRightHand ) ;
		PrepareControllerAxisIndexTable
			( &m_mapAxisIndex[controllerRight][0], m_tdiRightHand ) ;
	}
	if ( m_tdiLeftHand != vr::k_unTrackedDeviceIndexInvalid )
	{
		LogTrace( L"OpenVR left hand device : #%d\n", m_tdiLeftHand ) ;
		PrepareControllerAxisIndexTable
			( &m_mapAxisIndex[controllerLeft][0], m_tdiLeftHand ) ;
	}
	//
	// 両目の設定
	//
	const wchar_t *	pszEyeName[2] =
	{
		L"right eye", L"left eye"
	} ;
	const vr::EVREye	vrEyeIndexes[2] =
	{
		vr::Eye_Right, vr::Eye_Left
	} ;
	const EyeIndex	eyeIndexes[2] =
	{
		eyeRight, eyeLeft
	} ;
	for ( int i = 0; i < 2; i ++ )
	{
		LogTrace( L"\nOpenVR HMD %s;\n", pszEyeName[i] ) ;
		const EyeIndex	eye = eyeIndexes[i] ;
		//
		// 透視変換取得
		//
		m_mat4HMDProjection[eye] =
			m_pHMD->GetProjectionMatrix( vrEyeIndexes[i], 0.0f, 1.0f ) ;
		//
		SGLSize	sizeView( wRenderTarget, hRenderTarget ) ;
		ProjectionParamFromMatrix4
			( m_fovEyes[eye], m_flagReverseZ[eye],
					m_mat4HMDProjection[eye], sizeView ) ;
		LogTrace( L"  projection: (%f, %f) / %f : %f\n",
					m_fovEyes[eye].vScreenPos.x,
					m_fovEyes[eye].vScreenPos.y,
					m_fovEyes[eye].vScreenPos.z,
					m_fovEyes[eye].fpPixelAspect ) ;
		//
		// 目の位置取得
		//
		GetPostureFromHmdMatrix34
			( m_postureEyes[eye],
				m_pHMD->GetEyeToHeadTransform( vrEyeIndexes[i] ) ) ;
		LogTrace( L"  eye posture: %.5f %.5f %.5f %.5f\n"
				  "               %.5f %.5f %.5f %.5f\n"
				  "               %.5f %.5f %.5f %.5f\n",
					m_postureEyes[eye].matOrientation.m[0][0],
					m_postureEyes[eye].matOrientation.m[0][1],
					m_postureEyes[eye].matOrientation.m[0][2],
					m_postureEyes[eye].vPosition.x,
					m_postureEyes[eye].matOrientation.m[1][0],
					m_postureEyes[eye].matOrientation.m[1][1],
					m_postureEyes[eye].matOrientation.m[1][2],
					m_postureEyes[eye].vPosition.y,
					m_postureEyes[eye].matOrientation.m[2][0],
					m_postureEyes[eye].matOrientation.m[2][1],
					m_postureEyes[eye].matOrientation.m[2][2],
					m_postureEyes[eye].vPosition.z ) ;
		//
		// バッファ生成
		//
		m_imgRenderColor[i].CreateImage
			( wRenderTarget, hRenderTarget,
				formatImageABGR, 32,
				SGLImageObject::bufferMultisample
					| SGLImageObject::bufferNonPowerOf2
					| SGLImageObject::bufferOnDeviceOnly, 4 ) ;
		m_imgResolveColor[i].CreateImage
			( wRenderTarget, hRenderTarget,
				formatImageABGR, 32, SGLImageObject::bufferOnDeviceOnly ) ;
		m_imgRenderDepth[i].CreateImage
			( wRenderTarget, hRenderTarget,
				formatImageDepth, 32,
				SGLImageObject::bufferMultisample
					| SGLImageObject::bufferDeviceRenderBuffer
					| SGLImageObject::bufferNonPowerOf2
					| SGLImageObject::bufferOnDeviceOnly, 4 ) ;
		//
		// ミラー用フレームバッファ生成
		//
		SGLImageRect	rectRef ;
		m_pglResolve[eye] =
			SGLOpenGLTextureBuffer::CommitGLTexture
					( m_pOpenGL, &m_imgResolveColor[i], rectRef ) ;
		m_fboResolve[eye].AttachFrameBuffer( m_pglResolve[eye], NULL ) ;
	}
	//
	// ミラーリング設定
	//
	SGLImageRect	rectSrcFBO( 0, 0, wRenderTarget, hRenderTarget ) ;
	SGLImageRect	rectDstFBO( 0, 0, wRenderTarget, hRenderTarget ) ;
//	if ( m_flagReverseZ[0] )
	{
//		rectDstFBO.x = (int) wRenderTarget ;
		rectDstFBO.y = (int) hRenderTarget ;
//		rectDstFBO.w = - (int) wRenderTarget ;
		rectDstFBO.h = - (int) hRenderTarget ;
	}
	m_pRenderer->SetMirrorFrameBuffer
		( m_fboResolve[eyeRight].GetFrameBufferOGL(),
			m_fboResolve[eyeLeft].GetFrameBufferOGL(), rectSrcFBO, rectDstFBO ) ;
	//
	// ポーリングスレッド開始
	//
	ESLAssert( !m_flagBeganThread ) ;
	m_flagQuitThread = false ;
	m_signalReadyFrame.Initialize( false ) ;
	m_signalReadyInput.Initialize( false ) ;
	m_signalDoneFrame.Initialize( false ) ;
	if ( m_threadPoll.BeginThread( this ) )
	{
		LogTrace( L"failed to begin thread for polling.\n" ) ;
	}
	else
	{
		LogTrace( L"\nBegin OpenVR tracking thread.\n" ) ;
		m_flagBeganThread = true ;
	}

	m_status = statusInitialized ;
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::Release( void )
{
	if ( m_flagBeganThread )
	{
		m_flagQuitThread = true ;
		m_threadPoll.Wait() ;
		m_threadPoll.Delete() ;
		m_signalReadyFrame.Delete() ;
		m_signalReadyInput.Delete() ;
		m_signalDoneFrame.Delete() ;
		m_flagBeganThread = false ;
	}
	const EyeIndex	eyeIndexes[2] =
	{
		eyeRight, eyeLeft
	} ;
	for ( int i = 0; i < 2; i ++ )
	{
		const EyeIndex	eye = eyeIndexes[i] ;
		m_fboResolve[eye].ReleaseFrameBuffer() ;
		m_imgRenderColor[i].ReleaseBuffer() ;
		m_imgResolveColor[i].ReleaseBuffer() ;
		m_imgRenderDepth[i].ReleaseBuffer() ;
	}
	if ( m_status >= statusLibInit )
	{
		vr::VR_Shutdown() ;
		m_pHMD = NULL ;
		m_status = statusUnintialized ;
	}
	return	sglErrSuccess ;
}

// VR が使用可能か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenVRProducer::IsVRAvailable( void )
{
	return	vr::VR_IsHmdPresent() ;
}

// コントローラーの軸パラメータ番号変換テーブル生成
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::PrepareControllerAxisIndexTable
	( SGLVRViewProducer::AxisIndex * pAxisIndex,
					vr::TrackedDeviceIndex_t tdiDevice ) const
{
	const vr::ETrackedDeviceProperty	propAxisType[] =
	{
		vr::Prop_Axis0Type_Int32,
		vr::Prop_Axis1Type_Int32,
		vr::Prop_Axis2Type_Int32,
		vr::Prop_Axis3Type_Int32,
		vr::Prop_Axis4Type_Int32,
		vr::Prop_Invalid,
	} ;
	const AxisIndex	axisStickIndex[] =
	{
		axisThumbStick,
		axisJoyStick2,
		axisJoyStick3,
		axisJoyStick4,
	} ;
	const AxisIndex	axisTriggerIndex[] =
	{
		axisIndexTrigger,
		axisMiddleTrigger,
		axisRingTrigger,
		axisLittileTrigger,
	} ;
	int	iStick = 0 ;
	int	iTrigger = 0 ;
	for ( int i = 0; propAxisType[i] != vr::Prop_Invalid; i ++ )
	{
		vr::ETrackedPropertyError	propError ;
		vr::EVRControllerAxisType	typeAxis =
			(vr::EVRControllerAxisType)
				m_pHMD->GetInt32TrackedDeviceProperty
					( tdiDevice, propAxisType[i], &propError ) ;
		if ( propError == vr::TrackedProp_Success )
		{
			if ( typeAxis == vr::k_eControllerAxis_Trigger )
			{
				if ( iStick < sizeof(axisTriggerIndex)
								/sizeof(axisTriggerIndex[0]) )
				{
					pAxisIndex[i] = axisTriggerIndex[iTrigger ++] ;
				}
				else
				{
					pAxisIndex[i] = axisInvalid ;
				}
			}
			else if ( (typeAxis == vr::k_eControllerAxis_TrackPad)
					|| (typeAxis == vr::k_eControllerAxis_Joystick) )
			{
				if ( iStick < sizeof(axisStickIndex)
								/sizeof(axisStickIndex[0]) )
				{
					pAxisIndex[i] = axisStickIndex[iStick ++] ;
				}
				else
				{
					pAxisIndex[i] = axisInvalid ;
				}
			}
			else
			{
				pAxisIndex[i] = axisInvalid ;
			}
		}
	}
}

// HmdMatrix44_t から透視変換パラメータ逆算
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::ProjectionParamFromMatrix4
	( SGLVRViewProducer::EyeFieldOfView& fov, bool& zReverse,
		const vr::HmdMatrix44_t& mat, const SGLSize& sizeView )
{
	float32_t	ws = (float32_t) sizeView.w * 0.5f ;
	float32_t	hs = (float32_t) sizeView.h * 0.5f ;
	float32_t	xs = mat.m[0][0] / mat.m[3][2] ;
	float32_t	ys = mat.m[1][1] / mat.m[3][2] ;
	float32_t	xo = mat.m[0][2] / mat.m[3][2] ;
	float32_t	yo = mat.m[1][2] / mat.m[3][2] ;
	//
	fov.sizeOfView = sizeView ;
	fov.vScreenPos.x = (xo + 1.0f) * ws ;
	fov.vScreenPos.y = (yo + 1.0f) * hs ;
	fov.vScreenPos.z = ys * hs ;
	fov.fpPixelAspect = hs * ys / (ws * xs) ;
	zReverse = false ;
	//
//	if ( fov.vScreenPos.z < 0.0f )
	{
//		fov.vScreenPos.x = ws + (ws - fov.vScreenPos.x) ;
		fov.vScreenPos.y = hs + (hs - fov.vScreenPos.y) ;
		fov.vScreenPos.z = - fov.vScreenPos.z ;
		zReverse = true ;
	}
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLOpenVRProducer::BeginDrawView
	( SGLAbstractWindow * pPrimaryWnd )
{
	if ( m_status < statusInitialized )
	{
		return	NULL ;
	}
	if ( m_signalReadyFrame.Wait( 1000 ) )
	{
		return	NULL ;
	}
	m_signalReadyFrame.ResetSignal() ;
	//
	m_pRenderer->AttachStereoTargetImage
		( &m_imgRenderColor[eyeRight], &m_imgRenderColor[eyeLeft],
			&m_imgRenderDepth[eyeRight], &m_imgRenderDepth[eyeLeft] ) ;
	//
	S3DVector	vScreen ;
	vScreen.x = (float32_t) m_fovEyes[0].sizeOfView.w * 0.5f ;
	vScreen.y = (float32_t) m_fovEyes[0].sizeOfView.h * 0.5f ;
	vScreen.z = (vScreen.x + vScreen.y) * 0.5f ;
	m_pRenderer->SetProjectionScreen( vScreen ) ;
	//
	return	m_pRenderer ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::EndDrawView
	( SGLAbstractWindow * pPrimaryWnd, RenderContext * render )
{
	ESLAssert( m_status >= statusInitialized ) ;

	m_pRenderer->Finish() ;
	m_pRenderer->DetachTargetImage() ;
	//
	SGLOpenGLTextureBuffer::GLResource *
		pglRsrcRight =
			SGLOpenGLTextureBuffer::GetResourceAs
					( m_pglResolve[eyeRight], m_pOpenGL ) ;
	SGLOpenGLTextureBuffer::GLResource *
		pglRsrcLeft =
			SGLOpenGLTextureBuffer::GetResourceAs
					( m_pglResolve[eyeLeft], m_pOpenGL ) ;
	//
	vr::Texture_t leftEyeTexture =
	{
		(void*)(uintptr_t) (pglRsrcLeft ? pglRsrcLeft->m_glTexture : 0),
		vr::TextureType_OpenGL,
		vr::ColorSpace_Gamma
	} ;
	vr::VRCompositor()->Submit( vr::Eye_Left, &leftEyeTexture ) ;
	//
	vr::Texture_t rightEyeTexture =
	{
		(void*)(uintptr_t) (pglRsrcRight ? pglRsrcRight->m_glTexture : 0),
		vr::TextureType_OpenGL,
		vr::ColorSpace_Gamma
	} ;
	vr::VRCompositor()->Submit( vr::Eye_Right, &rightEyeTexture ) ;
	m_signalDoneFrame.SetSignal() ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::FlipView
	( SGLAbstractWindow * pPrimaryWnd, bool fVSync )
{
	ESLAssert( m_status >= statusInitialized ) ;
	if ( m_status < statusInitialized )
	{
		return ;
	}
	if ( m_flagDrawToPrimary )
	{
		SGLSize			sizeFrameBuffer =
								m_imgResolveColor[0].GetImageSize() ;
		SGLImageRect	rectRender, rectDisplay ;
		if ( !pPrimaryWnd->GetInternalDisplayPosition
								( rectRender, rectDisplay ) )
		{
			m_fboResolve[0].BlitFramebufferTo
				( 0, 0, sizeFrameBuffer.h,
					sizeFrameBuffer.w, 0,
					rectRender.x, rectRender.y,
					rectRender.x + rectRender.w,
					rectRender.y + rectRender.h ) ;
			glFlush() ;
		}
	}
//	glFinish() ;
}

// 表示状態か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOpenVRProducer::IsVisibleView( void ) const
{
	return	true ;
}

// デバイス状態更新タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::WaitForPollDeviceState( int64_t msecTimeout )
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
SGLError SGLOpenVRProducer::ResetCurrentHMDPosture( void )
{
	if ( (m_status < statusInitialized) || (m_pHMD == NULL) )
	{
		return	sglErrFailed ;
	}
	m_pHMD->ResetSeatedZeroPose() ;
	return	sglErrSuccess ;
}

// 現在の HMD 座標を基準座標にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::ResetCurrentHMDPosition( void )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	LockForDeviceState() ;
	GetPostureFromTrackedDevicePose
		( m_postureHead, m_tdpDevPose[m_tdiHMD] ) ;
	m_vPositionBase = m_postureHead.vPosition / - GetScaleHMDToModel() ;
	m_postureHead.vPosition = S3DVector( 0, 0, 0 ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// HMD 空間タイプ取得
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::ViewSpaceType SGLOpenVRProducer::GetViewSpaceType( void ) const
{
	return	(vr::VRCompositor()->GetTrackingSpace() == vr::TrackingUniverseSeated)
					? spaceEyeLevel : spaceFloorLevel ;
}

// デバイスモデル取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::GetControllerModel
	( SGLVRViewProducer::ModelInfo& mi,
			SGLVRViewProducer::ControllerIndex iCtrl )
{
	vr::TrackedDeviceIndex_t	tdiCtrl ;
	HandIndex	iHand = handInvalid ;
	switch ( iCtrl )
	{
	case	controllerRight:
		tdiCtrl = m_tdiRightHand ;
		iHand = handRight ;
		break ;
	case	controllerLeft:
		tdiCtrl = m_tdiLeftHand ;
		iHand = handLeft ;
		break ;
	default:
		return	sglErrFailed ;
	}
	DeviceModel *	pdmModel = NULL ;
	m_csModel.Lock() ;
	pdmModel = GetDeviceModel( tdiCtrl ) ;
	if ( pdmModel == NULL )
	{
		pdmModel = LoadDeviceModel( tdiCtrl ) ;
	}
	if ( pdmModel == NULL )
	{
		m_csModel.Unlock() ;
		return	sglErrFailed ;
	}
	mi.pVBO = NULL ;
	mi.nFlags = 0 ;
	mi.postureModel.nFlags = 0 ;
	mi.postureTipLocal.nFlags = 0 ;
	switch ( pdmModel->m_status )
	{
	case	DeviceModel::statusError:
		mi.status = statusModelError ;
		break ;
	case	DeviceModel::statusCompleted:
		mi.status = statusModelCompleted ;
		mi.pVBO = pdmModel->m_pModel ;
		if ( m_flagShowControllerModel[iCtrl] )
		{
			mi.nFlags |= modelVisible ;
		}
		mi.postureModel = GetHandPosture( iHand ) ;
		//
		if ( pdmModel->m_flagState )
		{
			GetPostureFromHmdMatrix34
				( mi.postureTipLocal,
					pdmModel->m_stateComponentTip.mTrackingToComponentLocal ) ;
		}
		break ;
	default:
		mi.status = statusModelLoading ;
		break ;
	}
	m_csModel.Unlock() ;
	return	sglErrSuccess ;
}

// HMD スケール
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::SetScaleHMDToModel( double fpScale )
{
	SGLVRViewProducer::SetScaleHMDToModel( fpScale ) ;
	//
	if ( m_status == statusInitialized )
	{
		LockForDeviceState() ;
		ConvertHeadEyeHandPosture() ;
		UnlockForDeviceState() ;
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::Run( void )
{
	while ( !m_flagQuitThread )
	{
		//
		// デバイス座標取得
		//
		vr::VRCompositor()->WaitGetPoses
			( m_tdpDevPose, vr::k_unMaxTrackedDeviceCount, NULL, 0 ) ;
		//
		LockForDeviceState() ;
		//
		if ( m_tdiRightHand == vr::k_unTrackedDeviceIndexInvalid )
		{
			m_tdiRightHand =
				m_pHMD->GetTrackedDeviceIndexForControllerRole
								( vr::TrackedControllerRole_RightHand ) ;
			if ( m_tdiRightHand != vr::k_unTrackedDeviceIndexInvalid )
			{
				LogTrace( L"OpenVR right hand device : #%d\n", m_tdiRightHand ) ;
				PrepareControllerAxisIndexTable
					( &m_mapAxisIndex[controllerRight][0], m_tdiRightHand ) ;
			}
		}
		if ( m_tdiLeftHand == vr::k_unTrackedDeviceIndexInvalid )
		{
			m_tdiLeftHand =
				m_pHMD->GetTrackedDeviceIndexForControllerRole
								( vr::TrackedControllerRole_LeftHand ) ;
			if ( m_tdiLeftHand != vr::k_unTrackedDeviceIndexInvalid )
			{
				LogTrace( L"OpenVR left hand device : #%d\n", m_tdiLeftHand ) ;
				PrepareControllerAxisIndexTable
					( &m_mapAxisIndex[controllerLeft][0], m_tdiLeftHand ) ;
			}
		}
		//
		m_mat34EyeToHead[vr::Eye_Right] =
			m_pHMD->GetEyeToHeadTransform( vr::Eye_Right ) ;
		m_mat34EyeToHead[vr::Eye_Left] =
			m_pHMD->GetEyeToHeadTransform( vr::Eye_Left ) ;
		//
		ConvertHeadEyeHandPosture() ;
		//
		UnlockForDeviceState() ;
		//
		// コントローラーの状態取得
		//
		for( vr::TrackedDeviceIndex_t iDev = 0;
					iDev < vr::k_unMaxTrackedDeviceCount; iDev ++ )
		{
			if( m_pHMD->GetControllerState
				( iDev, &m_statController[iDev],
							sizeof(vr::VRControllerState_t) ) )
			{
				if ( !m_flagControllerState[iDev] )
				{
					LogTrace( L"OpenVR controller #%d state is valid.\n", iDev ) ;
				}
				m_flagControllerState[iDev] = true ;
			}
			else
			{
				m_flagControllerState[iDev] = false ;
			}
		}
		LockForDeviceState() ;
		if ( m_flagControllerState[m_tdiRightHand] )
		{
			ConvertControllerState
				( m_stateControllers[controllerRight],
					controllerRight, m_statController[m_tdiRightHand] ) ;
		}
		else
		{
			eslFillMemory
				( &m_stateControllers[controllerRight],
					0, sizeof(m_stateControllers[controllerRight]) ) ;
		}
		if ( m_flagControllerState[m_tdiLeftHand] )
		{
			ConvertControllerState
				( m_stateControllers[controllerLeft],
					controllerLeft, m_statController[m_tdiLeftHand] ) ;
		}
		else
		{
			eslFillMemory
				( &m_stateControllers[controllerLeft],
					0, sizeof(m_stateControllers[controllerLeft]) ) ;
		}
		m_signalReadyInput.SetSignal() ;
		UnlockForDeviceState() ;
		//
		// フレーム描画タイミング
		//
		m_signalReadyFrame.SetSignal() ;
		//
		do
		{
			//
			// SteamVR イベント処理
			//
			vr::VREvent_t vrEvent;
			while( m_pHMD->PollNextEvent( &vrEvent, sizeof(vrEvent) ) )
			{
				ProcessVREvent( vrEvent ) ;
			}
			//
			// 読み込み中のモデル処理
			//
			ProcessPendingModels() ;
			//
			// 読み込み済みモデルの状態取得
			//
			PollModelComponentState() ;
			//
			// フレーム描画完了待ち
			//
			if ( m_flagQuitThread )
			{
				break ;
			}
		}
		while ( m_signalDoneFrame.Wait( 10 ) == errTimeout ) ;
		m_signalDoneFrame.ResetSignal() ;
	}
}

// 更新タイミング待ち
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOpenVRProducer::WaitForView( int64_t msecTimeout )
{
	return	(SGLError) m_signalReadyFrame.Wait( msecTimeout ) ;
}

// m_tdpDevPose, m_mat34EyeToHead から Head, Eye, Hand の座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::ConvertHeadEyeHandPosture( void )
{
	ESLAssert( m_csDevState.TestLocked() ) ;
	//
	GetPostureFromHmdMatrix34
		( m_postureEyes[eyeRight],
			m_mat34EyeToHead[vr::Eye_Right] ) ;
	GetPostureFromHmdMatrix34
		( m_postureEyes[eyeLeft],
			m_mat34EyeToHead[vr::Eye_Left] ) ;
	//
	S3DVector	vBasePos = m_vPositionBase * GetScaleHMDToModel() ;
	if ( m_tdiHMD != vr::k_unTrackedDeviceIndexInvalid )
	{
		GetPostureFromTrackedDevicePose
			( m_postureHead, m_tdpDevPose[m_tdiHMD] ) ;
		m_postureHead.vPosition += vBasePos ;
	}
	else
	{
		m_postureHead.nFlags = 0 ;
	}
	//
	if ( m_tdiRightHand != vr::k_unTrackedDeviceIndexInvalid )
	{
		GetPostureFromTrackedDevicePose
			( m_postureHands[handRight], m_tdpDevPose[m_tdiRightHand] ) ;
		m_postureHands[handRight].vPosition += vBasePos ;
	}
	else
	{
		m_postureHands[handRight].nFlags = 0 ;
	}
	//
	if ( m_tdiLeftHand != vr::k_unTrackedDeviceIndexInvalid )
	{
		GetPostureFromTrackedDevicePose
			( m_postureHands[handLeft], m_tdpDevPose[m_tdiLeftHand] ) ;
		m_postureHands[handLeft].vPosition += vBasePos ;
	}
	else
	{
		m_postureHands[handLeft].nFlags = 0 ;
	}
}

// HmdMatrix34_t から Posture へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::GetPostureFromHmdMatrix34
	( SGLVRViewProducer::Posture& postureDst,
				const vr::HmdMatrix34_t& matSrc ) const
{
	const float32_t	fpScale = (float32_t) GetScaleHMDToModel() ;
	//
	S3DMatrix	matPose ;
	for ( int i = 0; i < 3; i ++ )
	{
		matPose.m[i][0] = matSrc.m[i][0] ;
		matPose.m[i][1] = matSrc.m[i][1] ;
		matPose.m[i][2] = matSrc.m[i][2] ;
	}
	S3DMatrix	matSpace( 1, -1, -1 ) ;
	S3DMatrix	matISpace( 1, -1, -1 ) ;
	postureDst.matOrientation = matSpace * matPose * matISpace ;
	postureDst.vPosition.x =   matSrc.m[0][3] * fpScale ;
	postureDst.vPosition.y = - matSrc.m[1][3] * fpScale ;
	postureDst.vPosition.z = - matSrc.m[2][3] * fpScale	;
	postureDst.nFlags = trackedOrientation | trackedPosition ;
}

// TrackedDevicePose_t から Posture へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::GetPostureFromTrackedDevicePose
	( SGLVRViewProducer::Posture& postureDst,
		const vr::TrackedDevicePose_t& poseSrc ) const
{
	if ( poseSrc.bPoseIsValid )
	{
		GetPostureFromHmdMatrix34
			( postureDst, poseSrc.mDeviceToAbsoluteTracking ) ;
	}
	else
	{
		postureDst.nFlags = 0 ;
	}
}

// VRControllerState_t から ControllerState へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::ConvertControllerState
	( SGLVRViewProducer::ControllerState& stateDst,
			SGLVRViewProducer::ControllerIndex iController,
			const vr::VRControllerState_t& stateSrc ) const
{
	uint32_t	nLastState = stateDst.nState ;
	stateDst.nState = 0 ;
	stateDst.nCapacity = 0 ;
	//
	const wchar_t *				pszCtrlName = NULL ;
	vr::TrackedDeviceIndex_t	tdiDev = vr::k_unTrackedDeviceIndexInvalid ;
	if ( iController == controllerRight )
	{
		pszCtrlName = L"right" ;
		tdiDev = m_tdiRightHand ;
	}
	else if ( iController == controllerLeft )
	{
		pszCtrlName = L"left" ;
		tdiDev = m_tdiLeftHand ;
	}
	if ( tdiDev != vr::k_unTrackedDeviceIndexInvalid )
	{
		if ( m_tdpDevPose[tdiDev].bDeviceIsConnected )
		{
			if ( !(nLastState & controllerConnected) )
			{
				LogTrace( L"OpenVR %s controller is connected.\n", pszCtrlName ) ;
			}
			stateDst.nState |= controllerConnected ;
		}
		else
		{
			if ( nLastState & controllerConnected )
			{
				LogTrace( L"OpenVR %s controller is unconnected.\n", pszCtrlName ) ;
			}
		}
	}
	else
	{
		stateDst.nState |= controllerConnected ;
	}
	//
	// ボタンマスク変換
	//
	uint64_t	bitButton = 1 ;
	uint64_t	maskPressed = 0 ;
	uint64_t	maskTouched = 0 ;
	for ( int i = 0; i < 64; i ++, bitButton <<= 1 )
	{
		if ( m_iVRBurronTransTable[i] >= 0 )
		{
			uint64_t	bitTest =
				vr::ButtonMaskFromId
					( (vr::EVRButtonId) m_iVRBurronTransTable[i] ) ;
			if ( stateSrc.ulButtonPressed & bitTest )
			{
				maskPressed |= bitButton ;
			}
			if ( stateSrc.ulButtonTouched & bitTest )
			{
				maskTouched |= bitButton ;
			}
		}
	}
	if ( stateDst.maskButtonPressed != maskPressed )
	{
		LogTrace( L"OpenVR %s button pressed: %08X:%08X\n",
				pszCtrlName, (DWORD)(maskPressed >> 32), (DWORD) maskPressed ) ;
	}
	if ( stateDst.maskButtonTouched != maskTouched )
	{
		LogTrace( L"OpenVR %s button touched: %08X:%08X\n",
				pszCtrlName, (DWORD)(maskTouched >> 32), (DWORD) maskTouched ) ;
	}
	stateDst.maskButtonPressed = maskPressed ;
	stateDst.maskButtonTouched = maskTouched ;
	//
	// アナログスティック・トリガー変換
	//
	for ( int i = 0; i < 5; i ++ )
	{
		AxisIndex	axisIndex = m_mapAxisIndex[iController][i] ;
		if ( axisIndex != axisInvalid )
		{
			stateDst.nCapacity |= capAxisThumbStick << axisIndex ;
			stateDst.vAxis[axisIndex].x = stateSrc.rAxis[i].x ;
			stateDst.vAxis[axisIndex].y = - stateSrc.rAxis[i].y ;
		}
	}
}

// SteamVR イベント処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::ProcessVREvent( const vr::VREvent_t& vrEvent )
{
//	LogTrace( L"ProcessVREvent %d\n", vrEvent.eventType ) ;
	ControllerIndex				iCtrl ;
	switch( vrEvent.eventType )
	{
	case vr::VREvent_TrackedDeviceActivated:
		LogTrace( L"VREvent_TrackedDeviceActivated %d\n", vrEvent.trackedDeviceIndex ) ;
		LoadDeviceModel( vrEvent.trackedDeviceIndex ) ;
		break ;

	case vr::VREvent_TrackedDeviceDeactivated:
		break ;

	case vr::VREvent_TrackedDeviceUpdated:
		break ;

	case	vr::VREvent_HideRenderModels:
		iCtrl = ControllerIndexFromDeviceIndex( vrEvent.trackedDeviceIndex ) ;
		if ( iCtrl != controllerInvalid )
		{
			LogTrace( L"VREvent_HideRenderModels controller #%d\n", iCtrl ) ;
			m_flagShowControllerModel[iCtrl] = false ;
		}
		break ;

	case	vr::VREvent_ShowRenderModels:
		iCtrl = ControllerIndexFromDeviceIndex( vrEvent.trackedDeviceIndex ) ;
		if ( iCtrl != controllerInvalid )
		{
			LogTrace( L"VREvent_ShowRenderModels controller #%d\n", iCtrl ) ;
			m_flagShowControllerModel[iCtrl] = true ;
		}
		break ;
	}
}

// TrackedDeviceIndex_t から ControllerIndex へ変換
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::ControllerIndex
		SGLOpenVRProducer::ControllerIndexFromDeviceIndex
							( vr::TrackedDeviceIndex_t tdiDev ) const
{
	if ( m_tdiRightHand == tdiDev )
	{
		return	controllerRight ;
	}
	else if ( m_tdiLeftHand == tdiDev )
	{
		return	controllerLeft ;
	}
	return	controllerInvalid ;
}

// デバイス名取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString SGLOpenVRProducer::GetDeviceModelNameString
						( vr::TrackedDeviceIndex_t tdiDevice ) const
{
	vr::TrackedPropertyError	errProp ;
	uint32_t unReqBufferLen =
		m_pHMD->GetStringTrackedDeviceProperty
			( tdiDevice, vr::Prop_RenderModelName_String, NULL, 0, &errProp ) ;
	if( unReqBufferLen == 0 )
	{
		return	SString() ;
	}
	SArray<char>	bufName ;
	unReqBufferLen =
		m_pHMD->GetStringTrackedDeviceProperty
			( tdiDevice, vr::Prop_RenderModelName_String,
				bufName.GetArray( unReqBufferLen ), unReqBufferLen, &errProp ) ;
	bufName.FinishArray() ;
	return	SString( bufName.GetConstArray() ) ;
}

// デバイスモデル取得
//////////////////////////////////////////////////////////////////////////////
SGLOpenVRProducer::DeviceModel *
	SGLOpenVRProducer::GetDeviceModel
		( vr::TrackedDeviceIndex_t tdiDevice ) const
{
	DeviceModel *	pDevModel = NULL ;
	m_csModel.Lock() ;
	pDevModel = m_ssoaModels.GetAs( GetDeviceModelNameString( tdiDevice ) ) ;
	m_csModel.Unlock() ;
	return	pDevModel ;
}

// デバイスモデル取得／読み込み開始
//////////////////////////////////////////////////////////////////////////////
SGLOpenVRProducer::DeviceModel *
	SGLOpenVRProducer::LoadDeviceModel
		( vr::TrackedDeviceIndex_t tdiDevice )
{
	DeviceModel *	pDevModel = NULL ;
	SString	strModelName = GetDeviceModelNameString( tdiDevice ) ;
	m_csModel.Lock() ;
	pDevModel = m_ssoaModels.GetAs( strModelName ) ;
	if ( (pDevModel == NULL) && !strModelName.IsEmpty() )
	{
		pDevModel = new DeviceModel ;
		pDevModel->m_tdiDevice = tdiDevice ;
		m_ssoaModels.SetAs( strModelName, pDevModel ) ;
		//
		pDevModel->m_status = DeviceModel::statusLoadingModel ;
		strModelName.EncodeDefaultTo( pDevModel->m_bufModelName ) ;
		//
		m_aPendingModels.Add( pDevModel ) ;
	}
	m_csModel.Unlock() ;
	return	pDevModel ;
}

// 読み込み中のモデル処理
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::ProcessPendingModels( void )
{
	m_csModel.Lock() ;
	for ( size_t i = 0; i < m_aPendingModels.GetLength(); i ++ )
	{
		DeviceModel *	pDevModel = m_aPendingModels.GetAt( i ) ;
		if ( pDevModel == NULL )
		{
			continue ;
		}
		if ( pDevModel->m_status == DeviceModel::statusLoadingModel )
		{
			vr::RenderModel_t *		pvrModel = NULL ;
			vr::EVRRenderModelError vrError ;
			vrError = vr::VRRenderModels()->LoadRenderModel_Async
						( pDevModel->m_bufModelName.GetConstArray(), &pvrModel ) ;
			if ( vrError == vr::VRRenderModelError_None )
			{
				pDevModel->m_status = DeviceModel::statusLoadingTexture ;
				pDevModel->m_pvrSrcModel = pvrModel ;
			}
			else if ( vrError != vr::VRRenderModelError_Loading )
			{
				pDevModel->m_status = DeviceModel::statusError ;
				m_aPendingModels.SetAt( i, NULL ) ;
			}
		}
		else if ( pDevModel->m_status == DeviceModel::statusLoadingTexture )
		{
			vr::RenderModel_TextureMap_t *	pTexture = NULL ;
			vr::EVRRenderModelError			vrError ;
			vrError =
				vr::VRRenderModels()->LoadTexture_Async
					( pDevModel->m_pvrSrcModel->diffuseTextureId, &pTexture ) ;
			if ( vrError == vr::VRRenderModelError_None )
			{
				pDevModel->m_status = DeviceModel::statusLoaded ;
				pDevModel->m_pvrSrcTexture = pTexture ;
				//
				BuildDeviceModel( *pDevModel ) ;
			}
			else if ( vrError != vr::VRRenderModelError_Loading )
			{
				pDevModel->m_status = DeviceModel::statusError ;
				m_aPendingModels.SetAt( i, NULL ) ;
			}
		}
		else
		{
			m_aPendingModels.SetAt( i, NULL ) ;
		}
	}
	m_aPendingModels.TrimEmpty() ;
	m_csModel.Unlock() ;
}

// 読み込み済みのモデル状態取得
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::PollModelComponentState( void )
{
	m_csModel.Lock() ;
	for ( size_t i = 0; i < m_ssoaModels.GetLength(); i ++ )
	{
		DeviceModel *	pDevModel = m_ssoaModels.GetAt( i ) ;
		if ( (pDevModel == nullptr)
			|| (pDevModel->m_status != DeviceModel::statusCompleted) )
		{
			continue ;
		}
		pDevModel->m_flagState =
			vr::VRRenderModels()->GetComponentState
				( pDevModel->m_bufModelName.GetConstArray(),
					vr::k_pch_Controller_Component_Tip,
					&(pDevModel->m_stateController),
					&(pDevModel->m_stateCtrlMode),
					&(pDevModel->m_stateComponentTip) ) ;
	}
	m_csModel.Unlock() ;
}

// モデル構築
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::BuildDeviceModel( SGLOpenVRProducer::DeviceModel& model )
{
	//
	// マテリアル
	//
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodGouraud
						| shadingTextureSmoothing
						| shadingTextureMapping
						| shadingHintOfFullAlpha ;
	attr.colorBase.rgbMul = 0x00FFFFFF ;
	attr.colorBase.rgbAdd = 0 ;
	attr.nDiffusion = 0x100 ;
	attr.nAmbient = 0 ;
	attr.nSpecular = 0x80 ;
	attr.nSpecularSize = 0x10 ;
	//
	model.m_material.SetSurfaceAttribute( attr ) ;
	//
	// テクスチャ
	//
	model.m_imgTexture.CreateImage
		( model.m_pvrSrcTexture->unWidth,
			model.m_pvrSrcTexture->unHeight,
			formatImageABGR, 32 ) ;
	//
	SGLImageInfo	imginf ;
	uint8_t *		pbytDstBuf =
						model.m_imgTexture.LockBuffer
							( imginf, SGLImageObject::lockWrite ) ;
	const uint8_t *	pbytSrcBuf = model.m_pvrSrcTexture->rubTextureMapData ;
	size_t			pitchSrcLine = imginf.width * 4 ;
	for ( int y = 0; y < (int) imginf.height; y ++ )
	{
		eslCopyMemory
			( pbytDstBuf + imginf.pitchLine * y,
				pbytSrcBuf + pitchSrcLine * y, pitchSrcLine ) ;
	}
	model.m_imgTexture.UnlockBuffer( SGLImageObject::lockWrite ) ;
	//
	model.m_material.SetTexture( &model.m_imgTexture ) ;
	//
	// モデル
	//
	S3DVertexBuffer *	pVBO = new S3DVertexBuffer ;
	SArray<S3DVector4>	bufVertex ;
	SArray<S3DVector4>	bufNormal ;
	SArray<S2DVector>	bufUV ;
	const size_t		countVertex =
							(size_t) model.m_pvrSrcModel->unVertexCount ;
	S3DVector4 *		pvVertex = bufVertex.GetArray( countVertex ) ;
	S3DVector4 *		pvNormal = bufNormal.GetArray( countVertex ) ;
	S2DVector *			pvUV = bufUV.GetArray( countVertex ) ;
	const vr::RenderModel_Vertex_t *
						pvSrcModelVertex = model.m_pvrSrcModel->rVertexData ;
	for ( size_t i = 0; i < countVertex; i ++ )
	{
		pvVertex[i].x =   pvSrcModelVertex->vPosition.v[0] ;
		pvVertex[i].y = - pvSrcModelVertex->vPosition.v[1] ;
		pvVertex[i].z = - pvSrcModelVertex->vPosition.v[2] ;
		pvNormal[i].x =   pvSrcModelVertex->vNormal.v[0] ;
		pvNormal[i].y = - pvSrcModelVertex->vNormal.v[1] ;
		pvNormal[i].z = - pvSrcModelVertex->vNormal.v[2] ;
		pvUV[i].x = pvSrcModelVertex->rfTextureCoord[0]
									* (float32_t) imginf.width ;
		pvUV[i].y = pvSrcModelVertex->rfTextureCoord[1]
									* (float32_t) imginf.height ;
		pvSrcModelVertex ++ ;
	}
	bufVertex.FinishArray() ;
	bufNormal.FinishArray() ;
	bufUV.FinishArray() ;
	//
	// 指標
	//
	SArray<uint32_t>	bufIndex ;
	const uint32_t		countPolygon =
							(size_t) model.m_pvrSrcModel->unTriangleCount ;
	uint32_t *			pDstIndex = bufIndex.GetArray( countPolygon * 3 ) ;
	const uint16_t *	pSrcIndex = model.m_pvrSrcModel->rIndexData ;
	for ( size_t i = 0; i < countPolygon * 3; i ++ )
	{
		pDstIndex[i] = pSrcIndex[i] ;
	}
	bufIndex.FinishArray() ;
	//
	// 完成
	//
	pVBO->AddIndexedTriangleList
		( &(model.m_material), 0, countPolygon, countVertex,
					pvVertex, pvNormal, pvUV, NULL, pDstIndex ) ;
	//
	model.m_pModel = pVBO ;
	model.m_status = DeviceModel::statusCompleted ;
	//
	// 解放
	//
	vr::VRRenderModels()->FreeRenderModel( model.m_pvrSrcModel ) ;
	vr::VRRenderModels()->FreeTexture( model.m_pvrSrcTexture ) ;
	model.m_pvrSrcModel = NULL ;
	model.m_pvrSrcTexture = NULL ;
}

// ログ出力するファイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::SetLogFile( SSystem::SBufferedFile * pfile )
{
	m_pLogFile = pfile ;
}

// デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void SGLOpenVRProducer::LogTrace( const wchar_t * pwszFormat, ... ) const
{
	if ( m_pLogFile != nullptr )
	{
		va_list	vl ;
		va_start( vl, pwszFormat ) ;
		m_pLogFile->WriteFormatV( pwszFormat, vl ) ;
	}

#if	defined(__DEBUG__)
	ESLTrace( "%s", SString(pwszFormat).ToCharArray().GetConstArray() ) ;
#endif
}

