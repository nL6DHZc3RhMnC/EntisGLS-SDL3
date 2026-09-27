
#if	!defined(__SAKURAGL_RENDER_PARAMETER_CONTEXT_H__)
#define	__SAKURAGL_RENDER_PARAMETER_CONTEXT_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 3D レンダリング・コンテキスト基底
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderParameterContext : public S3DRenderContextInterface
	{
	protected:
		// 描画先情報
		SGLImageObject *	m_pTarget ;
		SGLImageObject *	m_pZBuffer ;
		SGLImageRect		m_rctView ;

		bool				m_fStereoView ;
		SGLImageObject *	m_pStereoTarget[2] ;
		SGLImageObject *	m_pStereoZBuffer[2] ;

		SSystem::SPointerArray<SGLImageObject>
							m_aMultiTarget ;

		// 座標変換
		struct	TransformationList
		{
			TransformationList *	pPrev ;
			SGLAffine				afTransform ;
			S3DDMatrix				matTransform ;
			S3DDVector				vTransform ;
			S3DColor				colorEffect ;
			unsigned int			nTransparency ;
			OptionalContextSet		optContext ;	// ※現在の値ではなく、Pop 時に Push 時の値を復元する為

			TransformationList( void )
				: pPrev( NULL ),
					matTransform
						( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 ),
					vTransform( 0.0, 0.0, 0.0 ),
					colorEffect( 0x00FFFFFF, 0 ), nTransparency( 0 )
			{
				DefaultOptionalContext( optContext ) ;
			}
			void Recycle( void )
			{
				pPrev = NULL ;
				#if	defined(__COTOPHA__)
					afTransform = SGLAffine ;
				#else
					afTransform = SGLAffine() ;
				#endif
				matTransform.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
				vTransform = S3DDVector( 0, 0, 0 ) ;
				colorEffect = S3DColor( 0x00FFFFFF, 0 ) ;
				nTransparency = 0 ;
				DefaultOptionalContext( optContext ) ;
			}
		} ;
		TransformationList *	m_pTransformation ;
		TransformationList *	m_pGarbage ;

		// デフォルト描画フラグ
		int64_t		m_flagsDefPaint ;

		// 投影スクリーン情報
		S3DVector	m_vProjectionScreen ;	// 画面中心座標とｚ座標
		double		m_zProjectionScale ;	// 拡大率
		double		m_fpPixelAspectRatio ;	// ピクセルアスペクト比
		float32_t	m_xProjectionScreenOrg ;
		bool		m_flagPersMatrix ;
		S4DMatrix	m_matPerspective[2] ;

		// カメラ
		S3DDMatrix	m_matCamera ;		// カメラによる変換行列
		S3DDVector	m_vCameraPos ;		// カメラの座標
		double		m_xParallax ;		// 視差
		double		m_zParallaxFocus ;
		double		m_xParallaxScreen ;

		// ｚクリップ
		double		m_zMinClip ;
		double		m_zMaxClip ;

		// 光源（座標は無変換）
		SSystem::SArray<S3DLightEntry>	m_arrayVectorLights ;
		SSystem::SArray<S3DLightEntry>	m_arrayPointLights ;
		SSystem::SArray<S3DLightEntry>	m_arrayFogLights ;
		SGLPalette						m_rgbAmbient ;
		SGLPalette						m_rgbAmbientMul ;

		struct	ShadowMapEntry
		{
			uint32_t			idLight ;
			SGLImageObject *	pDepth ;
			SGLImageObject *	pColor ;
			S3DShadowMapInfo	smiMapInfo ;
		} ;
		SSystem::SArray<ShadowMapEntry>	m_arrayShadowMapInf ;

		bool		m_flagFog ;
		SGLPalette	m_rgbFogColor ;
		double		m_zFogNear ;
		double		m_zFogFar ;

		// 描画コンテキスト
		OptionalContextSet			m_optContext ;
		S3DRenderRayTracingParam	m_rrtpRayTracing ;

		// 環境マッピング
		SGLImageObject *			m_pEnvMapping ;
		uint32_t					m_nEnvMappingFlags ;
		S3DMatrix					m_matEnvMapping ;	// 逆変換行列
		SGLImageObject *			m_pEnvViewportDepth ;
		SGLImageObject *			m_pEnvRefraction ;
		uint32_t					m_nEnvRefractionFlags ;
		S3DMatrix					m_matEnvRefraction ;	// 逆変換行列

		// 拡張機能設定
		bool						m_feature_sRGB ;

		// 立体視ビュー
		StereoViewIndex				m_sviView ;

		// 一時オブジェクト
		SSystem::SObjectArray<ESLObject>	m_arrayTemporary ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SakuraGL::S3DRenderParameterContext, S3DRenderContextInterface )
		// 構築関数
		S3DRenderParameterContext( void ) ;
		// 消滅関数
		virtual ~S3DRenderParameterContext( void ) ;
		// 描画ターゲットを複製設定
		void AttachTargetImagesTo( S3DRenderParameterContext& context ) ;
		// 描画ターゲット設定が同一か判定
		bool IsEqualTargetImages( const S3DRenderParameterContext& context ) const ;
		// 描画パラメータを設定
		void SetAllRenderingParameterTo
			( S3DRenderContextInterface * context, uint64_t flags = 0 ) ;
		// ステレオ立体視描画先設定
		SGLError AttachStereoTargetImage
			( SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft,
					SGLImageObject * pZBufferRight,
					SGLImageObject * pZBufferLeft,
					const SGLImageRect * pView = NULL ) ;
		// ステレオ立体視無効化
		void DisableStereoTargetImage( void ) ;
		// 一時オブジェクト削除
		void DeleteAllTemporaryObjects( void ) ;

	public:
		// デフォルト値
		static void DefaultOptionalContext( OptionalContextSet& optcs ) ;
		// 比較
		static bool IsEqualOptionalContext
			( const OptionalContextSet& optcs1,
				const OptionalContextSet& optcs2 ) ;
		static bool IsEqualBorderParam
			( const OffsetBorderParam& obp1,
				const OffsetBorderParam& obp2 ) ;

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
		// マルチターゲット（2つ目以降）描画先設定
		virtual SGLError AttachMultiTargetImages
			( SGLImageObject*const* ppTargets, size_t nCount ) ;
		// マルチターゲット（2つ目以降）取得
		virtual SGLImageObject*const* GetMultiTargetImages( size_t& nCount ) const ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
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
		// ２Ｄメッシュ描画（DrawImage 呼び出し）
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
	public:
		// 画像変形描画
		virtual SGLError DrawTrianglePolygon
			( const S2DVector * pDstVertices,
				const S2DVector * pSrcVertices,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage ) ;

	public:
		// ｚクリップ範囲を取得
		void GetZClipRange( double& zMin, double& zMax ) const ;
		// 2D 変換行列を取得（カメラは含まない）
		bool GetAffineTransformation( SGLAffine& af ) const ;
		// 3D 変換行列を取得（カメラを含む）
		void GetTransformMatrix( S3DDMatrix& mat, S3DDVector& pos ) ;
		// 透明度を取得
		unsigned int EffectTransparency( unsigned int nTransparency ) const ;
		// 色効果を取得
		bool GetColorEffect( S3DColor& colorEffect ) const ;
		// カメラの逆行列を設定する（カメラを無効化）
		void SetMatrixInverseOfCamera( void ) ;
		// 3D 変換行列（色・透明度含む）を
		// 別の S3DRenderBufferInterface に設定する
		void SetMatrixTransformationTo
						( S3DRenderBufferInterface * render ) ;
		// 2D 変換行列と互換性のある 3D 変換行列（色・透明度含む）を
		// 別の S3DRenderBufferInterface に設定する
		//（※ターゲットにも同じカメラが設定されていると仮定）
		void SetMatrixTransformationAsAffineTo
			( S3DRenderBufferInterface * render,
							double z, uint32_t flagsPaint ) ;
		// 視差カメラを別の S3DRenderBufferInterface に設定する
		void SetParallaxCameraTo
			( S3DRenderContextInterface * render, StereoViewIndex sviView ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// 描画座標空間設定
		virtual SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		virtual SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		virtual SGLError GetMatrixTransformation
			( S3DDMatrix& mat, S3DDVector& pos,
				S3DColor * color = NULL,
				unsigned int * pTransparency = NULL ) const ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		virtual SGLError ResetCustomShaderUniform( void ) ;
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 投影スクリーン座標取得
		virtual SGLError GetProjectionScreen
			( S3DVector& vScreen,
				double& zScale, double& fpPixelAspect ) const ;
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
		// カメラ取得
		virtual void GetCamera
			( S3DDMatrix& matCamera, S3DDVector& posCamera ) const ;
		// 立体視視差設定
		virtual void SetParallax
			( double xParallax, double zFocusRate, double xScreenDelta ) ;
		// 立体視視差取得
		virtual double GetParallax( void ) const ;
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
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) ;
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
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const ;

	public:
		// 選択中の立体視用バッファ取得
		virtual StereoViewIndex CurrentParallaxView( void ) ;
		// 立体視用バッファ選択
		virtual SGLError SelectParallaxView( StereoViewIndex sviView ) ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;
		// 非同期レンダリング開始
		virtual SGLError AsyncFlush
			( uint32_t nFlags = 0, SSystem::SSignalEvent * pSignal = NULL ) ;
		// 非同期レンダリング完了待機
		virtual SGLError WaitFlush( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		// 非同期レンダリングに適したスレッドで実行
		virtual void SuitableProcedure
					( PROCEDURE_RENDERING pfnRendering, void * pInstance ) ;

	public:
		// 遅延削除オブジェクト追加（Flush 時に削除）
		virtual void AddTemporaryObject( ESLObject * pObj ) ;
	} ;

}

#endif
