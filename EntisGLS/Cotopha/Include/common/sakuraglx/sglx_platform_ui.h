
#if	!defined(__SAKURAGLX_PLATFORM_UI_H__)
#define	__SAKURAGLX_PLATFORM_UI_H__

#if	defined(__PLATFORM_WINDOWS__)
struct ISensorManager ;
struct ISensorCollection ;
struct ISensor ;
#endif

namespace	SakuraGL
{
  namespace	UI
  {
	//////////////////////////////////////////////////////////////////////////
	// プラットフォーム属性
	//////////////////////////////////////////////////////////////////////////

	// タブレット判定
	bool IsPlatformTablet( void ) ;


	//////////////////////////////////////////////////////////////////////////
	// クリップボード
	//////////////////////////////////////////////////////////////////////////

	namespace	Clipboard
	{
		// テキストデータの有無
		__native bool HasPlaneText( void ) ;
		// テキストデータの取得
		__native bool GetPlaneText( SSystem::SString& strText ) ;
		// テキストデータの設定
		__native bool SetPlaneText( const wchar_t * pwszText ) ;

		#if	defined(__PLATFORM_ANDROID__)
		// ClipboardManager 取得
		bool GetClipboardManager( JNI::JavaObject & jobjClipboard ) ;
		#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// バイブレーション
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	__native Vibrator
	{
	public:
		// パターン設定
		__native void SetPattern
			( const uint32_t* pPattern, size_t nCount,
				bool fLoop = true, size_t iLoopStart = 0 ) ;
		// 振動
		__native SGLError Start( void ) ;
		// 停止
		__native void Stop( void ) ;
		// バイブレーション機能が使用可能か？
		__native bool IsInstalled( void ) ;
	} ;
	#endif

	class	SGLVibrator	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLVibrator, SGLObject )
		// 構築関数
		SGLVibrator( void ) ;
		// 消滅関数
		virtual ~SGLVibrator( void ) ;

	protected:
		#if	defined(__COTOPHA__)
			Vibrator *		m_pVibrator ;
		#else
			SSystem::SArray<uint32_t>	m_pattern ;
			bool						m_flagLoop ;
			size_t						m_iLoopStart ;

			#if	defined(__PLATFORM_ANDROID__)
				JNI::JavaObject	m_jobjVibrator ;
				jmethodID		m_jmidCancel ;
				jmethodID		m_jmidVibrate ;
				jmethodID		m_jmidHasVibrator ;
			#endif
		#endif

	public:
		// パターン設定
		virtual void SetPattern
			( const uint32_t* pPattern, size_t nCount,
				bool fLoop = true, size_t iLoopStart = 0 ) ;
		// 振動
		virtual SGLError Start( void ) ;
		// 停止
		virtual void Stop( void ) ;
		// バイブレーション機能が使用可能か？
		virtual bool IsInstalled( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 加速度・ジャイロセンサー
	//////////////////////////////////////////////////////////////////////////

	class	SGLPostureSensor	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLPostureSensor, SGLObject )
		// 構築関数
		SGLPostureSensor( void ) ;
		// 消滅関数
		virtual ~SGLPostureSensor( void ) ;

	protected:
		S3DDMatrix		m_matPosture ;			// 姿勢（回転）
		S3DDMatrix		m_matAccPosture ;
		S3DDMatrix		m_matBasePosture ;
		S3DDVector		m_vPosition ;			// 位置
		S3DDVector		m_vSpeed ;
		S3DVector		m_vGravityLowPass ;
		SSystem::STimeCounter
						m_timerPoll ;			// ポーリング用タイマ
		S3DVector		m_vBaseCompass ;		// 基準方位
		double			m_fpGravity ;			// 重力加速度 [m/s^2]
		double			m_fpGDeviation ;
		size_t			m_nPollCounter ;		// ポーリングカウンタ

		uint32_t		m_nFeatures ;
		S3DVector		m_vLastAccel ;
		S3DVector		m_vLastGyro ;
		S3DVector		m_vLastCompass ;

		#if	defined(__PLATFORM_WINDOWS__)
		#if	_MSC_VER >= 1700
		SSystem::SThread::IdType	m_tidSensor ;
		ISensorManager *	m_pSensorManager ;
		ISensorCollection *	m_pAccelCollection ;
		ISensorCollection *	m_pGyroCollection ;
		ISensorCollection *	m_pCompassCollection ;
		ISensor *			m_pAccelerometer ;
		ISensor *			m_pGyrometer ;
		ISensor *			m_pCompass ;
		#endif

		#elif	defined(__PLATFORM_ANDROID__)
		JNI::JavaObject	m_jobjSensor ;
		JNI::JavaObject	m_jobjValues ;
		jmethodID		m_jmidPrepareSensor ;
		jmethodID		m_jmidGetSensorFeatures ;
		jmethodID		m_jmidGetAccelerometer ;
		jmethodID		m_jmidGetGyroscope ;
		jmethodID		m_jmidGetCompass ;
		jmethodID		m_jmidWaitSensor ;
		#endif

	public:
		enum	FeatureFlag
		{
			featureAccelerometer	= 0x0001,
			featureGyroscope		= 0x0002,
			featureCompass			= 0x0004,
			featureRotateLandScape	= 0x0100,
		} ;
		// センサ準備
		SGLError PrepareSensor( uint32_t nFlags = 0 ) ;
		// センサ機能取得
		uint32_t GetSensorFeatures( void ) const ;
		// 所有リソース解放
		void Release( void ) ;
		// 加速度センサ取得 [m/s^2]
		SGLError GetAccelerometer( S3DVector& vAccel ) const ;
		// ジャイロセンサ取得 [rad/s]
		SGLError GetGyroscope( S3DVector& vRot ) const ;
		// コンパス（北方位ベクトル）取得
		SGLError GetCompass( S3DVector& vCompass ) const ;
		// センサ値の更新タイミング同期
		SGLError WaitSensor( uint32_t nFeatures, int64_t msecTimeout ) ;

	public:
		// センサ値ポーリング
		SGLError PollSensor( uint32_t nFlags = 0 ) ;
		// 姿勢と位置をリセット
		SGLError ResetPosture( void ) ;
		// 位置をリセット
		SGLError ResetPosition( void ) ;
		// 姿勢取得
		const S3DDMatrix & GetPosture( void ) const ;
		// 位置取得
		const S3DDVector & GetPosition( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// アプリ内購入
	//////////////////////////////////////////////////////////////////////////

	class	SGLBillingInApp	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLBillingInApp, SGLObject )
		// 構築関数
		SGLBillingInApp( void ) ;
		// 消滅関数
		virtual ~SGLBillingInApp( void ) ;

	public:
		enum	ItemType
		{
			typeInAppItem,			// アイテム
			typeSubscription,		// 定期購読
		} ;
		class	Purchase
		{
		public:
			ItemType				m_type ;
			SSystem::SObjectArray
				<SSystem::SString>	m_aProductIds ;
			#if	!defined(__ANDROID_BILLING_LIBRARY__)
			SSystem::SString		m_strDevPayload ;
			#endif
			SSystem::SString		m_strOrderID ;

			#if	defined(__PLATFORM_ANDROID__)
				JNI::JavaObject		m_jobjPurchase ;
			#endif

		public:
			// 構築関数
			Purchase( void ) ;
			Purchase( const Purchase& purchase ) ;
			// 消滅関数
			~Purchase( void ) ;
		} ;
		// テスト用定義済みプロダクトID
		#if	defined(__PLATFORM_ANDROID__)
		static const wchar_t *	TestProductPurchased ;		// 購入可能なテストアイテム
		static const wchar_t *	TestProductCanceled ;		// 購入がキャンセルされるテストアイテム
		static const wchar_t *	TestProductRefunded ;		// 払い戻しレスポンスされるテストアイテム
		static const wchar_t *	TestProductUnavailable ;	// 存在しないシミュレーションアイテム
		#endif

	protected:
		bool	m_flagStartup ;

		#if	defined(__PLATFORM_ANDROID__)
		// EntisGLS.getActivity
		jobject GetEntisGLActivity( JNI::JavaObject& jobjActivity ) ;
		// Purchase 変換
		SGLError ConvertFromJava( Purchase& purchase, jobject jobj ) ;
		SGLError ConvertListFromJava
			( SSystem::SObjectArray<Purchase>& listPurchase, jobject jobj ) ;
		#endif

	public:
		// 開始
		SGLError Startup( const wchar_t * pwszBase64PublicKey ) ;
		// 購入済みアイテムリスト
		SGLError QueryInventory
			( SSystem::SObjectArray<Purchase>& listPurchase ) ;
		// 購入フロー実行
		SGLError DoPurchaseItem
			( SSystem::SObjectArray<Purchase>& listPurchase,
				const wchar_t * pwszProductId,
				const wchar_t * pwszDevPayload ) ;
		SGLError DoPurchaseSubscription
			( SSystem::SObjectArray<Purchase>& listPurchase,
				const wchar_t * pwszProductId,
				const wchar_t * pwszDevPayload ) ;
		// アイテム消費
		SGLError DoConsumeItem( const Purchase& purchase ) ;

	} ;

  }
}

#endif

