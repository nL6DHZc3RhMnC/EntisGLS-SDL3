
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_opengl_render_context.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// OpenGL レンダリング・コンテキスト（直接／OpenGL スレッド上で利用）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DOpenGLDirectlyRenderer, S3DRenderParameterContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DOpenGLDirectlyRenderer::S3DOpenGLDirectlyRenderer( void )
{
	m_pglDefShader = NULL ;
	m_pglCustomShader = NULL ;
	m_pglColor = NULL ;
	m_pglDepth = NULL ;
	m_vertOrder = verticalAuto ;
	m_flagUpdate = false ;
	m_flag3DPriority = false ;
	m_flagIn3DRenderer = false ;
	m_flagDrawImage = false ;
	m_flagFixViewPort = false ;
	m_flagProjUpsideDown = false ;
	m_flagMirrorFBO = false ;
	m_glMirrorFBO = 0 ;
	//
	m_optContext.nShadingFlags = shadingMethodNothing | shadingTextureSmoothing ;
	ReflectShadingFlags() ;
}

// 3Dレンダリングを優先する（2D描画も透視行列で行う）
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::Have3DPriorityOver2D( bool flag3DPriority )
{
	m_flag3DPriority = flag3DPriority ;
	m_flagIn3DRenderer |= flag3DPriority ;
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::AttachShader( SGLOpenGLDefaultShader * pglShader )
{
	m_pglDefShader = pglShader ;
	if ( pglShader != NULL )
	{
		pglShader->InitializeShader() ;
	}
	else
	{
		SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		if ( pOpenGL != NULL )
		{
			pOpenGL->AttachShaderProgram( NULL ) ;
		}
		else if ( OpenGLExtension::g_supports_opengl_2_0 )
		{
			glUseProgram( 0 ) ;
			SGLOpenGLContext::VerifyError( "glUseProgram" ) ;
		}
	}
}

// ターゲット更新フラグ
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::IsUpdatedTarget( void ) const
{
	return	m_flagUpdate ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::AttachGLCustomShader( SGLOpenGLCustomShader * pglShader )
{
	m_pglCustomShader = pglShader ;
	//
	if ( m_flagIn3DRenderer )
	{
		ReflectShadingFlags() ;
	}
}

// ビューポート（m_glView）の設定は AttachTargetImage で変更しない
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetFixViewport( bool flagFixView )
{
	m_flagFixViewPort = flagFixView ;
}

// 描画の上下反転設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetViewVerticalOrder
		( S3DOpenGLDirectlyRenderer::VerticalOrder vertOrder )
{
	m_vertOrder = vertOrder ;
}

// ミラーリング FBO 転送設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetMirrorFrameBuffer
	( GLuint glDstRightFBO, GLuint glDstLeftFBO,
		const SGLImageRect& rectSrc, const SGLImageRect& rectDst )
{
	m_flagMirrorFBO = true ;
	m_glMirrorFBOs[stereoViewRight] = glDstRightFBO ;
	m_glMirrorFBOs[stereoViewLeft] = glDstLeftFBO ;
	m_rectMirrorSrcFBO = rectSrc ;
	m_rectMirrorDstFBO = rectDst ;
	//
	if ( m_fStereoView && (m_sviView != stereoViewAuto) )
	{
		ESLAssert( (m_sviView >= 0) && (m_sviView < 2) ) ;
		m_glMirrorFBO = m_glMirrorFBOs[m_sviView] ;
	}
	else
	{
		m_glMirrorFBO = m_glMirrorFBOs[stereoViewRight] ;
	}
}

void S3DOpenGLDirectlyRenderer::DetachMirrorFrameBuffer( void )
{
	m_flagMirrorFBO = false ;
	m_glMirrorFBO = 0 ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AttachTargetImage
	( SGLImageObject * pImage,
		SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	S3DRenderParameterContext::AttachTargetImage( pImage, pZBuffer, pView ) ;
	//
	SGLError		err ;
	SGLImageRect	rectViewTemp, rectRefImage, rectRefZBuf ;
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	m_pglColor =
		SGLOpenGLTextureBuffer::CommitGLTexture( pOpenGL, pImage, rectRefImage ) ;
	m_pglDepth =
		SGLOpenGLTextureBuffer::CommitGLTexture( pOpenGL, pZBuffer, rectRefImage ) ;
	if ( m_pglColor != NULL )
	{
		err = m_glFrameBuffer.CreateFrameBuffer() ;
		if ( err )
		{
			return	err ;
		}
		m_glFrameBuffer.AttachFrameBuffer( m_pglColor, m_pglDepth ) ;
		//
		SGLSize	sizeImage = pImage->GetImageSize() ;
		if ( !m_flagFixViewPort )
		{
			m_glView.SetPhysicalViewSize( sizeImage ) ;
			m_glView.SetVirtualViewSize( sizeImage ) ;
			m_glView.EnableViewRotation( false ) ;
		}
		//
		rectViewTemp.x = rectRefImage.x ;
		rectViewTemp.y = rectRefImage.y ;
		if ( pImage->GetBufferFlags()
				& (SGLImageObject::bufferCubeMapTexture
					| SGLImageObject::bufferTexture3D
					| SGLImageObject::bufferTextureArray) )
		{
			rectViewTemp.x = 0 ;
			rectViewTemp.y = 0 ;
		}
		rectViewTemp.w = sizeImage.w ;
		rectViewTemp.h = sizeImage.h ;
		//
		if ( pView != NULL )
		{
			rectViewTemp.x += pView->x ;
			rectViewTemp.y += pView->y ;
			rectViewTemp.w = pView->w ;
			rectViewTemp.h = pView->h ;
		}
		pView = &rectViewTemp ;
		m_flagProjUpsideDown = true ;
	}
	else
	{
		m_glFrameBuffer.ReleaseFrameBuffer() ;
		m_glFrameBuffer.DetachFrameBuffer() ;
		m_flagProjUpsideDown = false ;
	}
	if ( m_vertOrder != verticalAuto )
	{
		if ( m_vertOrder == verticalStright )
		{
			m_flagProjUpsideDown = !m_flagProjUpsideDown ;
		}
		else if ( m_vertOrder == verticalStright )
		{
			m_flagProjUpsideDown = false ;
		}
		else if ( m_vertOrder == verticalStright )
		{
			m_flagProjUpsideDown = true ;
		}
	}
	ReflectShadingFlags() ;
	//
	m_glView.SetOpenGLViewPort( pView ) ;
	if ( m_flagFixViewPort )
	{
		m_glView.m_flagViewport = false ;
	}

	UpdateGLProjection() ;
	return	sglErrSuccess ;
}

// マルチターゲット（2つ目以降）描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AttachMultiTargetImages
	( SGLImageObject *const* ppTargets, size_t nCount )
{
	S3DRenderParameterContext::AttachMultiTargetImages( ppTargets, nCount ) ;
	//
	if ( nCount > 0 )
	{
		SPointerArray<SGLOpenGLTextureBuffer>	aTargetColors ;
		SGLOpenGLContext *
				pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		ESLAssert( pOpenGL != NULL ) ;
		//
		SGLOpenGLTextureBuffer **
				ppTargetColors = aTargetColors.GetArray( nCount ) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( ppTargets[i] != NULL )
			{
				SGLImageRect	rectRefImage ;
				ppTargetColors[i] =
					SGLOpenGLTextureBuffer::CommitGLTexture
								( pOpenGL, ppTargets[i], rectRefImage ) ;
			}
		}
		m_glFrameBuffer.AddRenderTarget( ppTargetColors, nCount ) ;
		aTargetColors.FinishArray() ;
	}
	else
	{
		m_glFrameBuffer.AddRenderTarget( NULL, 0 ) ;
	}
	return	sglErrSuccess ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::DetachTargetImage( void )
{
	Finish() ;
	//
	m_glFrameBuffer.DetachFrameBuffer() ;
	m_pglColor = NULL ;
	m_pglDepth = NULL ;
	return	S3DRenderParameterContext::DetachTargetImage() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::FillClearTarget
						( uint32_t argb, int64_t flags )
{
	GLbitfield	bfClear = 0 ;
	if ( (flags == 0) || (flags & clearTargetColor) )
	{
//		if ( (m_glView.m_rectViewClip.w == m_glView.m_sizeVirtual.w)
//			&& (m_glView.m_rectViewClip.h == m_glView.m_sizeVirtual.h) )
		{
			//
			// 画面全体のクリア
			//
			SGLPalette	argbColor( argb ) ;
			const float	scaleBy255 = 1.0f / 255.0f ;
			glClearColor
				( (float) argbColor.argb.Red * scaleBy255,
					(float) argbColor.argb.Green * scaleBy255,
					(float) argbColor.argb.Blue * scaleBy255,
					(float) argbColor.argb.Alpha * scaleBy255 ) ;
			SGLOpenGLContext::VerifyError( "glClearColor" ) ;
			//
			bfClear |= GL_COLOR_BUFFER_BIT ;
		}
/*
		else
		{
			//
			// 部分のみのクリアでは矩形フィル
			//
			ESLTrace( "FillClearTarget partial rectangle with OpenGL.\n" ) ;
			if ( m_pglDefShader != NULL )
			{
				S3DColor	color ;
				color.rgbMul.ui32 = 0 ;
				color.rgbAdd.ui32 = argb ;
				//
				m_pglDefShader->SetMaterial( NULL ) ;
				m_pglDefShader->SetColorEffect( &color ) ;
			}
			else
			{
				m_glRenderer.FlushGLMaterial() ;
				//
				// ソース色で上書き
				glEnable( GL_BLEND ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_BLEND)" ) ;
				glBlendFunc( GL_ONE, GL_ZERO ) ;
				SGLOpenGLContext::VerifyError( "glBlendFunc" ) ;
				//
				// 両面
				glDisable( GL_CULL_FACE ) ;
				SGLOpenGLContext::VerifyError( "glDisable(GL_CULL_FACE)" ) ;
				//
				// ｚテスト／ｚバッファへの書き込み無効
				glDisable( GL_DEPTH_TEST ) ;
				SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
				glDepthMask( GL_FALSE ) ;
				SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
				//
				// 塗りつぶし色
				SGLPalette	argbColor( argb ) ;
				const float	scaleBy255 = 1.0f / 255.0f ;
				glColor4f
					( (float) argbColor.argb.Red * scaleBy255,
						(float) argbColor.argb.Green * scaleBy255,
						(float) argbColor.argb.Blue * scaleBy255,
						(float) argbColor.argb.Alpha * scaleBy255 ) ;
				SGLOpenGLContext::VerifyError( "glColor4f" ) ;
				//
				SGLOpenGLContext *	pOpenGL = m_pglDefShader->m_pOpenGL ;
				pOpenGL->m_modeBlend = SGLOpenGLContext::blendCopy ;
			}
			//
			// 頂点設定
			float32_t	xs = 0.0f, ys = 0.0f, z = 0.0f ;
			if ( !m_glView.IsOrthogonalProjection() )
			{
				xs = m_vProjectionScreen.x ;
				ys = m_vProjectionScreen.y ;
				z = m_vProjectionScreen.z ;
			}
			float32_t	left = (float32_t) m_glView.m_rectViewClip.x ;
			float32_t	top = (float32_t) m_glView.m_rectViewClip.y ;
			float32_t	right =
				(float32_t) (m_glView.m_rectViewClip.x
								+ m_glView.m_rectViewClip.w) ;
			float32_t	bottom =
				(float32_t) (m_glView.m_rectViewClip.y
								+ m_glView.m_rectViewClip.h) ;
			//
			S3DVector4	vRect[4] ;
			vRect[0].x = left - xs ;
			vRect[0].y = top - ys ;
			vRect[0].z = z ;
			vRect[1].x = left - xs ;
			vRect[1].y = bottom - ys ;
			vRect[1].z = z ;
			vRect[2].x = right - xs ;
			vRect[2].y = bottom - ys ;
			vRect[2].z = z ;
			vRect[3].x = right - xs ;
			vRect[3].y = top - ys ;
			vRect[3].z = z ;
			//
			// 描画
			if ( m_pglDefShader != NULL )
			{
				m_pglDefShader->SetVertexPointer( &vRect[0] ) ;
				m_pglDefShader->SetColorPointer
					( m_pglDefShader->AllocateDummyVertexColorBuffer( 4 ) ) ;
				//
				glDrawArrays( GL_TRIANGLE_FAN, 0, 4 ) ;
				SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_FAN)" ) ;
				//
				m_pglDefShader->SetVertexPointer( NULL ) ;
				m_pglDefShader->SetColorPointer( NULL ) ;
				//
				if ( m_pTransformation != NULL )
				{
					m_pglDefShader->SetColorEffect
						( &(m_pTransformation->colorEffect),
									m_pTransformation->nTransparency ) ;
				}
				else
				{
					m_pglDefShader->SetColorEffect() ;
				}
			}
			else
			{
				glEnableClientState( GL_VERTEX_ARRAY ) ;
				SGLOpenGLContext::VerifyError( "glEnableClientState(GL_VERTEX_ARRAY)" ) ;
				glVertexPointer( 3, GL_FLOAT, sizeof(S3DVector4), vRect ) ;
				SGLOpenGLContext::VerifyError( "glVertexPointer" ) ;
				//
				glDrawArrays( GL_TRIANGLE_FAN, 0, 4 ) ;
				SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_FAN)" ) ;
				//
				m_glRenderer.FlushVertexPointers() ;
			}
			m_flagUpdate = true ;
		}
*/
	}
	if ( (flags == 0) || (flags & clearTargetZBuffer) )
	{
		//
		// ｚバッファのクリアは常に全体に行う
		//
		glDepthMask( GL_TRUE ) ;
		SGLOpenGLContext::VerifyError( "glDepthMask(GL_TRUE)" ) ;
		glEnable( GL_DEPTH_TEST ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
		glDepthFunc( GL_LEQUAL );
		SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
		//
		SGLOpenGLContext *
				pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		ESLAssert( pOpenGL != NULL ) ;
		pOpenGL->m_funcDepthTest = 0 ;
		//
		bfClear |= GL_DEPTH_BUFFER_BIT ;
	}
	if ( bfClear != 0 )
	{
		glClear( bfClear ) ;
		if ( !SGLOpenGLContext::VerifyError( "glClear" ) )
		{
			return	sglErrFailed ;
		}
		m_flagUpdate = true ;
		//
		if ( m_pglDefShader == NULL )
		{
			m_glRenderer.FlushGLMaterial() ;
		}
	}
	return	sglErrSuccess ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::FillRectangle
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
	//
	return	FillPolygon( vRect, 4, argb, z, flags ) ;
}

SGLError S3DOpenGLDirectlyRenderer::FillPolygon
	( const S2DVector * vertices, size_t count,
		uint32_t argb, double z, uint32_t flags )
{
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;

	FlushGLMaterial() ;
	FlushVertexPointers() ;

	PutGLByPaintFlags( pOpenGL, flags ) ;
	PutGLPaintColor( argb ) ;
	if ( !PutVertex2D( vertices, count, z, flags ) )
	{
		FlushVertexPointers() ;
		return	sglErrSuccess ;
	}

	glDrawArrays( GL_TRIANGLE_FAN, 0, (GLsizei) count ) ;
	SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_FAN)" ) ;

	FlushVertexPointers() ;
	m_flagUpdate = true ;

	return	sglErrSuccess ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	FlushGLMaterial() ;
	//
	// 画像設定
	//
	SGLAffine	afUV ;
	if ( !PutTexture2D( afUV, ppPaint.nFlags, pSrcImage, pSrcClip ) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 色・透明度設定
	//
	uint32_t argbColor = 0xFFFFFFFF ;
	if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorMul )
	{
		argbColor = ppPaint.rgbColorParam.ui32 | 0xFF000000 ;
	}
	PutGLPaintColor( argbColor, ppPaint.nTransparency ) ;
	//
	// 頂点座標・UV座標設定
	//
	SGLAffine	af ;
	ppPaint.GetAffine( af ) ;
	//
	size_t	countVertex ;
	if ( ppPaint.nFlags & paintPolygonShaped )
	{
		if ( (ppPaint.pVertices == NULL)
			| (ppPaint.countVertex < 3) )
		{
			return	sglErrInvalidParam ;
		}
		if ( !PutVertex2D
			( ppPaint.pVertices, ppPaint.countVertex,
				ppPaint.zOrder, ppPaint.nFlags, &af ) )
		{
			return	sglErrSuccess ;
		}
		PutTextureUVMap
			( ppPaint.pVertices, ppPaint.countVertex, afUV ) ;
		countVertex = ppPaint.countVertex ;
	}
	else
	{
		S2DVector	vRect[4] ;
		if ( pSrcClip != NULL )
		{
			vRect[0].x = (float32_t) pSrcClip->x ;
			vRect[0].y = (float32_t) pSrcClip->y ;
			vRect[1].x = (float32_t) pSrcClip->x ;
			vRect[1].y = (float32_t) (pSrcClip->y + pSrcClip->h) ;
			vRect[2].x = (float32_t) (pSrcClip->x + pSrcClip->w) ;
			vRect[2].y = (float32_t) (pSrcClip->y + pSrcClip->h) ;
			vRect[3].x = (float32_t) (pSrcClip->x + pSrcClip->w) ;
			vRect[3].y = (float32_t) pSrcClip->y ;
		}
		else
		{
			SGLSize	sizeImage = pSrcImage->GetImageSize() ;
			vRect[0].x = 0.0f ;
			vRect[0].y = 0.0f ;
			vRect[1].x = 0.0f ;
			vRect[1].y = (float32_t) sizeImage.h ;
			vRect[2].x = (float32_t) sizeImage.w ;
			vRect[2].y = (float32_t) sizeImage.h ;
			vRect[3].x = (float32_t) sizeImage.w ;
			vRect[3].y = 0.0f ;
		}
		if ( !PutVertex2D( vRect, 4, ppPaint.zOrder, ppPaint.nFlags, &af ) )
		{
			return	sglErrSuccess ;
		}
		PutTextureUVMap( vRect, 4, afUV ) ;
		countVertex = 4 ;
	}
	//
	// 描画
	//
	glDrawArrays( GL_TRIANGLE_FAN, 0, (GLsizei) countVertex ) ;
	SGLOpenGLContext::VerifyError( "glDrawArrays(GL_TRIANGLE_FAN)" ) ;

	FlushVertexPointers() ;
	m_flagUpdate = true ;

	return	sglErrSuccess ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	size_t	countPolygon = widthMesh * heightMesh * 2 ;
	if ( (pDstMesh == NULL) | (countPolygon == 0) )
	{
		return	sglErrInvalidParam ;
	}
	FlushGLMaterial() ;
	//
	// 画像設定
	//
	SGLAffine	afUV ;
	if ( !PutTexture2D( afUV, ppPaint.nFlags, pSrcImage, pSrcClip ) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 色・透明度設定
	//
	uint32_t argbColor = 0xFFFFFFFF ;
	if ( (ppPaint.nFlags & paintMaskApply) == paintApplyColorMul )
	{
		argbColor = ppPaint.rgbColorParam.ui32 | 0xFF000000 ;
	}
	PutGLPaintColor( argbColor, ppPaint.nTransparency ) ;
	//
	// 頂点座標・UV座標設定
	//
	size_t	countVertex = (widthMesh + 1) * (heightMesh + 1) ;
	if ( pSrcMesh == NULL )
	{
		// デフォルトのメッシュ生成
		if ( m_bufMeshUV.GetLength() < countVertex )
		{
			m_bufMeshUV.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
		}
		S2DVector *	pvMeshUV = m_bufMeshUV.GetArray() ;
		float32_t	xStep, yStep ;
		if ( pSrcClip != NULL )
		{
			xStep = (float32_t) pSrcClip->w / widthMesh ;
			yStep = (float32_t) pSrcClip->h / heightMesh ;
		}
		else
		{
			SGLSize	sizeImage = pSrcImage->GetImageSize() ;
			xStep = (float32_t) sizeImage.w / widthMesh ;
			yStep = (float32_t) sizeImage.h / heightMesh ;
		}
		for ( size_t y = 0; y <= heightMesh; y ++ )
		{
			for ( size_t x = 0; x <= widthMesh; x ++ )
			{
				pvMeshUV->x = (float32_t) x * xStep ;
				pvMeshUV->y = (float32_t) y * yStep ;
				pvMeshUV ++ ;
			}
		}
		m_bufMeshUV.FinishArray() ;
		pSrcMesh = m_bufMeshUV.GetArrayPtr() ;
	}
	SGLAffine	af ;
	ppPaint.GetAffine( af ) ;
	//
	if ( !PutVertex2D
		( pDstMesh, countVertex,
			ppPaint.zOrder, ppPaint.nFlags, &af ) )
	{
		return	sglErrSuccess ;
	}
	PutTextureUVMap( pSrcMesh, countVertex, afUV ) ;
	//
	// 描画
	//
	size_t	countIndex = countPolygon * 3 ;
	if ( m_bufIndexed.GetLength() < countIndex )
	{
		m_bufIndexed.SetLength( (countIndex + 0xFF) & ~0xFF ) ;
	}
	#if	defined(__API_OPEN_GL_ES__)
		typedef	uint16_t	TYPE_INDEX ;
	#else
		typedef	uint32_t	TYPE_INDEX ;
	#endif
	TYPE_INDEX *	pIndexed = m_bufIndexed.GetArray() ;
	for ( size_t y = 0; y < heightMesh; y ++ )
	{
		TYPE_INDEX	indexLine0 = (TYPE_INDEX) (y * (widthMesh + 1)) ;
		TYPE_INDEX	indexLine1 = (TYPE_INDEX) (indexLine0 + (widthMesh + 1)) ;
		for ( size_t x = 0; x < widthMesh; x ++ )
		{
			pIndexed[0] = indexLine0 + (TYPE_INDEX) x ;
			pIndexed[1] = indexLine1 + (TYPE_INDEX) x ;
			pIndexed[2] = indexLine1 + (TYPE_INDEX) x + 1 ;
			pIndexed[3] = indexLine0 + (TYPE_INDEX) x ;
			pIndexed[4] = indexLine1 + (TYPE_INDEX) x + 1 ;
			pIndexed[5] = indexLine0 + (TYPE_INDEX) x + 1 ;
			pIndexed += 6 ;
		}
	}
	m_bufIndexed.FinishArray() ;
	//
	#if	defined(__API_OPEN_GL_ES__)
		glDrawElements
			( GL_TRIANGLES, countIndex,
				GL_UNSIGNED_SHORT, m_bufIndexed.GetConstArray() ) ;
	#else
		glDrawElements
			( GL_TRIANGLES, (GLsizei) countIndex,
				GL_UNSIGNED_INT, m_bufIndexed.GetConstArray() ) ;
	#endif
	SGLOpenGLContext::VerifyError( "glDrawElements(GL_TRIANGLES)" ) ;

	FlushVertexPointers() ;
	m_flagUpdate = true ;

	return	sglErrSuccess ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::Flush( void )
{
	/*
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->OnFlushContext() ;
	}
	else
	{
		FlushGLMaterial() ;
	}
	*/
	glFlush() ;
	SGLOpenGLContext::VerifyError( "glFlush" ) ;

	return	sglErrSuccess ;
}

SGLError S3DOpenGLDirectlyRenderer::Finish( void )
{
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->OnFlushContext() ;
	}
	else
	{
		FlushGLMaterial() ;
	}
	glFlush() ;
	SGLOpenGLContext::VerifyError( "glFlush" ) ;

	if ( m_flagUpdate )
	{
		if ( m_flagMirrorFBO )
		{
			m_glFrameBuffer.BlitFramebufferTo
				( m_glMirrorFBO,
					m_rectMirrorSrcFBO.x, m_rectMirrorSrcFBO.x,
					m_rectMirrorSrcFBO.x + m_rectMirrorSrcFBO.w,
					m_rectMirrorSrcFBO.y + m_rectMirrorSrcFBO.h,
					m_rectMirrorDstFBO.x, m_rectMirrorDstFBO.y,
					m_rectMirrorDstFBO.x + m_rectMirrorDstFBO.w,
					m_rectMirrorDstFBO.y + m_rectMirrorDstFBO.h ) ;
		}
		if ( m_pglColor != NULL )
		{
			ESLAssert( m_pTarget != NULL ) ;
			m_pTarget->ReflectImageObject( imageObjectGLTexture ) ;
			//
			if ( m_pZBuffer != NULL )
			{
				m_pZBuffer->ReflectImageObject( imageObjectGLTexture ) ;
			}
		}
		m_flagUpdate = false ;
	}

	return	sglErrSuccess ;
}

// 頂点の設定をクリアする
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::FlushVertexPointers( void )
{
	if ( m_pglDefShader != NULL )
	{
		if ( m_flagIn3DRenderer )
		{
//			PutCameraViewMatrix() ;
		}
		m_pglDefShader->DisableAllVertexPointer() ;
	}
	else
	{
		m_glRenderer.FlushGLMaterial() ;
	}
}

// マテリアル設定をクリアする
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::FlushGLMaterial( void )
{
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->SetMaterial( NULL ) ;
	}
	else
	{
		m_glRenderer.FlushGLMaterial() ;
	}
	m_flagDrawImage = false ;
}

// 描画フラグに応じた OpenGL の設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::PutGLByPaintFlags
		( SGLOpenGLContext * pOpenGL, uint32_t flags )
{
	//
	// 描画方法
	//
	SGLOpenGLContext::BlendMode	modeBlend = SGLOpenGLContext::blendProducted ;
	GLenum	sfactor = GL_ONE, dfactor = GL_ONE_MINUS_SRC_ALPHA ;
	if ( (flags & paintMaskApply) == paintApplyAlphaMul )
	{
		sfactor = GL_SRC_ALPHA ;
		modeBlend = SGLOpenGLContext::blendUnproducted ;
	}
	if ( (flags & paintMaskFunction) != 0 )
	{
		switch ( flags & paintMaskFunction )
		{
		case	paintFunctionAdd:
			dfactor = GL_ONE ;
			modeBlend = SGLOpenGLContext::blendAdd ;
			break ;
		case	paintFunctionMul:
			sfactor = GL_ZERO ;
			dfactor = GL_SRC_COLOR ;
			modeBlend = SGLOpenGLContext::blendMulColor ;
			break ;
		case	paintFunctionMove:
			dfactor = GL_ZERO ;
			sfactor = GL_ONE ;
			modeBlend = SGLOpenGLContext::blendCopy ;
			break ;
		}
	}
	if ( pOpenGL->m_modeBlend != modeBlend )
	{
		glEnable( GL_BLEND ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_BLEND)" ) ;
		//
		glBlendFunc( sfactor, dfactor ) ;
		SGLOpenGLContext::VerifyError( "glBlendFunc" ) ;
		pOpenGL->m_modeBlend = modeBlend ;
	}
	//
	// 両面ポリゴン
	//
	if ( !pOpenGL->m_faceDoubleSide )
	{
		glDisable( GL_CULL_FACE ) ;
		SGLOpenGLContext::VerifyError( "glDisable(GL_CULL_FACE)" ) ;
		pOpenGL->m_faceDoubleSide = true ;
	}
	//
	// αテスト
	//
	#if	!defined(__API_OPEN_GL_ES__)
	/*
	if ( !pOpenGL->m_alphaTestZeroClip )
	{
		glAlphaFunc( GL_GEQUAL, 0.0078125 ) ;
		SGLOpenGLContext::VerifyError( "glAlphaFunc" ) ;
		glEnable( GL_ALPHA_TEST ) ;
		SGLOpenGLContext::VerifyError( "glEnable(GL_ALPHA_TEST)" ) ;
		pOpenGL->m_alphaTestZeroClip = true ;
	}
	*/
	#endif
	//
	// ｚ比較
	//
	if ( flags & paintWithZOrder )
	{
		if ( flags & paintWithZOrderNoWrite )
		{
			if ( !(pOpenGL->m_funcDepthTest & shadingZBufferNoWrite) )
			{
				glEnable( GL_DEPTH_TEST ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
				glDepthFunc( GL_LEQUAL );
				SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
				glDepthMask( GL_FALSE ) ;
				SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
				pOpenGL->m_funcDepthTest = shadingZBufferNoWrite ;
			}
		}
		else
		{
			if ( pOpenGL->m_funcDepthTest != 0 )
			{
				glEnable( GL_DEPTH_TEST ) ;
				SGLOpenGLContext::VerifyError( "glEnable(GL_DEPTH_TEST)" ) ;
				glDepthFunc( GL_LEQUAL );
				SGLOpenGLContext::VerifyError( "glDepthFunc(GL_LEQUAL)" ) ;
				glDepthMask( GL_TRUE ) ;
				SGLOpenGLContext::VerifyError( "glDepthMask(GL_TRUE)" ) ;
				pOpenGL->m_funcDepthTest = 0 ;
			}
		}
	}
	else
	{
		if ( !(pOpenGL->m_funcDepthTest & shadingNoZBuffer) )
		{
			glDisable( GL_DEPTH_TEST ) ;
			SGLOpenGLContext::VerifyError( "glDisable(GL_DEPTH_TEST)" ) ;
			glDepthMask( GL_FALSE ) ;
			SGLOpenGLContext::VerifyError( "glDepthMask(GL_FALSE)" ) ;
			pOpenGL->m_funcDepthTest = shadingNoZBuffer ;
		}
	}
}

// 描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::PutGLPaintColor
			( uint32_t argb, unsigned int nTransparency )
{
	if ( m_pglDefShader != NULL )
	{
		S3DColor	colorPaint ;
		if ( m_flagDrawImage )
		{
			colorPaint.rgbMul.ui32 = argb & 0x00FFFFFF ;
			colorPaint.rgbAdd.ui32 = 0 ;
		}
		else
		{
			colorPaint.rgbMul.argb.Red = (~argb >> 24) & 0xFF ;
			colorPaint.rgbMul.argb.Green = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbMul.argb.Blue = colorPaint.rgbMul.argb.Red ;
			colorPaint.rgbAdd.ui32 = argb & 0x00FFFFFF ;
		}
		S3DColor	colorEffect ;
		if ( GetColorEffect( colorEffect ) )
		{
			colorPaint *= colorEffect ;
		}
		nTransparency = EffectTransparency( nTransparency ) ;
		//
		m_pglDefShader->SetColorEffect( &colorPaint, nTransparency ) ;
	}
	else
	{
		SGLPalette	color ;
		color.ui32 = argb ;
		nTransparency = EffectTransparency( nTransparency ) ;
		//
		GLfloat	scaleBy255 =
			(GLfloat) (0x100 - nTransparency) / (256.0f * 255.0f) ;
		glColor4f
			( (GLfloat) color.argb.Red * scaleBy255,
				(GLfloat) color.argb.Green * scaleBy255,
				(GLfloat) color.argb.Blue * scaleBy255,
				(GLfloat) color.argb.Alpha * scaleBy255 ) ;
		SGLOpenGLContext::VerifyError( "glColor4f" ) ;
	}
}

// 頂点座標正規化
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::PutVertex2D
	( const S2DVector * vertices,
		size_t count, double z,
		uint32_t flags, const SGLAffine * pAffine )
{
	//
	// ｚ座標正規化
	//
	if ( flags & paintWithZOrder )
	{
		if ( !m_glView.IsOrthogonalProjection() )
		{
			if ( z < 1.0f )
			{
				return	false ;
			}
		}
	}
	else
	{
		if ( !m_glView.IsOrthogonalProjection() )
		{
			z = m_vProjectionScreen.z ;
		}
		else
		{
			z = 0.0f ;
		}
	}
	//
	// 頂点バッファ生成
	//
	if ( m_bufVertex.GetLength() < count )
	{
		m_bufVertex.SetLength( (count + 0x0F) & ~0x0F ) ;
	}
	S3DVector4 *	pvBufVertex = m_bufVertex.GetArray() ;
	//
	SGLAffine	af ;
	GetAffineTransformation( af ) ;
	if ( pAffine != NULL )
	{
		af *= *pAffine ;
	}
	for ( size_t i = 0; i < count; i ++ )
	{
		float32_t	x = vertices[i].x ;
		float32_t	y = vertices[i].y ;
		pvBufVertex[i].x = af.a11 * x + af.a12 * y + af.a13 ;
		pvBufVertex[i].y = af.a21 * x + af.a22 * y + af.a23 ;
		pvBufVertex[i].z = (float32_t) z ;
	}
	if ( !m_glView.IsOrthogonalProjection() )
	{
		float32_t	xs = m_vProjectionScreen.x ;
		float32_t	ys = m_vProjectionScreen.y ;
		float32_t	r = (m_vProjectionScreen.z != 0.0f)
						? ((float32_t) z / m_vProjectionScreen.z) : 1.0f ;
		for ( size_t i = 0; i < count; i ++ )
		{
			pvBufVertex[i].x = (pvBufVertex[i].x - xs) * r ;
			pvBufVertex[i].y = (pvBufVertex[i].y - ys) * r ;
		}
	}
	//
	// 頂点設定
	//
	if ( m_pglDefShader != NULL )
	{
		S4DMatrix	matI( 1, 1, 1, 1 ) ;
		m_pglDefShader->SetCameraViewMatrix( matI ) ;
		m_pglDefShader->SetModelViewMatrix( matI ) ;
		m_pglDefShader->SetVertexPointer( pvBufVertex ) ;
		//
		m_pglDefShader->SetColorPointer
			( m_pglDefShader->AllocateDummyVertexColorBuffer( count ) ) ;
	}
	else
	{
		glEnableClientState( GL_VERTEX_ARRAY ) ;
		SGLOpenGLContext::VerifyError( "glEnableClientState(GL_VERTEX_ARRAY)" ) ;
		glVertexPointer( 3, GL_FLOAT, sizeof(S3DVector4), pvBufVertex ) ;
		SGLOpenGLContext::VerifyError( "glVertexPointer" ) ;
	}
	m_bufVertex.FinishArray() ;
	return	true ;
}

// テクスチャ設定
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::PutTexture2D
	( SGLAffine& afUV, uint32_t flags,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( m_pglDefShader != NULL )
	{
		uint64_t	flagsShading = shadingTextureMapping ;
		if ( ((flags | m_flagsDefPaint) & paintSmoothStretch)
				&& !((flags | m_flagsDefPaint) & paintUnsmoothStretch) )
		{
			flagsShading |= shadingTextureSmoothing ;
		}
		if ( m_pglDefShader->BindTexture
			( SGLOpenGLDefaultShader::glTextureDefault,
				pSrcImage, pSrcClip, flagsShading ) == NULL )
		{
			return	false ;
		}
		m_pglDefShader->BindLuminousTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL, 0, 0.0f ) ;
		m_pglDefShader->BindEnvironmentTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL,
				SGLOpenGLDefaultShader::ENV_MAPPING_NOTHING, false, NULL ) ;
		m_pglDefShader->BindRefractionTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL,
				SGLOpenGLDefaultShader::ENV_MAPPING_NOTHING, false, NULL, 0.0f ) ;
		m_pglDefShader->BindNormalTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL, 0, 0.0f ) ;
		m_pglDefShader->BindHeightTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL, 0, 0.0f ) ;
		m_pglDefShader->BindAlphaTexture
			( SGLOpenGLDefaultShader::glTextureMaterial0, NULL, NULL, 0, 1.0f, 0.0f ) ;
		//
		afUV.a11 = 1.0f ;
		afUV.a12 = 0.0f ;
		afUV.a13 = 0.0f ;
		afUV.a21 = 0.0f ;
		afUV.a22 = 1.0f ;
		afUV.a23 = 0.0f ;
	}
	else
	{
		if ( m_glRenderer.BindGLTexture( pSrcImage ) == NULL )
		{
			return	false ;
		}
		m_glRenderer.PutGLTextureSmoothing
			( ((flags | m_flagsDefPaint) & paintSmoothStretch)
				&& !((flags | m_flagsDefPaint) & paintUnsmoothStretch) ) ;
		m_glRenderer.PutGLTextureTiling( false ) ;
		//
		afUV = m_glRenderer.GetAffineForTextureUV() ;
		if ( pSrcClip != NULL )
		{
			afUV.a13 += (float32_t) pSrcClip->x ;
			afUV.a23 += (float32_t) pSrcClip->y ;
		}
		if ( m_glRenderer.IsNeedsMulAlpha() )
		{
			if ( (flags & paintMaskApply) == 0 )
			{
				flags |= paintApplyAlphaMul ;
			}
		}
		if ( m_glRenderer.IsTextureWithoutAlpha() )
		{
			if ( (flags & paintMaskFunction) == 0 )
			{
				flags |= paintFunctionMove ;
			}
		}
	}
	m_flagDrawImage = true ;
	PutGLByPaintFlags( pOpenGL, flags ) ;
	return	true ;
}

// UV座標設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::PutTextureUVMap
	( const S2DVector * pvUVMap,
		size_t count, const SGLAffine& afUV )
{
	//
	// 頂点設定
	//
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->SetTexCoordPointer( pvUVMap ) ;
	}
	else
	{
		//
		// UV 座標バッファ生成
		//
		if ( m_bufUVMap.GetLength() < count )
		{
			m_bufUVMap.SetLength( (count + 0x0F) & ~0x0F ) ;
		}
		S2DVector *	pvBufUVMap = m_bufUVMap.GetArray() ;
		//
		afUV.TransformVectors( pvBufUVMap, pvUVMap, count ) ;
		//
		// ポインタ設定
		//
		glEnableClientState( GL_TEXTURE_COORD_ARRAY ) ;
		SGLOpenGLContext::VerifyError
			( "glEnableClientState(GL_TEXTURE_COORD_ARRAY)" ) ;
		glTexCoordPointer( 2, GL_FLOAT, 0, pvBufUVMap ) ;
		SGLOpenGLContext::VerifyError( "glTexCoordPointer" ) ;
		//
		m_bufUVMap.FinishArray() ;
	}
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	S3DOpenGLDirectlyRenderer *	pogldRender =
			ESLTypeCast<S3DOpenGLDirectlyRenderer>( &renderSrc ) ;
	if ( pogldRender == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	GLuint	glDstFBO = m_glFrameBuffer.GetFrameBufferOGL() ;
	GLuint	glSrcFBO = pogldRender->m_glFrameBuffer.GetFrameBufferOGL() ;
	if ( (glDstFBO == 0) || (glSrcFBO == 0) )
	{
		return	sglErrFailed ;
	}
	SGLImageObject *	pSrcImage = renderSrc.GetTargetImage() ;
	SGLImageObject *	pDstImage = GetTargetImage() ;
	if ( (pSrcImage == nullptr) || (pDstImage == nullptr) )
	{
		return	sglErrFailed ;
	}
	//
	// 対象バッファ
	//
	GLbitfield	mask = 0 ;
	if ( nFlags == 0 )
	{
		mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ;
	}
	else
	{
		if ( nFlags & copyBufferColor )
		{
			mask |= GL_COLOR_BUFFER_BIT ;
		}
		if ( nFlags & copyBufferDepth )
		{
			mask |= GL_DEPTH_BUFFER_BIT ;
		}
	}
	if ( mask == 0 )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 対象領域の正規化
	//
	SGLSize			sizeSrc = pSrcImage->GetImageSize() ;
	SGLImageRect	rectSrc( 0, 0, sizeSrc.w, sizeSrc.h ) ;
	if ( pSrcRect != nullptr )
	{
		rectSrc = *pSrcRect ;
		if ( rectSrc.x < 0 )
		{
			xDst -= rectSrc.x ;
			rectSrc.x = 0 ;
		}
		if ( rectSrc.y < 0 )
		{
			yDst -= rectSrc.y ;
			rectSrc.y = 0 ;
		}
		if ( rectSrc.x + rectSrc.w > sizeSrc.w )
		{
			rectSrc.w = sizeSrc.w - rectSrc.x ;
		}
		if ( rectSrc.y + rectSrc.h > sizeSrc.h )
		{
			rectSrc.h = sizeSrc.h - rectSrc.y ;
		}
	}
	if ( xDst < 0 )
	{
		rectSrc.x -= xDst ;
		rectSrc.w += xDst ;
		xDst = 0 ;
	}
	if ( yDst < 0 )
	{
		rectSrc.y -= yDst ;
		rectSrc.h += yDst ;
		yDst = 0 ;
	}
	SGLSize			sizeDst = pDstImage->GetImageSize() ;
	SGLImageRect	rectDst( xDst, yDst, sizeDst.w - xDst, sizeDst.h - yDst ) ;
	//
	if ( rectSrc.IsEmpty() || rectDst.IsEmpty() )
	{
		return	sglErrInvalidParam ;
	}
	rectDst.w = (int32_t) esl_min( rectDst.w, rectSrc.w ) ;
	rectDst.h = (int32_t) esl_min( rectDst.h, rectSrc.h ) ;
	rectSrc.w = rectDst.w ;
	rectSrc.h = rectDst.h ;
	//
	// マルチレンダーターゲット判定
	//
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	ESLAssert( pOpenGL != nullptr ) ;
	//
	bool	flagMultiTargets = false ;
	if ( mask & GL_COLOR_BUFFER_BIT )
	{
		size_t	nMultiTargets = 0 ;
		size_t	nSrcMultiTargets = 0 ;
		if ( (GetMultiTargetImages( nMultiTargets ) != nullptr)
			&& (pogldRender->GetMultiTargetImages( nSrcMultiTargets ) != nullptr) )
		{
			if ( (nMultiTargets > 0) && (nSrcMultiTargets > 0) )
			{
				pOpenGL->AttachFrameBuffer( &m_glFrameBuffer ) ;
				m_glFrameBuffer.DetachMultiTarget() ;
				pogldRender->m_glFrameBuffer.DetachMultiTarget() ;
				flagMultiTargets = true ;
			}
		}
	}
	//
	// 転送
	//
	if ( m_pglColor == pogldRender->m_pglColor )
	{
		mask &= ~GL_COLOR_BUFFER_BIT ;
	}
	if ( m_pglDepth == pogldRender->m_pglDepth )
	{
		mask &= ~GL_DEPTH_BUFFER_BIT ;
	}
	if ( mask != 0 )
	{
		glBindFramebuffer( GL_READ_FRAMEBUFFER, glSrcFBO ) ;
		SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_READ_FRAMEBUFFER)" ) ;
		//
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, glDstFBO ) ;
		SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_DRAW_FRAMEBUFFER)" ) ;
		//
		glBlitFramebuffer
			( rectSrc.x, rectSrc.y, rectSrc.x + rectSrc.w, rectSrc.y + rectSrc.h,
				rectDst.x, rectDst.y, rectDst.x + rectDst.w, rectDst.y + rectDst.h,
				mask,  GL_NEAREST ) ;
		SGLOpenGLContext::VerifyError( "glBlitFramebuffer()" ) ;
	}
	//
	// マルチレンダーターゲット転送
	//
	if ( flagMultiTargets )
	{
		size_t	nDstMultiTargets = 0 ;
		size_t	nSrcMultiTargets = 0 ;
		SGLImageObject *const*
				ppDstTargets = GetMultiTargetImages( nDstMultiTargets ) ;
		SGLImageObject *const*
				ppSrcTargets = pogldRender->GetMultiTargetImages( nSrcMultiTargets ) ;
		//
		for ( size_t i = 0; (i < nDstMultiTargets) && (i < nSrcMultiTargets); i ++ )
		{
			if ( (ppDstTargets[i] == nullptr)
				|| (ppSrcTargets[i] == nullptr) )
			{
				continue ;
			}
			SGLImageRect	rectRefSrcImage, rectRefDstImage ;
			SGLOpenGLTextureBuffer *
				pglSrcColor = SGLOpenGLTextureBuffer::CommitGLTexture
									( pOpenGL, ppSrcTargets[i], rectRefSrcImage ) ;
			SGLOpenGLTextureBuffer *
				pglDstColor = SGLOpenGLTextureBuffer::CommitGLTexture
									( pOpenGL, ppDstTargets[i], rectRefDstImage ) ;
			if ( (pglSrcColor != nullptr)
				&& (pglDstColor != nullptr)
				&& (pglSrcColor != pglDstColor) )
			{
				pogldRender->m_glFrameBuffer.AttachFrameBuffer( pglSrcColor, nullptr ) ;
				m_glFrameBuffer.AttachFrameBuffer( pglDstColor, nullptr ) ;
				//
				glBindFramebuffer( GL_READ_FRAMEBUFFER, glSrcFBO ) ;
				SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_READ_FRAMEBUFFER)" ) ;
				//
				glBindFramebuffer( GL_DRAW_FRAMEBUFFER, glDstFBO ) ;
				SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_DRAW_FRAMEBUFFER)" ) ;
				//
				glBlitFramebuffer
					( rectSrc.x, rectSrc.y, rectSrc.x + rectSrc.w, rectSrc.y + rectSrc.h,
						rectDst.x, rectDst.y, rectDst.x + rectDst.w, rectDst.y + rectDst.h,
						GL_COLOR_BUFFER_BIT,  GL_NEAREST ) ;
				SGLOpenGLContext::VerifyError( "glBlitFramebuffer()" ) ;
			}
		}
		pogldRender->ReflectRenderTarget() ;
		ReflectRenderTarget() ;
	}
	//
	// 後始末
	//
	glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_READ_FRAMEBUFFER,0)" ) ;
	//
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) ;
	SGLOpenGLContext::VerifyError( "GL_READ_FRAMEBUFFER(GL_DRAW_FRAMEBUFFER,0)" ) ;
	//
	pOpenGL->AttachFrameBuffer( &m_glFrameBuffer ) ;
	//
	return	sglErrSuccess ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AttachCustomShader( S3DCustomShader * pShader )
{
	SGLOpenGLCustomShader *
		pglShader = ESLTypeCast<SGLOpenGLCustomShader>( pShader ) ;
	AttachGLCustomShader( pglShader ) ;
	return	sglErrSuccess ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DOpenGLDirectlyRenderer::GetCustomShader( void ) const
{
	return	m_pglCustomShader ;
}

// カスタムシェーダーパラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::SetCustomShaderUniform
	( const wchar_t * pwszUniformId,
			S3DCustomShader::UniformType type,
			const void * pData, size_t nCount )
{
	if ( m_pglCustomShader == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err ;
	m_pglCustomShader->LockUniform() ;
	err = m_pglCustomShader->SetCustomUniformAs
						( pwszUniformId, type, pData, nCount ) ;
	m_pglCustomShader->UnlockUniform() ;
	return	err ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	SGLError	err =
		S3DRenderParameterContext::SetProjectionScreen
							( vScreen, zScale, fpPixelAspect ) ;
	if ( m_flagIn3DRenderer )
	{
		UpdateGLPerspectiveProjection() ;
	}
	return	err ;
}

// 透視変換行列取得
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::GetPerspectiveMatrix( S4DMatrix& matPers ) const
{
	if ( !m_glView.m_flagMatrixPers )
	{
		return	S3DRenderParameterContext::GetPerspectiveMatrix( matPers ) ;
	}
	matPers = m_glView.m_matPerspective ;
	return	true ;
}

// 透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetPerspectiveMatrix
	( StereoViewIndex sviView, const S4DMatrix& matPers, bool fPersMatrix )
{
	S3DRenderParameterContext::SetPerspectiveMatrix( sviView, matPers, fPersMatrix ) ;
}

void S3DOpenGLDirectlyRenderer::EnablePerspectiveMatrix( bool fPersMatrix )
{
	S3DRenderParameterContext::EnablePerspectiveMatrix( fPersMatrix ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	S3DRenderParameterContext::SetCamera( matCamera, posCamera ) ;
	if ( m_flagIn3DRenderer )
	{
		PutCameraViewMatrix() ;
	}
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetZClipRange( double zMin, double zMax )
{
	S3DRenderParameterContext::SetZClipRange( zMin, zMax ) ;
	if ( m_flagIn3DRenderer )
	{
		UpdateGLPerspectiveProjection() ;
	}
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	S3DRenderParameterContext::SetLightEntries( pLights, countLight ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectLighting() ;
	}
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMapDepth,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	S3DRenderParameterContext::SetShadowMap
		( idLight, pShadowMapDepth, infShadowMap, pShadowMapColor ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectLighting() ;
	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	S3DRenderParameterContext::SetFog( rgbFog, zFogNear, zFogFar ) ;
	if ( m_flagIn3DRenderer && m_flagFog )
	{
		ReflectLighting() ;
	}
}

void S3DOpenGLDirectlyRenderer::EnableFog( bool fFog )
{
	S3DRenderParameterContext::EnableFog( fFog ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectLighting() ;
	}
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetShadingFlag( uint64_t nShadingMethod )
{
	S3DRenderParameterContext::SetShadingFlag( nShadingMethod ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectShadingFlags() ;
	}
}

// グローバル環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetEnvironmentMappingImage
			( SGLImageObject * pImage, uint32_t nFlags )
{
	S3DRenderParameterContext::SetEnvironmentMappingImage( pImage, nFlags ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectEnvironmentMapping() ;
	}
}

// グローバル環境マッピング変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetEnvironmentMappingMatrix( const S3DMatrix& matMapping )
{
	S3DRenderParameterContext::SetEnvironmentMappingMatrix( matMapping ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectEnvironmentMapping() ;
	}
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetOffsetBorderColor( uint32_t rgbBorder )
{
	S3DRenderParameterContext::SetOffsetBorderColor( rgbBorder ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectOffsetBorder() ;
	}
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	S3DRenderParameterContext::SetOffsetBorderCoefficient( a, b ) ;
	if ( m_flagIn3DRenderer )
	{
		ReflectOffsetBorder() ;
	}
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::SetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
			int32_t nParam1, const void * pParam2, size_t sizeOfParam2 )
{
	SGLError	err =
		S3DRenderParameterContext::SetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
	if ( err )
	{
		return	err ;
	}
	switch ( feature )
	{
	case	featureContextSet:
		{
			SGLOpenGLCustomShader *	pglShader =
				ESLTypeCast<SGLOpenGLCustomShader>( m_optContext.pShader ) ;
			AttachGLCustomShader( pglShader ) ;
			//
			if ( m_flagIn3DRenderer )
			{
				ReflectOffsetBorder() ;
			}
		}
		if ( m_pglDefShader != NULL )
		{
			m_pglDefShader->SetFaceCullingOperation( m_optContext.faceCulling ) ;
			m_pglDefShader->SetDepthMaskOperation( m_optContext.depthMask ) ;
			m_pglDefShader->SetBlendOperation( m_optContext.blendOp ) ;
			m_pglDefShader->SetAnisotropy( m_optContext.anisotropy ) ;
		}
		#if	!defined(__API_OPEN_GL_ES__)
		glPointSize( m_optContext.pointSize ) ;
		glLineWidth( m_optContext.lineWidth ) ;
		#endif
		break ;

	case	featureEnvMap:
		if ( m_flagIn3DRenderer )
		{
			ReflectEnvironmentMapping() ;
		}
		break ;

	case	featureOffsetBorder:
		if ( m_flagIn3DRenderer )
		{
			ReflectOffsetBorder() ;
		}
		break ;

	case	featureFaceCulling:
		if ( m_pglDefShader != NULL )
		{
			m_pglDefShader->SetFaceCullingOperation
				( (FaceCullingOperation) nParam1 ) ;
		}
		break ;

	case	featureDepthMask:
		if ( m_pglDefShader != NULL )
		{
			m_pglDefShader->SetDepthMaskOperation
				( (DepthMaskOperation) nParam1 ) ;
		}
		break ;

	case	featureBlendOperation:
		if ( m_pglDefShader != NULL )
		{
			m_pglDefShader->SetBlendOperation
				( (BlendOperation) nParam1 ) ;
		}
		break ;

	case	featurePointSize:
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glPointSize( (GLfloat) *((float32_t*)pParam2) ) ;
			#endif
		}
		break ;

	case	featureLineWidth:
		if ( sizeOfParam2 == sizeof(float32_t) )
		{
			#if	!defined(__API_OPEN_GL_ES__)
			glLineWidth( (GLfloat) *((float32_t*)pParam2) ) ;
			#endif
		}
		break ;

	case	featureAnisotropy:
		if ( (sizeOfParam2 == sizeof(float32_t))
			&& (m_pglDefShader != NULL) )
		{
			m_pglDefShader->SetAnisotropy( (GLfloat) *((float32_t*)pParam2) ) ;
		}
		break ;

	case	featureSRGB:
		#if	!defined(__API_OPEN_GL_ES__)
		if ( nParam1 )
		{
			glEnable( GL_FRAMEBUFFER_SRGB ) ;
			SGLOpenGLContext::VerifyError( "glEnable(GL_FRAMEBUFFER_SRGB)" ) ;
		}
		else
		{
			glDisable( GL_FRAMEBUFFER_SRGB ) ;
			SGLOpenGLContext::VerifyError( "glDisable(GL_FRAMEBUFFER_SRGB)" ) ;
		}
		#endif
		break ;

	default:
		return	sglErrNotSupported ;
	}
	return	sglErrSuccess ;
}

// 立体視用バッファ選択
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::SelectParallaxView
			( S3DRenderContextInterface::StereoViewIndex sviView )
{
	if ( m_fStereoView && (m_sviView != sviView) )
	{
		Finish() ;
	}
	SGLError	err =
		S3DRenderParameterContext::SelectParallaxView( sviView ) ;
	//
	if ( m_flagMirrorFBO && m_fStereoView )
	{
		if ( sviView != stereoViewAuto )
		{
			ESLAssert( (sviView >= 0) && (sviView < 2) ) ;
			m_glMirrorFBO = m_glMirrorFBOs[sviView] ;
		}
	}
	//
	#if	!defined(__API_OPEN_GL_ES__) || (defined(__PLATFORM_ANDROID__) && (ANDROID_API_LEVEL >= 18))
	switch ( sviView )
	{
	case	stereoViewAuto:
		if ( m_pglColor == NULL )
		{
			glDrawBuffer( GL_BACK ) ;
			SGLOpenGLContext::VerifyError( "glDrawBuffer(GL_BACK)" ) ;
		}
		break ;
	case	stereoViewRight:
		if ( !m_fStereoView || !m_pStereoTarget[0] )
		{
			#if	!defined(__API_OPEN_GL_ES__)
				glDrawBuffer( GL_BACK_RIGHT ) ;
				SGLOpenGLContext::VerifyError( "glDrawBuffer(GL_BACK_RIGHT)" ) ;
			#else
				glDrawBuffer( GL_BACK ) ;
				SGLOpenGLContext::VerifyError( "glDrawBuffer(GL_BACK_RIGHT)" ) ;
			#endif
		}
		break ;
	case	stereoViewLeft:
		if ( !m_fStereoView || !m_pStereoTarget[1] )
		{
			#if	!defined(__API_OPEN_GL_ES__)
				glDrawBuffer( GL_BACK_LEFT ) ;
				SGLOpenGLContext::VerifyError( "glDrawBuffer(GL_BACK_LEFT)" ) ;
			#else
				glDrawBuffer( GL_BACK ) ;
				SGLOpenGLContext::VerifyError( "glDrawBuffer(GL_BACK_LEFT)" ) ;
			#endif
		}
		break ;
	}
	#endif
	return	err ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	eslFillMemory( &caps, 0, sizeof(S3DRenderingCapacity) ) ;
	caps.flagsShading = S3DRenderingCapacity::shadingGouraud ;
	caps.typeHeadware = S3DRenderingCapacity::hardwareOpenGL ;
	//
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL == NULL )
	{
		pOpenGL = SGLOpenGLContext::GetDefault() ;
		if ( pOpenGL == NULL )
		{
			return ;
		}
	}
	GetRenderingCapacityWithOpenGL( caps, pOpenGL ) ;
}

void S3DOpenGLDirectlyRenderer::GetRenderingCapacityWithOpenGL
		( S3DRenderingCapacity& caps, SGLOpenGLContext * pOpenGL )
{
	caps.maxTextureSize = (uint32_t) pOpenGL->m_maxTextureSize ;
	caps.maxTextureUnit = (uint32_t) pOpenGL->m_maxTextureImages ;
	caps.maxLightCount = 8 ;
	caps.maxShadowmapCount = 0 ;
	//
	if ( OpenGLExtension::g_supports_framebuffer_object
						&& pOpenGL->m_flagDepthTexture )
	{
		caps.flagsExtensions1 |= S3DRenderingCapacity::extFramebuffer ;
	}
	if ( pOpenGL->m_flagTextureNonPowerOf2 )
	{
		caps.flagsExtensions1 |= S3DRenderingCapacity::extTextureNonPowerOf2 ;
	}
	if ( OpenGLExtension::g_supports_opengl_1_3
			&& (pOpenGL->m_maxTextureImages >= 2) )
	{
		caps.flagsExtensions1 |= S3DRenderingCapacity::extMultiTexture ;
	}
	if ( OpenGLExtension::g_supports_opengl_1_5 )
	{
		caps.flagsExtensions1 |= S3DRenderingCapacity::extVertexBuffer ;
	}
	if ( OpenGLExtension::g_supports_opengl_2_0 )
	{
		caps.flagsShading &= ~S3DRenderingCapacity::shadingGouraud ;
		//
		SGLOpenGLDefaultShader *	pGouraudShader =
			ESLTypeCast<SGLOpenGLDefaultShader>
				( pOpenGL->GetStandardShaderProgram( shadingMethodGouraud ) ) ;
		if ( pGouraudShader != NULL )
		{
			caps.flagsShading |= S3DRenderingCapacity::shadingGouraud ;
			caps.maxLightCount = (uint32_t) pGouraudShader->m_maxLightCount ;
		}
		//
		SGLOpenGLDefaultShader *	pPhongShader =
			ESLTypeCast<SGLOpenGLDefaultShader>
				( pOpenGL->GetStandardShaderProgram( shadingMethodPhong ) ) ;
		if ( pPhongShader != NULL )
		{
			caps.flagsShading |= S3DRenderingCapacity::shadingPhong ;
			caps.flagsExtensions1 |= S3DRenderingCapacity::extProgramableShader ;
			//
			if ( pOpenGL->m_flagDepthTexture
				&& (pOpenGL->m_maxTextureImages >= 2) )
			{
				caps.flagsShading |= S3DRenderingCapacity::shadingShadowMapping ;
				caps.maxShadowmapCount = (uint32_t) pPhongShader->m_maxShadowmapping ;
			}
			if ( pOpenGL->m_flagSupportedMRT )
			{
				caps.flagsExtensions1 |= S3DRenderingCapacity::extMultiRenderTarget ;
			}
			caps.maxLightCount = (uint32_t) pPhongShader->m_maxLightCount ;
		}
	}
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::Begin3DRenderer( uint64_t nFlags )
{
	SGLError	err =
		S3DRenderParameterContext::Begin3DRenderer( nFlags ) ;
	//
	ReflectShadingFlags() ;
	UpdateGLPerspectiveProjection() ;
	PutCameraViewMatrix() ;
	ReflectLighting() ;
	ReflectEnvironmentMapping() ;
	ReflectOffsetBorder() ;
	m_flagIn3DRenderer = true ;
	//
	return	err ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::End3DRenderer( uint64_t nFlags )
{
	SGLError	err =
		S3DRenderParameterContext::End3DRenderer( nFlags ) ;
	//
	if ( !m_flag3DPriority )
	{
		m_flagIn3DRenderer = false ;
		UpdateGLOrthogonalProjection() ;
	}
	//
	return	err ;
}

// 3D レンダリング用フラグ設定
//（OpenGL スレッド以外からフラグのみを変更する）
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::Set3DRendererFlag( bool flag3D )
{
	m_flagIn3DRenderer = flag3D ;
}

// 3D レンダリング用フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::IsIn3DRenderer( void ) const
{
	return	m_flagIn3DRenderer ;
}

// シェーダー基本設定スイッチング
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::SwitchShaderContext( void )
{
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->InitializeShader() ;
	}
	UpdateGLProjection() ;
	if ( m_flagIn3DRenderer )
	{
		PutCameraViewMatrix() ;
		ReflectLighting() ;
		ReflectEnvironmentMapping() ;
		ReflectOffsetBorder() ;
	}
}

// OpenGL の投影行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::UpdateGLProjection( void )
{
	if ( m_flagIn3DRenderer )
	{
		UpdateGLPerspectiveProjection() ;
	}
	else
	{
		UpdateGLOrthogonalProjection() ;
	}
}

// OpenGL の直行投影行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::UpdateGLOrthogonalProjection( void )
{
	m_glView.SetZBounds
		( (float32_t) -m_zMaxClip, (float32_t) m_zMaxClip, true ) ;
	m_glView.PutOpenGLOrthogonalProjection
		( m_pglDefShader, false /*m_flagProjUpsideDown*/ ) ;
	m_glView.InitializeOpenGLModelView
		( m_pglDefShader, false, m_flagProjUpsideDown, false, false ) ;
}

// OpenGL の透視投影行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::UpdateGLPerspectiveProjection( void )
{
	m_glView.SetZBounds
		( (float32_t) m_zMinClip, (float32_t) m_zMaxClip, true ) ;
	if ( m_flagPersMatrix )
	{
		StereoViewIndex	sviView = m_sviView ;
		if ( sviView == stereoViewAuto )
		{
			sviView = stereoViewRight ;
		}
		m_glView.PutOpenGLPerspectiveMatrix
			( m_pglDefShader,
				m_matPerspective[sviView], m_vProjectionScreen ) ;
		m_glView.InitializeOpenGLModelView
			( m_pglDefShader, false, m_flagProjUpsideDown, false, false ) ;
	}
	else if ( m_vProjectionScreen.z != 0.0f )
	{
		if ( m_flagProjUpsideDown )
		{
			m_glView.PutOpenGLPerspectiveProjection
				( m_pglDefShader,
					m_vProjectionScreen.x,
						m_glView.m_sizeVirtual.h - m_vProjectionScreen.y,
						m_vProjectionScreen.z * m_zProjectionScale,
						m_fpPixelAspectRatio ) ;
		}
		else
		{
			m_glView.PutOpenGLPerspectiveProjection
				( m_pglDefShader,
					m_vProjectionScreen.x, m_vProjectionScreen.y,
						m_vProjectionScreen.z * m_zProjectionScale,
						m_fpPixelAspectRatio ) ;
		}
		m_glView.InitializeOpenGLModelView
			( m_pglDefShader, false, m_flagProjUpsideDown, false, false ) ;
	}
	else
	{
		m_glView.PutOpenGLOrthogonalProjection
			( m_pglDefShader,
				m_vProjectionScreen.x, m_vProjectionScreen.y,
				1.0 / m_zProjectionScale, false /*m_flagProjUpsideDown*/ ) ;
		m_glView.InitializeOpenGLModelView
			( m_pglDefShader, false, m_flagProjUpsideDown, false, false ) ;
	}
}

// シェーディングフラグの設定を SGLOpenGLRenderingContext へ反映
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::ReflectShadingFlags( void )
{
	return	SetShadingFlagsToGLContext( m_optContext.nShadingFlags ) ;
}

// シェーディングフラグを現在のコンテキストに設定
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLDirectlyRenderer::SetShadingFlagsToGLContext( uint64_t nShadingMethod )
{
	SGLOpenGLDefaultShader *	pDefShader = m_pglCustomShader ;
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( (pOpenGL != NULL) && (pDefShader == NULL) )
	{
		pDefShader =
			ESLTypeCast<SGLOpenGLDefaultShader>
				( pOpenGL->GetDefaultShaderProgram( (int) nShadingMethod ) ) ;
	}
	bool	fChangedShader = false ;
	if ( (pDefShader != m_pglDefShader)
		|| (pOpenGL && (pOpenGL->GetCurrentShaderProgram() != pDefShader)) )
	{
		pOpenGL->m_pflog.countSwitchShader ++ ;
		AttachShader( pDefShader ) ;
		UpdateGLProjection() ;
		//
		if ( pDefShader != pOpenGL->GetDefaultShaderProgram(0) )
		{
			if ( m_flagIn3DRenderer )
			{
				PutCameraViewMatrix() ;
				ReflectLighting() ;
				ReflectEnvironmentMapping() ;
				ReflectOffsetBorder() ;
			}
		}
		fChangedShader = true ;
	}
	if ( pDefShader != NULL )
	{
		pDefShader->EnableTextureSmoothing
			( (nShadingMethod & shadingTextureSmoothing) != 0 ) ;
		pDefShader->EnableWriteDepth
			( (nShadingMethod & shadingZBufferNoWrite) == 0 ) ;
		pDefShader->ForceToonShader
			( (nShadingMethod & shadingMethodToon) != 0 ) ;
		pDefShader->ForceBorderShader
			( (nShadingMethod & shadingDrawOffsetBorder) != 0 ) ;
		pDefShader->ForceNoBorderShader
			( (nShadingMethod & shadingNoDrawOffsetBorder) != 0 ) ;
		pDefShader->EnableMeshSurfaceOffset
			( (nShadingMethod & shadingMeshSurfaceOffset) != 0 ) ;
		pDefShader->SetEmisiveTarget
			( (nShadingMethod & shadingEmisiveTarget) != 0 ) ;
	}
	else
	{
		m_glRenderer.EnableLightingByGL
			( (nShadingMethod & shadingMethodOpenGL) != 0 ) ;
		m_glRenderer.EnableTextureSmoothing
			( (nShadingMethod & shadingTextureSmoothing) != 0 ) ;
	}
	return	fChangedShader ;
}

// 光源の設定を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::ReflectLighting( void )
{
	m_arrayBufLights.SetLength( 0 ) ;
	//
	size_t	i, nCount ;
	bool	fTransform = (m_pglDefShader == NULL) ;
	nCount = m_arrayVectorLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry	light = m_arrayVectorLights.At(i) ;
		if ( fTransform )
		{
			light.vecPosition =
				m_matCamera * S3DDVector( light.vecPosition ) - m_vCameraPos ;
			light.vecDirection =
				m_matCamera * S3DDVector( light.vecDirection ) ;
		}
		m_arrayBufLights.Add( light ) ;
	}
	nCount = m_arrayPointLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry	light = m_arrayPointLights.At(i) ;
		if ( fTransform )
		{
			light.vecPosition =
				m_matCamera * S3DDVector( light.vecPosition ) - m_vCameraPos ;
			light.vecDirection =
				m_matCamera * S3DDVector( light.vecDirection ) ;
		}
		m_arrayBufLights.Add( light ) ;
	}
	nCount = m_arrayFogLights.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DLightEntry	light = m_arrayFogLights.At(i) ;
		if ( fTransform )
		{
			light.vecPosition =
				m_matCamera * S3DDVector( light.vecPosition ) - m_vCameraPos ;
			light.vecDirection =
				m_matCamera * S3DDVector( light.vecDirection ) ;
		}
		m_arrayBufLights.Add( light ) ;
	}
	if ( m_rgbAmbient.ui32 & 0x00FFFFFF )
	{
		S3DLightEntry	light ;
		light.typeLight = lightTypeAmbient ;
		light.rgbColor = m_rgbAmbient ;
		light.fpBrightness = 1.0f ;
		m_arrayBufLights.Add( light ) ;
	}
	if ( (m_rgbAmbientMul.ui32 & 0x00FFFFFF) != 0x00FFFFFF )
	{
		S3DLightEntry	light ;
		light.typeLight = lightTypeAmbientMul ;
		light.rgbColor = m_rgbAmbientMul ;
		light.fpBrightness = 1.0f ;
		m_arrayBufLights.Add( light ) ;
	}
	//
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->SetLightEntries
			( m_arrayBufLights.GetConstArray(), m_arrayBufLights.GetLength() ) ;
		m_pglDefShader->SetFog( m_rgbFogColor.ui32, m_zFogNear, m_zFogFar ) ;
		m_pglDefShader->EnableFog( m_flagFog ) ;
		//
		SArray<S4DMatrix>				aMat4ICameras ;
		SArray<uint32_t>				aIdLights ;
		SPointerArray<SGLImageObject>	aShadowmapDepths ;
		SArray<S3DShadowMapInfo>		aShadowmapInfos ;
		SPointerArray<SGLImageObject>	aShadowmapColors ;
		aMat4ICameras.SetLimit( 8 ) ;
		aIdLights.SetLimit( 8 ) ;
		aShadowmapDepths.SetLimit( 8 ) ;
		aShadowmapInfos.SetLimit( 8 ) ;
		aShadowmapColors.SetLimit( 8 ) ;
		//
		size_t	iShadow = 0 ;
		nCount = m_arrayShadowMapInf.GetLength() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ShadowMapEntry *	psme = m_arrayShadowMapInf.GetAt( i ) ;
			ESLAssert( psme != NULL ) ;
			if ( psme->pDepth == NULL )
			{
				continue ;
			}
			for ( size_t j = 0; j < m_arrayBufLights.GetLength(); j ++ )
			{
				S3DLightEntry *	pLight = m_arrayBufLights.GetAt( j ) ;
				if ( (pLight != NULL)
					&& (pLight->nReserved2[0] == psme->idLight) )
				{
					S3DDMatrix	matICamera ;
					S3DDVector	posICamera ;
					if ( m_flagProjUpsideDown )
					{
						S3DDMatrix	matCamera ;
						S3DDVector	vCameraPos = m_vCameraPos ;
						vCameraPos.y = - vCameraPos.y ;
						matCamera.InitializeMatrix( S3DDVector( 1, -1, 1 ) ) ;
						matCamera *= m_matCamera ;
						matICamera.InverseOf( matCamera ) ;
						posICamera = matICamera * vCameraPos ;
					}
					else
					{
						matICamera.InverseOf( m_matCamera ) ;
						posICamera = matICamera * m_vCameraPos ;
					}
					//
					S4DMatrix	mat4ICamera ;
					Matrix4x4From3x3
						( mat4ICamera, matICamera, posICamera ) ;
					//
					aMat4ICameras.Add( mat4ICamera ) ;
					aIdLights.Add( (uint32_t) j ) ;
					aShadowmapDepths.Add( psme->pDepth ) ;
					aShadowmapInfos.Add( psme->smiMapInfo ) ;
					aShadowmapColors.Add( psme->pColor ) ;
					/*
					m_pglDefShader->SetShadowMap
						( iShadow ++, mat4ICamera, (uint32_t) j,
							psme->pDepth, psme->smiMapInfo, psme->pColor ) ;
					*/
					break ;
				}
			}
		}
		m_pglDefShader->SetShadowMaps
			( aMat4ICameras.GetLength(),
				aMat4ICameras.GetConstArray(),
				aIdLights.GetConstArray(),
				aShadowmapInfos.GetConstArray(),
				aShadowmapDepths.GetConstArray(),
				aShadowmapColors.GetConstArray() ) ;
	}
	else
	{
		m_glRenderer.SetLightEntries
			( m_arrayBufLights.GetConstArray(), m_arrayBufLights.GetLength() ) ;
		m_glRenderer.SetFog( m_rgbFogColor.ui32, m_zFogNear, m_zFogFar ) ;
		m_glRenderer.EnableFog( m_flagFog ) ;
		//
		m_shader.SetLightEntries
			( m_arrayBufLights.GetConstArray(), m_arrayBufLights.GetLength() ) ;
		m_shader.SetFog( m_rgbFogColor.ui32, m_zFogNear, m_zFogFar ) ;
		m_shader.EnableFog( m_flagFog ) ;
	}
}

// 環境マッピングの設定を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::ReflectEnvironmentMapping( void )
{
	if ( m_pglDefShader != NULL )
	{
		S3DMatrix	matCamera = m_matCamera ;
		S3DMatrix	matICamera ;
		matICamera.InverseOf( matCamera ) ;
		//
		S3DMatrix	matEnvMapping = m_matEnvMapping * matICamera ;
		m_pglDefShader->SetEnvironmentMapping
			( m_pEnvMapping, m_nEnvMappingFlags, matEnvMapping ) ;
		//
		float32_t	fpDeepness = 1.0f ;
		if ( (m_nEnvRefractionFlags & envMappingTypeMask)
										== envMappingViewport )
		{
			fpDeepness =
				(float32_t) (m_vProjectionScreen.z
								* m_zProjectionScale * 0.1) ;
		}
		m_pglDefShader->SetRefractionMapping
			( m_pEnvRefraction, m_nEnvRefractionFlags, fpDeepness ) ;
		//
		m_pglDefShader->SetEnvironmentMappingViewportDepth( m_pEnvViewportDepth ) ;
	}
}

// 輪郭描画の設定を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::ReflectOffsetBorder( void )
{
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->SetOffsetBorderColor
				( m_optContext.opbBorder.rgbBorder ) ;
		m_pglDefShader->SetOffsetBorderCoefficient
				( m_optContext.opbBorder.aThickness,
					m_optContext.opbBorder.bThickness ) ;
	}
}

// フレームバッファのマルチレンダーターゲットを再設定する
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::ReflectRenderTarget( void )
{
	m_glFrameBuffer.AttachFrameBuffer( m_pglColor, m_pglDepth ) ;
	ReflectMultiRenderTargets() ;
}

// フレームバッファのマルチレンダーターゲットを再設定する
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::ReflectMultiRenderTargets( void )
{
	size_t	nMultiTargets = 0 ;
	SGLImageObject *const*
			ppTargetImages = GetMultiTargetImages( nMultiTargets ) ;
	if ( (ppTargetImages != nullptr) && (nMultiTargets > 0) )
	{
		SPointerArray<SGLOpenGLTextureBuffer>	aTargetColors ;
		SGLOpenGLContext *
				pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
		ESLAssert( pOpenGL != NULL ) ;
		//
		SGLOpenGLTextureBuffer **
				ppTargetColors = aTargetColors.GetArray( nMultiTargets ) ;
		for ( size_t i = 0; i < nMultiTargets; i ++ )
		{
			if ( ppTargetImages[i] != nullptr )
			{
				SGLImageRect	rectRefImage ;
				ppTargetColors[i] =
					SGLOpenGLTextureBuffer::CommitGLTexture
								( pOpenGL, ppTargetImages[i], rectRefImage ) ;
			}
		}
		m_glFrameBuffer.AddRenderTarget( ppTargetColors, nMultiTargets ) ;
		aTargetColors.FinishArray() ;
	}
	else
	{
		m_glFrameBuffer.AddRenderTarget( nullptr, 0 ) ;
	}
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	return	S3DOpenGLDirectlyRenderer::AddIndexedPrimitiveList
		( pMaterial, nFlags, primitiveTriangle,
			countPolygon * 3, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	return	S3DOpenGLDirectlyRenderer::AddIndexedPrimitiveList
		( pMaterial, nFlags, primitiveTriangleStrip,
			countTriangleStrip * 3, countTriangleStrip + 2,
			pvVertex, pvNormal, pvUVMap, pColor, NULL ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( pMaterial == NULL )
	{
		return	sglErrInvalidParam ;
	}
	if ( m_pglDefShader != NULL )
	{
		m_pglDefShader->SetDepthMaskOperation( m_optContext.depthMask ) ;
		m_pglDefShader->UpdateCustomUniform() ;
		//
		#if	defined(__API_OPEN_GL_ES__)
//		OptimizedMaterialShader( pMaterial ) ;
		#endif
		//
		// プログラマブル・シェーダーでレンダリング
		//
		S4DDMatrix	matModelView ;
		GetTransform4x4( matModelView ) ;
//		PutCameraViewMatrix() ;
		//
		if ( m_pglDefShader->IsForcedBorderShader()
			|| (pMaterial->m_attrSurface.flagsShading & shadingDrawOffsetBorder) )
		{
			// 輪郭描画
			S3DColor	colorEffect( 0x00FFFFFF, 0 ) ;
			GetColorEffect( colorEffect ) ;
			//
			m_pglDefShader->SetFaceCullingOperation( faceCullingDefault ) ;
			m_pglDefShader->SetModelViewMatrix( matModelView, true ) ;
			m_pglDefShader->SetBorderOffset
				( true, (float32_t) matModelView.m[2][3],
					colorEffect, pMaterial->m_attrSurface ) ;
			m_pglDefShader->SetMaterial
				( pMaterial, true,
					shadingMethodMask,
					shadingTextureTriming
						| shadingSingleSidePlane ) ;
			m_pglDefShader->AddIndexedPrimitiveList
				( nFlags, typePrimitive, countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
			m_pglDefShader->SetBorderOffset
				( false, 0.0f, colorEffect, pMaterial->m_attrSurface ) ;
		}
		//
		PutCurrentColorEffect
			( (pMaterial->m_attrSurface.flagsShading
							& shadingDisableColorEffect) != 0 ) ;
		if ( pMaterial->m_flagBack )
		{
			// 裏面描画
			uint32_t	nBumpFlag = 0 ;
			if ( pMaterial->m_attrBack.flagsShading & shadingNormalTexture )
			{
				nBumpFlag = S3DRenderBuffer::renderAutoTexAxis ;
			}
			m_pglDefShader->SetFaceCullingOperation( faceCullingDefault ) ;
			m_pglDefShader->SetModelViewMatrix( matModelView, true ) ;
			m_pglDefShader->SetMaterial( pMaterial, true ) ;
			m_pglDefShader->AddIndexedPrimitiveList
				( nFlags | nBumpFlag,
					typePrimitive, countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
		}
		// 表面描画
		uint32_t	nBumpFlag = 0 ;
		if ( pMaterial->m_attrSurface.flagsShading & shadingNormalTexture )
		{
			nBumpFlag = S3DRenderBuffer::renderAutoTexAxis ;
		}
		m_pglDefShader->SetFaceCullingOperation( m_optContext.faceCulling ) ;
		m_pglDefShader->SetModelViewMatrix( matModelView ) ;
		m_pglDefShader->SetMaterial( pMaterial ) ;
		m_flagUpdate = true ;
		return	m_pglDefShader->AddIndexedPrimitiveList
			( nFlags | nBumpFlag,
				typePrimitive, countIndex, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
	//
	// 座標変換と、色や透明度の設定を反映
	//
	TransformVertex3D( pvVertex, pvNormal, countVertex ) ;
	//
	// ソフトウェア・シェーディング
	//
	if ( !m_glRenderer.IsEnabledLightingByGL() )
	{
		if ( (pvNormal == NULL)
			&& (GetPrimitiveVertexCount(typePrimitive) == 3) )
		{
			// デフォルト法線生成
			if ( m_bufNormal.GetLength() < countVertex )
			{
				m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
			}
			S3DVector4 *	pvBufNormal = m_bufNormal.GetArray() ;
			eslFillMemory
				( pvBufNormal, 0, countVertex * sizeof(S3DVector4) ) ;
			//
			for ( size_t i = 0, j = 0; j < countIndex; i ++, j += 3 )
			{
				uint32_t	ix0 = pIndexedList[j] ;
				uint32_t	ix1 = pIndexedList[j + 1] ;
				uint32_t	ix2 = pIndexedList[j + 2] ;
				S3DVector	vd1 = pvVertex[ix1] - pvVertex[ix0] ;
				S3DVector	vd2 = pvVertex[ix2] - pvVertex[ix0] ;
				S3DVector	vNormal = vd1 * vd2 ;
				vNormal.Normalize() ;
				//
				pvBufNormal[ix0] += vNormal ;
				pvBufNormal[ix1] += vNormal ;
				pvBufNormal[ix2] += vNormal ;
			}
			m_bufNormal.FinishArray() ;
			pvNormal = pvBufNormal ;
		}
		//
		// 表面シェーディング
		//
		if ( m_bufColor.GetLength() < countVertex )
		{
			m_bufColor.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
		}
		S3DColor *	pBufColor = m_bufColor.GetArray() ;
		m_shader.ShadeVertexColors
			( pBufColor, pMaterial->m_attrSurface,
					pvNormal, pvVertex, pColor, countVertex ) ;
		m_bufColor.FinishArray() ;
		pColor = pBufColor ;
	}
	//
	// OpenGL レンダリング
	//
	m_flagUpdate = true ;
	return	m_glRenderer.AddIndexedPrimitiveList
		( pMaterial, nFlags,
			typePrimitive, countIndex, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLDirectlyRenderer::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing, const S3DColor * pColorInstancing )
{
	S3DRenderBuffer *			pglVB = NULL ;
	S3DRenderVariantBuffer *	pglVVB = NULL ;
	size_t						nVVBInstanceCount = 0 ;
	S3DVertexVariantBuffer **	ppVVBs = NULL ;
	S4DMatrix *					pVVBMatrixs = NULL ;
	S3DColor *					pVVBColors = NULL ;

	pglVVB = ESLTypeCast<S3DRenderVariantBuffer>( pBuffer ) ;
	if ( pglVVB == NULL )
	{
		pglVVB = ESLTypeCast<S3DRenderVariantBuffer>( pBuffer->GetVertexBufferObject() ) ;
	}
	if ( pglVVB != NULL )
	{
		pglVB = ESLTypeCast<S3DRenderBuffer>( pglVVB->GetVertexBuffer() ) ;
		ESLAssert( pglVB != NULL ) ;
		if ( pglVB != NULL )
		{
			if ( pglVVB->IsMultiInstancingMode() )
			{
				nVVBInstanceCount = pglVVB->GetInstancingCount() ;
				ppVVBs = m_bufInstanceVVB.GetArray( nVVBInstanceCount ) ;
				pVVBMatrixs = m_bufInstanceMatrix.GetArray( nVVBInstanceCount ) ;
				pVVBColors = m_bufInstanceColor.GetArray( nVVBInstanceCount ) ;
				nVVBInstanceCount =
					pglVVB->GetInstancingEntries
						( ppVVBs, pVVBMatrixs, pVVBColors, 0, nVVBInstanceCount ) ;
				m_bufInstanceVVB.FinishArray() ;
				m_bufInstanceMatrix.FinishArray() ;
				m_bufInstanceColor.FinishArray() ;
			}
			pglVB->UpdateVertexVariant( pglVVB->GetVariantBuffer(), iFirst, iEnd ) ;
		}
		else
		{
			pglVB = ESLTypeCast<S3DRenderBuffer>( pBuffer->GetVertexBufferObject() ) ;
		}
	}
	else
	{
		pglVB = ESLTypeCast<S3DRenderBuffer>( pBuffer->GetVertexBufferObject() ) ;
	}
	ESLAssert( pglVB != NULL ) ;
	if ( pglVB == NULL )
	{
		return	pBuffer->RenderBufferTo
			( this, 0, iFirst, iEnd,
				nInstancing, pmatInstancing, pColorInstancing ) ;
	}
	if ( m_pglDefShader == NULL )
	{
		return	pglVB->RenderTemporaryBufferTo
				( this, 0, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
	}
	m_pglDefShader->SetFaceCullingOperation( m_optContext.faceCulling ) ;
	m_pglDefShader->SetDepthMaskOperation( m_optContext.depthMask ) ;
	m_pglDefShader->UpdateCustomUniform() ;
	//
	S4DDMatrix		matModelView ;
	S3DColor		colorBase ;
	unsigned int	nTransparency = 0 ;
	//
	GetTransform4x4( matModelView ) ;
	GetColorEffect( colorBase ) ;
	nTransparency = EffectTransparency( nTransparency ) ;
	//
	m_flagUpdate = true ;
	//
	if ( nVVBInstanceCount > 0 )
	{
		SGLError	err = sglErrSuccess ;
		if ( pmatInstancing && pColorInstancing && (nInstancing >= 1) )
		{
			err = m_pglDefShader->AddVertexBuffer
					( matModelView, colorBase, nTransparency,
						nFlags, pglVB, iFirst, iEnd,
						nInstancing, pmatInstancing, pColorInstancing ) ;
		}
		if ( m_pglDefShader->m_enabledVTInstancing )
		{
			err = m_pglDefShader->AddVertexBuffer
					( matModelView, colorBase, nTransparency,
						nFlags, pglVB, iFirst, iEnd,
						nVVBInstanceCount, pVVBMatrixs, pVVBColors, ppVVBs ) ;
		}
		else
		{
			for ( size_t i = 0; i < nVVBInstanceCount; i ++ )
			{
				pglVB->UpdateVertexVariant( ppVVBs[i], iFirst, iEnd ) ;
				err = m_pglDefShader->AddVertexBuffer
						( matModelView, colorBase, nTransparency,
							nFlags, pglVB, iFirst, iEnd,
							1, pVVBMatrixs + i, pVVBColors + i, NULL ) ;
			}
		}
		return	err ;
	}
	else
	{
		return	m_pglDefShader->AddVertexBuffer
					( matModelView, colorBase, nTransparency,
						nFlags, pglVB, iFirst, iEnd,
						nInstancing, pmatInstancing, pColorInstancing ) ;
	}
}

// プログラマブルシェーダーをマテリアルに応じて変更する
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::OptimizedMaterialShader( S3DMaterial * pMaterial )
{
	ESLAssert( m_pglDefShader != NULL ) ;
	SGLOpenGLContext *	pOpenGL = SGLOpenGLContext::GetCurrentGLContext() ;
	if ( pOpenGL != NULL )
	{
		uint64_t nShadingMethod = m_optContext.nShadingFlags ;
		if ( pMaterial && !(pMaterial->m_flagBack)
			&& ((pMaterial->m_attrSurface.flagsShading
								& shadingMethodMask) == shadingMethodNothing) )
		{
			nShadingMethod &= ~shadingMethodMask ;
		}
		SGLOpenGLShaderProgram *	pDefShader =
				pOpenGL->GetDefaultShaderProgram( (int) nShadingMethod ) ;
		if ( (pDefShader != NULL)
			&& (pOpenGL->GetCurrentShaderProgram() != pDefShader) )
		{
			SetShadingFlagsToGLContext( nShadingMethod ) ;
		}
	}
}

// 現在のカメラ変換行列を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::PutCameraViewMatrix( void )
{
	if ( m_pglDefShader != NULL )
	{
		S3DDMatrix	matT = m_matCamera ;
		S3DDVector	posT = - m_vCameraPos ;
		//
		if ( m_flagProjUpsideDown )
		{
			matT.m[1][0] = - matT.m[1][0] ;
			matT.m[1][1] = - matT.m[1][1] ;
			matT.m[1][2] = - matT.m[1][2] ;
			posT.y = - posT.y ;
		}
		S4DDMatrix	mat4 ;
		Matrix4x4From3x3( mat4, matT, posT ) ;
		//
		m_pglDefShader->SetCameraViewMatrix( mat4 ) ;
	}
}

// 現在のモデル変換行列を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::GetTransform4x4( S4DDMatrix& mat4 )
{
	S3DDMatrix	matT ;
	S3DDVector	posT ;
	GetTransformMatrix( matT, posT ) ;
	//
	if ( m_flagProjUpsideDown )
	{
		matT.m[1][0] = - matT.m[1][0] ;
		matT.m[1][1] = - matT.m[1][1] ;
		matT.m[1][2] = - matT.m[1][2] ;
		posT.y = - posT.y ;
	}
	Matrix4x4From3x3( mat4, matT, posT ) ;
}

// 現在の色や透明度の設定を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::PutCurrentColorEffect( bool flagDisableColorEffect )
{
	if ( m_pglDefShader != NULL )
	{
		if ( m_pTransformation != NULL )
		{
			if ( !flagDisableColorEffect )
			{
				m_pglDefShader->SetColorEffect
					( &(m_pTransformation->colorEffect),
							m_pTransformation->nTransparency ) ;
			}
			else
			{
				m_pglDefShader->SetColorEffect
					( NULL, m_pTransformation->nTransparency ) ;
			}
		}
		else
		{
			m_pglDefShader->SetColorEffect( NULL, 0 ) ;
		}
	}
	else
	{
		if ( m_pTransformation != NULL )
		{
			if ( !flagDisableColorEffect )
			{
				m_glRenderer.SetColorEffect
					( &(m_pTransformation->colorEffect),
							m_pTransformation->nTransparency ) ;
			}
			else
			{
				m_glRenderer.SetColorEffect
					( NULL, m_pTransformation->nTransparency ) ;
			}
		}
		else
		{
			m_glRenderer.SetColorEffect( NULL, 0 ) ;
		}
	}
}

// 頂点座標と法線を変換し、現在の色と透明度属性を反映
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLDirectlyRenderer::TransformVertex3D
	( const S3DVector4 *& pvVertex,
		const S3DVector4 *& pvNormal, size_t countVertex )
{
	//
	// 変換行列取得
	//
	S3DDMatrix	matdTransform ;
	S3DDVector	posdTranslate ;
	GetTransformMatrix( matdTransform, posdTranslate ) ;
	PutCurrentColorEffect() ;
	//
	// 頂点座標変換
	//
	S3DMatrix	matTransform = matdTransform ;
	S3DVector	posTranslate = posdTranslate ;
	if ( m_bufVertex.GetLength() < countVertex )
	{
		m_bufVertex.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
	}
	S3DVector4 *	pvBufVertex = m_bufVertex.GetArray() ;
	matTransform.RevolveVectors
		( pvBufVertex, pvVertex, countVertex, posTranslate ) ;
	m_bufVertex.FinishArray() ;
	pvVertex = pvBufVertex ;
	//
	// 法線回転
	//
	if ( pvNormal != NULL )
	{
		if ( m_bufNormal.GetLength() < countVertex )
		{
			m_bufNormal.SetLength( (countVertex + 0xFF) & ~0xFF ) ;
		}
		S3DVector		vZero( 0, 0, 0 ) ;
		S3DVector4 *	pvBufNormal = m_bufNormal.GetArray() ;
		matTransform.RevolveVectors
			( pvBufNormal, pvNormal, countVertex, vZero ) ;
		m_bufNormal.FinishArray() ;
		pvNormal = pvBufNormal ;
	}
}

// ハードウェア描画オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * S3DOpenGLDirectlyRenderer::GetRenderDeviceObject( uint64_t nFlags )
{
	return	SGLOpenGLContext::GetCurrentGLContext() ;
}



//////////////////////////////////////////////////////////////////////////////
// OpenGL レンダリング・コンテキスト（バッファリング可能）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DOpenGLBufferedRenderer, S3DRenderBufferedContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DOpenGLBufferedRenderer::S3DOpenGLBufferedRenderer( SGLOpenGLContext * pOpenGL )
{
	ESLAssert( pOpenGL != NULL ) ;
	m_refOpenGL = pOpenGL ;
	m_optContext.nShadingFlags = shadingMethodNothing | shadingTextureSmoothing ;
	m_countFlush = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DOpenGLBufferedRenderer::~S3DOpenGLBufferedRenderer( void )
{
}

// 現在のスレッドに関連付けられた OpenGL レンダラか？
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLBufferedRenderer::IsCurrentOpenGLRenderer( void ) const
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL.GetReference() ;
	ESLAssert( pOpenGL != NULL ) ;
	return	pOpenGL->IsCurrentRenderContext( this ) ;
}

// OpenGL コンテキスト関連付け変更
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::AttachOpenGLContext( SGLOpenGLContext * pOpenGL )
{
	ESLAssert( pOpenGL != NULL ) ;
	m_refOpenGL = pOpenGL ;
}

// OpenGL スレッド上／レンダラが設定された状態で任意関数実行
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::ProcedureWithRederer( SSystem::SProcedure * pProc )
{
	m_queProc.AddProcedure( pProc ) ;
	//
	SGLError	err = ForceFlush() ;
	return	err ;
}

// OpenGL スレッド上／レンダラが設定された状態で任意関数実行（非同期遅延）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::PostProcedureWithRederer
	( SSystem::SProcedure * pProc, bool flagAutoDelete )
{
	m_queProc.AddProcedure( pProc, nullptr, flagAutoDelete ) ;
	return	sglErrSuccess ;
}

// 描画の実行（OpenGL スレッド上でなくても）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::ForceFlush( void )
{
	S3DRenderBufferedContext::Flush() ;
	//
	ESLAssert( m_refOpenGL != NULL ) ;
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL == NULL )
	{
		pOpenGL = SGLOpenGLContext::GetDefault() ;
		if ( pOpenGL == NULL )
		{
			return	sglErrFailed ;
		}
	}
	ESLAssert( pOpenGL != NULL ) ;
	FlushProcedure	flush( this, false, false ) ;
	SGLError	err = pOpenGL->Procedure( &flush, S3DRenderDevice::procedureSync ) ;
	if ( err )
	{
		return	err ;
	}
	return	sglErrSuccess ;
}

// ミラーリング FBO 転送設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetMirrorFrameBuffer
	( GLuint glDstRightFBO, GLuint glDstLeftFBO,
		const SGLImageRect& rectSrc, const SGLImageRect& rectDst )
{
	m_glRenderer.SetMirrorFrameBuffer
		( glDstRightFBO, glDstLeftFBO, rectSrc, rectDst ) ;
}

void S3DOpenGLBufferedRenderer::DetachMirrorFrameBuffer( void )
{
	m_glRenderer.DetachMirrorFrameBuffer() ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	SGLError	err =
		S3DRenderBufferedContext::AttachTargetImage
								( pImage, pZBuffer, nullptr /*pView*/ ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		err = m_glRenderer.AttachTargetImage( pImage, pZBuffer, pView ) ;
//	}
	return	err ;
}

// マルチターゲット（2つ目以降）描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::AttachMultiTargetImages
	( SGLImageObject*const* ppTargets, size_t nCount )
{
	SGLError	err =
		S3DRenderBufferedContext::AttachMultiTargetImages( ppTargets, nCount ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		err = m_glRenderer.AttachMultiTargetImages( ppTargets, nCount ) ;
//	}
	return	err ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::DetachTargetImage( void )
{
	if ( m_glRenderer.IsUpdatedTarget() || !IsEmptyRenderBuffer() )
	{
		Finish() ;
	}
	SGLError	err = S3DRenderBufferedContext::DetachTargetImage() ;
	if ( IsCurrentOpenGLRenderer() )
	{
		err = m_glRenderer.DetachTargetImage() ;
	}
	return	err ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetPaintFlags( int64_t nFlags )
{
	S3DRenderBufferedContext::SetPaintFlags( nFlags ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetPaintFlags( nFlags ) ;
//	}
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	S3DOpenGLBufferedRenderer *
			poglBufSrc = ESLTypeCast<S3DOpenGLBufferedRenderer>( &renderSrc ) ;
	if ( poglBufSrc == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	renderSrc.Flush() ;
	//
	CopyBufferProcedure *	pProc =
		new CopyBufferProcedure( poglBufSrc, this, nFlags, xDst, yDst, pSrcRect ) ;
	PostProcedureWithRederer( pProc, true ) ;
	return	ForceFlush() ;
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::AttachCustomShader( S3DCustomShader * pShader )
{
	return	S3DRenderBufferedContext::AttachCustomShader( pShader ) ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DOpenGLBufferedRenderer::GetCustomShader( void ) const
{
	return	S3DRenderBufferedContext::GetCustomShader() ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::SetProjectionScreen
		( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
//	if ( (m_vProjectionScreen != vScreen)
//		|| (m_zProjectionScale != zScale)
//		|| (m_fpPixelAspectRatio != fpPixelAspect) )
//	{
//		Flush() ;
//	}
	SGLError	err =
		S3DRenderBufferedContext::SetProjectionScreen
							( vScreen, zScale, fpPixelAspect ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		err = m_glRenderer.SetProjectionScreen
//							( vScreen, zScale, fpPixelAspect ) ;
//	}
	return	err ;
}

// 透視変換行列取得
//////////////////////////////////////////////////////////////////////////////
bool S3DOpenGLBufferedRenderer::GetPerspectiveMatrix( S4DMatrix& matPers ) const
{
	return	m_glRenderer.GetPerspectiveMatrix( matPers ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
//	if ( (m_vCameraPos != posCamera) || (m_matCamera != matCamera) )
//	{
//		Flush() ;
//	}
	S3DRenderBufferedContext::SetCamera( matCamera, posCamera ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetCamera( matCamera, posCamera ) ;
//	}
}

// 立体視視差設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	S3DRenderBufferedContext::SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
//	}
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetZClipRange( double zMin, double zMax )
{
	S3DRenderBufferedContext::SetZClipRange( zMin, zMax ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetZClipRange( zMin, zMax ) ;
//	}
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	S3DRenderBufferedContext::SetLightEntries( pLights, countLight ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetLightEntries( pLights, countLight ) ;
//	}
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMapDepth,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	S3DRenderBufferedContext::SetShadowMap
		( idLight, pShadowMapDepth, infShadowMap, pShadowMapColor ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetShadowMap
//			( idLight, pShadowMapDepth, infShadowMap, pShadowMapColor ) ;
//	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	S3DRenderBufferedContext::SetFog( rgbFog, zFogNear, zFogFar ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetFog( rgbFog, zFogNear, zFogFar ) ;
//	}
}

void S3DOpenGLBufferedRenderer::EnableFog( bool fFog )
{
	S3DRenderBufferedContext::EnableFog( fFog ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.EnableFog( fFog ) ;
//	}
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetShadingFlag( uint64_t nShadingMethod )
{
	S3DRenderBufferedContext::SetShadingFlag( nShadingMethod ) ;
}

// レイトレーシング設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetRayTracingParameter
			( const S3DRenderRayTracingParam& rrtp )
{
	S3DRenderBufferedContext::SetRayTracingParameter( rrtp ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetRayTracingParameter( rrtp ) ;
//	}
}

// グローバル環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetEnvironmentMappingImage
			( SGLImageObject * pImage, uint32_t nFlags )
{
	S3DRenderBufferedContext::SetEnvironmentMappingImage( pImage, nFlags ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetEnvironmentMappingImage( pImage, nFlags ) ;
//	}
}

// グローバル環境マッピング変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetEnvironmentMappingMatrix( const S3DMatrix& matMapping )
{
	S3DRenderBufferedContext::SetEnvironmentMappingMatrix( matMapping ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		m_glRenderer.SetEnvironmentMappingMatrix( matMapping ) ;
//	}
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetOffsetBorderColor( uint32_t rgbBorder )
{
	S3DRenderBufferedContext::SetOffsetBorderColor( rgbBorder ) ;
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	S3DRenderBufferedContext::SetOffsetBorderCoefficient( a, b ) ;
}

// オプショナル機能設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::SetOptionalFeature
	( S3DRenderContextInterface::FeatureType feature,
			int32_t nParam1, const void * pParam2, size_t sizeOfParam2 )
{
	SGLError	err =
		S3DRenderBufferedContext::SetOptionalFeature
					( feature, nParam1, pParam2, sizeOfParam2 ) ;
//	if ( IsCurrentOpenGLRenderer() )
//	{
//		err = m_glRenderer.SetOptionalFeature
//					( feature, nParam1, pParam2, sizeOfParam2 ) ;
//	}
	return	err ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::Flush( void )
{
	SGLError	err = S3DRenderBufferedContext::Flush() ;
	//
	if ( !IsEmptyRenderBuffer() )
	{
		ESLAssert( m_refOpenGL != NULL ) ;
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		if ( (pOpenGL != NULL)
			&& !pOpenGL->IsOnRenderThread() )
		{
			if ( (pOpenGL->GetCurrentRenderContext() != this)
				|| (AtomicAdd( &m_countFlush, 1 ) <= 1) )
			{
				FlushProcedure *	pFlushProc = new FlushProcedure( this, false, true ) ;
				err = pOpenGL->Procedure( pFlushProc, S3DRenderDevice::procedureAsync ) ;
			}
		}
	}

/*	ESLAssert( m_refOpenGL != NULL ) ;
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL == NULL )
	{
		pOpenGL = SGLOpenGLContext::GetDefault() ;
		if ( pOpenGL == NULL )
		{
			return	sglErrFailed ;
		}
	}
	if ( (pOpenGL != NULL)
		&& pOpenGL->IsOnRenderThread() && !IsEmptyRenderBuffer() )
	{
		FlushProcedure	flush( this, false ) ;
		err = pOpenGL->Procedure( &flush, true ) ;
		if ( err )
		{
			return	err ;
		}
	}
*/	return	err ;
}

SGLError S3DOpenGLBufferedRenderer::Finish( void )
{
	SGLError	errFlush = S3DRenderBufferedContext::Flush() ;
	//
	if ( m_glRenderer.IsUpdatedTarget() || !IsEmptyRenderBuffer() )
	{
		ESLAssert( m_refOpenGL != NULL ) ;
		SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
		if ( pOpenGL == NULL )
		{
			pOpenGL = SGLOpenGLContext::GetDefault() ;
			if ( pOpenGL == NULL )
			{
				return	sglErrFailed ;
			}
		}
		FlushProcedure	flush( this, true, false ) ;
		SGLError	errProc = pOpenGL->Procedure( &flush, S3DRenderDevice::procedureSync ) ;
		if ( errProc )
		{
			return	errProc ;
		}
	}
	return	errFlush ;
}

// 非同期レンダリングに適したスレッドで実行
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SuitableProcedure
		( PROCEDURE_RENDERING pfnRendering, void * pInstance )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL.GetReference() ;
	ESLAssert( pOpenGL != NULL ) ;
	//
	pOpenGL->SuitableProcedure( pfnRendering, pInstance ) ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	S3DRenderBufferedContext::GetRenderingCapacity( caps ) ;
	caps.flagsShading |= S3DRenderingCapacity::shadingGouraud ;
	caps.typeHeadware = S3DRenderingCapacity::hardwareOpenGL ;
	//
	SGLOpenGLContext *	pOpenGL = m_refOpenGL ;
	if ( pOpenGL != NULL )
	{
		S3DOpenGLDirectlyRenderer::GetRenderingCapacityWithOpenGL( caps, pOpenGL ) ;
	}
}

// ハードウェア描画オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * S3DOpenGLBufferedRenderer::GetRenderDeviceObject( uint64_t nFlags )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL.GetReference() ;
/*
	if ( nFlags & deviceNeedsOnThread )
	{
		if ( !pOpenGL->IsOnRenderThread() )
		{
			return	NULL ;
		}
	}
*/
	return	pOpenGL ;
}

// ハードウェア描画オブジェクト変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DOpenGLBufferedRenderer::SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags )
{
	SGLOpenGLContext *	pOpenGL = ESLTypeCast<SGLOpenGLContext>( pDevice ) ;
	if ( pOpenGL == NULL )
	{
		return	sglErrInvalidParam ;
	}
	AttachOpenGLContext( pOpenGL ) ;
	return	sglErrSuccess ;
}

// Flush 関数処理
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::OnGLThreadFlush( bool flagFinish )
{
	SGLOpenGLContext *	pOpenGL = m_refOpenGL.GetReference() ;
	ESLAssert( pOpenGL != NULL ) ;
	SwitchRenderContext( pOpenGL ) ;

	do
	{
		FlushRenderAllViewBufferTo( &m_glRenderer ) ;
	}
	while ( AtomicSub( &m_countFlush, 1 ) > 0 ) ;

	if ( flagFinish )
	{
		ClearAllViewBuffer() ;
		m_glRenderer.Finish() ;
	}
	else
	{
		m_glRenderer.Flush() ;
	}

	if ( (m_queProc.Flush() > 0) || flagFinish )
	{
		pOpenGL->AttachRenderContext( nullptr ) ;
	}
}

// OpenGL の現在の RenderContext を切り替える
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::SwitchRenderContext( SGLOpenGLContext * pOpenGL )
{
	ESLAssert( pOpenGL != NULL ) ;
	if ( pOpenGL->GetCurrentRenderContext() != this )
	{
		ESLAssert( pOpenGL->IsOnRenderThread() ) ;
		pOpenGL->m_pflog.countSwitchRenderer ++ ;
		m_glRenderer.Set3DRendererFlag( false ) ;	// 無駄な OpenGL 設定の抑制 ～ Begin3DRenderer
		//
		m_glRenderer.AttachCustomShader( GetCustomShader() ) ;	// ターゲット設定の前にシェーダーを設定して
		m_glRenderer.SetShadingFlag( GetShadingFlag() ) ;		// シェーダ―のセットアップが二重に行われるのを抑制する
		//
		AttachTargetImagesTo( m_glRenderer ) ;
		//
		SetAllRenderingParameterTo( &m_glRenderer ) ;	// 全パラメータ設定
		//
		m_glRenderer.Begin3DRenderer() ;	// OpenGL 設定を反映
		//
		pOpenGL->AttachRenderContext( this ) ;
	}
	else
	{
		if ( IsEqualTargetImages( m_glRenderer ) )
		{
			AttachTargetImagesTo( m_glRenderer ) ;
		}
	}
}


// Flush 処理
//////////////////////////////////////////////////////////////////////////////

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void S3DOpenGLBufferedRenderer::FlushProcedure::Run( void )
{
	m_render->OnGLThreadFlush( m_finish ) ;
}

void S3DOpenGLBufferedRenderer::FlushProcedure::Finalize( void )
{
	if ( m_flagDelete )
	{
		delete	this ;
	}
}

// CopyBufferFrom 処理
//////////////////////////////////////////////////////////////////////////////

// 構築関数
S3DOpenGLBufferedRenderer::CopyBufferProcedure::CopyBufferProcedure
	( S3DOpenGLBufferedRenderer * pSrc,
		S3DOpenGLBufferedRenderer * pDst,
		uint32_t nFlags, int xDst, int yDst,
		const SGLImageRect * pSrcRect )
	: m_poglBufSrc( pSrc ), m_poglBufDst( pDst ),
		m_nFlags( nFlags ), m_xDst( xDst ), m_yDst( yDst ), m_pSrcRect( nullptr )
{
	ESLAssert( pSrc != nullptr ) ;
	ESLAssert( pDst != nullptr ) ;
	if ( pSrcRect != nullptr )
	{
		m_rectSrc = *pSrcRect ;
		m_pSrcRect = &m_rectSrc ;
	}
}

// スレッド関数
void S3DOpenGLBufferedRenderer::CopyBufferProcedure::Run( void )
{
	ESLAssert( m_poglBufDst->IsEqualTargetImages( m_poglBufDst->m_glRenderer ) ) ;

	if ( !m_poglBufSrc->IsEqualTargetImages( m_poglBufSrc->m_glRenderer ) )
	{
		m_poglBufSrc->AttachTargetImagesTo( m_poglBufSrc->m_glRenderer ) ;
	}

	m_poglBufDst->m_glRenderer.CopyBufferFrom
		( m_poglBufSrc->m_glRenderer, m_nFlags, m_xDst, m_yDst, m_pSrcRect ) ;
}

