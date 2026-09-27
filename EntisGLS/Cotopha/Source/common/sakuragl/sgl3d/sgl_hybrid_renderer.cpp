
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuragl/sgl3d/sgl_hybrid_renderer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ハイブリッド・レンダラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST
	( SakuraGL::S3DHybridRenderContext, S3DRenderContext, m_paint )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DHybridRenderContext::S3DHybridRenderContext( SGLPaintContextType type )
	: S3DRenderContext( NULL, false )
{
	m_mode3d = false ;
	m_modeRenderOnly = false ;
	m_flagViewPort = false ;
	m_paint = NULL ;

	if ( type == typePaintDefault )
	{
		type = GetDefaultRenderType() ;
	}
#if	defined(__COTOPHA__)
	AttachRenderContext
		( RenderContext::NewContext( type ), true ) ;
#else
	if ( type == typePaintEntisGLS )
	{
		AttachRenderContext
			( new SakuraGL::SGLStandardRenderContext, true ) ;
		//
		#if	!defined(__PLATFORM_WINDOWS__) || !defined(__ENTIS_GLS__)
		m_paint = new SGLPaintBuffer ;
		#endif
	}
	else
	{
		AttachRenderContext
			( S3DRenderContextInterface::NewContext(type), true ) ;
	}
#endif
}

S3DHybridRenderContext::S3DHybridRenderContext
		( RenderContext * render, SGLPaintContextInterface * paint )
	: S3DRenderContext( render, true )
{
	m_mode3d = false ;
	m_modeRenderOnly = false ;
	m_flagViewPort = false ;
	m_paint = paint ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DHybridRenderContext::~S3DHybridRenderContext( void )
{
	delete	m_paint ;
	m_paint = NULL ;
}

// PaintContext 取得
//////////////////////////////////////////////////////////////////////////////
PaintContext * S3DHybridRenderContext::GetPaintContext( void ) const
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		#if	defined(__COTOPHA__)
			return	m_paint->GetPaintContextObject() ;
		#else
			return	m_paint ;
		#endif
	}
	return	S3DRenderContext::GetPaintContext() ;
}

S3DHybridRenderContext::operator PaintContext * ( void ) const
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		#if	defined(__COTOPHA__)
			return	m_paint->GetPaintContextObject() ;
		#else
			return	m_paint ;
		#endif
	}
	return	S3DRenderContext::GetPaintContext() ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DHybridRenderContext::GetTargetImage( void )
{
	if ( m_pTarget != NULL )
	{
		return	m_pTarget ;
	}
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		SGLImageObject *	pimgTarget = m_paint->GetTargetImage() ;
		if ( pimgTarget != NULL )
		{
			m_imgTarget.SetImageObject( pimgTarget->GetImageObject(), false ) ;
		}
		else
		{
			m_imgTarget.SetImageObject( NULL, false ) ;
		}
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		m_imgTarget.SetImageObject( m_render->GetTargetImage(), false ) ;
	}
	return	&m_imgTarget ;
}

SGLImageObject * S3DHybridRenderContext::GetTargetZBuffer( void )
{
	if ( m_pZBuffer != NULL )
	{
		return	m_pZBuffer ;
	}
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		SGLImageObject *	pimgZBuf = m_paint->GetTargetZBuffer() ;
		if ( pimgZBuf != NULL )
		{
			m_imgZBuffer.SetImageObject( pimgZBuf->GetImageObject(), false ) ;
		}
		else
		{
			m_imgZBuffer.SetImageObject( NULL, false ) ;
		}
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		m_imgZBuffer.SetImageObject( m_render->GetTargetZBuffer(), false ) ;
	}
	return	&m_imgZBuffer ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::GetViewPort( SGLImageRect & rctView ) const
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->GetViewPort( rctView ) ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->GetViewPort( rctView ) ;
	}
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pTarget = pImage ;
	m_pZBuffer = pZBuffer ;
	//
	m_flagViewPort = false ;
	if ( pView != NULL )
	{
		m_rectViewPort = *pView ;
		m_flagViewPort = true ;
	}
	m_modeRenderOnly = false ;
	if ( pImage && (pImage->GetBufferFlags()
						& SGLImageObject::bufferOnDeviceOnly) )
	{
		if ( (pZBuffer == NULL)
			|| (pZBuffer->GetBufferFlags()
						& SGLImageObject::bufferOnDeviceOnly) )
		{
			m_modeRenderOnly = true ;
		}
	}
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->AttachTargetImage( pImage, pZBuffer, pView ) ;
	}
	else
	{
		Image *	imgTarget = NULL ;
		Image *	imgZBuffer = NULL ;
		if ( pImage != NULL )
		{
			imgTarget = pImage->GetImageObject() ;
			if ( imgTarget == NULL )
			{
				Trace( "failed to GetImageObject for target "
						"at SGLRenderContext::AttachTargetImage\n" ) ;
			}
		}
		if ( pZBuffer != NULL )
		{
			imgZBuffer = pZBuffer->GetImageObject() ;
			if ( imgZBuffer == NULL )
			{
				Trace( "failed to GetImageObject for z buffer "
						"at SGLRenderContext::AttachTargetImage\n" ) ;
			}
		}
		ESLAssert( m_render != NULL ) ;
		return	m_render->AttachTargetImage( imgTarget, imgZBuffer, pView ) ;
	}
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::DetachTargetImage( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	m_flagViewPort = false ;
	//
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->DetachTargetImage() ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->DetachTargetImage() ;
	}
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	if ( m_paint != NULL )
	{
		m_paint->AppendTransformation( af, nTransparency ) ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->AppendTransformation( af, nTransparency ) ;
}

SGLError S3DHybridRenderContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	if ( m_paint != NULL )
	{
		m_paint->SetTransformation( af, nTransparency ) ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->SetTransformation( af, nTransparency ) ;
}

SGLError S3DHybridRenderContext::PushTransformation( void )
{
	if ( m_paint != NULL )
	{
		m_paint->PushTransformation() ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->PushTransformation() ;
}

SGLError S3DHybridRenderContext::PopTransformation( void )
{
	if ( m_paint != NULL )
	{
		m_paint->PopTransformation() ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->PopTransformation() ;
}

SGLError S3DHybridRenderContext::ResetTransformation( void )
{
	if ( m_paint != NULL )
	{
		m_paint->ResetTransformation() ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->ResetTransformation() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DHybridRenderContext::SetPaintFlags( int64_t nFlags )
{
	if ( m_paint != NULL )
	{
		m_paint->SetPaintFlags( nFlags ) ;
	}
	ESLAssert( m_render != NULL ) ;
	m_render->SetPaintFlags( nFlags ) ;
}

int64_t S3DHybridRenderContext::GetPaintFlags( void )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->GetPaintFlags() ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->GetPaintFlags() ;
	}
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::FillClearTarget
	( uint32_t argb, int64_t flags )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->FillClearTarget( argb, flags ) ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->FillClearTarget( argb, flags ) ;
	}
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::FillRectangle
	( int x, int y, int width, int height,
				uint32_t argb, double z, uint32_t flags )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->FillRectangle( x, y, width, height, argb, z, flags ) ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->FillRectangle( x, y, width, height, argb, z, flags ) ;
	}
}

SGLError S3DHybridRenderContext::FillPolygon
	( const S2DVector * vertices, size_t count,
				uint32_t argb, double z, uint32_t flags )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->FillPolygon( vertices, count, argb, z, flags ) ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->FillPolygon( vertices, count, argb, z, flags ) ;
	}
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->DrawImage( ppPaint, pSrcImage, pSrcClip ) ;
	}
	else
	{
		Image *	imgSrcImage = NULL ;
		if ( pSrcImage != NULL )
		{
			imgSrcImage = pSrcImage->GetImageObject() ;
			if ( imgSrcImage == NULL )
			{
				Trace( "failed to GetImageObject"
						" at SGLRenderContext::DrawImage\n" ) ;
				return	sglErrFailed ;
			}
		}
		ESLAssert( m_render != NULL ) ;
		return	m_render->DrawImage( ppPaint, imgSrcImage, pSrcClip ) ;
	}
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->DrawMesh
			( pDstMesh, pSrcMesh, widthMesh, heightMesh,
							ppPaint, pSrcImage, pSrcClip ) ;
	}
	else
	{
		Image *	imgSrcImage = NULL ;
		if ( pSrcImage != NULL )
		{
			imgSrcImage = pSrcImage->GetImageObject() ;
			if ( imgSrcImage == NULL )
			{
				Trace( "failed to GetImageObject"
						"at SGLRenderContext::DrawImage\n" ) ;
			}
		}
		ESLAssert( m_render != NULL ) ;
		return	m_render->DrawMesh
			( pDstMesh, pSrcMesh, widthMesh, heightMesh,
							ppPaint, imgSrcImage, pSrcClip ) ;
	}
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->DrawMultiImages
					( nCount, pParams, ppSrcImages, pSrcClips ) ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->DrawMultiImages
					( nCount, pParams, ppSrcImages, pSrcClips ) ;
	}
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::Flush( void )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->Flush() ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->Flush() ;
	}
}

SGLError S3DHybridRenderContext::Finish( void )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		return	m_paint->Finish() ;
	}
	else
	{
		ESLAssert( m_render != NULL ) ;
		return	m_render->Finish() ;
	}
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::Begin3DRenderer( uint64_t nFlags )
{
	if ( !m_mode3d && m_paint && !m_modeRenderOnly )
	{
		m_paint->Finish() ;
		m_paint->DetachTargetImage() ;
		m_mode3d = true ;
		//
		Image *			imgTarget = NULL ;
		Image *			imgZBuffer = NULL ;
		SGLImageRect *	pView = NULL ;
		if ( m_pTarget != NULL )
		{
			imgTarget = m_pTarget->GetImageObject() ;
		}
		if ( m_pZBuffer != NULL )
		{
			imgZBuffer = m_pZBuffer->GetImageObject() ;
		}
		if ( m_flagViewPort )
		{
			pView = &m_rectViewPort ;
		}
		ESLAssert( m_render != NULL ) ;
		m_render->AttachTargetImage( imgTarget, imgZBuffer, pView ) ;
	}
	ESLAssert( m_render != NULL ) ;
	return	m_render->Begin3DRenderer( nFlags ) ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DHybridRenderContext::End3DRenderer( uint64_t nFlags )
{
	ESLAssert( m_render != NULL ) ;
	SGLError	err = m_render->End3DRenderer( nFlags ) ;
	//
	if ( m_mode3d && m_paint && !m_modeRenderOnly )
	{
		m_render->Finish() ;
		m_render->DetachTargetImage() ;
		//
		SGLImageRect *	pView = NULL ;
		if ( m_flagViewPort )
		{
			pView = &m_rectViewPort ;
		}
		m_paint->AttachTargetImage( m_pTarget, m_pZBuffer, pView ) ;
		m_mode3d = false ;
	}
	return	err ;
}

#if	defined(__COTOPHA__)
// PaintContext 取得
//////////////////////////////////////////////////////////////////////////////
PaintContext * S3DHybridRenderContext::GetPaintContextObject( void ) const
{
	if ( !m_mode3d && m_paint )
	{
		return	m_paint->GetPaintContextObject() ;
	}
	return	S3DRenderContext::GetPaintContext() ;
}
#endif
