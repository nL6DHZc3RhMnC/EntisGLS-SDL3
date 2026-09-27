
#if	!defined(__SAKURAGLX3D_SCENE_H__)
#define	__SAKURAGLX3D_SCENE_H__	1

#include <sakuracl/erisa/sgl_erisa_crypt_math.h>
#include <sakuraglx/render/sglx3d_collision.h>
#include <sakuragl/sgl3d/sgl_render_shaper.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 3D シーン
	//////////////////////////////////////////////////////////////////////////

	class	S3DScene	: public S3DCollider
	{
	public:
		// アイテム種別（描画順）
		enum	ItemClass
		{
			classCamera,			// カメラ
			classLight,				// 光源
			classPreRender,			// レンダリング実行前処理（全視点共通）
			classPreRender2,
			classPreRenderFrame,	// フレーム前レンダリング処理（全視点共通）
			classBeginFrame,		// フレーム描画開始（視点毎）
			classBackscape,			// 背景球
			classScape,				// 背景
			classField,				// フィールド
			classStaticItem1,		// 固定アイテム
			classStaticItem2,
			classDynamicItem1,		// 動的アイテム
			classDynamicItem2,
			classDynamicItem3,
			classEffectItem,		// 効果アイテム
			classLayeredSpace,		// レイヤード空間アイテム（※これ以降はシャドウマッピング等に反映されない）
			// ←画面効果（前）
			classLayeredItem1,		// レイヤード効果アイテム
			classLayeredItem2,
			classEffect1,			// 効果
			classEffect2,			// 効果（後）
			// ←画面効果（後）
			classEffect3,			// 効果（最終）
			classEndFrame,			// フレーム描画終了（視点毎）
			classCount,
			classFirstRenderClass	= classBackscape,
			classLastRenderClass	= classEffect3,

			// シャドウマップの描画対象は classField ～ classEffect2
			classFirstRenderShadowmapClass	= classField,
			classLastRenderShadowmapClass	= classEffect2,
		} ;
		enum	ItemClassFlag
		{
			classBitBackscape		= (1 << classBackscape),
			classBitScape			= (1 << classScape),
			classBitField			= (1 << classField),
			classBitStaticItem1		= (1 << classStaticItem1),
			classBitStaticItem2		= (1 << classStaticItem2),
			classBitDynamicItem1	= (1 << classDynamicItem1),
			classBitDynamicItem2	= (1 << classDynamicItem2),
			classBitDynamicItem3	= (1 << classDynamicItem3),
			classBitEffectItem		= (1 << classEffectItem),
			classBitLayeredSpace	= (1 << classLayeredSpace),
			classBitLayeredItem1	= (1 << classLayeredItem1),
			classBitLayeredItem2	= (1 << classLayeredItem2),
			classBitEffect1			= (1 << classEffect1),
			classBitEffect2			= (1 << classEffect2),
			classBitEffect3			= (1 << classEffect3),
			classBitLayeredItems
					= classBitLayeredSpace
						| classBitLayeredItem1 | classBitLayeredItem2,
			classBitsAllScape
					= classBitBackscape | classBitScape | classBitField,
			classBitsAllStaticItem
					= classBitStaticItem1 | classBitStaticItem2,
			classBitsAllDynamicItem
					= classBitDynamicItem1
						| classBitDynamicItem2 | classBitDynamicItem3,
			classBitsItems
					= classBitsAllStaticItem | classBitsAllDynamicItem
						| classBitEffectItem | classBitLayeredItems,
			classBitsEffects
					= classBitEffect1 | classBitEffect2 | classBitEffect3,
		} ;

		// アイテム描画効果
		enum	ShaderFlags
		{
			shaderForceToon			= 0x00000001,
			shaderForceBorder		= 0x00000002,
			shaderForceEmision		= 0x00000004,
			shaderNoDrawBorder		= 0x00000010,
			shaderFreeToon			= 0x00010000,
			shaderFreeBorder		= 0x00020000,
			shaderFreeEmision		= 0x00040000,
			shaderForceMask			= 0x0000FFFF,
			shaderFreeShifter		= 16,
		} ;

		// アイテム作用フラグ
		enum	ItemBehaviorFlag
		{
			itemVisible			= 0x00000001,	// 可視状態
			itemCollision		= 0x00000002,	// 当たり判定有り
			itemTimer			= 0x00000004,	// OnTimer 呼び出し
			itemOwnerBehavior	= 0x00000008,	// OnUpdateBehavior 呼び出し
			itemGlobalSpace		= 0x00000010,	// 座標変換をグローバル空間にする
			itemCameraShift		= 0x00000020,	// カメラに対して平行移動する
			itemCameraSpace		= 0x00000040,	// カメラ姿勢を基準とした空間にする
			itemHideNear		= 0x00000200,	// カメラからの距離が近くなると表示しない
			itemHideFar			= 0x00000400,	// カメラからの距離が遠くなると表示しない
			itemSpaceHidden		= 0x00000800,	// 非表示空間
			itemIgnore			= 0x00001000,	// 無視空間
			itemInheritFlags	=				// アイテムから親空間へ継承されるフラグセット
				itemVisible | itemCollision,
			itemSpaceLocalFlags	=				// 子空間から親空間へ継承されないフラグセット
				itemHideNear | itemHideFar
					| itemSpaceHidden | itemIgnore,
		} ;

		// 空間要素
		enum	SpaceInfoElementFlag
		{
			spaceElementTransformation	= 0x0001,
			spaceElementPosition		= 0x0002,
			spaceElementTransparency	= 0x0004,
			spaceElementColorAdd		= 0x0008,
			spaceElementColorMul		= 0x0010,
		} ;

		// 透視変換情報
		struct	ProjectionParam
		{
			S3DVector		vScreen ;
			float32_t		fpZoom ;
			float32_t		fpPixelAspect ;
			float32_t		zNear ;
			float32_t		zFar ;

			ProjectionParam( void ) ;

			const S2DDVector& ViewProjectionOf
				( S2DDVector& vProj, const S3DDVector& vPos ) const ;
			const S2DDVector& ViewProjectionAndScaleOf
				( S2DDVector& vProj, S2DDVector& vScale, const S3DDVector& vPos ) const ;
		} ;

		// 大域疑似フォッグ
		struct	FogParam
		{
			SGLPalette	rgbFog ;
			double		zNear ;
			double		zFar ;

			FogParam( void ) ;
		} ;

		// 大域環境マッピング
		struct	EnvMappingParam
		{
			SGLImageObject *	pImage ;
			uint32_t			typeMap ;	// enum S3DRenderContextInterface::EnvironmentMappingType
			S3DMatrix			matMap ;

			EnvMappingParam( void ) ;
		} ;

		// 動的環境マッピング
		struct	DynamicEnvironment
		{
			S3DDVector	vCenterPos ;
			uint64_t	flagsExclusion ;	// default shadingNoReflectObject
			uint64_t	maskTargetClasses ;	// default classBitsAllScape | classBitStaticItem

			DynamicEnvironment( void ) ;
		} ;

		// 大域環境マッピングソース
		enum	EnvironmentMappingSource
		{
			envmapSourceDefault,
			envmapSourceViewport,
		} ;

		// 輪郭線
		struct	OffsetBorderParam
		{
			SGLPalette	rgbBorder ;
			float32_t	aThickness ;
			float32_t	bThickness ;

			OffsetBorderParam( void ) ;
		} ;

		// 視差情報
		struct	ParallaxParam
		{
			S3DDMatrix	matPosture ;
			S3DDVector	vParallax ;
			S3DVector	vScreenDelta ;
			float32_t	fpAspectDelta ;

			ParallaxParam( void ) ;
			ParallaxParam( const ParallaxParam& src ) ;
			const ParallaxParam& operator = ( const ParallaxParam& src ) ;
			void SetParallax
				( double xParallax, double zFocus, double xScreenDelta ) ;
		} ;
		struct	StereoParallaxParam
		{
			ParallaxParam	ppView[2] ;		// 指標は RenderContext::StereoViewIndex
		} ;

		// 空間情報
		struct	SpaceInfo
		{
			S3DDMatrix		m_matTransformation ;
			S3DDVector		m_vCenter ;
			uint32_t		m_flagsModified ;	// enum SpaceInfoElementFlag 集合
			uint32_t		m_flagsShader ;		// enum ShaderFlags 集合
			uint32_t		m_nTransparency ;
			S3DColor		m_colorEffect ;

			SpaceInfo( void )
				: m_matTransformation
						( 1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0 ),
					m_vCenter( 0.0, 0.0, 0.0 ),
					m_flagsModified( 0 ),
					m_flagsShader( 0 ), m_nTransparency( 0 ) { }
			SpaceInfo( const SpaceInfo& src )
				: m_matTransformation( src.m_matTransformation ),
					m_vCenter( src.m_vCenter ),
					m_flagsModified( src.m_flagsModified ),
					m_flagsShader( src.m_flagsShader ),
					m_nTransparency( src.m_nTransparency ),
					m_colorEffect( src.m_colorEffect ) { }
			// m_flagsShader enum ShaderFlags 効果フィルタ
			uint64_t /*S3DShadingFlags*/ EffectedShaderFlags
				( uint64_t flagsShading /* S3DShadingFlags */,
					uint32_t optAddShader /* ShaderFlags */ ) const
			{
				return	S3DScene::GetEffectedShaderFlags
						( flagsShading, optAddShader | m_flagsShader ) ;
			}
		} ;
		// enum ShaderFlags 効果フィルタ
		static uint64_t /*S3DShadingFlags*/ GetEffectedShaderFlags
			( uint64_t flagsShading /* S3DShadingFlags */,
				uint32_t optShaderFlags /* ShaderFlags */ ) ;

		// タイマー
		class	Space ;
		class	Timer	: public SGLObject
		{
		public:
			bool	m_flagAutoDelete ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Timer, SGLObject )
			// 構築関数
			Timer( bool flagAutoDelete = false ) ;
			Timer( const Timer& tm ) ;
			// タイマー処理
			virtual bool OnTimer
				( S3DScene& scene, Space& space, uint32_t msecPast ) ;
			// フラッシュ処理
			virtual bool OnFlush( S3DScene& scene, Space& space ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// アニメーション
		class	Animation	: public Timer
		{
		public:
			enum	LoopType
			{
				loopGo,
				loopGoBack,
			} ;
			SpaceInfo *					m_psiTarget ;
			SSystem::SArray<uint32_t>	m_keyDurations ;
			uint32_t					m_msecDuration ;
			uint32_t					m_msecCurrent ;
			uint32_t					m_typeLoop ;
			uint32_t					m_nLoopCount ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Animation, Timer )
			// 構築関数
			Animation( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// 継続時間
			void SetDuration( uint32_t msecDuration ) ;
			void SetDurations( const uint32_t * pDurations, size_t nDivision ) ;
			// ループ回数
			void SetLoop
				( uint32_t nLoop = 0xFFFFFFFF, LoopType typeLoop = loopGo ) ;
			// タイマー処理
			virtual bool OnTimer
				( S3DScene& scene, Space& space, uint32_t msecPast ) ;
			// フラッシュ処理
			virtual bool OnFlush( S3DScene& scene, Space& space ) ;
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// アイテムリスナ
		class	ItemEventListener
		{
		public:
			// 規定のレンダリングデバイス設定
			virtual void OnSetRenderDevice( S3DRenderDevice * pDevice ) = 0 ;
			// レンダリングの為のデバイスリソース準備
			virtual void OnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) = 0 ;
			// レンダリング前後処理（全視点共通）
			virtual void OnItemRenderEvent
				( S3DScene& scene, ItemClass clsItem ) = 0 ;
			// 当たり判定追加
			//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
			virtual void OnItemRenderCollision
				( const S3DScene& scene, S3DCollision& render ) = 0 ;
			// 表示モデル追加
			virtual void OnItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) = 0 ;
			// 描画前処理
			virtual void BeforeItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) = 0 ;
			// 描画後処理
			virtual void AfterItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) = 0 ;
		} ;

		// エフェクト描画インターフェース
		class	DrawLayerEffector
		{
		public:
			// 描画優先度
			virtual int GetDrawingPriority( void ) const = 0 ;
			// 要求カラーバッファ
			// ※renderTargetComposed の要求はバッファの複製を要求
			virtual uint32_t GetRequiredColorBufferMask( void ) const = 0 ;
			// 描画処理
			virtual void DrawEffect
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) = 0 ;
		} ;

		// ローカルシェーダー
		class	LocalShader	: public ESLObject
		{
		protected:
			S3DCustomShader *				m_pLocalShader ;
			S3DCustomShader::UniformSet		m_uniforms ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( LocalShader, ESLObject )
			// 構築関数
			LocalShader( void ) ;
			LocalShader( const LocalShader& src ) ;
			// 消滅関数
			virtual ~LocalShader( void ) ;
		public:
			// カスタムシェーダー設定
			virtual void SetCustomShader( S3DCustomShader * pShader ) ;
			// カスタムシェーダー取得
			virtual S3DCustomShader * GetCustomShader( void ) const ;
			// カスタムシェーダーパラメータ
			virtual S3DCustomShader::UniformSet& ShaderUniformSet( void ) ;
			virtual S3DCustomShader::UniformData *
					GetShaderUniformAs( const wchar_t * pwszName ) const ;
			virtual void SetShaderUniformDataAs
				( const wchar_t * pwszName,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nLength ) ;
		} ;

		// シーンアイテム
		class	Item	: public S3DPhysicsScene::Object
		{
		protected:
			SSystem::SSyncReference	m_refParentSpace ;	// 親空間
			SSystem::SSyncReference	m_refSpace ;	// 参照空間（デフォルトは親空間）
			S3DDMatrix				m_matLink ;		// 参照空間への接続変換
			S3DDVector				m_vLink ;
			ItemEventListener *		m_pListener ;
		public:
			const wchar_t *			m_pwszID ;
			SpaceInfo				m_space ;
			ItemClass				m_classItem ;
			uint32_t				m_nRenderPriority ;	// [0,15] 数値が小さい順に描画
			uint32_t				m_flagsBehavior ;	// enum ItemBehaviorFlag 集合
			uint32_t				m_maskClasses ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Item, Object )
			// 構築関数
			Item( void ) ;
			Item( const Item& src ) ;
			// 消滅関数
			virtual ~Item( void ) ;
			// 空間コンテナ取得
			Space * GetReferenceSpace( void ) const ;
			Space * GetParentSpace( void ) const ;
			// 親シーン取得
			S3DScene * GetScene( void ) const ;
			// ローカル変換行列を取得
			virtual const S3DDMatrix&
					GetLocalTransformation( S3DDMatrix& matLocal ) const ;
			// ローカル変換行列を設定
			virtual void SetLocalTransformation( const S3DDMatrix& matLocal ) ;
			// ローカル座標を取得
			virtual const S3DDVector&
					GetLocalItemPosition( S3DDVector& vLocal ) const ;
			// ローカル変換行列を設定
			virtual void SetLocalItemPosition( const S3DDVector& vLocal ) ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			void CalcItemLinkTransformation
					( S3DDMatrix& matrix, S3DDVector& pos ) const ;
			// グローバル空間変換行列を計算
			void CalcGlobalTransformation
					( S3DDMatrix& matrix, S3DDVector& pos ) const ;
			// グローバル色効果取得（乗算色α要素に不透明度取得）
			virtual void CalcGlobalColorEffect( S3DColor& clrEffect ) const ;
			// 相対空間変換行列を計算
			virtual void CalcOffsetTransformation
				( S3DDMatrix& matrix, S3DDVector& pos,
								const Space * pStandardSpace ) const ;
			virtual void CalcOffsetTransformation
				( S3DDMatrix& matrix, S3DDVector& pos,
						const S3DDMatrix& matStandard,
						const S3DDVector& vStandard ) const ;
			// 参照空間を変更する（現在の座標・姿勢を維持）
			void SwitchReferenceSpace( Space * pRefSpace ) ;
			// 参照空間を変更する（参照空間に対して正則）
			void SetReferenceSpace( Space * pRefSpace ) ;
			// リスナ設定
			ItemEventListener * AttachItemListener( ItemEventListener * pListener ) ;
			ItemEventListener * GetItemListener( void ) const ;
		public:
			// タイマ処理（アイテム毎に非同期的呼び出しに対応する必要あり）
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// アイテム作用の追加処理（アイテム毎に非同期的呼び出しに対応する必要あり）
			virtual void OnUpdateBehavior( S3DScene& scene ) ;
			// 規定のレンダリングデバイス設定通知
			virtual void SetRenderDevice( S3DRenderDevice * pDevice ) ;
			// レンダリングの為のデバイスリソース準備
			virtual void PrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// レンダリング前後処理
			//（全視点共通）（アイテム毎に非同期的呼び出しに対応する必要あり）
			virtual void OnRenderEvent
				( S3DScene& scene, ItemClass clsItem ) ;
			// 当たり判定追加
			//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
			virtual void RenderCollision
				( const S3DScene& scene, S3DCollision& render ) ;
			// 表示モデル追加
			virtual void RenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
		public:
			// 当たり判定追加（ローカル空間）
			virtual void RenderLocalCollision
				( const S3DScene& scene, S3DCollision& render ) ;
			// 表示モデル追加（ローカル空間）
			virtual void RenderLocalModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
		public:
			// レンダリングスレッド排他処理用
			virtual SSystem::SError Lock
				( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
			virtual SSystem::SError Unlock( void ) const ;
			virtual atomic_int_t TestLocked( void ) const ;

		public:	// SGLObject
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

		public:	// S3DPhysicsScene::Object
			// 関連付けられた物理演算オブジェクト取得
			virtual S3DPhysicsScene::Actor * GetPhysicalActor( void ) const ;
			// オーナーオブジェクト取得
			virtual ESLObject * GetOwnerObject( void ) const ;
			// 座標取得
			virtual const S3DDVector&
						GetPhysicsPosition( S3DDVector& vPos ) const ;
			// 座標設定
			virtual void SetPhysicsPosition( const S3DDVector& vPos ) ;
			// 姿勢取得
			virtual const S3DQuaternion&
						GetPhysicsPosture( S3DQuaternion& qPosture ) const ;
			// 姿勢設定
			virtual void SetPhysicsPosture( const S3DQuaternion& qPosture ) ;

		protected:
			// UpdateBehaviorFlags 呼び出し
			static void InvokeUpdateBehaviorFlags
				( S3DScene& scene, Space * pSpace, bool flagSpaceVisible = true )
			{
				scene.UpdateBehaviorFlags( pSpace, flagSpaceVisible ) ;
			}
			// レンダリングイベント呼び出し
			static void InvokeRenderSceneEventItems
				( S3DScene& scene, Space* pSpace, ItemClass clsItem ) ;
			// コリジョン呼び出し
			static void InvokeRenderSceneCollision
				( const S3DScene& scene,
					S3DCollision& collision,
					const Space* pSpace, uint32_t maskClasses ) ;
			// レンダリング呼び出し
			static void InvokeRenderSceneItems
				( const S3DScene& scene, S3DRenderContextInterface& render,
					const Space* pSpace,
					uint64_t flagsExclusion = 0, uint32_t optShader = 0 ) ;

			friend class S3DScene ;
			friend class Space ;
		} ;
		friend class Item ;

		// 空間コンテナ
		class	Space	: public SGLObject, public LocalShader,
							public SpaceInfo, public DrawLayerEffector
		{
		protected:
			const wchar_t *					m_pwszID ;
			SSystem::SSyncReference			m_refParent ;
			SSystem::SSyncReference			m_refSpace ;			// 参照空間（デフォルトは親空間）
			S3DDMatrix						m_matLink ;				// 参照空間への接続変換
			S3DDVector						m_vLink ;
			uint32_t						m_flagsBehavior ;		// 空間に含まれる子アイテムの動作 enum ItemBehaviorFlag
			uint32_t						m_maskItemClasses ;		// 空間に含まれる子アイテムの種別ビット集合 enum ItemClassFlag
			uint32_t						m_flagsSpaceBehavior ;	// 空間固有のアイテムの動作 enum ItemBehaviorFlag
			uint32_t						m_maskSpaceClasses ;	// 空間固有の種別ビット集合 enum ItemClassFlag
			uint32_t						m_maskLayeredClasses ;	// レイヤー描画する集合 enum ItemClassFlag
			int32_t							m_zLayeredPriority ;	// レイヤー描画する優先度（大きい方を先に描画）
			uint32_t						m_maskDrawSrcBuffers ;	// レイヤー描画に必要なソース
			float32_t						m_fpLayeredParam ;		// レイヤー描画パラメータ（デフォルトは透明度 [0,1]）
			EnvironmentMappingSource		m_emsReflectionSource ;	// レイヤー描画の際の環境マッピング
			EnvironmentMappingSource		m_emsRefractionSource ;
			ItemEventListener *				m_pListener ;
			DrawLayerEffector *				m_pLayerEffector ;
			SSystem::SPointerArray
					<DrawLayerEffector>		m_aTempEffector ;
			SSystem::SPointerArray<Space>	m_children ;
			SSystem::SPointerArray<Item>	m_items ;
			SSystem::SPointerArray<Timer>	m_timers ;
			SSystem::SObjectArray<ESLObject>	m_aDelayRemove ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2( Space, SGLObject, LocalShader )
			// 構築関数
			Space( void ) ;
			Space( const Space& src ) ;
			// 消滅関数
			virtual ~Space( void ) ;
			// ID （ポインタの管理は呼び出し側が行う）
			const wchar_t * GetSpaceID( void ) const ;
			void AttachSpaceIDString( const wchar_t * pwszID ) ;
		public:
			// 親空間取得
			Space * GetParentSpace( void ) const ;
			// 親シーン取得
			S3DScene * GetParentScene( void ) const ;
			virtual S3DScene * GetScene( void ) const ;
			// 参照空間取得
			Space * GetReferenceSpace( void ) const ;
			// 親判定
			bool IsDescendantChild( Space * pChild ) const ;
			// 空間変換行列（アイテム自身を含まないグローバル変換）を計算
			void CalcSpaceLinkTransformation
					( S3DDMatrix& matrix, S3DDVector& pos ) const ;
			// グローバル空間変換行列を計算
			virtual void CalcGlobalTransformation
				( S3DDMatrix& matrix, S3DDVector& pos ) const ;
			virtual const S3DDVector&
					CalcGlobalPosition( S3DDVector& pos ) const ;
			// グローバル色効果取得（乗算色α要素に不透明度取得）
			virtual void CalcGlobalColorEffect( S3DColor& clrEffect ) const ;
			// 相対空間変換行列を計算
			virtual void CalcOffsetTransformation
				( S3DDMatrix& matrix, S3DDVector& pos,
								const Space * pStandardSpace ) const ;
			// 参照空間を変更する（現在の座標・姿勢を維持）
			void SwitchReferenceSpace( Space * pRefSpace ) ;
			// 参照空間を変更する（参照空間に対して正則）
			void SetReferenceSpace( Space * pRefSpace ) ;
		public:
			// ローカル変換行列を取得
			virtual const S3DDMatrix&
					GetLocalTransformation( S3DDMatrix& matLocal ) const ;
			// ローカル変換行列を設定
			virtual void SetLocalTransformation( const S3DDMatrix& matLocal ) ;
			// ローカル座標を取得
			virtual const S3DDVector&
					GetLocalSpacePosition( S3DDVector& vLocal ) const ;
			// ローカル変換行列を設定
			virtual void SetLocalSpacePosition( const S3DDVector& vLocal ) ;
		public:
			// 作用フラグ変更
			uint32_t ModifyBehaviorFlags
				( uint32_t nAddFlags, uint32_t nRemoveFlags ) ;
			// 作用フラグ取得
			uint32_t GetBehaviorFlags( void ) const ;
			// 空間クラス追加
			void AddSpaceClass( ItemClass icls ) ;
			// 削除
			void RemoveSpaceClass( ItemClass icls ) ;
		public:
			// レイヤー描画対象クラス集合
			uint32_t GetLayeredSpaceClasses( void ) const ;
			bool IsLayeredSpaceClass( ItemClass icls ) const ;
			// レイヤー描画対象変更
			uint32_t ModifyLayeredSpaceClasses
				( uint32_t nAddClasses, uint32_t nRemoveClasses ) ;
			void AddLayeredSpaceClass( ItemClass icls ) ;
			void RemoveLayeredSpaceClass( ItemClass icls ) ;
			// レイヤー描画優先度
			void SetDrawingPriority( int zPriority ) ;
			// レイヤー描画パラメータ
			float32_t GetLayeredDrawParameter( void ) const ;
			void SetLayeredDrawParameter( float32_t fpParam ) ;
			// レイヤー描画環境マッピングソース
			void SetEnvironmentMappingSource
				( EnvironmentMappingSource emsReflection,
					EnvironmentMappingSource emsRefraction ) ;
			void GetEnvironmentMappingSource
				( EnvironmentMappingSource& emsReflection,
					EnvironmentMappingSource& emsRefraction ) const ;
			// レイヤー描画一時効果追加（classPreRender で追加）
			void AddTemporaryEffect( DrawLayerEffector * pEffector ) ;
		public:	// DrawLayerEffector 実装
			// 描画優先度
			virtual int GetDrawingPriority( void ) const ;
			// 要求カラーバッファ
			virtual uint32_t GetRequiredColorBufferMask( void ) const ;
			// classLayeredSpace 描画
			virtual void DrawEffect
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		public:
			// リスナ設定
			ItemEventListener * AttachItemListener( ItemEventListener * pListener ) ;
			ItemEventListener * GetItemListener( void ) const ;
			// レイヤー描画効果設定
			DrawLayerEffector * AttachDrawLayerEffector( DrawLayerEffector * pEffector ) ;
			DrawLayerEffector * GetDrawLayerEffector( void ) const ;
		public:
			// タイマ処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// タイマ処理フラッシュ
			virtual void FlushAllTimers( S3DScene& scene ) ;
			// タイマ追加
			void AddTimer( Timer * pTimer ) ;
			// タイマ削除
			void RemoveTimer( Timer * pTimer ) ;
			void RemoveAllTimers( void ) ;
			// タイマ検索
			ssize_t FindTimerTypeOf
				( const ESLRuntimeClass& rtClass, size_t iFirst = 0 ) const ;
			// タイマ数取得
			size_t GetTimerCount( void ) const ;
			// タイマ取得
			Timer * GetTimerAt( size_t iTimer ) const ;
			Timer * GetTimerTypeOf
				( const ESLRuntimeClass& rtClass, size_t iFirst = 0 ) const ;
		public:
			// アイテム作用の追加処理
			virtual void OnUpdateBehavior( S3DScene& scene ) ;
			// 含まれる子アイテムのクラス種別ビットマスク
			uint32_t GetItemClassesMask( void ) const
			{
				return	m_maskItemClasses ;
			}
			// 規定のレンダリングデバイス設定通知
			virtual void SetRenderDevice( S3DRenderDevice * pDevice ) ;
			// レンダリングの為のデバイスリソース準備
			virtual void PrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// レンダリング前後処理（全視点共通）
			virtual void OnRenderEvent
				( S3DScene& scene, ItemClass clsItem ) ;
			// 当たり判定追加
			virtual void RenderCollision
				( const S3DScene& scene,
					S3DCollision& render, uint32_t maskClasses ) const ;
			// 表示モデル追加
			virtual void RenderModel
				( const S3DScene& scene, S3DRenderContextInterface& render,
					uint32_t optShader,
					ItemClass clsItem, uint64_t flagsExclusion = 0 ) const ;
		protected:
			// カスタム描画
			virtual void RenderExtension
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					ItemClass clsItem, uint64_t flagsExclusion = 0 ) const ;
		public:
			// アイテム追加
			void AddItem( Item * pItem ) ;
			void InsertItem( size_t i, Item * pItem ) ;
			// アイテム削除（分離）
			bool RemoveItem( Item * pItem ) ;
			// アイテム遅延削除
			void DelayDeleteItem( Item * pItem ) ;
			// アイテム全削除（分離）
			void RemoveAllItems( void ) ;
			// アイテム数取得
			size_t GetItemCount( void ) const ;
			// アイテム取得
			Item * GetItemAt( size_t i ) const ;
			// アイテム検索
			ssize_t FindItemAs( const wchar_t * pwszID ) const ;
			Item * GetItemAs( const wchar_t * pwszID ) const ;
		public:
			// 子空間追加
			void AddChild( Space * pChild ) ;
			void InsertChild( size_t i, Space * pChild ) ;
			// 子空間削除
			bool RemoveChild( Space * pChild ) ;
			// 子空間全削除
			void RemoveAllChildren( void ) ;
			// 子空間数取得
			size_t GetChildrenCount( void ) const ;
			// 子空間取得
			Space * GetChildAt( size_t i ) const ;
			// 子空間検索
			ssize_t FindChildAs( const wchar_t * pwszID ) const ;
			Space * GetChildAs( const wchar_t * pwszID ) const ;
		public:
			// レンダリングスレッド排他処理用
			virtual SSystem::SError Lock
				( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
			virtual SSystem::SError Unlock( void ) const ;
			virtual atomic_int_t TestLocked( void ) const ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

			friend class S3DScene ;
			friend class Item ;
		} ;

		// シャドウマッピング
		enum	ShadowMapFlag
		{
			shadowPersDistance		= 0x0001,	// use zPersDistance except fpPixelDensity
			shadowAdjustByCamera	= 0x0002,	// use zCameraStdDistance
		} ;
		struct	ShadowMapParam
		{
			uint32_t	nFlags ;			// complex of enum ShadowMapFlag
			SGLSize		sizeMapping ;
			float32_t	fpPixelDensity ;	// default 1.0
			float32_t	tanAngleOfView ;	// default 1.0
			float32_t	zPersDistance ;		// nFlags with shadowPersDistance
			float32_t	zCameraStdDistance ;// nFlags with shadowAdjustByCamera
			float32_t	zPersNear ;
			float32_t	zPersFar ;
			float32_t	zErrorPrecision ;		// default -12.0
			float32_t	zErrorSubPrecision ;	// default -12.0
			float32_t	fpFilterGauss ;			// default 2.0
			uint32_t	nCascadeMaxCount ;		// default 0
			float32_t	zCascadeDensityStep ;		// default 3.0
			float32_t	zCascadeTargetLengthStep ;	// default 2.0
			float32_t	zCascadeTargetLengthOffset ;// default 20.0
			float32_t	zCascadeDepthStep ;			// default 1.0
			float32_t	zCascadeDistanceOffsetStep ;// default 0.0
		} ;
		class	ShadowMapBuffer
		{
		public:
			enum	ConstantValue
			{
				maxCascadeShadowmap	= 8,
			} ;
			ShadowMapParam		m_smpShadowMap ;
			S3DShadowMapInfo	m_smiShadowMapInfo[maxCascadeShadowmap] ;
			SGLImage			m_imgShadowColor ;
			SGLImage			m_imgShadowDepth ;
			SGLImage			m_imgShadowDepthBuf ;
			size_t				m_nRenderedShadowmap ;

		public:
			// 構築関数
			ShadowMapBuffer( void ) ;
			// シャドウマップ・パラメータ設定
			static SGLError SetShadowMappingInfo
				( S3DRenderContextInterface& renderForShadowmap,
					S3DShadowMapInfo& smi,
					const ShadowMapParam& smp,
					const S3DLightEntry& light,
					size_t iCascade, const SGLSize& sizeImage,
					const S3DDVector& vCameraTarget,
					const S3DDVector& vCameraPos ) ;
		} ;

		// 光源アイテム
		class	Light	: public Item, public ShadowMapBuffer
		{
		public:
			S3DLightEntry	m_light ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Light, Item )
			// 構築関数
			Light( void ) ;
			// 消滅関数
			virtual ~Light( void ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// カメラアイテム
		class	Camera	: public Item
		{
		public:
			enum	CameraBehaviorFlag
			{
				cameraForceHFOV		= 0x0001,	// カメラ固有視野角を有効にする
				cameraHasParallax	= 0x0002,	// 視差情報を保有
			} ;
			uint32_t		m_nCameraFlags ;
			S3DDVector		m_vTarget ;			// 注視点（親空間座標）
			S3DDVector		m_vTop ;			// カメラ頂点ベクトル（親空間）
			double			m_degHorzFOV ;		// 水平視野角 [deg]
			StereoParallaxParam
							m_sppParallax ;		// 視差情報 (cameraHasParallax 時有効)
			S3DDVector		m_vLeftEarDir ;		// サウンド左耳向き
			S3DDVector		m_vRightEarDir ;	// サウンド右耳向き
			double			m_cosSoundLow ;		// 最小音量になる cosθ
			double			m_fpLowVolume ;		// 最小音量
			SSystem::SPtrSortObjectArray
				<SGLSecondaryViewProducer,StereoParallaxParam>
							m_psoaViewParallax ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Camera, Item )
			// 構築関数
			Camera( void ) ;
			// カメラ視野角から投影スクリーン距離を計算する
			bool CalcProjectionZbyFOV
				( float32_t& zScreen, const SGLSize& sizeView ) const ;
		public:
			// カメラ視点
			virtual const S3DDVector& GetCameraPosition( void ) const ;
			virtual void SetCameraPosition( const S3DDVector& vPos ) ;
			// カメラ注視点
			virtual const S3DDVector& GetCameraTarget( void ) const ;
			virtual void SetCameraTarget( const S3DDVector& vTarget ) ;
			// カメラ頂点ベクトル
			virtual const S3DDVector& GetCameraTop( void ) const ;
			virtual void SetCameraTop( const S3DDVector& vTop ) ;
			// カメラ視野空間への変換行列を計算する (matCamera * x - vCamera)
			void CalcCameraMatrix( S3DDMatrix& matCamera, S3DDVector& vCamera ) const ;
		public:
			// 視差情報を取得する
			const StereoParallaxParam * GetParallaxOfView
						( SGLSecondaryViewProducer * psvp ) const ;
			// 視差情報を設定する
			void SetParallaxOfView
				( SGLSecondaryViewProducer * psvp,
							const StereoParallaxParam& spp ) ;
			// 視差情報を削除する
			void RemoveParallaxOfView( SGLSecondaryViewProducer * psvp ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// モデルデータアイテム
		class	ModelItem	: public Item, public LocalShader
		{
		protected:
			S3DVertexBufferInterface *	m_pModel ;
			S3DVertexBufferInterface *	m_pCollision ;
			uint64_t					m_flagsModelExclusion ;
			uint64_t					m_flagsModelRequest ;
			size_t						m_iModelViewFirst ;
			ssize_t						m_iModelViewEnd ;
			bool						m_flagCollision ;
			bool						m_flagLocalBorder ;
			uint32_t					m_maskColliderFlags ;
			OffsetBorderParam			m_borderParam ;
			S3DCollision				m_collisionMesh ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2( ModelItem, Item, LocalShader )
			// 構築関数
			ModelItem( void ) ;
			// モデルデータ関連付け
			virtual void AttachModel( S3DVertexBufferInterface * pModel ) ;
			virtual void AttachCollisionModel
				( S3DVertexBufferInterface * pColModel, bool fBuildCollision = true ) ;
			// 当たり判定モデル構築
			virtual void BuildCollisionMesh( void ) ;
			// 当たり判定モデル解放
			void ReleaseCollisionMesh( void ) ;
			// 表示除外フラグ（表面属性フラグ）
			void SetModelExclusionFlags( uint64_t flagsExclusion ) ;
			uint64_t GetModelExclusionFlags( void ) const ;
			// 表示選択フラグ（表面属性フラグ）
			void SetModelRequestFlags( uint64_t flagsRequest ) ;
			uint64_t GetModelRequestFlags( void ) const ;
			// 表示メッシュ
			void SetViewModelMeshRange( size_t iFirst = 0, ssize_t iEnd = -1 ) ;
			void GetViewModelMeshRange( size_t& iFirst, ssize_t& iEnd ) const ;
			// 当たり判定ユーザーフラグ集合
			void SetColliderUserFlags( uint32_t nFlags ) ;
			uint32_t GetColliderUserFlags( void ) const ;
			// モデルデータ取得
			S3DVertexBufferInterface * GetModel( void ) const ;
			// 衝突判定用モデルデータ取得
			S3DVertexBufferInterface * GetCollisionModel( void ) const ;
		public:
			// レンダリングの為のデバイスリソース準備
			virtual void PrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// 当たり判定追加
			virtual void RenderLocalCollision
				( const S3DScene& scene, S3DCollision& render ) ;
			// 表示モデル追加
			virtual void RenderLocalModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
			static void RenderLocalModelWithReq
				( S3DRenderContextInterface& render,
					S3DVertexBufferInterface * pModel,
					uint64_t flagsRegFlags, uint64_t flagsExclusion,
					size_t iFirstMesh = 0, ssize_t iEndMesh = -1,
					size_t nInstancingCount = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pclrInstancing = NULL ) ;
		public:
			// 固有ボーダーパラメータ
			void SetOffsetBorderParameter
				( const OffsetBorderParam& borderParam,
								bool fEnableLocalBorder = true ) ;
			bool GetOffsetBorderParameter
					( OffsetBorderParam& borderParam ) const ;
			void EnableLocalBorderParameter( bool fEnable ) ;
			bool IsEnabledLocalBorderParameter( void ) const ;
		public:
			// シェーダー設定
			struct	ShaderSaver
			{
				uint64_t	flagsShading ;
				uint64_t	flagsLocalShading ;
			} ;
			void PrepareShaderSettings
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					ShaderSaver& ss ) const ;
			void RestoreShaderSettings
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					const ShaderSaver& ss ) const ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// ビルボードアイテム
		class	BillboardItem	: public Item
		{
		protected:
			S3DMeshShaper					m_meshShaper ;
			S3DMeshShaper::BillboardParam	m_bp ;
			uint64_t						m_flagsZBuf ;
			SSystem::SArray<S3DVector4>		m_bufPoints ;
			SSystem::SArray<size_t>			m_bufFrames ;
			SSystem::SArray<S3DColor>		m_bufColors ;
			SSystem::SArray<float32_t>		m_bufZooms ;
			SSystem::SArray<float32_t>		m_bufXAspects ;
			SSystem::SArray<S4DVector>		m_bufFaceDirs ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( BillboardItem, Item )
			// 構築関数
			BillboardItem( void ) ;
			// 表示画像関連付け
			void AttachImage
				( SGLImageObject * pImage,
					size_t iFrame = 0, uint64_t flagsShading = 0,
					double xCenter = 0.0, double yCenter = 0.0,
					double xZoom = 1.0, double yZoom = 1.0,
					double zAngle = 0.0, double zBias = 0.0 ) ;
			// パーティクル表示設定
			void AttachParticle
				( const S3DMeshShaper::BillboardParam& bp,
					uint64_t flagsShading, size_t nCount,
					const S3DVector4 * pvPoints,
					const size_t * pFrames = NULL,
					const S3DColor * pColors = NULL,
					const float32_t * pZooms = NULL,
					const S4DVector * pFaceDirs = NULL,
					const float32_t * pxAspect = NULL ) ;
			// パーティクル追加
			void AddParticle
				( size_t nCount,
					const S3DVector4 * pvPoints,
					const size_t * pFrames = NULL,
					const S3DColor * pColors = NULL,
					const float32_t * pZooms = NULL,
					const S4DVector * pFaceDirs = NULL,
					const float32_t * pxAspect = NULL ) ;
		protected:
			void FitColorArray( size_t nLength ) ;
			void FitZoomArray( size_t nLength ) ;
			void FitAspectArray( size_t nLength ) ;
			void FitFaceDirArray( size_t nLength ) ;
		public:
			// パーティクルリセット
			void ClearAllParticles( void ) ;
			// パラメータ
			const S3DMeshShaper::BillboardParam&
							GetBillboardParam( void ) const
			{
				return	m_bp ;
			}
			// 描画フラグ
			uint64_t GetShadingDrawFlags( void ) const
			{
				return	m_flagsZBuf ;
			}
			// ｚ描画フラグ
			uint64_t GetZBufferDrawFlags( void ) const
			{
				return	m_flagsZBuf & (shadingNoZBuffer | shadingZBufferNoWrite) ;
			}
		public:
			// レンダリングの為のデバイスリソース準備
			virtual void PrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// 表示モデル追加
			virtual void RenderLocalModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// サウンドアイテム
		class	SoundItem	: public Item
		{
		public:
			enum	SoundFlag
			{
				soundDirectional	= 0x0001,	// 指向性音源
				soundEnvironment	= 0x0002,	// 環境音源（距離と方向による音量効果無効）
				soundFadeReach		= 0x0004,	// 有効距離外で音量フェードアウト
			} ;
		protected:
			SSystem::SSmartReference<SGLAudioPlayerInterface>
							m_refPlayer ;
			uint32_t		m_nSoundFlags ;			// enum SoundFlag
			double			m_fpVolume ;			// 音量
			double			m_fpFadeReach ;			// 有効距離
			double			m_fpFadeLatitude ;		// 有効距離外フェードアウト幅
			double			m_fpAttenuationPower ;	// 減衰率(x)＝(d/r)^x, 0 の時減衰無し
			double			m_fpAttenuationDistance ;
			S3DDVector		m_vDirection ;			// 指向性
			double			m_fpConeAngle ;			// 範囲 cosθ
			double			m_fpAngleGradation ;	// ぼかし範囲 Δcosθ
													// 範囲θ＝acos(m_fpConeAngle + m_fpAngleGradation)
			double			m_fpBaseVolume ;		// 指向性範囲外音量
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( SoundItem, Item )
			// 構築関数
			SoundItem( void ) ;
		public:
			// AudioPlayer 設定
			void AttachAudioPlayer( SGLAudioPlayerInterface * pPlayer ) ;
			void SetSmartAudioPlayer( SGLAudioPlayerInterface * pPlayer ) ;
			// AudioPlayer 取得
			SGLAudioPlayerInterface * GetAudioPlayer( void ) const ;
			// フラグ
			uint32_t GetSoundFlags( void ) const ;
			void SetSoundFlags( uint32_t nFlags ) ;
			// 音量
			void SetVolume( double fpVolume ) ;
			double GetVolume( void ) const ;
			// 有効距離
			void SetFadeReach( double fpReach, double fpLatitude ) ;
			double GetFadeReach( void ) const ;
			double GetFadeLatitude( void ) const ;
			// 距離減衰率
			void SetAttenuationPower
					( double fpPower, double fpDistance = 1.0 ) ;
			double GetAttenuationPower( void ) const ;
			double GetAttenuationDistance( void ) const ;
			// 指向性
			void SetDirection
				( const S3DDVector& vDir,
					double degCone = 0.0,
					double degGrad = 180.0, double volBase = 0.1 ) ;
			const S3DDVector& GetDirection( void ) const ;
			double GetDirectionConeAngle( void ) const ;
			double GetDirectionGradation( void ) const ;
			double GetDirectionBaseVolume( void ) const ;
			// 音量反映
			void ApplySoundVolume( Camera * pCamera ) const ;
			void ApplySoundVolume
				( const S3DDMatrix& matCamera,
					const S3DDVector& vCamera,
					const S3DDMatrix& matItem,
					const S3DDVector& vItem,
					Camera * pCamera, double fpItemVol,
					SGLAudioPlayerInterface * pPlayer ) const ;
			// 音量効果計算（このアイテムのパラメータと引数のみを使用）
			void CalcSoundVolume
				( float32_t vols[],
					const S3DDMatrix& matCamera,
					const S3DDVector& vCamera,
					const S3DDMatrix& matItem,
					const S3DDVector& vItem,
					Camera * pCamera = nullptr, double fpItemVol = 1.0 ) const ;
			void CalcSoundVolume
				( float32_t vols[],
					Camera * pCamera,
					const S3DDMatrix& matItem,
					const S3DDVector& vItem,
					double fpItemVol = 1.0 ) const ;
		public:
			// タイマ処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;

		// 物理演算シーンアイテム
		class	PhysicsSceneItem	: public Item, public S3DPhysicsScene
		{
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2( PhysicsSceneItem, Item, S3DPhysicsScene )
		} ;

		// 抽象効果
		class	Effector	: public SSystem::SObject,
								public DrawLayerEffector
		{
		protected:
			int	m_priority ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Effector, SObject )
			// 構築関数
			Effector( int priority = 0 ) ;
			// 処理優先度
			virtual int GetDrawingPriority( void ) const ;
			void SetDrawingPriority( int priority ) ;
		public:
			// 時間経過処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		} ;
		// レイヤード効果描画
		class	LayeredEffector	: public SSystem::SObject
		{
		protected:
			double	m_application ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( LayeredEffector, SObject )
			// 構築関数
			LayeredEffector( void ) ;
		public:
			// 適用度
			virtual double GetEffectApplication( void ) const ;
			virtual void SetEffectApplication( double apply ) ;
			// 時間経過処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// 描画処理
			virtual void DrawLayer
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		} ;
		// メッシュ変形子
		class	MeshOperator	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MeshOperator, ESLObject )
			// 構築関数
			MeshOperator( void ) ;
			// 座標変位
			virtual void WarpMesh
				( S3DVector4 * pvDst, S3DColor * pColor,
					const S2DVector * pvSrc, const SGLSize& sizeMesh ) ;
			virtual void WarpVertex
				( S3DVector4 * pvDst,
					S3DColor * pColor, const S2DVector * pvSrc,
					const SGLSize& sizeMesh, size_t iVertex ) ;
		} ;
		// メッシュ変形レイヤード効果描画
		class	LayeredMeshEffector	: public LayeredEffector
		{
		protected:
			S3DMaterial					m_material ;
			SGLSize						m_sizeMesh ;
			SSystem::SArray<S2DVector>	m_vSrcMesh ;
			SSystem::SArray<S3DVector4>	m_vDstMesh ;
			SSystem::SArray<S3DColor>	m_vDstColor ;
			SSystem::SArray<uint32_t>	m_aIndexList ;
			SSystem::SObjectArray<MeshOperator>
										m_operators ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( LayeredMeshEffector, LayeredEffector )
			// 構築関数
			LayeredMeshEffector( void ) ;
			// メッシュ分割数設定
			void SetMeshDivision( const SGLSize& sizeDivMesh ) ;
			// メッシュ変形子追加
			void AddOperator( MeshOperator * pOperator ) ;
		public:
			// 描画処理
			virtual void DrawLayer
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		} ;

		// 非同期処理・同期用
		class	SyncItemDispatcher
		{
		private:
			SSystem::SCriticalSection	m_csSync ;
			atomic_int_t				m_nDispatched ;
			SSystem::SSignalEvent		m_signalDone ;
		public:
			SyncItemDispatcher( void )
				: m_nDispatched( 0 )
			{
				m_signalDone.Initialize( true ) ;
			}
			int GetDispatched( void ) const
			{
				return	(int) m_nDispatched ;
			}
			void AddDispatch( void )
			{
				m_csSync.Lock() ;
				if ( ++ m_nDispatched == 1 )
				{
					m_signalDone.ResetSignal() ;
				}
				m_csSync.Unlock() ;
			}
			void ReleaseDispatch( void )
			{
				m_csSync.Lock() ;
				ESLAssert( m_nDispatched > 0 ) ;
				if ( -- m_nDispatched == 0 )
				{
					m_signalDone.SetSignal() ;
				}
				m_csSync.Unlock() ;
			}
			SSystem::SError Sync( int64_t msecTimeout = SSystem::SSynchronismInterface::Infinite )
			{
				return	m_signalDone.Wait( msecTimeout ) ;
			}
		} ;

	protected:
		// 非同期処理
		class	AsyncItemDispatcher
		{
		public:
			SSystem::SPointerArray<SyncItemDispatcher>
									m_aSyncDisp ;
			S3DScene *				m_pScene ;
			Space *					m_pSpace ;
			Item *					m_pItem ;
			bool					m_flagSpaceVisisble ;
			uint32_t				m_msecTimer ;
			ItemClass				m_clsItem ;

			typedef	void (AsyncItemDispatcher::*PFUNC_DISPATCH)( void ) ;
			PFUNC_DISPATCH			m_pfnDispatch ;

		public:
			AsyncItemDispatcher( void )
				: m_pScene( NULL ),
					m_pSpace( NULL ), m_pItem( NULL ),
					m_flagSpaceVisisble( false ),
					m_msecTimer( 0 ), m_clsItem( classCamera ) { }
			// 関数呼び出し
			void DispatchOnTimer( void ) ;
			void DispatchUpdateBehaviorFlags( void ) ;
			void DispatchOnRenderEvent( void ) ;
		} ;

		friend class AsyncItemDispatcher ;

		SSystem::STimeCounter						m_timerAsyncDisp ;
		double										m_msecAsyncDispTime ;
		atomic_int_t								m_nAsyncDispatching ;
		bool										m_flagEnteredTimer ;
		SSystem::SCriticalSection					m_csDispStock ;
		SSystem::SObjectArray<SyncItemDispatcher>	m_aSyncDispStock ;
		SSystem::SObjectArray<AsyncItemDispatcher>	m_aAsyncDispStock ;
		SSystem::SObjectArray<AsyncItemDispatcher>	m_aPendingDispatch ;
		SyncItemDispatcher							m_syncItemDispatcher ;
		atomic_int_t								m_nAsyncThreadRunning ;
		SSystem::SSignalEvent						m_signalNoAsyncThreads ;
		size_t										m_nAsyncThreadLimit ;

		class	ThreadSyncDispatcher
					: public SSystem::SPointerArray<SyncItemDispatcher>
		{
		public:
		} ;

		SSystem::SSortObjectArray
			< SSystem::SSortObjectElement
				< SSystem::SThread::IdType, ThreadSyncDispatcher > >
													m_soaAsyncThreads ;

		class	SpaceDelayRemoveProc
					: public ESLObject, public SSystem::SProcedure
		{
		public:
			SSystem::SObjectArray<ESLObject>	m_aDelayRemove ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( SpaceDelayRemoveProc, ESLObject, SProcedure )
			// 構築関数
			SpaceDelayRemoveProc
				( SSystem::SObjectArray<ESLObject>& aDelayRemove ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;

	public:
		// 非同期処理スレッド数（0 の時、非同期処理無効）
		size_t GetAsyncThreadLimit( void ) const ;
		void SetAsyncThreadLimit( size_t nCount ) ;
		// 非同期処理開始
		SGLError BeginAsyncDispatcher( void ) ;
		// 非同期処理終了（全処理完了同期）
		SGLError EndAsyncDispatcher( void ) ;
		// 全処理完了同期
		SGLError FenceAsyncDispatcher( void ) ;
		// 最後の BeginAsyncDispatcher～EndAsyncDispatcher 区間の経過時間 [ms] を取得
		double GetLastAsyncDispatchingDuration( void ) const ;
		// 同期オブジェクト取得
		SyncItemDispatcher * GetSyncDispatcher( void ) ;
		// 同期オブジェクト完了待ちと解放
		void ReleaseSyncDispatcher( SyncItemDispatcher * pSync ) ;
		// 現在のスレッドに同期オブジェクト追加
		SyncItemDispatcher * AddCurrentThreadSyncDispatcher( void ) ;
		// 現在のスレッドの同期オブジェクト完了待ちと解放
		void ReleaseCurrentThreadSyncDispatcher( SyncItemDispatcher * pSync ) ;
		// 非同期実行
		void AsyncDispatchTimer
			( Space * pSpace, uint32_t msecPast, SyncItemDispatcher * pSync ) ;
		void AsyncDispatchUpdateBehaviorFlags
			( Space * pSpace, bool flagSpaceVisible, SyncItemDispatcher * pSync ) ;
		void AsyncDispatchRenderEvent
			( Space * pSpace, ItemClass clsItem, SyncItemDispatcher * pSync ) ;

	protected:
		// AsyncItemDispatcher に同期オブジェクト設定
		void AttachSyncItemDispatcher
			( AsyncItemDispatcher * pDisp, SyncItemDispatcher * pSync ) ;
		// スレッド起動
		void RunAsyncDispatcherThread( void ) ;
		// 非同期ディスパッチャー取得
		AsyncItemDispatcher * GetAsyncItemDispatcher( void ) ;
		// 非同期処理実行
		void DispatchAsyncProc( AsyncItemDispatcher * pDisp ) ;
		// 非同期処理スレッド関数
		static void AsyncDispatcherThreadProc( void * pInstance ) ;
		void AsyncDispatcherProc( void ) ;
		// 処理ディスパッチ
		void FlushDispatchedAsyncProc( void ) ;

	protected:
		// 衝突判定オブジェクト
		bool				m_fUpdateFieldColl ;
		bool				m_fUpdatedCollision ;
		S3DCollision		m_colField ;
		S3DCollision		m_colItems ;

		// 物理演算
		PhysicsSceneItem	m_physScene ;
		S3DPhysicsScene::ErrorGap
							m_errGapPhys ;
		bool				m_flagManualPhysTime ;
		float32_t			m_secPhysAdvance ;

		SSystem::STimeCounter	m_timerPhys ;
		double					m_msecMaxPhysProcess ;
		double					m_msecAccPhysProcess ;
		size_t					m_nPhysProcess ;

		// レンダリングオブジェクト
		S3DRenderContext			m_render ;
		S3DRenderContext			m_renderEffect ;
		S3DRenderContext			m_renderSampler ;
		S3DRenderDevice *			m_pRenderDevice ;
		SSystem::SCriticalSection	m_csRender ;

		// 描画先情報
		SGLImageObject *	m_pTargetColor ;
		SGLImageObject *	m_pTargetColorSampler ;
		SGLImageObject *	m_pTargetDepth ;
		SGLImageObject *	m_pTargetDepthSampler ;
		SGLImageRect		m_rectTargetClip ;
		SGLImageRect		m_rectCurrentTargetClip ;

		SSystem::SPointerArray<SGLImageObject>
							m_aMultiTargets ;		// 2つ目以降の描画のサンプラー用
		SSystem::SPointerArray<SGLImageObject>
							m_aMultiDrawTargets ;	// 2つ目以降の描画先
		SGLImageObject *	m_pTargetLayered ;		// レイヤード描画サンプリング用
		SGLImageObject *	m_pDrawTargetLayered ;	// レイヤード描画先バッファ
		SGLImageObject *	m_pTargetLayeredDepth ;	// レイヤードｚバッファサンプリング用
		SGLImageObject *	m_pDrawTargetLayeredDepth ;	// レイヤード描画先ｚバッファ
		SSystem::SPointerArray<SGLImageObject>
							m_aTempRenderBufs ;	// 中間処理用バッファ
		uint32_t			m_maskRenderRequirement ;
		uint32_t			m_maskCurrentRenderRequirement ;

		SGLImageObject *	m_pTargetLeft ;
		SGLImageObject *	m_pTargetLeftSampler ;
		double				m_xParallax ;
		double				m_zParallaxFocus ;
		double				m_xParallaxScreenDelta ;
		bool				m_fParallaxParam ;
		ParallaxParam		m_parallaxParam[2] ;

		// 効果用中間バッファ情報
		struct	EffectInternalInfo
		{
			S3DRenderContextInterface *	pTempRenderer ;
			bool						fLocalRenderer ;
			bool						fCommonDepth ;
			size_t						nTargetColors ;
			SGLImageObject *			pTargetColor[8] ;
			SGLImageObject *			pEffectColor[8] ;
			SGLImageObject *			pTargetDepth ;
			SGLImageObject *			pEffectDepth ;

			EffectInternalInfo( void )
				: pTempRenderer( nullptr ),
					fLocalRenderer( false ),
					fCommonDepth( false ),
					nTargetColors( 1 ),
					pTargetDepth( nullptr ),
					pEffectDepth( nullptr )
			{
				for ( int i = 0; i < 8; i ++ )
				{
					pTargetColor[i] = nullptr ;
					pEffectColor[i] = nullptr ;
				}
			}
		} ;

		// 効果（背景～動的アイテム（レイヤード効果含まず））
		SSystem::SReferenceArray<Effector>			m_raEffector ;
		SSystem::SPointerArray<DrawLayerEffector>	m_aEffector ;

		// 後効果
		SSystem::SReferenceArray<Effector>			m_raPostEffector ;
		SSystem::SPointerArray<DrawLayerEffector>	m_aPostEffector ;

		// レイヤード効果
		SSystem::SSmartReference<LayeredEffector>	m_refLayerEffector1 ;
		SSystem::SSmartReference<LayeredEffector>	m_refLayerEffector2 ;

		SSystem::SCriticalSection					m_csEffectorSync ;

	protected:
		// 透視変換情報
		ProjectionParam		m_projParam ;
		ProjectionParam		m_projCurrent ;

		// シェーディング方法
		uint32_t			m_typeShading ;

		// メイン画面（VR 表示などの際、直接ステレオ化されるか？）
		bool				m_fMainRenderScene ;

		// 背景色
		bool				m_fFillBack ;
		SGLPalette			m_rgbaFillBack ;

		// 有効表示距離
		double				m_fpVisibleNearDistance ;	// itemHideNear で非表示にする距離
		double				m_fpVisibleFarDistance ;	// itemHideFar で非表示にする距離

		// 大域疑似フォッグ
		bool				m_fEnableFog ;
		FogParam			m_fogParam ;

		// 大域環境マッピング
		EnvMappingParam		m_envMapParam ;

		// 動的環境マッピング
		bool				m_flagDynEnvMap ;		// 有効フラグ
		bool				m_flagDynEnvUpdate ;	// 更新フラグ (SetUpdateDynamicEnvironment で設定)
		bool				m_flagDynEnvManual ;	// SetUpdateDynamicEnvironment 呼出時のみ更新
		DynamicEnvironment	m_envDynParam ;
		SGLImageObject *	m_pWorkDynMapCube ;		// Cube マッピング用ターゲット
		SGLImageObject *	m_pWorkDynMapZBuf ;		// Cube レンダリング用ｚバッファ

		// レイヤード効果レンダリング時の大域環境マッピング設定
		EnvironmentMappingSource
							m_emsReflectionSource[classCount] ;
		EnvironmentMappingSource
							m_emsRefractionSource[classCount] ;

		// 輪郭線
		OffsetBorderParam	m_borderParam ;

		// ルート空間
		Space				m_spaceRoot ;

		// カメラと光源
		Camera *						m_pFirstCamera ;
		SSystem::SArray<S3DLightEntry>	m_bufLights ;
		SSystem::SPointerArray<Light>	m_ptaLights ;
		SSystem::SArray<uint32_t>		m_aShadhowMapLIds ;

		double		m_fpDazzlement ;
		double		m_fpLastDazzlement ;
		double		m_fpLightSensitivity ;
		double		m_fpAmbientLightSensitivity ;

		S3DCustomShader *	m_pShadowmapFilterShader ;
		bool				m_fUnsupportedShadowmapFilter ;

		// メインカメラ（デフォルトに使用するカメラ）
		SSystem::SSmartReference<Camera>	m_refMainCamera ;

		// サウンド用カメラ
		SSystem::SSmartReference<Camera>	m_refSoundCamera ;

		// レンダリング中のカメラ情報
		Camera *	m_pCurrentCamera ;
		S3DDMatrix	m_matCameraSpace ;
		S3DDMatrix	m_matCamera ;
		S3DDVector	m_vCameraTrans ;
		S3DDMatrix	m_matICamera ;
		//
		S3DDVector	m_vCameraTarget ;
		S3DDVector	m_vCameraPos ;
		S3DDVector	m_vCameraTop ;

		bool		m_fRenderingOffset ;
		S3DDVector	m_vRenderingOffset ;

		// レンダリング中の透視変換行列
		S4DMatrix	m_mat4CurrentPerspective ;

		// レイヤー描画空間エントリ
		SSystem::SPointerArray<Space>	m_aLayeredSpaces ;

		// 現在のレンダリングフェーズ
	public:
		enum	RenderingStage
		{
			renderingMain,
			renderingShadow,
			renderingEnvironment,
			renderingMultiPass0,
		} ;
	protected:
		RenderingStage	m_rsCurrentRenderingStage ;
		RenderContext::StereoViewIndex
						m_sviStereoViewTarget ;
		ItemClass		m_classCurrentRendering ;
		uint32_t		m_maskCurrentCollision ;

		// レンダラ設定
		uint32_t	m_nRenderingBufferSize ;

		// 非同期処理キュー
		SSystem::SProcedureQueue	m_queAsyncProc ;
		bool						m_enabledAsyncProc ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DScene, S3DCollider )
		// 構築関数
		S3DScene( void ) ;
		// 消滅関数
		virtual ~S3DScene( void ) ;

	public:
		// ルート空間
		Space& GetRootSpace( void )
		{
			return	m_spaceRoot ;
		}
		// ルート空間へ子空間追加（静的なコライダ追加も）
		void AddSceneSpace( Space * pSpace, uint32_t nOptFlags = 0 ) ;
		// ルート空間から空間削除（分離）
		void DetachSceneSpace( Space * pSpace ) ;

	public:
		// レンダリングデバイスの設定
		virtual SGLError SetRenderDevice( S3DRenderDevice * pDevice ) ;
		// 描画先設定
		enum	RenderTargetIndex
		{
			renderTargetComposed	= S3DRenderDevice::renderTargetComposed,	// シェーディング結果
			renderTargetEmission	= S3DRenderDevice::renderTargetEmission,	// 発光成分（ポストエフェクト用）
			renderTargetNormal		= S3DRenderDevice::renderTargetNormal,		// 法線（遅延レンダリング用）
			renderTargetDiffusion	= S3DRenderDevice::renderTargetDiffusion,	// 拡散反射成分（遅延シェーディング用）
			renderTargetAmbient		= S3DRenderDevice::renderTargetAmbient,		// 環境光成分（SSAO用）
			renderTargetSpecular	= S3DRenderDevice::renderTargetSpecular,	// 鏡面反射成分（遅延シェーディング用）
			renderTargetCount		= S3DRenderDevice::renderTargetCount,
		} ;
		void AttachRenderTarget
			( SGLImageObject * pImage,
				SGLImageObject * pDepth,
				const SGLImageRect * pRect = nullptr,
				SGLImageObject * pImageForSampler = nullptr,
				SGLImageObject * pDepthForSampler = nullptr ) ;
		void AttachStereoTargetLeft
			( SGLImageObject * pImageLeft,
				double xParallax, double zFocusRatio,
				SGLImageObject * pImageLeftForSampler = nullptr ) ;
		void AttachMultiRenderTarget
			( SGLImageObject*const* ppImages, size_t nImages,
				SGLImageObject*const* ppForSampler = nullptr ) ;
		void AttachLayeredRenderTarget
			( SGLImageObject * pImageLayered,
				SGLImageObject * pDepthLayered,
				SGLImageObject * pImageLayeredForSampler = nullptr,
				SGLImageObject * pDepthLayeredForSampler = nullptr ) ;
		void AttachTemporaryRenderBuffers
			( SGLImageObject*const* ppImages, size_t nImages ) ;
		SGLImageObject * GetTemporaryRenderBuffer( size_t i ) const ;
		void RequireRenderTargetMask( uint32_t maskRT ) ;
		uint32_t GetRenderTargetRequirementMask( void ) const ;
		// 透視変換
		virtual void GetProjection( ProjectionParam& projParam ) const ;
		virtual void GetCurrentProjection( ProjectionParam& projParam ) const ;
		virtual SGLError GetProjectionOfView
			( ProjectionParam& projParam, SGLSecondaryViewProducer * psvp ) const ;
		virtual void SetProjection( const ProjectionParam& projParam ) ;
		// 視差設定
		virtual void SetParallax
			( double xParallax,
				double zFocusRate = 1.0, double xScreenDelta = 0.0 ) ;
		bool GetParallax
			( double& xParallax, double& zFocusRate, double& xScreenDelta ) const ;
		virtual void SetParallaxParam
			( const ParallaxParam& ppRight, const ParallaxParam& ppLeft ) ;
		bool GetParallaxParam
			( ParallaxParam& ppRight, ParallaxParam& ppLeft ) const ;
		// シェーディング方法
		uint32_t GetShadingMethod( void ) const ;
		void SetShadingMethod( uint32_t typeShading ) ;
		// メイン画面設定（※VR表示する際に必須ではないが一応設定しておく）
		void SetMainRenderSceneFlag( bool fMainScene ) ;
		bool IsMainRenderScene( void ) const ;
		// 背景色
		bool GetBackColor( SGLPalette& rgbaBack ) const ;
		void SetBackColor
			( const SGLPalette& rgbaBack, bool fFillBack = true ) ;
		// レンダリングオフセット（精度向上）を有効／禁止にする
		void EnableRenderingOffset( bool fDynamicOffset ) ;
		// 有効表示距離
		double GetVisibleNearDistance( void ) const ;
		double GetVisibleFarDistance( void ) const ;
		void SetVisibleDistance( double fpNear, double fpFar ) ;
		// 大域疑似フォッグ
		bool GetGlobalFog( FogParam& fogParam ) const ;
		void SetGlobalFog( const FogParam& fogParam, bool fFog = true ) ;
		void EnableGlobalFog( bool fFog ) ;
		// 大域環境マッピング
		void GetEnvironmentMapping( EnvMappingParam& envMapParam ) const ;
		void SetEnvironmentMapping( const EnvMappingParam& envMapParam ) ;
		// 大域環境マッピングソース
		void SetEnvironmentMappingSource
			( ItemClass clsItem,
				EnvironmentMappingSource emsReflection,
				EnvironmentMappingSource emsRefraction ) ;
		void GetEnvironmentMappingSource
			( ItemClass clsItem,
				EnvironmentMappingSource& emsReflection,
				EnvironmentMappingSource& emsRefraction ) const ;
		// 動的環境マッピング設定
		void EnableDynamicEnvironment( bool flafDynEnv ) ;
		bool IsEnabledDynamicEnvironment( void ) const ;
		void AttachDynamicEnvironmentTarget
			( SGLImageObject * pCubeTarget, SGLImageObject * pDepth ) ;
		bool GetDynamicEnvironment( DynamicEnvironment& dynEnvMap ) const ;
		void SetDynamicEnvironment
			( const DynamicEnvironment& dynEnvMap, bool flagManualUpdate ) ;
		// 動的環境マッピング更新設定
		void SetUpdateDynamicEnvironment( void ) ;
		// 輪郭線 (shaderForceBorder) 設定
		void GetOffsetBorderParameter( OffsetBorderParam& borderParam ) const ;
		void SetOffsetBorderParameter( const OffsetBorderParam& borderParam ) ;
		// レンダリングバッファサイズ設定
		void SetRenderingBufferSize( uint32_t countVertex ) ;

	public:
		// カメラ位置に対する光源の輝度取得
		double GetLastDazzlement( void ) const ;
		// カメラ位置に対する光源の輝度を加算（フレーム内積算／OnUpdateBehavior, or classPreRender 内から呼び出し）
		void AddLightDazzlement
			( const S3DDMatrix& matSpace,
				const S3DDVector& vSpace, const S3DLightEntry& light ) ;
		// 光源輝度感度を設定 (classPreRender, classPreRender2 で設定)
		void SetLightSensitivity( double fpLight, double fpAmbient = 1.0 ) ;
		// 光源輝度感度を取得
		double GetLightSensitivity( void ) const ;
		double GetAmbientLightSensitivity( void ) const ;

	public:
		// 画面効果設定（classLayeredSpace 後に処理）
		void AddEffect( Effector * pEffector ) ;
		void AddSmartEffect( Effector * pEffector ) ;
		// 画面効果設定（全ての描画の後に処理）
		void AddPostEffect( Effector * pEffector ) ;
		void AddSmartPostEffect( Effector * pEffector ) ;
		// 画面効果削除
		void RemoveEffect( Effector * pEffector ) ;
		void RemoveAllEffects( void ) ;
		void RemovePostEffect( Effector * pEffector ) ;
		void RemoveAllPostEffects( void ) ;
		// レイヤード効果設定
		void AttachLayeredEffect
			( ItemClass classItem, LayeredEffector * pEffector ) ;
		void SetSmartLayeredEffect
			( ItemClass classItem, LayeredEffector * pEffector ) ;
	public:	// 一時画面効果アイテム（１フレームのみ／OnUpdateBehavior, or classPreRender 内から呼び出し）
		// 一時画面効果追加（classLayeredSpace 後に処理）
		void AddTemporaryEffect( DrawLayerEffector * pEffector ) ;
		// 一時画面効果追加（全ての描画の後に処理）
		void AddTemporaryPostEffect( DrawLayerEffector * pEffector ) ;
	protected:
		// 常設エフェクタを一時エフェクタ配列に追加
		void AddAllEffectorToTemporary
			( SSystem::SPointerArray<DrawLayerEffector>& aTempEffect,
				const SSystem::SReferenceArray<Effector>& raEffect ) ;
		// 効果描画に必要なレンダーターゲットを収集
		static uint32_t CollectRequiredColorBuffers
			( SSystem::SPointerArray<DrawLayerEffector>& aTempEffect ) ;
		// 画面効果挿入位置検索
		static size_t OrderToAddEffect
			( const SSystem::SPointerArray<DrawLayerEffector>& aEffect, int nPriority ) ;

	public:
		// 非同期処理の開始
		SGLError BeginAsyncThread( void ) ;
		// 非同期処理の終了
		SGLError EndAsyncThread( void ) ;
		// 全ての非同期処理が完了するのを待つ
		SGLError WaitForAllAsyncProc
			( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		// 非同期処理にフェンスを設定する
		void SetFenceAsyncProcedure( void ) ;
		// 非同期処理を追加する
		void PostAsyncProcedure
			( SSystem::SProcedure * pProc,
				SSystem::SSignalEvent * pDoneSignal = nullptr,
				bool flagAutoDelete = true, bool flagFence = false ) ;
		void PostAsyncFuncProcedure
			( SSystem::SProcedureCaller::PFUNC_PTR pfnProc,
				void * pProcInstance,
				SSystem::SSignalEvent * pDoneSignal = nullptr,
				bool flagFence = false ) ;

	public:
		// タイマー処理
		void OnTimer( uint32_t msecPast ) ;
		// レンダリング実行
		virtual SGLError RenderScene
			( Camera * pCamera,
				S3DRenderContextInterface * pRender = NULL ) ;
		// レンダリング前処理
		enum	PrepareRenderSceneOption
		{
			prepareSceneWithoutRender	= 0x0001,
		} ;
		virtual SGLError PrepareRenderScene
			( Camera * pCamera, uint64_t nOptFlags = 0 ) ;
		// レンダリング後処理
		virtual SGLError FinishRenderScene( void ) ;
		// レンダリング実行処理
		virtual void DoRenderScene
			( S3DRenderContextInterface * pRender = NULL,
				RenderContext::StereoViewIndex
					sviView = RenderContext::stereoViewAuto ) ;
		// レンダリングは実行せず、コリジョンの更新のみ実行
		void UpdateSceneCollision( uint64_t nOptFlags = 0 ) ;

	public:
		// classPreRenderFrame 時に任意カメラでレンダリングする
		void RenderSceneTemporary
			( S3DRenderContextInterface& render,
				Camera * pCamera, bool flagFillBack,
				uint64_t flagsExclusion = shadingNoReflectObject,
				uint64_t maskTargetClasses = classBitsAllScape | classBitsAllStaticItem ) ;
		// Cube マッピング用レンダリング
		void RenderSceneToCube
			( SGLImageObject * pCubeImage,
				SGLImageObject * pDepthBuffer,
				const S3DDVector& vCenterPos,
				uint64_t flagsExclusion = shadingNoReflectObject,
				uint64_t maskTargetClasses = classBitsAllScape | classBitsAllStaticItem ) ;
		// Shadowmap 用レンダリング
		size_t RenderSceneToShadowmap
			( ShadowMapBuffer& bufShadowmap,
				const S3DLightEntry& light, size_t nCascadeMaxCount ) ;
		// 準備済みの光源を設定
		void SetPreparedLight( S3DRenderContextInterface & render ) const ;
		// 効果用レンダリング出力先情報取得
		S3DRenderContextInterface *
			GetRenderTargetForEffect
				( EffectInternalInfo& eii,
					S3DRenderContextInterface& renderSrc ) ;
		// 効果用描画バッファをサンプラー用バッファにコピーする
		void SampleLayeredTargetForEffect
			( EffectInternalInfo& eii, S3DRenderContextInterface& render ) ;
		// シェーディング・描画設定
		void SetShadingConfig
			( S3DRenderContextInterface& render, bool fDisableEnv = false ) const ;
		// 大域環境マッピング設定
		void SetGlobalEnvironmentMapping
			( S3DRenderContextInterface& render,
				SGLImageObject * pViewport,
				SGLImageObject * pDepth,
				EnvironmentMappingSource emsReflection,
				EnvironmentMappingSource emsRefraction ) const ;
		// エフェクタ描画処理
		void RenderEffectors
			( S3DRenderContextInterface& renderDst,
				S3DRenderContextInterface& renderLayered,
				const SSystem::SPointerArray<DrawLayerEffector>& aEffect,
				EffectInternalInfo& eii, uint32_t maskReqBuffers ) ;

	public:
		// S3DPhysicsScene 取得
		PhysicsSceneItem& PhysicsScene( void ) const ;
		const PhysicsSceneItem& GetPhysicsScene( void ) const
		{
			return	m_physScene ;
		}
		// 物理演算当たり判定誤差設定
		void SetPhysicsErrorGap
			( const S3DPhysicsScene::ErrorGap& errGap ) ;
		// 物理演算時間を手動実行するか？
		// ※デフォルト（false）では OnTimer から AddPhysicsTime が呼び出される
		void SetManualPhysTimeFlag( bool flagManualTime ) ;
		// 物理演算時間進行
		void AddPhysicsTime( float32_t secPhysTime ) ;
		// 物理演算処理パフォーマンス取得
		struct	PhysicsPerformanceLog
		{
			size_t	countProcess ;		// 処理回数
			double	msecAccTime ;		// 合計処理時間 [ms]
			double	msecMaxTime ;		// 1回あたりの最大処理時間 [ms]
		} ;
		void GetPhysicsPerformanceLog( PhysicsPerformanceLog& ppl ) const ;
		// パフォーマンスログ・リセット
		void ResetPhysicsPerformanceLog( void ) ;

	public:
		// フィールドコリジョンの更新フラグ設定
		void SetUpdateFieldCollisionFlag( void ) ;
		// コリジョン更新済みフラグ
		bool IsCollisionUpdated( void ) const ;
		void ResetCollisionUpdatedFlag( void ) ;
		// アイテムと球との交差判定
		Item * IsItemHitAgainstSphere
			( const S3DDVector& vPos,
				float fpRadius, S3DCollision::Result& rsHit ) const ;
		Item * IsItemHitAgainstSphereInMeshNextPoly
			( const S3DDVector& vPos,
				float fpRadius, S3DCollision::Result& rsHit ) const ;
		// アイテムと線分との交差判定
		Item * IsItemSegmentCrossing
			( const S3DDVector& vPos0, const S3DDVector& vPos1,
				float fpErrorGap, S3DCollision::Result& rsCross ) const ;

	public:	// S3DCollider
		// 範囲取得
		virtual bool GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const ;
		// 凸形状の内側判定
		virtual bool IsSphereInclusive
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsIncluded ) const ;
		// 球との交差判定
		virtual bool IsHitAgainstSphere
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsHit ) const ;
		// 線分との交差判定
		virtual bool IsSegmentCrossing
			( const S3DDVector& vPos0, const S3DDVector& vPos1,
							float fpErrorGap, S3DCollisionResult& rsCross ) const ;
		// Collision 取得
		const S3DCollision& GetFieldCollision( void ) const
		{
			return	m_colField ;
		}
		const S3DCollision& GetItemCollision( void ) const
		{
			return	m_colItems ;
		}

	public:
		// 視線ベクトル計算
		void RayProjectionFor
			( S3DDVector& vRay, S3DDVector& vRayOrigin,
				const S2DDVector& vPosOnScreen, Camera * pCamera ) const ;
		// 平面投影座標計算
		const S2DDVector& PlaneProjectionOf
			( S2DDVector& vScreen,
				const Camera * pCamera, const Space * pTargetSpace,
				const S3DDVector& vBaseO,
				const S3DDVector& vBaseX, const S3DDVector& vBaseY ) const ;
		// 座標投影
		const S2DDVector& ViewProjectionOf
			( S2DDVector& vScreen, const S3DDVector& vPos ) const ;
		const S2DDVector& ViewProjectionAndScaleOf
			( S2DDVector& vScreen, S2DDVector& vScale, const S3DDVector& vPos ) const ;
		// 現在のカメラで球（大域座標）が視界に入るか判定する
		bool IsSphereIntoView
			( const S3DDMatrix& matSpace,
				const S3DDVector& vGlobalPos,
				double fpLocalRadius, const SGLImageRect& rectView ) const ;
		// カメラからの距離が非表示距離を越えているか判定
		bool IsBehindFarHiddenDistance( S3DRenderContextInterface& render ) const ;
		bool IsAheadNearHiddenDistance( S3DRenderContextInterface& render ) const ;

		// カメラオブジェクト削除通知（参照している場合に安全に参照解除）
		virtual void NotifyDeleteCamera( Camera * pCamera ) ;
		// メインカメラ設定
		virtual void SetMainCamera( Camera * pCamera ) ;
		// メインカメラ取得
		Camera * GetMainCamera( void ) const
		{
			return	m_refMainCamera.GetReference() ;
		}
		// 現在のカメラに設定し各種パラメータを計算
		void SetCurrentCamera( Camera * pCamera ) ;
		// カメラによる変換行列計算（修正後座標）
		static void CalcCameraTransformation
			( S3DDMatrix& matCamera,
				S3DDVector& vCamera, const Camera * pCamera ) ;
		// カメラによる変換行列計算（修正前座標）
		static void CalcUnmodifiedCameraTransformation
			( S3DDMatrix& matCamera,
				S3DDVector& vCamera, const Camera * pCamera ) ;
		// 現在のカメラ
		Camera * GetCurrentCamera( void ) const
		{
			return	m_pCurrentCamera ;
		}
		// 現在のカメラの変換行列で座標変換
		const S3DDVector& TransformByCurrentCamera( S3DDVector& vPos ) const
		{
			m_matCamera.RevolveVector( vPos ) ;
			vPos -= m_vCameraTrans ;
			return	vPos ;
		}
		// ローカル空間から現在のカメラ視点の座標へ変換
		const S3DDVector& TransformByCurrentCamera
				( Item * pItem, S3DDVector& vPos ) const ;
		const S3DDVector& TransformByCurrentCamera
				( Space * pSpace, S3DDVector& vPos ) const ;
		// 現在のカメラの変換回転行列
		const S3DDMatrix& GetCurrentCameraTransformation( S3DDVector& vPos ) const
		{
			vPos = - m_vCameraTrans ;
			return	m_matCamera ;
		}
		// 現在のカメラの逆変換回転行列（ビルボードなどの基底に使用）
		const S3DDMatrix& GetCurrentCameraIMatrix( void ) const
		{
			return	m_matICamera ;
		}
		// 現在のカメラ座標（グローバル空間）
		const S3DDVector& GetCurrentCameraTarget( void ) const
		{
			return	m_vCameraTarget ;
		}
		const S3DDVector& GetCurrentCameraPosition( void ) const
		{
			return	m_vCameraPos ;
		}
		const S3DDVector& GetCurrentCameraTop( void ) const
		{
			return	m_vCameraTop ;
		}
		// 現在のグローバル空間の描画用オフセット取得
		const S3DDVector& GetRenderingOffset( void ) const
		{
			return	m_vRenderingOffset ;
		}
		// レンダリング立体視ターゲット
		RenderingStage GetCurrentRenderingStage( void ) const
		{
			return	m_rsCurrentRenderingStage ;
		}
		RenderContext::StereoViewIndex
					GetCurrentStereoViewTarget( void ) const
		{
			return	m_sviStereoViewTarget ;
		}
		// 現在のレンダリングフェーズ
		ItemClass GetCurrentRenderingPhase( void ) const
		{
			return	m_classCurrentRendering ;
		}
		// 現在のコリジョン追加フェーズ（ItemClass マスク）
		uint32_t GetCurrentCollisionMask( void ) const
		{
			return	m_maskCurrentCollision ;
		}
		// 現在レンダリング中の透視変換行列
		const S4DMatrix& GetCurrentPerspective( void ) const
		{
			return	m_mat4CurrentPerspective ;
		}

	public:
		// サウンド専用カメラ
		Camera * GetSoundCamera( void ) const
		{
			Camera *	pSoundCamera = m_refSoundCamera.GetReference() ;
			return	(pSoundCamera != nullptr)
						? pSoundCamera : m_refMainCamera.GetReference() ;
		}
		void SetSoundCamera( Camera * pCamera )
		{
			m_refSoundCamera = pCamera ;
		}
		// サウンド再生開始
		SGLError PlaySound( SoundItem * pSound, uint64_t nFlags ) ;

	protected:
		// アイテム作用フラグ更新／カメラ・光源情報収集
		void UpdateBehaviorFlags
			( Space * pSpace, bool flagSpaceVisible = true ) ;
		void CollectBehaviorFlags
			( Space * pSpace, bool flagSpaceVisible = true ) ;
		// レンダリングイベント
		void RenderSceneEventItems
			( Space* pSpace, ItemClass clsItem ) ;
		// コリジョン
		void RenderSceneCollision
			( S3DCollision& collision,
				const Space* pSpace, uint32_t maskClasses ) ;
		// レンダリング
		void RenderSceneItems
			( S3DRenderContextInterface& render,
				const Space* pSpace, ItemClass clsItem,
				uint64_t flagsExclusion = 0, uint32_t optShader = 0 ) ;
		// レイヤード空間レンダリング
		void RenderLayeredSpace
			( S3DRenderContextInterface& render,
				const Space* pSpace,
				uint64_t flagsExclusion = 0, uint32_t optShader = 0 ) ;
		// 全レイヤード空間をレンダリング
		void RenderAllLayeredSpaces
			( S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0, uint32_t optShader = 0 ) ;

	public:
		// 画面更新通知（S3DScene::PostSceneUpdate はプレースホルダ）
		virtual void PostSceneUpdate( void ) ;

	public:
		// レンダリングスレッド排他処理用
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t TestLocked( void ) const ;
		virtual SSystem::SSharableMutex * GetUIThreadMutex( void ) const ;

	public:
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// 回転行列アニメーション
		class	RotationBezier	: public Animation
		{
		protected:
			SGLBezierCurves<S3DDQuaternion>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( RotationBezier, Animation )
			// 構築関数
			RotationBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<S3DDQuaternion>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 回転設定
			void SetMatrixTo
				( const SpaceInfo& space,
					const S3DDMatrix& mat, double a0 = 0.0, double a1 = 0.0 ) ;
			void SetMatrixTo
				( const SpaceInfo& space,
					const S3DMatrix& mat, double a0 = 0.0, double a1 = 0.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 回転角アニメーション
		class	AngleBezier	: public Animation
		{
		protected:
			S3DDMatrix					m_matBase ;
			SGLBezierCurves<S3DVector>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( AngleBezier, Animation )
			// 構築関数
			AngleBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<S3DVector>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 回転設定
			void SetAngleTo
				( const SpaceInfo& space,
					double x, double y, double z,
					double a0 = 0.0, double a1 = 0.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 拡大アニメーション
		class	ZoomBezier	: public Animation
		{
		protected:
			S3DDMatrix					m_matBase ;
			SGLBezierCurves<S3DVector>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( ZoomBezier, Animation )
			// 構築関数
			ZoomBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<S3DVector>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 回転設定
			void SetZoom
				( const SpaceInfo& space,
					const S3DVector & vStart,
					const S3DVector & vEnd,
					double a0 = 0.0, double a1 = 0.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 平行移動アニメーション
		class	MoveBezier	: public Animation
		{
		protected:
			SGLBezierCurves<S3DDVector>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( MoveBezier, Animation )
			// 構築関数
			MoveBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<S3DDVector>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 移動設定
			void SetMoveTo
				( const SpaceInfo& space,
					double x, double y, double z,
					double a0 = 0.0, double a1 = 0.0 ) ;
			void SetMoveTo
				( const SpaceInfo& space,
					const S3DDVector& v, double a0 = 0.0, double a1 = 0.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 透明度アニメーション
		class	TransparencyBezier	: public Animation
		{
		protected:
			SGLBezierCurves<double>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( TransparencyBezier, Animation )
			// 構築関数
			TransparencyBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<double>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 透明度設定
			void SetTransparencyTo
				( const SpaceInfo& space,
					unsigned int nTransparency,
					double a0 = 1.0, double a1 = 1.0 ) ;
			void SetAlphaTo
				( const SpaceInfo& space,
					double alpha, double a0 = 1.0, double a1 = 1.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 色効果アニメーション
		class	ColorBezier	: public Animation
		{
		protected:
			SGLBezierCurves<S3DVector>	m_bzMul ;
			SGLBezierCurves<S3DVector>	m_bzAdd ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( ColorBezier, Animation )
			// 構築関数
			ColorBezier
				( bool flagAutoDelete = true, SpaceInfo * psiTarget = NULL ) ;
			// ベジェ曲線
			SGLBezierCurves<S3DVector>& BezierMul( void )
			{
				return	m_bzMul ;
			}
			SGLBezierCurves<S3DVector>& BezierAdd( void )
			{
				return	m_bzAdd ;
			}
			// 効果設定
			void SetColorTo
				( const SpaceInfo& space,
					const S3DColor& color,
					double a0 = 1.0, double a1 = 1.0 ) ;
		public:
			// アニメーションフレーム処理
			virtual void OnFrame
				( S3DScene& scene, SpaceInfo& space, double t ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// レイヤー描画パラメータアニメーション
		class	LayeredParamBezier	: public Timer
		{
		protected:
			uint32_t				m_msecDuration ;
			uint32_t				m_msecCurrent ;
			SGLBezierCurves<double>	m_bezier ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( LayeredParamBezier, Animation )
			// 構築関数
			LayeredParamBezier( bool flagAutoDelete = true ) ;
			// ベジェ曲線
			SGLBezierCurves<double>& Bezier( void )
			{
				return	m_bezier ;
			}
			// 継続時間
			void SetDuration( uint32_t msecDuration ) ;
			// パラメータ設定
			void SetParameterTo
				( const Space& space,
					float32_t fpParam,
					double a0 = 1.0, double a1 = 1.0 ) ;
			// 透明度設定
			void SetTransparencyTo
				( const Space& space,
					unsigned int nTransparency,
					double a0 = 1.0, double a1 = 1.0 ) ;
			void SetAlphaTo
				( const Space& space,
					double alpha, double a0 = 1.0, double a1 = 1.0 ) ;
		public:
			// タイマー処理
			virtual bool OnTimer
				( S3DScene& scene, Space& space, uint32_t msecPast ) ;
			// フラッシュ処理
			virtual bool OnFlush( S3DScene& scene, Space& space ) ;
		public:
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 擬似被写界深度効果
		class	DepthOfFieldEffector	: public Effector
		{
		protected:
			S3DRenderDevice *			m_pDevice ;
			S3DRenderContextInterface *	m_pRender ;
			SGLImage	m_imgBuffer[2] ;	// ぼかしバッファ
			float32_t	m_gauss ;			// ガウスぼかし
			float32_t	m_zFocus ;			// 焦点距離（ｚ座標）
			float32_t	m_zNearDepth ;		// ぼかし幅（ｚ距離）
			float32_t	m_zFarDepth ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( DepthOfFieldEffector, Effector )
			// 構築関数
			DepthOfFieldEffector( void ) ;
			// 消滅関数
			virtual ~DepthOfFieldEffector( void ) ;
		public:
			// ぼかし度合い（ガウスパラメータ）
			void SetGauss( float32_t g ) ;
			float32_t GetGauss( void ) const
			{
				return	m_gauss ;
			}
			// ピント距離
			void SetFocus( float32_t zFocus ) ;
			float32_t GetFocus( void ) const
			{
				return	m_zFocus ;
			}
			// 被写界深度
			void SetDepth( float32_t zNearDepth, float32_t zFarDepth ) ;
			float32_t GetNearDepth( void ) const
			{
				return	m_zNearDepth ;
			}
			float32_t GetFarDepth( void ) const
			{
				return	m_zNearDepth ;
			}
		public:
			// 要求カラーバッファ
			virtual uint32_t GetRequiredColorBufferMask( void ) const ;
			// 描画処理
			virtual void DrawEffect
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		} ;
		// 発光グロー効果
		class	EmissiveGlowEffector	: public Effector
		{
		protected:
			S3DRenderDevice *			m_pDevice ;
			S3DRenderContextInterface *	m_pRender ;
			SGLImage	m_imgBuffer[2] ;	// ぼかしバッファ
			int			m_iSource ;			// ソース
			float32_t	m_gauss ;			// ガウスぼかし
			float32_t	m_zoom ;			// 拡大率（バッファ縮小率）
			float32_t	m_brightness ;		// 輝度
			float32_t	m_alpha ;			// 重ね度合い [0,1]
			bool		m_drawAdd ;			// 加算描画
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( EmissiveGlowEffector, Effector )
			// 構築関数
			EmissiveGlowEffector( void ) ;
			// 消滅関数
			virtual ~EmissiveGlowEffector( void ) ;
		public:
			// ソース
			void SetSourceIndex( int iSource ) ;
			int GetSourceIndex( void ) const
			{
				return	m_iSource ;
			}
			// ぼかし度合い（ガウスパラメータ）
			void SetGauss( float32_t g ) ;
			float32_t GetGauss( void ) const
			{
				return	m_gauss ;
			}
			// ぼかし拡大率（バッファ縮小率）
			void SetZoom( float32_t z ) ;
			float32_t GetZoom( void ) const
			{
				return	m_zoom ;
			}
			// 輝度
			void SetBrightness( float32_t b ) ;
			float32_t GetBrightness( void ) const
			{
				return	m_brightness ;
			}
			// 重ね度合い [0,1]
			void SetAlpha( float32_t a ) ;
			float32_t GetAlpha( void ) const
			{
				return	m_alpha ;
			}
			// 加算描画
			void SetDrawAdd( bool a ) ;
			bool IsDrawAdd( void ) const
			{
				return	m_drawAdd ;
			}
		public:
			// 要求カラーバッファ
			virtual uint32_t GetRequiredColorBufferMask( void ) const ;
			// 描画処理
			virtual void DrawEffect
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		} ;
		// ぼかし・フラッシュ効果
		class	CurtainEffector	: public EmissiveGlowEffector
		{
		protected:
			SGLPalette	m_argbCurtain ;
			uint32_t	m_nCurtainAlpha ;
			SGLPalette	m_argbEffectColor ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CurtainEffector, EmissiveGlowEffector )
			// 構築関数
			CurtainEffector( void ) ;
			// 消滅関数
			virtual ~CurtainEffector( void ) ;
		public:
			// フラッシュ効果色
			void SetCurtainColor( const SGLPalette& argb ) ;
			const SGLPalette& GetCurtainColor( void ) const ;
			// フラッシュ不透明度
			void SetCurtainAlpha( uint32_t a ) ;
			uint32_t GetCurtainAlpha( void ) const ;
		public:
			// 描画処理
			virtual void DrawEffect
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					SGLImageObject*const* ppImage,
					size_t nMultiImages, SGLImageObject * pDepth ) ;
		} ;

	} ;

}

#endif

