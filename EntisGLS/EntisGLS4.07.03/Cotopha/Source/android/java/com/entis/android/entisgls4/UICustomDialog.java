package com.entis.android.entisgls4 ;

import java.util.Vector ;
import java.nio.ByteBuffer ;

import android.content.Context ;
import android.content.DialogInterface ;
import android.app.AlertDialog ;
import android.view.Gravity ;
import android.view.View ;
import android.view.ViewGroup ;
import android.widget.LinearLayout ;
import android.widget.TextView ;
import android.widget.EditText ;
import android.widget.Button ;
import android.widget.CompoundButton ;
import android.widget.CheckBox ;
import android.widget.RadioButton ;
import android.widget.RadioGroup ;
import android.widget.ProgressBar ;
import android.widget.RelativeLayout.LayoutParams ;
import android.widget.SeekBar ;
import android.widget.Spinner ;
import android.widget.AdapterView ;
import android.widget.ArrayAdapter ;
import android.text.InputType ;


public class	UICustomDialog	extends UIDialog
								implements UICustomDialogInterface
{
	protected String	m_strTitle = null ;
	protected boolean	m_flagCancelable = true ;
	protected UICustomDialogInterface.Item[]
						m_items = null ;
	public ByteBuffer	m_bufInstance = null ;

	// ビュー
	public static class	SubView	extends LinearLayout
	{
		protected Item[]	m_items = null ;
		public SubView( Context context, Item[] items )
		{
			super( context ) ;
			m_items = items ;
		}
		protected void onMeasure( int widthMeasureSpec, int heightMeasureSpec )
		{
			super.onMeasure( widthMeasureSpec, heightMeasureSpec ) ;
			arrangeVertical( m_items ) ;
		}
	}

	// キャプション設定
	public void setTitle( String title )
	{
		if ( m_dialog != null )
		{
			setTitle( title, true ) ;
		}
		else
		{
			m_strTitle = title ;
		}
	}

	// フォーム設定
	public void setCustomItems( UICustomDialogInterface.Item[] items )
	{
		m_items = items ;
	}

	// キャンセル可能か設定
	public void setCancelable( boolean cancelable )
	{
		m_flagCancelable = cancelable ;
	}

	// 実行
	public int doModal()
	{
		EntisGLS.procedureOnUIThread( this ) ;
		waitDone() ;
		return	getResult() ;
	}

	// 入力値取得
	@Override
	public int getInputIntegerAs( String id )
	{
		UICustomDialogInterface.Item	item = getItemAs( id ) ;
		if ( item != null )
		{
			return	item.m_nValue ;
		}
		return	0 ;
	}
	@Override
	public String getInputStringAs( String id )
	{
		UICustomDialogInterface.Item	item = getItemAs( id ) ;
		if ( item != null )
		{
			return	item.m_strText ;
		}
		return	null ;
	}
	protected UICustomDialogInterface.Item getItemAs( String id )
	{
		if ( m_items == null )
		{
			return	null ;
		}
		for ( int i = 0; i < m_items.length; i ++ )
		{
			if ( (m_items[i] != null)
				&& (m_items[i].m_strID != null)
				&& m_items[i].m_strID.equals( id ) )
			{
				return	m_items[i] ;
			}
		}
		return	null ;
	}

	// 数値設定
	@Override
	public void setItemIntegerAs( String id, int nValue )
	{
		UICustomDialogInterface.Item	item = getItemAs( id ) ;
		if ( item != null )
		{
			item.setViewInteger( nValue ) ;
		}
	}

	// テキスト設定
	@Override
	public void setItemStringAs( String id, String text )
	{
		UICustomDialogInterface.Item	item = getItemAs( id ) ;
		if ( item != null )
		{
			item.setViewString( text ) ;
		}
	}

	// ダイアログ終了
	@Override
	public synchronized void endDialog( int nResultCode )
	{
		m_nResult = nResultCode ;
		m_flagDone = true ;
		notifyAll() ;
	}

	// コールバック
	public native boolean callbackItem
			( UICustomDialogInterface.Item item, int code ) ;

	// キャンセル
	@Override
	public void onCancel( DialogInterface dialog )
	{
		if ( callbackItem( null, UICustomDialogInterface.codeOnCanceled ) )
		{
			return ;
		}
		super.onCancel( dialog ) ;
	}

	// メッセージボックス生成
	protected AlertDialog showDialog()
	{
		AlertDialog.Builder	builder =
			new AlertDialog.Builder( EntisGLS.getActivity() ) ;
		builder.setCancelable( m_flagCancelable ) ;
		builder.setOnCancelListener( this ) ;
		if ( m_strTitle != null )
		{
			builder.setTitle( m_strTitle ) ;
		}
		if ( m_items != null )
		{
			builder.setView( createViewLayout( m_items, this ) ) ;
		}
		builder.create() ;
		return	builder.show() ;
	}

	// ダイアログ生成後処理
	protected void onShowDialog( AlertDialog dlg )
	{
//		arrangeVertical( m_items ) ;
	}

	// アイテムからビューを生成する
	public static View createViewLayout
			( Item[] items, UICustomDialogInterface dialog )
	{
		LinearLayout
			viewLayout =
				new SubView( EntisGLS.getActivity(), items ) ;
		viewLayout.setOrientation( LinearLayout.VERTICAL ) ;
		//
		ViewGroup.LayoutParams	layoutLine =
			new ViewGroup.LayoutParams
				( ViewGroup.LayoutParams.FILL_PARENT,
						ViewGroup.LayoutParams.WRAP_CONTENT ) ;
		ViewGroup.LayoutParams	layoutItem =
			new ViewGroup.LayoutParams
				( ViewGroup.LayoutParams.WRAP_CONTENT,
						ViewGroup.LayoutParams.WRAP_CONTENT ) ;
		ViewGroup.LayoutParams	layoutEndItem =
			new ViewGroup.LayoutParams
				( ViewGroup.LayoutParams.FILL_PARENT,
						ViewGroup.LayoutParams.WRAP_CONTENT ) ;
		//
		LinearLayout	viewLine = null ;
		RadioGroup		radioGroup = null ;
		for ( int i = 0; i < items.length; i ++ )
		{
			Item	item = items[i] ;
			if ( item == null )
			{
				continue ;
			}
			//
			// アイテム生成
			//
			View	view = createViewItem( item, dialog ) ;
			if ( view == null )
			{
				continue ;
			}
			item.m_view = view ;
			//
			// アイテムレイアウト
			//
			if ( viewLine == null )
			{
				viewLine = new LinearLayout( EntisGLS.getActivity() ) ;
				viewLine.setOrientation( LinearLayout.HORIZONTAL ) ;
				viewLayout.addView( viewLine, layoutLine ) ;
			}
			ViewGroup.LayoutParams	layout = layoutItem ;
			if ( (item.m_nFlags & (UICustomDialog.flagEndOfLine
								| UICustomDialog.flagEndOfGroupBox)) != 0 )
			{
				if ( (item.m_type == UICustomDialog.itemEdit)
					|| (item.m_type == UICustomDialog.itemScroll) )
				{
					layout = layoutEndItem ;
				}
			}
			//
			// アイテム追加
			//
			if ( item.m_type == UICustomDialog.itemRadio )
			{
				if ( radioGroup == null )
				{
					radioGroup = new RadioGroup( EntisGLS.getActivity() ) ;
					if ( (item.m_nFlags &
								(UICustomDialog.flagEndOfLine
									| UICustomDialog.flagEndOfGroupBox)) != 0 )
					{
						radioGroup.setOrientation( LinearLayout.VERTICAL ) ;
					}
					else
					{
						radioGroup.setOrientation( LinearLayout.HORIZONTAL ) ;
					}
				}
				radioGroup.addView( view, layout ) ;
				viewLine.addView( radioGroup, layout ) ;
			}
			else
			{
				viewLine.addView( view, layout ) ;
			}
			//
			// 行レイアウト
			//
			if ( (item.m_nFlags & UICustomDialog.flagLineCenter) != 0 )
			{
				viewLine.setGravity( Gravity.CENTER ) ;
			}
			else if ( (item.m_nFlags & UICustomDialog.flagLineRight) != 0 )
			{
				viewLine.setGravity( Gravity.RIGHT ) ;
			}
			if ( (item.m_nFlags
					& (UICustomDialog.flagEndOfLine
						| UICustomDialog.flagEndOfGroupBox)) != 0 )
			{
				viewLine = null ;
			}
			if ( (item.m_nFlags & UICustomDialog.flagEndOfRadio) != 0 )
			{
				radioGroup = null ;
			}
		}
		return	viewLayout ;
	}

	// 垂直整列処理
	public static void arrangeVertical( Item[] items )
	{
		for ( int i = 0; i < 4; i ++ )
		{
			int	maxArrangeLeft = 0 ;
			int	xArrangeLeft = 0 ;
			for ( int j = 0; j < items.length; j ++ )
			{
				Item	item = items[j] ;
				if ( (item == null) || (item.m_view == null) )
				{
					continue ;
				}
				if ( (item.m_nFlags & (flagArrangeCol1 << i)) != 0 )
				{
					if ( maxArrangeLeft < xArrangeLeft )
					{
						maxArrangeLeft = xArrangeLeft ;
					}
				}
				xArrangeLeft += item.m_view.getWidth() ;
				//
				if ( (item.m_nFlags
					& (UICustomDialog.flagEndOfLine
						| UICustomDialog.flagEndOfGroupBox)) != 0 )
				{
					xArrangeLeft = 0 ;
				}
			}
			View	viewLeft = null ;
			xArrangeLeft = 0 ;
			for ( int j = 0; j < items.length; j ++ )
			{
				Item	item = items[j] ;
				if ( (item == null) || (item.m_view == null) )
				{
					continue ;
				}
				if ( (item.m_nFlags & (flagArrangeCol1 << i)) != 0 )
				{
					if ( (viewLeft != null)
						&& (viewLeft instanceof TextView) )
					{
						TextView	viewText = (TextView) viewLeft ;
						int	nWidth = viewLeft.getWidth()
										+ maxArrangeLeft - xArrangeLeft ;
						viewText.setMinWidth( nWidth ) ;
					}
				}
				xArrangeLeft += item.m_view.getWidth() ;
				viewLeft = item.m_view ;
				//
				if ( (item.m_nFlags
						& (UICustomDialog.flagEndOfLine
							| UICustomDialog.flagEndOfGroupBox)) != 0 )
				{
					viewLeft = null ;
					xArrangeLeft = 0 ;
				}
			}
		}
	}

	public static View createViewItem
			( Item item, UICustomDialogInterface dialog )
	{
		if ( item == null )
		{
			return	null ;
		}
		float	fpDensity =
					EntisGLS.getActivity().
						getResources().getDisplayMetrics().density ;
		if ( (item.m_type == UICustomDialog.itemText)
			|| (item.m_type == UICustomDialog.itemGroupBox) )
		{
			TextView	view = new TextView( EntisGLS.getActivity() ) ;
			if ( (item.m_nFlags & flagMinWidth) != 0 )
			{
				view.setMinWidth( (int) (item.m_nMinWidth * fpDensity) ) ;
			}
			if ( item.m_strText != null )
			{
				view.setText( item.m_strText ) ;
			}
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemEdit )
		{
			EditText	view = new EditText( EntisGLS.getActivity() ) ;
			int	typeInput = InputType.TYPE_CLASS_TEXT ;
			if ( (item.m_optFlags & UICustomDialog.editboxStyleNumber) != 0 )
			{
				typeInput = InputType.TYPE_CLASS_NUMBER ;
			}
			if ( (item.m_optFlags & UICustomDialog.editboxStyleMultiLine) != 0 )
			{
				typeInput |= InputType.TYPE_TEXT_FLAG_MULTI_LINE ;
			}
			if ( (item.m_optFlags & UICustomDialog.editboxStylePassword) != 0 )
			{
				typeInput |= InputType.TYPE_TEXT_VARIATION_PASSWORD ;
			}
			view.setInputType( typeInput ) ;
			//
			if ( (item.m_nFlags & flagMinWidth) != 0 )
			{
				view.setMinWidth( (int) (item.m_nMinWidth * fpDensity) ) ;
			}
			if ( item.m_strText != null )
			{
				view.setText( item.m_strText ) ;
			}
			view.setOnFocusChangeListener
					( new ItemFocusChangeListener( item, dialog ) ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemButton )
		{
			Button	view = new Button( EntisGLS.getActivity() ) ;
			if ( item.m_strText != null )
			{
				view.setText( item.m_strText ) ;
			}
			view.setOnClickListener
				( new ItemClickListener
					( item, UICustomDialogInterface.codeOnPushed, dialog ) ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemCheck )
		{
			CheckBox	view = new CheckBox( EntisGLS.getActivity() ) ;
			if ( item.m_strText != null )
			{
				view.setText( item.m_strText ) ;
			}
			view.setChecked( (item.m_nValue != 0) ) ;
			view.setOnClickListener
				( new ItemClickListener
					( item, UICustomDialogInterface.codeOnChanged, dialog ) ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemRadio )
		{
			RadioButton	view = new RadioButton( EntisGLS.getActivity() ) ;
			if ( item.m_strText != null )
			{
				view.setText( item.m_strText ) ;
			}
			view.setChecked( (item.m_nValue != 0) ) ;
			view.setOnClickListener
				( new ItemClickListener
					( item, UICustomDialogInterface.codeOnChanged, dialog ) ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemProgress )
		{
			ProgressBar	view =
				new ProgressBar
					( EntisGLS.getActivity(), null,
						android.R.attr.progressBarStyleHorizontal ) ;
			view.setMax( item.m_maxRange - item.m_minRange ) ;
			view.setProgress( item.m_nValue - item.m_minRange ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemScroll )
		{
			SeekBar	view = new SeekBar( EntisGLS.getActivity() ) ;
			view.setMax( item.m_maxRange - item.m_minRange ) ;
			view.setProgress( item.m_nValue - item.m_minRange ) ;
			view.setOnSeekBarChangeListener
					( new ItemSeekListener( item, dialog ) ) ;
			return	view ;
		}
		else if ( item.m_type == UICustomDialog.itemDropDownList )
		{
			String[]	aTextList = item.m_strText.split( "\n", 0 ) ;
			ArrayAdapter<String>
				adpList = new ArrayAdapter
					( EntisGLS.getActivity(),
						android.R.layout.simple_spinner_item, aTextList ) ;
			adpList.setDropDownViewResource
				( android.R.layout.simple_spinner_dropdown_item ) ;
			//
			Spinner	view = new Spinner( EntisGLS.getActivity() ) ;
			view.setAdapter( adpList ) ;
			view.setSelection( item.m_nValue ) ;
			view.setOnItemSelectedListener
					( new ItemSelChangedListener( item, dialog ) ) ;
			return	view ;
		}
		return	null ;
	}

	// フォーカス遷移リスナ
	public static class	ItemFocusChangeListener
							implements View.OnFocusChangeListener
	{
		protected UICustomDialogInterface.Item	m_item ;
		protected UICustomDialogInterface		m_dialog ;
		protected Thread						m_thread = null ;

		public ItemFocusChangeListener
			( UICustomDialogInterface.Item item,
						UICustomDialogInterface dialog )
		{
			m_item = item ;
			m_dialog = dialog ;
		}
		@Override
		public void onFocusChange( View v, boolean hasFocus )
		{
			if ( !hasFocus && (v == m_item.m_view) )
			{
				if ( m_thread != null )
				{
					try
					{
						m_thread.join() ;
					}
					catch ( Exception e )
					{
					}
				}
				m_item.getInputValue() ;
				//
				m_thread = new Thread( new Runnable()
					{
						public void run()
						{
							m_dialog.callbackItem
								( m_item, UICustomDialogInterface.codeOnChanged ) ;
						}
					} ) ;
				m_thread.start() ;
			}
		}
	}
	// クリックリスナ
	public static class	ItemClickListener
							implements View.OnClickListener
	{
		protected UICustomDialogInterface.Item	m_item ;
		protected int							m_code ;
		protected UICustomDialogInterface		m_dialog ;
		protected Thread						m_thread = null ;

		public ItemClickListener
			( UICustomDialogInterface.Item item,
					int code, UICustomDialogInterface dialog )
		{
			m_item = item ;
			m_code = code ;
			m_dialog = dialog ;
		}
		@Override
		public void onClick( View v )
		{
			if ( v != m_item.m_view )
			{
				return ;
			}
			if ( m_thread != null )
			{
				try
				{
					m_thread.join() ;
				}
				catch ( Exception e )
				{
				}
			}
			m_item.getInputValue() ;
			//
			m_thread = new Thread( new Runnable()
				{
					public void run()
					{
						if ( !m_dialog.callbackItem( m_item, m_code ) )
						{
							int	nType = m_item.m_nFlags
											& UICustomDialogInterface.flagMaskTypeButton ;
							if ( nType == UICustomDialogInterface.flagPositiveButton )
							{
								m_dialog.endDialog( UIDialog.resultOk ) ;
							}
							else if ( nType == UICustomDialogInterface.flagNegativeButton )
							{
								m_dialog.endDialog( UIDialog.resultCancel ) ;
							}
							else if ( nType == UICustomDialogInterface.flagNeutralButton )
							{
								m_dialog.endDialog( UIDialog.resultUser ) ;
							}
						}
					}
				} ) ;
			m_thread.start() ;
		}
	}
	// シークバーリスナ
	public static class	ItemSeekListener
							implements SeekBar.OnSeekBarChangeListener
	{
		protected UICustomDialogInterface.Item	m_item ;
		protected UICustomDialogInterface		m_dialog ;
		protected Thread						m_thread = null ;

		public ItemSeekListener
			( UICustomDialogInterface.Item item,
						UICustomDialogInterface dialog )
		{
			m_item = item ;
			m_dialog = dialog ;
		}
		@Override
		public void onProgressChanged
				( SeekBar seekBar, int progress, boolean fromUser )
		{
			if ( seekBar != m_item.m_view )
			{
				return ;
			}
			if ( m_thread != null )
			{
				try
				{
					m_thread.join() ;
				}
				catch ( Exception e )
				{
				}
			}
			m_item.getInputValue() ;
			//
			m_thread = new Thread( new Runnable()
				{
					public void run()
					{
						m_dialog.callbackItem
							( m_item, UICustomDialogInterface.codeOnChanged ) ;
					}
				} ) ;
			m_thread.start() ;
		}
		@Override
		public void onStartTrackingTouch( SeekBar seekBar )
		{
		}
		@Override
		public void onStopTrackingTouch( SeekBar seekBar )
		{
		}
	}
	// スピナリスナ
	public static class ItemSelChangedListener
							implements AdapterView.OnItemSelectedListener
	{
		protected UICustomDialogInterface.Item	m_item ;
		protected UICustomDialogInterface		m_dialog ;
		protected Thread						m_thread = null ;

		public ItemSelChangedListener
			( UICustomDialogInterface.Item item,
						UICustomDialogInterface dialog )
		{
			m_item = item ;
			m_dialog = dialog ;
		}
		@Override
		public void onItemSelected
			( AdapterView<?> parent, View view, int position, long id )
		{
			if ( m_thread != null )
			{
				try
				{
					m_thread.join() ;
				}
				catch ( Exception e )
				{
				}
			}
			// m_item.getInputValue() ;
			m_item.m_nValue = position ;
			//
			m_thread = new Thread( new Runnable()
				{
					public void run()
					{
						m_dialog.callbackItem
							( m_item, UICustomDialogInterface.codeOnChanged ) ;
					}
				} ) ;
			m_thread.start() ;
		}
		@Override
		public void onNothingSelected (AdapterView<?> parent)
		{
		}
	}

}

