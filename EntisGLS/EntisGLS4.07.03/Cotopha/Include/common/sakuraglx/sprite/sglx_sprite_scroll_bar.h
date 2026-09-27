
#if	!defined(__SAKURAGLX_SPRITE_SCROLL_BAR_H__)
#define	__SAKURAGLX_SPRITE_SCROLL_BAR_H__	1

#include <sakuraglx/sprite/sglx_sprite_button.h>


namespace	SakuraGL
{
	class	SGLSpriteScrollBar ;

	//////////////////////////////////////////////////////////////////////////
	// スプライトボタンリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteScrollListener	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteScrollListener, SObject )
		// 位置が移動した
		virtual bool OnScroll
			( SGLSpriteScrollBar& scroll, int64_t codeNotify ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクロールバースプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteScrollBar	: public SGLSprite
	{
	public:
		// スクロールバースタイル
		enum	BarType
		{
			typeVertBar,
			typeHorzBar,
		} ;
		enum	BarStatus
		{
			statusNormal,
			statusFocus,
			statusTracking,
			statusDisabled,
			statusCount,
		} ;
		struct	BarStyle
		{
			BarType								typeScroll ;
			bool								flagBarStretchable ;
			bool								flagProgressStretchable ;
			SGLRect								rectTrackMargin ;
			SGLImageRect						rectBarStretchable ;
			SGLImageRect						rectColStretchable ;
			SGLImageRect						rectProgressStretchable ;
			SGLSkinManager::ImageDescription	imgdscBar[statusCount] ;
			SGLSkinManager::ImageDescription	imgdscColumn[statusCount] ;
			SGLSkinManager::ImageDescription	imgdscProgress[statusCount] ;

			// 構築関数（デフォルト値）
			BarStyle( void ) ;
			// 構築関数（複製）
			BarStyle( const BarStyle& style ) ;
			// 代入
			const BarStyle& operator = ( const BarStyle& style ) ;
		} ;
		// スクロールバー通知メッセージ
		enum	NotificationCode
		{
			ncPosition,
			ncLineUp,
			ncLineDown,
			ncClickColumn,
			ncTracking,
			ncEndTracking
		} ;

	protected:
		SSystem::SSmartReference<SGLSpriteScrollListener>	m_refScrollListener ;

		SSystem::SSmartReference<SGLImageObject>	m_refBarImage[statusCount] ;
		SSystem::SSmartReference<SGLImageObject>	m_refColumnImage[statusCount] ;
		SSystem::SSmartReference<SGLImageObject>	m_refProgressImage[statusCount] ;

		SSystem::SSmartPointer<SGLImageObject>	m_pColumnImage[statusCount] ;
		SSystem::SSmartPointer<SGLImageObject>	m_pProgressImage[statusCount] ;

		BarStatus			m_statusBar ;		// 状態
		BarStatus			m_statusView ;		// 表示状態
	
		BarStyle			m_styleBar ;		// スタイル
		uint32_t			m_widthScrollBar ;	// 全体の長さ（スクロール方向）
		//
		SGLRect				m_rectTrack ;		// スクロールトラック矩形
		SGLSize				m_sizeBarKnob ;		// つまみサイズ

		int					m_posScroll ;		// スクロール位置
		int					m_rangeScroll ;		// スクロール範囲
		int					m_pageScroll ;		// スクロール・ページサイズ

		bool				m_flagKeyActive ;	// キー操作中＆マウス操作なし
		bool				m_flagTracking ;	// トラック中か？
		S2DDVector			m_vTrackOffset ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteScrollBar, SGLSprite )
		// 構築関数
		SGLSpriteScrollBar( void ) ;
		SGLSpriteScrollBar( const SGLSpriteScrollBar& src ) ;
		// 消滅関数
		virtual ~SGLSpriteScrollBar( void ) ;

	public:
		// ボタンリスナを設定
		void AttachScrollListener( SGLSpriteScrollListener * pListener ) ;
		void SetSmartScrollListener( SGLSpriteScrollListener * pListener ) ;
		// シンプルなスクロールバーを生成
		SGLError CreateSimpleScrollBar
			( BarType typeScroll,
				uint32_t widthScrollBar, SGLImageObject** ppBarKnob ) ;

	protected:
		// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;
		// 進捗バー表示サイズ
		SGLSize GetProgressViewSize( void ) const ;
		// つまみ表示位置
		SGLPoint GetBarKnobPosition( void ) const ;
		// つまみ表示サイズ
		SGLSize GetBarKnobViewSize( void ) const ;
		// つまみ位置からスクロール位置計算
		int GetScrollPosFromBarKnobPosition( double x, double y ) const ;

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
		// ドラッグ／スワイプ処理判定（子スプライトが処理するか？）
		virtual bool CanBeginDragOver( double x, double y ) const ;

	public:
		// スタイル取得
		const BarStyle& GetScrollBarStyle( void ) const
		{
			return	m_styleBar ;
		}
		// スタイル設定
		void SetScrollBarStyle( const BarStyle& style ) ;
		// サイズ設定
		void SetScrollBarSize( uint32_t widthScrollBar ) ;
		// バーステータス設定
		void SetScrollBarStatus( BarStatus status ) ;

	public:
		// キーフォーカス・禁止状態をステータスに加味する
		BarStatus EffectStatus( BarStatus status ) ;
		// 有効な表示用ステータスを取得する
		static BarStatus ValidStatusView
			( const SGLSkinManager::ImageDescription* pImages, BarStatus status ) ;
		// スクロールバー画像を更新する
		void UpdateScrollImage( void ) ;
		// スクロールバーの表示を更新する
		void UpdateScrollView( void ) ;
		// スクロール操作を通知する
		void NotifyScroll( NotificationCode ncode ) ;

	public:
		// スクロールバースタイルを解釈する
		static void ParseScrollBarStyle
			( SGLSkinManager& skin,
				BarStyle& style, const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// 入力禁止状態
		virtual void SetEnable( bool fEnable ) ;
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
		// コマンド処理
		virtual SGLError InvokeCommand
			( const SSystem::SXMLDocument& xmlCmd,
				SSystem::SXMLDocument * pxmlResult = NULL ) ;

	public:
		// マウスキャプチャーが解放された
		virtual void OnReleaseMouseCapture( void ) ;
		// フォーカスが設定された
		virtual void OnSetKeyFocus( void ) ;
		// キーフォーカスが解除された
		virtual void OnKillKeyFocus( void ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( int32_t zDelta, double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;

	public:
		// キー入力
		virtual bool OnKeyDown
			( int64_t nVirtKey, int64_t nFlags ) ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;

	} ;

}

#endif
