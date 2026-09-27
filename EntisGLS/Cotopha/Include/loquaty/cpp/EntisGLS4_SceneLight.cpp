
#include <loquaty.h>
#include "EntisGLS4_SceneLight.h"

using namespace Loquaty ;


// SceneLight( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneLight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneLight, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneLight.LightTypeIndex getLightType( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getLightType(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setLightType( EntisGLS4.SceneLight.LightTypeIndex type )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_INT( type ) ;

	// pThis->setLightType(...) ;

	LQT_RETURN_VOID() ;
}

// const ARGB8* getLightColor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LARGB8	valRet ;
	// valRet = pThis->getLightColor(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightColor( const ARGB8* rgbColor )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbColor ) ;
	LQT_VERIFY_NULL_PTR( rgbColor ) ;

	// pThis->setLightColor(...) ;

	LQT_RETURN_VOID() ;
}

// double getLightBrightness( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightBrightness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getLightBrightness(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setLightBrightness( double brightness )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightBrightness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( brightness ) ;

	// pThis->setLightBrightness(...) ;

	LQT_RETURN_VOID() ;
}

// double getAttenuationPower( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getAttenuationPower)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getAttenuationPower(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setAttenuationPower( double attenuation )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setAttenuationPower)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( attenuation ) ;

	// pThis->setAttenuationPower(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getLightPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getLightPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightPosition( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	// pThis->setLightPosition(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getLightDirection( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightDirection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getLightDirection(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightDirection( const Vector3d* vDir )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightDirection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vDir ) ;
	LQT_VERIFY_NULL_PTR( vDir ) ;

	// pThis->setLightDirection(...) ;

	LQT_RETURN_VOID() ;
}

// double getLightAngle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightAngle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getLightAngle(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setLightAngle( double degAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightAngle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( degAngle ) ;

	// pThis->setLightAngle(...) ;

	LQT_RETURN_VOID() ;
}

// double getLightGradation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getLightGradation(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setLightGradation( double degGradation )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( degGradation ) ;

	// pThis->setLightGradation(...) ;

	LQT_RETURN_VOID() ;
}

// boolean IsEnabledShadowMapping( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_IsEnabledShadowMapping)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->IsEnabledShadowMapping(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void enableShadowMapping( boolean shadowmap )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_enableShadowMapping)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_BOOL( shadowmap ) ;

	// pThis->enableShadowMapping(...) ;

	LQT_RETURN_VOID() ;
}

// void getShadowMappingParam( EntisGLS4.SceneLight.ShadowMapParam* param ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getShadowMappingParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneLight_ShadowMapParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	// pThis->getShadowMappingParam(...) ;

	LQT_RETURN_VOID() ;
}

// void setShadowMappingParam( const EntisGLS4.SceneLight.ShadowMapParam* param )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setShadowMappingParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneLight_ShadowMapParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	// pThis->setShadowMappingParam(...) ;

	LQT_RETURN_VOID() ;
}



