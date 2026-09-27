
#include <loquaty.h>
#include "EntisGLS4_TextureLibrary.h"

using namespace Loquaty ;


// TextureLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_TextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_TextureLibrary, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTextureAs( String id, boolean noRefOther ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( noRefOther ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getTextureAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean addTextureAs( String id, EntisGLS4.Image texture )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_addTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, texture ) ;
	LQT_VERIFY_NULL_PTR( texture ) ;

	LBoolean	valRet ;
	// valRet = pThis->addTextureAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean removeTextureAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_removeTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeTextureAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void removeAllTextures( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_removeAllTextures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	// pThis->removeAllTextures(...) ;

	LQT_RETURN_VOID() ;
}

// long findTexture( EntisGLS4.Image image ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_findTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;

	LInt64	valRet ;
	// valRet = pThis->findTexture(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong getTextureCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getTextureCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getTextureIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LString	valRet ;
	// valRet = pThis->getTextureIdentityAt(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getTextureIdentityOf( EntisGLS4.Image image ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;

	LString	valRet ;
	// valRet = pThis->getTextureIdentityOf(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// EntisGLS4.Image getTextureAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getTextureAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setParentLibrary( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->setParentLibrary(...) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	LUint64	valRet ;
	// valRet = pThis->addReferenceLibrary(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.TextureLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	// valRet = pThis->getParentLibrary(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_TextureLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getReferenceLibraryCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.TextureLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	// valRet = pThis->getReferenceLibraryAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_TextureLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	// pThis->detachReferenceLibraryAt(...) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;

	// pThis->detachReferenceLibraryOf(...) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	// pThis->detachAllReferenceLibraries(...) ;

	LQT_RETURN_VOID() ;
}

// boolean commitToDevice( EntisGLS4.RenderDevice device, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_commitToDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LBoolean	valRet ;
	// valRet = pThis->commitToDevice(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean releaseAllDeviceResources( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_releaseAllDeviceResources)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->releaseAllDeviceResources(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



