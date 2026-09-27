
#include <loquaty.h>
#include "EntisGLS4_ModelBuffer.h"

using namespace Loquaty ;


// ModelBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ModelBuffer, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadModel( String file, String mime )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_loadModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadModel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean readModel( File file, String mime )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_readModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;

	LBoolean	valRet ;
	// valRet = pThis->readModel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean saveModel( String file, String mime, String imageMIME )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_saveModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;
	LQT_FUNC_ARG_STRING( imageMIME ) ;

	LBoolean	valRet ;
	// valRet = pThis->saveModel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean writeModel( File file, String mime, String imageMIME )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_writeModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;
	LQT_FUNC_ARG_STRING( imageMIME ) ;

	LBoolean	valRet ;
	// valRet = pThis->writeModel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.TextureLibrary getTextureLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getTextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	// valRet = pThis->getTextureLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_TextureLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.MaterialLibrary getMaterialLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	// valRet = pThis->getMaterialLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_MaterialLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getPoseLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	// valRet = pThis->getPoseLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPoseLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelBone getBoneAs( String boneId ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getBoneAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( boneId ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBone) ) ) ;
	// valRet = pThis->getBoneAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelBone> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelBone>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// String getBoneIdentityOf( EntisGLS4.ModelBone bone ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getBoneIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBone, bone ) ;
	LQT_VERIFY_NULL_PTR( bone ) ;

	LString	valRet ;
	// valRet = pThis->getBoneIdentityOf(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// ulong getMarkerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMarkerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getMarkerIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LString	valRet ;
	// valRet = pThis->getMarkerIdentityAt(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// EntisGLS4.ModelMarker getMarkerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelMarker) ) ) ;
	// valRet = pThis->getMarkerAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelMarker> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelMarker>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelMarker getMarkerAs( String markerId ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_STRING( markerId ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelMarker) ) ) ;
	// valRet = pThis->getMarkerAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelMarker> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelMarker>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// const Vector3d* calcMarkerMatrix( float* pRadius, Matrix3d* pMarkerMatrix, EntisGLS4.ModelMarker marker, const Matrix3d* matSpace, const Vector3d* vSpace ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_calcMarkerMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pRadius ) ;
	LQT_VERIFY_NULL_PTR( pRadius ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, pMarkerMatrix ) ;
	LQT_VERIFY_NULL_PTR( pMarkerMatrix ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelMarker, marker ) ;
	LQT_VERIFY_NULL_PTR( marker ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, matSpace ) ;
	LQT_VERIFY_NULL_PTR( matSpace ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vSpace ) ;
	LQT_VERIFY_NULL_PTR( vSpace ) ;

	LVector3d	valRet ;
	// valRet = pThis->calcMarkerMatrix(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}



