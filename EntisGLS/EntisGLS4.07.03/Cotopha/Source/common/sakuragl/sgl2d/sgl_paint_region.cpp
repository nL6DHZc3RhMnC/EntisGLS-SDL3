
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 頂点走査コンテキスト
//////////////////////////////////////////////////////////////////////////////

class	ScanVertexContext
{
protected:
	size_t				m_nCount ;
	const S2DVector *	m_pVertices ;
	const S3DColor *	m_pColors ;
	const S3DVector4 *	m_pNormals ;

	ssize_t				m_index ;	// 走査指標
	size_t				m_last ;	// 以前の指標
	size_t				m_end ;
	double				m_yDelta ;	// m_index - m_last の間の現在の比率

public:
	// 構築関数
	ScanVertexContext
		( size_t nCount, const S2DVector * pVertices,
			const S3DColor * pColors, const S3DVector4 * pNormals )
	{
		m_nCount = nCount ;
		m_pVertices = pVertices ;
		m_pColors = pColors ;
		m_pNormals = pNormals ;
		m_index = 0 ;
		m_last = 0 ;
		m_yDelta = 0 ;
	}
	// 指標設定
	inline void SetIndex( int index )
	{
		m_index = index ;
		m_last = index ;
	}
	// 指標移動
	inline size_t IncreaseIndex( void )
	{
		m_last = m_index ;
		if ( (size_t) ++ m_index >= m_nCount )
		{
			m_index = 0 ;
		}
		return	m_index ;
	}
	inline size_t DecreaseIndex( void )
	{
		m_last = m_index ;
		if ( -- m_index < 0 )
		{
			m_index += (ssize_t) m_nCount ;
		}
		return	m_index ;
	}
	// 最もｙ座標が小さい頂点へ指標を移動する
	size_t SearchTopVertex( void )
	{
		float32_t	yMin = m_pVertices[0].y ;
		size_t		iMin = 0 ;
		//
		const size_t	nCount = m_nCount ;
		for ( size_t i = 1; i < nCount; i ++ )
		{
			if ( yMin > m_pVertices[i].y )
			{
				yMin = m_pVertices[i].y ;
				iMin = i ;
			}
		}
		m_index = (ssize_t) iMin ;
		m_last = iMin ;
		return	iMin ;
	}
	// 現在の指標からｙ座標が極大になる最初の頂点を終点に設定する
	size_t FindBottomVertex( bool fIncrease )
	{
		float32_t	yMax = m_pVertices[m_index].y ;
		size_t		iMax = m_index ;
		//
		const size_t	iStart = m_index ;
		const size_t	nCount = m_nCount ;
		for ( size_t i = 1; i < nCount; i ++ )
		{
			size_t	j ;
			if ( fIncrease )
			{
				j = IncreaseIndex() ;
			}
			else
			{
				j = DecreaseIndex() ;
			}
			if ( yMax < m_pVertices[j].y )
			{
				yMax = m_pVertices[j].y ;
				iMax = j ;
			}
			else if ( yMax > m_pVertices[j].y )
			{
				break ;
			}
		}
		m_index = (ssize_t) iStart ;
		m_last = iStart ;
		m_end = iMax ;
		return	iMax ;
	}
	// 現在の頂点ｙ座標を取得する
	inline float32_t GetCurrentY( void ) const
	{
		return	m_pVertices[m_index].y ;
	}
	// 終点のｙ座標を取得する
	inline float32_t GetEndY( void ) const
	{
		return	m_pVertices[m_end].y ;
	}
	// ｙ座標に対応する頂点へ移動し、中間比率を計算
	bool ScanYIncreasingly( int yLine )
	{
		while ( m_pVertices[m_index].y <= yLine )
		{
			if ( (size_t) m_index == m_end )
			{
				return	true ;		// 終了
			}
			IncreaseIndex() ;
		}
		double	yTotal = m_pVertices[m_index].y - m_pVertices[m_last].y ;
		double	yDelta = yLine - m_pVertices[m_last].y ;
		m_yDelta = 0 ;
		if ( yTotal > 1.0e-6 )
		{
			m_yDelta = yDelta / yTotal ;
		}
		return	false ;
	}
	bool ScanYDecreasingly( int yLine )
	{
		while ( m_pVertices[m_index].y <= yLine )
		{
			if ( (size_t) m_index == m_end )
			{
				return	true ;		// 終了
			}
			DecreaseIndex() ;
		}
		double	yTotal = m_pVertices[m_index].y - m_pVertices[m_last].y ;
		double	yDelta = yLine - m_pVertices[m_last].y ;
		m_yDelta = 0 ;
		if ( yTotal > 1.0e-6 )
		{
			m_yDelta = yDelta / yTotal ;
		}
		return	false ;
	}
	// ｘ座標を計算
	inline float32_t CurrentX( void ) const
	{
		REAL32	xLast = m_pVertices[m_last].x ;
		REAL32	xNext = m_pVertices[m_index].x ;
		REAL32	xDelta = xNext - xLast ;
		return	(float32_t) (xDelta * m_yDelta + xLast) ;
	}
	// 頂点色を計算
	inline void CurrentColor( S3DColor& clr ) const
	{
		ESLAssert( m_pColors != NULL ) ;
		uint32_t	t =
			(uint32_t) eslRoundR32ToInt( (float32_t) m_yDelta * 256.0f ) ;
		uint32_t	nt = 0x100 - t ;
		clr = m_pColors[m_last].imul( nt ) + m_pColors[m_index].imul(t) ;
	}

} ;


//////////////////////////////////////////////////////////////////////////////
// リージョン生成
//////////////////////////////////////////////////////////////////////////////

// 多角形リージョン生成
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglCreatePolygonRegion
	( SGLRegion * pRegion,
		const SGLRect & rctClipArg,
		const S2DVector * pVertices, size_t nCount,
		const S3DColor * pColors, const S3DVector4 * pNormals )
{
	//
	// 初期化
	//
	ScanVertexContext	svc1( nCount, pVertices, pColors, pNormals ) ;
	ScanVertexContext	svc2( nCount, pVertices, pColors, pNormals ) ;
	SGLRect				rctClip = rctClipArg ;
	//
	svc1.SearchTopVertex() ;
	svc2.SearchTopVertex() ;
	svc1.FindBottomVertex( true ) ;
	svc2.FindBottomVertex( false ) ;
	//
	if ( svc1.GetCurrentY() > rctClip.bottom )
	{
		return	false ;
	}
	if ( (svc1.GetEndY() <= rctClip.top)
		& (svc2.GetEndY() <= rctClip.top) )
	{
		return	false ;
	}
	//
	// 開始ｙ座標取得
	//
	float32_t	rTopY = svc1.GetCurrentY() ;
	int32_t		yTop = ((eslRoundR32ToInt( rTopY * 0x10000 ) + 0xFFFF) >> 16) ;
	if ( yTop < 0 )
	{
		yTop = 0 ;
	}
	if ( yTop < rctClip.top )
	{
		yTop = rctClip.top ;
	}
	//
	// 終了点までループ
	//
	SGLRegionLine *	prgLine = NULL ;
	uint32_t	areaPixels = 0 ;
	for ( int32_t yLine = yTop; yLine <= rctClip.bottom; yLine ++ )
	{
		bool	fExit ;
		fExit = svc1.ScanYIncreasingly( yLine ) ;
		fExit |= svc2.ScanYDecreasingly( yLine ) ;
		if ( fExit )
		{
			pRegion->areaPixels = areaPixels ;
			return	(prgLine != NULL) ;
		}
		S3DColor	clrLeft, clrRight ;
		int32_t		fxPos1 = (int32_t) esl_fclamp( (svc1.CurrentX() * 0x10000), -0x7FFF0000, 0x7FFF0000 ) ;
		int32_t		fxPos2 = (int32_t) esl_fclamp( (svc2.CurrentX() * 0x10000), -0x7FFF0000, 0x7FFF0000 ) ;
		if ( pColors != NULL )
		{
			svc1.CurrentColor( clrLeft ) ;
			svc2.CurrentColor( clrRight ) ;
		}
		if ( fxPos1 > fxPos2 )
		{
			int32_t	fxTemp = fxPos1 ;
			fxPos1 = fxPos2 ;
			fxPos2 = fxTemp ;
			//
			S3DColor	clrTemp = clrLeft ;
			clrLeft = clrRight ;
			clrRight = clrTemp ;
		}
		int32_t	xLeft = ((fxPos1 + 0xFFFF) >> 16) ;
		int32_t	xRight = ((fxPos2 - 1) >> 16) ;
		if ( (xRight < rctClip.left) | (rctClip.right < xLeft) )
		{
			// ｘ座標は画面外
			if ( prgLine != NULL )
			{
				break ;
			}
			continue ;
		}
		if ( prgLine == NULL )
		{
			prgLine = &(pRegion->rgLine[0]) ;
			pRegion->yTop = yLine ;
		}
		pRegion->yBottom = yLine ;
		//
		if ( xLeft < rctClip.left )
		{
			if ( pColors != NULL )
			{
				uint32_t	t =
					(rctClip.left - xLeft) * 0x100 / (xRight - xLeft) ;
				clrLeft = clrLeft.imul(0x100 - t) + clrRight.imul(t) ;
			}
			fxPos1 = (rctClip.left << 16) ;
			xLeft = rctClip.left ;
		}
		if ( xRight > rctClip.right )
		{
			if ( pColors != NULL )
			{
				uint32_t	t =
					(xRight - rctClip.right) * 0x100 / (xRight - xLeft) ;
				clrRight = clrRight.imul(0x100 - t) + clrLeft.imul(t) ;
			}
			fxPos2 = (rctClip.right << 16) | 0xFFFF ;
			xRight = rctClip.right ;
		}
		prgLine->fxLeft = fxPos1 ;
		prgLine->fxRight = fxPos2 ;
		prgLine->rgbaLeft = clrLeft ;
		prgLine->rgbaRight = clrRight ;
		areaPixels += xRight - xLeft + 1 ;
		prgLine ++ ;
	}
	pRegion->areaPixels = areaPixels ;
	return	(prgLine != NULL) ;
}


// 矩形リージョン生成
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglCreateRectangleRegion
	( SGLRegion * pRegion,
		const SGLRect & rctClip, const SGLRect & rctRegion )
{
	SGLRect	rect = rctRegion ;
	if ( !(rect &= rctClip) )
	{
		return	false ;
	}
	pRegion->yTop = rect.top ;
	pRegion->yBottom = rect.bottom ;
	pRegion->areaPixels = rect.GetWidth() * rect.GetHeight() ;
	//
	SGLRegionLine *	prgLine = &(pRegion->rgLine[0]) ;
	const int32_t	fxLeft = (rect.left << 16) ;
	const int32_t	fxRight = (rect.right << 16) | 0xFFFF ;
	for ( int y = rect.top; y <= rect.bottom; y ++ )
	{
		prgLine->fxLeft = fxLeft ;
		prgLine->fxRight = fxRight ;
		prgLine ++ ;
	}
	return	true ;
}

// 直線リージョン生成
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglCreateThinLineRegion
	( SGLRegion * pRegion,
		const SGLRect & rctClip,
		const S2DVector& vStart, const S2DVector& vEnd, int nFlags )
{
	SGLRect		rectClip = rctClip ;
	SGLPoint	pt0( (int) floor( vStart.x ), (int) floor( vStart.y ) ) ;
	SGLPoint	pt1( (int) floor( vEnd.x ), (int) floor( vEnd.y ) ) ;
	if ( pt0.y == pt1.y )
	{
		//
		// 横一線
		//
		if ( (pt1.y < rectClip.top) || (pt0.y > rectClip.bottom) )
		{
			return	false ;
		}
		bool	fLeftSideRight = false ;
		if ( pt0.x > pt1.x )
		{
			SGLPoint	ptTemp = pt0 ;
			pt0 = pt1 ;
			pt1 = ptTemp ;
			fLeftSideRight = true ;
		}
		if ( (pt1.x < rectClip.left) || (pt0.x > rectClip.right) )
		{
			return	false ;
		}
		if ( pt0.x < rectClip.left )
		{
			pt0.x = rectClip.left ;
		}
		if ( pt1.x > rectClip.right )
		{
			pt1.x = rectClip.right ;
		}
		if ( (!fLeftSideRight && (nFlags & thinLineExcludeStart))
			|| (fLeftSideRight && (nFlags & thinLineExcludeEnd)) )
		{
			pt0.x ++ ;
		}
		if ( (!fLeftSideRight && (nFlags & thinLineExcludeEnd))
			|| (fLeftSideRight && (nFlags & thinLineExcludeStart)) )
		{
			pt1.x -- ;
		}
		if ( pt0.x > pt1.x )
		{
			return	false ;
		}
		pRegion->yTop = pt0.y ;
		pRegion->yBottom = pt1.y ;
		pRegion->areaPixels = pt1.x - pt0.x + 1 ;
		pRegion->rgLine[0].fxLeft = pt0.x << 16 ;
		pRegion->rgLine[0].fxRight = (pt1.x << 16) | 0xFFFF ;
		return	true ;
	}
	//
	// ｙ方向正規化
	//
	bool	fUpSideDown = false ;
	if ( vStart.y > vEnd.y )
	{
		SGLPoint	ptTemp = pt0 ;
		pt0 = pt1 ;
		pt1 = ptTemp ;
		fUpSideDown = true ;
	}
	int32_t	yLine = pt0.y ;
	if ( yLine < rectClip.top )
	{
		yLine = rectClip.top ;
	}
	if ( yLine > rectClip.bottom )
	{
		return	false ;
	}
	//
	// 順次ライン処理
	//
	int64_t	fxDeltaX, fxX0, fxY0, fxX1, fxY1 ;
	if ( !fUpSideDown )
	{
		fxDeltaX = esl_lroundfi( ((vEnd.x - vStart.x)
							* 0x10000 / (vEnd.y - vStart.y)) ) ;
		fxX0 = esl_lroundfi( esl_fclamp( vStart.x * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxY0 = esl_lroundfi( esl_fclamp( vStart.y * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxX1 = esl_lroundfi( esl_fclamp( vEnd.x * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxY1 = esl_lroundfi( esl_fclamp( vEnd.y * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
	}
	else
	{
		fxDeltaX = esl_lroundfi
						( ((vStart.x - vEnd.x)
							* 0x10000 / (vStart.y - vEnd.y)) ) ;
		fxX0 = esl_lroundfi( esl_fclamp( vEnd.x * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxY0 = esl_lroundfi( esl_fclamp( vEnd.y * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxX1 = esl_lroundfi( esl_fclamp( vStart.x * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
		fxY1 = esl_lroundfi( esl_fclamp( vStart.y * 0x10000, -0x7FFFFFFF, 0x7FFFFFFF ) ) ;
	}
	int32_t		yBottom = yLine - 1 ;
	int32_t		xDeltaOne = (int32_t) (((fxDeltaX >> 63) << 1) + 1) ;
	int64_t		fxNextX = fxX0 ;
	uint32_t	areaPixels = 0 ;
	//
	SGLRegionLine *	prgLine = &(pRegion->rgLine[0]) ;
	bool	fValidLine = false ;
	//
	while ( (yLine <= rectClip.bottom) & (yLine <= pt1.y) )
	{
		int64_t	fxLX0 = fxNextX ;
		int64_t	fxLX1 =
			(((int64_t)(((yLine + 1) << 16) - fxY0)
							* fxDeltaX) >> 16) + fxX0 - xDeltaOne ;
		fxNextX = fxLX1 + xDeltaOne ;
		//
		if ( (yLine << 16) < fxY0 )
		{
			fxLX0 = fxX0 ;
			if ( nFlags & (thinLineExcludeStart | thinLineExcludeEnd) )
			{
				if ( (!fUpSideDown && (nFlags & thinLineExcludeStart))
					|| (fUpSideDown && (nFlags & thinLineExcludeEnd)) )
				{
					if ( (fxLX0 >> 16) == (fxLX1 >> 16) )
					{
						yLine ++ ;
						continue ;
					}
					fxLX0 += xDeltaOne ;
				}
			}
		}
		if ( ((yLine + 1) << 16) > fxY1 )
		{
			fxLX1 = fxX1 ;
			if ( nFlags & (thinLineExcludeStart | thinLineExcludeEnd) )
			{
				if ( (!fUpSideDown && (nFlags & thinLineExcludeEnd))
					|| (fUpSideDown && (nFlags & thinLineExcludeStart)) )
				{
					if ( (fxLX0 >> 16) == (fxLX1 >> 16) )
					{
						break ;
					}
					fxLX1 -= xDeltaOne ;
				}
			}
		}
		if ( fxLX0 > fxLX1 )
		{
			int64_t	fxLXTemp = fxLX0 ;
			fxLX0 = fxLX1 ;
			fxLX1 = fxLXTemp ;
		}
		int32_t	xLX0 = (int32_t) (fxLX0 >> 16) ;
		int32_t	xLX1 = (int32_t) (fxLX1 >> 16) ;
		if ( (xLX1 < rectClip.left) | (rectClip.right < xLX0) )
		{
			yLine ++ ;
			continue ;
		}
		if ( xLX0 < rectClip.left )
		{
			xLX0 = rectClip.left ;
			fxLX0 = (xLX0 << 16) ;
		}
		if ( xLX1 > rectClip.right )
		{
			xLX1 = rectClip.right;
			fxLX1 = (xLX1 << 16) | 0xFFFF ;
		}
		int32_t	pixels = xLX1 - xLX0 + 1 ;
		if ( pixels <= 1 )
		{
			prgLine->fxLeft = fxLX0 & ~0xFFFF ;
			prgLine->fxRight = (int32_t) fxLX1 | 0x0001 ;
		}
		else
		{
			prgLine->fxLeft = (int32_t) fxLX0 ;
			prgLine->fxRight = (int32_t) fxLX1 | 0x0001 ; ;
		}
		if ( !fValidLine )
		{
			pRegion->yTop = yLine ;
			fValidLine = true ;
		}
		yBottom = yLine ;
		areaPixels += pixels ;
		yLine ++ ;
		prgLine ++ ;
	}
	if ( !fValidLine || (pRegion->yTop > yBottom) )
	{
		return	false ;
	}
	pRegion->yBottom = yBottom ;
	pRegion->areaPixels = areaPixels ;
	return	true ;
}

// ベジェ曲線閉鎖領域（交差領域は除外）ビットマスク生成
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglHatchBezierBitmask
	( SGLImageBuffer& imgbuf,
		const S2DVector * pVertices, size_t nCount )
{
	bool	fFill = false ;
	size_t	nBeziers = (nCount - 1) / 3 ;
	for ( size_t i = 1; i + 1 <= nBeziers; i ++ )
	{
		S2DVector	vTriangle[3] ;
		vTriangle[0] = pVertices[0] ;
		vTriangle[1] = pVertices[i * 3] ;
		vTriangle[2] = pVertices[i * 3 + 3] ;
		//
		fFill |= sglFillXorTriangleBitmask( imgbuf, &vTriangle[0] ) ;
	}
	//
	SGLBezierCurves<S2DVector>	bzCurve ;
	bzCurve.SetLength( 4 ) ;
	//
	S2DVector	bufSubVert[0x100] ;
	S2DVector *	pbzCurve = bzCurve.GetArray() ;
	for ( size_t i = 0; i < nBeziers; i ++ )
	{
		pbzCurve[0] = pVertices[i * 3] ;
		pbzCurve[1] = pVertices[i * 3 + 1] ;
		pbzCurve[2] = pVertices[i * 3 + 2] ;
		pbzCurve[3] = pVertices[i * 3 + 3] ;
		//
		double	r = 0.0 ;
		r += (pbzCurve[1] - pbzCurve[0]).Absolute() ;
		r += (pbzCurve[2] - pbzCurve[1]).Absolute() ;
		r += (pbzCurve[3] - pbzCurve[2]).Absolute() ;
		//
		size_t	nDiv = (int) sqrt( r ) + 1 ;
		if ( nDiv <= 1 )
		{
			continue ;
		}
		else if ( nDiv >= 0xFF )
		{
			nDiv = 0xFF ;
		}
		double	dt = 1.0 / (double) nDiv ;
		for ( size_t j = 1; j < nDiv; j ++ )
		{
			bzCurve.PointAt( bufSubVert[j], (double) j * dt, 0 ) ;
		}
		bufSubVert[0] = pbzCurve[0] ;
		bufSubVert[nDiv] = pbzCurve[3] ;
		//
		fFill |= sglFillPolygonBitmask
					( imgbuf, &bufSubVert[0], nDiv + 1 ) ;
	}
	bzCurve.FinishArray() ;
	return	fFill ;
}

// 多角形（交差領域は除外）ビットマスク生成
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglFillPolygonBitmask
	( SGLImageBuffer& imgbuf,
		const S2DVector * pVertices, size_t nCount )
{
	bool	fFill = false ;
	for ( size_t i = 1; i + 1 < nCount; i ++ )
	{
		S2DVector	vTriangle[3] ;
		vTriangle[0] = pVertices[0] ;
		vTriangle[1] = pVertices[i] ;
		vTriangle[2] = pVertices[i + 1] ;
		//
		fFill |= sglFillXorTriangleBitmask( imgbuf, &vTriangle[0] ) ;
	}
	return	fFill ;
}

// 三角形ビットマスク反転描画
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::sglFillXorTriangleBitmask
	( SGLImageBuffer& imgbuf, const S2DVector * pVertices )
{
	ESLAssert( imgbuf.depth == 1 ) ;
	size_t	iTop, iMiddle, iBottom ;
	if ( pVertices[0].y < pVertices[1].y )
	{
		if ( pVertices[0].y < pVertices[2].y )
		{
			iTop = 0 ;
			if ( pVertices[1].y < pVertices[2].y )
			{
				iMiddle = 1 ;
				iBottom = 2 ;
			}
			else
			{
				iMiddle = 2 ;
				iBottom = 1 ;
			}
		}
		else
		{
			iTop = 2 ;
			iMiddle = 0 ;
			iBottom = 1 ;
		}
	}
	else if ( pVertices[1].y < pVertices[2].y )
	{
		iTop = 1 ;
		if ( pVertices[0].y < pVertices[2].y )
		{
			iMiddle = 0 ;
			iBottom = 2 ;
		}
		else
		{
			iMiddle = 2 ;
			iBottom = 0 ;
		}
	}
	else
	{
		iTop = 2 ;
		iMiddle = 1 ;
		iBottom = 0 ;
	}
	S2DVector	vTop = pVertices[iTop] ;
	S2DVector	vMiddle = pVertices[iMiddle] ;
	S2DVector	vBottom = pVertices[iBottom] ;
	//
	const int32_t	width = (int32_t) imgbuf.width ;
	const int32_t	height = (int32_t) imgbuf.height ;
	const int32_t	pitchLine = imgbuf.pitchLine ;
	//
	int32_t	yTop = ((eslRoundR32ToInt( vTop.y * 0x10000 ) + 0xFFFF) >> 16) ;
	int32_t	yMiddle = ((eslRoundR32ToInt( vMiddle.y * 0x10000 ) + 0xFFFF) >> 16) ;
	int32_t	yBottom = ((eslRoundR32ToInt( vBottom.y * 0x10000 ) + 0xFFFF) >> 16) ;
	//
	bool	fFill = false ;
	if ( vMiddle.y > 0 )
	{
		if ( yTop < yMiddle )
		{
			double	dx0 = (vBottom.x - vTop.x) / (vBottom.y - vTop.y) ;
			double	dx1 = (vMiddle.x - vTop.x) / (vMiddle.y - vTop.y) ;
			if ( yTop < 0 )
			{
				yTop = 0 ;
			}
			if ( yMiddle >= height )
			{
				if ( yTop >= height )
				{
					return	false ;
				}
				yMiddle = height ;
			}
			uint8_t *	pbytLine = imgbuf.ptrBuffer + pitchLine * yTop ;
			for ( int32_t y = yTop; y < yMiddle; y ++, pbytLine += pitchLine )
			{
				float32_t	dy = y - vTop.y ;
				int32_t	x0 = eslRoundR32ToInt
						( (float32_t) (vTop.x + dy * dx0) * 0x10000 ) ;
				int32_t	x1 = eslRoundR32ToInt
						( (float32_t) (vTop.x + dy * dx1) * 0x10000 ) ;
				if ( x0 > x1 )
				{
					int32_t	t = x0 ;
					x0 = x1 ;
					x1 = t ;
				}
				else if ( x0 == x1 )
				{
					continue ;
				}
				x0 = (x0 + 0xFFFF) >> 16 ;
				x1 = (x1 - 1) >> 16 ;
				if ( (x1 < 0) || (x0 >= width) || (x0 > x1) )
				{
					continue ;
				}
				if ( x0 < 0 )
				{
					x0 = 0 ;
				}
				if ( x1 >= width )
				{
					x1 = width - 1 ;
				}
				size_t		ix0 = x0 >> 3 ;
				size_t		ix1 = x1 >> 3 ;
				uint32_t	bm0 = (0x100 >> (x0 & 0x07)) - 1 ;
				uint32_t	bm1 = (-0x80 >> (x1 & 0x07)) ;
				if ( ix0 == ix1 )
				{
					pbytLine[ix0] ^= (uint8_t) (bm0 & bm1) ;
				}
				else
				{
					pbytLine[ix0] ^= (uint8_t) bm0 ;
					for ( size_t i = ix0 + 1; i < ix1; i ++ )
					{
						pbytLine[i] ^= 0xFF ;
					}
					pbytLine[ix1] ^= (uint8_t) bm1 ;
				}
				fFill = true ;
			}
		}
	}
	if ( yMiddle < yBottom )
	{
		double	dx0 = (vBottom.x - vTop.x) / (vBottom.y - vTop.y) ;
		double	dx1 = (vBottom.x - vMiddle.x) / (vBottom.y - vMiddle.y) ;
		if ( yMiddle < 0 )
		{
			yMiddle = 0 ;
		}
		if ( yBottom >= height )
		{
			if ( yMiddle >= height )
			{
				return	fFill ;
			}
			yBottom = height ;
		}
		uint8_t *	pbytLine = imgbuf.ptrBuffer + pitchLine * yMiddle ;
		for ( int32_t y = yMiddle; y < yBottom; y ++, pbytLine += pitchLine )
		{
			int32_t	x0 = eslRoundR32ToInt
					( (float32_t) (vTop.x + (y - vTop.y) * dx0) * 0x10000 ) ;
			int32_t	x1 = eslRoundR32ToInt
					( (float32_t) (vMiddle.x + (y - vMiddle.y) * dx1) * 0x10000 ) ;
			if ( x0 > x1 )
			{
				int32_t	t = x0 ;
				x0 = x1 ;
				x1 = t ;
			}
			else if ( x0 == x1 )
			{
				continue ;
			}
			x0 = (x0 + 0xFFFF) >> 16 ;
			x1 = (x1 - 1) >> 16 ;
			if ( (x1 < 0) || (x0 >= width) || (x0 > x1) )
			{
				continue ;
			}
			if ( x0 < 0 )
			{
				x0 = 0 ;
			}
			if ( x1 >= width )
			{
				x1 = width - 1 ;
			}
			size_t		ix0 = x0 >> 3 ;
			size_t		ix1 = x1 >> 3 ;
			uint32_t	bm0 = (0x100 >> (x0 & 0x07)) - 1 ;
			uint32_t	bm1 = (-0x80 >> (x1 & 0x07)) ;
			if ( ix0 == ix1 )
			{
				pbytLine[ix0] ^= (uint8_t) (bm0 & bm1) ;
			}
			else
			{
				pbytLine[ix0] ^= (uint8_t) bm0 ;
				for ( size_t i = ix0 + 1; i < ix1; i ++ )
				{
					pbytLine[i] ^= 0xFF ;
				}
				pbytLine[ix1] ^= (uint8_t) bm1 ;
			}
			fFill = true ;
		}
	}
	return	fFill ;
}

