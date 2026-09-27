package com.entis.android.entisgls4 ;

import javax.microedition.khronos.opengles.GL10 ;
import java.nio.ByteBuffer ;
import android.view.* ;


public class VirtualWindow	extends ViewInterface
{
	protected ByteBuffer	m_buf ;
	protected MenuData		m_menu = null ;
	protected boolean		m_flagUpdateMenu = false ;

	// 構築関数
	public VirtualWindow( ByteBuffer buf )
	{
		m_buf = buf ;
	}

	// 画面サイズ変更
	@Override
	public native void onSurfaceChanged( int width, int height ) ;
	// 描画
	@Override
	public native void draw( GL10 gl ) ;
	// タッチ
	@Override
	public native boolean onTouchedDown( double x, double y, int id ) ;
	@Override
	public native boolean onTouchedUp( double x, double y, int id ) ;
	@Override
	public native boolean onTouchMoved( double x, double y, int id ) ;
	// キー入力
	@Override
	public native boolean onKeyDown( int key ) ;
	@Override
	public native boolean onKeyUp( int key ) ;
	@Override
	public native boolean onChar( int code ) ;
	// ジョイスティック
	@Override
	public native boolean onJoystickAxis( float x, float y, float z, float rz ) ;
	// タイマ
	@Override
	public native boolean onTimer() ;
	// システムイベント
	@Override
	public native boolean onSystemEvent( int event ) ;
	// メニューコマンド
	@Override
	public native boolean onMenuCommand( int idItem ) ;

	// メニュー表示準備
	@Override
	public synchronized boolean prepareMenu( Menu menu, boolean fNew )
	{
		if ( m_menu != null )
		{
			if ( fNew || m_flagUpdateMenu || m_menu.isNewMenu() )
			{
				m_menu.createMenu( menu ) ;
				m_flagUpdateMenu = false ;
			}
			return	true ;
		}
		else
		{
			menu.clear() ;
		}
		return	false ;
	}
	// メニュー設定
	public synchronized void setMenu( MenuData menu )
	{
		m_menu = menu ;
		m_flagUpdateMenu = true ;

#if	ANDROID_API_LEVEL >= 11
		EntisGLActivity	activity = EntisGLS.getActivity() ;
		if ( activity != null )
		{
			activity.invalidateOptionsMenu() ;
		}
#endif
	}

}

