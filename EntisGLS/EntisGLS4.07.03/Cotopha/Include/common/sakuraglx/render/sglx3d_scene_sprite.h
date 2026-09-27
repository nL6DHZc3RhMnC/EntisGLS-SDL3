
#if	!defined(__SAKURAGLX3D_SCENE_SPRITE_H__)
#define	__SAKURAGLX3D_SCENE_SPRITE_H__	1

#include <sakuraglx/render/sglx3d_scene.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuraglx/sprite/sglx_sprite_multi_view.h>

namespace	SakuraGL
{
	class SGLVRViewProducer ;

	//////////////////////////////////////////////////////////////////////////
	// 3D シーンアイテム・スプライト
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneItemSprite	: public SGLSpriteFormed, public S3DScene::Item
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( S3DSceneItemSprite, SGLSpriteFormed, Item )
		// 構築関数
		S3DSceneItemSprite( void ) ;
		// 消滅関数
		virtual ~S3DSceneItemSprite( void ) ;

	public:
		// 座標モード
		enum	CoordinatesMode
		{
			coordinatesFrame3D,		// 3次元フレーム（フレームバッファ必須）
			coordinatesDirect2D,	// 2次元直接描画（フレームバッファ不要）
		} ;

	protected:
		CoordinatesMode	m_modeCoords ;			// 座標モード
		double			m_tanViewHalfAngle ;	// 2D 表示時の tan(水平視野角/2)
		int				m_pixelsViewWidth ;		// 2D 表示時の水平視野に対応するピクセル数
		S3DRenderContextInterface::DepthMaskOperation
						m_depthMaskOp ;			// バッファ無し＆coordinatesFrame3D の時のｚバッファ処理

	public:
		// 座標モード
		CoordinatesMode GetCoordinatesMode( void ) const ;
		void SetCoordinatesMode( CoordinatesMode mode ) ;
		// 2D 表示時の水平視野角
		double Get2DViewHalfAngle( void ) const ;
		void Set2DViewHalfAngle( double tanHalfAngle ) ;
		// 2D 表示時の水平視野角に対応するピクセル数
		int Get2DViewWidthPixels( void ) const ;
		void Set2DViewWidthPixels( int nPixels ) ;
		// 2D 表示座標変換取得
		SGLAffine Get2DViewAffine( const S3DScene& scene ) const ;
		// 3D直接描画時のｚバッファ操作
		S3DRenderContextInterface::DepthMaskOperation GetDepthMaskOperation( void ) const ;
		void SetDepthMaskOperation( S3DRenderContextInterface::DepthMaskOperation depthOp ) ;

	public:	// SGLSprite オーバーライド
		// 更新領域通知
		virtual void PostUpdate( SGLRect* pUpdate = nullptr ) ;
		// コマンド通知
		virtual bool NotifyCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0,
				int nPriority = commandNormal, bool fOverwritable = false ) ;
		// ローカル座標からグローバル座標へ変換
		virtual bool LocalToGlobal( S2DDVector& vPos ) const ;
		// グローバル座標からローカル座標へ変換
		virtual bool GlobalToLocal( S2DDVector& vPos ) const ;
		// 親スプライト取得
		virtual SGLSprite* GetParent( void ) const ;

	public:	// S3DScene::Item オーバーライド
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// 表示モデル追加
		virtual void RenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// レンダリングスレッド排他処理用
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t TestLocked( void ) const ;

	protected:
		// フレームバッファ描画
		void RenderSpriteFrameBuffer
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion, SGLImageObject * pImage ) ;
		// SGLDrawImageParamList を直接３次元空間へ描画
		void RenderDrawImageList
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion, SGLDrawImageParamList& dipList ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// スプライトｚバッファ付き描画インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteDepthDrawer	: public SGLSpriteDrawer
	{
	protected:
		SGLSprite *			m_pSprite ;
		S3DRenderDevice *	m_pDevice ;
		S3DCustomShader *	m_pShader ;
		S3DDrawWithDepthShaderInterface *
							m_pDrawWithDepth ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteDepthDrawer, SGLSpriteDrawer )
		// 構築関数
		SGLSpriteDepthDrawer( void ) ;

	public:
		// スプライトにアタッチされた
		virtual void OnAttachedSprite( SGLSprite * pSprite ) ;
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) ;
		// 複製（可能なら）
		virtual SGLObject * DuplicateObject( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 3D シーン表示スプライト
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneSprite	: public SGLSpriteMultiView, public S3DScene
	{
	public:
		// 描画先設定
		enum	SecondaryViewBehaviorFlag
		{
			behaviorAutoVRViewParams	= 0x0001,
			behaviorAutoVRViewSize		= 0x0002,
			behaviorAutoVRView			= 0x0003,
		} ;
		struct	ViewEntry
		{
			uint32_t					nBehaviorFlags ;	// complex of enum SecondaryViewBehaviorFlag
			S3DScene::ProjectionParam	paramProj ;
			S3DScene::Camera *			pCamera ;
			S3DScene::ParallaxParam		paramParallax[2] ;
			S3DRenderDevice *			pDevice ;
			SGLImageObject *			pTargetLayered ;
			SGLImageObject *			pTargetLayeredSampler ;
			SGLImageObject *			pTargetLayeredDepth ;
			SGLImageObject *			pTargetLayeredDepthSampler ;
			SSystem::SPointerArray<SGLImageObject>
										aMultiTargets ;
			SSystem::SPointerArray<SGLImageObject>
										aMultiTargetSamplers ;

			ViewEntry( void )
				: nBehaviorFlags( 0 ),
					pCamera( nullptr ),
					pDevice( nullptr ),
					pTargetLayered( nullptr ),
					pTargetLayeredSampler( nullptr ),
					pTargetLayeredDepth( nullptr ),
					pTargetLayeredDepthSampler( nullptr ) { }
			ViewEntry( const ViewEntry& ve )
				: nBehaviorFlags( ve.nBehaviorFlags ),
					paramProj( ve.paramProj ),
					pCamera( ve.pCamera ),
					pDevice( ve.pDevice ),
					pTargetLayered( ve.pTargetLayered ),
					pTargetLayeredSampler( ve.pTargetLayeredSampler ),
					pTargetLayeredDepth( ve.pTargetLayeredDepth ),
					pTargetLayeredDepthSampler( ve.pTargetLayeredDepthSampler ),
					aMultiTargets( ve.aMultiTargets ),
					aMultiTargetSamplers( ve.aMultiTargetSamplers )
			{
				paramParallax[0] = ve.paramParallax[0] ;
				paramParallax[1] = ve.paramParallax[1] ;
			}
			const ViewEntry& operator = ( const ViewEntry& ve )
			{
				nBehaviorFlags = ve.nBehaviorFlags ;
				paramProj =  ve.paramProj ;
				pCamera = ve.pCamera ;
				paramParallax[0] = ve.paramParallax[0] ;
				paramParallax[1] = ve.paramParallax[1] ;
				pDevice = ve.pDevice ;
				pTargetLayered = ve.pTargetLayered ;
				pTargetLayeredSampler = ve.pTargetLayeredSampler ;
				pTargetLayeredDepth = ve.pTargetLayeredDepth ;
				pTargetLayeredDepthSampler = ve.pTargetLayeredDepthSampler ;
				aMultiTargets = ve.aMultiTargets ;
				aMultiTargetSamplers = ve.aMultiTargetSamplers ;
				return	*this ;
			}
			~ViewEntry( void ) { }
		} ;

		// 表示バッファインデックス（0 以降は enum RenderTargetIndex）
		enum	ViewRenderTargetIndex
		{
			renderTargetZBuffer	= -1,	// depth バッファ
		} ;
		// 描画用中間バッファ
		enum	InternalBufferFlag
		{
			renderTargetTemporary0	= renderTargetCount,
			renderTargetTemporary1,
			renderTargetAllCount,
			bufferEffectTarget		= 0x0001,
			bufferEmissiveTarget	= (1 << renderTargetEmission),
			bufferNormalTarget		= (1 << renderTargetNormal),
			bufferDiffusionTarget	= (1 << renderTargetDiffusion),
			bufferAmbientTarget		= (1 << renderTargetAmbient),
			bufferSpecularTarget	= (1 << renderTargetSpecular),
			bufferTemporary0		= (1 << (renderTargetTemporary0)),
			bufferTemporary1		= (1 << (renderTargetTemporary1)),
			bufferAllTarget			= bufferEffectTarget
										| bufferEmissiveTarget
										| bufferNormalTarget
										| bufferAmbientTarget
										| bufferDiffusionTarget
										| bufferSpecularTarget
										| bufferTemporary0,
		} ;
		class	InternalBuffer	: public ESLObject
		{
		public:
			SGLImage			m_imgMultiRenderTarget[renderTargetAllCount] ;
			SGLImage			m_imgMultiRenderSampler[renderTargetAllCount] ;
			SGLImage			m_imgLayeredDepth ;
			SGLImage			m_imgLayeredDepthSampler ;
			S3DRenderContext	m_renderTempSrc ;
			S3DRenderContext	m_renderTempDst ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( InternalBuffer, ESLObject )
		} ;

	protected:
		// 描画先ごとの描画情報
		SSystem::SPtrSortArray<SGLSecondaryViewProducer,ViewEntry>
										m_psaViewEntries ;
		ViewEntry *						m_pSelectViewEntry ;

		SSystem::SStrSortObjectArray<S3DScene::Space>	m_ssoaSpaces ;
		SSystem::SStrSortObjectArray<S3DScene::Item>	m_ssoaItems ;

		class	Sprite3DEntry
		{
		public:
			SSystem::SSyncReference	m_refItem ;
			S3DDMatrix				m_matModel ;
			S3DDVector				m_vModel ;
		} ;
		SSystem::SObjectArray<Sprite3DEntry>	m_aSpriteItems ;
		SSystem::SCriticalSection				m_csSpriteItems ;

		uint32_t	m_flagsInternalBuffer ;
		int			m_iViewRenderTarget ;
		bool		m_flagSceneUpdate ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DSceneSprite, SGLSpriteMultiView, S3DScene )
		// 構築関数
		S3DSceneSprite( void ) ;
		// 消滅関数
		virtual ~S3DSceneSprite( void ) ;

	public:
		// 中間バッファフラグ (enum InternalBufferFlag)
		void SetInternalBufferFlags( uint32_t nFlags ) ;
		uint32_t GetInternalBufferFlags( void ) const
		{
			return	m_flagsInternalBuffer ;
		} ;
		// 表示する描画ターゲットバッファ
		// （enum ViewRenderTargetIndex or enum RenderTargetIndex）
		void SetViewRenderTargetBuffer
				( int iRenderTarget = renderTargetComposed ) ;
		int GetViewRenderTargetBuffer( void ) const
		{
			return	m_iViewRenderTarget ;
		}
		// 描画先（親スプライト）へデプスを反映する描画を行うよう設定する
		void SetSpriteDepthDrawer( void ) ;
		// ビュー毎に異なるカメラと透視変換を設定する
		void SetViewSettingAs
			( SGLSecondaryViewProducer * psvp, const ViewEntry& viewEntry ) ;
		// ビュー毎の描画設定を取得する
		ViewEntry * GetViewSettingAs( SGLSecondaryViewProducer * psvp ) const ;
		// ビュー毎の描画設定を削除する
		void RemoveViewSettingAs( SGLSecondaryViewProducer * psvp ) ;
		void RemoveAllViewSettings( void ) ;
		// 現在の設定を ViewEntry に取得する
		void GetCurrentViewSettings( ViewEntry& viewEntry ) ;
		// VR HMD 表示用のパラメータを取得する
		void GetViewSettingForVRHMD
			( ViewEntry& viewEntry, SGLSize& sizeFrame,
				const SGLVRViewProducer * pVR,
				float32_t zNear, float32_t zFar,
				S3DScene::Camera * pCamera, S3DRenderDevice * pDevice ) ;

	public:
		// 空間登録
		void AddSpaceProperty
			( const wchar_t * pwszName, S3DScene::Space * pSpace ) ;
		// 空間取得
		S3DScene::Space *
			GetSpaceProperty( const wchar_t * pwszName ) const ;
		// 空間削除
		void RemoveSpacePropertyAs( const wchar_t * pwszName ) ;
		void RemoveSpaceProperty( S3DScene::Space * pSpace ) ;
		// すべての空間を削除する
		void RemoveAllSpacesAndItems( void ) ;

	public:
		// アイテム登録
		void AddItemProperty
			( const wchar_t * pwszName, S3DScene::Item * pItem ) ;
		// アイテム取得
		S3DScene::Item *
			GetItemProperty( const wchar_t * pwszName ) const ;
		// 空間削除
		void RemoveItemPropertyAs( const wchar_t * pwszName ) ;
		void RemoveItemProperty( S3DScene::Item * pItem ) ;

	protected:
		// 3D アイテム・スプライト追加
		void AddSceneItemSprite( S3DSceneItemSprite * pSprite ) ;
		// 3D アイテム・スプライト削除通知
		void OnDeleteSceneItemSprite( S3DSceneItemSprite * pSprite ) ;

	public:
		// ヒットアイテム検索
		SGLSprite * GetHitRayForSprite
			( S2DDVector& vPos, double& zDistance,
				const S3DDVector& vRay,
				const S3DDVector& vRayOrigin, bool fHitLocal ) const ;

	public:
		// VR 空間内タッチ UI
		class	VRTouchContext
		{
		public:
			SSystem::SSyncReference	m_refHovering ;		// S3DSceneItemSprite*
			bool					m_flagTouching ;
			uint32_t				m_idMouse ;
			double					m_fpTouchReach ;
			double					m_fpHoverReach ;
		public:
			VRTouchContext( double fpTouch = 0.03, double fpHover = 0.1, uint32_t idMouse = 1 )
				: m_flagTouching( false ), m_idMouse( idMouse ),
					m_fpTouchReach( fpTouch ), m_fpHoverReach( fpHover ) { }
		} ;
		// ホバー／タッチ操作実行
		void VRTouchOperation( VRTouchContext& context, const S3DDVector& vTouchPos ) ;
		// ホバー／タッチアイテム検索
		S3DSceneItemSprite * GetTouchForSprite
			( S2DDVector& vLocalPos, double& fpDistance,
				const S3DDVector& vTouchPos, double fpMaxReach ) const ;

	public:
		// レンダリングデバイスの設定（S3DScene/SGLSprite オーバーライド）
		virtual SGLError SetRenderDevice( S3DRenderDevice * pDevice ) ;
		// レンダリングスレッド排他処理用（S3DScene/SGLSprite オーバーライド）
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t TestLocked( void ) const ;
		// 透視変換（S3DScene オーバーライド）
		virtual SGLError GetProjectionOfView
			( ProjectionParam& projParam, SGLSecondaryViewProducer * psvp ) const ;
		// 透視変換（S3DScene オーバーライド）
		virtual void SetProjection
			( const S3DScene::ProjectionParam& projParam ) ;
		// 透視変換（SGLSprite オーバーライド）
		virtual void SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 視差設定（S3DScene オーバーライド）
		virtual void SetParallax
			( double xParallax,
				double zFocusRate = 1.0, double xScreenDelta = 0.0 ) ;

	public:	// SGLSprite オーバーライド
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
		// 描画前処理
		virtual void BeforeDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// 描画後処理
		virtual void AfterDraw( Stereo3DView s3dView = s3dMonoview ) ;
	protected:
		// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;
		// 描画前処理
		virtual void BeforeDrawChildren( Stereo3DView s3dView = s3dMonoview ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;

		struct	SuitableRenderParam
		{
			S3DSceneSprite *				pScene ;
			S3DRenderContextInterface *		pRender ;
			RenderContext::StereoViewIndex	sviView ;
		} ;
		static void SuitableRenderProc( void * pInstance ) ;

	public:
		// ヒットアイテム検索
		virtual SGLSprite* GetHitSpriteAt( S2DDVector& vPos ) const ;
	protected:
		// 背景色取得
		virtual bool GetFillBackColor( uint32_t& argbFill ) const ;
		// 背景色設定
		virtual SGLError SetFillBackColor
			( uint32_t argbFill, bool flagFillBack = true ) ;

	public:	// SGLSpriteMultiView オーバーライド
		// 出力先バッファ選択
		virtual SGLError SelectSecondaryView( SGLSecondaryViewProducer * psvp ) ;
	protected:
		// ユーザー定義バッファ生成
		virtual void OnCreateUserLayerBuffer
			( SecondaryBuffer * pBuffer,
				const SGLSize& sizeRight, const SGLSize& sizeLeft,
				uint32_t format, uint32_t depth,
				uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D ) ;
		// ユーザー定義バッファ解放時処理
		virtual void OnReleaseUserLayerBuffer( ESLObject * pUserLayer ) ;

	public:	// S3DScene オーバーライド
		// レンダリング前処理
		virtual SGLError PrepareRenderScene
				( Camera * pCamera, uint64_t nOptFlags = 0 ) ;

		struct	SuitablePrepareRenderParam
		{
			SGLError			err ;
			S3DSceneSprite *	pScene ;
			S3DScene::Camera *	pCamera ;
			uint64_t			nOptFlags ;
		} ;
		static void SuitablePrepareRenderProc( void * pInstance ) ;

		// レンダリング後処理
		virtual SGLError FinishRenderScene( void ) ;
		// 画面更新通知
		virtual void PostSceneUpdate( void ) ;
		// レンダリングスレッド排他処理用
		virtual SSystem::SSharableMutex * GetUIThreadMutex( void ) const ;

		// カメラオブジェクト削除通知（参照している場合に安全に参照解除）
		virtual void NotifyDeleteCamera( Camera * pCamera ) ;
		// メインカメラ設定
		virtual void SetMainCamera( Camera * pCamera ) ;
		virtual void SetMainCamera( Camera * pCamera, SGLSecondaryViewProducer * psvp ) ;

	public:
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

		friend class S3DSceneItemSprite ;
	} ;

}

#endif

