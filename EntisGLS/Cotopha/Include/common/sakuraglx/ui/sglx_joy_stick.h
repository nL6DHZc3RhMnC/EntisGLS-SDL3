
#if	!defined(__SAKURAGLX_UI_JOYSTICK_H__)
#define	__SAKURAGLX_UI_JOYSTICK_H__

#if	defined(__PLATFORM_WINDOWS__)

#include <mmsystem.h>
#include <XInput.h>
//#pragma comment( lib, "Xinput.lib" )
#pragma comment( lib, "XInput9_1_0.lib" )

#if	_MSC_VER >= 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#endif

namespace	SakuraGL
{
  namespace	UI
  {
	//////////////////////////////////////////////////////////////////////////
	// ジョイスティック・デバイス
	//////////////////////////////////////////////////////////////////////////

	struct	SGLJoyStickState
	{
		S4DVector	vStickPos ;
		uint64_t	stateButtons ;
		float32_t	fpTriggers[8] ;

		SGLJoyStickState( void )
			: vStickPos( 0, 0, 0, 0 ),
				stateButtons(0)
		{
			for ( int i = 0; i < sizeof(fpTriggers)/sizeof(fpTriggers[0]); i ++ )
			fpTriggers[i] = 0.0f ;
		}
		bool IsButtonPushing( size_t iButton ) const
		{
			return	(stateButtons & (((uint64_t) 1) << iButton)) != 0 ;
		}
	} ;

	#if	defined(__COTOPHA__)
	class	native JoyStick
	{
	public:
		// キャプチャー開始
		native SGLError BeginCapture
			( Window* pWnd, uint32_t maskCaptureDev = 0xFFFFFFFF ) ;
		// キャプチャー開始完了
		native SGLError WaitReadyCapture( int64_t msecTimeout ) ;
		// キャプチャー終了
		native SGLError ReleaseCapture( Window* pWnd ) ;
		// ポーリング
		native SGLError PollJoyStick
			( SGLJoyStickState& joyState, size_t idJoyStick = 0 ) ;
	} ;
	#endif

	class	SGLJoyStick	: public SGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLJoyStick, SGLObject )
		// 構築関数
		SGLJoyStick( void ) ;
		// 消滅関数
		virtual ~SGLJoyStick( void ) ;

	public:
		// ジョイスティックボタン
		enum	JoyButton
		{
			joyUp,	joyDown,	joyLeft,	joyRight,
			joyButton1,	joyButton2,	joyButton3,	joyButton4,
			joyButtonCount	= 64,
		} ;
		enum	TriggerButton
		{
			triggerLeft, triggerRight,
			triggerCount	= 8,
		} ;
		#if	!defined(__COTOPHA__)
			#if	defined(__PLATFORM_WINDOWS__)
			enum	DeviceIndex
			{
				deviceJoyStick1,
				deviceJoyStick2,
				deviceXInput0,
				deviceXInput1,
				deviceXInput2,
				deviceXInput3,
				deviceJoyCount			= 2,
				deviceXInputCount		= 4,
				deviceMaskAllJoySick	= 0x0003,
				deviceMaskAllXInput		= 0x003C,
			} ;
			enum	XInputButtonIndex
			{
				buttonDPadUp,
				buttonDPadDown,
				buttonDPadLeft,
				buttonDPadRight,
				buttonStart,
				buttonBack,
				buttonLeftThumb,
				buttonRightThumb,
				buttonLeftShoulder,
				buttonRightShoulder,
				buttonA					= 12,
				buttonB,
				buttonX,
				buttonY,
				buttonLeftTrigger,
				buttonRightTrigger,
			} ;
			#endif
		#endif

	protected:
		#if	defined(__COTOPHA__)
			JoyStick *			m_pJoyStick ;

		#elif	defined(__PLATFORM_WINDOWS__)
			SSystem::SSignalEvent	m_signalReady ;
			uint32_t				m_maskCaptureDev ;

			DWORD				m_maskJoyCaptured ;				// 使用中のジョイスティック
			JOYCAPS				m_jcJoyCaps[deviceJoyCount] ;	// ジョイスティックデバイス情報
			SGLJoyStickState	m_jsJoyState[deviceJoyCount] ;

			DWORD				m_maskXInputDev ;
			XINPUT_STATE		m_xinState[deviceXInputCount] ;

		#elif	defined(__PLATFORM_ANDROID__)
			SGLGenericWindow *	m_pWnd ;
		#endif

	public:
		// キャプチャー開始
		virtual SGLError BeginCapture
			( SGLWindow* pWnd, uint32_t maskCaptureDev = 0xFFFFFFFF ) ;
		// キャプチャー開始完了
		virtual SGLError WaitReadyCapture( int64_t msecTimeout ) ;
		// キャプチャー終了
		virtual SGLError ReleaseCapture( SGLWindow* pWnd ) ;
		// ポーリング
		virtual SGLError PollJoyStick
			( SGLJoyStickState& joyState, size_t idJoyStick = 0 ) ;

	protected:
		#if	defined(__PLATFORM_WINDOWS__)
		// JoyStick ポーリング
		SGLError PollJoyStickState
			( UI::SGLJoyStickState& joyState, size_t iJoyStick ) ;
		// XInput ポーリング
		SGLError PollXInputState
			( UI::SGLJoyStickState& joyState, size_t iXInput ) ;
		// XInput 開始処理
		void PrepareXInput( void ) ;
		// ジョイスティック初期化関数
		class	BeginJoyStickProc	: public SSystem::SProcedure
		{
		public:
			SGLJoyStick *	m_pjs ;
			HWND			m_hWnd ;
		public:
			BeginJoyStickProc( SGLJoyStick * pjs, HWND hWnd ) ;
			virtual void Run( void ) ;
		} ;
		// ジョイスティック終了関数
		class	ReleaseJoyStickProc	: public SSystem::SProcedure
		{
		public:
			DWORD	m_dwDevMask ;
			HWND	m_hWnd ;
		public:
			ReleaseJoyStickProc( DWORD dwDevMask, HWND hWnd ) ;
			virtual void Run( void ) ;
		} ;
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SGLJoyStick	JoyStick ;
	#endif

  }
}

#endif

