
#if	!defined(__SAKURAGLX_SPRITE_WINDOW_H__)
#define	__SAKURAGLX_SPRITE_WINDOW_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Window-Sprite インターフェース実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowSprite ;
	class	SGLSpriteWindowPaintInterface
				: public SGLPaintInterface, public SGLTimerInterface
	{
	protected:
		SGLWindowSprite *		m_pSprite ;
		SSystem::STimeCounter	m_timer ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2_NV
			( SGLSpriteWindowPaintInterface,
					SGLPaintInterface, SGLTimerInterface )
		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( SGLPaintInterface )
		// 構築関数
		SGLSpriteWindowPaintInterface( void ) ;
		SGLSpriteWindowPaintInterface( SGLWindowSprite * pSprite ) ;
		// Sprite 関連付け
		void AttachSprite( SGLWindowSprite * pSprite ) ;

	public:	// SGLPaintInterface 実装
		// 描画
		virtual void OnPaint( Window * pWnd, RenderContext * context ) ;
		// 描画前フレーム準備処理（全視点共通処理）
		virtual void OnPrepareFrame( Window * pWnd ) ;
		// 全描画完了
		virtual void OnFinishedFrame( Window * pWnd ) ;

	public:	// SGLTimerInterface 実装
		// タイマー
		virtual void OnTimer( Window * pWnd, uint64_t idTimer ) ;

	} ;

	class	SGLSpriteWindowMouseInterface	: public SGLMouseInterface
	{
	protected:
		SGLSprite *					m_pSprite ;
		SSystem::SArray<uint32_t>	m_mapMouseID ;
		SSystem::SSmartReference<SGLSpriteMouseListener>
									m_refPostListener ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO
			( SGLSpriteWindowMouseInterface, SGLMouseInterface )
		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( SGLMouseInterface )
		// 構築関数
		SGLSpriteWindowMouseInterface( void ) ;
		SGLSpriteWindowMouseInterface( SGLSprite * pSprite ) ;
		// 関連付け
		void AttachSprite( SGLSprite * pSprite ) ;
		// 後方リスナ関連付け
		void AttachPostListener( SGLSpriteMouseListener * pListener ) ;

	public:
		// フラグ正規化
		int64_t NormalizeMouseFlags( int64_t nFlags ) ;
		// MouseID 割り当て
		uint32_t AllocateMouseID( uint32_t idMouse ) ;
		// MouseID 削除
		void FreeMouseID( uint32_t idMouse ) ;

	public:	// SGLMouseInterface 実装
		// マウス移動
		virtual bool OnMouseMove
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( Window * pWnd, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( Window * pWnd, int32_t zDelta,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;

	} ;

	class	SGLSpriteWindowKeyInterface
				: public SGLKeyInterface,
					public SGLCharInputInterface, public SGLCommandInterface
	{
	protected:
		SGLSprite *					m_pSprite ;
		SSystem::SSmartReference<SGLSpriteKeyListener>
									m_refPostListener ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3_NV
			( SGLSpriteWindowKeyInterface,
				SGLKeyInterface, SGLCharInputInterface, SGLCommandInterface )
		ESL_DECLARE_CLASS_OPERATOR_NEW_NV( SGLKeyInterface )
		// 構築関数
		SGLSpriteWindowKeyInterface( void ) ;
		SGLSpriteWindowKeyInterface( SGLSprite * pSprite ) ;
		// 関連付け
		void AttachSprite( SGLSprite * pSprite ) ;
		// 後方リスナ関連付け
		void AttachPostListener( SGLSpriteKeyListener * pListener ) ;
		// 後方リスナ取得
		SGLSpriteKeyListener * GetPostListener( void ) const ;

	public:	// SGLKeyInterface 実装
		// キー入力
		virtual bool OnKeyDown
			( Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( Window * pWnd, int64_t nVirtKey, int64_t nFlags ) ;
		// フォーカス
		virtual void OnSetFocus( Window * pWnd ) ;
		virtual void OnKillFocus( Window * pWnd ) ;

	public:	// SGLCharInputInterface 実装
		// 文字入力
		virtual bool OnChar( Window * pWnd, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( Window * pWnd, SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( Window * pWnd ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( Window * pWnd, const SGLInputCompositionString& icsComp ) ;

	public:	// SGLCommandInterface 実装
		// コマンド
		virtual bool OnCommand
			( Window * pWnd, const uint16_t * pszCmd,
							int64_t nParam, int64_t nCode ) ;

		friend class SGLWindowSprite ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sprite 基底 Window 実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowSprite
				: public SGLSprite, public SGLWindow,
					public SGLWindowMonitorInterface,
					protected SGLPaintInterface, protected SGLMouseInterface
	{
	protected:
		class	DirectSprite	: public SGLSprite
		{
		public:
			SGLWindow *	m_pWindow ;
		public:
			// 更新領域通知
			virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
			// コマンド通知
			virtual bool NotifyCommand
				( const wchar_t * pszCmd,
					int64_t nParam = 0, int64_t nCode = 0,
					int nPriority = commandNormal, bool fOverwritable = false ) ;
		} ;
		SGLSpriteWindowPaintInterface	m_listenPaint ;
		SGLSpriteWindowMouseInterface	m_listenMouse ;
		SGLSpriteWindowKeyInterface		m_listenKey ;
		DirectSprite					m_spriteDirect ;
		bool							m_flagDisablePostUpdate ;
		bool							m_flagFillBack ;
		uint32_t						m_rgbaFillBack ;

		SSystem::STimeCounter			m_timerStillMouse ;
		SGLPoint						m_ptLastMousePos ;
		int64_t							m_msecAutoHideMouse ;

		bool							m_flagInPaint ;
		SSystem::STimeCounter			m_timerPaint ;
		SSystem::SEventSignalNotifier	m_notifierPaint ;

		bool							m_flagAsyncQueue ;
		SSystem::SProcedureQueue		m_queAsyncProc ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3
			( SGLWindowSprite, SGLSprite, SGLWindow, SGLWindowMonitorInterface )
		// 構築関数
		SGLWindowSprite( void ) ;
		// 消滅関数
		virtual ~SGLWindowSprite( void ) ;

	public:
		// 直前のマウス座標を使って OnMouseMove を呼び出す
		void CallMouseMove( void ) ;
		// 直接描画系のルート Sprite を取得
		SGLSprite& GetDirectRootSprite( void )
		{
			return	m_spriteDirect ;
		}
		// マウス入力後置リスナ設定
		void AttachMousePostListener( SGLSpriteMouseListener * pListener ) ;
		// キー入力後置リスナ設定
		void AttachKeyPostListener( SGLSpriteKeyListener * pListener ) ;
		// タイマ処理の有効／無効化
		void EnableSpriteTimer( bool flagTimer ) ;
		// PostUpdate での遅延描画有効／無効化
		void EnablePostUpdate( bool flagUpdate ) ;
		// 自動カーソル消去時間設定（Synchronism::Infinite で自動消去無し）
		void SetAutoHideCursor( int64_t msecTimeout ) ;

	public:	// SGLWindowMonitorInterface
		// 描画完了通知受け取り登録（シグナルを受け取ると自動的に登録解除）
		virtual void AttachOnceSignalForFramePaint( SSystem::SSignalEvent * pSignal ) ;
		// 描画処理中か？
		virtual bool IsWindowPainting( double * pmsecPastPainting = nullptr ) const ;

	public:
		// 非同期処理追加
		virtual SSystem::SProcedureQueue::ProcIdentity
			PostAsyncProcedure
				( SSystem::SProcedure * pProc,
					SSystem::SSignalEvent * pDoneSignal = nullptr,
					bool flagAutoDelete = false, bool flagFence = false ) ;
		// 非同期処理キャンセル
		virtual bool CancelAsyncProcedure
			( SSystem::SProcedureQueue::ProcIdentity procId ) ;
		// 全ての非同期処理が完了するまで待つ
		virtual SGLError WaitForAllAsyncProcedure
			( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
	protected:
		// 非同期処理スレッド開始
		SGLError BeginAsyncQueueThread( void ) ;
		// 非同期処理スレッド終了
		void EndAsyncQueueThread( void ) ;

	public:
		// 背景色取得
		virtual bool GetFillBackColor( uint32_t& argbFill ) const ;
		// 背景色設定
		virtual SGLError SetFillBackColor
			( uint32_t argbFill, bool flagFillBack = true ) ;

	public:
		// SGLSprite の属する SGLWindowSprite を取得
		static SGLWindowSprite *
			WindowOf( SGLSprite * pSprite, S2DDVector * pvPos = NULL ) ;

	public:	// SGLSprite オーバーライド
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
		// フレーム描画完了後処理
		virtual void FinishDrawFrame( void ) ;
		// 更新領域通知
		virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
		// マウス入力キャプチャー要求
		virtual SGLError SetMouseCapture( void ) ;
		// マウス入力キャプチャー解放
		virtual SGLError ReleaseMouseCapture( void ) ;
		// キーフォーカスを要求
		virtual SGLError SetKeyFocus( void ) ;
		// コマンド通知
		virtual bool NotifyCommand
			( const wchar_t * pszCmd,
				int64_t nParam = 0, int64_t nCode = 0,
				int nPriority = commandNormal, bool fOverwritable = false ) ;

	public:	// SGLWindow オーバーライド
		// 仮想ディスプレイ開始
		virtual SGLError CreateDisplay
			( const wchar_t * pszWindowName,
				Window::CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイ終了
		virtual SGLError CloseDisplay( void ) ;
		// ウィンドウ生成
		virtual SGLError CreateWindow
			( const wchar_t * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags = 0, SGLAbstractWindow * pParentWnd = NULL ) ;
		// ウィンドウを閉じる
		virtual SGLError CloseWindow( void ) ;
		// マウスカーソル表示
		virtual SGLError ShowCursor( bool fShow ) ;

	public:
		// Sprite 用インターフェースを Window に関連付ける
		// ※ CreateSubclassWindow 等非標準の関数によってウィンドウを作成した場合に使用
		void BindWindowToSprite( void ) ;

	protected:
		// 描画ハンドラ
		virtual SGLPaintInterface *
					SetPaintInterface( SGLPaintInterface * pPaint ) ;
		virtual SGLPaintInterface *
					SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
		// タイマーハンドラ
		virtual SGLTimerInterface *
					SetTimerInterface( SGLTimerInterface * pTimer ) ;
		// マウス入力インターフェース
		virtual SGLMouseInterface *
					SetMouseInterface( SGLMouseInterface * pMouse ) ;
		virtual SGLMouseInterface *
					SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
		// キー入力インターフェース
		virtual SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
		// 文字入力インターフェース
		virtual SGLCharInputInterface *
					SetCharInputInterface( SGLCharInputInterface * pChar ) ;
		// コマンド・インターフェース
		virtual SGLCommandInterface *
					SetCommandInterface( SGLCommandInterface * pCmd ) ;

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
		// ウィンドウスレッド排他処理用ミューテックス変更
		void SetWindowUIThreadMutex( SSystem::SSharableMutex * pMutex ) ;

	public:
		// Rosetta 用クラス
		virtual const wchar_t * GetRSClassName( void ) const ;
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	protected:	// SGLPaintInterface 実装
		// 描画
		virtual void OnPaint( Window * pWnd, RenderContext * context ) ;
		// 描画前フレーム準備処理（全視点共通処理）
		virtual void OnPrepareFrame( Window * pWnd ) ;

	protected:	// SGLMouseInterface 実装
		// マウス移動
		virtual bool OnMouseMove
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( Window * pWnd, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( Window * pWnd, int32_t zDelta,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( Window * pWnd,
				int32_t xPos, int32_t yPos, int64_t nFlags ) ;

		friend class DirectSprite ;
	} ;


}

#endif
