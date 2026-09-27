
#if	!defined(__SAKURAGLX_RESOURCE_MANAGER_H__)
#define	__SAKURAGLX_RESOURCE_MANAGER_H__	1

#include <sakuragl/media/sgl_audio_player.h>
#include <sakuragl/sgl_erisa_lib.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// リソース管理
	//////////////////////////////////////////////////////////////////////////

	class	SGLResourceProducer: public SSystem::SObject
	{
	public:
		ESL_DECLARE_CLASS_INFO( SGLResourceProducer, SObject )
		virtual SSystem::SObject * GetResourceAs( const wchar_t * pwszID ) = 0 ;
	} ;


	class	SGLResourceManager	: public SGLObject, public SGLResourceProducer
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( SGLResourceManager, SGLObject, SGLResourceProducer )
		// 構築関数
		SGLResourceManager( void ) ;
		// 消滅関数
		virtual ~SGLResourceManager( void ) ;

	protected:
		SSystem::SStrSortObjectArray<SSystem::SObject>	m_resources ;

	public:
		// リソース追加
		virtual SGLError AddResourceAs
			( const wchar_t * pwszID, SSystem::SObject * pRsrc ) ;
		// リソース削除
		virtual SGLError RemoveResourceAs( const wchar_t * pwszID ) ;
		// 全リソース削除
		virtual void RemoveAllResource( void ) ;
		// 参照されていないリソースを削除する
		virtual void CleanupResource( void ) ;

	public:
		// リソース取得
		virtual SSystem::SObject * GetResourceAs( const wchar_t * pwszID ) ;
		// 画像リソース取得
		virtual SGLImageObject * GetImageAs( const wchar_t * pwszID ) ;
		// 音声リソース取得
		virtual SGLAudioPlayer * GetAudioAs( const wchar_t * pwszID ) ;

	public:
		// リソース配列を取得
		const SSystem::SStrSortObjectArray<SSystem::SObject>&
									GetResourceArray( void ) const
		{
			return	m_resources ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スキン管理
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFormed ;

	class	SGLSkinManager	: public SGLResourceManager
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSkinManager, SGLResourceManager )
		// 構築関数
		SGLSkinManager( void ) ;
		// 消滅関数
		virtual ~SGLSkinManager( void ) ;

	protected:
		SSystem::SCriticalSection								m_csSync ;
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>		m_ssoaResource ;
		SSystem::SStrSortObjectArray<SSystem::SFileInterface>	m_ssoaFile ;
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>		m_ssoaStyle ;
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>		m_ssoaForm ;

	public:
		// スキンファイルを読み込む
		virtual SGLError LoadSkinFile
			( const wchar_t * pwszFilePath, bool fStaticResource = false ) ;
		virtual SGLError ReadSkinFile
			( SSystem::SFileInterface & file, bool fStaticResource = false  ) ;
		// スキンデータを削除する
		virtual void RemoveSkinResource( void ) ;

	public:
		// 規定のスプライト優先度
		enum	ItemPriority
		{
			priorityFirst	= 0x100,
			priorityBG		= 0x7FFFFFFF,
			priorityStep	= 0x10,
		} ;
		// フォームを生成する
		virtual SGLSprite * CreateFormedSprite( const wchar_t * pwszID ) ;
		// フォームを構築する
		virtual SGLError BuildFormedPage
			( SGLSpriteFormed& sprPage, const wchar_t * pwszID ) ;
		// フォーム準備
		virtual SGLError PrepareFormedPage
			( SGLSpriteFormed& sprPage, SSystem::SXMLDocument& xmlForm ) ;

	public:
		// <button> アイテム生成
		virtual SGLSprite * CreateButtonItemOf( const wchar_t * pwszStyleID ) ;

	public:	// アイテムフォームからアイテム生成
		// フォームアイテム追加
		virtual SGLError AddFormedPageItems
			( SGLSpriteFormed& sprPage, SSystem::SXMLDocument& xmlForm ) ;
		// <rectangle> アイテム生成
		virtual SGLSprite * CreateRectangleItem( SSystem::SXMLDocument& xmlItem ) ;
		// <image> アイテム生成
		virtual SGLSprite * CreateImageItem( SSystem::SXMLDocument& xmlItem ) ;
		// <static_frame> アイテム生成
		virtual SGLSprite * CreateStaticFrameItem( SSystem::SXMLDocument& xmlItem ) ;
		// <static_text> アイテム生成
		virtual SGLSprite * CreateStaticTextItem( SSystem::SXMLDocument& xmlItem ) ;
		// <smart_text> アイテム生成
		virtual SGLSprite * CreateSmartTextItem( SSystem::SXMLDocument& xmlItem ) ;
		// <edit_text> アイテム生成
		virtual SGLSprite * CreateEditTextItem( SSystem::SXMLDocument& xmlItem ) ;
		// <progress_bar> アイテム生成
		virtual SGLSprite * CreateProgressBarItem( SSystem::SXMLDocument& xmlItem ) ;
		// <button> アイテム生成
		virtual SGLSprite * CreateButtonItem( SSystem::SXMLDocument& xmlItem ) ;
		// <scroll_bar> アイテム生成
		virtual SGLSprite * CreateScrollBarItem( SSystem::SXMLDocument& xmlItem ) ;

	protected:
		// リソースをリアライズする
		virtual SSystem::SObject * RealizeResource
			( SSystem::SXMLDocument & xmlRsrc,
						SSystem::SFileInterface & file ) ;

	public:
		// スタイルを取得する
		virtual SSystem::SXMLDocument *
				GetStyleAs( const wchar_t * pwszID ) const ;
		// フォームを取得する
		virtual SSystem::SXMLDocument *
				GetFormAs( const wchar_t * pwszID ) const ;

	public:
		// リソース追加
		virtual SGLError AddResourceAs
				( const wchar_t * pwszID, SSystem::SObject * pRsrc ) ;
		// リソース削除
		virtual SGLError RemoveResourceAs( const wchar_t * pwszID ) ;
		// 全リソース削除
		virtual void RemoveAllResource( void ) ;
		// 参照されていないリソースを削除する
		virtual void CleanupResource( void ) ;

	public:
		// リソース取得
		virtual SSystem::SObject * GetResourceAs( const wchar_t * pwszID ) ;
		// 画像リソース取得
		virtual SGLImageObject * GetImageAs( const wchar_t * pwszID ) ;
		// 音声リソース取得
		virtual SGLAudioPlayer * GetAudioAs( const wchar_t * pwszID ) ;

	public:
		// リソース定義配列を取得
		const SSystem::SStrSortObjectArray<SSystem::SXMLDocument>&
									GetResourceDefinitions( void ) const
		{
			return	m_ssoaResource ;
		}
		// スタイル定義配列を取得
		const SSystem::SStrSortObjectArray<SSystem::SXMLDocument>&
									GetStyleDefinitions( void ) const
		{
			return	m_ssoaStyle ;
		}
		// フォーム定義配列を取得
		const SSystem::SStrSortObjectArray<SSystem::SXMLDocument>&
									GetFormDefinitions( void ) const
		{
			return	m_ssoaForm ;
		}

	public:
		struct	ImageDescription
		{
			SGLImageObject *	pImage ;
			SGLImageRect *		pRect ;
			SGLImageRect		rectImage ;

			ImageDescription( void ) : pImage(NULL), pRect(NULL) {}
			ImageDescription( const ImageDescription& src ) ;
			const ImageDescription& operator = ( const ImageDescription& src ) ;
		} ;
		// 画像リソース取得（矩形指定を含む）
		SGLError GetRichImageAs
			( ImageDescription& imgdsc, const wchar_t * pwszID ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易フォーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLBasicFormParser ;
	class	SGLBasicForm	: public SSystem::SObject
	{
	public:
		class	Item ;

		// 画像セット
		class	ImageSet
		{
		public:
			uint32_t								m_msecDuration ;
			SSystem::SPointerArray<const wchar_t>	m_aImageIDs ;
			SSystem::SPointerArray<SGLImageObject>	m_aImages ;
			SSystem::SIndexedArray
				<SSystem::SString,const wchar_t *>	m_aFrameIDs ;
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			ssize_t FindFrameAs( const wchar_t * pwszID ) const ;
		} ;

		// インタラクティブ・リスナ
		enum	MouseButton
		{
			mouseLeft,
			mouseRight,
			mouseMiddle,
		} ;
		class	ItemInteractive	: public ESLObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( ItemInteractive, ESLObject )
			virtual bool OnPostMessage( Item * pItem, int nParam, const wchar_t * pwszOpt = nullptr ) ;
			virtual void OnMouseMove( Item * pItem, const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( Item * pItem ) ;
			virtual bool OnMouseWheel( Item * pItem, const S2DVector& vLocal, float32_t zDelta ) ;
			virtual bool OnClickDown( Item * pItem, const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( Item * pItem, const S2DVector& vLocal, MouseButton button ) ;
		} ;

		// スクロール・リスナ
		class	ScrollListener	: public SSystem::SObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( ScrollListener, SObject )
			virtual void OnScrollPos( uint32_t nPos ) = 0 ;
		} ;

		// タイマー・リスナ
		class	TimerListener	: public SSystem::SObject
		{
		public:
			enum	Result
			{
				resultContinue,
				resultTerminate,
				resultDetach,
			} ;
			ESL_DECLARE_CLASS_INFO( TimerListener, SSystem::SObject )
			virtual Result OnTimer( SGLBasicForm * pForm, uint32_t msecPast ) ;
			virtual Result OnCancel( SGLBasicForm * pForm ) ;
			virtual Result OnFinish( SGLBasicForm * pForm ) ;
		} ;

		// アニメーション
		class	ItemAnimation	: public TimerListener
		{
		protected:
			SSystem::SSyncReference					m_refItem ;
			bool									m_flagLoop ;
			size_t									m_iInterval ;
			uint32_t								m_msecCurrent ;
			SSystem::SArray<uint32_t>				m_aDurations ;
			SGLBezierCurves<S2DVector>				m_bzMove ;
			SGLBezierCurves<S2DVector>				m_bzZoom ;
			SGLBezierCurves<float32_t,float32_t>	m_bzRotation ;
			SGLBezierCurves<float32_t,float32_t>	m_bzTransparency ;
		public:
			ESL_DECLARE_CLASS_INFO( ItemAnimation, TimerListener )
			ItemAnimation( Item * pItem ) ;
			Item * GetItem( void ) const ;
			bool GetLoopFlag( void ) const ;
			void SetLoopFlag( bool flagLoop ) ;
			void AddLinear
				( uint32_t msecDuration,
					const S2DVector* pMove = NULL,
					const uint32_t * pTransparency = NULL,
					const S2DVector * pZoom = NULL,
					const float32_t * pRotation = NULL,
					float32_t fpSpeed0 = 0.0f,
					float32_t fpSpeed1 = 0.0f ) ;
			void AddBezier3Points
				( uint32_t msecDuration,
					const S2DVector* pMove = NULL,
					const float32_t * pTransparency = NULL,
					const S2DVector * pZoom = NULL,
					const float32_t * pRotation = NULL ) ;
			virtual Result OnTimer( SGLBasicForm * pForm, uint32_t msecPast ) ;
			virtual Result OnFinish( SGLBasicForm * pForm ) ;
		public:
			SSystem::SArray<uint32_t>& Intervals( void ) ;
			SGLBezierCurves<S2DVector>& PositionBezier( void ) ;
			SGLBezierCurves<S2DVector>& ZoomingBezier( void ) ;
			SGLBezierCurves<float32_t,float32_t>& RotationBezier( void ) ;
			SGLBezierCurves<float32_t,float32_t>& TransparencyBezier( void ) ;
		} ;

	public:
		// アイテム基底
		class	Item	: public SSystem::SObject
		{
		public:
			enum	ColorEffect
			{
				colorNoEffect,
				colorEffectAdd,
				colorEffectMul,
			} ;
		protected:
			SGLBasicForm *		m_pParentForm ;
			ItemInteractive *	m_pInteractive ;
			const wchar_t *		m_id ;
			SGLImageRect		m_rectOrg ;
			S2DVector			m_vPos ;
			S2DVector			m_vCenter ;
			S2DVector			m_vZoom ;
			float32_t			m_degRotation ;
			bool				m_fVisible ;
			bool				m_fDisabled ;
			uint32_t			m_nTransparency ;
			ColorEffect			m_colorEffect ;
			SGLPalette			m_rgbEffect ;
		public:
			ESL_DECLARE_CLASS_INFO( Item, SObject )
			Item( void ) ;
			// フォーム
			SGLBasicForm * GetParentForm( void ) const ;
			// リスナ
			ItemInteractive * GetInteractive( void ) ;
			void AttachInteractive( ItemInteractive * pInteractive ) ;
			// アイテム ID
			const wchar_t * GetID( void ) const ;
			void AttachID( const wchar_t * pwszID ) ;
			// 元のアイテム位置とサイズ
			const SGLImageRect& GetItemOrgRect( void ) const ;
			void SetItemOrgRect( const SGLImageRect& rect ) ;
			// 座標
			const S2DVector& GetPosition( void ) const ;
			const S2DVector& GetCenter( void ) const ;
			void SetPosition( const S2DVector& vPos ) ;
			void SetCenter( const S2DVector& vCenter ) ;
			// 拡大と回転
			const S2DVector& GetZoom( void ) const ;
			float32_t GetRotation( void ) const ;
			void SetZoom( const S2DVector& vZoom ) ;
			void SetRotation( float32_t degRot ) ;
			// アフィン行列・逆行列
			SGLAffine GetAffine( void ) const ;
			SGLAffine GetIAffine( void ) const ;
			S2DVector LocalToGlobal( const S2DVector& vLocal ) const ;
			S2DVector GlobalToLocal( const S2DVector& vGlobal ) const ;
			// 表示状態
			bool IsVisible( void ) const ;
			void SetVisible( bool fVisible ) ;
			// 禁止状態
			virtual bool IsDisabled( void ) const ;
			virtual void SetDisable( bool fDisable ) ;
			// 透明度
			uint32_t GetTransparency( void ) const ;
			void SetTransparency( uint32_t nTransparency ) ;
			// 色効果
			ColorEffect GetColorEffect( SGLPalette& rgbEffect ) const ;
			void SetColorEffect( ColorEffect colorEffect, const SGLPalette& rgbEffect ) ;
		public:
			// バー位置
			virtual uint32_t GetBarPos( void ) const ;
			virtual void SetBarPos( uint32_t nPos ) ;
			// バー値範囲
			virtual uint32_t GetBarRange( void ) const ;
			virtual void SetBarRange( uint32_t nRange ) ;
		public:
			// 描画パラメータ取得
			void GetPaintParam( SGLPaintParam& pp, SGLAffine& affine ) ;
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			// タイマー処理
			virtual bool OnTimer( uint32_t msecPast ) ;
			// マウス当たり判定
			virtual bool TestHitCursor( const S2DVector& vGlobal ) const ;
			virtual Item * GetHitItem( S2DVector& vHitLocal, const S2DVector& vGlobal ) ;
			// マウスキャプチャー
			void SetMouseCapture( void ) ;
			void ReleaseMouseCapture( void ) ;
			// インタラクティブ
			virtual void OnMouseMove( const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( void ) ;
			virtual bool OnMouseWheel( const S2DVector& vLocal, float32_t zDelta ) ;
			virtual bool OnClickDown( const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( const S2DVector& vLocal, MouseButton button ) ;
			// 描画更新通知
			void NotifyUpdate( void ) ;
			// メッセージ通知
			bool NotifyMessage( Item * pItem, int nParam, const wchar_t * pwszOpt = nullptr ) ;

			friend class	SGLBasicForm ;
		} ;

		// 画像アイテム
		class	Image	: public Item
		{
		protected:
			SGLImageObject *	m_pImage ;
			SSystem::SObjectArray<SGLImageObject>
								m_aTempFrames ;
			uint32_t			m_msecAnime ;
			bool				m_fClickable ;
			bool				m_fTestAlpha ;
		public:
			ESL_DECLARE_CLASS_INFO( Image, Item )
			Image( void ) ;
			// 画像
			SGLImageObject * GetImage( void ) const ;
			void AttachImage( SGLImageObject * pImage ) ;
			// マウス当たり判定有効化
			bool IsClickable( void ) const ;
			bool IsTestAlpha( void ) const ;
			void SetClickable( bool fClickable ) ;
			void SetTestAlpha( bool fTestAlpha ) ;
		public:
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			// タイマー処理
			virtual bool OnTimer( uint32_t msecPast ) ;
			// マウス当たり判定
			virtual bool TestHitCursor( const S2DVector& vGlobal ) const ;
			virtual bool TestHitImage( const S2DVector& vLocal ) const ;
		} ;

		// 画像セレクタアイテム
		class	ImageSelector	: public Image
		{
		protected:
			const ImageSet *	m_pImageSet ;
			size_t				m_iSelector ;
		public:
			ESL_DECLARE_CLASS_INFO( ImageSelector, Image )
			ImageSelector( void ) ;
			// 画像
			const ImageSet * GetImageSet( void ) const ;
			void AttachImageSet( const ImageSet * pImageSet ) ;
			// 値（画像選択）
			size_t GetSelector( void ) const ;
			void SetSelector( size_t iSel ) ;
			// 画像選択（IDで）
			bool SelectImageAs
				( const wchar_t * pwszID, size_t iDefault = 0,
					const wchar_t *const* ppwszCandidates = nullptr, size_t nCandidates = 0 )  ;
		} ;

		// ボタンアイテム
		class	Button	: public ImageSelector
		{
		public:
			enum	StatusIndex
			{
				statusNormal,
				statusFocus,
				statusPushed,
				statusPushedFocus,
				statusPushing,
				statusDisable,
				statusPushedDisable,
				statusIndexCount,
			} ;
			static const wchar_t *const	s_pwszStatusID[statusIndexCount] ;
			static const StatusIndex	s_statusSubstitute[statusIndexCount][3] ;
		protected:
			bool	m_flagFocus ;			// マウスヒット状態
			bool	m_flagPushing ;			// マウス押下状態
			bool	m_flagPushed ;			// ボタントグル状態
			bool	m_flagToggle ;			// トグルボタン
			size_t	m_nClicked ;			// クリックカウンタ
		public:
			ESL_DECLARE_CLASS_INFO( Button, ImageSelector )
			Button( void ) ;
			// 禁止状態
			virtual void SetDisable( bool flagDisable ) ;
			// トグルボタン
			bool IsToggleButton( void ) const ;
			void SetToggleButton( bool flagToggle ) ;
			void SetTogglePushed( bool flagPushed ) ;
			// ステータス取得
			bool IsPushing( void ) const ;
			bool IsPushed( void ) const ;
			StatusIndex GetStatus( void ) const ;
			// クリック回数
			size_t GetClickedCount( void ) const ;
			void ResetClickedCount( void ) ;
			// ステータスに対応する画像設定
			void SelectStatusImage( StatusIndex status ) ;
			StatusIndex GetSubstituteStatus( StatusIndex status ) const ;
		public:
			// インタラクティブ
			virtual void OnMouseMove( const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( void ) ;
			virtual bool OnClickDown( const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( const S2DVector& vLocal, MouseButton button ) ;
		} ;

		// ゲージ・バー（プログレス・バー）
		class	GaugeBar	: public Image
		{
		protected:
			bool		m_flagVertical ;
			bool		m_flagInverse ;
			uint32_t	m_nPos ;
			uint32_t	m_nRange ;
		public:
			ESL_DECLARE_CLASS_INFO( GaugeBar, Image )
			GaugeBar( void ) ;
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			// 表示スタイル
			bool IsVerticalBar( void ) const ;
			bool IsInverseBar( void ) const ;
			void SetBarStyle( bool flagVert, bool flagInverse ) ;
			// バー位置
			virtual uint32_t GetBarPos( void ) const ;
			virtual void SetBarPos( uint32_t nPos ) ;
			// バー値範囲
			virtual uint32_t GetBarRange( void ) const ;
			virtual void SetBarRange( uint32_t nRange ) ;
		} ;

		// トラック・バー（スクロール・バー）
		class	TrackBar	: public ImageSelector
		{
		public:
			enum	StatusIndex
			{
				statusNormal,
				statusFocus,
				statusDisable,
				statusIndexCount,
			} ;
			static const wchar_t *const	s_pwszStatusID[statusIndexCount] ;
		protected:
			SSystem::SSmartReference<ImageSelector>		m_refTrack ;
			SSystem::SSmartReference<GaugeBar>			m_refGauge ;
			SSystem::SReferenceArray<ScrollListener>	m_aListener ;
			bool		m_flagVertical ;
			bool		m_flagTracking ;
			S2DVector	m_vTrackingOffset ;
			uint32_t	m_nPos ;
			uint32_t	m_nRange ;
		public:
			ESL_DECLARE_CLASS_INFO( TrackBar, ImageSelector )
			TrackBar( void ) ;
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			// 禁止状態
			virtual void SetDisable( bool flagDisable ) ;
			// 表示スタイル
			bool IsVerticalBar( void ) const ;
			void SetBarStyle( bool flagVert ) ;
			// バー位置
			virtual uint32_t GetBarPos( void ) const ;
			virtual void SetBarPos( uint32_t nPos ) ;
			// バー値範囲
			virtual uint32_t GetBarRange( void ) const ;
			virtual void SetBarRange( uint32_t nRange ) ;
			// 背景トラックアイテム設定
			void AttachTrackItem( ImageSelector * pTrack ) ;
			void AttachGaugeBar( GaugeBar * pGauge ) ;
			// リスナ設定
			void AttachScrollListener( ScrollListener * pListener ) ;
			void DetachScrollListener( ScrollListener * pListener ) ;
			// ステータス画像設定
			void SelectStatusImage( StatusIndex status ) ;
			static void SelectStatusImage
				( ImageSelector& imgsel, StatusIndex status ) ;
		public:
			// マウス当たり判定
			virtual bool TestHitCursor( const S2DVector& vGlobal ) const ;
			// インタラクティブ
			virtual void OnMouseMove( const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( void ) ;
			virtual bool OnClickDown( const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( const S2DVector& vLocal, MouseButton button ) ;
		protected:
			void OnTrackMoved( uint32_t nPos ) ;
			uint32_t TrackPosFromLocalPos( const S2DVector& vLocal ) const ;
			S2DVector LocalPosFromTrackPos( uint32_t nPos ) const ;
		} ;

		// 伸縮フレーム
		class	StretchFrame	: public Item
		{
		protected:
			enum	PartsOrder
			{
				orderLeft	= 0,
				orderCenterLeft,
				orderCenter,
				orderCenterRight,
				orderRight,
				orderCount,
				orderUpper			= orderLeft,
				orderCenterUpper	= orderCenterLeft,
				orderCenterUnder	= orderCenterRight,
				orderUnder			= orderRight,
				orderInvalid		= -1,
			} ;
			enum	PartsIndex
			{
				partsUpperLeft,
				partsUpperCenterLeft,
				partsUpperCenter,
				partsUpperCenterRight,
				partsUpperRight,
				partsLeftUpper,
				partsLeft,
				partsLeftUnder,
				partsRightUpper,
				partsRight,
				partsRightUnder,
				partsUnderLeft,
				partsUnderCenterLeft,
				partsUnderCenter,
				partsUnderCenterRight,
				partsUnderRight,
				partsBackFrame,
				partsCount,
				partsInvalid	= -1,
			} ;
			const ImageSet *	m_pImageSet ;
			SGLImageObject *	m_pImageParts[partsCount] ;
			int32_t				m_wHorz[orderCount] ;
			int32_t				m_hVert[orderCount] ;
			SGLRect				m_rectInnerMargin ;
			SGLRect				m_rectBackMargin ;
			SGLSize				m_sizeFrame ;

			static const wchar_t *	s_pwszPartsID[partsCount] ;
			static const PartsIndex	s_partsInOrders[partsCount][partsCount] ;
			static const PartsOrder	s_orderHorzParts[partsCount] ;
			static const PartsOrder	s_orderVertParts[partsCount] ;

		public:
			ESL_DECLARE_CLASS_INFO( StretchFrame, Item )
			StretchFrame( void ) ;
			// 画像
			const ImageSet * GetImageSet( void ) const ;
			void AttachImageSet( const ImageSet * pImageSet ) ;
			// クライアント領域マージン
			const SGLRect& GetInnerMargin( void ) const ;
			void SetInnerMargin( const SGLRect& rectMargin ) ;
			// 背景画像マージン
			const SGLRect& GetBackFrameMargin( void ) const ;
			void SetBackFrameMargin( const SGLRect& rectMargin ) ;
			// 表示サイズ（外接矩形）
			const SGLSize& GetFrameSize( void ) const ;
			void SetFrameSize( const SGLSize& sizeFrame ) ;
			// サイズ計算（内接→外接矩形）
			SGLSize CalcExFrameSize( const SGLSize& sizeInner ) const ;
			// 内接矩形取得（ローカル座標）
			SGLImageRect GetInnerFrameRect( void ) const ;
			SGLImageRect GetInnerFrameImageRect( void ) const ;
			// 最小サイズ計算（外接）
			int32_t GetMinFrameWidth( void ) const ;
			int32_t GetMinFrameHeight( void ) const ;
		public:
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			
		} ;

		// フォーム
		class	SubForm	: public Item, public ScrollListener
		{
		protected:
			SSystem::SSmartPointer<SGLBasicForm>	m_pForm ;
			SSystem::SSmartReference<TrackBar>		m_refTrack ;
			S3DMaterial			m_material ;
			S3DRenderContext	m_render ;
			SGLImage			m_imgBuffer ;
			SGLImage			m_imgMasked ;
			SGLImageObject *	m_pAlpha ;
			SGLRect				m_rectMargin ;
			float32_t			m_fpScrollDelta ;
			S2DVector			m_vScroll ;
			S2DVector			m_vSwipeGrip ;
			bool				m_fBuffered ;
			bool				m_fAlphaMask ;
			bool				m_fScrollable ;
			bool				m_fVertScroll ;
			bool				m_fSwipping ;
		public:
			ESL_DECLARE_CLASS_INFO( SubForm, Item )
			SubForm( void ) ;
			// フォーム
			SGLBasicForm * GetForm( void ) const ;
			void SetForm( SGLBasicForm * pForm ) ;
			// バッファ作成
			SGLError CreateBuffer
				( uint32_t nWidth, uint32_t nHeight,
					uint64_t nBufFlags = SGLImageObject::bufferOnDeviceOnly ) ;
			// αマスク設定
			SGLError SetAlphaMask
				( SGLImageObject * pAlpha,
					uint64_t nBufFlags = SGLImageObject::bufferOnDeviceOnly ) ;
			// バッファ解放
			void ReleaseBuffer( void ) ;
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			void RenderBuffer( void ) ;
			// タイマー処理
			virtual bool OnTimer( uint32_t msecPast ) ;
			// マウス当たり判定
			virtual bool TestHitCursor( const S2DVector& vGlobal ) const ;
			virtual Item * GetHitItem( S2DVector& vHitLocal, const S2DVector& vGlobal ) ;
			// インタラクティブ
			virtual void OnMouseMove( const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( void ) ;
			virtual bool OnMouseWheel( const S2DVector& vLocal, float32_t zDelta ) ;
			virtual bool OnClickDown( const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( const S2DVector& vLocal, MouseButton button ) ;
			// ローカル座標からフォーム上の座標へ
			S2DVector LocalFormPosFrom( const S2DVector& vLocal ) const ;
		public:
			// スクロールが有効か？
			bool IsScrollable( void ) const ;
			void SetScrollable( bool fScrollable ) ;
			void SetScrollVertical( bool fVert ) ;
			// ホイールスクロール単位
			float32_t GetWheelScrollDelta( void ) const ;
			void SetWheelScrollDelta( float32_t fpDelta ) ;
			// 表示マージン設定
			void SetFormMargin( const SGLRect& rectMargin ) ;
			// スクロール用トラックバー関連付け
			void AttachTarckBar( TrackBar * pBar ) ;
			// スクロール位置更新
			void UpdateScrollPos( const S2DVector& vScroll, bool flagTrackBar ) ;
			// スクロールバーからの通知
			virtual void OnScrollPos( uint32_t nPos ) ;
		} ;

		// Sprite
		class	Sprite	: public Item
		{
		public:
			class	StubSprite	: public SGLSprite
			{
			protected:
				Sprite *	m_pOwner ;
			public:
				ESL_DECLARE_CLASS_INFO( StubSprite, SGLSprite )
				StubSprite( Sprite * pOwner ) ;
				Sprite * GetOwnerItem( void ) const ;
				virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
				virtual bool NotifyCommand
					( const wchar_t * pszCmd,
						int64_t nParam = 0, int64_t nCode = 0,
						int nPriority = commandNormal, bool fOverwritable = false ) ;
				virtual SGLError SetMouseCapture( void ) ;
				virtual SGLError ReleaseMouseCapture( void ) ;
			} ;
		protected:
			SSystem::SSmartPointer<SGLSprite>	m_pStub ;
			SSystem::SSmartPointer<SGLSprite>	m_pSprite ;
		public:
			ESL_DECLARE_CLASS_INFO2( Sprite, Item, SGLSprite )
			Sprite( void ) ;
			// Sprite
			SGLSprite * GetSprite( void ) const ;
			void SetSprite( SGLSprite * pSprite ) ;
			// 描画パラメータ
			virtual void AppendDrawParam( SGLDrawImageParamList& dipl ) ;
			// タイマー処理
			virtual bool OnTimer( uint32_t msecPast ) ;
			// マウス当たり判定
			virtual bool TestHitCursor( const S2DVector& vGlobal ) const ;
			// インタラクティブ
			virtual void OnMouseMove( const S2DVector& vLocal ) ;
			virtual void OnMouseLeave( void ) ;
			virtual bool OnMouseWheel( const S2DVector& vLocal, float32_t zDelta ) ;
			virtual bool OnClickDown( const S2DVector& vLocal, MouseButton button ) ;
			virtual bool OnClickUp( const S2DVector& vLocal, MouseButton button ) ;
		protected:
			static int64_t MouseFlagFromButton( MouseButton button ) ;
		} ;

	protected:
		SSystem::SCriticalSection			m_csSync ;
		SSystem::SSyncReference				m_refFormParser ;
		SubForm *							m_pOwnerItem ;
		SGLSprite *							m_pOwnerSprite ;
		SGLImageRect						m_rectForm ;
		bool								m_flagUpdate ;
		SSystem::SStrSortObjectArray<Item>	m_ssoaItems ;
		SSystem::SObjectArray<Item>			m_aNoNamed ;
		SSystem::SPointerArray<Item>		m_aItems ;

		friend class	SubForm ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLBasicForm, SObject )
		// 構築関数
		SGLBasicForm( void ) ;
		// 消滅関数
		virtual ~SGLBasicForm( void ) ;

	public:
		// SGLBasicFormParser 取得
		const SGLBasicFormParser * GetFormParser( void ) const ;
		void AttachFormParser( const SGLBasicFormParser * pFormParser ) ;
		// オーナーアイテム取得
		SubForm * GetOwnerItem( void ) const ;
		SGLSprite * GetOwnerSprite( void ) const ;
		// オーナースプライト設定
		void AttachOwnerSprite( SGLSprite * pOwner ) ;
		// 矩形
		const SGLImageRect& GetFormRect( void ) const ;
		void SetFormRect( const SGLImageRect& rect ) ;

	public:
		// アイテム取得
		Item * GetItemAs( const wchar_t * pwszID ) const ;
		template <class T> T * GetItem( const wchar_t * pwszID ) const
		{
			return	ESLTypeCast<T>( GetItemAs( pwszID ) ) ;
		}
		Item * GetItemAt( size_t nIndex ) const ;
		// アイテム数取得
		size_t GetItemCount( void ) const ;
		// アイテム追加
		void AddItem( Item * pItem, const wchar_t * pwszID = NULL ) ;
		void InsertItem( size_t nIndex, Item * pItem, const wchar_t * pwszID = NULL ) ;
		// アイテム順序取得
		ssize_t FindItemIndex( Item * pItem ) const ;
		// アイテム削除
		void RemoveItem( Item * pItem ) ;
		void RemoveAllItems( void ) ;

	public:
		// 描画
		void DrawForm( SGLDrawImageParamList& dipl ) ;

	public:
		// マウス当たり判定
		bool TestHitCursor( const S2DVector& vGlobal ) const ;
		Item * GetHitItem( S2DVector& vHitLocal, const S2DVector& vGlobal ) ;

	protected:
		SSystem::SSyncReference		m_refLastHit ;
		SSystem::SSyncReference		m_refCaptured ;

	public:
		// 描画更新通知
		void SetUpdateFlag( void ) ;
		void ResetUpdateFlag( void ) ;
		bool GetUpdateFlag( void ) const ;
		// マウスキャプチャー
		void SetMouseCapture( Item * pItem ) ;
		void ReleaseMouseCapture( void ) ;
		Item * GetMouseCapture( void ) const ;
		// インタラクティブ
		bool OnMouseMove( const S2DVector& vGlobal ) ;
		void OnMouseLeave( void ) ;
		bool OnMouseWheel( const S2DVector& vGlobal, float32_t zDelta ) ;
		bool OnClickDown( const S2DVector& vGlobal, MouseButton button ) ;
		bool OnClickUp( const S2DVector& vGlobal, MouseButton button ) ;
	protected:
		Item * GetHitItemOnMouse( S2DVector& vHitLocal, const S2DVector& vGlobal ) ;

	public:
		// 通知受け取り
		virtual bool OnPostMessage
			( Item * pItem, int nParam, const wchar_t * pwszOpt = NULL ) ;

	protected:
		SSystem::SObjectArray<TimerListener>	m_aTimers ;

	public:
		// タイマー処理
		bool OnTimer( uint32_t msecPast ) ;
		// タイマー追加
		void AddTimer( TimerListener * pTimer ) ;
		// タイマーキャンセル
		void CancelTimer( TimerListener * pTimer ) ;
		void CancelAllTimers( void ) ;
		// タイマー処理強制終了
		void FinishTimer( TimerListener * pTimer ) ;
		void FinishAllTimers( void ) ;
	protected:
		void ProcessTimerResult
			( size_t iTimer, TimerListener * pTimer, TimerListener::Result result ) ;

	public:
		// アイテムのアニメーション追加
		ItemAnimation * AddItemAnimation
			( Item * pItem, uint32_t msecDuration,
				const S2DVector * pMove = NULL,
				const uint32_t * pTransparency = NULL,
				const S2DVector * pZoom = NULL,
				const float32_t * pRotation = NULL,
				float32_t fpSpeed0 = 0.0f, float32_t fpSpeed1 = 0.0f ) ;
		ItemAnimation * AddItemMoveAnimation
			( Item * pItem, uint32_t msecDuration,
				const S2DVector& vMove,
				const uint32_t nTransparency,
				const S2DVector * pZoom = NULL,
				const float32_t * pRotation = NULL,
				float32_t fpSpeed0 = 0.0f, float32_t fpSpeed1 = 0.0f ) ;
		// アイテムのアニメーション取得
		ItemAnimation * FindItemAnimation( Item * pItem ) const ;
		// アイテムのアニメーションキャンセル
		void CancelAnimation( Item * pItem ) ;
		// アイテムのアニメーション即時終了
		void FinishAnimation( Item * pItem ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易フォーム・パーサー
	//////////////////////////////////////////////////////////////////////////

	class	SGLBasicFormParser	: public SGLResourceProducer
	{
	public:
		class	ResourceInfo
		{
		public:
			SSystem::SString		m_strType ;
			SSystem::SString		m_strSrc ;
			SSystem::SSyncReference	m_refRsrc ;
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
		} ;

	protected:
		SSystem::SStrSortArray<SGLImageRect>					m_ssaRects ;
		SSystem::SStrSortObjectArray<ResourceInfo>				m_ssoaRsrcs ;
		SSystem::SStrSortObjectArray<SGLBasicForm::ImageSet>	m_ssoaImageSets ;
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>		m_ssoaStyles ;
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>		m_ssoaForms ;
		SSystem::SSmartReference<SGLResourceProducer>			m_refRsrcProducer ;
		SSystem::SSmartPointer<ERISA::SGLArchiveFile>			m_pArcFile ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLBasicFormParser, SGLResourceProducer )
		// 構築関数
		SGLBasicFormParser( void ) ;
		// 消滅関数
		virtual ~SGLBasicFormParser( void ) ;

	public:
		// パッケージ（書庫）ファイルを開く
		SGLError OpenPackage( const wchar_t * pwszArcFile ) ;
		// パッケージ（書庫）ファイルを閉じる
		void ClosePackage( void ) ;

	public:
		// 読み込み
		SGLError LoadForm
			( const wchar_t * pwszFilePath,
				SGLResourceProducer * pRsrcProducer = NULL ) ;
		SGLError ReadForm
			( SSystem::SFileInterface& file,
				SGLResourceProducer * pRsrcProducer = NULL ) ;
		// 解釈
		SGLError ParseForm
			( const SSystem::SXMLDocument& xmlDoc,
				SGLResourceProducer * pRsrcProducer = NULL ) ;
		// リソース参照更新
		SGLError UpdateResourceRef( SGLResourceProducer * pRsrcProducer = NULL ) ;

	public:
		// 画像リソースのアトラス化
		size_t BuildImageAtlas
			( size_t nMaxAtlasSize = 2048, size_t nReqBatchCount = 3 ) ;

	public:
		// 矩形情報取得
		const SGLImageRect * GetRectAs( const wchar_t * pwszID ) const ;
		// リソース情報取得
		const ResourceInfo * GetResourceInfoAs( const wchar_t * pwszID ) const ;
		SGLImageObject * GetRsrcImageAs( const wchar_t * pwszID ) const ;
		// 画像セット取得
		const SGLBasicForm::ImageSet * GetImageSetAs( const wchar_t * pwszID ) const ;
		// 画像セットID取得
		const wchar_t * GetImageSetIDOf( const SGLBasicForm::ImageSet * pImageSet ) const ;
		// スタイル取得
		const SSystem::SXMLDocument * GetStyleAs( const wchar_t * pwszID ) const ;
		// フォーム情報取得
		const SSystem::SXMLDocument * GetFormAs( const wchar_t * pwszID ) const ;
		// フォームID列挙
		void EnumerateFormIDs( SSystem::SObjectArray<SSystem::SString>& aIDs ) const ;

	public:
		// リソース・プロデューサー
		SGLResourceProducer * GetResourceProducer( void ) const ;
		void AttachResourceProducer( SGLResourceProducer * pRsrcProducer ) ;
		// リソース読み込み
		virtual SSystem::SObject* LoadResource
			( const SSystem::SString& strType, const SSystem::SString& strFile ) ;
		// リソース取得
		virtual SSystem::SObject* GetResourceAs( const wchar_t * pwszID ) ;
		virtual SSystem::SObject* GetResourceAs
			( const wchar_t * pwszID,
				SGLResourceProducer * pRsrcProducer ) const ;
		virtual SGLImageObject * GetImageAs
			( const wchar_t * pwszID,
				SGLResourceProducer * pRsrcProducer = nullptr ) const ;
		virtual SGLAudioPlayer * GetSoundAs
			( const wchar_t * pwszID,
				SGLResourceProducer * pRsrcProducer = nullptr ) const ;

	public:
		// フォーム構築
		SGLError BuildFormAs
			( SGLBasicForm& form, const wchar_t * pwszID,
				SGLResourceProducer * pRsrcProducer = NULL ) const ;
		SGLError BuildForm
			( SGLBasicForm& form,
				const SSystem::SXMLDocument * pxmlForm,
				SGLResourceProducer * pRsrcProducer = NULL ) const ;
		// アイテム構築
		SGLBasicForm::Item * BuildItemAs
			( const SGLImageRect& rectItem,
				const wchar_t * pwszStyleID,
				const SGLBasicForm * pParentForm = nullptr,
				SGLResourceProducer * pRsrcProducer = nullptr ) const ;
		SGLBasicForm::Item * BuildItem
			( const SGLImageRect& rectItem,
				const SSystem::SXMLDocument * pxmlStyle,
				const SGLBasicForm * pParentForm = nullptr,
				SGLResourceProducer * pRsrcProducer = nullptr ) const ;

	} ;

}

#endif

