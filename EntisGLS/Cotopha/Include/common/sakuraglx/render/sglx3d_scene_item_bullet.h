
#if	!defined(__SAKURAGLX3D_SCENE_ITEM_BULLET_H__)
#define	__SAKURAGLX3D_SCENE_ITEM_BULLET_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 弾幕制御基底
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletItemInterface
	{
	public:
		enum	BulletStateFlag
		{
			stateNoCollider			= 0x00000001,	// 当たり判定なし
			stateLifeFadeout		= 0x00000002,	// フェードアウト中
			stateNoHitTimer			= 0x00000004,	// 当たり判定一時無効
			stateHitDestroy			= 0x00000008,	// 先頭がヒットし順次消去
			statePrepareLaser		= 0x00000010,	// レーザー予備動作（ガイド表示）
			stateNoHitConditions	= 0x0000000F,
			stateControlMoved		= 0x00000100,	// フレーム内で移動処理済み
			stateMovedTrack			= 0x00000200,	// 移動処理でトラック全体を更新
			stateMovedTrackTail		= 0x00000400,	// 移動処理でトラック末尾を削除した
			stateAllMovedFlags		= 0x00000F00,
			statePreventCollider	= 0x00001000,	// デフォルトのコライダを抑制する（コントローラー側のみで処理する場合）
			stateControlDestroy		= 0x00002000,	// 遅延削除アイテム
		} ;
		enum	BulletConstant
		{
			countMaxInstance	= 8,
		} ;
		struct	Bullet
		{
			size_t				nInstanceCount ;
			ESLObject *			pCtrlInstance[countMaxInstance] ;
			Rosetta::RSObject *	pUserObj ;
			Loquaty::LObject *	pUserLObj ;
			float32_t			secLife ;
			float32_t			secMaxLife ;
			float32_t			secTimer ;
			float32_t			fpBaseThickness ;
			float32_t			fpThickness ;
			float32_t			fpLength ;		// flagDirection での長さ
			S3DDVector			vPos ;			// （大域）先端位置（flagDirection/flagLaser では発射位置）
			S3DDVector			vSpeed ;		// 速度（flagLaser では向き）
			uint32_t			idBullet ;
			uint32_t			nStateFlags ;	// complex of enum BulletStateFlag
			uint32_t			nAlpha ;
			uint32_t			nTrackCount ;
			S3DDVector			vTrack[1] ;		// [0] == vPos
												// [nTrackCount] まで有効
		} ;
		enum	BulletBehaviorFlag
		{
			flagCollider	= 0x00000001,	// 当たり判定あり
			flagCollision	= 0x00000002,	// 被当たり判定追加
			flagPierce		= 0x00000100,	// 貫通弾
			flagDirection	= 0x00000200,	// 障害物に接触するまで直進し障害物の箇所で止まる
			flagLaser		= 0x00000400,	// 速度が無限大
			flagMultiHit	= 0x00000800,	// 複数対象に同時衝突時、全対象に OnHitBullet
			flagTransform	= 0x00001000,	// 座標変換あり
		} ;
		class	BulletListener	: public SSystem::SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( BulletListener, SObject )
			// 禁止状態
			virtual bool IsBulletDisabled( void ) const = 0 ;
			// 発射時処理
			virtual void OnFireBullet
				( S3DBulletItemInterface * pItem,
					S3DBulletItemInterface::Bullet& bullet ) = 0 ;
			// 着弾時処理
			virtual void OnHitBullet
				( S3DBulletItemInterface * pItem,
					S3DSceneComposer::ItemSerializer * pHitItem,
					const S3DCollision::MeshCollision * pmcHitMesh,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) = 0 ;
			// タイマー前処理
			virtual void BeforeBulletTimer
				( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) = 0 ;
			// タイマー処理
			virtual void OnBulletTimer
				( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) = 0 ;
			// 当たり判定追加処理
			virtual void RenderBulletCollision
				( const S3DScene& scene,
					S3DCollision& collision,
					const S3DBulletItemInterface * pItem,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) = 0 ;
			// 弾軌跡描画（追加処理）
			virtual void RenderBulletTracks
				( S3DScene& scene,
					const S3DBulletItemInterface * pItem,
					const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) = 0 ;
		} ;
		class	BulletController
				: public S3DSceneComposer::Controller, public BulletListener
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( BulletController, Controller, BulletListener )
			// 構築関数
			BulletController( const wchar_t * pwszClassID ) ;
			// 禁止状態
			virtual bool IsBulletDisabled( void ) const ;
		} ;

	protected:
		SSystem::SPointerArray<Bullet>	m_bullets ;
		uint32_t	m_idBulletNext ;

		uint32_t	m_nBulletFlags ;			// enum BulletBehaviorFlag
		size_t		m_nMaxTrackCount ;			// Bullet.vTrack の配列最大数（実際には１つ多い）
		size_t		m_nMaxColliderCount ;		// 当たり判定に使用する軌跡最大数
		float32_t	m_fpColliderRadius ;		// 当たり判定半径
		uint32_t	m_maskColliderClasses ;		// 当たり判定対象クラスビットマスク enum S3DCollision::UserColliderClass
		uint32_t	m_maskCollisionClasses ;	// 被当たり判定設定クラスビットマスク enum S3DCollision::UserColliderClass
		double		m_secBulletMaxLife ;		// 最大寿命 [sec]
		double		m_secBulletFadeout ;		// フェードアウト時間 [sec]
		double		m_secLaserPrepareDuration ;	// レーザー発射前時間（当たり判定無しガイド表示）
		double		m_fpLaserPrepareThickness ;	// レーザー発射前太さ比
		double		m_fpLaserPrepareAlpha ;		// レーザー発射前不透明度
		double		m_secLaserFlashDuration ;	// レーザー発射閃光時間 [sec]
		double		m_fpLaserFlashThickness ;	// レーザー発射閃光太さ比
		double		m_secNoHitInterval ;		// 貫通弾の当たり判定インターバル [sec]
		double		m_fpLaserMaxLength ;		// レーザー最大長

		SSystem::SPointerArray<BulletListener>	m_aListeners ;

		SSystem::SCriticalSection	m_csBullets ;

	public:
		// 複数当たり判定
		struct	HitBulletEntry
		{
			const S3DCollision::MeshCollision *	pmcHitPrimitive ;
			const S3DCollision::MeshCollision *	pmcHitGlobal ;
			S3DVector							vHitGlobalPos ;
			S3DVector							vHitGlobalNormal ;
			size_t								iHitTrack ;
		} ;
		class	MultiHitInstance	: public SSystem::SArray<HitBulletEntry>
		{
		protected:
			S3DBulletItemInterface *	m_pbii ;
			S3DVector					m_vRay ;
			size_t						m_iCurTrack ;

		public:
			MultiHitInstance( S3DBulletItemInterface * pbii ) ;
			void ResetInstance( void ) ;
			bool IsEmpty( void ) const ;
			void SetRay( const S3DVector& vRay ) ;
			void SetCurrentTrack( size_t iTrack ) ;

			static S3DCollision::HitColliderCallback
				Callback_OnHitSphereCollider
					( const S3DCollisionResult& rsHit,
						const S3DVector& vHitPos, const S3DVector& vHitNormal,
						const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
			static S3DCollision::HitColliderCallback
				Callback_OnHitRayCollider
					( const S3DCollisionResult& rsHit,
						const S3DVector& vHitPos, const S3DVector& vHitNormal,
						const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO(S3DBulletItemInterface)
		// 構築関数
		S3DBulletItemInterface( void ) ;
		// 消滅関数
		~S3DBulletItemInterface( void ) ;

	public:
		// 動作フラグ (BulletBehaviorFlag)
		uint32_t GetBulletFlags( void ) const ;
		void SetBulletFlags( uint32_t nFlags ) ;
		// 最大軌跡
		size_t GetMaxTrackCount( void ) const ;
		void SetMaxTrackCount( size_t nCount ) ;
		// 軌跡当たり判定数
		size_t GetTrackColliderCount( void ) const ;
		void SetTrackColliderCount( size_t nCount ) ;
		// 当たり判定半径
		float32_t GetColliderRadius( void ) const ;
		void SetColliderRadius( float32_t fpRadius ) ;
		// 当たり判定対象
		void SetColliderTargetClasses( uint32_t maskClasses ) ;
		uint32_t GetColliderTargetClasses( void ) const ;
		// 被当たり判定設定
		void SetCollisionClasses( uint32_t maskClasses ) ;
		uint32_t GetCollisionClasses( void ) const ;
		// 寿命
		double GetBulletMaxLife( void ) const ;
		double GetBulletFadeout( void ) const ;
		void SetBulletMaxLife( double sec ) ;
		void SetBulletFadeout( double sec ) ;
		// レーザー最大長
		double GetLaserMaxLength( void ) const ;
		void SetLaserMaxLength( double fpLength ) ;
		// レーザー予備動作
		double GetLaserPrepareDuration( void ) const ;
		double GetLaserPrepareThickness( void ) const ;
		double GetLaserPrepareAlpha( void ) const ;
		void SetLaserPrepareDuration( double sec ) ;
		void SetLaserPrepareThickness( double fpThickness ) ;
		void SetLaserPrepareAlpha( double alpha ) ;
		// レーザー発射直後閃光
		double GetLaserFlashDuration( void ) const ;
		double GetLaserFlashThickness( void ) const ;
		void SetLaserFlashDuration( double sec ) ;
		void SetLaserFlashThickness( double fpThickness ) ;
		// 貫通弾当たり判定インターバル
		double GetBulletHitInterval( void ) const ;
		void SetBulletHitInterval( double sec ) ;

	public:
		// 弾幕生成（大域座標）
		virtual void GenerateBullets
			( const S3DDVector * pvPositions,
				const S3DDVector * pvSpeeds,
				const float32_t * pThickness,
				Rosetta::RSObject ** ppObjects, size_t nCount ) ;
		virtual Bullet * GenerateBullet
			( const S3DDVector& vPosition,
				const S3DDVector& vSpeed,
				float32_t fpThickness = 1.0f,
				Rosetta::RSObject * pObject = nullptr,
				Loquaty::LObject * pLObject = nullptr /* AddRef された LObject */ ) ;
		// OnFireBullet 呼び出し
		void CallOnFireBullet( Bullet * pBullet ) ;
		// 弾幕取得
		bool IsValidBullet( Bullet * pBullet ) const ;

	protected:
		// 新規 Bullet 生成
		virtual Bullet * NewBullet( void ) ;
		// Bullet 解放
		virtual void DeleteBullet( Bullet * pBullet ) ;
	public:
		virtual void DeleteAllBullets( void ) ;

	public:
		// Bullet にインスタンス追加
		static bool AddBulletInstance( Bullet& bullet, ESLObject * pObj ) ;
		// Bullet インスタンス取得
		static ESLObject * GetBulletInstance
			( const Bullet& bullet, const ESLRuntimeClass& rtClass ) ;
		// Bullet　デフォルトの移動処理
		enum	MoveBulletResult
		{
			resultNoMove,
			resultMoved,
			resultDelete,
		} ;
		MoveBulletResult MoveBulletDefault( Bullet& bullet, double secPast ) const ;
		// Bullet をフェードアウト開始
		void MakeBulletFadeout( Bullet& bullet ) const ;
		// Bullet を当たり判定インターバル開始
		void MakeBulletNotHitInterval( Bullet& bullet ) const ;
		// Bullet を衝突（順次）削除設定
		void MakeBulletHitDestroy( Bullet& bullet ) const ;
		// Bullet を削除設定
		void MakeBulletDestroy( Bullet& bullet ) const ;
		// Bullet に新しい座標を追加
		void MoveBullet( Bullet& bullet, const S3DDVector& vPos ) const ;

	public:
		// リスナ追加
		void AddBulletListener( BulletListener * pListener ) ;
		// リスナ削除
		void DetachBulletListener( BulletListener * pListener ) ;
		// リスナ数取得
		size_t GetBulletListenerCount( void ) const ;
		BulletListener * GetBulletListenerAt( size_t i ) const ;

	public:
		// 空間行列取得
		virtual void GetGlobalTransformation
			( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const = 0 ;

	public:
		// 当たり判定
		void TestHitToCollider
			( MultiHitInstance& mhiHits,
				const S3DCollider& collider,
				const Bullet& bullet,
				const S3DDMatrix& matSpace,
				const S3DDVector& vSpace,
				float32_t fpHitRadius ) const ;
		// OnHitBullet 呼び出し
		void CallOnMultiHitBullet
			( const MultiHitInstance& mhi,
				const S3DMatrix& matISpace, const S3DVector& vSpace,
				S3DBulletItemInterface::Bullet& bullet ) ;
		void CallOnHitBullet
			( const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;

	public:
		// Bullet 移動処理
		virtual void OnBulletTimer
			( S3DCollider& collider,
				const S3DDMatrix& matSpace,
				const S3DDVector& vSpace, float32_t secPast ) ;
		// Bullet 当たり判定追加
		virtual void AddBulletCollision
			( const S3DScene& scene, S3DCollision& collision ) const ;
		// Bullet 描画
		virtual void RenderBulletTracks( S3DScene& scene ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾幕制御アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletItemSerializer
				: public S3DSceneComposer::ItemBasicSerializer,
					public S3DBulletItemInterface
	{
	public:
		enum	ParameterIndex
		{
			paramEnableCollider		= ItemBasicSerializer::paramItemTotalCount,
			paramColliderClasses,
			paramEnableCollision,
			paramCollisionClasses,
			paramColliderTrackCount,
			paramColliderRadius,
			paramEnablePierce,
			paramEnableDirection,
			paramEnableLaser,
			paramEnableMultiHit,
			paramEnableTransform,
			paramBulletTrackCount,
			paramBulletMaxLife,
			paramBeamMaxLength,
			paramLaserPrepareDuration,
			paramLaserPrepareThickness,
			paramLaserPrepareAlpha,
			paramLaserFlashDuration,
			paramLaserFlashThickness,
			paramBulletFadeout,
			paramBulletHitInterval,
			paramBulletTotalCount,
			paramBulletCount		= paramBulletTotalCount - paramEnableCollider,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramBulletCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DBulletItemSerializer,
					ItemBasicSerializer, S3DBulletItemInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletItemSerializer, bullet )
		// 構築関数
		S3DBulletItemSerializer( void ) ;
		// 消滅関数
		virtual ~S3DBulletItemSerializer( void ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;

	protected:	// S3DSceneComposer::ItemSerializer
		// コントローラー通知
		virtual void OnAddController( S3DSceneComposer::Controller * pController ) ;
		virtual void BeforeDetachController( S3DSceneComposer::Controller * pController ) ;

	public:	// ItemBasicSerializer
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// 当たり判定追加
		virtual void OnItemRenderCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:	// S3DBulletItemInterface
		// 空間行列取得
		virtual void GetGlobalTransformation
			( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 砲台アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DBatteryItemSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramBulletItem		= ItemBasicSerializer::paramItemTotalCount,
			paramInterval,
			paramFireCount,
			paramScatterType,
			paramDirection,
			paramSubDirection,
			paramSpeed,
			paramThickness,
			paramScatterAngle,
			paramConeAngle,
			paramBatteryTotalCount,
			paramBatteryCount		= paramBatteryTotalCount - paramBulletItem,
		} ;
		enum	ScatterType
		{
			scatter1Way,
			scatter1WayBeam,
			scatterOddNWay,
			scatterEvenNWay,
			scatter2DRandom,
			scatter3DRandom,
			scatter3DRandomCone,
			scatterCount,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramBatteryCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const wchar_t *	m_pwszScatterType[scatterCount] ;

		SSystem::SString		m_strBulletItem ;
		SSystem::SSyncReference	m_refBulletItem ;
		SSystem::SPointerArray
			<S3DBulletItemInterface::Bullet>	m_aBeamBullet ;

		size_t				m_nFireInterval ;
		double				m_fpIntervalCounter ;
		double				m_fpLastFrame ;
		size_t				m_nFireCount ;
		ScatterType			m_typeScatter ;
		S3DDVector			m_vDirection ;
		S3DDVector			m_vSubDirection ;
		double				m_fpBulletSpeed ;
		double				m_fpBulletThickness ;
		double				m_degScatterAngle ;
		double				m_degConeAngle ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBatteryItemSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DBatteryItemSerializer, battery )
		// 構築関数
		S3DBatteryItemSerializer( void ) ;
		// 消滅関数
		virtual ~S3DBatteryItemSerializer( void ) ;

	public:
		// ターゲットを設定
		void AttachBulletItem
			( S3DBulletItemSerializer * pItem, const wchar_t * pwszID ) ;
		bool UpdateBulletReference( void ) ;
		// 発射間隔 [frame]
		size_t GetFireInterval( void ) const ;
		void SetFireInterval( size_t nInterval ) ;
		// 発射弾数
		size_t GetFireCount( void ) const ;
		void SetFireCount( size_t nCount ) ;
		// ばら撒き方
		ScatterType GetScatterType( void ) const ;
		void SetScatterType( ScatterType type ) ;
		// 方向（ローカル空間）
		const S3DDVector& GetDirection( void ) const ;
		void SetDirection( const S3DDVector& vDir ) ;
		// 副方向（ローカル空間）
		const S3DDVector& GetSubDirection( void ) const ;
		void SetSubDirection( const S3DDVector& vSubDir ) ;
		// 弾速度
		double GetBulletSpeed( void ) const ;
		void SetBulletSpeed( double fpSpeed ) ;
		// 弾太さ比率
		double GetBulletThickness( void ) const ;
		void SetBulletThickness( double fpThickness ) ;
		// ばら撒き範囲角 [deg]
		double GetScatterAngle( void ) const ;
		void SetScatterAngle( double degAngle ) ;
		// 円錐角 [deg]
		double GetConeAngle( void ) const ;
		void SetConeAngle( double degAngle ) ;
		// 弾発射
		void FireBullet( size_t nCount ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾描画コントローラー（先端）
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletDrawController
					: public S3DBulletItemInterface::BulletController
	{
	public:
		enum	ParameterIndex
		{
			paramDrawTarget,
			paramDrawPosition,
			paramDrawZoom,
			paramRotateEnable,
			paramRotateDir,
			paramRotateAngle,
			paramRotateAxis,
			paramRotateSpeed,
		} ;
		enum	RotaionAxis
		{
			rotateOnX,
			rotateOnY,
			rotateOnZ,
			rotateOnW,
			rotateAxisCount
		} ;
		static const SSystem::SXMLDocument::AttrInteger
							m_aiRotateAxis[rotateAxisCount+1] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBulletDrawController, BulletController )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletDrawController, bullet_drawer )
		// 構築関数
		S3DBulletDrawController( void ) ;
		// 消滅関数
		virtual ~S3DBulletDrawController( void ) ;

	protected:
		SSystem::SSmartReference
			<S3DParticleSerializer::RenderTarget>
									m_refDrawTarget ;
		SSystem::SString			m_strDrawTarget ;
		double						m_fpDrawPosition ;
		double						m_fpZoomScale ;
		S3DDVector					m_vRotAngle ;
		double						m_degRotSpeed ;
		RotaionAxis					m_rotAxis ;
		bool						m_flagEnableRotation ;
		bool						m_flagRotDirection ;

		SSystem::SArray<S3DVector4>	m_bufPoints ;
		SSystem::SArray<size_t>		m_bufFrames ;
		SSystem::SArray<S3DColor>	m_bufColors ;
		SSystem::SArray<float32_t>	m_bufZooms ;
		SSystem::SArray<S3DMatrix>	m_bufMatrixs ;
		SSystem::SArray
			<S3DParticleSerializer::ParticleIndex>
									m_bufParticleIndex ;

	public:
		// 描画ターゲット設定
		void AttachRenderTarget
			( S3DParticleSerializer::RenderTarget * pTarget, const wchar_t * pwszID ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DBulletItemInterface::BulletListener
		// 発射時処理
		virtual void OnFireBullet
			( S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet& bullet ) ;
		// 着弾時処理
		virtual void OnHitBullet
			( S3DBulletItemInterface * pItem,
				S3DSceneComposer::ItemSerializer * pHitItem,
				const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;
		// タイマー前処理
		virtual void BeforeBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// タイマー処理
		virtual void OnBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 当たり判定追加処理
		virtual void RenderBulletCollision
			( const S3DScene& scene,
				S3DCollision& collision,
				const S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 弾軌跡描画（追加処理）
		virtual void RenderBulletTracks
			( S3DScene& scene, const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾描画コントローラー（軌跡）
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletRenderController
				: public S3DBulletItemInterface::BulletController
	{
	public:
		enum	ParameterIndex
		{
			paramRenderTarget,
			paramShapeType,
			paramUScale,
			paramVScale,
			paramVSpeed,
			paramZBais,
			paramTopCap,
			paramTailCap,
			paramHorzDivision,
			paramCapDivision,
			paramTopAlpha,
			paramTailAlpha,
			paramThickness0,
			paramThickness1,
			paramThickIndex1,
			paramThickness2,
			paramThickIndex2,
			paramThickness3,
			paramThickIndex3,
			paramThunderCount,
			paramThunderJointEffect,
			paramThunderJointLen,
			paramThunderWaveEffect,
			paramThunderWaveLen,
			paramColorAlpha,
			paramColorMul,
			paramColorAdd,
			paramColorDivision,
			paramColorLevel0,
			paramColorAlpha0,
			paramColorMul0,
			paramColorAdd0,
			paramColorEntryFirst	= paramColorLevel0,
			paramColorElementCount	= paramColorAdd0 - paramColorLevel0 + 1,
		} ;
		enum	ShapeTypeIndex
		{
			shapeBand,
			shapeTube,
			shapeCount,
		} ;
		static const wchar_t *	m_pwszShapeType[shapeCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBulletRenderController, BulletController )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletRenderController, beam_renderer )
		// 構築関数
		S3DBulletRenderController( void ) ;
		// 消滅関数
		virtual ~S3DBulletRenderController( void ) ;

	public:
		// 稲妻用 Bullet インスタンス
		class	ThunderInstance
			: public ESLObject,
				public SSystem::SObjectArray<S3DMeshShaper::ThunderContext>
		{
		public:
			ESL_DECLARE_CLASS_INFO( ThunderInstance, ESLObject )
			ESL_DECLARE_CLASS_OPERATOR_NEW( ESLObject )
		} ;

	protected:
		SSystem::SSyncReference		m_refRenderTarget ;
		SSystem::SString			m_strRenderTarget ;

		ShapeTypeIndex			m_shape ;

		double					m_fpUScale ;
		double					m_fpVScale ;
		double					m_fpVSpeed ;
		double					m_fpZBais ;
		double					m_fpTopAlpha ;
		double					m_fpTailAlpha ;
		double					m_fpThickness[4] ;
		size_t					m_iThickIndex[4] ;

		size_t					m_iThunderOffset ;
		size_t					m_nThunderCount ;
		double					m_fpThunderJointEffect ;
		double					m_fpThunderJointLen ;
		double					m_fpThunderWaveEffect ;
		double					m_fpThunderWaveLen ;

		bool					m_flagTubeHeadCap ;
		bool					m_flagTubeTailCap ;
		size_t					m_nTubeHDivision ;
		size_t					m_nTubeVDivision ;
		S3DColor				m_colorBase ;

		SSystem::SArray<S3DColor>	m_aColorDiv ;
		SSystem::SArray<float32_t>	m_aThickDiv ;
		SSystem::SObjectArray<SSystem::SString>
									m_aColorDivIDs ;
		SSystem::SObjectArray<SSystem::SString>
									m_aColorDivNames ;

	public:
		// 描画ターゲット設定
		void AttachRenderTarget
			( S3DMeshBufferItemSerializer * pTarget, const wchar_t * pwszID ) ;
		bool UpdateRenderTarget( void ) ;
		// 色分割数設定
		void SetColorDivCount( size_t nCount ) ;
	protected:
		SSystem::SString * GetColorDivPropID
			( size_t iProp, const wchar_t * pwszFormat, size_t i ) ;
		SSystem::SString * GetColorDivPropName
			( size_t iProp, const wchar_t * pwszFormat, size_t i ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::Controller
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DBulletItemInterface::BulletListener
		// 発射時処理
		virtual void OnFireBullet
			( S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet& bullet ) ;
		// 着弾時処理
		virtual void OnHitBullet
			( S3DBulletItemInterface * pItem,
				S3DSceneComposer::ItemSerializer * pHitItem,
				const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;
		// タイマー前処理
		virtual void BeforeBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// タイマー処理
		virtual void OnBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 当たり判定追加処理
		virtual void RenderBulletCollision
			( const S3DScene& scene,
				S3DCollision& collision,
				const S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 弾軌跡描画（追加処理）
		virtual void RenderBulletTracks
			( S3DScene& scene, const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;

	protected:
		SakuraCL::SCLRandomizer			m_random ;
		S3DMeshShaper::ThickLinesParam	m_tlpBeam ;
		S3DMeshShaper::TubeParam		m_tubeBeam ;

		S3DDMatrix	m_matBulletToVB ;
		S3DDVector	m_vBulletToVB ;
		S3DMatrix	m_matICamera ;
		S3DVector	m_vCameraRay ;
		S3DVector	m_vCameraPos ;

		SSystem::SArray<S3DVector4>	m_aPointBuf ;
		SSystem::SArray<float32_t>	m_aThickBuf ;
		SSystem::SArray<uint32_t>	m_aAlphaBuf ;

		void RenderBulletTrack
			( S3DVertexBufferInterface& vb,
				const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet& bullet ) ;
		void CalcThicknessAndAlpha
			( float32_t& fpThickness, uint32_t& nAlpha,
						double index, uint32_t nTotalAlpha ) const ;
		void AddThunderInstance
			( S3DVertexBufferInterface& vb, uint32_t nTotalAlpha,
				const S3DBulletItemInterface::Bullet& bullet ) ;
		void AddBeamInstance
			( S3DVertexBufferInterface& vb,
				size_t nTrackCount, float32_t secLife ) ;
		void AddBeamInstance
			( S3DVertexBufferInterface& vb,
				const S3DVector4 * pvPoints,
				const float32_t * pThickness,
				const uint32_t * pAlpha,
				size_t nTrackCount, float32_t secLife ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾パーティクル放出コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletParticleController
				: public S3DBulletItemInterface::BulletController
	{
	public:
		enum	ParameterIndex
		{
			paramEmitterTarget,
			paramGenCount,
			paramGenCountByThickness,
			paramTiming,
			paramFlightTracks,
			paramDirType,
			paramDirection,
			paramSpeedDir,
			paramSpeed,
			paramSpeedScale,
			paramSpeedByThickness,
			paramZoomScale,
			paramZoomByThickness,
			paramFaceDir,
			paramUseColor,
			paramColorMul,
			paramColorAdd,
			paramAlpha,
			paramCount,
		} ;
		enum	TimingOfGeneration
		{
			timingFire,
			timingBeamFire,
			timingHit,
			timingHitPrepare,
			timingHitNoPrepare,
			timingFlight,
			timingPreMove,
			timingCount,
		} ;
		enum	DirectionType
		{
			dirDefault,
			dirMoveFront,
			dirMoveBack,
			dirHitNormal,
			dirHitReflect,
			dirOption,
			dirTypeCount,
		} ;
		static const wchar_t *	m_pwszTiming[timingCount] ;
		static const wchar_t *	m_pwszDirType[dirTypeCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBulletParticleController, BulletController )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletParticleController, bullet_particle )
		// 構築関数
		S3DBulletParticleController( void ) ;
		// 消滅関数
		virtual ~S3DBulletParticleController( void ) ;

	protected:
		SSystem::SSyncReference		m_refParticleTarget ;
		SSystem::SString			m_strParticleTarget ;

		size_t				m_nGenCount ;
		size_t				m_nTrackCount ;
		TimingOfGeneration	m_timing;
		DirectionType		m_dirGeneration ;
		DirectionType		m_dirSpeed ;
		DirectionType		m_dirFace ;
		S3DVector			m_vDirOption ;
		double				m_fpGenCountByThickness ;
		double				m_fpParticleScaleByThickness ;
		double				m_fpSpeedScaleByThickness ;
		double				m_fpGenSpeed ;
		double				m_fpGenSpeedScale ;
		double				m_fpZoomScale ;
		bool				m_flagUseColor ;
		S3DColor			m_clrParticle ;

	public:
		// 描画ターゲット設定
		void AttachParticleTarget
			( S3DParticleSerializer * pTarget, const wchar_t * pwszID ) ;
		bool UpdateParticleTarget( void ) ;
		// タイミング文字列解釈
		static TimingOfGeneration ParseTimingOfGeneration( const wchar_t * pwszType ) ;
		// 方向文字列解釈
		static DirectionType ParseDirectionType( const wchar_t * pwszType ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::Controller
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DBulletItemInterface::BulletListener
		// 発射時処理
		virtual void OnFireBullet
			( S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet& bullet ) ;
		// 着弾時処理
		virtual void OnHitBullet
			( S3DBulletItemInterface * pItem,
				S3DSceneComposer::ItemSerializer * pHitItem,
				const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;
		// タイマー前処理
		virtual void BeforeBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// タイマー処理
		virtual void OnBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 当たり判定追加処理
		virtual void RenderBulletCollision
			( const S3DScene& scene,
				S3DCollision& collision,
				const S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 弾軌跡描画（追加処理）
		virtual void RenderBulletTracks
			( S3DScene& scene, const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;

	protected:
		// パーティクル生成
		void GenerateParticles
			( size_t nCount, float32_t secPast,
				S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet& bullet,
				const S3DVector * pvHitPos, const S3DVector * pvHitNormal ) ;
		// 方向取得
		S3DVector GetParticleDirection
			( DirectionType dirType,
				const S3DVector& vDefault,
				const S3DVector& vSpeed,
				const S3DVector * pvHitNormal ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾効果音・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DBulletSoundController
				: public S3DBulletItemInterface::BulletController
	{
	public:
		enum	ParameterIndex
		{
			paramFireSound,
			paramFireVolume,
			paramHitSound,
			paramHitVolume,
			paramWindNoiseSound,
			paramWindNoiseVolume,
			paramCount,
		} ;
		class	SoundInstancePtr	: public ESLObject
		{
		public:
			SSystem::SSyncReference				m_refSound ;
			S3DSoundItemSerializer::Instance *	m_pInstance ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SoundInstancePtr, ESLObject )
			// 構築関数
			SoundInstancePtr
				( S3DSoundItemSerializer * pSound,
					S3DSoundItemSerializer::Instance * pInstance )
				: m_refSound( (S3DScene::SoundItem*) pSound ), m_pInstance( pInstance ) { }
			// 消滅関数
			virtual ~SoundInstancePtr( void )
			{
				S3DSoundItemSerializer *	pSound =
							m_refSound.GetRef<S3DSoundItemSerializer>() ;
				if ( pSound != nullptr )
				{
					pSound->RemoveInstance( m_pInstance ) ;
				}
			}
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBulletSoundController, BulletController )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletSoundController, bullet_sound )
		// 構築関数
		S3DBulletSoundController( void ) ;
		// 消滅関数
		virtual ~S3DBulletSoundController( void ) ;

	protected:
		SSystem::SSyncReference	m_refFireSound ;
		SSystem::SSyncReference	m_refHitSound ;
		SSystem::SSyncReference	m_refWindNoise ;
		SSystem::SString		m_strFireSound ;
		SSystem::SString		m_strHitSound ;
		SSystem::SString		m_strWindNoise ;
		double					m_fpFireVolume ;
		double					m_fpHitVolume ;
		double					m_fpWindNoiseVolume ;

	public:
		// 参照サウンドアイテム更新
		void UpdatreFireSoundRef( void ) ;
		void UpdatreHitSoundRef( void ) ;
		void UpdatreWindNoiseRef( void ) ;

	public:
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DBulletItemInterface::BulletListener
		// 発射時処理
		virtual void OnFireBullet
			( S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet& bullet ) ;
		// 着弾時処理
		virtual void OnHitBullet
			( S3DBulletItemInterface * pItem,
				S3DSceneComposer::ItemSerializer * pHitItem,
				const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;
		// タイマー前処理
		virtual void BeforeBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// タイマー処理
		virtual void OnBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 当たり判定追加処理
		virtual void RenderBulletCollision
			( const S3DScene& scene,
				S3DCollision& collision,
				const S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 弾軌跡描画（追加処理）
		virtual void RenderBulletTracks
			( S3DScene& scene, const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 弾サブコンポジション・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DSubCompositionSerializer ;
	class	S3DBulletSubCompositionController
				: public S3DBulletItemInterface::BulletController
	{
	public:
		enum	ParameterIndex
		{
			paramBulletSubComp,
			paramBulletMatrixOp,
			paramBulletSize,
			paramFireEffectSubComp,
			paramFireEffectMatrixOp,
			paramFireEffectSize,
			paramHitEffectSubComp,
			paramHitEffectMatrixOp,
			paramHitEffectSize,
			paramDropSubComp,
			paramDropMatrixOp,
			paramDropSize,
			paramDropInterval,
			paramDropRndPos,
			paramDropRndZoom,
			paramDropRndAngle1,
			paramDropRndAngle2,
			paramCount,
		} ;
		enum	MatrixOperation
		{
			matrixIdentity,
			matrixDirection,
			matrixOpCount,
		} ;

		static const wchar_t *	s_pwszMatrixOperation[matrixOpCount] ;
		static const SSystem::SXMLDocument::AttrInteger
								s_aiMatrixOperation[matrixOpCount+1] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DBulletSubCompositionController, BulletController )
		S3D_DECLARE_COMPOSER_ITEM( S3DBulletSubCompositionController, bullet_subcomp )
		// 構築関数
		S3DBulletSubCompositionController( void ) ;
		// 消滅関数
		virtual ~S3DBulletSubCompositionController( void ) ;

	protected:
		class	Container	: public ESLObject
		{
		public:
			SSystem::SSyncReference			m_refOwner ;
			S3DSceneComposer::Composition *	m_pComp ;
			size_t							m_nDropCount ;
		public:
			ESL_DECLARE_CLASS_INFO( Container, ESLObject )
			Container( S3DSubCompositionSerializer * pOwner,
						S3DSceneComposer::Composition * pComp ) ;
			virtual ~Container( void ) ;
		} ;

		SSystem::SSyncReference		m_refBulletSubComp ;
		SSystem::SString			m_strBulletSubComp ;
		MatrixOperation				m_matrixBullet ;
		S3DDVector					m_vBulletSize ;
		SSystem::SSyncReference		m_refFireEffectSubComp ;
		SSystem::SString			m_strFireEffectSubComp ;
		MatrixOperation				m_matrixFireEffect ;
		S3DDVector					m_vFireEffectSize ;
		SSystem::SSyncReference		m_refHitEffectSubComp ;
		SSystem::SString			m_strHitEffectSubComp ;
		MatrixOperation				m_matrixHitEffect ;
		S3DDVector					m_vHitEffectSize ;
		SSystem::SSyncReference		m_refDropSubComp ;
		SSystem::SString			m_strDropSubComp ;
		MatrixOperation				m_matrixDrop ;
		S3DDVector					m_vDropSize ;
		double						m_secDropInterval ;
		S3DDVector					m_vDropRndPos ;
		double						m_fpZoomRndRate ;
		S3DDVector					m_vRndEulerAngles1 ;
		S3DDVector					m_vRndEulerAngles2 ;

		SSystem::SCriticalSection	m_csEffects ;
		SSystem::SPointerArray
			<S3DSceneComposer::Composition>
									m_aFireEffects ;
		SSystem::SPointerArray
			<S3DSceneComposer::Composition>
									m_aHitEffects ;
		SSystem::SPointerArray
			<S3DSceneComposer::Composition>
									m_aDropEffects ;
		SakuraCL::SCLRandomizer		m_randomizer ;

	public:
		// 参照コンポジション更新
		void UpdatreBulletSubCompRef( void ) ;
		void UpdatreFireEffectSubCompRef( void ) ;
		void UpdatreHitEffectSubCompRef( void ) ;
		void UpdatreDropSubCompRef( void ) ;

	public:
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DBulletItemInterface::BulletListener
		// 発射時処理
		virtual void OnFireBullet
			( S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet& bullet ) ;
		// 着弾時処理
		virtual void OnHitBullet
			( S3DBulletItemInterface * pItem,
				S3DSceneComposer::ItemSerializer * pHitItem,
				const S3DCollision::MeshCollision * pmcHitMesh,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack ) ;
		// タイマー前処理
		virtual void BeforeBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
					S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// タイマー処理
		virtual void OnBulletTimer
			( S3DBulletItemInterface * pItem, float32_t secPast,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 当たり判定追加処理
		virtual void RenderBulletCollision
			( const S3DScene& scene,
				S3DCollision& collision,
				const S3DBulletItemInterface * pItem,
				S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;
		// 弾軌跡描画（追加処理）
		virtual void RenderBulletTracks
			( S3DScene& scene, const S3DBulletItemInterface * pItem,
				const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount ) ;

	protected:
		// インスタンス・クリーンアップ
		void CleanupSubCompositions
			( S3DSubCompositionSerializer * pSubComp,
				SSystem::SPointerArray<S3DSceneComposer::Composition>& aEffects ) ;
		// サブコンポジション位置設定
		void ReflectSubCompositionMatrix
			( S3DSceneComposer::Composition * pInstance,
				const S3DSubCompositionSerializer& subcomp,
				const S3DBulletItemInterface::Bullet& bullet,
				const S3DDMatrix& matBase, const S3DDVector& vBase,
				MatrixOperation matrixOp, const S3DDVector& vSize ) ;
	} ;

}

#endif
