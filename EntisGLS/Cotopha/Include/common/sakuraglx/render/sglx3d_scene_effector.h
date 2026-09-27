
#if	!defined(__SAKURAGLX3D_SCENE_EFFECTOR_H__)
#define	__SAKURAGLX3D_SCENE_EFFECTOR_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 環境マッピングターゲット
	//////////////////////////////////////////////////////////////////////////

	class	S3DCubeEnvironmentMapSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramImageWidth		= ItemBasicSerializer::paramItemTotalCount,
			paramUpdateFlag,
			paramTargetField,
			paramTargetStaticItem1,
			paramTargetStaticItem2,
			paramTargetDynamicItem1,
			paramTargetDynamicItem2,
			paramTargetDynamicItem3,
			paramTargetEffectItem,
			paramTargetLayeredItem,
			paramEnvMapTotalCount,
			paramEnvMapCount	= paramEnvMapTotalCount - paramImageWidth,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCubeEnvironmentMapSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DCubeEnvironmentMapSerializer, envmap_target )
		// 構築関数
		S3DCubeEnvironmentMapSerializer( void ) ;
		// 消滅関数
		virtual ~S3DCubeEnvironmentMapSerializer( void ) ;

	protected:
		uint32_t			m_maskTargetClasses ;
		SGLSize				m_sizeImage ;
		bool				m_flagEnabled ;
		bool				m_flagUpdate ;
		SGLImage			m_imgCubemap ;
		SGLImage			m_imgZBuffer ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramEnvMapCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// 画像サイズ
		const SGLSize& GetImageSize( void ) const
		{
			return	m_sizeImage ;
		}
		void SetImageSize( const SGLSize& size ) ;
		// 描画ターゲット
		uint32_t GetTargetClasses( void ) const
		{
			return	m_maskTargetClasses ;
		}
		void SetTargetClasses( uint32_t maskClasses ) ;
		// 有効・無効化
		bool IsEnvMapEnabled( void ) const
		{
			return	m_flagEnabled ;
		}
		void EnableEnvMap( bool flagEnable ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// レンダリング後始末
		virtual void OnShoutdownSceneSettings
			( S3DScene& scene,
				SGLSecondaryViewProducer * psvp = NULL ) ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// エフェクター共通
	//////////////////////////////////////////////////////////////////////////

	class	S3DCommonEffectorItemSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramEffectorClass		= ItemBasicSerializer::paramItemTotalCount,
			paramEffectorPriority,
			paramEffectorTotalCount,
			paramEffectorCount		= paramEffectorTotalCount - paramEffectorClass,
		} ;
		enum	EffectorClass
		{
			effectorGlobalAbove,		// classLayeredItem1 前の実行
			effectorGlobalBelow,		// classEffect2 後の実行
			effectorLocalSpace,			// LayeredSpace でのローカル効果
			effectorClassCount,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCommonEffectorItemSerializer, ItemBasicSerializer )
		// 構築関数
		S3DCommonEffectorItemSerializer
			( const wchar_t * pwszClassID,
				const S3DSceneComposer::ParamSetClass * pClass ) ;
		// 消滅関数
		virtual ~S3DCommonEffectorItemSerializer( void ) ;

	protected:
		S3DScene::Effector *	m_pEffector ;
		EffectorClass			m_clsEffector ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramEffectorCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiEffectorClass[effectorClassCount+1] ;

	public:
		// エフェクタ関連付け
		void AttachSceneEffector( S3DScene::Effector * pEffector ) ;
		// エフェクタ取得
		S3DScene::Effector * GetSceneEffector( void ) const
		{
			return	m_pEffector ;
		}

	public:
		// 効果クラス
		EffectorClass GetEffectorClass( void ) const ;
		void SetEffectorClass( EffectorClass clsEffector ) ;
		// 効果優先度
		int GetEffectorPriority( void ) const ;
		void SetEffectorPriority( int nPriority ) ;

	public:
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:	// Parameter
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::ItemEventListener
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 遅延シェーダー光源
	//////////////////////////////////////////////////////////////////////////

	class	S3DDelayLightSerializer
				: public S3DCommonEffectorItemSerializer,
						public S3DScene::Effector
	{
	public:
		enum	ParameterIndex
		{
			paramLightType		= S3DCommonEffectorItemSerializer::paramEffectorTotalCount,
			paramWriteEmission,
			paramCutOffDistance,
			paramFadeOutDistance,
			paramLightDiffusion,
			paramLightSpecular,
			paramLightAirScattering,
			paramLightAirUnitLength,
			paramLightColor,
			paramLightBrightness,
			paramLightDirection,
			paramLightAttenuationPower,
			paramLightAngle,
			paramLightGradation,
			paramLightTotalCount,
			paramLightCount		= paramLightTotalCount - paramLightType,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DDelayLightSerializer, S3DCommonEffectorItemSerializer, Effector )
		S3D_DECLARE_COMPOSER_ITEM( S3DDelayLightSerializer, delay_light )
		// 構築関数
		S3DDelayLightSerializer( void ) ;
		// 消滅関数
		virtual ~S3DDelayLightSerializer( void ) ;

	protected:
		double				m_fpDiffusion ;
		double				m_fpSpecular ;
		double				m_fpAirScattering ;
		double				u_fpAirUnitLength ;

		S3DLightEntry		m_light ;
		double				m_degLightAngle ;
		double				m_degLightGradation ;

		bool				m_flagWriteEmission ;
		double				m_fpCutOffDistance ;
		double				m_fpFadeOutDistance ;
		double				m_fpDistanceFade ;

		S3DRenderDevice *			m_pDevice ;
		S3DRenderContextInterface *	m_pRender ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramLightCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiLightTypes[3] ;

	public:
		// 拡散反射適用度
		double GetDiffusion( void ) const ;
		void SetDiffusion( double fpDiffusion ) ;
		// 鏡面反射適用度
		double GetSpecular( void ) const ;
		void SetSpecular( double fpDiffusion ) ;
		// 大気散乱効果度
		double GetAirScattering( void ) const ;
		double GetAirScatteringUnit( void ) const ;
		void SetAirScattering
				( double fpAirScattering, double fpAirUnitLength ) ;
		// 光源タイプ
		uint32_t GetLightType( void ) const ;
		void SetLightType( uint32_t type ) ;
		// 光源色
		SGLPalette GetLightColor( void ) const ;
		void SetLightColor( const SGLPalette& rgbColor ) ;
		// 輝度
		double GetLightBrightness( void ) const ;
		void SetLightBrightness( double fpBrightness ) ;
		// 点光源減衰力 x : (1/r^x)
		double GetAttenuationPower( void ) const ;
		void SetAttenuationPower( double fpAttenuation ) ;
		// 光源位置
		const S3DDVector& GetLightPosition( void ) const ;
		void SetLightPosition( const S3DDVector& vPos ) ;
		// 光源向き
		const S3DVector& GetLightDirection( void ) const ;
		void SetLightDirection( const S3DVector& vDir ) ;
		// スポットライト範囲角 [deg]
		double GetLightAngle( void ) const ;
		void SetLightAngle( double degAngle ) ;
		// スポットライトぼかし角 [deg]
		double GetLightGradation( void ) const ;
		void SetLightGradation( double degGradation ) ;
		// 発光出力
		bool IsEnabledWriteEmission( void ) const ;
		void EnableWriteEmission( bool fEmission ) ;
		// 影響消失距離
		double GetCutOffDistance( void ) const ;
		double GetFadeOutDistance( void ) const ;
		void SetCutOffDistance( double fpCutOff ) ;
		void SetFadeOutDistance( double fpFadeOut ) ;

	public:
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:	// Parameter
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
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	public:	// S3DScene::ItemEventListener
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:	// S3DScene::DrawLayerEffector
		// 要求カラーバッファ
		virtual uint32_t GetRequiredColorBufferMask( void ) const ;
		// 描画処理
		virtual void DrawEffect
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				SGLImageObject*const* ppImage,
				size_t nMultiImages, SGLImageObject * pDepth ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 大域照明効果アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DSSGlobalIlluminationSerializer
				: public S3DCommonEffectorItemSerializer,
						public S3DScene::Effector
	{
	public:
		enum	ParameterIndex
		{
			paramWriteEmission	= S3DCommonEffectorItemSerializer::paramEffectorTotalCount,
			paramAOReachDistance,
			paramAOSamplingCount,
			paramGISamplingCount,
			paramSamplingScale,
			paramLuminousness,
			paramAOBlendRatio,
			paramAOShadeColor,
			paramGIBlendRatio,
			paramMakePanorama,
			paramPanoramaImageSize,
			paramRefRenderBackscape,
			paramRefRenderScape,
			paramRefRenderField,
			paramRefRenderStaticItem1,
			paramRefRenderStaticItem2,
			paramRefRenderDynamicItem1,
			paramRefRenderDynamicItem2,
			paramRefRenderDynamicItem3,
			paramRefRenderEffectItem,
			paramRefRenderLayerdSpace,
			paramRefRenderLayerdItem1,
			paramRefRenderLayerdItem2,
			paramGITotalCount,
			paramGICount		= paramGITotalCount - paramWriteEmission,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DSSGlobalIlluminationSerializer, S3DCommonEffectorItemSerializer, Effector )
		S3D_DECLARE_COMPOSER_ITEM( S3DSSGlobalIlluminationSerializer, ssgi_effect )
		// 構築関数
		S3DSSGlobalIlluminationSerializer( void ) ;
		// 消滅関数
		virtual ~S3DSSGlobalIlluminationSerializer( void ) ;

	protected:
		double				m_fpAOReachDistance ;
		size_t				m_nAOSamplingCount ;
		size_t				m_nGISamplingCount ;
		size_t				m_nSamplingScale ;
		double				m_fpLuminousness ;
		double				m_fpAOBlendRatio ;
		double				m_fpGIBlendRatio ;
		SGLPalette			m_rgbAOShadeColor ;
		bool				m_flagWriteEmission ;
		bool				m_flag3WayPanorama ;
		uint32_t			m_classes3WayPanorama ;
		S4DMatrix			m_mat3WayPanoramaPers ;
		int					m_n3WayPanoramaSize ;		// N
		SGLImage			m_image3WayPanorama[2] ;	// 3NxN
		SGLImage			m_depth3WayPanorama ;
		S3DScene::Camera	m_camraPanorama ;

		S3DRenderDevice *			m_pDevice ;
		S3DRenderContextInterface *	m_pRender ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramGICount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// AO 到達距離
		double GetAOReachDistance( void ) const ;
		void SetAOReachDistance( double fpReach ) ;
		// AO サンプリング数
		size_t GetAOSamplingCount( void ) const ;
		void SetAOSamplingCount( size_t nCount ) ;
		// GI サンプリング数
		size_t GetGISamplingCount( void ) const ;
		void SetGISamplingCount( size_t nCount ) ;
		// サンプリングスケール
		size_t GetSamplingScale( void ) const ;
		void SetSamplingScale( size_t nScale ) ;
		// 拡散反射輝度
		double GetDiffusionLuminousness( void ) const ;
		void SetDiffusionLuminousness( double fpLuminousness ) ;
		// 適用度
		double GetAOBlendRatio( void ) const ;
		double GetGIBlendRatio( void ) const ;
		void SetAOBlendRatio( double fpBlend ) ;
		void SetGIBlendRatio( double fpBlend ) ;
		// AO 影色
		const SGLPalette& GetAOShadeColor( void ) const ;
		void SetAOShadeColor( const SGLPalette& rgbColor ) ;
		// 発光出力
		bool IsEnabledWriteEmission( void ) const ;
		void EnableWriteEmission( bool fEmission ) ;
		// 3面パノラマ画像有効化
		bool IsEnabled3WayPanorama( void ) const ;
		void Enable3WayPanorama( bool fEnable ) ;
		// 3面パノラマ画像サイズ設定
		int Get3WayPanoramaSize( void ) const ;
		void Set3WayPanoramaSize( int nSize ) ;
		// 3面パノラマ描画対象
		uint32_t Get3WayPanoramaClassesMask( void ) const ;
		void Set3WayPanoramaClassesMask( uint32_t maskClasses ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::ItemEventListener
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	protected:
		// ３面パノラマレンダリング
		void Render3WayPanoramaBuffer( S3DScene& scene ) ;

	public:	// S3DScene::DrawLayerEffector
		// 要求カラーバッファ
		virtual uint32_t GetRequiredColorBufferMask( void ) const ;
		// 描画処理
		virtual void DrawEffect
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				SGLImageObject*const* ppImage,
				size_t nMultiImages, SGLImageObject * pDepth ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// グロー効果アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DGlowEffectSerializer
				: public S3DCommonEffectorItemSerializer,
						public S3DScene::EmissiveGlowEffector
	{
	public:
		enum	ParameterIndex
		{
			paramGlowSource	= S3DCommonEffectorItemSerializer::paramEffectorTotalCount,
			paramGauss,
			paramSamplingScale,
			paramBrighness,
			paramAlpha,
			paramGlowTotalCount,
			paramGlowCount	= paramGlowTotalCount - paramGlowSource,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramGlowCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiGlowSource[3] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DGlowEffectSerializer, S3DCommonEffectorItemSerializer, EmissiveGlowEffector )
		S3D_DECLARE_COMPOSER_ITEM( S3DGlowEffectSerializer, glow_effector )
		// 構築関数
		S3DGlowEffectSerializer( void ) ;
		// 消滅関数
		virtual ~S3DGlowEffectSerializer( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 擬似被写界深度効果アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DDepthOfFieldEffectSerializer
					: public S3DCommonEffectorItemSerializer,
							public S3DScene::DepthOfFieldEffector
	{
	public:
		enum	ParameterIndex
		{
			paramGauss	= S3DCommonEffectorItemSerializer::paramEffectorTotalCount,
			paramNearDepth,
			paramFarDepth,
			paramDOFTotalCount,
			paramDOFCount	= paramDOFTotalCount - paramGauss,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramDOFCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DDepthOfFieldEffectSerializer, S3DCommonEffectorItemSerializer, DepthOfFieldEffector )
		S3D_DECLARE_COMPOSER_ITEM( S3DDepthOfFieldEffectSerializer, dof_effector )
		// 構築関数
		S3DDepthOfFieldEffectSerializer( void ) ;
		// 消滅関数
		virtual ~S3DDepthOfFieldEffectSerializer( void ) ;

	public:
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::ItemEventListener
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ぼかし・フラッシュ効果アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DCurtainEffectSerializer
					: public S3DCommonEffectorItemSerializer,
							public S3DScene::CurtainEffector
	{
	public:
		enum	ParameterIndex
		{
			paramGauss	= S3DCommonEffectorItemSerializer::paramEffectorTotalCount,
			paramSamplingScale,
			paramAlpha,
			paramCurtainColor,
			paramCurtainAlpha,
			paramCurtainFadeAlpha,
			paramCurtainTotalCount,
			paramCurtainCount	= paramCurtainTotalCount - paramGauss,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramCurtainCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		struct	Flash
		{
			SGLPalette	argbColor ;
			uint32_t	msecPast ;
			uint32_t	msecFadein ;
			uint32_t	msecFadeout ;
		} ;
		SSystem::SArray<Flash>		m_aFlash ;
		SGLPalette					m_argbFlash ;

	public:
		enum	FadeParamMask
		{
			fadeMaskGauss			= 0x0001,
			fadeMaskBrightness		= 0x0002,
			fadeMaskAlpha			= 0x0004,
			fadeMaskCurtain			= 0x0008,
			fadeMaskCurtainAlpha	= 0x0010,
		} ;
		struct	FadeParam
		{
			uint32_t	maskParam ;
			float32_t	gauss ;
			float32_t	brightness ;
			float32_t	alpha ;
			SGLPalette	argbCurtain ;
			uint32_t	aCurtain ;

			FadeParam( void )
				: maskParam(0), gauss(0.0f), brightness(1.0f),
						alpha(1.0f), argbCurtain(0), aCurtain(0) {}
		} ;

	protected:
		struct	FadeEntry
		{
			FadeParam	fp0 ;
			FadeParam	fp1 ;
			uint32_t	msecPast ;
			uint32_t	msecDelay ;
			uint32_t	msecDuration ;
		} ;
		SSystem::SArray<FadeEntry>	m_aFade ;

		SSystem::SCriticalSection	m_csSync ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DCurtainEffectSerializer, S3DCommonEffectorItemSerializer, CurtainEffector )
		S3D_DECLARE_COMPOSER_ITEM( S3DCurtainEffectSerializer, shading_effector )
		// 構築関数
		S3DCurtainEffectSerializer( void ) ;
		// 消滅関数
		virtual ~S3DCurtainEffectSerializer( void ) ;

	public:
		// フラッシュ追加
		void AddFlash( const SGLPalette& argb, uint32_t msecFadein, uint32_t msecFadeout ) ;
		// フラッシュ処理中か？
		bool IsFlashing( void ) const ;

	public:
		// フェード追加
		void AddFade( const FadeParam& fp, uint32_t msecDuration, uint32_t msecDelay = 0 ) ;
		// フェード処理中か？
		bool IsFading( void ) const ;
		// 全てのフェード処理をキャンセル
		void ClearAllFading( void ) ;
		// 現在のパラメータ取得
		void GetCurrentFadeParam( FadeParam& fp ) const ;
		// パラメータ設定
		void SetFadeParam( const FadeParam& fp ) ;
		// フェードパラメータ補完
		static void LerpFadeParam
			( FadeParam& fpDst,
				const FadeParam& fp0, const FadeParam& fp1, float32_t t ) ;

	public:
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
	} ;

}

#endif
