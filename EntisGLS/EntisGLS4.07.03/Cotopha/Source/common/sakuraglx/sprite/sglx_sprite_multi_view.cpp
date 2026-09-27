
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_multi_view.h>
#include <sakuragl/sgl_vr_view_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 描画バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteMultiView::SecondaryBuffer, Buffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::SecondaryBuffer::SecondaryBuffer( SGLPaintContextType type )
	: Buffer( type )
{
	m_buffered = false ;
	m_visible = true ;
	m_nTransparency = 0 ;
}

SGLSpriteMultiView::SecondaryBuffer::SecondaryBuffer
		( const SGLSpriteMultiView::SecondaryBuffer& buf )
{
	m_flagStereo3D = buf.m_flagStereo3D ;
	m_flagZBuffer = buf.m_flagZBuffer ;
	m_flagFillBack = buf.m_flagFillBack ;
	m_rgbaFillBack = buf.m_rgbaFillBack ;
	m_nBufFlags = buf.m_nBufFlags ;
	m_buffered = buf.m_buffered ;
	m_visible = buf.m_visible ;
	m_nTransparency = buf.m_nTransparency ;
	m_affine[0] = buf.m_affine[0] ;
	m_affine[1] = buf.m_affine[1] ;
	m_pUserLayer = NULL ;
	//
	if ( buf.m_buffered )
	{
		SGLImageInfo	imginf ;
		if ( buf.m_imgBuffer.GetImageInfo( imginf ) == sglErrSuccess )
		{
			CreateBuffer
				( imginf.width, imginf.height,
					imginf.format, imginf.depth,
					buf.m_nBufFlags,
					buf.m_flagZBuffer, buf.m_flagStereo3D ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::SecondaryBuffer::~SecondaryBuffer( void )
{
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SecondaryBuffer::CreateBuffer
	( uint32_t width, uint32_t height,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D )
{
	SGLError	err =
		Buffer::CreateBuffer
			( width, height, format, depth,
				nBufFlags, flagZBuffer, flagStereo3D ) ;
	m_buffered = (err == sglErrSuccess) ;
	return	err ;
}

SGLError SGLSpriteMultiView::SecondaryBuffer::CreateStereoBuffer
	( const SGLSize& sizeRight, const SGLSize& sizeLeft,
		uint32_t format, uint32_t depth, uint64_t nBufFlags, bool flagZBuffer )
{
	m_imgBuffer.ReleaseBuffer() ;
	m_imgLeftBuffer.ReleaseBuffer() ;
	m_imgZBuffer.ReleaseBuffer() ;
	m_flagStereo3D = false ;
	m_flagZBuffer = false ;
	m_flagMultiSampling = ((nBufFlags & SGLImageObject::bufferRenderNonTextureFlags) != 0) ;
	m_nBufFlags = SGLImageObject::bufferNonPowerOf2 | nBufFlags ;
	nBufFlags = SGLImageObject::bufferNonPowerOf2
					| (nBufFlags & ~uint64_t(SGLImageObject::bufferRenderNonTextureFlags)) ;
	//
	SGLImageInfo	imginf ;
	imginf.format = format ;
	imginf.width = (uint32_t) sizeRight.w ;
	imginf.height = (uint32_t) sizeRight.h ;
	imginf.depth = depth ;
	if ( m_imgBuffer.CreateBuffer( imginf, nBufFlags ) )
	{
		return	sglErrFailed ;
	}
	imginf.width = (uint32_t) sizeLeft.w ;
	imginf.height = (uint32_t) sizeLeft.h ;
	if ( !m_imgLeftBuffer.CreateBuffer( imginf, nBufFlags ) )
	{
		m_flagStereo3D = true ;
	}
	if ( flagZBuffer )
	{
		imginf.format = formatImageDepth ;
		imginf.width = (uint32_t) esl_max( sizeRight.w, sizeLeft.w ) ;
		imginf.height = (uint32_t) esl_max( sizeRight.h, sizeLeft.h ) ;
		imginf.depth = 32 ;
		if ( !m_imgZBuffer.CreateBuffer( imginf, nBufFlags ) )
		{
			m_flagZBuffer = true ;
		}
	}
	if ( m_flagMultiSampling )
	{
		PrepareMultiSampling() ;
	}
	m_buffered = true ;
	return	sglErrSuccess ;
}

// 表示状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMultiView::SecondaryBuffer::IsVisible( void ) const
{
	return	m_visible ;
}

void SGLSpriteMultiView::SecondaryBuffer::SetVisible( bool fVisible )
{
	m_visible = fVisible ;
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteMultiView::SecondaryBuffer::GetTransparency( void ) const
{
	return	m_nTransparency ;
}

void SGLSpriteMultiView::SecondaryBuffer::SetTransparency( uint32_t nTransparency )
{
	m_nTransparency = nTransparency ;
}

// 変換行列
//////////////////////////////////////////////////////////////////////////////
const SGLAffine& SGLSpriteMultiView::SecondaryBuffer::GetAffine( size_t iView ) const
{
	ESLAssert( iView < 2 ) ;
	return	(iView != 1) ? m_affine[0] : m_affine[1] ;
}

SGLError SGLSpriteMultiView::SecondaryBuffer::SetAffine( size_t iView, const SGLAffine & affine )
{
	if ( iView >= 2 )
	{
		return	sglErrFailed ;
	}
	m_affine[iView] = affine ;
	return	sglErrSuccess ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteMultiView::SecondaryBuffer::DuplicateObject( void )
{
	return	new SecondaryBuffer( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SecondaryBuffer::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Buffer::OnSave( file ) ;
	//
	uint32_t	nFlags = 0 ;
	if ( m_visible )
	{
		nFlags |= 0x0001 ;
	}
	if ( m_buffered )
	{
		nFlags |= 0x0002 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	//
	SGLImageInfo	imginf ;
	m_imgLeftBuffer.GetImageInfo( imginf ) ;
	file.Write( &imginf.width, sizeof(uint32_t) ) ;
	file.Write( &imginf.height, sizeof(uint32_t) ) ;
	//
	file.Write( &m_affine[0], sizeof(SGLAffine) * 2 ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SecondaryBuffer::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Buffer::OnRestore( file ) ;
	//
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	//
	m_visible = ((nFlags & 0x0001) != 0) ;
	m_buffered = ((nFlags & 0x0002) != 0) ;
	//
	uint32_t	widthLeft, heightLeft ;
	file.Read( &widthLeft, sizeof(uint32_t) ) ;
	file.Read( &heightLeft, sizeof(uint32_t) ) ;
	//
	file.Read( &m_affine[0], sizeof(SGLAffine) * 2 ) ;
	//
	if ( m_buffered && m_flagStereo3D )
	{
		SGLImageInfo	imginf ;
		m_imgBuffer.GetImageInfo( imginf ) ;
		if ( (imginf.width != widthLeft) || (imginf.height != heightLeft) )
		{
			imginf.width = widthLeft ;
			imginf.height = heightLeft ;
			if ( m_imgLeftBuffer.CreateBuffer( imginf, m_nBufFlags ) )
			{
				m_flagStereo3D = false ;
			}
		}
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// 出力先ごとの表示パラメータを持つスプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMultiView, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::SGLSpriteMultiView( void )
{
	m_pSelectView = NULL ;
	m_pSelectBuffer = NULL ;
}

SGLSpriteMultiView::SGLSpriteMultiView( const SGLSpriteMultiView& src )
	: SGLSprite( src )
{
	m_pSelectView = NULL ;
	m_pSelectBuffer = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::~SGLSpriteMultiView( void )
{
	DetachSyncTimeout( 100 ) ;
}

// 出力先バッファ選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SelectSecondaryView( SGLSecondaryViewProducer * psvp )
{
	Lock() ;
	if ( m_pSelectBuffer != NULL )
	{
		m_pSelectBuffer->m_statusUpdate = m_statusUpdate ;
		m_pSelectBuffer->m_rectUpdate = m_rectUpdate ;
	}
	//
	m_pSelectView = psvp ;
	m_pSelectBuffer = GetSecondaryBufferOf( psvp ) ;
	//
	if ( m_pSelectBuffer != NULL )
	{
		m_statusUpdate = m_pSelectBuffer->m_statusUpdate ;
		m_rectUpdate = m_pSelectBuffer->m_rectUpdate ;
		//
		m_refImage = &(m_pSelectBuffer->m_imgBuffer) ;
		m_refLeftImage = NULL ;
		if ( m_pSelectBuffer->m_flagStereo3D )
		{
			m_refLeftImage = &(m_pSelectBuffer->m_imgLeftBuffer) ;
		}
	}
	else if ( m_pBuffer != NULL )
	{
		m_refImage = m_pBuffer->GetImage() ;
		m_refLeftImage = NULL ;
		if ( m_pBuffer->IsStereo3D() )
		{
			m_refLeftImage = m_pBuffer->GetLeftImage() ;
		}
	}
	else
	{
		m_refImage = NULL ;
		m_refLeftImage = NULL ;
	}
	Unlock() ;
	return	sglErrSuccess ;
}

// 出力先バッファを取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::SecondaryBuffer *
		SGLSpriteMultiView::GetSecondaryBufferOf
					( SGLSecondaryViewProducer * psvp ) const
{
	SecondaryBuffer *	pBuffer = NULL ;
	ESLAssert( TestLocked() ) ;
	if ( psvp != NULL )
	{
		pBuffer = m_psoaFrameBuffers.GetAs( psvp ) ;
	}
	else
	{
		pBuffer = ESLTypeCast<SecondaryBuffer>( m_pBuffer.Ptr() ) ;
	}
	return	pBuffer ;
}

// 選択中の出力ビュー取得
//////////////////////////////////////////////////////////////////////////////
SGLSecondaryViewProducer * SGLSpriteMultiView::GetSelectedSecondaryView( void ) const
{
	return	m_pSelectView ;
}

// バッファを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMultiView::SecondaryBuffer *
		SGLSpriteMultiView::GetSelectedFrameBuffer( void ) const
{
	return	m_pSelectBuffer ;
}

SGLSpriteMultiView::SecondaryBuffer *
	SGLSpriteMultiView::CreateSelectedFrameBuffer
								( SGLPaintContextType type )
{
	SecondaryBuffer *	pBuffer = m_pSelectBuffer ;
	if ( pBuffer == NULL )
	{
		if ( m_pSelectView == NULL )
		{
			pBuffer = new SecondaryBuffer( type ) ;
			if ( m_pBuffer != NULL )
			{
				SGLImageInfo	imginf ;
				if ( !pBuffer->m_imgBuffer.GetImageInfo( imginf ) )
				{
					pBuffer->CreateBuffer
						( imginf.width, imginf.height,
							imginf.format, imginf.depth,
							m_pBuffer->GetBufFlags(),
							m_pBuffer->HasZBuffer(),
							m_pBuffer->IsStereo3D() ) ;
				}
			}
			m_pBuffer = pBuffer ;
		}
		else
		{
			pBuffer = new SecondaryBuffer( type ) ;
			m_psoaFrameBuffers.SetAs( m_pSelectView, pBuffer ) ;
		}
		m_pSelectBuffer = pBuffer ;
	}
	return	pBuffer ;
}

// ユーザー定義バッファ取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * SGLSpriteMultiView::GetSelectedUserLayerBuffer( void ) const
{
	SecondaryBuffer *	pBuffer = m_pSelectBuffer ;
	if ( pBuffer == NULL )
	{
		return	pBuffer->m_pUserLayer ;
	}
	return	NULL ;
}

// レンダリングデバイスの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SetRenderDevice( S3DRenderDevice * pDevice )
{
	SGLError	err = sglErrSuccess ;
	Lock() ;
	Buffer *	pBuffer = m_pBuffer ;
	if ( pBuffer != NULL )
	{
		err = pBuffer->Renderer().SetRenderDeviceObject( pDevice ) ;
	}
	for ( size_t i = 0; i < m_psoaFrameBuffers.GetLength(); i ++ )
	{
		SecondaryBuffer *	pSubBuf = m_psoaFrameBuffers.GetAt( i ) ;
		if ( pSubBuf != NULL )
		{
			SGLError	errSub =
				pSubBuf->m_render.SetRenderDeviceObject( pDevice ) ;
			if ( !err )
			{
				err = errSub ;
			}
		}
	}
	Unlock() ;
	return	err ;
}

// ビュー固有可視状態
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SetVisibleOfView( bool fVisible )
{
	Lock() ;
	SecondaryBuffer *	pBuffer = CreateSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	pBuffer->m_visible = fVisible ;
	Unlock() ;
	return	sglErrSuccess ;
}

bool SGLSpriteMultiView::IsVisibleOfView( void ) const
{
	SecondaryBuffer *	pBuffer = GetSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		return	true ;
	}
	return	pBuffer->m_visible ;
}

// ビュー固有透明度
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SetTransparencyOfView( uint32_t nTransparency )
{
	Lock() ;
	SecondaryBuffer *	pBuffer = CreateSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	pBuffer->m_nTransparency = nTransparency ;
	Unlock() ;
	return	sglErrSuccess ;
}

uint32_t SGLSpriteMultiView::GetTransparencyOfView( void ) const
{
	SecondaryBuffer *	pBuffer = GetSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		return	0 ;
	}
	return	pBuffer->m_nTransparency ;
}

// ビュー固有座標変換
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SetAffineOfView
	( const SGLAffine * affine, size_t nCount )
{
	if ( nCount > 2 )
	{
		return	sglErrFailed ;
	}
	Lock() ;
	SecondaryBuffer *	pBuffer = CreateSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pBuffer->m_affine[i] = affine[i] ;
	}
	Unlock() ;
	return	sglErrSuccess ;
}

size_t SGLSpriteMultiView::GetAffineOfView( SGLAffine * affine, size_t nCount ) const
{
	SecondaryBuffer *	pBuffer = GetSelectedFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		return	0 ;
	}
	nCount = (size_t) esl_min( (int) nCount, 2 ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		affine[i] = pBuffer->m_affine[i] ;
	}
	return	nCount ;
}

// VR HMD 表示で任意距離のスクリーンとしてパラメータを設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::SetViewSettingForVRHMD
	( const SGLVRViewProducer * pVR,
		const S3DVector& vScreenDstPos /* 表示する３次元座標 */,
		const S2DVector& vScreenCenter /* 表示画像の中心座標 */,
		const S2DVector& vScreenZoom /* 見かけ上の拡大率 */ )
{
	Virtual3DParam*	pV3D = NULL ;
	if ( m_paramView.nSpriteFlags & flagZScale )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pV3D = pParent->GetVirtual3DParam() ;
		}
	}
	const SGLVRViewProducer::EyeIndex	eyeIndex[2] =
	{
		SGLVRViewProducer::eyeRight,
		SGLVRViewProducer::eyeLeft,
	} ;
	const SGLSprite::Stereo3DView	s3dView[2] =
	{
		s3dRightView,
		s3dLeftView,
	} ;
	SGLAffine	affineView[2] ;
	for ( int i = 0; i < 2; i ++ )
	{
		const SGLVRViewProducer::Posture&
			postureEye = pVR->GetEyePosture( eyeIndex[i] ) ;
		const SGLVRViewProducer::EyeFieldOfView&
			fovEye = pVR->GetEyeFieldOfView( eyeIndex[i] ) ;
		//
		SGLPaintParam	pp ;
		SGLAffine		affineOrg ;
		if ( !SGLSprite::GetPaintParam( pp, affineOrg, pV3D, s3dView[i] ) )
		{
			return	sglErrFailed ;
		}
		S3DMatrix	matCamera ;
		matCamera.InverseOf( postureEye.matOrientation ) ;
		//
		S3DVector	vViewPos =
			matCamera * (vScreenDstPos - postureEye.vPosition) ;
		//
		S2DVector	vProjPos ;
		float32_t	d = fovEye.vScreenPos.z / vViewPos.z ;
		vProjPos.x = vViewPos.x * d + fovEye.vScreenPos.x
						- vScreenZoom.x * vScreenCenter.x ;
		vProjPos.y = vViewPos.y * d + fovEye.vScreenPos.y
						- vScreenZoom.y * vScreenCenter.y ;
		//
		SGLAffine	affineDst( vScreenZoom.x, 0.0f, vProjPos.x,
								0.0f, vScreenZoom.y, vProjPos.y ) ;
		SGLAffine	affineInv ;
		affineInv.InverseOf( affineOrg ) ;
		//
		affineView[i] = affineInv * affineDst ;
	}
	return	SetAffineOfView( affineView, 2 ) ;
}

// スクリーン表示パラメータ計算
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::CalcViewSettingForVRHMD
	( S3DVector& vScreenDstPos,
		S2DVector& vScreenCenter,
		S2DVector& vScreenZoom,
		const SGLVRViewProducer * pVR,
		double tanHFOV2, /* 左右視野角90度に対する拡大率 */
		double zScreenPos /* スクリーン設置ｚ座標 */ )
{
	const SGLVRViewProducer::EyeFieldOfView&
				fovHMD = pVR->GetEyeFieldOfView
							( SGLVRViewProducer::eyeRight ) ;
	SGLSize		sizeScreen = GetImageSize() ;
	double		zoomLayer = tanHFOV2 * fovHMD.vScreenPos.z / sizeScreen.w ;
	vScreenDstPos = S3DVector( 0, 0, zScreenPos ) ;
	vScreenCenter = S2DVector( sizeScreen.w * 0.5, sizeScreen.h * 0.5 ) ;
	vScreenZoom = S2DVector( zoomLayer, zoomLayer ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::BeforeDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSecondaryViewProducer *	psvp = SGLSecondaryViewProducer::GetCurrent() ;
	SelectSecondaryView( psvp ) ;
	//
	SGLSprite::BeforeDraw( s3dView ) ;
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::AfterDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSprite::AfterDraw( s3dView ) ;
	//
	SelectSecondaryView( NULL ) ;
}

// 更新領域通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::PostUpdate( SGLRect* pUpdate )
{
	LockTrace( __FILE__, __LINE__ ) ;
	//
	UpdateStatus	statusLastUpdate = m_statusUpdate ;
	SGLRect			rectLastUpdate = m_rectUpdate ;
	//
	SGLSprite::PostUpdate( pUpdate ) ;
	//
	bool	fUpdateParent = (statusLastUpdate != m_statusUpdate)
							|| (rectLastUpdate != m_rectUpdate) ;
	//
	if ( m_pSelectBuffer != NULL )
	{
		m_pSelectBuffer->m_statusUpdate = m_statusUpdate ;
		m_pSelectBuffer->m_rectUpdate = m_rectUpdate ;
	}
	bool	fUpdate = false ;
	for ( size_t i = 0; i < m_psoaFrameBuffers.GetLength(); i ++ )
	{
		SecondaryBuffer *	pBuf = m_psoaFrameBuffers.GetAt( i ) ;
		if ( (pBuf != NULL) && (pBuf != m_pSelectBuffer) )
		{
			if ( pBuf->m_statusUpdate != updateFull )
			{
				if ( pUpdate != NULL )
				{
					pBuf->m_rectUpdate |= *pUpdate ;
					pBuf->m_statusUpdate = updateRect ;
				}
				else
				{
					pBuf->m_statusUpdate = updateFull ;
				}
				if ( pBuf->m_buffered && (pBuf->m_statusUpdate != updateFull) )
				{
					SGLImageInfo	imginf ;
					if ( pBuf->m_imgBuffer.
							GetImageInfo( imginf ) == sglErrSuccess )
					{
						if ( (pBuf->m_rectUpdate.left <= 0)
							& (pBuf->m_rectUpdate.top <= 0)
							& (pBuf->m_rectUpdate.right + 1 >= (int32_t) imginf.width)
							& (pBuf->m_rectUpdate.bottom + 1 >= (int32_t) imginf.height) )
						{
							pBuf->m_statusUpdate = updateFull ;
						}
					}
				}
				fUpdate = true ;
			}
		}
	}
	if ( !fUpdateParent && fUpdate )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			SGLRect	rectExt ;
			if ( GetRectangle( rectExt ) )
			{
				pParent->PostUpdate( &rectExt ) ;
			}
			else
			{
				pParent->PostUpdate() ;
			}
		}
	}
	Unlock() ;
}

// 描画パラメータ取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMultiView::GetPaintParam
	( SGLPaintParam& pp, SGLAffine& affine,
		const SGLSprite::Virtual3DParam* pV3D,
		SGLSprite::Stereo3DView s3dView ) const
{
	if ( !SGLSprite::GetPaintParam( pp, affine, pV3D, s3dView ) )
	{
		return	false ;
	}
	SGLSecondaryViewProducer *	psvp = SGLSecondaryViewProducer::GetCurrent() ;
	SecondaryBuffer *	pBuffer = GetSecondaryBufferOf( psvp ) ;
	if ( pBuffer == NULL )
	{
		return	true ;
	}
	if ( !pBuffer->m_visible )
	{
		return	false ;
	}
	if ( pBuffer->m_nTransparency > 0 )
	{
		pp.nTransparency =
			(uint32_t) (0x100 - (0x100 - (int) pp.nTransparency)
						* (0x100 - (int) pBuffer->m_nTransparency) / 0x100) ;
	}
	SGLAffine	affineView = pBuffer->m_affine[0] ;
	if ( s3dView == s3dLeftView )
	{
		affineView = pBuffer->m_affine[1] ;
	}
	if ( pp.pAffine != NULL )
	{
		ESLAssert( pp.pAffine == &affine ) ;
		affine *= affineView ;
	}
	else
	{
		affine = affineView ;
		pp.pAffine = &affine ;
	}
	return	true ;
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::CreateBuffer
	( uint32_t width, uint32_t height,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D,
		SGLPaintContextType type )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SecondaryBuffer *	pBuffer = CreateSelectedFrameBuffer( type ) ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	if ( pBuffer->CreateBuffer
		( width, height, format, depth, nBufFlags, flagZBuffer, flagStereo3D ) )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	m_refImage = &(pBuffer->m_imgBuffer) ;
	m_refLeftImage = NULL ;
	if ( pBuffer->m_flagStereo3D )
	{
		m_refLeftImage = &(pBuffer->m_imgLeftBuffer) ;
	}
	SGLSize	sizeBuffer( width, height ) ;
	OnCreateUserLayerBuffer
		( pBuffer, sizeBuffer, sizeBuffer,
			format, depth, nBufFlags, flagZBuffer, flagStereo3D ) ;
	Unlock() ;
	return	sglErrSuccess ;
}

SGLError SGLSpriteMultiView::CreateStereoBuffer
	( const SGLSize& sizeRight,
		const SGLSize& sizeLeft,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer,
		SGLPaintContextType type )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SecondaryBuffer *	pBuffer = CreateSelectedFrameBuffer( type ) ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	if ( pBuffer->CreateStereoBuffer
		( sizeRight, sizeLeft, format, depth, nBufFlags, flagZBuffer ) )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	m_refImage = &(pBuffer->m_imgBuffer) ;
	m_refLeftImage = NULL ;
	if ( pBuffer->m_flagStereo3D )
	{
		m_refLeftImage = &(pBuffer->m_imgLeftBuffer) ;
	}
	OnCreateUserLayerBuffer
		( pBuffer, sizeRight, sizeLeft,
			format, depth, nBufFlags, flagZBuffer, true ) ;
	Unlock() ;
	return	sglErrSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::ReleaseBuffer( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pSelectBuffer != NULL)
		&& (m_pSelectBuffer->m_pUserLayer != NULL) )
	{
		OnReleaseUserLayerBuffer( m_pSelectBuffer->m_pUserLayer ) ;
	}
	if ( m_pSelectView == NULL )
	{
		m_pBuffer = NULL ;
	}
	else
	{
		m_psoaFrameBuffers.RemoveAs( m_pSelectView ) ;
	}
	m_pSelectBuffer = NULL ;
	Unlock() ;
}

// バッファを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Buffer * SGLSpriteMultiView::GetFrameBuffer( void ) const
{
	if ( m_pSelectBuffer != NULL )
	{
		if ( m_pSelectBuffer->m_buffered )
		{
			return	m_pSelectBuffer ;
		}
	}
	else if ( m_pSelectView == NULL )
	{
		return	SGLSprite::GetFrameBuffer() ;
	}
	return	NULL ;
}

// ユーザー定義バッファ生成
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::OnCreateUserLayerBuffer
	( SGLSpriteMultiView::SecondaryBuffer * pBuffer,
		const SGLSize& sizeRight, const SGLSize& sizeLeft,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D )
{
}

// ユーザー定義バッファ解放時処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMultiView::OnReleaseUserLayerBuffer( ESLObject * pUserLayer )
{
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteMultiView::DuplicateObject( void )
{
	return	new SGLSpriteMultiView( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::OnSave( SSystem::SFileInterface& file )
{
	return	SGLSprite::OnSave( file ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMultiView::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	//
	SecondaryBuffer *	pBuffer = GetSecondaryBufferOf( nullptr ) ;
	if ( (pBuffer != nullptr) && (pBuffer->GetImage() != nullptr) )
	{
		SGLImageInfo	imginf ;
		pBuffer->GetImage()->GetImageInfo( imginf ) ;
		//
		SGLSize	sizeRight( imginf.width, imginf.height ) ;
		SGLSize	sizeLeft = sizeRight ;
		if ( pBuffer->GetTempLeftImage() != nullptr )
		{
			sizeLeft = pBuffer->GetTempLeftImage()->GetImageSize() ;
		}
		OnCreateUserLayerBuffer
			( pBuffer, sizeRight, sizeLeft,
				imginf.format, imginf.depth,
				pBuffer->GetBufFlags(),
				pBuffer->HasZBuffer(), pBuffer->IsStereo3D() ) ;
	}
	return	err ;
}


