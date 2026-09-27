
#if	!defined(__SAKURAGL_WINDOW_PRODUCER_H__)
#define	__SAKURAGL_WINDOW_PRODUCER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 汎用ウィンドウ・表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowViewProducer	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowViewProducer, ESLObject )
		// 対応機能フラグ
		enum	CapacityFlag
		{
			renderableAnyThread	= 0x0001,
		} ;
		virtual uint64_t GetCapacityFlags( void ) const = 0 ;
		// 論理ビューサイズ通知
		virtual void OnChangeVirtualViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) = 0 ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) = 0 ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) = 0 ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) = 0 ;
		// 論理ビュー表示座標取得
		virtual void GetInternalViewPosition( SGLImageRect& rctVirtualView ) = 0 ;
		// 物理ビュー表示領域取得
		virtual bool GetExternalViewPosition( SGLImageRect& rctPhysicalView ) = 0 ;
		// 論理座標→物理ビュー座標変換行列取得
		virtual void GetAffineVirtualToPhysical( SGLAffine& affine ) = 0 ;
		// 論理座標→物理ビュー座標変換
		virtual void VirtualToPhysicalPosition( S2DDVector& vPos ) ;
		// 物理ビュー座標→論理座標変換
		virtual void PhysicalToVirtualPosition( S2DDVector& vPos ) ;
		// 描画スレッドの関連付け
		virtual SGLError AttachViewThread( SGLAbstractWindow * pWnd ) = 0 ;
		// 描画スレッドの関連付け解除
		virtual SGLError DetachViewThread( SGLAbstractWindow * pWnd ) = 0 ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) = 0 ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) = 0 ;
		// 直接描画ハンドラ開始
		virtual RenderContext * BeginDirectView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) = 0 ;
		// 直接描画ハンドラ終了
		virtual void EndDirectView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) = 0 ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) = 0 ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer
			( SGLAbstractWindow * pWnd, bool flagZBuffer ) = 0 ;
		// レイヤードウィンドウ設定
		virtual SGLError EnableLayeredWindow
			( SGLAbstractWindow * pWnd, bool flagLayeredWindow ) = 0 ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) = 0 ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) = 0 ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) = 0 ;
		// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
		virtual SGLSize GetStandardDisplaySize( void ) const = 0 ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ更新タイミング同期用インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowViewSynchronizer
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLWindowViewSynchronizer )
		// 更新タイミング待ち
		virtual SGLError WaitForView( int64_t msecTimeout ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 非ウィンドウ・表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSecondaryViewProducer	: public SSystem::SObject
	{
	protected:
		// レンダリングの透視変換行列
		S4DMatrix	m_matPerspective[2] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSecondaryViewProducer, SObject )
		// 構築関数
		SGLSecondaryViewProducer( void ) ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
			( SGLAbstractWindow * pPrimaryWnd ) = 0 ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pPrimaryWnd, RenderContext * render ) = 0 ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pPrimaryWnd, bool fVSync ) = 0 ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) = 0 ;
		// プライマリウィンドウへの描画も行うか？
		virtual bool DoesDrawToPrimaryWindow( void ) = 0 ;
		// 表示状態か？
		virtual bool IsVisibleView( void ) const = 0 ;
		// レンダリング透視変換行列設定
		virtual void SetMainPerspectiveMatrix
			( RenderContext::StereoViewIndex sviIndex,
									const S4DMatrix& matPers ) ;

	public:
		// 現在の描画対象取得
		static SGLSecondaryViewProducer * GetCurrent( void ) ;
		// 現在の描画対象設定
		static void SetCurrent( SGLSecondaryViewProducer * psvp ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ描画フレームワーク
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowViewFramework	: public ESLObject
	{
	protected:
		// 表示インターフェース
		SGLAbstractWindow *							m_pWnd ;
		SSystem::SSmartPointer<SGLWindowViewProducer>
													m_pViewProducer ;

		SSystem::SPointerArray<SGLSecondaryViewProducer>
													m_aSecondaryViews ;
		SGLSecondaryViewProducer *					m_pVSyncSecondaryView ;
		SGLSecondaryViewProducer *					m_pCurrentView ;

		// 有効画面外枠表示設定
		uint32_t									m_flagsExFrame ;
		SGLPalette									m_rgbExColor ;
		SSystem::SSmartReference<SGLImageObject>	m_refExFrameTile ;
		SSystem::SSmartReference<SGLImageObject>	m_refExFrameLeft ;
		SSystem::SSmartReference<SGLImageObject>	m_refExFrameRight ;
		SSystem::SSmartReference<SGLImageObject>	m_refExFrameUpper ;
		SSystem::SSmartReference<SGLImageObject>	m_refExFrameUnder ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowViewFramework, ESLObject )
		// 構築関数
		SGLWindowViewFramework
			( SGLAbstractWindow * pWnd, SGLWindowViewProducer * pwvp ) ;
		// 消滅関数
		virtual ~SGLWindowViewFramework( void ) ;

	public:
		// SGLWindowViewProducer 取得
		SGLWindowViewProducer * GetView( void ) const
		{
			return	m_pViewProducer ;
		}
		// 表示インターフェース変更
		SGLWindowViewProducer *
			ChangeWindowViewProducer( SGLWindowViewProducer * pwvp ) ;
		// セカンダリビュー追加
		SGLError AttachSecondaryView
				( SGLSecondaryViewProducer * psvp, bool fVSync ) ;
		// セカンダリビュー削除
		SGLError DetachSecondaryView( SGLSecondaryViewProducer * psvp ) ;
		// VSync ビュー設定
		SGLError SetVSyncSecondaryView
				( SGLSecondaryViewProducer * psvp, bool fVSync ) ;
		// 仮想ディスプレイ・有効画面外枠表示設定
		SGLError SetExteriorBackgroundFrame
			( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
				SGLImageObject* pLeft = NULL, SGLImageObject* pRight = NULL,
				SGLImageObject* pUpper = NULL, SGLImageObject* pUnder = NULL ) ;

	public:
		// 描画処理
		void DrawWindow
			( SGLAbstractWindow * pWnd,
				bool fOnWinThread,
				const SGLImageRect * pWindow = NULL,
				SGLImageObject * pImage = NULL,
				SGLImageObject * pZBuffer = NULL,
				SGLImageObject * pStereoLeft = NULL ) ;
		// フレーム描画処理
		void DrawExteriorFrame
				( SGLAbstractWindow * pWnd, RenderContext * render ) ;
		void DrawExteriorFrameImage
			( RenderContext * render, SGLImageObject * pImage,
				int left, int top, int right, int bottom,
				double scale, bool flagLeft, bool flagUpper ) ;
		// 描画反映処理
		void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
		// 現在のビュー取得（セカンダリビューがメインウィンドウに描画している場合）
		SGLSecondaryViewProducer * GetCurrentView( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowDisplayMethod	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowDisplayMethod, ESLObject )
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) = 0 ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) = 0 ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) = 0 ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) = 0 ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd,
				SGLImageObject * pImageRight,
				SGLImageObject * pImageLeft = NULL ) = 0 ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) = 0 ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) = 0 ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) = 0 ;
	} ;

	class	SGLWindowDisplayMethodProducer	 : public SGLWindowViewProducer
	{
	protected:
		SGLAbstractWindow *			m_pWnd ;
		SGLSize						m_sizeVirtual ;
		SGLSize						m_sizePhysical ;

		// 表示インターフェース
		SGLWindowDisplayMethod *	m_pwdmDisplay ;
		SGLWindowViewProducer *		m_pwvpView ;

		// 表示バッファ（物理ビューサイズ）
		SGLImage	m_imgViewRight ;
		SGLImage	m_imgViewLeft ;
		SGLImage	m_imgZBuffer ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLWindowDisplayMethodProducer, SGLWindowViewProducer )
		// 構築関数
		SGLWindowDisplayMethodProducer
			( SGLAbstractWindow * pWnd, SGLWindowViewProducer * pwvpView ) ;
		// 消滅関数
		virtual ~SGLWindowDisplayMethodProducer( void ) ;
		// SGLWindowViewProducer 分離
		SGLWindowViewProducer * DetachWindowViewProducer( void ) ;
		// SGLWindowDisplayMethod 設定
		void SetDisplayMethod( SGLWindowDisplayMethod * pMethod ) ;
		// 対応 SGLWindowDisplayMethod の生成
		static SGLWindowDisplayMethod * NewDisplayMethod( const wchar_t * pszMethodID ) ;

	public:
		// 対応機能フラグ
		virtual uint64_t GetCapacityFlags( void ) const ;
		// 論理ビューサイズ通知
		virtual void OnChangeVirtualViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 論理ビュー表示座標取得
		virtual void GetInternalViewPosition( SGLImageRect& rctVirtualView ) ;
		// 物理ビュー表示領域取得
		virtual bool GetExternalViewPosition( SGLImageRect& rctPhysicalView ) ;
		// 論理座標→物理ビュー座標変換行列取得
		virtual void GetAffineVirtualToPhysical( SGLAffine& affine ) ;
		// 描画スレッドの関連付け
		virtual SGLError AttachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画スレッドの関連付け解除
		virtual SGLError DetachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 直接描画ハンドラ開始
		virtual RenderContext * BeginDirectView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 直接描画ハンドラ終了
		virtual void EndDirectView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer
			( SGLAbstractWindow * pWnd, bool flagZBuffer ) ;
		// レイヤードウィンドウ設定
		virtual SGLError EnableLayeredWindow
			( SGLAbstractWindow * pWnd, bool flagLayeredWindow ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
		// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
		virtual SGLSize GetStandardDisplaySize( void ) const ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;

	} ;

}

#endif

