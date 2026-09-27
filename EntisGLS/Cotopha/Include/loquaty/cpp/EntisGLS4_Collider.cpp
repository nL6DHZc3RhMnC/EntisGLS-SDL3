
#include <loquaty.h>
#include "EntisGLS4_Collider.h"

using namespace Loquaty ;


// boolean isHitAgainstSphere( const Vector3d* vPos, float radius, EntisGLS4.Collider.Result[] hitRes, uint resMax, ulong maskColliders, ulong maskClasses ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collider_isHitAgainstSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collider, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_FLOAT( radius ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, hitRes ) ;
	LQT_VERIFY_NULL_PTR( hitRes ) ;
	LQT_FUNC_ARG_UINT( resMax ) ;
	LQT_FUNC_ARG_ULONG( maskColliders ) ;
	LQT_FUNC_ARG_ULONG( maskClasses ) ;

	LBoolean	valRet ;
	// valRet = pThis->isHitAgainstSphere(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isSegmentCrossing( const Vector3d* vPos0, const Vector3d* vPos1, float errorGap, EntisGLS4.Collider.Result hitRes, ulong maskColliders, ulong maskClasses ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Collider_isSegmentCrossing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Collider, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos0 ) ;
	LQT_VERIFY_NULL_PTR( vPos0 ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos1 ) ;
	LQT_VERIFY_NULL_PTR( vPos1 ) ;
	LQT_FUNC_ARG_FLOAT( errorGap ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_Collider_Result, hitRes ) ;
	LQT_VERIFY_NULL_PTR( hitRes ) ;
	LQT_FUNC_ARG_ULONG( maskColliders ) ;
	LQT_FUNC_ARG_ULONG( maskClasses ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSegmentCrossing(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



