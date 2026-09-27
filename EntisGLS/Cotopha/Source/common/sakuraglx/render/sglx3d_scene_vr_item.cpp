
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_vr_item.h>
#include <sakuraglx/sprite/sglx_sprite_cursor.h>
#include <sakuraglx/render/sglx3d_scene_sprite.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// VR HMD ポジショントラッキング
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCameraHMDPosition, CameraController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCameraHMDPosition, camera_hmd_position )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraHMDPosition::S3DCameraHMDPosition
		( SGLVRViewProducer * pVR, uint32_t nFlags )
	: CameraController( m_ItemClassDescriptor.pwszClassID ),
		m_matIBase( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_vOrigin( 0, 0, 0 ), m_yExtraHMD( -1.1 ),
		m_flagModifiedCamera( false ),
		m_vModifiedCameraPos( 0, 0, 0 ),
		m_vModifiedCameraTarget( 0, 0, 1 ),
		m_vModifiedCameraTop( 0, -1, 0 )
{
	m_pVR = pVR ;
	m_nCtrlFlags = nFlags ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraHMDPosition::~S3DCameraHMDPosition( void )
{
}

// VR HMD 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DCameraHMDPosition::AttachVRHMD( SGLVRViewProducer * pVR )
{
	m_pVR = pVR ;
}

// 制御フラグ
//////////////////////////////////////////////////////////////////////////////
void S3DCameraHMDPosition::SetControllFlags( uint32_t nFlags )
{
	m_nCtrlFlags = nFlags ;
}

// HMD 基準 y 座標
//////////////////////////////////////////////////////////////////////////////
double S3DCameraHMDPosition::GetHMDExtraY( void ) const
{
	return	m_yExtraHMD ;
}

void S3DCameraHMDPosition::SetHMDExtraY( double y )
{
	m_yExtraHMD = y ;
}

// 現在の姿勢を基準姿勢にリセット
//////////////////////////////////////////////////////////////////////////////
void S3DCameraHMDPosition::ResetCurrentPosture( void )
{
	Lock() ;
	if ( m_postureLast.nFlags & SGLVRViewProducer::trackedOrientation )
	{
		m_matIBase.InverseOf( m_postureLast.matOrientation ) ;
	}
	if ( m_postureLast.nFlags & SGLVRViewProducer::trackedPosition )
	{
		m_vOrigin = - m_postureLast.vPosition ;
	}
	Unlock() ;
}

void S3DCameraHMDPosition::ResetCurrentPosition( void )
{
	Lock() ;
	if ( m_postureLast.nFlags & SGLVRViewProducer::trackedPosition )
	{
		m_vOrigin = - m_postureLast.vPosition ;
	}
	Unlock() ;
}

// 最後に取得された VR HMD で修正される前のカメラ座標取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCameraHMDPosition::GetLastModifiedCameraExceptVR
	( S3DDVector& vPos, S3DDVector& vTarget, S3DDVector& vTop ) const
{
	if ( !m_flagModifiedCamera )
	{
		return	false ;
	}
	Lock() ;
	vPos = m_vModifiedCameraPos ;
	vTarget = m_vModifiedCameraTarget ;
	vTop = m_vModifiedCameraTop ;
	Unlock() ;
	return	true ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DCameraHMDPosition::OnBeforeRender
	( S3DScene& scene, S3DDynamicCamera * pItem )
{
	if ( m_pVR == NULL )
	{
		return ;
	}
	m_flagModifiedCamera = true ;
	m_vModifiedCameraPos = pItem->m_space.m_vCenter ;
	m_vModifiedCameraTarget = pItem->m_vTarget ;
	m_vModifiedCameraTop = pItem->m_vTop ;
	//
	if ( (m_nCtrlFlags & flagExtraHMDLevel)
		&& !(m_nCtrlFlags & flagIgnoreHMDPosition) )
	{
		double	yExtraHMD = (m_pVR->GetViewSpaceType()
									== SGLVRViewProducer::spaceEyeLevel)
								? m_yExtraHMD : 0.0 ;
		double	yOffset = yExtraHMD - pItem->m_space.m_vCenter.y ;
		pItem->m_space.m_vCenter.y = yExtraHMD ;
		pItem->m_vTarget.y += yOffset ;
	}
	//
	m_postureLast = m_pVR->GetHeadPosture() ;
	//
	SGLVRViewProducer::Posture	postureBase ;
	if ( m_nCtrlFlags & flagIgnoreHMDRotation )
	{
		postureBase.matOrientation = m_postureLast.matOrientation * m_matIBase ;
		m_postureLast.matOrientation.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	}
	if ( m_nCtrlFlags & flagIgnoreHMDPosition )
	{
		postureBase.vPosition = m_postureLast.vPosition ;
		postureBase.vPosition -= postureBase.matOrientation
									* S3DVector( 0, pItem->m_space.m_vCenter.y, 0 ) ;
		m_postureLast.vPosition.x = 0 ;
		m_postureLast.vPosition.y = 0 ;
		m_postureLast.vPosition.z = 0 ;
	}
	m_pVR->SetHandsBasePosture( postureBase ) ;
	//
	S3DDMatrix	matCameraRotation =
					m_postureLast.matOrientation * m_matIBase ;
	S3DDVector	vCameraOffset = m_postureLast.vPosition - m_vOrigin ;
	bool		flagLevel = ((m_nCtrlFlags & flagLevelMatching) != 0) ;
	pItem->ModifyCameraPosture
		( matCameraRotation, vCameraOffset, flagLevel, flagLevel ) ;
	//
	S3DScene::ProjectionParam	projParam ;
	scene.GetProjectionOfView( projParam, m_pVR ) ;
	//
	RenderContext::StereoViewIndex	sviIndex[2] ;
	sviIndex[SGLVRViewProducer::eyeRight] = RenderContext::stereoViewRight ;
	sviIndex[SGLVRViewProducer::eyeLeft] = RenderContext::stereoViewLeft ;
	//
	S3DScene::StereoParallaxParam	spp ;
	for ( int i = 0; i < SGLVRViewProducer::eyeCount; i ++ )
	{
		const SGLVRViewProducer::Posture&
			postureEye = m_pVR->GetEyePosture
							( (SGLVRViewProducer::EyeIndex) i ) ;
		const SGLVRViewProducer::EyeFieldOfView&
			fovEye = m_pVR->GetEyeFieldOfView
							( (SGLVRViewProducer::EyeIndex) i ) ;
		RenderContext::StereoViewIndex	svi = sviIndex[i] ;
		spp.ppView[svi].matPosture = postureEye.matOrientation ;
		spp.ppView[svi].vParallax = postureEye.vPosition ;
		spp.ppView[svi].vScreenDelta =
							fovEye.vScreenPos - projParam.vScreen ;
		spp.ppView[svi].fpAspectDelta =
							fovEye.fpPixelAspect - projParam.fpPixelAspect ;
	}
	pItem->SetParallaxOfView( m_pVR, spp ) ;
	//
	scene.PostSceneUpdate() ;
}


//////////////////////////////////////////////////////////////////////////////
// VR コントローラーモデル表示アイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DVRControllerModelSerializer, ModelSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DVRControllerModelSerializer, vr_controller_model )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DVRControllerModelSerializer::S3DVRControllerModelSerializer( void )
	: m_matTipBeam( 1, 1, 1 ), m_vTipBeam( 0, 0, 0 )
{
	m_classItem = S3DScene::classEffect2 ;
	m_nRenderPriority = 12 ;
	m_flagsBehavior |= S3DScene::itemTimer ;
	m_pVR = NULL ;
	m_iHand = SGLVRViewProducer::handRight ;
	m_iController = SGLVRViewProducer::controllerRight ;
	m_flagVisibleController = false ;
	m_pCtrlModelVBO = NULL ;

	m_flagVisibleBeam = false ;
	m_fpBeamLength = m_bpBeam.fpDefaultLength ;
}

// VR 設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRControllerModelSerializer::SetVRController
	( SGLVRViewProducer * pVR,
		SGLVRViewProducer::HandIndex iHand,
		SGLVRViewProducer::ControllerIndex iController )
{
	m_pVR = pVR ;
	m_iHand = iHand ;
	m_iController = iController ;
}

// ポインタ用ビーム設定
//////////////////////////////////////////////////////////////////////////////
const S3DVRControllerModelSerializer::BeamParam&
		S3DVRControllerModelSerializer::GetBeamParam( void ) const
{
	return	m_bpBeam ;
}

void S3DVRControllerModelSerializer::SetBeamParam
		( const S3DVRControllerModelSerializer::BeamParam& bp )
{
	m_bpBeam = bp ;
	m_fpBeamLength = m_bpBeam.fpDefaultLength ;
}

bool S3DVRControllerModelSerializer::IsBeamVisible( void ) const
{
	return	m_flagVisibleBeam ;
}

void S3DVRControllerModelSerializer::SetBeamVisible( bool flagVisible )
{
	m_flagVisibleBeam = flagVisible ;
}

float32_t S3DVRControllerModelSerializer::GetBeamLength( void ) const
{
	return	m_fpBeamLength ;
}

void S3DVRControllerModelSerializer::SetBeamLength( float32_t fpLength )
{
	m_fpBeamLength = fpLength ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DVRControllerModelSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ModelSerializer::OnTimer( scene, msecPast ) ;
	//
	m_flagVisibleController = false ;
	if ( m_pVR != NULL )
	{
		const SGLVRViewProducer::Posture&
					pose = m_pVR->GetHandPosture( m_iHand ) ;
		uint32_t	nTrackedFlags = SGLVRViewProducer::trackedPosition
									| SGLVRViewProducer::trackedOrientation ;
		if ( (pose.nFlags & nTrackedFlags) == nTrackedFlags )
		{
			const SGLVRViewProducer::ControllerState&
					ctrl = m_pVR->GetControllerState( m_iController ) ;
			if ( ctrl.nState & SGLVRViewProducer::controllerConnected )
			{
				m_flagVisibleController = true ;
				//
				SGLVRViewProducer::ModelInfo	mi ;
				if ( !m_pVR->GetControllerModel( mi, m_iController ) )
				{
					if ( mi.status == SGLVRViewProducer::statusModelCompleted )
					{
						m_pCtrlModelVBO = mi.pVBO ;
						//
						if ( !(mi.nFlags & SGLVRViewProducer::modelVisible) )
						{
							m_flagVisibleController = false ;
						}
					}
					if ( mi.postureTipLocal.nFlags & SGLVRViewProducer::trackedOrientation )
					{
						m_matTipBeam = mi.postureTipLocal.matOrientation ;
					}
					if ( mi.postureTipLocal.nFlags & SGLVRViewProducer::trackedPosition )
					{
						m_vTipBeam = mi.postureTipLocal.vPosition ;
					}
				}
			}
		}
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DVRControllerModelSerializer::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_flagVisibleController )
	{
		if ( m_pModel != NULL )
		{
			ModelSerializer::RenderLocalModel( scene, render, flagsExclusion ) ;
		}
		else if ( (m_pCtrlModelVBO != NULL)
				&& m_pCtrlModelVBO->IsModelIntoView( &render ) )
		{
			ShaderSaver	ss ;
			PrepareShaderSettings( scene, render, ss ) ;
			render.PushTransformation() ;
			//
			double		fpScale = m_pVR ? m_pVR->GetScaleHMDToModel() : 1.0 ;
			S3DDMatrix	matScale( fpScale, fpScale, fpScale ) ;
			S3DDVector	vZero( 0, 0, 0 ) ;
			render.AppendMatrixTransformation( matScale, vZero ) ;
			//
			m_pCtrlModelVBO->RenderBufferTo
				( &render, flagsExclusion | m_flagsModelExclusion ) ;
			//
			render.PopTransformation() ;
			RestoreShaderSettings( scene, render, ss ) ;
			//
			Item::RenderLocalModel( scene, render, flagsExclusion ) ;
		}
		else
		{
			Item::RenderLocalModel( scene, render, flagsExclusion ) ;
		}
		if ( m_flagVisibleBeam )
		{
			S3DMaterial *	pMaterial = new S3DMaterial ;
			S3DSurfaceAttribute	attr ;
			attr.flagsShading =
				shadingMethodNothing | shadingZBufferNoWrite | shadingHintPriority2 ;
			attr.colorBase.rgbAdd = m_bpBeam.argbColor ;
			attr.colorBase.rgbMul.argb.Red = ~m_bpBeam.argbColor.argb.Alpha ;
			attr.colorBase.rgbMul.argb.Green = ~m_bpBeam.argbColor.argb.Alpha ;
			attr.colorBase.rgbMul.argb.Blue = ~m_bpBeam.argbColor.argb.Alpha ;
			pMaterial->SetSurfaceAttribute( attr ) ;
			//
			S3DVector4	vPoint[3] ;
			S3DVector4	vNormal[3] ;
			uint32_t	nIndex[3] = { 0, 1, 2 } ;
			//
			S3DVector	vRDir = m_matTipBeam * m_bpBeam.vDirection.Normalized() ;
			S3DVector	vTDir = m_matTipBeam * m_bpBeam.vThickDir.Normalized() ;
			S3DVector	vBase = vRDir * m_bpBeam.fpOffset + m_vTipBeam ;
			float32_t	fpLength =
							esl_fmaxf( m_fpBeamLength - m_bpBeam.fpOffset,
													m_bpBeam.fpMinLength ) ;
			float32_t	fpThickness = m_bpBeam.fpThickness * 0.5f ;
			//
			vPoint[0] = S3DVector4( vBase + vRDir * fpLength, 0 ) ;
			vPoint[1] = S3DVector4( vBase + vTDir * fpThickness, 0 ) ;
			vPoint[2] = S3DVector4( vBase - vTDir * fpThickness, 0 ) ;
			//
			vNormal[0] = S3DVector4( vRDir * vTDir, 0 ) ;
			vNormal[1] = vNormal[0] ;
			vNormal[2] = vNormal[0] ;
			//
			render.AddTemporaryObject( pMaterial ) ;
			render.AddIndexedPrimitiveList
				( pMaterial, RenderContext::renderUncombinable,
					primitiveTriangle, 3, 3,
					vPoint, vNormal, nullptr, nullptr, nIndex ) ;
		}
	}
	else
	{
		Item::RenderLocalModel( scene, render, flagsExclusion ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// VR コントローラーアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DVRHandController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DVRHandController, vr_hand_controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DVRHandController::S3DVRHandController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID )
{
	m_pInput = NULL ;
	m_pVR = NULL ;
	m_iHand = SGLVRViewProducer::handRight ;
	m_iController = SGLVRViewProducer::controllerRight ;
	m_maskPressed = 0 ;
	m_flagVirtualMouse = false ;
	m_flagVisibleMouse = false ;
	m_mppMousePointer.idVirtualMouse = 1 ;
	m_mppMousePointer.nLeftButtonEntries = 1 ;
	m_mppMousePointer.iLeftMouseButton[0] = SGLVRViewProducer::button1 ;
	m_mppMousePointer.nRightButtonEntries = 1 ;
	m_mppMousePointer.iRightMouseButton[0] = SGLVRViewProducer::button2 ;
	m_mppMousePointer.vPointerArrow = S3DVector( 0, 0, 1 ) ;
	//
	m_sprVirtualCursor.ModifyUIFlag
		( SGLSprite::uiDisabled
			| SGLSprite::uiUnclickable, SGLSprite::uiHaveFocus ) ;
	m_sprVirtualCursor.ChangePriority( -0x7FFFFFFF ) ;

	m_pFocusSprite = NULL ;
	m_pCamera = NULL ;
}

// 出力先入力キュー設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::AttachVirtualInput( SGLVirtualInput * pInput )
{
	m_pInput = pInput ;
}

// VR 設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::SetVRHand
	( SGLVRViewProducer * pVR,
		SGLVRViewProducer::HandIndex iHand,
		SGLVRViewProducer::ControllerIndex iController )
{
	m_pVR = pVR ;
	m_iHand = iHand ;
	m_iController = iController ;
}

// 仮想マウス設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::SetVirtualMouse
		( const S3DVRHandController::MousePointerParam& mpp )
{
	m_mppMousePointer = mpp ;
}

void S3DVRHandController::GetVirtualMouse( S3DVRHandController::MousePointerParam& mpp )
{
	mpp = m_mppMousePointer ;
}

void S3DVRHandController::EnableVirtualMouse( bool fVirtualMouse )
{
	m_flagVirtualMouse = fVirtualMouse ;
}

bool S3DVRHandController::IsEnabledVirtualMouse( void ) const
{
	return	m_flagVirtualMouse ;
}

void S3DVRHandController::SetVisibleMouseCursor( bool flagVisible )
{
	m_flagVisibleMouse = flagVisible ;
}

// 仮想マウスカーソル画像設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::AttachCursorImage( SGLImageObject * pImage )
{
	m_sprVirtualCursor.AttachImage( pImage ) ;
}

void S3DVRHandController::AttachDefaultCursor( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	SGLImageBuffer *	pimgCursor =
			SGLSpriteCursor::ConvertHCURSORtoImageBuffer
							( ::LoadCursor( NULL, IDC_ARROW ) ) ;
	if ( pimgCursor != NULL )
	{
		m_imgDefaultCursor.CreateCloneBuffer( *pimgCursor ) ;
		m_sprVirtualCursor.AttachImage( &m_imgDefaultCursor ) ;
		sglReleaseImageBuffer( pimgCursor ) ;
	}
#endif
}

// ボタンイベントマッピング
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVRHandController::SetButtonEvent
	( int iButton, const SGLVirtualInput::InputEvent& evOut )
{
	if ( (iButton < 0) || (iButton >= 64) )
	{
		return	sglErrFailed ;
	}
	m_csEventMap.Lock() ;
	m_aButtonEvents.SetAt
		( (size_t) iButton, new SGLVirtualInput::InputEvent( evOut ) ) ;
	m_csEventMap.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DVRHandController::GetButtonEvent
	( int iButton, SGLVirtualInput::InputEvent& evOut ) const
{
	if ( (iButton < 0) || (iButton >= 64) )
	{
		return	sglErrFailed ;
	}
	m_csEventMap.Lock() ;
	SGLVirtualInput::InputEvent *
			pEvent = m_aButtonEvents.GetAt( (size_t) iButton ) ;
	if ( pEvent == NULL )
	{
		m_csEventMap.Unlock() ;
		return	sglErrFailed ;
	}
	evOut = *pEvent ;
	m_csEventMap.Unlock() ;
	return	sglErrSuccess ;
}

SGLError S3DVRHandController::RemoveButtonEvent( int iButton )
{
	if ( (iButton < 0) || (iButton >= 64) )
	{
		return	sglErrFailed ;
	}
	m_csEventMap.Lock() ;
	m_aButtonEvents.SetAt( (size_t) iButton, NULL ) ;
	m_csEventMap.Unlock() ;
	return	sglErrSuccess ;
}

// 2Dレイヤー基準カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::SetCameraForLayer2D( S3DScene::Camera * pCamera )
{
	m_pCamera = pCamera ;
}

// 2D表示レイヤ情報設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::SetLayer2DEntry( const Layer2DEntry& layer2D )
{
	Lock() ;
	ssize_t	i = FindLayer2DEntry( layer2D.pSprite ) ;
	if ( i < 0 )
	{
		i = (ssize_t) m_aLayer2Ds.GetLength() ;
	}
	m_aLayer2Ds.SetAt( (size_t) i, layer2D ) ;
	Unlock() ;
}

// 2D表示レイヤ情報計算
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::CalcLayer2DEntry
	( S3DVRHandController::Layer2DEntry& layer2D,
		SGLVRViewProducer * pVR,
		SGLSprite * pLayer2D,
		const S3DVector& vScreenDstPos,
		const S2DVector& vScreenCenter,
		const S2DVector& vScreenZoom )
{
	const SGLVRViewProducer::EyeFieldOfView&
			fovHMD = pVR->GetEyeFieldOfView( SGLVRViewProducer::eyeRight ) ;
	double	xSpaceScale = vScreenZoom.x * vScreenDstPos.z / fovHMD.vScreenPos.z ;
	double	ySpaceScale = vScreenZoom.y * vScreenDstPos.z / fovHMD.vScreenPos.z ;
	layer2D.pSprite = pLayer2D ;
	layer2D.vOrigin.x = - vScreenCenter.x * xSpaceScale ;
	layer2D.vOrigin.y = - vScreenCenter.y * ySpaceScale ;
	layer2D.vOrigin.z = vScreenDstPos.z ;
	layer2D.vAxisX = S3DDVector( xSpaceScale, 0, 0 ) ;
	layer2D.vAxisY = S3DDVector( 0, ySpaceScale, 0 ) ;
}

// 2D表示レイヤ情報削除
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::RemoveLayer2DEntry( SGLSprite * pLayer2D )
{
	Lock() ;
	ssize_t	i = FindLayer2DEntry( pLayer2D ) ;
	if ( i >= 0 )
	{
		m_aLayer2Ds.RemoveAt( (size_t) i ) ;
	}
	Unlock() ;
}

// 全2D表示レイヤ情報削除
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::RemoveAllLayer2DEntries( void )
{
	Lock() ;
	m_aLayer2Ds.RemoveAll() ;
	Unlock() ;
}

// 2D表示レイヤ情報検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DVRHandController::FindLayer2DEntry( SGLSprite * pLayer2D ) const
{
	const Layer2DEntry *	pEntries = m_aLayer2Ds.GetConstArray() ;
	const size_t			nCount = m_aLayer2Ds.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pEntries[i].pSprite == pLayer2D )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 2D表示レイヤ当たり判定
//////////////////////////////////////////////////////////////////////////////
SGLSprite * S3DVRHandController::GetHitRayForLayer2D
	( const S3DScene& scene,
		S2DDVector& vPos, double& zDistance,
		const S3DDVector& vRay, const S3DDVector& vOrigin ) const
{
	//
	// カメラによる変換
	//
	S3DScene::Camera *	pCamera = m_pCamera ;
	if ( pCamera == NULL )
	{
		pCamera = scene.GetCurrentCamera() ;
		if ( pCamera == NULL )
		{
			pCamera = scene.GetMainCamera() ;
		}
	}
	S3DDVector	vTRay = vRay ;
	S3DDVector	vTOrg = vOrigin ;
	//
	if ( pCamera != NULL )
	{
		S3DDMatrix	matCamera( 1, 1, 1 ) ;
		S3DDVector	vCamera( 0, 0, 0 ) ;
		S3DScene::CalcCameraTransformation( matCamera, vCamera, pCamera ) ;
		//
		vTRay = matCamera * vTRay ;
		vTOrg = matCamera * vTOrg - vCamera ;
	}
	vTRay.Normalize() ;
	//
	// 当たり判定
	//
	const Layer2DEntry *	pEntries = m_aLayer2Ds.GetConstArray() ;
	const size_t			nCount = m_aLayer2Ds.GetLength() ;
	SGLSprite *				pHitSprite = NULL ;
	double					zHitMin = 0.0f ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const Layer2DEntry&	layer = pEntries[i] ;
		//
		// 基底ベクトル
		//
		S3DDVector	vX = layer.vAxisX ;
		S3DDVector	vY = layer.vAxisY ;
		S3DDVector	vO = layer.vOrigin - vTOrg ;
		//
		// 平面との交点
		//
		S3DDVector	vXY = vX * vY ;
		double		s = vXY.Absolute() ;
		vXY.Normalize() ;
		//
		double	r = (vTRay | vXY) ;
		if ( fabs( r ) < 1.0e-8 )
		{
			continue ;
		}
		double	zHit = (vO | vXY) / r ;
		//
		if ( (zHit < 0.0) || ((pHitSprite != NULL) && (zHitMin < zHit)) )
		{
			// 既にヒットしている Sprite より奥／又は基点より逆方向
			continue ;
		}
		S3DDVector	vHitPos = vTRay * zHit + vTOrg ;
		//
		// 平面座標系へ変換
		//
		S3DDVector	vLocal = vHitPos - layer.vOrigin ;
		S2DDVector	vLocal2D ;
		SGLSprite *	pSprite = layer.pSprite ;
		vLocal2D.x = vXY.InnerProduct( vLocal * vY ) / s ;
		vLocal2D.y = vXY.InnerProduct( vX * vLocal ) / s ;
		ESLAssert( pSprite != NULL ) ;
		pSprite->SGLSprite::GlobalToLocal( vLocal2D ) ;
		//
		// スプライト内でのヒットテスト
		//
//		S2DDVector	vTemp = vLocal2D ;
//		if ( pSprite->GetHitSpriteAt( vTemp ) != NULL )
		{
			pHitSprite = pSprite ;
			zHitMin = zHit ;
			//
			vPos = vLocal2D ;
			zDistance = zHit ;
		}
	}
	return	pHitSprite ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DVRHandController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;
	//
	if ( m_pVR == NULL )
	{
		return ;
	}
	//
	// VR デバイス状態取得
	//
	const uint64_t				maskLastPressed = m_maskPressed ;
	SGLVRViewProducer::Posture	poseHand ;
	m_pVR->LockForDeviceState() ;
	m_maskPressed =
		m_pVR->GetControllerState(m_iController).maskButtonPressed ;
	poseHand = m_pVR->GetHandPosture( m_iHand ) ;
	m_pVR->UnlockForDeviceState() ;
	if ( m_maskPressed ^ maskLastPressed )
	{
		ESLTrace( "VR button (%d): %08X\n", m_iController, (DWORD) m_maskPressed ) ;
	}
	//
	ESLAssert( pItem != NULL ) ;
	S3DSceneComposer::CommonSerializer *	pcsItem =
		ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
	if ( pcsItem == NULL )
	{
		return ;
	}
	//
	// 座標・姿勢更新
	//
	bool	flagTrackedOrientation = false ;
	bool	flagTrackedPosition = false ;
	if ( poseHand.nFlags & SGLVRViewProducer::trackedOrientation )
	{
		S3DDMatrix	matRotation = poseHand.matOrientation ;
		pcsItem->SetMatrixParameter
				( pcsItem->paramRotation, matRotation ) ;
		flagTrackedOrientation = true ;
	}
	if ( poseHand.nFlags & SGLVRViewProducer::trackedPosition )
	{
		S3DDVector	vHandPos = poseHand.vPosition ;
		pcsItem->SetVectorParameter
				( pcsItem->paramPosition, vHandPos ) ;
		flagTrackedPosition = true ;
	}
	if ( m_flagVirtualMouse && flagTrackedOrientation && flagTrackedPosition )
	{
		//
		// ポインタ当たり判定
		//
		SGLSprite *	pHitSprite = NULL ;
		S2DDVector	vHitPos ;
		double		zDistance ;
		//
		S3DDMatrix	matItem ;
		S3DDVector	vItem ;
		pcsItem->GetGlobalTransformation( matItem, vItem ) ;
		//
		SGLVRViewProducer::ModelInfo	miModel ;
		S3DDMatrix	matTip( 1, 1, 1 ) ;
		S3DDVector	vTip( 0, 0, 0 ) ;
		if ( !m_pVR->GetControllerModel( miModel, m_iController ) )
		{
			if ( miModel.postureTipLocal.nFlags
						& SGLVRViewProducer::trackedOrientation )
			{
				matTip = miModel.postureTipLocal.matOrientation ;
			}
			if ( miModel.postureTipLocal.nFlags
						& SGLVRViewProducer::trackedPosition )
			{
				vTip = miModel.postureTipLocal.vPosition ;
			}
		}
		//
		S3DDVector	vPointerRay = m_mppMousePointer.vPointerArrow ;
		S3DDVector	vPointerOrg = vItem ;
		vPointerRay = matItem * matTip * vPointerRay ;
		vPointerOrg += matItem * vTip ;
		//
		S3DSceneSprite *
			pSceneSprite = ESLTypeCast<S3DSceneSprite>( &scene ) ;
		if ( pSceneSprite != NULL )
		{
			// S3DSceneItemSprite との当たり判定
			pHitSprite =
				pSceneSprite->GetHitRayForSprite
					( vHitPos, zDistance, vPointerRay, vPointerOrg, false ) ;
		}
		Lock() ;
		S2DDVector	vHitLayer2D ;
		double		zDistance2D ;
		SGLSprite *	pLayer2D =
			GetHitRayForLayer2D
				( scene, vHitLayer2D, zDistance2D, vPointerRay, vPointerOrg ) ;
		if ( pLayer2D != NULL )
		{
			// 2D表示レイヤにヒット
			if ( (pHitSprite == NULL) || (zDistance2D < zDistance) )
			{
				pHitSprite = pLayer2D ;
				vHitPos = vHitLayer2D ;
				zDistance = zDistance2D ;
			}
		}
		S3DVRControllerModelSerializer *
			pCtrlModel = ESLTypeCast<S3DVRControllerModelSerializer>( pItem ) ;
		if ( pCtrlModel != nullptr )
		{
			if ( pHitSprite != nullptr )
			{
				pCtrlModel->SetBeamLength
					( esl_fminf( (float32_t) zDistance,
									pCtrlModel->GetBeamParam().fpDefaultLength ) ) ;
			}
			else
			{
				pCtrlModel->SetBeamLength
					( pCtrlModel->GetBeamParam().fpDefaultLength ) ;
			}
		}
		Unlock() ;
		//
		// マウス識別子
		//
		int64_t	nMouseFlags =
					m_mppMousePointer.idVirtualMouse
								& SGLSprite::MouseIDMask
					| SGLSprite::VirtualMouseFlag ;
		//
		// マウスポインタ移動
		//
		if ( pHitSprite != m_pFocusSprite )
		{
			if ( m_pFocusSprite != NULL )
			{
				m_pFocusSprite->OnMouseLeave( nMouseFlags ) ;
				if ( m_sprVirtualCursor.GetParent() != nullptr )
				{
					m_pFocusSprite->DetachChild( &m_sprVirtualCursor ) ;
				}
			}
			if ( m_flagVisibleMouse && (pHitSprite != NULL) )
			{
				pHitSprite->AddChild( &m_sprVirtualCursor ) ;
			}
			m_pFocusSprite = pHitSprite ;
		}
		m_sprVirtualCursor.SetPosition( vHitPos.x, vHitPos.y ) ;
		//
		if ( pHitSprite != NULL )
		{
			pHitSprite->OnMouseMove( vHitPos.x, vHitPos.y, nMouseFlags ) ;
		}
		//
		// 左ボタン相当押下テスト
		//
		for ( size_t i = 0; i < m_mppMousePointer.nLeftButtonEntries; i ++ )
		{
			MouseButtonEvent
				( vHitPos, maskLastPressed,
					m_mppMousePointer.iLeftMouseButton[i],
					SGLSprite::LeftButtonID, vkeyMouseLeft ) ;
		}
		//
		// 右ボタン相当押下テスト
		//
		for ( size_t i = 0; i < m_mppMousePointer.nRightButtonEntries; i ++ )
		{
			MouseButtonEvent
				( vHitPos, maskLastPressed,
					m_mppMousePointer.iRightMouseButton[i],
					SGLSprite::RightButtonID, vkeyMouseRight ) ;
		}
	}
	//
	// ボタンイベント処理
	//
	const uint64_t	maskChangePressed = m_maskPressed ^ maskLastPressed ;
	for ( size_t i = 0; i < m_aButtonEvents.GetLength(); i ++ )
	{
		const uint64_t	bitButton = ((uint64_t)1) << i ;
		if ( !(maskChangePressed & bitButton) )
		{
			continue ;
		}
		SGLVirtualInput::InputEvent	iev ;
		if ( GetButtonEvent( (int) i, iev ) )
		{
			continue ;
		}
		if ( m_pInput != NULL )
		{
			if ( m_maskPressed & bitButton )
			{
				m_pInput->PressInputEvent( iev ) ;
			}
			else
			{
				m_pInput->ReleaseInputEvent( iev ) ;
			}
		}
	}
}

void S3DVRHandController::MouseButtonEvent
	( const S2DDVector& vPos,
		uint64_t maskLastPressed, int iVRButton,
		int idMouseButton, int vkeyMouseButton ) const
{
	SGLVirtualInput::InputEvent	iev ;
	iev.typeDevice = SGLVirtualInput::deviceMouse ;
	iev.numDevice = 0 ;
	iev.codeKey = vkeyMouseButton ;
	//
	const uint64_t	maskButton = ((uint64_t)1) << iVRButton ;
	if ( (maskLastPressed ^ m_maskPressed) & maskButton )
	{
		int64_t	nMouseFlags =
					m_mppMousePointer.idVirtualMouse
								& SGLSprite::MouseIDMask
					| SGLSprite::VirtualMouseFlag ;
		if ( m_maskPressed & maskButton )
		{
			bool	fProcessed = false ;
			if ( m_pFocusSprite != NULL )
			{
				fProcessed =
					m_pFocusSprite->OnButtonDown
						( vPos.x, vPos.y, nMouseFlags
								| (idMouseButton
									<< SGLSprite::ButtonIDShifter) ) ;
			}
			if ( !fProcessed && (m_pInput != NULL) )
			{
				m_pInput->PressInputEvent( iev ) ;
			}
		}
		else
		{
			bool	fProcessed = false ;
			if ( m_pFocusSprite != NULL )
			{
				fProcessed =
					m_pFocusSprite->OnButtonUp
						( vPos.x, vPos.y, nMouseFlags
								| (idMouseButton
									<< SGLSprite::ButtonIDShifter) ) ;
			}
			if ( !fProcessed && (m_pInput != NULL) )
			{
				m_pInput->ReleaseInputEvent( iev ) ;
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// VR コントローラー表示アイテム空間セット
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DVRItemSetSpace, S3DCameraRelativeSpace ) 
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DVRItemSetSpace, vr_item_set_space )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DVRItemSetSpace::S3DVRItemSetSpace( void )
	: m_pVR( nullptr )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i] = new S3DVRHandController ;
		m_modelController[i].AddController( m_pctrlHand[i] ) ;
		AddItem( &m_modelController[i] ) ;
	}
}

// VR 設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetVRController( SGLVRViewProducer * pVR )
{
	Lock() ;
	m_pVR = pVR ;
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_modelController[i].SetVRController
			( pVR, (SGLVRViewProducer::HandIndex) i,
				(SGLVRViewProducer::ControllerIndex) i ) ;
		m_pctrlHand[i]->SetVRHand
			( pVR, (SGLVRViewProducer::HandIndex) i,
				(SGLVRViewProducer::ControllerIndex) i ) ;
	}
	Unlock() ;
}

void S3DVRItemSetSpace::DetachVRController( void )
{
	SetVRController( nullptr ) ;
}

// 参照カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetRelativeCamera
	( S3DScene::Camera * pCamera,
		const wchar_t * pwszCameraID, bool flagModifiedCameraSpace )
{
	S3DCameraRelativeSpace::SetRelativeCamera
		( pCamera, pwszCameraID, flagModifiedCameraSpace ) ;
	//
	S3DCameraRelationController *
				pRelCamera = m_pctrlRelCamera.GetReference() ;
	if ( pRelCamera != NULL )
	{
		S3DSceneComposer::ItemSerializer *
							pItem = pRelCamera->GetOwnerItem() ;
		if ( pItem != NULL )
		{
			pItem->RemoveControllerAt
				( (size_t) pItem->FindController( pRelCamera ) ) ;
		}
		m_pctrlRelCamera.SetReference( NULL ) ;
	}
	S3DDynamicCamera *
			pDynCamera = ESLTypeCast<S3DDynamicCamera>( pCamera ) ;
	if ( pDynCamera != NULL )
	{
		pRelCamera = new S3DCameraRelationController ;
		pRelCamera->AttachCameraRelativeSpace( this ) ;
		pDynCamera->AddController( pRelCamera ) ;
		m_pctrlRelCamera.SetReference( pRelCamera ) ;
	}
}

void S3DVRItemSetSpace::DetachRelativeCamera( void )
{
	SetRelativeCamera( nullptr ) ;
}

// 座標更新
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::GetReferenceCameraTransformation
	( S3DDMatrix& matCamera, S3DDVector& vCamera,
					const S3DScene::Camera * pCamera )
{
	const S3DDynamicCamera *
			pDynCamera = ESLTypeCast<S3DDynamicCamera>( pCamera ) ;
	if ( pDynCamera != nullptr )
	{
		S3DCameraHMDPosition *	pHMDPos =
			pDynCamera->GetController<S3DCameraHMDPosition>() ;
		if ( pHMDPos != nullptr )
		{
			S3DDMatrix	matCameraSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			S3DDVector	posCameraSpace( 0, 0, 0 ) ;
			pCamera->CalcItemLinkTransformation( matCameraSpace, posCameraSpace ) ;
			//
			S3DDVector	vCameraPos = pCamera->GetCameraPosition() ;
			S3DDVector	vCameraTarget = pCamera->GetCameraTarget() ;
			S3DDVector	vCameraTop = pCamera->GetCameraTop() ;
//			if ( m_flagModifiedCamera )
			{
				if ( !pHMDPos->GetLastModifiedCameraExceptVR
						( vCameraPos, vCameraTarget, vCameraTop ) )
				{
					vCameraPos = pCamera->m_space.m_vCenter ;
					vCameraTarget = pCamera->m_vTarget ;
					vCameraTop =pCamera->m_vTop ;
				}
			}
			if ( pHMDPos->GetControllFlags()
					& S3DCameraHMDPosition::flagExtraHMDLevel )
			{
				double	yHMD = ((m_pVR != nullptr)
								&& (m_pVR->GetViewSpaceType()
										== SGLVRViewProducer::spaceEyeLevel))
								? pHMDPos->GetHMDExtraY() : 0.0 ;
				double	yOffset = yHMD - vCameraPos.y ;
				vCameraPos.y = yHMD ;
				vCameraTarget.y += yOffset ;
			}
			if ( pHMDPos->GetControllFlags()
					& S3DCameraHMDPosition::flagLevelMatching )
			{
				vCameraTarget.y = vCameraPos.y ;
				vCameraTop.x = 0.0 ;
				vCameraTop.y = (vCameraTop.y < 0.0) ? -1.0 : 1.0 ;
				vCameraTop.z = 0.0 ;
			}
			//
			vCameraTarget = matCameraSpace * vCameraTarget + posCameraSpace ;
			vCameraPos = matCameraSpace * vCameraPos + posCameraSpace ;
			vCameraTop = matCameraSpace * vCameraTop ;
			//
			vCamera = matCamera.CameraAngleOf
							( vCameraTarget, vCameraPos, vCameraTop ) ;
			return ;
		}
	}
	S3DCameraRelativeSpace::
		GetReferenceCameraTransformation( matCamera, vCamera, pCamera ) ;
}

// コントローラー表示クラス
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetControllerRenderClass( int iHand, S3DScene::ItemClass cls )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_modelController[iHand].m_classItem = cls ;
	}
}

S3DScene::ItemClass S3DVRItemSetSpace::GetControllerRenderClass( int iHand ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_modelController[iHand].m_classItem ;
	}
	return	S3DScene::classEffect2 ;
}

// コントローラー表示
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetVisibleController( int iHand, bool fVisible )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		if ( fVisible )
		{
			m_modelController[iHand].m_flagsBehavior |= S3DScene::itemVisible ;
		}
		else
		{
			m_modelController[iHand].m_flagsBehavior &= ~S3DScene::itemVisible ;
		}
	}
}

bool S3DVRItemSetSpace::IsVisibleController( int iHand ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	(m_modelController[iHand].m_flagsBehavior & S3DScene::itemVisible) != 0 ;
	}
	return	false ;
}

// 仮想マウス設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVRItemSetSpace::SetVirtualMouse
	( int iHand, const S3DVRHandController::MousePointerParam& mpp )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_pctrlHand[iHand]->SetVirtualMouse( mpp ) ;
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

SGLError S3DVRItemSetSpace::GetVirtualMouse
	( int iHand, S3DVRHandController::MousePointerParam& mpp )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_pctrlHand[iHand]->GetVirtualMouse( mpp ) ;
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

SGLError S3DVRItemSetSpace::EnableVirtualMouse( int iHand, bool fVirtualMouse )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_pctrlHand[iHand]->EnableVirtualMouse( fVirtualMouse ) ;
		if ( fVirtualMouse )
		{
			m_pctrlHand[iHand]->AttachDefaultCursor() ;
		}
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

bool S3DVRItemSetSpace::IsEnabledVirtualMouse( int iHand ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_pctrlHand[iHand]->IsEnabledVirtualMouse() ;
	}
	else
	{
		return	false ;
	}
}

void S3DVRItemSetSpace::SetVisibleMouseCursor( int iHand, bool flagVisible )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_pctrlHand[iHand]->SetVisibleMouseCursor( flagVisible ) ;
	}
}

// ポインタ用ビーム設定
//////////////////////////////////////////////////////////////////////////////
const S3DVRItemSetSpace::BeamParam& S3DVRItemSetSpace::GetPointerBeamParam( int iHand ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	return	m_modelController[iHand].GetBeamParam() ;
}

void S3DVRItemSetSpace::SetPointerBeamParam( int iHand, const S3DVRItemSetSpace::BeamParam& bp )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_modelController[iHand].SetBeamParam( bp ) ;
	}
}

bool S3DVRItemSetSpace::IsPointerBeamVisible( int iHand ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_modelController[iHand].IsBeamVisible() ;
	}
	else
	{
		return	false ;
	}
}

void S3DVRItemSetSpace::SetPointerBeamVisible( int iHand, bool flagVisible )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_modelController[iHand].SetBeamVisible( flagVisible ) ;
	}
}

// 出力先入力キュー設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::AttachVirtualInput( SGLVirtualInput * pInput )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i]->AttachVirtualInput( pInput ) ;
	}
}

// ボタンイベントマッピング
//////////////////////////////////////////////////////////////////////////////
SGLError S3DVRItemSetSpace::SetButtonEvent
	( int iHand, int iButton, const SGLVirtualInput::InputEvent& evOut )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		m_pctrlHand[iHand]->SetButtonEvent( iButton, evOut ) ;
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

SGLError S3DVRItemSetSpace::GetButtonEvent
	( int iHand, int iButton, SGLVirtualInput::InputEvent& evOut ) const
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_pctrlHand[iHand]->GetButtonEvent( iButton, evOut ) ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

SGLError S3DVRItemSetSpace::RemoveButtonEvent( int iHand, int iButton )
{
	ESLAssert( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) ) ;
	if ( (iHand >= 0) && (iHand < SGLVRViewProducer::handCount) )
	{
		return	m_pctrlHand[iHand]->RemoveButtonEvent( iButton ) ;
	}
	else
	{
		return	sglErrInvalidParam ;
	}
}

// 2Dレイヤー基準カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetCameraForLayer2D( S3DScene::Camera * pCamera )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i]->SetCameraForLayer2D( pCamera ) ;
	}
}

// 2D表示レイヤ情報設定
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::SetLayer2DEntry
		( const S3DVRHandController::Layer2DEntry& layer2D )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i]->SetLayer2DEntry( layer2D ) ;
	}
}

// 2D表示レイヤ情報削除
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::RemoveLayer2DEntry( SGLSprite * pLayer2D )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i]->RemoveLayer2DEntry( pLayer2D ) ;
	}
}

// 全2D表示レイヤ情報削除
//////////////////////////////////////////////////////////////////////////////
void S3DVRItemSetSpace::RemoveAllLayer2DEntries( void )
{
	for ( int i = 0; i < SGLVRViewProducer::handCount; i ++ )
	{
		m_pctrlHand[i]->RemoveAllLayer2DEntries() ;
	}
}


