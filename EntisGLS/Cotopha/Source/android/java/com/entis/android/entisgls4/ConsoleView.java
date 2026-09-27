package com.entis.android.entisgls4 ;

import java.nio.* ;
import java.util.Vector ;
import android.graphics.* ;
import android.opengl.GLUtils ;
import javax.microedition.khronos.opengles.GL10 ;


public class ConsoleView	extends ViewInterface
{
	// 画像
	protected Bitmap		m_bitmap = null ;
	protected boolean		m_flagUpdateImage = true ;
	protected boolean		m_flagUpdateGL = true ;
	protected int			m_glTexture = 0 ;

	protected static final int	m_sizeFont = 16 ;
	protected static final int	m_width = 800 ;
	protected static final int	m_height = 480 ;
	protected static final int	m_widthTexture = 1024 ;
	protected static final int	m_heightTexture = 512 ;

	// ビューポート変換
	protected EntisViewTransformation
						m_vt = new EntisViewTransformation() ;

	// ビュー配列
	protected Vector<String>	m_vecLines = new Vector<String>() ;


	// 初期化
	//////////////////////////////////////////////////////////////////////////
	public void initialize()
	{
		m_bitmap =
			Bitmap.createBitmap
				( m_widthTexture,
					m_heightTexture, Bitmap.Config.ARGB_8888 ) ;
		m_vt.m_nViewWidth = m_width ;
		m_vt.m_nViewHeight = m_height ;
	}

	// テクスチャ更新
	//////////////////////////////////////////////////////////////////////////
	public void updateTexture( GL10 gl )
	{
		if ( m_glTexture == 0 )
		{
			int[]	tex = new int[1] ;
			gl.glGenTextures( 1, tex, 0 ) ;
			m_glTexture = tex[0] ;
		}
		if ( m_flagUpdateGL )
		{
			gl.glBindTexture( GL10.GL_TEXTURE_2D, m_glTexture ) ;
			GLUtils.texImage2D( GL10.GL_TEXTURE_2D, 0, m_bitmap, 0 ) ;
			gl.glTexParameterf
				( GL10.GL_TEXTURE_2D,
					GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR ) ;
			gl.glTexParameterf
				( GL10.GL_TEXTURE_2D,
					GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR ) ;
			gl.glBindTexture( GL10.GL_TEXTURE_2D, 0 ) ;
			m_flagUpdateGL = false ;
		}
	}

	// 画像更新
	//////////////////////////////////////////////////////////////////////////
	public synchronized void updateImage()
	{
		Canvas	canvas = new Canvas( m_bitmap ) ;
		Paint	paint = new Paint() ;
		paint.setTextSize( m_sizeFont ) ;
		paint.setTypeface( Typeface.DEFAULT ) ;
		paint.setColor( 0 ) ;
        paint.setXfermode
			( new PorterDuffXfermode( PorterDuff.Mode.SRC ) ) ;
		canvas.drawRect
			( 0.0f, 0.0f, (float) m_widthTexture,
							(float) m_heightTexture, paint ) ;
		//
		paint.setColor( 0xFFFFFFFF ) ;
        paint.setXfermode
			( new PorterDuffXfermode( PorterDuff.Mode.SRC_OVER ) ) ;
		final int	countLines = m_vecLines.size() ;
		for ( int i = 0; i < countLines; i ++ )
		{
			String	sLine = m_vecLines.get( i ) ;
			if ( sLine != null )
			{
				char[]	chText = sLine.toCharArray() ;
				for ( int j = 0; j < chText.length; j ++ )
				{
					if ( chText[j] <= 0x20 )
					{
						chText[j] = ' ' ;
					}
				}
				canvas.drawText
					( new String( chText ), 0.0f,
						(float) (i * m_sizeFont + m_sizeFont), paint ) ;
			}
		}
		m_flagUpdateGL = true ;
	}

	// 文字出力
	//////////////////////////////////////////////////////////////////////////
	public synchronized void printLine( String text )
	{
		if ( text == null )
		{
			return ;
		}
		final int	countMax = m_height / m_sizeFont ;
		boolean		flagClearLine = false ;
		int			iCR = text.indexOf( '\r' ) ;
		if ( iCR >= 0 )
		{
			if ( (iCR + 1 < text.length())
					&& (text.charAt( iCR + 1 ) != '\n') )
			{
				flagClearLine = true ;
				text = text.substring( iCR + 1 ) ;
			}
		}
		boolean		flagNewLine = false ;
		if ( m_vecLines.size() > 0 )
		{
			String	sLine = m_vecLines.get( m_vecLines.size() - 1 ) ;
			if ( sLine != null )
			{
				if ( sLine.indexOf( '\n' ) >= 0 )
				{
					flagNewLine = true ;
				}
				else if ( flagClearLine )
				{
					sLine = text ;
				}
				else
				{
					sLine += text ;
				}
			}
			else
			{
				sLine = text ;
			}
			m_vecLines.set( m_vecLines.size() - 1, sLine ) ;
		}
		else
		{
			flagNewLine = true ;
		}
		if ( flagNewLine )
		{
			while ( m_vecLines.size() >= countMax )
			{
				m_vecLines.remove( 0 ) ;
			}
			m_vecLines.add( text ) ;
		}
		EntisGLS.postUpdateView() ;
	}

	// 最後のライン取得
	//////////////////////////////////////////////////////////////////////////
	public String getLastLine()
	{
		if ( m_vecLines.size() > 0 )
		{
			return	m_vecLines.get( m_vecLines.size() - 1 ) ;
		}
		return	null ;
	}
	
	// 現在のライン取得
	//////////////////////////////////////////////////////////////////////////
	public String getCurrentLine()
	{
		String	sLine = getLastLine() ;
		if ( sLine != null )
		{
			if ( sLine.indexOf( '\n' ) >= 0 )
			{
				return	null ;
			}
		}
		return	sLine ;
	}
	
	// 画面サイズ変更
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void onSurfaceChanged( int width, int height )
	{
		m_vt.m_nWidth = width ;
		m_vt.m_nHeight = height ;
		m_vt.updateRendererViewPort() ;
	}

	// 描画
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void draw( GL10 gl )
	{
		// 2D 描画用座標系設定
		gl.glViewport
			( m_vt.m_xViewPort, m_vt.m_yViewPort,
						m_vt.m_widthViewPort, m_vt.m_heightViewPort ) ;
		gl.glMatrixMode( GL10.GL_PROJECTION ) ;
		gl.glLoadIdentity() ;
		gl.glOrthof
			( m_vt.m_xViewLeft, m_vt.m_xViewRight,
					m_vt.m_yViewTop, m_vt.m_yViewBottom, 0.5f, -0.5f ) ;
		//
		gl.glMatrixMode( GL10.GL_MODELVIEW ) ;
		gl.glLoadIdentity() ;

		// テクスチャ取得
		if ( m_flagUpdateImage )
		{
			updateImage() ;
		}
		if ( m_flagUpdateGL )
		{
			updateTexture( gl ) ;
		}

		// 描画
		float[]		vertices =
		{
			0.0f, 0.0f,
			(float) m_width, 0.0f,
			(float) m_width, (float) m_height,
			0.0f, (float) m_height,
		} ;
		float[]		colors =
		{
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
		} ;
		float[]		coords =
		{
			0.0f, 0.0f,
			(float) m_width / (float) m_widthTexture, 0.0f,
			(float) m_width / (float) m_widthTexture,
					(float) m_height / (float) m_heightTexture,
			0.0f, (float) m_height / (float) m_heightTexture,
		} ;
		//
		gl.glEnable( GL10.GL_BLEND ) ;
		gl.glBlendFunc( GL10.GL_SRC_ALPHA, GL10.GL_ONE_MINUS_SRC_ALPHA ) ;
		//
		gl.glDisable( GL10.GL_CULL_FACE ) ;
		gl.glEnable( GL10.GL_TEXTURE_2D ) ;
		gl.glBindTexture( GL10.GL_TEXTURE_2D, m_glTexture ) ;
		gl.glTexParameterf
			( GL10.GL_TEXTURE_2D,
				GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR ) ;
		gl.glTexParameterf
			( GL10.GL_TEXTURE_2D,
				GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR ) ;
		gl.glTexParameterf
			( GL10.GL_TEXTURE_2D,
					GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE ) ;
		gl.glTexParameterf
			( GL10.GL_TEXTURE_2D,
					GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE ) ;
		//
		gl.glVertexPointer
			( 2, GL10.GL_FLOAT, 0, makeFloatBuffer(vertices) ) ;
		gl.glEnableClientState( GL10.GL_VERTEX_ARRAY ) ;
		gl.glColorPointer
			( 4, GL10.GL_FLOAT, 0, makeFloatBuffer(colors) ) ;
		gl.glEnableClientState( GL10.GL_COLOR_ARRAY ) ;
		gl.glTexCoordPointer
			( 2, GL10.GL_FLOAT, 0, makeFloatBuffer(coords) ) ;
		gl.glEnableClientState( GL10.GL_TEXTURE_COORD_ARRAY ) ;
		gl.glDrawArrays( GL10.GL_TRIANGLE_FAN, 0, 4 ) ;

		gl.glDisableClientState( GL10.GL_TEXTURE_COORD_ARRAY ) ;
		gl.glDisable( GL10.GL_TEXTURE_2D ) ;
		gl.glDisable( GL10.GL_BLEND ) ;
	}

	public static FloatBuffer makeFloatBuffer( float[] buf )
	{
		ByteBuffer	bb = ByteBuffer.allocateDirect( buf.length * 4 ) ;
		bb.order( ByteOrder.nativeOrder() ) ;
		//
		FloatBuffer	fb = bb.asFloatBuffer() ;
		fb.put( buf ) ;
		fb.position( 0 ) ;
		return	fb ;
	}

}

