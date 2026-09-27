
#if	!defined(__SAKURAGL_OPENGL_CUSTOM_SHADER_H__)
#define	__SAKURAGL_OPENGL_CUSTOM_SHADER_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL カスタムシェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLCustomShader	: public SGLOpenGLDefaultShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLCustomShader, SGLOpenGLDefaultShader )
		// 構築関数
		SGLOpenGLCustomShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLCustomShader( void ) ;

	public:
		// 初期化
		SGLError InitializeProgram
			( const S3DRenderDevice::ShaderSourceInfo & features,
				SGLOpenGLShaderProgram::CompileListener * pListener ) ;
		// シェーダープログラム復元
		virtual SGLError LoadProgramBinary
			( const S3DShaderBinary& bin,
				SGLOpenGLShaderProgram::CompileListener * pListener = nullptr ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// GLSL Compute Shader
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLComputeShader	: public SGLOpenGLShaderProgram,
										public S3DComputeShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SakuraGL::SGLOpenGLComputeShader,
				SGLOpenGLShaderProgram, S3DComputeShaderInterface )
		// 構築関数
		SGLOpenGLComputeShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLComputeShader( void ) ;

	public:
		// 初期化
		SGLError InitializeProgram
			( const S3DRenderDevice::ShaderSourceInfo & features,
				SGLOpenGLShaderProgram::CompileListener * pListener ) ;
		// シェーダープログラム保存
		virtual SGLError SaveProgramBinary( S3DShaderBinary& bin ) ;
		// シェーダープログラム復元
		virtual SGLError LoadProgramBinary
			( const S3DShaderBinary& bin,
				SGLOpenGLShaderProgram::CompileListener * pListener = nullptr ) ;

	public:
		// ローカルサイズ取得
		virtual DimSize GetWorkLocalSize( void ) const ;
		// 実行（レンダリングスレッド上で）
		virtual SGLError Execute
			( S3DRenderDevice * pDevice, const ExecuteParam& param ) ;

	protected:
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix( const S4DMatrix& mat4 ) ;
		// 投影スクリーン座標設定
		virtual void SetProjectionScreen
			( float32_t xScreen, float32_t yScreen, float32_t zScreen ) ;
		// モデル変換行列設定
		virtual void SetModelViewMatrix
				( const S4DMatrix& mat4, bool fInverseNormal = false ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// 三角ポリゴン→ワイヤーフレーム描画シェーダ―
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLSimpleTriangle2LineShader
						: public SGLOpenGLCustomShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLOpenGLSimpleTriangle2LineShader, SGLOpenGLCustomShader )
		// 構築関数
		SGLOpenGLSimpleTriangle2LineShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSimpleTriangle2LineShader( void ) ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// UV→空間座標／三角ポリゴン→ワイヤーフレーム描画シェーダ―
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLSimpleTriangle2UVLineShader
						: public SGLOpenGLCustomShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLOpenGLSimpleTriangle2UVLineShader, SGLOpenGLCustomShader )
		// 構築関数
		SGLOpenGLSimpleTriangle2UVLineShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSimpleTriangle2UVLineShader( void ) ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DrawWithDepth シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLDrawWithDepthShader
				: public SGLOpenGLCustomShader,
					public S3DDrawWithDepthShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLDrawWithDepthShader,
				SGLOpenGLCustomShader, S3DDrawWithDepthShaderInterface )
		// 構築関数
		SGLOpenGLDrawWithDepthShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLDrawWithDepthShader( void ) ;

	protected:
		ssize_t		u_samplerDepth ;			// sampler2D
		ssize_t		u_mat4SamplePers ;			// mat4

		SGLImageObject *	m_samplerDepth ;
		S4DMatrix			m_mat4SamplePers ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// ｚバッファ設定
		virtual void SetDepthBuffer( SGLImageObject * pDepthBuf ) ;
		// ｚバッファ透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL ガウスぼかしシェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLGaussianBlurShader
				: public SGLOpenGLCustomShader,
					public S3DGaussianBlurShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLGaussianBlurShader,
				SGLOpenGLCustomShader, S3DGaussianBlurShaderInterface )
		// 構築関数
		SGLOpenGLGaussianBlurShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLGaussianBlurShader( void ) ;

	public:
		enum	Value
		{
			WeightCount	= 9,
		} ;
		static const wchar_t *	SHADER_ID ;

	protected:
		ssize_t		u_fpGaussianWeight ;		// float[9] 
		ssize_t		u_vSamplingDirection ;		// vec2
		double		m_fpGauss ;
		double		m_fpBrightness ;
		S2DVector	m_vSampleDirection ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// ガウス係数計算
		void CalcGauss( float32_t fpGauss[] ) const ;

	public:
		// ガウス係数設定
		virtual void SetGauss( double g ) ;
		// サンプリング方向設定
		virtual void SetDirection( double x, double y ) ;
		// 輝度設定
		virtual void SetBrightness( double b ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL ガウス放射状ブラーシェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLGaussianRadialBlurShader
				: public SGLOpenGLCustomShader,
					public S3DGaussianRadialBlurShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLGaussianRadialBlurShader,
				SGLOpenGLCustomShader, S3DGaussianRadialBlurShaderInterface )
		// 構築関数
		SGLOpenGLGaussianRadialBlurShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLGaussianRadialBlurShader( void ) ;

	public:
		enum	Value
		{
			WeightCount	= 9,
		} ;
		static const wchar_t *	SHADER_ID ;

	protected:
		ssize_t		u_fpGaussianWeight ;		// float[9] 
		ssize_t		u_vRadialCenter ;			// vec2
		ssize_t		u_fpSamplingUnit ;			// float
		ssize_t		u_fpBlurScale ;				// float
		ssize_t		u_fpBlurPower ;				// float
		double		m_fpGauss ;
		double		m_fpBrightness ;
		S2DVector	m_vRadialCenter ;
		float32_t	m_fpSamplingUnit ;
		float32_t	m_fpBlurScale ;
		float32_t	m_fpBlurPower ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// ガウス係数計算
		void CalcGauss( float32_t fpGauss[] ) const ;

	public:
		// ガウス係数設定
		virtual void SetGauss( double g ) ;
		// 放射中心設定
		virtual void SetCenter( double x, double y ) ;
		// 輝度設定
		virtual void SetBrightness( double b ) ;
		// サンプリング単位距離設定
		virtual void SetSamplingUnit( double d ) ;
		// ブラースケール : k = (r / x) ^ p,  r : 放射中心からの変位
		virtual void SetBlueScale( double x, double p ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL 被写界深度シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLDepthBlenderShader
				: public SGLOpenGLCustomShader,
					public S3DDepthBlenderShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLDepthBlenderShader,
				SGLOpenGLCustomShader, S3DDepthBlenderShaderInterface )
		// 構築関数
		SGLOpenGLDepthBlenderShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLDepthBlenderShader( void ) ;

	public:
		static const wchar_t *	SHADER_ID ;

	protected:
		ssize_t		u_samplerDepth ;		// sampler2D
		ssize_t		u_vDepthTextureScale ;	// vec2
		ssize_t		u_fpFocusDepth ;		// float
		ssize_t		u_fpFocusNearRange ;	// float
		ssize_t		u_fpFocusFarRange ;		// float
		ssize_t		u_fpPersM22 ;			// float
		ssize_t		u_fpPersM23 ;			// float

		SGLImageObject *	m_pDepthBuf ;
		S2DVector			m_vDepthScale ;
		float32_t			m_fpFocusDepth ;
		float32_t			m_fpFocusNearRange ;
		float32_t			m_fpFocusFarRange ;
		float32_t			m_fpPersM22 ;
		float32_t			m_fpPersM23 ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// ｚバッファ設定
		virtual void SetDepthBuffer( SGLImageObject * pDepthBuf ) ;
		// 焦点深度値設定
		virtual void SetFocusDepth( double zFocus ) ;
		// ぼかし深度幅設定
		virtual void SetDepthRange( double zNearRange, double zFarRange ) ;
		// ｚ値変換用係数設定
		virtual void SetPersParameter( double m22, double m23 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL 遅延シェーダー光源
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLDelayLightShader
				: public SGLOpenGLCustomShader,
					public S3DDelayLightShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLDelayLightShader,
				SGLOpenGLCustomShader, S3DDelayLightShaderInterface )
		// 構築関数
		SGLOpenGLDelayLightShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLDelayLightShader( void ) ;

	protected:
		ssize_t		u_samplerDepth ;			// sampler2D
		ssize_t		u_samplerNormal ;			// sampler2D
		ssize_t		u_samplerSpecular ;			// sampler2D
		ssize_t		u_mat4SamplePers ;			// mat4
		ssize_t		u_fpDiffusion ;				// float
		ssize_t		u_fpSpecular ;				// float
		ssize_t		u_fpAirScattering ;			// float
		ssize_t		u_fpAirRcpUnit ;			// float
		ssize_t		u_typeDLighting ;			// int
		ssize_t		u_vDLPosition ;				// vec3
		ssize_t		u_vDLDirection ;			// vec3
		ssize_t		u_vDLColor ;				// vec3
		ssize_t		u_fpDLBrightness ;			// float
		ssize_t		u_fpDLAttenuationPower ;	// float
		ssize_t		u_fpDLAngle ;				// float
		ssize_t		u_fpDLGradation ;			// float

		SGLImageObject *	m_samplerDepth ;
		SGLImageObject *	m_samplerNormal ;
		SGLImageObject *	m_samplerSpecular ;
		S4DMatrix			m_mat4SamplePers ;
		float32_t			m_fpDiffusion ;
		float32_t			m_fpSpecular ;
		float32_t			m_fpAirScattering ;
		float32_t			m_fpAirRcpUnit ;
		int32_t				m_typeDLighting ;
		S3DVector			m_vDLPosition ;
		S3DVector			m_vDLDirection ;
		S3DVector			m_vDLColor ;
		float32_t			m_fpDLBrightness ;
		float32_t			m_fpDLAttenuationPower ;
		float32_t			m_fpDLAngle ;
		float32_t			m_fpDLGradation ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// 光源情報設定（座標は透視空間）
		virtual void SetLightParam( const S3DLightEntry & light ) ;
		// 光源描画効果
		virtual void SetLightApplication
					( float32_t fpDiffusion, float32_t fpSpecular ) ;
		// 大気散乱効果
		virtual void SetAirScattering
					( float32_t fpScattering, float32_t fpDistanceUnit ) ;
		// 透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL 画面空間大域照明
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLSSGISamplingShader
				: public SGLOpenGLCustomShader,
					public S3DSSGISamplerInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLSSGISamplingShader,
				SGLOpenGLCustomShader, S3DSSGISamplerInterface )
		// 構築関数
		SGLOpenGLSSGISamplingShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSSGISamplingShader( void ) ;

	protected:
		ssize_t		u_samplerDepth ;			// sampler2D
		ssize_t		u_samplerNormal ;			// sampler2D
		ssize_t		u_samplerEmission ;			// sampler2D
		ssize_t		u_samplerDiffusion ;		// sampler2D
		ssize_t		u_samplerSpecular ;			// sampler2D
		ssize_t		u_mat4SamplePers ;			// mat4
		ssize_t		u_fpReachAO ;				// float
		ssize_t		u_fpReachAObyZ ;			// float
		ssize_t		u_nAOSamplingCount ;		// int
		ssize_t		u_nGISamplingCount ;		// int
		ssize_t		u_fpDiffusionLuminousness ;	// float
		ssize_t		u_bWith3WayMapping ;		// bool
		ssize_t		u_sampler3WayComposed ;		// sampler2D
		ssize_t		u_sampler3WayEmission ;		// sampler2D
		ssize_t		u_sampler3WayDepth ;		// sampler2D
		ssize_t		u_mat4Sample3WayPers ;		// mat4

		SGLImageObject *	m_samplerDepth ;
		SGLImageObject *	m_samplerNormal ;
		SGLImageObject *	m_samplerEmission ;
		SGLImageObject *	m_samplerDiffusion ;
		SGLImageObject *	m_samplerSpecular ;
		S4DMatrix			m_mat4SamplePers ;
		float32_t			m_fpReachAO ;
		float32_t			m_fpReachAObyZ ;
		int32_t				m_nAOSamplingCount ;
		int32_t				m_nGISamplingCount ;
		float32_t			m_fpDiffusionLuminousness ;
		int32_t				m_bWith3WayMapping ;
		SGLImageObject *	m_sampler3WayComposed ;
		SGLImageObject *	m_sampler3WayEmission ;
		SGLImageObject *	m_sampler3WayDepth ;
		S4DMatrix			m_mat4Sample3WayPers ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// AO 到達距離
		virtual void SetAOReachDistance
			( float32_t fpReach, float32_t fpZProportion ) ;
		// AO サンプリング数
		virtual void SetAOSamplingCount( size_t nCount ) ;
		// GI サンプリング数
		virtual void SetGISamplingCount( size_t nCount ) ;
		// GI 輝度
		virtual void SetGILuminousness( float32_t fpDiffusion ) ;
		// 透視変換行列
		virtual void SetPerspective( const S4DMatrix& matPers ) ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) ;
		// 3面パノラマフレーム有効化
		virtual void Enable3WayPanoramaBuffer( bool fEnable ) ;
		// 3面パノラマ透視変換行列
		virtual void Set3WayPerspective( const S4DMatrix& matPers ) ;
		// レンダリング済み3面パノラマバッファ
		virtual void Set3WayPanoramaBuffer
			( SGLImageObject*const* ppColorBufs,
				size_t nColorBufCount, SGLImageObject * pDepthBuf ) ;
	} ;

	class	SGLOpenGLSSGIComposerShader
				: public SGLOpenGLCustomShader,
					public S3DSSGIComposerInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLSSGIComposerShader,
				SGLOpenGLCustomShader, S3DSSGIComposerInterface )
		// 構築関数
		SGLOpenGLSSGIComposerShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSSGIComposerShader( void ) ;

	protected:
		ssize_t		u_samplerComposed ;			// sampler2D
		ssize_t		u_samplerAmbient ;			// sampler2D
		ssize_t		u_vSamplingScale ;			// vec2
		ssize_t		u_fpBlendAO ;				// float
		ssize_t		u_fpBlendGI ;				// float
		ssize_t		u_rgbAOShadeColor ;			// vec3

		SGLImageObject *	m_samplerComposed ;
		SGLImageObject *	m_samplerAmbient ;
		S2DVector			m_vSamplingScale ;
		float32_t			m_fpBlendAO ;
		float32_t			m_fpBlendGI ;
		S3DVector			m_rgbAOShadeColor ;

	public:
		// ShaderSourceInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// サンプリングスケール
		virtual void SetSamplingScale( const S2DVector& vScale ) ;
		// AO/GI 適用度
		virtual void SetBlendRatio( float32_t fpAO, float32_t fpGI ) ;
		// AO 影加算色
		virtual void SetAOShadeColor( const SGLPalette& rgbShade ) ;
		// レンダリング済みバッファ
		virtual void SetSourceBuffer
			( SGLImageObject*const* ppColorBufs, size_t nColorBufCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL モザイク描画シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLSimpleMosaicShader	: public SGLOpenGLCustomShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLOpenGLSimpleMosaicShader, SGLOpenGLCustomShader )
		// 構築関数
		SGLOpenGLSimpleMosaicShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSimpleMosaicShader( void ) ;

	protected:
		ssize_t		u_vMosaicSize ;		// vec2
		S2DVector	m_vMosaicSize ;

	public:
		// FeatureInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// モザイクサイズ設定
		void SetMosaicSize( double x, double y ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL 水面描画シェーダー（フォンシェーディング）
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLSimpleWaterShader	: public SGLOpenGLCustomShader,
											public S3DSimpleWaterShaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLOpenGLSimpleWaterShader,
				SGLOpenGLCustomShader, S3DSimpleWaterShaderInterface )
		// 構築関数
		SGLOpenGLSimpleWaterShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLSimpleWaterShader( void ) ;

	protected:
		ssize_t		u_fpAmplitude ;		// float : 振幅比
		ssize_t		u_fpNormalAmp ;		// float : 法線振幅比
		ssize_t		u_vAmplitude ;		// vec4 : 振幅
		ssize_t		u_vBumpAmplitude ;	// vec4
		ssize_t		u_vFrequency ;		// vec4 : 周波数 [Hz/2π]
		ssize_t		u_vBumpFrequency ;	// vec4
		ssize_t		u_vTime ;			// vec4 : 時間 [rad]
		ssize_t		u_vBumpTime ;		// vec4
		ssize_t		u_vDirection ;		// vec2[4] : 方向
		ssize_t		u_vBumpDirection ;	// vec2[4]
		ssize_t		u_vWaterAxisX ;		// vec3 : 水面基底ベクトル
		ssize_t		u_vWaterAxisY ;		// vec3
		ssize_t		u_fpCascadeFarZ ;		// float
		ssize_t		u_fpCascadePhase1 ;		// float
		ssize_t		u_fpCascadeAmplitude1 ;	// float

		float32_t	m_fpAmplitude ;
		float32_t	m_fpNormalAmp ;
		S4DVector	m_vAmplitude ;
		S4DVector	m_vBumpAmplitude ;
		S4DVector	m_vFrequency ;
		S4DVector	m_vBumpFrequency ;
		S4DVector	m_vTime ;
		S4DVector	m_vBumpTime ;
		S2DVector	m_vDirection[4] ;
		S2DVector	m_vBumpDirection[4] ;
		S3DVector	m_vWaterAxisX ;
		S3DVector	m_vWaterAxisY ;
		float32_t	m_fpCascadeFarZ ;
		float32_t	m_fpCascadePhase1 ;
		float32_t	m_fpCascadeAmplitude1 ;

	public:
		// FeatureInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	public:
		// パラメータ設定
		virtual void SetParameter( const S3DSimpleWaterShaderInterface::Parameter& param ) ;
		// 時間を更新
		virtual void SetTimeParameter
			( const float32_t * pTime, const float32_t * pBumpTime ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL Shadowmap 用 5x5 フィルタ (Compute Shader)
	//////////////////////////////////////////////////////////////////////////

	class	S3DOpenGLShadowmapDepthFilter5x5Shader : public SGLOpenGLComputeShader,
												public S3DShadowmapDepthFilter5x5Interface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DOpenGLShadowmapDepthFilter5x5Shader,
				SGLOpenGLComputeShader, S3DShadowmapDepthFilter5x5Interface )
		// 構築関数
		S3DOpenGLShadowmapDepthFilter5x5Shader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~S3DOpenGLShadowmapDepthFilter5x5Shader( void ) ;

	protected:
		ssize_t	u_imageInput ;
		ssize_t	u_imageOutput ;
		ssize_t	u_nImageWidth ;
		ssize_t	u_nImageHeight ;
		ssize_t	u_nImageLayers ;
		ssize_t	u_filterKernel ;

		SGLImageObject *	m_pInputDepth ;
		SGLImageObject *	m_pOutputDepth ;

		static constexpr const size_t	KernelSize = 5 * 5 ;
		float32_t						m_fpFilterKernel[KernelSize] ;

		static const float32_t			s_fpDefaultKernel[KernelSize] ;

	public:
		// FeatureInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;
		// 現在のパラメータに合致する実行ワークグループサイズを計算
		virtual S3DComputeShaderInterface::DimSize
						CalcWorkGroupSize( void ) const ;

	public:
		// 入力画像
		virtual void SetInputDepth( SGLImageObject * pInput ) ;
		// 出力画像
		virtual void SetOutputDepth( SGLImageObject * pOutput ) ;
		// フィルター行列 (5x5)
		virtual void SetFilterKernel( const float32_t * pKernel ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL アナグリフ表示シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLAnaglyphShader	: public SGLOpenGLCustomShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLAnaglyphShader, SGLOpenGLCustomShader )
		// 構築関数
		SGLOpenGLAnaglyphShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLAnaglyphShader( void ) ;

	public:
		static const wchar_t *	SHADER_ID ;

	public:
		// FeatureInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 GLSL インターリーブ表示シェーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLOpenGLInterleaveShader	: public SGLOpenGLCustomShader
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLOpenGLInterleaveShader, SGLOpenGLCustomShader )
		// 構築関数
		SGLOpenGLInterleaveShader( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~SGLOpenGLInterleaveShader( void ) ;

	public:
		static const wchar_t *	SHADER_ID ;

	public:
		// FeatureInfo 取得
		static void GetSourceInfo
				( S3DRenderDevice::ShaderSourceInfo & source ) ;
		// カスタムユニフォーム設定
		virtual void OnInitCustomUniform( void ) ;

	public:
		// シェーダーパラメータをレンダラに設定
		virtual void SetShaderUniformsTo( S3DRenderContextInterface& render ) ;
		// シェーダーパラメータを UniformSet に設定
		virtual void SetShaderUniformsTo( S3DCustomShader::UniformSet& unis ) ;

	} ;

}

#endif

