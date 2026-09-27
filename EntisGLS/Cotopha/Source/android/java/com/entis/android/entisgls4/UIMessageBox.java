package com.entis.android.entisgls4 ;

import java.util.Vector ;
import android.content.DialogInterface ;
import android.app.AlertDialog ;
import android.view.View ;
import android.widget.Button ;


public class	UIMessageBox	extends UIDialog
{
	// スタイル
	public static final int	styleOk					= 0 ;
	public static final int	styleOkCancel			= 1 ;
	public static final int	styleYesNo				= 2 ;
	public static final int	styleYesNoCancel		= 3 ;
	public static final int	styleRetryCancel		= 4 ;
	public static final int	styleAbortRetryIgnore	= 5 ;

	// ボタンタイプ
	public static final int	typePositiveButton	= 0 ;
	public static final int	typeNegativeButton	= 1 ;
	public static final int	typeNeutralButton	= 2 ;

	// ボタンエントリ
	public static class	ButtonEntry
			implements View.OnClickListener, DialogInterface.OnClickListener
	{
		public UIDialog	m_dialog = null ;
		public int		m_type = typeNeutralButton ;
		public String	m_text = null ;
		public int		m_value = 0 ;
		public boolean	m_manual = false ;

		public ButtonEntry( UIDialog dialog )
		{
			m_dialog = dialog ;
		}
		public ButtonEntry
			( UIDialog dialog, int type,
					String text, int value, boolean manual )
		{
			m_dialog = dialog ;
			m_type = type ;
			m_text = text ;
			m_value = value ;
			m_manual = manual ;
		}
		public void onClick( View v )
		{
			onClickButton() ;
		}
		public void onClick
			( DialogInterface dialog, int whichButton )
		{
			onClickButton() ;
		}
		public void onClickButton()
		{
			synchronized( m_dialog )
			{
				m_dialog.m_nResult = m_value ;
				m_dialog.m_flagDone = true ;
				m_dialog.notifyAll() ;
			}
		}
	}

	// メッセージボックス生成パラメータ
	protected String		m_strTitle = null ;
	protected String		m_strMessage = null ;
	protected boolean		m_flagCancelable = true ;
	protected Vector<ButtonEntry>
							m_vecButtons = new Vector<ButtonEntry>() ;


	// 構築関数
	public UIMessageBox( String title, String msg )
	{
		m_strTitle = title ;
		m_strMessage = msg ;
	}
	public UIMessageBox
		( String title, String msg, int style )
	{
		this( title, msg ) ;
		//
		switch ( style )
		{
		case	styleOk:
			addPositiveButton( "OK", resultOk ) ;
			break ;
		case	styleOkCancel:
			addPositiveButton( "OK", resultOk ) ;
			addNegativeButton( "CANCEL", resultCancel ) ;
			break ;
		case	styleYesNo:
			addPositiveButton( "はい", resultYes ) ;
			addNegativeButton( "いいえ", resultNo ) ;
			break ;
		case	styleYesNoCancel:
			addPositiveButton( "はい", resultYes ) ;
			addNeutralButton( "いいえ", resultNo ) ;
			addNegativeButton( "CANCEL", resultCancel ) ;
			break ;
		case	styleRetryCancel:
			addPositiveButton( "再試行", resultRetry ) ;
			addNegativeButton( "CANCEL", resultCancel ) ;
			break ;
		case	styleAbortRetryIgnore:
			addPositiveButton( "中断", resultAbort ) ;
			addNegativeButton( "無視", resultIgnore ) ;
			addNeutralButton( "再試行", resultRetry ) ;
			break ;
		}
	}
	// キャンセル可能か設定
	public void setCancelable( boolean cancelable )
	{
		m_flagCancelable = cancelable ;
	}
	// ボタン追加
	public void addButton( int type, String text, int value )
	{
		m_vecButtons.add( new ButtonEntry( this, type, text, value, false ) ) ;
	}
	public void addPositiveButton( String text, int value )
	{
		addButton( typePositiveButton, text, value ) ;
	}
	public void addNegativeButton( String text, int value )
	{
		addButton( typeNegativeButton, text, value ) ;
	}
	public void addNeutralButton( String text, int value )
	{
		addButton( typeNeutralButton, text, value ) ;
	}
	// メッセージボックス生成
	protected AlertDialog showDialog()
	{
		AlertDialog.Builder	builder =
			new AlertDialog.Builder( EntisGLS.getActivity() ) ;
		onBeforeDialogCreate( builder ) ;
		builder.create() ;
		return	builder.show() ;
	}
	protected void onBeforeDialogCreate( AlertDialog.Builder builder )
	{
		if ( m_strTitle != null )
		{
			builder.setTitle( m_strTitle ) ;
		}
		if ( m_strMessage != null )
		{
			builder.setMessage( m_strMessage ) ;
		}
		builder.setCancelable( m_flagCancelable ) ;
		builder.setOnCancelListener( this ) ;
		for ( int i = 0; i < m_vecButtons.size(); i ++ )
		{
			ButtonEntry						btn = m_vecButtons.get( i ) ;
			DialogInterface.OnClickListener	listener = btn ;
			if ( btn.m_manual )
			{
				listener = null ;
			}
			switch ( btn.m_type )
			{
			case	typePositiveButton:
				builder.setPositiveButton( btn.m_text, listener ) ;
				break ;
			case	typeNegativeButton:
				builder.setNegativeButton( btn.m_text, listener ) ;
				break ;
			case	typeNeutralButton:
				builder.setNeutralButton( btn.m_text, listener ) ;
				break ;
			}
		}
	}
	// ダイアログ生成後処理
	protected void onShowDialog( AlertDialog dlg )
	{
		super.onShowDialog( dlg ) ;
		//
		for ( int i = 0; i < m_vecButtons.size(); i ++ )
		{
			ButtonEntry	btn = m_vecButtons.get( i ) ;
			if ( btn.m_manual )
			{
				Button	button = null ;
				switch ( btn.m_type )
				{
				case	typePositiveButton:
					button = dlg.getButton( DialogInterface.BUTTON_POSITIVE ) ;
					break ;
				case	typeNegativeButton:
					button = dlg.getButton( DialogInterface.BUTTON_NEGATIVE ) ;
					break ;
				case	typeNeutralButton:
					button = dlg.getButton( DialogInterface.BUTTON_NEUTRAL ) ;
					break ;
				}
				if ( button != null )
				{
					button.setOnClickListener( btn ) ;
				}
			}
		}
	}


}

