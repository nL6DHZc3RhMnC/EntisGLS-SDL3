package com.entis.android.entisgls4 ;

import java.util.List ;
import android.content.Context ;
import android.hardware.Sensor ;
import android.hardware.SensorEvent ;
import android.hardware.SensorEventListener ;
import android.hardware.SensorManager ;


public class PostureSensor implements SensorEventListener
{
	public static final int		featureAccelerometer	= 0x0001 ;
	public static final int		featureGyroscope		= 0x0002 ;
	public static final int		featureCompass			= 0x0004 ;

	protected SensorManager	m_manager = null ;
	protected Sensor		m_ssAccel = null ;
	protected Sensor		m_ssGyro = null ;
	protected Sensor		m_ssCompass = null ;
	protected float[]		m_vAccel = new float[3] ;
	protected float[]		m_vGyro = new float[3] ;
	protected float[]		m_vCompass = new float[3] ;
	protected int			m_maskPolled = 0 ;

	// 終了
	//////////////////////////////////////////////////////////////////////////
	public void finalize()
	{
		if ( m_manager != null )
		{
			if ( m_ssAccel != null )
			{
				m_manager.unregisterListener( this, m_ssAccel ) ;
				m_ssAccel = null ;
			}
			if ( m_ssGyro != null )
			{
				m_manager.unregisterListener( this, m_ssGyro ) ;
				m_ssGyro = null ;
			}
			if ( m_ssCompass != null )
			{
				m_manager.unregisterListener( this, m_ssCompass ) ;
				m_ssCompass = null ;
			}
		}
	}

	// 準備
	//////////////////////////////////////////////////////////////////////////
	public boolean prepareSensor( int nFlags )
	{
		if ( nFlags == 0 )
		{
			nFlags = featureAccelerometer
						| featureGyroscope | featureCompass ;
		}
		Context	context = EntisGLS.getContext() ;
		m_manager = 
			(SensorManager) context.getSystemService( Context.SENSOR_SERVICE ) ;
		if ( m_manager == null )
		{
			return	false ;
		}
		if ( (m_ssAccel == null)
			&& ((nFlags & featureAccelerometer) != 0) )
		{
			List<Sensor> sensors =
				m_manager.getSensorList( Sensor.TYPE_ACCELEROMETER ) ;
			if ( (sensors != null)
				&& (sensors.size() > 0) )
			{
				m_ssAccel = sensors.get(0) ;
				m_manager.registerListener
					( this, m_ssAccel,
						SensorManager.SENSOR_DELAY_GAME ) ;
			}
		}
		if ( (m_ssGyro == null)
			&& ((nFlags & featureGyroscope) != 0) )
		{
			List<Sensor> sensors =
				m_manager.getSensorList( Sensor.TYPE_GYROSCOPE ) ;
			if ( (sensors != null)
				&& (sensors.size() > 0) )
			{
				m_ssGyro = sensors.get(0) ;
				m_manager.registerListener
					( this, m_ssGyro,
						SensorManager.SENSOR_DELAY_GAME ) ;
			}
		}
		if ( (m_ssCompass == null)
			&& ((nFlags & featureCompass) != 0) )
		{
			List<Sensor> sensors =
				m_manager.getSensorList( Sensor.TYPE_MAGNETIC_FIELD ) ;
			if ( (sensors != null)
				&& (sensors.size() > 0) )
			{
				m_ssCompass = sensors.get(0) ;
				m_manager.registerListener
					( this, m_ssCompass,
						SensorManager.SENSOR_DELAY_GAME ) ;
			}
		}
		m_maskPolled = 0 ;
		//
		if ( !EntisGLS.isPrimaryThread() )
		{
			waitSensor( getSensorFeatures(), 100 ) ;
		}
		return	true ;
	}

	// 準備完了センサ取得
	//////////////////////////////////////////////////////////////////////////
	public int getSensorFeatures()
	{
		int	nFlags = 0 ;
		if ( m_ssAccel != null )
		{
			nFlags |= featureAccelerometer ;
		}
		if ( m_ssGyro != null )
		{
			nFlags |= featureGyroscope ;
		}
		if ( m_ssCompass != null )
		{
			nFlags |= featureCompass ;
		}
		return	nFlags ;
	}

	// 加速度値取得
	//////////////////////////////////////////////////////////////////////////
	public void getAccelerometer( float[] vAccel )
	{
		vAccel[0] = m_vAccel[0] ;
		vAccel[1] = m_vAccel[1] ;
		vAccel[2] = m_vAccel[2] ;
	}

	// ジャイロ値取得
	//////////////////////////////////////////////////////////////////////////
	public void getGyroscope( float[] vGyro )
	{
		vGyro[0] = m_vGyro[0] ;
		vGyro[1] = m_vGyro[1] ;
		vGyro[2] = m_vGyro[2] ;
	}

	// 地磁気強度取得
	//////////////////////////////////////////////////////////////////////////
	public void getCompass( float[] vCompass )
	{
		vCompass[0] = m_vCompass[0] ;
		vCompass[1] = m_vCompass[1] ;
		vCompass[2] = m_vCompass[2] ;
	}

	// センサ値の更新待機
	//////////////////////////////////////////////////////////////////////////
	public synchronized boolean waitSensor( int bitsSensor, int timeout )
	{
		if ( (m_maskPolled & bitsSensor) == bitsSensor )
		{
			m_maskPolled &= ~bitsSensor ;
			return	true ;
		}
		try
		{
			wait( timeout ) ;
		}
		catch ( Exception e )
		{
		}
		if ( (m_maskPolled & bitsSensor) == bitsSensor )
		{
			m_maskPolled &= ~bitsSensor ;
			return	true ;
		}
		return	false ;
	}

	// センサが変化した
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void onAccuracyChanged( Sensor sensor, int accuracy )
	{
	}

	// センサ値が変化した
	//////////////////////////////////////////////////////////////////////////
	@Override
	public synchronized void onSensorChanged( SensorEvent event )
	{
		if ( event.sensor.getType() == Sensor.TYPE_ACCELEROMETER )
		{
			m_vAccel[0] = event.values[SensorManager.DATA_X] ;
			m_vAccel[1] = event.values[SensorManager.DATA_Y] ;
			m_vAccel[2] = event.values[SensorManager.DATA_Z] ;
			m_maskPolled |= featureAccelerometer ;
			notifyAll() ;
		}
		else if ( event.sensor.getType() == Sensor.TYPE_GYROSCOPE )
		{
			m_vGyro[0] = event.values[SensorManager.DATA_X] ;
			m_vGyro[1] = event.values[SensorManager.DATA_Y] ;
			m_vGyro[2] = event.values[SensorManager.DATA_Z] ;
			m_maskPolled |= featureGyroscope ;
			notifyAll() ;
		}
		else if ( event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD )
		{
			m_vCompass[0] = event.values[SensorManager.DATA_X] ;
			m_vCompass[1] = event.values[SensorManager.DATA_Y] ;
			m_vCompass[2] = event.values[SensorManager.DATA_Z] ;
			m_maskPolled |= featureCompass ;
			notifyAll() ;
		}
	}

}

