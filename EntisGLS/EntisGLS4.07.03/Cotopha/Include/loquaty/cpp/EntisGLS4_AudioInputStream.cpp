
#include <loquaty.h>
#include "EntisGLS4_AudioInputStream.h"

using namespace Loquaty ;


// const EntisGLS4.SoundFormat* getAudioFormat( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;

	LEntisGLS4_SoundFormat	valRet ;
	// valRet = pThis->getAudioFormat(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.MediaOptionalInfo getAudioOptinalInfo( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioOptinalInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->getAudioOptinalInfo(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long getAudioLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;

	LInt64	valRet ;
	// valRet = pThis->getAudioLength(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong readAudio( void* buf, ulong samples )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_readAudio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	LQT_FUNC_ARG_POINTER( void, buf ) ;
	LQT_VERIFY_NULL_PTR( buf ) ;
	LQT_FUNC_ARG_ULONG( samples ) ;

	LUint64	valRet ;
	// valRet = pThis->readAudio(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean seekAudio( ulong samples )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_seekAudio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	LQT_FUNC_ARG_ULONG( samples ) ;

	LBoolean	valRet ;
	// valRet = pThis->seekAudio(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



