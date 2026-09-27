
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneLight.h>


// SceneLight( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneLight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneLight, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
					new S3DSceneComposer::LightSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneLight.LightTypeIndex getLightType( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_INT( pLight->GetLightType() ) ;
}

// void setLightType( EntisGLS4.SceneLight.LightTypeIndex type )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_INT( type ) ;

	pLight->SetLightType
		( (S3DSceneComposer::LightSerializer::LightTypeIndex) type ) ;

	LQT_RETURN_VOID() ;
}

// const ARGB8* getLightColor( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LARGB8	valRet = pLight->GetLightColor() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightColor( const ARGB8* rgbColor )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, rgbColor ) ;
	LQT_VERIFY_NULL_PTR( rgbColor ) ;

	pLight->SetLightColor( *rgbColor ) ;

	LQT_RETURN_VOID() ;
}

// double getLightBrightness( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightBrightness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_DOUBLE( pLight->GetLightBrightness() ) ;
}

// void setLightBrightness( double brightness )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightBrightness)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_DOUBLE( brightness ) ;

	pLight->SetLightBrightness( brightness ) ;

	LQT_RETURN_VOID() ;
}

// double getAttenuationPower( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getAttenuationPower)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_DOUBLE( pLight->GetAttenuationPower() ) ;
}

// void setAttenuationPower( double attenuation )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setAttenuationPower)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_DOUBLE( attenuation ) ;

	pLight->SetAttenuationPower( attenuation ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getLightPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LVector3d	valRet = pLight->GetLightPosition() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightPosition( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	pLight->SetLightPosition( *vPos ) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getLightDirection( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightDirection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LVector3d	valRet = pLight->GetLightDirection() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLightDirection( const Vector3d* vDir )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightDirection)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vDir ) ;
	LQT_VERIFY_NULL_PTR( vDir ) ;

	pLight->SetLightDirection( *vDir ) ;

	LQT_RETURN_VOID() ;
}

// double getLightAngle( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightAngle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_DOUBLE( pLight->GetLightAngle() ) ;
}

// void setLightAngle( double degAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightAngle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_DOUBLE( degAngle ) ;

	pLight->SetLightAngle( degAngle ) ;

	LQT_RETURN_VOID() ;
}

// double getLightGradation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getLightGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_DOUBLE( pLight->GetLightGradation() ) ;
}

// void setLightGradation( double degGradation )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setLightGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_DOUBLE( degGradation ) ;

	pLight->SetLightGradation( degGradation ) ;

	LQT_RETURN_VOID() ;
}

// boolean IsEnabledShadowMapping( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_IsEnabledShadowMapping)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;

	LQT_RETURN_BOOL( pLight->IsEnabledShadowMapping() ) ;
}

// void enableShadowMapping( boolean shadowmap )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_enableShadowMapping)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_BOOL( shadowmap ) ;

	pLight->EnableShadowMapping( shadowmap ) ;

	LQT_RETURN_VOID() ;
}

// void getShadowMappingParam( EntisGLS4.SceneLight.ShadowMapParam* param ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_getShadowMappingParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneLight_ShadowMapParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	pLight->GetShadowMappingParam( *param ) ;

	LQT_RETURN_VOID() ;
}

// void setShadowMappingParam( const EntisGLS4.SceneLight.ShadowMapParam* param )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneLight_setShadowMappingParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneLight, pThis ) ;
	S3DSceneComposer::LightSerializer *
		pLight = pThis->GetRef<S3DSceneComposer::LightSerializer>() ;
	LQT_VERIFY_NULL_PTR( pLight ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneLight_ShadowMapParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	pLight->SetShadowMappingParam( *param ) ;

	LQT_RETURN_VOID() ;
}



