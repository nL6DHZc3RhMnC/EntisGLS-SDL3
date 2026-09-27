
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_vr_oculus_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Oculus Rift 出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// ライブラリ初期化カウンタ
atomic_int_t	SGLOculusVRProducer::m_countLibInit = 0 ;

// ボタン指標変換 [enum ButtonIndex] -> ovrButton
const int	SGLOculusVRProducer::m_iVRBurronTransTable[64] =
{
	ovrButton_Up,
	ovrButton_Down,
	ovrButton_Left,
	ovrButton_Right,
	ovrButton_A, ovrButton_B, ovrButton_RThumb, ovrButton_RShoulder,
	ovrButton_X, ovrButton_Y, ovrButton_LThumb, ovrButton_LShoulder,
	ovrButton_VolUp, ovrButton_VolDown, -1, -1,
	//
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	//
	ovrButton_Back, ovrButton_Enter, ovrButton_Home, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	//
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
} ;

// ディスプレイロストコマンド
const wchar_t *const
	SGLOculusVRProducer::ID_OCULUS_DISPLAY_LOST = L"ID_OCULUS_DISPLAY_LOST" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLOculusVRProducer,
		SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLOculusVRProducer::SGLOculusVRProducer( SGLOpenGLContext * pOpenGL )
	: m_pOpenGL( pOpenGL ),
		m_viewSpaceType( spaceEyeLevel ), m_vPositionBase( 0, 0, 0 )
{
	m_status = statusUnintialized ;
	m_flagDisplayLost = false ;
	m_flagPostDisplayLost = false ;
	m_flagVisibleHMD = true ;
	m_flagVisibleFrame = true ;
	m_session = nullptr ;
	m_indexFrame = 0 ;
	m_pRenderer = new S3DOpenGLBufferedRenderer( pOpenGL ) ;
	m_flagBeganThread = false ;
	m_flagQuitThread = false ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::Initialize
	( const SGLSize& sizeWindow, ViewSpaceType spaceType )
{
	if ( m_status != statusUnintialized )
	{
		return	sglErrFailed ;
	}
	if ( InitializeLib() )
	{
		m_status = statusFailedInitialize ;
		return	sglErrFailed ;
	}
	m_status = statusLibInit ;
	//
	if ( CreateSession( sizeWindow, spaceType ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::Release( void )
{
	if ( m_status >= statusInitialized )
	{
		ReleaseSession() ;
	}
	if ( m_status >= statusLibInit )
	{
		ShutdownLib() ;
	}
	m_status = statusUnintialized ;
	return	sglErrSuccess ;
}

// セッション再生成（デバイスロスト時）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::RecreateSession( void )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	SGLSize	sizeWindow = m_sizeWindow ;
	ReleaseSession() ;
	return	CreateSession( sizeWindow, m_viewSpaceType ) ;
}

// HMD 情報
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLOculusVRProducer::GetDisplaySizeInEye( void ) const
{
	SGLSize	sizeDisplay( 0, 0 ) ;
	if ( m_status == statusInitialized )
	{
		SGLImageObject *	pimgLeft = m_aRenderColor[ovrEye_Left].GetAt( 0 ) ;
		SGLImageObject *	pimgRight = m_aRenderColor[ovrEye_Right].GetAt( 0 ) ;
		if ( pimgLeft && pimgRight )
		{
			SGLSize	sizeLeft = pimgLeft->GetImageSize() ;
			SGLSize	sizeRight = pimgRight->GetImageSize() ;
			if ( sizeLeft != sizeRight )
			{
				LogTrace( L"not matched MHD size of each eye.\n" ) ;
				sizeDisplay.w = (sizeLeft.w + sizeRight.w) / 2 ;
				sizeDisplay.h = (sizeLeft.h + sizeRight.h) / 2 ;
			}
			else
			{
				sizeDisplay = sizeRight ;
			}
		}
	}
	return	sizeDisplay ;
}

// 描画タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::WaitFrameSync( int64_t msecTimeout )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	if ( !m_flagBeganThread )
	{
		ovrResult	result = ovr_WaitToBeginFrame( m_session, m_indexFrame ) ;
		if ( OVR_SUCCESS( result ) )
		{
			PollSessionStatus() ;
			PollEyePosture() ;
			PollInputState() ;
			return	sglErrSuccess ;
		}
		else
		{
			m_flagDisplayLost = true ;
			return	sglErrFailed ;
		}
	}
	SGLError	err = (SGLError) m_signalReadyFrame.Wait( msecTimeout ) ;
	return	err ;
}

// LibOVR 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::InitializeLib( void )
{
	if ( AtomicAdd( &m_countLibInit, 1 ) == 1 )
	{
		if ( OVR_FAILURE( ovr_Initialize( nullptr ) ) )
		{
			AtomicSub( &m_countLibInit, 1 ) ;
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// LibOVR 終了
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::ShutdownLib( void )
{
	ESLAssert( m_countLibInit >= 0 ) ;
	if ( AtomicSub( &m_countLibInit, 1 ) == 0 )
	{
		ovr_Shutdown() ;
	}
}

// LibOVR 初期化済みか？
//////////////////////////////////////////////////////////////////////////////
bool SGLOculusVRProducer::IsInitializedLib( void )
{
	return	(m_countLibInit >= 1) ;
}

// セッション生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::CreateSession
	( const SGLSize& sizeWindow, ViewSpaceType spaceType )
{
	//
	// セッション生成
	//
	m_signalRSDone.Initialize( true ) ;
	SGLError	err = CreateSessionWithouThread( sizeWindow, spaceType ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 同期用スレッド生成
	//
	ESLAssert( !m_flagBeganThread ) ;
	m_flagQuitThread = false ;
	m_signalReadyFrame.Initialize( false ) ;
	m_signalEndFrame.Initialize( false ) ;
	m_signalReadyInput.Initialize( false ) ;
	if ( m_threadSync.BeginThread( this ) )
	{
		LogTrace( L"failed to begin thread for sync.\n" ) ;
	}
	else
	{
		LogTrace( L"\nBegin Oculus Rift tracking thread.\n" ) ;
		m_flagBeganThread = true ;
	}
	return	sglErrSuccess ;
}

SGLError SGLOculusVRProducer::CreateSessionWithouThread
	( const SGLSize& sizeWindow, ViewSpaceType spaceType )
{
	ESLAssert( m_status == statusLibInit ) ;
	if (OVR_FAILURE( ovr_Create( &m_session, &m_luid ) ) )
	{
		return	sglErrFailed ;
	}
	//
	// HMD 情報取得
	//
	m_hmdDesc = ovr_GetHmdDesc( m_session ) ;
	//
	// 描画用バッファ生成
	//
	const wchar_t *	pszEyeName[ovrEye_Count] ;
	EyeIndex		eyeIndex[ovrEye_Count] ;
	pszEyeName[ovrEye_Left] = L"left eye" ;
	pszEyeName[ovrEye_Right] = L"right eye" ;
	eyeIndex[ovrEye_Left] = eyeLeft ;
	eyeIndex[ovrEye_Right] = eyeRight ;
	//
	for ( int i = 0; i < ovrEye_Count; i ++ )
	{
		LogTrace( L"\nOculus Rift HMD %s;\n", pszEyeName[i] ) ;

		// テクスチャサイズ
		ovrSizei	sizeFovTex =
			ovr_GetFovTextureSize
				( m_session, (ovrEyeType) i,
					m_hmdDesc.DefaultEyeFov[i], 1.0f ) ;
		LogTrace( L"  texture size: %d x %d\n", sizeFovTex.w, sizeFovTex.h ) ;

		// レンダリング情報
		m_eyeDesc[i] =
			ovr_GetRenderDesc
				( m_session, (ovrEyeType) i,
					m_hmdDesc.DefaultEyeFov[i] ) ;
		//
		EyeIndex	eye = eyeIndex[i] ;
		m_postureEyes[eye].nFlags =
						trackedOrientation | trackedPosition ;
		ConvertPosture
			( m_postureEyes[eye], m_eyeDesc[i].HmdToEyePose ) ;
		LogTrace( L"  eye posture: %.5f %.5f %.5f %.5f\n"
				  L"               %.5f %.5f %.5f %.5f\n"
				  L"               %.5f %.5f %.5f %.5f\n",
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
		float32_t	wFovTex = (float32_t) sizeFovTex.w ;
		float32_t	hFovTex = (float32_t) sizeFovTex.h ;
		float32_t	wFOV = m_eyeDesc[i].Fov.RightTan
							+ m_eyeDesc[i].Fov.LeftTan ;
		float32_t	hFOV = m_eyeDesc[i].Fov.UpTan
							+ m_eyeDesc[i].Fov.DownTan ;
		float32_t	zScreen = (float32_t) sizeFovTex.h / hFOV ;
		//
		m_fovEyes[eye].sizeOfView.w = (int32_t) sizeFovTex.w ;
		m_fovEyes[eye].sizeOfView.h = (int32_t) sizeFovTex.h ;
		m_fovEyes[eye].vScreenPos.x =
					wFovTex * m_eyeDesc[i].Fov.LeftTan / wFOV ;
		m_fovEyes[eye].vScreenPos.y =
					hFovTex * m_eyeDesc[i].Fov.UpTan / hFOV ;
		m_fovEyes[eye].vScreenPos.z = zScreen ;
		m_fovEyes[eye].fpPixelAspect =
					(wFOV * hFovTex) / (hFOV * wFovTex) ;
		LogTrace( L"  projection: (%f, %f) / %f\n",
					m_fovEyes[eye].vScreenPos.x,
					m_fovEyes[eye].vScreenPos.y,
					m_fovEyes[eye].vScreenPos.z ) ;

		// 色テクスチャ生成
		ovrTextureSwapChainDesc	descTexColor ;
		descTexColor.Type = ovrTexture_2D ;
		descTexColor.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB ;// OVR_FORMAT_R8G8B8A8_UNORM ;
		descTexColor.ArraySize = 1 ;
		descTexColor.Width = sizeFovTex.w ;
		descTexColor.Height = sizeFovTex.h ;
		descTexColor.MipLevels = 1 ;
		descTexColor.SampleCount = 1 ;
		descTexColor.StaticImage = ovrFalse ;
		descTexColor.MiscFlags = ovrTextureMisc_None ;
		descTexColor.BindFlags = ovrTextureBind_None ;
		//
		ovrTextureSwapChain		texColor ;
		if ( OVR_SUCCESS
			( ovr_CreateTextureSwapChainGL
					( m_session, &descTexColor, &texColor ) ) )
		{
			m_fCreatedColorTexture[i] = true ;
			//
			AttachTextureSwapChain
				( m_aRenderColor[i], texColor,
					(uint32_t) sizeFovTex.w,
					(uint32_t) sizeFovTex.h, formatImageABGR, 32 ) ;
		}
		else
		{
			m_fCreatedColorTexture[i] = false ;
			LogTrace( L"failed to create Oculus color texture.\n" ) ;
		}

		// 深度テクスチャ生成
		m_imgRenderDepth[i].CreateImage
			( (uint32_t) sizeFovTex.w,
				(uint32_t) sizeFovTex.h,
				formatImageDepth, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferNonPowerOf2
					| SGLImageObject::bufferOnDeviceOnly ) ;

		// 転送情報設定
		m_layer.Header.Type = ovrLayerType_EyeFov ;
		m_layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft ;
		m_layer.EyeFov.ColorTexture[i] = texColor ;
		m_layer.EyeFov.Viewport[i].Pos.x = 0 ;
		m_layer.EyeFov.Viewport[i].Pos.y = 0 ;
		m_layer.EyeFov.Viewport[i].Size = sizeFovTex ;
		m_layer.EyeFov.Fov[i] = m_hmdDesc.DefaultEyeFov[i] ;
	}

	// ミラーテクスチャ生成
	ovrMirrorTextureDesc	descMirror ;
	descMirror.Format = OVR_FORMAT_R8G8B8A8_UNORM ;
	descMirror.Width = sizeWindow.w ;
	descMirror.Height = sizeWindow.h ;
	descMirror.MiscFlags = ovrTextureMisc_None ;
	descMirror.MirrorOptions = ovrMirrorOption_Default ;

	if ( OVR_FAILURE
		( ovr_CreateMirrorTextureWithOptionsGL
			( m_session, &descMirror, &m_mirrorTexture ) ) )
	{
		m_fCreatedMirrorTexture = false ;
		LogTrace( L"failed to create Oculus mirror texture.\n" ) ;
	}
	else
	{
		m_fCreatedMirrorTexture = true ;
		//
		// OpenGL テクスチャを SGLImage に関連付け
		//
		unsigned int	glMirrotTexID = 0 ;
		if ( OVR_FAILURE( ovr_GetMirrorTextureBufferGL
			( m_session, m_mirrorTexture, &glMirrotTexID ) ) )
		{
			LogTrace( L"failed to get mirror texture id.\n" ) ;
		}
		//
		m_imgMirror.CreateImage
			( (uint32_t) sizeWindow.w,
				(uint32_t) sizeWindow.h,
				formatImageABGR, 32,
				SGLImageObject::bufferNonPowerOf2
				| SGLImageObject::bufferOnDeviceOnly ) ;
		//
		SGLOpenGLTextureBuffer::AttachGLTexture
					( m_pOpenGL, &m_imgMirror, glMirrotTexID, false ) ;
		//
		SGLImageRect	rectRef ;
		m_pglMirror = SGLOpenGLTextureBuffer::CommitGLTexture
								( m_pOpenGL, &m_imgMirror, rectRef ) ;
		m_fboMirror.SetFrameBufferTarget( GL_FRAMEBUFFER ) ;
		m_fboMirror.AttachFrameBuffer( m_pglMirror, NULL ) ;
		m_fboMirror.SetFrameBufferTarget( GL_READ_FRAMEBUFFER ) ;
	}
	m_sizeWindow = sizeWindow ;
	m_flagDisplayLost = false ;
	m_flagPostDisplayLost = false ;

	//
	// 位置初期化
	//
	ovr_SetTrackingOriginType
		( m_session, (spaceType == spaceEyeLevel)
						? ovrTrackingOrigin_EyeLevel
						: ovrTrackingOrigin_FloorLevel ) ;
	m_viewSpaceType = spaceType ;
	m_vPositionBase = S3DDVector( 0, 0, 0 ) ;

	m_status = statusInitialized ;
	return	sglErrSuccess ;
}

// セッション解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::ReleaseSession( void )
{
	if ( m_status < statusInitialized )
	{
		ESLAssert( !m_flagBeganThread ) ;
		return	sglErrFailed ;
	}
	if ( m_signalRSDone.Wait(100) == errTimeout )
	{
		LogTrace( L"Oculus Rift: waiting finish recreate session.\n" ) ;
		m_signalRSDone.Wait() ;
	}
	//
	// 同期用スレッド終了
	//
	if ( m_flagBeganThread )
	{
		do
		{
			m_flagQuitThread = true ;
			m_signalEndFrame.SetSignal() ;
		}
		while ( m_threadSync.Wait( 10 ) == errTimeout ) ;
		//
		m_flagBeganThread = false ;
		m_flagQuitThread = false ;
		m_threadSync.Delete() ;
		m_signalReadyFrame.Delete() ;
		m_signalEndFrame.Delete() ;
	}
	//
	// セッション終了
	//
	ReleaseSessionKeepThread() ;
	//
	m_signalRSDone.Delete() ;
	//
	return	sglErrSuccess ;
}

SGLError SGLOculusVRProducer::ReleaseSessionKeepThread( void )
{
	if ( m_status < statusInitialized )
	{
		ESLAssert( !m_flagBeganThread ) ;
		return	sglErrFailed ;
	}
	//
	// ミラー表示用テクスチャ削除
	//
	if ( m_fCreatedMirrorTexture )
	{
		ovr_DestroyMirrorTexture( m_session, m_mirrorTexture ) ;
		m_fboMirror.ReleaseFrameBuffer() ;
		m_imgMirror.ReleaseBuffer() ;
		m_pglMirror = NULL ;
		m_fCreatedMirrorTexture = false ;
	}
	//
	// 描画用テクスチャ削除
	//
	for ( int i = 0; i < ovrEye_Count; i ++ )
	{
		if ( m_fCreatedColorTexture[i] )
		{
			ovr_DestroyTextureSwapChain
				( m_session, m_layer.EyeFov.ColorTexture[i] ) ;
			m_fCreatedColorTexture[i] = false ;
		}
		m_aRenderColor[i].RemoveAll() ;
		m_imgRenderDepth[i].ReleaseBuffer() ;
	}
	//
	// セッション終了
	//
	ovr_Destroy( m_session ) ;
	m_session = nullptr ;
	m_status = statusLibInit ;
	return	sglErrSuccess ;
}

// セッション再生成（デバイスロスト時スレッド内）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::RecreateSessionOnThread( void )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	SGLSize	sizeWindow = m_sizeWindow ;
	ReleaseSessionKeepThread() ;
	return	CreateSessionWithouThread( sizeWindow, m_viewSpaceType ) ;
}

// 表示用 ovrTextureSwapChain を SGLImageObject 配列に関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::AttachTextureSwapChain
	( SSystem::SObjectArray<SGLImageObject>& aSwapChain,
		ovrTextureSwapChain textureChain,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFormat, uint32_t nBitsPerPixel ) const
{
	int	nLength = 0 ;
	if ( OVR_SUCCESS
		( ovr_GetTextureSwapChainLength
			( m_session, textureChain, &nLength ) ) )
	{
		LogTrace( L"  swap chain length: %d\n", nLength ) ;
		aSwapChain.SetLength( (size_t) nLength ) ;
		//
		for ( int i = 0; i < nLength; i ++ )
		{
			unsigned int	glTexID ;
			if ( OVR_SUCCESS
				( ovr_GetTextureSwapChainBufferGL
					( m_session, textureChain, i, &glTexID ) ) )
			{
				SGLImage *	pImage = new SGLImage ;
				aSwapChain.SetAt( (size_t) i, pImage ) ;
				//
				pImage->CreateImage
					( nWidth, nHeight,
						nFormat, nBitsPerPixel,
						SGLImageObject::bufferNonPowerOf2
						| SGLImageObject::bufferOnDeviceOnly ) ;
				//
				SGLOpenGLTextureBuffer::AttachGLTexture
							( m_pOpenGL, pImage, glTexID, false ) ;
			}
			else
			{
				LogTrace( L"failed to ovr_GetTextureSwapChainBufferGL.\n" ) ;
			}
		}
	}
	else
	{
		LogTrace( L"failed to ovr_GetTextureSwapChainLength.\n" ) ;
	}
}

// ステータス取得／処理
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::PollSessionStatus( void )
{
	//
	// 状態取得
	//
	ovrSessionStatus	status ;
	ovr_GetSessionStatus( m_session, &status ) ;
	//
	if ( m_flagVisibleHMD != (status.IsVisible != 0) )
	{
		LogTrace( L"Oculus Rift HMD: visible = %d\n", status.IsVisible ) ;
	}
	if ( status.DisplayLost )
	{
		LogTrace( L"Oculus Rift: display lost\n" ) ;
	}
	//
	m_flagVisibleHMD = (status.IsVisible != 0) ;
	if ( status.ShouldRecenter )
	{
		LogTrace( L"ovr_RecenterTrackingOrigin by ovrSessionStatus.ShouldRecenter\n" ) ;
		ovr_RecenterTrackingOrigin( m_session ) ;
	}
}

// 姿勢取得
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::PollEyePosture( void )
{
	//
	// 目の姿勢更新
	//
	for ( int i = 0; i < ovrEye_Count; i ++ )
	{
		m_eyeDesc[i] =
			ovr_GetRenderDesc
				( m_session, (ovrEyeType) i,
					m_hmdDesc.DefaultEyeFov[i] ) ;
	}
	const ovrPosef	hmdToEyeOffset[2] =
	{
		m_eyeDesc[0].HmdToEyePose,
		m_eyeDesc[1].HmdToEyePose,
	} ;
	ovr_GetEyePoses2
		( m_session, m_indexFrame, ovrTrue,
			hmdToEyeOffset, m_layer.EyeFov.RenderPose,
			&m_layer.EyeFov.SensorSampleTime ) ;
	//
	// 頭の姿勢更新
	//
	double	ft =
		ovr_GetPredictedDisplayTime( m_session, m_indexFrame ) ;
	m_layer.EyeFov.SensorSampleTime = ovr_GetTimeInSeconds() ;
	//
	m_tsLast = ovr_GetTrackingState( m_session, ft, ovrTrue ) ;
	//
	// 姿勢情報変換
	//
	ConvertHeadEyeHandPosture() ;
}

// m_tsLast, m_eyeDesc, から Head, Eye, Hand の座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::ConvertHeadEyeHandPosture( void )
{
	ESLAssert( m_csDevState.TestLocked() ) ;
	//
	// 目の姿勢
	//
	EyeIndex	eyeIndex[ovrEye_Count] ;
	eyeIndex[ovrEye_Left] = eyeLeft ;
	eyeIndex[ovrEye_Right] = eyeRight ;
	//
	for ( int i = 0; i < ovrEye_Count; i ++ )
	{
		EyeIndex	eye = eyeIndex[i] ;
		ConvertPosture
			( m_postureEyes[eye], m_eyeDesc[i].HmdToEyePose ) ;
	}
	//
	// 頭の姿勢
	//
	const ovrTrackingState&	ts = m_tsLast ;
	S3DVector	vBasePos = m_vPositionBase ;
	m_postureHead.nFlags = ConvertTrackingStatus( ts.StatusFlags ) ;
	ConvertPosture( m_postureHead, ts.HeadPose.ThePose ) ;
	m_postureHead.vPosition += vBasePos ;
	//
	// 手の位置
	//
	m_postureHands[handRight].nFlags =
		ConvertTrackingStatus( ts.HandStatusFlags[ovrHand_Right] ) ;
	ConvertPosture
		( m_postureHands[handRight],
			ts.HandPoses[ovrHand_Right].ThePose ) ;
	m_postureHands[handRight].vPosition += vBasePos ;
	//
	m_postureHands[handLeft].nFlags =
		ConvertTrackingStatus( ts.HandStatusFlags[ovrHand_Left] ) ;
	ConvertPosture
		( m_postureHands[handLeft],
			ts.HandPoses[ovrHand_Left].ThePose ) ;
	m_postureHands[handLeft].vPosition += vBasePos ;
}

// ovrStatusBits -> PostureFlag 変換
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLOculusVRProducer::ConvertTrackingStatus( unsigned int nStatusFlags )
{
	uint32_t	nFlags = 0 ;
	if ( nStatusFlags & ovrStatus_OrientationTracked )
	{
		nFlags |= trackedOrientation ;
	}
	if ( nStatusFlags & ovrStatus_PositionTracked )
	{
		nFlags |= trackedPosition ;
	}
	return	nFlags ;
}

// ovrPosef -> EyePosture 変換
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::ConvertPosture
	( SGLVRViewProducer::Posture& postureDst, const ovrPosef& poseSrc ) const
{
	S3DQuaternion	qPose ;
	S3DMatrix		matPose ;
	qPose.q[0] =   poseSrc.Orientation.w ;
	qPose.q[1] =   poseSrc.Orientation.x ;
	qPose.q[2] = - poseSrc.Orientation.y ;
	qPose.q[3] = - poseSrc.Orientation.z ;
	qPose.ToMatrix( matPose ) ;
	//
	const float32_t	fpScale = (float32_t) GetScaleHMDToModel() ;
	//
	postureDst.matOrientation = matPose ;
	postureDst.vPosition.x =   poseSrc.Position.x * fpScale ;
	postureDst.vPosition.y = - poseSrc.Position.y * fpScale ;
	postureDst.vPosition.z = - poseSrc.Position.z * fpScale ;
}

// コントローラー取得
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::PollInputState( void )
{
	ovrInputState	state ;
	ovrResult	result =
		ovr_GetInputState
			( m_session, ovrControllerType_RTouch, &state ) ;
	if ( OVR_SUCCESS( result ) )
	{
		ConvertControllerState
			( m_stateControllers[controllerRight],
								controllerRight, state ) ;
	}
	else
	{
		eslFillMemory
			( &m_stateControllers[controllerRight],
				0, sizeof(m_stateControllers[controllerRight]) ) ;
	}
	result = ovr_GetInputState
				( m_session, ovrControllerType_LTouch, &state ) ;
	if ( OVR_SUCCESS( result ) )
	{
		ConvertControllerState
			( m_stateControllers[controllerLeft],
								controllerLeft, state ) ;
	}
	else
	{
		eslFillMemory
			( &m_stateControllers[controllerLeft],
				0, sizeof(m_stateControllers[controllerLeft]) ) ;
	}
}

// ovrInputState -> ControllerState 変換
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::ConvertControllerState
	( SGLVRViewProducer::ControllerState& stateDst,
			SGLVRViewProducer::ControllerIndex iController,
							const ovrInputState& stateSrc ) const
{
	stateDst.nState = controllerConnected ;
	stateDst.nCapacity =
		capAxisThumbStick | capAxisIndexTrigger | capAxisMiddleTrigger ;
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
			int	bitMask = m_iVRBurronTransTable[i] ;
			if ( stateSrc.Buttons & bitMask )
			{
				maskPressed |= bitButton ;
			}
			if ( stateSrc.Touches & bitMask )
			{
				maskTouched |= bitButton ;
			}
		}
	}
	//
	// アナログスティック・トリガー変換
	//
	ovrHandType	iHand = (iController == controllerRight)
									? ovrHand_Right : ovrHand_Left ;
	stateDst.vAxis[axisThumbStick].x = stateSrc.Thumbstick[iHand].x ;
	stateDst.vAxis[axisThumbStick].y = - stateSrc.Thumbstick[iHand].y ;
	stateDst.vAxis[axisIndexTrigger].x = stateSrc.IndexTrigger[iHand] ;
	stateDst.vAxis[axisIndexTrigger].y = 0.0f ;
	stateDst.vAxis[axisMiddleTrigger].x = stateSrc.HandTrigger[iHand] ;
	stateDst.vAxis[axisMiddleTrigger].y = 0.0f ;
	//
	if ( stateSrc.IndexTrigger[iHand] > 0.7f )
	{
		maskPressed |= ((uint64_t) 1 << buttonAxis1) ;
	}
	if ( stateSrc.HandTrigger[iHand] > 0.7f )
	{
		maskPressed |= ((uint64_t) 1 << buttonAxis2) ;
	}
	//
	const wchar_t *	pszCtrlName = nullptr ;
	if ( iController == controllerRight )
	{
		pszCtrlName = L"right" ;
	}
	else if ( iController == controllerLeft )
	{
		pszCtrlName = L"left" ;
	}
	if ( stateDst.maskButtonPressed != maskPressed )
	{
		LogTrace( L"Oculus %s button pressed: %08X:%08X\n",
				pszCtrlName, (DWORD)(maskPressed >> 32), (DWORD) maskPressed ) ;
	}
	if ( stateDst.maskButtonPressed != maskPressed )
	{
		LogTrace( L"Oculus %s button touched: %08X:%08X\n",
				pszCtrlName, (DWORD)(maskTouched >> 32), (DWORD) maskTouched ) ;
	}
	stateDst.maskButtonPressed = maskPressed ;
	stateDst.maskButtonTouched = maskTouched ;
}

// セッション再生成
//////////////////////////////////////////////////////////////////////////////
SGLOculusVRProducer::RecreateSessionProcedure::
		RecreateSessionProcedure( SGLOculusVRProducer * pVR )
	: m_pVR( pVR )
{
}

void SGLOculusVRProducer::RecreateSessionProcedure::Run( void )
{
	m_pVR->LogTrace( L"\nOculus Rift: recreate session\n" ) ;
	ESLAssert( m_pVR != nullptr ) ;
	m_pVR->RecreateSessionOnThread() ;
	//
	m_pVR->m_csRSProc.Lock() ;
	m_pVR->m_signalRSDone.SetSignal() ;
	m_pVR->m_csRSProc.Unlock() ;
}

void SGLOculusVRProducer::RecreateSessionProcedure::Finalize( void )
{
	SGLOculusVRProducer *	pVR = m_pVR ;
	pVR->m_csRSProc.Lock() ;
	pVR->m_pRSProc = nullptr ;
	pVR->m_csRSProc.Unlock() ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLOculusVRProducer::BeginDrawView
	( SGLAbstractWindow * pPrimaryWnd )
{
	if ( m_status < statusInitialized )
	{
		return	NULL ;
	}
	//
	// OVR 描画準備
	//
	if ( WaitFrameSync() || m_flagDisplayLost )
	{
		if ( m_flagDisplayLost && !m_flagPostDisplayLost )
		{
			SGLCommandInterface *
					pCmdItf = pPrimaryWnd->GetCommandInterface() ;
			if ( pCmdItf != NULL )
			{
				SString	strID = ID_OCULUS_DISPLAY_LOST ;
				pCmdItf->OnCommand
					( pPrimaryWnd, strID.GetConstArray(), 0, 0 ) ;
			}
			m_flagPostDisplayLost = true ;
		}
		return	NULL ;
	}
	if ( OVR_FAILURE( ovr_BeginFrame( m_session, m_indexFrame ) ) )
	{
		LogTrace( L"failed to ovr_BeginFrame(%d).\n", (int) m_indexFrame ) ;
		return	NULL ;
	}
	//
	// Swap chain インデックス取得
	//
	int	iSwapChainLeft, iSwapChainRight ;
	if ( OVR_FAILURE( ovr_GetTextureSwapChainCurrentIndex
		( m_session, m_layer.EyeFov.ColorTexture[ovrEye_Left], &iSwapChainLeft ) ) )
	{
		LogTrace( L"failed to ovr_GetTextureSwapChainCurrentIndex for left eye.\n" ) ;
		return	NULL ;
	}
	if ( OVR_FAILURE( ovr_GetTextureSwapChainCurrentIndex
		( m_session, m_layer.EyeFov.ColorTexture[ovrEye_Right], &iSwapChainRight ) ) )
	{
		LogTrace( L"failed to ovr_GetTextureSwapChainCurrentIndex for right eye.\n" ) ;
		return	NULL ;
	}
	//
	// レンダリング用バッファ取得
	//
	SGLImageObject *	pImageLeft =
		m_aRenderColor[ovrEye_Left].GetAt( (size_t) iSwapChainLeft ) ;
	SGLImageObject *	pImageRight =
		m_aRenderColor[ovrEye_Right].GetAt( (size_t) iSwapChainRight ) ;
	if ( pImageLeft == NULL )
	{
		LogTrace( L"not render target for Oculus at left %d.\n", iSwapChainLeft ) ;
	}
	if ( pImageRight == NULL )
	{
		LogTrace( L"not render target for Oculus at right %d.\n", iSwapChainRight ) ;
	}
	m_pRenderer->GetDirectlyRenderer().
		SetViewVerticalOrder( S3DOpenGLDirectlyRenderer::verticalStright ) ;
	m_pRenderer->AttachStereoTargetImage
		( pImageRight, pImageLeft,
			&m_imgRenderDepth[ovrEye_Right],
			&m_imgRenderDepth[ovrEye_Left] ) ;
	m_pRenderer->SetOptionalFeature
		( RenderContext::featureSRGB, true, NULL, 0 ) ;
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
void SGLOculusVRProducer::EndDrawView
	( SGLAbstractWindow * pPrimaryWnd, RenderContext * render )
{
	ESLAssert( m_status >= statusInitialized ) ;

	m_pRenderer->Finish() ;
	m_pRenderer->DetachTargetImage() ;
	//
	if ( OVR_FAILURE( ovr_CommitTextureSwapChain
		( m_session, m_layer.EyeFov.ColorTexture[ovrEye_Left] ) ) )
	{
		LogTrace( L"failed to ovr_CommitTextureSwapChain for ovrEye_Left.\n" ) ;
	}
	if ( OVR_FAILURE( ovr_CommitTextureSwapChain
		( m_session, m_layer.EyeFov.ColorTexture[ovrEye_Right] ) ) )
	{
		LogTrace( L"failed to ovr_CommitTextureSwapChain for ovrEye_Right.\n" ) ;
	}
	/*
	ovrViewScaleDesc	vsd ;
	for ( int i = 0; i < ovrEye_Count; i ++ )
	{
		vsd.HmdToEyePose[i] = m_eyeDesc[i].HmdToEyeViewOffset ;
	}
	vsd.HmdSpaceToWorldScaleInMeters = (float) m_fpScaleHMDToModel ;
	*/

	/*
	m_layer.EyeFovDepth.ProjectionDesc.Projection22 =
									m_matPerspective[0].m[2][2] ;
	m_layer.EyeFovDepth.ProjectionDesc.Projection23 =
									m_matPerspective[0].m[3][2] ;
	m_layer.EyeFovDepth.ProjectionDesc.Projection32 =
									m_matPerspective[0].m[2][3] ;
	*/

	const ovrLayerHeader *	pLayerList[1] =
	{
		&m_layer.Header
	} ;

	if ( OVR_FAILURE
		( ovr_EndFrame
			( m_session, m_indexFrame, NULL, &pLayerList[0], 1 ) ) )
	{
		LogTrace( L"failed to ovr_EndFrame(%d).\n", (int) m_indexFrame ) ;
	}
	else
	{
		m_indexFrame ++ ;
	}
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::FlipView
	( SGLAbstractWindow * pPrimaryWnd, bool fVSync )
{
	ESLAssert( m_status >= statusInitialized ) ;
	if ( (m_status < statusInitialized) || m_flagDisplayLost )
	{
		return ;
	}

	LockForDeviceState() ;
	m_signalReadyFrame.ResetSignal() ;
	m_signalEndFrame.SetSignal() ;
	UnlockForDeviceState() ;

	if ( m_flagDrawToPrimary )
	{
		m_fboMirror.BlitFramebufferTo
			( 0, 0, m_sizeWindow.h, m_sizeWindow.w, 0,
					0, 0, m_sizeWindow.w, m_sizeWindow.h ) ;
		glFlush() ;
	}
}

// 表示状態か？
//////////////////////////////////////////////////////////////////////////////
bool SGLOculusVRProducer::IsVisibleView( void ) const
{
	return	true ;
}

// デバイス状態更新タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::WaitForPollDeviceState( int64_t msecTimeout )
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
SGLError SGLOculusVRProducer::ResetCurrentHMDPosture( void )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	ovrResult	result = ovr_RecenterTrackingOrigin( m_session ) ;
	if ( OVR_SUCCESS( result ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 現在の HMD 座標を基準座標にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::ResetCurrentHMDPosition( void )
{
	if ( m_status < statusInitialized )
	{
		return	sglErrFailed ;
	}
	LockForDeviceState() ;
	m_vPositionBase -= S3DDVector( GetHeadPosture().vPosition ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// HMD 空間タイプ取得
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::ViewSpaceType
	SGLOculusVRProducer::GetViewSpaceType( void ) const
{
	return	m_viewSpaceType ;
}

// デバイスモデル取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::GetControllerModel
	( SGLVRViewProducer::ModelInfo& mi,
			SGLVRViewProducer::ControllerIndex iCtrl )
{
	return	sglErrFailed ;
}

// HMD スケール
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::SetScaleHMDToModel( double fpScale )
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
void SGLOculusVRProducer::Run( void )
{
	while ( !m_flagQuitThread )
	{
		ovrResult	result = ovr_WaitToBeginFrame( m_session, m_indexFrame ) ;
		m_timerSignalFrame.Reset() ;
		if ( OVR_SUCCESS( result ) )
		{
			m_flagVisibleFrame = true ;
			//
			// 姿勢・入力取得
			//
			LockForDeviceState() ;
			PollSessionStatus() ;
			PollEyePosture() ;
			PollInputState() ;
			m_signalReadyInput.SetSignal() ;
			UnlockForDeviceState() ;
			//
			// フレーム準備完了
			//
			m_flagDisplayLost = false ;
			m_signalReadyFrame.SetSignal() ;
		}
		else
		{
			LogTrace( L"failed to ovr_WaitToBeginFrame(%d), result = %d\n", m_indexFrame, result ) ;
			if ( result == ovrError_DisplayLost )
			{
				LogTrace( L"Oculus Rift device lost.\n" ) ;
				m_flagDisplayLost = true ;
				//
				ESLAssert( m_pOpenGL != nullptr ) ;
				m_csRSProc.Lock() ;
				if ( (m_pRSProc == nullptr) && (m_pOpenGL != nullptr) )
				{
					m_signalRSDone.ResetSignal() ;
					//
					m_pRSProc = new RecreateSessionProcedure( this ) ;
					if ( m_pOpenGL->Procedure
						( m_pRSProc, S3DRenderDevice::procedureAsync ) != sglErrSuccess )
					{
						m_signalRSDone.SetSignal() ;
						m_pRSProc = nullptr ;
					}
				}
				m_csRSProc.Unlock() ;
			}
			m_signalReadyFrame.SetSignal() ;
			SleepMilliSec( 10 ) ;
		}
		while ( !m_flagQuitThread
			&& (m_signalEndFrame.Wait( 10 ) == errTimeout) )
		{
			/*
			// ※次のフレーム更新より先に HMD の位置を取得すると
			// 　フレームがカク付くのでポーリングはしちゃだめ
			LockForDeviceState() ;
			PollSessionStatus() ;
			PollEyePosture() ;
			PollInputState() ;
			UnlockForDeviceState() ;
			*/
		}
		m_signalEndFrame.ResetSignal() ;
	}
}

// 更新タイミング待ち
//////////////////////////////////////////////////////////////////////////////
SGLError SGLOculusVRProducer::WaitForView( int64_t msecTimeout )
{
	if ( msecTimeout == 0 )
	{
		if ( (m_status < statusInitialized) || !m_flagBeganThread )
		{
			return	sglErrFailed ;
		}
	}
	return	WaitFrameSync( msecTimeout ) ;
}


// ログ出力するファイル設定
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::SetLogFile( SSystem::SBufferedFile * pfile )
{
	m_pLogFile = pfile ;
}

// デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void SGLOculusVRProducer::LogTrace( const wchar_t * pwszFormat, ... ) const
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

