
#if	!defined(__SAKURAGLX_SPRITE_FRAME_H__)
#define	__SAKURAGLX_SPRITE_FRAME_H__	1

#include <sakuraglx/sprite/sglx_sprite_scroller.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// フレーム表示スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFrame	: public SGLSprite
	{
	public:
		// テキストスタイル
		enum	FrameType
		{
			frameUpperLeft,
			frameUpper,
			frameUpperRight,
			frameLeft,
			framePane,
			frameRight,
			frameUnderLeft,
			frameUnder,
			frameUnderRight,
			frameCount,
		} ;
		struct	FrameStyle
		{
			SGLSkinManager::ImageDescription	imgdscParts[frameCount] ;

			// 構築関数（デフォルト値）
			FrameStyle( void ) ;
			// 構築関数（複製）
			FrameStyle( const FrameStyle& style ) ;
			// 代入
			const FrameStyle& operator = ( const FrameStyle& style ) ;
			// フレームサイズ（太さ）
			void GetFrameThickness( SGLRect& rect ) const ;
		} ;

	protected:
		SSystem::SSmartPointer<SGLImageObject>		m_pFrameImage ;
		SSystem::SSmartReference<SGLImageObject>	m_refFrame[frameCount] ;

		FrameStyle		m_styleFrame ;
		SGLSize			m_sizeFrame ;
		SGLImageRect	m_rectInner ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFrame, SGLSprite )
		// 構築関数
		SGLSpriteFrame( void ) ;
		SGLSpriteFrame( const SGLSpriteFrame& src ) ;
		// 消滅関数
		virtual ~SGLSpriteFrame( void ) ;

	public:
		// フレームスタイル取得
		const FrameStyle& GetFrameStyle( void ) const
		{
			return	m_styleFrame ;
		}
		// フレームスタイル設定
		void SetFrameStyle( const FrameStyle& style ) ;
		// フレーム矩形取得
		const SGLSize& GetFrameSize( void ) const
		{
			return	m_sizeFrame ;
		}
		// フレーム矩形設定
		void SetFrameSize( const SGLSize& size ) ;
		// 内側矩形取得
		const SGLImageRect& GetInnerRect( void ) const
		{
			return	m_rectInner ;
		}

	protected:
		// フレーム画像を更新する
		void UpdateFrameImage( void ) ;

	public:
		// 画像化したフレームを生成する
		static SGLImageObject * CreateFrameImage
			( const FrameStyle& style,
				const SGLSize& size, SGLImageRect& rectInner ) ;
		// フレームスタイルを解釈する
		static void ParseFrameStyle
			( SGLSkinManager& skin,
				FrameStyle& style, const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 可変クライアントビュー・スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteLayoutView	: public SGLSprite
	{
	protected:
		SSystem::SSmartReference
				<SGLSpriteLayoutView>	m_refClient ;
		SGLSize							m_sizeLayout ;
		SGLSize							m_sizeView ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteLayoutView, SGLSprite )
		// 構築関数
		SGLSpriteLayoutView( void ) ;
		SGLSpriteLayoutView( const SGLSpriteLayoutView& src ) ;
		// 消滅関数
		virtual ~SGLSpriteLayoutView( void ) ;
		// クライアント設定
		void AttachClientView( SGLSpriteLayoutView * pView ) ;
		void AttachSmartClientView( SGLSpriteLayoutView * pView ) ;
		// クライアント取得
		SGLSpriteLayoutView * GetClientView( void ) const ;
		// クライアント分離
		SGLSpriteLayoutView * DetachClientView( void ) ;
		// ビューのサイズ取得
		const SGLSize& GetLayoutViewSize( void ) const
		{
			return	m_sizeLayout ;
		}

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;

	public:
		// レイアウトサイズ取得
		virtual bool GetViewLayoutSize( SGLSize& sizeLayout ) const ;
		// サイズ変更通知
		virtual void OnChangeSize( const SGLSize& sizeView ) ;
		// クライアントサイズ計算
		virtual void CalculateClientRect( SGLImageRect& rectClient ) const ;
		// クライアントが設定された
		virtual void OnAttachedClientView( SGLSpriteLayoutView * pView ) ;
		// クライアントが分離された
		virtual void OnDettachedClientView( SGLSpriteLayoutView * pView ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクロールビュー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteScrollView	: public SGLSpriteLayoutView
	{
	protected:
		SGLSpriteMouseScroller	m_scroller ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteScrollView, SGLSpriteLayoutView )
		// 構築関数
		SGLSpriteScrollView( void ) ;
		// 消滅関数
		virtual ~SGLSpriteScrollView( void ) ;

	public:
		// スクロール領域更新
		void UpdateScrollRange( void ) ;
		// スクローラー取得
		SGLSpriteMouseScroller& Scroller( void )
		{
			return	m_scroller ;
		}

	public:
		// サイズ変更通知
		virtual void OnChangeSize( const SGLSize& sizeView ) ;
		// クライアントが設定された
		virtual void OnAttachedClientView( SGLSpriteLayoutView * pView ) ;
		// クライアントが分離された
		virtual void OnDettachedClientView( SGLSpriteLayoutView * pView ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// リストビュー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteListView	: public SGLSpriteLayoutView
	{
	protected:
		int							m_yListStep ;
		SSystem::SObjectArray
			<SGLSpriteLayoutView>	m_lstItems ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteListView, SGLSpriteLayoutView )
		// 構築関数
		SGLSpriteListView( void ) ;
		// 消滅関数
		virtual ~SGLSpriteListView( void ) ;

	public:
		// リスト間隔設定
		void SetListLineHeight( int yStep ) ;
		// リスト間隔取得
		int GetListLineHeight( void ) const ;
		// アイテム数取得
		size_t GetListItemCount( void ) const ;
		// アイテム取得
		SGLSpriteLayoutView * GetListItemAt( size_t nIndex ) const ;
		// アイテム追加
		size_t AddListItem( SGLSpriteLayoutView * pItem ) ;
		size_t InsertListItem( size_t nIndex, SGLSpriteLayoutView * pItem ) ;
		// アイテム削除
		void RemoveListItem( size_t nIndex ) ;
		SGLSpriteLayoutView * DetachListItem( size_t nIndex ) ;
		// アイテム全削除
		void RemoveAllListItems( void ) ;
		// アイテム検索
		ssize_t FindListItem( SGLSpriteLayoutView * pItem ) const ;

	protected:
		// 子ビュー（リストアイテム）座標更新
		void UpdateChildrenPosition( void ) ;
		// 子ビューサイズ更新
		void UpdateChildrenSize( void ) ;

	public:
		// レイアウトサイズ取得
		virtual bool GetViewLayoutSize( SGLSize& sizeLayout ) const ;
		// サイズ変更通知
		virtual void OnChangeSize( const SGLSize& sizeView ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// フレームビュー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFrameView	: public SGLSpriteLayoutView
	{
	protected:
		SGLSpriteFrame	m_frame ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteFrameView, SGLSpriteLayoutView )
		// 構築関数
		SGLSpriteFrameView( void ) ;
		// 消滅関数
		virtual ~SGLSpriteFrameView( void ) ;

	public:
		// フレーム作成
		SGLError CreateFrame
			( const SGLSpriteFrame::FrameStyle& style, const SGLSize& size ) ;
		// フレーム矩形設定
		void SetFrameSize( const SGLSize& size ) ;

	public:
		// サイズ変更通知
		virtual void OnChangeSize( const SGLSize& sizeView ) ;
		// クライアントサイズ計算
		virtual void CalculateClientRect( SGLImageRect& rectClient ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易ボタンビュー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteTouchableView	: public SGLSpriteLayoutView
	{
	public:
		enum	TouchStatus
		{
			statusNormal,
			statusFocus,
			statusPushed,
			statusSelected,
			statusCount,
		} ;

	protected:
		SGLPalette			m_rgbaBackColor[statusCount] ;
		uint32_t			m_transDisabled ;
		bool				m_flagSelected ;
		bool				m_flagSelectable ;
		TouchStatus			m_status ;
		SGLAudioPlayer *	m_pAudioFocus ;
		SGLAudioPlayer *	m_pAudioPushed ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteTouchableView, SGLSpriteLayoutView )
		// 構築関数
		SGLSpriteTouchableView( void ) ;
		// 背景色設定
		void SetNormalViewColor( uint32_t rgbaNormal ) ;
		void SetFocusViewColor( uint32_t rgbaFocus ) ;
		void SetPushedViewColor( uint32_t rgbaPushed ) ;
		void SetSelectedViewColor( uint32_t rgbaSelected ) ;
		// 効果音設定
		void SetFocusSE( SGLAudioPlayer * pAudio ) ;
		void SetPushedSE( SGLAudioPlayer * pAudio ) ;
		// 禁止状態透明度
		void SetDisabledTransparency( uint32_t nTransparency ) ;
		// ボタン属性
		virtual bool IsButtonChecked( void ) ;
		virtual void CheckButton( bool fCheck ) ;
		// 選択可能状態
		void SetSelectable( bool fSelectable ) ;
		bool IsSelectable( void ) const ;

	public:
		// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		// マウス移動
		virtual bool OnMouseMove
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;

	public:
		// 押下時処理
		virtual void OnButtonPushed( void ) ;

	} ;

}

#endif
