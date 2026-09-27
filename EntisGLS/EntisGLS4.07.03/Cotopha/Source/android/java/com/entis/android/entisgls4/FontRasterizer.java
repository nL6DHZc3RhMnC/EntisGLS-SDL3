package com.entis.android.entisgls4 ;

import android.graphics.* ;

public class FontRasterizer
{
	protected Paint		m_paint = new Paint() ;

	public static class	Cache
	{
		public char		m_code = 0 ;
		public Metrics	m_metrics = null ;
		public byte[]	m_image = null ;
	}
	protected Cache[]	m_cache = new Cache[0x100] ;

	protected float[]	m_tmpWidths = new float[1] ;
	protected char[]	m_tmpText = new char[1] ;
	protected Rect		m_tmpRect = new Rect() ;

	protected Canvas	m_cvsWork = null ;
	protected Bitmap	m_bmpWork = null ;
	protected int		m_wCanvas = -1 ;
	protected int		m_hCanvas = -1 ;


	// 構築関数
	//////////////////////////////////////////////////////////////////////////
	public FontRasterizer()
	{
		m_paint.setAntiAlias( true ) ;
	}

	// フォント設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setStyle( String sFontFace, int nSize, int nStyle )
	{
		Typeface	tf =
				FontManager.getTypeFaceAs( sFontFace, nStyle ) ;
		if ( tf != null )
		{
			m_paint.setTypeface( tf ) ;
		}
		m_paint.setTextSize( (float) nSize ) ;
		m_cache = new Cache[0x100] ;
		return	(tf != null) ;
	}

	// フォント情報取得
	//////////////////////////////////////////////////////////////////////////
	public static class	Metrics
	{
		public int	nFlags ;
		public int	nAscent ;
		public int	nDescent ;
		public int	nLeading ;
		public int	nWidth ;
		public int	nHeight ;
		public int	xExterior ;
		public int	yExterior ;
		public int	wExterior ;
		public int	hExterior ;
	} ;
	public Metrics getMetrics( char c, byte[] bufImage )
	{
		int		iCache = (c ^ (c >> 8)) & 0xFF ;
		Cache	cache = m_cache[iCache] ;
		if ( (cache != null) && (cache.m_code == c) )
		{
			if ( bufImage != null )
			{
				byte[]	bufCache = cache.m_image ;
				int	nBytes = bufImage.length ;
				if ( nBytes > bufCache.length )
				{
					nBytes = bufCache.length ;
				}
				for ( int i = 0; i < nBytes; i ++ )
				{
					bufImage[i] = bufCache[i] ;
				}
			}
			return	cache.m_metrics ;
		}
		//
		Metrics	m = new Metrics() ;
		m.nFlags = 0 ;
		//
		m.nHeight = Math.round( m_paint.getTextSize() ) ;
		m.nAscent = m.nHeight ;
		m.nDescent = 0 ;
		m.nLeading = 0 ;
		//
		float[]	widths = m_tmpWidths ;
		char[]	text = m_tmpText ;
		text[0] = c ;
		m_paint.getTextWidths( text, 0, 1, widths ) ;
		m.nWidth = Math.round( widths[0] ) ;
		//
		Rect	rect = m_tmpRect ;
		m_paint.getTextBounds( text, 0, 1, rect ) ;
		m.xExterior = rect.left ;
		m.yExterior = rect.top + m.nHeight ;
		m.wExterior = rect.right - rect.left ;
		m.hExterior = rect.bottom - rect.top ;
		//
		if ( bufImage != null )
		{
			if ( (m_cvsWork == null)
				|| (m_wCanvas < m.wExterior)
				|| (m_hCanvas < m.hExterior) )
			{
				if ( m_wCanvas < m.wExterior )
				{
					m_wCanvas = m.wExterior ;
				}
				if ( m_hCanvas < m.hExterior )
				{
					m_hCanvas = m.hExterior ;
				}
				m_bmpWork =
					Bitmap.createBitmap
						( m_wCanvas, m_hCanvas, Bitmap.Config.ARGB_8888 ) ;
				m_cvsWork = new Canvas( m_bmpWork ) ;
			}
			Bitmap	bmp = m_bmpWork ;
			Canvas	canvas = m_cvsWork ;
			//
            m_paint.setXfermode
				( new PorterDuffXfermode( PorterDuff.Mode.SRC ) ) ;
			m_paint.setColor( 0 ) ;
			canvas.drawRect
				( 0, 0, m.wExterior, m.hExterior, m_paint ) ;
			//
			m_paint.setColor( 0xFFFFFFFF ) ;
            m_paint.setXfermode
				( new PorterDuffXfermode( PorterDuff.Mode.SRC_OVER ) ) ;
			canvas.drawText
				( new String(text), -rect.left, -rect.top, m_paint ) ;
			//
			int	countPixels = m.wExterior * m.hExterior ;
			int[]	pixels = new int[countPixels] ;
			bmp.getPixels
				( pixels, 0, m.wExterior, 0, 0, m.wExterior, m.hExterior ) ;
			if ( countPixels > bufImage.length )
			{
				countPixels = bufImage.length ;
			}
			for ( int i = 0; i < countPixels; i ++ )
			{
				bufImage[i] = (byte) ((pixels[i] >> 24) & 0xFF) ;
			}
			//
			cache = new Cache() ;
			cache.m_code = c ;
			cache.m_metrics = m ;
			cache.m_image = bufImage ;
			m_cache[iCache] = cache ;
		}
		return	m ;
	} ;

}

