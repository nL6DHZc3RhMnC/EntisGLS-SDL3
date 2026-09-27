
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


////////////////////////////////////////////////////////////////////////////////////////
// キャンバス・アイテム
////////////////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DCanvasSerializer::m_paramEntries[S3DCanvasSerializer::paramCanvasCount] =
{
	{ L"canvas_width",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"キャンバス幅", nullptr },
	{ L"canvas_height",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"キャンバス高", nullptr },
	{ L"canvas_framebuffer",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"フレームバッファ", nullptr },
	{ L"canvas_back_alpha",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"フレームバッファ背景α", nullptr, 0.0, 1.0 },
	{ L"canvas_center",
		S3DSceneComposer::typeVector2,
		S3DSceneComposer::attrConstant1,
		L"基準座標", nullptr },
	{ L"coords_mode",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"表示モード", nullptr },
	{ L"depth_mask_op",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"表示モード", nullptr },
	{ L"view3d_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"表示スケール", L"単位座標当たりのピクセル数を指定します" },
	{ L"view2d_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"表示視野角", nullptr, 1.0, 150.0 },
	{ L"fit_view_angle",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"表示視野角を合わせる", nullptr },
	{ L"fit_view_pos",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"位置を2Dと3Dが一致するように変更する", nullptr },
} ;

const S3DSceneComposer::ParamSetClass	S3DCanvasSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DCanvasSerializer::paramCanvasCount,
	&S3DCanvasSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
		S3DCanvasSerializer::m_aiCoordsModes[3] =
{
	{ L"frame_3d", S3DSceneItemSprite::coordinatesFrame3D },
	{ L"view_2d", S3DSceneItemSprite::coordinatesDirect2D },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
		S3DCanvasSerializer::m_aiDepthMaskOps[6] =
{
	{ L"default", S3DRenderContextInterface::depthMaskDefault },
	{ L"write", S3DRenderContextInterface::depthMaskEnable },
	{ L"no_write", S3DRenderContextInterface::depthMaskNoWrite },
	{ L"no_write_gt", S3DRenderContextInterface::depthMaskNoWriteGT },
	{ L"no_test", S3DRenderContextInterface::depthMaskNoTest },
	{ nullptr, 0 },
} ;

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasSerializer::ImageDrawer, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasSerializer::ImageEffector, ESLObject )
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DCanvasSerializer, S3DSceneItemSprite, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasSerializer, canvas )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasSerializer::S3DCanvasSerializer( void )
	: ItemCommonSerializer
			( m_ItemClassDescriptor.pwszClassID,
						&S3DCanvasSerializer::m_pscClass, nullptr ),
		m_sizeCanvas( 1280, 720 ),
		m_flagFrameBuffer( true ),
		m_flagStereoBuffer( false ),
		m_nFrameBackAlpha( 0 ),
		m_vCenterOffset( 0.0, 0.0 ),
		m_fp3DViewScale( 100.0 ), m_deg2DViewAngle( 60.0 )
{
	AttachSceneItem( (S3DSceneItemSprite*) this ) ;
	m_flagsBehavior |= S3DScene::itemTimer ;
	m_classItem = S3DScene::classEffect3 ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasSerializer::~S3DCanvasSerializer( void )
{
}

// キャンバスサイズ取得
////////////////////////////////////////////////////////////////////////////////////////
const SGLSize& S3DCanvasSerializer::GetCanvasSize( void ) const
{
	return	m_sizeCanvas ;
}

// ステレオバッファ設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::SetFrameStereoBuffer( bool flagStereo )
{
	if ( m_flagStereoBuffer != flagStereo )
	{
		m_flagStereoBuffer = flagStereo ;
		UpdateFrameBuffer() ;
	}
}

// フレームバッファ設定反映
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::UpdateFrameBuffer( void )
{
	if ( m_flagFrameBuffer )
	{
		SGLSprite::Buffer *	pBuffer = GetFrameBuffer() ;
		if ( pBuffer != nullptr )
		{
			if ( (GetImageSize() != m_sizeCanvas)
				|| (pBuffer->IsStereo3D() != m_flagStereoBuffer) )
			{
				ReleaseBuffer() ;
			}
		}
		if ( !IsBuffered() )
		{
			CreateBuffer
				( m_sizeCanvas.w, m_sizeCanvas.h,
					formatImageDefaultRGBA, 32,
					SGLImageObject::bufferOnDeviceOnly,
					false, m_flagStereoBuffer ) ;
			SetFillBackColor( (((uint32_t)m_nFrameBackAlpha) << 24), true ) ;
			PostUpdate( nullptr ) ;
		}
	}
	else
	{
		if ( IsBuffered() )
		{
			ReleaseBuffer() ;
		}
	}
}

// 中心座標反映
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::UpdateCenterPosition( void )
{
	SetCenterPosition
		( m_sizeCanvas.w * 0.5 + m_vCenterOffset.x,
			m_sizeCanvas.h * 0.5 + m_vCenterOffset.y ) ;
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCanvasBackAlpha:
		return	(double) m_nFrameBackAlpha / 255.0 ;

	case	paramCanvas3DScale:
		return	m_fp3DViewScale ;

	case	param2DViewAngle:
		return	m_deg2DViewAngle ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

int32_t S3DCanvasSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCanvasWidth:
		return	m_sizeCanvas.w ;

	case	paramCanvasHeight:
		return	m_sizeCanvas.h ;
	}
	return	ItemCommonSerializer::GetIntegerParameter( i ) ;
}

bool S3DCanvasSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCanvasFrameBuffer:
		return	m_flagFrameBuffer ;

	case	paramCmdFit2DViewAngle:
		return	false ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DCanvasSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCoordnatesMode:
		return	SXMLDocument::GetSymbolAsIntegerOf
					( m_aiCoordsModes, GetCoordinatesMode() ) ;

	case	paramDepthMaskOp:
		return	SXMLDocument::GetSymbolAsIntegerOf
					( m_aiDepthMaskOps, GetDepthMaskOperation() ) ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

size_t S3DCanvasSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramCanvasCenterOffset:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_vCenterOffset ;
			return	sizeof(S2DDVector) ;
		}
		return	0 ;
	}
	return	ItemCommonSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::SetScalarParameter( size_t i, double s )
{
	uint32_t	argbBack = 0 ;
	switch ( i )
	{
	case	paramCanvasBackAlpha:
		m_nFrameBackAlpha = esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 255 ) ;
		SetFillBackColor( ((uint32_t)m_nFrameBackAlpha) << 24, true ) ;
		PostUpdate( nullptr ) ;
		return ;

	case	paramCanvas3DScale:
		m_fp3DViewScale = s ;
		UpdateSpaceMatrix() ;
		return ;

	case	param2DViewAngle:
		m_deg2DViewAngle = s ;
		Set2DViewHalfAngle( tan( m_deg2DViewAngle * PI / 360.0 ) ) ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DCanvasSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramCanvasWidth:
		m_sizeCanvas.w = n ;
		Set2DViewWidthPixels( m_sizeCanvas.w ) ;
		return ;

	case	paramCanvasHeight:
		m_sizeCanvas.h = n ;
		return ;
	}
	ItemCommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DCanvasSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCanvasFrameBuffer:
		m_flagFrameBuffer = b ;
		return ;

	case	paramCmdFit2DViewAngle:
		CmdFit2DViewAngle() ;
		return ;

	case	paramCmdFit2DViewPos:
		CmdFit2DViewPosition() ;
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DCanvasSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramCoordnatesMode:
		SetCoordinatesMode
			( (CoordinatesMode) 
				SXMLDocument::GetIntegerAsSymbolOf
					( m_aiCoordsModes, pwszCmd, GetCoordinatesMode() ) ) ;
		UpdateSpaceMatrix() ;
		return ;

	case	paramDepthMaskOp:
		SetDepthMaskOperation
			( (S3DRenderContextInterface::DepthMaskOperation)
				SXMLDocument::GetIntegerAsSymbolOf
					( m_aiDepthMaskOps, pwszCmd, GetDepthMaskOperation() ) ) ;
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DCanvasSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramCanvasCenterOffset:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_vCenterOffset = *((const S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		return	0 ;
	}
	return	ItemCommonSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramCoordnatesMode:
		for ( j = 0; m_aiCoordsModes[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( m_aiCoordsModes[j].pszSymbol ) ) ;
		}
		return	true ;

	case	paramDepthMaskOp:
		for ( j = 0; m_aiDepthMaskOps[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( m_aiDepthMaskOps[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
		return	false ;

	case	paramDepthMaskOp:
		return	!m_flagFrameBuffer
				&& (GetCoordinatesMode() == coordinatesFrame3D) ;

	case	paramCanvas3DScale:
		return	GetCoordinatesMode() == coordinatesFrame3D ;

	case	param2DViewAngle:
	case	paramCmdFit2DViewAngle:
		return	GetCoordinatesMode() == coordinatesDirect2D ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
////////////////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCanvasSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"キャンバス" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// 行列設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::SetItemMatrix( const S3DDMatrix& matrix )
{
	ItemCommonSerializer::SetItemMatrix( matrix ) ;
	//
	if ( GetCoordinatesMode() == coordinatesFrame3D )
	{
		SpaceParameter().m_matTransformation /= m_fp3DViewScale ;
	}
}

// 変換行列更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::UpdateSpaceMatrix( void )
{
	ItemCommonSerializer::UpdateSpaceMatrix() ;
	//
	if ( GetCoordinatesMode() == coordinatesFrame3D )
	{
		SpaceParameter().m_matTransformation /= m_fp3DViewScale ;
	}
}

// アイテム作用の追加処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::OnUpdateBehavior( S3DScene& scene )
{
	UpdateFrameBuffer() ;
	UpdateCenterPosition() ;
	//
	if ( GetCoordinatesMode() == coordinatesFrame3D )
	{
		m_paramView.nFlags |= paintWithZOrderNoWrite ;
	}
	else
	{
		m_paramView.nFlags &= ~paintWithZOrderNoWrite ;
	}
	if ( m_flagFrameBuffer )
	{
		LockController() ;
		for ( size_t i = 0; i < GetControllerCount(); i ++ )
		{
			S3DSceneComposer::Controller *
				pCtrl = GetControllerAt( i ) ;
			if ( (pCtrl == nullptr)
				|| pCtrl->IsControllerDisabled() )
			{
				continue ;
			}
			ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
			if ( pDrawer != nullptr )
			{
				PostUpdate( nullptr ) ;
				break ;
			}
			ImageEffector *	pEffector = ESLTypeCast<ImageEffector>( pCtrl ) ;
			if ( (pEffector != nullptr) && pEffector->IsEnabledFilter() )
			{
				PostUpdate( nullptr ) ;
				break ;
			}
		}
		UnlockController() ;
	}
	//
	S3DSceneItemSprite::OnUpdateBehavior( scene ) ;
}

// タイマー処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
	S3DSceneItemSprite::OnTimer( scene, msecPast ) ;
}

// レンダリング設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::OnSetupSceneSettings
	( S3DScene& scene,
		const SGLSize& sizeFrame,
		SGLSecondaryViewProducer * psvp )
{
	SSharableMutex *	pMutex = scene.GetUIThreadMutex() ;
	if ( pMutex != nullptr )
	{
		S3DSceneItemSprite::SetUIThreadMutex( pMutex ) ;
	}
	ItemCommonSerializer::OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
}

// 規定のレンダリングデバイス設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::OnSetRenderDevice( S3DRenderDevice * pDevice )
{
	ItemCommonSerializer::OnSetRenderDevice( pDevice ) ;
	SGLSpriteFormed::SetRenderDevice( pDevice ) ;
}

// カスタムフィルタ
////////////////////////////////////////////////////////////////////////////////////////
SGLImageObject *
	S3DCanvasSerializer::CustomFilter
		( S3DRenderContextInterface& render,
			SGLImageObject * pFrameBuf, SGLImageObject * pSrcImage )
{
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *
			pCtrl = GetControllerAt( GetControllerCount() - 1 - i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageEffector *	pEffector = ESLTypeCast<ImageEffector>( pCtrl ) ;
		if ( (pEffector != nullptr) && pEffector->IsEnabledFilter() )
		{
			SGLImageObject *	pDstImage = pFrameBuf ;
			if ( pSrcImage == pFrameBuf )
			{
				if ( m_imgFilterBuf.GetImageSize() != m_sizeCanvas )
				{
					m_imgFilterBuf.ReleaseBuffer() ;
					m_imgFilterBuf.CreateImage
						( m_sizeCanvas.w, m_sizeCanvas.h,
							formatImageDefaultRGBA, 32,
							SGLImageObject::bufferOnDeviceOnly ) ;
				}
				pDstImage = &m_imgFilterBuf ;
			}
			pSrcImage = pEffector->OnImageFilter( render, pDstImage, pSrcImage ) ;
		}
	}
	UnlockController() ;
	return	pSrcImage ;
}

// ヒットアイテム検索
////////////////////////////////////////////////////////////////////////////////////////
SGLSprite* S3DCanvasSerializer::GetHitSpriteAt( S2DDVector& vPos ) const
{
	SGLSprite *	pHit = S3DSceneItemSprite::GetHitSpriteAt( vPos ) ;
	if ( pHit != nullptr )
	{
		return	pHit ;
	}
	if ( IsHitImageDrawer( vPos.x, vPos.y ) )
	{
		return	(SGLSprite*) this ;
	}
	return	nullptr ;
}

// ヒット判定
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::IsHitSprite( double x, double y ) const
{
	return	S3DSceneItemSprite::IsHitSprite( x, y ) || IsHitImageDrawer( x, y ) ;
}

bool S3DCanvasSerializer::IsHitImageDrawer( double x, double y ) const
{
	bool	flagHit = false ;
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			if ( pDrawer->IsHitCursor( S2DDVector( x, y ) ) )
			{
				flagHit = true ;
				break ;
			}
		}
	}
	UnlockController() ;
	return	flagHit ;
}

// マウス移動
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	bool	flagProcessed = false ;
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			flagProcessed = pDrawer->OnMouseMove( S2DVector( xPos, yPos ) ) ;
			if ( flagProcessed )
			{
				break ;
			}
		}
	}
	UnlockController() ;
	if ( flagProcessed )
	{
		return	true ;
	}
	return	S3DSceneItemSprite::OnMouseMove( xPos, yPos, nFlags ) ;
}

void S3DCanvasSerializer::OnMouseLeave( int64_t nFlags )
{
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			pDrawer->OnMouseLeave() ;
		}
	}
	UnlockController() ;
	S3DSceneItemSprite::OnMouseLeave( nFlags ) ;
}

// ホイール回転
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	bool	flagProcessed = false ;
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			flagProcessed =
				pDrawer->OnMouseWheel
					( S2DVector( xPos, yPos ),
						(float32_t) zDelta / WheelDeltaUnit ) ;
			if ( flagProcessed )
			{
				break ;
			}
		}
	}
	UnlockController() ;
	if ( flagProcessed )
	{
		return	true ;
	}
	return	S3DSceneItemSprite::OnMouseWheel( zDelta, xPos, yPos, nFlags ) ;
}

// マウスボタン
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasSerializer::OnButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	SGLBasicForm::MouseButton	button = SGLBasicForm::mouseLeft ;
	switch ( SGLMouseInterface::GetButtonID( nFlags ) )
	{
	case	SGLSpriteMouseListener::RightButtonID:
		button = SGLBasicForm::mouseRight ;
		break ;
	case	SGLSpriteMouseListener::MiddleButtonID:
		button = SGLBasicForm::mouseMiddle ;
		break ;
	}
	bool	flagProcessed = false ;
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			flagProcessed = pDrawer->OnClickDown( S2DVector( xPos, yPos ), button ) ;
			if ( flagProcessed )
			{
				break ;
			}
		}
	}
	UnlockController() ;
	if ( flagProcessed )
	{
		return	true ;
	}
	return	S3DSceneItemSprite::OnButtonDown( xPos, yPos, nFlags ) ;
}

bool S3DCanvasSerializer::OnButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	SGLBasicForm::MouseButton	button = SGLBasicForm::mouseLeft ;
	switch ( SGLMouseInterface::GetButtonID( nFlags ) )
	{
	case	SGLSpriteMouseListener::RightButtonID:
		button = SGLBasicForm::mouseRight ;
		break ;
	case	SGLSpriteMouseListener::MiddleButtonID:
		button = SGLBasicForm::mouseMiddle ;
		break ;
	}
	bool	flagProcessed = false ;
	LockController() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			flagProcessed = pDrawer->OnClickUp( S2DVector( xPos, yPos ), button ) ;
			if ( flagProcessed )
			{
				break ;
			}
		}
	}
	UnlockController() ;
	if ( flagProcessed )
	{
		return	true ;
	}
	return	S3DSceneItemSprite::OnButtonUp( xPos, yPos, nFlags ) ;
}

// 視野角を合わせる
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::CmdFit2DViewAngle( void )
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		const S3DSceneComposer::CompositionInfo *	pci = pComp->GetCompositionInfo() ;
		if ( pci != nullptr )
		{
			S3DCompositionEditorInterface *	pEditor = GetEditor() ;
			//
			const S3DScene::ProjectionParam&	projParam = pci->GetProjectionParam() ;
			double	tanHalfAngle = projParam.vScreen.x / projParam.vScreen.z ;
			//
			double	degAngle = atan( tanHalfAngle ) * 360.0 / PI ;
			if ( pEditor != nullptr )
			{
				pEditor->EditScalarProperty
					( pComp, this, param2DViewAngle, degAngle, true, L"視野角を合わせる" ) ;
			}
			else
			{
				m_deg2DViewAngle = degAngle ;
			}
		}
	}
}

// 現在のカメラに 2D 表示位置と 3D 座標空間が一致するように変更する
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::CmdFit2DViewPosition( void )
{
	if ( GetCoordinatesMode() != coordinatesDirect2D )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp == nullptr )
	{
		return ;
	}
	const S3DSceneComposer::CompositionInfo *	pci = pComp->GetCompositionInfo() ;
	if ( pci == nullptr )
	{
		return ;
	}
	S3DScene *	pScene = pComp->GetScene() ;
	if ( pScene == nullptr )
	{
		return ;
	}
	S3DScene::Camera *	pDefCamera = pScene->GetMainCamera() ;
	if ( pDefCamera == nullptr )
	{
		pDefCamera =
			ESLTypeCast<S3DScene::Camera>
				( pComp->GetSceneItemAs( pci->GetDefaultCameraID() ) ) ;
		if ( pDefCamera == nullptr )
		{
			return ;
		}
	}
	S3DCompositionEditorInterface *	pEditor = GetEditor() ;
	//
	// Camvas を配置すべきｚ座標を計算
	const S3DScene::ProjectionParam&	projParam = pci->GetProjectionParam() ;
	double	fpViewScale =
			tan(m_deg2DViewAngle * 0.5 * PI / 180.0)
				/ (projParam.vScreen.x / projParam.vScreen.z) ;
	double	zViewCanvas =
			((double) m_sizeCanvas.w / pci->GetScreenSize().w)
							* projParam.vScreen.z / fpViewScale ;
	//
	// カメラの位置と向き取得
	S3DDVector	vCameraPos = pDefCamera->GetCameraPosition() ;
	S3DDVector	vCameraView = (pDefCamera->GetCameraTarget() - vCameraPos).Normalized() ;
	//
	// カメラ変換行列取得 : x' = matCamera * x - vCamera
	S3DDMatrix	matCamera ;
	S3DDVector	vCamera ;
	pDefCamera->CalcCameraMatrix( matCamera, vCamera ) ;
	//
	// カメラ変換逆行列と設置座標
	S3DDMatrix	matICamera = matCamera.Inverse() ;
	S3DDVector	vICamera = matICamera * vCamera ;
	S3DDVector	vCanvasOrg =
		matICamera * S3DDVector(- m_sizeCanvas.w / 2,
								- m_sizeCanvas.h / 2, zViewCanvas) + vICamera ;
	//
	// 座標設定
	if ( pEditor != nullptr )
	{
		pEditor->EditVectorProperty
			( pComp, this, paramPosition, vCanvasOrg, true, L"2D Canvas 位置調整" ) ;
		pEditor->EditMatrixProperty
			( pComp, this, paramRotation, matICamera, false, L"2D Canvas 位置調整" ) ;
	}
	else
	{
		SpaceParameter().m_vCenter = vCanvasOrg ;
		SpaceParameter().m_matTransformation = matICamera ;
	}
}

// 画像描画リスト
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::OnDrawImage( SGLDrawImageParamList& dipl )
{
	LockController() ;
	m_csDrawList.Lock() ;
	for ( size_t i = 0; i < GetControllerCount(); i ++ )
	{
		S3DSceneComposer::Controller *
			pCtrl = GetControllerAt( GetControllerCount() - 1 - i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		ImageDrawer *	pDrawer = ESLTypeCast<ImageDrawer>( pCtrl ) ;
		if ( pDrawer != nullptr )
		{
			pDrawer->OnDrawImage( dipl ) ;
		}
	}
	m_csDrawList.Unlock() ;
	UnlockController() ;
}

// 画像描画追加
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasSerializer::AddDrawImageParam
	( const SGLPaintParam& param,
		SGLImageObject * pImage,
		const SGLImageRect * pSrcRect )
{
	m_csDrawList.Lock() ;
	DrawImageParamList().AddDrawParam( param, pImage, pSrcRect ) ;
	m_csDrawList.Unlock() ;
}

void S3DCanvasSerializer::AddDrawImageParams
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppImages,
		const SGLImageRect * pSrcRects )
{
	m_csDrawList.Lock() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DrawImageParamList().AddDrawParam
			( pParams[i], ppImages[i],
				((pSrcRects != nullptr) ? (pSrcRects + i) : nullptr) ) ;
	}
	m_csDrawList.Unlock() ;
}


////////////////////////////////////////////////////////////////////////////////////////
// 画像表示コントローラー基底
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( S3DCanvasBasicController, Controller, ImageDrawer )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicController::S3DCanvasBasicController( const wchar_t * pwszClassID )
	: Controller( pwszClassID ),
		m_vPosition( 0, 0, 0 ),
		m_vCenter( 0, 0 ),
		m_vZoom( 1, 1, 1 ),
		m_degRotation( 0 ),
		m_fpTransparency( 0 ),
		m_flagVisible( true )
{
	PrepareParameterEntryCount( paramBasicCount ) ;
	ESLVerify( AddParameterEntry
		( L"position",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrNoLocalTransform, L"座標" ) == paramPosition ) ;
	ESLVerify( AddParameterEntry
		( L"center2d",
			S3DSceneComposer::typeVector2, 0, L"基準座標" ) == paramCenter ) ;
	ESLVerify( AddParameterEntry
		( L"zoom2d",
			S3DSceneComposer::typeZoom, 0, L"拡大率" ) == paramZoom ) ;
	ESLVerify( AddParameterEntry
		( L"rotation_z",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"回転角", nullptr, -360.0, 360.0 ) == paramRotation ) ;
	ESLVerify( AddParameterEntry
		( L"transparency",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"透明度", nullptr, 0.0, 1.0 ) == paramTransparency ) ;
	ESLVerify( AddParameterEntry
		( L"visible",
			S3DSceneComposer::typeBoolean, 0, L"表示" ) == paramVisible ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicController::~S3DCanvasBasicController( void )
{
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCanvasBasicController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	m_vPosition ;
	case	paramZoom:
		return	m_vZoom ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DCanvasBasicController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_degRotation ;

	case	paramTransparency:
		return	m_fpTransparency ;
	}
	return	0.0 ;
}

bool S3DCanvasBasicController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVisible:
		return	m_flagVisible ;
	}
	return	0.0 ;
}

size_t S3DCanvasBasicController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( nBufBytes == sizeof(S2DDVector) )
	{
		S2DDVector&	vDst = *((S2DDVector*)pDst) ;
		switch ( i )
		{
		case	paramCenter:
			vDst = m_vCenter ;
			break ;
		}
		return	sizeof(S2DDVector) ;
	}
	return	0 ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		m_vPosition = vec ;
		return ;
	case	paramZoom:
		m_vZoom = vec ;
		return ;
	}
}

void S3DCanvasBasicController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramRotation:
		m_degRotation = s ;
		return ;

	case	paramTransparency:
		m_fpTransparency = s ;
		return ;
	}
}

void S3DCanvasBasicController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramVisible:
		m_flagVisible = b ;
		return ;
	}
}

size_t S3DCanvasBasicController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( nBufBytes == sizeof(S2DDVector) )
	{
		const S2DDVector&	vSrc = *((const S2DDVector*)pSrc) ;
		switch ( i )
		{
		case	paramCenter:
			m_vCenter = vSrc ;
			break ;
		}
		return	sizeof(S2DDVector) ;
	}
	return	0 ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicController::OnDrawImage( SGLDrawImageParamList& dipl )
{
	uint32_t	nTransparency =
		(uint32_t) esl_lroundfi( esl_fmax( m_fpTransparency, 0.0 ) * 256.0 ) ;
	if ( (nTransparency < 0x100) && m_flagVisible )
	{
		SGLAffine	affineRot ;
		affineRot.SetRotation( m_degRotation * PI / 180.0 ) ;
		affineRot.SetPosition( m_vPosition.x, m_vPosition.y ) ;
		//
		SGLAffine	affineZoom ;
		affineZoom.a11 = (float32_t) m_vZoom.x ;
		affineZoom.a22 = (float32_t) m_vZoom.y ;
		affineZoom.SetPosition
			( - m_vCenter.x * m_vZoom.x, - m_vCenter.y * m_vZoom.y ) ;
		//
		SGLAffine	affineSaved = dipl.GetAffine() ;
		uint32_t	nTransSaved = dipl.GetTransparency() ;
		//
		dipl.AppendAffine( affineRot * affineZoom ) ;
		dipl.AppendTransparency( nTransparency ) ;
		//
		OnLocalDrawImage( dipl ) ;
		//
		dipl.SetAffine( affineSaved ) ;
		dipl.SetTransparency( nTransSaved ) ;
	}
}

// カーソル当たり判定
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBasicController::IsHitCursor( const S2DDVector& vPos )
{
	return	false ;
}

// マウス入力
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBasicController::OnMouseMove( const S2DVector& vPos )
{
	return	false ;
}

void S3DCanvasBasicController::OnMouseLeave( void )
{
}

bool S3DCanvasBasicController::OnMouseWheel( const S2DVector& vGlobal, float32_t zDelta )
{
	return	false ;
}

bool S3DCanvasBasicController::OnClickDown( const S2DVector& vPos, SGLBasicForm::MouseButton button )
{
	return	false ;
}

bool S3DCanvasBasicController::OnClickUp( const S2DVector& vPos, SGLBasicForm::MouseButton button )
{
	return	false ;
}



////////////////////////////////////////////////////////////////////////////////////////
// 画像表示コントローラー
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasImageController, S3DCanvasBasicController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasImageController, canvas_image )

const SSystem::SXMLDocument::AttrInteger
	S3DCanvasImageController::s_aiColorEffectType[S3DCanvasImageController::effectColorCount+1] =
{
	{ L"no", effectColorNo },
	{ L"mul", effectColorMul },
	{ L"add", effectColorAdd },
} ;

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasImageController::S3DCanvasImageController( void )
	: S3DCanvasBasicController( m_ItemClassDescriptor.pwszClassID ),
		m_pImage( nullptr ), m_flagSmoothing( true ),
		m_colorEffect( effectColorNo ), m_rgbEffectColor( 0x808080 )
{
	ESLVerify( AddParameterEntry
		( L"image",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrStringEnumeration,
			L"画像" ) == paramImageID ) ;
	ESLVerify( AddParameterEntry
		( L"smoothing",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"ピクセル補完" ) == paramSmoothing ) ;
	ESLVerify( AddParameterEntry
		( L"color_effect_type",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrStringEnumeration,
			L"色効果タイプ" ) == paramColorEffectType ) ;
	ESLVerify( AddParameterEntry
		( L"effect_color",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory1,
			L"効果色" ) == paramEffectColor ) ;
	ESLVerify( AddParameterEntry
		( L"cmd_ref_center",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrEditorCommand,
			L"基準座標を画像情報から反映" ) == paramCmdRefCenter ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasImageController::~S3DCanvasImageController( void )
{
}

// 画像参照
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasImageController::UpdateImageRef( void )
{
	if ( m_strImageID.IsEmpty() )
	{
		m_pImage = nullptr ;
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		m_pImage = pComposer->GetAssets().GetImageAs( m_strImageID ) ;
	}
	else
	{
		m_pImage = nullptr ;
	}
}

// 画像基準座標を設定する
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasImageController::CmdRefImageCenter( void )
{
	if ( m_pImage != nullptr )
	{
		SGLImageInfo	imginf ;
		m_pImage->GetImageInfo( imginf ) ;
		//
		S2DDVector	vCenter( imginf.ptOrigin.x, imginf.ptOrigin.y ) ;
		S2DDVector	vOffset = vCenter - m_vCenter ;
		S3DDVector	vPosition = m_vPosition + S3DDVector( vOffset.x, vOffset.y, 0.0f ) ;
		//
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		S3DCompositionEditorInterface *	pEditor = GetEditor() ;
		if ( (pComp != nullptr) && (pEditor != nullptr) )
		{
			pEditor->EditBinaryProperty
				( pComp, this, paramCenter, &vCenter, sizeof(vCenter),
					true, L"画像基準座標を画像情報から反映" ) ;
			pEditor->EditVectorProperty
				( pComp, this, paramPosition, vPosition,
					true, L"画像座標を基準座標の修正に合わせて変更" ) ;
		}
		else
		{
			m_vPosition = vPosition ;
			m_vCenter = vCenter ;
		}
	}
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCanvasImageController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramEffectColor:
		return	VectorFromColor( m_rgbEffectColor ) ;
	}
	return	S3DCanvasBasicController::GetVectorParameter( iParam ) ;
}

bool S3DCanvasImageController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramSmoothing:
		return	m_flagSmoothing ;

	case	paramCmdRefCenter:
		return	false ;
	}
	return	S3DCanvasBasicController::GetBooleanParameter( iParam ) ;
}

const wchar_t * S3DCanvasImageController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramImageID:
		return	m_strImageID ;

	case	paramColorEffectType:
		return	SXMLDocument::GetSymbolAsIntegerOf
					( s_aiColorEffectType, m_colorEffect ) ;
	}
	return	S3DCanvasBasicController::GetCommandParameter( iParam ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasImageController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramEffectColor:
		m_rgbEffectColor = ColorFromVector( vec ) ;
		return ;
	}
	S3DCanvasBasicController::SetVectorParameter( iParam, vec ) ;
}

void S3DCanvasImageController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramSmoothing:
		m_flagSmoothing = b ;
		return ;

	case	paramCmdRefCenter:
		CmdRefImageCenter() ;
		return ;
	}
	S3DCanvasBasicController::SetBooleanParameter( iParam, b ) ;
}

void S3DCanvasImageController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramImageID:
		if ( m_strImageID != pwszCmd )
		{
			m_strImageID = pwszCmd ;
			UpdateImageRef() ;
		}
		return ;

	case	paramColorEffectType:
		m_colorEffect =
			(ColorEffectType) SXMLDocument::GetIntegerAsSymbolOf
					( s_aiColorEffectType, pwszCmd, m_colorEffect ) ;
		return ;
	}
	S3DCanvasBasicController::SetCommandParameter( iParam, pwszCmd ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasImageController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam == paramImageID )
	{
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateTextureStringSet( aStrSet ) ;
		}
		return	true ;
	}
	if ( iParam == paramColorEffectType )
	{
		for ( int i = 0; s_aiColorEffectType[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(s_aiColorEffectType[i].pszSymbol) ) ;
		}
		return	true ;
	}
	return	S3DCanvasBasicController::EnumerateStringSet( iParam, aStrSet ) ;
}

// パラメータカテゴリ名取得
////////////////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCanvasImageController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"画像設定" ;
	}
	return	S3DCanvasBasicController::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
////////////////////////////////////////////////////////////////////////////////////////
uint32_t S3DCanvasImageController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		S3DCanvasBasicController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateImageRef() ;
	}
	return	nResFlags ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasImageController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	if ( m_pImage != nullptr )
	{
		SGLPaintParam	param ;
		param.nFlags = paintDelayable ;
		if ( m_flagSmoothing )
		{
			param.nFlags |= paintSmoothStretch ;
		}
		switch ( m_colorEffect )
		{
		case	effectColorNo:
		default:
			break ;
		case	effectColorMul:
			param.nFlags |= paintApplyColorMul ;
			param.rgbColorParam = m_rgbEffectColor ;
			break ;
		case	effectColorAdd:
			param.nFlags |= paintApplyColorAdd ;
			param.rgbColorParam = m_rgbEffectColor ;
			break ;
		}
		dipl.AddDrawParam( param, m_pImage ) ;
	}
}



////////////////////////////////////////////////////////////////////////////////////////
// 簡易フォーム
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasBasicFormController, S3DCanvasBasicController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasBasicFormController, canvas_form )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicFormController::S3DCanvasBasicFormController( void )
	: S3DCanvasBasicController( m_ItemClassDescriptor.pwszClassID ),
		m_pFormParser( nullptr )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;
	//
	ESLVerify( AddParameterEntry
		( L"form_rsrc",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"リソース" ) == paramFormResource ) ;
	ESLVerify( AddParameterEntry
		( L"form_id",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"フォーム" ) == paramFormID ) ;
	ESLVerify( AddParameterEntry
		( L"form_x",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrReadOnlyParam,
			L"ｘ座標" ) == paramFormX ) ;
	ESLVerify( AddParameterEntry
		( L"form_y",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrReadOnlyParam,
			L"ｙ座標" ) == paramFormY ) ;
	ESLVerify( AddParameterEntry
		( L"form_width",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrReadOnlyParam,
			L"フォーム幅" ) == paramFormWidth ) ;
	ESLVerify( AddParameterEntry
		( L"form_height",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrReadOnlyParam,
			L"フォーム高" ) == paramFormHeight ) ;
	ESLVerify( AddParameterEntry
		( L"cmd_ref_pos",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrEditorCommand,
			L"デフォルト座標に移動" ) == paramCmdRefPos ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicFormController::~S3DCanvasBasicFormController( void )
{
}

// フォーム取得
////////////////////////////////////////////////////////////////////////////////////////
SGLBasicForm * S3DCanvasBasicFormController::GetForm( void ) const
{
	return	m_pForm ;
}

// 画像参照
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicFormController::UpdateForm( void )
{
	m_pFormParser = nullptr ;
	m_pForm = nullptr ;
	//
	if ( m_strRsrcID.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		m_pFormParser = pComposer->GetAssets().GetBasicFormAs( m_strRsrcID ) ;
		if ( (m_pFormParser != nullptr) && !m_strFormID.IsEmpty() )
		{
			m_pForm = new SGLBasicForm ;
			m_pFormParser->BuildFormAs( *m_pForm, m_strFormID ) ;
		}
	}
}

// 画像基準座標を設定する
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicFormController::CmdRefFormPos( void )
{
	if ( m_pForm != nullptr )
	{
		S3DDVector	vPosition( m_pForm->GetFormRect().x, m_pForm->GetFormRect().y, 0 ) ;
		//
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		S3DCompositionEditorInterface *	pEditor = GetEditor() ;
		if ( (pComp != nullptr) && (pEditor != nullptr) )
		{
			pEditor->EditVectorProperty
				( pComp, this, paramPosition, vPosition,
					true, L"デフォルト座標" ) ;
		}
		else
		{
			m_vPosition = vPosition ;
		}
	}
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
int32_t S3DCanvasBasicFormController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFormX:
		return	(m_pForm != nullptr) ? m_pForm->GetFormRect().x : 0 ;

	case	paramFormY:
		return	(m_pForm != nullptr) ? m_pForm->GetFormRect().y : 0 ;

	case	paramFormWidth:
		return	(m_pForm != nullptr) ? m_pForm->GetFormRect().w : 0 ;

	case	paramFormHeight:
		return	(m_pForm != nullptr) ? m_pForm->GetFormRect().h : 0 ;
	}
	return	S3DCanvasBasicController::GetIntegerParameter( i ) ;
}

bool S3DCanvasBasicFormController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCmdRefPos:
		return	false ;
	}
	return	S3DCanvasBasicController::GetBooleanParameter( i ) ;
}

const wchar_t * S3DCanvasBasicFormController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFormResource:
		return	m_strRsrcID ;

	case	paramFormID:
		return	m_strFormID ;
	}
	return	S3DCanvasBasicController::GetCommandParameter( i ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicFormController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramFormX:
	case	paramFormY:
	case	paramFormWidth:
	case	paramFormHeight:
		return ;
	}
	S3DCanvasBasicController::SetIntegerParameter( i, n ) ;
}

void S3DCanvasBasicFormController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCmdRefPos:
		CmdRefFormPos() ;
		return ;
	}
	S3DCanvasBasicController::SetBooleanParameter( i, b ) ;
}

void S3DCanvasBasicFormController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramFormResource:
		if ( m_strRsrcID != pwszCmd )
		{
			m_strRsrcID = pwszCmd ;
			UpdateForm() ;
		}
		return ;

	case	paramFormID:
		if ( m_strFormID != pwszCmd )
		{
			m_strFormID = pwszCmd ;
			UpdateForm() ;
		}
		return ;
	}
	return	S3DCanvasBasicController::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBasicFormController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer *	pComposer = nullptr ;
	switch ( i )
	{
	case	paramFormResource:
		pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateResourceIDsAs
				( aStrSet, S3DSceneComposer::resourceTypeBasicForm ) ;
		}
		return	true ;

	case	paramFormID:
		if ( m_pFormParser != nullptr )
		{
			m_pFormParser->EnumerateFormIDs( aStrSet ) ;
		}
		return	true ;
	}
	return	S3DCanvasBasicController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
////////////////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCanvasBasicFormController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"フォーム設定" ;
	}
	return	S3DCanvasBasicController::GetParameterCategoryName( iCategory ) ;
}

// タイマー処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicFormController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	S3DCanvasBasicController::OnTimer( scene, pItem, msecPast ) ;
	//
	if ( m_pForm != nullptr )
	{
		m_pForm->OnTimer( msecPast ) ;
	}
}

// アイテムプロパティのリソース等の参照を更新する
////////////////////////////////////////////////////////////////////////////////////////
uint32_t S3DCanvasBasicFormController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		S3DCanvasBasicController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateForm() ;
	}
	return	nResFlags ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicFormController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	if ( m_pForm != nullptr )
	{
		m_pForm->DrawForm( dipl ) ;
	}
}

// カーソル当たり判定
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBasicFormController::IsHitCursor( const S2DDVector& vPos )
{
	if ( m_flagVisible && (m_fpTransparency < 0.999) && (m_pForm != nullptr) )
	{
		return	m_pForm->TestHitCursor( vPos ) ;
	}
	return	false ;
}

// マウス入力
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBasicFormController::OnMouseMove( const S2DVector& vPos )
{
	if ( m_flagVisible && (m_fpTransparency < 0.999) && (m_pForm != nullptr) )
	{
		return	m_pForm->OnMouseMove( vPos ) ;
	}
	return	false ;
}

void S3DCanvasBasicFormController::OnMouseLeave( void )
{
	if ( m_pForm != nullptr )
	{
		m_pForm->OnMouseLeave() ;
	}
}

bool S3DCanvasBasicFormController::OnMouseWheel( const S2DVector& vPos, float32_t zDelta )
{
	if ( m_flagVisible && (m_fpTransparency < 0.999) && (m_pForm != nullptr) )
	{
		return	m_pForm->OnMouseWheel( vPos, zDelta ) ;
	}
	return	false ;
}

bool S3DCanvasBasicFormController::OnClickDown( const S2DVector& vPos, SGLBasicForm::MouseButton button )
{
	if ( m_flagVisible && (m_fpTransparency < 0.999) && (m_pForm != nullptr) )
	{
		return	m_pForm->OnClickDown( vPos, button ) ;
	}
	return	false ;
}

bool S3DCanvasBasicFormController::OnClickUp( const S2DVector& vPos, SGLBasicForm::MouseButton button )
{
	if ( m_flagVisible && (m_fpTransparency < 0.999) && (m_pForm != nullptr) )
	{
		return	m_pForm->OnClickDown( vPos, button ) ;
	}
	return	false ;
}



////////////////////////////////////////////////////////////////////////////////////////
// メディア表示コントローラー基底
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasBasicMediaController, S3DCanvasBasicController )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicMediaController::S3DCanvasBasicMediaController( const wchar_t * pwszClassID )
	: S3DCanvasBasicController( pwszClassID ),
		m_flagPlay( true ), m_flagPause( false ),
		m_flagStarted( false ), m_flagPaused( false ),
		m_nLoopCount( 1 ),
		m_secSeek( 0.0 ), m_fpPitch( 1.0 ),
		m_fpLastFrame( 0.0 ), m_secPlayingPos( 0.0 )
{
	ESLVerify( AddParameterEntry
		( L"media_source",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"ソース" ) == paramMediaSource ) ;
	ESLVerify( AddParameterEntry
		( L"playing",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrEditUpdateFrame,
			L"再生" ) == paramPlay ) ;
	ESLVerify( AddParameterEntry
		( L"pause",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrEditUpdateFrame,
			L"一時停止" ) == paramPause ) ;
	ESLVerify( AddParameterEntry
		( L"seek_pos",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrEditUpdateFrame
			| S3DSceneComposer::attrUIScalarSlider,
			L"再生開始位置", nullptr, 0.0, 10.0 ) == paramSeek ) ;
	ESLVerify( AddParameterEntry
		( L"loop_count",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrEditUpdateFrame,
			L"ループ回数", nullptr ) == paramLoopCount ) ;
	ESLVerify(  AddParameterEntry
		( L"fit_view_pos",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrEditorCommand,
			L"再生区間をタイムライン上で表示区間に設定する",
			nullptr ) == paramCmdVisDuration ) ;
	ESLVerify(  AddParameterEntry
		( L"set_play_range",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrEditorCommand,
			L"再生区間をタイムライン上に設定する",
			nullptr ) == paramCmdPlayDuration ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBasicMediaController::~S3DCanvasBasicMediaController( void )
{
}

// ソース更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::UpdateMediaSource( void )
{
}

// 再生開始
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnPlayStart( double secTime )
{
}

// 再生一時停止
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnPlayPause( void )
{
}

// 再生再開
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnPlayRestart( void )
{
}

// 再生停止
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnPlayEnd( void )
{
}

// フレーム更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnUpdateMediaFrame( double secTime )
{
}

// メディア時間の正規化
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasBasicMediaController::NormalizeMediaTime( double secTime )
{
	double	secDuration = GetMediaDuration() ;
	if ( secDuration <= 0.0 )
	{
		return	secTime ;
	}
	double	fpLoop = floor( secTime / secDuration ) ;
	double	secMedia = secTime - fpLoop * secDuration ;
	if ( (m_nLoopCount >= 1) && (fpLoop >= m_nLoopCount) )
	{
		secMedia = secDuration ;
	}
	return	secMedia ;
}

// メディアの長さ[秒]を取得する
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasBasicMediaController::GetMediaDuration( void )
{
	return	0.0 ;
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasBasicMediaController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramSeek:
		return	m_secSeek ;
	}
	return	S3DCanvasBasicController::GetScalarParameter( iParam ) ;
}

int32_t S3DCanvasBasicMediaController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramLoopCount:
		return	m_nLoopCount ;
	}
	return	S3DCanvasBasicController::GetIntegerParameter( iParam ) ;
}

bool S3DCanvasBasicMediaController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramPlay:
		return	m_flagPlay ;
	case	paramPause:
		return	m_flagPause ;
	}
	return	S3DCanvasBasicController::GetBooleanParameter( iParam ) ;
}

const wchar_t * S3DCanvasBasicMediaController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramMediaSource:
		return	m_strSource ;
	}
	return	S3DCanvasBasicController::GetCommandParameter( iParam ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramSeek:
		m_secSeek = s ;
		return ;
	}
	S3DCanvasBasicController::SetScalarParameter( iParam, s ) ;
}

void S3DCanvasBasicMediaController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramLoopCount:
		m_nLoopCount = n ;
		return ;
	}
	S3DCanvasBasicController::SetIntegerParameter( iParam, n ) ;
}

void S3DCanvasBasicMediaController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramPlay:
		m_flagPlay = b ;
		return ;
	case	paramPause:
		m_flagPause = b ;
		return ;
	case	paramCmdVisDuration:
		CmdVisDuration() ;
		return ;
	case	paramCmdPlayDuration:
		CmdMediaDuration() ;
		return ;
	}
	S3DCanvasBasicController::SetBooleanParameter( iParam, b ) ;
}

void S3DCanvasBasicMediaController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramMediaSource:
		if ( m_strSource != pwszCmd )
		{
			m_strSource = pwszCmd ;
			UpdateMediaSource() ;
		}
		return ;
	}
	S3DCanvasBasicController::SetCommandParameter( iParam, pwszCmd ) ;
}

// パラメータカテゴリ名取得
////////////////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCanvasBasicMediaController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"メディア設定" ;
	}
	return	S3DCanvasBasicController::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
////////////////////////////////////////////////////////////////////////////////////////
uint32_t S3DCanvasBasicMediaController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		S3DCanvasBasicController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & (S3DSceneComposer::updateRefResource
					| S3DSceneComposer::updateRefSubComposition) )
	{
		UpdateMediaSource() ;
	}
	return	nResFlags ;
}

// フレーム（パラメータ）更新後処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	S3DCanvasBasicController::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	const S3DSceneComposer::CompositionInfo *	pci = nullptr ;
	S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;
	if ( pComp != nullptr )
	{
		pci = pComp->GetCompositionInfo() ;
	}
	if ( pci == nullptr )
	{
		return ;
	}
	double	secSeekTime = m_secPlayingPos ;
	double	fpLastFrame = m_fpLastFrame ;
	if ( (seek == S3DSceneComposer::seekJump)
		|| (seek == S3DSceneComposer::seekJumpReset)
		|| (m_flagPlay && !m_flagStarted
			&& (seek == S3DSceneComposer::seekStream)) )
	{
		int32_t	iBaseFrame = 0 ;
		int32_t	iFrame = (int32_t) floor( fpFrame ) ;
		//
		S3DSceneComposer::Sequencer *	pSeqPlay = GetParameterSequencer( paramPlay ) ;
		if ( pSeqPlay != nullptr )
		{
			double	t ;
			size_t	iKeyPlay = pSeqPlay->GetKeyFrameProgress( t, iFrame ) ;
			//
			S3DSceneComposer::KeyFrameParam	kfpPlay ;
			if ( pSeqPlay->GetKeyFrameParameter( iKeyPlay, kfpPlay ) )
			{
				iFrame = kfpPlay.iFrame ;
				iBaseFrame = iFrame ;
			}
		}
		S3DSceneComposer::Sequencer *	pSeqSeek = GetParameterSequencer( paramSeek ) ;
		if ( m_flagPlay && (pSeqSeek != nullptr) )
		{
			secSeekTime = pSeqSeek->GetFrameScalar( iFrame ) ;
		}
		else
		{
			secSeekTime = m_secSeek ;
		}
		fpLastFrame = iBaseFrame ;
	}
	double	secTime = secSeekTime ;
	if ( m_flagPlay && !m_flagPause && (fpFrame > fpLastFrame) )
	{
		secTime += pci->FrameIndexToSecond( fpFrame - fpLastFrame ) * m_fpPitch ;
	}
	m_secPlayingPos = secTime ;
	m_fpLastFrame = fpFrame ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		if ( m_flagStarted )
		{
			OnPlayEnd() ;
			m_flagStarted = false ;
		}
		OnUpdateMediaFrame( NormalizeMediaTime(secTime) ) ;
		return ;
	}
	if ( seek != S3DSceneComposer::seekStreamPaused )
	{
		if ( m_flagPlay )
		{
			if ( !m_flagPause )
			{
				if ( !m_flagStarted )
				{
					OnPlayStart( NormalizeMediaTime(secTime) ) ;
					m_flagStarted = true ;
					m_flagPaused = false ;
				}
				else
				{
					if ( m_flagPaused )
					{
						ESLAssert( m_flagStarted ) ;
						OnPlayRestart() ;
						m_flagPaused = false ;
					}
					OnUpdateMediaFrame( NormalizeMediaTime(secTime) ) ;
				}
			}
			else
			{
				if ( m_flagStarted && !m_flagPaused )
				{
					OnPlayPause() ;
					m_flagPaused = true ;
				}
			}
		}
		else
		{
			if ( m_flagStarted )
			{
				OnPlayEnd() ;
				m_flagStarted = false ;
				m_flagPaused = false ;
			}
			OnUpdateMediaFrame( NormalizeMediaTime(secTime) ) ;
		}
	}
}

// 拡張的な処理の通知
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	S3DCanvasBasicController::OnExtendNotify
			( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStopItem ) == 0 )
	{
		if ( m_flagStarted )
		{
			OnPlayEnd() ;
			m_flagStarted = false ;
		}
	}
}

// 再生区間にタイムライン上の表示区間を設定する
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::CmdVisDuration( void )
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp == nullptr )
	{
		return ;
	}
	S3DCompositionEditorInterface *	pEditor = GetEditor() ;
	if ( pEditor == nullptr )
	{
		return ;
	}
	double	secDuration = GetMediaDuration() ;
	if ( secDuration == 0.0 )
	{
		return ;
	}
	pEditor->AddEditUndo( pComp, this, L"メディア表示区間を設定" ) ;
	//
	int	iFrame = pEditor->GetCurrentTimelineFrame( pComp ) ;
	//
	S3DSceneComposer::Sequencer *	pSeqPlay = GetParameterSequencer( paramPlay ) ;
	if ( pSeqPlay != nullptr )
	{
		double	t ;
		size_t	iKeyPlay = 
			m_flagPlay ? pSeqPlay->GetKeyFrameProgress( t, iFrame ) : 0 ;
		//
		S3DSceneComposer::KeyFrameParam	kfpPlay ;
		if ( pSeqPlay->GetKeyFrameParameter( iKeyPlay, kfpPlay ) )
		{
			iFrame = kfpPlay.iFrame ;
		}
	}
	S3DSceneComposer::KeyFrameParam	kfpVis ;
	kfpVis.iFrame = (int32_t) iFrame ;
	kfpVis.nFlags = 0 ;
	kfpVis.speedIn = 0 ;
	kfpVis.speedIn = 0 ;
	//
	S3DSceneComposer::Sequencer *	pSeqVis = CreateParameterSequencer( paramVisible ) ;
	ssize_t	iKeyVis = pSeqVis->FindKeyFrame( iFrame ) ;
	if ( iKeyVis < 0 )
	{
		iKeyVis = (ssize_t) pSeqVis->OrderKeyFrameIndex( iFrame ) ;
		pSeqVis->InsertKeyFrame( iKeyVis, kfpVis ) ;
	}
	pSeqVis->SetBooleanParameter( (size_t) iKeyVis, true ) ;
	//
	if ( iKeyVis == 0 )
	{
		iKeyVis = (ssize_t) pSeqVis->OrderKeyFrameIndex( 0 ) ;
		kfpVis.iFrame = 0 ;
		pSeqVis->InsertKeyFrame( iKeyVis, kfpVis ) ;
		pSeqVis->SetBooleanParameter( (size_t) iKeyVis, false ) ;
	}
	//
	const S3DSceneComposer::CompositionInfo *	pci = pComp->GetCompositionInfo() ;
	ESLAssert( pci != nullptr ) ;
	int		iEndFrame = iFrame + (int) pci->FrameIndexFromSecond( secDuration ) ;
	kfpVis.iFrame = (int32_t) iEndFrame ;
	iKeyVis = pSeqVis->FindKeyFrame( iEndFrame ) ;
	if ( iKeyVis < 0 )
	{
		iKeyVis = (ssize_t) pSeqVis->OrderKeyFrameIndex( iEndFrame ) ;
		pSeqVis->InsertKeyFrame( iKeyVis, kfpVis ) ;
	}
	pSeqVis->SetBooleanParameter( (size_t) iKeyVis, false ) ;
}

// 再生区間をタイムライン上に生成する
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBasicMediaController::CmdMediaDuration( void )
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp == nullptr )
	{
		return ;
	}
	S3DCompositionEditorInterface *	pEditor = GetEditor() ;
	if ( pEditor == nullptr )
	{
		return ;
	}
	double	secDuration = GetMediaDuration() ;
	if ( secDuration == 0.0 )
	{
		return ;
	}
	pEditor->AddEditUndo( pComp, this, L"メディア再生区間を生成" ) ;
	//
	// 再生開始フレーム取得
	//
	int	iStartFrame = pEditor->GetCurrentTimelineFrame( pComp ) ;
	//
	S3DSceneComposer::Sequencer *	pSeqPlay = GetParameterSequencer( paramPlay ) ;
	if ( pSeqPlay != nullptr )
	{
		double	t ;
		size_t	iKeyPlay = 
			m_flagPlay ? pSeqPlay->GetKeyFrameProgress( t, iStartFrame ) : 0 ;
		//
		S3DSceneComposer::KeyFrameParam	kfpPlay ;
		if ( pSeqPlay->GetKeyFrameParameter( iKeyPlay, kfpPlay ) )
		{
			iStartFrame = kfpPlay.iFrame ;
		}
	}
	S3DSceneComposer::KeyFrameParam	kfp ;
	kfp.iFrame = (int32_t) iStartFrame ;
	kfp.nFlags = 0 ;
	kfp.speedIn = 0 ;
	kfp.speedIn = 0 ;
	//
	// シーケンス取得
	//
	if ( pSeqPlay == nullptr )
	{
		pSeqPlay = CreateParameterSequencer( paramPlay ) ;
	}
	S3DSceneComposer::Sequencer *
			pSeqSource = CreateParameterSequencer( paramMediaSource ) ;
	//
	// 再生開始位置にキーフレーム設定
	//
	ssize_t	iKeyStart = pSeqPlay->FindKeyFrame( iStartFrame ) ;
	if ( iKeyStart < 0 )
	{
		iKeyStart = (ssize_t) pSeqPlay->OrderKeyFrameIndex( iStartFrame ) ;
		kfp.iFrame = (int32_t) iStartFrame ;
		pSeqPlay->InsertKeyFrame( iKeyStart, kfp ) ;
	}
	pSeqPlay->SetBooleanParameter( (size_t) iKeyStart, true ) ;
	//
	if ( (iKeyStart == 0) && (iStartFrame > 1) )
	{
		iKeyStart = (ssize_t) pSeqPlay->OrderKeyFrameIndex( 0 ) ;
		kfp.iFrame = 0 ;
		pSeqPlay->InsertKeyFrame( iKeyStart, kfp ) ;
		pSeqPlay->SetBooleanParameter( (size_t) iKeyStart, false ) ;
	}
	//
	// 再生開始位置にメディアソースのキーフレーム設定
	//
	iKeyStart = pSeqSource->FindKeyFrame( iStartFrame ) ;
	if ( iKeyStart < 0 )
	{
		iKeyStart = (ssize_t) pSeqSource->OrderKeyFrameIndex( iStartFrame ) ;
		kfp.iFrame = (int32_t) iStartFrame ;
		pSeqSource->InsertKeyFrame( iKeyStart, kfp ) ;
	}
	pSeqSource->SetCommandParameter( (size_t) iKeyStart, m_strSource ) ;
	//
	if ( (iKeyStart == 0) && (iStartFrame > 1) )
	{
		iKeyStart = (ssize_t) pSeqSource->OrderKeyFrameIndex( 0 ) ;
		kfp.iFrame = 0 ;
		pSeqSource->InsertKeyFrame( iKeyStart, kfp ) ;
		pSeqSource->SetCommandParameter( (size_t) iKeyStart, L"" ) ;
	}
	//
	// 再生終了位置にキーフレーム設定
	//
	const S3DSceneComposer::CompositionInfo *	pci = pComp->GetCompositionInfo() ;
	ESLAssert( pci != nullptr ) ;
	int		iEndFrame = iStartFrame + (int) pci->FrameIndexFromSecond( secDuration ) ;
	kfp.iFrame = (int32_t) iEndFrame ;
	ssize_t	iKeyEnd = pSeqPlay->FindKeyFrame( iEndFrame ) ;
	if ( iKeyEnd < 0 )
	{
		iKeyEnd = (ssize_t) pSeqPlay->OrderKeyFrameIndex( iEndFrame ) ;
		pSeqPlay->InsertKeyFrame( iKeyEnd, kfp ) ;
	}
	pSeqPlay->SetBooleanParameter( (size_t) iKeyEnd, false ) ;
	//
	iKeyEnd = pSeqSource->FindKeyFrame( iEndFrame ) ;
	if ( iKeyEnd < 0 )
	{
		iKeyEnd = (ssize_t) pSeqPlay->OrderKeyFrameIndex( iEndFrame ) ;
		pSeqSource->InsertKeyFrame( iKeyEnd, kfp ) ;
	}
	pSeqSource->SetCommandParameter( (size_t) iKeyEnd, L"" ) ;
}



////////////////////////////////////////////////////////////////////////////////////////
// 音声トラックコントローラー
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasAudioTrackController, S3DCanvasBasicMediaController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasAudioTrackController, audio_track )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasAudioTrackController::S3DCanvasAudioTrackController( const wchar_t * pwszClassID )
	:  S3DCanvasBasicMediaController
			( (pwszClassID == nullptr)
				? m_ItemClassDescriptor.pwszClassID : pwszClassID ),
		m_audioStream( nullptr ),
		m_flagSoundOpened( false ), m_nSoundPlayPos( 0 ),
		m_fpVolume( 1.0 ), m_fpPan( 0.0 )
{
	ESLVerify( AddParameterEntry
		( L"volume",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIScalarSlider,
			L"音量", nullptr, 0.0, 1.0 ) == paramVolume ) ;
	ESLVerify( AddParameterEntry
		( L"pan",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIScalarSlider,
			L"パン", nullptr, -1.0, 1.0 ) == paramPan ) ;
	ESLVerify( AddParameterEntry
		( L"pitch",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIScalarSlider,
			L"ピッチ", L"再生速度比", 0.125, 8.0 ) == paramPitch ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasAudioTrackController::~S3DCanvasAudioTrackController( void )
{
	ReleaseAudioStream() ;
}

// オーディオストリーム解放
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::ReleaseAudioStream( void )
{
	if ( (m_audioStream != nullptr) && (m_audioPlayer != nullptr) )
	{
		m_audioPlayer->ReleaseAudioStream( m_audioStream ) ;
		m_audioStream = nullptr ;
	}
	m_audioPlayer = nullptr ;
	//
	CloseSoundPlayer() ;
}

// 音声出力停止
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::CloseSoundPlayer( void )
{
	if ( m_flagSoundOpened )
	{
		m_soundPlayer.Close() ;
		m_flagStarted = false ;
		m_flagSoundOpened = false ;
	}
	m_queSoundBuf.ClearAll() ;
}

// ソース更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::UpdateMediaSource( void )
{
	ReleaseAudioStream() ;
	//
	if ( m_strSource.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		SGLAudioPlayerInterface *	pAudio =
				pComposer->GetAssets().GetAudioAs( m_strSource ) ;
		if ( pAudio != nullptr )
		{
			m_audioPlayer = pAudio->ClonePlayer() ;
			if ( m_audioPlayer != nullptr )
			{
				m_audioStream = m_audioPlayer->GetAudioStream() ;
				//
				S3DSceneComposer::ParamEntry&	peSeek = ParameterEntryAt( paramSeek ) ;
				peSeek.maxRange = (double) m_audioPlayer->GetTotalLength()
									/ (double) m_audioPlayer->GetSampleFrequency() ;
			}
		}
	}
}

// 再生開始
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnPlayStart( double secTime )
{
	if ( m_audioStream != nullptr )
	{
		if ( m_audioStream->GetAudioFormat( m_soundFormat ) == sglErrSuccess )
		{
			m_flagSoundOpened =
				(m_soundPlayer.Open( m_soundFormat ) == sglErrSuccess) ;
			m_soundPlayer.PrepareStream() ;
			//
			m_nSoundPlayPos =
				m_soundFormat.MilliSecToSamples
					( esl_lroundfi( secTime * 1000.0 ) ) ;
			m_audioStream->SeekAudio( m_nSoundPlayPos ) ;
			//
			RelfectAudioVolume() ;
			//
			m_soundPlayer.Play() ;
			PlayAudioStream( secTime + 0.1 ) ;		// 100ms バッファ
		}
	}
}

// 再生一時停止
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnPlayPause( void )
{
	if ( m_audioStream != nullptr )
	{
		m_soundPlayer.Pause() ;
	}
}

// 再生再開
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnPlayRestart( void )
{
	if ( m_audioStream != nullptr )
	{
		m_soundPlayer.Restart() ;
	}
}

// 再生停止
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnPlayEnd( void )
{
	CloseSoundPlayer() ;
}

// フレーム更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnUpdateMediaFrame( double secTime )
{
	if ( m_flagSoundOpened && (m_audioStream != nullptr) )
	{
		//
		// 音量反映
		//
		RelfectAudioVolume() ;
		//
		// データをフレームの分まで出力（200ms バッファ）
		//
		PlayAudioStream( secTime + 0.2 ) ;
	}
}

// メディアの長さ[秒]を取得する
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasAudioTrackController::GetMediaDuration( void )
{
	if ( m_audioStream != nullptr )
	{
		SGLSoundFormat	fmt ;
		if ( m_audioStream->GetAudioFormat( fmt ) == sglErrSuccess )
		{
			int64_t	nSamples = m_audioStream->GetAudioLength() ;
			return	(double) nSamples / (double) fmt.frequency ;
		}
	}
	return	0.0 ;
}

// 指定時間まで音声データを再生出力
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::PlayAudioStream( double secTime )
{
	if ( m_flagSoundOpened && (m_audioStream != nullptr) )
	{
		uint64_t	nSoundPos =
						m_soundFormat.MilliSecToSamples
							( esl_lroundfi( secTime * 1000.0 ) ) ;
		if ( nSoundPos > m_nSoundPlayPos )
		{
			size_t		nSamples = (size_t) (nSoundPos - m_nSoundPlayPos) ;
			size_t		nBytes = (size_t) m_soundFormat.SamplesToBytes( nSamples ) ;
			uint8_t *	pbytBuf = m_queSoundBuf.PutBuffer( nBytes ) ;
			size_t		nReadSamples = m_audioStream->ReadAudio( pbytBuf, nSamples ) ;
			m_queSoundBuf.FlushBuffer
				( (size_t) m_soundFormat.SamplesToBytes( nReadSamples ) ) ;
			m_nSoundPlayPos += nReadSamples ;
			//
			if ( nReadSamples == 0 )
			{
				pbytBuf = m_queSoundBuf.PutBuffer( nBytes ) ;
				eslFillMemory( pbytBuf, 0, nBytes ) ;
				m_queSoundBuf.FlushBuffer( (size_t) nBytes ) ;
				m_nSoundPlayPos += nSamples ;
			}
			//
			StreamPitchSound() ;
		}
	}
}

// 音声のピッチ処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::StreamPitchSound( void )
{
	size_t	nBufBytes = (size_t) m_queSoundBuf.GetLength() ;
	if ( nBufBytes == 0 )
	{
		return ;
	}
	const uint8_t *	pbytQueBuf = m_queSoundBuf.GetBuffer( nBufBytes ) ;
	if ( (pbytQueBuf == nullptr)
		|| (fabs(m_fpPitch - 1.0) < 0.001)
		|| (m_soundFormat.bitsPerSample != 16) )
	{
		//
		// ピッチ変換無し出力
		//
		size_t	nWrittenBytes = m_soundPlayer.Write( pbytQueBuf, nBufBytes ) ;
		m_queSoundBuf.ReleaseBuffer( (ssize_t) nWrittenBytes ) ;
		return ;
	}
	//
	// ピッチ変換
	//
	const size_t	nBytesInSample = m_soundFormat.channels
										* m_soundFormat.bitsPerSample / 8 ;
	const size_t	nSrcSamples = nBufBytes / nBytesInSample ;
	if ( nSrcSamples == 0 )
	{
		m_queSoundBuf.ReleaseBuffer( 0 ) ;
		return ;
	}
	const size_t	nOutSamples = (size_t) (nSrcSamples / m_fpPitch) ;
	uint8_t *		pbytOutBuf = m_queSoundOutBuf.PutBuffer
										( nOutSamples * nBytesInSample ) ;
	const size_t	nChannels = m_soundFormat.channels ;
	for ( size_t iDst = 0; iDst < nOutSamples; iDst ++ )
	{
		int64_t	ifxSrcSample = (int64_t) (iDst * m_fpPitch * 0x100) ;
		size_t	iSrcSample0 = (size_t) (ifxSrcSample >> 8) ;
		size_t	iSrcSample1 = iSrcSample0 + 1 ;
		int32_t	fxSrcSample = (int32_t) (ifxSrcSample & 0xFF) ;
		if ( iSrcSample0 >= nSrcSamples - 1 )
		{
			iSrcSample0 = nSrcSamples - 1 ;
			iSrcSample1 = iSrcSample0 ;
			fxSrcSample = 0 ;
		}
		int16_t *		pwOutBuf = ((int16_t*) pbytOutBuf) + iDst * nChannels ;
		const int16_t *	pwSrcBuf0 = ((const int16_t*) pbytQueBuf) + iSrcSample0 * nChannels ;
		const int16_t *	pwSrcBuf1 = ((const int16_t*) pbytQueBuf) + iSrcSample1 * nChannels ;
		for ( size_t ch = 0; ch < nChannels; ch ++ )
		{
			int32_t	src0 = pwSrcBuf0[ch] ;
			int32_t	src1 = pwSrcBuf1[ch] ;
			pwOutBuf[ch] = (int16_t) (src0 + (src1 - src0) * fxSrcSample / 0x100) ;
		}
	}
	m_queSoundBuf.ReleaseBuffer( (ssize_t) nBufBytes ) ;
	m_queSoundOutBuf.FlushBuffer( nOutSamples * nBytesInSample ) ;
	//
	// ピッチ変換後出力
	//
	size_t			nPitchBufBytes = (size_t) m_queSoundOutBuf.GetLength() ; 
	const uint8_t *	pbytPitchBuf = m_queSoundOutBuf.GetBuffer( nPitchBufBytes ) ;
	//
	size_t	nWrittenBytes = m_soundPlayer.Write( pbytPitchBuf, nPitchBufBytes ) ;
	m_queSoundOutBuf.ReleaseBuffer( (ssize_t) nWrittenBytes ) ;
}

// 音量反映
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::RelfectAudioVolume( void )
{
	float32_t	fpVol[2] ;
	fpVol[0] = (float32_t) (m_fpVolume * esl_fclamp( 1.0 - m_fpPan, 0.0, 1.0 )) ;
	fpVol[1] = (float32_t) (m_fpVolume * esl_fclamp( 1.0 + m_fpPan, 0.0, 1.0 )) ;
	m_soundPlayer.SetVolume( fpVol, 2 ) ;
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasAudioTrackController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVolume:
		return	m_fpVolume ;

	case	paramPan:
		return	m_fpPan ;

	case	paramPitch:
		return	m_fpPitch ;
	}
	return	S3DCanvasBasicMediaController::GetScalarParameter( i ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramVolume:
		m_fpVolume = s ;
		return ;

	case	paramPan:
		m_fpPan = s ;
		return ;

	case	paramPitch:
		m_fpPitch = s ;
		return ;
	}
	S3DCanvasBasicMediaController::SetScalarParameter( i, s ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasAudioTrackController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramMediaSource:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				pComposer->GetAssets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLAudioPlayerInterface) ) ;
			}
		}
		return	true ;
	}
	return	S3DCanvasBasicMediaController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasAudioTrackController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
	case	paramCenter:
	case	paramZoom:
	case	paramRotation:
	case	paramTransparency:
	case	paramVisible:
		return	false ;
	}
	return	S3DCanvasBasicMediaController::IsParameterValidation( i ) ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasAudioTrackController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
}


////////////////////////////////////////////////////////////////////////////////////
// 動画トラックコントローラー
////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasMovieTrackController, S3DCanvasAudioTrackController )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DCanvasMovieTrackController, movie_track )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////
S3DCanvasMovieTrackController::S3DCanvasMovieTrackController( void )
	: S3DCanvasAudioTrackController
			( S3DCanvasMovieTrackController::m_ItemClassDescriptor.pwszClassID ),
		m_pRefMovie( nullptr ), m_mediaPlayer( nullptr ),
		m_videoStream( nullptr ), m_nFrameIndex( 0 ),
		m_rectCutOff( 0, 0, 0, 0 ),
		m_flagKeepVisible( true ), m_flagOutOfDuration( false )
{
	ESLVerify( AddParameterEntry
		( L"cut_left",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"左辺切り落とし", nullptr ) == paramCutLeft ) ;
	ESLVerify( AddParameterEntry
		( L"cut_top",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"上辺切り落とし", nullptr ) == paramCutTop ) ;
	ESLVerify( AddParameterEntry
		( L"cut_right",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"右辺切り落とし", nullptr ) == paramCutRight ) ;
	ESLVerify( AddParameterEntry
		( L"cut_bottom",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"下辺切り落とし", nullptr ) == paramCutBottom ) ;
	ESLVerify( AddParameterEntry
		( L"keep_visible",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"再生終了後も表示", nullptr ) == paramKeepVisible ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////
S3DCanvasMovieTrackController::~S3DCanvasMovieTrackController( void )
{
	ReleaseVideoStream() ;
}

// ビデオストリーム解放
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::ReleaseVideoStream( void )
{
	if ( (m_mediaPlayer != nullptr) && (m_videoStream != nullptr) )
	{
		m_mediaPlayer->ReleaseVideoStream( m_videoStream ) ;
		m_videoStream = nullptr ;
	}
	m_mediaPlayer = nullptr ;
	m_pRefMovie = nullptr ;
	m_pFrame = nullptr ;
}

// ビデオフレームシーク
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::SeekVideoFrame( double secTime )
{
	m_flagOutOfDuration = true ;
	if ( m_videoStream != nullptr )
	{
		int64_t	nVideoFrames = m_videoStream->GetVideoLength() ;
		int64_t	msecDuration = m_videoStream->GetVideoDuration() ;
		if ( (nVideoFrames > 0) && (msecDuration > 0) )
		{
			uint64_t	nFrameIndex =
				(uint64_t) (nVideoFrames * secTime * 1000.0 / msecDuration) ;
			if ( (m_nFrameIndex != nFrameIndex)
				&& (nFrameIndex < (uint64_t) nVideoFrames) )
			{
				if ( m_nFrameIndex + 1 != nFrameIndex )
				{
					m_videoStream->SeekFrame( nFrameIndex ) ;
				}
				m_nFrameIndex = nFrameIndex ;
				//
				ReadVideoFrame() ;
			}
			m_flagOutOfDuration = (secTime * 1000.0 > msecDuration) ;
		}
	}
}

// ビデオフレーム読み込み
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::ReadVideoFrame( void )
{
	if ( m_pFrame != nullptr )
	{
		SGLImageInfo	imginf ;
		uint8_t *	pbytBuf =
			m_pFrame->LockBuffer( imginf, SGLImageObject::lockWrite ) ;
		m_videoStream->ReadFrame( imginf, pbytBuf ) ;
		m_pFrame->UnlockBuffer( SGLImageObject::lockWrite ) ;
	}
}

// ソース更新
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::UpdateMediaSource( void )
{
	if ( m_strSource.IsEmpty() )
	{
		ReleaseVideoStream() ;
		ReleaseAudioStream() ;
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		SGLMediaPlayerInterface *	pMovie =
				pComposer->GetAssets().GetMovieAs( m_strSource ) ;
		if ( (pMovie != nullptr) && (m_pRefMovie != pMovie) )
		{
			m_pRefMovie = pMovie ;
			m_mediaPlayer = ESLSmartCast<SGLMediaPlayerInterface>( pMovie->ClonePlayer() ) ;
			if ( m_mediaPlayer != nullptr )
			{
				m_audioPlayer = m_mediaPlayer ;
				m_audioStream = m_audioPlayer->GetAudioStream() ;
				m_videoStream = m_mediaPlayer->GetVideoStream() ;
				//
				if ( (m_videoStream != nullptr)
					&& (m_videoStream->GetImageFormat( m_imginf ) == sglErrSuccess) )
				{
					m_pFrame = new SGLImage ;
					m_pFrame->CreateBuffer( m_imginf ) ;
					//
					m_nFrameIndex = (uint64_t) -1 ;
					SeekVideoFrame( m_secSeek ) ;
				}
				//
				S3DSceneComposer::ParamEntry&	peSeek = ParameterEntryAt( paramSeek ) ;
				peSeek.maxRange = (double) m_mediaPlayer->GetTotalLength()
									/ (double) m_mediaPlayer->GetSampleFrequency() ;
				//
				m_flagStarted = false ;
				m_flagPaused = false ;
				m_secPlayingPos = m_secSeek ;
			}
		}
		else if ( pMovie == nullptr )
		{
			ReleaseVideoStream() ;
			ReleaseAudioStream() ;
		}
	}
}

// 再生開始
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnPlayStart( double secTime )
{
	if ( m_videoStream != nullptr )
	{
		SeekVideoFrame( secTime ) ;
	}
	S3DCanvasAudioTrackController::OnPlayStart( secTime ) ;
}

// 再生一時停止
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnPlayPause( void )
{
	S3DCanvasAudioTrackController::OnPlayPause() ;
}

// 再生再開
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnPlayRestart( void )
{
	S3DCanvasAudioTrackController::OnPlayRestart() ;
}

// 再生停止
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnPlayEnd( void )
{
	S3DCanvasAudioTrackController::OnPlayEnd() ;
}

// フレーム更新
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnUpdateMediaFrame( double secTime )
{
	if ( m_videoStream != nullptr )
	{
		SeekVideoFrame( secTime ) ;
	}
	S3DCanvasAudioTrackController::OnUpdateMediaFrame( secTime ) ;
}

// メディアの長さ[秒]を取得する
////////////////////////////////////////////////////////////////////////////////////
double S3DCanvasMovieTrackController::GetMediaDuration( void )
{
	if ( m_videoStream != nullptr )
	{
		return	m_videoStream->GetVideoDuration() / 1000.0 ;
	}
	return	S3DCanvasAudioTrackController::GetMediaDuration() ;
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////
int32_t S3DCanvasMovieTrackController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramCutLeft:
		return	m_rectCutOff.left ;
	case	paramCutTop:
		return	m_rectCutOff.top ;
	case	paramCutRight:
		return	m_rectCutOff.right ;
	case	paramCutBottom:
		return	m_rectCutOff.bottom ;
	}
	return	S3DCanvasAudioTrackController::GetIntegerParameter( iParam ) ;
}

bool S3DCanvasMovieTrackController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramKeepVisible:
		return	m_flagKeepVisible ;
	}
	return	S3DCanvasAudioTrackController::GetBooleanParameter( iParam ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramCutLeft:
		m_rectCutOff.left = n ;
		return ;
	case	paramCutTop:
		m_rectCutOff.top = n ;
		return ;
	case	paramCutRight:
		m_rectCutOff.right = n ;
		return ;
	case	paramCutBottom:
		m_rectCutOff.bottom = n ;
		return ;
	}
	S3DCanvasAudioTrackController::SetIntegerParameter( iParam, n ) ;
}

void S3DCanvasMovieTrackController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramKeepVisible:
		m_flagKeepVisible = b ;
		return ;
	}
	S3DCanvasAudioTrackController::SetBooleanParameter( iParam, b ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasMovieTrackController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramMediaSource:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				pComposer->GetAssets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLMediaPlayerInterface) ) ;
			}
		}
		return	true ;
	}
	return	S3DCanvasAudioTrackController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasMovieTrackController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
	case	paramCenter:
	case	paramZoom:
	case	paramRotation:
	case	paramTransparency:
	case	paramVisible:
		return	true ;
	}
	return	S3DCanvasAudioTrackController::IsParameterValidation( i ) ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasMovieTrackController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	if ( (m_pFrame != nullptr)
		&& (m_flagKeepVisible || !m_flagOutOfDuration) )
	{
		SGLSize			sizeFrame = m_pFrame->GetImageSize() ;
		SGLImageRect	rectFrame( m_rectCutOff.left, m_rectCutOff.top,
									sizeFrame.w - (m_rectCutOff.left + m_rectCutOff.right),
									sizeFrame.h - (m_rectCutOff.top + m_rectCutOff.bottom) ) ;
		if ( !rectFrame.IsEmpty() )
		{
			SGLPaintParam	param ;
			param.nFlags = paintDelayable | paintSmoothStretch ;
			dipl.AddDrawParam( param, m_pFrame, &rectFrame ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// コンポジション描画コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasCompositionTrackController, S3DCanvasBasicMediaController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasCompositionTrackController, comp_track )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCanvasCompositionTrackController::S3DCanvasCompositionTrackController( void )
	: S3DCanvasBasicMediaController( m_ItemClassDescriptor.pwszClassID ),
		m_sizeFrameBuf( 0, 0 ),
		m_flagUpdateFrame( true ),
		m_flagPlaying( false ),
		m_flagPaused( false ),
		m_flagOutOfDuration( false ),
		m_flagKeepVisible( false ),
		m_fpLastFrame( 0.0 ),
		m_pciCompInfo( nullptr )
{
	ESLVerify( AddParameterEntry
		( L"play_speed",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIScalarSlider,
			L"再生速度", L"再生速度比", 0.125, 8.0 ) == paramSpeed ) ;
	ESLVerify( AddParameterEntry
		( L"keep_visible",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"再生終了後も表示", nullptr ) == paramKeepVisible ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCanvasCompositionTrackController::~S3DCanvasCompositionTrackController( void )
{
	ReleaseComposition() ;
}

// ソース更新
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::UpdateMediaSource( void )
{
	ReleaseComposition() ;
	m_flagUpdateFrame = true ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_flagOutOfDuration = false ;
	m_fpLastFrame = 0.0 ;
	//
	if ( m_strSource.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		m_pciCompInfo = pComposer->GetCompositionAs( m_strSource ) ;
		if ( m_pciCompInfo != nullptr )
		{
			m_pComposition = pComposer->CreateComposition( *m_pciCompInfo ) ;
			if ( m_pComposition != nullptr )
			{
				UpdateFrameSize() ;
				//
				S3DScene::Camera *	pCamera =
					ESLTypeCast<S3DScene::Camera>
						( m_pComposition->GetSceneItemAs
							( m_pciCompInfo->GetDefaultCameraID() ) ) ;
				if ( pCamera == nullptr )
				{
					pCamera = &m_cameraDummy ;
				}
				//
				m_scene.GetRootSpace().AddChild( m_pComposition ) ;
				m_scene.SetMainCamera( pCamera ) ;
				//
				m_pComposition->EnableTimerEvent( false, true ) ;
				m_pComposition->InitializeFrameParameters() ;
				m_pComposition->InitializeItems( &m_scene ) ;
				m_pComposition->OnSetupSceneSettings( m_scene, m_sizeFrameBuf, nullptr ) ;
				m_pComposition->SetAllItemsFrame( 0, S3DSceneComposer::seekJumpReset ) ;
			}
		}
	}
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnPlayStart( double secTime )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	ESLAssert( m_pciCompInfo != nullptr ) ;
	double	fpFrame = m_pciCompInfo->FrameIndexFromSecond( secTime ) ;
	m_flagOutOfDuration = (fpFrame < 0.0)
						|| (fpFrame >= m_pciCompInfo->GetTotalFrameCount()) ;
	//
	m_pComposition->SetAllItemsFrame( fpFrame, S3DSceneComposer::seekJumpReset ) ;
	m_pComposition->OnExtendNotify( S3DSceneComposer::CmdStartItem, NULL, NULL, 0 ) ;
	m_flagUpdateFrame = true ;
	m_flagPlaying = true ;
	m_flagPaused = false ;
	m_fpLastFrame = fpFrame ;
}

// 再生一時停止
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnPlayPause( void )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	m_pComposition->SetAllItemsFrame( m_fpLastFrame, S3DSceneComposer::seekJumpReset ) ;
	m_pComposition->OnExtendNotify( S3DSceneComposer::CmdStopItem, NULL, NULL, 0 ) ;
	m_flagPaused = m_flagPlaying ;
}

// 再生再開
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnPlayRestart( void )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	if ( m_flagPlaying )
	{
		m_pComposition->OnExtendNotify( S3DSceneComposer::CmdStartItem, NULL, NULL, 0 ) ;
		m_flagPaused = false ;
	}
}

// 再生停止
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnPlayEnd( void )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	m_pComposition->StopCompositoin() ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
}

// フレーム更新
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnUpdateMediaFrame( double secTime )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	ESLAssert( m_pciCompInfo != nullptr ) ;
	double	fpFrame = m_pciCompInfo->FrameIndexFromSecond( secTime ) ;
	m_flagOutOfDuration = (fpFrame < 0.0)
						|| (fpFrame >= m_pciCompInfo->GetTotalFrameCount()) ;
	//
	if ( !m_flagOutOfDuration )
	{
		S3DSceneComposer::SeekMethod	seek = S3DSceneComposer::seekStream ;
		if ( !m_flagPlaying )
		{
			seek = S3DSceneComposer::seekJumpReset ;
		}
		else if ( m_flagPaused )
		{
			seek = S3DSceneComposer::seekStreamPaused ;
		}
		m_pComposition->SetAllItemsFrame( fpFrame, seek ) ;
		m_flagUpdateFrame = true ;
		m_fpLastFrame = fpFrame ;
	}
}

// メディアの長さ[秒]を取得する
//////////////////////////////////////////////////////////////////////////////
double S3DCanvasCompositionTrackController::GetMediaDuration( void )
{
	if ( m_pciCompInfo != nullptr )
	{
		return	m_pciCompInfo->FrameIndexToSecond
						( (double) m_pciCompInfo->GetTotalFrameCount() ) ;
	}
	return	0.0 ;
}

// コンポジション解放
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::ReleaseComposition( void )
{
	if ( m_pComposition == nullptr )
	{
		return ;
	}
	m_pComposition->FinishComposition() ;
	m_pComposition->OnShoutdownSceneSettings( m_scene ) ;
	m_scene.GetRootSpace().RemoveChild( m_pComposition ) ;
	m_scene.SetMainCamera( nullptr ) ;
	//
	m_pciCompInfo = nullptr ;
	m_pComposition = nullptr ;
}

// フレームサイズを更新する
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::UpdateFrameSize( void )
{
	if ( m_pciCompInfo == nullptr )
	{
		return ;
	}
	if ( m_sizeFrameBuf == m_pciCompInfo->GetScreenSize() )
	{
		return ;
	}
	m_sizeFrameBuf = m_pciCompInfo->GetScreenSize() ;
	//
	for ( int i = 0; i < renderTargetAllCount; i ++ )
	{
		m_imgRender[i].CreateImage
			( m_sizeFrameBuf.w, m_sizeFrameBuf.h,
				formatImageARGB, 32,
				SGLImageObject::bufferForRenderTarget ) ;
	}
	m_imgZBuffer.CreateImage
			( m_sizeFrameBuf.w, m_sizeFrameBuf.h,
				formatImageDepth, 32,
				SGLImageObject::bufferOnDeviceOnly
				| SGLImageObject::bufferForRenderTarget ) ;
	m_imgLayeredZBuffer.CreateImage
			( m_sizeFrameBuf.w, m_sizeFrameBuf.h,
				formatImageDepth, 32,
				SGLImageObject::bufferOnDeviceOnly
				| SGLImageObject::bufferForRenderTarget ) ;
	//
	SGLImageObject *	pMultiTarget[renderTargetAllCount] ;
	for ( int i = 0; i < renderTargetAllCount; i ++ )
	{
		pMultiTarget[i] = &m_imgRender[i] ;
	}
	m_pciCompInfo->ApplyAllParameters( m_scene, m_sizeFrameBuf ) ;
	m_scene.AttachLayeredRenderTarget
		( &m_imgRender[renderLayeredBuffer], &m_imgLayeredZBuffer ) ;
	m_scene.AttachMultiRenderTarget
			( &pMultiTarget[1], S3DScene::renderTargetCount - 1 ) ;
	m_scene.AttachTemporaryRenderBuffers
			( &pMultiTarget[renderTargetTemporary0], 2 ) ;
	//
	m_flagUpdateFrame = true ;
}

// フレームを更新する
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::UpdateFrameImage( void )
{
	UpdateFrameSize() ;
	//
	if ( (m_pciCompInfo == nullptr) || !m_flagUpdateFrame )
	{
		return ;
	}
	m_render.AttachTargetImage( &m_imgRender[0], &m_imgZBuffer ) ;
	m_render.FillClearTarget( m_pciCompInfo->GetFillBack() ) ;
	//
	m_scene.RenderScene( m_scene.GetMainCamera(), &m_render ) ;
	//
	m_render.DetachTargetImage() ;
	m_flagUpdateFrame = false ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DCanvasCompositionTrackController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramSpeed:
		return	m_fpPitch ;
	}
	return	S3DCanvasBasicMediaController::GetScalarParameter( iParam ) ;
}

bool S3DCanvasCompositionTrackController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramKeepVisible:
		return	m_flagKeepVisible ;
	}
	return	S3DCanvasBasicMediaController::GetBooleanParameter( iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramSpeed:
		m_fpPitch = s ;
		return ;
	}
	S3DCanvasBasicMediaController::SetScalarParameter( iParam, s ) ;
}

void S3DCanvasCompositionTrackController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramKeepVisible:
		m_flagKeepVisible = b ;
		return ;
	}
	S3DCanvasBasicMediaController::SetBooleanParameter( iParam, b ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCanvasCompositionTrackController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	switch ( iParam )
	{
	case	paramMediaSource:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				pComposer->EnumerateCompositionIDs( aStrSet ) ;
			}
		}
		return	true ;
	}
	return	S3DCanvasBasicMediaController::EnumerateStringSet( iParam, aStrSet ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
void S3DCanvasCompositionTrackController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	if ( (m_pComposition != nullptr)
		&& (m_flagKeepVisible || !m_flagOutOfDuration) )
	{
		UpdateFrameImage() ;
		//
		SGLPaintParam	param ;
		param.nFlags = paintDelayable | paintSmoothStretch ;
		dipl.AddDrawParam( param, &m_imgRender[0] ) ;
	}
}



////////////////////////////////////////////////////////////////////////////////////////
// 画像表示コントローラー
////////////////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DCanvasTextController::m_aiUsageTypes
		[S3DCanvasTextController::textUsageTypeCount+1] =
{
	{ L"plain", S3DCanvasTextController::textPlain },
	{ L"c_string", S3DCanvasTextController::textCString },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DCanvasTextController::m_aiAlignTypes[4] =
{
	{ L"left", SGLLetteringContext::alignLeft },
	{ L"right", SGLLetteringContext::alignRight },
	{ L"center", SGLLetteringContext::alignCenter },
	{ nullptr, 0 },
} ;

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasTextController, S3DCanvasBasicController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasTextController, canvas_text )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasTextController::S3DCanvasTextController( void )
	: S3DCanvasBasicController( m_ItemClassDescriptor.pwszClassID ),
		m_usageText( textPlain ),
		m_strFont( SGLFontStyle::StandardFont ),
		m_nFontSize( 16 ), m_nLineWidth( 10000 ),
		m_flagVCenter( true ),
		m_flagGradation( false ), m_flagTextModified( false )
{
	m_decoration.rgbaBody = 0xFFFFFFFF ;
	m_decoration.rgbaBorder = 0xFF000000 ;
	m_decoration.rgbaBorder2 = 0xFFFFFFFF ;
	m_decoration.rgbaShadow = 0x00000000 ;
	m_decoration.ptShadow.x = 1 ;
	m_decoration.ptShadow.y = 1 ;
	m_argbGradation[1] = 0xFF00FFFF ;
	//
	PrepareParameterEntryCount( paramTextCount ) ;
	ESLVerify( AddParameterEntry
		( L"text",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1,
			L"テキスト", nullptr ) == paramText ) ;
	ESLVerify( AddParameterEntry
		( L"text_usage",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"テキスト文法", nullptr ) == paramTextUsage ) ;
	ESLVerify( AddParameterEntry
		( L"font",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrStringEnumeration,
			L"フォント名", nullptr ) == paramFontFace ) ;
	ESLVerify( AddParameterEntry
		( L"font_size",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"フォントサイズ", nullptr ) == paramFontSize ) ;
	ESLVerify( AddParameterEntry
		( L"text_color",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory2,
			L"文字色", nullptr ) == paramTextColor ) ;
	ESLVerify( AddParameterEntry
		( L"use_gradation",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory2,
			L"グラデーション有効", nullptr ) == paramGradation ) ;
	ESLVerify( AddParameterEntry
		( L"gradation_color",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory2,
			L"グラデーション色", nullptr ) == paramGradationColor ) ;
	ESLVerify( AddParameterEntry
		( L"border_width1",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory2,
			L"文字縁幅1", nullptr ) == paramBorderWidth1 ) ;
	ESLVerify( AddParameterEntry
		( L"border_color1",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory2,
			L"文字縁色1", nullptr ) == paramBorderColor1 ) ;
	ESLVerify( AddParameterEntry
		( L"border_width2",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory2,
			L"文字縁幅2", nullptr ) == paramBorderWidth2 ) ;
	ESLVerify( AddParameterEntry
		( L"border_color2",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory2,
			L"文字縁色2", nullptr ) == paramBorderColor2 ) ;
	ESLVerify( AddParameterEntry
		( L"shadow_alpha",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory2
			| S3DSceneComposer::attrUIScalarSlider,
			L"文字影α", nullptr, 0.0, 1.0 ) == paramShadowAlpha ) ;
	ESLVerify( AddParameterEntry
		( L"shadow_color",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrCategory2,
			L"文字影色", nullptr ) == paramShadowColor ) ;
	ESLVerify( AddParameterEntry
		( L"shadow_offset_x",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory2,
			L"文字影オフセットｘ", nullptr ) == paramShadowOffsetX ) ;
	ESLVerify( AddParameterEntry
		( L"shadow_offset_y",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory2,
			L"文字影オフセットｙ", nullptr ) == paramShadowOffsetY ) ;
	ESLVerify( AddParameterEntry
		( L"line_width",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"行幅", nullptr ) == paramLineWidth ) ;
	ESLVerify( AddParameterEntry
		( L"text_align",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory3
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"アライメント", nullptr ) == paramAlign ) ;
	ESLVerify( AddParameterEntry
		( L"vert_center",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory3,
			L"垂直センタリング", nullptr ) == paramVCenter ) ;
	ESLVerify( AddParameterEntry
		( L"char_pitch",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"文字ピッチ", L"※ 0 の時はデフォルトのピッチ" ) == paramCharPitch ) ;
	ESLVerify( AddParameterEntry
		( L"pitch_offset",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"ピッチオフセット", nullptr ) == paramPitchOffset ) ;
	ESLVerify( AddParameterEntry
		( L"pitch_scale",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory3
			| S3DSceneComposer::attrUIScalarSlider,
			L"ピッチスケール", nullptr, 0.5, 2.0 ) == paramPitchScale ) ;
	ESLVerify( AddParameterEntry
		( L"in_half_offset",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"半角前ピッチ", nullptr ) == paramInHalfOffset ) ;
	ESLVerify( AddParameterEntry
		( L"out_half_offset",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"半角後ピッチ", nullptr ) == paramOutHalfOffset ) ;
	ESLVerify( AddParameterEntry
		( L"line_pitch",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory3,
			L"行間", L"※ 0 の時はデフォルトの行間" ) == paramLinePitch ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasTextController::~S3DCanvasTextController( void )
{
}

// 文字外観画像更新
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasTextController::UpdateTextImage( void )
{
	if ( m_strText.IsEmpty() )
	{
		Lock() ;
		m_pTextImage = nullptr ;
		m_flagTextModified = false ;
		Unlock() ;
		return ;
	}

	m_lettering.ptStartWriting.x = 0 ;
	m_lettering.ptStartWriting.y = 0 ;
	m_lettering.rectWritable.left = 0 ;
	m_lettering.rectWritable.top = 0 ;
	m_lettering.rectWritable.right = m_nLineWidth ;
	m_lettering.rectWritable.bottom = 0x7FFF ;
	//
	m_decoration.nFlags = 0 ;
	if ( m_decoration.widthBorder > 0 )
	{
		m_decoration.nFlags |= SGLLetterer::flagBorder ;
	}
	if ( m_decoration.widthBorder2 > 0 )
	{
		m_decoration.nFlags |= SGLLetterer::flagBorder2 ;
	}
	if ( m_decoration.rgbaShadow != 0 )
	{
		m_decoration.nFlags |= SGLLetterer::flagShadow ;
	}
	if ( m_flagGradation )
	{
		m_decoration.nFlags |= SGLLetterer::flagGradation ;
		m_argbGradation[0] = m_decoration.rgbaBody ;
		m_decoration.pGradation = m_argbGradation ;
		m_decoration.nGradationCount = 2 ;
		m_decoration.nGradationHeight = 0 ;
	}
	//
	SGLFontStyle	styleFont ;
	styleFont.pszFace = m_strFont ;
	styleFont.nSize = m_nFontSize ;
	//
	SGLFont	font ;
	font.SetStyle( styleFont ) ;
	//
	SString	strText ;
	switch ( m_usageText )
	{
	case	textCString:
		SStringParser::DecodeCLangString( strText, m_strText ) ;
		break ;
	default:
		strText = m_strText ;
		break ;
	}
	SGLLetterer	letterer ;
	letterer.WriteLetter( font, m_lettering, strText ) ;
	letterer.CombineLetter() ;
	letterer.DecorateLetter( m_decoration ) ;
	//
	SGLImageRect	rect ;
	letterer.GetLetterRect( rect ) ;
	//
	if ( rect.IsEmpty() )
	{
		Lock() ;
		m_pTextImage = nullptr ;
		m_flagTextModified = false ;
		Unlock() ;
		return ;
	}

	SGLImage *	pImage = new SGLImage ;
	pImage->CreateImage( rect.w + 2, rect.h + 2, formatImageARGB, 32 ) ;
	//
	SGLPaintBuffer	paint ;
	paint.AttachTargetImage( pImage, nullptr, nullptr ) ;
	letterer.DrawLetterTo( paint, 1 - rect.x, 1 - rect.y ) ;
	paint.DetachTargetImage() ;
	//
	int	yCenter = 1 - rect.y ;
	if ( m_flagVCenter )
	{
		yCenter = rect.h / 2 ;
	}
	switch ( m_lettering.typeAlignment )
	{
	case	SGLLetteringContext::alignLeft:
	default:
		pImage->SetImageOrigin( 1 - rect.x, yCenter ) ;
		break ;
	case	SGLLetteringContext::alignRight:
		pImage->SetImageOrigin( rect.w + 2, yCenter ) ;
		break ;
	case	SGLLetteringContext::alignCenter:
		pImage->SetImageOrigin( rect.w / 2, yCenter ) ;
		break ;
	}
	//
	Lock() ;
	m_pTextImage = pImage ;
	m_flagTextModified = false ;
	Unlock() ;
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCanvasTextController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTextColor:
		return	VectorFromColor( m_decoration.rgbaBody ) ;

	case	paramGradationColor:
		return	VectorFromColor( m_argbGradation[1] ) ;

	case	paramBorderColor1:
		return	VectorFromColor( m_decoration.rgbaBorder ) ;

	case	paramBorderColor2:
		return	VectorFromColor( m_decoration.rgbaBorder2 ) ;

	case	paramShadowColor:
		return	VectorFromColor( m_decoration.rgbaShadow ) ;
	}
	return	S3DCanvasBasicController::GetVectorParameter( i ) ;
}

double S3DCanvasTextController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramShadowAlpha:
		return	(double) m_decoration.rgbaShadow.argb.Alpha / 255.0 ;

	case	paramPitchScale:
		return	(double) m_lettering.scalePitch / 0x10000 ;
	}
	return	S3DCanvasBasicController::GetScalarParameter( i ) ;
}

int32_t S3DCanvasTextController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFontSize:
		return	m_nFontSize ;

	case	paramBorderWidth1:
		return	m_decoration.widthBorder ;

	case	paramBorderWidth2:
		return	m_decoration.widthBorder2 ;

	case	paramShadowOffsetX:
		return	m_decoration.ptShadow.x ;

	case	paramShadowOffsetY:
		return	m_decoration.ptShadow.y ;

	case	paramLineWidth:
		return	m_nLineWidth ;

	case	paramCharPitch:
		return	m_lettering.pitchChar ;

	case	paramPitchOffset:
		return	m_lettering.offsetChar ;

	case	paramInHalfOffset:
		return	m_lettering.offsetInHalf ;

	case	paramOutHalfOffset:
		return	m_lettering.offsetOutHalf ;

	case	paramLinePitch:
		return	m_lettering.pitchLine ;
	}
	return	S3DCanvasBasicController::GetIntegerParameter( i ) ;
}

bool S3DCanvasTextController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGradation:
		return	m_flagGradation ;

	case	paramVCenter:
		return	m_flagVCenter ;

	}
	return	S3DCanvasBasicController::GetBooleanParameter( i ) ;
}

const wchar_t * S3DCanvasTextController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramText:
		return	m_strText ;

	case	paramTextUsage:
		return	SXMLDocument::GetSymbolAsIntegerOf
						( m_aiUsageTypes, m_usageText ) ;

	case	paramFontFace:
		return	m_strFont ;

	case	paramAlign:
		return	SXMLDocument::GetSymbolAsIntegerOf
						( m_aiAlignTypes, m_lettering.typeAlignment ) ;
	}
	return	S3DCanvasBasicController::GetCommandParameter( i ) ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasTextController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	SGLPalette	argbColor ;
	switch ( i )
	{
	case	paramTextColor:
		argbColor = (ColorFromVector( vec ).ui32 & 0x00FFFFFF) | 0xFF000000 ;
		m_flagTextModified |= (m_decoration.rgbaBody != argbColor) ;
		m_decoration.rgbaBody = argbColor ;
		return ;

	case	paramGradationColor:
		argbColor = (ColorFromVector( vec ).ui32 & 0x00FFFFFF) | 0xFF000000 ;
		m_flagTextModified |= (m_argbGradation[1] != argbColor) ;
		m_argbGradation[1] = argbColor ;
		return ;

	case	paramBorderColor1:
		argbColor = (ColorFromVector( vec ).ui32 & 0x00FFFFFF) | 0xFF000000 ;
		m_flagTextModified |= (m_decoration.rgbaBorder != argbColor) ;
		m_decoration.rgbaBorder = argbColor;
		return ;

	case	paramBorderColor2:
		argbColor = (ColorFromVector( vec ).ui32 & 0x00FFFFFF) | 0xFF000000 ;
		m_flagTextModified |= (m_decoration.rgbaBorder2 != argbColor) ;
		m_decoration.rgbaBorder2 = argbColor;
		return ;

	case	paramShadowColor:
		argbColor = (ColorFromVector( vec ).ui32 & 0x00FFFFFF)
						| (m_decoration.rgbaShadow.ui32 & 0xFF000000) ;
		m_flagTextModified |= (m_decoration.rgbaShadow != argbColor) ;
		m_decoration.rgbaShadow = argbColor ;
		return ;
	}
	S3DCanvasBasicController::SetVectorParameter( i, vec ) ;
}

void S3DCanvasTextController::SetScalarParameter( size_t i, double s )
{
	uint8_t	alpha ;
	int32_t	scale ;
	switch ( i )
	{
	case	paramShadowAlpha:
		alpha = (uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 0xFF ) ;
		m_flagTextModified |= (m_decoration.rgbaShadow.argb.Alpha != alpha) ;
		m_decoration.rgbaShadow.argb.Alpha = alpha ;
		return ;

	case	paramPitchScale:
		scale = (int32_t) esl_lroundfi( s * 0x10000 ) ;
		m_flagTextModified |= (m_lettering.scalePitch != scale) ;
		m_lettering.scalePitch = scale ;
		return ;
	}
	S3DCanvasBasicController::SetScalarParameter( i, s ) ;
}

void S3DCanvasTextController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramFontSize:
		m_flagTextModified |= (m_nFontSize != n) ;
		m_nFontSize = n ;
		return ;

	case	paramBorderWidth1:
		m_flagTextModified |= (m_decoration.widthBorder != n) ;
		m_decoration.widthBorder = n ;
		return ;

	case	paramBorderWidth2:
		m_flagTextModified |= (m_decoration.widthBorder2 != n) ;
		m_decoration.widthBorder2 = n ;
		return ;

	case	paramShadowOffsetX:
		m_flagTextModified |= (m_decoration.ptShadow.x != n) ;
		m_decoration.ptShadow.x = n ;
		return ;

	case	paramShadowOffsetY:
		m_flagTextModified |= (m_decoration.ptShadow.y != n) ;
		m_decoration.ptShadow.y = n ;
		return ;

	case	paramLineWidth:
		m_flagTextModified |= (m_nLineWidth != n) ;
		m_nLineWidth = n ;
		return ;

	case	paramCharPitch:
		m_flagTextModified |= (m_lettering.pitchChar != n) ;
		m_lettering.pitchChar = n ;
		return ;

	case	paramPitchOffset:
		m_flagTextModified |= (m_lettering.offsetChar != n) ;
		m_lettering.offsetChar = n ;
		return ;

	case	paramInHalfOffset:
		m_flagTextModified |= (m_lettering.offsetInHalf != n) ;
		m_lettering.offsetInHalf = n ;
		return ;

	case	paramOutHalfOffset:
		m_flagTextModified |= (m_lettering.offsetOutHalf != n) ;
		m_lettering.offsetOutHalf = n ;
		return ;

	case	paramLinePitch:
		m_flagTextModified |= (m_lettering.pitchLine != n) ;
		m_lettering.pitchLine = n ;
		return ;
	}
	S3DCanvasBasicController::SetIntegerParameter( i, n ) ;
}

void S3DCanvasTextController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramGradation:
		m_flagTextModified |= (m_flagGradation != b) ;
		m_flagGradation = b ;
		return ;

	case	paramVCenter:
		m_flagTextModified |= (m_flagVCenter != b) ;
		m_flagVCenter = b ;
		return ;
	}
	S3DCanvasBasicController::SetBooleanParameter( i, b ) ;
}

void S3DCanvasTextController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	TextUsageType						textUsage ;
	SGLLetteringContext::AlignmentType	alignType ;
	switch ( i )
	{
	case	paramText:
		m_flagTextModified |= (m_strText != pwszCmd) ;
		m_strText = pwszCmd ;
		return ;

	case	paramTextUsage:
		textUsage = (TextUsageType)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiUsageTypes, pwszCmd, m_usageText ) ;
		m_flagTextModified |= (m_usageText != textUsage) ;
		m_usageText = textUsage ;
		return ;

	case	paramFontFace:
		m_flagTextModified |= (m_strFont != pwszCmd) ;
		m_strFont = pwszCmd ;
		return ;

	case	paramAlign:
		alignType = (SGLLetteringContext::AlignmentType)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiAlignTypes, pwszCmd, m_lettering.typeAlignment ) ;
		m_flagTextModified |= (m_lettering.typeAlignment != alignType) ;
		m_lettering.typeAlignment = alignType ;
		return ;
	}
	S3DCanvasBasicController::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasTextController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramTextUsage:
		for ( j = 0; m_aiUsageTypes[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiUsageTypes[j].pszSymbol) ) ;
		}
		return	true ;

	case	paramFontFace:
		aStrSet.Add( new SString(SGLFontStyle::StandardFont) ) ;
		aStrSet.Add( new SString(SGLFontStyle::FixedPitchFont) ) ;
		SGLFontObject::EnumerateFonts( aStrSet ) ;
		return	true ;

	case	paramAlign:
		for ( j = 0; m_aiAlignTypes[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiAlignTypes[j].pszSymbol) ) ;
		}
		return	true ;
	}
	return	S3DCanvasBasicController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
////////////////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCanvasTextController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"文字設定" ;
	case	2:
		return	L"文字装飾設定" ;
	case	3:
		return	L"文字配置設定" ;
	}
	return	nullptr ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasTextController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	if ( m_flagTextModified )
	{
		UpdateTextImage() ;
	}
	if ( m_pTextImage != nullptr )
	{
		SGLImageInfo	imginf ;
		m_pTextImage->GetImageInfo( imginf ) ;
		//
		SGLPaintParam	param ;
		param.nFlags = paintSmoothStretch | paintDelayable ;
		param.ptPaint.x = - imginf.ptOrigin.x ;
		param.ptPaint.y = - imginf.ptOrigin.y ;
		dipl.AddDrawParam( param, m_pTextImage ) ;
	}
}



////////////////////////////////////////////////////////////////////////////////////////
// ビルボード（疑似3D）パーティクル・コントローラー
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasBillboardController::Particle, ESLObject )
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasBillboardController, S3DCanvasBasicController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCanvasBillboardController, canvas_billboard )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBillboardController::S3DCanvasBillboardController( void )
	: S3DCanvasBasicController( m_ItemClassDescriptor.pwszClassID ),
		m_vImageScale( 1, 1 )
{
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBillboardController::~S3DCanvasBillboardController( void )
{
}

// 画像登録
////////////////////////////////////////////////////////////////////////////////////////
size_t S3DCanvasBillboardController::AddImageEntry
	( SGLImageObject * pImage,
		const SGLImageRect * pSrcRect, const S2DVector * pvScale )
{
	ImageEntry	ie ;
	ie.pImage = pImage ;
	if ( pSrcRect != nullptr )
	{
		ie.rectRef = *pSrcRect ;
	}
	else if ( pImage != nullptr )
	{
		ie.rectRef.x = 0 ;
		ie.rectRef.y = 0 ;
		ie.rectRef.SetSize( pImage->GetImageSize() ) ;
	}
	else
	{
		ie.rectRef.x = 0 ;
		ie.rectRef.y = 0 ;
		ie.rectRef.w = 0 ;
		ie.rectRef.h = 0 ;
	}
	if ( pvScale != nullptr )
	{
		ie.vScale = *pvScale ;
	}
	else
	{
		ie.vScale.x = 1.0f ;
		ie.vScale.y = 1.0f ;
	}
	return	m_aImages.Add( ie ) ;
}

size_t S3DCanvasBillboardController::AddImageEntries
	( const ImageEntry * pEntries, size_t nCount )
{
	return	m_aImages.AddArray( pEntries, nCount ) ;
}

// 画像登録解除
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardController::ClearAllImageEntries( void )
{
	m_aImages.RemoveAll() ;
}

// 画像表示スケール（距離/ピクセル）
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardController::SetImageScale( const S2DVector& vScale )
{
	m_vImageScale = vScale ;
}

const S2DVector& S3DCanvasBillboardController::GetImageScale( void ) const
{
	return	m_vImageScale ;
}

// 粒子追加
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardController::AddParticle( Particle * pParticle )
{
	m_csParticle.Lock() ;
	m_aParticles.Add( pParticle ) ;
	m_csParticle.Unlock() ;
}

// タイマー処理
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	m_csParticle.Lock() ;
	SSmartObjectArray<Particle>::Iterator	iter( m_aParticles, 0 ) ;
	while ( iter.HasNext() )
	{
		Particle *	pParticle = iter.Next() ;
		if ( pParticle == nullptr )
		{
			continue ;
		}
		if ( pParticle->OnTimer( *this, msecPast ) == stateDestroyed )
		{
			m_aParticles.SetAt( iter.Index(), nullptr ) ;
		}
	}
	m_aParticles.TrimEmpty() ;
	m_csParticle.Unlock() ;
}

// 画像描画
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardController::OnLocalDrawImage( SGLDrawImageParamList& dipl )
{
	S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
	if ( pItem == nullptr )
	{
		return ;
	}
	S3DScene *	pScene = pItem->GetScene() ;
	if ( pScene == nullptr )
	{
		return ;
	}
	double	zNear = esl_fclamp( pScene->GetVisibleNearDistance(), 0.0001, 0.1 ) ;
	//
	m_csParticle.Lock() ;
	SSmartObjectArray<Particle>::Iterator	iter( m_aParticles, 0 ) ;
	while ( iter.HasNext() )
	{
		Particle *	pParticle = iter.Next() ;
		if ( pParticle == nullptr )
		{
			continue ;
		}
		//
		// 表示情報取得
		//
		ParticleDesc	pdesc ;
		pdesc.nCount = 0 ;
		pParticle->GetParticle( *this, pdesc ) ;
		if ( pdesc.nCount == 0 )
		{
			continue ;
		}
		//
		// カメラ変換
		//
		S3DDVector	vPos = pdesc.vPosition ;
		pScene->TransformByCurrentCamera( vPos ) ;
		if ( vPos.z <= zNear )
		{
			continue ;
		}
		//
		// 透視変換
		//
		S2DDVector	vViewPos, vViewScale ;
		pScene->ViewProjectionAndScaleOf( vViewPos, vViewScale, vPos ) ;
		//
		S2DDVector	vImageScale = vViewScale ;
		vImageScale.x *= m_vImageScale.x ;
		vImageScale.y *= m_vImageScale.y ;
		//
		// 描画情報追加
		//
		SGLPaintParam	pp ;
		SGLAffine		affine( 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f ) ;
		pp.pAffine = &affine ;
		//
		for ( size_t i = 0; i < pdesc.nCount; i ++ )
		{
			const Description &	desc = pdesc.pDesc[i] ;
			const ImageEntry *	pie = m_aImages.GetAt( desc.iImage ) ;
			if ( (pie == nullptr) || (pie->pImage == nullptr) )
			{
				continue ;
			}
			pp.nTransparency = desc.nTransparency ;
			//
			affine.a11 = (float32_t) (desc.vScale.x * vImageScale.x) ;
			affine.a22 = (float32_t) (desc.vScale.y * vImageScale.y) ;
			affine.a13 = (float32_t) (vViewPos.x + desc.vOffset.x * affine.a11) ;
			affine.a23 = (float32_t) (vViewPos.y + desc.vOffset.y * affine.a22) ;
			affine.a13 -= (float32_t) pie->rectRef.w * (affine.a11 * 0.5f) ;
			affine.a23 -= (float32_t) pie->rectRef.h * (affine.a22 * 0.5f) ;
			//
			dipl.AddDrawParam( pp, pie->pImage, &(pie->rectRef) ) ;
		}
	}
	m_csParticle.Unlock() ;
}



////////////////////////////////////////////////////////////////////////////////////////
// ビルボード文字画像割り当て
////////////////////////////////////////////////////////////////////////////////////////

// クリア
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::FontMap::RemoveAll( void )
{
	SSortArray< SSortElement<wchar_t,size_t> >::RemoveAll() ;
	m_aImageEntries.RemoveAll() ;
}

// 文字画像追加
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::FontMap::AddFont
	( wchar_t wchCode, SGLImageObject * pImage, const SGLImageRect * pRect )
{
	S3DCanvasBillboardController::ImageEntry	ie ;
	ie.pImage = pImage ;
	ie.rectRef.x = 0 ;
	ie.rectRef.y = 0 ;
	ie.rectRef.w = 0 ;
	ie.rectRef.h = 0 ;
	ie.vScale.x = 1.0f ;
	ie.vScale.y = 1.0f ;
	//
	if ( pRect != nullptr )
	{
		ie.rectRef = *pRect ;
	}
	else if ( pImage != nullptr )
	{
		SGLSize	sizeImage = pImage->GetImageSize() ;
		ie.rectRef.w = sizeImage.w ;
		ie.rectRef.h = sizeImage.h ;
	}
	size_t	iImage = m_aImageEntries.Add( ie ) ;
	SetAs( wchCode, iImage ) ;
}

// 画像登録
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::FontMap::AddImageEntriesTo
	( S3DCanvasBillboardController& billboard )
{
	billboard.AddImageEntries
		( m_aImageEntries.GetConstArray(), m_aImageEntries.GetLength() ) ;
}



////////////////////////////////////////////////////////////////////////////////////////
// ビルボード（疑似3D）パーティクル基底
////////////////////////////////////////////////////////////////////////////////////////

// クラス情報
////////////////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCanvasBillboardParticle, Particle )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBillboardParticle::S3DCanvasBillboardParticle( void )
{
	m_param.vPos = S3DDVector( 0, 0, 0 ) ;
	m_param.vSpeed = S3DDVector( 0, 0, 0 ) ;
	m_param.fpAttenuation = 0.9 ;
	m_param.msecLife = 1000 ;
	m_param.msecFadeout = 500 ;
	m_msecPast = 0 ;
}

S3DCanvasBillboardParticle::S3DCanvasBillboardParticle( const ParticleParam& param, size_t iImage )
	: m_param( param ), m_msecPast( 0 )
{
	SetImageIndex( iImage ) ;
}

// パラメータ
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::SetParticleParam( const ParticleParam& param )
{
	m_param = param ;
}

const S3DCanvasBillboardParticle::ParticleParam& S3DCanvasBillboardParticle::GetParticleParam( void ) const
{
	return	m_param ;
}

// 画像
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::SetImageIndex( size_t iImage )
{
	S3DCanvasBillboardController::Description	desc ;
	desc.iImage = iImage ;
	desc.vOffset.x = 0 ;
	desc.vOffset.y = 0 ;
	desc.vScale.x = 1 ;
	desc.vScale.y = 1 ;
	desc.nTransparency = 0 ;
	//
	SetImageDescription( &desc, 1 ) ;
}

size_t S3DCanvasBillboardParticle::GetImageIndex( void ) const
{
	S3DCanvasBillboardController::Description *	pDesc = m_aImageDesc.GetAt(0) ;
	if ( pDesc != nullptr )
	{
		return	pDesc->iImage ;
	}
	return	0 ;
}

void S3DCanvasBillboardParticle::SetImageDescription
	( const S3DCanvasBillboardController::Description * pDesc, size_t nCount )
{
	m_aImageDesc.RemoveAll() ;
	m_aImageDesc.AddArray( pDesc, nCount ) ;
}

// 文字列画像
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::SetTextFont
	( const S3DCanvasBillboardParticle::FontMap& fontMap,
			const wchar_t * pwszText, size_t pitchChar, float32_t fpScale )
{
	m_aImageDesc.RemoveAll() ;

	size_t	nTextLen = 0 ;
	if ( pwszText != nullptr )
	{
		while ( pwszText[nTextLen] != 0 )
		{
			nTextLen ++ ;
		}
	}
	float32_t	xOffset = (float32_t) ((nTextLen - 1) * pitchChar) * -0.5f ;
	for ( size_t i = 0; i < nTextLen; i ++ )
	{
		size_t *	pIndex = fontMap.GetAs( pwszText[i] ) ;
		if ( pIndex == nullptr )
		{
			continue ;
		}
		S3DCanvasBillboardController::Description	desc ;
		desc.iImage = *pIndex ;
		desc.vOffset.x = xOffset + (float32_t) (i * pitchChar) ;
		desc.vOffset.y = 0.0f ;
		desc.vScale.x = fpScale ;
		desc.vScale.y = fpScale ;
		desc.nTransparency = 0 ;
		//
		m_aImageDesc.Add( desc ) ;
	}
}

// 時間変化
////////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBillboardController::ParticleState
	S3DCanvasBillboardParticle::OnTimer
		( S3DCanvasBillboardController& billboard, uint32_t msecPast )
{
	m_msecPast += msecPast ;
	if ( m_msecPast >= m_param.msecLife )
	{
		return	S3DCanvasBillboardController::stateDestroyed ;
	}
	double	secPast = (double) msecPast / 1000.0 ;
	m_param.vPos += m_param.vSpeed * secPast ;
	m_param.vSpeed *= pow( 1.0 - m_param.fpAttenuation, secPast ) ;
	return	S3DCanvasBillboardController::stateContinue ;
}

// 描画情報取得
////////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBillboardParticle::GetParticle
	( S3DCanvasBillboardController& billboard,
		S3DCanvasBillboardController::ParticleDesc& desc )
{
	if ( m_msecPast >= m_param.msecLife )
	{
		desc.nCount = 0 ;
		return ;
	}
	const size_t	nCount = m_aImageDesc.GetLength() ;
	const S3DCanvasBillboardController::Description *
					pDescSrc = m_aImageDesc.GetConstArray() ;
	S3DCanvasBillboardController::Description *
					pDescDst = m_aImageDescTemp.GetArray( nCount ) ;
	//
	uint32_t	nTransparency = 0 ;
	if ( m_param.msecLife - m_msecPast < m_param.msecFadeout )
	{
		nTransparency = 0x100 - (m_param.msecLife - m_msecPast)
									* 0x100 / m_param.msecFadeout ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pDescDst[i] = pDescSrc[i] ;
		pDescDst[i].nTransparency = nTransparency ;
	}
	//
	desc.vPosition = m_param.vPos ;
	desc.nCount = nCount ;
	desc.pDesc = pDescDst ;
}



////////////////////////////////////////////////////////////////////////////////////
// ブラー効果コントローラー
////////////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DCanvasBlurEffectController::m_aiBlurTypes
		[S3DCanvasBlurEffectController::blurCount + 1] =
{
	{ L"no", blurNo },
	{ L"biaxial", blurBiAxial },
	{ L"vector", blurVector },
	{ L"radial", blurRadial },
	{ nullptr, 0 }
} ;

// クラス情報
////////////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DCanvasBlurEffectController, Controller, ImageEffector )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DCanvasBlurEffectController, canvas_blur_effector )

// 構築関数
////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBlurEffectController::S3DCanvasBlurEffectController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_typeBlur( blurBiAxial ),
		m_vCenter( 0, 0, 0 ),
		m_vDirection( 1, 0 ),
		m_vOffset( 0, 0 ),
		m_fpGauss( 4.0 ),
		m_fpScaleUnit( 1.0 ),
		m_fpScalePower( 0.25 ),
		m_fpBrightness( 1.0 ),
		m_rgbMul( 0xFFFFFF ),
		m_rgbAdd( 0 )
{
	PrepareParameterEntryCount( paramBlurCount ) ;
	ESLVerify( AddParameterEntry
		( L"blur_type",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"ブラータイプ", nullptr ) == paramBlurType ) ;
	ESLVerify( AddParameterEntry
		( L"center_pos",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrGlobalTransform,
			L"放射中心座標", nullptr ) == paramBlurCenter ) ;
	ESLVerify( AddParameterEntry
		( L"blur_direction",
			S3DSceneComposer::typeVector2, 0,
			L"ぼかしベクトル", nullptr ) == paramBlurDirection ) ;
	ESLVerify( AddParameterEntry
		( L"blur_offset",
			S3DSceneComposer::typeVector2, 0,
			L"オフセット座標", nullptr ) == paramBlurOffset ) ;
	ESLVerify( AddParameterEntry
		( L"blur_gauss",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"ガウス値", nullptr, 0.0, 20.0 ) == paramBlurGauss ) ;
	ESLVerify( AddParameterEntry
		( L"blur_unit",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"距離単位", L"サンプリングの距離単位", 1.0, 4.0 ) == paramBlurScaleUnit ) ;
	ESLVerify( AddParameterEntry
		( L"blur_power",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"距離乗数", nullptr, 0.0, 1.0 ) == paramBlurScalePower ) ;
	ESLVerify( AddParameterEntry
		( L"brightness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"輝度", nullptr, 0.0, 2.0 ) == paramBlurBrightness ) ;
	ESLVerify( AddParameterEntry
		( L"color_mul",
			S3DSceneComposer::typeColor, 0,
			L"色乗算", nullptr ) == paramColorMul ) ;
	ESLVerify( AddParameterEntry
		( L"color_add",
			S3DSceneComposer::typeColor, 0,
			L"色加算", nullptr ) == paramColorAdd ) ;
}

// 消滅関数
////////////////////////////////////////////////////////////////////////////////////
S3DCanvasBlurEffectController::~S3DCanvasBlurEffectController( void )
{
}

// パラメータ値取得
////////////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCanvasBlurEffectController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBlurCenter:
		return	m_vCenter ;

	case	paramColorMul:
		return	VectorFromColor( m_rgbMul ) ;

	case	paramColorAdd:
		return	VectorFromColor( m_rgbAdd ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DCanvasBlurEffectController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBlurGauss:
		return	m_fpGauss ;

	case	paramBlurScaleUnit:
		return	m_fpScaleUnit ;

	case	paramBlurScalePower:
		return	m_fpScalePower ;

	case	paramBlurBrightness:
		return	m_fpBrightness ;
	}
	return	0.0 ;
}

const wchar_t * S3DCanvasBlurEffectController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBlurType:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiBlurTypes, m_typeBlur ) ;
	}
	return	nullptr ;
}

size_t S3DCanvasBlurEffectController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramBlurDirection:
		return	GetBinaryTypeParameter<S2DDVector>( pDst, nBufBytes, m_vDirection ) ;

	case	paramBlurOffset:
		return	GetBinaryTypeParameter<S2DDVector>( pDst, nBufBytes, m_vOffset ) ;
	}
	return	0 ;
}

// パラメータ値設定
////////////////////////////////////////////////////////////////////////////////////
void S3DCanvasBlurEffectController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBlurCenter:
		m_vCenter = vec ;
		return ;

	case	paramColorMul:
		m_rgbMul = ColorFromVector( vec ) ;
		return ;

	case	paramColorAdd:
		m_rgbAdd = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DCanvasBlurEffectController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBlurGauss:
		m_fpGauss = s ;
		return ;

	case	paramBlurScaleUnit:
		m_fpScaleUnit = s ;
		return ;

	case	paramBlurScalePower:
		m_fpScalePower = s ;
		return ;

	case	paramBlurBrightness:
		m_fpBrightness = s ;
		return ;
	}
}

void S3DCanvasBlurEffectController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramBlurType:
		m_typeBlur = (BlurType) SXMLDocument::GetIntegerAsSymbolOf
									( m_aiBlurTypes, pwszCmd, m_typeBlur ) ;
		return ;
	}
}

size_t S3DCanvasBlurEffectController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramBlurDirection:
		return	SetBinaryTypeParameter<S2DDVector>( m_vDirection, pSrc, nBufBytes ) ;

	case	paramBlurOffset:
		return	SetBinaryTypeParameter<S2DDVector>( m_vOffset, pSrc, nBufBytes ) ;
	}
	return	0 ;
}

// パラメータ値域列挙
////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBlurEffectController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramBlurType:
		{
			for ( int j = 0; m_aiBlurTypes[j].pszSymbol != nullptr; j ++ )
			{
				aStrSet.Add( new SString(m_aiBlurTypes[j].pszSymbol) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBlurEffectController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramBlurCenter:
	case	paramBlurScalePower:
		return	(m_typeBlur == blurRadial) ;

	case	paramBlurDirection:
		return	(m_typeBlur == blurVector) ;

	case	paramBlurScaleUnit:
	case	paramBlurGauss:
	case	paramBlurBrightness:
		return	(m_typeBlur != blurNo) ;
	}
	return	true ;
}

// フィルタ有効か？
////////////////////////////////////////////////////////////////////////////////////
bool S3DCanvasBlurEffectController::IsEnabledFilter( void ) const
{
	if ( ((m_rgbMul.ui32 & 0xFFFFFF) != 0xFFFFFF)
		|| ((m_rgbAdd.ui32 & 0xFFFFFF) != 0) )
	{
		return	true ;
	}
	if ( (m_typeBlur != blurNo) && (m_fpGauss != 0.0) )
	{
		return	true ;
	}
	if ( m_vOffset.Absolute() > 0.01 )
	{
		return	true ;
	}
	return	false ;
}

// フィルタ処理
////////////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DCanvasBlurEffectController::OnImageFilter
	( S3DRenderContextInterface& render,
		SGLImageObject * pDstImage, SGLImageObject * pSrcImage )
{
	SGLPaintParam	pp ;
		pp.nFlags = paintSmoothStretch ;
	//
	if ( m_vOffset.Absolute() > 0.01 )
	{
		pp.nFlags = paintFixedPosition | paintSmoothStretch | paintDelayable ;
		pp.ptPaint.x = (int32_t) esl_lroundfi( m_vOffset.x * 65536.0 ) ;
		pp.ptPaint.y = (int32_t) esl_lroundfi( m_vOffset.y * 65536.0 ) ;
		//
		SGLImageObject *	pNextDst = NextFilterBuffer( pDstImage, pSrcImage ) ;
		render.AttachTargetImage( pNextDst, nullptr, nullptr ) ;
		render.FillClearTarget( 0 ) ;
		render.DrawImage( pp, pSrcImage, nullptr ) ;
		render.DetachTargetImage() ;
		//
		pp.nFlags = paintSmoothStretch ;
		pp.ptPaint.x = 0 ;
		pp.ptPaint.y = 0 ;
		pSrcImage = pNextDst ;
	}
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( (pDevice != nullptr) && (m_typeBlur != blurNo) )
	do
	{
		//
		// ブラー処理
		//
		S3DCustomShader *	pBlurShader = nullptr ;
		if ( m_typeBlur == blurRadial )
		{
			pBlurShader = pDevice->GetDefaultShaderProgramAs
					( S3DRenderDevice::DefaultShaderId::RadialGaussianBlur ) ;
			S3DGaussianRadialBlurShaderInterface *
				pgrbiBlur = ESLTypeCast<S3DGaussianRadialBlurShaderInterface>( pBlurShader ) ;
			if ( pgrbiBlur == nullptr )
			{
				break ;
			}
			pgrbiBlur->SetGauss( m_fpGauss ) ;
			pgrbiBlur->SetCenter( m_vCenter.x, m_vCenter.y ) ;
			pgrbiBlur->SetBrightness( m_fpBrightness ) ;
			pgrbiBlur->SetSamplingUnit( 1.0 ) ;
			pgrbiBlur->SetBlueScale( m_fpScaleUnit, m_fpScalePower ) ;
		}
		else
		{
			pBlurShader = pDevice->GetDefaultShaderProgramAs
					( S3DRenderDevice::DefaultShaderId::GaussianBlur ) ;
			S3DGaussianBlurShaderInterface *
				pgbiBlur = ESLTypeCast<S3DGaussianBlurShaderInterface>( pBlurShader ) ;
			if ( pgbiBlur == nullptr )
			{
				break ;
			}
			S2DDVector	vDir = m_vDirection.Normalized() * m_fpScaleUnit ;
			pgbiBlur->SetGauss( m_fpGauss ) ;
			pgbiBlur->SetDirection( vDir.x, vDir.y ) ;
			pgbiBlur->SetBrightness( m_fpBrightness ) ;
			//
			if ( m_typeBlur == blurBiAxial )
			{
				SGLImageObject *	pNextDst = NextFilterBuffer( pDstImage, pSrcImage ) ;
				render.AttachTargetImage( pNextDst, nullptr, nullptr ) ;
				render.FillClearTarget( 0 ) ;
				pgbiBlur->SetDirection( 0.0, m_fpScaleUnit ) ;
				render.AttachCustomShader( pBlurShader ) ;
				pp.nFlags = paintSmoothStretch ;
				render.DrawImage( pp, pSrcImage, nullptr ) ;
				render.DetachTargetImage() ;
				//
				pgbiBlur->SetDirection( m_fpScaleUnit, 0.0 ) ;
				pSrcImage = pNextDst ;
			}
		}
		SGLImageObject *	pNextDst = NextFilterBuffer( pDstImage, pSrcImage ) ;
		render.AttachTargetImage( pNextDst, nullptr, nullptr ) ;
		render.FillClearTarget( 0 ) ;
		render.AttachCustomShader( pBlurShader ) ;
		pp.nFlags = paintSmoothStretch ;
		render.DrawImage( pp, pSrcImage, nullptr ) ;
		render.DetachTargetImage() ;
		//
		render.AttachCustomShader( nullptr ) ;
		pSrcImage = pNextDst ;
	}
	while ( false ) ;
	//
	// 色効果
	//
	if ( ((m_rgbMul.ui32 & 0xFFFFFF) != 0xFFFFFF)
		|| ((m_rgbAdd.ui32 & 0xFFFFFF) != 0) )
	{
		SGLImageObject *	pNextDst = pSrcImage ;
		if ( (m_rgbMul.ui32 & 0xFFFFFF) != 0xFFFFFF )
		{
			pNextDst = NextFilterBuffer( pDstImage, pSrcImage ) ;
			render.AttachTargetImage( pNextDst, nullptr, nullptr ) ;
			render.FillClearTarget( 0 ) ;
			//
			pp.nFlags = paintSmoothStretch | paintApplyColorMul | paintDelayable ;
			pp.rgbColorParam = m_rgbMul.ui32 | 0xFF000000 ;
			render.DrawImage( pp, pSrcImage, nullptr ) ;
		}
		else
		{
			render.AttachTargetImage( pNextDst, nullptr, nullptr ) ;
		}
		if ( (m_rgbAdd.ui32 & 0xFFFFFF) != 0 )
		{
			SGLSize	sizeImage = pNextDst->GetImageSize() ;
			render.FillRectangle
				( 0, 0, sizeImage.w, sizeImage.h,
					(m_rgbAdd.ui32 & 0xFFFFFF), 0.0, paintDelayable ) ;
		}
		render.DetachTargetImage() ;
		pSrcImage = pNextDst ;
	}
	return	pSrcImage ;
}

SGLImageObject * S3DCanvasBlurEffectController::NextFilterBuffer
	( SGLImageObject * pDstImage, SGLImageObject * pSrcImage )
{
	if ( pDstImage == pSrcImage )
	{
		SGLSize	sizeImage = pDstImage->GetImageSize() ;
		if ( m_imgWorkBuf.GetImageSize() != sizeImage )
		{
			m_imgWorkBuf.ReleaseBuffer() ;
			m_imgWorkBuf.CreateImage
				( sizeImage.w, sizeImage.h,
					formatImageDefaultRGBA, 32,
					SGLImageObject::bufferOnDeviceOnly ) ;
		}
		return	&m_imgWorkBuf ;
	}
	else
	{
		return	pDstImage ;
	}
}

