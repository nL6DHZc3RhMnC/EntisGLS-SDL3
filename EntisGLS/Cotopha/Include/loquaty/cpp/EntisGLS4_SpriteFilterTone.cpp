
#include <loquaty.h>
#include "EntisGLS4_SpriteFilterTone.h"

using namespace Loquaty ;


// SpriteFilterTone( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SpriteFilterTone, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// void setToneFilter( const uint8* pRed, const uint8* pGreen, const uint8* pBlue, const uint8* pAlpha, uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterTone_setToneFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterTone, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pRed ) ;
	LQT_VERIFY_NULL_PTR( pRed ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pGreen ) ;
	LQT_VERIFY_NULL_PTR( pGreen ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pBlue ) ;
	LQT_VERIFY_NULL_PTR( pBlue ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pAlpha ) ;
	LQT_VERIFY_NULL_PTR( pAlpha ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	// pThis->setToneFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void enableMorphing( boolean flagMorph )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterTone_enableMorphing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterTone, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagMorph ) ;

	// pThis->enableMorphing(...) ;

	LQT_RETURN_VOID() ;
}



