package com.entis.android.entisgls4 ;

import android.content.DialogInterface ;
import android.app.AlertDialog ;


public abstract class	UIDialog
					implements Runnable,
								DialogInterface.OnCancelListener
{
	// 結果
	public static final int	resultOk		= 0 ;
	public static final int	resultCancel	= 1 ;
	public static final int	resultYes		= 2 ;
	public static final int	resultNo		= 3 ;
	public static final int	resultRetry		= 4 ;
	public static final int	resultAbort		= 5 ;
	public static final int	resultIgnore	= 6 ;
	public static final int	resultUser		= 7 ;

	protected volatile boolean	m_flagDone = false ;
	protected volatile boolean	m_flagCanceled = false ;
	protected int				m_nResult = resultOk ;
	protected AlertDialog		m_dialog = null ;

	// 実行（表示）
	public void run()
	{
		m_dialog = showDialog() ;
		onShowDialog( m_dialog ) ;
	}

	// ダイアログ生成
	protected abstract AlertDialog showDialog() ;

	// ダイアログ生成後処理
	protected void onShowDialog( AlertDialog dlg )
	{
	}

	// タイトルを設定するハンドラ
	protected class	SetTitleRunnable	implements Runnable
	{
		protected String	m_title = null ;

		public SetTitleRunnable( String title )
		{
			m_title = title ;
		}
		public void run()
		{
			if ( m_dialog != null )
			{
				m_dialog.setTitle( m_title ) ;
			}
		}
	}

	// メッセージを設定するハンドラ
	protected class	SetMessageRunnable	implements Runnable
	{
		protected String	m_msg = null ;

		public SetMessageRunnable( String msg )
		{
			m_msg = msg ;
		}
		public void run()
		{
			if ( m_dialog != null )
			{
				m_dialog.setMessage( m_msg ) ;
			}
		}
	}

	// ダイアログを閉じるハンドラ
	protected class	CloseDialogRunnable	implements Runnable
	{
		public void run()
		{
			if ( m_dialog != null )
			{
				m_dialog.dismiss() ;
				m_dialog = null ;
			}
		}
	}

	// キャンセル
	public synchronized void onCancel( DialogInterface dialog )
	{
		m_nResult = resultCancel ;
		m_flagCanceled = true ;
		m_flagDone = true ;
		notifyAll() ;
	}

	// タイトル設定
	public void setTitle( String title, boolean fSync )
	{
		if ( m_dialog != null )
		{
			EntisGLS.procedureOnUIThread
				( new SetTitleRunnable( title ), fSync ) ;
		}
	}
	public void setTitle( String title )
	{
		setTitle( title, true ) ;
	}

	// メッセージ設定
	public void setMessage( String msg, boolean fSync )
	{
		if ( m_dialog != null )
		{
			EntisGLS.procedureOnUIThread
				( new SetMessageRunnable( msg ), fSync ) ;
		}
	}
	public void setMessage( String msg )
	{
		setMessage( msg, true ) ;
	}

	// 閉じる
	public void closeDialog( boolean fSync )
	{
		EntisGLS.procedureOnUIThread
			( new CloseDialogRunnable(), fSync ) ;
	}
	public void closeDialog()
	{
		closeDialog( true ) ;
	}

	// キャンセルされたか？
	public final boolean isCanceled()
	{
		return	m_flagCanceled ;
	}

	// 終了待ち
	public synchronized boolean waitDone( long timeout )
	{
		try
		{
			if ( !m_flagDone )
			{
				wait( timeout ) ;
			}
		}
		catch ( Exception e )
		{
		}
		return	m_flagDone ;
	}
	public synchronized boolean waitDone()
	{
		try
		{
			while ( !m_flagDone )
			{
				wait() ;
			}
		}
		catch ( Exception e )
		{
		}
		return	m_flagDone ;
	}

	// 終了コード取得
	public int getResult()
	{
		return	m_nResult ;
	}

}

