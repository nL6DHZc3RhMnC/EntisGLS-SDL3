package com.entis.android.entisgls4 ;


public class	SyncRunnable	implements Runnable
{
	protected Runnable	m_runnable = null ;
	protected boolean	m_flagDone = false ;

	public SyncRunnable( Runnable r )
	{
		m_runnable = r ;
	}
	public void run()
	{
		m_runnable.run() ;
		synchronized( this )
		{
			m_flagDone = true ;
			notifyAll() ;
		}
	}
	public synchronized boolean waitDone()
	{
		while ( !m_flagDone )
		{
			try
			{
				wait() ;
			}
			catch ( Exception e )
			{
			}
		}
		return	true ;
	}
}


