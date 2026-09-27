
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx_model_buffer.h>
#include <loquaty/EntisGLS4_MaterialLibrary.h>


// MaterialLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_MaterialLibrary, pThis,
			( new SSmartObject( new S3DMaterialLibrary ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadLibraryXML( String file, const EntisGLS4.TextureLibrary libTexture )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_loadLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, libTexture ) ;
	LQT_VERIFY_NULL_PTR( libTexture ) ;
	S3DTextureLibrary *	pTxtLib = libTexture->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	LQT_RETURN_BOOL
		( pMatLib->LoadLibraryXML( file.c_str(), *pTxtLib ) == sglErrSuccess ) ;
}

// EntisGLS4.Material getMaterialAs( String id, boolean noRefOther ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( noRefOther ) ;

	S3DMaterial *	pMaterial = pMatLib->GetMaterialAs( id.c_str(), noRefOther ) ;
	if ( pMaterial == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>(pMaterial) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean addMaterialAs( String id, EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_addMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	S3DMaterial *	pMaterial = material->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_BOOL
		( pMatLib->AddMaterialAs( id.c_str(), pMaterial ) == sglErrSuccess ) ;
}

// boolean removeMaterialAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_removeMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL
		( pMatLib->RemoveMaterialAs( id.c_str() ) == sglErrSuccess ) ;
}

// void removeAllMaterials( )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_removeAllMaterials)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;

	pMatLib->RemoveAllMaterial() ;

	LQT_RETURN_VOID() ;
}

// long findMaterial( EntisGLS4.Material material ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_findMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	S3DMaterial *	pMaterial = material->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_LONG( pMatLib->FindMaterialPtr( pMaterial ) ) ;
}

// ulong getMaterialCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;

	LQT_RETURN_ULONG( pMatLib->GetMaterialCount() ) ;
}

// String getMaterialIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_STRING( pMatLib->GetMaterialIdentityAt( (size_t) index ) ) ;
}

// String getMaterialIdentityOf( EntisGLS4.Material material ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	S3DMaterial *	pMaterial = material->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_STRING( pMatLib->GetMaterialIdentityOf( pMaterial ) ) ;
}

// EntisGLS4.Material getMaterialAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DMaterial *	pMaterial = pMatLib->GetMaterialAt( (size_t) index ) ;
	if ( pMaterial == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>(pMaterial) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setParentLibrary( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	S3DMaterialLibrary *	pParent = nullptr ;
	if ( lib != nullptr )
	{
		pParent = lib->GetRef<S3DMaterialLibrary>() ;
	}

	pMatLib->SetParentLibrary( pParent ) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DMaterialLibrary *	pRefLib = lib->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	LQT_RETURN_ULONG( pMatLib->AddReferenceLibrary( pRefLib ) ) ;
}

// EntisGLS4.MaterialLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;

	S3DMaterialLibrary *	pParent = pMatLib->GetParentLibrary() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;

	LQT_RETURN_ULONG( pMatLib->GetReferenceLibraryCount() ) ;
}

// EntisGLS4.MaterialLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DMaterialLibrary *	pRefLib = pMatLib->GetReferenceLibraryAt( (size_t) index ) ;
	if ( pRefLib == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>(pRefLib) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	pMatLib->DetachReferenceLibraryAt( (size_t) index ) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DMaterialLibrary *	pRefLib = lib->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	pMatLib->DetachReferenceLibraryOf( pRefLib ) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	S3DMaterialLibrary *	pMatLib = pThis->GetRef<S3DMaterialLibrary>() ;
	LQT_VERIFY_NULL_PTR( pMatLib ) ;

	pMatLib->DetachAllReferenceLibrarys() ;

	LQT_RETURN_VOID() ;
}



