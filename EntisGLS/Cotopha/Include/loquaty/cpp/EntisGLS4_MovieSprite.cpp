
#include <loquaty.h>
#include "EntisGLS4_MovieSprite.h"

using namespace Loquaty ;


// MovieSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MovieSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_MovieSprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean openMovieFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_openMovieFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->openMovieFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean closeMovieFile( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_closeMovieFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->closeMovieFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean playMovie( EntisGLS4.MovieSprite.PlayFlag nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_playMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->playMovie(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean stopMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_stopMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->stopMovie(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMovieLoop( boolean flagLoop, long nStart, long nEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_setMovieLoop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagLoop ) ;
	LQT_FUNC_ARG_LONG( nStart ) ;
	LQT_FUNC_ARG_LONG( nEnd ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMovieLoop(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isMovieLoop( long* pStart, long* pEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMovieLoop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pStart ) ;
	LQT_VERIFY_NULL_PTR( pStart ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pEnd ) ;
	LQT_VERIFY_NULL_PTR( pEnd ) ;

	LBoolean	valRet ;
	// valRet = pThis->isMovieLoop(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean pauseMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_pauseMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->pauseMovie(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean restartMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_restartMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->restartMovie(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getVolume( float* pVolumes, ulong nChannels )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( nChannels ) ;

	LBoolean	valRet ;
	// valRet = pThis->getVolume(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setVolume( const float* pVolumes, ulong nChannels )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_setVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( nChannels ) ;

	LBoolean	valRet ;
	// valRet = pThis->setVolume(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isMoviePlaying( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMoviePlaying)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isMoviePlaying(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isMoviePaused( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMoviePaused)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isMoviePaused(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint getMovieFrequency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieFrequency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getMovieFrequency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// ulong getMovieLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMovieLength(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getMoviePosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMoviePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMoviePosition(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void seekMovie( ulong nPos )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_seekMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nPos ) ;

	// pThis->seekMovie(...) ;

	LQT_RETURN_VOID() ;
}

// const Size* getMovieSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LSize	valRet ;
	// valRet = pThis->getMovieSize(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean hasEndedOfDuration( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_hasEndedOfDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->hasEndedOfDuration(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void resetEndOfDuration( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_resetEndOfDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;

	// pThis->resetEndOfDuration(...) ;

	LQT_RETURN_VOID() ;
}



