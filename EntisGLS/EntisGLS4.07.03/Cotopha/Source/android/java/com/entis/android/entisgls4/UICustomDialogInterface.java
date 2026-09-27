package com.entis.android.entisgls4 ;

import java.nio.ByteBuffer ;
import android.view.View ;
import android.widget.TextView ;
import android.widget.EditText ;
import android.widget.Button ;
import android.widget.CompoundButton ;
import android.widget.ProgressBar ;
import android.widget.Spinner ;

public interface	UICustomDialogInterface
{
	// m_type
	public static final int	itemNull			= 0 ;
	public static final int	itemText			= 1 ;
	public static final int	itemEdit			= 2 ;
	public static final int	itemButton			= 3 ;
	public static final int	itemCheck			= 4 ;
	public static final int	itemRadio			= 5 ;
	public static final int	itemGroupBox		= 6 ;
	public static final int	itemProgress		= 7 ;
	public static final int	itemScroll			= 8 ;
	public static final int	itemDropDownList	= 9 ;

	// m_nFlags
	public static final int	flagEndOfLine		= 0x0001 ;
	public static final int	flagEndOfRadio		= 0x0002 ;
	public static final int	flagEndOfGroupBox	= 0x0004 ;
	public static final int	flagLineCenter		= 0x0008 ;
	public static final int	flagLineRight		= 0x0010 ;
	public static final int	flagFullWidth		= 0x0020 ;
	public static final int	flagMaskTypeButton	= 0x0300 ;
	public static final int	flagPositiveButton	= 0x0100 ;
	public static final int	flagNegativeButton	= 0x0200 ;
	public static final int	flagNeutralButton	= 0x0300 ;
	public static final int	flagMinWidth		= 0x0400 ;
	public static final int	flagArrangeCol1		= 0x1000 ;
	public static final int	flagArrangeCol2		= 0x2000 ;
	public static final int	flagArrangeCol3		= 0x4000 ;
	public static final int	flagArrangeCol4		= 0x8000 ;

	// m_optFlags
	public static final int	editboxStyleMultiLine	= 0x00000001 ;
	public static final int	editboxStyleNumber		= 0x00000002 ;
	public static final int	editboxStylePassword	= 0x00000004 ;

	// アイテム
	public static class Item
	{
		public String		m_strID ;
		public int			m_type ;
		public int			m_nFlags ;
		public int			m_optFlags ;
		public int			m_nMinWidth ;
		public String		m_strText ;
		public int			m_nValue ;
		public int			m_minRange ;
		public int			m_maxRange ;
		public ByteBuffer	m_bufInstance ;
		public View			m_view ;

		// 入力値取得
		public void getInputValue()
		{
			if ( EntisGLS.isPrimaryThread() )
			{
				if ( m_view instanceof EditText )
				{
					m_strText = ((EditText)m_view).getText().toString() ;
				}
				else if ( m_view instanceof CompoundButton )
				{
					m_nValue = ((CompoundButton)m_view).isChecked() ? 1 : 0 ;
				}
				else if ( m_view instanceof ProgressBar )
				{
					m_nValue = ((ProgressBar)m_view).getProgress() + m_minRange ;
				}
				else if ( m_view instanceof Spinner )
				{
					/*
					String	strProp =
								((Spinner)m_view).getPrompt().toString() ;
					String[]	aTextList = m_strText.split( "\n", 0 ) ;
					for ( int i = 0; i < aTextList.length; i ++ )
					{
						if ( strProp.equals( aTextList[i] ) )
						{
							m_nValue = i ;
							break ;
						}
					}
					*/
				}
			}
			else
			{
				EntisGLS.procedureOnUIThread
					( new GetItemInputValueRunnable( this ), true ) ;
			}
		}
		// 数値設定
		public void setViewInteger( int nValue )
		{
			if ( EntisGLS.isPrimaryThread() )
			{
				if ( m_view instanceof EditText )
				{
					m_strText = "" + nValue ;
					((EditText)m_view).setText( m_strText ) ;
				}
				else if ( m_view instanceof CompoundButton )
				{
					m_nValue = (nValue != 0) ? 1 : 0 ;
					((CompoundButton)m_view).setChecked( nValue != 0 ) ;
				}
				else if ( m_view instanceof ProgressBar )
				{
					m_nValue = nValue ;
					((ProgressBar)m_view).setProgress( m_nValue - m_minRange ) ;
				}
				else if ( m_view instanceof Spinner )
				{
					m_nValue = nValue ;
					((Spinner)m_view).setPromptId( m_nValue ) ;
				}
			}
			else
			{
				EntisGLS.procedureOnUIThread
					( new SetItemIntegerRunnable( this, nValue ), true ) ;
			}
		}
		// テキスト設定
		public void setViewString( String text )
		{
			if ( EntisGLS.isPrimaryThread() )
			{
				if ( m_view instanceof TextView )
				{
					m_strText = text ;
					((TextView)m_view).setText( m_strText ) ;
				}
				else if ( m_view instanceof Button )
				{
					((Button)m_view).setText( text ) ;
				}
			}
			else
			{
				EntisGLS.procedureOnUIThread
					( new SetItemStringRunnable( this, text ), true ) ;
			}
		}
		// 有効状態設定
		public void setEnabled( boolean flagEnabled )
		{
			if ( EntisGLS.isPrimaryThread() )
			{
				if ( m_view != null )
				{
					m_view.setEnabled( flagEnabled ) ;
				}
			}
			else
			{
				EntisGLS.procedureOnUIThread
					( new SetItemEnabledRunnable( this, flagEnabled ), true ) ;
			}
		}
	}

	// アイテム値取得ハンドラ
	public static class	GetItemInputValueRunnable	implements Runnable
	{
		protected Item	m_item = null ;

		public GetItemInputValueRunnable( Item item )
		{
			m_item = item ;
		}
		public void run()
		{
			m_item.getInputValue() ;
		}
	}

	// アイテム値設定ハンドラ
	public static class	SetItemIntegerRunnable	implements Runnable
	{
		protected Item	m_item = null ;
		protected int	m_value = 0 ;

		public SetItemIntegerRunnable( Item item, int value )
		{
			m_item = item ;
			m_value = value ;
		}
		public void run()
		{
			m_item.setViewInteger( m_value ) ;
		}
	}
	public static class	SetItemStringRunnable	implements Runnable
	{
		protected Item		m_item = null ;
		protected String	m_value = null ;

		public SetItemStringRunnable( Item item, String value )
		{
			m_item = item ;
			m_value = value ;
		}
		public void run()
		{
			m_item.setViewString( m_value ) ;
		}
	}

	// アイテム状態設定ハンドラ
	public static class	SetItemEnabledRunnable	implements Runnable
	{
		protected Item		m_item = null ;
		protected boolean	m_enabled = false ;

		public SetItemEnabledRunnable( Item item, boolean enabled )
		{
			m_item = item ;
			m_enabled = enabled ;
		}
		public void run()
		{
			m_item.setEnabled( m_enabled ) ;
		}
	}

	// 入力値取得
	public int getInputIntegerAs( String id ) ;
	public String getInputStringAs( String id ) ;
	// 数値設定
	public void setItemIntegerAs( String id, int nValue ) ;
	// テキスト設定
	public void setItemStringAs( String id, String text ) ;
	// ダイアログ終了
	public void endDialog( int nResultCode ) ;
	// コールバック
	public static final int	codeOnPushed	= 1 ;
	public static final int	codeOnChanged	= 2 ;
	public static final int	codeOnCanceled	= 3 ;
	public boolean callbackItem( Item item, int code ) ;
}


