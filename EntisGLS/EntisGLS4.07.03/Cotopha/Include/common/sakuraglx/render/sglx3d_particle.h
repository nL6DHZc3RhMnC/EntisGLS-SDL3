
#if	!defined(__SAKURAGLX3D_PARTICLE_H__)
#define	__SAKURAGLX3D_PARTICLE_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 抽象パーティクル
	//////////////////////////////////////////////////////////////////////////

	class	S3DParticleGenerator	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DParticleGenerator, ESLObject )
		// 構築関数
		S3DParticleGenerator( void ) ;
		// 消滅関数
		virtual ~S3DParticleGenerator( void ) ;

	public:
		// 乱数範囲
		struct	NumberRange
		{
			float32_t	fpNumber ;
			float32_t	fpRange ;

			NumberRange( void ) : fpNumber(0.0), fpRange(0.0) {}
			NumberRange( double num, double r ) ;
			NumberRange( const NumberRange& nr ) ;
			const NumberRange& operator = ( const NumberRange& nr ) ;
		} ;
		// 生成パラメータ
		struct	GenerationParam
		{
			S3DVector	vPosition ;		// 生成位置
			S3DVector	vOffset ;		// 生成線分終端相対位置
			float32_t	fpRadius ;		// 生成半径
			S3DVector	vDirection ;	// 生成方向（長さは無視される）
			float32_t	fpAngle ;		// 生成幅片角 [deg]
			NumberRange	nrVelocity ;	// 初速
			size_t		nGenPerSec ;	// 生成数 [/sec]

			GenerationParam( void )
				: fpRadius(0.0), vDirection(0,0,1),
					fpAngle(180.0), nGenPerSec(0) {}
			GenerationParam( const GenerationParam& gp ) ;
			const GenerationParam& operator = ( const GenerationParam& gp ) ;
		} ;
		// 媒質パラメータ
		struct	FieldParam
		{
			S3DVector	vGravity ;		// 加速度 [/sec^2]
			S3DVector	vStream ;		// 流速 [/sec]
			float32_t	fpAttenuation ;	// 減衰率（＆流速影響）[/sec]

			FieldParam( void ) : fpAttenuation(0.9f) {}
			FieldParam( const FieldParam& fp ) ;
			const FieldParam& operator = ( const FieldParam& fp ) ;
		} ;
		// 揺らぎパラメータ
		struct	FlickeringParam
		{
			NumberRange	nrAmplitude ;		// 振幅
			NumberRange	nrCycle ;			// 周期 [sec]

			FlickeringParam( void ) {}
			FlickeringParam( const FlickeringParam& fp ) ;
			const FlickeringParam& operator = ( const FlickeringParam& fp ) ;
		} ;
		// 粒子パラメータ
		struct	ParticleParam
		{
			size_t			msecFadein ;		// フェードイン時間 [ms]
			size_t			msecFadeout ;		// フェードアウト時間 [ms]
			size_t			msecDuration ;		// 寿命 [ms]
			NumberRange		nrRotation ;		// 自転速度 [deg/sec]
			float32_t		fpObliquity ;		// 自転軸傾斜角幅 [deg]
			NumberRange		nrZoom ;			// 拡大率
			float32_t		fpZoomIn ;			// 開始時拡大率比
			float32_t		fpZoomOut ;			// 終了時拡大率比
			FlickeringParam	fpFlickering[2] ;	// 揺らぎ

			ParticleParam( void ) ;
			ParticleParam( const ParticleParam& pp ) ;
			const ParticleParam& operator = ( const ParticleParam& pp ) ;
		} ;
		// 揺らぎインスタンス
		struct	FlickeringInstance
		{
			S3DVector	vAmplitude ;		// 揺らぎベクトル
			float32_t	secCycle ;			// 周期 [sec]
			float32_t	secPhase ;			// 位相 [sec]
		} ;
		// 粒子インスタンス
		struct	ParticleInstance
		{
			S3DVector			vPosition ;			// 座標
			S3DVector			vSpeed ;			// 速度 [/sec]
			size_t				msecLife ;			// 経過時間 [ms]
			float32_t			speedRotation ;		// 自転速度 [deg/sec]
			float32_t			degRotation ;		// 自転角 [deg]
			S3DVector			axisRotation ;		// 自転軸
			float32_t			fpZoom ;			// 拡大率
			FlickeringInstance	fiFlickering[2] ;	// 揺らぎ

			void CalcPosition( S3DVector& vPos ) const ;
		} ;
		// 粒子効果結果
		enum	ParticleEffectResult
		{
			effectContiue,
			effectRemove,
		} ;
		// 粒子効果オブジェクト
		class	ParticleEffector	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ParticleEffector, ESLObject )
			// 構築関数
			ParticleEffector( void ) ;
			ParticleEffector( const ParticleEffector& pe ) ;
			// 消滅関数
			virtual ~ParticleEffector( void ) ;
		public:
			// 効果
			virtual ParticleEffectResult
				EffectParticle
					( const S3DParticleGenerator& pg,
						ParticleInstance& pi, size_t msecTime ) = 0 ;
		} ;
		// 吸引オブジェクト
		class	ParticleAbsorber	: public ParticleEffector
		{
		public:
			S3DVector	m_vPos ;		// 吸引座標
			double		m_rAbsob ;		// 吸引（消去）半径
			double		m_fpGravity ;	// 重力加速度 [/sec^2]
			double		m_fpStream ;	// 吸引流速 [/sec]

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ParticleAbsorber, ParticleEffector )
			// 構築関数
			ParticleAbsorber( void ) ;
			ParticleAbsorber
				( const S3DVector& vPos, double r, double g, double str ) ;
		public:
			// 効果
			virtual ParticleEffectResult
				EffectParticle
					( const S3DParticleGenerator& pg,
							ParticleInstance& pi, size_t msecTime ) ;
		} ;

	protected:
		SSystem::SObjectArray<ParticleInstance>	m_particles ;
		SSystem::SObjectArray<ParticleEffector>	m_effectors ;

		GenerationParam			m_gparam ;
		FieldParam				m_fparam ;
		ParticleParam			m_pparam ;

		SakuraCL::SCLRandomizer	m_random ;

	public:
		// 生成パラメータ
		const GenerationParam& GetGenerationParam( void ) const
		{
			return	m_gparam ;
		}
		void SetGenerationParam( const GenerationParam& gp ) ;
		// 媒質パラメータ
		const FieldParam& GetFieldParam( void ) const
		{
			return	m_fparam ;
		}
		void SetFieldParam( const FieldParam& fp ) ;
		// 粒子パラメータ
		const ParticleParam& GetParticleParam( void ) const
		{
			return	m_pparam ;
		}
		void SetParticleParam( const ParticleParam& pp ) ;
		// 疑似乱数の種を設定
		void SetRandomSeed( uint32_t nSeed ) ;
		// パーティクル取得
		const ParticleInstance *const * GetParticleArray( size_t& nCount ) const ;
		// 効果オブジェクト追加
		void AddEffector( ParticleEffector * pEffector ) ;
		// 効果オブジェクト削除
		void RemoveEffector( ParticleEffector * pEffector ) ;

	public:
		// 時間を進める
		void AdvanceTime( size_t msecTime ) ;
		// 乱数生成
		float32_t RandomizeNumber( double fpRange ) ;
		float32_t RandomizeNumber( const NumberRange& nr ) ;

	public:
		// 粒子生成
		virtual void GenerateParticle( size_t nCount ) ;
		// 生成座標・初速
		virtual void GenerateParticlePosition( ParticleInstance& pi ) ;

	public:
		// 粒子挙動
		virtual void AdvanceParticle( size_t msecTime ) ;
		// 粒子挙動
		virtual ParticleEffectResult
			MoveParticle(  ParticleInstance& pi, size_t msecTime ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーティクル・アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DParticleBillboardItem	: public S3DScene::BillboardItem,
											public S3DParticleGenerator
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( S3DParticleBillboardItem, BillboardItem, S3DParticleGenerator )
		// 構築関数
		S3DParticleBillboardItem( void ) ;
		// 消滅関数
		virtual ~S3DParticleBillboardItem( void ) ;

	public:
		// フラグ
		enum	BillboardFlag
		{
			flagLoopAnimation	= 0x0001,	// アニメーション画像はループする
		} ;
		struct	ImageParam
		{
			S3DMeshShaper::BillboardParam	bp ;
			uint32_t						nBillboardFlags ;
			uint32_t						nZBufFlags ;
			uint32_t						fxAnimeSpeed ;
		} ;

	protected:
		ImageParam	m_ipImage ;

	public:
		// パーティクル画像設定
		void SetBillboardParam
			( const S3DMeshShaper::BillboardParam& bp,
				uint32_t flagsBillboard = 0,
				uint32_t flagsZBuf = shadingZBufferNoWrite,
				uint32_t fxAnimeSpeed = 0x10000 ) ;

	public:
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	} ;


}

#endif

