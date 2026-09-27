
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <loquaty/EntisGLS4_SpriteFilterTone.h>


// SpriteFilterTone( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteFilterTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteFilterTone, pThis,
			( new SSmartObject( new SGLSpriteFilterTone ) ) ) ;

	LQT_RETURN_VOID() ;
}

// void setToneFilter( const uint8* pRed, const uint8* pGreen, const uint8* pBlue, const uint8* pAlpha, uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterTone_setToneFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterTone, pThis ) ;
	SGLSpriteFilterTone *	pFilter = pThis->GetRef<SGLSpriteFilterTone>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pRed, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pRed ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pGreen, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pGreen ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pBlue, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pBlue ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pAlpha, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pAlpha ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	pFilter->SetToneFilter( pRed, pGreen, pBlue, pAlpha, nFlags ) ;

	LQT_RETURN_VOID() ;
}

// void enableMorphing( boolean flagMorph )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteFilterTone_enableMorphing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilterTone, pThis ) ;
	SGLSpriteFilterTone *	pFilter = pThis->GetRef<SGLSpriteFilterTone>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;
	LQT_FUNC_ARG_BOOL( flagMorph ) ;

	pFilter->EnableMorphing( flagMorph ) ;

	LQT_RETURN_VOID() ;
}



