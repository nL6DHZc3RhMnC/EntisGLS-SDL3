
#if	!defined(__SAKURAGL_OPENGL_RENDER_CONTEXT_H__)
#define	__SAKURAGL_OPENGL_RENDER_CONTEXT_H__

#include <sakuragl/sgl3d/sgl_render_buffered_context.h>
#include <sakuragl/sgl_opengl_context.h>
#include <sakuragl/sgl_opengl_custom_shader.h>
#include <sakuragl/sgl3d/sgl_render_software_shader.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// OpenGL レンダリング・コンテキスト（直接／OpenGL スレッド上で利用）
	//////////////////////////////////////////////////////////////////////////

	class	S3DOpenGLDirectlyRenderer	: public S3DRenderParameterContext
	{
	public:
		SGLOpenGLDefaultShader *	m_pglDefShader ;
		SGLOpenGLCustomShader *		m_pglCustomShader ;
		SGLOpenGLRenderingContext	m_glRenderer ;
		SGLOpenGLView				m_glView ;
		SGLOpenGLFrameBuffer		m_glFrameBuffer ;
		S3DRenderingShader			m_shader ;

		// 描画時の上下反転
		enum	VerticalOrder
		{
			verticalAuto,			// 自動
			verticalAutoReverse,	// verticalAuto の上下反転
			verticalStright,		// 反転無し固定
			verticalReverse,		// 反転固定
		} ;

	protected:
		SGLOpenGLTextureBuffer *	m_pglColor ;
		SGLOpenGLTextureBuffer *	m_pglDepth ;
		VerticalOrder				m_vertOrder ;
		bool						m_flagUpdate ;
		bool						m_flag3DPriority ;
		bool						m_flagIn3DRenderer ;
		bool						m_flagDrawImage ;
		bool						m_flagFixViewPort ;
		bool						m_flagProjUpsideDown ;
		SSystem::SArray<S3DVector4>	m_bufVertex ;
		SSystem::SArray<S3DVector4>	m_bufNormal ;
		SSystem::SArray<S3DColor>	m_bufColor ;
		SSystem::SArray<S2DVector>	m_bufUVMap ;
		SSystem::SArray<S2DVector>	m_bufMeshUV ;
	#if	defined(__API_OPEN_GL_ES__)
		SSystem::SArray<uint16_t>	m_bufIndexed ;
	#else
		SSystem::SArray<uint32_t>	m_bufIndexed ;
	#endif
		SSystem::SArray<S3DLightEntry>	m_arrayBufLights ;

		SSystem::SArray<S4DMatrix>	m_bufInstanceMatrix ;
		SSystem::SArray<S3DColor>	m_bufInstanceColor ;
		SSystem::SPointerArray<S3DVertexVariantBuffer>
									m_bufInstanceVVB ;

		bool						m_flagMirrorFBO ;
		GLuint						m_glMirrorFBO ;
		GLuint						m_glMirrorFBOs[2] ;
		SGLImageRect				m_rectMirrorSrcFBO ;
		SGLImageRect				m_rectMirrorDstFBO ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( S3DOpenGLDirectlyRenderer, S3DRenderParameterContext )
		// 構築関数
		S3DOpenGLDirectlyRenderer( void ) ;
		// 3Dレンダリングを優先する（2D描画も透視行列で行う）
		void Have3DPriorityOver2D( bool flag3DPriority ) ;
		// シェーダー設定
		void AttachShader( SGLOpenGLDefaultShader * pglShader ) ;
		// ターゲット更新フラグ
		bool IsUpdatedTarget( void ) const ;
		// カスタムシェーダー設定
		void AttachGLCustomShader( SGLOpenGLCustomShader * pglShader ) ;
		// ビューポート（m_glView）の設定は AttachTargetImage で変更しない
		void SetFixViewport( bool flagFixView ) ;
		// 描画の上下反転設定
		void SetViewVerticalOrder( VerticalOrder vertOrder ) ;
		// ミラーリング FBO 転送設定
		void SetMirrorFrameBuffer
			( GLuint glDstRightFBO, GLuint glDstLeftFBO,
				const SGLImageRect& rectSrc, const SGLImageRect& rectDst ) ;
		void DetachMirrorFrameBuffer( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// マルチターゲット（2つ目以降）描画先設定
		virtual SGLError AttachMultiTargetImages
			( SGLImageObject*const* ppTargets, size_t nCount ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
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

	protected:
		// 頂点の設定をクリアする
		void FlushVertexPointers( void ) ;
		// マテリアル設定をクリアする
		void FlushGLMaterial( void ) ;
		// 描画フラグに応じた OpenGL の設定
		void PutGLByPaintFlags( SGLOpenGLContext * pOpenGL, uint32_t flags ) ;
		// 描画色設定
		void PutGLPaintColor
			( uint32_t argb, unsigned int nTransparency = 0 ) ;
		// 頂点座標正規化
		bool PutVertex2D
			( const S2DVector * vertices,
				size_t count, double z,
				uint32_t flags, const SGLAffine * pAffine = NULL ) ;
		// テクスチャ設定
		bool PutTexture2D
			( SGLAffine& afUV, uint32_t flags,
				SGLImageObject * pSrcImage, const SGLImageRect * pSrcClip ) ;
		// UV座標設定
		void PutTextureUVMap
			( const S2DVector * pvUVMap,
				size_t count, const SGLAffine& afUV ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 透視変換行列取得
		virtual bool GetPerspectiveMatrix( S4DMatrix& matPars ) const ;
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix
			( StereoViewIndex sviView,
				const S4DMatrix& matPers, bool fPersMatrix = true ) ;
		virtual void EnablePerspectiveMatrix( bool fPersMatrix ) ;
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
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
		// グローバル環境マッピング設定
		virtual void SetEnvironmentMappingImage
					( SGLImageObject * pImage, uint32_t nFlags ) ;
		// グローバル環境マッピング変換行列設定
		virtual void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) ;
		// 輪郭描画色設定
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// オプショナル機能設定
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
					const void * pParam2, size_t sizeOfParam2 ) ;
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		static void GetRenderingCapacityWithOpenGL
				( S3DRenderingCapacity& caps, SGLOpenGLContext * pOpenGL ) ;
		// 立体視用バッファ選択
		virtual SGLError SelectParallaxView( StereoViewIndex sviView ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;

	public:
		// 3D レンダリング用フラグ設定
		//（OpenGL スレッド以外からフラグのみを変更する）
		void Set3DRendererFlag( bool flag3D ) ;
		// 3D レンダリング用フラグ取得
		bool IsIn3DRenderer( void ) const ;

	public:
		// シェーダー基本設定スイッチング
		void SwitchShaderContext( void ) ;

	protected:
		// OpenGL の投影行列更新
		void UpdateGLProjection( void ) ;
		// OpenGL の直行投影行列更新
		void UpdateGLOrthogonalProjection( void ) ;
		// OpenGL の透視投影行列更新
		void UpdateGLPerspectiveProjection( void ) ;
		// シェーディングフラグの設定を SGLOpenGLRenderingContext へ反映
		bool ReflectShadingFlags( void ) ;
		// シェーディングフラグを現在のコンテキストに設定
		bool SetShadingFlagsToGLContext( uint64_t nShadingMethod ) ;
		// 光源の設定を反映
		void ReflectLighting( void ) ;
		// 環境マッピングの設定を反映
		void ReflectEnvironmentMapping( void ) ;
		// 輪郭描画の設定を反映
		void ReflectOffsetBorder( void ) ;
		// フレームバッファの全レンダーターゲットを再設定する
		void ReflectRenderTarget( void ) ;
		// フレームバッファのマルチレンダーターゲットを再設定する
		void ReflectMultiRenderTargets( void ) ;

	public:	// S3DRenderBufferInterface オーバーライド
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

	protected:
		// プログラマブルシェーダーをマテリアルに応じて変更する
		void OptimizedMaterialShader( S3DMaterial * pMaterial ) ;
		// 現在のカメラ変換行列を反映
		void PutCameraViewMatrix( void ) ;
		// 現在のモデル変換行列を計算する
		void GetTransform4x4( S4DDMatrix& mat4 ) ;
		// 現在の色や透明度の設定を反映
		void PutCurrentColorEffect( bool flagDisableColorEffect = false ) ;
		// 頂点座標と法線を変換し、現在の色や透明度の設定を反映
		void TransformVertex3D
			( const S3DVector4 *& pvVertex,
				const S3DVector4 *& pvNormal, size_t countVertex ) ;

	public:
		// ハードウェア描画オブジェクト取得
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// OpenGL レンダリング・コンテキスト（バッファリング可能）
	//////////////////////////////////////////////////////////////////////////

	class	S3DOpenGLBufferedRenderer	: public S3DRenderBufferedContext
	{
	protected:
		SSystem::SSmartReference<SGLOpenGLContext>
									m_refOpenGL ;
		S3DOpenGLDirectlyRenderer	m_glRenderer ;
		atomic_int_t				m_countFlush ;

		SSystem::SProcedureQueue	m_queProc ;


		// Flush 処理
		class	FlushProcedure	: public SSystem::SProcedure
		{
		protected:
			S3DOpenGLBufferedRenderer *	m_render ;
			bool						m_finish ;
			bool						m_flagDelete ;
		public:
			// 構築関数
			FlushProcedure
				( S3DOpenGLBufferedRenderer * render, bool finish, bool flagDelete )
					: m_render( render ), m_finish( finish ), m_flagDelete( flagDelete ) {}
			// スレッド関数
			virtual void Run( void ) ;
			virtual void Finalize( void ) ;
		} ;

		// CopyBufferFrom 処理
		class	CopyBufferProcedure	: public SSystem::SProcedure
		{
		protected:
			S3DOpenGLBufferedRenderer *	m_poglBufSrc ;
			S3DOpenGLBufferedRenderer *	m_poglBufDst ;
			uint32_t					m_nFlags ;
			int							m_xDst ;
			int							m_yDst ;
			SGLImageRect *				m_pSrcRect ;
			SGLImageRect				m_rectSrc ;
		public:
			// 構築関数
			CopyBufferProcedure
				( S3DOpenGLBufferedRenderer * pSrc,
					S3DOpenGLBufferedRenderer * pDst,
					uint32_t nFlags, int xDst, int yDst,
					const SGLImageRect * pSrcRect ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( S3DOpenGLBufferedRenderer, S3DRenderBufferedContext )
		// 構築関数
		S3DOpenGLBufferedRenderer( SGLOpenGLContext * pOpenGL ) ;
		// 消滅関数
		virtual ~S3DOpenGLBufferedRenderer( void ) ;
		// S3DOpenGLDirectlyRenderer 取得
		S3DOpenGLDirectlyRenderer & GetDirectlyRenderer( void )
		{
			return	m_glRenderer ;
		}
		// 現在のスレッドに関連付けられた OpenGL レンダラか？
		bool IsCurrentOpenGLRenderer( void ) const ;
		// OpenGL コンテキスト関連付け変更
		void AttachOpenGLContext( SGLOpenGLContext * pOpenGL ) ;
		// OpenGL スレッド上／レンダラが設定された状態で任意関数実行
		SGLError ProcedureWithRederer( SSystem::SProcedure * pProc ) ;
		// OpenGL スレッド上／レンダラが設定された状態で任意関数実行
		// （非同期・遅延・Flush/Finish/ForceFlush 等が実行された後のタイミングで）
		SGLError PostProcedureWithRederer
			( SSystem::SProcedure * pProc, bool flagAutoDelete ) ;
		// 描画の実行（OpenGL スレッド上でなくても）
		SGLError ForceFlush( void ) ;
		// ミラーリング FBO 転送設定
		void SetMirrorFrameBuffer
			( GLuint glDstRightFBO, GLuint glDstLeftFBO,
				const SGLImageRect& rectSrc, const SGLImageRect& rectDst ) ;
		void DetachMirrorFrameBuffer( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// マルチターゲット（2つ目以降）描画先設定
		virtual SGLError AttachMultiTargetImages
			( SGLImageObject*const* ppTargets, size_t nCount ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;

	public:	// S3DRenderBufferInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 透視変換行列取得
		virtual bool GetPerspectiveMatrix( S4DMatrix& matPars ) const ;
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
		// オプショナル機能設定
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
					const void * pParam2, size_t sizeOfParam2 ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;
		// 非同期レンダリングに適したスレッドで実行
		virtual void SuitableProcedure
				( PROCEDURE_RENDERING pfnRendering, void * pInstance ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		// ハードウェア描画オブジェクト取得
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
		// ハードウェア描画オブジェクト変更
		virtual SGLError SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags = 0 ) ;

	protected:
		// Flush 関数処理
		virtual void OnGLThreadFlush( bool flagFinish ) ;
		// OpenGL の現在の RenderContext を切り替える
		void SwitchRenderContext( SGLOpenGLContext * pOpenGL ) ;

		friend class	FlushProcedure ;
	} ;

}

#endif

