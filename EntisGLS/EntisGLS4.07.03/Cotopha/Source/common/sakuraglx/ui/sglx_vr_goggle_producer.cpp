
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_opengl_window_producer.h>
#include <sakuraglx/ui/sglx_vr_goggle_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// VR ゴーグル用 SideBySide 出力 HMD インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLSideBySideVRGoggleProducer,
			SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSideBySideVRGoggleProducer::SGLSideBySideVRGoggleProducer( void )
	: m_matBasePose( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vBasePosition( 0, 0, 0 )
{
	m_pWindow = NULL ;
	m_flagChangeMode = false ;
	m_flagPollingThread = false ;
	m_flagQuitPolling = false ;

	m_flagNoPosition = true ;

	m_lensDistortion.fpDistortion[0] = 1.1f ;
	m_lensDistortion.fpDistortion[1] = -0.1f ;
	m_lensDistortion.fpDistortion[2] = 0.0f ;
	m_lensDistortion.fpDistortion[3] = 0.0f ;
	m_lensDistortion.fpLensScale = 1.0f ;
	m_lensDistortion.fpOffsetX = 0.0f ;

	m_degHorzFOV = 70.0f ;

	m_xParallax = 0.03f ;
	m_zParallaxFocus = 0.5f ;
	UpdateEyePostures() ;

	m_msecFrameInterval = 16 ;
	m_flagPostUpdateFrame = false ;

	SetDrawingToPrimaryWindow( true ) ;
}

// 加速度センサでHMD座標を計算する
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::EnableHMDPositionByAccelerometer( bool fEnable )
{
	m_flagNoPosition = !fEnable ;
}

bool SGLSideBySideVRGoggleProducer::IsEnabledHMDPositionByAccelerometer( void ) const
{
	return	!m_flagNoPosition ;
}

// フレーム更新インターバル
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetFrameInterval( double msecInterval, bool fPostUpdate )
{
	m_msecFrameInterval = msecInterval ;
	m_flagPostUpdateFrame = fPostUpdate ;
}

double SGLSideBySideVRGoggleProducer::GetFrameInterval( void ) const
{
	return	m_msecFrameInterval ;
}

// レンズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetLensDistortion( const SGLSideBySideVRGoggleProducer::LensDistortion& ld )
{
	m_lensDistortion = ld ;
	//
	if ( m_pWindow != NULL )
	{
		UpdateLensDistortion() ;
	}
}

void SGLSideBySideVRGoggleProducer::GetLensDistortion( SGLSideBySideVRGoggleProducer::LensDistortion& ld ) const
{
	ld = m_lensDistortion ;
}

void SGLSideBySideVRGoggleProducer::UpdateLensDistortion( void )
{
	if ( m_pWindow == NULL )
	{
		return ;
	}
	SGLOpenGLWindowProducer *	poglwp =
			ESLTypeCast<SGLOpenGLWindowProducer>
						( m_pWindow->GetRenderDevice() ) ;
	if ( poglwp != NULL )
	{
		poglwp->SetLensDistortion
			( m_lensDistortion.fpDistortion, 4,
				m_lensDistortion.fpLensScale, m_lensDistortion.fpOffsetX ) ;
	}
}

// 視野角（水平）[deg]
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetFieldOfView( float32_t degHFOV )
{
	LockForDeviceState() ;
	m_degHorzFOV = degHFOV ;
	if ( m_flagChangeMode )
	{
		UpdateEyeFieldOfView() ;
	}
	UnlockForDeviceState() ;
}

float32_t SGLSideBySideVRGoggleProducer::GetFieldOfView( void ) const
{
	return	m_degHorzFOV ;
}

// 視差設定（目間距離の半分）
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetParallax( float32_t xParallax )
{
	LockForDeviceState() ;
	m_xParallax = xParallax ;
	UpdateEyePostures() ;
	UnlockForDeviceState() ;
}

float32_t SGLSideBySideVRGoggleProducer::GetParallax( void ) const
{
	return	m_xParallax ;
}

// 視差焦点設定
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetParallaxFocus( float32_t zFocus )
{
	LockForDeviceState() ;
	m_zParallaxFocus = zFocus ;
	UpdateEyePostures() ;
	UnlockForDeviceState() ;
}

float32_t SGLSideBySideVRGoggleProducer::GetParallaxFocus( void ) const
{
	return	m_zParallaxFocus ;
}

void SGLSideBySideVRGoggleProducer::UpdateEyePostures( void )
{
	Posture&		poseR = m_postureEyes[eyeRight] ;
	Posture&		poseL = m_postureEyes[eyeLeft] ;
	S3DVector		vI( 1, 1, 1 ) ;
	const float32_t	fpScale = (float32_t) GetScaleHMDToModel() ;
	//
	poseR.nFlags = trackedOrientation | trackedPosition ;
	poseR.vPosition.x = m_xParallax * fpScale ;
	poseR.vPosition.y = 0.0f ;
	poseR.vPosition.z = 0.0f ;
	poseR.matOrientation.InitializeMatrix( vI ) ;
	//
	poseL.nFlags = trackedOrientation | trackedPosition ;
	poseL.vPosition.x = - m_xParallax * fpScale ;
	poseL.vPosition.y = 0.0f ;
	poseL.vPosition.z = 0.0f ;
	poseL.matOrientation.InitializeMatrix( vI ) ;
	//
	if ( m_zParallaxFocus != 0.0f )
	{
		S3DVector	vDir( - m_xParallax, 0.0f, m_zParallaxFocus ) ;
		if ( m_zParallaxFocus < 0.0f )
		{
			vDir = - vDir ;
		}
		poseR.matOrientation.RevolveForAngle( vDir ) ;
		//
		vDir.x = - vDir.x ;
		poseL.matOrientation.RevolveForAngle( vDir ) ;
	}
}

void SGLSideBySideVRGoggleProducer::UpdateEyeFieldOfView( void )
{
	EyeFieldOfView&	fovR = m_fovEyes[eyeRight] ;
	EyeFieldOfView&	fovL = m_fovEyes[eyeLeft] ;
	//
	float32_t	tanHalfFOV =
					(float32_t) tan( (m_degHorzFOV * 0.5) * PI / 180.0 ) ;
	//
	fovR.sizeOfView = m_sizeLogicalView ;
	fovR.vScreenPos.x = (float32_t) m_sizeLogicalView.w * 0.5f ;
	fovR.vScreenPos.y = (float32_t) m_sizeLogicalView.h * 0.5f ;
	fovR.vScreenPos.z = fovR.vScreenPos.x / tanHalfFOV ;
	fovR.fpPixelAspect = 1.0f ;
	//
	fovL = fovR ;
}

// 開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::BeginHMDMode( SGLAbstractWindow * pWnd )
{
	if ( m_flagChangeMode )
	{
		return	sglErrFailed ;
	}
	ESLAssert( pWnd != NULL ) ;
	if ( pWnd == NULL )
	{
		return	sglErrInvalidParam ;
	}
	//
	// ウィンドウ・クライアントサイズ取得
	//
	SGLImageRect	rectRender, rectDisplay ;
	if ( pWnd->GetInternalDisplayPosition( rectRender, rectDisplay ) )
	{
		return	sglErrFailed ;
	}
	pWnd->GetDisplaySize( m_sizeOrgDisplay ) ;
	//
	m_sizePhysDisplay = rectRender.GetSize() ;
	m_sizeLogicalView.w = m_sizePhysDisplay.w / 2 ;
	m_sizeLogicalView.h = m_sizePhysDisplay.h ;
	//
	// SideBySide 表示モードへ変更
	//
	if ( pWnd->SetStereoDisplayMode
		( Window::Stereo3D::SideBySide,
			Window::stereoFlagPixelAspect1_1
				| Window::stereoFlagLensScale
				| Window::stereoFlagLensDistortion ) )
	{
		return	sglErrFailed ;
	}
	pWnd->ChangeDisplaySize( m_sizeLogicalView.w, m_sizeLogicalView.h ) ;
	m_pWindow = pWnd ;
	m_flagChangeMode = true ;
	//
	// 画面情報設定
	//
	UpdateEyePostures() ;
	UpdateEyeFieldOfView() ;
	UpdateLensDistortion() ;
	//
	// ポーリングスレッド
	//
	m_flagQuitPolling = false ;
	m_signalInitThread.Initialize( false ) ;
	m_signalDevPolled.Initialize( false ) ;
	m_signalFrameUpdate.Initialize( false ) ;
	if ( m_threadPolling.BeginThread( this ) )
	{
		return	sglErrFailed ;
	}
	m_signalInitThread.Wait() ;
	m_flagPollingThread = true ;
	return	sglErrSuccess ;
}

// 終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::EndHMDMode( void )
{
	if ( m_flagPollingThread )
	{
		m_flagQuitPolling = true ;
		m_threadPolling.Wait() ;
		m_threadPolling.Delete() ;
		m_signalInitThread.Delete() ;
		m_signalDevPolled.Delete() ;
		m_signalFrameUpdate.Delete() ;
		m_flagPollingThread = false ;
		m_flagQuitPolling = false ;
	}
	if ( m_flagChangeMode )
	{
		m_pWindow->SetStereoDisplayMode( Window::Stereo3D::MonoView ) ;
		m_pWindow->ChangeDisplaySize
						( m_sizeOrgDisplay.w, m_sizeOrgDisplay.h ) ;
		m_pWindow = NULL ;
		m_flagChangeMode = false ;
	}
	return	sglErrSuccess ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLSideBySideVRGoggleProducer::BeginDrawView
	( SGLAbstractWindow * pPrimaryWnd )
{
	if ( m_pWindow == NULL )
	{
		return	NULL ;
	}
	SGLWindowViewProducer *	pwvp =
		ESLTypeCast<SGLWindowViewProducer>( m_pWindow->GetRenderDevice() ) ;
	if ( pwvp == NULL )
	{
		return	NULL ;
	}
	return	pwvp->BeginDrawView( pPrimaryWnd, true ) ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::EndDrawView
	( SGLAbstractWindow * pPrimaryWnd, RenderContext * render )
{
	if ( m_pWindow == NULL )
	{
		return ;
	}
	SGLWindowViewProducer *	pwvp =
		ESLTypeCast<SGLWindowViewProducer>( m_pWindow->GetRenderDevice() ) ;
	if ( pwvp == NULL )
	{
		return ;
	}
	return	pwvp->EndDrawView( pPrimaryWnd, render, true ) ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::FlipView
	( SGLAbstractWindow * pPrimaryWnd, bool fVSync )
{
	if ( m_pWindow == NULL )
	{
		return ;
	}
	SGLWindowViewProducer *	pwvp =
		ESLTypeCast<SGLWindowViewProducer>( m_pWindow->GetRenderDevice() ) ;
	if ( pwvp == NULL )
	{
		return ;
	}
	return	pwvp->FlipView( pPrimaryWnd, fVSync, true ) ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSideBySideVRGoggleProducer::IsStereoDisplayMode( void )
{
	return	(m_pWindow != NULL) ;
}

// プライマリウィンドウへの描画も行うか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSideBySideVRGoggleProducer::DoesDrawToPrimaryWindow( void )
{
	return	true ;
}

// 表示状態か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSideBySideVRGoggleProducer::IsVisibleView( void ) const
{
	return	true ;
}

// デバイス状態更新タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::WaitForPollDeviceState( int64_t msecTimeout )
{
	SGLError	err =
		(SGLError) m_signalDevPolled.Wait( msecTimeout ) ;
	if ( !err )
	{
		LockForDeviceState() ;
		m_signalDevPolled.ResetSignal() ;
		UnlockForDeviceState() ;
	}
	return	err ;
}

// 現在の HMD 姿勢を基準座標・姿勢にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::ResetCurrentHMDPosture( void )
{
	if ( !m_flagPollingThread )
	{
		return	sglErrFailed ;
	}
	LockForDeviceState() ;
	m_posture.ResetPosture() ;
	m_matBasePose.InverseOf( m_posture.GetPosture() ) ;
	m_vBasePosition = S3DDVector( 0, 0, 0 ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// 現在の HMD 座標を基準座標にリセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::ResetCurrentHMDPosition( void )
{
	if ( !m_flagPollingThread )
	{
		return	sglErrFailed ;
	}
	LockForDeviceState() ;
	m_posture.ResetPosition() ;
	m_vBasePosition -= S3DDVector( GetHeadPosture().vPosition ) ;
	UnlockForDeviceState() ;
	return	sglErrSuccess ;
}

// デバイスモデル取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::GetControllerModel
	( SGLVRViewProducer::ModelInfo& mi, SGLVRViewProducer::ControllerIndex iCtrl )
{
	return	sglErrFailed ;
}

// HMD スケール
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::SetScaleHMDToModel( double fpScale )
{
	LockForDeviceState() ;
	SGLVRViewProducer::SetScaleHMDToModel( fpScale ) ;
	UpdateEyePostures() ;
	UnlockForDeviceState() ;
}

// HMD 空間タイプ取得
//////////////////////////////////////////////////////////////////////////////
SGLVRViewProducer::ViewSpaceType
	SGLSideBySideVRGoggleProducer::GetViewSpaceType( void ) const
{
	return	spaceEyeLevel ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLSideBySideVRGoggleProducer::Run( void )
{
	m_posture.PrepareSensor
		( UI::SGLPostureSensor::featureAccelerometer
			| UI::SGLPostureSensor::featureGyroscope
			| UI::SGLPostureSensor::featureRotateLandScape ) ;
	m_posture.ResetPosture() ;
	//
	const uint32_t	nSensorFeatures = m_posture.GetSensorFeatures() ;
	LockForDeviceState() ;
	m_matBasePose.InverseOf( m_posture.GetPosture() ) ;
	UnlockForDeviceState() ;
	//
	m_signalInitThread.SetSignal() ;
	//
	STimeCounter	timer ;
	while ( !m_flagQuitPolling )
	{
		//
		// 姿勢ポーリング
		//
		SleepMilliSec( 1 ) ;
		//
		if ( m_posture.WaitSensor( nSensorFeatures, 5 ) != sglErrSuccess )
		{
			SleepMilliSec( 1 ) ;
		}
		m_posture.PollSensor() ;
		//
		LockForDeviceState() ;
		m_postureHead.nFlags = trackedOrientation | trackedPosition ;
		m_postureHead.matOrientation = m_posture.GetPosture() * m_matBasePose ;
		if ( m_flagNoPosition )
		{
			m_postureHead.vPosition = m_vBasePosition ;
		}
		else
		{
			m_postureHead.vPosition =
					m_posture.GetPosition()
							* GetScaleHMDToModel() + m_vBasePosition ;
		}
		m_signalDevPolled.SetSignal() ;
		UnlockForDeviceState() ;
		//
		// フレーム更新
		//
		if ( timer.GetRealTime() >= m_msecFrameInterval )
		{
			timer.Reset() ;
			//
			LockForDeviceState() ;
			m_signalFrameUpdate.SetSignal() ;
			if ( m_flagPostUpdateFrame )
			{
				m_pWindow->PostUpdate() ;
			}
			UnlockForDeviceState() ;
		}
	}
	m_posture.Release() ;
}

// 更新タイミング待ち
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSideBySideVRGoggleProducer::WaitForView( int64_t msecTimeout )
{
	return	(SGLError) m_signalFrameUpdate.Wait( msecTimeout ) ;
}

