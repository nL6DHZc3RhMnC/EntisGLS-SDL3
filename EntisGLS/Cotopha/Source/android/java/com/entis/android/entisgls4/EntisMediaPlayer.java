package com.entis.android.entisgls4 ;

import android.view.SurfaceView ;
import android.view.SurfaceHolder ;
import android.view.MotionEvent ;
import android.widget.RelativeLayout ;
import android.media.MediaPlayer ;
import android.content.Context;
import android.content.res.AssetFileDescriptor ;
import android.graphics.PixelFormat ;

import java.io.File ;


public class	EntisMediaPlayer
{
	// 再生オブジェクト
	protected MediaPlayer		m_player = null ;

	// メディア長／サンプル単位
	protected long				m_nTotalLength = 0 ;		// [sample]
	protected int				m_nFrequency = 1000 ;		// [sample/sec]

	// 再生パラメータ
	protected boolean			m_flagPlaying = false ;
	protected long				m_countPaused = 0 ;

	protected double			m_fpLeftVol = 1.0 ;
	protected double			m_fpRightVol = 1.0 ;


	// 動画再生サーフェスビュー
	//////////////////////////////////////////////////////////////////////////
	public static class	MovieSurfaceView	extends SurfaceView
											implements SurfaceHolder.Callback
	{
		protected boolean	m_flagCreated = false ;
		protected boolean	m_flagDestroyed = false ;

		// 構築関数
		public MovieSurfaceView( Context context )
		{
			super( context ) ;
			//
			SurfaceHolder holder = getHolder() ;
			if ( holder != null )
			{
				holder.setType( SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS ) ;
				holder.addCallback( this ) ;
			}
		}

		// タッチイベント
		@Override
		public boolean onTouchEvent( MotionEvent ev )
		{
			int	action = ev.getAction() ;
			int	id = 0 ;
			double	x = ev.getX() ;
			double	y = ev.getY() ;
			//
			EntisGLSurfaceView	glsufview = EntisGLS.getMainSurfaceView() ;
			switch( action & 0x00ff )
			{
			case	MotionEvent.ACTION_DOWN:
				return	glsufview.onTouchedDown( x, y, id ) ;

			case	MotionEvent.ACTION_UP:
				return	glsufview.onTouchedUp( x, y, id ) ;

			case	MotionEvent.ACTION_MOVE:
				return	glsufview.onTouchMoved( x, y, id ) ;
			}
			return	true ;
		}

		// サーフェスが変更された
		public void surfaceChanged
			( SurfaceHolder holder, int format, int width, int height )
		{
		}

		// サーフェスが生成された
		public synchronized void surfaceCreated( SurfaceHolder holder )
		{
			m_flagCreated = true ;
			notifyAll() ;
		}

		// サーフェスが破棄された
		public synchronized void surfaceDestroyed( SurfaceHolder holder )
		{
			m_flagDestroyed = true ;
			notifyAll() ;
		}

		// サーフェスの生成同期
		public synchronized boolean waitSurfaceCreated()
		{
			while ( !m_flagCreated && !m_flagDestroyed )
			{
				try
				{
					wait( 100 ) ;
				}
				catch ( Exception e )
				{
					break ;
				}
			}
			return	m_flagCreated && !m_flagDestroyed ;
		}
	}

	// 表示ビュー
	protected MovieSurfaceView	m_view = null ;
	protected RelativeLayout	m_layout = null ;


	// 構築
	//////////////////////////////////////////////////////////////////////////
	public EntisMediaPlayer()
	{
	}

	// プレイヤーを生成する
	//////////////////////////////////////////////////////////////////////////
	public boolean createPlayer
		( String pathFile, SurfaceHolder holder )
	{
		destroyPlayer() ;
		//
		m_player = new MediaPlayer() ;
		try
		{
			m_player.setDataSource( pathFile ) ;
			if ( holder != null )
			{
				m_player.setDisplay( holder ) ;
			}
			m_player.prepare() ;
		}
		catch ( Exception e )
		{
			EntisGLS.logError( "exception at MediaPlayer open file" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			m_player = null ;
			return	false ;
		}
		m_nTotalLength =
			(long) m_player.getDuration() * m_nFrequency / 1000 ;
		return	true ;
	}
	public boolean createPlayerOnAssets
		( String pathOnAssets, SurfaceHolder holder )
	{
		destroyPlayer() ;
		//
		// assets のファイルを探す
		//
		AssetFileDescriptor	afd = null ;
		try
		{
			afd = EntisGLS.getActivity().
						getAssets().openFd( pathOnAssets ) ;
		}
		catch ( Exception e )
		{
			return	false ;
		}
		if ( afd == null )
		{
			return	false ;
		}
		//
		// プレイヤー生成
		//
		m_player = new MediaPlayer() ;
		try
		{
			m_player.setDataSource
				( afd.getFileDescriptor(),
					afd.getStartOffset(), afd.getLength() ) ;
			if ( holder != null )
			{
				m_player.setDisplay( holder ) ;
			}
			m_player.prepare() ;
		}
		catch ( Exception e )
		{
			EntisGLS.logError( "exception at MediaPlayer open file" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			m_player = null ;
			return	false ;
		}
		m_nTotalLength =
			(long) m_player.getDuration() * m_nFrequency / 1000 ;
		return	true ;
	}

	// プレイヤー削除
	//////////////////////////////////////////////////////////////////////////
	public void destroyPlayer()
	{
		if ( isPlaying() )
		{
			stop() ;
		}
		if ( m_player != null )
		{
			m_player.reset() ;
			m_player.release() ;
		}
		m_player = null ;
	}

	// ファイルを開く
	//////////////////////////////////////////////////////////////////////////
	public boolean openMovie( String pathFile, boolean flagAssets )
	{
		close() ;
		//
		// ビュー生成
		//
		EntisGLS.procedureOnUIThread
			( new Runnable() { public void run()
				{
					EntisGLActivity	activity = EntisGLS.getActivity() ;
					activity.getWindow().setFormat( PixelFormat.TRANSLUCENT ) ;
					m_layout = new RelativeLayout( activity ) ;
					m_view = new MovieSurfaceView( activity ) ;
					m_layout.addView( m_view ) ;
					activity.addView( m_layout, 0 ) ;
					m_view.setVisibility( SurfaceView.VISIBLE ) ;
					m_layout.setVisibility( SurfaceView.VISIBLE ) ;
				} } ) ;
		if ( m_view == null )
		{
			return	false ;
		}
		m_view.waitSurfaceCreated() ;
		//
		// ファイルを開く
		//
		EntisGLS.procedureOnUIThread
			( new MovieViewOpener( pathFile, flagAssets ) ) ;
		if ( m_player == null )
		{
			return	false ;
		}
		return	true ;
	}
	public boolean openMovie( String pathFile )
	{
		return	openMovie( pathFile, false ) ;
	}
	public boolean openMovieOnAssets( String pathFile )
	{
		return	openMovie( pathFile, true ) ;
	}
	public boolean openAudio( String pathFile )
	{
		close() ;
		return	createPlayer( pathFile, null ) ;
	}
	public boolean openAudioOnAssets( String pathFile )
	{
		close() ;
		return	createPlayerOnAssets( pathFile, null ) ;
	}
	protected class MovieViewOpener	implements Runnable
	{
		protected String	m_pathFile = null ;
		protected boolean	m_flagAssets = false ;

		public MovieViewOpener( String pathFile, boolean flagAssets )
		{
			m_pathFile = pathFile ;
			m_flagAssets = flagAssets ;
		}

		public void run()
		{
			//
			// メディアファイルを開く
			//
			EntisGLActivity	activity = EntisGLS.getActivity() ;
			boolean	flagSuccess = false ;
			if ( m_flagAssets )
			{
				flagSuccess =
					createPlayerOnAssets( m_pathFile, m_view.getHolder() ) ;
			}
			else
			{
				flagSuccess =
					createPlayer( m_pathFile, m_view.getHolder() ) ;
			}
			if ( !flagSuccess )
			{
				destroyPlayer() ;
				activity.removeView( m_view ) ;
				m_view = null ;
				return ;
			}
			else
			{
				EntisGLS.getMainSurfaceView().hideView() ;
				//
				int	wMovie = getVideoWidth() ;
				int	hMovie = getVideoHeight() ;
				if ( (wMovie != 0) && (hMovie != 0) )
				{
					android.view.Display
						display =
							EntisGLS.getActivity().
								getWindowManager().getDefaultDisplay() ;
					int	wWindow = display.getWidth() ;
					int	hWindow = display.getHeight() ;
					//
					android.view.ViewGroup.LayoutParams
										lp = m_view.getLayoutParams() ;
					if ( wWindow * hMovie <= hWindow * wMovie )
					{
						lp.width = wWindow ;
						lp.height = hMovie * wWindow / wMovie ;
					}
					else
					{
						lp.height = hWindow ;
						lp.width = wMovie * hWindow / hMovie ;
					}
					m_view.setLayoutParams( lp ) ;
					m_layout.setGravity( android.view.Gravity.CENTER ) ;
				}
			}
		}
	}

	// 動画ファイルを閉じる
	//////////////////////////////////////////////////////////////////////////
	public void close()
	{
		stop() ;
		destroyPlayer() ;
		//
		if ( m_view != null )
		{
			EntisGLS.procedureOnUIThread
				( new Runnable() { public void run()
					{
						EntisGLS.getMainSurfaceView().showView() ;
						if ( m_layout != null )
						{
							EntisGLS.getActivity().removeView( m_layout ) ;
							m_layout.removeView( m_view ) ;
							m_layout = null ;
						}
						else
						{
							EntisGLS.getActivity().removeView( m_view ) ;
						}
						m_view = null ;
					} } ) ;
		}
	}

	// 再生開始
	//////////////////////////////////////////////////////////////////////////
	public boolean play()
	{
		if ( m_flagPlaying )
		{
			return	false ;
		}
		if ( m_player == null )
		{
			return	false ;
		}
		try
		{
			m_player.start() ;
			m_flagPlaying = true ;
			m_countPaused = 0 ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at MediaPlayer.play" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		return	true ;
	}

	// 停止
	//////////////////////////////////////////////////////////////////////////
	public boolean stop()
	{
		if ( m_player == null )
		{
			return	false ;
		}
		try
		{
			m_player.stop() ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at MediaPlayer.stop" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		return	true ;
	}

	// 一時停止
	//////////////////////////////////////////////////////////////////////////
	public synchronized boolean pause()
	{
		if ( !m_flagPlaying )
		{
			return	false ;
		}
		if ( m_countPaused == 0 )
		{
			try
			{
				m_player.pause() ;
			}
			catch ( Exception e )
			{
				EntisGLS.logError( "exception at MediaPlayer.pause" ) ;
				EntisGLS.logError( e.getMessage() ) ;
				return	false ;
			}
		}
		m_countPaused ++ ;
		return	true ;
	}

	// 一時停止再開
	//////////////////////////////////////////////////////////////////////////
	public synchronized boolean restart()
	{
		if ( m_countPaused <= 0 )
		{
			return	false ;
		}
		if ( (-- m_countPaused) == 0 )
		{
			try
			{
				m_player.start() ;
			}
			catch ( Exception e )
			{
				EntisGLS.logError( "exception at MediaPlayer.start to restart" ) ;
				EntisGLS.logError( e.getMessage() ) ;
				return	false ;
			}
		}
		return	true ;
	}

	// 音量取得
	//////////////////////////////////////////////////////////////////////////
	public void getVolume( double[] volumes )
	{
		volumes[0] = m_fpLeftVol ;
		volumes[1] = m_fpRightVol ;
	}

	// 音量設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setVolume( double volLeft, double volRight )
	{
		m_fpLeftVol = Math.min( Math.max( volLeft, 0.0 ), 1.0 ) ;
		m_fpRightVol = Math.min( Math.max( volRight, 0.0 ), 1.0 ) ;
		//
		if ( m_player != null )
		{
			m_player.setVolume( (float) m_fpLeftVol, (float) m_fpRightVol ) ;
			return	true ;
		}
		return	false ;
	}

	// 現在の再生位置を取得
	//////////////////////////////////////////////////////////////////////////
	public synchronized long getPlayingPosition()
	{
		if ( m_player != null )
		{
			return	(long) m_player.getCurrentPosition()
										* m_nFrequency / 1000 ;
		}
		return	0 ;
	}

	// （スタティックバッファ）再生開始位置[/bytes]設定
	//////////////////////////////////////////////////////////////////////////
	public boolean seekPosition( long nPos )
	{
		if ( m_player == null )
		{
			return	false ;
		}
		try
		{
			m_player.seekTo( (int) (nPos * 1000 / m_nFrequency) ) ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at MediaPlayer.seekTo" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		return	true ;
	}

	// サンプル周波数を取得する
	//////////////////////////////////////////////////////////////////////////
	public int getSampleFrequency()
	{
		return	m_nFrequency ;
	}

	// メディア全長 [/sample] を取得する
	//////////////////////////////////////////////////////////////////////////
	public long getTotalLength()
	{
		return	m_nTotalLength ;
	}

	// 再生中か？
	//////////////////////////////////////////////////////////////////////////
	public boolean isPlaying()
	{
		if ( m_flagPlaying && (m_player != null) )
		{
			if ( m_player.isPlaying() )
			{
				return	true ;
			}
			m_flagPlaying = false ;
		}
		return	false ;
	}

	// 一時停止中か？
	//////////////////////////////////////////////////////////////////////////
	public synchronized boolean isPaused()
	{
		return	(m_countPaused > 0) ;
	}

	// リピートフラグ設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setLoop( boolean fRepeat )
	{
		if ( m_player != null )
		{
			m_player.setLooping( fRepeat ) ;
		}
		return	false ;
	}

	// ビデオサイズ取得
	//////////////////////////////////////////////////////////////////////////
	public int getVideoWidth()
	{
		if ( m_player != null )
		{
			return	m_player.getVideoWidth() ;
		}
		return	0 ;
	}
	public int getVideoHeight()
	{
		if ( m_player != null )
		{
			return	m_player.getVideoHeight() ;
		}
		return	0 ;
	}

}


