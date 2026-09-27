
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuragl/sgl3d/sgl_render_buffered_context.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 描画パラメータ
//////////////////////////////////////////////////////////////////////////////

// 描画座標の設定
//////////////////////////////////////////////////////////////////////////////
void SGLPaintParam::SetAffine
	( SGLAffine& affine, double xDst, double yDst,
		double xSrcOrg, double ySrcOrg,
		double xZoom, double yZoom, double zAngle, double xyCross )
{
	zAngle *= SSystem::PI / 180.0 ;
	affine.a11 = (float32_t) (xZoom * cos( zAngle )) ;
	affine.a21 = (float32_t) (xZoom * sin( zAngle )) ;
	//
	xyCross *= SSystem::PI / 180.0 ;
	xyCross += zAngle ;
	affine.a12 = (float32_t) (yZoom * cos( xyCross )) ;
	affine.a22 = (float32_t) (yZoom * sin( xyCross )) ;
	//
	affine.a13 =
		(float32_t) (xDst - xSrcOrg * affine.a11
							- ySrcOrg * affine.a12) ;
	affine.a23 =
		(float32_t) (yDst - xSrcOrg * affine.a21
							- ySrcOrg * affine.a22) ;
	//
	pAffine = &affine ;
}

// アフィン取得
//////////////////////////////////////////////////////////////////////////////
void SGLPaintParam::GetAffine( SGLAffine& affine ) const
{
	if ( pAffine != NULL )
	{
		affine = *pAffine ;
	}
	else
	{
		affine.a11 = 1.0f ;
		affine.a12 = 0.0f ;
		affine.a13 = 0.0f ;
		affine.a21 = 0.0f ;
		affine.a22 = 1.0f ;
		affine.a23 = 0.0f ;
	}
	if ( nFlags & paintFixedPosition )
	{
		affine.a13 += (float32_t) (ptPaint.x / 65536.0) ;
		affine.a23 += (float32_t) (ptPaint.y / 65536.0) ;
	}
	else
	{
		affine.a13 += (float32_t) ptPaint.x ;
		affine.a23 += (float32_t) ptPaint.y ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 抽象画像描画コンテキスト
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLPaintContextInterface, SObject )

#if	defined(__COTOPHA__)
// PaintContext 取得
PaintContext * SGLPaintContextInterface::GetPaintContextObject( void ) const
{
	return	NULL ;
}
#endif



//////////////////////////////////////////////////////////////////////////////
// 線描画コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDrawContextInterface, ESLObject )

// 直線描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::DrawThinLine
	( int x0, int y0, int x1, int y1,
		uint32_t argb, double z, uint32_t flags )
{
	S2DVector	pts[2] ;
	pts[0].x = (float32_t) x0 ;
	pts[0].y = (float32_t) y0 ;
	pts[1].x = (float32_t) x1 ;
	pts[1].y = (float32_t) y1 ;
	return	DrawThinLines( pts, 1, argb, z, flags ) ;
}

// 楕円輪郭描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::DrawEllipse
	( float32_t xCenter, float32_t yCenter,
		float32_t rWidth, float32_t rHeight,
		uint32_t argb, double z, uint32_t flags )
{
	int	nDiv = eslRoundR32ToInt( (float32_t) ((rWidth + rHeight) * PI * 0.125) ) & ~0x03 ;
	if ( nDiv < 32 )
	{
		nDiv = 32 ;
	}
	else if ( nDiv > 90 )
	{
		nDiv = 90 ;
	}
	SArray<S2DVector>	vertices ;
	vertices.SetLength( nDiv + 1 ) ;
	S2DVector *	pvVertices = vertices.GetArray() ;
	double	radStep = SSystem::PI * 2.0 / nDiv ;
	double	radPhase = radStep * 0.5 ;
	for ( int i = 0; i <= nDiv; i ++ )
	{
		double	rad = radStep * i + radPhase ;
		pvVertices[i].x = (float32_t) (xCenter + rWidth * cos( rad )) ;
		pvVertices[i].y = (float32_t) (yCenter + rHeight * sin( rad )) ;
	}
	SGLError	err = DrawThinLines( pvVertices, nDiv, argb, z, flags ) ;
	vertices.FinishArray() ;
	return	err ;
}

// 円弧輪郭描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::DrawArc
	( float32_t xCenter, float32_t yCenter,
		float32_t rWidth, float32_t rHeight,
		float32_t radFirst, float32_t radEnd,
		uint32_t argb, double z, uint32_t flags )
{
	if ( radEnd < radFirst )
	{
		float32_t	t = radFirst ;
		radFirst = radEnd ;
		radEnd = t ;
	}
	int	nDiv = eslRoundR32ToInt
		( (float32_t) ((rWidth + rHeight)
						* 0.25 * (radEnd - radFirst) / SSystem::PI) ) & ~0x03 ;
	if ( nDiv < 12 )
	{
		nDiv = 12 ;
	}
	else if ( nDiv > 90 )
	{
		nDiv = 90 ;
	}
	SArray<S2DVector>	vertices ;
	vertices.SetLength( nDiv + 3 ) ;
	S2DVector *	pvVertices = vertices.GetArray() ;
	double	radStep = (radEnd - radFirst) / nDiv ;
	for ( int i = 0; i <= nDiv; i ++ )
	{
		double	rad = radFirst + radStep * i ;
		pvVertices[i + 1].x = (float32_t) (xCenter + rWidth * cos( rad )) ;
		pvVertices[i + 1].y = (float32_t) (yCenter + rHeight * sin( rad )) ;
	}
	pvVertices[0].x = xCenter ;
	pvVertices[0].y = yCenter ;
	pvVertices[nDiv + 2].x = xCenter ;
	pvVertices[nDiv + 2].y = yCenter ;
	SGLError	err = DrawThinLines( pvVertices, nDiv + 2, argb, z, flags ) ;
	vertices.FinishArray() ;
	return	err ;
}

// ベジェ曲線描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::DrawBezier
	( const S2DVector * pPoints, size_t nPoints,
		uint32_t argb, double z, uint32_t flags )
{
	if ( nPoints < 4 )
	{
		return	sglErrFailed ;
	}
	SGLBezierCurves<S2DVector>	bezier ;
	bezier.AddArray( pPoints, nPoints ) ;
	//
	SArray<S2DVector>	vertices ;
	vertices.Add( pPoints[0] ) ;
	//
	size_t	nBeziers = (nPoints - 1) / 3 ;
	for ( size_t i = 0; i < nBeziers; i ++ )
	{
		double	r = 0.0 ;
		size_t	k = i * 3 ;
		for ( size_t j = 0; j < 3; j ++ )
		{
			r += (pPoints[k + j] - pPoints[k + j + 1]).Absolute() ;
		}
		//
		int	nDiv = eslRoundR32ToInt( (float32_t) (r * 0.25) ) ;
		if ( nDiv < 8 )
		{
			nDiv = 8 ;
		}
		S2DVector	p ;
		double	td = 1.0 / nDiv ;
		k = vertices.GetLength() ;
		vertices.SetLimit( k + nDiv ) ;
		for ( int d = 1; d <= nDiv; d ++ )
		{
			vertices.Add( bezier.PointAt( p, d * td, i ) ) ;
		}
	}
	return	DrawThinLines
		( vertices.GetConstArray(),
				vertices.GetLength() - 1, argb, z, flags ) ;
}

// 矩形塗りつぶし
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::FillRectangle
	( int x, int y, int width, int height,
				uint32_t argb, double z, uint32_t flags )
{
	S2DVector	vRect[4] ;
	vRect[0].x = (float32_t) x ;
	vRect[0].y = (float32_t) y ;
	vRect[1].x = (float32_t) (x + width) ;
	vRect[1].y = vRect[0].y ;
	vRect[2].x = vRect[1].x ;
	vRect[2].y = (float32_t) (y + height) ;
	vRect[3].x = vRect[0].x ;
	vRect[3].y = vRect[2].y ;
	//
	return	FillPolygon( &(vRect[0]), 4, argb, z, flags ) ;
}

// 楕円塗りつぶし
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::FillEllipse
	( float32_t xCenter, float32_t yCenter,
		float32_t rWidth, float32_t rHeight,
		uint32_t argb, double z, uint32_t flags )
{
	int	nDiv = eslRoundR32ToInt( (float32_t) ((rWidth + rHeight) * PI * 0.25) ) & ~0x03 ;
	if ( nDiv < 12 )
	{
		nDiv = 12 ;
	}
	SArray<S2DVector>	vertices ;
	vertices.SetLength( nDiv + 1 ) ;
	S2DVector *	pvVertices = vertices.GetArray() ;
	double	radStep = SSystem::PI * 2.0 / nDiv ;
	double	radPhase = radStep * 0.5 ;
	for ( int i = 0; i < nDiv; i ++ )
	{
		double	rad = radStep * i + radPhase ;
		pvVertices[i].x = (float32_t) (xCenter + rWidth * cos( rad )) ;
		pvVertices[i].y = (float32_t) (yCenter + rHeight * sin( rad )) ;
	}
	SGLError	err = FillPolygon( pvVertices, nDiv, argb, z, flags ) ;
	vertices.FinishArray() ;
	return	err ;
}

// 円弧塗りつぶし
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDrawContextInterface::FillArc
	( float32_t xCenter, float32_t yCenter,
		float32_t rWidth, float32_t rHeight,
		float32_t radFirst, float32_t radEnd,
		uint32_t argb, double z, uint32_t flags )
{
	if ( radEnd < radFirst )
	{
		float32_t	t = radFirst ;
		radFirst = radEnd ;
		radEnd = t ;
	}
	SArray<S2DVector>	vertices ;
	for ( double rad = radFirst; rad < radEnd; rad += SSystem::PI )
	{
		double	radNext = rad + SSystem::PI ;
		if ( radNext >= radEnd )
		{
			radNext = radEnd ;
		}
		int	nDiv = eslRoundR32ToInt
			( (float32_t) ((rWidth + rHeight)
							* 0.5 * (radNext - rad) / SSystem::PI) ) ;
		if ( nDiv < 6 )
		{
			nDiv = 6 ;
		}
		else if ( nDiv > 180 )
		{
			nDiv = 180 ;
		}
		vertices.SetLength( nDiv + 2 ) ;
		S2DVector *	pvVertices = vertices.GetArray() ;
		double	radStep = (radNext - rad) / nDiv ;
		for ( int i = 0; i <= nDiv; i ++ )
		{
			double	r = rad + radStep * i ;
			pvVertices[i].x = (float32_t) (xCenter + rWidth * cos( r )) ;
			pvVertices[i].y = (float32_t) (yCenter + rHeight * sin( r )) ;
		}
		pvVertices[nDiv + 1].x = xCenter ;
		pvVertices[nDiv + 1].y = yCenter ;
		FillPolygon( pvVertices, nDiv + 2, argb, z, flags ) ;
		vertices.FinishArray() ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// PaintContext - SGLPaintContextInterface インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLPaintContext, SGLPaintContextInterface )
#else
ESL_IMPLEMENT_CLASS_INFO_CAST( SakuraGL::SGLPaintContext, SGLPaintContextInterface, m_paint )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintContext::SGLPaintContext( void )
{
#if	defined(__COTOPHA__)
	m_paint = new PaintContext ;
#else
	m_paint = new SGLPaintBuffer ;
#endif
	m_flagOwner = true ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

SGLPaintContext::SGLPaintContext( PaintContext * paint, bool flagOwner )
{
	m_paint = paint ;
	m_flagOwner = flagOwner ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintContext::~SGLPaintContext( void )
{
	if ( m_flagOwner )
	{
		delete	m_paint ;
		m_paint = NULL ;
		m_flagOwner = false ;
	}
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLPaintContext::AttachPaintContext( PaintContext * paint, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_paint ;
		m_paint = NULL ;
		m_flagOwner = false ;
	}
	m_paint = paint ;
	m_flagOwner = flagOwner ;
	//
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLPaintContext::GetTargetImage( void )
{
	if ( m_pTarget != NULL )
	{
		return	m_pTarget ;
	}
	ESLAssert( m_paint != NULL ) ;
	m_imgTarget.SetImageObject( m_paint->GetTargetImage(), false ) ;
	return	&m_imgTarget ;
}

SGLImageObject * SGLPaintContext::GetTargetZBuffer( void )
{
	if ( m_pZBuffer != NULL )
	{
		return	m_pZBuffer ;
	}
	ESLAssert( m_paint != NULL ) ;
	m_imgZBuffer.SetImageObject( m_paint->GetTargetZBuffer(), false ) ;
	return	&m_imgZBuffer ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::GetViewPort( SGLImageRect & rctView ) const
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->GetViewPort( rctView ) ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pTarget = pImage ;
	m_pZBuffer = pZBuffer ;
	//
	Image *	imgTarget = NULL ;
	Image *	imgZBuffer = NULL ;
	if ( pImage != NULL )
	{
		imgTarget = pImage->GetImageObject() ;
		if ( imgTarget == NULL )
		{
			Trace( "failed to GetImageObject for target "
					"at SGLPaintContext::AttachTargetImage\n" ) ;
		}
	}
	if ( pZBuffer != NULL )
	{
		imgZBuffer = pZBuffer->GetImageObject() ;
		if ( imgZBuffer == NULL )
		{
			Trace( "failed to GetImageObject for z buffer "
					"at SGLPaintContext::AttachTargetImage\n" ) ;
		}
	}
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->AttachTargetImage( imgTarget, imgZBuffer, pView ) ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::DetachTargetImage( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	//
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->DetachTargetImage() ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->AppendTransformation( af, nTransparency ) ;
}

SGLError SGLPaintContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->SetTransformation( af, nTransparency ) ;
}

SGLError SGLPaintContext::CurrentAffine( SGLAffine & af )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->CurrentAffine( af ) ;
}

unsigned int SGLPaintContext::CurrentTransparency( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->CurrentTransparency() ;
}

SGLError SGLPaintContext::PushTransformation( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->PushTransformation() ;
}

SGLError SGLPaintContext::PopTransformation( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->PopTransformation() ;
}

SGLError SGLPaintContext::ResetTransformation( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->ResetTransformation() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void SGLPaintContext::SetPaintFlags( int64_t nFlags )
{
	ESLAssert( m_paint != NULL ) ;
	m_paint->SetPaintFlags( nFlags ) ;
}

int64_t SGLPaintContext::GetPaintFlags( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->GetPaintFlags() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::FillClearTarget( uint32_t argb, int64_t flags )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->FillClearTarget( argb, flags ) ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::FillRectangle
	( int x, int y, int width, int height,
				uint32_t argb, double z, uint32_t flags )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->FillRectangle( x, y, width, height, argb, z, flags ) ;
}

SGLError SGLPaintContext::FillPolygon
	( const S2DVector * vertices, size_t count,
				uint32_t argb, double z, uint32_t flags )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->FillPolygon( vertices, count, argb, z, flags ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	Image *	imgSrcImage = NULL ;
	if ( pSrcImage != NULL )
	{
		imgSrcImage = pSrcImage->GetImageObject() ;
		if ( imgSrcImage == NULL )
		{
			Trace( "failed to GetImageObject"
					"at SGLPaintContext::DrawImage\n" ) ;
		}
	}
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->DrawImage( ppPaint, imgSrcImage, pSrcClip ) ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	Image *	imgSrcImage = NULL ;
	if ( pSrcImage != NULL )
	{
		imgSrcImage = pSrcImage->GetImageObject() ;
		if ( imgSrcImage == NULL )
		{
			Trace( "failed to GetImageObject"
					"at SGLPaintContext::DrawImage\n" ) ;
		}
	}
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->DrawMesh
		( pDstMesh, pSrcMesh, widthMesh, heightMesh,
						ppPaint, imgSrcImage, pSrcClip ) ;
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->DrawMultiImages
		( nCount, pParams, ppSrcImages, pSrcClips ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintContext::Flush( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->Flush() ;
}

SGLError SGLPaintContext::Finish( void )
{
	ESLAssert( m_paint != NULL ) ;
	return	m_paint->Finish() ;
}

#if	defined(__COTOPHA__)
// PaintContext 取得
//////////////////////////////////////////////////////////////////////////////
PaintContext * SGLPaintContext::GetPaintContextObject( void ) const
{
	return	m_paint ;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// 2D 描画コンテキスト基底
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLPaintParameterContext, SGLPaintContextInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintParameterContext::SGLPaintParameterContext( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	m_pTransformation = NULL ;
	m_flagsDefPaint = paintSmoothStretch ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintParameterContext::~SGLPaintParameterContext( void )
{
	SGLPaintParameterContext::ResetTransformation() ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLPaintParameterContext::GetTargetImage( void )
{
	return	m_pTarget ;
}

SGLImageObject * SGLPaintParameterContext::GetTargetZBuffer( void )
{
	return	m_pZBuffer ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::GetViewPort( SGLImageRect & rctView ) const
{
	rctView = m_rctView ;
	return	sglErrSuccess ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::AttachTargetImage
	( SGLImageObject * pImage,
		SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_pTarget = pImage ;
	m_pZBuffer = pZBuffer ;
	//
	if ( pImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageInfo	imginf ;
	pImage->GetImageInfo( imginf ) ;
	m_rctView = imginf.GetImageRect() ;
	//
	if ( pView != NULL )
	{
		m_rctView = SGLRect( m_rctView ) & SGLRect( *pView ) ;
		if ( m_rctView.IsEmpty() )
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::DetachTargetImage( void )
{
	m_pTarget = NULL ;
	m_pZBuffer = NULL ;
	return	sglErrSuccess ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	if ( pTransform != NULL )
	{
		pTransform->afTransform = pTransform->afTransform * af ;
		pTransform->nTransparency =
			0x100 - (0x100 - nTransparency)
						* (0x100 - pTransform->nTransparency) / 0x100 ;
	}
	else
	{
		pTransform = new TransformationList ;
		pTransform->pPrev = NULL ;
		m_pTransformation = pTransform ;
		pTransform->afTransform = af ;
		pTransform->nTransparency = nTransparency ;
	}
	return	sglErrSuccess ;
}

SGLError SGLPaintParameterContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	TransformationList *	pTransform = m_pTransformation ;
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	if ( pTransform == NULL )
	{
		pTransform = new TransformationList ;
		pTransform->pPrev = NULL ;
		m_pTransformation = pTransform ;
	}
	pTransform->afTransform = af ;
	pTransform->nTransparency = nTransparency ;
	return	sglErrSuccess ;
}

SGLError SGLPaintParameterContext::CurrentAffine( SGLAffine & af )
{
	if ( GetTransformation( af ) )
	{
		return	sglErrSuccess ;
	}
	af.a11 = 1.0f ;
	af.a12 = 0.0f ;
	af.a13 = 0.0f ;
	af.a21 = 0.0f ;
	af.a22 = 1.0f ;
	af.a23 = 0.0f ;
	return	sglErrFailed ;
}

unsigned int SGLPaintParameterContext::CurrentTransparency( void )
{
	return	GetTransparencyOf( 0 ) ;
}

SGLError SGLPaintParameterContext::PushTransformation( void )
{
	TransformationList *	pTransform = new TransformationList ;
	pTransform->pPrev = m_pTransformation ;
	//
	if ( m_pTransformation != NULL )
	{
		pTransform->afTransform = m_pTransformation->afTransform ;
		pTransform->nTransparency = m_pTransformation->nTransparency ;
	}
	m_pTransformation = pTransform ;
	return	sglErrSuccess ;
}

SGLError SGLPaintParameterContext::PopTransformation( void )
{
	if ( m_pTransformation == NULL )
	{
		return	sglErrFailed ;
	}
	TransformationList *	pTransform = m_pTransformation ;
	m_pTransformation = pTransform->pPrev ;
	delete	pTransform ;
	return	sglErrSuccess ;
}

SGLError SGLPaintParameterContext::ResetTransformation( void )
{
	while ( m_pTransformation != NULL )
	{
		TransformationList *	pTransform = m_pTransformation ;
		m_pTransformation = pTransform->pPrev ;
		delete	pTransform ;
	}
	m_pTransformation = NULL ;
	return	sglErrSuccess ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void SGLPaintParameterContext::SetPaintFlags( int64_t nFlags )
{
	m_flagsDefPaint = nFlags ;
}

int64_t SGLPaintParameterContext::GetPaintFlags( void )
{
	return	m_flagsDefPaint ;
}

// ２Ｄメッシュ描画（DrawImage 呼び出し）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::DrawMesh
	( const S2DVector * pDstMesh, const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh, const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	const size_t	countWidth = widthMesh + 1 ;
	S2DVector		vVerticesDst[4] ;
	S2DVector		vVerticesSrc[4] ;
	SGLAffine		afMeshHalf ;
	SGLImageInfo	infSrcImage ;
	SGLImageRect	rctSrcImage ;
	SGLPaintParam	ppPaintMesh = ppPaint ;
	S2DVector		vSrcMeshSize ;
	//
	if ( pSrcImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	if ( pSrcClip != NULL )
	{
		rctSrcImage = *pSrcClip ;
	}
	else
	{
		pSrcImage->GetImageInfo( infSrcImage ) ;
		rctSrcImage = infSrcImage.GetImageRect() ;
	}
	vSrcMeshSize.x = (float32_t) rctSrcImage.w / (float32_t) widthMesh ;
	vSrcMeshSize.y = (float32_t) rctSrcImage.h / (float32_t) heightMesh ;
	//
	ppPaintMesh.nFlags |= paintDelayable | paintOrderNoCare | paintPolygonShaped ;
	ppPaintMesh.pAffine = &afMeshHalf ;
	ppPaintMesh.pVertices = &vVerticesDst[0] ;
	ppPaintMesh.countVertex = 3 ;
	//
	for ( size_t y = 0; y < heightMesh; y ++ )
	{
		for ( size_t x = 0; x < widthMesh; x ++ )
		{
			vVerticesDst[0] = pDstMesh[x] ;
			vVerticesDst[1] = pDstMesh[x + 1] ;
			vVerticesDst[2] = pDstMesh[x + countWidth] ;
			vVerticesDst[3] = pDstMesh[x + countWidth + 1] ;
			//
			if ( pSrcMesh != NULL )
			{
				vVerticesSrc[0] = pSrcMesh[x] ;
				vVerticesSrc[1] = pSrcMesh[x + 1] ;
				vVerticesSrc[2] = pSrcMesh[x + countWidth] ;
				vVerticesSrc[3] = pSrcMesh[x + countWidth + 1] ;
			}
			else
			{
				vVerticesSrc[0].x =
					(float32_t) rctSrcImage.x
									+ vSrcMeshSize.x * (float32_t) x ;
				vVerticesSrc[0].y =
					(float32_t) rctSrcImage.y
									+ vSrcMeshSize.y * (float32_t) y ;
				vVerticesSrc[1].x = vVerticesSrc[0].x + vSrcMeshSize.x ;
				vVerticesSrc[1].y = vVerticesSrc[0].y ;
				vVerticesSrc[2].x = vVerticesSrc[0].x ;
				vVerticesSrc[2].y = vVerticesSrc[0].y + vSrcMeshSize.y ;
				vVerticesSrc[3].x = vVerticesSrc[1].x ;
				vVerticesSrc[3].y = vVerticesSrc[2].y ;
			}
			ppPaintMesh.pVertices = &vVerticesSrc[0] ;
			afMeshHalf.MappingOf( &vVerticesDst[0], &vVerticesSrc[0] ) ;
			DrawImage( ppPaintMesh, pSrcImage ) ;
			//
			if ( (y == heightMesh - 1) & (x == widthMesh - 1) )
			{
				ppPaintMesh.nFlags &= ~(paintDelayable | paintOrderNoCare) ;
			}
			ppPaintMesh.pVertices = &vVerticesSrc[1] ;
			afMeshHalf.MappingOf( &vVerticesDst[1], &vVerticesSrc[1] ) ;
			DrawImage( ppPaintMesh, pSrcImage ) ;
		}
		pDstMesh += countWidth ;
		if ( pSrcMesh != NULL )
		{
			pSrcMesh += countWidth ;
		}
	}
	return	sglErrSuccess ;
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	SGLError	err = sglErrSuccess ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		err = DrawImage
			( pParams[i], ppSrcImages[i],
				((pSrcClips != NULL) ? pSrcClips + i : NULL) ) ;
	}
	return	err ;
}

// 画像変形描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintParameterContext::DrawTrianglePolygon
	( const S2DVector * pDstVertices,
		const S2DVector * pSrcVertices,
		const SGLPaintParam & ppPaint, SGLImageObject * pSrcImage )
{
	SGLPaintParam	ppPaintPoly = ppPaint ;
	SGLAffine		afMeshHalf ;
	afMeshHalf.MappingOf( pDstVertices, pSrcVertices ) ;
	//
	ppPaintPoly.nFlags |= paintPolygonShaped ;
	ppPaintPoly.pAffine = &afMeshHalf ;
	ppPaintPoly.pVertices = pDstVertices ;
	ppPaintPoly.countVertex = 3 ;
	//
	return	DrawImage( ppPaintPoly, pSrcImage ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像描画リスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDrawImageParamList, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDrawImageParamList::SGLDrawImageParamList( void )
	: m_affine( 1.0f, 0.0f, 0.0f,  0.0, 1.0f, 0.0f ), m_nTransparency( 0 )
{
}

// 変形
//////////////////////////////////////////////////////////////////////////////
const SGLAffine& SGLDrawImageParamList::GetAffine( void ) const
{
	return	m_affine ;
}

void SGLDrawImageParamList::SetAffine( const SGLAffine& affine )
{
	m_affine = affine ;
}

void SGLDrawImageParamList::AppendAffine( const SGLAffine& affine )
{
	m_affine = m_affine * affine ;
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDrawImageParamList::GetTransparency( void ) const
{
	return	m_nTransparency ;
}

void SGLDrawImageParamList::SetTransparency( uint32_t nTransparency )
{
	m_nTransparency = (nTransparency < 0x100) ? nTransparency : 0x100 ;
}

void SGLDrawImageParamList::AppendTransparency( uint32_t nTransparency )
{
	if ( nTransparency < 0x100 )
	{
		m_nTransparency =
			0x100 - (0x100 - m_nTransparency)
						* (0x100 - nTransparency) / 0x100 ;
	}
	else
	{
		m_nTransparency = 0x100 ;
	}
}

// 追加
//////////////////////////////////////////////////////////////////////////////
void SGLDrawImageParamList::AddDrawParam
	( const SGLPaintParam& param,
		SGLImageObject * pImage, const SGLImageRect * pSrcRect )
{
	if ( pImage == NULL )
	{
		return ;
	}
	SGLPaintParam	ppTemp = param ;
	ppTemp.nTransparency =
		0x100 - ((0x100 - ppTemp.nTransparency)
				* (0x100 - m_nTransparency) / 0x100) ;
	if ( (ppTemp.pAffine != NULL) || m_affine.IsRotation() )
	{
		S2DVector	vOffset( (float32_t) ppTemp.ptPaint.x,
								(float32_t) ppTemp.ptPaint.y ) ;
		if ( ppTemp.nFlags & paintFixedPosition )
		{
			vOffset *= 1.0f / 65536.0f ;
		}
		//
		SGLAffine	affTemp( 1.0f, 0.0f, vOffset.x,  0.0f, 1.0f, vOffset.y ) ;
		if ( ppTemp.pAffine != NULL )
		{
			affTemp = affTemp * *(ppTemp.pAffine) ;
		}
		affTemp = m_affine * affTemp ;
		ppTemp.ptPaint.x = 0 ;
		ppTemp.ptPaint.y = 0 ;
		ppTemp.pAffine = AddAffine( affTemp ) ;
	}
	else
	{
		if ( ppTemp.nFlags & paintFixedPosition )
		{
			ppTemp.ptPaint.x += (int32_t) esl_roundfi( m_affine.a13 * 65536.0f ) ;
			ppTemp.ptPaint.y += (int32_t) esl_roundfi( m_affine.a23 * 65536.0f ) ;
		}
		else
		{
			ppTemp.ptPaint.x += (int32_t) esl_roundfi( m_affine.a13 ) ;
			ppTemp.ptPaint.y += (int32_t) esl_roundfi( m_affine.a23 ) ;
		}
	}
	if ( (ppTemp.nFlags & paintPolygonShaped)
		&& (ppTemp.pVertices != NULL) && (ppTemp.countVertex > 0) )
	{
		ppTemp.pVertices = AddVertices( ppTemp.pVertices, ppTemp.countVertex ) ;
	}
	else
	{
		ppTemp.pVertices = NULL ;
		ppTemp.countVertex = 0 ;
	}
	m_aParams.Add( ppTemp ) ;
	m_pImages.Add( pImage ) ;
	//
	if ( pSrcRect != NULL )
	{
		m_pRects.Add( *pSrcRect ) ;
	}
	else
	{
		SGLSize			sizeImage = pImage->GetImageSize() ;
		SGLImageRect	rect( 0, 0, sizeImage.w, sizeImage.h ) ;
		m_pRects.Add( rect ) ;
	}
	ESLAssert( m_aParams.GetLength() == m_pImages.GetLength() ) ;
	ESLAssert( m_aParams.GetLength() == m_pRects.GetLength() ) ;
}

SGLAffine * SGLDrawImageParamList::AddAffine( const SGLAffine& affine )
{
	const SGLAffine *	pOldPointer = m_aAffines.GetConstArray() ;
	size_t	iAdded = m_aAffines.Add( affine ) ;
	//
	if ( m_aAffines.GetConstArray() != pOldPointer )
	{
		const SGLAffine *	pNewPointer = m_aAffines.GetConstArray() ;
		SGLPaintParam *		pParams = m_aParams.GetArray() ;
		const size_t		nCount = m_aParams.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLPaintParam&	pp = pParams[i] ;
			if ( pp.pAffine != NULL )
			{
				pp.pAffine = pNewPointer + (pp.pAffine - pOldPointer) ;
			}
		}
	}
	return	m_aAffines.GetAt( iAdded ) ;
}

S2DVector * SGLDrawImageParamList::AddVertices
				( const S2DVector * pVertices, size_t nCount )
{
	const S2DVector *	pOldPointer = m_aVertices.GetConstArray() ;
	size_t	iAdded = m_aVertices.AddArray( pVertices, nCount ) ;
	//
	if ( m_aVertices.GetConstArray() != pOldPointer )
	{
		const S2DVector *	pNewPointer = m_aVertices.GetConstArray() ;
		SGLPaintParam *		pParams = m_aParams.GetArray() ;
		const size_t		nCount = m_aParams.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLPaintParam&	pp = pParams[i] ;
			if ( pp.pVertices != NULL )
			{
				pp.pVertices = pNewPointer + (pp.pVertices - pOldPointer) ;
			}
		}
	}
	return	m_aVertices.GetAt( iAdded ) ;
}

// クリア
//////////////////////////////////////////////////////////////////////////////
void SGLDrawImageParamList::ClearList( void )
{
	m_aParams.RemoveAll() ;
	m_pImages.RemoveAll() ;
	m_pRects.RemoveAll() ;
	m_aAffines.RemoveAll() ;
	m_aVertices.RemoveAll() ;
}

// リストが空か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDrawImageParamList::IsEmptyList( void ) const
{
	return	(m_aParams.GetLength() == 0) ;
}

// 描画実行
//////////////////////////////////////////////////////////////////////////////
void SGLDrawImageParamList::Draw( SGLPaintContextInterface& paint ) const
{
	SGLImageObject *	pRefImage = NULL ;
	size_t	iLast = 0 ;
	//
	ESLAssert( m_aParams.GetLength() == m_pImages.GetLength() ) ;
	ESLAssert( m_aParams.GetLength() == m_pRects.GetLength() ) ;
	//
	for ( size_t i = 0; i < m_pImages.GetLength(); i ++ )
	{
		SGLImageObject *	pImage = m_pImages.GetAt(i) ;
		ESLAssert( pImage != NULL ) ;
		//
		SGLImageRect		rectRef ;
		SGLImageObject *	pRefTemp = pImage->GetImageReference( rectRef ) ;
		if ( pRefImage != pRefTemp )
		{
			if ( i - iLast > 0 )
			{
				paint.DrawMultiImages
					( i - iLast,
						m_aParams.GetConstArray() + iLast,
						m_pImages.GetConstArray() + iLast,
						m_pRects.GetConstArray() + iLast ) ;
				iLast = i ;
			}
			pRefImage = pRefTemp ;
		}
	}
	if ( m_pImages.GetLength() - iLast > 0 )
	{
		paint.DrawMultiImages
			( m_pImages.GetLength() - iLast,
				m_aParams.GetConstArray() + iLast,
				m_pImages.GetConstArray() + iLast,
				m_pRects.GetConstArray() + iLast ) ;
	}
}

void SGLDrawImageParamList::DrawToList( SGLDrawImageParamList& dpiList ) const
{
	for ( size_t i = 0; i < m_pImages.GetLength(); i ++ )
	{
		SGLPaintParam *		pParam = m_aParams.GetAt(i) ;
		SGLImageObject *	pImage = m_pImages.GetAt(i) ;
		SGLImageRect *		pRect = m_pRects.GetAt(i) ;
		ESLAssert( pParam != NULL ) ;
		ESLAssert( pImage != NULL ) ;
		//
		dpiList.AddDrawParam( *pParam, pImage, pRect ) ;
	}
}

size_t SGLDrawImageParamList::CountDraw( void ) const
{
	SGLImageObject *	pRefImage = NULL ;
	size_t	iLast = 0 ;
	//
	ESLAssert( m_aParams.GetLength() == m_pImages.GetLength() ) ;
	ESLAssert( m_aParams.GetLength() == m_pRects.GetLength() ) ;
	//
	size_t	nDrawCount = 0 ;
	for ( size_t i = 0; i < m_pImages.GetLength(); i ++ )
	{
		SGLImageObject *	pImage = m_pImages.GetAt(i) ;
		ESLAssert( pImage != NULL ) ;
		//
		SGLImageRect		rectRef ;
		SGLImageObject *	pRefTemp = pImage->GetImageReference( rectRef ) ;
		if ( pRefImage != pRefTemp )
		{
			if ( i - iLast > 0 )
			{
				nDrawCount ++ ;
				iLast = i ;
			}
			pRefImage = pRefTemp ;
		}
	}
	if ( m_pImages.GetLength() - iLast > 0 )
	{
		nDrawCount ++ ;
	}
	return	nDrawCount ;
}

// 描画実行（3次元空間上へ）
//////////////////////////////////////////////////////////////////////////////
void SGLDrawImageParamList::RenderAs3D
	( S3DRenderBufferInterface& render,
		uint64_t flagsExclusion,
		SGLDrawImageParamList::StereoViewIndex sviView )
{
	SGLImageObject *	pRefImage = nullptr ;
	size_t	iLast = 0 ;
	//
	ESLAssert( m_aParams.GetLength() == m_pImages.GetLength() ) ;
	ESLAssert( m_aParams.GetLength() == m_pRects.GetLength() ) ;
	//
	for ( size_t i = 0; i < m_pImages.GetLength(); i ++ )
	{
		SGLImageObject *	pImage = m_pImages.GetAt(i) ;
		ESLAssert( pImage != nullptr ) ;
		//
		SGLImageRect		rectRef ;
		SGLImageObject *	pRefTemp = pImage->GetImageReference( rectRef ) ;
		if ( pRefImage != pRefTemp )
		{
			if ( i - iLast > 0 )
			{
				RenderMultiImages
					( render, flagsExclusion, sviView,
						i - iLast,
						m_aParams.GetConstArray() + iLast,
						m_pImages.GetConstArray() + iLast,
						m_pRects.GetConstArray() + iLast ) ;
				iLast = i ;
			}
			pRefImage = pRefTemp ;
		}
	}
	if ( m_pImages.GetLength() - iLast > 0 )
	{
		RenderMultiImages
			( render, flagsExclusion, sviView,
				m_pImages.GetLength() - iLast,
				m_aParams.GetConstArray() + iLast,
				m_pImages.GetConstArray() + iLast,
				m_pRects.GetConstArray() + iLast ) ;
	}
}

void SGLDrawImageParamList::RenderMultiImages
	( S3DRenderBufferInterface& render,
		uint64_t flagsExclusion,
		StereoViewIndex sviView,
		size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	if ( (nCount == 0)
		|| (pParams == nullptr)
		|| (ppSrcImages == nullptr)
		|| (ppSrcImages[0] == nullptr)
		|| (pSrcClips == nullptr) )
	{
		return ;
	}
	SGLImageRect		rectRef0 ;
	SGLImageObject *	pSrcImage =
		ppSrcImages[0]->GetImageReference( rectRef0, 0 ) ;
	if ( pSrcImage == nullptr )
	{
		return ;
	}
	const uint32_t	format0 = ppSrcImages[0]->GetImageFormat() ;
	if ( format0 & formatImageFlagSideBySide )
	{
		if ( (sviView == stereoViewRight)
			|| (sviView == stereoViewLeft) )
		{
			rectRef0.w /= 2 ;
			rectRef0.x += rectRef0.w * (int32_t) sviView ;
		}
	}
	bool			flagCombinable ;
	S3DMaterial *	pMaterial =
		S3DRenderBufferedContext::GetNoShadeMaterialOf
			( flagCombinable, pSrcImage, pParams[0].nFlags, shadingVertexAlpha ) ;
	if ( (pMaterial == nullptr)
		|| (pMaterial->m_attrSurface.flagsShading & flagsExclusion) )
	{
		return ;
	}
	//
	// 画像参照矩形取得
	//
	m_bufImageRect.SetLength( nCount ) ;
	m_bufImageRect.SetAt( 0, rectRef0 ) ;
	for ( size_t i = 1; i < nCount; i ++ )
	{
		SGLImageRect	rectRef = rectRef0 ;
		ppSrcImages[i]->GetReferenceRectOfAtlas( rectRef ) ;
		m_bufImageRect.SetAt( i, rectRef ) ;
	}
	//
	// 頂点
	//
	size_t	nVertexCount = 0 ;
	size_t	nPolyCount = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pParams[i].nFlags & paintPolygonShaped )
		{
			if ( (pParams[i].pVertices != NULL)
				&& (pParams[i].countVertex >= 3) )
			{
				nVertexCount += pParams[i].countVertex ;
				nPolyCount += pParams[i].countVertex - 2 ;
			}
		}
		else
		{
			nVertexCount += 4 ;
			nPolyCount += 2 ;
		}
	}
	//
	const SGLImageRect *	pRefRect = m_bufImageRect.GetConstArray() ;
	S3DVector4 *	pvBufVertex = m_bufVertex.GetArray( nVertexCount ) ;
	S2DVector *		pvBufUV = m_bufUVMap.GetArray( nVertexCount ) ;
	S3DColor *		pBufColor = m_bufColor.GetArray( nVertexCount ) ;
	uint32_t *		pBufIndex = m_bufIndexedList.GetArray( nPolyCount * 3 ) ;
	uint32_t		iVertex = 0 ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SGLPaintParam&	ppPaint = pParams[i] ;
		const SGLImageRect		rectSrcRef = pRefRect[i] ;
		SGLSize					sizeImage = rectSrcRef.GetSize() ;
		if ( pSrcClips != NULL )
		{
			sizeImage = pSrcClips[i].GetSize() ;
		}
		//
		// UV 座標
		//
		size_t	countVertex = 4 ;
		if ( ppPaint.nFlags & paintPolygonShaped )
		{
			if ( (ppPaint.pVertices == NULL)
				|| (ppPaint.countVertex < 3) )
			{
				continue ;
			}
			countVertex = ppPaint.countVertex ;
			//
			const S2DVector *	pvSrcUV = ppPaint.pVertices ;
			for ( size_t j = 0; j < countVertex; j ++ )
			{
				pvBufUV[j].x = pvSrcUV[j].x ;
				pvBufUV[j].y = pvSrcUV[j].y ;
			}
		}
		else
		{
			pvBufUV[0].x = 0.0f ;
			pvBufUV[0].y = 0.0f ;
			pvBufUV[1].x = 0.0f ;
			pvBufUV[1].y = (float32_t) sizeImage.h ;
			pvBufUV[2].x = (float32_t) sizeImage.w ;
			pvBufUV[2].y = (float32_t) sizeImage.h ;
			pvBufUV[3].x = (float32_t) sizeImage.w ;
			pvBufUV[3].y = 0.0f ;
		}
		//
		// 頂点座標
		//
		SGLAffine	affine( 1.0f, 0.0f, (float32_t) ppPaint.ptPaint.x,
							0.0f, 1.0f, (float32_t) ppPaint.ptPaint.y ) ;
		if ( ppPaint.nFlags & paintFixedPosition )
		{
			affine.a13 *= 1.0f / 65536.0f ;
			affine.a23 *= 1.0f / 65536.0f ;
		}
		if ( ppPaint.pAffine != NULL )
		{
			affine = affine * *(ppPaint.pAffine) ;
		}
		float32_t	z = 0.0f ;
		if ( ppPaint.nFlags & paintWithZOrder )
		{
			z = ppPaint.zOrder ;
		}
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			S2DVector	v = affine * pvBufUV[j] ;
			pvBufVertex[j].x = v.x ;
			pvBufVertex[j].y = v.y ;
			pvBufVertex[j].z = z ;
			pvBufVertex[j].d = 0.0f ;
		}
		//
		// UV 切り出し補正
		//
		float32_t	dx = (float32_t) rectSrcRef.x ;
		float32_t	dy = (float32_t) rectSrcRef.y ;
		if ( pSrcClips != NULL )
		{
			dx += (float32_t) pSrcClips[i].x ;
			dy += (float32_t) pSrcClips[i].y ;
		}
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			pvBufUV[j].x += dx ;
			pvBufUV[j].y += dy ;
		}
		//
		// 頂点色
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorAdd )
		{
			color.rgbAdd = ppPaint.rgbColorParam ;
		}
		else if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorMul )
		{
			color.rgbMul = ppPaint.rgbColorParam ;
			color.rgbMul.argb.Alpha = 0xFF ;
		}
		if ( ppPaint.nTransparency < 0x100 )
		{
			color.rgbMul.argb.Alpha = (uint32_t) (0xFF - ppPaint.nTransparency) ;
			color.rgbAdd *= (unsigned int) (0x100 - ppPaint.nTransparency) ;
		}
		else
		{
			color.rgbMul.argb.Alpha = 0 ;
		}
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			pBufColor[j] = color ;
		}
		//
		// インデックス
		//
		for ( size_t j = 2, k = 0; j < countVertex; j ++, k += 3 )
		{
			pBufIndex[k] = iVertex ;
			pBufIndex[k+1] = iVertex + (uint32_t) j - 1 ;
			pBufIndex[k+2] = iVertex + (uint32_t) j ;
		}
		//
		pvBufVertex += countVertex ;
		pvBufUV += countVertex ;
		pBufColor += countVertex ;
		pBufIndex += (countVertex - 2) * 3 ;
		iVertex += (uint32_t) countVertex ;
	}
	//
	m_bufVertex.FinishArray() ;
	m_bufUVMap.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndexedList.FinishArray() ;
	//
	render.AddIndexedTriangleList
		( pMaterial,
			RenderContext::renderFenceOrder
				| RenderContext::renderUncombinable,
			nPolyCount, nVertexCount,
			m_bufVertex.GetConstArray(), NULL,
			m_bufUVMap.GetConstArray(),
			m_bufColor.GetConstArray(),
			m_bufIndexedList.GetConstArray() ) ;
}

