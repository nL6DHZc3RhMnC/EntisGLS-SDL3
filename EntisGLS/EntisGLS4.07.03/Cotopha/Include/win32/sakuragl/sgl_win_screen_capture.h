
#if	!defined(__SAKURAGL_WIN_SCREEN_CAPTURE_H__)
#define	__SAKURAGL_WIN_SCREEN_CAPTURE_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画面キャプチャー
	//////////////////////////////////////////////////////////////////////////

	class	SGLScreenCapture	: public ESLObject
	{
	protected:
		HWND			m_hWndCapture ;		// キャプチャーターゲット
		bool			m_fWndClientArea ;
		bool			m_fCaptureMode ;
		SGLImageRect	m_rectCapture ;		// キャプチャー領域
		SGLImage		m_imgCapture ;		// キャプチャー用バッファ

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLScreenCapture, ESLObject )
		// 構築関数
		SGLScreenCapture( void ) ;
		// 消滅関数
		virtual ~SGLScreenCapture( void ) ;

	public:
		// キャプチャーターゲット設定
		void SetCaptureTarget
			( HWND hWnd, bool fClientArea = false, bool fCaptureMode = true ) ;
		// キャプチャーサイズ設定
		void SetCaptureSize( int nWidth, int nHeight ) ;
		// キャプチャー座標設定
		void SetCapturePosition( int xPos, int yPos ) ;
		// キャプチャーターゲット取得
		HWND GetCaptureTarget( void ) const ;
		bool IsCaptureClient( void ) const ;
		bool IsCaptureMode( void ) ;
		// キャプチャー実行
		SGLImageObject * Capture( void ) ;

	public:
		// キー操作イベント発生
		void KeyboardEvent
			( int64_t nVirtKey, int64_t nFlags, bool fRelease ) ;
		// マウス操作イベント発生
		enum	MouseEventCode
		{
			mouseLeftDown,
			mouseLeftUp,
			mouseRightDown,
			mouseRightUp,
			mouseMiddleDown,
			mouseMiddleUp,
			mouseWheel,
			mouseXButton1Down,
			mouseXButton1Up,
			mouseXButton2Down,
			mouseXButton2Up,
		} ;
		void MouseEvent( MouseEventCode code, int nDelta ) ;
		// マウス座標移動
		void MouseMove( int xPos, int yPos, bool fDelta ) ;

	} ;

}


#endif

