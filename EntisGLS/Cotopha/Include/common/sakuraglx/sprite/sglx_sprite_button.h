
#if	!defined(__SAKURAGLX_SPRITE_BUTTON_H__)
#define	__SAKURAGLX_SPRITE_BUTTON_H__	1

#include <sakuraglx/sprite/sglx_sprite_text.h>


namespace	SakuraGL
{
	class	SGLSpriteButton ;

	//////////////////////////////////////////////////////////////////////////
	// スプライトボタンリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteButtonListener	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteButtonListener, SObject )
		// ボタンが押された
		virtual bool OnButtonPushed( SGLSpriteButton& button, bool fRepeat ) ;
		// ボタンのステータスが変化した
		virtual bool OnChangedButtonStatus( SGLSpriteButton& button ) ;
		// ドラッグが開始した
		virtual void OnBeginDrag
			( SGLSpriteButton& button, double xOffset, double yOffset ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ボタンスプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteButton	: public SGLSprite
	{
	public:
		// ボタンスタイル
		enum	ButtonType
		{
			typeNormal,
			typeCheck,
			typeRadio,
		} ;
		enum	ButtonStatus
		{
			statusInvalid	= -1,
			statusNormal,
			statusFocus,
			statusPushed,
			statusPushedFocus,
			statusActive,
			statusPushedActive,
			statusDisabled,
			statusPushDisabled,
			statusCount,
		} ;
		enum	ButtonStatusFlag
		{
			flagNormal			= (1 << statusNormal),
			flagFocus			= (1 << statusFocus),
			flagPushed			= (1 << statusPushed),
			flagPushedFocus		= (1 << statusPushedFocus),
			flagActive			= (1 << statusActive),
			flagPushedActive	= (1 << statusPushedActive),
			flagDisabled		= (1 << statusDisabled),
			flagPushDisabled	= (1 << statusPushDisabled),
			flagNormal4Buttons	= flagNormal | flagFocus
									| flagPushed | flagDisabled,
			flagAllStatus		= flagNormal | flagFocus
									| flagPushed | flagPushedFocus
									| flagActive | flagPushedActive
									| flagDisabled | flagPushDisabled,
		} ;
		struct	ButtonStyle
		{
			ButtonType							typeButton ;
			bool								flagHitRect ;
			uint32_t							maskStatus ;
			SGLSkinManager::ImageDescription	imgdscMask ;
			SGLSkinManager::ImageDescription	imgdscButton[statusCount] ;
			SGLSpriteText::TextStyle			textStyle[statusCount] ;
			SGLPalette							rgbaBackColor[statusCount] ;

			// 構築関数（デフォルト値）
			ButtonStyle( void ) ;
			// 構築関数（複製）
			ButtonStyle( const ButtonStyle& style ) ;
			// 代入
			const ButtonStyle& operator = ( const ButtonStyle& style ) ;
		} ;

	protected:
		SSystem::SSmartReference<SGLImageObject>	m_refMask ;
		SSystem::SSmartReference<SGLImageObject>	m_refButton[statusCount] ;
		SSystem::SSmartReference<SGLSpriteButtonListener>
													m_refButtonListener ;
		SSystem::SSmartReference<SGLAudioPlayerInterface>
													m_refFocusSE ;
		SSystem::SSmartReference<SGLAudioPlayerInterface>
													m_refPushedSE ;

		ButtonStatus		m_statusButton ;
		ButtonStatus		m_statusView ;

		ButtonStyle			m_styleButton ;
		SSystem::SString	m_strFontFace[statusCount] ;

		SGLSize				m_sizeButton ;
		SSystem::SString	m_strText ;
		bool				m_flagChecked ;
		bool				m_flagButtonActive ;
		bool				m_flagKeyActive ;
		uint32_t			m_idActiveMouse ;

		SSystem::SSmartPointer<SGLImageObject>	m_pTextImage[statusCount] ;
		SGLPoint								m_ptTextOffset[statusCount] ;

		bool				m_flagPushRepeat ;
		uint32_t			m_msecBeforeRepeat ;
		uint32_t			m_msecRepeatInterval ;
		uint32_t			m_countPushRepeat ;
		uint32_t			m_msecPastLastClicked ;

		bool				m_flagRightClick ;
		bool				m_flagStatusNotification ;
		int64_t				m_nRightClickParam ;
		int64_t				m_nStatusNotifyParam ;

		bool				m_flagDraggable ;
		bool				m_flagDragged ;
		int32_t				m_thresholdDrag ;
		S2DDVector			m_vDragStart ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteButton, SGLSprite )
		// 構築関数
		SGLSpriteButton( void ) ;
		SGLSpriteButton( const SGLSpriteButton& src ) ;
		// 消滅関数
		virtual ~SGLSpriteButton( void ) ;

	public:
		// ボタンリスナを設定
		void AttachButtonListener( SGLSpriteButtonListener * pListener ) ;
		void SetSmartButtonListener( SGLSpriteButtonListener * pListener ) ;
		// SE を設定
		void AttachSoundEffect
			( SGLAudioPlayerInterface * pFocusSE,
					SGLAudioPlayerInterface * pPushedSE ) ;
		// ボタンを生成
		SGLError CreateSimpleButton
			( SGLImageObject** ppImages,
				bool flagHitRect, SGLImageObject* pHitMask,
				const SGLSize& sizeButton,
				const SGLSpriteText::TextStyle& textStyle,
				const SGLPalette* pTextColors,
				const SGLPalette* pBackColors,
				uint32_t maskStatus = flagNormal4Buttons,
				ButtonType typeButton = typeNormal ) ;
		// シンプルな画像ボタンを生成
		SGLError CreateSimpleImageButton
			( SGLImageObject** ppImages,
				bool flagHitRect = false,
				SGLImageObject* pHitMask = NULL,
				uint32_t maskStatus = flagNormal4Buttons,
				ButtonType typeButton = typeNormal ) ;
		// シンプルなテキストボタンを生成
		SGLError CreateSimpleTextButton
			( const SGLSpriteText::TextStyle& textStyle,
				const SGLSize& sizeTextExt,
				const SGLPalette* pTextColors,
				const SGLPalette* pBackColors,
				uint32_t maskStatus = flagNormal4Buttons,
				ButtonType typeButton = typeNormal ) ;
		// シンプルな矩形ボタンを生成
		SGLError CreateSimpleRectButton
			( const SGLSize& sizeRect,
				const SGLPalette* pColors,
				uint32_t maskStatus = flagNormal4Buttons,
				ButtonType typeButton = typeNormal ) ;

	protected:
		// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;

	public:
		// ボタンスタイル取得
		const ButtonStyle& GetButtonStyle( void ) const
		{
			return	m_styleButton ;
		}
		// ボタンスタイル設定
		void SetButtonStyle( const ButtonStyle& style ) ;
		// ボタンサイズ（当たり判定・背景色塗りつぶし矩形）設定
		// 空の場合には画像・テキスト矩形が使用される
		void SetButtonSize( const SGLSize& sizeButton ) ;
		// ボタンステータス取得
		ButtonStatus GetButtonStatus( void ) const
		{
			return	m_statusButton ;
		}
		// ボタンステータス設定
		void SetButtonStatus( ButtonStatus status ) ;
		// ボタンのステータス画像を取得
		SGLImageObject * NewButtonImageReference( ButtonStatus status ) const ;

	public:
		// チェック・キーフォーカス・禁止状態をステータスに加味する
		ButtonStatus EffectStatus( ButtonStatus status ) ;
		// 有効な表示用ステータスを取得する
		static ButtonStatus ValidStatusView
				( uint32_t maskStatus, ButtonStatus status ) ;
		// ボタン画像を更新する
		void UpdateButtonImage( void ) ;
		// ボタンの表示を更新する
		void UpdateButtonView( void ) ;

	public:
		// ボタンスタイルを解釈する
		static void ParseButtonStyle
			( SGLSkinManager& skin,
				ButtonStyle& style, SSystem::SString* pstrFontFace,
							const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;
		// 入力禁止状態
		virtual void SetEnable( bool fEnable ) ;
		// ボタン属性
		virtual bool IsButtonChecked( void ) ;
		virtual void CheckButton( bool fCheck ) ;
		virtual bool IsButtonPushing( void ) ;
		// コマンド処理
		virtual SGLError InvokeCommand
			( const SSystem::SXMLDocument& xmlCmd,
				SSystem::SXMLDocument * pxmlResult = NULL ) ;
		// リピート機能設定
		void SetButtonRepeat
			( bool flagRepeat,
				uint32_t msecBefore, uint32_t msecInterval ) ;
		// 右クリック設定
		void SetRightClickNotify
			( bool flagRightClick, int64_t nRightClickParam ) ;
		// ステータス通知設定
		void SetStatusNotification
			( bool flagNotify, int64_t nNotifyParam ) ;
		// ドラッグ設定
		void EnableDrag( bool flagDraggable, int32_t nThreshold = 8 ) ;

	public:
		// フォーカスが設定された
		virtual void OnSetKeyFocus( void ) ;
		// キーフォーカスが解除された
		virtual void OnKillKeyFocus( void ) ;

	protected:
		// ボタン押下処理
		virtual void OnButtonPushed( bool fRepeat ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool OnRButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;

	public:
		// キー入力
		virtual bool OnKeyDown
			( int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( int64_t nVirtKey, int64_t nFlags ) ;

	public:
		// Rosetta 用クラス
		virtual const wchar_t * GetRSClassName( void ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ボタンステータス反映ボタンリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteButtonStatusReflectionListener	: public SGLSpriteButtonListener
	{
	protected:
		SSystem::SSmartReference<SGLSprite>			m_refTargetSprite ;
		SSystem::SSmartReference<SGLImageObject>	m_refImage[SGLSpriteButton::statusCount] ;
		uint32_t									m_maskStatus ;
		SGLSkinManager::ImageDescription			m_imgdscImage[SGLSpriteButton::statusCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteButtonStatusReflectionListener, SGLSpriteButtonListener )
		// 構築関数
		SGLSpriteButtonStatusReflectionListener( void ) ;
		// 消滅関数
		virtual ~SGLSpriteButtonStatusReflectionListener( void ) ;

	public:
		// ステータス反映設定
		void AttachStatusReflection
			( SGLSprite * pTarget,
				uint32_t maskStatus, SGLImageObject** ppImages ) ;
		// <status_reflection> コマンド処理
		SGLError InvokeCommand
			( SGLSprite& sprite, const SSystem::SXMLDocument& xmlCmd ) ;

	public:
		// ボタンのステータスが変化した
		virtual bool OnChangedButtonStatus( SGLSpriteButton& button ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクロールボタンリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteScrollButtonListener	: public SGLSpriteButtonListener
	{
	protected:
		SSystem::SSmartReference<SGLSprite>	m_refTargetSprite ;
		int									m_offsetScroll ;
		SGLSprite::ScrollDirection			m_scrollDirection ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLSpriteScrollButtonListener, SGLSpriteButtonListener )
		// 構築関数
		SGLSpriteScrollButtonListener( void ) ;
		// 消滅関数
		virtual ~SGLSpriteScrollButtonListener( void ) ;

	public:
		// スクロールターゲット設定
		void AttachScrollTarget
			( SGLSprite * pTarget,
				int offsetScroll,
				SGLSprite::ScrollDirection scrlDir = SGLSprite::scrollDefault ) ;
		// <scroll> コマンド処理
		SGLError InvokeCommand
			( SGLSprite& sprite, const SSystem::SXMLDocument& xmlCmd ) ;

	public:
		// ボタンが押された
		virtual bool OnButtonPushed( SGLSpriteButton& button, bool fRepeat ) ;

	} ;

}

#endif
