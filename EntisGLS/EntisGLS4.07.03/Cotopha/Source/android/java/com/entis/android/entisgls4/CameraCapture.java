package com.entis.android.entisgls4 ;

import android.app.Activity ;
import android.content.Context ;
import android.util.* ;

#if	ANDROID_API_LEVEL >= 21
import android.hardware.camera2.* ;
#endif


public class	CameraCapture
{
	public static class	Resolution
	{
		public int		iFormat ;
		public int		widthFrame ;
		public int		heightFrame ;
		public double	framesPerSec ;
	}

	public static class	DeviceInfo
	{
		public String		m_strDevId ;
		public String		m_strDevName ;
		public Resolution[]	m_Resolutions ;
	}

#if	ANDROID_API_LEVEL >= 21

	private Activity				m_activity = null ;
	private int						m_glTextureID = 0 ;
	private CameraDevice			m_devCamera = null ;
	private CaptureRequest.Builder	m_builderPreview = null ;
	private CameraCaptureSession	m_sessionPreview = null ;
	private SurfaceTexture			m_surfaceTexture = null ;
	private Size					m_sizeCamera = null ;
	private boolean					m_flagPortrait = false ;
	private boolean					m_flagOpened = false ;
	private boolean					m_signalCameraState = false ;
	private boolean					m_signalCaptureState = false ;

	// カメラ・コールバック
	public class	CameraStateCallback	extends CameraDevice.StateCallback
	{
		private Object	m_objSync = null ;

		CameraStateCallback( Object objSync )
		{
			m_objSync = objSync ;
		}

		@Override
		public void onOpened( CameraDevice camera )
		{
			m_devCamera = camera ;
			createCaptureSession() ;
			setDone() ;
		}

		@Override
		public void onDisconnected( CameraDevice camera )
		{
			camera.close() ;
			m_devCamera = null ;
			setDone() ;
		}

		@Override
		public void onError( CameraDevice camera, int error )
		{
			camera.close() ;
			m_devCamera = null ;
			setDone() ;
		}

		public void setDone()
		{
			synchronized( m_objSync )
			{
				m_signalCameraState = true ;
				m_objSync.notifyAll() ;
			}
		}
	}

	private CameraStateCallback	m_callbackCamera = new CameraStateCallback( this ) ;

	// キャプチャーセッション・コールバック
	class	CaptureStateCallback	extends CameraCaptureSession.StateCallback
	{
		private Object	m_objSync = null ;

		CaptureStateCallback( Object objSync )
		{
			m_objSync = objSync ;
		}

		@Override
		public void onConfigured( CameraCaptureSession session )
		{
			m_sessionPreview = session ;
			updatePreview() ;
			setDone() ;
		}

		@Override
		public void onConfigureFailed( CameraCaptureSession session )
		{
			m_sessionPreview = null ;
			setDone() ;
		}

		public void setDone()
		{
			synchronized( m_objSync )
			{
				m_signalCaptureState = true ;
				m_objSync.notifyAll() ;
			}
		}
	}

	private CaptureStateCallback	m_callbackCapture = new CaptureStateCallback( this ) ;


	// 構築関数
	public CameraCapture( Activity act, int glTexture )
	{
		m_activity = act ;

		boolean	portrait =
				(act.getResources().getConfiguration().orientation
									== Configuration.ORIENTATION_PORTRAIT) ;
		int	orientation = act.getWindowManager().getDefaultDisplay().getRotation() ;
		if ( portrait )
		{
			m_flagPortrait = (orientation == Surface.ROTATION_0)
							|| (orientation == Surface.ROTATION_180) ;
		}
		else
		{
			m_flagPortrait = (orientation == Surface.ROTATION_90)
							|| (orientation == Surface.ROTATION_270) ;
		}
	}

	// GLテクスチャ関連付け
	public void attachGLTexture( int glTexture )
	{
		m_glTextureID = glTexture ;
	}

	// デバイス列挙
	public DeviceInfo[] enumerateCaptureDevices()
	{
		int[]	aLensFacings =
		{
			CameraCharacteristics.LENS_FACING_FONT,
			CameraCharacteristics.LENS_FACING_BACK,
			CameraCharacteristics.LENS_FACING_EXTERNAL
		} ;
		String[]	aFacingNames =
		{
			"FONT", "BACK", "EXTERNAL"
		} ;
		CameraManager	manCamera = null ;
		String[]		aCameraIDs = null ;
		try
		{
			manCamera = (CameraManager)
				m_activity.getSystemService( Context.CAMERA_SERVICE ) ;
			aCameraIDs = manCamera.getCameraIdList() ;
		}
		catch ( Exception e )
		{
			return	null ;
		}
		Vector<DeviceInfo>	vecDevInfos = new Vector<DeviceInfo>() ;
		for ( int i = 0; i < aLensFacings.length; i ++ )
		{
			try
			{
				for ( int j = 0; j < aCameraIDs.length; j ++ )
				{
					CameraCharacteristics
						characteristics =
							manCamera.getCameraCharacteristics( aCameraIDs[j] ) ;
					if ( characteristics.get
							( CameraCharacteristics.LENS_FACING ) == aLensFacings[i] )
					{
						DeviceInfo	devInfo = new DeviceInfo() ;
						devInfo.m_strDevName = aFacingNames[i] ;
						devInfo.m_strDevId = aCameraIDs[j] ;
						//
						StreamConfigurationMap	map =
							characteristics.get
								( CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP ) ;
						Size[]	sizeOut = map.getOutputSizes( SurfaceTexture.class ) ;
						devInfo.m_Resolutions = new Resolution[sizeOut.length] ;
						for ( int k = 0; k < sizeOut.length; k ++ )
						{
							Resolution	res = new Resolution() ;
							devInfo.m_Resolutions[k] = res ;
							//
							res.iFormat = k ;
							res.widthFrame = sizeOut[k].getWidth() ;
							res.heightFrame = sizeOut[k].getHeight() ;
							res.framesPerSec = 30.0 ;
						}
						vecDevInfos.add( devInfo ) ;
					}
				}
			}
			catch ( Exception e )
			{
			}
		}
		return	vecDevInfos.toArray() ;
	}

	// カメラを開く
	public boolean openCamera( DeviceInfo devInfo, int iFormat )
	{
		if ( m_flagOpened
			|| (devInfo == null)
			|| (devInfo.m_Resolutions == null) )
		{
			return	false ;
		}
		m_signalCameraState = false ;
		m_signalCaptureState = false ;
		m_sizeCamera = null ;
		for ( int i = 0; i < devInfo.m_Resolutions.length; i ++ )
		{
			if ( (devInfo.m_Resolutions[i] != null)
				&& (devInfo.m_Resolutions[i].iFormat == iFormat) )
			{
				m_sizeCamera =
					new Size( devInfo.m_Resolutions[i].widthFrame,
								devInfo.m_Resolutions[i].HeightFrame ) ;
				break ;
			}
		}
		if ( m_sizeCamera == null )
		{
			return	false ;
		}
		CameraManager	manCamera = null ;
		try
		{
			manCamera = (CameraManager)
				m_activity.getSystemService( Context.CAMERA_SERVICE ) ;
		}
		catch ( Exception e )
		{
			return	false ;
		}
		HandlerThread	thread = new HandlerThread( "CameraHandler" ) ;
		thread.start() ;
		//
		Handler	handlerCamera = new Handler( thread.getLooper() ) ;
		manCamera.openCamera
			( devInfo.m_strDevId, m_callbackCamera, handlerCamera ) ;
		//
		return	true ;
	}

	protected void createCaptureSession()
	{
		m_surfaceTexture = new SurfaceTexture( m_glTextureID ) ;
		m_surfaceTexture.setDefaultBufferSize
			( m_sizeCamera.getWidth(), m_sizeCamera.getHeight() ) ;
		//
        Surface	surface = new Surface( m_surfaceTexture ) ;
		try
		{
			m_builderPreview =
				m_devCamera.createCaptureRequest( CameraDevice.TEMPLATE_PREVIEW ) ;
			m_flagOpened = true ;
			//
			m_builderPreview.addTarget( surface ) ;
			//
			m_devCamera.createCaptureSession
				( Collections.singletonList( surface ), m_callbackCapture, null ) ;
		}
		catch ( Exception e )
		{
			m_flagOpened = false ;
		}
	}

	protected void updatePreview()
	{
		m_builderPreview.set
			( CaptureRequest.CONTROL_AF_MODE,
				CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE ) ;
		//
		HandlerThread	thread = new HandlerThread( "CameraRepeating" ) ;
		thread.start() ;
		Handler	handlerRepeating = new Handler( thread.getLooper() ) ;
		//
		try
		{
			m_sessionPreview.setRepeatingRequest
				( m_builderPreview.build(), null, handlerRepeating ) ;
		}
		catch ( Exception e )
		{
			m_sessionPreview.close() ;
			m_sessionPreview = null ;
		}
	}

	// 初期化完了を待つ
	public boolean waitInitialized()
	{
		while ( !m_signalCameraState )
		{
			try
			{
				wait( 10 ) ;
			}
			catch ( Exceptioin e )
			{
			}
		}
		if ( !m_flagOpened )
		{
			return	false ;
		}
		while ( !m_signalCaptureState )
		{
			try
			{
				wait( 10 ) ;
			}
			catch ( Exceptioin e )
			{
			}
		}
		return	(m_sessionPreview != null) ;
	}

	// 終了
	public void close()
	{
		if ( m_sessionPreview != null )
		{
			m_sessionPreview.stopRepeating() ;
			m_sessionPreview.close() ;
		}
		if ( m_devCamera != null )
		{
			m_devCamera.close() ;
		}
		m_builderPreview = null ;
		m_surfaceTexture = null ;
		m_devCamera = null ;
		m_sizeCamera = null ;
		m_flagOpened = false ;
		m_signalCameraState = false ;
		m_signalCaptureState = false ;
	}

	// テクスチャ更新
	public void updateTexture()
	{
		if ( m_flagOpened && (m_surfaceTexture != null) )
		{
			m_surfaceTexture.updateTexImage() ;
		}
	}

#endif
}

