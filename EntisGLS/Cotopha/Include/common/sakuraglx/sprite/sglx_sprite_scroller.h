
#if	!defined(__SAKURAGLX_SPRITE_SCROLLER_H__)
#define	__SAKURAGLX_SPRITE_SCROLLER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ドラッグスクロール用リスナ用リスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMouseScrollerListener : public SGLSpriteMouseListener
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO
			( SGLSpriteMouseScrollerListener, SGLSpriteMouseListener )
		// 構築関数
		SGLSpriteMouseScrollerListener( void ) ;

	public:
		// スクロール処理
		virtual void OnScrolled( SGLSprite& sprite ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ドラッグスクロール用リスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMouseScroller
				: public SGLSpriteMouseListener, public SGLSpriteTimer
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( SGLSpriteMouseScroller, SGLSpriteMouseListener, SGLSpriteTimer )
		// 構築関数
		SGLSpriteMouseScroller( void ) ;

	public:
		enum	ScrollFlag
		{
			scrollHorizontal	= 0x01,
			scrollVertical		= 0x02,
		} ;

	protected:
		uint32_t	m_nFlags ;
		double		m_xMin, m_yMin ;	// スクロール範囲
		double		m_xMax, m_yMax ;
		S2DDVector	m_vLastMove ;		// 直前マウス座標
		S2DDVector	m_vTotalMoved ;		// 移動量合計
		int64_t		m_msLastMove ;
		S2DDVector	m_vSpeed ;			// 速度
		double		m_fpDamper ;		// 減衰 [/sec]
		S2DDVector	m_vWheelSpeed ;		// ホイール速度 [pixel/sec]

		SSystem::SSmartReference
			<SGLSpriteMouseScrollerListener>
					m_refListener ;		// リスナ

		SSystem::SSmartReference<SGLSprite>
					m_refScrollTarget ;	// スクロールターゲット

		SSystem::SSmartReference<SGLSprite>
					m_refHorzScroll ;	// スクロールバー
		SSystem::SSmartReference<SGLSprite>
					m_refVertScroll ;
		S3DDVector	m_vRateScroll ;		// 座標からスクロール値への比率

		bool		m_flagAboveMouse ;
		bool		m_flagNoMove ;
		double		m_threasholdNoMove ;

		bool		m_flagTracking ;

	public:
		// スクロール用リスナ設定
		void AttachScrollerTo( SGLSprite& sprite, bool flagAbove = false ) ;
		// スクロール用リスナ解除
		void DetachScrollerFrom( SGLSprite& sprite ) ;
		// スクロール対象設定
		//（AttachScrollerTo で設定したものと別のものを設定する場合）
		void AttachScrollTarget( SGLSprite * pTarget ) ;
		// リスナ設定
		void AttachListener( SGLSpriteMouseScrollerListener * pListener ) ;
		// リスナ解除
		void DetachListener( SGLSpriteMouseScrollerListener * pListener ) ;
		// スクロール座標範囲設定
		void SetScrollRange
			( uint32_t nFlags,
				double xMin, double yMin, double xMax, double yMax ) ;
		void SetScrollViewPort
			( const SGLSize& sizeTotal,
				const SGLImageRect& rectViewPort,
				SGLSprite * pHorzScrollBar = NULL,
				SGLSprite * pVertScrollBar = NULL ) ;
		// スクロール速度減衰設定
		void SetScrollDamper( double fpDamper ) ;
		// ホイール速度設定
		void SetWheelSpeed( double xSpeed, double ySpeed ) ;
		// 前置リスナでのクリック判定閾値設定
		void SetThresholdNoMove( double thresholdNoMove ) ;
		// 現在のスクロール速度リセット
		void ResetCurrentScrollSpeed( void ) ;
		// 水平スクロールバー関連付け
		void AttachHorzScrollBar
			( SGLSprite * pScrollBar, double rateScroll = 1 ) ;
		// 垂直スクロールバー関連付け
		void AttachVertScrollBar
			( SGLSprite * pScrollBar, double rateScroll = 1 ) ;
		// スクロール位置反映
		void ReflectScrollPosOf( SGLSprite& sprite ) ;
		// スクロール位置設定
		void SetScrollPositionTo( SGLSprite& sprite, double xPos, double yPos ) ;

	public: // SGLSpriteTimer オーバーライド
		// タイマー処理
		virtual bool OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;

	public:	// SGLSpriteMouseListener オーバーライド
		// マウス移動
		virtual bool OnMouseMove
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( SGLSprite& sprite, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( SGLSprite& sprite, int32_t zDelta,
				double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;

	} ;


}

#endif

