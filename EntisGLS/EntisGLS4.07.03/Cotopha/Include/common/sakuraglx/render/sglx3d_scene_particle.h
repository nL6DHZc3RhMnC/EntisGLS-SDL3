
#if	!defined(__SAKURAGLX3D_SCENE_PARTICLE_H__)
#define	__SAKURAGLX3D_SCENE_PARTICLE_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// パーティクル発生源
	//////////////////////////////////////////////////////////////////////////

	class	S3DParticleSerializer	: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		struct	ParticleIndex
		{
			size_t	nIndex ;
			size_t	nBlurCount ;
			size_t	nIdentity ;
		} ;
		class	RenderTarget	: public SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RenderTarget, SObject )
			// アニメーション長取得
			virtual bool GetTargetAnimationLength( double& secLength ) const = 0 ;
			// 全フレーム数取得
			virtual size_t GetTargetAnimationFrames( void ) const = 0 ;
			// ターゲット空間（逆変換用）
			virtual void GetTargetSpaceTransformation
					( S3DDMatrix& matITarget, S3DDVector& vITarget ) = 0 ;
			// パーティクル追加（classPreRender で呼び出す）
			virtual void AddParticles
				( size_t nCount,
					const S3DVector4 * pvPoints,
					const size_t * pFrames = NULL,
					const S3DColor * pColors = NULL,
					const float32_t * pZooms = NULL,
					const S4DVector * pFaceDirs = NULL,
					const float32_t * pxAspect = NULL ) = 0 ;
			// AddIndexedParticles を使うか？
			virtual bool IsUsingIndexedParticles( void ) = 0 ;
			// パーティクル追加（高機能）（classPreRender で呼び出す）
			virtual void AddIndexedParticles
				( size_t nCount,
					const S3DVector4 * pvPoints,
					const ParticleIndex * pIndexes,
					const size_t * pFrames = NULL,
					const S3DColor * pColors = NULL,
					const S3DMatrix * pFaceDirs = NULL ) = 0 ;
		} ;
	public:
		enum	ParameterIndex
		{
			paramRenderItem			= ItemBasicSerializer::paramItemTotalCount,
			paramUseDuration,
			paramDuration,
			paramFadein,
			paramFadeout,
			paramParticleZoom,
			paramParticleZoomIndefinition,
			paramParticleZoomEnd,
			paramAnimationSpeed,
			paramRotationType,
			paramRotationSpeed,
			paramUseFaceDir,
			paramFaceDir,
			paramLocalSpace,
			paramBlurFrame,
			paramBlurSupplement,
			paramMoveAspect,
			paramBaseAspectSpeed,
			paramEmissionDirection,
			paramEmissionBaseSpeed,
			paramEmissionMinAngle,
			paramEmissionAngle,
			paramEmissionCount,
			paramEmissionSpeed,
			paramEmissionSpeedIndefinition,
			paramEmissionScale,
			paramEmissionShape,
			paramAccelerationDirection,
			paramAccelerationVelocity,
			paramStreamDirection,
			paramStreamVelocity,
			paramAttenuation,
			paramWithCollision,
			paramWithExtinction,
			paramReaction,
			paramWithAbsorption,
			paramAbsorbAccel,
			paramAbsorbExAccel,
			paramAbsorbRadius,
			paramParticleTotalCount,
			paramParticleCount		= paramParticleTotalCount - paramRenderItem,
		} ;
		enum	EmissionShape
		{
			shapeSphere,
			shapeDisc,
			shapeCount,
		} ;
		struct	EmissionParam
		{
			S3DVector		vDirection ;		// 発生方向
			S3DVector		vBaseSpeed ;		// 速度
			float32_t		degMinAngle ;		// 角度範囲最小値 degMinAngle ～ (degMinAngle+degAngle) [deg]
			float32_t		degAngle ;			// 角度範囲 [deg]
			float32_t		countPerSec ;		// 発生数 [/sec]
			float32_t		fpSpeed ;			// 速度
			float32_t		fpIndefinition ;	// 速度ゆらぎ率 [0,1]
			float32_t		fpAreaScale ;		// 発生領域範囲
			EmissionShape	shapeType ;			// 発生領域形状

			EmissionParam( void )
				: vDirection( 0, -1, 0 ),
					vBaseSpeed( 0, 0, 0 ),
					degMinAngle( 0.0f ),
					degAngle( 30.0f ),
					countPerSec( 100.0f ),
					fpSpeed( 10.0f ), fpIndefinition( 0.5f ),
					fpAreaScale( 1.0f ), shapeType( shapeSphere ) { }
		} ;
		enum	PhysicsFlag
		{
			physicsCollision	= 0x0001,	// 当たり判定あり
			physicsExtinction 	= 0x0002,	// 衝突時に消滅
			physicsAbsorption	= 0x0004,	// 吸引
		} ;
		struct	PhysicsParam
		{
			uint32_t	nFlags ;			// complex of enum PhysicsFlag
			S3DVector	vAcceleration ;		// 空間加速度 [/sec^2]
			S3DVector	vStream ;			// 媒質流速 [/sec]
			float32_t	fpAttenuation ;		// 減衰率 [/sec]
			float32_t	fpReaction ;		// 衝突反発係数
			float32_t	fpAbsorbAccel ;		// 吸収加速度 [/sec^2]
			float32_t	fpAbsorbExAccel ;	// 領域外吸収加速比
			float32_t	fpAbsorbRadius ;	// 吸収半径

			PhysicsParam( void )
				: nFlags( 0 ), vAcceleration( 0, 0, 0 ),
					vStream( 0, 0, 0 ),
					fpAttenuation( 0.99f ),
					fpReaction( 0.5f ),
					fpAbsorbAccel( 0.1f ),
					fpAbsorbExAccel( 0.1f ),
					fpAbsorbRadius( 0.1f ) { }
		} ;
		enum	ParticleFlag
		{
			particleDuration	= 0x0001,
			particleRotation	= 0x0002,
			particleRotation3D	= 0x0004,
			particleFaceDir		= 0x0008,
			particleLocalSpace	= 0x0010,
			particleMoveAspect	= 0x0020,
		} ;
		struct	ParticleParam
		{
			uint32_t	nFlags ;			// complex of enum ParticleFlag
			float32_t	secDuration ;		// 寿命 [sec]
			float32_t	secFadein ;			// フェードイン時間 [sec]
			float32_t	secFadeout ;		// フェードアウト時間 [sec]
			float32_t	fpSizeScale ;		// 粒子拡大率
			float32_t	fpSizeIndefinition ;// 粒子拡大率ゆらぎ率 [0,1]
			float32_t	fpSizeScaleEnd ;	// 消滅時の粒子サイズ
			float32_t	fpAnimationSpeed ;	// 画像アニメーション速度 [/sec]
			float32_t	dpsRotationSpeed ;	// 回転速度 [deg/sec]
			S3DVector	vDefFaceDir ;		// particleFaceDir での表示方向
			uint32_t	nBlurFrames ;		// ブラー影響フレーム数
			float32_t	fpBlurSupplement ;	// ブラー補完挿入距離
			float32_t	fpAspectSpeed ;		// 移動方向へのアスペクト拡大基準移動速度 [/sec]

			ParticleParam( void )
				: nFlags( 0 ), secDuration( 1.0f ),
					secFadein( 0.0f ), secFadeout( 0.5f ),
					fpSizeScale( 1.0f ), fpSizeIndefinition( 0.0f ),
					fpSizeScaleEnd( 1.0f ),
					fpAnimationSpeed( 1.0f ), dpsRotationSpeed( 0.0f ),
					vDefFaceDir( 0.0f, 0.0f, 1.0f ),
					nBlurFrames( 0 ), fpBlurSupplement( 10.0f ),
					fpAspectSpeed( 1.0 ) { }
		} ;
		struct	Particle
		{
			S3DVector	vPos ;
			S3DVector	vSpeed ;
			S3DVector	vRotate ;
			S3DVector	vRotateSpeed ;
			S3DVector	vFaceDir ;
			S3DColor	clrParticle ;
			float32_t	fpZoom ;
			float32_t	secLife ;
			float32_t	secAnimation ;
			float32_t	fpLifeSpeed ;
			size_t		nIdentity ;
			uint32_t	nLastPosCount ;
			S3DVector	vLastPos[4] ;
		} ;

	protected:
		SakuraCL::SCLRandomizer		m_randomizer ;
		EmissionParam				m_emission ;
		PhysicsParam				m_physics ;
		ParticleParam				m_particle ;

		SSystem::SSmartReference<RenderTarget>
									m_refRenderTarget ;
		SSystem::SString			m_strRenderTarget ;

		S3DDVector					m_vParamAccel ;
		double						m_fpParamAccel ;
		S3DDVector					m_vParamStream ;
		double						m_fpParamStream ;

		size_t						m_idNxtParticle ;
		SSystem::SArray<Particle>	m_aParticles ;

		SSystem::SArray<size_t>			m_bufTempVerCount ;
		SSystem::SArray<size_t>			m_bufTempMeshIndex ;
		SSystem::SArray<S3DDVector>		m_bufTempPos ;
		SSystem::SArray<S3DVector>		m_bufTempDir ;
		SSystem::SArray<S3DVector>		m_bufTempSpeed ;
		SSystem::SArray<float32_t>		m_bufTempLifeSpeed ;

		SSystem::SArray<S3DVector4>		m_bufPoints ;
		SSystem::SArray<size_t>			m_bufFrames ;
		SSystem::SArray<S3DColor>		m_bufColors ;
		SSystem::SArray<float32_t>		m_bufZooms ;
		SSystem::SArray<float32_t>		m_bufXAspects ;
		SSystem::SArray<S4DVector>		m_bufFaceDirs ;
		SSystem::SArray<ParticleIndex>	m_bufParticleIndex ;
		SSystem::SArray<S3DMatrix>		m_bufFaceMatrixs ;

		SSystem::SCriticalSection		m_csLock ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramParticleCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;
		static const wchar_t *	m_pwszEmissionTypeIDs[shapeCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DParticleSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DParticleSerializer, particle_emitter )
		// 構築関数
		S3DParticleSerializer( void ) ;

	public:
		// 出力ターゲット設定
		void AttachRenderTarget
			( RenderTarget * pTarget, const wchar_t * pwszID ) ;
		RenderTarget * GetRenderTarget( void ) const ;
		// 放出パラメータ
		void SetEmissionParameter( const EmissionParam& param ) ;
		const EmissionParam& GetEmissionParameter( void ) const ;
		// 物理パラメータ
		void SetPhysicParameter( const PhysicsParam& param ) ;
		const PhysicsParam& GetPhysicsParameter( void ) const ;
		// 粒子パラメータ
		void SetParticleParameter( const ParticleParam& param ) ;
		const ParticleParam& GetParticleParameter( void ) const ;
		// 粒子生成
		void GenerateParticles( double fpCount ) ;
		// 生成数計算
		size_t GenerateCount( double fpCount ) ;
		// 粒子生成（大域座標・方向・速度指定）
		void GenerateParticlesByParam
			( size_t nCount,
				const S3DDVector& vEmittionPos,
				const S3DVector * pvDir = NULL,
				const S3DVector * pvSpeed = NULL,
				const S3DVector * pvFaceDir = NULL,
				const S3DColor * pColor = NULL,
				float32_t fpZoom = 1.0f,
				float32_t fpLifeSpeed = 1.0f,
				float32_t fpSpeedScale = 1.0f,
				float32_t fpEmissionSpeed = 0.0f ) ;
		void GenerateParticlesWithParam
			( size_t nCount,
				const S3DDVector& vEmittionPos,
				const EmissionParam&  paramEmission,
				const S3DColor * pColor = NULL,
				float32_t fpZoom = 1.0f,
				float32_t fpLifeSpeed = 1.0f,
				float32_t fpSpeedScale = 1.0f,
				float32_t fpEmissionSpeed = 0.0f ) ;
		void GenerateParticlesByParams
			( size_t nCount,
				const S3DDVector * pvEmittionPoss,
				const S3DVector * pvDirs = NULL,
				const S3DVector * pvSpeeds = NULL,
				const S3DVector * pvFaceDirs = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pfpZooms = NULL,
				const float32_t * pfpLifeSpeeds = NULL ) ;
		// 粒子生成（発生形状メッシュ指定）
		enum	ParticleGenerationFlag
		{
			flagWithColorTexture	= 0x0001,
			flagWithoutColor		= 0x0002,
		} ;
		void GenerateParticlesOnMesh
			( S3DVertexBufferInterface * pVBO,
				const S3DDMatrix& matVBO,
				const S3DDVector& vGlobalPos,
				size_t nCount, uint32_t nFlags,
				const S3DColor& clrMul, const S3DColor& clrAdd,
				float32_t fpZoomRate = 1.0f, float32_t fpLifeSpeed = 1.0f,
				float32_t fpSpeedScale = 1.0f,
				float32_t fpEmissionSpeed = 0.0f,
				const size_t * pMeshIndexes = NULL, size_t nMeshCount = 0 ) ;
		// 乱数生成
		SakuraCL::SCLRandomizer& Randomizer( void )
		{
			return	m_randomizer ;
		}
		// 粒子消去
		void ClearAllParticles( void ) ;
		void FadeoutAllParticles( void ) ;

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

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:	// Item
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
	protected:
		// パーティクルをターゲットに出力
		void AddParticleToTarget( const S3DScene& scene ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーティクル放出アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DParticleShootSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramParticleItem	= ItemBasicSerializer::paramItemTotalCount,
			paramGenCount,
			paramCountPerFrame,
			paramUseSpaceScale,
			paramSizeScale,
			paramSpeedScale,
			paramEmissionSpeed,
			paramUseDirection,
			paramDirection,
			paramUseBaseSpeed,
			paramBaseSpeedDir,
			paramBaseSpeed,
			paramUseFaceDir,
			paramFaceDir,
			paramShootTotalCount,
			paramShootCount		= paramShootTotalCount - paramParticleItem,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramShootCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		SSystem::SString		m_strParticle ;
		SSystem::SSyncReference	m_refParticle ;

		bool		m_flagCountPerFrame ;
		bool		m_flagSpaceScale ;
		bool		m_flagDirection ;
		bool		m_flagBaseSpeed ;
		bool		m_flagFaceDir ;
		double		m_fpGenCount ;
		double		m_fpZoomScale ;
		double		m_fpEmissionSpeedScale ;
		double		m_fpEmissionSpeed ;
		S3DDVector	m_vDirection ;
		S3DDVector	m_vSpeedDir ;
		double		m_fpSpeed ;
		S3DDVector	m_vFaceDir ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DParticleShootSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DParticleShootSerializer, particle_shooter )
		// 構築関数
		S3DParticleShootSerializer( void ) ;

	public:
		// 出力先
		void AttachParticleItem
			( S3DParticleSerializer * pParticle, const wchar_t * pwszID ) ;
		bool UpdateParticleReference( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
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


}

#endif

