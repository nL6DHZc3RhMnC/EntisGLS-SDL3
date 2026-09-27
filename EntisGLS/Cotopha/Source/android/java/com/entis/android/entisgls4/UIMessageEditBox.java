package com.entis.android.entisgls4 ;

import android.app.AlertDialog ;
import android.content.DialogInterface ;
import android.widget.EditText ;
import android.text.InputType ;

public class	UIMessageEditBox	extends UIMessageBox
{
	// スタイル
	public static final int	styleMsgBoxMask			= 0x00FF ;
	public static final int	styleEditMultiLine		= 0x0100 ;
	public static final int	styleEditNumber			= 0x0200 ;
	public static final int	styleEditPassword		= 0x0400 ;

	// エディット
	protected EditText	m_editText = null ;
	protected String	m_strEdit = null ;
	protected int		m_styleEdit = 0 ;

	// ボタンエントリ
	public static class	ButtonEntry	extends UIMessageBox.ButtonEntry
	{
		public ButtonEntry( UIDialog dialog, int type, String text, int value )
		{
			super( dialog, type, text, value, false ) ;
		}
		public void onClickButton()
		{
			if ( (m_type == typePositiveButton)
					&& (m_dialog instanceof UIMessageEditBox) )
			{
				UIMessageEditBox	editbox = (UIMessageEditBox) m_dialog ;
				editbox.m_strEdit = editbox.m_editText.getText().toString() ;
			}
			super.onClickButton() ;
		}
	}

	// 構築関数
	public UIMessageEditBox( String title, String msg )
	{
		super( title, msg ) ;
	}
	public UIMessageEditBox
		( String title, String msg, String edit, int style )
	{
		super( title, msg, style & styleMsgBoxMask ) ;
		m_strEdit = edit ;
		m_styleEdit = style ;
	}
	// ボタン追加
	@Override
	public void addButton( int type, String text, int value )
	{
		m_vecButtons.add( new ButtonEntry( this, type, text, value ) ) ;
	}
	// メッセージボックス生成
	@Override
	protected void onBeforeDialogCreate( AlertDialog.Builder builder )
	{
		super.onBeforeDialogCreate( builder ) ;
		m_editText = new EditText( EntisGLS.getActivity() ) ;
		if ( m_strEdit != null )
		{
			m_editText.setText( m_strEdit ) ;
		}
		int	typeInput = InputType.TYPE_CLASS_TEXT ;
		if ( (m_styleEdit & styleEditNumber) != 0 )
		{
			typeInput = InputType.TYPE_CLASS_NUMBER ;
		}
		if ( (m_styleEdit & styleEditMultiLine) != 0 )
		{
			typeInput |= InputType.TYPE_TEXT_FLAG_MULTI_LINE ;
		}
		if ( (m_styleEdit & styleEditPassword) != 0 )
		{
			typeInput |= InputType.TYPE_TEXT_VARIATION_PASSWORD ;
		}
		m_editText.setInputType( typeInput ) ;
		builder.setView( m_editText ) ;
	}
	// 入力文字列取得
	public String getEditString()
	{
		return	m_strEdit ;
	}

}

