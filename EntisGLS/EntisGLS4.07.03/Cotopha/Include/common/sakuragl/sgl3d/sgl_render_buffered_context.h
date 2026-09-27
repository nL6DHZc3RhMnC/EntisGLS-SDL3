
#if	!defined(__SAKURAGL_RENDER_BUFFERED_CONTEXT_H__)
#define	__SAKURAGL_RENDER_BUFFERED_CONTEXT_H__

#include <sakuragl/sgl3d/sgl_render_parameter_context.h>
#include <sakuragl/sgl3d/sgl_render_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 3D レンダリング・バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderBufferedContext
				: public S3DRenderParameterContext,
					public SGLDrawContextInterface
	{
	protected:
		class	RenderBuffer
		{
		public:
			S3DRenderBuffer *	m_render ;
			bool				m_flagFillClear ;
			uint32_t			m_argbFillClear ;
			int64_t				m_flagsFillClear ;
		public:
			RenderBuffer( void )
				: m_render(NULL), m_flagFillClear(false) {}
			~RenderBuffer( void )
			{
				delete	m_render ;
				m_render = NULL ;
			}
		} ;
		enum	RenderBufferViewIndex
		{
			indexAutoView,
			indexRightView,
			indexLeftView,
			countView,
		} ;
		RenderBuffer					m_rbRenderBuffer[countView] ;
		RenderBuffer *					m_prbBuffer ;
		S3DRenderBuffer *				m_pRender ;

		SSystem::SArray<S3DVector4>		m_bufVertex ;
		SSystem::SArray<S2DVector>		m_bufUVMap ;
		SSystem::SArray<S3DColor>		m_bufColor ;
		SSystem::SArray<uint32_t>		m_bufIndexedList ;
		SSystem::SArray<SGLImageRect>	m_bufImageRect ;
		S3DMaterial						m_material2DPolygon ;
		S3DMaterial						m_material2DPolygonNZ ;
		S3DMaterial						m_material2DPolygonNWZ ;

		// 塗りつぶしタイプ (SGLDrawContextInterface)
		enum	FillOperation
		{
			fillFlat,
			fillLinearGradation,
			fillRingedGradation,
			fillTypeCount,
		} ;
		FillOperation	m_opFillType ;
		SSystem::SArray<SGLPalette>
						m_aGradation ;			// グラデーション色
		S2DVector		m_vGradationCenter ;	// グラデーション基準点
		S2DVector		m_vGradationDelta ;		// グラデーション基底（*色数/長さ）
		float32_t		m_radGradation ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SakuraGL::S3DRenderBufferedContext,
				S3DRenderParameterContext, SGLDrawContextInterface )
		// 構築関数
		S3DRenderBufferedContext( void ) ;
		// 消滅関数
		virtual ~S3DRenderBufferedContext( void ) ;

	public:
		// 全ビューのバッファをクリア
		void ClearAllViewBuffer( void ) ;
		// 全ビューのバッファをレンダリング
		SGLError FlushRenderAllViewBufferTo
			( S3DRenderContextInterface * render,
								uint64_t flagsExclusion = 0 ) ;
		// インデックス変換
		static ssize_t IndexOfStereoView( StereoViewIndex sviView ) ;
		// S3DRenderBuffer 取得
		S3DRenderBuffer * GetRenderBuffer( StereoViewIndex sviView ) ;
		// S3DRenderBuffer 準備
		void SetRenderBuffer
			( S3DRenderBuffer * pAutoView = NULL,
				S3DRenderBuffer * pRightView = NULL,
				S3DRenderBuffer * pLeftView = NULL ) ;
		// 描画の更新が存在するか？
		bool IsEmptyRenderBuffer( void ) const ;

	protected:
		// S3DRenderBuffer 生成
		virtual S3DRenderBuffer * NewRenderBuffer( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
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
		// 複数画像描画
		virtual SGLError DrawMultiImages
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips = NULL ) ;

	protected:
		// 画像をテクスチャに持つシェーディング無しの表面属性を取得する
		static S3DMaterial * GetNoShadeMaterialOf
				( bool& flagCombinable,
					SGLImageObject * pSrcImage,
					uint32_t flagsPaint, uint64_t flagsShading = 0 ) ;
		// 描画パラメータから 2D 描画用座標変換と色効果・透明度を設定する
		void PrepareTransfomationToDrawImage
			( S3DRenderBuffer * pRender, const SGLPaintParam & ppPaint ) ;
		// 2D 画像描画の際の頂点座標を準備する
		S3DVector4 * PrepareVertexToDrawImage
			( const S2DVector * pvUVMap, size_t countVertex ) ;

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

	protected:
		// グラデーションを頂点に反映
		void GradationVertexColor
			( S3DColor * pColor,
				const S2DVector * pPoints, size_t nCount,
				uint32_t argb, uint32_t flags ) const ;

	protected:
		S3DRenderBuffer::MeshBuffer	m_mbufDelay ;
		SSystem::SCriticalSection	m_csDelayBuf ;
		uint32_t					m_nDelayFlags ;

	public:
		// 遅延描画バッファ
		SGLError DelayIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 遅延バッファフラッシュ
		SGLError FlushDelayDraw( void ) ;

	public:	// S3DRenderBufferInterface オーバーライド
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
		// 輪郭描画色設定
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// オプショナル機能設定
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						const void * pParam2, size_t sizeOfParam2 ) ;
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const ;
		// 描画座標空間設定
		virtual SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		virtual SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		virtual SGLError ResetCustomShaderUniform( void ) ;
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
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
		// 立体視用バッファ選択
		virtual SGLError SelectParallaxView( StereoViewIndex sviView ) ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) ;

		friend class SGLDrawImageParamList ;
	} ;

}

#endif

