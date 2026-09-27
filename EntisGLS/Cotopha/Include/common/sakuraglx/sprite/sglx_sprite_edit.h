
#if	!defined(__SAKURAGLX_SPRITE_EDIT_H__)
#define	__SAKURAGLX_SPRITE_EDIT_H__	1

#include <sakura/ssys_queue_buffer.h>
#include <sakuraglx/sprite/sglx_sprite_text.h>

namespace	SakuraGL
{
	class	SGLSpriteEdit ;

	//////////////////////////////////////////////////////////////////////////
	// エディットテキストリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteEditListener	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteEditListener, SObject )
		// ユーザー入力によってテキストが変更された
		virtual void OnChangedText( SGLSpriteEdit& edit ) ;
		// 選択範囲・カーソルが移動した
		virtual void OnMovedCursor( SGLSpriteEdit& edit ) ;
		// スクロールした
		virtual void OnScrolled( SGLSpriteEdit& edit ) ;
		// テキスト入力フィルタ
		virtual void FilterInputText
			( SGLSpriteEdit& edit, SSystem::SString& strText ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// エディット・テキスト・スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteEdit	: public SGLSprite
	{
	public:
		// エディットスタイル
		enum	EditStyleFlag
		{
			editSingleLine		= 0x00000000,		// １行
			editMultiLine		= 0x00000001,		// 複数行
			editMultiLineMask	= 0x00000001,
			editLineWrap		= 0x00000002,		// 行を右端で折り返す
			editAcceptReturn	= 0x00000010,		// 改行コードの入力許可 
			editAcceptTab		= 0x00000020,		// タブコードの入力許可
			editAutoIndent		= 0x00000040,		// 自動インデント
			editMultiLineTab	= 0x00000080,		// 複数行選択時のタブは行頭に挿入
			editReadOnly		= 0x00000100,		// 読み取り専用
			editDenyAlphabet	= 0x00001000,		// アルファベット入力禁止
			editDenyNumber		= 0x00002000,		// 数字入力禁止
			editDeny8bitChar	= 0x00004000,		// 半角文字入力禁止
			editDenyMBChar		= 0x00008000,		// 全角文字入力禁止
			editNumber			= 0x00010000,		// 数値入力
			editPassword		= 0x00020000,		// パスワード入力
			editColumnCaret		= 0x00000000,		// 垂直カレット
			editUnderbarCaret	= 0x00100000,		// 水平カレット
			editCaretMask		= 0x00100000,
			editFontForIME		= 0x10000000,		// IME 入力表示用フォント指定
		} ;
		struct	EditStyle	: public SGLSpriteText::TextStyle
		{
			uint32_t				nEditFlags ;
			uint32_t				nCaretWidth ;
			uint32_t				nCaretBlinkInterval ;
			SGLPalette				rgbaCaretColor ;
			SGLPalette				rgbaSelBackColor ;
			SGLLetterer::Decoration	decoSel ;
			SGLFontStyle			fontIME ;

			// 構築関数（デフォルト値）
			EditStyle( void ) ;
			// 構築関数（複製）
			EditStyle( const EditStyle& style ) ;
			// 代入
			const EditStyle& operator = ( const EditStyle& style ) ;
		} ;
		// ブックマーク
		class	Bookmark	: public ESLObject
		{
		public:
			size_t	m_iPos ;	// ブックマーク位置（文字指標）

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Bookmark, ESLObject )
			// 構築関数
			Bookmark( size_t iPos ) : m_iPos( iPos ) { }
			Bookmark( const Bookmark& bm ) : m_iPos( bm.m_iPos ) { }

		} ;

	protected:
		// リスナ
		SSystem::SSmartReference<SGLSpriteEditListener>
										m_refEditListener ;

		// スタイル
		SSystem::SString	m_strFontFace ;
		SSystem::SString	m_strIMEFontFace ;
		EditStyle			m_styleEdit ;

		// 項目名
		SSystem::SString	m_strItemName ;

		// 文字列
		SSystem::SString	m_strEdit ;

		// 行頭インデックス
		SSystem::SArray<uint32_t>	m_indexLine ;

		// ブックマーク
		SSystem::SObjectArray<Bookmark>	m_arrBookmark ;

		// 表示用グラフィック
		class	LineView
		{
		public:
			SGLImage	m_imgView ;
			SGLPalette	m_rgbaBackColor ;
			SGLPoint	m_ptView ;
			SGLPoint	m_ptOffset ;
			SGLSize		m_sizeView ;
			bool		m_fUpdate ;
			size_t		m_iFirstChar ;
			size_t		m_iSelFirst ;
			size_t		m_iSelEnd ;
			SSystem::SArray<SGLImageRect>
						m_arrCharRects ;
			SSystem::SArray<SGLPalette>
						m_arrCharBack ;
		public:
			// 構築関数
			LineView( void ) ;
			LineView( const LineView& lv ) ;
			// 行幅（ピクセル）取得
			size_t GetLineWidth( void ) const ;
			// 指定文字の m_imgView 内表示位置取得
			bool CharRectInView( SGLImageRect& rect, size_t i ) const ;
		} ;
		SSystem::SObjectArray<LineView>	m_arrLineViewBuf ;

		// 表示位置
		int		m_yViewLine ;
		int		m_iViewLineOffset ;
		int		m_xScroll ;
		int		m_yScrollOffset ;

		// 選択範囲
		size_t	m_iSelFirst, m_iSelEnd ;
		size_t	m_iCursor ;

		// カーソルｘ位置
		int		m_xCursor ;

	public:
		// アンドゥ・リドゥ抽象オブジェクト
		class	UndoObject	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UndoObject, ESLObject )
			// アンドゥ実行
			virtual UndoObject * Undo( SGLSpriteEdit& edit ) = 0 ;
			// リドゥ実行
			virtual UndoObject * Redo( SGLSpriteEdit& edit ) ;
		} ;
		// アンドゥ・リドゥ用記録
		class	UndoRecord	: public UndoObject
		{
		public:
			size_t				m_iFirst, m_iEnd ;
			SSystem::SString	m_strText ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UndoRecord, UndoObject )
			// 構築関数
			UndoRecord( void ) ;
			UndoRecord( const UndoRecord& undo ) ;
			UndoRecord( int iFirst, int iEnd, const wchar_t * pwszText ) ;
			// アンドゥ実行
			virtual UndoObject * Undo( SGLSpriteEdit& edit ) ;
		} ;

	protected:
		SSystem::SObjectArray<UndoObject>	m_arrUndo ;
		SSystem::SObjectArray<UndoObject>	m_arrRedo ;
		size_t								m_limitUndo ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteEdit, SGLSprite )
		// 構築関数
		SGLSpriteEdit( void ) ;
		SGLSpriteEdit( const SGLSpriteEdit& src ) ;
		// 消滅関数
		virtual ~SGLSpriteEdit( void ) ;

	public:
		// テキストスタイル取得
		const EditStyle& GetEditStyle( void ) const
		{
			return	m_styleEdit ;
		}
		// テキストスタイル設定
		void SetEditStyle( const EditStyle& style ) ;
		// 項目名取得
		const SSystem::SString& GetEditItemName( void ) const
		{
			return	m_strItemName ;
		}
		// 項目名設定
		void SetEditItemName( const wchar_t * pwszName ) ;
		// リスナ設定
		void AttachEditListener( SGLSpriteEditListener * pListener ) ;
		// リスナ取得
		SGLSpriteEditListener * GetEditListener( void ) const ;

	public:
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;

	public:
		// エディットスタイルを解釈する
		static void ParseTextStyle
			( EditStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strIMEFontFace,
				const SSystem::SXMLDocument& xmlStyle ) ;
		static void ParseTextStyle_CompatibleGLS3
			( EditStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strIMEFontFace,
				const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// カーソル位置取得
		virtual size_t GetCursorIndex( void ) const ;
		// 選択範囲取得
		virtual void GetSel( size_t & iSelFirst, size_t & iSelEnd ) const ;
		// 選択範囲設定
		virtual void SetSel( ssize_t iFirst, ssize_t iEnd ) ;
		// 選択範囲の文字列を取得
		virtual SSystem::SString GetSelText( void ) const ;
		// 指定範囲の文字列を取得
		virtual SSystem::SString GetRangeText( ssize_t iFirst, ssize_t iEnd ) const ;
		// 文字コピー可能か？
		virtual bool CanCopyText( void ) const ;
		// 文字切り取り可能か？
		virtual bool CanCutText( void ) const ;
		// 文字列貼り付け可能か？
		virtual bool CanPasteText( void ) const ;
		// 選択文字列削除
		virtual void DoClear( void ) ;
		virtual void ClearSelText( UndoRecord * pUndo = NULL ) ;
		// 選択文字列切り取り
		virtual void DoCut( void ) ;
		virtual void CutSelText( UndoRecord * pUndo = NULL ) ;
		// 選択文字列コピー
		virtual void DoCopy( void ) ;
		virtual void CopySelText( void ) ;
		// 選択文字列貼りつけ
		virtual void DoPaste( void ) ;
		virtual void PasteSelText( UndoRecord * pUndo = NULL ) ;
		// 選択文字列置き換え
		virtual void DoReplace( const wchar_t * pwszText ) ;
		virtual void ReplaceSelText
			( const wchar_t * pwszText, UndoRecord * pUndo = NULL ) ;
		// UNDO 可能か？
		virtual bool CanUndo( void ) ;
		// UNDO 実行
		virtual void Undo( void ) ;
		// REDO 可能か？
		virtual bool CanRedo( void ) ;
		// REDO 実行
		virtual void Redo( void ) ;
		// 文字検索
		enum	FindTextFlag
		{
			findUp			= 0x0001,
			findNoCase		= 0x0002,
			findWholeWord	= 0x0004,
		} ;
		virtual bool DoFindText
			( const wchar_t * pwszText, uint32_t nFlags = 0 ) ;
		virtual bool DoFindTextFrom
			( size_t iStartChar,
				const wchar_t * pwszText, uint32_t nFlags = 0 ) ;

	public:
		// 文字指標から座標を計算
		SGLPoint GetCharPosFromIndex( size_t iChar ) const ;
		// 文字指標から行番号(0～)を取得
		size_t GetLineFromIndex( size_t iChar ) const ;
		// 行(0～)の先頭の文字指標を取得
		size_t GetLineIndex( size_t nLine ) const ;
		// 行(0～)の文字数を取得
		size_t GetLineLength( size_t nLine ) const ;
		// 行数を取得
		size_t GetLineCount( void ) const ;
		// 全文字数取得
		size_t GetLength( void ) const ;
		// 指定行(0～)の文字列を取得
		SSystem::SString GetLineText( size_t nLine, size_t iOffset = 0 ) const ;
		// スクロール位置取得（ピクセル, 行）
		SGLPoint GetScrollPos( size_t* pLineOffset = NULL ) const ;
		// スクロール位置設定
		void SetScrollPos
			( int xPos, int yLine, int iLineOffset = 0, int yOffset = 0 ) ;
		// 行の最大幅（ピクセル）を取得
		size_t GetMaxLineWidth( void ) const ;
		// 行間取得
		size_t GetLineHeight( void ) const ;

	protected:
		// 文字表示幅計算（元の文字幅からスタイルで修正された表示幅を計算）
		size_t GetCharWidthOf( size_t nCharWidth ) const ;
		// テキスト指標を更新する
		void UpdateTextIndex( void ) ;

	public:
		// 表示画像を更新する
		void UpdateTextImage( void ) ;
	protected:
		// 文字装飾のカスタマイズを準備する
		virtual void PrepareToCustomTextLineDecoration
			( const SSystem::SString& strLine, size_t iLineChar ) ;
		// 行全体の背景色を決定する
		virtual SGLPalette CustomTextLineBackColor
			( const LineView * plvLine, size_t iLineChar, size_t nLineNum ) ;
		// 文字装飾をカスタマイズする
		virtual void CustomTextLineDecoration
			( SGLLetterer::Decoration& deco,
				SGLPalette& rgbaBack, size_t& iNextDeco,
				const SSystem::SString& strLine, size_t iLineChar ) ;
		// 表示用画像バッファサイズを調整する
		virtual void AdjustmentTextLineSize
			( SGLSize& sizeImage, SGLPoint& ptOffset,
				const LineView * plvLine, const wchar_t * pwszLine ) ;
		// 表示画像に後処理する
		virtual void AfterUpdateTextLineImage
			( LineView * plvLine, const wchar_t * pwszLine ) ;

	protected:
		// 更新通知
		void SetUpdateRange
			( size_t iFirst, size_t iEnd,
				ssize_t iOffset = 0, bool fUpdateText = true ) ;
		// Undo を記録
		void RecordUndo( UndoRecord * pUndo ) ;

	public:
		// ユーザー定義 Undo を記録
		void RecordUndoObject( UndoObject * pUndo ) ;
		// 直前の Undo を取得
		UndoObject * GetLastUndoObject( void ) const ;

	public:
		// ブックマーク追加
		void AddBookmark( Bookmark * pBookmark ) ;
		// ブックマーク数取得
		size_t GetBookmarkCount( void ) const ;
		// ブックマーク取得
		Bookmark * GetBookmarkAt( size_t iBookmark ) const ;
		// 指定行(0～)に含まれるブックマーク取得
		Bookmark * GetBookmarkLineAt( size_t nLine ) const ;
		// ブックマーク指標検索
		ssize_t FindBookmark( Bookmark * pBookmark ) const ;
		ssize_t FindBookmarkOf( size_t iFirstChar, size_t iEndChar ) const ;
		// ブックマーク削除
		void RemoveBookmarkAt( size_t iBookmark ) ;
		// 全ブックマーク削除
		void RemoveAllBookmarks( void ) ;

	protected:
		// ブックマーク・バイナリ検索
		size_t OrderBookmarkIndex( size_t iCharPos ) const ;

	public:
		// 指定行の更新通知
		void PostUpdateLine( int nLine ) ;
		// すべての表示行の更新通知
		void PostUpdateAllLines( void ) ;
		// 指定文字指標を表示領域に収まるようにスクロールする
		void TrackCharacterFor( size_t iChar ) ;
		// 指定文字指標の座標を取得する（表示されている文字のみ）
		bool GetCharacterPosOfView( SGLImageRect& rectChar, size_t iChar ) const ;
		// 指定行の表示行数を取得する（表示されている範囲のみ）
		size_t GetLineCountOfView( size_t nLine ) const ;
		// 座標から文字指標へ変換
		ssize_t GetCharIndexFromPosOfView( int xPos, int yPos ) const ;
		// カレット表示矩形を取得
		bool GetCaretRect( SGLImageRect& rectCaret ) const ;
		// 垂直相対スクロール
		void ScrollDeltaVertical( int yDelta ) ;

		// 文字種類
		enum	WordType
		{
			wordControl,
			wordAlphabet,
			wordMark,
			wordSeparator,
			wordWideSpace,
			wordWideAlphabet,
			wordHiraKana,
			wordWideKana,
			wordWidePunctuation,
			wordOtherCharacter,
		} ;
		virtual int WordKindOf( wchar_t wch ) const ;
		// 禁則文字判定
		virtual bool IsProhibitChar( wchar_t wch ) const ;
		// 単語範囲判定
		void FindWordRange
			( size_t& iFirst, size_t& iEnd, size_t iChar ) const ;

	protected:
		// 入力テキストフィルタ
		virtual void FilterInputText( SSystem::SString& strText ) ;

	protected:
		// カレット表示用
		uint32_t	m_msecCaret ;		// 点滅用時間カウンタ

		// マウス入力
		bool		m_fMouseLDown ;		// 左ボタン押下状態
		bool		m_fMouseLDblClk ;
		bool		m_fMouseLDownMoved ;
		size_t		m_iDownFirstChar ;
		size_t		m_iDblClkEndChar ;

		// テキスト編集メッセージボックスを使用
		class	CallEditMessageBox
		{
		public:
			SSystem::SSmartReference<SGLSpriteEdit>	m_refEdit ;
			SSystem::SString	m_strEdit ;
			SSystem::SString	m_strCaption ;
			int					m_nStyles ;
		} ;
		static void CallEditMessageBoxProc( void * pInstance ) ;

		// コンテキストメニュー
		SSystem::SSmartPointer<SGLWindowMenu>
								m_pMenu ;

	protected:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
		// コマンド処理
		virtual SGLError InvokeCommand
			( const SSystem::SXMLDocument& xmlCmd,
				SSystem::SXMLDocument * pxmlResult = NULL ) ;

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
		// コマンド発行処理（フォーカスアイテムへ）
		virtual bool DispatchCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0 ) ;

	public:
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// エディット・カラム（行番号表示・エディット領域サイズ調整）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteEditColumn
				: public SGLSprite, public SGLSpriteEditListener
	{
	public:
		// スタイル
		struct	ColumnStyle
		{
			SGLFontStyle	font ;
			uint32_t		minLineNumWidth ;	// 行番号最小幅
			int32_t			leftMargin ;		// 行番号左側マージン
			int32_t			rightMargin ;		// 行番号右側マージン

			// 構築関数（デフォルト値）
			ColumnStyle( void ) ;
			// 構築関数（複製）
			ColumnStyle( const ColumnStyle& style ) ;
			// 代入
			const ColumnStyle& operator = ( const ColumnStyle& style ) ;
		} ;
		// リスナ
		class	Listener	: public SSystem::SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Listener, SObject )
			// 行番号表示色（行番号0～）
			virtual SGLPalette OnDrawLineNumber
				( SGLSpriteEditColumn& colEdit, SGLSpriteEdit& edit,
					size_t iLine, SGLImageObject * pDstImage, int yLine ) = 0 ;
		} ;

	protected:
		// 対象エディットアイテム
		SGLSpriteEdit *			m_pEdit ;
		bool					m_flagOwnEdit ;

		// リスナ
		SSystem::SSmartReference<SGLSpriteEditListener>
								m_refEditListener ;
		SSystem::SSmartReference<Listener>
								m_refColListener ;

		// スタイル
		ColumnStyle				m_styleCol ;
		SSystem::SString		m_strFont ;
		SGLSize					m_sizeFrame ;

		class	NumberChar
		{
		public:
			SGLFontMetrics	m_metrics ;
			SGLImage		m_imgChar ;
		} ;
		int						m_nMaxNumWidth ;
		NumberChar				m_ncImage[10] ;
		SGLImage				m_imgNumBuffer ;

		bool					m_flagUpdateColumn ;
		size_t					m_nLastLineCount ;
		size_t					m_nLineNumWidth ;
		SGLImage				m_imgColumn ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( SGLSpriteEditColumn, SGLSprite, SGLSpriteEditListener )
		// 構築関数
		SGLSpriteEditColumn( void ) ;
		SGLSpriteEditColumn( const SGLSpriteEditColumn& src ) ;
		// 消滅関数
		virtual ~SGLSpriteEditColumn( void ) ;

	public:
		// エディットアイテム設定
		SGLError AttachEdit
			( SGLSpriteEdit * pEdit, bool fAutoDelete = false ) ;
		// エディットアイテム解除
		SGLError DetachEdit( void ) ;
		// リスナ設定
		void AttachEditListener( SGLSpriteEditListener * pListener ) ;
		void AttachColumnListener( Listener * pListener ) ;
		// サイズ変更
		void AdjustSize( int nWidth, int nHeight ) ;
		// 行番号表示設定
		void SetColumnStyle( const ColumnStyle& style ) ;
		// 更新
		void SetUpdateColumn( void ) ;

	protected:
		// レイアウト調整
		void AdjustColumnLayout( void ) ;
		// 行番号描画
		void RedrawColumn( void ) ;
		void DrawLineNum
			( SGLImageObject * pDst,
				int xPos, int yPos, int nLineNum,
				int nWidth, uint32_t argbColor = 0xFFFFFF ) ;

	protected:
		// 表示状態の子スプライトに対し BeforeDraw を呼び出し
		virtual void BeforeDrawChildren
			( Stereo3DView s3dView = s3dMonoview ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	public:	// SGLSpriteEditListener 実装
		// ユーザー入力によってテキストが変更された
		virtual void OnChangedText( SGLSpriteEdit& edit ) ;
		// 選択範囲・カーソルが移動した
		virtual void OnMovedCursor( SGLSpriteEdit& edit ) ;
		// スクロールした
		virtual void OnScrolled( SGLSpriteEdit& edit ) ;
		// テキスト入力フィルタ
		virtual void FilterInputText
			( SGLSpriteEdit& edit, SSystem::SString& strText ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 疑似コンソール・スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteConsole	: public SGLSprite
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSpriteConsole, SGLSprite )
		// 構築関数
		SGLSpriteConsole( void ) ;
		SGLSpriteConsole( const SGLSpriteConsole& src ) ;
		// 消滅関数
		virtual ~SGLSpriteConsole( void ) ;

	public:
		enum	ConsoleFlag
		{
			flagScrollDrag	= 0x0001,
			flagScrollWheel	= 0x0002,
		} ;

	protected:
		class	Character
		{
		public:
			SGLImage	m_imgChar ;
			SGLPoint	m_ptOffset ;
		} ;
		class	Line
		{
		public:
			bool					m_flagUpdate ;
			bool					m_flagLineFeed ;
			bool					m_flagReturn ;
			SGLImage				m_imgLine ;
			SGLPoint				m_ptOffset ;
			SSystem::SString		m_strLine ;
			int						m_nHeight ;
			size_t					m_nFixedChars ;
			SSystem::SArray
					<SGLImageRect>	m_aCharRects ;
		public:
			// 構築関数
			Line( void )
				: m_flagUpdate( false ),
					m_flagLineFeed( false ),
					m_flagReturn( false ),
					m_nHeight( 0 ), m_nFixedChars( 0 ) { }
			Line( const Line& line )
				: m_flagUpdate( true ),
					m_flagLineFeed( line.m_flagLineFeed ),
					m_flagReturn( line.m_flagReturn ),
					m_strLine( line.m_strLine ),
					m_nHeight( line.m_nHeight ),
					m_nFixedChars( line.m_nFixedChars ) { }
		} ;
		SSystem::SObjectArray<Line>	m_lines ;
		SGLFont				m_font ;
		SGLFontStyle		m_fontStyle ;
		SSystem::SString	m_strFontFace ;
		SGLPalette			m_rgbaFont ;
		uint32_t			m_nLineHeight ;
		size_t				m_nLimitLines ;

		uint32_t			m_flagsConsole ;
		uint32_t			m_yValidInput ;

		atomic_int_t		m_nInputable ;
		size_t				m_iCurLine ;
		size_t				m_iCurChar ;
		SGLPalette			m_rgbaCursor ;
		size_t				m_msecCurBlink ;
		size_t				m_msecCurBlinkInterval ;

		int32_t				m_yScroll ;

		bool				m_flagDragScroll ;
		bool				m_flagDragMoved ;
		S2DDVector			m_vBeginDragPos ;
		int32_t				m_yBeginDragScroll ;

		SSystem::SObjectArray
				<SSystem::SString>	m_lstHistory ;
		size_t						m_limitHistory ;
		ssize_t						m_iRefHistory ;

		SSystem::Charset::EncodingType	m_encoding ;
		SSystem::SQueueBuffer			m_qbufInput ;
		SSystem::SSignalEvent			m_eventInput ;

		// 入出力ファイルインターフェース
		class	FileInterface	: public SSystem::SFileInterface
		{
		protected:
			SSystem::SSmartReference<SGLSpriteConsole>	m_refConsole ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( FileInterface, SFileInterface )
			// 構築関数
			FileInterface( SGLSpriteConsole * pConsole ) ;

		public:
			// ファイルインターフェースの複製
			virtual SFileInterface * Duplicate( void ) const ;
			// ファイルから読み込み
			virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
			// ファイルへ書き込み
			virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
			// シーク可能か否か？
			virtual bool IsSeekable( void ) const ;
			// ファイル長の取得
			virtual int64_t GetLength( void ) const ;
			// ファイルポインタを移動
			virtual int64_t Seek
				( int64_t posFile, SeekOrigin seekFrom ) ;
			// ファイルポインタを取得
			virtual int64_t GetPosition( void ) const ;
			// ファイルの終端を現在の位置に設定する
			virtual SSystem::SError SetEndOfFile( void ) ;
		} ;

	public:
		// フォント設定
		void SetConsoleFont
			( const SGLFontStyle& style,
				uint32_t nLinePitch, SGLPalette rgbaColor ) ;
		// サイズ設定
		void SetConsoleSize
			( uint32_t nWidth, uint32_t nHeight, uint32_t yValidInput = 0 ) ;
		// 文字エンコーディング設定
		void SetEncoding( SSystem::Charset::EncodingType encoding ) ;
		// 文字エンコーディング取得
		SSystem::Charset::EncodingType GetEncoding( void ) const
		{
			return	m_encoding ;
		}
		// ファイルインターフェース生成
		SSystem::SFileInterface * NewFileInterface( void ) ;
		// 文字列出力
		void OutputString( const wchar_t * pwszString ) ;
		// エンコードされた文字列出力
		size_t WriteString( const void * ptrBuf, size_t nBytes ) ;
		// 入力有効化
		atomic_int_t EnableInput( bool flagInput ) ;
		// 入力データ取得（非同期）
		size_t ReadInput( void * ptrBuf, size_t nBytes ) ;
		// 何らかのデータが入力されるまで待つ
		SGLError WaitForInput
			( int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
		// 末尾が画面内に収まるようスクロール
		void ScrollToEndLine( void ) ;
		// 入力カーソルが画面内に収まるようスクロール
		void ScrollForInputCursor( void ) ;
		// 画面全消去
		void ClearAllLines( void ) ;
		// 入力履歴追加
		void AddInputHistory( const wchar_t * pwszHistory ) ;
		// 履歴最大数設定
		void SetInputHistoryLimit( size_t nLimit ) ;

	protected:
		// 入力文字列を追加する
		bool InputString( const wchar_t * pwszString ) ;
		// 最終行を取得（新規行が制限数を超える場合には削除）
		Line * GetLastLine( size_t& iLine ) ;
		// カーソル行を取得
		Line * GetCursorLine( void ) ;
		// 指定行の絶対ｙ座標取得
		int GetLineAbsolutePos( Line * pLine ) const ;
		// 指定行矩形の更新通知
		void PostUpdateLine( Line * pLine ) ;
		// 指定行のラスタライズ（更新通知）
		void RasterizeLine( Line * pLine ) ;
		// 文字表示矩形取得
		bool GetCharacterViewRect
			( SGLImageRect& rectChar, size_t iLine, size_t iChar ) const ;
		// カーソル表示矩形取得
		bool GetInputCursorRect( SGLImageRect& rectCur ) const ;
		// スクロール範囲を正規化
		void NormalizeScrollPos( void ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;

	protected:
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;

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
	} ;

}

#endif

