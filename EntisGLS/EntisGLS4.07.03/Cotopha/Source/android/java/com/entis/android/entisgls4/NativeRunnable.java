package com.entis.android.entisgls4 ;

import java.nio.ByteBuffer ;


public class	NativeRunnable	implements Runnable
{
	protected ByteBuffer	m_buf ;

	public NativeRunnable( ByteBuffer buf )
	{
		m_buf = buf ;
	}
	public void run()
	{
		nativeRun( m_buf ) ;
	}
	protected static native void nativeRun( ByteBuffer buf ) ;

}

