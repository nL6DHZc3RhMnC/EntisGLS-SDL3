
#include <loquaty.h>
#include "EntisGLS4_AudioPlayer.h"

using namespace Loquaty ;


// double getLineVolume( uint line )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getLineVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_UINT( line ) ;

	LDouble	valRet ;
	// valRet = getLineVolume(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setLineVolume( uint line, double volume )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_setLineVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_UINT( line ) ;
	LQT_FUNC_ARG_DOUBLE( volume ) ;

	// setLineVolume(...) ;

	LQT_RETURN_VOID() ;
}

// AudioPlayer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_AudioPlayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_AudioPlayer, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean open( String file, EntisGLS4.AudioPlayer.OpenMode flags )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_open)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->open(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean create( File file, EntisGLS4.AudioPlayer.OpenMode flags )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_create)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->create(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.AudioPlayer clonePlayer( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_clonePlayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.AudioPlayer) ) ) ;
	// valRet = pThis->clonePlayer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_AudioPlayer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_AudioPlayer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean close( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_close)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->close(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean play( ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_play)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->play(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean stop( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_stop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->stop(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setLoop( boolean loop, long nStart, long nEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_setLoop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_BOOL( loop ) ;
	LQT_FUNC_ARG_LONG( nStart ) ;
	LQT_FUNC_ARG_LONG( nEnd ) ;

	LBoolean	valRet ;
	// valRet = pThis->setLoop(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean pause( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_pause)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->pause(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean restart( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_restart)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->restart(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getVolume( float* pVolumes, ulong channels )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( channels ) ;

	LBoolean	valRet ;
	// valRet = pThis->getVolume(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setVolume( const float* pVolumes, ulong channels )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_setVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( channels ) ;

	LBoolean	valRet ;
	// valRet = pThis->setVolume(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isPlaying( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_isPlaying)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isPlaying(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isPaused( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_isPaused)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isPaused(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint getSampleFrequency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getSampleFrequency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getSampleFrequency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// ulong getTotalLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getTotalLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getTotalLength(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getPosition( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getPosition(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void seekPosition( ulong pos )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_seekPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_ULONG( pos ) ;

	// pThis->seekPosition(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.AudioInputStream getAudioStream( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getAudioStream)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.AudioInputStream) ) ) ;
	// valRet = pThis->getAudioStream(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_AudioInputStream> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_AudioInputStream>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void releaseAudioStream( EntisGLS4.AudioInputStream stream )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_releaseAudioStream)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_AudioInputStream, stream ) ;
	LQT_VERIFY_NULL_PTR( stream ) ;

	// pThis->releaseAudioStream(...) ;

	LQT_RETURN_VOID() ;
}

// uint getVolumeLineMask( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_getVolumeLineMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getVolumeLineMask(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void clearAllVolumeLines( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_clearAllVolumeLines)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	// pThis->clearAllVolumeLines(...) ;

	LQT_RETURN_VOID() ;
}

// void setVolumeLineMask( uint maskLines )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_setVolumeLineMask)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_UINT( maskLines ) ;

	// pThis->setVolumeLineMask(...) ;

	LQT_RETURN_VOID() ;
}

// void setVolumeLine( uint line )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_setVolumeLine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_UINT( line ) ;

	// pThis->setVolumeLine(...) ;

	LQT_RETURN_VOID() ;
}

// void resetVolumeLine( uint line )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_resetVolumeLine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_UINT( line ) ;

	// pThis->resetVolumeLine(...) ;

	LQT_RETURN_VOID() ;
}

// void beginFadeVolume( const float* pVolumes, ulong channels, ulong msecDuration )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_beginFadeVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( channels ) ;
	LQT_FUNC_ARG_ULONG( msecDuration ) ;

	// pThis->beginFadeVolume(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isVolumeFading( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_isVolumeFading)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isVolumeFading(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void cancelFadeVolume( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_cancelFadeVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	// pThis->cancelFadeVolume(...) ;

	LQT_RETURN_VOID() ;
}

// void flushFadeVolume( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioPlayer_flushFadeVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioPlayer, pThis ) ;

	// pThis->flushFadeVolume(...) ;

	LQT_RETURN_VOID() ;
}



