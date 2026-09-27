package com.entis.android.entisgls4 ;

import java.util.* ;
import java.lang.reflect.Method ;

import android.graphics.PixelFormat ;
import android.opengl.GLSurfaceView ;
import android.view.* ;
import android.content.Context ;
import android.opengl.GLSurfaceView.Renderer ;
#if	ANDROID_API_LEVEL >= 8
import android.content.pm.ConfigurationInfo ;
import android.app.ActivityManager ;
#endif

import javax.microedition.khronos.egl.EGL10 ;
import javax.microedition.khronos.egl.EGLConfig ;
import javax.microedition.khronos.egl.EGLContext ;
import javax.microedition.khronos.egl.EGLDisplay ;
import javax.microedition.khronos.egl.EGLSurface ;
import javax.microedition.khronos.opengles.GL10 ;


public class EntisGLSurfaceView	extends GLSurfaceView
									implements Renderer, Runnable
{
	// GL10 オブジェクト
	protected GL10		m_gl = null ;
	protected long		m_idGLThread = 0 ;

	// ビューサイズ
	protected int		m_widthView = 800 ;
	protected int		m_heightView = 480 ;

	// アクティブ状態
	protected boolean	m_flagActive = false ;

	// 表示状態
	protected long		m_countHideView = 0 ;

	// ビュー配列
	protected Vector<ViewInterface>
						m_vecViews = new Vector<ViewInterface>() ;

	// コンソール・ビュー
	protected ConsoleView	m_viewConsole = null ;

#if	ANDROID_API_LEVEL >= 8
	// ディスプレイ
	protected int			m_nGLESVersion = 2 ;
	protected EGL10			m_egl = null ;
	protected EGLDisplay	m_display = null ;
	protected EGLContext	m_context = null ;
	protected EGLContext	m_ctxNonRender = null ;
	protected EGLSurface	m_eglSurface2nd = null ;

	// 非レンダリングスレッド
	protected Thread		m_thread = null ;
	protected boolean		m_flagQuitNoRender = false ;
	protected ArrayList<Runnable>
							m_queNonRender = new ArrayList<Runnable>() ;


	//////////////////////////////////////////////////////////////////////////
	// OpenGL コンテキスト・ファクトリ
	//////////////////////////////////////////////////////////////////////////

	protected static class	ContextFactory
						implements	GLSurfaceView.EGLContextFactory
	{
		private static int				EGL_CONTEXT_CLIENT_VERSION = 0x3098 ;
		protected EntisGLSurfaceView	m_sufview = null ;
		protected int					m_nGLESVersion ;

		public ContextFactory( EntisGLSurfaceView sufview, int nVersion )
		{
			m_sufview = sufview ;
			m_nGLESVersion = nVersion ;
		}

		public EGLContext createContext
			( EGL10 egl, EGLDisplay display, EGLConfig eglConfig )
		{
			EntisGLS.logDebug
				( "creating OpenGL ES " + m_nGLESVersion + ".0 context" ) ;
			//
			int[]	attrib_list =
			{
				EGL_CONTEXT_CLIENT_VERSION,	m_nGLESVersion,
				EGL10.EGL_NONE,
			} ;
			EGLContext	context =
				egl.eglCreateContext
					( display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list ) ;
			m_sufview.m_egl = egl ;
			m_sufview.m_display = display ;
			m_sufview.m_context = context ;
			return	context ;
		}

		public void destroyContext
			( EGL10 egl, EGLDisplay display, EGLContext context )
		{
			egl.eglDestroyContext( display, context ) ;
		}

		private static void checkEGLError( String msg, EGL10 egl )
		{
			int	error ;
			while ( (error = egl.eglGetError()) != EGL10.EGL_SUCCESS )
			{
				EntisGLS.logError
					( msg + String.format( ": EGL error: 0x%x", error) ) ;
			}
		}
	}



	//////////////////////////////////////////////////////////////////////////
	// OpenGL コンフィグ選択
	//////////////////////////////////////////////////////////////////////////

    private static class	ConfigChooser
					implements	GLSurfaceView.EGLConfigChooser
	{
		// 最低条件 RGBADS
		protected int	m_bitsReqRed ;
		protected int	m_bitsReqGreen ;
		protected int	m_bitsReqBlue ;
		protected int	m_bitsReqAlpha ;
		protected int	m_bitsReqDepth ;
		protected int	m_bitsReqStencil ;

		// 一致条件 RGBADS
		protected int	m_bitsBestRed ;
		protected int	m_bitsBestGreen ;
		protected int	m_bitsBestBlue ;
		protected int	m_bitsBestAlpha ;
		protected int	m_bitsBestDepth ;
		protected int	m_bitsBestStencil ;

		private int[]	m_valueAttrTemp = new int[1] ;

		// 構築関数
		public ConfigChooser
			( int reqRed, int reqGreen, int reqBlue,
					int reqAlpha, int reqDepth, int reqStencil,
				int bestRed, int bestGreen, int bestBlue,
					int bestAlpha, int bestDepth, int bestStencil )
		{
			m_bitsReqRed = reqRed ;
			m_bitsReqGreen = reqGreen ;
			m_bitsReqBlue = reqBlue ;
			m_bitsReqAlpha = reqAlpha ;
			m_bitsReqDepth = reqDepth ;
			m_bitsReqStencil = reqStencil ;
			m_bitsBestRed = bestRed ;
			m_bitsBestGreen = bestGreen ;
			m_bitsBestBlue = bestBlue ;
			m_bitsBestAlpha = bestAlpha ;
			m_bitsBestDepth = bestDepth ;
			m_bitsBestStencil = bestStencil ;
		}

		// OpenGL ES 2.0 レンダリングに必要な EGL コンフィグ指定
		// RGB 各要素に 4bit 以上, Depth 要素ありを選択
		private static int		EGL_OPENGL_ES2_BIT = 4 ;
		private static int[]	m_attribs_min_es2 =
		{
			EGL10.EGL_RED_SIZE,			4,
			EGL10.EGL_GREEN_SIZE,		4,
			EGL10.EGL_BLUE_SIZE,		4,
			EGL10.EGL_DEPTH_SIZE,		8,
			EGL10.EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL10.EGL_NONE,
		} ;

		// 選択関数実装
		public EGLConfig chooseConfig( EGL10 egl, EGLDisplay display )
		{
			// 最低限度条件を満たす EGL コンフィグ検索
			int[]	num_config = new int[1] ;
			egl.eglChooseConfig
				( display, m_attribs_min_es2, null, 0, num_config ) ;

			EGLConfig[]	configs = null ;
			int	numConfigs = num_config[0] ;
			if ( numConfigs <= 0 )
			{
				EntisGLS.logError( "No configs match configSpec" ) ;
				egl.eglGetConfigs( display, null, 0, num_config ) ;
				numConfigs = num_config[0] ;
				if ( numConfigs <= 0 )
				{
					EntisGLS.logError( "No configs eglGetConfigs" ) ;
				    throw new IllegalArgumentException( "No configs match configSpec" ) ;
				}
				configs = new EGLConfig[numConfigs] ;
				egl.eglGetConfigs
					( display, configs, numConfigs, num_config ) ;
			}
			else
			{
				// 条件を満たす EGLConfig を列挙
				configs = new EGLConfig[numConfigs] ;
				egl.eglChooseConfig
					( display, m_attribs_min_es2, configs, numConfigs, num_config ) ;
			}
			if ( EntisGLS.DEBUG )
			{
			     printConfigs( egl, display, configs ) ;
			}

			// 最適なコンフィグを選択
			return chooseConfig( egl, display, configs ) ;
		}

		// 最適なコンフィグを選択
		public EGLConfig chooseConfig
					( EGL10 egl, EGLDisplay display, EGLConfig[] configs )
		{
			EGLConfig	configEqualsRGBA = null ;
			EGLConfig	configMatched = null ;

			for ( int i = 0; i < configs.length; i ++ )
			{
				EGLConfig	config = configs[i] ;
				int r = findConfigAttrib
							( egl, display, config, EGL10.EGL_RED_SIZE, 0 ) ;
				int g = findConfigAttrib
							( egl, display, config, EGL10.EGL_GREEN_SIZE, 0 ) ;
				int b = findConfigAttrib
							( egl, display, config, EGL10.EGL_BLUE_SIZE, 0 ) ;
				int a = findConfigAttrib
							( egl, display, config, EGL10.EGL_ALPHA_SIZE, 0 ) ;
				int d = findConfigAttrib
							( egl, display, config, EGL10.EGL_DEPTH_SIZE, 0 ) ;
				int s = findConfigAttrib
							( egl, display, config, EGL10.EGL_STENCIL_SIZE, 0 ) ;

				// 最低条件を満たさないものは除外する
				if ( (r < m_bitsReqRed) || (g < m_bitsReqGreen)
					|| (b < m_bitsReqBlue) || (a < m_bitsReqAlpha)
					|| (d < m_bitsReqDepth) || (s < m_bitsReqStencil) )
				{
					continue ;
				}
				if ( configMatched == null )
				{
					configMatched = config ;
				}

				// RGBA の条件の一致判定
				if ( (r == m_bitsBestRed) && (g == m_bitsBestGreen)
					&& (b == m_bitsBestBlue) && (a == m_bitsBestAlpha) )
				{
					// RGBA が一致のコンフィグ
					if ( configEqualsRGBA == null )
					{
						configEqualsRGBA = config ;
					}
					if ( (d == m_bitsBestDepth) || (s == m_bitsBestStencil) )
					{
						// すべてが一致のコンフィグ
						EntisGLS.logDebug( "choose EGLConfig;" ) ;
						printConfig( egl, display, config ) ;
						return	config ;
					}
				}
			}
			if ( configEqualsRGBA != null )
			{
				EntisGLS.logDebug( "choose EGLConfig;" ) ;
				printConfig( egl, display, configEqualsRGBA ) ;
				return	configEqualsRGBA ;
			}
			EntisGLS.logDebug( "choose EGLConfig;" ) ;
			printConfig( egl, display, configMatched ) ;
			return	configMatched ;
		}

		// コンフィグ属性値取得
		private int findConfigAttrib
				( EGL10 egl, EGLDisplay display,
					EGLConfig config, int attribute, int defaultValue )
		{
			if ( egl.eglGetConfigAttrib
					( display, config, attribute, m_valueAttrTemp ) )
			{
			    return	m_valueAttrTemp[0];
			}
			return defaultValue ;
		}

		// コンフィグ列挙
        private void printConfigs
				( EGL10 egl, EGLDisplay display, EGLConfig[] configs)
		{
			int	numConfigs = configs.length ;
			EntisGLS.logDebug( String.format( "%d configurations", numConfigs ) ) ;
			for ( int i = 0; i < numConfigs; i++ )
			{
				EntisGLS.logDebug( String.format( "Configuration %d:\n", i ) ) ;
				printConfig( egl, display, configs[i] ) ;
			}
		}

        private void printConfig
				( EGL10 egl, EGLDisplay display, EGLConfig config )
		{
			int[]	attributes =
			{
				EGL10.EGL_BUFFER_SIZE,
				EGL10.EGL_ALPHA_SIZE,
				EGL10.EGL_BLUE_SIZE,
				EGL10.EGL_GREEN_SIZE,
				EGL10.EGL_RED_SIZE,
				EGL10.EGL_DEPTH_SIZE,
				EGL10.EGL_STENCIL_SIZE,
				EGL10.EGL_CONFIG_CAVEAT,
				EGL10.EGL_CONFIG_ID,
				EGL10.EGL_LEVEL,
				EGL10.EGL_MAX_PBUFFER_HEIGHT,
				EGL10.EGL_MAX_PBUFFER_PIXELS,
				EGL10.EGL_MAX_PBUFFER_WIDTH,
				EGL10.EGL_NATIVE_RENDERABLE,
				EGL10.EGL_NATIVE_VISUAL_ID,
				EGL10.EGL_NATIVE_VISUAL_TYPE,
				0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
				EGL10.EGL_SAMPLES,
				EGL10.EGL_SAMPLE_BUFFERS,
				EGL10.EGL_SURFACE_TYPE,
				EGL10.EGL_TRANSPARENT_TYPE,
				EGL10.EGL_TRANSPARENT_RED_VALUE,
				EGL10.EGL_TRANSPARENT_GREEN_VALUE,
				EGL10.EGL_TRANSPARENT_BLUE_VALUE,
				0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
				0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
				0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
				0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
				EGL10.EGL_LUMINANCE_SIZE,
				EGL10.EGL_ALPHA_MASK_SIZE,
				EGL10.EGL_COLOR_BUFFER_TYPE,
				EGL10.EGL_RENDERABLE_TYPE,
				0x3042 // EGL10.EGL_CONFORMANT
			} ;
			String[]	names =
			{
				"EGL_BUFFER_SIZE",
				"EGL_ALPHA_SIZE",
				"EGL_BLUE_SIZE",
				"EGL_GREEN_SIZE",
				"EGL_RED_SIZE",
				"EGL_DEPTH_SIZE",
				"EGL_STENCIL_SIZE",
				"EGL_CONFIG_CAVEAT",
				"EGL_CONFIG_ID",
				"EGL_LEVEL",
				"EGL_MAX_PBUFFER_HEIGHT",
				"EGL_MAX_PBUFFER_PIXELS",
				"EGL_MAX_PBUFFER_WIDTH",
				"EGL_NATIVE_RENDERABLE",
				"EGL_NATIVE_VISUAL_ID",
				"EGL_NATIVE_VISUAL_TYPE",
				"EGL_PRESERVED_RESOURCES",
				"EGL_SAMPLES",
				"EGL_SAMPLE_BUFFERS",
				"EGL_SURFACE_TYPE",
				"EGL_TRANSPARENT_TYPE",
				"EGL_TRANSPARENT_RED_VALUE",
				"EGL_TRANSPARENT_GREEN_VALUE",
				"EGL_TRANSPARENT_BLUE_VALUE",
				"EGL_BIND_TO_TEXTURE_RGB",
				"EGL_BIND_TO_TEXTURE_RGBA",
				"EGL_MIN_SWAP_INTERVAL",
				"EGL_MAX_SWAP_INTERVAL",
				"EGL_LUMINANCE_SIZE",
				"EGL_ALPHA_MASK_SIZE",
				"EGL_COLOR_BUFFER_TYPE",
				"EGL_RENDERABLE_TYPE",
				"EGL_CONFORMANT"
			} ;
			int[]	value = new int[1] ;
			for ( int i = 0; i < attributes.length; i ++ )
			{
				int		attribute = attributes[i] ;
				String	name = names[i] ;
				if ( egl.eglGetConfigAttrib
					( display, config, attribute, value ) )
				{
				    EntisGLS.logDebug( String.format( "  %s: %d\n", name, value[0] ) ) ;
				}
				else
				{
					while ( egl.eglGetError() != EGL10.EGL_SUCCESS ) ;
				}
			}
		}
	}

#endif


	//////////////////////////////////////////////////////////////////////////
	// 構築関数
	//////////////////////////////////////////////////////////////////////////
	public EntisGLSurfaceView( Context context )
	{
		super( context ) ;
//		getHolder().setFormat( PixelFormat.TRANSLUCENT ) ;

		#if	ANDROID_API_LEVEL >= 8
		m_nGLESVersion = 2 ;

		ActivityManager	actMan =
			(ActivityManager) context.getSystemService
									( Context.ACTIVITY_SERVICE ) ;
		if ( actMan != null )
		{
			ConfigurationInfo	cfgInfo = actMan.getDeviceConfigurationInfo() ;
			if ( cfgInfo != null )
			{
				if ( cfgInfo.reqGlEsVersion >= 0x30000 )
				{
					m_nGLESVersion = 3 ;
				}
			}
		}

		// OpenGL ES 2.0 / 3.0 コンテキスト
		setEGLContextFactory( new ContextFactory( this, m_nGLESVersion ) ) ;

		// RGB444 以上を検索／推奨 RGBA8888 D16
		setEGLConfigChooser
			( new ConfigChooser( 4, 4, 4, 0, 8, 0,
								 8, 8, 8, 8, 16, 0 ) ) ;
		#endif

		setFocusable( true ) ;
		setFocusableInTouchMode( true ) ;
		setRenderer( this ) ;
		setRenderMode( RENDERMODE_WHEN_DIRTY ) ;
	}


	//////////////////////////////////////////////////////////////////////////
	// コンソール・ビュー
	//////////////////////////////////////////////////////////////////////////
	public synchronized void printConsole( String text )
	{
		if ( m_viewConsole == null )
		{
			m_viewConsole = new ConsoleView() ;
			m_viewConsole.initialize() ;
			m_viewConsole.onSurfaceChanged( m_widthView, m_heightView ) ;
		}
		int	iLine = 0 ;
		while ( iLine < text.length() )
		{
			int	iNextLine = text.indexOf( '\n', iLine ) ;
			if ( iNextLine < 0 )
			{
				break ;
			}
			m_viewConsole.printLine
				( text.substring( iLine, iNextLine + 1 ) ) ;
			iLine = iNextLine + 1 ;
		}
		m_viewConsole.printLine( text.substring( iLine ) ) ;
	}
	public synchronized String getConsoleCurrentLine()
	{
		if ( m_viewConsole != null )
		{
			return	m_viewConsole.getCurrentLine() ;
		}
		return	null ;
	}
	public synchronized String getConsoleLastLine()
	{
		if ( m_viewConsole != null )
		{
			return	m_viewConsole.getLastLine() ;
		}
		return	null ;
	}


	//////////////////////////////////////////////////////////////////////////
	// ビュー
	//////////////////////////////////////////////////////////////////////////
	public synchronized void addView( ViewInterface view )
	{
		view.onSurfaceChanged( m_widthView, m_heightView ) ;
		m_vecViews.add( view ) ;
		m_flagNewMenu = true ;
	}
	public synchronized void detachView( ViewInterface view )
	{
		for ( int i = 0; i < m_vecViews.size(); i ++ )
		{
			if ( m_vecViews.get( i ) == view )
			{
				m_vecViews.remove( i -- ) ;
			}
		}
		m_flagNewMenu = true ;
	}


	//////////////////////////////////////////////////////////////////////////
	// タッチイベント
	//////////////////////////////////////////////////////////////////////////
#if	ANDROID_API_LEVEL >= 5
	protected static class	TouchPoint
	{
		public double	x ;
		public double	y ;
		public int		id ;
		public int		act ;
		public int		index ;
	} ;
	protected Vector<TouchPoint>	m_vecTouchPoints = new Vector<TouchPoint>() ;
#endif

	@Override
	public boolean onTouchEvent( MotionEvent ev )
	{
#if	ANDROID_API_LEVEL >= 5
		int				action = ev.getAction() ;
		int				actMasked = action & MotionEvent.ACTION_MASK ;
		int				actIndex = (action & 0xff00 /*MotionEvent.ACTION_POINTER_INDEX_MASK*/)
												>> 8 /*MotionEvent.ACTION_POINTER_INDEX_SHIFT*/ ;
		int				count = ev.getPointerCount() ;
		int				i, j, nPoints ;
		TouchPoint[]	tpPoints = null ;
		synchronized( this )
		{
			nPoints = m_vecTouchPoints.size() ;
			for ( i = 0; i < nPoints; i ++ )
			{
				TouchPoint	tp = m_vecTouchPoints.get( i ) ;
				if ( tp != null )
				{
					tp.act = MotionEvent.ACTION_UP ;
					tp.index = -1 ;
				}
				else
				{
					m_vecTouchPoints.remove( i -- ) ;
					nPoints -- ;
				}
			}
			//
			// 移動位置の対応
			//
			if ( actMasked != MotionEvent.ACTION_CANCEL )
			{
				for ( i = 0; i < count; i ++ )
				{
					int			id = ev.getPointerId(i) ;
					TouchPoint	tp = null ;
					for ( j = 0; j < nPoints; j ++ )
					{
						tp = m_vecTouchPoints.get(j) ;
						if ( tp.id == id )
						{
							break ;
						}
					}
					if ( (tp != null) && (tp.id == id) )
					{
						tp.x = ev.getX(i) ;
						tp.y = ev.getY(i) ;
						tp.act = MotionEvent.ACTION_MOVE ;
						tp.index = i ;
						//
						if ( ((i == 0)
								&& (actMasked == MotionEvent.ACTION_UP))
							|| ((i == actIndex)
								&& (actMasked == MotionEvent.ACTION_POINTER_UP)) )
						{
							tp.act = MotionEvent.ACTION_UP ;
						}
					}
					else
					{
						tp = new TouchPoint() ;
						tp.x = ev.getX(i) ;
						tp.y = ev.getY(i) ;
						tp.id = id ;
						tp.act = MotionEvent.ACTION_DOWN ;
						tp.index = i ;
						m_vecTouchPoints.add( tp ) ;
						nPoints = m_vecTouchPoints.size() ;
					}
				}
			}
			//
			// イベントリスト生成
			//
			nPoints = m_vecTouchPoints.size() ;
			tpPoints = new TouchPoint[nPoints] ;
			for ( i = 0; i < nPoints; i ++ )
			{
				tpPoints[i] = m_vecTouchPoints.get( i ) ;
			}
			for ( i = 0; i < m_vecTouchPoints.size(); i ++ )
			{
				TouchPoint	tp = m_vecTouchPoints.get( i ) ;
				if ( (tp.index < 0)
					|| (tp.act == MotionEvent.ACTION_UP) )
				{
					m_vecTouchPoints.remove( i -- ) ;
				}
			}
		}
		for ( i = 0; i < tpPoints.length; i ++ )
		{
			TouchPoint	tp = tpPoints[i] ;
			switch( tp.act )
			{
			case	MotionEvent.ACTION_DOWN:
				onTouchedDown( tp.x, tp.y, tp.id ) ;
				break ;

			case	MotionEvent.ACTION_UP:
				onTouchedUp( tp.x, tp.y, tp.id ) ;
				break ;

			case	MotionEvent.ACTION_MOVE:
				onTouchMoved( tp.x, tp.y, tp.id ) ;
				break ;
			}
		}
#else
		int	action = ev.getAction() ;
		int	id = 0 ;
		double	x = ev.getX() ;
		double	y = ev.getY() ;
		//
		switch( action & 0x00ff )
		{
		case	MotionEvent.ACTION_DOWN:
			return	onTouchedDown( x, y, id ) ;

		case	MotionEvent.ACTION_UP:
			return	onTouchedUp( x, y, id ) ;

		case	MotionEvent.ACTION_MOVE:
			return	onTouchMoved( x, y, id ) ;
		}
#endif
		return	true ;
	}
	public boolean onTouchedDown( double x, double y, int id )
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onTouchedDown( x, y, id ) )
				{
					break ;
				}
			}
		}
		return	true ;
	}
	public boolean onTouchedUp( double x, double y, int id )
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onTouchedUp( x, y, id ) )
				{
					break ;
				}
			}
		}
		return	true ;
	}
	public boolean onTouchMoved( double x, double y, int id )
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onTouchMoved( x, y, id ) )
				{
					break ;
				}
			}
		}
		return	true ;
	}

	//////////////////////////////////////////////////////////////////////////
	// キー入力
	//////////////////////////////////////////////////////////////////////////
	@Override
	public synchronized boolean onKeyDown( int keyCode, KeyEvent event )
	{
		boolean	fProcessed = onKeyDown( keyCode ) ;
		//
		int	codeChar = event.getUnicodeChar( event.getMetaState() ) ;
		if ( codeChar == 0 )
		{
			return	fProcessed ;
		}
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onChar( codeChar ) )
				{
					return	true ;
				}
			}
		}
		return	fProcessed ;
	}

	@Override
	public synchronized boolean onKeyUp( int keyCode, KeyEvent event )
	{
		return	onKeyUp( keyCode ) ;
	}

	@Override
	public boolean dispatchKeyEvent( KeyEvent event )
	{
		if ( event.getAction() == KeyEvent.ACTION_MULTIPLE )
		{
			String	sInput = event.getCharacters() ;
			if ( sInput == null )
			{
				return	false ;
			}
			char[]	chrInputs = sInput.toCharArray() ;
			if ( chrInputs == null )
			{
				return	false ;
			}
			final int	countViews = m_vecViews.size() ;
			int	j = 0 ;
			for ( int i = 0; i < countViews; i ++ )
			{
				ViewInterface	view = m_vecViews.get( i ) ;
				if ( view != null )
				{
					while ( j < chrInputs.length )
					{
						view.onChar( chrInputs[j ++] ) ;
					}
					break ;
				}
			}
			return	true ;
		}
		return	super.dispatchKeyEvent( event ) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 汎用モーションイベント
	//////////////////////////////////////////////////////////////////////////

#if	ANDROID_API_LEVEL >= 12
	@Override
#endif
	public boolean onGenericMotionEvent( MotionEvent ev )
	{
	#if	ANDROID_API_LEVEL >= 12
		int	sourceClass = ev.getSource() & InputDevice.SOURCE_CLASS_MASK ;
		if ( sourceClass == InputDevice.SOURCE_CLASS_JOYSTICK )
	#else
		boolean	flagGamePad = false ;
		try
		{
			Class<?>	clsEvent = ev.getClass() ;
			Method		mthodGetSource =
							clsEvent.getMethod( "getSource", (Class[]) null ) ;
			Object		ret = mthodGetSource.invoke( ev, (Object[]) null ) ;
			if ( ret instanceof Number )
			{
				flagGamePad = ((((Number) ret).intValue() & 0xFF) == 0x00000010) ;
			}
		}
		catch ( Throwable e )
		{
		}
		if ( flagGamePad )
	#endif
		{
		#if	ANDROID_API_LEVEL >= 12
			float	xAxis = ev.getAxisValue( MotionEvent.AXIS_X ) ;
			float	yAxis = ev.getAxisValue( MotionEvent.AXIS_Y ) ;
			float	zAxis = ev.getAxisValue( MotionEvent.AXIS_Z ) ;
			float	rzAxis = ev.getAxisValue( MotionEvent.AXIS_RZ ) ;
		#else
			float	xAxis = 0, yAxis = 0, zAxis = 0, rzAxis = 0 ;
			try
			{
				Class<?>	clsEvent = ev.getClass() ;
				Method	mthodGetAxisValue =
							clsEvent.getMethod
								( "getAxisValue", new Class[]{ int.class } ) ;
				Object	ret = mthodGetAxisValue.invoke
							( ev, new Object[]{ new Integer(0x00000000) } ) ;
				if ( ret instanceof Number )
				{
					xAxis = ((Number) ret).floatValue() ;
				}
				ret = mthodGetAxisValue.invoke
							( ev, new Object[]{ new Integer(0x00000001) } ) ;
				if ( ret instanceof Number )
				{
					yAxis = ((Number) ret).floatValue() ;
				}
				ret = mthodGetAxisValue.invoke
							( ev, new Object[]{ new Integer(0x0000000B) } ) ;
				if ( ret instanceof Number )
				{
					zAxis = ((Number) ret).floatValue() ;
				}
				ret = mthodGetAxisValue.invoke
							( ev, new Object[]{ new Integer(0x0000000E) } ) ;
				if ( ret instanceof Number )
				{
					rzAxis = ((Number) ret).floatValue() ;
				}
			}
			catch ( Throwable e )
			{
			}
		#endif
			final int	countViews = m_vecViews.size() ;
			for ( int i = 0; i < countViews; i ++ )
			{
				ViewInterface	view = m_vecViews.get( i ) ;
				if ( view != null )
				{
					if ( view.onJoystickAxis( xAxis, yAxis, zAxis, rzAxis ) )
					{
						return	true ;
					}
				}
			}
			return	true ;
		}
		return	false ;
	}

	//////////////////////////////////////////////////////////////////////////
	// ビュー描画
	//////////////////////////////////////////////////////////////////////////
	public synchronized void onDrawFrame( GL10 gl )
	{
		m_gl = gl ;
		//
		Thread	threadCurrent = Thread.currentThread() ;
		if ( threadCurrent != null )
		{
			m_idGLThread = threadCurrent.getId() ;
		}
		//
		// 2D 描画用座標系設定
		//
		gl.glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ) ;
		gl.glClear( GL10.GL_COLOR_BUFFER_BIT ) ;
		//
		// 描画
		//
		final int	countViews = m_vecViews.size() ;
		for ( int i = countViews - 1; i >= 0; i -- )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.draw( gl ) ;
			}
		}
		if ( m_viewConsole != null )
		{
			m_viewConsole.draw( gl ) ;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// サーフェス変更
	//////////////////////////////////////////////////////////////////////////
	public void onSurfaceChanged( GL10 gl, int width, int height )
	{
		EntisGLS.logInfo( "onSurfaceChanged " + width + "x" + height ) ;
		m_gl = gl ;
		m_widthView = width ;
		m_heightView = height ;
		//
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.onSurfaceChanged( width, height ) ;
			}
		}
		if ( m_viewConsole != null )
		{
			m_viewConsole.onSurfaceChanged( width, height ) ;
		}
		//
		EntisGLActivity	activity = EntisGLS.getActivity() ;
		activity.notifySurfaceCreated() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// サーフェスが生成された
	//////////////////////////////////////////////////////////////////////////
	public void onSurfaceCreated( GL10 gl, EGLConfig config )
	{
		EntisGLS.logInfo( "onSurfaceCreated" ) ;
		m_gl = gl ;
		//
		try
		{
			requestFocus() ;
		}
		catch( Throwable e )
		{
			EntisGLS.logDebug( "exception at requestFocus" ) ;
		}
		#if	ANDROID_API_LEVEL >= 8
		if ( (m_display != null)
			&& (m_context != null)
			&& (m_ctxNonRender == null) )
		{
			int	EGL_CONTEXT_CLIENT_VERSION = 0x3098 ;
			int[] attrib_list =
			{
				EGL_CONTEXT_CLIENT_VERSION, m_nGLESVersion,
				EGL10.EGL_NONE
			} ;
			m_ctxNonRender =
				m_egl.eglCreateContext
					( m_display, config, m_context, attrib_list ) ;
			if( m_ctxNonRender != null )
			{
			    int pbufferAttribs[] =
				{
					EGL10.EGL_WIDTH, 1,
					EGL10.EGL_HEIGHT, 1,
					EGL10.EGL_NONE
				} ; 
			    m_eglSurface2nd =
					m_egl.eglCreatePbufferSurface
						( m_display, config, pbufferAttribs ) ;
				if ( m_eglSurface2nd != null )
				{
					EntisGLS.logInfo( "success eglCreatePbufferSurface" ) ;

					m_thread = new Thread( this ) ;
					m_thread.start() ;
				}
			}
		}
		#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// ビュー開始処理（前面移動時）
	//////////////////////////////////////////////////////////////////////////
	public void onBeginView()
	{
		m_flagActive = true ;

		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.onSystemEvent( ViewInterface.SYS_EVENT_RESUME ) ;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ビュー終了処理（背面移動時）
	//////////////////////////////////////////////////////////////////////////
	public void onEndView()
	{
		m_flagActive = false ;

		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.onSystemEvent( ViewInterface.SYS_EVENT_PAUSE ) ;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// アプリケーション終了時処理
	//////////////////////////////////////////////////////////////////////////
	public void onDestroy()
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.onSystemEvent( ViewInterface.SYS_EVENT_DESTROY ) ;
			}
		}
		#if	ANDROID_API_LEVEL >= 8
		if ( m_thread != null )
		{
			synchronized( this )
			{
				m_flagQuitNoRender = true ;
				notifyAll() ;
			}
			while ( m_thread != null )
			{
				synchronized( this )
				{
					try
					{
						wait( 100 ) ;
					}
					catch ( Throwable e )
					{
					}
				}
			}
		}
		if ( m_ctxNonRender != null )
		{
			m_egl.eglDestroyContext( m_display, m_ctxNonRender ) ;
			m_ctxNonRender = null ;
		}
		#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// アクティブ状態取得
	//////////////////////////////////////////////////////////////////////////
	public final boolean isViewActive()
	{
		return	m_flagActive ;
	}
	public final boolean isViewActive( ViewInterface view )
	{
		return	m_flagActive && (m_vecViews.get(0) == view) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// ハードウェアボタン処理実装
	//////////////////////////////////////////////////////////////////////////
	public boolean onSystemKeyDown( int key )
	{
		/*
		if ( onKeyDown( key ) )
		{
			return	true ;
		}
		if ( key == KeyEvent.KEYCODE_BACK )
		{
			EntisGLS.getActivity().finish() ;
			return	true ;
		}
		*/
		return	false ;
	}
	public boolean onSystemKeyUp( int key )
	{
		/*
		if ( onKeyUp( key ) )
		{
			return	true ;
		}
		if ( key == KeyEvent.KEYCODE_BACK )
		{
			return	true ;
		}
		*/
		return	false ;
	}
	public synchronized boolean onKeyDown( int key )
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onKeyDown( key ) )
				{
					return	true ;
				}
			}
		}
		return	false ;
	}
	public synchronized boolean onKeyUp( int key )
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				if ( view.onKeyUp( key ) )
				{
					return	true ;
				}
			}
		}
		return	false ;
	}

	//////////////////////////////////////////////////////////////////////////
	// メニュー
	//////////////////////////////////////////////////////////////////////////
	protected boolean	m_flagNewMenu = false ;

	public boolean onPrepareOptionsMenu( Menu menu )
	{
		ViewInterface	view = m_vecViews.get(0) ;
		if ( view != null )
		{
			boolean	r = view.prepareMenu( menu, m_flagNewMenu ) ;
			m_flagNewMenu = false ;
			return	r ;
		}
		return	false ;
	}
	public boolean onMenuItemSelected( MenuItem item )
	{
		ViewInterface	view = m_vecViews.get(0) ;
		if ( view != null )
		{
			return	view.onMenuCommand( item. getItemId() ) ;
		}
		return	false ;
	}

	//////////////////////////////////////////////////////////////////////////
	// コンテキストメニュー
	//////////////////////////////////////////////////////////////////////////
	protected MenuData	m_menuPopup = null ;

	@Override
	protected void onCreateContextMenu( ContextMenu menu )
	{
		if ( m_menuPopup != null )
		{
			m_menuPopup.createMenu( menu ) ;
			m_menuPopup = null ;
		}
	}

	// メニュー表示
	public void showPopupMenu( MenuData menu )
	{
		m_menuPopup = menu ;
		if ( menu != null )
		{
			EntisGLS.procedureOnUIThread
				( new ShowContextMenuRunnable(), false ) ;
		}
	}
	protected class	ShowContextMenuRunnable	implements Runnable
	{
		public void run()
		{
			showContextMenu() ;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// タイマ処理
	//////////////////////////////////////////////////////////////////////////
	public synchronized void onTimer()
	{
		final int	countViews = m_vecViews.size() ;
		for ( int i = 0; i < countViews; i ++ )
		{
			ViewInterface	view = m_vecViews.get( i ) ;
			if ( view != null )
			{
				view.onTimer() ;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// GL10 取得
	//////////////////////////////////////////////////////////////////////////
	public final GL10 getGL()
	{
		return	m_gl ;
	}

	//////////////////////////////////////////////////////////////////////////
	// レンダリングスレッドで実行する
	//////////////////////////////////////////////////////////////////////////
	public boolean procedureOnRenderingThread( Runnable r )
	{
		if ( isGLThread() )
		{
			r.run() ;
			return	true ;
		}
		else
		{
			SyncRunnable	sr = new SyncRunnable( r ) ;
			queueEvent( sr ) ;
			return	sr.waitDone() ;
		}
	}
	public boolean procedureOnRenderingThread
				( Runnable r, boolean fSync, boolean fDelay )
	{
		if ( !fDelay && isGLThread() )
		{
			r.run() ;
			return	true ;
		}
		else if ( fSync )
		{
			return	procedureOnRenderingThread( r ) ;
		}
		else
		{
			queueEvent( r ) ;
			return	true ;
		}
	}
	// GLスレッド判定
	public final boolean isGLThread()
	{
		Thread	threadCurrent = Thread.currentThread() ;
		if ( threadCurrent != null )
		{
			if ( m_idGLThread == threadCurrent.getId() )
			{
				return	true ;
			}
		}
		return	false ;
	}


	//////////////////////////////////////////////////////////////////////////
	// 非同期実行用スレッド
	//////////////////////////////////////////////////////////////////////////

	final public void run()
	{
#if	ANDROID_API_LEVEL >= 8
		m_egl.eglMakeCurrent
			( m_display, m_eglSurface2nd, m_eglSurface2nd, m_ctxNonRender ) ;
#endif

		while ( !m_flagQuitNoRender )
		{
			Runnable	r = null ;
			synchronized( m_queNonRender )
			{
				try
				{
					m_queNonRender.wait( 1000 ) ;
				}
				catch ( Throwable e )
				{
				}
				if ( m_queNonRender.size() > 0 )
				{
					r = m_queNonRender.remove( 0 ) ;
				}
			}
			if ( r != null )
			{
				r.run() ;
			}
		}
#if	ANDROID_API_LEVEL >= 8
		synchronized( this )
		{
			m_thread = null ;
			notifyAll() ;
		}
#endif
	}

	public boolean procedureAsyncNoRenderingThread( Runnable r )
	{
#if	ANDROID_API_LEVEL >= 8
		if ( m_thread == null )
		{
			queueEvent( r ) ;
			return	true ;
		}
		synchronized( m_queNonRender )
		{
			m_queNonRender.add( r ) ;
			m_queNonRender.notifyAll() ;
			return	true ;
		}
#else
		queueEvent( r ) ;
		return	true ;
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// 一時的に非表示状態にする
	//////////////////////////////////////////////////////////////////////////
	public void hideView()
	{
		synchronized( this )
		{
			m_countHideView ++ ;
		}
		setVisibility( INVISIBLE ) ;
	}
	public void showView()
	{
		boolean	fShow = false ;
		synchronized( this )
		{
			m_countHideView -- ;
			fShow = (m_countHideView <= 0) ;
		}
		if ( fShow )
		{
			setVisibility( VISIBLE ) ;
		}
	}

}

