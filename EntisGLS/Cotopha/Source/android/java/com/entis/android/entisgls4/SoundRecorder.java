package com.entis.android.entisgls4 ;

import java.nio.ByteBuffer ;
import android.media.AudioRecord ;
import android.media.AudioFormat ;
import android.media.MediaRecorder ;

public class SoundRecorder	implements Runnable
{
	// 入力デバイス種別
	public static int			DEVICE_DEFAULT	= 0 ;
	public static int			DEVICE_MIC		= 1 ;
	public static int			DEVICE_CAMERA	= 2 ;

	// ネイティブオブジェクト
	protected ByteBuffer		m_bufNativeObject = null ;

	// 出力オブジェクト
	protected AudioRecord		m_record = null ;

	// サウンドフォーマット
	protected int				m_device = DEVICE_DEFAULT ;
	protected int				m_format = 0 ;
	protected int				m_frequency = 44100 ;
	protected int				m_channels = 1 ;
	protected int				m_bitsPerSample = 16 ;
	protected int				m_bytesStreamingBuffer = 0 ;

	// 状態
	protected boolean			m_modeRecording = false ;

	// ストリーミングデータ
	protected static final int	STREAMING_BUF_COUNT = 32 ;
	protected byte[][]			m_bufStreamByteData = new byte[STREAMING_BUF_COUNT][] ;
	protected short[][]			m_bufStreamWordData = new short[STREAMING_BUF_COUNT][] ;
	protected int				m_bufStreamingCount = 0 ;

	// ストリーミングスレッド
	protected Thread			m_thread = null ;
	protected boolean			m_flagStreaming = false ;


	// 構築関数
	//////////////////////////////////////////////////////////////////////////
	public SoundRecorder( ByteBuffer buf )
	{
		m_bufNativeObject = buf ;
	}

	// ネイティブオブジェクト取得
	//////////////////////////////////////////////////////////////////////////
	public ByteBuffer getNativeObject()
	{
		return	m_bufNativeObject ;
	}

	// 入力オブジェクト生成
	//////////////////////////////////////////////////////////////////////////
	protected boolean createAudioRecord()
	{
		if ( m_record != null )
		{
			return	true ;
		}
		int	audioSource = 0 ;
		int	sampleRateInHz = m_frequency ;
		int	channelConfig = 0 ;
		int	audioFormat = 0 ;
		//
		if ( m_device == DEVICE_DEFAULT )
		{
			audioSource = MediaRecorder.AudioSource.DEFAULT ;
		}
		else if ( m_device == DEVICE_MIC )
		{
			audioSource = MediaRecorder.AudioSource.MIC ;
		}
#if	ANDROID_API_LEVEL >= 7
		else if ( m_device == DEVICE_CAMERA )
		{
			audioSource = MediaRecorder.AudioSource.CAMCORDER ;
		}
#endif
		else
		{
			return	false ;
		}
		switch ( m_channels )
		{
		case	2:
			channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO ;
			break ;
		case	1:
			channelConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO ;
			break ;
		default:
			return	false ;
		}
		switch ( m_bitsPerSample )
		{
		case	16:
			audioFormat = AudioFormat.ENCODING_PCM_16BIT ;
			break ;
		case	8:
			audioFormat = AudioFormat.ENCODING_PCM_8BIT ;
			break ;
		default:
			return	false ;
		}
		int	sizeMinBuf =
				AudioRecord.getMinBufferSize
					( sampleRateInHz, channelConfig, audioFormat ) ;
		if ( m_bytesStreamingBuffer < sizeMinBuf )
		{
			m_bytesStreamingBuffer = sizeMinBuf ;
		}
		try
		{
			m_record = new AudioRecord
				( audioSource,
					sampleRateInHz,
					channelConfig, audioFormat, m_bytesStreamingBuffer ) ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at new AudioRecord" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		m_modeRecording = false ;
		return	true ;
	}

	// 入力オブジェクト破棄
	//////////////////////////////////////////////////////////////////////////
	protected void releaseAudioRecord()
	{
		if ( m_record != null )
		{
			try
			{
				if ( m_modeRecording )
				{
					m_record.stop() ;
				}
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioRecord.stop()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
			m_record.release() ;
			m_record = null ;
			m_modeRecording = false ;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// ストリーミング再生スレッド
	//////////////////////////////////////////////////////////////////////////
	public void run()
	{
		AudioRecord	record = m_record ;
		while ( m_flagStreaming )
		{
			if ( m_bitsPerSample == 8 )
			{
				byte[]	buf = new byte[m_bytesStreamingBuffer] ;
				int	nRead = record.read( buf, 0, buf.length ) ;
				if ( nRead > 0 )
				{
					if ( nRead < buf.length )
					{
						byte[]	bufTemp = new byte[nRead] ;
						for ( int i = 0; i < nRead; i ++ )
						{
							bufTemp[i] = buf[i] ;
						}
						buf = bufTemp ;
					}
					synchronized( this )
					{
						if ( m_bufStreamingCount < STREAMING_BUF_COUNT )
						{
							m_bufStreamByteData[m_bufStreamingCount ++] = buf ;
						}
					}
				}
			}
			else if ( m_bitsPerSample == 16 )
			{
				short[]	buf = new short[m_bytesStreamingBuffer >> 1] ;
				int	nRead = record.read( buf, 0, buf.length ) ;
				if ( nRead > 0 )
				{
					if ( nRead < buf.length )
					{
						short[]	bufTemp = new short[nRead] ;
						for ( int i = 0; i < nRead; i ++ )
						{
							bufTemp[i] = buf[i] ;
						}
						buf = bufTemp ;
					}
					synchronized( this )
					{
						if ( m_bufStreamingCount < STREAMING_BUF_COUNT )
						{
							m_bufStreamWordData[m_bufStreamingCount ++] = buf ;
						}
					}
				}
			}
			else
			{
				break ;
			}
			try
			{
				Thread.sleep( 1 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		synchronized( this )
		{
			m_thread = null ;
			notifyAll() ;
		}
	}

	// ストリーミングスレッド終了
	protected synchronized void abortStreamingThread()
	{
		if ( m_flagStreaming || (m_thread != null) )
		{
			m_flagStreaming = false ;
			notifyAll() ;
			//
			while ( m_thread != null )
			{
				try
				{
					wait( 100 ) ;
				}
				catch ( Exception e )
				{
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	// 出力フォーマット設定
	public boolean setFormat
		( int device, int format, int frequency, int channels, int bitsPerSample )
	{
		releaseAudioRecord() ;
		//
		m_device = device ;
		m_format = format ;
		m_frequency = frequency ;
		m_channels = channels ;
		m_bitsPerSample = bitsPerSample ;
		return	(format == 0) && (channels >= 1) && (channels <= 2)
					&& ((bitsPerSample == 8) || (bitsPerSample == 16)) ;
	}

	// 出力オブジェクト解放
	public void close()
	{
		stop() ;
	}

	// ストリーミングの準備を行う
	public void prepareStreaming( int bytesBuf )
	{
		m_bytesStreamingBuffer = bytesBuf ;
	}

	// 開始
	public boolean start()
	{
		if ( m_modeRecording )
		{
			return	false ;
		}
		abortStreamingThread() ;
		//
		if ( !createAudioRecord() )
		{
			return	false ;
		}
		try
		{
			m_record.startRecording() ;
			//
			m_thread = new Thread( this ) ;
			m_thread.start() ;
			m_flagStreaming = true ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at AudioRecord.startRecording()" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		return	true ;
	}

	// 停止
	public boolean stop()
	{
		if ( m_record == null )
		{
			return	false ;
		}
		abortStreamingThread() ;
		releaseAudioRecord() ;
		m_modeRecording = false ;
		return	true ;
	}

	// 読み込み
	public synchronized byte[] getByteData()
	{
		if ( m_bufStreamingCount == 0 )
		{
			return	null ;
		}
		byte[]	buf = m_bufStreamByteData[0] ;
		for ( int i = 1; i < m_bufStreamingCount; i ++ )
		{
			m_bufStreamByteData[i - 1] = m_bufStreamByteData[i] ;
		}
		m_bufStreamingCount -- ;
		//
		return	buf ;
	}
	public synchronized short[] getWordData()
	{
		if ( m_bufStreamingCount == 0 )
		{
			return	null ;
		}
		short[]	buf = m_bufStreamWordData[0] ;
		for ( int i = 1; i < m_bufStreamingCount; i ++ )
		{
			m_bufStreamWordData[i - 1] = m_bufStreamWordData[i] ;
		}
		m_bufStreamingCount -- ;
		//
		return	buf ;
	}

	// 録音中か？
	public boolean isRecording()
	{
		return	m_modeRecording ;
	}
}


