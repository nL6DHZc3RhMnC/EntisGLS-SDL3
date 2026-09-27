package com.entis.android.entisgls4 ;

import java.io.* ;
import java.util.UUID ;
import java.nio.ByteBuffer ;

import android.net.Uri ;
import android.content.Context ;
import android.content.Intent ;
import android.content.res.AssetManager ;
import android.app.Activity ;
import android.app.ActivityManager ;
import android.util.Log ;
import android.os.Environment ;
import android.os.Handler ;
import android.os.Looper ;
import android.telephony.TelephonyManager ;
import android.provider.Settings ;
import android.view.inputmethod.InputMethodManager ;


public class EntisGLS
{
	public static final boolean			DEBUG = false ;

	// ネイティブライブラリ
	protected static boolean			m_loadedEntisGLS4 = false ;

	// メイン
	protected static EntisGLActivity	m_activity = null ;
	protected static Context			m_context = null ;
	public static Looper				m_looper = null ;
	protected static EntisGLSurfaceView	m_glsufview = null ;

	protected static ActivityManager	m_actManager = null ;
	protected static ActivityManager.MemoryInfo
										m_meminfActivity = new ActivityManager.MemoryInfo() ;

	protected static long				m_idPrimaryThread = 0 ;


	// 初期化
	//////////////////////////////////////////////////////////////////////////
	public static void initialize()
	{
		Thread	threadCurrent = Thread.currentThread() ;
		if ( threadCurrent != null )
		{
			m_idPrimaryThread = threadCurrent.getId() ;
		}
		m_looper = Looper.getMainLooper() ;
		//
		loadNativeLibrary() ;
	}

	// 終了
	//////////////////////////////////////////////////////////////////////////
	public static void close()
	{
	}

	// CPU ネイティブコードをロードする
	//////////////////////////////////////////////////////////////////////////
	public static final void loadNativeLibrary()
	{
		try
		{
			if ( !m_loadedEntisGLS4 )
			{
				System.loadLibrary( "entisgls4" ) ;
				m_loadedEntisGLS4 = true ;
			}
		}
		catch ( Throwable e )
		{
			m_loadedEntisGLS4 = false ;
			EntisGLS.logError( "exception at loadLibrary entisgls4" ) ;
			if ( e != null )
			{
				EntisGLS.logError( e.getMessage() ) ;
			}
		}
	}
	public static final boolean isLoadedNativeLibrary()
	{
		return	m_loadedEntisGLS4 ;
	}

	// アクティビティ
	//////////////////////////////////////////////////////////////////////////
	public static void setActivity( EntisGLActivity act )
	{
		if ( (m_activity != null) && (m_activity != act) && (act != null) )
		{
			EntisGLS.logDebug( "another Activity" ) ;
		}
		m_activity = act ;
		//
		if ( act != null )
		{
			Object	objService =
					act.getSystemService( Activity.ACTIVITY_SERVICE ) ;
			if ( objService instanceof ActivityManager )
			{
				m_actManager = (ActivityManager) objService ;
			}
			m_context = act ;
		}
	}
	public static EntisGLActivity getActivity()
	{
		return	m_activity ;
	}
	public static void setContext( Context ctx )
	{
		m_context = ctx ;
	}
	public static Context getContext()
	{
		if ( m_activity != null )
		{
			return	m_activity ;
		}
		return	m_context ;
	}
	public static String getPackageName()
	{
		Context	ctx = getContext() ;
		if ( ctx != null )
		{
			return	ctx.getPackageName() ;
		}
		return	null ;
	}
	public static String getActivityClassName()
	{
		if ( m_activity != null )
		{
			return	m_activity.getClass().getName() ;
		}
		return	null ;
	}
	public static String getLocalFilesDirectoryPath()
	{
		try
		{
			Context	ctx = getContext() ;
			if ( ctx != null )
			{
				File	file = ctx.getFilesDir() ;
				if ( file != null )
				{
					return	file.getAbsolutePath() ;
				}
			}
		}
		catch ( Exception e )
		{
			EntisGLS.logDebug( "Exception at getLocalFilesDirectoryPath" ) ;
		}
		return	null ;
	}
	public static String getExternalStorageDirectoryPath()
	{
		try
		{
			File	file = Environment.getExternalStorageDirectory() ;
			if ( file != null )
			{
				return	file.getAbsolutePath() ;
			}
		}
		catch ( Exception e )
		{
			EntisGLS.logDebug( "Exception at getExternalStorageDirectoryPath" ) ;
		}
		return	null ;
	}
	public static String getExternalStoragePrivatePath()
	{
		try
		{
			Context	ctx = getContext() ;
			if ( ctx != null )
			{
				File	file = ctx.getExternalFilesDir( null ) ;
				if ( file != null )
				{
					return	file.getAbsolutePath() ;
				}
			}
		}
		catch ( Exception e )
		{
			EntisGLS.logDebug( "Exception at getExternalStoragePrivatePath" ) ;
		}
		return	null ;
	}
	public static void activateActivity( Class clsActivity )
	{
		if ( clsActivity == null )
		{
			if ( m_activity == null )
			{
				return ;
			}
			clsActivity = m_activity.getClass() ;
		}
		Context	ctx = getContext() ;
		if ( ctx != null )
		{
			Intent	intent = new Intent( ctx, clsActivity ) ;
			intent.addFlags( Intent.FLAG_ACTIVITY_SINGLE_TOP ) ;
			intent.addFlags( Intent.FLAG_ACTIVITY_NEW_TASK ) ;
			ctx.startActivity( intent ) ;
		}
	}

	// メインビュー
	//////////////////////////////////////////////////////////////////////////
	public static void setMainSurfaceView( EntisGLSurfaceView glsufview )
	{
		m_glsufview = glsufview ;
	}
	public static EntisGLSurfaceView getMainSurfaceView()
	{
		return	m_glsufview ;
	}
	public static void postUpdateView()
	{
		m_glsufview.requestRender() ;
	}

	// ログ出力
	//////////////////////////////////////////////////////////////////////////
	public static void logVerbose( String log )
	{
		if ( log != null )
		{
	        Log.v( "EntisGLS", log ) ;
		}
	}
	public static void logDebug( String log )
	{
		if ( log != null )
		{
	        Log.d( "EntisGLS", log ) ;
		}
	}
	public static void logInfo( String log )
	{
		if ( log != null )
		{
	        Log.i( "EntisGLS", log ) ;
		}
	}
	public static void logWarning( String log )
	{
		if ( log != null )
		{
	        Log.w( "EntisGLS", log ) ;
		}
	}
	public static void logError( String log )
	{
		if ( log != null )
		{
			Log.e( "EntisGLS", log ) ;
		}
	}
	public static void trace( String text )
	{
		if ( DEBUG && (text != null) )
		{
			if ( m_glsufview != null )
			{
				m_glsufview.printConsole( text + "\n" ) ;
			}
		}
	}
	public static void traceError( String text )
	{
		if ( DEBUG && (text != null) )
		{
			if ( m_glsufview != null )
			{
				m_glsufview.printConsole( text + "\n" ) ;
			}
	        Log.e( "EntisGLS", text ) ;
		}
	}
	public static void traceLog( String text )
	{
		if ( DEBUG && (text != null) )
		{
			if ( m_glsufview != null )
			{
				m_glsufview.printConsole( text + "\n" ) ;
			}
	        Log.i( "EntisGLS", text ) ;
		}
	}

	// 疑似コンソール
	//////////////////////////////////////////////////////////////////////////
	public static void consoleOutput( String text )
	{
		if ( text != null )
		{
			logDebug( "consoleOutput " + text ) ;
			if ( m_glsufview != null )
			{
				m_glsufview.printConsole( text ) ;
			}
		}
	}
	public static String consoleInput()
	{
		String	strCurConsole = null ;
		if ( m_glsufview != null )
		{
			strCurConsole = m_glsufview.getConsoleLastLine() ;
		}
		if ( strCurConsole != null )
		{
			while ( strCurConsole.length() > 0 )
			{
				int		len = strCurConsole.length() ;
				char	c = strCurConsole.charAt( len - 1 ) ;
				if ( c <= 0x20 )
				{
					strCurConsole = strCurConsole.substring( 0, len - 1 ) ;
				}
				else
				{
					break ;
				}
			}
			if ( strCurConsole.length() == 0 )
			{
				strCurConsole = null ;
			}
		}
		String	strInput =
			doMessageEditBox
				( "コンソール入力", strCurConsole, null, UIMessageBox.styleOk ) ;
		if ( (strInput != null) && (strInput.length() > 0) )
		{
			if ( strInput.charAt( strInput.length() - 1 ) != '\n' )
			{
				strInput += "\n" ;
			}
			consoleOutput( strInput ) ;
		}
		else
		{
			consoleOutput( "\n" ) ;
		}
		return	strInput ;
	}

	// UIスレッドで実行する
	//////////////////////////////////////////////////////////////////////////
	public static boolean procedureOnUIThread( Runnable r )
	{
		if ( isPrimaryThread() )
		{
			r.run() ;
			return	true ;
		}
		else
		{
			SyncRunnable	sr = new SyncRunnable( r ) ;
			Handler			handler = new Handler( m_looper ) ;
			if ( handler.post( sr ) )
			{
				return	sr.waitDone() ;
			}
			return	false ;
		}
	}
	public static boolean procedureOnUIThread( Runnable r, boolean fSync )
	{
		if ( isPrimaryThread() )
		{
			r.run() ;
			return	true ;
		}
		else if ( fSync )
		{
			return	procedureOnUIThread( r ) ;
		}
		else
		{
			Handler	handler = new Handler( m_looper ) ;
			return	handler.post( r ) ;
		}
	}
	public static boolean callNativeOnUIThread( ByteBuffer buf )
	{
		return	procedureOnUIThread( new NativeRunnable( buf ), false ) ;
	}
	// プライマリスレッド（UIスレッド）判定
	public static boolean isPrimaryThread()
	{
		Thread	threadCurrent = Thread.currentThread() ;
		if ( threadCurrent != null )
		{
			if ( m_idPrimaryThread == threadCurrent.getId() )
			{
				return	true ;
			}
		}
		return	false ;
	}

	// レンダリングスレッドで実行する
	//////////////////////////////////////////////////////////////////////////
	public static boolean procedureOnRenderingThread( Runnable r )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.procedureOnRenderingThread( r ) ;
		}
		return	false ;
	}
	public static boolean procedureOnRenderingThread
					( Runnable r, boolean fSync, boolean fDelay )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.procedureOnRenderingThread( r, fSync, fDelay ) ;
		}
		return	false ;
	}
	public static boolean callNativeOnRenderingThread( ByteBuffer buf, boolean fDelay )
	{
		return	procedureOnRenderingThread( new NativeRunnable( buf ), !fDelay, fDelay ) ;
	}
	// GLスレッド判定
	public static boolean isGLThread()
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.isGLThread() ;
		}
		return	false ;
	}

	// 非描画レンダリングスレッドで実行する
	//////////////////////////////////////////////////////////////////////////
	public static boolean procedureAsyncNoRenderingThread( Runnable r )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.procedureAsyncNoRenderingThread( r ) ;
		}
		return	false ;
	}
	public static boolean callNativeAsyncNoRenderingThread( ByteBuffer buf )
	{
		return	procedureAsyncNoRenderingThread( new NativeRunnable( buf ) ) ;
	}

	// メモリ情報
	//////////////////////////////////////////////////////////////////////////
	public static long sizeOfAvailableMemory()
	{
		if ( m_actManager != null )
		{
			m_actManager.getMemoryInfo( m_meminfActivity ) ;
			return	m_meminfActivity.availMem ;
		}
		return	0 ;
	}
	public static boolean isLowMemory()
	{
		if ( m_actManager != null )
		{
			m_actManager.getMemoryInfo( m_meminfActivity ) ;
			return	m_meminfActivity.lowMemory  ;
		}
		return	false ;
	}
	public static long sizeOfTotalMemory()
	{
		try
		{
		#if	ANDROID_API_LEVEL >= 16
			if ( m_actManager != null )
			{
				m_actManager.getMemoryInfo( m_meminfActivity ) ;
				return	m_meminfActivity.totalMem ;
			}
		#endif
			Runtime		runtime = Runtime.getRuntime() ;
			if ( runtime != null )
			{
				return	runtime.totalMemory() ;
			}
		}
		catch ( Throwable e )
		{
		}
		return	0 ;
	}

	// assets ファイル
	//////////////////////////////////////////////////////////////////////////
	public static InputStream openAssetFile( String filepath )
	{
		try
		{
			AssetManager	asm = getContext().getAssets() ;
			if ( asm != null )
			{
				return	asm.open( filepath, AssetManager.ACCESS_RANDOM ) ;
			}
		}
		catch ( IOException e )
		{
			logVerbose( "failed to open \"" + filepath + "\" of asset" ) ;
	        logVerbose( e.getMessage() ) ;
		}
		return	null ;
	}
	public static String[] listAssetFiles( String filepath )
	{
		try
		{
			AssetManager	asm = getContext().getAssets() ;
			if ( asm != null )
			{
				return	asm.list( filepath ) ;
			}
		}
		catch ( IOException e )
		{
			logVerbose( "failed to list \"" + filepath + "\" of asset" ) ;
	        logVerbose( e.getMessage() ) ;
		}
		return	null ;
	}

	// 固有値
	//////////////////////////////////////////////////////////////////////////
	// ANDROID_ID
	public static String getAndroidId()
	{
		try
		{
			return	Settings.Secure.getString
				( getContext().getContentResolver(), Settings.Secure.ANDROID_ID ) ;
		}
		catch ( Throwable e )
		{
		}
		return	null ;
	}
	// 携帯端末固有ID (SIM必須)
	public static String getDeviceId()
	{
		try
		{
			TelephonyManager tm =
				(TelephonyManager)
					getContext().getSystemService( Context.TELEPHONY_SERVICE ) ;
			if ( tm != null )
			{
				return	tm.getDeviceId() ;
			}
		}
		catch ( Throwable e )
		{
		}
		return	null ;
	}
	// 電話番号 (SIM必須)
	public static String getPhoneNumber()
	{
		try
		{
			TelephonyManager tm =
				(TelephonyManager)
					getContext().getSystemService( Context.TELEPHONY_SERVICE ) ;
			if ( tm != null )
			{
				return	tm.getLine1Number() ;
			}
		}
		catch ( Throwable e )
		{
		}
		return	null ;
	}
	// ハードウェアシリアル
	public static String getSerialNumber()
	{
	#if	ANDROID_API_LEVEL >= 9
		return	android.os.Build.SERIAL ;
	#else
		return	getAndroidId() ;
	#endif
	}
	// API level
	public static int getAPILevel()
	{
		return	android.os.Build.VERSION.SDK_INT ;
	}
	// OS version
	public static String getOSVersionString()
	{
		return	android.os.Build.VERSION.RELEASE ;
	}
	// ランダム UUID
	public static String generateRandomUUID()
	{
		return	UUID.randomUUID().toString() ;
	}

	// Intent 起動
	//////////////////////////////////////////////////////////////////////////
	public static boolean intentFileView
		( String sPath, int iAct, String sPackage, String sClass )
	{
		try
		{
			Intent	i = new Intent( Intent.ACTION_VIEW, Uri.parse( sPath ) ) ;
			if ( (sPackage != null) && (sClass != null) )
			{
				i.setClassName( sPackage, sClass ) ;
			}
			Context	ctx = getContext() ;
			if ( ctx != null )
			{
				ctx.startActivity( i ) ;
				return	true ;
			}
		}
		catch ( Throwable e )
		{
		}
		return	false ;
	}

	// ソフトキーボード表示
	//////////////////////////////////////////////////////////////////////////
	public static void showSoftKeyboard()
	{
		Handler	handler = new Handler( m_looper ) ;
		handler.post( new ShowSoftKeyboardRunnable() ) ;
	}
	protected static class	ShowSoftKeyboardRunnable	implements Runnable
	{
		public void run()
		{
			Context	ctx = getContext() ;
			if ( ctx == null )
			{
				return ;
			}
			EntisGLSurfaceView	view = getMainSurfaceView() ;
			if ( view == null )
			{
				return ;
			}
			InputMethodManager	imm =
				(InputMethodManager) ctx.getSystemService
										( Context.INPUT_METHOD_SERVICE ) ;
			view.requestFocus() ;
			imm.showSoftInput( view, InputMethodManager.SHOW_FORCED ) ;
		}
	}

	// メッセージボックス
	//////////////////////////////////////////////////////////////////////////
	public static int doMessageBox( String title, String msg, int style )
	{
		UIMessageBox	uiMsgBox = new UIMessageBox( title, msg, style ) ;
		procedureOnUIThread( uiMsgBox ) ;
		uiMsgBox.waitDone() ;
		return	uiMsgBox.getResult() ;
	}
	public static String doMessageEditBox( String title, String msg, String edit, int style )
	{
		UIMessageEditBox	uiMsgBox = new UIMessageEditBox( title, msg, edit, style ) ;
		procedureOnUIThread( uiMsgBox ) ;
		uiMsgBox.waitDone() ;
		return	uiMsgBox.getEditString() ;
	}

	// ファイル選択ダイアログ
	//////////////////////////////////////////////////////////////////////////
	public static String doBrowseDirectoryDialog
					( String title, String sInitDir, int nFlags )
	{
		UIFileChoiceDialog	uiDialog =
			new UIFileChoiceDialog( title, sInitDir, null ) ;
		uiDialog.setChoiceStyle
			( UIFileChoiceDialog.styleOpenDirectory | nFlags ) ;
		uiDialog.addPositiveButton( "OK", UIMessageBox.resultOk ) ;
		uiDialog.addNegativeButton( "CANCEL", UIMessageBox.resultCancel ) ;
		procedureOnUIThread( uiDialog ) ;
		uiDialog.waitDone() ;
		if ( uiDialog.getResult() != UIMessageBox.resultOk )
		{
			return	null ;
		}
		return	uiDialog.getResultPath() ;
	}
	public static String doOpenFileDialog
		( String title, String sInitDir, String[] aFileExts, int nFlags )
	{
		UIFileChoiceDialog	uiDialog =
			new UIFileChoiceDialog( title, sInitDir, aFileExts ) ;
		uiDialog.setChoiceStyle
			( UIFileChoiceDialog.styleOpenFile | nFlags ) ;
		uiDialog.addNegativeButton( "CANCEL", UIMessageBox.resultCancel ) ;
		procedureOnUIThread( uiDialog ) ;
		uiDialog.waitDone() ;
		if ( uiDialog.getResult() != UIMessageBox.resultOk )
		{
			return	null ;
		}
		return	uiDialog.getResultPath() ;
	}
	public static String doSaveFileDialog
		( String title, String sInitDir, String[] aFileExts, int nFlags )
	{
		UIFileChoiceDialog	uiDialog =
			new UIFileChoiceDialog( title, sInitDir, aFileExts ) ;
		uiDialog.setChoiceStyle
			( UIFileChoiceDialog.styleWriteFile | nFlags ) ;
		uiDialog.addPositiveButton( "OK", UIMessageBox.resultOk ) ;
		uiDialog.addNegativeButton( "CANCEL", UIMessageBox.resultCancel ) ;
		procedureOnUIThread( uiDialog ) ;
		uiDialog.waitDone() ;
		if ( uiDialog.getResult() != UIMessageBox.resultOk )
		{
			return	null ;
		}
		return	uiDialog.getResultPath() ;
	}

}

