
#include <loquaty.h>
#include "EntisGLS4_ModelPoseLibrary.h"

using namespace Loquaty ;


// ModelPoseLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadLibrary( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_loadLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadLibrary(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean readLibrary( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_readLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->readLibrary(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean loadLibraryXML( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_loadLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadLibraryXML(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean saveLibrary( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_saveLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->saveLibrary(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean writeLibrary( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_writeLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->writeLibrary(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean saveLibraryXML( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_saveLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->saveLibraryXML(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.ModelPose getPoseAs( String id, boolean onlyLocal ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( onlyLocal ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPose) ) ) ;
	// valRet = pThis->getPoseAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPose> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPose>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPose getPoseAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPose) ) ) ;
	// valRet = pThis->getPoseAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPose> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPose>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long findPose( EntisGLS4.ModelPose pose ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_findPose)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;

	LInt64	valRet ;
	// valRet = pThis->findPose(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong getPoseCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getPoseCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getPoseIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LString	valRet ;
	// valRet = pThis->getPoseIdentityAt(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getPoseIdentityOf( EntisGLS4.ModelPose pose, boolean onlyLocal ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;
	LQT_FUNC_ARG_BOOL( onlyLocal ) ;

	LString	valRet ;
	// valRet = pThis->getPoseIdentityOf(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void addPoseAs( String id, EntisGLS4.ModelPose pose )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_addPoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;

	// pThis->addPoseAs(...) ;

	LQT_RETURN_VOID() ;
}

// void removePoseAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_removePoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	// pThis->removePoseAs(...) ;

	LQT_RETURN_VOID() ;
}

// void removeAllPoses( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_removeAllPoses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;

	// pThis->removeAllPoses(...) ;

	LQT_RETURN_VOID() ;
}

// void setParentLibrary( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->setParentLibrary(...) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	LUint64	valRet ;
	// valRet = pThis->addReferenceLibrary(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	// valRet = pThis->getParentLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPoseLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getReferenceLibraryCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	// valRet = pThis->getReferenceLibraryAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPoseLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	// pThis->detachReferenceLibraryAt(...) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->detachReferenceLibraryOf(...) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;

	// pThis->detachAllReferenceLibraries(...) ;

	LQT_RETURN_VOID() ;
}



