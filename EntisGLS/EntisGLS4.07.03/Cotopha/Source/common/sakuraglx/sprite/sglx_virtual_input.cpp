
#include <sakuraglx/sakuraglx.h>

using namespace SSystem ;
using namespace SakuraGL ;



//////////////////////////////////////////////////////////////////////////////
// 入力フィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLVirtualInput::InputFilter,
			 SGLSpriteMouseListener, SGLSpriteKeyListener )

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
const SGLVirtualInput::InputEvent*
	SGLVirtualInput::InputFilter::GetFilterAs
				( const SGLVirtualInput::InputEvent& evIn ) const
{
	const InputEvent*
			pevOut = m_filterEvent.GetAs( InputEvent(evIn) ) ;
	if ( pevOut == NULL )
	{
		if ( evIn.codeKey & vkeyContextMask )
		{
			InputEvent	evInTemp = evIn ;
			evInTemp.codeKey &= vkeyCodeMask ;
			pevOut = m_filterEvent.GetAs( evInTemp ) ;
		}
	}
	if ( pevOut != NULL )
	{
		for ( int i = 0; i < 0x100; i ++ )	// 無限ループ防止
		{
			const InputEvent*
					pevNext = m_filterEvent.GetAs( InputEvent(*pevOut) ) ;
			if ( pevNext == NULL )
			{
				break ;
			}
			pevOut = pevNext ;
		}
	}
	return	pevOut ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::FilterInputEvent
		( const SGLVirtualInput::InputEvent& evIn, bool fKeyPress )
{
	const InputEvent*	pevOut = GetFilterAs( evIn ) ;
	if ( pevOut == NULL )
	{
		return	false ;
	}
	SGLSprite *	pSprite = m_refRedirect ;
	if ( pevOut->typeDevice == deviceKeyboard )
	{
		if ( pSprite != NULL )
		{
			if ( fKeyPress )
			{
				pSprite->OnKeyDown( pevOut->codeKey, 0 ) ;
			}
			else
			{
				pSprite->OnKeyUp( pevOut->codeKey, 0 ) ;
			}
			return	true ;
		}
	}
	else if ( pevOut->typeDevice == deviceCommand )
	{
		if ( pSprite != NULL )
		{
			if ( fKeyPress )
			{
				pSprite->OnCommand( pevOut->strCommand ) ;
			}
			return	true ;
		}
	}
	else if ( pevOut->typeDevice == deviceSignal )
	{
		if ( pSprite != NULL )
		{
			if ( fKeyPress )
			{
				pSprite->OnCommand
					( pevOut->strCommand,
						0, 0, SGLSprite::commandAbove, true ) ;
			}
			return	true ;
		}
	}
	else if ( pevOut->typeDevice == deviceJoyStick )
	{
		if ( m_pQueue != NULL )
		{
			if ( fKeyPress )
			{
				m_pQueue->PressInputEvent( *pevOut ) ;
			}
			else
			{
				m_pQueue->ReleaseInputEvent( *pevOut ) ;
			}
		}
		else if ( pSprite != NULL )
		{
			int	nVirtKey = -1 ;
			switch ( pevOut->codeKey )
			{
			case	joyUp:
				nVirtKey = vkeyUp ;
				break ;
			case	joyDown:
				nVirtKey = vkeyDown ;
				break ;
			case	joyLeft:
				nVirtKey = vkeyLeft ;
				break ;
			case	joyRight:
				nVirtKey = vkeyRight ;
				break ;
			}
			if ( nVirtKey >= 0 )
			{
				if ( fKeyPress )
				{
					pSprite->OnKeyDown( nVirtKey, 0 ) ;
				}
				else
				{
					pSprite->OnKeyUp( nVirtKey, 0 ) ;
				}
				return	true ;
			}
		}
	}
	return	false ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::OnMouseWheel
	( SGLSprite& sprite, int32_t zDelta,
		double xPos, double yPos, int64_t nFlags )
{
	if ( zDelta != 0 )
	{
		InputEvent	evIn ;
		evIn.typeDevice = deviceMouse ;
		if ( zDelta < 0 )
		{
			evIn.codeKey = vkeyUp ;
		}
		else
		{
			evIn.codeKey = vkeyDown ;
		}
		if ( m_pQueue != NULL )
		{
			m_pQueue->PressInputEvent( evIn ) ;
			m_pQueue->ReleaseInputEvent( evIn ) ;
		}
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::OnLButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceMouse ;
	evIn.codeKey = vkeyMouseLeft ;
	//
	if ( FilterInputEvent( evIn, true ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->PressInputEvent( evIn ) ;
	}
	return	false ;
}

bool SGLVirtualInput::InputFilter::OnLButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceMouse ;
	evIn.codeKey = vkeyMouseLeft ;
	//
	if ( FilterInputEvent( evIn, false ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->ReleaseInputEvent( evIn ) ;
	}
	return	false ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::OnRButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceMouse ;
	evIn.codeKey = vkeyMouseRight ;
	//
	if ( FilterInputEvent( evIn, true ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->PressInputEvent( evIn ) ;
	}
	return	false ;
}

bool SGLVirtualInput::InputFilter::OnRButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceMouse ;
	evIn.codeKey = vkeyMouseRight ;
	//
	if ( FilterInputEvent( evIn, false ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->ReleaseInputEvent( evIn ) ;
	}
	return	false ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::OnKeyDown
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceKeyboard ;
	evIn.codeKey = (nVirtKey & vkeyCodeMask) | (nFlags & vkeyContextMask) ;
	//
	if ( FilterInputEvent( evIn, true ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->PressInputEvent( evIn ) ;
	}
	return	false ;
}

bool SGLVirtualInput::InputFilter::OnKeyUp
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceKeyboard ;
	evIn.codeKey = (nVirtKey & vkeyCodeMask) | (nFlags & vkeyContextMask) ;
	//
	if ( FilterInputEvent( evIn, false ) )
	{
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		m_pQueue->ReleaseInputEvent( evIn ) ;
	}
	return	false ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::InputFilter::OnCommand
	( SGLSprite& sprite, const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	InputEvent	evIn ;
	evIn.typeDevice = deviceCommand ;
	evIn.strCommand = pszCmd ;
	//
	if ( FilterInputEvent( evIn, true ) )
	{
		FilterInputEvent( evIn, false ) ;
		return	true ;
	}
	if ( m_pQueue != NULL )
	{
		if ( evIn.strCommand == SysCommandId::WindowPollJoyStick )
		{
			m_pQueue->PollJoyStick() ;
			return	true ;
		}
		m_pQueue->AddCommand
			( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
		return	true ;
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// 仮想ジョイスティックボタンリスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLVirtualInput::JoyButtonListener, SGLSpriteButtonListener, ItemInteractive )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLVirtualInput::JoyButtonListener::JoyButtonListener
	( SGLVirtualInput * pInput, int64_t codeKey, int64_t numDevice )
{
	m_refInput.SetReference( pInput ) ;
	m_numDevice = numDevice ;
	m_codeKey = codeKey ;
	m_flagPushing = false ;
}

// ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::JoyButtonListener::OnButtonPushed( SGLSpriteButton& button, bool fRepeat )
{
	return	true ;
}

// ボタンのステータスが変化した
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::JoyButtonListener::OnChangedButtonStatus( SGLSpriteButton& button )
{
	SGLVirtualInput *	pInput = m_refInput ;
	InputEvent			evInput ;
	evInput.typeDevice = deviceJoyStick ;
	evInput.numDevice = m_numDevice ;
	evInput.codeKey = m_codeKey ;
	//
	if ( button.IsButtonPushing() )
	{
		if ( !m_flagPushing )
		{
			m_flagPushing = true ;
			//
			if ( pInput != NULL )
			{
				pInput->PressInputEvent( evInput ) ;
			}
		}
	}
	else
	{
		if ( m_flagPushing )
		{
			m_flagPushing = false ;
			//
			if ( pInput != NULL )
			{
				pInput->ReleaseInputEvent( evInput ) ;
			}
		}
	}
	return	true ;
}

// 通知
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::JoyButtonListener::PostMessage
	( SGLBasicForm::Item * pItem, int nParam, const wchar_t * pwszOpt )
{
	SGLBasicForm::Button *	pButton = ESLTypeCast<SGLBasicForm::Button>( pItem ) ;
	if ( pButton == NULL )
	{
		return ;
	}
	SGLVirtualInput *	pInput = m_refInput ;
	InputEvent			evInput ;
	evInput.typeDevice = deviceJoyStick ;
	evInput.numDevice = m_numDevice ;
	evInput.codeKey = m_codeKey ;
	//
	if ( pButton->IsPushing() )
	{
		if ( !m_flagPushing )
		{
			m_flagPushing = true ;
			//
			if ( pInput != NULL )
			{
				pInput->PressInputEvent( evInput ) ;
			}
		}
	}
	else
	{
		if ( m_flagPushing )
		{
			m_flagPushing = false ;
			//
			if ( pInput != NULL )
			{
				pInput->ReleaseInputEvent( evInput ) ;
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// ジョイスティック・ポーリング用タイマ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVirtualInput::PollingTimer, SGLSpriteTimer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLVirtualInput::PollingTimer::PollingTimer( void )
{
}

SGLVirtualInput::PollingTimer::PollingTimer( SGLVirtualInput * pInput )
	: m_refInput( pInput )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::PollingTimer::OnTimer
			( SGLSprite& sprite, uint32_t msecPast )
{
	SGLVirtualInput *	pInput = m_refInput ;
	if ( pInput != NULL )
	{
		pInput->PollJoyStick() ;
		return	false ;
	}
	else
	{
		return	true ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 仮想入力
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVirtualInput, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLVirtualInput::SGLVirtualInput( void )
{
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	m_filterInput.m_pQueue = this ;
	m_flagBeginJoyStick = false ;
	m_flagPendingJoyStick = false ;
	//
	m_iVRJoyStick = 0 ;
	//
	for ( int i = 0; i < joyStickMaxCount; i ++ )
	{
		m_flagJoyState[i] = false ;
	}
	m_maskJoyDevices = 0 ;
	m_timeLastPollJoy = 0 ;
	m_pPollingTimer = nullptr ;
	m_flagsBehavior = 0 ;
	m_limInputQueue = 16 ;
	//
	ResetAllJoyButtonPushed() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLVirtualInput::~SGLVirtualInput( void )
{
}

// マウス・キー入力リスナ関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::AttachPostListenerToWindow
	( SGLWindowSprite * pWindow, uint32_t maskJoyDev )
{
	pWindow->AttachMousePostListener( &m_filterInput ) ;
	pWindow->AttachKeyPostListener( &m_filterInput ) ;
	//
	m_pPollingTimer = new PollingTimer( this ) ;
	pWindow->AddSmartTimer( m_pPollingTimer ) ;
	//
	if ( !m_flagBeginJoyStick && (pWindow != NULL) )
	{
		SGLError	err = m_joyStick.BeginCapture( pWindow, maskJoyDev ) ;
		if( err == sglErrSuccess )
		{
			m_flagBeginJoyStick = true ;
			PollJoyStick() ;
		}
		else if ( err == sglErrPending )
		{
			m_flagPendingJoyStick = true ;
		}
	}
}

// マウス・キー入力リスナ解除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::DetachPostListenerToWindow( SGLWindowSprite * pWindow )
{
	pWindow->AttachMousePostListener( NULL ) ;
	pWindow->AttachKeyPostListener( NULL ) ;
	//
	if ( m_pPollingTimer != NULL )
	{
		pWindow->RemoveTimer( m_pPollingTimer ) ;
		m_pPollingTimer = NULL ;
	}
	//
	if ( m_flagBeginJoyStick )
	{
		m_joyStick.ReleaseCapture( pWindow ) ;
		m_flagBeginJoyStick = false ;
	}
}

// VR コントローラーを仮想ジョイスティックにマップ
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::AttachVRController( SGLVRViewProducer * pVR, size_t iJoyStick )
{
	m_iVRJoyStick = iJoyStick ;
	m_refVR.SetReference( pVR ) ;
}

// 動作フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLVirtualInput::GetBehaviorFlags( void ) const
{
	return	m_flagsBehavior ;
}

// 動作フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::SetBehaviorFlags( uint64_t nFlags )
{
	m_flagsBehavior = nFlags ;
}

// ジョイスティックをポーリング
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::PollJoyStick( void )
{
	if ( m_flagPendingJoyStick )
	{
		if ( m_joyStick.WaitReadyCapture(0) != sglErrSuccess )
		{
			return	sglErrFailed ;
		}
		m_flagBeginJoyStick = true ;
		m_flagPendingJoyStick = false ;
	}
	if ( !m_flagBeginJoyStick )
	{
		return	sglErrFailed ;
	}
	Lock() ;
	uint32_t	maskDevices = 0 ;
	for ( size_t i = 0; i < joyStickDevCount; i ++ )
	{
		//
		// JoyStick 入力
		//
		UI::SGLJoyStickState	stateJoy ;
		if ( m_joyStick.PollJoyStick( stateJoy, i ) == sglErrSuccess )
		{
			if ( m_flagJoyState[i] )
			{
				InputEvent	evIn ;
				evIn.typeDevice = deviceJoyStick ;
				evIn.numDevice = i ;
				//
				if ( m_flagsBehavior & behaviorStickToDPad )
				{
					if ( (stateJoy.vStickPos.x < -0.5)
						& (m_joyPhysState[i].vStickPos.x >= -0.5) )
					{
						evIn.codeKey = joyLeft ;
						PressInputEvent( evIn ) ;
					}
					else if ( (stateJoy.vStickPos.x >= -0.5)
						& (m_joyPhysState[i].vStickPos.x < -0.5) )
					{
						evIn.codeKey = joyLeft ;
						ReleaseInputEvent( evIn ) ;
					}
					if ( (stateJoy.vStickPos.x > 0.5)
						& (m_joyPhysState[i].vStickPos.x <= 0.5) )
					{
						evIn.codeKey = joyRight ;
						PressInputEvent( evIn ) ;
					}
					else if ( (stateJoy.vStickPos.x <= 0.5)
						& (m_joyPhysState[i].vStickPos.x > 0.5) )
					{
						evIn.codeKey = joyRight ;
						ReleaseInputEvent( evIn ) ;
					}
					if ( (stateJoy.vStickPos.y < -0.5)
						& (m_joyPhysState[i].vStickPos.y >= -0.5) )
					{
						evIn.codeKey = joyUp ;
						PressInputEvent( evIn ) ;
					}
					else if ( (stateJoy.vStickPos.y >= -0.5)
						& (m_joyPhysState[i].vStickPos.y < -0.5) )
					{
						evIn.codeKey = joyUp ;
						ReleaseInputEvent( evIn ) ;
					}
					if ( (stateJoy.vStickPos.y > 0.5)
						& (m_joyPhysState[i].vStickPos.y <= 0.5) )
					{
						evIn.codeKey = joyDown ;
						PressInputEvent( evIn ) ;
					}
					else if ( (stateJoy.vStickPos.y <= 0.5)
						& (m_joyPhysState[i].vStickPos.y > 0.5) )
					{
						evIn.codeKey = joyDown ;
						ReleaseInputEvent( evIn ) ;
					}
				}
				uint64_t	maskChanged =
					stateJoy.stateButtons ^ m_joyPhysState[i].stateButtons ;
				for ( size_t j = 0; (j < UI::SGLJoyStick::joyButtonCount)
										& (maskChanged != 0); j ++ )
				{
					uint64_t	maskButton = ((uint64_t) 1) << j ;
					if ( maskChanged & maskButton )
					{
						evIn.codeKey = j ;
						if ( stateJoy.stateButtons & maskButton )
						{
							PressInputEvent( evIn ) ;
						}
						else
						{
							ReleaseInputEvent( evIn ) ;
						}
					}
				}
			}
			m_flagJoyState[i] = true ;
			m_joyPhysState[i] = stateJoy ;
			maskDevices |= 1 << i ;
			//
			m_joyState[i].vStickPos = stateJoy.vStickPos ;
			for ( int j = 0; j < sizeof(stateJoy.fpTriggers)/sizeof(stateJoy.fpTriggers[0]); j ++ )
			{
				m_joyState[i].fpTriggers[j] = stateJoy.fpTriggers[j] ;
			}
		}
		else
		{
			m_flagJoyState[i] = false ;
		}
	}
	SGLVRViewProducer *	pVR = m_refVR ;
	if ( (pVR != NULL) && (m_iVRJoyStick < (size_t) joyStickMaxCount) )
	{
		UI::SGLJoyStickState&	joyState = m_joyPhysState[m_iVRJoyStick] ;
		InputEvent	evIn ;
		evIn.typeDevice = deviceJoyStick ;
		evIn.numDevice = m_iVRJoyStick ;
		//
		pVR->LockForDeviceState() ;
		//
		const SGLVRViewProducer::ControllerState&
			ctrlL = pVR->GetControllerState
						( SGLVRViewProducer::controllerLeft ) ;
		const SGLVRViewProducer::ControllerState&
			ctrlR = pVR->GetControllerState
						( SGLVRViewProducer::controllerRight ) ;
		//
		// VR コントローラー・ボタン
		//
		const uint64_t	maskVRNormalButtons = 0x000FFFFF ;
		const uint64_t	maskVRSystemButtons =
			((uint64_t) 0x07) << SGLVRViewProducer::buttonSystemFirst ;
		const uint64_t	maskVRAxisButtons =
			((uint64_t) 0x1F) << SGLVRViewProducer::buttonAxis0 ;
		uint64_t		maskVRButtons = 0 ;
		if ( ctrlL.nState & SGLVRViewProducer::controllerConnected )
		{
			maskVRButtons |=
				(ctrlL.maskButtonPressed & maskVRNormalButtons) << vrLCtrlUp ;
			maskVRButtons |=
				((ctrlL.maskButtonPressed & maskVRSystemButtons)
						>> SGLVRViewProducer::buttonSystemFirst) << vrCtrlSystem ;
			maskVRButtons |=
				((ctrlL.maskButtonPressed & maskVRAxisButtons)
						>> SGLVRViewProducer::buttonAxis0) << vrLAxis0 ;
			//
			if ( ctrlL.nCapacity & SGLVRViewProducer::capAxisThumbStick )
			{
				S2DVector	vVRStick =
					ctrlL.vAxis[SGLVRViewProducer::axisThumbStick] ;
				if ( (vVRStick.x < -0.5)
					& (joyState.vStickPos.x >= -0.5) )
				{
					evIn.codeKey = vrLCtrlLeft ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.x >= -0.5)
					& (joyState.vStickPos.x < -0.5) )
				{
					evIn.codeKey = vrLCtrlLeft ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.x > 0.5)
					& (joyState.vStickPos.x <= 0.5) )
				{
					evIn.codeKey = vrLCtrlRight ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.x <= 0.5)
					& (joyState.vStickPos.x > 0.5) )
				{
					evIn.codeKey = vrLCtrlRight ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.y < -0.5)
					& (joyState.vStickPos.y >= -0.5) )
				{
					evIn.codeKey = vrLCtrlUp ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.y >= -0.5)
						& (joyState.vStickPos.y < -0.5) )
				{
					evIn.codeKey = vrLCtrlUp ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.y > 0.5)
					& (joyState.vStickPos.y <= 0.5) )
				{
					evIn.codeKey = vrLCtrlDown ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.y <= 0.5)
						& (joyState.vStickPos.y > 0.5) )
				{
					evIn.codeKey = vrLCtrlDown ;
					ReleaseInputEvent( evIn ) ;
				}
				joyState.vStickPos.x = vVRStick.x ;
				joyState.vStickPos.y = vVRStick.y ;
			}
		}
		if ( ctrlR.nState & SGLVRViewProducer::controllerConnected )
		{
			maskVRButtons |=
				(ctrlR.maskButtonPressed & maskVRNormalButtons) << vrRCtrlUp ;
			maskVRButtons |=
				((ctrlR.maskButtonPressed & maskVRSystemButtons)
						>> SGLVRViewProducer::buttonSystemFirst) << vrCtrlSystem ;
			maskVRButtons |=
				((ctrlR.maskButtonPressed & maskVRAxisButtons)
						>> SGLVRViewProducer::buttonAxis0) << vrRAxis0 ;
			//
			if ( ctrlR.nCapacity & SGLVRViewProducer::capAxisThumbStick )
			{
				S2DVector	vVRStick =
					ctrlR.vAxis[SGLVRViewProducer::axisThumbStick] ;
				if ( (vVRStick.x < -0.5)
					& (joyState.vStickPos.z >= -0.5) )
				{
					evIn.codeKey = vrRCtrlLeft ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.x >= -0.5)
					& (joyState.vStickPos.z < -0.5) )
				{
					evIn.codeKey = vrRCtrlLeft ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.x > 0.5)
					& (joyState.vStickPos.z <= 0.5) )
				{
					evIn.codeKey = vrRCtrlRight ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.x <= 0.5)
					& (joyState.vStickPos.z > 0.5) )
				{
					evIn.codeKey = vrRCtrlRight ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.y < -0.5)
					& (joyState.vStickPos.w >= -0.5) )
				{
					evIn.codeKey = vrRCtrlUp ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.y >= -0.5)
						& (joyState.vStickPos.w < -0.5) )
				{
					evIn.codeKey = vrRCtrlUp ;
					ReleaseInputEvent( evIn ) ;
				}
				if ( (vVRStick.y > 0.5)
					& (joyState.vStickPos.w <= 0.5) )
				{
					evIn.codeKey = vrRCtrlDown ;
					PressInputEvent( evIn ) ;
				}
				else if ( (vVRStick.y <= 0.5)
						& (joyState.vStickPos.w > 0.5) )
				{
					evIn.codeKey = vrRCtrlDown ;
					ReleaseInputEvent( evIn ) ;
				}
				joyState.vStickPos.z = vVRStick.x ;
				joyState.vStickPos.w = vVRStick.y ;
			}
		}
		uint64_t	maskChanged = joyState.stateButtons ^ maskVRButtons ;
		for ( size_t i = 0; (i < UI::SGLJoyStick::joyButtonCount)
										& (maskChanged != 0); i ++ )
		{
			uint64_t	maskButton = ((uint64_t) 1) << i ;
			if ( maskChanged & maskButton )
			{
				evIn.codeKey = i ;
				if ( joyState.stateButtons & maskButton )
				{
					PressInputEvent( evIn ) ;
				}
				else
				{
					ReleaseInputEvent( evIn ) ;
				}
			}
		}
		joyState.stateButtons = maskVRButtons ;
		//
		pVR->UnlockForDeviceState() ;
		//
		m_flagJoyState[m_iVRJoyStick] = true ;
	}
	//
	m_timeLastPollJoy = CurrentMilliSec() ;
	m_maskJoyDevices = maskDevices ;
	Unlock() ;
	return	sglErrSuccess ;
}

// ジョイスティック接続状態取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLVirtualInput::GetJoyStickDeviceMask( void ) const
{
	return	m_maskJoyDevices ;
}

// アナログスティック状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::GetAnalogJoyPosition( S3DVector4& vPos, size_t joyStick )
{
	if ( joyStick < joyStickMaxCount )
	{
		if ( m_flagBeginJoyStick
			&& (CurrentMilliSec() - m_timeLastPollJoy > 5) )
		{
			PollJoyStick() ;
		}
		if ( m_flagJoyState[joyStick] )
		{
			vPos.x = m_joyState[joyStick].vStickPos.x ;
			vPos.y = m_joyState[joyStick].vStickPos.y ;
			vPos.z = m_joyState[joyStick].vStickPos.z ;
			vPos.d = m_joyState[joyStick].vStickPos.w ;
			return	true ;
		}
	}
	vPos.x = 0 ;
	vPos.y = 0 ;
	vPos.z = 0 ;
	vPos.d = 0 ;
	return	false ;
}

bool SGLVirtualInput::GetJoyStickState
	( UI::SGLJoyStickState& joyState, size_t joyStick )
{
	if ( joyStick < joyStickMaxCount )
	{
		if ( m_flagBeginJoyStick
			&& (CurrentMilliSec() - m_timeLastPollJoy > 5) )
		{
			PollJoyStick() ;
		}
		if ( m_flagJoyState[joyStick] )
		{
			joyState = m_joyState[joyStick] ;
			return	true ;
		}
	}
	joyState.vStickPos.x = 0.0f ;
	joyState.vStickPos.y = 0.0f ;
	joyState.vStickPos.z = 0.0f ;
	joyState.vStickPos.w = 0.0f ;
	joyState.stateButtons = 0 ;
	for ( int i = 0; i < UI::SGLJoyStick::triggerCount; i ++ )
	{
		joyState.fpTriggers[i] = 0.0f ;
	}
	return	false ;
}

// ボタン状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLVirtualInput::IsJoyButtonPushing
		( size_t joyButton, size_t joyStick ) const
{
	if ( (joyButton < joyButtonCount) & (joyStick < joyStickMaxCount) )
	{
		return	(m_joyStatus[joyStick][joyButton] & bmPushing) != 0 ;
	}
	return	false ;
}

// ボタン押下回数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLVirtualInput::GetJoyButtonPushed
			( size_t joyButton, size_t joyStick ) const
{
	if ( (joyButton < joyButtonCount) & (joyStick < joyStickMaxCount) )
	{
		return	m_joyStatus[joyStick][joyButton] & bmPushedMask ;
	}
	return	false ;
}

// ボタン押下回数リセット
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::ResetJoyButtonPushed( size_t joyButton, size_t joyStick )
{
	if ( (joyButton < joyButtonCount) & (joyStick < joyStickMaxCount) )
	{
		Lock() ;
		m_joyStatus[joyStick][joyButton] &= ~bmPushedMask ;
		Unlock() ;
	}
}

// 全ボタン押下回数リセット
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::ResetAllJoyButtonPushed( void )
{
	Lock() ;
	for ( size_t j = 0; j < joyStickMaxCount; j ++ )
	{
		for ( size_t i = 0; i < joyButtonCount; i ++ )
		{
			m_joyStatus[j][i] = 0 ;
		}
	}
	Unlock() ;
}

// 入力押下イベント
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::PressInputEvent( const SGLVirtualInput::InputEvent& evIn )
{
	InputEvent	evMapped = evIn ;
	Lock() ;
	const InputEvent*	pevOut = GetInputMapAs( evIn ) ;
	if ( pevOut != NULL )
	{
		evMapped = *pevOut ;
	}
	if ( evMapped.typeDevice == deviceJoyStick )
	{
		size_t	i = (size_t) evMapped.codeKey ;
		size_t	j = (size_t) evMapped.numDevice ;
		if ( (i < joyButtonCount) & (j < joyStickMaxCount) )
		{
			m_flagJoyState[j] = true ;
			m_joyStatus[j][i] =
				bmPushing | ((m_joyStatus[j][i] + 1) & bmPushedMask) ;
			m_joyState[j].stateButtons |= (uint64_t) 1 << i ;
		}
	}
	else if ( evMapped.typeDevice == deviceCommand )
	{
		AddCommand( evMapped.strCommand ) ;
	}
	if ( m_queueInput.GetLength() >= m_limInputQueue )
	{
		m_queueInput.Remove
			( 0, m_queueInput.GetLength() - m_limInputQueue + 1 ) ;
	}
	m_queueInput.Add( new InputEvent( evIn ) ) ;
	Unlock() ;
}

// 入力解放イベント
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::ReleaseInputEvent( const SGLVirtualInput::InputEvent& evIn )
{
	InputEvent	evMapped = evIn ;
	const InputEvent*	pevOut = GetInputMapAs( evIn ) ;
	if ( pevOut != NULL )
	{
		evMapped = *pevOut ;
	}
	if ( evMapped.typeDevice == deviceJoyStick )
	{
		size_t	i = (size_t) evMapped.codeKey ;
		size_t	j = (size_t) evMapped.numDevice ;
		if ( (i < joyButtonCount) & (j < joyStickMaxCount) )
		{
			Lock() ;
			m_joyStatus[j][i] &= (uint32_t) ~bmPushing ;
			m_joyState[j].stateButtons &= ~((uint64_t) 1 << i) ;
			Unlock() ;
		}
	}
}

// ソフトウェア・アナログスティック入力
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::SetSoftwareAnalogPosition
		( S3DVector4& vPos, size_t joyStick )
{
	if ( (joyStick >= joyStickUser1)
		&& (joyStick < joyStickMaxCount) )
	{
		m_flagJoyState[joyStick] = true ;
		m_joyState[joyStick].vStickPos.x = vPos.x ;
		m_joyState[joyStick].vStickPos.y = vPos.y ;
		m_joyState[joyStick].vStickPos.z = vPos.z ;
		m_joyState[joyStick].vStickPos.w = vPos.d ;
		//
		InputEvent	evIn ;
		evIn.typeDevice = deviceJoyStick ;
		evIn.numDevice = joyStick ;
		//
		if ( (vPos.x < -0.5)
			& (m_joyState[joyStick].vStickPos.x >= -0.5) )
		{
			evIn.codeKey = joyLeft ;
			PressInputEvent( evIn ) ;
		}
		else if ( (vPos.x >= -0.5)
			& (m_joyState[joyStick].vStickPos.x < -0.5) )
		{
			evIn.codeKey = joyLeft ;
			ReleaseInputEvent( evIn ) ;
		}
		if ( (vPos.x > 0.5)
			& (m_joyState[joyStick].vStickPos.x <= 0.5) )
		{
			evIn.codeKey = joyRight ;
			PressInputEvent( evIn ) ;
		}
		else if ( (vPos.x <= 0.5)
			& (m_joyState[joyStick].vStickPos.x > 0.5) )
		{
			evIn.codeKey = joyRight ;
			ReleaseInputEvent( evIn ) ;
		}
		if ( (vPos.y < -0.5)
			& (m_joyState[joyStick].vStickPos.y >= -0.5) )
		{
			evIn.codeKey = joyUp ;
			PressInputEvent( evIn ) ;
		}
		else if ( (vPos.y >= -0.5)
			& (m_joyState[joyStick].vStickPos.y < -0.5) )
		{
			evIn.codeKey = joyUp ;
			ReleaseInputEvent( evIn ) ;
		}
		if ( (vPos.y > 0.5)
			& (m_joyState[joyStick].vStickPos.y <= 0.5) )
		{
			evIn.codeKey = joyDown ;
			PressInputEvent( evIn ) ;
		}
		else if ( (vPos.y <= 0.5)
			& (m_joyState[joyStick].vStickPos.y > 0.5) )
		{
			evIn.codeKey = joyDown ;
			ReleaseInputEvent( evIn ) ;
		}
	}
}

// 入力イベントをキューから取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::GetInputEvent( SGLVirtualInput::InputEvent& ev )
{
	SGLError	err = sglErrFailed ;
	Lock() ;
	if ( m_queueInput.GetLength() > 0 )
	{
		InputEvent *	pev = m_queueInput.DetachAt( 0 ) ;
		if ( pev != NULL )
		{
			ev = *pev ;
			delete	pev ;
			err = sglErrSuccess ;
		}
	}
	Unlock() ;
	return	err ;
}

// 入力イベントキューの最大数を設定
//（溢れたものは古いものから削除される／デフォルト：16）
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::SetInputQueueLimit( size_t nLimit )
{
	if ( nLimit >= 1 )
	{
		m_limInputQueue = nLimit ;
	}
}

// 入力イベントキューを全削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::FlushInputQueue( void )
{
	Lock() ;
	m_queueInput.RemoveAll() ;
	Unlock() ;
}

// コマンドキューへ追加
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::AddCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	Lock() ;
	m_queueCommand.TrimEmpty() ;
	//
	size_t		i ;
	size_t		nCount = m_queueCommand.GetLength() ;
	Command *	pCmd ;
	if ( fOverwritable )
	{
		for ( i = 0; i < nCount; i ++ )
		{
			pCmd = m_queueCommand.GetAt( i ) ;
			if ( pCmd->fOverwritable && (pCmd->strFullID == pszCmd) )
			{
				pCmd->nParam = nParam ;
				pCmd->nCode = nCode ;
				Unlock() ;
				return ;
			}
		}
	}
	pCmd = new Command ;
	pCmd->strFullID = pszCmd ;
	pCmd->strID = pCmd->strFullID.GetFileNamePart() ;
	pCmd->nParam = nParam ;
	pCmd->nCode = nCode ;
	pCmd->nPriority = nPriority ;
	pCmd->fOverwritable = fOverwritable ;
	//
	for ( i = 0; i < nCount; i ++ )
	{
		Command *	pCmdTemp = m_queueCommand.GetAt( i ) ;
		if ( nPriority > pCmdTemp->nPriority )
		{
			break ;
		}
	}
	m_queueCommand.InsertAt( i, pCmd ) ;
	//
	Unlock() ;
}

// コマンドをキューから取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::GetCommand( SGLVirtualInput::Command& cmd )
{
	SGLError	err = sglErrFailed ;
	Lock() ;
	if ( m_queueCommand.GetLength() > 0 )
	{
		Command *	pCmd = m_queueCommand.DetachAt( 0 ) ;
		if ( pCmd != NULL )
		{
			cmd = *pCmd ;
			delete	pCmd ;
			err = sglErrSuccess ;
		}
	}
	Unlock() ;
	return	err ;
}

// 入力コマンドキューを全削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::FlushCommandQueue( void )
{
	Lock() ;
	m_queueCommand.RemoveAll() ;
	Unlock() ;
}

// 前置フィルタ追加
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::AddFilter
	( const SGLVirtualInput::InputEvent& evIn,
			const SGLVirtualInput::InputEvent& evOut )
{
	if ( evIn != evOut )
	{
		Lock() ;
		m_filterInput.m_filterEvent.SetAs( InputEvent(evIn), evOut ) ;
		Unlock() ;
	}
}

// 前置フィルタ削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::RemoveFilter( const SGLVirtualInput::InputEvent& evIn )
{
	Lock() ;
	m_filterInput.m_filterEvent.RemoveAs( InputEvent(evIn) ) ;
	Unlock() ;
}

// 前置フィルタ全削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::RemoveAllFilter( void )
{
	Lock() ;
	m_filterInput.m_filterEvent.RemoveAll() ;
	Unlock() ;
}

// 前置フィルタ
//////////////////////////////////////////////////////////////////////////////
const SGLVirtualInput::InputEvent*
		SGLVirtualInput::GetFilterAs
			( const SGLVirtualInput::InputEvent& evIn ) const
{
	const InputEvent*	pevOut ;
	Lock() ;
	pevOut = m_filterInput.GetFilterAs( evIn ) ;
	Unlock() ;
	return	pevOut ;
}

// 後置フィルタ追加
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::AddInputMap
	( const SGLVirtualInput::InputEvent& evIn,
				const SGLVirtualInput::InputEvent& evOut )
{
	if ( evIn != evOut )
	{
		Lock() ;
		m_mapEvent.SetAs( InputEvent(evIn), evOut ) ;
		Unlock() ;
	}
}

// 後置フィルタ削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::RemoveInputMap
	( const SGLVirtualInput::InputEvent& evIn )
{
	Lock() ;
	m_mapEvent.RemoveAs( InputEvent(evIn) ) ;
	Unlock() ;
}

// 後置フィルタ全削除
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::RemoveAllInputMap( void )
{
	Lock() ;
	m_mapEvent.RemoveAll() ;
	Unlock() ;
}

// 後置フィルタ
//////////////////////////////////////////////////////////////////////////////
const SGLVirtualInput::InputEvent*
		SGLVirtualInput::GetInputMapAs
			( const SGLVirtualInput::InputEvent& evIn ) const
{
	Lock() ;
	const InputEvent*	pevOut = m_mapEvent.GetAs( InputEvent(evIn) ) ;
	if ( (pevOut == NULL)
		&& (evIn.codeKey & vkeyContextMask) )
	{
		InputEvent	evInTemp = evIn ;
		evInTemp.codeKey &= vkeyCodeMask ;
		pevOut = m_mapEvent.GetAs( evInTemp ) ;
	}
	if ( pevOut != NULL )
	{
		for ( int i = 0; i < 0x100; i ++ )	// 無限ループ防止
		{
			const InputEvent*
					pevNext = m_mapEvent.GetAs( InputEvent(*pevOut) ) ;
			if ( pevNext == NULL )
			{
				break ;
			}
			pevOut = pevNext ;
		}
	}
	Unlock() ;
	return	pevOut ;
}

// 前置フィルタ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::LoadPrefilter( const wchar_t * pwszFilterFile )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilterFile, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	return	ParsePrefilter( xmlDoc ) ;
}

SGLError SGLVirtualInput::ParsePrefilter( SSystem::SXMLDocument& xmlDoc )
{
	SGLError	err ;
	Lock() ;
	m_filterInput.m_filterEvent.RemoveAll() ;
	err = ParseXMLFilter( m_filterInput.m_filterEvent, xmlDoc )  ;
	Unlock() ;
	return	err ;
}

// 後置フィルタ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::LoadPostfilter( const wchar_t * pwszFilterFile )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilterFile, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	return	ParsePostfilter( xmlDoc ) ;
}

SGLError SGLVirtualInput::ParsePostfilter( SSystem::SXMLDocument& xmlDoc )
{
	SGLError	err ;
	Lock() ;
	m_mapEvent.RemoveAll() ;
	err = ParseXMLFilter( m_mapEvent, xmlDoc )  ;
	Unlock() ;
	return	err ;
}

// フィルタ解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::ParseXMLFilter
	( SSystem::SSortArray
		< SSystem::SGenSortElement
			<SGLVirtualInput::InputEvent,
				SGLVirtualInput::InputEvent> >& mapFilter,
							SSystem::SXMLDocument& xmlDoc )
{
	SXMLDocument *	pxmlFilter = NULL ;
	if ( xmlDoc.GetType() == SXMLDocument::typeRoot )
	{
		pxmlFilter = xmlDoc.GetElementTagAs( L"filter" ) ;
		if ( pxmlFilter == NULL )
		{
			return	sglErrFailed ;
		}
	}
	else
	{
		pxmlFilter = &xmlDoc ;
	}
	size_t	nCount = pxmlFilter->GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = pxmlFilter->GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetTag() != L"key_assign") )
		{
			continue ;
		}
		SXMLDocument *
			pxmlInput = pxmlTag->GetElementTagAs( L"input" ) ;
		SXMLDocument *
			pxmlOutput = pxmlTag->GetElementTagAs( L"output" ) ;
		if ( (pxmlInput == NULL) || (pxmlOutput == NULL) )
		{
			continue ;
		}
		InputEvent	evIn, evOut ;
		if ( ParseXMLInputEvent( evIn, *pxmlInput )
			|| ParseXMLInputEvent( evOut, *pxmlOutput ) )
		{
			continue ;
		}
		if ( evIn != evOut )
		{
			mapFilter.SetAs( evIn, evOut ) ;
		}
	}
	return	sglErrSuccess ;
}

// InputEvent 解釈
//////////////////////////////////////////////////////////////////////////////
SGLError SGLVirtualInput::ParseXMLInputEvent
	( SGLVirtualInput::InputEvent& iev, SSystem::SXMLDocument& xmlTag )
{
	SString *	pstrDev = xmlTag.GetAttributeAs( L"device" ) ;
	if ( pstrDev == NULL )
	{
		return	sglErrFailed ;
	}
	if ( *pstrDev == L"keyboard" )
	{
		iev.typeDevice = deviceKeyboard ;
	}
	else if ( *pstrDev == L"mouse" )
	{
		iev.typeDevice = deviceMouse ;
	}
	else if ( *pstrDev == L"joystick" )
	{
		iev.typeDevice = deviceJoyStick ;
	}
	else if ( *pstrDev == L"command" )
	{
		iev.typeDevice = deviceCommand ;
	}
	else if ( *pstrDev == L"signal" )
	{
		iev.typeDevice = deviceSignal ;
	}
	else
	{
		return	sglErrFailed ;
	}
	iev.numDevice = xmlTag.GetAttrIntegerAs( L"device_number", 0 ) ;
	if ( (iev.typeDevice != deviceCommand)
		&& (iev.typeDevice != deviceSignal) )
	{
		SString *	pstrKey = xmlTag.GetAttributeAs( L"key" ) ;
		if ( pstrKey == NULL )
		{
			return	sglErrFailed ;
		}
		SStringParser	sparsKey ;
		sparsKey.AttachString( *pstrKey ) ;
		//
		SString	strKey ;
		sparsKey.NextToken( strKey ) ;
		if ( sparsKey.HasToComeChar( L":" ) == L':' )
		{
			if ( strKey == L"button" )
			{
				int	typeNum = sparsKey.IsNextNumber() ;
				if ( typeNum == SStringParser::numberInvalid )
				{
					return	sglErrFailed ;
				}
				iev.codeKey = joyButton1 + sparsKey.NextInteger( typeNum ) ;
				if ( (iev.codeKey < joyButton1)
					|| (iev.codeKey >= joyButton1 + 32) )
				{
					return	sglErrFailed ;
				}
			}
			else if ( strKey == L"code" )
			{
				int	typeNum = sparsKey.IsNextNumber() ;
				if ( typeNum == SStringParser::numberInvalid )
				{
					return	sglErrFailed ;
				}
				iev.codeKey = sparsKey.NextInteger( typeNum ) ;
			}
			else if ( strKey == L"ascii" )
			{
				iev.codeKey = sparsKey.GetCharacter() ;
			}
			else
			{
				return	sglErrFailed ;
			}
		}
		else
		{
			int64_t	codeVKey =
				SXMLDocument::GetIntegerAsNoCaseSymbolOf
									( g_aiVirtualKeyCode, strKey, 0 ) ;
			if ( codeVKey == 0 )
			{
				return	sglErrFailed ;
			}
			if ( iev.typeDevice == deviceJoyStick )
			{
				switch ( codeVKey )
				{
				case	vkeyLeft:
					iev.codeKey = SGLVirtualInput::joyLeft ;
					break ;
				case	vkeyUp:
					iev.codeKey = SGLVirtualInput::joyUp ;
					break ;
				case	vkeyRight:
					iev.codeKey = SGLVirtualInput::joyRight ;
					break ;
				case	vkeyDown:
					iev.codeKey = SGLVirtualInput::joyDown ;
					break ;
				default:
					iev.codeKey = codeVKey ;
					break ;
				}
			}
			else
			{
				iev.codeKey = codeVKey ;
			}
		}
	}
	else
	{
		iev.strCommand = xmlTag.GetAttrStringAs( L"command" ) ;
	}
	return	sglErrSuccess ;
}

// スレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLVirtualInput::Lock( int64_t msecTimeout ) const
{
	return	m_pMutexUI->Lock( msecTimeout ) ;
}

SSystem::SError SGLVirtualInput::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout ) const
{
#if	defined(__DEBUG__)
	return	m_pMutexUI->LockTrace( pszSource, nLineNum, msecTimeout ) ;
#else
	return	m_pMutexUI->Lock( msecTimeout ) ;
#endif
}

SSystem::SError SGLVirtualInput::Unlock( void ) const
{
	m_pMutexUI->Unlock() ;
	return	errSuccess ;
}

atomic_int_t SGLVirtualInput::UnlockAll( void ) const
{
	return	m_pMutexUI->UnlockAll() ;
}

SSystem::SError SGLVirtualInput::Relock( atomic_int_t nLock ) const
{
	return	m_pMutexUI->Relock( nLock ) ;
}

atomic_int_t SGLVirtualInput::TestLocked( void ) const
{
	return	m_pMutexUI->TestLocked() ;
}

// スレッド排他オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void SGLVirtualInput::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	ESLAssert( pMutex != NULL ) ;
	m_pMutexUI = pMutex ;
}
