
#if	!defined(__SAKURAGL_RENDER_SOFTWARE_RENDERER_H__)
#define	__SAKURAGL_RENDER_SOFTWARE_RENDERER_H__

#include <sakura/ssys_stack_buffer.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuragl/sgl3d/sgl_render_software_shader.h>
#include <sakuragl/sgl3d/sgl_render_buffered_context.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ソフトウェア・レンダラ・デバイス
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoftwareRenderDevice	: public S3DRenderDevice
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoftwareRenderDevice, S3DRenderDevice )
		// 構築関数
		SGLSoftwareRenderDevice( void ) ;
		// 消滅関数
		virtual ~SGLSoftwareRenderDevice( void ) ;

	public:
		// レンダリングスレッドか判定
		virtual bool IsOnRenderThread( void ) ;
		// レンダリングスレッドで実行する
		virtual SGLError Procedure
			( SSystem::SProcedure* pProc, ProcedurePriority priority ) ;
		// レンダリングスレッドでの遅延実行が全て完了するまで待機
		virtual SGLError WaitUntilAsyncAllProcedures
					( int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
		// レンダラ生成
		virtual S3DRenderContextInterface * NewRenderer( void ) const ;
		// レンダリングデバイス用の画像インスタンスを生成
		virtual SGLError CommitDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の VBO インスタンスを生成／更新
		virtual SGLError CommitDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の画像インスタンスを解放
		virtual SGLError ReleaseDeviceImage
			( SGLImageObject * pImage, int64_t msecTimeout = 0 ) ;
		// レンダリングデバイス用の VBO インスタンスを解放
		virtual SGLError ReleaseDeviceVertexBuffer
			( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout = 0 ) ;

	public:
		// デバイス機能
		virtual SGLError GetDeviceFeatures( Features& features ) ;

	public:
		// カスタムシェーダー・オブジェクト生成
		virtual S3DCustomShader * NewCustomShader( ShaderProgramType type ) ;
		// 定義済み標準シェーダー生成／コンパイル／取得
		virtual S3DCustomShader *
				GetDefaultShaderProgramAs( const wchar_t * pwszID ) ;
		// カスタムシェーダーコンパイル
		virtual SGLError CompileCustomShader
				( S3DCustomShader * pShader,
					const ShaderSourceInfo& src,
					S3DCustomShader::CompileListener * pListener = NULL ) ;
		// カスタムシェーダー読み込み
		virtual SGLError LoadCustomShader
				( S3DCustomShader * pShader,
					const S3DShaderBinary& bin,
					S3DCustomShader::CompileListener * pListener = NULL ) ;
		// カスタムシェーダー・バイナリの保存
		virtual SGLError SaveCustomShaderBinary
			( S3DShaderBinary& bin, S3DCustomShader * pShader ) ;

	protected:
		// SGLSoftwareRenderDevice チェーン
		SGLSoftwareRenderDevice *			m_pChainNext ;
		static SGLSoftwareRenderDevice *	m_pChainFirst ;

		void AddToChain( void ) ;
		void DetachFromChain( void ) ;

	public:
		// デフォルトの SGLSoftwareRenderDevice を取得する
		static SGLSoftwareRenderDevice * GetDefault( void ) ;

	public:
		// パフォーマンスログ・フレーム開始
		virtual void BeginFramePerformanceLog( void ) ;
		// パフォーマンスログ・フレーム終了
		virtual void EndFramePerformanceLog( void ) ;
		// パフォーマンスログ・デバッグ出力
		virtual void DebugTracePerformanceLog( PerformanceLogInfo * pli ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ソフトウェア・レンダラ
	//////////////////////////////////////////////////////////////////////////

	class	S3DSoftwareRenderer
				: public S3DRenderParameterContext, public SGLPaintBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DSoftwareRenderer, S3DRenderParameterContext, SGLPaintBuffer )
		// 構築関数
		S3DSoftwareRenderer( void ) ;
		// 消滅関数
		virtual ~S3DSoftwareRenderer( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) ;
		virtual SGLImageObject * GetTargetZBuffer( void ) ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError CurrentAffine( SGLAffine & af ) ;
		virtual unsigned int CurrentTransparency( void ) ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;
		virtual int64_t GetPaintFlags( void ) ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// ２Ｄメッシュ描画
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;


	public:	// S3DRenderBufferInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// ポリゴンリストをレンダリングバッファに追加
		virtual SGLError AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		virtual SGLError AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DVertexBufferInterface * pBuffer,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
		// 光源を設定
		virtual void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		virtual void SetShadowMap
			( uint32_t idLight,
				SGLImageObject* pShadowMapDepth,
				const S3DShadowMapInfo& infShadowMap,
				SGLImageObject* pShadowMapColor = NULL ) ;
		// 疑似フォッグを設定
		virtual void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		virtual void EnableFog( bool fFog ) ;
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;

	public:
		#if	!defined(__COTOPHA__)
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
		#endif

	public:
		// 光源更新（カメラ変換反映）
		void UpdateLightEntries( void ) ;
		// 非同期レンダリングの同期
		void SyncRender( void ) ;
		// 実行中のスレッドを終了
		void ExitRenderThread( void ) ;
		// レンダリング中のメッシュをすべてスケジューリングし終えるまでレンダリング処理
		void DoRender( void ) ;

	protected:
		class	PolygonBuffer ;
		class	RasterizeBuffer ;

		typedef void (*FUNC_RASTER_SAMPLING)
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		typedef void (*FUNC_RASTER_FILL)( RasterizeBuffer * prb ) ;

		// 関数ポインタセット
		struct	RasterFunctionSet
		{
			FUNC_RASTER_SAMPLING	pfnSampling ;
			FUNC_RASTER_SAMPLING	pfnPreTransparency ;
			FUNC_RASTER_SAMPLING	pfnPreVertexColor ;
			FUNC_RASTER_SAMPLING	pfnTexture ;
			FUNC_RASTER_FILL		pfnWriteFill ;
		} ;

		class	PolygonBuffer
		{
		public:
			bool				m_flagReady ;	// 準備済み

			// 関数ポインタ
			RasterFunctionSet	m_rfs ;

			// 描画属性パラメータ
			uint32_t			m_nAlpha ;		// 描画不透明度 [0,0x100]

			// テクスチャ
			SGLImageBuffer		m_imgTexture ;

			// リージョンバッファ
			SGLRegion *					m_pbufRegion ;
			SSystem::SArray<uint8_t>	m_bufRegion ;

			// テクスチャ原点Ｏ
			// テクスチャｘ基底＝Ａ、ｙ基底＝Ｂ
			// 法線ベクトルＮ＝Ａ×Ｂ
			// スクリーン座標Ｓ＝(sx,sy,r)と
			// ポリゴン平面との交点Ｘ＝((Ｏ・Ｎ)／(Ｓ・Ｎ))Ｓ
			// Ｐ＝Ｘ－Ｏ とすると
			// u・|Ｎ|＝(Ｎ・(Ｐ×Ｂ))
			//        ＝Ｐ・(Ｂ×Ｎ)＝Ｘ・(Ｂ×Ｎ)－Ｏ・(Ｂ×Ｎ)
			// v・|Ｎ|＝(Ｎ・(Ａ×Ｐ))
			//        ＝Ｐ・(Ｎ×Ａ)＝Ｘ・(Ｎ×Ａ)－Ｏ・(Ｎ×Ａ)
			S3DVector		m_vScreen ;			// 投影スクリーン中心座標
			S3DVector		m_vTextureO ;		// テクスチャ原点
			S3DVector		m_vTextureX ;		// テクスチャｘ基底
			S3DVector		m_vTextureY ;		// テクスチャｙ基底
			S3DVector		m_vNormal ;			// 平面法線（正規化済）
			float32_t		m_fpTexO_Normal ;	// m_vTextureO・m_vNormal
			float32_t		m_fpSpt_Normal ;	// 次の行の画面左端・m_vNormal
			S3DVector		m_vDeltaTexX ;		// m_vTextureY×m_vNormal
			S3DVector		m_vDeltaTexY ;		// m_vNormal×m_vTextureX
			float32_t		m_fpBaseTexX ;		// - m_vTextureO・m_vDeltaTexX
			float32_t		m_fpBaseTexY ;		// - m_vTextureO・m_vDeltaTexY

			// 次のスキャンライン（ｙ座標）
			atomic_int_t	m_yScanLine ;
			uint8_t *		m_pDstColor ;
			uint8_t *		m_pDstZBuf ;

			// 処理中参照カウンタ
			volatile atomic_int_t	m_countRef ;

			// スピンロック用フラグ
			volatile atomic_int_t	m_flagSpinLock ;

			// 構築関数
			PolygonBuffer( void )
			{
				m_flagReady = false ;
				m_imgTexture.ptrBuffer = NULL ;
				m_pbufRegion = NULL ;
				m_yScanLine = 0 ;
				m_pDstColor = NULL ;
				m_pDstZBuf = NULL ;
				m_countRef = 0 ;
				m_flagSpinLock = 0 ;
			}
		} ;

		class	RasterizeBuffer
		{
		public:
			SSystem::SArray<uint32_t>	m_bufTempSrc ;
			SSystem::SArray<S3DColor>	m_bufTempColor ;
			SSystem::SArray<float32_t>	m_bufTempZ ;
			uint32_t *					m_pTempSrc ;
			S3DColor *					m_pTempColor ;
			float32_t *					m_pTempZ ;
			uint32_t *					m_pDstColor ;
			uint32_t *					m_pDstZBuf ;

			SGLRegionLine	m_rglLine ;			// 行情報
			size_t			m_nWidth ;			// 行のピクセル幅
			S3DVector		m_vPosScreen ;		// m_fpTexO_Normal・画面座標
			float32_t		m_fpSpt_Normal ;	// 行の左端・m_vNormal

			bool						m_flagThreading ;
			SSystem::SSignalEvent		m_sigEndThread ;
			S3DSoftwareRenderer *		m_pRenderer ;
			SSystem::SThread::IdType	m_tidMain ;
			size_t						m_iThread ;

			// 構築関数
			RasterizeBuffer( void )
			{
				m_pTempSrc = NULL ;
				m_pTempColor = NULL ;
				m_pTempZ = NULL ;
				m_pDstColor = NULL ;
				m_pDstZBuf = NULL ;
				//
				m_flagThreading = false ;
				m_sigEndThread.Initialize( false ) ;
				m_pRenderer = NULL ;
			}
		} ;

		struct	MeshEntry
		{
			MeshEntry *		pChainNext ;
			size_t			countPolygon ;
			S3DMaterial *	pMaterial ;
			uint32_t		nAlpha ;
			S3DVector4 *	pvVertex ;
			S3DVector4 *	pvNormal ;
			S2DVector *		pvUVMap ;
			S3DColor *		pColor ;
			uint32_t *		pIndexedList ;
		} ;

		struct	S3DWColor
		{
			int16_t	rgbMul[3] ;
			int16_t	rgbAdd[3] ;
		} ;

	protected:
		// 状態フラグ
		bool						m_flagBegin3D ;

		// ソフトウェアシェーダー
		S3DRenderingShader			m_shader ;

		// レンダリングキュー
		SSystem::SCriticalSection	m_csQueue ;		// メッシュキュー同期用
		SSystem::SSignalEvent		m_sigQueue ;	// メッシュキューが空でない
		SSystem::SStackBuffer		m_bufRender ;	// 作業バッファ
		SSystem::SArray<S3DLightEntry>
									m_lstLightEntries ;
		S3DTemporaryNormalBuffer	m_tnbNormal ;
		S3DTemporaryIndexTriangleStrip
									m_titsIndex ;
		MeshEntry *					m_pFirstMesh ;	// 待ち行列先頭
		MeshEntry *					m_pLastMesh ;	// 待ち行列終端
		atomic_int_t				m_nQueueMesh ;	// 待ちメッシュ個数

		MeshEntry *			m_pRenderingMesh ;	// レンダリング中メッシュ
		PolygonBuffer		m_bufPolygon[2] ;	// ポリゴンラスタライズ用バッファ
		size_t				m_iPolygon ;		// レンダリング中 m_bufPolygon 指標
		size_t				m_iNextPolygon ;	// ポリゴン指標（m_pRenderingMeshメッシュ内）

		// 現在のメッシュ描画用
		RasterFunctionSet	m_rfsFuncSet ;		// 関数ポインタ
		uint32_t			m_nMeshAlpha ;		// 描画不透明度 [0,0x100]
		uint64_t			m_flagsShading ;
		SGLImageBuffer		m_imgTexture ;
		SGLImageObject *	m_pLastTexture ;

		// スレッド用
		SSystem::SObjectArray<RasterizeBuffer>
								m_arrRasterizeBuf ;
		bool					m_flagAbortThread ;
		atomic_int_t			m_nRunningThreads ;
		SSystem::SSignalEvent	m_sigEndThread ;


		// メッシュ描画パラメータ準備
		bool SetupRenderMesh( void ) ;
		// ポリゴン描画パラメータ準備
		bool SetupPolygonBuffer
			( PolygonBuffer * ppb, MeshEntry * pMesh, size_t iPolygon ) ;
		// ポリゴン描画完了待ち
		static void SyncPolygonBuffer( PolygonBuffer& pb ) ;
		// ポリゴン割り当て
		bool AssinMeshPolygon( void ) ;
		// ラスタライズ割り当て
		bool AssinRasterize
			( PolygonBuffer& pb, RasterizeBuffer& rb ) ;
		// スピンロック
		static void SpinLock( PolygonBuffer& pb ) ;
		static void SpinUnlock( PolygonBuffer& pb ) ;

		// 描画スレッド関数
		static void RenderingThreadProc( void * pInstance ) ;
		void RenderingThread( RasterizeBuffer * prb ) ;


		// サンプリング関数
		static void rasterize_TextureRGB
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_TextureBGR
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_TextureABGR
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_VertexColorAfterTexture
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_VertexColorWithoutTexture
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_PostTransparencyEffect
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;

		// テクスチャサンプリング関数
		static void rasterize_SampleTexture
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_SampleTextureTile
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		static void rasterize_SampleTextureTileSmooth
			( PolygonBuffer * ppb, RasterizeBuffer * prb ) ;
		// 頂点色補完
		static void rasterize_ComplementVertexColor( RasterizeBuffer * prb ) ;
		// 色効果処理
		static void rasterize_EffectVertexColor
			( uint32_t * prgbaDst, const uint32_t * prgbaSrc,
						const S3DColor * pSrcColors, size_t nCount ) ;
		static void rasterize_ConvertVertexColor
			( uint32_t * prgbaDst,
					const S3DColor * pSrcColors, size_t nCount ) ;
		// 書き出し
		static void rasterize_WriteSimple( RasterizeBuffer * prb ) ;
		static void rasterize_WriteCompareZ( RasterizeBuffer * prb ) ;
		static void rasterize_WriteWithZ( RasterizeBuffer * prb ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ソフトウェア・レンダラ（バッファリング：ソート／ステレオ立体視対応）
	//////////////////////////////////////////////////////////////////////////

	class	S3DSoftwareBufferedRenderer : public S3DRenderBufferedContext
	{
	protected:
		S3DSoftwareRenderer	m_render ;
		bool				m_flagDelayUpdate ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( S3DSoftwareBufferedRenderer, S3DRenderBufferedContext )
		// 構築関数
		S3DSoftwareBufferedRenderer( void ) ;
		// 消滅関数
		virtual ~S3DSoftwareBufferedRenderer( void ) ;
		// S3DSoftwareRenderer 取得
		S3DSoftwareRenderer & GetDirectRenderer( void )
		{
			return	m_render ;
		}
		// 現在の2D描画変換をS3DSoftwareRendererに反映
		void Reflect2DTransformation( void ) ;

	public:
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// ２Ｄメッシュ描画
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;

	public:	// S3DRenderBufferInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
		// 立体視視差設定
		virtual void SetParallax
			( double xParallax, double zFocusRate, double xScreenDelta ) ;
		// ｚクリップ範囲を設定
		virtual void SetZClipRange( double zMin, double zMax ) ;
		// 光源を設定
		virtual void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		virtual void SetShadowMap
			( uint32_t idLight,
				SGLImageObject* pShadowMapDepth,
				const S3DShadowMapInfo& infShadowMap,
				SGLImageObject* pShadowMapColor = NULL ) ;
		// 疑似フォッグを設定
		virtual void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		virtual void EnableFog( bool fFog ) ;
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
		// レイトレーシング設定
		virtual void SetRayTracingParameter
					( const S3DRenderRayTracingParam& rrtp ) ;
		// グローバル環境マッピング設定
		virtual void SetEnvironmentMappingImage
					( SGLImageObject * pImage, uint32_t nFlags ) ;
		// グローバル環境マッピング変換行列設定
		virtual void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) ;
		// 輪郭描画色設定
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// ポリゴンリストをレンダリングバッファに追加
		virtual SGLError AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		virtual SGLError AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
					S3DVertexBufferInterface * pBuffer,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) ;
		// ハードウェア描画オブジェクト取得
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;

	public:	// SGLDrawContextInterface オーバーライド
		// 点描画
		virtual SGLError DrawPoints
			( const S2DVector * pPoints, size_t nPoints,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 直線描画
		virtual SGLError DrawThinLines
			( const S2DVector * pLines, size_t nLines,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// グラデーション解除
		virtual void FreeGradation( void ) ;
		// 線形グラデーション設定
		virtual SGLError SetLinearGradation
			( float32_t x0, float32_t y0, float32_t x1, float32_t y1,
					const SGLPalette * pGradation, size_t nCount ) ;
		// 環状グラデーション設定
		virtual SGLError SetRingedGradation
			( float32_t xCenter, float32_t yCenter, float32_t radAngle,
				const SGLPalette * pGradation, size_t nCount ) ;

	} ;

}

#endif

