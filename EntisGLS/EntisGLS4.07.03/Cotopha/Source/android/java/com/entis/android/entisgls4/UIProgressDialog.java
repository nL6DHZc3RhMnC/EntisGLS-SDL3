package com.entis.android.entisgls4 ;

import android.app.AlertDialog ;
import android.app.ProgressDialog ;


public class	UIProgressDialog	extends UIDialog
{
	// スタイル
	public static final int	STYLE_HORIZONTAL	= ProgressDialog.STYLE_HORIZONTAL ;
	public static final int	STYLE_SPINNER		= ProgressDialog.STYLE_SPINNER ;

	// プログレスバー生成パラメータ
	protected String		m_strTitle = "進行状況" ;
	protected String		m_strMessage = "..." ;
	protected int			m_maxRange = 1000 ;
	protected int			m_style = STYLE_HORIZONTAL ;
	protected boolean		m_flagCancelable = true ;

	// 構築関数
	UIProgressDialog()
	{
	}

	// 進行状況を設定するハンドラ
	protected class SetProgressRunnable	implements Runnable
	{
		protected int	m_pos ;
		protected int	m_total ;

		public SetProgressRunnable( int pos, int total )
		{
			m_pos = pos ;
			m_total = total ;
		}
		public void run()
		{
			if ( m_dialog instanceof ProgressDialog )
			{
				((ProgressDialog)m_dialog).setMax( m_total ) ;
				((ProgressDialog)m_dialog).setProgress( m_pos ) ;
			}
		}
	}

	// ダイアログ生成
	protected AlertDialog showDialog()
	{
		ProgressDialog	dlg =
				new ProgressDialog( EntisGLS.getActivity() ) ;
		dlg.setTitle( m_strTitle ) ;
		dlg.setMessage( m_strMessage ) ;
		dlg.setProgressStyle( m_style ) ;
		dlg.setMax( m_maxRange ) ;
		dlg.setCancelable( m_flagCancelable ) ;
		dlg.setCanceledOnTouchOutside( false ) ;
		dlg.setOnCancelListener( this ) ;
		dlg.show() ;
		return	dlg ;
	}

	// スピナースタイルを設定
	public void setStyleSplinner()
	{
		m_style = STYLE_SPINNER ;
		m_flagCancelable = false ;
	}

	// タイトル設定
	public void setTitle( String title, boolean fSync )
	{
		m_strTitle = title ;
		super.setTitle( title, fSync ) ;
	}

	// メッセージ設定
	public void setMessage( String msg, boolean fSync )
	{
		m_strMessage = msg ;
		super.setMessage( msg, fSync ) ;
	}

	// 進行状況設定
	public void setProgress( int pos, int total, boolean fSync )
	{
		m_maxRange = total ;
		if ( m_dialog != null )
		{
			EntisGLS.procedureOnUIThread
				( new SetProgressRunnable( pos, total ), fSync ) ;
		}
	}
	public void setProgress( int pos, int total )
	{
		setProgress( pos, total, true ) ;
	}

}
