package com.entis.android.entisgls4 ;

import java.nio.ByteBuffer ;
import android.media.AudioTrack ;
import android.media.AudioFormat ;
import android.media.AudioManager ;

public class SoundPlayer
			implements Runnable,
					AudioTrack.OnPlaybackPositionUpdateListener 
{
	// ネイティブオブジェクト
	protected ByteBuffer		m_bufNativeObject = null ;

	// 出力オブジェクト
	protected AudioTrack		m_audio = null ;

	// サウンドフォーマット
	protected int				m_format = 0 ;
	protected int				m_frequency = 44100 ;
	protected int				m_channels = 1 ;
	protected int				m_bitsPerSample = 16 ;
	protected int				m_bytesStreamingBuffer = 0 ;

	// 状態
	protected boolean			m_modeStatic = false ;
	protected boolean			m_modePlaying = false ;
	protected boolean			m_modeRepeat = false ;
	protected int				m_countPaused = 0 ;

	// 音量
	protected double			m_fpLeftVolume = 1.0 ;
	protected double			m_fpRightVolume = 1.0 ;

	// 再生位置更新通知処理用パラメータ
	protected int				m_samplesPeriod ;
	protected int				m_countPeriod ;
	protected long				m_bytesPlayingStart = 0 ;

	// スタティックデータ
	protected byte[]			m_bufStaticByteData ;
	protected short[]			m_bufStaticWordData ;
	protected long				m_posSeekStart ;
	protected int				m_posStaticEndMarker ;
	protected boolean			m_flagStaticLoaded ;
	protected long				m_msecStartPlay ;

	// ストリーミングデータ
	protected static final int	STREAMING_BUF_COUNT = 32 ;
	protected byte[][]			m_bufStreamByteData = new byte[STREAMING_BUF_COUNT][] ;
	protected short[][]			m_bufStreamWordData = new short[STREAMING_BUF_COUNT][] ;
	protected int				m_bufStreamingCount = 0 ;
	
	// ストリーミングスレッド
	protected Thread			m_thread = null ;
	protected boolean			m_flagStreaming = false ;

	// ストリーミングリスナ
	public static interface	StreamingListener
	{
		public void onStream( SoundPlayer player ) ;
	}
	public StreamingListener	m_listener = null ;

	
	// 構築関数
	//////////////////////////////////////////////////////////////////////////
	public SoundPlayer( ByteBuffer buf )
	{
		m_bufNativeObject = buf ;
	}

	// ネイティブオブジェクト取得
	//////////////////////////////////////////////////////////////////////////
	public ByteBuffer getNativeObject()
	{
		return	m_bufNativeObject ;
	}

	// 出力オブジェクト生成
	//////////////////////////////////////////////////////////////////////////
	protected boolean createAudioTrack( boolean flagStatic, int sizeBuf )
	{
		if ( m_audio != null )
		{
			return	true ;
		}
		int	sampleRateInHz = m_frequency ;
		int	channelConfig = 0 ;
		int	audioFormat = 0 ;
		//
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
				AudioTrack.getMinBufferSize
					( sampleRateInHz, channelConfig, audioFormat ) ;
		if ( sizeBuf < sizeMinBuf )
		{
			sizeBuf = sizeMinBuf ;
		}
		try
		{
			if ( flagStatic )
			{
				m_audio = new AudioTrack
					( AudioManager.STREAM_MUSIC,
						sampleRateInHz, channelConfig, audioFormat,
						sizeBuf, AudioTrack.MODE_STATIC ) ;
				m_modeStatic = true ;
			}
			else
			{
				m_audio = new AudioTrack
					( AudioManager.STREAM_MUSIC,
						sampleRateInHz, channelConfig, audioFormat,
						sizeBuf, AudioTrack.MODE_STREAM ) ;
				m_modeStatic = false ;
			}
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at new AudioTrack" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			return	false ;
		}
		m_samplesPeriod = m_frequency / 16 ;
		m_countPeriod = 0 ;
		m_audio.setPlaybackPositionUpdateListener( this ) ;
		m_audio.setPositionNotificationPeriod( m_samplesPeriod ) ;
		//
		setVolume( m_fpLeftVolume, m_fpRightVolume ) ;
		//
		m_modePlaying = false ;
		m_countPaused = 0 ;
		return	true ;
	}

	// 出力オブジェクト破棄
	//////////////////////////////////////////////////////////////////////////
	protected void releaseAudioTrack()
	{
		if ( m_audio != null )
		{
			try
			{
				if ( m_modePlaying )
				{
					m_audio.stop() ;
				}
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioTrack.stop()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
			//
			abortStreamingThread() ;
			//
			m_audio.release() ;
			m_audio = null ;
			m_modeStatic = false ;
			m_modePlaying = false ;
			m_countPaused = 0 ;
			m_countPeriod = 0 ;
			m_bytesPlayingStart = 0 ;
		}
	}

	// スタティック再生データ準備
	//////////////////////////////////////////////////////////////////////////
	protected boolean prepateStaticData( long posStart )
	{
		int	bytesSample = (m_channels * m_bitsPerSample) >> 3 ;
		int	lengthPlay = 0 ;
		int	offsetStart = (int) posStart * bytesSample ;
		if ( m_bufStaticByteData != null )
		{
			lengthPlay = m_bufStaticByteData.length ;
		}
		else if ( m_bufStaticWordData != null )
		{
			lengthPlay = m_bufStaticWordData.length ;
			offsetStart >>= 1 ;
		}
		else
		{
			return	false ;
		}
		if ( offsetStart >= lengthPlay )
		{
			return	false ;
		}
		lengthPlay -= offsetStart ;
		if ( m_bufStaticByteData != null )
		{
			if ( createAudioTrack( true, lengthPlay ) )
			{
//				m_audio.reloadStaticData() ;
				m_posStaticEndMarker = lengthPlay / bytesSample ;
				m_audio.write
					( m_bufStaticByteData, offsetStart, lengthPlay ) ;
				m_audio.setNotificationMarkerPosition( m_posStaticEndMarker - 1 ) ;
				m_flagStaticLoaded = true ;
				return	true ;
			}
		}
		else if ( m_bufStaticWordData != null )
		{
			if ( createAudioTrack( true, (lengthPlay << 1) ) )
			{
//				m_audio.reloadStaticData() ;
				m_posStaticEndMarker = (lengthPlay << 1) / bytesSample ;
				m_audio.write
					( m_bufStaticWordData, offsetStart, lengthPlay ) ;
				m_audio.setNotificationMarkerPosition( m_posStaticEndMarker - 1 ) ;
				m_flagStaticLoaded = true ;
				return	true ;
			}
		}
		return	false ;
	}


	//////////////////////////////////////////////////////////////////////////
	// AudioTrack.OnPlaybackPositionUpdateListener 実装
	//////////////////////////////////////////////////////////////////////////
	public synchronized void onMarkerReached( AudioTrack track )
	{
		if ( m_modeStatic && (m_audio != null) )
		{
			m_posSeekStart = 0 ;
			m_countPeriod = 0 ;
			m_bytesPlayingStart = 0 ;
			//
			if ( m_modeRepeat )
			{
				/*
//				m_modePlaying = prepateStaticData( 0 ) ;
				if ( m_modePlaying )
				{
					try
					{
						m_audio.reloadStaticData() ;
						m_audio.setNotificationMarkerPosition
											( m_posStaticEndMarker ) ;
						m_audio.play() ;
					}
					catch ( Throwable e )
					{
						EntisGLS.logError
							( "exception at AudioTrack.play() onMarkerReached" ) ;
						EntisGLS.logError( e.getMessage() ) ;
						m_modePlaying =
							(m_audio.getPlayState() ==  AudioTrack.PLAYSTATE_PLAYING) ;
					}
				}
				*/
			}
			else // if ( m_audio.getPlayState() == AudioTrack.PLAYSTATE_PLAYING )
			{
				m_modePlaying = false ;
				try
				{
					m_audio.stop() ;
				}
				catch ( Throwable e )
				{
					EntisGLS.logError
						( "exception at AudioTrack.stop() onMarkerReached" ) ;
					EntisGLS.logError( e.getMessage() ) ;
				}
			}
		}
	}
	public synchronized void onPeriodicNotification( AudioTrack track )
	{
		m_countPeriod ++ ;
	}


	//////////////////////////////////////////////////////////////////////////
	// ストリーミング再生スレッド
	//////////////////////////////////////////////////////////////////////////
	public void run()
	{
		AudioTrack	audio = m_audio ;
		while ( m_flagStreaming )
		{
			byte[]	bufByte = null ;
			short[]	bufWord = null ;
			synchronized( this )
			{
				while ( m_flagStreaming && (m_bufStreamingCount == 0) )
				{
					try
					{
						wait( 100 ) ;
					}
					catch ( Exception e )
					{
					}
				}
				if ( !m_flagStreaming )
				{
					break ;
				}
				bufByte = m_bufStreamByteData[0] ;
				bufWord = m_bufStreamWordData[0] ;
				for ( int i = 1; i < m_bufStreamingCount; i ++ )
				{
					m_bufStreamByteData[i - 1] = m_bufStreamByteData[i] ;
					m_bufStreamWordData[i - 1] = m_bufStreamWordData[i] ;
				}
				m_bufStreamingCount -- ;
				for ( int i = m_bufStreamingCount; i < STREAMING_BUF_COUNT; i ++ )
				{
					m_bufStreamByteData[i] = null ;
					m_bufStreamWordData[i] = null ;
				}
			}
			if ( bufByte != null )
			{
				audio.write( bufByte, 0, bufByte.length ) ;
			}
			else if ( bufWord != null )
			{
				audio.write( bufWord, 0, bufWord.length ) ;
			}
			if ( m_listener != null )
			{
				m_listener.onStream( this ) ;
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
			m_countPeriod = 0 ;
			m_bytesPlayingStart = 0 ;
		}
	}

	
	//////////////////////////////////////////////////////////////////////////
	// 再生インターフェース
	//////////////////////////////////////////////////////////////////////////

	// 出力フォーマット設定
	public boolean setFormat
		( int format, int frequency, int channels, int bitsPerSample )
	{
		releaseAudioTrack() ;
		//
		m_format = format ;
		m_frequency = frequency ;
		m_channels = channels ;
		m_bitsPerSample = bitsPerSample ;
		m_modeStatic = false ;
		return	(format == 0) && (channels >= 1) && (channels <= 2)
					&& ((bitsPerSample == 8) || (bitsPerSample == 16)) ;
	}

	// 出力オブジェクト解放
	public void close()
	{
		releaseAudioTrack() ;
	}

	// スタティックバッファを確保し書き込む
	public boolean writeStatic( byte[] buf )
	{
		releaseAudioTrack() ;
		//
		m_bufStaticByteData = buf ;
		m_bufStaticWordData = null ;
		m_posSeekStart = 0 ;
		//
		return	prepateStaticData( 0 ) ;
	}
	public boolean writeStatic( short[] buf )
	{
		releaseAudioTrack() ;
		//
		m_bufStaticByteData = null ;
		m_bufStaticWordData = buf ;
		m_posSeekStart = 0 ;
		//
		return	prepateStaticData( 0 ) ;
	}

	// ストリーミングの準備を行う
	public void prepareStreaming( int bytesBuf )
	{
		m_bytesStreamingBuffer = bytesBuf ;
	}

	// ストリーミングデータを追加する
	public synchronized int writeStreaming( byte[] buf )
	{
		if ( (buf == null)
			|| (buf.length == 0)
			|| (m_bufStreamingCount >= STREAMING_BUF_COUNT) )
		{
			return	0 ;
		}
		int	nBytes = 0 ;
		for ( int i = 0; i < m_bufStreamingCount; i ++ )
		{
			if ( m_bufStreamByteData[i] != null )
			{
				nBytes += m_bufStreamByteData[i].length ;
			}
		}
		if ( (m_bufStreamingCount >= 2)
			&& (nBytes >= m_frequency * m_channels * (m_bitsPerSample >> 3)) )
		{
			return	0 ;
		}
		m_bufStreamWordData[m_bufStreamingCount] = null ;
		m_bufStreamByteData[m_bufStreamingCount ++] = buf ;
		notifyAll() ;
		return	buf.length ;
	}
	public synchronized int writeStreaming( short[] buf )
	{
		if ( (buf == null)
			|| (buf.length == 0)
			|| (m_bufStreamingCount >= STREAMING_BUF_COUNT) )
		{
			return	0 ;
		}
		int	nBytes = 0 ;
		for ( int i = 0; i < m_bufStreamingCount; i ++ )
		{
			if ( m_bufStreamWordData[i] != null )
			{
				nBytes += (m_bufStreamWordData[i].length << 1) ;
			}
		}
		if ( (m_bufStreamingCount >= 2)
			&& (nBytes >= m_frequency * m_channels * (m_bitsPerSample >> 3)) )
		{
			return	0 ;
		}
		m_bufStreamByteData[m_bufStreamingCount] = null ;
		m_bufStreamWordData[m_bufStreamingCount ++] = buf ;
		notifyAll() ;
		return	buf.length ;
	}

	// 再生開始
	public boolean play( boolean flagRepeat )
	{
		if ( m_modePlaying )
		{
			return	false ;
		}
		if ( m_modeStatic )
		{
			if ( m_audio == null )
			{
				return	false ;
			}
			try
			{
				if ( !m_flagStaticLoaded )
				{
					m_audio.reloadStaticData() ;
					m_audio.setNotificationMarkerPosition( m_posStaticEndMarker - 1 ) ;
				}
				m_audio.setLoopPoints( 0, 0, 0 ) ;
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioTrack.reloadStaticData()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
			m_modeRepeat = flagRepeat ;
			m_bytesPlayingStart = m_posSeekStart ;
			if ( flagRepeat )
			{
				int	bytesInSample = m_channels * m_bitsPerSample / 8 ;
				if ( (bytesInSample != 0) && (m_bufStaticByteData != null) )
				{
					int	samples = m_bufStaticByteData.length / bytesInSample ;
					m_audio.setLoopPoints( 0, samples - (int) m_posSeekStart - 1, -1 ) ;
				}
				else if ( (bytesInSample != 0) && (m_bufStaticWordData != null) )
				{
					int	samples = m_bufStaticWordData.length * 2 / bytesInSample ;
					m_audio.setLoopPoints( 0, samples - (int) m_posSeekStart - 1, -1 ) ;
				}
			}
		}
		else
		{
			abortStreamingThread() ;
			//
			if ( !createAudioTrack( false, m_bytesStreamingBuffer ) )
			{
				return	false ;
			}
			if ( m_listener != null )
			{
				m_listener.onStream( this ) ;
			}
			m_flagStreaming = true ;
			m_bytesPlayingStart = 0 ;
			m_thread = new Thread( this ) ;
			m_thread.start() ;
		}
		try
		{
			m_flagStaticLoaded = false ;
			m_audio.play() ;
			m_modePlaying = true ;
			m_msecStartPlay = System.currentTimeMillis() ;
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at AudioTrack.play()" ) ;
			EntisGLS.logError( e.getMessage() ) ;
			if ( m_audio.getPlayState() ==  AudioTrack.PLAYSTATE_PLAYING )
			{
				m_modePlaying = true ;
				return	false ;
			}
			return	false ;
		}
		return	true ;
	}

	// 停止
	public boolean stop()
	{
		if ( m_audio == null )
		{
			return	false ;
		}
		try
		{
			if ( m_modePlaying )
			{
				m_audio.stop() ;
			}
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception at AudioTrack.stop()" ) ;
			EntisGLS.logError( e.getMessage() ) ;
		}
		m_modePlaying = false ;
		m_countPaused = 0 ;
		//
		abortStreamingThread() ;
		//
		return	true ;
	}

	// 一時停止
	public synchronized boolean pause()
	{
		if ( (m_audio == null) || !m_modePlaying )
		{
			return	false ;
		}
		if ( m_countPaused == 0 )
		{
			try
			{
				m_audio.pause() ;
				m_msecStartPlay = System.currentTimeMillis() - m_msecStartPlay ;
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioTrack.pause()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
		}
		m_countPaused ++ ;
		return	true ;
	}

	// 再生再開
	public synchronized boolean restart()
	{
		if ( (m_audio == null)
				|| !m_modePlaying || (m_countPaused == 0) )
		{
			return	false ;
		}
		if ( -- m_countPaused == 0 )
		{
			try
			{
				m_audio.play() ;
				m_msecStartPlay = System.currentTimeMillis() - m_msecStartPlay ;
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioTrack.play()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
		}
		return	true ;
	}

	// 音量取得
	public void getVolume( double[] volumes )
	{
		volumes[0] = m_fpLeftVolume ;
		volumes[1] = m_fpRightVolume ;
	}

	// 音量設定
	public boolean setVolume( double volLeft, double volRight )
	{
		m_fpLeftVolume = Math.min( Math.max( volLeft, 0.0 ), 1.0 ) ;
		m_fpRightVolume = Math.min( Math.max( volRight, 0.0 ), 1.0 ) ;
		//
		if ( m_audio != null )
		{
			float	fpMax = AudioTrack.getMaxVolume() ;
			float	fpMin = AudioTrack.getMinVolume() ;
			try
			{
				int	nResult = m_audio.setStereoVolume
					( (float) (fpMin + (fpMax - fpMin) * volLeft),
						(float) (fpMin + (fpMax - fpMin) * volRight) ) ;
				if ( nResult == AudioTrack.SUCCESS )
				{
					return	true ;
				}
				else
				{
					EntisGLS.logDebug
						( "failed to set volume of AudioTrack ("
							+ volLeft + "," + volRight + ") #" + nResult ) ;
					return	false ;
				}
			}
			catch ( Throwable e )
			{
				EntisGLS.logError( "exception at AudioTrack.setStereoVolume()" ) ;
				EntisGLS.logError( e.getMessage() ) ;
			}
		}
		return	true ;
	}

	// 現在の再生位置を取得
	public synchronized long getPlayingPosition()
	{
		int	bytesSample = ((m_channels * m_bitsPerSample) >> 3) ;
		if ( bytesSample > 0 )
		{
			return	(m_bytesPlayingStart / bytesSample)
							+ (m_countPeriod * m_samplesPeriod) ;
		}
		return	0 ;
	}

	// （スタティックバッファ）再生開始位置[/bytes]設定
	public boolean seekPosition( long nPos )
	{
		m_posSeekStart = nPos ;
		//
		boolean	flagPlaying = m_modePlaying ;
		boolean	flagRepeat = m_modeRepeat ;
		if ( flagPlaying )
		{
			stop() ;
		}
		if ( m_modeStatic )
		{
			if ( !prepateStaticData( m_posSeekStart ) )
			{
				return	false ;
			}
		}
		if ( flagPlaying )
		{
			return	play( flagRepeat ) ;
		}
		return	true ;
	}
	
	// 再生中か？
	public boolean isPlaying()
	{
		if ( m_modePlaying && m_modeStatic
			&& !m_modeRepeat && (m_countPaused == 0) && (m_audio != null) )
		{
			if ( m_audio.getPlayState() == AudioTrack.PLAYSTATE_STOPPED )
			{
				m_modePlaying = false ;
			}
			else if ( m_frequency >= 1 )
			{
				long	msecCurrent =
							System.currentTimeMillis() - m_msecStartPlay ;
				long	msecEndMarker =
							(long) m_posStaticEndMarker * 1000 / m_frequency ;
				if ( msecCurrent > msecEndMarker + 500 )
				{
					return	false ;
				}
			}
		}
		return	m_modePlaying ;
	}

	// 一時停止中か？
	public boolean isPaused()
	{
		return	(m_countPaused > 0) ;
	}

	// リスナ設定
	public synchronized StreamingListener
				setStreamingListener( StreamingListener listener )
	{
		StreamingListener	last = m_listener ;
		m_listener = listener ;
		return	last ;
	}
}

