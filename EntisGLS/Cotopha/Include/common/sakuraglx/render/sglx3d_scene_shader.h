
#if	!defined(__SAKURAGLX3D_SCENE_SHADER_H__)
#define	__SAKURAGLX3D_SCENE_SHADER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// シェーダー共通処理
	//////////////////////////////////////////////////////////////////////////

	class	S3DCommonShaderController	: public S3DSceneComposer::Controller
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCommonShaderController, Controller )
		// 構築関数
		S3DCommonShaderController
			( const wchar_t * pwszClassID, int iClassTargetParam = -1 ) ;
		// 消滅関数
		virtual ~S3DCommonShaderController( void ) ;
		// ターゲットクラス・パラメータ追加
		void AddTargetClassParameterEntry( void ) ;

	protected:
		bool	m_flagExceptShadow ;
		int		m_iClassTargetParam ;
		int		m_clsTarget ;				// enum S3DScene::ItemClass, or -1

		static const SSystem::SXMLDocument::AttrInteger	m_aiTargetClass[16] ;

	public:
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// 描画前処理
		virtual void BeforeRenderModel
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
	protected:
		// シェーダー設定
		virtual void UpdateShader
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DRenderContextInterface& render ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DStandardShaderController : public S3DCommonShaderController
	{
	public:
		enum	ParameterIndex
		{
			paramShaderType,
			paramTargetClass,
		} ;
		enum	ShadingType
		{
			shadingNo,
			shadingGouraud,
			shadingPhong,
			shadingCount,
		} ;
		static const wchar_t *	m_pwszShadingType[shadingCount] ;
		static const uint64_t	m_nShadingFlag[shadingCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DStandardShaderController, S3DCommonShaderController )
		S3D_DECLARE_COMPOSER_ITEM( S3DStandardShaderController, standard_shader )
		// 構築関数
		S3DStandardShaderController( void ) ;
		// 消滅関数
		virtual ~S3DStandardShaderController( void ) ;

	protected:
		ShadingType	m_shading ;

	public:
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	protected:
		// シェーダー設定
		virtual void UpdateShader
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DRenderContextInterface& render ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ユーザー・シェーダー・シリアライザ
	//////////////////////////////////////////////////////////////////////////

	class	S3DUserShaderSerializer
	{
	public:
		enum	SpecialElementType
		{
			typeOrdinaryElement,
			typeTimerElement,
		} ;
		class	UniformElement
		{
		public:
			struct	Value	: public S4DDMatrix
			{
				Value( void ) : S4DDMatrix( 0, 0, 0, 0 )
				{
				}
				const Value& operator = ( const Value& src )
				{
					eslCopyMemory( this, &src, sizeof(Value) ) ;
					return	*this ;
				}
				int32_t& Int32( void )
				{
					return	*((int32_t*)this) ;
				}
				double& Float64( void )
				{
					return	*((double*)this) ;
				}
				S2DDVector& Vector2D( void )
				{
					return	*((S2DDVector*)this) ;
				}
				S3DDVector& Vector3D( void )
				{
					return	*((S3DDVector*)this) ;
				}
				S4DDVector& Vector4D( void )
				{
					return	*((S4DDVector*)this) ;
				}
				S3DDMatrix& Matrix3D( void )
				{
					return	*((S3DDMatrix*)this) ;
				}
				S4DDMatrix& Matrix4D( void )
				{
					return	*((S4DDMatrix*)this) ;
				}
			} ;
			SSystem::SString	m_idParam ;
			SSystem::SString	m_idDispName ;
			S3DSceneComposer::ParameterType
								m_typeProp ;
			S3DCustomShader::UniformType
								m_typeUniform ;
			SpecialElementType	m_typeSpecial ;
			size_t				m_iUniform ;
			size_t				m_iIndex ;
			Value				m_value ;
			SSystem::SString	m_idTexture ;
			SGLImageObject *	m_pTexture ;
			SSystem::SArray
				<SSystem::SXMLDocument::AttrInteger>
								m_aStrIntPairs ;
			SSystem::SObjectArray<SSystem::SString>
								m_aStrBuffers ;
		public:
			UniformElement( void )
				: m_typeProp(S3DSceneComposer::typeInvalid),
					m_typeUniform(S3DCustomShader::uniformInt),
					m_typeSpecial(typeOrdinaryElement),
					m_iUniform(0), m_iIndex(0), m_pTexture(nullptr) { }
		} ;

	protected:
		const size_t							m_iShaderParamFirst ;

		SSystem::SString						m_strUserShaderID ;
		S3DSceneComposer::UserShader *			m_pUserShader ;
		S3DRenderDevice *						m_pDevice ;
		S3DCustomShader *						m_pShader ;

		SSystem::SObjectArray<UniformElement>	m_elements ;
		SSystem::SArray<size_t>					m_uniformIndex ;

		double									m_secTimer ;

	public:
		// 構築関数
		S3DUserShaderSerializer( size_t iFirstParam ) ;
		// シェーダ―・プロパティ更新
		void UpdateShaderProperty
			( S3DSceneComposer::Controller& prop,
				S3DSceneComposer * pComposer,
				const wchar_t * pwszShaderID,
				S3DSceneComposer::UserShader * pUserShader,
				uint32_t nAttrCategoryFlag, bool flagSwitchShader ) ;
		// テクスチャ参照更新
		void UpdateTextureReference
			( S3DSceneComposer * pComposer, UniformElement * pue ) ;
		// パラメータ保存
		void SaveShaderParameters
			( S3DSceneComposer::ParameterProperty& prop,
				SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;
		// パラメータ復帰
		void ResotreShaderParameters
			( S3DSceneComposer::ParameterProperty& prop,
				const SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;
		// シェーダー設定
		void AttachShaderParamTo
			( S3DRenderContextInterface& render,
				S3DSceneComposer * pComposer ) ;

	public:
		// タイマー処理
		void OnTimer( double secPast ) ;
		// パラメータ値取得
		S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
		S3DDVector GetVectorParameter( size_t iParam ) const ;
		double GetScalarParameter( size_t iParam ) const ;
		int32_t GetIntegerParameter( size_t iParam ) const ;
		const wchar_t * GetCommandParameter( size_t iParam ) const ;
		size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t iParam ) const ;
		// パラメータ値設定
		void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
		void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		void SetScalarParameter( size_t iParam, double s ) ;
		void SetIntegerParameter( size_t iParam, int32_t n ) ;
		void SetCommandParameter
			( S3DSceneComposer * pComposer, size_t iParam, const wchar_t * pwszCmd ) ;
		size_t SetBinaryParameter
			( size_t iParam, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		bool EnumerateStringSet
			( S3DSceneComposer * pComposer,
				size_t iParam, SSystem::SStringArray& aStrSet ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// ユーザー・シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DUserShaderController
			: public S3DCommonShaderController, public S3DUserShaderSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramShaderID,
			paramTargetClass,
			paramFirstUniform,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DUserShaderController, S3DCommonShaderController )
		S3D_DECLARE_COMPOSER_ITEM( S3DUserShaderController, user_shader )
		// 構築関数
		S3DUserShaderController( void ) ;
		// 消滅関数
		virtual ~S3DUserShaderController( void ) ;

	public:
		// シェーダ―更新
		void UpdateShaderReference( bool flagSwitchShader ) ;
		// パラメータ保存
		void SaveShaderParameters
			( SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;
		// パラメータ復帰
		void ResotreShaderParameters
			( const SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;

	protected:
		SSystem::SString	m_strPropShaderID ;
		int					m_clsTarget ;	// enum S3DScene::ItemClass or -1

	public:
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t iParam, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;

	protected:
		// シェーダー設定
		virtual void UpdateShader
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DRenderContextInterface& render ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// ｚバッファ書き込み制御
	//////////////////////////////////////////////////////////////////////////

	class	S3DShadowDepthFuncController
				: public S3DSceneComposer::Controller,
					public S3DItemInstancingSerializer::MultiRenderer
	{
	public:
		enum	ParameterIndex
		{
			paramMainDepthFunc,
			paramShadowDepthFunc,
			paramEnvironmentDepthFunc,
			paramUserMultiPassDepthFunc,
			paramTransparentDepthFunc,
			paramCount,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiDepthFunc[6] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DShadowDepthFuncController, Controller, MultiRenderer )
		S3D_DECLARE_COMPOSER_ITEM( S3DShadowDepthFuncController, depth_func )
		// 構築関数
		S3DShadowDepthFuncController( void ) ;
		// 消滅関数
		virtual ~S3DShadowDepthFuncController( void ) ;

	protected:
		S3DRenderContext::DepthMaskOperation	m_depthFunc[paramCount] ;
		SSystem::SArray<S4DMatrix>	m_aTempMatrix ;
		SSystem::SArray<S3DColor>	m_aTempColor ;

	public:
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// 描画前処理
		virtual void BeforeRenderModel
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	public:	// MultiRenderer
		// 描画処理
		virtual S3DItemInstancingSerializer::RenderResult
			RenderMultiInstance
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					const S3DItemInstancingSerializer& instancing,
					S3DItemInstancingSerializer::RenderingType type,
					S3DItemInstancingSerializer::RenderInfo& info ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// 水面シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DSimpleWaterShaderController : public S3DCommonShaderController
	{
	public:
		enum	ParameterIndex
		{
			paramTargetClass,
			paramSimpleParam,
			paramRandomSeed,
			paramMasterAmplitude,
			paramMasterScale,
			paramMasterTimeScale,
			paramCascadeFarZ,
			paramCascadePhase1,
			paramCascadeAmp1,
			paramRotation,
			paramTimeMethod,
			paramTime,
			paramAmplitude0,
			paramFrequency0,
			paramPhaseSpeed0,
			paramDirection0,
			paramFirstWave		= paramAmplitude0,
			paramWaveParamCount	= paramDirection0 - paramAmplitude0 + 1,
			paramWaveCount		= 8,
		} ;
		enum	TimeMethod
		{
			timeByTimer,
			timeByFrame,
			timeByParameter,
			timeMethodCount,
		} ;
		struct	WaveParameter
		{
			float32_t	fpAmplitude ;	// 振幅
			float32_t	fpFrequency ;	// 周波数 [Hz/2π]
			float32_t	radSpeed ;		// 位相速度 [rad/sec]
			S2DVector	vDirection ;	// 方向
		} ;
		static const wchar_t *	m_pwszTimeMethod[timeMethodCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DSimpleWaterShaderController, S3DCommonShaderController )
		S3D_DECLARE_COMPOSER_ITEM( S3DSimpleWaterShaderController, wave8_shader )
		// 構築関数
		S3DSimpleWaterShaderController( void ) ;
		// 消滅関数
		virtual ~S3DSimpleWaterShaderController( void ) ;

	protected:
		WaveParameter		m_wave[paramWaveCount] ;
		S3DRenderDevice *	m_pDevice ;
		S3DCustomShader *	m_pShader ;
		bool				m_flagSimpleParam ;
		uint32_t			m_nRandomSeed ;
		TimeMethod			m_timeMethod ;
		double				m_secTime ;
		S3DDMatrix			m_matRotation ;
		double				m_fpMasterAmplitube ;
		double				m_fpMasterScale ;
		double				m_fpMasterTimeScale ;
		double				m_fpCascadeFarZ ;
		double				m_fpCascadePhase1 ;
		double				m_fpCascadeAmplitude1 ;

		static const wchar_t *	m_pwszParamIDs[paramWaveParamCount * paramWaveCount] ;
		static const wchar_t *	m_pwszParamNames[paramWaveParamCount * paramWaveCount] ;

	public:
		// 簡易パラメータ設定
		void SetSimpleParameter( uint32_t nGenParam ) ;
		static void WaveSimpleParameter
			( WaveParameter* pwp, size_t nCount, uint32_t nGenParam ) ;
		// 詳細パラメータ設定
		void AddWaveDetailParameters( void ) ;

	public:
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
					double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	protected:
		// シェーダー設定
		virtual void UpdateShader
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DRenderContextInterface& render ) ;

	} ;


}

#endif
