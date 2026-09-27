
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_sprite.h>
#include <sakuragl/sgl_vr_view_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////
// 3D シーンアイテム・スプライト
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DSceneItemSprite, SGLSpriteFormed, Item )

// 構築関数
//////////////////////////////////////////////////////////////////////////
S3DSceneItemSprite::S3DSceneItemSprite( void )
	: m_modeCoords( coordinatesFrame3D ),
		m_tanViewHalfAngle( 0.5 ), m_pixelsViewWidth( 1280 ),
		m_depthMaskOp( S3DRenderContextInterface::depthMaskNoWrite )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_paramView.nFlags |= paintWithZOrderNoWrite ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
S3DSceneItemSprite::~S3DSceneItemSprite( void )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent != nullptr )
	{
		if ( LockTrace( __FILE__, __LINE__, 100 ) == errSuccess )
		{
			pParent->DetachChild( this ) ;
			Unlock() ;
		}
	}
	S3DScene::Space *	pSpace = GetParentSpace() ;
	if ( pSpace != nullptr )
	{
		S3DSceneSprite *
			pScene = ESLTypeCast<S3DSceneSprite>( pSpace->GetScene() ) ;
		if ( pScene != nullptr )
		{
			pScene->OnDeleteSceneItemSprite( this ) ;
		}
		pSpace->RemoveItem( this ) ;
	}
}

// 座標モード
//////////////////////////////////////////////////////////////////////////
S3DSceneItemSprite::CoordinatesMode S3DSceneItemSprite::GetCoordinatesMode( void ) const
{
	return	m_modeCoords ;
}

void S3DSceneItemSprite::SetCoordinatesMode( S3DSceneItemSprite::CoordinatesMode mode )
{
	m_modeCoords = mode ;
}

// 2D 表示時の水平視野角
//////////////////////////////////////////////////////////////////////////
double S3DSceneItemSprite::Get2DViewHalfAngle( void ) const
{
	return	m_tanViewHalfAngle ;
}

void S3DSceneItemSprite::Set2DViewHalfAngle( double tanHalfAngle )
{
	m_tanViewHalfAngle = tanHalfAngle ;
}

// 2D 表示時の水平視野角に対応するピクセル数
//////////////////////////////////////////////////////////////////////////
int S3DSceneItemSprite::Get2DViewWidthPixels( void ) const
{
	return	m_pixelsViewWidth ;
}

void S3DSceneItemSprite::Set2DViewWidthPixels( int nPixels )
{
	m_pixelsViewWidth = nPixels ;
}

// 2D 表示座標変換取得
//////////////////////////////////////////////////////////////////////////
SGLAffine S3DSceneItemSprite::Get2DViewAffine( const S3DScene& scene ) const
{
	S3DScene::ProjectionParam	projParam ;
	scene.GetCurrentProjection( projParam ) ;
	//
	double	tanWidth = projParam.vScreen.x / projParam.vScreen.z ;
	double	fpAngleScale = m_tanViewHalfAngle / tanWidth ;
	double	fpCoordScale = projParam.vScreen.x * 2.0 * projParam.fpZoom
								* fpAngleScale / (double) m_pixelsViewWidth ;
	//
	return	SGLAffine( (float32_t) fpCoordScale, 0.0f, projParam.vScreen.x,
						0.0f, (float32_t) fpCoordScale, projParam.vScreen.y ) ;
}

// 3D直接描画時のｚバッファ操作
//////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface::DepthMaskOperation
	S3DSceneItemSprite::GetDepthMaskOperation( void ) const
{
	return	m_depthMaskOp ;
}

void S3DSceneItemSprite::SetDepthMaskOperation
	( S3DRenderContextInterface::DepthMaskOperation depthOp )
{
	m_depthMaskOp = depthOp ;
}

// 更新領域通知
//////////////////////////////////////////////////////////////////////////
void S3DSceneItemSprite::PostUpdate( SGLRect* pUpdate )
{
	SGLSprite *	pSprite = GetParent() ;
	if ( pSprite != nullptr )
	{
		pSprite->PostUpdate( nullptr ) ;
	}
	SGLSpriteFormed::PostUpdate( pUpdate ) ;
}

// コマンド通知
//////////////////////////////////////////////////////////////////////////
bool S3DSceneItemSprite::NotifyCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	if ( SGLSpriteFormed::NotifyCommand
			( pszCmd, nParam, nCode, nPriority, fOverwritable ) )
	{
		return	true ;
	}
	SGLSprite *	pSprite = GetParent() ;
	if ( pSprite != nullptr )
	{
		return	pSprite->OnCommand
				( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
	}
	return	false ;
}

// ローカル座標からグローバル座標へ変換
//////////////////////////////////////////////////////////////////////////
bool S3DSceneItemSprite::LocalToGlobal( S2DDVector& vPos ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene == nullptr )
	{
		return	SGLSpriteFormed::LocalToGlobal( vPos ) ;
	}
	if ( m_modeCoords == coordinatesFrame3D )
	{
		//
		// 3次元フレーム
		//
		S3DScene::Camera *	pCamera = pScene->GetCurrentCamera() ;
		if ( pCamera == nullptr )
		{
			pCamera = pScene->GetMainCamera() ;
			if ( pCamera == nullptr )
			{
				return	SGLSpriteFormed::LocalToGlobal( vPos ) ;
			}
		}
		//
		// 平面変換
		//
		if ( !SGLSpriteFormed::LocalToGlobal( vPos ) )
		{
			return	false ;
		}
		//
		// ３次元変換
		//
		S3DDMatrix	matItem ;
		S3DDVector	posItem ;
		CalcGlobalTransformation( matItem, posItem ) ;
		//
		S3DDVector	v3dPos( vPos.x, vPos.y, 0.0 ) ;
		v3dPos = matItem * v3dPos + posItem ;
		//
		// カメラ変換
		//
		S3DDMatrix	matCamera ;
		S3DDVector	posCamera ;
		S3DScene::CalcCameraTransformation( matCamera, posCamera, pCamera ) ;
		//
		v3dPos = matCamera * v3dPos - posCamera ;
		if ( v3dPos.z <= 0.0 )
		{
			return	false ;
		}
		//
		// 透視変換
		//
		pScene->ViewProjectionOf( vPos, v3dPos ) ;
	}
	else if ( m_modeCoords == coordinatesDirect2D )
	{
		//
		// 2次元直接描画（フレームバッファ不要）
		//
		if ( !SGLSpriteFormed::LocalToGlobal( vPos ) )
		{
			return	false ;
		}
		SGLAffine	affine = Get2DViewAffine( *pScene ) ;
		vPos = affine * vPos ;
	}
	else
	{
		return	false ;
	}
	return	true ;
}

// グローバル座標からローカル座標へ変換
//////////////////////////////////////////////////////////////////////////
bool S3DSceneItemSprite::GlobalToLocal( S2DDVector& vPos ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene == nullptr )
	{
		return	false ;
	}
	if ( m_modeCoords == coordinatesFrame3D )
	{
		//
		// 3次元フレーム
		//
		S3DScene::Camera *	pCamera = pScene->GetCurrentCamera() ;
		if ( pCamera == nullptr )
		{
			pCamera = pScene->GetMainCamera() ;
			if ( pCamera == nullptr )
			{
				return	false ;
			}
		}
		S3DDVector	vRay, vRayOrigin ;
		pScene->RayProjectionFor( vRay, vRayOrigin, vPos, pCamera ) ;
		//
		S3DDMatrix	matItem ;
		S3DDVector	posItem ;
		CalcGlobalTransformation( matItem, posItem ) ;
		//
		// 基底ベクトル
		//
		S3DDVector	vX( 1, 0, 0 ) ;
		S3DDVector	vY( 0, 1, 0 ) ;
		S3DDVector	vO = posItem - vRayOrigin ;
		matItem.RevolveVector( vX ) ;
		matItem.RevolveVector( vY ) ;
		//
		// 平面との交点
		//
		S3DDVector	vXY = vX * vY ;
		vRay.Normalize() ;
		vXY.Normalize() ;
		//
		double	r = (vRay | vXY) ;
		if ( fabs( r ) < 1.0e-8 )
		{
			return	false ;
		}
		r = (vO | vXY) / r ;
		//
		S3DDVector	vLocal = vRay * r - vO ;
		vPos.x = vXY.InnerProduct( vLocal * vY ) ;
		vPos.y = vXY.InnerProduct( vX * vLocal ) ;
	}
	else if ( m_modeCoords == coordinatesDirect2D )
	{
		//
		// 2次元直接描画（フレームバッファ不要）
		//
		SGLAffine	affine = Get2DViewAffine( *pScene ) ;
		vPos = affine.Inverse() * vPos ;
	}
	else
	{
		return	false ;
	}
	return	SGLSpriteFormed::GlobalToLocal( vPos ) ;
}

// 親スプライト取得
//////////////////////////////////////////////////////////////////////////
SGLSprite* S3DSceneItemSprite::GetParent( void ) const
{
	S3DScene::Space *	pSpace = GetParentSpace() ;
	if ( pSpace != nullptr )
	{
		return	ESLTypeCast<SGLSprite>( pSpace->GetScene() ) ;
	}
	return	nullptr ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneItemSprite::OnUpdateBehavior( S3DScene& scene )
{
	SGLSpriteFormed::PrepareDrawFrame() ;

	const Stereo3DView	s3dView = FromStereoViewIndex
									( scene.GetCurrentStereoViewTarget() ) ;
	if ( !m_dipList.IsEmptyList() )
	{
		PostUpdate( nullptr ) ;
	}
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( (s3dView == s3dMonoview)
		&& (pBuffer != nullptr) && pBuffer->IsStereo3D() )
	{
		BeforeDraw( s3dRightView ) ;
		AfterDraw( s3dRightView ) ;
		BeforeDraw( s3dLeftView ) ;
		AfterDraw( s3dLeftView ) ;
	}
	else
	{
		BeforeDraw( s3dView ) ;
		AfterDraw( s3dView ) ;
	}

	S3DSceneSprite *	pScene = ESLTypeCast<S3DSceneSprite>( &scene ) ;
	if ( pScene != nullptr )
	{
		pScene->AddSceneItemSprite( this ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////
void S3DSceneItemSprite::RenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	Item::RenderModel( scene, render, flagsExclusion ) ;
	//
	if ( m_visible && (m_flagsBehavior & S3DScene::itemVisible) )
	{
		render.PushTransformation() ;
		render.SetOptionalFeature
			( RenderContext::featureOrderPriority,
							m_nRenderPriority, nullptr, 0 ) ;
		if ( m_pListener != nullptr )
		{
			m_pListener->BeforeItemRenderModel
						( scene, render, flagsExclusion ) ;
		}
		if ( m_modeCoords == coordinatesFrame3D )
		{
			const Stereo3DView	s3dView = FromStereoViewIndex
											( scene.GetCurrentStereoViewTarget() ) ;
			SGLImageObject *	pImage = (s3dView == s3dLeftView)
											? GetAttachedLeftImage() : GetAttachedImage() ;
			if ( (pImage == nullptr) && (s3dView == s3dLeftView) )
			{
				pImage = GetAttachedImage() ;
			}
			if ( pImage != nullptr )
			{
				RenderSpriteFrameBuffer( scene, render, flagsExclusion, pImage ) ;
			}
			else
			{
				SGLDrawImageParamList	diplTemp ;
				DrawImageList( diplTemp, nullptr, s3dView ) ;
				RenderDrawImageList( scene, render, flagsExclusion, diplTemp ) ;
			}
		}
		else if ( m_modeCoords == coordinatesDirect2D )
		{
			SGLAffine	affine = Get2DViewAffine( scene ) ;
			render.PushTransformation() ;
			render.AppendTransformation( affine, m_space.m_nTransparency ) ;
			//
			Draw( render, nullptr, FromStereoViewIndex( scene.GetCurrentStereoViewTarget() ) ) ;
			//
			render.PopTransformation() ;
		}
		if ( m_pListener != nullptr )
		{
			m_pListener->AfterItemRenderModel
						( scene, render, flagsExclusion ) ;
		}
		render.PopTransformation() ;
	}
}

// レンダリングスレッド排他処理用
//////////////////////////////////////////////////////////////////////////
SSystem::SError S3DSceneItemSprite::Lock( int64_t msecTimeout ) const
{
	return	SGLSpriteFormed::Lock( msecTimeout ) ;
}

SSystem::SError S3DSceneItemSprite::Unlock( void ) const
{
	return	SGLSpriteFormed::Unlock() ;
}

atomic_int_t S3DSceneItemSprite::TestLocked( void ) const
{
	return	SGLSpriteFormed::TestLocked() ;
}

// フレームバッファ描画
//////////////////////////////////////////////////////////////////////////
void S3DSceneItemSprite::RenderSpriteFrameBuffer
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion, SGLImageObject * pImage )
{
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	if ( !GetPaintParam( pp, affine, nullptr, s3dMonoview ) )
	{
		return ;
	}
	S3DMaterial *	pMaterial = nullptr ;
	SGLImageRect	rectRef ;
	if ( pp.nFlags & paintWithZOrderNoWrite )
	{
		pMaterial = SGLImageNoShadeMaterialInterface::
							GetMaterialNoWriteZOf( pImage, &rectRef ) ;
	}
	else if ( pp.nFlags & paintWithZOrder )
	{
		pMaterial = SGLImageNoShadeMaterialInterface::
									GetMaterialOf( pImage, &rectRef ) ;
	}
	else
	{
		pMaterial = SGLImageNoShadeMaterialInterface::
									GetMaterialNoZOf( pImage, &rectRef ) ;
	}
	ESLAssert( pMaterial != nullptr ) ;
	//
	render.PushTransformation() ;
	if ( m_flagsBehavior & S3DScene::itemGlobalSpace )
	{
		render.SetMatrixTransformation
			( m_space.m_matTransformation, m_space.m_vCenter,
				&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
	}
	else
	{
		render.AppendMatrixTransformation
			( m_space.m_matTransformation, m_space.m_vCenter,
				&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
	}
	S3DDMatrix	mat
		( affine.a11, affine.a12, 0.0,
			affine.a21, affine.a22, 0.0,
			0.0, 0.0, 1.0 ) ;
	S3DDVector	pos
		( affine.a13 + pp.ptPaint.x,
			affine.a23 + pp.ptPaint.y, 0.0 ) ;
	render.AppendMatrixTransformation
		( mat, pos, nullptr, pp.nTransparency ) ;
	//
	if ( (pp.pVertices != nullptr) && (pp.countVertex >= 3) )
	{
		SArray<S3DVector4>	aVertex ;
		S3DVector4 *	pvVertex = aVertex.GetArray( pp.countVertex ) ;
		for ( size_t i = 0; i < pp.countVertex; i ++ )
		{
			pvVertex[i].x = pp.pVertices[i].x ;
			pvVertex[i].y = pp.pVertices[i].y ;
			pvVertex[i].z = 0.0 ;
		}
		SArray<uint32_t>	aIndex ;
		uint32_t *	pIndex = aIndex.GetArray( pp.countVertex * 3 ) ;
		for ( size_t i = 2, j = 0; i < pp.countVertex; i ++, j += 3 )
		{
			pIndex[j] = 0 ;
			pIndex[j + 1] = (uint32_t) i - 1 ;
			pIndex[j + 2] = (uint32_t) i ;
		}
		render.AddIndexedTriangleList
			( pMaterial, 0,
				pp.countVertex - 2, pp.countVertex,
				pvVertex, nullptr, pp.pVertices, nullptr, pIndex ) ;
		aVertex.FinishArray() ;
		aIndex.FinishArray() ;
	}
	else
	{
		S3DVector4	vVertex[4] ;
		S3DVector4	vNormal[4] ;
		S2DVector	vUVMap[4] ;
		S3DColor	aColor[4] ;
		//
		vUVMap[0].x = (float32_t) rectRef.x ;
		vUVMap[0].y = (float32_t) rectRef.y ;
		vUVMap[1].x = (float32_t) (rectRef.x + rectRef.w - 1) ;
		vUVMap[1].y = vUVMap[0].y ;
		vUVMap[2].x = vUVMap[0].x ;
		vUVMap[2].y = (float32_t) (rectRef.y + rectRef.h - 1) ;
		vUVMap[3].x = vUVMap[1].x ;
		vUVMap[3].y = vUVMap[2].y ;
		//
		for ( size_t i = 0; i < 4; i ++ )
		{
			vVertex[i].x = vUVMap[i].x ;
			vVertex[i].y = vUVMap[i].y ;
			vVertex[i].z = 0.0f ;
			vVertex[i].d = 0.0f ;
			//
			vNormal[i].x = 0.0f ;
			vNormal[i].y = 0.0f ;
			vNormal[i].z = -1.0f ;
			vNormal[i].d = 0.0f ;
			//
			aColor[i].rgbAdd = 0 ;
			aColor[i].rgbMul = 0x00FFFFFF ;
		}
		render.AddTriangleStrip
			( pMaterial, RenderContext::renderUncombinable,
				2, &vVertex[0], &vNormal[0], &vUVMap[0], &aColor[0] ) ;
	}
	render.PopTransformation() ;
}

// SGLDrawImageParamList を直接３次元空間へ描画
//////////////////////////////////////////////////////////////////////////////
void S3DSceneItemSprite::RenderDrawImageList
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion, SGLDrawImageParamList& dipList )
{
	render.PushTransformation() ;
	if ( m_flagsBehavior & S3DScene::itemGlobalSpace )
	{
		render.SetMatrixTransformation
			( m_space.m_matTransformation, m_space.m_vCenter,
				&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
	}
	else
	{
		render.AppendMatrixTransformation
			( m_space.m_matTransformation, m_space.m_vCenter,
				&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
	}
	if ( m_depthMaskOp != S3DRenderContextInterface::depthMaskDefault )
	{
		render.SetOptionalFeature
			( S3DRenderContextInterface::featureDepthMask, m_depthMaskOp, nullptr, 0 ) ;
	}
	//
	dipList.RenderAs3D
		( render, flagsExclusion,
			(SGLDrawImageParamList::StereoViewIndex) scene.GetCurrentStereoViewTarget() ) ;
	//
	render.PopTransformation() ;
}



//////////////////////////////////////////////////////////////////////////////
// スプライトｚバッファ付き描画インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteDepthDrawer, SGLSpriteDrawer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteDepthDrawer::SGLSpriteDepthDrawer( void )
	: m_pSprite( nullptr ), m_pDevice( nullptr ),
		m_pShader( nullptr ), m_pDrawWithDepth( nullptr )
{
}

// スプライトにアタッチされた
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteDepthDrawer::OnAttachedSprite( SGLSprite * pSprite )
{
	m_pSprite = pSprite ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteDepthDrawer::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( image != nullptr )
	{
		//
		// シェーダー取得
		//
		S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
		if ( m_pDevice != pDevice )
		{
			if ( pDevice != nullptr )
			{
				m_pShader =
					pDevice->GetDefaultShaderProgramAs
						( S3DRenderDevice::DefaultShaderId::DrawWithDepth ) ;
				m_pDrawWithDepth =
					ESLTypeCast<S3DDrawWithDepthShaderInterface>( m_pShader ) ;
			}
			else
			{
				m_pShader = nullptr ;
				m_pDrawWithDepth = nullptr ;
			}
			m_pDevice = pDevice ;
		}
		//
		// ソースｚバッファ取得
		//
		SGLImageObject *	pDepth = nullptr ;
		S4DMatrix			matPers( 1, 1, 1, 1 ) ;
		if ( m_pSprite != nullptr )
		{
			SGLSprite::Buffer *	pBuffer = m_pSprite->GetFrameBuffer() ;
			if ( (pBuffer != nullptr) && pBuffer->HasZBuffer() )
			{
				if ( pBuffer->Renderer().GetPerspectiveMatrix( matPers ) )
				{
					pDepth = pBuffer->GetZBuffer() ;
				}
			}
		}
		//
		// 描画
		//
		render.PushTransformation() ;
		if ( (pDepth != nullptr)
			&& (m_pShader != nullptr) && (m_pDrawWithDepth != nullptr) )
		{
			render.AttachCustomShader( m_pShader ) ;
			m_pDrawWithDepth->LockParameter() ;
			m_pDrawWithDepth->SetDepthBuffer( pDepth ) ;
			m_pDrawWithDepth->SetPerspective( matPers ) ;
			m_pDrawWithDepth->SetShaderUniformsTo( render ) ;
			m_pDrawWithDepth->UnlockParameter() ;
		}
		render.DrawImage( pp, image ) ;
		render.PopTransformation() ;
	}
}

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteDepthDrawer::DuplicateObject( void )
{
	return	new SGLSpriteDepthDrawer ;
}



//////////////////////////////////////////////////////////////////////////
// 3D シーン表示スプライト
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneSprite, SGLSpriteMultiView, S3DScene )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneSprite::InternalBuffer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////
S3DSceneSprite::S3DSceneSprite( void )
{
	m_pSelectViewEntry = nullptr ;
	m_flagsInternalBuffer = bufferAllTarget ;
	m_iViewRenderTarget = renderTargetComposed ;
	m_flagSceneUpdate = false ;
	//
	S3DScene::SetBackColor( SGLPalette( 0 ), true ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
S3DSceneSprite::~S3DSceneSprite( void )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent != nullptr )
	{
		if ( LockTrace( __FILE__, __LINE__, 100 ) == errSuccess )
		{
			pParent->DetachChild( this ) ;
			Unlock() ;
		}
	}
}

// 中間バッファフラグ
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetInternalBufferFlags( uint32_t nFlags )
{
	m_flagsInternalBuffer = nFlags ;
}

// 表示する描画ターゲットバッファ
// （enum ViewRenderTargetIndex or enum RenderTargetIndex）
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetViewRenderTargetBuffer( int iRenderTarget )
{
	m_iViewRenderTarget = iRenderTarget ;
}

// 描画先（親スプライト）へデプスを反映する描画を行うよう設定する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetSpriteDepthDrawer( void )
{
	SetSpriteDrawer( new SGLSpriteDepthDrawer ) ;
}

// ビュー毎に異なるカメラと透視変換を設定する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetViewSettingAs
	( SGLSecondaryViewProducer * psvp,
		const S3DSceneSprite::ViewEntry& viewEntry )
{
	Lock() ;
	m_psaViewEntries.SetAs( psvp, viewEntry ) ;
	Unlock() ;
}

// ビュー毎の描画設定を取得する
//////////////////////////////////////////////////////////////////////////
S3DSceneSprite::ViewEntry *
	S3DSceneSprite::GetViewSettingAs
		( SGLSecondaryViewProducer * psvp ) const
{
	S3DSceneSprite::ViewEntry *	pView ;
	Lock() ;
	pView = m_psaViewEntries.GetAs( psvp ) ;
	Unlock() ;
	return	pView ;
}

// ビュー毎の描画設定を削除する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::RemoveViewSettingAs( SGLSecondaryViewProducer * psvp )
{
	Lock() ;
	if ( m_pSelectView == psvp )
	{
		SGLImageObject *	pMultiTargets[1] = { nullptr } ;
		AttachLayeredRenderTarget( nullptr, nullptr ) ;
		AttachMultiRenderTarget( &pMultiTargets[0], 0 ) ;
		m_pSelectViewEntry = nullptr ;
	}
	m_psaViewEntries.RemoveAs( psvp ) ;
	Unlock() ;
}

void S3DSceneSprite::RemoveAllViewSettings( void )
{
	Lock() ;
	SGLImageObject *	pMultiTargets[1] = { nullptr } ;
	AttachLayeredRenderTarget( nullptr, nullptr ) ;
	AttachMultiRenderTarget( &pMultiTargets[0], 0 ) ;
	m_pSelectViewEntry = nullptr ;
	//
	m_psaViewEntries.RemoveAll() ;
	Unlock() ;
}

// 現在の設定を ViewEntry に取得する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::GetCurrentViewSettings( S3DSceneSprite::ViewEntry& viewEntry )
{
	viewEntry.paramProj = m_projParam ;
	viewEntry.pCamera = GetMainCamera() ;
	if ( m_fParallaxParam )
	{
		viewEntry.paramParallax[0] = m_parallaxParam[0] ;
		viewEntry.paramParallax[1] = m_parallaxParam[1] ;
	}
	else
	{
		viewEntry.paramParallax[0].SetParallax
			( m_xParallax,
				m_projParam.vScreen.z * m_zParallaxFocus,
				m_xParallaxScreenDelta ) ;
		viewEntry.paramParallax[1].SetParallax
			( - m_xParallax,
				m_projParam.vScreen.z * m_zParallaxFocus,
				- m_xParallaxScreenDelta ) ;
	}
	viewEntry.pTargetLayered = m_pDrawTargetLayered ;
	viewEntry.pTargetLayeredSampler = m_pTargetLayered ;
	viewEntry.pTargetLayeredDepth = m_pDrawTargetLayeredDepth ;
	viewEntry.pTargetLayeredDepthSampler = m_pTargetLayeredDepth ;
	viewEntry.aMultiTargets = m_aMultiDrawTargets ;
	viewEntry.aMultiTargetSamplers = m_aMultiTargets ;
}

// VR HMD 表示用のパラメータを取得する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::GetViewSettingForVRHMD
	( S3DSceneSprite::ViewEntry& viewEntry, SGLSize& sizeFrame,
		const SGLVRViewProducer * pVR,
		float32_t zNear, float32_t zFar,
		S3DScene::Camera * pCamera, S3DRenderDevice * pDevice )
{
	if ( GetSelectedSecondaryView() == pVR )
	{
		GetCurrentViewSettings( viewEntry ) ;
	}

	const SGLVRViewProducer::EyeFieldOfView&
		fovRight = pVR->GetEyeFieldOfView
							( SGLVRViewProducer::eyeRight ) ;
	viewEntry.nBehaviorFlags = behaviorAutoVRView ;
	viewEntry.paramProj.vScreen = fovRight.vScreenPos ;
	viewEntry.paramProj.fpZoom = 1.0f ;
	viewEntry.paramProj.fpPixelAspect = fovRight.fpPixelAspect ;
	viewEntry.paramProj.zNear = zNear ;
	viewEntry.paramProj.zFar = zFar ;
	viewEntry.pCamera = pCamera ;
	viewEntry.pDevice = pDevice ;
	//
	const SGLVRViewProducer::Posture&
		postureRight = pVR->GetEyePosture
							( SGLVRViewProducer::eyeRight ) ;
	S3DScene::ParallaxParam&
		parallaxRight = viewEntry.paramParallax
							[RenderContext::stereoViewRight] ;
	parallaxRight.matPosture = postureRight.matOrientation ;
	parallaxRight.vParallax = postureRight.vPosition ;
	parallaxRight.vScreenDelta = S3DVector( 0, 0, 0 ) ;
	parallaxRight.fpAspectDelta = 0.0f ;
	//
	const SGLVRViewProducer::EyeFieldOfView&
		fovLeft = pVR->GetEyeFieldOfView
							( SGLVRViewProducer::eyeLeft ) ;
	const SGLVRViewProducer::Posture&
		postureLeft = pVR->GetEyePosture
							( SGLVRViewProducer::eyeLeft ) ;
	S3DScene::ParallaxParam&
		parallaxLeft = viewEntry.paramParallax
							[RenderContext::stereoViewLeft] ;
	parallaxLeft.matPosture = postureLeft.matOrientation ;
	parallaxLeft.vParallax = postureLeft.vPosition ;
	parallaxLeft.vScreenDelta = fovLeft.vScreenPos - fovRight.vScreenPos ;
	parallaxLeft.fpAspectDelta = fovLeft.fpPixelAspect - fovRight.fpPixelAspect ;
	//
	sizeFrame.w = (int32_t) esl_max
					( fovRight.sizeOfView.w, fovLeft.sizeOfView.w ) ;
	sizeFrame.h = (int32_t) esl_max
					( fovRight.sizeOfView.h, fovLeft.sizeOfView.h ) ;
}

// 空間登録
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::AddSpaceProperty
	( const wchar_t * pwszName, S3DScene::Space * pSpace )
{
	m_ssoaSpaces.Add( pwszName, pSpace ) ;
}

// 空間取得
//////////////////////////////////////////////////////////////////////////
S3DScene::Space *
	S3DSceneSprite::GetSpaceProperty( const wchar_t * pwszName ) const
{
	return	m_ssoaSpaces.GetAs( pwszName ) ;
}

// 空間削除
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::RemoveSpacePropertyAs( const wchar_t * pwszName )
{
	ssize_t	i = m_ssoaSpaces.FindAs( pwszName ) ;
	if ( i >= 0 )
	{
		S3DScene::Space *	pSpace = m_ssoaSpaces.GetAt( i ) ;
		if ( pSpace != nullptr )
		{
			S3DScene::Space *	pParent = pSpace->GetParentSpace() ;
			if ( pParent != nullptr )
			{
				pParent->RemoveChild( pSpace ) ;
			}
		}
		m_ssoaSpaces.RemoveAt( i ) ;
	}
}

void S3DSceneSprite::RemoveSpaceProperty( S3DScene::Space * pSpace )
{
	ssize_t	i = m_ssoaSpaces.FindPtr( pSpace ) ;
	if ( i >= 0 )
	{
		S3DScene::Space *	pParent = pSpace->GetParentSpace() ;
		if ( pParent != nullptr )
		{
			pParent->RemoveChild( pSpace ) ;
		}
		m_ssoaSpaces.RemoveAt( i ) ;
	}
}

// すべての空間を削除する
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::RemoveAllSpacesAndItems( void )
{
	m_spaceRoot.RemoveAllChildren() ;
	m_spaceRoot.RemoveAllItems() ;
	m_ssoaSpaces.RemoveAll() ;
	m_ssoaItems.RemoveAll() ;
}

// アイテム登録
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::AddItemProperty
	( const wchar_t * pwszName, S3DScene::Item * pItem )
{
	m_ssoaItems.Add( pwszName, pItem ) ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////
S3DScene::Item *
	S3DSceneSprite::GetItemProperty( const wchar_t * pwszName ) const
{
	return	m_ssoaItems.GetAs( pwszName ) ;
}

// 空間削除
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::RemoveItemPropertyAs( const wchar_t * pwszName )
{
	ssize_t	i = m_ssoaItems.FindAs( pwszName ) ;
	if ( i >= 0 )
	{
		S3DScene::Item *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			S3DScene::Space *	pParent = pItem->GetParentSpace() ;
			if ( pParent != nullptr )
			{
				pParent->RemoveItem( pItem ) ;
			}
		}
		m_ssoaItems.RemoveAt( i ) ;
	}
}

void S3DSceneSprite::RemoveItemProperty( S3DScene::Item * pItem )
{
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		S3DScene::Space *	pParent = pItem->GetParentSpace() ;
		if ( pParent != nullptr )
		{
			pParent->RemoveItem( pItem ) ;
		}
		m_ssoaItems.RemoveAt( i ) ;
	}
}

// 3D アイテム・スプライト追加
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::AddSceneItemSprite( S3DSceneItemSprite * pSprite )
{
	SSmartLock<SCriticalSection>	lock( &m_csSpriteItems ) ;
#if	defined(__DEBUG__)
	size_t	nCount = m_aSpriteItems.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( pse != nullptr )
		{
			ESLAssert( ESLTypeCast<S3DSceneItemSprite>( pse->m_refItem.GetReference() ) != pSprite ) ;
		}
	}
#endif
	Sprite3DEntry *	pse = new Sprite3DEntry ;
	pse->m_refItem = (SGLSprite*) pSprite ;
	m_aSpriteItems.Add( pse ) ;
}

// 3D アイテム・スプライト削除通知
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::OnDeleteSceneItemSprite( S3DSceneItemSprite * pSprite )
{
	SSmartLock<SCriticalSection>	lock( &m_csSpriteItems ) ;
	size_t	nCount = m_aSpriteItems.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( (pse != nullptr)
			&& (ESLTypeCast<S3DSceneItemSprite>
					( pse->m_refItem.GetReference() ) != pSprite) )
		{
			pse->m_refItem.SetReference( nullptr ) ;
		}
	}
}

// ヒットアイテム検索
//////////////////////////////////////////////////////////////////////////
SGLSprite* S3DSceneSprite::GetHitRayForSprite
	( S2DDVector& vPos, double& zDistance,
		const S3DDVector& vRay,
		const S3DDVector& vRayOrigin, bool fHitLocal ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSpriteItems ) ;
	double		zHitMin = 1.0e+99 ;
	SGLSprite *	pHitSprite = nullptr ;
	for ( size_t i = 0; i < m_aSpriteItems.GetLength(); i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( pse == nullptr )
		{
			continue ;
		}
		SGLSprite *	pSprite =
			ESLTypeCast<SGLSprite>( pse->m_refItem.GetReference() ) ;
		if ( pSprite == nullptr )
		{
			continue ;
		}
		//
		S3DSceneItemSprite *	pSceneItem = ESLTypeCast<S3DSceneItemSprite>( pSprite ) ;
		if ( (pSceneItem != nullptr)
			&& (pSceneItem->GetCoordinatesMode()
					== S3DSceneItemSprite::coordinatesDirect2D) )
		{
			S2DDVector	vLocal = vPos ;
			if ( pSceneItem->GlobalToLocal( vLocal ) )
			{
				SGLSprite *	pHit = pSceneItem->GetHitSpriteAt( vLocal ) ;
				if ( pHit != nullptr )
				{
					pHitSprite = pHit ;
					zHitMin = 0.0 ;
					//
					vPos = vLocal ;
					zDistance = 0.0 ;
				}
			}
			continue ;
		}
		//
		// 基底ベクトル
		//
		S3DDVector	vX( 1, 0, 0 ) ;
		S3DDVector	vY( 0, 1, 0 ) ;
		S3DDVector	vO = pse->m_vModel - vRayOrigin ;
		pse->m_matModel.RevolveVector( vX ) ;
		pse->m_matModel.RevolveVector( vY ) ;
		//
		// 平面との交点
		//
		S3DDVector	vRayNormal = vRay ;
		S3DDVector	vXY = vX * vY ;
		double		s = vXY.Absolute() ;
		vRayNormal.Normalize() ;
		vXY.Normalize() ;
		//
		double	r = (vRayNormal | vXY) ;
		if ( fabs( r ) < 1.0e-8 )
		{
			continue ;
		}
		double	zHit = (vO | vXY) / r ;
		//
		if ( (zHit < 0.0) || ((pHitSprite != nullptr) && (zHitMin < zHit)) )
		{
			// 既にヒットしている Sprite より奥／又は基点より逆方向
			continue ;
		}
		S3DDVector	vHitPos = vRayNormal * zHit + vRayOrigin ;
		//
		// 平面座標系へ変換
		//
		S3DDVector	vLocal = vHitPos - pse->m_vModel ;
		S2DDVector	vLocal2D ;
		vLocal2D.x = vXY.InnerProduct( vLocal * vY ) / s ;
		vLocal2D.y = vXY.InnerProduct( vX * vLocal ) / s ;
		pSprite->SGLSprite::GlobalToLocal( vLocal2D ) ;
		//
		// スプライト内でのヒットテスト
		//
		S2DDVector	vHitLocal2D = vLocal2D ;
		SGLSprite *	pHit = nullptr ;
		if ( !fHitLocal && pSprite->IsHitSprite( vLocal2D.x, vLocal2D.y ) )
		{
			pHit = pSprite ;
		}
		if ( pHit == nullptr )
		{
			pHit = pSprite->GetHitSpriteAt( vHitLocal2D ) ;
		}
		if ( pHit != nullptr )
		{
			if ( fHitLocal )
			{
				pHitSprite = pHit ;
				vPos = vHitLocal2D ;
			}
			else
			{
				pHitSprite = pSprite ; ;
				vPos = vLocal2D ;
			}
			zHitMin = zHit ;
			//
			zDistance = zHit ;
		}
	}
	return	pHitSprite ;
}

// ホバー／タッチ操作実行
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::VRTouchOperation
	( S3DSceneSprite::VRTouchContext& context, const S3DDVector& vTouchPos )
{
	S2DDVector	vHitPos ;
	double		fpDistance ;
	S3DSceneItemSprite *
		pHitSprite = GetTouchForSprite
			( vHitPos, fpDistance, vTouchPos, context.m_fpHoverReach ) ;
	S3DSceneItemSprite *
		pLastHovering = context.m_refHovering.GetRef<S3DSceneItemSprite>() ;
	//
	const uint64_t	nMouseFlags = (context.m_idMouse
										& SGLSprite::MouseIDMask)
									| VirtualTouchFlag ;
	if ( pHitSprite != pLastHovering )
	{
		if ( pLastHovering != nullptr )
		{
			pLastHovering->OnMouseLeave( nMouseFlags ) ;
		}
		context.m_refHovering.SetReference( pHitSprite->GetESLPointer(pHitSprite) ) ;
		context.m_flagTouching = false ;
	}
	if ( pHitSprite != nullptr )
	{
		pHitSprite->OnMouseMove( vHitPos.x, vHitPos.y, nMouseFlags ) ;
		//
		if ( fpDistance <= context.m_fpTouchReach )
		{
			if ( !context.m_flagTouching )
			{
				pHitSprite->OnButtonDown
					( vHitPos.x, vHitPos.y,
						nMouseFlags | (LeftButtonID << ButtonIDShifter) ) ;
				context.m_flagTouching = true ;
			}
		}
		else
		{
			if ( context.m_flagTouching )
			{
				pHitSprite->OnButtonUp
					( vHitPos.x, vHitPos.y,
						nMouseFlags | (LeftButtonID << ButtonIDShifter) ) ;
				context.m_flagTouching = false ;
			}
		}
	}
}

// ホバー／タッチアイテム検索
//////////////////////////////////////////////////////////////////////////
S3DSceneItemSprite * S3DSceneSprite::GetTouchForSprite
	( S2DDVector& vLocalPos, double& fpDistance,
		const S3DDVector& vTouchPos, double fpMaxReach ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSpriteItems ) ;
	double					zHitMin = fpMaxReach ;
	S3DSceneItemSprite *	pHitSprite = nullptr ;
	for ( size_t i = 0; i < m_aSpriteItems.GetLength(); i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( pse == nullptr )
		{
			continue ;
		}
		S3DSceneItemSprite *	pSceneItem =
			ESLTypeCast<S3DSceneItemSprite>( pse->m_refItem.GetReference() ) ;
		if ( (pSceneItem == nullptr)
			|| (pSceneItem->GetCoordinatesMode()
					== S3DSceneItemSprite::coordinatesDirect2D) )
		{
			continue ;
		}
		//
		// 基底ベクトル
		//
		S3DDVector	vX( 1, 0, 0 ) ;
		S3DDVector	vY( 0, 1, 0 ) ;
		S3DDVector	vZ( 0, 0, 1 ) ;
		S3DDVector	vO = vTouchPos - pse->m_vModel ;
		pse->m_matModel.RevolveVector( vX ) ;
		pse->m_matModel.RevolveVector( vY ) ;
		pse->m_matModel.RevolveVector( vZ ) ;
		vZ.Normalize() ;
		//
		// 平面との距離
		//
		double	z = vO.InnerProduct( vZ ) ;
		if ( zHitMin < fabs(z) )
		{
			continue ;
		}
		//
		// 平面座標系へ変換
		//
		S3DDVector	vHitPos = vTouchPos - vZ * z ;
		S3DDVector	vLocal = vHitPos - pse->m_vModel ;
		S2DDVector	vLocal2D ;
		S3DDVector	vXY = vX * vY ;
		double		s = vXY.Absolute() ;
		vXY.Normalize() ;
		vLocal2D.x = vXY.InnerProduct( vLocal * vY ) / s ;
		vLocal2D.y = vXY.InnerProduct( vX * vLocal ) / s ;
		pSceneItem->SGLSpriteFormed::GlobalToLocal( vLocal2D ) ;
		//
		// スプライト内でのヒットテスト
		//
		if ( pSceneItem->SGLSpriteFormed::IsHitSprite( vLocal2D.x, vLocal2D.y ) )
		{
			zHitMin = fabs(z) ;
			fpDistance = zHitMin ;
			vLocalPos = vLocal2D ;
			pHitSprite = pSceneItem ;
		}
	}
	return	pHitSprite ;
}

// レンダリングデバイスの設定（S3DScene/SGLSprite オーバーライド）
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::SetRenderDevice( S3DRenderDevice * pDevice )
{
	SGLError	err1, err2 ;
	err1 = S3DScene::SetRenderDevice( pDevice ) ;
	err2 = SGLSpriteMultiView::SetRenderDevice( pDevice ) ;
	return	err1 ? err1 : err2 ;
}

// レンダリングスレッド排他処理用（S3DScene/SGLSprite オーバーライド）
//////////////////////////////////////////////////////////////////////////
SSystem::SError S3DSceneSprite::Lock( int64_t msecTimeout ) const
{
	return	SGLSprite::Lock( msecTimeout ) ;
}

SSystem::SError S3DSceneSprite::Unlock( void ) const
{
	return	SGLSprite::Unlock() ;
}

atomic_int_t S3DSceneSprite::TestLocked( void ) const
{
	return	SGLSprite::TestLocked() ;
}

// 透視変換（S3DScene オーバーライド）
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::GetProjectionOfView
	( S3DScene::ProjectionParam& projParam, SGLSecondaryViewProducer * psvp ) const
{
	ViewEntry *	pViewEntry = GetViewSettingAs( psvp ) ;
	if ( pViewEntry != nullptr )
	{
		projParam = pViewEntry->paramProj ;
		return	sglErrSuccess ;
	}
	return	S3DScene::GetProjectionOfView( projParam, psvp ) ;
}

// 透視変換（S3DScene オーバーライド）
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetProjection
	( const S3DScene::ProjectionParam& projParam )
{
	Lock() ;
	S3DScene::SetProjection( projParam ) ;
	SGLSpriteMultiView::SetProjectionScreen
		( projParam.vScreen,
			projParam.fpZoom, projParam.fpPixelAspect ) ;
	//
	if ( m_pSelectViewEntry != nullptr )
	{
		m_pSelectViewEntry->paramProj = m_projParam ;
	}
	Unlock() ;
}

// 透視変換（SGLSprite オーバーライド）
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	Lock() ;
	m_projParam.vScreen = vScreen ;
	m_projParam.fpZoom = (float32_t) zScale ;
	m_projParam.fpPixelAspect = (float32_t) fpPixelAspect ;
	SGLSpriteMultiView::SetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
	//
	if ( m_pSelectViewEntry != nullptr )
	{
		m_pSelectViewEntry->paramProj = m_projParam ;
	}
	Unlock() ;
}

// 視差設定（SGLSprite オーバーライド）
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	Lock() ;
	SGLSpriteMultiView::SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
	S3DScene::SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
	//
	if ( m_pSelectViewEntry != nullptr )
	{
		if ( m_fParallaxParam )
		{
			m_pSelectViewEntry->paramParallax[0] = m_parallaxParam[0] ;
			m_pSelectViewEntry->paramParallax[1] = m_parallaxParam[1] ;
		}
		else
		{
			m_pSelectViewEntry->paramParallax[0].SetParallax
				( m_xParallax,
					m_projParam.vScreen.z * m_zParallaxFocus,
					m_xParallaxScreenDelta ) ;
			m_pSelectViewEntry->paramParallax[1].SetParallax
				( - m_xParallax,
					m_projParam.vScreen.z * m_zParallaxFocus,
					- m_xParallaxScreenDelta ) ;
		}
	}
	Unlock() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::AdvanceTime( uint32_t msecPast )
{
	m_csSpriteItems.Lock() ;
	for ( size_t i = 0; i < m_aSpriteItems.GetLength(); i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( pse == nullptr )
		{
			continue ;
		}
		SGLSprite *	pSprite =
			ESLTypeCast<SGLSprite>( pse->m_refItem.GetReference() ) ;
		if ( pSprite != nullptr )
		{
			m_csSpriteItems.Unlock() ;
			pSprite->AdvanceTime( msecPast ) ;
			m_csSpriteItems.Lock() ;
		}
	}
	m_csSpriteItems.Unlock() ;
	//
	S3DScene::OnTimer( msecPast ) ;
	SGLSpriteMultiView::AdvanceTime( msecPast ) ;
	//
	if ( m_flagSceneUpdate )
	{
		SGLSpriteMultiView::PostUpdate( nullptr ) ;
		m_flagSceneUpdate = false ;
	}
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::PrepareDrawFrame( void )
{
	PrepareRenderScene( GetMainCamera() ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::BeforeDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSpriteMultiView::BeforeDraw( s3dView ) ;
	//
	if ( m_pSelectViewEntry != nullptr )
	{
		SetCurrentCamera( m_pSelectViewEntry->pCamera ) ;
	}
	else
	{
		SetCurrentCamera( GetMainCamera() ) ;
	}
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::AfterDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSpriteMultiView::AfterDraw( s3dView ) ;
	FinishRenderScene() ;
}

// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	if ( m_iViewRenderTarget != renderTargetComposed )
	{
		Buffer *	pBuffer = GetFrameBuffer() ;
		if ( m_iViewRenderTarget == renderTargetZBuffer )
		{
			if ( (pBuffer != nullptr) && pBuffer->HasZBuffer() )
			{
				image = pBuffer->GetZBuffer() ;
			}
		}
		else if ( m_flagsInternalBuffer & (1 << m_iViewRenderTarget)
				&& (m_iViewRenderTarget >= 0)
				&& (m_iViewRenderTarget < renderTargetCount) )
		{
			SecondaryBuffer *
				pSecondaryBuf = ESLTypeCast<SecondaryBuffer>( pBuffer ) ;
			if ( pSecondaryBuf != nullptr )
			{
				InternalBuffer *	pib =
					ESLTypeCast<InternalBuffer>
							( pSecondaryBuf->m_pUserLayer.Ptr() ) ;
				if ( pib != nullptr )
				{
					if ( pBuffer->IsMultisampling() )
					{
						image = &(pib->m_imgMultiRenderSampler[m_iViewRenderTarget]) ;
						//
						S3DRenderDevice *	pDevice =
							pSecondaryBuf->Renderer().GetRenderDeviceObject() ;
						if ( pib->m_renderTempDst.GetRenderDeviceObject() != pDevice )
						{
							pib->m_renderTempDst.AttachRenderContext( pDevice->NewRenderer(), true ) ;
							pib->m_renderTempSrc.AttachRenderContext( pDevice->NewRenderer(), true ) ;
						}
						pib->m_renderTempDst.AttachTargetImage( image, nullptr, nullptr ) ;
						pib->m_renderTempSrc.AttachTargetImage
							( &(pib->m_imgMultiRenderTarget[m_iViewRenderTarget]), nullptr, nullptr ) ;
						pib->m_renderTempDst.CopyBufferFrom
							( pib->m_renderTempSrc, RenderContext::copyBufferColor ) ;
						pib->m_renderTempDst.DetachTargetImage() ;
						pib->m_renderTempSrc.DetachTargetImage() ;
					}
					else
					{
						image = &(pib->m_imgMultiRenderTarget[m_iViewRenderTarget]) ;
					}
				}
			}
		}
	}
	SGLSpriteMultiView::DrawSprite( render, pp, image ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::BeforeDrawChildren( SGLSprite::Stereo3DView s3dView )
{
	SGLSpriteMultiView::BeforeDrawChildren( s3dView ) ;
}

// ヒットアイテム検索
//////////////////////////////////////////////////////////////////////////
SGLSprite* S3DSceneSprite::GetHitSpriteAt( S2DDVector& vPos ) const
{
	SGLSprite *	pHit = SGLSpriteMultiView::GetHitSpriteAt( vPos ) ;
	if ( pHit == nullptr )
	{
		S3DScene::Camera *	pCamera = GetCurrentCamera() ;
		if ( pCamera == nullptr )
		{
			pCamera = GetMainCamera() ;
		}
		S3DDVector	vRay, vRayOrigin ;
		RayProjectionFor( vRay, vRayOrigin, vPos, pCamera ) ;
		//
		double	zDistance ;
		pHit = GetHitRayForSprite( vPos, zDistance, vRay, vRayOrigin, true ) ;
	}
	return	pHit ;
}

// 背景色取得
//////////////////////////////////////////////////////////////////////////
bool S3DSceneSprite::GetFillBackColor( uint32_t& argbFill ) const
{
	SGLPalette	rgbaBack ;
	bool		flagBack = S3DScene::GetBackColor( rgbaBack ) ;
	argbFill = rgbaBack ;
	return	flagBack ;
}

// 背景色設定
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::SetFillBackColor
				( uint32_t argbFill, bool flagFillBack )
{
	SGLPalette	rgbaBack = argbFill ;
	S3DScene::SetBackColor( rgbaBack, flagFillBack ) ;
	return	sglErrSuccess ;
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::DrawChildren
	( S3DRenderContextInterface& render,
			SGLSprite::Stereo3DView s3dView ) const
{
	RenderContext::StereoViewIndex
				sviView = RenderContext::stereoViewAuto ;
	if ( s3dView == s3dRightView )
	{
		sviView = RenderContext::stereoViewRight ;
	}
	else if ( s3dView == s3dLeftView )
	{
		sviView = RenderContext::stereoViewLeft ;
	}
	//
	SuitableRenderParam	srp ;
	srp.pScene = (S3DSceneSprite*) this ;
	srp.pRender = &render ;
	srp.sviView = sviView ;
	render.SuitableProcedure( &S3DSceneSprite::SuitableRenderProc, &srp ) ;

//	render.PushTransformation() ;
//	((S3DScene*)this)->DoRenderScene( &render, sviView ) ;
//	render.PopTransformation() ;
	//
	SGLSpriteMultiView::DrawChildren( render, s3dView ) ;
}

void S3DSceneSprite::SuitableRenderProc( void * pInstance )
{
	SuitableRenderParam *	psrp = (SuitableRenderParam*) pInstance ;
	SSharableMutex *	pMutex = psrp->pScene->GetUIThreadMutex() ;
	pMutex->SharedLock() ;
	psrp->pRender->PushTransformation() ;
	psrp->pScene->DoRenderScene( psrp->pRender, psrp->sviView ) ;
	psrp->pRender->PopTransformation() ;
	pMutex->SharedUnlock() ;
}

// 出力先バッファ選択
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::SelectSecondaryView( SGLSecondaryViewProducer * psvp )
{
	SGLError	err ;
	Lock() ;
	err = SGLSpriteMultiView::SelectSecondaryView( psvp ) ;
	//
	m_pSelectViewEntry = GetViewSettingAs( psvp ) ;
	if ( m_pSelectViewEntry != nullptr )
	{
		SGLVRViewProducer *	pVR = ESLTypeCast<SGLVRViewProducer>( psvp ) ;
		if ( (pVR != nullptr)
			&& (m_pSelectViewEntry->nBehaviorFlags & behaviorAutoVRView) )
		{
			SGLSize		sizeFrame ;
			ViewEntry	viewEntry = *m_pSelectViewEntry ;
			GetViewSettingForVRHMD
				( viewEntry, sizeFrame, pVR,
					viewEntry.paramProj.zNear,
					viewEntry.paramProj.zFar,
					viewEntry.pCamera, viewEntry.pDevice ) ;
			//
			if ( m_pSelectViewEntry->nBehaviorFlags & behaviorAutoVRViewParams )
			{
				m_pSelectViewEntry->paramParallax[0] = viewEntry.paramParallax[0] ;
				m_pSelectViewEntry->paramParallax[1] = viewEntry.paramParallax[1] ;
			}
			if ( m_pSelectViewEntry->nBehaviorFlags & behaviorAutoVRViewSize )
			{
				m_pSelectViewEntry->paramProj = viewEntry.paramProj ;
				//
				const SGLVRViewProducer::EyeFieldOfView&
					fovRight = pVR->GetEyeFieldOfView
										( SGLVRViewProducer::eyeRight ) ;
				const SGLVRViewProducer::EyeFieldOfView&
					fovLeft = pVR->GetEyeFieldOfView
										( SGLVRViewProducer::eyeLeft ) ;
				//
				uint64_t	nBufFlags = SGLImageObject::bufferOnDeviceOnly
										| SGLImageObject::bufferForRenderTarget ;
				if ( (m_pBuffer != nullptr)
					&& m_pBuffer->IsMultisampling() )
				{
					nBufFlags |= m_pBuffer->GetBufFlags()
									& SGLImageObject::bufferRenderNonTextureFlags ;
				}
				//
				Buffer *	pBuffer = GetFrameBuffer() ;
				if ( pVR->IsStereoDisplayMode() )
				{
					if ( (pBuffer == nullptr)
						|| (pBuffer->GetImage()->GetImageSize() != fovRight.sizeOfView)
						|| (pBuffer->GetLeftImage()->GetImageSize() != fovLeft.sizeOfView) )
					{
						ESLTrace( "create stereo view for VR HMD. (%dx%d) (%dx%d)\n",
									fovLeft.sizeOfView.w, fovLeft.sizeOfView.h,
									fovRight.sizeOfView.w, fovRight.sizeOfView.h ) ;
						CreateStereoBuffer
							( fovRight.sizeOfView, fovLeft.sizeOfView,
								formatImageDefaultRGBA, 32, nBufFlags, true ) ;
						//
						if ( m_pSelectViewEntry->pDevice != nullptr )
						{
							SetBufferRenderer
								( m_pSelectViewEntry->pDevice->NewRenderer(), true ) ;
						}
					}
				}
				else if ( (pBuffer == nullptr)
					|| (pBuffer->GetImage()->GetImageSize() != fovRight.sizeOfView) )
				{
					ESLTrace( "create mono view for VR. (%dx%d)\n",
								fovRight.sizeOfView.w, fovRight.sizeOfView.h ) ;
					CreateBuffer
						( fovRight.sizeOfView.w, fovRight.sizeOfView.h,
							formatImageDefaultRGBA, 32, nBufFlags, true ) ;
					//
					if ( m_pSelectViewEntry->pDevice != nullptr )
					{
						SetBufferRenderer
							( m_pSelectViewEntry->pDevice->NewRenderer(), true ) ;
					}
				}
			}
		}
		const size_t	nMRTCount = m_pSelectViewEntry->aMultiTargets.GetLength() ;
		double			xParallax, zFocusRate, xScreenDelta ;
		S3DScene::GetParallax( xParallax, zFocusRate, xScreenDelta ) ;
		//
		if ( (m_pSelectBuffer != nullptr)
			&& m_pSelectBuffer->IsMultisampling() )
		{
			ESLAssert( m_pSelectViewEntry->aMultiTargetSamplers.GetLength()
							>= m_pSelectViewEntry->aMultiTargets.GetLength() ) ;
			S3DScene::AttachRenderTarget
				( m_pSelectBuffer->GetTempImage(),
					m_pSelectBuffer->GetTempZBuffer(), nullptr,
					m_pSelectBuffer->GetImage(),
					m_pSelectBuffer->GetZBuffer() ) ;
			if ( m_pSelectBuffer->IsStereo3D() )
			{
				S3DScene::AttachStereoTargetLeft
					( m_pSelectBuffer->GetTempLeftImage(),
						xParallax, zFocusRate, m_pSelectBuffer->GetLeftImage() ) ;
			}
			else
			{
				S3DScene::AttachStereoTargetLeft( nullptr, xParallax, zFocusRate, nullptr ) ;
			}
			S3DScene::AttachLayeredRenderTarget
				( m_pSelectViewEntry->pTargetLayered,
					m_pSelectViewEntry->pTargetLayeredDepth,
					m_pSelectViewEntry->pTargetLayeredSampler,
					m_pSelectViewEntry->pTargetLayeredDepthSampler ) ;
			S3DScene::AttachMultiRenderTarget
				( m_pSelectViewEntry->aMultiTargets.GetConstArray(),
						m_pSelectViewEntry->aMultiTargets.GetLength(),
						m_pSelectViewEntry->aMultiTargetSamplers.GetConstArray() ) ;
			if ( nMRTCount > renderTargetTemporary0 )
			{
				S3DScene::AttachTemporaryRenderBuffers
					( m_pSelectViewEntry->aMultiTargets.GetConstArray() + renderTargetTemporary0,
								nMRTCount - renderTargetTemporary0 ) ;
			}
		}
		else
		{
			S3DScene::AttachRenderTarget
				( m_pSelectBuffer->GetImage(), m_pSelectBuffer->GetZBuffer() ) ;
			if ( m_pSelectBuffer->IsStereo3D() )
			{
				S3DScene::AttachStereoTargetLeft
					( m_pSelectBuffer->GetLeftImage(), xParallax, zFocusRate, nullptr ) ;
			}
			else
			{
				S3DScene::AttachStereoTargetLeft( nullptr, xParallax, zFocusRate, nullptr ) ;
			}
			S3DScene::AttachLayeredRenderTarget
				( m_pSelectViewEntry->pTargetLayered,
					m_pSelectViewEntry->pTargetLayeredDepth ) ;
			S3DScene::AttachMultiRenderTarget
				( m_pSelectViewEntry->aMultiTargets.GetConstArray(),
						m_pSelectViewEntry->aMultiTargets.GetLength() ) ;
			if ( nMRTCount > renderTargetTemporary0 )
			{
				S3DScene::AttachTemporaryRenderBuffers
					( m_pSelectViewEntry->aMultiTargets.GetConstArray() + renderTargetTemporary0,
								nMRTCount - renderTargetTemporary0 ) ;
			}
		}
		if ( nMRTCount <= renderTargetTemporary0 )
		{
			S3DScene::AttachTemporaryRenderBuffers( nullptr, 0 ) ;
		}
		//
		S3DScene::SetProjection( m_pSelectViewEntry->paramProj ) ;
		S3DScene::SetParallaxParam
			( m_pSelectViewEntry->paramParallax[RenderContext::stereoViewRight],
				m_pSelectViewEntry->paramParallax[RenderContext::stereoViewLeft] ) ;
		S3DScene::SetCurrentCamera( m_pSelectViewEntry->pCamera ) ;
	}
	Unlock() ;
	return	err;
}

// ユーザー定義バッファ生成
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::OnCreateUserLayerBuffer
	( SGLSpriteMultiView::SecondaryBuffer * pBuffer,
		const SGLSize& sizeRight, const SGLSize& sizeLeft,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D )
{
	uint64_t	nRenderBufFlags = (nBufFlags & SGLImageObject::bufferRenderNonTextureFlags)
								| SGLImageObject::bufferNonPowerOf2
								| SGLImageObject::bufferOnDeviceOnly ;
	uint64_t	nSamplerBufFlags = nRenderBufFlags
								& ~uint64_t(SGLImageObject::bufferRenderNonTextureFlags) ;
	//
	InternalBuffer *	pib =
			ESLTypeCast<InternalBuffer>( pBuffer->m_pUserLayer.Ptr() ) ;
	if ( pib == nullptr )
	{
		pib = new InternalBuffer ;
		pBuffer->m_pUserLayer = pib ;
	}
	SGLImageObject *	pMultiTargets[renderTargetAllCount] ;
	SGLImageObject *	pMultiTargetSamplers[renderTargetAllCount] ;
	uint32_t	width = (uint32_t) esl_max( sizeRight.w, sizeLeft.w ) ;
	uint32_t	height = (uint32_t) esl_max( sizeRight.h, sizeLeft.h ) ;
	size_t		nMRTCount = 0 ;
	for( int i = 0; i < renderTargetAllCount; i ++ )
	{
		if ( m_flagsInternalBuffer & (1 << i) )
		{
			pib->m_imgMultiRenderTarget[i].CreateImage
				( width, height, format, depth, nRenderBufFlags ) ;
			pMultiTargets[i] = &(pib->m_imgMultiRenderTarget[i]) ;
			//
			if ( pBuffer->IsMultisampling() )
			{
				pib->m_imgMultiRenderSampler[i].CreateImage
					( width, height, format, depth, nSamplerBufFlags ) ;
				pMultiTargetSamplers[i] = &(pib->m_imgMultiRenderSampler[i]) ;
			}
			nMRTCount = i + 1 ;
		}
		else
		{
			pMultiTargets[i] = nullptr ;
			pMultiTargetSamplers[i] = nullptr ;
		}
	}
	if ( flagZBuffer && (m_flagsInternalBuffer & bufferEffectTarget) )
	{
		pib->m_imgLayeredDepth.CreateImage
			( width, height, formatImageDepth, 32, nRenderBufFlags ) ;
		//
		if ( pBuffer->IsMultisampling() )
		{
			pib->m_imgLayeredDepthSampler.CreateImage
				( width, height, formatImageDepth, 32, nSamplerBufFlags ) ;
		}
	}
	SGLSecondaryViewProducer *	psvp = GetSelectedSecondaryView() ;
	ViewEntry *	pViewEntry = GetViewSettingAs( psvp ) ;
	if ( pViewEntry != nullptr )
	{
		if ( m_flagsInternalBuffer & bufferEffectTarget )
		{
			pViewEntry->pTargetLayered =
						&(pib->m_imgMultiRenderTarget[0]) ;
			pViewEntry->pTargetLayeredSampler = nullptr ;
			pViewEntry->pTargetLayeredDepth = nullptr ;
			pViewEntry->pTargetLayeredDepthSampler = nullptr ;
			//
			if ( flagZBuffer )
			{
				pViewEntry->pTargetLayeredDepth = &(pib->m_imgLayeredDepth) ;
			}
			if ( pBuffer->IsMultisampling() )
			{
				pViewEntry->pTargetLayeredSampler =
						&(pib->m_imgMultiRenderSampler[0]) ;
				//
				if ( flagZBuffer )
				{
					pViewEntry->pTargetLayeredDepthSampler =
									&(pib->m_imgLayeredDepthSampler) ;
				}
			}
		}
		if ( nMRTCount > 1 )
		{
			pViewEntry->aMultiTargets.SetLength( 0 ) ;
			pViewEntry->aMultiTargetSamplers.SetLength( 0 ) ;
			pViewEntry->aMultiTargets.AddArray( &pMultiTargets[1], nMRTCount - 1 ) ;
			//
			if ( pBuffer->IsMultisampling() )
			{
				pViewEntry->aMultiTargetSamplers.AddArray
						( &pMultiTargetSamplers[1], nMRTCount - 1 ) ;
			}
		}
	}
	if ( m_flagsInternalBuffer & bufferEffectTarget )
	{
		if ( pBuffer->IsMultisampling() )
		{
			S3DScene::AttachLayeredRenderTarget
				( &(pib->m_imgMultiRenderTarget[0]),
					(flagZBuffer ? &(pib->m_imgLayeredDepth) : nullptr),
					&(pib->m_imgMultiRenderSampler[0]),
					(flagZBuffer ? &(pib->m_imgLayeredDepthSampler) : nullptr) ) ;
		}
		else
		{
			S3DScene::AttachLayeredRenderTarget
				( &(pib->m_imgMultiRenderTarget[0]),
					(flagZBuffer ? &(pib->m_imgLayeredDepth) : nullptr) ) ;
		}
	}
	double	xParallax, zFocusRate, xScreenDelta ;
	S3DScene::GetParallax( xParallax, zFocusRate, xScreenDelta ) ;
	if ( pBuffer->IsMultisampling() )
	{
		S3DScene::AttachRenderTarget
			( pBuffer->GetTempImage(), pBuffer->GetTempZBuffer(),
				nullptr, pBuffer->GetImage(), pBuffer->GetZBuffer() ) ;
		if ( flagStereo3D )
		{
			S3DScene::AttachStereoTargetLeft
				( pBuffer->GetTempLeftImage(),
					xParallax, zFocusRate, pBuffer->GetLeftImage() ) ;
		}
		else
		{
			S3DScene::AttachStereoTargetLeft( nullptr, xParallax, zFocusRate, nullptr ) ;
		}
	}
	else
	{
		S3DScene::AttachRenderTarget
			( pBuffer->GetImage(), pBuffer->GetZBuffer() ) ;
		if ( flagStereo3D )
		{
			S3DScene::AttachStereoTargetLeft
				( pBuffer->GetLeftImage(), xParallax, zFocusRate, nullptr ) ;
		}
		else
		{
			S3DScene::AttachStereoTargetLeft( nullptr, xParallax, zFocusRate, nullptr ) ;
		}
	}
	if ( nMRTCount > 1 )
	{
		S3DScene::AttachMultiRenderTarget
			( &pMultiTargets[1],
				esl_min( renderTargetCount, (int) nMRTCount ) - 1,
				(pBuffer->IsMultisampling() ? &pMultiTargetSamplers[1] : nullptr) ) ;
		//
		if ( nMRTCount > renderTargetTemporary0 )
		{
			if ( pBuffer->IsMultisampling() )
			{
				S3DScene::AttachTemporaryRenderBuffers
					( &pMultiTargetSamplers[renderTargetTemporary0],
								nMRTCount - renderTargetTemporary0 ) ;
			}
			else
			{
				S3DScene::AttachTemporaryRenderBuffers
					( &pMultiTargets[renderTargetTemporary0],
								nMRTCount - renderTargetTemporary0 ) ;
			}
		}
		else
		{
			S3DScene::AttachTemporaryRenderBuffers( nullptr, 0 ) ;
		}
	}
	else
	{
		S3DScene::AttachMultiRenderTarget( nullptr, 0 ) ;
		S3DScene::AttachTemporaryRenderBuffers( nullptr, 0 ) ;
	}
	pBuffer->DisableFillBack() ;
}

// ユーザー定義バッファ解放時処理
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::OnReleaseUserLayerBuffer( ESLObject * pUserLayer )
{
	InternalBuffer *	pib = ESLTypeCast<InternalBuffer>( pUserLayer ) ;
	if ( pib != nullptr )
	{
		double	xParallax, zFocusRate, xScreenDelta ;
		S3DScene::GetParallax( xParallax, zFocusRate, xScreenDelta ) ;
		//
		S3DScene::AttachRenderTarget( nullptr, nullptr ) ;
		S3DScene::AttachStereoTargetLeft( nullptr, xParallax, zFocusRate, nullptr ) ;
		S3DScene::AttachMultiRenderTarget( nullptr, 0 ) ;
		S3DScene::AttachLayeredRenderTarget( nullptr, nullptr ) ;
		S3DScene::AttachTemporaryRenderBuffers( nullptr, 0 ) ;
	}
}

// レンダリング前処理
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::PrepareRenderScene
	( S3DScene::Camera * pCamera, uint64_t nOptFlags )
{
	SGLError	err ;
	m_csSpriteItems.Lock() ;
	m_aSpriteItems.RemoveAll() ;
	m_csSpriteItems.Unlock() ;

	SuitablePrepareRenderParam	sprp ;
	sprp.pScene = this ;
	sprp.pCamera = pCamera ;
	sprp.nOptFlags = nOptFlags ;
	m_render.SuitableProcedure( &S3DSceneSprite::SuitablePrepareRenderProc, &sprp ) ;
	err = sprp.err ;

//	err = S3DScene::PrepareRenderScene( pCamera, nOptFlags ) ;
	//
	m_csSpriteItems.Lock() ;
	size_t	nCount = m_aSpriteItems.GetLength() ;
	bool	fTrimEntry = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sprite3DEntry *	pse = m_aSpriteItems.GetAt( i ) ;
		if ( pse == nullptr )
		{
			continue ;
		}
		S3DSceneItemSprite *	pItem =
				ESLTypeCast<S3DSceneItemSprite>
						( pse->m_refItem.GetReference() ) ;
		if ( pItem == nullptr )
		{
			fTrimEntry = true ;
			m_aSpriteItems.SetAt( i, nullptr ) ;
			continue ;
		}
		pItem->CalcGlobalTransformation
			( pse->m_matModel, pse->m_vModel ) ;
	}
	if ( fTrimEntry )
	{
		m_aSpriteItems.TrimEmpty() ;
	}
	m_csSpriteItems.Unlock() ;
	return	err ;
}

void S3DSceneSprite::SuitablePrepareRenderProc( void * pInstance )
{
	SuitablePrepareRenderParam *
			psprp = (SuitablePrepareRenderParam*) pInstance ;
	SSharableMutex *	pMutex = psprp->pScene->GetUIThreadMutex() ;
	pMutex->SharedLock() ;
	psprp->err = psprp->pScene->S3DScene::PrepareRenderScene
								( psprp->pCamera, psprp->nOptFlags ) ;
	pMutex->SharedUnlock() ;
}

// レンダリング後処理
//////////////////////////////////////////////////////////////////////////
SGLError S3DSceneSprite::FinishRenderScene( void )
{
	return	S3DScene::FinishRenderScene() ;
}

// 画面更新通知
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::PostSceneUpdate( void )
{
	m_flagSceneUpdate = true ;
}

// レンダリングスレッド排他処理用
//////////////////////////////////////////////////////////////////////////
SSystem::SSharableMutex * S3DSceneSprite::GetUIThreadMutex( void ) const
{
	return	m_pMutexUI ;
}

// カメラオブジェクト削除通知（参照している場合に安全に参照解除）
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::NotifyDeleteCamera( Camera * pCamera )
{
	S3DScene::NotifyDeleteCamera( pCamera ) ;

	Lock() ;
	for ( size_t i = 0; i < m_psaViewEntries.GetLength(); i ++ )
	{
		S3DSceneSprite::ViewEntry *	pView = m_psaViewEntries.GetAt(i) ;
		if ( (pView != nullptr)
			&& (pView->pCamera == pCamera) )
		{
			pView->pCamera = nullptr ;
		}
	}
	Unlock() ;
}

// メインカメラ設定
//////////////////////////////////////////////////////////////////////////
void S3DSceneSprite::SetMainCamera( Camera * pCamera )
{
	S3DSceneSprite::SetMainCamera( pCamera, nullptr ) ;
}

void S3DSceneSprite::SetMainCamera( Camera * pCamera, SGLSecondaryViewProducer * psvp )
{
	if ( psvp == nullptr )
	{
		S3DScene::SetMainCamera( pCamera ) ;
	}
	S3DSceneSprite::ViewEntry *	pView = GetViewSettingAs( psvp ) ;
	if ( pView != nullptr )
	{
		pView->pCamera = pCamera ;
	}
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneSprite::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneSprite" ;
}

