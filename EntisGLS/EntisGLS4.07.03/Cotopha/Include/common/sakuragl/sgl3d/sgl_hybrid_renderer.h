
#if	!defined(__SAKURAGLX_HYBRID_RENDERER_H__)
#define	__SAKURAGLX_HYBRID_RENDERER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ハイブリッド・レンダラ
	//////////////////////////////////////////////////////////////////////////

	class	S3DHybridRenderContext	: public S3DRenderContext
	{
	protected:
		// フラグ
		bool	m_mode3d ;
		bool	m_modeRenderOnly ;
		bool	m_flagViewPort ;

		// 描画オブジェクト
		SGLPaintContextInterface *	m_paint ;

		// 描画対象領域
		SGLImageRect	m_rectViewPort ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SakuraGL::S3DHybridRenderContext, S3DRenderContext )
		// 構築関数
		S3DHybridRenderContext
			( SGLPaintContextType type = typePaintDefault ) ;
		S3DHybridRenderContext
			( RenderContext * render, SGLPaintContextInterface * paint ) ;
		// 消滅関数
		virtual ~S3DHybridRenderContext( void ) ;
		// PaintContext 取得
		PaintContext * GetPaintContext( void ) const ;
		operator PaintContext * ( void ) const ;

	public:	// SGLPaintContextInterface 実装
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) ;
		virtual SGLImageObject * GetTargetZBuffer( void ) ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
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
						uint32_t argb, double z, uint32_t flags ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
						uint32_t argb, double z, uint32_t flags ) ;
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
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;

	public:	// S3DRenderContextInterface 実装
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;

	public:
		#if	defined(__COTOPHA__)
		// PaintContext 取得
		virtual PaintContext * GetPaintContextObject( void ) const ;
		#endif
	} ;


}

#endif
