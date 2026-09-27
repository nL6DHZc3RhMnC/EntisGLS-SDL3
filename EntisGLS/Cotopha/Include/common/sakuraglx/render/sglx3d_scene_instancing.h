
#if	!defined(__SAKURAGLX3D_SCENE_INSTANCING_H__)
#define	__SAKURAGLX3D_SCENE_INSTANCING_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// インスタンスエントリ・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DInstancingEntryInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DInstancingEntryInterface, ESLObject ) ;
		// インスタンシング・リスト取得
		virtual size_t GetInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const = 0 ;
	} ;

	class	S3DInstancingEntryEditInterface	: public S3DInstancingEntryInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DInstancingEntryEditInterface, S3DInstancingEntryInterface ) ;
		// インスタンス削除
		virtual void RemoveInstancingEntryAt( size_t i ) = 0 ;
	} ;

	class	S3DInstanceEntryController
				: public S3DSceneComposer::Controller,
					public S3DInstancingEntryInterface
	{
	protected:
		size_t			m_iParamPosition ;
		size_t			m_iParamRotation ;
		size_t			m_iParamZoom ;
		size_t			m_iParamTransparency ;
		size_t			m_iParamColorMul ;
		size_t			m_iParamColorAdd ;

		S3DVector		m_vPosition ;
		S3DMatrix		m_matRotation ;
		S3DVector		m_vZoom ;
		S4DMatrix		m_matInstance ;
		S3DColor		m_clrInstance ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DInstanceEntryController, Controller, S3DInstancingEntryInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DInstanceEntryController, instance_entry )
		// 構築関数
		S3DInstanceEntryController( void ) ;

	public:
		// 座標
		const S3DVector& GetPosition( void ) const ;
		void SetPosition( const S3DVector& vPos ) ;
		// 回転
		const S3DMatrix& GetRotation( void ) const ;
		void SetRotation( const S3DMatrix& matRot ) ;
		// 拡大
		const S3DVector& GetZoom( void ) const ;
		void SetZoom( const S3DVector& vZoom ) ;
		// 色効果・α
		const S3DColor& GetColorEffect( void ) const ;
		void SetColorEffect( const S3DColor& clrEffect ) ;
		// インスタンス
		void GetInstanceEntry( S4DMatrix& mat, S3DColor& clr ) const ;
	protected:
		void CalcInstanceMatrix( void ) ;

	public:	// S3DInstancingEntryInterface
		// インスタンシング・リスト取得
		virtual size_t GetInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// インスタンシング・シリアライザ（共通処理）
	//////////////////////////////////////////////////////////////////////////

	class	S3DItemInstancingSerializer
	{
	public:
		enum	SortingMethod
		{
			sortNothing,
			sortHalfAlphaItems,
			sortAllItems,
			sortMethodCount,
		} ;
		enum	CullingMethod
		{
			cullingNothing,
			cullingByZ,
			cullingByFrustum,
			CullingMethodCount,
		} ;
		static const wchar_t *	m_pwszSortingMethodName[sortMethodCount] ;
		static const wchar_t *	m_pwszCullingMethodName[CullingMethodCount] ;

		// インデックス参照オブジェクト
		class	RefIndex	: public SSystem::SObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( RefIndex, SObject )
		public:
			size_t	m_iInstanceIndex ;
		} ;

		class	InstanceRef
		{
		public:
			SSystem::SSyncReference									m_refInstance ;
			SSystem::SSmartRef<S3DSceneComposer::ItemSerializer>	m_refItem ;

		public:
			// 構築
			InstanceRef( void ) {}
			InstanceRef( const InstanceRef& ref )
				: m_refInstance( ref.m_refInstance ),
					m_refItem( ref.m_refItem ) { }
			InstanceRef( S3DSceneComposer::ItemSerializer * pItem, RefIndex * pRefIndex )
				: m_refInstance( pRefIndex ), m_refItem( pItem ) {}
			// 代入
			const InstanceRef& operator = ( const InstanceRef& ref ) ;
			// 参照先がなくなったか？
			bool IsEmpty( void ) const ;
			// 参照先が同一か？
			bool IsEqual( const InstanceRef& ref ) const ;
			// 参照解放
			void ReleaseRef( void ) ;
			// 参照先アイテム
			S3DSceneComposer::ItemSerializer * GetRefItem( void ) const ;
			// インスタンス参照か？
			bool IsInstance( void ) const ;
			// インスタンス指標取得
			ssize_t GetInstanceIndex( void ) const ;
		} ;

		// インスタンス・ソート情報
		struct	SortIndex
		{
			float32_t	z ;
			uint32_t	i ;			// 元のインデックス（フレーム中に削除されると -1）
		} ;

		// 描画情報
		struct	InstanceRange
		{
			size_t	nIndex ;
			size_t	nCount ;
		} ;
		struct	RenderInfo
		{
			S3DVertexBufferInterface *	pModel ;
			uint64_t					flagsExclusion ;
			size_t						iMeshFirst ;
			ssize_t						iMeshEnd ;
			size_t						nInstanceCount ;
			const S4DMatrix *			pInstanceMatrices ;
			const S3DColor *			pInstanceColors ;
			const SortIndex *			pInstanceSortIndexes ;
			size_t						nInstanceOpaqueCount ;
			size_t						nRangeCount ;
			size_t						nRangeBufSize ;
			InstanceRange *				pInstanceRanges ;
		} ;
		enum	RenderingType
		{
			renderStaticInstance,
			renderDynamicInstance,
			renderVariantInstance,
		} ;
		enum	RenderResult
		{
			renderFinished,
			renderContinue,
		} ;
		class	MultiRenderer	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MultiRenderer, ESLObject ) ;
			// 描画処理
			virtual RenderResult RenderMultiInstance
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					const S3DItemInstancingSerializer& instancing,
					RenderingType type,
					RenderInfo& info ) = 0 ;
		} ;

	protected:
		SSystem::SString			m_strInstancingBase64 ;
		S3DDQuaternion				m_qRotation ;
		S3DDVector					m_vZoom ;
		S3DMatrix					m_matInstanceBase ;
		S3DMatrix					m_matFaceDirection ;
		bool						m_flagFaceDirection ;
		bool						m_flagPotentialMode ;
		SSystem::SArray<S3DMatrix>	m_aInstanceOrgMatrixs ;
		SSystem::SArray<S4DMatrix>	m_aInstanceMatrixs ;
		SSystem::SArray<S3DColor>	m_aInstanceColors ;
		SSystem::SArray<S4DMatrix>	m_aTempMatrixs ;
		SSystem::SArray<S3DColor>	m_aTempColors ;
		SSystem::SPointerArray<ESLObject>
									m_aTempSourceItems ;
		SSystem::SArray<size_t>		m_aTempSourceIndexes ;

		SortingMethod				m_sorting ;
		CullingMethod				m_culling ;
		float32_t					m_zNear ;
		float32_t					m_zFar ;
		SSystem::SArray<S4DMatrix>	m_aDynamicMatrixs ;
		SSystem::SArray<S3DColor>	m_aDynamicColors ;
		SSystem::SArray<S4DMatrix>	m_aDynamicMatrixs2 ;
		SSystem::SArray<S3DColor>	m_aDynamicColors2 ;

		size_t						m_nOpaqueCount ;
		SSystem::SArray<SortIndex>	m_aDynamicSort ;	// 表示アイテムのみ
		SSystem::SArray<size_t>		m_aDynamicIndex ;	// すべてのアイテム（フレーム中の削除によるインデックス変更へ対応）

		SSystem::SObjectArray<RefIndex>
									m_aRefIndexes ;

		SSystem::SCriticalSection	m_csLock ;

	public:
		// 視錐台
		struct	FrustumInfo
		{
			SGLImageRect	rectView ;
			S3DVector		vScreen ;
			double			zScale, fpPixelAspect ;
			S3DVector		vFrame[4] ;
		} ;

		// インスタンス情報
		enum	InstanceType
		{
			typeNothing,
			typeItem,
			typeController,
			typeStaticInstance,
		} ;
		struct	InstanceInfo
		{
			InstanceType						type ;
			const S3DCollision::MeshCollision *	pMeshCol ;
			S3DItemInstancingSerializer *		pInstancing ;
			S3DSceneComposer::ItemSerializer *	pItem ;
			S3DInstancingEntryInterface *		pEntry ;
			size_t								iController ;
			size_t								iInstance ;		// コントローラーなどのローカル指標
			size_t								iAbsInstance ;	// S3DItemInstancingSerializer に対する全体指標
			S4DMatrix							mat4Instance ;
			S3DColor							clrInstance ;

			InstanceInfo( void )
				: type( typeNothing ), pMeshCol( NULL ),
					pInstancing( NULL ), pItem( NULL ), pEntry( NULL ),
					iController( 0 ), iInstance( 0 ), iAbsInstance( 0 ),
					mat4Instance( 1, 1, 1, 1 ), clrInstance( 0xFFFFFFFF, 0 ) { }
			InstanceInfo( const InstanceInfo& info )
				: type( info.type ), pMeshCol( info.pMeshCol ),
					pInstancing( info.pInstancing ),
					pItem( info.pItem ), pEntry( info.pEntry ),
					iController( info.iController ),
					iInstance( info.iInstance ), iAbsInstance( info.iAbsInstance ),
					mat4Instance( info.mat4Instance ), clrInstance( info.clrInstance ) { }
			const InstanceInfo& operator = ( const InstanceInfo& info ) ;
			bool IsEqual( const InstanceInfo& info ) const ;
			bool IsEmpty( void ) const ;
			bool IsInstance( void ) const ;
			SSystem::SObject * GetRefObject( void ) const ;
			void GetInstanceRef( InstanceRef& ref ) const ;
			bool GetSafeInstanceMatrix
				( S4DMatrix& matrix, S3DColor& color, const InstanceRef& ref ) const ;
			bool GetSafeGlobalMatrix
				( S3DDMatrix& matrix, S3DDVector& pos, S3DColor& color,
											const InstanceRef& ref ) const ;
		} ;

	public:
		// 構築関数
		S3DItemInstancingSerializer( void ) ;
		// モデル基本回転
		const S3DDQuaternion& GetBaseRotation( void ) const
		{
			return	m_qRotation ;
		}
		void SetBaseRotation( const S3DDQuaternion& qRot ) ;
		// モデル基本拡大率
		const S3DDVector& GetBaseZoom( void ) const
		{
			return	m_vZoom ;
		}
		void SetBaseZoom( const S3DDVector& vZoom ) ;
		// 面の方向を固定する
		bool IsForceFaceDirection( void ) const
		{
			return	m_flagFaceDirection ;
		}
		void ForceFaceDirection( bool flagEnable ) ;
		void SetFaceDirection( const S3DMatrix& matFaceDir ) ;
		// ソート方式
		SortingMethod GetSorting( void ) const
		{
			return	m_sorting ;
		}
		const wchar_t * GetSortingName( void ) const
		{
			return	m_pwszSortingMethodName[m_sorting] ;
		}
		void SetSorting( SortingMethod sort ) ;
		void SetSortingByName( const wchar_t * pwszName ) ;
		// カリング方式
		CullingMethod GetCulling( void ) const
		{
			return	m_culling ;
		}
		const wchar_t * GetCullingName( void ) const
		{
			return	m_pwszCullingMethodName[m_culling] ;
		}
		void SetCulling( CullingMethod culling ) ;
		void SetCullingByName( const wchar_t * pwszName ) ;
		// ｚ範囲
		float32_t GetCullingNearZ( void ) const
		{
			return	m_zNear ;
		}
		float32_t GetCullingFarZ( void ) const
		{
			return	m_zFar ;
		}
		void SetCullingNearZ( float32_t zNear ) ;
		void SetCullingFarZ( float32_t zFar ) ;
		// 基本変換行列反映
		void UpdateBaseMatrix( void ) ;

	public:
		// インスタンシング・リスト設定
		void SetInstancingEntriesBase64( const wchar_t * pwszBase64 ) ;
		void SetInstancingEntries
				( const S3DSceneComposer::BinaryInstancingData& bid ) ;
		// インスタンシング・リスト取得
		const SSystem::SString& GetInstancingEntriesBase64( void ) const
		{
			return	m_strInstancingBase64 ;
		}
		size_t GetInstancingDataLengthInBytes( void ) const ;
		size_t GetInstancingData( S3DSceneComposer::BinaryInstancingData& bid ) const ;
		// インスタンス・データ・パース
		static S3DSceneComposer::BinaryInstancingData *
			ParseData( SSystem::SArray<uint8_t>& aBinary,
						const wchar_t * pwszBase64, ssize_t nStrLenght = -1 ) ;
		// インスタンス・データ・エンコード
		static void EncodeData
			( SSystem::SString& strBase64,
				const S3DSceneComposer::BinaryInstancingData& bid ) ;
	protected:
		void UpdateInstancingEntriesFromBase64( void ) ;
		void UpdateInstancingEntries
				( const S3DSceneComposer::BinaryInstancingData& bid ) ;

	public:
		// 静インスタンス数取得
		size_t GetStaticInstanceCount( void ) const ;
		// 静インスタンス取得
		bool GetStaticInstanceAt
			( size_t i, S4DMatrix& matrix, S3DColor& color ) const ;
		// 静インスタンス挿入
		void InsertStaticInstanceAt
			( size_t nIndex, const S4DMatrix& matrix, const S3DColor& color,
					S3DSceneComposer::ItemSerializer * pNotifyItem = NULL ) ;
		void NotifyOnInsertedInstance
			( size_t nIndex, const S4DMatrix& matrix, const S3DColor& color,
					S3DSceneComposer::ItemSerializer * pNotifyItem ) ;
		// 静インスタンス削除
		void RemoveStaticInstanceAt
			( size_t nIndex, S3DSceneComposer::ItemSerializer * pNotifyItem = NULL ) ;
		void NotifyOnRemovedInstance
			( size_t nIndex, S3DSceneComposer::ItemSerializer * pNotifyItem ) ;
		// 静インスタンスを m_aInstanceOrgMatrixs にも複製して base64 文字列更新
		void UpdateStaticInstancingEntriesBase64( void ) ;
		// base64 文字列更新
		void UpdateInstancingEntriesBase64( void ) ;

	public:
		// ダイナミック・インスタンス・リセット
		void ResetDynamicInstancingEntries( void ) ;
		// インスタンシング・エントリ・コントローラー反映更新
		void UpdateDynamicInstancingEntries
				( const S3DSceneComposer::ItemSerializer * pItem ) ;
		// PotentialMode 用の初期化呼び出し
		void NotifyResetPotentialInstance
				( const S3DSceneComposer::ItemSerializer * pItem ) ;
		// 追加用バッファ処理
		void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors,
				size_t nCount, ESLObject * pSourceItem ) ;
		// ダイナミック・インスタンス（ソート前の静インスタンスを含む指標）取得
		bool GetDynamicInstanceAt
			( size_t i, S4DMatrix& matrix, S3DColor& color ) const ;

	protected:
		// PotentialMode へ静的インスタンスを移行
		void MakeStaticInstancePotentialMode
			( const S3DSceneComposer::ItemSerializer * pItem ) ;
		// Potential なインスタンスを更新
		void UpdatePotentialInstance
			( const S4DMatrix* pMatrixs,
				const S3DColor * pColors, size_t nCount ) ;

	public:
		// インスタンスの参照取得
		InstanceRef GetInstanceRefAt
			( S3DSceneComposer::ItemSerializer * pItem, size_t iInsyanceIndex ) ;
		// インスタンス指標への参照オブジェクトを取得
		RefIndex * GetIndexReference( size_t iInstanceIndex ) ;
		// インスタンス指標取得（存在しない場合には -1）
		ssize_t GetReferenceIndexOf( RefIndex * pRefIndex ) const ;

	public:
		// インスタンス・インデックス・リセット
		void ResetInstanceIndex( void ) ;
		// 動的処理が必要か？
		bool IsNeededDynamicProcess( void ) const
		{
			return	m_flagFaceDirection
					| (m_sorting != sortNothing) | (m_culling != cullingNothing) ;
		}
		// 動的処理実行
		void DoDynamicProcess
			( S3DScene& scene, const S3DDMatrix & matdModel,
				const S3DDVector& vdModel,
				float32_t fpInstanceSize, float32_t radPlayOfAngle ) ;
		// 絶対インデックス（フレーム開始時のアイテム削除の影響のない）から
		// 削除の影響を受けた実際のインスタンス・インデックスを取得（削除済みは -1）
		// （静的インスタンス＋動的インスタンス統合順）
		size_t InstanceIndexOfAbsInstance( size_t iAbsInstance ) const ;
		// 描画されたインデックスからソート前のインスタンス・インデックスを取得
		// （静的インスタンス＋動的インスタンス統合順）
		size_t InstanceIndexOfSortedInstance( size_t iSortedInstance ) const ;
		// インスタンス・インデックスから絶対インデックスを取得
		size_t AbsInstanceOfInstanceIndex( size_t iInstanceIndex ) const ;
		// インデックスを挿入する
		void InsertIndexByInstanceIndex( size_t iInstanceIndex ) ;
		// インデックスを削除する
		bool DeleteIndexByAbsInstance( size_t iAbsInstance ) ;
		bool DeleteIndexByInstanceIndex( size_t iInstanceIndex ) ;
		// AddDynamicInstancingEntries で設定されたソースを取得する
		ESLObject * GetSourceItemOfAbsInstance
						( size_t& iSubIndex, size_t iAbsInstance ) const ;
		// インスタンス情報取得
		static InstanceType GetInstanceInfo
			( InstanceInfo& info,
				const S3DCollision::MeshCollision * pMeshCol ) ;
		static InstanceType GetInstanceInfo
			( InstanceInfo& info,
				S3DSceneComposer::ItemSerializer * pItem,
				size_t iAbsInstance,
				const S3DCollision::MeshCollision * pMeshCol ) ;
		// インスタンス挿入に対応するインスタンス指標を修正
		static bool ModifyOnInsertInstance
			( InstanceInfo& info,
				S3DItemInstancingSerializer * pInstancing, size_t iInstance ) ;
		// インスタンス削除に対応するインスタンス指標を修正
		static bool ModifyOnRemoveInstance
			( InstanceInfo& info,
				S3DItemInstancingSerializer * pInstancing, size_t iInstance ) ;
		// 参照先を安全に保管する
		static void SaveInstanceReference( InstanceRef& ref, const InstanceInfo& info ) ;
		// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して現在のフレーム用に修正
		// InstanceInfo は ModifyOnRemoveInstance の操作を含む
		static bool CorrectInstanceInfo( InstanceInfo& info, const InstanceRef& ref ) ;
		// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して
		// 安全に現在のフレームでのインスタンス行列（ローカル）を取得
		static bool GetSafeInstanceMatrixOf
			( S4DMatrix& matrix, S3DColor& color,
				const InstanceInfo& info, const InstanceRef& ref ) ;
		// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して
		// 安全に現在のフレームでのインスタンス行列（グローバル）を取得
		static bool GetSafeInstanceGlobalMatrixOf
			( S3DDMatrix& matrix, S3DDVector& pos, S3DColor& color,
				const InstanceInfo& info, const InstanceRef& ref ) ;
		// アイテムに対するインスタンス指標を取得
		//（コントローラーの場合、コントローラー内のインスタンス番号になるため）
		static size_t GetInstanceIndexOfItem
			( const InstanceInfo& info, size_t	iInstance ) ;
		// インスタンス空間取得
		static bool GetInstanceSpace
			( S3DDMatrix& matrix, S3DDVector& vPos, const InstanceInfo& info ) ;
		// カリング判定準備
		void PrepareCullingInfo
			( FrustumInfo& fi, float32_t sinPlayOfHAngle, float32_t sinPlayOfVAngle ) const ;
		// カリング判定
		bool IsCullingInstance
			( const FrustumInfo& fi,
				/*const S3DMatrix& mat3,*/ const S3DVector& pos,
				float32_t fpInstanceSize ) const ;
	protected:
		// 面の方向を強制する
		void ForceInstanceFaceDirection
				( S4DMatrix& mat4, const S3DMatrix& mat3Face /*, const S3DVector& vCameraDir*/ ) const ;
		// ソート実行
		static void DoSortIndex( SortIndex * psi, size_t nCount ) ;

	public:
		// インスタンシング・リスト取得
		size_t GetStaticInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;
		size_t GetDynamicInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;
		size_t GetProcessedInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;
		// ソート情報取得
		const SortIndex * GetProcessedInstanceSortIndexArray
					( size_t& nTotalCount, size_t& nOpaqueCount ) const ;

	public:
		// レンダリング
		void RenderMultiInstance
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				S3DSceneComposer::ItemSerializer& item,
				S3DVertexBufferInterface *	pModel,
				RenderingType type,
				uint64_t flagsExclusion = 0,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = nullptr,
				const S3DColor * pColorInstancing = nullptr,
				const SortIndex * pSortIndexes = nullptr,
				size_t nOpaqueCount = 0 ) const ;

	public:
		// スレッド排他処理
		void Lock( void ) const ;
		void Unlock( void ) const ;
		atomic_int_t UnlockAll( void ) const ;
		void Relock( atomic_int_t nLock ) const ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// アイテム／インスタンス参照オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	S3DItemInstanceRef	: public SSystem::SObject,
									public S3DItemInstancingSerializer::InstanceRef
	{
	protected:
		S3DItemInstancingSerializer::InstanceInfo	m_info ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DItemInstanceRef, SObject )
		// 構築関数
		S3DItemInstanceRef( void ) ;
		S3DItemInstanceRef( const S3DItemInstanceRef& ref ) ;
		S3DItemInstanceRef
			( const S3DCollision::MeshCollision * pMeshCol ) ;
		S3DItemInstanceRef
			(  S3DSceneComposer::ItemSerializer * pItem, size_t iAbsInstance ) ;
		// 代入
		const S3DItemInstanceRef& operator = ( const S3DItemInstanceRef& ref ) ;

		// 異なるフレームで InstanceInfo を現在のフレーム用に修正する
		bool CorrectInstance( void ) ;
		// 安全に現在のフレームでのインスタンス行列（ローカル）を取得
		bool GetInstanceMatrix( S4DMatrix& matrix, S3DColor& color ) ;
		// インスタンスを削除する
		bool DeleteInstance( void ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// インスタンシング・抽象アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DInstancingItemInterface	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DInstancingItemInterface, SObject )
		// インスタンス処理（classPreRender で呼び出す）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors,
				size_t nCount, ESLObject * pSrcItem ) = 0 ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) = 0 ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// マルチインスタンスアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DPotentialInstancingInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DPotentialInstancingInterface, ESLObject ) ;
		// インスタンシング・リスト初期化（ローカル座標）
		virtual void ResetInstance
			( const S4DMatrix* pMatrixs,
				const S3DColor* pColors, size_t nCount ) = 0 ;
		// インスタンス処理（ローカル座標）
		virtual bool ProcessInstance
			( const S4DMatrix* pMatrixs,
				const S3DColor* pColors, size_t nCount ) = 0 ;
		// 処理済みインスタンシング・リスト取得（ローカル座標）
		virtual size_t GetPotentialInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const = 0 ;
		// インスタンス挿入通知
		virtual void OnInsertedInstance
			( S3DItemInstancingSerializer * pInstancing,
				size_t nIndex, const S4DMatrix& matrix, const S3DColor& color ) = 0 ;
		// インスタンス削除通知
		virtual void OnRemovedInstance
			( S3DItemInstancingSerializer * pInstancing, size_t nIndex ) = 0 ;
	} ;

	class	S3DMultiInstanceSerializer
				: public S3DSceneComposer::ItemBasicSerializer,
					public S3DInstancingItemInterface
	{
	public:
		enum	ParameterIndex
		{
			paramInstanceTarget	= ItemBasicSerializer::paramItemTotalCount,
			paramInstancing,
			paramInstanceRotate,
			paramInstanceZoom,
			paramMultiInstanceTotalCount,
			paramMultiInstanceCount	= paramMultiInstanceTotalCount - paramInstanceTarget,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramMultiInstanceCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DMultiInstanceSerializer,
					ItemBasicSerializer, S3DInstancingItemInterface )
		S3D_DECLARE_COMPOSER_ITEM
			( S3DMultiInstanceSerializer, multi_instance )
		// 構築関数
		S3DMultiInstanceSerializer( void ) ;

	protected:
		SSystem::SString			m_strTargetID ;
		SSystem::SSyncReference		m_refTargetItem ;

		SSystem::SArray<S4DMatrix>	m_bufMatrix ;
		SSystem::SArray<S3DColor>	m_bufColor ;
		SSystem::SArray<S4DMatrix>	m_bufTempMatrix ;
		SSystem::SArray<S3DColor>	m_bufTempColor ;
		SSystem::SArray<S3DMatrix>	m_bufFaceMatrix ;
		SSystem::SArray<S3DVector4>	m_bufPoint ;
		SSystem::SArray<S4DVector>	m_bufFaceDir ;
		SSystem::SArray<float32_t>	m_bufZoom ;
		SSystem::SArray<S3DParticleSerializer::ParticleIndex>
									m_bufIndex ;

		S3DItemInstancingSerializer	m_instancing ;

	public:
		// ターゲット設定
		void AttachInstancingTarget
			( S3DInstancingItemInterface * pTarget, const wchar_t * pwszID ) ;
		bool UpdateInstancingTarget( void ) ;
		// ターゲット取得
		S3DSceneComposer::ItemSerializer * GetInstancingTarget( void ) const ;
		// S3DItemInstancingSerializer 取得
		S3DItemInstancingSerializer& Instancing( void )
		{
			return	m_instancing ;
		}

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:	// S3DInstancingItemInterface
		// 動的インスタンス追加（classPreRender で呼び出す／但しこのアイテムより先）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
					const S3DColor * pColors,
					size_t nCount, ESLObject * pSourceItem ) ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) ;

	public:	// ItemSerializer
		// フレーム更新後処理
		virtual void OnUpdateFrame
				( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// Rosetta インスタンス取得
		virtual Rosetta::RSObject * GetRosettaInstanceOf( const wchar_t * pwszClass ) const ;
		// アイテムのプライマリモデル取得
		virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
		// アイテムのコリジョンバッファ取得
		virtual S3DCollider * GetItemPrimaryCollider( void ) ;
		// スクリプト・インスタンス取得
		virtual class S3DSceneScriptInstance * GetScriptInstanceOf( const wchar_t * pwszClass ) const ;

	protected:
		// インスタンス収集
		void UpdateInstancingArray( void ) ;
		// 座標変換
		const S4DMatrix * TransformInstanceMatrics
			( S3DSceneComposer::ItemSerializer * pTargetItem,
							const S4DMatrix * pMatrixs, size_t nCount ) ;
		void TransformParticleInstances
			( S3DParticleSerializer::RenderTarget * pRenderTarget,
							const S4DMatrix * pMatrixs, size_t nCount ) ;
		// 色効果
		const S3DColor * EffectInstanceColors
			( const S3DColor * pColors, size_t nCount ) ;
		// インスタンス出力
		void AddParticleInstances
			( S3DParticleSerializer::RenderTarget * pRenderTarget,
				const S4DMatrix * pMatrixs, const S3DColor * pColors, size_t nCount ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// インスタンス周期回転コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DInstanceRotationController
					: public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramBaseRotation,
			paramRotationAxisType,
			paramRotationAxisVector,
			paramRotationOnAxis,
			paramRotationSpeed,
			paramRotationSpeedAmp,
			paramRotationSpeedCycle,
		} ;
		enum	RotationAxisType
		{
			rotationAxisX,
			rotationAxisY,
			rotationAxisZ,
			rotationAxisVector,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiRotationAxisType[5] ;

	protected:
		S3DDMatrix			m_matBaseRotation ;
		RotationAxisType	m_rxtType ;
		S3DDVector			m_vRotAxis ;
		double				m_degRotation ;
		double				m_degRotCurrent ;
		double				m_secSpeedPhase ;
		double				m_dpsRotSpeed ;
		double				m_dpsRotSpeedAmp ;
		double				m_dpsRotSpeedCycle ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DInstanceRotationController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DInstanceRotationController, instance_rotator )
		// 構築関数
		S3DInstanceRotationController( void ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t iParam ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

		// 回転反映
		void ReflectInstanceBaseRotation( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 動的インスタンス制御テンプレート
	//////////////////////////////////////////////////////////////////////////

	template <class T> class S3DBasicPotentialInstancing
								: public S3DPotentialInstancingInterface
	{
	public:
		class	Instance	: public T
		{
		public:
			S4DMatrix	m_mat4 ;
			S3DColor	m_color ;
		public:
			Instance( void )
				: m_mat4( 1, 1, 1, 1 ), m_color( 0xFFFFFFFF, 0 ) { }
			Instance( const Instance& src )
				: T( src ), m_mat4( src.m_mat4 ), m_color( src.m_color ) { }
			Instance( const S4DMatrix& mat4, const S3DColor& color )
				: m_mat4( mat4 ), m_color( color ) { }
		} ;

	protected:
		SSystem::SObjectArray<Instance>	m_aInstance ;
		SSystem::SArray<S4DMatrix>		m_bufMatrix ;
		SSystem::SArray<S3DColor>		m_bufColor ;

	public:
		// インスタンシング・リスト初期化（ローカル座標）
		virtual void ResetInstance
			( const S4DMatrix* pMatrixs,
				const S3DColor* pColors, size_t nCount )
		{
			m_aInstance.SetLength( nCount ) ;
			m_bufMatrix.SetLength( nCount ) ;
			m_bufColor.SetLength( nCount ) ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				m_aInstance.SetAt( i, new Instance( pMatrixs[i], pColors[i] ) ) ;
			}
		}
		// インスタンス処理（ローカル座標）
		virtual bool ProcessInstance
			( const S4DMatrix* pMatrixs,
				const S3DColor* pColors, size_t nCount )
		{
			Instance *const*	ppInstance = m_aInstance.GetConstArray() ;
			nCount = m_aInstance.GetLength() ;
			S4DMatrix *	pDstMatrix = m_bufMatrix.GetArray( nCount ) ;
			S3DColor *	pDstColor = m_bufColor.GetArray( nCount ) ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				ESLAssert( ppInstance[i] != NULL ) ;
				pDstMatrix[i] = ppInstance[i]->m_mat4 ;
				pDstColor[i] = ppInstance[i]->m_color ;
			}
			m_bufMatrix.FinishArray() ;
			m_bufColor.FinishArray() ;
			return	true ;
		}
		// 処理済みインスタンシング・リスト取得（ローカル座標）
		virtual size_t GetPotentialInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
		{
			ESLAssert( m_bufMatrix.GetLength() >= m_aInstance.GetLength() ) ;
			ESLAssert( m_bufColor.GetLength() >= m_aInstance.GetLength() ) ;
			pMatrixs = m_bufMatrix.GetConstArray() ;
			pColors = m_bufColor.GetConstArray() ;
			return	m_aInstance.GetLength() ;
		}
		// インスタンス挿入通知
		virtual void OnInsertedInstance
			( S3DItemInstancingSerializer * pInstancing,
				size_t nIndex, const S4DMatrix& matrix, const S3DColor& color )
		{
			m_aInstance.InsertAt( nIndex, new Instance( matrix, color ) ) ;
		}
		// インスタンス削除通知
		virtual void OnRemovedInstance
			( S3DItemInstancingSerializer * pInstancing, size_t nIndex )
		{
			m_aInstance.RemoveAt( nIndex ) ;
		}
		// インスタンス取得
		Instance * GetInstanceAt( size_t nIndex ) const
		{
			return	m_aInstance.GetAt( nIndex ) ;
		}
		// インスタンス検索
		ssize_t FindInstance( Instance * pInstance ) const
		{
			return	m_aInstance.FindPtr( pInstance ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 物理演算動的インスタンス
	//////////////////////////////////////////////////////////////////////////

	class	S3DOPhysicsDynamicInstancingInterface
						: public S3DInstancingEntryEditInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( S3DOPhysicsDynamicInstancingInterface, S3DInstancingEntryEditInterface ) ;
		// 構築関数
		S3DOPhysicsDynamicInstancingInterface( void ) ;
		// 消滅関数
		virtual ~S3DOPhysicsDynamicInstancingInterface( void ) ;

	protected:
		SSystem::SArray<S4DMatrix>						m_aMatrixs ;
		SSystem::SArray<S3DColor>						m_aColors ;
		SSystem::SObjectArray<S3DPhysicsScene::Actor>	m_aActors ;
		SSystem::SObjectArray<S3DPhysicsScene::Actor>	m_aRemoveActors ;

		S3DMatrix										m_matBase ;		// ベース行列
		S3DPhysicsScene::Material						m_material ;	// 物性
		bool											m_flagCollider ;// 物理演算当たり判定
		S3DCollision::ColliderDescription				m_collider ;	// 当たり判定形状
		SSystem::SArray<S3DVector4>						m_aHitPoint ;	// 当たり判定座標（d は半径）
																		// 初めの要素は接地判定にも使用
		uint32_t										m_maskColliderClasses ;

	public:
		// インスタンスのベース行列
		const S3DMatrix& GetBaseMatrix( void ) const
		{
			return	m_matBase ;
		}
		void SetBaseMatrix( const S3DMatrix& matBase )
		{
			m_matBase = matBase ;
		}
		// 物性
		const S3DPhysicsScene::Material& GetPhysMaterial( void ) const
		{
			return	m_material ;
		}
		void SetPhysMaterial( const S3DPhysicsScene::Material& material )
		{
			m_material = material ;
		}
		// 当たり判定形状
		const S3DCollision::ColliderDescription& GetColliderDescription( void ) const
		{
			return	m_collider ;
		}
		void SetColliderDescription( const S3DCollision::ColliderDescription& collider )
		{
			m_collider = collider ;
		}
		// 当たり判定点座標
		const S3DVector4 * GetHitPointList( size_t& nCount ) const
		{
			nCount = m_aHitPoint.GetLength() ;
			return	m_aHitPoint.GetConstArray() ;
		}
		void SetHitPointList( const S3DVector4 * pvHitPoints, size_t nCount )
		{
			m_aHitPoint.RemoveAll() ;
			m_aHitPoint.AddArray( pvHitPoints, nCount ) ;
		}
		// 当たり判定クラス
		uint32_t GetColliderUserClasses( void ) const
		{
			return	m_maskColliderClasses ;
		}
		void SetColliderUserClasses( uint32_t maskClasses )
		{
			m_maskColliderClasses = maskClasses ;
		}

	public:
		// アクターをインスタンスに更新
		void UpdateInstanceEntries
			( const S3DDMatrix& matSpace, const S3DDVector& vSpace ) ;
		// アクターを S3DPhysicsScene へ追加（コライダも追加）
		void AddActorToPhysicsScene( S3DPhysicsScene& scene ) ;

	public:
		// 全インスタンス削除
		void RemoveAllInstance( void ) ;
		// インスタンス追加
		size_t AddInstance
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors, size_t nCount,
				const S3DVector * pSpeeds = nullptr,
				const S4DVector * pRotSpeeds = nullptr,
				const void * pExInitData = nullptr, size_t nExDataStride = 0 ) ;
		// インスタンス数取得
		size_t GetInstanceCount( void ) const ;
		// インスタンス・アクター取得
		S3DPhysicsScene::Actor * GetInstanceActorAt( size_t i ) const ;
		// インスタンス色
		S3DColor * GetInstanceColorAt( size_t i ) const ;
		// アクター生成
		virtual S3DPhysicsScene::Actor * NewActor( const void * pExInitData = nullptr ) const ;

	public:	// S3DInstancingEntryInterface
		// インスタンシング・リスト取得
		virtual size_t GetInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;

	public:	// S3DInstancingEntryEditInterface
		// インスタンス削除
		virtual void RemoveInstancingEntryAt( size_t i ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 物理演算動的インスタンスコントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DOPhysicsInstancingController
				: public S3DSceneComposer::Controller,
					public S3DOPhysicsDynamicInstancingInterface
	{
	public:
		enum	ParameterIndex
		{
			paramInstancing,
			paramRotation,
			paramZoom,
			paramReflectInstance,
			paramNoRotation,
			paramNoMove,
			paramWeight,
			paramRotWeight,
			paramVolume,
			paramStillFriction,
			paramResistance,
			paramElasticity,
			paramActorCollision,
			paramCollisionShape,
			paramCollisionCenter,
			paramCollisionRadius,
			paramCollisionCubeSize,
			paramColliderMarkerType,
			paramColliderFootMarkerID,
			paramColliderClassesMask,
			paramValidBox,
			paramValidBoxMin,
			paramValidBoxMax,
			paramCount,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiCollisionShapeType[4] ;

	protected:
		S3DItemInstancingSerializer	m_instancing ;
		bool		m_flagReflectInstance ;

		S3DMatrix	m_matBaseRotation ;
		S3DVector	m_vBaseZoom ;

		bool		m_flagUpdateCollision ;
		bool		m_flagUpdateHitPoints ;
		bool		m_flagValidBox ;
		S3DVector	m_vValidBoxMin ;
		S3DVector	m_vValidBoxMax ;

		S3DModelData::MarkerInfo::Type	m_typeColliderMarker ;
		SSystem::SString				m_strFootMarkerID ;

		S3DVertexBufferInterface *		m_pRefCollider ;
		S3DCollision					m_collisionBuf ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DOPhysicsInstancingController,
					Controller, S3DOPhysicsDynamicInstancingInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DOPhysicsInstancingController, physics_instance )
		// 構築関数
		S3DOPhysicsInstancingController( void ) ;
		S3DOPhysicsInstancingController( const wchar_t * pwszClassID ) ;
		void InitPhysicsInstancingController( void ) ;
		// 消滅関数
		virtual ~S3DOPhysicsInstancingController( void ) ;

	public:
		// 回転
		const S3DMatrix& GetBaseRotation( void ) const ;
		void SetBaseRotation( const S3DMatrix& matRot ) ;
		// 拡大
		const S3DVector& GetBaseZoom( void ) const ;
		void SetBaseZoom( const S3DVector& vZoom ) ;

	public:
		// ベース行列更新
		void UpdateBaseMatrix( void ) ;
		// コリジョン更新
		void UpdateCollision( void ) ;
		// 当たり判定点更新
		void UpdateHitPoints( void ) ;
	protected:
		void MakeHitPointsFromModel( S3DModelBuffer * pModel ) ;
		void MakeHitPointsFromVBO( S3DVertexBufferInterface * pVBO ) ;
		void MakeHitPointsFromVertex( const S3DVector4 * pvVertex, size_t nCount ) ;
		void MakeHitPointsFromSphere
			( const S3DVector& vCenter, float32_t fpRadius ) ;
		void MakeHitPointsFromCube
			( const S3DVector& vCenter, const S3DVector& vCubeSize ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
					double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// 当たり判定追加
		//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
		virtual void RenderCollision
			( const S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, S3DCollision& render ) ;

	public:
		// インスタンス削除（ReflectInstance の時、通知あり）
		virtual void RemoveInstanceAt( size_t i ) ;

	public:	// S3DOPhysicsDynamicInstancingInterface
		// 全インスタンス削除
		void RemoveAllInstance( void ) ;
		// インスタンス追加
		size_t AddInstance
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors, size_t nCount,
				const S3DVector * pSpeeds = nullptr,
				const S4DVector * pRotSpeeds = nullptr,
				const void * pExInitData = nullptr, size_t nExDataStride = 0 ) ;
	protected:
		size_t GetFirstDynamicInstanceIndex( void ) const ;

	public:
		// アクター生成
		virtual S3DPhysicsScene::Actor * NewActor( const void * pExInitData = nullptr ) const ;

	} ;

}

#endif
