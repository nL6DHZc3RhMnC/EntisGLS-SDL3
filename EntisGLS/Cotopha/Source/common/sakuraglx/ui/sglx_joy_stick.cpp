
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/ui/sglx_joy_stick.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ジョイスティック・デバイス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::UI::SGLJoyStick, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLJoyStick::SGLJoyStick( void )
{
#if	defined(__COTOPHA__)
	m_pJoyStick = NULL ;
#elif	defined(__PLATFORM_WINDOWS__)
	m_signalReady.Initialize( false ) ;
	m_maskCaptureDev = 0 ;
	m_maskJoyCaptured = 0 ;
	m_maskXInputDev = 0 ;
#elif	defined(__PLATFORM_ANDROID__)
	m_pWnd = NULL ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLJoyStick::~SGLJoyStick( void )
{
#if	defined(__COTOPHA__)
	delete	m_pJoyStick ;
	m_pJoyStick = NULL ;
#elif	defined(__PLATFORM_WINDOWS__)
	m_signalReady.Delete() ;
#elif	defined(__PLATFORM_ANDROID__)
	m_pWnd = NULL ;
#endif
}

// キャプチャー開始
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::BeginCapture
	( SGLWindow* pWnd, uint32_t maskCaptureDev )
{
	if ( pWnd == NULL )
	{
		return	sglErrFailed ;
	}
#if	defined(__COTOPHA__)
	if ( m_pJoyStick == NULL )
	{
		try
		{
			m_pJoyStick = new UI::JoyStick ;
		}
		catch ( ... )
		{
			m_pJoyStick = NULL ;
			return	sglErrNotSupported ;
		}
	}
	return	m_pJoyStick->BeginCapture
				( pWnd->GetWindowObject(), maskCaptureDev ) ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_maskJoyCaptured != 0 )
	{
		// すでに使用中
		return	sglErrFailed ;
	}
	m_maskCaptureDev = maskCaptureDev ;
	/*
	if ( ::joyGetNumDevs() == 0 )
	{
		return	sglErrNotSupported ;
	}
	*/
	SSyncProcedure *	pProc =
			new SSyncProcedure
				( new BeginJoyStickProc
					( this, pWnd->GetWindowHandle() ), true ) ;
	m_signalReady.ResetSignal() ;
	pWnd->PostUIThread( pProc ) ;
	if ( pProc->WaitDone( 100 ) == errSuccess )
	{
		delete	pProc ;
		return	sglErrSuccess ;
	}
	pProc->SetAutoDelete( true ) ;
	return	sglErrPending ;

#elif	defined(__PLATFORM_ANDROID__)
	m_pWnd = ESLTypeCast<SGLGenericWindow>( pWnd ) ;
	return	sglErrSuccess ;

#else
	return	sglErrNotSupported ;
#endif
}

// キャプチャー開始完了
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::WaitReadyCapture( int64_t msecTimeout )
{
#if	defined(__COTOPHA__)
	if ( m_pJoyStick != NULL )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;

#elif	defined(__PLATFORM_WINDOWS__)
	return	(SGLError) m_signalReady.Wait( msecTimeout ) ;

#elif	defined(__PLATFORM_ANDROID__)
	return	sglErrSuccess ;

#else
	return	sglErrNotSupported ;
#endif
}

// キャプチャー終了
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::ReleaseCapture( SGLWindow* pWnd )
{
	if ( pWnd == NULL )
	{
		return	sglErrFailed ;
	}
#if	defined(__COTOPHA__)
	if ( m_pJoyStick != NULL )
	{
		return	m_pJoyStick->ReleaseCapture( pWnd->GetWindowObject() ) ;
	}
	return	sglErrNotSupported ;

#elif	defined(__PLATFORM_WINDOWS__)
	m_signalReady.Wait( 1000 ) ;
	m_signalReady.ResetSignal() ;
	//
	if ( m_maskJoyCaptured & deviceMaskAllJoySick )
	{
		SSyncProcedure *	pProc =
			new SSyncProcedure
				( new ReleaseJoyStickProc
					( m_maskJoyCaptured, pWnd->GetWindowHandle() ), true ) ;
		pWnd->PostUIThread( pProc ) ;
		if ( pProc->WaitDone( 100 ) == errSuccess )
		{
			delete	pProc ;
		}
		else
		{
			pProc->SetAutoDelete( true ) ;
		}
	}
	m_maskJoyCaptured = 0 ;
	return	sglErrSuccess ;

#elif	defined(__PLATFORM_ANDROID__)
	m_pWnd = NULL ;
	return	sglErrSuccess ;

#else
	return	sglErrNotSupported ;
#endif
}

// ポーリング
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::PollJoyStick
	( UI::SGLJoyStickState& joyState, size_t idJoyStick )
{
#if	defined(__COTOPHA__)
	if ( m_pJoyStick != NULL )
	{
		return	m_pJoyStick->PollJoyStick( joyState, idJoyStick ) ;
	}
	return	sglErrNotSupported ;

#elif	defined(__PLATFORM_WINDOWS__)
	if ( !(m_maskCaptureDev & (1 << idJoyStick)) )
	{
		return	sglErrInvalidParam ;
	}
	if ( (idJoyStick >= deviceJoyStick1)
		&& (idJoyStick <= deviceJoyStick2) )
	{
		return	PollJoyStickState
					( joyState, idJoyStick - deviceJoyStick1 ) ;
	}
	if ( (idJoyStick >= deviceXInput0)
		&& (idJoyStick <= deviceXInput3) )
	{
		return	PollXInputState
					( joyState, idJoyStick - deviceXInput0 ) ;
	}
	return	sglErrNotSupported ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( (m_pWnd != NULL) && (idJoyStick == 0) )
	{
		joyState.vStickPos.x = 0 ;
		joyState.vStickPos.y = 0 ;
		joyState.vStickPos.z = 0 ;
		joyState.vStickPos.w = 0 ;
		joyState.stateButtons = 0 ;
		for ( int i = 0; i < triggerCount; i ++ )
		{
			joyState.fpTriggers[i] = 0.0f ;
		}
		//
		m_pWnd->GetJoyStickPosition( joyState.vStickPos ) ;
		//
		joyState.stateButtons = m_pWnd->GetJoyButtonMask() ;
		/*
		static const int	joyButtons[] =
		{
			SGLGenericWindow::joyStickUp, SGLGenericWindow::joyStickDown,
			SGLGenericWindow::joyStickLeft, SGLGenericWindow::joyStickRight,
			SGLGenericWindow::joyButtonA, SGLGenericWindow::joyButtonB, SGLGenericWindow::joyButtonC,
			SGLGenericWindow::joyButtonX, SGLGenericWindow::joyButtonY, SGLGenericWindow::joyButtonZ,
			SGLGenericWindow::joyButtonL1, SGLGenericWindow::joyButtonR1,
			SGLGenericWindow::joyButtonL2, SGLGenericWindow::joyButtonR2,
			SGLGenericWindow::joyButtonStart, SGLGenericWindow::joyButtonSelect,
			-1,
		} ;
		for ( int i = 0; joyButtons[i] >= 0; i ++ )
		{
			if ( m_pWnd->IsJoyButtonPushing( joyButtons[i] ) )
			{
				joyState.stateButtons |= (1 << i) ;
			}
		}
		*/
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;

#else
	return	sglErrNotSupported ;
#endif
}


#if	defined(__PLATFORM_WINDOWS__)
// JoyStick ポーリング
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::PollJoyStickState
	( UI::SGLJoyStickState& joyState, size_t iJoyStick )
{
	if ( iJoyStick >= deviceJoyCount )
	{
		return	sglErrNotSupported ;
	}
	if ( !(m_maskJoyCaptured & (1 << iJoyStick)) )
	{
		return	sglErrNotSupported ;
	}
	static const UINT	nJoyIDs[2] =
	{
		JOYSTICKID1, JOYSTICKID2
	} ;
	JOYINFOEX	jix ;
	jix.dwSize = sizeof(jix) ;
	jix.dwFlags = JOY_RETURNALL ;
	if ( ::joyGetPosEx( nJoyIDs[iJoyStick], &jix ) != JOYERR_NOERROR )
	{
		return	sglErrNotSupported ;
	}
	//
	// 座標取得
	//
	joyState.vStickPos.x =
		(float32_t) (jix.dwXpos - m_jcJoyCaps[iJoyStick].wXmin) * 2
			/ (float32_t) (m_jcJoyCaps[iJoyStick].wXmax
						- m_jcJoyCaps[iJoyStick].wXmin) - 1.0F ;
	joyState.vStickPos.y =
		(float32_t) (jix.dwYpos - m_jcJoyCaps[iJoyStick].wYmin) * 2
			/ (float32_t) (m_jcJoyCaps[iJoyStick].wYmax
						- m_jcJoyCaps[iJoyStick].wYmin) - 1.0F ;
	if ( m_jcJoyCaps[iJoyStick].wCaps & JOYCAPS_HASZ )
	{
		joyState.vStickPos.z =
			(float32_t) (jix.dwZpos - m_jcJoyCaps[iJoyStick].wZmin) * 2
				/ (float32_t) (m_jcJoyCaps[iJoyStick].wZmax
							- m_jcJoyCaps[iJoyStick].wZmin) - 1.0F ;
	}
	else
	{
		joyState.vStickPos.z = 0 ;
	}
	if ( m_jcJoyCaps[iJoyStick].wCaps & JOYCAPS_HASR )
	{
		joyState.vStickPos.w =
			(REAL32) (jix.dwRpos - m_jcJoyCaps[iJoyStick].wRmin) * 2
				/ (REAL32) (m_jcJoyCaps[iJoyStick].wRmax
							- m_jcJoyCaps[iJoyStick].wRmin) - 1.0F ;
	}
	else
	{
		joyState.vStickPos.w = 0 ;
	}
	//
	// ボタン状態設定
	//
	joyState.stateButtons =
				((uint32_t) jix.dwButtons << 4)
						| ((uint32_t) jix.dwButtons >> 28) ;
	//
	// POV
	//
	if ( (jix.dwFlags & JOY_RETURNPOV) && (jix.dwPOV != JOY_POVCENTERED) )
	{
		if ( (jix.dwPOV < JOY_POVRIGHT) || (jix.dwPOV > JOY_POVLEFT) )
		{
			joyState.stateButtons |= (1 << joyUp) ;
		}
		if ( (jix.dwPOV > JOY_POVRIGHT) && (jix.dwPOV < JOY_POVLEFT) )
		{
			joyState.stateButtons |= (1 << joyDown) ;
		}
		if ( (jix.dwPOV > JOY_POVFORWARD) && (jix.dwPOV < JOY_POVBACKWARD) )
		{
			joyState.stateButtons |= (1 << joyRight) ;
		}
		if ( jix.dwPOV > JOY_POVBACKWARD )
		{
			joyState.stateButtons |= (1 << joyLeft) ;
		}
	}
	//
	// トリガー
	//
	for ( int i = 0; i < triggerCount; i ++ )
	{
		joyState.fpTriggers[i] = 0.0f ;
	}
	//
	m_jsJoyState[deviceJoyStick1 + iJoyStick] = joyState ;
	return	sglErrSuccess;
}

// XInput ポーリング
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLJoyStick::PollXInputState
	( UI::SGLJoyStickState& joyState, size_t iXInput )
{
	if ( iXInput >= deviceXInputCount )
	{
		return	sglErrNotSupported ;
	}
	DWORD	dwResult =
				XInputGetState( (DWORD) iXInput, &m_xinState[iXInput] ) ;
	if ( dwResult != ERROR_SUCCESS )
	{
		m_maskXInputDev &= ~(1 << iXInput) ;
		return	sglErrNotSupported ;
	}
	m_maskXInputDev |= (1 << iXInput) ;
	//
	const XINPUT_STATE	xis = m_xinState[iXInput] ;
	joyState.stateButtons = xis.Gamepad.wButtons ;
	//
	for ( int i = 0; i < triggerCount; i ++ )
	{
		joyState.fpTriggers[i] = 0.0f ;
	}
	//
	if ( xis.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
	{
		joyState.stateButtons |= 1 << buttonLeftTrigger ;
		joyState.fpTriggers[triggerLeft] =
						(float32_t) xis.Gamepad.bLeftTrigger / 255.0f ;
	}
	if ( xis.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
	{
		joyState.stateButtons |= 1 << buttonRightTrigger ;
		joyState.fpTriggers[triggerRight] =
						(float32_t) xis.Gamepad.bRightTrigger / 255.0f ;
	}
	if ( (xis.Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
	{
		joyState.vStickPos.x = (float32_t) xis.Gamepad.sThumbLX / 0x8000 ;
	}
	else
	{
		joyState.vStickPos.x = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
	{
		joyState.vStickPos.y = - (float32_t) xis.Gamepad.sThumbLY / 0x8000 ;
	}
	else
	{
		joyState.vStickPos.y = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
	{
		joyState.vStickPos.z = (float32_t) xis.Gamepad.sThumbRX / 0x8000 ;
	}
	else
	{
		joyState.vStickPos.z = 0.0f ;
	}
	if ( (xis.Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		|| (xis.Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
	{
		joyState.vStickPos.w = - (float32_t) xis.Gamepad.sThumbRY / 0x8000 ;
	}
	else
	{
		joyState.vStickPos.w = 0.0f ;
	}
	return	sglErrSuccess;
}

// XInput 開始処理
//////////////////////////////////////////////////////////////////////////////
void UI::SGLJoyStick::PrepareXInput( void )
{
//	XInputEnable( TRUE ) ;
	//
	UI::SGLJoyStickState	joyState ;
	for ( size_t i = 0; i < deviceXInputCount; i ++ )
	{
		if ( m_maskCaptureDev & (1 << (deviceXInput0 + i)) )
		{
			PollXInputState( joyState, i ) ;
		}
	}
}

// ジョイスティック初期化関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLJoyStick::BeginJoyStickProc::BeginJoyStickProc( SGLJoyStick * pjs, HWND hWnd )
{
	m_pjs = pjs ;
	m_hWnd = hWnd ;
}

void UI::SGLJoyStick::BeginJoyStickProc::Run( void )
{
	if ( m_pjs->m_maskCaptureDev & deviceMaskAllXInput )
	{
		m_pjs->PrepareXInput() ;
	}
	//
	::joyGetDevCaps( JOYSTICKID1, &(m_pjs->m_jcJoyCaps[0]), sizeof(JOYCAPS) ) ;
	::joyGetDevCaps( JOYSTICKID2, &(m_pjs->m_jcJoyCaps[1]), sizeof(JOYCAPS) ) ;
	//
	m_pjs->m_maskJoyCaptured = 0 ;
	//
	if ( (m_pjs->m_maskCaptureDev & (1 << deviceJoyStick1))
		&& ::joySetCapture
			( m_hWnd, JOYSTICKID1, 15, FALSE ) == JOYERR_NOERROR )
	{
		m_pjs->m_maskJoyCaptured |= 0x01 ;
	}
	if ( (m_pjs->m_maskCaptureDev & (1 << deviceJoyStick2))
		&& ::joySetCapture
			( m_hWnd, JOYSTICKID2, 15, FALSE ) == JOYERR_NOERROR )
	{
		m_pjs->m_maskJoyCaptured |= 0x02 ;
	}
	if ( m_pjs->m_maskJoyCaptured )
	{
		for ( int i = 0; i < 2; i ++ )
		{
			m_pjs->m_jsJoyState[i].vStickPos.x = 0 ;
			m_pjs->m_jsJoyState[i].vStickPos.y = 0 ;
			m_pjs->m_jsJoyState[i].vStickPos.z = 0 ;
			m_pjs->m_jsJoyState[i].vStickPos.w = 0 ;
			m_pjs->m_jsJoyState[i].stateButtons = 0 ;
		}
	}
	m_pjs->m_signalReady.SetSignal() ;
}

// ジョイスティック終了関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLJoyStick::ReleaseJoyStickProc::ReleaseJoyStickProc( DWORD dwDevMask, HWND hWnd )
{
	m_dwDevMask = dwDevMask ;
	m_hWnd = hWnd ;
}

void UI::SGLJoyStick::ReleaseJoyStickProc::Run( void )
{
	if ( m_dwDevMask & 0x01 )
	{
		::joyReleaseCapture( JOYSTICKID1 ) ;
	}
	if ( m_dwDevMask & 0x02 )
	{
		::joyReleaseCapture( JOYSTICKID2 ) ;
	}
}

#endif
