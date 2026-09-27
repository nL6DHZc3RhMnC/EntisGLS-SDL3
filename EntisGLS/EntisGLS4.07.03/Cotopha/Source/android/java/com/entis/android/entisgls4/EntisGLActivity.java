package com.entis.android.entisgls4 ;

import android.app.Activity ;
import android.content.Intent ;
import android.os.Bundle ;
import android.view.* ;
import android.widget.FrameLayout ;

#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_BILLING < 3) && (ANDROID_BILLING > 0)
import java.util.List ;
import com.entis.android.entisgls4.util.IabHelper ;
import com.entis.android.entisgls4.util.IabResult ;
import com.entis.android.entisgls4.util.Inventory ;
import com.entis.android.entisgls4.util.Purchase ;

#elseif (ANDROID_BILLING >= 3)
import java.util.List ;

#endif


public class EntisGLActivity extends Activity
								implements Runnable
{
	protected static EntisGLSurfaceView	m_glsufview = null ;
	protected FrameLayout				m_layout = null ;
	protected FrameLayout.LayoutParams	m_layoutParam = null ;

	protected boolean					m_flagSurfaceCreated = false ;
	protected boolean					m_flagAutoStart = true ;
	protected Thread					m_thread = null ;
	protected String					m_argVM = null ;
	protected boolean					m_flagAbortThread = false ;


	//////////////////////////////////////////////////////////////////////////
	// アクティビティが生成された
	//////////////////////////////////////////////////////////////////////////
	@Override
    protected void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState ) ;
		//
		// ライブラリの初期化
		//
		EntisGLS.initialize() ;
		EntisGLS.setActivity( this ) ;
		EntisGLS.logInfo( "onCreate" ) ;
		//
		// ビューの設定
		//
		getWindow().addFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN ) ;
		getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON ) ;
		requestWindowFeature( Window.FEATURE_NO_TITLE ) ;
		//
		m_glsufview = createGLSurfaceView() ;
		m_layoutParam =
				new FrameLayout.LayoutParams
					( FrameLayout.LayoutParams.FILL_PARENT,
							FrameLayout.LayoutParams.FILL_PARENT ) ;
		m_layout = new FrameLayout( this ) ;
		m_layout.addView( m_glsufview, m_layoutParam ) ;
		setContentView( m_layout ) ;
		EntisGLS.setMainSurfaceView( m_glsufview ) ;
		//
		// 引数取得
		//
		m_argVM = getIntent().getStringExtra( "ARGUMENT" ) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// アクティビティ開始
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onStart()
	{
		EntisGLS.logInfo( "onStart" ) ;
		super.onStart() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 停止後の再スタート
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onRestart()
	{
		EntisGLS.logInfo( "onRestart" ) ;
		super.onRestart() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 動作開始
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onResume()
	{
		EntisGLS.logInfo( "onResume" ) ;
		super.onResume() ;
		//
		if ( m_flagAutoStart )
		{
			beginMainThread() ;
		}
	}

	protected void beginMainThread()
	{
		if ( m_glsufview != null )
		{
			m_glsufview.onBeginView() ;
		}
		if ( m_thread == null )
		{
			EntisGLS.logInfo( "begin main thread" ) ;
			m_thread = new Thread( this ) ;
			m_thread.start() ;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 一時停止
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onPause()
	{
		EntisGLS.logInfo( "onPause" ) ;
		//
		if ( m_glsufview != null )
		{
			m_glsufview.onEndView() ;
		}
		super.onPause() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 停止
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onStop()
	{
		EntisGLS.logInfo( "onStop" ) ;
		super.onStop() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 破棄
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onDestroy()
	{
		EntisGLS.logInfo( "onDestroy" ) ;
		if ( m_glsufview != null )
		{
			m_glsufview.onDestroy() ;
		}
		if ( m_thread != null )
		{
			m_flagAbortThread = true ;
			nativeAbort() ;
			//
			synchronized( this )
			{
				long	msecStart = System.currentTimeMillis() ;
				try
				{
					while ( m_thread != null )
					{
						wait( 100 ) ;
						if ( System.currentTimeMillis() - msecStart > 1000 )
						{
							EntisGLS.logInfo( "timeout abort native thread" ) ;
							break ;
						}
					}
				}
				catch ( Throwable e )
				{
				}
			}
		}
#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_BILLING < 3) && (ANDROID_BILLING > 0)
		if ( m_IabHelper != null )
		{
			m_IabHelper.dispose() ;
			m_IabHelper = null ;
		}
#elseif (ANDROID_BILLING >= 3)
		if ( m_billing != null )
		{
			m_billing.endBilling() ;
		}
#endif
		EntisGLS.close() ;
		super.onDestroy() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// 復元処理
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onRestoreInstanceState( Bundle savedInstanceState )
	{
		EntisGLS.logInfo( "onRestoreInstanceState" ) ;
		super.onRestoreInstanceState(savedInstanceState);
	}

	//////////////////////////////////////////////////////////////////////////
	// Activity 結果受け取り
	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onActivityResult
			( int requestCode, int resultCode, Intent data )
	{
#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_BILLING < 3) && (ANDROID_BILLING > 0)
		if ( (m_IabHelper != null)
			&& m_IabHelper.handleActivityResult
						( requestCode, resultCode, data ) )
		{
			return ;
		}
#endif
		super.onActivityResult( requestCode, resultCode, data ) ;
	}

    //////////////////////////////////////////////////////////////////////////
	// ハードウェアボタン
	//////////////////////////////////////////////////////////////////////////
	@Override
	public boolean dispatchKeyEvent( KeyEvent kev )
	{
		final int	act = kev.getAction() ;
		if ( act == KeyEvent.ACTION_DOWN )
		{
			if ( onSystemKeyDown( kev.getKeyCode() ) )
			{
				return	false ;
			}
		}
		else if ( act == KeyEvent.ACTION_UP )
		{
			if ( onSystemKeyUp( kev.getKeyCode() ) )
			{
				return	false ;
			}
		}
		return	super.dispatchKeyEvent( kev ) ;
	}
	public boolean onSystemKeyDown( int key )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.onSystemKeyDown( key ) ;
		}
		return	false ;
	}
	public boolean onSystemKeyUp( int key )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.onSystemKeyUp( key ) ;
		}
		return	false ;
	}

	//////////////////////////////////////////////////////////////////////////
	// メニュー
	//////////////////////////////////////////////////////////////////////////
	@Override
	public boolean onCreateOptionsMenu( Menu menu )
	{
		return	true ;
	}

	@Override
	public boolean onPrepareOptionsMenu( Menu menu )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.onPrepareOptionsMenu( menu ) ;
		}
		return	false ;
	}

	@Override
	public boolean onOptionsItemSelected( MenuItem item )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.onMenuItemSelected( item ) ;
		}
		return	false ;
	}

	@Override
	public boolean onContextItemSelected( MenuItem item )
	{
		if ( m_glsufview != null )
		{
			return	m_glsufview.onMenuItemSelected( item ) ;
		}
		return	false ;
	}

	//////////////////////////////////////////////////////////////////////////
	// サーフェスが生成された時の処理
	//////////////////////////////////////////////////////////////////////////
	public synchronized void notifySurfaceCreated()
	{
		m_flagSurfaceCreated = true ;
		notifyAll() ;
	}

	//////////////////////////////////////////////////////////////////////////
	// GLSurfaceView を生成する
	//////////////////////////////////////////////////////////////////////////
	public EntisGLSurfaceView createGLSurfaceView()
	{
		return	new EntisGLSurfaceView( this ) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// GLSurfaceView を取得する
	//////////////////////////////////////////////////////////////////////////
	public static EntisGLSurfaceView getGLSurfaceView()
	{
		return	m_glsufview ;
	}

	//////////////////////////////////////////////////////////////////////////
	// ビューを追加する
	//////////////////////////////////////////////////////////////////////////
	public void addView( View child, int index )
	{
		m_layout.addView( child, index, m_layoutParam ) ;
	}

	//////////////////////////////////////////////////////////////////////////
	// ビューを削除する
	//////////////////////////////////////////////////////////////////////////
	public void removeView( View child )
	{
		m_layout.removeView( child ) ;
	}


	//////////////////////////////////////////////////////////////////////////
	// タイマ処理
	//////////////////////////////////////////////////////////////////////////

	protected class	ViewTimerRunnable	implements Runnable
	{
		protected boolean		m_flagExit = false ;

		@Override
		public void run()
		{
			while ( !m_flagExit )
			{
				m_glsufview.onTimer() ;
				try
				{
					Thread.sleep( 16 ) ;
				}
				catch ( Exception e )
				{
				}
			}
		}
		public void exit()
		{
			m_flagExit = true ;
		}
	}
	protected SyncRunnable		m_srTimer = null ;
	protected ViewTimerRunnable	m_vtrTimer = null ;
	protected Thread			m_threadTimer = null ;


	//////////////////////////////////////////////////////////////////////////
	// 実行スレッド
	//////////////////////////////////////////////////////////////////////////
	final public void run()
	{
		try
		{
			synchronized( this )
			{
				while ( !m_flagSurfaceCreated )
				{
					wait() ;
				}
			}
			m_vtrTimer = new ViewTimerRunnable() ;
			m_srTimer = new SyncRunnable( m_vtrTimer ) ;
			m_threadTimer = new Thread( m_srTimer ) ;
			m_threadTimer.start() ;
			//
			if ( EntisGLS.isLoadedNativeLibrary() )
			{
				nativeMain( m_argVM ) ;
				EntisGLS.logDebug( "finished nativeMain" ) ;
			}
			else
			{
				EntisGLS.doMessageBox
					( "エラー", "実行に対応していない CPU です。",
												UIMessageBox.styleOk ) ;
			}
		}
		catch ( Throwable e )
		{
			EntisGLS.logError( "exception in nativeMain" ) ;
			if ( e != null )
			{
				EntisGLS.logError( e.getMessage() ) ;
			}
		}
		if ( m_threadTimer != null )
		{
			m_vtrTimer.exit() ;
			m_srTimer.waitDone() ;
			m_vtrTimer = null ;
			m_srTimer = null ;
			m_threadTimer = null ;
		}
		synchronized( this )
		{
			m_thread = null ;
			notifyAll() ;
		}
		if ( !m_flagAbortThread )
		{
			finish() ;
		}
	}
	protected native int nativeMain( String arg ) ;
	protected native int nativeAbort() ;

#if	(ANDROID_API_LEVEL >= 8) && (ANDROID_BILLING < 3) && (ANDROID_BILLING > 0)

	//////////////////////////////////////////////////////////////////////////
	// Google Play 支払い API
	//////////////////////////////////////////////////////////////////////////

	private IabHelper	m_IabHelper = null ;
	protected boolean	m_flagIabSetupFinished = false ;
	protected boolean	m_flagIabSetupSuccessed = false ;

	// 開始
	//////////////////////////////////////////////////////////////////////////
	public boolean startupBilling( String base64EncodedPublicKey )
	{
		if ( m_IabHelper == null )
		{
			m_IabHelper = new IabHelper( this, base64EncodedPublicKey ) ;
			m_IabHelper.enableDebugLogging( EntisGLS.DEBUG ) ;
			m_IabHelper.startSetup
				( new IabHelper.OnIabSetupFinishedListener()
				{
					public void onIabSetupFinished(IabResult result)
					{
						EntisGLS.logDebug( "in-app billing setup finished.") ;
						synchronized( EntisGLS.getActivity() )
						{
							m_flagIabSetupFinished = true ;
							if ( !result.isSuccess() )
							{
								m_flagIabSetupSuccessed = false ;
							    EntisGLS.logError
									( "error in-app billing setup: " + result ) ;
							}
							else
							{
								m_flagIabSetupSuccessed = true ;
							}
							EntisGLS.getActivity().notifyAll() ;
						}
					}
				} ) ;
		}
		if ( EntisGLS.isPrimaryThread() )
		{
			return	m_flagIabSetupSuccessed ;
		}
		return	waitStartupBilling() ;
	}
	public synchronized boolean waitStartupBilling()
	{
		while ( !m_flagIabSetupFinished )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_flagIabSetupSuccessed ;
	}

	// 購入済みリスト取得
	//////////////////////////////////////////////////////////////////////////
	protected boolean			m_flagGotInventory = false ;
	protected List<Purchase>	m_listGotInventory = null ;

	private IabHelper.QueryInventoryFinishedListener
		m_listenerGotInventory =
			new IabHelper.QueryInventoryFinishedListener()
		{
			public void onQueryInventoryFinished
							( IabResult result, Inventory inventory )
			{
				synchronized( EntisGLS.getActivity() )
				{
					m_flagGotInventory = true ;
					if ( (m_IabHelper == null) || result.isFailure() )
					{
						m_listGotInventory = null ;
					}
					else
					{
						m_listGotInventory = inventory.getAllPurchases() ;
					}
					EntisGLS.getActivity().notifyAll() ;
				}
			}
		} ;

	public synchronized List<Purchase> queryInventory()
	{
		m_flagGotInventory = false ;
		//
		EntisGLS.procedureOnUIThread( new Runnable()
			{
				public void run()
				{
					m_IabHelper.queryInventoryAsync( m_listenerGotInventory ) ;
				}
			}, false ) ;
		//
		while ( !m_flagGotInventory )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_listGotInventory ;
	}

	// アプリ内購入
	//////////////////////////////////////////////////////////////////////////
	protected boolean	m_flagFinishedPurchase = false ;
	protected boolean	m_flagSuccessedPurchase = false ;
	protected Purchase	m_purchaseItem = null ;

    private IabHelper.OnIabPurchaseFinishedListener
		m_listenerPurchaseFinished =
			new IabHelper.OnIabPurchaseFinishedListener()
		{
			public void onIabPurchaseFinished
						( IabResult result, Purchase purchase )
			{
				synchronized( EntisGLS.getActivity() )
				{
					m_flagFinishedPurchase = true ;
					if ( (m_IabHelper == null) || result.isFailure() )
					{
						m_flagSuccessedPurchase = false ;
						m_purchaseItem = null ;
					}
					else
					{
						m_flagSuccessedPurchase = true ;
						m_purchaseItem = purchase ;
					}
					EntisGLS.getActivity().notifyAll() ;
				}
			}
		} ;

	public synchronized boolean doPurchaseItem( String sku, String payload )
	{
		m_flagFinishedPurchase = false ;
		m_flagSuccessedPurchase = false ;
		m_purchaseItem = null ;
		//
		m_IabHelper.launchPurchaseFlow
			( this, sku, 17321, m_listenerPurchaseFinished, payload ) ;
		//
		return	waitPurchaseFlow() ;
	}
	public synchronized boolean doPurchaseSubscription( String sku, String payload )
	{
		m_flagFinishedPurchase = false ;
		m_flagSuccessedPurchase = false ;
		m_purchaseItem = null ;
		//
		if ( !m_IabHelper.subscriptionsSupported() )
		{
			return	false ;
		}
		//
		m_IabHelper.launchPurchaseFlow
			( this, sku, IabHelper.ITEM_TYPE_SUBS,
				17321, m_listenerPurchaseFinished, payload ) ;
		//
		return	waitPurchaseFlow() ;
	}
	public synchronized boolean waitPurchaseFlow()
	{
		while ( !m_flagFinishedPurchase )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_flagSuccessedPurchase ;
	}
	public synchronized List<Purchase> getPurchaseItemList()
	{
		ArrayList<Purchase>	listPurchase = new ArrayList<Purchase>() ;
		if ( m_purchaseItem != null )
		{
			listPurchase.add( m_purchaseItem ) ;
		}
		return	m_purchaseItem ;
	}

	// 購入アイテム消費
	//////////////////////////////////////////////////////////////////////////
	protected boolean	m_flagFinishedConsume = false ;
	protected boolean	m_flagSuccessedConsume = false ;

	private IabHelper.OnConsumeFinishedListener
		m_listenerConsumeFinished =
			new IabHelper.OnConsumeFinishedListener()
		{
			public void onConsumeFinished
					( Purchase purchase, IabResult result )
			{
				synchronized( EntisGLS.getActivity() )
				{
					m_flagFinishedConsume = true ;
					if ( (m_IabHelper != null) && result.isSuccess() )
					{
						m_flagSuccessedConsume = true ;
					}
					else
					{
						m_flagSuccessedConsume = false ;
					}
					EntisGLS.getActivity().notifyAll() ;
				}
			}
		} ;

	private class BillingConsumeRunnable implements Runnable
	{
		private Purchase	m_purchase ;

		public BillingConsumeRunnable( Purchase purchase )
		{
			m_purchase = purchase ;
		}
		public void run()
		{
			m_IabHelper.consumeAsync( m_purchase, m_listenerConsumeFinished ) ;
		}
	}

	public synchronized boolean doConsumeItem( Purchase purchase )
	{
		if ( purchase == null )
		{
			return	false ;
		}
		m_flagFinishedConsume = false ;
		m_flagSuccessedConsume = false ;
		//
		EntisGLS.procedureOnUIThread
			( new BillingConsumeRunnable( purchase ), false ) ;
		//
		while ( !m_flagFinishedConsume )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_flagSuccessedConsume ;
	}

#elseif	(ANDROID_BILLING >= 3)

	//////////////////////////////////////////////////////////////////////////
	// Google Play Billing Library 3
	//////////////////////////////////////////////////////////////////////////

	private BillingHandler	m_billing = new BillingHandler() ;

	// 開始
	//////////////////////////////////////////////////////////////////////////
	public boolean startupBilling( String base64EncodedPublicKey )
	{
		return	m_billing.startupBilling( this ) ;
	}

	// 購入済みリスト取得
	//////////////////////////////////////////////////////////////////////////
	public List<BillingHandler.PurchaseInfo> queryInventory()
	{
		return	m_billing.queryInventory() ;
	}

	// アプリ内購入
	//////////////////////////////////////////////////////////////////////////
	public boolean doPurchaseItem( String sku, String payload )
	{
		return	m_billing.doPurchaseItem( sku ) ;
	}

	public boolean doPurchaseSubscription( String sku, String payload )
	{
		return	m_billing.doPurchaseItem( sku ) ;
	}

	public List<BillingHandler.PurchaseInfo> getPurchaseItemList()
	{
		return	m_billing.getPurchaseItemList() ;
	}

	// 購入アイテム消費
	//////////////////////////////////////////////////////////////////////////
	public boolean doConsumeItem( BillingHandler.PurchaseInfo purchase )
	{
		return	m_billing.doConsumeItem( purchase ) ;
	}

#else

	public boolean startupBilling( String base64EncodedPublicKey )
	{
		return	false ;
	}

#endif
}
