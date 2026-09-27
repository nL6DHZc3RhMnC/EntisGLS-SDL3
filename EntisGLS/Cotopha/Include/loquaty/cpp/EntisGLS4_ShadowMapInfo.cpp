
#include <loquaty.h>
#include "EntisGLS4_ShadowMapInfo.h"

using namespace Loquaty ;


// boolean setShadowMappingInfo( EntisGLS4.RenderContext renderShadowMap, const EntisGLS4.LightEntry* lightShadowMap, const Vector3d* vCameraTarget, const Size* sizeDepthMap, double zScreen, double zDistance, double zScale, double zNear, double zFar, double zErrorPrec, double zErrorSubPrec, double degAngleVarX, double degAngleVarY )
IMPL_LOQUATY_FUNC(EntisGLS4_ShadowMapInfo_setShadowMappingInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_POINTER( LEntisGLS4_ShadowMapInfo, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderContext, renderShadowMap ) ;
	LQT_VERIFY_NULL_PTR( renderShadowMap ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_LightEntry, lightShadowMap ) ;
	LQT_VERIFY_NULL_PTR( lightShadowMap ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vCameraTarget ) ;
	LQT_VERIFY_NULL_PTR( vCameraTarget ) ;
	LQT_FUNC_ARG_STRUCT( LSize, sizeDepthMap ) ;
	LQT_VERIFY_NULL_PTR( sizeDepthMap ) ;
	LQT_FUNC_ARG_DOUBLE( zScreen ) ;
	LQT_FUNC_ARG_DOUBLE( zDistance ) ;
	LQT_FUNC_ARG_DOUBLE( zScale ) ;
	LQT_FUNC_ARG_DOUBLE( zNear ) ;
	LQT_FUNC_ARG_DOUBLE( zFar ) ;
	LQT_FUNC_ARG_DOUBLE( zErrorPrec ) ;
	LQT_FUNC_ARG_DOUBLE( zErrorSubPrec ) ;
	LQT_FUNC_ARG_DOUBLE( degAngleVarX ) ;
	LQT_FUNC_ARG_DOUBLE( degAngleVarY ) ;

	LBoolean	valRet ;
	// valRet = pThis->setShadowMappingInfo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void getCameraMatrix( Matrix3d* matCamera ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ShadowMapInfo_getCameraMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_POINTER( LEntisGLS4_ShadowMapInfo, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matCamera ) ;
	LQT_VERIFY_NULL_PTR( matCamera ) ;

	// pThis->getCameraMatrix(...) ;

	LQT_RETURN_VOID() ;
}



