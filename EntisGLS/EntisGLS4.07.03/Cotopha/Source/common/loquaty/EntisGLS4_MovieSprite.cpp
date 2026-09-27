
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_MovieSprite.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>


// MovieSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MovieSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_MovieSprite, pThis,
			( new SSmartObject( new SGLSpriteMovie ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean openMovieFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_openMovieFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LQT_RETURN_BOOL( pSprite->OpenMovieFile( path.c_str() ) == sglErrSuccess ) ;
}

// boolean closeMovieFile( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_closeMovieFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->CloseMovieFile() == sglErrSuccess ) ;
}

// boolean playMovie( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_playMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LQT_RETURN_BOOL( pSprite->PlayMovie( nFlags ) == sglErrSuccess ) ;
}

// boolean stopMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_stopMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->StopMovie() == sglErrSuccess ) ;
}

// boolean setMovieLoop( boolean flagLoop, long nStart, long nEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_setMovieLoop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_BOOL( flagLoop ) ;
	LQT_FUNC_ARG_LONG( nStart ) ;
	LQT_FUNC_ARG_LONG( nEnd ) ;

	LQT_RETURN_BOOL
		( pSprite->SetMovieLoop( flagLoop, nStart, nEnd ) == sglErrSuccess ) ;
}

// boolean isMovieLoop( long* pStart, long* pEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMovieLoop)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pStart ) ;
	LQT_FUNC_ARG_POINTER( LInt64, pEnd ) ;

	LQT_RETURN_BOOL( pSprite->IsMovieLoop( pStart, pEnd ) ) ;
}

// boolean pauseMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_pauseMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->PauseMovie() == sglErrSuccess ) ;
}

// boolean restartMovie( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_restartMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->RestartMovie() == sglErrSuccess ) ;
}

// boolean getVolume( float* pVolumes, ulong nChannels )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( nChannels ) ;

	LQT_RETURN_BOOL
		( pSprite->GetVolume( pVolumes, (size_t) nChannels ) == sglErrSuccess ) ;
}

// boolean setVolume( const float* pVolumes, ulong nChannels )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_setVolume)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pVolumes ) ;
	LQT_VERIFY_NULL_PTR( pVolumes ) ;
	LQT_FUNC_ARG_ULONG( nChannels ) ;

	LQT_RETURN_BOOL
		( pSprite->SetVolume( pVolumes, (size_t) nChannels ) == sglErrSuccess ) ;
}

// boolean isMoviePlaying( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMoviePlaying)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsMoviePlaying() ) ;
}

// boolean isMoviePaused( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_isMoviePaused)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsMoviePaused() ) ;
}

// uint getMovieFrequency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieFrequency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_UINT( pSprite->GetMovieFrequency() ) ;
}

// ulong getMovieLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetMovieLength() ) ;
}

// ulong getMoviePosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMoviePosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetMoviePosition() ) ;
}

// void seekMovie( ulong nPos )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_seekMovie)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nPos ) ;

	pSprite->SeekMovie( nPos ) ;

	LQT_RETURN_VOID() ;
}

// const Size* getMovieSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_getMovieSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LSize	valRet = pSprite->GetMovieSize() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean hasEndedOfDuration( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_hasEndedOfDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->HasEndedOfDuration() ) ;
}

// void resetEndOfDuration( )
IMPL_LOQUATY_FUNC(EntisGLS4_MovieSprite_resetEndOfDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MovieSprite, pThis ) ;
	SGLSpriteMovie *	pSprite = pThis->GetRef<SGLSpriteMovie>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->ResetEndOfDuration() ;

	LQT_RETURN_VOID() ;
}



