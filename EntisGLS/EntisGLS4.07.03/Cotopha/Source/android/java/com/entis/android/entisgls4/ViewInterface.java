package com.entis.android.entisgls4 ;

import javax.microedition.khronos.opengles.GL10 ;
import android.view.* ;


public class ViewInterface
{
	public static final int		SYS_EVENT_PAUSE		= 0x0001 ;
	public static final int		SYS_EVENT_RESUME	= 0x0002 ;
	public static final int		SYS_EVENT_DESTROY	= 0x0003 ;

	// 画面サイズ変更
	public void onSurfaceChanged( int width, int height )
	{
	}
	// 描画
	public void draw( GL10 gl )
	{
	}
	// タッチ
	public boolean onTouchedDown( double x, double y, int id )
	{
		return	false ;
	}
	public boolean onTouchedUp( double x, double y, int id )
	{
		return	false ;
	}
	public boolean onTouchMoved( double x, double y, int id )
	{
		return	false ;
	}
	// キー入力
	public boolean onKeyDown( int key )
	{
		return	false ;
	}
	public boolean onKeyUp( int key )
	{
		return	false ;
	}
	public boolean onChar( int code )
	{
		return	false ;
	}
	// ジョイスティック
	public boolean onJoystickAxis( float x, float y, float z, float rz )
	{
		return	false ;
	}
	// タイマ
	public boolean onTimer()
	{
		return	false ;
	}
	// システムイベント
	public boolean onSystemEvent( int event )
	{
		return	false ;
	}
	// メニューコマンド
	public boolean onMenuCommand( int idItem )
	{
		return	false ;
	}
	// メニュー表示準備
	public boolean prepareMenu( Menu menu, boolean fNew )
	{
		return	false ;
	}
}

