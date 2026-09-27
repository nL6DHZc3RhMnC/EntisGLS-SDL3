
#include <loquaty.h>
#include "EntisGLS4_MaterialLibrary.h"

using namespace Loquaty ;


// MaterialLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_MaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_MaterialLibrary, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// boolean loadLibraryXML( String file, const EntisGLS4.TextureLibrary libTexture )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_loadLibraryXML)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, libTexture ) ;
	LQT_VERIFY_NULL_PTR( libTexture ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadLibraryXML(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Material getMaterialAs( String id, boolean noRefOther ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( noRefOther ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	// valRet = pThis->getMaterialAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Material> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean addMaterialAs( String id, EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_addMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;

	LBoolean	valRet ;
	// valRet = pThis->addMaterialAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean removeMaterialAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_removeMaterialAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeMaterialAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void removeAllMaterials( )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_removeAllMaterials)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;

	// pThis->removeAllMaterials(...) ;

	LQT_RETURN_VOID() ;
}

// long findMaterial( EntisGLS4.Material material ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_findMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;

	LInt64	valRet ;
	// valRet = pThis->findMaterial(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong getMaterialCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMaterialCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getMaterialIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LString	valRet ;
	// valRet = pThis->getMaterialIdentityAt(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getMaterialIdentityOf( EntisGLS4.Material material ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;

	LString	valRet ;
	// valRet = pThis->getMaterialIdentityOf(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// EntisGLS4.Material getMaterialAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getMaterialAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	// valRet = pThis->getMaterialAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Material> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setParentLibrary( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->setParentLibrary(...) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	LUint64	valRet ;
	// valRet = pThis->addReferenceLibrary(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.MaterialLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	// valRet = pThis->getParentLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_MaterialLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getReferenceLibraryCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.MaterialLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	// valRet = pThis->getReferenceLibraryAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_MaterialLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_MaterialLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	// pThis->detachReferenceLibraryAt(...) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.MaterialLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_MaterialLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->detachReferenceLibraryOf(...) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_MaterialLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_MaterialLibrary, pThis ) ;

	// pThis->detachAllReferenceLibraries(...) ;

	LQT_RETURN_VOID() ;
}



