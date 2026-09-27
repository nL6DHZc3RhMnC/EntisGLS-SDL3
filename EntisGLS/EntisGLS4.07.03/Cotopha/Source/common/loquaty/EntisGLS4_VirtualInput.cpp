
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_VirtualInput.h>


// VirtualInput( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_VirtualInput)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_VirtualInput, pThis,
			( new SSmartObject( new SGLVirtualInput ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void attachPostListenerToWindow( EntisGLS4.WindowSprite window, uint maskJoyDevs )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_attachPostListenerToWindow)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_WindowSprite, window ) ;
	LQT_VERIFY_NULL_PTR( window ) ;
	SGLWindowSprite *	pWindow = window->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_UINT( maskJoyDevs ) ;

	pInput->AttachPostListenerToWindow( pWindow, maskJoyDevs ) ;

	LQT_RETURN_VOID() ;
}

// void detachPostListenerToWindow( EntisGLS4.Window window )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_detachPostListenerToWindow)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Window, window ) ;
	LQT_VERIFY_NULL_PTR( window ) ;
	SGLWindowSprite *	pWindow = window->GetRef<SGLWindowSprite>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	pInput->DetachPostListenerToWindow( pWindow ) ;

	LQT_RETURN_VOID() ;
}

// uint getJoyStickDeviceMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyStickDeviceMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	LQT_RETURN_UINT( pInput->GetJoyStickDeviceMask() ) ;
}

// EntisGLS4.VirtualInput.BehaviorFlag getBehaviorFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	LQT_RETURN_ULONG( pInput->GetBehaviorFlags() ) ;
}

// void setBehaviorFlags( EntisGLS4.VirtualInput.BehaviorFlag flags )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	pInput->SetBehaviorFlags( flags ) ;

	LQT_RETURN_VOID() ;
}

// boolean getAnalogJoyPosition( Vector4* pos, uint iJoyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getAnalogJoyPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_UINT( iJoyStick ) ;

	LQT_RETURN_BOOL( pInput->GetAnalogJoyPosition( *pos, (size_t) iJoyStick ) ) ;
}

// boolean getJoyStickState( EntisGLS4.VirtualInput.JoyStickState* state, uint iJoyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyStickState)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_VirtualInput_JoyStickState, state ) ;
	LQT_VERIFY_NULL_PTR( state ) ;
	LQT_FUNC_ARG_UINT( iJoyStick ) ;

	LQT_RETURN_BOOL( pInput->GetJoyStickState( *state, (size_t) iJoyStick ) ) ;
}

// boolean isJoyButtonPushing( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_isJoyButtonPushing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	LQT_RETURN_BOOL
		( pInput->IsJoyButtonPushing( (size_t) joyButton, (size_t) joyStick ) ) ;
}

// uint getJoyButtonPushed( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	LQT_RETURN_UINT
		( (LUint) pInput->GetJoyButtonPushed
					( (size_t) joyButton, (size_t) joyStick ) ) ;
}

// void resetJoyButtonPushed( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_resetJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	pInput->ResetJoyButtonPushed( (size_t) joyButton, (size_t) joyStick ) ;

	LQT_RETURN_VOID() ;
}

// void resetAllJoyButtonPushed( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_resetAllJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->ResetAllJoyButtonPushed() ;

	LQT_RETURN_VOID() ;
}

// void pressInputEvent( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_pressInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	SGLVirtualInput::InputEvent	iev ;
	GetLInputEvent( iev, evIn ) ;

	pInput->PressInputEvent( iev ) ;

	LQT_RETURN_VOID() ;
}

// void releaseInputEvent( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_releaseInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	SGLVirtualInput::InputEvent	iev ;
	GetLInputEvent( iev, evIn ) ;

	pInput->ReleaseInputEvent( iev ) ;

	LQT_RETURN_VOID() ;
}

// void setSoftwareAnalogPosition( const Vector4* vPos, uint joyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setSoftwareAnalogPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	pInput->SetSoftwareAnalogPosition( *vPos, (size_t) joyStick ) ;

	LQT_RETURN_VOID() ;
}

// boolean getInputEvent( EntisGLS4.VirtualInput.Event ev )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, ev ) ;
	LQT_VERIFY_NULL_PTR( ev ) ;

	SGLVirtualInput::InputEvent	iev ;
	LBoolean	valRet = (pInput->GetInputEvent( iev ) == sglErrSuccess) ;
	if ( valRet )
	{
		ev->SetElementLongAs( L"type", iev.typeDevice ) ;
		ev->SetElementLongAs( L"device", iev.numDevice ) ;
		ev->SetElementLongAs( L"key", iev.codeKey ) ;
		ev->SetElementStringAs( L"command", iev.strCommand ) ;
	}

	LQT_RETURN_BOOL( valRet ) ;
}

// void setInputQueueLimit( uint limit )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setInputQueueLimit)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_UINT( limit ) ;

	pInput->SetInputQueueLimit( (size_t) limit ) ;

	LQT_RETURN_VOID() ;
}

// void flushInputQueue( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_flushInputQueue)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->FlushInputQueue() ;

	LQT_RETURN_VOID() ;
}

// void addCommand( String cmd, long param, long code, int priority, boolean overwritable )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;
	LQT_FUNC_ARG_LONG( param ) ;
	LQT_FUNC_ARG_LONG( code ) ;
	LQT_FUNC_ARG_INT( priority ) ;
	LQT_FUNC_ARG_BOOL( overwritable ) ;

	pInput->AddCommand( cmd.c_str(), param, code, priority, overwritable ) ;

	LQT_RETURN_VOID() ;
}

// boolean getCommand( EntisGLS4.VirtualInput.Command cmd )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Command, cmdObj ) ;
	LQT_VERIFY_NULL_PTR( cmdObj ) ;

	SGLVirtualInput::Command	cmd ;
	LBoolean	valRet = (pInput->GetCommand( cmd ) == sglErrSuccess) ;
	if ( valRet )
	{
		cmdObj->SetElementStringAs( L"fullId", cmd.strFullID ) ;
		cmdObj->SetElementStringAs( L"id", cmd.strID ) ;
		cmdObj->SetElementLongAs( L"param", cmd.nParam ) ;
		cmdObj->SetElementLongAs( L"code", cmd.nCode ) ;
		cmdObj->SetElementLongAs( L"priority", cmd.nPriority ) ;
		cmdObj->SetElementLongAs( L"overwritable", cmd.fOverwritable ) ;
	}

	LQT_RETURN_BOOL( valRet ) ;
}

// void flushCommandQueue( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_flushCommandQueue)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->FlushCommandQueue() ;

	LQT_RETURN_VOID() ;
}

// boolean loadFilter( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_loadFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pInput->LoadPrefilter( file.c_str() ) == sglErrSuccess ) ;
}

// void addFilter( const EntisGLS4.VirtualInput.Event evIn, const EntisGLS4.VirtualInput.Event evOut )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evInObj ) ;
	LQT_VERIFY_NULL_PTR( evInObj ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evOutObj ) ;
	LQT_VERIFY_NULL_PTR( evOutObj ) ;

	SGLVirtualInput::InputEvent	ievIn, ievOut ;
	GetLInputEvent( ievIn, evInObj ) ;
	GetLInputEvent( ievOut, evOutObj ) ;

	pInput->AddFilter( ievIn, ievOut ) ;

	LQT_RETURN_VOID() ;
}

// void removeFilter( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evInObj ) ;
	LQT_VERIFY_NULL_PTR( evInObj ) ;

	SGLVirtualInput::InputEvent	ievIn ;
	GetLInputEvent( ievIn, evInObj ) ;

	pInput->RemoveFilter( ievIn ) ;

	LQT_RETURN_VOID() ;
}

// void removeAllFilter( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeAllFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->RemoveAllFilter() ;

	LQT_RETURN_VOID() ;
}

// void addInputMap( const EntisGLS4.VirtualInput.Event evIn, const EntisGLS4.VirtualInput.Event evOut )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evInObj ) ;
	LQT_VERIFY_NULL_PTR( evInObj ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evOutObj ) ;
	LQT_VERIFY_NULL_PTR( evOutObj ) ;

	SGLVirtualInput::InputEvent	ievIn, ievOut ;
	GetLInputEvent( ievIn, evInObj ) ;
	GetLInputEvent( ievOut, evOutObj ) ;

	pInput->AddInputMap( ievIn, ievOut ) ;

	LQT_RETURN_VOID() ;
}

// void removeInputMap( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evInObj ) ;
	LQT_VERIFY_NULL_PTR( evInObj ) ;

	SGLVirtualInput::InputEvent	ievIn ;
	GetLInputEvent( ievIn, evInObj ) ;

	pInput->RemoveInputMap( ievIn ) ;

	LQT_RETURN_VOID() ;
}

// void removeAllInputMap( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeAllInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->RemoveAllInputMap() ;

	LQT_RETURN_VOID() ;
}

// boolean lock( long msecTimeout ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_lock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	std::shared_ptr<LTimeoutRemaining>
		remaining = std::make_shared<LTimeoutRemaining>( msecTimeout ) ;
	_context.SetAwaiting
		( [&_context,pInput,remaining]( std::int64_t msecTimeout )
		{
			if ( remaining->IsFinished() )
			{
				return	true ;
			}
			if ( pInput->Lock( remaining->Remaining( msecTimeout ) ) == errSuccess )
			{
				remaining->Finish() ;
				_context.SetReturnValue
					( LValue(LType::typeBoolean, LValue::MakeBool(true) ) ) ;
				return	true ;
			}
			return	remaining->IsTimeout() ;
		} ) ;

	LQT_RETURN_BOOL( false ) ;
}

// void unlock( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_unlock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	SGLVirtualInput *	pInput = pThis->GetRef<SGLVirtualInput>() ;
	LQT_VERIFY_NULL_PTR( pInput ) ;

	pInput->Unlock() ;

	LQT_RETURN_VOID() ;
}



