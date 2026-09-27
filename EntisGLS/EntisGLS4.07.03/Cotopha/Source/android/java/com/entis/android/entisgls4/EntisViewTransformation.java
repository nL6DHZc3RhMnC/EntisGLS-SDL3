package com.entis.android.entisgls4 ;


public class EntisViewTransformation
{
	// サーフェスの実際のサイズ
	public int		m_nWidth = 800 ;
	public int		m_nHeight = 480 ;

	// 擬似的なビューのサイズ
	public int		m_nViewWidth = 640 ;
	public int		m_nViewHeight = 480 ;
	public boolean	m_fRotatable = false ;

	// ビューポート
	public int		m_xViewPort = 0 ;
	public int		m_yViewPort = 0 ;
	public int		m_widthViewPort = 640 ;
	public int		m_heightViewPort = 480 ;
	//
	public float	m_xViewLeft = 0.0f ;
	public float	m_xViewRight = 640.0f ;
	public float	m_yViewTop = 480.0f ;
	public float	m_yViewBottom = 0.0f ;
	public float	m_zViewRotate = 0.0f ;

	// 擬似的な座標→実際の座標変換行列
	public float	m_matView00, m_matView01, m_matView02 ;
	public float	m_matView10, m_matView11, m_matView12 ;

	// 実際の座標→擬似的な座標変換行列
	public float	m_matIView00, m_matIView01 ;
	public float	m_matIView10, m_matIView11 ;


	//////////////////////////////////////////////////////////////////////////
	// ビューポートセットアップ
	//////////////////////////////////////////////////////////////////////////
	protected void updateRendererViewPort()
	{
		boolean	fRotate = false ;
		int		nWidth = m_nWidth ;
		int		nHeight = m_nHeight ;
		if ( m_fRotatable )
		{
			// 回転しない（Android 側の向き変更に逆らわないで回転する）
			if ( ((m_nViewWidth > m_nViewHeight) && (m_nWidth < m_nHeight))
				|| ((m_nViewWidth < m_nViewHeight) && (m_nWidth > m_nHeight)) )
			{
				fRotate = true ;
				nWidth = m_nHeight ;
				nHeight = m_nWidth ;
			}
		}
		int	xViewPort = 0, yViewPort = 0 ;
		int	widthViewPort = nWidth ;
		int	heightViewPort = nHeight ;
		if ( nWidth * m_nViewHeight <= nHeight * m_nViewWidth )
		{
			// 横の比率に合わせる
			heightViewPort = m_nViewHeight * nWidth / m_nViewWidth ;
			yViewPort = (nHeight - heightViewPort) / 2 ;
		}
		else
		{
			// 縦の比率に合わせる
			widthViewPort = m_nViewWidth * nHeight / m_nViewHeight ;
			xViewPort = (nWidth - widthViewPort) / 2 ;
		}
		if ( fRotate )
		{
			m_xViewPort = yViewPort ;
			m_yViewPort = xViewPort ;
			m_widthViewPort = heightViewPort ;
			m_heightViewPort = widthViewPort ;
			//
			float	xScale = (float) widthViewPort / (float) m_nViewWidth ;
			float	yScale = (float) heightViewPort / (float) m_nViewHeight ;
			m_matView00 = 0.0f ;
			m_matView01 = - yScale ;
			m_matView02 = (float) m_widthViewPort ;
			m_matView10 = xScale ;
			m_matView11 = 0.0f ;
			m_matView12 = 0.0f ;
			//
			m_xViewLeft = 0.0f ;
			m_xViewRight = (float) m_nViewHeight ;
			m_yViewBottom = 0.0f ;
			m_yViewTop = (float) m_nViewWidth ;
			m_zViewRotate = 0.0f ;
		}
		else
		{
			m_xViewPort = xViewPort ;
			m_yViewPort = yViewPort ;
			m_widthViewPort = widthViewPort ;
			m_heightViewPort = heightViewPort ;
			//
			float	xScale = (float) m_widthViewPort / (float) m_nViewWidth ;
			float	yScale = (float) m_heightViewPort / (float) m_nViewHeight ;
			m_matView00 = xScale ;
			m_matView01 = 0.0f ;
			m_matView02 = 0.0f ;
			m_matView10 = 0.0f ;
			m_matView11 = yScale ;
			m_matView12 = 0.0f ;
			//
			m_xViewLeft = 0.0f ;
			m_xViewRight = (float) m_nViewWidth ;
			m_yViewBottom = 0.0f ;
			m_yViewTop = (float) m_nViewHeight ;
			m_zViewRotate = 0.0f ;
		}
		double	d = m_matView00 * m_matView11 - m_matView01 * m_matView10 ;
		if ( Math.abs( d ) > 1.0e-8 )
		{
			d = 1.0 / d ;
		}
		m_matIView00 = (float) (  m_matView11 * d) ;
		m_matIView01 = (float) (- m_matView01 * d) ;
		m_matIView10 = (float) (- m_matView10 * d) ;
		m_matIView11 = (float) (  m_matView00 * d) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 座標変換
	//////////////////////////////////////////////////////////////////////////
	public final void pointFromScreen( float[] pos )
	{
		double	x0 = pos[0] - (m_matView02 + m_xViewPort) ;
		double	y0 = pos[1] - (m_matView12 + m_yViewPort) ;
		double	x = x0 * m_matIView00 + y0 * m_matIView01 ;
		double	y = x0 * m_matIView10 + y0 * m_matIView11 ;
		pos[0] = (float) x ;
		pos[1] = (float) y ;
	}
	public final void pointToScreen( float[] pos )
	{
		double	x = pos[0] * m_matView00
					+ pos[1] * m_matView01
					+ (m_matView02 + m_xViewPort) ;
		double	y = pos[0] * m_matView10
					+ pos[1] * m_matView11
					+ (m_matView12 + m_yViewPort) ;
		pos[0] = (float) x ;
		pos[1] = (float) y ;
	}
	public final void verticesToScreen( float[] vertices )
	{
		for ( int i = 0; i < vertices.length; i += 2 )
		{
			double	x0 = vertices[i] ;
			double	y0 = vertices[i + 1] ;
			double	x1 = x0 * m_matView00 + y0 * m_matView01 + m_matView02 ;
			double	y1 = x0 * m_matView10 + y0 * m_matView11 + m_matView12 ;
			vertices[i]     = (float) x1 ;
			vertices[i + 1] = (float) y1 ;
		}
	}

}
