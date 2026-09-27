
#if	!defined(__SAKURAGLX_SPRITE_MESSAGE_H__)
#define	__SAKURAGLX_SPRITE_MESSAGE_H__	1

#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メッセージ表示スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMessage	: public SGLSprite
	{
	public:
		// リンク情報
		class	LinkInfo
		{
		public:
			enum	Status
			{
				statusNormal,
				statusFocus,
				statusPushing,
				statusCount,
			} ;
			Status						m_status ;
			SSystem::SArray<SGLRect>	m_aHitRects ;
			SGLImageObject *			m_pImage[statusCount] ;
			SSystem::SString			m_strLinkURL ;
		public:
			// 構築関数
			LinkInfo( void ) ;
			LinkInfo( const LinkInfo& li ) ;
			// 消滅関数
			~LinkInfo( void ) ;
		} ;

		// 文字パーツ
		class	Character
		{
		public:
			LinkInfo *			m_pLink ;		// リンク情報
			SGLImageObject *	m_pImage ;		// 表示画像
			SGLPoint			m_ptWriting ;	// 描画座標
			SGLPoint			m_ptOffset ;	// 画像描画座標
			SGLSize				m_sizeChar ;	// 文字ピッチ
			uint32_t			m_wchCode ;		// 文字コード
			uint32_t			m_msecStart ;	// 表示開始時間
		public:
			// 構築関数
			Character( void ) ;
			Character( const Character& chr ) ;
			// 消滅関数
			~Character( void ) ;
		} ;

		// 表示アクションスタイル
		struct	ViewActionStyle
		{
		public:
			uint32_t	msecPerChar ;	// 表示速度 [ms/char]
			uint32_t	msecFade ;		// フェードイン時間 [ms]
			S2DVector	vMove ;			// 表示移動元オフセット
			S2DVector	vZoom ;			// 表示開始時拡大率
			float32_t	zRotation ;		// 表示開始時回転角 [deg]
			uint32_t	reserved ;
		public:
			// 構築関数
			ViewActionStyle( void ) ;
			ViewActionStyle( const ViewActionStyle& src ) ;
			// 代入
			const ViewActionStyle& operator = ( const ViewActionStyle& src ) ;
		} ;

		// スタイル
		struct	RichTextStyle	: public SGLSpriteText::TextStyle
		{
			SGLFontStyle			fontRuby ;
			SGLLetterer::Decoration	decoLink[LinkInfo::statusCount] ;

			// 構築関数（デフォルト値）
			RichTextStyle( void ) ;
			// 構築関数（複製）
			RichTextStyle( const RichTextStyle& style ) ;
			// 代入
			const RichTextStyle& operator = ( const RichTextStyle& style ) ;
			// シリアライズ（ポインタを除く）
			SGLError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
			SGLError LoadWithoutPointer( SSystem::SFileInterface& file ) ;
		} ;
		struct	MessageStyle	: public RichTextStyle
		{
			ViewActionStyle	viewAction ;

			// 構築関数（デフォルト値）
			MessageStyle( void ) ;
			// 構築関数（複製）
			MessageStyle( const MessageStyle& style ) ;
			// 代入
			const MessageStyle& operator = ( const MessageStyle& style ) ;
			// シリアライズ（ポインタを除く）
			SGLError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
			SGLError LoadWithoutPointer( SSystem::SFileInterface& file ) ;
		} ;

		// 文字画像アトラス
		class	Atlas
		{
		public:
			SGLImage	m_imgAtlas ;
			SGLSize		m_sizeAtlas ;
			size_t		m_xNext ;
			size_t		m_yLine ;
			size_t		m_hLine ;

		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			// 構築関数
			Atlas( void ) ;
			// アトラス画像サイズ設定
			void SetAtlasSize( uint32_t nWidth, uint32_t nHeight ) ;
			// 画像クリア
			void ClearAll( void ) ;
			// 文字画像追加
			SGLImageObject * Allocate( SGLImageObject& imgChar ) ;
		} ;

	protected:
		// 表示文字配列
		SSystem::SObjectArray<Character>	m_characters ;
		bool						m_flagRectChars ;
		SGLRect						m_rectChars ;
		uint32_t					m_msecDuration ;
		uint32_t					m_msecLastTiming ;
		uint32_t					m_msecFading ;
		uint32_t					m_fxSpeedRatio ;	// /256

		// スタイル
		SSystem::SString			m_strFontFace ;
		SSystem::SString			m_strRubyFont ;
		SSystem::SString			m_strProhibition ;
		MessageStyle				m_styleMsg ;
		SSystem::SSmartReference<SGLSkinManager>
									m_refSkin ;

		// 文字ラスタライザ
		SGLLetteringContext			m_lcLettering ;
		SGLLetterer::Decoration		m_ltDecoration ;
		SSystem::SString			m_strFontCur ;
		SGLFontStyle				m_fsFont ;
		SGLFont						m_font ;

		// 表示文字履歴
		SSystem::SXMLDocument		m_xmlHistory ;

		// フォーカスリンク
		LinkInfo *					m_pFocusLink ;

		// アトラス
		SSystem::SSmartPointer<Atlas>	m_pAtlas ;
		atomic_int_t					m_nPausedMsg ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMessage, SGLSprite )
		// 構築関数
		SGLSpriteMessage( void ) ;
		SGLSpriteMessage( const SGLSpriteMessage& src ) ;
		// 消滅関数
		virtual ~SGLSpriteMessage( void ) ;

	public:
		// テキストスタイル取得
		const SGLSpriteText::TextStyle& GetTextStyle( void ) const ;
		const RichTextStyle& GetRichTextStyle( void ) const ;
		const MessageStyle& GetMessageStyle( void ) const ;
		// テキストスタイル設定
		void SetTextStyle( const SGLSpriteText::TextStyle& style ) ;
		void SetRichTextStyle( const RichTextStyle& style ) ;
		void SetMessageStyle( const MessageStyle& style ) ;
 		// ルビ表示フォントスタイル取得
		const SGLFontStyle& GetRubyFontStyle( void ) const ;
		// ルビ表示フォントスタイル設定
		void SetRubyFontStyle( const SGLFontStyle& style ) ;
		// 表示スタイル取得
		const ViewActionStyle & GetViewActionStyle( void ) const ;
		// 表示スタイル設定
		void SetViewActionStyle( const ViewActionStyle& style ) ;
		// 関連スキン取得
		SGLSkinManager * GetAttachedSkin( void ) const ;
		// スキンを関連付け
		void AttachSkin( SGLSkinManager * pSkin ) ;
		// リンク装飾取得
		const SGLLetterer::Decoration& GetLinkDecoration( int nStatus ) const ;
		// リンク装飾設定
		void SetLinkDecoration
			( int nStatus, const SGLLetterer::Decoration& deco ) ;
		// 文字アトラス化設定
		void SetCharacterAtlas( uint32_t nWidth, uint32_t nHeight ) ;
		void ReleaseCharacterAtlas( void ) ;

	public:
		// 表示文字を全消去
		void ClearMessage( void ) ;
		// 表示中の文字を即座に完了させる
		void FlushMessage( void ) ;
		// 文字が表示中（フェード中）か？
		bool IsMessagePending( void ) const ;
		// 出力速度比 [x256] 設定 (0x10000 は一瞬表示)
		void SetMessageSpeedRatio( uint32_t fxSpeedRatio ) ;

	public:
		// スタイル解釈
		static void ParseRichTextStyle
			( RichTextStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strRubyFont,
				const SSystem::SXMLDocument& xmlStyle ) ;
		static void ParseMessageStyle
			( MessageStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strRubyFont,
				const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// 表示文字列追加
		void AddMessageText( const wchar_t * pwszText ) ;
		// 表示文字列追加
		void AddMessageXML( const SSystem::SXMLDocument& xmlMsg ) ;
		void AddMessageXML( const wchar_t * pwszTextXML ) ;
		// 画像追加
		void AddMessageImage
			( SGLImageObject * pImage,
				int pitchChar = 0, int xOffset = 0, int yOffset = 0 ) ;
		// 垂直アライメント適用
		void VerticalAlignmentMessage( void ) ;
		// 次の出力位置を取得
		const SGLPoint& GetNextMessagePoint( void ) const ;
		// 表示文字の外接矩形（ローカル座標）取得
		bool GetCircumscribedRect( SGLRect& rectMsg ) const ;
		// 表示文字数取得
		size_t GetMessageCharacterCount( void ) const ;
		// 表示文字取得
		Character * GetMessageCharacterAt( size_t iMsg ) const ;
		// 表示メッセージのプレーンテキスト取得
		SSystem::SString GetPlainText( void ) const ;

	public:
		// 書式コンテキスト
		class	LetteringContext
		{
		public:
			SGLLetteringContext		context ;
			SGLLetterer::Decoration	decoration ;
			SGLFontStyle			font ;
			SSystem::SString		strFontFace ;
		} ;
		void SaveLetteringContext( LetteringContext& context ) ;
		void RestoreLetteringContext( const LetteringContext& context ) ;
		// 文字色
		void SetTextColor( SGLPalette argbText ) ;
		void SetTextBorderColor( SGLPalette argbBorder ) ;
		void SetTextBorderWidth( uint32_t nWidth ) ;
		void SetTextBorder2Color( SGLPalette argbBorder2 ) ;
		void SetTextBorder2Width( uint32_t nWidth2 ) ;
		void SetTextShadowColor( SGLPalette argbBorder ) ;

	protected:
		// 表示文字追加
		SGLError AddLettering( const SSystem::SXMLDocument& xmlMsg ) ;
		// 表示文字追加（XML要素）
		SGLError AddLetteringElements( const SSystem::SXMLDocument& xmlMsg ) ;
		// 改行判定
		size_t CountLineFeedFrom( size_t iStart ) const ;
		// 文字のアライメント処理
		void AlignmentLetter
			( size_t iStart, SGLLetteringContext::AlignmentType typeAlign ) ;
		// 表示属性保存
		void SaveLetteringContext
			( SGLLetteringContext& context,
				SGLLetterer::Decoration& decoration,
				SGLFontStyle& font, SSystem::SString& strFontFace ) ;
		// 表示属性復帰
		void RestoreLetteringContext
			( const SGLLetteringContext& context,
				const SGLLetterer::Decoration& decoration,
				const SGLFontStyle& font ) ;

	protected:
		// リンクあたり判定
		LinkInfo * HitTestLinkInfo( int x, int y ) const ;
		// リンククリック時処理
		virtual void OnClickLinkInfo( const LinkInfo * pLink ) ;

	public:	// SGLSprite オーバーライド
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	protected:
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		virtual void DrawChildrenImageList
			( SGLDrawImageParamList& dipl,
					Stereo3DView s3dView = s3dMonoview ) const ;
	public:
		void DrawCharacterImageList
			( SGLDrawImageParamList& dipl,
					Stereo3DView s3dView = s3dMonoview ) const ;
		// 画像化
		SGLError RasterizeTextImage
			( SGLImageObject& imgText, SGLPoint& ptOffset ) ;

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;

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

	public:
		// Rosetta 用クラス
		virtual const wchar_t * GetRSClassName( void ) const ;
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:	// SGLObject オーバーライド
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// テキスト表示スプライト（テキスト装飾／省メモリスクロール対応）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteSmartTextView
			: public SGLSprite, public SGLSpriteScrollListener
	{
	public:
		// スタイル
		struct	TextStyle	: public SGLSpriteMessage::RichTextStyle
		{
			// 構築関数（デフォルト値）
			TextStyle( void ) { }
			// 構築関数（複製）
			TextStyle( const TextStyle& style )
				: SGLSpriteMessage::RichTextStyle( style ) { }
			// 代入
			const TextStyle& operator = ( const TextStyle& style )
			{
				SGLSpriteMessage::RichTextStyle::operator = ( style ) ;
				return	*this ;
			}
		} ;

	protected:
		// スタイル
		SSystem::SString	m_strFontFace ;
		SSystem::SString	m_strRubyFont ;
		SSystem::SString	m_strProhibition ;
		TextStyle			m_styleText ;
		SSystem::SSmartReference<SGLSkinManager>
									m_refSkin ;

		class	TextLine
		{
		public:
			SGLSpriteMessage *		m_pSprite ;
			SSystem::SXMLDocument	m_xmlLine ;
			int						m_yLineTop ;
			int						m_nLineHeight ;
		public:
			TextLine( void )
				: m_pSprite( NULL ),
					m_yLineTop( -1 ), m_nLineHeight( -1 ) {}
			~TextLine( void )
			{
				delete	m_pSprite ;
				m_pSprite = NULL ;
			}
		} ;
		SGLSize							m_sizeView ;
		SGLPoint						m_ptScroll ;
		SSystem::SString				m_strXMLText ;
		SSystem::SObjectArray<TextLine>	m_aLines ;

		// 関連スクロールバー
		SSystem::SSmartReference<SGLSprite>					m_refScrollBar ;
		SSystem::SSmartReference<SGLBasicForm::TrackBar>	m_refTrackBar ;

		// スクロール・スワイプ操作
		bool			m_flagSwiping ;
		S2DDVector		m_vLastSwipe ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLSpriteSmartTextView, SGLSprite, SGLSpriteScrollListener )
		// 構築関数
		SGLSpriteSmartTextView( void ) ;
		SGLSpriteSmartTextView( const SGLSpriteSmartTextView& src ) ;
		// 消滅関数
		virtual ~SGLSpriteSmartTextView( void ) ;

	public:
		// 作成
		SGLError CreateView
			( uint32_t width, uint32_t height, bool flagBuffered = true ) ;
		// 解放
		void ReleaseView( void ) ;
		// スクロール高計算（暫定）
		int EstimateScrollHeight( void ) const ;
		// 現在のスクロール位置をスクロールバーへ反映
		void ReflectToScrollBar( void ) ;

	public:
		// テキストスタイル取得
		const SGLSpriteSmartTextView::TextStyle& GetTextStyle( void ) const ;
		// テキストスタイル設定
		void SetTextStyle( const SGLSpriteSmartTextView::TextStyle& style ) ;
		// ルビ表示フォントスタイル取得
		const SGLFontStyle& GetRubyFontStyle( void ) const ;
		// ルビ表示フォントスタイル設定
		void SetRubyFontStyle( const SGLFontStyle& style ) ;
		// 関連スキン取得
		SGLSkinManager * GetAttachedSkin( void ) const ;
		// スキンを関連付け
		void AttachSkin( SGLSkinManager * pSkin ) ;
		// リンク装飾取得
		const SGLLetterer::Decoration& GetLinkDecoration( int nStatus ) const ;
		// リンク装飾設定
		void SetLinkDecoration
			( int nStatus, const SGLLetterer::Decoration& deco ) ;
		// 更新処理
		void UpdateView( void ) ;

	public:
		// スタイル解釈
		static void ParseRichTextStyle
			( TextStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strRubyFont,
				const SSystem::SXMLDocument& xmlStyle ) ;

	public:	// SGLSprite オーバーライド
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
	protected:
		void UpdateTextView( void ) ;
		void PrepareTextDraw( bool fAllSize = false ) ;
	public:
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;
		// スクロール・トラック位置属性
		virtual int GetScrollPos
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollPos
			( int nPos, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・トラック位置範囲属性
		virtual int GetScrollRange
			( ScrollDirection scrlDir = scrollDefault ) ;
		// スクロールバー関連付け
		virtual void AttachScrollBar
			( SGLSprite * pScrollBar, ScrollDirection scrlDir = scrollDefault ) ;
		void AttachTrackBar( SGLBasicForm::TrackBar * pScrollBar ) ;
		// スクロールバー関連付け
		virtual void DetachScrollBar
			( SGLSprite * pScrollBar, ScrollDirection scrlDir = scrollDefault ) ;
		void DetachTrackBar( SGLBasicForm::TrackBar * pScrollBar ) ;
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

	public:	// SGLSpriteScrollListener オーバーライド
		// 位置が移動した
		virtual bool OnScroll
			( SGLSpriteScrollBar& scroll, int64_t codeNotify ) ;

	public:	// SGLObject オーバーライド
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;


}

#endif

