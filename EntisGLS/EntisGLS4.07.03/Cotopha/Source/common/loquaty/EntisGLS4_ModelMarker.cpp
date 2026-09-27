
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ModelMarker.h>


// const EntisGLS4.ModelMarker.Info* getInfo( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelMarker_getInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelMarker, pThis ) ;
	S3DModelData::MarkerInfo *	pMarker = pThis->GetRef<S3DModelData::MarkerInfo>() ;
	LQT_VERIFY_NULL_PTR( pMarker ) ;

	LEntisGLS4_ModelMarker_Info	valRet ;
	valRet.FromMarkerInfo( *pMarker ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// String getRefBoneId( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelMarker_getRefBoneId)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelMarker, pThis ) ;
	S3DModelData::MarkerInfo *	pMarker = pThis->GetRef<S3DModelData::MarkerInfo>() ;
	LQT_VERIFY_NULL_PTR( pMarker ) ;

	LQT_RETURN_STRING( pMarker->m_strRefBone ) ;
}



