package com.entis.android.entisgls4 ;

#if	ANDROID_BILLING >= 3
import java.util.*;
import android.app.Activity ;
import com.android.billingclient.api.*;
import com.android.billingclient.api.BillingClient.*;
#if	ANDROID_BILLING >= 5
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.BillingFlowParams.ProductDetailsParams;
import com.android.billingclient.api.QueryProductDetailsParams.Product;
#elif ANDROID_BILLING == 4
#else
import com.android.billingclient.api.Purchase.PurchasesResult;
#endif
#endif


public class BillingHandler
#if	ANDROID_BILLING >= 3
		implements BillingClientStateListener,
#if	ANDROID_BILLING >= 5
					ProductDetailsResponseListener,
#else
					SkuDetailsResponseListener,
#endif
					PurchasesUpdatedListener,
					ConsumeResponseListener
#endif
{
#if	ANDROID_BILLING >= 3
	public static final int	TYPE_IN_APP	= 0 ;
	public static final int	TYPE_SUBS	= 1 ;

	public static final int	STATE_UNSPECIFIED_STATE = Purchase.PurchaseState.UNSPECIFIED_STATE ;
	public static final int	STATE_PENDING           = Purchase.PurchaseState.PENDING ;
	public static final int	STATE_PURCHASED         = Purchase.PurchaseState.PURCHASED ;

	public static class PurchaseInfo
	{
		public Purchase m_purchase = null ;
		public int      m_nType = TYPE_IN_APP ;
		public int      m_nState = STATE_UNSPECIFIED_STATE ;
		public boolean  m_acknowledged = false ;                // 消費不可アイテムの購入承認
		public String   m_strOrderId = null ;                   // 注文ID ex. GPA.1234-5678-9012-34567
#if	ANDROID_BILLING >= 4
		public List<String>	m_listSkus = null ;
#else
		public String   m_strSku = null ;
#endif

		PurchaseInfo( Purchase purchase, int nType )
		{
			m_purchase = purchase ;
			m_nType = nType ;
			m_nState = purchase.getPurchaseState() ;
			m_acknowledged = purchase.isAcknowledged() ;
			m_strOrderId = purchase.getOrderId() ;
#if	ANDROID_BILLING >= 5
			m_listSkus = purchase.getProducts() ;
#elif	ANDROID_BILLING == 4
			m_listSkus = purchase.getSkus() ;
#else
			m_strSku = purchase.getSku() ;
#endif
		}
	}

	public static void addPurchaseInfoList
			( List<PurchaseInfo> listInfos, List<Purchase> listPurchase, int nType )
	{
		for ( Purchase purchase : listPurchase )
		{
			listInfos.add( new PurchaseInfo( purchase, nType ) ) ;
		}
	}

	public static List<PurchaseInfo>
			purchaseInfoListFrom( List<Purchase> listPurchase, int nType )
	{
		ArrayList<PurchaseInfo> listInfos = new ArrayList<PurchaseInfo>() ;
		addPurchaseInfoList( listInfos, listPurchase, nType ) ;
		return  listInfos ;
	}

	public static List<Purchase> purchaseListFromInfos( List<PurchaseInfo> listInfos )
	{
		ArrayList<Purchase> listPurchase = new ArrayList<Purchase>() ;
		for ( PurchaseInfo info : listInfos )
		{
			listPurchase.add( info.m_purchase ) ;
		}
		return  listPurchase ;
	}

	private BillingClient   m_billingClient = null ;
	private Activity        m_activity = null ;
	private boolean         m_flagSetupFinished = false ;
	private boolean         m_flagSetupSucceed = false ;
	private List<Purchase>  m_listInAppPurchases = null ;
	private List<Purchase>  m_listSubsPurchases = null ;

	// 接続開始
	//////////////////////////////////////////////////////////////////////////
	public boolean startupBilling( Activity activity )
	{
		m_activity = activity ;
		m_flagSetupFinished = false ;
		m_flagSetupSucceed = false ;
		//
		if ( m_billingClient == null )
		{
#if	ANDROID_BILLING >= 7
			PendingPurchasesParams	params =
					PendingPurchasesParams.newBuilder()
								.enableOneTimeProducts().build() ;
#endif
			m_billingClient = BillingClient.newBuilder(activity)
								.setListener(this)
#if	ANDROID_BILLING >= 7
								.enablePendingPurchases(params)
#else
								.enablePendingPurchases()
#endif
								.build() ;
		}
		m_billingClient.startConnection( this ) ;
		//
		if ( EntisGLS.isPrimaryThread() )
		{
			return  m_flagSetupFinished ;
		}
		return  waitStartupBilling() ;
	}
	public synchronized boolean waitStartupBilling()
	{
		while ( !m_flagSetupFinished )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_flagSetupFinished ;
	}

	@Override
	public void onBillingSetupFinished( BillingResult billingResult )
	{
		if ( billingResult.getResponseCode() == BillingResponseCode.OK )
		{
			fetchPurchasesList() ;
			//
			synchronized( this )
			{
				m_flagSetupFinished = true ;
				m_flagSetupSucceed = true ;
				notifyAll() ;
			}
		}
		else
		{
			synchronized( this )
			{
				m_flagSetupFinished = true ;
				notifyAll() ;
			}
		}
	}

	@Override
	public void onBillingServiceDisconnected()
	{
		m_flagSetupFinished = false ;
		m_flagSetupSucceed = false ;
	}

	// 終了
	//////////////////////////////////////////////////////////////////////////
	public void endBilling()
	{
		if ( m_billingClient != null )
		{
			m_billingClient.endConnection() ;
			m_billingClient = null ;
		}
	}

	// 購入リストのフェッチ
	//////////////////////////////////////////////////////////////////////////
	public boolean fetchPurchasesList()
	{
		if ( !m_flagSetupSucceed || (m_billingClient == null) )
		{
			return  false ;
		}
		try
		{
#if	ANDROID_BILLING >= 4
			MyPurchasesResponseResult
				purchaseResInApp = new MyPurchasesResponseResult() ;
#if	ANDROID_BILLING >= 5
			m_billingClient.queryPurchasesAsync
				( QueryPurchasesParams.newBuilder()
					.setProductType(BillingClient.ProductType.INAPP).build(), purchaseResInApp ) ;
#else
			m_billingClient.queryPurchasesAsync
				( BillingClient.SkuType.INAPP, purchaseResInApp ) ;
#endif
			purchaseResInApp.waitResponse() ;
			//
			MyPurchasesResponseResult
				purchaseResSubs = new MyPurchasesResponseResult() ;
#if	ANDROID_BILLING >= 5
			m_billingClient.queryPurchasesAsync
				( QueryPurchasesParams.newBuilder()
					.setProductType(BillingClient.ProductType.SUBS).build(), purchaseResSubs ) ;
#else
			m_billingClient.queryPurchasesAsync
				( BillingClient.SkuType.SUBS, purchaseResSubs ) ;
#endif
			purchaseResSubs.waitResponse() ;
			//
			synchronized( this )
			{
				if ( purchaseResInApp.m_codeResult
						== BillingClient.BillingResponseCode.OK )
				{
					m_listInAppPurchases = purchaseResInApp.m_listPurchase ;
				}
				if ( purchaseResSubs.m_codeResult
						== BillingClient.BillingResponseCode.OK )
				{
					m_listSubsPurchases = purchaseResSubs.m_listPurchase ;
				}
			}
#else
			Purchase.PurchasesResult    purchaseResult = null ;
			List<Purchase>  listInAppPurchase = null ;
			purchaseResult = m_billingClient.queryPurchases( BillingClient.SkuType.INAPP ) ;
			listInAppPurchase = purchaseResult.getPurchasesList() ;
			//
			List<Purchase>  listSubsPurchase = null ;
			purchaseResult = m_billingClient.queryPurchases( BillingClient.SkuType.SUBS ) ;
			listSubsPurchase = purchaseResult.getPurchasesList() ;
			//
			synchronized( this )
			{
				m_listInAppPurchases = listInAppPurchase ;
				m_listSubsPurchases = listSubsPurchase ;
			}
#endif
		}
		catch ( Exception e )
		{
			return  false ;
		}
		return  true ;
	}

#if	ANDROID_BILLING >= 4
	class	MyPurchasesResponseResult	implements PurchasesResponseListener
	{
		public boolean			m_flagDone = false ;
		public int				m_codeResult ;
		public List<Purchase>	m_listPurchase = null ;

		@Override
		public synchronized void onQueryPurchasesResponse
			( BillingResult billingResult, List<Purchase> purchases )
		{
			m_codeResult = billingResult.getResponseCode() ;
			m_listPurchase = purchases ;
			m_flagDone = true ;
			notifyAll() ;
		}

		public synchronized void waitResponse()
		{
			while ( !m_flagDone )
			{
				try
				{
					wait( 100 ) ;
				}
				catch ( Exception e )
				{
				}
			}
		}
	}
#endif

	// 購入済みリスト取得
	//////////////////////////////////////////////////////////////////////////
	public synchronized List<PurchaseInfo> queryInventory()
	{
		if ( !m_flagSetupSucceed || (m_billingClient == null) )
		{
			return  null ;
		}
		if ( m_listInAppPurchases == null )
		{
			fetchPurchasesList() ;
			if ( m_listInAppPurchases == null )
			{
				return  null ;
			}
		}
		List<Purchase>  listInventory = new ArrayList<Purchase>() ;
		for ( Purchase purchase : m_listInAppPurchases )
		{
			if ( purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED )
			{
				listInventory.add( purchase ) ;
			}
		}
		List<Purchase>  listSubsInventory = new ArrayList<Purchase>() ;
		if ( m_listSubsPurchases != null )
		{
			for ( Purchase purchase : m_listSubsPurchases )
			{
				if ( purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED )
				{
					listSubsInventory.add( purchase ) ;
				}
			}
		}
		List<PurchaseInfo>
			listPurchases =
				purchaseInfoListFrom( listInventory, TYPE_IN_APP ) ;
		addPurchaseInfoList( listPurchases, listSubsInventory, TYPE_SUBS ) ;
		return	listPurchases ;
	}

	// アプリ内購入（購入フローを起動する）
	//////////////////////////////////////////////////////////////////////////
	public synchronized boolean doPurchaseItem( String sku, int skuType )
	{
		if ( !m_flagSetupSucceed || (m_billingClient == null) )
		{
			return  false ;
		}
		// SkuDetails 取得
		List<String>  skuList = new ArrayList<String>() ;
		skuList.add( sku ) ;
		asyncQuerySkuDetails( skuList, skuType ) ;
		//
#if	ANDROID_BILLING >= 5
		List<ProductDetails>	listSkuDetails = getQuerySkuDetails() ;
#else
		List<SkuDetails>	listSkuDetails = getQuerySkuDetails() ;
#endif
		if ( (listSkuDetails == null) || (listSkuDetails.size() == 0) )
		{
			return  false ;
		}
#if	ANDROID_BILLING >= 5
		ProductDetails	skuDetails = listSkuDetails.get(0) ;
#else
		SkuDetails  skuDetails = listSkuDetails.get(0) ;
#endif
		if ( skuDetails == null )
		{
			return  false ;
		}
		// 購入フロー開始
		if ( !launchBillingFlow( skuDetails, skuType ) )
		{
			return  false ;
		}
		return  waitPurchaseFlow() ;
	}

	// 購入可能アイテムの取得
	// skuList : 取得したい商品の sku のリスト
	public synchronized void asyncQuerySkuDetails( List<String> skuList, int skuType )
	{
#if	ANDROID_BILLING >= 5
		String	typeProduct =
					(skuType == TYPE_IN_APP)
						? ProductType.INAPP : ProductType.SUBS ;
		ArrayList<Product>	listProduct = new ArrayList<Product>() ;
		for ( int i = 0; i < skuList.size(); i ++ )
		{
			listProduct.add
				( Product.newBuilder()
					.setProductId( skuList.get(i) )
					.setProductType( typeProduct )
					.build() ) ;
		}
		QueryProductDetailsParams
			params = QueryProductDetailsParams.newBuilder()
						.setProductList(listProduct)
						.build() ;
		//
		m_listSkuDetails = null ;
		m_billingClient.queryProductDetailsAsync( params, this ) ;
#else
		SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder() ;
		params.setSkusList(skuList).setType
			( (skuType == TYPE_IN_APP) ? SkuType.INAPP : SkuType.SUBS ) ;
		//
		m_listSkuDetails = null ;
		m_billingClient.querySkuDetailsAsync( params.build(), this ) ;
#endif
	}

#if	ANDROID_BILLING >= 5
	public synchronized List<ProductDetails> getQuerySkuDetails()
#else
	public synchronized List<SkuDetails> getQuerySkuDetails()
#endif
	{
		while ( m_listSkuDetails == null )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
#if	ANDROID_BILLING >= 5
		List<ProductDetails>	listSkuDetails = m_listSkuDetails ;
#else
		List<SkuDetails>	listSkuDetails = m_listSkuDetails ;
#endif
		m_listSkuDetails = null ;
		return	listSkuDetails ;
	}

#if	ANDROID_BILLING >= 5
	private List<ProductDetails>    m_listSkuDetails = null ;

	@Override
	public synchronized void onProductDetailsResponse
			( BillingResult billingResult, List<ProductDetails> productDetailsList )
	{
		m_listSkuDetails = productDetailsList ;
		notifyAll() ;
	}
#else
	private List<SkuDetails>    m_listSkuDetails = null ;

	@Override
	public synchronized void onSkuDetailsResponse
			( BillingResult billingResult, List<SkuDetails> skuDetailsList )
	{
		m_listSkuDetails = skuDetailsList ;
		notifyAll() ;
	}
#endif

	// 購入フローを起動する
#if	ANDROID_BILLING >= 5
	public synchronized boolean launchBillingFlow( ProductDetails productDetails, int skuType )
	{
		m_flagFinishedPurchase = false ;
		m_listPurchases = null ;

		// Retrieve a value for "productDetails" by calling queryProductDetailsAsync()
		// Get the offerToken of the selected offer

		// Set the parameters for the offer that will be presented
		// in the billing flow creating separate productDetailsParamsList variable
		ArrayList<ProductDetailsParams>
			listProductDetailsParams = new ArrayList<ProductDetailsParams>() ;
		ProductDetailsParams.Builder
			builderParams = ProductDetailsParams.newBuilder()
								.setProductDetails( productDetails ) ;
		if ( skuType == TYPE_SUBS )
		{
			int	selectedOfferIndex = 0 ;
			String	offerToken = productDetails
									.getSubscriptionOfferDetails()
									.get(selectedOfferIndex)
									.getOfferToken();
			builderParams = builderParams.setOfferToken(offerToken) ;
		}
		listProductDetailsParams.add( builderParams.build() ) ;

		BillingFlowParams	billingFlowParams =
			BillingFlowParams.newBuilder()
				.setProductDetailsParamsList( listProductDetailsParams )
				.build() ;

		BillingResult	billingResult =
				m_billingClient.launchBillingFlow( m_activity, billingFlowParams ) ;
		int	responseCode = billingResult.getResponseCode() ;
		return	responseCode == BillingClient.BillingResponseCode.OK ;
	}
#else
	public synchronized boolean launchBillingFlow( SkuDetails skuDetails )
	{
		m_flagFinishedPurchase = false ;
		m_listPurchases = null ;

		// Retrieve a value for "skuDetails" by calling querySkuDetailsAsync().
		BillingFlowParams billingFlowParams =
				BillingFlowParams.newBuilder()
						.setSkuDetails(skuDetails)
						.build();
		int responseCode =
				m_billingClient.launchBillingFlow
					( m_activity, billingFlowParams ).getResponseCode() ;
		return  responseCode == BillingClient.BillingResponseCode.OK ;
	}
#endif

	// 購入フロー完了待ち
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
		return	(m_listPurchases != null) ;
	}

	// 購入完了アイテムリスト取得
	public synchronized List<PurchaseInfo> getPurchaseItemList()
	{
		return	purchaseInfoListFrom( m_listPurchases, TYPE_IN_APP ) ;
	}

	private boolean         m_flagFinishedPurchase = false ;
	private List<Purchase>  m_listPurchases = null ;

	@Override
	public synchronized void onPurchasesUpdated
			( BillingResult billingResult, List<Purchase> purchases )
	{
		if ( (billingResult.getResponseCode() == BillingResponseCode.OK) && (purchases != null) )
		{
			m_listInAppPurchases = null ;
			m_listPurchases = purchases ;
		}
		else if ( billingResult.getResponseCode() == BillingResponseCode.USER_CANCELED )
		{
			// Handle an error caused by a user cancelling the purchase flow.
		}
		else
		{
			// Handle any other error codes.
		}
		m_flagFinishedPurchase = true ;
		notifyAll() ;
	}

	// 購入アイテムの消費
	public synchronized boolean doConsumeItem( PurchaseInfo purchaseInfo )
	{
		if ( !m_flagSetupSucceed
				|| (m_billingClient == null)
				|| (purchaseInfo == null)
				|| (purchaseInfo.m_purchase == null) )
		{
			return  false ;
		}
		m_flagConsumeFinished = false ;
		m_flagConsumeSucceed = false ;

		// Verify the purchase.
		// Ensure entitlement was not already granted for this purchaseToken.
		// Grant entitlement to the user.
		ConsumeParams consumeParams =
			ConsumeParams.newBuilder()
				.setPurchaseToken
						(purchaseInfo.m_purchase.getPurchaseToken())
				.build() ;
		m_billingClient.consumeAsync( consumeParams, this ) ;

		return  waitConsumeItem() ;
	}

	public synchronized boolean waitConsumeItem()
	{
		while ( !m_flagConsumeFinished )
		{
			try
			{
				wait( 100 ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	m_flagConsumeSucceed ;
	}

	private boolean m_flagConsumeFinished = false ;
	private boolean m_flagConsumeSucceed  = false ;

	@Override
	public synchronized void onConsumeResponse
				( BillingResult billingResult, String purchaseToken )
	{
		if ( billingResult.getResponseCode() == BillingResponseCode.OK )
		{
			m_listInAppPurchases = null ;
			m_flagConsumeSucceed = true ;
		}
		m_flagConsumeFinished = true ;
		notifyAll() ;
	}

#endif
}
