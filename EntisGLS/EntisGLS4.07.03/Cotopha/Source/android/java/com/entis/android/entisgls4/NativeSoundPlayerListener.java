package com.entis.android.entisgls4 ;

import java.nio.ByteBuffer ;

public class	NativeSoundPlayerListener
						implements SoundPlayer.StreamingListener
{
	protected ByteBuffer	m_bufListener = null ;

	public NativeSoundPlayerListener( ByteBuffer bufListener )
	{
		m_bufListener = bufListener ;
	}

	public void onStream( SoundPlayer player )
	{
		if ( (m_bufListener != null) && (player != null) )
		{
			nativeOnStream( m_bufListener, player.getNativeObject() ) ;
		}
	}

	protected native static void nativeOnStream
			( ByteBuffer bufListener, ByteBuffer bufPlayer ) ;

}

