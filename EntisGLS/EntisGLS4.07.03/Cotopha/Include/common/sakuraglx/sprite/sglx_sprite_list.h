
#if	!defined(__SAKURAGLX_SPRITE_LIST_H__)
#define	__SAKURAGLX_SPRITE_LIST_H__	1

#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// リスト表示基底スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteBasicList
				: public SGLSprite, public SGLSpriteScrollListener
	{
	public:
		// 表示エリア
		enum	ViewArea
		{
			viewOutside		= -1,	// 表示エリア外
			viewNearInside,			// 表示エリア近傍
			viewInside,				// 表示エリア内
		} ;
		struct	ViewListRange
		{
			size_t	iFirst ;
			size_t	iLast ;
		} ;
	
		// リストアイテム
		class	Entry	: public ESLObject
		{
		protected:
			SSystem::SProcedureQueue::ProcIdentity	m_procAsync ;
			ViewArea	m_viewArea ;
			bool		m_flagDetached ;
			bool		m_flagFocus ;
			bool		m_flagSelected ;
			bool		m_flagPressed ;
			bool		m_flagDisabled ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Entry, ESLObject )
			// 構築関数
			Entry( void ) ;
			// 分離前処理
			virtual void OnDetachEntry( SGLSpriteBasicList& listView ) ;
			// フォーカス状態
			virtual bool HasEntryFocus( void ) const ;
			virtual void SetEntryFocus( bool flagFocus ) ;
			// 選択状態
			virtual bool IsEntrySelected( void ) const ;
			virtual void SelectEntry( bool flagSelect ) ;
			// 禁止状態
			virtual bool IsEntryDisabled( void ) const ;
			virtual void DisableEntry( bool flagDisable ) ;
			// クリック時
			virtual void OnPressEntry( SGLSpriteBasicList& listView ) ;
			virtual void OnReleasePressEntry( SGLSpriteBasicList& listView ) ;
			virtual bool IsPressingEntry( void ) const ;
			virtual void OnClickEntry( SGLSpriteBasicList& listView ) ;
			virtual void OnDoubleClickEntry( SGLSpriteBasicList& listView ) ;
			// 現在の表示エリア
			virtual ViewArea CurrentViewArea( void ) const ;
			// 表示エリア変更
			virtual void ChangeViewArea( SGLSpriteBasicList& listView, ViewArea area ) ;
			// 表示サイズ計算
			virtual size_t GetListViewWidth( void ) const = 0 ;
			virtual size_t GetListViewHeight( void ) const = 0 ;
			// 描画準備
			virtual void OnPrepareToDraw( const SGLSpriteBasicList& listView ) ;
			// 描画
			virtual void DrawListEntry
				( const SGLSpriteBasicList& listView,
					S3DRenderContextInterface& render,
					const SGLImageRect& rectList ) = 0 ;
		} ;

		// 動作フラグ
		enum	ListBehaviorFlag
		{
			behaviorScrollable		= 0x0001,	// ホイール操作でスクロール可能
			behaviorSwipable		= 0x0002,	// ドラッグ／スワイプでスクロール
			behaviorAsyncPrepare	= 0x0010,	// 非同期に描画順を行う
		} ;

	protected:
		uint32_t						m_nBehaviorFlags ;
		SGLSize							m_sizeView ;
		int								m_nViewProximity ;
		SGLPoint						m_ptScroll ;

		atomic_int_t					m_nBatchEdit ;
		SSystem::SObjectArray<Entry>	m_list ;
		SSystem::SArray<size_t>			m_yList ;
		
		SSystem::SObjectArray<Entry>	m_delayRemove ;

		// 関連スクロールバー
		SSystem::SSmartReference<SGLSprite>					m_refScrollBar ;
		SSystem::SSmartReference<SGLBasicForm::TrackBar>	m_refTrackBar ;

		// スクロール・スワイプ操作
		bool			m_flagSwiping ;
		uint64_t		m_msStartSwiping ;
		double			m_fpSwipeMoved ;
		S2DDVector		m_vLastSwipe ;

		uint64_t		m_msLastClicked ;
		Entry *			m_pLastClicked ;

		ssize_t			m_iKeyFocus ;
		ssize_t			m_iMouseFocus ;
		Entry *			m_pPressingEntry ;

		// OnPrepareToDraw 呼び出し Procedure
		class	PrepareToDrawProc	: public SSystem::SProcedure
		{
		protected:
			const SGLSpriteBasicList&	m_list ;
			Entry&						m_entry ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PrepareToDrawProc, SProcedure )
			// 構築関数
			PrepareToDrawProc( const SGLSpriteBasicList& list, Entry& entry )
				: m_list( list ), m_entry( entry ) { }
			// スレッド関数
			virtual void Run( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLSpriteBasicList, SGLSprite, SGLSpriteScrollListener )
		// 構築関数
		SGLSpriteBasicList( void ) ;
		SGLSpriteBasicList( const SGLSpriteBasicList& src ) ;
		// 消滅関数
		virtual ~SGLSpriteBasicList( void ) ;

	public:
		// 作成
		SGLError CreateView
			( uint32_t width, uint32_t height, bool flagBuffered = true ) ;
		// 解放
		void ReleaseView( void ) ;
		//ビューサイズ
		SGLError ResizeView( uint32_t width, uint32_t height ) ;
		const SGLSize& GetViewSize( void ) const ;
		// 動作フラグ
		uint32_t GetListBehaviorFlags( void ) const ;
		void SetListBehaviorFlags( uint32_t nFlags ) ;
		// 表示近傍距離
		int GetViewProximity( void ) const ;
		void SetViewProximity( int nProximity ) ;
		// 現在のスクロール位置をスクロールバーへ反映
		void ReflectToScrollBar( void ) ;
		// スクロール位置変更に伴う表示エリア情報の更新通知
		void NorifyListChangeViewArea( void ) ;

	public:
		// リストエントリ数
		size_t GetListEntryCount( void ) const ;
		// リストエントリ取得
		Entry * GetListEntryAt( size_t i ) const ;
		// リストエントリ追加
		size_t AddListEntry( Entry * pEntry ) ;
		void InsertListEntryAt( size_t i, Entry * pEntry ) ;
		// リストエントリ削除
		void RemoveAllListEntries( void ) ;
		void RemoveListEntryAt( size_t i ) ;
		Entry * DetachListEntryAt( size_t i ) ;
		// リストエントリ検索
		ssize_t FindListEntry( Entry * pEntry ) const ;
		// バッチ編集
		void BeginBatchEdit( void ) ;
		void EndBatchEdit( void ) ;
		// リストエントリ描画準備 (OnPrepareToDraw 呼び出し)
		SSystem::SProcedureQueue::ProcIdentity
			AsyncPrepareToDrawEntry( Entry * pEntry ) ;
		void CancelToPrepareEntry( SSystem::SProcedureQueue::ProcIdentity procId ) ;
		void WaitForAsyncPrepare( void ) ;

	public:
		// フォーカス設定（true の場合、それ以外のエントリは false に設定）
		virtual void SetKeyFocusEntryAt( size_t iEntry, bool flagFocus = true ) ;
		virtual void SetFocusEntryAt( size_t iEntry, bool flagFocus = true ) ;
		// フォーカス取得
		virtual ssize_t FindFocusEntry( void ) const ;
		// アイテムの選択状態設定
		//（flagFocus&&flagRadio が true の場合、それ以外のエントリは false に設定）
		virtual void SelectEntryAt
			( size_t iEntry, bool flagFocus, bool flagRadio = false ) ;
		// アイテムが選択状態か？
		virtual bool IsSelectedEntryAt( size_t iEntry ) const ;
		// 禁止状態を設定
		virtual void DisableEntryAt( size_t iEntry, bool flagDisable ) ;
		// アイテムが禁止状態か？
		virtual bool IsDisabledEntryAt( size_t iEntry ) const ;

	public:
		// 全表示高取得
		size_t GetTotalListHeight( void ) const ;
		// 表示幅最大取得
		size_t GetMaxWidthOfList( void ) const ;
		// 表示リスト範囲取得
		void GetViewListRange( ViewListRange& range ) const ;
		// 表示リスト範囲が変わったか？
		bool IsChangedViewListRange( const ViewListRange& range ) const ;
		// ローカル座標→エントリ指標
		size_t IndexFromPointY( int yView ) const ;
		ssize_t TestHitListEntry( int yView ) const ;
		// エントリ指標→ローカル座標
		SGLPoint PointFromIndex( size_t i ) const ;
	protected:
		// リスト毎の下辺ｙ座標更新
		void UpdateListHeight( size_t iUpdateFrom = 0 ) ;

	protected:	// SGLSprite オーバーライド
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		// 表示状態の子スプライトに対し BeforeDraw を呼び出し
		virtual void BeforeDrawChildren
			( Stereo3DView s3dView = s3dMonoview ) ;
	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
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


	//////////////////////////////////////////////////////////////////////////
	// メニュースプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteMenuList	: public SGLSpriteBasicList
	{
	public:
		enum	MenuType
		{
			menuButton,
			menuCheck,
			menuRadio,
			menuTypeCount,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiMenuType[menuTypeCount+1] ;

		enum	MenuStatus
		{
			statusNormal,
			statusFocus,
			statusSelected,
			statusFocusSelected,
			statusPressed,
			statusDisabled,
			statusSelectDisabled,
			statusCount,
		} ;

		enum	MenuNotificationCode
		{
			ncodeClicked,
			ncodeChecked,
			ncodeUnchecked,
		} ;

		// メニュー表示スタイル
		struct	MenuStyle	: public SGLSpriteMessage::RichTextStyle
		{
			MenuType	menuType ;
			SGLPoint	ptTextOffset ;
			SGLRect		rectTextMargin ;
			size_t		nMenuHeight ;
			SGLPalette	argbBackColor[statusCount] ;

			// 構築関数（デフォルト値）
			MenuStyle( void ) ;
			// 構築関数（複製）
			MenuStyle( const MenuStyle& style ) ;
			// 代入
			const MenuStyle& operator = ( const MenuStyle& style ) ;
		} ;

		// リストアイテム
		class	MenuEntry	: public SGLSpriteBasicList::Entry
		{
		protected:
			SSystem::SString			m_strID ;
			int64_t						m_nCmdParam ;

			size_t						m_nHeight ;
			SGLRect						m_rectMargin ;
			SGLPoint					m_ptText ;
			SGLSize						m_sizeAlign ;
			SSystem::SSmartPointer
					<SGLImageObject>	m_imgText ;
			SSystem::SString			m_strText ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MenuEntry, Entry )
			// 構築関数
			MenuEntry( const wchar_t * pwszID, int64_t nCmdParam ) ;
			// 文字列設定
			void SetText( SGLSpriteMenuList& menu, const wchar_t * pwszText ) ;
			// 文字列取得
			const SSystem::SString& GetText( void ) const ;
			// クリック通知処理
			void NotifyMenuClick
				( SGLSpriteBasicList& listView,
					MenuNotificationCode ncode = ncodeClicked ) ;

		public:	// Entry
			// クリック時
			virtual void OnPressEntry( SGLSpriteBasicList& listView ) ;
			virtual void OnReleasePressEntry( SGLSpriteBasicList& listView ) ;
			virtual void OnClickEntry( SGLSpriteBasicList& listView ) ;
			virtual void OnDoubleClickEntry( SGLSpriteBasicList& listView ) ;
			// 表示サイズ計算
			virtual size_t GetListViewWidth( void ) const ;
			virtual size_t GetListViewHeight( void ) const ;
			// 描画準備
			virtual void OnPrepareToDraw( const SGLSpriteBasicList& listView ) ;
			// 描画
			virtual void DrawListEntry
				( const SGLSpriteBasicList& listView,
					S3DRenderContextInterface& render,
					const SGLImageRect& rectList ) ;
		} ;

	protected:
		// スタイル
		SSystem::SString	m_strFontFace ;
		SSystem::SString	m_strRubyFont ;
		SSystem::SString	m_strProhibition ;
		MenuStyle			m_styleMenu ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteMenuList, SGLSpriteBasicList )
		// 構築関数
		SGLSpriteMenuList( void ) ;
		SGLSpriteMenuList( const SGLSpriteMenuList& src ) ;
		// 消滅関数
		virtual ~SGLSpriteMenuList( void ) ;

	public:
		// メニュースタイル取得
		const MenuStyle& GetMenuStyle( void ) const ;
		// メニュースタイル設定
		void SetMenuStyle( const MenuStyle& style ) ;
		void SetMenuStyleXML( const SSystem::SXMLDocument& xmlStyle ) ;
		// スタイル解釈
		static void ParseMenuStyle
			( MenuStyle& style,
				SSystem::SString& strFontFace,
				SSystem::SString& strRubyFont,
				const SSystem::SXMLDocument& xmlStyle ) ;
		// 文字画像生成
		SGLError RasterizeImageOfTextXML
			( SGLImageObject& imgText,
				SGLPoint& ptOffset,
				SGLSize& sizeAlign, const wchar_t * pwszTextXML ) const ;
		// 描画位置アライメント調整
		SGLPoint CalcDrawOffsetOfImageText
			( const SGLPoint& ptOffset, const SGLSize& sizeAlign ) const ;
		// 現在のスタイルでのメニュー文字表示領域サイズを計算
		SGLSize CalcSizeOfMenuTextArea( void ) const ;

	public:
		// メニューエントリ追加
		size_t AddMenuEntry
			( const wchar_t * pwszTextXML,
				const wchar_t * pwszID = nullptr, int64_t nCmdParam = 0 ) ;
	} ;

}

#endif

