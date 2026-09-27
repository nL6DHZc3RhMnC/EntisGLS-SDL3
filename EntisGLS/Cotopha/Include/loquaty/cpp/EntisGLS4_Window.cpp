
#include <loquaty.h>
#include "EntisGLS4_Window.h"

using namespace Loquaty ;


// boolean createDisplay( String caption, EntisGLS4.Window.CooperationMode mode, uint width, uint height, uint bitsPerPixel, uint frequency )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_createDisplay)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRING( caption ) ;
	LQT_FUNC_ARG_INT( mode ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( bitsPerPixel ) ;
	LQT_FUNC_ARG_UINT( frequency ) ;

	LBoolean	valRet ;
	// valRet = pThis->createDisplay(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean closeDisplay( )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_closeDisplay)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->closeDisplay(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Window.OptionalFlag getOptionalFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getOptionalFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getOptionalFlags(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void setOptionalFlags( EntisGLS4.Window.OptionalFlag flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setOptionalFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	// pThis->setOptionalFlags(...) ;

	LQT_RETURN_VOID() ;
}

// boolean changeCooperationLevel( EntisGLS4.Window.CooperationMode mode )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_changeCooperationLevel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_INT( mode ) ;

	LBoolean	valRet ;
	// valRet = pThis->changeCooperationLevel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Window.CooperationMode getCooperationLevel( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getCooperationLevel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getCooperationLevel(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean changeDisplaySize( uint width, uint height, uint bitsPerPixel, uint frequency )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_changeDisplaySize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( bitsPerPixel ) ;
	LQT_FUNC_ARG_UINT( frequency ) ;

	LBoolean	valRet ;
	// valRet = pThis->changeDisplaySize(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getDisplaySize( Size* sizeDisplay )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getDisplaySize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LSize, sizeDisplay ) ;
	LQT_VERIFY_NULL_PTR( sizeDisplay ) ;

	LBoolean	valRet ;
	// valRet = pThis->getDisplaySize(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean enableChangePhysicalMode( boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_enableChangePhysicalMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	LBoolean	valRet ;
	// valRet = pThis->enableChangePhysicalMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean enableZBuffer( boolean flagZBuffer )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_enableZBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagZBuffer ) ;

	LBoolean	valRet ;
	// valRet = pThis->enableZBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setStereoDisplayMode( String methodID, ulong param )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setStereoDisplayMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRING( methodID ) ;
	LQT_FUNC_ARG_ULONG( param ) ;

	LBoolean	valRet ;
	// valRet = pThis->setStereoDisplayMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isSupportedStereoDisplayMode( String methodID )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isSupportedStereoDisplayMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRING( methodID ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSupportedStereoDisplayMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean initWindowPosition( int xPos, int yPos, const Size* pInitExSize )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_initWindowPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;
	LQT_FUNC_ARG_STRUCT( LSize, pInitExSize ) ;
	LQT_VERIFY_NULL_PTR( pInitExSize ) ;

	LBoolean	valRet ;
	// valRet = pThis->initWindowPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getNormalWindowPosition( Point* ptWindow, Size* pWindowSize )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getNormalWindowPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LPoint, ptWindow ) ;
	LQT_VERIFY_NULL_PTR( ptWindow ) ;
	LQT_FUNC_ARG_STRUCT( LSize, pWindowSize ) ;
	LQT_VERIFY_NULL_PTR( pWindowSize ) ;

	LBoolean	valRet ;
	// valRet = pThis->getNormalWindowPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getInternalDisplayPosition( ImageRect* rctRender, ImageRect* rctDisplay )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getInternalDisplayPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rctRender ) ;
	LQT_VERIFY_NULL_PTR( rctRender ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rctDisplay ) ;
	LQT_VERIFY_NULL_PTR( rctDisplay ) ;

	LBoolean	valRet ;
	// valRet = pThis->getInternalDisplayPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setExteriorBackgroundFrame( EntisGLS4.Window.ExteriorFrameType flags, uint rgbColor, EntisGLS4.Image pTile, EntisGLS4.Image pLeft, EntisGLS4.Image pRight, EntisGLS4.Image pUpper, EntisGLS4.Image pUnder )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setExteriorBackgroundFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_UINT( rgbColor ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pTile ) ;
	LQT_VERIFY_NULL_PTR( pTile ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pLeft ) ;
	LQT_VERIFY_NULL_PTR( pLeft ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pRight ) ;
	LQT_VERIFY_NULL_PTR( pRight ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pUpper ) ;
	LQT_VERIFY_NULL_PTR( pUpper ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pUnder ) ;
	LQT_VERIFY_NULL_PTR( pUnder ) ;

	LBoolean	valRet ;
	// valRet = pThis->setExteriorBackgroundFrame(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean postUpdate( const ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_postUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;
	LQT_VERIFY_NULL_PTR( pUpdate ) ;

	LBoolean	valRet ;
	// valRet = pThis->postUpdate(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean postUIThread( Function<void()> func )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_postUIThread)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LFunctionObj, func ) ;
	LQT_VERIFY_NULL_PTR( func ) ;

	LBoolean	valRet ;
	// valRet = pThis->postUIThread(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isWindowActive( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isWindowActive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isWindowActive(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setWindowCaption( String caption )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setWindowCaption)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRING( caption ) ;

	LBoolean	valRet ;
	// valRet = pThis->setWindowCaption(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean showCursor( boolean flagShow )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_showCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagShow ) ;

	LBoolean	valRet ;
	// valRet = pThis->showCursor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isShowCursor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_isShowCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isShowCursor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setCursor( String cursorID )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_setCursor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRING( cursorID ) ;

	LBoolean	valRet ;
	// valRet = pThis->setCursor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean moveCursorPosition( int xPos, int yPos, int idMouse )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_moveCursorPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;
	LQT_FUNC_ARG_INT( idMouse ) ;

	LBoolean	valRet ;
	// valRet = pThis->moveCursorPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getCursorPosition( Point* ptCursor, int idMouse )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getCursorPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LPoint, ptCursor ) ;
	LQT_VERIFY_NULL_PTR( ptCursor ) ;
	LQT_FUNC_ARG_INT( idMouse ) ;

	LBoolean	valRet ;
	// valRet = pThis->getCursorPosition(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// int getMonitorFrequency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getMonitorFrequency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getMonitorFrequency(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// EntisGLS4.RenderDevice getRenderDevice( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Window_getRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	// valRet = pThis->getRenderDevice(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_RenderDevice> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean lock( long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_lock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->lock(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean unlock( )
IMPL_LOQUATY_FUNC(EntisGLS4_Window_unlock)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Window, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->unlock(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



