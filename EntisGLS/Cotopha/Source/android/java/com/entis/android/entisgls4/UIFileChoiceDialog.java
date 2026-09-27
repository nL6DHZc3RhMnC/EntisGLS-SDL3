package com.entis.android.entisgls4 ;

import java.io.File ;
import java.io.FileFilter ;
import java.util.List ;
import java.util.ArrayList ;
import java.util.Collections ;

import android.app.AlertDialog ;
import android.content.Context ;
import android.content.DialogInterface ;
import android.graphics.Color ;
import android.view.Display ;
import android.view.WindowManager ;
import android.view.View ;
import android.view.ViewGroup ;
import android.widget.ListView ;
import android.widget.ArrayAdapter;
import android.widget.AdapterView ;
import android.widget.LinearLayout ;
import android.widget.TextView ;
import android.widget.EditText ;
import android.text.InputType ;

public class	UIFileChoiceDialog	extends UIMessageBox
{
	// スタイル
	public static final int	styleMsgBoxMask		= 0x00FF ;
	public static final int	styleOpenDirectory	= 0x0100 ;
	public static final int	styleOpenFile		= 0x0200 ;
	public static final int	styleWriteFile		= 0x0400 ;

	// ファイル選択
	protected int		m_styleChoice = 0 ;
	protected String	m_strDirectory = null ;
	protected String[]	m_aFileExts = null ;

	// リニアレイアウト
	protected LinearLayout	m_viewLayout = null ;

	// リスト
	protected ListView	m_listView = null ;
	protected FileArrayAdapter	m_faAdapter = null ;

	// エディット
	protected EditText	m_editFileName = null ;
	protected String	m_strFileName = null ;

	// ボタンエントリ
	//////////////////////////////////////////////////////////////////////////
	public static class	ButtonEntry	extends UIMessageBox.ButtonEntry
	{
		public ButtonEntry( UIDialog dialog, int type, String text, int value )
		{
			super( dialog, type, text, value, true ) ;
		}
		public void onClickButton()
		{
			if ( (m_type == typePositiveButton)
					&& (m_dialog instanceof UIFileChoiceDialog) )
			{
				UIFileChoiceDialog	choice = (UIFileChoiceDialog) m_dialog ;
				if ( choice.m_editFileName != null )
				{
					choice.m_strFileName =
							choice.m_editFileName.getText().toString() ;
					if ( (choice.m_strFileName == null)
						|| choice.m_strFileName.equals("") )
					{
						return ;
					}
				}
			}
			//
			super.onClickButton() ;
			//
			if ( m_dialog != null )
			{
				m_dialog.closeDialog() ;
			}
		}
	}

	// ファイルエントリ
	//////////////////////////////////////////////////////////////////////////
	public static class	FileEntry	implements Comparable<FileEntry>
	{
		public File		m_file = null ;
		public String	m_filename = null ;
		public boolean	m_directory = false ;

		public FileEntry( File file )
		{
			m_file = file ;
			m_filename = file.getName() ;
			m_directory = file.isDirectory() ;
		}
		public FileEntry( File file, String name )
		{
			m_file = file ;
			m_filename = name ;
			m_directory = file.isDirectory() ;
		}

		public int compareTo( FileEntry another )
		{
			if ( m_directory && !another.m_directory )
			{
				return	-1 ;
			}
			if ( !m_directory && another.m_directory )
			{
				return	1 ;
			}
			return	m_filename.toLowerCase().compareTo
							( another.m_filename.toLowerCase() ) ;
		}
	}

	// リストビュー・アダプタ
	//////////////////////////////////////////////////////////////////////////
	public class FileArrayAdapter	extends ArrayAdapter<FileEntry>
	{
		protected FileEntry[]	m_aFileList ;

		// 構築関数
		public FileArrayAdapter( FileEntry[] aFileList )
		{
			super( EntisGLS.getActivity(), -1, aFileList ) ;
			m_aFileList = aFileList ;
		}

		// 要素取得
		@Override
		public FileEntry getItem( int position )
		{
			if ( (position >= 0) && (m_aFileList != null)
							&& (position < m_aFileList.length) )
			{
				return	m_aFileList[position] ;
			}
			return	null ;
		}

		// ビュー生成
		@Override
		public View getView( int position, View convertView, ViewGroup parent )
		{
			if ( convertView == null )
			{
				// レイアウト
				LinearLayout	layout =
						new LinearLayout( EntisGLS.getActivity() ) ;
				layout.setPadding( 10, 10, 10, 10 ) ;
				layout.setBackgroundColor( Color.WHITE ) ;
				convertView = layout ;

				// テキスト
				TextView	text = new TextView( EntisGLS.getActivity() ) ;
				text.setTag( "text" );
				text.setTextColor( Color.BLACK ) ;
				text.setPadding( 10, 10, 10, 10 ) ;
				layout.addView( text ) ;
			}

			// 表示文字列設定
			FileEntry	feEntry = getItem( position ) ;
			TextView	text =
					(TextView) convertView.findViewWithTag( "text" ) ;
			if ( feEntry.m_directory )
			{
				text.setText( feEntry.m_filename + "/" ) ;
			}
			else
			{
				text.setText( feEntry.m_filename ) ;
			}
			return	convertView ;
		}
	}

	// リスト・クリック・リスナ
	//////////////////////////////////////////////////////////////////////////
	public class	ListItemClickListener
						implements AdapterView.OnItemClickListener
	{
		public void onItemClick
			( AdapterView<?> l, View v, int position, long id )
		{
			FileEntry feEntry = m_faAdapter.getItem( position ) ;
			if ( feEntry.m_directory )
			{
				m_strDirectory = feEntry.m_file.getAbsolutePath() ;
				m_faAdapter = createArrayAdapter() ;
				m_listView.setAdapter( m_faAdapter ) ;
				m_dialog.setMessage( m_strDirectory ) ;
				if ( m_editFileName != null )
				{
					m_editFileName.setText( "" ) ;
				}
			}
			else
			{
				if ( (m_styleChoice & styleWriteFile) != 0 )
				{
					if ( m_editFileName != null )
					{
						m_editFileName.setText( feEntry.m_filename ) ;
					}
				}
				else
				{
					setResult( feEntry.m_filename, resultOk ) ;
					m_dialog.dismiss() ;
				}
			}
		}
	}

	// ファイルフィルタ
	//////////////////////////////////////////////////////////////////////////
	public class	ChoiceFileFilter	implements FileFilter
	{
		public boolean accept( File path )
		{
			if ( path.isDirectory() )
			{
				return	true ;
			}
			else if ( (m_styleChoice & styleOpenDirectory) != 0 )
			{
				return	false ;
			}
			if ( m_aFileExts == null )
			{
				return	true ;
			}
			for ( int i = 0; i < m_aFileExts.length; i ++ )
			{
				if ( path.getName().toLowerCase().
								endsWith( "." + m_aFileExts[i] ) )
				{
					return	true ;
				}
			}
			return	false ;
		}
	}

	// 構築関数
	//////////////////////////////////////////////////////////////////////////
	public UIFileChoiceDialog
		( String title, String sInitDir, String[] aFileExts )
	{
		super( title, sInitDir ) ;
		m_strDirectory = sInitDir ;
		m_aFileExts = aFileExts ;
	}
	public UIFileChoiceDialog
		( String title, String sInitDir, String[] aFileExts, int style )
	{
		super( title, sInitDir, style ) ;
		m_styleChoice = style ;
		m_strDirectory = sInitDir ;
		m_aFileExts = aFileExts ;
	}

	// ファイル選択スタイル設定
	//////////////////////////////////////////////////////////////////////////
	void setChoiceStyle( int style )
	{
		m_styleChoice = style ;
	}

	// ボタン追加
	//////////////////////////////////////////////////////////////////////////
	@Override
	public void addButton( int type, String text, int value )
	{
		m_vecButtons.add( new ButtonEntry( this, type, text, value ) ) ;
	}

	// メッセージボックス生成
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onBeforeDialogCreate( AlertDialog.Builder builder )
	{
		super.onBeforeDialogCreate( builder ) ;
		//
		m_viewLayout = new LinearLayout( EntisGLS.getActivity() ) ;
		m_viewLayout.setOrientation( LinearLayout.VERTICAL ) ;
		//
		Display	display =
			EntisGLS.getActivity().getWindowManager().getDefaultDisplay() ;
		int	nListViewHeight = 200 ;
		if ( display != null )
		{
			nListViewHeight += (display.getHeight() - 480) / 2 ;
		}
		ViewGroup.LayoutParams	layoutList =
			new ViewGroup.LayoutParams
				( ViewGroup.LayoutParams.FILL_PARENT, nListViewHeight ) ;
		m_listView = new ListView( EntisGLS.getActivity() ) ;
		m_listView.setScrollingCacheEnabled( false ) ;
		m_listView.setOnItemClickListener( new ListItemClickListener() ) ;
		m_faAdapter = createArrayAdapter() ;
		m_listView.setAdapter( m_faAdapter ) ;
		m_viewLayout.addView( m_listView, layoutList ) ;
		//
		if ( (m_styleChoice & styleWriteFile) != 0 )
		{
			ViewGroup.LayoutParams	layoutEdit =
				new ViewGroup.LayoutParams
					( ViewGroup.LayoutParams.FILL_PARENT,
						ViewGroup.LayoutParams.WRAP_CONTENT ) ;
			m_editFileName = new EditText( EntisGLS.getActivity() ) ;
			m_editFileName.setInputType( InputType.TYPE_CLASS_TEXT ) ;
			m_viewLayout.addView( m_editFileName, layoutEdit ) ;
		}
		//
		builder.setView( m_viewLayout ) ;
		builder.setMessage( m_strDirectory ) ;
	}

	// アダプタ生成
	//////////////////////////////////////////////////////////////////////////
	protected FileArrayAdapter createArrayAdapter()
	{
		return	new FileArrayAdapter( getFileList() ) ;
	}

	// ファイルリスト取得
	//////////////////////////////////////////////////////////////////////////
	protected FileEntry[] getFileList()
	{
		//
		// 有効なディレクトリへ移動
		//
		File	fileDir = null ;
		for ( ; ; )
		{
			if ( m_strDirectory == null )
			{
				m_strDirectory = "/" ;
				fileDir = new File( "/" ) ;
				break ;
			}
			fileDir = new File( m_strDirectory ) ;
			if ( fileDir.exists() )
			{
				break ;
			}
			m_strDirectory = fileDir.getParent() ;
		}
		//
		// ファイル列挙
		//
		List<FileEntry>	listFile = new ArrayList<FileEntry>() ;
		File[]	aFiles = fileDir.listFiles( (FileFilter) new ChoiceFileFilter() ) ;
		if ( aFiles != null )
		{
			for ( int i = 0; i < aFiles.length; i ++ )
			{
				listFile.add( new FileEntry( aFiles[i] ) ) ;
			}
			Collections.sort( listFile ) ;
		}
		//
		// 親ディレクトリ
		//
		String	sParentDir = fileDir.getParent() ;
		if ( sParentDir != null )
		{
			listFile.add( 0, new FileEntry( new File( sParentDir ), ".." ) ) ;
		}
		//
		// ファイル列挙
		//
		return	listFile.toArray( new FileEntry[listFile.size()] ) ;
	}

	// 結果設定
	protected synchronized void setResult( String strFileName, int nResult )
	{
		m_strFileName = strFileName ;
		m_nResult = nResult ;
		m_flagDone = true ;
		notifyAll() ;
	}

	// 選択ファイルパス取得
	public String getResultPath()
	{
		if ( (m_styleChoice & styleOpenDirectory) != 0 )
		{
			return	m_strDirectory ;
		}
		if ( (m_strDirectory != null)
			&& m_strDirectory.endsWith( "/" ) )
		{
			return	m_strDirectory + m_strFileName ;
		}
		return	m_strDirectory + "/" + m_strFileName ;
	}
}

