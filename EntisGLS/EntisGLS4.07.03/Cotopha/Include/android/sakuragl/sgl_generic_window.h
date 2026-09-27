
#if	!defined(__SAKURAGL_GENERIC_WINDOW_H__)
#define	__SAKURAGL_GENERIC_WINDOW_H__	1

#include <esl/esl_java_object.h>
#include <sakura/ssys_reference_array.h>
#include <sakuragl/window/sgl_window_producer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 汎用ウィンドウ
	//////////////////////////////////////////////////////////////////////////

	class	SGLGenericWindow
				: public SGLAbstractWindow, public JNI::JavaObject
	{
	protected:
		// 環境
		SSystem::SEnvironmentInterface *	m_pEnv ;

		// 表示インターフェース
		SGLWindowViewFramework		m_wvfFramework ;
		SSystem::SCriticalSection	m_csViewSync ;
		SGLWindowViewSynchronizer *	m_pViewSync ;
		int							m_nRequestFPS ;

		// UIスレッド同期ミューテックス
		SSystem::SMutex *			m_pMutexUI ;

		// 動作モード
		bool					m_flagCreated ;
		bool					m_flagModeDisplay ;
		bool					m_flagFullscreen ;
		bool					m_flagLayeredWindow ;
		SSystem::SString		m_strCaption ;
		Window::CooperationMode	m_modeCooperation ;
		uint64_t				m_flagsOption ;
		SGLSize					m_sizeVirtual ;
		SGLSize					m_sizePhysical ;
		uint32_t				m_nBitsPerPixel ;
		uint32_t				m_nFrequency ;
		uint32_t				m_flagsLayout ;
		SGLPoint				m_ptLayoutOffset ;
		SGLSize					m_sizeLayoutOriginal ;

		// ウィンドウ座標
		SGLImageRect			m_rectWindow ;

		// 画面の物理サイズ
		SGLSize					m_sizePhysicalDisplay ;

		// マウス座標
		atomic_int_t			m_countTouching ;
		int						m_idPrimaryTouch ;
		bool					m_flagPrimaryTouchDown ;
		S2DDVector				m_vPrimaryTouch ;

		// ジョイスティック
		bool					m_flagJoyStick ;
		S4DVector				m_vJoystickPos ;

		// 関連ウィンドウ
		SSystem::SSmartReference<SGLAbstractWindow>	m_refParentWnd ;
		SSystem::SReferenceArray<SGLGenericWindow>	m_arrChildren ;

		// メニュー
		JNI::JSmartObject		m_jsobjMenu ;

		// 同期シグナル
		SSystem::SSignalEvent	m_signalDonePaint ;

		// 同期プロシージャ
		class	SmartLockProcedure	: public SSystem::SProcedure
		{
		protected:
			SSystem::SProcedure *	m_pProc ;
			SSystem::SMutex *		m_pMutexUI ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( SmartLockProcedure, SProcedure )
			// 構築関数
			SmartLockProcedure
				( SSystem::SProcedure * pProc, SSystem::SMutex * pMutex ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 開始前の処理
			virtual void Prepare( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLGenericWindow, SGLAbstractWindow, JavaObject )
		// 構築関数
		SGLGenericWindow
			( SGLWindowViewProducer * pwvp,
				SSystem::SEnvironmentInterface * env ) ;
		// 消滅関数
		virtual ~SGLGenericWindow( void ) ;
		// 表示インターフェース取得
		SGLWindowViewProducer * GetWindowViewProducer( void ) const ;
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
		// 描画タイミングインターフェース設定
		SGLWindowViewSynchronizer *
				SetViewSynchronizer( SGLWindowViewSynchronizer * pViewSync ) ;
		// 設定されている描画タイミングインターフェース取得
		SGLWindowViewSynchronizer * GetViewSynchronizer( void ) const
		{
			return	m_pViewSync ;
		}

	public:	// 仮想ディスプレイメソッド
		// 仮想ディスプレイ開始
		virtual SGLError CreateDisplay
			( const wchar_t * pszWindowName,
				Window::CooperationMode mode,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイ終了
		virtual SGLError CloseDisplay( void ) ;
		// オプション機能フラグ取得
		virtual uint64_t GetOptionalFlags( void ) ;
		// オプション機能フラグ設定
		virtual void SetOptionalFlags( uint64_t nFlags ) ;
		// ウィンドウモード変更
		virtual SGLError ChangeCooperationLevel( Window::CooperationMode mode ) ;
		// ウィンドウモード取得
		virtual Window::CooperationMode GetCooperationLevel( void ) ;
		// 仮想ディスプレイサイズ変更
		virtual SGLError ChangeDisplaySize
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
		// 仮想ディスプレイサイズ取得
		virtual SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
		// 物理モニタの解像度を変更するか？
		virtual SGLError EnableChangePhysicalMode( bool flagEnable ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer( bool flagZBuffer ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode( const wchar_t * pszMethodID ) ;
		// 仮想ディスプレイ・ウィンドウ初期座標設定
		virtual SGLError InitWindowPosition
			( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウの通常座標取得
		virtual SGLError GetNormalWindowPosition
			( SGLPoint& ptWindow, SGLSize * pWindowSize = NULL ) ;
		// 仮想ディスプレイ・ウィンドウ内表示座標取得
		virtual SGLError GetInternalDisplayPosition
				( SGLImageRect& rctRender, SGLImageRect& rctDisplay ) ;
		// 仮想ディスプレイ・有効画面外枠表示設定
		virtual SGLError SetExteriorBackgroundFrame
			( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
				SGLImageObject* pLeft = NULL, SGLImageObject* pRight = NULL,
				SGLImageObject* pUpper = NULL, SGLImageObject* pUnder = NULL ) ;

	public:	// 汎用ウィンドウ専用
		// ウィンドウ生成
		virtual SGLError CreateWindow
			( const wchar_t * pszWindowName,
				uint32_t nWidth, uint32_t nHeight,
				uint32_t nFlags = 0, SGLAbstractWindow * pParentWnd = NULL ) ;
		// ウィンドウを閉じる
		virtual SGLError CloseWindow( void ) ;
		// ウィンドウ位置を設定する
		virtual SGLError SetWindowLayout
			( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;
		// クライアント座標→スクリーン座標変換
		virtual S2DDVector& ScreenPositionFromClient( S2DDVector& vClient ) ;
		// スクリーン座標→クライアント座標変換
		virtual S2DDVector& ClientPositionFromScreen( S2DDVector& vScreen ) ;

	public:	// 仮想ディスプレイ・汎用ウィンドウ共通
		// 画面の更新通知
		virtual SGLError PostUpdate( const SGLImageRect* pUpdate = NULL ) ;
		// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
		virtual SGLError UpdateWindow( Window::UpdateParameter * pUpdate = NULL ) ;
		// ユーザー入力処理
		virtual SGLError ProcessUserInput( int64_t msecTimeout = 1 ) ;
		// 描画スレッドで実行
		virtual SGLError PostRenderingThread
				( SSystem::SProcedure * pProc, PostThreadType postType ) ;
		// UI スレッドで実行
		virtual SGLError PostUIThread( SSystem::SProcedure * pProc ) ;
		// ウィンドウがアクティブ（最前面）か？
		virtual bool IsWindowActive( void ) ;
		// ウィンドウキャプション設定
		virtual SGLError SetWindowCaption( const wchar_t * pszWindowName ) ;
		// マウスカーソル表示
		virtual SGLError ShowCursor( bool fShow ) ;
		// マウスカーソル表示状態取得
		virtual bool IsShowCursor( void ) ;
		// マウスカーソル変更
		virtual SGLError SetCursor( const wchar_t * pszCursorID ) ;
		// マウスカーソル座標移動
		virtual SGLError MoveCursorPosition
			( int32_t xPos, int32_t yPos, int idMouse = 0 ) ;
		// マウスカーソル座標取得
		virtual SGLError GetCursorPosition
			( SGLPoint& ptCursor, int idMouse = 0 ) ;
		// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
		virtual int GetMonitorFrequency( void ) ;
		// ウィンドウメニューの設定
		virtual SGLError AttachMenu( SGLWindowMenu * pMenu ) ;

		// マウスイベントキャプチャー
		virtual SGLError CaptureMouse( int idMouse = 0 ) ;
		virtual SGLError ReleaseMouse( int idMouse = 0 ) ;

	public:
		// 描画インターフェース取得
		virtual S3DRenderContextInterface * GetRenderContext
			( S3DRenderContextInterface::StereoViewIndex sviView
						= S3DRenderContextInterface::stereoViewAuto ) ;
		virtual void ReleaseRenderContext
						( S3DRenderContextInterface* context ) ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;

	public:
		// ウィンドウスレッド排他処理用ミューテックス変更
		void SetWindowUIThreadMutex( SSystem::SMutex * pMutex ) ;

	protected:	// ウィンドウ・レイアウト
		// ウィンドウ作成（低水準）
		SGLError CreateWindowSimply
			( const wchar_t * pwszWindowName,
						uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウ破棄（低水準）
		SGLError CloseWindowSimply( void ) ;
		// ウィンドウのレイアウトに基づいて位置を調整する
		void UpdateWindowLayout( void ) ;
		// 描画処理
		void DrawWindow( bool fOnWinThread ) ;
		// 表示反映処理
		void FlipView( bool fVSync, bool fOnWinThread ) ;

	protected:	// Java 呼び出し
		// EntisGLSurfaceView EntisGLS.getMainSurfaceView()
		jobject java_EntisGLS_getMainSurfaceView( void ) ;
		// void EntisGLS.postUpdateView()
		void java_EntisGLS_postUpdateView( void ) ;
		// boolean EntisGLS.callNativeOnUIThread( ByteBuffer buf )
		bool java_EntisGLS_callNativeOnUIThread( jobject buf ) ;
		// boolean EntisGLS.callNativeOnRenderingThread( ByteBuffer buf, boolean fDelay )
		bool java_EntisGLS_callNativeOnRenderingThread( jobject buf, bool flagDelay ) ;
		// boolean EntisGLS.procedureAsyncNoRenderingThread( ByteBuffer buf )
		bool java_EntisGLS_callNativeOnAsyncNoRenderingThread( jobject buf ) ;

	public:	// Java フレームワークから呼び出される
		// 画面サイズ変更通知
		void OnSurfaceChanged( int width, int height ) ;
		// 描画
		void OnDraw( void ) ;
		// タッチ通知
		bool OnTouchedDown( double x, double y, int id ) ;
		bool OnTouchedUp( double x, double y, int id ) ;
		bool OnTouchedMoved( double x, double y, int id ) ;
		// キー入力通知
		bool OnKeyDown( int key ) ;
		bool OnKeyUp( int key ) ;
		bool OnChar( int code ) ;
		// ジョイスティック通知
		bool OnJoystickAxis( float x, float y, float z, float rz ) ;
		// タイマー処理
		void OnTimer( void ) ;
		// システムイベント通知
		void OnSystemEvent( const wchar_t * pwszSysCommand ) ;
		// メニューコマンド
		bool OnMenuCommand( int id ) ;

	public:
		// ジョイパッド・ボタン
		enum	JoyButtonIndex
		{
			joyButtonNull,
			joyStickUp, joyStickDown, joyStickLeft, joyStickRight,
			joyButtonA, joyButtonB, joyButtonC,
			joyButtonX, joyButtonY, joyButtonZ,
			joyButtonL1, joyButtonR1, joyButtonL2, joyButtonR2,
			joyButtonStart, joyButtonSelect,
		} ;
	protected:
		int64_t		m_maskJoyButtonPushed ;

		// ジョイボタン変換テーブル
		static const int	g_joyButtonFromAndroidKeyCode[0x100] ;
		// 仮想キーコード変換テーブル
		static const int	g_vkeyFromAndroidKeyCode[0x100] ;

	public:
		// Android キーコードを仮想キーコードへ変換
		static int VirtualKeyFromAndroidKeyCode( int key )
		{
			if ( (key >= 0) & (key < 0x100) )
			{
				return	g_vkeyFromAndroidKeyCode[key] ;
			}
			return	0 ;
		}
		// ジョイパッドボタンの状態を取得
		int64_t GetJoyButtonMask( void ) const
		{
			return	m_maskJoyButtonPushed ;
		}
		bool IsJoyButtonPushing( int joyButton ) const
		{
			return	((m_maskJoyButtonPushed & (1 << joyButton)) != 0) ;
		}
		// ジョイスティックの座標を取得
		void GetJoyStickPosition( S4DVector & vPos ) const
		{
			vPos = m_vJoystickPos ;
		}

	} ;

}


#endif

