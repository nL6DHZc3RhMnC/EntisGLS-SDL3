package com.entis.android.entisgls4 ;

import android.content.BroadcastReceiver ;
import android.content.Context ;
import android.content.Intent ;
import android.util.Log ;


public class EntisServiceBootReceiver extends BroadcastReceiver
{
	// ブロードキャスト・インテント検知時
	@Override
	public void onReceive( final Context context, Intent intent )
	{
		// 端末起動時？
		if ( Intent.ACTION_BOOT_COMPLETED.equals( intent.getAction() ) )
		{
			Thread	thread = new Thread
				( new Runnable(){
					@Override
					public void run()
					{
					    onDeviceBoot( context ) ;
					}
				} ) ;
			thread.start() ;
		}
	}

	// デバイス起動時
	protected void onDeviceBoot( Context context )
	{
		try
		{
			EntisGLS.setContext( context ) ;
			//
			if ( !EntisGLS.isLoadedNativeLibrary() )
			{
				EntisGLS.loadNativeLibrary() ;
			}
			onNativeDeviceBoot() ;
			EntisService.startupService( context ) ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at onDeviceBoot" ) ;
			EntisGLS.logError( e.getMessage() ) ;
		}
	}
	protected native void onNativeDeviceBoot() ;

}

