
#if	!defined(__ANTIRRHINUM_GRAPHICS_H__)
#define	__ANTIRRHINUM_GRAPHICS_H__

namespace	AntirrhinumGL
{
	//////////////////////////////////////////////////////////////////////////
	// グラフィック処理用レイヤー Sprite
	//////////////////////////////////////////////////////////////////////////

	class	AGLGraphicsLayer	: public SakuraGL::SGLSprite
	{
	public:
		enum	LayerType
		{
			typeDefault	= -1,
			typeLayer,
			typeSubLayer,
			typeBufferedLayer,
			typeLayerSet,
			typeFrameBufferVRAM,
			typeFrameBufferRAM,
			typeFrameBufferCPU,
		} ;

		struct	LayerStyle
		{
			uint32_t			nFlags ;		// = 0
			uint32_t			nType ;			// enum LayerType
			int32_t				nBasePriority ;	// 基準優先度
			SakuraGL::SGLSize	sizeScreen ;	// スクリーン（フレームバッファ）サイズ
			SakuraGL::SGLPoint	ptDstOffset ;	// デフォルト表示左上座標
			SakuraGL::SGLPoint	ptSrcPivot ;	// デフォルトの pivot 座標

			LayerStyle( void )
				: nFlags(0), nType(typeLayer), nBasePriority(0),
					sizeScreen( 1280, 720 ),
					ptDstOffset( 0, 0 ), ptSrcPivot( 0, 0 ) { }
			LayerStyle( const LayerStyle& style )
				: nFlags(style.nFlags), nType(style.nType),
					nBasePriority(style.nBasePriority),
					sizeScreen(style.sizeScreen),
					ptDstOffset(style.ptDstOffset), ptSrcPivot(style.ptSrcPivot) { }
			SakuraGL::S2DDVector ParamOrgPos( void ) const
			{
				return	SakuraGL::S2DDVector( ptDstOffset.x, ptDstOffset.y ) ;
			}
		} ;

		struct	GraphicsParam
		{
			SakuraGL::S2DVector	vPosition ;
			SakuraGL::S2DVector	vZoom ;
			float32_t			degRotate ;
			uint32_t			nTransparency ;
			uint32_t			nFilterParam ;
			uint32_t			nFilterParam2 ;

			GraphicsParam( void )
				: vPosition( 0.0f, 0.0f ), vZoom( 1.0f, 1.0f ),
						degRotate( 0.0f ), nTransparency( 0 ),
						nFilterParam( 0 ), nFilterParam2( 0 ) { }
			GraphicsParam( const GraphicsParam& param )
				: vPosition( param.vPosition ),
					vZoom( param.vZoom ),
					degRotate( param.degRotate ),
					nTransparency( param.nTransparency ),
					nFilterParam( param.nFilterParam ),
					nFilterParam2( param.nFilterParam2 ) { }
			const GraphicsParam& operator = ( const GraphicsParam& param )
			{
				vPosition = param.vPosition ;
				vZoom = param.vZoom ;
				degRotate = param.degRotate ;
				nTransparency = param.nTransparency ;
				nFilterParam = param.nFilterParam ;
				nFilterParam2 = param.nFilterParam2 ;
				return	*this ;
			}
		} ;

		struct	MoveAnimeParam	: public GraphicsParam
		{
			uint32_t	msecDuration ;
			float32_t	fpSpeedStart ;
			float32_t	fpSpeedEnd ;

			MoveAnimeParam( void )
				: msecDuration( 1000 ),
					fpSpeedStart( 1.0 ), fpSpeedEnd( 1.0 ) { }
			MoveAnimeParam( const MoveAnimeParam& param )
				: GraphicsParam( param ),
					msecDuration( param.msecDuration ),
					fpSpeedStart( param.fpSpeedStart ),
					fpSpeedEnd( param.fpSpeedEnd ) { }
			MoveAnimeParam( const GraphicsParam& param )
				: GraphicsParam( param ),
					msecDuration( 1000 ),
					fpSpeedStart( 1.0 ), fpSpeedEnd( 1.0 ) { }
			const MoveAnimeParam& operator = ( const MoveAnimeParam& param )
			{
				GraphicsParam::operator = ( param ) ;
				msecDuration = param.msecDuration ;
				fpSpeedStart = param.fpSpeedStart ;
				fpSpeedEnd = param.fpSpeedEnd ;
				return	*this ;
			}
			const MoveAnimeParam& operator = ( const GraphicsParam& param )
			{
				GraphicsParam::operator = ( param ) ;
				return	*this ;
			}
		} ;

		class	LayerMoveAction	: public SGLSprite::Action
		{
		public:
			bool							m_flagOffset ;
			SSystem::SArray<MoveAnimeParam>	m_aMoveAnime ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( LayerMoveAction, Action )
			// 構築関数
			LayerMoveAction( void ) ;
			LayerMoveAction( const LayerMoveAction& act ) ;
			// 設定
			void SetAnimation
				( const SSystem::SArray<MoveAnimeParam>& aAnime, bool flagOffset ) ;

		public:
			// パラメータ反映
			virtual void EffectParameter
				( SGLSprite::Parameter& param, double t ) ;
			// 複製
			virtual SGLObject * DuplicateObject( void ) ;
			// シリアライズ
			virtual SakuraGL::SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SakuraGL::SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

	protected:
		LayerStyle						m_style ;
		bool							m_flagNextParam ;
		GraphicsParam					m_gpNext ;
		SSystem::SArray<MoveAnimeParam>	m_aMoveAnime ;

		static const wchar_t *	s_pwszEntitySpriteID ;
		static const wchar_t *	s_pwszFadeoutSpriteID ;
		static const int32_t	s_nEntityPriority ;
		static const int32_t	s_nFadeoutPriority ;

		SSystem::SSmartReference<AGLGraphicsLayer>	m_refEntity ;
		SSystem::SSmartReference<AGLGraphicsLayer>	m_refFadeout ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLGraphicsLayer, SakuraGL::SGLSprite )
		// 構築関数
		AGLGraphicsLayer( void ) ;
		AGLGraphicsLayer( const AGLGraphicsLayer& layer ) ;
		// スタイル
		const LayerStyle& GetLayerStyle( void ) const ;
		void SetLayerStyle( const LayerStyle& style ) ;
		static void SetLayerStyle( SakuraGL::SGLSprite& sprite, const LayerStyle& style ) ;
		// フレームバッファ設定反映
		void ApplyFrameBufferStyle( SakuraGL::S3DRenderDevice * pDevice ) ;
		static void ApplyFrameBufferStyle
			( SakuraGL::SGLSprite& sprite,
				const LayerStyle& style,
				SakuraGL::S3DRenderDevice * pDevice ) ;

	public:
		// 移動アニメの最後のパラメータか、現在のパラメータ取得
		GraphicsParam GetLastMovedParam( void ) const ;
		// 現在の Sprite 表示状態を取得
		GraphicsParam GetCurrentParam( void ) const ;
		static GraphicsParam GetCurrentParam( const SakuraGL::SGLSprite& sprite ) ;
		// 現在の Sprite 表示状態を設定
		void SetCurrentParam( const GraphicsParam& param ) ;
		static void SetCurrentParam
			( SakuraGL::SGLSprite& sprite, const GraphicsParam& param ) ;
		// 移動アニメ設定開始
		void PrepareMoveAnimation( bool flagOffset ) ;
		// 移動アニメ追加
		void AddMoveAnimeParam( const MoveAnimeParam& param ) ;
		// 移動アニメ開始
		void StartMoveAnimation
			( ActionType type = actionOnce, bool flagOffset = false ) ;

	public:
		// 画像読み込み
		SakuraGL::SGLError LoadLayerImage
			( const wchar_t * pwszFilePath, uint32_t msecFadeTime = 500 ) ;

	public:
		// レイヤーアイテム取得
		SakuraGL::SGLSprite * GetSpriteAs( const wchar_t * pwszLayerID ) ;
		AGLGraphicsLayer * GetLayerAs( const wchar_t * pwszLayerID ) ;
		// レイヤー移動／フェード・アニメーション中か？
		bool IsPendingLayerAnimation
			( bool flagAllLayers = true, bool flagAllAction = false ) const ;
		// レイヤー移動／フェード・アニメーション即時完了
		void FlushLayerAnimation
			( bool flagAllLayers = true, bool flagAllAction = false ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	public:
		// 複製（可能なら）
		virtual SakuraGL::SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SakuraGL::SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SakuraGL::SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// グラフィック処理
	//////////////////////////////////////////////////////////////////////////

	class	AGLGraphicsProcessor	: public AGLEpicFuncProcessor
	{
	protected:
		SakuraGL::S3DRenderDevice *		m_pDevice ;
		SakuraGL::SGLSprite *			m_pRootScreen ;
		int32_t							m_nBasePriority ;

		AGLGraphicsLayer::LayerStyle	m_styleDef ;
		SSystem::SStrSortArray
			<AGLGraphicsLayer::LayerStyle>
										m_ssoaStyles ;

		bool							m_flagCfgInitScreen ;
		size_t							m_iCfgInitScreen ;
		AGLGraphicsLayer::LayerStyle	m_styleInitScreen ;

		SSystem::SObjectArray
				<AGLGraphicsLayer>		m_aScreens ;
		AGLGraphicsLayer *				m_pScreen ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLGraphicsProcessor, AGLEpicFuncProcessor )
		AGL_DECLARE_EPIC_PROCESSOR( AGLGraphicsProcessor )
		// 構築関数
		AGLGraphicsProcessor( void ) ;
		// 消滅関数
		virtual ~AGLGraphicsProcessor( void ) ;
		// 表示設定
		void SetDisplaySettings
			( SakuraGL::S3DRenderDevice * pDevice,
				SakuraGL::SGLSprite * pRootScreen, int32_t nBasePriority ) ;

	public:	// AGLObject
		// シリアライズ
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

	public:	// AGLEpicProcessor
		// 設定
		virtual void LoadConfiguration
			( const SSystem::SXMLDocument& xmlConfig ) ;
		void ParseLayerStyle
			( AGLGraphicsLayer::LayerStyle& style,
				const SSystem::SXMLDocument& xmlStyle ) ;
		// タイマ処理 (実行フレーム前処理)
		virtual void OnKernelTimer( void ) ;
		// ゲーム開始時処理
		virtual void InitializeGame( void ) ;
		// ゲーム終了前フェードアウト処理
		virtual void FadeoutGame( uint32_t msecFadeout ) ;
		// ゲーム終了時処理
		virtual void ReleaseGame( void ) ;

	public:
		// レイヤースタイル取得
		const AGLGraphicsLayer::LayerStyle *
					GetLayerStyleAs( const wchar_t * pwszID ) const ;
		AGLGraphicsLayer::LayerStyle MakeLayerStyleAs
			( const wchar_t * pwszID,
				AGLGraphicsLayer::LayerType type = AGLGraphicsLayer::typeDefault ) const ;

	public:
		// スクリーン作成
		void InitScreen
			( size_t iScreen, int32_t nPriority = 0, bool flagVisible = true,
				const wchar_t * pwszStyleID = NULL,
				AGLGraphicsLayer::LayerType type = AGLGraphicsLayer::typeFrameBufferRAM ) ;
		void CloneScreen( size_t iScreen, size_t iSrcScreen, int32_t nPriority ) ;
		void ScreenPriority( size_t iScreen, int32_t nPriority ) ;
		void ScreenVisible( size_t iScreen, bool flagVisible ) ;
		void SetScreenAt( size_t iScreen, AGLGraphicsLayer * pScreen ) ;
		void SelectScreen( size_t iScreen ) ;
		// スクリーン解放
		void ReleaseScreen( size_t iScreen ) ;
		// スクリーン・スワップ
		void SwapScreen( size_t iScreen0, size_t iScreen1 ) ;
		// レイヤー取得
		SakuraGL::SGLSprite * GetSpriteAs( const wchar_t * pwszLayerID ) ;
		AGLGraphicsLayer * GetLayerAs( const wchar_t * pwszLayerID ) ;
		SakuraGL::SGLSprite * GetTargetSpriteAs
			( AGLThread& thread, const SSystem::SXMLDocument & xmlCode ) ;

	public:
		// レイヤー作成
		virtual AGLGraphicsLayer * CreateLayer
			( const wchar_t * pwszStyleID = NULL,
				AGLGraphicsLayer::LayerType type = AGLGraphicsLayer::typeLayer ) ;
		// レイヤーアイテム作成
		virtual SakuraGL::SGLSprite * NewLayerItem( const SSystem::SString& strItemType ) ;
		virtual void OnInitLayerItem
			( SakuraGL::SGLSprite * pNewSprite,
				const SSystem::SString& strItemType,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;
		virtual SakuraGL::SGLSprite * CreateLayerItem
			( const SSystem::SString& strItemType,
				const AGLGraphicsLayer::LayerStyle& style,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;
		// レイヤー共通パラメータ設定
		virtual void ParseSpriteParameters
			( SakuraGL::SGLSprite& sprite,
				const AGLGraphicsLayer::LayerStyle& style,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;
		virtual void ParseLayerParameters
			( AGLGraphicsLayer::GraphicsParam& param,
				const AGLGraphicsLayer::LayerStyle& style,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;
		virtual AGLGraphicsLayer::LayerType ParseLayerType
			( const SSystem::SXMLDocument & xmlParams,
					AGLGraphicsLayer::LayerType typeDefault ) ;
		// フィルター作成
		virtual SakuraGL::SGLSpriteFilter * CreateSpriteFilter
			( const SSystem::SString& strFilterType,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;
		// レイヤー固有操作
		virtual CodeProcessResult OperateLayer
			( SakuraGL::SGLSprite& sprite,
				const SSystem::SString& strOperate,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams ) ;

	public:
		template <class T> inline T ParseNumParameter
			( const wchar_t * pwszAttrName, T nDefault,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
		{
			const SSystem::SString *	pstrAttr = xmlParams.GetAttributeAs( pwszAttrName );
			if ( pstrAttr != NULL )
			{
				return	(T) EvaluateNumberExpression( thread, *pstrAttr, nDefault ) ;
			}
			return	nDefault ;
		}
		template <class T> inline T ParseUIntParameter
			( const wchar_t * pwszAttrName, T nDefault,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
		{
			const SSystem::SString *	pstrAttr = xmlParams.GetAttributeAs( pwszAttrName );
			if ( pstrAttr != NULL )
			{
				return	(T) esl_max( (int) EvaluateIntExpression( thread, *pstrAttr, nDefault ), 0 ) ;
			}
			return	nDefault ;
		}
		template <class T> inline T ParseIntParameter
			( const wchar_t * pwszAttrName, T nDefault,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
		{
			const SSystem::SString *	pstrAttr = xmlParams.GetAttributeAs( pwszAttrName );
			if ( pstrAttr != NULL )
			{
				return	(T) EvaluateIntExpression( thread, *pstrAttr, nDefault ) ;
			}
			return	nDefault ;
		}
		inline bool ParseBoolParameter
			( const wchar_t * pwszAttrName, bool bDefault,
				AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
		{
			const SSystem::SString *	pstrAttr = xmlParams.GetAttributeAs( pwszAttrName );
			if ( pstrAttr != NULL )
			{
				return	EvaluateBoolExpression( thread, *pstrAttr, bDefault ) ;
			}
			return	bDefault ;
		}

	public:
		// コマンド実装
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_init)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_clone)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_delete)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_swap)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_select)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_create)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_image)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_delete)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_exist)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_parameter)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_pre_move)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_add_move)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_start_move)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_wait_move)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_flush_move)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_add_filter)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_clear_filter)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_operate)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_freeze)
		DECL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_defrost)

	} ;

}

#endif

