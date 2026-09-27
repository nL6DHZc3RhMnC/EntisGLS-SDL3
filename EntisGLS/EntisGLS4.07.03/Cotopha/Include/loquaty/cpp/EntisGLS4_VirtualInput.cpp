
#include <loquaty.h>
#include "EntisGLS4_VirtualInput.h"

using namespace Loquaty ;


// VirtualInput( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_VirtualInput)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_VirtualInput, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void attachPostListenerToWindow( EntisGLS4.WindowSprite window, uint maskJoyDevs )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_attachPostListenerToWindow)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_WindowSprite, window ) ;
	LQT_VERIFY_NULL_PTR( window ) ;
	LQT_FUNC_ARG_UINT( maskJoyDevs ) ;

	// pThis->attachPostListenerToWindow(...) ;

	LQT_RETURN_VOID() ;
}

// void detachPostListenerToWindow( EntisGLS4.Window window )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_detachPostListenerToWindow)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Window, window ) ;
	LQT_VERIFY_NULL_PTR( window ) ;

	// pThis->detachPostListenerToWindow(...) ;

	LQT_RETURN_VOID() ;
}

// uint getJoyStickDeviceMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyStickDeviceMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getJoyStickDeviceMask(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// EntisGLS4.VirtualInput.BehaviorFlag getBehaviorFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getBehaviorFlags(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void setBehaviorFlags( EntisGLS4.VirtualInput.BehaviorFlag flags )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setBehaviorFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	// pThis->setBehaviorFlags(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getAnalogJoyPosition( Vector4* pos, uint iJoyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getAnalogJoyPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_UINT( iJoyStick ) ;

	LBoolean	valRet ;
	// valRet = pThis->getAnalogJoyPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getJoyStickState( EntisGLS4.VirtualInput.JoyStickState* state, uint iJoyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyStickState)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_VirtualInput_JoyStickState, state ) ;
	LQT_VERIFY_NULL_PTR( state ) ;
	LQT_FUNC_ARG_UINT( iJoyStick ) ;

	LBoolean	valRet ;
	// valRet = pThis->getJoyStickState(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isJoyButtonPushing( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_isJoyButtonPushing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	LBoolean	valRet ;
	// valRet = pThis->isJoyButtonPushing(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint getJoyButtonPushed( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	LUint32	valRet ;
	// valRet = pThis->getJoyButtonPushed(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void resetJoyButtonPushed( uint joyButton, uint joyStick ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_resetJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_UINT( joyButton ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	// pThis->resetJoyButtonPushed(...) ;

	LQT_RETURN_VOID() ;
}

// void resetAllJoyButtonPushed( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_resetAllJoyButtonPushed)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->resetAllJoyButtonPushed(...) ;

	LQT_RETURN_VOID() ;
}

// void pressInputEvent( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_pressInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	// pThis->pressInputEvent(...) ;

	LQT_RETURN_VOID() ;
}

// void releaseInputEvent( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_releaseInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	// pThis->releaseInputEvent(...) ;

	LQT_RETURN_VOID() ;
}

// void setSoftwareAnalogPosition( const Vector4* vPos, uint joyStick )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setSoftwareAnalogPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_UINT( joyStick ) ;

	// pThis->setSoftwareAnalogPosition(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getInputEvent( EntisGLS4.VirtualInput.Event ev )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getInputEvent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, ev ) ;
	LQT_VERIFY_NULL_PTR( ev ) ;

	LBoolean	valRet ;
	// valRet = pThis->getInputEvent(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setInputQueueLimit( uint limit )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_setInputQueueLimit)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_UINT( limit ) ;

	// pThis->setInputQueueLimit(...) ;

	LQT_RETURN_VOID() ;
}

// void flushInputQueue( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_flushInputQueue)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->flushInputQueue(...) ;

	LQT_RETURN_VOID() ;
}

// void addCommand( String cmd, long param, long code, int priority, boolean overwritable )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;
	LQT_FUNC_ARG_LONG( param ) ;
	LQT_FUNC_ARG_LONG( code ) ;
	LQT_FUNC_ARG_INT( priority ) ;
	LQT_FUNC_ARG_BOOL( overwritable ) ;

	// pThis->addCommand(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getCommand( EntisGLS4.VirtualInput.Command cmd )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_getCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Command, cmd ) ;
	LQT_VERIFY_NULL_PTR( cmd ) ;

	LBoolean	valRet ;
	// valRet = pThis->getCommand(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void flushCommandQueue( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_flushCommandQueue)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->flushCommandQueue(...) ;

	LQT_RETURN_VOID() ;
}

// boolean loadFilter( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_loadFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadFilter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void addFilter( const EntisGLS4.VirtualInput.Event evIn, const EntisGLS4.VirtualInput.Event evOut )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evOut ) ;
	LQT_VERIFY_NULL_PTR( evOut ) ;

	// pThis->addFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void removeFilter( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	// pThis->removeFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void removeAllFilter( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeAllFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->removeAllFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void addInputMap( const EntisGLS4.VirtualInput.Event evIn, const EntisGLS4.VirtualInput.Event evOut )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_addInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evOut ) ;
	LQT_VERIFY_NULL_PTR( evOut ) ;

	// pThis->addInputMap(...) ;

	LQT_RETURN_VOID() ;
}

// void removeInputMap( const EntisGLS4.VirtualInput.Event evIn )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VirtualInput_Event, evIn ) ;
	LQT_VERIFY_NULL_PTR( evIn ) ;

	// pThis->removeInputMap(...) ;

	LQT_RETURN_VOID() ;
}

// void removeAllInputMap( )
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_removeAllInputMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->removeAllInputMap(...) ;

	LQT_RETURN_VOID() ;
}

// boolean lock( long msecTimeout ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_lock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->lock(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void unlock( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VirtualInput_unlock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VirtualInput, pThis ) ;

	// pThis->unlock(...) ;

	LQT_RETURN_VOID() ;
}



