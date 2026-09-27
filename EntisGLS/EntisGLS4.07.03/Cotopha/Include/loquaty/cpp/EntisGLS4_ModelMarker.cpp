
#include <loquaty.h>
#include "EntisGLS4_ModelMarker.h"

using namespace Loquaty ;


// const EntisGLS4.ModelMarker.Info* getInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelMarker_getInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelMarker, pThis ) ;

	LEntisGLS4_ModelMarker_Info	valRet ;
	// valRet = pThis->getInfo(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// String getRefBoneId( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelMarker_getRefBoneId)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelMarker, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getRefBoneId(...) ;

	LQT_RETURN_STRING( valRet ) ;
}



