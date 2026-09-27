package com.entis.android.entisgls4 ;

import java.util.Calendar ;

import android.app.AlarmManager ;
import android.app.PendingIntent ;
import android.app.Service ;
import android.content.BroadcastReceiver ;
import android.content.ComponentName ;
import android.content.ServiceConnection ;
import android.content.Context ;
import android.content.Intent ;
import android.content.IntentFilter ;
import android.os.Build ;
import android.os.Binder ;
import android.os.IBinder ;
import android.os.Parcel ;
import android.os.RemoteException ;


public class EntisService extends Service
{
	// サービスインスタンス
	//////////////////////////////////////////////////////////////////////////

	public static EntisService	m_service = null ;

	public static Context getDefaultContext()
	{
		Context	context = EntisGLS.getActivity() ;
		if ( context == null )
		{
			context = m_service ;
		}
		return	context ;
	}


	// サービスクラス
	//////////////////////////////////////////////////////////////////////////

	protected static Class		m_defServiceClass = null ;

	public static void setDefaultServiceClass( Class clsService )
	{
		m_defServiceClass = clsService ;
	}

	// バインダ
	//////////////////////////////////////////////////////////////////////////
    protected final IBinder m_binder = new Binder()
	{
		@Override
		protected boolean onTransact
			( int code, Parcel data, Parcel reply, int flags ) throws RemoteException
		{
			return	super.onTransact( code, data, reply, flags ) ;
		}
	};

	@Override
	public IBinder onBind( Intent intent )
	{
		return m_binder ;
	}

	// サービス生成時
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void onCreate()
	{
		EntisGLS.logDebug( "Service.onCreate" ) ;
		super.onCreate() ;
		//
		m_service = this ;
		EntisGLS.setContext( this ) ;
		//
		Thread	thread = new Thread
			( new Runnable()
				{
					@Override
					public void run()
					{
						if ( !EntisGLS.isLoadedNativeLibrary() )
						{
							EntisGLS.loadNativeLibrary() ;
						}
						onNativeServiceCreate() ;
					}
				} ) ;
		thread.start() ;
	}

	// サービス終了時
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void onDestroy()
	{
		EntisGLS.logDebug( "Service.onDestroy" ) ;
		super.onDestroy() ;
		onNativeServiceDestroy() ;
		m_service = null ;
		shutdownService( this ) ;
		EntisGLS.setContext( null ) ;
	}

	// サービスアクション
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void onStart( Intent intent, int startId )
	{
		onStartService( intent, startId ) ;
	}

	@Override
	public int onStartCommand( Intent intent, int flags, int startId )
	{
		onStartService( intent, startId ) ;
		return	START_STICKY ;
	}

	protected class	StartServiceRunnable	implements Runnable
	{
		protected String	m_sAction ;

		StartServiceRunnable( String sAction )
		{
			m_sAction = sAction ;
		}

		@Override
		public void run()
		{
			onNativeServiceAction( m_sAction ) ;
		}
	}

	public void onStartService( Intent intent, int startId )
	{
		String	sAction = intent.getStringExtra( "ACTION" ) ;
		if ( sAction != null )
		{
			if ( "start".equals( sAction ) )
			{
	//			onNativeServiceStart( sAction ) ;
			}
			else
			{
				Thread	thread = new Thread
					( new StartServiceRunnable
							( intent.getStringExtra( "SERVICE" ) ) ) ;
				thread.start() ;
			}
		}
	}

	protected native void onNativeServiceCreate() ;
	protected native void onNativeServiceDestroy() ;
	protected native void onNativeServiceStart() ;
	protected native void onNativeServiceAction( String sAction ) ;


	// アラームサービス登録
	//////////////////////////////////////////////////////////////////////////

	public static void scheduleService
		( Context context, long msecUTC, String sAction )
	{
		Class	clsService = m_defServiceClass ;
		if ( clsService == null )
		{
			clsService = EntisService.class ;
		}
		Intent intent = new Intent( context, clsService ) ;
		intent.putExtra( "ACTION", "schedule" ) ;
		intent.putExtra( "SERVICE", sAction ) ;
		//
		PendingIntent piService =
//			PendingIntent.getService( context, 0, intent, 0 ) ;
			PendingIntent.getBroadcast
				( context, 0, intent, PendingIntent.FLAG_IMMUTABLE ) ;
		//
		AlarmManager am =
			(AlarmManager) context.getSystemService( Context.ALARM_SERVICE ) ;
		//
		if ( Build.VERSION.SDK_INT < 19 )
		{
			am.set( AlarmManager.RTC_WAKEUP, msecUTC, piService ) ;
		}
		/*
		else if ( Build.VERSION.SDK_INT < 31 )
		{
			am.setExact( AlarmManager.RTC_WAKEUP, msecUTC, piService ) ;
		}
		 */
		else
		{
			am.setInexactRepeating
				( AlarmManager.RTC_WAKEUP, msecUTC,
					AlarmManager.INTERVAL_HOUR, piService ) ;
		}
	}

	public static void scheduleService
		( Context context,
			int year, int month, int day,
			int hour, int minute, int second, String sAction )
	{
		long	msecDelta =
					Calendar.getInstance().getTimeInMillis()
									- System.currentTimeMillis() ;
		Calendar	calendar = Calendar.getInstance() ;
		calendar.set( Calendar.YEAR, year ) ;
		calendar.set( Calendar.MONTH, month - 1 ) ;
		calendar.set( Calendar.DAY_OF_MONTH, day ) ;
		calendar.set( Calendar.HOUR_OF_DAY, hour ) ;
		calendar.set( Calendar.MINUTE, minute ) ;
		calendar.set( Calendar.SECOND, second ) ;
		//
		scheduleService
			( context, calendar.getTimeInMillis() - msecDelta, sAction ) ;
	}

	public static boolean scheduleService
		( int year, int month, int day,
			int hour, int minute, int second, String sAction )
	{
		Context	context = getDefaultContext() ;
		if ( context == null )
		{
			return	false ;
		}
		scheduleService
			( context, year, month, day, hour, minute, second, sAction ) ;
		return	true ;
	}


	// アラームサービス解除
	//////////////////////////////////////////////////////////////////////////

	public static void cancelSchedule( Context context )
	{
		Class	clsService = m_defServiceClass ;
		if ( clsService == null )
		{
			clsService = EntisService.class ;
		}
		Intent intent = new Intent( context, clsService ) ;
		PendingIntent pi =
			PendingIntent.getService
				( context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT ) ;
		AlarmManager am =
			(AlarmManager) context.getSystemService( Context.ALARM_SERVICE ) ;
		am.cancel( pi ) ;
	}
	public static void cancelSchedule()
	{
		Context	context = getDefaultContext() ;
		if ( context == null )
		{
			return ;
		}
		cancelSchedule( context ) ;
	}

	public static final String	ACTION = "Entis service action" ;


	// サービス開始
	//////////////////////////////////////////////////////////////////////////

	public static void startupService( Context context )
	{
		if ( m_service != null )
		{
			EntisGLS.logDebug( "already started service" ) ;
		}
		Class	clsService = m_defServiceClass ;
		if ( clsService == null )
		{
			clsService = EntisService.class ;
		}
		Intent intent = new Intent( context, clsService ) ;
		intent.putExtra( "ACTION", "start" ) ;
		context.startService( intent ) ;
	}
	public static void startupService()
	{
		startupService( getDefaultContext() ) ;
	}


	// サービス終了
	//////////////////////////////////////////////////////////////////////////

	public static void shutdownService( Context context )
	{
		cancelSchedule() ;
		//
		if ( m_service != null )
		{
			m_service.stopSelf() ;
			m_service = null ;
		}
	}
	public static void shutdownService()
	{
		shutdownService( getDefaultContext() ) ;
	}
}


