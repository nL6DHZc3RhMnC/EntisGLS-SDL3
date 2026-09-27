
#include <loquaty.h>
#include "EntisGLS4_Material.h"

using namespace Loquaty ;


// Material( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Material)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_Material, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SurfaceAttribute* faceAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_faceAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;

	LEntisGLS4_SurfaceAttribute	valRet ;
	// valRet = pThis->faceAttr(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.SurfaceAttribute* backAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_backAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;

	LEntisGLS4_SurfaceAttribute	valRet ;
	// valRet = pThis->backAttr(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean isEnabledBackAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_isEnabledBackAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEnabledBackAttr(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void enableBackAttr( boolean enableBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_enableBackAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_BOOL( enableBack ) ;

	// pThis->enableBackAttr(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTexture( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getTexture(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getBackTexture( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getBackTexture(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// uint getTextureType( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LUint32	valRet ;
	// valRet = pThis->getTextureType(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getBackTextureType( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LUint32	valRet ;
	// valRet = pThis->getBackTextureType(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// float getTextureApply( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureApply)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LFloat	valRet ;
	// valRet = pThis->getTextureApply(...) ;

	LQT_RETURN_FLOAT( valRet ) ;
}

// float getBackTextureApply( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureApply)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LFloat	valRet ;
	// valRet = pThis->getBackTextureApply(...) ;

	LQT_RETURN_FLOAT( valRet ) ;
}

// float getTextureParam( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LFloat	valRet ;
	// valRet = pThis->getTextureParam(...) ;

	LQT_RETURN_FLOAT( valRet ) ;
}

// float getBackTextureParam( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LFloat	valRet ;
	// valRet = pThis->getBackTextureParam(...) ;

	LQT_RETURN_FLOAT( valRet ) ;
}

// int findTextureTypeOf( EntisGLS4.Material.TextureType type ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_findTextureTypeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( type ) ;

	LInt32	valRet ;
	// valRet = pThis->findTextureTypeOf(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// int findBackTextureTypeOf( EntisGLS4.Material.TextureType type ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_findBackTextureTypeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_INT( type ) ;

	LInt32	valRet ;
	// valRet = pThis->findBackTextureTypeOf(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setTexture( EntisGLS4.Image image, int index, EntisGLS4.Material.TextureType type, float apply, float param )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_setTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	LQT_FUNC_ARG_INT( index ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_FLOAT( apply ) ;
	LQT_FUNC_ARG_FLOAT( param ) ;

	// pThis->setTexture(...) ;

	LQT_RETURN_VOID() ;
}

// void setBackTexture( EntisGLS4.Image image, int index, EntisGLS4.Material.TextureType type, float apply, float param )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_setBackTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	LQT_FUNC_ARG_INT( index ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_FLOAT( apply ) ;
	LQT_FUNC_ARG_FLOAT( param ) ;

	// pThis->setBackTexture(...) ;

	LQT_RETURN_VOID() ;
}

// void updateBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_updateBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;

	// pThis->updateBuffer(...) ;

	LQT_RETURN_VOID() ;
}



