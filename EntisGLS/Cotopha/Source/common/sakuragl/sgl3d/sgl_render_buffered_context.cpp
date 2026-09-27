
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl3d_image.h>
#include <sakuragl/sgl3d/sgl_render_buffered_context.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 3D レンダリング・バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRenderBufferedContext,
			S3DRenderParameterContext, SGLDrawContextInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBufferedContext::S3DRenderBufferedContext( void )
{
	m_prbBuffer = NULL ;
	m_pRender = NULL ;
	//
	S3DSurfaceAttribute	sufattr ;
	sufattr.flagsShading = shadingMethodNothing
						| shadingNoShadowObject
						| shadingNoReflectObject
						| shadingVertexAlpha ;
	sufattr.colorBase.rgbAdd.ui32 = 0x000000 ;
	sufattr.colorBase.rgbMul.ui32 = 0xFFFFFF ;
	//
	m_material2DPolygon.SetSurfaceAttribute( sufattr ) ;
	m_material2DPolygon.EnableBackSurfaceAttribute( false ) ;
	//
	sufattr.flagsShading |= shadingNoZBuffer ;
	m_material2DPolygonNZ.SetSurfaceAttribute( sufattr ) ;
	m_material2DPolygonNZ.EnableBackSurfaceAttribute( false ) ;
	//
	sufattr.flagsShading &= ~shadingNoZBuffer ;
	sufattr.flagsShading |= shadingZBufferNoWrite ;
	m_material2DPolygonNWZ.SetSurfaceAttribute( sufattr ) ;
	m_material2DPolygonNWZ.EnableBackSurfaceAttribute( false ) ;
	//
	m_opFillType = fillFlat ;
	//
	m_nDelayFlags = 0 ;
	//
	SetRenderBuffer() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRenderBufferedContext::~S3DRenderBufferedContext( void )
{
}

// 全ビューのバッファをクリア
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::ClearAllViewBuffer( void )
{
	QuickLock() ;
	for ( size_t i = 0; i < countView; i ++ )
	{
		S3DRenderBuffer *	render = m_rbRenderBuffer[i].m_render ;
		if ( render != NULL )
		{
			render->ClearBuffer() ;
		}
		m_rbRenderBuffer[i].m_flagFillClear = false ;
	}
	DeleteAllTemporaryObjects() ;
	QuickUnlock() ;
}

// 全ビューのバッファをレンダリング
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::FlushRenderAllViewBufferTo
	( S3DRenderContextInterface * render, uint64_t flagsExclusion )
{
	S3DRenderBuffer *	pBufAutoView ;
	S3DRenderBuffer *	pBufRightView ;
	S3DRenderBuffer *	pBufLeftView ;
	bool				fRightView = false ;
	bool				fLeftView = false ;
	bool				fStereoView = false ;
	//
	FlushDelayDraw() ;
	//
	QuickLock() ;
	pBufAutoView = m_rbRenderBuffer[0].m_render ;
	pBufRightView = m_rbRenderBuffer[1].m_render ;
	pBufLeftView = m_rbRenderBuffer[2].m_render ;
	pBufAutoView->Flush() ;
	fRightView = (pBufRightView != NULL) && !pBufRightView->IsEmptyBuffer() ;
	fLeftView = (pBufLeftView != NULL) && !pBufLeftView->IsEmptyBuffer() ;
	fStereoView = fRightView | fLeftView ;
	QuickUnlock() ;
	//
	if ( fStereoView || m_fStereoView )
	{
		S3DRenderContextInterface::StereoViewIndex
					sviView = render->CurrentParallaxView() ;
		S3DDMatrix	matCamera ;
		S3DDVector	vCameraPos ;
		render->GetCamera( matCamera, vCameraPos ) ;
		//
		// 右視点描画
		//
		render->SelectParallaxView( stereoViewRight ) ;
		if ( m_rbRenderBuffer[indexAutoView].m_flagFillClear )
		{
			render->FillClearTarget
				( m_rbRenderBuffer[indexAutoView].m_argbFillClear,
					m_rbRenderBuffer[indexAutoView].m_flagsFillClear ) ;
		}
		else if ( m_rbRenderBuffer[indexRightView].m_flagFillClear )
		{
			render->FillClearTarget
				( m_rbRenderBuffer[indexRightView].m_argbFillClear,
					m_rbRenderBuffer[indexRightView].m_flagsFillClear ) ;
			m_rbRenderBuffer[indexRightView].m_flagFillClear = false ;
		}
		if ( pBufAutoView != NULL )
		{
			SetParallaxCameraTo( render, stereoViewRight ) ;
			pBufAutoView->FlushRenderTemporaryBufferTo( render, flagsExclusion ) ;
		}
		if ( pBufRightView != NULL )
		{
			render->SetCamera( m_matCamera, m_vCameraPos ) ;
			pBufRightView->Flush() ;
			pBufRightView->FlushRenderTemporaryBufferTo( render, flagsExclusion ) ;
		}
		//
		// 左視点描画
		//
		render->SelectParallaxView( stereoViewLeft ) ;
		if ( m_rbRenderBuffer[indexAutoView].m_flagFillClear )
		{
			render->FillClearTarget
				( m_rbRenderBuffer[indexAutoView].m_argbFillClear,
					m_rbRenderBuffer[indexAutoView].m_flagsFillClear ) ;
		}
		else if ( m_rbRenderBuffer[indexLeftView].m_flagFillClear )
		{
			render->FillClearTarget
				( m_rbRenderBuffer[indexLeftView].m_argbFillClear,
					m_rbRenderBuffer[indexLeftView].m_flagsFillClear ) ;
			m_rbRenderBuffer[indexLeftView].m_flagFillClear = false ;
		}
		m_rbRenderBuffer[indexAutoView].m_flagFillClear = false ;
		//
		if ( pBufAutoView != NULL )
		{
			SetParallaxCameraTo( render, stereoViewLeft ) ;
			pBufAutoView->FlushRenderTemporaryBufferTo( render, flagsExclusion ) ;
		}
		if ( pBufLeftView != NULL )
		{
			render->SetCamera( m_matCamera, m_vCameraPos ) ;
			pBufLeftView->Flush() ;
			pBufLeftView->FlushRenderTemporaryBufferTo( render, flagsExclusion ) ;
		}
		render->SelectParallaxView( sviView ) ;
		render->SetCamera( matCamera, vCameraPos ) ;
	}
	else
	{
		//
		// 汎用描画
		//
		render->SelectParallaxView( stereoViewAuto ) ;
		if ( m_rbRenderBuffer[indexAutoView].m_flagFillClear )
		{
			render->FillClearTarget
				( m_rbRenderBuffer[indexAutoView].m_argbFillClear,
					m_rbRenderBuffer[indexAutoView].m_flagsFillClear ) ;
			m_rbRenderBuffer[indexAutoView].m_flagFillClear = false ;
		}
		if ( pBufAutoView != NULL )
		{
			pBufAutoView->FlushRenderTemporaryBufferTo( render, flagsExclusion ) ;
		}
	}
	return	sglErrSuccess ;
}

// インデックス変換
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DRenderBufferedContext::IndexOfStereoView
			( S3DRenderContextInterface::StereoViewIndex sviView )
{
	switch ( sviView )
	{
	case	stereoViewAuto:
		return	indexAutoView ;
	case	stereoViewRight:
		return	indexRightView ;
	case	stereoViewLeft:
		return	indexLeftView ;
	}
	return	-1 ;
}

// S3DRenderBuffer 取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer *
	S3DRenderBufferedContext::GetRenderBuffer
		( S3DRenderContextInterface::StereoViewIndex sviView )
{
	QuickLock() ;
	ssize_t				iView = IndexOfStereoView( sviView ) ;
	S3DRenderBuffer *	pBuf = NULL ;
	if ( iView >= 0 )
	{
		pBuf = m_rbRenderBuffer[iView].m_render ;
		if ( pBuf == NULL )
		{
			ESLAssert( m_rbRenderBuffer[iView].m_render == NULL ) ;
			pBuf = NewRenderBuffer() ;
			pBuf->EnableSorting( true ) ;
			m_rbRenderBuffer[iView].m_render = pBuf ;
		}
	}
	QuickUnlock() ;
	return	pBuf ;
}

// S3DRenderBuffer 準備
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::SetRenderBuffer
	( S3DRenderBuffer * pAutoView,
		S3DRenderBuffer * pRightView, S3DRenderBuffer * pLeftView )
{
	if ( pAutoView == NULL )
	{
		pAutoView = NewRenderBuffer() ;
		pAutoView->EnableSorting( true ) ;
	}
	QuickLock() ;
	delete	m_rbRenderBuffer[indexAutoView].m_render ;
	delete	m_rbRenderBuffer[indexRightView].m_render ;
	delete	m_rbRenderBuffer[indexLeftView].m_render ;
	//
	m_rbRenderBuffer[indexAutoView].m_render = pAutoView ;
	m_rbRenderBuffer[indexRightView].m_render = pRightView ;
	m_rbRenderBuffer[indexLeftView].m_render = pLeftView ;
	//
	ssize_t	iView = IndexOfStereoView( m_sviView ) ;
	if ( iView >= 0 )
	{
		m_prbBuffer = &(m_rbRenderBuffer[iView]) ;
		m_pRender = GetRenderBuffer( m_sviView ) ;
		//
		m_pRender->SetShadingFlag( m_optContext.nShadingFlags ) ;
		m_pRender->AttachCustomShader( m_optContext.pShader ) ;
		m_pRender->SetOffsetBorderColor( m_optContext.opbBorder.rgbBorder ) ;
		m_pRender->SetOffsetBorderCoefficient
				( m_optContext.opbBorder.aThickness,
						m_optContext.opbBorder.bThickness ) ;
	}
	QuickUnlock() ;
}

// 描画の更新が存在するか？
//////////////////////////////////////////////////////////////////////////////
bool S3DRenderBufferedContext::IsEmptyRenderBuffer( void ) const
{
	bool	flagEmpty = true ;
	QuickLock() ;
	if ( m_mbufDelay.IsEmpty() )
	{
		for ( size_t i = 0; i < countView; i ++ )
		{
			if ( m_rbRenderBuffer[i].m_flagFillClear )
			{
				flagEmpty = false ;
				break ;
			}
			if ( m_rbRenderBuffer[i].m_render != NULL )
			{
				if ( !(m_rbRenderBuffer[i].m_render->IsEmptyBuffer()) )
				{
					flagEmpty = false ;
					break ;
				}
			}
		}
	}
	else
	{
		flagEmpty = false ;
	}
	QuickUnlock() ;
	return	flagEmpty ;
}

// S3DRenderBuffer 生成
//////////////////////////////////////////////////////////////////////////////
S3DRenderBuffer * S3DRenderBufferedContext::NewRenderBuffer( void )
{
	return	new S3DRenderBuffer ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	return	S3DRenderParameterContext::AppendTransformation( af, nTransparency ) ;
}

SGLError S3DRenderBufferedContext::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	return	S3DRenderParameterContext::SetTransformation( af, nTransparency ) ;
}

SGLError S3DRenderBufferedContext::PushTransformation( void )
{
	if ( m_pRender != NULL )
	{
		m_pRender->PushTransformation() ;
	}
	return	S3DRenderParameterContext::PushTransformation() ;
}

SGLError S3DRenderBufferedContext::PopTransformation( void )
{
	if ( m_pRender != NULL )
	{
		const S3DRenderBuffer::Transformation *
							pPrevTrans = m_pRender->GetPrevTransformation() ;
		S3DCustomShader *	pCurShader = m_pRender->GetCustomShader() ;
		uint64_t			nShadingFlags = m_pRender->GetShadingFlag() ;
		if ( (pPrevTrans != nullptr)
			&& !m_mbufDelay.IsEmpty()
			&& ((pPrevTrans->optContext.pShader != pCurShader)
				|| (pPrevTrans->optContext.nShadingFlags != nShadingFlags)) )
		{
			FlushDelayDraw() ;
		}
		m_pRender->PopTransformation() ;
	}
	return	S3DRenderParameterContext::PopTransformation() ;
}

SGLError S3DRenderBufferedContext::ResetTransformation( void )
{
	if ( m_pRender != NULL )
	{
		m_pRender->ResetTransformation() ;
	}
	return	S3DRenderParameterContext::ResetTransformation() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::FillClearTarget( uint32_t argb, int64_t flags )
{
	if ( m_prbBuffer == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (m_pRender != NULL) && (flags == 0) )
	{
		m_pRender->ClearBuffer() ;
	}
	else
	{
		Flush() ;
	}
	m_prbBuffer->m_flagFillClear = true ;
	m_prbBuffer->m_argbFillClear = argb ;
	m_prbBuffer->m_flagsFillClear = flags ;
	return	sglErrSuccess ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::FillRectangle
	( int x, int y, int width, int height,
		uint32_t argb, double z, uint32_t flags )
{
	S2DVector	vRect[4] ;
	vRect[0].x = (float32_t) x ;
	vRect[0].y = (float32_t) y ;
	vRect[1].x = (float32_t) x ;
	vRect[1].y = (float32_t) (y + height) ;
	vRect[2].x = (float32_t) (x + width) ;
	vRect[2].y = (float32_t) (y + height) ;
	vRect[3].x = (float32_t) (x + width) ;
	vRect[3].y = (float32_t) y ;
	return	FillPolygon( vRect, 4, argb, z, flags ) ;
}

SGLError S3DRenderBufferedContext::FillPolygon
	( const S2DVector * vertices, size_t count,
		uint32_t argb, double z, uint32_t flags )
{
	S3DRenderBuffer *	pRender = m_pRender ;
	if ( pRender == NULL )
	{
		return	sglErrFailed ;
	}
	pRender->PushTransformation() ;
	pRender->LockSyncBuffer() ;
	SetMatrixTransformationAsAffineTo( pRender, z, flags ) ;
	//
	size_t	countTriangles = count - 2 ;
	m_bufVertex.SetLength( count ) ;
	m_bufColor.SetLength( count ) ;
	m_bufIndexedList.SetLength( countTriangles * 3 ) ;
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DColor *		pColor = m_bufColor.GetArray() ;
	uint32_t *		pIndexedList = m_bufIndexedList.GetArray() ;
	//
	S3DMaterial *	pMaterial = &m_material2DPolygonNZ ;
	if ( flags & paintWithZOrder )
	{
		pMaterial = &m_material2DPolygon ;
	}
	if ( flags & paintWithZOrderNoWrite )
	{
		pMaterial = &m_material2DPolygonNWZ ;
	}
	//
	uint32_t	i, j ;
	for ( i = 0; i < count; i ++ )
	{
		pvVertex[i].x = vertices[i].x ;
		pvVertex[i].y = vertices[i].y ;
		pvVertex[i].z = 0 ;
	}
	for ( i = 0, j = 0; i < countTriangles; i ++, j += 3 )
	{
		pIndexedList[j] = 0 ;
		pIndexedList[j+1] = i + 1 ;
		pIndexedList[j+2] = i + 2 ;
	}
	GradationVertexColor( pColor, vertices, count, argb, flags ) ;
	//
	SGLError	err =
		AddIndexedTriangleList
			( pMaterial,
				RenderContext::renderFenceOrder,
				countTriangles, count,
				pvVertex, NULL, NULL, pColor, pIndexedList ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndexedList.FinishArray() ;
	pRender->UnlockSyncBuffer() ;
	//
	if ( !(flags & (paintDelayable | paintOrderNoCare)) )
	{
		Flush() ;
	}
	pRender->PopTransformation() ;
	return	err ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	S3DRenderBuffer *	pRender = m_pRender ;
	if ( (pRender == NULL) | (pSrcImage == NULL) )
	{
		return	sglErrFailed ;
	}
	SGLImageRect	rectSrcClip ;
	pSrcImage = pSrcImage->GetImageReference( rectSrcClip ) ;
	if ( pSrcImage == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pSrcClip != NULL )
	{
		rectSrcClip.x += pSrcClip->x ;
		rectSrcClip.y += pSrcClip->y ;
		rectSrcClip.w = pSrcClip->w ;
		rectSrcClip.h = pSrcClip->h ;
	}
	bool			flagCombinable ;
	S3DMaterial *	pMaterial =
		GetNoShadeMaterialOf( flagCombinable, pSrcImage, ppPaint.nFlags ) ;
	if ( pMaterial == NULL )
	{
		return	sglErrFailed ;
	}
	//
	pRender->PushTransformation() ;
	pRender->LockSyncBuffer() ;
	PrepareTransfomationToDrawImage( pRender, ppPaint ) ;
	//
	S3DVector4 *		pvVertex ;
	const S2DVector *	pvUVMap ;
	uint32_t *			pIndexedList ;
	uint32_t			i, j, countVertex ;
	//
	SGLError	err = sglErrFailed ;
	do
	{
		//
		// UV 座標
		//
		S2DVector *	pvBufUVMap ;
		if ( ppPaint.nFlags & paintPolygonShaped )
		{
			if ( (ppPaint.pVertices == NULL) || (ppPaint.countVertex < 3) )
			{
				break ;
			}
			countVertex = ppPaint.countVertex ;
			//
			if ( m_bufUVMap.GetLength() < countVertex )
			{
				m_bufUVMap.SetLength( countVertex ) ;
			}
			pvBufUVMap = m_bufUVMap.GetArray() ;
			eslMoveMemory
				( pvBufUVMap, ppPaint.pVertices,
					countVertex * sizeof(S2DVector) ) ;
			m_bufUVMap.FinishArray() ;
			pvUVMap = pvBufUVMap ;
		}
		else
		{
			countVertex = 4 ;
			if ( m_bufUVMap.GetLength() < countVertex )
			{
				m_bufUVMap.SetLength( countVertex ) ;
			}
			pvBufUVMap = m_bufUVMap.GetArray() ;
			pvUVMap = pvBufUVMap ;
			pvBufUVMap[0].x = (float32_t) 0.0f ;
			pvBufUVMap[0].y = (float32_t) 0.0f ;
			pvBufUVMap[1].x = (float32_t) 0.0f ;
			pvBufUVMap[1].y = (float32_t) (rectSrcClip.h) ;
			pvBufUVMap[2].x = (float32_t) (rectSrcClip.w) ;
			pvBufUVMap[2].y = (float32_t) (rectSrcClip.h) ;
			pvBufUVMap[3].x = (float32_t) (rectSrcClip.w) ;
			pvBufUVMap[3].y = (float32_t) 0.0f ;
			m_bufUVMap.FinishArray() ;
		}
		//
		// 頂点座標
		//
		pvVertex = PrepareVertexToDrawImage( pvUVMap, countVertex ) ;
		//
		if ( rectSrcClip.x | rectSrcClip.y )
		{
			ESLAssert( pvBufUVMap == pvUVMap ) ;
			float32_t	dx = (float32_t) rectSrcClip.x ;
			float32_t	dy = (float32_t) rectSrcClip.y ;
			for ( i = 0; i < countVertex; i ++ )
			{
				pvBufUVMap[i].x += dx ;
				pvBufUVMap[i].y += dy ;
			}
		}
		//
		// インデックス
		//
		size_t	countPolygon = countVertex - 2 ;
		if ( m_bufIndexedList.GetLength() < countPolygon * 3 )
		{
			m_bufIndexedList.SetLength( countPolygon * 3 ) ;
		}
		pIndexedList = m_bufIndexedList.GetArray() ;
		//
		for ( i = 0, j = 0; i < countPolygon; i ++, j += 3 )
		{
			pIndexedList[j] = 0 ;
			pIndexedList[j+1] = i + 1 ;
			pIndexedList[j+2] = i + 2 ;
		}
		//
		// 描画追加
		//
		uint32_t	nFlags = RenderContext::renderFenceOrder ;
		if ( !flagCombinable
			|| !(ppPaint.nFlags & (paintDelayable | paintOrderNoCare)) )
		{
			nFlags |= RenderContext::renderUncombinable ;
		}
		err = AddIndexedTriangleList
				( pMaterial, nFlags,
					countPolygon, countVertex,
					pvVertex, NULL, pvUVMap, NULL, pIndexedList ) ;
		//
		m_bufIndexedList.FinishArray() ;
	}
	while ( false ) ;
	pRender->UnlockSyncBuffer() ;
	//
	if ( !(ppPaint.nFlags & (paintDelayable | paintOrderNoCare)) )
	{
		Flush() ;
	}
	pRender->PopTransformation() ;
	return	err ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage,
		const SGLImageRect * pSrcClip )
{
	S3DRenderBuffer *	pRender = m_pRender ;
	size_t	countPolygon = widthMesh * heightMesh * 2 ;
	if ( (pRender == NULL) | (pSrcImage == NULL)
			| (pDstMesh == NULL) | (countPolygon == 0) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageRect	rectSrcClip ;
	pSrcImage = pSrcImage->GetImageReference( rectSrcClip ) ;
	if ( pSrcImage == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pSrcClip != NULL )
	{
		rectSrcClip.x += pSrcClip->x ;
		rectSrcClip.y += pSrcClip->y ;
		rectSrcClip.w = pSrcClip->w ;
		rectSrcClip.h = pSrcClip->h ;
	}
	bool			flagCombinable ;
	S3DMaterial *	pMaterial =
		GetNoShadeMaterialOf( flagCombinable, pSrcImage, ppPaint.nFlags ) ;
	if ( pMaterial == NULL )
	{
		return	sglErrFailed ;
	}
	S3DVector4 *		pvVertex ;
	const S2DVector *	pvUVMap ;
	uint32_t *			pIndexedList ;
	size_t				countVertex = (widthMesh + 1) * (heightMesh + 1) ;
	//
	// UV 座標
	//
	pRender->LockSyncBuffer() ;
	if ( pSrcMesh == NULL )
	{
		S2DVector *	pvBufUVMap ;
		if ( m_bufUVMap.GetLength() < countVertex )
		{
			m_bufUVMap.SetLength( countVertex ) ;
		}
		pvBufUVMap = m_bufUVMap.GetArray() ;
		pvUVMap = pvBufUVMap ;
		//
		float32_t	xStep, yStep ;
		xStep = (float32_t) rectSrcClip.w / widthMesh ;
		yStep = (float32_t) rectSrcClip.h / heightMesh ;
		//
		for ( size_t y = 0; y <= heightMesh; y ++ )
		{
			for ( size_t x = 0; x <= widthMesh; x ++ )
			{
				pvBufUVMap->x = (float32_t) (x * xStep + rectSrcClip.x) ;
				pvBufUVMap->y = (float32_t) (y * yStep + rectSrcClip.y) ;
				pvBufUVMap ++ ;
			}
		}
		m_bufUVMap.FinishArray() ;
	}
	else
	{
		pvUVMap = pSrcMesh ;
	}
	//
	// 座標変換設定
	//
	pRender->PushTransformation() ;
	PrepareTransfomationToDrawImage( pRender, ppPaint ) ;
	//
	// 頂点座標
	//
	pvVertex = PrepareVertexToDrawImage( pDstMesh, countVertex ) ;
	//
	// インデックス
	//
	if ( m_bufIndexedList.GetLength() < countPolygon * 3 )
	{
		m_bufIndexedList.SetLength( countPolygon * 3 ) ;
	}
	pIndexedList = m_bufIndexedList.GetArray() ;
	//
	for ( size_t y = 0; y < heightMesh; y ++ )
	{
		uint32_t	indexLine0 = (uint32_t) (y * (widthMesh + 1)) ;
		uint32_t	indexLine1 = (uint32_t) (indexLine0 + (widthMesh + 1)) ;
		for ( size_t x = 0; x < widthMesh; x ++ )
		{
			pIndexedList[0] = indexLine0 + (uint32_t) x ;
			pIndexedList[1] = indexLine1 + (uint32_t) x ;
			pIndexedList[2] = indexLine1 + (uint32_t) x + 1 ;
			pIndexedList[3] = indexLine0 + (uint32_t) x ;
			pIndexedList[4] = indexLine1 + (uint32_t) x + 1 ;
			pIndexedList[5] = indexLine0 + (uint32_t) x + 1 ;
			pIndexedList += 6 ;
		}
	}
	m_bufIndexedList.FinishArray() ;
	//
	// 描画追加
	//
	uint32_t	nFlags = RenderContext::renderFenceOrder ;
	if ( !flagCombinable
		|| !(ppPaint.nFlags & (paintDelayable | paintOrderNoCare)) )
	{
		nFlags |= RenderContext::renderUncombinable ;
	}
	SGLError	err =
		AddIndexedTriangleList
			( pMaterial, nFlags,
				countPolygon, countVertex,
				pvVertex, NULL, pvUVMap, NULL,
				m_bufIndexedList.GetConstArray() ) ;
	pRender->UnlockSyncBuffer() ;
	//
	if ( !(ppPaint.nFlags & (paintDelayable | paintOrderNoCare)) )
	{
		Flush() ;
	}
	pRender->PopTransformation() ;
	return	err ;
}

// 複数画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DrawMultiImages
	( size_t nCount,
		const SGLPaintParam * pParams,
		SGLImageObject *const* ppSrcImages,
		const SGLImageRect * pSrcClips )
{
	if ( nCount == 0 )
	{
		return	sglErrSuccess ;
	}
	S3DRenderBuffer *	pRender = m_pRender ;
	if ( (pRender == NULL) || (pParams == NULL) || (ppSrcImages == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageRect		rectRef0 ;
	SGLImageObject *	pSrcImage =
		ppSrcImages[0]->GetImageReference( rectRef0, 0 ) ;
	if ( pSrcImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	bool			flagCombinable ;
	S3DMaterial *	pMaterial =
		GetNoShadeMaterialOf
			( flagCombinable, pSrcImage, pParams[0].nFlags, shadingVertexAlpha ) ;
	if ( pMaterial == NULL )
	{
		return	sglErrFailed ;
	}
	pRender->PushTransformation() ;
	SetMatrixTransformationAsAffineTo
			( pRender, pParams[0].zOrder, pParams[0].nFlags ) ;
	//
	pRender->LockSyncBuffer() ;
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
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			S2DVector	v = affine * pvBufUV[j] ;
			pvBufVertex[j].x = v.x ;
			pvBufVertex[j].y = v.y ;
			pvBufVertex[j].z = 0.0f ;
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
	SGLError	err =
		AddIndexedTriangleList
			( pMaterial,
				RenderContext::renderFenceOrder
					| RenderContext::renderUncombinable,
				nPolyCount, nVertexCount,
				m_bufVertex.GetConstArray(), NULL,
				m_bufUVMap.GetConstArray(),
				m_bufColor.GetConstArray(),
				m_bufIndexedList.GetConstArray() ) ;
	//
	pRender->UnlockSyncBuffer() ;
	pRender->PopTransformation() ;
	//
	return	err ;
}

// 画像をテクスチャに持つシェーディング無しの表面属性を取得する
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DRenderBufferedContext::GetNoShadeMaterialOf
	( bool& flagCombinable,
		SGLImageObject * pSrcImage,
		uint32_t flagsPaint, uint64_t flagsShading )
{
	uint64_t	nShadingFlags = flagsShading ;
	if ( flagsPaint & paintWithZOrderNoWrite )
	{
		nShadingFlags |= shadingZBufferNoWrite ;
	}
	else if ( flagsPaint & paintWithZOrder )
	{
	}
	else
	{
		nShadingFlags |= shadingNoZBuffer ;
	}
	if ( (flagsPaint & paintMaskFunction) == paintFunctionAdd )
	{
		flagCombinable = false ;
		return	SGLImageNoShadeMaterialInterface::GetMaterialBy
				( pSrcImage, shadingTextureSmoothing
								| shadingMakeBlendAdd | nShadingFlags, NULL ) ;
	}
	else if ( flagsPaint & paintUnsmoothStretch )
	{
		flagCombinable = false ;
		return	SGLImageNoShadeMaterialInterface::GetMaterialBy
				( pSrcImage, nShadingFlags, NULL ) ;
	}
	else
	{
		flagCombinable = true ;
		return	SGLImageNoShadeMaterialInterface::GetMaterialBy
				( pSrcImage, shadingTextureSmoothing
							| shadingVertexAlpha | nShadingFlags, NULL ) ;
	}
}

// 描画パラメータから 2D 描画用座標変換と色効果・透明度を設定する
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::PrepareTransfomationToDrawImage
			( S3DRenderBuffer * pRender, const SGLPaintParam & ppPaint )
{
	SetMatrixTransformationAsAffineTo
			( pRender, ppPaint.zOrder, ppPaint.nFlags ) ;
	//
	S3DColor	colorPaint ;
	S3DColor *	pColor = NULL ;
	if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorAdd )
	{
		colorPaint.rgbMul.ui32 = 0x00FFFFFF ;
		colorPaint.rgbAdd = ppPaint.rgbColorParam ;
		pColor = &colorPaint ;
	}
	else if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorMul )
	{
		colorPaint.rgbMul = ppPaint.rgbColorParam ;
		colorPaint.rgbAdd.ui32 = 0 ;
		pColor = &colorPaint ;
	}
	SGLAffine	aff ;
	ppPaint.GetAffine( aff ) ;
	S3DDMatrix	matUnit( aff.a11, aff.a12, 0,  aff.a21, aff.a22, 0,  0, 0, 1 ) ;
	S3DDVector	vZero( aff.a13, aff.a23, 0 ) ;
	pRender->AppendMatrixTransformation
			( matUnit, vZero, pColor, ppPaint.nTransparency ) ;
}

// 2D 画像描画の際の頂点座標を準備する
//////////////////////////////////////////////////////////////////////////////
S3DVector4 * S3DRenderBufferedContext::PrepareVertexToDrawImage
	( const S2DVector * pvUVMap, size_t countVertex )
{
	S3DVector4 *	pvVertex ;
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.SetLength( countVertex ) ;
	}
	pvVertex = m_bufVertex.GetArray() ;
	//
	for ( size_t i = 0; i < countVertex; i ++ )
	{
		pvVertex[i].x = pvUVMap[i].x ;
		pvVertex[i].y = pvUVMap[i].y ;
		pvVertex[i].z = 0.0f ;
	}
	m_bufVertex.FinishArray() ;
	//
	return	pvVertex ;
}

// 点描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DrawPoints
	( const S2DVector * pPoints, size_t nPoints,
		uint32_t argb, double z, uint32_t flags )
{
	S3DRenderBuffer *	pRender = m_pRender ;
	if ( pRender == NULL )
	{
		return	sglErrFailed ;
	}
	pRender->PushTransformation() ;
	pRender->LockSyncBuffer() ;
	SetMatrixTransformationAsAffineTo( pRender, z, flags ) ;
	//
	size_t	nCount = nPoints ;
	m_bufVertex.SetLength( nCount ) ;
	m_bufColor.SetLength( nCount ) ;
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DColor *		pColor = m_bufColor.GetArray() ;
	//
	S3DMaterial *	pMaterial = &m_material2DPolygonNZ ;
	if ( flags & paintWithZOrder )
	{
		pMaterial = &m_material2DPolygon ;
	}
	if ( flags & paintWithZOrderNoWrite )
	{
		pMaterial = &m_material2DPolygonNWZ ;
	}
	//
	uint32_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		pvVertex[i].x = pPoints[i].x ;
		pvVertex[i].y = pPoints[i].y ;
		pvVertex[i].z = 0 ;
	}
	GradationVertexColor( pColor, pPoints, nCount, argb, flags ) ;
	//
	SGLError	err =
		AddIndexedPrimitiveList
			( pMaterial,
				RenderContext::renderFenceOrder,
				primitivePoint, nCount, nCount,
				pvVertex, NULL, NULL, pColor, NULL ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufColor.FinishArray() ;
	pRender->UnlockSyncBuffer() ;
	//
	if ( !(flags & (paintDelayable | paintOrderNoCare)) )
	{
		Flush() ;
	}
	pRender->PopTransformation() ;
	return	err ;
}

// 直線描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DrawThinLines
	( const S2DVector * pLines, size_t nLines,
		uint32_t argb, double z, uint32_t flags )
{
	S3DRenderBuffer *	pRender = m_pRender ;
	if ( pRender == NULL )
	{
		return	sglErrFailed ;
	}
	pRender->PushTransformation() ;
	pRender->LockSyncBuffer() ;
	SetMatrixTransformationAsAffineTo( pRender, z, flags ) ;
	//
	size_t	nCount = nLines + 1 ;
	m_bufVertex.SetLength( nCount ) ;
	m_bufColor.SetLength( nCount ) ;
	m_bufIndexedList.SetLength( nLines * 2 ) ;
	//
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DColor *		pColor = m_bufColor.GetArray() ;
	//
	S3DMaterial *	pMaterial = &m_material2DPolygonNZ ;
	if ( flags & paintWithZOrder )
	{
		pMaterial = &m_material2DPolygon ;
	}
	if ( flags & paintWithZOrderNoWrite )
	{
		pMaterial = &m_material2DPolygonNWZ ;
	}
	//
	uint32_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		pvVertex[i].x = pLines[i].x ;
		pvVertex[i].y = pLines[i].y ;
		pvVertex[i].z = 0 ;
	}
	GradationVertexColor( pColor, pLines, nCount, argb, flags ) ;
	//
	uint32_t *	pIndexes = m_bufIndexedList.GetArray() ;
	for ( i = 0; i < nLines; i ++ )
	{
		pIndexes[i + i]     = i ;
		pIndexes[i + i + 1] = i + 1 ;
	}
	//
	SGLError	err =
		AddIndexedPrimitiveList
			( pMaterial,
				RenderContext::renderFenceOrder,
				primitiveLine, nLines * 2, nCount,
				pvVertex, NULL, NULL, pColor, pIndexes ) ;
	//
	m_bufVertex.FinishArray() ;
	m_bufColor.FinishArray() ;
	m_bufIndexedList.FinishArray() ;
	pRender->UnlockSyncBuffer() ;
	//
	if ( !(flags & (paintDelayable | paintOrderNoCare)) )
	{
		Flush() ;
	}
	pRender->PopTransformation() ;
	return	err ;
}

// グラデーション解除
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::FreeGradation( void )
{
	m_opFillType = fillFlat ;
}

// 線形グラデーション設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SetLinearGradation
	( float32_t x0, float32_t y0, float32_t x1, float32_t y1,
			const SGLPalette * pGradation, size_t nCount )
{
	m_aGradation.RemoveAll() ;
	m_aGradation.AddArray( pGradation, nCount ) ;
	//
	float32_t	dx = x1 - x0 ;
	float32_t	dy = y1 - y0 ;
	float32_t	r = (float32_t) sqrt( dx * dx + dy * dy ) ;
	if ( r > 0.0 )
	{
		float32_t	s = (float32_t) (nCount - 1) / (r  * r);
		dx *= s ;
		dy *= s ;
	}
	//
	m_vGradationCenter.x = x0 ;
	m_vGradationCenter.y = y0 ;
	m_vGradationDelta.x = dx ;
	m_vGradationDelta.y = dy ;
	//
	m_opFillType = fillLinearGradation ;
	return	sglErrSuccess ;
}

// 環状グラデーション設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SetRingedGradation
	( float32_t xCenter, float32_t yCenter, float32_t radAngle,
		const SGLPalette * pGradation, size_t nCount )
{
	m_aGradation.RemoveAll() ;
	m_aGradation.AddArray( pGradation, nCount ) ;
	//
	m_vGradationCenter.x = xCenter ;
	m_vGradationCenter.y = yCenter ;
	//
	float32_t	a = (float32_t) floor( radAngle / (2.0 * PI) ) ;
	m_radGradation = (4.0f + a * 2.0f) * (float32_t) PI - radAngle ;
	//
	m_opFillType = fillRingedGradation ;
	return	sglErrSuccess ;
}

// グラデーションを頂点に反映
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::GradationVertexColor
	( S3DColor * pColor,
		const S2DVector * pPoints, size_t nCount,
		uint32_t argb, uint32_t flags ) const
{
	S3DColor	colorPaint ;
	if ( m_opFillType == fillFlat )
	{
		colorPaint.rgbAdd.ui32 = argb & 0x00FFFFFF ;
		colorPaint.rgbMul.argb.Red = (uint8_t) (argb >> 24) ^ 0xFF ;
		colorPaint.rgbMul.argb.Green = colorPaint.rgbMul.argb.Red ;
		colorPaint.rgbMul.argb.Blue = colorPaint.rgbMul.argb.Red ;
		colorPaint.rgbMul.argb.Alpha = 0xFF ;
		//
		if ( flags & paintApplyAlphaMul )
		{
			colorPaint.rgbAdd *= (unsigned int) colorPaint.rgbMul.argb.Red + 1 ;
		}
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pColor[i] = colorPaint ;
		}
	}
	else if ( m_opFillType == fillLinearGradation )
	{
		const SGLPalette *
					pGradarions = m_aGradation.GetConstArray() ;
		const int	nGradations = (int) m_aGradation.GetLength() - 1 ;
		int32_t		fxDegree ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			S2DVector	v = pPoints[i] ;
			v -= m_vGradationCenter ;
			fxDegree = eslRoundR32ToInt
						( (v.x * m_vGradationDelta.x
							+ v.y * m_vGradationDelta.y) * 0x10000 ) ;
			//
			const int	j = esl_clampi
								( (int) (fxDegree >> 16), 0, nGradations ) ;
			const int	k = esl_clampi
								( (j + 1), 0, nGradations ) ;
			uint32_t	nDec = (uint32_t) (fxDegree >> 8) & 0xFF ;
			//
			SGLPalette	rgbaPoint = pGradarions[j].imul(0x100 - nDec)
									+ pGradarions[k].imul(nDec) ;
			colorPaint.rgbAdd.ui32 = rgbaPoint & 0x00FFFFFF ;
			colorPaint.rgbMul.argb.Red = (uint8_t) (rgbaPoint >> 24) ^ 0xFF ;
			colorPaint.rgbMul.argb.Green = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbMul.argb.Blue = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbMul.argb.Alpha = 0xFF ;
			//
			pColor[i] = colorPaint ;
		}
	}
	else if ( m_opFillType == fillRingedGradation )
	{
		const SGLPalette *
					pGradarions = m_aGradation.GetConstArray() ;
		const int	nGradations = (int) m_aGradation.GetLength() - 1 ;
		double		r = (double) (nGradations - 1) / (2.0 * PI) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			S2DVector	v = pPoints[i] ;
			v -= m_vGradationCenter ;
			//
			float32_t	a = (float32_t) ((atan2( - v.y, v.x )
											+ m_radGradation) * r) ;
			uint32_t	j = (uint32_t) eslRoundR32ToInt( a * 0x100 ) ;
			uint32_t	nDec = j % 0xFF ;
			j %= (uint32_t) nGradations ;
			//
			SGLPalette	rgbaPoint = pGradarions[j].imul(0x100 - nDec)
									+ pGradarions[j + 1].imul(nDec) ;
			colorPaint.rgbAdd.ui32 = rgbaPoint & 0x00FFFFFF ;
			colorPaint.rgbMul.argb.Red = (uint8_t) (rgbaPoint >> 24) ^ 0xFF ;
			colorPaint.rgbMul.argb.Green = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbMul.argb.Blue = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbMul.argb.Alpha = 0xFF ;
			//
			pColor[i] = colorPaint ;
		}
	}
}

// 遅延描画バッファ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::DelayIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( m_pRender == nullptr )
	{
		return	sglErrFailed ;
	}
	m_csDelayBuf.Lock() ;
	if ( m_mbufDelay.GetIndexCount() + countIndex >= 0x1000 )
	{
		m_csDelayBuf.Unlock() ;
		FlushDelayDraw() ;
		m_csDelayBuf.Lock() ;
	}
	if ( m_mbufDelay.AddIndexedPrimitiveList
		( pMaterial, typePrimitive, countIndex, countVertex, 0,
			pvVertex, pvNormal, pvUVMap, pColor, nullptr,
			pIndexedList, m_pRender->GetTransformation() ) )
	{
		m_csDelayBuf.Unlock() ;
		FlushDelayDraw() ;
		m_csDelayBuf.Lock() ;
		//
		if ( m_mbufDelay.AddIndexedPrimitiveList
			( pMaterial, typePrimitive, countIndex, countVertex, 0,
				pvVertex, pvNormal, pvUVMap, pColor, nullptr,
				pIndexedList, m_pRender->GetTransformation() ) )
		{
			m_csDelayBuf.Unlock() ;
			return	m_pRender->AddIndexedPrimitiveList
						( pMaterial, nFlags,
							typePrimitive, countIndex, countVertex,
							pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
		}
	}
	m_nDelayFlags |= nFlags ;
	m_csDelayBuf.Unlock() ;
	return	sglErrSuccess ;
}

// 遅延バッファフラッシュ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::FlushDelayDraw( void )
{
	if ( m_pRender != nullptr )
	{
		m_pRender->LockSyncBuffer() ;
	}
	m_csDelayBuf.Lock() ;
	if ( m_mbufDelay.IsEmpty() )
	{
		m_csDelayBuf.Unlock() ;
		if ( m_pRender != nullptr )
		{
			m_pRender->UnlockSyncBuffer() ;
		}
		return	sglErrSuccess ;
	}
	if ( m_pRender == nullptr )
	{
		m_mbufDelay.ClearBuffer() ;
		m_nDelayFlags = 0 ;
		m_csDelayBuf.Unlock() ;
		return	sglErrFailed ;
	}
	if ( m_nDelayFlags & RenderContext::renderFenceOrder )
	{
		m_pRender->Flush() ;
	}
	S3DDMatrix	matI( 1, 1, 1 ) ;
	S3DDVector	vZero( 0, 0, 0 ) ;
	S3DColor	clrStraight( 0xFFFFFFFF, 0 ) ;
	m_pRender->PushTransformation() ;
	m_pRender->SetMatrixTransformation( matI, vZero, &clrStraight, 0 ) ;
	m_mbufDelay.Render( *m_pRender ) ;
	m_mbufDelay.ClearBuffer() ;
	m_nDelayFlags = 0 ;
	m_pRender->PopTransformation() ;
	m_csDelayBuf.Unlock() ;
	m_pRender->UnlockSyncBuffer() ;
	//
	return	sglErrSuccess ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::SetShadingFlag( uint64_t nShadingMethod )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetShadingFlag( nShadingMethod ) ;
	}
	S3DRenderParameterContext::SetShadingFlag( nShadingMethod ) ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AttachCustomShader( S3DCustomShader * pShader )
{
	SGLError	err1 = sglErrSuccess ;
	if ( m_pRender != NULL )
	{
		FlushDelayDraw() ;
		err1 = m_pRender->AttachCustomShader( pShader ) ;
	}
	SGLError	err2 = S3DRenderParameterContext::AttachCustomShader( pShader ) ;
	return	err1 ? err1 : err2 ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DRenderBufferedContext::GetCustomShader( void ) const
{
	if ( m_pRender != NULL )
	{
		return	m_pRender->GetCustomShader() ;
	}
	return	S3DRenderParameterContext::GetCustomShader() ;
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::SetOffsetBorderColor( uint32_t rgbBorder )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetOffsetBorderColor( rgbBorder ) ;
	}
	S3DRenderParameterContext::SetOffsetBorderColor( rgbBorder ) ;
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetOffsetBorderCoefficient( a, b ) ;
	}
	S3DRenderParameterContext::SetOffsetBorderCoefficient( a, b ) ;
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SetOptionalFeature
	( FeatureType feature, int32_t nParam1,
				const void * pParam2, size_t sizeOfParam2 )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetOptionalFeature
			( feature, nParam1, pParam2, sizeOfParam2 ) ;
	}
	return	S3DRenderParameterContext::SetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// オプショナル機能取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::GetOptionalFeature
	( FeatureType feature, int32_t nParam1,
				void * pParam2, size_t sizeOfParam2 ) const
{
	if ( m_pRender != NULL )
	{
		return	m_pRender->GetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
	}
	return	S3DRenderParameterContext::GetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AppendMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	if ( m_pRender != NULL )
	{
		m_pRender->AppendMatrixTransformation( mat, pos, color, nTransparency ) ;
	}
	return	S3DRenderParameterContext::AppendMatrixTransformation
									( mat, pos, color, nTransparency ) ;
}

SGLError S3DRenderBufferedContext::SetMatrixTransformation
	( const S3DDMatrix& mat, const S3DDVector& pos,
		const S3DColor * color, unsigned int nTransparency )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetMatrixTransformation( mat, pos, color, nTransparency ) ;
	}
	return	S3DRenderParameterContext::SetMatrixTransformation
									( mat, pos, color, nTransparency ) ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
			S3DCustomShader::UniformType type,
			const void * pData, size_t nCount )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetCustomShaderUniform( pwszUniformId, type, pData, nCount ) ;
	}
	return	S3DRenderParameterContext::SetCustomShaderUniform
									( pwszUniformId, type, pData, nCount ) ;
}

SGLError S3DRenderBufferedContext::ResetCustomShaderUniform( void )
{
	if ( m_pRender != NULL )
	{
		return	m_pRender->ResetCustomShaderUniform() ;
	}
	return	S3DRenderParameterContext::ResetCustomShaderUniform() ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetCamera( matCamera, posCamera ) ;
	}
	return	S3DRenderParameterContext::SetCamera( matCamera, posCamera ) ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	return	AddIndexedPrimitiveList
				( pMaterial, nFlags, primitiveTriangle,
					countPolygon * 3, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	return	AddIndexedPrimitiveList
				( pMaterial, nFlags, primitiveTriangleStrip,
					countTriangleStrip + 2, countTriangleStrip + 2,
					pvVertex, pvNormal, pvUVMap, pColor, nullptr ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( m_pRender == NULL )
	{
		return	sglErrFailed ;
	}
	if ( countVertex == 0 )
	{
		return	sglErrSuccess ;
	}
	if ( !(nFlags & RenderContext::renderUncombinable)
		&& (countIndex < 0x100) && (countVertex < 0x100) )
	{
		return	DelayIndexedPrimitiveList
					( pMaterial, nFlags, typePrimitive,
						countIndex, countVertex,
						pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
	else if ( nFlags & RenderContext::renderFenceOrder )
	{
		FlushDelayDraw() ;
	}
	return	m_pRender->AddIndexedPrimitiveList
				( pMaterial, nFlags,
					typePrimitive, countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing )
{
	if ( m_pRender == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRender->AddVertexBuffer
				( pMaterial, nFlags, pBuffer, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::Flush( void )
{
	if ( m_pRender == NULL )
	{
		return	sglErrSuccess ;
	}
	FlushDelayDraw() ;
	return	m_pRender->Flush() ;
}

SGLError S3DRenderBufferedContext::Finish( void )
{
	FlushDelayDraw() ;
	return	sglErrSuccess ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DRenderBufferedContext::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	eslFillMemory( &caps, 0, sizeof(S3DRenderingCapacity) ) ;
	caps.flagsRendering |= S3DRenderingCapacity::renderingAutoStereo
							| S3DRenderingCapacity::renderingSortBuffered ;
	caps.flagsShading |= S3DRenderingCapacity::shadingGouraud ;
}

// 立体視用バッファ選択
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SelectParallaxView
			( S3DRenderContextInterface::StereoViewIndex sviView )
{
	FlushDelayDraw() ;

	SGLError	err =
		S3DRenderParameterContext::SelectParallaxView( sviView ) ;
	//
	QuickLock() ;
	ssize_t	iView = IndexOfStereoView( sviView ) ;
	if ( iView >= 0 )
	{
		m_prbBuffer = &(m_rbRenderBuffer[iView]) ;
		m_pRender = GetRenderBuffer( m_sviView ) ;
	}
	else
	{
		err = sglErrFailed ;
	}
	QuickUnlock() ;
	//
	return	err ;
}

// 内部バッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRenderBufferedContext::SetRenderingBufferSize( uint32_t countVertex )
{
	if ( m_pRender != NULL )
	{
		m_pRender->SetBufferUnitSize( countVertex * sizeof(S3DVector4) * 3 ) ;
	}
	return	S3DRenderParameterContext::SetRenderingBufferSize( countVertex ) ;
}

