
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Window.h>


// boolean createDisplay( String caption, int mode, uint width, uint height, uint bitsPerPixel, uint frequency )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_createDisplay)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRING( caption ) ;
	LQT_FUNC_ARG_INT( mode ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( bitsPerPixel ) ;
	LQT_FUNC_ARG_UINT( frequency ) ;

	LQT_RETURN_BOOL
		( pWindow->CreateDisplay
			( caption.c_str(),
				(SGLAbstractWindow::CooperationMode) mode,
				width, height, bitsPerPixel, frequency ) == sglErrSuccess ) ;
}

// boolean closeDisplay( )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_closeDisplay)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_BOOL( pWindow->CloseDisplay() == sglErrSuccess ) ;
}

// ulong getOptionalFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getOptionalFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_ULONG( pWindow->GetOptionalFlags() ) ;
}

// void setOptionalFlags( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setOptionalFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	pWindow->SetOptionalFlags( flags ) ;

	LQT_RETURN_VOID() ;
}

// boolean changeCooperationLevel( int mode )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_changeCooperationLevel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_INT( mode ) ;

	LQT_RETURN_BOOL
		( pWindow->ChangeCooperationLevel
			( (SGLAbstractWindow::CooperationMode) mode ) == sglErrSuccess ) ;
}

// int getCooperationLevel( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getCooperationLevel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_INT( pWindow->GetCooperationLevel() ) ;
}

// boolean changeDisplaySize( uint width, uint height, uint bitsPerPixel, uint frequency )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_changeDisplaySize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( bitsPerPixel ) ;
	LQT_FUNC_ARG_UINT( frequency ) ;

	LQT_RETURN_BOOL
		( pWindow->ChangeDisplaySize
			( width, height, bitsPerPixel, frequency ) == sglErrSuccess ) ;
}

// boolean getDisplaySize( Size* sizeDisplay )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getDisplaySize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRUCT( LSize, sizeDisplay ) ;
	LQT_VERIFY_NULL_PTR( sizeDisplay ) ;

	LQT_RETURN_BOOL( pWindow->GetDisplaySize( *sizeDisplay ) == sglErrSuccess ) ;
}

// boolean enableChangePhysicalMode( boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_enableChangePhysicalMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	LQT_RETURN_BOOL
		( pWindow->EnableChangePhysicalMode( flagEnable ) == sglErrSuccess ) ;
}

// boolean enableZBuffer( boolean flagZBuffer )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_enableZBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_BOOL( flagZBuffer ) ;

	LQT_RETURN_BOOL( pWindow->EnableZBuffer( flagZBuffer ) == sglErrSuccess ) ;
}

// boolean setStereoDisplayMode( String methodID, ulong param )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setStereoDisplayMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRING( methodID ) ;
	LQT_FUNC_ARG_ULONG( param ) ;

	LQT_RETURN_BOOL
		( pWindow->SetStereoDisplayMode
			( methodID.c_str(), param ) == sglErrSuccess ) ;
}

// boolean isSupportedStereoDisplayMode( String methodID )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isSupportedStereoDisplayMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRING( methodID ) ;

	LQT_RETURN_BOOL( pWindow->IsSupportedStereoDisplayMode( methodID.c_str() ) ) ;
}

// boolean initWindowPosition( int xPos, int yPos, const Size* pInitExSize )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_initWindowPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;
	LQT_FUNC_ARG_STRUCT( LSize, pInitExSize ) ;
	LQT_VERIFY_NULL_PTR( pInitExSize ) ;

	LQT_RETURN_BOOL
		( pWindow->InitWindowPosition
			( xPos, yPos, pInitExSize ) == sglErrSuccess ) ;
}

// boolean getNormalWindowPosition( Point* ptWindow, Size* pWindowSize )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getNormalWindowPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRUCT( LPoint, ptWindow ) ;
	LQT_VERIFY_NULL_PTR( ptWindow ) ;
	LQT_FUNC_ARG_STRUCT( LSize, pWindowSize ) ;
	LQT_VERIFY_NULL_PTR( pWindowSize ) ;

	LQT_RETURN_BOOL
		( pWindow->GetNormalWindowPosition
			( *ptWindow, pWindowSize ) == sglErrSuccess ) ;
}

// boolean getInternalDisplayPosition( ImageRect* rctRender, ImageRect* rctDisplay )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getInternalDisplayPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rctRender ) ;
	LQT_VERIFY_NULL_PTR( rctRender ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rctDisplay ) ;
	LQT_VERIFY_NULL_PTR( rctDisplay ) ;

	LQT_RETURN_BOOL
		( pWindow->GetInternalDisplayPosition
			( *rctRender, *rctDisplay ) == sglErrSuccess ) ;
}

// boolean setExteriorBackgroundFrame( uint flags, uint rgbColor, EntisGLS4.Image pTile, EntisGLS4.Image pLeft, EntisGLS4.Image pRight, EntisGLS4.Image pUpper, EntisGLS4.Image pUnder )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setExteriorBackgroundFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_UINT( rgbColor ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pTile ) ;
	SGLImageObject *	pTileImage = nullptr ;
	if ( pTile != nullptr )
	{
		pTileImage = pTile->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pLeft ) ;
	SGLImageObject *	pLeftImage = nullptr ;
	if ( pLeft != nullptr )
	{
		pLeftImage = pLeft->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pRight ) ;
	SGLImageObject *	pRightImage = nullptr ;
	if ( pRight != nullptr )
	{
		pRightImage = pRight->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pUpper ) ;
	SGLImageObject *	pUpperImage = nullptr ;
	if ( pUpper != nullptr )
	{
		pUpperImage = pUpper->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pUnder ) ;
	SGLImageObject *	pUnderImage = nullptr ;
	if ( pUnder != nullptr )
	{
		pUnderImage = pUnder->GetRef<SGLImageObject>() ;
	}

	LQT_RETURN_BOOL
		( pWindow->SetExteriorBackgroundFrame
			( flags, rgbColor, pTileImage,
				pLeftImage, pRightImage,
				pUpperImage, pUnderImage ) == sglErrSuccess ) ;
}

// boolean postUpdate( const ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_postUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;

	LQT_RETURN_BOOL( pWindow->PostUpdate( pUpdate ) == sglErrSuccess ) ;
}

// boolean postUIThread( Function<void()> func )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_postUIThread)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, func ) ;
	LQT_VERIFY_NULL_PTR( func ) ;

	LPtr<LTaskObj>	pTask( new LTaskObj( _context.VM().GetTaskClass() ) ) ;

	LQT_RETURN_BOOL
		( pWindow->PostUIThread
			( new LProcedure( pTask, func ) ) == sglErrSuccess ) ;
}

// boolean isWindowActive( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isWindowActive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_BOOL( pWindow->IsWindowActive() ) ;
}

// boolean setWindowCaption( String caption )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setWindowCaption)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRING( caption ) ;

	LQT_RETURN_BOOL( pWindow->SetWindowCaption( caption.c_str() ) == sglErrSuccess ) ;
}

// boolean showCursor( boolean flagShow )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_showCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_BOOL( flagShow ) ;

	LQT_RETURN_BOOL( pWindow->ShowCursor( flagShow ) == sglErrSuccess ) ;
}

// boolean isShowCursor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isShowCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_BOOL( pWindow->IsShowCursor() ) ;
}

// boolean setCursor( String cursorID )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRING( cursorID ) ;

	LQT_RETURN_BOOL( pWindow->SetCursor( cursorID.c_str() ) == sglErrSuccess ) ;
}

// boolean moveCursorPosition( int xPos, int yPos, int idMouse )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_moveCursorPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;
	LQT_FUNC_ARG_INT( idMouse ) ;

	LQT_RETURN_BOOL
		( pWindow->MoveCursorPosition( xPos, yPos, idMouse ) == sglErrSuccess ) ;
}

// boolean getCursorPosition( Point* ptCursor, int idMouse )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getCursorPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_STRUCT( LPoint, ptCursor ) ;
	LQT_VERIFY_NULL_PTR( ptCursor ) ;
	LQT_FUNC_ARG_INT( idMouse ) ;

	LQT_RETURN_BOOL
		( pWindow->GetCursorPosition( *ptCursor, idMouse ) == sglErrSuccess ) ;
}

// int getMonitorFrequency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getMonitorFrequency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_INT( pWindow->GetMonitorFrequency() ) ;
}

// EntisGLS4.RenderDevice getRenderDevice( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	S3DRenderDevice *	pDevice = pWindow->GetRenderDevice() ;
	if ( pDevice == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>(pDevice) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean lock( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_lock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	std::shared_ptr<LTimeoutRemaining>
		remaining = std::make_shared<LTimeoutRemaining>( msecTimeout ) ;
	_context.SetAwaiting
		( [&_context,pWindow,remaining]( std::int64_t msecTimeout )
		{
			if ( remaining->IsFinished() )
			{
				return	true ;
			}
			if ( pWindow->Lock( remaining->Remaining( msecTimeout ) ) == errSuccess )
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

// boolean unlock( )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_unlock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	SGLAbstractWindow *	pWindow = pThis->GetRef<SGLAbstractWindow>() ;
	LQT_VERIFY_NULL_PTR( pWindow ) ;

	LQT_RETURN_BOOL( pWindow->Unlock() == errSuccess ) ;
}



