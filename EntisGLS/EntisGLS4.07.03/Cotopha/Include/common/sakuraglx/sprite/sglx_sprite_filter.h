
#if	!defined(__SAKURAGLX_SPRITE_FILTER_H__)
#define	__SAKURAGLX_SPRITE_FILTER_H__	1

#include <sakuragl/sgl3d/sgl_render_shaper.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 透明度描画フィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilterTransparencyDrawer	: public SGLSpriteFilter
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilterTransparencyDrawer, SGLSpriteFilter )
		// 構築関数
		SGLSpriteFilterTransparencyDrawer( void ) ;
		SGLSpriteFilterTransparencyDrawer
				( const SGLSpriteFilterTransparencyDrawer& ftd ) ;
		// 消滅関数
		virtual ~SGLSpriteFilterTransparencyDrawer( void ) ;

	public:	// SGLSpriteDrawer 実装
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) ;
		// 動的描画フィルタか？
		virtual bool IsDynamicDrawer( void ) const ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// αチャネル窓関数画像フィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilterBlendAlpha	: public SGLSpriteFilter
	{
	protected:
		SSystem::SString	m_strAlphaFile ;
		SSystem::SSmartPointer<SGLImageObject>
							m_pAlphaImage ;

		SSystem::SSmartReference<SGLImageObject>
							m_refAlphaImage ;
		int32_t				m_fxAlphaCoefficient ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilterBlendAlpha, SGLSpriteFilter )
		// 構築関数
		SGLSpriteFilterBlendAlpha( void ) ;
		SGLSpriteFilterBlendAlpha( const SGLSpriteFilterBlendAlpha& filter ) ;
		// 消滅関数
		virtual ~SGLSpriteFilterBlendAlpha( void ) ;
		// 画像ファイル読み込み
		SGLError LoadAlphaImage( const wchar_t * pwszFilePath ) ;
		// 画像を関連付ける
		void AttachAlphaImage( SGLImageObject* pImage ) ;
		// パラメータ設定
		void SetAlphaParameter( int32_t fxAlphaCoefficient = 0x100 ) ;

	public:	// SGLSpriteFilter 実装
		// フィルタ処理
		virtual void Filter
			( S3DRenderContextInterface& render, SGLImageObject* image ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// トーンフィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilterTone	: public SGLSpriteFilter
	{
	public:
		enum	FilterFlag
		{
			flagFixZero			= 0x0001,
			flagMaskWithAlpha	= 0x0002,
			flagYUVFilter		= 0x0004,
			flagGrayFilter		= 0x0008,
		} ;

	protected:
		bool		m_flagMorphFilter ;
		uint32_t	m_nFilterFlags ;
		uint8_t		m_bufTone[4][0x100] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilterTone, SGLSpriteFilter )
		// 構築関数
		SGLSpriteFilterTone( void ) ;
		SGLSpriteFilterTone( const SGLSpriteFilterTone& filter ) ;
		// 消滅関数
		virtual ~SGLSpriteFilterTone( void ) ;

	public:
		// フィルタファイルを読み込む
		virtual SGLError LoadFilterFile( const wchar_t * pwszFilePath ) ;
		virtual SGLError ReadFilterFile( SSystem::SFileInterface& file ) ;
		// ストレートフィルタを設定
		void LoadStraightFilter( void ) ;
		// トーンカーブを取得
		uint32_t GetToneFilter
				( uint8_t * pbytRed, uint8_t * pbytGreen,
					uint8_t * pbytBlue, uint8_t * pbytAlpha ) ;
		// トーンカーブを設定
		virtual void SetToneFilter
			( const uint8_t * pbytRed,
				const uint8_t * pbytGreen,
				const uint8_t * pbytBlue,
				const uint8_t * pbytAlpha, uint32_t nFlags = 0 ) ;
		// トーンカーブを生成
		virtual void GenerateToneFilter
			( int nRed, int nGreen, int nBlue, int nAlpha,
								int nType, uint32_t nFlags = 0 ) ;
		// モーフィング有効／無効化
		void EnableMorphing( bool flagMorph ) ;

	public:
		// 代入
		const SGLSpriteFilterTone&
				operator = ( const SGLSpriteFilterTone& filter ) ;
		// 積
		SGLSpriteFilterTone operator * ( double t ) ;
		const SGLSpriteFilterTone& operator *= ( double t ) ;
		// 和
		SGLSpriteFilterTone
			operator + ( const SGLSpriteFilterTone& filter ) ;
		const SGLSpriteFilterTone&
			operator += ( const SGLSpriteFilterTone& filter ) ;

	public:	// SGLSpriteFilter 実装
		// フィルタ処理
		virtual void Filter
			( S3DRenderContextInterface& render, SGLImageObject* image ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易ぼかしフィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilterShadingOff	: public SGLSpriteFilter
	{
	protected:
		SSystem::SObjectArray<SGLImage>	m_arrScaledBuffer ;
		SGLImage	m_imgTempBuf ;
		SGLImage	m_imgTempBuf2 ;
		uint32_t	m_nShadingScale ;
		bool		m_flagTransition ;
		SGLPalette	m_argbOverColor ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilterShadingOff, SGLSpriteFilter )
		// 構築関数
		SGLSpriteFilterShadingOff( void ) ;
		SGLSpriteFilterShadingOff( const SGLSpriteFilterShadingOff& filter ) ;
		// 消滅関数
		virtual ~SGLSpriteFilterShadingOff( void ) ;
		// トランジッション設定（第二パラメータでの透明度と色効果）
		void SetTransitionOption
			( bool flagTransition, uint32_t argbOverColor ) ;

	public:	// SGLSpriteFilter 実装
		// 動的描画フィルタか？
		virtual bool IsDynamicDrawer( void ) const ;
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) ;
		// フィルタ処理
		virtual void Filter
			( S3DRenderContextInterface& render, SGLImageObject* image ) ;
	protected:
		#if	!defined(__COTOPHA__)
		void FilterByGPU
			( S3DRenderContextInterface * render, SGLImageObject* image ) ;
		#endif
		void FilterByCPU
			( S3DRenderContextInterface * render, SGLImageObject* image ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// メッシュワープフィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilterMeshWarp	: public SGLSpriteFilter
	{
	public:
		// メッシュ・エフェクタ
		class	Effector	: public SGLObject
		{
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Effector, SGLObject )
			// 構築関数
			Effector( void ) ;
			Effector( const Effector& eff ) ;
			// 初期設定
			virtual void OnAttachedMesh
					( size_t wMesh, size_t hMesh,
							size_t wCanvas, size_t hCanvas ) ;
			// 時間経過
			virtual bool OnTimer
				( SGLSprite& sprite,
					SGLSpriteFilterMeshWarp& mesh, uint32_t msecPast ) ;
			// メッシュ頂点変位加算
			virtual void WarpMesh
				( SGLSpriteFilterMeshWarp& mesh,
					S2DVector * pvDst,
					const S2DVector * pvSrc, size_t wMesh, size_t hMesh ) ;
		} ;

	protected:
		SSystem::SObjectArray<Effector>	m_effectors ;
		SSystem::SArray<S2DVector>		m_meshSrc ;
		SSystem::SArray<S2DVector>		m_meshDst ;
		bool							m_flagSpcSrcMesh ;
		bool							m_flagDynamicDraw ;
		bool							m_flagFixBorder ;
		SGLSize							m_sizeInPixels ;
		SGLSize							m_sizeInMesh ;
		SGLImageRect					m_rectDstMesh ;
		SGLImage						m_imgFilterBuffer ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilterMeshWarp, SGLSpriteFilter )
		// 構築関数
		SGLSpriteFilterMeshWarp( void ) ;
		SGLSpriteFilterMeshWarp( const SGLSpriteFilterMeshWarp& filter ) ;
		// 消滅関数
		virtual ~SGLSpriteFilterMeshWarp( void ) ;

	public:
		// メッシュサイズ設定
		void SetMeshSize( size_t width, size_t height ) ;
		// 画像サイズ設定
		void SetCanvasSize( size_t width, size_t height ) ;
		// メッシュサイズ取得
		const SGLSize& GetMeshSize( void ) const ;
		// 画像サイズ取得
		const SGLSize& GetCanvasSize( void ) const ;
		// ソースメッシュを設定
		void SetSourceMesh( const S2DVector * pvMesh ) ;
		// 出力メッシュを設定
		void SetDestinationMesh( const S2DVector * pvMesh ) ;
		// 描画方式設定
		void SetDynamicDrawMesh( bool fDynamic = true ) ;
		// メッシュの縁を固定する
		void FixMeshBorder( bool fFix = true ) ;

	public:
		// エフェクタ追加
		void AddEffector( Effector * pEffector ) ;

	public:	// SGLSpriteDrawer 実装
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) ;
		// 描画域取得
		virtual bool GetRectangle
			( SGLImageRect& rectDraw, SGLImageObject* image ) const ;

	public:	// SGLSpriteFilter 実装
		// 動的描画フィルタか？
		virtual bool IsDynamicDrawer( void ) const ;
		// フィルタ処理
		virtual void Filter
			( S3DRenderContextInterface& render, SGLImageObject* image ) ;
		// タイマー処理
		virtual void OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;

	protected:
		// フィルタパラメータに応じたメッシュ頂点設定
		virtual void SetFilteredMesh( void ) ;
		// メッシュの縁を描画元座標に一致させる
		void CopyMeshBorderFromSource( void ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// メッシュハイライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMeshHighlight	: public SGLSprite
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMeshHighlight, SGLSprite )
		// 構築関数
		SGLSpriteMeshHighlight( void ) ;
		SGLSpriteMeshHighlight( const SGLSpriteMeshHighlight& smh ) ;
		// 消滅関数
		virtual ~SGLSpriteMeshHighlight( void ) ;

	protected:
		S3DMeshShaper				m_meshShaper ;
		S3DVertexBuffer				m_vboBuffer ;
		SSystem::SArray<S3DVector4>	m_bufMesh ;
		SGLSize						m_sizeInPixels ;
		SGLSize						m_sizeInMesh ;
		S3DVector					m_vLight ;
		S3DMaterial					m_material ;

	public:
		// メッシュサイズ設定
		void SetMeshSize( size_t width, size_t height ) ;
		// 画像サイズ設定
		void SetCanvasSize( size_t width, size_t height ) ;
		// メッシュサイズ取得
		const SGLSize& GetMeshSize( void ) const ;
		// 画像サイズ取得
		const SGLSize& GetCanvasSize( void ) const ;
		// 出力メッシュを設定
		void SetMesh( const S3DVector4 * pvMesh ) ;
		// 光源ベクトル設定
		void SetLight( const S3DVector& vLight ) ;

	protected:
		// 描画前処理
		virtual void BeforeDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 波紋ハイライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteWaveHighlight	: public SGLSpriteMeshHighlight
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteWaveHighlight, SGLSprite )
		// 構築関数
		SGLSpriteWaveHighlight( void ) ;
		SGLSpriteWaveHighlight( const SGLSpriteWaveHighlight& swh ) ;
		// 消滅関数
		virtual ~SGLSpriteWaveHighlight( void ) ;

	public:
		// メッシュ・エフェクタ
		class	Effector	: public SGLObject
		{
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Effector, SGLObject )
			// 構築関数
			Effector( void ) ;
			Effector( const Effector& eff ) ;
			// 初期設定
			virtual void OnAttachedMesh
					( size_t wMesh, size_t hMesh,
							size_t wCanvas, size_t hCanvas ) ;
			// 時間経過
			virtual bool OnTimer
				( SGLSpriteWaveHighlight& swh, uint32_t msecPast ) ;
			// メッシュ頂点変位加算
			virtual bool WarpMesh
				( SGLSpriteWaveHighlight& swh,
					S3DVector4 * pvMesh, size_t wMesh, size_t hMesh ) ;
		} ;
		class	WaveEffector	: public Effector
		{
		protected:
			S2DDVector		m_vCenter ;
			double			m_fpPhase ;			// 位相 [sec]
			double			m_fpAmplitude ;		// 振幅
			double			m_fpCycle ;			// 周期 [sec]
			double			m_fpSpeed ;			// 速度 [pixel/sec]

			size_t			m_nWrapCount ;		// 矩形でラップアラウンド
			SGLImageRect	m_rectWrap ;		// ラップ矩形
			double			m_fpReach ;			// 減衰距離

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( WaveEffector, Effector )
			// 構築関数
			WaveEffector( void ) ;
			WaveEffector( const WaveEffector& eff ) ;
			// パラメータ設定
			void SetParameter
				( const S2DDVector& center,
					double amp, double cycle, double speed ) ;
			// ラップアラウンド設定
			void SetWrapRect
				( const SGLImageRect& rect,
					double fpReach, size_t nWrapCount = 1 ) ;
			// 初期設定
			virtual void OnAttachedMesh
					( size_t wMesh, size_t hMesh,
							size_t wCanvas, size_t hCanvas ) ;
			// 時間経過
			virtual bool OnTimer
				( SGLSpriteWaveHighlight& swh, uint32_t msecPast ) ;
			// メッシュ頂点変位加算
			virtual bool WarpMesh
				( SGLSpriteWaveHighlight& swh,
					S3DVector4 * pvMesh, size_t wMesh, size_t hMesh ) ;
			// 複製
			virtual SGLObject * DuplicateObject( void ) ;
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

	protected:
		SSystem::SObjectArray<Effector>	m_effectors ;

	public:
		// エフェクタ追加
		void AddEffector( Effector * pEffector ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	protected:
		// 描画前処理
		virtual void BeforeDraw( Stereo3DView s3dView = s3dMonoview ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;

}

#endif

