
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx_model_buffer.h>
#include <loquaty/EntisGLS4_TextureLibrary.h>


// TextureLibrary( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_TextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_TextureLibrary, pThis,
			( new SSmartObject( new S3DTextureLibrary ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTextureAs( String id, boolean noRefOther ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( noRefOther ) ;

	SGLImageObject *	pImage = pTxtLib->GetTextureAs( id.c_str(), noRefOther ) ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pImage) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean addTextureAs( String id, EntisGLS4.Image texture )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_addTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, texture ) ;
	LQT_VERIFY_NULL_PTR( texture ) ;
	SGLImageObject *	pTexture = texture->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pTexture ) ;

	LQT_RETURN_BOOL
		( pTxtLib->AddTextureAs( id.c_str(), pTexture ) == sglErrSuccess ) ;
}

// boolean removeTextureAs( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_removeTextureAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL( pTxtLib->RemoveTextureAs( id.c_str() ) == sglErrSuccess ) ;
}

// void removeAllTextures( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_removeAllTextures)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	pTxtLib->RemoveAllTexture() ;

	LQT_RETURN_VOID() ;
}

// long findTexture( EntisGLS4.Image image ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_findTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_LONG( pTxtLib->FindTexturePtr( pImage ) ) ;
}

// ulong getTextureCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	LQT_RETURN_ULONG( pTxtLib->GetTextureCount() ) ;
}

// String getTextureIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_STRING( pTxtLib->GetTextureIdentityAt( (size_t) index ) ) ;
}

// String getTextureIdentityOf( EntisGLS4.Image image ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_STRING( pTxtLib->GetTextureIdentityOf( pImage ) ) ;
}

// EntisGLS4.Image getTextureAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getTextureAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SGLImageObject *	pImage = pTxtLib->GetTextureAt( (size_t) index ) ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pImage) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void setParentLibrary( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_setParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	S3DTextureLibrary *	pParent = nullptr ;
	if ( lib != nullptr )
	{
		pParent = lib->GetRef<S3DTextureLibrary>() ;
	}

	pTxtLib->SetParentLibrary( pParent ) ;

	LQT_RETURN_VOID() ;
}

// ulong addReferenceLibrary( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_addReferenceLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DTextureLibrary *	pRefLib = lib->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	LQT_RETURN_ULONG( pTxtLib->AddReferenceLibrary( pRefLib ) ) ;
}

// EntisGLS4.TextureLibrary getParentLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getParentLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	S3DTextureLibrary *	pParent = pTxtLib->GetParentLibrary() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getReferenceLibraryCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getReferenceLibraryCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	LQT_RETURN_ULONG( pTxtLib->GetReferenceLibraryCount() ) ;
}

// EntisGLS4.TextureLibrary getReferenceLibraryAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_getReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DTextureLibrary *	pRefLib = pTxtLib->GetReferenceLibraryAt( (size_t) index ) ;
	if ( pRefLib == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_TextureLibrary>(pRefLib) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void detachReferenceLibraryAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachReferenceLibraryAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	pTxtLib->DetachReferenceLibraryAt( (size_t) index ) ;

	LQT_RETURN_VOID() ;
}

// void detachReferenceLibraryOf( EntisGLS4.TextureLibrary lib )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachReferenceLibraryOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_TextureLibrary, lib ) ;
	LQT_VERIFY_NULL_PTR( lib ) ;
	S3DTextureLibrary *	pRefLib = lib->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pRefLib ) ;

	pTxtLib->DetachReferenceLibraryOf( pRefLib ) ;

	LQT_RETURN_VOID() ;
}

// void detachAllReferenceLibraries( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_detachAllReferenceLibraries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	pTxtLib->DetachAllReferenceLibrarys() ;

	LQT_RETURN_VOID() ;
}

// boolean commitToDevice( EntisGLS4.RenderDevice device, long msecTimeout )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_commitToDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;
	S3DRenderDevice *	pDevice = device->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;
	LQT_FUNC_ARG_LONG( msecTimeout ) ;

	LQT_RETURN_BOOL
		( pTxtLib->CommitToDevice( pDevice, msecTimeout ) == sglErrSuccess ) ;
}

// boolean releaseAllDeviceResources( )
IMPL_LOQUATY_FUNC(EntisGLS4_TextureLibrary_releaseAllDeviceResources)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_TextureLibrary, pThis ) ;
	S3DTextureLibrary *	pTxtLib = pThis->GetRef<S3DTextureLibrary>() ;
	LQT_VERIFY_NULL_PTR( pTxtLib ) ;

	LQT_RETURN_BOOL( pTxtLib->ReleaseAllDeviceResources() == sglErrSuccess ) ;
}



