
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// CPU 画像描画コンテキスト
//////////////////////////////////////////////////////////////////////////////

// フィルタ関数テーブル
//////////////////////////////////////////////////////////////////////////////
const SGLPaintBuffer::PROC_FILTER_PIXELS
				SGLPaintBuffer::m_tableFilterProc[0x10] =
{
	&SGLPaintBuffer::FilterColorAddProc,
	NULL,
	&SGLPaintBuffer::FilterColorMulProc,
	NULL,
	NULL, NULL, NULL, NULL,
	//
	&SGLPaintBuffer::FilterAlphaMulProc,
	&SGLPaintBuffer::FilterColorMaskProc,
	NULL, NULL,
	NULL, NULL, NULL, NULL,
} ;

// 通常描画関数テーブル [alpha?][transparency?]
//////////////////////////////////////////////////////////////////////////////
const SGLPaintBuffer::PAINT_PROC_ENTRY
			SGLPaintBuffer::m_tableNormalPaintProc[2][2] =
{
	{
		// RGB, opaque
		{ &SGLPaintBuffer::PaintNormalProc, false },
		// RGB, transparency
		#if	defined(__COTOPHA__)
		{ &SGLPaintBuffer::PaintTransparencyProc, false },
		#else
		{ &SGLPaintBuffer::PaintTransparencyProc, true },
		#endif
	},
	{
		// RGBA, opaque
		{ &SGLPaintBuffer::PaintNormalBlendProc, false },
		// RGBA, transparency
		{ &SGLPaintBuffer::PaintTransparencyBlendProc, false },
	},
} ;

// 描画関数テーブル
//////////////////////////////////////////////////////////////////////////////
const SGLPaintBuffer::PROC_PAINT_PIXELS
			SGLPaintBuffer::m_tablePaintLineProc[2][0x10] =
{
	{
		&SGLPaintBuffer::PaintAddBlendProc,
		&SGLPaintBuffer::PaintSubBlendProc,
		&SGLPaintBuffer::PaintMulBlendProc,
		&SGLPaintBuffer::PaintDivBlendProc,
		&SGLPaintBuffer::PaintMaxBlendProc,
		&SGLPaintBuffer::PaintMinBlendProc,
		&SGLPaintBuffer::PaintNegMulBlendProc,
		NULL,
		&SGLPaintBuffer::PaintNoBlendAlphaProc,
		&SGLPaintBuffer::PaintMulAlphaBlendProc,
		&SGLPaintBuffer::PaintDstAlphaMaskBlendProc,
		NULL,
		NULL, NULL, NULL, NULL,
	},
	{
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		&SGLPaintBuffer::PaintTransparencyNoBlendAlphaProc,
		NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
	},
} ;

// 塗りつぶし関数用関数テーブル [FillOperation]
//////////////////////////////////////////////////////////////////////////////
const SGLPaintBuffer::PROC_SAMPLING_PIXELS
	SGLPaintBuffer::m_tableFillSamplingProc[SGLPaintBuffer::fillTypeCount] =
{
	&SGLPaintBuffer::SamplingFillColorProc,
	&SGLPaintBuffer::SamplingLinearGradationProc,
	&SGLPaintBuffer::SamplingRingedGradationProc,
} ;

// サンプリング関数テーブル [depth/8][pitch==depth/8?][smoothing?]
//////////////////////////////////////////////////////////////////////////////
const SGLPaintBuffer::PROC_SAMPLING_PIXELS
				SGLPaintBuffer::m_tableSamplingProc[4][2][2] =
{
	// 8 bits depth
	{
		{	// 8 bits depth N pitch
			&SGLPaintBuffer::Sampling8bitsNpitchProc,
			&SGLPaintBuffer::Sampling8bitsNpitchProc,
		},
		{	// 8 bits depth 1 pitch
			&SGLPaintBuffer::Sampling8bitsNpitchProc,
			&SGLPaintBuffer::Sampling8bitsNpitchProc,
		},
	},
	// 16 bits depth
	{
		{	// 16 bits depth N pitch
			&SGLPaintBuffer::Sampling16bitsNpitchProc,
			&SGLPaintBuffer::Sampling16bitsNpitchProc,
		},
		{	// 16 bits depth 2 pitch
			&SGLPaintBuffer::Sampling16bitsNpitchProc,
			&SGLPaintBuffer::Sampling16bitsNpitchProc,
		},
	},
	// 24 bits depth
	{
		{	// 24 bits depth N pitch
			&SGLPaintBuffer::Sampling24bitsNpitchProc,
			&SGLPaintBuffer::Sampling24bitsNpitchProc,
		},
		{	// 24 bits depth 3 pitch
			&SGLPaintBuffer::Sampling24bitsNpitchProc,
			&SGLPaintBuffer::Sampling24bitsNpitchProc,
		},
	},
	// 32 bits depth
	{
		{	// 32 bits depth N pitch
			&SGLPaintBuffer::Sampling32bitsNpitchProc,
			&SGLPaintBuffer::Sampling32bitsNpitchProc,
		},
		{	// 32 bits depth 4 pitch
			&SGLPaintBuffer::Sampling32bitsProc,
			&SGLPaintBuffer::SamplingSmooth32bitsProc,
		},
	},
} ;

// 除算描画用逆数テーブル
//////////////////////////////////////////////////////////////////////////////
const uint32_t	SGLPaintBuffer::m_tableDivBlend[0x100] =
{
	0x0040, 0x0040, 0x0040, 0x0041, 0x0041, 0x0041, 0x0041, 0x0042,
	0x0042, 0x0042, 0x0042, 0x0043, 0x0043, 0x0043, 0x0043, 0x0044,
	0x0044, 0x0044, 0x0045, 0x0045, 0x0045, 0x0046, 0x0046, 0x0046,
	0x0046, 0x0047, 0x0047, 0x0047, 0x0048, 0x0048, 0x0048, 0x0049,
	0x0049, 0x0049, 0x004A, 0x004A, 0x004A, 0x004B, 0x004B, 0x004B,
	0x004C, 0x004C, 0x004C, 0x004D, 0x004D, 0x004E, 0x004E, 0x004E,
	0x004F, 0x004F, 0x004F, 0x0050, 0x0050, 0x0051, 0x0051, 0x0051,
	0x0052, 0x0052, 0x0053, 0x0053, 0x0054, 0x0054, 0x0054, 0x0055,
	0x0055, 0x0056, 0x0056, 0x0057, 0x0057, 0x0058, 0x0058, 0x0059,
	0x0059, 0x005A, 0x005A, 0x005B, 0x005B, 0x005C, 0x005C, 0x005D,
	0x005D, 0x005E, 0x005E, 0x005F, 0x005F, 0x0060, 0x0060, 0x0061,
	0x0062, 0x0062, 0x0063, 0x0063, 0x0064, 0x0065, 0x0065, 0x0066,
	0x0067, 0x0067, 0x0068, 0x0069, 0x0069, 0x006A, 0x006B, 0x006B,
	0x006C, 0x006D, 0x006D, 0x006E, 0x006F, 0x0070, 0x0070, 0x0071,
	0x0072, 0x0073, 0x0074, 0x0075, 0x0075, 0x0076, 0x0077, 0x0078,
	0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F, 0x0080,
	0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088,
	0x0089, 0x008A, 0x008C, 0x008D, 0x008E, 0x008F, 0x0090, 0x0092,
	0x0093, 0x0094, 0x0096, 0x0097, 0x0099, 0x009A, 0x009C, 0x009D,
	0x009F, 0x00A0, 0x00A2, 0x00A3, 0x00A5, 0x00A7, 0x00A8, 0x00AA,
	0x00AC, 0x00AE, 0x00B0, 0x00B2, 0x00B4, 0x00B6, 0x00B8, 0x00BA,
	0x00BC, 0x00BE, 0x00C0, 0x00C3, 0x00C5, 0x00C7, 0x00CA, 0x00CC,
	0x00CF, 0x00D2, 0x00D4, 0x00D7, 0x00DA, 0x00DD, 0x00E0, 0x00E3,
	0x00E6, 0x00EA, 0x00ED, 0x00F0, 0x00F4, 0x00F8, 0x00FC, 0x0100,
	0x0104, 0x0108, 0x010C, 0x0111, 0x0115, 0x011A, 0x011F, 0x0124,
	0x0129, 0x012F, 0x0135, 0x013B, 0x0141, 0x0147, 0x014E, 0x0155,
	0x015C, 0x0164, 0x016C, 0x0174, 0x017D, 0x0186, 0x018F, 0x0199,
	0x01A4, 0x01AF, 0x01BA, 0x01C7, 0x01D4, 0x01E1, 0x01F0, 0x0200,
	0x0210, 0x0222, 0x0234, 0x0249, 0x025E, 0x0276, 0x028F, 0x02AA,
	0x02C8, 0x02E8, 0x030C, 0x0333, 0x035E, 0x038E, 0x03C3, 0x0400,
	0x0444, 0x0492, 0x04EC, 0x0555, 0x05D1, 0x0666, 0x071C, 0x0800,
	0x0924, 0x0AAA, 0x0CCC, 0x1000, 0x1555, 0x2000, 0x4000, 0x4000,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLPaintBuffer, SGLPaintParameterContext, SGLDrawContextInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintBuffer::SGLPaintBuffer( void )
{
	m_pbytTarget = NULL ;
	m_pbytZBuffer = NULL ;
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	m_pbytSource = NULL ;
	m_pbytRectDst = NULL ;
	m_pbytRectZBuf = NULL ;
	m_pbufRegion = NULL ;
	m_opFillType = fillFlat ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintBuffer::~SGLPaintBuffer( void )
{
	SGLPaintBuffer::DetachTargetImage() ;
	//
	if ( m_pbufRegion != NULL )
	{
		m_bufRegion.FreeArray() ;
		m_pbufRegion = NULL ;
	}
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	SGLPaintBuffer::DetachTargetImage() ;
	//
	if ( pImage != NULL )
	{
		pImage->NormalizeToTexture( SGLImageObject::bufferNonPowerOf2 ) ;
	}
	if ( pZBuffer != NULL )
	{
		pZBuffer->NormalizeToTexture( SGLImageObject::bufferNonPowerOf2 ) ;
	}
	SGLError	err =
		SGLPaintParameterContext::AttachTargetImage
								( pImage, pZBuffer, pView ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 画像バッファ取得
	//
	m_pbytTarget =
		m_pTarget->LockBuffer
			( m_infTarget, SGLImageObject::lockReadWrite, &m_rctView ) ;
	if ( m_pbytTarget == NULL )
	{
		DetachTargetImage() ;
		return	sglErrFailed ;
	}
	if ( m_infTarget.format & formatImageFlagS3TC )
	{
		DetachTargetImage() ;
		return	sglErrFailed ;
	}
	m_pbytZBuffer = NULL ;
	if ( m_pZBuffer != NULL )
	{
		m_pbytZBuffer =
			m_pZBuffer->LockBuffer
				( m_infZBuffer, SGLImageObject::lockReadWrite, &m_rctView ) ;
		if ( m_pbytZBuffer == NULL )
		{
			DetachTargetImage() ;
			return	sglErrFailed ;
		}
	}
	if ( pView != NULL )
	{
		// 内部的にターゲットの画像全体へのバッファ情報として
		// 保持しているので情報を補正
		m_pbytTarget -= m_rctView.x * m_infTarget.pitchPixel
							+ m_rctView.y * m_infTarget.pitchLine ;
		if ( m_pbytZBuffer != NULL )
		{
			m_pbytZBuffer -= m_rctView.x * m_infZBuffer.pitchPixel
								+ m_rctView.y * m_infZBuffer.pitchLine ;
		}
		SGLSize	sizeTarget = m_pTarget->GetImageSize() ;
		m_infTarget.width = sizeTarget.w ;
		m_infTarget.height = sizeTarget.h ;
		if ( m_pZBuffer != NULL )
		{
			sizeTarget = m_pZBuffer->GetImageSize() ;
			m_infZBuffer.width = sizeTarget.w ;
			m_infZBuffer.height = sizeTarget.h ;
		}
	}
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	//
	// リージョンバッファ確保
	//
	const size_t	nRegionBytes =
						sizeof(SGLRegion)
							+ m_rctView.h * sizeof(SGLRegionLine) ;
	m_pbufRegion = (SGLRegion*) m_bufRegion.GetArray( nRegionBytes ) ;
	//
	// 中間描画バッファ確保
	//
	m_countThreads = SSystem::g_cpuLogicalCount ;
	if ( m_countThreads > MAX_THREADS )
	{
		m_countThreads = MAX_THREADS ;
	}
	for ( size_t i = 0; i < m_countThreads; i ++ )
	{
		m_bufInternal[i].PrepareBuffer( m_rctView.w ) ;
	}
	return	sglErrSuccess ;
}

// バッファ準備
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::InternalBuffer::PrepareBuffer( uint32_t width )
{
	m_pbufSrcLine = m_bufSrcSampled.GetArray( width ) ;
	m_pbufDstLine = m_bufDstSampled.GetArray( width ) ;
	m_pbufZBufLine = m_bufZSampled.GetArray( width ) ;
	m_pbytBufUnformat = m_bufSrcUnformat.GetArray( width * 4 ) ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::DetachTargetImage( void )
{
	if ( (m_pTarget != NULL) && (m_pbytTarget != NULL) )
	{
		int	flags = 0 ;
		if ( m_flagUpdateTarget )
		{
			flags |= SGLImageObject::lockWrite ;
		}
		m_pTarget->UnlockBuffer( flags ) ;
	}
	if ( (m_pZBuffer != NULL) && (m_pbytZBuffer != NULL) )
	{
		int	flags = 0 ;
		if ( m_flagUpdateZBuf )
		{
			flags |= SGLImageObject::lockWrite ;
		}
		m_pZBuffer->UnlockBuffer( flags ) ;
	}
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	//
	m_pTarget = NULL ;
	m_pbytTarget = NULL ;
	m_pZBuffer = NULL ;
	m_pbytZBuffer = NULL ;
	//
	return	SGLPaintParameterContext::DetachTargetImage() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::FillClearTarget( uint32_t argb, int64_t flags )
{
	if ( (m_pTarget != NULL) && (m_pbytTarget != NULL) )
	{
		if ( (flags == 0) || (flags & clearTargetColor) )
		{
			SGLImageBuffer	imgbuf( m_infTarget ) ;
			SGLPalette	pxFill( argb ) ;
			imgbuf.ptrBuffer = m_pbytTarget ;
			imgbuf.width = m_rctView.w ;
			imgbuf.height = m_rctView.h ;
			imgbuf.ptrBuffer +=
					imgbuf.pitchPixel * m_rctView.x
						+ imgbuf.pitchLine * m_rctView.y ;
			sglFillImageBuffer( imgbuf, pxFill, NULL ) ;
			//
			/*
			if ( ((uint32_t) m_rctView.w == m_infTarget.width)
				&& ((uint32_t) m_rctView.h == m_infTarget.height) )
			{
				m_pTarget->NotifyClearImageObject( pxFill ) ;
				m_flagUpdateTarget = true ;
			}
			else
			*/
			{
				m_flagUpdateTarget = true ;
			}
		}
	}
	if ( (m_pZBuffer != NULL) && (m_pbytZBuffer != NULL) )
	{
		if ( (flags == 0) || (flags & clearTargetZBuffer) )
		{
			SGLImageBuffer	imgbuf( m_infZBuffer ) ;
			SGLPalette	pxFill( 0x7F000000 ) ;
			imgbuf.ptrBuffer = m_pbytZBuffer ;
			imgbuf.width = m_rctView.w ;
			imgbuf.height = m_rctView.h ;
			imgbuf.ptrBuffer +=
					imgbuf.pitchPixel * m_rctView.x
						+ imgbuf.pitchLine * m_rctView.y ;
			sglFillImageBuffer( imgbuf, pxFill, NULL ) ;
			//
			/*
			if ( ((uint32_t) m_rctView.w == m_infZBuffer.width)
				&& ((uint32_t) m_rctView.h == m_infZBuffer.height) )
			{
				m_pZBuffer->NotifyClearImageObject( pxFill ) ;
				m_flagUpdateZBuf = true ;
			}
			else
			*/
			{
				m_flagUpdateZBuf = true ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::FillRectangle
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
	return	SGLPaintBuffer::FillPolygon( &(vRect[0]), 4, argb, z, flags ) ;
}

SGLError SGLPaintBuffer::FillPolygon
	( const S2DVector * vertices, size_t count,
				uint32_t argb, double z, uint32_t flags )
{
	if ( (count < 3) || ((m_opFillType == fillFlat) && (argb == 0)) )
	{
		return	sglErrInvalidParam ;
	}
	if ( m_pbufRegion == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// 頂点座標変換
	//
	SGLAffine	afTransform ;
	if ( GetTransformation( afTransform ) )
	{
		m_bufPolyVertices.SetLength( count ) ;
		afTransform.TransformVectors
			( m_bufPolyVertices.GetArray(), vertices, count ) ;
		m_bufPolyVertices.FinishArray() ;
		vertices = m_bufPolyVertices.GetConstArray() ;
	}
	//
	// リージョン生成
	//
	SGLRect	rctView = m_rctView ;
	if ( !sglCreatePolygonRegion
		( m_pbufRegion, rctView, vertices, count ) )
	{
		return	sglErrSuccess ;
	}
	//
	// サンプリング関数設定
	//
	m_argbFill.ui32 = argb ;
	m_pfnSampleSrc = m_tableFillSamplingProc[m_opFillType] ;
	//
	// フィルタ関数・描画関数設定
	//
	SGLImageInfo	infSource ;
	infSource.format = formatImageARGB ;
	infSource.depth = 32 ;
	infSource.pitchPixel = 4 ;
	//
	PrepareFilterPaintProc( flags, infSource, NULL ) ;
	//
	// 描画実行
	//
	PerformPaintTransformedGeneric() ;
	//
	return	sglErrSuccess ;
}

// 点描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::DrawPoints
	( const S2DVector * pPoints, size_t nPoints,
		uint32_t argb, double z, uint32_t flags )
{
	if ( (m_opFillType == fillFlat) && (argb == 0) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 頂点座標変換
	//
	SGLAffine	afTransform ;
	bool		fTransform = GetTransformation( afTransform ) ;
	//
	// サンプリング関数設定
	//
	m_argbFill.ui32 = argb ;
	m_pfnSampleSrc = m_tableFillSamplingProc[m_opFillType] ;
	//
	// フィルタ関数・描画関数設定
	//
	SGLImageInfo	infSource ;
	infSource.format = formatImageARGB ;
	infSource.depth = 32 ;
	infSource.pitchPixel = 4 ;
	//
	PrepareFilterPaintProc( flags, infSource, NULL ) ;
	//
	// 点順次描画
	//
	SGLRect	rctView = m_rctView ;
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		S2DVector	v = pPoints[i] ;
		if ( fTransform )
		{
			afTransform.TransformVectors( &v, pPoints + i, 1 ) ;
		}
		SGLPoint	pt ;
		pt.x = eslRoundR32ToInt( v.x ) ;
		pt.y = eslRoundR32ToInt( v.y ) ;
		if ( (pt.x < rctView.left)
			|| (pt.x >= rctView.right + 1)
			|| (pt.y < rctView.top)
			|| (pt.y >= rctView.bottom + 1) )
		{
			continue ;
		}
		m_pbufRegion->yTop = pt.y ;
		m_pbufRegion->yBottom = pt.y ;
		m_pbufRegion->areaPixels = 1 ;
		m_pbufRegion->rgLine[0].fxLeft = pt.x << 16 ;
		m_pbufRegion->rgLine[0].fxRight = (pt.x << 16) | 0xFFFF ;
		//
		PerformPaintTransformedGeneric() ;
	}
	return	sglErrSuccess ;
}

// 直線描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::DrawThinLines
	( const S2DVector * pLines, size_t nLines,
		uint32_t argb, double z, uint32_t flags )
{
	if ( (m_opFillType == fillFlat) && (argb == 0) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 頂点座標変換
	//
	SGLAffine	afTransform ;
	bool		fTransform = GetTransformation( afTransform ) ;
	//
	// サンプリング関数設定
	//
	m_argbFill.ui32 = argb ;
	m_pfnSampleSrc = m_tableFillSamplingProc[m_opFillType] ;
	//
	// フィルタ関数・描画関数設定
	//
	SGLImageInfo	infSource ;
	infSource.format = formatImageARGB ;
	infSource.depth = 32 ;
	infSource.pitchPixel = 4 ;
	//
	PrepareFilterPaintProc( flags, infSource, NULL ) ;
	//
	// 線分順次描画
	//
	SGLRect	rctView = m_rctView ;
	for ( size_t i = 0; i < nLines; i ++ )
	{
		int nFlags = thinLineExcludeStart ;
		if ( i == 0 )
		{
			nFlags = 0 ;
		}
		if ( pLines[i + 1] == pLines[0] )
		{
			nFlags |= thinLineExcludeEnd ;
		}
		if ( fTransform )
		{
			S2DVector	pts[2] ;
			afTransform.TransformVectors( &pts[0], pLines + i, 2 ) ;
			if ( !sglCreateThinLineRegion
				( m_pbufRegion, rctView, pts[0], pts[1], nFlags ) )
			{
				continue ;
			}
		}
		else
		{
			if ( !sglCreateThinLineRegion
				( m_pbufRegion, rctView,
					pLines[i], pLines[i + 1], nFlags ) )
			{
				continue ;
			}
		}
		//
		// 描画実行
		//
		PerformPaintTransformedGeneric() ;
	}
	return	sglErrSuccess ;
}

// グラデーション解除
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FreeGradation( void )
{
	m_opFillType = fillFlat ;
}

// 線形グラデーション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::SetLinearGradation
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
		float32_t	s = (float32_t) (nCount - 1) / (r * r) ;
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
SGLError SGLPaintBuffer::SetRingedGradation
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

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip )
{
	if ( (pSrcImage == NULL) | (m_pbytTarget == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 変換行列取得
	//
	uint32_t	nFlags = ppPaint.nFlags | (uint32_t) m_flagsDefPaint ;
	if ( pSrcImage->GetBufferFlags() & SGLImageObject::bufferSampleNoSmooth )
	{
		nFlags |= paintUnsmoothStretch ;
	}
	bool		fRotation = false ;
	SGLAffine	afTransform ;
	S2DVector	vPaintOffset ;
	if ( nFlags & paintFixedPosition )
	{
		const float32_t	rFixedScale = (float32_t) (1.0 / 0x10000) ;
		vPaintOffset.x += (float32_t) (ppPaint.ptPaint.x * rFixedScale) ;
		vPaintOffset.y += (float32_t) (ppPaint.ptPaint.y * rFixedScale) ;
	}
	else
	{
		vPaintOffset.x += (float32_t) ppPaint.ptPaint.x ;
		vPaintOffset.y += (float32_t) ppPaint.ptPaint.y ;
	}
	if ( m_pTransformation != NULL )
	{
		if ( ppPaint.pAffine != NULL )
		{
			afTransform = *(ppPaint.pAffine) ;
		}
		afTransform.a13 += vPaintOffset.x ;
		afTransform.a23 += vPaintOffset.y ;
		afTransform = m_pTransformation->afTransform * afTransform ;
		fRotation = afTransform.IsRotation() ;
	}
	else if ( ppPaint.pAffine != NULL )
	{
		afTransform = *(ppPaint.pAffine) ;
		afTransform.a13 += vPaintOffset.x ;
		afTransform.a23 += vPaintOffset.y ;
		fRotation = afTransform.IsRotation() ;
	}
	else
	{
		afTransform.a13 = vPaintOffset.x ;
		afTransform.a23 = vPaintOffset.y ;
	}
	//
	// フィルタ・描画関数設定
	//
	SGLImageInfo	infSource ;
	pSrcImage->GetImageInfo( infSource ) ;
	//
	bool	fSrcCompressed = ((infSource.format & formatImageFlagS3TC) != 0) ;
	bool	fFilterProc =
		PrepareFilterPaintProc
			( nFlags, infSource, pSrcImage,
				ppPaint.nTransparency,
				ppPaint.zOrder, ppPaint.rgbColorParam.ui32 ) ;
	//
	// 変形描画判定
	//
	const S2DVector *	pVertices = ppPaint.pVertices ;
	bool	fPolygonShaped =
		((nFlags & paintPolygonShaped) != 0) & (pVertices != NULL) ;
	if ( (nFlags & (paintSmoothStretch | paintUnsmoothStretch))
												== paintSmoothStretch )
	{
		int32_t	xDec = (int32_t) (afTransform.a13 * 0x80) & 0x7F ;
		int32_t	yDec = (int32_t) (afTransform.a23 * 0x80) & 0x7F ;
		fRotation |= (xDec != 0) | (yDec != 0) ;
	}
	//
	SGLError	err = sglErrSuccess ;
	if ( fRotation | fPolygonShaped | fSrcCompressed )
	{
		//
		// 変形描画・入力画像取得
		//
		m_pbytSource =
			pSrcImage->LockBuffer
				( m_infSource, SGLImageObject::lockRead ) ;
		if ( m_pbytSource == NULL )
		{
			return	sglErrFailed ;
		}
		do
		{
			int	nDepth = m_infSource.depth >> 3 ;
			if ( !fSrcCompressed && ((nDepth < 1) | (nDepth > 4)) )
			{
				err = sglErrInvalidParam ;
				break ;
			}
			if ( pSrcClip != NULL )
			{
				SGLImageRect	rctSource =
					SGLRect( *pSrcClip )
						& SGLRect( m_infSource.GetImageRect() ) ;
				m_pbytSource += rctSource.x * m_infSource.pitchPixel
								+ rctSource.y * m_infSource.pitchLine ;
				m_infSource.width = rctSource.w ;
				m_infSource.height = rctSource.h ;
			}
			//
			// 頂点座標生成
			//
			S2DVector	vRect[4] ;
			uint32_t	countVertex ;
			if ( fPolygonShaped )
			{
				countVertex = ppPaint.countVertex ;
				if ( countVertex < 3 )
				{
					err = sglErrInvalidParam ;
					break ;
				}
			}
			else
			{
				vRect[0].x = 0 ;
				vRect[0].y = 0 ;
				vRect[1].x = (float32_t) m_infSource.width ;
				vRect[1].y = vRect[0].y ;
				vRect[2].x = vRect[1].x ;
				vRect[2].y = (float32_t) m_infSource.height ;
				vRect[3].x = vRect[0].x ;
				vRect[3].y = vRect[2].y ;
				//
				pVertices = &(vRect[0]) ;
				countVertex = 4 ;
			}
			m_bufPolyVertices.SetLength( countVertex ) ;
			afTransform.TransformVectors
				( m_bufPolyVertices.GetArray(), pVertices, countVertex ) ;
			m_bufPolyVertices.FinishArray() ;
			pVertices = m_bufPolyVertices.GetConstArray() ;
			//
			// リージョン生成
			//
			SGLRect	rctView = m_rctView ;
			if ( !sglCreatePolygonRegion
				( m_pbufRegion, rctView, pVertices, countVertex ) )
			{
				err = sglErrSuccess ;
				break ;
			}
			//
			// サンプリング関数設定
			//
			if ( fSrcCompressed )
			{
				m_pfnSampleSrc = &SGLPaintBuffer::SamplingRGBA_S3TC_DXT1 ;
			}
			else
			{
				if ( nFlags & paintUnsmoothStretch )
				{
					nFlags &= ~paintSmoothStretch ;
				}
				m_pfnSampleSrc =
					m_tableSamplingProc[nDepth - 1]
						[(nDepth == m_infSource.pitchPixel) & 0x01]
						[((nFlags & paintSmoothStretch) != 0) & 0x01] ;
			}
			//
			// 変換行列設定
			//
			SGLAffine	afSampling ;
			afSampling.InverseOf( afTransform ) ;
			//
			m_fxSampling00 = (int32_t) (afSampling.a11 * 0x10000) ;
			m_fxSampling10 = (int32_t) (afSampling.a21 * 0x10000) ;
			m_fxSampling01 = (int32_t) (afSampling.a12 * 0x10000) ;
			m_fxSampling11 = (int32_t) (afSampling.a22 * 0x10000) ;
			m_fxSampling02 = (int32_t) (afSampling.a13 * 0x10000) ;
			m_fxSampling12 = (int32_t) (afSampling.a23 * 0x10000) ;
			//
			// 描画実行
			//
			PerformPaintTransformedGeneric() ;
		}
		while ( false ) ;
		//
		pSrcImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		m_pbytSource = NULL ;
	}
	else
	{
		//
		// 変形無し描画・入力矩形取得
		//
		SGLPoint	ptPaint ;
		ptPaint.x = eslRoundR32ToInt( afTransform.a13 ) ;
		ptPaint.y = eslRoundR32ToInt( afTransform.a23 ) ;
		//
		SGLImageRect	rctSource ;
		if ( pSrcClip != NULL )
		{
			rctSource = SGLRect( *pSrcClip )
							& SGLRect( infSource.GetImageRect() ) ;
		}
		else
		{
			rctSource = infSource.GetImageRect() ;
		}
		//
		// 描画矩形取得
		//
		SGLImageRect	rctDstView = m_rctView ;
		if ( ptPaint.x < rctDstView.x )
		{
			int32_t	xCut = rctDstView.x - ptPaint.x ;
			ptPaint.x = rctDstView.x ;
			rctSource.x += xCut ;
			rctSource.w -= xCut ;
		}
		if ( ptPaint.y < rctDstView.y )
		{
			int32_t	yCut = rctDstView.y - ptPaint.y ;
			ptPaint.y = rctDstView.y ;
			rctSource.y += yCut ;
			rctSource.h -= yCut ;
		}
		if ( ptPaint.x + rctSource.w > rctDstView.x + rctDstView.w )
		{
			int32_t	xCut = (ptPaint.x + rctSource.w)
								- (rctDstView.x + rctDstView.w) ;
			rctSource.w -= xCut ;
		}
		if ( ptPaint.y + rctSource.h > rctDstView.y + rctDstView.h )
		{
			int32_t	yCut = (ptPaint.y + rctSource.h)
								- (rctDstView.y + rctDstView.h) ;
			rctSource.h -= yCut ;
		}
		if ( rctSource.IsEmpty() )
		{
			return	sglErrSuccess ;
		}
		//
		// 描画矩形情報取得
		//
		m_pbytSource =
			pSrcImage->LockBuffer
				( m_infSource, SGLImageObject::lockRead, &rctSource ) ;
		if ( m_pbytSource == NULL )
		{
			return	sglErrFailed ;
		}
		do
		{
			m_pbytRectDst =
				m_pbytTarget + (ptPaint.y * m_infTarget.pitchLine
									+ ptPaint.x * m_infTarget.pitchPixel) ;
			m_pbytRectZBuf =
				m_pbytZBuffer + (ptPaint.y * m_infTarget.pitchLine
									+ ptPaint.x * m_infTarget.pitchPixel) ;
			//
			// 描画実行
			//
			if ( !fFilterProc & (m_pfnConvertSrc == NULL) )
			{
				PerformPaintRectSimple() ;
			}
			else
			{
				if ( m_pfnConvertSrc == NULL )
				{
					m_pfnConvertSrc =
						&SakuraGL::sglDecodePixelCompositionARGB32 ;
				}
				PerformPaintRectGeneric() ;
			}
		}
		while ( false ) ;
		//
		pSrcImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		m_pbytSource = NULL ;
	}
	return	err ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLPaintBuffer::Flush( void )
{
	if ( (m_pTarget != NULL) && (m_pbytTarget != NULL) )
	{
		if ( m_flagUpdateTarget )
		{
			m_pTarget->FlushBuffer( SGLImageObject::lockWrite ) ;
		}
		else
		{
			m_pTarget->FlushBuffer( 0 ) ;
		}
//		m_pbytTarget = m_pTarget->LockBuffer( m_infTarget ) ;
	}
	if ( (m_pZBuffer != NULL) && (m_pbytZBuffer != NULL) )
	{
		if ( m_flagUpdateZBuf )
		{
			m_pZBuffer->FlushBuffer( SGLImageObject::lockWrite ) ;
		}
		else
		{
			m_pZBuffer->FlushBuffer( 0 ) ;
		}
//		m_pbytZBuffer = m_pZBuffer->LockBuffer( m_infZBuffer ) ;
	}
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	return	sglErrSuccess ;
}

SGLError SGLPaintBuffer::Finish( void )
{
	if ( (m_pTarget != NULL) && (m_pbytTarget != NULL) )
	{
		if ( m_flagUpdateTarget )
		{
			m_pTarget->FlushBuffer( SGLImageObject::lockWrite ) ;
		}
		else
		{
			m_pTarget->FlushBuffer( 0 ) ;
		}
//		m_pbytTarget = m_pTarget->LockBuffer( m_infTarget ) ;
	}
	if ( (m_pZBuffer != NULL) && (m_pbytZBuffer != NULL) )
	{
		if ( m_flagUpdateZBuf )
		{
			m_pZBuffer->FlushBuffer( SGLImageObject::lockWrite ) ;
		}
		else
		{
			m_pZBuffer->FlushBuffer( 0 ) ;
		}
//		m_pbytZBuffer = m_pZBuffer->LockBuffer( m_infZBuffer ) ;
	}
	m_flagUpdateTarget = false ;
	m_flagUpdateZBuf = false ;
	return	sglErrSuccess ;
}

// フィルタ関数・ストア関数・描画関数設定
//////////////////////////////////////////////////////////////////////////////
bool SGLPaintBuffer::PrepareFilterPaintProc
	( uint32_t nFlags,
		const SGLImageInfo& infSrc, SGLImageObject * pSrcImage,
		uint32_t nTransparency, float32_t zOrder, uint32_t argbColorParam )
{
	//
	// 透明度正規化
	//
	if ( nTransparency > 0x100 )
	{
		nTransparency = 0x100 ;
	}
	if ( m_pTransformation != NULL )
	{
		nTransparency =
			0x100 - (0x100 - nTransparency)
				* (0x100 - m_pTransformation->nTransparency) / 0x100 ;
	}
	//
	// 関数初期化
	//
	bool	fFilterProc = false ;
	bool	fSrcCompressed = false ;
	m_pfnNormalizeSrc = NULL ;
	m_pfnConvertSrc = NULL ;
	m_pfnSampleDst = NULL ;
	m_pfnSampleZBuf = NULL ;
	m_pfnFilterSrc = NULL ;
	m_pfnFilterPost = NULL ;
	m_pfnStoreDst = NULL ;
	m_pfnPaintLine = NULL ;
	m_pfnPaintPost = NULL ;
	//
	// サンプリング関数取得
	//
	uint32_t	fmtSrc = infSrc.format ;
	uint32_t	depthSrc = infSrc.depth ;
	if ( fmtSrc & formatImageFlagS3TC )
	{
		fmtSrc = formatImageRGB | (fmtSrc & formatImageFlagAlpha) ;
		depthSrc = 32 ;
		fSrcCompressed = true ;
	}
	if ( ((fmtSrc & formatImageTypeMask) != formatImageGray)
		& (!(fmtSrc & formatImageFlagAlpha)
			| ((fmtSrc & formatImageTypeMask)
					!= (m_infTarget.format & formatImageTypeMask))) )
	{
		m_pfnNormalizeSrc =
			sglGetColorFormatConvertor( m_infTarget.format, fmtSrc ) ;
		fFilterProc |= (m_pfnNormalizeSrc != NULL) ;
	}
	m_pfnConvertSrc =
		sglGetPixelCompositionDecoder( fmtSrc, depthSrc ) ;
	fFilterProc |= (m_pfnConvertSrc != NULL) ;
	//
	// 出力先フォーマット変換関数取得
	//
	if ( m_infTarget.pitchPixel != 4 )
	{
		m_pfnSampleDst =
			sglGetPixelCompositionDecoder
				( m_infTarget.format, m_infTarget.depth ) ;
		m_pfnStoreDst =
			sglGetPixelCompositionEncoder
				( m_infTarget.format, m_infTarget.depth ) ;
		fFilterProc |= (m_pfnSampleDst != NULL) | (m_pfnStoreDst != NULL) ;
	}
	//
	// フィルタ関数取得
	//
	if ( nFlags & paintMaskApply )
	{
		m_pfnFilterSrc =
			m_tableFilterProc[(nFlags >> paintApplyShifter) & 0x0F] ;
		fFilterProc |= (m_pfnFilterSrc != NULL) ;
	}
	if ( (fmtSrc & formatImageFlagNoProductOfAlpha)
			&& ((nFlags & paintMaskApply) != paintApplyAlphaMul) )
	{
		m_pfnFilterPost = m_pfnFilterSrc ;
		m_pfnFilterSrc = &SGLPaintBuffer::FilterAlphaMulProc ;
		fFilterProc = true ;
	}
	if ( (fmtSrc & formatImageFlagClipping)
			& ((nFlags & paintMaskApply) != paintApplyColorMask) )
	{
		argbColorParam = infSrc.colorClip ;
		m_pfnFilterPost = m_pfnFilterSrc ;
		m_pfnFilterSrc = &SGLPaintBuffer::FilterColorMaskProc ;
		fFilterProc = true ;
	}
	if ( fmtSrc & formatImageFlagPalette )
	{
		pSrcImage->GetPaletteTable( &(m_tableSrcPalette[0]), 0x100 ) ;
		m_pfnFilterPost = m_pfnFilterSrc ;
		m_pfnFilterSrc = &SGLPaintBuffer::FilterIndexedPaletteProc ;
		fFilterProc = true ;
	}
	//
	// 描画関数取得
	//
	const size_t	funcPaint = (nFlags >> paintFunctionShifter) & 0x0F ;
	if ( nFlags & paintMaskFunction )
	{
		m_pfnPaintLine = m_tablePaintLineProc[0][funcPaint] ;
	}
	//
	// 描画機能の組み合わせ判定
	//
	bool	fNeedsMulTable = false ;
	if ( m_pbytZBuffer == NULL )
	{
		nFlags &= ~paintWithZOrder ;
	}
	m_nTransparency = nTransparency ;
	m_zOrder = zOrder ;
	m_argbColorParam1.ui32 = argbColorParam ;
	//
	if ( !(nFlags & paintWithZOrder) & (m_pfnPaintLine == NULL) )
	{
		const PAINT_PROC_ENTRY & procPaintEntry =
			m_tableNormalPaintProc
				[((fmtSrc & formatImageFlagAlpha) != 0) & 0x01]
				[(nTransparency != 0) & 0x01] ;
		m_pfnPaintLine = procPaintEntry.pfnPaintProc ;
		fNeedsMulTable = procPaintEntry.fNeedsMulTable ;
	}
	else
	{
		if ( nTransparency != 0 )
		{
			PROC_PAINT_PIXELS
				pfnPaintLine = m_tablePaintLineProc[1][funcPaint] ;
			if ( pfnPaintLine != NULL )
			{
				m_pfnPaintLine = pfnPaintLine ;
				fNeedsMulTable = true ;
			}
			else
			{
				if ( m_pfnFilterSrc == NULL )
				{
					m_pfnFilterSrc = &SGLPaintBuffer::FilterTransparencyProc ;
				}
				else
				{
					m_pfnFilterPost = &SGLPaintBuffer::FilterTransparencyProc ;
				}
				#if	!defined(__COTOPHA__)
				fNeedsMulTable = true ;
				#endif
				fFilterProc = true ;
			}
		}
		if ( nFlags & paintWithZOrder )
		{
			if ( m_pfnPaintLine == NULL )
			{
				m_pfnPaintLine = &SGLPaintBuffer::PaintBlendWithZProc ;
			}
			else
			{
				m_pfnPaintPost = &SGLPaintBuffer::PaintWriteWithZProc ;
			}
			if ( nFlags & paintWithZOrderNoWrite )
			{
				m_pfnSampleZBuf = &sglDecodePixelCompositionARGB32 ;
			}
			else
			{
				m_flagUpdateZBuf = true ;
			}
			fFilterProc = true ;
		}
	}
	if ( fNeedsMulTable )
	{
		uint32_t	i, vDst = 0, vSrc = 0 ;
		uint32_t	nOpacity = 0x100 - nTransparency ;
		for ( i = 0; i < 0x100; ++ i )
		{
			m_mulDstTable[i] = (uint8_t) (vDst >> 8) ;
			m_mulSrcTable[i] = (uint8_t) (vSrc >> 8) ;
			vDst += nTransparency ;
			vSrc += nOpacity ;
		}
	}
	m_flagUpdateTarget = true ;
	return	fFilterProc ;
}


//////////////////////////////////////////////////////////////////////////////
// サンプリング関数
//////////////////////////////////////////////////////////////////////////////

// fill color
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::SamplingFillColorProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	uint32_t	argbFill = m_argbFill.ui32 ;
	//
	while ( nPixels != 0 )
	{
		pargbDst->ui32 = argbFill ;
		//
		++ pargbDst ;
		-- nPixels ;
	}
}

// linear gradation color
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::SamplingLinearGradationProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	float32_t	dx = (float32_t) xDst - m_vGradationCenter.x ;
	float32_t	dy = (float32_t) yDst - m_vGradationCenter.y ;
	int32_t		fxDegree =
					eslRoundR32ToInt
						( (dx * m_vGradationDelta.x
							+ dy * m_vGradationDelta.y) * 0x10000 ) ;
	int32_t		fxDelta =
					eslRoundR32ToInt( m_vGradationDelta.x * 0x10000 ) ;
	const SGLPalette *
				pGradarions = m_aGradation.GetConstArray() ;
	const int	nGradations = (int) m_aGradation.GetLength() - 1 ;
	//
	while ( nPixels != 0 )
	{
		const int	i = esl_clampi
							( (int) (fxDegree >> 16), 0, nGradations ) ;
		const int	j = esl_clampi
							( (i + 1), 0, nGradations ) ;
		uint32_t	nDec = (uint32_t) (fxDegree >> 8) & 0xFF ;
		//
		*pargbDst = pGradarions[i].imul(0x100 - nDec)
						+ pGradarions[j].imul(nDec) ;
		//
		fxDegree += fxDelta ;
		++ pargbDst ;
		-- nPixels ;
	}
}

// ringed gradation color
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::SamplingRingedGradationProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	const SGLPalette *
				pGradarions = m_aGradation.GetConstArray() ;
	const int	nGradations = (int) m_aGradation.GetLength() - 1 ;
	//
	double		dx = (double) xDst - m_vGradationCenter.x ;
	double		dy = (double) yDst - m_vGradationCenter.y ;
	double		r = (double) (nGradations - 1) / (2.0 * PI) ;
	//
	while ( nPixels != 0 )
	{
		float32_t		a = (float32_t) ((atan2( - dy, dx )
											+ m_radGradation) * r) ;
		uint32_t	i = (uint32_t) eslRoundR32ToInt( a * 0x100 ) ;
		uint32_t	nDec = i % 0xFF ;
		i %= (uint32_t) nGradations ;
		//
		*pargbDst = pGradarions[i].imul(0x100 - nDec)
						+ pGradarions[i + 1].imul(nDec) ;
		//
		dx += 1.0f ;
		++ pargbDst ;
		-- nPixels ;
	}
}

// source 32bit 4pitch
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::Sampling32bitsProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	SGLPalette *	pargbDstNext = pargbDst ;
	uint32_t		nCount = (uint32_t) nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	//
#if	defined(__COTOPHA__)
	asm
	{
		REG ALLOC	xmm(8) : int64
		REG ALLOC	xmmAddrMul(2) : int64
		REG ALLOC	xmmAddrAdd(2) : int64
		REG ALLOC	xmmSatCoord(2) : int64
		REG ALLOC	xmmAddrHigh(2) : int64
		REG ALLOC	xmmPackedPos : int64
		REG ALLOC	xmmPackedDeltaPos : int64
		//
		move		xmmPackedPos, fxSrcX
		move		xmm(0), fxSrcY
		punpack.ldq	xmmPackedPos, xmm(0)
		//
		move		xmmPackedDeltaPos, [tp].m_fxSampling00
		move		xmm(0), [tp].m_fxSampling10
		punpack.ldq	xmmPackedDeltaPos, xmm(0)
		//
		move		xmmAddrMul(0), pitchPixel
		move		xmmAddrMul(1), pitchLine
		punpack.lwd	xmmAddrMul(0), xmmAddrMul(1)
		punpack.ldq	xmmAddrMul(0), xmmAddrMul(0)
		move		xmmAddrMul(1), xmmAddrMul(0)
		//
		move		xmmAddrAdd(0), pbytSource
		srl			xmmAddrHigh(0), xmmAddrAdd(0), 32
		punpack.ldq	xmmAddrAdd(0), xmmAddrAdd(0)
		punpack.ldq	xmmAddrHigh(0), xmmAddrHigh(0)
		move		xmmAddrAdd(1), xmmAddrAdd(0)
		move		xmmAddrHigh(1), xmmAddrHigh(0)
		//
		move		xmm(0), widthSrc
		move		xmm(1), heightSrc
		sub			xmm(0), #one
		sub			xmm(1), #one
		punpack.lwd	xmm(0), xmm(1)
		punpack.ldq	xmm(0), xmm(0)
		psrl.w		xmmSatCoord(0), #fill, 1
		xor			xmmSatCoord(0), xmm(0)
		move		xmmSatCoord(1), xmmSatCoord(0)
		//
		REG LOAD	pargbDstNext
		REG LOAD	nCount
		REG ALLOC	nCountX4 : int64
		move		xmm(0), 3
		srl			nCountX4, nCount, 2
		and			nCount, xmm(0)
		//
		.WHILE		(uint32) nCountX4 != (uint32) #zero
			psra.d		xmm(0), xmmPackedPos, 16
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			psra.d		xmm(1), xmmPackedPos, 16
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			pcvt.sdw	xmm(0), xmm(1)
			//
			psra.d		xmm(1), xmmPackedPos, 16
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			psra.d		xmm(2), xmmPackedPos, 16
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			pcvt.sdw	xmm(1), xmm(2)
			//
			padd.sw		xmm(0), xmmSatCoord(0)
			padd.sw		xmm(1), xmmSatCoord(1)
			psub.uw		xmm(0), xmmSatCoord(0)
			psub.uw		xmm(1), xmmSatCoord(1)
			//
			pmadd.wd	xmm(0), xmmAddrMul(0)
			pmadd.wd	xmm(1), xmmAddrMul(1)
			padd.d		xmm(0), xmmAddrAdd(0)
			padd.d		xmm(1), xmmAddrAdd(1)
			srl			xmm(2), xmm(0), 32
			srl			xmm(3), xmm(1), 32
			//
			punpack.ldq	xmm(0), xmmAddrHigh(0)
			punpack.ldq	xmm(1), xmmAddrHigh(0)
			punpack.ldq	xmm(2), xmmAddrHigh(0)
			punpack.ldq	xmm(3), xmmAddrHigh(0)
			//
			load.uint32	xmm(0), [xmm(0)]
			load.uint32	xmm(1), [xmm(1)]
			load.uint32	xmm(2), [xmm(2)]
			load.uint32	xmm(3), [xmm(3)]
			punpack.ldq	xmm(0), xmm(2)
			punpack.ldq	xmm(1), xmm(3)
			//
			store.64	[pargbDstNext], xmm(0)
			store.64	[pargbDstNext+8], xmm(1)
			//
			add			pargbDstNext, 16
			dec			nCountX4
		.ENDW
		//
		.WHILE		(uint32) nCount != (uint32) #zero
			psra.d		xmm(0), xmmPackedPos, 16
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			pcvt.sdw	xmm(0), xmm(0)
			//
			padd.sw		xmm(0), xmmSatCoord(0)
			psub.uw		xmm(0), xmmSatCoord(0)
			//
			pmadd.wd	xmm(0), xmmAddrMul(0)
			padd.d		xmm(0), xmmAddrAdd(0)
			//
			punpack.ldq	xmm(0), xmmAddrHigh(0)
			//
			load.uint32	xmm(0), [xmm(0)]
			store.uint32	[pargbDstNext], xmm(0)
			//
			add			pargbDstNext, 4
			dec			nCount
		.ENDW
	}
#else
	while ( nCount != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		pargbDstNext->ui32
			= *((uint32_t*)(pbytSource
							+ ((pitchLine * yPos) + (xPos << 2)))) ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
		++ pargbDstNext ;
		-- nCount ;
	}
#endif
}

void SGLPaintBuffer::Sampling32bitsNpitchProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	SGLPalette *	pargbDstNext = pargbDst ;
	uint32_t		nCount = (uint32_t) nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	//
	while ( nCount != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		pargbDstNext->ui32
			= *((uint32_t*)(pbytSource
						+ ((pitchLine * yPos) + (pitchPixel * xPos)))) ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
		++ pargbDstNext ;
		-- nCount ;
	}
}

void SGLPaintBuffer::SamplingSmooth32bitsProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	SGLPalette *	pargbDstNext = pargbDst ;
	uint32_t		nCount = (uint32_t) nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	//
#if	defined(__COTOPHA__)
	asm
	{
		REG ALLOC	xmm(8) : int64
		REG ALLOC	xmmAddrMul(2) : int64
		REG ALLOC	xmmAddrAdd(2) : int64
		REG ALLOC	xmmSatCoord(2) : int64
		REG ALLOC	xmmOffsetCoord(2) : int64
		REG ALLOC	xmmAddrHigh(2) : int64
		REG ALLOC	xmmPackedPos : int64
		REG ALLOC	xmmPackedDeltaPos : int64
		REG ALLOC	xmmDecimalX(2) : int64
		REG ALLOC	xmmDecimalY : int64
		REG ALLOC	xmmDecimalX2(2) : int64
		REG ALLOC	xmmDecimalY2 : int64
		REG ALLOC	xmmMask7F(2) : int64
		//
		move		xmmPackedPos, fxSrcX
		move		xmm(0), fxSrcY
		punpack.ldq	xmmPackedPos, xmm(0)
		//
		move		xmmPackedDeltaPos, [tp].m_fxSampling00
		move		xmm(0), [tp].m_fxSampling10
		punpack.ldq	xmmPackedDeltaPos, xmm(0)
		//
		move		xmmAddrMul(0), pitchPixel
		move		xmmAddrMul(1), pitchLine
		punpack.lwd	xmmAddrMul(0), xmmAddrMul(1)
		punpack.ldq	xmmAddrMul(0), xmmAddrMul(0)
		move		xmmAddrMul(1), xmmAddrMul(0)
		//
		move		xmmAddrAdd(0), pbytSource
		srl			xmmAddrHigh(0), xmmAddrAdd(0), 32
		punpack.ldq	xmmAddrAdd(0), xmmAddrAdd(0)
		punpack.ldq	xmmAddrHigh(0), xmmAddrHigh(0)
		move		xmmAddrAdd(1), xmmAddrAdd(0)
		move		xmmAddrHigh(1), xmmAddrHigh(0)
		//
		move		xmm(0), widthSrc
		move		xmm(1), heightSrc
		sub			xmm(0), #one
		sub			xmm(1), #one
		punpack.lwd	xmm(0), xmm(1)
		punpack.ldq	xmm(0), xmm(0)
		psrl.w		xmmSatCoord(0), #fill, 1
		xor			xmmSatCoord(0), xmm(0)
		move		xmmSatCoord(1), xmmSatCoord(0)
		//
		move		xmmOffsetCoord(0), 0x0000000100000000
		move		xmmOffsetCoord(1), 0x0001000100010000
		//
		move		xmmMask7F(0), 0x7F
		move		xmmMask7F(1), 0x7F
		//
		REG LOAD	pargbDstNext
		REG LOAD	nCount
		REG ALLOC	nCountX2 : int64
		//
		srl			nCountX2, nCount, 1
		and			nCount, #one
		//
		move		xmm(0), xmmAddrHigh
		move		xmm(1), xmmAddrHigh
		move		xmm(2), xmmAddrHigh
		move		xmm(3), xmmAddrHigh
		move		xmm(4), xmmAddrHigh
		move		xmm(5), xmmAddrHigh
		move		xmm(6), xmmAddrHigh
		move		xmm(7), xmmAddrHigh
		prefetch.tlb0	xmm(0)
		prefetch.tlb0	xmm(1)
		prefetch.tlb0	xmm(2)
		prefetch.tlb0	xmm(3)
		prefetch.tlb0	xmm(4)
		prefetch.tlb0	xmm(5)
		prefetch.tlb0	xmm(6)
		prefetch.tlb0	xmm(7)
		prefetch.tlb1	pargbDstNext
		//
		.WHILE		(uint32) nCountX2 != (uint32) #zero
			psra.d		xmm(0), xmmPackedPos, 16
			psrl.d		xmmDecimalX, xmmPackedPos, 9
			srl			xmmDecimalY, xmmPackedPos, 32+9
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			and			xmmDecimalX, xmmMask7F
			and			xmmDecimalY, xmmMask7F
			pshuf.w		xmmDecimalX, xmmDecimalX, 0
			pshuf.w		xmmDecimalY, xmmDecimalY, 0
			//
				psra.d		xmm(4), xmmPackedPos, 16
			//
			pcvt.sdw	xmm(0), xmm(0)
				pcvt.sdw	xmm(4), xmm(4)
			punpack.ldq	xmm(0), xmm(0)
				punpack.ldq	xmm(4), xmm(4)
			move		xmm(1), xmm(0)
				move		xmm(5), xmm(4)
			padd.sw		xmm(0), xmmOffsetCoord(0)
			padd.sw		xmm(1), xmmOffsetCoord(1)
				padd.sw		xmm(4), xmmOffsetCoord(0)
				padd.sw		xmm(5), xmmOffsetCoord(1)
			//
			padd.sw		xmm(0), xmmSatCoord(0)
			padd.sw		xmm(1), xmmSatCoord(1)
				padd.sw		xmm(4), xmmSatCoord(0)
				padd.sw		xmm(5), xmmSatCoord(1)
			psub.uw		xmm(0), xmmSatCoord(0)
			psub.uw		xmm(1), xmmSatCoord(1)
				psub.uw		xmm(4), xmmSatCoord(0)
				psub.uw		xmm(5), xmmSatCoord(1)
			//
			pmadd.wd	xmm(0), xmmAddrMul(0)
			pmadd.wd	xmm(1), xmmAddrMul(1)
				pmadd.wd	xmm(4), xmmAddrMul(0)
				pmadd.wd	xmm(5), xmmAddrMul(1)
				psrl.d		xmmDecimalX2, xmmPackedPos, 9
				srl			xmmDecimalY2, xmmPackedPos, 32+9
				padd.d		xmmPackedPos, xmmPackedDeltaPos
				and			xmmDecimalX2, xmmMask7F
				and			xmmDecimalY2, xmmMask7F
				pshuf.w		xmmDecimalX2, xmmDecimalX2, 0
				pshuf.w		xmmDecimalY2, xmmDecimalY2, 0
			padd.d		xmm(0), xmmAddrAdd(0)
			padd.d		xmm(1), xmmAddrAdd(1)
				padd.d		xmm(4), xmmAddrAdd(0)
				padd.d		xmm(5), xmmAddrAdd(1)
			//
			srl			xmm(2), xmm(0), 32
			srl			xmm(3), xmm(1), 32
			punpack.ldq	xmm(0), xmmAddrHigh(0)
			punpack.ldq	xmm(1), xmmAddrHigh(0)
			punpack.ldq	xmm(2), xmmAddrHigh(0)
			punpack.ldq	xmm(3), xmmAddrHigh(0)
			//
			load.uint32	xmm(0), [xmm(0)]
			load.uint32	xmm(1), [xmm(1)]
			load.uint32	xmm(2), [xmm(2)]
			load.uint32	xmm(3), [xmm(3)]
			//
			punpack.lbw	xmm(0), #zero
			punpack.lbw	xmm(1), #zero
			punpack.lbw	xmm(2), #zero
			punpack.lbw	xmm(3), #zero
			//
			move		xmmDecimalX(1), xmmDecimalX(0)
			psub.w		xmm(2), xmm(0)
			psub.w		xmm(3), xmm(1)
			pmul.lw		xmm(2), xmmDecimalX(0)
			pmul.lw		xmm(3), xmmDecimalX(1)
				srl			xmm(6), xmm(4), 32
				srl			xmm(7), xmm(5), 32
				punpack.ldq	xmm(4), xmmAddrHigh(0)
				punpack.ldq	xmm(5), xmmAddrHigh(0)
				punpack.ldq	xmm(6), xmmAddrHigh(0)
				punpack.ldq	xmm(7), xmmAddrHigh(0)
				load.uint32	xmm(4), [xmm(4)]
				load.uint32	xmm(5), [xmm(5)]
			psra.w		xmm(2), 7
			psra.w		xmm(3), 7
			padd.w		xmm(0), xmm(2)
			padd.w		xmm(1), xmm(3)
			psub.w		xmm(1), xmm(0)
			pmul.lw		xmm(1), xmmDecimalY
				load.uint32	xmm(2), [xmm(6)]
				load.uint32	xmm(3), [xmm(7)]
			psra.w		xmm(1), 7
			padd.w		xmm(0), xmm(1)
				punpack.lbw	xmm(4), #zero
				punpack.lbw	xmm(5), #zero
				punpack.lbw	xmm(2), #zero
				punpack.lbw	xmm(3), #zero
				move		xmmDecimalX2(1), xmmDecimalX2(0)
				psub.w		xmm(2), xmm(4)
				psub.w		xmm(3), xmm(5)
				pmul.lw		xmm(2), xmmDecimalX2(0)
				pmul.lw		xmm(3), xmmDecimalX2(1)
				psra.w		xmm(2), 7
				psra.w		xmm(3), 7
				padd.w		xmm(4), xmm(2)
				padd.w		xmm(5), xmm(3)
				psub.w		xmm(5), xmm(4)
				pmul.lw		xmm(5), xmmDecimalY2
				psra.w		xmm(5), 7
				padd.w		xmm(4), xmm(5)
			pcvt.uswb	xmm(0), xmm(4)
			store.64	[pargbDstNext], xmm(0)
			//
			add			pargbDstNext, 8
			dec			nCountX2
		.ENDW
		//
		.WHILE		(uint32) nCount != (uint32) #zero
			psra.d		xmm(0), xmmPackedPos, 16
			pcvt.sdw	xmm(0), xmm(0)
			punpack.ldq	xmm(0), xmm(0)
			move		xmm(1), xmm(0)
			padd.sw		xmm(0), xmmOffsetCoord(0)
			padd.sw		xmm(1), xmmOffsetCoord(1)
			//
			padd.sw		xmm(0), xmmSatCoord(0)
			padd.sw		xmm(1), xmmSatCoord(1)
			psub.uw		xmm(0), xmmSatCoord(0)
			psub.uw		xmm(1), xmmSatCoord(1)
			//
			pmadd.wd	xmm(0), xmmAddrMul(0)
			pmadd.wd	xmm(1), xmmAddrMul(1)
			padd.d		xmm(0), xmmAddrAdd(0)
			padd.d		xmm(1), xmmAddrAdd(1)
			//
			srl			xmm(2), xmm(0), 32
			srl			xmm(3), xmm(1), 32
			punpack.ldq	xmm(0), xmmAddrHigh(0)
			punpack.ldq	xmm(1), xmmAddrHigh(0)
			punpack.ldq	xmm(2), xmmAddrHigh(0)
			punpack.ldq	xmm(3), xmmAddrHigh(0)
			//
			load.uint32	xmm(0), [xmm(0)]
			load.uint32	xmm(1), [xmm(1)]
			load.uint32	xmm(2), [xmm(2)]
			load.uint32	xmm(3), [xmm(3)]
			//
			punpack.lbw	xmm(0), #zero
			punpack.lbw	xmm(1), #zero
			punpack.lbw	xmm(2), #zero
			punpack.lbw	xmm(3), #zero
			//
			psrl.d		xmmDecimalX, xmmPackedPos, 9
			and			xmmDecimalX, xmmMask7F(0)
			pshuf.w		xmmDecimalX, xmmDecimalX, 0
			move		xmmDecimalX(1), xmmDecimalX(0)
			psub.w		xmm(2), xmm(0)
			psub.w		xmm(3), xmm(1)
			pmul.lw		xmm(2), xmmDecimalX(0)
			pmul.lw		xmm(3), xmmDecimalX(1)
			srl			xmmDecimalY, xmmPackedPos, 32+9
			and			xmmDecimalY, xmmMask7F(0)
			pshuf.w		xmmDecimalY, xmmDecimalY, 0
			psra.w		xmm(2), 7
			psra.w		xmm(3), 7
			padd.w		xmm(0), xmm(2)
			padd.w		xmm(1), xmm(3)
			psub.w		xmm(1), xmm(0)
			pmul.lw		xmm(1), xmmDecimalY
			psra.w		xmm(1), 7
			padd.w		xmm(0), xmm(1)
			pcvt.uswb	xmm(0), xmm(0)
			//
			store.uint32	[pargbDstNext], xmm(0)
			//
			padd.d		xmmPackedPos, xmmPackedDeltaPos
			add			pargbDstNext, 4
			dec			nCount
		.ENDW
		//
		unfetch.tlb0	xmm(0)
		unfetch.tlb0	xmm(1)
		unfetch.tlb0	xmm(2)
		unfetch.tlb0	xmm(3)
		unfetch.tlb0	xmm(4)
		unfetch.tlb0	xmm(5)
		unfetch.tlb0	xmm(6)
		unfetch.tlb0	xmm(7)
		unfetch.tlb1	pargbDstNext
	}
#else
	while ( nCount != 0 )
	{
		int32_t		xPos0 = (fxSrcX >> 16) ;
		int32_t		yPos0 = (fxSrcY >> 16) ;
		uint32_t	xDecimal = (fxSrcX >> 8) & 0xFF ;
		uint32_t	yDecimal = (fxSrcY >> 8) & 0xFF ;
		//
		if ( (uint32_t) xPos0 >= widthSrc )
		{
			xPos0 = ~(xPos0 >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos0 >= heightSrc )
		{
			yPos0 = ~(yPos0 >> 31) & (heightSrc - 1) ;
		}
		int32_t	xPos1 = xPos0 + 1 ;
		int32_t	yPos1 = yPos0 + 1 ;
		if ( (uint32_t) xPos1 >= widthSrc )
		{
			xPos1 = ~(xPos1 >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos1 >= heightSrc )
		{
			yPos1 = ~(yPos1 >> 31) & (heightSrc - 1) ;
		}
		//
		uint32_t*	pdwSrcLine0 =
						(uint32_t*) (pbytSource + (pitchLine * yPos0)) ;
		uint32_t	argbDst00 = pdwSrcLine0[xPos0] ;
		uint32_t	argbDst01 = pdwSrcLine0[xPos1] ;
		//
		uint32_t*	pdwSrcLine1 =
						(uint32_t*) (pbytSource + (pitchLine * yPos1)) ;
		uint32_t	argbDst10 = pdwSrcLine1[xPos0] ;
		uint32_t	argbDst11 = pdwSrcLine1[xPos1] ;
		//
		argbDst00 = sglPackedColorMul(argbDst00,0x100-xDecimal)
						+ sglPackedColorMul(argbDst01,xDecimal) ;
		argbDst10 = sglPackedColorMul(argbDst10,0x100-xDecimal)
						+ sglPackedColorMul(argbDst11,xDecimal) ;
		pargbDstNext->ui32 =
					sglPackedColorMul(argbDst00,0x100-yDecimal)
						+ sglPackedColorMul(argbDst10,yDecimal) ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
		++ pargbDstNext ;
		-- nCount ;
	}
#endif
}

// source 24bit
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::Sampling24bitsNpitchProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	const uint8_t *	pbytPixel ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	//
	while ( nPixels != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		pbytPixel = pbytSource + ((pitchLine * yPos) + (pitchPixel * xPos)) ;
		pargbDst->argb.Blue  = pbytPixel[0] ;
		pargbDst->argb.Green = pbytPixel[1] ;
		pargbDst->argb.Red   = pbytPixel[2] ;
		pargbDst->argb.Alpha = 0xFF ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
		++ pargbDst ;
		-- nPixels ;
	}
}

// source 16bit
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::Sampling16bitsNpitchProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	const uint8_t *	pbytPixel ;
	SGLPalette *	pargbDstFirst = pargbDst ;
	const size_t	nCount = nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	uint8_t *		pbytSrcUnformat = pbytBufUnformat ;
	//
	while ( nPixels != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		pbytPixel = pbytSource
						+ ((pitchLine * yPos)
						+ (pitchPixel * xPos)) ;
		*((uint16_t*)pbytSrcUnformat) = *((uint16_t*)pbytPixel) ;
		pbytSrcUnformat += 2 ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
//		++ pargbDst ;
		-- nPixels ;
	}
	m_pfnConvertSrc( pargbDstFirst, pbytBufUnformat, nCount ) ;
}

// source 8bit
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::Sampling8bitsNpitchProc
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	SGLPalette *	pargbDstFirst = pargbDst ;
	const size_t	nCount = nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	uint8_t *		pbytSrcUnformat = pbytBufUnformat ;
	//
	while ( nPixels != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		*(pbytSrcUnformat ++) =
				pbytSource[(pitchLine * yPos)
							+ (pitchPixel * xPos)] ;
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
//		++ pargbDst ;
		-- nPixels ;
	}
	m_pfnConvertSrc( pargbDstFirst, pbytBufUnformat, nCount ) ;
}

// source S3TC DXT1 RGBA
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::SamplingRGBA_S3TC_DXT1
	( SGLPalette * pargbDst,
		int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat )
{
	int32_t	fxSrcX =
			(int32_t) (m_fxSampling00 * xDst
						+ m_fxSampling01 * yDst + m_fxSampling02) ;
	int32_t	fxSrcY =
			(int32_t) (m_fxSampling10 * xDst
						+ m_fxSampling11 * yDst + m_fxSampling12) ;
	SGLPalette *	pargbDstFirst = pargbDst ;
	const size_t	nCount = nPixels ;
	const uint8_t *	pbytSource = m_pbytSource ;
	const uint32_t	widthSrc = m_infSource.width ;
	const uint32_t	heightSrc = m_infSource.height ;
	const int32_t	pitchPixel = m_infSource.pitchPixel ;
	const int32_t	pitchLine = m_infSource.pitchLine ;
	uint8_t *		pbytSrcUnformat = pbytBufUnformat ;
	//
	while ( nPixels != 0 )
	{
		int32_t	xPos = (fxSrcX >> 16) ;
		int32_t	yPos = (fxSrcY >> 16) ;
		//
		if ( (uint32_t) xPos >= widthSrc )
		{
			xPos = ~(fxSrcX >> 31) & (widthSrc - 1) ;
		}
		if ( (uint32_t) yPos >= heightSrc )
		{
			yPos = ~(fxSrcY >> 31) & (heightSrc - 1) ;
		}
		//
		int32_t	xBlock = xPos >> 2 ;
		int32_t	yBlock = yPos >> 2 ;
		int32_t	xSub = xPos & 0x03 ;
		int32_t	ySub = yPos & 0x03 ;
		//
		S3TC_DXT1_Block *	pDXT1Block =
			(S3TC_DXT1_Block*) (pbytSource + (pitchLine * yBlock * 4)) ;
		pDXT1Block += xBlock ;
		//
		switch ( (pDXT1Block->code[ySub] >> (xSub * 2)) & 0x03 )
		{
		case	0:
			*pargbDst = sglRGB565toARGB32( pDXT1Block->rgb16[0] ) ;
			break ;
		case	1:
			*pargbDst = sglRGB565toARGB32( pDXT1Block->rgb16[1] ) ;
			break ;
		case	2:
			if ( pDXT1Block->rgb16[0] > pDXT1Block->rgb16[1] )
			{
				*pargbDst = sglRGB565toARGB32( pDXT1Block->rgb16[0] ).imul(170)
							+ sglRGB565toARGB32( pDXT1Block->rgb16[1] ).imul(85) ;
			}
			else
			{
				*pargbDst = sglRGB565toARGB32( pDXT1Block->rgb16[0] ).imul(128)
							+ sglRGB565toARGB32( pDXT1Block->rgb16[1] ).imul(128) ;
			}
			break ;
		case	3:
			if ( pDXT1Block->rgb16[0] > pDXT1Block->rgb16[1] )
			{
				*pargbDst = sglRGB565toARGB32( pDXT1Block->rgb16[0] ).imul(85)
							+ sglRGB565toARGB32( pDXT1Block->rgb16[1] ).imul(170) ;
			}
			else
			{
				*pargbDst = 0 ;
			}
			break ;
		}
		//
		fxSrcX += m_fxSampling00 ;
		fxSrcY += m_fxSampling10 ;
		++ pargbDst ;
		-- nPixels ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// フィルタ処理関数
//////////////////////////////////////////////////////////////////////////////

// 何もしない
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterNothingProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterTransparencyProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	pargbPixels
		REG LOAD	nPixels
		REG ALLOC	nTransparency(2) : int64_t
		REG ALLOC	argbTemp(4) : int64_t
		REG ALLOC	nPixelsX4 : int64_t
		//
		move		nTransparency(0), [tp].m_nTransparency
		pshuf.w		nTransparency(0), nTransparency(0), 0
		move		nTransparency(1), nTransparency(0)
		move		acc, 3
		move		nPixelsX4, nPixels
		and			nPixelsX4, acc
		//
		prefetch.tlb0	pargbPixels
		.WHILE		(uint32) nPixelsX4 > (uint32) #zero
			load.64		argbTemp(0), [pargbPixels]
			load.64		argbTemp(2), [pargbPixels][8]
			//
			srl			argbTemp(1), argbTemp(0), 32
			srl			argbTemp(3), argbTemp(2), 32
			punpack.lbw	argbTemp(0), #zero
			punpack.lbw	argbTemp(1), #zero
			punpack.lbw	argbTemp(2), #zero
			punpack.lbw	argbTemp(3), #zero
			pmul.lw		argbTemp(0), nTransparency(0)
			pmul.lw		argbTemp(1), nTransparency(1)
			pmul.lw		argbTemp(2), nTransparency(0)
			pmul.lw		argbTemp(3), nTransparency(1)
			psrl.w		argbTemp(0), argbTemp(0), 8
			psrl.w		argbTemp(1), argbTemp(1), 8
			psrl.w		argbTemp(2), argbTemp(2), 8
			psrl.w		argbTemp(3), argbTemp(3), 8
			pcvt.uswb	argbTemp(0), argbTemp(1)
			pcvt.uswb	argbTemp(2), argbTemp(3)
			store.64	[pargbPixels], argbTemp(0)
			store.64	[pargbPixels][8], argbTemp(2)
			//
			dec			nPixelsX4
			add			pargbPixels, sizeof(SGLPalette)*4
		.ENDW
		//
		.WHILE	(uint32) nPixels != (uint32) #zero
			move		argbTemp, [pargbPixels].ui32
			punpack.lbw	argbTemp, #zero
			pmul.lw		argbTemp, nTransparency
			psrl.w		argbTemp, argbTemp(0), 8
			pcvt.uswb	argbTemp, #zero
			move		[pargbPixels].ui32, argbTemp
			//
			dec			nPixels
			add			pargbPixels, sizeof(SGLPalette)
		.ENDW
		unfetch.tlb0	pargbPixels
	}
#else
	uint32_t	nTransparency = m_nTransparency ;
	while ( nPixels != 0 )
	{
//		*pargbPixels *= nTransparency ;
		pargbPixels->argb.Blue	= m_mulSrcTable[pargbPixels->argb.Blue] ;
		pargbPixels->argb.Green	= m_mulSrcTable[pargbPixels->argb.Green] ;
		pargbPixels->argb.Red	= m_mulSrcTable[pargbPixels->argb.Red] ;
		pargbPixels->argb.Alpha	= m_mulSrcTable[pargbPixels->argb.Alpha] ;
		//
		++ pargbPixels ;
		-- nPixels ;
	}
#endif
}

// パレットテーブル展開
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterIndexedPaletteProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
	sglDecodePixelColorIndexed8
		( pargbPixels, &(m_tableSrcPalette[0]), nPixels ) ;
}

// paintApplyColorAdd
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterColorAddProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
	SGLPalette	rgbFilter, rgbTemp ;
	rgbFilter.ui32 = m_argbColorParam1.ui32 ;
	while ( nPixels != 0 )
	{
		rgbTemp.ui32 = rgbFilter.ui32 ;
		rgbTemp *= ((uint32_t)pargbPixels->argb.Alpha + 1) ;
		rgbTemp.ui32 &= 0x00FFFFFF ;
		*pargbPixels += rgbTemp ;
		//
		++ pargbPixels ;
		-- nPixels ;
	}
}

// paintApplyColorMul
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterColorMulProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
	uint32_t	a = (uint32_t) m_argbColorParam1.argb.Alpha + 1 ;
	uint32_t	r = (uint32_t) m_argbColorParam1.argb.Red + 1 ;
	uint32_t	g = (uint32_t) m_argbColorParam1.argb.Green + 1 ;
	uint32_t	b = (uint32_t) m_argbColorParam1.argb.Blue + 1 ;
	while ( nPixels != 0 )
	{
		pargbPixels->argb.Alpha
			= (uint8_t) ((pargbPixels->argb.Alpha * a) >> 8) ;
		pargbPixels->argb.Red
			= (uint8_t) ((pargbPixels->argb.Red * r) >> 8) ;
		pargbPixels->argb.Green
			= (uint8_t) ((pargbPixels->argb.Green * g) >> 8) ;
		pargbPixels->argb.Blue
			= (uint8_t) ((pargbPixels->argb.Blue * b) >> 8) ;
		//
		++ pargbPixels ;
		-- nPixels ;
	}
}

// paintApplyAlphaMul
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterAlphaMulProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		uint32_t	s = pargbPixels->ui32 ;
		uint32_t	a = (s >> 24) ;
		uint32_t	m = a + 1 ;
		s = (((s & 0x00FF00FF) * m) & 0xFF00FF00)
				| (((s & 0x0000FF00) * m) & 0x00FF0000) ;
		pargbPixels->ui32 = (s >> 8) | (a << 24) ;
		//
		++ pargbPixels ;
		-- nPixels ;
	}
}

// paintApplyColorMask
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::FilterColorMaskProc
	( SGLPalette * pargbPixels, size_t nPixels )
{
	uint32_t	rgbFilter = m_argbColorParam1.ui32 & 0x00FFFFFF ;
	while ( nPixels != 0 )
	{
		uint32_t	s = pargbPixels->ui32 ;
		if ( (s & 0x00FFFFFF) == rgbFilter )
		{
			pargbPixels->ui32 = 0 ;
		}
		++ pargbPixels ;
		-- nPixels ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 描画処理関数
//////////////////////////////////////////////////////////////////////////////

// 通常描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintNormalBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	pargbSrc
		REG LOAD	pargbDst
		REG LOAD	nPixels
		REG ALLOC	argbSrc : uint32
		REG ALLOC	argbDst : uint32
		REG ALLOC	aSrc : int32
		REG ALLOC	aMask : uint32
		REG ALLOC	rgbMask : uint32
		//
		prefetch.tlb0	pargbSrc
		prefetch.tlb1	pargbDst
		move		aMask, 0xFF000000
		move		rgbMask, 0x00FFFFFF
		.WHILE	(uint32) nPixels != (uint32) #zero
			load.uint32	argbSrc, [pargbSrc]
			//
			.IF	(uint32) argbSrc != (uint32) #zero
				psrl.d		aSrc, argbSrc, 24
				.IF	(int32) aSrc > (int32) 0xFD
					or				argbSrc, aMask
					store.uint32	[pargbDst], argbSrc
				.ELSE
					load.uint32		argbDst, [pargbDst]
					xor				aSrc, #ff
					padd.d			aSrc, #one
					xor				argbDst, aMask
					pshuf.w			aSrc, aSrc, 0
					punpack.lbw		argbDst, #zero
					pmul.lw			argbDst, aSrc
					and				argbSrc, rgbMask
					psrl.w			argbDst, argbDst, 8
					pcvt.uswb		argbDst, #zero
					padd.ub			argbDst, argbSrc
					xor				argbDst, aMask
					store.uint32	[pargbDst], argbDst
				.ENDIF
			.ENDIF
			//
			add			pargbSrc, sizeof(SGLPalette)
			add			pargbDst, sizeof(SGLPalette)
			dec			nPixels
		.ENDW
		unfetch.tlb0	pargbSrc
		unfetch.tlb1	pargbDst
	}
#else
	while ( nPixels != 0 )
	{
		uint32_t	argbSrc = pargbSrc->ui32 ;
		if ( argbSrc != 0 )
		{
			uint32_t	aSrc = (argbSrc >> 24) ;
			if ( aSrc >= 0xFE )
			{
				pargbDst->ui32 = argbSrc | 0xFF000000 ;
			}
			else
			{
				pargbDst->ui32 =
					sglPackedColorBlend( pargbDst->ui32, argbSrc ) ;
			}
		}
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
#endif
}

// 通常描画（RGB ソース）
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintNormalProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	pargbSrc
		REG LOAD	pargbDst
		REG LOAD	nPixels
		REG ALLOC	nPixelsX8 : int64
		REG ALLOC	numConst7 : int64
		REG ALLOC	argbTemp(2) : int64
		REG ALLOC	pairFF000000(2) : int64
		//
		move		pairFF000000(0), 0xFF000000FF000000
		move		pairFF000000(1), pairFF000000(0)
		move		numConst7, 7
		srl			nPixelsX8, nPixels, 3
		and			nPixels, numConst7
		prefetch.tlb0	pargbSrc
		prefetch.tlb1	pargbDst
		.WHILE		(uint32) nPixelsX8 != (uint32) #zero
			vmove		argbTemp, [pargbSrc]
			or			argbTemp(0), pairFF000000(0)
			or			argbTemp(1), pairFF000000(1)
			vmove		[pargbDst], argbTemp
			//
			vmove		argbTemp, [pargbSrc+16]
			or			argbTemp(0), pairFF000000(0)
			or			argbTemp(1), pairFF000000(1)
			vmove		[pargbDst+16], argbTemp
			//
			dec			nPixelsX8
			add			pargbSrc, sizeof(SGLPalette)*8
			add			pargbDst, sizeof(SGLPalette)*8
		.ENDW
		//
		.WHILE	(uint32) nPixels != (uint32) #zero
			load.uint32		argbTemp, [pargbSrc]
			or				argbTemp, pairFF000000
			store.uint32	[pargbDst], argbTemp
			//
			dec			nPixels
			add			pargbSrc, sizeof(SGLPalette)
			add			pargbDst, sizeof(SGLPalette)
		.ENDW
		unfetch.tlb0	pargbSrc
		unfetch.tlb1	pargbDst
	}
#else
	while ( nPixels != 0 )
	{
		pargbDst->ui32 = pargbSrc->ui32 | 0xFF000000 ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
#endif
}

// 透明度付き描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintTransparencyBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	const uint32_t	nOpacity = 0x100 - m_nTransparency ;
	while ( nPixels != 0 )
	{
		uint32_t	argbSrc = pargbSrc->ui32 ;
		if ( argbSrc != 0 )
		{
			pargbDst->ui32 =
				sglPackedColorBlend
					( pargbDst->ui32,
						sglPackedColorMul( argbSrc, nOpacity ) ) ;
		}
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 透明度付き描画（RGB ソース）
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintTransparencyProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
#if	defined(__COTOPHA__)
	asm
	{
		REG LOAD	pargbDst
		REG LOAD	pargbSrc
		REG LOAD	nPixels
		REG ALLOC	nTransparency(2) : int64
		REG ALLOC	nOpacity(2) : int64
		REG ALLOC	argbTempDst(2) : int64
		REG ALLOC	argbTempSrc(2) : int64
		REG ALLOC	numConstAlphaMask : int64
		//
		move		nOpacity(0), 0x100
		move		nTransparency(0), [tp].m_nTransparency
		sub			nOpacity(0), nTransparency(0)
		pshuf.w		nTransparency(0), nTransparency(0), 0
		pshuf.w		nOpacity(0), nOpacity(0), 0xC0
		move		nTransparency(1), nTransparency(0)
		move		nOpacity(1), nOpacity(0)
		//
		move		numConstAlphaMask, 0xFF000000FF000000
		//
		.WHILE	(int32) nPixels > (int32) #one
			load.64		argbTempDst(0), [pargbDst]
			load.64		argbTempSrc(0), [pargbSrc]
			xor			argbTempDst(0), numConstAlphaMask
			//
			srl			argbTempDst(1), argbTempDst(0), 32
				srl			argbTempSrc(1), argbTempSrc(0), 32
			punpack.lbw	argbTempDst(0), #zero
			punpack.lbw	argbTempDst(1), #zero
				punpack.lbw	argbTempSrc(0), #zero
				punpack.lbw	argbTempSrc(1), #zero
			//
			pmul.lw		argbTempDst(0), nTransparency(0)
			pmul.lw		argbTempDst(1), nTransparency(1)
				pmul.lw		argbTempSrc(0), nOpacity(0)
				pmul.lw		argbTempSrc(1), nOpacity(1)
			//
			psrl.w		argbTempDst(0), argbTempDst(0), 8
			psrl.w		argbTempDst(1), argbTempDst(1), 8
				psrl.w		argbTempSrc(0), argbTempSrc(0), 8
				psrl.w		argbTempSrc(1), argbTempSrc(1), 8
			pcvt.uswb	argbTempDst(0), argbTempDst(1)
				pcvt.uswb	argbTempSrc(0), argbTempSrc(1)
			//
			padd.ub		argbTempDst(0), argbTempSrc(0)
			xor			argbTempDst(0), numConstAlphaMask
			//
			store.64	[pargbDst], argbTempDst(0)
			//
			add			nPixels, nPixels, -2
			add			pargbDst, pargbDst, sizeof(SGLPalette)*2
			add			pargbSrc, pargbSrc, sizeof(SGLPalette)*2
		.ENDW
		//
		.IF	(int32) nPixels > (int32) #zero
			load.uint32	argbTempDst(0), [pargbDst]
			load.uint32	argbTempSrc(0), [pargbSrc]
			xor			argbTempDst(0), numConstAlphaMask
			//
			punpack.lbw	argbTempDst(0), #zero
				punpack.lbw	argbTempSrc(0), #zero
			//
			pmul.lw		argbTempDst(0), nTransparency(0)
				pmul.lw		argbTempSrc(0), nOpacity(0)
			//
			psrl.w		argbTempDst(0), argbTempDst(0), 8
				psrl.w		argbTempSrc(0), argbTempSrc(0), 8
			pcvt.uswb	argbTempDst(0), argbTempDst(1)
				pcvt.uswb	argbTempSrc(0), argbTempSrc(1)
			//
			padd.ub		argbTempDst(0), argbTempSrc(0)
			xor			argbTempDst(0), numConstAlphaMask
			//
			store.uint32	[pargbDst], argbTempDst(0)
		.ENDIF
	}
#else
	while ( nPixels != 0 )
	{
		pargbDst->argb.Blue =
			m_mulDstTable[pargbDst->argb.Blue]
				+ m_mulSrcTable[pargbSrc->argb.Blue] ;
		pargbDst->argb.Green =
			m_mulDstTable[pargbDst->argb.Green]
				+ m_mulSrcTable[pargbSrc->argb.Green] ;
		pargbDst->argb.Red =
			m_mulDstTable[pargbDst->argb.Red]
				+ m_mulSrcTable[pargbSrc->argb.Red] ;
		pargbDst->argb.Alpha =
			m_mulDstTable[pargbDst->argb.Alpha ^ 0xFF] ^ 0xFF ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
#endif
}

// ｚ比較付き描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintBlendWithZProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	const float32_t	zOrder = m_zOrder ;
	while ( nPixels != 0 )
	{
		uint32_t	argbSrc = pargbSrc->ui32 ;
		if ( (argbSrc != 0) & (zOrder <= *pzBuf) )
		{
			uint32_t	aSrc = (argbSrc >> 24) ;
			if ( aSrc >= 0xFE )
			{
				pargbDst->ui32 = argbSrc | 0xFF000000 ;
				*pzBuf = zOrder ;
			}
			else
			{
				pargbDst->ui32 =
					sglPackedColorBlend( pargbDst->ui32, argbSrc ) ;
			}
		}
		++ pargbDst ;
		++ pzBuf ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 上書き描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintNoBlendAlphaProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		pargbDst->ui32 = pargbSrc->ui32 ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 透明度付き上書き描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintTransparencyNoBlendAlphaProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		pargbDst->argb.Blue =
			m_mulDstTable[pargbDst->argb.Blue]
				+ m_mulSrcTable[pargbSrc->argb.Blue] ;
		pargbDst->argb.Green =
			m_mulDstTable[pargbDst->argb.Green]
				+ m_mulSrcTable[pargbSrc->argb.Green] ;
		pargbDst->argb.Red =
			m_mulDstTable[pargbDst->argb.Red]
				+ m_mulSrcTable[pargbSrc->argb.Red] ;
		pargbDst->argb.Alpha =
			m_mulDstTable[pargbDst->argb.Alpha]
				+ m_mulSrcTable[pargbSrc->argb.Alpha] ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 加算描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintAddBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		uint32_t	b = pargbDst->argb.Blue + pargbSrc->argb.Blue ;
		uint32_t	g = pargbDst->argb.Green + pargbSrc->argb.Green ;
		uint32_t	r = pargbDst->argb.Red + pargbSrc->argb.Red ;
		uint32_t	a = pargbDst->argb.Alpha + pargbSrc->argb.Alpha ;
		b |= ~(b + 0xFF00) >> 8 ;
		g |= ~(g + 0xFF00) >> 8 ;
		r |= ~(r + 0xFF00) >> 8 ;
		a |= ~(a + 0xFF00) >> 8 ;
		pargbDst->argb.Blue = (uint8_t) b ;
		pargbDst->argb.Green = (uint8_t) g ;
		pargbDst->argb.Red = (uint8_t) r ;
		pargbDst->argb.Alpha = (uint8_t) a ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 減算描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintSubBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		uint32_t	b = pargbDst->argb.Blue - pargbSrc->argb.Blue ;
		uint32_t	g = pargbDst->argb.Green - pargbSrc->argb.Green ;
		uint32_t	r = pargbDst->argb.Red - pargbSrc->argb.Red ;
		b &= ~b >> 8 ;
		g &= ~g >> 8 ;
		r &= ~r >> 8 ;
		pargbDst->argb.Blue = (uint8_t) b ;
		pargbDst->argb.Green = (uint8_t) g ;
		pargbDst->argb.Red = (uint8_t) r ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 積算描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintMulBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		pargbDst->argb.Blue =
			(uint8_t) ((pargbDst->argb.Blue
							* (pargbSrc->argb.Blue + 1)) >> 8) ;
		pargbDst->argb.Green =
			(uint8_t) ((pargbDst->argb.Green
							* (pargbSrc->argb.Green + 1)) >> 8) ;
		pargbDst->argb.Red =
			(uint8_t) ((pargbDst->argb.Red
							* (pargbSrc->argb.Red + 1)) >> 8) ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 除算描画（覆い焼き）
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintDivBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	const uint32_t *	pDivTable = &m_tableDivBlend[0] ;
	while ( nPixels != 0 )
	{
		uint32_t	b =
			(pargbDst->argb.Blue
					* pDivTable[pargbSrc->argb.Blue]) >> 6 ;
		uint32_t	g =
			(pargbDst->argb.Green
					* pDivTable[pargbSrc->argb.Green]) >> 6 ;
		uint32_t	r =
			(pargbDst->argb.Red
					* pDivTable[pargbSrc->argb.Red]) >> 6 ;
		if ( b >= 0x100 )
		{
			b = 0xFF ;
		}
		if ( g >= 0x100 )
		{
			g = 0xFF ;
		}
		if ( r >= 0x100 )
		{
			r = 0xFF ;
		}
		pargbDst->argb.Blue = (uint8_t) b ;
		pargbDst->argb.Green = (uint8_t) g ;
		pargbDst->argb.Red = (uint8_t) r ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 最大値選択
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintMaxBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	uint32_t	src, dst, msk ;
	while ( nPixels != 0 )
	{
		src = pargbSrc->argb.Blue ;
		dst = pargbDst->argb.Blue ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Blue = (uint8_t) ((dst & ~msk) | (src & msk)) ;
		//
		src = pargbSrc->argb.Green ;
		dst = pargbDst->argb.Green ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Green = (uint8_t) ((dst & ~msk) | (src & msk)) ;
		//
		src = pargbSrc->argb.Red ;
		dst = pargbDst->argb.Red ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Red = (uint8_t) ((dst & ~msk) | (src & msk)) ;
		//
		src = pargbSrc->argb.Alpha ;
		dst = pargbDst->argb.Alpha ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Alpha = (uint8_t) ((dst & ~msk) | (src & msk)) ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 最小値選択
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintMinBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	uint32_t	src, dst, msk ;
	while ( nPixels != 0 )
	{
		src = pargbSrc->argb.Blue ;
		dst = pargbDst->argb.Blue ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Blue = (uint8_t) ((dst & msk) | (src & ~msk)) ;
		//
		src = pargbSrc->argb.Green ;
		dst = pargbDst->argb.Green ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Green = (uint8_t) ((dst & msk) | (src & ~msk)) ;
		//
		src = pargbSrc->argb.Red ;
		dst = pargbDst->argb.Red ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Red = (uint8_t) ((dst & msk) | (src & ~msk)) ;
		//
		src = pargbSrc->argb.Alpha ;
		dst = pargbDst->argb.Alpha ;
		msk = (dst - src) >> 8 ;
		pargbDst->argb.Alpha = (uint8_t) ((dst & msk) | (src & ~msk)) ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 反転乗算描画（スクリーン）
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintNegMulBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	while ( nPixels != 0 )
	{
		pargbDst->argb.Blue =
			(uint8_t) (((pargbDst->argb.Blue ^ 0xFF)
						* ((pargbSrc->argb.Blue ^ 0xFF) + 1)) >> 8) ^ 0xFF ;
		pargbDst->argb.Green =
			(uint8_t) (((pargbDst->argb.Green ^ 0xFF)
						* ((pargbSrc->argb.Green ^ 0xFF) + 1)) >> 8) ^ 0xFF ;
		pargbDst->argb.Red =
			(uint8_t) (((pargbDst->argb.Red ^ 0xFF)
						* ((pargbSrc->argb.Red ^ 0xFF) + 1)) >> 8) ^ 0xFF ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// αチャネルで ARGB 乗算して、入力 RGB を加算
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintMulAlphaBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	uint32_t	rgbSrc, alphaSrc ;
	while ( nPixels != 0 )
	{
		rgbSrc = pargbSrc->ui32 ;
		alphaSrc = (rgbSrc >> 24) + 1 ;
		//
		pargbDst->ui32 =
			sglPackedColorAdd
				( sglPackedColorMul
					( pargbDst->ui32, alphaSrc ), (rgbSrc & 0x00FFFFFF) ) ;
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// 出力先αチャネルで入力 ARGB 乗算して描画、出力先αチャネルは不変
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintDstAlphaMaskBlendProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	uint32_t	rgbaSrc, alphaSrc, rgbDst, alphaDst ;
	while ( nPixels != 0 )
	{
		rgbDst = pargbDst->ui32 ;
		alphaDst = (rgbDst >> 24) ;
		//
		if ( alphaDst )
		{
			rgbaSrc = sglPackedColorMul( pargbSrc->ui32, alphaDst + 1 ) ;
			if ( rgbaSrc )
			{
				alphaSrc = (~rgbaSrc >> 24) + 1 ;
				//
				pargbDst->ui32 =
					sglPackedColorAdd
						( sglPackedColorMul
							( (rgbDst & 0x00FFFFFF), alphaSrc ),
											(rgbaSrc & 0x00FFFFFF) )
					| (rgbDst & 0xFF000000) ;
			}
		}
		//
		++ pargbDst ;
		++ pargbSrc ;
		-- nPixels ;
	}
}

// ｚ比較出力
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintWriteWithZProc
	( SGLPalette * pargbDst, float32_t * pzBuf,
			const SGLPalette * pargbSrc, size_t nPixels )
{
	const float32_t	zOrder = m_zOrder ;
	while ( nPixels != 0 )
	{
		if ( zOrder <= *pzBuf )
		{
			pargbDst->ui32 = pargbSrc->ui32 ;
			*pzBuf = zOrder ;
		}
		++ pargbDst ;
		++ pzBuf ;
		++ pargbSrc ;
		-- nPixels ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 変形なし描画処理フレームワーク関数
//////////////////////////////////////////////////////////////////////////////

// ARGB -> ARGB 描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PerformPaintRectSimple( void )
{
	if ( (m_infSource.width * m_infSource.height >= 0x4000) && (m_countThreads >= 2) )
	{
		void *	pInstance[SGLPaintBuffer::MAX_THREADS] ;
		for ( size_t i = 0; i < m_countThreads; i ++ )
		{
			pInstance[i] = &m_bufInternal[i] ;
		}
		PaintRectSimpleProc	prsp( this ) ;
		prsp.Start( &pInstance[0], m_countThreads ) ;
	}
	else
	{
		uint8_t *		pbytRectDst = m_pbytRectDst ;
		uint8_t *		pbytRectZBuf = m_pbytRectZBuf ;
		uint8_t *		pbytRectSrc = m_pbytSource ;
		const uint32_t	widthRect = m_infSource.width ;
		const uint32_t	heightRect = m_infSource.height ;
		const int32_t	pitchDst = m_infTarget.pitchLine ;
		const int32_t	pitchZBuf = m_infZBuffer.pitchLine ;
		const int32_t	pitchSrc = m_infSource.pitchLine ;
		//
		for ( uint32_t y = 0; y < heightRect; ++ y )
		{
			(this->*m_pfnPaintLine)
				( (SGLPalette*) pbytRectDst,
					(float32_t*) pbytRectZBuf,
					(const SGLPalette*) pbytRectSrc, widthRect ) ;
			//
			pbytRectDst += pitchDst ;
			pbytRectZBuf += pitchZBuf ;
			pbytRectSrc += pitchSrc ;
		}
	}
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintBuffer::PaintRectSimpleProc::PaintRectSimpleProc( SGLPaintBuffer * paint )
{
	m_paint = paint ;
	//
	m_pfnPaintLine = paint->m_pfnPaintLine ;
	//
	m_pbytRectDst = paint->m_pbytRectDst ;
	m_pbytRectZBuf = paint->m_pbytRectZBuf ;
	m_pbytRectSrc = paint->m_pbytSource ;
	m_widthRect = paint->m_infSource.width ;
	m_heightRect = paint->m_infSource.height ;
	m_pitchDst = paint->m_infTarget.pitchLine ;
	m_pitchZBuf = paint->m_infZBuffer.pitchLine ;
	m_pitchSrc = paint->m_infSource.pitchLine ;
	//
	m_yNext = 0 ;
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool SGLPaintBuffer::PaintRectSimpleProc::Continue( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	if ( m_yNext < m_heightRect )
	{
		pib->m_pbytDst = m_pbytRectDst ;
		pib->m_pbytZBuf = m_pbytRectZBuf ;
		pib->m_pbytSrc = m_pbytRectSrc ;
		//
		++ m_yNext ;
		m_pbytRectDst += m_pitchDst ;
		m_pbytRectZBuf += m_pitchZBuf ;
		m_pbytRectSrc += m_pitchSrc ;
		return	true ;
	}
	return	false ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintRectSimpleProc::RunParallel( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	SGLPaintBuffer *	paint = m_paint ;
	SGLPalette *	pSrcLine = (SGLPalette*) (pib->m_pbytSrc) ;
	SGLPalette *	pDstLine = (SGLPalette*) (pib->m_pbytDst) ;
	float32_t *		pZBufLine = (float32_t*) (pib->m_pbytZBuf) ;
	const uint32_t	widthRect = m_widthRect ;
	//
	(paint->*m_pfnPaintLine)
		( pDstLine, pZBufLine, pSrcLine, widthRect ) ;
}



// 変形なし汎用描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PerformPaintRectGeneric( void )
{
	if ( (m_infSource.width * m_infSource.height >= 0x4000) && (m_countThreads >= 2) )
	{
		void *	pInstance[SGLPaintBuffer::MAX_THREADS] ;
		for ( size_t i = 0; i < m_countThreads; i ++ )
		{
			pInstance[i] = &m_bufInternal[i] ;
		}
		PaintRectGenericProc	prgp( this ) ;
		prgp.Start( &pInstance[0], m_countThreads ) ;
	}
	else
	{
		uint8_t *		pbytRectDst = m_pbytRectDst ;
		uint8_t *		pbytRectZBuf = m_pbytRectZBuf ;
		uint8_t *		pbytRectSrc = m_pbytSource ;
		const uint32_t	widthRect = m_infSource.width ;
		const uint32_t	heightRect = m_infSource.height ;
		const int32_t	pitchDst = m_infTarget.pitchLine ;
		const int32_t	pitchZBuf = m_infZBuffer.pitchLine ;
		const int32_t	pitchSrc = m_infSource.pitchLine ;
		SGLPalette *	pbufSrcLine = m_bufInternal[0].m_bufSrcSampled.GetArray() ;
		SGLPalette *	pbufDstLine = m_bufInternal[0].m_bufDstSampled.GetArray() ;
		float32_t *		pbufZBufLine = m_bufInternal[0].m_bufZSampled.GetArray() ;
		//
		for ( uint32_t y = 0; y < heightRect; ++ y )
		{
			SGLPalette *	pSrcLine = (SGLPalette*) pbytRectSrc ;
			SGLPalette *	pDstLine = (SGLPalette*) pbytRectDst ;
			float32_t *		pZBufLine = (float32_t*) pbytRectZBuf ;
			//
			if ( m_pfnConvertSrc != NULL )
			{
				pSrcLine = pbufSrcLine ;
				m_pfnConvertSrc( pSrcLine, pbytRectSrc, widthRect ) ;
			}
			if ( m_pfnNormalizeSrc != NULL )
			{
				m_pfnNormalizeSrc( pSrcLine, pSrcLine, widthRect ) ;
			}
			if ( m_pfnSampleDst != NULL )
			{
				pDstLine = pbufDstLine ;
				m_pfnSampleDst( pDstLine, pbytRectDst, widthRect ) ;
			}
			if ( m_pfnSampleZBuf != NULL )
			{
				pZBufLine = pbufZBufLine ;
				m_pfnSampleZBuf
					( (SGLPalette*) pZBufLine, pbytRectZBuf, widthRect ) ;
			}
			if ( m_pfnFilterSrc != NULL )
			{
				ESLAssert( m_pfnConvertSrc != NULL ) ;
				(this->*m_pfnFilterSrc)( pSrcLine, widthRect ) ;
				//
				if ( m_pfnFilterPost != NULL )
				{
					(this->*m_pfnFilterPost)( pSrcLine, widthRect ) ;
				}
			}
			//
			(this->*m_pfnPaintLine)
				( pDstLine, pZBufLine, pSrcLine, widthRect ) ;
			//
			if ( m_pfnPaintPost != NULL )
			{
				(this->*m_pfnPaintPost)
					( pDstLine, pZBufLine, pSrcLine, widthRect ) ;
			}
			if ( m_pfnStoreDst != NULL )
			{
				m_pfnStoreDst( pbytRectDst, pDstLine, widthRect ) ;
			}
			//
			pbytRectDst += pitchDst ;
			pbytRectZBuf += pitchZBuf ;
			pbytRectSrc += pitchSrc ;
		}
		m_bufInternal[0].m_bufSrcSampled.FinishArray() ;
		m_bufInternal[0].m_bufDstSampled.FinishArray() ;
		m_bufInternal[0].m_bufZSampled.FinishArray() ;
	}
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintBuffer::PaintRectGenericProc::PaintRectGenericProc( SGLPaintBuffer * paint )
{
	m_paint = paint ;
	//
	m_pfnNormalizeSrc = paint->m_pfnNormalizeSrc ;
	m_pfnConvertSrc = paint->m_pfnConvertSrc ;
	m_pfnSampleDst = paint->m_pfnSampleDst ;
	m_pfnSampleZBuf = paint->m_pfnSampleZBuf ;
	//
	m_pfnFilterSrc = paint->m_pfnFilterSrc ;
	m_pfnFilterPost = paint->m_pfnFilterPost ;
	//
	m_pfnPaintLine = paint->m_pfnPaintLine ;
	m_pfnPaintPost = paint->m_pfnPaintPost ;
	//
	m_pfnStoreDst = paint->m_pfnStoreDst ;
	//
	m_pbytRectDst = paint->m_pbytRectDst ;
	m_pbytRectZBuf = paint->m_pbytRectZBuf ;
	m_pbytRectSrc = paint->m_pbytSource ;
	m_widthRect = paint->m_infSource.width ;
	m_heightRect = paint->m_infSource.height ;
	m_pitchDst = paint->m_infTarget.pitchLine ;
	m_pitchZBuf = paint->m_infZBuffer.pitchLine ;
	m_pitchSrc = paint->m_infSource.pitchLine ;
	//
	m_yNext = 0 ;
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool SGLPaintBuffer::PaintRectGenericProc::Continue( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	if ( m_yNext < m_heightRect )
	{
		pib->m_pbytDst = m_pbytRectDst ;
		pib->m_pbytZBuf = m_pbytRectZBuf ;
		pib->m_pbytSrc = m_pbytRectSrc ;
		//
		++ m_yNext ;
		m_pbytRectDst += m_pitchDst ;
		m_pbytRectZBuf += m_pitchZBuf ;
		m_pbytRectSrc += m_pitchSrc ;
		return	true ;
	}
	return	false ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintRectGenericProc::RunParallel( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	SGLPaintBuffer *	paint = m_paint ;
	SGLPalette *	pSrcLine = (SGLPalette*) (pib->m_pbytSrc) ;
	SGLPalette *	pDstLine = (SGLPalette*) (pib->m_pbytDst) ;
	float32_t *		pZBufLine = (float32_t*) (pib->m_pbytZBuf) ;
	const uint32_t	widthRect = m_widthRect ;
	//
	if ( m_pfnConvertSrc != NULL )
	{
		pSrcLine = pib->m_bufSrcSampled.GetArray() ;
		m_pfnConvertSrc( pSrcLine, pib->m_pbytSrc, widthRect ) ;
		pib->m_bufSrcSampled.FinishArray() ;
	}
	if ( m_pfnNormalizeSrc != NULL )
	{
		m_pfnNormalizeSrc( pSrcLine, pSrcLine, widthRect ) ;
	}
	if ( m_pfnSampleDst != NULL )
	{
		pDstLine = pib->m_bufDstSampled.GetArray() ;
		m_pfnSampleDst( pDstLine, pib->m_pbytDst, widthRect ) ;
		pib->m_bufDstSampled.FinishArray() ;
	}
	if ( m_pfnSampleZBuf != NULL )
	{
		pZBufLine = pib->m_bufZSampled.GetArray() ;
		m_pfnSampleZBuf
			( (SGLPalette*) pZBufLine, pib->m_pbytZBuf, widthRect ) ;
		pib->m_bufZSampled.FinishArray() ;
	}
	if ( m_pfnFilterSrc != NULL )
	{
		ESLAssert( m_pfnConvertSrc != NULL ) ;
		(paint->*m_pfnFilterSrc)( pSrcLine, widthRect ) ;
		//
		if ( m_pfnFilterPost != NULL )
		{
			(paint->*m_pfnFilterPost)( pSrcLine, widthRect ) ;
		}
	}
	//
	(paint->*m_pfnPaintLine)
		( pDstLine, pZBufLine, pSrcLine, widthRect ) ;
	//
	if ( m_pfnPaintPost != NULL )
	{
		(paint->*m_pfnPaintPost)
			( pDstLine, pZBufLine, pSrcLine, widthRect ) ;
	}
	if ( m_pfnStoreDst != NULL )
	{
		m_pfnStoreDst( pib->m_pbytDst, pDstLine, widthRect ) ;
	}
}


// 変形あり汎用描画
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PerformPaintTransformedGeneric( void )
{
	if ( (m_pbufRegion->areaPixels >= 0x4000) && (m_countThreads >= 2) )
	{
		void *	pInstance[SGLPaintBuffer::MAX_THREADS] ;
		for ( size_t i = 0; i < m_countThreads; i ++ )
		{
			pInstance[i] = &m_bufInternal[i] ;
		}
		PaintTransformedGenericProc	ptgp( this ) ;
		ptgp.Start( &pInstance[0], m_countThreads ) ;
	}
	else
	{
		uint8_t *		pbytDst = m_pbytTarget ;
		uint8_t *		pbytZBuf = m_pbytZBuffer ;
		const int32_t	pitchDst = m_infTarget.pitchLine ;
		const int32_t	pitchZBuf = m_infZBuffer.pitchLine ;
		//
		const int32_t	yRegionTop = m_pbufRegion->yTop ;
		const int32_t	yRegionBottom = m_pbufRegion->yBottom ;
		SGLRegionLine *	prgLine = &(m_pbufRegion->rgLine[0]) ;
		//
		SGLPalette *	pbufSrcLine = m_bufInternal[0].m_bufSrcSampled.GetArray() ;
		SGLPalette *	pbufDstLine = m_bufInternal[0].m_bufDstSampled.GetArray() ;
		float32_t *		pbufZBufLine = m_bufInternal[0].m_bufZSampled.GetArray() ;
		uint8_t *		pbytBufUnformat = m_bufInternal[0].m_bufSrcUnformat.GetArray() ;
		//
		pbytDst += yRegionTop * pitchDst ;
		pbytZBuf += yRegionTop * pitchZBuf ;
		//
		for ( int32_t y = yRegionTop; y <= yRegionBottom; ++ y )
		{
			uint32_t	xLeft = (prgLine->fxLeft + 0xFFFF) >> 16 ;
			uint32_t	xRight = (prgLine->fxRight - 1) >> 16 ;
			if ( xLeft <= xRight )
			{
				const uint32_t	widthLine = xRight - xLeft + 1 ;
				SGLPalette *	pSrcLine = pbufSrcLine ;
				SGLPalette *	pDstLine = ((SGLPalette*) pbytDst) + xLeft ;
				float32_t *		pZBufLine = ((float32_t*) pbytZBuf) + xLeft ;
				//
				(this->*m_pfnSampleSrc)
					( pSrcLine, xLeft, y, widthLine, pbytBufUnformat ) ;
				//
				if ( m_pfnNormalizeSrc != NULL )
				{
					m_pfnNormalizeSrc( pSrcLine, pSrcLine, widthLine ) ;
				}
				if ( m_pfnSampleDst != NULL )
				{
					pDstLine = pbufDstLine ;
					m_pfnSampleDst
						( pDstLine, pbytDst + (xLeft << 2), widthLine ) ;
				}
				if ( m_pfnSampleZBuf != NULL )
				{
					pZBufLine = pbufZBufLine ;
					m_pfnSampleZBuf
						( (SGLPalette*) pZBufLine,
							pbytZBuf + (xLeft << 2), widthLine ) ;
				}
				if ( m_pfnFilterSrc != NULL )
				{
					(this->*m_pfnFilterSrc)( pSrcLine, widthLine ) ;
					//
					if ( m_pfnFilterPost != NULL )
					{
						(this->*m_pfnFilterPost)( pSrcLine, widthLine ) ;
					}
				}
				//
				(this->*m_pfnPaintLine)
					( pDstLine, pZBufLine, pSrcLine, widthLine ) ;
				//
				if ( m_pfnPaintPost != NULL )
				{
					(this->*m_pfnPaintPost)
						( pDstLine, pZBufLine, pSrcLine, widthLine ) ;
				}
				if ( m_pfnStoreDst != NULL )
				{
					m_pfnStoreDst
						( pbytDst + (xLeft << 2), pDstLine, widthLine ) ;
				}
			}
			++ prgLine ;
			pbytDst += pitchDst ;
			pbytZBuf += pitchZBuf ;
		}
		m_bufInternal[0].m_bufSrcSampled.FinishArray() ;
		m_bufInternal[0].m_bufDstSampled.FinishArray() ;
		m_bufInternal[0].m_bufZSampled.FinishArray() ;
		m_bufInternal[0].m_bufSrcUnformat.FinishArray() ;
	}
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLPaintBuffer::PaintTransformedGenericProc::PaintTransformedGenericProc
	( SGLPaintBuffer * paint )
{
	m_paint = paint ;
	//
	m_pfnSampleSrc = paint->m_pfnSampleSrc ;
	m_pfnNormalizeSrc = paint->m_pfnNormalizeSrc ;
	m_pfnSampleDst = paint->m_pfnSampleDst ;
	m_pfnSampleZBuf = paint->m_pfnSampleZBuf ;
	m_pfnFilterSrc = paint->m_pfnFilterSrc ;
	m_pfnFilterPost = paint->m_pfnFilterPost ;
	m_pfnStoreDst = paint->m_pfnStoreDst ;
	m_pfnPaintLine = paint->m_pfnPaintLine ;
	m_pfnPaintPost = paint->m_pfnPaintPost ;
	//
	m_pbytDst = paint->m_pbytTarget ;
	m_pbytZBuf = paint->m_pbytZBuffer ;
	m_pitchDst = paint->m_infTarget.pitchLine ;
	m_pitchZBuf = paint->m_infZBuffer.pitchLine ;
	//
	m_yRegionTop = paint->m_pbufRegion->yTop ;
	m_yRegionBottom = paint->m_pbufRegion->yBottom ;
	m_prgLine = &(paint->m_pbufRegion->rgLine[0]) ;
	//
	m_pbytDst += m_yRegionTop * m_pitchDst ;
	m_pbytZBuf += m_yRegionTop * m_pitchZBuf ;
	//
	m_yNext = m_yRegionTop ;
}

// ループ処理／終了判定関数
//////////////////////////////////////////////////////////////////////////////
bool SGLPaintBuffer::PaintTransformedGenericProc::Continue( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	while ( m_yNext <= m_yRegionBottom )
	{
		uint32_t	xLeft = (m_prgLine->fxLeft + 0xFFFF) >> 16 ;
		uint32_t	xRight = (m_prgLine->fxRight - 1) >> 16 ;
		if ( xLeft <= xRight )
		{
			pib->m_xLeft = xLeft ;
			pib->m_xRight = xRight ;
			pib->m_yLine = m_yNext ;
			pib->m_pbytDst = m_pbytDst ;
			pib->m_pbytZBuf = m_pbytZBuf ;
			//
			++ m_yNext ;
			++ m_prgLine ;
			m_pbytDst += m_pitchDst ;
			m_pbytZBuf += m_pitchZBuf ;
			return	true ;
		}
		++ m_yNext ;
		++ m_prgLine ;
		m_pbytDst += m_pitchDst ;
		m_pbytZBuf += m_pitchZBuf ;
	}
	return	false ;
}

// 並列処理関数
//////////////////////////////////////////////////////////////////////////////
void SGLPaintBuffer::PaintTransformedGenericProc::RunParallel( void * pInstance )
{
	SGLPaintBuffer::InternalBuffer *
		pib = (SGLPaintBuffer::InternalBuffer*) pInstance ;
	//
	SGLPaintBuffer *	paint = m_paint ;
	const uint32_t	xLeft = pib->m_xLeft ;
	const uint32_t	widthLine = pib->m_xRight - xLeft + 1 ;
	SGLPalette *	pSrcLine = pib->m_pbufSrcLine ;
	SGLPalette *	pDstLine = ((SGLPalette*)(pib->m_pbytDst)) + xLeft ;
	float32_t *		pZBufLine = ((float32_t*)(pib->m_pbytZBuf)) + xLeft ;
	//
	(paint->*m_pfnSampleSrc)
		( pSrcLine, xLeft, pib->m_yLine,
				widthLine, pib->m_pbytBufUnformat ) ;
	//
	if ( m_pfnNormalizeSrc != NULL )
	{
		m_pfnNormalizeSrc( pSrcLine, pSrcLine, widthLine ) ;
	}
	if ( m_pfnSampleDst != NULL )
	{
		pDstLine = pib->m_pbufDstLine ;
		m_pfnSampleDst
			( pDstLine, m_pbytDst + (xLeft << 2), widthLine ) ;
	}
	if ( m_pfnSampleZBuf != NULL )
	{
		pZBufLine = pib->m_pbufZBufLine ;
		m_pfnSampleZBuf
			( (SGLPalette*) pZBufLine,
				m_pbytZBuf + (xLeft << 2), widthLine ) ;
	}
	if ( m_pfnFilterSrc != NULL )
	{
		(paint->*m_pfnFilterSrc)( pSrcLine, widthLine ) ;
		//
		if ( m_pfnFilterPost != NULL )
		{
			(paint->*m_pfnFilterPost)( pSrcLine, widthLine ) ;
		}
	}
	//
	(paint->*m_pfnPaintLine)
		( pDstLine, pZBufLine, pSrcLine, widthLine ) ;
	//
	if ( m_pfnPaintPost != NULL )
	{
		(paint->*m_pfnPaintPost)
			( pDstLine, pZBufLine, pSrcLine, widthLine ) ;
	}
	if ( m_pfnStoreDst != NULL )
	{
		m_pfnStoreDst
			( m_pbytDst + (xLeft << 2), pDstLine, widthLine ) ;
	}
}

