
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ModelPoseLibrary.h>


// ModelPoseLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_ModelPoseLibrary, pThis,
			( new SSmartObject( new S3DModelPoseLibrary ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadLibrary( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_loadLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pPoseLib->LoadLibrary( file.c_str() ) == sglErrSuccess ) ;
}

// boolean readLibrary( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_readLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL( pPoseLib->ReadLibrary( lfile ) == sglErrSuccess ) ;
}

// boolean loadLibraryXML( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_loadLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pPoseLib->LoadLibraryXML( file.c_str() ) == sglErrSuccess ) ;
}

// boolean saveLibrary( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_saveLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pPoseLib->SaveLibrary( file.c_str() ) == sglErrSuccess ) ;
}

// boolean writeLibrary( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_writeLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL( pPoseLib->WriteLibrary( lfile ) == sglErrSuccess ) ;
}

// boolean saveLibraryXML( String file )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_saveLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( file ) ;

	LQT_RETURN_BOOL( pPoseLib->SaveLibraryXML( file.c_str() ) == sglErrSuccess ) ;
}

// EntisGLS4.ModelPose getPoseAs( String id, boolean onlyLocal ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( onlyLocal ) ;

	S3DModelPose *	pPose = pPoseLib->GetPoseAs( id.c_str() ) ;
	if ( pPose == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPose) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPose>(pPose) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPose getPoseAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DModelPose *	pPose = pPoseLib->GetPoseAt( (size_t) index ) ;
	if ( pPose == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPose) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPose>(pPose) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long findPose( EntisGLS4.ModelPose pose ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_findPose)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;
	S3DModelPose *	pPose = pose->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;

	LQT_RETURN_LONG( pPoseLib->FindPosePtr( pPose ) ) ;
}

// ulong getPoseCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;

	LQT_RETURN_ULONG( pPoseLib->GetPoseCount() ) ;
}

// String getPoseIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_STRING( pPoseLib->GetPoseIdentityAt( (size_t) index ) ) ;
}

// String getPoseIdentityOf( EntisGLS4.ModelPose pose, boolean onlyLocal ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getPoseIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;
	S3DModelPose *	pPose = pose->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;
	LQT_FUNC_ARG_BOOL( onlyLocal ) ;

	LQT_RETURN_STRING( pPoseLib->GetPoseIdentityOf( pPose, onlyLocal ) ) ;
}

// void addPoseAs( String id, EntisGLS4.ModelPose pose )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_addPoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPose, pose ) ;
	LQT_VERIFY_NULL_PTR( pose ) ;
	S3DModelPose *	pPose = pose->GetRef<S3DModelPose>() ;
	LQT_VERIFY_NULL_PTR( pPose ) ;

	pPoseLib->AddPoseAs( id.c_str(), pPose ) ;

	LQT_RETURN_VOID() ;
}

// void removePoseAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_removePoseAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	pPoseLib->RemovePoseAs( id.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void removeAllPoses( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_removeAllPoses)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;

	pPoseLib->RemoveAllPoses() ;

	LQT_RETURN_VOID() ;
}

// void setParentLibrary( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	S3DModelPoseLibrary *	pParent = nullptr ;
	if ( lib != nullptr )
	{
		pParent = lib->GetRef<S3DModelPoseLibrary>() ;
	}
	pPoseLib->SetParentLibrary( pParent ) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DModelPoseLibrary *	pRefLib = lib->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	LQT_RETURN_ULONG( pPoseLib->AddReferenceLibrary( pRefLib ) ) ;
}

// EntisGLS4.ModelPoseLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;

	S3DModelPoseLibrary *	pParent = pPoseLib->GetParentLibrary() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;

	LQT_RETURN_ULONG( pPoseLib->GetReferenceLibraryCount() ) ;
}

// EntisGLS4.ModelPoseLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DModelPoseLibrary *	pRefLib = pPoseLib->GetReferenceLibraryAt( (size_t) index ) ;
	if ( pRefLib == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>(pRefLib) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	pPoseLib->DetachReferenceLibraryAt( (size_t) index ) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.ModelPoseLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelPoseLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DModelPoseLibrary *	pRefLib = lib->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	pPoseLib->DetachReferenceLibraryOf( pRefLib ) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelPoseLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelPoseLibrary, pThis ) ;
	S3DModelPoseLibrary *	pPoseLib = pThis->GetRef<S3DModelPoseLibrary>() ;
	LQT_VERIFY_NULL_PTR( pPoseLib ) ;

	pPoseLib->DetachAllReferenceLibrarys() ;

	LQT_RETURN_VOID() ;
}



