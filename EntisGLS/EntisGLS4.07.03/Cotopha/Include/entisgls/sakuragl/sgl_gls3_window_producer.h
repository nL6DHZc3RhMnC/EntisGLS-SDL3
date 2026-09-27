
#if	!defined(__SAKURAGL_ENTISGLS3_WINDOW_PRODUCER_H__)
#define	__SAKURAGL_ENTISGLS3_WINDOW_PRODUCER_H__	1

#include <sakuragl/sgl_generic_window.h>
#include <sakuragl/window/sgl_window_producer.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// SGLImageBufferInterface の EGL_IMAGE_INFO 実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageE3DTexture	: public SGLImageBufferInterface
	{
	public:
		PEGL_IMAGE_INFO	m_pTexture ;
		bool			m_flagUpdate ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageE3DTexture, SGLImageBufferInterface )
		// 構築関数
		SGLImageE3DTexture( void ) ;
		// 消滅関数
		virtual ~SGLImageE3DTexture( void ) ;
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;
		// テクスチャ生成
		void CreateTexture( SGLImageBuffer * pImageBuf ) ;

	public:
		// SGLImageInfo から EGL_IMAGE_INFO へ変換
		static void SGLImageInfo2EGL
			( EGL_IMAGE_INFO& eglInf, const SGLImageInfo& sglInf ) ;
		// SGLImageBuffer から EGL_IMAGE_INFO へ変換
		static void SGLImageBuffer2EGL
			( EGL_IMAGE_INFO& eglInf, const SGLImageBuffer& sglBuf ) ;
		// SGLImageObject から PEGL_IMAGE_INFO テクスチャ取得
		static PEGL_IMAGE_INFO CommitE3DTexture( SGLImageObject * pImage ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// S3DMaterialBuffer の E3D_SURFACE_ATTRIBUTE 実装
	//////////////////////////////////////////////////////////////////////////

	class	S3DMaterialE3DSurfaceAttribute : public S3DMaterialBuffer
	{
	public:
		bool					m_flagBack ;
		E3D_SURFACE_ATTRIBUTE	m_sufAttr ;
		E3D_SURFACE_ATTRIBUTE	m_sufBackAttr ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMaterialE3DSurfaceAttribute, S3DMaterialBuffer )
		// 構築関数
		S3DMaterialE3DSurfaceAttribute( void ) ;
		// 情報変換
		virtual void UpdateMaterial( const S3DMaterial * pMaterial ) ;

	public:
		// S3DSurfaceAttribute から E3D_SURFACE_ATTRIBUTE へ変換
		static void SGLSurfaceAttribute2EGL
			( E3D_SURFACE_ATTRIBUTE& eglAttr,
					const S3DSurfaceAttribute& sglAttr ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// HEGL_RENDER_POLYGON S3DRenderContextInterface 変換
	//////////////////////////////////////////////////////////////////////////

	class	SGLRenderPolygonInterface : public S3DRenderParameterContext
	{
	protected:
		E3DRenderPolygon	m_render ;
		HEGL_RENDER_POLYGON	m_hRender ;
		HEGL_DRAW_IMAGE		m_hDraw ;

		EGL_IMAGE_INFO *	m_pinfTarget ;
		EGL_IMAGE_INFO *	m_pinfZBuf ;
		EGL_RECT *			m_pDstViewRect ;
		E3D_VECTOR			m_vScreenPos ;
		EGL_IMAGE_INFO		m_infTarget ;
		EGL_IMAGE_INFO		m_infZBuffer ;
		EGL_RECT			m_rctDstView ;

		bool				m_flagUpdateTarget ;
		bool				m_flagUpdateZBuf ;
		bool				m_flagAnyPaint ;
		bool				m_flagAnyPaintZ ;
		bool				m_flagQueuePolygon ;

		SSystem::SArray<S2DVector>	m_bufPolygonVertices ;
		SSystem::SArray<EGL_POINT>	m_bufPolygonPoints ;

		SSystem::SArray<E3D_LIGHT_ENTRY>			m_bufLightEntries ;
		SSystem::SObjectArray<E3D_SHADOW_MAP_INFO>	m_bufShadowMap ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLRenderPolygonInterface, S3DRenderParameterContext )
		// 構築関数
		SGLRenderPolygonInterface( HEGL_RENDER_POLYGON render ) ;
		// 消滅関数
		virtual ~SGLRenderPolygonInterface( void ) ;
		// 関連付け
		void AttachRenderPolygon( HEGL_RENDER_POLYGON render ) ;
		// 描画履歴取得
		bool IsAnyPaint( void ) const
		{
			return	m_flagAnyPaint ;
		}
		bool IsAnyPaintZBuffer( void ) const
		{
			return	m_flagAnyPaintZ ;
		}

	public:
		// 描画機能フラグを更新
		void ReflectFunctionFlags( void ) ;
		// 光源設定を反映
		void ReflectLightEntries( void ) ;
		// シャドウマッピングを設定
		void ReflectShadowMapInfo
			( E3D_LIGHT_ENTRY& eglLight, const S3DLightEntry& light ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
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
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;
		// 非同期レンダリング開始
		virtual SGLError AsyncFlush
			( uint32_t nFlags, SSystem::SSignalEvent * pSignal ) ;
		// 非同期レンダリング完了待機
		virtual SGLError WaitFlush( int64_t msecTimeout ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// ｚクリップ範囲を設定
		virtual void SetZClipRange( double zMin, double zMax ) ;
		// 光源を設定
		virtual void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		virtual void SetShadowMap
			( uint32_t idLight,
				SGLImageObject* pShadowMap,
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
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;

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
		// 座標要素が未確定の E3D_PRIMITIVE_POLYGON を追加する
		SGLError AddPolygonMeshPrimitive
			( E3D_PRIMITIVE_POLYGON * pppMesh,
				bool fBackSurface, uint32_t nFlags,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS3 レンダラ・ウィンドウ表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLEntisGLS3WindowProducer	: public SGLWindowViewProducer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLEntisGLS3WindowProducer, SGLWindowViewProducer )
		// 構築関数
		SGLEntisGLS3WindowProducer( void ) ;
		// 消滅関数
		virtual ~SGLEntisGLS3WindowProducer( void ) ;

	protected:
		SSystem::SSmartReference<SGLAbstractWindow>	m_refWindow ;
		HWND						m_hWnd ;

		class	StandardRenderer : public SGLRenderPolygonInterface
		{
		public:
			// 構築関数
			StandardRenderer( void )
				: SGLRenderPolygonInterface( ::eglCreateRenderPolygon() )
			{
			}
			// 消滅関数
			virtual ~StandardRenderer( void )
			{
				if ( m_hRender != NULL )
				{
					m_hRender->Release() ;
					m_hRender = NULL ;
				}
			}
		} ;

		StandardRenderer	m_render ;
		SGLPaintBuffer		m_paint ;
		bool				m_flagZBuffer ;
		bool				m_flagLastPhysZPaint ;
		SGLSize				m_sizeVirtual ;
		SGLSize				m_sizePhysical ;
		SGLImageRect		m_rectViewPort ;
		SGLImage			m_imgVirtScreen ;
		SGLImage			m_imgVirtZBuf ;
		SGLImage			m_imgPhysScreen ;
		SGLImage			m_imgPhysZBuf ;

	protected:
		// 論理ビューポート取得
		void GetVirtualViewPort( SGLImageRect& rectView ) ;

	public:	// SGLWindowViewProducer 実装
		// 対応機能フラグ
		virtual uint64_t GetCapacityFlags( void ) const ;
		// 論理ビューサイズ通知
		virtual void OnChangeVirtualViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 論理ビュー表示座標取得
		virtual void GetInternalViewPosition( SGLImageRect& rctVirtualView ) ;
		// 物理ビュー表示領域取得
		virtual bool GetExternalViewPosition( SGLImageRect& rctPhysicalView ) ;
		// 論理座標→物理ビュー座標変換行列取得
		virtual void GetAffineVirtualToPhysical( SGLAffine& affine ) ;
		// 描画スレッドの関連付け
		virtual SGLError AttachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画スレッドの関連付け解除
		virtual SGLError DetachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 直接描画ハンドラ開始
		virtual RenderContext * BeginDirectView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 直接描画ハンドラ終了
		virtual void EndDirectView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer
			( SGLAbstractWindow * pWnd, bool flagZBuffer ) ;
		// レイヤードウィンドウ設定
		virtual SGLError EnableLayeredWindow
			( SGLAbstractWindow * pWnd, bool flagLayeredWindow ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
		// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
		virtual SGLSize GetStandardDisplaySize( void ) const ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS3 描画ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	class	SGLGenericWindowGLS3View	: public SGLGenericWindow
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLGenericWindowGLS3View, SGLGenericWindow )
		// 構築関数
		SGLGenericWindowGLS3View
			( SSystem::SEnvironmentInterface * env = NULL ) ;

	} ;

}


#endif

