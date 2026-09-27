
#if	!defined(__SAKURAGLX_SPRITE_H__)
#define	__SAKURAGLX_SPRITE_H__	1

#include <sakura/ssys_reference_array.h>

namespace	SakuraGL
{
	class	SGLSprite ;
	class	SGLSpriteAction ;

	//////////////////////////////////////////////////////////////////////////
	// スプライト描画インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteDrawer	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteDrawer, SGLObject )
		// スプライトにアタッチされた
		virtual void OnAttachedSprite( SGLSprite * pSprite ) ;
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) ;
		// 描画域取得
		virtual bool GetRectangle
			( SGLImageRect& rectDraw, SGLImageObject* image ) const ;
		// 当たり判定
		virtual bool IsHitPointAt
			( SGLImageObject* image, double x, double y ) const ;

	public:	// SGLObject 実装
		// 複製（可能なら）
		virtual SGLObject * DuplicateObject( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スプライト画像フィルタインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFilter	: public SGLSpriteDrawer
	{
	protected:
		SSystem::SSmartPointer<SGLImageObject>	m_pBuffer ;
		SGLAffine	m_afParam ;
		int32_t		m_paramFilter ;
		int32_t		m_paramFilter2 ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFilter, SGLSpriteDrawer )
		// 構築関数
		SGLSpriteFilter( void ) ;
		SGLSpriteFilter( const SGLSpriteFilter& filter ) ;
		// 動的描画フィルタか？
		virtual bool IsDynamicDrawer( void ) const ;
		// 中間バッファを取得
		virtual SGLImageObject *
			GetInternalBuffer( const SGLImageInfo& infImage ) ;
		// フィルタ処理
		virtual void Filter
			( S3DRenderContextInterface& render, SGLImageObject* image ) ;
		// フィルター進行度設定
		virtual void SetFilterParameter( int nParam, int nParam2 ) ;
		// フィルター変換行列設定
		void SetParameterAffine( const SGLAffine& af ) ;
		// タイマー処理
		virtual void OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// タイマー処理
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteTimer	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteTimer, SGLObject )
		// 構築関数
		SGLSpriteTimer( void ) ;
		// 構築関数（ダミー）
		SGLSpriteTimer( const SGLSpriteTimer& timer ) ;
		// タイマー処理（true で終了）
		virtual bool OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// マウス入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMouseListener	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMouseListener, SGLObject )
		// 構築関数
		SGLSpriteMouseListener( void ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( SGLSprite& sprite, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( SGLSprite& sprite, int32_t zDelta,
				double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン（前処理）
		virtual bool OnButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool OnRButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 中央ボタン
		virtual bool OnMButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン（後処理）
		virtual bool AfterButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool AfterLButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterLButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterLButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool AfterRButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterRButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterRButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 中央ボタン
		virtual bool AfterMButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterMButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterMButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;

	public:
		// フラグ
		enum	MouseFlag
		{
			MouseIDMask			= 0x0000FFFF,
			ButtonIDMask		= 0x00FF0000,
			TouchFlag			= 0x01000000,		// タッチパネル操作時
			VirtualMouseFlag	= 0x02000000,		// 仮想マウス操作時
			VirtualTouchFlag	= 0x04000000,		// 仮想タッチ操作時
			ButtonIDShifter		= 16,
			LeftButtonID		= 0,
			RightButtonID		= 1,
			MiddleButtonID		= 2,
			WheelDeltaUnit		= 0x100,
			NoMouseListener		= 0x10000000,
		} ;
		// マウス識別子を取得（マルチタッチ用）
		static uint32_t GetMouseID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & MouseIDMask) ;
		}
		// マウスボタン識別子を取得
		static uint32_t GetButtonID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & ButtonIDMask) >> ButtonIDShifter ;
		}
		// タッチパネル入力判定
		static bool IsFromTouch( int64_t nFlags )
		{
			return	((nFlags & TouchFlag) != 0) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// マウス入力インターフェース（マルチタッチ座標記録）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMouseStateListener	: public SGLSpriteMouseListener
	{
	protected:
		struct	MouseState
		{
			S2DDVector	vPos ;
			uint32_t	idMouse ;
			bool		fLeftDown ;
			bool		fRightDown ;
			bool		fMiddleDown ;
		} ;
		SSystem::SObjectArray<MouseState>	m_vecMousePointers ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMouseStateListener, SGLSpriteMouseListener )
		// 構築関数
		SGLSpriteMouseStateListener( void ) ;
		// 消滅関数
		virtual ~SGLSpriteMouseStateListener( void ) ;

	public:
		// 有効ポインタ数取得
		size_t GetPointerCount( void ) const ;
		// 指標検索
		ssize_t FindMouseIndexById( uint32_t idMouse ) const ;
		// ポインタ座標取得
		bool GetMousePointAt( size_t i, S2DDVector& vPos ) const ;
		// マウスボタンの押下状態取得
		bool IsLButtonDownAt( size_t i ) const ;
		bool IsRButtonDownAt( size_t i ) const ;
		// 左ドラッグ中／タップ中ポインタ数取得
		size_t GetLDownPointsCount( void ) const ;
		// 左ドラッグ中／タップ中ポインタ座標取得
		size_t EnumerateLDownPoints
			( SSystem::SArray<S2DDVector>& arrPoints ) const ;

	protected:
		// マウスステータス取得／存在しない場合には生成
		MouseState * CreateMouseStateAs
			( double xPos, double yPos, uint32_t idMouse ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( SGLSprite& sprite, int64_t nFlags ) ;
		// マウスボタン（前処理）
		virtual bool OnButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// キー入力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteKeyListener	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteKeyListener, SGLObject )
		// 構築関数
		SGLSpriteKeyListener( void ) ;

	public:
		// キー入力
		virtual bool OnKeyDown
			( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		// 文字入力
		virtual bool OnChar( SGLSprite& sprite, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( SGLSprite& sprite, SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( SGLSprite& sprite ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( SGLSprite& sprite, const SGLInputCompositionString& icsComp ) ;
		// コマンド
		virtual bool OnCommand
			( SGLSprite& sprite, const wchar_t * pszCmd,
				int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スプライト基底
	//////////////////////////////////////////////////////////////////////////

	class	SGLSprite	: public SGLObject
	{
	public:
		// 表示パラメータ
		struct	Parameter
		{
			uint32_t			nFlags ;		// 描画フラグ
			uint32_t			nSpriteFlags ;	// スプライトフラグ
			S3DDVector			vDst ;			// 表示座標
			S2DDVector			vCenter ;		// ソース画像の中心座標
			S2DDVector			vZoom ;			// 拡大率
			double				zAngle ;		// 回転角 [deg]
			double				xyCross ;		// ｘｙ軸交差角度 [deg]
			uint32_t			nTransparency ;	// 透明度 [0,256]
			int32_t				paramFilter ;	// フィルター進行度
			int32_t				paramFilter2 ;
			SGLPalette			rgbColorParam ;	// 色パラメータ
			size_t				countVertex ;
			const S2DVector *	pVertices ;

			// 構築関数
			Parameter( void )
				: nFlags(paintSmoothStretch), nSpriteFlags(0),
					vZoom(1,1), zAngle(0.0), xyCross(90.0),
					nTransparency(0), paramFilter(0), paramFilter2(0),
					countVertex(0), pVertices(NULL) {}
			// シリアライズ（ポインタを除く）
			SGLError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
			// デシリアライズ
			SGLError LoadWithoutPointer( SSystem::SFileInterface& file ) ;
		} ;
		// スプライトフラグ
		enum	SpriteFlag
		{
			flagZScale	= 0x0001,
		} ;
		// 要素フラグ
		enum	ElementFlag
		{
			flagParamPosX			= 0x00000001,
			flagParamPosY			= 0x00000002,
			flagParamPosZ			= 0x00000004,
			flagParamPos			= 0x00000007,
			flagParamCenterX		= 0x00000010,
			flagParamCenterY		= 0x00000020,
			flagParamCenter			= 0x00000030,
			flagParamZoomX			= 0x00000100,
			flagParamZoomY			= 0x00000200,
			flagParamZoom			= 0x00000300,
			flagParamAngle			= 0x00001000,
			flagParamTransparency	= 0x00002000,
			flagParamFilter			= 0x00004000,
			flagParamFilter2		= 0x00008000,
		} ;
		// アニメーションタイプ
		enum	ActionType
		{
			actionOnce,
			actionLoop,
			actionTurn,
		} ;
		// スプライト・パラメータ・アニメーション
		class	Action	: public SGLObject
		{
		public:
			uint32_t	m_typeAction ;				// アニメーションタイプ
			bool		m_flagPaused ;				// 一時停止
			uint32_t	m_msecPast ;				// 経過時間
			uint32_t	m_msecStart ;				// 開始時間
			uint32_t	m_msecDuration ;			// 継続時間・周期
			uint32_t	m_maskSetElement ;			// 値設定要素マスク
			uint32_t	m_maskModifyElement ;		// 値変更要素マスク
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Action, SGLObject )
			// 構築関数
			Action( void ) ;
			Action( const Action& act ) ;
			// アニメーション処理
			virtual bool OnAction( Parameter& param, uint32_t msecPast ) ;
			// アニメーション完了処理
			virtual void OnFinish( Parameter& param ) ;
			// パラメータ反映
			virtual void EffectParameter
						( Parameter& param, double t ) ;
		public:
			// アニメーションタイプ設定
			void SetActionType( uint32_t typeAct ) ;
			// 時間設定
			void SetDuration
					( uint32_t msecDuration, uint32_t msecStart = 0 ) ;
			// 一時停止
			void Pause( void ) ;
			// 再開
			void Restart( void ) ;
		public:
			// 複製
			virtual SGLObject * DuplicateObject( void ) ;
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
		// 表示画像制御
		class	Imager	: public SGLObject
		{
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Imager, SGLObject )
			// 構築関数
			Imager( void ) ;
			// 関連付け時処理
			virtual void OnAttached( SGLSprite& sprite ) ;
			// アニメーション処理
			virtual void OnAnimation( SGLSprite& sprite, uint32_t msecPast ) ;
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSprite, SGLObject )
		// 構築関数
		SGLSprite( void ) ;
		SGLSprite( const SGLSprite& src ) ;
		// 消滅関数
		virtual ~SGLSprite( void ) ;

	protected:
		// デフォルト表示フラグ
		static uint32_t	m_nDefParamFlags ;

		// 表示パラメータ
		Parameter		m_paramView ;
		SSystem::SArray<S2DVector>
						m_paramVertex ;
		S2DDVector		m_vImageCenter ;

	public:
		// UI 動作フラグ
		enum	UIFlag
		{
			uiDisabled				= 0x00000001,	// disable all input
			uiDisabledKeyInput		= 0x00000002,
			uiDisabledMouseWheel	= 0x00000004,
			uiHaveFocus				= 0x00000010,
			uiFocusable				= 0x00000020,
			uiUnclickable			= 0x00000040,
			uiNoMoveFocusByKey		= 0x00000080,
			uiGroupMember			= 0x00000100,
			uiModalFirst			= 0x00000200,
			uiModalEnd				= 0x00000400,
		} ;
	protected:
		uint64_t			m_flagsUI ;
		SGLImageRect		m_rectClickable ;

		#if	defined(__DEBUG__)
		bool				m_debug ;
		#endif

		// 表示フラグ
		bool				m_visible ;

		// 表示優先度
		int32_t				m_priority ;

		// 識別子（主に UI で使用）
		SSystem::SString	m_strID ;

		// 描画インターフェース
		SSystem::SSmartPointer<SGLSpriteDrawer>	m_pDrawer ;

		// 画像フィルター
		SSystem::SReferenceArray<SGLSpriteFilter>	m_filters ;

		// タイマ処理
		SSystem::SReferenceArray<SGLSpriteTimer>	m_timers ;

		// パラメータ・アニメーション
		SSystem::SObjectArray<Action>	m_actions ;
		Parameter						m_paramAction ;

		// 表示画像
		SSystem::SSmartReference<SGLImageObject>	m_refImage ;
		SSystem::SSmartReference<SGLImageObject>	m_refLeftImage ;
		SSystem::SSmartPointer<Imager>				m_pImager ;

	public:
		// 立体視用
		enum	Stereo3DView
		{
			s3dMonoview	= -1,
			s3dRightView,
			s3dLeftView,
		} ;
		static RenderContext::StereoViewIndex ToStereoViewIndex( Stereo3DView s3dView ) ;
		static Stereo3DView FromStereoViewIndex( RenderContext::StereoViewIndex sviView ) ;

		// 描画バッファ
		class	Buffer	: public SGLObject
		{
		protected:
			S3DRenderContext	m_render ;
			SGLPaintContextType	m_typePaint ;
			bool				m_flagStereo3D ;
			bool				m_flagZBuffer ;
			bool				m_flagMultiSampling ;
			bool				m_flagFillBack ;
			SGLPalette			m_rgbaFillBack ;
			uint64_t			m_nBufFlags ;
			SGLImage			m_imgBuffer ;
			SGLImage			m_imgLeftBuffer ;
			SGLImage			m_imgZBuffer ;
			S3DRenderContext	m_renderTemp ;
			SGLImage			m_imgTempBuffer ;
			SGLImage			m_imgTempLeftBuffer ;
			SGLImage			m_imgTempZBuffer ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( Buffer, SGLObject )
			// 構築関数
			Buffer( SGLPaintContextType type = typePaintEntisGLS ) ;
			Buffer( const Buffer& buf ) ;
			// 消滅関数
			virtual ~Buffer( void ) ;
			// バッファ生成
			SGLError CreateBuffer
				( uint32_t width, uint32_t height,
					uint32_t format = formatImageARGB, uint32_t depth = 32,
					uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
					bool flagZBuffer = false, bool flagStereo3D = false ) ;
		protected:
			void PrepareMultiSampling( void ) ;
		public:
			// 描画ターゲット設定
			SGLImageObject * AttachRenderTarget
				( Stereo3DView s3dView, const SGLImageRect * pRectView ) ;
			// 描画ターゲット解除
			SGLImageObject * DetachRenderTarget
				( Stereo3DView s3dView, const SGLImageRect * pRectView ) ;
			// レンダラ設定
			void SetFrameRenderer
				( RenderContext * render, bool flagOwner = true ) ;
			// 背景色
			void SetFillBack( SGLPalette rgbaFillBack ) ;
			void DisableFillBack( void ) ;
			bool IsFillBack( void ) const ;
			const SGLPalette& GetFillBackColor( void ) const ;
		public:
			// レンダラ取得
			S3DRenderContext& Renderer( void ) ;
			// カラーバッファ
			SGLImage * GetImage( void ) ;
			SGLImage * GetLeftImage( void ) ;
			// ｚバッファ取得
			SGLImage * GetZBuffer( void ) ;
			// バッファフラグ
			uint64_t GetBufFlags( void ) const ;
			// ステレオ立体視か？
			bool IsStereo3D( void ) const ;
			// ｚバッファ
			bool HasZBuffer( void ) const ;
			// マルチサンプリング
			bool IsMultisampling( void ) const ;
			// 一時バッファ（レンダリング対象）
			SGLImage * GetTempImage( void ) ;
			SGLImage * GetTempLeftImage( void ) ;
			SGLImage * GetTempZBuffer( void ) ;
		public:
			// 複製
			virtual SGLObject * DuplicateObject( void ) ;
			// シリアライズ
			virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
			// 復元
			virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		} ;
	protected:
		SSystem::SSmartPointer<Buffer>		m_pBuffer ;

		// 更新フラグ
		enum	UpdateStatus
		{
			updateEmpty,
			updateRect,
			updateFull,
		} ;
		UpdateStatus	m_statusUpdate ;
		SGLRect			m_rectUpdate ;
		atomic_int_t	m_countFrozen ;

	public:
		// 疑似 3D 表示用パラメータ
		class	Virtual3DParam
		{
		public:
			S3DVector		m_vProjectScreen ;
			float32_t		m_zProjectScale ;
			float32_t		m_fpPixelAspect ;
			float32_t		m_xParallax ;
			float32_t		m_zParallaxFocus ;
			float32_t		m_xParallaxScreen ;
			S3DDVector		m_vCameraView ;
			S3DDVector		m_vCameraTarget ;
		public:
			Virtual3DParam( void ) ;
			Virtual3DParam( const Virtual3DParam& src ) ;
		} ;
	protected:
		SSystem::SSmartPointer<Virtual3DParam>	m_pVirtual3D ;

		// 親スプライト
		SSystem::SSmartReference<SGLSprite>	m_refParent ;

		// 子スプライト
		SSystem::SReferenceArray<SGLSprite>	m_children ;
		SSystem::SReferenceArray<SGLSprite>	m_rfarMouseFocus ;
		SSystem::SSmartReference<SGLSprite>	m_refKeyFocus ;
		SSystem::SSmartReference<SGLSprite>	m_refCaptured ;

		// マウス入力リスナ
		SSystem::SSmartReference<SGLSpriteMouseListener>	m_refMouseListener ;

		// キー入力リスナ
		SSystem::SSmartReference<SGLSpriteKeyListener>	m_refKeyListener ;

	public:
		#if	defined(__DEBUG__)
		void PostDebugTrace( void ) { m_debug = true ; }
		#else
		void PostDebugTrace( void ) {}
		#endif

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	public:	// 画像表示インターフェース
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
		// フレーム描画完了後処理
		virtual void FinishDrawFrame( void ) ;
		// 描画前処理
		virtual void BeforeDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// 描画
		virtual void Draw
			( S3DRenderContextInterface& render,
				const Virtual3DParam* pV3D = NULL,
				Stereo3DView s3dView = s3dMonoview ) const ;
		// 描画後処理
		virtual void AfterDraw( Stereo3DView s3dView = s3dMonoview ) ;
		// SGLDrawImageParamList への描画
		virtual void DrawImageList
			( SGLDrawImageParamList& dipl,
				const Virtual3DParam* pV3D = NULL,
				Stereo3DView s3dView = s3dMonoview ) ;
		// 中間バッファへ描画
		virtual void Refresh( Stereo3DView s3dView = s3dMonoview ) ;
		// カスタムフィルタ
		virtual SGLImageObject * CustomFilter
			( S3DRenderContextInterface& render,
				SGLImageObject * pFrameBuf, SGLImageObject * pSrcImage ) ;
		// 更新領域通知
		virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
		void NotifyUpdate( void ) ;
		// 更新領域状態取得
		bool HasUpdate( void ) const ;
		// フレームバッファ更新一時停止
		void FreezeFrameUpdate( void ) ;
		bool DefrostFrameUpdate( void ) ;
	protected:
		// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;
		// 表示状態の子スプライトに対し BeforeDraw を呼び出し
		virtual void BeforeDrawChildren
			( Stereo3DView s3dView = s3dMonoview ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		virtual void DrawChildrenImageList
			( SGLDrawImageParamList& dipl,
					Stereo3DView s3dView = s3dMonoview ) const ;

	public:	// 座標変換
		// 描画パラメータ取得
		virtual bool GetPaintParam
			( SGLPaintParam& pp, SGLAffine& affine,
				const Virtual3DParam* pV3D = NULL,
				Stereo3DView s3dView = s3dMonoview ) const ;
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// 小スプライトの外接矩形の集合を取得
		bool GetAllChildrenRectangle( SGLRect& rectExt ) const ;
		// ローカル座標からグローバル座標へ変換
		virtual bool LocalToGlobal( S2DDVector& vPos ) const ;
		virtual bool LocalToGlobalRect( SGLRect& rect ) const ;
		// グローバル座標からローカル座標へ変換
		virtual bool GlobalToLocal( S2DDVector& vPos ) const ;

	public:
		// 表示パラメータ取得
		const Parameter& GetParameter( void ) const
		{
			return	m_paramView ;
		}
		// 描画フラグ取得
		uint32_t GetDrawParamFlags( void ) const
		{
			return	m_paramView.nFlags ;
		}
		// スプライト処理フラグ取得
		uint32_t GetSpriteFlags( void ) const
		{
			return	m_paramView.nSpriteFlags ;
		}
		// 座標取得
		const S3DDVector& GetPosition( void ) const
		{
			return	m_paramView.vDst ;
		}
		S2DDVector GetPosition2D( void ) const
		{
			return	S2DDVector( m_paramView.vDst.x, m_paramView.vDst.y ) ;
		}
		// 中心座標取得
		const S2DDVector& GetCenterPosition( void ) const
		{
			return	m_paramView.vCenter ;
		}
		const S2DDVector& GetImageCenter( void ) const
		{
			return	m_vImageCenter ;
		}
		// 拡大率取得
		const S2DDVector& GetZoom( void ) const
		{
			return	m_paramView.vZoom ;
		}
		// 回転角取得
		double GetRotation( void ) const
		{
			return	m_paramView.zAngle ;
		}
		// 透明度取得
		uint32_t GetTransparency( void ) const
		{
			return	m_paramView.nTransparency ;
		}
		// フィルタパラメータ取得
		int32_t GetFilterParameter( void ) const
		{
			return	m_paramView.paramFilter ;
		}
		int32_t GetFilter2Parameter( void ) const
		{
			return	m_paramView.paramFilter2 ;
		}

	public:
		// デフォルト表示フラグ設定
		static void SetDefaultParamFlags( uint32_t nDefFlags ) ;
		// 表示パラメータ設定
		void SetParameter( const Parameter& param ) ;
		// 描画フラグ変更
		void ModifyDrawFlags( uint32_t nAddFlags, uint32_t nRemoveFlags = 0 ) ;
		// スプライト処理フラグ変更
		void ModifySpriteFlags( uint32_t nAddFlags, uint32_t nRemoveFlags = 0 ) ;
		// 座標設定
		void SetPosition( double x, double y ) ;
		void SetPosition3D( double x, double y, double z ) ;
		void SetPosition3D( const S3DDVector& vPos ) ;
		// 中心座標設定
		void SetCenterPosition( double x, double y ) ;
		// 拡大率設定
		void SetZoom( double x, double y ) ;
		// 回転角設定
		void SetRotation( double zAngle ) ;
		// 透明度設定
		void SetTransparency( uint32_t nTransparency ) ;
		// フィルタパラメータ設定
		void SetFilterParameter( int32_t paramFilter ) ;
		void SetFilter2Parameter( int32_t paramFilter ) ;
		// 表示中心座標を調整する
		enum	RegulateFlag
		{
			regCenter	= 0x0000,
			regLeft		= 0x0001,
			regRight	= 0x0002,
			regVCenter	= 0x0000,
			regTop		= 0x0010,
			regBottom	= 0x0020,
		} ;
		bool RegulateCenter
			( uint32_t nFlags = 0,
				double xOffset = 0.0, double yOffset = 0.0 ) ;

	public:
		// UI フラグ取得
		uint64_t GetUIFlag( void ) const
		{
			return	m_flagsUI ;
		}
		// UI フラグ変更
		uint64_t ModifyUIFlag
			( uint64_t nAddFlags, uint64_t nRemoveFlags = 0 ) ;
		// ヒット領域取得
		const SGLImageRect & GetClickableRect( void ) const
		{
			return	m_rectClickable ;
		}
		// ヒット領域設定
		void SetClickableRect( SGLImageRect& rect ) ;
		// 表示フラグ取得
		bool IsVisible( void ) const
		{
			return	m_visible ;
		}
		// 表示フラグ設定
		void SetVisible( bool visible ) ;
		// 表示優先度取得
		int32_t GetPriority( void ) const
		{
			return	m_priority ;
		}
		// 表示優先度変更
		void ChangePriority( int32_t nPriority ) ;
		// 識別子取得
		const SSystem::SString& GetID( void ) const
		{
			return	m_strID ;
		}
		// 識別子設定
		void SetID( const wchar_t * pwszID ) ;

	protected:
		// 優先度位置検索
		size_t OrderIndexAs( int32_t nPriority ) const ;
	public:
		// 子スプライト追加
		virtual void AddChild( SGLSprite* pSprite ) ;
		virtual void AddSmartChild( SGLSprite* pSprite ) ;
		// 子スプライトは自動的に削除されるか？
		bool IsSmartChild( SGLSprite* pSprite ) const ;
		// 子スプライト削除
		virtual SGLSprite * DetachChild( SGLSprite* pSprite ) ;
		virtual bool RemoveChild( SGLSprite* pSprite ) ;
		// 親スプライトから分離（同期タイムアウト時間付）
		SGLError DetachSyncTimeout( int64_t msecTimeout ) ;
		// 全子スプライト削除
		virtual void DetachAllChildren( void ) ;
		virtual void RemoveAllChildren( void ) ;
	protected:
		void AsyncReleaseChildReference( SGLSprite* pSprite ) ;
		void AsyncReleaseAllChildrenReference( void ) ;
		void AsyncRemoveAllChildren( void ) ;
	public:
		// 親スプライト取得
		virtual SGLSprite* GetParent( void ) const ;
		// 指定アイテム取得 (孫アイテム以下は \ で区切って指定)
		SGLSprite* GetItemAs( const wchar_t * pwszID ) const ;
		// ヒットアイテム検索
		virtual SGLSprite* GetHitSpriteAt( S2DDVector& vPos ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
		// 画像ヒット判定
		static bool IsHitSpriteImage
			( SGLImageObject * pImage,
				double x, double y, bool fAlphaImage = false ) ;
		// ドラッグ／スワイプ処理判定（子スプライトが処理するか？）
		virtual bool CanBeginDragOver( double x, double y ) const ;

	public:
		// 子スプライト数取得
		size_t GetChildCount( void ) const ;
		// 子スプライト取得
		SGLSprite * GetChildAt( size_t i ) const ;
		// 子スプライトを検索
		ssize_t FindChildSprite( SGLSprite * pChild ) const ;
		// 指定された方向へ最も近い
		// フォーカスを受け取り可能なスプライトを検索する
		SGLSprite * SearchNearestFocusableSprite
			( SGLSprite * pOrigin,
				const S2DDVector& vOrigin, const S2DDVector& vDir ) const ;
		// フォーカス座標（キー操作フォーカス移動用）
		virtual bool GetFocusPoint( S2DDVector& vPos ) ;

	public:
		// 画像ファイル読み込み
		SGLError LoadImage( const wchar_t * pwszFilePath ) ;
		// アニメーション関連付け
		void AttachAnimation
			( SGLImageObject * pImage, const SGLImageRect * pClip = NULL ) ;
		// アニメーション開始
		void BeginAnimation
			( ssize_t countLoop = -1,
				size_t iLoopStart = 0, size_t iLoopEnd = 0,
				size_t iStartFrame = 0, size_t msecDuration = 0 ) ;
		// ループ設定変更
		void SetLoopAnimation
			( ssize_t countLoop = -1,
				size_t iLoopStart = 0, size_t iLoopEnd = 0 ) ;

	public:
		// 画像関連付け
		void AttachImage
			( SGLImageObject * pImage,
				SGLImageObject * pLeftImage = NULL ) ;
		// 関連付けられた画像取得
		SGLImageObject * GetAttachedImage( void ) const ;
		SGLImageObject * GetAttachedLeftImage( void ) const ;
		// 関連付けられた画像サイズ取得
		SGLSize GetImageSize( void ) const ;
		// 関連付けられた画像情報取得
		SGLError GetImageInfo( SGLImageInfo& imginf ) const ;
		// 描画オブジェクト設定
		void SetSpriteDrawer( SGLSpriteDrawer * pDrawer ) ;
		// 描画インターフェース取得
		SGLSpriteDrawer * GetSpriteDrawer( void ) const
		{
			return	m_pDrawer ;
		}
		// 画像制御オブジェクト設定
		void SetSpriteImager( Imager * pImager ) ;
		// バッファ生成
		virtual SGLError CreateBuffer
			( uint32_t width, uint32_t height,
				uint32_t format = formatImageDefaultRGBA,
				uint32_t depth = 32,
				uint64_t nBufFlags = SGLImageObject::bufferOnMemory,
				bool flagZBuffer = false, bool flagStereo3D = false,
				SGLPaintContextType type = typePaintEntisGLS ) ;
		// バッファ解放
		virtual void ReleaseBuffer( void ) ;
		// バッファを保持しているか？
		virtual bool IsBuffered( void ) const ;
		// レンダリングデバイスの設定
		virtual SGLError SetRenderDevice( S3DRenderDevice * pDevice ) ;
		// 描画オブジェクト
		virtual S3DRenderContext * GetBufferRenderer( void ) const ;
		virtual S3DRenderDevice * GetBufferRenderDevice( void ) const ;
		// レンダラ設定
		virtual void SetBufferRenderer
			( RenderContext * render, bool flagOwner = true ) ;
		// バッファを取得する
		virtual Buffer * GetFrameBuffer( void ) const ;
		// 背景色取得
		virtual bool GetFillBackColor( uint32_t& argbFill ) const ;
		// 背景色設定
		virtual SGLError SetFillBackColor
			( uint32_t argbFill, bool flagFillBack = true ) ;
		// フィルタ追加
		void AddReferenceFilter( SGLSpriteFilter * pFilter ) ;
		void AddSmartFilter( SGLSpriteFilter * pFilter ) ;
		// フィルタ削除
		void RemoveFilter( SGLSpriteFilter * pFilter ) ;
		void RemoveAllFilter( void ) ;
		// 特定クラスのフィルタ検索
		SGLSpriteFilter * GetFilterTypeOf( const ESLRuntimeClass& rtClass ) const ;
		SGLSpriteFilter * GetFilterTypeOf( const ESLRuntimeClass& rtClass, size_t& iNext ) const ;
		// フィルタ取得
		const SSystem::SReferenceArray<SGLSpriteFilter>& GetFilterList( void ) const ;

	public:
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;
		// 入力禁止状態
		virtual bool IsEnabled( void ) const ;
		virtual void SetEnable( bool fEnable ) ;
		// スクロール方向
		enum	ScrollDirection
		{
			scrollDefault	= 0,
			scrollHorz,
			scrollVert,
		} ;
		// スクロール・トラック位置属性
		virtual int GetScrollPos
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollPos
			( int nPos, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・トラック位置範囲属性
		virtual int GetScrollRange
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollRange
			( int nRange, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・ページサイズ属性
		virtual int GetScrollPageSize
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollPageSize
			( int nPageSize, ScrollDirection scrlDir = scrollDefault ) ;
		// ボタン属性
		virtual bool IsButtonChecked( void ) ;
		virtual void CheckButton( bool fCheck ) ;
		// コマンド処理
		virtual SGLError InvokeCommand
			( const SSystem::SXMLDocument& xmlCmd,
				SSystem::SXMLDocument * pxmlResult = NULL ) ;
		SGLError InvokeCommands
			( const wchar_t * pwszXMLCommands,
				SSystem::SXMLDocument * pxmlResults = NULL ) ;
		// スクロールバー関連付け
		virtual void AttachScrollBar
			( SGLSprite * pScrollBar, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロールバー関連付け
		virtual void DetachScrollBar
			( SGLSprite * pScrollBar, ScrollDirection scrlDir = scrollDefault ) ;

	public:	// 子アイテムへの属性
		// 可視状態
		virtual bool IsSpriteVisible( const wchar_t * pwszID ) const ;
		virtual void SetSpriteVisible( const wchar_t * pwszID, bool fVisible ) ;
		// 表示優先度
		virtual int GetSpritePriority( const wchar_t * pwszID ) const ;
		virtual void ChangeSpritePriority( const wchar_t * pwszID, int32_t nPriority ) ;
		// 透明度
		virtual uint32_t GetSpriteTransparency( const wchar_t * pwszID ) const ;
		virtual void SetSpriteTransparency
			( const wchar_t * pwszID, uint32_t nTransparency ) ;
		// 表示領域
		virtual bool GetSpriteRectangle
				( const wchar_t * pwszID, SGLRect& rectExt ) const ;
		virtual bool GetSpriteTextRectangle
				( const wchar_t * pwszID, SGLRect& rectExt ) const ;
		// 文字列属性
		virtual SSystem::SString GetSpriteText( const wchar_t * pwszID ) const ;
		virtual void SetSpriteText
			( const wchar_t * pwszID, const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetSpriteTextFont
			( const wchar_t * pwszID,
				const wchar_t * pwszFont, int nSize = 0 ) ;
		// 画像属性
		virtual void SetSpriteImage
			( const wchar_t * pwszID, const wchar_t * pwszImageID ) ;
		// 入力禁止状態
		virtual bool IsSpriteEnabled( const wchar_t * pwszID ) const ;
		virtual void SetSpriteEnable( const wchar_t * pwszID, bool fEnable ) ;
		// スクロール・トラック位置属性
		virtual int GetSpriteScrollPos
			( const wchar_t * pwszID,
				ScrollDirection scrlDir = scrollDefault ) const ;
		virtual void SetSpriteScrollPos
			( const wchar_t * pwszID,
				int nPos, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・トラック位置範囲属性
		virtual int GetSpriteScrollRange
			( const wchar_t * pwszID,
				ScrollDirection scrlDir = scrollDefault ) const ;
		virtual void SetSpriteScrollRange
			( const wchar_t * pwszID,
				int nRange, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・ページサイズ属性
		virtual int GetSpriteScrollPageSize
			( const wchar_t * pwszID,
				ScrollDirection scrlDir = scrollDefault ) const ;
		virtual void SetSpriteScrollPageSize
			( const wchar_t * pwszID,
				int nPageSize, ScrollDirection scrlDir = scrollDefault ) ;
		// ボタン属性
		virtual bool IsSpriteButtonChecked( const wchar_t * pwszID ) const ;
		virtual void CheckSpriteButton
			( const wchar_t * pwszID, bool fCheck ) ;

	public:
		// 擬似 3D 投影スクリーン座標設定
		virtual void SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		bool GetProjectionScreen
			( S3DVector& vScreen,
				double& zScale, double& fpPixelAspect ) const ;
		// 視差設定
		virtual void SetParallax
			( double xParallax,
				double zFocusRate = 1.0, double xScreenDelta = 0.0 ) ;
		double GetParallax( void ) const ;
		double GetParallaxFocusRatio( void ) const ;
		double GetParallaxScreenX( void ) const ;
		// カメラ設定
		virtual void SetVirtualCamera
			( const S3DDVector& vCamera, const S3DDVector& vTarget ) ;
		// 仮想３Ｄ設定取得
		Virtual3DParam * GetVirtual3DParam( void ) const ;

	public:
		// タイマ処理追加
		void AddReferenceTimer( SGLSpriteTimer * pTimer ) ;
		void AddSmartTimer( SGLSpriteTimer * pTimer ) ;
		// タイマ処理削除
		void RemoveTimer( SGLSpriteTimer * pTimer ) ;
		void RemoveAllTimer( void ) ;
		// 特定クラスのフィルタ検索
		SGLSpriteTimer * GetTimerTypeOf( const ESLRuntimeClass& rtClass ) const ;

	public:
		// 簡易アニメーション設定
		SGLSpriteAction * SetActionLinearTo
			( uint32_t msecDuration,
				uint32_t nTransparency,
				const S2DDVector * pPos = NULL,
				const S2DDVector * pZoom = NULL,
				double a0 = 0.0, double a1 = 0.0 ) ;

	public:
		// アニメーション追加
		void AddAction( Action * pAct ) ;
		void InsertActionAt( size_t i, Action * pAct ) ;
		// アニメーション即時完了
		void FlushAction( void ) ;
		void FlushAction( ActionType type ) ;
		// アニメーションキャンセル
		void CancelAction( void ) ;
		void CancelAction( ActionType type ) ;
		// アニメーション中か？
		bool IsAction( void ) const ;
		bool IsAction( ActionType type ) const ;
		// 全アニメーション一時停止
		void PauseAllAction( void ) ;
		// 全アニメーション再開
		void RestartAllAction( void ) ;

	protected:
		// アニメーション処理
		void UpdateAllActions( uint32_t msecPast ) ;

	public:
		// マウス入力リスナ設定
		void AttachMouseListener( SGLSpriteMouseListener * pListener ) ;
		// マウス入力リスナ解除
		void DetachMouseListener( SGLSpriteMouseListener * pListener ) ;
		// マウス入力リスナ設定
		void AttachKeyListener( SGLSpriteKeyListener * pListener ) ;
		// マウス入力リスナ解除
		void DetachKeyListener( SGLSpriteKeyListener * pListener ) ;
		// マウス入力キャプチャー要求
		virtual SGLError SetMouseCapture( void ) ;
		// マウス入力キャプチャー解放
		virtual SGLError ReleaseMouseCapture( void ) ;
		// マウスキャプチャーが解放された
		virtual void OnReleaseMouseCapture( void ) ;
		// キャプチャー中の子スプライトを取得
		virtual SGLSprite * GetMouseCapture( void ) const ;
		// キーフォーカスを要求
		virtual SGLError SetKeyFocus( void ) ;
		// キーフォーカスを解除
		virtual SGLError KillKeyFocus( void ) ;
		// フォーカスが設定された
		virtual void OnSetKeyFocus( void ) ;
		// キーフォーカスが解除された
		virtual void OnKillKeyFocus( void ) ;
		// キーフォーカスを有しているか？
		bool HasKeyFocus( void ) const
		{
			return	(m_flagsUI & uiHaveFocus) != 0 ;
		}
		// 次のフォーカスを受け取り可能なスプライトへフォーカスを移動する
		bool MoveNextKeyFocus( void ) ;
		// 前のフォーカスを受け取り可能なスプライトへフォーカスを移動する
		bool MovePrevKeyFocus( void ) ;
		// 指定の方向へフォーカスを移動する
		bool MoveKeyFocusDirectionOf( const S2DDVector& vDir ) ;

	protected:
		// マウスフォーカス取得
		virtual SGLSprite * GetMouseFocusAt
			( S2DDVector& vPos, double xPos, double yPos, int64_t nFlags ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( int64_t nFlags ) ;
		// マウスカーソル取得
		virtual const wchar_t * HitTestMouseCursor
			( double xPos, double yPos, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( int32_t zDelta, double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonDblClk
			( double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool OnRButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonDblClk
			( double xPos, double yPos, int64_t nFlags ) ;
		// 中央ボタン
		virtual bool OnMButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonDblClk
			( double xPos, double yPos, int64_t nFlags ) ;
		// フラグ
		enum	MouseFlag
		{
			MouseIDMask			= 0x0000FFFF,
			ButtonIDMask		= 0x00FF0000,
			TouchFlag			= 0x01000000,		// タッチパネル操作時
			VirtualMouseFlag	= 0x02000000,		// 仮想マウス操作時
			VirtualTouchFlag	= 0x04000000,		// 仮想タッチ操作時
			ButtonIDShifter		= 16,
			LeftButtonID		= 0,
			RightButtonID		= 1,
			MiddleButtonID		= 2,
			WheelDeltaUnit		= 0x100,
		} ;
		// マウス識別子を取得（マルチタッチ用）
		static uint32_t GetMouseID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & MouseIDMask) ;
		}
		// マウスボタン識別子を取得
		static uint32_t GetButtonID( int64_t nFlags )
		{
			return	(uint32_t) (nFlags & ButtonIDMask) >> ButtonIDShifter ;
		}
		// タッチパネル入力判定
		static bool IsFromTouch( int64_t nFlags )
		{
			return	((nFlags & TouchFlag) != 0) ;
		}

	public:
		// キー入力
		virtual bool OnKeyDown
			( int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( int64_t nVirtKey, int64_t nFlags ) ;
		// 文字入力
		virtual bool OnChar( uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( void ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( const SGLInputCompositionString& icsComp ) ;

	public:
		enum	CommandPriority
		{
			commandLow		= -5,
			commandBelow	= -1,
			commandNormal	= 0,
			commandAbove	= 1,
			commandHigh		= 5,
		} ;
		// コマンド通知
		virtual bool NotifyCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0,
				int nPriority = commandNormal, bool fOverwritable = false ) ;
		// コマンド発生（コマンド処理／親へ通知）
		virtual bool OnCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0,
				int nPriority = commandNormal, bool fOverwritable = false ) ;
		// コマンド発行処理（フォーカスアイテムへ）
		virtual bool DispatchCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0 ) ;

	protected:
		SSystem::SSharableMutex *	m_pMutexUI ;

	public:
		// スレッド排他処理用
		virtual SSystem::SError Lock
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError LockTrace
			( const char * pszSource,
				size_t nLineNum,
				int64_t msecTimeout = SSystem::Synchronism::Infinite ) const ;
		virtual SSystem::SError Unlock( void ) const ;
		virtual atomic_int_t UnlockAll( void ) const ;
		virtual SSystem::SError Relock( atomic_int_t nLock ) const ;
		virtual atomic_int_t TestLocked( void ) const ;
		// スレッド排他オブジェクト設定
		void SetUIThreadMutex( SSystem::SSharableMutex * pMutex ) ;

	public:
		// Rosetta 用クラス
		virtual const wchar_t * GetRSClassName( void ) const ;
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
		// 復元後処理
		virtual SGLError OnAfterRestore( void ) ;

	} ;

	//////////////////////////////////////////////////////////////////////////
	// パラメータ・アニメーション（ベジェ曲線）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteAction	: public SGLSprite::Action
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteAction, Action )
		// 構築関数
		SGLSpriteAction( void ) ;
		SGLSpriteAction( const SGLSpriteAction& act ) ;

	public:
		SGLBezierCurves<S3DDVector>	m_bzPos ;
		SGLBezierCurves<S2DDVector>	m_bzCenter ;
		SGLBezierCurves<S2DDVector>	m_bzZoom ;
		SGLBezierCurves<double>		m_bzAngle ;
		SGLBezierCurves<double>		m_bzTransparency ;
		SGLBezierCurves<double>		m_bzFilterParam ;
		SGLBezierCurves<double>		m_bzFilterParam2 ;

	public:
		// パラメータ反映
		virtual void EffectParameter
					( SGLSprite::Parameter& param, double t ) ;

	public:
		// 移動アニメーション設定
		void SetMoveTo
			( const SGLSprite& sprite,
				double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
		// 拡大アニメーション設定
		void SetZoomTo
			( const SGLSprite& sprite,
				double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
		// 回転アニメーション設定
		void SetRotationTo
			( const SGLSprite& sprite,
				double z, double a0 = 0.0, double a1 = 0.0 ) ;
		// 透明度アニメーション設定
		void SetTransparencyTo
			( const SGLSprite& sprite, uint32_t nTransparency ) ;
		// フィルタアニメーション設定
		void SetFilterTo
			( const SGLSprite& sprite, uint32_t paramFilter ) ;
		void SetFilter2To
			( const SGLSprite& sprite, uint32_t paramFilter ) ;

	public:
		// 座標
		void SetBezierCurve
			( const SSystem::SArray<S3DDVector>& bzCurve, bool fOffset = false ) ;
		// 中心座標
		void SetCenterCurve
			( const SSystem::SArray<S2DDVector>& bzCurve, bool fOffset = false ) ;
		// 拡大率
		void SetZoomCurve
			( const SSystem::SArray<S2DDVector>& bzCurve, bool fOffset = false ) ;
		// 回転角
		void SetAngleCurve
			( const SSystem::SArray<double>& bzCurve, bool fOffset = false ) ;
		// 透明度
		void SetTransparencyCurve
			( const SSystem::SArray<double>& bzCurve, bool fOffset = false ) ;
		// フィルタパラメータ
		void SetFilterParamCurve
			( const SSystem::SArray<double>& bzCurve, bool fOffset = false ) ;
		void SetFilter2ParamCurve
			( const SSystem::SArray<double>& bzCurve, bool fOffset = false ) ;

	public:
		// 複製（可能なら）
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 疑似 3D カメラ移動アニメーション
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteCameraAction	: public SGLSpriteTimer
	{
	public:
		SGLBezierCurves<S3DDVector>	m_bzPos ;
		SGLBezierCurves<S3DDVector>	m_bzTarget ;
		uint32_t					m_msecDuration ;
		uint32_t					m_msecElapsed ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteCameraAction, SGLSpriteTimer )
		// 構築関数
		SGLSpriteCameraAction( void ) ;
		SGLSpriteCameraAction( const SGLSpriteCameraAction& act ) ;

	public:
		// カメラ座標
		void SetMoveTo
			( const SGLSprite& sprite,
				const S3DDVector& vPos, double a0 = 0.0, double a1 = 0.0 ) ;
		void SetPositionBezier
			( const SSystem::SArray<S3DDVector>& bzCurve ) ;
		// ターゲット座標
		void SetTargetTo
			( const SGLSprite& sprite,
				const S3DDVector& vTarget, double a0 = 0.0, double a1 = 0.0 ) ;
		void SetTargetBezier
			( const SSystem::SArray<S3DDVector>& bzCurve ) ;
		// 開始
		void StartAction( uint32_t msecDuration ) ;
		// アニメーション中か？
		bool IsAction( void ) const ;

	public:	// SGLObject 実装
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	public:	// SGLSpriteTimer
		// タイマー処理（true で終了）
		virtual bool OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像アニメーション
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteAnimator	: public SGLSprite::Imager
	{
	protected:
		SSystem::SString			m_strFilePath ;
		SSystem::SSmartPointer<SGLImageObject>
									m_pAnimation ;
		SSystem::SSmartPointer<SGLImageObject>
									m_pLeftAnimation ;
		SSystem::SArray<uint32_t>	m_tableSeq ;
		ssize_t						m_countLoop ;
		size_t						m_iLoopStart ;
		size_t						m_iLoopEnd ;
		size_t						m_msecDuration ;
		size_t						m_msecFrameDelta ;
		size_t						m_iFrame ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteAnimator, Imager )
		// 構築関数
		SGLSpriteAnimator( void ) ;
		SGLSpriteAnimator( const SGLSpriteAnimator& anime ) ;
		// 関連付け時処理
		virtual void OnAttached( SGLSprite& sprite ) ;
		// アニメーション処理
		virtual void OnAnimation( SGLSprite& sprite, uint32_t msecPast ) ;

	public:
		// 画像ファイル読み込み
		SGLError LoadImage( const wchar_t * pwszFilePath ) ;
		// 画像を関連付ける
		void AttachImage
			( SGLImageObject* pImage, const SGLImageRect * pClip = NULL ) ;
		// アニメーション開始
		void BeginAnimation
			( ssize_t countLoop = -1,
				size_t iLoopStart = 0, size_t iLoopEnd = 0,
				size_t iStartFrame = 0, size_t msecDuration = 0 ) ;
		// ループ設定変更
		void SetLoop
			( ssize_t countLoop = -1,
				size_t iLoopStart = 0, size_t iLoopEnd = 0 ) ;
		// フレーム選択
		void SelectFrame( size_t iFrame ) ;
		// 画像を関連付ける
		void AttachImageToSprite( SGLSprite& sprite ) ;

	public:
		// 複製（可能なら）
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


}

#endif

